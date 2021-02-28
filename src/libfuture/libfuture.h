#ifndef __LIB_FUTURE_H__
#define __LIB_FUTURE_H__

#include "promise.h"
#include "future.h"
#include "awaitable.h"
#include "sleep.h"
#include "buffer.h"
#include "scheduler.h"
#include "socket.h"
#include "event.h"
#include "utils.h"

namespace libfuture
{

#ifndef cpp
#define cpp *current_scheduler()+
#endif

#ifndef FUTURE
#define FUTURE current_scheduler()
#endif
}

#endif
