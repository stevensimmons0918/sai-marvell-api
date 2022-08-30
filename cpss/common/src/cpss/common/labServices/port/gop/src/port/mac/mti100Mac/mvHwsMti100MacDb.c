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
* @file mvHwsMti100MacDb.c
*
* @brief
*
* @version   1
********************************************************************************
*/
#include <cpss/common/labServices/port/gop/common/configElementDb/mvCfgSeqDbIf.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/siliconAddress.h>
#include <cpss/common/labServices/port/gop/port/mac/mti100Mac/mvHwsMti100MacDb.h>

static const MV_OP_PARAMS xgMti100MacModeSeqParams[] = { /* 10G-25G no rsfec*/
    /*{UNIT_MTI_MAC100,   MTI_MAC_TX_IPG_LENGTH,           0x4FFF0000, 0xFFFF0000},*/
    {MTI_MAC100_UNIT,   MTI_MAC_TX_IPG_LENGTH,           0x00000, 0xFFFF0000},
    {MTI_MAC100_UNIT,   MTI_MAC_COMMAND_CONFIG,          0x2810,     0x2850},

    /* MTI JIRA workaround - GOPMTIP-259
        In a rare case of “end of frame” and “start of frame” on the same cycle
        on the MII interface, when the start of frame as a corrupted SFD, the
        MAC transmits to the application SOP without EOP. */
    {MTI_MAC100_UNIT,   MTI_MAC_COMMAND_CONFIG,          0x200000,   0x200000},
    {MTI_MAC100_UNIT,   MTI_MAC_TX_FIFO_SECTIONS,        0xA,        0xFFFF},
    {MTI_MAC100_UNIT,   MTI_MAC_RX_FIFO_SECTIONS,        0x1,        0xFFFF},
    {MTI_MAC100_UNIT,   MTI_MAC_XIF_MODE,                0x21,       0x31} /*Onestepena = 1,Pausetimerx8 = 0, Xgmii = 1*/
};

static const MV_OP_PARAMS xlgMti100MacNrzModeSeqParamsSpecialSpeeds[] = { /* 106G rsfec R4, 42G R4, 53G R2 */
    {MTI_MAC100_UNIT,   MTI_MAC_TX_IPG_LENGTH,           0x3FFF0000,     0xFFFF0000},
    {MTI_MAC100_UNIT,   MTI_MAC_COMMAND_CONFIG,          0x202850,     0x202850},
    {RAVEN_MPFS_UNIT,   MPFS_SAU_CONTROL,                0x0,            0x1},

    /* MTI JIRA workaround - GOPMTIP-259
        In a rare case of “end of frame” and “start of frame” on the same cycle
        on the MII interface, when the start of frame as a corrupted SFD, the
        MAC transmits to the application SOP without EOP. */
    /*{MTI_MAC100_UNIT,   MTI_MAC_COMMAND_CONFIG,          0x200000,   0x200000},*/ /* already taken care of above */
    {MTI_MAC100_UNIT,   MTI_MAC_TX_FIFO_SECTIONS,        0xA,        0xFFFF},
    {MTI_MAC100_UNIT,   MTI_MAC_RX_FIFO_SECTIONS,        0x1,        0xFFFF},
    {MTI_MAC100_UNIT,   MTI_MAC_XIF_MODE,                0x20,       0x31}/*Onestepena = 1,Pausetimerx8 = 0, Xgmii = 0*/
};

