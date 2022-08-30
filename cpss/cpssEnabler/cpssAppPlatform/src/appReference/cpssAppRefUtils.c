/*******************************************************************************
*              (c), Copyright 2018, Marvell International Ltd.                 *
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
* @file cpssAppRefUtils.c
*
* @brief .
*
* @version   1
********************************************************************************
*/

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
    #include <gtOs/gtOsExc.h>
    #include <gtExtDrv/drivers/gtDmaDrv.h>
#ifdef __cplusplus
}
#endif /* __cplusplus */

#ifdef INCLUDE_UTF
#include <utf/utfMain.h>
#endif
#include <appReference/cpssAppRefUtils.h>
#include <cmdShell/cmdDb/cmdBase.h>
#include <cmdShell/common/cmdCommon.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortManager.h>
#include <cpssAppPlatformPortInit.h>
#include <cpss/dxCh/dxChxGen/virtualTcam/private/prvCpssDxChVirtualTcamDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>



/* replace cpssDxChPortModeSpeedSet when working with port manager */
typedef GT_STATUS (*appDemoDxChPortMgrPortModeSpeedSet_TYPE)
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  GT_BOOL                         powerUp,
    IN  CPSS_PORT_INTERFACE_MODE_ENT    ifMode,
    IN  CPSS_PORT_SPEED_ENT             speed
);

extern appDemoDxChPortMgrPortModeSpeedSet_TYPE  appDemoDxChPortMgrPortModeSpeedSet_func;

extern GT_STATUS appPlatformDxAldrin2TailDropDbaEnableConfigure
(
    IN  GT_U8     dev,
    IN  GT_BOOL   enable
);

/* fatal error handling type */
static CPSS_ENABLER_FATAL_ERROR_TYPE prvAppRefFatalErrorType = CPSS_ENABLER_FATAL_ERROR_EXECUTE_TYPE_E;

extern GT_U16   prvCpssDrvDebugDeviceId[PRV_CPSS_MAX_PP_DEVICES_CNS];
static GT_BOOL  devSupportSystemReset_HwSoftReset[PRV_CPSS_MAX_PP_DEVICES_CNS];

#ifdef CHX_FAMILY

/* Event Counter increment function.*/
DXCH_DATA_INTEGRITY_EVENT_COUNTER_INCREMENT_FUNC *dxChDataIntegrityEventIncrementFunc = NULL;
#endif

GT_BOOL  useDebugDeviceId = GT_FALSE;

static GT_BOOL appRefTtiTcamUseAppdemoOffset = GT_TRUE;
static GT_BOOL appRefPclTcamUseIndexConversion = GT_TRUE;

/* base offset for PCL client in TCAM */
static GT_U32 appRefTcamPclRuleBaseIndexOffset = 0;

static GT_U32 appRefTcamIpcl0RuleBaseIndexOffset = 0;
static GT_U32 appRefTcamIpcl1RuleBaseIndexOffset = 0;
static GT_U32 appRefTcamIpcl2RuleBaseIndexOffset = 0;
static GT_U32 appRefTcamEpclRuleBaseIndexOffset = 0;
/* number of rules that can be used per PLC clients */
static GT_U32 appRefTcamIpcl0MaxNum = 0;
static GT_U32 appRefTcamIpcl1MaxNum = 0;
static GT_U32 appRefTcamIpcl2MaxNum = 0;
static GT_U32 appRefTcamEpclMaxNum  = 0;

#define  HARD_WIRE_TCAM_MAC(dev) \
    ((PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_BOBCAT3_E) ? 1 : 0)

/* base offset for TTI client in TCAM */
static GT_U32 appRefTcamTtiHit0RuleBaseIndexOffset;
static GT_U32 appRefTcamTtiHit1RuleBaseIndexOffset;
static GT_U32 appRefTcamTtiHit2RuleBaseIndexOffset;
static GT_U32 appRefTcamTtiHit3RuleBaseIndexOffset;
/* number of rules that can be used per lookup of TTI */
static GT_U32 appRefTcamTtiHit0MaxNum = 0;
static GT_U32 appRefTcamTtiHit1MaxNum = 0;
static GT_U32 appRefTcamTtiHit2MaxNum = 0;
static GT_U32 appRefTcamTtiHit3MaxNum = 0;

static GT_U32  save_appRefTcamTtiHit0RuleBaseIndexOffset;
static GT_U32  save_appRefTcamTtiHit1RuleBaseIndexOffset;
static GT_U32  save_appRefTcamTtiHit2RuleBaseIndexOffset;
static GT_U32  save_appRefTcamTtiHit3RuleBaseIndexOffset;

static GT_U32  save_appRefTcamTtiHit0MaxNum;
static GT_U32  save_appRefTcamTtiHit1MaxNum;
static GT_U32  save_appRefTcamTtiHit2MaxNum;
static GT_U32  save_appRefTcamTtiHit3MaxNum;

static GT_U32 save_appRefTcamIpcl0RuleBaseIndexOffset = 0;
static GT_U32 save_appRefTcamIpcl1RuleBaseIndexOffset = 0;
static GT_U32 save_appRefTcamIpcl2RuleBaseIndexOffset = 0;
static GT_U32 save_appRefTcamEpclRuleBaseIndexOffset = 0;

static GT_U32 save_appRefTcamIpcl0MaxNum = 0;
static GT_U32 save_appRefTcamIpcl1MaxNum = 0;
static GT_U32 save_appRefTcamIpcl2MaxNum = 0;
static GT_U32 save_appRefTcamEpclMaxNum  = 0;

#define CMD_DXCH_DIAG_DATA_INTEGRITY_BANK_NUM_CNS 2
#define CMD_DXCH_DIAG_DATA_INTEGRITY_MPPM_NUM_CNS 2
#define CMD_DXCH_DIAG_DATA_INTEGRITY_MPPM_DB_SIZE_CNS (CMD_DXCH_DIAG_DATA_INTEGRITY_BANK_NUM_CNS*CMD_DXCH_DIAG_DATA_INTEGRITY_MPPM_NUM_CNS*8*2/*Single and Multiple*/)

#define CMD_DXCH_DIAG_DATA_INTEGRITY_TCAM_DB_SIZE_CNS (2/*IP/PCL*/ * 2/*X/Y*/ * 8/*potgGroup*/ * 16384/*arrayAddress*/ * 4/*rules in line*/)

typedef struct
{
    GT_U16 eventCounterArr[BIT_17];
    GT_U16 memTypeArr[BIT_17];
    GT_U8  origPortGroupId[BIT_17];
} CMD_DXCH_DIAG_DATA_INTEGRITY_COUNTER_DB_STC;

typedef struct
{
    GT_U16 eventCounterArr[CMD_DXCH_DIAG_DATA_INTEGRITY_MPPM_DB_SIZE_CNS];
    GT_U8  origPortGroupId[CMD_DXCH_DIAG_DATA_INTEGRITY_MPPM_DB_SIZE_CNS];
} CMD_DXCH_DIAG_DATA_INTEGRITY_MPPM_COUNTER_DB_STC;

typedef struct
{
    GT_U32 eventCounterArr[CMD_DXCH_DIAG_DATA_INTEGRITY_TCAM_DB_SIZE_CNS];
} CMD_DXCH_DIAG_DATA_INTEGRITY_TCAM_COUNTER_DB_STC;

/**
* @enum CMD_DXCH_DIAG_DATA_INTEGRITY_EVENT_TYPE_FILTER_ENT
 *
 * @brief This enum defines error cause type display filter
*/
typedef enum{

    /** get all events. */
    CMD_DXCH_DIAG_DATA_INTEGRITY_EVENT_TYPE_FILTER_ALL_E,

    /** get single ECC events. */
    CMD_DXCH_DIAG_DATA_INTEGRITY_EVENT_TYPE_FILTER_SINGLE_ECC_E,

    /** get multiple ECC events. */
    CMD_DXCH_DIAG_DATA_INTEGRITY_EVENT_TYPE_FILTER_MULTIPLE_ECC_E,

    /** get parity events. */
    CMD_DXCH_DIAG_DATA_INTEGRITY_EVENT_TYPE_FILTER_PARITY_E

} CMD_DXCH_DIAG_DATA_INTEGRITY_EVENT_TYPE_FILTER_ENT;


extern GT_STATUS prvCpssDxChFdbManagerAllDbDelete
(
    GT_VOID
);

extern GT_STATUS prvCpssDxChExactMatchManagerAllDbDelete
(
    GT_VOID
);



static CMD_DXCH_DIAG_DATA_INTEGRITY_COUNTER_DB_STC *cmdDataIntegrityEventsDbPtr = NULL;
static CMD_DXCH_DIAG_DATA_INTEGRITY_MPPM_COUNTER_DB_STC *cmdDataIntegrityMppmEventsDbPtr = NULL;
static CMD_DXCH_DIAG_DATA_INTEGRITY_TCAM_COUNTER_DB_STC *cmdDataIntegrityTcamEventsDbPtr = NULL;
static GT_BOOL isDataIntegrityInitDone = GT_FALSE;

GT_STATUS cpssDevSupportSystemResetSet(

    IN  GT_U8       devNum,
    IN  GT_BOOL     enable
)
{
   if(PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[devNum] == NULL)
   {
        return GT_NOT_FOUND;
   }

   devSupportSystemReset_HwSoftReset[devNum] = enable;
   return GT_OK;
}

GT_STATUS cpssResetSystem (

    IN  GT_BOOL     doHwReset
)
{
    GT_STATUS rc = GT_OK;
    GT_U8   devNum;
    GT_BOOL pmTaskDeleted = GT_FALSE;



    /* Reset Exact Match Manager SW Db */
    rc = prvCpssDxChExactMatchManagerAllDbDelete();
    if(rc != GT_OK)
    {
        return rc;
    }

    for (devNum = 0; devNum < PRV_CPSS_MAX_PP_DEVICES_CNS; devNum++)
    {
        if(PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[devNum] == NULL)
        {
            continue;
        }

        if(devSupportSystemReset_HwSoftReset[devNum] == GT_FALSE)
        {
            return GT_NOT_SUPPORTED;
        }

        if(GT_FALSE==pmTaskDeleted)
        {
            /*Kill PM task*/
            rc = cpssAppPlatformPmTaskDelete();
            if(rc != GT_OK)
            {
                 return rc;
            }

            pmTaskDeleted = GT_TRUE;
        }


        rc = cpssAppPlatformPpRemove(devNum,
                                     doHwReset == GT_TRUE ?
                                     CPSS_APP_PLATFORM_PP_REMOVAL_RESET_E:
                                     CPSS_APP_PLATFORM_PP_REMOVAL_UNMANAGED_E);
        if(rc != GT_OK)
        {
            return rc;
        }
    }


    rc = prvCpssDxChVirtualTcamLibraryShutdown();
    if (rc != GT_OK)
    {
        return rc;
    }

    /* Reset FDB Manager SW Db */
    rc = prvCpssDxChFdbManagerAllDbDelete();
    if(rc != GT_OK)
    {
        return rc;
    }

    return GT_OK;
}

/* debug flag to open trace of events */
GT_U32 appRefDxChTraceEvents = 0;

GT_U32 currentDbKeyAppRef = 0;
GT_U32 currentMppmDbKeyAppRef = 0;
GT_U32 currentTcamDbKeyAppRef = 0;
GT_U32 endDbKeyAppRef = 0x20000; /* DB size */

GT_STATUS appRefTcamLibParamsSet()
{
    GT_STATUS rc = GT_OK;
    CPSS_APP_PLATFORM_TCAM_LIB_INFO_STC tcamInfo;
    rc = cpssAppPlatformTcamLibParamsGet(&tcamInfo);
    if(rc != GT_OK)
    {
        return rc;
    }
    appRefTtiTcamUseAppdemoOffset  = tcamInfo.ttiTcamUseOffset ;
    appRefPclTcamUseIndexConversion  = tcamInfo.pclTcamUseIndexConversion ;
    appRefTcamPclRuleBaseIndexOffset  = tcamInfo.tcamPclRuleBaseIndexOffset ;
    appRefTcamIpcl0RuleBaseIndexOffset  = tcamInfo.tcamIpcl0RuleBaseIndexOffset ;
    appRefTcamIpcl1RuleBaseIndexOffset  = tcamInfo.tcamIpcl1RuleBaseIndexOffset ;
    appRefTcamIpcl2RuleBaseIndexOffset  = tcamInfo.tcamIpcl2RuleBaseIndexOffset ;
    appRefTcamEpclRuleBaseIndexOffset  = tcamInfo.tcamEpclRuleBaseIndexOffset ;
    appRefTcamIpcl0MaxNum  = tcamInfo.tcamIpcl0MaxNum ;
    appRefTcamIpcl1MaxNum  = tcamInfo.tcamIpcl1MaxNum ;
    appRefTcamIpcl2MaxNum  = tcamInfo.tcamIpcl2MaxNum ;
    appRefTcamEpclMaxNum   = tcamInfo.tcamEpclMaxNum  ;
    appRefTcamTtiHit0RuleBaseIndexOffset = tcamInfo.tcamTtiHit0RuleBaseIndexOffset;
    appRefTcamTtiHit1RuleBaseIndexOffset = tcamInfo.tcamTtiHit1RuleBaseIndexOffset;
    appRefTcamTtiHit2RuleBaseIndexOffset = tcamInfo.tcamTtiHit2RuleBaseIndexOffset;
    appRefTcamTtiHit3RuleBaseIndexOffset = tcamInfo.tcamTtiHit3RuleBaseIndexOffset;
    appRefTcamTtiHit0MaxNum  = tcamInfo.tcamTtiHit0MaxNum ;
    appRefTcamTtiHit1MaxNum  = tcamInfo.tcamTtiHit1MaxNum ;
    appRefTcamTtiHit2MaxNum  = tcamInfo.tcamTtiHit2MaxNum ;
    appRefTcamTtiHit3MaxNum  = tcamInfo.tcamTtiHit3MaxNum ;
    save_appRefTcamTtiHit0RuleBaseIndexOffset = tcamInfo.save_tcamTtiHit0RuleBaseIndexOffset;
    save_appRefTcamTtiHit1RuleBaseIndexOffset = tcamInfo.save_tcamTtiHit1RuleBaseIndexOffset;
    save_appRefTcamTtiHit2RuleBaseIndexOffset = tcamInfo.save_tcamTtiHit2RuleBaseIndexOffset;
    save_appRefTcamTtiHit3RuleBaseIndexOffset = tcamInfo.save_tcamTtiHit3RuleBaseIndexOffset;
    save_appRefTcamTtiHit0MaxNum = tcamInfo.save_tcamTtiHit0MaxNum;
    save_appRefTcamTtiHit1MaxNum = tcamInfo.save_tcamTtiHit1MaxNum;
    save_appRefTcamTtiHit2MaxNum = tcamInfo.save_tcamTtiHit2MaxNum;
    save_appRefTcamTtiHit3MaxNum = tcamInfo.save_tcamTtiHit3MaxNum;
    save_appRefTcamIpcl0RuleBaseIndexOffset = tcamInfo.save_tcamIpcl0RuleBaseIndexOffset;
    save_appRefTcamIpcl1RuleBaseIndexOffset = tcamInfo.save_tcamIpcl1RuleBaseIndexOffset;
    save_appRefTcamIpcl2RuleBaseIndexOffset = tcamInfo.save_tcamIpcl2RuleBaseIndexOffset;
    save_appRefTcamEpclRuleBaseIndexOffset = tcamInfo.save_tcamEpclRuleBaseIndexOffset;
    save_appRefTcamIpcl0MaxNum = tcamInfo.save_tcamIpcl0MaxNum;
    save_appRefTcamIpcl1MaxNum = tcamInfo.save_tcamIpcl1MaxNum;
    save_appRefTcamIpcl2MaxNum = tcamInfo.save_tcamIpcl2MaxNum;
    save_appRefTcamEpclMaxNum  = tcamInfo.save_tcamEpclMaxNum;

    return rc;
}

