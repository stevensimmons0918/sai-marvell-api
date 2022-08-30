/********************************************************************************
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
* @file cpssDxChPortManagerSamples.c
*
* @brief CPSS implementation for Port management.
*
* @version   1
********************************************************************************
*/
#define CPSS_LOG_IN_MODULE_ENABLE

#include <cpss/dxCh/dxChxGen/port/private/prvCpssDxChPortLog.h>
#include <cpss/common/port/private/prvCpssPortManager.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortManager.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfoEnhanced.h>
#include <cpssDriver/pp/prvCpssDrvPpDefs.h>
#include <cpss/dxCh/dxChxGen/txq/private/utils/prvCpssDxChTxqPreemptionUtils.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

/*global variables macros*/

#define PRV_SHARED_PORT_DIR_PORT_MANAGER_SAMPLE_SRC_GLOBAL_VAR_SET(_var,_value)\
    PRV_SHARED_GLOBAL_VAR_SET(mainPpDrvMod.portDir.portManagerSampleSrc._var,_value)

#define PRV_SHARED_PORT_DIR_PORT_MANAGER_SAMPLE_SRC_GLOBAL_VAR_GET(_var)\
    PRV_SHARED_GLOBAL_VAR_GET(mainPpDrvMod.portDir.portManagerSampleSrc._var)



/* debugLegacyCommandsMode: flag to allow the LUA and UT-E commands of the port like: speed , loopback behave like
   in legacy mode , meaning that not need to call command to delete port , when
   port that already created need to be in loopback mode */
/* function to allow setting the 'debug' mode */
/* called before running the legacy tests that run the lua CLI commands */
GT_STATUS   cpssDxChSamplePortManager_debugLegacyCommandsMode_Set(IN GT_BOOL legacyMode)
{
    PRV_NON_SHARED_NON_VOLATILE_GLOBAL_VAR_SET(generalVars.debugLegacyCommandsMode,legacyMode);

    return GT_OK;
}

GT_STATUS   cpssDxChSamplePortManager_debugLegacyCommandsMode_Get(IN GT_BOOL * legacyModePtr)
{
    *legacyModePtr = PRV_NON_SHARED_NON_VOLATILE_GLOBAL_VAR_GET(generalVars.debugLegacyCommandsMode);

    return GT_OK;
}

#if 0
static GT_BOOL  trace_debugLuaLegacyCommandsMode = GT_FALSE;
GT_STATUS   trace_debugLuaLegacyCommandsMode_Set(IN GT_BOOL trace)
{
    trace_debugLuaLegacyCommandsMode = trace;

    return GT_OK;
}



static GT_BOOL  debugLoopbackWaitForInitDone = GT_FALSE;
GT_STATUS   debugLoopbackWaitForInitDone_Set(IN GT_BOOL wait)
{
    debugLoopbackWaitForInitDone = wait;

    return GT_OK;
}
#endif

GT_STATUS   cpssDxChSamplePortManager_debugFlagsDump()
{
    cpssOsPrintf("debugLegacyCommandsMode %d \n",PRV_NON_SHARED_NON_VOLATILE_GLOBAL_VAR_GET(generalVars.debugLegacyCommandsMode));
#if 0
    cpssOsPrintf("trace_debugLuaLegacyCommandsMode %d \n",trace_debugLuaLegacyCommandsMode);
    cpssOsPrintf("debugLoopbackWaitForInitDone %d \n",debugLoopbackWaitForInitDone);
#endif
    return GT_OK;
}



/*-----Regular-Samples-----*/

/**
* @internal cpssDxChSamplePortManagerMandatoryParamsExtSet function
* @endinternal
*
* @brief   set the mandtory params in the pm database(with preemption support)
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] ifMode                   - port mode
* @param[in] speed                        - port speed
* @param[in] fecMode                    - port fec mode
* @param[in] preemptionType     - preemption feature parameters
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on passing null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChSamplePortManagerMandatoryParamsExtSet
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  CPSS_PORT_INTERFACE_MODE_ENT    ifMode,
    IN  CPSS_PORT_SPEED_ENT             speed,
    IN  CPSS_PORT_FEC_MODE_ENT          fecMode,
    IN  CPSS_PM_MAC_PREEMPTION_TYPE_ENT preemptionType
)
{
    CPSS_PM_PORT_PARAMS_STC             portParams;
    GT_STATUS                           rc = GT_OK;
    GT_BOOL                             preemptionAllowed = GT_FALSE;
#if 0
    GT_U32                              laneNum;
    CPSS_PORT_SERDES_TX_CONFIG_STC      portParamsInputStc;
    GT_U32                              portMacNum;
    MV_HWS_PORT_INIT_PARAMS             curPortParams;
    MV_HWS_PORT_STANDARD                portMode;
#endif

    PRV_CPSS_DEV_CHECK_MAC(devNum);

    rc = prvCpssDxChTxqSip6PreemptionFeatureAllowedGet(devNum,&preemptionAllowed);
    if (rc != GT_OK)
    {
        return rc;
    }

    if((GT_FALSE==preemptionAllowed)&&(preemptionType!=CPSS_PM_MAC_PREEMPTION_DISABLED_E))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Device does not support preemption.");
    }

    rc = cpssDxChPortManagerPortParamsStructInit(CPSS_PORT_MANAGER_PORT_TYPE_REGULAR_E, &portParams);
    if (rc != GT_OK)
    {
        return rc;
    }

    portParams.portType = CPSS_PORT_MANAGER_PORT_TYPE_REGULAR_E;
    portParams.portParamsType.regPort.ifMode = ifMode;
    portParams.portParamsType.regPort.speed = speed;

    CPSS_PM_SET_VALID_ATTR(&portParams, CPSS_PM_ATTR_FEC_MODE_E);
    portParams.portParamsType.regPort.portAttributes.fecMode = fecMode;

    /* sample code that configured the port manager to ensure the EO parameters are within pre defined range,
       this to ensure stable signal, note that the threshold may vary depending on board */
    if ((PRV_CPSS_SIP_5_20_CHECK_MAC(devNum)) && !(PRV_CPSS_SIP_6_CHECK_MAC(devNum)))
    {
        CPSS_PM_SET_VALID_ATTR(&portParams, CPSS_PM_ATTR_CALIBRATION_E);
        /* in order to select optinal calibration need to call the function with CPSS_PORT_MANAGER_OPTICAL_CALIBRATION_TYPE_E */
        portParams.portParamsType.regPort.portAttributes.calibrationMode.calibrationType = CPSS_PORT_MANAGER_OPTICAL_CALIBRATION_TYPE_E;
        /* set min Eo Threshold */
        portParams.portParamsType.regPort.portAttributes.calibrationMode.confidenceCfg.eoMinThreshold = 130;
        /* set max Eo Threshold */
        portParams.portParamsType.regPort.portAttributes.calibrationMode.confidenceCfg.eoMaxThreshold = 800;
        portParams.portParamsType.regPort.portAttributes.calibrationMode.confidenceCfg.calibrationTimeOutSec = 10;

        /* portParams.portParamsType.regPort.portAttributes.calibrationMode.confidenceCfg.confidenceEnableBitMap = 0x30; */
        portParams.portParamsType.regPort.portAttributes.calibrationMode.confidenceCfg.confidenceEnableBitMap = 0x30;
    }

    if (speed == CPSS_PORT_SPEED_1000_E && !PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
    {
        CPSS_PM_SET_VALID_ATTR(&portParams, CPSS_PM_ATTR_TRAIN_MODE_E);
        portParams.portParamsType.regPort.portAttributes.trainMode = CPSS_PORT_SERDES_AUTO_TUNE_MODE_RX_TRAINING_VSR_E;
    }

    CPSS_PM_SET_VALID_ATTR(&portParams, CPSS_PM_PORT_ATTR_PREEMPTION_E);
    portParams.portParamsType.regPort.portAttributes.preemptionParams.type = preemptionType;

    rc = cpssDxChPortManagerPortParamsSet(devNum, portNum, &portParams);
    if (rc != GT_OK)
    {
        return rc;
    }
    return GT_OK;
}

/**
* @internal cpssDxChSamplePortManagerMandatoryParamsSet function
* @endinternal
*
* @brief   set the mandtory params in the pm database
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] ifMode                   - port mode
* @param[in] speed                    - port speed
* @param[in] speed                    - port fec mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on passing null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/

GT_STATUS cpssDxChSamplePortManagerMandatoryParamsSet
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  CPSS_PORT_INTERFACE_MODE_ENT    ifMode,
    IN  CPSS_PORT_SPEED_ENT             speed,
    IN  CPSS_PORT_FEC_MODE_ENT          fecMode
)
{
    return cpssDxChSamplePortManagerMandatoryParamsExtSet(devNum,portNum,ifMode,speed,
        fecMode,CPSS_PM_MAC_PREEMPTION_DISABLED_E);
}


#if 0
/* similar to appDemoPortInitSequentialInit() function */
/* operations to force port out of state CPSS_PORT_MANAGER_STATE_INIT_IN_PROGRESS_E */
/* at the end of operation we want traffic to be able to IN/OUT the port :
   so we need to have 'port enabled' and 'EGF link UP'  */
