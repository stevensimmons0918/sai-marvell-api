/*******************************************************************************
*            Copyright 2001, Marvell International Ltd.
* This code contains confidential information of Marvell semiconductor, inc.
* no rights are granted herein under any patent, mask work right or copyright
* of Marvell or any third party.
* Marvell reserves the right at its sole discretion to request that this code
* be immediately returned to Marvell. This code is provided "as is".
* Marvell makes no warranties, express, implied or otherwise, regarding its
* accuracy, completeness or performance.
********************************************************************************
* mvHwsPortCfgIf.c
*
* DESCRIPTION:
*           This file contains API for port configuartion and tuning parameters
*
* DEPENDENCIES:
*
* FILE REVISION NUMBER:
*       $Revision: 48 $
******************************************************************************/
#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>
#include <cpss/common/labServices/port/gop/port/mvHwsPortInitIf.h>
#include <cpss/common/labServices/port/gop/port/private/mvHwsPortPrvIf.h>
#include <cpss/common/labServices/port/gop/port/mvHwsPortMiscIf.h>
#include <cpss/common/labServices/port/gop/port/mvPortModeElements.h>
#include <cpss/generic/labservices/port/gop/silicon/lion2/mvHwsLion2PortIf.h>
#include <cpss/common/labServices/port/gop/port/serdes/comPhyHRev2/mvComPhyHRev2If.h>
#include <cpss/common/labServices/port/gop/port/serdes/mvHwsSerdesPrvIf.h>
#include <cpss/common/labServices/port/gop/port/pcs/mvHwsPcsIf.h>
#include <cpss/common/labServices/port/gop/port/mac/mvHwsMacIf.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/siliconAddress.h>

/**************************** Definition *************************************************/
/* #define GT_DEBUG_HWS */
#ifdef  GT_DEBUG_HWS
#include <common/os/gtOs.h>
#define DEBUG_HWS_FULL(level,s) if (optPrintLevel >= level) {hwsOsPrintf s;}
#else
#define DEBUG_HWS_FULL(level,s)
#endif

