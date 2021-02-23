#include "scheduler.h"
#include "buffer.h"
#include "utils.h"
#include "clog.h"
#include "include.h"

/**
 * @brief 初始化
 * @param
 * @return
 */
void scheduler_t::init()
{
#ifdef _WIN32
	iocp_.create();
	iocp_.load_func(init_socket_);
	iocp_.reg(init_socket_);
	io_data_.wsaBuff.buf = buffer_;
	io_data_.wsaBuff.len = sizeof(buffer_);
#else
	epoll_.create(10240);
#endif
}

#ifdef _WIN32
/**
 * @brief iocp的处理io事件方式
 * @param
 * @return bool 为update_socketio_queue的返回值
 */
bool scheduler_t::iocp_loop()
{
	int64_t sleep_msec;
	IO_EVENT io_event;
	while (true)
	{
		if (socketio_queue_.empty())
		{
			if (ready_queue_.empty())
			{
				if (sleep_queue_.empty())
				{
					if (suspend_queue_.empty() && depend_queue_.empty())
						return true;
					else
						return false;
				}
				else
				{
					sleep_msec = sleep_queue_.begin()->first - utils_t::get_cur_timestamp();
					if (sleep_msec <= 0)
						return false;
				}
			}
			else
				return false;
		}
		else
		{
			if (ready_queue_.empty())
			{
				if (sleep_queue_.empty())
				{
					if (ready_queue_.empty())
						sleep_msec = INFINITE;
					else
						return false;
				}
				else
				{
					sleep_msec = sleep_queue_.begin()->first - utils_t::get_cur_timestamp();
					if (sleep_msec < 0)
						return false;
				}
			}
			else
				return false;
		}
		
		//std::cout << "iocp will sleep " << sleep_msec << std::endl;
		int ret = iocp_.wait(io_event, sleep_msec);
		if (ret < 0)
		{
			//IOCP出错
			LOG_ERROR("iocp_loop::iocp error\n");
			break;
		}
		else if (ret == 0)
			//没有事件
			break;

		switch (io_event.pIOData->iotype)
		{
		case IO_TYPE::RECV:
		{
			socket_t* socket = reinterpret_cast<socket_t*>(io_event.data.ptr);
			auto iter = socketio_queue_.find(socket->sockfd());
			if (iter != socketio_queue_.end())
			{
				ready_queue_.insert(iter->second);
				socketio_queue_.erase(iter);
			}
			auto p_buffer = socket->p_recv_buf();
			if (p_buffer)
				p_buffer->read4iocp(io_event.bytesTrans);
		}
		break;
		case IO_TYPE::SEND:
		{
			socket_t* socket = reinterpret_cast<socket_t*>(io_event.data.ptr);
			auto iter = socketio_queue_.find(socket->sockfd());
			if (iter != socketio_queue_.end())
			{
				ready_queue_.insert(iter->second);
				socketio_queue_.erase(iter);
			}
			auto p_buffer = socket->p_send_buf();
			if (p_buffer)
				p_buffer->write2iocp(io_event.bytesTrans);
		}
		break;
		case IO_TYPE::ACCEPT:
		{
			int sockfd = io_event.pIOData->sockfd;
			auto iter = socketio_queue_.find(sockfd);
			setsockopt(sockfd, SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT, (char*)&init_socket_, sizeof(init_socket_));
			getpeername(sockfd, (sockaddr*)&client_addr_, &client_addr_len_);
			if (iter != socketio_queue_.end())
			{
				ready_queue_.insert(iter->second);
				socketio_queue_.erase(iter);
			}
			
		}
		break;
		case IO_TYPE::CONNECT:
		{
			//暂未实现
		}
		break;
		default:
			break;
		}
	}
	return false;
}
#else
/**
 * @brief epoll的处理io事件方式
 * @param
 * @return bool 为update_socketio_queue的返回值
 */
