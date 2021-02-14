#include "utils.h"
#include <chrono>
using namespace std;
using namespace chrono;

uint64_t utils_t::get_cur_timestamp()
{
	return duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
}
