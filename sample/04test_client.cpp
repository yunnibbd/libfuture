#include "libfuture.h"
#include <string>
#include <iostream>
using namespace std;
using namespace libfuture;
#define BUF_LEN 10240

string send_str = "\
GET / HTTP/1.1\r\n\
Host: 42.192.165.127\r\n\
Connection: keep-alive\r\n\r\n";

future_t<> test_connect(const char* ip, unsigned short port)
{
	//空间要大
	buffer_t buffer(BUF_LEN + 1);
	socket_t client_socket(AF_INET, SOCK_STREAM, 0);

	bool has_c = co_await open_connection(&client_socket, ip, port);
	if (!has_c)
	{
		cout << "连接失败" << endl;
		co_return;
	}

	cout << "连接成功" << endl;

	buffer.push(send_str.c_str(), send_str.size());

	bool is_timeout = co_await buffer_write(&buffer, &client_socket, 5s);

	if (is_timeout)
	{
		cout << "超时未发送" << endl;
		co_return;
	}
	cout << "发送消息成功" << endl;

	buffer.clear();

	//看看回了什么消息
	is_timeout = co_await buffer_read(&buffer, &client_socket, 5s);

	if (is_timeout)
	{
		cout << "超时未读取到消息" << endl;
		co_return;
	}

	if (buffer.has_data())
	{
		//防止烫烫或屯屯
		int len = buffer.data_len();
		if (len >= BUF_LEN)
			len = BUF_LEN;
		buffer.data()[len] = 0;
		cout << buffer.data() << endl;
	}

	co_return;
}

int main(int argc, char** argv)
{
#ifdef _WIN32
	WSADATA data;
	WSAStartup(MAKEWORD(2, 2), &data);
#endif
	auto sche = current_scheduler();
	sche->init();

	for (int i = 0; i < 10; ++i)
		cpp test_connect("42.192.165.127", 80);

	sche->run_until_no_task();

#ifdef _WIN32
	WSACleanup();
#endif

	return 0;
}
