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
* @file appDemoUtils.h
*
* @brief Contains externs for all appDemo functions referenced by mainUT
*
* @version
********************************************************************************
*/
#ifndef __appDemoUtilsh
#define __appDemoUtilsh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <appDemo/sysHwConfig/gtAppDemoSysConfig.h>

#ifdef CHX_FAMILY
#include <appDemo/userExit/dxCh/appDemoDxChEventHandle.h>
#include <cpss/dxCh/dxChxGen/tcam/cpssDxChTcam.h>

extern GT_U32 appDemo_trunk_phy1690_WA_B;

extern GT_STATUS appDemoAldrinPortGobalResourcesPrint
(
    IN GT_U8 dev
);

extern GT_STATUS appDemoBc2DramOrTmInit
(
    IN  GT_U8       dev,
    IN  GT_U32      flags,
    OUT GT_U32     *dramIfBmpPtr
);

extern GT_STATUS appDemoBobk2PortGobalResourcesPrint(GT_U8 dev);

extern GT_STATUS appDemoBobk2PortListResourcesPrint(GT_U8 dev);

extern GT_STATUS appDemoCaelumEgressCntrReset
(
    IN GT_U8        devNum,
    IN GT_U32       cntrSetNum
);

extern GT_STATUS appDemoDxAldrin2TailDropDbaEnableRestore
(
    IN  GT_U8     dev
);

extern GT_STATUS appDemoDxAldrin2TailDropDbaEnableSet
(
    IN  GT_U8     dev,
    IN  GT_BOOL   enable
);

extern GT_VOID appDemoDxChNewTtiTcamSupportSet
(
    IN  GT_BOOL             useAppdemoOffset
);

extern GT_U32 appDemoDxChTcamEpclBaseIndexGet
(
    IN     GT_U8           devNum
);

extern GT_U32 appDemoDxChTcamEpclNumOfIndexsGet
(
    IN     GT_U8           devNum
);

extern GT_U32 appDemoDxChTcamIpclBaseIndexGet
(
    IN     GT_U8           devNum,
    IN     GT_U32          lookupId
);

extern GT_U32 appDemoDxChTcamIpclNumOfIndexsGet
(
    IN     GT_U8           devNum,
    IN     GT_U32          lookupId
);

extern GT_VOID appDemoDxChTcamSectionsRestore(GT_VOID);

extern GT_U32 appDemoDxChTcamTtiBaseIndexGet
(
    IN     GT_U8                            devNum,
    IN     GT_U32                           hitNum
);

extern GT_U32 appDemoDxChTcamTtiConvertedIndexGet
(
    IN     GT_U8                            devNum,
    IN     GT_U32                           index
);

extern GT_U32 appDemoDxChTcamTtiNumOfIndexsGet
(
    IN     GT_U8                            devNum,
    IN     GT_U32                           hitNum
);

extern GT_STATUS appDemoDxHwDevNumChange
(
    IN  GT_U8                       dev,
    IN  GT_HW_DEV_NUM               hwDevNum
);

extern GT_STATUS appDemoFalconIpLpmRamDefaultConfigCalc
(
    IN  GT_U8                                    devNum,
    IN  CPSS_DXCH_CFG_SHARED_TABLE_MODE_ENT      sharedTableMode,
    IN  GT_U32                                   maxNumOfPbrEntries,
    OUT CPSS_DXCH_LPM_RAM_CONFIG_STC             *ramDbCfgPtr
);

extern GT_STATUS appDemoBc2IpLpmRamDefaultConfigCalc
(
    IN  GT_U8                                    devNum,
    IN  GT_U32                                   maxNumOfPbrEntries,
    OUT CPSS_DXCH_LPM_RAM_CONFIG_STC             *ramDbCfgPtr
);

extern GT_U32 appDemoDxChTcamPclConvertedIndexGet_fromUT
(
    IN     GT_U8                                devNum,
    IN     GT_U32                               index,
    IN     CPSS_DXCH_TCAM_RULE_SIZE_ENT         ruleSize
);

extern GT_STATUS appDemoDxChPhase1ParamsGet
(
    OUT CPSS_DXCH_PP_PHASE1_INIT_INFO_STC *ppPhase1ParamsPtr
);

