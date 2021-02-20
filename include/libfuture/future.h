#ifndef __FUTURE_H__
#define __FUTURE_H__
#include "scheduler.h"
#include "export_api.h"
#include <coroutine>
#include <iostream>

template <typename _Ty>
struct promise_t;

template <typename _Ty>
class LIBFUTURE_API future_impl_t
{
public:
	using value_type = _Ty;
	using promise_type = promise_t<value_type>;
	using coro_handle = std::coroutine_handle<promise_type>;

	future_impl_t() = default;

	explicit future_impl_t(coro_handle handle) : handle_(handle) 
	{
		
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
		//此时的handle_是co_await的协程的
		do
		{
			if (!handle_)
				//当前是一个awaitable对象
				break;

			do
			{
				if (!handle_.done())
				{
					current_scheduler()->set_current_handle(handle_);
					handle_.resume();
					if (handle_.done())
					{
						//被co_await的协程执行完毕,执行调用者协程
						//handle_.destroy();
						break;
					}
					//被co_await的协程没有执行完毕,添加进依赖队列
					current_scheduler()->add_to_depend(h, handle_);
					return;
				}
			} while (0);

			if (!h.done())
			{
				current_scheduler()->set_current_handle(h);
				h.resume();
			}
		} while (0);
	}

	/**
	 * @brief 切换到当前协程
	 * @param
	 * @return bool 当前协程是否执行完毕
	 */
	bool resume()
	{
		if (!handle_.done())
		{
			current_scheduler()->set_current_handle(handle_);
			handle_.resume();
		}
		return !handle_.done();
	}

	/**
	 * @brief 获取当前协程句柄
	 * @param
	 * @return coro_handle 协程句柄
	 */
	coro_handle handle()
	{
		return handle_;
	}

protected:
	//协程句柄
	coro_handle handle_;
};

template <typename _Ty = void>
class LIBFUTURE_API future_t : public future_impl_t<_Ty>
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

	/**
	 * @brief co_await完毕后返回给调用者
	 * @param
	 * @return value_type 返回值
	 */
	value_type await_resume() 
	{
		return this->handle_.promise().value_;
	}

	value_type get() 
	{
		return this->handle_.promise().value_;
	}
};

template <>
class LIBFUTURE_API future_t<void> : public future_impl_t<void>
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

	future_t(const future_t& future) = delete;

	future_t(future_t&&) = default;

	/**
	 * @brief co_await完毕后返回给调用者
	 * @param
	 * @return
	 */
	void await_resume()
	{
		
	}

	void get()
	{
		
	}
};

#endif
