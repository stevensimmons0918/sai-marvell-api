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
* @file mvHwsTcamMacDb.c
*
* @brief
*
* @version   3
********************************************************************************
*/
#include <cpss/common/labServices/port/gop/common/configElementDb/mvCfgSeqDbIf.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/siliconAddress.h>
#include <cpss/common/labServices/port/gop/port/mac/tcamMac/mvHwsTcamMacDb.h>

static const MV_OP_PARAMS modeSeqParams[] = {
    {ETI_ILKN_RF_UNIT, ILKN_0_MAC_CFG_0,            0x520D,    0x0},
    {ETI_ILKN_RF_UNIT, ILKN_0_MAC_CFG_3,            0x170B,    0x0}
};

static const MV_OP_PARAMS resetSeqParams[] = {
    {ETI_UNIT,         General_ETI_configuration0,  0x0,      (1 << 20)},
    {ETI_ILKN_RF_UNIT, ILKN_0_EN,                   0x0,       0xC}
};

static const MV_OP_PARAMS unresetSeqParams[] = {
    {ETI_UNIT,         General_ETI_configuration0, (1 << 20), (1 << 20)},
    {ETI_ILKN_RF_UNIT, ILKN_0_EN,                   0xC,       0xC}
};

/* seqId to params array mapping */
const MV_MAC_PCS_CFG_SEQ hwsTcamMacSeqDb[] =
{
    {unresetSeqParams, MV_SEQ_SIZE(unresetSeqParams)}, /* TCAMMAC_UNRESET_SEQ */
    {resetSeqParams,   MV_SEQ_SIZE(resetSeqParams)},   /* TCAMMAC_RESET_SEQ   */
    {modeSeqParams,    MV_SEQ_SIZE(modeSeqParams)}     /* TCAMMAC_MODE_SEQ    */
};

GT_STATUS hwsTcamMacSeqGet(MV_HWS_TCAM_MAC_SUB_SEQ seqType, MV_OP_PARAMS *seqLine, GT_U32 lineNum)
{
  if ((seqType >= MV_MAC_TCAM_LAST_SEQ) ||
      (hwsTcamMacSeqDb[seqType].cfgSeq == NULL))
  {
      return GT_NO_SUCH;
  }

  *seqLine = hwsTcamMacSeqDb[seqType].cfgSeq[lineNum];
  return GT_OK;
}
