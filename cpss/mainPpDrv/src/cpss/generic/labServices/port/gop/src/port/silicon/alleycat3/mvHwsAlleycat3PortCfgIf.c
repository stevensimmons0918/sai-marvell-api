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
#include <cpss/generic/labservices/port/gop/silicon/alleycat3/mvHwsAlleycat3PortIf.h>
#include <cpss/common/labServices/port/gop/port/serdes/mvHwsSerdesPrvIf.h>
#include <cpss/common/labServices/port/gop/port/pcs/mvHwsPcsIf.h>
#include <cpss/common/labServices/port/gop/port/mac/mvHwsMacIf.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/siliconAddress.h>
#include <cpss/common/labServices/port/gop/port/serdes/comPhyH28nm/mvComPhyH28nmIf.h>
#include <cpss/common/labServices/port/gop/port/serdes/comPhyH28nmRev3/mvComPhyH28nmRev3If.h>

/**
* @internal mvHwsAlleycat3PortFixAlign90Ext function
* @endinternal
*
* @brief   Fix Align90 parameters
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAlleycat3PortFixAlign90Ext
(
    IN GT_U8                  devNum,
    IN GT_U32                 portGroup,
    IN GT_U32                 phyPortNum,
    IN MV_HWS_PORT_STANDARD   portMode
)
{
#ifndef AC5_DEV_SUPPORT
    switch (portMode)
    {
    case _10GBase_KR:
    case _20GBase_KR2:
    case _10GBase_SR_LR:
    case _20GBase_SR_LR2:
    case _12GBaseR:
    case _12_1GBase_KR:  /* XLHGL_KR */
        CHECK_STATUS(mvHwsAlleycat3PortFineTune(devNum, portGroup, phyPortNum, portMode, GT_TRUE));
        break;
    default:
        break;
    }
#endif
    return GT_OK;
}

/**
* @internal mvHwsAlleycat5PortRxAutoTuneSetExt function
* @endinternal
*
* @brief   Sets the port Rx parameters according to different working
*         modes/topologies.
*         Can be run any time after create port.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
* @param[in] portTuningMode           - port tuning mode
* @param[in] optAlgoMask              - bit mask for optimization algorithms
*
* @param[out] results                 - results
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
static GT_STATUS mvHwsAlleycat5PortRxAutoTuneSetExt
(
    IN GT_U8                       devNum,
    IN GT_U32                      portGroup,
    IN GT_U32                      phyPortNum,
    IN MV_HWS_PORT_STANDARD        portMode,
    IN MV_HWS_PORT_AUTO_TUNE_MODE  portTuningMode,
    IN GT_U32                      optAlgoMask,
    OUT void *                     results
)
{
    MV_HWS_PORT_INIT_PARAMS curPortParams;
    GT_U32  curLanesList[HWS_MAX_SERDES_NUM];
    GT_U32  i;
    GT_U32 numOfActLanes;

    /* avoid warnings */
    portTuningMode = portTuningMode;
    optAlgoMask = optAlgoMask;

    (void)results; /* avoid warnings */

    if ((phyPortNum >= HWS_CORE_PORTS_NUM(devNum)) |
        ((GT_U32)portMode >= HWS_DEV_PORT_MODES(devNum)))
    {
        return GT_BAD_PARAM;
    }

    if (GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams))
    {
        return GT_BAD_PTR;
    }

    /* rebuild active lanes list according to current configuration (redundancy) */
    CHECK_STATUS(mvHwsRebuildActiveLaneList(devNum, portGroup, phyPortNum, portMode, curLanesList));

    numOfActLanes = curPortParams.numOfActLanes;

    /* on each related serdes */
    for (i = 0; (i < HWS_MAX_SERDES_NUM) && (i < numOfActLanes); i++)
    {
        CHECK_STATUS(mvHwsSerdesRxAutoTuneStart(devNum, (portGroup + ((curLanesList[i] >> 16) & 0xFF)),
                                    (curLanesList[i] & 0xFFFF), HWS_DEV_SERDES_TYPE(devNum, (curLanesList[i] & 0xFFFF)), GT_TRUE));
    }
    return GT_OK;
}

