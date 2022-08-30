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
* @file mvHwsCgMac28nmDb.c
*
* @brief
*
* @version   7
********************************************************************************
*/
#include <cpss/common/labServices/port/gop/common/configElementDb/mvCfgSeqDbIf.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/siliconAddress.h>
#include <cpss/common/labServices/port/gop/port/mac/cgMac/mvHwsCgMac28nmDb.h>

#ifdef MV_HWS_REDUCED_BUILD
#if defined(ALDRIN2_DEV_SUPPORT) || defined(PIPE_DEV_SUPPORT)
static const MV_OP_PARAMS cgMacMode4LaneSeqParams[] = {
    {CG_UNIT,       CG_RESETS,                                  0x44000000, 0x44000000}, /*this reset must be released before accessing MTI specific Register Files*/
    {CG_UNIT,       CG_COMMAND_CONFIG,                          0x201D3,    0xFFFFFFF3},
    {CG_UNIT,       CG_TX_FIFO_SECTIONS,                        0x5,        0xFF}
};
#else
static const MV_OP_PARAMS cgMacMode4LaneSeqParams[] = {
    {CG_UNIT,       CG_RESETS,                                  0x4000000,  0x4000000}, /*this reset must be released before accessing MTI specific Register Files*/
    {CG_UNIT,       CG_COMMAND_CONFIG,                          0x201D3,    0xFFFFFFF3},
    {CG_UNIT,       CG_TX_FIFO_SECTIONS,                        0x5,        0xFF}
};
#endif /*defined(ALDRIN_DEV_SUPPORT) || defined(PIPE_DEV_SUPPORT)*/
#else
static const MV_OP_PARAMS cgMacMode4LaneSeqParams[] = {
    {CG_UNIT,       CG_RESETS,                                  0x4000000,  0x4000000}, /*this reset must be released before accessing MTI specific Register Files*/
    {CG_UNIT,       CG_COMMAND_CONFIG,                          0x201D3,    0xFFFFFFF3},
    {CG_UNIT,       CG_TX_FIFO_SECTIONS,                        0x5,        0xFF}
};

static const MV_OP_PARAMS cgMacMode4LaneUpdateSeqParams[] = {
    {CG_UNIT,       CG_RESETS,                                  0x44000000, 0x44000000}, /*this reset must be released before accessing MTI specific Register Files*/
    {CG_UNIT,       CG_COMMAND_CONFIG,                          0x201D3,    0xFFFFFFF3},
    {CG_UNIT,       CG_TX_FIFO_SECTIONS,                        0x5,        0xFF}
};
#endif /*MV_HWS_REDUCED_BUILD*/

static const MV_OP_PARAMS xlgMacPowerDown4LaneSeqParams[] = {
    {CG_UNIT,       CG_RESETS,                                  0x4000000,  0x4000000}, /*this reset must be released before accessing MTI specific Register Files*/
    {CG_UNIT,       CG_COMMAND_CONFIG,                          0x0,        0xFFFFFFF0},
    {CG_UNIT,       CG_TX_FIFO_SECTIONS,                        0x10,       0xFF}/*,
    {CG_UNIT,       CG_RESETS,                                  0x0,        0x4000000},*/  /*this reset must be released before accessing MTI specific Register Files*/
};

static const MV_OP_PARAMS cgMaclbNormalSeqParams[] = {
    {CG_UNIT,       CG_RESETS,                                  0x4000000,  0x4000000}, /*this reset must be released before accessing MTI specific Register Files*/
    {CG_UNIT,       CG_COMMAND_CONFIG,                          0x0,        0x400}
};

static const MV_OP_PARAMS cgMaclbTx2RxSeqParams[] = {
    {CG_UNIT,       CG_RESETS,                                  0x4000000,  0x4000000}, /*this reset must be released before accessing MTI specific Register Files*/
    {CG_UNIT,       CG_COMMAND_CONFIG,                          0x400,      0x400}
};
static const MV_OP_PARAMS cgMacFcDisableSeqParams[] = {
    {CG_UNIT,       CG_RESETS,                                  0x4000000,  0x4000000}, /*this reset must be released before accessing MTI specific Register Files*/
    {MMPCS_UNIT,    0x600 /* FCA Control register offset */,    (1 << 1),   (1 << 1)},  /* enable bypass FCA */
    {CG_UNIT,       CG_FC_CONTROL_0,                            0,          0x8000}
};
static const MV_OP_PARAMS cgMacFcBothSeqParams[] = {
    {CG_UNIT,       CG_RESETS,                                  0x4000000,  0x4000000}, /*this reset must be released before accessing MTI specific Register Files*/
    {MMPCS_UNIT,    0x600,                                      (0 << 1),   (1 << 1)},  /* disable bypass FCA */
    {CG_UNIT,       CG_FC_CONTROL_0,                            0x8000,     0x8000}
};
static const MV_OP_PARAMS cgMacFcRxOnlySeqParams[] = {
    {CG_UNIT,       CG_RESETS,                                  0x4000000,  0x4000000}, /*this reset must be released before accessing MTI specific Register Files*/
    {MMPCS_UNIT,    0x600,                                      (1 << 1),   (1 << 1)},  /* enable bypass FCA */
    {CG_UNIT,       CG_FC_CONTROL_0,                            0x8000,     0x8000}
};
static const MV_OP_PARAMS cgMacFcTxOnlySeqParams[] = {
    {CG_UNIT,       CG_RESETS,                                  0x4000000,  0x4000000}, /*this reset must be released before accessing MTI specific Register Files*/
    {MMPCS_UNIT,    0x600,                                      (0 << 1),   (1 << 1)},  /* disable bypass FCA */
    {CG_UNIT,       CG_FC_CONTROL_0,                            0x0000,     0x8000}
};

