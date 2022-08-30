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
* @file appDemoBoardConfig.h
*
* @brief Includes board specific initialization definitions and data-structures.
*
* @version   29
********************************************************************************
*/
#ifndef __appDemoBoardConfigh
#define __appDemoBoardConfigh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifdef IMPL_FA
#include <cpssFa/generic/cpssFaHwInit/gtCoreFaHwInit.h>
#endif

#ifdef IMPL_XBAR
#include <cpssXbar/generic/xbarHwInit/gtCoreXbarHwInit.h>
#endif

#include <extUtils/common/cpssEnablerUtils.h>
#include <appDemo/os/appOs.h>
#include <appDemo/sysHwConfig/gtAppDemoSysConfig.h>
#ifdef CHX_FAMILY
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortMapping.h>
extern CPSS_DXCH_PP_PHASE1_INIT_INFO_STC ppPhase1ParamsDb;
#endif /*CHX_FAMILY*/

#define APP_DEMO_MAX_HIR_SUPPORTED_PP_CNS 32
#define IMPL_DBG_TRACE

#ifdef IMPL_DBG_TRACE
    #ifdef  _WIN32
        #define DBG_TRACE(x) osPrintf x
    #else /*!_WIN32*/
        #define DBG_TRACE(x) osPrintSync x
    #endif /*!_WIN32*/
#else
    #define DBG_TRACE(x)
#endif

#define APPDEMO_BAD_VALUE (GT_U32)(~0)

/* flag to state that the trace will print also functions that return GT_OK */
/* usually we not care about 'GT_OK' only about fails */
/* the flag can be changed before running the cpssInitSystem(...) --> from terminal vxWorks */
extern GT_U32  appDemoTraceOn_GT_OK;

#define CPSS_ENABLER_DBG_TRACE_RC_MAC(_title, _rc)     \
   if(_rc != GT_OK || appDemoTraceOn_GT_OK)             \
       DBG_TRACE(("%s returned: 0x%X at file %s, line = %d\r\n", _title, _rc, __FILE__, __LINE__))

extern GT_BOOL appDemoCpssInitSkipHwReset;/* indication that the appDemo initialize devices that skipped HW reset.
                    GT_TRUE  - the device is doing cpssInitSystem(...) after HW did NOT do soft/hard reset.
                    GT_FALSE - the device is doing cpssInitSystem(...) after HW did        soft/hard reset.
 */
extern GT_U32    appDemoCpssCurrentDevIndex;/* the appDemo device index in appDemoPpConfigList[] that currently initialized */
/* initialized in cpssInitSystem */
extern GT_BOOL  appDemoInitRegDefaults;

/* indication for working in 'port manager' mode (and port manager appDemo task) */
extern GT_BOOL portMgr;

/*******************************************************************************************/
/* Trace enabled by appDemoTraceEnableFilterAdd(line_start, line_end, file_name_substring) */

/**
* @internal appDemoPortManagerTaskCreate function
* @endinternal
*
* @brief   create port manager application task
*
* @param[in] devNum                 - pdevNum.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS appDemoPortManagerTaskCreate
(
    IN GT_U8 devNum
);

/**
* @internal appDemoEagleTsenTemperatureGetcreateTask function
* @endinternal
*
* @brief   create TSEN read from Eagle application task
*
* @param[in] devNum                 - pdevNum.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS appDemoEagleTsenTemperatureGetCreateTask
(
    IN  GT_U8                   devNum
);

/* Trace enabled by appDemoTraceEnableFilterAdd(line_start, line_end, file_name_substring) */
/**
* @internal appDemoTraceEnableFilterCheck function
* @endinternal
*
* @brief   This function ckecks is trace printing enable for this trace statement
*
* @param[in] filePath                 - path to source file returned by __FILE__ macro.
* @param[in] lineNum                  - number of line returned by __LINE__ macro.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_BOOL appDemoTraceEnableFilterCheck
(
    IN GT_U8  *filePath,
    IN GT_U32 lineNum
);

/* Trace */
/* Trace print line */
#define CPSS_ENABLER_FILTERED_TRACE_MAC(x) \
    if (appDemoTraceEnableFilterCheck((GT_U8*)__FILE__, (GT_U32)__LINE__) != GT_FALSE) {DBG_TRACE(x);}
/* Trace call evaluate expession on trace condition */
#define CPSS_ENABLER_FILTERED_TRACE_EXEC_MAC(x) \
    if (appDemoTraceEnableFilterCheck((GT_U8*)__FILE__, (GT_U32)__LINE__) != GT_FALSE) {x;}
/*******************************************************************************************/

/* when appDemoOnDistributedSimAsicSide == 1
   this image is running on simulation:
   1. our application on distributed "Asic side"
   2. the Other application that run on the "application side" is in
      charge about all Asic configurations
   3. our application responsible to initialize the cpss DB.
   4. the main motivation is to allow "Galtis wrappers" to be called on
      this side !
      (because the Other side application may have difficult running
       "Galtis wrappers" from there)
*/
extern GT_U32  appDemoOnDistributedSimAsicSide;

/*
 * Typedef: FUNCP_GET_BOARD_INFO
 *
 * Description:
 *      Provides board specific memory size configuration.
 *
 * Fields:
 *      boardRevId      - The board revision Id.
 *      numOfMemBytes   - Memory size in bytes.
 *
 */
typedef GT_STATUS (*FUNCP_GET_BOARD_MEMORY_INFO)
(
    IN  GT_U8   boardRevId,
    OUT GT_U32  *numOfMemBytes
);

/*
 * Typedef: FUNCP_BOARD_CLEAN_DB_DURING_SYSTEM_RESET
 *
 * Description:
 *      clear the DB of the specific board config file , as part of the 'system rest'
 *      to allow the 'cpssInitSystem' to run again as if it is the first time it runs
 *
 * Fields:
 *      boardRevId      - The board revision Id.
 *
 */
typedef GT_STATUS (*FUNCP_BOARD_CLEAN_DB_DURING_SYSTEM_RESET)
(
    IN  GT_U8   boardRevId
);

/*
 * Typedef: FUNCP_GET_BOARD_INFO
 *
 * Description:
 *      General board information for initial initialization.
 *
 * Fields:
 *      boardRevId      - The board revision Id.
 *      numOfPp         - Number of Packet processors in system.
 *      numOfFa         - Number of Fabric Adaptors in system.
 *      numOfXbar       - Number of Crossbar devices in system.
 *      isB2bSystem     - GT_TRUE, the system is a B2B system.
 *
 */
typedef GT_STATUS (*FUNCP_GET_BOARD_INFO)
(
    IN  GT_U8   boardRevId,
    OUT GT_U8   *numOfPp,
    OUT GT_U8   *numOfFa,
    OUT GT_U8   *numOfXbar,
    OUT GT_BOOL *isB2bSystem
);

#ifdef PX_FAMILY
/*
 * Typedef: FUNCP_GET_BOARD_HW_INFO
 *
 * Description:
 *      General board information for initial initialization.
 *
 * Fields:
 *      hwInfoPtr       - HW info structure.
 *      pciInfo         - PCI device information.
 *
 */
typedef GT_STATUS (*FUNCP_GET_BOARD_HW_INFO)
(
    OUT CPSS_HW_INFO_STC *hwInfoPtr,
    OUT GT_PCI_INFO      *pciInfo
);
#endif /*PX_FAMILY*/

#ifdef IMPL_XBAR
/*
 * Typedef: FUNCP_GET_CORE_XBAR_CFG_PARAMS
 *
 * Description:
 *
 * Fields:
 *      boardRevId      - The board revision Id.
 *      coreCfg1Params  - Core xbar config parameters struct.
 */
typedef GT_STATUS (*FUNCP_GET_CORE_XBAR_CFG_PARAMS)
(
    IN  GT_U8                   boardRevId,
    OUT GT_XBAR_CORE_SYS_CFG    *xbarCfgParams
);


