/*******************************************************************************
*              (c), Copyright 2013, Marvell International Ltd.                 *
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
* @file prvCpssMainPpDrvModGlobalNonSharedDb.h
*
* @brief This file define mainPpDrv module non shared variables defenitions.
*
* @version   1
********************************************************************************
*/

#ifndef __prvCpssMainPpDrvModGlobalNonSharedDb
#define __prvCpssMainPpDrvModGlobalNonSharedDb
#ifdef CHX_FAMILY
#include <cpss/dxCh/dxChxGen/txq/private/prvCpssDxChTxqMain.h>
#include <cpss/dxCh/dxChxGen/systemRecovery/catchUp/private/prvCpssDxChCatchUpDbg.h>

/**
 *@struct
 *        PRV_CPSS_MAIN_PP_DRV_MOD_LAB_SERVICES_DIR_NON_SHARED_FALCON_DEV_INIT_SRC_GLOBAL_DB
 *
 * @brief  Structure contain global variables that are not meant to be shared
 * between processes (when CPSS is linked as shared object)
 *   in mainPpDrv module ,labServices directory, src file :
 *   mvHwsHarrierpDevInit.c
*/
typedef struct
{
    HWS_UNIT_BASE_ADDR_CALC_BIND    falconBaseAddrCalcFunc;
} PRV_CPSS_MAIN_PP_DRV_MOD_LAB_SERVICES_DIR_NON_SHARED_FALCON_DEV_INIT_SRC_GLOBAL_DB;

/**
 *@struct
 *        PRV_CPSS_MAIN_PP_DRV_MOD_LAB_SERVICES_DIR_NON_SHARED_GLOBAL_DB
 *
 * @brief  Structure contain global variables that are not meant to be shared
 * between processes (when CPSS is linked as shared object)
 *   in mainPpDrv module ,labServices directory
*/
typedef struct
{
    PRV_CPSS_MAIN_PP_DRV_MOD_LAB_SERVICES_DIR_NON_SHARED_FALCON_DEV_INIT_SRC_GLOBAL_DB   falconDevinitSrc;

} PRV_CPSS_MAIN_PP_DRV_MOD_LAB_SERVICES_DIR_NON_SHARED_GLOBAL_DB;


/**
 *@struct PRV_CPSS_MAIN_PP_DRV_MOD_PORT_DIR_NON_SHARED_PORT_SPEED_SRC_GLOBAL_DB
 *
 * @brief  Structure contain global variables that are not meant to be shared
 * between processes (when CPSS is linked as shared object)
 *   in mainPpDrv module ,port  directory , source :
 *   cpssDxChPortSpeed.c
*/
typedef struct
{
    /*add here global shared variables used in source files*/

    /* Table of pointers to ifMode configuration functions per DXCH ASIC family
     * per interface mode if mode not supported NULL pointer provided
     */
    PRV_CPSS_DXCH_PORT_SPEED_SET_FUN portSpeedSetFuncPtrArray[CPSS_PP_FAMILY_END_DXCH_E-CPSS_PP_FAMILY_START_DXCH_E-1][PRV_CPSS_XG_PORT_OPTIONS_MAX_E];

    /* Array of pointers to port interface mode get functions per DXCH ASIC family */
    PRV_CPSS_DXCH_PORT_SPEED_GET_FUN portSpeedGetFuncPtrArray[CPSS_PP_FAMILY_END_DXCH_E-CPSS_PP_FAMILY_START_DXCH_E-1];
} PRV_CPSS_MAIN_PP_DRV_MOD_PORT_DIR_NON_SHARED_PORT_SPEED_SRC_GLOBAL_DB;


