/*******************************************************************************
*              (c), Copyright 2013, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *\
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
*/
/**
********************************************************************************
* @file prvCpssMainPpDrvModGlobalSharedDb.h
*
* @brief This file define mainPpDrv module shared variables defenitions.
*
* @version   1
********************************************************************************
*/

#ifndef __prvCpssMainPpDrvModGlobalSharedDb
#define __prvCpssMainPpDrvModGlobalSharedDb

#include <cpss/common/diag/private/prvCpssCommonDiag.h>
#include <cpss/common/cpssHwInit/private/prvCpssHwInit.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>

#ifdef CHX_FAMILY
#include <cpss/dxCh/dxChxGen/ptp/private/prvCpssDxChPtp.h>
#include <cpss/dxCh/dxChxGen/packetAnalyzer/private/prvCpssDxChPacketAnalyzer.h>
#include <cpss/dxCh/dxChxGen/flowManager/prvCpssDxChFlowManagerDb.h>
#include <cpss/dxCh/dxChxGen/bridgeFdbManager/private/prvCpssDxChBrgFdbManager_db.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwRegAddrVer1_ppDb.h>
#include <cpss/dxCh/dxChxGen/virtualTcam/private/prvCpssDxChVirtualTcamDb.h>
#include <cpss/dxCh/dxChxGen/cuckoo/prvCpssDxChCuckoo.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwInitTypes.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChErrataMng.h>
#include <cpss/dxCh/dxChxGen/port/PizzaArbiter/cpssDxChPortPizzaArbiterProfile.h>
#include <cpss/dxCh/dxChxGen/port/PizzaArbiter/DynamicPizzaArbiter/prvCpssDxChPortDynamicPAUnitDrv.h>
#include <cpss/dxCh/dxChxGen/port/PizzaArbiter/DynamicPizzaArbiter/prvCpssDxChPortDynamicPizzaArbiterWS.h>
#include <cpss/dxCh/dxChxGen/port/PizzaArbiter/TablePizzaArbiter/prvCpssDxChPortPizzaArbiterProfileStorage.h>
#include <cpss/dxCh/dxChxGen/exactMatchManager/private/prvCpssDxChExactMatchManager_db.h>
#include <cpss/dxCh/dxChxGen/port/private/prvCpssDxChPortIfModeCfgBcat2Resource.h>
#include <cpss/dxCh/dxChxGen/port/private/prvCpssDxChPortIfModeCfgBobKResource.h>
#include <cpss/dxCh/dxChxGen/tti/private/prvCpssDxChTtiTypes.h>
#include <cpss/dxCh/dxChxGen/tti/cpssDxChTtiTypes.h>
#include <cpss/dxCh/dxChxGen/private/lpm/ram/prvCpssDxChSip6LpmRamMng.h>
#include <cpss/dxCh/dxChxGen/private/lpm/ram/prvCpssDxChLpmRam.h>
#include <cpss/dxCh/dxChxGen/private/lpm/tcam/prvCpssDxChLpmTcamPatTrie.h>
#endif

#define MAX_LPM_LEVELS_CNS 17

/**
* @struct PRV_CPSS_MAIN_PP_DRV_MOD_SHARED_GLOBAL_DB
 *
 * @brief  Structure contain global variables that are meant to be shared
 * between processes (when CPSS is linked as shared object)
 *   in mainPpDrv module ,hwInit directory , source : cpssHwInit.c
*/
typedef struct
{
    /** DB of hw devices modes. By default devices are in single mode */
    /** support <hwDevNum> of 12 bits (Lion2B) */
    CPSS_GEN_CFG_HW_DEV_NUM_MODE_ENT prvCpssHwDevNumModeDb[BIT_12];

} PRV_CPSS_MAIN_PP_DRV_MOD_CPSS_HW_INIT_DIR_SHARED_HW_INIT_SRC_GLOBAL_DB;

/**
* @struct PRV_CPSS_MAIN_PP_DRV_MOD_SHARED_GLOBAL_DB
 *
 * @brief  Structure contain global variables that are meant to be shared
 * between processes (when CPSS is linked as shared object)
 *   in mainPpDrv module ,hwInit directory
*/
typedef struct
{
    /*add here source files*/
    PRV_CPSS_MAIN_PP_DRV_MOD_CPSS_HW_INIT_DIR_SHARED_HW_INIT_SRC_GLOBAL_DB hwInitSrc;

} PRV_CPSS_MAIN_PP_DRV_MOD_CPSS_HW_INIT_DIR_SHARED_GLOBAL_DB;

#ifdef CHX_FAMILY
/**
 *@struct
 *        PRV_CPSS_MAIN_PP_MOD_LAB_SERVICES_DIR_SHARED_SAC5P_DEV_INIT_SRC_GLOBAL_DB
 *
 * @brief  Structure contain global variables that are meant to be shared
 * between processes (when CPSS is linked as shared object)
 *   in mainPpDrv module ,labServices directory, src file :
 *   mvHwsAc5pDevInit.c
*/
typedef struct
{
    GT_BOOL hwsInitDone[HWS_MAX_DEVICE_NUM];
    GT_BOOL hawkDbInitDone;
} PRV_CPSS_MAIN_PP_DRV_MOD_LAB_SERVICES_DIR_SHARED_AC5P_DEV_INIT_SRC_GLOBAL_DB;

/**
 *@struct
 *        PRV_CPSS_MAIN_PP_MOD_LAB_SERVICES_DIR_SHARED_AC5X_DEV_INIT_SRC_GLOBAL_DB
 *
 * @brief  Structure contain global variables that are meant to be shared
 * between processes (when CPSS is linked as shared object)
 *   in mainPpDrv module ,labServices directory, src file :
 *   mvHwsAc5xDevInit.c
*/
typedef struct
{
    GT_BOOL hwsInitDone[HWS_MAX_DEVICE_NUM];
    GT_BOOL phoenixDbInitDone;
} PRV_CPSS_MAIN_PP_DRV_MOD_LAB_SERVICES_DIR_SHARED_AC5X_DEV_INIT_SRC_GLOBAL_DB;

/**
 *@struct
 *        PRV_CPSS_MAIN_PP_MOD_LAB_SERVICES_DIR_SHARED_HARRIER_DEV_INIT_SRC_GLOBAL_DB
 *
 * @brief  Structure contain global variables that are meant to be shared
 * between processes (when CPSS is linked as shared object)
 *   in mainPpDrv module ,labServices directory, src file :
 *   mvHwsHarrierpDevInit.c
*/
typedef struct
{
    GT_BOOL hwsInitDone[HWS_MAX_DEVICE_NUM];
    GT_BOOL harrierDbInitDone;
} PRV_CPSS_MAIN_PP_DRV_MOD_LAB_SERVICES_DIR_SHARED_HARRIER_DEV_INIT_SRC_GLOBAL_DB;


/**
 *@struct
 *        PRV_CPSS_COMMON_MOD_LAB_SERVICES_DIR_SHARED_GLOBAL_DB
 *
 * @brief  Structure contain global variables that are meant to be shared
 * between processes (when CPSS is linked as shared object)
 *   in mainPpDrv module ,labServices directory, src file :
 *   mvHwsIpcApis.c
*/
typedef struct
{
    PRV_CPSS_MAIN_PP_DRV_MOD_LAB_SERVICES_DIR_SHARED_AC5P_DEV_INIT_SRC_GLOBAL_DB     ac5pDevInitSrc;
    PRV_CPSS_MAIN_PP_DRV_MOD_LAB_SERVICES_DIR_SHARED_AC5X_DEV_INIT_SRC_GLOBAL_DB     ac5xDevInitSrc;
    PRV_CPSS_MAIN_PP_DRV_MOD_LAB_SERVICES_DIR_SHARED_HARRIER_DEV_INIT_SRC_GLOBAL_DB  harrierDevInitSrc;
} PRV_CPSS_MAIN_PP_DRV_MOD_LAB_SERVICES_DIR_SHARED_GLOBAL_DB;

/**
 *@struct PRV_CPSS_MAIN_PP_DRV_MOD_PORT_DIR_SHARED_PORT_MANAGER_SAMPLES_SRC_GLOBAL_DB
 *
 * @brief  Structure contain global variables that are meant to be shared
 * between processes (when CPSS is linked as shared object)
 *   in mainPpDrv module ,port  directory , source :
 *   cpssDxChPortManagerSamples.c
*/
typedef struct
{
    /*add here global shared variables used in source files*/
    GT_BOOL     inloopBackMode[CPSS_MAX_PORTS_NUM_CNS];
} PRV_CPSS_MAIN_PP_DRV_MOD_PORT_DIR_SHARED_PORT_MANAGER_SAMPLES_SRC_GLOBAL_DB;

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
    GT_BOOL     aldrinDebugCpllInitFlag;
    GT_BOOL     bobkDebugCpllInitFlag;
    GT_BOOL     bc3DebugCpllInitFlag;
} PRV_CPSS_MAIN_PP_DRV_MOD_PORT_DIR_SHARED_PORT_CTRL_SRC_GLOBAL_DB;

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
    GT_U16                                              numOfSerdesRxauiConfig;
    CPSS_DXCH_PORT_SERDES_AC3X1690TX_PARAM_CONFIG_STC   *serdesRxauiTxConfig;

    /* array of pointers to port SerDes power set functions per DXCH ASIC family */
    PRV_CPSS_DXCH_PORT_SERDES_POWER_STATUS_SET_FUN portSerdesPowerStatusSetFuncPtrArray[CPSS_PP_FAMILY_END_DXCH_E-CPSS_PP_FAMILY_START_DXCH_E-1];
} PRV_CPSS_MAIN_PP_DRV_MOD_PORT_DIR_SHARED_PORT_SERDES_CFG_SRC_GLOBAL_DB;


/**
* @struct PRV_CPSS_MAIN_PP_DRV_MOD_PORT_DIR_SHARED_PORT_STAT_SRC_GLOBAL_DB
 *
 * @brief  Structure contain global variables that are meant to be shared
 * between processes (when CPSS is linked as shared object)
 *   in mainPpDrv module ,port  directory , source : cpssDxChPortStat.c
*/
typedef struct
{
    /*add here global shared variables used in source files*/
    GT_BOOL isMacCounterOffsetInit;
} PRV_CPSS_MAIN_PP_DRV_MOD_PORT_DIR_SHARED_PORT_STAT_SRC_GLOBAL_DB;

typedef struct prvIfModeSupportedPortsModes_STC
{
    PRV_CPSS_PORT_TYPE_OPTIONS_ENT  portType;
    CPSS_PORT_INTERFACE_MODE_ENT   *supportedIfList;
}prvIfModeSupportedPortsModes_STC;

/**
 *@struct PRV_CPSS_MAIN_PP_DRV_MOD_PORT_DIR_SHARED_PORT_IF_MODE_CFG_SRC_GLOBAL_DB
 *
 * @brief  Structure contain global variables that are meant to be shared
 * between processes (when CPSS is linked as shared object)
 *   in mainPpDrv module ,port  directory , source :
 *   cpssDxChPortIfModeCfg.c
*/
typedef struct
{
    /*add here global shared variables used in source files*/
    GT_U32 prvPrintSpeedChangeTime;
    CPSS_PORT_INTERFACE_MODE_ENT supportedPortsModes_XG_PORT_XG_ONLY[2];
    CPSS_PORT_INTERFACE_MODE_ENT supportedPortsModes_XG_PORT_HX_QX_ONLY[4];
    CPSS_PORT_INTERFACE_MODE_ENT supportedPortsModes_XG_PORT_XG_HX_QX[8];
    CPSS_PORT_INTERFACE_MODE_ENT supportedPortsModes_GE_PORT_GE_ONLY[9];
    CPSS_PORT_INTERFACE_MODE_ENT supportedPortsModes_XG_PORT_XLG_SGMII[15]; /* Lion - sip 5.15*/
    CPSS_PORT_INTERFACE_MODE_ENT supportedPortsModes_XG_PORT_CG_SGMII[20]; /* sip 5.20 and above , flex link */
    prvIfModeSupportedPortsModes_STC supportedPortsModesList[7];
} PRV_CPSS_MAIN_PP_DRV_MOD_PORT_DIR_SHARED_PORT_IF_MODE_CFG_SRC_GLOBAL_DB;

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
    PRV_CPSS_MAIN_PP_DRV_MOD_PORT_DIR_SHARED_PORT_STAT_SRC_GLOBAL_DB                portStatSrc;
    PRV_CPSS_MAIN_PP_DRV_MOD_PORT_DIR_SHARED_PORT_MANAGER_SAMPLES_SRC_GLOBAL_DB     portManagerSampleSrc;
    PRV_CPSS_MAIN_PP_DRV_MOD_PORT_DIR_SHARED_PORT_CTRL_SRC_GLOBAL_DB                portCtrlSrc;
    PRV_CPSS_MAIN_PP_DRV_MOD_PORT_DIR_SHARED_PORT_SERDES_CFG_SRC_GLOBAL_DB          portSerdesCfgSrc;
    PRV_CPSS_MAIN_PP_DRV_MOD_PORT_DIR_SHARED_PORT_IF_MODE_CFG_SRC_GLOBAL_DB         portIfModeCfgSrc;
} PRV_CPSS_MAIN_PP_DRV_MOD_PORT_DIR_SHARED_GLOBAL_DB;

