#ifndef __INCLUDE_H__
#define __INCLUDE_H__

#ifdef _WIN32 
#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define IOVEC_TYPE WSABUF
typedef unsigned long socket_unread_t;
#include <WS2tcpip.h>
#include <Windows.h>
#include <WinSock2.h>
#include <process.h>
#include <MSWSock.h>
#include <ctime>
#pragma comment(lib, "ws2_32.lib")
#else
typedef int socket_unread_t;
#define INFINITE -1
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <signal.h>
#include <pthread.h>
#include <sys/uio.h>
#endif

#include <cstring>
#include <cstdlib>
#include <cstdio>

#endif