/**
* @internal hwsPortFixAlign90ExtMultipleLaneOptimization function
* @endinternal
*
* @brief   Run fix Align90 process on current port.
*         Can be run any time after create port.
*         == Hooper A0, Lion2 B0 ==
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
static GT_STATUS hwsPortFixAlign90ExtMultipleLaneOptimization
(
    GT_U8   devNum,
    GT_U32  portGroup,
    GT_U32  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    GT_U32  optAlgoMask
)
{
    MV_HWS_PORT_INIT_PARAMS curPortParams;
    GT_U32 curLanesList[HWS_MAX_SERDES_NUM];
    GT_U32 regulatorLanesList[HWS_MAX_SERDES_NUM];
    GT_U32 tmpSerdesList[HWS_MAX_SERDES_NUM];
    GT_U32 numOfActLanes;
    MV_HWS_PORT_MAN_TUNE_MODE portTuningMode = StaticLongReach;
    GT_BOOL isTrainingMode = GT_FALSE;
    GT_BOOL allLanesPass = GT_TRUE;
    GT_U32 align90Count = 0;
    GT_U32 i;

    if ((phyPortNum >= HWS_CORE_PORTS_NUM(devNum)) ||
        ((GT_U32)portMode >= HWS_DEV_PORT_MODES(devNum)))
    {
        return GT_BAD_PARAM;
    }

    hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams);

    /* rebuild active lanes list according to current configuration (redundancy) */
    CHECK_STATUS(mvHwsRebuildActiveLaneList(devNum, portGroup, phyPortNum, portMode, curLanesList));

    numOfActLanes = curPortParams.numOfActLanes;
    for (i = 0; i < numOfActLanes; i++)
    {
        tmpSerdesList[i] = curLanesList[i];
    }

    do
    {
        DEBUG_HWS_FULL(2, ("\nAlign90 Iteration %d\n", align90Count));

        /* for Lion2/Hooper we try first to change the regulator. if it won't work, the optimization
        algorithm will be executed (later) */
        if (align90Count == 10)
        {
            DEBUG_HWS_FULL(0, ("Configure regulator to high value\n"));

            CHECK_STATUS(mvHwsComHRev2SerdesConfigRegulator(devNum, portGroup, tmpSerdesList,
                                        numOfActLanes, GT_FALSE));

            /* save the SERDES with update regulator */
            for (i = 0; i < numOfActLanes; i++)
            {
                regulatorLanesList[i] = tmpSerdesList[i];
            }
        }


        CHECK_STATUS(hwsPortFixAlign90Flow(devNum, portGroup, tmpSerdesList, numOfActLanes, &allLanesPass));
        align90Count++;
    } while ((allLanesPass != GT_TRUE) && (align90Count < 20));

    if (align90Count > 10)
    {
        DEBUG_HWS_FULL(0, ("Configure regulator to low value\n"));

        CHECK_STATUS(mvHwsComHRev2SerdesConfigRegulator(devNum, portGroup, regulatorLanesList,
                                    numOfActLanes, GT_TRUE));
    }

    if (allLanesPass == GT_FALSE)
    {
        DEBUG_HWS_FULL(2, ("****** Align90 failed for 20 iterations\n"));
    }

    /* For Lion2 B0/Hooper only we run optimization algorithms to tune the Serdes better */
    if (allLanesPass == GT_TRUE)
    {
        /* check if port mode needs to be tuned and if it's Short/Long reach */
        CHECK_STATUS(hwsPortGetTuneMode(portMode, &portTuningMode, &isTrainingMode));

        if (isTrainingMode == GT_TRUE)
        {
            CHECK_STATUS(mvHwsComHRev2SerdesTrainingOptimization(devNum, portGroup, curLanesList,
                                    numOfActLanes, portTuningMode, optAlgoMask));
        }
    }

    for (i = 0; (i < HWS_MAX_SERDES_NUM) && (i < numOfActLanes); i++)
    {
        mvHwsSerdesRev2DfeCheck(devNum, (portGroup + ((curLanesList[i] >> 16) & 0xFF)),
                    (curLanesList[i] & 0xFFFF), (MV_HWS_SERDES_TYPE)HWS_DEV_SERDES_TYPE(devNum, (curLanesList[i] & 0xFFFF)));
    }

    if (allLanesPass)
    {
        return GT_OK;
    }

    return GT_FAIL;
}

/**
* @internal mvHwsLion2PCSMarkModeSet function
* @endinternal
*
* @brief   Mark/Un-mark PCS unit
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
* @param[in] enable                   - GT_TRUE  for mark the PCS,
*                                      GT_FALSE for un-mark the PCS
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
*/
GT_STATUS mvHwsLion2PCSMarkModeSet
(
    GT_U8   devNum,
    GT_UOPT portGroup,
    GT_U32  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    GT_BOOL enable
)
{
    MV_HWS_PORT_INIT_PARAMS curPortParams;

    if ((phyPortNum >= HWS_CORE_PORTS_NUM(devNum)) ||
        ((GT_U32)portMode >= HWS_DEV_PORT_MODES(devNum)))
    {
        return GT_BAD_PARAM;
    }

    hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams);
    switch (portMode)
    {
        case _10GBase_KR:
        case _40GBase_KR4:
        case _10GBase_SR_LR:
        case _40GBase_SR_LR4:
        case _12GBaseR:
        case _48GBaseR4:
        case _12GBase_SR:
        case _48GBase_SR4:
            CHECK_STATUS(genUnitRegisterSet(devNum, portGroup, MMPCS_UNIT, curPortParams.portPcsNumber,
                                            FEC_DEC_DMA_WR_DATA, enable, 1));
            break;
        case RXAUI:
            CHECK_STATUS(genUnitRegisterSet(devNum, portGroup, XPCS_UNIT, curPortParams.portPcsNumber,
                                            XPCS_Internal_Metal_Fix, enable, 1));
            break;
        default:
            hwsOsPrintf("mvHwsLion2PCSMarkModeSet: portMode %d is not supported for Mark/Un-mark PCS unit\n", portMode);
            return GT_BAD_PARAM;
    }

    return GT_OK;
}

