#ifndef __ERROR_CODE_H__
#define __ERROR_CODE_H__

namespace libfuture
{

	enum SocketCode
	{
		SOCK_ERROR = -1,			//socket函数出现错误
		SOCK_NOT_INIT = 0,			//socket未初始化
	};

}

#endif
