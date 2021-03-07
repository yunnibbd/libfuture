#include "scheduler_private_api.h"
#include "scheduler.h"
#include <cinttypes>
using namespace libfuture;

static scheduler_t* g_scheduler = current_scheduler();

/**
 * @brief 添加进socketio队列
 * @param socket 要通信的socket
 * @param type 本socket要进行的操作类型
 * @param timeout 超时时间戳
 * @return
 */
void scheduler_private_api::add_to_socketio(socket_t* socket, event_type_enum type, uint64_t timeout)
{
	g_scheduler->add_to_socketio(socket, type, timeout);
}

/**
 * @brief 添加一个需要等待到某一时刻运行的协程
 * @param msec 要等待的时间
 * @return
 */
void scheduler_private_api::sleep_until(uint64_t msec)
{
	g_scheduler->sleep_until(msec);
}

/**
 * @brief 添加进协程关系依赖队列
 * @param handle 要等待别的协程的协程
 * @param dependent 被依赖的协程
 * @return
 */
void scheduler_private_api::add_to_depend(handle_type handle, handle_type dependent)
{
	g_scheduler->add_to_depend(handle, dependent);
}

/**
 * @brief 添加进挂起队列
 * @param handle 调用co_yield的协程句柄
 * @return
 */
void scheduler_private_api::add_to_suspend(handle_type handle)
{
	g_scheduler->add_to_suspend(handle);
}
