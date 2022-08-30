/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
*/
/**
********************************************************************************
* @file cpssLog.c
*
* @brief CPSS log implementation.
*
* @version   1
********************************************************************************
*/
#include <cpss/generic/log/cpssLog.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>
#include <cpss/generic/log/prvCpssLog.h>
#include <cpss/generic/log/prvCpssGenCommonTypesLog.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpssCommon/cpssPresteraDefs.h>
#include <cpssCommon/cpssBuffManagerPool.h>

#define PRV_CPSS_LOG_NOT_VALID_CNS   0xFFFFFFFF

#define PRV_LOG_GLOBAL_NON_VOLATILE_VAR_DIRECT_GET(_var) \
PRV_NON_SHARED_GLOBAL_VAR_DIRECT_GET(nonVolatileDb.cpssLogDb._var)

/* get Global Volatile DB member for Log */
#define PRV_LOG_GLOBAL_VAR_VOLATILE_GET(_var) \
PRV_NON_SHARED_GLOBAL_VAR_DIRECT_GET(commonMod.logDir._var)

/************************** Internal definitions  *****************************/

/*******************************************************************************
* prvCpssLogTabs
*
* DESCRIPTION:
*       Log tab for recursive structures
*
* INPUTS:
*       contextLib -
*           lib log activity
*       logType -
*           type of the log
*
* OUTPUTS:
*       None.
*
* RETURNS :
*       None.
*
* COMMENTS:
*       None.
*
*******************************************************************************/
void prvCpssLogTabs
(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType
);

/************************** Variables definitions *****************************/

/* Maximal number of threads from where log functions are called */
#define PRV_CPSS_LOG_MAX_THREADS_CNS     40

/* Time formats */
#define CPSS_LOG_TIME_FORMAT_DATE_TIME_CNS              "%c"
#define CPSS_LOG_TIME_FORMAT_SHORT_DATE_TIME_CNS        "%m/%d/%y  %I:%M:%S %p"
#define CPSS_LOG_TIME_FORMAT_SHORT_DATE_ISO_TIME_CNS    "%m/%d/%y  %H:%M:%S"

/* Log numeric format */
const GT_CHAR *prvCpssLogFormatNumFormat            = "%s = %d";

/* String constants */
const GT_CHAR *prvCpssLogErrorMsgDeviceNotExist     = "PRV_CPSS_DEV_CHECK_MAC: Device %d not exist.\n";
const GT_CHAR *prvCpssLogErrorMsgFdbIndexOutOfRange = "PRV_CPSS_DXCH_FDB_INDEX_CHECK_MAC: Device %d, FDB index %d is out of range\n";
const GT_CHAR *prvCpssLogErrorMsgPortGroupNotValid  = "PRV_CPSS_DRV_CHECK_PORT_GROUP_ID_MAC: Multi-port group device %d, port group %d not valid\n";
const GT_CHAR *prvCpssLogErrorMsgPortGroupNotActive = "PRV_CPSS_DRV_CHECK_ACTIVE_PORT_GROUP_ID_FOR_READ_OPERATION_MAC: Multi-port group device %d, port group %d not active\n";
const GT_CHAR *prvCpssLogErrorMsgIteratorNotValid   = "__MAX_NUM_ITERATIONS_CHECK_CNS: Device %d, current iterator %d not valid\n";
const GT_CHAR *prvCpssLogErrorMsgGlobalIteratorNotValid   = "__MAX_NUM_ITERATIONS_CHECK_CNS: current iterator %d not valid\n";
const GT_CHAR *prvCpssLogErrorMsgPortGroupBitmapNotValid
                                                    = "PRV_CPSS_MULTI_PORT_GROUPS_BMP_CHECK_MAC: Multi-port group device %d, port group bitmap %d not valid \n";
const GT_CHAR *prvCpssLogErrorMsgDeviceNotInitialized
                                                    = "PRV_CPSS_DRV_CHECK_PP_DEV_MAC: Device %d not initialized.\n";
const GT_CHAR *prvCpssLogErrorMsgLogContextCurrentPointer
                                                    = "Unable to get pointer to current log context. rc = %d\n";

/* Log API parameters and save history in database */
#define PRV_CPSS_LOG_AND_HISTORY_MAC(log_phase, log_lib, log_type, ...) \
    prvCpssLogAndHistory(log_phase, log_lib, log_type, __VA_ARGS__)


/******************************************************************************/
#ifdef SHARED_MEMORY


/**
* @internal prvLogCbDoOperation function
* @endinternal
*
* @brief   Execute single operation
*/
static void prvLogCbDoOperation(void)
{
    PRV_CPSS_LOG_API_HISTORY_ENTRY_STC *entry;

    entry = (PRV_CPSS_LOG_API_HISTORY_ENTRY_STC*)PRV_LOG_GLOBAL_VAR_VOLATILE_GET(prvLogCbDataPtr);
    switch (PRV_LOG_GLOBAL_VAR_VOLATILE_GET(prvLogCbOperation))
    {
        case PRV_LOG_CB_OPERATION_OS_LOG_E:
            /* Required data:
             *      prvLogCbLib
             *      prvLogCbType
             *      prvLogCbDataPtr   - string pointer
             */
            cpssOsLog(PRV_LOG_GLOBAL_VAR_VOLATILE_GET(prvLogCbLib), PRV_LOG_GLOBAL_VAR_VOLATILE_GET(prvLogCbType), (const char*)PRV_LOG_GLOBAL_VAR_VOLATILE_GET(prvLogCbDataPtr));
            break;
        case PRV_LOG_CB_OPERATION_FILE_OPEN_R_E:
            /* Required data:
             *      prvLogCbDataPtr   - PRV_CPSS_LOG_API_HISTORY_ENTRY_STC*
             */
            cpssOsFopen(entry->entryHistoryFileName, "r", &entry->entryHistoryFile);
            break;
        case PRV_LOG_CB_OPERATION_FILE_OPEN_W_E:
            /* Required data:
             *      prvLogCbDataPtr   - PRV_CPSS_LOG_API_HISTORY_ENTRY_STC*
             */
            cpssOsFopen(entry->entryHistoryFileName, "w", &entry->entryHistoryFile);
            break;
        case PRV_LOG_CB_OPERATION_FILE_CLOSE_E:
            /* Required data:
             *      prvLogCbDataPtr   - PRV_CPSS_LOG_API_HISTORY_ENTRY_STC*
             */
            cpssOsFclose(&entry->entryHistoryFile);
            break;
        case PRV_LOG_CB_OPERATION_FILE_REWIND_E:
            /* Required data:
             *      prvLogCbDataPtr   - PRV_CPSS_LOG_API_HISTORY_ENTRY_STC*
             */
            cpssOsRewind(entry->entryHistoryFile.fd);
            break;
        case PRV_LOG_CB_OPERATION_FILE_WRITE_E:
            /* Required data:
             *      prvLogCbDataPtr   - string pointer
             */
            cpssOsFprintf(PRV_LOG_GLOBAL_VAR_VOLATILE_GET(currentHistoryEntryPtr)->entryHistoryFile.fd, "%s", (const char*)PRV_LOG_GLOBAL_VAR_VOLATILE_GET(prvLogCbDataPtr));
            break;
        case PRV_LOG_CB_OPERATION_FILE_READ_E:
            /* Required data:
             *      prvLogCbLib
             *      prvLogCbDataPtr   - PRV_CPSS_LOG_API_HISTORY_ENTRY_STC*
             */
            /* Print history file */
            while (cpssOsFgets(PRV_LOG_GLOBAL_VAR_VOLATILE_GET(buffer), 1024, entry->entryHistoryFile.fd) != 0)
            {
                cpssOsLog(PRV_LOG_GLOBAL_VAR_VOLATILE_GET(prvLogCbLib), CPSS_LOG_TYPE_ERROR_E, PRV_LOG_GLOBAL_VAR_VOLATILE_GET(buffer));
            }
            break;
    }
}

/*******************************************************************************
* prvLogCbHandlerTask
*
* DESCRIPTION:
*       Log task. Manages operations issued by secondary applications
*       like luaCLI
*
* INPUTS:
*       None.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       None.
*
* COMMENTS:
*       None.
*
*******************************************************************************/
static unsigned __TASKCONV prvLogCbHandlerTask(void *param GT_UNUSED)
{
    while (1)
    {
        cpssOsSigSemWait(PRV_LOG_GLOBAL_NON_VOLATILE_VAR_DIRECT_GET(prvLogCbHandleSem), CPSS_OS_SEM_WAIT_FOREVER_CNS);
        prvLogCbDoOperation();
        cpssOsSigSemSignal(PRV_LOG_GLOBAL_NON_VOLATILE_VAR_DIRECT_GET(prvLogCbHandleDoneSem));
    }
    return 0;
}
/**
* @internal prvLogCbInit function
* @endinternal
*
* @brief   Init callback task data and create log task to handle operations
*         issued by secondary applications like luaCLI
*/
static void prvLogCbInit(void)
{
    cpssOsMutexCreate("prvLogCbMtx", &PRV_LOG_GLOBAL_NON_VOLATILE_VAR_DIRECT_GET(prvLogCbMtx));
    cpssOsSigSemBinCreate("prvLogCbHandleSem", CPSS_OS_SEMB_EMPTY_E, &PRV_LOG_GLOBAL_NON_VOLATILE_VAR_DIRECT_GET(prvLogCbHandleSem));
    cpssOsSigSemBinCreate("prvLogCbHandleDoneSem", CPSS_OS_SEMB_EMPTY_E, &PRV_LOG_GLOBAL_NON_VOLATILE_VAR_DIRECT_GET(prvLogCbHandleDoneSem));
    PRV_LOG_GLOBAL_NON_VOLATILE_VAR_DIRECT_GET(prvLogCbProcessId) = cpssOsTaskGetPid();
    cpssOsTaskCreate("prvLogCb", 10/*prio*/, 0x10000/*stack*/,
            prvLogCbHandlerTask, NULL, &PRV_LOG_GLOBAL_NON_VOLATILE_VAR_DIRECT_GET(prvLogCbHandlerTaskId));
}

/**
* @internal prvLogExecuteOperation function
* @endinternal
*
* @brief   Execute single log operation.
*         Use direct call when called from the same process where cpssOsLog bound
*         Otherwise wakeup log handler task
*/
static void prvLogExecuteOperation(void)
{
    if (cpssOsTaskGetPid() == PRV_LOG_GLOBAL_NON_VOLATILE_VAR_DIRECT_GET(prvLogCbProcessId))
    {
        prvLogCbDoOperation();
    }
    /* reset sem */
    cpssOsSigSemWait(PRV_LOG_GLOBAL_NON_VOLATILE_VAR_DIRECT_GET(prvLogCbHandleDoneSem), CPSS_OS_SEM_NO_WAIT_CNS);
    cpssOsSigSemSignal(PRV_LOG_GLOBAL_NON_VOLATILE_VAR_DIRECT_GET(prvLogCbHandleSem));
    cpssOsSigSemWait(PRV_LOG_GLOBAL_NON_VOLATILE_VAR_DIRECT_GET(prvLogCbHandleDoneSem), CPSS_OS_SEM_WAIT_FOREVER_CNS);
}

static void prvLogCbOsLog(
    IN CPSS_LOG_LIB_ENT     lib,
    IN CPSS_LOG_TYPE_ENT    type,
    IN const char *         format,
    ...
)
{
    va_list args;
    cpssOsMutexLock(PRV_LOG_GLOBAL_NON_VOLATILE_VAR_DIRECT_GET(prvLogCbMtx));
    /* format */
    if (format != PRV_LOG_GLOBAL_VAR_VOLATILE_GET(buffer))
    {
        va_start(args, format);
        cpssOsVsprintf(PRV_LOG_GLOBAL_VAR_VOLATILE_GET(buffer), format, args);
        va_end(args);
    }

    PRV_LOG_GLOBAL_VAR_VOLATILE_GET(prvLogCbOperation) = PRV_LOG_CB_OPERATION_OS_LOG_E;
    PRV_LOG_GLOBAL_VAR_VOLATILE_GET(prvLogCbLib) = lib;
    PRV_LOG_GLOBAL_VAR_VOLATILE_GET(prvLogCbType) = type;
    PRV_LOG_GLOBAL_VAR_VOLATILE_GET(prvLogCbDataPtr) = (void*)PRV_LOG_GLOBAL_VAR_VOLATILE_GET(buffer);
    prvLogExecuteOperation();
    cpssOsMutexUnlock(PRV_LOG_GLOBAL_NON_VOLATILE_VAR_DIRECT_GET(prvLogCbMtx));
}

static void prvLogCbFileOpen(
    IN PRV_CPSS_LOG_API_HISTORY_ENTRY_STC *entry,
    IN GT_BOOL  write
)
{
    cpssOsMutexLock(PRV_LOG_GLOBAL_NON_VOLATILE_VAR_DIRECT_GET(prvLogCbMtx));
    PRV_LOG_GLOBAL_VAR_VOLATILE_GET(prvLogCbOperation) = (write == GT_TRUE)
            ? PRV_LOG_CB_OPERATION_FILE_OPEN_W_E
            : PRV_LOG_CB_OPERATION_FILE_OPEN_R_E;
    PRV_LOG_GLOBAL_VAR_VOLATILE_GET(prvLogCbDataPtr) = (void*)entry;
    prvLogExecuteOperation();
    cpssOsMutexUnlock(PRV_LOG_GLOBAL_NON_VOLATILE_VAR_DIRECT_GET(prvLogCbMtx));
}

static void prvLogCbFileClose(
    IN PRV_CPSS_LOG_API_HISTORY_ENTRY_STC *entry
)
{
    cpssOsMutexLock(PRV_LOG_GLOBAL_NON_VOLATILE_VAR_DIRECT_GET(prvLogCbMtx));
    PRV_LOG_GLOBAL_VAR_VOLATILE_GET(prvLogCbOperation) = PRV_LOG_CB_OPERATION_FILE_CLOSE_E;
    PRV_LOG_GLOBAL_VAR_VOLATILE_GET(prvLogCbDataPtr) = (void*)entry;
    prvLogExecuteOperation();
    cpssOsMutexUnlock(PRV_LOG_GLOBAL_NON_VOLATILE_VAR_DIRECT_GET(prvLogCbMtx));
}

static void prvLogCbFileRewind(
    IN PRV_CPSS_LOG_API_HISTORY_ENTRY_STC *entry
)
{
    cpssOsMutexLock(PRV_LOG_GLOBAL_NON_VOLATILE_VAR_DIRECT_GET(prvLogCbMtx));
    PRV_LOG_GLOBAL_VAR_VOLATILE_GET(prvLogCbOperation) = PRV_LOG_CB_OPERATION_FILE_REWIND_E;
    PRV_LOG_GLOBAL_VAR_VOLATILE_GET(prvLogCbDataPtr) = (void*)entry;
    prvLogExecuteOperation();
    cpssOsMutexUnlock(PRV_LOG_GLOBAL_NON_VOLATILE_VAR_DIRECT_GET(prvLogCbMtx));
}

static void prvLogCbFileWrite(
    IN const char *str
)
{
    cpssOsMutexLock(PRV_LOG_GLOBAL_NON_VOLATILE_VAR_DIRECT_GET(prvLogCbMtx));
    PRV_LOG_GLOBAL_VAR_VOLATILE_GET(prvLogCbOperation) = PRV_LOG_CB_OPERATION_FILE_WRITE_E;
    PRV_LOG_GLOBAL_VAR_VOLATILE_GET(prvLogCbDataPtr) = (void*)str;
    prvLogExecuteOperation();
    cpssOsMutexUnlock(PRV_LOG_GLOBAL_NON_VOLATILE_VAR_DIRECT_GET(prvLogCbMtx));
}

static void prvLogCbFileRead(
    IN PRV_CPSS_LOG_API_HISTORY_ENTRY_STC *entry,
    IN CPSS_LOG_LIB_ENT     lib
)
{
    cpssOsMutexLock(PRV_LOG_GLOBAL_NON_VOLATILE_VAR_DIRECT_GET(prvLogCbMtx));
    PRV_LOG_GLOBAL_VAR_VOLATILE_GET(prvLogCbOperation) = PRV_LOG_CB_OPERATION_FILE_REWIND_E;
    PRV_LOG_GLOBAL_VAR_VOLATILE_GET(prvLogCbDataPtr) = (void*)entry;
    PRV_LOG_GLOBAL_VAR_VOLATILE_GET(prvLogCbLib) = lib;
    prvLogExecuteOperation();
    cpssOsMutexUnlock(PRV_LOG_GLOBAL_NON_VOLATILE_VAR_DIRECT_GET(prvLogCbMtx));
}

#ifdef cpssOsLog
#undef cpssOsLog
#define cpssOsLog prvLogCbOsLog
#endif

#endif /* SHARED_MEMORY */


/******************************************************************************/
#define PRV_CPSS_LOG_TYPE_GET(function_id) \
    (PRV_LOG_GLOBAL_NON_VOLATILE_VAR_DIRECT_GET(prvCpssLogContextEntryPtr)->prvCpssLogEntryFuncId == function_id) ? CPSS_LOG_TYPE_ENTRY_LEVEL_FUNCTION_E : CPSS_LOG_TYPE_NON_ENTRY_LEVEL_FUNCTION_E;


#define PRV_CPSS_LOG_PARAM_LIB_DB_DX(lib_name) \
    prvCpssLogParamLibDbGet_DX_##lib_name

#define PRV_CPSS_LOG_PARAM_LIB_DB_PX(lib_name) \
    prvCpssLogParamLibDbGet_PX_##lib_name


/* Get API log entries database from specific lib */
#define DECLARE_EXTERNAL_LOG_PARAM_LIB_DB_GET(lib_name) \
extern void PRV_CPSS_LOG_PARAM_LIB_DB_DX(lib_name) \
( \
    OUT const PRV_CPSS_LOG_FUNC_ENTRY_STC ** logFuncDbPtrPtr, \
    OUT GT_U32 * logFuncDbSizePtr \
);\
extern void PRV_CPSS_LOG_PARAM_LIB_DB_PX(lib_name) \
( \
    OUT const PRV_CPSS_LOG_FUNC_ENTRY_STC ** logFuncDbPtrPtr, \
    OUT GT_U32 * logFuncDbSizePtr \
);

DECLARE_EXTERNAL_LOG_PARAM_LIB_DB_GET(NOT_DEFINED);
DECLARE_EXTERNAL_LOG_PARAM_LIB_DB_GET(CPSS_LOG_LIB_APP_DRIVER_CALL);
DECLARE_EXTERNAL_LOG_PARAM_LIB_DB_GET(CPSS_LOG_LIB_BRIDGE);
DECLARE_EXTERNAL_LOG_PARAM_LIB_DB_GET(CPSS_LOG_LIB_PCL);
DECLARE_EXTERNAL_LOG_PARAM_LIB_DB_GET(CPSS_LOG_LIB_TRUNK);
DECLARE_EXTERNAL_LOG_PARAM_LIB_DB_GET(CPSS_LOG_LIB_TTI);
DECLARE_EXTERNAL_LOG_PARAM_LIB_DB_GET(CPSS_LOG_LIB_TUNNEL);
DECLARE_EXTERNAL_LOG_PARAM_LIB_DB_GET(CPSS_LOG_LIB_PROTECTION);
DECLARE_EXTERNAL_LOG_PARAM_LIB_DB_GET(CPSS_LOG_LIB_PTP);
DECLARE_EXTERNAL_LOG_PARAM_LIB_DB_GET(CPSS_LOG_LIB_SYSTEM_RECOVERY);
DECLARE_EXTERNAL_LOG_PARAM_LIB_DB_GET(CPSS_LOG_LIB_TCAM);
DECLARE_EXTERNAL_LOG_PARAM_LIB_DB_GET(CPSS_LOG_LIB_EXACT_MATCH);
DECLARE_EXTERNAL_LOG_PARAM_LIB_DB_GET(CPSS_LOG_LIB_VERSION);
DECLARE_EXTERNAL_LOG_PARAM_LIB_DB_GET(CPSS_LOG_LIB_VNT);
DECLARE_EXTERNAL_LOG_PARAM_LIB_DB_GET(CPSS_LOG_LIB_PHY);
DECLARE_EXTERNAL_LOG_PARAM_LIB_DB_GET(CPSS_LOG_LIB_POLICER);
DECLARE_EXTERNAL_LOG_PARAM_LIB_DB_GET(CPSS_LOG_LIB_HW_INIT);
DECLARE_EXTERNAL_LOG_PARAM_LIB_DB_GET(CPSS_LOG_LIB_CONFIG);
DECLARE_EXTERNAL_LOG_PARAM_LIB_DB_GET(CPSS_LOG_LIB_COS);
DECLARE_EXTERNAL_LOG_PARAM_LIB_DB_GET(CPSS_LOG_LIB_CSCD);
DECLARE_EXTERNAL_LOG_PARAM_LIB_DB_GET(CPSS_LOG_LIB_CUT_THROUGH);
DECLARE_EXTERNAL_LOG_PARAM_LIB_DB_GET(CPSS_LOG_LIB_CNC);
DECLARE_EXTERNAL_LOG_PARAM_LIB_DB_GET(CPSS_LOG_LIB_DIAG);
DECLARE_EXTERNAL_LOG_PARAM_LIB_DB_GET(CPSS_LOG_LIB_PORT);

#if defined (INCLUDE_TM)
DECLARE_EXTERNAL_LOG_PARAM_LIB_DB_GET(CPSS_LOG_LIB_TM);
DECLARE_EXTERNAL_LOG_PARAM_LIB_DB_GET(CPSS_LOG_LIB_TM_GLUE);
#endif /*#if defined (INCLUDE_TM)*/

