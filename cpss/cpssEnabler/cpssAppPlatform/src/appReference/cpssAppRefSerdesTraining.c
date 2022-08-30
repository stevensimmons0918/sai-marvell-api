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
* @file cpssAppRefSerdesTraining.c
*
* @brief this file contains TRX and RX serdes training task implementations
*
* @version   0
********************************************************************************
*/
#include <profiles/cpssAppPlatformProfile.h>
#include <cpssAppPlatformSysConfig.h>
#include <cpssAppPlatformRunTimeConfig.h>
#include <cpssAppPlatformPortInit.h>

#include <cpss/dxCh/dxChxGen/diag/cpssDxChDiagDataIntegrity.h>
#include <cpss/dxCh/dxChxGen/networkIf/cpssDxChNetIf.h>
#include <cpss/dxCh/dxChxGen/networkIf/cpssDxChNetIfMii.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortAp.h>
#include <cpss/dxCh/dxChxGen/vnt/cpssDxChVnt.h>
#include <cpss/dxCh/dxChxGen/port/private/prvCpssDxChPortCtrl.h>

#include <extUtils/rxEventHandler/rxEventHandler.h>
#include <extUtils/auEventHandler/auEventHandler.h>

#include <appReference/cpssAppRefUtils.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

CPSS_OS_SIG_SEM    serdesTuneSmId[CPSS_APP_PLATFORM_MAX_PP_CNS] = { 0 };
CPSS_PORTS_BMP_STC todoTuneBmp[CPSS_APP_PLATFORM_MAX_PP_CNS]; /* bitmap of ports to run tuning*/
GT_U32             serdesTuneTaskId[CPSS_APP_PLATFORM_MAX_PP_CNS] = { 0 };
GT_BOOL            serdesTaskReset[CPSS_APP_PLATFORM_MAX_PP_CNS] = { 0 };

GT_BOOL trxTraining = 0;
static CPSS_OS_SIG_SEM    trxTuneSmId[CPSS_APP_PLATFORM_MAX_PP_CNS] = { 0 };
static GT_U32             trxTuneTaskId[CPSS_APP_PLATFORM_MAX_PP_CNS] = { 0 };
static CPSS_PORTS_BMP_STC trxTodoTuneBmp[CPSS_APP_PLATFORM_MAX_PP_CNS]; /* bitmap of ports to run TRX training*/
static CPSS_OS_SIG_SEM    align90SmId[CPSS_APP_PLATFORM_MAX_PP_CNS] = { 0 };
static GT_U32             align90TaskId[CPSS_APP_PLATFORM_MAX_PP_CNS] = { 0 };
static CPSS_PORTS_BMP_STC align90TodoBmp[CPSS_APP_PLATFORM_MAX_PP_CNS]; /* bitmap of ports to run TRX training*/

GT_BOOL rxTraining = 0;
static CPSS_OS_SIG_SEM    rxTuneSmId[CPSS_APP_PLATFORM_MAX_PP_CNS] = { 0 };
static GT_U32             rxTuneTaskId[CPSS_APP_PLATFORM_MAX_PP_CNS] = { 0 };
static CPSS_PORTS_BMP_STC rxTodoTuneBmp[CPSS_APP_PLATFORM_MAX_PP_CNS]; /* bitmap of ports to run RX training*/

GT_BOOL  changeToDownHappend[CPSS_APP_PLATFORM_MAX_PP_CNS][CPSS_MAX_PORTS_NUM_CNS] = {{ 0 }};
GT_BOOL  changeToUpHappend[CPSS_APP_PLATFORM_MAX_PP_CNS][CPSS_MAX_PORTS_NUM_CNS] = {{ 0 }};

static GT_U32 trxTimeoutArray[CPSS_APP_PLATFORM_MAX_PP_CNS][CPSS_MAX_PORTS_NUM_CNS] = {{ 0 }};
static GT_U32 trxTrainingRetryCntArray[CPSS_APP_PLATFORM_MAX_PP_CNS][CPSS_MAX_PORTS_NUM_CNS] = {{ 0 }};
static GT_U32 al90TimeoutArray[CPSS_APP_PLATFORM_MAX_PP_CNS][CPSS_MAX_PORTS_NUM_CNS] = {{ 0 }};

/* for now maximal number of serdes per port where align90 could run is 4 */
static CPSS_DXCH_PORT_ALIGN90_PARAMS_STC serdesParams[CPSS_APP_PLATFORM_MAX_PP_CNS][CPSS_MAX_PORTS_NUM_CNS][4];

/**
* @internal waTriggerInterruptsMaskNoLinkStatusChangeInterrupt function
* @endinternal
*
* @brief   Mask/unmask specific interrupts on port that can trigger WA's restart
*         or other not needed for now actions.
*         This function is almost the same as waTriggerInterruptsMask but uses
*         When it is no signal to not unmask CPSS_PP_PORT_LINK_STATUS_CHANGED_E interrupt
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] ifMode                   - current interface on port
* @param[in] operation                - mask/unmask
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*
* @note Affected interrupts: CPSS_PP_PORT_LANE_SIGNAL_DETECT_CHANGED_E,
*       CPSS_PP_PORT_MMPCS_SIGNAL_DETECT_CHANGE_E.
*
*/
GT_STATUS waTriggerInterruptsMaskNoLinkStatusChangeInterrupt
(
    IN  GT_U8                       devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    CPSS_PORT_INTERFACE_MODE_ENT    ifMode,
    IN  CPSS_EVENT_MASK_SET_ENT     operation
)
{
    GT_STATUS           rc = GT_OK;
    GT_U32              laneNum,
                        lanesCount;
    CPSS_PORT_SPEED_ENT speed;

    if((CPSS_PP_FAMILY_DXCH_LION2_E == PRV_CPSS_PP_MAC(devNum)->devFamily) || ((CPSS_PORT_INTERFACE_MODE_RXAUI_E != ifMode) &&
        (CPSS_PORT_INTERFACE_MODE_XGMII_E != ifMode)))
    {
        if((ifMode != CPSS_PORT_INTERFACE_MODE_XGMII_E)
           || (operation != CPSS_EVENT_UNMASK_E))
        {
            rc = cpssDxChPortSpeedGet(devNum, portNum, &speed);
            CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChPortSpeedGet);

            if((CPSS_PORT_INTERFACE_MODE_RXAUI_E == ifMode)
               || (CPSS_PORT_INTERFACE_MODE_XGMII_E == ifMode))
            {
                GT_BOOL supported;

                rc = cpssDxChPortInterfaceSpeedSupportGet(devNum, portNum, ifMode,
                                                          CPSS_PORT_SPEED_10000_E,
                                                          &supported);
                CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChPortInterfaceSpeedSupportGet);

                if(supported)
                {
                    lanesCount = (CPSS_PORT_INTERFACE_MODE_RXAUI_E == ifMode) ? 2 : 6;
                    for(laneNum = 0; laneNum < lanesCount; laneNum++)
                    {
                        CPSS_APP_PLATFORM_LOG_DBG_MAC("cpssEventDeviceMaskWithEvExtDataSet(LANE_SIGNAL_DETECT_CHANGED,((portNum(%d)<<8)|laneNum(%d))=0x%x,op=%d\n", portNum, laneNum, ((portNum<<8)|laneNum), operation);
                        rc = cpssEventDeviceMaskWithEvExtDataSet(devNum,
                                                                 CPSS_PP_PORT_LANE_SYNC_STATUS_CHANGED_E,
                                                                 ((portNum<<8)|laneNum),
                                                                 operation);
                        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssEventDeviceMaskWithEvExtDataSet);
                    }

                    rc = cpssEventDeviceMaskWithEvExtDataSet(devNum,
                                                             CPSS_PP_PORT_MMPCS_SIGNAL_DETECT_CHANGE_E,
                                                             portNum,
                                                             CPSS_EVENT_MASK_E);
                    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssEventDeviceMaskWithEvExtDataSet);
                }
            }
            else if(   (CPSS_PORT_INTERFACE_MODE_KR_E == ifMode && CPSS_PORT_SPEED_5000_E != speed)
                    || (CPSS_PORT_INTERFACE_MODE_KR2_E == ifMode)
                    || (CPSS_PORT_INTERFACE_MODE_KR4_E == ifMode)
                    || (CPSS_PORT_INTERFACE_MODE_CR_E == ifMode)
                    || (CPSS_PORT_INTERFACE_MODE_CR2_E == ifMode)
                    || (CPSS_PORT_INTERFACE_MODE_CR4_E == ifMode)
                    || (CPSS_PORT_INTERFACE_MODE_SR_LR_E == ifMode)
                    || (CPSS_PORT_INTERFACE_MODE_XHGS_E == ifMode)
                    || (CPSS_PORT_INTERFACE_MODE_XHGS_SR_E == ifMode))

            {
                rc = cpssEventDeviceMaskWithEvExtDataSet(devNum,
                                                         CPSS_PP_PORT_MMPCS_SIGNAL_DETECT_CHANGE_E,
                                                         portNum,
                                                         operation);
                CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssEventDeviceMaskWithEvExtDataSet);
            }
        }
    }

    return rc;
}