/* used for loopback port that we know that will have link UP in the HW */
static GT_STATUS debugLuaLegacyCommandsMode_loopbackPortProgress
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum
)
{
    GT_STATUS   rc;
    CPSS_PORT_MANAGER_STC portEventStc;
    CPSS_PORT_MANAGER_STATUS_STC portStage;
    GT_BOOL portEn;
    CPSS_DXCH_BRG_EGR_FLT_PORT_LINK_STATUS_ENT portLinkStatusState;


#if 0 /* using direct low level APIs ... collide with the 'port manager' concept */
    /* enable the port */
    rc = cpssDxChPortEnableSet(devNum,portNum,GT_TRUE);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* allow traffic to it (EGF) */
    rc = cpssDxChBrgEgrFltPortLinkEnableSet(devNum,portNum,
        CPSS_DXCH_BRG_EGR_FLT_PORT_LINK_STATUS_FORCE_LINK_UP_E);
    if(rc != GT_OK)
    {
        return rc;
    }

    return GT_OK;
#endif /* 0 */


    portEventStc.portEvent = CPSS_PORT_MANAGER_EVENT_LOW_LEVEL_STATUS_CHANGED_E;
    rc = cpssDxChPortManagerEventSet(devNum, portNum, &portEventStc);
    if(rc != GT_OK)
    {
        return rc;
    }

    /*
        do loop that check if we in init,
        we will keep giving it init until he reach link_up/link_down/mac_link_down
        or reset in case he got delete.
    */
    do {
        rc = cpssDxChPortManagerStatusGet(devNum, portNum, &portStage);
        if(rc != GT_OK)
        {
            return rc;
        }

        if (portStage.portState == CPSS_PORT_MANAGER_STATE_INIT_IN_PROGRESS_E)
        {
            portEventStc.portEvent = CPSS_PORT_MANAGER_EVENT_INIT_E;
            rc = cpssDxChPortManagerEventSet(devNum, portNum, &portEventStc);
            if(rc != GT_OK)
            {
                return rc;
            }
        }
    } while(portStage.portState == CPSS_PORT_MANAGER_STATE_INIT_IN_PROGRESS_E);


    /* is the port enabled ? */
    rc = cpssDxChPortEnableGet(devNum,portNum,&portEn);
    if(rc != GT_OK)
    {
        return rc;
    }

    if(portEn == GT_FALSE)
    {
        osPrintf("WARNING : port[%d] was not enabled by debugLuaLegacyCommandsMode_cpssDxChSamplePortManagerLoopbackSet \n",
            portNum);

        /* enable the port */
        rc = cpssDxChPortEnableSet(devNum,portNum,GT_TRUE);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    /* check the egf */
    rc = cpssDxChBrgEgrFltPortLinkEnableGet(devNum,portNum,
        &portLinkStatusState);
    if(rc != GT_OK)
    {
        return rc;
    }

    if(portLinkStatusState != CPSS_DXCH_BRG_EGR_FLT_PORT_LINK_STATUS_FORCE_LINK_UP_E)
    {
        osPrintf("WARNING : port[%d] was not 'EGF link up' by debugLuaLegacyCommandsMode_cpssDxChSamplePortManagerLoopbackSet \n",
            portNum);

        /* allow traffic to it (EGF) */
        rc = cpssDxChBrgEgrFltPortLinkEnableSet(devNum,portNum,
            CPSS_DXCH_BRG_EGR_FLT_PORT_LINK_STATUS_FORCE_LINK_UP_E);
        if(rc != GT_OK)
        {
            return rc;
        }
    }
    return GT_OK;
}
#endif

/* debug function for : debugLuaLegacyCommandsMode == GT_TRUE

   function called from cpssDxChSamplePortManagerLoopbackSet when GT_BAD_STATE
   returned from 'port manager' from cpssDxChPortManagerPortParamsSet

   the function do next steps:
       1. delete the port
       2. set new parameters
       3. re-create the port

   NOTE: the function is called under 'CPSS LOCK' because we do multiple operations
    that we not want port manager task (appDemoPortInitSeqSignalDetectedStage)
    to come in the middle of it !
*/


static GT_STATUS debugLuaLegacyCommandsMode_cpssDxChSamplePortManagerLoopbackSet
(
    IN  GT_U8                                   devNum,
    IN  GT_PHYSICAL_PORT_NUM                    portNum,
    IN CPSS_PM_PORT_PARAMS_STC                  *portParamsPtr
)
{
    GT_STATUS   rc;
    CPSS_PORT_MANAGER_STC portEventStc;
    CPSS_PORT_MANAGER_STATUS_STC portStage;
    GT_BOOL loopBackEnable = GT_FALSE;
    /* do next steps:
       1. delete the port
       2. set new parameters
       3. re-create the port
    */

    loopBackEnable = portParamsPtr->portParamsType.regPort.portAttributes.loopback.loopbackMode.macLoopbackEnable;

    rc = cpssDxChPortManagerStatusGet(devNum,portNum,&portStage);
    if(rc != GT_OK)
    {
        return rc;
    }

    if(loopBackEnable == GT_TRUE)
    {
        /* if port alraedy in loopback and link up return,
        this saves time when in PM mode */
        if((PRV_SHARED_PORT_DIR_PORT_MANAGER_SAMPLE_SRC_GLOBAL_VAR_GET(inloopBackMode)[portNum] == GT_TRUE) && (portStage.portState == CPSS_PORT_MANAGER_STATE_LINK_UP_E))
        {
            return GT_OK;
        }
    }

    portEventStc.portEvent = CPSS_PORT_MANAGER_EVENT_DELETE_E;
    rc = cpssDxChPortManagerEventSet(devNum, portNum, &portEventStc);
    if(rc != GT_OK)
    {
        return rc;
    }

    rc = cpssDxChPortManagerPortParamsSet(devNum, portNum, portParamsPtr);
    if(rc != GT_OK)
    {
        return rc;
    }

    portEventStc.portEvent = CPSS_PORT_MANAGER_EVENT_CREATE_E;
    rc = cpssDxChPortManagerEventSet(devNum, portNum, &portEventStc);
    if(rc != GT_OK)
    {
        return rc;
    }
    PRV_SHARED_PORT_DIR_PORT_MANAGER_SAMPLE_SRC_GLOBAL_VAR_GET(inloopBackMode)[portNum] = loopBackEnable;
#if 0
    if(portParamsPtr->portParamsType.regPort.portAttributes.loopback.loopbackType ==
        CPSS_PORT_MANAGER_LOOPBACK_TYPE_MAC_E &&
       portParamsPtr->portParamsType.regPort.portAttributes.loopback.loopbackMode.macLoopbackEnable == GT_TRUE)
    {
        if(debugLoopbackWaitForInitDone == GT_TRUE)
        {
            /*we put the port in loopback ... meaning it will have LINK UP ... soon */

            /* the appDemo task for 'port manager' may be in sleep of up to :
                appDemoPortManagerTaskSleepTime * (1 + 1/4 * 32 ports) */
            for (ii = 0 ; ii < 10 ; ii++)
            {
                rc = cpssDxChPortManagerStatusGet(devNum,portNum,&portStage);
                if(rc != GT_OK)
                {
                    return rc;
                }

                if(portStage.portState > CPSS_PORT_MANAGER_STATE_INIT_IN_PROGRESS_E)
                {
                    if(trace_debugLuaLegacyCommandsMode)
                    {
                        cpssOsPrintf("port[%d] : got to portStage.portState[%d] (can continue test) \n",portNum,portStage.portState);
                    }
                    break;
                }

                if(trace_debugLuaLegacyCommandsMode)
                {
                    cpssOsPrintf("port[%d] : portStage.portState[%d] (wait to be > 'init in progress') \n",portNum,portStage.portState);
                }

                CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
                cpssOsTimerWkAfter(30);
                CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
            }

            if((ii == 10) && trace_debugLuaLegacyCommandsMode)
            {
                cpssOsPrintf("WARNING : port[%d] : not got to proper stage ... traffic may not egress it \n",portNum);
            }
        }
        else
        {
            /* do not wait !  force the missing configurations as the LB port is
               in link-up in the HW */
            /* save a lot of time from the test ! */
            rc = debugLuaLegacyCommandsMode_loopbackPortProgress(devNum,portNum);
            if(rc != GT_OK)
            {
                return rc;
            }
        }
    }
#endif
    return GT_OK;
}


/**
* @internal cpssDxChSamplePortManagerLoopbackSet function
* @endinternal
*
* @brief   set the Loopback params in the pm database
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] loopbackType             - loopback type
* @param[in] macLoopbackEnable        - MAC loopback
*       enable/disable
* @param[in] serdesLoopbackMode       - SerDes loopback type
* @param[in] enableRegularTrafic      - enable trafic
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on passing null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChSamplePortManagerLoopbackSet
(
    IN  GT_U8                                   devNum,
    IN  GT_PHYSICAL_PORT_NUM                    portNum,
    IN  CPSS_PORT_MANAGER_LOOPBACK_TYPE_ENT     loopbackType,
    IN  GT_BOOL                                 macLoopbackEnable,
    IN  CPSS_PORT_SERDES_LOOPBACK_MODE_ENT      serdesLoopbackMode,
    IN  GT_BOOL                                 enableRegularTrafic
)
{
    GT_STATUS rc = GT_OK;
    CPSS_PM_PORT_PARAMS_STC portParams;
    CPSS_PORT_MANAGER_STATUS_STC portStage;

    PRV_CPSS_DEV_CHECK_MAC(devNum);

    rc = cpssDxChPortManagerPortParamsGet(devNum, portNum, &portParams);
    if (rc != GT_OK)
    {
        return rc;
    }

    if(portParams.portType != CPSS_PORT_MANAGER_PORT_TYPE_REGULAR_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    /* checking mandtory params */
    if ((portParams.portParamsType.regPort.ifMode == CPSS_PORT_INTERFACE_MODE_NA_E) ||
        (portParams.portParamsType.regPort.speed == CPSS_PORT_SPEED_NA_E))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    CPSS_PM_SET_VALID_ATTR(&portParams, CPSS_PM_ATTR_LOOPBACK_E);

    portParams.portParamsType.regPort.portAttributes.loopback.loopbackType = loopbackType;

    switch (loopbackType)
    {
        case CPSS_PORT_MANAGER_LOOPBACK_TYPE_MAC_E:
            portParams.portParamsType.regPort.portAttributes.loopback.loopbackMode.macLoopbackEnable = macLoopbackEnable;
            break;
        case CPSS_PORT_MANAGER_LOOPBACK_TYPE_SERDES_E:
            portParams.portParamsType.regPort.portAttributes.loopback.loopbackMode.serdesLoopbackMode = serdesLoopbackMode;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    portParams.portParamsType.regPort.portAttributes.loopback.enableRegularTrafic = enableRegularTrafic;

    if(PRV_NON_SHARED_NON_VOLATILE_GLOBAL_VAR_GET(generalVars.debugLegacyCommandsMode) == GT_TRUE)
    {
         rc = cpssDxChPortManagerStatusGet(devNum,portNum,&portStage);
         if(rc != GT_OK)
         {
             return rc;
         }
        if(portStage.portState != CPSS_PORT_MANAGER_STATE_RESET_E)
        {
            /* the cpssDxChPortManagerPortParamsSet is going to return 'GT_BAD_STATE'
                due to port already created */
            /* so do WA for it */
            /* NOTE: the function is called under 'CPSS LOCK' because we do multiple operations
               that we not want port manager task (appDemoPortInitSeqSignalDetectedStage)
                to come in the middle of it !
            */
            CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
            rc = debugLuaLegacyCommandsMode_cpssDxChSamplePortManagerLoopbackSet(
                    devNum, portNum, &portParams);
            CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
            return rc;
        }
    }

    rc = cpssDxChPortManagerPortParamsSet(devNum, portNum, &portParams);
    if (rc!=GT_OK)
    {
        return rc;
    }


    return GT_OK;
}

/**
* @internal cpssDxChSamplePortManagerSerdesTxParamsSet function
* @endinternal
*
* @brief   set the serdes tx params in the pm database
*
* @param[in] devNum               - device number
* @param[in] portNum              - port number
* @param[in] laneNum              - lane number
* @param[in] portParamsInputSt    - a structure contains port
*                                    configuration
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on passing null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChSamplePortManagerSerdesTxParamsSet
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  GT_U32                          laneNum,
    IN  CPSS_PORT_SERDES_TX_CONFIG_STC  *portParamsInputStcPtr
)
{
    GT_STATUS                       rc = GT_OK;
    CPSS_PM_PORT_PARAMS_STC         portParams;
    MV_HWS_PORT_INIT_PARAMS         curPortParams;
    GT_U32                          portMacNum;
    MV_HWS_PORT_STANDARD            portMode;

    PRV_CPSS_DEV_CHECK_MAC(devNum);

    if (!PRV_CPSS_SIP_5_15_CHECK_MAC(devNum) &&
        !PRV_CPSS_DXCH_IS_AC3_BASED_DEVICE_MAC(devNum))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }

    PRV_CPSS_DXCH_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum,portMacNum);

    if(portParamsInputStcPtr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    rc = cpssDxChPortManagerPortParamsGet(devNum, portNum, &portParams);
    if (rc != GT_OK)
    {
        return rc;
    }

    if(portParams.portType != CPSS_PORT_MANAGER_PORT_TYPE_REGULAR_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }


    /* checking mandtory params */
    if ((portParams.portParamsType.regPort.ifMode == CPSS_PORT_INTERFACE_MODE_NA_E) ||
        (portParams.portParamsType.regPort.speed == CPSS_PORT_SPEED_NA_E))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    rc = prvCpssCommonPortIfModeToHwsTranslate(devNum,portParams.portParamsType.regPort.ifMode,portParams.portParamsType.regPort.speed, &portMode);
    if (rc!=GT_OK)
    {
        return rc;
    }

    rc = hwsPortModeParamsGetToBuffer(CAST_SW_DEVNUM(devNum), 0, portMacNum, portMode, &curPortParams);
    if (rc!=GT_OK)
    {
        return rc;
    }

    if (laneNum >= curPortParams.numOfActLanes)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }

    CPSS_PM_SET_VALID_LANE_PARAM(&portParams, laneNum, CPSS_PM_LANE_PARAM_TX_E);

    portParams.portParamsType.regPort.laneParams[laneNum].txParams.type = portParamsInputStcPtr->type;
    switch (portParamsInputStcPtr->type)
    {
        case CPSS_PORT_SERDES_AVAGO_E:
              cpssOsMemCpy(&(portParams.portParamsType.regPort.laneParams[laneNum].txParams.txTune.avago),
              &(portParamsInputStcPtr->txTune.avago),
              sizeof(CPSS_PORT_AVAGO_SERDES_TX_CONFIG_STC));
            break;
        case CPSS_PORT_SERDES_COMPHY_H_E:
               cpssOsMemCpy(&(portParams.portParamsType.regPort.laneParams[laneNum].txParams.txTune.comphy),
               &(portParamsInputStcPtr->txTune.comphy),
               sizeof(CPSS_PORT_COMPHY_SERDES_TX_CONFIG_STC));
            break;
        case CPSS_PORT_SERDES_COMPHY_C12G_E:
               cpssOsMemCpy(&(portParams.portParamsType.regPort.laneParams[laneNum].txParams.txTune.comphy_C12G),
               &(portParamsInputStcPtr->txTune.comphy_C12G),
               sizeof(CPSS_PORT_COMPHY_C12G_TX_CONFIG_STC));
            break;
        case CPSS_PORT_SERDES_COMPHY_C28G_E:
               cpssOsMemCpy(&(portParams.portParamsType.regPort.laneParams[laneNum].txParams.txTune.comphy_C28G),
               &(portParamsInputStcPtr->txTune.comphy_C28G),
               sizeof(CPSS_PORT_COMPHY_C28G_TX_CONFIG_STC));
            break;
        case CPSS_PORT_SERDES_COMPHY_C112G_E:
               cpssOsMemCpy(&(portParams.portParamsType.regPort.laneParams[laneNum].txParams.txTune.comphy_C112G),
               &(portParamsInputStcPtr->txTune.comphy_C112G),
               sizeof(CPSS_PORT_COMPHY_C112G_TX_CONFIG_STC));
            break;
        case CPSS_PORT_SERDES_COMPHY_C56G_E:
               cpssOsMemCpy(&(portParams.portParamsType.regPort.laneParams[laneNum].txParams.txTune.comphy_C56G),
               &(portParamsInputStcPtr->txTune.comphy_C56G),
               sizeof(CPSS_PORT_COMPHY_C56G_TX_CONFIG_STC));
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
            break;
    }

    rc = cpssDxChPortManagerPortParamsSet(devNum, portNum, &portParams);
    if (rc!=GT_OK)
    {
        return rc;
    }
    return GT_OK;
}