bool scheduler_t::epoll_loop()
{
	int64_t sleep_msec = -1;

	if (socketio_queue_.empty())
	{
		if (ready_queue_.empty())
		{
			if (sleep_queue_.empty())
			{
				if (suspend_queue_.empty() && depend_queue_.empty())
					return true;
				else
					return false;
			}
			else
			{
				sleep_msec = sleep_queue_.begin()->first - utils_t::get_cur_timestamp();
				if (sleep_msec <= 0)
					return false;
			}
		}
		else
			return false;
	}
	else
	{
		if (ready_queue_.empty())
		{
			if (sleep_queue_.empty())
			{
				if (ready_queue_.empty())
					sleep_msec = -1;
				else
					return false;
			}
			else
			{
				sleep_msec = sleep_queue_.begin()->first - utils_t::get_cur_timestamp();
				if (sleep_msec < 0)
					return false;
			}
		}
		else
			return false;
	}

	int ret = epoll_.wait(sleep_msec);
	if (ret < 0)
	{
		LOG_WARNING("epoll_loop::epoll error\n");
		return false;
	}
	auto events = epoll_.events();
	for (int i = 0; i < ret; ++i)
	{
		socket_t* socket = reinterpret_cast<socket_t*>(events[i].data.ptr);
		if (!socket) continue;
		switch (socket->event_type())
		{
			case EVENT_SEND:
			{
				if (events[i].events & EPOLLOUT)
				{
					int sockfd = socket->sockfd();
					//收数据
					buffer_t* buffer = socket->p_send_buf();
					if (buffer)
						buffer->write2socket(sockfd);
					auto iter = socketio_queue_.find(sockfd);
					if (iter != socketio_queue_.end())
					{
						ready_queue_.insert(iter->second);
						socketio_queue_.erase(iter);
					}
				}
			}
			break;
			case EVENT_RECV:
			{
				if (events[i].events & EPOLLIN)
				{
					int sockfd = socket->sockfd();
					//收数据
					buffer_t* buffer = socket->p_recv_buf();
					if (buffer)
						buffer->read4socket(sockfd);
					auto iter = socketio_queue_.find(sockfd);
					if (iter != socketio_queue_.end())
					{
						ready_queue_.insert(iter->second);
						socketio_queue_.erase(iter);
					}
				}
			}
			break;
			case EVENT_ACCEPT:
			{
				if (events[i].events & EPOLLIN)
				{
					int c_sock = ::accept(init_socket_, (sockaddr*)&client_addr_, &client_addr_len_);
					socket->set_sockfd(c_sock);
					auto iter = socketio_queue_.find(init_socket_);
					if (iter != socketio_queue_.end())
					{
						ready_queue_.insert(iter->second);
						socketio_queue_.erase(iter);
					}
				}
			}
			break;
			case EVENT_CONNECT:
			{
				//暂未实现
			}
			break;
			default:
				break;
		}
	}
	return false;
}
#endif

scheduler_t::~scheduler_t()
{

}

/**
 * @brief 添加进socketio队列
 * @param socket 要通信的socket
 * @param type 本socket要进行的操作类型
 * @return
 */