/**
* @internal waTriggerInterruptsMask function
* @endinternal
*
* @brief   Mask/unmask specific interrupts on port that can trigger WA's restart
*         or other not needed for now actions.
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] ifMode                   - current interface on port
* @param[in] operation                - mask/unmask
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*
* @note Affected interrupts: CPSS_PP_PORT_LANE_SIGNAL_DETECT_CHANGED_E,
*       CPSS_PP_PORT_LINK_STATUS_CHANGED_E,
*       CPSS_PP_PORT_MMPCS_SIGNAL_DETECT_CHANGE_E.
*
*/
GT_STATUS waTriggerInterruptsMask
(
    IN  GT_U8                       devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    CPSS_PORT_INTERFACE_MODE_ENT    ifMode,
    IN  CPSS_EVENT_MASK_SET_ENT     operation
)
{
    GT_STATUS rc = GT_OK;

    rc = waTriggerInterruptsMaskNoLinkStatusChangeInterrupt(devNum,  portNum, ifMode, operation);
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, waTriggerInterruptsMaskNoLinkStatusChangeInterrupt);

    rc = cpssEventDeviceMaskWithEvExtDataSet(devNum,
                                             CPSS_PP_PORT_LINK_STATUS_CHANGED_E,
                                             portNum,
                                             operation);
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssEventDeviceMaskWithEvExtDataSet);

    return rc;
}

/**
* @internal cpssAppRefDxChTrxTrainingRun function
* @endinternal
*
* @brief   Start TRX training on given port
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - global number of port where to run TRX training
* @param[in] interface                - port  mode
*
* @retval GT_OK                    - on success
*/
static GT_STATUS cpssAppRefDxChTrxTrainingRun
(
    IN  GT_U8                        devNum,
    IN  GT_PHYSICAL_PORT_NUM         portNum,
    IN  CPSS_PORT_INTERFACE_MODE_ENT interface
)
{
    GT_STATUS rc = GT_OK;

    CPSS_APP_PLATFORM_LOG_DBG_MAC("cpssAppRefDxChTrxTrainingRun:portNum=%d,interface=%d\n",
                                                                    portNum, interface);

    if(CPSS_PORTS_BMP_IS_PORT_SET_MAC(&trxTodoTuneBmp[devNum], portNum)
        || CPSS_PORTS_BMP_IS_PORT_SET_MAC(&align90TodoBmp[devNum], portNum))
    {
        return GT_OK;
    }

    CPSS_APP_PLATFORM_LOG_DBG_MAC("cpssAppRefDxChTrxTrainingRun:changeToDownHappend[%d][%d]=%d\n",
                                                   devNum, portNum, changeToDownHappend[devNum][portNum]);
    if(!changeToDownHappend[devNum][portNum])
    {
        changeToDownHappend[devNum][portNum] = GT_TRUE;
        return GT_OK;
    }
    else
    {
        changeToDownHappend[devNum][portNum] = GT_FALSE;
    }

    rc = waTriggerInterruptsMask(devNum, portNum, interface, CPSS_EVENT_MASK_E);
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, waTriggerInterruptsMask);

    CPSS_APP_PLATFORM_LOG_INFO_MAC("SERDES TRx training - port %d\n",portNum);

    rc = cpssDxChPortSerdesAutoTune(devNum, portNum, CPSS_DXCH_PORT_SERDES_AUTO_TUNE_MODE_TX_TRAINING_CFG_E);
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChPortSerdesAutoTune);

    rc = cpssDxChPortSerdesAutoTune(devNum, portNum, CPSS_DXCH_PORT_SERDES_AUTO_TUNE_MODE_TX_TRAINING_START_E);
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChPortSerdesAutoTune);

    CPSS_PORTS_BMP_PORT_SET_MAC(&trxTodoTuneBmp[devNum], portNum);
    rc = cpssOsSigSemSignal(trxTuneSmId[devNum]);
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssOsSigSemSignal);

    return rc;
}