/**
* @internal mvHwsAlleycat5PortTxAutoTuneSetExt function
* @endinternal
*
* @brief   Sets the port Tx only parameters according to different working
*         modes/topologies.
*         Can be run any time after create port.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
* @param[in] portTuningMode           - port TX related tuning mode
*
* @param[out] results                 - results
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
static GT_STATUS mvHwsAlleycat5PortTxAutoTuneSetExt
(
    IN GT_U8                       devNum,
    IN GT_U32                      portGroup,
    IN GT_U32                      phyPortNum,
    IN MV_HWS_PORT_STANDARD        portMode,
    IN MV_HWS_PORT_AUTO_TUNE_MODE  portTuningMode,
    OUT void                       *results
)
{
    GT_U32                          i;
    MV_HWS_PORT_INIT_PARAMS         curPortParams;
    MV_HWS_AUTO_TUNE_STATUS_RES     *tuneRes;
    GT_U32  curLanesList[HWS_MAX_SERDES_NUM];
    GT_U32 numOfActLanes;

    if ((phyPortNum >= HWS_CORE_PORTS_NUM(devNum)) |
        ((GT_U32)portMode >= HWS_DEV_PORT_MODES(devNum)))
    {
        return GT_BAD_PARAM;
    }

    if (GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams))
    {
        return GT_NOT_SUPPORTED;
    }
    /* rebuild active lanes list according to current configuration (redundancy) */
    CHECK_STATUS(mvHwsRebuildActiveLaneList(devNum, portGroup, phyPortNum, portMode, curLanesList));

    numOfActLanes = curPortParams.numOfActLanes;

    switch (portTuningMode)
    {
    case TRxTuneStart:
        /* on each related serdes */
        for (i = 0; (i < HWS_MAX_SERDES_NUM) && (i < numOfActLanes); i++)
        {
            CHECK_STATUS(mvHwsSerdesTxAutoTuneStart(devNum, portGroup,
                                                (curLanesList[i] & 0xFFFF), HWS_DEV_SERDES_TYPE(devNum, (curLanesList[i] & 0xFFFF)), GT_TRUE));
        }
        tuneRes = (MV_HWS_AUTO_TUNE_STATUS_RES*)results;
        tuneRes->txTune = TUNE_READY;
        break;
    case TRxTuneStatusNonBlocking:
        if (results == NULL)
        {
            hwsOsPrintf("Error: results cannot be NULL when using TRxTuneStatusNonBlocking param\n");
            return GT_BAD_PARAM;
        }
        tuneRes = (MV_HWS_AUTO_TUNE_STATUS_RES*)results;

        CHECK_STATUS(mvHwsPortAutoTuneStateCheck(devNum, portGroup, phyPortNum, portMode,
                                                 &tuneRes->rxTune, &tuneRes->txTune));

        break;
    case TRxTuneStop:
        for (i = 0; (i < HWS_MAX_SERDES_NUM) && (i < numOfActLanes); i++)
        {
            CHECK_STATUS(mvHwsSerdesTxAutoTuneStart(devNum, portGroup,
                                                (curLanesList[i] & 0xFFFF), HWS_DEV_SERDES_TYPE(devNum, (curLanesList[i] & 0xFFFF)), GT_FALSE));
        }
        break;
    default:
        hwsOsPrintf("mvHwsAlleycat5PortTxAutoTuneSetExt phyPortNum=%d portTuningMode=%d not implemented\n",phyPortNum,portTuningMode);
        return GT_BAD_PARAM;
    }


    return GT_OK;
}

