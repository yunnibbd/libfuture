#ifndef __LIB_FUTURE_H__
#define __LIB_FUTURE_H__

#include "./libfuture/promise.h"
#include "./libfuture/future.h"
#include "./libfuture/awaitable.h"
#include "./libfuture/sleep.h"
#include "./libfuture/buffer.h"
#include "./libfuture/scheduler.h"
#include "./libfuture/socket.h"
#include "./libfuture/event.h"
#include "./libfuture/utils.h"

#ifndef cpp
#define cpp *current_scheduler()+
#endif

#ifndef FUTURE
#define FUTURE current_scheduler()
#endif

#endif
