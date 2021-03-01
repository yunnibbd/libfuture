#include "socket.h"
#include "error_code.h"
#include "utils.h"
#include <cstring>
using namespace libfuture;

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

int socket_t::bind(unsigned short port, const char* ip)
{
	sockaddr_in serv;
	memset(&serv, 0, sizeof(serv));
	serv.sin_family = AF_INET;
	serv.sin_port = htons(port);
	if (ip)
		serv.sin_addr.s_addr = inet_addr(ip);
	return this->bind((sockaddr * ) & serv, sizeof(serv));
}

int socket_t::listen(int backlog)
{
	return ::listen(sockfd_, backlog);
}

//设置端口可复用
bool socket_t::reuse_addr()
{
	int flag = 1;
	if (setsockopt(sockfd_, SOL_SOCKET, SO_REUSEADDR, (const char*)&flag, sizeof(flag)))
		return false;
	
	return true;
}

int socket_t::accept()
{
	sockaddr_in sockAddr;
	socklen_t len = sizeof(sockAddr);
	memset(&sockAddr, 0, sizeof(sockAddr));
	
#ifdef _WIN32
	int connfd = ::accept(sockfd_, (sockaddr*)&sockAddr, &len);
#else
	int connfd = accept4(sockfd_, (sockaddr*)&sockAddr, &len, SOCK_NONBLOCK);
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
	return ::closesocket(sockfd_);
#else
	return ::close(sockfd_);
#endif
}

int socket_t::recv(char* buf, const int size)
{
	return ::recv(sockfd_, buf, size, 0);
}

int socket_t::send(const char* buf, const int size)
{
	return ::send(sockfd_, buf, size, 0);
}

int socket_t::set_non_block()
{
#ifdef _WIN32
	u_long mode = 1;
	int ret = ioctlsocket(sockfd_, FIONBIO, &mode);

#else
	int oldFlags = fcntl(sockfd_, F_GETFL);
	int ret = fcntl(sockfd_, F_SETFL, oldFlags | O_NONBLOCK);

#endif
	return ret;
}