/**
* @internal mvHwsAlleycat5PortAutoTuneSetExt function
* @endinternal
*
* @brief   Sets the port Tx and Rx parameters according to different working
*         modes/topologies.
*         Can be run any time after create port.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
* @param[in] optAlgoMask              - bit mask for optimization algorithms
* @param[in] portTuningMode           - port tuning mode
*
* @param[out] results                 - results
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAlleycat5PortAutoTuneSetExt
(
    IN GT_U8                        devNum,
    IN GT_U32                       portGroup,
    IN GT_U32                       phyPortNum,
    IN MV_HWS_PORT_STANDARD         portMode,
    IN MV_HWS_PORT_AUTO_TUNE_MODE   portTuningMode,
    IN GT_U32                       optAlgoMask,
    OUT void                        *results
)
{
    /* avoid warnings */
    optAlgoMask = optAlgoMask;

    if (portTuningMode == RxTrainingOnly)
    {
        return mvHwsAlleycat5PortRxAutoTuneSetExt(devNum, portGroup, phyPortNum, portMode, portMode, portTuningMode, results);
    }
    else
    {
        return mvHwsAlleycat5PortTxAutoTuneSetExt(devNum, portGroup, phyPortNum, portMode, portTuningMode, results);
    }
}

#ifndef AC5_DEV_SUPPORT
/**
* @internal mvHwsAlleycat3PortRxAutoTuneSetExt function
* @endinternal
*
* @brief   Sets the port Rx parameters according to different working
*         modes/topologies.
*         Can be run any time after create port.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
* @param[in] portTuningMode           - port tuning mode
* @param[in] optAlgoMask              - bit mask for optimization algorithms
*
* @param[out] results                 - results
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
static GT_STATUS mvHwsAlleycat3PortRxAutoTuneSetExt
(
    IN GT_U8                       devNum,
    IN GT_U32                      portGroup,
    IN GT_U32                      phyPortNum,
    IN MV_HWS_PORT_STANDARD        portMode,
    IN MV_HWS_PORT_AUTO_TUNE_MODE  portTuningMode,
    IN GT_U32                      optAlgoMask,
    OUT void *                     results
)
{
    MV_HWS_SERDES_AUTO_TUNE_RESULTS_UNT res;
    MV_HWS_PORT_INIT_PARAMS curPortParams;
    MV_HWS_AUTO_TUNE_STATUS rxStatus;
    GT_U32  curLanesList[HWS_MAX_SERDES_NUM];
    GT_BOOL laneFail = GT_FALSE;
    GT_U32  i;
    GT_U32 numOfActLanes;
    GT_U32 pollingTime, pi;

    /* avoid warnings */
    portTuningMode = portTuningMode;
    optAlgoMask = optAlgoMask;

    (void)results; /* avoid warnings */

    if ((phyPortNum >= HWS_CORE_PORTS_NUM(devNum)) |
        ((GT_U32)portMode >= HWS_DEV_PORT_MODES(devNum)))
    {
        return GT_BAD_PARAM;
    }

    if (GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams))
    {
        return GT_BAD_PTR;
    }

    /* rebuild active lanes list according to current configuration (redundancy) */
    CHECK_STATUS(mvHwsRebuildActiveLaneList(devNum, portGroup, phyPortNum, portMode, curLanesList));

    numOfActLanes = curPortParams.numOfActLanes;

    /* on each related serdes */
    for (i = 0; (i < HWS_MAX_SERDES_NUM) && (i < numOfActLanes); i++)
    {
        CHECK_STATUS(mvHwsSerdesRxAutoTuneStart(devNum, (portGroup + ((curLanesList[i] >> 16) & 0xFF)),
                                    (curLanesList[i] & 0xFFFF), HWS_DEV_SERDES_TYPE(devNum, (curLanesList[i] & 0xFFFF)), GT_TRUE));
    }
    hwsOsTimerWkFuncPtr(10);

    pollingTime = (HWS_DEV_SILICON_TYPE(devNum) == AC5) ? 500 : 1;
    for (pi = 0; pi < pollingTime; pi++)
    {
        laneFail = GT_FALSE;
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
        if (laneFail == GT_FALSE) break; /* All lanes completed training successfuly */
        hwsOsTimerWkFuncPtr(1);
    }

    if (HWS_DEV_SILICON_TYPE(devNum) != AC5)
    {
        /* get results */
         for (i = 0; (i < HWS_MAX_SERDES_NUM) && (i < numOfActLanes); i++)
        {
            hwsOsMemSetFuncPtr(&res, 0, sizeof(res));
            CHECK_STATUS(mvHwsSerdesAutoTuneResult(devNum, (portGroup + ((curLanesList[i] >> 16) & 0xFF)),
                                    (curLanesList[i] & 0xFFFF), HWS_DEV_SERDES_TYPE(devNum, (curLanesList[i] & 0xFFFF)), &res));
         }
    }

    /* disable training on each related serdes */
    for (i = 0; (i < HWS_MAX_SERDES_NUM) && (i < numOfActLanes); i++)
    {
        CHECK_STATUS(mvHwsSerdesRxAutoTuneStart(devNum, (portGroup + ((curLanesList[i] >> 16) & 0xFF)),
                                (curLanesList[i] & 0xFFFF), HWS_DEV_SERDES_TYPE(devNum, (curLanesList[i] & 0xFFFF)), GT_FALSE));
    }

    if (laneFail == GT_TRUE)
    {
        return GT_FAIL;
    }

    if (HWS_DEV_SILICON_TYPE(devNum) != AC5)
    {
        CHECK_STATUS(mvHwsAlleycat3PortFixAlign90Ext(devNum, portGroup, phyPortNum, portMode));
    }

     /* unreset PCS Rx only */
    hwsOsTimerWkFuncPtr(1);

    return GT_OK;
}


