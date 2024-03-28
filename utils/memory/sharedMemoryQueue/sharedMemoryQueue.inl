#include "sharedMemoryQueue.h"

template <typename T>
SMQueue<T>::SMQueue(size_t size, key_t key) : mAllocSize(size + 1), mKey(key)
{
    // 创建共享内存
    mShmid = shmget(mKey, sizeof(T) * (mAllocSize + sizeof(SharedData)), IPC_CREAT | 0666);
    if (mShmid == -1) {
        std::cerr << "Failed to create shared memory" << std::endl;
        exit(EXIT_FAILURE);
    }

    // 将共享内存附加到进程地址空间
    mBuffer = (T *) shmat(mShmid, nullptr, 0);
    if (mBuffer == (T *) -1) {
        std::cerr << "Failed to attach shared memory" << std::endl;
        exit(EXIT_FAILURE);
    }

    mPAttr = (SharedData *) (mBuffer + mAllocSize);

    // 初始化信号量
    mSemName = "SMQueue_key" + std::to_string(key);
    mSemMutex = sem_open(mSemName.c_str(), O_CREAT, 0644, 1);

    if (mSemMutex == SEM_FAILED) {
        printf("Failed to create semaphore\n");
        exit(EXIT_FAILURE);
    }

    // 初始化队列
    mPAttr->front = mPAttr->rear = 0;
}

template <typename T>
SMQueue<T>::~SMQueue()
{
    // 分离共享内存
    shmdt(mBuffer);

    // 删除共享内存和信号量
    shmctl(mShmid, IPC_RMID, nullptr);
    sem_unlink(mSemName.c_str());
}

template <typename T>
int SMQueue<T>::Push(T value)
{
    sem_wait(mSemMutex);
    int pushSize = 0;

    if ((mPAttr->rear + 1) % mAllocSize == mPAttr->front) {
        // printf("queue is full, push failed\n");
    } else {
        mBuffer[mPAttr->rear] = value;
        mPAttr->rear = (mPAttr->rear + 1) % mAllocSize;
        pushSize = 1;

        std::cout << "push " << value << std::endl;
        Show();
    }

    sem_post(mSemMutex);

    return pushSize;
}

template <typename T>
int SMQueue<T>::Pop()
{
    sem_wait(mSemMutex);
    T value = -1;

    if (mPAttr->front == mPAttr->rear)  // 队列为空
    {
        // printf("queue is empty, pop failed\n");
    } else {
        value = mBuffer[mPAttr->front];
        mPAttr->front = (mPAttr->front + 1) % mAllocSize;

        std::cout << "pop " << value << std::endl;
        Show();
    }

    sem_post(mSemMutex);

    return value;
}

template <typename T>
bool SMQueue<T>::Empty()
{
    sem_wait(mSemMutex);
    bool isEmpty = (mPAttr->front == mPAttr->rear);
    sem_post(mSemMutex);
    return isEmpty;
}

template <typename T>
size_t SMQueue<T>::Size()
{
    sem_wait(mSemMutex);
    size_t queueSize = (mPAttr->rear - mPAttr->front + mAllocSize) % mAllocSize;
    sem_post(mSemMutex);
    return queueSize;
}

template <typename T>
void SMQueue<T>::Show()
{
#if 0
    for (size_t i = 0; i < mAllocSize; ++i) {
        printf("%d,", mBuffer[i]);
    }
    printf("\n");
#endif
}
