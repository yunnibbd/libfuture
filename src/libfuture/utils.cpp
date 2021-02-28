#include "utils.h"
#include <chrono>
#ifdef _WIN32
#include <Windows.h>
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>
#endif
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
