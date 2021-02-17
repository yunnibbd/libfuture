#ifndef __CEPOLL_H__
#define __CEPOLL_H__

#if __linux__
#include "export_api.h"
#include "include.h"
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <string.h>
#include <errno.h>

/**
 * @brief 对epoll的封装
 */
class LIBFUTURE_API epoll_t
{
public:
	/**
	 * @brief 析构,在其中调用销毁epoll
	 * @param
	 * @return
	 */
	~epoll_t();

	/**
	 * @brief 创建epoll根节点
	 * @param max_events 最大可容纳的事件数量
	 * @return int 成功返回epoll根节点,失败返回-1
	 */
	int create(int max_events);

	/**
	 * @brief 销毁epoll
	 * @param
	 * @return
	 */
	void destory();

	/**
	 * @brief 挂载socket到epoll上
	 * @param opt 操作类型
	 * @param sockfd 待挂载的套接字
	 * @param events 事件类型
	 * @return int 挂载结果
	 */
	int ctl(int opt, int sockfd, uint32_t events);

	/**
	 * @brief 挂载client指针到epoll上
	 * @param opt 操作类型
	 * @param pclient 待挂载的指针类型
	 * @param events 事件类型
	 * @return int 挂载结果
	 */
	int ctl(int opt, CClient* pclient, uint32_t events);

	/**
	 * @brief 等待事件
	 * @param milliseconds 超时的事件 毫秒
	 * @return int 等待的结果
	 */
	int wait(int milliseconds = 0);

	/**
	 * @brief 获得所有事件指针
	 * @param
	 * @return epoll_event* 所有的事件指针
	 */
	epoll_event *events();
private:
	//epoll树根节点
	int epfd_ = -1;
	//epoll_event数组指针
	epoll_event* p_events_ = nullptr;
	//用于Wait函数
	int max_events_ = -1;
};

#endif

#endif
