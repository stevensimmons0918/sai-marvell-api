/******************************************************************************
*              Copyright (c) Marvell International Ltd. and its affiliates
*
* This software file (the "File") is owned and distributed by Marvell
* International Ltd. and/or its affiliates ("Marvell") under the following
* alternative licensing terms.
* If you received this File from Marvell, you may opt to use, redistribute
* and/or modify this File under the following licensing terms.
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*  -   Redistributions of source code must retain the above copyright notice,
*       this list of conditions and the following disclaimer.
*  -   Redistributions in binary form must reproduce the above copyright
*       notice, this list of conditions and the following disclaimer in the
*       documentation and/or other materials provided with the distribution.
*  -    Neither the name of Marvell nor the names of its contributors may be
*       used to endorse or promote products derived from this software without
*       specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
* ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
* LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
* CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
* SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
* OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
* WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
* OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
* ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*******************************************************************************
*/
/**
********************************************************************************
* @file mvHwsMtiPcs200Db.c
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

static const MV_OP_PARAMS mtiPcs200resetSeqParams[] = {
    {MTI_PCS200_UNIT, MTI_PCS_CONTROL1,      /*dummy write 0x8000*/0x0000,     0x8000}  /*CONTROL1.Reset.set(1)*/

};

static const MV_OP_PARAMS mtiPcs200unresetSeqParams[] = {
    {MTI_PCS200_UNIT, MTI_PCS_CONTROL1,                     0x8000,        0x8000}  /*CONTROL1.Reset.set(0)*/

};


/**********************************************************************************************
 **********************************************************************************************
 * 200G_BaseR4 on offset 4 should be defined via PCS400 unit on offset 0.
 * Only unreset/reset operation is done via PCS200 unit.
 **********************************************************************************************
 ***********************************************************************************************/
static const MV_OP_PARAMS mtiPcs200PowerDownSeqParams[] = {
    {MTI_PCS400_UNIT, MTI_PCS_VENDOR_TX_LANE_THRESH,        0x7,        0xF},     /*VENDOR_TX_LANE_THRESH.Tx_lanethresh.set(7)*/
    {MTI_PCS400_UNIT, MTI_PCS_VENDOR_VL_INTVL,              0x2000,     0x7FFF},  /*VENDOR_VL_INTVL.Marker_counter.set ((20479 + 1)/5);*/
    {MTI_PCS400_UNIT, MTI_PCS_VENDOR_AM_0,                  0x4a9a,     0xFFFF},  /*VENDOR_AM_0.Cm1.set(8'h4a)+VENDOR_AM_0.Cm0.set(8'h9a);*/
    {MTI_PCS400_UNIT, MTI_PCS_VENDOR_AM_1,                  0x26,       0xFF},    /*VENDOR_AM_1.Cm2.set(8'h26);*/
    {MTI_PCS400_UNIT, MTI_PCS_VENDOR_TX_CDMII_PACE,         0x8,        0xF},     /*VENDOR_TX_CDMII_PACE.Tx_cdmii_pace.set(8)*/
    {MTI_PCS400_UNIT, MTI_PCS_CONTROL1,                     0x0,        0x4},     /*CONTROL1.Speed_selection.set(mii == _200GMII ? 1 : 0)*/

    {MTI_RSFEC_UNIT,  MTI_RSFEC_VENDOR_VL0_0,               0x7101,     0xFFFF}, /*VENDOR_VL0_0.set(16'hc0b3)*/
    {MTI_RSFEC_UNIT,  MTI_RSFEC_VENDOR_VL0_1,               0xf3,       0xFF},   /*VENDOR_VL0_1.set(8'h8c)*/
    {MTI_RSFEC_UNIT,  MTI_RSFEC_VENDOR_VL1_0,               0xde5a,     0xFFFF}, /*VENDOR_VL1_0.set(16'hde5a)*/
    {MTI_RSFEC_UNIT,  MTI_RSFEC_VENDOR_VL1_1,               0x7e,       0xFF},   /*VENDOR_VL1_1.set(8'h7e)*/
    {MTI_RSFEC_UNIT,  MTI_RSFEC_VENDOR_VL2_0,               0xf33e,     0xFFFF}, /*VENDOR_VL2_0.set(16'hf33e)*/
    {MTI_RSFEC_UNIT,  MTI_RSFEC_VENDOR_VL2_1,               0x56,       0xFF},   /*VENDOR_VL2_1.set(8'h56)*/
    {MTI_RSFEC_UNIT,  MTI_RSFEC_VENDOR_VL3_0,               0x8086,     0xFFFF}, /*VENDOR_VL3_0.set(16'h8086)*/
    {MTI_RSFEC_UNIT,  MTI_RSFEC_VENDOR_VL3_1,               0xd0,       0xFF},   /*VENDOR_VL3_1.set(8'hd0)*/

    {MTI_RSFEC_UNIT,  MTI_RSFEC_VENDOR_VL4_0,               0x512a,     0xFFFF}, /*VENDOR_VL4_0.set(16'h512a)*/
    {MTI_RSFEC_UNIT,  MTI_RSFEC_VENDOR_VL4_1,               0xf2,       0xFF},   /*VENDOR_VL4_1.set(8'f2)*/
    {MTI_RSFEC_UNIT,  MTI_RSFEC_VENDOR_VL5_0,               0x4f12,     0xFFFF}, /*VENDOR_VL5_0.set(16'h4f12)*/
    {MTI_RSFEC_UNIT,  MTI_RSFEC_VENDOR_VL5_1,               0xd1,       0xFF},   /*VENDOR_VL5_1.set(8'hd1)*/
    {MTI_RSFEC_UNIT,  MTI_RSFEC_VENDOR_VL6_0,               0x9c42,     0xFFFF}, /*VENDOR_VL6_0.set(16'h9c42)*/
    {MTI_RSFEC_UNIT,  MTI_RSFEC_VENDOR_VL6_1,               0xa1,       0xFF},   /*VENDOR_VL6_1.set(8'ha1)*/
    {MTI_RSFEC_UNIT,  MTI_RSFEC_VENDOR_VL7_0,               0x76d6,     0xFFFF}, /*VENDOR_VL7_0.set(16'h76d6)*/
    {MTI_RSFEC_UNIT,  MTI_RSFEC_VENDOR_VL7_1,               0x5b,       0xFF},   /*VENDOR_VL7_1.set(8'h5b)*/

};


