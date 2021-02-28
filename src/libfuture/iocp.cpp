#include "iocp.h"
#include "clog.h"
#include <cinttypes>
using namespace libfuture;

#ifdef _WIN32

iocp_t::AcceptExPtr iocp_t::s_acceptEx = NULL;
iocp_t::ConnectExPtr iocp_t::s_connectEx = NULL;
iocp_t::GetAcceptExSockaddrsPtr iocp_t::s_getAcceptExSockaddrs = NULL;

/**
 * @brief 构造
 * @param
 * @return
 */
iocp_t::iocp_t()
{
	
}

/**
 * @brief 析构,在之中调用销毁iocp
 * @param
 * @return
 */
iocp_t::~iocp_t()
{
	destory();
}

/**
 * @brief 将需要使用的函数加载到内存中
 * @param sockfd
 * @return bool 是否加载成功
 */
bool iocp_t::load_func(int sockfd)
{
	if (!s_acceptEx || !s_connectEx || !s_getAcceptExSockaddrs)
	{
		GUID acceptex = WSAID_ACCEPTEX;
		GUID connectex = WSAID_CONNECTEX;
		GUID getacceptexsockaddrs = WSAID_GETACCEPTEXSOCKADDRS;

		if (INVALID_SOCKET == sockfd)
			return false;

		DWORD bytes = 0;

		if (0 != WSAIoctl(sockfd, SIO_GET_EXTENSION_FUNCTION_POINTER, &acceptex, sizeof(acceptex),
			&s_acceptEx, sizeof(s_acceptEx), &bytes, NULL, NULL))
			return false;

		if (0 != WSAIoctl(sockfd, SIO_GET_EXTENSION_FUNCTION_POINTER, &connectex, sizeof(connectex),
			&s_connectEx, sizeof(s_connectEx), &bytes, NULL, NULL))
			return false;

		if (0 != WSAIoctl(sockfd, SIO_GET_EXTENSION_FUNCTION_POINTER, &getacceptexsockaddrs, sizeof(getacceptexsockaddrs),
			&s_getAcceptExSockaddrs, sizeof(s_getAcceptExSockaddrs), &bytes, NULL, NULL))
			return false;
	}
	return true;
}

static int addr_size = sizeof(sockaddr_in);

/**
 * @brief 获得地址
 * @param src 数据缓冲
 * @param l_addr 近端地址
 * @param l_len 近端地址长度
 * @param r_addr 远端地址
 * @param r_len 远端地址长度
 * @return
 */
void iocp_t::get_addr(char *src, sockaddr_in *l_addr, int l_len, sockaddr_in *r_addr, int r_len)
{
	if (!s_getAcceptExSockaddrs)
		return;
	s_getAcceptExSockaddrs(src,
					       0,
						   addr_size + 16, addr_size + 16,
						   (struct sockaddr**)&l_addr, &l_len,
						   (struct sockaddr**)&r_addr, &r_len);
}

/**
 * @brief 创建一个IO完成端口
 * @param
 * @return int 成功返回完成端口句柄,失败返回-1
 */
int iocp_t::create()
{
	completion_port_ = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	if (NULL == completion_port_)
	{
		LOG_WARNING("iocp_t create failed with error %d\n", GetLastError());
		return -1;
	}
	return 0;
}

/**
 * @brief 销毁IOCP
 * @param
 * @return
 */
void iocp_t::destory()
{
	if (completion_port_)
	{
		CloseHandle(completion_port_);
		completion_port_ = NULL;
	}
}

/**
 * @brief 关联文件sockfd和IOCP
 * @param sockfd 待关联的套接字
 * @return HANDLE 返回句柄
 */
HANDLE iocp_t::reg(int sockfd)
{
	auto ret = CreateIoCompletionPort((HANDLE)sockfd, completion_port_, (ULONG_PTR)sockfd, 0);
	if (NULL == ret)
	{
		LOG_WARNING("iocp_t reg sockfd failed with error %d\n", GetLastError());
		return NULL;
	}
	return ret;
}

/**
 * @brief 关联自定义数据地址和IOCP
 * @param sockfd 待关联的套接字
 * @param ptr 待关联的指针
 * @return HANDLE 返回句柄
 */
HANDLE iocp_t::reg(int sockfd, void* ptr)
{
	auto ret = CreateIoCompletionPort((HANDLE)sockfd, completion_port_, (ULONG_PTR)ptr, 0);
	if (NULL == ret)
	{
		LOG_WARNING("iocp_t reg ptr failed with error %d\n", GetLastError());
		return NULL;
	}
	return ret;
}

/**
 * @brief 投递接收链接任务
 * @param pIoData 数据缓冲区
 * @param listen_socket 监听套接字
 * @param client_socket 客户端套接字
 * @return bool 是否投递任务成功
 */
