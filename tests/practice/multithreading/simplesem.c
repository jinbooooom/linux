/*
一个通过信号量实现多线程同步的实例
*/

#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <unistd.h>

#define MAXSIZE 10
// const int MAXSIZE = 10;
int stack[MAXSIZE][2];
int size = 0;
sem_t sem;                                          // 其实 sem_t 就是长整型
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;  // 编译时初始化锁为解锁状态

void provide_data(void)
{
    printf("start provide_data\n");
    for (int i = 0; i < MAXSIZE; ++i)
    {
        stack[i][0] = i;
        stack[i][1] = i;
        sem_post(&sem);  // 信号量加 1
    }
    // 如果加了下面这两行，意味着执行 handle_data1 和 handle_data2需要 10个资源，但实际上只给了 8 个，那么线程会在打印 8
    // 次 (Plus 或者 Multiple) 后，一直等待资源。 sem_wait(&sem); sem_wait(&sem);
    printf("exit provide_data\n");
}

void handle_data1(void)
{
    printf("start handle_data1\n");
    int i;
    while (pthread_mutex_lock(&mutex), ((i = size++) < MAXSIZE))  // 访问前先上锁，保护 size
    {
        pthread_mutex_unlock(&mutex);  // Q1：马上解锁？
        sem_wait(&sem);                // 信号量减 1
        printf("Plus: %d + %d = %d\n", stack[i][0], stack[i][1], stack[i][0] + stack[i][1]);
    }
    pthread_mutex_unlock(&mutex);  // Q2：解两次锁？
    printf("exit handle_data1\n");
}

void handle_data2(void)
{
    printf("start handle_data2\n");
    int i;
    while (pthread_mutex_lock(&mutex), ((i = size++) < MAXSIZE))
    {
        pthread_mutex_unlock(&mutex);
        sem_wait(&sem);
        printf("Multiple: %d * %d = %d\n", stack[i][0], stack[i][1], stack[i][0] * stack[i][1]);
    }
    pthread_mutex_unlock(&mutex);
    // int a = pthread_mutex_unlock(&mutex);
    // printf("a = %d\n", a);
    printf("exit handle_data2\n");
}

int main(int argc, char **argv)
{
    pthread_t thrd1, thrd2, thrd3;
    sem_init(&sem, 0, 0);
    /**
     * int sem_init (sem_t *__sem, int __pshared, unsigned int __value)
     * __sem：指向信号量结构的一个指针
     * __pshared：不为 0 时，信号量在进程间共享，否则只能为当前进程的所有线程共享
     * __value：信号量初值
     */
    pthread_create(&thrd1, NULL, (void *)handle_data1, NULL);
    pthread_create(&thrd2, NULL, (void *)handle_data2, NULL);
    pthread_create(&thrd3, NULL, (void *)provide_data, NULL);
    pthread_join(thrd1, NULL);
    pthread_join(thrd2, NULL);
    pthread_join(thrd3, NULL);
    sem_destroy(&sem);  // 删除信号量
    return 0;
}

/* #####################################################################

// build && run
gcc simplesem.c -lpthread -o exec/simplesem.o
exec/simplesem.o

Plus: 0 + 0 = 0
Plus: 2 + 2 = 4
Plus: 3 + 3 = 6
Plus: 4 + 4 = 8
Plus: 5 + 5 = 10
Plus: 6 + 6 = 12
Plus: 7 + 7 = 14
Plus: 8 + 8 = 16
Plus: 9 + 9 = 18
Multiple: 1 * 1 = 1

Question:
运行结果跟书里完全一样，但有两处不理解：

源代码 Q1 处: while 里上锁后为什么不执行关键区代码就马上解锁？
A：关键区代码要看你保护什么了，例子里觉得要保护 size。

源代码 Q2 处：如果 while 循环里解过一次锁，跳出 while 循环后，又要再解一次锁？
A：while 循环如果不满足，循环体是不执行的，所以退出时需要再解锁一次。即使满足 while
循环。在循环内部解过一次锁，退出后去解锁一个未加锁的互斥量，对程序运行结果没影响。
*/
