#ifndef __NON_COPYABLE_H__
#define __NON_COPYABLE_H__
#include "export_api.h"

namespace libfuture
{

	class LIBFUTURE_API noncopyable
	{
	protected:
		noncopyable() {}
		virtual ~noncopyable() {}

	private:
		noncopyable(const noncopyable&) = delete;
		void operator=(const noncopyable&) = delete;
	};

}

#endif