DECLARE_EXTERNAL_LOG_PARAM_LIB_DB_GET(CPSS_LOG_LIB_SMI);
DECLARE_EXTERNAL_LOG_PARAM_LIB_DB_GET(CPSS_LOG_LIB_I2C);
DECLARE_EXTERNAL_LOG_PARAM_LIB_DB_GET(CPSS_LOG_LIB_INIT);
DECLARE_EXTERNAL_LOG_PARAM_LIB_DB_GET(CPSS_LOG_LIB_NETWORK_IF);
DECLARE_EXTERNAL_LOG_PARAM_LIB_DB_GET(CPSS_LOG_LIB_NST);
DECLARE_EXTERNAL_LOG_PARAM_LIB_DB_GET(CPSS_LOG_LIB_OAM);
DECLARE_EXTERNAL_LOG_PARAM_LIB_DB_GET(CPSS_LOG_LIB_IP);
DECLARE_EXTERNAL_LOG_PARAM_LIB_DB_GET(CPSS_LOG_LIB_LPM);
DECLARE_EXTERNAL_LOG_PARAM_LIB_DB_GET(CPSS_LOG_LIB_IP_LPM);
DECLARE_EXTERNAL_LOG_PARAM_LIB_DB_GET(CPSS_LOG_LIB_MIRROR);
DECLARE_EXTERNAL_LOG_PARAM_LIB_DB_GET(CPSS_LOG_LIB_RESOURCE_MANAGER);
DECLARE_EXTERNAL_LOG_PARAM_LIB_DB_GET(CPSS_LOG_LIB_IPFIX);
DECLARE_EXTERNAL_LOG_PARAM_LIB_DB_GET(CPSS_LOG_LIB_FABRIC);
DECLARE_EXTERNAL_LOG_PARAM_LIB_DB_GET(CPSS_LOG_LIB_MULTI_PORT_GROUP);
DECLARE_EXTERNAL_LOG_PARAM_LIB_DB_GET(CPSS_LOG_LIB_DRAGONITE);
DECLARE_EXTERNAL_LOG_PARAM_LIB_DB_GET(CPSS_LOG_LIB_L2_MLL);
DECLARE_EXTERNAL_LOG_PARAM_LIB_DB_GET(CPSS_LOG_LIB_LOGICAL_TARGET);
DECLARE_EXTERNAL_LOG_PARAM_LIB_DB_GET(CPSS_LOG_LIB_VIRTUAL_TCAM);
DECLARE_EXTERNAL_LOG_PARAM_LIB_DB_GET(CPSS_LOG_LIB_INGRESS);
DECLARE_EXTERNAL_LOG_PARAM_LIB_DB_GET(CPSS_LOG_LIB_EGRESS);
DECLARE_EXTERNAL_LOG_PARAM_LIB_DB_GET(CPSS_LOG_LIB_HW_INIT_COMMON);
DECLARE_EXTERNAL_LOG_PARAM_LIB_DB_GET(CPSS_LOG_LIB_CONFIG_COMMON);
DECLARE_EXTERNAL_LOG_PARAM_LIB_DB_GET(CPSS_LOG_LIB_SYSTEM_RECOVERY_COMMON);
DECLARE_EXTERNAL_LOG_PARAM_LIB_DB_GET(CPSS_LOG_LIB_LATENCY_MONITORING);
DECLARE_EXTERNAL_LOG_PARAM_LIB_DB_GET(CPSS_LOG_LIB_PHA);
DECLARE_EXTERNAL_LOG_PARAM_LIB_DB_GET(CPSS_LOG_LIB_PACKET_ANALYZER);
DECLARE_EXTERNAL_LOG_PARAM_LIB_DB_GET(CPSS_LOG_LIB_FLOW_MANAGER);
DECLARE_EXTERNAL_LOG_PARAM_LIB_DB_GET(CPSS_LOG_LIB_BRIDGE_FDB_MANAGER);
DECLARE_EXTERNAL_LOG_PARAM_LIB_DB_GET(CPSS_LOG_LIB_PPU);
DECLARE_EXTERNAL_LOG_PARAM_LIB_DB_GET(CPSS_LOG_LIB_EXACT_MATCH_MANAGER);
DECLARE_EXTERNAL_LOG_PARAM_LIB_DB_GET(CPSS_LOG_LIB_MAC_SEC);
DECLARE_EXTERNAL_LOG_PARAM_LIB_DB_GET(CPSS_LOG_LIB_PTP_MANAGER);
DECLARE_EXTERNAL_LOG_PARAM_LIB_DB_GET(CPSS_LOG_LIB_HSR_PRP);
DECLARE_EXTERNAL_LOG_PARAM_LIB_DB_GET(CPSS_LOG_LIB_STREAM);
DECLARE_EXTERNAL_LOG_PARAM_LIB_DB_GET(CPSS_LOG_LIB_IPFIX_MANAGER);

/* Array of lib specific database access functions */
#ifdef CHX_FAMILY
PRV_CPSS_LOG_PARAM_LIB_DB_GET_FUNC prvCpssLogParamLibDbArray_DX[CPSS_LOG_LIB_LAST_CNS];
#endif

#ifdef PX_FAMILY
PRV_CPSS_LOG_PARAM_LIB_DB_GET_FUNC prvCpssLogParamLibDbArray_PX[CPSS_LOG_LIB_LAST_CNS];
#endif


/* Array of CPSS API return code strings */
static const PRV_CPSS_ENUM_STRING_VALUE_PAIR_STC prvCpssLogEnum_map_PRV_CPSS_LOG_FUNC_RET_CODE_ENT[] =
{
    /* L2 Bridge */
    PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(GT_ERROR                ),
    PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(GT_OK                   ),
    PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(GT_FAIL                 ),
    PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(GT_BAD_VALUE            ),
    PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(GT_OUT_OF_RANGE         ),
    PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(GT_BAD_PARAM            ),
    PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(GT_BAD_PTR              ),
    PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(GT_BAD_SIZE             ),
    PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(GT_BAD_STATE            ),
    PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(GT_SET_ERROR            ),
    PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(GT_GET_ERROR            ),
    PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(GT_CREATE_ERROR         ),
    PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(GT_NOT_FOUND            ),
    PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(GT_NO_MORE              ),
    PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(GT_NO_SUCH              ),
    PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(GT_TIMEOUT              ),
    PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(GT_NO_CHANGE            ),
    PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(GT_NOT_SUPPORTED        ),
    PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(GT_NOT_IMPLEMENTED      ),
    PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(GT_NOT_INITIALIZED      ),
    PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(GT_NO_RESOURCE          ),
    PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(GT_FULL                 ),
    PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(GT_EMPTY                ),
    PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(GT_INIT_ERROR           ),
    PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(GT_NOT_READY            ),
    PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(GT_ALREADY_EXIST        ),
    PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(GT_OUT_OF_CPU_MEM       ),
    PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(GT_ABORTED              ),
    PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(GT_NOT_APPLICABLE_DEVICE),
    PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(GT_UNFIXABLE_ECC_ERROR  ),
    PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(GT_UNFIXABLE_BIST_ERROR ),
    PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(GT_CHECKSUM_ERROR       ),
    PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(GT_DSA_PARSING_ERROR    ),
    PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(GT_TX_RING_ERROR                  ),
    PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(GT_NOT_ALLOWED                    ),
    PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(GT_HW_ERROR_NEED_RESET            ),
    PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(GT_LEARN_LIMIT_PORT_ERROR         ),
    PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(GT_LEARN_LIMIT_TRUNK_ERROR        ),
    PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(GT_LEARN_LIMIT_GLOBAL_EPORT_ERROR ),
    PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(GT_LEARN_LIMIT_FID_ERROR          ),
    PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(GT_LEARN_LIMIT_GLOBAL_ERROR       )

};
PRV_CPSS_LOG_STC_ENUM_MAP_ARRAY_SIZE_MAC(PRV_CPSS_LOG_FUNC_RET_CODE_ENT);



/**
* @internal prvCpssLogStatDump function
* @endinternal
*
* @brief   Dump statistic information of the log.
*/
GT_VOID prvCpssLogStatDump(GT_VOID)
{
    cpssOsPrintf("\n LOG Statistic \n Maximal number of parameters: %d\n Maximal stack size: %d\n",PRV_LOG_GLOBAL_VAR_VOLATILE_GET(prvLogStat).maxNumOfApiParams, PRV_LOG_GLOBAL_VAR_VOLATILE_GET(prvLogStat).maxStackDbSize);
    cpssOsPrintf("\n LOG Errors \n Stack DB Full: %d\n Stack params DB Full: %d\n",PRV_LOG_GLOBAL_VAR_VOLATILE_GET(prvLogStat).errorStackDbFull, PRV_LOG_GLOBAL_VAR_VOLATILE_GET(prvLogStat).errorStackParamDbFull);
    cpssOsPrintf("Stack DB Index is 0: %d\n Wrong function ID: %d\n Param pointer is NULL: %d\n",PRV_LOG_GLOBAL_VAR_VOLATILE_GET(prvLogStat).errorStackIndex0, PRV_LOG_GLOBAL_VAR_VOLATILE_GET(prvLogStat).errorWrongFuncId, PRV_LOG_GLOBAL_VAR_VOLATILE_GET(prvLogStat).errorParamInfoNullPointer);
    return;
}

/**
* @internal prvCpssErrorLogEnableSet function
* @endinternal
*
* @brief  function to enable/disable the 'ERROR LOG' on ALL libs
*
*    function to replace the need to do:
*    do shell-execute cpssLogEnableSet 1
*    do shell-execute cpssLogLibEnableSet 47,4,1
*
*    because the '47' (CPSS_LOG_LIB_ALL_E) value changes every time new LIB supported
*
*   [in] enable - enable/disable the 'ERROR LOG'
*/
GT_STATUS prvCpssErrorLogEnableSet(IN GT_BOOL enable)
{
    if(enable == GT_TRUE)
    {
        cpssLogEnableSet(GT_TRUE);
    }

    return cpssLogLibEnableSet (CPSS_LOG_LIB_ALL_E,CPSS_LOG_TYPE_ERROR_E,enable);
}


/**
* @internal prvCpssLogContextEntryCompare function
* @endinternal
*
* @brief   AVL tree node compare function.
*/
static GT_COMP_RES prvCpssLogContextEntryCompare
(
    IN  GT_VOID    *data_ptr1,
    IN  GT_VOID    *data_ptr2
)
{
    PRV_CPSS_LOG_CONTEXT_ENTRY_STC* ptr1;
    PRV_CPSS_LOG_CONTEXT_ENTRY_STC* ptr2;

    ptr1 = (PRV_CPSS_LOG_CONTEXT_ENTRY_STC*)data_ptr1;
    ptr2 = (PRV_CPSS_LOG_CONTEXT_ENTRY_STC*)data_ptr2;

    if (ptr1->prvCpssLogApiTid > ptr2->prvCpssLogApiTid)
        return GT_GREATER;
    if (ptr1->prvCpssLogApiTid < ptr2->prvCpssLogApiTid)
        return GT_SMALLER;
    return GT_EQUAL;
}

/**
* @internal prvCpssLogContextDbCreate function
* @endinternal
*
* @brief   Create log context database pool and tree.
*/
static GT_STATUS prvCpssLogContextDbCreate
(
    GT_VOID
)
{
    GT_STATUS rc;

    if (PRV_LOG_GLOBAL_NON_VOLATILE_VAR_DIRECT_GET(prvCpssLogAvlTreeId))
    {
        /* tree is already allocated. just empty it. */
        rc = prvCpssAvlTreeReset(PRV_LOG_GLOBAL_NON_VOLATILE_VAR_DIRECT_GET(prvCpssLogAvlTreeId), NULL, NULL);
        if(rc != GT_OK )
        {
            return rc;
        }

        rc = cpssBmPoolReCreate(PRV_LOG_GLOBAL_NON_VOLATILE_VAR_DIRECT_GET(prvCpssLogDbPoolId));
        return rc;
    }
    /* create data base memory pool */
    rc = cpssBmPoolCreate(sizeof(PRV_CPSS_LOG_CONTEXT_ENTRY_STC),
                          CPSS_BM_POOL_4_BYTE_ALIGNMENT_E,
                          PRV_CPSS_LOG_MAX_THREADS_CNS, &PRV_LOG_GLOBAL_NON_VOLATILE_VAR_DIRECT_GET(prvCpssLogDbPoolId));
    if (rc != GT_OK)
    {
      return rc;
    }
    /* create AVL memory pool */
    rc = prvCpssAvlMemPoolCreate(PRV_CPSS_LOG_MAX_THREADS_CNS, &PRV_LOG_GLOBAL_NON_VOLATILE_VAR_DIRECT_GET(prvCpssLogAvlPoolId));
    if (rc != GT_OK)
    {
        return rc;
    }

    /* create AVL tree */
    rc = prvCpssAvlTreeCreate((GT_INTFUNCPTR)prvCpssLogContextEntryCompare, PRV_LOG_GLOBAL_NON_VOLATILE_VAR_DIRECT_GET(prvCpssLogAvlPoolId), &PRV_LOG_GLOBAL_NON_VOLATILE_VAR_DIRECT_GET(prvCpssLogAvlTreeId));
    if (rc != GT_OK)
    {
        return rc;
    }

    return GT_OK;
}

/**
* @internal prvCpssLogContextEntryGet function
* @endinternal
*
* @brief   Get log context entry from database for current thread.
*/
static GT_STATUS prvCpssLogContextEntryGet
(
    OUT PRV_CPSS_LOG_CONTEXT_ENTRY_STC ** entryPtrPtr
)
{
    GT_STATUS rc;
    GT_U32 currentThreadId;
    /* currentEntry DB protected by CPSS_ZERO_LEVEL_API_LOCK_MAC */
    PRV_CPSS_LOG_CONTEXT_ENTRY_STC *currentEntryPtr = &PRV_LOG_GLOBAL_VAR_VOLATILE_GET(currentEntry);
    PRV_CPSS_LOG_CONTEXT_ENTRY_STC *entryPtr;

    /* get current thread ID */
    rc = cpssOsTaskGetSelf(&currentThreadId);
#ifdef _WIN32
    if(rc == GT_NOT_FOUND)
    {
        /* the implementation not found the task as 'osTaskCreate' , but returned
           it's windows handle ... it is OK for us !

           it happen for the 'ISR context' that calls function like : cpssSystemRecoveryStateGet(...)
        */
        rc = GT_OK;
    }
#endif /*_WIN32*/
    if (rc != GT_OK)
    {
        return rc;
    }

    currentEntryPtr->prvCpssLogApiTid = currentThreadId;
    /* find entry in AVL tree */
    entryPtr = prvCpssAvlSearch(PRV_LOG_GLOBAL_NON_VOLATILE_VAR_DIRECT_GET(prvCpssLogAvlTreeId), currentEntryPtr);
    if (entryPtr == NULL)
    {
        /* get free buffer from pool */
        entryPtr = cpssBmPoolBufGet(PRV_LOG_GLOBAL_NON_VOLATILE_VAR_DIRECT_GET(prvCpssLogDbPoolId));
        if (entryPtr == 0)
        {
            return GT_NO_MORE;
        }
        /* init new context data */
        cpssOsMemSet(entryPtr, 0, sizeof(PRV_CPSS_LOG_CONTEXT_ENTRY_STC));
        entryPtr->prvCpssLogApiTid = currentThreadId;
        entryPtr->prvCpssLogApiContextLib = CPSS_LOG_LIB_ALL_E;
        entryPtr->prvCpssLogEntryFuncId = PRV_CPSS_LOG_NOT_VALID_CNS;
        /* add new entry to the tree for current context */
        rc = prvCpssAvlItemInsert(PRV_LOG_GLOBAL_NON_VOLATILE_VAR_DIRECT_GET(prvCpssLogAvlTreeId), entryPtr);
        if (rc != GT_OK)
        {
            return rc;
        }
    }
    /* return pointer to current context */
    *entryPtrPtr = entryPtr;

    return GT_OK;
}

/**
* @internal prvCpssLogApiHistoryDbClose function
* @endinternal
*
* @brief   Close history log database.
*/
static void prvCpssLogApiHistoryDbClose
(
    INOUT PRV_CPSS_LOG_API_HISTORY_ENTRY_STC ** entryPtrPtr,
    INOUT GT_U32 * numEntriesPtr,
    IN    GT_BOOL  clear
)
{
    GT_U32 numEntries;
    PRV_CPSS_LOG_API_HISTORY_ENTRY_STC * currentHistoryEntryPtr;

    numEntries = *numEntriesPtr;
    currentHistoryEntryPtr = *entryPtrPtr;

    /* Database is empty */
    if (numEntries == 0)
    {
        return;
    }

    /* Iterate all database entries */
    if (numEntries > 1)
    {
        *numEntriesPtr = numEntries - 1;
        prvCpssLogApiHistoryDbClose(&currentHistoryEntryPtr->nextEntryPtr, numEntriesPtr, clear);
    }

    if (currentHistoryEntryPtr->entryHistoryFile.fd != CPSS_OS_FILE_INVALID)
    {
        /* Close file */
#ifndef SHARED_MEMORY
        cpssOsFclose(&currentHistoryEntryPtr->entryHistoryFile);
#else
        prvLogCbFileClose(currentHistoryEntryPtr);
#endif
        if (clear == GT_TRUE) {
#ifndef SHARED_MEMORY
            cpssOsFopen((const char *)currentHistoryEntryPtr->entryHistoryFileName, "w",
                    &currentHistoryEntryPtr->entryHistoryFile);
            cpssOsFclose(&currentHistoryEntryPtr->entryHistoryFile);
#else
            prvLogCbFileOpen(currentHistoryEntryPtr, GT_TRUE);
            prvLogCbFileClose(currentHistoryEntryPtr);
#endif
        }
        if (currentHistoryEntryPtr != &PRV_LOG_GLOBAL_VAR_VOLATILE_GET(firstHistoryDbEntry))
        {
            /* Free entry */
            cpssOsFree(currentHistoryEntryPtr);
        }
    }
    /* Reset current entry pointer and number of entries */
    *numEntriesPtr = 0;
    *entryPtrPtr = 0;
}

/**
* @internal prvCpssLogApiHistoryDbLogOutput function
* @endinternal
*
* @brief   Output history log database.
*
* @param[in] currentHistoryEntryPtr   - Start database entry to start from.
* @param[in] numEntries               - Number of database entries to print.
*                                       None.
*/
static void prvCpssLogApiHistoryDbLogOutput
(
    IN PRV_CPSS_LOG_API_HISTORY_ENTRY_STC * currentHistoryEntryPtr,
    IN GT_U32 numEntries
)
{
    GT_U32 entryIndex;

    /* Iterate all database entries */
    for (entryIndex = 0; entryIndex < numEntries; entryIndex++)
    {
        if (currentHistoryEntryPtr->entryHistoryFile.fd != CPSS_OS_FILE_INVALID)
        {
            cpssOsLog(PRV_LOG_GLOBAL_NON_VOLATILE_VAR_DIRECT_GET(prvCpssLogContextEntryPtr)->prvCpssLogApiContextLib, CPSS_LOG_TYPE_ENTRY_LEVEL_FUNCTION_E, "API history log entry %d.\n", (numEntries - entryIndex - 1));

            /* Close file */
#ifndef SHARED_MEMORY
            cpssOsFclose(&currentHistoryEntryPtr->entryHistoryFile);

            /* Open file in read mode */
            cpssOsFopen((const char *)currentHistoryEntryPtr->entryHistoryFileName, "r",
                    &currentHistoryEntryPtr->entryHistoryFile);
#else
            prvLogCbFileClose(currentHistoryEntryPtr);
            prvLogCbFileOpen(currentHistoryEntryPtr, GT_FALSE);
#endif
            if (currentHistoryEntryPtr->entryHistoryFile.fd == CPSS_OS_FILE_INVALID)
            {
                cpssOsLog(PRV_LOG_GLOBAL_NON_VOLATILE_VAR_DIRECT_GET(prvCpssLogContextEntryPtr)->prvCpssLogApiContextLib, CPSS_LOG_TYPE_ERROR_E, " ERROR: File opening failed!\n");
                return;
            }

#ifndef SHARED_MEMORY
            /* Print history file */
            while (cpssOsFgets(PRV_LOG_GLOBAL_VAR_VOLATILE_GET(buffer), 1024, currentHistoryEntryPtr->entryHistoryFile.fd) != 0)
            {
                cpssOsLog(PRV_LOG_GLOBAL_NON_VOLATILE_VAR_DIRECT_GET(prvCpssLogContextEntryPtr)->prvCpssLogApiContextLib, CPSS_LOG_TYPE_ERROR_E, PRV_LOG_GLOBAL_VAR_VOLATILE_GET(buffer));
            }
#else
            prvLogCbFileRead(currentHistoryEntryPtr, PRV_LOG_GLOBAL_NON_VOLATILE_VAR_DIRECT_GET(prvCpssLogContextEntryPtr)->prvCpssLogApiContextLib);
#endif
        }

        /* Get next entry */
        currentHistoryEntryPtr = currentHistoryEntryPtr->nextEntryPtr;
    }
}

/**
* @internal prvCpssLogApiHistory function
* @endinternal
*
* @brief   Manage history log database.
*
* @param[in] logPhase                 - API call log phase - enter or exit.
* @param[in] strBufferPtr             - (pointre to) log string buffer
*                                       None.
*/
static GT_VOID prvCpssLogApiHistory
(
    IN PRV_CPSS_LOG_PHASE_ENT   logPhase,
    IN const char * strBufferPtr
)
{

    if (PRV_LOG_GLOBAL_NON_VOLATILE_VAR_DIRECT_GET(prvCpssLogStackSize) == 0)
    {
        /* History log disabled */
        return;
    }

    switch (logPhase)
    {
        case PRV_CPSS_LOG_PHASE_ENTER_E:
            if (PRV_LOG_GLOBAL_NON_VOLATILE_VAR_DIRECT_GET(prvCpssLogContextEntryPtr)->prvCpssLogApiContextIndex > 1)
            {
                /* Print history in context of previous API */
                break;
            }
            /* New API parsing started */
            if (PRV_LOG_GLOBAL_VAR_VOLATILE_GET(currentHistoryEntryNum) < PRV_LOG_GLOBAL_NON_VOLATILE_VAR_DIRECT_GET(prvCpssLogStackSize))
            {
                if (PRV_LOG_GLOBAL_VAR_VOLATILE_GET(currentHistoryEntryPtr) == 0)
                {
                    /* Init current history entry */
                    PRV_LOG_GLOBAL_VAR_VOLATILE_GET(currentHistoryEntryPtr) = &PRV_LOG_GLOBAL_VAR_VOLATILE_GET(firstHistoryDbEntry);
                    /* Set list tail to list head */
                    PRV_LOG_GLOBAL_VAR_VOLATILE_GET(currentHistoryEntryPtr)->nextEntryPtr = PRV_LOG_GLOBAL_VAR_VOLATILE_GET(currentHistoryEntryPtr);
                }
                else
                {
                    /* Allocate new list entry */
                    PRV_LOG_GLOBAL_VAR_VOLATILE_GET(currentHistoryEntryPtr)->nextEntryPtr = cpssOsMalloc(sizeof(PRV_CPSS_LOG_API_HISTORY_ENTRY_STC));
                    /* Set current list entry */
                    PRV_LOG_GLOBAL_VAR_VOLATILE_GET(currentHistoryEntryPtr) = PRV_LOG_GLOBAL_VAR_VOLATILE_GET(currentHistoryEntryPtr)->nextEntryPtr;
                    /* Set list tail to list head */
                    PRV_LOG_GLOBAL_VAR_VOLATILE_GET(currentHistoryEntryPtr)->nextEntryPtr = &PRV_LOG_GLOBAL_VAR_VOLATILE_GET(firstHistoryDbEntry);
                }
                PRV_LOG_GLOBAL_VAR_VOLATILE_GET(currentHistoryEntryPtr)->entryHistoryFile.type = CPSS_OS_FILE_REGULAR;

                cpssOsSprintf(PRV_LOG_GLOBAL_VAR_VOLATILE_GET(currentHistoryEntryPtr)->entryHistoryFileName, "%s_%d", PRV_LOG_GLOBAL_VAR_VOLATILE_GET(prvCpssLogHistoryName), PRV_LOG_GLOBAL_VAR_VOLATILE_GET(currentHistoryEntryNum));

                /* Open new file stream */
#ifndef SHARED_MEMORY
                cpssOsFopen((const char *)PRV_LOG_GLOBAL_VAR_VOLATILE_GET(currentHistoryEntryPtr)->entryHistoryFileName, "w",
                        &PRV_LOG_GLOBAL_VAR_VOLATILE_GET(currentHistoryEntryPtr)->entryHistoryFile);
#else
                prvLogCbFileOpen(PRV_LOG_GLOBAL_VAR_VOLATILE_GET(currentHistoryEntryPtr), GT_TRUE);
#endif
                if (PRV_LOG_GLOBAL_VAR_VOLATILE_GET(currentHistoryEntryPtr)->entryHistoryFile.fd == CPSS_OS_FILE_INVALID)
                {
                    cpssOsLog(PRV_LOG_GLOBAL_NON_VOLATILE_VAR_DIRECT_GET(prvCpssLogContextEntryPtr)->prvCpssLogApiContextLib, CPSS_LOG_TYPE_ERROR_E, " ERROR: File opening failed!\n");
                    break;
                }
                /* Increment entry counter */
                PRV_LOG_GLOBAL_VAR_VOLATILE_GET(currentHistoryEntryNum)++;
            }
            else
            {
                if (PRV_LOG_GLOBAL_VAR_VOLATILE_GET(currentHistoryEntryPtr)->entryHistoryFile.fd != CPSS_OS_FILE_INVALID)
                {
                    /* Sets the position to the beginning of the file */
#ifndef SHARED_MEMORY
                    cpssOsRewind(PRV_LOG_GLOBAL_VAR_VOLATILE_GET(currentHistoryEntryPtr)->entryHistoryFile.fd);
#else
                    prvLogCbFileRewind(PRV_LOG_GLOBAL_VAR_VOLATILE_GET(currentHistoryEntryPtr));
#endif
                    /* Set current list entry to next in database */
                    PRV_LOG_GLOBAL_VAR_VOLATILE_GET(currentHistoryEntryPtr) = PRV_LOG_GLOBAL_VAR_VOLATILE_GET(currentHistoryEntryPtr)->nextEntryPtr;
                }
            }
            break;
        case PRV_CPSS_LOG_PHASE_EXIT_E:
            if (PRV_LOG_GLOBAL_VAR_VOLATILE_GET(currentHistoryEntryPtr))
            {
                if (PRV_LOG_GLOBAL_VAR_VOLATILE_GET(lastState) == PRV_CPSS_LOG_PHASE_ERROR_E)
                {
                    if (PRV_LOG_GLOBAL_NON_VOLATILE_VAR_DIRECT_GET(prvCpssLogContextEntryPtr)->prvCpssLogApiContextIndex == 1)
                    {
                        /* Start logging from list head */
                        prvCpssLogApiHistoryDbLogOutput(PRV_LOG_GLOBAL_VAR_VOLATILE_GET(currentHistoryEntryPtr)->nextEntryPtr, PRV_LOG_GLOBAL_VAR_VOLATILE_GET(currentHistoryEntryNum));
                        /* Clean up database structure */
                        prvCpssLogApiHistoryDbClose(&PRV_LOG_GLOBAL_VAR_VOLATILE_GET(currentHistoryEntryPtr), &PRV_LOG_GLOBAL_VAR_VOLATILE_GET(currentHistoryEntryNum), GT_FALSE);

                        PRV_LOG_GLOBAL_VAR_VOLATILE_GET(lastState) = PRV_CPSS_LOG_PHASE_EXIT_E;
                    }
                }
            }
            break;
        case PRV_CPSS_LOG_PHASE_ERROR_E:
            PRV_LOG_GLOBAL_VAR_VOLATILE_GET(lastState) = PRV_CPSS_LOG_PHASE_ERROR_E;
            break;
        default:
            return;
    }

    if (PRV_LOG_GLOBAL_VAR_VOLATILE_GET(lastState) != PRV_CPSS_LOG_PHASE_ERROR_E)
    {
        PRV_LOG_GLOBAL_VAR_VOLATILE_GET(lastState) = logPhase;
    }

    if (PRV_LOG_GLOBAL_VAR_VOLATILE_GET(currentHistoryEntryPtr))
    {
#ifndef SHARED_MEMORY
        if(0 != PRV_LOG_GLOBAL_VAR_VOLATILE_GET(currentHistoryEntryPtr)->entryHistoryFile.fd)
        {
            cpssOsFprintf(PRV_LOG_GLOBAL_VAR_VOLATILE_GET(currentHistoryEntryPtr)->entryHistoryFile.fd, strBufferPtr);
        }
#else
        prvLogCbFileWrite(strBufferPtr);
#endif
    }
}

