#ifndef __FUTURE_H__
#define __FUTURE_H__
#include "scheduler.h"
#include <coroutine>
#include <iostream>

template <typename _Ty>
struct promise_t;

template <typename _Ty>
class future_impl_t
{
public:
	using value_type = _Ty;
	using promise_type = promise_t<value_type>;
	using coro_handle = std::coroutine_handle<promise_type>;

	future_impl_t() = default;

	explicit future_impl_t(coro_handle handle) : handle_(handle) 
	{
	
	}

	//调用co_await时先进行判断
	bool await_ready()
	{
		return false;
	}

	//co_await一个协程
	void await_suspend(std::coroutine_handle<> h)
	{
		//此时的handle_是co_await的协程的
		do
		{
			if (!handle_)
				//表示当前是awaitable返回的future被resume了, 到这里还不满足条件去resume
				break;

			if (!handle_.done())
			{
				current_scheduler()->set_current_handle(handle_);
				handle_.resume();
			}

			if (!h.done())
			{
				current_scheduler()->set_current_handle(h);
				h.resume();
			}
		} while (0);
	}

	//切换到这个协程
	bool resume()
	{
		if (!handle_.done())
		{
			current_scheduler()->set_current_handle(handle_);
			handle_.resume();
		}
		return !handle_.done();
	}

	coro_handle handle()
	{
		return handle_;
	}

protected:
	//协程句柄
	coro_handle handle_;
};

template <typename _Ty = void>
class future_t : public future_impl_t<_Ty>
{
public:
	using typename future_impl_t<_Ty>::value_type;
	using typename future_impl_t<_Ty>::promise_type;
	using typename future_impl_t<_Ty>::coro_handle;

	future_t() = default;

	explicit future_t(coro_handle handle) : 
		future_impl_t<_Ty>(handle)
	{

	}

	//co_await后的返回值
	value_type await_resume() 
	{
		return this->handle_.promise().value_;
	}

	//
	value_type get() 
	{
		return this->handle_.promise().value_;
	}
};

template <>
class future_t<void> : public future_impl_t<void>
{
public:
	using typename future_impl_t<void>::value_type;
	using typename future_impl_t<void>::promise_type;
	using typename future_impl_t<void>::coro_handle;

	future_t() = default;

	explicit future_t(coro_handle handle) : 
		future_impl_t<void>(handle)
	{

	}

	~future_t()
	{

	}

	future_t(const future_t& future) = delete;

	future_t(future_t&&) = default;

	//co_await后的返回值
	void await_resume()
	{
		
	}

	//
	void get()
	{
		
	}
};

#endif
