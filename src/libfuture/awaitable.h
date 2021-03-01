#ifndef __AWAITABLE_H__
#define __AWAITABLE_H__
#include "export_api.h"
#include <coroutine>
#include <iostream>

namespace libfuture
{

	template <typename _Ty>
	class future_t;

	template <typename _Ty>
	struct promise_t;

	/**
	 * @brief 一个可被co_await的对象
	 */
	template <typename _Ty = void>
	class LIBFUTURE_API awaitable_t
	{
	public:
		using value_type = _Ty;
		using promise_type = promise_t<value_type>;
		using future_type = future_t<value_type>;

		using coro_handle = std::coroutine_handle<promise_type>;

		awaitable_t() noexcept = default;

		explicit awaitable_t(coro_handle handle) : handle_(handle) {}

		awaitable_t(const awaitable_t&) noexcept = delete;

		awaitable_t(awaitable_t&&) noexcept = default;

		awaitable_t& operator = (const awaitable_t&) noexcept = default;
		awaitable_t& operator = (awaitable_t&&) = default;

		/**
		 * @brief 返回一个空handle的future_t对象
		 * @param
		 * @return future_type futuret_t对象
		 */
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

		/**
		 * @brief 设置当前handle
		 * @param h 待设置的handle
		 * @return
		 */
		void set_value(std::coroutine_handle<> h) const
		{
			handle_ = h;
		}

		/**
		 * @brief 获得当前awaitable对象所持有的handle
		 * @param
		 * @return coro_handle 持有的handle
		 */
		coro_handle handle() const noexcept
		{
			return handle_;
		}
	private:
		coro_handle handle_;
	};

}

#endif
