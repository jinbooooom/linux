#include <signal.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <iostream>

int shmid;  // 全局变量

void cleanup(int signum, siginfo_t* info, void* context) {
    std::cout << "Caught signal " << signum << ", cleaning up shared memory..." << std::endl;
    if (shmid != -1) {
        shmctl(shmid, IPC_RMID, NULL);
    }
    exit(signum);
}

int main() {
    struct sigaction sa;
    sa.sa_sigaction = cleanup;
    sa.sa_flags = SA_SIGINFO;
    sigemptyset(&sa.sa_mask);

    // 注册信号处理函数
    sigaction(SIGSEGV, &sa, NULL);
    sigaction(SIGABRT, &sa, NULL);
    sigaction(SIGINT, &sa, NULL);

    // 创建共享内存
    shmid = shmget(IPC_PRIVATE, 1024, IPC_CREAT | 0666);
    if (shmid == -1) {
        perror("shmget");
        return 1;
    }

    // 模拟程序崩溃
    int* ptr = nullptr;
    printf("before crash\n");
    *ptr = 42;  // 这将导致段错误

    // 正常情况下删除共享内存
    shmctl(shmid, IPC_RMID, NULL);

    return 0;
}