/* Copyright (C) 2018 Marvell International Ltd.
   This program is provided "as is" without any warranty of any kind, and is distributed under the applicable Marvell limited use license agreement. */

#ifndef __iDbgPdlLoggerApih

#define __iDbgPdlLoggerApih

/**********************************************************************************
 * @file loggerApi.h   
 * @copyright
 *    (c), Copyright 2001, Marvell International Ltd.
 *    THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.
 *    NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT
 *    OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE
 *    DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.
 *    THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,
 *    IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.
********************************************************************************
 * 
 * @brief Debug lib Logger definitions
 * 
 * @version   1 
********************************************************************************
*/
#include <pdl/common/pdlTypes.h>
#include <pdl/init/pdlInit.h>
#include <iDbgPdl/logger/private/prvIDbgPdlLogger.h>


#define IDBG_PDL_LOGGER_CMD_DEFINE_TEST_MAC(_category) (_category | IDBG_PDL_LOGGER_CATEGORY_LAST_E)
#define IDBG_PDL_LOGGER_CMD_IS_TEST_MAC(_category) (_category & IDBG_PDL_LOGGER_CATEGORY_LAST_E)
#define IDBG_PDL_LOGGER_CMD_CLEAR_TEST_FLAG_MAC(_category) (_category & (~(UINT_32)IDBG_PDL_LOGGER_CATEGORY_LAST_E))

#ifndef _WIN32
#ifndef snprintf
    /* #include <stdio.h>*/ /* not help the snprintf ?! */
    extern int snprintf ( char * s, size_t n, const char * format, ... );
#endif
#define PRV_IDBG_PDL_LOGGER_SNPRINTF_MAC    snprintf
#else
#define PRV_IDBG_PDL_LOGGER_SNPRINTF_MAC    _snprintf
#endif
/**
 * @enum    IDBG_PDL_LOGGER_CATEGORY_ENT
 *
 * @brief   Enumerator for pdl categories
 */
typedef enum {
        IDBG_PDL_LOGGER_CATEGORY_BUTTON_E    = (1 << 1),
        IDBG_PDL_LOGGER_CATEGORY_CPU_E       = (1 << 2),
        IDBG_PDL_LOGGER_CATEGORY_FAN_E       = (1 << 3),
        IDBG_PDL_LOGGER_CATEGORY_INTERFACE_E = (1 << 4),
        IDBG_PDL_LOGGER_CATEGORY_LED_E       = (1 << 5),
        IDBG_PDL_LOGGER_CATEGORY_OOB_E       = (1 << 6),
        IDBG_PDL_LOGGER_CATEGORY_PP_E        = (1 << 7),
        IDBG_PDL_LOGGER_CATEGORY_PHY_E       = (1 << 8),
        IDBG_PDL_LOGGER_CATEGORY_POWER_E     = (1 << 9),
        IDBG_PDL_LOGGER_CATEGORY_SENSOR_E    = (1 << 10),
        IDBG_PDL_LOGGER_CATEGORY_SERDES_E    = (1 << 11),
        IDBG_PDL_LOGGER_CATEGORY_SFP_E       = (1 << 12),
        IDBG_PDL_LOGGER_CATEGORY_SYSTEM_E    = (1 << 13),
        IDBG_PDL_LOGGER_CATEGORY_LAST_E      = (1 << 31)
} IDBG_PDL_LOGGER_CATEGORY_ENT;

/* START COMMAND SECTION */
#define IDBG_PDL_LOGGER_CMD_START(_cmd_line, _category, _result_stc) \
    do { \
        memset(&_result_stc, 0, sizeof(_result_stc)); \
        _result_stc.result = iDbgPdlLoggerCommandLogStart(_cmd_line, _category, __FUNCTION__)

/* END/CLOSE COMMAND SECTION */
#define IDBG_PDL_LOGGER_CMD_END(_result_stc) \
    } \
    while (0); \
    iDbgPdlLoggerCommandEnd(&_result_stc)

/* LOG DEBUGGER API OUTPUT  */
#define IDBG_PDL_LOGGER_API_OUTPUT_LOG(_format , ...) iDbgPdlLoggerApiOutDescriptionAdd(_format, ##__VA_ARGS__)