/**
* @internal cpssDxChSamplePortManagerSerdesRxParamsSet function
* @endinternal
*
* @brief   set the serdes rx params in the pm database
*
* @param[in] devNum               - device number
* @param[in] portNum              - port number
* @param[in] laneNum              - lane number
* @param[in] portParamsInputSt    - a structure contains port
*                                    configuration
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on passing null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChSamplePortManagerSerdesRxParamsSet
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  GT_U32                          laneNum,
    IN  CPSS_PORT_SERDES_RX_CONFIG_STC  *portParamsInputStcPtr
)
{
    GT_STATUS rc = GT_OK;
    CPSS_PM_PORT_PARAMS_STC portParams;
    MV_HWS_PORT_INIT_PARAMS         curPortParams;
    GT_U32                          portMacNum;
    MV_HWS_PORT_STANDARD            portMode;

    PRV_CPSS_DEV_CHECK_MAC(devNum);

    if (!PRV_CPSS_SIP_5_15_CHECK_MAC(devNum) &&
        !PRV_CPSS_DXCH_IS_AC3_BASED_DEVICE_MAC(devNum))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }

    PRV_CPSS_DXCH_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum,portMacNum);

    rc = cpssDxChPortManagerPortParamsGet(devNum, portNum, &portParams);
    if (rc != GT_OK)
    {
        return rc;
    }

    if(portParams.portType != CPSS_PORT_MANAGER_PORT_TYPE_REGULAR_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    /* checking mandtory params */
    if ((portParams.portParamsType.regPort.ifMode == CPSS_PORT_INTERFACE_MODE_NA_E) ||
        (portParams.portParamsType.regPort.speed == CPSS_PORT_SPEED_NA_E))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    rc = prvCpssCommonPortIfModeToHwsTranslate(devNum,portParams.portParamsType.regPort.ifMode,portParams.portParamsType.regPort.speed, &portMode);
    if (rc!=GT_OK)
    {
        return rc;
    }

    rc = hwsPortModeParamsGetToBuffer(CAST_SW_DEVNUM(devNum), 0, portMacNum, portMode, &curPortParams);
    if (rc!=GT_OK)
    {
        return rc;
    }

    if (laneNum >= curPortParams.numOfActLanes)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }

    CPSS_PM_SET_VALID_LANE_PARAM(&portParams, laneNum, CPSS_PM_LANE_PARAM_RX_E);

    portParams.portParamsType.regPort.laneParams[laneNum].rxParams.type = portParamsInputStcPtr->type;

    switch (portParamsInputStcPtr->type)
    {
        case CPSS_PORT_SERDES_AVAGO_E:
               cpssOsMemCpy(&(portParams.portParamsType.regPort.laneParams[laneNum].rxParams.rxTune.avago),
               &(portParamsInputStcPtr->rxTune.avago),
               sizeof(CPSS_PORT_AVAGO_SERDES_RX_CONFIG_STC));
            break;
        case CPSS_PORT_SERDES_COMPHY_H_E:
               cpssOsMemCpy(&(portParams.portParamsType.regPort.laneParams[laneNum].rxParams.rxTune.comphy),
               &(portParamsInputStcPtr->rxTune.comphy),
               sizeof(CPSS_PORT_COMPHY_SERDES_RX_CONFIG_STC));
            break;
        case CPSS_PORT_SERDES_COMPHY_C12G_E:
               cpssOsMemCpy(&(portParams.portParamsType.regPort.laneParams[laneNum].rxParams.rxTune.comphy_C12G),
               &(portParamsInputStcPtr->rxTune.comphy_C12G),
               sizeof(CPSS_PORT_COMPHY_C12GP41P2V_RX_CONFIG_STC));
            break;
        case CPSS_PORT_SERDES_COMPHY_C28G_E:
               cpssOsMemCpy(&(portParams.portParamsType.regPort.laneParams[laneNum].rxParams.rxTune.comphy_C28G),
               &(portParamsInputStcPtr->rxTune.comphy_C28G),
               sizeof(CPSS_PORT_COMPHY_C28GP4_RX_CONFIG_STC));
            break;
        case CPSS_PORT_SERDES_COMPHY_C112G_E:
               cpssOsMemCpy(&(portParams.portParamsType.regPort.laneParams[laneNum].rxParams.rxTune.comphy_C112G),
               &(portParamsInputStcPtr->rxTune.comphy_C112G),
               sizeof(CPSS_PORT_COMPHY_C112G_RX_CONFIG_STC));
            break;
        case CPSS_PORT_SERDES_COMPHY_C56G_E:
               cpssOsMemCpy(&(portParams.portParamsType.regPort.laneParams[laneNum].rxParams.rxTune.comphy_C56G),
               &(portParamsInputStcPtr->rxTune.comphy_C56G),
               sizeof(CPSS_PORT_COMPHY_C56G_RX_CONFIG_STC));
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
            break;
    }

    rc = cpssDxChPortManagerPortParamsSet(devNum, portNum, &portParams);
    if (rc!=GT_OK)
    {
        return rc;
    }
    return GT_OK;
}

