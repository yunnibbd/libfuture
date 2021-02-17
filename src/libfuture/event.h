#ifndef __EVENT_H__
#define __EVENT_H__
#include "awaitable.h"
#include "future.h"
#include "socket.h"

class buffer_t;
class socket_t;

future_t<> open_connection(socket_t *socket, const char* ip, unsigned short port)
{
	awaitable_t<> awaitable;
	
	

	return awaitable.get_future();
}

future_t<> open_accept(socket_t *socket, int listen_socket)
{
	awaitable_t<> awaitable;



	return awaitable.get_future();
}

future_t<> buffer_read(buffer_t *buffer, socket_t *socket)
{
	awaitable_t<> awaitable;

	socket->set_recv_buf(buffer);

	return awaitable.get_future();
}

future_t<> buffer_write(buffer_t* buffer, socket_t* socket)
{
	awaitable_t awaitable;

	socket->set_send_buf(buffer);

	return awaitable.get_future();
}

#endif
