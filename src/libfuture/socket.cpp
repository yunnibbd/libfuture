#include "socket.h"
#include "error_code.h"
#include "utils.h"

socket_t::~socket_t()
{
	close();
}

socket_t::socket_t(int af, int type, int protocol)
{
	sockfd_ = socket(af, type, protocol);
}

int socket_t::bind(const sockaddr* sa, int salen)
{
	if (!sa)
		return -1;
	if (0 != ::bind(sockfd_, sa, salen))
		return -1;
	return 0;
}

int socket_t::listen(int backlog)
{
	return ::listen(sockfd_, backlog);
}

int socket_t::accept()
{
	sockaddr_in sockAddr;
	socklen_t len = sizeof(sockAddr);
	memset(&sockAddr, 0, sizeof(sockAddr));

#ifdef _WIN32
	int connfd = ::accept(sockfd_, (sockaddr*)&sockAddr, &len);
#else
	int connfd = accept4(m_fd, (sockaddr*)&sockAddr, &len, SOCK_NONBLOCK);
#endif
	return connfd;
}

int socket_t::connect(int af, const char* strIP, const int nPort)
{
	sockaddr_in sockAddr;
	memset(&sockAddr, 0, sizeof(sockAddr));

	sockAddr.sin_family = af;
	sockAddr.sin_port = htons(nPort);
	inet_pton(af, strIP, &sockAddr.sin_addr);

	return this->connect((sockaddr*)&sockAddr, sizeof(sockAddr));
}

int socket_t::connect(sockaddr* sa, int salen)
{
	return ::connect(sockfd_, sa, salen);
}

int socket_t::close()
{
#ifdef _WIN32
	return closesocket(sockfd_);
#else
	return close(sockfd_);
#endif
}

int socket_t::recv(char* buf, const int size)
{
	return ::recv(sockfd_, buf, size, 0);
}

int socket_t::recv(IOVEC_TYPE* iov, int iovcnt)
{
	int transBytes = 0;
#ifdef _WIN32
	DWORD readBytes = 0;
	DWORD flags = 0;
	if (WSARecv(sockfd_, iov, iovcnt, &readBytes, &flags, NULL, NULL))
	{
		if (WSAGetLastError() != WSAECONNABORTED)
			transBytes = -1;
	}
	else
		transBytes = readBytes;
#else
	transBytes = readv(m_fd, iov, iovcnt);
#endif

	return transBytes;
}

int socket_t::send(const char* buf, const int size)
{
	return ::send(sockfd_, buf, size, 0);
}

int socket_t::send(IOVEC_TYPE* iov, int iovcnt)
{
	int transBytes = 0;
#ifdef _WIN32
	DWORD sendBytes = 0;
	if (WSASend(sockfd_, iov, iovcnt, &sendBytes, 0, NULL, NULL))
		transBytes = -1;
	else
		transBytes = sendBytes;
#else
	transBytes = writev(m_fd, iov, iovcnt);
#endif

	return transBytes;
}

socket_unread_t socket_t::get_unread_byte() const
{
	socket_unread_t n = 0;
#ifdef _WIN32
	if (ioctlsocket(sockfd_, FIONREAD, &n) < 0)
	{
		printf("call ioctlsocket failed! err:%d\n", WSAGetLastError());
		return 0;
	}
#else
	if (ioctl(sockfd_, FIONREAD, &n) < 0)
	{
		printf("call ioctlsocket failed!\n");
		return -1;
	}

#endif

	return n;
}

int socket_t::set_non_block()
{
#ifdef _WIN32
	u_long mode = 1;
	int ret = ioctlsocket(sockfd_, FIONBIO, &mode);

#else
	int oldFlags = fcntl(m_fd, F_GETFL);
	int ret = fcntl(sockfd_, F_SETFL, oldFlags | O_NONBLOCK);

#endif
	return ret;
}

int socket_t::close_on_exec()
{
#ifndef _WIN32
	if (fcntl(sockfd_, F_SETFD, FD_CLOEXEC) == -1)
		return -1;
#endif
	return 0;
}