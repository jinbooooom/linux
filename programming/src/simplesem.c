/*
一个通过信号量实现多线程同步的实例
*/

#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>

#define MAXSIZE 10
// const int MAXSIZE = 10;
int stack[MAXSIZE][2];
int size = 0;
sem_t sem;						// 其实 sem_t 就是长整型
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;	// 编译时初始化锁为解锁状态

void provide_data(void)
{
	for (int i = 0; i < MAXSIZE; ++i)
	{
		stack[i][0] = i;
		stack[i][1] = i;
		sem_post(&sem);				// 信号量加 1 
	}
}

void handle_data1(void)
{
	int i;
	while (pthread_mutex_lock(&mutex),((i = size++) < MAXSIZE))	// 访问前先上锁
	{
		pthread_mutex_unlock(&mutex);				// Q1：马上解锁？
		sem_wait(&sem);						// 信号量减 1
		printf("Plus: %d + %d = %d\n", stack[i][0], stack[i][1], stack[i][0] + stack[i][1]);
	}
	pthread_mutex_unlock(&mutex);					// Q2：解两次锁？
}


void handle_data2(void)
{
	int i;
	while (pthread_mutex_lock(&mutex),((i = size++) < MAXSIZE))
	{
		pthread_mutex_unlock(&mutex);
		sem_wait(&sem);
		printf("Multiple: %d * %d = %d\n", stack[i][0], stack[i][1], stack[i][0] + stack[i][1]);
	}
	pthread_mutex_unlock(&mutex);
}

int main(int argc, char **argv)
{
	pthread_t thrd1, thrd2, thrd3;
	sem_init(&sem, 0, 0);
	pthread_create(&thrd1, NULL, (void *)handle_data1, NULL);
	pthread_create(&thrd2, NULL, (void *)handle_data2, NULL);
	pthread_create(&thrd3, NULL, (void *)provide_data, NULL);
	pthread_join(thrd1, NULL);
	pthread_join(thrd2, NULL);
	pthread_join(thrd3, NULL);
	sem_destroy(&sem);						// 删除信号量
	return 0;
}



/* #####################################################################
jinbo@fang:~/gitme/linux/programming/src$ gcc simplesem.c -lpthread -o simplesem.o
jinbo@fang:~/gitme/linux/programming/src$ ./simplesem.o 
Plus: 0 + 0 = 0
Plus: 2 + 2 = 4
Plus: 3 + 3 = 6
Plus: 4 + 4 = 8
Plus: 5 + 5 = 10
Plus: 6 + 6 = 12
Plus: 7 + 7 = 14
Plus: 8 + 8 = 16
Plus: 9 + 9 = 18
Multiple: 1 * 1 = 2

Question:
运行结果跟书里完全一样，但有两处不理解：

源代码 Q1 处: while 里上锁后为什么不执行关键区代码就马上解锁？
源代码 Q2 处：如果 while 循环里解过一次锁，跳出 while 循环后，又要再解一次锁？

*/

