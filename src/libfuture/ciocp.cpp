#include "ciocp.h"
#include "clog.h"

#ifdef _WIN32

CIOCP::CIOCP()
{

}

CIOCP::~CIOCP()
{
	Destory();
}

//将AcceptEX函数加载到内存中
bool CIOCP::LoadFunc(int ListenSocket)
{
	if (SOCK_NOT_INIT != listen_sock_)
	{
		LOG_WARNING("loadAcceptEX listen_sock_ != INVALID_SOCKET\n");
		return false;
	}
	if (_AcceptEx)
	{
		LOG_WARNING("loadAcceptEX _AcceptEx != NULL\n");
		return false;
	}
	listen_sock_ = ListenSocket;
	GUID GuidAcceptEx = WSAID_ACCEPTEX;
	DWORD dwBytes = 0;
	int iResult = WSAIoctl(ListenSocket, SIO_GET_EXTENSION_FUNCTION_POINTER,
		&GuidAcceptEx, sizeof(GuidAcceptEx),
		&_AcceptEx, sizeof(_AcceptEx),
		&dwBytes, NULL, NULL);
	if (iResult == SOCKET_ERROR) 
	{
		LOG_WARNING("WSAIoctl failed with error: %u\n", WSAGetLastError());
		return false;
	}
	GUID guidGetAcceptExSockaddrs = WSAID_GETACCEPTEXSOCKADDRS;
	iResult = WSAIoctl(ListenSocket, SIO_GET_EXTENSION_FUNCTION_POINTER,
		&guidGetAcceptExSockaddrs,
		sizeof(guidGetAcceptExSockaddrs),
		&_lpfnGetAcceptExSockaddrs,
		sizeof(_lpfnGetAcceptExSockaddrs),
		&dwBytes, NULL, NULL);
	if (iResult == SOCKET_ERROR) 
	{
		LOG_WARNING("WSAIoctl failed with error: %u\n", WSAGetLastError());
		return false;
	}
	return true;
}

static int addr_size = sizeof(sockaddr_in);

//获得近端地址
void CIOCP::GetAddr(IO_DATA_BASE *pIoData, sockaddr_in *l_addr, int l_len, sockaddr_in *r_addr, int r_len)
{
	_lpfnGetAcceptExSockaddrs(pIoData->wsaBuff.buf,
		0,
		addr_size, addr_size,
		(struct sockaddr**)&l_addr, &l_len,
		(struct sockaddr**)&r_addr, &r_len);
}

//创建一个IO完成端口(IOCP)
int CIOCP::Create()
{
	completion_port_ = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	if (NULL == completion_port_)
	{
		LOG_WARNING("CIOCP create failed with error %d\n", GetLastError());
		return -1;
	}
	return 0;
}

//销毁IOCP
void CIOCP::Destory()
{
	if (completion_port_)
	{
		CloseHandle(completion_port_);
		completion_port_ = NULL;
	}
}

//关联文件sockfd和IOCP
HANDLE CIOCP::Reg(int sockfd)
{
	auto ret = CreateIoCompletionPort((HANDLE)sockfd, completion_port_, (ULONG_PTR)sockfd, 0);
	if (NULL == ret)
	{
		LOG_WARNING("CIOCP reg sockfd failed with error %d\n", GetLastError());
		return NULL;
	}
	return ret;
}

//关联自定义数据地址和IOCP
HANDLE CIOCP::Reg(int sockfd, void* ptr)
{
	auto ret = CreateIoCompletionPort((HANDLE)sockfd, completion_port_, (ULONG_PTR)ptr, 0);
	if (NULL == ret)
	{
		LOG_WARNING("CIOCP reg ptr failed with error %d\n", GetLastError());
		return NULL;
	}
	return ret;
}

//投递接收链接任务
bool CIOCP::PostAccept(IO_DATA_BASE* pIoData)
{
	if (!_AcceptEx)
	{
		LOG_WARNING("postAccept _AcceptEX is NULL\n");
		return false;
	}
	pIoData->iotype = IO_TYPE::ACCEPT;
	pIoData->sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	/*
		! AcceptEx函数第四个参数只要是非0值，那么只有连接的客户端发送
			一条消息(大小随意)后，IOCP才会告诉程序这个客户端连接了
		? 为什么第三个参数这样写
			ack: 远端地址和本地地址都会存储在buffer尾部，
				而远端地址和本地地址的大小都是sizeof(sockaddr_in) + 16,
				剩下960字节可用
	*/
	if (FALSE == _AcceptEx(listen_sock_,
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

//投递接收数据任务
bool CIOCP::PostRecv(IO_DATA_BASE* pIoData)
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
			LOG_WARNING("CIOCP WSARecv failed with error %d\n", err);
			return false;
		}
	}
	return true;
}

//投递发送数据任务
bool CIOCP::PostSend(IO_DATA_BASE* pIoData)
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
			LOG_WARNING("CIOCP WSASend failed with error %d\n", err);
			return false;
		}
	}
	return true;
}

//获取所有任务的状态
int CIOCP::Wait(IO_EVENT& ioEvent, unsigned int timeout)
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
		{
			//超时
			return 0;
		}
		else if (ERROR_NETNAME_DELETED == err)
		{
			//客户端断开连接
			return 1;
		}
		else if (ERROR_CONNECTION_ABORTED == err)
		{
			//主动断开客户端连接
			return 1;
		}
		LOG_WARNING(" CIOCP GetQueuedCompletionStatus falied with error %d\n", err);
		return -1;
	}
	return 1;
}

#endif //#ifdef _WIN32
