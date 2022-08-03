#include <malloc.h>
#include <string.h>
#include <thread>
#include <iostream>
#include <vector>
#include <string>
#include <assert.h>
struct NODE
{
	int ID;
	char Name[40];
	int age;
	NODE *prev;
	NODE *next;
};
struct NODE *node_head = NULL;
int member_id = 0;
void add_member()
{
	struct NODE *new_node = (NODE *)malloc(sizeof(NODE));
	new_node->next = NULL;
	NODE *prev_node = node_head->prev;
	if (prev_node)
	{
		prev_node->next = new_node;
		new_node->prev = prev_node;
		node_head->prev = new_node;
	}
	else
	{
		node_head->next = new_node;
		new_node->prev = node_head;
		node_head->prev = new_node;
	}
	new_node->ID = member_id++;
	printf("请输入会员姓名,然后按回车\n");
	scanf("%s", new_node->Name);
	printf("请输入会员年龄,然后按回车\n");
	scanf("%d", &new_node->age);

	printf("添加新会员成功\n");
}
class test_1
{
public:
	test_1()
	{
		x = 10;
		y = 100;
	}
	virtual ~test_1() {}
	virtual void test_fun()
	{
		printf("test_1 test_fun\n");
	}

private:
	int x;
	int y;
};
class test_2 : public test_1
{
public:
	test_2() {}
	virtual ~test_2() {}
	virtual void test_fun2()
	{
		printf("test_fun2\n");
	}
	virtual void test_fun()
	{
		printf("test_2 test_fun\n");
	}
};

void test_fun(int i)
{
	printf("i is %d\n", i);
}

void test_fun(const char *str)
{
	printf("str is %s\n", str);
}

void test_fun_x()
{
	printf("test fun x\n");
}

void test_loop()
{
	for (int i = 0; i < 1000; i++)
	{
		printf("i is %d\n", i);
	}
	printf("exit the loop\n");
}

void cond_fun_test(int a, const char *str)
{
	int x = a * a;
	printf("a is %d,x is %d,str is %s\n", a, x, str);
	x *= 2;
	printf("quit fun\n");
}

void print_arr_test()
{
	int iarr[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
	const char *strarr[] = {"this", "is", "a", "test", "string"};
	for (unsigned long i = 0; i < sizeof(iarr) / sizeof(int); i++)
	{
		printf("%d ", iarr[i]);
	}
	for (int i = 0; i < 5; i++)
	{
		printf("%s ", strarr[i]);
	}
	printf("arr test done\n");
}

class test_3
{
	int x;
	int y;
	void test()
	{
	}
};

struct TEST_NODE
{
	char gender[3];
	int ID;
	char name[7];
};

void test_memory()
{
	const char *str = "test";
	int number = 0x12345678;
	TEST_NODE *node = new TEST_NODE;
	node->ID = 100;
	strcpy(node->gender, "M");
	strcpy(node->name, "Jack");

	printf("str is %s,number is %d, node id is %d, %s, %s, test end\n", str, number, node->ID, node->gender, node->name);
	delete node;
}

int call_fun_test_2(int level, const char *str)
{
	int number = 102;
	const char *name = "call_fun_test_2";
	printf("level is %d,str is %s,name is %s\n", level, str, name);
	return 2;
}

int call_fun_test_1(int level, const char *str)
{
	int number = 101;
	const char *name = "call_fun_test_1";
	printf("level is %d,str is %s,name is %s\n", level, str, name);
	call_fun_test_2(level + 1, "call_fun_test_2");
	return 1;
}

int count = 0;

void do_work(void *arg)
{
	std::cout << "thread id = " << std::this_thread::get_id() << " start" << std::endl;
	//模拟做一些事情
	int local_data = count;
	count++;
	std::this_thread::sleep_for(std::chrono::seconds(3));
	std::cout << "thread id = " << std::this_thread::get_id() << " finish" << std::endl;
}

int start_threads(int thread_num)
{
	std::vector<std::thread> threads;
	//启动 thread_num 个线程
	for (int i = 0; i < thread_num; ++i)
	{
		threads.push_back(std::thread(do_work, &i));
		std::cout << "启动新线程：" << i << std::endl;
	}
	//等待所有线程结束
	for (auto &thread : threads)
	{
		thread.join();
	}

	return 0;
}
void test_throw(int number)
{
	int local_data = number;
	const char *name = "test throw";
	printf("name is %s,%d\n", name, local_data);
	throw "test exception";
}
void test_try_catch(int number)
{
	int local_data = number;
	const char *name = "test_try_catch";
	printf("name is %s,%d\n", name, local_data);
	try
	{
		int throw_num = 50;
		printf("throw\n");
		throw 10;
	}
	catch (...) // 捕获任何异常
	{
		int catch_num = 100;
		printf("catch ...\n");
	}
}

void test_jump(int cnt)
{
	while (true)
	{
		if (cnt == 100)
		{
			printf("in if, cnt = %d\n", cnt);
			break;
		}
		else
		{
			printf("in else, cnt = %d\n", cnt);
			++cnt;
		}
	}
}

int main(int argc, char *argv[])
{
	//test_jump(0);
	test_1 *t1 = NULL;
	test_3 t3;
	test_try_catch(10);
	start_threads(3);
	int number = 100;
	const char *name = "main";
	call_fun_test_1(1, "call_fun_test_1");
	//test_memory();
	// print_arr_test();
	//assert(t1 != NULL);
	// cond_fun_test(10, "test");
	// test_loop();

	// for (int i = 0; i < 10; i++)
	// {
	// 	printf("execute test_fun_x: i = %d\n", i);
	// 	test_fun_x();
	// }

	test_fun(10);
	test_fun("test");
	test_1 *test = new test_1();
	test->test_fun();
	test_1 *test2 = new test_2();
	test2->test_fun();
	test_2 test3;
	// printf("传入的参数信息为:\n");
	// for (int i = 0; i < argc; i++)
	// {
	// 	printf("参数 %d=%s\n", i, argv[i]);
	// }
	// node_head = (struct NODE *)malloc(sizeof(NODE));
	// node_head->next = node_head->prev = NULL;
	// printf("会员管理系统\n1:录入会员信息\nq:退出\n");
	// while (true)
	// {
	// 	switch (getchar())
	// 	{
	// 	case '1':
	// 		add_member();
	// 		break;
	// 	case 'q':
	// 		return 0;
	// 	default:
	// 		break;
	// 	}
	// }
	return 0;
}
栈