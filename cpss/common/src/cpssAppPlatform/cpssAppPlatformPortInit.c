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
* @file cpssAppPlatformPortInit.c
*
* @brief Initialization functions for port using Port Manager.
*
* @version   1
********************************************************************************
*/

#if (defined CHX_FAMILY)
    #define DXCH_CODE
#endif /* (defined CHX_FAMILY) */
#if (defined PX_FAMILY)
    #define PX_CODE
#endif /* (defined PX_FAMILY) */

#include <cpssAppPlatformPortInit.h>
#include <cpssAppPlatformSysConfig.h>

#include <cpss/common/port/cpssPortManager.h>

#ifdef DXCH_CODE
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortManager.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortDiag.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortMapping.h>
#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgEgrFlt.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfoEnhanced.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortAp.h>
#endif

#ifdef PX_CODE
#include <cpss/px/port/cpssPxPortManager.h>
#endif

#include <cpss/common/port/private/prvCpssPortPcsCfg.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

#define CAP_PORT_MANAGER_MAC_LOOPBACK_CHECK(portparams) (                                                                          \
                 (portParams.portType == CPSS_PORT_MANAGER_PORT_TYPE_REGULAR_E) &&                                                      \
                 (portParams.portParamsType.regPort.portAttributes.loopback.loopbackType == CPSS_PORT_MANAGER_LOOPBACK_TYPE_MAC_E) &&   \
                 (portParams.portParamsType.regPort.portAttributes.loopback.loopbackMode.macLoopbackEnable))
#define CAP_PORT_MANAGER_ANALOG_TX2RX_LOOPBACK_CHECK(portparams) (                                                                  \
                 (portParams.portType == CPSS_PORT_MANAGER_PORT_TYPE_REGULAR_E) &&                                                       \
                 (portParams.portParamsType.regPort.portAttributes.loopback.loopbackType == CPSS_PORT_MANAGER_LOOPBACK_TYPE_SERDES_E) && \
                 (portParams.portParamsType.regPort.portAttributes.loopback.loopbackMode.serdesLoopbackMode ==                           \
                 CPSS_PORT_SERDES_LOOPBACK_ANALOG_TX2RX_E))                                                                              \

CPSS_OS_SIG_SEM portInitSeqSignalSem=0;
GT_BOOL isSip6 = GT_FALSE;
static GT_U32    pmTid;
GT_BOOL          stopPmTask=GT_FALSE;

extern GT_BOOL portMgr;


GT_STATUS cpssGetDevicesValidity
(
    IN  GT_U8    devNum,
    OUT GT_BOOL *deviceValidityPtr
);

GT_STATUS cpssGetMaxPortNumber
(
    IN   GT_U8 devNum,
    OUT  GT_U16 *maxPortNumPtr
);

#ifdef ASIC_SIMULATION
static GT_U32   appRefPortManagerTaskSleepTime = 10;/*100 times in sec*/
#else
static GT_U32   appRefPortManagerTaskSleepTime = 100;/*10 times in sec */
#endif
/* debug option to state the 'sleep' for the port manager ! between 'all devices' iterations */
GT_STATUS appRefPortManagerTaskSleepTimeSet(IN GT_U32 timeInMilli)
{
    appRefPortManagerTaskSleepTime = timeInMilli;
    return GT_OK;
}


/**
* @internal cpssAppPlatformPmPortCreate function
* @endinternal
*
* @brief   Create port using port manager.
*
* @param[in] devNum                - device number
* @param[in] portNum               - port number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on passing null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssAppPlatformPmPortCreate
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum
)
{
    GT_STATUS rc;
    CPSS_PORT_MANAGER_STC portEventStc;

    rc = GT_NOT_APPLICABLE_DEVICE;

    CPSS_APP_PLATFORM_LOG_DBG_MAC("Inside first stage: Inside init stage\n");

    /****************************************
            Create Port
    *****************************************/
    portEventStc.portEvent = CPSS_PORT_MANAGER_EVENT_CREATE_E;
    CPSS_APP_PLATFORM_LOG_DBG_MAC("calling the stage config with stage INIT\n");
#ifdef PX_CODE
    if (CPSS_IS_PX_FAMILY_MAC(PRV_CPSS_PP_MAC(devNum)->devFamily))
    {
        rc = cpssPxPortManagerEventSet(devNum, portNum, &portEventStc);
    }
#endif
#ifdef DXCH_CODE
    if (CPSS_IS_DXCH_FAMILY_MAC(PRV_CPSS_PP_MAC(devNum)->devFamily))
    {
        rc = cpssDxChPortManagerEventSet(devNum, portNum, &portEventStc);
    }
#endif
    if(rc != GT_OK)
    {
        CPSS_APP_PLATFORM_LOG_ERR_MAC("calling PortManagerEventSet event=%d ret=%d, portNum=%d\n", portEventStc.portEvent, rc, portNum);
        return rc;
    }

    return GT_OK;
}

/**
* @internal cpssAppPlatformPmPortStatusChangeSignal function
* @endinternal
*
* @brief   Handler for port events. This API will pass
*         CPSS_PORT_MANAGER_EVENT_LOW_LEVEL_STATUS_CHANGED_E (or
*         CPSS_PORT_MANAGER_EVENT_PORT_AP_HCD_FOUND_E) event to port manager
*         in order to notify it about a (possibly) change in the port status,
*         and then will initialize the port (if needed) using CPSS_PORT_MANAGER_EVENT_INIT_E
*         event (as long as the port is in CPSS_PORT_MANAGER_STATE_INIT_IN_PROGRESS_E state)
*
* @param[in] devNum                - device number
* @param[in] portNum               - port number
* @param[in] uniEvent              - unified event raised on the port
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on passing null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssAppPlatformPmPortStatusChangeSignal
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    GT_U32                      uniEvent
)
{
    GT_STATUS                       rc;
    CPSS_PORT_MANAGER_STATUS_STC    portConfigOutParams;
    CPSS_PORT_MANAGER_STC           portEventStc;

    rc = GT_NOT_APPLICABLE_DEVICE;

    CPSS_APP_PLATFORM_LOG_DBG_MAC("(port %d)uniEvent: %d",portNum, uniEvent);

    /************************************************************
        1. Getting port status
    ***********************************************************/
