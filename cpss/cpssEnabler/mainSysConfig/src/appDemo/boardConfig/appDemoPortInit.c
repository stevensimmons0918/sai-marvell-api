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
* @file appDemoPortInit.c
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

#include <appDemo/boardConfig/appDemoBoardConfig.h>
#include <cpssDriver/pp/prvCpssDrvPpDefs.h>
#include <appDemo/sysHwConfig/appDemoDb.h>

#ifdef DXCH_CODE
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortManager.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortManagerSamples.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortDiag.h>
#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgEgrFlt.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfoEnhanced.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortAp.h>
#endif

#ifdef PX_CODE
#include <cpss/px/port/cpssPxPortManager.h>
#include <cpss/px/port/cpssPxPortManagerSamples.h>
#include <cpss/px/port/cpssPxPortCtrl.h>
#endif

#include <cpss/common/port/cpssPortManager.h>
#include <cpss/common/labServices/port/gop/port/mvHwsPortCfgIf.h>
#include <cpss/common/labServices/port/gop/port/serdes/avago/mvAvagoIf.h>
#include <cpss/common/port/private/prvCpssPortPcsCfg.h>
#include <gtOs/gtOsTimer.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>

static  GT_U32 appDemoPortMgrDebugLogEnable = 0;
static  GT_U32 tuneStartTaskId = 0;

GT_STATUS appDemoPortMgrDebugLogEnableSet(GT_U32 enable)
{
    appDemoPortMgrDebugLogEnable = enable;
    return GT_OK;
}


#define APP_DEMO_LOG(...)               \
    if(appDemoPortMgrDebugLogEnable)    \
    do{                                 \
        osPrintf("\n ** APP_DEMO:");    \
        osPrintf(__VA_ARGS__);          \
        osPrintf("** \n");              \
    }while (0)


#define APP_DEMO_LOG1(...)\
    do{                                          \
        osPrintf("\n APP_DEMO: "); \
        osPrintf(__VA_ARGS__);                   \
        osPrintf("\n");                          \
    }while (0)

static  GT_U32 appDemoPortMgrErrorLogEnable = 1;

typedef GT_STATUS (*APPDEMO_PORTMGR_LOW_LEVEL_FUNC_PTR)
(
    GT_U8                           devNum,
    GT_UOPT                         portNum,
    CPSS_PORT_MANAGER_STATUS_STC   *portMgrStatus,
    GT_BOOL                        *lowLevelDetectPtr
);

typedef struct
{
  APPDEMO_PORTMGR_LOW_LEVEL_FUNC_PTR       appDemoPrtMgrLowLevelFunc;

}APPDEMO_PORTMGR_FUNC_PTRS;

GT_STATUS appDemoPortMgrDefaultLowLevelFunc(
    GT_U8                           devNum,
    GT_UOPT                         portNum,
    CPSS_PORT_MANAGER_STATUS_STC   *portMgrStatus,
    GT_BOOL                         *lowLevelDetectPtr
)
{
    GT_STATUS rc;
    GT_BOOL signalDetect = GT_FALSE;

    portMgrStatus = portMgrStatus;
    *lowLevelDetectPtr = GT_FALSE;
#ifdef DXCH_CODE
    rc = cpssDxChPortSerdesSignalDetectGet(devNum,portNum,&signalDetect);
    if(rc != GT_OK)
    {
        return rc;
    }
#endif
#ifdef PX_CODE
    rc = cpssPxPortSerdesSignalDetectGet(devNum,portNum,&signalDetect);
    if(rc != GT_OK)
    {
        return rc;
    }
#endif
    *lowLevelDetectPtr = signalDetect;
    return GT_OK;
}

static APPDEMO_PORTMGR_FUNC_PTRS appDemoPortMgrFuncPtrs = {0};
static GT_STATUS appDemoPortMgrfuncInit(void)
{
    if (appDemoPortMgrFuncPtrs.appDemoPrtMgrLowLevelFunc == NULL)
        appDemoPortMgrFuncPtrs.appDemoPrtMgrLowLevelFunc = appDemoPortMgrDefaultLowLevelFunc;
    return GT_OK;
}

GT_STATUS appDemoPortMgrBindFunc(APPDEMO_PORTMGR_FUNC_PTRS *appDemoFuncPtr)
{
    if ( appDemoFuncPtr->appDemoPrtMgrLowLevelFunc != NULL )
    {
        appDemoPortMgrFuncPtrs.appDemoPrtMgrLowLevelFunc = appDemoFuncPtr->appDemoPrtMgrLowLevelFunc;
    }
    return GT_OK;
}

GT_STATUS appDemoPortMgrErrorLogEnableSet(GT_U32 enable)
{
    appDemoPortMgrErrorLogEnable = enable;
    return GT_OK;
}

#define APP_DEMO_LOG_ERROR(...) \
    if(appDemoPortMgrErrorLogEnable)        \
    do{                                     \
        osPrintf("\n APP_DEMO ERROR: ");    \
        osPrintf("[line %d]",__LINE__);     \
        osPrintf(__VA_ARGS__);              \
        osPrintf("\n");                     \
    }while (0)


/* whether or not multi-threaded usage should be performed. It is recommended to use
   the multi-threaded version, in which a dedicated task exist for the initialization
   of all ports in the system. This file contain examples for both usages */
/* NOTE: currently, port manager does not support the value of zero when managing
   loopbacked ports on a device, i.e when managing ports on a device where the ports
   are connected to each other.*/
static GT_U8 appDemoPortManagerMultiThreadedUsage =  1;
GT_BOOL appDemoAdaptiveCtleCreateDone = GT_FALSE;
GT_BOOL appDemoAdaptiveCtleTaskEnable = GT_FALSE;
GT_BOOL appDemoAdaptiveCtlePortUp[72] = {GT_FALSE};

extern GT_BOOL portMgr;

#define APP_DEMO_PORT_MANAGER_MAC_LOOPBACK_CHECK(portparams) (                                                                          \
                 (portParams.portType == CPSS_PORT_MANAGER_PORT_TYPE_REGULAR_E) &&                                                      \
                 (portParams.portParamsType.regPort.portAttributes.loopback.loopbackType == CPSS_PORT_MANAGER_LOOPBACK_TYPE_MAC_E) &&   \
                 (portParams.portParamsType.regPort.portAttributes.loopback.loopbackMode.macLoopbackEnable))                            \


#define APP_DEMO_PORT_MANAGER_ANALOG_TX2RX_LOOPBACK_CHECK(portparams) (                                                                  \
                 (portParams.portType == CPSS_PORT_MANAGER_PORT_TYPE_REGULAR_E) &&                                                       \
                 (portParams.portParamsType.regPort.portAttributes.loopback.loopbackType == CPSS_PORT_MANAGER_LOOPBACK_TYPE_SERDES_E) && \
                 (portParams.portParamsType.regPort.portAttributes.loopback.loopbackMode.serdesLoopbackMode ==                           \
                 CPSS_PORT_SERDES_LOOPBACK_ANALOG_TX2RX_E))                                                                              \


/*****************************************************************
                         Tasks semaphores
*****************************************************************/
CPSS_OS_SIG_SEM     portInitSeqSignalDetSem[PRV_CPSS_MAX_PP_DEVICES_CNS] = {0};


/*****************************************************************
                         Forward declarations
*****************************************************************/
unsigned __TASKCONV appDemoPortInitSeqSignalDetectedStage
(
        GT_VOID * param
);


unsigned __TASKCONV appDemoAdaptiveCtleBasedTemperature
(
        GT_VOID * param
);

GT_STATUS appDemoPortInitSequentialInit
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum
);

GT_STATUS appDemoPortInitPortParametersSet
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  CPSS_PORT_INTERFACE_MODE_ENT    ifMode,
    IN  CPSS_PORT_SPEED_ENT             speed
);

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

#ifdef DXCH_CODE
GT_STATUS cpssDxChPortPhysicalPortMapIsCpuGet
(
    IN GT_U8                       devNum,
    IN GT_PHYSICAL_PORT_NUM        portNum,
    OUT GT_BOOL                    *isCpuPtr
);

