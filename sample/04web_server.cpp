#include "libfuture.h"
#include <iostream>
using namespace std;

future_t<> test_send_and_recv(socket_t* client_socket)
{
	buffer_t* buffer = new buffer_t();
	while (true)
	{
		buffer->clear();
		co_await buffer_read(buffer, client_socket);

		//契峭面面面面面面面面面面面面面賜様様様様様様様様様様様様様様
		buffer->data()[buffer->data_len()] = 0;

		cout << "recv from client " << buffer->data() << endl;

		co_await buffer_write(buffer, client_socket);
	}
	co_return;
}

future_t<> test_accept()
{
	while (true)
	{
		socket_t* client_socket = new socket_t(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		co_await open_accept(client_socket);
		current_scheduler()->ensure_future(test_send_and_recv(client_socket));
	}

	co_return;
}

int main(int argc, char** argv)
{
	WSADATA _data;
	WSAStartup(MAKEWORD(2, 2), &_data);

	auto sche = current_scheduler();

	socket_t* listen_socket = new socket_t(AF_INET, SOCK_STREAM, 0);
	listen_socket->bind(8000, "127.0.0.1");
	listen_socket->listen(128);
	sche->set_init_sockfd(listen_socket->sockfd());
	sche->init();
	sche->ensure_future(test_accept());
	sche->run_until_no_task();

	WSACleanup();
	return 0;
}
