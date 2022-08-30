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
* @file mvHwsTcamMacIf.c
*
* @brief CG MAC reset interface
*
* @version   3
********************************************************************************
*/
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/siliconAddress.h>
#include <cpss/common/labServices/port/gop/port/mac/tcamMac/mvHwsTcamMacIf.h>
#include <cpss/common/labServices/port/gop/port/mac/tcamMac/mvHwsTcamMacDb.h>

#include <cpss/common/labServices/port/gop/port/mvPortModeElements.h>

static char* mvHwsMacTypeGet(void)
{
  return "TCAMMAC";
}

/**
* @internal hwsTcamMacIfInit function
* @endinternal
*
* @brief   Init XLG MAC configuration sequences and IF functions.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS hwsTcamMacIfInit(MV_HWS_MAC_FUNC_PTRS **funcPtrArray)
{
    funcPtrArray[TCAM_MAC] = (MV_HWS_MAC_FUNC_PTRS*)hwsOsMallocFuncPtr(sizeof(MV_HWS_MAC_FUNC_PTRS));
    if(!funcPtrArray[TCAM_MAC])
    {
        return GT_NO_RESOURCE;
    }
    funcPtrArray[TCAM_MAC]->macRestFunc = mvHwsTcamMacReset;
    funcPtrArray[TCAM_MAC]->macModeCfgFunc = mvHwsTcamMacModeCfg;
    funcPtrArray[TCAM_MAC]->macLinkGetFunc = mvHwsTcamMacLinkStatus;
    funcPtrArray[TCAM_MAC]->macTypeGetFunc = mvHwsMacTypeGet;

    return GT_OK;
}


/**
* @internal mvHwsTcamMacReset function
* @endinternal
*
* @brief   Set the GE MAC to reset or exit from reset.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] macNum                   - MAC number
* @param[in] portMode                 - port mode
* @param[in] action                   - normal or reset
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsTcamMacReset
(
    IN GT_U8                   devNum,
    IN GT_U32                  portGroup,
    IN GT_U32                  portMacNum,
    IN MV_HWS_PORT_STANDARD    portMode,
    IN MV_HWS_RESET            action
)
{
    MV_MAC_PCS_CFG_SEQ_PTR seq;
    portMode = portMode;

    if ((RESET == action) || (POWER_DOWN == action))
    {
        seq = &hwsTcamMacSeqDb[TCAMMAC_RESET_SEQ];
    }
    else if (UNRESET == action)
    {
        seq = &hwsTcamMacSeqDb[TCAMMAC_UNRESET_SEQ];
    }
    else
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "undefined reset action resetMode = %d \n", action);
    }

    CHECK_STATUS(mvCfgMacPcsSeqExec(devNum, portGroup, portMacNum, seq->cfgSeq, seq->cfgSeqSize));

    return GT_OK;
}

/**
* @internal mvHwsTcamMacModeCfg function
* @endinternal
*
* @brief   Set the internal mux's to the required MAC in the GOP.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] portMacNum               - MAC number
* @param[in] portMode                 - port mode
* @param[in] attributesPtr            - port attributes
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsTcamMacModeCfg
(
    IN GT_U8                   devNum,
    IN GT_U32                  portGroup,
    IN GT_U32                  portMacNum,
    IN MV_HWS_PORT_STANDARD    portMode,
    IN MV_HWS_PORT_ATTRIBUTES_INPUT_PARAMS *attributesPtr
)
{
    MV_HWS_PORT_INIT_PARAMS curPortParams;
    MV_MAC_PCS_CFG_SEQ_PTR seq;

    GT_UNUSED_PARAM(attributesPtr);

    if (GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, portMacNum, portMode, &curPortParams))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    seq = &hwsTcamMacSeqDb[TCAMMAC_MODE_SEQ];
    CHECK_STATUS(mvCfgMacPcsSeqExec(devNum, portGroup, curPortParams.portMacNumber, seq->cfgSeq, seq->cfgSeqSize));

    return GT_OK;
}

/**
* @internal mvHwsTcamMacLinkStatus function
* @endinternal
*
* @brief   Get MAC link status.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] portMacNum               - MAC number
* @param[in] portMode                 - port mode
*
* @param[out] linkStatus              - link status
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsTcamMacLinkStatus
(
    IN GT_U8                   devNum,
    IN GT_U32                  portGroup,
    IN GT_U32                  macNum,
    IN MV_HWS_PORT_STANDARD    portMode,
    OUT GT_BOOL                 *linkStatus
)
{
    GT_U32 data;
    GT_U32 accessAddr;
    GT_U32 unitIndex;
    GT_U32 baseAddr;

    portMode = portMode;
    CHECK_STATUS(mvUnitInfoGet(devNum, ETI_ILKN_RF_UNIT, &baseAddr, &unitIndex));
    accessAddr  = baseAddr + ILKN_0_ALIGN_STATUS + macNum * unitIndex;

    CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, portGroup, accessAddr, &data, 0));

    *linkStatus = (((data >> 1) & 3) == 3) ? GT_TRUE : GT_FALSE;

    return GT_OK;
}
