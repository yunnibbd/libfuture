#ifndef __BUFFER_H__
#define __BUFFER_H__
#include <cstring>
#include "iocp.h"
#include "export_api.h"

/**
 * @brief 对网络数据缓冲区的封装
 */
class LIBFUTURE_API buffer_t
{
public:
	/**
	 * @brief 构造,初始buffer大小为8k
	 * @param size 初始大小
	 * @return
	 */
	buffer_t(int size = 8192);

	/**
	 * @brief 析构,在里面会释放空间
	 * @param
	 * @return
	 */
	~buffer_t();

	/**
	 * @brief 往缓冲区里放数据
	 * @param data 源数据
	 * @param data_len 源数据长度
	 * @return bool 是否存放成功
	 */
	bool push(const char *data, int data_len);

	/**
	 * @brief 将缓冲区一部分数据移除
	 * @param len 要移除的数据长度
	 * @return
	 */
	void pop(int len);

	/**
	 * @brief 将缓冲区中的所有数据移除
	 * @param
	 * @return
	 */
	void clear();

	/**
	 * @brief 从socket中读取数据
	 * @param sockfd 从哪个socket中读取数据
	 * @return int 本次读取的长度
	 */
	int read4socket(int sockfd);

	/**
	 * @brief 往socket中写数据
	 * @param sockfd 往哪个socket中写数据
	 * @return int 本次写入的数据长度
	 */
	int write2socket(int sockfd);

#ifdef USE_IOCP
	/**
	 * @brief 创建一个用于iocp接收的数据缓冲区
	 * @param sockfd 要接收的socket
	 * @return IO_DATA_BASE* 缓冲区指针
	 */
	IO_DATA_BASE *make_recv_io_data(int sockfd);
	
	/**
	 * @brief 告知缓冲区iocp为缓冲区写入了多少数据
	 * @param nRecv 写入的长度
	 * @return bool 是否成功
	 */
	bool read4iocp(int nRecv);

	/**
	 * @brief 创建一个用于iocp发送的数据缓冲区
	 * @param sockfd 要发送的socket
	 * @return IO_DATA_BASE* 数据缓冲区
	 */
	IO_DATA_BASE* make_send_io_data(int sockfd);

	/**
	 * @brief 告知缓冲区iocp发送了缓冲区中多少的数据
	 * @param nSend 发送的长度
	 * @return bool 是否成功
	 */
	bool write2iocp(int nSend);
#endif

	/**
	 * @brief 获得连接的地址
	 * @param sockfd 要获得地址的套接字
	 * @return
	 */
	static void get_addr(int sockfd);

	//缓冲区里是否有数据
	bool has_data();

	//得到缓冲区指针
	char *data();

	//返回buffer总大小
	int buff_size();

	//返回已有数据的长度
	int data_len();
private:
	// 缓冲区指针
	char *p_buffer_ = nullptr;
	// 缓冲区的数据尾部位置(已有数据大小)
	int n_last_ = 0;
	// 缓冲区总的空间大小(字节长度)
	int n_size_ = 0;
	// 缓冲区写满计数
	int full_count_ = 0;
#ifdef USE_IOCP
	//IOCP使用的数据上下文
	IO_DATA_BASE io_data_ = { 0 };
#endif
};

#endif
