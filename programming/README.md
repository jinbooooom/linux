# 多线程编程初步

## 使用多线程的好处

- 每个进程有一个地址空间和一个控制线程，多线程并行实体共享同一个地址空间和所有可用数据的能力，这是多进程模型（具有不同的地址空间）无法做到的。
- 提高并发性。通过线程可方便有效地实现并发性。进程可创建多个线程来执行同一程序的不同部分。
- 开销少，易于调度。线程比进程更轻量级，创建线程比创建进程要快，所需开销很少。。
- 利于充分发挥多处理器的功能。通过创建多线程进程，每个线程在一个处理器上运行，从而实现应用程序的并发性，使每个处理器都得到充分运行。

## 基本 API

- `pthread_create` 创建线程

```c++
int pthread_create(pthread_t *thread, pthread_attr_t *attr, void *(*func)(void *), void *arg); // 函数原型
// pthread_t *thread：线程的标识
// pthread_attr_t *attr：线程的属性指针，如果填 NULL 表明用系统缺省定义的属性
// void *(*func)(void *)：被创建线程的程序代码
// void *arg：func 指向的函数的参数
```

- `pthread_exit` 结束线程

``` C++
pthread_exit(void *retval)  // retval 用于存放结束线程的退出状态
```

- `pthread_join` 等待线程结束

```C++
int pthread_join(pthread_t th, void ** thread_return); // 函数原型
// 指针 thread_return 指向的位置存放终止线程的返回状态
```

### 互斥锁

在主线程中初始化锁为解锁状态：

- `pthread_mutex_t mutex;`
- `pthread_mutex_init(&mutex, NULL);`

在编译阶段初始化锁为解锁状态

- `pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;`

在访问共享对象前进行枷锁操作

- `pthread_mutex_lock(&mutex);`

访问共享对象后进行解锁操作

- `pthread_mutex_unlock(&mutex);`

### 信号量

以下方法均需要头文件 `#include <semaphore.h>`

```C
int sem_init(sem_t *sem, int pshared, unsigned int value);
```

功能：初始化信号量。成功返回 0，失败 -1

```C
int sem_wait(sem_t *sem);
```

功能：实现信号量 -1，当 sem = 0 阻塞。成功返回 0，失败 -1

```
int sem_post(sem_t *sem);
```

功能：实现信号量 +1，当有线程阻塞在这个信号量上，调用这个函数会使其中的一个线程不再阻塞，选择机制同样是由线程的调度策略来决定。

成功返回 0，错误返回 -1

```C
int sem_destroy(sem_t *sem);
```

功能：删除信号量。成功返回 0，失败 -1

### 示例程序

- [创建多线程](multithreading/thrdcreat.c)
- [互斥量](multithreading/simplemutex.c)

- [信号量](multithreading/simplesem.c)