/**
* @internal cpssDxChSamplePortManagerTrainModeSet function
* @endinternal
*
* @brief   set the TrainMode attr in the pm database
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] trainMode                - trainMode type
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on passing null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChSamplePortManagerTrainModeSet
(
    IN  GT_U8                                   devNum,
    IN  GT_PHYSICAL_PORT_NUM                    portNum,
    IN  CPSS_PORT_SERDES_AUTO_TUNE_MODE_ENT     trainModeType
)
{
    GT_STATUS rc = GT_OK;
    CPSS_PM_PORT_PARAMS_STC portParams;

    PRV_CPSS_DEV_CHECK_MAC(devNum);

    if (trainModeType >=  CPSS_PORT_SERDES_AUTO_TUNE_MODE_LAST_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    rc = cpssDxChPortManagerPortParamsGet(devNum, portNum, &portParams);
    if (rc != GT_OK)
    {
        return rc;
    }

    if(portParams.portType != CPSS_PORT_MANAGER_PORT_TYPE_REGULAR_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    /* checking mandtory params */
    if ((portParams.portParamsType.regPort.ifMode == CPSS_PORT_INTERFACE_MODE_NA_E) ||
        (portParams.portParamsType.regPort.speed == CPSS_PORT_SPEED_NA_E))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    CPSS_PM_SET_VALID_ATTR(&portParams, CPSS_PM_ATTR_TRAIN_MODE_E);
    portParams.portParamsType.regPort.portAttributes.trainMode = trainModeType;

    rc = cpssDxChPortManagerPortParamsSet(devNum, portNum, &portParams);

    return rc;
}

/**
* @internal cpssDxChSamplePortManagerAdaptiveRxTrainSuppSet function
* @endinternal
*
* @brief   set the AdaptiveRxTrainSupp attr in the pm database
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] adaptRxTrainSupp         - boolean whether adaptive rx tune is supported
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on passing null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChSamplePortManagerAdaptiveRxTrainSuppSet
(
    IN  GT_U8                                   devNum,
    IN  GT_PHYSICAL_PORT_NUM                    portNum,
    IN  GT_BOOL                                 adaptRxTrainSupp
)
{
    GT_STATUS rc = GT_OK;
    CPSS_PM_PORT_PARAMS_STC portParams;

    PRV_CPSS_DEV_CHECK_MAC(devNum);

    if ((adaptRxTrainSupp != GT_TRUE) && (adaptRxTrainSupp != GT_FALSE))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    rc = cpssDxChPortManagerPortParamsGet(devNum, portNum, &portParams);
    if (rc != GT_OK)
    {
        return rc;
    }

    if(portParams.portType != CPSS_PORT_MANAGER_PORT_TYPE_REGULAR_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    /* checking mandatory params */
    if ((portParams.portParamsType.regPort.ifMode == CPSS_PORT_INTERFACE_MODE_NA_E) ||
        (portParams.portParamsType.regPort.speed == CPSS_PORT_SPEED_NA_E))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    CPSS_PM_SET_VALID_ATTR(&portParams, CPSS_PM_ATTR_RX_TRAIN_SUPP_E);
    portParams.portParamsType.regPort.portAttributes.adaptRxTrainSupp = adaptRxTrainSupp;

    rc = cpssDxChPortManagerPortParamsSet(devNum, portNum, &portParams);

    return rc;
}


/**
* @internal cpssDxChSamplePortManagerEdgeDetectSuppSet function
* @endinternal
*
* @brief   set the EdgeDetectSupp attr in the pm database
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] edgeDetectSupported      - boolean whether edge detect is supported
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on passing null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChSamplePortManagerEdgeDetectSuppSet
(
    IN  GT_U8                                   devNum,
    IN  GT_PHYSICAL_PORT_NUM                    portNum,
    IN  GT_BOOL                                 edgeDetectSupported
)
{
    GT_STATUS rc = GT_OK;
    CPSS_PM_PORT_PARAMS_STC portParams;

    PRV_CPSS_DEV_CHECK_MAC(devNum);

    if ((edgeDetectSupported != GT_TRUE) && (edgeDetectSupported != GT_FALSE))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    rc = cpssDxChPortManagerPortParamsGet(devNum, portNum, &portParams);
    if (rc != GT_OK)
    {
        return rc;
    }

    if(portParams.portType != CPSS_PORT_MANAGER_PORT_TYPE_REGULAR_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    /* checking mandatory params */
    if ((portParams.portParamsType.regPort.ifMode == CPSS_PORT_INTERFACE_MODE_NA_E) ||
        (portParams.portParamsType.regPort.speed == CPSS_PORT_SPEED_NA_E))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    CPSS_PM_SET_VALID_ATTR(&portParams, CPSS_PM_ATTR_EDGE_DETECT_SUPP_E);
    portParams.portParamsType.regPort.portAttributes.edgeDetectSupported = edgeDetectSupported;

    rc = cpssDxChPortManagerPortParamsSet(devNum, portNum, &portParams);

    return rc;
}

/**
* @internal cpssDxChSamplePortManagerFecModeSet function
* @endinternal
*
* @brief   set the FecMode attr in the pm database
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] fecMode                  - FEC mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on passing null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChSamplePortManagerFecModeSet
(
    IN  GT_U8                                   devNum,
    IN  GT_PHYSICAL_PORT_NUM                    portNum,
    IN  CPSS_PORT_FEC_MODE_ENT                  fecMode
)
{
    GT_STATUS rc = GT_OK;
    CPSS_PM_PORT_PARAMS_STC portParams;

    PRV_CPSS_DEV_CHECK_MAC(devNum);

    if (fecMode >= CPSS_PORT_FEC_MODE_LAST_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    rc = cpssDxChPortManagerPortParamsGet(devNum, portNum, &portParams);
    if (rc != GT_OK)
    {
        return rc;
    }

    if(portParams.portType != CPSS_PORT_MANAGER_PORT_TYPE_REGULAR_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    /* checking mandatory params */
    if ((portParams.portParamsType.regPort.ifMode == CPSS_PORT_INTERFACE_MODE_NA_E) ||
        (portParams.portParamsType.regPort.speed == CPSS_PORT_SPEED_NA_E))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    CPSS_PM_SET_VALID_ATTR(&portParams, CPSS_PM_ATTR_FEC_MODE_E);
    portParams.portParamsType.regPort.portAttributes.fecMode = fecMode;

    rc = cpssDxChPortManagerPortParamsSet(devNum, portNum, &portParams);

    return rc;
}