GT_STATUS cpssDxChPortPhysicalPortMapIsValidGet
(
    IN GT_U8                       devNum,
    IN GT_PHYSICAL_PORT_NUM        portNum,
    OUT GT_BOOL                    *isValidPtr
);
#endif
/**
* @internal appDemoPortInitSeqConfigureStage function
* @endinternal
*
* @brief   Create port using port manager.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on passing null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS appDemoPortInitSeqConfigureStage
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum
)
{
    GT_STATUS rc;
    CPSS_PORT_MANAGER_STC portEventStc;

    rc = GT_NOT_APPLICABLE_DEVICE;

    APP_DEMO_LOG("Inside first stage: Inside init stage");

    /****************************************
            Create Port
    *****************************************/
    portEventStc.portEvent = CPSS_PORT_MANAGER_EVENT_CREATE_E;
    APP_DEMO_LOG("calling the stage config with stage INIT");
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
        APP_DEMO_LOG_ERROR("calling PortManagerEventSet event %d returned=%d, portNum=%d", portEventStc.portEvent, rc, portNum);
        return rc;
    }

    return GT_OK;
}

/**
* @internal appDemoPortManagerTaskCreate function
* @endinternal
*
* @brief   create port manager application demo task
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] uniEvent                 - unified event raised on the port
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on passing null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS appDemoPortManagerTaskCreate
(
    IN GT_U8 devNum
)
{
    /* create handler task for Port Manager to treat all events from application */
    if (portMgr)
    {
        GT_STATUS rc;
        if(PRV_CPSS_SIP_6_CHECK_MAC(devNum) == GT_FALSE)
        {
            /* If handler was not made yet and we are in multi-threaded mode */
            if( (0 == portInitSeqSignalDetSem[devNum]) && appDemoPortManagerMultiThreadedUsage)
            {
                if(cpssOsSigSemBinCreate("portInitSeqSignalDetSem", CPSS_OS_SEMB_EMPTY_E, &portInitSeqSignalDetSem[devNum]) != GT_OK)
                {
                    APP_DEMO_LOG_ERROR("calling cpssOsSigSemBinCreate returned %d", GT_FAIL);
                    return GT_FAIL;
                }
            }
        }
        rc = cpssOsTaskCreate("portManagerTask",                     /* Task Name      */
                              500,                                   /* Task Priority  */
                              _64K,                                  /* Stack Size     */
                              appDemoPortInitSeqSignalDetectedStage, /* Starting Point */
                              (GT_VOID*)((GT_UINTPTR)devNum),        /* Arguments list */
                              &tuneStartTaskId);                     /* task ID        */
        if (rc != GT_OK)
        {
            APP_DEMO_LOG_ERROR("calling cpssOsTaskCreate returned=%d", rc);
            return rc;
        }
    }

    return GT_OK;
}

/**
* @internal appDemoPortManagerTaskReNumberDevNum function
* @endinternal
*
* @brief   changing the devNum to new number
*
* @param[in] newDevNum             - old device number
* @param[in] newDevNum             - new device number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on passing null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS appDemoPortManagerTaskReNumberDevNum
(
    IN GT_U8 oldDevNum,
    IN GT_U8 newDevNum
)
{
#ifdef DXCH_CODE
    if (portMgr)
    {
        GT_STATUS rc;

        /* Delete the old task */
        rc = cpssOsTaskDelete(tuneStartTaskId);
        if (rc != GT_OK)
        {
            APP_DEMO_LOG_ERROR("calling cpssOsTaskDelete returned=%d", rc);
            return rc;
        }

        /* Renumber to the new devNum */
        rc = cpssDxChCfgReNumberDevNum(oldDevNum,newDevNum);
        if (rc != GT_OK)
        {
            APP_DEMO_LOG_ERROR("calling cpssDxChCfgReNumberDevNum returned=%d", rc);
            return rc;
        }

        if(PRV_CPSS_SIP_6_CHECK_MAC(newDevNum) == GT_FALSE)
        {
            /* If handler was not made yet and we are in multi-threaded mode */
            if( (0 == portInitSeqSignalDetSem[newDevNum]) && appDemoPortManagerMultiThreadedUsage)
            {
                if(cpssOsSigSemBinCreate("portInitSeqSignalDetSem", CPSS_OS_SEMB_EMPTY_E, &portInitSeqSignalDetSem[newDevNum]) != GT_OK)
                {
                    APP_DEMO_LOG_ERROR("calling cpssOsSigSemBinCreate returned %d", GT_FAIL);
                    return GT_FAIL;
                }
            }
        }

        /* create new task with new devNum */
        rc = cpssOsTaskCreate("portManagerTask",                     /* Task Name      */
                              500,                                   /* Task Priority  */
                              _64K,                                  /* Stack Size     */
                              appDemoPortInitSeqSignalDetectedStage, /* Starting Point */
                              (GT_VOID*)((GT_UINTPTR)newDevNum),        /* Arguments list */
                              &tuneStartTaskId);                     /* task ID        */
        if (rc != GT_OK)
        {
            APP_DEMO_LOG_ERROR("calling cpssOsTaskCreate returned=%d", rc);
            return rc;
        }
    }
#else
    GT_UNUSED_PARAM(oldDevNum);
    GT_UNUSED_PARAM(newDevNum);
#endif
    return GT_OK;
}

/**
* @internal appDemoPortInitSeqPortStatusChangeSignal function
* @endinternal
*
* @brief   Handler for port events. This API will pass
*         CPSS_PORT_MANAGER_EVENT_LOW_LEVEL_STATUS_CHANGED_E (or
*         CPSS_PORT_MANAGER_EVENT_PORT_AP_HCD_FOUND_E) event to port manager
*         in order to notify it about a (possibly) change in the port status,
*         and then will initialize the port (if needed) using CPSS_PORT_MANAGER_EVENT_INIT_E
*         event (as long as the port is in CPSS_PORT_MANAGER_STATE_INIT_IN_PROGRESS_E state)
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] uniEvent                 - unified event raised on the port
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on passing null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS appDemoPortInitSeqPortStatusChangeSignal
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

    APP_DEMO_LOG("(port %d)uniEvent: %d",portNum,uniEvent);

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
        APP_DEMO_LOG_ERROR("calling PortManagerStatusGet returned=%d, portNum=%d", rc, portNum);
        return rc;
    }
    /* if port is in failure state, not passing the event to cpss, app need to furthur check
       this issue, possible bad connectivity.
       (Alternatively, app can decide to notify port manager with the event and then port will continue the flow).
       also if port is disabled not doing it. */
    if ( portConfigOutParams.portState == CPSS_PORT_MANAGER_STATE_FAILURE_E ||
        ((portConfigOutParams.portUnderOperDisable == GT_TRUE) && (!PRV_CPSS_SIP_6_CHECK_MAC(devNum)) && (!PRV_CPSS_DXCH_IS_AC3_BASED_DEVICE_MAC(devNum))))
    {
        APP_DEMO_LOG("port is in HW failure state or in disable so abort");
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
        APP_DEMO_LOG_ERROR("calling PortManagerEventSet event %d returned=%d, portNum=%d", portEventStc.portEvent, rc, portNum);
        return rc;
    }

    /************************************************************
        3. Signaling dedicated task if configured, or else,
        performing initialization operations on the main context
    ***********************************************************/
    if ((CPSS_PORT_MANAGER_EVENT_PORT_AP_HCD_FOUND_E != portEventStc.portEvent) &&
        (CPSS_PORT_MANAGER_EVENT_PORT_AP_DISABLE_E != portEventStc.portEvent) &&
        (CPSS_PORT_MANAGER_EVENT_PORT_AP_RESTART_E != portEventStc.portEvent) &&
        (CPSS_PORT_MANAGER_EVENT_PORT_AP_PARALLEL_DETECT_E != portEventStc.portEvent))
    {
        /* if we want to use the multi-threaded version (currently,
           only multi-threaded version is supported in port manager
           when configuring ports on the same device, i.e when a port
           and it's peer are on the same device)*/
        if (appDemoPortManagerMultiThreadedUsage)
        {
            APP_DEMO_LOG("signaling signal detect task");
            /* Signal the handler */
            if(PRV_CPSS_SIP_6_CHECK_MAC(devNum) == GT_FALSE)
            {
                rc = cpssOsSigSemSignal(portInitSeqSignalDetSem[devNum]);
            }
        }
        else /* currently not supported */
        {
            rc = appDemoPortInitSequentialInit(devNum, portNum);
        }
    }

    if(rc != GT_OK)
    {
        APP_DEMO_LOG_ERROR("calling cpssOsSigSemSignal returned=%d, portNum=%d", rc, portNum);
        return rc;
    }

    return GT_OK;
}

