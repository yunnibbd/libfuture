#include "libfuture.h"
#include <iostream>
using namespace std;
using namespace libfuture;

future_t<> test_accept_recv()
{
	socket_t client_socket = socket_t(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	//可以根据客户端地址打印出来，此处不打印
	sockaddr_in* client_addr = co_await open_accept(&client_socket);

	buffer_t buffer;

	while (true)
	{
		buffer.clear();
		bool is_timeout = co_await buffer_read(&buffer, &client_socket, 3s);
		if (is_timeout)
		{
			cout << "读取超时" << endl;
			co_return;
		}

		if (buffer.has_data())
		{
			//防止烫烫烫烫烫烫烫烫烫烫烫烫烫或屯屯屯屯屯屯屯屯屯屯屯屯屯屯
			buffer.data()[buffer.data_len()] = 0;
			cout << "recv from client " << buffer.data() << endl;
			//设置超时时间为2s
			bool is_timeout = co_await buffer_write(&buffer, &client_socket, 2000ms);
			if (is_timeout)
			{
				cout << "发送超时" << endl;
				co_return;
			}
		}
		else
		{
			cout << "client leave" << endl;
			co_return;
		}
	}

	co_return;
}

int main(int argc, char** argv)
{
#ifdef _WIN32
	WSADATA _data;
	WSAStartup(MAKEWORD(2, 2), &_data);
#endif

	auto sche = current_scheduler();

	socket_t* listen_socket = new socket_t(AF_INET, SOCK_STREAM, 0);
	listen_socket->bind(8000, "127.0.0.1");
	listen_socket->listen(128);
	sche->set_init_sockfd(listen_socket->sockfd());
	//要成为服务端必须使用一个监听套接字来初始化
	sche->init();

	//开启一个协程
	cpp test_accept_recv();

	sche->run_until_no_task();

#ifdef _WIN32
	WSACleanup();
#endif
	return 0;
}
