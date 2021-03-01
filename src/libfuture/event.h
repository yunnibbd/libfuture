#ifndef __EVENT_H__
#define __EVENT_H__
#include "awaitable.h"
#include "future.h"
#include "common.h"

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
	LIBFUTURE_API inline future_t<> open_accept(socket_t* socket);

	/**
	 * @brief ������
	 * @param buffer ���ݵĴ�ŵ�
	 * @param socket �������
	 * @return Э�̶���
	 */
	LIBFUTURE_API inline future_t<> buffer_read(buffer_t* buffer, socket_t* socket);

	/**
	 * @brief д����
	 * @param buffer ����Դ
	 * @param socket ������д
	 * @return Э�̶���
	 */
	LIBFUTURE_API inline future_t<> buffer_write(buffer_t* buffer, socket_t* socket);
}

#include "event.inl"

#endif