/**
* @internal appDemoPortInitSequentialInit function
* @endinternal
*
* @brief   Initialize the port (if needed) using CPSS_PORT_MANAGER_EVENT_INIT_E
*         event, as long as the port is in CPSS_PORT_MANAGER_STATE_INIT_IN_PROGRESS_E state.
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on passing null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note This API is an alternative implementation for appDemoPortInitSeqSignalDetectedStage
*       API, While appDemoPortInitSeqSignalDetectedStage act as a dedicated task for the
*       init process, it can be implemented straight forward as this API in the same context.
*
*/
GT_STATUS appDemoPortInitSequentialInit
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum
)
{
    GT_STATUS rc;
    CPSS_PORT_MANAGER_STATUS_STC portConfigOutParams;
    CPSS_PORT_MANAGER_STC        portEventStc;

    rc = GT_NOT_APPLICABLE_DEVICE;
    portConfigOutParams.portState = CPSS_PORT_MANAGER_STATE_RESET_E;

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
        APP_DEMO_LOG_ERROR("calling cpssDxChPortManagerStatusGet returned=%d, portNum=%d", rc, portNum);
    }

    portEventStc.portEvent = CPSS_PORT_MANAGER_EVENT_INIT_E;

    /* port will stay in CPSS_PORT_MANAGER_STATE_INIT_IN_PROGRESS_E for limited time
       so this 'while' loop on the main execution is safe to use */
    while (portConfigOutParams.portState == CPSS_PORT_MANAGER_STATE_INIT_IN_PROGRESS_E)
    {
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
            APP_DEMO_LOG_ERROR("calling PortManagerEventSet event %d returned=%d, portNum=%d", portEventStc.portEvent,rc, portNum);
        }
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
            APP_DEMO_LOG_ERROR("calling PortManagerEventGet returned=%d, portNum=%d", rc, portNum);
        }
    }

    return GT_OK;
}
#ifdef ASIC_SIMULATION
static GT_U32   appDemoPortManagerTaskSleepTime = 10;/*100 times in sec*/
#else
static GT_U32   appDemoPortManagerTaskSleepTime = 50;/*10 times in sec */
#endif
/* debug option to state the 'sleep' for the port manager ! between 'all devices' iterations */
GT_STATUS appDemoPortManagerTaskSleepTimeSet(IN GT_U32 timeInMilli)
{
    appDemoPortManagerTaskSleepTime = timeInMilli;
    return GT_OK;
}

/* debug option to disable the port manager ! */
/* since user can enter an infinite loop, need volatile to state that there's another thread running that also can change this variable */
static volatile GT_U32   appDemoPortManagerTaskEnable = 1;

GT_STATUS appDemoPortManagerTaskEnableSet(IN GT_U32 enable)
{
    appDemoPortManagerTaskEnable = enable;
    return GT_OK;
}

GT_STATUS appDemoPortManagerActivePortListGet
(
    GT_U8                devNum,
    GT_BOOL              isAlleyCat,
    GT_PHYSICAL_PORT_NUM portArr[],
    GT_U32               *listSize
)
{
    GT_STATUS                       rc = GT_OK;
    GT_PHYSICAL_PORT_NUM            portNum = 0;
    GT_U32                          maxPhysicalPort = 0;
    GT_U16                          maxMacPort =0, i,idx;
#ifdef DXCH_CODE
    CPSS_DXCH_DETAILED_PORT_MAP_STC portMapShadow;
    GT_BOOL                         portValidity = GT_FALSE;
    GT_BOOL                         MacValidity  = GT_FALSE;
#endif

    if (devNum >= PRV_CPSS_MAX_PP_DEVICES_CNS)
    {
        APP_DEMO_LOG_ERROR("appDemoPortManagerActivePortListGet: out of range devNum %d", devNum);
        return GT_BAD_PARAM;
    }

#ifdef PX_CODE
    GT_UNUSED_PARAM(isAlleyCat);
    if (CPSS_IS_PX_FAMILY_MAC(PRV_CPSS_PP_MAC(devNum)->devFamily))
    {
        /* optimize :  not need CPSS_MAX_PORTS_NUM_CNS */
        maxPhysicalPort = 16;
    }
#endif /*PX_CODE*/
#ifdef DXCH_CODE
    if (CPSS_IS_DXCH_FAMILY_MAC(PRV_CPSS_PP_MAC(devNum)->devFamily))
    {
         /* optimize :  not need CPSS_MAX_PORTS_NUM_CNS */
         /* in BC2 : 256                                */
         /* in Caelum,Aldrin,Aldrin2 : 128              */
         /* in BC3 : 256/512                            */
         /* in Falcon : 64/128/256/512/1024             */
        maxPhysicalPort = PRV_CPSS_DXCH_MAX_PHY_PORT_NUMBER_MAC(devNum);
    }
#endif

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    rc = cpssGetMaxPortNumber(devNum, &maxMacPort);
    if(rc != GT_OK)
    {
        APP_DEMO_LOG_ERROR("cpssGetMaxPortNumber: devNum %d returned with error %d", devNum, rc);
        CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    }
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    idx = 0;

    for (i = 0; i < maxMacPort; i++)
    {
#ifdef DXCH_CODE
        /*skip on invalid ports and "cpu sdma" and remote ports*/
        if ((CPSS_IS_DXCH_FAMILY_MAC(PRV_CPSS_PP_MAC(devNum)->devFamily)) && (isAlleyCat == GT_FALSE))
        {
            do
            {
                if(portNum == maxPhysicalPort)
                {
                    /* no need to continue to higher port numbers ! */
                    /* CPSS_MAX_PORTS_NUM_CNS : cpssDxChPortPhysicalPortDetailedMapGet(...) it will cause 'ERROR LOG' */
                    break;
                }
                rc = cpssDxChPortPhysicalPortDetailedMapGet(devNum, portNum, &portMapShadow);
                if(rc != GT_OK)
                {
                    if(portNum < maxPhysicalPort)
                    {
                        portNum++;
                        continue;
                    }
                    else
                    {
                        break;
                    }
                }
                portValidity = portMapShadow.valid;
                /* skip 'CPU SDMA' and 'remote' */
                MacValidity = (portMapShadow.portMap.mappingType != CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E) ? GT_FALSE/* no MAC */ : GT_TRUE/*MAC*/;

                if((portValidity == GT_FALSE) || (MacValidity == GT_FALSE))
                {
                    if(portNum < maxPhysicalPort)
                    {
                        portNum++;
                    }
                    else
                    {
                        break;
                    }
                }
            }
            while((portValidity == GT_FALSE) || (MacValidity == GT_FALSE));

            if(portNum == maxPhysicalPort)
            {
                continue;
            }
        }
        else if ( isAlleyCat == GT_TRUE )
        {
            if(PRV_CPSS_PP_MAC(devNum)->phyPortInfoArray[portNum].portType == PRV_CPSS_PORT_NOT_EXISTS_E)
            {
                portNum++;
                continue;
            }
        }
#endif
        if(portNum < maxPhysicalPort)
        {
            portArr[idx++] = portNum++;
        }
    }
    *listSize = idx;

    return GT_OK;
}

