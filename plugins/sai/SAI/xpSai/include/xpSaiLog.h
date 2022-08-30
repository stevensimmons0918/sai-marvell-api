// xpSaiLog.h

/*******************************************************************************
* copyright (c) 2021 marvell. all rights reserved. the following file is       *
* subject to the limited use license agreement by and between marvell and you, *
* your employer or other entity on behalf of whom you act. in the absence of   *
* such license agreement the following file is subject to marvell’s standard   *
* limited use license agreement.                                               *
********************************************************************************/

#ifndef _xpSaiLog_h
#define _xpSaiLog_h

#ifdef __cplusplus
extern "C" {
#endif

#include "sai.h"
#include "../experimental/saiextensions.h"

void xpSaiLogInit(const char *logFile);
void xpSaiLogDeInit(void);
void xpSaiLogSetLevel(sai_log_level_t level);

void xpSaiApiLog(const sai_api_t api, const sai_log_level_t logLevel,
                 const uint32_t line, const char *file, const char *funcName,
                 const char *msg, ...);

// For choosing logging engine: file or console
typedef enum _xpSaiLogDestT
{
    XP_SAI_LOG_FILE,
    XP_SAI_LOG_CONSOLE,
    XP_SAI_LOG_CALLBACK ///< for future use
} xpSaiLogDest_t;

#define XP_SAI_LOG_FILENAME "/var/log/xpliant-sai.log"
#define XP_XDK_LOG_FILENAME "/var/log/xpliant-xdk.log"

// Macro to define logging level applied on startup
#define XPSAI_DEFAULT_LOG_LEVEL SAI_LOG_LEVEL_WARN

// Macro to define log destination applied on startup
#define XPSAI_DEFAULT_LOG_DEST XP_SAI_LOG_CONSOLE

// Macro to register api on specific *.c file.
#define XP_SAI_LOG_REGISTER_API(apiId)                                          \
    static sai_api_t xpSaiLogApiDefined = apiId

// Macro to use api-defined logs, it requires to call XP_SAI_LOG_REGISTER_API first
// Example of usage:
//      XP_SAI_LOG(SAI_LOG_LEVEL_DEBUG, "some log, %s\n", "!!!");
//  It`s eqvivalent to use:
//      XP_SAI_LOG_DBG("some log\n");
#define XP_SAI_LOG(logLvl, msg, ...) {                                          \
    xpSaiApiLog(xpSaiLogApiDefined, logLvl, __LINE__, __FILE__, __FUNCTION__, msg,\
                ##__VA_ARGS__);                                                 \
}

#define XP_SAI_LOG_DBG(msg, ...) {                                              \
    XP_SAI_LOG(SAI_LOG_LEVEL_DEBUG, msg, ##__VA_ARGS__);}

#define XP_SAI_LOG_INFO(msg, ...) {                                             \
    XP_SAI_LOG(SAI_LOG_LEVEL_INFO, msg, ##__VA_ARGS__);}

#define XP_SAI_LOG_NOTICE(msg, ...) {                                           \
    XP_SAI_LOG(SAI_LOG_LEVEL_NOTICE, msg, ##__VA_ARGS__);}

#define XP_SAI_LOG_WARNING(msg, ...) {                                          \
    XP_SAI_LOG(SAI_LOG_LEVEL_WARN, msg, ##__VA_ARGS__);}

#define XP_SAI_LOG_ERR(msg, ...) {                                              \
    XP_SAI_LOG(SAI_LOG_LEVEL_ERROR, msg, ##__VA_ARGS__);}

#define XP_SAI_LOG_CRITICAL(msg, ...) {                                         \
    XP_SAI_LOG(SAI_LOG_LEVEL_CRITICAL, msg, ##__VA_ARGS__);}

// Macro to use api-defined logs, it`s not relay on api registered before.
//  It prints out log by manually defined api
// Example of usage:
//      XP_SAI_LOG_API_ERROR(SAI_API_SWITCH, "ERR from switch!!!\n");
// It`s equvivalent to use:
//      XP_SAI_LOG_API(SAI_API_SWITCH, SAI_LOG_LEVEL_DEBUG, "some debug msg!\n");
#define XP_SAI_LOG_API(api, logLvl, msg, ...) {                                 \
    xpSaiApiLog(api, logLvl, __LINE__, __FILE__, __FUNCTION__,                  \
                msg, ##__VA_ARGS__);                                            \
}

#define XP_SAI_LOG_API_DBG(api, msg, ...) {                                     \
    XP_SAI_LOG_API(api, SAI_LOG_LEVEL_DEBUG, msg, ##__VA_ARGS__);}

#define XP_SAI_LOG_API_INFO(api, msg, ...) {                                    \
    XP_SAI_LOG_API(api, SAI_LOG_LEVEL_INFO, msg, ##__VA_ARGS__);}

#define XP_SAI_LOG_API_NOTICE(api, msg, ...) {                                  \
    XP_SAI_LOG_API(api, SAI_LOG_LEVEL_NOTICE, msg, ##__VA_ARGS__);}

#define XP_SAI_LOG_API_WARNING(api, msg, ...) {                                 \
    XP_SAI_LOG_API(api, SAI_LOG_LEVEL_WARN, msg, ##__VA_ARGS__);}

#define XP_SAI_LOG_API_ERROR(api, msg, ...) {                                   \
    XP_SAI_LOG_API(api, SAI_LOG_LEVEL_ERROR, msg, ##__VA_ARGS__);}

#define XP_SAI_LOG_API_CRITICAL(api, msg, ...) {                                \
    XP_SAI_LOG_API(api, SAI_LOG_LEVEL_CRITICAL, msg, ##__VA_ARGS__);}


#ifdef __cplusplus
}
#endif

#endif // _xpSaiLog_h
