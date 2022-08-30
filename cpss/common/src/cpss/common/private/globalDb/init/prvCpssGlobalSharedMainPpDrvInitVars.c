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
* @file prvCpssGlobalSharedMainPpDrvInitVars.c
*
* @brief This file  Initialize global shared variables used in  module:mainPpDrv
*
* @version   1
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/common/config/private/prvCpssConfigTypes.h>
#include <cpss/common/cpssHwInit/cpssHwInit.h>
#include <cpss/common/diag/private/prvCpssCommonDiag.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>
#ifdef CHX_FAMILY
#include <cpss/dxCh/dxChxGen/diag/private/prvCpssDxChDiag.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortSerdesCfg.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortSpeed.h>
#endif

/*global variables macros*/
#define PRV_SHARED_HW_INIT_DIR_HW_INIT_SRC_GLOBAL_VAR_INIT(_var,_value)\
    PRV_SHARED_GLOBAL_VAR_SET(mainPpDrvMod.hwInitDir.hwInitSrc._var,_value)

#define PRV_SHARED_PORT_DIR_IP_LPM_SRC_GLOBAL_VAR_INIT(_var,_value)\
    PRV_SHARED_GLOBAL_VAR_SET(mainPpDrvMod.ipLpmDir.ipLpmSrc._var,_value)

#define PRV_SHARED_PORT_DIR_TXQ_DBG_SRC_GLOBAL_VAR_INIT(_var,_value)\
    PRV_SHARED_GLOBAL_VAR_SET(mainPpDrvMod.txqDbgDir.txqDbgSrc._var,_value)

#define PRV_SHARED_FLOW_MANAGER_DIR_FLOW_MANAGER_SRC_GLOBAL_VAR_INIT(_var,_value)\
    PRV_SHARED_GLOBAL_VAR_SET(mainPpDrvMod.flowMgrDir.flowMgrSrc._var,_value)

#define PRV_SHARED_PORT_DIR_IP_LPM_HW_SRC_GLOBAL_VAR_INIT(_var,_value)\
    PRV_SHARED_GLOBAL_VAR_SET(mainPpDrvMod.ipLpmDir.ipLpmHwSrc._var,_value)

#define PRV_SHARED_HW_INIT_DIR_DXCH_HW_INIT_REG_V1_SRC_GLOBAL_VAR_INIT(_var,_value)\
    PRV_SHARED_GLOBAL_VAR_SET(mainPpDrvMod.dxChCpssHwInit.regV1Src._var,_value)

#define PRV_SHARED_HW_INIT_DIR_DXCH_HW_INIT_REG_V1_SRC_GLOBAL_VAR_GET(_var)\
    PRV_SHARED_GLOBAL_VAR_GET(mainPpDrvMod.dxChCpssHwInit.regV1Src._var)

#define PRV_SHARED_HW_INIT_DIR_DXCH_HW_INIT_DXCH_HW_INIT_SRC_GLOBAL_VAR_GET(_var)\
    PRV_SHARED_GLOBAL_VAR_GET(mainPpDrvMod.dxChCpssHwInit.dxChHwInitSrc._var)

#define PRV_SHARED_DIAG_DIR_DIAG_BIRST_SIZE_GEN_DIAG_SRC_GLOBAL_VAR_INIT(_var,_value)\
    PRV_SHARED_GLOBAL_VAR_SET(mainPpDrvMod.diagDir.genDiagSrc._var,_value)

#define PRV_SHARED_DIAG_DIR_DATA_INTEGRITY_SRC_GLOBAL_VAR_INIT(_var,_value) \
    PRV_SHARED_GLOBAL_VAR_SET(mainPpDrvMod.dxChDiagDir.dataIntegrityDb._var,_value)

#define PRV_SHARED_IP_LPM_DIR_IP_LPM_RAM_SRC_GLOBAL_VAR_INIT(_var,_value)\
    PRV_SHARED_GLOBAL_VAR_SET(mainPpDrvMod.ipLpmDir.ipLpmRamSrc._var,_value)

#define PRV_SHARED_IP_LPM_DIR_SIP6_IP_LPM_RAM_SRC_GLOBAL_VAR_INIT(_var,_value)\
    PRV_SHARED_GLOBAL_VAR_SET(mainPpDrvMod.ipLpmDir.sip6IpLpmRamSrc._var,_value)

#define PRV_SHARED_IP_LPM_DIR_SIP6_IP_LPM_RAM_DEFRAG_SRC_GLOBAL_VAR_INIT(_var,_value)\
    PRV_SHARED_GLOBAL_VAR_SET(mainPpDrvMod.ipLpmDir.sip6IpLpmRamDefragSrc._var,_value)

#define PRV_SHARED_IP_LPM_DIR_LPM_RAM_DBG_SRC_GLOBAL_VAR_INIT(_var,_value)\
    PRV_SHARED_GLOBAL_VAR_SET(mainPpDrvMod.ipLpmDir.lpmRamDbgSrc._var,_value)

#define PRV_SHARED_IP_LPM_DIR_LPM_TCAM_SRC_GLOBAL_VAR_INIT(_var,_value)\
    PRV_SHARED_GLOBAL_VAR_SET(mainPpDrvMod.ipLpmDir.lpmTcamSrc._var,_value)

#define PRV_SHARED_IP_LPM_DIR_LPM_TCAM_DBG_SRC_GLOBAL_VAR_INIT(_var,_value)\
    PRV_SHARED_GLOBAL_VAR_SET(mainPpDrvMod.ipLpmDir.lpmTcamDbgSrc._var,_value)

#define PRV_SHARED_IP_LPM_DIR_LPM_TCAM_COMMON_SRC_GLOBAL_VAR_INIT(_var,_value)\
    PRV_SHARED_GLOBAL_VAR_SET(mainPpDrvMod.ipLpmDir.lpmTcamCommonSrc._var,_value)

#define PRV_SHARED_IP_LPM_DIR_LPM_TCAM_PAT_TRIE_SRC_GLOBAL_VAR_INIT(_var,_value)\
    PRV_SHARED_GLOBAL_VAR_SET(mainPpDrvMod.ipLpmDir.lpmTcamPatTrieSrc._var,_value)

/*global variables macros*/
#define PRV_SHARED_IP_LPM_DIR_IP_LPM_RAM_SRC_GLOBAL_VAR_GET(_var)\
    PRV_SHARED_GLOBAL_VAR_GET(mainPpDrvMod.ipLpmDir.ipLpmRamSrc._var)

#define PRV_SHARED_IP_LPM_DIR_LPM_RAM_DBG_SRC_GLOBAL_VAR_GET(_var)\
    PRV_SHARED_GLOBAL_VAR_GET(mainPpDrvMod.ipLpmDir.lpmRamDbgSrc._var)

#define PRV_SHARED_IP_LPM_DIR_LPM_TCAM_COMMON_SRC_GLOBAL_VAR_GET(_var)\
    PRV_SHARED_GLOBAL_VAR_GET(mainPpDrvMod.ipLpmDir.lpmTcamCommonSrc._var)

#define PRV_SHARED_IP_LPM_DIR_SIP6_IP_LPM_RAM_SRC_GLOBAL_VAR_GET(_var)\
    PRV_SHARED_GLOBAL_VAR_GET(mainPpDrvMod.ipLpmDir.sip6IpLpmRamSrc._var)

#define PRV_SHARED_IP_LPM_DIR_SIP6_IP_LPM_RAM_DEFRAG_SRC_GLOBAL_VAR_GET(_var)\
    PRV_SHARED_GLOBAL_VAR_GET(mainPpDrvMod.ipLpmDir.sip6IpLpmRamDefragSrc._var)

#define PRV_SHARED_PORT_DIR_PORT_CTRL_SRC_GLOBAL_VAR_INIT(_var,_value)\
    PRV_SHARED_GLOBAL_VAR_SET(mainPpDrvMod.portDir.portCtrlSrc._var,_value)

/**
 * @internal cpssGlobalSharedDbMainPpDrvModDiagDirGenDiagSrcDataSectionInit function
 * @endinternal
 *
 * @brief   Initialize global shared variables used in :
 *          module:mainPpDrv
 *          dir:diag
 *          src:prvCpssGenDiag.c
 *
 */
static GT_VOID cpssGlobalSharedDbMainPpDrvModDiagDirGenDiagSrcDataSectionInit
(
    GT_VOID
)
{
    PRV_SHARED_DIAG_DIR_DIAG_BIRST_SIZE_GEN_DIAG_SRC_GLOBAL_VAR_INIT(memDiagBurstSize, 4);
#ifdef CHX_FAMILY
    PRV_SHARED_DIAG_DIR_DATA_INTEGRITY_SRC_GLOBAL_VAR_INIT(cetusDbSize, PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_UNASSIGNED_CNS);
#endif
}

/**
 * @internal cpssGlobalSharedDbMainPpDrvModHwInitDirHwInitSrcDataSectionInit function
 * @endinternal
 *
 * @brief   Initialize global shared variables used in :
 *          module:mainPpDrv
 *          dir:cpssHwInit
 *          src:cpssHwInit
 *
 */
static GT_VOID cpssGlobalSharedDbMainPpDrvModHwInitDirHwInitSrcDataSectionInit
(
    GT_VOID
)
{
    PRV_SHARED_HW_INIT_DIR_HW_INIT_SRC_GLOBAL_VAR_INIT(
        prvCpssHwDevNumModeDb[0], CPSS_GEN_CFG_HW_DEV_NUM_MODE_SINGLE_E);
}


/**
 * @internal cpssGlobalSharedDbMainPpDrvModHwInitDirDataSectionInit function
 * @endinternal
 *
 * @brief   Initialize global shared variables used in :
 *          module:mainPpDrv
 *          dir:cpssHwInit
 *
 */
