#ifndef __EVENT_H__
#define __EVENT_H__
#include "awaitable.h"
#include "future.h"
#include "common.h"
#include <chrono>

namespace libfuture
{

	class buffer_t;
	class socket_t;

	/**
	 * @brief �첽����
	 * @param socket 
	 * @param ip Ҫ���ӵ�ip��ַ
	 * @param port Ҫ���ӵĶ˿�
	 * @return
	 */
	LIBFUTURE_API inline future_t<bool> open_connection(socket_t* socket, const char* ip, unsigned short port);

	/**
	 * @brief ����һ������
	 * @param socket 
	 * @return Э�̶���
	 */
	LIBFUTURE_API inline future_t<> open_accept_raw(socket_t* socket);

	/**
	 * @brief ����һ������
	 * @param socket
	 * @return �ͻ��˵�ַָ��
	 */
	LIBFUTURE_API inline future_t<sockaddr_in*> open_accept(socket_t* socket);

	/**
	 * @brief �����ݣ�������ֱ�ӵ��ã�ʹ��buffer_read
	 * @param buffer ���ݵĴ�ŵ�
	 * @param socket �������
	 * @param timeout ��ʱʱ��
	 * @return Э�̶���
	 */
	template <class Rep, class Period>
	LIBFUTURE_API inline future_t<> buffer_read_raw(buffer_t* buffer, socket_t* socket, std::chrono::duration<Rep, Period> timeout);

	/**
	 * @brief д���ݣ�������ֱ�ӵ��ã�ʹ��buffer_write
	 * @param buffer ����Դ
	 * @param socket ������д
	 * @param timeout ��ʱʱ��
	 * @return Э�̶���
	 */
	template <class Rep, class Period>
	LIBFUTURE_API inline future_t<> buffer_write_raw(buffer_t* buffer, socket_t* socket, std::chrono::duration<Rep, Period> timeout);

	/**
	 * @brief ������
	 * @param buffer ���ݵĴ�ŵ�
	 * @param socket �������
	 * @param timeout ��ʱʱ��
	 * @return bool �Ƿ�ʱ
	 */
	template <class Rep, class Period>
	LIBFUTURE_API inline future_t<bool> buffer_read(buffer_t* buffer, socket_t* socket, std::chrono::duration<Rep, Period> timeout);

	/**
	 * @brief д����
	 * @param buffer ����Դ
	 * @param socket ������д
	 * @param timeout ��ʱʱ��
	 * @return bool �Ƿ�ʱ
	 */
	template <class Rep, class Period>
	LIBFUTURE_API inline future_t<bool> buffer_write(buffer_t* buffer, socket_t* socket, std::chrono::duration<Rep, Period> timeout);
}

#include "event.inl"

#endif
