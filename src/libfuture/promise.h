#ifndef __PROMISE_H__
#define __PROMISE_H__
#include "future.h"
#include <coroutine>
#include <iostream>
#include <exception>

template <typename _Ty>
struct promise_impl_t
{
	using value_type = _Ty;
	using promise_type = promise_t<value_type>;
	using future_type = future_t<value_type>;

	using coro_handle = std::coroutine_handle<promise_type>;

	/**
	 * @brief get_return_object后执行
	 * @param
	 * @return
	 */
	constexpr auto initial_suspend() noexcept
	{
		return std::suspend_always();
	}

	/**
	 * @brief 协程关闭后执行
	 * @param
	 * @return
	 */
	constexpr auto final_suspend() noexcept
	{
		return std::suspend_always();
	}

	/**
	 * @brief 异常时执行
	 * @param
	 * @return
	 */
	void unhandled_exception()
	{
		std::terminate();
	}
};

template <typename _Ty>
struct promise_t : public promise_impl_t<_Ty>
{
	using typename promise_impl_t<_Ty>::value_type;
	using typename promise_impl_t<_Ty>::promise_type;
	using typename promise_impl_t<_Ty>::future_type;
	using typename promise_impl_t<_Ty>::coro_handle;

	/**
	 * @brief 对一个协程进行构造后会执行，返回为当前协程
	 * @param
	 * @return
	 */
	future_type get_return_object() noexcept 
	{
		return future_type(coro_handle::from_promise(*this));
	}

	/**
	 * @brief 使用co_yield的时候会调用
	 * @param value co_yield右边的值
	 * @return
	 */
	std::suspend_always yield_value(value_type value) noexcept
	{
		value_ = value;
		auto scheduler = current_scheduler();
		scheduler->add_to_suspend(scheduler->current_handle());
		return std::suspend_always();
	}

	/**
	 * @brief 使用co_return一个值时会调用,或者协程结束后也会调用
	 * @param value co_return右边的值
	 * @return
	 */
	void return_value(value_type value) noexcept 
	{
		value_ = value;
	}

	value_type value_;
};

template <>
struct promise_t<void> : public promise_impl_t<void>
{
	using typename promise_impl_t<void>::value_type;
	using typename promise_impl_t<void>::promise_type;
	using typename promise_impl_t<void>::future_type;
	using typename promise_impl_t<void>::coro_handle;

	/**
	 * @brief 对一个协程进行构造后会执行，返回为当前协程
	 * @param
	 * @return
	 */
	future_type get_return_object() noexcept 
	{
		return future_type(coro_handle::from_promise(*this));
	}

	/**
	 * @brief 使用co_yield的时候会调用
	 * @param
	 * @return
	 */
	std::suspend_always yield_value() noexcept
	{
		auto scheduler = current_scheduler();
		scheduler->add_to_suspend(scheduler->current_handle());
		return std::suspend_always();
	}

	/**
	 * @brief 使用co_return一个值时会调用,或者协程结束后也会调用
	 * @param
	 * @return
	 */
	auto return_void() 
	{
	
	}
};

#endif