#ifdef PX_CODE
    if (CPSS_IS_PX_FAMILY_MAC(PRV_CPSS_PP_MAC(devNum)->devFamily))
    {
        rc = cpssPxPortManagerStatusGet(devNum, portNum, &portConfigOutParams);
    }
#endif
#ifdef DXCH_CODE
    if (CPSS_IS_DXCH_FAMILY_MAC(PRV_CPSS_PP_MAC(devNum)->devFamily))
    {
        rc = cpssDxChPortManagerStatusGet(devNum, portNum, &portConfigOutParams);
    }
#endif
    if(rc != GT_OK)
    {
        CPSS_APP_PLATFORM_LOG_ERR_MAC("PortManagerStatusGet returned=%d, portNum=%d\n", rc, portNum);
        return rc;
    }
    /* if port is in failure state, not passing the event to cpss, app need to furthur check
       this issue, possible bad connectivity.
       (Alternatively, app can decide to notify port manager with the event and then port will continue the flow).
       also if port is disabled not doing it. */
    if ( portConfigOutParams.portState == CPSS_PORT_MANAGER_STATE_FAILURE_E ||
        ((portConfigOutParams.portUnderOperDisable == GT_TRUE) && (!PRV_CPSS_SIP_6_10_CHECK_MAC(devNum)) && (!PRV_CPSS_DXCH_IS_AC3_BASED_DEVICE_MAC(devNum))))
    {
        CPSS_APP_PLATFORM_LOG_DBG_MAC("Port %d is in HW failure state or in disable so abort\n", portNum);
        return GT_OK;
    }

    /************************************************************
        2. Passing event to port manager
    ***********************************************************/
    switch (uniEvent) {
        case CPSS_SRVCPU_PORT_AP_DISABLE_E:
            portEventStc.portEvent = CPSS_PORT_MANAGER_EVENT_PORT_AP_DISABLE_E;
            break;
        case CPSS_SRVCPU_PORT_802_3_AP_E:
        case CPSS_PP_PORT_AN_HCD_FOUND_E:
            portEventStc.portEvent = CPSS_PORT_MANAGER_EVENT_PORT_AP_HCD_FOUND_E;
            break;
        case CPSS_PP_PORT_AN_RESTART_E:
            portEventStc.portEvent = CPSS_PORT_MANAGER_EVENT_PORT_AP_RESTART_E;
            break;
        case CPSS_PP_PORT_AN_PARALLEL_DETECT_E:
            portEventStc.portEvent = CPSS_PORT_MANAGER_EVENT_PORT_AP_PARALLEL_DETECT_E;
            break;
        default:
            portEventStc.portEvent = CPSS_PORT_MANAGER_EVENT_LOW_LEVEL_STATUS_CHANGED_E;
            break;
    }
#ifdef PX_CODE
    if (CPSS_IS_PX_FAMILY_MAC(PRV_CPSS_PP_MAC(devNum)->devFamily))
    {
        rc = cpssPxPortManagerEventSet(devNum, portNum, &portEventStc);
    }
#endif
#ifdef DXCH_CODE
    if (CPSS_IS_DXCH_FAMILY_MAC(PRV_CPSS_PP_MAC(devNum)->devFamily))
    {
        rc = cpssDxChPortManagerEventSet(devNum, portNum, &portEventStc);
    }
#endif
    if(rc != GT_OK)
    {
        CPSS_APP_PLATFORM_LOG_ERR_MAC("calling PortManagerEventSet event=%d returned=%d, portNum=%d\n", portEventStc.portEvent, rc, portNum);
        return rc;
    }

    /************************************************************
        3. Signaling dedicated task.
    ***********************************************************/
    if ((CPSS_PORT_MANAGER_EVENT_PORT_AP_HCD_FOUND_E != portEventStc.portEvent) &&
        (CPSS_PORT_MANAGER_EVENT_PORT_AP_DISABLE_E != portEventStc.portEvent) &&
        (CPSS_PORT_MANAGER_EVENT_PORT_AP_RESTART_E != portEventStc.portEvent) &&
        (CPSS_PORT_MANAGER_EVENT_PORT_AP_PARALLEL_DETECT_E != portEventStc.portEvent))
    {
        CPSS_APP_PLATFORM_LOG_DBG_MAC("signaling signal detect task\n");
        /* Signal the handler */
        if(isSip6 == GT_FALSE)
        {
            rc = cpssOsSigSemSignal(portInitSeqSignalSem);
        }
    }

    if(rc != GT_OK)
    {
        CPSS_APP_PLATFORM_LOG_ERR_MAC("calling cpssOsSigSemSignal returned=%d, portNum=%d", rc, portNum);
        return rc;
    }

    return GT_OK;
}