/**
* @internal mvHwsAlleycat3PortTxAutoTuneSetExt function
* @endinternal
*
* @brief   Sets the port Tx only parameters according to different working
*         modes/topologies.
*         Can be run any time after create port.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
* @param[in] portTuningMode           - port TX related tuning mode
*
* @param[out] results                 - results
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
static GT_STATUS mvHwsAlleycat3PortTxAutoTuneSetExt
(
    IN GT_U8                       devNum,
    IN GT_U32                      portGroup,
    IN GT_U32                      phyPortNum,
    IN MV_HWS_PORT_STANDARD        portMode,
    IN MV_HWS_PORT_AUTO_TUNE_MODE  portTuningMode,
    OUT void                       *results
)
{
    MV_HWS_AUTO_TUNE_STATUS_RES *tuneRes;

    if (portTuningMode <= TRxTuneStatus)
    {
     if (portTuningMode == TRxTuneStart)
        {
            if (results == NULL)
            {
                hwsOsPrintf("Error: results cannot be NULL when using TRxTuneStart param\n");
                return GT_BAD_PARAM;
            }
            else
            {
                tuneRes = (MV_HWS_AUTO_TUNE_STATUS_RES*)results;
                tuneRes->txTune = TUNE_PASS;
            }
        }
        CHECK_STATUS(hwsPortTxAutoTuneStartSet(devNum, portGroup, phyPortNum, portMode, portTuningMode, 0/*optAlgoMask*/));
    }

    if (portTuningMode == TRxTuneStatus)
    {
        CHECK_STATUS(mvHwsAlleycat3PortFixAlign90Ext(devNum, portGroup, phyPortNum, portMode));

        /* during Tx config both MAC and PCS are in Reset. Status part only Un-Reset the PCS
           so MAC needs to be Un-Reset as well */
        CHECK_STATUS(hwsPortTxAutoTuneActivateSet(devNum, portGroup, phyPortNum, portMode, portTuningMode));
    }
    else if (portTuningMode == TRxTuneStatusNonBlocking)
    {
        if (results == NULL)
        {
            hwsOsPrintf("Error: results cannot be NULL when using TRxTuneStatusNonBlocking param\n");
            return GT_BAD_PARAM;
        }
        tuneRes = (MV_HWS_AUTO_TUNE_STATUS_RES*)results;

        CHECK_STATUS(mvHwsPortAutoTuneStateCheck(devNum, portGroup, phyPortNum, portMode,
                                                 &tuneRes->rxTune, &tuneRes->txTune));
    }
    else if (portTuningMode == TRxTuneStop)
    {
        /* Fix align90 wll be executed during AP stop )and after that the MAC will be un-reset */
        CHECK_STATUS(mvHwsPortAutoTuneStop(devNum, portGroup, phyPortNum, portMode, GT_FALSE /*Rx*/, GT_TRUE /*Tx*/));
    }

    return GT_OK;
}

