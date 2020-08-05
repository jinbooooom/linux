#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>
#include <unistd.h>

void task1(int *counter);
void task2(int *counter);
void cleanup(int counter1, int counter2);

int g1 = 0;
int g2 = 0;

int main(int argc, char **argv)
{
	pthread_t thrd1, thrd2;
	int ret;
	void *retval;
	
	ret = pthread_create(&thrd1, NULL, (void *)task1, (void *)&g1);
	printf("I'm in main function\n"); 		// 【Q1】
	
	ret = pthread_create(&thrd2, NULL, (void *)task2, (void *)&g2);
 	cleanup(g1, g2);				// 【Q2】
	getchar();

	int tmp;	
	if ((tmp = pthread_join(thrd1, &retval)) != 0)  // pthread_join 成功调用后，将 thrd1 的退出值存到 retval
	{
		switch (tmp)
		{
			case EINVAL:
				printf("pthread_join return value is EINVAL: %d\n", tmp);
				break;
			case ESRCH:
				printf("pthread_join return value is ESRCH: %d\n", tmp);
				break;
			default:
				printf("pthread_join return value is others: %d\n", tmp);
				break;
		}

	}

	printf("return val of task1 is %d\n", (int)retval); // 警告强制类型转换
	cleanup(g1, g2);
	exit(EXIT_SUCCESS);
}

void task1(int *counter)
{
	while (*counter < 5)
	{
		printf("task1 count: %d\n", *counter);
		(*counter)++;
		sleep(1);
	}

	//pthread_exit((void *)100);
	pthread_exit((int *)100);
}


void task2(int *counter)
{
	while (*counter < 5)
	{
		printf("task2 count: %d\n", *counter);
		(*counter)++;
	}
}

void cleanup(int counter1, int counter2)
{
	printf("total iterators: %d\n", counter1 + counter2);
}



/* #####################################################################

jinbo@fang:~/gitme/linux/programming/src$ gcc thrdcreat.c -lpthread -o thrdcreat.o
jinbo@fang:~/gitme/linux/programming/src$ ./thrdcreat.o 
I'm in main function   // Q1
task1 count: 0
task2 count: 0
task2 count: 1
task2 count: 2
total iterators: 0     // Q2
task2 count: 3
task2 count: 4
task1 count: 1
task1 count: 2

task1 count: 3
task1 count: 4
return val of task1 is 100
total iterators: 10

Question:
运行结果跟书里不一样，体现在：

源代码 Q1 处: 
先创建线程 thrd1 不是应该先进入函数 task1 中，然后打印 task1 count: 0 吗？为什么先打印的是 I'm in main function?

源代码 Q2 处: 
第一次执行 cleanup(g1, g2); 打印 total iterators: 0，而之前已经打印了 task1 count: 0(（此时g1 = 1），task2 count: 2(此时g2 = 3)，
那么执行完 cleanup(g1, g2) 应该等于 4，但运行结果却是 0？
 

*/