/**
* @struct PRV_CPSS_MAIN_PP_DRV_MOD_DXCH_PORT_DIR_SHARED_PORT_PA_SRC_GLOBAL_DB
 *
 * @brief  Structure contain global variables that are meant to be shared
 * between processes (when CPSS is linked as shared object)
*    in mainPpDrv module ,port  directory , sources :
*    port/PizzaArbiter/cpssDxChPortPizzaArbiter.c
*/
typedef struct
{
    /** Pointer to resources driver for Bobcat2
     *  Actual DB is
     *  PRV_CPSS_DXCH_ARR_DRV_STC      prv_dxChBcat2B0 */
    GT_VOID_PTR prv_dxChBcat2B0Ptr;

    /** Pointer to fields array for resources driver of Bobcat2
     *  Acual DB is
     *  PRV_CPSS_DRV_FLD_ARR_DEF_STC   prv_dxChBcat2B0_FldInitStc[BC2_PORT_FLD_MAX] */
    GT_VOID_PTR prv_dxChBcat2B0_FldInitStcArr;

    /** Pointer to resources configurations for Bobcat2
     *  Actual DB is
     *  CPSS_DXCH_BCAT2_PORT_RESOURCE_LIST_STC        prv_bc2_B0_resConfig */
    GT_VOID_PTR prv_bc2_B0_resConfigPtr;

    /** Pizza Arbirer time measure work variables */
    GT_BOOL  pa_TimeTake;
    GT_U32   prv_paTime_ms;
    GT_U32   prv_paTime_us;

    /** Pizza Arbirer stored Slice Id*/
    CPSS_DXCH_PIZZA_SLICE_ID storedSliceId;

    /** work variables */
    GT_FLOAT64            txQPort64Bandwidth;
    GT_U32 prvCpssDxChPortPizzaArbiterBWTxFifoChannelFactorArr[CPSS_PORT_PA_BW_MODE_LAST_E];

    /** prvCpssDxChPortDynamicPAUnitDrv.c variables */
    PRV_CPSS_DXCH_BC2_PA_UNIT_STC   prv_unitsDescrAll_bc2     [CPSS_DXCH_PA_UNIT_MAX_E];
    PRV_CPSS_DXCH_BC2_PA_UNIT_STC   prv_unitsDescrAll_bobk    [CPSS_DXCH_PA_UNIT_MAX_E];
    PRV_CPSS_DXCH_BC2_PA_UNIT_STC   prv_unitsDescrAll_aldrin  [CPSS_DXCH_PA_UNIT_MAX_E];
    PRV_CPSS_DXCH_BC2_PA_UNIT_STC   prv_unitsDescrAll_bc3     [CPSS_DXCH_PA_UNIT_MAX_E];
    PRV_CPSS_DXCH_BC2_PA_UNIT_STC   prv_unitsDescrAll_aldrin2 [CPSS_DXCH_PA_UNIT_MAX_E];
    PRV_CPSS_DXCH_DEV_X_UNITDEFLIST  prv_dev_unitDefList[8];
    PRV_CPSS_DXCH_BC2_PA_UNITS_DRV_STC prv_paUnitsDrv;

    /** flag to allow optimization of the amount of times that 'PIZZA' configured
       for 'X' ports.
       assumption is that caller knows that no traffic should exists in the device
       while 'optimizing'
    */
    GT_BOOL  prvCpssDxChPortDynamicPizzaArbiterIfConfigure_optimize;

    /** substructures of this pointed by per-defice info */
    PRV_CPSS_DXCH_PA_WS_SET_STC  prv_paWsSet;

    /**  storage for speed conv table prvCpssDxChPortPizzaArbiterProfileStorage.c */
    PRV_CPSS_DXCH_PROFILE_SPEED_CONV_TBL_STORAGE_STC g_usedProfileSpeedConvTableStorage;

    /** arbiter speed tables */
    CPSS_DXCH_PORT_SPEED_TO_SLICE_NUM_STC portSpeed2SliceNumListDefault[16];
    CPSS_DXCH_PORT_PIZZA_PORTGROUP_SPEED_TO_SLICENUM_STC prvPortGroupPortSpeed2SliceNumDefault;
    CPSS_DXCH_PORT_SPEED_TO_SLICE_NUM_STC portSpeed2SliceNumListLionB0_240Mhz_7x10G[8];
    CPSS_DXCH_PORT_PIZZA_PORTGROUP_SPEED_TO_SLICENUM_STC prvPortGroupPortSpeed2SliceNumLionB0_240Mhz_7x10G;

} PRV_CPSS_MAIN_PP_DRV_MOD_DXCH_PORT_DIR_SHARED_PORT_PA_SRC_GLOBAL_DB;

/**
* @struct PRV_CPSS_MAIN_PP_DRV_MOD_DXCH_PORT_DIR_SHARED_PORT_PRIVATE_SRC_GLOBAL_DB
 *
 * @brief  Structure contain global variables that are meant to be shared
 * between processes (when CPSS is linked as shared object)
*    in mainPpDrv module ,port  directory , sources :
*    cpssDxChPortIfModeCfgBcat2Resource.c
*/
typedef struct
{
    /** resource tables data */
    PRV_CPSS_RXDMA_IfWidth_ENT              prvSpeed_2_rxDmaIFWidth_ARR             [CPSS_PORT_SPEED_NA_E];
    PRV_CPSS_TxFIFO_OutGoungBusWidth_ENT    prvSpeed_2_txFifoOutGoungBusWidth_ARR   [CPSS_PORT_SPEED_NA_E];
    GT_U32                                  prvSpeed_2_ethTxFifoShifterThreshold_ARR[CPSS_PORT_SPEED_NA_E];
    PRV_CPSS_EthTxFIFO_OutGoungBusWidth_ENT prvSpeed_2_ethTxFifoOutGoungBusWidth_ARR[CPSS_PORT_SPEED_NA_E];
    /** for test manual configuration */
    PORT_MANUAL_TXQ_CREDIT_STC prv_txqPortManualCredit;
    /** BobK data */
    PRV_CPSS_DXCH_CAELUM_PORT_RESOURCE_DATABASE_STC  portResDb[CPSS_MAX_PORTS_NUM_CNS];
    GT_BOOL g_doNotCheckCredits;
    /** for test manual configuration */
    PORT_MANUAL_TXQ_CREDIT_STC prv_txqPortManualCredit_BobK;
    /** Falcon pizza debug variable */
    GT_U32   debug_falcon_sliceMode;
    /* Hawk Pizza debug variable */
    GT_U32 dpHawkDebugOptions;

} PRV_CPSS_MAIN_PP_DRV_MOD_DXCH_PORT_DIR_SHARED_PORT_PRIVATE_SRC_GLOBAL_DB;

/**
* @struct PRV_CPSS_MAIN_PP_DRV_MOD_DIAG_DIR_SHARED_SRC_GLOBAL_DB
*
* @brief  Structure contain global variables that are meant to be shared
* between processes (when CPSS is linked as shared object)
* in mainPpDrv module ,port directory
*/
typedef struct
{
    /** DB of port pizza adapter and resources related modules */
    PRV_CPSS_MAIN_PP_DRV_MOD_DXCH_PORT_DIR_SHARED_PORT_PA_SRC_GLOBAL_DB portPaSrc;

    /** port/private directory data */
    PRV_CPSS_MAIN_PP_DRV_MOD_DXCH_PORT_DIR_SHARED_PORT_PRIVATE_SRC_GLOBAL_DB portPrvSrc;

} PRV_CPSS_MAIN_PP_DRV_MOD_DXCH_PORT_DIR_SHARED_GLOBAL_DB;

#endif /*CHX_FAMILY*/

/**
* @struct PRV_CPSS_MAIN_PP_DRV_MOD_DIAG_DIR_SHARED_SRC_GLOBAL_DB
 *
 * @brief  Structure contain global variables that are meant to be shared
 * between processes (when CPSS is linked as shared object)
 *   in mainPpDrv module, Diag directory, source : mainPpDrv/src/cpss/generic/diag/prvCpssGenDiag.c
*/
typedef struct
{
    /** add here global shared variables used in source files */
    /** the background data is kept in here  we assume no more than 16*4=64B burst */
    GT_U32   backgroundBuffer[PRV_DIAG_MAX_BURST_SIZE_CNS];
    /** the data is kept in here  we assume no more than 16*4=64B burst */
    GT_U32   writeBuffer[PRV_DIAG_MAX_BURST_SIZE_CNS];
    /** the data is kept in here  we assume no more than 16*4=64B burst */
    GT_U32   readBuffer[PRV_DIAG_MAX_BURST_SIZE_CNS];

    /**********************************************************************
    * The following Mask data is declared to diagnose a specific memory
    * areas such as XCAT3 MAC or VLAN tables where each element contains
    * several words and each word uses certain set of bits only.
    * Mask Array Size is equal to area's element words number
    * and every word in the Mask Array is a mask to corresponding
    * element's word (mask bit n = 1 if bit n is used in the word)
    * In the regular memory case Mask Array contains only one word
    * with all bits = 1 (i.e. FFFFFFFF) and Mask Array Size = 1.
    **********************************************************************/
    GT_U32   memMaskArSize;
    GT_U32   memMaskArray[PRV_DIAG_MAX_MASK_NUM_CNS];

    /** Memory diag burst size in bytes */
    GT_U32   memDiagBurstSize; /* = 4 */
    /** Test in progress */
    GT_BOOL  testInProgress;

} PRV_CPSS_MAIN_PP_DRV_MOD_DIAG_DIR_SHARED_SRC_GLOBAL_DB;

/**
* @struct PRV_CPSS_MAIN_PP_DRV_MOD_DIAG_DIR_SHARED_GLOBAL_DB
*
* @brief  Structure contain global variables that are meant to be shared
* between processes (when CPSS is linked as shared object)
* in mainPpDrv module, Diag directory
*/
typedef struct
{
    /** Data of prvCpssGenDiag.c */
    PRV_CPSS_MAIN_PP_DRV_MOD_DIAG_DIR_SHARED_SRC_GLOBAL_DB genDiagSrc;

} PRV_CPSS_MAIN_PP_DRV_MOD_DIAG_DIR_SHARED_GLOBAL_DB;

#ifdef CHX_FAMILY
/**
* @struct PRV_CPSS_MAIN_PP_DRV_MOD_PTP_DIR_SHARED_PTP_SRC_GLOBAL_DB
 *
 * @brief  Structure contain global variables that are meant to be shared
 * between processes (when CPSS is linked as shared object)
 *   in mainPpDrv module ,ptp  directory , source : cpssDxChPtp.c
*/
typedef struct
{
    /*add here global shared variables used in source files*/
    GT_BOOL                             debugInstanceEnabled;
    PRV_CPSS_DXCH_PTP_TAI_ITERATOR_STC  debugtaiIterator;

} PRV_CPSS_MAIN_PP_DRV_MOD_PTP_DIR_SHARED_PTP_SRC_GLOBAL_DB;

/**
* @struct PRV_CPSS_MAIN_PP_DRV_MOD_PTP_DIR_SHARED_GLOBAL_DB
*
* @brief  Structure contain global variables that are meant to be shared
* between processes (when CPSS is linked as shared object)
* in mainPpDrv module ,ptp directory
*/
typedef struct
{
    /*add here source files*/
    PRV_CPSS_MAIN_PP_DRV_MOD_PTP_DIR_SHARED_PTP_SRC_GLOBAL_DB ptpSrc;

} PRV_CPSS_MAIN_PP_DRV_MOD_PTP_DIR_SHARED_GLOBAL_DB;