#endif
/**
* @struct PRV_CPSS_MAIN_PP_DRV_MOD_CPSS_HW_INIT_DIR_NON_SHARED_HW_INIT_SRC_GLOBAL_DB
 *
 * @brief  Structure contain global variables that are not ment to be shared
 * between processes (when CPSS is linked as shared object)
 *   in mainPpDrv module ,hwInit directory , source : cpssHwInit.c
*/
typedef struct
{
    /*add here global shared variables used in source files*/
    GT_U32 dummy;
} PRV_CPSS_MAIN_PP_DRV_MOD_CPSS_HW_INIT_DIR_NON_SHARED_HW_INIT_SRC_GLOBAL_DB;

#ifdef CHX_FAMILY

/**
* @struct PRV_CPSS_MAIN_PP_DRV_MOD_CPSS_TXQ_DIR_NON_SHARED_TXQ_MAIN_SRC_GLOBAL_DB
 *
 * @brief  Structure contain global variables that are not ment to be shared
 * between processes (when CPSS is linked as shared object)
 *   in mainPpDrv module ,txq  directory , source : prvCpssDxChTxqMain.c
*/
typedef struct
{
    /*add here global non- shared variables used in source files*/

    PRV_CPSS_DXCH_SIP6_TXQ_QUEUE_PROFILE_LONG_Q_STC  queuePdsProfiles[PRV_CPSS_DXCH_SIP6_TXQ_MAX_NUM_OF_SUPPORTED_SPEEDS_CNS];
    GT_BOOL                                          profilesInitialized;

} PRV_CPSS_MAIN_PP_DRV_MOD_CPSS_TXQ_DIR_NON_SHARED_TXQ_MAIN_SRC_GLOBAL_DB;

/**
* @struct PRV_CPSS_MAIN_PP_DRV_MOD_CPSS_TXQ_DIR_NON_SHARED_TXQ_MAIN_SRC_GLOBAL_DB
 *
 * @brief  Structure contain global variables that are not ment to be shared
 * between processes (when CPSS is linked as shared object)
 *   in mainPpDrv module ,txq  directory , source : prvCpssDxChTxqDebugUtils.c
*/
typedef struct
{
    /*add here global non -shared variables used in source files*/

    GT_CHAR logBuffer[PRV_TXQ_LOG_STRING_BUFFER_SIZE_CNS];

} PRV_CPSS_MAIN_PP_DRV_MOD_CPSS_TXQ_DIR_NON_SHARED_TXQ_DBG_SRC_GLOBAL_DB;


#endif
/**
* @struct PRV_CPSS_MAIN_PP_DRV_MOD_CPSS_HW_INIT_DIR_NON_SHARED_GLOBAL_DB
 *
 * @brief  Structure contain global variables that are not ment to be shared
 * between processes (when CPSS is linked as shared object)
 *   in mainPpDrv module ,hwInit directory
*/
typedef struct
{
    /*add here source files*/
    PRV_CPSS_MAIN_PP_DRV_MOD_CPSS_HW_INIT_DIR_NON_SHARED_HW_INIT_SRC_GLOBAL_DB hwInitSrc;

} PRV_CPSS_MAIN_PP_DRV_MOD_CPSS_HW_INIT_DIR_NON_SHARED_GLOBAL_DB;

#ifdef CHX_FAMILY

/**
 *@struct PRV_CPSS_MAIN_PP_DRV_MOD_PORT_DIR_SHARED_PORT_AP_SRC_GLOBAL_DB
 *
 * @brief  Structure contain global variables that are meant to be shared
 * between processes (when CPSS is linked as shared object)
 *   in mainPpDrv module ,port  directory , source :
 *   cpssDxChPortAp.c
*/
typedef struct
{
    /*add here global shared variables used in source files*/
    GT_U32                  allowPrint;
    GT_U32                  apSemPrintEn;
} PRV_CPSS_MAIN_PP_DRV_MOD_PORT_DIR_NON_SHARED_PORT_AP_SRC_GLOBAL_DB;