/*******************************************************************************
* trxTrainingTask
*
* DESCRIPTION:
*       Perform continuous polling of ports where TRX training started.
*
* INPUTS:
*       param                    - device number
*
* RETURNS:
*       GT_BAD_PARAM             - wrong devNum, portNum
*       GT_FAIL                  - on error
*       GT_BAD_PTR               - one of the parameters is NULL pointer
*       GT_HW_ERROR              - on hardware error
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*******************************************************************************/
static unsigned __TASKCONV trxTrainingTask
(
    GT_VOID *param
)
{
    GT_STATUS  rc = GT_OK;
    GT_U8      portNum;
    GT_U8      devNum;
    GT_UINTPTR tmp;
    CPSS_DXCH_PORT_SERDES_AUTO_TUNE_STATUS_ENT rxTuneStatus;
    CPSS_DXCH_PORT_SERDES_AUTO_TUNE_STATUS_ENT txTuneStatus;
    CPSS_PORT_INTERFACE_MODE_ENT    interface;

    /* to avoid warning: cast from pointer to integer of different size */
    tmp = (GT_UINTPTR)param;
    devNum = (GT_U8)tmp;

    while(1)
    {
        cpssOsSigSemWait(trxTuneSmId[devNum], 0);
        if(serdesTaskReset[devNum])
        {
            break;
        }

        for(portNum = 0; GT_FALSE == CPSS_PORTS_BMP_IS_ZERO_MAC(&trxTodoTuneBmp[devNum]);
                                portNum = (portNum+1)%CPSS_MAX_PORTS_NUM_CNS)
        {
            PRV_CPSS_DXCH_SKIP_NOT_EXIST_PORT_MAC(devNum, portNum);

            if(CPSS_CPU_PORT_NUM_CNS == portNum)
            {
                continue;
            }

            if(0 == CPSS_PORTS_BMP_IS_PORT_SET_MAC(&trxTodoTuneBmp[devNum], portNum))
            {
                continue;
            }

            rc = cpssDxChPortSerdesAutoTuneStatusGet(devNum,portNum,&rxTuneStatus,&txTuneStatus);
            if(rc != GT_OK)
            {
                CPSS_APP_PLATFORM_LOG_ERR_MAC("cpssDxChPortSerdesAutoTuneStatusGet(portNum=%d):rc=%d\n", portNum, rc);
                continue;
            }

            switch(txTuneStatus)
            {
                case CPSS_DXCH_PORT_SERDES_AUTO_TUNE_FAIL_E:
                case CPSS_DXCH_PORT_SERDES_AUTO_TUNE_PASS_E:
                    CPSS_APP_PLATFORM_LOG_DBG_MAC("port TRX training-%s:portNum=%d\n",
                                 (CPSS_DXCH_PORT_SERDES_AUTO_TUNE_PASS_E ==
                                  txTuneStatus) ? "PASS" : "FAIL", portNum);
                    CPSS_PORTS_BMP_PORT_CLEAR_MAC(&trxTodoTuneBmp[devNum], portNum);
                    trxTimeoutArray[devNum][portNum] = 0;
                    rc = cpssDxChPortSerdesAutoTune(devNum, portNum, CPSS_DXCH_PORT_SERDES_AUTO_TUNE_MODE_TX_TRAINING_STOP_E);
                    if(rc != GT_OK)
                    {
                        CPSS_APP_PLATFORM_LOG_ERR_MAC("cpssDxChPortSerdesAutoTune(TRX_STOP,portNum=%d,txTuneStatus=%d):rc=%d\n",
                                        portNum, txTuneStatus, rc);
                        continue;
                    }

                    rc = cpssDxChPortInterfaceModeGet(devNum,portNum,&interface);
                    if (rc != GT_OK)
                    {
                        CPSS_APP_PLATFORM_LOG_ERR_MAC("cpssDxChPortInterfaceModeGet(portNum=%d):rc=%d\n",
                                        portNum, rc);
                        continue;
                    }

                    if(CPSS_DXCH_PORT_SERDES_AUTO_TUNE_FAIL_E == txTuneStatus)
                    {
                        CPSS_APP_PLATFORM_LOG_DBG_MAC("TRX fail:port=%d,try=%d\n", portNum, trxTrainingRetryCntArray[devNum][portNum]);
                        if(++(trxTrainingRetryCntArray[devNum][portNum]) <= 5)
                        {
                            changeToDownHappend[devNum][portNum] = GT_TRUE;
                            (GT_VOID)cpssAppRefDxChTrxTrainingRun(devNum, portNum, interface);
                            continue;
                        }
                        else
                        {
                            rc = waTriggerInterruptsMask(devNum, portNum, interface, CPSS_EVENT_UNMASK_E);
                            if(rc != GT_OK)
                            {
                                CPSS_APP_PLATFORM_LOG_ERR_MAC("waTriggerInterruptsMask(portNum=%d,unmask):rc=%d\n",
                                                portNum, rc);
                            }
                        }
                    }

                    break;

                case CPSS_DXCH_PORT_SERDES_AUTO_TUNE_NOT_COMPLITED_E:
                    if((trxTimeoutArray[devNum][portNum]+1) > 30)
                    {
                        CPSS_APP_PLATFORM_LOG_DBG_MAC("port TRX training-TIMEOUT:portNum=%d\n", portNum);
                        CPSS_PORTS_BMP_PORT_CLEAR_MAC(&trxTodoTuneBmp[devNum], portNum);
                        trxTimeoutArray[devNum][portNum] = 0;
                        rc = cpssDxChPortSerdesAutoTune(devNum, portNum, CPSS_DXCH_PORT_SERDES_AUTO_TUNE_MODE_TX_TRAINING_STOP_E);
                        if(rc != GT_OK)
                        {
                            CPSS_APP_PLATFORM_LOG_ERR_MAC("cpssDxChPortSerdesAutoTune(TRX_STOP on timeout,portNum=%d):rc=%d\n",
                                            portNum, rc);
                            continue;
                        }

                        rc = cpssDxChPortInterfaceModeGet(devNum,portNum,&interface);
                        if (rc != GT_OK)
                        {
                            CPSS_APP_PLATFORM_LOG_ERR_MAC("cpssDxChPortInterfaceModeGet(portNum=%d):rc=%d\n", portNum, rc);
                            continue;
                        }

                        CPSS_APP_PLATFORM_LOG_DBG_MAC("TRX timedout:port=%d,try=%d\n", portNum, trxTrainingRetryCntArray[devNum][portNum]);
                        if(++(trxTrainingRetryCntArray[devNum][portNum]) <= 5)
                        {
                            changeToDownHappend[devNum][portNum] = GT_TRUE;
                            (GT_VOID)cpssAppRefDxChTrxTrainingRun(devNum, portNum, interface);
                            continue;
                        }
                        else
                        {
                            rc = waTriggerInterruptsMask(devNum, portNum, interface, CPSS_EVENT_UNMASK_E);
                            if(rc != GT_OK)
                            {
                                CPSS_APP_PLATFORM_LOG_ERR_MAC("waTriggerInterruptsMask(portNum=%d,unmask):rc=%d\n",
                                                portNum, rc);
                            }
                        }
                    }
                    else
                    {
                        CPSS_APP_PLATFORM_LOG_DBG_MAC("TRX wait:port=%d,try=%d\n", portNum, trxTrainingRetryCntArray[devNum][portNum]);
                        trxTimeoutArray[devNum][portNum]++;
                        cpssOsTimerWkAfter(32);
                        continue;
                    }

                    break;

                default:
                    CPSS_APP_PLATFORM_LOG_ERR_MAC("AutoTuneStatus-bad state:portNum=%d,txTuneStatus=%d\n", portNum, txTuneStatus);
                    break;

            } /* switch(txTuneStatus) */

            /* continue to align90 even if training failed because unreset PCS etc.
                done only in align90 */
            trxTrainingRetryCntArray[devNum][portNum] = 0;

            rc = cpssDxChPortSerdesAlign90StateSet(devNum, portNum, GT_TRUE,
                (CPSS_DXCH_PORT_ALIGN90_PARAMS_STC*)&(serdesParams[devNum][portNum]));
            if(rc != GT_OK)
            {
                CPSS_APP_PLATFORM_LOG_ERR_MAC("cpssDxChPortSerdesAlign90StateSet:rc=%d,portNum=%d\n",
                                rc, portNum);
            }
            else
            {
                CPSS_PORTS_BMP_PORT_SET_MAC(&align90TodoBmp[devNum], portNum);
                CPSS_APP_PLATFORM_LOG_DBG_MAC("Set port=%d in align90TodoBmp\n", portNum);
                rc = cpssOsSigSemSignal(align90SmId[devNum]);
                if(rc != GT_OK)
                {
                    CPSS_APP_PLATFORM_LOG_ERR_MAC("cpssOsSigSemSignal:rc=%d\n", rc);
                }
            }

        }/* for(portNum) */

    }/* while(1)*/

    return 0;
}