/*
 * Typedef: FUNCP_GET_XBAR_PHASE1_PARAMS
 *
 * Description:
 *
 * Fields:
 *      boardRevId      - The board revision Id.
 *      xbarIdx         - The Xbar device index to get the parameters for.
 *      xbarPhase1Params- Phase1 parameters struct.
 *
 */
typedef GT_STATUS (*FUNCP_GET_XBAR_PHASE1_PARAMS)
(
    IN  GT_U8                       boardRevId,
    IN  GT_U8                       xbarIdx,
    OUT GT_XBAR_PHASE1_INIT_PARAMS  *phase1Params
);

/*
 * Typedef: FUNCP_GET_XBAR_PHASE2_PARAMS
 *
 * Description:
 *
 * Fields:
 *      boardRevId      - The board revision Id.
 *      oldXbardevNum   - The old Xbar device number to get the parameters for.
 *      xbarPhase2Params- Phase2 parameters struct.
 *
 */
typedef GT_STATUS (*FUNCP_GET_XBAR_PHASE2_PARAMS)
(
    IN  GT_U8                       boardRevId,
    IN  GT_U8                       oldXbardevNum,
    OUT GT_XBAR_PHASE2_INIT_PARAMS  *phase2Params
);

#endif

#ifdef IMPL_FA
/*
 * Typedef: FUNCP_GET_FA_PHASE1_PARAMS
 *
 * Description:
 *
 * Fields:
 *      boardRevId      - The board revision Id.
 *      faIdx           - The Fa device index to get the parameters for.
 *      faPhase1Params  - Phase1 parameters struct.
 *
 */
typedef GT_STATUS (*FUNCP_GET_FA_PHASE1_PARAMS)
(
    IN  GT_U8                       boardRevId,
    IN  GT_U8                       faIdx,
    OUT GT_FA_PHASE1_INIT_PARAMS    *phase1Params
);

/*
 * Typedef: FUNCP_GET_FA_PHASE2_PARAMS
 *
 * Description:
 *
 * Fields:
 *      boardRevId      - The board revision Id.
 *      oldFaDevNum     - The old Fa device number to get the parameters for.
 *      faPhase2Params  - Phase2 parameters struct.
 *
 */
typedef GT_STATUS (*FUNCP_GET_FA_PHASE2_PARAMS)
(
    IN  GT_U8                       boardRevId,
    IN  GT_U8                       oldFaDevNum,
    OUT GT_FA_PHASE2_INIT_PARAMS    *phase2Params
);


/*
 * Typedef: FUNCP_GET_FA_REG_CFG_LIST
 *
 * Description:
 *
 * Fields:
 *      boardRevId      - The board revision Id.
 *      devNum          - The device number to get the parameters for.
 *      regCfgList      - A pointer to the register list.
 *      regCfgListSize  - Number of elements in regListPtr.
 *
 */
typedef GT_STATUS (*FUNCP_GET_FA_REG_CFG_LIST)
(
    IN  GT_U8                       boardRevId,
    IN  GT_U8                       devNum,
    OUT CPSS_REG_VALUE_INFO_STC     **regCfgList,
    OUT GT_U32                      *regCfgListSize
);
#endif


/*
 * Typedef: FUNCP_BEFORE_PHASE1_CONFIG
 *
 * Description:  prototype for function that initialize "pre-phase 1"
 *               configuration
 *
 * Fields:
 *      boardRevId      - The board revision Id.
 *
 */
typedef GT_STATUS (*FUNCP_BEFORE_PHASE1_CONFIG)
(
    IN  GT_U8                       boardRevId
);

/*
 * Typedef: FUNCP_GET_PP_PHASE1_PARAMS
 *
 * Description:
 *
 * Fields:
 *      boardRevId      - The board revision Id.
 *      devIdx          - The Pp Index to get the parameters for.
 *      ppPhase1Params  - Phase1 parameters struct.
 *
 */
typedef GT_STATUS (*FUNCP_GET_PP_PHASE1_PARAMS)
(
    IN  GT_U8                       boardRevId,
    IN  GT_U8                       devIdx,
    OUT CPSS_PP_PHASE1_INIT_PARAMS *phase1Params
);


/*
 * Typedef: FUNCP_AFTER_PHASE1_CONFIG
 *
 * Description:
 *
 * Fields:
 *      boardRevId      - The board revision Id.
 *
 */
typedef GT_STATUS (*FUNCP_AFTER_PHASE1_CONFIG)
(
    IN  GT_U8   boardRevId
);


/*
 * Typedef: FUNCP_GET_PP_PHASE2_PARAMS
 *
 * Description:
 *
 * Fields:
 *      boardRevId      - The board revision Id.
 *      oldDevNum       - The old Pp device number to get the parameters for.
 *      ppPhase2Params  - Phase2 parameters struct.
 *
 */
typedef GT_STATUS (*FUNCP_GET_PP_PHASE2_PARAMS)
(
    IN  GT_U8                       boardRevId,
    IN  GT_U8                       oldDevNum,
    OUT CPSS_PP_PHASE2_INIT_PARAMS  *phase2Params
);


/*
 * Typedef: FUNCP_AFTER_PHASE2_CONFIG
 *
 * Description:
 *
 * Fields:
 *      boardRevId      - The board revision Id.
 *
 */
typedef GT_STATUS (*FUNCP_AFTER_PHASE2_CONFIG)
(
    IN  GT_U8   boardRevId
);

/*
 * Typedef: FUNCP_GET_PP_LOGICAL_INIT_PARAMS
 *
 * Description:
 *
 * Fields:
 *      boardRevId      - The board revision Id.
 *      devNum          - The Pp device number to get the parameters for.
 *      ppLogInitParams - Pp logical init parameters struct.
 *
 */
typedef GT_STATUS (*FUNCP_GET_PP_LOGICAL_INIT_PARAMS)
(
    IN  GT_U8                      boardRevId,
    IN  GT_U8                      devNum,
    OUT CPSS_PP_CONFIG_INIT_STC   *ppLogInitParams
);


/*
 * Typedef: FUNCP_GET_LIB_INIT_PARAMS
 *
 * Description:
 *
 * Fields:
 *      boardRevId      - The board revision Id.
 *      devNum          - The Pp device number to get the parameters for.
 *      libInitParams   - Library initialization parameters.
 *
 */
typedef GT_STATUS (*FUNCP_GET_LIB_INIT_PARAMS)
(
    IN  GT_U8                        boardRevId,
    IN  GT_U8                        devNum,
    OUT APP_DEMO_LIB_INIT_PARAMS    *libInitParams
);


/*
 * Typedef: FUNCP_AFTER_INIT_CONFIG
 *
 * Description:
 *
 * Fields:
 *      boardRevId      - The board revision Id.
 *      libInitParams   - Library initialization parameters.
 *
 */
typedef GT_STATUS (*FUNCP_AFTER_INIT_CONFIG)
(
    IN  GT_U8                       boardRevId
);

/* The following 5 function pointer were added to support stack. */
/*
 * Typedef: FUNCP_GET_PP_REG_CFG_LIST
 *
 * Description:
 *
 * Fields:
 *      boardRevId      - The board revision Id.
 *      devNum          - The device number to get the parameters for.
 *      regCfgList      - A pointer to the register list.
 *      regCfgListSize  - Number of elements in regListPtr.
 *
 */
typedef GT_STATUS (*FUNCP_GET_PP_REG_CFG_LIST)
(
    IN  GT_U8                       boardRevId,
    IN  GT_U8                       devNum,
    OUT CPSS_REG_VALUE_INFO_STC     **regCfgList,
    OUT GT_U32                      *regCfgListSize
);

/*
 * Typedef: FUNCP_GET_STACK_CONFIG_PARAMS
 *
 * Description:
 *
 * Fields:
 *      boardRevId      - The board revision Id.
 *      deviceIDsPtr    - List of the device IDs
 *      localUnitNum    - Local unit number
 *      stackParams     - Stack configuration parameters.
 *
 */