/**
* @internal prvCpssLogAndHistory function
* @endinternal
*
* @brief   Log output and log history
*
* @param[in] logPhase                 - API call log phase - enter or exit.
* @param[in] lib                      - the function will print the log of the functions in "lib".
* @param[in] type                     - the function will print the logs from "type".
* @param[in] format                   - usual printf  string.
*                                      ...             - additional parameters.
*                                       None.
*/
static GT_VOID prvCpssLogAndHistory
(
    IN PRV_CPSS_LOG_PHASE_ENT   logPhase,
    IN CPSS_LOG_LIB_ENT         lib,
    IN CPSS_LOG_TYPE_ENT        type,
    IN const char*              format,
    ...
)
{
    va_list args;
    GT_BOOL enableLib = GT_FALSE;

    cpssLogLibEnableGet(lib,type,&enableLib);

    cpssOsMemSet(PRV_LOG_GLOBAL_VAR_VOLATILE_GET(buffer), 0, sizeof(PRV_LOG_GLOBAL_VAR_VOLATILE_GET(buffer)));

    va_start(args, format);
    cpssOsVsprintf(PRV_LOG_GLOBAL_VAR_VOLATILE_GET(buffer), format, args);
    va_end(args);

    switch (type)
    {
        case CPSS_LOG_TYPE_INFO_E:
        case CPSS_LOG_TYPE_ERROR_E:
            if (enableLib == GT_TRUE) {
                cpssOsLog(lib, type, PRV_LOG_GLOBAL_VAR_VOLATILE_GET(buffer));
            }
            break;
        case CPSS_LOG_TYPE_ENTRY_LEVEL_FUNCTION_E:
        case CPSS_LOG_TYPE_NON_ENTRY_LEVEL_FUNCTION_E:
            if (PRV_LOG_GLOBAL_NON_VOLATILE_VAR_DIRECT_GET(prvCpssLogSilent) == GT_FALSE)
            {
                cpssOsLog(lib, type, PRV_LOG_GLOBAL_VAR_VOLATILE_GET(buffer));
            }
            break;
        default:
            cpssOsLog(lib, type, PRV_LOG_GLOBAL_VAR_VOLATILE_GET(buffer));
            break;
    }

    if (type != CPSS_LOG_TYPE_INFO_E)
    {
        prvCpssLogApiHistory(logPhase, PRV_LOG_GLOBAL_VAR_VOLATILE_GET(buffer));
    }

    return;
}

/**
* @internal prvCpssLogAndHistoryParam function
* @endinternal
*
* @brief   Log output and log history of function's parameter
*
* @param[in] lib                      - the function will print the log of the functions in "lib".
* @param[in] type                     - the function will print the logs from "type".
* @param[in] format                   - usual printf  string.
*                                      ...             - additional parameters.
*                                       None.
*/
GT_VOID prvCpssLogAndHistoryParam
(
    IN CPSS_LOG_LIB_ENT         lib,
    IN CPSS_LOG_TYPE_ENT        type,
    IN const char*              format,
    ...
)
{
    va_list args;

    cpssOsBzero(PRV_LOG_GLOBAL_VAR_VOLATILE_GET(buffer), sizeof(PRV_LOG_GLOBAL_VAR_VOLATILE_GET(buffer)));

    va_start(args, format);
    cpssOsVsnprintf(PRV_LOG_GLOBAL_VAR_VOLATILE_GET(buffer), sizeof(PRV_LOG_GLOBAL_VAR_VOLATILE_GET(buffer)), format, args);
    va_end(args);

    switch (type)
    {
        case CPSS_LOG_TYPE_INFO_E:
        case CPSS_LOG_TYPE_ERROR_E:
        default:
            cpssOsLog(lib, type, PRV_LOG_GLOBAL_VAR_VOLATILE_GET(buffer));
            break;
        case CPSS_LOG_TYPE_ENTRY_LEVEL_FUNCTION_E:
        case CPSS_LOG_TYPE_NON_ENTRY_LEVEL_FUNCTION_E:
            if (PRV_LOG_GLOBAL_NON_VOLATILE_VAR_DIRECT_GET(prvCpssLogSilent) == GT_FALSE)
            {
                cpssOsLog(lib, type, PRV_LOG_GLOBAL_VAR_VOLATILE_GET(buffer));
            }
            break;
    }

    if (type != CPSS_LOG_TYPE_INFO_E)
    {
        prvCpssLogApiHistory(PRV_CPSS_LOG_PHASE_INFO_E, PRV_LOG_GLOBAL_VAR_VOLATILE_GET(buffer));
    }

    return;
}

/**
* @internal prvCpssLogCpssLogEnterParams function
* @endinternal
*
* @brief   Log API enter parameters function.
*
* @param[in] functionId               - function ID
* @param[in] logType                  - log type
* @param[in] funcEntryPtr             - pointer to API log entry.
* @param[in] args                     - pointer to current argument in argument list.
*                                       None.
*/
static void prvCpssLogCpssLogEnterParams
(
    IN GT_U32                         functionId,
    IN CPSS_LOG_TYPE_ENT              logType,
    IN const PRV_CPSS_LOG_FUNC_ENTRY_STC  * funcEntryPtr,
    IN va_list                      * args
)
{
    GT_U32                            paramIdx;
    const PRV_CPSS_LOG_FUNC_PARAM_STC * paramPtr;
    PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC paramData;
    GT_BOOL                           skipLog;
    CPSS_LOG_LIB_ENT                  contextLib = PRV_LOG_GLOBAL_NON_VOLATILE_VAR_DIRECT_GET(prvCpssLogContextEntryPtr)->prvCpssLogApiContextLib;
    GT_U32                            numOfStoredParams;
    GT_BOOL                           saveParam;
    va_list                           preLogicArgc;
    PRV_CPSS_LOG_API_STACK_STC      * apiStackDbPtr;

    if (PRV_LOG_GLOBAL_NON_VOLATILE_VAR_DIRECT_GET(prvCpssLogSilent) == GT_FALSE &&
        PRV_LOG_GLOBAL_NON_VOLATILE_VAR_DIRECT_GET(prvCpssLogFormat) == CPSS_LOG_API_FORMAT_NO_PARAMS_E)
    {
        /* Silent mode is off and API parameters not eligible to log */
        return;
    }

    cpssOsMemSet(&paramData, 0, sizeof(PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC));

    if (funcEntryPtr->prvCpssLogPreLogFunc)
    {
        /* copy argc because prvCpssLogPreLogFunc changes argc state */
        #ifdef va_copy
        va_copy(preLogicArgc, *args);
        #else
        cpssOsMemCpy(&preLogicArgc, args, sizeof(*args));
        #endif

        /* Call pre-log API function */
        funcEntryPtr->prvCpssLogPreLogFunc(preLogicArgc, &paramData);

        va_end(preLogicArgc);
    }

    if (PRV_LOG_GLOBAL_VAR_VOLATILE_GET(prvLogStat).maxNumOfApiParams < funcEntryPtr->numOfParams)
    {
        PRV_LOG_GLOBAL_VAR_VOLATILE_GET(prvLogStat).maxNumOfApiParams = funcEntryPtr->numOfParams;
    }

    if (PRV_LOG_GLOBAL_NON_VOLATILE_VAR_DIRECT_GET(prvCpssLogContextEntryPtr)->prvCpssLogStackCurrentIndex >= PRV_CPSS_LOG_API_STACK_SIZE_CNS)
    {
        PRV_LOG_GLOBAL_VAR_VOLATILE_GET(prvLogStat).errorStackDbFull++;
        PRV_CPSS_LOG_AND_HISTORY_MAC(PRV_CPSS_LOG_PHASE_ERROR_E, contextLib, CPSS_LOG_TYPE_ERROR_E, "ERROR: stack DB is full. size %d\n", PRV_LOG_GLOBAL_NON_VOLATILE_VAR_DIRECT_GET(prvCpssLogContextEntryPtr)->prvCpssLogStackCurrentIndex);
        return;
    }

    /* store function related information */
    apiStackDbPtr = &PRV_LOG_GLOBAL_NON_VOLATILE_VAR_DIRECT_GET(prvCpssLogContextEntryPtr)->prvCpssLogApiStack[PRV_LOG_GLOBAL_NON_VOLATILE_VAR_DIRECT_GET(prvCpssLogContextEntryPtr)->prvCpssLogStackCurrentIndex];
    apiStackDbPtr->functionId = functionId;
    apiStackDbPtr->paramKey = paramData.paramKey;
    numOfStoredParams = 0;

    /* Parsing of parameters */
    for (paramIdx = 0; paramIdx < funcEntryPtr->numOfParams; paramIdx++)
    {
        /* Reset parameter LOG format to default */
        paramData.formatPtr = 0;

        /* get pointer to parameter's information */
        paramPtr = funcEntryPtr->paramsPtrPtr[paramIdx];

        skipLog = GT_FALSE;
        saveParam = GT_TRUE;
        switch (paramPtr->direction)
        {
            case PRV_CPSS_LOG_PARAM_IN_E:
                saveParam = GT_FALSE;
                break;
            case PRV_CPSS_LOG_PARAM_OUT_E:
                skipLog = GT_TRUE;
                break;
            default: /* INOUT */
                break;
        }

        /* Log CPSS API parameters */
        if (paramPtr->logFunc)
        {
            paramPtr->logFunc(contextLib, logType, paramPtr->paramNamePtr, args, skipLog, &paramData);
            if (saveParam)
            {
                if (numOfStoredParams >= PRV_CPSS_LOG_API_PARAM_MAX_NUM_CNS)
                {
                    PRV_LOG_GLOBAL_VAR_VOLATILE_GET(prvLogStat).errorStackParamDbFull++;
                    PRV_CPSS_LOG_AND_HISTORY_MAC(PRV_CPSS_LOG_PHASE_ERROR_E, contextLib, CPSS_LOG_TYPE_ERROR_E, "ERROR: Param could not be stored %s\n", paramPtr->paramNamePtr);
                }
                else
                {
                    /* save INOUT or OUT parameter's information and value for further processing in Exit function */
                    apiStackDbPtr->paramsArr[numOfStoredParams].paramsPtr = paramPtr;
                    apiStackDbPtr->paramsArr[numOfStoredParams].paramValue = paramData.paramValue;
                    numOfStoredParams++;
                }
            }
        }
        else
        {
            PRV_LOG_GLOBAL_VAR_VOLATILE_GET(prvLogStat).errorParamInfoNullPointer++;
            PRV_CPSS_LOG_AND_HISTORY_MAC(PRV_CPSS_LOG_PHASE_ERROR_E, contextLib, CPSS_LOG_TYPE_ERROR_E, "ERROR: Param could not be logged %s\n", paramPtr->paramNamePtr);
        }
    }

    apiStackDbPtr->numOfParams = numOfStoredParams;
    PRV_LOG_GLOBAL_NON_VOLATILE_VAR_DIRECT_GET(prvCpssLogContextEntryPtr)->prvCpssLogStackCurrentIndex++;

    if (PRV_LOG_GLOBAL_VAR_VOLATILE_GET(prvLogStat).maxStackDbSize  < PRV_LOG_GLOBAL_NON_VOLATILE_VAR_DIRECT_GET(prvCpssLogContextEntryPtr)->prvCpssLogStackCurrentIndex)
    {
        PRV_LOG_GLOBAL_VAR_VOLATILE_GET(prvLogStat).maxStackDbSize = PRV_LOG_GLOBAL_NON_VOLATILE_VAR_DIRECT_GET(prvCpssLogContextEntryPtr)->prvCpssLogStackCurrentIndex;
    }
}

/**
* @internal prvCpssLogCpssLogExitParams function
* @endinternal
*
* @brief   Log API parameters function.
*
* @param[in] functionId               - function ID
* @param[in] logType                  - log type
* @param[in] funcEntryPtr             - pointer to API log entry.
*                                      args            - pointer to current argument in argument list.
*                                       None.
*/
static void prvCpssLogCpssLogExitParams
(
    IN GT_U32                         functionId,
    IN CPSS_LOG_TYPE_ENT              logType,
    IN const PRV_CPSS_LOG_FUNC_ENTRY_STC  * funcEntryPtr
)
{
    GT_U32                            paramIdx;
    const PRV_CPSS_LOG_FUNC_PARAM_STC * paramPtr;
    PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC paramData;
    GT_BOOL                           skipLog;
    CPSS_LOG_LIB_ENT                  contextLib = PRV_LOG_GLOBAL_NON_VOLATILE_VAR_DIRECT_GET(prvCpssLogContextEntryPtr)->prvCpssLogApiContextLib;
    GT_U32                            numOfStoredParams;
    PRV_CPSS_LOG_API_STACK_STC      * apiStackDbPtr;

    (void)funcEntryPtr;
    if (PRV_LOG_GLOBAL_NON_VOLATILE_VAR_DIRECT_GET(prvCpssLogSilent) == GT_FALSE &&
        PRV_LOG_GLOBAL_NON_VOLATILE_VAR_DIRECT_GET(prvCpssLogFormat) == CPSS_LOG_API_FORMAT_NO_PARAMS_E)
    {
        /* Silent mode is off and API parameters not eligible to log */
        return;
    }

    if (PRV_LOG_GLOBAL_NON_VOLATILE_VAR_DIRECT_GET(prvCpssLogContextEntryPtr)->prvCpssLogStackCurrentIndex)
    {
        PRV_LOG_GLOBAL_NON_VOLATILE_VAR_DIRECT_GET(prvCpssLogContextEntryPtr)->prvCpssLogStackCurrentIndex--;
    }
    else
    {
        PRV_LOG_GLOBAL_VAR_VOLATILE_GET(prvLogStat).errorStackIndex0++;
        PRV_CPSS_LOG_AND_HISTORY_MAC(PRV_CPSS_LOG_PHASE_ERROR_E, contextLib, CPSS_LOG_TYPE_ERROR_E, "ERROR: stack index 0, functionId %d\n", functionId);
    }

    /* check log's DB consistency */
    apiStackDbPtr = &PRV_LOG_GLOBAL_NON_VOLATILE_VAR_DIRECT_GET(prvCpssLogContextEntryPtr)->prvCpssLogApiStack[PRV_LOG_GLOBAL_NON_VOLATILE_VAR_DIRECT_GET(prvCpssLogContextEntryPtr)->prvCpssLogStackCurrentIndex];
    if(apiStackDbPtr->functionId != functionId)
    {
        PRV_LOG_GLOBAL_VAR_VOLATILE_GET(prvLogStat).errorWrongFuncId++;
        PRV_CPSS_LOG_AND_HISTORY_MAC(PRV_CPSS_LOG_PHASE_ERROR_E, contextLib, CPSS_LOG_TYPE_ERROR_E, "ERROR: wrong functionId %d expected %d\n", functionId, apiStackDbPtr->functionId);
        return;
    }

    cpssOsMemSet(&paramData, 0, sizeof(PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC));

    /* use function related info from DB */
    paramData.paramKey = apiStackDbPtr->paramKey;
    numOfStoredParams = apiStackDbPtr->numOfParams;

    skipLog = GT_FALSE;

    /* Parsing of parameters */
    for (paramIdx = 0; paramIdx < numOfStoredParams; paramIdx++)
    {
        /* Reset parameter LOG format to default */
        paramData.formatPtr = 0;

        paramPtr =  apiStackDbPtr->paramsArr[paramIdx].paramsPtr;
        paramData.paramValue = apiStackDbPtr->paramsArr[paramIdx].paramValue;

        /* Log CPSS API parameters */
        if (paramPtr->logFunc)
        {
            paramPtr->logFunc(contextLib, logType, paramPtr->paramNamePtr, NULL, skipLog, &paramData);
        }
        else
        {
            PRV_LOG_GLOBAL_VAR_VOLATILE_GET(prvLogStat).errorParamInfoNullPointer++;
            PRV_CPSS_LOG_AND_HISTORY_MAC(PRV_CPSS_LOG_PHASE_ERROR_E, contextLib, CPSS_LOG_TYPE_ERROR_E, "ERROR: Param could not be logged %s\n", paramPtr->paramNamePtr);
        }
    }
}
/**
* @internal prvCpssLogApiFunctionLogTrigger function
* @endinternal
*
* @brief   Check triggered API function log types
*
* @param[in] logType                  - type of the log
*
* @retval GT_TRUE                  - API log triggered
* @retval GT_FALSE                 - API log not triggered
*/
static GT_BOOL prvCpssLogApiFunctionLogTrigger
(
    IN CPSS_LOG_TYPE_ENT logType
)
{
    GT_BOOL enableLib = GT_FALSE;
    GT_STATUS rc;

    /* Get entrance lib status */
    rc = cpssLogLibEnableGet(PRV_LOG_GLOBAL_NON_VOLATILE_VAR_DIRECT_GET(prvCpssLogContextEntryPtr)->prvCpssLogApiContextLib, logType, &enableLib);
    if (rc != GT_OK)
    {
        PRV_CPSS_LOG_AND_HISTORY_MAC(PRV_CPSS_LOG_PHASE_ERROR_E, PRV_LOG_GLOBAL_NON_VOLATILE_VAR_DIRECT_GET(prvCpssLogContextEntryPtr)->prvCpssLogApiContextLib, CPSS_LOG_TYPE_ERROR_E, "prvCpssLogApiFunctionLogTrigger: rc = %d\n", rc);
        return GT_FALSE;
    }

    if (PRV_LOG_GLOBAL_NON_VOLATILE_VAR_DIRECT_GET(prvCpssLogStackSize))
    {
        /* Log history enabled if API context lib log is disabled and vise versa */
        PRV_LOG_GLOBAL_NON_VOLATILE_VAR_DIRECT_GET(prvCpssLogSilent) = (enableLib) ? GT_FALSE : GT_TRUE;
        /* Activate API history log */
        enableLib = GT_TRUE;
    }
    else
    {
        /* Log history disabled */
        PRV_LOG_GLOBAL_NON_VOLATILE_VAR_DIRECT_GET(prvCpssLogSilent) = GT_FALSE;
    }

    /* API log  trigger status */
    return enableLib;
}

/**
* @internal prvCpssLogApiLibContextSet function
* @endinternal
*
* @brief   Set API LIB context logic
*
* @param[in] functionId               - function identifier
* @param[in] logPhase                 - API call log phase - enter or exit
* @param[in] mngFunctionId            - function ID for Log management
*                                       None.
*/
static void prvCpssLogApiLibContextSet
(
    IN GT_U32   functionId,
    IN PRV_CPSS_LOG_PHASE_ENT  logPhase,
    IN GT_U32   mngFunctionId
)
{
    GT_U32 funcLibId = (mngFunctionId >> 16) & 0xFF;

    switch (logPhase)
    {
        case PRV_CPSS_LOG_PHASE_ENTER_E:
            if (PRV_LOG_GLOBAL_NON_VOLATILE_VAR_DIRECT_GET(prvCpssLogContextEntryPtr)->prvCpssLogApiContextIndex == 0)
            {
                /* Set API call lib context */
                PRV_LOG_GLOBAL_NON_VOLATILE_VAR_DIRECT_GET(prvCpssLogContextEntryPtr)->prvCpssLogApiContextLib = funcLibId;
                /* Set API entry level type */
                PRV_LOG_GLOBAL_NON_VOLATILE_VAR_DIRECT_GET(prvCpssLogContextEntryPtr)->prvCpssLogEntryFuncId = functionId;
            }
            /* Increment API context index */
            PRV_LOG_GLOBAL_NON_VOLATILE_VAR_DIRECT_GET(prvCpssLogContextEntryPtr)->prvCpssLogApiContextIndex++;
            break;
        case PRV_CPSS_LOG_PHASE_EXIT_E:
            /* Decrement API context index */
            PRV_LOG_GLOBAL_NON_VOLATILE_VAR_DIRECT_GET(prvCpssLogContextEntryPtr)->prvCpssLogApiContextIndex--;
            if (PRV_LOG_GLOBAL_NON_VOLATILE_VAR_DIRECT_GET(prvCpssLogContextEntryPtr)->prvCpssLogApiContextIndex == 0)
            {
                /* Reset API call lib context */
                PRV_LOG_GLOBAL_NON_VOLATILE_VAR_DIRECT_GET(prvCpssLogContextEntryPtr)->prvCpssLogApiContextLib = CPSS_LOG_LIB_ALL_E;
                /* Reset API log type */
                PRV_LOG_GLOBAL_NON_VOLATILE_VAR_DIRECT_GET(prvCpssLogContextEntryPtr)->prvCpssLogEntryFuncId = PRV_CPSS_LOG_NOT_VALID_CNS;
            }
            break;
        default:
            PRV_CPSS_LOG_AND_HISTORY_MAC(PRV_CPSS_LOG_PHASE_ERROR_E, PRV_LOG_GLOBAL_NON_VOLATILE_VAR_DIRECT_GET(prvCpssLogContextEntryPtr)->prvCpssLogApiContextLib, CPSS_LOG_TYPE_ERROR_E, "prvCpssLogApiLibContextSet: wrong log phase.\n");
    }
}

