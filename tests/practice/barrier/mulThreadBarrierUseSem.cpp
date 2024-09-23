#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <unistd.h>

#define NUM_THREADS 5

sem_t semMutex;
sem_t semBarrier;
int count = 0;

void *thread_function(void *arg)
{
    int thread_id = *(int *) arg;

    // 模拟一些工作，且模拟第一个线程运行最慢
    if (0 == thread_id) {
        sleep(1);
    }
    printf("Thread %d is doing some work...\n", thread_id);

    // 到达屏障
    sem_wait(&semMutex);
    count++;
    sem_post(&semMutex);

    // 如果所有线程都到达屏障，释放屏障
    if (count == NUM_THREADS) {
        printf("Thread %d 释放屏障\n", thread_id);
        for (int i = 0; i < NUM_THREADS; i++) {
            sem_post(&semBarrier);
        }
    }

    // 等待其他线程
    sem_wait(&semBarrier);

    // 继续执行
    printf("Thread %d passed the semBarrier and continues...\n", thread_id);

    pthread_exit(NULL);
}

int main()
{
    pthread_t threads[NUM_THREADS];
    int thread_ids[NUM_THREADS];

    // 初始化信号量,
    // 用于初始化无名信号量，适用于同一进程内的线程间同步或父子进程间的同步（如果信号量是在共享内存中创建的）
    sem_init(&semMutex, 0, 1);
    sem_init(&semBarrier, 0, 0);

    // 创建线程
    for (int i = 0; i < NUM_THREADS; i++) {
        thread_ids[i] = i;
        pthread_create(&threads[i], NULL, thread_function, &thread_ids[i]);
    }

    // 等待所有线程完成
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    // 销毁信号量
    sem_destroy(&semMutex);
    sem_destroy(&semBarrier);

    return 0;
}