/**
* @internal mvHwsLion2PortFixAlign90Ext function
* @endinternal
*
* @brief   Fix Align90 parameters
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
* @param[in] optAlgoMask              - bit mask for optimization algorithms
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsLion2PortFixAlign90Ext
(
    GT_U8   devNum,
    GT_U32  portGroup,
    GT_U32  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    GT_U32  optAlgoMask
)
{
    switch (portMode)
    {
    case _10GBase_KR:
    case _20GBase_KR2:
    case _40GBase_KR4:
    case _10GBase_SR_LR:
    case _20GBase_SR_LR2:
    case _40GBase_SR_LR4:
    case _12GBaseR:
    case _48GBaseR4:
    case _12GBase_SR:
    case _48GBase_SR4:
        if (hwsDeviceSpecInfo[devNum].devType == Lion2A0)
        {
            CHECK_STATUS(mvHwsPortFixAlign90(devNum, portGroup, phyPortNum, portMode, optAlgoMask));
        }
        else /* Lion2B0, HooperA0*/
        {
            CHECK_STATUS(hwsPortFixAlign90ExtMultipleLaneOptimization(devNum, portGroup, phyPortNum, portMode, optAlgoMask));
        }
        break;
    default:
        break;
    }

    return GT_OK;
}

/**
* @internal mvHwsLion2PortRxAutoTuneSetExt function
* @endinternal
*
* @brief   Sets the port Tx and Rx parameters according to different working
*         modes/topologies.
*         Can be run any time after create port.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
* @param[in] optAlgoMask              - bit mask for optimization algorithms
* @param[in] portTuningMode           - port tuning mode
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsLion2PortRxAutoTuneSetExt
(
    GT_U8   devNum,
    GT_U32  portGroup,
    GT_U32  phyPortNum,
    MV_HWS_PORT_STANDARD        portMode,
    MV_HWS_PORT_AUTO_TUNE_MODE      portTuningMode,
    GT_U32  optAlgoMask,
    void *  results
)
{
    MV_HWS_SERDES_AUTO_TUNE_RESULTS_UNT res[HWS_MAX_SERDES_NUM];
    MV_HWS_PORT_INIT_PARAMS curPortParams;
    MV_HWS_AUTO_TUNE_STATUS rxStatus;
    GT_U32  curLanesList[HWS_MAX_SERDES_NUM];
    GT_BOOL laneFail = GT_FALSE;
    GT_U32  sqleuch, ffeR, ffeC, align90;
    GT_U32  txAmp, txEmph0, txEmph1;
    GT_U32  i;
    GT_U32 numOfActLanes;

    /* avoid warnings */
    portTuningMode = portTuningMode;
    (void)results;

    if ((phyPortNum >= HWS_CORE_PORTS_NUM(devNum)) ||
        ((GT_U32)portMode >= HWS_DEV_PORT_MODES(devNum)))
    {
        return GT_BAD_PARAM;
    }

    hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams);
    hwsOsMemSetFuncPtr(res, 0, sizeof(res));

    /* rebuild active lanes list according to current configuration (redundancy) */
    CHECK_STATUS(mvHwsRebuildActiveLaneList(devNum, portGroup, phyPortNum, portMode, curLanesList));

    /* reset PCS Rx only */
    if (curPortParams.portPcsType == MMPCS)
    {
        CHECK_STATUS(mvHwsPcsRxReset(devNum, portGroup, curPortParams.portPcsNumber,
                            curPortParams.portPcsType, RESET));
    }

    numOfActLanes = curPortParams.numOfActLanes;

    /* on each related serdes */
    for (i = 0; (i < HWS_MAX_SERDES_NUM) && (i < numOfActLanes); i++)
    {
        CHECK_STATUS(mvHwsSerdesRxAutoTuneStart(devNum, (portGroup + ((curLanesList[i] >> 16) & 0xFF)),
                                    (curLanesList[i] & 0xFFFF), HWS_DEV_SERDES_TYPE(devNum, (curLanesList[i] & 0xFFFF)), GT_TRUE));
    }

    /* check status and start auto tune again if not passed */
    for (i = 0; (i < HWS_MAX_SERDES_NUM) && (i < numOfActLanes); i++)
    {
        rxStatus = TUNE_PASS;
        mvHwsSerdesAutoTuneStatus(devNum, (portGroup + ((curLanesList[i] >> 16) & 0xFF)),
                        (curLanesList[i] & 0xFFFF), HWS_DEV_SERDES_TYPE(devNum, (curLanesList[i] & 0xFFFF)), &rxStatus, NULL);
        if (rxStatus != TUNE_PASS)
        {
            laneFail = GT_TRUE;
        }
    }

    hwsOsTimerWkFuncPtr(1);

    if (laneFail == GT_TRUE)
    {
        /* unreset PCS Rx only */
        if (curPortParams.portPcsType == MMPCS)
        {
            CHECK_STATUS(mvHwsPcsRxReset(devNum, portGroup, curPortParams.portPcsNumber,
                                curPortParams.portPcsType, UNRESET));
        }

        return GT_FAIL;
    }

    /* get results */
    sqleuch = ffeR = ffeC = align90 = 0;
    txAmp = txEmph0 = txEmph1 = 0;
    for (i = 0; (i < HWS_MAX_SERDES_NUM) && (i < numOfActLanes); i++)
    {
        CHECK_STATUS(mvHwsSerdesAutoTuneResult(devNum, (portGroup + ((curLanesList[i] >> 16) & 0xFF)),
                                (curLanesList[i] & 0xFFFF), HWS_DEV_SERDES_TYPE(devNum, (curLanesList[i] & 0xFFFF)), &res[i]));
        sqleuch += res[i].comphyHResults.sqleuch;
        ffeR    += res[i].comphyHResults.ffeR;
        ffeC    += res[i].comphyHResults.ffeC;
        txAmp   += res[i].comphyHResults.txAmp;
        txEmph0 += res[i].comphyHResults.txEmph0;
        txEmph1 += res[i].comphyHResults.txEmph1;
     }

    /* disable training on each related serdes */
    for (i = 0; (i < HWS_MAX_SERDES_NUM) && (i < numOfActLanes); i++)
    {
         CHECK_STATUS(mvHwsSerdesRxAutoTuneStart(devNum, (portGroup + ((curLanesList[i] >> 16) & 0xFF)),
                                (curLanesList[i] & 0xFFFF), HWS_DEV_SERDES_TYPE(devNum, (curLanesList[i] & 0xFFFF)), GT_FALSE));
    }

    /* calculate average tune parameters for all lanes and force values only on Lion2A0 */
    /* on each related serdes */
    for (i = 0; (i < HWS_MAX_SERDES_NUM) && (hwsDeviceSpecInfo[devNum].devType == Lion2A0) && (i < numOfActLanes); i++)
    {
        CHECK_STATUS(mvHwsSerdesDfeOpti(devNum, (portGroup + ((curLanesList[i] >> 16) & 0xFF)),
                            (curLanesList[i] & 0xFFFF), HWS_DEV_SERDES_TYPE(devNum, (curLanesList[i] & 0xFFFF)), &ffeR));
        CHECK_STATUS(mvHwsSerdesFfeConfig(devNum, (portGroup + ((curLanesList[i] >> 16) & 0xFF)),
                            (curLanesList[i] & 0xFFFF), HWS_DEV_SERDES_TYPE(devNum, (curLanesList[i] & 0xFFFF)), ffeR,
                            (ffeC / numOfActLanes), res[i].comphyHResults.sampler));
    }

    CHECK_STATUS(mvHwsLion2PortFixAlign90Ext(devNum, portGroup, phyPortNum, portMode, optAlgoMask));

    /* unreset PCS Rx only */
    hwsOsTimerWkFuncPtr(1);

    if (curPortParams.portPcsType == MMPCS)
    {
        CHECK_STATUS(mvHwsPcsRxReset(devNum, portGroup, curPortParams.portPcsNumber,
                            curPortParams.portPcsType, UNRESET));
    }

    if (portMode == RXAUI)
    {
        /* call to WA */
        mvHwsXPcsConnect(devNum, portGroup, phyPortNum);
    }

    return GT_OK;
}