typedef GT_STATUS (*FUNCP_GET_STACK_CONFIG_PARAMS)
(
    IN  GT_U8                       boardRevId,
    IN  GT_U32                      *deviceIDsPtr,
    OUT GT_VOID                     *stackParams
);

/*
 * Typedef: FUNCP_GET_MNG_PORT_NAME
 *
 * Description:
 *
 * Fields:
 *      boardRevId      - The board revision Id.
 *      mngPortName     - Management port name string.
 *
 */
typedef GT_STATUS (*FUNCP_GET_MNG_PORT_NAME)
(
    IN  GT_U8                       boardRevId,
    OUT GT_U8                       *mngPortName
);

/*
 * Typedef: FUNCP_AFTER_STACK_READY_CONFIG
 *
 * Description:
 *
 * Fields:
 *      boardRevId      - The board revision Id.
 *
 */
typedef GT_STATUS (*FUNCP_AFTER_STACK_READY_CONFIG)
(
    IN  GT_U8                       boardRevId
);

/*
 * Typedef: FUNCP_SIMPLE_INIT
 *
 * Description:
 *
 * Fields:
 *      boardRevId      - The board revision Id.
 *
 */
typedef GT_STATUS (*FUNCP_SIMPLE_INIT)
(
    IN  GT_U8                       boardRevId
);

/*
 * Typedef: FUNCP_DEVICE_SIMPLE_INIT
 *
 * Description:
 *
 * Fields:
 *      devIdx          - Device index.
 *      boardRevId      - The board revision Id.
 *
 */
typedef GT_STATUS (*FUNCP_DEVICE_SIMPLE_INIT)
(
    IN  GT_U8   devIdx,
    IN  GT_U8   boardRevId
);

/*
 * Typedef: FUNCP_DEVICE_AFTER_INIT_CONFIG
 *
 * Description:
 *
 * Fields:
 *      devIdx          - Device index.
 *      boardRevId      - The board revision Id.
 *
 */
typedef GT_STATUS (*FUNCP_DEVICE_AFTER_INIT_CONFIG)
(
    IN  GT_U8   devIdx,
    IN  GT_U8   boardRevId
);

/*
 * Typedef: FUNCP_DEVICE_AFTER_PHASE1
 *
 * Description:
 *
 * Fields:
 *      devIdx          - Device index.
 *      boardRevId      - The board revision Id.
 *
 */
typedef GT_STATUS (*FUNCP_DEVICE_AFTER_PHASE1)
(
    IN  GT_U8   devIdx,
    IN  GT_U8   boardRevId
);

/*
 * Typedef: FUNCP_DEVICE_AFTER_PHASE2
 *
 * Description:
 *
 * Fields:
 *      devIdx          - Device index.
 *      boardRevId      - The board revision Id.
 *
 */
typedef GT_STATUS (*FUNCP_DEVICE_AFTER_PHASE2)
(
    IN  GT_U8   devIdx,
    IN  GT_U8   boardRevId
);

/*
 * Typedef: FUNCP_DEVICE_CLEANUP
 *
 * Description:
 *
 * Fields:
 *      devIdx          - Device index.
 *      boardRevId      - The board revision Id.
 *
 */

typedef GT_STATUS (*FUNCP_DEVICE_CLEANUP)
(
    IN  GT_U8   devIdx,
    IN  GT_U8   boardRevId
);

/*
 * Typedef: FUNCP_DEVICE_SWCLEANUP
 *
 * Description:
 *
 * Fields:
 *      devIdx          - Device index.
 *      boardRevId      - The board revision Id.
 *
 */
typedef GT_STATUS (*FUNCP_DEVICE_SWCLEANUP)
(
    IN  GT_U8   devIdx,
    IN  GT_U8   boardRevId
);

/**
* @enum APP_DEMO_CPSS_HIR_INSERTION_TYPE_ENT
*
* @brief Determines the type of initialization of PP device inserted in HIR Mode.
         Currently only full-init is supported.
*/
typedef enum{

    /* Full-Init */
    APP_DEMO_CPSS_HIR_INSERTION_FULL_INIT_E,

    /** the last parameter */
    APP_DEMO_CPSS_HIR_INSERTION_LAST_E

} APP_DEMO_CPSS_HIR_INSERTION_TYPE_ENT;

/**
* @internal FUNCP_DEVICE_SIMPLE_CPSS_PP_INSERT function
* @endinternal
*
* @brief   'board specific' support for Insert packet processor.
*       NOTE: called by cpssPpInsert(...)
*       and if such function exists , the cpssPpInsert(...) only 'count the time'
*
* @param[in] busType        - PP interface type connected to the CPU.
* @param[in] busNum         - PCI/SMI bus number.
* @param[in] busDevNum      - PCI/SMI Device number.
* @param[in] devNum         - cpss Logical Device number.
* @param[in] parentDevNum   - Device number of the device to which the
*                             current device is connected. A value 0xff
*                             means PP is directly connected to cpu.
* @param[in] insertionType  - place holder for future options to initialize PP.
*
* @retval GT_OK             - on success.
* @retval GT_BAD_PARAM      - on wrong devNum(>31). Already used busNum/busDevNum.
* @retval GT_FAIL           - otherwise.
*/
typedef GT_STATUS (*FUNCP_DEVICE_SIMPLE_CPSS_PP_INSERT)
(
    IN  CPSS_PP_INTERFACE_CHANNEL_ENT busType,
    IN  GT_U8  busNum,
    IN  GT_U8  busDevNum,
    IN  GT_SW_DEV_NUM  devNum,
    IN  GT_SW_DEV_NUM  parentDevNum,
    IN  APP_DEMO_CPSS_HIR_INSERTION_TYPE_ENT insertionType
);

/**
* @struct APP_DEMO_INIT_DEV_INFO_STC
 *
 * @brief PCI device init information
*/

typedef struct
{
/**Prestera PCI device Id */
GT_U32  devType;

/** software device Id */
GT_U32 swDevId;

/** hw device Id */
GT_U32 hwDevId;

/** The PCI bus number. */
GT_U32 pciBus;
/** The PCI device number. */
GT_U32  pciDev;

/** The PCI device function number */
GT_U32  pciFunc;

/** The PCI header info*/
GT_U32 pciHeaderInfo[16];

}APP_DEMO_INIT_DEV_INFO_STC;


extern GT_BOOL appDemoCpssPciProvisonDone;



/**
* @struct GT_BOARD_CONFIG_FUNCS
 *
 * @brief Includes board specific control functions.
*/
typedef struct
{
    FUNCP_GET_BOARD_MEMORY_INFO         boardGetMemInfo;
    FUNCP_GET_BOARD_INFO                boardGetInfo;
#ifdef PX_FAMILY
    FUNCP_GET_BOARD_HW_INFO             boardGetHwInfo;
#endif
#ifdef IMPL_XBAR
    FUNCP_GET_CORE_XBAR_CFG_PARAMS      boardGetXbarCfgParams;
#endif

#if defined (IMPL_FA) || defined (IMPL_XBAR)
    FUNCP_GET_XBAR_PHASE1_PARAMS        boardGetXbarPh1Params;
    FUNCP_GET_XBAR_PHASE2_PARAMS        boardGetXbarPh2Params;
#endif
#ifdef IMPL_FA
    FUNCP_GET_FA_PHASE1_PARAMS          boardGetFaPh1Params;
    FUNCP_GET_FA_PHASE2_PARAMS          boardGetFaPh2Params;
    FUNCP_GET_FA_REG_CFG_LIST           boardGetFaRegCfgList;
#endif

    FUNCP_BEFORE_PHASE1_CONFIG          boardBeforePhase1Config;

    FUNCP_GET_PP_PHASE1_PARAMS          boardGetPpPh1Params;
    FUNCP_AFTER_PHASE1_CONFIG           boardAfterPhase1Config;
    FUNCP_GET_PP_PHASE2_PARAMS          boardGetPpPh2Params;
    FUNCP_AFTER_PHASE2_CONFIG           boardAfterPhase2Config;
    FUNCP_GET_PP_LOGICAL_INIT_PARAMS    boardGetPpLogInitParams;
    FUNCP_GET_LIB_INIT_PARAMS           boardGetLibInitParams;
    FUNCP_AFTER_INIT_CONFIG             boardAfterInitConfig;
    FUNCP_GET_PP_REG_CFG_LIST           boardGetPpRegCfgList;
    FUNCP_GET_STACK_CONFIG_PARAMS       boardGetStackConfigParams;
    FUNCP_GET_MNG_PORT_NAME             boardGetMngPortName;
    FUNCP_AFTER_STACK_READY_CONFIG      boardAfterStackReadyConfig;

    FUNCP_BOARD_CLEAN_DB_DURING_SYSTEM_RESET boardCleanDbDuringSystemReset;
    FUNCP_SIMPLE_INIT                   boardSimpleInit;
    /* APIs related to PP insert/remove */
    FUNCP_DEVICE_SIMPLE_INIT            deviceSimpleInit;
    FUNCP_DEVICE_AFTER_INIT_CONFIG      deviceAfterInitConfig;
    FUNCP_DEVICE_AFTER_PHASE1           deviceAfterPhase1;
    FUNCP_DEVICE_AFTER_PHASE2           deviceAfterPhase2;
    FUNCP_DEVICE_CLEANUP                deviceCleanup;
    FUNCP_DEVICE_SIMPLE_CPSS_PP_INSERT  DeviceSimpleCpssPpInsert;
}GT_BOARD_CONFIG_FUNCS;