/*******************************************************************************
* cpssAppPlatformPortInitSeqSignalDetectedStage
*
* DESCRIPTION:
*       Initialize the port (if needed) using CPSS_PORT_MANAGER_EVENT_INIT_E
*       event, as long as the port is in CPSS_PORT_MANAGER_STATE_INIT_IN_PROGRESS_E state.
*
* INPUTS:
*       None.
*
* OUTPUTS:
*       None.
*
* COMMENTS:
*       None.
*
*******************************************************************************/
unsigned __TASKCONV cpssAppPlatformPortInitSeqSignalDetectedStage
(
    GT_VOID_PTR param
)
{
    GT_PHYSICAL_PORT_NUM    portNum = 0;
    GT_U8                   devNum;
    GT_STATUS               rc, rc1;
    GT_U8                   portsExistFlag;
    CPSS_PORT_MANAGER_STATUS_STC   portConfigOutParams;
    CPSS_PORT_MANAGER_STC          portEventStc;
    GT_U16                  maxPort, i,jj;
    GT_U8                   indexDevices;
    GT_BOOL                 deviceValidity;
#ifdef DXCH_CODE
    CPSS_DXCH_DETAILED_PORT_MAP_STC portMapShadow;
    GT_BOOL                 portValidity = GT_FALSE;
    GT_BOOL                 MacValidity = GT_FALSE;
    GT_BOOL                 signalDetect = GT_FALSE;
    GT_BOOL                 serdesLoopback = GT_FALSE;
    CPSS_PM_PORT_PARAMS_STC portParams;
    CPSS_DXCH_PORT_AP_STATUS_STC apStatusStc;
    GT_BOOL                          linkUp = GT_FALSE;
    GT_BOOL                          loopback = GT_FALSE;
#endif

    rc = GT_NOT_APPLICABLE_DEVICE;

    (void)param;

    while(1)
    {
        if(isSip6 == GT_FALSE)
        {
            cpssOsSigSemWait(portInitSeqSignalSem, 0);
        }
        /* marking port-exist flag so iteration cycles will be performed at least once */
        portsExistFlag = 1;

        while (portsExistFlag>0)
        {
            /* by here, the next 'for' loop is promised to run, so we can mark this flag with
               zero. The only case in which the 'while' will be iterated again is if there is existing port
               in INIT state (checked in the next scope), at which we want to continue the iterations */
            if(isSip6 == GT_FALSE)
            {
                /* for SIP6  the Semaphore is not used so the task need to run each polling interval */
                portsExistFlag = 0;
            }

            if(GT_TRUE==stopPmTask)
            {
                stopPmTask = GT_FALSE;
                /* go into sleeps ...   waiting for the 'osTaskDelete' to delete this task */
                for(jj = 0 ; jj < 100 ; jj++)
                {
                    cpssOsTimerWkAfter(1000);
                }
            }


            for (indexDevices = 0; indexDevices < PRV_CPSS_MAX_PP_DEVICES_CNS; indexDevices++)
            {
                portNum = 0;
                devNum = indexDevices;


                rc = cpssGetDevicesValidity(devNum, &deviceValidity);
                if(rc != GT_OK)
                {

                    continue; /* must not return to keep the task alive */
                }
                if (deviceValidity == GT_TRUE)
                {
                    if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_FALCON_E)
                    {
                        maxPort = 256;
                    }
                    else
                    {
                        rc = cpssGetMaxPortNumber(devNum, &maxPort);
                        if(rc != GT_OK)
                        {
                            CPSS_APP_PLATFORM_LOG_ERR_MAC("cpssGetMaxPortNumber: devNum %d returned with error %d", devNum, rc);
                            continue; /* must not return to keep the task alive */
                        }
                    }

                    for (i = 0; i < maxPort; i++)
                    {
#ifdef DXCH_CODE
                        /*skip on invalid ports and "cpu sdma" and remote ports*/
                        if (CPSS_IS_DXCH_FAMILY_MAC(PRV_CPSS_PP_MAC(devNum)->devFamily))
                        {
                            do
                            {
                                rc = cpssDxChPortPhysicalPortDetailedMapGet(devNum, portNum, &portMapShadow);
                                if(rc != GT_OK)
                                {
                                    if (portNum < CPSS_MAX_PORTS_NUM_CNS)
                                    {
                                        portNum++;
                                        portValidity = GT_FALSE;
                                        MacValidity = GT_FALSE;
                                        continue;
                                    }
                                    else
                                    {
                                        break;
                                    }
                                }

                                portValidity = portMapShadow.valid;
                                /* skip 'CPU SDMA' and 'remote' */
                                MacValidity = (portMapShadow.portMap.mappingType == CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E) ? GT_TRUE : GT_FALSE;


                                if ((portValidity == GT_FALSE) || (MacValidity == GT_FALSE))
                                {
                                    if (portNum < CPSS_MAX_PORTS_NUM_CNS)
                                    {
                                        portNum++;
                                    }
                                    else
                                    {
                                        break;
                                    }
                                }
                            }
                            while ((portValidity == GT_FALSE) || (MacValidity == GT_FALSE));
                        }

                        if ((portValidity != GT_FALSE) && (MacValidity != GT_FALSE))
                        {
                            rc = cpssDxChPortManagerPortParamsGet(devNum, portNum, &portParams);
                            if (((rc != GT_OK) && (rc != GT_NOT_INITIALIZED)))
                            {
                                CPSS_APP_PLATFORM_LOG_ERR_MAC("cpssDxChPortManagerPortParamsGet %d:   %d:\n", rc, portParams.portType);
                                if (portNum < CPSS_MAX_PORTS_NUM_CNS)
                                {
                                    portNum++;
                                    continue;
                                }
                                else
                                {
                                    break;
                                }
                            }
                        }
#endif
                        /************************************************************************
                           1. Get port status
                        **********************************************************************/
                        portConfigOutParams.portState               = CPSS_PORT_MANAGER_STATE_RESET_E;
                        portConfigOutParams.portUnderOperDisable    = GT_FALSE;
                        portConfigOutParams.failure                 = CPSS_PORT_MANAGER_FAILURE_NONE_E;
                        portConfigOutParams.ifMode                  = CPSS_PORT_INTERFACE_MODE_NA_E;
                        portConfigOutParams.speed                   = CPSS_PORT_SPEED_NA_E;
                        portConfigOutParams.fecType                 = CPSS_PORT_FEC_MODE_LAST_E;
                        portConfigOutParams.remoteFaultConfig       = GT_FALSE;
#ifdef PX_CODE
                        if (CPSS_IS_PX_FAMILY_MAC(PRV_CPSS_PP_MAC(devNum)->devFamily))
                        {
                            rc = cpssPxPortManagerStatusGet(devNum, portNum, &portConfigOutParams);
                        }
#endif
#ifdef DXCH_CODE
                        if (CPSS_IS_DXCH_FAMILY_MAC(PRV_CPSS_PP_MAC(devNum)->devFamily))
                        {
                            rc = cpssDxChPortManagerStatusGet(devNum, portNum, &portConfigOutParams);
                        }
#endif

                        /* GT_BAD_PARAM will be received upon bad portNum argument. We iterating
                           all possible portNum values so in GT_BAD_PARAM case we will ignore this
                           error code and continue execution, either way we won't enter the 2nd 'if'
                           statement as bad portNum argument will not meet the statement condition */
                        if((rc != GT_OK) || (portConfigOutParams.portState == CPSS_PORT_MANAGER_STATE_RESET_E) ||
                                   ((portConfigOutParams.portUnderOperDisable == GT_TRUE) && (!PRV_CPSS_SIP_6_10_CHECK_MAC(devNum)) && (!PRV_CPSS_DXCH_IS_AC3_BASED_DEVICE_MAC(devNum))))
                        {
                            if (portNum < CPSS_MAX_PORTS_NUM_CNS)
                            {
                                portNum++;
                                continue;
                            }
                            else
                            {
                                break;
                            }
                        }

                        /************************************************************************
                           2. If port state is INIT than continue to call INIT_EVENT
                        **********************************************************************/
#ifdef DXCH_CODE
                        if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
                        {
                            rc = cpssDxChPortLinkStatusGet(devNum, portNum, &linkUp);
                            if(rc != GT_OK)
                            {
                                CPSS_APP_PLATFORM_LOG_ERR_MAC("cpssDxChPortLinkStatusGet returned=%d, portNum=%d", rc, portNum);
                            }

                            if (( portParams.portType == CPSS_PORT_MANAGER_PORT_TYPE_802_3AP_E ) ||
                                    ((portParams.portParamsType.regPort.portAttributes.validAttrsBitMask & CPSS_PM_PORT_ATTR_PM_OVER_FW_E) && (portParams.portParamsType.regPort.portAttributes.pmOverFw)) )
                            {
                                if(portConfigOutParams.portState == CPSS_PORT_MANAGER_STATE_LINK_DOWN_E && (linkUp == GT_TRUE))
                                {
                                    rc = cpssDxChPortApPortStatusGet(devNum, portNum, &apStatusStc);
                                    if(rc != GT_OK)
                                    {
                                        CPSS_APP_PLATFORM_LOG_ERR_MAC("calling cpssDxChPortApPortStatusGet returned=%d, portNum=%d", rc, portNum);
                                    }
                                    if ( apStatusStc.hcdFound )
                                    {
                                        portEventStc.portEvent = CPSS_PORT_MANAGER_EVENT_MAC_LEVEL_STATUS_CHANGED_E;
                                        rc = cpssDxChPortManagerEventSet(devNum, portNum, &portEventStc);
                                        if(rc != GT_OK)
                                        {
                                            CPSS_APP_PLATFORM_LOG_ERR_MAC("calling PortManagerEventSet event %d returned=%d, portNum=%d", portEventStc.portEvent, rc, portNum);
                                        }
                                    }
                                }
                                if (portNum < CPSS_MAX_PORTS_NUM_CNS)
                                {
                                    portNum++;
                                    continue;
                                }
                                else
                                {
                                    break;
                                }
                            }
                            if(linkUp == GT_TRUE && (portConfigOutParams.portState == CPSS_PORT_MANAGER_STATE_MAC_LINK_DOWN_E))
                            {
                                portEventStc.portEvent = CPSS_PORT_MANAGER_EVENT_MAC_LEVEL_STATUS_CHANGED_E;
                                rc = cpssDxChPortManagerEventSet(devNum, portNum, &portEventStc);
                                if(rc != GT_OK)
                                {
                                    CPSS_APP_PLATFORM_LOG_ERR_MAC("calling PortManagerEventSet event %d returned=%d, portNum=%d", portEventStc.portEvent, rc, portNum);
                                }
                            }
                            loopback = CAP_PORT_MANAGER_MAC_LOOPBACK_CHECK(portParams);
                            if(!loopback)
                            {
                                if(portConfigOutParams.portState == CPSS_PORT_MANAGER_STATE_LINK_DOWN_E)
                                {
                                    rc = cpssDxChPortSerdesSignalDetectGet(devNum,portNum,&signalDetect);
                                    if(rc != GT_OK)
                                    {
                                        /* some of the errors in signal detect get can be because we checking the port manager status and is on link up/down and then immediately the port deleted
                                           and the app demo stay on link up/down status and not reset until next iteration.
                                           This is not error but in order to ingore flase return errors, check the status again and if in reset, stop the loop for this port until next create */
                                        rc1 = cpssDxChPortManagerStatusGet(devNum, portNum, &portConfigOutParams);
                                        if(rc1 != GT_OK)
                                        {
                                            CPSS_APP_PLATFORM_LOG_ERR_MAC("calling cpssDxChPortManagerStatusGet returned=%d, portNum=%d", rc1, portNum);
                                        }

                                        if(portConfigOutParams.portState == CPSS_PORT_MANAGER_STATE_RESET_E)
                                        {
                                            continue;
                                        }
                                        CPSS_APP_PLATFORM_LOG_ERR_MAC("calling cpssDxChPortSerdesSignalDetectGet returned=%d, portNum=%d", rc, portNum);
                                    }
                                    if(signalDetect)
                                    {
                                        portEventStc.portEvent = CPSS_PORT_MANAGER_EVENT_LOW_LEVEL_STATUS_CHANGED_E;

                                        rc = cpssDxChPortManagerEventSet(devNum, portNum, &portEventStc);
                                        if(rc != GT_OK)
                                        {
                                            CPSS_APP_PLATFORM_LOG_ERR_MAC("calling PortManagerEventSet event %d returned=%d, portNum=%d", portEventStc.portEvent, rc, portNum);
                                        }
                                    }
                                }
                                serdesLoopback = CAP_PORT_MANAGER_ANALOG_TX2RX_LOOPBACK_CHECK(portParams);
                                if((portConfigOutParams.portState == CPSS_PORT_MANAGER_STATE_MAC_LINK_DOWN_E) && serdesLoopback)
                                {
                                    rc = cpssDxChPortSerdesSignalDetectGet(devNum,portNum,&signalDetect);
                                    if(rc != GT_OK)
                                    {
                                        CPSS_APP_PLATFORM_LOG_ERR_MAC("calling cpssDxChPortSerdesSignalDetectGet returned=%d, portNum=%d", rc, portNum);
                                    }
                                    if(signalDetect)
                                    {
                                        portEventStc.portEvent = CPSS_PORT_MANAGER_EVENT_LOW_LEVEL_STATUS_CHANGED_E;

                                        rc = cpssDxChPortManagerEventSet(devNum, portNum, &portEventStc);
                                        if(rc != GT_OK)
                                        {
                                            CPSS_APP_PLATFORM_LOG_ERR_MAC("calling PortManagerEventSet event %d returned=%d, portNum=%d", portEventStc.portEvent, rc, portNum);
                                        }
                                    }
                                }
                                if((portConfigOutParams.ifMode != CPSS_PORT_INTERFACE_MODE_QSGMII_E) && (portConfigOutParams.portState == CPSS_PORT_MANAGER_STATE_LINK_UP_E) && !(CAP_PORT_MANAGER_ANALOG_TX2RX_LOOPBACK_CHECK(portParams)))
                                {
                                    if (PRV_CPSS_SIP_6_CHECK_MAC(devNum))
                                    {
                                        GT_BOOL alignLockStatus;
                                        GT_U32 portMacNum;
                                        PRV_CPSS_DXCH_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);
                                        /* read align lock*/
                                        rc = prvCpssPortPcsAlignLockStatusGet(devNum, portNum, portMacNum, &alignLockStatus);
                                        if ( !alignLockStatus )
                                        {
                                            portEventStc.portEvent = CPSS_PORT_MANAGER_EVENT_LOW_LEVEL_STATUS_CHANGED_E;
                                            rc = cpssDxChPortManagerEventSet(devNum, portNum, &portEventStc);
                                            if(rc != GT_OK)
                                            {
                                                CPSS_APP_PLATFORM_LOG_ERR_MAC("calling PortManagerEventSet event %d returned=%d, portNum=%d", portEventStc.portEvent, rc, portNum);
                                            }
                                        }
                                    }
                                    else
                                    {
                                        rc = cpssDxChPortSerdesSignalDetectGet(devNum,portNum,&signalDetect);
                                        if(rc != GT_OK)
                                        {
                                                CPSS_APP_PLATFORM_LOG_ERR_MAC("calling cpssDxChPortSerdesSignalDetectGet returned=%d, portNum=%d", rc, portNum);
                                        }
                                        if(!signalDetect)
                                        {
                                            portEventStc.portEvent = CPSS_PORT_MANAGER_EVENT_LOW_LEVEL_STATUS_CHANGED_E;

                                            rc = cpssDxChPortManagerEventSet(devNum, portNum, &portEventStc);
                                            if(rc != GT_OK)
                                            {
                                                CPSS_APP_PLATFORM_LOG_ERR_MAC("calling PortManagerEventSet event %d returned=%d, portNum=%d", portEventStc.portEvent, rc, portNum);
                                            }
                                        }
                                    }
                                }

                            }
                        }
#endif
                        if (portConfigOutParams.portState == CPSS_PORT_MANAGER_STATE_INIT_IN_PROGRESS_E)
                        {
                            CPSS_APP_PLATFORM_LOG_DBG_MAC("Launching whole INIT on port\n");
                            /* marking that there is a port in INIT state so iteration cycles will continue */
                            portsExistFlag = 1;
                            portEventStc.portEvent = CPSS_PORT_MANAGER_EVENT_INIT_E;
#ifdef PX_CODE
                            if (CPSS_IS_PX_FAMILY_MAC(PRV_CPSS_PP_MAC(devNum)->devFamily))
                            {
                                rc = cpssPxPortManagerEventSet(devNum, portNum, &portEventStc);
                            }
#endif
#ifdef DXCH_CODE
                            if (CPSS_IS_DXCH_FAMILY_MAC(PRV_CPSS_PP_MAC(devNum)->devFamily))
                            {
                                rc = cpssDxChPortManagerEventSet(devNum, portNum, &portEventStc);
                            }
#endif
                            if(rc != GT_OK)
                            {
                                CPSS_APP_PLATFORM_LOG_ERR_MAC("calling PortManagerEventSet event %d returned=%d, portNum=%d", portEventStc.portEvent, rc, portNum);
                            }
                        }
                        if (portNum < CPSS_MAX_PORTS_NUM_CNS)
                        {
                            portNum++;
                        }
                        else
                        {
                            break;
                        }
                        if (i % 32 == 31)
                        {


                            cpssOsTimerWkAfter(appRefPortManagerTaskSleepTime/4);


                        }
                    } /* for (portNum) */
                }

            }

        cpssOsTimerWkAfter(appRefPortManagerTaskSleepTime);
        } /* while (portsExistFlag>0) */

    } /* while (1) */

    return GT_OK;
}

