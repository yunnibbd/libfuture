#include "libfuture.h"
#include <iostream>
using namespace std;

future_t<> test3()
{
	cout << "test3 begin" << endl;
	co_await 10s;
	cout << "test3 end" << endl;
	co_return;
}

future_t<void> test2()
{
	cout << "test2 begin" << endl;
	//co_await test3();
	cout << "test 2 end" << endl;
	co_return;
}

future_t<int> test1()
{
	cout << "test1 begin" << endl;
	co_await test2();
	co_await 1s;
	cout << "test1 end" << endl;
	co_return 666;
}

future_t<int> test() 
{
	//cout << "当前正在执行" << current_scheduler()->current_handle().address() << endl;
	cout << "test begin" << endl;
	//int ret = co_await test1();
	co_await 10s;
	cout << "test end" << endl;
	//cout << "test end ret = " << ret << endl;

	/*cout << "test before co_yield" << endl;
	co_yield 1;
	cout << "test after co_yield" << endl;*/
	co_return 3;
}

int main(int argc, char** argv) 
{
	auto sche = current_scheduler();
	sche->ensure_future(test());
	/*sche->ensure_future(test1());
	sche->ensure_future(test2());
	sche->ensure_future(test3());*/
	sche->run_until_no_task();
	return 0;
}