extern GT_STATUS appDemoDxChTcamClientSectionSet
(
    IN     CPSS_DXCH_TCAM_CLIENT_ENT        client,
    IN     GT_U32                           hitNum,
    IN     GT_U32                           baseIndex,
    IN     GT_U32                           numOfIndexes
);

extern GT_STATUS appDemoLion2FabricCleanUp
(
    IN GT_U8              lion2FabricDevNum
);

extern GT_STATUS appDemoLion2FabricConfigureQos
(
    IN GT_U8              lion2FabricDevNum,
    IN GT_U32             toCpuTc,
    IN CPSS_DP_LEVEL_ENT  toCpuDp,
    IN GT_U32             toAnalyzerTc,
    IN CPSS_DP_LEVEL_ENT  toAnalyzerDp,
    IN CPSS_DP_LEVEL_ENT  fromCpuDp
);

extern GT_STATUS appDemoLion2FabricForwardAddBc2Device
(
    IN  GT_U8           lion2FabricDevNum,
    IN  GT_U32          hwDevId,
    IN  GT_PORT_NUM     linkPortNum
);

extern GT_STATUS appDemoLion2FabricForwardAddBc2Vidx
(
    IN  GT_U8                 lion2FabricDevNum,
    IN  GT_U32                bc2Vidx,
    IN  CPSS_PORTS_BMP_STC    *targetPortsBmpPtr
);

extern GT_STATUS appDemoLion2FabricForwardAddBc2Vlan
(
    IN  GT_U8                 lion2FabricDevNum,
    IN  GT_U32                bc2VlanId,
    IN  CPSS_PORTS_BMP_STC    *targetPortsBmpPtr
);

extern GT_STATUS appDemoLion2FabricInit
(
    IN GT_U8              lion2FabricDevNum,
    IN GT_PORT_NUM        cpuLinkPortNum
);

extern GT_STATUS appDemoLionTrunkSrcPortHashMappingSet_ForMultiCoreFdbLookup
(
    IN  GT_U8                       boardRevId
);
#endif

extern GT_BOOL isHirApp;

extern GT_STATUS trunkEPortGet_phy1690_WA_B(
    IN GT_TRUNK_ID              trunkId,
    OUT GT_PORT_NUM              *globalEPortPtr
);

extern void cpssInitSystemGet
(
    OUT GT_U32  *boardIdxPtr,
    OUT GT_U32  *boardRevIdPtr,
    OUT GT_U32  *reloadEepromPtr
);

extern GT_STATUS   appDemoAllowProcessingOfAuqMessages
(
    GT_BOOL     enable
);

extern GT_STATUS appDemoDbEntryAdd
(
    IN  GT_CHAR *namePtr,
    IN  GT_U32  value
);


extern GT_STATUS appDemoDbEntryGet
(
    IN  GT_CHAR *namePtr,
    OUT GT_U32  *valuePtr
);

extern GT_STATUS appDemoDebugDeviceIdReset
(
    IN GT_VOID
);

extern GT_STATUS appDemoDebugDeviceIdSet
(
    IN GT_U8    devNum,
    IN GT_U32   deviceId
);

extern GT_STATUS appDemoDevIdxGet
(
    IN  GT_U8   devNum,
    OUT GT_U32  *devIdxPtr
);

extern GT_STATUS appDemoEventsToTestsHandlerBind
(
    IN  CPSS_UNI_EV_CAUSE_ENT       cpssUniEventArr[],
    IN  GT_U32                      arrLength,
    IN  CPSS_EVENT_MASK_SET_ENT     operation
);

extern GT_STATUS appDemoDeviceSkipEventMaskSet
(
    IN GT_U8                       devNum,
    IN CPSS_EVENT_MASK_SET_ENT     operation
);

extern GT_STATUS appDemoHwAccessCounterGet(
    OUT GT_U32   *hwAccessCounterPtr
);

extern GT_STATUS appDemoHwAccessFailerBind(
    IN GT_BOOL                  bind,
    GT_U32                      failCounter
);

extern GT_BOOL appDemoIsTmEnabled(void);     /* SW */