/**
* @internal prvCpssLogParamLibDbGet_NOT_DEFINED function
* @endinternal
*
* @brief   Functions DB entry for not defined yet lib
*/
void prvCpssLogParamLibDbGet_NOT_DEFINED
(
    OUT const PRV_CPSS_LOG_FUNC_ENTRY_STC ** logFuncDbPtrPtr,
    OUT GT_U32 * logFuncDbSizePtr
)
{
    *logFuncDbPtrPtr = 0;
    *logFuncDbSizePtr = 0;
}

#ifdef CHX_FAMILY

/**
* @internal prvCpssLogDxChExtLibBind function
* @endinternal
*
* @brief   Bind callback function to get log DB for extended lib.
*
* @param[in] extLibId                 - ID of extended lib
* @param[in] dbGetFunc                - callback function
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong extLibId
* @retval GT_BAD_PTR               - on NULL pointer
*
*/
GT_STATUS prvCpssLogDxChExtLibBind
(
    IN GT_U32  extLibId,
    IN PRV_CPSS_LOG_PARAM_LIB_DB_GET_FUNC dbGetFunc
)
{
    if (extLibId >= CPSS_LOG_LIB_LAST_CNS)
    {
        return GT_BAD_PARAM;
    }

    CPSS_NULL_PTR_CHECK_MAC(dbGetFunc);

    prvCpssLogParamLibDbArray_DX[extLibId] = dbGetFunc;
    return GT_OK;
}

#endif /* CHX_FAMILY */

/**
* @internal prvCpssLogInit function
* @endinternal
*
* @brief   Init log related info
*/
GT_STATUS prvCpssLogInit
(
    GT_VOID
)
{
    GT_U32 funcId; /* ID of lib for function ID to entry resolution algorithm */
    GT_STATUS rc;  /* return code */

/* Log history name */
#ifdef  WIN32
    cpssOsSprintf(PRV_LOG_GLOBAL_VAR_VOLATILE_GET(prvCpssLogHistoryName), "%s", "c:\\temp\\cpss_log_entry");
#else
    cpssOsSprintf(PRV_LOG_GLOBAL_VAR_VOLATILE_GET(prvCpssLogHistoryName), "%s", "/tmp/cpss_log_entry");
#endif

#ifdef SHARED_MEMORY
    prvLogCbInit();
#endif
    for (funcId = 0; funcId < CPSS_LOG_LIB_LAST_CNS; funcId++)
    {

#ifdef CHX_FAMILY
        switch (funcId)
        {
            case CPSS_LOG_LIB_APP_DRIVER_CALL_E:
                prvCpssLogParamLibDbArray_DX[funcId] = PRV_CPSS_LOG_PARAM_LIB_DB_DX(CPSS_LOG_LIB_APP_DRIVER_CALL);
                break;
            case CPSS_LOG_LIB_CNC_E:
                prvCpssLogParamLibDbArray_DX[funcId] = PRV_CPSS_LOG_PARAM_LIB_DB_DX(CPSS_LOG_LIB_CNC);
                break;
            case CPSS_LOG_LIB_COS_E:
                prvCpssLogParamLibDbArray_DX[funcId] = PRV_CPSS_LOG_PARAM_LIB_DB_DX(CPSS_LOG_LIB_COS);
                break;
            case CPSS_LOG_LIB_CUT_THROUGH_E:
                prvCpssLogParamLibDbArray_DX[funcId] = PRV_CPSS_LOG_PARAM_LIB_DB_DX(CPSS_LOG_LIB_CUT_THROUGH);
                break;
            case CPSS_LOG_LIB_BRIDGE_E:
                prvCpssLogParamLibDbArray_DX[funcId] = PRV_CPSS_LOG_PARAM_LIB_DB_DX(CPSS_LOG_LIB_BRIDGE);
                break;
            case CPSS_LOG_LIB_CSCD_E:
                prvCpssLogParamLibDbArray_DX[funcId] = PRV_CPSS_LOG_PARAM_LIB_DB_DX(CPSS_LOG_LIB_CSCD);
                break;
            case CPSS_LOG_LIB_IPFIX_E:
                prvCpssLogParamLibDbArray_DX[funcId] = PRV_CPSS_LOG_PARAM_LIB_DB_DX(CPSS_LOG_LIB_IPFIX);
                break;
            case CPSS_LOG_LIB_PCL_E:
                prvCpssLogParamLibDbArray_DX[funcId] = PRV_CPSS_LOG_PARAM_LIB_DB_DX(CPSS_LOG_LIB_PCL);
                break;
            case CPSS_LOG_LIB_FABRIC_E:
                prvCpssLogParamLibDbArray_DX[funcId] = PRV_CPSS_LOG_PARAM_LIB_DB_DX(CPSS_LOG_LIB_FABRIC);
                break;
            case CPSS_LOG_LIB_PHY_E:
                prvCpssLogParamLibDbArray_DX[funcId] = PRV_CPSS_LOG_PARAM_LIB_DB_DX(CPSS_LOG_LIB_PHY);
                break;
            case CPSS_LOG_LIB_MIRROR_E:
                prvCpssLogParamLibDbArray_DX[funcId] = PRV_CPSS_LOG_PARAM_LIB_DB_DX(CPSS_LOG_LIB_MIRROR);
                break;
            case CPSS_LOG_LIB_POLICER_E:
                prvCpssLogParamLibDbArray_DX[funcId] = PRV_CPSS_LOG_PARAM_LIB_DB_DX(CPSS_LOG_LIB_POLICER);
                break;
            case CPSS_LOG_LIB_MULTI_PORT_GROUP_E:
                prvCpssLogParamLibDbArray_DX[funcId] = PRV_CPSS_LOG_PARAM_LIB_DB_DX(CPSS_LOG_LIB_MULTI_PORT_GROUP);
                break;
            case CPSS_LOG_LIB_PROTECTION_E:
                prvCpssLogParamLibDbArray_DX[funcId] = PRV_CPSS_LOG_PARAM_LIB_DB_DX(CPSS_LOG_LIB_PROTECTION);
                break;
            case CPSS_LOG_LIB_NST_E:
                prvCpssLogParamLibDbArray_DX[funcId] = PRV_CPSS_LOG_PARAM_LIB_DB_DX(CPSS_LOG_LIB_NST);
                break;
            case CPSS_LOG_LIB_SYSTEM_RECOVERY_E:
                prvCpssLogParamLibDbArray_DX[funcId] = PRV_CPSS_LOG_PARAM_LIB_DB_DX(CPSS_LOG_LIB_SYSTEM_RECOVERY);
                break;
            case CPSS_LOG_LIB_OAM_E:
                prvCpssLogParamLibDbArray_DX[funcId] = PRV_CPSS_LOG_PARAM_LIB_DB_DX(CPSS_LOG_LIB_OAM);
                break;
            case CPSS_LOG_LIB_PTP_E:
                prvCpssLogParamLibDbArray_DX[funcId] = PRV_CPSS_LOG_PARAM_LIB_DB_DX(CPSS_LOG_LIB_PTP);
                break;
            case CPSS_LOG_LIB_TRUNK_E:
                prvCpssLogParamLibDbArray_DX[funcId] = PRV_CPSS_LOG_PARAM_LIB_DB_DX(CPSS_LOG_LIB_TRUNK);
                break;
            case CPSS_LOG_LIB_TTI_E:
                prvCpssLogParamLibDbArray_DX[funcId] = PRV_CPSS_LOG_PARAM_LIB_DB_DX(CPSS_LOG_LIB_TTI);
                break;
            case CPSS_LOG_LIB_LATENCY_MONITORING_E:
                prvCpssLogParamLibDbArray_DX[funcId] = PRV_CPSS_LOG_PARAM_LIB_DB_DX(CPSS_LOG_LIB_LATENCY_MONITORING);
                break;
            case CPSS_LOG_LIB_PHA_E:
                prvCpssLogParamLibDbArray_DX[funcId] = PRV_CPSS_LOG_PARAM_LIB_DB_DX(CPSS_LOG_LIB_PHA);
                break;
            case CPSS_LOG_LIB_TUNNEL_E:
                prvCpssLogParamLibDbArray_DX[funcId] = PRV_CPSS_LOG_PARAM_LIB_DB_DX(CPSS_LOG_LIB_TUNNEL);
                break;
            case CPSS_LOG_LIB_VNT_E:
                prvCpssLogParamLibDbArray_DX[funcId] = PRV_CPSS_LOG_PARAM_LIB_DB_DX(CPSS_LOG_LIB_VNT);
                break;
            case CPSS_LOG_LIB_RESOURCE_MANAGER_E:
                prvCpssLogParamLibDbArray_DX[funcId] = PRV_CPSS_LOG_PARAM_LIB_DB_DX(CPSS_LOG_LIB_RESOURCE_MANAGER);
                break;
            case CPSS_LOG_LIB_VERSION_E:
                prvCpssLogParamLibDbArray_DX[funcId] = PRV_CPSS_LOG_PARAM_LIB_DB_DX(CPSS_LOG_LIB_VERSION);
                break;

            #if defined (INCLUDE_TM)
            case CPSS_LOG_LIB_TM_E:
                prvCpssLogParamLibDbArray_DX[funcId] = PRV_CPSS_LOG_PARAM_LIB_DB_DX(CPSS_LOG_LIB_TM);
                break;
            case CPSS_LOG_LIB_TM_GLUE_E:
                prvCpssLogParamLibDbArray_DX[funcId] = PRV_CPSS_LOG_PARAM_LIB_DB_DX(CPSS_LOG_LIB_TM_GLUE);
                break;
            #endif /*#if defined (INCLUDE_TM)*/

            case CPSS_LOG_LIB_TCAM_E:
                prvCpssLogParamLibDbArray_DX[funcId] = PRV_CPSS_LOG_PARAM_LIB_DB_DX(CPSS_LOG_LIB_TCAM);
                break;
            case CPSS_LOG_LIB_EXACT_MATCH_E:
                prvCpssLogParamLibDbArray_DX[funcId] = PRV_CPSS_LOG_PARAM_LIB_DB_DX(CPSS_LOG_LIB_EXACT_MATCH);
                break;
            case CPSS_LOG_LIB_SMI_E:
                prvCpssLogParamLibDbArray_DX[funcId] = PRV_CPSS_LOG_PARAM_LIB_DB_DX(CPSS_LOG_LIB_SMI);
                break;
            case CPSS_LOG_LIB_I2C_E:
                prvCpssLogParamLibDbArray_DX[funcId] = PRV_CPSS_LOG_PARAM_LIB_DB_DX(CPSS_LOG_LIB_I2C);
                break;
            case CPSS_LOG_LIB_IP_E:
                prvCpssLogParamLibDbArray_DX[funcId] = PRV_CPSS_LOG_PARAM_LIB_DB_DX(CPSS_LOG_LIB_IP);
                break;
            case CPSS_LOG_LIB_LPM_E:
                prvCpssLogParamLibDbArray_DX[funcId] = PRV_CPSS_LOG_PARAM_LIB_DB_DX(CPSS_LOG_LIB_LPM);
                break;
            case CPSS_LOG_LIB_IP_LPM_E:
                prvCpssLogParamLibDbArray_DX[funcId] = PRV_CPSS_LOG_PARAM_LIB_DB_DX(CPSS_LOG_LIB_IP_LPM);
                break;
            case CPSS_LOG_LIB_L2_MLL_E:
                prvCpssLogParamLibDbArray_DX[funcId] = PRV_CPSS_LOG_PARAM_LIB_DB_DX(CPSS_LOG_LIB_L2_MLL);
                break;
            case CPSS_LOG_LIB_LOGICAL_TARGET_E:
                prvCpssLogParamLibDbArray_DX[funcId] = PRV_CPSS_LOG_PARAM_LIB_DB_DX(CPSS_LOG_LIB_LOGICAL_TARGET);
                break;
            case CPSS_LOG_LIB_VIRTUAL_TCAM_E:
                prvCpssLogParamLibDbArray_DX[funcId] = PRV_CPSS_LOG_PARAM_LIB_DB_DX(CPSS_LOG_LIB_VIRTUAL_TCAM);
                break;
            case CPSS_LOG_LIB_CONFIG_E:
                prvCpssLogParamLibDbArray_DX[funcId] = PRV_CPSS_LOG_PARAM_LIB_DB_DX(CPSS_LOG_LIB_CONFIG);
                break;

            case CPSS_LOG_LIB_HW_INIT_E:
                prvCpssLogParamLibDbArray_DX[funcId] = PRV_CPSS_LOG_PARAM_LIB_DB_DX(CPSS_LOG_LIB_HW_INIT);
                break;
            case CPSS_LOG_LIB_DIAG_E:
                prvCpssLogParamLibDbArray_DX[funcId] = PRV_CPSS_LOG_PARAM_LIB_DB_DX(CPSS_LOG_LIB_DIAG);
                break;
            case CPSS_LOG_LIB_NETWORK_IF_E:
                prvCpssLogParamLibDbArray_DX[funcId] = PRV_CPSS_LOG_PARAM_LIB_DB_DX(CPSS_LOG_LIB_NETWORK_IF);
                break;
            case CPSS_LOG_LIB_PORT_E:
                prvCpssLogParamLibDbArray_DX[funcId] = PRV_CPSS_LOG_PARAM_LIB_DB_DX(CPSS_LOG_LIB_PORT);
                break;
            case CPSS_LOG_LIB_INIT_E:
                prvCpssLogParamLibDbArray_DX[funcId] = PRV_CPSS_LOG_PARAM_LIB_DB_DX(CPSS_LOG_LIB_INIT);
                break;
            case CPSS_LOG_LIB_PACKET_ANALYZER_E:
                prvCpssLogParamLibDbArray_DX[funcId] = PRV_CPSS_LOG_PARAM_LIB_DB_DX(CPSS_LOG_LIB_PACKET_ANALYZER);
                break;
            case CPSS_LOG_LIB_FLOW_MANAGER_E:
                prvCpssLogParamLibDbArray_DX[funcId] = PRV_CPSS_LOG_PARAM_LIB_DB_DX(CPSS_LOG_LIB_FLOW_MANAGER);
                break;
            case CPSS_LOG_LIB_BRIDGE_FDB_MANAGER_E:
                prvCpssLogParamLibDbArray_DX[funcId] = PRV_CPSS_LOG_PARAM_LIB_DB_DX(CPSS_LOG_LIB_BRIDGE_FDB_MANAGER);
                break;
            case CPSS_LOG_LIB_PPU_E:
                prvCpssLogParamLibDbArray_DX[funcId] = PRV_CPSS_LOG_PARAM_LIB_DB_DX(CPSS_LOG_LIB_PPU);
                break;
            case CPSS_LOG_LIB_EXACT_MATCH_MANAGER_E:
                prvCpssLogParamLibDbArray_DX[funcId] = PRV_CPSS_LOG_PARAM_LIB_DB_DX(CPSS_LOG_LIB_EXACT_MATCH_MANAGER);
                break;
            case CPSS_LOG_LIB_MAC_SEC_E:
                prvCpssLogParamLibDbArray_DX[funcId] = PRV_CPSS_LOG_PARAM_LIB_DB_DX(CPSS_LOG_LIB_MAC_SEC);
                break;
            case CPSS_LOG_LIB_PTP_MANAGER_E:
                prvCpssLogParamLibDbArray_DX[funcId] = PRV_CPSS_LOG_PARAM_LIB_DB_DX(CPSS_LOG_LIB_PTP_MANAGER);
                break;
            case CPSS_LOG_LIB_HSR_PRP_E:
                prvCpssLogParamLibDbArray_DX[funcId] = PRV_CPSS_LOG_PARAM_LIB_DB_DX(CPSS_LOG_LIB_HSR_PRP);
                break;
            case CPSS_LOG_LIB_STREAM_E:
                prvCpssLogParamLibDbArray_DX[funcId] = PRV_CPSS_LOG_PARAM_LIB_DB_DX(CPSS_LOG_LIB_STREAM);
                break;
            case CPSS_LOG_LIB_IPFIX_MANAGER_E:
                prvCpssLogParamLibDbArray_DX[funcId] = PRV_CPSS_LOG_PARAM_LIB_DB_DX(CPSS_LOG_LIB_IPFIX_MANAGER);
                break;
#ifdef DRAGONITE_TYPE_A1
            case CPSS_LOG_LIB_DRAGONITE_E:
                prvCpssLogParamLibDbArray_DX[funcId] = PRV_CPSS_LOG_PARAM_LIB_DB_DX(CPSS_LOG_LIB_DRAGONITE);
                break;
#endif

            case CPSS_LOG_LIB_CONFIG_COMMON_E:
                prvCpssLogParamLibDbArray_DX[funcId] = PRV_CPSS_LOG_PARAM_LIB_DB_DX(CPSS_LOG_LIB_CONFIG_COMMON);
                break;
            case CPSS_LOG_LIB_SYSTEM_RECOVERY_COMMON_E:
                prvCpssLogParamLibDbArray_DX[funcId] = PRV_CPSS_LOG_PARAM_LIB_DB_DX(CPSS_LOG_LIB_SYSTEM_RECOVERY_COMMON);
                break;
            case CPSS_LOG_LIB_HW_INIT_COMMON_E:
                prvCpssLogParamLibDbArray_DX[funcId] = PRV_CPSS_LOG_PARAM_LIB_DB_DX(CPSS_LOG_LIB_HW_INIT_COMMON);
                break;
            default:
                prvCpssLogParamLibDbArray_DX[funcId] = prvCpssLogParamLibDbGet_NOT_DEFINED;
                break;
        }
#endif /* CHX_FAMILY */



#ifdef PX_FAMILY
        switch (funcId)
        {
            case CPSS_LOG_LIB_APP_DRIVER_CALL_E:
                prvCpssLogParamLibDbArray_PX[funcId] = PRV_CPSS_LOG_PARAM_LIB_DB_PX(CPSS_LOG_LIB_APP_DRIVER_CALL);
                break;
            case CPSS_LOG_LIB_CNC_E:
                prvCpssLogParamLibDbArray_PX[funcId] = PRV_CPSS_LOG_PARAM_LIB_DB_PX(CPSS_LOG_LIB_CNC);
                break;
            case CPSS_LOG_LIB_COS_E:/* also in PX */
                prvCpssLogParamLibDbArray_PX[funcId] = PRV_CPSS_LOG_PARAM_LIB_DB_PX(CPSS_LOG_LIB_COS);
                break;
            case CPSS_LOG_LIB_CUT_THROUGH_E:/* also in PX */
                prvCpssLogParamLibDbArray_PX[funcId] = PRV_CPSS_LOG_PARAM_LIB_DB_PX(CPSS_LOG_LIB_CUT_THROUGH);
                break;
            case CPSS_LOG_LIB_CONFIG_E:
                prvCpssLogParamLibDbArray_PX[funcId] = PRV_CPSS_LOG_PARAM_LIB_DB_PX(CPSS_LOG_LIB_CONFIG);
                break;

            case CPSS_LOG_LIB_HW_INIT_E:
                prvCpssLogParamLibDbArray_PX[funcId] = PRV_CPSS_LOG_PARAM_LIB_DB_PX(CPSS_LOG_LIB_HW_INIT);
                break;
            case CPSS_LOG_LIB_DIAG_E:
                prvCpssLogParamLibDbArray_PX[funcId] = PRV_CPSS_LOG_PARAM_LIB_DB_PX(CPSS_LOG_LIB_DIAG);
                break;
            case CPSS_LOG_LIB_NETWORK_IF_E:
                prvCpssLogParamLibDbArray_PX[funcId] = PRV_CPSS_LOG_PARAM_LIB_DB_PX(CPSS_LOG_LIB_NETWORK_IF);
                break;
            case CPSS_LOG_LIB_PORT_E:
                prvCpssLogParamLibDbArray_PX[funcId] = PRV_CPSS_LOG_PARAM_LIB_DB_PX(CPSS_LOG_LIB_PORT);
                break;
            case CPSS_LOG_LIB_INIT_E:
                prvCpssLogParamLibDbArray_PX[funcId] = PRV_CPSS_LOG_PARAM_LIB_DB_PX(CPSS_LOG_LIB_INIT);
                break;

#ifdef DRAGONITE_TYPE_A1
            case CPSS_LOG_LIB_DRAGONITE_E:
                /*
                prvCpssLogParamLibDbArray_PX[funcId] = PRV_CPSS_LOG_PARAM_LIB_DB_PX(CPSS_LOG_LIB_DRAGONITE);
                */
                break;
#endif

            case CPSS_LOG_LIB_INGRESS_E:
                prvCpssLogParamLibDbArray_PX[funcId] = PRV_CPSS_LOG_PARAM_LIB_DB_PX(CPSS_LOG_LIB_INGRESS);
                break;
            case CPSS_LOG_LIB_EGRESS_E:
                prvCpssLogParamLibDbArray_PX[funcId] = PRV_CPSS_LOG_PARAM_LIB_DB_PX(CPSS_LOG_LIB_EGRESS);
                break;
            case CPSS_LOG_LIB_CONFIG_COMMON_E:
                prvCpssLogParamLibDbArray_PX[funcId] = PRV_CPSS_LOG_PARAM_LIB_DB_PX(CPSS_LOG_LIB_CONFIG_COMMON);
                break;
            case CPSS_LOG_LIB_SYSTEM_RECOVERY_COMMON_E:
                prvCpssLogParamLibDbArray_PX[funcId] = PRV_CPSS_LOG_PARAM_LIB_DB_PX(CPSS_LOG_LIB_SYSTEM_RECOVERY_COMMON);
                break;
            case CPSS_LOG_LIB_HW_INIT_COMMON_E:
                prvCpssLogParamLibDbArray_PX[funcId] = PRV_CPSS_LOG_PARAM_LIB_DB_PX(CPSS_LOG_LIB_HW_INIT_COMMON);
                break;
            case CPSS_LOG_LIB_SMI_E:
                prvCpssLogParamLibDbArray_PX[funcId] = PRV_CPSS_LOG_PARAM_LIB_DB_PX(CPSS_LOG_LIB_SMI);
                break;
            case CPSS_LOG_LIB_I2C_E:
                prvCpssLogParamLibDbArray_PX[funcId] = PRV_CPSS_LOG_PARAM_LIB_DB_PX(CPSS_LOG_LIB_I2C);
                break;

            default:
                prvCpssLogParamLibDbArray_PX[funcId] = prvCpssLogParamLibDbGet_NOT_DEFINED;
                break;
        }
#endif /* PX_FAMILY */


    }

    /* init variables */
    PRV_LOG_GLOBAL_NON_VOLATILE_VAR_DIRECT_GET(prvCpssLogFormat) = CPSS_LOG_API_FORMAT_ALL_PARAMS_E;
    PRV_LOG_GLOBAL_NON_VOLATILE_VAR_DIRECT_GET(prvCpssLogLineNumberEnabled) = GT_TRUE;
    PRV_LOG_GLOBAL_NON_VOLATILE_VAR_DIRECT_GET(timeFormat) = CPSS_LOG_TIME_FORMAT_NO_DATE_TIME_E;

    /* create data base pools and tree */
    rc = prvCpssLogContextDbCreate();
    return rc;
}

