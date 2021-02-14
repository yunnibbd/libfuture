#ifndef __CEPOLL_H__
#define __CEPOLL_H__

#if __linux__
#include "export_api.h"
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <string.h>
#include <errno.h>

class CClient;

//对epoll的封装
class COMMON_EXPORT CEpoll
{
public:
	//析构
	~CEpoll();

	//创建epoll根节点
	int Create(int max_events);

	//销毁epoll
	void Destory();

	//挂载socket到epoll上
	int Ctl(int opt, int sockfd, uint32_t events);

	//挂载client指针到epoll上
	int Ctl(int opt, CClient* pclient, uint32_t events);

	//等待事件
	int Wait(int milliseconds = 0);

	//获得所有事件指针
	epoll_event *events();
private:
	// epoll树根节点
	int epfd_ = -1;
	// epoll_event数组指针
	epoll_event* p_events_ = nullptr;
	// 用于Wait函数
	int max_events_ = -1;
};

#endif

#endif
