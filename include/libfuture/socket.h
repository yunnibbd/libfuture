#ifndef __SOCKET_H__
#define __SOCKET_H__
#include "export_api.h"
#include "noncopyable.h"
#include "include.h"
#include "common.h"

namespace libfuture
{

	class buffer_t;

	/**
	 * @brief 对socket的封装
	 */
	class LIBFUTURE_API socket_t : public noncopyable
	{
	public:
		socket_t() {}

		explicit socket_t(int sockfd) :
			sockfd_(sockfd) {}

		~socket_t();

		socket_t(int af, int type, int protocol);

		int bind(const sockaddr* sa, int salen);

		int bind(unsigned short port, const char* ip = nullptr);

		int listen(int backlog);

		//设置端口可复用
		bool reuse_addr();

		int accept();

		int connect(int af, const char* strIP, const int nPort);

		int connect(sockaddr* sa, int salen);

		int close();

		int recv(char* buf, const int size);

		int send(const char* buf, const int size);

		void set_sockfd(int sockfd) { sockfd_ = sockfd; }

		int sockfd() const { return sockfd_; }

		int set_non_block();

		void set_send_buf(buffer_t* buffer) { p_send_buf_ = buffer; }
		buffer_t* p_send_buf() { return p_send_buf_; }

		void set_recv_buf(buffer_t* buffer) { p_recv_buf_ = buffer; }
		buffer_t* p_recv_buf() { return p_recv_buf_; }

		//是否读写超时，作为判断用
		bool is_timeout = false;

#ifdef _WIN32
		//是否被注册
		bool is_register = false;
		//是否投递了事件
		bool is_post_event = false;
#endif

		void set_event_type(event_type_enum event_type) { event_type_ = event_type; }
		event_type_enum event_type() { return event_type_; }
	private:
		//注册的时候的事件(初始为未知事件)
		event_type_enum event_type_ = EVENT_UNKNOW;
		int sockfd_ = -1;
		buffer_t* p_send_buf_ = nullptr;
		buffer_t* p_recv_buf_ = nullptr;
	};
}

#endif
