#ifndef _LOGTRACE_H_
#define _LOGTRACE_H_

#ifdef ANDROID
#include <android/log.h>

#define FFTAG "FFCODEC"

#define LOGD(...) ((void)__android_log_print(ANDROID_LOG_DEBUG, FFTAG, __VA_ARGS__))
#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, FFTAG, __VA_ARGS__))
#define LOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN, FFTAG, __VA_ARGS__))
#define LOGE(...) ((void)__android_log_print(ANDROID_LOG_ERROR, FFTAG, __VA_ARGS__))
#else
#define LOGD(...) {fprintf(stdout, __VA_ARGS__); fprintf(stdout, "\n");}
#define LOGI(...) {fprintf(stdout, __VA_ARGS__); fprintf(stdout, "\n");}
#define LOGW(...) {fprintf(stdout, __VA_ARGS__); fprintf(stdout, "\n");}
#define LOGE(...) {fprintf(stdout, __VA_ARGS__); fprintf(stdout, "\n");}
#endif

#endif
