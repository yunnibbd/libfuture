#ifndef __SCHEDULER_H__
#define __SCHEDULER_H__
#include "iocp.h"
#include "epoll.h"
#include "socket.h"
#include "export_api.h"
#include "base_scheduler.h"

namespace libfuture
{
	/**
	 * @brief 调度器，调度io
	 */
	class LIBFUTURE_API scheduler_t : public scheduler_impl_t
	{
	public:
		/**
		 * @brief 获得scheduler单件对象
		 * @param
		 * @return scheduler* scheduler单例对象指针
		 */
		static scheduler_t* get_scheduler()
		{
			//本对象单件对象指针
			static scheduler_t signal_instance_;
			return &signal_instance_;
		}

		virtual ~scheduler_t();

		/**
		 * @brief 添加进socketio队列
		 * @param socket 要通信的socket
		 * @param type 本socket要进行的操作类型
		 * @param timeout 超时时间戳
		 */
		virtual void add_to_socketio(socket_t* socket, event_type_enum type, uint64_t timeout = 0) override;

		/**
		 * @brief 调度socketio_queue_
		 * @param
		 * @return bool 所有任务是否处理完毕
		 */
		virtual bool update_socketio_queue() override;

		/**
		 * @brief 初始化
		 * @param
		 * @return
		 */
		void init();

		void set_init_sockfd(int sockfd) { init_socket_ = sockfd; }

		sockaddr_in* get_accept_addr() { return &client_addr_; }

	private:
#ifdef _WIN32
		/**
		 * @brief iocp的处理io事件方式
		 * @param
		 * @return bool 为update_socketio_queue的返回值
		 */
		bool iocp_loop();
#else
		/**
		 * @brief epoll的处理io事件方式
		 * @param
		 * @return bool 为update_socketio_queue的返回值
		 */
		bool epoll_loop();
#endif

		/**
		 * @brief io事件在超时时间内触发，取消超时
		 * @param socket
		 * @return
		 */
		void cancel_to_timeout(socket_t* socket);

		//用于accept的时候的客户端地址
		struct sockaddr_in client_addr_ = { 0 };
		//用于accept的时候的客户端地址长度
		socklen_t client_addr_len_ = sizeof(client_addr_);
#ifdef _WIN32
		//用于初始化的套接字
		int init_socket_ = INVALID_SOCKET;
		//关于socket的通信对象
		iocp_t iocp_;
		//用于接收新客户端的数据指针
		IO_DATA_BASE io_data_ = { 0 };
		//用于接收新客户端的缓冲区
		char buffer_[512] = { 0 };
#else
		//用于初始化的套接字
		int init_socket_ = -1;
		//关于socket的通信对象
		epoll_t epoll_;
#endif
	};

	/**
	 * @brief 获得全局scheduler指针
	 * @param
	 * @return scheduler_impl_t*
	 */
	LIBFUTURE_API inline scheduler_t* current_scheduler()
	{
		return scheduler_t::get_scheduler();
	}

}

#endif//__SCHEDULER_H__
