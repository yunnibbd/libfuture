﻿#include "libfuture.h"
#include <iostream>
using namespace std;
using namespace libfuture;

future_t<int> task2()
{
	cout << "task2 begin" << endl;
	co_await 2s;
	cout << "task2 end" << endl;
	co_return 666;
}

future_t<> task1()
{
	cout << "task1 begin" << endl;
	co_await 1s;
	int ret = co_await task2();
	cout << "co_await task2 ret = " << ret << endl;
	cout << "task1 end" << endl;
}

int main(int argc, char** argv)
{
	auto sche = current_scheduler();
	//使用睡眠功能要先进行init
	sche->init();
	//开启一个协程
	cpp task1();

	sche->run_until_no_task();
	return 0;
}
