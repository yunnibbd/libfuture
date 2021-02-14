#ifndef __ERROR_CODE_H__
#define __ERROR_CODE_H__

enum SocketCode
{
	SOCK_ACCEPT_ERR = -3,		//csocket对象Accept函数错误
	SERVER_PORT_NOT_INIT = -2,	//cserver对象未设置端口 
	SOCK_ERROR = -1,			//socket函数出现错误
	SOCK_NOT_INIT = 0,			//socket未初始化
};

#endif