/**
 *@struct PRV_CPSS_MAIN_PP_DRV_MOD_PORT_DIR_SHARED_PORT_MANAGER_SAMPLES_SRC_GLOBAL_DB
 *
 * @brief  Structure contain global variables that are meant to be shared
 * between processes (when CPSS is linked as shared object)
 *   in mainPpDrv module ,port  directory , source :
 *   cpssDxChPortCtrl.c
*/
typedef struct
{
    /*add here global shared variables used in source files*/
    GT_U32      debug_halt_on_TXQ_stuck;
    GT_U32      tickTimeMsec; /* length of system tick in millisecond */
    GT_U32      prvCpssDxChPortXlgUnidirectionalEnableSetTraceEnable;
} PRV_CPSS_MAIN_PP_DRV_MOD_PORT_DIR_NON_SHARED_PORT_CTRL_SRC_GLOBAL_DB;

/**
 *@struct PRV_CPSS_MAIN_PP_DRV_MOD_PORT_DIR_SHARED_PORT_SERDES_CFG_SRC_GLOBAL_DB
 *
 * @brief  Structure contain global variables that are meant to be shared
 * between processes (when CPSS is linked as shared object)
 *   in mainPpDrv module ,port  directory , source :
 *   cpssDxChPortSerdesCfg.c
*/
typedef struct
{
    /*add here global shared variables used in source files*/
    GT_BOOL                                             forceDisableLowPowerMode;
    GT_BOOL                                             skipLion2PortDelete;
} PRV_CPSS_MAIN_PP_DRV_MOD_PORT_DIR_NON_SHARED_PORT_SERDES_CFG_SRC_GLOBAL_DB;

/**
* @struct PRV_CPSS_MAIN_PP_DRV_MOD_PORT_DIR_SHARED_GLOBAL_DB
*
* @brief  Structure contain global variables that are meant to be shared
* between processes (when CPSS is linked as shared object)
* in mainPpDrv module ,port directory
*/
typedef struct
{
    /** DB of port statistic module */
    PRV_CPSS_MAIN_PP_DRV_MOD_PORT_DIR_NON_SHARED_PORT_AP_SRC_GLOBAL_DB              portApSrc;
    PRV_CPSS_MAIN_PP_DRV_MOD_PORT_DIR_NON_SHARED_PORT_CTRL_SRC_GLOBAL_DB            portCtrlSrc;
    PRV_CPSS_MAIN_PP_DRV_MOD_PORT_DIR_NON_SHARED_PORT_SERDES_CFG_SRC_GLOBAL_DB      portSerdesCfgSrc;
#ifdef CHX_FAMILY
    PRV_CPSS_MAIN_PP_DRV_MOD_PORT_DIR_NON_SHARED_PORT_SPEED_SRC_GLOBAL_DB           portSpeedSrc;
#endif

} PRV_CPSS_MAIN_PP_DRV_MOD_PORT_DIR_NON_SHARED_GLOBAL_DB;

/**
* @struct PRV_CPSS_MAIN_PP_DRV_MOD_CPSS_HW_INIT_DIR_NON_SHARED_GLOBAL_DB
 *
 * @brief  Structure contain global variables that are not ment to be shared
 * between processes (when CPSS is linked as shared object)
 *   in mainPpDrv module ,txq directory
*/
typedef struct
{
    /*add here source files*/
    PRV_CPSS_MAIN_PP_DRV_MOD_CPSS_TXQ_DIR_NON_SHARED_TXQ_MAIN_SRC_GLOBAL_DB txqMainSrc;
    PRV_CPSS_MAIN_PP_DRV_MOD_CPSS_TXQ_DIR_NON_SHARED_TXQ_DBG_SRC_GLOBAL_DB  txqDbgSrc;

} PRV_CPSS_MAIN_PP_DRV_MOD_CPSS_TXQ_DIR_NON_SHARED_GLOBAL_DB;