/**
* @struct PRV_CPSS_MAIN_PP_DRV_MOD_IDEBUG_DIR_SHARED_IDEBUG_SRC_GLOBAL_DB
 *
 * @brief  Structure contain global variables that are meant to be shared
 * between processes (when CPSS is linked as shared object)
 *   in mainPpDrv module ,idebug  directory , source : prvCpssDxChIdebug.c
*/
typedef struct
{
    PRV_CPSS_DXCH_IDEBUG_DB_STC idebugDB;

} PRV_CPSS_MAIN_PP_DRV_MOD_IDEBUG_DIR_SHARED_IDEBUG_SRC_GLOBAL_DB;

/**
* @struct PRV_CPSS_MAIN_PP_DRV_MOD_IDEBUG_DIR_SHARED_PACKET_ANALYZER_SRC_GLOBAL_DB
 *
 * @brief  Structure contain global variables that are meant to be shared
 * between processes (when CPSS is linked as shared object)
 *   in mainPpDrv module ,idebug  directory , source : prvCpssDxChPacketAnalyzer.c
*/
typedef struct
{
    PRV_CPSS_DXCH_PACKET_ANALYZER_MNG_PTR paMngDB[PRV_CPSS_DXCH_PACKET_ANALYZER_MAX_MANAGERS_NUM_CNS + 1];

} PRV_CPSS_MAIN_PP_DRV_MOD_IDEBUG_DIR_SHARED_PACKET_ANALYZER_SRC_GLOBAL_DB;



/**
* @struct PRV_CPSS_MAIN_PP_DRV_MOD_IDEBUG_DIR_SHARED_GLOBAL_DB
*
* @brief  Structure contain global variables that are meant to be shared
* between processes (when CPSS is linked as shared object)
* in mainPpDrv module ,idebug directory
*/
typedef struct
{
    PRV_CPSS_MAIN_PP_DRV_MOD_IDEBUG_DIR_SHARED_IDEBUG_SRC_GLOBAL_DB             idebugSrc;
    PRV_CPSS_MAIN_PP_DRV_MOD_IDEBUG_DIR_SHARED_PACKET_ANALYZER_SRC_GLOBAL_DB    packetAnalyzerSrc;

} PRV_CPSS_MAIN_PP_DRV_MOD_IDEBUG_DIR_SHARED_GLOBAL_DB;

/**
* @struct
*         PRV_CPSS_MAIN_PP_DRV_MOD_IP_LPM_DIR_SHARED_IP_LPM_HW_GLOBAL_DB
 *
 * @brief  Structure contain global variables that are meant to be shared
 * between processes (when CPSS is linked as shared object)
*    in mainPpDrv module ,lpm  directory , source : prvCpssDxChLpmHw.c
*/
typedef struct
{
    /* add here global shared variables used in source files */
    int k; /* offset inside word */
    GT_U32 *tempGonArrayPtr;

} PRV_CPSS_MAIN_PP_DRV_MOD_IP_LPM_DIR_SHARED_IP_LPM_HW_GLOBAL_DB;

/**
* @struct
*         PRV_CPSS_MAIN_PP_DRV_MOD_IP_LPM_DIR_SHARED_IP_LPM_SRC_GLOBAL_DB
 *
 * @brief  Structure contain global variables that are meant to be shared
 * between processes (when CPSS is linked as shared object)
 *   in mainPpDrv module ,ipLpm  directory , source : cpssDxChIpLpm.c
*/
typedef struct
{
    /*add here global shared variables used in source files*/
    GT_VOID *lpmDbSL;

} PRV_CPSS_MAIN_PP_DRV_MOD_IP_LPM_DIR_SHARED_IP_LPM_SRC_GLOBAL_DB;

/**
* @struct
*         PRV_CPSS_MAIN_PP_DRV_MOD_IP_LPM_DIR_SHARED_IP_LPM_RAM_SRC_GLOBAL_DB
*
* @brief  Structure contain global variables that are meant to be shared
* between processes (when CPSS is linked as shared object)
*   in mainPpDrv module ,ipLpm  directory , source : prvCpssDxChIpLpmRamMng.c
*/
typedef struct
{
    /*add here global shared variables used in prvCpssDxChLpmRamMngEntryDelete*/
    PRV_CPSS_DXCH_LPM_RAM_POINTER_SHADOW_UNT       *lpmPtrArray[MAX_LPM_LEVELS_CNS];
    PRV_CPSS_DXCH_LPM_RAM_RANGE_SHADOW_STC         *pRangeArray[MAX_LPM_LEVELS_CNS];
    PRV_CPSS_DXCH_LPM_RAM_RANGE_SHADOW_STC         *pPrvRangeArray[MAX_LPM_LEVELS_CNS];
    CPSS_DXCH_LPM_NEXT_POINTER_TYPE_ENT            *bucketPtrTypeArray[MAX_LPM_LEVELS_CNS];

    /*add here global shared variables used in prvCpssDxChLpmRamMngInsert */
    GT_BOOL                                         resizeBucket[MAX_LPM_LEVELS_CNS];
    GT_BOOL                                         doHwUpdate[MAX_LPM_LEVELS_CNS];
    PRV_CPSS_DXCH_LPM_RAM_BUCKET_SHADOW_STC         *bucketPtrArry[MAX_LPM_LEVELS_CNS];
    CPSS_DXCH_LPM_NEXT_POINTER_TYPE_ENT             *pointerTypePtrArray[MAX_LPM_LEVELS_CNS];
    PRV_CPSS_DXCH_LPM_RAM_RANGE_SHADOW_STC          *pRangeArr[MAX_LPM_LEVELS_CNS];

    /*add here global shared variables used in updateMirrorBucket */
    CPSS_DXCH_LPM_NODE_RANGE_SELECTION_UNT          rangeSelectionSec;
    CPSS_DXCH_LPM_NODE_NEXT_POINTER_STC             nextPointersArray[256];

    /*add here global shared variables used in prvCpssDxChLpmRamMngShrinkHwWriteAndPointersUpdatePhase1Phase2 */
    CPSS_DXCH_LPM_NODE_RANGE_SELECTION_UNT          rangeSelectionSec_1;
    CPSS_DXCH_LPM_NODE_NEXT_POINTER_STC             nextPointersArray_1[256];


} PRV_CPSS_MAIN_PP_DRV_MOD_IP_LPM_DIR_SHARED_IP_LPM_RAM_SRC_GLOBAL_DB;

/**
* @struct
*         PRV_CPSS_MAIN_PP_DRV_MOD_IP_LPM_DIR_SHARED_SIP6_IP_LPM_RAM_SRC_GLOBAL_DB
*
* @brief  Structure contain global variables that are meant to be shared
* between processes (when CPSS is linked as shared object)
*   in mainPpDrv module ,ipLpm  directory , source : prvCpssDxChSip6IpLpmRamMng.c
*/
typedef struct
{
    /*add here global shared variables used in prvCpssDxChLpmSip6RamMngEntryDelete*/
    PRV_CPSS_DXCH_LPM_RAM_POINTER_SHADOW_UNT       *lpmPtrArray[MAX_LPM_LEVELS_CNS];
    PRV_CPSS_DXCH_LPM_RAM_RANGE_SHADOW_STC         *pRangeArray[MAX_LPM_LEVELS_CNS];
    PRV_CPSS_DXCH_LPM_RAM_RANGE_SHADOW_STC         *pPrvRangeArray[MAX_LPM_LEVELS_CNS];
    CPSS_DXCH_LPM_NEXT_POINTER_TYPE_ENT            *bucketPtrTypeArray[MAX_LPM_LEVELS_CNS];

    /*add here global shared variables used in prvCpssDxChLpmSip6RamMngInsert */
    PRV_CPSS_DXCH_LPM_RAM_RANGE_SHADOW_STC          *pRangeArr[MAX_LPM_LEVELS_CNS];
    LPM_ROOT_BUCKET_UPDATE_ENT                      rootBucketUpdateState[MAX_LPM_LEVELS_CNS];
    GT_BOOL                                         resizeGroup[MAX_LPM_LEVELS_CNS];
    GT_BOOL                                         doHwUpdate[MAX_LPM_LEVELS_CNS];
    GT_BOOL                                         bitVectorCompConvertion[MAX_LPM_LEVELS_CNS];
    GT_BOOL                                         newRangeIsAdded[MAX_LPM_LEVELS_CNS];
    GT_BOOL                                         overriteIsDone[MAX_LPM_LEVELS_CNS];
    PRV_CPSS_DXCH_LPM_RAM_BUCKET_SHADOW_STC         *bucketPtrArry[MAX_LPM_LEVELS_CNS];
    CPSS_DXCH_LPM_NEXT_POINTER_TYPE_ENT             *pointerTypePtrArray[MAX_LPM_LEVELS_CNS];
    LPM_RAM_AFFECTED_BV_LINES_STC                   pAffectedBvLineArray[MAX_LPM_LEVELS_CNS];

    /*add here global shared variables used in updateMirrorGroupOfNodes*/
    PRV_CPSS_DXCH_LPM_GROUP_OF_NODES_RANGE_STC      groupOfNodes_1[PRV_CPSS_DXCH_LPM_RAM_BIT_VECTOR_LINES_NUMBER_CNS];

    /*add here global shared variables used in lpmFalconUpdateRangeInHw*/
    PRV_CPSS_DXCH_LPM_GROUP_OF_NODES_RANGE_STC      groupOfNodes_2;

    /*add here global shared variables used in updateHwRangeDataAndGonPtr*/
    PRV_CPSS_DXCH_LPM_GROUP_OF_NODES_RANGE_STC      groupOfNodes_3[PRV_CPSS_DXCH_LPM_RAM_BIT_VECTOR_LINES_NUMBER_CNS];

    /*add here global shared variables used in prvCpssDxChLpmSip6RamMngRootBucketCreate */
    PRV_CPSS_DXCH_LPM_GROUP_OF_NODES_RANGE_STC      rootGroupOfNodes;

} PRV_CPSS_MAIN_PP_DRV_MOD_IP_LPM_DIR_SHARED_SIP6_IP_LPM_RAM_SRC_GLOBAL_DB;

/**
* @struct
*         PRV_CPSS_MAIN_PP_DRV_MOD_IP_LPM_DIR_SHARED_SIP6_IP_LPM_RAM_DEFRAG_SRC_GLOBAL_DB
*
* @brief  Structure contain global variables that are meant to be shared
* between processes (when CPSS is linked as shared object)
*   in mainPpDrv module ,ipLpm  directory , source : prvCpssDxChSip6LpmRamMngDefrag.c
*/
typedef struct
{
    /*add here global shared variables used in prvCpssDxChLpmSip6RamMngShrinkHwWriteAndPointersUpdatePhase1Phase2*/
    PRV_CPSS_DXCH_LPM_GROUP_OF_NODES_RANGE_STC   tempGroupOfNodes[PRV_CPSS_DXCH_LPM_RAM_BIT_VECTOR_LINES_NUMBER_CNS];
    PRV_CPSS_DXCH_LPM_GROUP_OF_NODES_RANGE_STC   rootGroupOfNodes;

} PRV_CPSS_MAIN_PP_DRV_MOD_IP_LPM_DIR_SHARED_SIP6_IP_LPM_RAM_DEFRAG_SRC_GLOBAL_DB;

