#include "scheduler.h"
#include "future.h"
#include "utils.h"
#include "iocp.h"
#include "socket.h"
#include "event.h"
#include "clog.h"
#include "buffer.h"
#include "common.h"

/**
 * @brief ����ʱ��������Э�̾��
 * @param
 * @return
 */
scheduler_t::~scheduler_t()
{
	destory_scheduler();
}

/**
 * @brief ��������Э�̾��
 * @param
 * @return
 */
void scheduler_t::destory_scheduler()
{
	for (auto& handle : ready_queue_)
		handle.destroy();
	ready_queue_.clear();
	for (auto& entry : sleep_queue_)
		entry.second.destroy();
	sleep_queue_.clear();
	for (auto& entry : depend_queue_)
		entry.second.destroy();
	depend_queue_.clear();
	for (auto& handle : suspend_queue_)
		handle.destroy();
	suspend_queue_.clear();
	for (auto& entry : socketio_queue_)
		entry.second.destroy();
	socketio_queue_.clear();
	for (auto& entry : accept_socket_queue_)
		entry.second.destroy();
	accept_socket_queue_.clear();
}

/**
 * @brief ��ӽ�Э�̹�ϵ��������
 * @param handle Ҫ�ȴ����Э�̵�Э��
 * @param dependent ��������Э��
 * @return
 */
void scheduler_t::add_to_depend(handle_type handle, handle_type dependent)
{
	depend_queue_.insert(std::make_pair(handle, dependent));
}

/**
 * @brief ��ӽ��������
 * @param handle ����co_yield��Э�̾��
 * @return
 */
void scheduler_t::add_to_suspend(handle_type handle)
{
	suspend_queue_.insert(handle);
}

/**
 * @brief ��ӽ�socketio����
 * @param socket Ҫͨ�ŵ�socket
 * @param type ��socketҪ���еĲ�������
 * @return
 */
void scheduler_t::add_to_socketio(socket_t* socket, event_type_enum type)
{
	int sockfd = socket->sockfd();
	do
	{
		if (type == EVENT_SEND)
		{
			auto p_buffer = socket->p_send_buf();
			if (!p_buffer)
				break;
			auto p_io_data = p_buffer->make_send_io_data(sockfd);
			if (!p_io_data)
				break;
			iocp_.post_send(p_io_data, sockfd);
		}
		else if (type == EVENT_RECV)
		{
			auto p_buffer = socket->p_recv_buf();
			if (!p_buffer)
				break;
			auto p_io_data = p_buffer->make_recv_io_data(sockfd);
			if (!p_io_data)
				break;
			iocp_.post_recv(p_io_data, sockfd);
		}

		socketio_queue_.insert(std::make_pair(socket, current_handle()));
	} while (0);
}

/*
	* @brief ��ӽ�accept����
	* @param socket Ҫͨ�ŵ�socket
	* @return
	*/
	/*void add_to_accept(socket_t* socket)
	{
		g_io_data.wsaBuff.buf = g_accept_buffer;
		g_io_data.wsaBuff.len = ACCEPT_BUFFER_LEN;
		iocp_.post_accept(&g_io_data, socket->sockfd());
		accept_socket_queue_.insert(std::make_pair(socket, current_handle()));
	}*/

/**
 * @brief ���һ����Ҫ�ȴ���ĳһʱ�����е�Э��
 * @param msec Ҫ�ȴ���ʱ��
 * @return
 */
void scheduler_t::sleep_until(uint64_t msec)
{
	auto handle = current_handle();
	sleep_queue_.insert(std::make_pair(msec, handle));
	in_sleep_queue_.insert(handle);
}

/**
 * @brief ��ʼ��������Э�̣�ֱ���������
 * @param
 * @return
 */
void scheduler_t::run_until_no_task()
{
	while (true)
	{
		update_ready_queue();
		update_sleep_queue();
		update_depend_queue();
		update_suspend_queue();
		if (update_socketio_queue())
			break;
	}
}

/**
 * @brief �洢scheduler��ǰҪִ�е�Э�̾��
 * @param handle Ҫ�洢��Э�̾��
 * @return
 */
void scheduler_t::set_current_handle(handle_type handle)
{
	current_handle_ = handle;
}

/**
 * @brief ��õ�ǰ����ִ�е�Э�̾��
 * @param
 * @return handle_type Э�̾��
 */
scheduler_t::handle_type scheduler_t::current_handle()
{
	return current_handle_;
}

/**
 * @brief ����
 * @param
 * @return
 */
scheduler_t::scheduler_t()
{
	iocp_.create();
	iocp_.load_func();
}

/**
 * @brief �������߶���
 * @param
 * @return bool sleep_queue_�Ƿ�Ϊ��
 */
