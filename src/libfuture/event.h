#ifndef __EVENT_H__
#define __EVENT_H__
#include "awaitable.h"
#include "future.h"
#include "common.h"

namespace libfuture
{

	class buffer_t;
	class socket_t;

	LIBFUTURE_API inline future_t<> open_connection(socket_t* socket, const char* ip, unsigned short port)
	{
		awaitable_t<> awaitable;

		current_scheduler()->add_to_connect(socket, ip, port);

		return awaitable.get_future();
	}

	LIBFUTURE_API inline future_t<> open_accept(socket_t* socket)
	{
		awaitable_t<> awaitable;

		current_scheduler()->add_to_socketio(socket, EVENT_ACCEPT);

		return awaitable.get_future();
	}

	LIBFUTURE_API inline future_t<> buffer_read(buffer_t* buffer, socket_t* socket)
	{
		awaitable_t<> awaitable;

		socket->set_recv_buf(buffer);
		current_scheduler()->add_to_socketio(socket, EVENT_RECV);

		return awaitable.get_future();
	}

	LIBFUTURE_API inline future_t<> buffer_write(buffer_t* buffer, socket_t* socket)
	{
		awaitable_t<> awaitable;

		socket->set_send_buf(buffer);
		current_scheduler()->add_to_socketio(socket, EVENT_SEND);

		return awaitable.get_future();
	}

}

#endif