/**
* @internal cpssAppPlatformPmPortEnable function
* @endinternal
*
* @brief   Enable the port. This API is the complement of cpssAppPlatformPmPortDisable
*          API, while the later disabled the port, this API will enable it back.
*
* @param[in] devNum                - device number
* @param[in] portNum               - port number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on passing null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssAppPlatformPmPortEnable
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum
)
{
    GT_STATUS rc;
    CPSS_PORT_MANAGER_STC portEventStc;

    rc = GT_NOT_APPLICABLE_DEVICE;

    CPSS_APP_PLATFORM_LOG_INFO_MAC("Enabling port bitmaps");
    /**************************
           Enable Port
    **************************/
    portEventStc.portEvent = CPSS_PORT_MANAGER_EVENT_ENABLE_E;
#ifdef PX_CODE
    if (CPSS_IS_PX_FAMILY_MAC(PRV_CPSS_PP_MAC(devNum)->devFamily))
    {
        rc = cpssPxPortManagerEventSet(devNum, portNum, &portEventStc);
    }
#endif
#ifdef DXCH_CODE
    if (CPSS_IS_DXCH_FAMILY_MAC(PRV_CPSS_PP_MAC(devNum)->devFamily))
    {
        rc = cpssDxChPortManagerEventSet(devNum, portNum, &portEventStc);
    }