/**
* @internal mvHwsLion2PortTxAutoTuneSetExt function
* @endinternal
*
* @brief   Sets the port Tx only parameters according to different working
*         modes/topologies.
*         Can be run any time after create port.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
* @param[in] portTuningMode           - port TX related tuning mode
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsLion2PortTxAutoTuneSetExt
(
    GT_U8   devNum,
    GT_U32  portGroup,
    GT_U32  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    MV_HWS_PORT_AUTO_TUNE_MODE      portTuningMode,
    GT_U32  optAlgoMask
)
{
    if ((phyPortNum >= HWS_CORE_PORTS_NUM(devNum)) ||
       ((GT_U32)portMode >= HWS_DEV_PORT_MODES(devNum)))
    {
         return GT_BAD_PARAM;
    }

    CHECK_STATUS(hwsPortTxAutoTuneStartSet(devNum, portGroup, phyPortNum, portMode, portTuningMode, optAlgoMask));

    if (portTuningMode == TRxTuneStatus)
    {
        CHECK_STATUS(mvHwsLion2PortFixAlign90Ext(devNum, portGroup, phyPortNum, portMode, optAlgoMask));

        CHECK_STATUS(hwsPortTxAutoTuneActivateSet(devNum, portGroup, phyPortNum, portMode, portTuningMode));

        if (portMode == RXAUI)
        {
            /* call to WA */
            mvHwsXPcsConnect(devNum, portGroup, phyPortNum);
        }
    }

    return GT_OK;
}


