#ifndef __EVENT_H__
#define __EVENT_H__
#include "awaitable.h"
#include "future.h"
#include "common.h"

namespace libfuture
{

	class buffer_t;
	class socket_t;

	/**
	 * @brief 异步连接
	 * @param socket 
	 * @param ip 要连接的ip地址
	 * @param port 要连接的端口
	 * @return
	 */
	LIBFUTURE_API inline future_t<bool> open_connection(socket_t* socket, const char* ip, unsigned short port);

	/**
	 * @brief 接收一个连接
	 * @param socket 
	 * @return 协程对象
	 */
	LIBFUTURE_API inline future_t<> open_accept(socket_t* socket);

	/**
	 * @brief 读数据
	 * @param buffer 数据的存放点
	 * @param socket 往哪里读
	 * @return 协程对象
	 */
	LIBFUTURE_API inline future_t<> buffer_read(buffer_t* buffer, socket_t* socket);

	/**
	 * @brief 写数据
	 * @param buffer 数据源
	 * @param socket 往哪里写
	 * @return 协程对象
	 */
	LIBFUTURE_API inline future_t<> buffer_write(buffer_t* buffer, socket_t* socket);
}

#include "event.inl"

#endif