/*******************************************************************************
* appDemoPortInitSeqSignalDetectedStage
*
* DESCRIPTION:
*       Initialize the port (if needed) using CPSS_PORT_MANAGER_EVENT_INIT_E
*       event, as long as the port is in CPSS_PORT_MANAGER_STATE_INIT_IN_PROGRESS_E state.
*
* INPUTS:
*       param   - device number
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - on wrong port number or device
*       GT_HW_ERROR              - on hardware error
*       GT_BAD_PTR               - on passing null pointer
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       None.
*
*******************************************************************************/
unsigned __TASKCONV appDemoPortInitSeqSignalDetectedStage
(
        GT_VOID * param
)
{
    GT_PHYSICAL_PORT_NUM    portNum = 0;
    GT_U8                   devNum;
    GT_STATUS               rc, rc1;
    GT_BOOL                 portsExistFlag;
    CPSS_PORT_MANAGER_STATUS_STC   portConfigOutParams;
    CPSS_PORT_MANAGER_STC          portEventStc;
    GT_U16                  ii = 0;
    GT_BOOL                 isSip6 = GT_FALSE;
    GT_BOOL                 isAlleyCat = GT_FALSE;
    GT_BOOL                 pollingDevice = GT_FALSE;
    GT_PHYSICAL_PORT_NUM    portArr[CPSS_MAX_PORTS_NUM_CNS] = {0};
    GT_U32                  portListSize = 0;
#ifdef DXCH_CODE
    GT_BOOL                 loopback = GT_FALSE;
    GT_BOOL                 serdesLoopback = GT_FALSE;
    GT_BOOL                 signalDetect = GT_FALSE;
    GT_BOOL                 linkUp = GT_FALSE;
/*    GT_U32                  remainder = 0;*/
    CPSS_PM_PORT_PARAMS_STC portParams;
    /*CPSS_PM_PORT_PARAMS_STC portParamsTmp;*/
    CPSS_DXCH_PORT_AP_STATUS_STC apStatusStc;
#endif
#ifdef PX_CODE
     GT_UNUSED_PARAM(isAlleyCat);
     GT_UNUSED_PARAM(rc1);
#endif
    /* Variables initialization */
    devNum = (GT_U8)((GT_UINTPTR)param);
    appDemoPortMgrfuncInit();

    isSip6 = PRV_CPSS_SIP_6_CHECK_MAC(devNum);

    if ((PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT3_E) ||
        (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_AC5_E) ||
        (isSip6 == GT_TRUE))
    {
        pollingDevice = GT_TRUE;
    }

    if ((PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT3_E) ||
        (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_AC5_E))
    {
        isAlleyCat = GT_TRUE;
    }

    /* state that the task supports 'Graceful exit' */
    appDemoTaskSupportGracefulExit(NULL/*no CB needed*/,NULL/*no cookie needed*/);
    if(isSip6)
    {
        /* state that the task should not generate info to the LOG , because it
           is doing 'polling' */
        appDemoForbidCpssLogOnSelfSet(1);
        /* state that the task should not generate info to the 'Register trace' , because it
           is doing 'polling' */
        appDemoForbidCpssRegisterTraceOnSelfSet(1);
    }

    rc = appDemoPortManagerActivePortListGet(devNum, isAlleyCat, portArr, &portListSize);
    if(rc != GT_OK)
    {
        cpssOsPrintf("Error in getting the active port list \n");
        portListSize = 0;
    }
    rc = GT_NOT_APPLICABLE_DEVICE;
    /* overcome KlockWork warnings */
    portConfigOutParams.portState               = CPSS_PORT_MANAGER_STATE_RESET_E;
    portConfigOutParams.portUnderOperDisable    = GT_FALSE;
    portConfigOutParams.failure                 = CPSS_PORT_MANAGER_FAILURE_NONE_E;
    portConfigOutParams.ifMode                  = CPSS_PORT_INTERFACE_MODE_NA_E;
    portConfigOutParams.speed                   = CPSS_PORT_SPEED_NA_E;
    portConfigOutParams.fecType                 = CPSS_PORT_FEC_MODE_LAST_E;
    portConfigOutParams.remoteFaultConfig       = GT_FALSE;

    while(1)
    {
        if(pollingDevice == GT_FALSE)
        {
            cpssOsSigSemWait(portInitSeqSignalDetSem[devNum], 0);
        }

        if(portListSize == 0)
        {
            rc = appDemoPortManagerActivePortListGet(devNum, isAlleyCat, portArr, &portListSize);
            if(rc != GT_OK)
            {
                cpssOsPrintf("Error in getting the active port list \n");
                portListSize = 0;
            }
        }

        /* marking port-exist flag so iteration cycles will be performed at least once */

        portsExistFlag = GT_TRUE;

        while (portsExistFlag != GT_FALSE)
        {
            /* by here, the next 'for' loop is promised to run, so we can mark this flag with
               zero. The only case in which the 'while' will be iterated again is if there is existing port
               in INIT state (checked in the next scope), at which we want to continue the iterations */
            if(isSip6 == GT_FALSE)
            {
                /* for SIP6  the Semaphore is not used so the task need to run each polling interval */
                portsExistFlag = GT_FALSE;
            }

            /* check if task need termination */
            appDemoTaskCheckIfNeedTermination();

            for(ii = 0; ii < portListSize; ii++)
            {
                portNum = portArr[ii];
#ifdef DXCH_CODE
                rc = cpssDxChPortManagerPortParamsGet(devNum, portNum, &portParams);
                if ((rc != GT_OK) && (rc != GT_NOT_INITIALIZED))
                {
                    APP_DEMO_LOG_ERROR("portNum[%d]: cpssDxChPortManagerPortParamsGet: devNum %d returned with error %d", portNum, devNum, rc);
                    continue;
                }
#endif
            /************************************************************************
               1. Get port status
            **********************************************************************/
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

                /*  - GT_BAD_PARAM will be received upon bad portNum argument. We iterating
                all possible portNum values so in GT_BAD_PARAM case we will ignore this
                error code and continue execution, either way we won't enter the 2nd 'if'
                statement as bad portNum argument will not meet the statement condition */
                if((rc != GT_OK) || (portConfigOutParams.portState == CPSS_PORT_MANAGER_STATE_RESET_E) ||
                   ((portConfigOutParams.portUnderOperDisable == GT_TRUE) && (!PRV_CPSS_SIP_6_CHECK_MAC(devNum)) && (!PRV_CPSS_DXCH_IS_AC3_BASED_DEVICE_MAC(devNum))))
                {
                   continue;
                }

                /************************************************************************
                   2. If port state is INIT than continue to call INIT_EVENT
                **********************************************************************/
#ifdef DXCH_CODE
                if(pollingDevice == GT_TRUE)
                {
                    rc = cpssDxChPortLinkStatusGet(devNum, portNum, &linkUp);
                    if(rc != GT_OK)
                    {
                        APP_DEMO_LOG_ERROR("cpssDxChPortLinkStatusGet returned=%d, portNum=%d", rc, portNum);
                    }

                    if (( portParams.portType == CPSS_PORT_MANAGER_PORT_TYPE_802_3AP_E ) ||
                         ((portParams.portParamsType.regPort.portAttributes.validAttrsBitMask & CPSS_PM_PORT_ATTR_PM_OVER_FW_E) && (portParams.portParamsType.regPort.portAttributes.pmOverFw)) )
                    {
                        if(((portConfigOutParams.portState == CPSS_PORT_MANAGER_STATE_LINK_DOWN_E) || (portConfigOutParams.portState == CPSS_PORT_MANAGER_STATE_MAC_LINK_DOWN_E)) &&
                           (linkUp == GT_TRUE))
                        {
                            rc = cpssDxChPortApPortStatusGet(devNum, portNum, &apStatusStc);
                            if(rc != GT_OK)
                            {
                                APP_DEMO_LOG_ERROR("calling cpssDxChPortApPortStatusGet returned=%d, portNum=%d", rc, portNum);
                            }
                            if ( apStatusStc.hcdFound )
                            {
                                portEventStc.portEvent = CPSS_PORT_MANAGER_EVENT_MAC_LEVEL_STATUS_CHANGED_E;
                                rc = cpssDxChPortManagerEventSet(devNum, portNum, &portEventStc);
                                if(rc != GT_OK)
                                {
                                    APP_DEMO_LOG_ERROR("calling PortManagerEventSet event %d returned=%d, portNum=%d", portEventStc.portEvent, rc, portNum);
                                }
                            }
                        }
#if 0
                        if((portConfigOutParams.portState == CPSS_PORT_MANAGER_STATE_LINK_UP_E) &&
                           (linkUp == GT_FALSE))
                        {
                            rc = cpssDxChPortApPortStatusGet(devNum, portNum, &apStatusStc);
                            if(rc != GT_OK)
                            {
                                APP_DEMO_LOG_ERROR("calling cpssDxChPortApPortStatusGet returned=%d, portNum=%d", rc, portNum);
                            }
                            portEventStc.portEvent = CPSS_PORT_MANAGER_EVENT_MAC_LEVEL_STATUS_CHANGED_E;
                            rc = cpssDxChPortManagerEventSet(devNum, portNum, &portEventStc);
                            if(rc != GT_OK)
                            {
                                APP_DEMO_LOG_ERROR("calling PortManagerEventSet event %d returned=%d, portNum=%d", portEventStc.portEvent, rc, portNum);
                            }
                        }
#endif
                        continue;
                    }
                    if(linkUp == GT_TRUE && (portConfigOutParams.portState == CPSS_PORT_MANAGER_STATE_MAC_LINK_DOWN_E))
                    {
                        portEventStc.portEvent = CPSS_PORT_MANAGER_EVENT_MAC_LEVEL_STATUS_CHANGED_E;
                        rc = cpssDxChPortManagerEventSet(devNum, portNum, &portEventStc);
                        if(rc != GT_OK)
                        {
                            APP_DEMO_LOG_ERROR("calling PortManagerEventSet event %d returned=%d, portNum=%d", portEventStc.portEvent, rc, portNum);
                        }
                    }
                    loopback = APP_DEMO_PORT_MANAGER_MAC_LOOPBACK_CHECK(portParams);
                    if(!loopback)
                    {
                        if(portConfigOutParams.portState == CPSS_PORT_MANAGER_STATE_LINK_DOWN_E)
                        {
                            if (appDemoPortMgrFuncPtrs.appDemoPrtMgrLowLevelFunc != NULL)
                            {
                                rc = appDemoPortMgrFuncPtrs.appDemoPrtMgrLowLevelFunc(devNum, portNum, &portConfigOutParams, &signalDetect);
                                if(rc != GT_OK)
                                {
                                   /* some of the errors in signal detect get can be because we checking the port manager status and is on link up/down and then immediately the port deleted
                                   and the app demo stay on link up/down status and not reset until next iteration.
                                   This is not error but in order to ingore flase return errors, check the status again and if in reset, stop the loop for this port until next create */
                                    rc1 = cpssDxChPortManagerStatusGet(devNum, portNum, &portConfigOutParams);
                                    if(rc1 != GT_OK)
                                    {
                                        APP_DEMO_LOG_ERROR("calling cpssDxChPortManagerStatusGet returned=%d, portNum=%d", rc1, portNum);
                                    }

                                    if(portConfigOutParams.portState == CPSS_PORT_MANAGER_STATE_RESET_E)
                                    {
                                        continue;
                                    }
                                    APP_DEMO_LOG_ERROR("calling cpssDxChPortSerdesSignalDetectGet returned=%d, portNum=%d", rc, portNum);
                                }
                                if(signalDetect)
                                {
                                    portEventStc.portEvent = CPSS_PORT_MANAGER_EVENT_LOW_LEVEL_STATUS_CHANGED_E;

                                    rc = cpssDxChPortManagerEventSet(devNum, portNum, &portEventStc);
                                    if(rc != GT_OK)
                                    {
                                        APP_DEMO_LOG_ERROR("calling PortManagerEventSet event %d returned=%d, portNum=%d", portEventStc.portEvent, rc, portNum);
                                    }
                                }
                            }
                        }

                        serdesLoopback = APP_DEMO_PORT_MANAGER_ANALOG_TX2RX_LOOPBACK_CHECK(portParams);
                        if((portConfigOutParams.portState == CPSS_PORT_MANAGER_STATE_MAC_LINK_DOWN_E) && serdesLoopback)
                        {
                            if (appDemoPortMgrFuncPtrs.appDemoPrtMgrLowLevelFunc != NULL)
                            {
                                rc = appDemoPortMgrFuncPtrs.appDemoPrtMgrLowLevelFunc(devNum, portNum, &portConfigOutParams, &signalDetect);
                                if(rc != GT_OK)
                                {
                                    APP_DEMO_LOG_ERROR("calling cpssDxChPortSerdesSignalDetectGet returned=%d, portNum=%d", rc, portNum);
                                }
                                if(signalDetect)
                                {
                                    portEventStc.portEvent = CPSS_PORT_MANAGER_EVENT_LOW_LEVEL_STATUS_CHANGED_E;

                                    rc = cpssDxChPortManagerEventSet(devNum, portNum, &portEventStc);
                                    if(rc != GT_OK)
                                    {
                                        APP_DEMO_LOG_ERROR("calling PortManagerEventSet event %d returned=%d, portNum=%d", portEventStc.portEvent, rc, portNum);
                                    }
                                }
                            }
                        }
                        if((!CPSS_PORT_IF_MODE_QSGMII_USX_QUSGMII_CHECK_MAC(portConfigOutParams.ifMode)) && (portConfigOutParams.portState == CPSS_PORT_MANAGER_STATE_LINK_UP_E) && !(APP_DEMO_PORT_MANAGER_ANALOG_TX2RX_LOOPBACK_CHECK(portparams)))
                        {
                            if (isSip6)
                            {
                                GT_BOOL alignLockStatus;
                                GT_U32 portMacNum;
                                PRV_CPSS_DXCH_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);
                                /* read align lock*/
                                rc = prvCpssPortPcsAlignLockStatusGet(devNum, portNum, portMacNum, &alignLockStatus);
                                if(rc != GT_OK)
                                {
                                    APP_DEMO_LOG_ERROR("calling prvCpssPortPcsAlignLockStatusGet returned=%d, portNum=%d", rc, portNum);
                                }
                                if ( !alignLockStatus )
                                {
                                    portEventStc.portEvent = CPSS_PORT_MANAGER_EVENT_LOW_LEVEL_STATUS_CHANGED_E;
                                    rc = cpssDxChPortManagerEventSet(devNum, portNum, &portEventStc);
                                    if(rc != GT_OK)
                                    {
                                       APP_DEMO_LOG_ERROR("calling PortManagerEventSet event %d returned=%d, portNum=%d", portEventStc.portEvent, rc, portNum);
                                    }
                                }
                            }
                            else
                            {
                                if (appDemoPortMgrFuncPtrs.appDemoPrtMgrLowLevelFunc != NULL)
                                {
                                    rc = appDemoPortMgrFuncPtrs.appDemoPrtMgrLowLevelFunc(devNum, portNum, &portConfigOutParams, &signalDetect);
                                    if(rc != GT_OK)
                                    {
                                        APP_DEMO_LOG_ERROR("calling cpssDxChPortSerdesSignalDetectGet returned=%d, portNum=%d", rc, portNum);
                                    }
                                    if(!signalDetect)
                                    {
                                        portEventStc.portEvent = CPSS_PORT_MANAGER_EVENT_LOW_LEVEL_STATUS_CHANGED_E;

                                        rc = cpssDxChPortManagerEventSet(devNum, portNum, &portEventStc);
                                        if(rc != GT_OK)
                                        {
                                            APP_DEMO_LOG_ERROR("calling PortManagerEventSet event %d returned=%d, portNum=%d", portEventStc.portEvent, rc, portNum);
                                        }
                                    }
                                }
                            }

                        }

                    }
                }
#endif
                if (portConfigOutParams.portState == CPSS_PORT_MANAGER_STATE_INIT_IN_PROGRESS_E)
                {
                    APP_DEMO_LOG("calling to launching whole INIT on port");
                    /* marking that there is a port in INIT state so iteration cycles will continue */
                    portsExistFlag = GT_TRUE;
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
                        APP_DEMO_LOG_ERROR("calling PortManagerEventSet event %d returned=%d, portNum=%d", portEventStc.portEvent, rc, portNum);
                    }
                }
                if (portNum % 32 == 31)
                {
                    cpssOsTimerWkAfter(appDemoPortManagerTaskSleepTime/8);
                }
            }

            cpssOsTimerWkAfter(appDemoPortManagerTaskSleepTime);
            if (appDemoPortManagerTaskEnable == 0)
            {
                /* debug option to disable the port manager ! */
                do{
                    /* wait for 're-enabling' of the task */
                    cpssOsTimerWkAfter(100);
                }
                while(appDemoPortManagerTaskEnable == 0);
            }
        } /* while (portsExistFlag != GT_FALSE) */
    } /* while (1) */

    return GT_OK;
}

