#include "sharedMemoryQueue.h"

// 队列最大容量
const int KEY = 5;
const int MAX_SIZE = 3;
const int MAX_PROCESS_SIZE = 3;

template <typename T>
void producerProcess(SMQueue<T> &queue, int base)
{
    for (int i = 1; i <= 5; ++i) {
        usleep(1000000);  // 每秒生产一个数据
        auto product = base * 100 + i;

        while (!queue.Push(product)) {
            usleep(1000);
        }
    }
}

template <typename T>
void consumerProcess(SMQueue<T> &queue)
{

    for (int i = 0; i < 5 * MAX_PROCESS_SIZE; ++i) {
        usleep(500000);  // 每1.5秒消费一个数据
        printf("epoch%d, ", i);
        while (queue.Empty()) {
            usleep(1000);
        }
        queue.Pop();
    }
}

int main()
{
    SMQueue<float> queue(MAX_SIZE, KEY);
    pid_t pid[MAX_PROCESS_SIZE];
    for (int i = 0; i < MAX_PROCESS_SIZE; ++i) {
        pid[i] = fork();

        if (pid[i] == -1) {
            std::cerr << "Fork failed" << std::endl;
            return 1;
        }

        if (pid[i] > 0) {

        } else {
            // 子进程作为生产者
            producerProcess(queue, i);
            return 0;
        }
    }

    // 父进程作为消费者
    consumerProcess(queue);

    // 等待子进程结束
    for (int i = 0; i < MAX_PROCESS_SIZE; ++i) {
        waitpid(pid[i], nullptr, 0);
    }

    return 0;
}