extern GT_STATUS appDemoIsTmSupported
(
    IN   GT_U8 devNum,
    OUT  GT_BOOL *isTmSupported
);

extern GT_STATUS appDemoEventFatalErrorEnable
(
    CPSS_ENABLER_FATAL_ERROR_TYPE fatalErrorType
);

extern GT_VOID appDemoOsLogStop(void);

extern GT_STATUS appDemoStaticDbEntryGet
(
    IN  GT_CHAR *namePtr,
    OUT GT_U32  *valuePtr
);

extern GT_VOID appDemoSupportAaMessageSet
(
    IN GT_BOOL supportAaMessage
);

extern GT_STATUS appDemoTmGeneral48PortsInit
(
    IN  GT_U8   dev
);

extern GT_STATUS appDemoTmGeneral4PortsInit
(
    IN  GT_U8   dev
);

extern GT_STATUS appDemoTmScenario20Init
(
    IN  GT_U8   dev
);

extern GT_STATUS appDemoTmScenario2Init
(
    IN  GT_U8   dev
);

extern GT_STATUS appDemoTmScenario3Init
(
    IN  GT_U8   dev
);

extern GT_STATUS appDemoTmScenario4Init
(
    IN  GT_U8   dev
);

extern GT_STATUS appDemoTmScenario5Init
(
    IN  GT_U8   dev
);

extern GT_STATUS appDemoTmScenario6Init
(
    IN  GT_U8   dev
);

extern GT_STATUS appDemoTmScenario7Init
(
    IN  GT_U8   dev
);

extern GT_STATUS appDemoTmScenario8Init
(
    IN  GT_U8   dev
);

extern GT_STATUS appDemoTmScenario9Init
(
    IN  GT_U8   dev
);

extern GT_STATUS appDemoTmStressScenarioInit
(
    IN  GT_U8   dev,
    IN  GT_U8   mode
);

extern GT_STATUS appDemoTraceHwAccessClearDb
(
    GT_VOID
);

extern GT_STATUS appDemoTraceHwAccessDbIsCorrupted
(
    GT_BOOL * corrupted
);

extern GT_STATUS appDemoTraceHwAccessEnable
(
    GT_U8                                  devNum,
    CPSS_ENABLER_TRACE_HW_ACCESS_TYPE_ENT      accessType,
    GT_BOOL                                enable
);

extern GT_STATUS appDemoTraceHwAccessInfoCompare
(
    IN GT_U8                                dev,
    CPSS_ENABLER_TRACE_HW_ACCESS_TYPE_ENT       accessType,
    IN GT_U32                               index,
    IN GT_U32                               portGroupId,
    IN GT_BOOL                              isrContext,
    IN CPSS_TRACE_HW_ACCESS_ADDR_SPACE_ENT  addrSpace,
    IN GT_U32                               addr,
    IN GT_U32                               mask,
    IN GT_U32                               data
);

extern GT_STATUS appDemoTraceHwAccessOutputModeSet
(
    IN CPSS_ENABLER_TRACE_OUTPUT_MODE_ENT   mode
);

extern GT_STATUS appDemoFdbManagerGlobalIDSet
(
    IN GT_U32 fdbManagerId
);

extern GT_STATUS appDemoFdbManagerAuMsgHandlerMethod
(
    IN APP_DEMO_AU_MESSAGE_HANDLER_METHORD_ENT auMsgHandler
);

extern GT_STATUS appDemoFdbManagerControlSet
(
    IN APP_DEMO_AU_MESSAGE_HANDLER_METHORD_ENT  auMsgHandler,
    IN GT_BOOL                                  autoAgingEn,
    IN GT_U32                                   autoAgingInterval
);

extern GT_STATUS appDemoFdbManagerAutoAgingStatisticsGet
(
    OUT APP_DEMO_CPSS_DXCH_BRG_FDB_MANAGER_STATISTICS_STC *statistics
);

GT_STATUS appDemoFdbManagerAutoAgingStatisticsClear();

extern GT_STATUS appDemoPrintPortEeeInterruptInfoSet
(
    IN GT_U32   enable
);

extern GT_VOID appDemoDxChNewPclTcamSupportSet
(
    IN  GT_BOOL             enableIndexConversion
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __appDemoUtilsh */