/**
* @internal mvHwsAlleycat3PortAutoTuneSetExt function
* @endinternal
*
* @brief   Sets the port Tx and Rx parameters according to different working
*         modes/topologies.
*         Can be run any time after create port.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
* @param[in] optAlgoMask              - bit mask for optimization algorithms
* @param[in] portTuningMode           - port tuning mode
*
* @param[out] results                 - results
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAlleycat3PortAutoTuneSetExt
(
    IN GT_U8                        devNum,
    IN GT_U32                       portGroup,
    IN GT_U32                       phyPortNum,
    IN MV_HWS_PORT_STANDARD         portMode,
    IN MV_HWS_PORT_AUTO_TUNE_MODE   portTuningMode,
    IN GT_U32                       optAlgoMask,
    OUT void                        *results
)
{
    /* avoid warnings */
    optAlgoMask = optAlgoMask;

    if (portTuningMode == RxTrainingOnly)
    {
        if (HWS_DEV_SILICON_TYPE(devNum) == AC5)
        {
            return mvHwsAlleycat5PortRxAutoTuneSetExt(devNum, portGroup, phyPortNum, portMode, portMode, portTuningMode, results);

        }
        else
        {
            return mvHwsAlleycat3PortRxAutoTuneSetExt(devNum, portGroup, phyPortNum, portMode, portMode, portTuningMode, results);
        }
    }
    else if ((portTuningMode >= TRxTuneCfg) && (portTuningMode <= TRxTuneStop))
    {
        return mvHwsAlleycat3PortTxAutoTuneSetExt(devNum, portGroup, phyPortNum, portMode, portTuningMode, results);
    }
    else return GT_BAD_PARAM;
}