/**
* @internal appRefDebugDeviceIdReset function
* @endinternal
*
* @brief   Reverts to HW device ID value for all devices.
*         Should be called before cpssInitSystem().
*
* @retval GT_OK                    - on success
*/
GT_STATUS appRefDebugDeviceIdReset
(
    IN GT_VOID
)
{
    cpssOsMemSet(prvCpssDrvDebugDeviceId, 0, sizeof(prvCpssDrvDebugDeviceId));
    useDebugDeviceId = GT_FALSE;

    return GT_OK;
}

/**
* @internal appRefDxChPhase1ParamsGet function
* @endinternal
*
* @brief   Gets the parameters used during phase 1 init.
*
* @param[out] ppPhase1ParamsPtr        - (pointer to) parameters used during phase 1 init.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS appRefDxChPhase1ParamsGet
(
    OUT CPSS_DXCH_PP_PHASE1_INIT_INFO_STC *ppPhase1ParamsPtr
)
{
    CPSS_DXCH_PP_PHASE1_INIT_INFO_STC cpssPpPhase1 = {0};
    GT_U8 devNum = 0;
    GT_32 i = 0;
/* parameters not available in CPSS
    GT_BOOL isExternalCpuConnected;
    GT_BOOL enableLegacyVplsModeSupport;
    GT_U32 maxNumOfPhyPortsToUse;
    GT_U32 numOfDataIntegrityElements;
    CPSS_DXCH_LOGICAL_TABLES_SHADOW_STC *dataIntegrityShadowPtr;
*/
    cpssPpPhase1.devNum = devNum;
    for(i=0;i<CPSS_MAX_PORT_GROUPS_CNS;i++)
        cpssPpPhase1.hwInfo[i] = PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[devNum]->hwInfo[i];
    cpssPpPhase1.coreClock = PRV_CPSS_DXCH_PP_MAC(devNum)->genInfo.coreClock;
    cpssPpPhase1.mngInterfaceType = PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[devNum]->mngInterfaceType;
    cpssPpPhase1.serdesRefClock = PRV_CPSS_DXCH_PP_MAC(devNum)->port.serdesRefClock;
    cpssPpPhase1.isrAddrCompletionRegionsBmp = PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[devNum]->hwCtrl[0].isrAddrCompletionRegionsBmp;
    cpssPpPhase1.appAddrCompletionRegionsBmp = PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[devNum]->hwCtrl[0].appAddrCompletionRegionsBmp;
    cpssPpPhase1.numOfPortGroups = PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[devNum]->portGroupsInfo.numOfPortGroups;
    cpssPpPhase1.tcamParityCalcEnable =  PRV_CPSS_DXCH_PP_MAC(devNum)->diagInfo.tcamParityCalcEnable;
    cpssPpPhase1.ppHAState =  PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_OBJ_CONFIG[devNum]->HAState;
    cpssPpPhase1.cutThroughEnable =  PRV_CPSS_DXCH_PP_MAC(devNum)->cutThroughEnable;

    if( NULL == ppPhase1ParamsPtr )
    {
        return GT_FAIL;
    }

    *ppPhase1ParamsPtr = cpssPpPhase1;
    return GT_OK;
}

GT_STATUS appRefAldrinPortGobalResourcesPrint
(
    IN GT_U8 dev
)
{
    PRV_CPSS_DXCH_GROUP_RESOURCE_STATUS_STC *groupResorcesStatusPtr;
    GT_U32 dpIndex;
    GT_U32 maxDp;
    GT_U32 totalBWMbps;

    groupResorcesStatusPtr = &PRV_CPSS_DXCH_PP_MAC(dev)->portGroupsExtraInfo.groupResorcesStatus;

    maxDp = 1;
    if(PRV_CPSS_DXCH_PP_MAC(dev)->hwInfo.multiDataPath.supportMultiDataPath)
    {
        maxDp = PRV_CPSS_DXCH_PP_MAC(dev)->hwInfo.multiDataPath.maxDp;
    }

    totalBWMbps = 0;
    CPSS_APP_PLATFORM_LOG_PRINT_MAC("\n+-----+---------------+---------------+---------------+----------+");
    CPSS_APP_PLATFORM_LOG_PRINT_MAC("\n|     |  TxQ Descr's  |   Header Cnt  |   PayLoad Cnt |  Core    |");
    CPSS_APP_PLATFORM_LOG_PRINT_MAC("\n| DP# |---------------|---------------|---------------| Overall  |");
    CPSS_APP_PLATFORM_LOG_PRINT_MAC("\n|     |  used   total |  used   total |  used   total |  Speed   |");
    CPSS_APP_PLATFORM_LOG_PRINT_MAC("\n+-----+---------------+---------------+---------------+----------+");
    for (dpIndex = 0 ; dpIndex < maxDp; dpIndex++)
    {
        CPSS_APP_PLATFORM_LOG_PRINT_MAC("\n| %3d |",dpIndex);
        CPSS_APP_PLATFORM_LOG_PRINT_MAC("  %3d     %3d  |",groupResorcesStatusPtr->usedDescCredits   [dpIndex], groupResorcesStatusPtr->maxDescCredits   [dpIndex]);
        CPSS_APP_PLATFORM_LOG_PRINT_MAC("  %3d     %3d  |",groupResorcesStatusPtr->usedHeaderCredits [dpIndex], groupResorcesStatusPtr->maxHeaderCredits [dpIndex]);
        CPSS_APP_PLATFORM_LOG_PRINT_MAC("  %3d     %3d  |",groupResorcesStatusPtr->usedPayloadCredits[dpIndex], groupResorcesStatusPtr->maxPayloadCredits[dpIndex]);
        CPSS_APP_PLATFORM_LOG_PRINT_MAC("  %6d  |",groupResorcesStatusPtr->coreOverallSpeedSummary[dpIndex]);
        totalBWMbps += groupResorcesStatusPtr->coreOverallSpeedSummary[dpIndex];
    }
    CPSS_APP_PLATFORM_LOG_PRINT_MAC("\n+-----+---------------+---------------+---------------+----------+");
    CPSS_APP_PLATFORM_LOG_PRINT_MAC("\n|Total|               |               |               |  %6d  |",totalBWMbps);
    CPSS_APP_PLATFORM_LOG_PRINT_MAC("\n+-----+---------------+---------------+---------------+----------+");
    CPSS_APP_PLATFORM_LOG_PRINT_MAC("\n");
    return GT_OK;
}

extern GT_U32 appRefDxChTcamPclConvertedIndexGet_fromUT
(
    IN     GT_U8                                devNum,
    IN     GT_U32                               index,
    IN     CPSS_DXCH_TCAM_RULE_SIZE_ENT         ruleSize
);

typedef GT_U32 (*APP_REF_DXCH_TCAM_PCL_CONVERTED_INDEX_GET_FUN)
(
    IN     GT_U8                                devNum,
    IN     GT_U32                               index,
    IN     CPSS_DXCH_TCAM_RULE_SIZE_ENT         ruleSize
);

extern APP_REF_DXCH_TCAM_PCL_CONVERTED_INDEX_GET_FUN appDxChTcamPclConvertedIndexGetPtr;

typedef GT_STATUS (*appDemoDbEntryGet_TYPE)
(
    IN  GT_CHAR *namePtr,
    OUT GT_U32  *valuePtr
);
extern appDemoDbEntryGet_TYPE appDemoDbEntryGet_func;

extern GT_STATUS appPlatformDbEntryGet
(
    IN  GT_CHAR *namePtr,
    OUT GT_U32  *valuePtr
);

typedef struct{
    IN char*        testName;
}FORBIDEN_TESTS;
typedef struct{
    IN char*        suiteName;
}FORBIDEN_SUITES ;

void appRefFalconForbidenTests(IN FORBIDEN_TESTS   forbidenTests[])
{
#ifdef INCLUDE_UTF
    GT_U32  ii = 0;

    while(forbidenTests[ii].testName)
    {
        utfAddPreSkippedRule(
            "",
            "",
            forbidenTests[ii].testName,
            forbidenTests[ii].testName);

        ii++;
    }
#else
    GT_UNUSED_PARAM(forbidenTests);
#endif
    return;
}
void appRefFalconForbidenSuites(IN FORBIDEN_SUITES   forbidenSuites[])
{
#ifdef INCLUDE_UTF
    GT_U32  ii = 0;

    while(forbidenSuites[ii].suiteName)
    {
        utfAddPreSkippedRule(
            forbidenSuites[ii].suiteName,
            forbidenSuites[ii].suiteName,
            "",
            "");

        ii++;
    }
#else
    GT_UNUSED_PARAM(forbidenSuites);
#endif
    return;
}

/* make suite name and test name without "" to become a string */
#define TEST_NAME(test)    {#test}
/* make suite name to become a string */
#define SUITE_NAME(suite)   {#suite}
/*
    MainUT tests:
    list of tests/suites that forbidden because crash the CPSS or the device
   (fatal error in simulation on unknown address)
*/
GT_STATUS appRefFalconMainUtForbidenTests(void)
{
    static FORBIDEN_TESTS mainUt_forbidenTests_CRASH[] =
    {
        /* test CRASH or FATAL ERROR ! */

        /* test fail an may fail others */
        /*doing remap that fail in TQX scheduler :
            ERROR GT_BAD_PARAM in function: falcon_xel_to_cpss_err_code, file prvcpssdxchtxqpdq.c, line[137].
            [prvCpssFalconTxqPdqPeriodicSchemeConfig , line[415]] failed : in pdqNum[0] (retCode_xel[-1])

            but this test may fail others
            */
        TEST_NAME(cpssDxChPortPhysicalPortMapSet                  )

        ,{NULL}/* must be last */
    };

    /* suites that most or all tests CRASH or FATAL ERROR ! */
    static FORBIDEN_SUITES mainUt_forbidenSuites_CRASH [] =
    {
         SUITE_NAME(cpssDxChDiagDataIntegrityTables                )
        ,SUITE_NAME(cpssDxChPortCtrl                               )
        ,SUITE_NAME(cpssDxChPortStat                               )
        ,{NULL}/* must be last */
    };

    static FORBIDEN_TESTS mainUt_forbidenTests_many_tests_fail [] =
    {
         TEST_NAME(/*cpssDxChCscd.*/cpssDxChCscdHyperGPortCrcModeSet                             )
        ,TEST_NAME(/*cpssDxChCscd.*/cpssDxChCscdHyperGPortCrcModeGet                             )
        ,TEST_NAME(/*cpssDxChDiag.*/cpssDxChDiagPrbsSerdesStatusGet                              )
        ,TEST_NAME(/*cpssDxChDiag.*/cpssDxChDiagPrbsSerdesTestEnableSet                          )
        ,TEST_NAME(/*cpssDxChDiag.*/cpssDxChDiagPrbsSerdesTestEnableGet                          )
        ,TEST_NAME(/*cpssDxChDiag.*/cpssDxChDiagPrbsSerdesTransmitModeSet                        )
        ,TEST_NAME(/*cpssDxChDiag.*/cpssDxChDiagPrbsSerdesTransmitModeGet                        )
        ,TEST_NAME(/*cpssDxChDiag.*/cpssDxChDiagDeviceTemperatureThresholdSet                    )
        ,TEST_NAME(/*cpssDxChDiag.*/cpssDxChDiagDeviceVoltageGet                                 )
        ,TEST_NAME(/*cpssDxChDiagDataIntegrity.*/cpssDxChDiagDataIntegrityErrorInfoGet           )
        ,TEST_NAME(/*cpssDxChHwInit.*/cpssDxChHwAccessInfoStoreTest                              )
        ,TEST_NAME(/*cpssDxChPhySmi.*/cpssDxChPhyPortAddrSet                                     )
        ,TEST_NAME(/*cpssDxChPhySmi.*/cpssDxChPhyPortAddrGet                                     )
        ,TEST_NAME(/*cpssDxChPhySmi.*/cpssDxChPhyAutonegSmiGet                                   )
        ,TEST_NAME(/*cpssDxChPhySmi.*/cpssDxChPhyAutonegSmiSet                                   )
        ,TEST_NAME(/*cpssDxChPhySmi.*/cpssDxChPhySmiAutoMediaSelectSet                           )
        ,TEST_NAME(/*cpssDxChPhySmi.*/cpssDxChPhySmiAutoMediaSelectGet                           )
        ,TEST_NAME(/*cpssDxChPhySmi.*/cpssDxChPhySmiMdcDivisionFactorGet                         )
        ,TEST_NAME(/*cpssDxChPhySmi.*/cpssDxChPhySmiMdcDivisionFactorSet                         )
        ,TEST_NAME(/*cpssDxChPhySmiPreInit.*/cpssDxChPhyAutoPollNumOfPortsSet                    )
        ,TEST_NAME(/*cpssDxChPhySmiPreInit.*/cpssDxChPhyAutoPollNumOfPortsGet                    )
        ,TEST_NAME(/*cpssDxChPortIfModeCfg.*/cpssDxChPortModeSpeedSet                            )

        ,TEST_NAME(/*cpssDxChPortTx.*/cpssDxChPortTxQShaperProfileSet                            )
        ,TEST_NAME(/*cpssDxChPortTx.*/cpssDxChPortTx4TcTailDropProfileSet                        )
        ,TEST_NAME(/*cpssDxChPortTx.*/cpssDxChPortTx4TcTailDropProfileGet                        )
        ,TEST_NAME(/*cpssDxChPortTx.*/cpssDxChPortTxTailDropProfileSet                           )
        ,TEST_NAME(/*cpssDxChPortTx.*/cpssDxChPortTxTailDropProfileGet                           )
        ,TEST_NAME(/*cpssDxChPortTx.*/cpssDxChPortTxBindPortToDpSet                              )
        ,TEST_NAME(/*cpssDxChPortTx.*/cpssDxChPortTxBindPortToDpGet                              )

        ,TEST_NAME(/*cpssDxChPortBufMg.*/cpssDxChPortFcHolSysModeSet                             )
        ,TEST_NAME(/*cpssDxChPortBufMg.*/cpssDxChPortGlobalPacketNumberGet                       )
        ,TEST_NAME(/*cpssDxChPortBufMg.*/cpssDxChPortRxBufNumberGet                              )

        ,TEST_NAME(/*cpssDxChPortPfc.*/cpssDxChPortPfcCounterGet                                 )


        ,{NULL}/* must be last */
    };

#ifdef IMPL_TGF

    static FORBIDEN_TESTS enhUt_forbidenTests_many_tests_fail [] =
    {
        TEST_NAME(/*tgfDiag.*/prvTgfDiagMppmEccProtection                   )
        ,{NULL}/* must be last */
    };

    /* suites that most or all tests fail ! */
    /* those tests are about 'timing' */
    static FORBIDEN_SUITES onEmulator_enhUt_forbidenSuites_many_tests_fail [] =
    {
         SUITE_NAME(tgfNetIf/* this suite must be 'debugged' before running it on emulator */)
        ,{NULL}/* must be last */
    };

#endif /*IMPL_TGF*/

    appRefFalconForbidenTests(mainUt_forbidenTests_CRASH);
    appRefFalconForbidenSuites(mainUt_forbidenSuites_CRASH);
    appRefFalconForbidenTests(mainUt_forbidenTests_many_tests_fail);

#ifdef IMPL_TGF
    appRefFalconForbidenTests (enhUt_forbidenTests_many_tests_fail);

    if(cpssDeviceRunCheck_onEmulator())
    {
        appRefFalconForbidenSuites(onEmulator_enhUt_forbidenSuites_many_tests_fail);
    }

#endif /*IMPL_TGF*/

    return GT_OK;
}

