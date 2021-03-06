﻿#ifndef __UTILST_H__
#define __UTILST_H__
#include "export_api.h"
#include <cinttypes>

namespace libfuture
{

	class LIBFUTURE_API utils_t
	{
	public:
		/**
		 * @brief 获得当前的时间戳
		 * @param
		 * @return
		 */
		static uint64_t get_cur_timestamp();

	private:
		utils_t() {}
	};

}

#endif