static const MV_OP_PARAMS xgMti100Mac25RsFecModeSeqParams[] = { /* 25G rsfec*/

    /*
    GOPMTIP-282
        Issue Summary
    IPG compensation is used to remove IPG due to the insertion of Alignment markers by the PCS.
    The configuration of how often IPG is removed is configured in the register TX_IPG_LENGTH bits
    [31:16] (or [31:8] in some MAC versions). In particular for XGMII mode (set via XIF_MODE register
    bit [0]) the IPG is not be removed if all transmitted frames lengths are multiple of 4 bytes (i.e. 64,
    68, 72, etc), and instead IPG is only removed when the DIC (Deficit Idle Count) logic adds 4-bytes
    of extra IPG. This can cause degraded performance in particular scenarios (benchmarking for
    instance where single-sized frames are used).
        Workaround
    Setting bit TX_IPG_LENGTH bit 1 (documented as reserved) enables the compensation logic to
    always compensate. The side-effect is that the minimum IPG seen can be as low as 5-bytes, whilst
    normally the expected minimum IPG in XGMII modes due to DIC is 9 bytes (or 8 bytes due to IPG
    compensation). This however should not be a problem since IPG compensation in XGMII mode is
    needed for links operating at 25GBase-R with RSFEC where the PCS encodes data in 64-bit
    blocks. */

    {MTI_MAC100_UNIT,   MTI_MAC_TX_IPG_LENGTH,           0x4FFF0002, 0xFFFF0002},
    {MTI_MAC100_UNIT,   MTI_MAC_COMMAND_CONFIG,          0x2810,     0x2850},

    /* MTI JIRA workaround - GOPMTIP-259
        In a rare case of “end of frame” and “start of frame” on the same cycle
        on the MII interface, when the start of frame as a corrupted SFD, the
        MAC transmits to the application SOP without EOP. */
    {MTI_MAC100_UNIT,   MTI_MAC_COMMAND_CONFIG,          0x200000,   0x200000},
    {MTI_MAC100_UNIT,   MTI_MAC_TX_FIFO_SECTIONS,        0xA,        0xFFFF},
    {MTI_MAC100_UNIT,   MTI_MAC_RX_FIFO_SECTIONS,        0x1,        0xFFFF},
    {MTI_MAC100_UNIT,   MTI_MAC_XIF_MODE,                0x21,       0x31} /*Onestepena = 1,Pausetimerx8 = 0, Xgmii = 1*/
};

static const MV_OP_PARAMS xlgMti100MacModeSeqParams[] = { /*50R1,100R2*/
    {MTI_MAC100_UNIT,   MTI_MAC_TX_IPG_LENGTH,           0x4FFF0000, 0xFFFF0000},
    {MTI_MAC100_UNIT,   MTI_MAC_COMMAND_CONFIG,          0x2810,     0x2850},

    /* MTI JIRA workaround - GOPMTIP-259
        In a rare case of “end of frame” and “start of frame” on the same cycle
        on the MII interface, when the start of frame as a corrupted SFD, the
        MAC transmits to the application SOP without EOP. */
    {MTI_MAC100_UNIT,   MTI_MAC_COMMAND_CONFIG,          0x200000,   0x200000},
    {MTI_MAC100_UNIT,   MTI_MAC_TX_FIFO_SECTIONS,        0xA,        0xFFFF},
    {MTI_MAC100_UNIT,   MTI_MAC_RX_FIFO_SECTIONS,        0x1,        0xFFFF},
    {MTI_MAC100_UNIT,   MTI_MAC_XIF_MODE,                0x20,       0x31}/*Onestepena = 1,Pausetimerx8 = 0, Xgmii = 0*/
};

static const MV_OP_PARAMS xlgMti100MacNrzModeSeqParams[] = { /*40R4,50R2*/
    {MTI_MAC100_UNIT,   MTI_MAC_TX_IPG_LENGTH,           0x3FFF0000, 0xFFFF0000},
    {MTI_MAC100_UNIT,   MTI_MAC_COMMAND_CONFIG,          0x2810,     0x2850},

    /* MTI JIRA workaround - GOPMTIP-259
        In a rare case of “end of frame” and “start of frame” on the same cycle
        on the MII interface, when the start of frame as a corrupted SFD, the
        MAC transmits to the application SOP without EOP. */
    {MTI_MAC100_UNIT,   MTI_MAC_COMMAND_CONFIG,          0x200000,   0x200000},
    {MTI_MAC100_UNIT,   MTI_MAC_TX_FIFO_SECTIONS,        0xA,        0xFFFF},
    {MTI_MAC100_UNIT,   MTI_MAC_RX_FIFO_SECTIONS,        0x1,        0xFFFF},
    {MTI_MAC100_UNIT,   MTI_MAC_XIF_MODE,                0x20,       0x31}/*Onestepena = 1,Pausetimerx8 = 0, Xgmii = 0*/
};