/**
* @internal mvHwsAlleycat3PortFineTune function
* @endinternal
*
* @brief   - Set the PPM compensation for Rx/TRX training in 10G and above.
*         Relevant to GOP_28NM_REV1 and above.
*         - Set FixAlign90 for Serdes 28nmRev3 process on current SERDES
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
* @param[in] configPpm                - enable/disable the PPM configuration
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAlleycat3PortFineTune
(
    IN GT_U8                  devNum,
    IN GT_U32                 portGroup,
    IN GT_U32                 phyPortNum,
    IN MV_HWS_PORT_STANDARD   portMode,
    IN GT_BOOL                configPpm
)
{
    MV_HWS_PORT_INIT_PARAMS curPortParams;
    GT_U32 curLanesList[HWS_MAX_SERDES_NUM];
    GT_U32 i;

    if ((phyPortNum >= HWS_CORE_PORTS_NUM(devNum)) ||
        ((GT_U32)portMode >= HWS_DEV_PORT_MODES(devNum)))
    {
        return GT_BAD_PARAM;
    }

    if (GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams))
    {
        return GT_BAD_PTR;
    }

    /* rebuild active lanes list according to current configuration (redundancy) */
    CHECK_STATUS(mvHwsRebuildActiveLaneList(devNum, portGroup, phyPortNum, portMode, curLanesList));

    /* on each related serdes */
    for (i = 0; i < curPortParams.numOfActLanes; i++)
    {
        if (curPortParams.serdesSpeed >= _10_3125G)
        {
            /* Set the PPM compensation on Serdes after Rx/TRX training in 10G and above */
            CHECK_STATUS(mvHwsComH28nmSerdesAdaptPpm(devNum, (portGroup + ((curLanesList[i] >> 16) & 0xFF)),
                                        (curLanesList[i] & 0xFFFF), configPpm));
        }

         /* Run fix Align90 process on SERDES */
         CHECK_STATUS(mvHwsComH28nmRev3SerdesFixAlign90(devNum, (portGroup + ((curLanesList[i] >> 16) & 0xFF)),
                                        (curLanesList[i] & 0xFFFF)));
    }

    return GT_OK;
}
#endif
#if 0
/**
* @internal mvHwsAlleycat3PortBetterAdaptationSet function
* @endinternal
*
* @brief   Run the better Align90 adaptation algorithm on specific port number.
*         The function returns indication if the Better adaptation algorithm
*         succeeded to run on port or not.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - serdes number
*                                      portMode   - port standard metric
*
* @param[out] betterAlign90            - indicates if the Better adaptation algorithm run on
*                                      specific port or not
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS mvHwsAlleycat3PortBetterAdaptationSet
(
    GT_U8       devNum,
    GT_U32      portGroup,
    GT_U32      serdesNum,
    GT_BOOL     *betterAlign90
)
{
    CHECK_STATUS(mvHwsComH28nmRev3FindBetterAdaptation(devNum, portGroup, serdesNum, 300, 10, betterAlign90));

    return GT_OK;
}
#endif

/**
* @internal mvHwsAlleycat3PortExtendedModeCfg function
* @endinternal
*
* @brief   Enable / disable extended mode on port specified.
*         Extended ports supported only in Lion2 and Alleycat3 devices.
*         For Lion2:   1G, 10GBase-R, 20GBase-R2, RXAUI - can be normal or extended
*         XAUI, DXAUI, 40GBase-R - only extended
*         For Alleycat3: ports 25 and 27 can be 10GBase_KR, 10GBase_SR_LR - normal or extended modes
*         port 27 can be 20GBase_KR, 20GBase_SR_LR - only in extended mode
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
* @param[in] extendedMode             - enable / disable
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAlleycat3PortExtendedModeCfg
(
    IN GT_U8                 devNum,
    IN GT_U32                portGroup,
    IN GT_U32                phyPortNum,
    IN MV_HWS_PORT_STANDARD  portMode,
    IN GT_BOOL               extendedMode
)
{
    GT_U32 offset;

    portGroup = portGroup;

    if ((phyPortNum >= HWS_CORE_PORTS_NUM(devNum)) ||
        ((GT_U32)portMode >= HWS_DEV_PORT_MODES(devNum)))
    {
        return GT_BAD_PARAM;
    }

    /* for Alleycat3A0: extended ports 25 or 27
    In Alleycat3 there is a Mux for DMA unit between regular and extended Stacking ports 25,27-29 in MSMS4 unit
    The selector (port RX interface) for changing the configuration exists
    in XLGMAC unit with MAC number 24 in External_control register:
    - External_Control_0 (bit 0) - select between regular port 25 (value=0, internal MAC) and extended port 28 (value=1, external client 1)
    - External_Control_1 (bit 1) - select between regular port 27 (value=0, internal MAC) and extended port 29 (value=1, external client 3) */
    if (phyPortNum != 25 && phyPortNum != 27)
    {
        return GT_OK;
    }

    offset = (phyPortNum == 25) ? 0 : 1;
    CHECK_STATUS(genUnitRegisterSet(devNum, portGroup, XLGMAC_UNIT, 24, EXTERNAL_CONTROL, (extendedMode << offset), (1 << offset)));

    return GT_OK;
}

