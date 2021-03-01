#include "utils.h"
#include "include.h"
#include <chrono>
using namespace std;
using namespace chrono;
using namespace libfuture;

/**
 * @brief 获得当前的时间戳
 * @param
 * @return
 */
uint64_t utils_t::get_cur_timestamp()
{
	return duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
}
