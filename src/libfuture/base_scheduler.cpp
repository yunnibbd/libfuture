﻿#include "base_scheduler.h"
#include "future.h"
#include "utils.h"
#include "iocp.h"
#include "socket.h"
#include "event.h"
#include "clog.h"
#include "buffer.h"
#include "common.h"
using namespace libfuture;

/**
	* @brief 构造
	* @param
	* @return
	*/
scheduler_impl_t::scheduler_impl_t()
{

}

/**
	* @brief 析构时销毁所有协程句柄
	* @param
	* @return
	*/
scheduler_impl_t::~scheduler_impl_t()
{
	destory_scheduler();
}

/**
	* @brief 销毁所有协程句柄
	* @param
	* @return
	*/
void scheduler_impl_t::destory_scheduler()
{
	for (auto& handle : ready_queue_)
		handle.destroy();
	ready_queue_.clear();
	for (auto& entry : sleep_queue_)
		entry.second.destroy();
	sleep_queue_.clear();
	for (auto& entry : depend_queue_)
		entry.second.destroy();
	depend_queue_.clear();
	for (auto& handle : suspend_queue_)
		handle.destroy();
	suspend_queue_.clear();
	for (auto& entry : socketio_queue_)
		entry.second.destroy();
	socketio_queue_.clear();
}

/**
	* @brief 添加进协程关系依赖队列
	* @param handle 要等待别的协程的协程
	* @param dependent 被依赖的协程
	* @return
	*/
void scheduler_impl_t::add_to_depend(handle_type handle, handle_type dependent)
{
	depend_queue_.insert(std::make_pair(handle, dependent));
}

/**
	* @brief 添加进挂起队列
	* @param handle 调用co_yield的协程句柄
	* @return
	*/
void scheduler_impl_t::add_to_suspend(handle_type handle)
{
	suspend_queue_.insert(handle);
}

/**
	* @brief 添加一个需要等待到某一时刻运行的协程
	* @param msec 要等待的时间
	* @return
	*/
void scheduler_impl_t::sleep_until(uint64_t msec)
{
	auto handle = current_handle();
	sleep_queue_.insert(std::make_pair(msec, handle));
	in_sleep_queue_.insert(handle);
}

/**
	* @brief 开始处理所有协程，直至处理完毕
	* @param
	* @return
	*/
void scheduler_impl_t::run_until_no_task()
{
	while (true)
	{
		update_ready_queue();
		update_sleep_queue();
		update_depend_queue();
		update_suspend_queue();
		if (update_socketio_queue())
			break;
	}
}

/**
	* @brief 存储scheduler当前要执行的协程句柄
	* @param handle 要存储的协程句柄
	* @return
	*/
void scheduler_impl_t::set_current_handle(handle_type handle)
{
	current_handle_ = handle;
}

/**
	* @brief 获得当前正在执行的协程句柄
	* @param
	* @return handle_type 协程句柄
	*/
scheduler_impl_t::handle_type scheduler_impl_t::current_handle()
{
	return current_handle_;
}

/**
	* @brief 调度休眠队列
	* @param
	* @return bool sleep_queue_是否为空
	*/
void scheduler_impl_t::update_sleep_queue()
{
	auto begin = sleep_queue_.begin();
	auto end = sleep_queue_.end();
	auto cur_ms = utils_t::get_cur_timestamp();
	for (; begin != end; )
	{
		if (begin->first <= cur_ms)
		{
			do
			{
				auto handle = begin->second;
				if (handle.done())
					break;
				auto start = depend_queue_.lower_bound(handle);
				auto stop = depend_queue_.upper_bound(handle);
				bool can_trigger = true;
				for (; start != stop; )
				{
					if (!start->second.done())
					{
						can_trigger = false;
						break;
					}
					++start;
				}
				if (can_trigger)
				{
					set_current_handle(handle);
					handle.resume();
				}
			} while (0);
			in_sleep_queue_.erase(begin->second);
			begin = sleep_queue_.erase(begin);
			continue;
		}
		break;
	}
}

/**
	* @brief 调度预备队列
	* @param
	* @return
	*/
void scheduler_impl_t::update_ready_queue()
{
	auto begin = ready_queue_.begin();
	auto end = ready_queue_.end();
	for (; begin != end; )
	{
		do
		{
			if (begin->done())
			{
				begin->destroy();
				break;
			}
			set_current_handle(*begin);
			begin->resume();
			if (begin->done())
				begin->destroy();
		} while (0);
		begin = ready_queue_.erase(begin);
	}
}

/**
	* @brief 调度依赖队列
	* @param
	* @return
	*/
void scheduler_impl_t::update_depend_queue()
{
	auto begin = depend_queue_.begin();
	auto end = depend_queue_.end();
	for (; begin != end; )
	{
		bool can_trigger = true;
		auto start = depend_queue_.lower_bound(begin->first);
		auto stop = depend_queue_.upper_bound(begin->first);
		for (; start != end; )
		{
			if (!start->second.done())
			{
				can_trigger = false;
				break;
			}
			++start;
		}
		if (can_trigger)
		{
			if (!begin->first.done())
			{
				//查询当前协程是否在sleep_queue_中
				if (in_sleep_queue_.find(begin->first) == in_sleep_queue_.end())
				{
					set_current_handle(begin->first);
					begin->first.resume();
				}
			}
			if (begin->first.done())
			{
				auto start = depend_queue_.lower_bound(begin->first);
				auto stop = depend_queue_.upper_bound(begin->first);
				for (; start != stop; )
				{
					//到这里代表被依赖项都执行完毕了,销毁协程句柄
					start->second.destroy();
					++start;
				}
				begin->first.destroy();
				begin = depend_queue_.erase(begin);
				continue;
			}
		}
		begin = stop;
	}
}

/**
	* @brief 调度挂起队列
	* @param
	* @return bool 挂起队列是否为空
	*/
void scheduler_impl_t::update_suspend_queue()
{
	auto begin = suspend_queue_.begin();
	auto end = suspend_queue_.end();
	for (; begin != end; )
	{
		if (!begin->done())
		{
			ready_queue_.insert(*begin);
			begin = suspend_queue_.erase(begin);
			continue;
		}
		++begin;
	}
}