/**
* @internal mvHwsAlleycat3PortExtendedModeCfgGet function
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
GT_STATUS mvHwsAlleycat3PortExtendedModeCfgGet
(
    IN GT_U8                    devNum,
    IN GT_U32                   portGroup,
    IN GT_U32                   phyPortNum,
    IN MV_HWS_PORT_STANDARD     portMode,
    OUT GT_BOOL                 *extendedMode
)
{
    GT_UREG_DATA data;
    GT_U32 offset;

    if ((GT_U32)portMode >= HWS_DEV_PORT_MODES(devNum))
    {
        return GT_BAD_PARAM;
    }

    if (phyPortNum != 25 && phyPortNum != 27)
    {
        *extendedMode = GT_FALSE;
        return GT_OK;
    }

    CHECK_STATUS(genUnitRegisterGet(devNum, portGroup, XLGMAC_UNIT, 24, EXTERNAL_CONTROL, &data, 0));

    /* In Alleycat3:
    - for port#25 - if bit 0 in External_Control reg is 0x1, the port is in Extended mode
    - for port#27 - if bit 1 in External_Control reg is 0x1, the port is in Extended mode */
    offset = (phyPortNum == 25) ? 0 : 1;
    *extendedMode = ((data >> offset) & 0x1) ? GT_TRUE : GT_FALSE;

    return GT_OK;
}

/**
* @internal mvHwsAlleycat3PortPsyncBypassCfg function
* @endinternal
*
* @brief   Configures the bypass of synchronization module.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAlleycat3PortPsyncBypassCfg
(
    IN GT_U8                   devNum,
    IN GT_U32                  portGroup,
    IN GT_U32                  phyPortNum,
    IN MV_HWS_PORT_STANDARD    portMode
)
{
    MV_HWS_PORT_INIT_PARAMS   curPortParams;
    GT_UOPT macNum;

    if (GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams))
    {
        return GT_NOT_SUPPORTED;
    }

    macNum = curPortParams.portMacNumber;

    /* for Network ports */
    if (phyPortNum <= 23)
    {
        if ((portMode == SGMII2_5) || (portMode == _2500Base_X))
        {
            /* set P Sync Bypass */
            CHECK_STATUS(genUnitRegisterSet(devNum, portGroup, GEMAC_UNIT, macNum,
                                GIG_PORT_MAC_CONTROL_REGISTER4, (1 << 6), (1 << 6)));
            return GT_OK;
        }
        else
        {
            /* unset P Sync Bypass */
            CHECK_STATUS(genUnitRegisterSet(devNum, portGroup, GEMAC_UNIT, macNum,
                                GIG_PORT_MAC_CONTROL_REGISTER4, 0, (1 << 6)));
            return GT_OK;
        }
    }

    /* for Stacking ports */
    if ((phyPortNum >= 24) && (phyPortNum <= 29))
    {
        /* set P Sync Bypass */
        CHECK_STATUS(genUnitRegisterSet(devNum, portGroup, GEMAC_UNIT, macNum,
                            GIG_PORT_MAC_CONTROL_REGISTER4, (1 << 6), (1 << 6)));
        return GT_OK;
    }

    /* for CPU port */
    if (phyPortNum == 31)
    {
        /* unset P Sync Bypass */
        CHECK_STATUS(genUnitRegisterSet(devNum, portGroup, GEMAC_UNIT, macNum,
                            GIG_PORT_MAC_CONTROL_REGISTER4, 0, (1 << 6)));
        return GT_OK;
    }

    /* unset P Sync Bypass */
    CHECK_STATUS(genUnitRegisterSet(devNum, portGroup, GEMAC_UNIT, curPortParams.portMacNumber,
                        GIG_PORT_MAC_CONTROL_REGISTER4, 0, (1 << 6)));

    return GT_OK;
}