/**
* @internal prvCpssLogApiFunctionEntryGet function
* @endinternal
*
* @brief   Get function entry from specific lib database by function ID
*
* @param[in] functionId               - function identifier
* @param[in] logPhase                 - API call log phase - enter or exit
*                                       None.
*/
static void prvCpssLogApiFunctionEntryGet
(
    IN GT_U32 functionId,
    IN PRV_CPSS_LOG_PHASE_ENT  logPhase GT_UNUSED,
    OUT const PRV_CPSS_LOG_FUNC_ENTRY_STC ** functionEntryPtrPtr,
    OUT GT_U32 *mngFunctionIdPtr
)
{
    /* 0 - common; 1 - mainPpDrv; 2 - mainpxDrv */
    GT_U32 funcLibFamily = (functionId >> 24) & 0xFF;
    GT_U32 funcLibId = (functionId >> 16) & 0xFF;        /* 255 CPSS_LOG_LIB_XX max */
    GT_U32 funcDbIdx = functionId & 0xFFFF;

    const PRV_CPSS_LOG_FUNC_ENTRY_STC * functionDbPtr = 0;
    GT_U32 functionEntryDbSize;

    (void)logPhase; /* suppress 'unused variable' warning in VC10 */

    /* Null entry pointer by default */
    *functionEntryPtrPtr = 0;
    *mngFunctionIdPtr = functionId;

    if (funcLibId >= CPSS_LOG_LIB_LAST_CNS)
    {
        PRV_CPSS_LOG_AND_HISTORY_MAC(PRV_CPSS_LOG_PHASE_ERROR_E, PRV_LOG_GLOBAL_NON_VOLATILE_VAR_DIRECT_GET(prvCpssLogContextEntryPtr)->prvCpssLogApiContextLib, CPSS_LOG_TYPE_ERROR_E, " ERROR: lib %d is out of range %d \n", funcLibId, CPSS_LOG_LIB_LAST_CNS);
        return;
    }


    /* Get API log entries database from specific lib */
    switch (funcLibFamily)
    {

#ifdef CHX_FAMILY
        case 1:
            prvCpssLogParamLibDbArray_DX[funcLibId](&functionDbPtr, &functionEntryDbSize);
            break;
#endif

#ifdef PX_FAMILY
        case 2:
            prvCpssLogParamLibDbArray_PX[funcLibId](&functionDbPtr, &functionEntryDbSize);
            break;
#endif

        case 0:
        default:
#ifdef PX_FAMILY
            prvCpssLogParamLibDbArray_PX[funcLibId](&functionDbPtr, &functionEntryDbSize);
#else
            prvCpssLogParamLibDbArray_DX[funcLibId](&functionDbPtr, &functionEntryDbSize);
#endif
            break;
    }


    if (funcDbIdx >= functionEntryDbSize)
    {
        PRV_CPSS_LOG_AND_HISTORY_MAC(PRV_CPSS_LOG_PHASE_ERROR_E, PRV_LOG_GLOBAL_NON_VOLATILE_VAR_DIRECT_GET(prvCpssLogContextEntryPtr)->prvCpssLogApiContextLib, CPSS_LOG_TYPE_ERROR_E, " ERROR: functionId[%d] is out of range \n", funcDbIdx);
        return;
    }

    /* Get specific API function log entry from database */
    *functionEntryPtrPtr = &functionDbPtr[funcDbIdx];

    if (funcLibId >= CPSS_LOG_LIB_ALL_E)
    {
        /* convert special IDs of common libs to manageable IDs*/
        switch (funcLibId)
        {
            case CPSS_LOG_LIB_CONFIG_COMMON_E:
                funcLibId = CPSS_LOG_LIB_CONFIG_E;
                break;
            case CPSS_LOG_LIB_SYSTEM_RECOVERY_COMMON_E:
                funcLibId = CPSS_LOG_LIB_SYSTEM_RECOVERY_E;
                break;
            case CPSS_LOG_LIB_HW_INIT_COMMON_E:
                funcLibId = CPSS_LOG_LIB_HW_INIT_E;
                break;
            case CPSS_LOG_LIB_MAC_SEC_EXT_E:
                funcLibId = CPSS_LOG_LIB_MAC_SEC_E;
                break;
            default: break;
        }
        *mngFunctionIdPtr = funcDbIdx | (funcLibId << 16) | (funcLibFamily << 24);
    }
}

/**
* @internal prvCpssLogApiEnter function
* @endinternal
*
* @brief   Internal API for API/Internal/Driver function call log - enter function.
*
* @param[in] functionId               - function identifier (of the function we just entered into).
*                                      ... - the parameters values of the function we just entered into.
*                                       None.
*/
void prvCpssLogApiEnter
(
    IN GT_U32 functionId,
    ...
)
{
    const PRV_CPSS_LOG_FUNC_ENTRY_STC  *funcEntryPtr;
    va_list args;
    CPSS_LOG_TYPE_ENT logType;
    GT_STATUS rc;
    GT_U32 mngFunctionId;
    /* Lock the Logger */
    CPSS_ZERO_LEVEL_API_LOCK_NO_RETURN_MAC;

    /* get current log context entry */
    rc = prvCpssLogContextEntryGet(&PRV_LOG_GLOBAL_NON_VOLATILE_VAR_DIRECT_GET(prvCpssLogContextEntryPtr));
    if (rc != GT_OK)
    {
        cpssOsLog(CPSS_LOG_LIB_ALL_E, CPSS_LOG_TYPE_ENTRY_LEVEL_FUNCTION_E, prvCpssLogErrorMsgLogContextCurrentPointer, rc);
        CPSS_ZERO_LEVEL_API_UNLOCK_MAC;
        return;
    }

    /* Get function DB Pointer */
    prvCpssLogApiFunctionEntryGet(functionId, PRV_CPSS_LOG_PHASE_ENTER_E, &funcEntryPtr, &mngFunctionId);
    if (funcEntryPtr == 0)
    {
        CPSS_ZERO_LEVEL_API_UNLOCK_MAC;
        return;
    }

    /* Set API context */
    prvCpssLogApiLibContextSet(functionId, PRV_CPSS_LOG_PHASE_ENTER_E, mngFunctionId);
    /* Get log type after API/lib context set */
    logType = PRV_CPSS_LOG_TYPE_GET(functionId);

    /* Check API log trigger conditions */
    if (prvCpssLogApiFunctionLogTrigger(logType) == GT_FALSE)
    {
        CPSS_ZERO_LEVEL_API_UNLOCK_MAC;
        return;
    }

    /* Log header with name of function */
    PRV_CPSS_LOG_AND_HISTORY_MAC(PRV_CPSS_LOG_PHASE_ENTER_E, PRV_LOG_GLOBAL_NON_VOLATILE_VAR_DIRECT_GET(prvCpssLogContextEntryPtr)->prvCpssLogApiContextLib, logType, "Enter: %s\n",funcEntryPtr->apiName);

    va_start(args, functionId);

    prvCpssLogCpssLogEnterParams(functionId, logType, funcEntryPtr, &args);

    va_end(args);

    CPSS_ZERO_LEVEL_API_UNLOCK_MAC;
}

/**
* @internal prvCpssLogApiExit function
* @endinternal
*
* @brief   Internal API for API/Internal function call log - exit function.
*
* @param[in] functionId               - function identifier (of the function we just exit from).
* @param[in] rc                       - the return code of the function.
*                                       None.
*/
void prvCpssLogApiExit
(
    IN GT_U32 functionId,
    IN GT_STATUS rc
)
{
    const PRV_CPSS_LOG_FUNC_ENTRY_STC  *funcEntryPtr;
    CPSS_LOG_TYPE_ENT logType;
    CPSS_LOG_LIB_ENT  contextLib;
    GT_STATUS retVal;
    GT_U32 mngFunctionId;

    /* Lock the Logger */
    CPSS_ZERO_LEVEL_API_LOCK_NO_RETURN_MAC;

    /* get current log context entry */
    retVal = prvCpssLogContextEntryGet(&PRV_LOG_GLOBAL_NON_VOLATILE_VAR_DIRECT_GET(prvCpssLogContextEntryPtr));
    if (retVal != GT_OK)
    {
        cpssOsLog(CPSS_LOG_LIB_ALL_E, CPSS_LOG_TYPE_ENTRY_LEVEL_FUNCTION_E, prvCpssLogErrorMsgLogContextCurrentPointer, retVal);
        CPSS_ZERO_LEVEL_API_UNLOCK_MAC;
        return;
    }

    /* Function DB Pointer */
    prvCpssLogApiFunctionEntryGet(functionId, PRV_CPSS_LOG_PHASE_EXIT_E, &funcEntryPtr, &mngFunctionId);
    if (funcEntryPtr == 0)
    {
        CPSS_ZERO_LEVEL_API_UNLOCK_MAC;
        return;
    }

    /* Get log type after API/lib context set */
    logType = PRV_CPSS_LOG_TYPE_GET(functionId);

    /* Check API log trigger conditions */
    if (prvCpssLogApiFunctionLogTrigger(logType) == GT_TRUE)
    {
        contextLib = PRV_LOG_GLOBAL_NON_VOLATILE_VAR_DIRECT_GET(prvCpssLogContextEntryPtr)->prvCpssLogApiContextLib;

        /* log header with name of function */
        PRV_CPSS_LOG_AND_HISTORY_MAC(PRV_CPSS_LOG_PHASE_EXIT_E, contextLib, logType, "Exit: %s\n",funcEntryPtr->apiName);

        prvCpssLogCpssLogExitParams(functionId, logType, funcEntryPtr);

        /* log API return code */
        PRV_CPSS_LOG_VAR_ENUM_MAP_MAC(rc, PRV_CPSS_LOG_FUNC_RET_CODE_ENT);
    }

    /* Reset API context */
    prvCpssLogApiLibContextSet(functionId, PRV_CPSS_LOG_PHASE_EXIT_E, mngFunctionId);

    CPSS_ZERO_LEVEL_API_UNLOCK_MAC;
}

/**
* @internal prvCpssLogErrorCommon function
* @endinternal
*
* @brief   Log API for API/Internal function error message
*
* @param[in] functionName             - name of function that generates error.
* @param[in] fileName                 - name of file that generates error.
* @param[in] line                     -  number in file, may be excluded from log by global configuration.
* @param[in] rc                       - the return code of the function.
* @param[in] errorMsgPtr              - cpssOsLog formatted message.
*                                       None.
*/
GT_VOID prvCpssLogErrorCommon
(
    IN const char *functionName,
    IN const char *fileName,
    IN GT_U32 line,
    IN GT_STATUS rc,
    const char * errorMsgPtr
)
{
    CPSS_LOG_LIB_ENT  contextLib = PRV_LOG_GLOBAL_NON_VOLATILE_VAR_DIRECT_GET(prvCpssLogContextEntryPtr)->prvCpssLogApiContextLib;
    GT_U8  * enumStringPtr;
    GT_U8  * timeStampPtr = (GT_U8 *)PRV_LOG_GLOBAL_VAR_VOLATILE_GET(buffer1);
    GT_U32 timeStampSize = sizeof(PRV_LOG_GLOBAL_VAR_VOLATILE_GET(buffer1));

    cpssOsMemSet(PRV_LOG_GLOBAL_VAR_VOLATILE_GET(buffer1), 0, sizeof(PRV_LOG_GLOBAL_VAR_VOLATILE_GET(buffer1)));

    switch (PRV_LOG_GLOBAL_NON_VOLATILE_VAR_DIRECT_GET(timeFormat))
    {
        case CPSS_LOG_TIME_FORMAT_DATE_TIME_E:
            /* Get current time */
            cpssOsStrfTime((char *)timeStampPtr, timeStampSize, CPSS_LOG_TIME_FORMAT_DATE_TIME_CNS, cpssOsTime());
            break;
        case CPSS_LOG_TIME_FORMAT_SHORT_DATE_TIME_E:
            /* Get current time */
            cpssOsStrfTime((char *)timeStampPtr, timeStampSize, CPSS_LOG_TIME_FORMAT_SHORT_DATE_TIME_CNS, cpssOsTime());
            break;
        case CPSS_LOG_TIME_FORMAT_SHORT_DATE_ISO_TIME_E:
            /* Get current time */
            cpssOsStrfTime((char *)timeStampPtr, timeStampSize, CPSS_LOG_TIME_FORMAT_SHORT_DATE_ISO_TIME_CNS, cpssOsTime());
            break;
        case CPSS_LOG_TIME_FORMAT_NO_DATE_TIME_E:
            timeStampPtr = 0;
            break;
        default:
            PRV_CPSS_LOG_AND_HISTORY_MAC(PRV_CPSS_LOG_PHASE_ERROR_E, contextLib, CPSS_LOG_TYPE_ERROR_E, "Bad time format %d\n", PRV_LOG_GLOBAL_NON_VOLATILE_VAR_DIRECT_GET(timeFormat));
            return;
    }

    if (timeStampPtr)
    {
        /* log error timestamp */
        PRV_CPSS_LOG_AND_HISTORY_MAC(PRV_CPSS_LOG_PHASE_ERROR_E, contextLib, CPSS_LOG_TYPE_INFO_E, "%s ", timeStampPtr);
    }

    enumStringPtr = (GT_U8 *)PRV_LOG_GLOBAL_VAR_VOLATILE_GET(buffer1);
    /* Get API return code string */
    PRV_CPSS_LOG_ENUM_STRING_GET_MAC(rc, PRV_CPSS_LOG_FUNC_RET_CODE_ENT, enumStringPtr);

    /* log error message */
    if (GT_TRUE == PRV_LOG_GLOBAL_NON_VOLATILE_VAR_DIRECT_GET(prvCpssLogLineNumberEnabled))
    {
        PRV_CPSS_LOG_AND_HISTORY_MAC(PRV_CPSS_LOG_PHASE_ERROR_E, contextLib, CPSS_LOG_TYPE_ERROR_E,
                                     "ERROR %s in function: %s, file %s, line[%d]. %s\n",
                                     enumStringPtr, functionName, fileName, line, errorMsgPtr);
    }
    else
    {
        PRV_CPSS_LOG_AND_HISTORY_MAC(PRV_CPSS_LOG_PHASE_ERROR_E, contextLib, CPSS_LOG_TYPE_ERROR_E,
                                     "ERROR %s in function: %s, file %s. %s\n",
                                     enumStringPtr, functionName, fileName, errorMsgPtr);
    }

}

/**
* @internal prvCpssLogError function
* @endinternal
*
* @brief   Internal API for API/Internal function error log
*
* @param[in] functionName             - name of function that generates error.
* @param[in] fileName                 - name of file that generates error.
* @param[in] line                     -  number in file, may be excluded from log by global configuration.
* @param[in] rc                       - the return code of the function.
* @param[in] formatPtr                - cpssOsLog format starting.
*                                      ...               optional parameters according to formatPtr
*                                       None.
*/
GT_VOID prvCpssLogError
(
    IN const char *functionName,
    IN const char *fileName,
    IN GT_U32 line,
    IN GT_STATUS rc,
    IN const char * formatPtr,
    ...
)
{
    va_list args;
    GT_STATUS retVal;

    /* Lock the Logger */
    CPSS_ZERO_LEVEL_API_LOCK_NO_RETURN_MAC;

    /* get current log context entry */
    retVal = prvCpssLogContextEntryGet(&PRV_LOG_GLOBAL_NON_VOLATILE_VAR_DIRECT_GET(prvCpssLogContextEntryPtr));
    if (retVal != GT_OK)
    {
        cpssOsLog(CPSS_LOG_LIB_ALL_E, CPSS_LOG_TYPE_ENTRY_LEVEL_FUNCTION_E, prvCpssLogErrorMsgLogContextCurrentPointer, retVal);
        CPSS_ZERO_LEVEL_API_UNLOCK_MAC;
        return;
    }

    /* Need separate buffer for info message to prevent override data by recursive call using global buffer */
    va_start(args, formatPtr);
    cpssOsVsprintf(PRV_LOG_GLOBAL_VAR_VOLATILE_GET(buffer3), formatPtr, args);
    va_end(args);

    /* Call common error log function */
    prvCpssLogErrorCommon(functionName, fileName, line, rc, PRV_LOG_GLOBAL_VAR_VOLATILE_GET(buffer3));

    CPSS_ZERO_LEVEL_API_UNLOCK_MAC;
}

/**
* @internal prvCpssLogInformation function
* @endinternal
*
* @brief   Internal API for API/Internal function information log
*
* @param[in] functionName             - name of function that generates error.
* @param[in] fileName                 - name of file that generates error.
* @param[in] line                     -  number in file, may be excluded from log by global configuration.
* @param[in] formatPtr                - cpssOsLog format starting.
*                                      ...               optional parameters according to formatPtr
*                                       None.
*/
GT_VOID prvCpssLogInformation
(
    IN const char *functionName,
    IN const char *fileName,
    IN GT_U32 line,
    IN const char * formatPtr,
    ...
)
{
    CPSS_LOG_LIB_ENT  contextLib;
    va_list args;
    GT_STATUS rc;

    /* Lock the Logger */
    CPSS_ZERO_LEVEL_API_LOCK_NO_RETURN_MAC;

    /* get current log context entry */
    rc = prvCpssLogContextEntryGet(&PRV_LOG_GLOBAL_NON_VOLATILE_VAR_DIRECT_GET(prvCpssLogContextEntryPtr));
    if (rc != GT_OK)
    {
        cpssOsLog(CPSS_LOG_LIB_ALL_E, CPSS_LOG_TYPE_ENTRY_LEVEL_FUNCTION_E, prvCpssLogErrorMsgLogContextCurrentPointer, rc);
        CPSS_ZERO_LEVEL_API_UNLOCK_MAC;
        return;
    }

    contextLib = PRV_LOG_GLOBAL_NON_VOLATILE_VAR_DIRECT_GET(prvCpssLogContextEntryPtr)->prvCpssLogApiContextLib;

    /* Need separate buffer for info message to prevent override data by recursive call using global buffer */
    va_start(args, formatPtr);
    cpssOsVsprintf(PRV_LOG_GLOBAL_VAR_VOLATILE_GET(buffer4), formatPtr, args);
    va_end(args);

    /* log information message */
    if (GT_TRUE == PRV_LOG_GLOBAL_NON_VOLATILE_VAR_DIRECT_GET(prvCpssLogLineNumberEnabled))
    {
        PRV_CPSS_LOG_AND_HISTORY_MAC(PRV_CPSS_LOG_PHASE_INFO_E, contextLib, CPSS_LOG_TYPE_INFO_E,
                                     "Information: in function %s, file %s, line[%d]. %s\n",
                                     functionName, fileName, line, PRV_LOG_GLOBAL_VAR_VOLATILE_GET(buffer4));
    }
    else
    {
        PRV_CPSS_LOG_AND_HISTORY_MAC(PRV_CPSS_LOG_PHASE_INFO_E, contextLib, CPSS_LOG_TYPE_INFO_E,
                                     "Information: in function %s, file %s. %s\n",
                                     functionName, fileName, PRV_LOG_GLOBAL_VAR_VOLATILE_GET(buffer4));
    }

    CPSS_ZERO_LEVEL_API_UNLOCK_MAC;
}

/**
* @internal cpssLogEnableSet function
* @endinternal
*
* @brief   The function enables/disables Log feature.
*
* @param[in] enable                   - GT_TRUE for log  and GT_False for log disable
*                                       None.
*/
GT_VOID cpssLogEnableSet
(
     IN GT_BOOL enable
)
{
    PRV_NON_SHARED_GLOBAL_VAR_LOG_SET(enable);
}

/**
* @internal cpssLogEnableGet function
* @endinternal
*
* @brief   The function gets the status of the Log feature (enabled/disabled).
*
* @retval GT_TRUE                  - if the log is enabled
* @retval GT_FALSE                 - if the log is disabled
*/
GT_BOOL cpssLogEnableGet(GT_VOID)
{
    return PRV_NON_SHARED_GLOBAL_VAR_LOG_GET();
}

/**
* @internal cpssLogLibEnableSet function
* @endinternal
*
* @brief   The function enables/disables specific type of log information for
*         specific or all libs.
* @param[in] lib                      -  to enable/disable log for.
* @param[in] type                     - log type.
* @param[in] enable                   - GT_TRUE for log  and GT_FALSE for log disable
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - bad parameter lib or type
*/
GT_STATUS cpssLogLibEnableSet
(
    IN  CPSS_LOG_LIB_ENT    lib,
    IN  CPSS_LOG_TYPE_ENT   type,
    IN  GT_BOOL             enable
)
{
    GT_U32 i; /* iterator */
    GT_U32 k; /* iterator */

    if (lib > CPSS_LOG_LIB_ALL_E)
        return GT_BAD_PARAM;

    if (type > CPSS_LOG_TYPE_ALL_E)
        return GT_BAD_PARAM;

    if ((type != CPSS_LOG_TYPE_ALL_E) && (lib != CPSS_LOG_LIB_ALL_E))
    {
        /* specific lib and specific type */
        PRV_LOG_GLOBAL_NON_VOLATILE_VAR_DIRECT_GET(prvCpssLogLibsLogsTypesTable)[lib][type] = enable;
    }
    else if (lib != CPSS_LOG_LIB_ALL_E)
    {
        /* specific lib but all types */
        for (i = 0; i < CPSS_LOG_TYPE_ALL_E; i++)
        {
            PRV_LOG_GLOBAL_NON_VOLATILE_VAR_DIRECT_GET(prvCpssLogLibsLogsTypesTable)[lib][i] = enable;
        }
    }
    else if (type != CPSS_LOG_TYPE_ALL_E)
    {
        /* specific type but all libs */
        for (i = 0; i < CPSS_LOG_LIB_ALL_E; i++)
        {
            PRV_LOG_GLOBAL_NON_VOLATILE_VAR_DIRECT_GET(prvCpssLogLibsLogsTypesTable)[i][type] = enable;
        }
    }
    else
    {   /* all libs and all types */
        PRV_LOG_GLOBAL_NON_VOLATILE_VAR_DIRECT_GET(prvCpssLogLibsLogs_errorAllEnable) = GT_TRUE;/* error LOG from all LIBs (registered and unregistered) */

        for (i = 0; i < CPSS_LOG_LIB_ALL_E; i++)
        {
            for (k = 0; k < CPSS_LOG_TYPE_ALL_E; k++)
            {
                PRV_LOG_GLOBAL_NON_VOLATILE_VAR_DIRECT_GET(prvCpssLogLibsLogsTypesTable)[i][k] = enable;
            }
        }
    }

    for (i = 0; i < CPSS_LOG_LIB_ALL_E; i++)
    {
        if(PRV_LOG_GLOBAL_NON_VOLATILE_VAR_DIRECT_GET(prvCpssLogLibsLogsTypesTable)[i][CPSS_LOG_TYPE_ERROR_E] == GT_FALSE)
        {
            /* at least one registered LIB not need it  */
            break;
        }
    }

    if(i < CPSS_LOG_LIB_ALL_E)
    {
        /* at least one registered LIB not need 'ERROR LOG'  */
        PRV_LOG_GLOBAL_NON_VOLATILE_VAR_DIRECT_GET(prvCpssLogLibsLogs_errorAllEnable) = GT_FALSE;
    }
    else
    {
        /* ALL registered LIB need 'ERROR LOG' --> apply it to unregistered LIBs too */
        PRV_LOG_GLOBAL_NON_VOLATILE_VAR_DIRECT_GET(prvCpssLogLibsLogs_errorAllEnable) = GT_TRUE;
    }

    return GT_OK;
}

