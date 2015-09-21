#ifndef _FFHEADER_H_
#define _FFHEADER_H_

#include <string>

extern "C"
{
#define INT64_C
#define __STDC_LIMIT_MACROS
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
}


#ifdef FF_DLL // for win32 dll
#ifdef DLL_FILE
#   define FF_EXPORT _declspec(dllexport)
#else
#   define FF_EXPORT _declspec(dllimport)
#endif
#else
#   define FF_EXPORT
#endif


#endif // _FFHEADER_H_

