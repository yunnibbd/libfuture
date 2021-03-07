#include "libfuture.h"
#include <iostream>
#include <sstream>
#include <string>
using namespace std;
using namespace libfuture;
#define BUF_LEN 10240

future_t<> test_send_and_recv(socket_t* client_socket, string addr)
{
	buffer_t buffer(BUF_LEN + 1);
	while (true)
	{
		buffer.clear();
		//超时时间为5秒
		bool is_timeout = co_await buffer_read(&buffer, client_socket, 5s);
		if (is_timeout)
		{
			cout << "读取超时" << endl;
			break;
		}

		if (buffer.has_data())
		{
			//防止烫烫烫烫烫烫烫烫烫烫烫烫烫或屯屯屯屯屯屯屯屯屯屯屯屯屯屯
			int len = buffer.data_len();
			if (len > BUF_LEN)
				len = BUF_LEN;
			buffer.data()[len] = 0;
			cout << "recv from " << addr << ":" << buffer.data() << endl;
			//超时时间为5秒
			bool is_timeout = co_await buffer_write(&buffer, client_socket, 5000ms);
			if (is_timeout)
			{
				cout << "发送超时" << endl;
				break;
			}
		}
		else
		{
			client_socket->close();
			break;
		}
	}
	cout << "client leave" << endl;
	delete client_socket;
	co_return;
}

future_t<> test_accept()
{
	socket_t* client_socket = nullptr;
	while (true)
	{
		client_socket = new socket_t();
		//在接收到客户端之前会一直挂起
		sockaddr_in* client_addr = co_await open_accept(client_socket);
		stringstream ss;
		ss << inet_ntoa(client_addr->sin_addr) << ":";
		ss << ntohs(client_addr->sin_port);
		cout << ss.str() << " join" << endl;
		//开启一个协程来处理这个socket的接收和发送数据
		cpp test_send_and_recv(client_socket, ss.str());
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
	listen_socket->reuse_addr();
	listen_socket->bind(8000, "127.0.0.1");
	listen_socket->listen(128);
	sche->set_init_sockfd(listen_socket->sockfd());
	//要成为一个服务端必须要设置一个监听套接字进行初始化
	sche->init();

	//开启一个协程
	cpp test_accept();

	sche->run_until_no_task();

#ifdef _WIN32
	WSACleanup();
#endif
	return 0;
}
