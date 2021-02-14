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

	static scheduler_t* get_scheduler()
	{
		static scheduler_t instance;
		return &instance;
	}

	~scheduler_t()
	{
		destory_scheduler();
	}

	void destory_scheduler()
	{
		for (auto& handle : handle_queue_)
			handle.destroy();
		handle_queue_.clear();
		for (auto& entry : sleep_queue_)
			entry.second.destroy();
		sleep_queue_.clear();
	}

	template <typename _Ty>
	void ensure_future(_Ty &&future)
	{
		handle_queue_.insert(future.handle());
	}

	void sleep_until(uint64_t msec)
	{
		sleep_queue_.insert(std::make_pair(msec, current_handle()));
	}
	
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
	

	void set_current_handle(handle_type handle)
	{
		current_handle_ = handle;
	}

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

inline scheduler_t* current_scheduler()
{
	return scheduler_t::get_scheduler();
}

#endif