static GT_VOID cpssGlobalSharedDbMainPpDrvModHwInitDirDataSectionInit
(
    GT_VOID
)
{
     /*add here source files   initializers*/
    cpssGlobalSharedDbMainPpDrvModHwInitDirHwInitSrcDataSectionInit();
}

#ifdef CHX_FAMILY
/**
 * @internal
 *           cpssGlobalSharedDbMainPpDrvModIpLpmDirIpLpmSrcDataSectionInit
 *           function
 * @endinternal
 *
 * @brief   Initialize global shared variables used in :
 *          module:mainPpDrv
 *          dir:cpssDxChIpLpm
 *          src:cpssDxChIpLpm
 *
 */
static GT_VOID cpssGlobalSharedDbMainPpDrvModIpLpmDirIpLpmSrcDataSectionInit
(
    GT_VOID
)
{
    PRV_SHARED_PORT_DIR_IP_LPM_SRC_GLOBAL_VAR_INIT(lpmDbSL, NULL);
    PRV_SHARED_PORT_DIR_IP_LPM_HW_SRC_GLOBAL_VAR_INIT(k, 0);
    PRV_SHARED_PORT_DIR_IP_LPM_HW_SRC_GLOBAL_VAR_INIT(tempGonArrayPtr, NULL);

    /* reset ipLpmRamSrc fields*/
    cpssOsMemSet(PRV_SHARED_IP_LPM_DIR_IP_LPM_RAM_SRC_GLOBAL_VAR_GET(pRangeArr),0xFF, /*0xFFFFFFFF indicates untouched level*/
                 sizeof(PRV_CPSS_DXCH_LPM_RAM_RANGE_SHADOW_STC*)*MAX_LPM_LEVELS_CNS);

    cpssOsMemSet(PRV_SHARED_IP_LPM_DIR_SIP6_IP_LPM_RAM_SRC_GLOBAL_VAR_GET(pRangeArr),0xFF, /*0xFFFFFFFF indicates untouched level*/
                 sizeof(PRV_SHARED_IP_LPM_DIR_SIP6_IP_LPM_RAM_SRC_GLOBAL_VAR_GET(pRangeArr)));

    /* reset lpmTcamCommonSrc fields*/
    PRV_SHARED_IP_LPM_DIR_LPM_TCAM_COMMON_SRC_GLOBAL_VAR_INIT(tcamRows,0);
    cpssOsMemSet(PRV_SHARED_IP_LPM_DIR_LPM_TCAM_COMMON_SRC_GLOBAL_VAR_GET(cpssDxChDefaultMcIpAddr),
                 0,sizeof(PRV_SHARED_IP_LPM_DIR_LPM_TCAM_COMMON_SRC_GLOBAL_VAR_GET(cpssDxChDefaultMcIpAddr)));
    PRV_SHARED_IP_LPM_DIR_LPM_TCAM_COMMON_SRC_GLOBAL_VAR_GET(cpssDxChDefaultMcIpAddr)[0][0]=0xE0;
    PRV_SHARED_IP_LPM_DIR_LPM_TCAM_COMMON_SRC_GLOBAL_VAR_GET(cpssDxChDefaultMcIpAddr)[1][0]=0xFF;

    cpssOsMemSet(PRV_SHARED_IP_LPM_DIR_LPM_TCAM_COMMON_SRC_GLOBAL_VAR_GET(cpssDxChDefaultMcIpPrefixLen),
                 0,sizeof(PRV_SHARED_IP_LPM_DIR_LPM_TCAM_COMMON_SRC_GLOBAL_VAR_GET(cpssDxChDefaultMcIpPrefixLen)));
    PRV_SHARED_IP_LPM_DIR_LPM_TCAM_COMMON_SRC_GLOBAL_VAR_GET(cpssDxChDefaultMcIpPrefixLen)[0]=4;
    PRV_SHARED_IP_LPM_DIR_LPM_TCAM_COMMON_SRC_GLOBAL_VAR_GET(cpssDxChDefaultMcIpPrefixLen)[1]=8;
}


/**
 * @internal
 *           cpssGlobalSharedDbMainPpDrvModIpLpmDirDataSectionInit
 *           function
 * @endinternal
 *
 * @brief   Initialize global shared variables used in :
 *          module:mainPpDrv
 *          dir:cpssDxChIpLpm
 *
 */
static GT_VOID cpssGlobalSharedDbMainPpDrvModIpLpmDirDataSectionInit
(
    GT_VOID
)
{
     /*add here source files   initializers*/
    cpssGlobalSharedDbMainPpDrvModIpLpmDirIpLpmSrcDataSectionInit();
}


/**
 * @internal
 *           cpssGlobalSharedDbMainPpDrvModDxChTrunkDirDataSectionInit
 *           function
 * @endinternal
 *
 * @brief   Initialize global shared variables used in :
 *          module:mainPpDrv
 *          dir:cpssDxChxGen/trunk
 *
 */
static GT_VOID cpssGlobalSharedDbMainPpDrvModDxChTrunkDirDataSectionInit
(
    GT_VOID
)
{
#define DXCH_TRUNK_DIR   mainPpDrvMod.dxChTrunkDir
    PRV_SHARED_GLOBAL_VAR_SET(DXCH_TRUNK_DIR.debug_internal_cpssDxChTrunkTableEntrySet_sip5,GT_FALSE);
    PRV_SHARED_GLOBAL_VAR_SET(DXCH_TRUNK_DIR.flexCompatibleTo8Members,GT_FALSE);
    PRV_SHARED_GLOBAL_VAR_SET(DXCH_TRUNK_DIR.eArchDoStrechNumMembersWa,1);
    PRV_SHARED_GLOBAL_VAR_SET(DXCH_TRUNK_DIR.eArchDoStrechNumMembersWa_maxDuplications,4);
    PRV_SHARED_GLOBAL_VAR_SET(DXCH_TRUNK_DIR.eArchDoStrechNumMembersWa_maxMembersAfterStretch,_4K);
#undef DXCH_TRUNK_DIR
}

/**
 * @internal
 *           cpssGlobalSharedDbMainPpDrvModDxChConfigDirDataSectionInit
 *           function
 * @endinternal
 *
 * @brief   Initialize global shared variables used in :
 *          module:mainPpDrv
 *          dir:cpssDxChxGen/Config
 *
 */
static GT_VOID cpssGlobalSharedDbMainPpDrvModDxChConfigDirDataSectionInit
(
    GT_VOID
)
{
    /* empty , as all the variables are : 0/GT_FALSE and all pointers are NULL */
}

/**
 * @internal
 *           cpssGlobalSharedDbMainPpDrvModTxqDbgDirTxqDbgSrcDataSectionInit
 *           function
 * @endinternal
 *
 * @brief   Initialize global shared variables used in :
 *          module:mainPpDrv
 *          dir:port
 *          src:cpssDxChPortCtrl
 *
 */
static GT_VOID cpssGlobalSharedDbMainPpDrvModTxqDbgDirTxqDbgSrcDataSectionInit
(
    GT_VOID
)
{
#define PRV_CPSS_DXCH_SIP6_TXQ_DRAIN_ITERATION_NUM_ON_EMULATOR_MAC 300

    PRV_SHARED_PORT_DIR_TXQ_DBG_SRC_GLOBAL_VAR_INIT(debug_sip6_cpssDxChPortTxBufNumberGet, 0);
    PRV_SHARED_PORT_DIR_TXQ_DBG_SRC_GLOBAL_VAR_INIT(debug_sip6_TO_polling_iterations,
                                                   PRV_CPSS_DXCH_SIP6_TXQ_DRAIN_ITERATION_NUM_ON_EMULATOR_MAC);
    PRV_SHARED_PORT_DIR_TXQ_DBG_SRC_GLOBAL_VAR_INIT(debug_sip6_TO_txq_polling_interval, 200);
    PRV_SHARED_PORT_DIR_TXQ_DBG_SRC_GLOBAL_VAR_INIT(debug_sip6_TO_after_txq_drain, 500);
    PRV_SHARED_PORT_DIR_TXQ_DBG_SRC_GLOBAL_VAR_INIT(debug_sip6_TO_after_mac_disabled, 500);
    PRV_SHARED_PORT_DIR_TXQ_DBG_SRC_GLOBAL_VAR_INIT(old_value, 0);
}

/**
 * @internal
 *           cpssGlobalSharedDbMainPpDrvModTxQDbgDirDataSectionInit
 *           function
 * @endinternal
 *
 * @brief   Initialize global shared variables used in :
 *          module:mainPpDrv
 *          dir:cpssDxChPortCtrl
 *
 */
static GT_VOID cpssGlobalSharedDbMainPpDrvModTxqDbgDirDataSectionInit
(
    GT_VOID
)
{
     /*add here source files   initializers*/
    cpssGlobalSharedDbMainPpDrvModTxqDbgDirTxqDbgSrcDataSectionInit();
}

/**
 * @internal cpssGlobalSharedDbMainPpDrvModDxChCpssHwInitDirRegV1SrcDataSectionInit function
 * @endinternal
 *
 * @brief   Initialize global shared variables used in :
 *          module:mainPpDrv
 *          dir:DxCh cpssHwInit
 *          src:prvCpssDxChHwRegAddrVer1_ppDb.c
 *
 */