/*
 * Typedef: FUNCP_REGISTER_BOARD_FUNCS
 *
 * Description:
 *      Register board initialization functions.
 *
 * Fields:
 *
 */
typedef GT_STATUS (*FUNCP_REGISTER_BOARD_FUNCS)
(
    IN  GT_U8                   boardRevId,
    OUT GT_BOARD_CONFIG_FUNCS   *boardFuncs
);

/* number of different initializations for the specific board */
#define GT_BOARD_LIST_NUM_OF_BORAD_INITS_CNS   40
/* number of characters in name of board / name of revision */
#define GT_BOARD_LIST_STRING_LEN_CNS            100
/* string for revision that not used */
#define GT_BOARD_LIST_NOT_USE_REVISION_STR_CNS      "not used"

/**
* @struct GT_BOARD_LIST_ELEMENT
 *
 * @brief Holds a board's information for the show function and registration.
*/
typedef struct{

    /** Registration function pointer. */
    FUNCP_REGISTER_BOARD_FUNCS registerFunc;

    GT_CHAR boardName[GT_BOARD_LIST_STRING_LEN_CNS];

    GT_U8 numOfRevs;

    GT_CHAR boardRevs[GT_BOARD_LIST_NUM_OF_BORAD_INITS_CNS][GT_BOARD_LIST_STRING_LEN_CNS];

    GT_BOOL portMgr;

} GT_BOARD_LIST_ELEMENT;


/**
* @enum APP_DEMO_CPSS_LOG_TASK_MODE_ENT
 *
 * @brief Determines if to log all CPSS API calls or just calls by a specified task(s).
*/
typedef enum{

    /** log all threads API calls */
    APP_DEMO_CPSS_LOG_TASK_ALL_E,

    /** log single task API calls */
    APP_DEMO_CPSS_LOG_TASK_SINGLE_E,

    /** @brief log task(s) whose id is read from
     *  some specified address
     */
    APP_DEMO_CPSS_LOG_TASK_REF_E

} APP_DEMO_CPSS_LOG_TASK_MODE_ENT;

/**
* @enum APP_DEMO_CPSS_HIR_REMOVAL_TYPE_ENT
*
* @brief Determines the type of Removal PP device in HIR Mode.
*/
typedef enum{

    /* Managed Removal */
    APP_DEMO_CPSS_HIR_REMOVAL_MANAGED_E,

    /* Unexpected Removal or Crash */
    APP_DEMO_CPSS_HIR_REMOVAL_UNMANAGED_E,

    /* Managed full Reset*/
    APP_DEMO_CPSS_HIR_REMOVAL_RESET_E,

    /** the last parameter */
    APP_DEMO_CPSS_HIR_REMOVAL_LAST_E

} APP_DEMO_CPSS_HIR_REMOVAL_TYPE_ENT;

/**
* @enum APP_DEMO_SOC_MDC_FREQUENCY_MODE_ENT
*
* @brief Determines SOC MDC clock frequency.
*/
typedef enum{

    /* Normal Mode - 1.5MHz */
    APP_DEMO_SOC_MDC_FREQUENCY_MODE_NORMAL_E,

    /* Fast Mode - 12.5MHz */
    APP_DEMO_SOC_MDC_FREQUENCY_MODE_FAST_E,

    /* Accelerated Mode - 25MHz */
    APP_DEMO_SOC_MDC_FREQUENCY_MODE_ACCELERATED_E

} APP_DEMO_SOC_MDC_FREQUENCY_MODE_ENT;

/*******************************************************************************
* APP_DEMO_CALL_BACK_ON_REMOTE_PORTS_FUNC
*
* DESCRIPTION:
*       function called for remote physical ports to set extra config on the port,
*
* INPUTS:
*       devNum     - physical device number
*       portNum    - physical or CPU port number
*       linkUp     - indication that the port is currently 'up'/'down'
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK   - on success,
*       GT_FAIL - otherwise.
*
* COMMENTS:
*       the function called from the event handler task that recognized :
*       link_change / auto-negotiation ended
*
*******************************************************************************/
typedef GT_STATUS (*APP_DEMO_CALL_BACK_ON_REMOTE_PORTS_FUNC)
(
    IN GT_U8     devNum,
    IN GT_PHYSICAL_PORT_NUM  portNum,
    IN GT_BOOL  linkUp
);

typedef enum{
    APP_DEMO_CALL_BACK_ON_LINK_STATUS_CHANGE_BEFORE_MAIN_LOGIC_E,
    APP_DEMO_CALL_BACK_ON_LINK_STATUS_CHANGE_AFTER_MAIN_LOGIC_E,
}APP_DEMO_CALL_BACK_ON_LINK_STATUS_CHANGE_ENT;
/*******************************************************************************
* APP_DEMO_CALL_BACK_ON_LINK_STATUS_CHANGE_FUNC
*
* DESCRIPTION:
*       function called for 'link status change'
*
* INPUTS:
*       devNum     - physical device number
*       portNum    - physical port
*       linkUp     - indication that the port is currently 'up'/'down'
*                   relevant only to 'APP_DEMO_CALL_BACK_ON_LINK_STATUS_CHANGE_AFTER_MAIN_LOGIC_E'
*       partIndex  - indication of calling before main logic or after main logic.
*
* OUTPUTS:
*       stopLogicPtr - (pointer to) indication to stop the logic after CB ended.
*                      NOTE: relevant to 'APP_DEMO_CALL_BACK_ON_LINK_STATUS_CHANGE_BEFORE_MAIN_LOGIC_E'
*                       GT_TRUE - stop the logic after CB ended.
*                            meaning caller will not call the main logic
*                       GT_TRUE - continue the logic after CB ended.
*                            meaning caller call the main logic
*
* RETURNS:
*       GT_OK   - on success,
*       GT_FAIL - otherwise.
*
* COMMENTS:
*
*
*******************************************************************************/
typedef GT_STATUS (*APP_DEMO_CALL_BACK_ON_LINK_STATUS_CHANGE_FUNC)
(
    IN GT_U8     devNum,
    IN GT_PHYSICAL_PORT_NUM  portNum,
    IN GT_BOOL  linkUp,
    IN APP_DEMO_CALL_BACK_ON_LINK_STATUS_CHANGE_ENT partIndex,
    OUT GT_BOOL     *stopLogicPtr
);