/**
* @internal mvHwsLion2PortAutoTuneSetExt function
* @endinternal
*
* @brief   Sets the port Tx and Rx parameters according to different working
*         modes/topologies.
*         Can be run any time after create port.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
* @param[in] optAlgoMask              - bit mask for optimization algorithms
* @param[in] portTuningMode           - port tuning mode
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsLion2PortAutoTuneSetExt
(
    GT_U8   devNum,
    GT_U32  portGroup,
    GT_U32  phyPortNum,
    MV_HWS_PORT_STANDARD        portMode,
    MV_HWS_PORT_AUTO_TUNE_MODE      portTuningMode,
    GT_U32  optAlgoMask,
    void *  results
)
{
    if (portTuningMode == RxTrainingOnly)
    {
        return mvHwsLion2PortRxAutoTuneSetExt(devNum, portGroup, phyPortNum, portMode, portTuningMode, optAlgoMask, results);
    }
    else
    {
        return mvHwsLion2PortTxAutoTuneSetExt(devNum, portGroup, phyPortNum, portMode, portTuningMode, optAlgoMask);
    }
}

/**
* @internal mvHwsPortAutoTuneDelayInit function
* @endinternal
*
* @brief   Sets the delay values which are used in Serdes training optimization
*         algorithm
* @param[in] dynamicDelayInterval     - determines the number of training iteration in
*                                      which the delay will be executed (DFE algorithm)
* @param[in] dynamicDelayDuration     - delay duration in mSec (DFE algorithm)
* @param[in] staticDelayDuration      - delay duration in mSec (Align90 algorithm)
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortAutoTuneDelayInit
(
    GT_U32  dynamicDelayInterval, /* DFE */
    GT_U32  dynamicDelayDuration, /* DFE */
    GT_U32  staticDelayDuration   /* Align90 */
)
{
    return mvHwsComHRev2SerdesTrainingOptDelayInit(dynamicDelayInterval, dynamicDelayDuration, staticDelayDuration);
}

