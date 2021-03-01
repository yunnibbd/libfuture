#include "libfuture.h"
#include <string>
#include <iostream>
using namespace std;
using namespace libfuture;

string send_str = "\
Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,image/apng,*/*;q=0.8,application/signed-exchange;v=b3;q=0.9\
Accept-Encoding: gzip, deflate\
Accept-Language: zh-CN, zh; q=0.9\
Cache-Control: max-age=0\
Connection: keep-alive\
Host: 14.215.177.39\
Upgrade-Insecure-Requests : 1\
User-Agent: Mozilla/5.0 (Windows NT 6.1; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/88.0.4324.190 Safari/537.36";

future_t<> test_connect()
{
	buffer_t buffer;
	socket_t client_socket(AF_INET, SOCK_STREAM, 0);

	//连接b站，肯定会被拒绝，但是还是会返回一些消息的
	bool has_c = co_await open_connection(&client_socket, "110.43.34.66", 443);
	if (!has_c)
		cout << "连接失败" << endl;

	cout << "连接成功" << endl;

	buffer.push(send_str.c_str(), send_str.size());

	co_await buffer_write(&buffer, &client_socket);

	cout << "发送消息成功" << endl;

	buffer.clear();

	//看看回了什么消息
	co_await buffer_read(&buffer, &client_socket);

	if (buffer.has_data())
	{
		//防止烫烫或屯屯
		buffer.data()[buffer.data_len()] = 0;
		cout << buffer.data() << endl;
	}

	co_return;
}

int main(int argc, char** argv)
{
#ifdef _WIN32
	WSADATA data;
	WSAStartup(MAKEWORD(2, 2), &data);
#endif
	auto sche = current_scheduler();
	sche->init();

	cpp test_connect();

	sche->run_until_no_task();

#ifdef _WIN32
	WSACleanup();
#endif

	return 0;
}