/**
* @struct APP_DEMO_MICRO_INIT_STC
 *
 * @brief cpss micto-init parameters
*/
typedef struct{

    /** device Id */
    GT_U32 deviceId;

    /** interface channel to CPU */
    GT_U32 mngInterfaceType;

    /** packet processor core clock */
    GT_U32 coreClock;

    /** Own Device number */
    GT_U32 deviceNum;

    /** @brief cpss features participated in code generation
     *  Comments:
     */
    GT_U32 featuresBitmap;

} APP_DEMO_MICRO_INIT_STC;

extern APP_DEMO_CALL_BACK_ON_REMOTE_PORTS_FUNC appDemoCallBackOnRemotePortsFunc;
extern APP_DEMO_CALL_BACK_ON_LINK_STATUS_CHANGE_FUNC appDemoCallBackOnLinkStatusChaneFunc;


#define CPU_PORT    CPSS_CPU_PORT_NUM_CNS
#define IGNORE_DMA_STEP 0 /*value 0 or 1 means 'ignore' */
/*stay with same DMA - needed for remote ports */
#define DMA_NO_STEP   0xFFFF
#define SIZE_OF_MAC(arr)    sizeof(arr)/sizeof(arr[0])

/* value for APP_DEMO_PORT_MAP_STC::startTxqNumber
   means 'use startGlobalDmaNumber' for the definition  */
#define TXQ_BY_DMA_CNS   0xFFFF

/* based on : FALCON_PORT_MAP_STC */
typedef struct
{
    GT_PHYSICAL_PORT_NUM                startPhysicalPortNumber;
    GT_U32                              numOfPorts;
    GT_U32                              startGlobalDmaNumber;
    GT_U32                              startTxqNumber;/* value GT_TXQ_BY_DMA_CNS means 'use startGlobalDmaNumber' for the definition  */
#ifdef CHX_FAMILY
    CPSS_DXCH_PORT_MAPPING_TYPE_ENT     mappingType;
#else /*!CHX_FAMILY*/
    GT_U32                              mappingType;
#endif /*!CHX_FAMILY*/
    GT_U32                              jumpDmaPorts;/* allow to skip DMA ports (from startGlobalDmaNumber).
                                                        in case that mappingType is CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E
                                                        this field Contains the cascade port number*/
    GT_U32                              startRemoteMacNumber;/* relevant to 'CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E' indicate first the MAC number */
}APP_DEMO_PORT_MAP_STC;


/**
* @internal appDemoShowBoardsList function
* @endinternal
*
* @brief   This function displays the boards list which have an automatic
*         initialization support.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*
* @note The printed table should look like:
*       +-------------------------------+-----------------------+
*       | Board name          | Revisions       |
*       +-------------------------------+-----------------------+
*       | 01 - RD-EX120-24G       |            |
*       |                | 01 - Rev 1.0     |
*       |                | 02 - Rev 2.0     |
*       +-------------------------------+-----------------------+
*       | 02 - DB-MX610-48F4GS     |            |
*       |                | 01 - Rev 1.0     |
*       |                | 02 - Rev 2.5     |
*       +-------------------------------+-----------------------+
*
*/
GT_STATUS appDemoShowBoardsList
(
    GT_VOID
);


/**
* @internal cpssInitSystem function
* @endinternal
*
* @brief   This is the main board initialization function for CPSS driver.
*
* @param[in] boardIdx                 - The index of the board to be initialized from the board
*                                      list.
* @param[in] boardRevId               - Board revision Id.
* @param[in] reloadEeprom             - Whether the Eeprom should be reloaded when
*                                      corePpHwStartInit() is called.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS cpssInitSystem
(
    IN  GT_U32  boardIdx,
    IN  GT_U32  boardRevId,
    IN  GT_U32  reloadEeprom
);

/**
* @internal cpssReInitSystem function
* @endinternal
*
* @brief   Run cpssInitSystem according to previously initialized parameters
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS cpssReInitSystem
(
    GT_VOID
);

/**
* @internal appDemoTraceOn_GT_OK_Set function
* @endinternal
*
* @brief   This function configures 'print returning GT_OK' functions flag
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*
* @note - in unix-like systems special function needed to configure appDemoTraceOn_GT_OK
*       specific ifdef's needless, because this function can't harm anything
*       - must be external to ensure it's not ommited by optimization.
*
*/
GT_VOID appDemoTraceOn_GT_OK_Set
(
    GT_U32 enable
);


/**
* @internal appDemoOnDistributedSimAsicSideSet function
* @endinternal
*
* @brief   set value to the appDemoOnDistributedSimAsicSide flag.
*
* @param[in] value                    -  to be set
*
* @retval GT_OK                    - on success
* @retval GT_BAD_STATE             - if called after initialization of system -- after cpssInitSystem
*
* @note Function is called from the TERMINAL
*
*/
GT_STATUS appDemoOnDistributedSimAsicSideSet
(
    IN GT_U32  value
);

/**
* @internal appDemoTraceHwAccessEnable function
* @endinternal
*
* @brief   Trace HW read access information.
*
* @param[in] devNum                   - PP device number
* @param[in] accessType               - access type: read or write
* @param[in] enable                   - GT_TRUE:  tracing for given access type
*                                      GT_FALSE: disable tracing for given access type
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on fail
* @retval GT_BAD_PARAM             - on bad access type
*/
GT_STATUS appDemoTraceHwAccessEnable
(
    GT_U8                               devNum,
    CPSS_ENABLER_TRACE_HW_ACCESS_TYPE_ENT   accessType,
    GT_BOOL                             enable
);

/**
* @internal appDemoTraceHwAccessOutputModeSet function
* @endinternal
*
* @brief   Set output tracing mode.
*
* @param[in] mode                     - output tracing mode: print, printSync or store.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_STATE             - on bad state
* @retval GT_BAD_PARAM             - on bad mode
*
* @note It is not allowed to change mode, while one of the HW Access DB
*       is enabled.
*
*/
GT_STATUS appDemoTraceHwAccessOutputModeSet
(
    IN CPSS_ENABLER_TRACE_OUTPUT_MODE_ENT   mode
);

/**
* @internal appDemoDbPpDump function
* @endinternal
*
* @brief   Dump App Demo DB
*
* @retval GT_OK                    - GT_OK ant case
*/
GT_STATUS appDemoDbPpDump(void);

/**
* @internal appDemoDebugDeviceIdSet function
* @endinternal
*
* @brief   Set given device ID value for specific device.
*         Should be called before cpssInitSystem().
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_OUT_OF_RANGE          - on devIndex > 127
*/
GT_STATUS appDemoDebugDeviceIdSet
(
    IN GT_U8    devNum,
    IN GT_U32   deviceId
);

/**
* @internal appDemoDebugDeviceIdReset function
* @endinternal
*
* @brief   Reverts to HW device ID value for all devices.
*         Should be called before cpssInitSystem().
*
* @retval GT_OK                    - on success
*/
GT_STATUS appDemoDebugDeviceIdReset
(
    IN GT_VOID
);

/**
* @internal appDemoDebugDeviceIdGet function
* @endinternal
*
* @brief   Get given device ID value for specific device.
*
* @param[out] deviceIdPtr              - pointer to device type
*
* @retval GT_OK                    - on success
* @retval GT_NO_SUCH               - on not set debug device ID
* @retval GT_OUT_OF_RANGE          - on devIndex > 127
*/
GT_STATUS appDemoDebugDeviceIdGet
(
    IN  GT_U8    devNum,
    OUT GT_U32  *deviceIdPtr
);

/**
* @internal appDemoTrafficDisableSet function
* @endinternal
*
* @brief   Set traffic enable/disable mode - API that will enable/disable
*         execution of traffic enable algorithm during cpssInitSystem
* @param[in] disable                  - GT_TRUE - do not enable traffic during cpssInitSystem
*                                      - GT_FALSE - enable traffic during cpssInitSystem
*                                       None
*/
GT_VOID appDemoTrafficDisableSet
(
    IN GT_BOOL disable
);

