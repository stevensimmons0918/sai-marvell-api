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
* @file mvHwsXlgMacDb.c
*
* @brief
*
* @version   20
********************************************************************************
*/
#include <cpss/common/labServices/port/gop/common/configElementDb/mvCfgSeqDbIf.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/siliconAddress.h>
#include <cpss/common/labServices/port/gop/port/mac/xlgMac/mvHwsXlgMacDb.h>

static const MV_OP_PARAMS mode1LaneSeqParams[] = {
    {XLGMAC_UNIT, MSM_PORT_MAC_CONTROL_REGISTER3, (1 << 13), (7 << 13)},
    {XLGMAC_UNIT, MSM_PORT_MAC_CONTROL_REGISTER4,  0x0,      (1 << 8)},
    {XLGMAC_UNIT, MSM_PORT_MAC_CONTROL_REGISTER4, (1 << 4),  (1 << 4)},
    {XLGMAC_UNIT, MSM_PORT_MAC_CONTROL_REGISTER0,  0x0,      (1 << 11)}
};

static const MV_OP_PARAMS mode2LaneSeqParams[] = {
    {XLGMAC_UNIT, MSM_PORT_MAC_CONTROL_REGISTER3, (2 << 13), (7 << 13)},
    {XLGMAC_UNIT, MSM_PORT_MAC_CONTROL_REGISTER4,  0x0,      (1 << 8)},
    {XLGMAC_UNIT, MSM_PORT_MAC_CONTROL_REGISTER4, (1 << 4),  (1 << 4)},
    {XLGMAC_UNIT, MSM_PORT_MAC_CONTROL_REGISTER0,  0x0,      (1 << 11)}
};

static const MV_OP_PARAMS mode4LaneSeqParams[] = {
    {XLGMAC_UNIT, MSM_PORT_MAC_CONTROL_REGISTER3, (3 << 13), (7 << 13)},
    {XLGMAC_UNIT, MSM_PORT_MAC_CONTROL_REGISTER4,  0x0,      (1 << 8)},
    {XLGMAC_UNIT, MSM_PORT_MAC_CONTROL_REGISTER4, (1 << 4),  (1 << 4)},
    {XLGMAC_UNIT, MSM_PORT_MAC_CONTROL_REGISTER0,  0x0,      (1 << 11)}
};

static const MV_OP_PARAMS resetSeqParams[] = {
    {XLGMAC_UNIT, MSM_PORT_MAC_CONTROL_REGISTER0,  0x0,      (1 << 1)}
};

static const MV_OP_PARAMS unresetSeqParams[] = {
    {XLGMAC_UNIT, MSM_PORT_MAC_CONTROL_REGISTER0, (1 << 1),  (1 << 1)}
};

static const MV_OP_PARAMS lbNormalSeqParams[] = {
    {XLGMAC_UNIT, MSM_PORT_MAC_CONTROL_REGISTER1,  0x0,      (3 << 13)}
};

static const MV_OP_PARAMS lbRx2TxSeqParams[] = {
    {XLGMAC_UNIT, MSM_PORT_MAC_CONTROL_REGISTER1, (2 << 13), (3 << 13)}
};

static const MV_OP_PARAMS lbTx2RxSeqParams[] = {
    {XLGMAC_UNIT, MSM_PORT_MAC_CONTROL_REGISTER1, (1 << 13), (3 << 13)}
};

