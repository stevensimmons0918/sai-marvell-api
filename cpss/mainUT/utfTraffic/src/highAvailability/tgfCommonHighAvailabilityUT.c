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
* @file tgfCommonHighAvailabilityUT.c
*
* @brief Enhanced UTs for CPSS High Availability
* @version   1
********************************************************************************
*/


/* HA UT not supported in CAP - commenting the entire file for CAP - since there are many references to appDemo
   Note: Please use wrappers for appDemo functions - Refer wrappers in cpssEnabler/utfWrappers/src/utfExtHelpers.c
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>
#include <common/tgfCommon.h>
#include <trafficEngine/tgfTrafficGenerator.h>
#include <cpss/common/systemRecovery/cpssGenSystemRecovery.h>
#include <appDemo/boardConfig/appDemoBoardConfig.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortManager.h>
#include <common/tgfBridgeGen.h>
#include <ip/prvTgfBasicIpv4UcRouting.h>
#include <ip/prvTgfBasicIpv4McRouting.h>
#include <ip/prvTgfBasicIpv6UcRouting.h>
#include <ip/prvTgfBasicIpv6McRouting.h>
#include <exactMatch/prvTgfExactMatchTtiPclFullPath.h>
#include <cpss/dxCh/dxChxGen/exactMatchManager/private/prvCpssDxChExactMatchManager_db.h>
#include <cpss/dxCh/dxChxGen/trunk/private/prvCpssDxChTrunkHa.h>
#include <cpss/common/systemRecovery/private/prvCpssCommonSystemRecoveryParallel.h>
#include <exactMatchManager/prvTgfExactMatchManagerTtiPclFullPath.h>
#include <common/tgfTrunkGen.h>
#include <cpss/dxCh/dxChxGen/networkIf/cpssDxChNetIf.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>


extern GT_VOID prvTgfExactMatchManagerTtiPclFullPathHaAddBasicConfigSingleEntry
(
    IN GT_U32   exactMatchManagerId,
    IN GT_BOOL  firstCall
);
extern GT_VOID prvTgfExactMatchManagerTtiPclFullPathHaValidityCheckAndTrafficSendSingleEntry
(
    IN GT_U32   exactMatchManagerId
);
extern GT_VOID prvTgfExactMatchManagerTtiPclFullPathHaSystemRecoveryStateSet();

extern GT_VOID prvTgfExactMatchManagerTtiPclFullPathHaRestoreBasicConfigSingleEntry
(
    IN GT_U32   exactMatchManagerId
);

#include <trunk/prvTgfTrunk.h>
#include <cpss/dxCh/dxChxGen/trunk/cpssDxChTrunk.h>

extern GT_STATUS appDemoHaEmulateSwCrash
(
    GT_VOID
);
extern GT_STATUS appDemoEventRequestDrvnModeInit
(
    IN GT_VOID
);
extern GT_VOID tgfBasicTrafficSanity_extern
(
    GT_VOID
);

extern GT_STATUS appDemoDebugDeviceIdSet
(
    IN GT_U8    devNum,
    IN GT_U32   deviceId
);

extern GT_STATUS appDemoDebugDeviceIdGet
(
    IN  GT_U8    devNum,
    OUT GT_U32  *deviceIdPtr
);

extern GT_STATUS cpssGetDevicesValidity
(
    IN  GT_U8   devNum,
    OUT GT_BOOL *deviceValidityPtr
);

#define RECREATION_MAX_SIZE 256

typedef struct{
    GT_BOOL                         vaild;
    CPSS_PORT_INTERFACE_MODE_ENT    ifMode;
    CPSS_PORT_SPEED_ENT             speed;
    CPSS_PORT_FEC_MODE_ENT          fecMode;
    GT_BOOL                         isMacLoopback;

} RECREATION_INFO;


static GT_CHAR_PTR prvTgfHighAvailabilitySpeedToStr
(
    CPSS_PORT_SPEED_ENT speed
)
{   /*add new speed strings here if required*/
    GT_CHAR_PTR ptr =NULL;
    switch(speed)
    {
        case CPSS_PORT_SPEED_10000_E:
            ptr=" 10G";
            break;
        case CPSS_PORT_SPEED_1000_E:
            ptr="  1G";
            break;
        case CPSS_PORT_SPEED_50000_E:
            ptr=" 50G";
            break;
        case CPSS_PORT_SPEED_100G_E:
            ptr="100G";
            break;
        case CPSS_PORT_SPEED_40000_E:
            ptr=" 40G";
            break;
        case CPSS_PORT_SPEED_2500_E:
            ptr="2.5G";
            break;
        default:
            break;
    }

    return ptr;
}

static GT_CHAR_PTR prvTgfHighAvailabilityModeToStr
(
    CPSS_PORT_INTERFACE_MODE_ENT mode
)
{   /*add new mode strings here if required*/
    GT_CHAR_PTR ptr =NULL;
    switch(mode)
    {
        case CPSS_PORT_INTERFACE_MODE_KR_E:
            ptr="   KR";
            break;
        case CPSS_PORT_INTERFACE_MODE_SR_LR_E:
            ptr="SR_LR";
            break;
        case CPSS_PORT_INTERFACE_MODE_KR2_E:
            ptr="  KR2";
            break;
        case CPSS_PORT_INTERFACE_MODE_KR4_E:
            ptr="  KR4";
            break;
        case CPSS_PORT_INTERFACE_MODE_SGMII_E:
            ptr="SGMII";
            break;
         default:
            break;
    }

    return ptr;
}



static GT_CHAR_PTR prvTgfHighAvailabilityFecToStr
(
    CPSS_PORT_FEC_MODE_ENT fecMode
)
{   /*add new fec modes strings here if required*/
    GT_CHAR_PTR ptr =NULL;
    switch(fecMode)
    {
        case CPSS_PORT_FEC_MODE_ENABLED_E:
            ptr="        enabled";
            break;
        case CPSS_PORT_FEC_MODE_DISABLED_E:
            ptr="       disabled";
            break;
        case CPSS_PORT_RS_FEC_MODE_ENABLED_E:
            ptr="     RS enabled";
            break;
        case CPSS_PORT_BOTH_FEC_MODE_ENABLED_E:
            ptr="   both enabled";
            break;
        case CPSS_PORT_RS_FEC_544_514_MODE_ENABLED_E:
            ptr="544_514 enabled";
            break;
         default:
            break;
    }

    return ptr;
}


/**
* @internal prvTgfHighAvailabilityReplayAllPort function
* @endinternal
*
* @brief   configure all device port ,skip cpu and sdma ports
*
*/
static GT_STATUS prvTgfHighAvailabilityReplayAllPort
(
    RECREATION_INFO *recreationRequiredPtr
)
{
    GT_STATUS                       st = 0 ;
    GT_U32                          portNum,maxPhysicalPort;
    CPSS_PORT_INTERFACE_MODE_ENT    ifMode;
    CPSS_PORT_SPEED_ENT             speed;
    CPSS_PORT_FEC_MODE_ENT          fecMode;
    CPSS_PORT_MANAGER_STC           portEventStc;
    GT_U8                           i;
    GT_CHAR_PTR                     speedStr,modeStr,feqStr;
    GT_BOOL                         valid;

    for (i = 0; i < PRV_CPSS_MAX_PP_DEVICES_CNS; i++)
    {

        st = cpssGetDevicesValidity(i,&valid);
        if (GT_OK != st)
        {
            return st;
        }

        if( valid == GT_FALSE)
        {
            continue;
        }


        maxPhysicalPort = PRV_CPSS_DXCH_MAX_PHY_PORT_NUMBER_MAC(i);
        PRV_UTF_LOG2_MAC("Start replay for ports device-%d .Loop until %d ports\n",i,maxPhysicalPort);
        for (portNum = 0; portNum < maxPhysicalPort; portNum++)
        {
            if (recreationRequiredPtr[portNum].vaild == GT_FALSE)
            {
                continue;
            }

            speed = recreationRequiredPtr[portNum].speed;
            ifMode = recreationRequiredPtr[portNum].ifMode;
            fecMode = recreationRequiredPtr[portNum].fecMode;

            speedStr = prvTgfHighAvailabilitySpeedToStr(speed);
            modeStr = prvTgfHighAvailabilityModeToStr(ifMode);
            feqStr = prvTgfHighAvailabilityFecToStr(fecMode);

            if((NULL == speedStr)||(NULL == modeStr)||(NULL == feqStr))
            {
                PRV_UTF_LOG4_MAC("Port %3d params - speed [%d] ,ifMode [%d],fecMode [%d]\n",portNum,speed,ifMode,fecMode);
            }
            else
            {
                PRV_UTF_LOG4_MAC("Port %3d params - speed [%s] ,ifMode [%s],fecMode [%s]\n",portNum,speedStr,modeStr,feqStr);
            }

            st = cpssDxChSamplePortManagerMandatoryParamsSet(i, portNum, ifMode, speed ,fecMode);
            if (GT_OK != st)
            {
                PRV_UTF_LOG4_MAC("cpssDxChSamplePortManagerMandatoryParamsSet FAILED:st-%d,portNum-%d,ifMode-%d,speed-%d\n",
                                                        st, portNum, ifMode, speed);
                return st;
            }

            if(recreationRequiredPtr[portNum].isMacLoopback == GT_TRUE)
            {
                st = cpssDxChSamplePortManagerLoopbackSet(i, portNum, CPSS_PORT_MANAGER_LOOPBACK_TYPE_MAC_E,
                                                            GT_TRUE , CPSS_PORT_SERDES_LOOPBACK_DISABLE_E, GT_FALSE);
                if (GT_OK != st)
                {
                    PRV_UTF_LOG4_MAC("cpssDxChSamplePortManagerLoopbackSet FAILED:st-%d,portNum-%d,ifMode-%d,speed-%d\n",
                                                    st, portNum, ifMode, speed);
                    return st;
                }
            }

            /*recreate port*/
            portEventStc.portEvent = CPSS_PORT_MANAGER_EVENT_CREATE_E;
            st = cpssDxChPortManagerEventSet(i,portNum, &portEventStc);
            if (st !=GT_OK)
            {
                PRV_UTF_LOG1_MAC("=> FAIL to recreate port %d\n",portNum);
                PRV_UTF_LOG3_MAC("cpssDxChPortManagerEventSet FAILED:st-%d,portNum-%d,portEvent %d \n",
                                                st, portNum, portEventStc.portEvent);
                return st;
            }
        }
    }
    return st;
}

static GT_STATUS prvTgfHighAvailabilityInitPhase
(
    RECREATION_INFO *recreationRequiredPtr
)
{
    GT_STATUS                       st = 0 ;
    GT_U32                          portNum,maxPhysicalPort;
    CPSS_DXCH_DETAILED_PORT_MAP_STC portMapShadow;
    GT_BOOL                         portValidity = GT_FALSE;
    GT_BOOL                         MacValidity = GT_FALSE;
    CPSS_PM_PORT_PARAMS_STC         portParams;

    maxPhysicalPort = PRV_CPSS_DXCH_MAX_PHY_PORT_NUMBER_MAC(prvTgfDevNum);

    for (portNum = 0; portNum < maxPhysicalPort; portNum++)
    {
        st = cpssDxChPortPhysicalPortDetailedMapGet(prvTgfDevNum, portNum, &portMapShadow);
        if (st !=GT_OK)
        {
            PRV_UTF_LOG3_MAC("cpssDxChPortPhysicalPortDetailedMapGet FAILED:st-%d devNum %d portNum %d",
                                          st, prvTgfDevNum,portNum);
            return st;
        }

        portValidity = portMapShadow.valid;
        /* skip 'CPU SDMA' and 'remote' */
        MacValidity = (portMapShadow.portMap.mappingType != CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E) ? GT_FALSE/* no MAC */ : GT_TRUE/*MAC*/;

        if ((portValidity == GT_FALSE) || (MacValidity == GT_FALSE))
        {
            continue;
        }

        if (portMapShadow.portMap.mappingType== CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E)
        {
            st = cpssDxChPortManagerPortParamsGet(prvTgfDevNum, portNum,&portParams);
            if (GT_OK != st)
            {
                if(st== GT_NOT_INITIALIZED)
                {
                    continue;
                }
                else
                {
                    PRV_UTF_LOG2_MAC("cpssDxChPortManagerPortParamsGet FAILED:st-%d,portNum-%d\n",
                                                            st, portNum);
                    return st;
                }
            }

            recreationRequiredPtr[portNum].vaild = GT_TRUE;
            recreationRequiredPtr[portNum].speed = portParams.portParamsType.regPort.speed;
            recreationRequiredPtr[portNum].ifMode = portParams.portParamsType.regPort.ifMode;
            recreationRequiredPtr[portNum].fecMode = portParams.portParamsType.regPort.portAttributes.fecMode;
            recreationRequiredPtr[portNum].isMacLoopback = portParams.portParamsType.regPort.portAttributes.loopback.loopbackMode.macLoopbackEnable;
        }
    }
    return st;
}



/**
* @internal prvTgHighAvailabilityStatusSet function
* @endinternal
*
* @brief set default system Recovery Info for high availablity
*
* [in] stage      - select the stage of system recovery
*
*/

