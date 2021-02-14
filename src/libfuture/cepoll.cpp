#include "cepoll.h"
#if __linux__
#include "clog.h"

//析构
CEpoll::~CEpoll()
{
	Destory();
}

//创建epoll根节点
int CEpoll::Create(int max_events)
{
	if (epfd_ > 0)
	{
		Destory();
		LOG_WARNING("CEpoll create has Destory old epfd\n");
	}
	max_events_ = max_events;
	p_events_ = new epoll_event[max_events];
	epfd_ = epoll_create(max_events);
	if (epfd_ == -1)
	{
		LOG_WARNING("CEpoll epoll_create error: %s\n", strerror(errno));
	}
	return epfd_;
}

//销毁epoll
void CEpoll::Destory()
{
	if (epfd_ > 0)
	{
		close(epfd_);
		epfd_ = -1;
	}
	if (p_events_)
	{
		delete[] p_events_;
		p_events_ = nullptr;
	}
}

//挂载socket到epoll上
int CEpoll::Ctl(int opt, int sockfd, uint32_t events)
{
	epoll_event ev;
	ev.events = events;
	ev.data.fd = sockfd;
	int ret = epoll_ctl(epfd_, opt, sockfd, &ev);
	if (ret == -1)
	{
		LOG_WARNING("CEpoll ctl1 failed\n");
	}
	return ret;
}

//挂载client指针到epoll上
int CEpoll::Ctl(int opt, CClient* pclient, uint32_t events)
{
	epoll_event ev;
	ev.events = events;
	ev.data.ptr = pclient;
	int ret = epoll_ctl(epfd_, opt, pclient->sockfd(), &ev);
	if (ret == -1)
	{
		LOG_WARNING("CEpoll ctl2 failed\n");
	}
	return ret;
}

//等待事件
int CEpoll::Wait(int milliseconds)
{
	int n = epoll_wait(epfd_, p_events_, max_events_, milliseconds);
	if (n == -1)
	{
		if (errno == EAGAIN || errno == EWOULDBLOCK)
		{
			return 0;
		}
		if (errno == EINTR)
		{
			return 0;
		}
		if (errno == ECONNRESET)
		{
			//客户端没有正常的关闭socket连接，却关闭了整个运行程序
			return 0;
		}

		LOG_WARNING("CEpoll Wait failed\n");
	}
	return n;
}

//获得所有事件指针
epoll_event *CEpoll::events()
{
	return p_events_;
}

#endif
