#ifndef __SLEEP_H__
#define __SLEEP_H__
#include "future.h"

namespace libfuture
{

	/**
	 * @brief 与sleep功能一样
	 * @return [co_await] void
	 * @throw
	 */
	template <class Rep, class Period>
	LIBFUTURE_API inline future_t<> operator co_await(std::chrono::duration<Rep, Period> dt_);

	/**
	 * @brief 协程专用的睡眠功能。
	 * @return [co_await] void
	 * @throw
	 */
	template <class Rep, class Period>
	LIBFUTURE_API inline future_t<> sleep(std::chrono::duration<Rep, Period> dt_);

}

#include "sleep.inl"

#endif
