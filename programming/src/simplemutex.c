/*
一个多线程通过互斥锁同步的实例
如果注释掉【1】【2】【3】【4】处的 if 块，线程 thrd1 和 线程 thrd2 会竞争访问全局变量 sharedi，导致线程不同步。
不同步的原因：假设 thrd1 执行了 1000 次后，在 ++tmp 处时间片用完，此时 sharedi = 1000，tmp = 1001，OS 调度 thrd2，
假设 thrd 2 也执行了 1000 次，使 sharedi 由 1000 增长到 2000, 此时时间片用完，OS 调度 thrd1，开始执行 task1 中
sharedi = tmp;，此语句将 task1 中的 tmp = 1001 赋值给 sharedi 而不是 task2 中值为 2001 的 tmp。

去掉【1】【2】【3】【4】处的注释后，可以实现线程同步。


*/


#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

void task1(void);
void task2(void);
void task3(void);
int sharedi = 0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER; // 初始化锁为解锁状态

int main(int argc, char **argv)
{
	pthread_t thrd1, thrd2, thrd3;
	int ret;
	ret = pthread_create(&thrd1, NULL, (void *)task1, NULL);
	ret = pthread_create(&thrd2, NULL, (void *)task2, NULL);
	pthread_join(thrd1, NULL);
	pthread_join(thrd2, NULL);
	printf("sharedi = %d\n", sharedi);

	return 0;
}

void task1(void)
{
	long i, tmp;
	for (i = 0; i < 1000000; ++i)
	{
		// /* 【1】
		if (pthread_mutex_lock(&mutex) != 0)  // 上锁成功返回 0
		{
			perror("pthread_mutex_lock");
			exit(EXIT_FAILURE);
		}
		// */ 
		
		tmp = sharedi;	// 关键区不应该被打断，采用锁的机制对关键区的保护
		++tmp;
		sharedi = tmp;
		
		// /* 【2】
		if (pthread_mutex_unlock(&mutex) != 0)  // 解锁成功返回 0
		{
			perror("pthread_mutex_unlock");
			exit(EXIT_FAILURE);
		}
		// */ 
	}
}

void task2(void)
{
	long i, tmp;
	for (i = 0; i < 1000000; ++i)
	{
		// /* 【3】
		if (pthread_mutex_lock(&mutex) != 0)
		{
			perror("pthread_mutex_lock");
			exit(EXIT_FAILURE);
		}
		// */

		tmp = sharedi;
		++tmp;
		sharedi = tmp;
		
		// /* 【4】
		if (pthread_mutex_unlock(&mutex) != 0)
		{
			perror("pthread_mutex_unlock");
			exit(EXIT_FAILURE);
		}
		// */
	}
}