/*******************************************************************************
* align90WaTask
*
* DESCRIPTION:
*       Perform continuous polling of ports where Align90 optimization started.
*
* INPUTS:
*       param                    - device number
*
* RETURNS:
*       GT_BAD_PARAM             - wrong devNum, portNum
*       GT_FAIL                  - on error
*       GT_BAD_PTR               - one of the parameters is NULL pointer
*       GT_HW_ERROR              - on hardware error
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*******************************************************************************/
static unsigned __TASKCONV align90WaTask
(
    GT_VOID * param
)
{
    GT_STATUS  rc = GT_OK;
    GT_U8      portNum;
    GT_U8      devNum;
    GT_UINTPTR tmp;
    GT_BOOL    stopAlign90;
    CPSS_DXCH_PORT_SERDES_AUTO_TUNE_STATUS_ENT align90Status;

    /* to avoid warning: cast from pointer to integer of different size */
    tmp = (GT_UINTPTR)param;
    devNum = (GT_U8)tmp;

    while(1)
    {
        cpssOsSigSemWait(align90SmId[devNum], 0);
        if(serdesTaskReset[devNum])
        {
            break;
        }

        for(portNum = 0; GT_FALSE == CPSS_PORTS_BMP_IS_ZERO_MAC(&align90TodoBmp[devNum]);
                                portNum = (portNum+1)%CPSS_MAX_PORTS_NUM_CNS)
        {
            PRV_CPSS_DXCH_SKIP_NOT_EXIST_PORT_MAC(devNum, portNum);

            if(CPSS_CPU_PORT_NUM_CNS == portNum)
            {
                continue;
            }

            if(0 == CPSS_PORTS_BMP_IS_PORT_SET_MAC(&align90TodoBmp[devNum], portNum))
            {
                continue;
            }

            stopAlign90 = GT_TRUE;
            rc = cpssDxChPortSerdesAlign90StatusGet(devNum, portNum, &align90Status);
            if(rc != GT_OK)
            {/* if error in execution */
                CPSS_APP_PLATFORM_LOG_ERR_MAC("cpssDxChPortSerdesAlign90StatusGet(portNum=%d):rc=%d\n", portNum, rc);
            }
            else if(((al90TimeoutArray[devNum][portNum]+1) > 14) &&
                (CPSS_DXCH_PORT_SERDES_AUTO_TUNE_NOT_COMPLITED_E == align90Status))
            {/* if timeout*/
                CPSS_APP_PLATFORM_LOG_DBG_MAC("Align90-timeout:portNum=%d\n", portNum);
            }
            else if(CPSS_DXCH_PORT_SERDES_AUTO_TUNE_FAIL_E == align90Status)
            {/* if failed */
                CPSS_APP_PLATFORM_LOG_DBG_MAC("Align90-fail:portNum=%d\n", portNum);
            }
            else if(CPSS_DXCH_PORT_SERDES_AUTO_TUNE_PASS_E == align90Status)
            {
                CPSS_APP_PLATFORM_LOG_DBG_MAC("Align90-OK:portNum=%d\n", portNum);
            }
            else
            {
                al90TimeoutArray[devNum][portNum]++;
                CPSS_APP_PLATFORM_LOG_DBG_MAC("Align90-NOT_COMPLETED:portNum=%d\n", portNum);
                stopAlign90 = GT_FALSE;
                cpssOsTimerWkAfter(16);
            }

            if(stopAlign90)
            {
                rc = cpssDxChPortSerdesAlign90StateSet(devNum, portNum, GT_FALSE,
                            (CPSS_DXCH_PORT_ALIGN90_PARAMS_STC*)&(serdesParams[devNum][portNum]));
                if(rc != GT_OK)
                {
                    CPSS_APP_PLATFORM_LOG_ERR_MAC("cpssDxChPortSerdesAlign90StateSet:portNum=%d,FALSE,rc=%d\n", portNum, rc);
                }
                CPSS_PORTS_BMP_PORT_CLEAR_MAC(&align90TodoBmp[devNum], portNum);
                al90TimeoutArray[devNum][portNum] = 0;

                if(CPSS_DXCH_PORT_SERDES_AUTO_TUNE_PASS_E != align90Status)
                {
                    CPSS_PORT_SPEED_ENT speed;

                    rc = cpssDxChPortSpeedGet(devNum,portNum,&speed);
                    if (rc != GT_OK)
                    {
                        CPSS_APP_PLATFORM_LOG_ERR_MAC("cpssDxChPortSpeedGet(portNum=%d):rc=%d\n", portNum, rc);
                        continue;
                    }

                    if(speed != CPSS_PORT_SPEED_10000_E)
                    {
                        rc = waTriggerInterruptsMask(devNum, portNum,
                                                    /* here doesn't matter SR_LR or KR */
                                                    CPSS_PORT_INTERFACE_MODE_KR_E,
                                                    CPSS_EVENT_UNMASK_E);
                        if(rc != GT_OK)
                        {
                            CPSS_APP_PLATFORM_LOG_ERR_MAC("waTriggerInterruptsMask(portNum=%d,unmask):rc=%d\n", portNum, rc);
                        }
                    }
                }
            } /* if(stopAlign90)*/
        }/* for(portNum) */
    }

    return 0;
}

