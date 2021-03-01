#include "libfuture.h"
#include <iostream>
#include <sstream>
#include <string>
using namespace std;
using namespace libfuture;

future_t<> test_send_and_recv(socket_t* client_socket, string addr)
{
	buffer_t buffer;
	while (true)
	{
		buffer.clear();
		co_await buffer_read(&buffer, client_socket);

		if (buffer.has_data())
		{
			//防止烫烫烫烫烫烫烫烫烫烫烫烫烫或屯屯屯屯屯屯屯屯屯屯屯屯屯屯
			buffer.data()[buffer.data_len()] = 0;
			cout << "recv from " << addr << ":" << buffer.data() << endl;
			co_await buffer_write(&buffer, client_socket);
		}
		else
		{
			cout << "client leave" << endl;
			delete client_socket;
			co_return;
		}
	}
	co_return;
}

future_t<> test_accept()
{
	socket_t* client_socket = nullptr;
	while (true)
	{
		client_socket = new socket_t();
		//在接收到客户端之前会一直挂起
		co_await open_accept(client_socket);
		//获得地址信息
		sockaddr_in* client_addr = current_scheduler()->get_accept_addr();
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
