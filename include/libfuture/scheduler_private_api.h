#ifndef __SCHEDULER_PRIVATE_API_H__
#define __SCHEDULER_PRIVATE_API_H__
#include "common.h"
#include "export_api.h"
#include <cinttypes>
#include <coroutine>

namespace libfuture
{
	class socket_t;

	/**
	 * @brief 避免直接访问scheduler私有成员的友元类
	 */
	class LIBFUTURE_API scheduler_private_api
	{
	public:
		using handle_type = std::coroutine_handle<>;

		/**
		 * @brief 添加进socketio队列
		 * @param socket 要通信的socket
		 * @param type 本socket要进行的操作类型
		 * @param timeout 超时时间戳
		 * @return
		 */
		static void add_to_socketio(socket_t* socket, event_type_enum type, uint64_t timeout = 0);

		/**
		 * @brief 添加一个需要等待到某一时刻运行的协程
		 * @param msec 要等待的时间
		 * @return
		 */
		static void sleep_until(uint64_t msec);

		/**
		 * @brief 添加进协程关系依赖队列
		 * @param handle 要等待别的协程的协程
		 * @param dependent 被依赖的协程
		 * @return
		 */
		static void add_to_depend(handle_type handle, handle_type dependent);

		/**
		 * @brief 添加进挂起队列
		 * @param handle 调用co_yield的协程句柄
		 * @return
		 */
		static void add_to_suspend(handle_type handle);
	};
}

#endif