/*******************************************************************************
* rxTrainingTask
*
* DESCRIPTION:
*       Wait till Rx training will be finish.
*
* INPUTS:
*       param                    -  device number.
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - on wrong port number or device
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*       GT_FAIL                  - on error
*       GT_BAD_PTR               - one of the parameters is NULL pointer
*       GT_HW_ERROR              - on hardware error
*
*******************************************************************************/
unsigned __TASKCONV rxTrainingTask
(
    GT_VOID * param
)
{
    GT_U32                                     Msc = 100;   /* Number of iteration for checking is rx tuning finished */
    GT_U32                                     i;            /* Iteration number */
    CPSS_DXCH_PORT_SERDES_AUTO_TUNE_STATUS_ENT rxTuneStatus; /* RX tuning status */
    CPSS_DXCH_PORT_SERDES_AUTO_TUNE_STATUS_ENT txTuneStatus; /* TX tuning status */
    GT_U8                                      portNum;      /* Port number */
    GT_STATUS                                  rc;           /* Return status */
    GT_U8                                      devNum;       /* Device number */
    GT_UINTPTR                                 tmp;          /* temporary variable */
    GT_U32                                     counter[CPSS_MAX_PORTS_NUM_CNS]; /* counter for pulling getting training status */

    /* to avoid warning: cast from pointer to integer of different size */
    tmp = (GT_UINTPTR)param;
    devNum = (GT_U8)tmp;
    cpssOsMemSet(counter, 0, CPSS_MAX_PORTS_NUM_CNS * sizeof(GT_U32));

    while(1)
    {
        cpssOsSigSemWait(rxTuneSmId[devNum], 0);
        if(serdesTaskReset[devNum])
        {
            break;
        }

        for(i = 0; i < Msc; i++)
        {
            for(portNum = 0; GT_FALSE == CPSS_PORTS_BMP_IS_ZERO_MAC(&rxTodoTuneBmp[devNum]);
                                    portNum = (portNum+1)%CPSS_MAX_PORTS_NUM_CNS)
            {
                PRV_CPSS_DXCH_SKIP_NOT_EXIST_PORT_MAC(devNum, portNum);

                if(CPSS_CPU_PORT_NUM_CNS == portNum)
                {
                    continue;
                }

                if(0 == CPSS_PORTS_BMP_IS_PORT_SET_MAC(&rxTodoTuneBmp[devNum], portNum))
                {
                    continue;
                }
                else
                {
                    counter[portNum]++;
                    if(CPSS_PP_FAMILY_DXCH_XCAT3_E == PRV_CPSS_PP_MAC(devNum)->devFamily)
                    {
                        rc = GT_OK;
                        rxTuneStatus = CPSS_DXCH_PORT_SERDES_AUTO_TUNE_PASS_E;
                    }
                    else
                    {
                        rc = cpssDxChPortSerdesAutoTuneStatusGet(devNum, portNum, &rxTuneStatus, &txTuneStatus);
                    }
                    if(GT_OK == rc)
                    {
                        if(CPSS_DXCH_PORT_SERDES_AUTO_TUNE_PASS_E == rxTuneStatus)
                        {
                            CPSS_APP_PLATFORM_LOG_DBG_MAC("Serdes Auto Tune PASS port=%d, i=%d  counter=%d\n", portNum, i, counter[portNum]);
                            counter[portNum] = 0;
                            CPSS_PORTS_BMP_PORT_CLEAR_MAC(&rxTodoTuneBmp[devNum], portNum);
                            rc = cpssDxChPortEnableSet(devNum, portNum, GT_TRUE);
                            if(rc != GT_OK)
                            {
                                CPSS_APP_PLATFORM_LOG_ERR_MAC("cpssDxChPortEnableSet(portNum=%d,true):rc=%d", portNum, rc);
                                continue;
                            }
                            rc = waTriggerInterruptsMask(devNum, portNum,
                                                         /* here important not RXAUI and not XGMII */
                                                         CPSS_PORT_INTERFACE_MODE_KR_E,
                                                         CPSS_EVENT_UNMASK_E);
                            if(rc != GT_OK)
                            {
                                CPSS_APP_PLATFORM_LOG_ERR_MAC("waTriggerInterruptsMask(portNum=%d):rc=%d\n", portNum, rc);
                            }
                        }
                        else if(CPSS_DXCH_PORT_SERDES_AUTO_TUNE_FAIL_E == rxTuneStatus)
                        {
                            CPSS_APP_PLATFORM_LOG_DBG_MAC("Serdes Auto Tune FAIL port=%d, i=%d  counter=%d\n", portNum, i,counter[portNum]);
                            counter[portNum] = 0;
                            CPSS_PORTS_BMP_PORT_CLEAR_MAC(&rxTodoTuneBmp[devNum], portNum);
                            rc = waTriggerInterruptsMask(devNum, portNum,
                                                         /* here important not RXAUI and not XGMII */
                                                         CPSS_PORT_INTERFACE_MODE_KR_E,
                                                         CPSS_EVENT_UNMASK_E);
                            if(rc != GT_OK)
                            {
                                CPSS_APP_PLATFORM_LOG_ERR_MAC("waTriggerInterruptsMask(portNum=%d):rc=%d\n", portNum, rc);
                            }
                        }
                        else if(counter[portNum] > 50)
                        {
                             /* WA for Serdes Auto Tune NOT COMPLETE - Reset Port.
                               To use this code remember that not all port configuration will be restored */
                             /* CPSS_PORTS_BMP_STC             portsBmp;
                             CPSS_PORT_SPEED_ENT            speed;
                             CPSS_PORT_INTERFACE_MODE_ENT   ifMode;
                             CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portsBmp);
                             CPSS_PORTS_BMP_PORT_SET_MAC(&portsBmp, portNum);
                             rc = cpssDxChPortInterfaceModeGet(devNum, portNum, &ifMode);
                             if (rc != GT_OK)
                             {
                                 CPSS_APP_PLATFORM_LOG_ERR_MAC("cpssDxChPortInterfaceModeGet:rc=%d,portNum=%d\n",  rc, portNum);
                             }

                             rc = cpssDxChPortSpeedGet(devNum,portNum,&speed);
                             if (rc != GT_OK)
                             {
                                CPSS_APP_PLATFORM_LOG_ERR_MAC("cpssDxChPortSpeedGet(portNum=%d):rc=%d\n", portNum, rc);
                             }

                             changeToDownHappend[portNum] = GT_TRUE;
                             rc = cpssDxChPortModeSpeedSet(devNum, &portsBmp, GT_FALSE, ifMode, speed);
                             if (rc != GT_OK)
                             {
                                 CPSS_APP_PLATFORM_LOG_ERR_MAC("cpssDxChPortModeSpeedSet Power Off:rc=%d,portNum=%d\n",  rc, portNum);
                             }
                             cpssOsTimerWkAfter(10);
                             CPSS_PORTS_BMP_PORT_SET_MAC(&portsBmp, portNum);
                             rc = cpssDxChPortModeSpeedSet(devNum, &portsBmp, GT_TRUE, ifMode, speed);
                             if (rc != GT_OK)
                             {
                                 CPSS_APP_PLATFORM_LOG_ERR_MAC("cpssDxChPortModeSpeedSet Power On:rc=%d,portNum=%d\n",  rc, portNum);
                             }*/
                             CPSS_APP_PLATFORM_LOG_DBG_MAC("Serdes Auto Tune NOT COMPLETE port=%d i=%d counter=%d\n", portNum, i, counter[portNum]);
                             counter[portNum] = 0;
                             CPSS_PORTS_BMP_PORT_CLEAR_MAC(&rxTodoTuneBmp[devNum], portNum);
                             rc = waTriggerInterruptsMaskNoLinkStatusChangeInterrupt(devNum, portNum,
                                                          /* here important not RXAUI and not XGMII */
                                                         CPSS_PORT_INTERFACE_MODE_KR_E,
                                                         CPSS_EVENT_UNMASK_E);
                             if(rc != GT_OK)
                             {
                                 CPSS_APP_PLATFORM_LOG_ERR_MAC("waTriggerInterruptsMask(portNum=%d):rc=%d\n", portNum, rc);
                             }
                       }
                   }
                   else
                   {
                        CPSS_APP_PLATFORM_LOG_ERR_MAC("cpssDxChPortSerdesAutoTuneStatusGet(portNum=%d,true):rc=%d", portNum, rc);
                        continue;
                   }
               }
               cpssOsTimerWkAfter(10);
           }
           cpssOsTimerWkAfter(10);
       }
       cpssOsTimerWkAfter(10);
    }

    return 0;
}