/**
* cpssDxChSamplePortManagerEnhanceTuneOverrideSet function
*
* @brief   Enhanced Tune Override Params Set fn.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] minLF                    - minimum LF
* @param[in] maxLF                    - max LF
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on passing null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/

GT_STATUS cpssDxChSamplePortManagerEnhanceTuneOverrideSet
(
    IN  GT_U8                                   devNum,
    IN  GT_PHYSICAL_PORT_NUM                    portNum,
    IN  GT_U8                                   minLF,
    IN  GT_U8                                   maxLF
)
{
    CPSS_PM_PORT_PARAMS_STC portParams;
    GT_STATUS               rc;

    PRV_CPSS_DEV_CHECK_MAC(devNum);

    rc = cpssDxChPortManagerPortParamsGet(devNum, portNum, &portParams);
    if (rc != GT_OK)
    {
        return rc;
    }

    if(portParams.portType != CPSS_PORT_MANAGER_PORT_TYPE_REGULAR_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    /* checking mandatory params */
    if ((portParams.portParamsType.regPort.ifMode == CPSS_PORT_INTERFACE_MODE_NA_E) ||
        (portParams.portParamsType.regPort.speed == CPSS_PORT_SPEED_NA_E))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    CPSS_PM_SET_VALID_ATTR (&portParams, CPSS_PM_ATTR_ET_OVERRIDE_E);
    portParams.portParamsType.regPort.portAttributes.etOverride.minLF = minLF;
    portParams.portParamsType.regPort.portAttributes.etOverride.maxLF = maxLF;

    rc = cpssDxChPortManagerPortParamsSet(devNum, portNum, &portParams);

    return rc;
}

/**
* cpssDxChSamplePortManagerOpticalCalibrationSet function
*
* @brief   pre defined calibration type set command for port
*          manager
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] calibrationType          - Cu or Optical
* @param[in] minEoThreshold           - minimum EO threshold
* @param[in] maxEoThreshold           - max EO threshold
* @param[in] bitMapEnable             - BM enable
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on passing null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/

GT_STATUS cpssDxChSamplePortManagerOpticalCalibrationSet
(
    IN  GT_U8                                   devNum,
    IN  GT_PHYSICAL_PORT_NUM                    portNum,
    IN  CPSS_PORT_MANAGER_CALIBRATION_TYPE_ENT  calibrationType,
    IN  GT_U32                                  minEoThreshold,
    IN  GT_U32                                  maxEoThreshold,
    IN  GT_U32                                  bitMapEnable
)
{
    CPSS_PM_PORT_PARAMS_STC portParams;
    GT_STATUS               rc;

    PRV_CPSS_DEV_CHECK_MAC(devNum);

    if (CPSS_PORT_MANAGER_CALIBRATION_TYPE_LAST_E <= calibrationType)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    rc = cpssDxChPortManagerPortParamsGet(devNum, portNum, &portParams);
    if (rc != GT_OK)
    {
        return rc;
    }

    if(portParams.portType != CPSS_PORT_MANAGER_PORT_TYPE_REGULAR_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    /* checking mandatory params */
    if ((portParams.portParamsType.regPort.ifMode == CPSS_PORT_INTERFACE_MODE_NA_E) ||
        (portParams.portParamsType.regPort.speed == CPSS_PORT_SPEED_NA_E))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    /* standard */
    CPSS_PM_SET_VALID_ATTR (&portParams, CPSS_PM_ATTR_CALIBRATION_E);

    /* in order to select optinal calibration need to call the function with CPSS_PORT_MANAGER_OPTICAL_CALIBRATION_TYPE_E */
    portParams.portParamsType.regPort.portAttributes.calibrationMode.calibrationType = calibrationType;
    /* setting confidence interval parameters */
    portParams.portParamsType.regPort.portAttributes.calibrationMode.confidenceCfg.lfMinThreshold = 12;
    portParams.portParamsType.regPort.portAttributes.calibrationMode.confidenceCfg.lfMaxThreshold = 15;
    portParams.portParamsType.regPort.portAttributes.calibrationMode.confidenceCfg.hfMinThreshold = 0;
    portParams.portParamsType.regPort.portAttributes.calibrationMode.confidenceCfg.hfMaxThreshold = 4;
    portParams.portParamsType.regPort.portAttributes.calibrationMode.confidenceCfg.eoMinThreshold = minEoThreshold;
    portParams.portParamsType.regPort.portAttributes.calibrationMode.confidenceCfg.eoMaxThreshold = maxEoThreshold;
    portParams.portParamsType.regPort.portAttributes.calibrationMode.confidenceCfg.calibrationTimeOutSec = 10;

    /* for now the requirment is to use only EO as filtring until CI data will be collected */
    /* portParams.portParamsType.regPort.portAttributes.calibrationMode.confidenceCfg.confidenceEnableBitMap = 0x30; */
    portParams.portParamsType.regPort.portAttributes.calibrationMode.confidenceCfg.confidenceEnableBitMap = bitMapEnable;

    rc = cpssDxChPortManagerPortParamsSet(devNum, portNum, &portParams);

    return rc;
}

/**
* cpssDxChSamplePortManagerUnMaskModeSet function
*
* @brief   defines whether to allow port manager unmask LOW/MAC
*          level events or not
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] unMaskMode               - unmask mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on passing null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChSamplePortManagerUnMaskModeSet
(
    IN  GT_U8                                     devNum,
    IN  GT_PHYSICAL_PORT_NUM                      portNum,
    IN  CPSS_PORT_MANAGER_UNMASK_EVENTS_MODE_ENT  unMaskMode
)
{
    CPSS_PM_PORT_PARAMS_STC portParams;
    GT_STATUS               rc;

    PRV_CPSS_DEV_CHECK_MAC(devNum);

    if (CPSS_PORT_MANAGER_UNMASK_EVENTS_MODE_LAST_E <= unMaskMode)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    rc = cpssDxChPortManagerPortParamsGet(devNum, portNum, &portParams);
    if (rc != GT_OK)
    {
        return rc;
    }

    if(portParams.portType != CPSS_PORT_MANAGER_PORT_TYPE_REGULAR_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    /* checking mandatory params */
    if ((portParams.portParamsType.regPort.ifMode == CPSS_PORT_INTERFACE_MODE_NA_E) ||
        (portParams.portParamsType.regPort.speed == CPSS_PORT_SPEED_NA_E))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }


    CPSS_PM_SET_VALID_ATTR (&portParams, CPSS_PM_ATTR_UNMASK_EV_MODE_E);
    portParams.portParamsType.regPort.portAttributes.unMaskEventsMode = unMaskMode;

    rc = cpssDxChPortManagerPortParamsSet(devNum, portNum, &portParams);

    return rc;
}

/**
* cpssDxChSamplePortManagerBwModeSet function
*
* @brief   defines whether to allow port manager unmask LOW/MAC
*          level events or not
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] bwMode                   - bw mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on passing null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChSamplePortManagerBwModeSet
(
    IN  GT_U8                     devNum,
    IN  GT_PHYSICAL_PORT_NUM      portNum,
    IN  CPSS_PORT_PA_BW_MODE_ENT  bwMode
)
{
    CPSS_PM_PORT_PARAMS_STC portParams;
    GT_STATUS               rc;

    PRV_CPSS_DEV_CHECK_MAC(devNum);

    if (CPSS_PORT_PA_BW_MODE_LAST_E <= bwMode)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    rc = cpssDxChPortManagerPortParamsGet(devNum, portNum, &portParams);
    if (rc != GT_OK)
    {
        return rc;
    }

    if(portParams.portType != CPSS_PORT_MANAGER_PORT_TYPE_REGULAR_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    /* checking mandatory params */
    if ((portParams.portParamsType.regPort.ifMode == CPSS_PORT_INTERFACE_MODE_NA_E) ||
        (portParams.portParamsType.regPort.speed == CPSS_PORT_SPEED_NA_E))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }


    CPSS_PM_SET_VALID_ATTR (&portParams, CPSS_PM_ATTR_BW_MODE_E);
    portParams.portParamsType.regPort.portAttributes.bwMode = bwMode;

    rc = cpssDxChPortManagerPortParamsSet(devNum, portNum, &portParams);

    return rc;
}

