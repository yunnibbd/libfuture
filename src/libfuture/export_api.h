#ifndef __EXPORT_API_H__
#define __EXPORT_API_H__

//#define CEXPORT

//作为动态库导出宏
#ifdef _WIN32
#ifdef CEXPORT
#define LIBFUTURE_API __declspec(dllexport)
#else
#define LIBFUTURE_API /*__declspec(dllimport)*/
#endif
#else
#define LIBFUTURE_API
#endif

#endif
