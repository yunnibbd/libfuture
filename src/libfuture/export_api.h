#ifndef __EXPORT_API_H__
#define __EXPORT_API_H__

//#define CEXPORT

//作为动态库导出宏
#ifdef _WIN32
#ifdef CEXPORT
#define COMMON_EXPORT __declspec(dllexport)
#else
#define COMMON_EXPORT
#endif
#else
#define COMMON_EXPORT
#endif

#endif
