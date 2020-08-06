/*
一个杀死线程的示例
*/

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

void task1(int *counter);
void task2(int *counter);
void cleanup(int counter1, int counter2);

int g1 = 0, g2 = 0;

int main(int argc, char **argv)
{
	pthread_t thrd1, thrd2;
	int ret;
	ret = pthread_create(&thrd1, NULL, (void *)task1, (void *)&g1);
	ret = pthread_create(&thrd2, NULL, (void *)task2, (void *)&g2);
	sleep(2); // sleep 导致 task2 结束后，task1 还有一些时间打印

	pthread_join(thrd2, NULL);
	printf("in main, after func pthread_join(thrd2, NULL);\n");
	pthread_cancel(thrd1);										// 杀死线程
	printf("in main, after pthread_cancel\n");
	pthread_join(thrd1, NULL);
	printf("in main, after func pthread_join(thrd1, NULL);\n");

	cleanup(g1, g2);
	exit(EXIT_SUCCESS);
}

void task1(int *counter)
{
	volatile int i;
	// pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);			// 设置线程属性以拒绝被杀死
	pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);	// 【1】设置线程属性为立即被杀死，默认是延迟被杀死
	while ((*counter) < 5)
	{
		printf("task1 count: %d\n", *counter);
		++(*counter);
		sleep(1);
	}
	printf("*counter in task1 is: %d\n", *counter);
}

void task2(int *counter)
{
	while (*counter < 5)
	{
		printf("task2 count: %d\n", *counter);
		++(*counter);
		// sleep(1);
	}
}

void cleanup(int counter1, int counter2)
{
	printf("total iterations: %d\n", counter1 + counter2);
}

/*#######################################################################

jinbo@fang:~/gitme/linux/programming/multithreading$ gcc thrdcancel.c -o thrdcancel.o -lpthread
jinbo@fang:~/gitme/linux/programming/multithreading$ ./thrdcancel.o
task1 count: 0
task2 count: 0
task2 count: 1
task2 count: 2
task2 count: 3
task2 count: 4
task1 count: 1
in main, after func pthread_join(thrd2, NULL);
in main, after pthread_cancel
in main, after func pthread_join(thrd1, NULL);
total iterations: 7

*/
