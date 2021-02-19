﻿#ifndef __SCHEDULER_H__
#define __SCHEDULER_H__
#include "base_scheduler.h"

class scheduler_t : public scheduler_impl_t
{
public:
	/**
	 * @brief 获得scheduler单件对象
	 * @param
	 * @return scheduler* scheduler单例对象指针
	 */
	static scheduler_t* get_scheduler()
	{
		if (!signal_instance_)
		{
			signal_instance_ = new scheduler_t();
			static free_scheduler_ptr free_ins;
		}
		return signal_instance_;
	}

	/**
	 * @brief 添加进socketio队列
	 * @param socket 要通信的socket
	 * @param type 本socket要进行的操作类型
	 * @return
	 */
	virtual void add_to_socketio(socket_t* socket, event_type_enum type) override;

	/*
	 * @brief 添加connect事件进入socketio队列
	 * @param socket 要通信的socket
	 * @param buffer 连接上时要发送的数据
	 * @param ip 要连接的ip地址
	 * @param port 要连接的端口
	 * @return
	 */
	virtual void add_to_connect(socket_t* socket, buffer_t* buffer, const char* ip, unsigned short port) override;

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

private:
	//本对象单件对象指针
	static scheduler_t* signal_instance_;

	class free_scheduler_ptr
	{
	public:
		~free_scheduler_ptr()
		{
			if (signal_instance_)
			{
				delete signal_instance_;
				signal_instance_ = nullptr;
			}
		}
	};

	//关于socket通信的对象
	iocp_t iocp_;
	//用于监听的套接字
	int init_socket_ = INVALID_SOCKET;
	//用于接收新客户端的数据指针
	IO_DATA_BASE io_data_ = { 0 };
	//用于接收新客户端的缓冲区
	char buffer_[512] = { 0 };
};

/**
 * @brief 获得全局scheduler指针
 * @param
 * @return scheduler_impl_t*
 */
inline scheduler_t* current_scheduler()
{
	return scheduler_t::get_scheduler();
}

#endif//__SCHEDULER_H__
