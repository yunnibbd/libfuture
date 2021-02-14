#include "libfuture.h"
#include <iostream>
using namespace std;

future_t<> test3()
{
	cout << "test3" << endl;
	co_return;
}

future_t<void> test2()
{
	cout << "test2" << endl;
	co_return;
}

future_t<int> test1()
{
	cout << "test1" << endl;
	co_return 666;
}

future_t<int> test() 
{
	cout << "当前正在执行" << current_scheduler()->current_handle().address() << endl;
	co_await 1s;
	cout << "co_await 1s end" << endl;
	int ret = co_await test1();
	cout << "co_await test1 ret = " << ret << endl;
	co_return 2;
}

int main(int argc, char** argv) 
{
	auto sche = current_scheduler();
	sche->ensure_future(test());
	sche->ensure_future(test1());
	sche->ensure_future(test2());
	sche->ensure_future(test3());
	sche->run_until_no_task();
	return 0;
}