/**
* @struct
*         PRV_CPSS_MAIN_PP_DRV_MOD_IP_LPM_DIR_SHARED_LPM_RAM_SRC_GLOBAL_DB
*
* @brief  Structure contain global variables that are meant to be shared
* between processes (when CPSS is linked as shared object)
*   in mainPpDrv module ,ipLpm  directory , source : prvCpssDxChLpmRam.c
*/
typedef struct
{
    /*add here global shared variables used in prvCpssDxChLpmRamSyncSwHwForHaCheckPerVrIdAndProtocolSip5_Sync*/
    PRV_CPSS_DXCH_LPM_RAM_BUCKET_SHADOW_STC  *bucketPtrArry[MAX_LPM_LEVELS_CNS*2];
    CPSS_DXCH_LPM_NEXT_POINTER_TYPE_ENT      *pointerTypePtrArray[MAX_LPM_LEVELS_CNS*2];
    PRV_CPSS_DXCH_LPM_RAM_RANGE_SHADOW_STC   *pRangeArray[MAX_LPM_LEVELS_CNS*2];
    GT_U32                                   hwBucketsAddressArray[MAX_LPM_LEVELS_CNS*2];/* hold the addresses of the buckets,
                                                                                            it will be dynamicaly updated*/
    GT_U32                                   hwBucketsNumberOfRangesArray[MAX_LPM_LEVELS_CNS*2];/* hold the number of ranges
                                                                                                   of the buckets, it will be
                                                                                                   dynamicaly updated*/
    GT_U32                                   hwBucketsNmberOfRangesHandledArray[MAX_LPM_LEVELS_CNS*2];/* hold the number of
                                                                                                        ranges already handled,
                                                                                                        it will be dynamicaly updated*/
    /* Create 2 arrays of size 32 (max size according to ipv6 MC that have
       16 octets for Group and 16 for Source). This array will hold,
       in any given moment, the total number of ranges in the bucket and a
       counter that will be incremented each time a range is handled.*/
    PRV_CPSS_DXCH_LPM_RANGES_STC             hwMultiRangesArr[MAX_LPM_LEVELS_CNS*2];
    PRV_CPSS_DXCH_LPM_BUCKET_DATA_STC        hwMultiBucketsDataArr[MAX_LPM_LEVELS_CNS*2];
    PRV_CPSS_DXCH_LPM_RANGES_STC             hwRanges;
    PRV_CPSS_DXCH_LPM_BUCKET_DATA_STC        hwBucketDataArr;
    GT_U32                                   duplicateBucketData[PRV_CPSS_DXCH_LPM_RAM_MAX_SIZE_OF_BUCKET_IN_LPM_LINES_CNS];

    /*add here global shared variables used in prvCpssDxChLpmRamSyncSwHwForHaCheckPerVrIdAndProtocolSip5_Clean*/
    PRV_CPSS_DXCH_LPM_RAM_BUCKET_SHADOW_STC  *bucketPtrArr[MAX_LPM_LEVELS_CNS*2];
    CPSS_DXCH_LPM_NEXT_POINTER_TYPE_ENT      *pointerTypePtrArr[MAX_LPM_LEVELS_CNS*2];
    PRV_CPSS_DXCH_LPM_RAM_RANGE_SHADOW_STC   *pRangeArr[MAX_LPM_LEVELS_CNS*2];

    /*add here global shared variables used in prvCpssDxChLpmRamDbDevListAdd*/
    GT_U8                                shadowDevList[PRV_CPSS_MAX_PP_DEVICES_CNS];

} PRV_CPSS_MAIN_PP_DRV_MOD_IP_LPM_DIR_SHARED_LPM_RAM_SRC_GLOBAL_DB;

/**
* @struct
*         PRV_CPSS_MAIN_PP_DRV_MOD_IP_LPM_DIR_SHARED_LPM_RAM_DBG_SRC_GLOBAL_DB
*
* @brief  Structure contain global variables that are meant to be shared
* between processes (when CPSS is linked as shared object)
*   in mainPpDrv module ,ipLpm  directory , source : prvCpssDxChLpmRamDbg.c
*/
typedef struct
{
    GT_U8 baseAddr[PRV_CPSS_DXCH_LPM_MAX_NUM_OF_OCTETS_FOR_ADDRESS_CNS];
    GT_U8 baseGroupAddr[PRV_CPSS_DXCH_LPM_MAX_NUM_OF_OCTETS_FOR_ADDRESS_CNS];
    GT_U32 grpPrefix;
    GT_BOOL InSrcTree;
    GT_U8 *baseGroupAddrPtr;
    GT_U32 numOfErrors;
} PRV_CPSS_MAIN_PP_DRV_MOD_IP_LPM_DIR_SHARED_LPM_RAM_DBG_SRC_GLOBAL_DB;

/**
* @struct
*         PRV_CPSS_MAIN_PP_DRV_MOD_IP_LPM_DIR_SHARED_LPM_TCAM_SRC_GLOBAL_DB
*
* @brief  Structure contain global variables that are meant to be shared
* between processes (when CPSS is linked as shared object)
*   in mainPpDrv module ,ipLpm  directory , source : prvCpssDxChLpmTcam.c
*/
typedef struct
{
    /* Pointers to dummy trie nodes to support policy based routing MC defaults */
    GT_U32                                    pbrDefaultMcTrieNodePtrArraySize;
    PRV_CPSS_DXCH_LPM_TCAM_PAT_TRIE_NODE_STC  **pbrDefaultMcTrieNodePtrArray;

} PRV_CPSS_MAIN_PP_DRV_MOD_IP_LPM_DIR_SHARED_LPM_TCAM_SRC_GLOBAL_DB;

/**
* @struct
*         PRV_CPSS_MAIN_PP_DRV_MOD_IP_LPM_DIR_SHARED_LPM_TCAM_DBG_SRC_GLOBAL_DB
*
* @brief  Structure contain global variables that are meant to be shared
* between processes (when CPSS is linked as shared object)
*   in mainPpDrv module ,ipLpm  directory , source : prvCpssDxChLpmTcamDbg.c
*/
typedef struct
{
   GT_BOOL printMcSrcTreeInProgress;
   GT_U32  mcPrintSpaceNum;

} PRV_CPSS_MAIN_PP_DRV_MOD_IP_LPM_DIR_SHARED_LPM_TCAM_DBG_SRC_GLOBAL_DB;

/**
* @struct
*         PRV_CPSS_MAIN_PP_DRV_MOD_IP_LPM_DIR_SHARED_LPM_TCAM_COMMON_SRC_GLOBAL_DB
*
* @brief  Structure contain global variables that are meant to be shared
* between processes (when CPSS is linked as shared object)
*   in mainPpDrv module ,ipLpm  directory , source : prvCpssDxChLpmTcamCommon.c
*/
typedef struct
{
    GT_U8   cpssDxChDefaultMcIpAddr[2][16];
    GT_U32  cpssDxChDefaultMcIpPrefixLen[2];
    GT_U32  tcamRows;

} PRV_CPSS_MAIN_PP_DRV_MOD_IP_LPM_DIR_SHARED_LPM_TCAM_COMMON_SRC_GLOBAL_DB;

/**
* @struct
*         PRV_CPSS_MAIN_PP_DRV_MOD_IP_LPM_DIR_SHARED_LPM_TCAM_PAT_TRIE_SRC_GLOBAL_DB
*
* @brief  Structure contain global variables that are meant to be shared
* between processes (when CPSS is linked as shared object)
*   in mainPpDrv module ,ipLpm  directory , source : prvCpssDxChLpmTcamPatTrie.c
*/
typedef struct
{
    GT_U32 dbgTcamWrites;

} PRV_CPSS_MAIN_PP_DRV_MOD_IP_LPM_DIR_SHARED_LPM_TCAM_PAT_TRIE_SRC_GLOBAL_DB;


/**
* @struct PRV_CPSS_MAIN_PP_DRV_MOD_IP_LPM_DIR_SHARED_GLOBAL_DB
*
* @brief  Structure contain global variables that are meant to be shared
* between processes (when CPSS is linked as shared object)
* in mainPpDrv module ,ipLpm directory
*/
typedef struct
{
    /*add here source files*/
    PRV_CPSS_MAIN_PP_DRV_MOD_IP_LPM_DIR_SHARED_IP_LPM_HW_GLOBAL_DB                  ipLpmHwSrc;
    PRV_CPSS_MAIN_PP_DRV_MOD_IP_LPM_DIR_SHARED_IP_LPM_SRC_GLOBAL_DB                 ipLpmSrc;
    PRV_CPSS_MAIN_PP_DRV_MOD_IP_LPM_DIR_SHARED_IP_LPM_RAM_SRC_GLOBAL_DB             ipLpmRamSrc;
    PRV_CPSS_MAIN_PP_DRV_MOD_IP_LPM_DIR_SHARED_SIP6_IP_LPM_RAM_SRC_GLOBAL_DB        sip6IpLpmRamSrc;
    PRV_CPSS_MAIN_PP_DRV_MOD_IP_LPM_DIR_SHARED_SIP6_IP_LPM_RAM_DEFRAG_SRC_GLOBAL_DB sip6IpLpmRamDefragSrc;

    PRV_CPSS_MAIN_PP_DRV_MOD_IP_LPM_DIR_SHARED_LPM_RAM_SRC_GLOBAL_DB                lpmRamSrc;
    PRV_CPSS_MAIN_PP_DRV_MOD_IP_LPM_DIR_SHARED_LPM_RAM_DBG_SRC_GLOBAL_DB            lpmRamDbgSrc;
    PRV_CPSS_MAIN_PP_DRV_MOD_IP_LPM_DIR_SHARED_LPM_TCAM_SRC_GLOBAL_DB               lpmTcamSrc;
    PRV_CPSS_MAIN_PP_DRV_MOD_IP_LPM_DIR_SHARED_LPM_TCAM_DBG_SRC_GLOBAL_DB           lpmTcamDbgSrc;
    PRV_CPSS_MAIN_PP_DRV_MOD_IP_LPM_DIR_SHARED_LPM_TCAM_COMMON_SRC_GLOBAL_DB        lpmTcamCommonSrc;
    PRV_CPSS_MAIN_PP_DRV_MOD_IP_LPM_DIR_SHARED_LPM_TCAM_PAT_TRIE_SRC_GLOBAL_DB      lpmTcamPatTrieSrc;

} PRV_CPSS_MAIN_PP_DRV_MOD_IP_LPM_DIR_SHARED_GLOBAL_DB;

/**
* @struct
*         PRV_CPSS_MAIN_PP_DRV_MOD_TXQ_DIR_SHARED_TXQ_DBG_SRC_GLOBAL_DB
 *
 * @brief  Structure contain global variables that are meant to be shared
 * between processes (when CPSS is linked as shared object)
*    in mainPpDrv module ,Port directory , source :
*    cpssDxChPortCtrl.c
                                                              */
typedef struct
{
    /*add here global shared variables used in source files*/

    /* flag to allow print of the 'TXQ num of buffers' when we do operations to wait
       for it to drain */
    GT_U32   debug_sip6_cpssDxChPortTxBufNumberGet;

    GT_U32 debug_sip6_TO_polling_iterations;
    GT_U32 debug_sip6_TO_txq_polling_interval;
    GT_U32 debug_sip6_TO_after_txq_drain;
    GT_U32 debug_sip6_TO_after_mac_disabled;

    GT_U32   old_value;
    PRV_CPSS_TXQ_CIDER_INFO_STC ciderInfo[PRV_CPSS_DXCH_CIDER_INFO_SIZE];
    GT_BOOL ciderInfoInitDone;
} PRV_CPSS_MAIN_PP_DRV_MOD_TXQ_DIR_SHARED_TXQ_DBG_SRC_GLOBAL_DB;

/**
* @struct PRV_CPSS_MAIN_PP_DRV_MOD_FLOW_MANAGER_DIR_SHARED_FLOW_MANAGER_SRC_GLOBAL_DB
 *
 * @brief  Structure contain global variables that are meant to be shared
 * between processes (when CPSS is linked as shared object)
 *   in mainPpDrv module ,flowManager directory , source : cpssDxChFlowManager.c
*/
typedef struct
{
    /*add here global shared variables used in source files*/

    /* flow manager DB to */
    CPSS_DXCH_FLOW_MNG_PTR flowMngDB[PRV_CPSS_DXCH_FLOW_MANAGER_MAX_ID_CNS + 1];

} PRV_CPSS_MAIN_PP_DRV_MOD_FLOW_MANAGER_DIR_SHARED_FLOW_MANAGER_SRC_GLOBAL_DB;

/**
* @struct PRV_CPSS_MAIN_PP_DRV_MOD_TXQ_DBG_DIR_SHARED_GLOBAL_DB
*
* @brief  Structure contain global variables that are meant to be shared
* between processes (when CPSS is linked as shared object)
* in mainPpDrv module ,Port directory
*/
typedef struct
{
    /*add here source files*/
    PRV_CPSS_MAIN_PP_DRV_MOD_TXQ_DIR_SHARED_TXQ_DBG_SRC_GLOBAL_DB txqDbgSrc;

} PRV_CPSS_MAIN_PP_DRV_MOD_TXQ_DBG_DIR_SHARED_GLOBAL_DB;