/**
* @internal cpssLogLibEnableGet function
* @endinternal
*
* @brief   The function gets the status of specific type of log information for
*         specific lib (enabled/disabled).
* @param[in] lib                      -  to get the log status for.
* @param[in] type                     - log type.
*
* @param[out] enablePtr                - GT_TRUE - log is enabled for the specific lib and log type.
*                                      GT_FALSE - log is enabled for the specific lib and log type.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - bad pointer enablePtr
* @retval GT_BAD_PARAM             - bad parameter lib or type
*/
GT_STATUS cpssLogLibEnableGet
(
    IN   CPSS_LOG_LIB_ENT   lib,
    IN   CPSS_LOG_TYPE_ENT  type,
    OUT  GT_BOOL            * enablePtr
)
{
    if(enablePtr &&
       PRV_LOG_GLOBAL_NON_VOLATILE_VAR_DIRECT_GET(prvCpssLogLibsLogs_errorAllEnable) == GT_TRUE &&
       type == CPSS_LOG_TYPE_ERROR_E)
    {
        /* we not care about 'lib' because we allow ALL LIBs :
           lib <  CPSS_LOG_LIB_ALL_E --> meaning registered   LIB
           lib == CPSS_LOG_LIB_ALL_E --> meaning unregistered LIB

        */

        /* special case in which we allow to print the ERROR */
        /* this case is valid when calling 'internal CPSS' function (but non-static)
           that will trigger ERROR (without call to any 'CPSS_LOG_API_ENTER_MAC'). */
        *enablePtr = GT_TRUE;
        return GT_OK;
    }

    if (enablePtr != NULL)
    {
        /* return GT_FALSE in case wrong IN parameters */
        *enablePtr = GT_FALSE;
    }

    if (lib >= CPSS_LOG_LIB_ALL_E)
        return GT_BAD_PARAM;

    if (type >= CPSS_LOG_TYPE_ALL_E)
        return GT_BAD_PARAM;

    if (enablePtr != NULL)
    {
        *enablePtr = PRV_LOG_GLOBAL_NON_VOLATILE_VAR_DIRECT_GET(prvCpssLogLibsLogsTypesTable)[lib][type];
        return GT_OK;
    }
    return GT_BAD_PTR;
}

/**
* @internal cpssLogApiFormatSet function
* @endinternal
*
* @brief   The function set format for API related log.
*
* @param[in] format                   -  of API related logs.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - bad parameter format
*/
GT_STATUS cpssLogApiFormatSet
(
    IN CPSS_LOG_API_FORMAT_ENT format
)
{
    PRV_LOG_GLOBAL_NON_VOLATILE_VAR_DIRECT_GET(prvCpssLogFormat)=format;
    return GT_OK;
}

/**
* @internal cpssLogApiFormatGet function
* @endinternal
*
* @brief   The function gets the format of API related log.
*
* @retval CPSS_LOG_API_FORMAT_NO_PARAMS_E - not documenting any paramter
* @retval CPSS_LOG_API_FORMAT_ALL_PARAMS_E - documenting all the parameters
*                                       values
* @retval CPSS_LOG_API_FORMAT_NON_ZERO_PARAMS_E - documenting only the non zero
*                                       parameters values
*/
CPSS_LOG_API_FORMAT_ENT cpssLogApiFormatGet(GT_VOID)
{
    return PRV_LOG_GLOBAL_NON_VOLATILE_VAR_DIRECT_GET(prvCpssLogFormat);
}

/**
* @internal cpssLogStackSizeSet function
* @endinternal
*
* @brief   Defines the size of the stack, how many function will be logged before
*         an error occurred.
* @param[in] size                     - the amount of functions.
*
* @retval GT_OK                    - on success
*/
GT_STATUS cpssLogStackSizeSet
(
    GT_U32 size
)
{
    CPSS_ZERO_LEVEL_API_LOCK_NO_RETURN_MAC;

    if (PRV_LOG_GLOBAL_VAR_VOLATILE_GET(currentHistoryEntryPtr) != NULL || PRV_LOG_GLOBAL_VAR_VOLATILE_GET(currentHistoryEntryNum) != 0) {
        prvCpssLogApiHistoryDbClose(&PRV_LOG_GLOBAL_VAR_VOLATILE_GET(currentHistoryEntryPtr), &PRV_LOG_GLOBAL_VAR_VOLATILE_GET(currentHistoryEntryNum), GT_TRUE);
    }
    PRV_LOG_GLOBAL_NON_VOLATILE_VAR_DIRECT_GET(prvCpssLogStackSize) = size;
    CPSS_ZERO_LEVEL_API_UNLOCK_MAC;

    return GT_OK;
}

/**
* @internal cpssLogStackSizeGet function
* @endinternal
*
* @brief   The function gets the size of the log stack
*/
GT_U32 cpssLogStackSizeGet(GT_VOID)
{
    return PRV_LOG_GLOBAL_NON_VOLATILE_VAR_DIRECT_GET(prvCpssLogStackSize);
}

/* Compare two enum value */
static int enumCompare
(
    const void *a,
    const void *b
)
{
    PRV_CPSS_ENUM_STRING_VALUE_PAIR_STC *a1=(PRV_CPSS_ENUM_STRING_VALUE_PAIR_STC*)a;
    PRV_CPSS_ENUM_STRING_VALUE_PAIR_STC *b1=(PRV_CPSS_ENUM_STRING_VALUE_PAIR_STC*)b;
    if (a1->enumValue==b1->enumValue)
    {
        return 0;
    }
    if (a1->enumValue<b1->enumValue)
    {
        return -1;
    }
    if (a1->enumValue>b1->enumValue)
    {
        return 1;
    }
    return 0;
}

/**
* @internal cpssLogTimeFormatSet function
* @endinternal
*
* @brief   The function sets log time format.
*
* @param[in] format                   - log time format.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - bad parameter format
*/
GT_STATUS cpssLogTimeFormatSet
(
    IN CPSS_LOG_TIME_FORMAT_ENT format
)
{
    switch (format)
    {
        case CPSS_LOG_TIME_FORMAT_DATE_TIME_E:
        case CPSS_LOG_TIME_FORMAT_SHORT_DATE_TIME_E:
        case CPSS_LOG_TIME_FORMAT_SHORT_DATE_ISO_TIME_E:
        case CPSS_LOG_TIME_FORMAT_NO_DATE_TIME_E:
            PRV_LOG_GLOBAL_NON_VOLATILE_VAR_DIRECT_GET(timeFormat) = format;
            break;
        default:
            return GT_BAD_PARAM;
    }

    return GT_OK;
}

/**
* @internal cpssLogDelete function
* @endinternal
*
* @brief  The function frees all allocations done by the lib.
*
* @retval GT_OK               - on success
* @retval GT_NOT_INITIALIZED  - on CPSS Log lib was not initialized
* @retval GT_FAIL             - on failure
*/
GT_STATUS cpssLogDelete
(
    GT_VOID
)
{
    GT_STATUS rc; /* return code */

    /* delete tree */
    rc = prvCpssAvlTreeDelete(PRV_LOG_GLOBAL_NON_VOLATILE_VAR_DIRECT_GET(prvCpssLogAvlTreeId), NULL, NULL);
    if(rc != GT_OK )
    {
        return rc;
    }

    PRV_LOG_GLOBAL_NON_VOLATILE_VAR_DIRECT_GET(prvCpssLogAvlTreeId) = 0;

    rc = prvCpssAvlMemPoolDelete(PRV_LOG_GLOBAL_NON_VOLATILE_VAR_DIRECT_GET(prvCpssLogAvlPoolId));
    if (rc != GT_OK)
    {
        return rc;
    }

    PRV_LOG_GLOBAL_NON_VOLATILE_VAR_DIRECT_GET(prvCpssLogAvlPoolId) = 0;

    /* delete pool */
    rc = cpssBmPoolDelete(PRV_LOG_GLOBAL_NON_VOLATILE_VAR_DIRECT_GET(prvCpssLogDbPoolId));
    if(rc != GT_OK )
    {
        return rc;
    }

    PRV_LOG_GLOBAL_NON_VOLATILE_VAR_DIRECT_GET(prvCpssLogDbPoolId) = 0;

    return GT_OK;
}

/**
* @internal prvCpssLogEnumMap function
* @endinternal
*
* @brief   The function to log mapped enum parameter
*
* @param[in] contextLib               -
*                                      lib log activity
* @param[in] logType                  -
*                                      type of the log
* @param[in] fieldName                - parameter/structure field name.
* @param[in] keyValue                 - lookup enum value
* @param[in] enumArray                - array of emumerator strings.
* @param[in] enumArrayEntries         - number of enum array entries.
* @param[in] enumArrayEntrySize       - array entry size
*                                       None.
*/
void prvCpssLogEnumMap
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              fieldName,
    IN GT_U32                   keyValue,
    IN void                   * enumArray,
    IN GT_U32                   enumArrayEntries,
    IN GT_U32                   enumArrayEntrySize
)
{
    GT_U8 * enumStringPtr = (GT_U8 *)PRV_LOG_GLOBAL_VAR_VOLATILE_GET(buffer1);

    /* Get enum string value by its key */
    prvCpssEnumStringValueGet(fieldName, keyValue, enumArray, enumArrayEntries, enumArrayEntrySize, enumStringPtr);
    /* Indent tabs  */
    prvCpssLogTabs(contextLib, logType);
    /* Log out enum string value */
    PRV_CPSS_LOG_AND_HISTORY_MAC(PRV_CPSS_LOG_PHASE_INFO_E, contextLib, logType, "%s = %s\n", fieldName, enumStringPtr);
}

/**
* @internal prvCpssEnumStringValueGet function
* @endinternal
*
* @brief   The function maps enum parameter to string value
*
* @param[in] fieldName                - parameter/structure field name.
* @param[in] keyValue                 - lookup enum value
* @param[in] enumArray                - array of emumerator strings.
* @param[in] enumArrayEntries         - number of enum array entries.
* @param[in] enumArrayEntrySize       - array entry size
*
* @param[out] keyStringPtr             - (pointer to) key value mapped string
*                                       None.
*/
void prvCpssEnumStringValueGet
(
    IN GT_CHAR_PTR              fieldName GT_UNUSED,
    IN GT_U32                   keyValue,
    IN void                   * enumArray,
    IN GT_U32                   enumArrayEntries,
    IN GT_U32                   enumArrayEntrySize,
    OUT GT_U8                 * keyStringPtr
)
{
    GT_STATUS rc;
    PRV_CPSS_ENUM_STRING_VALUE_PAIR_STC entry;
    PRV_CPSS_ENUM_STRING_VALUE_PAIR_STC * currentHistoryEntryPtr = &entry;
    (void)fieldName; /* suppress 'unused variable' warning in VC10 */

    currentHistoryEntryPtr->enumValue = keyValue;
    rc = cpssOsBsearch((void *)currentHistoryEntryPtr, (void *)enumArray, enumArrayEntries, enumArrayEntrySize, enumCompare, (void*)&currentHistoryEntryPtr);
    if (rc == GT_OK) {
        cpssOsSprintf((char *)keyStringPtr, "%s", currentHistoryEntryPtr->namePtr);
    } else {
        cpssOsSprintf((char *)keyStringPtr, "0x%08x", keyValue);
    }
}

/**
* @internal prvCpssLogStcNumber function
* @endinternal
*
* @brief   The function to log numeric parameter
*
* @param[in] contextLib               -
*                                      lib log activity
* @param[in] logType                  -
*                                      type of the log
* @param[in] fieldName                - structure field name
* @param[in] numValue                 - field value.
*                                       None.
*/
void prvCpssLogStcNumber
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              fieldName,
    IN GT_U32                   numValue
)
{
    PRV_CPSS_LOG_ZERO_VALUE_LOG_CHECK_MAC(numValue);
    prvCpssLogTabs(contextLib, logType);
    PRV_CPSS_LOG_AND_HISTORY_MAC(PRV_CPSS_LOG_PHASE_INFO_E, contextLib, logType, "%s = %u\n", fieldName, numValue);
}

/**
* @internal prvCpssLogStcUintptr function
* @endinternal
*
* @brief   The function to log GT_UINTPTR parameter
*
* @param[in] contextLib               -
*                                      lib log activity
* @param[in] logType                  -
*                                      type of the log
* @param[in] fieldName                - structure field name
* @param[in] numValue                 - field value.
*                                       None.
*/
void prvCpssLogStcUintptr
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              fieldName,
    IN GT_UINTPTR               numValue
)
{
    GT_CHAR *formatStr;
    PRV_CPSS_LOG_ZERO_VALUE_LOG_CHECK_MAC(numValue);
    prvCpssLogTabs(contextLib, logType);
    formatStr = (PRV_LOG_GLOBAL_NON_VOLATILE_VAR_DIRECT_GET(prvCpssLogPointerFormat) == CPSS_LOG_POINTER_FORMAT_PREFIX_E) ?
        "%s = (addr)%p\n" :  "%s = %p\n";
    PRV_CPSS_LOG_AND_HISTORY_MAC(PRV_CPSS_LOG_PHASE_INFO_E, contextLib, logType,
                                 formatStr, fieldName, numValue);
}

/**
* @internal prvCpssLogStcByte function
* @endinternal
*
* @brief   The function to log byte parameter
*
* @param[in] contextLib               -
*                                      lib log activity
* @param[in] logType                  -
*                                      type of the log
* @param[in] fieldName                - structure field name
*                                      numValue - field value.
*                                       None.
*/
void prvCpssLogStcByte
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              fieldName,
    IN GT_U8                    byteValue
)
{
    PRV_CPSS_LOG_ZERO_VALUE_LOG_CHECK_MAC(byteValue);
    prvCpssLogTabs(contextLib, logType);
    PRV_CPSS_LOG_AND_HISTORY_MAC(PRV_CPSS_LOG_PHASE_INFO_E, contextLib, logType, "%s = 0x%x\n", fieldName, byteValue);
}

/**
* @internal prvCpssLogParamFuncStc_GT_FLOAT32_PTR function
* @endinternal
*
* @brief   The function to log GT_FLOAT32 parameter by pointer
*
* @param[in] contextLib               -
*                                      lib log activity
* @param[in] logType                  -
*                                      type of the log
*                                      fieldName - structure field name
* @param[in] fieldPtr                 - (pointer to) field value
*                                       None.
*/
void prvCpssLogParamFuncStc_GT_FLOAT32_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    GT_CHAR_PTR                 namePtr,
    IN GT_VOID                * fieldPtr
)
{
    PRV_CPSS_LOG_SET_FIELD_VAL_PTR_MAC(GT_FLOAT32 *, valPtr);
    prvCpssLogStcFloat(contextLib, logType, namePtr, *valPtr);
}

/**
* @internal prvCpssLogStcFloat function
* @endinternal
*
* @brief   The function to log float parameter
*
* @param[in] contextLib               -
*                                      lib log activity
* @param[in] logType                  -
*                                      type of the log
* @param[in] fieldName                - structure field name
* @param[in] numValue                 - field value.
*                                       None.
*/
void prvCpssLogStcFloat
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              fieldName,
    IN float                    numValue
)
{
    PRV_CPSS_LOG_ZERO_VALUE_LOG_CHECK_MAC(numValue);
    prvCpssLogTabs(contextLib, logType);
    PRV_CPSS_LOG_AND_HISTORY_MAC(PRV_CPSS_LOG_PHASE_INFO_E, contextLib, logType, "%s = %f\n", fieldName, numValue);
}

/**
* @internal prvCpssLogStcPointer function
* @endinternal
*
* @param[in] contextLib               -
*                                      lib log activity
* @param[in] logType                  -
*                                      type of the log
* @param[in] fieldName                - structure field name
* @param[in] ptrValue                 - (pointer to) field value.
*                                       None.
*/
void prvCpssLogStcPointer
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              fieldName,
    IN void *                   ptrValue
)
{
    GT_CHAR *formatStr;
    PRV_CPSS_LOG_ZERO_VALUE_LOG_CHECK_MAC(ptrValue);
    prvCpssLogTabs(contextLib, logType);
    formatStr = (PRV_LOG_GLOBAL_NON_VOLATILE_VAR_DIRECT_GET(prvCpssLogPointerFormat) == CPSS_LOG_POINTER_FORMAT_PREFIX_E) ?
        "%s = (addr)%p\n" :  "%s = %p\n";
    PRV_CPSS_LOG_AND_HISTORY_MAC(PRV_CPSS_LOG_PHASE_INFO_E, contextLib, logType,
                                 formatStr, fieldName, ptrValue);
}

/**
* @internal prvCpssLogParamFuncStc_GT_U8_PTR_PTR function
* @endinternal
*
* @param[in] contextLib               -
*                                      lib log activity
* @param[in] logType                  -
*                                      type of the log
*                                      fieldName - structure field name
* @param[in] fieldPtr                 - (pointer to) field value.
* @param[in,out] inOutParamInfoPtr        - (pointer to) auxiliary parameters.
*                                       None.
*/
void prvCpssLogParamFuncStc_GT_U8_PTR_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_FIELD_VAL_PTR_MAC(GT_U8**, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    prvCpssLogStcPointer(contextLib, logType, namePtr, *valPtr);
}
/**
* @internal prvCpssLogTabs function
* @endinternal
*
* @brief   Log tab for recursive structures
*
* @param[in] contextLib               -
*                                      lib log activity
* @param[in] logType                  -
*                                      type of the log
*                                       None.
*/
void prvCpssLogTabs
(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType
)
{
    GT_U32 i;
    for (i = 0; i < PRV_LOG_GLOBAL_NON_VOLATILE_VAR_DIRECT_GET(prvCpssLogTabIndex); i++){
        PRV_CPSS_LOG_AND_HISTORY_MAC(PRV_CPSS_LOG_PHASE_INFO_E, contextLib, logType, "\t");
    }
}

/**
* @internal prvCpssLogStcLogStart function
* @endinternal
*
* @brief   The function starts log structure
*
* @param[in] contextLib               -
*                                      lib log activity
* @param[in] logType                  -
*                                      type of the log
* @param[in] stcName                  - structure field name.
*                                       None.
*/
void prvCpssLogStcLogStart
(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 stcName
)
{
    prvCpssLogTabs(contextLib, logType);
    PRV_CPSS_LOG_AND_HISTORY_MAC(PRV_CPSS_LOG_PHASE_INFO_E, contextLib, logType, "%s {\n", stcName);
    PRV_LOG_GLOBAL_NON_VOLATILE_VAR_DIRECT_GET(prvCpssLogTabIndex)++;
}

/**
* @internal prvCpssLogStcLogEnd function
* @endinternal
*
* @brief   The function ends log structure
*
* @param[in] contextLib               -
*                                      lib log activity
* @param[in] logType                  -
*                                      type of the log
*                                       None.
*/
void prvCpssLogStcLogEnd
(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType
)
{
    if (PRV_LOG_GLOBAL_NON_VOLATILE_VAR_DIRECT_GET(prvCpssLogTabIndex))
    {
        PRV_LOG_GLOBAL_NON_VOLATILE_VAR_DIRECT_GET(prvCpssLogTabIndex)--;
    }
    prvCpssLogTabs(contextLib, logType);
    PRV_CPSS_LOG_AND_HISTORY_MAC(PRV_CPSS_LOG_PHASE_INFO_E, contextLib, logType, "}\n");
}

/**
* @internal prvCpssLogPacket function
* @endinternal
*
* @brief   The function logs packet as array of bytes
*
* @param[in] contextLib               -
*                                      lib log activity
* @param[in] logType                  -
*                                      type of the log
* @param[in] packetSize               -
*                                      size of packet (in bytes)
* @param[in] packetPtr                -
*                                      a pointer to the first byte of the packet
*                                       None.
*/
void prvCpssLogPacket
(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_U32                      packetSize,
    IN GT_U8                       *packetPtr
)
{
    GT_U32  byteCount = 0; /* index */

    /* printing the packet data, 16 byte in each line*/
    for (byteCount = 0; byteCount < packetSize; byteCount++)
    {
        /* adress of the new 16 byte series */
        if((byteCount & 0x0F) == 0)
        {
            cpssOsSprintf(PRV_LOG_GLOBAL_VAR_VOLATILE_GET(buffer5),"0x%4.4x :", byteCount);
        }

        cpssOsSprintf(PRV_LOG_GLOBAL_VAR_VOLATILE_GET(buffer5),"%s %2.2x",PRV_LOG_GLOBAL_VAR_VOLATILE_GET(buffer5),((GT_U8*)packetPtr)[byteCount]);

        /* end of 16 bytes series */
        if((byteCount & 0x0F) == 0x0F)
        {
            cpssOsSprintf(PRV_LOG_GLOBAL_VAR_VOLATILE_GET(buffer5),"%s\n",PRV_LOG_GLOBAL_VAR_VOLATILE_GET(buffer5));
            prvCpssLogTabs(contextLib, logType);
            PRV_CPSS_LOG_AND_HISTORY_MAC(PRV_CPSS_LOG_PHASE_INFO_E, contextLib, logType, PRV_LOG_GLOBAL_VAR_VOLATILE_GET(buffer5));
            cpssOsSprintf(PRV_LOG_GLOBAL_VAR_VOLATILE_GET(buffer5),"");
        }
    }

    /* the packet size is less than 16 bytes so we print the only line*/
    if (packetSize != 0 && packetSize < 16)
    {
        cpssOsSprintf(PRV_LOG_GLOBAL_VAR_VOLATILE_GET(buffer5),"%s\n",PRV_LOG_GLOBAL_VAR_VOLATILE_GET(buffer5));
        prvCpssLogTabs(contextLib, logType);
        PRV_CPSS_LOG_AND_HISTORY_MAC(PRV_CPSS_LOG_PHASE_INFO_E, contextLib, logType, PRV_LOG_GLOBAL_VAR_VOLATILE_GET(buffer5));
        cpssOsSprintf(PRV_LOG_GLOBAL_VAR_VOLATILE_GET(buffer5),"");
    }
}