/**
* @internal appDemoTraceHwAccessDbLine function
* @endinternal
*
* @brief   returns a line of the trace database, either the read or write database,
*         delay uses the write database
* @param[in] index                    -  of the line to retrieve
*
* @param[out] dbLine                   - A non-null pointer to a APP_DEMO_CPSS_HW_ACCESS_DB_STC struct
*
* @retval GT_OK                    - on success else if failed
*/
GT_STATUS appDemoTraceHwAccessDbLine
(
    GT_U32 index,
    OUT APP_DEMO_CPSS_HW_ACCESS_DB_STC * dbLine
);

/**
* @internal appDemoTraceHwAccessClearDb function
* @endinternal
*
* @brief   Clear HW access db
*
* @retval GT_OK                    - on success else if failed
*/
GT_STATUS appDemoTraceHwAccessClearDb
(
    GT_VOID
);

/**
* @internal appDemoTraceHwAccessDbIsCorrupted function
* @endinternal
*
* @brief   Check if the HW access data base has reached the limit
*
* @retval GT_OK                    - on success else if failed
*/

GT_STATUS appDemoTraceHwAccessDbIsCorrupted
(
    GT_BOOL * corrupted
);



/**
* @internal appDemoSupportAaMessageSet function
* @endinternal
*
* @brief   Configure the appDemo to support/not support the AA messages to CPU
*
* @param[in] supportAaMessage         -
*                                      GT_FALSE - not support processing of AA messages
*                                      GT_TRUE - support processing of AA messages
*                                       nothing
*/
GT_VOID appDemoSupportAaMessageSet
(
    IN GT_BOOL supportAaMessage
);
/**
* @internal cpssResetSystem function
* @endinternal
*
* @brief   API performs CPSS reset that includes device remove, mask interrupts,
*         queues and tasks destroy, all static variables/arrays clean.
* @param[in] doHwReset                - indication to do HW reset
*
* @retval GT_OK                    - on success else if failed
*/
GT_STATUS cpssResetSystem(
    IN GT_BOOL doHwReset
);

/**
* @internal cpssDevSupportSystemResetSet function
* @endinternal
*
* @brief   API controls if SYSTEM reset is supported, it is used to disable the
*         support on devices that do support. vice versa is not possible
*
* @retval GT_OK                    - on success else if failed
*/
GT_STATUS cpssDevSupportSystemResetSet(

    IN  GT_U8       devNum,
    IN  GT_BOOL     enable
);
/**
* @internal cpssDevMustNotResetPortSet function
* @endinternal
*
* @brief   API controls if SYSTEM reset (without HW reset) should skip reset of
*         specific port.
*         this for example needed for 'OOB' port (CPU out of band port) that serves
*         , for example : 'TELNET' . so need skip reset this port to avoid
*         disconnection of the session.
*
* @retval GT_OK                    - on success else if failed
*/
GT_STATUS cpssDevMustNotResetPortSet(

    IN  GT_U8       devNum,
    IN  GT_U32      portNum,
    IN  GT_BOOL     enable
);


/**
* @internal appDemoDevIdxGet function
* @endinternal
*
* @brief   Find device index in "appDemoPpConfigList" array based on device SW number.
*
* @param[in] devNum                   - SW device number
*
* @param[out] devIdxPtr                - (pointer to) device index in array "appDemoPpConfigList".
*
* @retval GT_OK                    - on success, match found for devNum.
* @retval GT_FAIL                  - no match found for devNum.
*/
GT_STATUS appDemoDevIdxGet
(
    IN  GT_U8   devNum,
    OUT GT_U32  *devIdxPtr
);

/************************************************************************/
/* Tracing specific registers access */
/************************************************************************/

/**
* @enum APP_DEMO_HW_PP_TRACING_ADDR_FILTER_ENT
 *
 * @brief Types of register address filters
*/
typedef enum
{
    /** Filter parameteres: Base, size */
    APP_DEMO_HW_PP_TRACING_ADDR_FILTER_BASE_SIZE_E,
    /** Filter parameteres: Mask, pattern  */
    APP_DEMO_HW_PP_TRACING_ADDR_FILTER_MASK_PATTERN_E,
    /** end of filer table mark */
    APP_DEMO_HW_PP_TRACING_ADDR_FILTER_TABLE_END_E

} APP_DEMO_HW_PP_TRACING_ADDR_FILTER_ENT;

/**
* @internal appDemoHwPpDrvRegTracingRegAddressesRangeSet function
* @endinternal
*
* @brief Set register address ranges to be traced.
*
* @param[in] regAddrMask0    - The first register's address mask.
* @param[in] regAddrPattern0 - The first register's address pattern.
* @param[in] ...             - other pairs of register's addresses masks and patterns
*                              terminated by 0 value
*
*/
void appDemoHwPpDrvRegTracingRegAddressesRangeSet
(
    IN GT_U32 regAddrMask0,
    IN GT_U32 regAddrPattern0,
    ...
);

/**
* @internal appDemoHwPpDrvRegTracingRegAddressesBoundsSet function
* @endinternal
*
* @brief Set register address bounds to be traced.
*
* @param[in] base0    - The first register's address base.
* @param[in] size0    - The first register's address bounds size.
* @param[in] ...             - other pairs of register's addresses base and size
*                              terminated by size==0 value
*
*/
void appDemoHwPpDrvRegTracingRegAddressesBoundsSet
(
    IN GT_U32 base0,
    IN GT_U32 size0,
    ...
);

/**
* @internal appDemoHwPpDrvRegTracingRegAddressesBoundsFromArraySet function
* @endinternal
*
* @brief Set register address bounds to be traced.
*
* @param[in] baseArrPtr    - (Pointer to array) bases: The first register's address base of address range.
* @param[in] sizeArrPtr    - (Pointer to array) sizess: The size or address range.
*                            The ranges list terminated by size==0 value in the array.
*
*/
void appDemoHwPpDrvRegTracingRegAddressesBoundsFromArraySet
(
    IN GT_U32 *baseArrPtr,
    IN GT_U32 *sizeArrPtr
);

/**
* @enum APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_TYPE_ENT
 *
 * @brief Types of register address clasification rule
*/
typedef enum
{
    /** Range parameteres: Base, size */
    APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_RANGE_E,
    /** Range parameteres: base, step, amount, size  */
    APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_RANGE_SERIAL_E,
    /** Range parameteres: addr */
    APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_ADDR_E,
    /** Range parameteres: base, step, amount  */
    APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_ADDR_SERIAL_E,
    /** marks end of array */
    APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_LAST_E,

} APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_TYPE_ENT;

/**
* @struct APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_STC
*
* @brief Structure for register address classification rule
*/
typedef struct
{
    /** state Id value for wich the rule is applicable */
    GT_U32                                          applicableId;
    /** enum rule type */
    APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_TYPE_ENT ruleType;
    /** new state Id value if the rule matches the address */
    GT_U32                                          resultApplicableId;
    /** the part of register name-path appended by this rule */
    const char*                                     resultName;
    /** rule type depended parameters */
    GT_U32                                          ruleParam[4];
} APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_STC;

/**
* @internal appDemoHwPpDrvRegTracingIsTraced function
* @endinternal
*
* @brief   Check is register address is traced.
*
* @param[in] regAddr    - The register's address.
*
* @retval 1             - the register address should be traced.
* @retval 0             - the register address should not be traced.
*
*/
GT_U32 appDemoHwPpDrvRegTracingIsTraced
(
    IN GT_U32 regAddr
);

/**
* @internal appDemoHwPpDrvRegTracingRegNameGet function
* @endinternal
*
* @brief   Get name (with path) of register by address.
*
* @param[in] rulesArr    - array of rules to reconize register by address.
* @param[in] regAddr     - address of register.
*
* @param[out] resultStr  - pointer to buffer for name of register.
*
*/
void appDemoHwPpDrvRegTracingRegNameGet
(
    IN APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_STC rulesArr[],
    IN GT_U32                                     regAddr,
    OUT char*                                     resultStr
);