/* LOG DEBUGGER API OUTPUT  */
#define IDBG_PDL_LOGGER_PDL_DEBUG_LOG(_format , ...) iDbgPdlLoggerPdlDebugTextAdd(_format, ##__VA_ARGS__)

/* RUN DEBUGGER API - MUST BE USED WHEN SEVERAL APIs MUST BE RUNNED IN SUCCESSION */
#define IDBG_PDL_LOGGER_API_RUN(_result_stc, _api, ...) \
    _result_stc.outDescriptionPtr = NULL; \
    _result_stc.result = iDbgPdlLoggerApiRunStart(PRV_IDBG_PDL_LOGGER_STRINGIFY_MAC(_api), \
                                                  PRV_IDBG_PDL_LOG_API_LOG_ARG(__VA_ARGS__) \
                                                  PRV_IDBG_PDL_LOGGER_TYPEOF_MAC(LAST));\
    if (_result_stc.result != PDL_OK) \
        break; \
    _result_stc.result = _api(&_result_stc PRV_IDBG_PDL_LOG_API_CALL_ARG(__VA_ARGS__))

/* RUN DEBUGGER API - MUST BE USED WHEN SEVERAL APIs MUST BE RUNNED IN SUCCESSION */
#define IDBG_PDL_LOGGER_API_END(_result_stc, ...) \
    if (PDL_OK!= iDbgPdlLoggerApiResultLog(&_result_stc, \
                                           PRV_IDBG_PDL_LOG_API_LOG_ARG(__VA_ARGS__)   \
                                           PRV_IDBG_PDL_LOGGER_TYPEOF_MAC(LAST))) \
        break

/* RUN SINGLE DEBUGGER API - MUST BE USED WHEN THERE IS A SINGLE DEBUGGER API PER CLI COMMAND/TEST */
#define IDBG_PDL_LOGGER_API_EXECUTE(_cmd_line, _category, _result_stc, _api, ...) \
    IDBG_PDL_LOGGER_CMD_START(_cmd_line, _category, _result_stc); \
    if (_result_stc.result != PDL_OK) \
        break; \
    IDBG_PDL_LOGGER_API_RUN(_result_stc, _api, __VA_ARGS__); \
    IDBG_PDL_LOGGER_API_END(_result_stc, __VA_ARGS__); \
    IDBG_PDL_LOGGER_CMD_END(_result_stc)

/* LOG DEBUGGER API PARAMETERS  */
#define IDBG_PDL_LOGGER_API_IN_PARAM_MAC(_type, _param) \
    iDbgPdlLoggerApiParamLog(PRV_IDBG_PDL_LOGGER_STRINGIFY_MAC(_param), ((UINT_32)(PRV_IDBG_PDL_LOGGER_TYPEOF_MAC(_type))), &(_param))
#define IDBG_PDL_LOGGER_API_OUT_PARAM_MAC(_type, _param) \
    iDbgPdlLoggerApiParamLog(PRV_IDBG_PDL_LOGGER_STRINGIFY_MAC(_param), ((UINT_32)(IDBG_PDL_LOGGER_PARAM_OUT_PARAM_MAC(_type))), &(_param))
#define IDBG_PDL_LOGGER_API_OUT_HEX_PARAM_MAC(_type, _param) \
    iDbgPdlLoggerApiParamLog(PRV_IDBG_PDL_LOGGER_STRINGIFY_MAC(_param), ((UINT_32)(IDBG_PDL_LOGGER_PARAM_OUT_HEX_PARAM_MAC(_type))), &(_param))

