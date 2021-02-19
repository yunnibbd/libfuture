#include "scheduler.h"
#include "buffer.h"
#include "utils.h"
#include "clog.h"

scheduler_t* scheduler_t::signal_instance_ = nullptr;

/**
 * @brief 初始化
 * @param
 * @return
 */
void scheduler_t::init()
{
	iocp_.create();
	iocp_.load_func(init_socket_);
	iocp_.reg(init_socket_);
	io_data_.wsaBuff.buf = buffer_;
	io_data_.wsaBuff.len = sizeof(buffer_);
}

/**
 * @brief 添加进socketio队列
 * @param socket 要通信的socket
 * @param type 本socket要进行的操作类型
 * @return
 */
void scheduler_t::add_to_socketio(socket_t* socket, event_type_enum type)
{
	int sockfd = socket->sockfd();
	do
	{
		switch (type)
		{
		case EVENT_SEND:
		{
			auto p_buffer = socket->p_send_buf();
			if (!p_buffer)
				break;
			auto p_io_data = p_buffer->make_send_io_data(sockfd);
			if (!p_io_data)
				break;
			if (!socket->is_register)
			{
				iocp_.reg(sockfd, socket);
				socket->is_register = true;
			}
			iocp_.post_send(p_io_data, sockfd);
		}
		break;
		case EVENT_RECV:
		{
			auto p_buffer = socket->p_recv_buf();
			if (!p_buffer)
				break;
			auto p_io_data = p_buffer->make_recv_io_data(sockfd);
			if (!p_io_data)
				break;
			if (!socket->is_register)
			{
				iocp_.reg(sockfd, socket);
				socket->is_register = true;
			}
			iocp_.post_recv(p_io_data, sockfd);
		}
		break;
		case EVENT_ACCEPT:
		{
			iocp_.post_accept(&io_data_, init_socket_, sockfd);
		}
		break;
		default:
			break;
		}

		socketio_queue_.insert(std::make_pair(sockfd, current_handle()));
	} while (0);
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
}

/**
 * @brief 调度socketio_queue_
 * @param
 * @return bool 所有任务是否处理完毕
 */
bool scheduler_t::update_socketio_queue()
{
	int64_t sleep_msec;
	IO_EVENT io_event;

	while (true)
	{
		if (!sleep_queue_.empty())
		{
			sleep_msec = sleep_queue_.begin()->first - utils_t::get_cur_timestamp();
			if (sleep_msec < 0)
				return false;
		}
		else
		{
			if (socketio_queue_.empty())
			{
				if (suspend_queue_.empty() &&
					depend_queue_.empty() &&
					ready_queue_.empty())
					//此处用于终止事件循环
					return true;
				else
					return false;
			}
			else
			{
				if (ready_queue_.empty())
					sleep_msec = INFINITE;
				else
					//预备队列中有需要立马执行的协程
					return false;
			}
		}
		//std::cout << "iocp will sleep " << sleep_msec << std::endl;
		int ret = iocp_.wait(io_event, sleep_msec);
		if (ret < 0)
		{
			//IOCP出错
			LOG_ERROR("update_socketio_queue::iocp error\n");
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
			auto iter = socketio_queue_.find(io_event.pIOData->sockfd);
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
