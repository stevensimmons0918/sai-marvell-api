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
* @file mvHwsHglPcsDb.c
*
* @brief
*
* @version   10
********************************************************************************
*/
#include <cpss/common/labServices/port/gop/common/configElementDb/mvCfgSeqDbIf.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/siliconAddress.h>
#include <cpss/common/labServices/port/gop/port/pcs/hglPcs/mvHwsHglPcsDb.h>

static const MV_OP_PARAMS modeMiscSeqParams[] = {
    {XPCS_UNIT, XPCS_Global_Configuration_Reg1,           (1 << 4),  (1 << 4)},
    {XPCS_UNIT, XPCS_Global_Fifo_Threshold_Configuration,  0xF,       0xF},
    {XPCS_UNIT, XPCS_Global_Configuration_Reg0,           (1 << 3),  (3 << 3)}
};

static const MV_OP_PARAMS modeLane4SeqParams[] = {
    {XPCS_UNIT, XPCS_Global_Configuration_Reg0,           (2 << 5),  (3 << 5)},
    {XPCS_UNIT, XPCS_Global_Configuration_Reg0,           (7 << 13), (7 << 13)}
};

static const MV_OP_PARAMS modeLane6SeqParams[] = {
    {XPCS_UNIT, XPCS_Global_Configuration_Reg0,           (3 << 5),  (3 << 5)},
    {XPCS_UNIT, XPCS_Global_Configuration_Reg0,           (5 << 13), (7 << 13)}
};

/* seqId to params array mapping */
const MV_MAC_PCS_CFG_SEQ hwsHglPscSeqDb[] =
{
    {modeMiscSeqParams,  MV_SEQ_SIZE(modeMiscSeqParams)},  /* HGLPCS_MODE_MISC_SEQ   */
    {modeLane4SeqParams, MV_SEQ_SIZE(modeLane4SeqParams)}, /* HGLPCS_MODE_4_LANE_SEQ */
    {modeLane6SeqParams, MV_SEQ_SIZE(modeLane6SeqParams)}  /* HGLPCS_MODE_6_LANE_SEQ */
};

GT_STATUS hwsHglPcsSeqGet(MV_HWS_HGLPCS_SUB_SEQ seqType, MV_OP_PARAMS *seqLine, GT_U32 lineNum)
{
  if ((seqType >= MV_HGLPCS_LAST_SEQ) ||
      (hwsHglPscSeqDb[seqType].cfgSeq == NULL))
  {
      return GT_NO_SUCH;
  }

  *seqLine = hwsHglPscSeqDb[seqType].cfgSeq[lineNum];
  return GT_OK;
}
