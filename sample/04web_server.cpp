#include "libfuture.h"
#include <iostream>
using namespace std;

future_t<> test_send_and_recv(socket_t* client_socket)
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
			cout << "recv from client " << buffer.data() << endl;
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
		co_await open_accept(client_socket);
		cpp test_send_and_recv(client_socket);
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
	sche->init();
	cpp test_accept();

	sche->run_until_no_task();

#ifdef _WIN32
	WSACleanup();
#endif
	return 0;
}