/*
    MainUT tests:
    list of tests/suites that forbidden because crash the CPSS or the device(AC5X)
   (fatal error in simulation on unknown address)
*/
static GT_STATUS   appRefPhoenixMainUtForbidenTests(void)
{
#ifdef IMPL_TGF
    static FORBIDEN_TESTS mainUt_serdes [] =
    {
         TEST_NAME(cpssDxChPortSerdesGroupGet)
        ,TEST_NAME(cpssDxChPortSerdesPowerStatusSet)
        ,TEST_NAME(cpssDxChPortSerdesResetStateSet)
        ,TEST_NAME(cpssDxChPortSerdesAutoTune)
        ,TEST_NAME(cpssDxChPortSerdesLoopbackModeSet)
        ,TEST_NAME(cpssDxChPortSerdesLoopbackModeGet)
        ,TEST_NAME(cpssDxChPortSerdesPolaritySet)
        ,TEST_NAME(cpssDxChPortSerdesPolarityGet)
        ,TEST_NAME(cpssDxChPortSerdesTuningSet)
        ,TEST_NAME(cpssDxChPortSerdesTuningGet)
        ,TEST_NAME(cpssDxChPortSerdesTxEnableSet)
        ,TEST_NAME(cpssDxChPortSerdesTxEnableGet)
        ,TEST_NAME(cpssDxChPortSerdesManualTxConfigSet)
        ,TEST_NAME(cpssDxChPortSerdesManualTxConfigGet)
        ,TEST_NAME(cpssDxChPortEomBaudRateGet)
        ,TEST_NAME(cpssDxChPortEomDfeResGet)
        ,TEST_NAME(cpssDxChPortEomMatrixGet)
        ,TEST_NAME(cpssDxChPortSerdesAutoTuneStatusGet)
        ,TEST_NAME(cpssDxChPortSerdesPpmGet)
        ,TEST_NAME(cpssDxChPortSerdesPpmSet)
        ,TEST_NAME(cpssDxChPortSerdesManualRxConfigSet)
        ,TEST_NAME(cpssDxChPortSerdesManualRxConfigGet)
        ,TEST_NAME(cpssDxChPortSerdesEyeMatrixGet)
        ,TEST_NAME(cpssDxChPortSerdesLaneTuningSet)
        ,TEST_NAME(cpssDxChPortSerdesLaneTuningGet)

        ,TEST_NAME(cpssDxChPortSyncEtherRecoveredClkAutomaticMaskingEnableSet)/* also read SERDES non supported register */
        ,TEST_NAME(cpssDxChCatchUpValidityCheckTest)                          /* also read SERDES non supported register - for recovery */

        /* other non-serdes tests : */
        ,TEST_NAME(cpssDxChPhyPortSmiRegisterRead) /*SMI is not supported as we not init the PHY lib ... see appRefDbEntryAdd ("initPhy",0); */
        ,TEST_NAME(cpssDxChPhyPortSmiRegisterWrite)/*SMI is not supported as we not init the PHY lib ... see appRefDbEntryAdd ("initPhy",0); */

        /* PLL tests */
        ,TEST_NAME(cpssDxChPtpPLLBypassEnableGet)
        ,TEST_NAME(cpssDxChPtpPLLBypassEnableSet)


        ,{NULL}/* must be last */
    };

    static FORBIDEN_TESTS mainUt_forbidenTests_FatalError [] =
    {
         TEST_NAME(/*cpssDxChPhySmi.*/cpssDxChPhyXsmiMdcDivisionFactorSet )
        ,{NULL}/* must be last */
    };

    /* suites that most or all tests CRASH or FATAL ERROR or Fail ! */
    static FORBIDEN_SUITES mainUt_forbidenSuites_CRASH_FAIL [] =
    {

        SUITE_NAME(cpssDxChHwInitLedCtrl      )/* lib is not aligned yet */
        ,SUITE_NAME(cpssDxChDiagBist           )/* lib is not aligned yet */
        ,SUITE_NAME(cpssDxChVirtualTcam        )/* lib is not aligned yet - 10 Byte rules alignment must be done */
        ,SUITE_NAME(cpssDxChHwInitDeviceMatrix )/* lib is not aligned yet */
        ,SUITE_NAME(cpssDxChPortSyncEther      )/* lib is not aligned yet */

        ,{NULL}/* must be last */
    };

    static FORBIDEN_SUITES mainUt_forbidenSuites_Emulator_too_long_and_PASS [] =
    {
         SUITE_NAME(cpssDxChTrunkHighLevel_1) /* cpssDxChTrunkHighLevel is enough to run */
        ,SUITE_NAME(cpssDxChTrunkLowLevel_1)  /* cpssDxChTrunkLowLevel  is enough to run */

        ,{NULL}/* must be last */
    };

    static FORBIDEN_TESTS enhUt_forbidenTests_FatalError [] =
    {
         TEST_NAME(/*tgfPtp.*/tgfPtpEgressTimestampQueueinterrupt   )
        ,TEST_NAME(/*tgfCutThrough.*/prvTgfCutThroughIpv4CtTermination_SlowToFast         ) /* port modes not supported yet */

#ifdef WIN32
        ,SUITE_NAME(prvTgfIpLpmHaCheck         )/* crash on NULL pointer only on WIN32 !!! */
#endif /*WIN32*/

        ,{NULL}/* must be last */
    };

    static FORBIDEN_TESTS enhUt_forbidenTests_too_long_and_fail [] =
    {
         TEST_NAME(tgfPortTxSip6QcnQueueGlobalResourcesWithoutIngressHdr) /* failed [20] times ,takes [618] sec  */
        ,TEST_NAME(tgfPortTxSip6QcnQueueGlobalResourcesWithIngressHdr)    /* failed [20] times ,takes [534] sec  */
        ,TEST_NAME(tgfPortTxSip6QcnQueuePool0ResourcesWithoutIngressHdr)  /* failed [46] times ,takes [1707] sec */
        ,TEST_NAME(tgfPortTxSip6QcnQueuePool1ResourcesWithoutIngressHdr)
        ,TEST_NAME(tgfPortTxSip6QcnQueuePool0ResourcesWithIngressHdr)
        ,TEST_NAME(tgfPortTxSip6QcnQueuePool1ResourcesWithIngressHdr)

        ,TEST_NAME(tgfEOamKeepAliveFlowHashVerificationBitSelection )
        ,TEST_NAME(prvTgfCutThroughBypassOam                        )


        ,{NULL}/* must be last */
    };

    static FORBIDEN_TESTS mainUt_forbidenTests_Emulator_too_long_and_PASS [] =
    {
         TEST_NAME(cpssDxChDiagAllMemTest)                  /* : takes [313] sec */
        ,TEST_NAME(cpssDxChLatencyMonitoringPortCfgSet)     /* : takes [219] sec */
        ,TEST_NAME(cpssDxChPtpTaiTodStepSet)                /* : takes [119] sec */
        ,TEST_NAME(cpssDxChPtpTsuTSFrameCounterControlSet)  /* : takes [1322] sec*/

        ,TEST_NAME(prvCpssDxChTestFdbHash_8K)               /* : takes [92] sec */
        ,TEST_NAME(prvCpssDxChTestFdbHash_16K)              /* : takes [92] sec */
        /*,TEST_NAME(prvCpssDxChTestFdbHash_32K) allow one of those to run */  /* : takes [92] sec */
        ,TEST_NAME(cpssDxChTrunkDbMembersSortingEnableSet_membersManipulations)/* : takes [167] sec*/
        ,TEST_NAME(cpssDxChExactMatchRulesDump)             /* : takes [209] sec */
        ,TEST_NAME(cpssDxChPortLoopbackPktTypeToCpuSet)     /* : takes [295] sec */

        /* enhUT : */
        ,TEST_NAME(tgfIOamKeepAliveFlowHashVerificationBitSelection) /* : takes [252] sec */
        ,TEST_NAME(prvTgfIpv4NonExactMatchMcRouting    ) /*takes [147] sec*/
        ,TEST_NAME(tgfIpv4UcRoutingHiddenPrefixes      ) /*takes [215] sec*/
        ,TEST_NAME(tgfBasicIpv4UcEcmpRoutingHashCheck  ) /*takes [371] sec*/
        ,TEST_NAME(prvTgfPclMetadataPacketType         ) /*takes [804] sec*/

        ,{NULL}/* must be last */
    };


    static FORBIDEN_TESTS mainUt_forbidenTests_Emulator_Stuck [] =
    {
         TEST_NAME(/*cpssDxChCscd.*/cpssDxChCscdQosTcDpRemapTableAccessModeGet)
        ,TEST_NAME(/*cpssDxChPtp.*/cpssDxChPtpTaiPtpPulseInterfaceSet       )
        ,TEST_NAME(/*cpssDxChPtp.*/cpssDxChPtpTaiPtpPulseInterfaceGet       )

        ,TEST_NAME(/*cpssDxChPtp.*/cpssDxChPtpTsuTSFrameCounterControlSet)
        ,TEST_NAME(/*cpssDxChPtp.*/cpssDxChPtpTsuTSFrameCounterControlGet)
        ,TEST_NAME(/*cpssDxChPtp.*/cpssDxChPtpTsuTsFrameCounterIndexSet)

        ,{NULL}/* must be last */
    };

    static FORBIDEN_TESTS enhancedUt_forbidenTests_Emulator_Stuck [] =
    {
         TEST_NAME(/*tgfMpls.*/tgfMplsSrEntropyLabel) /* segmentation fail */
        ,{NULL}/* must be last */
    };

    static FORBIDEN_SUITES enhancedUt_forbidenSuites_Emulator_Killer [] =
    {

        {NULL}/* must be last */
    };

    static FORBIDEN_TESTS enhancedUt_forbidenTests_Emulator_LongerThanOneMinute [] =
    {
         TEST_NAME(tgfIOamKeepAliveFlowHashVerificationBitSelection)
        ,TEST_NAME(tgfPortTxSchedulerSp)
        ,TEST_NAME(tgfPortTxTailDropDbaPortResources)
        ,TEST_NAME(prvTgfBrgVplsBasicTest1)
        ,TEST_NAME(tgfBasicTrafficSanity)
        ,TEST_NAME(tgfBasicDynamicLearning_fromCascadePortDsaTagForward)
        ,TEST_NAME(tgfVlanManipulationEthernetOverMpls)
        ,TEST_NAME(prvTgfFdbIpv4UcPointerRouteByIndex)
        ,TEST_NAME(prvTgfFdbIpv4UcPointerRouteByMsg)
        ,TEST_NAME(prvTgfFdbIpv4UcRoutingLookupMask)
        ,TEST_NAME(prvTgfFdbIpv4UcRoutingAgingEnable)
        ,TEST_NAME(prvTgfFdbIpv6UcRoutingLookupMask)
        ,TEST_NAME(prvTgfFdbIpv6UcRoutingAgingEnable)
        ,TEST_NAME(prvTgfBrgGenMtuCheck)
        ,TEST_NAME(tgfBasicIpv4UcEcmpRouting)
        ,TEST_NAME(tgfBasicIpv4UcEcmpRoutingRandomEnable)
        ,TEST_NAME(tgfBasicIpv4UcEcmpRoutingIndirectAccess)
        ,TEST_NAME(prvTgfIpv4NonExactMatchUcRouting)
        ,TEST_NAME(prvTgfIpv4NonExactMatchMcRouting)
        ,TEST_NAME(tgfIpv4UcRoutingHiddenPrefixes)
        ,TEST_NAME(tgfBasicIpv4UcEcmpRoutingHashCheck)
        ,TEST_NAME(prvTgfPclMetadataPacketType)
        ,TEST_NAME(prvTgfPclMetadataPacketTypeIpOverMpls)
        ,TEST_NAME(prvTgfIngressMirrorEportVsPhysicalPort)
        ,TEST_NAME(prvTgfEgressMirrorEportVsPhysicalPort)
        ,TEST_NAME(tgfTrunkSaLearning)
        ,TEST_NAME(tgfTrunkSaLearning_globalEPortRepresentTrunk)
        ,TEST_NAME(tgfTrunkSimpleHashMacSaDa)
        ,TEST_NAME(tgfTrunkSimpleHashMacSaDa_globalEPortRepresentTrunk)
        ,TEST_NAME(tgfTrunkWithRemoteMembers)
        ,TEST_NAME(tgfTrunkCrcHashMaskPriority)
        ,TEST_NAME(tgfTrunkCascadeTrunk)
        ,TEST_NAME(tgfTrunkDesignatedTableModes)
        ,TEST_NAME(tgfTrunkCascadeTrunkWithWeights)
        ,TEST_NAME(tgfTrunkSortMode)
        ,TEST_NAME(tgfTrunkCrcHash_liteMode_CRC32_cpssPredictEgressPort)
        ,TEST_NAME(tgfTrunk_80_members_flood_EPCL)
        ,TEST_NAME(tgfTrunkPacketTypeHashModeTest)
        ,TEST_NAME(tgfIpfixSamplingToCpuTest)
        ,TEST_NAME(tgfIpfixAlarmEventsTest)
        ,TEST_NAME(prvTgfUnucUnregmcBcFiltering)
        ,TEST_NAME(prvTgfEgressInterface)
        ,TEST_NAME(tgfPortTxSip6QcnQueuePool0ResourcesWithoutIngressHdr)
        ,TEST_NAME(tgfPortTxSip6QcnQueuePool1ResourcesWithoutIngressHdr)
        ,TEST_NAME(tgfPortTxSip6QcnQueuePool0ResourcesWithIngressHdr)
        ,TEST_NAME(tgfPortTxSip6QcnQueuePool1ResourcesWithIngressHdr)
        ,TEST_NAME(tgfTunnelTermEtherOverMplsPwLabelParallelLookup)
        ,TEST_NAME(prvTgfTunnelTermPbrDualLookup)
        ,{NULL}/* must be last */
    };
#endif /*IMPL_TGF*/


    /* tests that defined forbidden by Phoenix */

#ifdef IMPL_TGF
    appRefFalconForbidenTests (mainUt_forbidenTests_FatalError);
    appRefFalconForbidenSuites(mainUt_forbidenSuites_CRASH_FAIL);
    appRefFalconForbidenTests (enhUt_forbidenTests_FatalError);
    appRefFalconForbidenTests (enhUt_forbidenTests_too_long_and_fail);

    if (cpssDeviceRunCheck_onEmulator())
    {
        /* very slow suites */
        utfAddPreSkippedRule("cpssDxChBridgeFdbManager","*","","");
        utfAddPreSkippedRule("cpssDxChExactMatchManager","*","","");
        utfAddPreSkippedRule("tgfBridgeFdbManager","*","","");

        appRefFalconForbidenTests (mainUt_forbidenTests_Emulator_Stuck);
        appRefFalconForbidenTests (enhancedUt_forbidenTests_Emulator_Stuck);
        appRefFalconForbidenTests (mainUt_serdes);
        appRefFalconForbidenTests (mainUt_forbidenTests_Emulator_too_long_and_PASS);
        appRefFalconForbidenSuites(mainUt_forbidenSuites_Emulator_too_long_and_PASS);
        appRefFalconForbidenSuites (enhancedUt_forbidenSuites_Emulator_Killer);

        {
            GT_U32 skip = 0;
            appPlatformDbEntryGet("onEmulatorSkipSlowEnhancedUt", &skip);
            if (skip)
            {
                appRefFalconForbidenTests(
                    enhancedUt_forbidenTests_Emulator_LongerThanOneMinute);
            }
        }
    }
#endif /*IMPL_TGF*/

    /* add tests that defined forbidden by Falcon */
    return appRefFalconMainUtForbidenTests();
}