#endif
    if(rc != GT_OK)
    {
        CPSS_APP_PLATFORM_LOG_ERR_MAC("calling PortManagerEventSet event %d returned=%d, portNum=%d", portEventStc.portEvent, rc, portNum);
        return rc;
    }

    return GT_OK;
}

/**
* @internal cpssAppPlatformPmPortLinkStatusChangeSignal function
* @endinternal
*
* @brief   Handler function for link status change event. This API will pass
*         the event to the port manager and perform actions if needed.
*
* @param[in] devNum                - device number
* @param[in] portNum               - port number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on passing null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssAppPlatformPmPortLinkStatusChangeSignal
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum
)
{
    GT_STATUS rc;
    CPSS_PORT_MANAGER_STC        portEventStc;
    CPSS_PORT_MANAGER_STATUS_STC portStage;
    CPSS_PM_PORT_PARAMS_STC      portParams;
    GT_BOOL                      linkUp        = GT_FALSE;
    GT_BOOL                      linkStatusPM  = GT_FALSE; /* the link status accroding to the pm data base*/

    CPSS_APP_PLATFORM_LOG_ENTRY_MAC();

    rc = GT_NOT_APPLICABLE_DEVICE;

    /*******************************
          1. Notify port manager
    *******************************/
    CPSS_APP_PLATFORM_LOG_DBG_MAC("Launching whole tune on ports\n");
