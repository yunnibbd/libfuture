#include "awaitable.h"
#include "future.h"
#include "promise.h"
#include "sleep.h"
#include "common.h"

namespace libfuture
{

	/**
	 * @brief 异步连接
	 * @param socket
	 * @param ip 要连接的ip地址
	 * @param port 要连接的端口
	 * @return
	 */
	LIBFUTURE_API inline future_t<bool> open_connection(socket_t* socket, const char* ip, unsigned short port)
	{
		socket->set_non_block();
		int sockfd = socket->sockfd();
		sockaddr_in sin;
		memset(&sin, 0, sizeof(sin));
		sin.sin_family = AF_INET;
		sin.sin_port = htons(port);
		sin.sin_addr.s_addr = inet_addr(ip);
		int ret = -1;
		std::chrono::seconds sleep_sec(1);
		while (true)
		{
			ret = ::connect(sockfd, (sockaddr*)&sin, sizeof(sin));
			if (ret == 0)
				co_return true;
#ifdef _WIN32
			int err_code = GetLastError();
			if (err_code == WSAEISCONN)
				co_return true;
			else
			{
				if (err_code != WSAEWOULDBLOCK)
					co_return false;
			}
			fd_set e_set;
			FD_ZERO(&e_set);
			FD_SET(sockfd, &e_set);
			timeval tm = { 0, 0 };
			int sret = select(0, nullptr, &e_set, nullptr, &tm);
			if (sret == 1)
			{
				if (FD_ISSET(sockfd, &e_set))
					co_return true;
				else
					co_return false;
			}
#else
			int err_code = errno;
			if (errno == EINTR)
				//connect 动作被信号中断，重试connect
				continue;
			else if (errno == EINPROGRESS)
			{
				//连接正在尝试中
				fd_set e_set;
				FD_ZERO(&e_set);
				FD_SET(sockfd, &e_set);
				timeval tm = { 0, 0 };
				int sret = select(0, nullptr, &e_set, nullptr, &tm);
				if (sret == 1)
				{
					int err;
					socklen_t len = static_cast<socklen_t>(sizeof err);
					getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &err, &len);
					if (err == 0)
						co_return true;
					co_return false;
				}
			}
			else
				//真的出错了，
				co_return false;
#endif
			std::cout << "co_await open_connection" << std::endl;
			co_await sleep_sec;
		}
	}


	/**
	 * @brief 接收一个连接
	 * @param socket
	 * @return 协程对象
	 */
	LIBFUTURE_API inline future_t<> open_accept_raw(socket_t* socket)
	{
		awaitable_t<> awaitable;

		current_scheduler()->add_to_socketio(socket, EVENT_ACCEPT);

		return awaitable.get_future();
	}

	/**
	 * @brief 接收一个连接
	 * @param socket
	 * @return 客户端地址指针
	 */
	LIBFUTURE_API inline future_t<sockaddr_in*> open_accept(socket_t* socket)
	{
		co_await open_accept_raw(socket);
		sockaddr_in* client_addr = current_scheduler()->get_accept_addr();
		co_return client_addr;
	}

	/**
	 * @brief 读数据，不建议直接调用，使用buffer_read
	 * @param buffer 数据的存放点
	 * @param socket 往哪里读
	 * @param timeout 超时时间
	 * @return 协程对象
	 */
	template <class Rep, class Period>
	LIBFUTURE_API inline future_t<> buffer_read_raw(buffer_t* buffer, socket_t* socket, std::chrono::duration<Rep, Period> timeout)
	{
		awaitable_t<> awaitable;

		socket->set_recv_buf(buffer);
		auto sche = current_scheduler();
		int64_t timeout_msec = std::chrono::duration_cast<std::chrono::milliseconds>(timeout).count() +
			utils_t::get_cur_timestamp();
		sche->add_to_socketio(socket, EVENT_RECV, timeout_msec);
		sche->sleep_until(timeout_msec);

		return awaitable.get_future();
	}

	/**
	 * @brief 写数据，不建议直接调用，使用buffer_write
	 * @param buffer 数据源
	 * @param socket 往哪里写
	 * @param timeout 超时时间
	 * @return 协程对象
	 */
	template <class Rep, class Period>
	LIBFUTURE_API inline future_t<> buffer_write_raw(buffer_t* buffer, socket_t* socket, std::chrono::duration<Rep, Period> timeout)
	{
		awaitable_t<> awaitable;

		socket->set_send_buf(buffer);
		auto sche = current_scheduler();
		int64_t timeout_msec = std::chrono::duration_cast<std::chrono::milliseconds>(timeout).count() +
			utils_t::get_cur_timestamp();
		sche->add_to_socketio(socket, EVENT_SEND, timeout_msec);
		sche->sleep_until(timeout_msec);

		return awaitable.get_future();
	}

	/**
	 * @brief 读数据
	 * @param buffer 数据的存放点
	 * @param socket 往哪里读
	 * @param timeout 超时时间
	 * @return bool 是否超时
	 */
	template <class Rep, class Period>
	LIBFUTURE_API inline future_t<bool> buffer_read(buffer_t* buffer, socket_t* socket, std::chrono::duration<Rep, Period> timeout)
	{
		co_await buffer_read_raw(buffer, socket, timeout);
		if (socket->is_timeout)
			co_return true;
		else
			co_return false;
	}

	/**
	 * @brief 写数据
	 * @param buffer 数据源
	 * @param socket 往哪里写
	 * @param timeout 超时时间
	 * @return bool 是否超时
	 */
	template <class Rep, class Period>
	LIBFUTURE_API inline future_t<bool> buffer_write(buffer_t* buffer, socket_t* socket, std::chrono::duration<Rep, Period> timeout)
	{
		co_await buffer_write_raw(buffer, socket, timeout);
		if (socket->is_timeout)
			co_return true;
		else
			co_return false;
	}
}