/**
* @internal appDemoPortInitSeqEnablePortStage function
* @endinternal
*
* @brief   Enable the port. This API is the complement of appDemoPortInitSeqDisablePortStage
*         API, while the later disabled the port, this API will enable it back.
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on passing null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS appDemoPortInitSeqEnablePortStage
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum
)
{
    GT_STATUS rc;
    CPSS_PORT_MANAGER_STC portEventStc;

    rc = GT_NOT_APPLICABLE_DEVICE;

    APP_DEMO_LOG("Enabling port bitmaps");
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
        APP_DEMO_LOG_ERROR("calling PortManagerEventSet event %d returned=%d, portNum=%d", portEventStc.portEvent, rc, portNum);
        return rc;
    }

    return GT_OK;
}

/**
* @internal appDemoPortInitSeqLinkStatusChangedStage function
* @endinternal
*
* @brief   Handler function for link status change event. This API will pass
*         the event to the port manager and perform actions if needed.
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on passing null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS appDemoPortInitSeqLinkStatusChangedStage
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum
)
{
    GT_STATUS                    rc = GT_NOT_APPLICABLE_DEVICE;
    CPSS_PORT_MANAGER_STC        portEventStc;
    CPSS_PORT_MANAGER_STATUS_STC portStage;
    CPSS_PM_PORT_PARAMS_STC      portParams;
    GT_BOOL                      linkUp        = GT_FALSE;
    GT_BOOL                      linkStatusPM  = GT_FALSE; /* the link status accroding to the pm data base*/


    /*******************************
          1. Notify port manager
    *******************************/
    APP_DEMO_LOG("calling to launching whole tune on ports");
    /* reduce KW warnings */
    portParams.portType = CPSS_PORT_MANAGER_PORT_TYPE_REGULAR_E;
    portStage.portState = CPSS_PORT_MANAGER_STATE_RESET_E;

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
        APP_DEMO_LOG_ERROR("calling PortManagerStatusGet returned=%d, portNum=%d", rc, portNum);
        return rc;
    }

    if (portStage.portState == CPSS_PORT_MANAGER_STATE_FAILURE_E ||
        ((portStage.portUnderOperDisable == GT_TRUE) && (!PRV_CPSS_SIP_6_CHECK_MAC(devNum)) && (!PRV_CPSS_DXCH_IS_AC3_BASED_DEVICE_MAC(devNum))))
    {
        APP_DEMO_LOG("port is in HW failure state or in disable so abort mac_level_change notification");
        return GT_OK;
    }
    if (portStage.portState == CPSS_PORT_MANAGER_STATE_RESET_E )
    {
        APP_DEMO_LOG("port is in reset state so abort mac_level_change notification");
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
            APP_DEMO_LOG_ERROR("cpssDxChPortLinkStatusGet returned=%d, portNum=%d", rc, portNum);
        }