/**
* @internal appDemoHwPpDrvRegTracingRegNameBind function
* @endinternal
*
* @brief   Bind register names array.
*
*/
void appDemoHwPpDrvRegTracingRegNameBind
(
    IN APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_STC *regNameArrPtr
);

/**
* @internal appDemoHwPpDrvRegTracingBind function
* @endinternal
*
* @brief   The function binds/unbinds a callback routines for HW access.
*
* @param[in] bind                     -  GT_TRUE -  callback routines.
*                                        GT_FALSE - un bind callback routines.
*
* @retval GT_OK                    - on success
*/
GT_STATUS appDemoHwPpDrvRegTracingBind
(
    GT_BOOL                     bind
);

#ifdef CHX_FAMILY
/**
* @internal appDemoDxChFillDbForCpssPortMappingInfo function
* @endinternal
*
* @brief   This function converts 'appDemo port mapping' style to
*           'cpss DXCH port mapping' style .
*
* @param[in] devNum                   - The device number.
* @param[in] appDemoInfoPtr           - array of port mapping in AppDemo style
* @param[in]  appDemoInfoPtr          - array of port mapping in CPSS style
*                                       array allocated array by the caller !!!
* @param[out] appDemoInfoPtr          - (filled) array of port mapping in CPSS style
* @param[out] numOfMappedPortsPtr     - (pointer to) the number of entries filled in appDemoInfoPtr[]
* @param[out] numCpuSdmasPtr          - (pointer to) the number of CPU SDMAs
*                                       can be NULL
*
* @retval GT_OK      - on success
* @retval GT_FULL    - asking for too many physical ports.
* @retval GT_NOT_IMPLEMENTED - for device that code not implemented
*
*/
GT_STATUS appDemoDxChFillDbForCpssPortMappingInfo(
    IN GT_SW_DEV_NUM                devNum,
    IN APP_DEMO_PORT_MAP_STC        *appDemoInfoPtr,
    INOUT CPSS_DXCH_PORT_MAP_STC    *cpssInfoPtr,
    OUT   GT_U32                    *numOfMappedPortsPtr,
    OUT   GT_U32                    *numCpuSdmasPtr
);

/**
* @internal appDemoDataIntegrityInit function
* @endinternal
*
* @brief   Data Integrity module initialization
*
* @param[in] devNum                   - device number
*/
GT_STATUS appDemoDataIntegrityInit
(
    IN  GT_U8   devNum
);

#endif /*CHX_FAMILY*/

/*******************************************************************************
* appDemo_configure_dma_per_devNum
*   Configure DMA for PP , per device using it's 'cpssDriver' to write the
*   registers needed in Falcon that the cpssDriver is complex
*  NOTE: implemented for SIP6 (and AC5)
*******************************************************************************/
GT_STATUS   appDemo_configure_dma_per_devNum(
    IN GT_U8    devNum,
    IN GT_BOOL  dmaInsidePp /* is DMA over the pex (GT_FALSE) or inside the PP (GT_TRUE) */
);

/**
* @internal cpssSystemBaseInit function
* @endinternal
*
* @brief   General initializations that are not relevant to specific devices. This part will
*          be called during CPSS init regardless to the existence of any PP in the system.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS cpssSystemBaseInit
(
    GT_VOID
);

/**
* @internal cpssPpInsert function
* @endinternal
*
* @brief   Insert packet processor.
* @param[in] busType        - PP interface type connected to the CPU.
* @param[in] busNum         - PCI/SMI bus number.
* @param[in] busDevNum      - PCI/SMI Device number.
* @param[in] devNum         - cpss Logical Device number.
* @param[in] parentDevNum   - Device number of the device to which the
*                             current device is connected. A value 0xff
*                             means PP is directly connected to cpu.
* @param[in] insertionType  - place holder for future options to initialize PP.
*
* @retval GT_OK             - on success.
* @retval GT_BAD_PARAM      - on wrong devNum(>31). Already used busNum/busDevNum.
* @retval GT_FAIL           - otherwise.
*/
GT_STATUS cpssPpInsert
(
    IN  CPSS_PP_INTERFACE_CHANNEL_ENT busType,
    IN  GT_U8 busNum,
    IN  GT_U8 busDevNum,
    IN  GT_SW_DEV_NUM devNum,
    IN  GT_SW_DEV_NUM parentDevNum,
    IN  APP_DEMO_CPSS_HIR_INSERTION_TYPE_ENT insertionType
);

/**
* @internal cpssPpRemove function
* @endinternal
*
* @brief   Remove packet processor.
* @param[in] devNum         - cpss Logical Device number.
* @param[in] removalType    - Removal type of a PP specifies managed/unmanaged Removal.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS cpssPpRemove
(
    IN  GT_SW_DEV_NUM devNum,
    IN  APP_DEMO_CPSS_HIR_REMOVAL_TYPE_ENT removalType
);

/**
* @internal cpssPpShowDevices function
* @endinternal
*
* @brief   List all PP devices' PCI info.
*
*/
void cpssPpShowDevices (void);

/**
* @internal cpssLspci function
* @endinternal
*
* @brief   List all detected Marvell PCI devices.
*
*/
void cpssLspci (void);

/**
* @internal cpssLspciGet function
* @endinternal
*
* @brief  Return a list of all detected Marvell PCI devices.
*
* @param[out] numOfPciDevElemPtr   -   number of valid elements
*                                      in pciInfoArray.
* @param[out] pciInfoArray         -   list of all detected PCI devices
*
*/
void cpssLspciGet
(
    OUT GT_U32       *numOfPciDevElemPtr,
    OUT GT_PCI_INFO  pciInfoArray[]
);

/**
* @internal cpssLsSmi function
* @endinternal
*
* @brief   List all detected Marvell SMI devices.
* @param[in] smiBus   -   SMI bus number.
*
*/
void cpssLsSmi (
    IN  GT_U8 smiBus
);

/**
* @internal cpssPciRemove function
* @endinternal
*
* @brief   Remove specific PCI device from Sysfs.
*
*/
void cpssPciRemove (
    IN  GT_U8 pciBus,
    IN  GT_U8 pciDev
);

/**
* @internal cpssPciRescan function
* @endinternal
*
* @brief   Rescan all PCI devices.
*
*/
void cpssPciRescan (void);

/**
* @internal appDemoSoCMDCFrequencySet function
* @endinternal
*
* @brief  set SoC MDC frequency
* @param[in] mode    -  0 - normal speed(/128),
                        1 - fast mode(/16),
                        2 - accel (/8)
*
*/
GT_STATUS appDemoSoCMDCFrequencySet (
    APP_DEMO_SOC_MDC_FREQUENCY_MODE_ENT mode
);

/* function to save value needed to be called to appDemoSoCMDCFrequencySet(...)
   after calling extDrvSoCInit()
   this function only save in DB the value !
*/
GT_STATUS appDemoSoCMDCFrequency_setInDb(IN     APP_DEMO_SOC_MDC_FREQUENCY_MODE_ENT mode);
/* function to get value needed to be called to appDemoSoCMDCFrequencySet(...)
   after calling extDrvSoCInit()
   this function only get saved value from DB!
   */
APP_DEMO_SOC_MDC_FREQUENCY_MODE_ENT appDemoSoCMDCFrequency_getFromDb(void);


/* prototype to wrap cpssDxChTrunkMemberRemove with extra logic */
typedef GT_STATUS (*appDemo_cpssDxChTrunkMemberRemove_CB_FUNC)
(
    IN GT_U8                    devNum,
    IN GT_TRUNK_ID              trunkId,
    IN CPSS_TRUNK_MEMBER_STC    *memberPtr
);
/* prototype to wrap cpssDxChTrunkMemberAdd with extra logic */
typedef GT_STATUS (*appDemo_cpssDxChTrunkMemberAdd_CB_FUNC)
(
    IN GT_U8                    devNum,
    IN GT_TRUNK_ID              trunkId,
    IN CPSS_TRUNK_MEMBER_STC    *memberPtr
);
/* prototype to wrap cpssDxChTrunkMembersSet with extra logic */
typedef GT_STATUS (*appDemo_cpssDxChTrunkMembersSet_CB_FUNC)
(
    IN GT_U8                    devNum,
    IN GT_TRUNK_ID              trunkId,
    IN GT_U32                   numOfEnabledMembers,
    IN CPSS_TRUNK_MEMBER_STC    enabledMembersArray[],
    IN GT_U32                   numOfDisabledMembers,
    IN CPSS_TRUNK_MEMBER_STC    disabledMembersArray[]
);