#ifdef PX_CODE
    if (CPSS_IS_PX_FAMILY_MAC(PRV_CPSS_PP_MAC(devNum)->devFamily))
    {
        rc = cpssPxPortManagerStatusGet(devNum, portNum, &portStage);
    }
#endif
#ifdef DXCH_CODE
    if (CPSS_IS_DXCH_FAMILY_MAC(PRV_CPSS_PP_MAC(devNum)->devFamily))
    {
        rc = cpssDxChPortManagerStatusGet(devNum, portNum, &portStage);
    }
#endif
    if(rc != GT_OK)
    {
        CPSS_APP_PLATFORM_LOG_ERR_MAC("PortManagerStatusGet returned=%d, portNum=%d\n", rc, portNum);
        return rc;
    }

    if (portStage.portState == CPSS_PORT_MANAGER_STATE_FAILURE_E ||
        ((portStage.portUnderOperDisable == GT_TRUE) && (!PRV_CPSS_SIP_6_10_CHECK_MAC(devNum)) && (!PRV_CPSS_DXCH_IS_AC3_BASED_DEVICE_MAC(devNum))))
    {
        CPSS_APP_PLATFORM_LOG_DBG_MAC("port is in HW failure state or in disable so abort mac_level_change notification\n");
        return GT_OK;
    }
    if (portStage.portState == CPSS_PORT_MANAGER_STATE_RESET_E )
    {
        CPSS_APP_PLATFORM_LOG_DBG_MAC("port is in reset state so abort mac_level_change notification\n");
        return GT_OK;
    }

#ifdef PX_CODE
    if (CPSS_IS_PX_FAMILY_MAC(PRV_CPSS_PP_MAC(devNum)->devFamily))
    {
        rc = cpssPxPortManagerPortParamsGet(devNum, portNum, &portParams);
    }
#endif
#ifdef DXCH_CODE
    if (CPSS_IS_DXCH_FAMILY_MAC(PRV_CPSS_PP_MAC(devNum)->devFamily))
    {
        rc = cpssDxChPortManagerPortParamsGet(devNum, portNum, &portParams);
    }
#endif

    if (portParams.portType == CPSS_PORT_MANAGER_PORT_TYPE_802_3AP_E)
    {
#ifdef DXCH_CODE
        rc = cpssDxChPortLinkStatusGet(devNum, portNum, &linkUp);
        if(rc != GT_OK)
        {
            CPSS_APP_PLATFORM_LOG_INFO_MAC("cpssDxChPortLinkStatusGet returned=%d, portNum=%d", rc, portNum);
        }
#endif
#ifdef PX_CODE
        rc = cpssPxPortLinkStatusGet(devNum, portNum, &linkUp);
        if(rc != GT_OK)
        {
            CPSS_APP_PLATFORM_LOG_INFO_MAC("cpssPxPortLinkStatusGet returned=%d, portNum=%d", rc, portNum);
        }
#endif
        if(portStage.portState == CPSS_PORT_MANAGER_STATE_LINK_UP_E)
        {
            linkStatusPM = GT_TRUE;
        }

        if (linkStatusPM == linkUp)
        {
            /* there is no change in the signal status, no need to handle status change anymore.
               this was made to ignore interrupts that has no effect on the link */
            return GT_OK;
        }
    }

    portEventStc.portEvent = CPSS_PORT_MANAGER_EVENT_MAC_LEVEL_STATUS_CHANGED_E;


#ifdef PX_CODE
    if (CPSS_IS_PX_FAMILY_MAC(PRV_CPSS_PP_MAC(devNum)->devFamily))
    {
        rc = cpssPxPortManagerEventSet(devNum, portNum, &portEventStc);
    }
#endif
#ifdef DXCH_CODE
    if (CPSS_IS_DXCH_FAMILY_MAC(PRV_CPSS_PP_MAC(devNum)->devFamily))
    {
        rc = cpssDxChPortManagerEventSet(devNum, portNum, &portEventStc);
    }
#endif
    if(rc != GT_OK)
    {
        CPSS_APP_PLATFORM_LOG_ERR_MAC("PortManagerEventSet event %d returned=%d, portNum=%d\n", portEventStc.portEvent, rc, portNum);
        return rc;
    }

    return GT_OK;
}