/**
* @internal prvCpssLogEnum function
* @endinternal
*
* @brief   The function to log simple enum parameter
*
* @param[in] contextLib               -
*                                      lib log activity
* @param[in] logType                  -
*                                      type of the log
* @param[in] fieldName                - parameter/structure field name.
* @param[in] enumArray[]              - array of emumerator strings.
* @param[in] enumArrayIndex           - log string access index in enum array.
* @param[in] enumArrayEntries         - number of enum array entries.
*                                       None.
*/
void prvCpssLogEnum
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              fieldName,
    IN const char       * const enumArray[],
    IN GT_U32                   enumArrayIndex,
    IN GT_U32                   enumArrayEntries
)
{
    prvCpssLogTabs(contextLib, logType);
    if (enumArrayIndex < enumArrayEntries) {
        PRV_CPSS_LOG_AND_HISTORY_MAC(PRV_CPSS_LOG_PHASE_INFO_E, contextLib, logType, "%s = %s\n", fieldName, enumArray[enumArrayIndex]);
    } else {
        PRV_CPSS_LOG_AND_HISTORY_MAC(PRV_CPSS_LOG_PHASE_INFO_E, contextLib, logType, "%s = 0x%08x\n", fieldName, enumArrayIndex);
    }
}

/**
* @internal prvCpssLogStcBool function
* @endinternal
*
* @brief   The function logs GT_BOOL structure field
*
* @param[in] contextLib               -
*                                      lib log activity
* @param[in] logType                  -
*                                      type of the log
* @param[in] fieldName                - structure field name
* @param[in] bValue                   - field value.
*                                       None.
*/
void prvCpssLogStcBool
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              fieldName,
    IN GT_BOOL                  bValue
)
{
    prvCpssLogTabs(contextLib, logType);
    PRV_CPSS_LOG_AND_HISTORY_MAC(PRV_CPSS_LOG_PHASE_INFO_E, contextLib, logType, "%s = %s\n", fieldName, (bValue == GT_FALSE) ? "GT_FALSE" : "GT_TRUE");
}

/**
* @internal prvCpssLogStcMac function
* @endinternal
*
* @brief   Function to log mac address parameter
*
* @param[in] contextLib               -
*                                      lib log activity
* @param[in] logType                  -
*                                      type of the log
* @param[in] fieldName                - structure field name
* @param[in] macPtr                   - pointer to ethernet address.
*                                       None.
*/
void prvCpssLogStcMac
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              fieldName,
    IN GT_U8                  * macPtr
)
{
    prvCpssLogTabs(contextLib, logType);
    PRV_CPSS_LOG_AND_HISTORY_MAC(PRV_CPSS_LOG_PHASE_INFO_E, contextLib, logType, "%s = %02x:%02x:%02x:%02x:%02x:%02x\n", fieldName,
                macPtr[0],macPtr[1],macPtr[2],macPtr[3],macPtr[4],macPtr[5]);
}

/**
* @internal prvCpssLogStcIpV4 function
* @endinternal
*
* @brief   Function to log IPV4 address parameter
*
* @param[in] contextLib               -
*                                      lib log activity
* @param[in] logType                  -
*                                      type of the log
* @param[in] fieldName                - structure field name
* @param[in] ipAddrPtr                - pointer to IPV4 address.
*                                       None.
*/
void prvCpssLogStcIpV4
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              fieldName,
    IN GT_U8                  * ipAddrPtr
)
{
    prvCpssLogTabs(contextLib, logType);
    PRV_CPSS_LOG_AND_HISTORY_MAC(PRV_CPSS_LOG_PHASE_INFO_E, contextLib, logType, "%s = %d.%d.%d.%d\n", fieldName,
                ipAddrPtr[0],ipAddrPtr[1],ipAddrPtr[2], ipAddrPtr[3]);
}

/**
* @internal prvCpssLogStcIpV6 function
* @endinternal
*
* @brief   Function to log IPV6 address parameter
*
* @param[in] contextLib               -
*                                      lib log activity
* @param[in] logType                  -
*                                      type of the log
* @param[in] fieldName                - structure field name
* @param[in] ipV6Ptr                  - pointer to IPV6 address.
*                                       None.
*/
void prvCpssLogStcIpV6
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              fieldName,
    IN GT_U8                  * ipV6Ptr
)
{
    prvCpssLogTabs(contextLib, logType);
    PRV_CPSS_LOG_AND_HISTORY_MAC(PRV_CPSS_LOG_PHASE_INFO_E, contextLib, logType, "%s = %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x\n", fieldName,
                ipV6Ptr[0],ipV6Ptr[1],ipV6Ptr[2], ipV6Ptr[3],ipV6Ptr[4],ipV6Ptr[5],ipV6Ptr[6], ipV6Ptr[7],
                ipV6Ptr[8],ipV6Ptr[9],ipV6Ptr[10], ipV6Ptr[11],ipV6Ptr[12],ipV6Ptr[13],ipV6Ptr[14], ipV6Ptr[15]);
}

/**
* @internal prvCpssLogParamDataCheck function
* @endinternal
*
* @brief   Parameters validation.
*
* @param[in] skipLog                  - skip log flag.
* @param[in] inOutParamInfoPtr        - pointer to parameter log data.
*                                       None.
*/
GT_BOOL prvCpssLogParamDataCheck
(
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC  * inOutParamInfoPtr
)
{
    if (inOutParamInfoPtr == 0)
    {
        return GT_FALSE;
    }
    return (skipLog) ? GT_FALSE : GT_TRUE;
}

/**
* @internal cpssLogHistoryDump function
* @endinternal
*
* @brief   Dump log history database.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - if history log disabled
*/
GT_STATUS cpssLogHistoryDump
(
    GT_VOID
)
{
    if (PRV_LOG_GLOBAL_NON_VOLATILE_VAR_DIRECT_GET(prvCpssLogContextEntryPtr) == NULL)
    {
        cpssOsLog(CPSS_LOG_LIB_ALL_E,CPSS_LOG_TYPE_INFO_E,"cpssLogHistoryDump: LOG is empty!\n");
        return GT_FAIL;
    }
    if (PRV_LOG_GLOBAL_NON_VOLATILE_VAR_DIRECT_GET(prvCpssLogStackSize) == 0)
    {
        cpssOsLog(PRV_LOG_GLOBAL_NON_VOLATILE_VAR_DIRECT_GET(prvCpssLogContextEntryPtr)->prvCpssLogApiContextLib, CPSS_LOG_TYPE_INFO_E, "cpssLogHistoryDump: API history log disabled!\n");
        return GT_FAIL;
    }

    if (PRV_LOG_GLOBAL_VAR_VOLATILE_GET(currentHistoryEntryPtr) == NULL)
    {
        cpssOsLog(PRV_LOG_GLOBAL_NON_VOLATILE_VAR_DIRECT_GET(prvCpssLogContextEntryPtr)->prvCpssLogApiContextLib, CPSS_LOG_TYPE_INFO_E, "cpssLogHistoryDump: API history log - NULL pointer!\n");
        return GT_FAIL;
    }

    CPSS_ZERO_LEVEL_API_LOCK_NO_RETURN_MAC;

    /* Start logging from list head */
    prvCpssLogApiHistoryDbLogOutput(PRV_LOG_GLOBAL_VAR_VOLATILE_GET(currentHistoryEntryPtr)->nextEntryPtr, PRV_LOG_GLOBAL_VAR_VOLATILE_GET(currentHistoryEntryNum));

    CPSS_ZERO_LEVEL_API_UNLOCK_MAC;

    return GT_OK;
}

/**
* @internal prvCpssLogParamFuncStcTypeArray function
* @endinternal
*
* @brief   Log array in structure
*
* @param[in] contextLib               -
*                                      lib log activity
* @param[in] logType                  -
*                                      type of the log
* @param[in] namePtr                  -
*                                      array name.
* @param[in] firstElementPtr          -
*                                      pointer to first array element
* @param[in] elementNum               -
*                                      number of array elements
* @param[in] elementSize              -
*                                      size of array element in bytes
* @param[in] logFunc                  -
*                                      pointer to specific type log function
*                                       None.
*/
GT_VOID prvCpssLogParamFuncStcTypeArray
(
    IN CPSS_LOG_LIB_ENT        contextLib,
    IN CPSS_LOG_TYPE_ENT       logType,
    IN GT_CHAR_PTR             namePtr,
    IN GT_VOID               * firstElementPtr,
    IN GT_U32                  elementNum,
    IN GT_U32                  elementSize,
    IN PRV_CPSS_LOG_STC_FUNC   logFunc,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    GT_U32 i;
    GT_U8 * currPtr;

    if (firstElementPtr == NULL)
    {
        prvCpssLogTabs(contextLib, logType);
        PRV_CPSS_LOG_AND_HISTORY_MAC(PRV_CPSS_LOG_PHASE_INFO_E, contextLib, logType, "%s = NULL\n", namePtr);
        return;
    }

    currPtr = (GT_U8 *)firstElementPtr;
    /* Log all array elements */
    for (i = 0; i < elementNum; i++)
    {
        cpssOsSprintf(PRV_LOG_GLOBAL_VAR_VOLATILE_GET(buffer2),"%s[%d]", namePtr, i);
        logFunc(contextLib, logType, PRV_LOG_GLOBAL_VAR_VOLATILE_GET(buffer2), (GT_VOID *)currPtr, inOutParamInfoPtr);
        /* Increment pointer address */
        currPtr += elementSize;
    }
}

/**
* @internal prvCpssLogParamFuncStcTypeArray2 function
* @endinternal
*
* @brief   Log two-dimentional array
*
* @param[in] contextLib               -
*                                      lib log activity
* @param[in] logType                  -
*                                      type of the log
* @param[in] namePtr                  -
*                                      array name.
* @param[in] firstElementPtr          -
*                                      pointer to first array element
* @param[in] rowNum                   -
*                                      number of array rows
* @param[in] rowNum                   -
*                                      number of array columns
* @param[in] elementSize              -
*                                      size of array element in bytes
* @param[in] logFunc                  -
*                                      pointer to specific type log function
*                                       None.
*/
GT_VOID prvCpssLogParamFuncStcTypeArray2
(
    IN CPSS_LOG_LIB_ENT        contextLib,
    IN CPSS_LOG_TYPE_ENT       logType,
    IN GT_CHAR_PTR             namePtr,
    IN GT_VOID               * firstElementPtr,
    IN GT_U32                  rowNum,
    IN GT_U32                  colNum,
    IN GT_U32                  elementSize,
    IN PRV_CPSS_LOG_STC_FUNC   logFunc,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    GT_U32 i, j;
    GT_U8 * currPtr;

    (void)colNum;
    if (firstElementPtr == NULL)
    {
        prvCpssLogTabs(contextLib, logType);
        PRV_CPSS_LOG_AND_HISTORY_MAC(PRV_CPSS_LOG_PHASE_INFO_E, contextLib, logType, "%s = NULL\n", namePtr);
        return;
    }

    currPtr = (GT_U8 *)firstElementPtr;
    /* Log all array elements */
    for (i = 0; i < rowNum; i++)
    {
        for (j = 0; j < colNum; j++)
        {

            cpssOsSprintf(PRV_LOG_GLOBAL_VAR_VOLATILE_GET(buffer2), "%s[%d][%d]", namePtr, i, j);
            logFunc(contextLib, logType, PRV_LOG_GLOBAL_VAR_VOLATILE_GET(buffer2), (GT_VOID *)currPtr, inOutParamInfoPtr);
            /* Increment pointer address */
            currPtr += elementSize;
        }
    }
}

/**
* @internal getArraySize function
* @endinternal
*
* @brief   extract array size from param info
*
* @param[in] inOutParamInfoPtr        -
*                                      pointer to information about the array of parameters
* @param[in] inOutParamInfoPtr        - pointer to output array size
* @param[in] inOutParamInfoPtr        - the array max size
* @param[in] inOutParamInfoPtr        - size of an input array
* @param[in] inOutParamInfoPtr        - value of 1 is a sign that the output might be an array of strucutures
*
* @param[out] arrSizePtr               -
*                                      pointer to the array size
*
* @retval GT_OK                    - if the extraction successed
* @retval GT_BAD_PARAM             - otherwise
*/
static GT_STATUS getArraySize
(
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr,
    OUT GT_U32                 * arrSizePtr
)
{
    GT_U32 size, maxSize; /*index, number of structures, max number of structures */
    GT_U32  *sizePtr; /* pointer for size of an output array */

    sizePtr = (GT_U32 *)inOutParamInfoPtr->paramKey.paramKeyArr[0]; /* extracting the pointer to the output array size */
    maxSize = (GT_U32)inOutParamInfoPtr->paramKey.paramKeyArr[1]; /* extracting the max size valid for the output array */
    if (sizePtr != 0) /* the array is output paramenter */
    {
        size = *sizePtr; /* getting the size of the output array */
        if ((*sizePtr > maxSize) || (size == 0)) /* the size is not valid, the array is not valid, no need to log it */
        {
            arrSizePtr = NULL;
            return GT_BAD_PARAM;
        }
    }
    else
    {
        /* if it is output array the sizePtr is null, there is no structure to output - no need to log it  */
        if (inOutParamInfoPtr->paramKey.paramKeyArr[3] == 1)
        {
            arrSizePtr = NULL;
            return GT_BAD_PARAM;
        }
        size = (GT_U32)inOutParamInfoPtr->paramKey.paramKeyArr[2]; /* extracting number of structures (input parameter) */
    }
    *arrSizePtr = size;
    return GT_OK;
}

/**
* @internal prvCpssLogArrayOfParamsHandle function
* @endinternal
*
* @brief   Log array of parameters (input and output arrays)
*
* @param[in] contextLib               -
*                                      lib log activity
* @param[in] logType                  -
*                                      type of the log
* @param[in] namePtr                  -
*                                      array name.
* @param[in] firstElementPtr          -
*                                      pointer to first array element
* @param[in] elementSize              -
*                                      size of array element in bytes
* @param[in] logFunc                  -
*                                      pointer to specific type log function
* @param[in] inOutParamInfoPtr        -
*                                      pointer to information about the array of parameters
* @param[in] inOutParamInfoPtr        - pointer to output array size
* @param[in] inOutParamInfoPtr        - the array max size
* @param[in] inOutParamInfoPtr        - size of an input array
* @param[in] inOutParamInfoPtr        - value of 1 is a sign that the output might be an array of strucutures
*                                       None.
*/
GT_VOID prvCpssLogArrayOfParamsHandle
(
    IN CPSS_LOG_LIB_ENT        contextLib,
    IN CPSS_LOG_TYPE_ENT       logType,
    IN GT_CHAR_PTR             namePtr,
    IN GT_VOID               * firstElementPtr,
    IN GT_U32                  elementSize,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr,
    IN PRV_CPSS_LOG_STC_FUNC   logFunc
)
{
    GT_U32 size; /* pointer for size of an output array */
    GT_STATUS rc; /* result status */

    rc = getArraySize(inOutParamInfoPtr, &size); /* extracting the array size using helper function */
    if (rc != GT_OK)
    {
        prvCpssLogStcLogStart(contextLib, logType, namePtr);
        prvCpssLogStcLogEnd(contextLib, logType);
        return;
    }

    if (size > 0) /* array */
    {
        prvCpssLogStcLogStart(contextLib, logType, namePtr);
        prvCpssLogParamFuncStcTypeArray(contextLib, logType, namePtr, firstElementPtr, size, elementSize, logFunc, inOutParamInfoPtr); /* Log all array elements */
        prvCpssLogStcLogEnd(contextLib, logType);
    }
    else /* one structure */
        logFunc(contextLib,logType,namePtr,firstElementPtr,inOutParamInfoPtr);
}
/**
* @internal prvCpssLogArrayOfEnumsHandle function
* @endinternal
*
* @brief   Log array of parameters (input and output arrays)
*
* @param[in] contextLib               -
*                                      lib log activity
* @param[in] logType                  -
*                                      type of the log
* @param[in] namePtr                  -
*                                      array name
* @param[in] firstElementPtr          -
*                                      pointer to first enum in the array
* @param[in] enumSize                 -
*                                      size of the enum
* @param[in] enumArray[]              -
*                                      key value array of the enum values
* @param[in] enumArrayEntries         -
*                                      the size of the enum array
* @param[in] inOutParamInfoPtr        -
*                                      pointer to information about the array of parameters
* @param[in] inOutParamInfoPtr        - pointer to output array size
* @param[in] inOutParamInfoPtr        - the array max size
* @param[in] inOutParamInfoPtr        - size of an input array
* @param[in] inOutParamInfoPtr        - value of 1 is a sign that the output might be an array of strucutures
*                                       None.
*/
GT_VOID prvCpssLogArrayOfEnumsHandle
(
    IN CPSS_LOG_LIB_ENT        contextLib,
    IN CPSS_LOG_TYPE_ENT       logType,
    IN GT_CHAR_PTR             namePtr,
    IN GT_VOID               * firstElementPtr,
    IN GT_U32                  enumSize,
    IN const char      * const enumArray[],
    IN GT_U32                  enumArrayEntries,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    GT_U32  i, size; /* index, the array size */
    GT_U8   *currPtr; /* temporary pointer for iterating on the array */
    GT_U32  *enumPtr; /* pointer for the current enum */
    GT_STATUS rc; /* result status */

    rc = getArraySize(inOutParamInfoPtr, &size); /* extracting the array size using helper function */
    if (rc != GT_OK)
    {
        prvCpssLogStcLogStart(contextLib, logType, namePtr);
        prvCpssLogStcLogEnd(contextLib, logType);
        return;
    }

    enumPtr = (GT_U32*)firstElementPtr;
    if (size > 0) /* array */
    {
        currPtr = (GT_U8 *)firstElementPtr;
        prvCpssLogStcLogStart(contextLib, logType, namePtr);
        /* Log all array elements */
        for (i = 0; i < size; i++)
        {
            enumPtr = (GT_U32 *)currPtr;
            cpssOsSprintf(PRV_LOG_GLOBAL_VAR_VOLATILE_GET(buffer2),"%s[%d]", namePtr, i);
            prvCpssLogEnum(contextLib, logType, PRV_LOG_GLOBAL_VAR_VOLATILE_GET(buffer2), enumArray, *enumPtr, enumArrayEntries);
            /* Increment pointer address */
            currPtr += enumSize;
        }
        prvCpssLogStcLogEnd(contextLib, logType);
    }
    else /* one structure */
    {
        enumPtr = (GT_U32 *)firstElementPtr;
        prvCpssLogEnum(contextLib, logType, namePtr, enumArray, *enumPtr, enumArrayEntries);
    }
}
/**
* @internal cpssLogHistoryFileNameSet function
* @endinternal
*
* @brief   Set log history file name.
*
* @param[in] fileNamePtr              - pointer to history file name string
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_BAD_PARAM             - string exceeds maximmal size
*/
GT_STATUS cpssLogHistoryFileNameSet
(
    IN const char * fileNamePtr
)
{
    if (fileNamePtr == NULL)
    {
        return GT_BAD_PTR;
    }

    if (cpssOsStrlen(fileNamePtr) > 80)
    {
        return GT_BAD_PARAM;
    }

    CPSS_ZERO_LEVEL_API_LOCK_NO_RETURN_MAC;
    cpssOsSprintf(PRV_LOG_GLOBAL_VAR_VOLATILE_GET(prvCpssLogHistoryName), "%s", fileNamePtr);

    /* Clean up database structure */
    prvCpssLogApiHistoryDbClose(&PRV_LOG_GLOBAL_VAR_VOLATILE_GET(currentHistoryEntryPtr), &PRV_LOG_GLOBAL_VAR_VOLATILE_GET(currentHistoryEntryNum), GT_FALSE);
    CPSS_ZERO_LEVEL_API_UNLOCK_MAC;

    return GT_OK;
}

/**
* @internal prvCpssLogStateCheck function
* @endinternal
*
* @brief   Debug function to check log readiness before new API call
*
* @retval GT_OK                    - log is ready to run
* @retval GT_BAD_STATE             - log is state machine in bad state to run log
*/
GT_STATUS prvCpssLogStateCheck
(
    GT_VOID
)
{
    CPSS_ZERO_LEVEL_API_LOCK_NO_RETURN_MAC;
    if (PRV_LOG_GLOBAL_NON_VOLATILE_VAR_DIRECT_GET(prvCpssLogContextEntryPtr))
    {
        /* Check context of entrance */
        if (PRV_LOG_GLOBAL_NON_VOLATILE_VAR_DIRECT_GET(prvCpssLogContextEntryPtr)->prvCpssLogApiContextIndex != 0 ||
            PRV_LOG_GLOBAL_NON_VOLATILE_VAR_DIRECT_GET(prvCpssLogContextEntryPtr)->prvCpssLogApiContextLib != CPSS_LOG_LIB_ALL_E ||
            PRV_LOG_GLOBAL_NON_VOLATILE_VAR_DIRECT_GET(prvCpssLogContextEntryPtr)->prvCpssLogEntryFuncId != PRV_CPSS_LOG_NOT_VALID_CNS)
        {
            CPSS_ZERO_LEVEL_API_UNLOCK_MAC;
            return GT_BAD_STATE;
        }
    }
    CPSS_ZERO_LEVEL_API_UNLOCK_MAC;

    return GT_OK;
}


/**
* @internal cpssLogLineNumberEnableSet function
* @endinternal
*
* @brief   The function enables/disables using of a source code line number by
*         some logging functions
* @param[in] enable                   - GT_TRUE - enable, GT_FALSE - disable
*                                       None.
*/
GT_VOID cpssLogLineNumberEnableSet
(
     IN GT_BOOL enable
)
{
    PRV_LOG_GLOBAL_NON_VOLATILE_VAR_DIRECT_GET(prvCpssLogLineNumberEnabled) = enable;
}

/**
* @internal cpssLogLineNumberEnableGet function
* @endinternal
*
* @brief   The function gets the status of the using of a sorce code line number
*         by some logging functions
*
* @retval GT_TRUE                  - if enabled
* @retval GT_FALSE                 - if disabled
*/
GT_BOOL cpssLogLineNumberEnableGet
(
    GT_VOID
)
{
    return PRV_LOG_GLOBAL_NON_VOLATILE_VAR_DIRECT_GET(prvCpssLogLineNumberEnabled);
}

/**
* @internal cpssLogPointerFormatSet function
* @endinternal
*
* @brief   Set a format of pointer values
*
* @param[in] value                    - a format type
*                                       None.
*/
GT_VOID cpssLogPointerFormatSet
(
     IN CPSS_LOG_POINTER_FORMAT_ENT value
)
{
    PRV_LOG_GLOBAL_NON_VOLATILE_VAR_DIRECT_GET(prvCpssLogPointerFormat) = value;
}

/**
* @internal cpssLogPointerFormatGet function
* @endinternal
*
* @brief   get a format used for a pointer-values logging.
*
* @retval a format used for a pointer -values logging.
*/
CPSS_LOG_POINTER_FORMAT_ENT cpssLogPointerFormatGet
(
    GT_VOID
)
{
    return PRV_LOG_GLOBAL_NON_VOLATILE_VAR_DIRECT_GET(prvCpssLogPointerFormat);
}

/**
* @internal prvCpssLogParamFunc_GT_U8_PTR_ARRAY function
* @endinternal
*
* @brief   logs an array of GT_U8 pointers
*
* @param[in] contextLib               -
*                                      lib log activity
* @param[in] logType                  -
*                                      type of the log
* @param[in] namePtr                  -
*                                      array name.
* @param[in] argsPtr                  -
*                                      additional args
* @param[in] skipLog                  -
*                                      to skip the log or not
* @param[in] inOutParamInfoPtr        -
*                                      pointer to information about the array of parameters
*                                       None.
*/
void prvCpssLogParamFunc_GT_U8_PTR_ARRAY(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(GT_U8**, paramVal);
    if (paramVal == NULL)
    {
        prvCpssLogStcLogStart(contextLib, logType, namePtr);
        prvCpssLogStcLogEnd(contextLib, logType);
        return;
    }
    prvCpssLogArrayOfParamsHandle(contextLib,logType,namePtr,paramVal,sizeof(GT_U8 *),inOutParamInfoPtr,prvCpssLogParamFuncStc_GT_U8_PTR_PTR);
}