/* DO NOT MAKE this function STATIC  ,we may want to use it from shell in order to debug*/
GT_STATUS prvTgHighAvailabilityStatusSet
(
        GT_U32  stage,
        GT_BOOL parallelMode
)
{
    CPSS_SYSTEM_RECOVERY_INFO_STC   systemRecoveryInfoSet;
    GT_STATUS                       st = 0 ;

    /*set default values for High Availability */
    systemRecoveryInfoSet.systemRecoveryMode.continuousRx = GT_FALSE;
    systemRecoveryInfoSet.systemRecoveryMode.continuousTx = GT_FALSE;
    systemRecoveryInfoSet.systemRecoveryMode.continuousAuMessages = GT_FALSE;
    systemRecoveryInfoSet.systemRecoveryMode.continuousFuMessages = GT_FALSE;
    systemRecoveryInfoSet.systemRecoveryMode.haCpuMemoryAccessBlocked = GT_FALSE;
    systemRecoveryInfoSet.systemRecoveryMode.ha2phasesInitPhase = CPSS_SYSTEM_RECOVERY_HA_2_PHASES_INIT_NONE_E;
    systemRecoveryInfoSet.systemRecoveryMode.haReadWriteState = CPSS_SYSTEM_RECOVERY_HA_STATE_READ_ENABLE_WRITE_ENABLE_E;
    systemRecoveryInfoSet.systemRecoveryProcess = CPSS_SYSTEM_RECOVERY_PROCESS_HA_E;

    switch (stage)
    {
    case 1:
        PRV_UTF_LOG0_MAC("\n Set system recovery state CPSS_SYSTEM_RECOVERY_INIT_STATE_E \n");
        systemRecoveryInfoSet.systemRecoveryState = CPSS_SYSTEM_RECOVERY_INIT_STATE_E;
        break;
    case 2:
        PRV_UTF_LOG0_MAC("\n Set system recovery state CPSS_SYSTEM_RECOVERY_HW_CATCH_UP_STATE_E \n");
        systemRecoveryInfoSet.systemRecoveryState = CPSS_SYSTEM_RECOVERY_HW_CATCH_UP_STATE_E;
        break;
    case 3:
        PRV_UTF_LOG0_MAC("\n Set system recovery state CPSS_SYSTEM_RECOVERY_COMPLETION_STATE_E \n");
        systemRecoveryInfoSet.systemRecoveryState = CPSS_SYSTEM_RECOVERY_COMPLETION_STATE_E;
        break;
    default:
        return GT_NOT_SUPPORTED;
        break;

    }

    if (parallelMode == GT_TRUE)
        systemRecoveryInfoSet.systemRecoveryProcess = CPSS_SYSTEM_RECOVERY_PROCESS_PARALLEL_HA_E;
    st = cpssSystemRecoveryStateSet(&systemRecoveryInfoSet);
    return st;
}


/**
* @internal prvTgHighAvailabilityStatusSet function
* @endinternal
*
* @brief run test for ipv4/6 uc and mc
*
* [in] testNum      - select the test number
*                     1-ipv4 uc
*                     2-ipv6 uc
*                     3-ipv4 mc
*                     4-ipv6 mc
*
*/
static GT_VOID tgfHighAvailabilityParallelLpm4_6_Uc_Mc
(
    GT_U32 testNum
)
{

    GT_BOOL                             portMng;
    GT_STATUS                           st = 0 ;
    GT_U32                              prvUtfVrfId = 0;
    GT_U32                              prvTgfLpmDBId = 0;
    CPSS_SYSTEM_RECOVERY_STATE_ENT      state;
    GT_U32                              numOfManagers = 1;
    CPSS_SYSTEM_RECOVERY_MANAGER_ENT    managerListArray[CPSS_SYSTEM_RECOVERY_LAST_MANAGER_E];
    RECREATION_INFO                     *recreationRequired=NULL;

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, (~(UTF_ALDRIN2_E|UTF_FALCON_E | UTF_AC5P_E |UTF_AC5X_E | UTF_HARRIER_E )))

    st = cpssDxChPortManagerEnableGet(prvTgfDevNum,&portMng);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "ERROR : cpssPxPortManagerEnableGet FAILED:st-%d",st);
    if (portMng == GT_FALSE)
    {
        PRV_UTF_LOG0_MAC("tgfHighAvailabilityParallelLpm4_6_Uc_Mc only support port manager ");
        SKIP_TEST_MAC
    }

    managerListArray [0]= CPSS_SYSTEM_RECOVERY_LPM_MANAGER_E ;
    switch (testNum)
    {
    case 1:

        prvTgfBasicIpv4UcRoutingBaseConfigurationSet(prvTgfPortsArray[1]);
        prvTgfBasicIpv4UcRoutingRouteConfigurationSet(prvUtfVrfId, 0,
                                                  prvTgfPortsArray[3], GT_FALSE,
                                                  GT_FALSE, GT_FALSE, GT_FALSE);
        prvTgfBasicIpv4UcRoutingTrafficGenerate(prvTgfPortsArray[0], prvTgfPortsArray[3], PRV_TGF_DEFAULT_BOOLEAN_CNS, PRV_TGF_DEFAULT_BOOLEAN_CNS, PRV_TGF_DEFAULT_BOOLEAN_CNS, PRV_TGF_DEFAULT_BOOLEAN_CNS, GT_FALSE);
        break;
    case 2:
        prvTgfBasicIpv6UcRoutingBaseConfigurationSet();
        prvTgfBasicIpv6UcRoutingRouteConfigurationSet(0, GT_FALSE);
        prvTgfBasicIpv6UcRoutingTrafficGenerate(GT_FALSE,GT_TRUE,prvTgfPortsArray[0]);
        break;
    case 3 :
        prvTgfBasicIpv4McRoutingBaseConfigurationSet();
        prvTgfBasicIpv4McRoutingRouteConfigurationSet();
        prvTgfBasicIpv4McRoutingTrafficGenerate(GT_TRUE);
        break;
    case 4 :
        prvTgfBasicIpv6McRoutingBaseConfigurationSet();
        prvTgfBasicIpv6McRoutingRouteConfigurationSet(128);
        prvTgfBasicIpv6McRoutingTrafficGenerate(PRV_TGF_DEFAULT_BOOLEAN_CNS);
        break;
    default:
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, GT_BAD_PARAM);
    }

    recreationRequired = cpssOsMalloc(sizeof(RECREATION_INFO)*RECREATION_MAX_SIZE);
    if (recreationRequired == NULL)
    {
        st = GT_OUT_OF_CPU_MEM;
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, " error at recreationRequired cpssOsMalloc ");
    }
    cpssOsMemSet(recreationRequired, 0, sizeof(RECREATION_INFO)*RECREATION_MAX_SIZE);
    st = prvTgfHighAvailabilityInitPhase(recreationRequired);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "ERROR : prvTgfHighAvailabilityInitPhase FAILED:st-%d devNum %d",st,prvTgfDevNum);

    st = appDemoHaEmulateSwCrash();
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "ERROR : appDemoHaEmulateSwCrash FAILED:st-%d devNum %d",st,prvTgfDevNum);

    st = prvTgHighAvailabilityStatusSet(1,GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "ERROR : prvTgHighAvailabilityStatusSet :st-%d ",st);

    PRV_UTF_LOG0_MAC("\nInit with port configuration \n");

    st = cpssReInitSystem();
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "ERROR : cpssReInitSystem FAILED:st-%d ", st);
    prvUtfSetAfterSystemResetState();

    st = prvTgHighAvailabilityStatusSet(2,GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "ERROR : prvTgHighAvailabilityStatusSet :st-%d ",st);

    st = prvTgfHighAvailabilityReplayAllPort(recreationRequired);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "ERROR : tfgHighAvailabilityReplayAllPort :st-%d ", st);

    st = prvTgHighAvailabilityStatusSet(3,GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "ERROR : prvTgHighAvailabilityStatusSet :st-%d ",st);

    PRV_UTF_LOG0_MAC("\nStart event handlers.\n");
    st = appDemoEventRequestDrvnModeInit();
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "ERROR : appDemoEventRequestDrvnModeInit :st-%d ",st);
    st = cpssDxChNetIfRestore(prvTgfDevNum);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "ERROR : cpssDxChNetIfRestore :st-%d ",st);


    /*clear the default lpm manager */
    st = prvTgfIpLpmDBDevsListRemove(prvTgfLpmDBId, &prvTgfDevNum, 1);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "ERROR : prvTgfIpLpmDBDevsListRemove FAILED:st-%d devNum %d",st,prvTgfDevNum);

    state = CPSS_SYSTEM_RECOVERY_HW_CATCH_UP_STATE_E;
    st = cpssSystemRecoveryHaParallelModeSet(state,numOfManagers,managerListArray);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "ERROR : cpssSystemRecoveryHaParallelModeSet FAILED: state-%d manager %d", state, managerListArray[0]);

    st = prvTgfIpLpmDBDevListAdd(prvTgfLpmDBId, &prvTgfDevNum, 1);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "ERROR : cpssDxChIpLpmDBDevListAdd FAILED:st-%d devNum %d",st,prvTgfDevNum);

    /* replay route configuration */
    switch (testNum)
    {
    case 1:
        prvTgfBasicIpv4UcRoutingRouteConfigurationSet(prvUtfVrfId, 0,
                                                  prvTgfPortsArray[3], GT_FALSE,
                                                  GT_FALSE, GT_FALSE, GT_FALSE);
        break;
    case 2:
        prvTgfBasicIpv6UcRoutingRouteConfigurationSet(0, GT_FALSE);
        break;
    case 3 :
        prvTgfBasicIpv4McRoutingRouteConfigurationSet();
        break;
    case 4 :
        prvTgfBasicIpv6McRoutingRouteConfigurationSet(128);
        break;
    default:
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, GT_BAD_PARAM);
    }

    state = CPSS_SYSTEM_RECOVERY_COMPLETION_STATE_E;
    st = cpssSystemRecoveryHaParallelModeSet(state,numOfManagers,managerListArray);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "ERROR : cpssSystemRecoveryHaParallelModeSet FAILED: state-%d manager %d", state, managerListArray[0]);

   /* Generate traffic check and restore */
     switch (testNum)
    {
    case 1:
        prvTgfBasicIpv4UcRoutingTrafficGenerate(prvTgfPortsArray[0], prvTgfPortsArray[3],
                                                 PRV_TGF_DEFAULT_BOOLEAN_CNS, PRV_TGF_DEFAULT_BOOLEAN_CNS,
                                                 PRV_TGF_DEFAULT_BOOLEAN_CNS, PRV_TGF_DEFAULT_BOOLEAN_CNS, GT_FALSE);
        /* for SIP5 we have a stack issue overflow - after the problem will be fixed we can return this check */
        if (PRV_CPSS_SIP_6_CHECK_MAC(prvTgfDevNum))
        {
            st = prvTgfIpLpmValidityCheck(prvTgfLpmDBId, 0, CPSS_IP_PROTOCOL_IPV4_E, CPSS_UNICAST_E,GT_TRUE);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "fail on prvTgfIpLpmValidityCheck ipv4\n");
        }
        prvTgfBasicIpv4UcRoutingConfigurationRestore(prvUtfVrfId, 0, PRV_TGF_DEFAULT_U8_CNS, GT_FALSE);
    break;
    case 2:
        prvTgfBasicIpv6UcRoutingTrafficGenerate(GT_FALSE,GT_TRUE,prvTgfPortsArray[0]);
        /* for SIP5 we have a stack issue overflow - after the problem will be fixed we can return this check */
        if (PRV_CPSS_SIP_6_CHECK_MAC(prvTgfDevNum))
        {
            st = prvTgfIpLpmValidityCheck(prvTgfLpmDBId, 0, CPSS_IP_PROTOCOL_IPV6_E, CPSS_UNICAST_E,GT_TRUE);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "fail on prvTgfIpLpmValidityCheck ipv4\n");
        }
        prvTgfBasicIpv6UcRoutingConfigurationRestore(prvUtfVrfId, GT_FALSE);
        break;
    case 3 :
        prvTgfBasicIpv4McRoutingTrafficGenerate(GT_TRUE);
        /* for SIP5 we have a stack issue overflow - after the problem will be fixed we can return this check */
        if (PRV_CPSS_SIP_6_CHECK_MAC(prvTgfDevNum))
        {
            st = prvTgfIpLpmValidityCheck(prvTgfLpmDBId, 0, CPSS_IP_PROTOCOL_IPV4_E, CPSS_MULTICAST_E,GT_TRUE);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "fail on prvTgfIpLpmValidityCheck ipv4\n");
        }
        prvTgfBasicIpv4McRoutingConfigurationRestore();
        break;
    case 4 :
        prvTgfBasicIpv6McRoutingTrafficGenerate(PRV_TGF_DEFAULT_BOOLEAN_CNS);
        /* for SIP5 we have a stack issue overflow - after the problem will be fixed we can return this check */
        if (PRV_CPSS_SIP_6_CHECK_MAC(prvTgfDevNum))
        {
            st = prvTgfIpLpmValidityCheck(prvTgfLpmDBId, 0, CPSS_IP_PROTOCOL_IPV6_E, CPSS_MULTICAST_E,GT_TRUE);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "fail on prvTgfIpLpmValidityCheck ipv4\n");
        }
        prvTgfBasicIpv6McRoutingConfigurationRestore();
        break;
    default:
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, GT_BAD_PARAM);
    }
    if(recreationRequired)
        cpssOsFree(recreationRequired);
}


UTF_TEST_CASE_MAC(tgfHighAvailabilitySimpleTest)
{
    /*
      1. Set high availablity basic configuration:
          - reset SW
          - set system recovery status to CPSS_SYSTEM_RECOVERY_INIT_STATE_E
          - reInitSysem with same configurations
          - set system recovery status to CPSS_SYSTEM_RECOVERY_HW_CATCH_UP_STATE_E
          - replay ports for port manager DB
          - set system recovery status to CPSS_SYSTEM_RECOVERY_COMPLETION_STATE_E

      2. run simple test to verify system restore
          - BasicTrafficSanity

  */

    GT_BOOL                         portMng;
    GT_STATUS                       st = 0 ;
    RECREATION_INFO                 *recreationRequired=NULL;
#ifndef CPSS_APP_PLATFORM_REFERENCE /*not supported  */
    GT_U32                          devId =0;
#endif

    PRV_TGF_SUPPORTED_DEV_DECLARE_MAC(UTF_FALCON_E | UTF_ALDRIN2_E | UTF_AC5P_E | UTF_AC5X_E | UTF_HARRIER_E | UTF_AC5_E )
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, (~(UTF_ALDRIN2_E|UTF_FALCON_E | UTF_ALDRIN_E |
                                                         UTF_AC3X_E | UTF_AC5P_E |UTF_AC5X_E | UTF_HARRIER_E | UTF_AC5_E )))

