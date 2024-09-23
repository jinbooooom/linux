#include <iostream>
#include <thread>

pthread_barrier_t barrier;

void *worker(void *arg)
{
    printf("Thread %ld is working\n", (long) arg);
    pthread_barrier_wait(&barrier);  // 线程到达 barrier 并等待
    printf("Thread %ld passed the barrier\n", (long) arg);
    return nullptr;
}

int main()
{
    const int N = 5;
    pthread_t threads[N];
    pthread_barrier_init(&barrier, nullptr, N);  // 初始化 barrier，需要 N 个线程到达

    for (long i = 0; i < N; ++i) {
        pthread_create(&threads[i], nullptr, worker, (void *) i);
    }

    for (int i = 0; i < N; ++i) {
        pthread_join(threads[i], nullptr);
    }

    pthread_barrier_destroy(&barrier);  // 销毁 barrier

    return 0;
}