/**
* @internal cpssAppPlatformPmPortDisable function
* @endinternal
*
* @brief   Disable the port. This API is the complement of cpssAppPlatformPmPortEnable
*         API, while the later enables the port, this API will disable it.
*
* @param[in] devNum                - device number
* @param[in] portNum               - port number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on passing null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssAppPlatformPmPortDisable
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum
)
{
    GT_STATUS rc;
    CPSS_PORT_MANAGER_STC portEventStc;

    CPSS_APP_PLATFORM_LOG_ENTRY_MAC();

    rc = GT_NOT_APPLICABLE_DEVICE;

    /*******************************
            Disable Port
    *******************************/
    portEventStc.portEvent = CPSS_PORT_MANAGER_EVENT_DISABLE_E;
#ifdef PX_CODE
    if (CPSS_IS_PX_FAMILY_MAC(PRV_CPSS_PP_MAC(devNum)->devFamily))
    {
        rc = cpssPxPortManagerEventSet(devNum, portNum, &portEventStc);
    }
#endif
#ifdef DXCH_CODE
    if (CPSS_IS_DXCH_FAMILY_MAC(PRV_CPSS_PP_MAC(devNum)->devFamily))
    {
        rc = cpssDxChPortManagerEventSet(devNum, portNum, &portEventStc);
    }
#endif

    if(rc != GT_OK)
    {
        CPSS_APP_PLATFORM_LOG_ERR_MAC("PortManagerEventSet with PORT_STAGE_DISABLE_E ended-bad and returned=%d, portNum=%d", rc, portNum);
        return rc;
    }


    CPSS_APP_PLATFORM_LOG_EXIT_MAC(rc);
    return GT_OK;
}

/**
* @internal cpssAppPlatformPmPortDelete function
* @endinternal
*
* @brief   Delete port.
*
* @param[in] devNum                - device number
* @param[in] portNum               - port number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on passing null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssAppPlatformPmPortDelete
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum
)
{
    GT_STATUS rc;
    CPSS_PORT_MANAGER_STC portEventStc;

    CPSS_APP_PLATFORM_LOG_ENTRY_MAC();

    rc = GT_NOT_APPLICABLE_DEVICE;

    /*******************************
            Delete Port
    *******************************/
    portEventStc.portEvent = CPSS_PORT_MANAGER_EVENT_DELETE_E;
#ifdef PX_CODE
    if (CPSS_IS_PX_FAMILY_MAC(PRV_CPSS_PP_MAC(devNum)->devFamily))
    {
        rc = cpssPxPortManagerEventSet(devNum, portNum, &portEventStc);
    }
#endif
#ifdef DXCH_CODE
    if (CPSS_IS_DXCH_FAMILY_MAC(PRV_CPSS_PP_MAC(devNum)->devFamily))
    {
        rc = cpssDxChPortManagerEventSet(devNum, portNum, &portEventStc);
    }
#endif
    if(rc != GT_OK)
    {
        CPSS_APP_PLATFORM_LOG_ERR_MAC("PortManagerEventSet event %d returned=%d, portNum=%d", portEventStc.portEvent, rc, portNum);
        return rc;
    }

    return GT_OK;
}

/**
* @internal cpssAppPlatformPmTaskCreate function
* @endinternal
*
* @brief   create port manager application demo task
*
* @param[in] priority              - task priority
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on passing null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssAppPlatformPmTaskCreate
(
    IN GT_U32 priority
)
{
    GT_STATUS rc = GT_OK;
    GT_U8 devNum = 0;

    CPSS_APP_PLATFORM_LOG_ENTRY_MAC();

    for(devNum = 0; devNum < CPSS_APP_PLATFORM_MAX_PP_CNS; devNum++)
    {
        if(PRV_CPSS_IS_DEV_EXISTS_MAC(devNum))
        {
            isSip6 = PRV_CPSS_SIP_6_CHECK_MAC(devNum);
            break;
        }
    }

    if(isSip6 == GT_FALSE)
    {
        /* If handler was not made yet and we are in multi-threaded mode */
        if( (0 == portInitSeqSignalSem))
        {
            if(cpssOsSigSemBinCreate("portInitSeqSignalSem", CPSS_OS_SEMB_EMPTY_E, &portInitSeqSignalSem) != GT_OK)
            {
                CPSS_APP_PLATFORM_LOG_ERR_MAC("cpssOsSigSemBinCreate returned %d\n", GT_FAIL);
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
            }
        }
    }

    rc = cpssOsTaskCreate("portManagerTask", /* Task Name      */
                           priority,         /* Task Priority  */
                           _64K,             /* Stack Size     */
                           cpssAppPlatformPortInitSeqSignalDetectedStage, /* Starting Point */
                           NULL,             /* Arguments list */
                           &pmTid);       /* task ID        */
    if (rc != GT_OK)
    {
        CPSS_APP_PLATFORM_LOG_ERR_MAC("cpssOsTaskCreate returned=%d\n", rc);
        return rc;
    }

    CPSS_APP_PLATFORM_LOG_INFO_MAC("portManagerTask created with task id %d (0x%x)\n",pmTid,pmTid);

    return GT_OK;
}

/**
* @internal cpssAppPlatformPortInitSeqStart function
* @endinternal
*
* @brief   Bring-up port using port manager.
*
* @param[in] devNum                - device number
* @param[in] portNum               - port number
* @param[in] ifMode                - interface mode for the port
* @param[in] speed                 -  for the port
* @param[in] powerUp               - whether or not to power up the port
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssAppPlatformPortInitSeqStart
(
    IN  GT_U8                        devNum,
    IN  GT_PHYSICAL_PORT_NUM         portNum,
    IN  CPSS_PORT_INTERFACE_MODE_ENT ifMode,
    IN  CPSS_PORT_SPEED_ENT          speed,
    IN  CPSS_PORT_FEC_MODE_ENT       fecMode,
    IN  GT_BOOL                      powerUp
)
{
    GT_STATUS rc = GT_OK;

    CPSS_APP_PLATFORM_LOG_ENTRY_MAC();

    if (powerUp)
    {
        /* set port parameters */
#ifdef DXCH_CODE
        if (CPSS_IS_DXCH_FAMILY_MAC(PRV_CPSS_PP_MAC(devNum)->devFamily))
        {
            rc = cpssDxChSamplePortManagerMandatoryParamsSet(devNum, portNum, ifMode, speed, fecMode);
        }
