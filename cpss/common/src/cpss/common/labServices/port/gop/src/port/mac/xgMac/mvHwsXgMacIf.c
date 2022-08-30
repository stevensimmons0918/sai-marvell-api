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
* @file mvHwsXgMacIf.c
*
* @brief XGL MAC reset interface
*
* @version   20
********************************************************************************
*/
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/siliconAddress.h>
#include <cpss/common/labServices/port/gop/port/mac/xgMac/mvHwsXgMacIf.h>
#include <cpss/common/labServices/port/gop/port/mac/xlgMac/mvHwsXlgMacDb.h>
#include <cpss/common/labServices/port/gop/port/mac/xgMac/mvHwsXgMacDb.h>

#include <cpss/common/labServices/port/gop/port/mvPortModeElements.h>

static char* mvHwsMacTypeGet(void)
{
  return "XGMAC";
}

/**
* @internal hwsXgMacIfInit function
* @endinternal
*
* @brief   Init XG MAC configuration sequences and IF functions.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS hwsXgMacIfInit(MV_HWS_MAC_FUNC_PTRS **funcPtrArray)
{
    if(!funcPtrArray[XGMAC])
    {
        funcPtrArray[XGMAC] = (MV_HWS_MAC_FUNC_PTRS*)hwsOsMallocFuncPtr(sizeof(MV_HWS_MAC_FUNC_PTRS));
        if(!funcPtrArray[XGMAC])
        {
            return GT_NO_RESOURCE;
        }
        hwsOsMemSetFuncPtr(funcPtrArray[XGMAC], 0, sizeof(MV_HWS_MAC_FUNC_PTRS));
    }
    funcPtrArray[XGMAC]->macRestFunc = mvHwsXgMacReset;
    funcPtrArray[XGMAC]->macModeCfgFunc = mvHwsXgMacModeCfg;
    funcPtrArray[XGMAC]->macLinkGetFunc = mvHwsXgMacLinkStatus;
    funcPtrArray[XGMAC]->macLbCfgFunc = mvHwsXgMacLoopbackCfg;
#ifndef CO_CPU_RUN
    funcPtrArray[XGMAC]->macLbStatusGetFunc = mvHwsXgMacLoopbackStatusGet;
    funcPtrArray[XGMAC]->macActiveStatusFunc = mvHwsXgMacActiveStatusGet;
#endif
    funcPtrArray[XGMAC]->macTypeGetFunc = mvHwsMacTypeGet;

  return GT_OK;
}


/**
* @internal mvHwsXgMacReset function
* @endinternal
*
* @brief   Set the XG MAC to reset or exit from reset.
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
GT_STATUS mvHwsXgMacReset
(
    IN GT_U8                   devNum,
    IN GT_U32                  portGroup,
    IN GT_U32                  portMacNum,
    IN MV_HWS_PORT_STANDARD    portMode,
    IN MV_HWS_RESET            action
)
{
    MV_MAC_PCS_CFG_SEQ_PTR seq;
    GT_U32 accessAddr;
    GT_U32 baseAddr;
    GT_U32 unitIndex;
    GT_U32 data;

    portMode =  portMode;
    if ((RESET == action) || (POWER_DOWN == action))
    {
        seq = &hwsXlgMacSeqDb[XLGMAC_RESET_SEQ];
        data = 0;
    }
    else if (UNRESET == action)
    {
        seq = &hwsXlgMacSeqDb[XLGMAC_UNRESET_SEQ];
        data = 1;
    }
    else
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "undefined reset action resetMode = %d \n", action);
    }

    CHECK_STATUS(mvCfgMacPcsSeqExec(devNum, portGroup, portMacNum, seq->cfgSeq, seq->cfgSeqSize));

    CHECK_STATUS(mvUnitInfoGet(devNum, XLGMAC_UNIT, &baseAddr, &unitIndex));
    accessAddr  = baseAddr + MACRO_CONTROL + 8 * unitIndex;

    if (portMacNum == 12)
    {
        CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, portGroup, accessAddr, (data << 5), (1 << 5)));
    }
    if (portMacNum == 14)
    {
        CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, portGroup, accessAddr, (data << 7), (1 << 7)));
    }

    return GT_OK;
}

/**
* @internal mvHwsXgMacModeCfg function
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
GT_STATUS mvHwsXgMacModeCfg
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
    GT_U32 accessAddr;
    GT_U32 baseAddr;
    GT_U32 unitIndex;

    GT_UNUSED_PARAM(attributesPtr);

    if (GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, portMacNum, portMode, &curPortParams))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    CHECK_STATUS(mvUnitInfoGet(devNum, XLGMAC_UNIT, &baseAddr, &unitIndex));

    accessAddr  = baseAddr + MACRO_CONTROL + 8 * unitIndex;
    if (curPortParams.portMacNumber == 12)
    {
        CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, portGroup, accessAddr, (1 << 5), (1 << 5)));
    }

    if (curPortParams.portMacNumber == 14)
    {
        CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, portGroup, accessAddr, (1 << 7), (1 << 7)));
    }

    seq = &hwsXgMacSeqDb[XGMAC_MODE_SEQ];
    CHECK_STATUS(mvCfgMacPcsSeqExec(devNum, portGroup, curPortParams.portMacNumber, seq->cfgSeq, seq->cfgSeqSize));

    return GT_OK;
}

/**
* @internal mvHwsXgMacLoopbackCfg function
* @endinternal
*
* @brief   Configure MAC loopback.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] macNum                   - MAC number
* @param[in] portMode                 - port mode
* @param[in] lbType                   - supported loopback type
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsXgMacLoopbackCfg
(
    IN GT_U8                   devNum,
    IN GT_U32                  portGroup,
    IN GT_U32                  macNum,
    IN MV_HWS_PORT_STANDARD    portMode,
    IN MV_HWS_PORT_LB_TYPE     lbType
)
{
    MV_MAC_PCS_CFG_SEQ_PTR seq;

    portMode =  portMode;

    switch (lbType)
    {
    case DISABLE_LB:
      seq = &hwsXlgMacSeqDb[XLGMAC_LPBK_NORMAL_SEQ];
      break;
    case RX_2_TX_LB:
      seq = &hwsXlgMacSeqDb[XLGMAC_LPBK_RX2TX_SEQ];
      break;
    case TX_2_RX_LB:
      seq = &hwsXlgMacSeqDb[XLGMAC_LPBK_TX2RX_SEQ];
      break;
    default:
      return GT_BAD_PARAM;
    }

    CHECK_STATUS(mvCfgMacPcsSeqExec(devNum, portGroup, macNum, seq->cfgSeq, seq->cfgSeqSize));

    return GT_OK;
}

#ifndef CO_CPU_RUN
/**
* @internal mvHwsXgMacLoopbackStatusGet function
* @endinternal
*
* @brief   Retrive MAC loopback status.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] macNum                   - MAC number
* @param[in] portMode                 - port mode
*
* @param[out] lbType                   - supported loopback type
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsXgMacLoopbackStatusGet
(
    IN GT_U8                   devNum,
    IN GT_U32                  portGroup,
    IN GT_U32                  macNum,
    IN MV_HWS_PORT_STANDARD    portMode,
    OUT MV_HWS_PORT_LB_TYPE     *lbType
)
{
    GT_U32 data;
    GT_U32 accessAddr;
    GT_U32 unitIndex;
    GT_U32 baseAddr;

    portMode = portMode;
    CHECK_STATUS(mvUnitInfoGet(devNum, XLGMAC_UNIT, &baseAddr, &unitIndex));
    accessAddr  = baseAddr + MSM_PORT_MAC_CONTROL_REGISTER1 + macNum * unitIndex;

    CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, portGroup, accessAddr, &data, 0));

    switch ((data >> 13) & 3)
    {
    case 0:
        *lbType = DISABLE_LB;
      break;
    case 1:
        *lbType = TX_2_RX_LB;
      break;
    case 2:
        *lbType = RX_2_TX_LB;
      break;
    default:
      return GT_NOT_SUPPORTED;
    }

    return GT_OK;
}
#endif

/**
* @internal mvHwsXgMacLinkStatus function
* @endinternal
*
* @brief   Get MAC link status.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] macNum                   - MAC number
* @param[in] portMode                 - port mode
*
* @param[out] linkStatus              - link status
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsXgMacLinkStatus
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
    CHECK_STATUS(mvUnitInfoGet(devNum, XLGMAC_UNIT, &baseAddr, &unitIndex));
    accessAddr  = baseAddr + MSM_PORT_STATUS + macNum * unitIndex;

    CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, portGroup, accessAddr, &data, 0));

    *linkStatus = data & 1;
    return GT_OK;
}

#ifndef CO_CPU_RUN
/**
* @internal mvHwsXgMacActiveStatusGet function
* @endinternal
*
* @brief   Return number of MAC active lanes or 0, if current MAC isn't active.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] macNum                   - MAC number
* @param[in] portMode                 - port mode
*
* @param[out] numOfLanes               - number of lanes agregated in PCS
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsXgMacActiveStatusGet
(
    IN GT_U8                   devNum,
    IN GT_U32                  portGroup,
    IN GT_U32                  macNum,
    IN MV_HWS_PORT_STANDARD    portMode,
    OUT GT_U32                  *numOfLanes
)
{
    GT_U32 data;
    GT_U32 baseAddr;
    GT_U32 unitIndex;
    GT_U32 accessAddr;

    portMode = portMode;
    if (numOfLanes == NULL)
    {
        return GT_BAD_PARAM;
    }

    CHECK_STATUS(mvUnitInfoGet(devNum, XLGMAC_UNIT, &baseAddr, &unitIndex));

    *numOfLanes = 0;
    accessAddr = baseAddr + MSM_PORT_MAC_CONTROL_REGISTER0 + macNum * unitIndex;
    CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, portGroup, accessAddr, &data, 0));
    if (((data >> 1) & 1) == 0)
    {
        /* unit is under RESET */
        return GT_OK;
    }
    /* check if XPCS used */
    accessAddr  = baseAddr + MSM_PORT_MAC_CONTROL_REGISTER4 + macNum * unitIndex;
    CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, portGroup, accessAddr, &data, 0));
    if (((data >> 8) & 1) == 0)
    {
        /* XPCS doesn't used */
        return GT_OK;
    }

    *numOfLanes = 1;

    return GT_OK;
}
#endif

