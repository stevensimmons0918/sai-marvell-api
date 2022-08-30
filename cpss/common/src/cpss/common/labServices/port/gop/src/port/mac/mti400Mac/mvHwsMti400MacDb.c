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
* @file mvHwsMti400MacDb.c
*
* @brief
*
* @version   1
********************************************************************************
*/
#include <cpss/common/labServices/port/gop/common/configElementDb/mvCfgSeqDbIf.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/siliconAddress.h>
#include <cpss/common/labServices/port/gop/port/mac/mti400Mac/mvHwsMti400MacDb.h>

static const MV_OP_PARAMS cgMti400MacModeSeqParams[] = {
    {MTI_MAC400_UNIT,   MTI_MAC_COMMAND_CONFIG,          0x2810,     0x2850},

    /* MTI JIRA workaround - GOPMTIP-259
        In a rare case of “end of frame” and “start of frame” on the same cycle
        on the MII interface, when the start of frame as a corrupted SFD, the
        MAC transmits to the application SOP without EOP. */
    {MTI_MAC400_UNIT,   MTI_MAC_COMMAND_CONFIG,          0x200000,   0x200000},
    {MTI_MAC400_UNIT,   MTI_MAC_TX_IPG_LENGTH,           0x50000000, 0xFFF80000}, /* TxIpgCompensation.set(0xA00) */
    {MTI_MAC400_UNIT,   MTI_MAC_TX_FIFO_SECTIONS,        0x7,        0xFFFF},
    {MTI_MAC400_UNIT,   MTI_MAC_RX_FIFO_SECTIONS,        0x1,        0xFFFF},
    {MTI_MAC400_UNIT,   MTI_MAC_XIF_MODE,                0x20,       0x20}
};

static const MV_OP_PARAMS cgMti400MacPowerDownSeqParams[] = {
    {MTI_MAC400_UNIT,   MTI_MAC_COMMAND_CONFIG,          0x800,      0x2850},
    {MTI_MAC400_UNIT,   MTI_MAC_COMMAND_CONFIG,          0x0,        0x200000},   /* MTI JIRA workaround - GOPMTIP-259*/
    {MTI_MAC400_UNIT,   MTI_MAC_TX_IPG_LENGTH,           0x50000000, 0xFFF80000}, /* TxIpgCompensation.set(0xA00) */
    {MTI_MAC400_UNIT,   MTI_MAC_TX_FIFO_SECTIONS,        0x8,        0xFFFF},
    {MTI_MAC400_UNIT,   MTI_MAC_RX_FIFO_SECTIONS,        0x8,        0xFFFF},
    {MTI_MAC400_UNIT,   MTI_MAC_XIF_MODE,                0x0,        0x20}
};

#if 0
static MV_OP_PARAMS xlgMti400MacResetSeqParams[] = {
    {UNIT_MTI_MAC400,   MTI64_MAC_COMMAND_CONFIG,          0x0,        0x3},
};
#endif
static const MV_OP_PARAMS cgMti400MacUnresetSeqParams[] = {
    /* dummy access to MTI MAC in order to "connect" register file and rtl logic,
       should be done for UNRESET sequence for MAC400 */
    {MTI_MAC400_UNIT,   MTI_MAC_SCRATCH,                 0x0,        0x1}
};


/* seqId to params array mapping */
const MV_MAC_PCS_CFG_SEQ hwsMti400MacSeqDb[] =
{
    {cgMti400MacModeSeqParams,      MV_SEQ_SIZE(cgMti400MacModeSeqParams)},         /* HWS_MTI_MAC400_SEGMENTED_4_LANES_MODE_SEQ */
    {cgMti400MacPowerDownSeqParams, MV_SEQ_SIZE(cgMti400MacPowerDownSeqParams)},    /* HWS_MTI_MAC400_SEGMENTED_4_LANES_POWER_DOWN_SEQ */
    {cgMti400MacPowerDownSeqParams, MV_SEQ_SIZE(cgMti400MacPowerDownSeqParams)},    /* HWS_MTI_MAC400_SEGMENTED_8_LANES_POWER_DOWN_SEQ */
    {cgMti400MacUnresetSeqParams,   MV_SEQ_SIZE(cgMti400MacUnresetSeqParams)},      /* HWS_MTI_MAC400_SEGMENTED_4_LANES_UNRESET_SEQ */
    {cgMti400MacUnresetSeqParams,   MV_SEQ_SIZE(cgMti400MacUnresetSeqParams)},      /* HWS_MTI_MAC400_SEGMENTED_8_LANES_UNRESET_SEQ */
};

GT_STATUS hwsMti400MacSeqGet(HWS_MTI_MAC400_SUB_SEQ seqType, MV_OP_PARAMS *seqLine, GT_U32 lineNum)
{
  if ((seqType >= HWS_MTI_MAC400_SEGMENTED_LAST_SEQ) ||
      (hwsMti400MacSeqDb[seqType].cfgSeq == NULL))
  {
      return GT_NO_SUCH;
  }

  *seqLine = hwsMti400MacSeqDb[seqType].cfgSeq[lineNum];
  return GT_OK;
}