#endif
#ifdef PX_CODE
        if (CPSS_IS_PX_FAMILY_MAC(PRV_CPSS_PP_MAC(devNum)->devFamily))
        {
            rc = cpssPxSamplePortManagerMandatoryParamsSet(devNum, portNum, ifMode, speed, fecMode);
        }
#endif
        if (rc != GT_OK)
        {
            CPSS_APP_PLATFORM_LOG_ERR_MAC("cpssDxChSamplePortManagerMandatoryParamsSet returned %d\n", rc);
            return rc;
        }

        /* start the port */
        rc = cpssAppPlatformPmPortCreate(devNum, portNum);
        if (rc != GT_OK)
        {
            CPSS_APP_PLATFORM_LOG_ERR_MAC("cpssAppPlatformPmPortCreate returned %d\n", rc);
            return rc;
        }
    }
    else
    {
        /* delete port */
        rc = cpssAppPlatformPmPortDelete(devNum, portNum);
        if (rc!=GT_OK)
        {
            CPSS_APP_PLATFORM_LOG_ERR_MAC("cpssAppPlatformPmPortDelete returned %d\n", rc);
            return rc;
        }
    }

    return GT_OK;
}

/**
* @internal cpssAppPlatformApPortInitSeqStart function
* @endinternal
*
* @brief   Bring-up AP port using port manager.
*
* @param[in] devNum                - device number
* @param[in] portNum               - port number
* @param[in] ifMode                - interface mode for the port
* @param[in] speed                 -  for the port
* @param[in] powerUp               - whether or not to power up the port
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssAppPlatformApPortInitSeqStart
(
    IN  GT_U8                        devNum,
    IN  GT_PHYSICAL_PORT_NUM         portNum,
    IN  CPSS_PORT_INTERFACE_MODE_ENT ifMode,
    IN  CPSS_PORT_SPEED_ENT          speed,
    IN  GT_BOOL                      powerUp
)
{
    GT_STATUS    rc = GT_NOT_APPLICABLE_DEVICE;

    CPSS_APP_PLATFORM_LOG_ENTRY_MAC();

    if (powerUp)
    {
#ifdef PX_CODE
        if (CPSS_IS_PX_FAMILY_MAC(PRV_CPSS_PP_MAC(devNum)->devFamily))
        {
            rc = cpssPxSamplePortManagerMandatoryParamsSet(devNum,portNum,ifMode,speed,CPSS_PORT_FEC_MODE_DISABLED_E);
        }
#endif
#ifdef DXCH_CODE
        if (CPSS_IS_DXCH_FAMILY_MAC(PRV_CPSS_PP_MAC(devNum)->devFamily))
        {
            rc = cpssDxChSamplePortManagerMandatoryParamsSet(devNum,portNum,ifMode,speed,CPSS_PORT_FEC_MODE_DISABLED_E);
        }
#endif

        if (rc!=GT_OK)
        {
            CPSS_APP_PLATFORM_LOG_ERR_MAC("cpssDxChSamplePortManagerApMandatoryParamsSet returned=%d, portNum=%d\n", rc, portNum);
            return rc;
        }

        /* start the port */
        rc = cpssAppPlatformPmPortCreate(devNum, portNum);
        if (rc!=GT_OK)
        {
            CPSS_APP_PLATFORM_LOG_ERR_MAC("cpssAppPlatformPmPortCreate returned=%d, portNum=%d\n", rc, portNum);
            return rc;
        }
    }
    else
    {
        /* delete port */
        rc = cpssAppPlatformPmPortDelete(devNum, portNum);
        if (rc!=GT_OK)
        {
            CPSS_APP_PLATFORM_LOG_ERR_MAC("cpssAppPlatformPmPortCreate returned=%d, portNum=%d\n", rc, portNum);
            return rc;
        }
    }

    return GT_OK;
}

/**
* @internal cpssAppPlatformIsPortMgrPort function
* @endinternal
*
* @brief   Check whether port is managed by port manager
*
* @param[in] devNum  - device number
* @param[in] portNum - port number
*
* @retval GT_TRUE    - If port is managed by port manager
* @retval GT_FALSE   - If port is not managed by port manager
*/
GT_BOOL cpssAppPlatformIsPortMgrPort
(
    IN  GT_U8                devNum
)
{
    GT_BOOL   enable;
    GT_STATUS    rc = GT_FAIL;
    if(0 == PRV_CPSS_IS_DEV_EXISTS_MAC(devNum))
    {
        /* device was removed */
        return GT_FALSE;
    }
    rc = cpssDxChPortManagerEnableGet(devNum, &enable);
    if (rc!=GT_OK)
    {
        CPSS_APP_PLATFORM_LOG_ERR_MAC("cpssDxChPortManagerEnableGet returned=%d\n", rc);
        return GT_FALSE;
    }

    return enable;
}

/**
* @internal cpssAppPlatformPmTaskDelete function
* @endinternal
*
* @brief   Delete port manager application demo task
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on passing null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssAppPlatformPmTaskDelete
(
    GT_VOID
)
{
    GT_STATUS rc;
    GT_U32   iterator=100;

    if(pmTid)
    {
        stopPmTask = GT_TRUE;

        while(iterator--)
        {
            if(GT_FALSE==stopPmTask)
            {
                break;
            }

            cpssOsTimerWkAfter(appRefPortManagerTaskSleepTime);
        }
        rc = cpssOsTaskDelete(pmTid);       /* task ID        */
        if (rc != GT_OK)
        {
            CPSS_APP_PLATFORM_LOG_ERR_MAC("cpssOsTaskDelete returned=%d\n", rc);
            return rc;
        }

        CPSS_APP_PLATFORM_LOG_INFO_MAC("portManagerTask task id %d (0x%x) is deleted. Graceful exit %d\n",pmTid,
            pmTid,0!=iterator);

        portMgr = GT_FALSE;
        stopPmTask = GT_FALSE;
        pmTid = 0;
    }

    return GT_OK;
}