/**
* @internal
*           cpssDxChSamplePortManagerOverrideInterconnectProfileSet
*           function
* @endinternal
*
* @brief   set the interconnect profile in the pm database
*
* @param[in] devNum                - device number
* @param[in] portNum               - port number
* @param[in] interconnectProfile    - interconnect profile
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on passing null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChSamplePortManagerOverrideInterconnectProfileSet
(
    IN  GT_U8                                       devNum,
    IN  GT_PHYSICAL_PORT_NUM                        portNum,
    IN  CPSS_PORT_MANAGER_INTERCONNECT_PROFILE_ENT  interconnectProfile
)
{
    CPSS_PM_PORT_PARAMS_STC portParams;
    GT_STATUS rc = GT_OK;

    PRV_CPSS_DEV_CHECK_MAC(devNum);

    rc = cpssDxChPortManagerPortParamsGet(devNum, portNum, &portParams);
    if (rc != GT_OK)
    {
        return rc;
    }

    if(portParams.portType != CPSS_PORT_MANAGER_PORT_TYPE_REGULAR_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    /* checking mandatory params */
    if ((portParams.portParamsType.regPort.ifMode == CPSS_PORT_INTERFACE_MODE_NA_E) ||
        (portParams.portParamsType.regPort.speed == CPSS_PORT_SPEED_NA_E))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    CPSS_PM_SET_VALID_ATTR(&portParams,CPSS_PM_PORT_ATTR_INTERCONNECT_PROFILE_E);

    if (PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
            portParams.portParamsType.regPort.portAttributes.interconnectProfile = interconnectProfile;
    }
    else
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    rc = cpssDxChPortManagerPortParamsSet(devNum, portNum, &portParams);
    if (rc != GT_OK)
    {
        return rc;
    }

    return GT_OK;
}

/**
* @internal
*           cpssDxChSamplePortManagerAutoNegotiationSet
*           function
* @endinternal
*
* @brief   set auto negotiation enable in the pm database
*
* @param[in] devNum                - device number
* @param[in] portNum               - port number
* @param[in] enableInband          - enable inband
* @param[in] enableDuplex          - enable duplex
* @param[in] enableSpeed           - enable speed
* @param[in] enableByPass          - enable bypass
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on passing null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChSamplePortManagerAutoNegotiationSet
(
    IN  GT_U8                  devNum,
    IN  GT_PHYSICAL_PORT_NUM   portNum,
    IN  GT_BOOL                enableInband,
    IN  GT_BOOL                enableDuplex,
    IN  GT_BOOL                enableSpeed,
    IN  GT_BOOL                enableFlowCtrl,
    IN  GT_BOOL                enableFlowCtrlPauseAdvertise,
    IN  GT_BOOL                enableFlowCtrlAsmAdvertise,
    IN  GT_BOOL                enableByPass
)
{
    CPSS_PM_PORT_PARAMS_STC portParams;
    GT_STATUS rc = GT_OK;

    PRV_CPSS_DEV_CHECK_MAC(devNum);

    rc = cpssDxChPortManagerPortParamsGet(devNum, portNum, &portParams);
    if (rc != GT_OK)
    {
        return rc;
    }

    if(portParams.portType != CPSS_PORT_MANAGER_PORT_TYPE_REGULAR_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    /* checking mandatory params */
    if ((portParams.portParamsType.regPort.ifMode == CPSS_PORT_INTERFACE_MODE_NA_E) ||
        (portParams.portParamsType.regPort.speed == CPSS_PORT_SPEED_NA_E))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    CPSS_PM_SET_VALID_ATTR(&portParams,CPSS_PM_PORT_ATTR_AUTO_NEG_ENABLE);

    portParams.portParamsType.regPort.portAttributes.autoNegotiation.inbandEnable  = enableInband;
    portParams.portParamsType.regPort.portAttributes.autoNegotiation.duplexEnable  = enableDuplex;
    portParams.portParamsType.regPort.portAttributes.autoNegotiation.speedEnable   = enableSpeed;
    portParams.portParamsType.regPort.portAttributes.autoNegotiation.flowCtrlEnable = enableFlowCtrl;
    portParams.portParamsType.regPort.portAttributes.autoNegotiation.flowCtrlPauseAdvertiseEnable = enableFlowCtrlPauseAdvertise;
    portParams.portParamsType.regPort.portAttributes.autoNegotiation.flowCtrlAsmAdvertiseEnable = enableFlowCtrlAsmAdvertise;
    portParams.portParamsType.regPort.portAttributes.autoNegotiation.byPassEnable  = enableByPass;

    rc = cpssDxChPortManagerPortParamsSet(devNum, portNum, &portParams);
    if (rc != GT_OK)
    {
        return rc;
    }

    return GT_OK;
}


/**
* @internal
*           cpssDxChSamplePortManagerPmOverFwSet function
* @endinternal
*
* @brief   set PM over FW enable in the pm database
*
* @param[in] devNum                - device number
* @param[in] portNum               - port number
* @param[in] enablePmOverFw        - enable enablePmOverFw
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on passing null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChSamplePortManagerPmOverFwSet
(
    IN  GT_U8                  devNum,
    IN  GT_PHYSICAL_PORT_NUM   portNum,
    IN  GT_BOOL                enablePmOverFw
)
{
    CPSS_PM_PORT_PARAMS_STC portParams;
    GT_STATUS rc = GT_OK;

    PRV_CPSS_DEV_CHECK_MAC(devNum);

    rc = cpssDxChPortManagerPortParamsGet(devNum, portNum, &portParams);
    if (rc != GT_OK)
    {
        return rc;
    }

    if(portParams.portType != CPSS_PORT_MANAGER_PORT_TYPE_REGULAR_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    /* checking mandatory params */
    if ((portParams.portParamsType.regPort.ifMode == CPSS_PORT_INTERFACE_MODE_NA_E) ||
        (portParams.portParamsType.regPort.speed == CPSS_PORT_SPEED_NA_E))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    CPSS_PM_SET_VALID_ATTR(&portParams,CPSS_PM_PORT_ATTR_PM_OVER_FW_E);

    portParams.portParamsType.regPort.portAttributes.pmOverFw  = enablePmOverFw;

    rc = cpssDxChPortManagerPortParamsSet(devNum, portNum, &portParams);
    if (rc != GT_OK)
    {
        return rc;
    }
    return GT_OK;
}

/*-----AP-Samples-----*/

/**
* @internal cpssDxChSamplePortManagerApMandatoryParamsSet function
* @endinternal
*
* @brief   set the mandtory params in the pm database
*
* @param[in] devNum                - device number
* @param[in] portNum               - port number
* @param[in] ifMode                - port mode
* @param[in] speed                 - port speed
* @param[in] fecSupported          - fec supported
* @param[in] fecRequested          - fec requested
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on passing null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChSamplePortManagerApMandatoryParamsSet
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  CPSS_PORT_INTERFACE_MODE_ENT    ifMode,
    IN  CPSS_PORT_SPEED_ENT             speed,
    IN  CPSS_PORT_FEC_MODE_ENT          fecSupported,
    IN  CPSS_PORT_FEC_MODE_ENT          fecRequested
)
{
    CPSS_PM_PORT_PARAMS_STC portParams;
    GT_STATUS rc = GT_OK;

    PRV_CPSS_DEV_CHECK_MAC(devNum);

    portParams.portType = CPSS_PORT_MANAGER_PORT_TYPE_802_3AP_E;
    rc = cpssDxChPortManagerPortParamsStructInit(portParams.portType, &portParams);
    if (rc != GT_OK)
    {
        return rc;
    }

    portParams.portParamsType.apPort.numOfModes++;

    /* configure the first port mode and speed */
    portParams.portParamsType.apPort.modesArr[0].ifMode = ifMode;
    portParams.portParamsType.apPort.modesArr[0].speed = speed;
    portParams.portParamsType.apPort.modesArr[0].fecRequested = fecRequested;
    portParams.portParamsType.apPort.modesArr[0].fecSupported = fecSupported;

    /* checking mandtory params */
    if ((portParams.portParamsType.apPort.modesArr[0].ifMode == CPSS_PORT_INTERFACE_MODE_NA_E) ||
    (portParams.portParamsType.apPort.modesArr[0].speed == CPSS_PORT_SPEED_NA_E))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    rc = cpssDxChPortManagerPortParamsSet(devNum, portNum, &portParams);
    if (rc != GT_OK)
    {
        return rc;
    }

    return GT_OK;
}

/**
* @internal cpssDxChSamplePortManagerApAddMultiSpeedParamsSet
*           function
* @endinternal
*
* @brief   set the multi speed params in the pm database
*
* @param[in] devNum                - device number
* @param[in] portNum               - port number
* @param[in] ifMode                - port mode
* @param[in] speed                 - port speed
* @param[in] fecSupported          - fec supported
* @param[in] fecRequested          - fec requested
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on passing null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* Hint: this is for multi speed set,
* cpssDxChSamplePortManagerApMandatoryParamsSet must be called
* first
*/
GT_STATUS cpssDxChSamplePortManagerApAddMultiSpeedParamsSet
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  CPSS_PORT_INTERFACE_MODE_ENT    ifMode,
    IN  CPSS_PORT_SPEED_ENT             speed,
    IN  CPSS_PORT_FEC_MODE_ENT          fecSupported,
    IN  CPSS_PORT_FEC_MODE_ENT          fecRequested
)
{
    CPSS_PM_PORT_PARAMS_STC portParams;
    GT_STATUS rc = GT_OK;
    GT_U32 numOfModes;

    PRV_CPSS_DEV_CHECK_MAC(devNum);

    rc = cpssDxChPortManagerPortParamsGet(devNum, portNum, &portParams);
    if (rc != GT_OK)
    {
        return rc;
    }

    if(portParams.portType != CPSS_PORT_MANAGER_PORT_TYPE_802_3AP_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    numOfModes = portParams.portParamsType.apPort.numOfModes;
    if (numOfModes == 0)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    if (numOfModes >= CPSS_PM_AP_MAX_MODES)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }

    portParams.portParamsType.apPort.modesArr[numOfModes].ifMode = ifMode;
    portParams.portParamsType.apPort.modesArr[numOfModes].speed = speed;
    portParams.portParamsType.apPort.modesArr[numOfModes].fecRequested = fecRequested;
    portParams.portParamsType.apPort.modesArr[numOfModes].fecSupported = fecSupported;

    portParams.portParamsType.apPort.numOfModes++;

    rc = cpssDxChPortManagerPortParamsSet(devNum, portNum, &portParams);
    if (rc != GT_OK)
    {
        return rc;
    }

    return GT_OK;
}

