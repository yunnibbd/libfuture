#include "future.h"
#include "utils.h"
#include "awaitable.h"
#include "scheduler.h"
#include <iostream>
#include <cinttypes>
#include <chrono>

namespace libfuture
{

	/**
	 * @brief 与sleep功能一样
	 * @return [co_await] void
	 * @throw
	 */
	template <class Rep, class Period>
	LIBFUTURE_API inline future_t<> operator co_await(std::chrono::duration<Rep, Period> dt_)
	{
		awaitable_t<> awaitable;
		scheduler_impl_t* sch = current_scheduler();
		uint64_t tmp = std::chrono::duration_cast<std::chrono::milliseconds>(dt_).count();
		sch->sleep_until(tmp + utils_t::get_cur_timestamp());
		return awaitable.get_future();
	}

	/**
	 * @brief 协程专用的睡眠功能。
	 * @return [co_await] void
	 * @throw
	 */
	template <class Rep, class Period>
	LIBFUTURE_API inline future_t<> sleep(std::chrono::duration<Rep, Period> dt_)
	{
		awaitable_t<> awaitable;
		scheduler_impl_t* sch = current_scheduler();
		uint64_t tmp = std::chrono::duration_cast<std::chrono::milliseconds>(dt_).count();
		sch->sleep_until(tmp + utils_t::get_cur_timestamp());
		return awaitable.get_future();
	}

}