/**
* @internal cpssAppRefDxChRxTrainingRun function
* @endinternal
*
* @brief   Run RX training only on port
*
* @param[in] devNum                - device number
* @param[in] portNum               - global port number
* @param[in] interface             - current  of port
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
* @retval GT_HW_ERROR              - hw error
*/
GT_STATUS cpssAppRefDxChRxTrainingRun
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  CPSS_PORT_INTERFACE_MODE_ENT    interface
)
{
    GT_STATUS rc = GT_OK;

    CPSS_APP_PLATFORM_LOG_DBG_MAC("cpssAppRefDxChRxTrainingRun:portNum=%d\n", portNum);

    if(CPSS_PORTS_BMP_IS_PORT_SET_MAC(&rxTodoTuneBmp[devNum], portNum))
    {
        return GT_OK;
    }

    CPSS_APP_PLATFORM_LOG_DBG_MAC("cpssAppRefDxChRxTrainingRun:changeToDownHappend[%d][%d]=%d\n",
                                   devNum, portNum, changeToDownHappend[devNum][portNum]);


    if(!changeToDownHappend[devNum][portNum])
    {/* in case partner port changed interface from 10G to 40G it could happen
        very fast and although interrupt arrive we'll see signal status up,
        so for 10G/11G ports lets try WA on every signal change interrupt */
        CPSS_PORT_SPEED_ENT speed;

        rc = cpssDxChPortSpeedGet(devNum, portNum, &speed);
        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChPortSpeedGet);

        if((CPSS_PORT_SPEED_10000_E == speed) || (CPSS_PORT_SPEED_11800_E == speed))
        {
            CPSS_PORTS_BMP_PORT_SET_MAC(&rxTodoTuneBmp[devNum], portNum);
            rc = cpssOsSigSemSignal(rxTuneSmId[devNum]);
            CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssOsSigSemSignal);
        }
        return GT_OK;
    }
    else
    {
        changeToDownHappend[devNum][portNum] = GT_FALSE;
    }

    rc = waTriggerInterruptsMask(devNum, portNum, interface, CPSS_EVENT_MASK_E);
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, waTriggerInterruptsMask);

    CPSS_APP_PLATFORM_LOG_INFO_MAC("SERDES RX training - port %d\n",portNum);

    rc = cpssDxChPortSerdesAutoTune(devNum, portNum, CPSS_DXCH_PORT_SERDES_AUTO_TUNE_MODE_RX_TRAINING_E);
    if(rc != GT_OK)
    {
        changeToDownHappend[devNum][portNum] = GT_TRUE;
        (GT_VOID)waTriggerInterruptsMask(devNum, portNum, interface, CPSS_EVENT_UNMASK_E);
        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChPortSerdesAutoTune);
    }

    CPSS_PORTS_BMP_PORT_SET_MAC(&rxTodoTuneBmp[devNum], portNum);
    rc = cpssOsSigSemSignal(rxTuneSmId[devNum]);
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssOsSigSemSignal);

    return rc;
}

