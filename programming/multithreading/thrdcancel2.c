/*
一个在调用点杀死线程的示例
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
	sleep(2);
	pthread_join(thrd2, NULL);
	pthread_cancel(thrd1);										// 杀死线程
	printf("in main, after pthread_cancel\n");
	pthread_join(thrd1, NULL);

	cleanup(g1, g2);
	exit(EXIT_SUCCESS);
}

void task1(int *counter)
{
	volatile int i;
	while ((*counter) < 5)
	{
		printf("task1 count: %d\n", *counter);
		++(*counter);
		for (i = 0; i < 800000000; ++i)
		{
			pthread_testcancel();
			++(*counter);
		}
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
	}
}

void cleanup(int counter1, int counter2)
{
	printf("total iterations: %d\n", counter1 + counter2);
}

/*#######################################################################

jinbo@fang:~/gitme/linux/programming/multithreading$ gcc thrdcancel2.c -o thrdcancel2.o -lpthread
jinbo@fang:~/gitme/linux/programming/multithreading$  ./thrdcancel2.o
task1 count: 0
task2 count: 0
task2 count: 1
task2 count: 2
task2 count: 3
task2 count: 4
in main, after pthread_cancel
total iterations: 800000006

解析：在延迟杀死的情况下，线程会在接受到杀死要求后，延迟遇到第一个杀死点被杀死。
而接收到杀死命令时，线程正在执行 while 循环，故没有被杀死，直到退出循环后，遇到杀死点 sleep(1) 才被杀死。 


*/