#endif
#ifdef PX_CODE
        rc = cpssPxPortLinkStatusGet(devNum, portNum, &linkUp);
        if(rc != GT_OK)
        {
            APP_DEMO_LOG_ERROR("cpssPxPortLinkStatusGet returned=%d, portNum=%d", rc, portNum);
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
        APP_DEMO_LOG_ERROR("calling PortManagerEventSet event %d returned=%d, portNum=%d", portEventStc.portEvent, rc, portNum);
        return rc;
    }

    return GT_OK;
}

/**
* @internal appDemoPortInitSeqDisablePortStage function
* @endinternal
*
* @brief   Disable the port. This API is the complement of appDemoPortInitSeqEnablePortStage
*         API, while the later enables the port, this API will disable it.
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on passing null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS appDemoPortInitSeqDisablePortStage
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum
)
{
    GT_STATUS rc;
    CPSS_PORT_MANAGER_STC portEventStc;

    rc = GT_NOT_APPLICABLE_DEVICE;

    /*******************************
            Disable Port
    *******************************/
    APP_DEMO_LOG("disabling port start");
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
        APP_DEMO_LOG_ERROR("calling PortManagerEventSet with PORT_STAGE_DISABLE_E ended-bad and returned=%d, portNum=%d", rc, portNum);
        return rc;
    }

    APP_DEMO_LOG("disabling port end");

    return GT_OK;
}

/**
* @internal appDemoPortInitSeqDeletePortStage function
* @endinternal
*
* @brief   Delete port.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on passing null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS appDemoPortInitSeqDeletePortStage
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum
)
{
    GT_STATUS rc;
    CPSS_PORT_MANAGER_STC portEventStc;

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
        APP_DEMO_LOG_ERROR("calling PortManagerEventSet event %d returned=%d, portNum=%d", portEventStc.portEvent, rc, portNum);
        return rc;
    }

    return GT_OK;
}

/* ##############################################
   #############App Demo commands for port manager ##########
   ############################################## */

/**
* @internal appDemoApPortInitSeqStart function
* @endinternal
*
* @brief   Bring-up AP port using port manager.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] ifMode                   - interface mode for the port
* @param[in] speed                    -  for the port
* @param[in] powerUp                  - whether or not to power up the port
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on passing null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS appDemoApPortInitSeqStart
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  CPSS_PORT_INTERFACE_MODE_ENT    ifMode,
    IN  CPSS_PORT_SPEED_ENT             speed,
    IN  GT_BOOL                         powerUp
)
{
    GT_STATUS    rc = GT_NOT_APPLICABLE_DEVICE;

    APP_DEMO_LOG("Starting: inside appDemoPortInitSeqStart");

    if (powerUp)
    {
#ifdef PX_CODE
        if (CPSS_IS_PX_FAMILY_MAC(PRV_CPSS_PP_MAC(devNum)->devFamily))
        {
            rc = cpssPxSamplePortManagerApMandatoryParamsSet(devNum,portNum,ifMode,speed,CPSS_PORT_FEC_MODE_DISABLED_E, CPSS_PORT_FEC_MODE_DISABLED_E);
        }
#endif
#ifdef DXCH_CODE
        if (CPSS_IS_DXCH_FAMILY_MAC(PRV_CPSS_PP_MAC(devNum)->devFamily))
        {
            rc = cpssDxChSamplePortManagerApMandatoryParamsSet(devNum,portNum,ifMode,speed,CPSS_PORT_FEC_MODE_DISABLED_E, CPSS_PORT_FEC_MODE_DISABLED_E);
        }
#endif

        if (rc!=GT_OK)
        {
            APP_DEMO_LOG_ERROR("calling appDemoPortInitPortParametersSet returned=%d, portNum=%d", rc, portNum);
            return rc;
        }

        /* start the port */
        rc = appDemoPortInitSeqConfigureStage(devNum, portNum);
        if (rc!=GT_OK)
        {
            APP_DEMO_LOG_ERROR("calling appDemoPortInitSeqConfigureStage returned=%d, portNum=%d", rc, portNum);
            return rc;
        }
    }
    else
    {
        /* delete port */
        rc = appDemoPortInitSeqDeletePortStage(devNum, portNum);
        if (rc!=GT_OK)
        {
            APP_DEMO_LOG_ERROR("calling appDemoPortInitSeqConfigureStage returned=%d, portNum=%d", rc, portNum);
            return rc;
        }
    }

    return GT_OK;
}