#ifndef CPSS_APP_PLATFORM_REFERENCE /*not supported  */
    st=appDemoDebugDeviceIdGet(prvTgfDevNum,&devId);
    if(st==GT_OK)
    {
        PRV_UTF_LOG1_MAC("Get Dev id configured  = 0x%x ",devId);
    }
#endif
    st = cpssDxChPortManagerEnableGet(prvTgfDevNum,&portMng);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "ERROR : cpssPxPortManagerEnableGet FAILED:st-%d",st);
    if (portMng == GT_FALSE)
    {
        PRV_UTF_LOG0_MAC("tgfHighAvailabilitySimpleTest only support port manager ");
        SKIP_TEST_MAC
    }

    recreationRequired = cpssOsMalloc(sizeof(RECREATION_INFO)*RECREATION_MAX_SIZE);
    if (recreationRequired == NULL)
    {
        st = GT_OUT_OF_CPU_MEM;
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, " error at recreationRequired cpssOsMalloc ");
    }
    cpssOsMemSet(recreationRequired, 0,sizeof(RECREATION_INFO)*RECREATION_MAX_SIZE);

    st = prvTgfHighAvailabilityInitPhase(recreationRequired);

    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "ERROR : prvTgfHighAvailabilityInitPhase FAILED:st-%d devNum %d",st,prvTgfDevNum);

    st = appDemoHaEmulateSwCrash();
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "ERROR : appDemoHaEmulateSwCrash FAILED:st-%d devNum %d",st,prvTgfDevNum);

    st = prvTgHighAvailabilityStatusSet(1,GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "ERROR : prvTgHighAvailabilityStatusSet :st-%d ",st);

    PRV_UTF_LOG0_MAC("\nInit with port configuration \n");

#ifndef CPSS_APP_PLATFORM_REFERENCE /*not supported  */
    if(devId)
    {
        PRV_UTF_LOG1_MAC("Set Dev id configured  = 0x%x ",devId);
        appDemoDebugDeviceIdSet(prvTgfDevNum,devId);
    }
#endif

    st = cpssReInitSystem();
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "ERROR : cpssReInitSystem FAILED:st-%d ", st);
    prvUtfSetAfterSystemResetState();

    st = prvTgHighAvailabilityStatusSet(2,GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "ERROR : prvTgHighAvailabilityStatusSet :st-%d ",st);

    st = prvTgfHighAvailabilityReplayAllPort(recreationRequired);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "ERROR : tfgHighAvailabilityReplayAllPort :st-%d ", st);

    st = prvTgHighAvailabilityStatusSet(3,GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "ERROR : prvTgHighAvailabilityStatusSet :st-%d ",st);

    PRV_UTF_LOG0_MAC("\nStart event handlers.\n");
    st = appDemoEventRequestDrvnModeInit();
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "ERROR : appDemoEventRequestDrvnModeInit :st-%d ",st);

    st = cpssDxChNetIfRestore(prvTgfDevNum);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "ERROR : cpssDxChNetIfRestore :st-%d ",st);
    PRV_UTF_LOG0_MAC("\n flush FDB ");
    /* flush FDB include static entries */
    prvTgfBrgFdbFlush(GT_TRUE);

    /*run test to verify system restore */
    tgfBasicTrafficSanity_extern();

    PRV_UTF_LOG0_MAC("\n flush FDB ");
    /* flush FDB include static entries */
    prvTgfBrgFdbFlush(GT_TRUE);

    if(recreationRequired)
        cpssOsFree(recreationRequired);
}

UTF_TEST_CASE_MAC(tgfHighAvailabilityLpmSimpleTest)
{
    /*
      1. Set LPM configuration
      2. Check traffic
      3. Do HA:
          - reset SW
          - set system recovery status to CPSS_SYSTEM_RECOVERY_INIT_STATE_E
          - reInitSysem with same configurations
          - set system recovery status to CPSS_SYSTEM_RECOVERY_HW_CATCH_UP_STATE_E
          - replay ports for port manager DB
          - replay LPM configuration
          - set system recovery status to CPSS_SYSTEM_RECOVERY_COMPLETION_STATE_E
      4. Check LPM configuration after HA, compare with lpm configuration before HA.
      5. Check traffic
  */

    GT_BOOL                         portMng;
    GT_STATUS                       st = 0 ;
    GT_U32                          prvUtfVrfId = 0;
    GT_U32                          prvTgfLpmDBId = 0;
    RECREATION_INFO                 *recreationRequired=NULL;

    PRV_TGF_SUPPORTED_DEV_DECLARE_MAC(UTF_FALCON_E|UTF_ALDRIN2_E | UTF_AC5P_E | UTF_AC5X_E | UTF_HARRIER_E)
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, (~(UTF_ALDRIN2_E|UTF_FALCON_E  | UTF_AC5P_E | UTF_AC5X_E | UTF_HARRIER_E )))
    st = cpssDxChPortManagerEnableGet(prvTgfDevNum,&portMng);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "ERROR : cpssPxPortManagerEnableGet FAILED:st-%d",st);
    if (portMng == GT_FALSE)
    {
        PRV_UTF_LOG0_MAC("tgfHighAvailabilitySimpleTest only support port manager ");
        SKIP_TEST_MAC
    }

    /* Set Base configuration */
    prvTgfBasicIpv4UcRoutingBaseConfigurationSet(prvTgfPortsArray[1]);

    /* Set Route configuration for vrfId[0] */
    prvTgfBasicIpv4UcRoutingRouteConfigurationSet(prvUtfVrfId, 0,
                                                  prvTgfPortsArray[3], GT_FALSE,
                                                  GT_FALSE, GT_FALSE, GT_FALSE);
        /* Generate traffic */
    prvTgfBasicIpv4UcRoutingTrafficGenerate(prvTgfPortsArray[0], prvTgfPortsArray[3], PRV_TGF_DEFAULT_BOOLEAN_CNS, PRV_TGF_DEFAULT_BOOLEAN_CNS, PRV_TGF_DEFAULT_BOOLEAN_CNS, PRV_TGF_DEFAULT_BOOLEAN_CNS, GT_FALSE);

    recreationRequired = cpssOsMalloc(sizeof(RECREATION_INFO)*RECREATION_MAX_SIZE);
    if (recreationRequired == NULL)
    {
        st = GT_OUT_OF_CPU_MEM;
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, " error at recreationRequired cpssOsMalloc ");
    }
    cpssOsMemSet(recreationRequired, 0,sizeof(RECREATION_INFO)*RECREATION_MAX_SIZE);
    st = prvTgfHighAvailabilityInitPhase(recreationRequired);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "ERROR : prvTgfHighAvailabilityInitPhase FAILED:st-%d devNum %d",st,prvTgfDevNum);

    st = appDemoHaEmulateSwCrash();
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "ERROR : appDemoHaEmulateSwCrash FAILED:st-%d devNum %d",st,prvTgfDevNum);

    st = prvTgHighAvailabilityStatusSet(1,GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "ERROR : prvTgHighAvailabilityStatusSet :st-%d ",st);

    PRV_UTF_LOG0_MAC("\nInit with port configuration \n");

    st = cpssReInitSystem();
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "ERROR : cpssReInitSystem FAILED:st-%d ", st);
    prvUtfSetAfterSystemResetState();

    st = prvTgHighAvailabilityStatusSet(2,GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "ERROR : prvTgHighAvailabilityStatusSet :st-%d ",st);

    st = prvTgfHighAvailabilityReplayAllPort(recreationRequired);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "ERROR : tfgHighAvailabilityReplayAllPort :st-%d ", st);
    /* replay route configuration */
    prvTgfBasicIpv4UcRoutingRouteConfigurationSet(prvUtfVrfId, 0,
                                                  prvTgfPortsArray[3], GT_FALSE,
                                                  GT_FALSE, GT_FALSE, GT_FALSE);

    st = prvTgHighAvailabilityStatusSet(3,GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "ERROR : prvTgHighAvailabilityStatusSet :st-%d ",st);

    PRV_UTF_LOG0_MAC("\nStart event handlers.\n");
    st = appDemoEventRequestDrvnModeInit();
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "ERROR : appDemoEventRequestDrvnModeInit :st-%d ",st);

    st = cpssDxChNetIfRestore(prvTgfDevNum);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "ERROR : cpssDxChNetIfRestore :st-%d ",st);
    /* Generate traffic */
    prvTgfBasicIpv4UcRoutingTrafficGenerate(prvTgfPortsArray[0], prvTgfPortsArray[3], PRV_TGF_DEFAULT_BOOLEAN_CNS, PRV_TGF_DEFAULT_BOOLEAN_CNS, PRV_TGF_DEFAULT_BOOLEAN_CNS, PRV_TGF_DEFAULT_BOOLEAN_CNS, GT_FALSE);

    /* for SIP5 we have a stack issue overflow - after the problem will be fixed we can return this check */
    if (PRV_CPSS_SIP_6_CHECK_MAC(prvTgfDevNum))
    {
        /* check router configuration */
        st = prvTgfIpLpmValidityCheck(prvTgfLpmDBId, 0, CPSS_IP_PROTOCOL_IPV4_E, CPSS_UNICAST_E,GT_TRUE);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "fail on prvTgfIpLpmValidityCheck ipv4\n");
    }
    /* Restore configuration */
    prvTgfBasicIpv4UcRoutingConfigurationRestore(prvUtfVrfId, 0, PRV_TGF_DEFAULT_U8_CNS, GT_FALSE);
    if(recreationRequired)
        cpssOsFree(recreationRequired);
}

UTF_TEST_CASE_MAC(tgfHighAvailabilityExactMatchManagerSimpleTest)
{
    /*
      1. Set Exact Match Manager configuration
      2. Check traffic
      3. Do HA:
          - reset SW
          - set system recovery status to CPSS_SYSTEM_RECOVERY_INIT_STATE_E
          - reInitSysem with same configurations
          - set system recovery status to CPSS_SYSTEM_RECOVERY_HW_CATCH_UP_STATE_E
          - replay ports for port manager DB
          - replay Exact Match Manager configuration
          - set system recovery status to CPSS_SYSTEM_RECOVERY_COMPLETION_STATE_E
      4. Check EMM configuration after HA, compare with EMM configuration before HA.
      5. Check traffic
  */

    GT_U32                          exactMatchManagerId = 25;
    GT_BOOL                         portMng;
    GT_STATUS                       st = 0 ;
    RECREATION_INFO                 *recreationRequired=NULL;

    PRV_TGF_SUPPORTED_DEV_DECLARE_MAC(UTF_FALCON_E | UTF_AC5P_E | UTF_AC5X_E | UTF_HARRIER_E)
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC( prvTgfDevNum, (~(UTF_FALCON_E | UTF_AC5P_E | UTF_AC5X_E | UTF_HARRIER_E )) )
    st = cpssDxChPortManagerEnableGet(prvTgfDevNum,&portMng);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "ERROR : cpssPxPortManagerEnableGet FAILED:st-%d",st);
    if (portMng == GT_FALSE)
    {
        PRV_UTF_LOG0_MAC("tgfHighAvailabilitySimpleTest only support port manager ");
        SKIP_TEST_MAC
    }

    /* Set Base configuration */
    prvTgfExactMatchManagerTtiPclFullPathHaAddBasicConfigSingleEntry(exactMatchManagerId,GT_TRUE);
    /* Generate traffic */
    prvTgfExactMatchManagerTtiPclFullPathHaValidityCheckAndTrafficSendSingleEntry(exactMatchManagerId);

    recreationRequired = cpssOsMalloc(sizeof(RECREATION_INFO)*RECREATION_MAX_SIZE);
    if (recreationRequired == NULL)
    {
        st = GT_OUT_OF_CPU_MEM;
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, " error at recreationRequired cpssOsMalloc ");
    }
    cpssOsMemSet(recreationRequired, 0, sizeof(RECREATION_INFO)*RECREATION_MAX_SIZE);
    st = prvTgfHighAvailabilityInitPhase(recreationRequired);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "ERROR : prvTgfHighAvailabilityInitPhase FAILED:st-%d devNum %d",st,prvTgfDevNum);

    st = appDemoHaEmulateSwCrash();
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "ERROR : appDemoHaEmulateSwCrash FAILED:st-%d devNum %d",st,prvTgfDevNum);

    st = prvTgHighAvailabilityStatusSet(1,GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "ERROR : prvTgHighAvailabilityStatusSet :st-%d ",st);

    PRV_UTF_LOG0_MAC("\nInit with port configuration \n");

    st = cpssReInitSystem();
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "ERROR : cpssReInitSystem FAILED:st-%d ", st);
    prvUtfSetAfterSystemResetState();

    st = prvTgHighAvailabilityStatusSet(2,GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "ERROR : prvTgHighAvailabilityStatusSet :st-%d ",st);

    /* delete all managers, devices and database configuration - delete only from SW and not from HW */
    st = prvCpssDxChExactMatchManagerLibraryShutdown();
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "ERROR : prvCpssDxChExactMatchManagerLibraryShutdown FAILED:st-%d devNum %d",st,prvTgfDevNum);

    st = prvTgfHighAvailabilityReplayAllPort(recreationRequired);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "ERROR : tfgHighAvailabilityReplayAllPort :st-%d ", st);

    /* Replay Exact Match Manager configuration */
    prvTgfExactMatchManagerTtiPclFullPathHaAddBasicConfigSingleEntry(exactMatchManagerId,GT_FALSE);

    st = prvTgHighAvailabilityStatusSet(3,GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "ERROR : prvTgHighAvailabilityStatusSet :st-%d ",st);

    PRV_UTF_LOG0_MAC("\nStart event handlers.\n");
    st = appDemoEventRequestDrvnModeInit();
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "ERROR : appDemoEventRequestDrvnModeInit :st-%d ",st);

    st = cpssDxChNetIfRestore(prvTgfDevNum);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "ERROR : cpssDxChNetIfRestore :st-%d ",st);
    /* Generate traffic */
    prvTgfExactMatchManagerTtiPclFullPathHaValidityCheckAndTrafficSendSingleEntry(exactMatchManagerId);

    /* Restore configuration */
    prvTgfExactMatchManagerTtiPclFullPathHaRestoreBasicConfigSingleEntry(exactMatchManagerId);
    if(recreationRequired)
        cpssOsFree(recreationRequired);
}




UTF_TEST_CASE_MAC(tgfHighAvailabilityTrunkSimpleTest)
{
    /*
      1. Set high availablity basic configuration:
      1.1 set 2 regular trunk groups and 1 cascade
      1.2 reset SW
      1.3 set system recovery status to CPSS_SYSTEM_RECOVERY_INIT_STATE_E
      1.4 reInitSysem with same configurations
      1.5 set system recovery status to CPSS_SYSTEM_RECOVERY_HW_CATCH_UP_STATE_E
      1.6 replay ports for port manager DB
      1.7 replay trunk groups with diffrent setting
          group 1 with other port member,
          group 2 cascade with more port members in it
          group 50 diffrent members order
      1.8 set system recovery status to CPSS_SYSTEM_RECOVERY_COMPLETION_STATE_E

      2 verify all trunk groups are fix
        - hw DB updated for groups 1,2
        - sw DB updated for group 50
  */

    GT_BOOL                 portMng;
    GT_STATUS               st = GT_OK;

    GT_TRUNK_ID             trunkIdArry[3];
    GT_U32                  numOfEnabledMembers[2],numOfEnabledMembersGet, ii;
    GT_U32                  numOfDisabledMembers[2];
    CPSS_TRUNK_MEMBER_STC   enabledMembersArray[CPSS_TRUNK_MAX_NUM_OF_MEMBERS_CNS] = {{47,16},{46,16},{45,16},{111,16}};
    CPSS_TRUNK_MEMBER_STC   disabledMembersArray[CPSS_TRUNK_MAX_NUM_OF_MEMBERS_CNS] ={{4,16},{5,16}};
    CPSS_TRUNK_MEMBER_STC   enabledMembersArray2[CPSS_TRUNK_MAX_NUM_OF_MEMBERS_CNS] = {{100,17},{101,17},{102,16},{103,16},{104,16}};
    CPSS_TRUNK_MEMBER_STC   oldMemberArray[CPSS_TRUNK_MAX_NUM_OF_MEMBERS_CNS] = {{100,17},{101,17},{102,16},{103,16},{104,16}};
    CPSS_TRUNK_MEMBER_STC   hwMembersArray[CPSS_TRUNK_MAX_NUM_OF_MEMBERS_CNS];
    CPSS_PORTS_BMP_STC      cascadePorts={{0,0}} ,cascadePortsGet={{0,0}};
    RECREATION_INFO         *recreationRequired=NULL;
    GT_HW_DEV_NUM           hwDevice = PRV_CPSS_HW_DEV_NUM_MAC(prvTgfDevNum);

    PRV_TGF_SUPPORTED_DEV_DECLARE_MAC((UTF_ALDRIN2_E|UTF_FALCON_E | UTF_AC5P_E | UTF_AC5X_E | UTF_HARRIER_E))
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, (~(UTF_ALDRIN2_E|UTF_FALCON_E| UTF_AC5P_E | UTF_AC5X_E | UTF_HARRIER_E)))

    st = cpssDxChPortManagerEnableGet(prvTgfDevNum,&portMng);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "ERROR : cpssPxPortManagerEnableGet FAILED:st-%d",st);
    if (portMng == GT_FALSE)
    {
        PRV_UTF_LOG0_MAC("tgfHighAvailabilitySimpleTest only support port manager ");
        SKIP_TEST_MAC
    }

    cpssOsMemCpy(oldMemberArray, enabledMembersArray2, sizeof(enabledMembersArray2));

    trunkIdArry[0] = 1;
    trunkIdArry[1]=50;
    trunkIdArry[2]=2;
    /* Set configuration */

    numOfEnabledMembers[0]  = 4;
    numOfDisabledMembers[0] = 2;

    /*update hwDevice number */
    for (ii=0;ii<numOfEnabledMembers[0];ii++)
    {
         enabledMembersArray[ii].hwDevice=hwDevice;
    }
    for (ii=0;ii<numOfDisabledMembers[0];ii++)
    {
         disabledMembersArray[ii].hwDevice=hwDevice;
    }

    st = cpssDxChTrunkMembersSet(prvTgfDevNum, trunkIdArry[0], numOfEnabledMembers[0], enabledMembersArray,numOfDisabledMembers[0], disabledMembersArray);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "ERROR : cpssDxChTrunkMembersSet :st-%d ", st);

    cascadePorts.ports[1] = 0xf00;
    st = cpssDxChTrunkCascadeTrunkPortsSet(prvTgfDevNum,trunkIdArry[2],&cascadePorts);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "ERROR : cpssDxChTrunkCascadeTrunkPortsSet :st-%d ", st);

    numOfEnabledMembers[1]  = 5;
    numOfDisabledMembers[1] = 0;
    /*update hwDevice number */
    for (ii=2;ii<numOfEnabledMembers[1];ii++)
    {
         oldMemberArray[ii].hwDevice = enabledMembersArray2[ii].hwDevice = hwDevice ;
    }
    oldMemberArray[0].hwDevice = enabledMembersArray2[0].hwDevice = hwDevice+1; /*diffrent hwDevice*/
    oldMemberArray[1].hwDevice = enabledMembersArray2[1].hwDevice = hwDevice+1; /*diffrent hwDevice*/

    st = cpssDxChTrunkMembersSet(prvTgfDevNum, trunkIdArry[1], numOfEnabledMembers[1], enabledMembersArray2,numOfDisabledMembers[1], disabledMembersArray);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "ERROR : cpssDxChTrunkMembersSet :st-%d ", st);

    recreationRequired = cpssOsMalloc(sizeof(RECREATION_INFO)*RECREATION_MAX_SIZE);
    if (recreationRequired == NULL)
    {
        st = GT_OUT_OF_CPU_MEM;
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, " error at recreationRequired cpssOsMalloc ");
    }
    cpssOsMemSet(recreationRequired, 0,sizeof(RECREATION_INFO)*RECREATION_MAX_SIZE);
    st = prvTgfHighAvailabilityInitPhase(recreationRequired);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "ERROR : prvTgfHighAvailabilityInitPhase FAILED:st-%d devNum %d",st,prvTgfDevNum);

    st = appDemoHaEmulateSwCrash();
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "ERROR : appDemoHaEmulateSwCrash FAILED:st-%d devNum %d",st,prvTgfDevNum);

    st = prvTgHighAvailabilityStatusSet(1,GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "ERROR : prvTgHighAvailabilityStatusSet :st-%d ",st);

    PRV_UTF_LOG0_MAC("\nInit with port configuration \n");

    st = cpssReInitSystem();
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "ERROR : cpssReInitSystem FAILED:st-%d ", st);
    prvUtfSetAfterSystemResetState();

    st = prvTgHighAvailabilityStatusSet(2,GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "ERROR : prvTgHighAvailabilityStatusSet :st-%d ",st);

    st = prvTgfHighAvailabilityReplayAllPort(recreationRequired);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "ERROR : tfgHighAvailabilityReplayAllPort :st-%d ", st);

    /*change trunk port members in replay -make hw and sw db diffrent*/
    enabledMembersArray[2].port = 20;
    enabledMembersArray[2].hwDevice = hwDevice;
    cascadePorts.ports[0] = 0xf00;

    /*change only the inpuy order*/
    enabledMembersArray2[2].port = oldMemberArray[4].port;
    enabledMembersArray2[2].hwDevice = oldMemberArray[4].hwDevice;
    enabledMembersArray2[4].port = oldMemberArray[2].port;
    enabledMembersArray2[4].hwDevice = oldMemberArray[2].hwDevice;

    /* Set configuration */
    st = cpssDxChTrunkMembersSet(prvTgfDevNum, trunkIdArry[1], numOfEnabledMembers[1], enabledMembersArray2,numOfDisabledMembers[1], disabledMembersArray);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "ERROR : cpssDxChTrunkMembersSet :st-%d ", st);

    st = cpssDxChTrunkMembersSet(prvTgfDevNum, trunkIdArry[0], numOfEnabledMembers[0], enabledMembersArray,numOfDisabledMembers[0], disabledMembersArray);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "ERROR : cpssDxChTrunkMembersSet :st-%d ", st);

    st = cpssDxChTrunkCascadeTrunkPortsSet(prvTgfDevNum, trunkIdArry[2], &cascadePorts);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "ERROR : cpssDxChTrunkCascadeTrunkPortsSet :st-%d ", st);

    st = prvTgHighAvailabilityStatusSet(3,GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "ERROR : prvTgHighAvailabilityStatusSet :st-%d ",st);

    PRV_UTF_LOG0_MAC("\nStart event handlers.\n");
    st = appDemoEventRequestDrvnModeInit();
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "ERROR : appDemoEventRequestDrvnModeInit :st-%d ",st);

    st = cpssDxChNetIfRestore(prvTgfDevNum);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "ERROR : cpssDxChNetIfRestore :st-%d ",st);
    /*verify HW and SW match */
    numOfEnabledMembersGet = CPSS_TRUNK_MAX_NUM_OF_MEMBERS_CNS;
    st = cpssDxChTrunkTableEntryGet(prvTgfDevNum, trunkIdArry[0], &numOfEnabledMembersGet, hwMembersArray);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChTrunkTableEntryGet ");

    if (numOfEnabledMembersGet != numOfEnabledMembers[0])
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, GT_FAIL, "ERROR : numOfEnabledMembersGet != swNumOfEnableMembers ");

    for (ii=0;ii<numOfEnabledMembersGet;ii++)
    {
        /*verify new SW member is set in HW */
        if ((enabledMembersArray[ii].port !=  hwMembersArray[ii].port) ||
         (enabledMembersArray[ii].hwDevice !=  hwMembersArray[ii].hwDevice))
        {
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, GT_FAIL, "ERROR : SW and HW doesnt match ");
        }
    }

    /*verify HW and SW match and sw is set with old Hw config*/
    numOfEnabledMembersGet = CPSS_TRUNK_MAX_NUM_OF_MEMBERS_CNS;
    st = cpssDxChTrunkTableEntryGet(prvTgfDevNum, trunkIdArry[1], &numOfEnabledMembersGet, hwMembersArray);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChTrunkTableEntryGet ");

    st = cpssDxChTrunkDbEnabledMembersGet(prvTgfDevNum,trunkIdArry[1],&numOfEnabledMembers[1],enabledMembersArray2);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChTrunkDbEnabledMembersGet ");

    if (numOfEnabledMembersGet != numOfEnabledMembers[1])
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, GT_FAIL, "ERROR : numOfEnabledMembersGet != swNumOfEnableMembers ");

    for (ii=0;ii<numOfEnabledMembersGet;ii++)
    {
    /*verify new SW member is set in HW */
     if ((oldMemberArray[ii].port       !=  hwMembersArray[ii].port) ||
         (oldMemberArray[ii].hwDevice   !=  hwMembersArray[ii].hwDevice)||
         (oldMemberArray[ii].port       !=  enabledMembersArray2[ii].port) ||
         (oldMemberArray[ii].hwDevice   !=  enabledMembersArray2[ii].hwDevice))
        {
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, GT_FAIL, "ERROR : SW and HW doesnt match ");
        }
    }

    st = cpssDxChTrunkCascadeTrunkPortsGet(prvTgfDevNum,trunkIdArry[2],&cascadePortsGet);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChTrunkCascadeTrunkPortsGet ");

    if (!(CPSS_PORTS_BMP_ARE_EQUAL_MAC(&cascadePorts,&cascadePortsGet)))
       UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, GT_FAIL, "ERROR : SW and HW doesnt match ");


    /*restore */
    st = cpssDxChTrunkMembersSet(prvTgfDevNum, trunkIdArry[0],0,NULL, 0,NULL);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChTrunkMembersSet ");
    st = cpssDxChTrunkMembersSet(prvTgfDevNum, trunkIdArry[1],0,NULL, 0,NULL);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChTrunkMembersSet ");
    st = cpssDxChTrunkCascadeTrunkPortsSet(prvTgfDevNum, trunkIdArry[2],NULL);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChTrunkCascadeTrunkPortsSet ");

    PRV_UTF_LOG0_MAC("\n flush FDB ");
    /* flush FDB include static entries */
    prvTgfBrgFdbFlush(GT_TRUE);
    if(recreationRequired)
        cpssOsFree(recreationRequired);

}



UTF_TEST_CASE_MAC(tgfHighAvailabilityTrunkCascadeTrunk)
{
    /*
        based on tgfTrunkCascadeTrunk test
      1. init cascade ports
      2. Set high availablity basic configuration:
          - reset SW
          - set system recovery status to CPSS_SYSTEM_RECOVERY_INIT_STATE_E
          - reInitSysem with same configurations
          - set system recovery status to CPSS_SYSTEM_RECOVERY_HW_CATCH_UP_STATE_E
          - replay ports for port manager DB
          - replay same init as before the sw reset
          - set system recovery status to CPSS_SYSTEM_RECOVERY_COMPLETION_STATE_E
      3. send traffic
      4. restore configuration

  */

    GT_BOOL                         portMng;
    GT_STATUS                       st = 0 ;
    RECREATION_INFO                 *recreationRequired=NULL;
    PRV_TGF_TRUNK_LBH_GLOBAL_MODE_ENT hashModeGet;

    PRV_TGF_SUPPORTED_DEV_DECLARE_MAC((UTF_ALDRIN2_E|UTF_FALCON_E | UTF_AC5P_E | UTF_AC5X_E | UTF_HARRIER_E))
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, (~(UTF_ALDRIN2_E|UTF_FALCON_E | UTF_AC5P_E | UTF_AC5X_E | UTF_HARRIER_E )))

    st = cpssDxChPortManagerEnableGet(prvTgfDevNum,&portMng);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "ERROR : cpssPxPortManagerEnableGet FAILED:st-%d",st);
    if (portMng == GT_FALSE)
    {
        PRV_UTF_LOG0_MAC("tgfHighAvailabilitySimpleTest only support port manager ");
        SKIP_TEST_MAC
    }

    /* Get the general hashing mode of trunk hash generation as CRC Hash based on the packet's data*/
    st =  prvTgfTrunkHashGlobalModeGet(prvTgfDevNum,&hashModeGet);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "prvTgfTrunkHashGlobalModeGet: %d", prvTgfDevNum);

    /* Set configuration*/
    tgfTrunkCascadeTrunkTestInit(GT_FALSE);

    recreationRequired = cpssOsMalloc(sizeof(RECREATION_INFO)*RECREATION_MAX_SIZE);
    if (recreationRequired == NULL)
    {
        st = GT_OUT_OF_CPU_MEM;
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, " error at recreationRequired cpssOsMalloc ");
    }
    cpssOsMemSet(recreationRequired, 0, sizeof(RECREATION_INFO)*RECREATION_MAX_SIZE);
    st = prvTgfHighAvailabilityInitPhase(recreationRequired);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "ERROR : prvTgfHighAvailabilityInitPhase FAILED:st-%d devNum %d",st,prvTgfDevNum);

    st = appDemoHaEmulateSwCrash();
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "ERROR : appDemoHaEmulateSwCrash FAILED:st-%d devNum %d",st,prvTgfDevNum);

    st = prvTgHighAvailabilityStatusSet(1,GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "ERROR : prvTgHighAvailabilityStatusSet :st-%d ",st);

    PRV_UTF_LOG0_MAC("\nInit with port configuration \n");

    st = cpssReInitSystem();
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "ERROR : cpssReInitSystem FAILED:st-%d ", st);
    prvUtfSetAfterSystemResetState();

    st = prvTgHighAvailabilityStatusSet(2,GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "ERROR : prvTgHighAvailabilityStatusSet :st-%d ",st);

    st = prvTgfHighAvailabilityReplayAllPort(recreationRequired);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "ERROR : tfgHighAvailabilityReplayAllPort :st-%d ", st);

    /* Set configuration as replay*/
    tgfTrunkCascadeTrunkTestInit(GT_FALSE);

    st = prvTgHighAvailabilityStatusSet(3,GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "ERROR : prvTgHighAvailabilityStatusSet :st-%d ",st);

    PRV_UTF_LOG0_MAC("\nStart event handlers.\n");
    st = appDemoEventRequestDrvnModeInit();
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "ERROR : appDemoEventRequestDrvnModeInit :st-%d ",st);

    st = cpssDxChNetIfRestore(prvTgfDevNum);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "ERROR : cpssDxChNetIfRestore :st-%d ",st);
    /* Generate traffic*/
    tgfTrunkCascadeTrunkTest();

    /* Restore configuration*/
    tgfTrunkCascadeTrunkTestRestore();

    /* Get the general hashing mode of trunk hash generation as CRC Hash based on the packet's data*/
    st =  prvTgfTrunkHashGlobalModeSet(hashModeGet);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "prvTgfTrunkHashGlobalModeSet");

    PRV_UTF_LOG0_MAC("\n flush FDB ");
    /* flush FDB include static entries */
    prvTgfBrgFdbFlush(GT_TRUE);
    if(recreationRequired)
        cpssOsFree(recreationRequired);
}

UTF_TEST_CASE_MAC(tgfHighAvailabilityTrunkHighLowMixApisTest)
{
    /*
      1. Set high availablity basic configuration:
      1.1 set 2 trunk groups via high level apis (trunk id 1,4) and 1 via low level (trunk id 3)
      1.2 reset SW
      1.3 set system recovery status to CPSS_SYSTEM_RECOVERY_INIT_STATE_E
      1.4 reInitSysem with same configurations
      1.5 set system recovery status to CPSS_SYSTEM_RECOVERY_HW_CATCH_UP_STATE_E
      1.6 replay ports for port manager DB
      1.7 replay only 2 groups (trunk id 1,3)
      1.8 set system recovery status to CPSS_SYSTEM_RECOVERY_COMPLETION_STATE_E

      2 verify all trunk trunk 3 not deleted while trunk 4 data did
  */

    GT_BOOL                 portMng,isTrunkMember;
    GT_STATUS               st = GT_OK;

    GT_TRUNK_ID             trunkIdArry[3],trunkIdGet;
    GT_U32                  numOfEnabledMembers[2],numOfDisabledMembers[2];
    GT_U32                  ii,port,portFoundCounter[3];
    CPSS_TRUNK_MEMBER_STC   enabledMembersArray[CPSS_TRUNK_MAX_NUM_OF_MEMBERS_CNS] = {{47,16},{46,16},{45,16},{111,16}};
    CPSS_TRUNK_MEMBER_STC   disabledMembersArray[CPSS_TRUNK_MAX_NUM_OF_MEMBERS_CNS] ={{4,16},{5,16}};
    CPSS_TRUNK_MEMBER_STC   enabledMembersArray2[CPSS_TRUNK_MAX_NUM_OF_MEMBERS_CNS] = {{10,16},{11,16},{80,16}};

    CPSS_PORTS_BMP_STC      tmpDesignatedPort = { { 0, 0 } } ,nonPort = { { 0, 0 } };
    CPSS_PORTS_BMP_STC      designatedPortsBmpGet ,nonTrunkPortsBmpGet;
    RECREATION_INFO         *recreationRequired=NULL;
    GT_HW_DEV_NUM           hwDevice = PRV_CPSS_HW_DEV_NUM_MAC(prvTgfDevNum);

    PRV_TGF_SUPPORTED_DEV_DECLARE_MAC((UTF_ALDRIN2_E|UTF_FALCON_E | UTF_AC5P_E | UTF_AC5X_E | UTF_HARRIER_E ))
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, (~(UTF_ALDRIN2_E|UTF_FALCON_E | UTF_AC5P_E | UTF_AC5X_E | UTF_HARRIER_E )))

    st = cpssDxChPortManagerEnableGet(prvTgfDevNum,&portMng);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "ERROR : cpssPxPortManagerEnableGet FAILED:st-%d",st);
    if (portMng == GT_FALSE)
    {
        PRV_UTF_LOG0_MAC("tgfHighAvailabilitySimpleTest only support port manager ");
        SKIP_TEST_MAC
    }

    /* Set configuration */
    trunkIdArry[0]=1;
    trunkIdArry[1]=4;
    trunkIdArry[2]=3;
    numOfEnabledMembers[0]  = 4;
    numOfDisabledMembers[0] = 2;

    /*update hwDevice number */
    for (ii=0;ii<numOfEnabledMembers[0];ii++)
    {
         enabledMembersArray[ii].hwDevice=hwDevice;
    }
        for (ii=0;ii<numOfDisabledMembers[0];ii++)
    {
         disabledMembersArray[ii].hwDevice=hwDevice;
    }

    st = cpssDxChTrunkMembersSet(prvTgfDevNum, trunkIdArry[0], numOfEnabledMembers[0], enabledMembersArray, numOfDisabledMembers[0], disabledMembersArray);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "ERROR : cpssDxChTrunkMembersSet :st-%d ", st);
    numOfEnabledMembers[1]  = 3;
    numOfDisabledMembers[1] = 0;

    /*update hwDevice number */
    for (ii=0;ii<numOfEnabledMembers[1];ii++)
    {
         enabledMembersArray2[ii].hwDevice=hwDevice;
    }

    st = cpssDxChTrunkMembersSet(prvTgfDevNum, trunkIdArry[1], numOfEnabledMembers[1], enabledMembersArray2,numOfDisabledMembers[1], disabledMembersArray);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "ERROR : cpssDxChTrunkMembersSet :st-%d ", st);

    /*low level api trunk set */
    cpssDxChTrunkPortTrunkIdSet(prvTgfDevNum,1,GT_TRUE,trunkIdArry[2]);
    cpssDxChTrunkPortTrunkIdSet(prvTgfDevNum,2,GT_TRUE,trunkIdArry[2]);
    cpssDxChTrunkPortTrunkIdSet(prvTgfDevNum,3,GT_TRUE,trunkIdArry[2]);

    /*negative logic */
    nonPort.ports[0] = ~(0xEE);
    nonPort.ports[1] = 0xFFFFFFFF;
    nonPort.ports[2] = 0xFFFFFFFF;
    nonPort.ports[3] = 0xFFFFFFFF;
    st = cpssDxChTrunkNonTrunkPortsEntrySet(prvTgfDevNum,trunkIdArry[2],&nonPort);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "ERROR : cpssDxChTrunkNonTrunkPortsEntrySet :st-%d ", st);

    tmpDesignatedPort.ports[0] = 0xFFFFFFf3;
    tmpDesignatedPort.ports[1] = 0xFFFFFFf5;
    tmpDesignatedPort.ports[2] = 0xFFFFFFf9;

    for (ii=0;ii<PRV_CPSS_DEV_TRUNK_INFO_MAC(prvTgfDevNum)->numOfDesignatedTrunkEntriesHw;ii++)
    {
        /*get designated Ports from HW */
        st = cpssDxChTrunkDesignatedPortsEntryGet(prvTgfDevNum,ii,&designatedPortsBmpGet);
        if(st != GT_OK)
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "ERROR : cpssDxChTrunkDesignatedPortsEntryGet :st-%d ", st);
        }
        designatedPortsBmpGet.ports[0] &= tmpDesignatedPort.ports[ii % 3];
        st = cpssDxChTrunkDesignatedPortsEntrySet(prvTgfDevNum, ii, &designatedPortsBmpGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "ERROR : cpssDxChTrunkDesignatedPortsEntrySet :st-%d ", st);
    }

    recreationRequired = cpssOsMalloc(sizeof(RECREATION_INFO)*RECREATION_MAX_SIZE);
    if (recreationRequired == NULL)
    {
        st = GT_OUT_OF_CPU_MEM;
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, " error at recreationRequired cpssOsMalloc ");
    }
    cpssOsMemSet(recreationRequired, 0, sizeof(RECREATION_INFO)*RECREATION_MAX_SIZE);
    st = prvTgfHighAvailabilityInitPhase(recreationRequired);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "ERROR : prvTgfHighAvailabilityInitPhase FAILED:st-%d devNum %d",st,prvTgfDevNum);

    st = appDemoHaEmulateSwCrash();
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "ERROR : appDemoHaEmulateSwCrash FAILED:st-%d devNum %d", st, prvTgfDevNum);

    st = prvTgHighAvailabilityStatusSet(1,GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "ERROR : prvTgHighAvailabilityStatusSet :st-%d ", st);

    PRV_UTF_LOG0_MAC("\nInit with port configuration \n");

    st = cpssReInitSystem();
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "ERROR : cpssReInitSystem FAILED:st-%d ", st);
    prvUtfSetAfterSystemResetState();

    st = prvTgHighAvailabilityStatusSet(2,GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "ERROR : prvTgHighAvailabilityStatusSet :st-%d ", st);

/**************************************Replay************************************************************************/

    st = prvTgfHighAvailabilityReplayAllPort(recreationRequired);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "ERROR : tfgHighAvailabilityReplayAllPort :st-%d ", st);

    st = cpssDxChTrunkMembersSet(prvTgfDevNum, trunkIdArry[0], numOfEnabledMembers[0],
                                 enabledMembersArray,numOfDisabledMembers[0], disabledMembersArray);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "ERROR : cpssDxChTrunkMembersSet :st-%d ", st);

    /*skip trunkIdArry[2] in replay - we want it to be deleted in complition state*/

    /*low level api trunk set */
    cpssDxChTrunkPortTrunkIdSet(prvTgfDevNum,1,GT_TRUE,trunkIdArry[2]);
    cpssDxChTrunkPortTrunkIdSet(prvTgfDevNum,2,GT_TRUE,trunkIdArry[2]);
    cpssDxChTrunkPortTrunkIdSet(prvTgfDevNum,3,GT_TRUE,trunkIdArry[2]);

    cpssDxChTrunkNonTrunkPortsEntrySet(prvTgfDevNum,trunkIdArry[2],&nonPort);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "ERROR : cpssDxChTrunkNonTrunkPortsEntrySet :st-%d ", st);

    for (ii=0;ii<PRV_CPSS_DEV_TRUNK_INFO_MAC(prvTgfDevNum)->numOfDesignatedTrunkEntriesHw;ii++)
    {
        /*get designated Ports from HW */
        st = cpssDxChTrunkDesignatedPortsEntryGet(prvTgfDevNum,ii,&designatedPortsBmpGet);
        if(st != GT_OK)
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "ERROR : cpssDxChTrunkDesignatedPortsEntryGet :st-%d ", st);
        }
        designatedPortsBmpGet.ports[0] &= tmpDesignatedPort.ports[ii % 3];
        st = cpssDxChTrunkDesignatedPortsEntrySet(prvTgfDevNum, ii, &designatedPortsBmpGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "ERROR : cpssDxChTrunkDesignatedPortsEntrySet :st-%d ", st);
    }
/*******************************end*of*Replay************************************************************************/

    st = prvTgHighAvailabilityStatusSet(3,GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "ERROR : prvTgHighAvailabilityStatusSet :st-%d ",st);

    PRV_UTF_LOG0_MAC("\nStart event handlers.\n");
    st = appDemoEventRequestDrvnModeInit();
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "ERROR : appDemoEventRequestDrvnModeInit :st-%d ",st);

    st = cpssDxChNetIfRestore(prvTgfDevNum);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "ERROR : cpssDxChNetIfRestore :st-%d ",st);
    portFoundCounter[0]=0;
    portFoundCounter[1]=0;
    portFoundCounter[2]=0;
    /*verify trunk group are set as expected */
    for (port=0;port<PRV_CPSS_DXCH_MAX_PHY_PORT_NUMBER_MAC(prvTgfDevNum);port++)
    {
        st = cpssDxChTrunkPortTrunkIdGet(prvTgfDevNum, port, &isTrunkMember, &trunkIdGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "ERROR : cpssDxChTrunkPortTrunkIdGet :st-%d ", st);

        if (isTrunkMember == GT_TRUE)
        {
            if(trunkIdGet == trunkIdArry[0])
                portFoundCounter[0]++;
            else
            {
                if(trunkIdGet == trunkIdArry[1])
                    portFoundCounter[1]++;
                else
                {
                    if(trunkIdGet == trunkIdArry[2])
                        portFoundCounter[2]++;
                    else
                        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, GT_FAIL, "port %d match trunk %d that doesn't exist ",port,trunkIdGet);
                }
            }
        }
    }
    if (portFoundCounter[0]!=6 || portFoundCounter[1]!=0 || portFoundCounter[2]!=3)
    {
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, GT_FAIL, "ports number not as expected  portFoundCounter[0] %d !=6 || portFoundCounter[1] %d !=0 || portFoundCounter[2] %d !=3 ",
                                     portFoundCounter[0],portFoundCounter[1],portFoundCounter[2]);
    }

    /*get HW nonTrunk port members*/
    st = cpssDxChTrunkNonTrunkPortsEntryGet(prvTgfDevNum,trunkIdArry[2],&nonTrunkPortsBmpGet);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "ERROR : cpssDxChTrunkNonTrunkPortsEntryGet :st-%d ",st);

    if (!(CPSS_PORTS_BMP_ARE_EQUAL_MAC(&nonPort,&nonTrunkPortsBmpGet)))
    {
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, GT_FAIL, "ERROR : nonPort HW not as expected  ");
    }

    /*restore */
    st = cpssDxChTrunkMembersSet(prvTgfDevNum, trunkIdArry[0],0,NULL, 0,NULL);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChTrunkMembersSet ");
    st = cpssDxChTrunkCascadeTrunkPortsSet(prvTgfDevNum, trunkIdArry[2], NULL);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChTrunkMembersSet ");

    PRV_UTF_LOG0_MAC("\n flush FDB ");
    /* flush FDB include static entries */
    prvTgfBrgFdbFlush(GT_TRUE);
    if(recreationRequired)
        cpssOsFree(recreationRequired);
}



UTF_TEST_CASE_MAC(tgfHighAvailabilityHaParallelModeBasicUt)
{
    /*
      1. check basic UT test of ha parallel moda flow
    */

    GT_BOOL                             portMng;
    GT_STATUS                           st = 0;
    CPSS_SYSTEM_RECOVERY_STATE_ENT      state;
    GT_U32                              numOfManagers, ii;
    CPSS_SYSTEM_RECOVERY_MANAGER_ENT    managerListArray[CPSS_SYSTEM_RECOVERY_LAST_MANAGER_E];
    RECREATION_INFO                     *recreationRequired=NULL;

    PRV_TGF_SUPPORTED_DEV_DECLARE_MAC((UTF_ALDRIN2_E|UTF_FALCON_E | UTF_AC5P_E | UTF_AC5X_E | UTF_HARRIER_E))
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, (~(UTF_ALDRIN2_E|UTF_FALCON_E | UTF_AC5P_E | UTF_AC5X_E | UTF_HARRIER_E )))

    st = cpssDxChPortManagerEnableGet(prvTgfDevNum, &portMng);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "ERROR : cpssPxPortManagerEnableGet FAILED:st-%d", st);
    if (portMng == GT_FALSE)
    {
        PRV_UTF_LOG0_MAC("tgfHighAvailabilitySimpleTest only support port manager ");
        SKIP_TEST_MAC
    }

    recreationRequired = cpssOsMalloc(sizeof(RECREATION_INFO)*RECREATION_MAX_SIZE);
    if (recreationRequired == NULL)
    {
        st = GT_OUT_OF_CPU_MEM;
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, " error at recreationRequired cpssOsMalloc ");
    }
    cpssOsMemSet(recreationRequired, 0, sizeof(RECREATION_INFO)*RECREATION_MAX_SIZE);
    st = prvTgfHighAvailabilityInitPhase(recreationRequired);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "ERROR : prvTgfHighAvailabilityInitPhase FAILED:st-%d devNum %d",st,prvTgfDevNum);

    st = appDemoHaEmulateSwCrash();
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "ERROR : appDemoHaEmulateSwCrash FAILED:st-%d devNum %d",st,prvTgfDevNum);

    st = prvTgHighAvailabilityStatusSet(1,GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "ERROR : prvTgHighAvailabilityStatusSet :st-%d ",st);

    PRV_UTF_LOG0_MAC("\nInit with port configuration \n");

    st = cpssReInitSystem();
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "ERROR : cpssReInitSystem FAILED:st-%d ", st);
    prvUtfSetAfterSystemResetState();
    st = prvTgHighAvailabilityStatusSet(2,GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "ERROR : prvTgHighAvailabilityStatusSet :st-%d ",st);

    st = prvTgfHighAvailabilityReplayAllPort(recreationRequired);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "ERROR : tfgHighAvailabilityReplayAllPort :st-%d ", st);

    /*
       1.1 set manager to catch up before system recovery done
       expected GT_BAD_STATE
    */
    state = CPSS_SYSTEM_RECOVERY_HW_CATCH_UP_STATE_E;
    numOfManagers = 1;
    managerListArray[0] = CPSS_SYSTEM_RECOVERY_PORT_MANAGER_E;
    st = cpssSystemRecoveryHaParallelModeSet(state, numOfManagers, managerListArray);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_STATE, st, "ERROR : cpssSystemRecoveryHaParallelModeSet FAILED: state-%d manager %d", state, managerListArray[0]);

    /*
       1.2 set manager to complition before system recovery done
       expected GT_BAD_STATE
    */
    state = CPSS_SYSTEM_RECOVERY_COMPLETION_STATE_E;
    st = cpssSystemRecoveryHaParallelModeSet(state, numOfManagers, managerListArray);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_STATE, st, "ERROR : cpssSystemRecoveryHaParallelModeSet FAILED: state-%d manager %d", state, managerListArray[0]);

    st = prvTgHighAvailabilityStatusSet(3, GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "ERROR : prvTgHighAvailabilityStatusSet :st-%d ", st);

    PRV_UTF_LOG0_MAC("\nStart event handlers.\n");
    st = appDemoEventRequestDrvnModeInit();
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "ERROR : appDemoEventRequestDrvnModeInit :st-%d ",st);
    st = cpssDxChNetIfRestore(prvTgfDevNum);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "ERROR : cpssDxChNetIfRestore :st-%d ",st);

    /*
       1.3 set manager catch up/complition after manager recovery done
       expected GT_BAD_PARAM
    */
    state = CPSS_SYSTEM_RECOVERY_COMPLETION_STATE_E;
    managerListArray[0] = CPSS_SYSTEM_RECOVERY_FDB_MANAGER_E;
    st = cpssSystemRecoveryHaParallelModeSet(state, numOfManagers, managerListArray);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "ERROR : cpssSystemRecoveryHaParallelModeSet FAILED: state-%d manager %d", state, managerListArray[0]);

    st = cpssSystemRecoveryHaParallelModeSet(state, numOfManagers, managerListArray);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PARAM, st, "ERROR : cpssSystemRecoveryHaParallelModeSet FAILED: state-%d manager %d", state, managerListArray[0]);

    state = CPSS_SYSTEM_RECOVERY_HW_CATCH_UP_STATE_E;
    st = cpssSystemRecoveryHaParallelModeSet(state, numOfManagers, managerListArray);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PARAM, st, "ERROR : cpssSystemRecoveryHaParallelModeSet FAILED: state-%d manager %d", state, managerListArray[0]);


    /*
       1.4 set manager CPSS_SYSTEM_RECOVERY_LAST_MANAGER_E
       expected GT_BAD_PARAM
    */
    state = CPSS_SYSTEM_RECOVERY_HW_CATCH_UP_STATE_E;
    managerListArray[0] = CPSS_SYSTEM_RECOVERY_LAST_MANAGER_E;
    st = cpssSystemRecoveryHaParallelModeSet(state, numOfManagers, managerListArray);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PARAM, st, "ERROR : appDemoHaEmulateSwCrash FAILED:st-%d devNum %d", st, prvTgfDevNum);

    /*
       1.5 set all managers to complition check db is clear at the end
       expected GT_OK
    */

    PRV_SYSTEM_RECOVERY_MANAGER_STATUS_MAC(CPSS_SYSTEM_RECOVERY_TRUNK_MANAGER_E).recoveryStarted = GT_TRUE;
    PRV_SYSTEM_RECOVERY_MANAGER_STATUS_MAC(CPSS_SYSTEM_RECOVERY_TRUNK_MANAGER_E).recoveryDone = GT_TRUE ;
    state = CPSS_SYSTEM_RECOVERY_COMPLETION_STATE_E;
    numOfManagers = 4;
    managerListArray[0] = CPSS_SYSTEM_RECOVERY_PORT_MANAGER_E;
    managerListArray[1] = CPSS_SYSTEM_RECOVERY_LPM_MANAGER_E;
    managerListArray[2] = CPSS_SYSTEM_RECOVERY_TCAM_MANAGER_E;
    managerListArray[3] = CPSS_SYSTEM_RECOVERY_EXACT_MATCH_MANAGER_E;
    st = cpssSystemRecoveryHaParallelModeSet(state, numOfManagers, managerListArray);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "ERROR : cpssSystemRecoveryHaParallelModeSet st %d state %d ", st, state);

    for (ii = 0; ii < CPSS_SYSTEM_RECOVERY_LAST_MANAGER_E; ii++)
    {
        if ((PRV_SYSTEM_RECOVERY_MANAGER_STATUS_MAC(ii).recoveryDone != GT_FALSE) || (PRV_SYSTEM_RECOVERY_MANAGER_STATUS_MAC(ii).recoveryStarted != GT_FALSE))
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, GT_FAIL, "ERROR : manager status DB is not cleared manager %d", ii);
    }

    PRV_UTF_LOG0_MAC("\n flush FDB ");
    /* flush FDB include static entries */
    prvTgfBrgFdbFlush(GT_TRUE);

    /*run test to verify system restore */
    tgfBasicTrafficSanity_extern();
    if(recreationRequired)
        cpssOsFree(recreationRequired);
}

UTF_TEST_CASE_MAC(tgfHighAvailabilityHaParallelModePortsTest)
{
    /*
      1. Set parallel high availablity basic configuration:
          - reset SW
          - set system recovery status to CPSS_SYSTEM_RECOVERY_INIT_STATE_E
          - reInitSysem with same configurations
          - set system recovery status to CPSS_SYSTEM_RECOVERY_HW_CATCH_UP_STATE_E
          - set system recovery status to CPSS_SYSTEM_RECOVERY_COMPLETION_STATE_E
            with parallel_ha mode set
          - set parallel ha status for port manager to CPSS_SYSTEM_RECOVERY_HW_CATCH_UP_STATE_E
          - replay ports for port manager DB
          - set parallel ha status for port manager to CPSS_SYSTEM_RECOVERY_COMPLETION_STATE_E

      2. run simple test to verify system restore
          - tgfBasicTrafficSanity_extern

    */

    GT_BOOL                             portMng;
    GT_STATUS                           st = 0;
    CPSS_SYSTEM_RECOVERY_STATE_ENT      state;
    GT_U32                              numOfManagers;
    CPSS_SYSTEM_RECOVERY_MANAGER_ENT    managerListArray[CPSS_SYSTEM_RECOVERY_LAST_MANAGER_E];
    RECREATION_INFO                     *recreationRequired=NULL;

    PRV_TGF_SUPPORTED_DEV_DECLARE_MAC((UTF_ALDRIN2_E|UTF_FALCON_E | UTF_AC5P_E | UTF_AC5X_E | UTF_HARRIER_E ))
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, (~(UTF_ALDRIN2_E|UTF_FALCON_E | UTF_AC5P_E | UTF_AC5X_E | UTF_HARRIER_E )))

    st = cpssDxChPortManagerEnableGet(prvTgfDevNum, &portMng);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "ERROR : cpssPxPortManagerEnableGet FAILED:st-%d", st);
    if (portMng == GT_FALSE)
    {
        PRV_UTF_LOG0_MAC("tgfHighAvailabilitySimpleTest only support port manager ");
        SKIP_TEST_MAC
    }

    recreationRequired = cpssOsMalloc(sizeof(RECREATION_INFO)*RECREATION_MAX_SIZE);
    if (recreationRequired == NULL)
    {
        st = GT_OUT_OF_CPU_MEM;
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, " error at recreationRequired cpssOsMalloc ");
    }
    cpssOsMemSet(recreationRequired, 0, sizeof(RECREATION_INFO)*RECREATION_MAX_SIZE);
    st = prvTgfHighAvailabilityInitPhase(recreationRequired);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "ERROR : prvTgfHighAvailabilityInitPhase FAILED:st-%d devNum %d",st,prvTgfDevNum);

    st = appDemoHaEmulateSwCrash();
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "ERROR : appDemoHaEmulateSwCrash FAILED:st-%d devNum %d",st,prvTgfDevNum);

    st = prvTgHighAvailabilityStatusSet(1,GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "ERROR : prvTgHighAvailabilityStatusSet :st-%d ",st);

    PRV_UTF_LOG0_MAC("\nInit with port configuration \n");

    st = cpssReInitSystem();
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "ERROR : cpssReInitSystem FAILED:st-%d ", st);
    prvUtfSetAfterSystemResetState();
    st = prvTgHighAvailabilityStatusSet(2,GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "ERROR : prvTgHighAvailabilityStatusSet :st-%d ",st);
    /* start complition stage with parallel HA flag*/
    st = prvTgHighAvailabilityStatusSet(3, GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "ERROR : prvTgHighAvailabilityStatusSet :st-%d ", st);

    PRV_UTF_LOG0_MAC("\nStart event handlers.\n");
    st = appDemoEventRequestDrvnModeInit();
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "ERROR : appDemoEventRequestDrvnModeInit :st-%d ",st);
    st = cpssDxChNetIfRestore(prvTgfDevNum);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "ERROR : cpssDxChNetIfRestore :st-%d ",st);

    /*replay port manager */
    state = CPSS_SYSTEM_RECOVERY_HW_CATCH_UP_STATE_E;
    numOfManagers=1;
    managerListArray[0] = CPSS_SYSTEM_RECOVERY_PORT_MANAGER_E;
    st = cpssSystemRecoveryHaParallelModeSet(state, numOfManagers, managerListArray);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "ERROR : cpssSystemRecoveryHaParallelModeSet FAILED: state-%d ", state);

    st = prvTgfHighAvailabilityReplayAllPort(recreationRequired);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "ERROR : tfgHighAvailabilityReplayAllPort :st-%d ", st);

    numOfManagers=2;
    state = CPSS_SYSTEM_RECOVERY_COMPLETION_STATE_E;
    managerListArray[1] = CPSS_SYSTEM_RECOVERY_LPM_MANAGER_E;
    st = cpssSystemRecoveryHaParallelModeSet(state, numOfManagers, managerListArray);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "ERROR : cpssSystemRecoveryHaParallelModeSet FAILED: state-%d", state);

    PRV_UTF_LOG0_MAC("\n flush FDB ");
    /* flush FDB include static entries */
    prvTgfBrgFdbFlush(GT_TRUE);

    /*run test to verify system restore */
    tgfBasicTrafficSanity_extern();
    if(recreationRequired)
        cpssOsFree(recreationRequired);
}

UTF_TEST_CASE_MAC(tgfHighAvailabilityHaParallelTrunkSimpleTest)
{
    /*
      1. Set high availablity basic configuration same logic of tgfHighAvailabilityTrunkSimpleTest:
      1.1 set 2 regular trunk groups and 1 cascade
      1.2 reset SW
      1.3 set system recovery status to CPSS_SYSTEM_RECOVERY_INIT_STATE_E
      1.4 reInitSysem with same configurations
      1.5 set system recovery status to CPSS_SYSTEM_RECOVERY_HW_CATCH_UP_STATE_E
      1.6 set system recovery status to CPSS_SYSTEM_RECOVERY_COMPLETION_STATE_E with parallel_ha mode set
      1.7 set parallel ha status for trunk manager to CPSS_SYSTEM_RECOVERY_HW_CATCH_UP_STATE_E
      1.8 replay trunk groups with diffrent setting
          group 1 with other port member,
          group 2 cascade with more port members in it
          group 50 diffrent members order
      1.9 set parallel ha status for trunk manager to CPSS_SYSTEM_RECOVERY_COMPLETION_STATE_E

      2 verify all trunk groups are fix
        - hw DB updated for groups 1,2
        - sw DB updated for group 50
      3 - set parallel ha status for port and lpm managers to CPSS_SYSTEM_RECOVERY_HW_CATCH_UP_STATE_E
        - replay ports for port manager DB
        - set parallel ha status for port lpm managers to CPSS_SYSTEM_RECOVERY_COMPLETION_STATE_E
  */

    GT_BOOL                 portMng;
    GT_STATUS               st = GT_OK;

    GT_TRUNK_ID             trunkIdArry[3];
    GT_U32                  numOfEnabledMembers[2],numOfEnabledMembersGet, ii;
    GT_U32                  numOfDisabledMembers[2];
    CPSS_TRUNK_MEMBER_STC   enabledMembersArray[CPSS_TRUNK_MAX_NUM_OF_MEMBERS_CNS] = {{47,16},{46,16},{45,16},{111,16}};
    CPSS_TRUNK_MEMBER_STC   disabledMembersArray[CPSS_TRUNK_MAX_NUM_OF_MEMBERS_CNS] ={{4,16},{5,16}};
    CPSS_TRUNK_MEMBER_STC   enabledMembersArray2[CPSS_TRUNK_MAX_NUM_OF_MEMBERS_CNS] = {{100,17},{101,17},{102,16},{103,16},{104,16}};
    CPSS_TRUNK_MEMBER_STC   oldMemberArray[CPSS_TRUNK_MAX_NUM_OF_MEMBERS_CNS] = {{100,17},{101,17},{102,16},{103,16},{104,16}};
    CPSS_TRUNK_MEMBER_STC   hwMembersArray[CPSS_TRUNK_MAX_NUM_OF_MEMBERS_CNS];
    CPSS_PORTS_BMP_STC      cascadePorts={{0,0}} ,cascadePortsGet={{0,0}};

    CPSS_SYSTEM_RECOVERY_STATE_ENT      state;
    GT_U32                              numOfManagers;
    CPSS_SYSTEM_RECOVERY_MANAGER_ENT    managerListArray[CPSS_SYSTEM_RECOVERY_LAST_MANAGER_E];
    RECREATION_INFO                     *recreationRequired=NULL;
    GT_HW_DEV_NUM                       hwDevice = PRV_CPSS_HW_DEV_NUM_MAC(prvTgfDevNum);

    PRV_TGF_SUPPORTED_DEV_DECLARE_MAC((UTF_ALDRIN2_E|UTF_FALCON_E | UTF_AC5P_E | UTF_AC5X_E | UTF_HARRIER_E ))
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, (~(UTF_ALDRIN2_E|UTF_FALCON_E | UTF_AC5P_E | UTF_AC5X_E | UTF_HARRIER_E )))

    st = cpssDxChPortManagerEnableGet(prvTgfDevNum,&portMng);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "ERROR : cpssPxPortManagerEnableGet FAILED:st-%d",st);
    if (portMng == GT_FALSE)
    {
        PRV_UTF_LOG0_MAC("tgfHighAvailabilitySimpleTest only support port manager ");
        SKIP_TEST_MAC
    }

    cpssOsMemCpy(oldMemberArray, enabledMembersArray2, sizeof(enabledMembersArray2));

    trunkIdArry[0] = 1;
    trunkIdArry[1]=50;
    trunkIdArry[2]=2;
    /* Set configuration */

    numOfEnabledMembers[0]  = 4;
    numOfDisabledMembers[0] = 2;

    /*update hwDevice number */
    for (ii=0;ii<numOfEnabledMembers[0];ii++)
    {
         enabledMembersArray[ii].hwDevice=hwDevice;
    }
    for (ii=0;ii<numOfDisabledMembers[0];ii++)
    {
         disabledMembersArray[ii].hwDevice=hwDevice;
    }
    st = cpssDxChTrunkMembersSet(prvTgfDevNum, trunkIdArry[0], numOfEnabledMembers[0], enabledMembersArray,numOfDisabledMembers[0], disabledMembersArray);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "ERROR : cpssDxChTrunkMembersSet :st-%d ", st);

    cascadePorts.ports[1] = 0xf00;
    st = cpssDxChTrunkCascadeTrunkPortsSet(prvTgfDevNum,trunkIdArry[2],&cascadePorts);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "ERROR : cpssDxChTrunkCascadeTrunkPortsSet :st-%d ", st);

    numOfEnabledMembers[1] = 5;
    numOfDisabledMembers[1] = 0;

    /*update hwDevice number */
    for (ii=2;ii<numOfEnabledMembers[1];ii++)
    {
         oldMemberArray[ii].hwDevice = enabledMembersArray2[ii].hwDevice = hwDevice ;
    }
    oldMemberArray[0].hwDevice = enabledMembersArray2[0].hwDevice = hwDevice+1; /*diffrent hwDevice*/
    oldMemberArray[1].hwDevice = enabledMembersArray2[1].hwDevice = hwDevice+1; /*diffrent hwDevice*/

    st = cpssDxChTrunkMembersSet(prvTgfDevNum, trunkIdArry[1], numOfEnabledMembers[1], enabledMembersArray2, numOfDisabledMembers[1], disabledMembersArray);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "ERROR : cpssDxChTrunkMembersSet :st-%d ", st);

    recreationRequired = cpssOsMalloc(sizeof(RECREATION_INFO)*RECREATION_MAX_SIZE);
    if (recreationRequired == NULL)
    {
        st = GT_OUT_OF_CPU_MEM;
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, " error at recreationRequired cpssOsMalloc ");
    }
    cpssOsMemSet(recreationRequired, 0, sizeof(RECREATION_INFO)*RECREATION_MAX_SIZE);
    st = prvTgfHighAvailabilityInitPhase(recreationRequired);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "ERROR : prvTgfHighAvailabilityInitPhase FAILED:st-%d devNum %d",st,prvTgfDevNum);

    st = appDemoHaEmulateSwCrash();
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "ERROR : appDemoHaEmulateSwCrash FAILED:st-%d devNum %d",st,prvTgfDevNum);

    st = prvTgHighAvailabilityStatusSet(1,GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "ERROR : prvTgHighAvailabilityStatusSet :st-%d ",st);

    PRV_UTF_LOG0_MAC("\nInit with port configuration \n");

    st = cpssReInitSystem();
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "ERROR : cpssReInitSystem FAILED:st-%d ", st);
    prvUtfSetAfterSystemResetState();

    st = prvTgHighAvailabilityStatusSet(2,GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "ERROR : prvTgHighAvailabilityStatusSet :st-%d ",st);

    st = prvTgHighAvailabilityStatusSet(3, GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "ERROR : prvTgHighAvailabilityStatusSet :st-%d ", st);

    PRV_UTF_LOG0_MAC("\nStart event handlers.\n");
    st = appDemoEventRequestDrvnModeInit();
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "ERROR : appDemoEventRequestDrvnModeInit :st-%d ",st);
    st = cpssDxChNetIfRestore(prvTgfDevNum);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "ERROR : cpssDxChNetIfRestore :st-%d ",st);

    state = CPSS_SYSTEM_RECOVERY_HW_CATCH_UP_STATE_E;
    numOfManagers=1;
    managerListArray[0] = CPSS_SYSTEM_RECOVERY_TRUNK_MANAGER_E;
    st = cpssSystemRecoveryHaParallelModeSet(state, numOfManagers, managerListArray);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "ERROR : cpssSystemRecoveryHaParallelModeSet FAILED: state-%d ", state);

    /*change trunk port members in replay -make hw and sw db diffrent*/
    enabledMembersArray[2].port = 20;
    enabledMembersArray[2].hwDevice = hwDevice;
    cascadePorts.ports[0] = 0xf00;

    /*change only the inpuy order*/
    enabledMembersArray2[2].port = oldMemberArray[4].port;
    enabledMembersArray2[2].hwDevice = oldMemberArray[4].hwDevice;
    enabledMembersArray2[4].port = oldMemberArray[2].port;
    enabledMembersArray2[4].hwDevice = oldMemberArray[2].hwDevice;


    /* Set configuration */
    st = cpssDxChTrunkMembersSet(prvTgfDevNum, trunkIdArry[1], numOfEnabledMembers[1], enabledMembersArray2, numOfDisabledMembers[1], disabledMembersArray);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "ERROR : cpssDxChTrunkMembersSet :st-%d ", st);

    st = cpssDxChTrunkMembersSet(prvTgfDevNum, trunkIdArry[0], numOfEnabledMembers[0], enabledMembersArray,numOfDisabledMembers[0], disabledMembersArray);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "ERROR : cpssDxChTrunkMembersSet :st-%d ", st);

    st = cpssDxChTrunkCascadeTrunkPortsSet(prvTgfDevNum, trunkIdArry[2], &cascadePorts);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "ERROR : cpssDxChTrunkCascadeTrunkPortsSet :st-%d ", st);

    state = CPSS_SYSTEM_RECOVERY_COMPLETION_STATE_E;
    st = cpssSystemRecoveryHaParallelModeSet(state, numOfManagers, managerListArray);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "ERROR : cpssSystemRecoveryHaParallelModeSet FAILED: state-%d", state);

    /*verify HW and SW match */
    numOfEnabledMembersGet = CPSS_TRUNK_MAX_NUM_OF_MEMBERS_CNS;
    st = cpssDxChTrunkTableEntryGet(prvTgfDevNum, trunkIdArry[0], &numOfEnabledMembersGet, hwMembersArray);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChTrunkTableEntryGet ");

    if (numOfEnabledMembersGet != numOfEnabledMembers[0])
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, GT_FAIL, "ERROR : numOfEnabledMembersGet != swNumOfEnableMembers ");

    for (ii=0;ii<numOfEnabledMembersGet;ii++)
    {
        /*verify new SW member is set in HW */
        if ((enabledMembersArray[ii].port !=  hwMembersArray[ii].port) ||
         (enabledMembersArray[ii].hwDevice !=  hwMembersArray[ii].hwDevice))
        {
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, GT_FAIL, "ERROR : SW and HW doesnt match ");
        }
    }

    /*verify HW and SW match and sw is set with old Hw config*/
    numOfEnabledMembersGet = CPSS_TRUNK_MAX_NUM_OF_MEMBERS_CNS;
    st = cpssDxChTrunkTableEntryGet(prvTgfDevNum, trunkIdArry[1], &numOfEnabledMembersGet, hwMembersArray);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChTrunkTableEntryGet ");

    st = cpssDxChTrunkDbEnabledMembersGet(prvTgfDevNum, trunkIdArry[1], &numOfEnabledMembers[1], enabledMembersArray2);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChTrunkDbEnabledMembersGet ");

    if (numOfEnabledMembersGet != numOfEnabledMembers[1]) UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, GT_FAIL, "ERROR : numOfEnabledMembersGet != swNumOfEnableMembers ");

    for (ii = 0; ii < numOfEnabledMembersGet; ii++)
    {
        /*verify new SW member is set in HW */
        if ((oldMemberArray[ii].port       !=  hwMembersArray[ii].port) ||
            (oldMemberArray[ii].hwDevice   !=  hwMembersArray[ii].hwDevice) ||
            (oldMemberArray[ii].port       !=  enabledMembersArray2[ii].port) ||
            (oldMemberArray[ii].hwDevice   !=  enabledMembersArray2[ii].hwDevice))
        {
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, GT_FAIL, "ERROR : SW and HW doesnt match ");
        }
    }

    st = cpssDxChTrunkCascadeTrunkPortsGet(prvTgfDevNum,trunkIdArry[2],&cascadePortsGet);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChTrunkCascadeTrunkPortsGet ");

    if (!(CPSS_PORTS_BMP_ARE_EQUAL_MAC(&cascadePorts,&cascadePortsGet)))
       UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, GT_FAIL, "ERROR : SW and HW doesnt match ");

    /*restore */
    st = cpssDxChTrunkMembersSet(prvTgfDevNum, trunkIdArry[0],0,NULL, 0,NULL);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChTrunkMembersSet ");

    st = cpssDxChTrunkMembersSet(prvTgfDevNum, trunkIdArry[1], 0, NULL, 0, NULL);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChTrunkMembersSet ");

    st = cpssDxChTrunkCascadeTrunkPortsSet(prvTgfDevNum, trunkIdArry[2],NULL);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChTrunkCascadeTrunkPortsSet ");

    /*restore LPM and PORT MANAGER for next tests*/
    state = CPSS_SYSTEM_RECOVERY_HW_CATCH_UP_STATE_E;
    numOfManagers=2;
    managerListArray[0] = CPSS_SYSTEM_RECOVERY_PORT_MANAGER_E;
    managerListArray[1] = CPSS_SYSTEM_RECOVERY_LPM_MANAGER_E;
    st = cpssSystemRecoveryHaParallelModeSet(state, numOfManagers, managerListArray);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "ERROR : cpssSystemRecoveryHaParallelModeSet FAILED: state-%d ", state);

    st = prvTgfHighAvailabilityReplayAllPort(recreationRequired);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "ERROR : tfgHighAvailabilityReplayAllPort :st-%d ", st);

    /*restore LPM and PORT MANAGER for next tests*/
    state = CPSS_SYSTEM_RECOVERY_COMPLETION_STATE_E;
    st = cpssSystemRecoveryHaParallelModeSet(state, numOfManagers, managerListArray);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "ERROR : cpssSystemRecoveryHaParallelModeSet FAILED: state-%d ", state);

    PRV_UTF_LOG0_MAC("\n flush FDB ");
    /* flush FDB include static entries */
    prvTgfBrgFdbFlush(GT_TRUE);
    if(recreationRequired)
        cpssOsFree(recreationRequired);
}

UTF_TEST_CASE_MAC(tgfHighAvailabilityHaParallelLpm4_6_Uc_McTest)
{
  /*
      1. Set LPM configuration for ipv4/6 uc and mc
      2. Check traffic
      3. Do parallel HA:
          - reset SW
          - set system recovery status to CPSS_SYSTEM_RECOVERY_INIT_STATE_E
          - reInitSysem with same configurations
          - set system recovery status to CPSS_SYSTEM_RECOVERY_HW_CATCH_UP_STATE_E
          - replay ports for port manager DB
          - set system recovery status to CPSS_SYSTEM_RECOVERY_COMPLETION_STATE_E with parallel_ha mode set
          - set parallel ha status for port and lpm manager to CPSS_SYSTEM_RECOVERY_HW_CATCH_UP_STATE_E
          - replay LPM configuration
          - set parallel ha status for port lpm managers to CPSS_SYSTEM_RECOVERY_COMPLETION_STATE_E
      4. Check LPM configuration after parallel HA, compare with lpm configuration before parallel HA.
      5. Check traffic
  */
    tgfHighAvailabilityParallelLpm4_6_Uc_Mc(1);
#if 0 /*skip ipv6 uc and ipv4 mc to reduce test time*/
    tgfHighAvailabilityParallelLpm4_6_Uc_Mc(2);
    tgfHighAvailabilityParallelLpm4_6_Uc_Mc(3);
#endif
    tgfHighAvailabilityParallelLpm4_6_Uc_Mc(4);

}

UTF_TEST_CASE_MAC(tgfHighAvailabilityHaParallelExactMatchManager)
{
    /*
        1. Set Exact Match Non Default Action configuration
        2. Set TTI UDB configuration
        3. Set PCL UDB configuration
        4. Generate traffic - expect Exact Match hit - traffic drop
        5. Do parallel HA:
          - reset SW
          - set system recovery status to CPSS_SYSTEM_RECOVERY_INIT_STATE_E
          - reInitSysem with same configurations
          - set system recovery status to CPSS_SYSTEM_RECOVERY_HW_CATCH_UP_STATE_E
          - replay ports for port manager DB
          - replay TTI UDB configuration
          - replay PCL UDB configuration
          - set system recovery status to CPSS_SYSTEM_RECOVERY_COMPLETION_STATE_E with parallel_ha mode set
          - set parallel ha status for Exact match manager to CPSS_SYSTEM_RECOVERY_HW_CATCH_UP_STATE_E
          - replay Exact Match Manager configuration
          - set parallel ha status for Exact match manager to CPSS_SYSTEM_RECOVERY_COMPLETION_STATE_E
        6. Generate traffic - expect Exact Match hit - traffic drop
        7. Invalidate Exact Match Entry
        8. Generate traffic - expect no hit in TCAM or Exact Match - traffic forwarded
        9. Set Exact Match Non Default Action configuration
        10.Generate traffic - expect Exact Match hit - traffic drop
        11.Restore Exact Match configuration
  */

    GT_U32                          exactMatchManagerId = 23;
    GT_BOOL                         portMng;
    GT_STATUS                       st = 0 ;


    CPSS_SYSTEM_RECOVERY_STATE_ENT      state;
    GT_U32                              numOfManagers;
    CPSS_SYSTEM_RECOVERY_MANAGER_ENT    managerListArray[CPSS_SYSTEM_RECOVERY_LAST_MANAGER_E];
    RECREATION_INFO                     *recreationRequired=NULL;


    PRV_TGF_SUPPORTED_DEV_DECLARE_MAC(UTF_FALCON_E | UTF_AC5P_E | UTF_AC5X_E | UTF_HARRIER_E)
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC( prvTgfDevNum, (~(UTF_FALCON_E | UTF_AC5P_E | UTF_AC5X_E | UTF_HARRIER_E )) )
    st = cpssDxChPortManagerEnableGet(prvTgfDevNum,&portMng);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "ERROR : cpssPxPortManagerEnableGet FAILED:st-%d",st);
    if (portMng == GT_FALSE)
    {
        PRV_UTF_LOG0_MAC("tgfHighAvailabilitySimpleTest only support port manager ");
        SKIP_TEST_MAC
    }

        if((GT_FALSE == prvTgfBrgTtiCheck())||
       (GT_FALSE == prvTgfExactMatchCheck()))
    {
            SKIP_TEST_MAC
    }

    /* Create manager with device */
    prvTgfExactMatchManagerTtiPclFullPathManagerCreate(exactMatchManagerId,CPSS_PACKET_CMD_TRAP_TO_CPU_E);

    /* Set TTI Generic configuration not related to Exact Match */
    prvTgfExactMatchTtiPclFullPath_TTI_GenericConfigSet(GT_TRUE);

    /* Set PCL Generic configuration not related to Exact Match */
    prvTgfExactMatchTtiPclFullPath_PCL_GenericConfigSet(0);

    /* Set Exact Match Manager Expanded Action PCL configuration */
    prvTgfExactMatchManagerTtiPclFullPathConfigSet(exactMatchManagerId,CPSS_PACKET_CMD_TRAP_TO_CPU_E);

    /* Generate traffic - check no traffic, no match in TCAM,
       get Exact Match Expanded Action */
    prvTgfExactMatchTtiPclFullPathTrafficGenerate(GT_TRUE,GT_FALSE);

    recreationRequired = cpssOsMalloc(sizeof(RECREATION_INFO)*RECREATION_MAX_SIZE);
    if (recreationRequired == NULL)
    {
        st = GT_OUT_OF_CPU_MEM;
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, " error at recreationRequired cpssOsMalloc ");
    }
    cpssOsMemSet(recreationRequired, 0, sizeof(RECREATION_INFO)*RECREATION_MAX_SIZE);
    st = prvTgfHighAvailabilityInitPhase(recreationRequired);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "ERROR : prvTgfHighAvailabilityInitPhase FAILED:st-%d devNum %d",st,prvTgfDevNum);

    st = appDemoHaEmulateSwCrash();
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "ERROR : appDemoHaEmulateSwCrash FAILED:st-%d devNum %d",st,prvTgfDevNum);

    st = prvTgHighAvailabilityStatusSet(1,GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "ERROR : prvTgHighAvailabilityStatusSet :st-%d ",st);

    PRV_UTF_LOG0_MAC("\nInit with port configuration \n");

    st = cpssReInitSystem();
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "ERROR : cpssReInitSystem FAILED:st-%d ", st);
    prvUtfSetAfterSystemResetState();

    st = prvTgHighAvailabilityStatusSet(2,GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "ERROR : prvTgHighAvailabilityStatusSet :st-%d ",st);

    st = prvTgfHighAvailabilityReplayAllPort(recreationRequired);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "ERROR : tfgHighAvailabilityReplayAllPort :st-%d ", st);

    /* Set TTI Generic configuration not related to Exact Match */
    prvTgfExactMatchTtiPclFullPath_TTI_GenericConfigSet(GT_TRUE);

    /* Set PCL Generic configuration not related to Exact Match */
    prvTgfExactMatchTtiPclFullPath_PCL_GenericConfigSet(0);

    st = prvTgHighAvailabilityStatusSet(3,GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "ERROR : prvTgHighAvailabilityStatusSet :st-%d ",st);

    st = appDemoEventRequestDrvnModeInit();
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "ERROR : appDemoEventRequestDrvnModeInit :st-%d ",st);

    st = cpssDxChNetIfRestore(prvTgfDevNum);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "ERROR : cpssDxChNetIfRestore :st-%d ",st);
    state = CPSS_SYSTEM_RECOVERY_HW_CATCH_UP_STATE_E;
    numOfManagers=1;
    managerListArray[0] = CPSS_SYSTEM_RECOVERY_EXACT_MATCH_MANAGER_E;
    st = cpssSystemRecoveryHaParallelModeSet(state, numOfManagers, managerListArray);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "ERROR : cpssSystemRecoveryHaParallelModeSet FAILED: state-%d ", state);

    /* Create manager with device */
    prvTgfExactMatchManagerTtiPclFullPathManagerCreate(exactMatchManagerId, CPSS_PACKET_CMD_TRAP_TO_CPU_E);

    /* Set Exact Match Manager Expanded Action PCL configuration */
    prvTgfExactMatchManagerTtiPclFullPathConfigSet(exactMatchManagerId, CPSS_PACKET_CMD_TRAP_TO_CPU_E);

    state = CPSS_SYSTEM_RECOVERY_COMPLETION_STATE_E;
    numOfManagers=2;
    managerListArray[0] = CPSS_SYSTEM_RECOVERY_EXACT_MATCH_MANAGER_E;
    managerListArray[1] = CPSS_SYSTEM_RECOVERY_LPM_MANAGER_E;
    st = cpssSystemRecoveryHaParallelModeSet(state, numOfManagers, managerListArray);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "ERROR : cpssSystemRecoveryHaParallelModeSet FAILED: state-%d", state);

    /* Generate traffic - check no traffic, no match in TCAM,
       get Exact Match Expanded Action */
    prvTgfExactMatchTtiPclFullPathTrafficGenerate(GT_TRUE,GT_FALSE);

    /*check for traffic after remove rule*/
        /* invalidate Exact Match Entry in Manager */
    prvTgfExactMatchManagerTtiPclFullPathInvalidateEmEntry(exactMatchManagerId);

    /* Generate traffic - no match in TCAM, no match in Exact Match
       traffic forwarded */
    prvTgfExactMatchTtiPclFullPathTrafficGenerate(GT_FALSE, GT_TRUE);

    /* Set Exact Match Manager Expanded Action PCL configuration */
    prvTgfExactMatchManagerTtiPclFullPathConfigSet(exactMatchManagerId, CPSS_PACKET_CMD_TRAP_TO_CPU_E);

    /* Generate traffic - check no traffic, no match in TCAM,
       get Exact Match Expanded Action */
    prvTgfExactMatchTtiPclFullPathTrafficGenerate(GT_TRUE,GT_FALSE);

    /* Restore configuration */
    prvTgfExactMatchManagerTtiPclFullPathHaRestoreBasicConfigSingleEntry(exactMatchManagerId);
    if(recreationRequired)
        cpssOsFree(recreationRequired);
}

/*
 * Configuration of tgfsystemRecovery suit
 */
UTF_SUIT_BEGIN_TESTS_MAC(tgfHighAvailability)

    UTF_SUIT_DECLARE_TEST_MAC(tgfHighAvailabilitySimpleTest)
    UTF_SUIT_DECLARE_TEST_MAC(tgfHighAvailabilityLpmSimpleTest)
    UTF_SUIT_DECLARE_TEST_MAC(tgfHighAvailabilityExactMatchManagerSimpleTest)
    UTF_SUIT_DECLARE_TEST_MAC(tgfHighAvailabilityTrunkSimpleTest)
    UTF_SUIT_DECLARE_TEST_MAC(tgfHighAvailabilityTrunkCascadeTrunk)
    UTF_SUIT_DECLARE_TEST_MAC(tgfHighAvailabilityTrunkHighLowMixApisTest)
    UTF_SUIT_DECLARE_TEST_MAC(tgfHighAvailabilityHaParallelModeBasicUt)
    UTF_SUIT_DECLARE_TEST_MAC(tgfHighAvailabilityHaParallelModePortsTest)
    UTF_SUIT_DECLARE_TEST_MAC(tgfHighAvailabilityHaParallelTrunkSimpleTest)
    UTF_SUIT_DECLARE_TEST_MAC(tgfHighAvailabilityHaParallelLpm4_6_Uc_McTest)
    UTF_SUIT_DECLARE_TEST_MAC(tgfHighAvailabilityHaParallelExactMatchManager)

UTF_SUIT_END_TESTS_MAC(tgfHighAvailability)

