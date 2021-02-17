#ifndef __SOCKET_H__
#define __SOCKET_H__
#include "export_api.h"
#include "noncopyable.h"
#include "include.h"

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

private:
	int sockfd_;
};

#endif
