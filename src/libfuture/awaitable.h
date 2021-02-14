#ifndef __AWAITABLE_H__
#define __AWAITABLE_H__
#include <coroutine>
#include <iostream>

template <typename _Ty>
struct yield_awaitor;

template <typename _Ty>
class future_t;

template <typename _Ty = void>
class awaitable_t
{
public:
	using value_type = _Ty;
	using promise_type = yield_awaitor<value_type>;
	using future_type = future_t<value_type>;
	
	using coro_handle = std::coroutine_handle<promise_type>;
	
	awaitable_t() noexcept = default;

	explicit awaitable_t(coro_handle handle) : handle_(handle) {}

	awaitable_t(const awaitable_t&) noexcept = delete;

	awaitable_t(awaitable_t&&) noexcept = default;

	awaitable_t& operator = (const awaitable_t&) noexcept = default;
	awaitable_t& operator = (awaitable_t&&) = default;

	future_type get_future()
	{
		return future_type();
	}

	/**
	 * @brief co_await之前调用
	 * @param
	 * @return bool 是否需要挂起
	 */
	bool await_ready()
	{
		return false;
	}

	/**
	 * @brief 协程挂起调用
	 * @param h 当前协程
	 * @return
	 */
	void await_suspend(std::coroutine_handle<> h)
	{

	}

	/**
	 * @brief co_await完毕后返回给调用者
	 * @param
	 * @return value_type 返回值
	 */
	value_type await_resume()
	{
		std::cout << 1 << std::endl;
	}

	void set_value(std::coroutine_handle<> h) const
	{
		handle_ = h;
	}

	coro_handle handle() const noexcept
	{
		return handle_;
	}
private:
	coro_handle handle_;
};

#endif
