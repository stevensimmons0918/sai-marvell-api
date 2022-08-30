/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE IS A REFERENCE CODE FOR MARVELL SWITCH PRODUCTS.  IT IS PROVIDED   *
* "AS IS" WITH NO WARRANTIES, EXPRESSED, IMPLIED OR OTHERWISE, REGARDING ITS   *
* ACCURACY, COMPLETENESS OR PERFORMANCE.                                       *
* CUSTOMERS ARE FREE TO MODIFY IT AND USE IT ONLY IN THEIR PRODUCTION          *
* SOFTWARE RELEASES WITH MARVELL SWITCH CHIPSETS.                              *
*******************************************************************************/
/**
********************************************************************************
* @file cpssEnablerUtils.h
*
* @brief Declarations of structures/enums to be used in common by CAP-
*        Cpss App Platform/Legacy AppDemo.
*
* @version  1 
********************************************************************************
*/
#ifndef __cpssEnablerUtilsh
#define __cpssEnablerUtilsh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
#include <cpss/common/cpssHwInit/cpssHwInit.h>
#ifdef CHX_FAMILY
#include <cpss/dxCh/dxChxGen/diag/cpssDxChDiagDataIntegrity.h>
#endif

#ifdef CPSS_APP_PLATFORM

#ifdef CHX_FAMILY
typedef GT_STATUS DXCH_DATA_INTEGRITY_EVENT_COUNTER_INCREMENT_FUNC
(
    IN  GT_U8                                       devNum,
    IN  CPSS_DXCH_DIAG_DATA_INTEGRITY_EVENT_STC     *eventPtr
);
#endif

#endif

typedef GT_VOID EVENT_NOTIFY_FUNC
(
    IN  GT_U8   devNum,
    IN GT_U32   uniEv,
    IN GT_U32   evExtData
);

extern EVENT_NOTIFY_FUNC *notifyEventArrivedFunc;

/* maximum length of the name of database entry */
#define CPSS_ENABLER_DB_NAME_MAX_LEN_CNS    128

/* maximum size of app demo database */
#define CPSS_ENABLER_DB_MAX_SIZE_CNS        100

#define CPSS_ENABLER_HW_ACCESS_DB_STEP_SIZE_CNS    _128K
/**
* @struct CPSS_ENABLER_DB_ENTRY_STC
 *
 * @brief Holds single app demo databse entry
*/
typedef struct{

    GT_CHAR name[CPSS_ENABLER_DB_NAME_MAX_LEN_CNS];

    /** value of the database entry */
    GT_U32 value;

} CPSS_ENABLER_DB_ENTRY_STC;

/**
* @enum CPSS_ENABLER_TRACE_HW_ACCESS_TYPE_ENT
 *
 * @brief PP access type enumeration
*/
typedef enum{

    /** PP access type is read. */
    CPSS_ENABLER_TRACE_HW_ACCESS_TYPE_READ_E,

    /** PP access type is write */
    CPSS_ENABLER_TRACE_HW_ACCESS_TYPE_WRITE_E,

    /** PP access type is read and write */
    CPSS_ENABLER_TRACE_HW_ACCESS_TYPE_BOTH_E,

    /** PP access type is write and delay */
    CPSS_ENABLER_TRACE_HW_ACCESS_TYPE_WRITE_DELAY_E,

    /** PP access type is read,write and delay */
    CPSS_ENABLER_TRACE_HW_ACCESS_TYPE_ALL_E

} CPSS_ENABLER_TRACE_HW_ACCESS_TYPE_ENT;

/**
* @enum CPSS_ENABLER_TRACE_OUTPUT_MODE_ENT
 *
 * @brief PP access type enumeration
*/
typedef enum{

    /** use osPrintf. */
    CPSS_ENABLER_TRACE_OUTPUT_MODE_DIRECT_E,

    /** use osPrintSynch need fo ISR debug */
    CPSS_ENABLER_TRACE_OUTPUT_MODE_DIRECT_SYNC_E,

    /** store the data in db */
    CPSS_ENABLER_TRACE_OUTPUT_MODE_DB_E,

    /** store the data in file */
    CPSS_ENABLER_TRACE_OUTPUT_MODE_FILE_E,

    /** the last parameter */
    CPSS_ENABLER_TRACE_OUTPUT_MODE_LAST_E

} CPSS_ENABLER_TRACE_OUTPUT_MODE_ENT;