/**
* @struct PRV_CPSS_MAIN_PP_DRV_MOD_FLOW_MANAGER_DIR_SHARED_GLOBAL_DB
 *
 * @brief  Structure contain global variables that are not meant to be shared
 * between processes (when CPSS is linked as shared object)
 *   in mainPpDrv module ,flowManager directory
*/
typedef struct
{
    /*add here source files*/
    PRV_CPSS_MAIN_PP_DRV_MOD_FLOW_MANAGER_DIR_SHARED_FLOW_MANAGER_SRC_GLOBAL_DB flowMgrSrc;

} PRV_CPSS_MAIN_PP_DRV_MOD_FLOW_MANAGER_DIR_SHARED_GLOBAL_DB;

/**
* @struct PRV_CPSS_MAIN_PP_DRV_MOD_EM_MANAGER_DIR_SHARED_EM_MANAGER_SRC_GLOBAL_DB
 *
 * @brief  Structure contain global variables that are meant to be shared
 * between processes (when CPSS is linked as shared object)
 *   in mainPpDrv module ,exactMatchManager directory , source : cpssDxChExactMatchManager.c
*/
typedef struct
{
    /*add here global shared variables used in source files*/
    GT_BOOL debugPrint;

} PRV_CPSS_MAIN_PP_DRV_MOD_EM_MANAGER_DIR_SHARED_EM_MANAGER_SRC_GLOBAL_DB;

/**
* @struct PRV_CPSS_MAIN_PP_DRV_MOD_EM_MANAGER_DIR_SHARED_EM_MANAGER_DEBUG_SRC_GLOBAL_DB
 *
 * @brief  Structure contain global variables that are meant to be shared
 * between processes (when CPSS is linked as shared object)
 *   in mainPpDrv module ,exactMatchManager directory , source : prvCpssDxChExactMatchManager_debug.c
*/
typedef struct
{
    /*add here global shared variables used in source files*/
    PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_INSTANCE_INFO_STC  *globalExactMatchManagerPtr;

    CPSS_OS_MALLOC_FUNC            prvCpssDxChExactMatchManagerDebug_cpssOsMalloc;
    CPSS_OS_FREE_FUNC              prvCpssDxChExactMatchManagerDebug_cpssOsFree;

} PRV_CPSS_MAIN_PP_DRV_MOD_EM_MANAGER_DIR_SHARED_EM_MANAGER_DEBUG_SRC_GLOBAL_DB;

/**
* @struct PRV_CPSS_MAIN_PP_DRV_MOD_EM_MANAGER_DIR_SHARED_EM_MANAGER_DB_SRC_GLOBAL_DB
 *
 * @brief  Structure contain global variables that are meant to be shared
 * between processes (when CPSS is linked as shared object)
 *   in mainPpDrv module ,exactMatchManager directory , source : prvCpssDxChExactMatchManager_db.c
*/
typedef struct
{
    /*add here global shared variables used in source files*/

    /* the Exact Match manager array. hold pointers , each to 'created' Exact Match manager (and not 'deleted') */
    PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_INSTANCE_INFO_STC  *prvCpssDxChExactMatchManagerDbArr[CPSS_DXCH_EXACT_MATCH_MANAGER_MAX_INSTANCE_NUM_CNS];

    /* bitmap of devices registered to ANY of the manager */
    /* NOTE : a device can't be registered to more than single manager with the same portGroup !!! */
    GT_U32              prvCpssDxChExactMatchManagerDbGlobalDevsBmpArr[NUM_WORDS_DEVS_BMP_CNS];
    GT_PORT_GROUPS_BMP  prvCpssDxChExactMatchManagerDbGlobalPortGroupBmpArr[PRV_CPSS_MAX_PP_DEVICES_CNS];

} PRV_CPSS_MAIN_PP_DRV_MOD_EM_MANAGER_DIR_SHARED_EM_MANAGER_DB_SRC_GLOBAL_DB;

/**
* @struct PRV_CPSS_MAIN_PP_DRV_MOD_DXCH_EM_MANAGER_DIR_SHARED_GLOBAL_DB
 *
 * @brief  Structure contain global variables that are not meant to be shared
 * between processes (when CPSS is linked as shared object)
 *   in mainPpDrv module ,exactMatchManager directory
*/
typedef struct
{
    /*add here source files*/
    PRV_CPSS_MAIN_PP_DRV_MOD_EM_MANAGER_DIR_SHARED_EM_MANAGER_DEBUG_SRC_GLOBAL_DB emMgrDbgSrc;
    PRV_CPSS_MAIN_PP_DRV_MOD_EM_MANAGER_DIR_SHARED_EM_MANAGER_DB_SRC_GLOBAL_DB    emMgrDbSrc;
    PRV_CPSS_MAIN_PP_DRV_MOD_EM_MANAGER_DIR_SHARED_EM_MANAGER_SRC_GLOBAL_DB       emMgrSrc;


} PRV_CPSS_MAIN_PP_DRV_MOD_DXCH_EM_MANAGER_DIR_SHARED_GLOBAL_DB;

/**
* @struct PRV_CPSS_MAIN_PP_DRV_MOD_CUCKOO_DIR_SHARED_CUCKOO_SRC_GLOBAL_DB
 *
 * @brief  Structure contain global variables that are meant to be shared
 * between processes (when CPSS is linked as shared object)
 *   in mainPpDrv module ,cuckoo  directory , source : prvCpssDxChCuckoo.c
*/
typedef struct
{
    /*add here global shared variables used in source files*/
    GT_BOOL                             debug1Print;
    GT_BOOL                             debug2Print;

    /* cuckoo DB */
    PRV_CPSS_DXCH_CUCKOO_DB_STC *cuckooDB[PRV_CPSS_DXCH_CUCKOO_CLIENTS_LAST_E][PRV_CPSS_DXCH_CUCKOO_MAX_MANAGER_NUM_CNS];

} PRV_CPSS_MAIN_PP_DRV_MOD_CUCKOO_DIR_SHARED_CUCKOO_SRC_GLOBAL_DB;

/**
* @struct PRV_CPSS_MAIN_PP_DRV_MOD_CUCKOO_DIR_SHARED_GLOBAL_DB
*
* @brief  Structure contain global variables that are meant to be shared
* between processes (when CPSS is linked as shared object)
* in mainPpDrv module ,cuckoo directory
*/
typedef struct
{
    /*add here source files*/
    PRV_CPSS_MAIN_PP_DRV_MOD_CUCKOO_DIR_SHARED_CUCKOO_SRC_GLOBAL_DB cuckooSrc;

} PRV_CPSS_MAIN_PP_DRV_MOD_CUCKOO_DIR_SHARED_GLOBAL_DB;

/**
* @struct PRV_CPSS_MAIN_PP_DRV_MOD_TTI_DIR_SHARED_TTI_SRC_GLOBAL_DB
 *
 * @brief  Structure contain global variables that are meant to be shared
 * between processes (when CPSS is linked as shared object)
 *   in mainPpDrv module ,tti  directory , source : cpssDxChTti.c
*/
typedef struct
{
    /*add here global shared variables used in source files*/
    PRV_CPSS_ENTRY_FORMAT_TABLE_STC ttiLegacyKeyFieldsFormat[TTI_LEGACY_KEY_FIELDS___LAST_VALUE___E];
    PRV_CPSS_ENTRY_FORMAT_TABLE_STC ttiMetaDataFieldsFormat[TTI_META_DATA_FIELDS___LAST_VALUE___E];

    /* the index (byte index) in the legacy key of Bobcat2 and above that should represent the mac SA/DA */
    GT_U32   sip5legacyMacSaDaIndexInKey;

    /* In order to support backward compatibility for numberOfLabels*/
    /* only "full don't care" (mask 0) and  "full care" (mask 3) are supported.*/
    /* The numberOfLabels pattern and mask are translated to corresponding s-bits.*/
    /* The pattern s-bit decisions are needed to make correct s-bit masks.*/
    GT_U32 patternS0;
    GT_U32 patternS1;
    GT_U32 patternS2;

} PRV_CPSS_MAIN_PP_DRV_MOD_TTI_DIR_SHARED_TTI_SRC_GLOBAL_DB;

/**
* @struct PRV_CPSS_MAIN_PP_DRV_MOD_DXCH_TTI_DIR_SHARED_GLOBAL_DB
*
* @brief  Structure contain global variables that are meant to be shared
* between processes (when CPSS is linked as shared object)
* in mainPpDrv module ,tti directory
*/
typedef struct
{
    /*add here source files*/
    PRV_CPSS_MAIN_PP_DRV_MOD_TTI_DIR_SHARED_TTI_SRC_GLOBAL_DB ttiSrc;

} PRV_CPSS_MAIN_PP_DRV_MOD_DXCH_TTI_DIR_SHARED_GLOBAL_DB;

/**
 * @struct PRV_CPSS_FORBIDDEN_ADDRESS_RANGE_STC
 *
 * @brief  Structure for register addresses ranges bypassed on bring-up
*/
typedef struct
{
    /** register addresses range      */
    /** if mask == 0 entry is skipped */
    GT_U32      baseAddr;
    GT_U32      mask;
} PRV_CPSS_FORBIDDEN_ADDRESS_RANGE_STC;


/**
 * @struct PRV_CPSS_MAIN_PP_DRV_MOD_DXCH_CPSS_HW_INIT_DIR_SHARED_REG_SRC_GLOBAL_DB
 *
 * @brief  Structure contain global variables that are meant to be shared
 * between processes (when CPSS is linked as shared object)
 *   in mainPpDrv module ,DxCh cpssHwInit directory, RegAddr, RegAddrVer1 files
*/
typedef struct
{

    /** prvCpssDxChHwRegAddr.c */

    /** Units base address is derived from the 9 MSbits.                                */
    /** xCat3 registers and tables DB initialization is derived from xCat ones with     */
    /** respective modification to the units base address.                              */
    /** In fact all the necessary information for the conversion is found in            */
    /** prvCpssDxChUnitsBaseAddrXcat3AndXcat (defined here above). Doing each           */
    /** conversion directly based on prvCpssDxChUnitsBaseAddrXcat3AndXcat will          */
    /** prvCpssDxChUnitsBaseAddrXcat3AndXcat defined in prvCpssDxChHwRegAddr.c          */
    /** required address with mask matching and then some additional bit operations.    */
    /** In order to make the conversion more efficient and eliminating the search       */
    /** for matching activity(done in O(n)), an auxiliary array is defined (the one     */
    /** here below) which enables finding the new base address in O(1) -                */
    /** the 9 MSbits are the reason for the array size of 512 entries (2^9 = 512).      */
    GT_U32 prvCpssDxChBaseCovertAddrXcatToXcat3[BIT_9];

    /** prvCpssDxChHwRegAddrVer1.c */

    /* debug trace variables */
    GT_U32 trace_ADDRESS_NOT_SUPPORTED_MAC;
    GT_U32 debug_onEmulator_allow_anyRegisterAccess;

    /* Unit-Id arrays - filled in runtime */
    PRV_CPSS_DXCH_UNIT_ENT prvDxChBobcat2UnitIdPer8MSBitsBaseAddrArr[256];
    PRV_CPSS_DXCH_UNIT_ENT prvDxChBobcat3UnitIdPer8MSBitsBaseAddrArr[256];
    PRV_CPSS_DXCH_UNIT_ENT prvDxChAldrinUnitIdPer8MSBitsBaseAddrArr[256];
    PRV_CPSS_DXCH_UNIT_ENT prvDxChAldrin2UnitIdPer8MSBitsBaseAddrArr[256];

    /** register addresses ranges bypassed on bring-up */
    GT_U32   use_forbiddenRangeDb;
    PRV_CPSS_FORBIDDEN_ADDRESS_RANGE_STC forbiddenRangeDb[16];

} PRV_CPSS_MAIN_PP_DRV_MOD_DXCH_CPSS_HW_INIT_DIR_SHARED_REG_SRC_GLOBAL_DB;


