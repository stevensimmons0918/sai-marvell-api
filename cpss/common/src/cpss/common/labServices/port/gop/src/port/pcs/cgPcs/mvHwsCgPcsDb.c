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
* @file mvHwsCgPcsDb.c
*
* @brief
*
* @version   7
********************************************************************************
*/
#include <cpss/common/labServices/port/gop/common/configElementDb/mvCfgSeqDbIf.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/siliconAddress.h>
#include <cpss/common/labServices/port/gop/port/pcs/cgPcs/mvHwsCgPcsDb.h>

static const MV_OP_PARAMS modeMiscSeqParams[] = {
    {CG_UNIT, CG_CONTROL_0, 0x1B,      0x3FFFFFF},
    {CG_UNIT, CG_RESETS,    0x0,       0x4000000},
    {CG_UNIT, CG_RESETS,    0x4000000, 0x4000000}
};

static const MV_OP_PARAMS modeLane10SeqParams[] = {
    {CG_UNIT, CG_CONTROL_0, 0x400000,  0x400000},
    {CG_UNIT, CG_CONTROL_0, 0x3,       0x3}
};

static const MV_OP_PARAMS modeLane12SeqParams[] = {
    {CG_UNIT, CG_CONTROL_0, 0x0,       0x400000},
    {CG_UNIT, CG_CONTROL_0, 0x3,       0x3}
};

static const MV_OP_PARAMS lbNormalSeqParams[] = {
    {CG_UNIT, CG_CONTROL_1, 0x0,       0x4000}
};

static const MV_OP_PARAMS lbTx2RxSeqParams[] = {
    {CG_UNIT, CG_CONTROL_1, 0x4000,    0x4000}
};

/* seqId to params array mapping */
const MV_MAC_PCS_CFG_SEQ hwsCgPcsPscSeqDb_default[] = 
{
    {NULL,                MV_SEQ_SIZE(0)},                   /* CGPCS_RESET_SEQ        */
    {NULL,                MV_SEQ_SIZE(0)},                   /* CGPCS_UNRESET_SEQ      */
    {modeMiscSeqParams,   MV_SEQ_SIZE(modeMiscSeqParams)},   /* CGPCS_MODE_MISC_SEQ    */
    {modeLane10SeqParams, MV_SEQ_SIZE(modeLane10SeqParams)}, /* CGPCS_MODE_10_LANE_SEQ */
    {modeLane12SeqParams, MV_SEQ_SIZE(modeLane12SeqParams)}, /* CGPCS_MODE_12_LANE_SEQ */
    {lbNormalSeqParams,   MV_SEQ_SIZE(lbNormalSeqParams)},   /* CGPCS_LPBK_NORMAL_SEQ  */
    {lbTx2RxSeqParams,    MV_SEQ_SIZE(lbTx2RxSeqParams)}     /* CGPCS_LPBK_TX2RX_SEQ   */
};

/* hwsCgPcsPscSeqDb may be point to other DB (hwsCgPcs28nmSeqInit) */
const MV_MAC_PCS_CFG_SEQ *hwsCgPcsPscSeqDb = &hwsCgPcsPscSeqDb_default[0];

GT_STATUS hwsCgPcsSeqGet(MV_HWS_CGPCS_SUB_SEQ seqType, MV_OP_PARAMS *seqLine, GT_U32 lineNum)
{
  if ((seqType >= MV_CGPCS_LAST_SEQ) ||
      (hwsCgPcsPscSeqDb[seqType].cfgSeq == NULL))
  {
      return GT_NO_SUCH;
  }

  *seqLine = hwsCgPcsPscSeqDb[seqType].cfgSeq[lineNum];
  return GT_OK;
}