/**
* @internal prvCpssLogParamFunc_GT_U8_ARRAY function
* @endinternal
*
* @brief   logs an array of GT_U8 elements
*
* @param[in] contextLib               -
*                                      lib log activity
* @param[in] logType                  -
*                                      type of the log
* @param[in] namePtr                  -
*                                      array name.
* @param[in] argsPtr                  -
*                                      additional args
* @param[in] skipLog                  -
*                                      to skip the log or not
* @param[in] inOutParamInfoPtr        -
*                                      pointer to information about the array of parameters
*                                       None.
*/
void prvCpssLogParamFunc_GT_U8_ARRAY(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(GT_U8*, paramVal);
    prvCpssLogArrayOfParamsHandle(contextLib,logType,namePtr,paramVal,sizeof(GT_U8),inOutParamInfoPtr,prvCpssLogParamFuncStc_GT_U8_PTR);
}

/**
* @internal prvCpssLogParamFunc_GT_U32_ARRAY function
* @endinternal
*
* @brief   logs an array of GT_U32 elements
*
* @param[in] contextLib               -
*                                      lib log activity
* @param[in] logType                  -
*                                      type of the log
* @param[in] namePtr                  -
*                                      array name.
* @param[in] argsPtr                  -
*                                      additional args
* @param[in] skipLog                  -
*                                      to skip the log or not
* @param[in] inOutParamInfoPtr        -
*                                      pointer to information about the array of parameters
*                                       None.
*/
void prvCpssLogParamFunc_GT_U32_ARRAY(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(GT_U32*, paramVal);
    prvCpssLogArrayOfParamsHandle(contextLib,logType,namePtr,paramVal,sizeof(GT_U32),inOutParamInfoPtr,prvCpssLogParamFuncStc_GT_U32_PTR);
}

/**
* @internal prvCpssLogParamFunc_PACKET_PTR function
* @endinternal
*
* @brief   logs an array of GT_U8 pointers as a packet (stream of bytes)
*
* @param[in] contextLib               -
*                                      lib log activity
* @param[in] logType                  -
*                                      type of the log
* @param[in] namePtr                  -
*                                      array name.
* @param[in] argsPtr                  -
*                                      additional args
* @param[in] skipLog                  -
*                                      to skip the log or not
* @param[in] inOutParamInfoPtr        -
*                                      pointer to information about the array of parameters
*                                       None.
*/
void prvCpssLogParamFunc_PACKET_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    GT_U32 size; /* pointer for size of an output array */
    GT_STATUS rc; /* result status */

    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(GT_U8*, paramVal);

    rc = getArraySize(inOutParamInfoPtr, &size); /* extracting the packet size using helper function */
    /* if the packet size is not valid or the packet size is 0 or the pointer to the packet is NULL, there is nothing to log */
    if (rc != GT_OK || size == 0 || paramVal == NULL)
    {
        prvCpssLogStcLogStart(contextLib, logType, namePtr);
        prvCpssLogStcLogEnd(contextLib, logType);
        return;
    }

    /* log the packet using the hekper function prvCpssLogPacket */
    prvCpssLogStcLogStart(contextLib, logType, namePtr);
    prvCpssLogPacket(contextLib,logType,size,paramVal);
    prvCpssLogStcLogEnd(contextLib, logType);
}

/**
* @internal prvCpssLogParamFunc_PACKET_PTR_ARRAY function
* @endinternal
*
* @brief   logs an array of packets pointers
*
* @param[in] contextLib               -
*                                      lib log activity
* @param[in] logType                  -
*                                      type of the log
* @param[in] namePtr                  -
*                                      array name.
* @param[in] argsPtr                  -
*                                      additional args
* @param[in] skipLog                  -
*                                      to skip the log or not
* @param[in] inOutParamInfoPtr        -
*                                      pointer to information about the array of parameters
*                                       None.
*/
void prvCpssLogParamFunc_PACKET_PTR_ARRAY(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    GT_U32 i, size; /*index, number of packets */
    GT_U8  *currBufferPtr; /* temporary pointer to the buffer we working on */
    GT_U32 *currLenPtr; /* temporary pointer to the size of the packet we working on */
    GT_STATUS rc; /* result status */

    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(GT_U8**, paramVal);
    if (paramVal == NULL)
    {
        prvCpssLogTabs(contextLib, logType);
        PRV_CPSS_LOG_AND_HISTORY_MAC(PRV_CPSS_LOG_PHASE_INFO_E, contextLib, logType, "%s = NULL\n", namePtr);
        return;
    }

    rc = getArraySize(inOutParamInfoPtr, &size); /* extracting the packet size using helper function */
    /* if the size is not valid or the packet size is 0, there is nothing to log */
    if (rc != GT_OK || size == 0)
    {
        prvCpssLogStcLogStart(contextLib, logType, namePtr);
        prvCpssLogStcLogEnd(contextLib, logType);
        return;
    }

    currLenPtr = (GT_U32 *)inOutParamInfoPtr->paramKey.paramKeyArr[4]; /* extracting the pointer to the output array of packets size */
    if (currLenPtr == NULL)
    {
        /* length array is not valid. log as array of pointers. */
        prvCpssLogArrayOfParamsHandle(contextLib,logType,namePtr,paramVal,sizeof(GT_U8 *),inOutParamInfoPtr,prvCpssLogParamFuncStc_GT_U8_PTR_PTR);
        return;
    }

    /* iterate over both arrays (packets and sizes) for loging the packets */
    prvCpssLogStcLogStart(contextLib, logType, namePtr);
    /* Log all array elements */
    for (i = 0; i < size; i++)
    {
        currBufferPtr  = paramVal[i];

        cpssOsSprintf(PRV_LOG_GLOBAL_VAR_VOLATILE_GET(buffer2),"%s[%d]", namePtr, i);
        if (currBufferPtr == NULL)
        {
            prvCpssLogTabs(contextLib, logType);
            PRV_CPSS_LOG_AND_HISTORY_MAC(PRV_CPSS_LOG_PHASE_INFO_E, contextLib, logType, "%s = NULL\n", PRV_LOG_GLOBAL_VAR_VOLATILE_GET(buffer2));
        }
        else
        {
            prvCpssLogStcLogStart(contextLib, logType, PRV_LOG_GLOBAL_VAR_VOLATILE_GET(buffer2));
            prvCpssLogPacket(contextLib,logType,*currLenPtr,currBufferPtr);
            prvCpssLogStcLogEnd(contextLib, logType);
        }

        /* Increment pointer address */
        currLenPtr++;
    }
    prvCpssLogStcLogEnd(contextLib, logType);
}

/**
* @internal prvCpssLogParamFunc_GT_BOOL_ARRAY function
* @endinternal
*
* @brief   logs an array of GT_BOOL elements
*
* @param[in] contextLib               -
*                                      lib log activity
* @param[in] logType                  -
*                                      type of the log
* @param[in] namePtr                  -
*                                      array name.
* @param[in] argsPtr                  -
*                                      additional args
* @param[in] skipLog                  -
*                                      to skip the log or not
* @param[in] inOutParamInfoPtr        -
*                                      pointer to information about the array of parameters
*                                       None.
*/
void prvCpssLogParamFunc_GT_BOOL_ARRAY(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(GT_BOOL*, paramVal);
    prvCpssLogArrayOfParamsHandle(contextLib,logType,namePtr,paramVal,sizeof(GT_BOOL),inOutParamInfoPtr,prvCpssLogParamFuncStc_GT_BOOL_PTR);
}

/**
* @internal prvCpssLogParamFunc_DSA_PTR function
* @endinternal
*
* @brief   logs an array of GT_U8 as DSA (array of words)
*
* @param[in] contextLib               -
*                                      lib log activity
* @param[in] logType                  -
*                                      type of the log
* @param[in] namePtr                  -
*                                      array name.
* @param[in] argsPtr                  -
*                                      additional args
* @param[in] skipLog                  -
*                                      to skip the log or not
* @param[in] inOutParamInfoPtr        -
*                                      pointer to information about the array of parameters
*                                       None.
*/
void prvCpssLogParamFunc_DSA_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    GT_U32  word; /* for saving a word in DSA tag */

    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(GT_U8*, paramVal);
    if (paramVal == NULL)
    {
        prvCpssLogTabs(contextLib, logType);
        PRV_CPSS_LOG_AND_HISTORY_MAC(PRV_CPSS_LOG_PHASE_INFO_E, contextLib, logType, "%s = NULL\n", namePtr);
        return;
    }

    /* first word is always exists */
    word = ((paramVal[0]) << 24) |
           ((paramVal[1]) << 16) |
           ((paramVal[2]) << 8)  |
           ((paramVal[3]) << 0) ;
    prvCpssLogStcLogStart(contextLib, logType, namePtr);
    prvCpssLogTabs(contextLib, logType);
    cpssOsLog(contextLib,logType,"%s[0] = %x\n", namePtr,word); /*log first word*/

    /* check the extended bit */
    if(0 == U32_GET_FIELD_MAC(word,12,1))
    {
        /* word 0 is the last word in the DSA tag */
        prvCpssLogStcLogEnd(contextLib, logType);
        return;
    }
    /* copy the data of the second word */
    word = ((paramVal[4]) << 24) |
           ((paramVal[5]) << 16) |
           ((paramVal[6]) <<  8) |
           ((paramVal[7]) <<  0) ;
    prvCpssLogTabs(contextLib, logType);
    cpssOsLog(contextLib,logType,"%s[1] = %x\n", namePtr,word); /*log second word*/

    /* check the extended bit */
    if(0 == U32_GET_FIELD_MAC(word,31,1))
    {
        /* word 1 is the last word in the DSA tag */
        prvCpssLogStcLogEnd(contextLib, logType);
        return;
    }
    /* copy the data of the third word */
    word = ((paramVal[8]) << 24) |
           ((paramVal[9]) << 16) |
           ((paramVal[10]) <<  8) |
           ((paramVal[11]) <<  0) ;
    prvCpssLogTabs(contextLib, logType);
    cpssOsLog(contextLib,logType,"%s[2] = %x\n", namePtr,word); /*log third word*/

    /* check the extended bit */
    if(0 == U32_GET_FIELD_MAC(word,31,1))
    {
        /* 3 words DSA tag is not supported */
        prvCpssLogStcLogEnd(contextLib, logType);
        return;
    }
    /* copy the data of the forth word */
    word = ((paramVal[12]) << 24) |
           ((paramVal[13]) << 16) |
           ((paramVal[14]) <<  8) |
           ((paramVal[15]) <<  0) ;
    prvCpssLogTabs(contextLib, logType);
    cpssOsLog(contextLib,logType,"%s[3] = %x\n", namePtr,word); /*log fourth word*/
    prvCpssLogStcLogEnd(contextLib, logType);
}

/**
* @internal prvCpssLogParamFunc_GT_CHAR_ARRAY function
* @endinternal
*
* @brief   logs an array of GT_CHAR as a string
*
* @param[in] contextLib               -
*                                      lib log activity
* @param[in] logType                  -
*                                      type of the log
* @param[in] namePtr                  -
*                                      array name.
* @param[in] argsPtr                  -
*                                      additional args
* @param[in] skipLog                  -
*                                      to skip the log or not
* @param[in] inOutParamInfoPtr        -
*                                      pointer to information about the array of parameters
*                                       None.
*/
void prvCpssLogParamFunc_GT_CHAR_ARRAY(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(GT_CHAR*, paramVal);
    prvCpssLogTabs(contextLib, logType);
    PRV_CPSS_LOG_AND_HISTORY_MAC(PRV_CPSS_LOG_PHASE_INFO_E, contextLib, logType, "%s = %s\n", namePtr, paramVal);
}

/**
* @internal prvCpssLogStc32HexNumber function
* @endinternal
*
* @brief   The function to log 32 bit hex parameter
*
* @param[in] contextLib               -
*                                      lib log activity
* @param[in] logType                  -
*                                      type of the log
* @param[in] fieldName                - structure field name
* @param[in] numValue                 - field value.
*                                       None.
*/
void prvCpssLogStc32HexNumber
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              fieldName,
    IN GT_U32                   numValue
)
{
    PRV_CPSS_LOG_ZERO_VALUE_LOG_CHECK_MAC(numValue);
    prvCpssLogTabs(contextLib, logType);
    PRV_CPSS_LOG_AND_HISTORY_MAC(PRV_CPSS_LOG_PHASE_INFO_E, contextLib, logType, "%s = 0x%08x\n", fieldName, numValue);
}

/**
* @internal prvCpssLogStc16HexNumber function
* @endinternal
*
* @brief   The function to log 16 bit hex parameter
*
* @param[in] contextLib               -
*                                      lib log activity
* @param[in] logType                  -
*                                      type of the log
* @param[in] fieldName                - structure field name
* @param[in] numValue                 - field value.
*                                       None.
*/
void prvCpssLogStc16HexNumber
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              fieldName,
    IN GT_U32                   numValue
)
{
    PRV_CPSS_LOG_ZERO_VALUE_LOG_CHECK_MAC(numValue);
    prvCpssLogTabs(contextLib, logType);
    PRV_CPSS_LOG_AND_HISTORY_MAC(PRV_CPSS_LOG_PHASE_INFO_E, contextLib, logType, "%s = 0x%04x\n", fieldName, numValue);
}

/**
* @internal prvCpssLogU64Number function
* @endinternal
*
* @brief   The function to log GT_U64 variable as two GT_U32 variables
*
* @param[in] contextLib               -
*                                      lib log activity
* @param[in] logType                  -
*                                      type of the log
* @param[in] variableName             - variable name
* @param[in] variablePtr              - pointer to the variable.
* @param[in] isHex                    - GT_TRUE for hexadecimal format and GT_FALSE otherwise
*                                       None.
*/
void prvCpssLogU64Number
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              variableName,
    IN GT_U64                   * variablePtr,
    IN GT_BOOL                  isHex
)
{
    double varDouble;

    varDouble = ((double)(variablePtr->l[1])) * 4294967296.0 /* 0x100000000*/ + variablePtr->l[0];
    prvCpssLogTabs(contextLib, logType);
    if (isHex == 0)
    {
        PRV_CPSS_LOG_AND_HISTORY_MAC(PRV_CPSS_LOG_PHASE_INFO_E, contextLib, logType, "%s = %1.0f\n", variableName, varDouble);
    }
    else
    {
        PRV_CPSS_LOG_AND_HISTORY_MAC(PRV_CPSS_LOG_PHASE_INFO_E, contextLib, logType, "%s = 0x%08x%08x\n", variableName, variablePtr->l[1], variablePtr->l[0]);
    }
}

/**
* @internal prvCpssLogParamFuncStc_GT_U32_HEX_PTR function
* @endinternal
*
* @brief   logs a variable from GT_U32 type in hexa format
*
* @param[in] contextLib               -
*                                      lib log activity
* @param[in] logType                  -
*                                      type of the log
* @param[in] namePtr                  -
*                                      array name.
* @param[in] fieldPtr                 -
*                                      pointer to the field
* @param[in,out] inOutParamInfoPtr        -
*                                      pointer to information about the array of parameters
*                                       None.
*/
void prvCpssLogParamFuncStc_GT_U32_HEX_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_FIELD_VAL_PTR_MAC(GT_U32*, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    prvCpssLogStc32HexNumber(contextLib, logType, namePtr, *valPtr);
}

/**
* @internal prvCpssLogParamFunc_GT_U32_HEX_PTR function
* @endinternal
*
* @brief   logs a variable from GT_U32 type in hexa format
*
* @param[in] contextLib               -
*                                      lib log activity
* @param[in] logType                  -
*                                      type of the log
* @param[in] namePtr                  -
*                                      array name.
* @param[in] argsPtr                  -
*                                      additional args
* @param[in] skipLog                  -
*                                      to skip the log or not
* @param[in] inOutParamInfoPtr        -
*                                      pointer to information about the parameters
*                                       None.
*/
void prvCpssLogParamFunc_GT_U32_HEX_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(GT_U32*, paramVal);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    if (paramVal == NULL)
    {
        PRV_CPSS_LOG_AND_HISTORY_PARAM_MAC(contextLib, logType, "%s = NULL\n", namePtr);
        return;
    }

    prvCpssLogStc32HexNumber(contextLib, logType, namePtr, *paramVal);
}

/**
* @internal prvCpssLogParamFunc_GT_U32_HEX function
* @endinternal
*
* @brief   logs a variable from GT_U32 type in hexa format
*
* @param[in] contextLib               -
*                                      lib log activity
* @param[in] logType                  -
*                                      type of the log
* @param[in] namePtr                  -
*                                      array name.
* @param[in] argsPtr                  -
*                                      additional args
* @param[in] skipLog                  -
*                                      to skip the log or not
* @param[in] inOutParamInfoPtr        -
*                                      pointer to information about the parameters
*                                       None.
*/
void prvCpssLogParamFunc_GT_U32_HEX(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(GT_U32, paramVal);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    prvCpssLogStc32HexNumber(contextLib, logType, namePtr, paramVal);
}

/**
* @internal prvCpssLogParamFunc_GT_U32_HEX_ARRAY function
* @endinternal
*
* @brief   logs an array of GT_U32_HEX elements
*
* @param[in] contextLib               -
*                                      lib log activity
* @param[in] logType                  -
*                                      type of the log
* @param[in] namePtr                  -
*                                      array name.
* @param[in] argsPtr                  -
*                                      additional args
* @param[in] skipLog                  -
*                                      to skip the log or not
* @param[in] inOutParamInfoPtr        -
*                                      pointer to information about the array of parameters
*                                       None.
*/
void prvCpssLogParamFunc_GT_U32_HEX_ARRAY(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(GT_U32*, paramVal);
    prvCpssLogArrayOfParamsHandle(contextLib,logType,namePtr,paramVal,sizeof(GT_U32),inOutParamInfoPtr,prvCpssLogParamFuncStc_GT_U32_HEX_PTR);
}

/**
* @internal prvCpssLogParamFuncStc_GT_U64_HEX_PTR function
* @endinternal
*
* @brief   logs a variable from GT_U64 type in hexa format
*
* @param[in] contextLib               -
*                                      lib log activity
* @param[in] logType                  -
*                                      type of the log
* @param[in] namePtr                  -
*                                      array name.
* @param[in] fieldPtr                 -
*                                      pointer to the field
* @param[in,out] inOutParamInfoPtr        -
*                                      pointer to information about the array of parameters
*                                       None.
*/
void prvCpssLogParamFuncStc_GT_U64_HEX_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_FIELD_VAL_PTR_MAC(GT_U64*, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    prvCpssLogU64Number(contextLib,logType,namePtr,valPtr,GT_TRUE);
}

/**
* @internal prvCpssLogParamFunc_GT_U64_HEX function
* @endinternal
*
* @brief   logs a variable from GT_U64 type in hexa format
*
* @param[in] contextLib               -
*                                      lib log activity
* @param[in] logType                  -
*                                      type of the log
* @param[in] namePtr                  -
*                                      array name.
*                                      fieldPtr -
*                                      pointer to the field
* @param[in] inOutParamInfoPtr        -
*                                      pointer to information about the array of parameters
*                                       None.
*/
void prvCpssLogParamFunc_GT_U64_HEX(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(GT_U64, paramVal);
    prvCpssLogParamFuncStc_GT_U64_HEX_PTR(contextLib, logType, namePtr, &paramVal, inOutParamInfoPtr);
}

/**
* @internal prvCpssLogParamFunc_GT_U64_HEX_PTR function
* @endinternal
*
* @brief   logs a variable from GT_U64 type in hexa format
*
* @param[in] contextLib               -
*                                      lib log activity
* @param[in] logType                  -
*                                      type of the log
* @param[in] namePtr                  -
*                                      array name.
*                                      fieldPtr -
*                                      pointer to the field
* @param[in] inOutParamInfoPtr        -
*                                      pointer to information about the array of parameters
*                                       None.
*/
void prvCpssLogParamFunc_GT_U64_HEX_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(GT_U64*, paramVal);
    prvCpssLogParamFuncStc_GT_U64_HEX_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}

/**
* @internal prvCpssLogParamFuncStc_GT_U16_HEX_PTR function
* @endinternal
*
* @brief   logs a variable from GT_U16 type in hexa format
*
* @param[in] contextLib               -
*                                      lib log activity
* @param[in] logType                  -
*                                      type of the log
* @param[in] namePtr                  -
*                                      array name.
* @param[in] fieldPtr                 -
*                                      pointer to the field
* @param[in,out] inOutParamInfoPtr        -
*                                      pointer to information about the array of parameters
*                                       None.
*/
void prvCpssLogParamFuncStc_GT_U16_HEX_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_FIELD_VAL_PTR_MAC(GT_U16*, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);

    prvCpssLogStc16HexNumber(contextLib, logType, namePtr, *valPtr);
}

/**
* @internal prvCpssLogParamFunc_GT_U16_HEX_PTR function
* @endinternal
*
* @brief   logs a variable from GT_U16 type in hexa format
*
* @param[in] contextLib               -
*                                      lib log activity
* @param[in] logType                  -
*                                      type of the log
* @param[in] namePtr                  -
*                                      array name.
* @param[in] argsPtr                  -
*                                      additional args
* @param[in] skipLog                  -
*                                      to skip the log or not
* @param[in] inOutParamInfoPtr        -
*                                      pointer to information about the parameters
*                                       None.
*/
void prvCpssLogParamFunc_GT_U16_HEX_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(GT_U16*, paramVal);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    if (paramVal == NULL)
    {
        PRV_CPSS_LOG_AND_HISTORY_PARAM_MAC(contextLib, logType, "%s = NULL\n", namePtr);
        return;
    }

    prvCpssLogStc16HexNumber(contextLib, logType, namePtr, *paramVal);
}

/**
* @internal prvCpssLogParamFunc_GT_U16_HEX function
* @endinternal
*
* @brief   logs a variable from GT_U16 type in hexa format
*
* @param[in] contextLib               -
*                                      lib log activity
* @param[in] logType                  -
*                                      type of the log
* @param[in] namePtr                  -
*                                      array name.
* @param[in] argsPtr                  -
*                                      additional args
* @param[in] skipLog                  -
*                                      to skip the log or not
* @param[in] inOutParamInfoPtr        -
*                                      pointer to information about the parameters
*                                       None.
*/
void prvCpssLogParamFunc_GT_U16_HEX(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(int, paramVal);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    prvCpssLogStc16HexNumber(contextLib, logType, namePtr, paramVal);
}

/**
* @internal prvCpssLogParamFunc_GT_U16_HEX_ARRAY function
* @endinternal
*
* @brief   logs an array of GT_U16_HEX elements
*
* @param[in] contextLib               -
*                                      lib log activity
* @param[in] logType                  -
*                                      type of the log
* @param[in] namePtr                  -
*                                      array name.
* @param[in] argsPtr                  -
*                                      additional args
* @param[in] skipLog                  -
*                                      to skip the log or not
* @param[in] inOutParamInfoPtr        -
*                                      pointer to information about the array of parameters
*                                       None.
*/
void prvCpssLogParamFunc_GT_U16_HEX_ARRAY(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(GT_U16*, paramVal);
    prvCpssLogArrayOfParamsHandle(contextLib,logType,namePtr,paramVal,sizeof(GT_U16),inOutParamInfoPtr,prvCpssLogParamFuncStc_GT_U16_HEX_PTR);
}
