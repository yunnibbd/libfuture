#include "libfuture.h"
#include <iostream>
using namespace std;

future_t<int> task2()
{
	cout << "task2" << endl;
	co_return 666;
}

future_t<> task1()
{
	cout << "task1 begin" << endl;
	int ret = co_await task2();
	cout << "co_await task2 ret = " << ret << endl;
	cout << "task1 end" << endl;
}

int main(int argc, char** argv)
{
	auto sche = current_scheduler();

	sche->ensure_future(task1());

	sche->run_until_no_task();
	return 0;
}