static const MV_OP_PARAMS xlgMti100MacPowerDownSeqParams[] = {
    {MTI_MAC100_UNIT,   MTI_MAC_TX_IPG_LENGTH,           0x0,        0xFFFF0002},   /* bit_1 - MTI JIRA workaround - GOPMTIP-282 */
    {MTI_MAC100_UNIT,   MTI_MAC_COMMAND_CONFIG,          0x800,      0x2850},
    /*{MTI_MAC100_UNIT,   MTI_MAC_COMMAND_CONFIG,          0x0,        0x200000},*/     /* MTI JIRA workaround - GOPMTIP-259 */
    {MTI_MAC100_UNIT,   MTI_MAC_COMMAND_CONFIG,          0x0,        0x200040},   /* revert 106G sequence (bit 6 set to 0 as default */
    {MTI_MAC100_UNIT,   MTI_MAC_TX_FIFO_SECTIONS,        0x8,        0xFFFF},
    {MTI_MAC100_UNIT,   MTI_MAC_RX_FIFO_SECTIONS,        0x8,        0xFFFF},
    {MTI_MAC100_UNIT,   MTI_MAC_XIF_MODE,                0x0,        0x31},
    {RAVEN_MPFS_UNIT,   MPFS_SAU_CONTROL,                0x1,        0x1}
};

#if 0
static MV_OP_PARAMS xlgMti100MacResetSeqParams[] = {
    {UNIT_MTI_MAC100,   MTI_MAC_COMMAND_CONFIG,          0x0,        0x3},
};
#endif
static const MV_OP_PARAMS xlgMti100MacUnresetSeqParams[] = {
    /* dummy access to MTI MAC in order to "connect" register file and rtl logic,
       should be done for UNRESET sequence for MAC64 */
    {MTI_MAC100_UNIT,   MTI_MAC_SCRATCH,                 0x0,        0x1}
};



/* seqId to params array mapping */
const MV_MAC_PCS_CFG_SEQ hwsMti100MacSeqDb[] =
{
    {xgMti100MacModeSeqParams,                   MV_SEQ_SIZE(xgMti100MacModeSeqParams)},                  /* HWS_MTI_MAC100_XG_MODE_SEQ,  10G - 25G */
    {xgMti100Mac25RsFecModeSeqParams,            MV_SEQ_SIZE(xgMti100Mac25RsFecModeSeqParams)},           /* HWS_MTI_MAC100_XG_25RSFEC_MODE_SEQ,  25G rs fec*/
    {xlgMti100MacModeSeqParams,                  MV_SEQ_SIZE(xlgMti100MacModeSeqParams)},                 /* HWS_MTI_MAC100_XLG_MODE_SEQ  50R1,100R2 */
    {xlgMti100MacNrzModeSeqParams,               MV_SEQ_SIZE(xlgMti100MacNrzModeSeqParams)},              /* HWS_MTI_MAC100_XLG_NRZ_MODE_SEQ  50R2,40R4 100R4*/
    {xlgMti100MacNrzModeSeqParamsSpecialSpeeds,  MV_SEQ_SIZE(xlgMti100MacNrzModeSeqParamsSpecialSpeeds)}, /* HWS_MTI_MAC100_XLG_NRZ_SPECIAL_SPEEDS_SEQ,  106G rs_fec, 42G, 53G */
    {xlgMti100MacPowerDownSeqParams,             MV_SEQ_SIZE(xlgMti100MacPowerDownSeqParams)},            /* HWS_MTI_MAC100_XLG_POWER_DOWN_SEQ */
    {xlgMti100MacUnresetSeqParams,               MV_SEQ_SIZE(xlgMti100MacUnresetSeqParams)},              /* HWS_MTI_MAC100_XLG_UNRESET_SEQ */
#if 0
    {xlgMti100MacResetSeqParams,    MV_SEQ_SIZE(xlgMti100MacResetSeqParams)},         /* HWS_MTI_MAC100_XLG_RESET_SEQ */
#endif
};

GT_STATUS hwsMti100MacSeqGet(HWS_MTI_MAC100_SUB_SEQ seqType, MV_OP_PARAMS *seqLine, GT_U32 lineNum)
{
  if ((seqType >= HWS_MTI_MAC100_LAST_SEQ) ||
      (hwsMti100MacSeqDb[seqType].cfgSeq == NULL))
  {
      return GT_NO_SUCH;
  }

  *seqLine = hwsMti100MacSeqDb[seqType].cfgSeq[lineNum];
  return GT_OK;
}
