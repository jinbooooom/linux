#include <cstring>
#include <iostream>
#include <memory>

struct ncclMemoryStack {
    struct Hunk {            // 内存块
        struct Hunk *above;  // reverse stack pointer
        size_t size;         // size of this allocation (including this header struct)
    };
    struct Unhunk {  // proxy header for objects allocated out-of-hunk
        struct Unhunk *next;
        void *obj;
    };
    struct Frame {
        struct Hunk *hunk;       // top of non-empty hunks， 指向当前hunk的指针
        uintptr_t bumper, end;   // points into top hunk， 当前分配指针和当前hunk的结束指针
        struct Unhunk *unhunks;  // 指向unhunked内存块链表的头指针
        struct Frame *below;     // 指向下一个frame的指针
    };

    static void *allocateSpilled(struct ncclMemoryStack *me, size_t size, size_t align);
    static void *allocate(struct ncclMemoryStack *me, size_t size, size_t align);

    struct Hunk stub;
    struct Frame topFrame;
};

inline void ncclMemoryStackConstruct(struct ncclMemoryStack *me)
{
    me->stub.above = nullptr;
    me->stub.size = 0;
    me->topFrame.hunk = &me->stub;
    me->topFrame.bumper = 0;
    me->topFrame.end = 0;
    me->topFrame.unhunks = nullptr;
    me->topFrame.below = nullptr;
}

// 尝试在当前hunk中分配内存，如果当前hunk不足以分配所需内存，则调用allocateSpilled函数
inline void *ncclMemoryStack::allocate(struct ncclMemoryStack *me, size_t size, size_t align)
{
    uintptr_t o = (me->topFrame.bumper + align - 1) & -uintptr_t(align);
    void *obj;
    if (__builtin_expect(o + size <= me->topFrame.end, true)) {
        me->topFrame.bumper = o + size;
        obj = reinterpret_cast<void *>(o);
    } else {
        obj = allocateSpilled(me, size, align);
    }
    return obj;
}

template <typename T>
inline T *ncclMemoryStackAlloc(struct ncclMemoryStack *me, size_t n)
{
    void *obj = ncclMemoryStack::allocate(me, n * sizeof(T), alignof(T));
    memset(obj, 0, n * sizeof(T));
    return (T *) obj;
}

inline void ncclMemoryStackPush(struct ncclMemoryStack *me)
{
    using Frame = ncclMemoryStack::Frame;
    Frame tmp = me->topFrame;
    Frame *snapshot = (Frame *) ncclMemoryStack::allocate(me, sizeof(Frame), alignof(Frame));
    *snapshot = tmp;  // C++ struct assignment
    me->topFrame.unhunks = nullptr;
    me->topFrame.below = snapshot;
}

inline void ncclMemoryStackPop(struct ncclMemoryStack *me)
{
    ncclMemoryStack::Unhunk *un = me->topFrame.unhunks;
    while (un != nullptr) {
        free(un->obj);
        un = un->next;
    }
    me->topFrame = *me->topFrame.below;  // C++ struct assignment
}

