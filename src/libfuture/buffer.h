#ifndef __CBUFFER_H__
#define __CBUFFER_H__
#include <string.h>
#include "iocp.h"
#include "export_api.h"

//对网络数据缓冲区的封装
class LIBFUTURE_API buffer_t
{
public:
	buffer_t(int size = 8192);

	~buffer_t();

	//往缓冲区例放数据
	bool push(const char *data, int data_len);

	//将缓冲区一部分数据移除
	void pop(int len);

	//从socket中读取数据
	int read4socket(int sockfd);

	//往socket中写数据
	int write2socket(int sockfd);

#ifdef USE_IOCP
	//所有针对iocp的数据存储和读取
	IO_DATA_BASE *make_recv_io_data(int sockfd);

	// 数据对于IOCP来说是接收完成了告诉程序，那么就需要告诉缓冲区本次接收到了多少长度
	bool read4iocp(int nRecv);

	IO_DATA_BASE* make_send_io_data(int sockfd);

	bool write2iocp(int nSend);
#endif

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