/**
* @internal mvHwsPortTxAutoTuneUpdateWaExt function
* @endinternal
*
* @brief   Update auto tune parameters according to tune result
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
* @param[in] curLanesList             - active Serdes lanes list according to configuration
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortTxAutoTuneUpdateWaExt
(
    GT_U8   devNum,
    GT_U32  portGroup,
    GT_U32  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    GT_U32  *curLanesList
)
{
    GT_U32 i;
    GT_U32  sqleuch, ffeR, ffeC, align90;
    GT_U32  txAmp, txEmph0, txEmph1;
    MV_HWS_SERDES_AUTO_TUNE_RESULTS_UNT res[HWS_MAX_SERDES_NUM];
    MV_HWS_PORT_INIT_PARAMS curPortParams;
    hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams);

    hwsOsMemSetFuncPtr(res, 0, sizeof(res));

    if (HWS_DEV_SERDES_TYPE(devNum, curLanesList[0]) == COM_PHY_H)
    {
        sqleuch = ffeR = ffeC = align90 = 0;
        txAmp = txEmph0 = txEmph1 = 0;

        for (i = 0; (i < HWS_MAX_SERDES_NUM) && (i < curPortParams.numOfActLanes); i++)
        {
            CHECK_STATUS(mvHwsSerdesAutoTuneResult(devNum, (portGroup + ((curLanesList[i] >> 16) & 0xFF)),
                                (curLanesList[i] & 0xFFFF), HWS_DEV_SERDES_TYPE(devNum, (curLanesList[i] & 0xFFFF)), &res[i]));

            sqleuch += res[i].comphyHResults.sqleuch;
            ffeR    += res[i].comphyHResults.ffeR;
            ffeC    += res[i].comphyHResults.ffeC;
            txAmp   += res[i].comphyHResults.txAmp;
            txEmph0 += res[i].comphyHResults.txEmph0;
            txEmph1 += res[i].comphyHResults.txEmph1;
        }

        /* calculate average tune parameters for all lanes and force values on each related serdes */
        for (i = 0; (i < HWS_MAX_SERDES_NUM) && (i < curPortParams.numOfActLanes); i++)
        {
            CHECK_STATUS(mvHwsSerdesDfeOpti(devNum, (portGroup + ((curLanesList[i] >> 16) & 0xFF)),
                                (curLanesList[i] & 0xFFFF), HWS_DEV_SERDES_TYPE(devNum, (curLanesList[i] & 0xFFFF)), &ffeR));
            CHECK_STATUS(mvHwsSerdesFfeConfig(devNum, (portGroup + ((curLanesList[i] >> 16) & 0xFF)),
                                (curLanesList[i] & 0xFFFF), HWS_DEV_SERDES_TYPE(devNum, (curLanesList[i] & 0xFFFF)), ffeR,
                                (ffeC / curPortParams.numOfActLanes), res[i].comphyHResults.sampler));
        }
    }

    return GT_OK;
}

/**
* @internal mvHwsPortLion2ExtendedModeCfgGet function
* @endinternal
*
* @brief   Returns the extended mode status on port specified.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
*
* @param[out] extendedMode             - enable / disable
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortLion2ExtendedModeCfgGet
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    GT_BOOL                 *extendedMode
)
{
    GT_U32 data;
    GT_U32 offset;

    /* avoid warnings */
    portMode  = portMode;

    if ((GT_U32)portMode >= HWS_DEV_PORT_MODES(devNum))
    {
        return GT_BAD_PARAM;
    }

    if (phyPortNum != 9 && phyPortNum != 11)
    {
        *extendedMode = GT_FALSE;
        return GT_OK;
    }

    /* CPSS set register 0x0F800070 when port need to configure to extended mode */
    CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, portGroup, EXTENEDED_PORTS_CONFIGURATION, &data, 0));

    /* In Lion2:
    - for port#9 - if bit 0 in External_Control reg is 0x1, the port is in Extended mode
    - for port#11 - if bit 1 in External_Control reg is 0x1, the port is in Extended mode */
    offset = (phyPortNum == 9) ? 0 : 1;
    *extendedMode = ((data >> offset) & 0x1) ? GT_TRUE : GT_FALSE;

    return GT_OK;
}