/*******************************************************************************
* cpssAppRefDxChSerdesAutoTuningTask
*
* DESCRIPTION:
*       Run required type of training on serdeses of port if it's ready
*
* INPUTS:
*       param - device number
*
* RETURNS:
*       GT_OK   - on success,
*       GT_FAIL - otherwise.
*       GT_HW_ERROR - hw error
*
*******************************************************************************/
unsigned __TASKCONV cpssAppRefDxChSerdesAutoTuningTask
(
    GT_VOID *param
)
{
    GT_U8     devNum;
    GT_STATUS rc = GT_OK;
    GT_U32    trxTraining = 0;
    GT_BOOL   forceLinkUp = GT_FALSE;
    GT_BOOL   internalLoopEn = GT_FALSE;
    GT_BOOL   signalState = GT_FALSE;
    GT_BOOL   apEnable = GT_FALSE;
    GT_PHYSICAL_PORT_NUM         portNum;
    CPSS_PORT_INTERFACE_MODE_ENT interface;
    CPSS_DXCH_PORT_AP_PARAMS_STC apParams;
    CPSS_DXCH_PORT_SERDES_LOOPBACK_MODE_ENT serdesLbMode;

    devNum = (GT_U8)((GT_UINTPTR)param);
    CPSS_APP_PLATFORM_LOG_DBG_MAC("cpssAppRefDxChSerdesAutoTuningTask:devNum=%d\n", devNum);

    while(1)
    {
        cpssOsSigSemWait(serdesTuneSmId[devNum], 0);
        if(serdesTaskReset[devNum])
        {
            break;
        }

        for(portNum = 0; portNum < CPSS_MAX_PORTS_NUM_CNS; portNum++)
        {
            PRV_CPSS_DXCH_SKIP_NOT_EXIST_PORT_MAC(devNum, portNum);

            if(CPSS_CPU_PORT_NUM_CNS == portNum)
            {
                continue;
            }

            if(0 == CPSS_PORTS_BMP_IS_PORT_SET_MAC(&todoTuneBmp[devNum], portNum))
            {
                continue;
            }

            rc = cpssDxChPortInterfaceModeGet(devNum,portNum,&interface);
            if (rc != GT_OK)
            {
                CPSS_APP_PLATFORM_LOG_ERR_MAC("cpssDxChPortInterfaceModeGet fail port=%d,rc=%d\n",
                                               portNum, rc);
                continue;
            }

            /* Although SR_LR supposed to work on optical cables that doesn't support TRX
               training, here in demo we allow such option for debugging purposes only */
            if ((interface != CPSS_PORT_INTERFACE_MODE_KR_E) &&
                (interface != CPSS_PORT_INTERFACE_MODE_KR2_E) &&
                (interface != CPSS_PORT_INTERFACE_MODE_KR4_E) &&
                (interface != CPSS_PORT_INTERFACE_MODE_CR_E) &&
                (interface != CPSS_PORT_INTERFACE_MODE_CR2_E) &&
                (interface != CPSS_PORT_INTERFACE_MODE_CR4_E) &&
                (interface != CPSS_PORT_INTERFACE_MODE_SR_LR_E) &&
                (interface != CPSS_PORT_INTERFACE_MODE_XHGS_E) &&
                (interface != CPSS_PORT_INTERFACE_MODE_XHGS_SR_E))
            {
                CPSS_PORTS_BMP_PORT_CLEAR_MAC(&todoTuneBmp[devNum], portNum);
                rc = cpssDxChPortEnableSet(devNum, portNum, GT_TRUE);
                if(rc != GT_OK)
                {
                    CPSS_APP_PLATFORM_LOG_ERR_MAC("cpssDxChPortEnableSet(portNum=%d,true):rc=%d\n", portNum, rc);
                }
                continue;
            }

            rc = cpssDxChPortForceLinkPassEnableGet(devNum,portNum,/*OUT*/&forceLinkUp);
            if (rc != GT_OK)
            {
                CPSS_APP_PLATFORM_LOG_ERR_MAC("cpssDxChPortForceLinkPassEnableGet fail port=%d,rc=%d\n",
                                               portNum, rc);
                continue;
            }

            /* do not perform auto-tune on ports with force link-up*/
            if (forceLinkUp == GT_TRUE)
            {
                CPSS_PORTS_BMP_PORT_CLEAR_MAC(&todoTuneBmp[devNum], portNum);
                rc = cpssDxChPortEnableSet(devNum, portNum, GT_TRUE);
                if(rc != GT_OK)
                {
                    CPSS_APP_PLATFORM_LOG_ERR_MAC("cpssDxChPortEnableSet(portNum=%d,true):rc=%d\n", portNum, rc);
                }
                continue;
            }

            rc = cpssDxChPortInternalLoopbackEnableGet(devNum,portNum,&internalLoopEn);
            if (rc != GT_OK)
            {
                CPSS_APP_PLATFORM_LOG_ERR_MAC("cpssDxChPortInternalLoopbackEnableGet fail portNum=%d, rc=%d\n", portNum, rc);
                continue;
            }
            /* do not perform auto-tune on ports with loopback */
            if (internalLoopEn == GT_TRUE)
            {
                CPSS_PORTS_BMP_PORT_CLEAR_MAC(&todoTuneBmp[devNum], portNum);
                rc = cpssDxChPortEnableSet(devNum, portNum, GT_TRUE);
                if(rc != GT_OK)
                {
                    CPSS_APP_PLATFORM_LOG_ERR_MAC("cpssDxChPortEnableSet(portNum=%d,true):rc=%d\n", portNum, rc);
                }
                continue;
            }

            rc = cpssDxChPortSerdesLoopbackModeGet(devNum, portNum, 0, &serdesLbMode);
            if (rc != GT_OK)
            {
                CPSS_APP_PLATFORM_LOG_ERR_MAC("cpssDxChPortSerdesLoopbackModeGet fail portNum=%d, rc=%d\n", portNum, rc);
                continue;
            }
            /* do not perform auto-tune on ports with loopback */
            if (serdesLbMode != CPSS_DXCH_PORT_SERDES_LOOPBACK_DISABLE_E)
            {
                CPSS_PORTS_BMP_PORT_CLEAR_MAC(&todoTuneBmp[devNum], portNum);
                rc = cpssDxChPortEnableSet(devNum, portNum, GT_TRUE);
                if(rc != GT_OK)
                {
                    CPSS_APP_PLATFORM_LOG_ERR_MAC("cpssDxChPortEnableSet(portNum=%d,true):rc=%d\n", portNum, rc);
                }
                continue;
            }

            rc = cpssDxChPortApPortConfigGet(devNum, portNum, &apEnable, &apParams);
            if (rc != GT_OK)
            {
                CPSS_APP_PLATFORM_LOG_ERR_MAC("cpssDxChPortApPortConfigGet fail portNum=%d, rc=%d\n", portNum, rc);
                continue;
            }
            if(apEnable)
            {    /* don't run rx training if AP enabled on port,
                 also no need to re-enable port in case of AP it should be done by
                 application */
                CPSS_PORTS_BMP_PORT_CLEAR_MAC(&todoTuneBmp[devNum], portNum);
                continue;
            }

            if(rxTraining)
            {
                /* XCAT3 and SIP_5_15 */
                GT_BOOL pcsSyncState = GT_TRUE;
                if(GT_FALSE == changeToDownHappend[devNum][portNum])
                {
                     signalState = GT_FALSE;
                     rc = cpssDxChPortPcsSyncStatusGet(devNum, portNum, &pcsSyncState);
                     if (rc != GT_OK)
                     {
                         CPSS_APP_PLATFORM_LOG_ERR_MAC("cpssDxChPortPcsSyncStatusGet:rc=%d,portNum=%d\n", rc, portNum);
                     }
                }

                if(GT_TRUE == pcsSyncState)
                {
                    /* check that Serdes Signal detected on port is OK and stable - on all its serdes lanes */
                    rc = cpssDxChPortPcsSyncStableStatusGet(devNum, portNum, &signalState);
                    if (rc != GT_OK)
                    {
                        CPSS_APP_PLATFORM_LOG_ERR_MAC("cpssDxChPortPcsSyncStableStatusGet fail port=%d,rc=%d\n", portNum, rc);
                        CPSS_PORTS_BMP_PORT_CLEAR_MAC(&todoTuneBmp[devNum], portNum);
                        signalState = GT_FALSE;
                    }
                }
            }
            else if(trxTraining)
            {/* In case of TRX training if other side started already training
                there won't be PCS sync, so just check signal to see if there is
                partner and try to start training.
                With fiber connection there could be signal detect just because connector
                is in, while no real peer connected to port or peer not configured
                to appropriate mode */
                rc = cpssDxChPortSerdesSignalDetectGet(devNum, portNum, &signalState);
                if (rc != GT_OK)
                {
                    CPSS_APP_PLATFORM_LOG_ERR_MAC("cpssDxChPortSerdesSignalDetectGet fail port=%d,rc=%d\n", portNum, rc);
                    CPSS_PORTS_BMP_PORT_CLEAR_MAC(&todoTuneBmp[devNum], portNum);
                    continue;
                }
            }

            CPSS_APP_PLATFORM_LOG_DBG_MAC("portNum=%d:signalState/pcsSync=%d\n", portNum, signalState);

            if(GT_FALSE == signalState)
            {
                if(GT_TRUE == changeToUpHappend[devNum][portNum])
                {
                    rc = cpssEventDeviceMaskWithEvExtDataSet(devNum,
                                  CPSS_PP_PORT_LINK_STATUS_CHANGED_E,
                                  portNum,
                                  CPSS_EVENT_MASK_E);
                    if (rc != GT_OK)
                    {
                         CPSS_APP_PLATFORM_LOG_ERR_MAC("cpssEventDeviceMaskWithEvExtDataSet:rc=%d,portNum=%d\n",
                                         rc, portNum);
                    }
                }

                changeToDownHappend[devNum][portNum] = GT_TRUE;
                CPSS_PORTS_BMP_PORT_CLEAR_MAC(&todoTuneBmp[devNum], portNum);
                continue;
            }

            changeToUpHappend[devNum][portNum] = GT_TRUE;
            /* although link could be already up partner could be changed to 40G,
                so must continue till 10G-to-40G WA */
            if(trxTraining)
            {
                rc = cpssAppRefDxChTrxTrainingRun(devNum, portNum, interface);
                if(rc != GT_OK)
                {
                    CPSS_APP_PLATFORM_LOG_ERR_MAC("cpssAppRefDxChTrxTrainingRun(port=%d,interface=%d):rc=%d\n",
                                                   portNum, interface, rc);
                }
            }
            else
            {
                rc = cpssAppRefDxChRxTrainingRun(devNum,portNum,interface);
                if(rc != GT_OK)
                {
                    CPSS_APP_PLATFORM_LOG_ERR_MAC("cpssAppRefDxChRxTrainingRun(port=%d,interface=%d):rc=%d\n",
                                                   portNum, interface, rc);
                }
            }

            CPSS_PORTS_BMP_PORT_CLEAR_MAC(&todoTuneBmp[devNum], portNum);
        } /* for(portNum) */

        cpssOsTimerWkAfter(32);

    } /* while(1) */

    return 0;
}

