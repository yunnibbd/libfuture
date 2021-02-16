﻿#ifndef _iocp_t_H_
#define _iocp_t_H_
#include "export_api.h"

#ifdef _WIN32

#ifndef USE_IOCP
#define USE_IOCP
#endif

#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <Windows.h>
#include <WinSock2.h>
#include <MSWSock.h>
#include "error_code.h"

//对iocp的封装

enum IO_TYPE
{
	ACCEPT = 10,
	RECV,
	SEND
};

//数据缓冲区大小
#define DATA_BUFF_SIZE 1024

struct IO_DATA_BASE
{
	//重叠体
	OVERLAPPED overlapped;
	//
	int sockfd;
	/*
		数据缓冲区
		? 为什么要这样一个结构存储数据，数据指针和记录指向数据的大小
		!: 每个客户端都有自己的接收和发送缓冲区，那么在定义多余的空间就有点浪费，
			所以只要一个指针指向接收或者发送缓冲区数据具体的位置就可以，
			IOCP在接收数据完成以后就会通知接收或发送数据了，而不是跟
			select和epoll一样通知可以接收/发送才告诉程序，IOCP是接收完/发送完
			才会告知程序
	*/
	WSABUF wsaBuff;
	//操作类型
	IO_TYPE iotype;
};

struct IO_EVENT
{
	union
	{
		void* ptr;
		int sockfd;
	} data;
	IO_DATA_BASE* pIOData;
	DWORD bytesTrans = 0;
};

class LIBFUTURE_API iocp_t
{
public:
	/**
	 * @brief 构造
	 * @param
	 * @return
	 */
	iocp_t();

	/**
	 * @brief 析构,在之中调用销毁iocp
	 * @param
	 * @return
	 */
	~iocp_t();

	/**
	 * @brief 将_AcceptE和_lpfnGetAcceptExSockaddrs函数加载到内存中
	 * @param Listenint 关联的监听sockfd
	 * @return bool 时候加载成功
	 */
	bool load_func(int Listenint);
	
	/**
	 * @brief 获得近端地址
	 * @param pIoData IO_DATA_BASE数据缓冲
	 * @param l_addr 近端地址
	 * @param l_len 近端地址长度
	 * @param r_addr 远端地址
	 * @param r_len 远端地址长度
	 * @return
	 */
	void get_addr(IO_DATA_BASE *pIoData, sockaddr_in *l_addr, int l_len, sockaddr_in *r_addr, int r_len);

	/**
	 * @brief 创建一个IO完成端口
	 * @param
	 * @return int 成功返回完成端口句柄,失败返回-1
	 */
	int create();

	/**
	 * @brief 销毁IOCP
	 * @param
	 * @return
	 */
	void destory();

	/**
	 * @brief 关联文件sockfd和IOCP
	 * @param sockfd 待关联的套接字
	 * @return HANDLE 返回句柄
	 */
	HANDLE reg(int sockfd);

	/**
	 * @brief 关联自定义数据地址和IOCP
	 * @param sockfd 待关联的套接字
	 * @param ptr 待关联的指针
	 * @return HANDLE 返回句柄
	 */
	HANDLE reg(int sockfd, void* ptr);

	/**
	 * @brief 投递接收链接任务
	 * @param pIoData 数据缓冲区
	 * @return bool 是否投递任务成功
	 */
	bool post_accept(IO_DATA_BASE* pIoData);

	/**
	 * @brief 投递接收数据任务
	 * @param pIoData 数据缓冲区
	 * @return bool 是否投递任务成功
	 */
	bool post_recv(IO_DATA_BASE* pIoData);

	/**
	 * @brief 投递发送数据任务
	 * @param pIoData 数据缓冲区
	 * @return bool 是否投递任务成功
	 */
	bool post_send(IO_DATA_BASE* pIoData);

	/**
	 * @brief 获取所有任务的状态
	 * @param ioEvent 本次的任务类型
	 * @param timeout 本次等待的最大事件 毫秒
	 * @return int 返回等待的结果
	 */
	int wait(IO_EVENT& ioEvent, unsigned int timeout = INFINITE);
private:
	//接收客户端函数指针
	LPFN_ACCEPTEX _AcceptEx = NULL;
	//获得地址函数指针
	LPFN_GETACCEPTEXSOCKADDRS _lpfnGetAcceptExSockaddrs;
	//IOCP完成端口
	HANDLE completion_port_ = NULL;
	//监听的套接字
	int listen_sock_ = SOCK_NOT_INIT;
};

#endif //#ifdef _WIN32

#endif //#ifndef _iocp_t_H_