void scheduler_t::update_sleep_queue()
{
	auto begin = sleep_queue_.begin();
	auto end = sleep_queue_.end();
	auto cur_ms = utils_t::get_cur_timestamp();
	for (; begin != end; )
	{
		if (begin->first <= cur_ms)
		{
			do
			{
				auto handle = begin->second;
				if (handle.done())
					break;
				auto start = depend_queue_.lower_bound(handle);
				auto stop = depend_queue_.upper_bound(handle);
				bool can_trigger = true;
				for (; start != stop; )
				{
					if (!start->second.done())
					{
						can_trigger = false;
						break;
					}
					++start;
				}
				if (can_trigger)
				{
					set_current_handle(handle);
					handle.resume();
				}
			} while (0);
			in_sleep_queue_.erase(begin->second);
			begin = sleep_queue_.erase(begin);
		}
		break;
	}
}

/**
 * @brief ����socketio_queue_
 * @param
 * @return bool ���������Ƿ������
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
			if (sleep_msec < -1)
				sleep_msec = INFINITE;
		}
		else
		{
			if (ready_queue_.empty() && suspend_queue_.empty() &&
				depend_queue_.empty() && socketio_queue_.empty())
				//������ж��ж�Ϊ���򷵻���,������ֹ�¼�ѭ��
				return true;
			sleep_msec = INFINITE;
		}
		std::cout << "iocp will sleep " << sleep_msec << std::endl;
		int ret = iocp_.wait(io_event, sleep_msec);
		if (ret < 0)
		{
			//IOCP����
			LOG_ERROR("update_socketio_queue::iocp error\n");
			break;
		}
		else if (ret == 0)
			//û���¼�
			break;

		switch (io_event.pIOData->iotype)
		{
		case IO_TYPE::RECV:
		{
			socket_t* socket = reinterpret_cast<socket_t*>(io_event.data.ptr);
			auto iter = socketio_queue_.find(socket);
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
			auto iter = socketio_queue_.find(socket);
			if (iter != socketio_queue_.end())
			{
				ready_queue_.insert(iter->second);
				socketio_queue_.erase(iter);
			}
			auto p_buffer = socket->p_send_buf();
			if (p_buffer)
				p_buffer->write2socket(io_event.bytesTrans);
		}
		break;
		case IO_TYPE::ACCEPT:
		{
			socket_t* socket = reinterpret_cast<socket_t*>(io_event.data.ptr);
			auto iter = accept_socket_queue_.find(socket);
			if (iter != accept_socket_queue_.end())
			{
				ready_queue_.insert(iter->second);
				accept_socket_queue_.erase(iter);
			}
		}
		break;
		case IO_TYPE::CONNECT:
		{
			//��δʵ��
		}
		break;
		default:
			break;
		}
	}

	return false;
}

/**
 * @brief ����Ԥ������
 * @param
 * @return
 */
void scheduler_t::update_ready_queue()
{
	auto begin = ready_queue_.begin();
	auto end = ready_queue_.end();
	for (; begin != end; )
	{
		do
		{
			if (begin->done())
			{
				begin->destroy();
				break;
			}
			set_current_handle(*begin);
			begin->resume();
		} while (0);
		begin = ready_queue_.erase(begin);
	}
}

/**
 * @brief ������������
 * @param
 * @return
 */
void scheduler_t::update_depend_queue()
{
	auto begin = depend_queue_.begin();
	auto end = depend_queue_.end();
	for (; begin != end; )
	{
		bool can_trigger = true;
		auto start = depend_queue_.lower_bound(begin->first);
		auto stop = depend_queue_.upper_bound(begin->first);
		for (; start != end; )
		{
			if (!start->second.done())
			{
				can_trigger = false;
				break;
			}
			++start;
		}
		if (can_trigger)
		{
			if (!begin->first.done())
			{
				//��ѯ��ǰЭ���Ƿ���sleep_queue_��
				if (in_sleep_queue_.find(begin->first) == in_sleep_queue_.end())
				{
					set_current_handle(begin->first);
					begin->first.resume();
				}
			}
			if (begin->first.done())
			{
				begin = depend_queue_.erase(begin);
				continue;
			}
		}
		begin = stop;
	}
}

/**
 * @brief ���ȹ������
 * @param
 * @return bool ��������Ƿ�Ϊ��
 */
void scheduler_t::update_suspend_queue()
{
	auto begin = suspend_queue_.begin();
	auto end = suspend_queue_.end();
	for (; begin != end; )
	{
		if (!begin->done())
		{
			ready_queue_.insert(*begin);
			begin = suspend_queue_.erase(begin);
			continue;
		}
		++begin;
	}
}