/* RUN SINGLE DEBIUGGER API - MUST BE USED WHEN THERE IS A SINGLE DEBUGGER API PER CLI COMMAND/TEST */
#define IDBG_PDL_LOGGER_PDL_EXECUTE(_pdl_result_value, _api, ...) \
    IDBG_PDL_LOGGER_PDL_RUN(_pdl_result_value, _api, ## __VA_ARGS__); \
    IDBG_PDL_LOGGER_PDL_END(_pdl_result_value, __VA_ARGS__)

/* RUN SINGLE DEBIUGGER API - MUST BE USED WHEN THERE IS A SINGLE DEBUGGER API PER CLI COMMAND/TEST - NO RESULT LOGGING ON FAIL */
#define IDBG_PDL_LOGGER_IGNORE_RESULT_PDL_EXECUTE(_pdl_result_value, _api, ...) \
    IDBG_PDL_LOGGER_PDL_RUN(_pdl_result_value, _api, ## __VA_ARGS__); \
    IDBG_PDL_LOGGER_IGNORE_RESULT_PDL_END(_pdl_result_value, __VA_ARGS__)

/* RUN PDL API - MUST BE USED WHEN SEVERAL APIs MUST BE RUNNED IN SUCSESSION */
#define IDBG_PDL_LOGGER_PDL_RUN(_pdl_result_value, _api, ...) \
    _pdl_result_value = iDbgPdlLoggerPdlRunStart(PRV_IDBG_PDL_LOGGER_STRINGIFY_MAC(_api), \
                                                 PRV_IDBG_PDL_LOG_API_LOG_ARG(__VA_ARGS__) \
                                                 PRV_IDBG_PDL_LOGGER_TYPEOF_MAC(LAST));\
    if (_pdl_result_value == PDL_OK) \
        _pdl_result_value = _api(PRV_IDBG_PDL_LOG_API_CALL_NO_1ST_ARG(__VA_ARGS__))

/* LOG DEBUGGER ACTION API  */
#define IDBG_PDL_LOGGER_PDL_END(_pdl_result_value, ...)\
    iDbgPdlLoggerPdlResultLog(_pdl_result_value, \
                              PRV_IDBG_PDL_LOG_API_LOG_ARG(__VA_ARGS__)   \
                              PRV_IDBG_PDL_LOGGER_TYPEOF_MAC(LAST)); \
    if (_pdl_result_value != PDL_OK)\
        iDbgPdlLoggerApiOutDescriptionAdd("FUNCTION: %s LINE: %d ERROR CODE: %d\n", __FUNCTION__, __LINE__, _pdl_result_value)

/* LOG DEBUGGER ACTION API  */
#define IDBG_PDL_LOGGER_IGNORE_RESULT_PDL_END(_pdl_result_value, ...)\
    iDbgPdlLoggerPdlResultLog(_pdl_result_value, \
                              PRV_IDBG_PDL_LOG_API_LOG_ARG(__VA_ARGS__)   \
                              PRV_IDBG_PDL_LOGGER_TYPEOF_MAC(LAST))

/* LOG DEBUGGER ACTION PARAMETERS  */
#define IDBG_PDL_LOGGER_PDL_IN_PARAM_MAC(_type, _param) \
    iDbgPdlLoggerPdlParamLog(PRV_IDBG_PDL_LOGGER_STRINGIFY_MAC(_param), ((UINT_32)(PRV_IDBG_PDL_LOGGER_TYPEOF_MAC(_type))), &(_param))
#define IDBG_PDL_LOGGER_PDL_OUT_PARAM_MAC(_type, _param) \
    iDbgPdlLoggerPdlParamLog(PRV_IDBG_PDL_LOGGER_STRINGIFY_MAC(_param), ((UINT_32)(IDBG_PDL_LOGGER_PARAM_OUT_PARAM_MAC(_type))), &(_param))
#define IDBG_PDL_LOGGER_PDL_OUT_HEX_PARAM_MAC(_type, _param) \
    iDbgPdlLoggerPdlParamLog(PRV_IDBG_PDL_LOGGER_STRINGIFY_MAC(_param), ((UINT_32)(IDBG_PDL_LOGGER_PARAM_OUT_HEX_PARAM_MAC(_type))), &(_param))

#define IDBG_PDL_LOGGER_PARAM_TYPE_IS_IN_PARAM_MAC(type) (((type) & IDBG_PDL_LOGGER_PARAM_TYPE_OUT_CNS) == 0)
#define IDBG_PDL_LOGGER_PARAM_TYPE_IS_OUT_PARAM_MAC(type) ((type) & IDBG_PDL_LOGGER_PARAM_TYPE_OUT_CNS)
#define IDBG_PDL_LOGGER_PARAM_TYPE_IS_HEX_PARAM_MAC(type) (((type) & IDBG_PDL_LOGGER_PARAM_TYPE_HEX_CNS))
#define IDBG_PDL_LOGGER_PARAM_TYPE_FLAGS_CLEAR_MAC(type) ((type) & ~(IDBG_PDL_LOGGER_PARAM_TYPE_FLAGS_CNS))

#define IDBG_PDL_LOGGER_PARAM_TYPE_SET_IN_PARAM_MAC(type) (type)
#define IDBG_PDL_LOGGER_PARAM_TYPE_SET_OUT_PARAM_MAC(type) ((type) | IDBG_PDL_LOGGER_PARAM_TYPE_OUT_CNS)
#define IDBG_PDL_LOGGER_PARAM_TYPE_SET_HEX_PARAM_MAC(type) ((type) | IDBG_PDL_LOGGER_PARAM_TYPE_HEX_CNS)

#define IDBG_PDL_LOGGER_PARAM_HEX_PARAM_MAC(type) IDBG_PDL_LOGGER_PARAM_TYPE_SET_HEX_PARAM_MAC(PRV_IDBG_PDL_LOGGER_TYPEOF_MAC(type))
#define IDBG_PDL_LOGGER_PARAM_OUT_PARAM_MAC(type) IDBG_PDL_LOGGER_PARAM_TYPE_SET_OUT_PARAM_MAC(PRV_IDBG_PDL_LOGGER_TYPEOF_MAC(type))
#define IDBG_PDL_LOGGER_PARAM_OUT_HEX_PARAM_MAC(type) IDBG_PDL_LOGGER_PARAM_TYPE_SET_OUT_PARAM_MAC(IDBG_PDL_LOGGER_PARAM_TYPE_SET_OUT_PARAM_MAC(PRV_IDBG_PDL_LOGGER_TYPEOF_MAC(type)))

#define IDBG_PDL_LOGGER_PARAM_TYPE_OUT_CNS         (1 << 31)
#define IDBG_PDL_LOGGER_PARAM_TYPE_HEX_CNS         (1 << 30)
#define IDBG_PDL_LOGGER_PARAM_TYPE_FLAGS_CNS       (IDBG_PDL_LOGGER_PARAM_TYPE_OUT_CNS | IDBG_PDL_LOGGER_PARAM_TYPE_HEX_CNS)

#define IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(_type, _param) \
    ((UINT_32)(PRV_IDBG_PDL_LOGGER_TYPEOF_MAC(_type))) , \
    PRV_IDBG_PDL_LOGGER_STRINGIFY_MAC(_param), \
    _param
#define IDBG_PDL_LOGGER_ADD_IN_HEX_PARAM_MAC(_type, _param) \
    ((UINT_32)(IDBG_PDL_LOGGER_PARAM_TYPE_HEX_CNS | PRV_IDBG_PDL_LOGGER_TYPEOF_MAC(_type))) , \
    PRV_IDBG_PDL_LOGGER_STRINGIFY_MAC(_param), \
    _param
#define IDBG_PDL_LOGGER_ADD_OUT_PARAM_MAC(_type, _param) \
    ((UINT_32)(IDBG_PDL_LOGGER_PARAM_TYPE_OUT_CNS | PRV_IDBG_PDL_LOGGER_TYPEOF_MAC(_type))) , \
    PRV_IDBG_PDL_LOGGER_STRINGIFY_MAC(_param), \
    _param
#define IDBG_PDL_LOGGER_ADD_OUT_HEX_PARAM_MAC(_type, _param) \
    ((UINT_32)(IDBG_PDL_LOGGER_PARAM_TYPE_OUT_CNS | IDBG_PDL_LOGGER_PARAM_TYPE_HEX_CNS | PRV_IDBG_PDL_LOGGER_TYPEOF_MAC(_type))) , \
    PRV_IDBG_PDL_LOGGER_STRINGIFY_MAC(_param), \
    _param

#define IDBG_PDL_LOGGER_TYPEOF_UINT_8_CNS       0
#define IDBG_PDL_LOGGER_TYPEOF_UINT_16_CNS      1
#define IDBG_PDL_LOGGER_TYPEOF_UINT_32_CNS      2
#define IDBG_PDL_LOGGER_TYPEOF_BOOLEAN_CNS      3
#define IDBG_PDL_LOGGER_TYPEOF_CHAR_PTR_CNS     4
#define IDBG_PDL_LOGGER_TYPEOF_PTR_CNS          5
#define IDBG_PDL_LOGGER_TYPEOF_LAST_CNS         MAX_UINT_16

#define IDBG_PDL_LOGGER_TYPEOF_SIZE_MAC(_type_val)\
   ((_type_val == IDBG_PDL_LOGGER_TYPEOF_UINT_8_CNS)             ?  sizeof(UINT_8)               :\
    (_type_val == IDBG_PDL_LOGGER_TYPEOF_UINT_16_CNS)            ?  sizeof(UINT_16)              :\
    (_type_val == IDBG_PDL_LOGGER_TYPEOF_UINT_32_CNS)            ?  sizeof(UINT_32)              :\
    (_type_val == IDBG_PDL_LOGGER_TYPEOF_BOOLEAN_CNS)            ?  sizeof(BOOLEAN)              :\
    (_type_val == IDBG_PDL_LOGGER_TYPEOF_CHAR_PTR_CNS)           ?  sizeof(char *)               :\
    (_type_val == IDBG_PDL_LOGGER_TYPEOF_PTR_CNS)                ?  sizeof(void *)               :\
                                                                    0)
#define IDBG_PDL_LOGGER_TYPEOF_TEXT_MAC(_type_val)\
   ((_type_val == IDBG_PDL_LOGGER_TYPEOF_UINT_8_CNS)             ?  "UINT_8"                 :\
    (_type_val == IDBG_PDL_LOGGER_TYPEOF_UINT_16_CNS)            ?  "UINT_16"                :\
    (_type_val == IDBG_PDL_LOGGER_TYPEOF_UINT_32_CNS)            ?  "UINT_32"                :\
    (_type_val == IDBG_PDL_LOGGER_TYPEOF_BOOLEAN_CNS)            ?  "BOOLEAN"                :\
    (_type_val == IDBG_PDL_LOGGER_TYPEOF_CHAR_PTR_CNS)           ?  "string"                 :\
    (_type_val == IDBG_PDL_LOGGER_TYPEOF_PTR_CNS)                ?  "pointer"                :\
                                                                    "--UNKNOWN--")
typedef struct {
    PDL_STATUS      result;
    char          * outDescriptionPtr;
    BOOLEAN         successIsNegativeResult;
} IDBG_PDL_LOGGER_RESULT_DATA_STC;

extern PDL_STATUS iDbgPdlLoggerCommandLogStart(IN char * cliCommandPtr, IN IDBG_PDL_LOGGER_CATEGORY_ENT categoryType, IN const char * cliFuncNamePtr);
extern PDL_STATUS iDbgPdlLoggerCommandEnd(IN IDBG_PDL_LOGGER_RESULT_DATA_STC * resultDataPtr);
extern PDL_STATUS iDbgPdlLoggerApiRunStart(IN char * apiNamePtr, ... /* arg list  of type/value, ends with typeof(last) */ );
extern PDL_STATUS iDbgPdlLoggerApiResultLog(IN IDBG_PDL_LOGGER_RESULT_DATA_STC * resultDataPtr, ...);
extern PDL_STATUS iDbgPdlLoggerApiParamLog(IN char * paramNamePtr, IN UINT_32 paramType, IN void  * paramValuePtr);
extern PDL_STATUS iDbgPdlLoggerApiOutDescriptionAdd(IN char * format, ...);
extern PDL_STATUS iDbgPdlLoggerPdlRunStart(IN char * apiNamePtr, ... /* arg list  of type/value, ends with typeof(last) */ );
extern PDL_STATUS iDbgPdlLoggerPdlParamLog(IN char * paramNamePtr, IN UINT_32 paramType, IN void  * paramValuePtr);
extern PDL_STATUS iDbgPdlLoggerPdlAnyResultLog(IN char * resultValueStringPtr, IN UINT_32 * resultValueIntPtr);
extern PDL_STATUS iDbgPdlLoggerPdlResultLog(IN PDL_STATUS resulltValue, ...);
extern PDL_STATUS iDbgPdlLoggerPdlDebugTextAdd(IN char * format, ...);

extern PDL_STATUS iDbgPdlLoggerInit(IN PDL_OS_CALLBACK_API_STC *callbacksPtr, IN char *rootPathPtr);
extern PDL_STATUS iDbgPdlLoggerShowResults(IN BOOLEAN showCurrentOnly);

#endif /* __iDbgPdlLoggerApih */
