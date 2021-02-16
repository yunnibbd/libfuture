#ifndef __SOCKET_H__
#define __SOCKET_H__
#include "export_api.h"

class LIBFUTURE_API socket_t
{
public:
	/**
	 * @brief 构造,主要调用初始化
	 * @param
	 * @return
	 */
	socket_t();

	~socket_t();

	/**
	 * @brief 初始化socket
	 * @param
	 * @return
	 */
	void init_socket();
private:
	int sockfd_;
};

#endif
