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
* @file mvHwsTcamPcsDb.c
*
* @brief
*
* @version   3
********************************************************************************
*/
#include <cpss/common/labServices/port/gop/common/configElementDb/mvCfgSeqDbIf.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/siliconAddress.h>
#include <cpss/common/labServices/port/gop/port/pcs/tcamPcs/mvHwsTcamPcsDb.h>

static const MV_OP_PARAMS resetSeqParams[] = {
    {ETI_ILKN_RF_UNIT, ILKN_0_EN,                  0x0,       0x3}
};

static const MV_OP_PARAMS unresetSeqParams[] = {
    {ETI_ILKN_RF_UNIT, ILKN_0_EN,                  0x3,       0x3}
};

static const MV_OP_PARAMS modeMiscSeqParams[] = {
    {ETI_UNIT,         General_ETI_configuration0, 0x0,       0x100000},
    {ETI_ILKN_RF_UNIT, ILKN_0_PCS_CFG_0,           0x7FF07FF, 0x0},
    {ETI_ILKN_RF_UNIT, ILKN_0_PCS_CFG_1,           0xFFF,     0x0},
    {ETI_ILKN_RF_UNIT, ILKN_0_STAT_EN,             0x3,       0x3}
};

/* seqId to params array mapping */
const MV_MAC_PCS_CFG_SEQ hwsTcamPcsPscSeqDb[] =
{
    {resetSeqParams,    MV_SEQ_SIZE(resetSeqParams)},   /* TCAMPCS_RESET_SEQ     */
    {unresetSeqParams,  MV_SEQ_SIZE(unresetSeqParams)}, /* TCAMPCS_UNRESET_SEQ   */
    {modeMiscSeqParams, MV_SEQ_SIZE(modeMiscSeqParams)} /* TCAMPCS_MODE_MISC_SEQ */
};

GT_STATUS hwsTcamPcsSeqGet(MV_HWS_TCAMPCS_SUB_SEQ seqType, MV_OP_PARAMS *seqLine, GT_U32 lineNum)
{
  if ((seqType >= MV_TCAMPCS_LAST_SEQ) ||
      (hwsTcamPcsPscSeqDb[seqType].cfgSeq == NULL))
  {
      return GT_NO_SUCH;
  }

  *seqLine = hwsTcamPcsPscSeqDb[seqType].cfgSeq[lineNum];
  return GT_OK;
}
