#ifndef __SOCKET_H__
#define __SOCKET_H__
#include "export_api.h"

class LIBFUTURE_API socket_t
{
public:
	/**
	 * @brief ����,��Ҫ���ó�ʼ��
	 * @param
	 * @return
	 */
	socket_t();

	~socket_t();

	/**
	 * @brief ��ʼ��socket
	 * @param
	 * @return
	 */
	void init_socket();
private:
	int sockfd_;
};

#endif
