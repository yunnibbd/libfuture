#ifndef __BASE_SCHEDULE_H__
#define __BASE_SCHEDULE_H__
#include "common.h"
#include "iocp.h"
#include "export_api.h"
#include <list>
#include <coroutine>
#include <iostream>
#include <map>
#include <set>
#include <cinttypes>
#include <cstddef>
#include <chrono>

namespace libfuture
{

	class socket_t;

	/**
	 * @brief 调度器的基类 调度所有与io无关的代码
	 */
	class LIBFUTURE_API scheduler_impl_t
	{
	public:

		using handle_type = std::coroutine_handle<>;

		/**
		 * @brief 构造
		 * @param
		 * @return
		 */
		scheduler_impl_t();

		/**
		 * @brief 析构时销毁所有协程句柄
		 * @param
		 * @return
		 */
		virtual ~scheduler_impl_t();

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
		void ensure_future(_Ty&& future)
		{
			ready_queue_.insert(future.handle());
		}

		/**
		 * @brief 配合宏定义可直接使用cpp来加入一个协程,和ensure_future功能一样
		 * @param
		 * @return
		 */
		template <typename _Ty>
		void operator + (_Ty&& future)
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
		 * @param timeout 超时时间戳
		 * @return
		 */
		virtual void add_to_socketio(socket_t* socket, event_type_enum type, uint64_t timeout = 0) = 0;

		///*
		// * @brief 添加connect事件进入socketio队列
		// * @param socket 要通信的socket
		// * @param ip 要连接的ip地址
		// * @param port 要连接的端口
		// * @return
		// */
		//virtual void add_to_connect(socket_t* socket, const char* ip, unsigned short port) = 0;

		/**
		 * @brief 调度socketio_queue_
		 * @param
		 * @return bool 所有任务是否处理完毕
		 */
		virtual bool update_socketio_queue() = 0;

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

	protected:

		/**
		 * @brief 调度休眠队列
		 * @param
		 * @return bool sleep_queue_是否为空
		 */
		void update_sleep_queue();

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
		
		//调度器正在执行的协程句柄
		handle_type current_handle_;
		//预备队列
		std::set<handle_type> ready_queue_;
		//挂起队列
		std::set<handle_type> suspend_queue_;
		//依赖队列
		std::multimap<handle_type, handle_type> depend_queue_;
		//被依赖的协程句柄
		std::set<handle_type> in_depend_queue_second_;
		//socket收发消息队列
		std::map<int, handle_type> socketio_queue_;
		//为了增加socketio收发的超时，提供一个休眠队列和socket队列的关联
		//让socketio队列事件触发后可以找到休眠队列取消超时
		std::map<int, uint64_t> sockfd_sleep_queue_;
		//当休眠队列触发后可以找到sockio队列对应的事件从而取消
		std::map<uint64_t, socket_t*> sleep_socket_queue_;
		//休眠队列
		std::multimap<uint64_t, handle_type> sleep_queue_;
		//休眠队列中的协程
		std::set<handle_type> in_sleep_queue_;
	};

}

#endif