/**
 * @struct PRV_CPSS_MAIN_PP_DRV_MOD_DXCH_CPSS_HW_INIT_DIR_SHARED_REG_V1_SRC_GLOBAL_DB
 *
 * @brief  Structure contain global variables that are meant to be shared
 * between processes (when CPSS is linked as shared object)
 *   in mainPpDrv module ,DxCh cpssHwInit directory, RegAddrVer1_ppDb file
*/
typedef struct
{
    /** is init done for unit base address DB */
    GT_BOOL                                              unitBaseDbInitDone;

    /** DB that holds base address related pointers and flags */
    PRV_CPSS_DXCH_UNITID_UNITBASEADDR_x_TABLE2FILL_STC    prv_UnitBasedArrTbl[PRV_CPSS_DXCH_UNITID_DB_FAMILY__LAST__E];

    /** Bobcat2 unit base address array */
    GT_U32 prvDxChBobcat2UnitsBaseAddr[PRV_CPSS_DXCH_UNIT_LAST_E + 1];
    /** Bobk unit base address array */
    GT_U32 prvDxChBobkUnitsBaseAddr   [PRV_CPSS_DXCH_UNIT_LAST_E + 1];
    /** Bobcat3 unit base address array */
    GT_U32 prvDxChBobcat3UnitsBaseAddr[PRV_CPSS_DXCH_UNIT_LAST_E + 1];
    /** Aldrin unit base address array */
    GT_U32 prvDxChAldrinUnitsBaseAddr [PRV_CPSS_DXCH_UNIT_LAST_E + 1];
    /** Aldrin2 unit base address array */
    GT_U32 prvDxChAldrin2UnitsBaseAddr[PRV_CPSS_DXCH_UNIT_LAST_E + 1];
    /** Falcon unit base address array */
    GT_U32 prvDxChFalconUnitsBaseAddr [PRV_CPSS_DXCH_UNIT_LAST_E + 1];
    /** Hawk   unit base address array */
    GT_U32 prvDxChHawkUnitsBaseAddr   [PRV_CPSS_DXCH_UNIT_LAST_E + 1];
    /** Phoenix   unit base address array */
    GT_U32 prvDxChPhoenixUnitsBaseAddr[PRV_CPSS_DXCH_UNIT_LAST_E + 1];
    /** Harrier   unit base address array */
    GT_U32 prvDxChHarrierUnitsBaseAddr[PRV_CPSS_DXCH_UNIT_LAST_E + 1];
    /** Ironman-L unit base address array */
    GT_U32 prvDxChIronmanLUnitsBaseAddr[PRV_CPSS_DXCH_UNIT_LAST_E + 1];

    /** Falcon unit ID to base address DB */
    PRV_CPSS_DXCH_UNITID_UNITBASEADDR_STC prvDxChFalconUnitsIdUnitBaseAddrArr[FALCON_MAX_UNITS];
    /** AC5P unit ID to base address DB */
    PRV_CPSS_DXCH_UNITID_UNITBASEADDR_STC prvDxChHawkUnitsIdUnitBaseAddrArr[HAWK_MAX_UNITS];
    /** AC5X unit ID to base address DB */
    PRV_CPSS_DXCH_UNITID_UNITBASEADDR_STC prvDxChPhoenixUnitsIdUnitBaseAddrArr[PHOENIX_MAX_UNITS];
    /** Harrier unit ID to base address DB */
    PRV_CPSS_DXCH_UNITID_UNITBASEADDR_STC prvDxChHarrierUnitsIdUnitBaseAddrArr[HARRIER_MAX_UNITS];
    /** IronMan L unit ID to base address DB */
    PRV_CPSS_DXCH_UNITID_UNITBASEADDR_STC prvDxChIronmanLUnitsIdUnitBaseAddrArr[IRONMAN_MAX_UNITS];

    /** getGopPortBaseAddress: previous pipe index */
    GT_U32  prevPipeIndex;
    /** getGopPortBaseAddress: gop memory base address */
    GT_U32  gopBaseAddr;

} PRV_CPSS_MAIN_PP_DRV_MOD_DXCH_CPSS_HW_INIT_DIR_SHARED_REG_V1_SRC_GLOBAL_DB;

/**
 * @struct PRV_CPSS_DXCH_TABLES_INFO_PER_FAMILY_STC
 *
 * @brief  Structure contain tables info : 'tableInfo' and 'directAccessTableInfo'
 *          the info needed per family
 *         to support multiple DXCH family , in parallel , to not change DB of tables
 *         of one family from other family.
 *         part of fix for : CPSS-13946 : CPSS table initialization with multiple
 *                  device Aldrin2 + Falcon failed
*/
typedef struct{
    /** 'tableInfo' of the family */
    PRV_CPSS_DXCH_TABLES_INFO_STC*               tablesInfoArr;
    /** 'directAccessTableInfo' of the family */
    PRV_CPSS_DXCH_TABLES_INFO_DIRECT_UNIT_STC*   directAccessTableInfoArr;
}PRV_CPSS_DXCH_TABLES_INFO_PER_FAMILY_STC;

/**
 * @struct PRV_CPSS_DXCH_TEMP_TABLES_DB_STC
 *
 * @brief  Temporary tables info that needed to support multiple family devices
 *         part of fix for : CPSS-13946 : CPSS table initialization with multiple
 *                  device Aldrin2 + Falcon failed
*/
typedef struct{
    /** tables info for bobcat3 */
    PRV_CPSS_DXCH_TABLES_INFO_PER_FAMILY_STC bobcat3Tables;
    /** tables info for aldrin2 */
    PRV_CPSS_DXCH_TABLES_INFO_PER_FAMILY_STC aldrin2Tables;
    /** tables info for falcon */
    PRV_CPSS_DXCH_TABLES_INFO_PER_FAMILY_STC falconTables;
    /** tables info for hawk */
    PRV_CPSS_DXCH_TABLES_INFO_PER_FAMILY_STC hawkTables;
    /** tables info for phoenix */
    PRV_CPSS_DXCH_TABLES_INFO_PER_FAMILY_STC phoenixTables;
    /** tables info for harrier */
    PRV_CPSS_DXCH_TABLES_INFO_PER_FAMILY_STC harrierTables;
    /** tables info for ironman */
    PRV_CPSS_DXCH_TABLES_INFO_PER_FAMILY_STC ironmanTables;
}PRV_CPSS_DXCH_TEMP_TABLES_DB_STC;

/**
 * @struct PRV_CPSS_MAIN_PP_DRV_MOD_DXCH_CPSS_HW_INIT_DIR_SHARED_TABLES_SRC_GLOBAL_DB
 *
 * @brief  Structure contain global variables that are meant to be shared
 * between processes (when CPSS is linked as shared object)
 *   in mainPpDrv module ,DxCh cpssHwInit directory, prvCpssDxChHwTables.c file
*/
typedef struct
{
    /**@brief - info to support multiple family devices
     *     each family need info about the DB created for other families
     *     as the Init of tables is done incrementally
     *         part of fix for : CPSS-13946 : CPSS table initialization with multiple
     *                  device Aldrin2 + Falcon failed
     */
    PRV_CPSS_DXCH_TEMP_TABLES_DB_STC    tempTablesPerFamilyArr[CPSS_PP_FAMILY_LAST_E];

    /** bit per family in the DB about the tables already initialized */
    GT_U32   tablesDbInitPreFamily[(CPSS_PP_FAMILY_LAST_E+31)/32];

    /** pointer to xCat3, AC5 tables info DB */
    PRV_CPSS_DXCH_TABLES_INFO_STC *dxChXcat3TablesInfoPtr;

    /** pointer to Lion2 tables info DB */
    PRV_CPSS_DXCH_TABLES_INFO_STC *lion2TablesInfoPtr;

    /** pointer to BobCat2 tables info DB */
    PRV_CPSS_DXCH_TABLES_INFO_STC *bobcat2TablesInfoPtr;

    /** pointer to Bobk tables info DB */
    PRV_CPSS_DXCH_TABLES_INFO_STC *bobkTablesInfoPtr;

    /**@brief - pointer to BobCat3 tables info DB
      *     this pointer is not doing malloc/free as it point to :
      *     tempTablesPerFamilyArr[BobCat3].bobcat3Tables.tablesInfoArr
    */
    PRV_CPSS_DXCH_TABLES_INFO_STC *bobcat3TablesInfoPtr;

    /**@brief - pointer to Aldrin2 tables info DB
      *     this pointer is not doing malloc/free as it point to :
      *     tempTablesPerFamilyArr[Aldrin2].aldrin2Tables.tablesInfoArr
    */
    PRV_CPSS_DXCH_TABLES_INFO_STC *aldrin2TablesInfoPtr;

    /**@brief - pointer to Falcon tables info DB
      *     this pointer is not doing malloc/free as it point to :
      *     tempTablesPerFamilyArr[Falcon].falconTables.tablesInfoArr
    */
    PRV_CPSS_DXCH_TABLES_INFO_STC *falconTablesInfoPtr;

    /**@brief - pointer to AC5P tables info DB
      *     this pointer is not doing malloc/free as it point to :
      *     tempTablesPerFamilyArr[AC5P].hawkTables.tablesInfoArr
    */
    PRV_CPSS_DXCH_TABLES_INFO_STC *hawkTablesInfoPtr;

    /**@brief - pointer to AC5X tables info DB
      *     this pointer is not doing malloc/free as it point to :
      *     tempTablesPerFamilyArr[AC5X].phoenixTables.tablesInfoArr
    */
    PRV_CPSS_DXCH_TABLES_INFO_STC *phoenixTablesInfoPtr;

    /**@brief - pointer to Harrier tables info DB
      *     this pointer is not doing malloc/free as it point to :
      *     tempTablesPerFamilyArr[Harrier].harrierTables.tablesInfoArr
    */
    PRV_CPSS_DXCH_TABLES_INFO_STC *harrierTablesInfoPtr;

    /**@brief - pointer to Ironman tables info DB
      *     this pointer is not doing malloc/free as it point to :
      *     tempTablesPerFamilyArr[Ironman].harrierTables.tablesInfoArr
    */
    PRV_CPSS_DXCH_TABLES_INFO_STC *ironman_L_TablesInfoPtr;

    /** init done variables */
   GT_BOOL directAccessBobkTableInfo_initDone;

   /** Bobcat2 direct table information */
   PRV_CPSS_DXCH_TABLES_INFO_DIRECT_UNIT_STC *directAccessBobcat2TableInfoPtr;

   /** Bobcat2B0 direct table override information */
   PRV_CPSS_DXCH_TABLES_INFO_EXT_STC *bobcat2B0TablesInfo_overrideA0Ptr;

   /** BobK direct table information */
   PRV_CPSS_DXCH_TABLES_INFO_DIRECT_UNIT_STC *directAccessBobkTableInfoPtr;

    /**@brief - pointer to Bobcat3 direct tables info DB
      *     this pointer is not doing malloc/free as it point to :
      *     tempTablesPerFamilyArr[Bobcat3].bobcat3Tables.directAccessTableInfoArr
    */
   PRV_CPSS_DXCH_TABLES_INFO_DIRECT_UNIT_STC *directAccessBobcat3TableInfoPtr;

    /**@brief - pointer to Aldrin2 direct tables info DB
      *     this pointer is not doing malloc/free as it point to :
      *     tempTablesPerFamilyArr[Aldrin2].aldrin2Tables.directAccessTableInfoArr
    */
   PRV_CPSS_DXCH_TABLES_INFO_DIRECT_UNIT_STC *aldrin2directAccessTableInfoPtr;

    /**@brief - pointer to Falcon direct tables info DB
      *     this pointer is not doing malloc/free as it point to :
      *     tempTablesPerFamilyArr[Falcon].falconTables.directAccessTableInfoArr
    */
   PRV_CPSS_DXCH_TABLES_INFO_DIRECT_UNIT_STC *falcon_directAccessTableInfoPtr;

    /**@brief - pointer to AC5P direct tables info DB
      *     this pointer is not doing malloc/free as it point to :
      *     tempTablesPerFamilyArr[AC5P].hawkTables.directAccessTableInfoArr
    */
   PRV_CPSS_DXCH_TABLES_INFO_DIRECT_UNIT_STC *hawk_directAccessTableInfoPtr;

    /**@brief - pointer to AC5X direct tables info DB
      *     this pointer is not doing malloc/free as it point to :
      *     tempTablesPerFamilyArr[AC5X].phoenixTables.directAccessTableInfoArr
    */
   PRV_CPSS_DXCH_TABLES_INFO_DIRECT_UNIT_STC *phoenix_directAccessTableInfoPtr;

    /**@brief - pointer to Harrier direct tables info DB
      *     this pointer is not doing malloc/free as it point to :
      *     tempTablesPerFamilyArr[Harrier].harrierTables.directAccessTableInfoArr
    */
   PRV_CPSS_DXCH_TABLES_INFO_DIRECT_UNIT_STC *harrier_directAccessTableInfoPtr;

    /**@brief - pointer to Ironman direct tables info DB
      *     this pointer is not doing malloc/free as it point to :
      *     tempTablesPerFamilyArr[Ironman].ironmanTables.directAccessTableInfoArr
    */
   PRV_CPSS_DXCH_TABLES_INFO_DIRECT_UNIT_STC *ironman_L_directAccessTableInfoPtr;

   /** Object that holds callback function to table HW access */
   CPSS_DXCH_CFG_HW_ACCESS_OBJ_STC prvDxChTableCpssHwAccessObj;

} PRV_CPSS_MAIN_PP_DRV_MOD_DXCH_CPSS_HW_INIT_DIR_SHARED_TABLES_SRC_GLOBAL_DB;