static GT_VOID cpssGlobalSharedDbMainPpDrvModDxChCpssHwInitDirRegV1SrcDataSectionInit
(
    GT_VOID
)
{
    PRV_CPSS_DXCH_UNITID_UNITBASEADDR_STC invalidEntry = {PRV_CPSS_DXCH_UNIT_LAST_E, NON_VALID_ADDR_CNS};

    PRV_SHARED_HW_INIT_DIR_DXCH_HW_INIT_REG_V1_SRC_GLOBAL_VAR_GET(prvDxChFalconUnitsIdUnitBaseAddrArr)[0] =
    PRV_SHARED_HW_INIT_DIR_DXCH_HW_INIT_REG_V1_SRC_GLOBAL_VAR_GET(prvDxChHawkUnitsIdUnitBaseAddrArr)[0] =
    PRV_SHARED_HW_INIT_DIR_DXCH_HW_INIT_REG_V1_SRC_GLOBAL_VAR_GET(prvDxChPhoenixUnitsIdUnitBaseAddrArr)[0] =
    PRV_SHARED_HW_INIT_DIR_DXCH_HW_INIT_REG_V1_SRC_GLOBAL_VAR_GET(prvDxChHarrierUnitsIdUnitBaseAddrArr)[0] =
    PRV_SHARED_HW_INIT_DIR_DXCH_HW_INIT_REG_V1_SRC_GLOBAL_VAR_GET(prvDxChIronmanLUnitsIdUnitBaseAddrArr)[0] =
        invalidEntry;
    PRV_SHARED_HW_INIT_DIR_DXCH_HW_INIT_REG_V1_SRC_GLOBAL_VAR_GET(prevPipeIndex) = 0xFFFFFFFF;
    PRV_SHARED_HW_INIT_DIR_DXCH_HW_INIT_REG_V1_SRC_GLOBAL_VAR_GET(gopBaseAddr) = 0xFFFFFFFF;
}

/**
 * @internal cpssGlobalSharedDbMainPpDrvModDxChCpssHwInitDirDxChHwInitSrcDataSectionInit function
 * @endinternal
 *
 * @brief   Initialize global shared variables used in :
 *          module:mainPpDrv
 *          dir:DxCh cpssHwInit
 *          src:cpssDxChHwInit.c
 *
 */
static GT_VOID cpssGlobalSharedDbMainPpDrvModDxChCpssHwInitDirDxChHwInitSrcDataSectionInit
(
    GT_VOID
)
{
    PRV_SHARED_HW_INIT_DIR_DXCH_HW_INIT_DXCH_HW_INIT_SRC_GLOBAL_VAR_GET(isResetDbEnabled) = GT_TRUE;
    PRV_SHARED_HW_INIT_DIR_DXCH_HW_INIT_DXCH_HW_INIT_SRC_GLOBAL_VAR_GET(microInitCpllFirstTime) = GT_TRUE;
}

/**
 * @internal cpssGlobalSharedDbMainPpDrvModDxChCpssHwInitDirDataSectionInit function
 * @endinternal
 *
 * @brief   Initialize global shared variables used in :
 *          module:mainPpDrv
 *          dir:DxCh cpssHwInit
 *
 */
static GT_VOID cpssGlobalSharedDbMainPpDrvModDxChCpssHwInitDirDataSectionInit
(
    GT_VOID
)
{
    cpssGlobalSharedDbMainPpDrvModDxChCpssHwInitDirRegV1SrcDataSectionInit();
    cpssGlobalSharedDbMainPpDrvModDxChCpssHwInitDirDxChHwInitSrcDataSectionInit();
}

/**
 * @internal
 *           cpssGlobalSharedDbMainPpDrvModDxChTcamDirDataSectionInit
 *           function
 * @endinternal
 *
 * @brief   Initialize global shared variables used in :
 *          module:mainPpDrv
 *          dir:cpssDxChxGen/TCAM
 *
 */
static GT_VOID cpssGlobalSharedDbMainPpDrvModDxChTcamDirDataSectionInit
(
    GT_VOID
)
{
    PRV_SHARED_GLOBAL_VAR_SET(mainPpDrvMod.dxChTcamDir.prvTcamWaEnableCount, 1);
    PRV_SHARED_GLOBAL_VAR_SET(mainPpDrvMod.dxChTcamDir.prvTcamWaDisableCount, 1);
}

/**
 * @internal
 *           cpssGlobalSharedDbMainPpDrvModDxChVTcamDirDataSectionInit
 *           function
 * @endinternal
 *
 * @brief   Initialize global shared variables used in :
 *          module:mainPpDrv
 *          dir:cpssDxChxGen/VTCAM
 *
 */
static GT_VOID cpssGlobalSharedDbMainPpDrvModDxChVTcamDirDataSectionInit
(
    GT_VOID
)
{
    /* currently by default priority driven prepare position algorithm is version 1 */
    PRV_SHARED_GLOBAL_VAR_SET(
        mainPpDrvMod.dxChVTcamDir.priorityAddNewRuleNeedNewLogicalIndexFuncionVersion, 1);
    /** HA debug variables */
    PRV_SHARED_GLOBAL_VAR_SET(
        mainPpDrvMod.dxChVTcamDir.haAppVtcamMngCfgParam.haSupportEnabled, GT_TRUE);
    PRV_SHARED_GLOBAL_VAR_SET(
        mainPpDrvMod.dxChVTcamDir.haAppVtcamMngCfgParam.haFeaturesEnabledBmp,
        CPSS_DXCH_VIRTUAL_TCAM_HA_FEATURE_REPLAY_FAILED_RULES_ADD_E);
}

/**
 * @internal
 *           cpssGlobalSharedDbMainPpDrvModDxChPortDirPortCtrlSrcDataSectionInit
 *           function
 * @endinternal
 *
 * @brief   Initialize global shared variables used in :
 *          module:mainPpDrv
 *          dir:cpssDxChGen/port
 *          src:cpssDxChPortCtrl.c
 *
 */
static GT_VOID cpssGlobalSharedDbMainPpDrvModDxChPortDirPortCtrlSrcDataSectionInit
(
    GT_VOID
)
{
    PRV_SHARED_PORT_DIR_PORT_CTRL_SRC_GLOBAL_VAR_INIT(aldrinDebugCpllInitFlag, GT_TRUE);
    PRV_SHARED_PORT_DIR_PORT_CTRL_SRC_GLOBAL_VAR_INIT(bc3DebugCpllInitFlag, GT_TRUE);
    PRV_SHARED_PORT_DIR_PORT_CTRL_SRC_GLOBAL_VAR_INIT(bobkDebugCpllInitFlag, GT_TRUE);
}

/**
 * @internal
 *           cpssGlobalSharedDbMainPpDrvModDxChPortDirDataSectionInit
 *           function
 * @endinternal
 *
 * @brief   Initialize global shared variables used in :
 *          module:mainPpDrv
 *          dir:cpssDxChxGen/port
 *
 */
