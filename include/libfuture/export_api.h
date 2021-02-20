#ifndef __EXPORT_API_H__
#define __EXPORT_API_H__

#define LIBFUTURE_EXPORT

//作为动态库导出宏
#ifdef _WIN32
#ifdef LIBFUTURE_EXPORT
#define LIBFUTURE_API __declspec(dllexport)
#else
#define LIBFUTURE_API __declspec(dllimport)
#endif
#else
#define LIBFUTURE_API
#endif

#endif