/**
* @enum CPSS_ENABLER_FATAL_ERROR_TYPE
 *
 * @brief Enumeration for fatal error handling.
*/
typedef enum{

    /** @brief No execution of osFatalError
     *  from event handler.
     *  Error notification only.
     */
    CPSS_ENABLER_FATAL_ERROR_NOTIFY_ONLY_TYPE_E,

    /** @brief Execute osFatalError
     *  from event handler and
     *  error notification.
     */
    CPSS_ENABLER_FATAL_ERROR_EXECUTE_TYPE_E,

    /** @brief No execution of osFatalError,
     *  no error notification,
     *  increment Galtis event counter only.
     */
    CPSS_ENABLER_FATAL_ERROR_SILENT_TYPE_E

} CPSS_ENABLER_FATAL_ERROR_TYPE;

/**
* @enum APP_DEMO_AU_MESSAGE_HANDLER_METHORD_ENT
*
* @brief This enum defines different methods handling AU message processing 
*/
typedef enum APP_DEMO_AU_MESSAGE_HANDLER_METHORD_ENT{
    /* AU Message will be processed by the low level CPSS APIs - Old Method */
    APP_DEMO_AU_MESSAGE_HANDLER_LOW_LEVEL_E,

    /* AU Message will be processed by the FDB Manager APIs - New Method */
    APP_DEMO_AU_MESSAGE_HANDLER_FDB_MANAGER_E,

    /* AU Message will be processed by the FDB Manager APIs With Cuckoo - New Method */
    APP_DEMO_AU_MESSAGE_HANDLER_FDB_MANAGER_WITH_CUCKOO_E,

    /* AU Message will not be processed automatically,
     * (Explicit call to learning scan required)
     */
    APP_DEMO_AU_MESSAGE_HANDLER_DISABLED_E
} APP_DEMO_AU_MESSAGE_HANDLER_METHORD_ENT;

/* statistics about the 'auto aging' task appDemoFdbManagerAutoAging(...) */
typedef enum{
     APP_DEMO_PRV_CPSS_DXCH_FDB_MANAGER_AUTO_AGING_API_STATISTICS_OK_E
    ,APP_DEMO_PRV_CPSS_DXCH_FDB_MANAGER_AUTO_AGING_API_STATISTICS_ERROR_INPUT_INVALID_E
    ,APP_DEMO_PRV_CPSS_DXCH_FDB_MANAGER_AUTO_AGING_API_STATISTICS_ERROR_FAIL_E
    ,APP_DEMO_PRV_CPSS_DXCH_FDB_MANAGER_AUTO_AGING_API_STATISTICS_ERROR_HW_ERROR_E

    ,APP_DEMO_PRV_CPSS_DXCH_FDB_MANAGER_AUTO_AGING_API_STATISTICS___LAST___E
}APP_DEMO_PRV_CPSS_DXCH_FDB_MANAGER_AUTO_AGING_API_STATISTICS_ENT;

/**
* @struct APP_DEMO_CPSS_DXCH_BRG_FDB_MANAGER_STATISTICS_STC
 *
 * @brief app Demo statistics
*/
typedef struct
{
    /* Auto aging ok */
    GT_U32 autoAgingOk;

    /* Auto aging Error: input invalid */
    GT_U32 autoAgingErrorInputInvalid;

    /* Auto aging Error: Failed */
    GT_U32 autoAgingErrorFail;

    /* Auto aging Error: HW Error */
    GT_U32 autoAgingErrorHwError;
}APP_DEMO_CPSS_DXCH_BRG_FDB_MANAGER_STATISTICS_STC;