void scheduler_t::add_to_socketio(socket_t* socket, event_type_enum type)
{
#ifdef _WIN32
	switch (type)
	{
		case EVENT_SEND:
		{
			int sockfd = socket->sockfd();
			auto p_buffer = socket->p_send_buf();
			if (!p_buffer)
				break;
			auto p_io_data = p_buffer->make_send_io_data(sockfd);
			if (!p_io_data)
				break;
			if (!socket->is_register)
			{
				if (!iocp_.reg(sockfd, socket))
					break;
				socket->is_register = true;
			}
			iocp_.post_send(p_io_data, sockfd);
			socketio_queue_.insert(std::make_pair(sockfd, current_handle()));
		}
		break;
		case EVENT_RECV:
		{
			int sockfd = socket->sockfd();
			auto p_buffer = socket->p_recv_buf();
			if (!p_buffer)
				break;
			auto p_io_data = p_buffer->make_recv_io_data(sockfd);
			if (!p_io_data)
				break;
			if (!socket->is_register)
			{
				if (!iocp_.reg(sockfd, socket))
					break;
				socket->is_register = true;
			}
			iocp_.post_recv(p_io_data, sockfd);
			socketio_queue_.insert(std::make_pair(sockfd, current_handle()));
		}
		break;
		case EVENT_ACCEPT:
		{
			int sockfd = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
			socket->set_sockfd(sockfd);
			if (!iocp_.post_accept(&io_data_, init_socket_, sockfd))
				break;
			socketio_queue_.insert(std::make_pair(sockfd, current_handle()));
		}
		break;
		default:
		break;
	}
#else
	switch (type)
	{
		case EVENT_SEND:
		{
			socket->set_event_type(EVENT_SEND);
			if (socket->is_register)
				epoll_.ctl(EPOLL_CTL_MOD, socket, EPOLLOUT);
			else
			{
				if (-1 == epoll_.ctl(EPOLL_CTL_ADD, socket, EPOLLOUT))
					break;
				socket->is_register = true;
			}
			socketio_queue_.insert(std::make_pair(socket->sockfd(), current_handle()));
		}
		break;
		case EVENT_RECV:
		{
			socket->set_event_type(EVENT_RECV);
			if (socket->is_register)
				epoll_.ctl(EPOLL_CTL_MOD, socket, EPOLLIN);
			else
			{
				if (-1 == epoll_.ctl(EPOLL_CTL_ADD, socket, EPOLLIN))
					break;
				socket->is_register = true;
			}
			socketio_queue_.insert(std::make_pair(socket->sockfd(), current_handle()));
		}
		break;
		case EVENT_ACCEPT:
		{
			//只需第一次要注册监听套接字
			static bool is_first = true;
			socket->set_event_type(EVENT_ACCEPT);
			socket->set_sockfd(init_socket_);
			if (is_first)
			{
				if (-1 == epoll_.ctl(EPOLL_CTL_ADD, socket, EPOLLIN))
					break;
				is_first = false;
			}
			else
			{
				if (-1 == epoll_.ctl(EPOLL_CTL_MOD, socket, EPOLLIN))
					break;
			}
			socketio_queue_.insert(std::make_pair(init_socket_, current_handle()));
		}
		break;
		default:
		break;
	}
#endif
}

/*
 * @brief 添加connect事件进入socketio队列
 * @param socket 要通信的socket
 * @param buffer 连接上时要发送的数据
 * @param ip 要连接的ip地址
 * @param port 要连接的端口
 * @return
 */
void scheduler_t::add_to_connect(socket_t* socket, buffer_t* buffer, const char* ip, unsigned short port)
{
#ifdef _WIN32
	int sockfd = socket->sockfd();
	auto p_buffer = socket->p_send_buf();
	if (!p_buffer)
		return;
	auto p_io_data = p_buffer->make_send_io_data(sockfd);
	if (!p_io_data)
		return;
	//iocp_.reg(sockfd, socket);
	sockaddr_in serv;
	memset(&serv, 0, sizeof(serv));
	serv.sin_family = AF_INET;
	serv.sin_port = htons(port);
	serv.sin_addr.s_addr = inet_addr(ip);
	iocp_.post_connect(p_io_data, sockfd, (sockaddr*)&serv, sizeof(serv));
	socketio_queue_.insert(std::make_pair(sockfd, current_handle()));
#else

#endif
}

/**
 * @brief 调度socketio_queue_
 * @param
 * @return bool 所有任务是否处理完毕
 */
bool scheduler_t::update_socketio_queue()
{
#ifdef _WIN32
	return iocp_loop();
#else
	return epoll_loop();
#endif
}