/**
* @internal mvHwsPortSetOptAlgoParams function
* @endinternal
*
* @brief   Change default thresholds of:
*         ffe optimizer for first iteration, final iteration and f0d stop threshold
* @param[in] ffeFirstTh               - threshold value for changing ffe optimizer for first iteration
* @param[in] ffeFinalTh               - threshold value for changing ffe optimizer for final iteration
* @param[in] f0dStopTh                - value for changing default of f0d stop threshold
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortSetOptAlgoParams
(
    GT_U8     devNum,
    GT_U32    portGroup,
    GT_32     ffeFirstTh,
    GT_32     ffeFinalTh,
    GT_U32    f0dStopTh
)
{
    /* avoid warnings */
    devNum = devNum;
    portGroup = portGroup;

    return mvHwsComHRev2SerdesOptAlgoParams(ffeFirstTh, ffeFinalTh, f0dStopTh);
}

/**
* @internal mvHwsPortLion2AutoTuneStop function
* @endinternal
*
*/
GT_STATUS mvHwsPortLion2AutoTuneStop
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    GT_BOOL                 stopRx,
    GT_BOOL                 stopTx
)
{
    MV_HWS_PORT_INIT_PARAMS curPortParams;
    GT_U32  curLanesList[HWS_MAX_SERDES_NUM];
    GT_U32  i;

    /* rebuild active lanes list according to current configuration (redundancy) */
    CHECK_STATUS(mvHwsRebuildActiveLaneList(devNum, portGroup, phyPortNum, portMode,curLanesList));
    hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams);

    /*  Configure Serdes to 20 bit mode before executing PCS WA - UnReset for Serdes(RF) */
    for (i = 0; (i < HWS_MAX_SERDES_NUM) && (i < curPortParams.numOfActLanes); i++)
    {
        /* setting the register in 20 bit mode */
        CHECK_STATUS(genUnitRegisterSet(devNum, portGroup, SERDES_UNIT, MV_HWS_SERDES_NUM(curLanesList[i]), COM_H_REV2_Loopback_Reg0, (1 << 1), (7 << 1)));
    }

    /* perform UnReset for Serdes(RF), PCS and MAC units after TRX-Training */
    CHECK_STATUS(mvHwsPortPostTraining(devNum, portGroup, phyPortNum, portMode));

    /* on each related serdes */
    for (i = 0; (i < HWS_MAX_SERDES_NUM) && (i < curPortParams.numOfActLanes); i++)
    {
        /* stop an appropriate engine */
        if (stopTx == GT_TRUE)
        {
            CHECK_STATUS(mvHwsSerdesTxAutoTuneStop(devNum, (portGroup + ((curLanesList[i] >> 16) & 0xFF)),
                (curLanesList[i] & 0xFFFF), HWS_DEV_SERDES_TYPE(devNum, (curLanesList[i] & 0xFFFF))));
        }

        if (stopRx == GT_TRUE)
        {
            CHECK_STATUS(mvHwsSerdesRxAutoTuneStart(devNum, (portGroup + ((curLanesList[i] >> 16) & 0xFF)),
                (curLanesList[i] & 0xFFFF), HWS_DEV_SERDES_TYPE(devNum, (curLanesList[i] & 0xFFFF)), GT_FALSE));
        }
    }

    return GT_OK;
}

/**
* @internal mvHwsPortSetSerdesTuningParam function
* @endinternal
*
* @brief   Get tuning values for changing default tuning parameters.
*         Enables/Disabled eye check, Setting PRESET command (INIT=1 or PRESET=2),
*         Gen1 Amplitude values, initialize Amplitude values preset Amplitude values.
*         Parameter will not change if received value not in the range.
*         Actual configuration (to the device) will be done in mvHwsComHRev2SerdesPostTrainingConfig().
* @param[in] params                   - pointer to structure to store new values of tuning parameters.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortSetSerdesTuningParam
(
    GT_U8                       devNum,
    GT_U32                      portGroup,
    MV_HWS_SERDES_TUNING_PARAMS *params
)
{
    /* avoid warnings */
    devNum = devNum;
    portGroup = portGroup;

    return mvHwsComHRev2SerdesSetTuningParam(params);

}