/**********************************************************************************************
 **********************************************************************************************
 * 200G_BaseR4 on offset 4 should be ddefined via PCS400 unit on offset 0.
 * Only unreset/reset operation is done via PCS200 unit.
 **********************************************************************************************
 ***********************************************************************************************/
static const MV_OP_PARAMS mtiPcs200R4ModeSeqParams[] = { /* speed  200G R4 */
    /**************** MTI_PCS400::ConfigureCM *************/

    {MTI_PCS400_UNIT, MTI_PCS_VENDOR_TX_LANE_THRESH,        0x9,        0xF},     /*VENDOR_TX_LANE_THRESH.Tx_lanethresh.set(9)*/
    /*Marker_counter = ((20479 + 1)/5);*/
    {MTI_PCS400_UNIT, MTI_PCS_VENDOR_VL_INTVL,              0x1000,     0x7FFF},  /*VENDOR_VL_INTVL.Marker_counter.set ((20479 + 1)/5);*/
    /* Common Marker values same as 400G*/
    {MTI_PCS400_UNIT, MTI_PCS_VENDOR_AM_0,                  0x4a9a,     0xFFFF},  /*VENDOR_AM_0.Cm1.set(8'h4a)+VENDOR_AM_0.Cm0.set(8'h9a);*/
    {MTI_PCS400_UNIT, MTI_PCS_VENDOR_AM_1,                  0x26,       0xFF},    /*VENDOR_AM_1.Cm2.set(8'h26);*/

    /*************** MTI_PCS400::SpeedSelect(MII_TYPE _200GMII);********/
    {MTI_PCS400_UNIT, MTI_PCS_VENDOR_TX_CDMII_PACE,         0x8,        0xF},     /*VENDOR_TX_CDMII_PACE.Tx_cdmii_pace.set(8)*/
    {MTI_PCS400_UNIT, MTI_PCS_CONTROL1,                     0x4,        0x4},     /*CONTROL1.Speed_selection.set(mii == _200GMII ? 1 : 0)*/


    /*************** MTI_RSFEC::Configure(FEC_TYPE _200G_544_514); ***************/
       /* Unique markers values from rsfec file VL1..VL7 same as 400G */
    {MTI_RSFEC_UNIT,  MTI_RSFEC_VENDOR_VL0_0,               0xc0b3,     0xFFFF},  /*VENDOR_VL0_0.set(16'hc0b3)*/
    {MTI_RSFEC_UNIT,  MTI_RSFEC_VENDOR_VL0_1,               0x8c,       0xFF},    /*VENDOR_VL0_1.set(8'h8c)*/
    {MTI_RSFEC_UNIT,  MTI_RSFEC_VENDOR_VL1_0,               0xde5a,     0xFFFF},  /*VENDOR_VL1_0.set(16'hde5a)*/
    {MTI_RSFEC_UNIT,  MTI_RSFEC_VENDOR_VL1_1,               0x7e,       0xFF},    /*VENDOR_VL1_1.set(8'h7e)*/
    {MTI_RSFEC_UNIT,  MTI_RSFEC_VENDOR_VL2_0,               0xf33e,     0xFFFF},  /*VENDOR_VL2_0.set(16'hf33e)*/
    {MTI_RSFEC_UNIT,  MTI_RSFEC_VENDOR_VL2_1,               0x56,       0xFF},    /*VENDOR_VL2_1.set(8'h56)*/
    {MTI_RSFEC_UNIT,  MTI_RSFEC_VENDOR_VL3_0,               0x8086,     0xFFFF},  /*VENDOR_VL3_0.set(16'h8086)*/
    {MTI_RSFEC_UNIT,  MTI_RSFEC_VENDOR_VL3_1,               0xd0,       0xFF},    /*VENDOR_VL3_1.set(8'hd0)*/

    {MTI_RSFEC_UNIT,  MTI_RSFEC_VENDOR_VL4_0,               0x512a,     0xFFFF}, /*VENDOR_VL4_0.set(16'h512a)*/
    {MTI_RSFEC_UNIT,  MTI_RSFEC_VENDOR_VL4_1,               0xf2,       0xFF},   /*VENDOR_VL4_1.set(8'f2)*/
    {MTI_RSFEC_UNIT,  MTI_RSFEC_VENDOR_VL5_0,               0x4f12,     0xFFFF}, /*VENDOR_VL5_0.set(16'h4f12)*/
    {MTI_RSFEC_UNIT,  MTI_RSFEC_VENDOR_VL5_1,               0xd1,       0xFF},   /*VENDOR_VL5_1.set(8'hd1)*/
    {MTI_RSFEC_UNIT,  MTI_RSFEC_VENDOR_VL6_0,               0x9c42,     0xFFFF}, /*VENDOR_VL6_0.set(16'h9c42)*/
    {MTI_RSFEC_UNIT,  MTI_RSFEC_VENDOR_VL6_1,               0xa1,       0xFF},   /*VENDOR_VL6_1.set(8'ha1)*/
    {MTI_RSFEC_UNIT,  MTI_RSFEC_VENDOR_VL7_0,               0x76d6,     0xFFFF}, /*VENDOR_VL7_0.set(16'h76d6)*/
    {MTI_RSFEC_UNIT,  MTI_RSFEC_VENDOR_VL7_1,               0x5b,       0xFF},   /*VENDOR_VL7_1.set(8'h5b)*/
};

