#include "socket.h"
#ifdef _WIN32
#include <Windows.h>
#else
#include <sys/types.h>
#include <sys/socket.h>
#endif

/**
 * @brief 构造,主要调用初始化
 * @param
 * @return
 */
socket_t::socket_t()
{

}

socket_t::~socket_t()
{

}

/**
 * @brief 初始化socket
 * @param
 * @return
 */
void socket_t::init_socket()
{

}