void *ncclMemoryStack::allocateSpilled(struct ncclMemoryStack *me, size_t size, size_t align)
{
    // `me->hunks` points to the top of the stack non-empty hunks. Hunks above
    // this (reachable via `->above`) are empty.
    struct Hunk *top = me->topFrame.hunk;
    size_t mallocSize = 0;

    // If we have lots of space left in hunk but that wasn't enough then we'll
    // allocate the object unhunked.
    if (me->topFrame.end - me->topFrame.bumper >= 8 << 10)
        goto unhunked;

    // If we have another hunk (which must be empty) waiting above this one and
    // the object fits then use that.
    if (top && top->above) {
        struct Hunk *top1 = top->above;
        uintptr_t uobj = (reinterpret_cast<uintptr_t>(top1) + sizeof(struct Hunk) + align - 1) & -uintptr_t(align);
        if (uobj + size <= reinterpret_cast<uintptr_t>(top1) + top1->size) {
            me->topFrame.hunk = top1;
            me->topFrame.bumper = uobj + size;
            me->topFrame.end = reinterpret_cast<uintptr_t>(top1) + top1->size;
            return reinterpret_cast<void *>(uobj);
        }
    }

    {  // If the next hunk we're going to allocate wouldn't be big enough but the
        // Unhunk proxy fits in the current hunk then go allocate as unhunked.
        size_t nextSize = (top ? top->size : 0) + (64 << 10);
        constexpr size_t maxAlign = 64;
        if (nextSize < sizeof(struct Hunk) + maxAlign + size) {
            uintptr_t uproxy = (me->topFrame.bumper + alignof(Unhunk) - 1) & -uintptr_t(alignof(Unhunk));
            if (uproxy + sizeof(struct Unhunk) <= me->topFrame.end)
                goto unhunked;
        }

        // At this point we must need another hunk, either to fit the object
        // itself or its Unhunk proxy.
        mallocSize = nextSize;
        printf("%s:%d memory stack hunk malloc(%llu)\n", __FILE__, __LINE__, (unsigned long long) mallocSize);
        struct Hunk *top1 = (struct Hunk *) malloc(mallocSize);
        if (top1 == nullptr)
            goto malloc_exhausted;
        top1->size = nextSize;
        top1->above = nullptr;
        if (top)
            top->above = top1;
        top = top1;
        me->topFrame.hunk = top;
        me->topFrame.end = reinterpret_cast<uintptr_t>(top) + nextSize;
        me->topFrame.bumper = reinterpret_cast<uintptr_t>(top) + sizeof(struct Hunk);
    }

    {  // Try to fit object in the new top hunk.
        uintptr_t uobj = (me->topFrame.bumper + align - 1) & -uintptr_t(align);
        if (uobj + size <= me->topFrame.end) {
            me->topFrame.bumper = uobj + size;
            return reinterpret_cast<void *>(uobj);
        }
    }

unhunked: {  // We need to allocate the object out-of-band and put an Unhunk proxy in-band
    // to keep track of it.
    uintptr_t uproxy = (me->topFrame.bumper + alignof(Unhunk) - 1) & -uintptr_t(alignof(Unhunk));
    Unhunk *proxy = reinterpret_cast<Unhunk *>(uproxy);
    me->topFrame.bumper = uproxy + sizeof(Unhunk);
    proxy->next = me->topFrame.unhunks;
    me->topFrame.unhunks = proxy;
    mallocSize = size;
    proxy->obj = malloc(mallocSize);
    printf("%s:%d memory stack non-hunk malloc(%llu)\n", __FILE__, __LINE__, (unsigned long long) mallocSize);
    if (proxy->obj == nullptr)
        goto malloc_exhausted;
    return proxy->obj;
}

malloc_exhausted:
    printf("%s:%d Unrecoverable error detected: malloc(size=%llu) returned null.\n", __FILE__, __LINE__,
           (unsigned long long) mallocSize);
    abort();
}

void ncclMemoryStackDestruct(struct ncclMemoryStack *me)
{
    // Free unhunks first because both the frames and unhunk proxies lie within the hunks.
    struct ncclMemoryStack::Frame *f = &me->topFrame;
    while (f != nullptr) {
        struct ncclMemoryStack::Unhunk *u = f->unhunks;
        while (u != nullptr) {
            free(u->obj);
            u = u->next;
        }
        f = f->below;
    }
    // Free hunks
    struct ncclMemoryStack::Hunk *h = me->stub.above;
    while (h != nullptr) {
        struct ncclMemoryStack::Hunk *h1 = h->above;
        free(h);
        h = h1;
    }
}

int main()
{
    // 初始化 ncclMemoryStack
    struct ncclMemoryStack stack;
    ncclMemoryStackConstruct(&stack);

    // 压入第一个栈帧
    ncclMemoryStackPush(&stack);

    // 在第一个栈帧中分配内存
    int *array1 = ncclMemoryStackAlloc<int>(&stack, 5);
    if (array1) {
        for (int i = 0; i < 5; ++i) {
            array1[i] = i;
        }
        for (int i = 0; i < 5; ++i) {
            printf("%d ", array1[i]);
        }
        printf("\n");
    }

    // 压入第二个栈帧
    ncclMemoryStackPush(&stack);

    // 在第二个栈帧中分配内存
    int *array2 = ncclMemoryStackAlloc<int>(&stack, 5);
    if (array2) {
        for (int i = 0; i < 5; ++i) {
            array2[i] = i + 5;
        }
        for (int i = 0; i < 5; ++i) {
            printf("%d ", array2[i]);
        }
        printf("\n");
    }

    // 弹出第二个栈帧，释放内存
    ncclMemoryStackPop(&stack);

    // 弹出第一个栈帧，释放内存
    ncclMemoryStackPop(&stack);

    // 销毁 stack
    ncclMemoryStackDestruct(&stack);

    return 0;
}