/**
* @internal localUtfInitRef function
* @endinternal
*
* @brief   init the UTF
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS localUtfInitRef
(
    IN GT_U8 devNum
)
{
    GT_STATUS rc = GT_OK;

#ifdef IMPL_TGF
    appDxChTcamPclConvertedIndexGetPtr = appRefDxChTcamPclConvertedIndexGet_fromUT;
    appDemoDbEntryGet_func = appPlatformDbEntryGet;
    appDemoDxChPortMgrPortModeSpeedSet_func = appRefDxChPortMgrPortModeSpeedSet;
#endif

    rc = appRefTcamLibParamsSet();
    if(rc!=GT_OK)
    {
        return rc;
    }

#ifdef INCLUDE_UTF
    /* Initialize unit tests for CPSS */
    rc = utfPreInitPhase();
    if (rc != GT_OK)
    {
        utfPostInitPhase(GT_OK);
        return rc;
    }

    if(!PRV_CPSS_IS_DEV_EXISTS_MAC(devNum))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    rc = utfInit(devNum);
    if (rc != GT_OK)
    {
        utfPostInitPhase(rc);
        return rc;
    }

    utfPostInitPhase(GT_OK);
#endif /* INCLUDE_UTF */

    if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        if(PRV_CPSS_SIP_6_15_CHECK_MAC(devNum))
        {
            appRefPhoenixMainUtForbidenTests();
        }
        else
        {
            appRefFalconMainUtForbidenTests();
        }
    }

    return rc;
}

/**
* @internal appRefDxChDiagDataIntegrityEventCounterIncrement function
* @endinternal
*
* @brief   Function for increment counter per data integrity event
*
* @param[in] devNum                   - device number
* @param[in] eventPtr                 - (pointer to) data integrity event structure
*
* @retval CMD_OK                   - on success.
* @retval GT_BAD_PARAM             - on wrong values of input parameters.
* @retval GT_OUT_OF_CPU_MEM        - on out of CPU memory
*
* @note First call is init DB, eventPtr = NULL
*
*/
GT_STATUS appRefDxChDiagDataIntegrityEventCounterIncrement
(
    IN  GT_U8                                       devNum,
    IN  CPSS_DXCH_DIAG_DATA_INTEGRITY_EVENT_STC     *eventPtr
)
{
    GT_U32 key = 0; /* key index to DB */
    GT_U32 portGroupId;
    GT_U32 bmp;
    CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT      memType;

    if(devNum >= PRV_CPSS_MAX_PP_DEVICES_CNS)
    {
        /* check device out of range */
        return GT_BAD_PARAM;
    }

    if(isDataIntegrityInitDone == GT_FALSE)
    {
        isDataIntegrityInitDone = GT_TRUE;
        /* first call - DB initialization only */
        cmdDataIntegrityEventsDbPtr = (CMD_DXCH_DIAG_DATA_INTEGRITY_COUNTER_DB_STC*)cpssOsMalloc(sizeof(CMD_DXCH_DIAG_DATA_INTEGRITY_COUNTER_DB_STC));
        cmdDataIntegrityMppmEventsDbPtr = (CMD_DXCH_DIAG_DATA_INTEGRITY_MPPM_COUNTER_DB_STC*)cpssOsMalloc(sizeof(CMD_DXCH_DIAG_DATA_INTEGRITY_MPPM_COUNTER_DB_STC));
        cmdDataIntegrityTcamEventsDbPtr = (CMD_DXCH_DIAG_DATA_INTEGRITY_TCAM_COUNTER_DB_STC*)cpssOsMalloc(sizeof(CMD_DXCH_DIAG_DATA_INTEGRITY_TCAM_COUNTER_DB_STC));

        if((cmdDataIntegrityEventsDbPtr == NULL) || (cmdDataIntegrityMppmEventsDbPtr == NULL) || (cmdDataIntegrityTcamEventsDbPtr == NULL))
        {
            return GT_OUT_OF_CPU_MEM;
        }
        else
        {
            cpssOsMemSet(cmdDataIntegrityEventsDbPtr, 0, sizeof(CMD_DXCH_DIAG_DATA_INTEGRITY_COUNTER_DB_STC));
            cpssOsMemSet(cmdDataIntegrityMppmEventsDbPtr, 0, sizeof(CMD_DXCH_DIAG_DATA_INTEGRITY_MPPM_COUNTER_DB_STC));
            cpssOsMemSet(cmdDataIntegrityTcamEventsDbPtr, 0, sizeof(CMD_DXCH_DIAG_DATA_INTEGRITY_TCAM_COUNTER_DB_STC));
            return GT_OK;
        }
    }
    else
    {
        CPSS_NULL_PTR_CHECK_MAC(eventPtr);

        /* get portGroupId from bitmap */
        bmp = eventPtr->location.portGroupsBmp;
        if (bmp == 0)
        {
            return GT_BAD_PARAM;
        }
        for (portGroupId = 0, bmp >>=1; bmp; portGroupId++, bmp>>=1);

        memType = eventPtr->location.ramEntryInfo.memType;
        if((memType == CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_MPPM_DATA_BUFFER_MEMORY_E) ||
           (memType == CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_MPPM_DATA_BUFFER_MEMORY_ECC_E))
        {
            if((eventPtr->location.mppmMemLocation.mppmId > 1) ||
               (eventPtr->location.mppmMemLocation.bankId > 1) ||
               (GT_FALSE == eventPtr->location.isMppmInfoValid))
            {
                return GT_BAD_PARAM;
            }

            key = eventPtr->location.mppmMemLocation.mppmId;
            key |= (eventPtr->location.mppmMemLocation.bankId << 1);
            if(eventPtr->eventsType == CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_SINGLE_ECC_E)
            {
                key |= (0 << 2);
            }
            else if(eventPtr->eventsType == CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_MULTIPLE_ECC_E)
            {
                key |= (1 << 2);
            }
            else
            {
                return GT_BAD_PARAM;
            }
            key |= (portGroupId << 3);

            if(key >= CMD_DXCH_DIAG_DATA_INTEGRITY_MPPM_DB_SIZE_CNS)
            {
                /* need to update size of DB */
                return GT_FAIL;
            }

            cmdDataIntegrityMppmEventsDbPtr->origPortGroupId[key] = (GT_U8)portGroupId;
            cmdDataIntegrityMppmEventsDbPtr->eventCounterArr[key] += 1;

            return GT_OK;
        }
        else if((memType == CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_ROUTER_TCAM_E) ||
                (memType == CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_POLICY_TCAM_E) ||
                (memType == CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TCAM_PARITY_E))
        {
            /*
                Key Structure:
                    bits[0:10] - TCAM line index for Router TCAM, PCL rule index for policy TCAM
                    bits[11] - array type X/Y
                    bits[12:14] - portGroup
                    bits[15] - Policy or Router
            */
            if (GT_FALSE == eventPtr->location.isTcamInfoValid)
            {
                return GT_BAD_PARAM;
            }

            key = eventPtr->location.tcamMemLocation.ruleIndex & 0x7FF;

            if(eventPtr->location.tcamMemLocation.arrayType == CPSS_DIAG_DATA_INTEGRITY_TCAM_ARRAY_TYPE_X_E)
            {
                key |= (0 << 11);
            }
            else if(eventPtr->location.tcamMemLocation.arrayType == CPSS_DIAG_DATA_INTEGRITY_TCAM_ARRAY_TYPE_Y_E)
            {
                key |= (1 << 11);
            }
            else
            {
                return GT_BAD_PARAM;
            }

            key |= (portGroupId << 12);

            if(memType == CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_POLICY_TCAM_E)
            {
                key |= (0 << 15);
            }
            else
            {
                key |= (1 << 15);
            }

            if(key >= CMD_DXCH_DIAG_DATA_INTEGRITY_TCAM_DB_SIZE_CNS)
            {
                /* need to update size of DB */
                return GT_FAIL;
            }

            cmdDataIntegrityTcamEventsDbPtr->eventCounterArr[key] += 1;

            return GT_OK;
        }
        else
        {
            /* calculate index for DB access */
            key = eventPtr->location.ramEntryInfo.memLocation.dfxMemoryId;
            key |= (eventPtr->location.ramEntryInfo.memLocation.dfxClientId << 7);
            key |= (eventPtr->location.ramEntryInfo.memLocation.dfxPipeId << 12);
        }

        switch(eventPtr->eventsType)
        {
            case CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_PARITY_E:
                U32_SET_FIELD_MAC(key, 15, 2, 1);
                break;
            case CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_SINGLE_ECC_E:
                U32_SET_FIELD_MAC(key, 15, 2, 2);
                break;
            case CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_MULTIPLE_ECC_E:
                U32_SET_FIELD_MAC(key, 15, 2, 3);
                break;
            default:
                return GT_BAD_PARAM;
        }

        if (appRefDxChTraceEvents)
        {
            CPSS_APP_PLATFORM_LOG_INFO_MAC("eventPtr->memLocation.dfxPipeId = 0x%x\r\n", U32_GET_FIELD_MAC(key, 12 , 3));
            CPSS_APP_PLATFORM_LOG_INFO_MAC("eventPtr->memLocation.dfxClientId = 0x%x\r\n", U32_GET_FIELD_MAC(key, 7 , 5));
            CPSS_APP_PLATFORM_LOG_INFO_MAC("eventPtr->memLocation.dfxMemoryId = %d\r\n", U32_GET_FIELD_MAC(key, 0 , 7));
            CPSS_APP_PLATFORM_LOG_INFO_MAC("eventPtr->eventsType = 0x%x\r\n", eventPtr->eventsType);
            CPSS_APP_PLATFORM_LOG_INFO_MAC("eventPtr->memType = 0x%x\r\n", memType);
            CPSS_APP_PLATFORM_LOG_INFO_MAC("key = 0x%x\r\n", key);
        }

        if(key >= BIT_17)
        {
            /* need to update size of DB */
            return GT_FAIL;
        }

        cmdDataIntegrityEventsDbPtr->eventCounterArr[key] += 1;
        cmdDataIntegrityEventsDbPtr->memTypeArr[key] = memType;
        cmdDataIntegrityEventsDbPtr->origPortGroupId[key] = (GT_U8)portGroupId;
    }

    return GT_OK;
}

/**
* @internal appRefDxChDiagDataIntegrityCountersCbGet function
* @endinternal
*
* @brief   Routine to bind a CB function that receives block of
*         data integrity event counters for given device.
* @param[in] dataIntegrityEventCounterBlockGetCB - callback function
*
* @retval CMD_OK                   - on success.
*/
GT_STATUS appRefDxChDiagDataIntegrityCountersCbGet
(
    DXCH_DATA_INTEGRITY_EVENT_COUNTER_INCREMENT_FUNC **dataIntegrityEventCounterBlockGetCB
)
{
    *dataIntegrityEventCounterBlockGetCB = appRefDxChDiagDataIntegrityEventCounterIncrement;
    return GT_OK;
}

CMD_DXCH_DIAG_DATA_INTEGRITY_COUNTER_DB_STC * appRefDxChDiagDataIntegrityCountersDbGet
(
    GT_VOID
)
{
    return cmdDataIntegrityEventsDbPtr;
}