/**
* @internal cpssAppRefSerdesTrainingTaskCreate function
* @endinternal
*
* @brief   create serdes training task.
*
* @param[in] devNum    - device number.
*
* @retval GT_OK        - on success,
* @retval GT_FAIL      - otherwise.
*
*/
GT_STATUS cpssAppRefSerdesTrainingTaskCreate
(
    GT_U8  devNum
)
{
    GT_STATUS rc = GT_OK;
    GT_U32    value = 0;

    if((appPlatformDbEntryGet("trxTraining", &value) == GT_OK) && (value == 1))
    {
         trxTraining = GT_TRUE;
         rxTraining = GT_FALSE;
    }
    else if((appPlatformDbEntryGet("rxTraining", &value) == GT_OK) && (value == 1))
    {
         trxTraining = GT_FALSE;
         rxTraining = GT_TRUE;
    }
    else
    {
         trxTraining = GT_FALSE;
         rxTraining = GT_FALSE;
    }

    if(trxTraining || rxTraining)
    {
        if(serdesTuneSmId[devNum] == 0)
        {
            GT_CHAR name[20];

            cpssOsSprintf(name, "tuneSem_%d", devNum);

            rc = cpssOsSigSemBinCreate(name, CPSS_OS_SEMB_EMPTY_E, &serdesTuneSmId[devNum]);
            CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssOsSigSemBinCreate);

            CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&todoTuneBmp[devNum]);

            cpssOsSprintf(name, "tuneTask_%d", devNum);
            rc = cpssOsTaskCreate(name,                                   /* Task Name      */
                                  210,                                    /* Task Priority  */
                                  _64K,                                   /* Stack Size     */
                                  cpssAppRefDxChSerdesAutoTuningTask,     /* Starting Point */
                                  (GT_VOID*)((GT_UINTPTR)devNum),         /* Arguments list */
                                  &serdesTuneTaskId[devNum]);             /* task ID        */

            CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssOsTaskCreate);
        }
    }

    if(rxTraining)
    {
        if(rxTuneSmId[devNum] == 0)
        {
            GT_CHAR name[20];

            cpssOsSprintf(name, "rxTuneSem_%d", devNum);

            rc = cpssOsSigSemBinCreate(name, CPSS_OS_SEMB_EMPTY_E, &rxTuneSmId[devNum]);
            CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssOsSigSemBinCreate);

            CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&rxTodoTuneBmp[devNum]);

            cpssOsSprintf(name, "rxTuneTask_%d", devNum);
            rc = cpssOsTaskCreate(name,                                   /* Task Name      */
                                  210,                                    /* Task Priority  */
                                  _64K,                                   /* Stack Size     */
                                  rxTrainingTask,                         /* Starting Point */
                                  (GT_VOID*)((GT_UINTPTR)devNum),         /* Arguments list */
                                  &rxTuneTaskId[devNum]);                 /* task ID        */
            CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssOsTaskCreate);
        }
    }

    if(trxTraining)
    {
        if(trxTuneSmId[devNum] == 0)
        {
            GT_CHAR name[20];

            cpssOsSprintf(name, "trxTuneSem_%d", devNum);

            rc = cpssOsSigSemBinCreate(name, CPSS_OS_SEMB_EMPTY_E, &trxTuneSmId[devNum]);
            CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssOsSigSemBinCreate);

            CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&trxTodoTuneBmp[devNum]);

            cpssOsSprintf(name, "trxTuneTask_%d", devNum);
            rc = cpssOsTaskCreate(name,                                   /* Task Name      */
                                  210,                                    /* Task Priority  */
                                  _64K,                                   /* Stack Size     */
                                  trxTrainingTask,                        /* Starting Point */
                                  (GT_VOID*)((GT_UINTPTR)devNum),         /* Arguments list */
                                  &trxTuneTaskId[devNum]);                /* task ID        */
            CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssOsTaskCreate);
        }

        if(align90SmId[devNum] == 0)
        {
            GT_CHAR name[20];

            cpssOsSprintf(name, "align90Sem_%d", devNum);

            rc = cpssOsSigSemBinCreate(name, CPSS_OS_SEMB_EMPTY_E, &align90SmId[devNum]);
            CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssOsSigSemBinCreate);

            CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&align90TodoBmp[devNum]);

            cpssOsSprintf(name, "align90Task_%d", devNum);
            rc = cpssOsTaskCreate(name,                                   /* Task Name      */
                                  210,                                    /* Task Priority  */
                                  _64K,                                   /* Stack Size     */
                                  align90WaTask,                          /* Starting Point */
                                  (GT_VOID*)((GT_UINTPTR)devNum),         /* Arguments list */
                                  &align90TaskId[devNum]);                /* task ID        */
            CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssOsTaskCreate);
        }
    }

    return rc;
}

/**
* @internal cpssAppRefSerdesTrainingTaskDelete function
* @endinternal
*
* @brief   Delete serdes training task.
*
* @param[in] devNum    - device number.
*
* @retval GT_OK        - on success,
* @retval GT_FAIL      - otherwise.
*
*/
GT_STATUS cpssAppRefSerdesTrainingTaskDelete
(
    GT_U8 devNum
)
{
    serdesTaskReset[devNum] = GT_TRUE;

    if(trxTraining || rxTraining)
    {
        cpssOsSigSemSignal(serdesTuneSmId[devNum]);

        if(trxTraining)
        {
            cpssOsSigSemSignal(trxTuneSmId[devNum]);
            cpssOsSigSemSignal(align90SmId[devNum]);
        }

        if(rxTraining)
        {
            cpssOsSigSemSignal(rxTuneSmId[devNum]);
        }

        /* provide time for the tasks to exit */
        cpssOsTimerWkAfter(200);

        cpssOsSigSemDelete(serdesTuneSmId[devNum]);
        serdesTuneSmId[devNum] = 0;
        if(trxTraining)
        {
            cpssOsSigSemDelete(trxTuneSmId[devNum]);
            trxTuneSmId[devNum] = 0;
            cpssOsSigSemDelete(align90SmId[devNum]);
            align90SmId[devNum] = 0;
        }
        if(rxTraining)
        {
            cpssOsSigSemDelete(rxTuneSmId[devNum]);
            rxTuneSmId[devNum] = 0;
        }

        CPSS_APP_PLATFORM_LOG_INFO_MAC("cpssAppRefEventFuncReset: serdes training tasks of device %d deleted\n", devNum);
    }

    serdesTaskReset[devNum] = GT_FALSE;

    return GT_OK;
}
