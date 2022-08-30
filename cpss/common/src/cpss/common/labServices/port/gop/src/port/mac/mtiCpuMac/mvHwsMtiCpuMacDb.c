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
* @file mvHwsMtiCpuMacDb.c
*
* @brief
*
* @version   1
********************************************************************************
*/
#include <cpss/common/labServices/port/gop/common/configElementDb/mvCfgSeqDbIf.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/siliconAddress.h>
#include <cpss/common/labServices/port/gop/port/mac/mtiCpuMac/mvHwsMtiCpuMacDb.h>

static const MV_OP_PARAMS xlgMtiCpuMacModeSeqParams[] = {

    {MTI_CPU_MAC_UNIT,   MTI_MAC_COMMAND_CONFIG,          0x2810,     0x2850},
    {MTI_CPU_MAC_UNIT,   MTI_MAC_TX_FIFO_SECTIONS,        0xA,        0xFFFF},
    {MTI_CPU_MAC_UNIT,   MTI_MAC_RX_FIFO_SECTIONS,        0x1,        0xFFFF},
    {MTI_CPU_MAC_UNIT,   MTI_MAC_XIF_MODE,                0x21,       0x21}
};

static const MV_OP_PARAMS xlgMtiCpuMacPowerDownSeqParams[] = {
    {MTI_CPU_MAC_UNIT,   MTI_MAC_COMMAND_CONFIG,          0x800,      0x2850},
    {MTI_CPU_MAC_UNIT,   MTI_MAC_TX_FIFO_SECTIONS,        0x8,        0xFFFF},
    {MTI_CPU_MAC_UNIT,   MTI_MAC_RX_FIFO_SECTIONS,        0x8,        0xFFFF},
    {MTI_CPU_MAC_UNIT,   MTI_MAC_XIF_MODE,                0x0,        0x21}
};

static const MV_OP_PARAMS xlgMtiCpuMacUnresetSeqParams[] = {
    /* dummy access to MTI MAC in order to "connect" register file and rtl logic,
       should be done for UNRESET sequence for MAC64 */
    {MTI_CPU_MAC_UNIT,   MTI_MAC_SCRATCH,                 0x0,        0x1}
};

/* seqId to params array mapping */
const MV_MAC_PCS_CFG_SEQ hwsMtiCpuMacSeqDb[] =
{
    {xlgMtiCpuMacModeSeqParams,     MV_SEQ_SIZE(xlgMtiCpuMacModeSeqParams)},          /* MTI_CPU_MAC_XLG_MODE_SEQ */
    {xlgMtiCpuMacPowerDownSeqParams,MV_SEQ_SIZE(xlgMtiCpuMacPowerDownSeqParams)},     /* MTI_CPU_MAC_XLG_POWER_DOWN_SEQ */
    {xlgMtiCpuMacUnresetSeqParams,  MV_SEQ_SIZE(xlgMtiCpuMacUnresetSeqParams)},       /* MTI_CPU_MAC_XLG_UNRESET_SEQ */
};

GT_STATUS hwsMtiCpuMacSeqGet(HWS_MTI_CPU_MAC_SUB_SEQ seqType, MV_OP_PARAMS *seqLine, GT_U32 lineNum)
{
  if ((seqType >= MTI_CPU_MAC_LAST_SEQ) ||
      (hwsMtiCpuMacSeqDb[seqType].cfgSeq == NULL))
  {
      return GT_NO_SUCH;
  }

  *seqLine = hwsMtiCpuMacSeqDb[seqType].cfgSeq[lineNum];
  return GT_OK;
}
