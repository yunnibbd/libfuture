#include "epoll.h"
#if __linux__
#include "clog.h"
#include "socket.h"
using namespace libfuture;

/**
 * @brief 析构,在其中调用销毁epoll
 * @param
 * @return
 */
epoll_t::~epoll_t()
{
	destory();
}

/**
 * @brief 创建epoll根节点
 * @param max_events 最大可容纳的事件数量
 * @return int 成功返回epoll根节点,失败返回-1
 */
int epoll_t::create(int max_events)
{
	if (epfd_ > 0)
	{
		destory();
		LOG_WARNING("epoll_t create has Destory old epfd\n");
	}
	max_events_ = max_events;
	p_events_ = new epoll_event[max_events];
	epfd_ = epoll_create(max_events);
	if (epfd_ == -1)
	{
		LOG_WARNING("epoll_t epoll_create error: %s\n", strerror(errno));
	}
	return epfd_;
}

/**
 * @brief 销毁epoll
 * @param
 * @return
 */
void epoll_t::destory()
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

/**
 * @brief 挂载socket到epoll上
 * @param opt 操作类型
 * @param sockfd 待挂载的套接字
 * @param events 事件类型
 * @return int 挂载结果
 */
int epoll_t::ctl(int opt, int sockfd, uint32_t events)
{
	epoll_event ev;
	ev.events = events;
	ev.data.fd = sockfd;
	int ret = epoll_ctl(epfd_, opt, sockfd, &ev);
	if (ret == -1)
		LOG_WARNING("epoll_t ctl1 failed\n");
	if (errno == EEXIST)
	{
		if (epoll_ctl(epfd_, EPOLL_CTL_MOD, sockfd, &ev) == -1)
			return -1;
		else
			return 0;
	}
	return ret;
}

/**
 * @brief 挂载client指针到epoll上
 * @param opt 操作类型
 * @param pclient 待挂载的指针类型
 * @param events 事件类型
 * @return int 挂载结果
 */
int epoll_t::ctl(int opt, socket_t* socket, uint32_t events)
{
	epoll_event ev;
	ev.events = events;
	ev.data.ptr = socket;
	int ret = epoll_ctl(epfd_, opt, socket->sockfd(), &ev);
	if (ret == -1)
	{
		LOG_WARNING("epoll_t ctl2 failed\n");
	}
	return ret;
}

/**
 * @brief 等待事件
 * @param milliseconds 超时的事件 毫秒
 * @return int 等待的结果
 */
int epoll_t::wait(int milliseconds)
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

		LOG_WARNING("epoll_t Wait failed\n");
	}
	return n;
}

/**
 * @brief 获得所有事件指针
 * @param
 * @return epoll_event* 所有的事件指针
 */
epoll_event *epoll_t::events()
{
	return p_events_;
}

#endif
