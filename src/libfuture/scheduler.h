#ifndef __SCHEDULE_H__
#define __SCHEDULE_H__
#include "future.h"
#include "utils.h"
#include <list>
#include <coroutine>
#include <iostream>
#include <map>
#include <set>
#include <cinttypes>
#include <cstddef>
#include <chrono>

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
		static scheduler_t instance;
		return &instance;
	}

	/**
	 * @brief 析构时销毁所有协程句柄
	 * @param
	 * @return
	 */
	~scheduler_t()
	{
		destory_scheduler();
	}

	/**
	 * @brief 销毁所有协程句柄
	 * @param
	 * @return
	 */
	void destory_scheduler()
	{
		for (auto& handle : ready_queue_)
			handle.destroy();
		ready_queue_.clear();
		for (auto& entry : sleep_queue_)
			entry.second.destroy();
		sleep_queue_.clear();
		for (auto& entry: depend_queue_)
			entry.second.destroy();
		depend_queue_.clear();
	}

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
	 * @brief 添加协程关系依赖队列
	 * @param handle 要等待别的协程的协程
	 * @param dependent 被依赖的协程
	 * @return
	 */
	void add_to_suspend(handle_type handle, handle_type dependent)
	{
		depend_queue_.insert(std::make_pair(handle, dependent));
	}

	/**
	 * @brief 添加一个需要等待到某一时刻运行的协程
	 * @param msec 要等待的时间
	 * @return
	 */
	void sleep_until(uint64_t msec)
	{
		sleep_queue_.insert(std::make_pair(msec, current_handle()));
	}

	/**
	 * @brief 开始处理所有协程，直至处理完毕
	 * @param
	 * @return
	 */
	void run_until_no_task()
	{
		bool sleep_queue_empty = false;
		bool ready_queue_empty = false;
		bool depend_queue_empty = false;
		while (true)
		{
			if (update_sleep_queue())
				sleep_queue_empty = true;
			else
				sleep_queue_empty = false;

			if (update_ready_queue())
				ready_queue_empty = true;
			else
				ready_queue_empty = false;

			if (update_depend_queue())
				depend_queue_empty = true;
			else
				depend_queue_empty = false;

			if (sleep_queue_empty && ready_queue_empty && depend_queue_empty)
				break;
		}
	}
	
	/**
	 * @brief 存储scheduler当前要执行的协程句柄
	 * @param handle 要存储的协程句柄
	 * @return
	 */
	void set_current_handle(handle_type handle)
	{
		current_handle_ = handle;
	}

	/**
	 * @brief 获得当前正在执行的协程句柄
	 * @param
	 * @return handle_type 协程句柄
	 */
	handle_type current_handle()
	{
		return current_handle_;
	}
private:
	scheduler_t() {}
	
	/**
	 * @brief 调度休眠队列
	 * @param
	 * @return bool sleep_queue_是否为空
	 */
	bool update_sleep_queue()
	{
		if (sleep_queue_.empty())
			return true;
		auto begin = sleep_queue_.begin();
		auto end = sleep_queue_.end();
		auto cur_ms = utils_t::get_cur_timestamp();
		for (; begin != end; )
		{
			if (begin->first <= cur_ms)
			{
				auto handle = begin->second;
				if (!handle.done())
				{
					set_current_handle(handle);
					handle.resume();
				}
				begin = sleep_queue_.erase(begin);
			}
			break;
		}
		return false;
	}

	/**
	 * @brief 调度预备队列
	 * @param
	 * @return bool ready_queue是否为空
	 */
	bool update_ready_queue()
	{
		if (ready_queue_.empty())
			return true;
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
			} while (0);
			begin = ready_queue_.erase(begin);
		}
		return false;
	}

	/**
	 * @brief 调度依赖队列
	 * @param
	 * @return bool depend_queue是否为空
	 */
	bool update_depend_queue()
	{
		if (depend_queue_.empty())
			return true;
		auto begin = depend_queue_.begin();
		auto end = depend_queue_.end();
		for (; begin != end; )
		{
			if (begin->second.done())
			{
				begin->first.resume();
				if (begin->first.done())
				{
					begin = depend_queue_.erase(begin);
					continue;
				}
			}
			++begin;
		}

		return false;
	}

	//调度器正在执行的协程句柄
	handle_type current_handle_;
	//预备队列
	std::set<handle_type> ready_queue_;
	//依赖队列
	std::multimap<handle_type, handle_type> depend_queue_;
	//休眠队列
	std::multimap<uint64_t, handle_type> sleep_queue_;
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
