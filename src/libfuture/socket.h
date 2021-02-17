#ifndef __SOCKET_H__
#define __SOCKET_H__
#include "export_api.h"
#include "noncopyable.h"
#include "include.h"
#define BUFFER_LEN 1024

class buffer_t;

class socket_t : public noncopyable
{
public:
	~socket_t();

	explicit socket_t(int sockfd) :
		sockfd_(sockfd){}

	socket_t(int af, int type, int protocol);

	int bind(const sockaddr* sa, int salen);

	int listen(int backlog);

	int accept();

	int connect(int af, const char* strIP, const int nPort);

	int connect(sockaddr* sa, int salen);

	int close();

	int recv(char* buf, const int size);

	int recv(IOVEC_TYPE* iov, int iovcnt);

	int send(const char* buf, const int size);

	int send(IOVEC_TYPE* iov, int iovcnt);

	int sockfd() const { return sockfd_; }

	//缓冲区中的待接收数据大小
	socket_unread_t get_unread_byte() const;

	int set_non_block();

	int close_on_exec();

	void set_send_buf(buffer_t* buffer) { p_send_buf_ = buffer;  }
	buffer_t* p_send_buf() { return p_send_buf_; }

	void set_recv_buf(buffer_t* buffer) { p_recv_buf_ = buffer; }
	buffer_t* p_recv_buf() { return p_recv_buf_; }
private:
	int sockfd_;
	buffer_t* p_send_buf_ = nullptr;
	buffer_t* p_recv_buf_ = nullptr;
	char accept_buffer_[BUFFER_LEN] = { 0 };
};

#endif
