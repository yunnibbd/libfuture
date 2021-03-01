#include "buffer.h"
#include "clog.h"
#include "error_code.h"
#include "include.h"
#include <iostream>
#include <string>
using namespace std;
using namespace libfuture;

/**
 * @brief 构造,初始buffer大小为8k
 * @param size 初始大小
 * @return
 */
buffer_t::buffer_t(int size) : n_size_(size)
{
	p_buffer_ = new char[size];
}

/**
 * @brief 析构,在里面会释放空间
 * @param
 * @return
 */
buffer_t::~buffer_t()
{
	if (p_buffer_)
	{
		delete[] p_buffer_;
		p_buffer_ = nullptr;
	}
}

/**
 * @brief 往缓冲区里放数据
 * @param data 源数据
 * @param data_len 源数据长度
 * @return bool 是否存放成功
 */
bool buffer_t::push(const char *data, int data_len)
{
	//如果还有空间
	if (n_last_ + data_len <= n_size_)
	{
		memcpy(p_buffer_ + n_last_, data, data_len);
		//更新缓冲区数据长度
		n_last_ += data_len;
		if (n_last_ == n_size_)
			++full_count_;
		return true;
	}
	++full_count_;
	return false;
}

/**
 * @brief 将缓冲区一部分数据移除
 * @param len 要移除的数据长度
 * @return
 */
void buffer_t::pop(int len)
{
	//判断长度正常
	int n = n_last_ - len;
	if (n > 0 && n < n_size_)
	{
		//等于0就不用移动了
		memmove(p_buffer_, p_buffer_ + len, n);
	}
	//缓冲区尾部指针前移
	n_last_ = n;
	if (full_count_ > 0)
		--full_count_;
}

/**
 * @brief 将缓冲区中的所有数据移除
 * @param
 * @return
 */
void buffer_t::clear()
{
	pop(data_len());
}

/**
 * @brief 从socket中读取数据
 * @param sockfd 从哪个socket中读取数据
 * @return int 本次读取的长度
 */
int buffer_t::read4socket(int sockfd)
{
	int ret = 0;
	do
	{
		if (n_size_ - n_last_ > 0)
		{
			//还有空位置可以用
			//接受客户端数据
			char* szRecv = p_buffer_ + n_last_;
			ret = (int)recv(sockfd, szRecv, n_size_ - n_last_, 0);
			if (ret <= 0)
				break;
			//缓冲区尾部位置后移
			n_last_ += ret;
		}
	} while (0);
	return ret;
}

/**
 * @brief 往socket中写数据
 * @param sockfd 往哪个socket中写数据
 * @return int 本次写入的数据长度
 */
int buffer_t::write2socket(int sockfd)
{
	int ret = 0;
	do
	{
		//确保缓冲区中有数据，并且sockfd是一个有效的socket
		if (n_last_ > 0 && sockfd != SOCK_NOT_INIT)
		{
			ret = send(sockfd, p_buffer_, n_last_, 0);
			if (ret <= 0)
				//发送错误
				break;
			else if (ret == n_size_)
				//全部都发送出去了,更新尾部数据指针
				n_last_ = 0;
			else
			{
				//数据没有全部发送出去
				n_last_ -= ret;
				memmove(p_buffer_, p_buffer_ + ret, n_last_);
			}
			full_count_ = 0;
		}
	} while (0);
	//返回0或者send的数据大小
	return ret;
}

#ifdef _WIN32
/**
 * @brief 创建一个用于iocp接收的数据缓冲区
 * @param sockfd 要接收的socket
 * @return IO_DATA_BASE* 缓冲区指针
 */
IO_DATA_BASE *buffer_t::make_recv_io_data(int sockfd)
{
	int nLen = n_size_ - n_last_;
	if (nLen > 0)
	{
		//如果接收缓冲区还可以空余空间的话，就将本次io缓冲区中指针指向本次接收
		//缓冲区数据位置，IOCP就会把数据放在这个位置上面
		io_data_.wsaBuff.buf = p_buffer_ + n_last_;
		io_data_.wsaBuff.len = nLen;
		io_data_.sockfd = sockfd;
		return &io_data_;
	}
	//在缓冲区没有位置存放数据的时候返回空指针
	return nullptr;
}

/**
 * @brief 告知缓冲区iocp为缓冲区写入了多少数据
 * @param nRecv 写入的长度
 * @return bool 是否成功
 */
bool buffer_t::read4iocp(int nRecv)
{
	if (nRecv > 0 && n_size_ - n_last_ >= nRecv)
	{
		//makeIOData的时候是判断了有没有剩余的空间去收数据的，
		//告诉缓冲区本次接收了多少数据的时候还是判断了一下
		n_last_ += nRecv;
		return true;
	}
	LOG_DEBUG("buffer_t read4iocp:sockfd<%d> nSize<%d> nLast<%d> nRecv<%d>\n", io_data_.sockfd, n_size_, n_last_, nRecv);
	return false;
}

/**
 * @brief 创建一个用于iocp发送的数据缓冲区
 * @param sockfd 要发送的socket
 * @return IO_DATA_BASE* 数据缓冲区
 */
IO_DATA_BASE *buffer_t::make_send_io_data(int sockfd)
{
	if (n_last_ > 0)
	{
		//如果接收缓冲区还可以空余空间的话，就将本次io缓冲区中指针指向本次接收
		//缓冲区数据位置，IOCP就会把数据放在这个位置上面
		io_data_.wsaBuff.buf = p_buffer_;
		io_data_.wsaBuff.len = n_last_;
		io_data_.sockfd = sockfd;
		return &io_data_;
	}
	//在缓冲区没有数据要发送的时候返回空指针
	return nullptr;
}

/**
 * @brief 告知缓冲区iocp发送了缓冲区中多少的数据
 * @param nSend 发送的长度
 * @return bool 是否成功
 */
bool buffer_t::write2iocp(int nSend)
{
	if (n_last_ < nSend)
	{
		LOG_DEBUG("write2iocp:sockfd<%d> nSize<%d> nLast<%d> nSend<%d>\n", io_data_.sockfd, n_size_, n_last_, nSend);
		return false;
	}
	else if (n_last_ == nSend)
	{//n_last_=2000 实际发送nSend=2000
	 //数据尾部位置清零
		n_last_ = 0;
	}
	else {
		//n_last_=2000 实际发送ret=1000
		n_last_ -= nSend;
		memcpy(p_buffer_, p_buffer_ + nSend, n_last_);
	}

	full_count_ = 0;
	return true;
}
#endif

static sockaddr_in remote_addr;
static sockaddr_in local_addr;
static int remote_addr_len = sizeof(sockaddr_in);
static int local_addr_len = sizeof(sockaddr_in);

/**
 * @brief 获得连接的地址
 * @param sockfd 要获得地址的套接字
 * @return
 */
void buffer_t::get_addr(int sockfd)
{
	
}

//缓冲区里是否有数据
bool buffer_t::has_data()
{
	return n_last_ > 0;
}

//得到缓冲区指针
char* buffer_t::data()
{
	return p_buffer_;
}

//返回buffer总大小
int buffer_t::buff_size()
{
	return n_size_;
}

//返回已有数据的长度
int buffer_t::data_len()
{
	return n_last_;
}