bool iocp_t::post_accept(IO_DATA_BASE* pIoData, int listen_socket, int client_socket)
{
	if (!s_acceptEx)
	{
		LOG_WARNING("postAccept _AcceptEX is NULL\n");
		return false;
	}
	pIoData->iotype = IO_TYPE::ACCEPT;
	pIoData->sockfd = client_socket;
	/*
		! AcceptEx函数第四个参数只要是非0值，那么只有连接的客户端发送
			一条消息(大小随意)后，IOCP才会告诉程序这个客户端连接了
		? 为什么第三个参数这样写
			ack: 远端地址和本地地址都会存储在buffer尾部，
			s	而远端地址和本地地址的大小都是sizeof(sockaddr_in) + 16,
				剩下960字节可用
	*/
	if (FALSE == s_acceptEx(
		listen_socket,
		pIoData->sockfd,
		pIoData->wsaBuff.buf,
		//sizeof(ioData.buffer) - (sizeof(sockaddr_in) + 16) * 2,
		0,
		sizeof(sockaddr_in) + 16,
		sizeof(sockaddr_in) + 16,
		NULL,
		&pIoData->overlapped))
	{
		int errCode = WSAGetLastError();
		//ERROR_IO_PENDING:操作还在进行，没有出错
		if (errCode != ERROR_IO_PENDING)
		{
			/*
				在打开了超过接收上限的连接再关闭后，再关掉一个现有连接，
					此时再投递接收数据，会失败并报错以下错误
					WSAECONNRESET                    10054L
			*/
			if (WSAECONNRESET == errCode)
			{
				//远端关闭
				return false;
			}
			LOG_WARNING("acceptexRet falied with error %d\n", errCode);
			return false;
		}
	}
	return true;
}

/**
 * @brief 投递连接服务端任务
 * @param pIoData 数据缓冲区
 * @param sockfd 要连接的套接字
 * @param addr 地址信息
 * @param adde_len 地址长度
 * @return bool 是否投递任务成功
 */
bool iocp_t::post_connect(IO_DATA_BASE* pIoData, int sockfd, sockaddr* addr, int addr_len)
{
	pIoData->iotype = IO_TYPE::CONNECT;
	if (FALSE == s_connectEx(sockfd, (PSOCKADDR)addr, addr_len,
		pIoData->wsaBuff.buf, 1, NULL, &pIoData->overlapped))
	{
		int errCode = WSAGetLastError();
		if (errCode != ERROR_IO_PENDING)
		{
			LOG_WARNING("post_connect falied with error %d\n", errCode);
			return false;
		}
	}
	return true;
}

/**
 * @brief 投递接收数据任务
 * @param pIoData 数据缓冲区
 * @param 数据接收源
 * @return bool 是否投递任务成功
 */
bool iocp_t::post_recv(IO_DATA_BASE* pIoData, int sockfd)
{
	pIoData->iotype = IO_TYPE::RECV;
	DWORD flags = 0;
	ZeroMemory(&pIoData->overlapped, sizeof(OVERLAPPED));

	if (SOCKET_ERROR == WSARecv(pIoData->sockfd, &pIoData->wsaBuff, 1, NULL, &flags, &pIoData->overlapped, NULL))
	{
		int err = WSAGetLastError();
		if (ERROR_IO_PENDING != err)
		{
			if (WSAECONNABORTED == err)
			{
				//websocket直接刷新网页会导致这个错误
				return false;
			}
			if (WSAECONNRESET == err)
			{
				//远端客户端关闭
				return false;
			}
			if (WSAENOTSOCK == err)
			{
				//socket已经被关闭, 这里是心跳检测主动服务端主动关的
				return false;
			}
			LOG_WARNING("iocp_t WSARecv failed with error %d\n", err);
			return false;
		}
	}
	return true;
}

/**
 * @brief 投递发送数据任务
 * @param pIoData 数据缓冲区
 * @param sockfd 数据发送目的地
 * @return bool 是否投递任务成功
 */
bool iocp_t::post_send(IO_DATA_BASE* pIoData, int sockfd)
{
	pIoData->iotype = IO_TYPE::SEND;
	DWORD flags = 0;
	ZeroMemory(&pIoData->overlapped, sizeof(OVERLAPPED));

	if (SOCKET_ERROR == WSASend(pIoData->sockfd, &pIoData->wsaBuff, 1, 0, flags, &pIoData->overlapped, NULL))
	{
		int err = WSAGetLastError();
		if (ERROR_IO_PENDING != err)
		{
			if (WSAECONNRESET == err)
			{
				//远端关闭
				return false;
			}
			if (WSAENOTSOCK == err)
			{
				//socket已经被关闭, 这里是心跳检测主动服务端主动关的
				return false;
			}
			LOG_WARNING("iocp_t WSASend failed with error %d\n", err);
			return false;
		}
	}
	return true;
}

/**
 * @brief 获取所有任务的状态
 * @param ioEvent 本次的任务类型
 * @param timeout 本次等待的最大事件 毫秒
 * @return int 返回等待的结果
 */
int iocp_t::wait(IO_EVENT& ioEvent, unsigned long timeout)
{
	ioEvent.bytesTrans = 0;
	ioEvent.pIOData = NULL;
	ioEvent.data.ptr = NULL;
	//最后参数如果传INFINITE会等到有事件完成才返回(阻塞)
	if (FALSE == GetQueuedCompletionStatus(completion_port_,
		&ioEvent.bytesTrans,
		(PULONG_PTR)& ioEvent.data,
		(LPOVERLAPPED*)&ioEvent.pIOData,
		timeout))
	{
		int err = GetLastError();
		if (WAIT_TIMEOUT == err)
			//超时
			return 0;
		else if (ERROR_NETNAME_DELETED == err)
			//客户端断开连接
			return 1;
		else if (ERROR_CONNECTION_ABORTED == err)
			//主动断开客户端连接
			return 1;
		LOG_WARNING(" iocp_t GetQueuedCompletionStatus falied with error %d\n", err);
		return -1;
	}
	return 1;
}

#endif //#ifdef _WIN32
