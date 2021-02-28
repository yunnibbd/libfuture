#ifndef __IOCP_H__
#define __IOCP_H__
#include "export_api.h"
#include "error_code.h"
#include "socket.h"
#ifdef _WIN32

#ifndef USE_IOCP
#define USE_IOCP
#endif

#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <Windows.h>
#include <WinSock2.h>
#include <MSWSock.h>

namespace libfuture
{

	enum IO_TYPE
	{
		ACCEPT = 10,
		CONNECT,
		RECV,
		SEND
	};

	struct IO_DATA_BASE
	{
		//重叠体
		OVERLAPPED overlapped;
		//
		int sockfd;
		//
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

	/**
	 * @brief 对iocp的封装
	 */
	class LIBFUTURE_API iocp_t
	{
	public:
		using AcceptExPtr = BOOL(WINAPI*)(SOCKET, SOCKET, PVOID, DWORD, DWORD, DWORD, LPDWORD, LPOVERLAPPED);
		using ConnectExPtr = BOOL(WINAPI*)(SOCKET, const struct sockaddr*, int, PVOID, DWORD, LPDWORD, LPOVERLAPPED);
		using GetAcceptExSockaddrsPtr = void(WINAPI*)(PVOID, DWORD, DWORD, DWORD, LPSOCKADDR*, LPINT, LPSOCKADDR*, LPINT);

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
		 * @brief 将需要使用的函数加载到内存中
		 * @param sockfd
		 * @return bool 是否加载成功
		 */
		bool load_func(int sockfd);

		/**
		 * @brief 获得地址
		 * @param src 数据缓冲
		 * @param l_addr 近端地址
		 * @param l_len 近端地址长度
		 * @param r_addr 远端地址
		 * @param r_len 远端地址长度
		 * @return
		 */
		static void get_addr(char* src, sockaddr_in* l_addr, int l_len, sockaddr_in* r_addr, int r_len);

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
		 * @param listen_socket 监听套接字
		 * @param client_socket 客户端套接字
		 * @return bool 是否投递任务成功
		 */
		bool post_accept(IO_DATA_BASE* pIoData, int listen_socket, int client_socket);

		/**
		 * @brief 投递连接服务端任务
		 * @param pIoData 数据缓冲区
		 * @param sockfd 要连接的套接字
		 * @param addr 地址信息
		 * @param adde_len 地址长度
		 * @return bool 是否投递任务成功
		 */
		bool post_connect(IO_DATA_BASE* pIoData, int sockfd, sockaddr* addr, int addr_len);

		/**
		 * @brief 投递接收数据任务
		 * @param pIoData 数据缓冲区
		 * @return bool 是否投递任务成功
		 */
		bool post_recv(IO_DATA_BASE* pIoData, int sockfd);

		/**
		 * @brief 投递发送数据任务
		 * @param pIoData 数据缓冲区
		 * @return bool 是否投递任务成功
		 */
		bool post_send(IO_DATA_BASE* pIoData, int sockfd);

		/**
		 * @brief 获取所有任务的状态
		 * @param ioEvent 本次的任务类型
		 * @param timeout 本次等待的最大事件 毫秒
		 * @return int 返回等待的结果
		 */
		int wait(IO_EVENT& ioEvent, unsigned long timeout = INFINITE);
	private:
		//接收客户端函数指针
		static AcceptExPtr s_acceptEx;
		//连接服务端函数指针
		static ConnectExPtr s_connectEx;
		//获得地址函数指针
		static GetAcceptExSockaddrsPtr s_getAcceptExSockaddrs;
		//IOCP完成端口
		HANDLE completion_port_ = NULL;
	};

}

#endif //#ifdef _WIN32

#endif //#ifndef _iocp_t_H_