/**
* @internal appDemoPortInitPortParametersGet function
* @endinternal
*
* @brief   Get port parameters and print them.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on passing null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/

GT_STATUS appDemoPortInitPortParametersGet
(
    IN  GT_U8                  devNum,
    IN  GT_PHYSICAL_PORT_NUM   portNum
)
{
    GT_U32                                portMacNum     = 0;
    MV_HWS_PORT_STANDARD                  portMode;
    MV_HWS_PORT_INIT_PARAMS               curPortParams;
    GT_U32                                bm;
    GT_U32                                lanesArrIdx;
    CPSS_PM_PORT_ATTR_STC                *portAttr;
    CPSS_PM_PORT_PARAMS_STC               portParams;
    GT_STATUS                             rc             = GT_NOT_APPLICABLE_DEVICE;
    CPSS_PM_MODE_LANE_PARAM_STC          *laneParams     = NULL;
    CPSS_PORT_MANAGER_CONFIDENCE_DATA_CONFIG_STC    *confiData      = NULL;

    APP_DEMO_LOG("Starting: inside appDemoPmPortParamsGet");

#ifdef PX_CODE
    if (CPSS_IS_PX_FAMILY_MAC(PRV_CPSS_PP_MAC(devNum)->devFamily))
    {
        rc = cpssPxPortManagerPortParamsStructInit(CPSS_PORT_MANAGER_PORT_TYPE_LAST_E, &portParams);
    }
#endif
#ifdef DXCH_CODE
    if (CPSS_IS_DXCH_FAMILY_MAC(PRV_CPSS_PP_MAC(devNum)->devFamily))
    {
        rc = cpssDxChPortManagerPortParamsStructInit(CPSS_PORT_MANAGER_PORT_TYPE_LAST_E, &portParams);
    }
#endif
  if (rc != GT_OK)
  {
    APP_DEMO_LOG_ERROR("calling internal_cpssPortManagerInitParamsStruct returned %d", rc);
    return rc;
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

    if (rc != GT_OK)
    {
        APP_DEMO_LOG_ERROR("calling internal_cpssPortManagerPortParamsGet returned %d", rc);
        return rc;
    }

    /* prepering data structures for parameters get according to the port type */
    if (portParams.portType != CPSS_PORT_MANAGER_PORT_TYPE_REGULAR_E)
    {
        APP_DEMO_LOG_ERROR("New PortManager APIs currently supports regular ports alone");
        return rc;
    }

    /* ifMode and speed for non-AP port */
    if (portParams.portType == CPSS_PORT_MANAGER_PORT_TYPE_REGULAR_E)
    {
        APP_DEMO_LOG1("ifMode=%d, speed=%d",
                      portParams.portParamsType.regPort.ifMode, portParams.portParamsType.regPort.speed);

        portAttr    = &(portParams.portParamsType.regPort.portAttributes);
        bm          = portAttr->validAttrsBitMask;

        if ( bm & CPSS_PM_ATTR_TRAIN_MODE_E )
        {
            APP_DEMO_LOG1("trainMode=%d", portAttr->trainMode);
        }

        if ( bm & CPSS_PM_ATTR_RX_TRAIN_SUPP_E )
        {
            APP_DEMO_LOG1("adaptiveRxTrainSupported=%d", portAttr->adaptRxTrainSupp);
        }

        if ( bm & CPSS_PM_ATTR_EDGE_DETECT_SUPP_E )
        {
            APP_DEMO_LOG1("edgeDetectSupported=%d", portAttr->edgeDetectSupported);
        }

        if ( bm & CPSS_PM_ATTR_LOOPBACK_E )
        {
            APP_DEMO_LOG1("loopbackType=%d", portAttr->loopback.loopbackType);

            if (portAttr->loopback.loopbackType ==
                CPSS_PORT_MANAGER_LOOPBACK_TYPE_MAC_E)
            {
                APP_DEMO_LOG1("macLoopbackModeEnabled=%d",
                              portAttr->loopback.loopbackMode.macLoopbackEnable);
            }
            else if (portAttr->loopback.loopbackType ==
                     CPSS_PORT_MANAGER_LOOPBACK_TYPE_SERDES_E)
            {
                APP_DEMO_LOG1("serdesLoopbackMode=%d",
                              portAttr->loopback.loopbackMode.serdesLoopbackMode);
            }
            else if (portAttr->loopback.loopbackType ==
                     CPSS_PORT_MANAGER_LOOPBACK_TYPE_LAST_E)
            {
                APP_DEMO_LOG1("NO Loopback Mode Configured");
            }
            else
            {
                APP_DEMO_LOG1("INVALID loopback type=%d",
                              portAttr->loopback.loopbackType);
            }
        }

        if ( bm & CPSS_PM_ATTR_ET_OVERRIDE_E )
        {
            APP_DEMO_LOG1("Enhance Tune is Active. minLF=%d, maxLF=%d",
                          portAttr->etOverride.minLF, portAttr->etOverride.maxLF);
        }

        if ( bm & CPSS_PM_ATTR_FEC_MODE_E )
        {
            APP_DEMO_LOG1("FEC mode is %d", portAttr->fecMode);
        }

        if ( bm & CPSS_PM_ATTR_CALIBRATION_E )
        {
            APP_DEMO_LOG1("Enhance calibration mode is %d", portAttr->calibrationMode.calibrationType);
            if (portAttr->calibrationMode.calibrationType == CPSS_PORT_MANAGER_OPTICAL_CALIBRATION_TYPE_E)
            {
                confiData = &(portAttr->calibrationMode.confidenceCfg);
                APP_DEMO_LOG1("CI - lfMin = %d, lfMax=%d, hfMin=%d, hfMax=%d, eoMin=%d, eoMax=%d bitmap=0x%08x",
                              confiData->lfMinThreshold, confiData->lfMaxThreshold, confiData->hfMinThreshold,
                              confiData->hfMaxThreshold, confiData->eoMinThreshold, confiData->eoMaxThreshold,
                              confiData->confidenceEnableBitMap);
            }
        }

        if ( bm & CPSS_PM_ATTR_UNMASK_EV_MODE_E )
        {
            APP_DEMO_LOG1("Enhance Unmask Event mode is %d", portAttr->unMaskEventsMode);
        }

        if ( bm & CPSS_PM_ATTR_BW_MODE_E )
        {
            APP_DEMO_LOG1("BW mode is %d", portAttr->bwMode);
        }
    }

    rc = PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppCheckAndGetMacFunc(devNum, portNum, &portMacNum);
    if (rc != GT_OK)
    {
        APP_DEMO_LOG1( "Port %d ppCheckAndGetMacFunc failed=%d", portNum, rc);
        return rc;
    }

    rc = prvCpssCommonPortIfModeToHwsTranslate(devNum, portParams.portParamsType.regPort.ifMode,
                                               portParams.portParamsType.regPort.speed, &portMode);
    if (rc != GT_OK)
    {
        APP_DEMO_LOG1(
                     "prvCpssCommonPortIfModeToHwsTranslate returned rc %d on ifMode %d and speed %d",
                     rc, portParams.portParamsType.regPort.ifMode, portParams.portParamsType.regPort.speed);
        return rc;
    }

    rc = hwsPortModeParamsGetToBuffer(CAST_SW_DEVNUM(devNum), 0, portMacNum, portMode, &curPortParams);
    if (rc != GT_OK)
    {
        APP_DEMO_LOG1("hwsPortModeParamsGetToBuffer returned null");
        return rc;
    }

    /* Lane Params */
    for (lanesArrIdx = 0; lanesArrIdx < curPortParams.numOfActLanes; lanesArrIdx++)
    {
        laneParams      = &(portParams.portParamsType.regPort.laneParams[lanesArrIdx]);
        bm              = laneParams->validLaneParamsBitMask;

        if ( bm & CPSS_PM_LANE_PARAM_RX_E )
        {
            if (laneParams->rxParams.type == CPSS_PORT_SERDES_AVAGO_E)
            {
                APP_DEMO_LOG1(
                             "[Lane:%d] RX params BC %d, DC %d,"
                             "LF %d, HF %d, sqlch %d",
                             lanesArrIdx, laneParams->rxParams.rxTune.avago.BW, laneParams->rxParams.rxTune.avago.DC,
                             laneParams->rxParams.rxTune.avago.LF,
                             laneParams->rxParams.rxTune.avago.HF,
                             laneParams->rxParams.rxTune.avago.sqlch
                             );

            }
            else if (laneParams->rxParams.type == CPSS_PORT_SERDES_COMPHY_H_E)
            {
                APP_DEMO_LOG1(
                             "[Lane:%d] RX params bandWidth %d, dcGain %d,"
                             "ffeCap %d, ffeRes %d, sqlch %d",
                             lanesArrIdx, laneParams->rxParams.rxTune.comphy.bandWidth,
                             laneParams->rxParams.rxTune.comphy.dcGain,
                             laneParams->rxParams.rxTune.comphy.ffeC,
                             laneParams->rxParams.rxTune.comphy.ffeR,
                             laneParams->rxParams.rxTune.comphy.sqlch
                             );

            }
        }

        if ( bm & CPSS_PM_LANE_PARAM_TX_E )
        {
            if (laneParams->txParams.type == CPSS_PORT_SERDES_AVAGO_E)
            {
                APP_DEMO_LOG1(
                             "[Lane:%d] TX params Atten %d, "
                             "post %d, pre %d, pre2 %d, pre3 %d ",
                             lanesArrIdx, laneParams->txParams.txTune.avago.atten, laneParams->txParams.txTune.avago.post, laneParams->txParams.txTune.avago.pre,
                             laneParams->txParams.txTune.avago.pre2, laneParams->txParams.txTune.avago.pre3
                             );

            }
            else if (laneParams->txParams.type == CPSS_PORT_SERDES_COMPHY_H_E)
            {
                APP_DEMO_LOG1(
                             "[Lane:%d] TX params txAmp %d, "
                             "emph0 %d, emph1 %d ",
                             lanesArrIdx, laneParams->txParams.txTune.comphy.txAmp,
                             laneParams->txParams.txTune.comphy.emph0,
                             laneParams->txParams.txTune.comphy.emph1
                             );

            }
        }

    }
    return GT_OK;
}


GT_STATUS appDemoAdaptiveCtleBasedTemperatureTaskEnable
(
    IN  GT_U8                   devNum,
    IN  GT_BOOL                 enable
)
{
    GT_UNUSED_PARAM(devNum);

    if (enable == GT_TRUE)
    {
        appDemoAdaptiveCtleTaskEnable = GT_TRUE;
    }
    else
    {
        appDemoAdaptiveCtleTaskEnable = GT_FALSE;
    }

    return GT_OK;
}
/*debug function only!!!*/
#ifdef RUN_ADAPTIVE_CTLE
GT_STATUS appDemoAdaptiveCtleBasedTemperatureCreateTask
(
    IN  GT_U8                   devNum
)
{
    GT_STATUS                       rc;

    /********************************************
    Creating handling task if not created yet.
    *******************************************/
    if (appDemoAdaptiveCtleCreateDone == GT_FALSE)
    {
        GT_U32 adaptiveCtleStartTaskId;

        rc = cpssOsTaskCreate("adaptiveCtleTask",    /* Task Name      */
                              500,                  /* Task Priority  */
                              _64K,                 /* Stack Size     */
                              appDemoAdaptiveCtleBasedTemperature, /* Starting Point */
                              (GT_VOID*)((GT_UINTPTR)devNum),        /* Arguments list */
                              &adaptiveCtleStartTaskId);             /* task ID        */
        if (rc != GT_OK)
        {
            APP_DEMO_LOG_ERROR("calling cpssOsTaskCreate returned=%d", rc);
            return rc;
        }

        appDemoAdaptiveCtleCreateDone = GT_TRUE;

        mvHwsAvagoAdaptiveCtleSerdesesInitDb(devNum);
    }
    return GT_OK;
}