static GT_VOID cpssGlobalSharedDbMainPpDrvModDxChPortDirDataSectionInit
(
    GT_VOID
)
{

    cpssGlobalSharedDbMainPpDrvModDxChPortDirPortCtrlSrcDataSectionInit();

    PRV_SHARED_GLOBAL_VAR_SET(
        mainPpDrvMod.dxChPortDir.portPaSrc.storedSliceId, CPSS_PA_INVALID_SLICE);

    {
        static const GT_U32 pattern_prvCpssDxChPortPizzaArbiterBWTxFifoChannelFactorArr[
            CPSS_PORT_PA_BW_MODE_LAST_E] =
        {
             /*CPSS_PORT_PA_BW_MODE_REGULAR_E*/            100
            ,/*CPSS_PORT_PA_BW_EXT_MODE_1_E*/              147
            ,/*CPSS_PORT_PA_BW_EXT_MODE_2_E*/              108
            ,/*CPSS_PORT_PA_BW_EXT_MODE_3_E*/               83
            ,/*CPSS_PORT_PA_BW_EXT_MODE_4_E*/                1
            ,/*CPSS_PORT_PA_BW_EXT_MODE_5_E*/       0xFFFFFFFF
            ,/*CPSS_PORT_PA_BW_EXT_MODE_6_E*/       0xFFFFFFFF
            ,/*CPSS_PORT_PA_BW_EXT_MODE_7_E*/       0xFFFFFFFF
            ,/*CPSS_PORT_PA_BW_EXT_MODE_8_E*/       0xFFFFFFFF
        };

        cpssOsMemCpy(
            PRV_SHARED_GLOBAL_VAR_GET(
                mainPpDrvMod.dxChPortDir.portPaSrc.prvCpssDxChPortPizzaArbiterBWTxFifoChannelFactorArr),
            pattern_prvCpssDxChPortPizzaArbiterBWTxFifoChannelFactorArr,
            sizeof(pattern_prvCpssDxChPortPizzaArbiterBWTxFifoChannelFactorArr));

    }

    {
        static const PRV_CPSS_DXCH_DEV_X_UNITDEFLIST  pattern_prv_dev_unitDefList[] =
        {
             { CPSS_PP_FAMILY_DXCH_BOBCAT2_E, CPSS_PP_SUB_FAMILY_NONE_E,         &prv_paUnitDef_All_bc2    [0]   , NULL}
            ,{ CPSS_PP_FAMILY_DXCH_BOBCAT2_E, CPSS_PP_SUB_FAMILY_BOBCAT2_BOBK_E, &prv_paUnitDef_All_bobk   [0]   , NULL}
            ,{ CPSS_PP_FAMILY_DXCH_ALDRIN_E,  CPSS_PP_SUB_FAMILY_NONE_E,         &prv_paUnitDef_All_aldrin [0]   , NULL}
            ,{ CPSS_PP_FAMILY_DXCH_AC3X_E,    CPSS_PP_SUB_FAMILY_NONE_E,         &prv_paUnitDef_All_aldrin [0]   , NULL}
            ,{ CPSS_PP_FAMILY_DXCH_BOBCAT3_E, CPSS_PP_SUB_FAMILY_NONE_E,         &prv_paUnitDef_All_bc3    [0]   , NULL}
            ,{ CPSS_PP_FAMILY_DXCH_ALDRIN2_E, CPSS_PP_SUB_FAMILY_NONE_E,         &prv_paUnitDef_All_aldrin2[0]   , NULL}

            /* not relevant to sip6 devices */

            ,{ CPSS_MAX_FAMILY,               CPSS_BAD_SUB_FAMILY,               NULL                    , NULL}
        };


        cpssOsMemCpy(
            PRV_SHARED_GLOBAL_VAR_GET(
                mainPpDrvMod.dxChPortDir.portPaSrc.prv_dev_unitDefList),
            pattern_prv_dev_unitDefList,
            sizeof(pattern_prv_dev_unitDefList));
        PRV_SHARED_GLOBAL_VAR_SET(
                mainPpDrvMod.dxChPortDir.portPaSrc.prv_dev_unitDefList[0].unitsDescrAllPtr,
                PRV_SHARED_GLOBAL_VAR_GET(mainPpDrvMod.dxChPortDir.portPaSrc.prv_unitsDescrAll_bc2));
        PRV_SHARED_GLOBAL_VAR_SET(
                mainPpDrvMod.dxChPortDir.portPaSrc.prv_dev_unitDefList[1].unitsDescrAllPtr,
                PRV_SHARED_GLOBAL_VAR_GET(mainPpDrvMod.dxChPortDir.portPaSrc.prv_unitsDescrAll_bobk));
        PRV_SHARED_GLOBAL_VAR_SET(
                mainPpDrvMod.dxChPortDir.portPaSrc.prv_dev_unitDefList[2].unitsDescrAllPtr,
                PRV_SHARED_GLOBAL_VAR_GET(mainPpDrvMod.dxChPortDir.portPaSrc.prv_unitsDescrAll_aldrin));
        PRV_SHARED_GLOBAL_VAR_SET(
                mainPpDrvMod.dxChPortDir.portPaSrc.prv_dev_unitDefList[3].unitsDescrAllPtr,
                PRV_SHARED_GLOBAL_VAR_GET(mainPpDrvMod.dxChPortDir.portPaSrc.prv_unitsDescrAll_aldrin));
        PRV_SHARED_GLOBAL_VAR_SET(
                mainPpDrvMod.dxChPortDir.portPaSrc.prv_dev_unitDefList[4].unitsDescrAllPtr,
                PRV_SHARED_GLOBAL_VAR_GET(mainPpDrvMod.dxChPortDir.portPaSrc.prv_unitsDescrAll_bc3));
        PRV_SHARED_GLOBAL_VAR_SET(
                mainPpDrvMod.dxChPortDir.portPaSrc.prv_dev_unitDefList[5].unitsDescrAllPtr,
                PRV_SHARED_GLOBAL_VAR_GET(mainPpDrvMod.dxChPortDir.portPaSrc.prv_unitsDescrAll_aldrin2));

        PRV_SHARED_GLOBAL_VAR_SET(
            mainPpDrvMod.dxChPortDir.portPaSrc.prv_paUnitsDrv.dev_x_unitDefList,
            PRV_SHARED_GLOBAL_VAR_GET(
                mainPpDrvMod.dxChPortDir.portPaSrc.prv_dev_unitDefList));
    }

    {
        static const CPSS_DXCH_PORT_SPEED_TO_SLICE_NUM_STC pattern_portSpeed2SliceNumListDefault[] =
        {     /* port speed        ,     slices N */
             {   CPSS_PORT_SPEED_10_E,        1 }
            ,{   CPSS_PORT_SPEED_100_E,       1 }
            ,{   CPSS_PORT_SPEED_1000_E,      1 }
            ,{   CPSS_PORT_SPEED_2500_E,      1 }
            ,{   CPSS_PORT_SPEED_10000_E,     1 }
            ,{   CPSS_PORT_SPEED_11800_E,     1 }
            ,{   CPSS_PORT_SPEED_12000_E,     2 }
            ,{   CPSS_PORT_SPEED_16000_E,     2 }
            ,{   CPSS_PORT_SPEED_15000_E,     2 }
            ,{   CPSS_PORT_SPEED_20000_E,     2 }
            ,{   CPSS_PORT_SPEED_40000_E,     4 }
            ,{   CPSS_PORT_SPEED_47200_E,     4 }
            ,{   CPSS_PORT_SPEED_NA_E,        CPSS_INVALID_SLICE_NUM }
        };
        GT_U32 ii;


        cpssOsMemCpy(
            PRV_SHARED_GLOBAL_VAR_GET(
                mainPpDrvMod.dxChPortDir.portPaSrc.portSpeed2SliceNumListDefault),
            pattern_portSpeed2SliceNumListDefault,
            sizeof(pattern_portSpeed2SliceNumListDefault));

        for (ii = 0; (ii < 12); ii++)
        {
            PRV_SHARED_GLOBAL_VAR_SET(
                mainPpDrvMod.dxChPortDir.portPaSrc.prvPortGroupPortSpeed2SliceNumDefault.arr[ii].portN, ii);
            PRV_SHARED_GLOBAL_VAR_SET(
                mainPpDrvMod.dxChPortDir.portPaSrc.prvPortGroupPortSpeed2SliceNumDefault.arr[ii].portSpeed2SliceNumListPtr,
                PRV_SHARED_GLOBAL_VAR_GET(
                    mainPpDrvMod.dxChPortDir.portPaSrc.portSpeed2SliceNumListDefault));
        }
        PRV_SHARED_GLOBAL_VAR_SET(
            mainPpDrvMod.dxChPortDir.portPaSrc.prvPortGroupPortSpeed2SliceNumDefault.arr[ii].portN,
            CPSS_PA_INVALID_PORT);
    }

    {
        static const CPSS_DXCH_PORT_SPEED_TO_SLICE_NUM_STC pattern_portSpeed2SliceNumListLionB0_240Mhz_7x10G[] =
        {     /* port speed        ,     slices N */
             {   CPSS_PORT_SPEED_10_E,        1 }
            ,{   CPSS_PORT_SPEED_100_E,       1 }
            ,{   CPSS_PORT_SPEED_1000_E,      1 }
            ,{   CPSS_PORT_SPEED_2500_E,      1 }
            ,{   CPSS_PORT_SPEED_10000_E,     1 }
            ,{   CPSS_PORT_SPEED_NA_E,        CPSS_INVALID_SLICE_NUM }
        };
        GT_U32 ii;

        cpssOsMemCpy(
            PRV_SHARED_GLOBAL_VAR_GET(
                mainPpDrvMod.dxChPortDir.portPaSrc.portSpeed2SliceNumListLionB0_240Mhz_7x10G),
            pattern_portSpeed2SliceNumListLionB0_240Mhz_7x10G,
            sizeof(pattern_portSpeed2SliceNumListLionB0_240Mhz_7x10G));
        for (ii = 0; (ii < 7); ii++)
        {
            PRV_SHARED_GLOBAL_VAR_SET(
                mainPpDrvMod.dxChPortDir.portPaSrc.
                prvPortGroupPortSpeed2SliceNumLionB0_240Mhz_7x10G.arr[ii].portN, ii);
            PRV_SHARED_GLOBAL_VAR_SET(
                mainPpDrvMod.dxChPortDir.portPaSrc.
                prvPortGroupPortSpeed2SliceNumLionB0_240Mhz_7x10G.arr[ii].portSpeed2SliceNumListPtr,
                PRV_SHARED_GLOBAL_VAR_GET(
                    mainPpDrvMod.dxChPortDir.portPaSrc.portSpeed2SliceNumListLionB0_240Mhz_7x10G));
        }
        PRV_SHARED_GLOBAL_VAR_SET(
            mainPpDrvMod.dxChPortDir.portPaSrc.prvPortGroupPortSpeed2SliceNumLionB0_240Mhz_7x10G.arr[ii].portN,
            CPSS_PA_INVALID_PORT);

    }

    {
        PRV_CPSS_DXCH_PORT_SERDES_POWER_STATUS_SET_FUN portSerdesPowerStatusSetFuncPtrArray[CPSS_PP_FAMILY_END_DXCH_E-CPSS_PP_FAMILY_START_DXCH_E-1]=
        {
        /* CPSS_PP_FAMILY_CHEETAH_E     */  NULL,
        /* CPSS_PP_FAMILY_CHEETAH2_E    */  NULL,
        /* CPSS_PP_FAMILY_CHEETAH3_E    */  NULL,
        /* CPSS_PP_FAMILY_DXCH_XCAT_E   */  NULL,
        /* CPSS_PP_FAMILY_DXCH_XCAT3_E  */  NULL,
        /* CPSS_PP_FAMILY_DXCH_AC5_E    */  NULL,
        /* CPSS_PP_FAMILY_DXCH_LION_E   */  NULL,
        /* CPSS_PP_FAMILY_DXCH_XCAT2_E  */  NULL,
        /* CPSS_PP_FAMILY_DXCH_LION2_E  */  lion2PortSerdesPowerStatusSet,
        /* CPSS_PP_FAMILY_DXCH_LION3_E  */  lion2PortSerdesPowerStatusSet,
        /* CPSS_PP_FAMILY_DXCH_BOBCAT2_E */ NULL
        };

        cpssOsMemCpy(
            PRV_SHARED_GLOBAL_VAR_GET(
                mainPpDrvMod.portDir.portSerdesCfgSrc.portSerdesPowerStatusSetFuncPtrArray),
            portSerdesPowerStatusSetFuncPtrArray,
            sizeof(portSerdesPowerStatusSetFuncPtrArray));
    }



    {
        CPSS_PORT_INTERFACE_MODE_ENT supportedPortsModes_XG_PORT_XG_ONLY[2] =
        {
            CPSS_PORT_INTERFACE_MODE_XGMII_E,
            CPSS_PORT_INTERFACE_MODE_NA_E
        };

        cpssOsMemCpy(
            PRV_SHARED_GLOBAL_VAR_GET(
                mainPpDrvMod.portDir.portIfModeCfgSrc.supportedPortsModes_XG_PORT_XG_ONLY),
            supportedPortsModes_XG_PORT_XG_ONLY,
            sizeof(supportedPortsModes_XG_PORT_XG_ONLY));
	}

	{

        CPSS_PORT_INTERFACE_MODE_ENT supportedPortsModes_XG_PORT_HX_QX_ONLY[4] =
        {
            CPSS_PORT_INTERFACE_MODE_SGMII_E,
            CPSS_PORT_INTERFACE_MODE_QX_E,
            CPSS_PORT_INTERFACE_MODE_HX_E,
            CPSS_PORT_INTERFACE_MODE_NA_E
        };

        cpssOsMemCpy(
            PRV_SHARED_GLOBAL_VAR_GET(
                mainPpDrvMod.portDir.portIfModeCfgSrc.supportedPortsModes_XG_PORT_HX_QX_ONLY),
            supportedPortsModes_XG_PORT_HX_QX_ONLY,
            sizeof(supportedPortsModes_XG_PORT_HX_QX_ONLY));
	}

	{
        CPSS_PORT_INTERFACE_MODE_ENT supportedPortsModes_XG_PORT_XG_HX_QX[8] =
        {
            CPSS_PORT_INTERFACE_MODE_SGMII_E,
            CPSS_PORT_INTERFACE_MODE_XGMII_E,
            CPSS_PORT_INTERFACE_MODE_1000BASE_X_E,
            CPSS_PORT_INTERFACE_MODE_QX_E,
            CPSS_PORT_INTERFACE_MODE_HX_E,
            CPSS_PORT_INTERFACE_MODE_RXAUI_E,
            CPSS_PORT_INTERFACE_MODE_100BASE_FX_E,
            CPSS_PORT_INTERFACE_MODE_NA_E
        };

        cpssOsMemCpy(
            PRV_SHARED_GLOBAL_VAR_GET(
                mainPpDrvMod.portDir.portIfModeCfgSrc.supportedPortsModes_XG_PORT_XG_HX_QX),
            supportedPortsModes_XG_PORT_XG_HX_QX,
            sizeof(supportedPortsModes_XG_PORT_XG_HX_QX));
	}

	{
        CPSS_PORT_INTERFACE_MODE_ENT supportedPortsModes_GE_PORT_GE_ONLY[9] =
        {
            CPSS_PORT_INTERFACE_MODE_REDUCED_GMII_E,
            CPSS_PORT_INTERFACE_MODE_MII_E,
            CPSS_PORT_INTERFACE_MODE_SGMII_E,

            CPSS_PORT_INTERFACE_MODE_1000BASE_X_E,
            CPSS_PORT_INTERFACE_MODE_GMII_E,
            CPSS_PORT_INTERFACE_MODE_MII_PHY_E,

            CPSS_PORT_INTERFACE_MODE_100BASE_FX_E,
            CPSS_PORT_INTERFACE_MODE_QSGMII_E,
            CPSS_PORT_INTERFACE_MODE_NA_E
        };

        cpssOsMemCpy(
            PRV_SHARED_GLOBAL_VAR_GET(
                mainPpDrvMod.portDir.portIfModeCfgSrc.supportedPortsModes_GE_PORT_GE_ONLY),
            supportedPortsModes_GE_PORT_GE_ONLY,
            sizeof(supportedPortsModes_GE_PORT_GE_ONLY));
	}

	{
        CPSS_PORT_INTERFACE_MODE_ENT supportedPortsModes_XG_PORT_XLG_SGMII[15] =  /* Lion - sip 5.15*/
        {
            CPSS_PORT_INTERFACE_MODE_SGMII_E,
            CPSS_PORT_INTERFACE_MODE_XGMII_E,

            CPSS_PORT_INTERFACE_MODE_1000BASE_X_E,
            CPSS_PORT_INTERFACE_MODE_HX_E,
            CPSS_PORT_INTERFACE_MODE_RXAUI_E,

            CPSS_PORT_INTERFACE_MODE_XLG_E,
            CPSS_PORT_INTERFACE_MODE_LOCAL_XGMII_E,
            CPSS_PORT_INTERFACE_MODE_KR_E,
            CPSS_PORT_INTERFACE_MODE_SR_LR_E,
            CPSS_PORT_INTERFACE_MODE_KR4_E,
            CPSS_PORT_INTERFACE_MODE_SR_LR4_E,
            CPSS_PORT_INTERFACE_MODE_KR2_E,
            CPSS_PORT_INTERFACE_MODE_SR_LR2_E,
            CPSS_PORT_INTERFACE_MODE_NA_E
        };

        cpssOsMemCpy(
            PRV_SHARED_GLOBAL_VAR_GET(
                mainPpDrvMod.portDir.portIfModeCfgSrc.supportedPortsModes_XG_PORT_XLG_SGMII),
            supportedPortsModes_XG_PORT_XLG_SGMII,
            sizeof(supportedPortsModes_XG_PORT_XLG_SGMII));
	}

	{
        CPSS_PORT_INTERFACE_MODE_ENT supportedPortsModes_XG_PORT_CG_SGMII[20] = /* sip 5.20 and above , flex link */
        {
            CPSS_PORT_INTERFACE_MODE_SGMII_E,
            CPSS_PORT_INTERFACE_MODE_XGMII_E,

            CPSS_PORT_INTERFACE_MODE_1000BASE_X_E,
            CPSS_PORT_INTERFACE_MODE_HX_E,
            CPSS_PORT_INTERFACE_MODE_RXAUI_E,

            CPSS_PORT_INTERFACE_MODE_LOCAL_XGMII_E,
            CPSS_PORT_INTERFACE_MODE_KR_E,
            CPSS_PORT_INTERFACE_MODE_HGL_E,
            CPSS_PORT_INTERFACE_MODE_CHGL_12_E,
            CPSS_PORT_INTERFACE_MODE_SR_LR_E,
            CPSS_PORT_INTERFACE_MODE_XHGS_E,
            CPSS_PORT_INTERFACE_MODE_XHGS_SR_E,
            CPSS_PORT_INTERFACE_MODE_KR4_E,
            CPSS_PORT_INTERFACE_MODE_SR_LR4_E,
            CPSS_PORT_INTERFACE_MODE_KR2_E,
            CPSS_PORT_INTERFACE_MODE_SR_LR2_E,
            CPSS_PORT_INTERFACE_MODE_KR8_E,
            CPSS_PORT_INTERFACE_MODE_SR_LR8_E,
            CPSS_PORT_INTERFACE_MODE_2500BASE_X_E,

            CPSS_PORT_INTERFACE_MODE_NA_E
        };

        cpssOsMemCpy(
            PRV_SHARED_GLOBAL_VAR_GET(
                mainPpDrvMod.portDir.portIfModeCfgSrc.supportedPortsModes_XG_PORT_CG_SGMII),
            supportedPortsModes_XG_PORT_CG_SGMII,
            sizeof(supportedPortsModes_XG_PORT_CG_SGMII));
	}

	{
#define g_supportedPortsModes_XG_PORT_XG_ONLY PRV_SHARED_GLOBAL_VAR_GET(mainPpDrvMod.portDir.portIfModeCfgSrc.supportedPortsModes_XG_PORT_XG_ONLY)
#define g_supportedPortsModes_XG_PORT_HX_QX_ONLY PRV_SHARED_GLOBAL_VAR_GET(mainPpDrvMod.portDir.portIfModeCfgSrc.supportedPortsModes_XG_PORT_HX_QX_ONLY)
#define g_supportedPortsModes_XG_PORT_XG_HX_QX PRV_SHARED_GLOBAL_VAR_GET(mainPpDrvMod.portDir.portIfModeCfgSrc.supportedPortsModes_XG_PORT_XG_HX_QX)
#define g_supportedPortsModes_GE_PORT_GE_ONLY PRV_SHARED_GLOBAL_VAR_GET(mainPpDrvMod.portDir.portIfModeCfgSrc.supportedPortsModes_GE_PORT_GE_ONLY)
#define g_supportedPortsModes_XG_PORT_XLG_SGMII PRV_SHARED_GLOBAL_VAR_GET(mainPpDrvMod.portDir.portIfModeCfgSrc.supportedPortsModes_XG_PORT_XLG_SGMII)
#define g_supportedPortsModes_XG_PORT_CG_SGMII PRV_SHARED_GLOBAL_VAR_GET(mainPpDrvMod.portDir.portIfModeCfgSrc.supportedPortsModes_XG_PORT_CG_SGMII)

        prvIfModeSupportedPortsModes_STC supportedPortsModesList[7] =
        {
            {  PRV_CPSS_XG_PORT_XG_ONLY_E,     &g_supportedPortsModes_XG_PORT_XG_ONLY[0]        }
           ,{  PRV_CPSS_XG_PORT_HX_QX_ONLY_E,  &g_supportedPortsModes_XG_PORT_HX_QX_ONLY[0]     }
           ,{  PRV_CPSS_XG_PORT_XG_HX_QX_E,    &g_supportedPortsModes_XG_PORT_XG_HX_QX[0]       }
           ,{  PRV_CPSS_GE_PORT_GE_ONLY_E,     &g_supportedPortsModes_GE_PORT_GE_ONLY[0]        }
           ,{  PRV_CPSS_XG_PORT_XLG_SGMII_E,   &g_supportedPortsModes_XG_PORT_XLG_SGMII[0]      }
           ,{  PRV_CPSS_XG_PORT_CG_SGMII_E,    &g_supportedPortsModes_XG_PORT_CG_SGMII[0]       }
           ,{  PRV_CPSS_XG_PORT_OPTIONS_MAX_E, (CPSS_PORT_INTERFACE_MODE_ENT *)NULL           }
        };

        cpssOsMemCpy(
            PRV_SHARED_GLOBAL_VAR_GET(
                mainPpDrvMod.portDir.portIfModeCfgSrc.supportedPortsModesList),
            supportedPortsModesList,
            sizeof(supportedPortsModesList));
    }

    PRV_SHARED_GLOBAL_VAR_SET(
        mainPpDrvMod.dxChPortDir.portPrvSrc.prv_txqPortManualCredit.portNum,
        0xFFFFFFFF);
    PRV_SHARED_GLOBAL_VAR_SET(
        mainPpDrvMod.dxChPortDir.portPrvSrc.prv_txqPortManualCredit.txqCredits,
        (CPSS_PORT_SPEED_ENT)(-1));

    PRV_SHARED_GLOBAL_VAR_SET(
        mainPpDrvMod.dxChPortDir.portPrvSrc.prv_txqPortManualCredit_BobK.portNum,
        0xFFFFFFFF);
    PRV_SHARED_GLOBAL_VAR_SET(
        mainPpDrvMod.dxChPortDir.portPrvSrc.prv_txqPortManualCredit_BobK.txqCredits,
        (CPSS_PORT_SPEED_ENT)(-1));

    PRV_SHARED_GLOBAL_VAR_SET(
        mainPpDrvMod.dxChPortDir.portPrvSrc.debug_falcon_sliceMode, 1);
}

/**
 * @internal
 *           cpssGlobalSharedDbMainPpDrvModDxChTtiDirDataSectionInit
 *           function
 * @endinternal
 *
 * @brief   Initialize global shared variables used in :
 *          module:mainPpDrv
 *          dir:cpssDxChxTti
 *
 */
static GT_VOID cpssGlobalSharedDbMainPpDrvModDxChTtiDirDataSectionInit
(
    GT_VOID
)
{
    {
        PRV_CPSS_ENTRY_FORMAT_TABLE_STC ttiLegacyKeyFieldsFormat[TTI_LEGACY_KEY_FIELDS___LAST_VALUE___E] =
        {
        /* byte 0 */
            /*TTI_LEGACY_KEY_FIELDS_PCLID_4_0_E,                                 */
            PRV_CPSS_STANDARD_FIELD_MAC(5  ),
            /*TTI_LEGACY_KEY_FIELDS_LOCAL_DEVICE_SOURCE_IS_TRUNK_E,              */
            PRV_CPSS_STANDARD_FIELD_MAC(1  ),
            /*TTI_LEGACY_KEY_FIELDS_DSA_TAG_SRCISTRUNK_E,                        */
            PRV_CPSS_STANDARD_FIELD_MAC(1  ),
            /*TTI_LEGACY_KEY_FIELDS_MAC_TO_ME_E,                                 */
            PRV_CPSS_STANDARD_FIELD_MAC(1  ),
        /* byte 1 */
            /*TTI_LEGACY_KEY_FIELDS_PCLID_9_5_E,                                 */
            PRV_CPSS_STANDARD_FIELD_MAC(5  ),
            /*TTI_LEGACY_KEY_FIELDS_SRC_COREID_E,                                */
            PRV_CPSS_STANDARD_FIELD_MAC(3  ),
        /* byte 2 */
            /*TTI_LEGACY_KEY_FIELDS_EVLAN_7_0_E,                                 */
            PRV_CPSS_STANDARD_FIELD_MAC(8  ),
        /* byte 3 */
            /*TTI_LEGACY_KEY_FIELDS_EVLAN_12_8_E,                                */
            PRV_CPSS_STANDARD_FIELD_MAC(5  ),
            /*TTI_LEGACY_KEY_FIELDS_RESERVED_0_E,                                */
            PRV_CPSS_STANDARD_FIELD_MAC(3  ),
        /* byte 4 */
            /*TTI_LEGACY_KEY_FIELDS_LOCAL_DEVICE_SOURCE_EPORT_TRUNKID_7_0_E,     */
            PRV_CPSS_STANDARD_FIELD_MAC(8  ),
        /* byte 5 */
            /*TTI_LEGACY_KEY_FIELDS_LOCAL_DEVICE_SOURCE_EPORT_TRUNKID_12_8_E,    */
            PRV_CPSS_STANDARD_FIELD_MAC(5  ),
            /*TTI_LEGACY_KEY_FIELDS_RESERVED_1_E,                                */
            PRV_CPSS_STANDARD_FIELD_MAC(3  ),
        /* byte 6 */
            /*TTI_LEGACY_KEY_FIELDS_DSA_TAG_SRCPORT_SRCEPORT_TRUNKID_7_0_E,      */
            PRV_CPSS_STANDARD_FIELD_MAC(8  ),
        /* byte 7 */
            /*TTI_LEGACY_KEY_FIELDS_DSA_TAG_SRCPORT_SRCEPORT_TRUNKID_12_8_E,     */
            PRV_CPSS_STANDARD_FIELD_MAC(5  ),
            /*TTI_LEGACY_KEY_FIELDS_RESERVED_2_E,                                */
            PRV_CPSS_STANDARD_FIELD_MAC(3  ),

            /* mim and trill and ethernet */
        /* byte 8 */
            /*TTI_LEGACY_KEY_FIELDS_UP0_E,           */
            PRV_CPSS_STANDARD_FIELD_MAC(3  ),
            /*TTI_LEGACY_KEY_FIELDS_CFI0_E,        */
            PRV_CPSS_STANDARD_FIELD_MAC(1  ),
            /*TTI_LEGACY_KEY_FIELDS_VLAN_TAG0_EXISTS_E,   */
            PRV_CPSS_STANDARD_FIELD_MAC(1  ),
            /*TTI_LEGACY_KEY_FIELDS_RESERVED_4_E,         */
            PRV_CPSS_STANDARD_FIELD_MAC(2  ),
            /*TTI_LEGACY_KEY_FIELDS_PASSENGER_OUTER_CFI_E,*/
            PRV_CPSS_STANDARD_FIELD_MAC(1  ),

        /* byte 9..14 */
            /*TTI_LEGACY_KEY_FIELDS_MAC_DA_MAC_SA_E,     */
            PRV_CPSS_STANDARD_FIELD_MAC(48  ),

        /* byte 28 */
            /*TTI_LEGACY_KEY_FIELDS_DSA_TAG_SRCDEV_7_0_E,                        */
            {(28*8),/*explicit byte 28*/
             8,
             0/*not care when startBit is explicit*/},
        /* byte 29 */
            /*TTI_LEGACY_KEY_FIELDS_DSA_TAG_SRCDEV_9_8_E,                        */
            PRV_CPSS_STANDARD_FIELD_MAC(4  ),
            /*TTI_LEGACY_KEY_FIELDS_RESERVED_3_E,                                */
            PRV_CPSS_STANDARD_FIELD_MAC(3  ),
            /*TTI_LEGACY_KEY_FIELDS_MUST_BE_1_E,                                 */
            PRV_CPSS_STANDARD_FIELD_MAC(1  ),

        /* IPV4 specific fields */
        /* byte 15..18 */
            /*TTI_LEGACY_KEY_FIELDS_IPV4_SIP_E,                                 */
            {PRV_CPSS_FIELD_SET_IN_RUNTIME_CNS,
             32,
             TTI_LEGACY_KEY_FIELDS_MAC_DA_MAC_SA_E},
        /* byte 19..22 */
            /*TTI_LEGACY_KEY_FIELDS_IPV4_DIP_E,                                 */
            PRV_CPSS_STANDARD_FIELD_MAC(32  ),
        /* byte 23 */
            /*TTI_LEGACY_KEY_FIELDS_IPV4_IS_ARP_E,                              */
            PRV_CPSS_STANDARD_FIELD_MAC(1  ),
            /*TTI_LEGACY_KEY_FIELDS_IPV4_TUNNELING_PROTOCOL_E,                  */
            PRV_CPSS_STANDARD_FIELD_MAC(3  ),

        /* MPLS specific fields */
        /* byte 15..17 */
            /*TTI_LEGACY_KEY_FIELDS_MPLS_S0_E      ,*/
            {PRV_CPSS_FIELD_SET_IN_RUNTIME_CNS,
             1,
             TTI_LEGACY_KEY_FIELDS_MAC_DA_MAC_SA_E},
            /*TTI_LEGACY_KEY_FIELDS_MPLS_EXP0_E    ,*/
            PRV_CPSS_STANDARD_FIELD_MAC(3  ),
            /*TTI_LEGACY_KEY_FIELDS_MPLS_LABEL0_E  ,*/
            PRV_CPSS_STANDARD_FIELD_MAC(20  ),

        /* byte 18..20 */
            /*TTI_LEGACY_KEY_FIELDS_MPLS_S1_E      ,*/
            PRV_CPSS_STANDARD_FIELD_MAC(1  ),
            /*TTI_LEGACY_KEY_FIELDS_MPLS_EXP1_E    ,*/
            PRV_CPSS_STANDARD_FIELD_MAC(3  ),
            /*TTI_LEGACY_KEY_FIELDS_MPLS_LABEL1_E  ,*/
            PRV_CPSS_STANDARD_FIELD_MAC(20  ),
        /* byte 21..23 */
            /*TTI_LEGACY_KEY_FIELDS_MPLS_S2_E      ,*/
            PRV_CPSS_STANDARD_FIELD_MAC(1  ),
            /*TTI_LEGACY_KEY_FIELDS_MPLS_EXP2_E    ,*/
            PRV_CPSS_STANDARD_FIELD_MAC(3  ),
            /*TTI_LEGACY_KEY_FIELDS_MPLS_LABEL2_E  ,*/
            PRV_CPSS_STANDARD_FIELD_MAC(20  ),
        /* byte 24 */
            /*TTI_LEGACY_KEY_FIELDS_MPLS_RESERVED_LABEL_VALUE_E      ,*/
            PRV_CPSS_STANDARD_FIELD_MAC(4  ),
            /*TTI_LEGACY_KEY_FIELDS_MPLS_CONTROL_WORD_CHANNEL_TYPE_PROFILE_E      ,*/
            PRV_CPSS_STANDARD_FIELD_MAC(4  ),
        /* byte 25 */
            /*TTI_LEGACY_KEY_FIELDS_MPLS_PROTOCOL_AFTER_MPLS_E      ,*/
            PRV_CPSS_STANDARD_FIELD_MAC(2  ),
            /*TTI_LEGACY_KEY_FIELDS_MPLS_RESERVED_LABEL_EXISTS_E      ,*/
            PRV_CPSS_STANDARD_FIELD_MAC(1  ),
            /*TTI_LEGACY_KEY_FIELDS_MPLS_DATA_AFTER_INNER_LABEL_E      ,*/
            PRV_CPSS_STANDARD_FIELD_MAC(5  ),


        /* MIM specific fields */
        /* byte 15..17 */
            /*TTI_LEGACY_KEY_FIELDS_MIM_I_SID_E      ,                */
            {PRV_CPSS_FIELD_SET_IN_RUNTIME_CNS,
             24,
             TTI_LEGACY_KEY_FIELDS_MAC_DA_MAC_SA_E},
        /* byte 18 */
            /*TTI_LEGACY_KEY_FIELDS_MIM_I_TAG_OCTET1_BITS_1_4_E      ,*/
            PRV_CPSS_STANDARD_FIELD_MAC(4  ),
            /*TTI_LEGACY_KEY_FIELDS_MIM_I_DP_E      ,                 */
            PRV_CPSS_STANDARD_FIELD_MAC(1  ),
            /*TTI_LEGACY_KEY_FIELDS_MIM_I_UP_E      ,                 */
            PRV_CPSS_STANDARD_FIELD_MAC(3  ),

        /* byte 19..20*/
            /*TTI_LEGACY_KEY_FIELDS_MIM_PASSENGER_S_TAG_VID_E      ,   */
            PRV_CPSS_STANDARD_FIELD_MAC(12  ),
            /*TTI_LEGACY_KEY_FIELDS_MIM_PASSENGER_S_TAG_UP_E      ,    */
            PRV_CPSS_STANDARD_FIELD_MAC(3  ),
            /*TTI_LEGACY_KEY_FIELDS_MIM_PASSENGER_S_TAG_EXISTS_E      ,*/
            PRV_CPSS_STANDARD_FIELD_MAC(1  ),

        /* ethernet specific fields */
        /* byte 15..16 */
            /*TTI_LEGACY_KEY_FIELDS_ETHERNET_VLAN1_E      ,               */
            {PRV_CPSS_FIELD_SET_IN_RUNTIME_CNS,
             12,
             TTI_LEGACY_KEY_FIELDS_MAC_DA_MAC_SA_E},
            /*TTI_LEGACY_KEY_FIELDS_ETHERNET_UP1_E      ,                 */
            PRV_CPSS_STANDARD_FIELD_MAC(3  ),
            /*TTI_LEGACY_KEY_FIELDS_ETHERNET_CFI1_E      ,                */
            PRV_CPSS_STANDARD_FIELD_MAC(1  ),
        /* byte 17..18 */
            /*TTI_LEGACY_KEY_FIELDS_ETHERNET_ETHERTYPE_E      ,           */
            PRV_CPSS_STANDARD_FIELD_MAC(16  ),
        /* byte 19 */
            /*TTI_LEGACY_KEY_FIELDS_ETHERNET_VLAN_TAG1_EXISTS_E      ,    */
            PRV_CPSS_STANDARD_FIELD_MAC(1  ),
            /*TTI_LEGACY_KEY_FIELDS_ETHERNET_TAG0_TPID_INDEX_E      ,     */
            PRV_CPSS_STANDARD_FIELD_MAC(3  ),
            /*TTI_LEGACY_KEY_FIELDS_ETHERNET_TAG1_TPID_INDEX_E      ,     */
            PRV_CPSS_STANDARD_FIELD_MAC(3  ),
            /*TTI_LEGACY_KEY_FIELDS_ETHERNET_RESERVED_E      ,            */
            PRV_CPSS_STANDARD_FIELD_MAC(1  ),
        /* byte 20 */
            /*TTI_LEGACY_KEY_FIELDS_ETHERNET_DSA_QOS_PROFILE_E      ,     */
            PRV_CPSS_STANDARD_FIELD_MAC(7  ),
            /*TTI_LEGACY_KEY_FIELDS_ETHERNET_RESERVED1_E      ,            */
            PRV_CPSS_STANDARD_FIELD_MAC(1  ),
        /* byte 21..22 */
            /*TTI_LEGACY_KEY_FIELDS_ETHERNET_DSA_SOURCE_ID_11_0_E      ,  */
            PRV_CPSS_STANDARD_FIELD_MAC(12  )
        };

        cpssOsMemCpy(
            PRV_SHARED_GLOBAL_VAR_GET(
                mainPpDrvMod.dxchTtiDir.ttiSrc.ttiLegacyKeyFieldsFormat),
            ttiLegacyKeyFieldsFormat,
            sizeof(ttiLegacyKeyFieldsFormat));

    }

    {
        PRV_CPSS_ENTRY_FORMAT_TABLE_STC ttiMetaDataFieldsFormat[TTI_META_DATA_FIELDS___LAST_VALUE___E] =
        { /* byte 0 */
            /*TTI_META_DATA_FIELDS_IS_ARP_E,                                    */
            PRV_CPSS_STANDARD_FIELD_MAC(1  ),
            /*TTI_META_DATA_FIELDS_IPV4_TUNNELING_PROTOCOL_E,                   */
            PRV_CPSS_STANDARD_FIELD_MAC(3  ),
            /*TTI_META_DATA_FIELDS_RESERVED_0_E,                                */
            PRV_CPSS_STANDARD_FIELD_MAC(4  ),
        /* byte 1..4 */
            /*TTI_META_DATA_FIELDS_IPV4_ARP_DIP_E,                              */
            PRV_CPSS_STANDARD_FIELD_MAC(32 ),
        /* byte 5..8 */
            /*TTI_META_DATA_FIELDS_IPV4_ARP_SIP_E,                              */
            PRV_CPSS_STANDARD_FIELD_MAC(32 ),
        /* byte 9 */
            /*TTI_META_DATA_FIELDS_PROTOCOL_ABOVE_MPLS_E,                       */
            PRV_CPSS_STANDARD_FIELD_MAC(2  ),
            /*TTI_META_DATA_FIELDS_RESERVED_LABEL_EXISTS_E,                     */
            PRV_CPSS_STANDARD_FIELD_MAC(1  ),
            /*TTI_META_DATA_FIELDS_DATA_AFTER_INNER_LABEL_E,                    */
            PRV_CPSS_STANDARD_FIELD_MAC(5  ),
        /* byte 10 */
            /*TTI_META_DATA_FIELDS_RESERVED_LABEL_VALUE_E,                      */
            PRV_CPSS_STANDARD_FIELD_MAC(4  ),
            /*TTI_META_DATA_FIELDS_CONTROL_WORD_CHANNEL_TYPE_PROFILE_E,         */
            PRV_CPSS_STANDARD_FIELD_MAC(4  ),
        /* byte 11 */
            /*TTI_META_DATA_FIELDS_UP0_B_UP_E,                                  */
            PRV_CPSS_STANDARD_FIELD_MAC(3  ),
            /*TTI_META_DATA_FIELDS_CFI0_B_DEI__E,                               */
            PRV_CPSS_STANDARD_FIELD_MAC(1  ),
            /*TTI_META_DATA_FIELDS_VLAN_TAG0_EXISTS_E,                          */
            PRV_CPSS_STANDARD_FIELD_MAC(1  ),
            /*TTI_META_DATA_FIELDS_RESERVED_1_E,                                */
            PRV_CPSS_STANDARD_FIELD_MAC(2  ),
            /*TTI_META_DATA_FIELDS_PASSENGER_OUTER_CFI_E,                       */
            PRV_CPSS_STANDARD_FIELD_MAC(1  ),
        /* byte 12 */
            /*TTI_META_DATA_FIELDS_PASSENGER_OUTER_TAG_VID_7_0_E,               */
            PRV_CPSS_STANDARD_FIELD_MAC(8  ),
        /* byte 13 */
            /*TTI_META_DATA_FIELDS_PASSENGER_OUTER_TAG_VID_11_8_E,              */
            PRV_CPSS_STANDARD_FIELD_MAC(4  ),
            /*TTI_META_DATA_FIELDS_PASSENGER_OUTER_TAG_UP_E,                    */
            PRV_CPSS_STANDARD_FIELD_MAC(3  ),
            /*TTI_META_DATA_FIELDS_PASSENGER_OUTER_TAG_EXIST_E,                 */
            PRV_CPSS_STANDARD_FIELD_MAC(1  ),
        /* byte 14 */
            /*TTI_META_DATA_FIELDS_VLAN1_7_0_E,                                 */
            PRV_CPSS_STANDARD_FIELD_MAC(8  ),
        /* byte 15 */
            /*TTI_META_DATA_FIELDS_VLAN1_11_8_E,                                */
            PRV_CPSS_STANDARD_FIELD_MAC(4  ),
            /*TTI_META_DATA_FIELDS_UP1_E,                                       */
            PRV_CPSS_STANDARD_FIELD_MAC(3  ),
            /*TTI_META_DATA_FIELDS_CFI1_E,                                      */
            PRV_CPSS_STANDARD_FIELD_MAC(1  ),
        /* byte 16 */
            /*TTI_META_DATA_FIELDS_VLAN1_EXIST_E,                               */
            PRV_CPSS_STANDARD_FIELD_MAC(1  ),
            /*TTI_META_DATA_FIELDS_TAG0_TPID_INDEX_E,                           */
            PRV_CPSS_STANDARD_FIELD_MAC(3  ),
            /*TTI_META_DATA_FIELDS_TAG1_TPID_INDEX_E,                           */
            PRV_CPSS_STANDARD_FIELD_MAC(3  ),
            /*TTI_META_DATA_FIELDS_RESERVED_2_E,                                */
            PRV_CPSS_STANDARD_FIELD_MAC(1  ),
        /* byte 17 */
            /*TTI_META_DATA_FIELDS_DSA_QOS_PROFILE_E,                           */
            PRV_CPSS_STANDARD_FIELD_MAC(7  ),
            /*TTI_META_DATA_FIELDS_RESERVED_3_E,                                */
            PRV_CPSS_STANDARD_FIELD_MAC(1  ),
        /* byte 18 */
            /*TTI_META_DATA_FIELDS_DSA_SOURCE_ID__7_0_E,                        */
            PRV_CPSS_STANDARD_FIELD_MAC(8  ),
        /* byte 19 */
            /*TTI_META_DATA_FIELDS_DSA_SOURCE_ID__11_8_E,                       */
            PRV_CPSS_STANDARD_FIELD_MAC(4  ),
            /*TTI_META_DATA_FIELDS_NUMBER_OF_MPLS_LABELS_14_12_E,               */
            PRV_CPSS_STANDARD_FIELD_MAC(3  ),
            /*TTI_META_DATA_FIELDS_RESERVED_4_E,                                */
            PRV_CPSS_STANDARD_FIELD_MAC(1  ),
        /* byte 20 */
            /*TTI_META_DATA_FIELDS_TRILL_OPTIONS_1ST_NIBBLE_E,                  */
            PRV_CPSS_STANDARD_FIELD_MAC(4  ),
            /*TTI_META_DATA_FIELDS_TRILL_OPTIONS_EXIST_E,                       */
            PRV_CPSS_STANDARD_FIELD_MAC(1  ),
            /*TTI_META_DATA_FIELDS_TRILL_M_BIT_E,                               */
            PRV_CPSS_STANDARD_FIELD_MAC(1  ),
            /*TTI_META_DATA_FIELDS_TRILL_OPTION_LENGTH_EXCEEDED_E,              */
            PRV_CPSS_STANDARD_FIELD_MAC(1  ),
            /*TTI_META_DATA_FIELDS_REP_LAST_E,                                  */
            PRV_CPSS_STANDARD_FIELD_MAC(1  ),
        /* byte 21 */
            /*TTI_META_DATA_FIELDS_IPV6_TUNNELING_PROTOCOL_E,                   */
            PRV_CPSS_STANDARD_FIELD_MAC(3  ),
            /*TTI_META_DATA_FIELDS_L3_DATA_WORD0_FIRST_NIBBLE_E,                */
            PRV_CPSS_STANDARD_FIELD_MAC(4  ),
            /*TTI_META_DATA_FIELDS_IPV6_L4_VALID_E,                             */
            PRV_CPSS_STANDARD_FIELD_MAC(1  ),
        /* byte 22 */
            /*TTI_META_DATA_FIELDS_PCLID_4_0_E,                                 */
            PRV_CPSS_STANDARD_FIELD_MAC(5  ),
            /*TTI_META_DATA_FIELDS_LOCAL_DEVICE_SOURCE_IS_TRUNK_E,              */
            PRV_CPSS_STANDARD_FIELD_MAC(1  ),
            /*TTI_META_DATA_FIELDS_DSA_TAG_SRCISTRUNK_E,                        */
            PRV_CPSS_STANDARD_FIELD_MAC(1  ),
            /*TTI_META_DATA_FIELDS_MAC_TO_ME_E,                                 */
            PRV_CPSS_STANDARD_FIELD_MAC(1  ),
        /* byte 23 */
            /*TTI_META_DATA_FIELDS_PCLID_9_5_E,                                 */
            PRV_CPSS_STANDARD_FIELD_MAC(5  ),
            /*TTI_META_DATA_FIELDS_SRC_COREID_E,                                */
            PRV_CPSS_STANDARD_FIELD_MAC(3  ),
        /* byte 24 */
            /*TTI_META_DATA_FIELDS_EVLAN_7_0_E,                                 */
            PRV_CPSS_STANDARD_FIELD_MAC(8  ),
        /* byte 25 */
            /*TTI_META_DATA_FIELDS_EVLAN_12_8_E,                                */
            PRV_CPSS_STANDARD_FIELD_MAC(5  ),
            /*TTI_META_DATA_FIELDS_RESERVED_5_E,                                */
            PRV_CPSS_STANDARD_FIELD_MAC(3  ),
        /* byte 26 */
            /*TTI_META_DATA_FIELDS_LOCAL_DEVICE_SOURCE_EPORT_TRUNKID_7_0_E,     */
            PRV_CPSS_STANDARD_FIELD_MAC(8  ),
        /* byte 27 */
            /*TTI_META_DATA_FIELDS_LOCAL_DEVICE_SOURCE_EPORT_TRUNKID_12_8_E,    */
            PRV_CPSS_STANDARD_FIELD_MAC(5  ),
            /*TTI_META_DATA_FIELDS_RESERVED_6_E,                                */
            PRV_CPSS_STANDARD_FIELD_MAC(3  ),
        /* byte 28 */
            /*TTI_META_DATA_FIELDS_DSA_TAG_SRCPORT_SRCEPORT_TRUNKID_7_0_E,      */
            PRV_CPSS_STANDARD_FIELD_MAC(8  ),
        /* byte 29 */
            /*TTI_META_DATA_FIELDS_DSA_TAG_SRCPORT_SRCEPORT_TRUNKID_12_8_E,     */
            PRV_CPSS_STANDARD_FIELD_MAC(5  ),
            /*TTI_META_DATA_FIELDS_RESERVED_7_E,                                */
            PRV_CPSS_STANDARD_FIELD_MAC(3  ),
        /* byte 30 */
            /*TTI_META_DATA_FIELDS_DSA_TAG_SRCDEV_7_0_E,                        */
            PRV_CPSS_STANDARD_FIELD_MAC(8  ),
        /* byte 31 */
            /*TTI_META_DATA_FIELDS_DSA_TAG_SRCDEV_9_8_E,                        */
            PRV_CPSS_STANDARD_FIELD_MAC(4  ),
            /*TTI_META_DATA_FIELDS_RESERVED_8_E,                                */
            PRV_CPSS_STANDARD_FIELD_MAC(3  ),
            /*TTI_META_DATA_FIELDS_MUST_BE_1_E,                                 */
            PRV_CPSS_STANDARD_FIELD_MAC(1  ),
            /*TTI_META_DATA_FIELDS_SIP5_LAST_VALUE_E*/
            PRV_CPSS_STANDARD_FIELD_MAC(0  ),
        /* SIP6 META_DATA_FIELDS */
        /* byte 32..33 */
            /*TTI_META_DATA_FIELDS_RESERVED_9_E,                                */
            PRV_CPSS_STANDARD_FIELD_MAC(16  ),
        /* byte 34..35*/
            /*TTI_META_DATA_FIELDS_SRC_PHY_PORT_7_0_E,                          */
            PRV_CPSS_STANDARD_FIELD_MAC(8  ),
            /*TTI_META_DATA_FIELDS_SRC_PHY_PORT_9_8_E*/
            PRV_CPSS_STANDARD_FIELD_MAC(2  ),
             /*TTI_META_DATA_FIELDS_PREEMPTED_E*/
            PRV_CPSS_STANDARD_FIELD_MAC(1  ),
            /*TTI_META_DATA_FIELDS_RESERVED_10_E*/
            PRV_CPSS_STANDARD_FIELD_MAC(5  ),
        /* byte 36 */
            /*TTI_META_DATA_FIELDS_IPVX_PROTOCOL_E,                             */
            PRV_CPSS_STANDARD_FIELD_MAC(8  ),
        /* byte 37 */
            /*TTI_META_DATA_FIELDS_IP2ME_MATCH_FOUND_E,                         */
            PRV_CPSS_STANDARD_FIELD_MAC(1  ),
            /*TTI_META_DATA_FIELDS_IP2ME_MATCH_INDEX_E,                         */
            PRV_CPSS_STANDARD_FIELD_MAC(7  )
        };

        cpssOsMemCpy(
            PRV_SHARED_GLOBAL_VAR_GET(
                mainPpDrvMod.dxchTtiDir.ttiSrc.ttiMetaDataFieldsFormat),
            ttiMetaDataFieldsFormat,
            sizeof(ttiMetaDataFieldsFormat));

    }

}

#endif

/**
 * @internal cpssGlobalSharedDbMainPpDrvModDataSectionInit function
 * @endinternal
 *
 * @brief  Initialize global shared variables used in :
 *         module:mainPpDrv
 *
 */
GT_VOID cpssGlobalSharedDbMainPpDrvModDataSectionInit
(
    GT_VOID
)
{
    /*add here directory  initializers*/
    cpssGlobalSharedDbMainPpDrvModHwInitDirDataSectionInit();
    cpssGlobalSharedDbMainPpDrvModDiagDirGenDiagSrcDataSectionInit();

#ifdef CHX_FAMILY
    cpssGlobalSharedDbMainPpDrvModIpLpmDirDataSectionInit();
    cpssGlobalSharedDbMainPpDrvModTxqDbgDirDataSectionInit();
    cpssGlobalSharedDbMainPpDrvModDxChCpssHwInitDirDataSectionInit();
    cpssGlobalSharedDbMainPpDrvModDxChTrunkDirDataSectionInit();
    cpssGlobalSharedDbMainPpDrvModDxChConfigDirDataSectionInit();
    cpssGlobalSharedDbMainPpDrvModDxChTcamDirDataSectionInit();
    cpssGlobalSharedDbMainPpDrvModDxChVTcamDirDataSectionInit();
    cpssGlobalSharedDbMainPpDrvModDxChPortDirDataSectionInit();
    cpssGlobalSharedDbMainPpDrvModDxChTtiDirDataSectionInit();
#endif
}