/**
* @internal appRefDxChDiagDataIntegrityEventTableGetFirst function
* @endinternal
*
* @brief   get first data integrity event counter entry.
*
* @retval CMD_OK                   - on success.
* @retval CMD_AGENT_ERROR          - on failure.
* @retval CMD_FIELD_UNDERFLOW      - not enough field arguments.
* @retval CMD_FIELD_OVERFLOW       - too many field arguments.
*
* @note none
*
*/
GT_STATUS appRefDxChDiagDataIntegrityEventTableGetFirst
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    numFields = numFields;
    GT_U8       devNum;
    CMD_DXCH_DIAG_DATA_INTEGRITY_EVENT_TYPE_FILTER_ENT  filterType;
    CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_ENT       errorType = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_SINGLE_AND_MULTIPLE_ECC_E;
    GT_BOOL filterEnable = GT_TRUE;

    (void)inFields;
    (void)numFields;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return GT_FAIL;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    filterType = (CMD_DXCH_DIAG_DATA_INTEGRITY_EVENT_TYPE_FILTER_ENT)inArgs[1];

        return GT_FAIL;


    if(devNum >= PRV_CPSS_MAX_PP_DEVICES_CNS || cmdDataIntegrityEventsDbPtr == NULL)
    {
        cpssOsSprintf((GT_CHAR *)outArgs, "@@@%x", GT_OK);/*may replace later*/
        return GT_OK;
    }

    currentDbKeyAppRef = 0;

    while(currentDbKeyAppRef < endDbKeyAppRef)
    {
        switch((currentDbKeyAppRef >> 15) & 0x3)
        {
            case 1:
                errorType = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_PARITY_E;
                if(filterType == CMD_DXCH_DIAG_DATA_INTEGRITY_EVENT_TYPE_FILTER_PARITY_E)
                {
                    filterEnable = GT_FALSE;
                }
                break;
            case 2:
                errorType = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_SINGLE_ECC_E;
                if(filterType == CMD_DXCH_DIAG_DATA_INTEGRITY_EVENT_TYPE_FILTER_SINGLE_ECC_E)
                {
                    filterEnable = GT_FALSE;
                }
                break;
            case 3:
                errorType = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_MULTIPLE_ECC_E;
                if(filterType == CMD_DXCH_DIAG_DATA_INTEGRITY_EVENT_TYPE_FILTER_MULTIPLE_ECC_E)
                {
                    filterEnable = GT_FALSE;
                }
                break;
            default:
                filterEnable = GT_TRUE;
                break;
        }
        if(filterType == CMD_DXCH_DIAG_DATA_INTEGRITY_EVENT_TYPE_FILTER_ALL_E)
        {
            filterEnable = GT_FALSE;
        }

        if((cmdDataIntegrityEventsDbPtr->eventCounterArr[currentDbKeyAppRef] == 0) || (filterEnable == GT_TRUE))
        {
            currentDbKeyAppRef++;
            continue;
        }
        else
        {
            break;
        }

    }

    if(currentDbKeyAppRef < endDbKeyAppRef)
    {
        inFields[0] = errorType;
        inFields[1] = cmdDataIntegrityEventsDbPtr->memTypeArr[currentDbKeyAppRef];
        inFields[2] = cmdDataIntegrityEventsDbPtr->origPortGroupId[currentDbKeyAppRef];
        inFields[3] = U32_GET_FIELD_MAC(currentDbKeyAppRef, 12, 3);
        inFields[4] = U32_GET_FIELD_MAC(currentDbKeyAppRef, 7, 5);
        inFields[5] = U32_GET_FIELD_MAC(currentDbKeyAppRef, 0, 7);
        inFields[6] = cmdDataIntegrityEventsDbPtr->eventCounterArr[currentDbKeyAppRef];
        cpssOsSprintf((GT_CHAR *)outArgs, "@@@%x", GT_OK);/*may replace later*/
    }
    else
    {
        cpssOsSprintf((GT_CHAR *)outArgs, "@@@%x", GT_OK);/*may replace later*/
    }

    return GT_OK;
}

/**
* @internal appRefDxChDiagDataIntegrityEventTableGetNext function
* @endinternal
*
* @brief   get next data integrity event counter entry.
*
* @retval CMD_OK                   - on success.
* @retval CMD_AGENT_ERROR          - on failure.
* @retval CMD_FIELD_UNDERFLOW      - not enough field arguments.
* @retval CMD_FIELD_OVERFLOW       - too many field arguments.
*
* @note none
*
*/
GT_STATUS appRefDxChDiagDataIntegrityEventTableGetNext
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    numFields = numFields;
    CMD_DXCH_DIAG_DATA_INTEGRITY_EVENT_TYPE_FILTER_ENT  filterType;
    CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_ENT       errorType = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_SINGLE_AND_MULTIPLE_ECC_E;
    GT_BOOL filterEnable = GT_TRUE;

    (void)numFields;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return GT_FAIL;

    filterType = (CMD_DXCH_DIAG_DATA_INTEGRITY_EVENT_TYPE_FILTER_ENT)inArgs[1];

    currentDbKeyAppRef++;

    while(currentDbKeyAppRef < endDbKeyAppRef)
    {
        switch((currentDbKeyAppRef >> 15) & 0x3)
        {
            case 1:
                errorType = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_PARITY_E;
                if(filterType == CMD_DXCH_DIAG_DATA_INTEGRITY_EVENT_TYPE_FILTER_PARITY_E)
                {
                    filterEnable = GT_FALSE;
                }
                break;
            case 2:
                errorType = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_SINGLE_ECC_E;
                if(filterType == CMD_DXCH_DIAG_DATA_INTEGRITY_EVENT_TYPE_FILTER_SINGLE_ECC_E)
                {
                    filterEnable = GT_FALSE;
                }
                break;
            case 3:
                errorType = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_MULTIPLE_ECC_E;
                if(filterType == CMD_DXCH_DIAG_DATA_INTEGRITY_EVENT_TYPE_FILTER_MULTIPLE_ECC_E)
                {
                    filterEnable = GT_FALSE;
                }
                break;
            default:
                filterEnable = GT_TRUE;
                break;
        }
        if(filterType == CMD_DXCH_DIAG_DATA_INTEGRITY_EVENT_TYPE_FILTER_ALL_E)
        {
            filterEnable = GT_FALSE;
        }

        if((cmdDataIntegrityEventsDbPtr->eventCounterArr[currentDbKeyAppRef] == 0) || (filterEnable == GT_TRUE))
        {
            currentDbKeyAppRef++;
            continue;
        }
        else
        {
            break;
        }

    }

    if(currentDbKeyAppRef < endDbKeyAppRef)
    {
        inFields[0] = errorType;
        inFields[1] = cmdDataIntegrityEventsDbPtr->memTypeArr[currentDbKeyAppRef];
        inFields[2] = cmdDataIntegrityEventsDbPtr->origPortGroupId[currentDbKeyAppRef];
        inFields[3] = U32_GET_FIELD_MAC(currentDbKeyAppRef, 12, 3);
        inFields[4] = U32_GET_FIELD_MAC(currentDbKeyAppRef, 7, 5);
        inFields[5] = U32_GET_FIELD_MAC(currentDbKeyAppRef, 0, 7);
        inFields[6] = cmdDataIntegrityEventsDbPtr->eventCounterArr[currentDbKeyAppRef];

        cpssOsSprintf((GT_CHAR *)outArgs, "@@@%x", GT_OK);/*may replace later*/
    }
    else
    {
        cpssOsSprintf((GT_CHAR *)outArgs, "@@@%x", GT_OK);/*may replace later*/
    }

    return GT_OK;
}

/**
* @internal appRefDxChDiagDataIntegrityEventTableClear function
* @endinternal
*
* @brief   clear all entries in event counter entry block.
*
* @retval CMD_OK                   - on success.
* @retval CMD_AGENT_ERROR          - on failure.
* @retval CMD_FIELD_UNDERFLOW      - not enough field arguments.
* @retval CMD_FIELD_OVERFLOW       - too many field arguments.
*
* @note none
*
*/
GT_STATUS appRefDxChDiagDataIntegrityEventTableClear
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_U8       devNum;

    (void)inFields;
    (void)numFields;

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return GT_FAIL;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];

    if(devNum >= PRV_CPSS_MAX_PP_DEVICES_CNS || cmdDataIntegrityEventsDbPtr == NULL)
    {
        cpssOsSprintf((GT_CHAR *)outArgs, "@@@%x", GT_OK);/*may replace later*/
        return GT_OK;
    }

    cpssOsMemSet(cmdDataIntegrityEventsDbPtr, 0, sizeof(CMD_DXCH_DIAG_DATA_INTEGRITY_COUNTER_DB_STC));

    cpssOsSprintf((GT_CHAR *)outArgs, "@@@%x", GT_OK);/*may replace later*/

    return GT_OK;
}

/**
* @internal appRefDxChNewTtiTcamSupportSet function
* @endinternal
*
* @brief   set the useAppOffset for new TTI TCAM mode.
*
* @param[in] useAppOffset         - whether to take the TTI offset in TCAM into consideration
*/
GT_VOID appRefDxChNewTtiTcamSupportSet
(
    IN  GT_BOOL             useAppOffset
)
{
    appRefTtiTcamUseAppdemoOffset = useAppOffset;
}

/**
* @internal appRefDxChNewTtiTcamSupportGet function
* @endinternal
*
* @brief   get the utilNewTtiTcamSupportEnable for new TTI TCAM mode.
*
* @param[out] useAppdemoBase           - (pointer to) whether to take the TTI offset in TCAM
*                                      into consideration
*
* @retval GT_OK                    - OK
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS appRefDxChNewTtiTcamSupportGet
(
    OUT GT_BOOL *useAppBase
)
{
    CPSS_NULL_PTR_CHECK_MAC(useAppBase);

    *useAppBase = appRefTtiTcamUseAppdemoOffset;

    return GT_OK;
}

/**
* @internal appRefDxChNewPclTcamSupportSet function
* @endinternal
*
* @brief   set the appRefPclTcamUseIndexConversion for new PCL TCAM mode.
*
* @param[in] enableIndexConversion    - whether to convert PCL TCAM index
*/
GT_VOID appRefDxChNewPclTcamSupportSet
(
    IN  GT_BOOL             enableIndexConversion
)
{
    appRefPclTcamUseIndexConversion = enableIndexConversion;
}

/**
* @internal appRefDxChNewPclTcamSupportGet function
* @endinternal
*
* @brief   get the appRefPclTcamUseIndexConversion for new PCL TCAM mode.
*
* @param[out] enableIndexConversion    - (pointer to) whether to convert PCL TCAM index
*
* @retval GT_OK                    - OK
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS appRefDxChNewPclTcamSupportGet
(
    OUT GT_BOOL *enableIndexConversion
)
{
    CPSS_NULL_PTR_CHECK_MAC(enableIndexConversion);

    *enableIndexConversion = appRefPclTcamUseIndexConversion;

    return GT_OK;
}

/**
* @internal appRefDxChTcamTtiConvertedIndexGet function
* @endinternal
*
* @brief   Gets TCAM converted index for client TTI
*
* @param[in] devNum                   - device number
* @param[in] index                    - global line  in TCAM to write to.
*                                       The converted rule index.
*/
GT_U32 appRefDxChTcamTtiConvertedIndexGet
(
    IN     GT_U8                            devNum,
    IN     GT_U32                           index
)
{
    GT_BOOL useAppOffset;
    GT_U32  ttiMaxIndex;
    GT_U32  tcamFloorsNum;
    GT_STATUS st = GT_OK;

    /* Call cpssDxChCfgTableNumEntriesGet. */
    st = cpssDxChCfgTableNumEntriesGet(devNum, CPSS_DXCH_CFG_TABLE_TTI_TCAM_E,&ttiMaxIndex);

    if (st != GT_OK)
    {
        return 0;
    }

    appRefDxChNewTtiTcamSupportGet(&useAppOffset);

    if(PRV_CPSS_SIP_5_CHECK_MAC(devNum) == GT_TRUE)
    {
        tcamFloorsNum = ttiMaxIndex / CPSS_DXCH_TCAM_MAX_NUM_RULES_PER_FLOOR_CNS;

        if (useAppOffset == GT_TRUE)
        {
            if(tcamFloorsNum == 3)
            {
                /* TTI_0 get only half floor. TTI rules may be on indexes 0, 3 on a floor */
                return (((index >> 1)*12) + ((index & 1) * 3) + appRefTcamTtiHit0RuleBaseIndexOffset);
            }
            else
            {
                return ((index*3) + appRefTcamTtiHit0RuleBaseIndexOffset);
            }
        }
        else
        {
            if ((index*3) >= ttiMaxIndex)
                return 3*(index/3);
            else
                return (index*3);
        }
    }
    else
    {
        return index;
    }
}

/**
* @internal appRefDxChTcamClientBaseIndexGet function
* @endinternal
*
* @brief   Gets TCAM base index for Client rules
*
* @param[in] devNum                   - device number
* @param[in] hitNum                   - hit index, applicable range depends on device
*                                       The converted rule index.
*/
GT_U32 appRefDxChTcamClientBaseIndexGet
(
    IN     GT_U8                            devNum,
    IN     CPSS_DXCH_TCAM_CLIENT_ENT        client,
    IN     GT_U32                           hitNum
)
{
    if((PRV_CPSS_IS_DEV_EXISTS_MAC(devNum) == 0) ||
       PRV_CPSS_SIP_5_CHECK_MAC(devNum) == GT_FALSE)
    {
        return 0;
    }
    if(client == CPSS_DXCH_TCAM_TTI_E)
    {
        switch (hitNum)
        {
            case 0: return appRefTcamTtiHit0RuleBaseIndexOffset;
            case 1: return appRefTcamTtiHit1RuleBaseIndexOffset;
            case 2: return appRefTcamTtiHit2RuleBaseIndexOffset;
            case 3: return appRefTcamTtiHit3RuleBaseIndexOffset;
            default: return 0;
        }
    }
    else
    if(client == CPSS_DXCH_TCAM_IPCL_0_E)/*no hitNum support */
    {
        return appRefTcamIpcl0RuleBaseIndexOffset;
    }
    else
    if(client == CPSS_DXCH_TCAM_IPCL_1_E)/*no hitNum support */
    {
        return appRefTcamIpcl1RuleBaseIndexOffset;
    }
    else
    if(client == CPSS_DXCH_TCAM_IPCL_2_E)/*no hitNum support */
    {
        return appRefTcamIpcl2RuleBaseIndexOffset;
    }
    else
    if(client == CPSS_DXCH_TCAM_EPCL_E)/*no hitNum support */
    {
        return appRefTcamEpclRuleBaseIndexOffset;
    }

    return 0;
}