#ifdef CHX_FAMILY
/* check that port has valid mapping or not CPU port */
#define CPSS_ENABLER_PORT_SKIP_CHECK(dev,port)                                                   \
    if(PRV_CPSS_DXCH_PP_HW_INFO_PORT_MAP_CHECK_SUPPORTED_MAC(dev) == GT_TRUE)                    \
    {                                                                                            \
        GT_BOOL   _isCpu, _isValid;                                                                        \
        GT_STATUS _rc, _rc1;                                                                           \
        if ((port) >= PRV_CPSS_DXCH_MAX_PHY_PORT_NUMBER_MAC(dev))                                \
            break; /* no more */                                                                 \
        _rc = cpssDxChPortPhysicalPortMapIsCpuGet(dev, port, &_isCpu);                           \
        _rc1 = cpssDxChPortPhysicalPortMapIsValidGet(dev, port, &_isValid);                      \
        if((_rc != GT_OK) || (_isCpu == GT_TRUE) ||(_rc1 != GT_OK) || (_isValid != GT_TRUE ) )   \
            continue;                                                                            \
    }                                                                                            \
    else                                                                                         \
    {                                                                                            \
        if(PRV_CPSS_PP_MAC(dev)->phyPortInfoArray[port].portType == PRV_CPSS_PORT_NOT_EXISTS_E)  \
            continue;                                                                            \
    }
#endif

/**
* @struct CPSS_ENABLER_HW_ACCESS_DB_STC
 *
 * @brief This struct defines HW Access parameters and holds the data.
*/
typedef struct{

    /** @brief Holds the following access parameters in a bitmap:
     *  bit 7..0  device number
     *  bit 15..8  port group Id
     *  bit 16   ISR Context (1 - callback called from ISR context,
     *  0 - callback called not from ISR)
     *  bit 23..17 address space
     *  bit 24   Is the access for memory (1 - the access is for Memory,
     *  0 - the access is for Register)
     *  Note: Valid in case of bit0 is 1.
     *  bit 25   Memory end flag (1 - this access entry is memory end,
     *  0 - this access entry is not memory end)
     *  bit 26   access type ( 0 - read , 1 write)
     */
    GT_U32 accessParamsBmp;

    /** HW access address */
    GT_U32 addr;

    /** mask for read/written data */
    GT_U32 mask;

    /** The HW access read/written data */
    GT_U32 data;

} CPSS_ENABLER_HW_ACCESS_DB_STC;


/**
* @struct CPSS_ENABLER_HW_ACCESS_INFO_STC
 *
 * @brief This struct defines HW Access Info.
*/
typedef struct{

    /** The Max size allocated for HW Access DB in words. */
    GT_U32 maxSize;

    /** Current HW Access DB size in words */
    GT_U32 curSize;

    /** @brief Current DB state.
     *  GT_TRUE: trace HW Access to db enabled
     *  GT_FALSE: trace HW Access to db disabled
     */
    GT_BOOL outputToDbIsActive;

    /** @brief GT_TRUE: There was an attempt to write to a data base when data base was full
     *  GT_FALSE: There was no attempt to write to a data base when data base was full
     */
    GT_BOOL corrupted;

} CPSS_ENABLER_HW_ACCESS_INFO_STC;

GT_STATUS appRefHwAccessFailerBind
(
    IN GT_BOOL                  bind,
    GT_U32                      failCounter
);

GT_STATUS appRefHwAccessCounterGet
(
    OUT GT_U32*   hwAccessCounterPtr
);

GT_STATUS appRefTraceHwAccessClearDb
(
    GT_VOID
);

GT_STATUS appRefTraceHwAccessDbIsCorrupted
(
    GT_BOOL*      corruptedPtr
);

GT_STATUS appRefTraceHwAccessEnable
(
    IN GT_U8                                    devNum,
    IN CPSS_ENABLER_TRACE_HW_ACCESS_TYPE_ENT    accessType,
    IN GT_BOOL                                  enable
);

GT_STATUS appRefTraceHwAccessOutputModeSet
(
    IN CPSS_ENABLER_TRACE_OUTPUT_MODE_ENT   mode
);

GT_STATUS appRefTraceHwAccessInfoCompare
(
    IN GT_U8                            dev,
    CPSS_ENABLER_TRACE_HW_ACCESS_TYPE_ENT   accessType,
    IN GT_U32                           index,
    IN GT_U32                           portGroupId,
    IN GT_BOOL                          isrContext,
    IN CPSS_TRACE_HW_ACCESS_ADDR_SPACE_ENT addrSpace,
    IN GT_U32                           addr,
    IN GT_U32                           mask,
    IN GT_U32                           data
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssEnablerUtilsh */

