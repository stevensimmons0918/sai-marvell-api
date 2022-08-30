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
* @file tgfPxHighAvailabilityUT.c
*
* @brief Enhanced UTs for CPSS High Availability
* @version   1
********************************************************************************
*/


/* HA UT not supported in CAP - commenting the entire file for CAP - since there are many references to appDemo
   Note: Please use wrappers for appDemo functions - Refer wrappers in cpssEnabler/utfWrappers/src/utfExtHelpers.c
*/
#ifndef CPSS_APP_PLATFORM_REFERENCE

/* includes */
#include <cpss/px/cpssPxTypes.h>
#include <utfTraffic/basic/cpssPxTgfBasicUT.h>
#include <cpss/px/ingress/cpssPxIngress.h>
#include <cpssCommon/cpssPresteraDefs.h>
#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>
#include <common/tgfCommon.h>
#include <trafficEngine/tgfTrafficGenerator.h>
#include <appDemo/boardConfig/appDemoBoardConfig.h>
#include <cpss/px/port/cpssPxPortManager.h>
#include <cpss/px/port/cpssPxPortManagerSamples.h>
#include <cpss/common/systemRecovery/cpssGenSystemRecovery.h>
#include <cpss/common/systemRecovery/private/prvCpssCommonSystemRecoveryParallel.h>

extern GT_STATUS appDemoHaEmulateSwCrash
(
    GT_VOID
);
extern GT_STATUS appDemoEventRequestDrvnModeInit
(
    IN GT_VOID
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


#define RECREATION_MAX_SIZE 20

typedef struct{
    GT_BOOL                         vaild;
    CPSS_PORT_INTERFACE_MODE_ENT    ifMode;
    CPSS_PORT_SPEED_ENT             speed;
    CPSS_PORT_FEC_MODE_ENT          fecMode;

} RECREATION_INFO;


static GT_CHAR_PTR prvTgfHighAvailabilityPxSpeedToStr
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
        case CPSS_PORT_SPEED_25000_E:
            ptr="25G";
            break;
        default:
            break;
    }

    return ptr;
}

static GT_CHAR_PTR prvTgfHighAvailabilityPxModeToStr
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

static GT_CHAR_PTR prvTgfHighAvailabilityPxFecToStr
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
* @internal prvTgfHighAvailabilityPxReplayAllPort function
* @endinternal
*
* @brief   configure all device port ,skip cpu and sdma ports
*
*/
static GT_STATUS prvTgfHighAvailabilityPxReplayAllPort
(
    GT_BOOL          portMng,
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
    GT_U32                          portIter;
    GT_CHAR_PTR                     speedStr,modeStr,feqStr;
    CPSS_PORTS_BMP_STC              portBmp;

    for (i = 0; i < PRV_CPSS_MAX_PP_DEVICES_CNS; i++)
    {
        if( appDemoPpConfigList[i].valid == GT_FALSE)
        {
            continue;
        }

        maxPhysicalPort = PRV_CPSS_PX_PORTS_NUM_CNS;
        PRV_UTF_LOG2_MAC("Start replay for ports device-%d .Loop until %d ports\n",i,maxPhysicalPort);
        for (portNum = 0; portNum < maxPhysicalPort; portNum++)
        {
            if (recreationRequiredPtr[portNum].vaild == GT_FALSE)
            {
                continue;
            }
            speedStr = NULL;
            modeStr = NULL;
            feqStr = NULL;
            speed = recreationRequiredPtr[portNum].speed;
            ifMode = recreationRequiredPtr[portNum].ifMode;
            fecMode = recreationRequiredPtr[portNum].fecMode;

            speedStr = prvTgfHighAvailabilityPxSpeedToStr(speed);
            modeStr = prvTgfHighAvailabilityPxModeToStr(ifMode);
            feqStr = prvTgfHighAvailabilityPxFecToStr(fecMode);

            if((NULL == speedStr)||(NULL == modeStr)||(NULL == feqStr))
            {
                PRV_UTF_LOG4_MAC("Port %3d params - speed [%d] ,ifMode [%d],fecMode [%d]\n",portNum,speed,ifMode,fecMode);
            }
            else
            {
                PRV_UTF_LOG4_MAC("Port %3d params - speed [%s] ,ifMode [%s],fecMode [%s]\n",portNum,speedStr,modeStr,feqStr);
            }
            if (portMng == GT_TRUE)
            {
                st = cpssPxSamplePortManagerMandatoryParamsSet(i, portNum, ifMode, speed, fecMode);
                if (GT_OK != st)
                {
                    PRV_UTF_LOG4_MAC("cpssPxSamplePortManagerMandatoryParamsSet FAILED:st-%d,portNum-%d,ifMode-%d,speed-%d\n",
                                                            st, portNum, ifMode, speed);
                    return st;
                }
            }
            else
            {
                CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portBmp);
                CPSS_PORTS_BMP_PORT_SET_MAC(&portBmp, portNum);
                st = cpssPxPortModeSpeedSet(i, &portBmp,GT_TRUE,ifMode, speed);
                if (GT_OK != st)
                {
                    PRV_UTF_LOG4_MAC("cpssPxPortModeSpeedSet FAILED:st-%d,portNum-%d,ifMode-%d,speed-%d\n",
                                                            st, portNum, ifMode, speed);
                    return st;
                }

                st = cpssPxIngressPortTargetEnableSet(i,portNum,GT_TRUE);
                if (GT_OK != st)
                {
                    PRV_UTF_LOG2_MAC("cpssPxIngressPortTargetEnableSet FAILED:st-%d,portNum-%d\n",
                                                            st, portNum);
                    return st;
                }
            }

            for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
            {
                if(prvTgfPortsArray[portIter] == portNum)
                {   
                    if (portMng == GT_TRUE)
                    {
                        st = cpssPxSamplePortManagerLoopbackSet(i, portNum, CPSS_PORT_MANAGER_LOOPBACK_TYPE_MAC_E,
                                                                   GT_TRUE , CPSS_PORT_SERDES_LOOPBACK_DISABLE_E, GT_FALSE);
                        if (GT_OK != st)
                        {
                            PRV_UTF_LOG4_MAC("cpssPxSamplePortManagerLoopbackSet FAILED:st-%d,portNum-%d,ifMode-%d,speed-%d\n",
                                                            st, portNum, ifMode, speed);
                            return st;
                        }
                    }
                    else
                    {
                        st = cpssPxPortInternalLoopbackEnableSet(i,portNum,GT_TRUE);
                        if (GT_OK != st)
                        {
                            PRV_UTF_LOG4_MAC("cpssPxSamplePortManagerLoopbackSet FAILED:st-%d,portNum-%d,ifMode-%d,speed-%d\n",
                                                            st, portNum, ifMode, speed);
                            return st;
                        }
                    }
                    break;
                }
            }
            if (portMng == GT_FALSE)
            {
                continue;
            }
            /*recreate port*/
            portEventStc.portEvent = CPSS_PORT_MANAGER_EVENT_CREATE_E;
            st = cpssPxPortManagerEventSet(i,portNum, &portEventStc);
            if (st !=GT_OK)
            {
                PRV_UTF_LOG1_MAC("=> FAIL to recreate port %d\n",portNum);
                PRV_UTF_LOG3_MAC("cpssPxPortManagerEventSet FAILED:st-%d,portNum-%d,portEvent %d \n",
                                                st, portNum, portEventStc.portEvent);
                return st;
            }
        }
    }
    return st;
}

static GT_STATUS prvTgfHighAvailabilityPxInitPhase
(
    GT_BOOL          portMng,
    RECREATION_INFO *recreationRequiredPtr
)
{
    GT_STATUS                       st = 0 ;
    GT_U32                          portNum,maxPhysicalPort;
    CPSS_PX_DETAILED_PORT_MAP_STC   portMapShadow;
    GT_BOOL                         portValidity = GT_FALSE;
    GT_BOOL                         MacValidity = GT_FALSE;
    CPSS_PM_PORT_PARAMS_STC         portParams;
    CPSS_PORT_SPEED_ENT             speed;
    CPSS_PORT_INTERFACE_MODE_ENT    ifMode;

    maxPhysicalPort = PRV_CPSS_PX_PORTS_NUM_CNS;

    for (portNum = 0; portNum < maxPhysicalPort; portNum++)
    {
        st = cpssPxPortPhysicalPortDetailedMapGet(prvTgfDevNum, portNum, &portMapShadow);
        if (st !=GT_OK)
        {
            PRV_UTF_LOG3_MAC("cpssPxPortPhysicalPortDetailedMapGet FAILED:st-%d devNum %d portNum %d",
                                          st, prvTgfDevNum,portNum);
            return st;
        }

        portValidity = portMapShadow.valid;
        /* skip 'CPU SDMA' and 'remote' */
        MacValidity = (portMapShadow.portMap.mappingType != CPSS_PX_PORT_MAPPING_TYPE_ETHERNET_MAC_E) ? GT_FALSE/* no MAC */ : GT_TRUE/*MAC*/;

        if ((portValidity == GT_FALSE) || (MacValidity == GT_FALSE))
        {
            continue;
        }

        if (portMapShadow.portMap.mappingType== CPSS_PX_PORT_MAPPING_TYPE_ETHERNET_MAC_E)
        {
            if (portMng == GT_TRUE)
            {
                st = cpssPxPortManagerPortParamsGet(prvTgfDevNum, portNum,&portParams);
                if (GT_OK != st)
                {
                    if(st== GT_NOT_INITIALIZED)
                    {
                        continue;
                    }
                    else
                    {
                        PRV_UTF_LOG2_MAC("cpssPxPortManagerPortParamsGet FAILED:st-%d,portNum-%d\n",
                                                                st, portNum);
                        return st;
                    }
                }

                recreationRequiredPtr[portNum].vaild = GT_TRUE;
                recreationRequiredPtr[portNum].speed = portParams.portParamsType.regPort.speed;
                recreationRequiredPtr[portNum].ifMode = portParams.portParamsType.regPort.ifMode;
                recreationRequiredPtr[portNum].fecMode = portParams.portParamsType.regPort.portAttributes.fecMode;
            }
            else
            {
                st = cpssPxPortSpeedGet(prvTgfDevNum,portNum,&speed);
                if (st !=GT_OK)
                {
                    PRV_UTF_LOG3_MAC("cpssPxPortSpeedGet FAILED:st-%d devNum %d portNum %d",
                                                  st, prvTgfDevNum,portNum);
                    return st;
                }
                st = cpssPxPortInterfaceModeGet(prvTgfDevNum,portNum,&ifMode);
                if (st !=GT_OK)
                {
                    PRV_UTF_LOG3_MAC("cpssPxPortInterfaceModeGet FAILED:st-%d devNum %d portNum %d",
                                                  st, prvTgfDevNum,portNum);
                    return st;
                }
                if ((speed != CPSS_PORT_SPEED_NA_E) && (ifMode != CPSS_PORT_INTERFACE_MODE_NA_E))
                {
                        recreationRequiredPtr[portNum].vaild = GT_TRUE;
                        recreationRequiredPtr[portNum].speed = speed;
                        recreationRequiredPtr[portNum].ifMode = ifMode;
                }
            }
        }
    }
    return st;
}

/**
* @internal prvTgfHighAvailabilityPxStatusSet function
* @endinternal
*
* @brief set default system Recovery Info for high availablity
*
* [in] stage      - select the stage of system recovery
*
*/
/* DO NOT MAKE this function STATIC  ,we may want to use it from shell in order to debug*/
GT_STATUS prvTgfHighAvailabilityPxStatusSet
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


UTF_TEST_CASE_MAC(tgfHighAvailabilityPxSimpleTest)
{
    /*
      1. Set high availablity basic configuration:
          - reset SW
          - set system recovery status to CPSS_SYSTEM_RECOVERY_INIT_STATE_E
          - reInitSysem with same configurations
          - set system recovery status to CPSS_SYSTEM_RECOVERY_HW_CATCH_UP_STATE_E
          - replay ports
          - set system recovery status to CPSS_SYSTEM_RECOVERY_COMPLETION_STATE_E
  */

    GT_BOOL                         portMng;
    GT_STATUS                       st = 0 ;
    RECREATION_INFO                 *recreationRequired=NULL;
    GT_U32                          devId =0;

	PRV_TGF_SUPPORTED_DEV_DECLARE_MAC((UTF_PIPE_E))
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, (~(UTF_PIPE_E)))

    st=appDemoDebugDeviceIdGet(prvTgfDevNum,&devId);
    if(st==GT_OK)
    {
        PRV_UTF_LOG1_MAC("Get Dev id configured  = 0x%x ",devId);
    }

    st = cpssPxPortManagerEnableGet(prvTgfDevNum,&portMng);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "ERROR : cpssPxPortManagerEnableGet FAILED:st-%d",st);

    recreationRequired = cpssOsMalloc(sizeof(RECREATION_INFO)*RECREATION_MAX_SIZE);
    if (recreationRequired == NULL)
    {
        st = GT_OUT_OF_CPU_MEM;
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, " error at recreationRequired cpssOsMalloc ");
    }
    cpssOsMemSet(recreationRequired, 0,sizeof(RECREATION_INFO)*RECREATION_MAX_SIZE);

    st = prvTgfHighAvailabilityPxInitPhase(portMng,recreationRequired);

    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "ERROR : prvTgfHighAvailabilityPxInitPhase FAILED:st-%d devNum %d",st,prvTgfDevNum);

    st = appDemoHaEmulateSwCrash();
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "ERROR : appDemoHaEmulateSwCrash FAILED:st-%d devNum %d",st,prvTgfDevNum);

    st = prvTgfHighAvailabilityPxStatusSet(1,GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "ERROR : prvTgfHighAvailabilityPxStatusSet :st-%d ",st);

    PRV_UTF_LOG0_MAC("\nInit w/o port configuration \n");
    st = appDemoDbEntryAdd("initSerdesDefaults", 1);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "ERROR : appDemoDbEntryAdd FAILED:st-%d ", st);

    if(devId)
    {
        PRV_UTF_LOG1_MAC("Set Dev id configured  = 0x%x ",devId);
        appDemoDebugDeviceIdSet(prvTgfDevNum,devId);
    }

    st = cpssReInitSystem();
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "ERROR : cpssReInitSystem FAILED:st-%d ", st);
    prvUtfSetAfterSystemResetState();
    st = prvTgfHighAvailabilityPxStatusSet(2,GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "ERROR : prvTgfHighAvailabilityPxStatusSet :st-%d ",st);

    st = prvTgfHighAvailabilityPxReplayAllPort(portMng,recreationRequired);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "ERROR : tfgHighAvailabilityReplayAllPort :st-%d ", st);

    st = prvTgfHighAvailabilityPxStatusSet(3,GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "ERROR : prvTgfHighAvailabilityPxStatusSet :st-%d ",st);

    PRV_UTF_LOG0_MAC("\nStart event handlers.\n");
    st = appDemoEventRequestDrvnModeInit();
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "ERROR : appDemoEventRequestDrvnModeInit :st-%d ",st);
}


/*
 * Configuration of tgfsystemRecovery suit
 */
UTF_SUIT_BEGIN_TESTS_MAC(tgfHighAvailabilityPx)
	UTF_SUIT_DECLARE_TEST_MAC(tgfHighAvailabilityPxSimpleTest)
UTF_SUIT_END_TESTS_MAC(tgfHighAvailabilityPx)

#endif