/**
* @internal appRefDxChTcamClientNumOfIndexsGet function
* @endinternal
*
* @brief   Gets TCAM number of indexes for Client rules
*
* @param[in] devNum                   - device number
* @param[in] hitNum                   - hit index, applicable range depends on device
*                                       TCAM number of indexes for TTI rules.
*/
GT_U32 appRefDxChTcamClientNumOfIndexsGet
(
    IN     GT_U8                            devNum,
    IN     CPSS_DXCH_TCAM_CLIENT_ENT        client,
    IN     GT_U32                           hitNum
)
{
    GT_U32 numOfIndexes = 0;

    if((PRV_CPSS_IS_DEV_EXISTS_MAC(devNum) == 0) ||
        PRV_CPSS_SIP_5_CHECK_MAC(devNum) == GT_FALSE)
    {
        return 0;
    }

    if(client == CPSS_DXCH_TCAM_TTI_E)
    {
        switch (hitNum)
        {
            case 0:
                numOfIndexes = appRefTcamTtiHit0MaxNum;
                break;

            case 1:
                numOfIndexes = appRefTcamTtiHit1MaxNum; /* one floor used */
                break;

            case 2:
                numOfIndexes = appRefTcamTtiHit2MaxNum;
                break;

            case 3:
                numOfIndexes = appRefTcamTtiHit3MaxNum;
                break;

            case 0xFF:
                numOfIndexes = appRefTcamTtiHit0MaxNum +
                                appRefTcamTtiHit1MaxNum +
                                appRefTcamTtiHit2MaxNum +
                                appRefTcamTtiHit3MaxNum;
                break;
            default:
                numOfIndexes = 0;
                break;
        }
    }
    else
    if(client == CPSS_DXCH_TCAM_IPCL_0_E)/*no hitNum support */
    {
        numOfIndexes = appRefTcamIpcl0MaxNum;
    }
    else
    if(client == CPSS_DXCH_TCAM_IPCL_1_E)/*no hitNum support */
    {
        numOfIndexes = appRefTcamIpcl1MaxNum;
    }
    else
    if(client == CPSS_DXCH_TCAM_IPCL_2_E)/*no hitNum support */
    {
        numOfIndexes = appRefTcamIpcl2MaxNum;
    }
    else
    if(client == CPSS_DXCH_TCAM_EPCL_E)/*no hitNum support */
    {
        numOfIndexes = appRefTcamEpclMaxNum;
    }

    return numOfIndexes;
}

/**
* @internal appRefDxChTcamIpclBaseIndexGet function
* @endinternal
*
* @brief   Gets TCAM base index for IPCL0/1/2 rules
*
* @param[in] devNum                   - device number
* @param[in] lookupId                 - lookup 0/1/2
*                                       The converted rule index.
*/
GT_U32 appRefDxChTcamIpclBaseIndexGet
(
    IN     GT_U8                            devNum,
    IN     GT_U32                           lookupId
)
{
    return
        appRefDxChTcamClientBaseIndexGet(devNum ,
            CPSS_DXCH_TCAM_IPCL_0_E + lookupId ,
            0);
}

/**
* @internal appRefDxChTcamIpclNumOfIndexsGet function
* @endinternal
*
* @brief   Gets TCAM number of indexes for IPCL0/1/2 rules
*
* @param[in] devNum                   - device number
* @param[in] lookupId                 - lookup 0/1/2
*                                       TCAM number of indexes for IPCL0/1/2 rules.
*/
GT_U32 appRefDxChTcamIpclNumOfIndexsGet
(
    IN     GT_U8                            devNum,
    IN     GT_U32                           lookupId
)
{
    return
        appRefDxChTcamClientNumOfIndexsGet(devNum ,
            CPSS_DXCH_TCAM_IPCL_0_E + lookupId ,
            0);
}

/**
* @internal appRefDxChTcamEpclBaseIndexGet function
* @endinternal
*
* @brief   Gets TCAM base index for EPCL rules
*
* @param[in] devNum                   - device number
*                                       The converted rule index.
*/
GT_U32 appRefDxChTcamEpclBaseIndexGet
(
    IN     GT_U8                            devNum
)
{
    return
        appRefDxChTcamClientBaseIndexGet(devNum ,
            CPSS_DXCH_TCAM_EPCL_E ,
            0);
}

/**
* @internal appRefDxChTcamEpclNumOfIndexsGet function
* @endinternal
*
* @brief   Gets TCAM number of indexes for EPCL rules
*
* @param[in] devNum                   - device number
*                                       TCAM number of indexes for EPCL rules.
*/
GT_U32 appRefDxChTcamEpclNumOfIndexsGet
(
    IN     GT_U8                            devNum
)
{
    return
        appRefDxChTcamClientNumOfIndexsGet(devNum ,
            CPSS_DXCH_TCAM_EPCL_E ,
            0);
}


/**
* @internal appRefDxChTcamTtiBaseIndexGet function
* @endinternal
*
* @brief   Gets TCAM base index for TTI rules
*
* @param[in] devNum                   - device number
* @param[in] hitNum                   - hit index, applicable range depends on device
*                                       The converted rule index.
*/
GT_U32 appRefDxChTcamTtiBaseIndexGet
(
    IN     GT_U8                            devNum,
    IN     GT_U32                           hitNum
)
{
    return appRefDxChTcamClientBaseIndexGet(devNum,CPSS_DXCH_TCAM_TTI_E,hitNum);
}

/**
* @internal appRefDxChTcamTtiNumOfIndexsGet function
* @endinternal
*
* @brief   Gets TCAM number of indexes for TTI rules
*
* @param[in] devNum                   - device number
* @param[in] hitNum                   - hit index, applicable range depends on device
*                                       TCAM number of indexes for TTI rules.
*/
GT_U32 appRefDxChTcamTtiNumOfIndexsGet
(
    IN     GT_U8                            devNum,
    IN     GT_U32                           hitNum
)
{
    return appRefDxChTcamClientNumOfIndexsGet(devNum,CPSS_DXCH_TCAM_TTI_E,hitNum);
}

/* save TCAM 'Sections' (base+size) for TTI and IPCL/EPLC */
GT_VOID appRefDxChTcamSectionsSave(GT_VOID)
{
    save_appRefTcamTtiHit0RuleBaseIndexOffset  = appRefTcamTtiHit0RuleBaseIndexOffset;
    save_appRefTcamTtiHit1RuleBaseIndexOffset  = appRefTcamTtiHit1RuleBaseIndexOffset;
    save_appRefTcamTtiHit2RuleBaseIndexOffset  = appRefTcamTtiHit2RuleBaseIndexOffset;
    save_appRefTcamTtiHit3RuleBaseIndexOffset  = appRefTcamTtiHit3RuleBaseIndexOffset;

    save_appRefTcamTtiHit0MaxNum               = appRefTcamTtiHit0MaxNum;
    save_appRefTcamTtiHit1MaxNum               = appRefTcamTtiHit1MaxNum;
    save_appRefTcamTtiHit2MaxNum               = appRefTcamTtiHit2MaxNum;
    save_appRefTcamTtiHit3MaxNum               = appRefTcamTtiHit3MaxNum;

    save_appRefTcamIpcl0RuleBaseIndexOffset    = appRefTcamIpcl0RuleBaseIndexOffset;
    save_appRefTcamIpcl1RuleBaseIndexOffset    = appRefTcamIpcl1RuleBaseIndexOffset;
    save_appRefTcamIpcl2RuleBaseIndexOffset    = appRefTcamIpcl2RuleBaseIndexOffset;
    save_appRefTcamEpclRuleBaseIndexOffset     = appRefTcamEpclRuleBaseIndexOffset;

    save_appRefTcamIpcl0MaxNum                 = appRefTcamIpcl0MaxNum;
    save_appRefTcamIpcl1MaxNum                 = appRefTcamIpcl1MaxNum;
    save_appRefTcamIpcl2MaxNum                 = appRefTcamIpcl2MaxNum;
    save_appRefTcamEpclMaxNum                  = appRefTcamEpclMaxNum;

    return;
}

/* restore TCAM 'Sections' (base+size) for TTI and IPCL/EPLC */
GT_VOID appRefDxChTcamSectionsRestore(GT_VOID)
{
    appRefTcamTtiHit0RuleBaseIndexOffset  = save_appRefTcamTtiHit0RuleBaseIndexOffset;
    appRefTcamTtiHit1RuleBaseIndexOffset  = save_appRefTcamTtiHit1RuleBaseIndexOffset;
    appRefTcamTtiHit2RuleBaseIndexOffset  = save_appRefTcamTtiHit2RuleBaseIndexOffset;
    appRefTcamTtiHit3RuleBaseIndexOffset  = save_appRefTcamTtiHit3RuleBaseIndexOffset;

    appRefTcamTtiHit0MaxNum               = save_appRefTcamTtiHit0MaxNum;
    appRefTcamTtiHit1MaxNum               = save_appRefTcamTtiHit1MaxNum;
    appRefTcamTtiHit2MaxNum               = save_appRefTcamTtiHit2MaxNum;
    appRefTcamTtiHit3MaxNum               = save_appRefTcamTtiHit3MaxNum;

    appRefTcamIpcl0RuleBaseIndexOffset    = save_appRefTcamIpcl0RuleBaseIndexOffset;
    appRefTcamIpcl1RuleBaseIndexOffset    = save_appRefTcamIpcl1RuleBaseIndexOffset;
    appRefTcamIpcl2RuleBaseIndexOffset    = save_appRefTcamIpcl2RuleBaseIndexOffset;
    appRefTcamEpclRuleBaseIndexOffset     = save_appRefTcamEpclRuleBaseIndexOffset;

    appRefTcamIpcl0MaxNum                 = save_appRefTcamIpcl0MaxNum;
    appRefTcamIpcl1MaxNum                 = save_appRefTcamIpcl1MaxNum;
    appRefTcamIpcl2MaxNum                 = save_appRefTcamIpcl2MaxNum;
    appRefTcamEpclMaxNum                  = save_appRefTcamEpclMaxNum;
}

/**
* @internal appRefDxChTcamClientSectionSet function
* @endinternal
*
* @brief   Sets TCAM Section for Client rules : Base index + Num Of Indexes
*/
GT_STATUS appRefDxChTcamClientSectionSet
(
    IN     CPSS_DXCH_TCAM_CLIENT_ENT        client,
    IN     GT_U32                           hitNum,
    IN     GT_U32                           baseIndex,
    IN     GT_U32                           numOfIndexes
)
{
    switch(client)
    {
        case CPSS_DXCH_TCAM_TTI_E:
            switch(hitNum)
            {
                case 0:
                    appRefTcamTtiHit0RuleBaseIndexOffset = baseIndex;
                    appRefTcamTtiHit0MaxNum = numOfIndexes;
                    break;
                case 1:
                    appRefTcamTtiHit1RuleBaseIndexOffset = baseIndex;
                    appRefTcamTtiHit1MaxNum = numOfIndexes;
                    break;
                case 2:
                    appRefTcamTtiHit2RuleBaseIndexOffset = baseIndex;
                    appRefTcamTtiHit2MaxNum = numOfIndexes;
                    break;
                case 3:
                    appRefTcamTtiHit3RuleBaseIndexOffset = baseIndex;
                    appRefTcamTtiHit3MaxNum = numOfIndexes;
                    break;
                default:
                    return GT_BAD_PARAM;
            }

            return GT_OK;

        case CPSS_DXCH_TCAM_IPCL_0_E:
            appRefTcamIpcl0RuleBaseIndexOffset = baseIndex;
            appRefTcamIpcl0MaxNum = numOfIndexes;
            return GT_OK;
        case CPSS_DXCH_TCAM_IPCL_1_E:
            appRefTcamIpcl1RuleBaseIndexOffset = baseIndex;
            appRefTcamIpcl1MaxNum = numOfIndexes;
            return GT_OK;
        case CPSS_DXCH_TCAM_IPCL_2_E:
            appRefTcamIpcl2RuleBaseIndexOffset = baseIndex;
            appRefTcamIpcl2MaxNum = numOfIndexes;
            return GT_OK;
        case CPSS_DXCH_TCAM_EPCL_E:
            appRefTcamEpclRuleBaseIndexOffset = baseIndex;
            appRefTcamEpclMaxNum = numOfIndexes;
            return GT_OK;
        default:
            return GT_BAD_PARAM;
    }
}