static const MV_OP_PARAMS mtiPcs200StartSendFaultSeqParams[] = {
    {MTI_EXT_UNIT, MTIP_EXT_SEG_PORT_CONTROL,               0x2,        0x2}
};

static const MV_OP_PARAMS mtiPcs200StopSendFaultSeqParams[] = {
    {MTI_EXT_UNIT, MTIP_EXT_SEG_PORT_CONTROL,               0x0,        0x2}
};

static const MV_OP_PARAMS mtiPcs200LbNormalSeqParams[] = {
    {MTI_PCS200_UNIT, MTI_PCS_CONTROL1,                     0x0,        0x4000}
};

static const MV_OP_PARAMS mtiPcs200LbTx2RxSeqParams[] = {
    {MTI_PCS200_UNIT, MTI_PCS_CONTROL1,                     0x4000,     0x4000}
};

/* seqId to params array mapping */
const MV_MAC_PCS_CFG_SEQ hwsMtiPcs200SeqDb[] =
{
    {mtiPcs200resetSeqParams,            MV_SEQ_SIZE(mtiPcs200resetSeqParams)},           /* MTI_PCS_RESET_SEQ            */
    {mtiPcs200unresetSeqParams,          MV_SEQ_SIZE(mtiPcs200unresetSeqParams)},         /* MTI_PCS_UNRESET_SEQ          */
    {NULL,                               MV_SEQ_SIZE(NULL)},                              /* MTI_PCS_POWER_DOWN_SEQ       */

    {NULL,                               MV_SEQ_SIZE(NULL)},                              /* MTI_PCS_XG_MODE_SEQ     */
    {NULL,                               MV_SEQ_SIZE(NULL)},                              /* MTI_PCS_XG_25_MODE_SEQ */
    {NULL,                               MV_SEQ_SIZE(NULL)},                              /* MTI_PCS_XG_25_RS_FEC_MODE_SEQ */

    {NULL,                               MV_SEQ_SIZE(NULL)},                              /* MTI_PCS_XLG_50R1_MODE_SEQ    */
    {NULL,                               MV_SEQ_SIZE(NULL)},                              /* MTI_PCS_XLG_50R2_RS_FEC_MODE_SEQ    */
    {NULL,                               MV_SEQ_SIZE(NULL)},                              /* MTI_PCS_XlG_MODE_SEQ    */

    {NULL,                               MV_SEQ_SIZE(NULL)},                              /* MTI_PCS_CG_MODE_SEQ     */

    {mtiPcs200R4ModeSeqParams,           MV_SEQ_SIZE(mtiPcs200R4ModeSeqParams)},          /* MTI_PCS_200R4_MODE_SEQ     */
    {NULL,                               MV_SEQ_SIZE(NULL)},                              /* MTI_PCS_200R8_MODE_SEQ     */
    {NULL,                               MV_SEQ_SIZE(NULL)},                              /* MTI_PCS_400R8_MODE_SEQ     */

    {mtiPcs200StartSendFaultSeqParams,   MV_SEQ_SIZE(mtiPcs200StartSendFaultSeqParams)},  /* MTI_PCS_START_SEND_FAULT_SEQ */
    {mtiPcs200StopSendFaultSeqParams,    MV_SEQ_SIZE(mtiPcs200StopSendFaultSeqParams)},   /* MTI_PCS_STOP_SEND_FAULT_SEQ  */
    {mtiPcs200LbNormalSeqParams,         MV_SEQ_SIZE(mtiPcs200LbNormalSeqParams)},        /* MTI_PCS_LPBK_NORMAL_SEQ      */
    {mtiPcs200LbTx2RxSeqParams,          MV_SEQ_SIZE(mtiPcs200LbTx2RxSeqParams)},         /* MTI_PCS_LPBK_TX2RX_SEQ       */

    {mtiPcs200PowerDownSeqParams,        MV_SEQ_SIZE(mtiPcs200PowerDownSeqParams)},       /* MTI_PCS_200G_R4_POWER_DOWN_SEQ */
    {NULL,                               MV_SEQ_SIZE(NULL)},                              /* MTI_PCS_POWER_DOWN_NO_RS_FEC_SEQ */
#if (!defined(MV_HWS_REDUCED_BUILD_EXT_CM3)) || defined (FALCON_DEV_SUPPORT)
    {NULL,                               MV_SEQ_SIZE(NULL)},                              /* MTI_PCS_LPBK_TX2RX_WA_SEQ */
#endif
};

GT_STATUS hwsMtiPcs200SeqGet(MV_HWS_MTI_PCS_SUB_SEQ seqType, MV_OP_PARAMS *seqLine, GT_U32 lineNum)
{
  if (seqType >= MTI_PCS_LAST_SEQ)
  {
      return GT_NO_SUCH;
  }

  *seqLine = hwsMtiPcs200SeqDb[seqType].cfgSeq[lineNum];
  return GT_OK;
}
