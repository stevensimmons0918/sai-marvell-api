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
#include <cpss/generic/labservices/port/gop/silicon/hooper/mvHwsHooperPortIf.h>
#include <cpss/common/labServices/port/gop/port/serdes/mvHwsSerdesPrvIf.h>
#include <cpss/common/labServices/port/gop/port/pcs/mvHwsPcsIf.h>
#include <cpss/common/labServices/port/gop/port/mac/mvHwsMacIf.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/siliconAddress.h>

extern GT_STATUS mvHwsLion2PortRxAutoTuneSetExt (GT_U8 devNum, GT_U32  portGroup, GT_U32  phyPortNum,
                         MV_HWS_PORT_STANDARD portMode, MV_HWS_PORT_AUTO_TUNE_MODE portTuningMode,
                         GT_U32 optAlgoMask, void *results);

extern GT_STATUS mvHwsLion2PortTxAutoTuneSetExt(GT_U8 devNum, GT_U32 portGroup, GT_U32 phyPortNum,
                         MV_HWS_PORT_STANDARD portMode, MV_HWS_PORT_AUTO_TUNE_MODE portTuningMode,
                         GT_U32 optAlgoMask);

extern GT_STATUS mvHwsLion2PortTxAutoTuneSet (GT_U8 devNum, GT_U32  portGroup, GT_U32  phyPortNum,
                         MV_HWS_PORT_STANDARD portMode, MV_HWS_PORT_AUTO_TUNE_MODE portTuningMode,
                         GT_U32 optAlgoMask);

/**
* @internal mvHwsHooperPCSMarkModeSet function
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
GT_STATUS mvHwsHooperPCSMarkModeSet
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
            hwsOsPrintf("mvHwsPCSMarkModeSet: portMode %d is not supported for Mark/Un-mark PCS unit\n", portMode);
            return GT_BAD_PARAM;
    }

    return GT_OK;
}