/**
* @internal pclConvertedIndexGet function
* @endinternal
*
* @brief   Gets TCAM converted index for client PCL
*
* @param[in] devNum                   - device number
* @param[in] index                    - global line  in TCAM to write to.
* @param[in] ruleSize                 - rule size in TCAM
*                                       The converted rule index.
*/
GT_U32 pclConvertedIndexGet
(
    IN     GT_U8                                devNum,
    IN     GT_U32                               index,
    IN     CPSS_DXCH_TCAM_RULE_SIZE_ENT         ruleSize,
    IN     GT_BOOL                              calledFromGaltisWrapper
)
{
    GT_BOOL   enableIndexConversion;
    GT_BOOL   alreadyWithBaseAddr = HARD_WIRE_TCAM_MAC(devNum) ? GT_TRUE : GT_FALSE;
    GT_U32   finalIndex;
    GT_U32   origBaseIndex = 0;
    GT_U32   relativeIndex = index;

    if(calledFromGaltisWrapper == GT_TRUE)
    {
        /* GaltisWrappers must hold 0 based value that need full multiple of the index */
        alreadyWithBaseAddr = GT_FALSE;
    }

    appRefDxChNewPclTcamSupportGet(&enableIndexConversion);
    if((PRV_CPSS_IS_DEV_EXISTS_MAC(devNum) == 0) ||
        PRV_CPSS_SIP_5_CHECK_MAC(devNum) == GT_TRUE)
    {
        if (enableIndexConversion == GT_TRUE)
        {
            if(alreadyWithBaseAddr == GT_TRUE)
            {
                if(index >= appRefTcamIpcl0RuleBaseIndexOffset &&
                   (index - appRefTcamIpcl0RuleBaseIndexOffset) < appRefTcamIpcl0MaxNum)
                {
                    /* rule belongs to ipcl0 */
                    origBaseIndex = appRefTcamIpcl0RuleBaseIndexOffset;
                }
                else
                if(index >= appRefTcamIpcl1RuleBaseIndexOffset &&
                   (index - appRefTcamIpcl1RuleBaseIndexOffset) < appRefTcamIpcl1MaxNum)
                {
                    /* rule belongs to ipcl1 */
                    origBaseIndex = appRefTcamIpcl1RuleBaseIndexOffset;
                }
                else
                if(index >= appRefTcamIpcl2RuleBaseIndexOffset &&
                   (index - appRefTcamIpcl2RuleBaseIndexOffset) < appRefTcamIpcl2MaxNum)
                {
                    /* rule belongs to ipcl2 */
                    origBaseIndex = appRefTcamIpcl2RuleBaseIndexOffset;
                }
                else
                if(index >= appRefTcamEpclRuleBaseIndexOffset &&
                   (index - appRefTcamEpclRuleBaseIndexOffset) < appRefTcamEpclMaxNum)
                {
                    /* rule belongs to epcl */
                    origBaseIndex = appRefTcamEpclRuleBaseIndexOffset;
                }
                else
                {
                    /*error*/
                    origBaseIndex = 0;
                }

                relativeIndex -= origBaseIndex;
            }

            switch(ruleSize)
            {
                case CPSS_DXCH_TCAM_RULE_SIZE_10_B_E:
                    if (PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
                    {
                        /* sip6_10 devices allow to place 10-byte rules only from odd indexes */
                        relativeIndex =  ((relativeIndex * 2) + 1);
                    }
                    else
                    {
                        relativeIndex = relativeIndex;
                    }
                    break;
                case CPSS_DXCH_TCAM_RULE_SIZE_20_B_E:
                    relativeIndex =  (relativeIndex*2);
                    break;
                case CPSS_DXCH_TCAM_RULE_SIZE_40_B_E:
                case CPSS_DXCH_TCAM_RULE_SIZE_50_B_E:
                case CPSS_DXCH_TCAM_RULE_SIZE_60_B_E:
                    relativeIndex =  (relativeIndex*6);
                    break;
                case CPSS_DXCH_TCAM_RULE_SIZE_80_B_E:
                    relativeIndex =  (relativeIndex*12);
                    break;
                case CPSS_DXCH_TCAM_RULE_SIZE_30_B_E:
                default:
                    relativeIndex =  (relativeIndex*3);
                    break;
            }

            if(alreadyWithBaseAddr == GT_FALSE)
            {
                finalIndex =  relativeIndex + appRefTcamPclRuleBaseIndexOffset;
            }
            else
            {
                finalIndex =  relativeIndex + origBaseIndex;
            }

            return finalIndex;
        }
        else
        {
            return index;
        }
    }
    else
    {
        return index;
    }

}

/**
* @internal appRefDxChTcamPclConvertedIndexGet_fromUT function
* @endinternal
*
* @brief   Gets TCAM converted index for client PCL - from UT
*
* @param[in] devNum                   - device number
* @param[in] index                    - global line  in TCAM to write to.
* @param[in] ruleSize                 - rule size in TCAM
*                                       The converted rule index.
*/
GT_U32 appRefDxChTcamPclConvertedIndexGet_fromUT
(
    IN     GT_U8                                devNum,
    IN     GT_U32                               index,
    IN     CPSS_DXCH_TCAM_RULE_SIZE_ENT         ruleSize
)
{
    return pclConvertedIndexGet(devNum,index,ruleSize,GT_FALSE/* called from UT !!!*/);
}

static GT_BOOL                            traceReadHwAccess = GT_FALSE;
/* flag for tracing write hw access */
static GT_BOOL                            traceWriteHwAccess = GT_FALSE;
/* flag for tracing write hw access */
static GT_BOOL                            traceDelayHwAccess = GT_FALSE;

static CPSS_ENABLER_HW_ACCESS_INFO_STC    appRefAccessInfo;
/* Pointer to read and write H/w access DB */
static CPSS_ENABLER_HW_ACCESS_DB_STC     *appRefAccessDbPtr;

static GT_U32                             appRefHwAccessFailCounterLimit = 0;
static GT_U32                             appRefHwWriteAccessCounter = 0;
static GT_BOOL                            appRefInsertHwFailEnable = GT_FALSE;

CPSS_ENABLER_TRACE_OUTPUT_MODE_ENT        appRefTraceOutputMode = CPSS_ENABLER_TRACE_OUTPUT_MODE_DIRECT_E;
/**
* @internal appRefWriteFailerCb function
* @endinternal
*
* @brief   write fail callback function for Hw access.
*
* @param[in] devNum                   -  The PP to read from.
* @param[in] portGroupId              - The port group id. relevant only to 'multi-port-groups'
*                                      devices. Supports value CPSS_PORT_GROUP_UNAWARE_MODE_CNS
* @param[in] regAddr                  - The register's address to read from.
* @param[in] length                   - Number of Words (4 byte) to write.
* @param[in] data                     - The value to be written.
* @param[in] stage                    -  for callback run.
*
* @retval GT_OK                    - callback success to get the data.
* @retval GT_FAIL                  - callback failed.
*
* @note When stage equals to CPSS_DRV_HW_ACCESS_STAGE_PRE_E:
*       1. If callback returns GT_OK the parrent CPSS function continue processing.
*       2. If callback returns GT_ABORTED, this indicates to the parent CPSS function to stop
*       and return GT_OK to its caller.
*
*/
static GT_STATUS appRefWriteFailerCb
(
    IN  GT_U8    devNum,
    IN  GT_U32   portGroupId,
    IN  GT_U32   regAddr,
    IN  GT_U32   length,
    IN  GT_U32   *data,
    IN  CPSS_DRV_HW_ACCESS_STAGE_ENT  stage
)
{

    /* to prevent warnings */
    (void)devNum;
    (void)portGroupId;
    (void)regAddr;
    (void)length;
    (void)data;
    if (appRefInsertHwFailEnable == GT_TRUE)
    {
        if (stage == CPSS_DRV_HW_ACCESS_STAGE_PRE_E)
        {
            appRefHwWriteAccessCounter++;
        }

        if (appRefHwAccessFailCounterLimit > 0)
        {
            if (appRefHwWriteAccessCounter >= appRefHwAccessFailCounterLimit)
            {
                appRefHwWriteAccessCounter = 0;
                appRefHwAccessFailCounterLimit = 0;
                return GT_FAIL;
            }
        }
    }
    return GT_OK;
}


/**
* @internal appRefHwAccessFailerBind function
* @endinternal
*
* @brief   The function binds/unbinds a appRefWriteFailerCb callback for HW write access
*         and set hw write fail counter.
* @param[in] bind                     -  GT_TRUE -  callback routines.
*                                      GT_FALSE - un bind callback routines.
* @param[in] failCounter              - hw write fail counter: set hw write sequence
*                                      number on which write operation should fail.
*
* @retval GT_OK                    - on success
*/
GT_STATUS appRefHwAccessFailerBind
(
    IN GT_BOOL                  bind,
    GT_U32                      failCounter
)
{
    CPSS_DRV_HW_ACCESS_OBJ_STC hwAccessObj;
    GT_STATUS rc;
    osMemSet(&hwAccessObj, 0, sizeof(hwAccessObj));
    hwAccessObj.hwAccessRamWriteFunc = appRefWriteFailerCb;

    rc = cpssDrvHwAccessObjectBind(&hwAccessObj, bind);
    appRefHwWriteAccessCounter = 0;
    if (bind == GT_TRUE)
    {
        appRefInsertHwFailEnable = GT_TRUE;
        appRefHwAccessFailCounterLimit  = failCounter;
    }
    else
    {
        appRefInsertHwFailEnable = GT_FALSE;
        appRefHwAccessFailCounterLimit = 0;
    }
    return rc;
}

/**
* @internal appRefHwAccessCounterGet function
* @endinternal
*
* @brief   The function gets number of hw accesses
*
* @param[out] hwAccessCounterPtr       - points to hw access counter value.
*
* @retval GT_OK                    - on success
*/
GT_STATUS appRefHwAccessCounterGet
(
    OUT GT_U32   *hwAccessCounterPtr
)
{
    *hwAccessCounterPtr = appRefHwWriteAccessCounter;
    return GT_OK;
}

/**
* @internal appRefTraceHwAccessClearDb function
* @endinternal
*
* @brief   Clear HW access db
*
* @retval GT_OK                    - on success else if failed
*/

GT_STATUS appRefTraceHwAccessClearDb
(
    GT_VOID
)
{

    appRefAccessInfo.curSize = 0;
    appRefAccessInfo.corrupted = GT_FALSE;

    return GT_OK;

}

/**
* @internal appRefTraceHwAccessDbIsCorrupted function
* @endinternal
*
* @brief   Check if the HW access data base has reached the limit
*
* @retval GT_OK                    - on success else if failed
*/

GT_STATUS appRefTraceHwAccessDbIsCorrupted
(
    GT_BOOL * corruptedPtr
)
{

   * corruptedPtr = appRefAccessInfo.corrupted ;

   return GT_OK;
}

/**
* @internal appRefSetHwAccessDbActiveState function
* @endinternal
*
* @brief   Update HW Access Read/Write DB state.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad access type
*/
static GT_STATUS appRefSetHwAccessDbActiveState
(
    IN      GT_BOOL                                active,
    INOUT   CPSS_ENABLER_HW_ACCESS_DB_STC      **dbPtrPtr,
    INOUT   CPSS_ENABLER_HW_ACCESS_INFO_STC    *dbInfoPtr
)
{
    if (active == GT_TRUE)
    {
        /* Enable Tracing */

        if (dbInfoPtr->outputToDbIsActive == GT_FALSE)
        {
            if (*dbPtrPtr == NULL)
            {
                /* Initialize HW Access DB for the first time */
                *dbPtrPtr = osMalloc(CPSS_ENABLER_HW_ACCESS_DB_STEP_SIZE_CNS *
                                            sizeof(CPSS_ENABLER_HW_ACCESS_DB_STC));
                if (*dbPtrPtr == NULL)
                {
                    return GT_OUT_OF_CPU_MEM;
                }

                dbInfoPtr->maxSize = CPSS_ENABLER_HW_ACCESS_DB_STEP_SIZE_CNS;

                    /* The Access DB was already initialized.
                    Move cursor to point to the first access entry */
                dbInfoPtr->curSize = 0;
                dbInfoPtr->corrupted = GT_FALSE;
            }

       /* enable HW Access DB */
            dbInfoPtr->outputToDbIsActive = GT_TRUE;


        }
        else
            /* The HW Access DB is already initialized */
            return GT_OK;
    }
    else
    {
        /* Disable Tracing */

        if (dbInfoPtr->outputToDbIsActive == GT_TRUE)
        {
             if ((traceReadHwAccess == GT_FALSE) &&(traceWriteHwAccess == GT_FALSE))
             {
                /* disable HW Access DB */
                dbInfoPtr->outputToDbIsActive = GT_FALSE;
              }
             else
            {
                 /* if one of read or write is disabled, but the other is enabled,
                 the return code should not be failure, it is a legal state */
                 return GT_OK;
            }
        }
        else
            /* The HW Access DB is already disabled */
            return GT_OK;
    }

    return GT_OK;
}

/**
* @internal appRefTraceHwAccessEnable function
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
* @retval GT_BAD_PARAM             - on bad access type
*/
GT_STATUS appRefTraceHwAccessEnable
(
    IN GT_U8                                devNum,
    IN CPSS_ENABLER_TRACE_HW_ACCESS_TYPE_ENT    accessType,
    IN GT_BOOL                              enable
)
{
    GT_STATUS                   rc;
    CPSS_DRV_HW_TRACE_TYPE_ENT  cpssDrvType;
    switch (accessType)
    {
        case CPSS_ENABLER_TRACE_HW_ACCESS_TYPE_READ_E:
            /* Read HW Access */
            traceReadHwAccess = enable;

            if (appRefTraceOutputMode == CPSS_ENABLER_TRACE_OUTPUT_MODE_DB_E)
            {
                /* update Read HW Access DB */
                rc = appRefSetHwAccessDbActiveState(enable,
                                                  &appRefAccessDbPtr,
                                                  &appRefAccessInfo);
                if (rc != GT_OK)
                {
                    return rc;
                }
            }
            cpssDrvType = CPSS_DRV_HW_TRACE_TYPE_READ_E;


            break;
            /* No break*/
        CPSS_COVERITY_NON_ISSUE_BOOKMARK
        /* coverity[unterminated_case] */
        case CPSS_ENABLER_TRACE_HW_ACCESS_TYPE_WRITE_DELAY_E:
            traceDelayHwAccess = enable;

        CPSS_COVERITY_NON_ISSUE_BOOKMARK
             GT_ATTR_FALLTHROUGH;
        /* coverity[fallthrough] */
        case CPSS_ENABLER_TRACE_HW_ACCESS_TYPE_WRITE_E:
            /* Write HW Access */
             traceWriteHwAccess = enable;

            if (appRefTraceOutputMode == CPSS_ENABLER_TRACE_OUTPUT_MODE_DB_E)
            {
                /* update Write HW Access DB */
                rc = appRefSetHwAccessDbActiveState(enable,
                                                  &appRefAccessDbPtr,
                                                  &appRefAccessInfo);
                if (rc != GT_OK)
                {
                    return rc;
                }
            }


            if (accessType==CPSS_ENABLER_TRACE_HW_ACCESS_TYPE_WRITE_DELAY_E)
            {
                cpssDrvType = CPSS_DRV_HW_TRACE_TYPE_WRITE_DELAY_E;
            }
            else
            {
                cpssDrvType = CPSS_DRV_HW_TRACE_TYPE_WRITE_E;
            }

            if (enable==GT_FALSE) /*Delay access stops with write access*/
            {
                traceDelayHwAccess=GT_FALSE;
            }

            break;

        CPSS_COVERITY_NON_ISSUE_BOOKMARK
        /* coverity[unterminated_case] */
        case CPSS_ENABLER_TRACE_HW_ACCESS_TYPE_ALL_E: /* No break*/
            traceDelayHwAccess=enable;

        CPSS_COVERITY_NON_ISSUE_BOOKMARK
             GT_ATTR_FALLTHROUGH;
        /* coverity[fallthrough] */
        case CPSS_ENABLER_TRACE_HW_ACCESS_TYPE_BOTH_E:
            /* Read and Write HW Access */
             traceWriteHwAccess = enable;
            traceReadHwAccess = enable;

            if (appRefTraceOutputMode == CPSS_ENABLER_TRACE_OUTPUT_MODE_DB_E)
            {
                /* update Read HW Access DB */
                rc = appRefSetHwAccessDbActiveState(enable,
                                                  &appRefAccessDbPtr,
                                                  &appRefAccessInfo);
                if (rc != GT_OK)
                {
                    return rc;
                }


            }


            if (accessType==CPSS_ENABLER_TRACE_HW_ACCESS_TYPE_ALL_E)
            {
                cpssDrvType = CPSS_DRV_HW_TRACE_TYPE_ALL_E;
            }
            else
            {
                cpssDrvType = CPSS_DRV_HW_TRACE_TYPE_BOTH_E;
            }

            if (enable==GT_FALSE) /*Delay access stops with write access*/
            {
                traceDelayHwAccess=GT_FALSE;
            }


            break;

        default:
            return GT_BAD_PARAM;
    }

#if 0
    if (enable == GT_FALSE)
    {
        /* trace hw access was disabled */
        #ifdef ASIC_SIMULATION
        if (appRefTraceOutputMode == CPSS_ENABLER_TRACE_OUTPUT_MODE_FILE_E)
        {
            rc = prvAppDemoTraceHwFilesClose();
            if (rc != GT_OK)
            {
                return rc;
            }
        }
        #endif
    }
#endif

    return cpssDrvPpHwTraceEnable(devNum, cpssDrvType, enable);
}


/**
* @internal appRefTraceHwAccessOutputModeSet function
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
GT_STATUS appRefTraceHwAccessOutputModeSet
(
    IN CPSS_ENABLER_TRACE_OUTPUT_MODE_ENT   mode
)
{
    GT_STATUS   rc;

    if (mode >= CPSS_ENABLER_TRACE_OUTPUT_MODE_LAST_E)
    {
        return GT_BAD_PARAM;
    }

    if (appRefTraceOutputMode == mode)
    {
        /* this mode is already set */
        return GT_OK;
    }

    if (appRefAccessInfo.outputToDbIsActive == GT_TRUE)
    {
        /* check that  DB is enabled */
        return GT_BAD_STATE;
    }

    if (mode == CPSS_ENABLER_TRACE_OUTPUT_MODE_DB_E)
    {
        if ((traceReadHwAccess == GT_TRUE) ||(traceWriteHwAccess == GT_TRUE))
        {
            /* update HW Access DB */
            rc = appRefSetHwAccessDbActiveState(GT_TRUE,
                                              &appRefAccessDbPtr,
                                              &appRefAccessInfo);
            if (rc != GT_OK)
            {
                return rc;
            }
        }


    }

