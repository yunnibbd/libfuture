#ifndef __SLEEP_H__
#define __SLEEP_H__
#include "future.h"
#include "awaitable.h"
#include "scheduler.h"
#include <iostream>
#include <cinttypes>
#include <chrono>

/**
 * @brief 协程专用的睡眠功能。
 * @return [co_await] void
 * @throw 
 */
template <class Rep, class Period>
inline future_t<> operator co_await(std::chrono::duration<Rep, Period> dt_)
{
	awaitable_t<> awaitable;
	scheduler_t* sch = current_scheduler();
	uint64_t tmp = std::chrono::duration_cast<std::chrono::milliseconds>(dt_).count();
	sch->sleep_until(tmp + utils_t::get_cur_timestamp());
	return awaitable.get_future();
}
#endif
