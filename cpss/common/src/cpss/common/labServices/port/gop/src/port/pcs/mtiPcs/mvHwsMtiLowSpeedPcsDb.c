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
* @file mvHwsMtiLowSpeedPcsDb.c
*
* @brief
*
* @version   1
********************************************************************************
*/
#include <cpss/common/labServices/port/gop/common/configElementDb/mvCfgSeqDbIf.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/siliconAddress.h>
#include <cpss/common/labServices/port/gop/port/pcs/mtiPcs/mvHwsMtiPcsDb.h>

static const MV_OP_PARAMS mtiLowSpeedPcs1000BaseXModeSeqParams[] = {
    {MTI_LOW_SP_PCS_UNIT,     MTI_LPCS_GMODE,     0x1,      0x1},   /* Per channel 1G PCS enable */
    {MTI_LOW_SP_PCS_UNIT,     MTI_LPCS_IF_MODE,   0x0,      0x1},   /* Choses between 1000BASE-X and SGMII */
    {MTI_LOW_SP_PCS_UNIT,     MTI_LPCS_IF_MODE,   0x0,      0x2},   /* Currently not supported AutoNegotiation */
    {MTI_LOW_SP_PCS_UNIT,     MTI_LPCS_IF_MODE,   0x8,      0xC},   /* choose speed {10M,100M,1000M} */
    {MTI_LOW_SP_PCS_UNIT,     MTI_LPCS_IF_MODE,   0x0,      0x40},  /* ifmode_rx_preamble_sync */
    {MTI_LOW_SP_PCS_UNIT,     MTI_LPCS_IF_MODE,   0x0,      0x80},  /* ifmode_tx_preamble_sync */
    {MTI_LOW_SP_PCS_UNIT,     MTI_LPCS_CONTROL,   0x0,    0x1000},  /* disable 'ap_enable' */
#if 0
    {UNIT_MTI_LOW_SP_PCS,     MTI_LPCS_CONTROL,   0x8000, 0x8000},  /* self-cleared reset */
#endif
};

static const MV_OP_PARAMS mtiLowSpeedPcs1000BaseXPowerDownSeqParams[] = {
    {MTI_LOW_SP_PCS_UNIT,     MTI_LPCS_GMODE,     0x0,      0x1},   /* Per channel 1G PCS enable */
    {MTI_LOW_SP_PCS_UNIT,     MTI_LPCS_IF_MODE,   0x0,      0x1},   /* Choses between 1000BASE-X and SGMII */
    {MTI_LOW_SP_PCS_UNIT,     MTI_LPCS_IF_MODE,   0x0,      0x2},   /* Currently not supported AutoNegotiation */
    {MTI_LOW_SP_PCS_UNIT,     MTI_LPCS_IF_MODE,   0x0,      0xC},   /* choose speed {10M,100M,1000M} */
    {MTI_LOW_SP_PCS_UNIT,     MTI_LPCS_IF_MODE,   0x0,      0x40},  /* ifmode_rx_preamble_sync */
    {MTI_LOW_SP_PCS_UNIT,     MTI_LPCS_IF_MODE,   0x0,      0x80},  /* ifmode_tx_preamble_sync */
    {MTI_LOW_SP_PCS_UNIT,     MTI_LPCS_CONTROL,   0x0,    0x1000},  /* disable 'ap_enable' */
};

static const MV_OP_PARAMS mtiLowSpeedPcs1000BaseXresetSeqParams[] = {
    {MTI_LOW_SP_PCS_UNIT,     MTI_LPCS_CONTROL,     /*dummy write 0x8000*/0x0000,     0x8000}  /*CONTROL.Reset.set(0)*/

};

static const MV_OP_PARAMS mtiLowSpeedPcs1000BaseXunresetSeqParams[] = {
    {MTI_LOW_SP_PCS_UNIT,     MTI_LPCS_CONTROL,   0x8000,     0x8000}  /* self-cleared CONTROL.Reset.set(1) */
};

static const MV_OP_PARAMS mtiLowSpeedPcsLpDisableSeqParams[] = {
    {MTI_LOW_SP_PCS_UNIT,     MTI_PCS_CONTROL1,       0x0,        0x4000}
};

static const MV_OP_PARAMS mtiLowSpeedPcsLpTxRxSeqParams[] = {
    {MTI_LOW_SP_PCS_UNIT,     MTI_PCS_CONTROL1,       0x4000,     0x4000}
};

/* seqId to params array mapping */
const MV_MAC_PCS_CFG_SEQ hwsMtiLowSpeedPcsSeqDb[] =
{
    {mtiLowSpeedPcs1000BaseXModeSeqParams,  MV_SEQ_SIZE(mtiLowSpeedPcs1000BaseXModeSeqParams)},     /* MTI_LOW_SPEED_PCS_1000_BASE_X_MODE_SEQ */
    {mtiLowSpeedPcs1000BaseXPowerDownSeqParams, MV_SEQ_SIZE(mtiLowSpeedPcs1000BaseXPowerDownSeqParams)},     /* MTI_LOW_SPEED_PCS_1000_BASE_X_POWER_DOWN_SEQ */
    {mtiLowSpeedPcs1000BaseXresetSeqParams, MV_SEQ_SIZE(mtiLowSpeedPcs1000BaseXresetSeqParams)},    /* MTI_LOW_SPEED_PCS_1000_BASE_X_RESET_SEQ */
    {mtiLowSpeedPcs1000BaseXunresetSeqParams, MV_SEQ_SIZE(mtiLowSpeedPcs1000BaseXunresetSeqParams)},/* MTI_LOW_SPEED_PCS_1000_BASE_X_UNRESET_SEQ */
    {mtiLowSpeedPcsLpDisableSeqParams, MV_SEQ_SIZE(mtiLowSpeedPcsLpDisableSeqParams)},       /* MTI_LOW_SPEED_PCS_LPBK_NORMAL_SEQ */
    {mtiLowSpeedPcsLpTxRxSeqParams, MV_SEQ_SIZE(mtiLowSpeedPcsLpTxRxSeqParams)},          /* MTI_LOW_SPEED_PCS_LPBK_TX2RX_SEQ */

};

GT_STATUS hwsMtiLowSpeedPcsSeqGet(MV_HWS_MTI_LOW_SPEED_PCS_SUB_SEQ seqType, MV_OP_PARAMS *seqLine, GT_U32 lineNum)
{
  if (seqType >= MTI_LOW_SPEED_PCS_LAST_SEQ)
  {
      return GT_NO_SUCH;
  }

  *seqLine = hwsMtiLowSpeedPcsSeqDb[seqType].cfgSeq[lineNum];
  return GT_OK;
}