#if 0
    if (appRefTraceOutputMode == CPSS_ENABLER_TRACE_OUTPUT_MODE_FILE_E)
    {
        #ifdef ASIC_SIMULATION
        rc = prvAppDemoTraceHwFilesClose();
        if (rc != GT_OK)
        {
            return rc;
        }
        #else
          return GT_NOT_SUPPORTED;
        #endif
    }
#endif

    appRefTraceOutputMode = mode;

    return GT_OK;
}

/**
* @internal appRefTraceHwAccessInfoCompare function
* @endinternal
*
* @brief   Compare given data and parameters with trace hw access DB info on
*         specified index.
* @param[in] dev                      - device number
* @param[in] accessType               - access type: read or write
* @param[in] index                    -  in the hw access db
* @param[in] portGroupId              - port group ID
* @param[in] isrContext               - GT_TRUE: ISR contextt
*                                      GT_FALSE: TASK context
* @param[in] addr                     - memory address space
* @param[in] addr                     - addresses to compare
* @param[in] mask                     -  to  the data from the db
* @param[in] data                     - data (read/written) to compare
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - when there is no match between db info and input
*                                       parameters.
* @retval GT_BAD_STATE             - on unexpected ISR context
*/
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
)
{
    CPSS_ENABLER_HW_ACCESS_DB_STC      *accessDbPtr;
    CPSS_ENABLER_HW_ACCESS_DB_STC       accessDbEntry;
    GT_U8                               writeBit;

    switch (accessType)
    {
        case CPSS_ENABLER_TRACE_HW_ACCESS_TYPE_READ_E:
        case CPSS_ENABLER_TRACE_HW_ACCESS_TYPE_WRITE_E:
            accessDbPtr = appRefAccessDbPtr;
            break;
        default:
            return GT_BAD_PARAM;
    }

    writeBit = (accessType == CPSS_ENABLER_TRACE_HW_ACCESS_TYPE_READ_E)?0x0:0x1;

    accessDbEntry = accessDbPtr[index];

    /* check if the expected context is TASK and it is same as in DB,
       if not skip, this may be not expected ISR context */
    if (isrContext == GT_FALSE)
    {
        /* expected context is TASK */
        if ((BIT2BOOL_MAC((accessDbEntry.accessParamsBmp >> 16) & 0x1)) != isrContext)
        {
            /* skip */
            return GT_BAD_STATE;
        }
    }

    /* parse the data from DB entry and compare to input data and parameters */
    if ((accessDbEntry.addr != addr) || /* hw address */
        ((accessDbEntry.data & mask & accessDbEntry.mask) != (data & accessDbEntry.mask)) || /* hw data */
        ((accessDbEntry.accessParamsBmp & 0xFF) != dev) || /* devNum */
        (((accessDbEntry.accessParamsBmp >> 8) & 0xFF) != portGroupId) || /* portGroupId */
        ((BIT2BOOL_MAC((accessDbEntry.accessParamsBmp >> 16) & 0x1)) != isrContext) || /* isrContext */
        (((accessDbEntry.accessParamsBmp >> 17) & 0x7f) != (GT_U32)addrSpace)|| /* addrSpace */
        (((accessDbEntry.accessParamsBmp >> 26) & 0x1) != writeBit))/*read or write*/
    {
        /* there is no match between parameters or data */
        return GT_BAD_PARAM;
    }

    return GT_OK;
}

/* Supposed the same enable state on all devices */
static GT_BOOL appRefDxAldrin2TailDropDbaEnableSetCalled = GT_FALSE;
static GT_BOOL appRefDxAldrin2TailDropDbaEnableDefault;

/**
* @internal appRefDxAldrin2TailDropDbaEnableSet function
* @endinternal
*
* @brief   Enables/disables Tail Drop DBA.
*          Relevant for Aldrin2 only, does nothing for other devices.
*          On first call saves the found DBA enable state for restore.
*
* @param[in] dev                      -   device number
* @param[in] enable                   -   enable (GT_TRUE) or disable (GT_FALSE) Tail Drop DBA
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS appRefDxAldrin2TailDropDbaEnableSet
(
    IN  GT_U8     dev,
    IN  GT_BOOL   enable
)
{
    GT_STATUS rc;

    /* If not Aldrin2 do nothing */
    if (0 == PRV_CPSS_SIP_5_25_CHECK_MAC(dev)) return GT_OK;
    if (PRV_CPSS_SIP_6_CHECK_MAC(dev))         return GT_OK;

    if (appRefDxAldrin2TailDropDbaEnableSetCalled == GT_FALSE)
    {
        rc =  cpssDxChPortTxDbaEnableGet(dev, &appRefDxAldrin2TailDropDbaEnableDefault);
        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChPortTxDbaEnableGet);
        if (rc != GT_OK)
        {
            return rc;
        }
        appRefDxAldrin2TailDropDbaEnableSetCalled = GT_TRUE;
    }
    return appPlatformDxAldrin2TailDropDbaEnableConfigure(dev, enable);
}

/**
* @internal appRefDxAldrin2TailDropDbaEnableRestore function
* @endinternal
*
* @brief   Restores Tail Drop DBA enable state. Relevant for Aldrin2 only, does nothing for other devices.
*
* @param[in] dev                      -   device number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS appRefDxAldrin2TailDropDbaEnableRestore
(
    IN  GT_U8     dev
)
{
    /* If not Aldrin2 do nothing */
    if (0 == PRV_CPSS_SIP_5_25_CHECK_MAC(dev)) return GT_OK;
    if (PRV_CPSS_SIP_6_CHECK_MAC(dev))         return GT_OK;

    if (appRefDxAldrin2TailDropDbaEnableSetCalled == GT_FALSE)
    {
        return GT_OK;
    }
    return appPlatformDxAldrin2TailDropDbaEnableConfigure(
        dev, appRefDxAldrin2TailDropDbaEnableDefault);
}

/**
* @internal appRefEventFatalErrorEnable function
* @endinternal
*
* @brief   Set fatal error handling type.
*
* @param[in] fatalErrorType           - fatal error handling type
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong fatalErrorType
*/
GT_STATUS appRefEventFatalErrorEnable
(
    CPSS_ENABLER_FATAL_ERROR_TYPE fatalErrorType
)
{
    switch(fatalErrorType)
    {
        case CPSS_ENABLER_FATAL_ERROR_NOTIFY_ONLY_TYPE_E:
        case CPSS_ENABLER_FATAL_ERROR_EXECUTE_TYPE_E:
        case CPSS_ENABLER_FATAL_ERROR_SILENT_TYPE_E:
            prvAppRefFatalErrorType = fatalErrorType;
            break;
        default:
            return GT_BAD_PARAM;
    }
    return GT_OK;
}

GT_STATUS prvAppRefGetDefaultFecMode
(
    IN  CPSS_PORT_INTERFACE_MODE_ENT    mode,
    IN  CPSS_PORT_SPEED_ENT             speed,
    OUT CPSS_PORT_FEC_MODE_ENT          *fecMode
)
{
    if (fecMode == NULL)
    {
        return GT_BAD_PTR;
    }

    if ((mode == CPSS_PORT_INTERFACE_MODE_KR8_E) ||
        (mode == CPSS_PORT_INTERFACE_MODE_CR8_E) ||
        ((mode == CPSS_PORT_INTERFACE_MODE_KR4_E) && (speed == CPSS_PORT_SPEED_200G_E)) ||
        ((mode == CPSS_PORT_INTERFACE_MODE_CR4_E) && (speed == CPSS_PORT_SPEED_200G_E)) ||
        ((mode == CPSS_PORT_INTERFACE_MODE_SR_LR4_E) && (speed == CPSS_PORT_SPEED_200G_E)) ||
        ((mode == CPSS_PORT_INTERFACE_MODE_KR2_E) && (speed == CPSS_PORT_SPEED_100G_E)) ||
        ((mode == CPSS_PORT_INTERFACE_MODE_CR2_E) && (speed == CPSS_PORT_SPEED_100G_E)) ||
        ((mode == CPSS_PORT_INTERFACE_MODE_SR_LR2_E) && (speed == CPSS_PORT_SPEED_100G_E)) ||
        ((mode == CPSS_PORT_INTERFACE_MODE_KR_E)  && (speed == CPSS_PORT_SPEED_50000_E)) ||
        ((mode == CPSS_PORT_INTERFACE_MODE_CR_E)  && (speed == CPSS_PORT_SPEED_50000_E)) ||
        ((mode == CPSS_PORT_INTERFACE_MODE_SR_LR_E)  && (speed == CPSS_PORT_SPEED_50000_E)))
    {
        *fecMode = CPSS_PORT_RS_FEC_544_514_MODE_ENABLED_E;
    } else if (((mode == CPSS_PORT_INTERFACE_MODE_KR4_E) && (speed == CPSS_PORT_SPEED_100G_E)) ||
               ((mode == CPSS_PORT_INTERFACE_MODE_CR4_E) && (speed == CPSS_PORT_SPEED_100G_E)) ||
               ((mode == CPSS_PORT_INTERFACE_MODE_SR_LR4_E) && (speed == CPSS_PORT_SPEED_100G_E)))
    {
        *fecMode = CPSS_PORT_RS_FEC_MODE_ENABLED_E;
    } else
    {
        *fecMode = CPSS_PORT_FEC_MODE_DISABLED_E;
    }
    return GT_OK;
}

/* replace cpssDxChPortModeSpeedSet when working with port manager */
GT_STATUS appRefDxChPortMgrPortModeSpeedSet
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  GT_BOOL                         powerUp,
    IN  CPSS_PORT_INTERFACE_MODE_ENT    ifMode,
    IN  CPSS_PORT_SPEED_ENT             speed
)
{
    GT_STATUS rc;
    CPSS_PORT_FEC_MODE_ENT fecMode;
    CPSS_PORT_MANAGER_STC portEventStc;
    CPSS_PORTS_BMP_STC portsBmp;
    GT_BOOL portMgr;

    if(appDemoDbEntryGet_func)
    {
        appDemoDbEntryGet_func("portMgr", &portMgr);
    }

    if(!portMgr)
    {
        CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portsBmp);
        CPSS_PORTS_BMP_PORT_SET_MAC(&portsBmp, portNum);
        rc = cpssDxChPortModeSpeedSet(devNum, &portsBmp, powerUp,
                                      ifMode,
                                      speed);
        return rc;
    }

    if(powerUp == GT_FALSE)
    {
        portEventStc.portEvent = CPSS_PORT_MANAGER_EVENT_DELETE_E;
    }
    else
    {
        fecMode = CPSS_PORT_FEC_MODE_DISABLED_E;
        portEventStc.portEvent = CPSS_PORT_MANAGER_EVENT_CREATE_E;

        rc = prvAppRefGetDefaultFecMode(ifMode,speed,&fecMode);
        if (rc != GT_OK)
        {
            cpssOsPrintf("appRefDxChPortMgrPortModeSpeedSet FAIL : prvAppDemoGetDefaultFecMode port[%d] rc[%d]\n",
                             portNum,rc);
            return rc;
        }

        rc = cpssDxChSamplePortManagerMandatoryParamsSet(devNum, portNum,
                                                    ifMode,
                                                    speed,
                                                    fecMode);

        if (rc != GT_OK)
        {
            cpssOsPrintf("appRefDxChPortMgrPortModeSpeedSet FAIL : cpssDxChSamplePortManagerMandatoryParamsSet port[%d] rc[%d]\n",
                             portNum,rc);
            return rc;
        }
    }

    rc = cpssDxChPortManagerEventSet(devNum, portNum, &portEventStc);
    if (rc != GT_OK)
    {
        if(powerUp == GT_FALSE)
        {
            cpssOsPrintf("appRefDxChPortMgrPortModeSpeedSet DELETE FAILED : cpssDxChPortManagerEventSet port[%d] rc[%d]\n",
                             portNum,rc);
        }
        else
        {
            cpssOsPrintf("appRefDxChPortMgrPortModeSpeedSet CREATE FAILED : cpssDxChPortManagerEventSet port[%d] rc[%d]\n",
                             portNum,rc);
        }
        cpssOsTimerWkAfter(30);
        /* retry ?! */
        rc = cpssDxChPortManagerEventSet(devNum, portNum, &portEventStc);

        if (rc == GT_OK)
        {
            cpssOsPrintf("appRefDxChPortMgrPortModeSpeedSet retry PASS : cpssDxChPortManagerEventSet port[%d]\n",
                             portNum);
        }
        return rc;
    }

    return GT_OK;
}