GT_STATUS appDemoAdaptiveCtleBasedTemperatureIsTaskCreateGet
(
    IN  GT_U8                   devNum
)
{
    return appDemoAdaptiveCtleCreateDone;
}

/*******************************************************************************
* appDemoAdaptiveCtleBasedTemperature
*
* DESCRIPTION:
*      Loop on all ports and run adaptive ctle base temperture for each port
*
* INPUTS:
*       param   - device number
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - on wrong port number or device
*       GT_HW_ERROR              - on hardware error
*       GT_BAD_PTR               - on passing null pointer
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       None.
*
*******************************************************************************/
unsigned __TASKCONV appDemoAdaptiveCtleBasedTemperature
(
        GT_VOID * param
)
{
    GT_PHYSICAL_PORT_NUM    portNum;
    GT_U8                   devNum;
    GT_STATUS               rc;
    GT_U32 bitMapIndex;
    GT_BOOL portAdaptiveCtleEnable;
    GT_U32  delay;
    GT_U32 phase;
    GT_U32 ii;

    /* Variables initialization */
    devNum = (GT_U8)((GT_UINTPTR)param);

    rc = GT_NOT_APPLICABLE_DEVICE;
    delay = 1000;
    while(1)
    {
        if (appDemoAdaptiveCtleTaskEnable == GT_TRUE)
        {
            phase = 1;
            for (portNum = 0; portNum < MV_HWS_MAX_PORT_NUM; portNum++) /* phy ports*/
            {
                /************************************************************************
                 Phase 1. call port adaptive ctle, write new parameters
                **********************************************************************/

                /*check adaptive CTLE enable*/
                bitMapIndex = portNum / HWS_MAX_ADAPT_CTLE_DB_SIZE;
                portAdaptiveCtleEnable =
                    (( hwsDeviceSpecInfo[devNum].adaptiveCtleDb.bitMapEnableCtleCalibrationBasedTemperture[bitMapIndex] &(0x1 << (portNum%HWS_MAX_ADAPT_CTLE_DB_SIZE))) > 0 ? GT_TRUE : GT_FALSE);
                if (portAdaptiveCtleEnable == GT_TRUE)
                {
                    rc = cpssDxChPortSerdesAdaptiveCtleBasedTemperature(devNum, portNum, phase);
                }

                /* GT_BAD_PARAM will be received upon bad portNum argument. We iterating
                   all possible portNum values so in GT_BAD_PARAM case we will ignore this
                   error code and continue execution, either way we won't enter the 2nd 'if'
                   statement as bad portNum argument will not meet the statement condition */
                if(rc != GT_OK)
                {
                    continue;
                }
            } /* for (portNum) */

            /* wait for update the EO*/
            osTimerWkAfter(250);

            /* phase 2*/
            phase = 2;
            for (portNum = 0; portNum < MV_HWS_MAX_PORT_NUM; portNum++) /* phy ports*/
            {
                /************************************************************************
                 Phase 2.Check if the EO  improved, if not, do roll back to old prameters
                **********************************************************************/
                /*check adaptive CTLE enable*/
                bitMapIndex = portNum / HWS_MAX_ADAPT_CTLE_DB_SIZE;
                portAdaptiveCtleEnable =
                    (( hwsDeviceSpecInfo[devNum].adaptiveCtleDb.bitMapEnableCtleCalibrationBasedTemperture[bitMapIndex] &(0x1 << (portNum%HWS_MAX_ADAPT_CTLE_DB_SIZE))) > 0 ? GT_TRUE : GT_FALSE);

                if (portAdaptiveCtleEnable == GT_TRUE)
                {
                    rc = cpssDxChPortSerdesAdaptiveCtleBasedTemperature(devNum, portNum,phase);
                }

                /* GT_BAD_PARAM will be received upon bad portNum argument. We iterating
                   all possible portNum values so in GT_BAD_PARAM case we will ignore this
                   error code and continue execution, either way we won't enter the 2nd 'if'
                   statement as bad portNum argument will not meet the statement condition */
                if(rc != GT_OK)
                {
                    continue;
                }
            } /* for (portNum) */

            /* clear old values valid bitMap*/
            for (ii = 0 ; ii < ((MV_HWS_MAX_PORT_NUM+(HWS_MAX_ADAPT_CTLE_DB_SIZE-1))/HWS_MAX_ADAPT_CTLE_DB_SIZE); ii++)
            {
                hwsDeviceSpecInfo[devNum].adaptiveCtleDb.bitMapOldValuesValid[ii] = 0x00;
            }

            delay = 5000 /* 5 seconds*/;

        }

       osTimerWkAfter(delay);
    } /* while (1) */

    return GT_OK;
}
#endif /*RUN_ADAPTIVE_CTLE*/

/**
* @internal appDemoGetTsenTemperature function
* @endinternal
*
* @brief   TSEN read from Eagle application task
*
* @param[in] devNum                 - pdevNum.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
unsigned __TASKCONV appDemoGetTsenTemperature
(
        GT_VOID * param
)
{
    GT_U8                   devNum;
    GT_U32  delay;
    GT_U32 data;
    int tsenTemp0;
    int tsenTemp2;
    GT_U32 dlyInterval;
    float f_tsenTemp0 = 0;
    float f_tsenTemp2 = 0;
    GT_BOOL t0Neg = GT_FALSE;
    GT_BOOL t2Neg = GT_FALSE;
    GT_U32 regAddr;
    GT_STATUS rc = GT_NOT_APPLICABLE_DEVICE;

    /* Variables initialization */
    devNum = (GT_U8)((GT_UINTPTR)param);
    delay = 30000;
    while(1)
    {
        if(appDemoDbEntryGet("tsenDly", &dlyInterval) == GT_OK)
        {
            delay = dlyInterval;
            if (dlyInterval == 0)
            {
                /* to avoid printing set the delay to 0 */
                osTimerWkAfter(10000);
                continue;
            }
        }
        t0Neg = GT_FALSE;
        regAddr = PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->
            sip6_tile_DFXServerUnits[0/*tile0*/].DFXServerRegs.externalTemperatureSensorStatus;

        rc = cpssDrvPpHwRegisterRead(devNum,0,regAddr,(&data));
        if (rc != GT_OK)
        {
            return rc;
        }
        tsenTemp0 = data & 0x3FF;
        regAddr = 0;
        data = 0;
        if (((tsenTemp0) >> 9) & 0x1)
        {
            t0Neg = GT_TRUE;
        }

        tsenTemp0 = ~((tsenTemp0));
        tsenTemp0 &= 0x3FF;

        if (t0Neg)
        {
            tsenTemp0 += 0x1;
            tsenTemp0 *= (-1);
            f_tsenTemp0 = (tsenTemp0 * 0.394) + 128.9;
        }
        osPrintf("The temperature from TSEN tile 0 is  %f Celsius\n", f_tsenTemp0);
        if(PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles > 2)
        {
            t2Neg = GT_FALSE;
            regAddr = PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->
                sip6_tile_DFXServerUnits[2/*tile2*/].DFXServerRegs.externalTemperatureSensorStatus;
            rc = cpssDrvPpHwRegisterRead(devNum,0,regAddr,(&data));
            if (rc != GT_OK)
            {
                return rc;
            }
            tsenTemp2 = data & 0x3FF;
            if (((tsenTemp2) >> 9) & 0x1)
            {
                t2Neg = GT_TRUE;
            }
            tsenTemp2 = ~((tsenTemp2));
            tsenTemp2 &= 0x3FF;
            if (t2Neg)
            {
                tsenTemp2 += 0x1;
                tsenTemp2 *= (-1);
                f_tsenTemp2 = (tsenTemp2 * 0.394) + 128.9;
            }
            osPrintf("The temperature from TSEN tile 2 is  %f Celsius\n", f_tsenTemp2);
        }
        osTimerWkAfter(delay);
    } /* while (1) */

    return GT_OK;
}
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
)
{
    GT_STATUS                       rc;

    /********************************************
    Creating handling task if not created yet.
    *******************************************/
        GT_U32 eagleTsenTemperatureGetStartTaskId;

        rc = cpssOsTaskCreate("eagleTsenRead",    /* Task Name      */
                              500,                  /* Task Priority  */
                              _64K,                 /* Stack Size     */
                              appDemoGetTsenTemperature, /* Starting Point */
                              (GT_VOID*)((GT_UINTPTR)devNum),        /* Arguments list */
                              &eagleTsenTemperatureGetStartTaskId);             /* task ID        */
        if (rc != GT_OK)
        {
            APP_DEMO_LOG_ERROR("calling cpssOsTaskCreate returned=%d", rc);
            return rc;
        }

    return GT_OK;
}