extern appDemo_cpssDxChTrunkMemberRemove_CB_FUNC appDemo_cpssDxChTrunkMemberRemove;
extern appDemo_cpssDxChTrunkMemberAdd_CB_FUNC    appDemo_cpssDxChTrunkMemberAdd;
extern appDemo_cpssDxChTrunkMembersSet_CB_FUNC   appDemo_cpssDxChTrunkMembersSet;


extern GT_STATUS appDemoWrap_osTaskDelete
(
    IN GT_TASK tid
);
extern GT_STATUS appDemoWrap_osTaskCreate
(
    IN  const GT_CHAR *name,
    IN  GT_U32  prio,
    IN  GT_U32  stack,
    IN  unsigned (__TASKCONV *start_addr)(void*),
    IN  void    *arglist,
    OUT GT_TASK *tid
);
/* prototype for CB function to clean task semaphores and flags before termination */
/* the function will be called with the 'cookie' from appDemoWrap_osTaskDelete()
   as was given during appDemoTaskSupportGracefulExit() */
typedef void (*appDemoTaskGracefulExitLastOperation_CB_FUNC)(IN void*   cookiePtr);

/* function called from the task it self to state that it supports 'Graceful Exit'     */
/* the cbFunc+cookiePtr will be called during appDemoWrap_osTaskDelete() for this task */
GT_STATUS appDemoTaskSupportGracefulExit(
        IN appDemoTaskGracefulExitLastOperation_CB_FUNC    cbFunc,
        IN void*   cookiePtr);

/* function called from the task to check if need to be terminated by osTaskDelete() */
/* calling this function is not expected to 'return' when the to osTaskDelete() need to 'kill' it */
void appDemoTaskCheckIfNeedTermination(void);
/* the 'system' notify to all 'supporting' tasks to be ready to terminate their
   operation and be ready for graceful exit */
void appDemoTaskStateToTerminateAllSupporingTasks_notify(void);
/* the 'system' FORCE all 'supporting' tasks to terminate now */
void appDemoTaskStateToTerminateAllSupporingTasks_force(void );

/* state that the task should not generate info to the LOG , because it
   is doing 'polling' */
void appDemoForbidCpssLogOnSelfSet(IN GT_U32   forbid);

/* state that the task should not generate 'registers trace' , because it
   is doing 'polling' */
void appDemoForbidCpssRegisterTraceOnSelfSet(IN GT_U32   forbid);

GT_STATUS appDemoPortInitSeqDeletePortStage
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum
);

/* replace cpssDxChPortModeSpeedSet when working with port manager */
GT_STATUS appDemoDxChPortMgrPortModeSpeedSet
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  GT_BOOL                         powerUp,
    IN  CPSS_PORT_INTERFACE_MODE_ENT    ifMode,
    IN  CPSS_PORT_SPEED_ENT             speed
);

/**
* @internal getDevFamily function
* @endinternal
*
* @brief   Gets the device family from vendor Id and device Id (read from PCI bus)
*
* @param[in] pciDevVendorIdPtr        - pointer to PCI/PEX device identification data.
*
* @param[out] devFamilyPtr             - pointer to CPSS PP device family.
* @param[out] isPexPtr                 - pointer to is PEX flag:
*                                      GT_TRUE - device has PEX interface
*                                      GT_FALSE - device has not PEX interface
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS getDevFamily(
    IN GT_PCI_DEV_VENDOR_ID    *pciDevVendorIdPtr,
    OUT CPSS_PP_FAMILY_TYPE_ENT *devFamilyPtr,
    OUT GT_BOOL     *isPexPtr
);

/**
* @internal appDemoBusDevInfo function
* @endinternal
*
* @brief   This routine sets the PCI device init information of the managed device(s) by this CPSS instance
*
* @param[in] deviceArray           - init information array for managed devices.
*
* @param[in] sizeOfArray           - number of elements in array
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - otherwise.
*
* @note
*       None.
*
*/

GT_STATUS appDemoBusDevInfo
(
    IN APP_DEMO_INIT_DEV_INFO_STC deviceArray[],
    IN GT_U32 sizeOfArray
);

/**
* @internal appDemoSystemRecoveryHa2phasesInitDataSet function
* @endinternal
*
* @brief  This function is intended to set data for HA two phases init.
*
* @note   APPLICABLE DEVICES:      Aldrin2.
* @note   NOT APPLICABLE DEVICES:  Lion2; Bobcat2; AC3X; xCat3; AC5; Caelum;
*                                  Aldrin; Bobcat3; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                - device number
*
 *@param[in] ha2PhasesPhase1ParamsPhase1- (pointer to) struct
 *             that contain parameters for phase1 of HA two
 *             phases Init.
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on passing null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note none.
*
*/

GT_STATUS appDemoSystemRecoveryHa2phasesInitDataSet
(
    IN GT_U8 devNum,
    IN CPSS_PP_HA_2_PHASES_INIT_PHASE1_STC *ha2PhasesPhase1ParamsPhase1
);

/**
* @internal appDemoSystemRecoveryHa2phasesInitDataGet function
* @endinternal
*
 *@brief  This function is intended to get data for HA two
 *        phases init.
*
* @note   APPLICABLE DEVICES:      Aldrin2.
* @note   NOT APPLICABLE DEVICES:  Lion2; Bobcat2; AC3X; xCat3; AC5; Caelum;
*                                  Aldrin; Bobcat3; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                - device number
*
* @param[out] ha2PhasesPhase1ParamsPhase1- (pointer to) struct that contain
*              parameters for phase1 of HA two phases Init.


* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on passing null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note none.
*
*/

GT_STATUS appDemoSystemRecoveryHa2phasesInitDataGet
(
    IN GT_U8 devNum,
    OUT CPSS_PP_HA_2_PHASES_INIT_PHASE1_STC *ha2PhasesPhase1ParamsPhase1
);

/**
* @internal appDemoSystemRecoveryHsPostCompletionState function
* @endinternal
*
*@brief  This function wraps all appDemo functions that needs to be called at the end of Hitless startup (HS) process,
*        after completion stage.
*        meaning need to be called after calling cpssSystemRecoveryStateSet with CPSS_SYSTEM_RECOVERY_COMPLETION_STATE_E
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - otherwise.
*
*/
GT_STATUS appDemoSystemRecoveryHsPostCompletionState
(
    GT_VOID
);

/** @struct APP_DEMO_LED_UNIT_CONFIG_STC
*
*   @brief LED Unit configurations.
*
*/
typedef struct {
    /** @brief The first bit in the LED stream indication to be driven in current LED unit.
     *  (APPLICABLE RANGES: 0..255) */
    GT_U32     ledStart;

    /** @brief The last bit in the LED stream indication to be driven in the current LED unit.
     *  (APPLICABLE RANGES: 0..255) */
    GT_U32     ledEnd;

    /** Bypass LED unit */
    GT_BOOL    bypassLedUnit;
} APP_DEMO_LED_UNIT_CONFIG_STC;

/** @struct APP_DEMO_LED_PORT_MAC_POSITION_STC
*
*   @brief Structure maps port MAC number into it's LED port
*          position in LED server.
*
*/
typedef struct {
    /** MAC number of port */
    GT_U32      macPortNumber;

    /** LED Position of port */
    GT_U32      ledPortPosition;
}APP_DEMO_LED_PORT_MAC_POSITION_STC;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __appDemoBoardConfigh */