/**
* @internal cpssDxChSamplePortManagerApAttributeParamsSet
*           function
* @endinternal
*
* @brief   set the mandtory params in the pm database
*
* @param[in] devNum                - device number
* @param[in] portNum               - port number
* @param[in] nonceDisable          - nonceDisable
* @param[in] fcPause               - fcPause
* @param[in] fcAsmDir              - fcAsmDir
* @param[in] negotiationLaneNum    - negotiation lane number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on passing null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChSamplePortManagerApAttributeParamsSet
(
    IN  GT_U8                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                portNum,
    IN  GT_BOOL                             nonceDisable,
    IN  GT_BOOL                             fcPause,
    IN  CPSS_PORT_AP_FLOW_CONTROL_ENT       fcAsmDir,
    IN  GT_U32                              negotiationLaneNum
)
{
    CPSS_PM_PORT_PARAMS_STC portParams;
    GT_STATUS rc = GT_OK;

    PRV_CPSS_DEV_CHECK_MAC(devNum);

    rc = cpssDxChPortManagerPortParamsGet(devNum, portNum, &portParams);
    if (rc != GT_OK)
    {
        return rc;
    }

    if(portParams.portType != CPSS_PORT_MANAGER_PORT_TYPE_802_3AP_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    CPSS_PM_SET_VALID_AP_ATTR(&portParams,CPSS_PM_AP_PORT_ATTR_NONCE_E);
    CPSS_PM_SET_VALID_AP_ATTR(&portParams,CPSS_PM_AP_PORT_ATTR_FC_PAUSE_E);
    CPSS_PM_SET_VALID_AP_ATTR(&portParams,CPSS_PM_AP_PORT_ATTR_FC_ASM_DIR_E);
    CPSS_PM_SET_VALID_AP_ATTR(&portParams,CPSS_PM_AP_PORT_ATTR_LANE_NUM_E);

    portParams.portParamsType.apPort.apAttrs.nonceDisable = nonceDisable;
    portParams.portParamsType.apPort.apAttrs.fcPause = fcPause;
    portParams.portParamsType.apPort.apAttrs.fcAsmDir = fcAsmDir;
    portParams.portParamsType.apPort.apAttrs.negotiationLaneNum = negotiationLaneNum;

    rc = cpssDxChPortManagerPortParamsSet(devNum, portNum, &portParams);
    if (rc != GT_OK)
    {
        return rc;
    }

    return GT_OK;
}

/**
* @internal cpssDxChSamplePortManagerAttrExtOperationParamsSet
*           function
* @endinternal
*
* @brief   Set the port extra operations in the pm database.
*          Supported for ap and non-ap port.
*
* @param[in] devNum                - device number
* @param[in] portNum               - port number
* @param[in] portExtraOperation    - extra operations bitmap
*           supported:
*   MV_HWS_PORT_SERDES_OPERATION_PRECODING_E = 0x1   - port
*   operations serdes precoding
*   MV_HWS_PORT_SERDES_OPERATION_RX_TRAIN_E  = 0x20  - port
*   operations do ap rx train
*   MV_HWS_PORT_SERDES_OPERATION_IS_FIBER_E  = 0x40  - port
*   operations change ap port to fiber
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on passing null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChSamplePortManagerAttrExtOperationParamsSet
(
    IN  GT_U8                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                portNum,
    IN  GT_U32                              portExtraOperation
)
{
    CPSS_PM_PORT_PARAMS_STC portParams;
    GT_STATUS rc = GT_OK;

    PRV_CPSS_DEV_CHECK_MAC(devNum);

    rc = cpssDxChPortManagerPortParamsGet(devNum, portNum, &portParams);
    if (rc != GT_OK)
    {
        return rc;
    }

    if(portParams.portType == CPSS_PORT_MANAGER_PORT_TYPE_802_3AP_E)
    {
        CPSS_PM_SET_VALID_AP_ATTR(&portParams,CPSS_PM_AP_PORT_ATTR_OPERATIONS_E);
        portParams.portParamsType.apPort.apAttrs.portExtraOperation = portExtraOperation;
    }
    else if(portParams.portType == CPSS_PORT_MANAGER_PORT_TYPE_REGULAR_E)
    {
        CPSS_PM_SET_VALID_ATTR(&portParams,CPSS_PM_ATTR_PORT_OPERATIONS_E);
        portParams.portParamsType.regPort.portAttributes.portExtraOperation = portExtraOperation;
    }
    else
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    rc = cpssDxChPortManagerPortParamsSet(devNum, portNum, &portParams);
    if (rc != GT_OK)
    {
        return rc;
    }

    return GT_OK;
}

/**
* @internal cpssDxChSamplePortManagerApOverrideTxLaneParamsSet
*           function
* @endinternal
*
* @brief   set the tx params in the pm database
*
* @param[in] devNum                - device number
* @param[in] portNum               - port number
* @param[in] laneNum               - lane number
* @param[in] laneSpeed             - lane speed
* @param[in] portParamsInputStc    - structe with attributes
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on passing null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChSamplePortManagerApOverrideTxLaneParamsSet
(
    IN  GT_U8                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                portNum,
    IN  GT_U32                              laneNum,
    IN  CPSS_PM_AP_LANE_SERDES_SPEED        laneSpeed,
    IN  CPSS_PORT_SERDES_TX_CONFIG_STC      *portParamsInputStcPtr
)
{
    CPSS_PM_PORT_PARAMS_STC portParams;
    GT_STATUS rc = GT_OK;

    PRV_CPSS_DEV_CHECK_MAC(devNum);

    rc = cpssDxChPortManagerPortParamsGet(devNum, portNum, &portParams);
    if (rc != GT_OK)
    {
        return rc;
    }

    if(portParams.portType != CPSS_PORT_MANAGER_PORT_TYPE_802_3AP_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    /* checking mandtory params */
    if ((portParams.portParamsType.apPort.modesArr[0].ifMode == CPSS_PORT_INTERFACE_MODE_NA_E) ||
        (portParams.portParamsType.apPort.modesArr[0].speed == CPSS_PORT_SPEED_NA_E))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    if ((CPSS_PM_MAX_LANES <= laneNum) || (CPSS_PM_AP_LANE_SERDES_SPEED_NONE_E <= laneSpeed))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }

    CPSS_PM_SET_VALID_AP_ATTR(&portParams,CPSS_PM_AP_PORT_ATTR_LANE_OVR_PARAMS_E);

    CPSS_PM_SET_VALID_AP_ATTR_OVERRIDE(&portParams, laneSpeed, laneNum, CPSS_PM_AP_LANE_OVERRIDE_TX_OFFSET_E);

    /*The place to change in order to support additional devices or serdes types*/
    if ((PRV_CPSS_SIP_6_CHECK_MAC(devNum) && !(PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))) || (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_AC5_E))
    {
        portParams.portParamsType.apPort.apAttrs.overrideLaneParams[laneSpeed][laneNum].txParams.type = portParamsInputStcPtr->type;

        switch (portParamsInputStcPtr->type)
        {
            case CPSS_PORT_SERDES_AVAGO_E:
                cpssOsMemCpy(&(portParams.portParamsType.apPort.apAttrs.overrideLaneParams[laneSpeed][laneNum].txParams.txTune.avago),
                             &(portParamsInputStcPtr->txTune.avago),
                             sizeof(CPSS_PORT_AVAGO_SERDES_TX_CONFIG_STC));
                break;
            case CPSS_PORT_SERDES_COMPHY_C12G_E:
                cpssOsMemCpy(&(portParams.portParamsType.apPort.apAttrs.overrideLaneParams[laneSpeed][laneNum].txParams.txTune.comphy_C12G),
                             &(portParamsInputStcPtr->txTune.comphy_C12G),
                             sizeof(CPSS_PORT_COMPHY_C12G_TX_CONFIG_STC));
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
        }
    }
    else
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }

    rc = cpssDxChPortManagerPortParamsSet(devNum, portNum, &portParams);
    if (rc != GT_OK)
    {
        return rc;
    }

    return GT_OK;
}