/**
 * @struct PRV_CPSS_MAIN_PP_DRV_MOD_DXCH_CPSS_DXCH_HW_INIT_DIR_SHARED_SRC_GLOBAL_DB
 *
 * @brief  Structure contain global variables that are meant to be shared
 * between processes (when CPSS is linked as shared object)
 *   in mainPpDrv module ,DxCh cpssHwInit directory, cpssDxChHwInit.c file
*/
typedef struct
{
    /** PLL WA Related Flag */
    GT_BOOL isResetDbEnabled;

    /** Mmicro Init CPLL First Time */
    GT_BOOL microInitCpllFirstTime;

    /* debug variables for multy DP devices */
    GT_U32  debug_force_numOfDp;
    GT_U32  debug_force_numOfPipes;
    GT_U32  falcon_force_stop_init_after_d2d_init;
    GT_U32  falcon_force_early_check_for_device_not_reset;
    GT_U32 allow_phase1_fail_without_cleanup;
    GT_U32 EGR_FILTER_PHYSICAL_PORT_LINK_STATUS_MASK_debug_print;

    /** Tail Drop unit register accsess WA */
    /** Bobcat2, BobK */
    ERRATA_BOBK_TXQ_TAIL_DROP_DATA_STC bobkTxqTailDropData;
    /** Aldrin; AC3X */
    ERRATA_BOBK_TXQ_TAIL_DROP_DATA_STC aldrinTxqTailDropData;
    /** Bobcat3 */
    ERRATA_BOBK_TXQ_TAIL_DROP_DATA_STC bc3TxqTailDropData;

    /** Units bad addresses ranges array for xCat3 */
    PRV_CPSS_DXCH_ERRATA_UNIT_RANGES_STC xCat3BadAddressRanges[64];

    /** For init system with no HW write to the device Device_ID within a Prestera chipset */
    GT_BOOL dxChFastBootSkipOwnDeviceInit;
    GT_BOOL dxChInitRegDefaults;

    /** pointer to a function used for DXCH devices that will do enhanced
    initialization of SW and HW parameters -- used by customers */
    PRV_CPSS_DXCH_PP_CONFIG_ENHANCED_INIT_FUNC prvCpssDxChPpConfigEnhancedInitFuncPtr;

} PRV_CPSS_MAIN_PP_DRV_MOD_DXCH_CPSS_DXCH_HW_INIT_DIR_SHARED_SRC_GLOBAL_DB;


/**
 * @struct PRV_CPSS_MAIN_PP_DRV_MOD_DXCH_CPSS_HW_INIT_DIR_SHARED_GLOBAL_DB
 *
 * @brief  Structure contain global variables that are meant to be shared
 * between processes (when CPSS is linked as shared object)
 *   in mainPpDrv module ,DxCh cpssHwInit directory
*/
typedef struct
{
    /** data of RegAddr, RegAddrVer1 files */
    PRV_CPSS_MAIN_PP_DRV_MOD_DXCH_CPSS_HW_INIT_DIR_SHARED_REG_SRC_GLOBAL_DB regSrc;

    /** data of RegAddrVer1_ppDb.c file */
    PRV_CPSS_MAIN_PP_DRV_MOD_DXCH_CPSS_HW_INIT_DIR_SHARED_REG_V1_SRC_GLOBAL_DB regV1Src;
    /** data of prvCpssDxChHwTables.c file */
    PRV_CPSS_MAIN_PP_DRV_MOD_DXCH_CPSS_HW_INIT_DIR_SHARED_TABLES_SRC_GLOBAL_DB tablesSrc;

    /* data of cpssDxChHwInit.c file */
    PRV_CPSS_MAIN_PP_DRV_MOD_DXCH_CPSS_DXCH_HW_INIT_DIR_SHARED_SRC_GLOBAL_DB dxChHwInitSrc;

    /** array of additional pointers to per device info for debugging */
    PRV_CPSS_DXCH_PP_CONFIG_STC* dxCh_prvCpssPpConfig[PRV_CPSS_MAX_PP_DEVICES_CNS];

    /** Falcon DP errata */
    GT_BOOL prvFalconPortDeleteWa_disabled;

} PRV_CPSS_MAIN_PP_DRV_MOD_DXCH_CPSS_HW_INIT_DIR_SHARED_GLOBAL_DB;

/**
* @struct PRV_CPSS_MAIN_PP_DRV_MOD_MACSEC_DRIVER_DIR_SHARED_GLOBAL_DB
 *
 * @brief  Structure contain global variables that are meant to be shared
 * between processes (when CPSS is linked as shared object)
 *   in mainPpDrv module ,macSec driver directory
*/
typedef struct
{
    /** pointer to MACSec DB */
    GT_VOID * dbPtr;

} PRV_CPSS_MAIN_PP_DRV_MOD_MACSEC_DRIVER_DIR_SHARED_GLOBAL_DB;

/**
* @struct PRV_CPSS_MAIN_PP_DRV_MOD_DXCH_TRUNK_DIR_SHARED_GLOBAL_DB
*
* @brief  Structure contain global variables that are meant to be shared
* between processes (when CPSS is linked as shared object)
* in mainPpDrv module , DxCh trunk directory
*/
typedef struct
{
    /**@brief debug flag to allow extra 'print info' during cpss trunk configurations */
    GT_BOOL debug_internal_cpssDxChTrunkTableEntrySet_sip5 /*= GT_FALSE*/;
    /**@brief debug flag to allow stretch to minimum of 8 members in the trunk */
    GT_BOOL flexCompatibleTo8Members /*= GT_FALSE*/;
    /**@brief indication to do 'stretch' number of members emulation in eArch */
    GT_U32   eArchDoStrechNumMembersWa /*= 1*/;
    /**@brief when stretching : this number sets MAX the number of replications */
    GT_U32   eArchDoStrechNumMembersWa_maxDuplications /*= 4*/;
    /**@brief performance stretch limit - we not want the stretch to impact performance
     * so limit the stretch to up to 64 members
     * current use of 4K means --> 'no limit' on the stretch */
    GT_U32   eArchDoStrechNumMembersWa_maxMembersAfterStretch /*= _4K*/;

} PRV_CPSS_MAIN_PP_DRV_MOD_DXCH_TRUNK_DIR_SHARED_GLOBAL_DB;

/**
* @struct PRV_CPSS_MAIN_PP_DRV_MOD_DXCH_CONFIG_DIR_SHARED_GLOBAL_DB
*
* @brief  Structure contain global variables that are meant to be shared
* between processes (when CPSS is linked as shared object)
* in mainPpDrv module , DxCh config directory
*/
typedef struct
{
    /**@brief Hawk (AC5P) : indication of shared tables : SMB clients init done.
    */
    GT_BOOL  hawkNumberOfClientsArr_initDone /*= GT_FALSE*/;
    /**@brief Hawk (AC5P) : array of clients : malloc at run-time
        size : SBM_CLIENT_TYPE__LAST__E
    */
    GT_U32   *hawkNumberOfClientsArr;
    /**@brief Hawk (AC5P) : array of clients : malloc at run-time
        size : SBM_CLIENT_TYPE__LAST__E
    */
    GT_U32   *hawkPriorityArr;
    /* @brief flag to allow debug prints for the shared tables */
    GT_U32   debug_sharedTables /*= 0*/;

    struct{
        /**@brief the tree of the 88e1690 device
         * NOTE: each 88e1690 device requires alloc of it own instance of this tree
         *   malloc at run-time
        */
        PRV_CPSS_DRV_INTERRUPT_SCAN_STC  *MacPhy88E1690_IntrScanArr;
        /**@brief This array maps an interrupt index to its relevant interrupt mask reg
        * default value, cell i indicates the address for interrupts
        * (32 * i  --> 32 * (i + 1) - 1).
        * summary bits are turned on and the specific cause mask is enabled when
        * bounded by appl.
        *   malloc at run-time
        */
        GT_U32 *MacPhy88E1690_MaskRegDefaultSummaryArr;
        /**@brief This array maps an interrupt index to its
        * relevant interrupt mask registers address,
        * cell i indicates the address for interrupts
        * (32 * i  --> 32 * (i + 1) - 1).
        *   malloc at run-time
        */
        GT_U32 *MacPhy88E1690_MaskRegMapArr;

        /**@brief This is the 'driver object' of the Phy88e1690.
        *   malloc at run-time
        */
        CPSS_MACDRV_OBJ_STC *macPhy88e1690ObjPtr;

        /**@brief pointer to memory of type :
        *   GT_CPSS_STATS_COUNTER_SET    saveStatsCounterSet[SMI_DEVICE_MAX_NUM][10]
        *   malloc at run-time
        */
        void*           saveStatsCounterSet;

        /**@brief : !!! WM only !!! indication to speedup the SMI operations as 'direct' or as 'indirect'
        */
        GT_U32 directAccessMode1 /*=  0*/;
        /**@brief : !!! WM only !!! indication to speedup the SMI operations as 'direct' or as 'indirect'
            only for counters
        */
        GT_U32 directAccessMode /*=  0*/;

    }config88e1690;/* sub directory in dxch/config */


} PRV_CPSS_MAIN_PP_DRV_MOD_DXCH_CONFIG_DIR_SHARED_GLOBAL_DB;

/**
* @struct PRV_CPSS_MAIN_PP_DRV_MOD_FDB_MANAGER_DIR_SHARED_GLOBAL_DB
 *
 * @brief  Structure contains global variables that are meant to be shared
 * between processes (when CPSS is linked as shared object)
 *   in mainPpDrv module ,fdbManager directory
*/
typedef struct
{
    /** FDB manager DB - for all instance */
    PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC* prvCpssDxChFdbManagerDbArr[CPSS_DXCH_BRG_FDB_MANAGER_MAX_INSTANCE_NUM_CNS];

    /** Bitmap of devices registered to ANY of the manager
     *   NOTE : a device can't be registered to more than single manager !!!
     */
    GT_U32      prvCpssDxChFdbManagerDbGlobalDevsBmpArr[NUM_WORDS_DEVS_BMP_CNS];

    /** Old FDB manager memory for HA process, required to compare after HA, to ensure successful transition */
    PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC  *globalFdbManagerPtr;

    /** Debug checks for FDB Manager */
    GT_BOOL                                          debug1Print;

    /** Debug print for FDB Manager rehash/relocate entries */
    GT_BOOL                                          reHashDebugPrint;
} PRV_CPSS_MAIN_PP_DRV_MOD_FDB_MANAGER_DIR_SHARED_GLOBAL_DB;

/**
* @struct PRV_CPSS_MAIN_PP_DRV_MOD_DXCH_TCAM_DIR_SHARED_GLOBAL_DB
 *
 * @brief  Structure contains global variables that are meant to be shared
 * between processes (when CPSS is linked as shared object)
 *   in mainPpDrv module, TCAM directory
*/
typedef struct
{
    /** Number of register read operation to wait completion of all internal
       processes in packet processor before enable parity daemon.
       The WA requires 120 PP core clocks delay.
       The read register procedure takes at least 114
       (number was get by tests on BC2 device) Core Clocks for code run in internal
       CPU. Two read register procedures guaranties 120 PP core clock delay.
       The WA algorithm uses prvCpssHwPpSetRegField function that already includes
       one read. So need only
       (2 - 1) = 1 additional read operations. */
    GT_U32 prvTcamWaEnableCount;

    /** Number of register read operation to wait completion of all internal
       processes in packet processor after disable parity daemon and before write
       TCAM entry. The delay is calculated same as above. */
    GT_U32 prvTcamWaDisableCount;

} PRV_CPSS_MAIN_PP_DRV_MOD_DXCH_TCAM_DIR_SHARED_GLOBAL_DB;