/* seqId to params array mapping */
const MV_MAC_PCS_CFG_SEQ hwsXlgMacSeqDb[MV_MAC_XLG_LAST_SEQ] =
{
     {unresetSeqParams,   MV_SEQ_SIZE(unresetSeqParams)}   /* XLGMAC_UNRESET_SEQ     */
    ,{resetSeqParams,     MV_SEQ_SIZE(resetSeqParams)}     /* XLGMAC_RESET_SEQ       */
    ,{mode1LaneSeqParams, MV_SEQ_SIZE(mode1LaneSeqParams)} /* XLGMAC_MODE_1_Lane_SEQ */
    ,{mode2LaneSeqParams, MV_SEQ_SIZE(mode2LaneSeqParams)} /* XLGMAC_MODE_2_Lane_SEQ */
    ,{mode4LaneSeqParams, MV_SEQ_SIZE(mode4LaneSeqParams)} /* XLGMAC_MODE_4_Lane_SEQ */
    ,{lbNormalSeqParams,  MV_SEQ_SIZE(lbNormalSeqParams)}  /* XLGMAC_LPBK_NORMAL_SEQ */
    ,{lbRx2TxSeqParams,   MV_SEQ_SIZE(lbRx2TxSeqParams)}   /* XLGMAC_LPBK_RX2TX_SEQ  */
    ,{lbTx2RxSeqParams,   MV_SEQ_SIZE(lbTx2RxSeqParams)}   /* XLGMAC_LPBK_TX2RX_SEQ  */
    ,{NULL,               MV_SEQ_SIZE(0)}                  /* XLGMAC_FC_DISABLE_SEQ  */
    ,{NULL,               MV_SEQ_SIZE(0)}                  /* XLGMAC_FC_BOTH_SEQ     */
    ,{NULL,               MV_SEQ_SIZE(0)}                  /* XLGMAC_FC_RX_ONLY_SEQ  */
    ,{NULL,               MV_SEQ_SIZE(0)}                  /* XLGMAC_FC_TX_ONLY_SEQ  */
    ,{NULL,               MV_SEQ_SIZE(0)}                  /* XLGMAC_POWER_DOWN_SEQ  */
    ,{NULL,               MV_SEQ_SIZE(0)}                  /* XLGMAC_MODE_1_Lane_25G_SEQ */
    ,{NULL,               MV_SEQ_SIZE(0)}                  /* XLGMAC_MODE_2_Lane_50G_SEQ */
    ,{NULL,               MV_SEQ_SIZE(0)}                  /* XLGMAC_MODE_1_Lane_25G_RS_FEC_SEQ */
    ,{NULL,               MV_SEQ_SIZE(0)}                  /* XLGMAC_MODE_2_Lane_50G_RS_FEC_SEQ */
    ,{NULL,               MV_SEQ_SIZE(0)}                  /* XLGMAC_MODE_4_Lane_MLG_SEQ */
    ,{NULL,               MV_SEQ_SIZE(0)}                  /* XLGMAC_RESET_4_Lane_MLG_SEQ */
    ,{NULL,               MV_SEQ_SIZE(0)}                  /* XLGMAC_UNRESET_4_Lane_MLG_SEQ */
    ,{NULL,               MV_SEQ_SIZE(0)}                  /* XLGMAC_POWER_DOWN_4_Lane_MLG_SEQ */
#ifndef MV_HWS_REDUCED_BUILD
    ,{NULL,               MV_SEQ_SIZE(0)}                  /* XLGMAC_MODE_1_Lane_UPDATE_SEQ */
    ,{NULL,               MV_SEQ_SIZE(0)}                  /* XLGMAC_MODE_2_Lane_UPDATE_SEQ */
    ,{NULL,               MV_SEQ_SIZE(0)}                  /* XLGMAC_MODE_4_Lane_UPDATE_SEQ */
    ,{NULL,               MV_SEQ_SIZE(0)}                  /* XLGMAC_MODE_2_Lane_FULL_UPDATE_SEQ */
#endif
};

GT_STATUS hwsXlgMacSeqGet(MV_HWS_XLG_MAC_SUB_SEQ seqType, MV_OP_PARAMS *seqLine, GT_U32 lineNum)
{
  if ((seqType >= MV_MAC_XLG_LAST_SEQ) ||
      (hwsXlgMacSeqDb[seqType].cfgSeq == NULL))
  {
      return GT_NO_SUCH;
  }

  *seqLine = hwsXlgMacSeqDb[seqType].cfgSeq[lineNum];
  return GT_OK;
}