/**
* @internal cpssDxChSamplePortManagerApOverrideRxLaneParamsSet
*           function
* @endinternal
*
* @brief   set the rx params in the pm database
*
* @param[in] devNum                - device number
* @param[in] portNum               - port number
* @param[in] laneNum               - lane number
* @param[in] laneSpeed             - lane speed
* @param[in] portParamsInputStc    - structe with attributes
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on passing null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChSamplePortManagerApOverrideRxLaneParamsSet
(
    IN  GT_U8                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                portNum,
    IN  GT_U32                              laneNum,
    IN  CPSS_PM_AP_LANE_SERDES_SPEED        laneSpeed,
    IN  CPSS_PORT_SERDES_RX_CONFIG_STC      *portParamsInputStcPtr
)
{
    CPSS_PM_PORT_PARAMS_STC portParams;
    GT_STATUS rc = GT_OK;

    PRV_CPSS_DEV_CHECK_MAC(devNum);

    rc = cpssDxChPortManagerPortParamsGet(devNum, portNum, &portParams);
    if (rc != GT_OK)
    {
        return rc;
    }

    if(portParams.portType != CPSS_PORT_MANAGER_PORT_TYPE_802_3AP_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    /* checking mandtory params */
    if ((portParams.portParamsType.apPort.modesArr[0].ifMode == CPSS_PORT_INTERFACE_MODE_NA_E) ||
        (portParams.portParamsType.apPort.modesArr[0].speed == CPSS_PORT_SPEED_NA_E))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    if ((CPSS_PM_MAX_LANES <= laneNum) || (CPSS_PM_AP_LANE_SERDES_SPEED_NONE_E <= laneSpeed))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }

    CPSS_PM_SET_VALID_AP_ATTR(&portParams,CPSS_PM_AP_PORT_ATTR_LANE_OVR_PARAMS_E);

    CPSS_PM_SET_VALID_AP_ATTR_OVERRIDE(&portParams, laneSpeed, laneNum, CPSS_PM_AP_LANE_OVERRIDE_RX_E);

    /*The place to change in order to support additional devices or serdes types*/
    if ((PRV_CPSS_SIP_6_CHECK_MAC(devNum) && !(PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))) || (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_AC5_E))
    {
        portParams.portParamsType.apPort.apAttrs.overrideLaneParams[laneSpeed][laneNum].rxParams.type = portParamsInputStcPtr->type;

        switch (portParamsInputStcPtr->type)
        {
            case CPSS_PORT_SERDES_AVAGO_E:
                cpssOsMemCpy(&(portParams.portParamsType.apPort.apAttrs.overrideLaneParams[laneSpeed][laneNum].rxParams.rxTune.avago),
                             &(portParamsInputStcPtr->rxTune.avago),
                             sizeof(CPSS_PORT_AVAGO_SERDES_RX_CONFIG_STC));
                break;
            case CPSS_PORT_SERDES_COMPHY_C12G_E:
                cpssOsMemCpy(&(portParams.portParamsType.apPort.apAttrs.overrideLaneParams[laneSpeed][laneNum].rxParams.rxTune.comphy_C12G),
                             &(portParamsInputStcPtr->rxTune.comphy_C12G),
                             sizeof(CPSS_PORT_COMPHY_C12GP41P2V_RX_CONFIG_STC));
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
        }
    }
    else
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }

    rc = cpssDxChPortManagerPortParamsSet(devNum, portNum, &portParams);
    if (rc != GT_OK)
    {
        return rc;
    }

    return GT_OK;
}

/**
* @internal
*           cpssDxChSamplePortManagerApOverrideInterconnectProfileSet
*           function
* @endinternal
*
* @brief   set the interconnect profile in the pm
*          database
*
* @param[in] devNum                - device number
* @param[in] portNum               - port number
* @param[in] interconnectProfile    - interconnect profile
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on passing null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChSamplePortManagerApOverrideInterconnectProfileSet
(
    IN  GT_U8                                       devNum,
    IN  GT_PHYSICAL_PORT_NUM                        portNum,
    IN  CPSS_PORT_MANAGER_INTERCONNECT_PROFILE_ENT  interconnectProfile
)
{
    CPSS_PM_PORT_PARAMS_STC portParams;
    GT_STATUS rc = GT_OK;

    PRV_CPSS_DEV_CHECK_MAC(devNum);

    rc = cpssDxChPortManagerPortParamsGet(devNum, portNum, &portParams);
    if (rc != GT_OK)
    {
        return rc;
    }

    if(portParams.portType != CPSS_PORT_MANAGER_PORT_TYPE_802_3AP_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    /* checking mandtory params */
    if ((portParams.portParamsType.apPort.modesArr[0].ifMode == CPSS_PORT_INTERFACE_MODE_NA_E) ||
        (portParams.portParamsType.apPort.modesArr[0].speed == CPSS_PORT_SPEED_NA_E))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    CPSS_PM_SET_VALID_AP_ATTR(&portParams,CPSS_PM_AP_PORT_ATTR_INTERCONNECT_PROFILE_E);

    if (PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        portParams.portParamsType.apPort.apAttrs.interconnectProfile = interconnectProfile;
    }
    else
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    rc = cpssDxChPortManagerPortParamsSet(devNum, portNum, &portParams);
    if (rc != GT_OK)
    {
        return rc;
    }

    return GT_OK;
}


/**
* @internal cpssDxChSamplePortManagerUpd function
* @endinternal
*
* @brief   update port manager params without deleting the port
*
* @param[in] devNum                - device number
* @param[in] portNum               - port number
* @param[in] bwMode                - bw mode to update
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on passing null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChSamplePortManagerUpd
(
    IN  GT_U8                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                portNum,
    IN  CPSS_PORT_PA_BW_MODE_ENT            bwMode
)
{
    GT_STATUS rc;
    CPSS_PORT_MANAGER_UPDATE_PARAMS_STC updateParamsStc;

    PRV_CPSS_DEV_CHECK_MAC(devNum);

    updateParamsStc.updateParamsBitmapType = CPSS_PORT_MANAGER_UPDATE_PARAMS_BW_TYPE_E;
    updateParamsStc.bwMode = bwMode;

    rc = cpssDxChPortManagerPortParamsUpdate(devNum, portNum, &updateParamsStc);
    if (rc!=GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "calling cpssDxChPortManagerPortParamsUpdate returned=%d, portNum=%d", rc, portNum);
    }

    return GT_OK;
}

/**
* @internal cpssDxChSamplePortManagerApSkipResSet function
* @endinternal
*
* @brief   Instruct Port Manager to run TRX training on a port
*          without the need to run the AN part of AP protocol
*          a.k.a. Skip resolution part of AP
*
* @param[in] devNum                - device number
* @param[in] portNum               - port number
* @param[in] skipRes               - skipRes
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on passing null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChSamplePortManagerApSkipResSet
(
    IN  GT_U8                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                portNum,
    IN  GT_BOOL                             skipRes
)
{
    CPSS_PM_PORT_PARAMS_STC portParams;
    GT_STATUS rc = GT_OK;

    PRV_CPSS_DEV_CHECK_MAC(devNum);

    rc = cpssDxChPortManagerPortParamsGet(devNum, portNum, &portParams);
    if (rc != GT_OK)
    {
        return rc;
    }

    if(portParams.portType != CPSS_PORT_MANAGER_PORT_TYPE_802_3AP_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    /* checking mandtory params */
    if ((portParams.portParamsType.apPort.modesArr[0].ifMode == CPSS_PORT_INTERFACE_MODE_NA_E) ||
        (portParams.portParamsType.apPort.modesArr[0].speed == CPSS_PORT_SPEED_NA_E))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    if (PRV_CPSS_SIP_6_CHECK_MAC(devNum) && !(PRV_CPSS_SIP_6_10_CHECK_MAC(devNum)))
    {
        CPSS_PM_SET_VALID_AP_ATTR(&portParams,CPSS_PM_AP_PORT_ATTR_SKIP_RES_E);
        portParams.portParamsType.apPort.apAttrs.skipRes = skipRes;
    }
    else
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, LOG_ERROR_NO_MSG);
    }

    rc = cpssDxChPortManagerPortParamsSet(devNum, portNum, &portParams);
    if (rc != GT_OK)
    {
        return rc;
    }

    return GT_OK;
}

/**
* @internal cpssDxChSamplePortManagerApParallelDetectSet function
* @endinternal
*
* @brief   set Parallel Detect parameters
*
* @param[in] devNum                - device number
* @param[in] portNum               - port number
* @param[in] restarMaxtNum         - restartMaxNum
* @param[in] enable                - enable
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on passing null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChSamplePortManagerApParallelDetectSet
(
    IN  GT_U8                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                portNum,
    IN  GT_U8                               restartMaxNum,
    IN  GT_BOOL                             enable
)
{
    CPSS_PM_PORT_PARAMS_STC portParams;
    GT_STATUS rc = GT_OK;

    PRV_CPSS_DEV_CHECK_MAC(devNum);

    rc = cpssDxChPortManagerPortParamsGet(devNum, portNum, &portParams);
    if (rc != GT_OK)
    {
        return rc;
    }

    if(portParams.portType != CPSS_PORT_MANAGER_PORT_TYPE_802_3AP_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    /* checking mandtory params */
    if ((portParams.portParamsType.apPort.modesArr[0].ifMode == CPSS_PORT_INTERFACE_MODE_NA_E) ||
        (portParams.portParamsType.apPort.modesArr[0].speed == CPSS_PORT_SPEED_NA_E))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    if (PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
    {
        CPSS_PM_SET_VALID_AP_ATTR(&portParams,CPSS_PM_AP_PORT_ATTR_PARALLEL_DETECT_E);
        portParams.portParamsType.apPort.apAttrs.parallelDetect.restartMaxNum = restartMaxNum;
        portParams.portParamsType.apPort.apAttrs.parallelDetect.enable = enable;
    }
    else
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, LOG_ERROR_NO_MSG);
    }

    rc = cpssDxChPortManagerPortParamsSet(devNum, portNum, &portParams);
    if (rc != GT_OK)
    {
        return rc;
    }

    return GT_OK;
}
