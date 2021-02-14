#ifndef __SCHEDULE_H__
#define __SCHEDULE_H__
#include "future.h"
#include "utils.h"
#include <list>
#include <coroutine>
#include <iostream>
#include <map>
#include <set>
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
		for (auto& handle : handle_queue_)
			handle.destroy();
		handle_queue_.clear();
		for (auto& entry : sleep_queue_)
			entry.second.destroy();
		sleep_queue_.clear();
	}

	/**
	 * @brief 添加一个协程进入队列
	 * @param future 待添加的协程
	 * @return
	 */
	template <typename _Ty>
	void ensure_future(_Ty &&future)
	{
		handle_queue_.insert(future.handle());
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
		while (!sleep_queue_.empty() || !handle_queue_.empty())
		{
			do
			{
				if (sleep_queue_.empty())
					break;
				auto begin = sleep_queue_.begin();
				auto end = sleep_queue_.end();
				auto cur_ms = utils_t::get_cur_timestamp();
				for (; begin != end; )
				{
					if (begin->first <= cur_ms)
					{
						std::cout << "key1 = " << begin->first;
						std::cout << ",  key2 = " << cur_ms << std::endl;
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
			} while (0);

			auto begin = handle_queue_.begin();
			auto end = handle_queue_.end();
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
				begin = handle_queue_.erase(begin);
			}
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

	handle_type current_handle_;
	std::multiset<handle_type> handle_queue_;
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
