#ifndef __FFLOG_H_
#define __FFLOG_H_

#include <sstream>

#ifdef ANDROID
#include <android/log.h>
#endif


// enable/disable log
#define FF_ENABLE_LOG 0 

#ifndef return_if_fail
#define return_if_fail(p) do{if(!(p)) return;}while(0)
#endif

#ifndef returnv_if_fail
#define returnv_if_fail(p, v) do{if(!(p)) return(v);}while(0)
#endif

#ifndef safe_delete
#define safe_delete(p) do{if(p) {delete (p); (p) = NULL;}}while(0)
#endif


enum {
    FF_LOG_DEBUG    = 0x01,
    FF_LOG_INFO     = 0x02,
    FF_LOG_WARN     = 0x04,
    FF_LOG_ERROR    = 0x08,
};

#ifndef __FUNC__
#if defined (__GNUC__)  
#  define __FUNC__     ((const char*) (__PRETTY_FUNCTION__))  
#elif defined (__STDC_VERSION__) && __STDC_VERSION__ >= 19901L  
#  define __FUNC__     ((const char*) (__func__))  
#else  
#  define __FUNC__     ((const char*) (__FUNCTION__))  
#endif
#endif

#if FF_ENABLE_LOG
#  define __FF_TAG "FFCODEC"
#  define LOGD(str)   __LOG_PRINT(FF_LOG_DEBUG, __FF_TAG, str)
#  define LOGI(str)   __LOG_PRINT(FF_LOG_INFO, __FF_TAG, str)
#  define LOGW(str)   __LOG_PRINT(FF_LOG_WARN, __FF_TAG, str)
#  define LOGE(str)   __LOG_PRINT(FF_LOG_ERROR, __FF_TAG, str)
#  define __LOG_PRINT(level, tag, str) \
    do { \
        std::stringstream ssteam; \
        sstream << __FUNC__ << "[#" << __LINE__ << "]: "; \
        sstream << str; \
        __ff_log_print(level, tag, ssteam.str()); \
    }while(0)
#else
#  define LOGD(str)
#  define LOGI(str)
#  define LOGW(str)
#  define LOGE(str)
#endif

void __ff_log_print(int level, const char *tag, const std::string &str) {
#ifdef ANDROID
    int androidLevel = ANDROID_LOG_DEBUG;
    switch(level) { 
        case FF_LOG_DEBUG: 
            androidLevel = ANDROID_LOG_DEBUG;
            break; 
        case FF_LOG_INFO: 
            androidLevel = ANDROID_LOG_INFO;
            break; 
        case FF_LOG_WARN: 
            androidLevel = ANDROID_LOG_WARN;
            break; 
        case FF_LOG_ERROR: 
            androidLevel = ANDROID_LOG_ERROR;
            break; 
        default:
            return;
    }
    __android_log_write(androidLevel, tag, str.c_str());
#else
    std::string szHead = "";
    switch(level) { 
        case FF_LOG_DEBUG: 
            szHead = "DEBUG";
            break; 
        case FF_LOG_INFO: 
            szHead = "INFO";
            break; 
        case FF_LOG_WARN: 
            szHead = "WARN";
            break; 
        case FF_LOG_ERROR: 
            szHead = "ERROR";
            break; 
        default:
            return;
    }
    fprintf(stdout, "[%s][%s] %s\n", tag, szHead.c_str(), str.c_str());
#endif
}


#endif // __FFLOG_H_

