#ifndef __SCHEDULE_H__
#define __SCHEDULE_H__
#include "common.h"
#include "iocp.h"
#include <list>
#include <coroutine>
#include <iostream>
#include <map>
#include <set>
#include <cinttypes>
#include <cstddef>
#include <chrono>

class socket_t;

class scheduler_t
{
public:

	using handle_type = std::coroutine_handle<>;

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
	 * @brief 析构时销毁所有协程句柄
	 * @param
	 * @return
	 */
	~scheduler_t();

	/**
	 * @brief 初始化
	 * @param
	 * @return
	 */
	void init();

	/**
	 * @brief 销毁所有协程句柄
	 * @param
	 * @return
	 */
	void destory_scheduler();

	/**
	 * @brief 添加一个协程进入队列
	 * @param future 待添加的协程
	 * @return
	 */
	template <typename _Ty>
	void ensure_future(_Ty &&future)
	{
		ready_queue_.insert(future.handle());
	}

	/**
	 * @brief 添加进协程关系依赖队列
	 * @param handle 要等待别的协程的协程
	 * @param dependent 被依赖的协程
	 * @return
	 */
	void add_to_depend(handle_type handle, handle_type dependent);

	/**
	 * @brief 添加进挂起队列
	 * @param handle 调用co_yield的协程句柄
	 * @return
	 */
	void add_to_suspend(handle_type handle);

	/**
	 * @brief 添加进socketio队列
	 * @param socket 要通信的socket
	 * @param type 本socket要进行的操作类型
	 * @return
	 */
	void add_to_socketio(socket_t* socket, event_type_enum type);

	/*
	 * @brief 添加进accept队列
	 * @param socket 要通信的socket
	 * @return
	 */
	/*void add_to_accept(socket_t* socket)
	{
		g_io_data.wsaBuff.buf = g_accept_buffer;
		g_io_data.wsaBuff.len = ACCEPT_BUFFER_LEN;
		iocp_.post_accept(&g_io_data, socket->sockfd());
		accept_socket_queue_.insert(std::make_pair(socket, current_handle()));
	}*/
	
	/**
	 * @brief 添加一个需要等待到某一时刻运行的协程
	 * @param msec 要等待的时间
	 * @return
	 */
	void sleep_until(uint64_t msec);

	/**
	 * @brief 开始处理所有协程，直至处理完毕
	 * @param
	 * @return
	 */
	void run_until_no_task();
	
	/**
	 * @brief 存储scheduler当前要执行的协程句柄
	 * @param handle 要存储的协程句柄
	 * @return
	 */
	void set_current_handle(handle_type handle);

	/**
	 * @brief 获得当前正在执行的协程句柄
	 * @param
	 * @return handle_type 协程句柄
	 */
	handle_type current_handle();

	void set_listen_sockfd(int sockfd) { listen_socket_ = sockfd; }

private:
	/**
	 * @brief 构造
	 * @param
	 * @return
	 */
	scheduler_t();
	
	/**
	 * @brief 调度休眠队列
	 * @param
	 * @return bool sleep_queue_是否为空
	 */
	void update_sleep_queue();

	/**
	 * @brief 调度socketio_queue_
	 * @param
	 * @return bool 所有任务是否处理完毕
	 */
	bool update_socketio_queue();

	/**
	 * @brief 调度预备队列
	 * @param
	 * @return
	 */
	void update_ready_queue();

	/**
	 * @brief 调度依赖队列
	 * @param
	 * @return
	 */
	void update_depend_queue();
	
	/**
	 * @brief 调度挂起队列
	 * @param
	 * @return bool 挂起队列是否为空
	 */
	void update_suspend_queue();
	
	//本对象单件对象指针
	static scheduler_t *signal_instance_;

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

	//调度器正在执行的协程句柄
	handle_type current_handle_;
	//预备队列
	std::set<handle_type> ready_queue_;
	//挂起队列
	std::set<handle_type> suspend_queue_;
	//依赖队列
	std::multimap<handle_type, handle_type> depend_queue_;
	//socket收发消息队列
	std::map<int, handle_type> socketio_queue_;
	//休眠队列
	std::multimap<uint64_t, handle_type> sleep_queue_;
	//休眠队列中的协程
	std::set<handle_type> in_sleep_queue_;
	//关于socket通信的对象
	iocp_t iocp_;
	//用于监听的套接字
	int listen_socket_ = INVALID_SOCKET;
	//用于接收新客户端的数据指针
	IO_DATA_BASE io_data_ = { 0 };
	//用于接收新客户端的缓冲区
	char buffer_[512] = { 0 };
};

/**
 * @brief 获得全局scheduler指针
 * @param
 * @return scheduler_t*
 */
inline scheduler_t* current_scheduler()
{
	return scheduler_t::get_scheduler();
}

#endif