/**
* @struct PRV_CPSS_MAIN_PP_DRV_MOD_DXCH_DIAG_DIR_SHARED_DATA_INTEGRITY_MAIN_MAPPING_DB_SRC_GLOBAL_DB
*
* @brief  Structure contain global variables that are meant to be shared
* between processes (when CPSS is linked as shared object)
* in mainPpDrv module, DxCh diag directory
*/
typedef struct
{
    /** Number of entries in DB for Cetus device */
    GT_U32 cetusDbSize; /* = PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_UNASSIGNED_CNS */
} PRV_CPSS_MAIN_PP_DRV_MOD_DXCH_DIAG_DIR_SHARED_DATA_INTEGRITY_SRC_GLOBAL_DB;

/**
* @struct PRV_CPSS_MAIN_PP_DRV_MOD_DXCH_DIAG_DIR_SHARED_GLOBAL_DB
*
* @brief  Structure contain global variables that are meant to be shared
* between processes (when CPSS is linked as shared object)
* in mainPpDrv module , DxCh diag directory
*/
typedef struct
{
    /** Should serdes RX optimization algorithm pring intermidiate reports */
    GT_BOOL diagSerdesTuningTracePrintEnable;
    PRV_CPSS_MAIN_PP_DRV_MOD_DXCH_DIAG_DIR_SHARED_DATA_INTEGRITY_SRC_GLOBAL_DB dataIntegrityDb;
} PRV_CPSS_MAIN_PP_DRV_MOD_DXCH_DIAG_DIR_SHARED_GLOBAL_DB;
/**
* @struct PRV_CPSS_MAIN_PP_DRV_MOD_NETWORK_IF_DIR_SHARED_GLOBAL_DB
*
* @brief  Structure contain global variables that are meant to be shared
* between processes (when CPSS is linked as shared object)
* in mainPpDrv module , networkIf directory
*/
typedef struct
{
    /** debug flag to allow print (dump) of the Rx Packet include DSA tag.
         (before DSA tag is removed) */
    GT_BOOL debug_dumpFullPacket;

    /** debug flag to allow print (dump) of the RAW DSA tags that got on the packet .
       regardless to CPSS parsing of the DSA tag */
    GT_BOOL debug_dumpDsa;

}PRV_CPSS_MAIN_PP_DRV_MOD_NETWORK_IF_DIR_SHARED_GLOBAL_DB;

/**
* @struct PRV_CPSS_MAIN_PP_DRV_MOD_DXCH_VTCAM__DIR_SHARED_GLOBAL_DB
*
* @brief  Structure contain global variables that are meant to be shared
* between processes (when CPSS is linked as shared object)
* in mainPpDrv module , DxCh VTCAM directory
*/
typedef struct
{
    /** TCAM manager DB */
    PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_PTR vTcamMngDB[CPSS_DXCH_VIRTUAL_TCAM_MNG_MAX_ID_CNS + 1];
    /** TCAM Manager HA Replay DB
     * Used to store list of rules which couldn't be located in the the TCAM HA DB
     * during config replay. These rules will be re-installed at the last leg of HA
     * completion.
     */
    CPSS_DXCH_VIRTUAL_TCAM_HA_DB_REPLAY_RULE_ENTRY_STC *replayRuleDb;
    CPSS_DXCH_VIRTUAL_TCAM_HA_DB_REPLAY_RULE_ENTRY_STC *replayRuleDbTail;
    /** TCAM Manager HA Replay DB
     * Used to store list of rules which were unmapped found during HA config-replay
     */
    CPSS_DXCH_VIRTUAL_TCAM_HA_UNMAPPED_RULE_ENTRY_STC *replayUnmappedRuleDb;
    CPSS_DXCH_VIRTUAL_TCAM_HA_UNMAPPED_RULE_ENTRY_STC *replayUnmappedRuleDbTail;
    /** debug flag to enable the use of debug_writeRulePosition
     *  to allow 'ruleWrite' to add rule at the start of the priority
     *  (and not to the end)
     */
    GT_BOOL     debug_writeRuleForceFirstPosition_enable;
    /** enable debug trace of priority driven VTCAMs rules moving */
    GT_U32      prvCpssDxChVirtualTcamDbUtilNewRuleLogIndexVer1GetMoveTraceEnable;
    /** version of priority driven VTCAMs position prepare function */
    GT_U32      priorityAddNewRuleNeedNewLogicalIndexFuncionVersion;

    /** enable debug dump of segment work-space */
    GT_BOOL prvCpssDxChVirtualTcamDebugTraceEnable;
    GT_U32  prvCpssDxChVirtualTcamDbSegmentTableExceptionCounter;
    GT_BOOL prvCpssDxChVirtualTcamDbSegmentTablePrimaryTreeMrgSplTraceEnable;
    GT_BOOL prvCpssDxChVirtualTcamSegmentTableAvlItemDebugPrintEnable;
    GT_BOOL prvCpssDxChVirtualTcamDbSegSpaceWorkDbDumpEnable;
    /** memory to build list of cTCAM space-conversion rules */
    PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_CHANGE_RULE_PTR work_rules_arr[100];
    GT_U32  work_rules_arr_used;
    GT_BOOL prvCpssDxChVirtualTcamDbSegSpaceWorkDbConvertByRuleTraceEnable;
    GT_U32  prvCpssDxChVirtualTcamDbSegSpaceWorkDbRetrieveNewFreeSpaceTraceEnable;
    /** HA debug variables */
    CPSS_DXCH_VIRTUAL_TCAM_MNG_CONFIG_PARAM_STC haAppVtcamMngCfgParam;
    GT_BOOL haapp_ignore_write_ret;
    GT_BOOL haVtcamDebugSegmentsFlag;
    GT_BOOL haVtcamDebugDbFlag;
    GT_BOOL haVtcamDebug;
    GT_BOOL haVtcamTimeDebug;
    GT_U32  vtcamHaErrDbCounter;
    PRV_VTCAM_HA_ERR_DB_ENTRY_STC vtcamHaErrDb[PRV_VTCAM_HA_MAX_ERR_CNS];
    /* dynamic allocated array for vTCAM resize */
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_RESIZE_HW_INDEX_MAP_INFO_STC    *prvCpssDxChVirtualTcamDbResizeHwIndexDbArr;
    GT_U32 prvCpssDxChVirtualTcamDbNumEntries_resizeHwIndexDbArr;

    /** work variables of prvCpssDxChVirtualTcamDbSegSpaceLookupRowsCountByType */
    GT_U32 rowsBitmapArr0[PRV_WORDS_IN_BLOCK_ROWS_BITMAP_CNS];
    GT_U32 rowsBitmapArr1[PRV_WORDS_IN_BLOCK_ROWS_BITMAP_CNS];
    GT_U32 rowsBitmapArr0_1[PRV_WORDS_IN_BLOCK_ROWS_BITMAP_CNS];
    GT_U32 rowsBitmapArr1_1[PRV_WORDS_IN_BLOCK_ROWS_BITMAP_CNS];
    GT_U32 rowsBitmapArrRes[PRV_WORDS_IN_BLOCK_ROWS_BITMAP_CNS];

} PRV_CPSS_MAIN_PP_DRV_MOD_DXCH_VTCAM_DIR_SHARED_GLOBAL_DB;

/**
* @struct PRV_CPSS_MAIN_PP_DRV_MOD_DXCH_PCL_DIR_SHARED_GLOBAL_DB
*
* @brief  Structure contain global variables that are meant to be shared
* between processes (when CPSS is linked as shared object)
* in mainPpDrv module , DxCh pcl directory
*/
typedef struct
{
    /** Debug environment parameter to allow the 'redirect' action to have bypassIngressPipe = 0 and bypassBridge = 0
        NOTE: in non sip5 devices this flag can be used to debug 'redirect' action with
        bypassIngressPipe = 0 and bypassBridge = 0

        In sip5 devices the full control is with the application anyway !!!
    */
    GT_U32      debugBypassFullControl;

} PRV_CPSS_MAIN_PP_DRV_MOD_DXCH_PCL_DIR_SHARED_GLOBAL_DB;

#endif /*CHX_FAMILY*/


/**
* @struct PRV_CPSS_MAIN_PP_DRV_MOD_SHARED_GLOBAL_DB
 *
 * @brief  Structure contain global variables that are meant to be shared
 * between processes (when CPSS is linked as shared object)
 *   in mainPpDrv module
*/
typedef struct
{
    /*add here directories*/
    PRV_CPSS_MAIN_PP_DRV_MOD_CPSS_HW_INIT_DIR_SHARED_GLOBAL_DB hwInitDir;
    PRV_CPSS_MAIN_PP_DRV_MOD_DIAG_DIR_SHARED_GLOBAL_DB         diagDir;
#ifdef CHX_FAMILY
    PRV_CPSS_MAIN_PP_DRV_MOD_LAB_SERVICES_DIR_SHARED_GLOBAL_DB labServicesDir;
    PRV_CPSS_MAIN_PP_DRV_MOD_PORT_DIR_SHARED_GLOBAL_DB         portDir;
    PRV_CPSS_MAIN_PP_DRV_MOD_DXCH_PORT_DIR_SHARED_GLOBAL_DB    dxChPortDir;
    PRV_CPSS_MAIN_PP_DRV_MOD_PTP_DIR_SHARED_GLOBAL_DB          ptpDir;
    PRV_CPSS_MAIN_PP_DRV_MOD_IDEBUG_DIR_SHARED_GLOBAL_DB       idebugDir;
    PRV_CPSS_MAIN_PP_DRV_MOD_IP_LPM_DIR_SHARED_GLOBAL_DB       ipLpmDir;
    PRV_CPSS_MAIN_PP_DRV_MOD_TXQ_DBG_DIR_SHARED_GLOBAL_DB      txqDbgDir;
    PRV_CPSS_MAIN_PP_DRV_MOD_FLOW_MANAGER_DIR_SHARED_GLOBAL_DB flowMgrDir;
    PRV_CPSS_MAIN_PP_DRV_MOD_CUCKOO_DIR_SHARED_GLOBAL_DB       cuckooDir;
    PRV_CPSS_MAIN_PP_DRV_MOD_DXCH_CPSS_HW_INIT_DIR_SHARED_GLOBAL_DB dxChCpssHwInit;
    PRV_CPSS_MAIN_PP_DRV_MOD_MACSEC_DRIVER_DIR_SHARED_GLOBAL_DB        macSecDriverDir;
    PRV_CPSS_MAIN_PP_DRV_MOD_DXCH_TRUNK_DIR_SHARED_GLOBAL_DB   dxChTrunkDir;
    PRV_CPSS_MAIN_PP_DRV_MOD_DXCH_CONFIG_DIR_SHARED_GLOBAL_DB  dxChConfigDir;
    PRV_CPSS_MAIN_PP_DRV_MOD_FDB_MANAGER_DIR_SHARED_GLOBAL_DB  fdbManagerDir;
    PRV_CPSS_MAIN_PP_DRV_MOD_DXCH_TCAM_DIR_SHARED_GLOBAL_DB     dxChTcamDir;
    PRV_CPSS_MAIN_PP_DRV_MOD_DXCH_DIAG_DIR_SHARED_GLOBAL_DB    dxChDiagDir;
    PRV_CPSS_MAIN_PP_DRV_MOD_NETWORK_IF_DIR_SHARED_GLOBAL_DB   networkIfDir;
    PRV_CPSS_MAIN_PP_DRV_MOD_DXCH_VTCAM_DIR_SHARED_GLOBAL_DB   dxChVTcamDir;
    PRV_CPSS_MAIN_PP_DRV_MOD_DXCH_PCL_DIR_SHARED_GLOBAL_DB     dxChPclDir;
    PRV_CPSS_MAIN_PP_DRV_MOD_DXCH_EM_MANAGER_DIR_SHARED_GLOBAL_DB   dxchEmMgrDir;
    PRV_CPSS_MAIN_PP_DRV_MOD_DXCH_TTI_DIR_SHARED_GLOBAL_DB      dxchTtiDir;
#endif
} PRV_CPSS_MAIN_PP_DRV_MOD_SHARED_GLOBAL_DB;

#endif /* __prvCpssMainPpDrvModGlobalSharedDb */


