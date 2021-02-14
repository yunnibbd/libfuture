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

	//调用co_await时先进行判断
	bool await_ready()
	{
		return false;
	}

	//co_await一个协程
	void await_suspend(std::coroutine_handle<> h)
	{
		//cout << h.address() << endl;
		//此时的handle_是await的协程的
		
	}

	//co_await后的返回值
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