/* seqId to params array mapping */
const MV_MAC_PCS_CFG_SEQ hwsCgMac28nmSeqDb[MV_MAC_CG_LAST_SEQ] =
{
    {NULL,                             MV_SEQ_SIZE(0)},                                /* CGMAC_UNRESET_SEQ            */
    {NULL,                             MV_SEQ_SIZE(0)},                                /* CGMAC_RESET_SEQ              */
    {cgMacMode4LaneSeqParams,          MV_SEQ_SIZE(cgMacMode4LaneSeqParams)},          /* CGMAC_MODE_SEQ               */
    {cgMaclbNormalSeqParams,           MV_SEQ_SIZE(cgMaclbNormalSeqParams)},           /* CGMAC_LPBK_NORMAL_SEQ        */
    {cgMaclbTx2RxSeqParams,            MV_SEQ_SIZE(cgMaclbTx2RxSeqParams)},            /* CGMAC_LPBK_TX2RX_SEQ         */
    {NULL,                             MV_SEQ_SIZE(0)},                                /* CGMAC_XLGMAC_LPBK_NORMAL_SEQ */
    {NULL,                             MV_SEQ_SIZE(0)},                                /* CGMAC_XLGMAC_LPBK_RX2TX_SEQ  */
    {NULL,                             MV_SEQ_SIZE(0)},                                /* CGMAC_XLGMAC_LPBK_TX2RX_SEQ  */
    {NULL,                             MV_SEQ_SIZE(0)},                                /* CGMAC_MODE_1_LANE_SEQ        */
    {NULL,                             MV_SEQ_SIZE(0)},                                /* CGMAC_MODE_2_LANE_SEQ        */
    {cgMacMode4LaneSeqParams,          MV_SEQ_SIZE(cgMacMode4LaneSeqParams)},          /* CGMAC_MODE_4_LANE_SEQ        */
    {NULL,                             MV_SEQ_SIZE(0)},                                /* CGMAC_POWER_DOWN_1_LANE_SEQ  */
    {NULL,                             MV_SEQ_SIZE(0)},                                /* CGMAC_POWER_DOWN_2_LANE_SEQ  */
    {xlgMacPowerDown4LaneSeqParams,    MV_SEQ_SIZE(xlgMacPowerDown4LaneSeqParams)},    /* CGMAC_POWER_DOWN_4_LANE_SEQ  */
    {cgMacFcDisableSeqParams,          MV_SEQ_SIZE(cgMacFcDisableSeqParams)},          /* CGMAC_FC_DISABLE_SEQ         */
    {cgMacFcBothSeqParams,             MV_SEQ_SIZE(cgMacFcBothSeqParams)},             /* CGMAC_FC_BOTH_SEQ            */
    {cgMacFcRxOnlySeqParams,           MV_SEQ_SIZE(cgMacFcRxOnlySeqParams)},           /* CGMAC_FC_RX_ONLY_SEQ         */
    {cgMacFcTxOnlySeqParams,           MV_SEQ_SIZE(cgMacFcTxOnlySeqParams)}           /* CGMAC_FC_TX_ONLY_SEQ         */
#ifndef MV_HWS_REDUCED_BUILD
    ,{cgMacMode4LaneUpdateSeqParams,    MV_SEQ_SIZE(cgMacMode4LaneUpdateSeqParams)}    /* CGMAC_MODE_4_LANE_UPDATE_SEQ        */
#endif

};

GT_STATUS hwsCgMac28nmSeqInit(GT_U8 devNum)
{
    devNum = devNum;
#if 0
    MV_OP_PARAMS* updateParams;
    if (HWS_DEV_SILICON_TYPE(devNum) == Pipe || HWS_DEV_SILICON_TYPE(devNum) == Aldrin2)
    {
        updateParams = (MV_OP_PARAMS*)&hwsCgMac28nmSeqDb[CGMAC_MODE_4_LANE_SEQ].cfgSeq[0];
        updateParams->operData = 0x44000000;
        updateParams->mask = 0x44000000;
    }
#endif
    return GT_OK;
}

GT_STATUS hwsCgMac28nmSeqGet(MV_HWS_CG_MAC_SUB_SEQ seqType, MV_OP_PARAMS *seqLine, GT_U32 lineNum)
{
  if ((seqType >= MV_MAC_CG_LAST_SEQ) ||
      (hwsCgMac28nmSeqDb[seqType].cfgSeq == NULL))
  {
      return GT_NO_SUCH;
  }

  *seqLine = hwsCgMac28nmSeqDb[seqType].cfgSeq[lineNum];
  return GT_OK;
}