/**
* @struct PRV_CPSS_MAIN_PP_DRV_MOD_CATCH_UP_DIR_NON_SHARED_CATCH_UP_DBG_SRC_GLOBAL_DB
 *
 * @brief  Structure contain global variables that are not ment to be shared
 * between processes (when CPSS is linked as shared object)
 *   in mainPpDrv module ,dxCh catchUp  directory , source : prvCpssDxChCatchUpDbg.h
*/
typedef struct
{
    PRV_CPSS_DXCH_CATCHUP_PARAMS_STC ppConfigCatchUpParams[PRV_CPSS_MAX_PP_DEVICES_CNS];

    /* array of MAC counters offsets, assigned with value PRV_CPSS_DXCH_CATCHUP_8_BIT_PATTERN_CNS for CatchUpDbg procedure. */
    GT_U8       gtMacCounterOffset[CPSS_LAST_MAC_COUNTER_NUM_E];
    GT_BOOL     isMacCounterOffsetInit ;

} PRV_CPSS_MAIN_PP_DRV_MOD_CATCH_UP_DIR_NON_SHARED_CATCH_UP_DBG_SRC_GLOBAL_DB;

/**
* @struct PRV_CPSS_MAIN_PP_DRV_MOD_CATCH_UP_DIR_NON_SHARED_CATCH_UP_SRC_GLOBAL_DB
 *
 * @brief  Structure contain global variables that are not ment to be shared
 * between processes (when CPSS is linked as shared object)
 *   in mainPpDrv module ,catch Up  directory , source : prvCpssDxChCatchUp.h
*/
typedef struct
{
    PRV_CPSS_DXCH_AUQ_ENABLE_DATA_STC *auqMsgEnableStatus[PRV_CPSS_MAX_PP_DEVICES_CNS] ;

} PRV_CPSS_MAIN_PP_DRV_MOD_CATCH_UP_DIR_NON_SHARED_CATCH_UP_SRC_GLOBAL_DB;



/**
* @struct PRV_CPSS_MAIN_PP_DRV_MOD_CPSS_CATCH_UP_DIR_NON_SHARED_GLOBAL_DB
 *
 * @brief  Structure contain global variables that are not ment to be shared
 * between processes (when CPSS is linked as shared object)
 *   in mainPpDrv module ,catch up directory
*/
typedef struct
{
    PRV_CPSS_MAIN_PP_DRV_MOD_CATCH_UP_DIR_NON_SHARED_CATCH_UP_DBG_SRC_GLOBAL_DB     catchUpDbgSrc;
    PRV_CPSS_MAIN_PP_DRV_MOD_CATCH_UP_DIR_NON_SHARED_CATCH_UP_SRC_GLOBAL_DB         catchUpSrc;

} PRV_CPSS_MAIN_PP_DRV_MOD_CPSS_CATCH_UP_DIR_NON_SHARED_GLOBAL_DB;

#endif /*CHX_FAMILY */
/**
* @struct PRV_CPSS_MAIN_PP_DRV_MOD_NON_SHARED_GLOBAL_DB
 *
 * @brief  Structure contain global variables that are not ment to be shared
 * between processes (when CPSS is linked as shared object)
 *   in mainPpDrv module
*/
typedef struct
{
    /*add here directories*/
    PRV_CPSS_MAIN_PP_DRV_MOD_CPSS_HW_INIT_DIR_NON_SHARED_GLOBAL_DB hwInitDir;
#ifdef CHX_FAMILY
    PRV_CPSS_MAIN_PP_DRV_MOD_CPSS_TXQ_DIR_NON_SHARED_GLOBAL_DB      txqDir;
    PRV_CPSS_MAIN_PP_DRV_MOD_CPSS_CATCH_UP_DIR_NON_SHARED_GLOBAL_DB catchUpDir;
    PRV_CPSS_MAIN_PP_DRV_MOD_PORT_DIR_NON_SHARED_GLOBAL_DB          portDir;
    PRV_CPSS_MAIN_PP_DRV_MOD_LAB_SERVICES_DIR_NON_SHARED_GLOBAL_DB  labServicesDir;
#endif

} PRV_CPSS_MAIN_PP_DRV_MOD_NON_SHARED_GLOBAL_DB;




#endif /* __prvCpssMainPpDrvModGlobalNonSharedDb */


