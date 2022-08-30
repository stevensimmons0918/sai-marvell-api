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
* @file mvHwsMtiCpuPcsDb.c
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

static const MV_OP_PARAMS mtiCpuPcsResetSeqParams[] = {
    {MTI_CPU_PCS_UNIT, MTI_PCS_CONTROL1,     /*dummy write 0x8000*/0x0000,     0x8000}  /*CONTROL1.Reset.set(1)*/

};

static const MV_OP_PARAMS mtiCpuPcsUnresetSeqParams[] = {
    {MTI_CPU_PCS_UNIT, MTI_PCS_CONTROL1,           0x8000,     0x8000}   /*CONTROL1.Reset.set(1) - SELF CLEARED */
};

static const MV_OP_PARAMS mtiCpuPcsPowerDownSeqParams[] = {
    {MTI_CPU_PCS_UNIT, MTI_PCS_VENDOR_PCS_MODE,    0x0,        0x4},     /*VENDOR_PCS_MODE.Disable_mld;*/
    {MTI_CPU_PCS_UNIT, MTI_PCS_VENDOR_PCS_MODE,    0x1,        0x1},     /*VENDOR_PCS_MODE.Ena_clause49;*/
    {MTI_CPU_PCS_UNIT, MTI_PCS_VENDOR_VL_INTVL,    0x3FFF,     0xFFFF},  /*VENDOR_VL_INTVL.Marker_counter;*/
};

static const MV_OP_PARAMS mtiCpuPcsXgModeSeqParams[] = {    /* 10G */
    {MTI_CPU_PCS_UNIT, MTI_PCS_VENDOR_PCS_MODE,    0x0,        0x4},     /*VENDOR_PCS_MODE.Hi_ber25.set(0);*/
    {MTI_CPU_PCS_UNIT, MTI_PCS_VENDOR_PCS_MODE,    0x1,        0x1},     /*VENDOR_PCS_MODE.Ena_clause49.set(1);*/
    {MTI_CPU_PCS_UNIT, MTI_PCS_VENDOR_VL_INTVL,    0x7F,       0xFFFF},  /*VENDOR_VL_INTVL.Marker_counter(127);*/
};

static const MV_OP_PARAMS mtiCpuPcsXlgModeSeqParams[] = {    /* 25G */
    {MTI_CPU_PCS_UNIT, MTI_PCS_VENDOR_PCS_MODE,    0x1,        0x4},     /*VENDOR_PCS_MODE.Hi_ber25.set(1);*/
    {MTI_CPU_PCS_UNIT, MTI_PCS_VENDOR_PCS_MODE,    0x1,        0x1},     /*VENDOR_PCS_MODE.Ena_clause49.set(1);*/
    {MTI_CPU_PCS_UNIT, MTI_PCS_VENDOR_VL_INTVL,    0x7F,       0xFFFF},  /*VENDOR_VL_INTVL.Marker_counter(127);*/
};

static const MV_OP_PARAMS mtiCpuPcsLbNormalSeqParams[] = {
    {MTI_CPU_EXT_UNIT, MTIP_CPU_EXT_PORT_CONTROL,  0x0,        0x8000},
    {MTI_CPU_MAC_UNIT, MTI_MAC_TX_IPG_LENGTH,      0x0,        0xFFFF0000},
    {MTI_CPU_PCS_UNIT, MTI_PCS_CONTROL1,           0x0,        0x4000}
};                                                             

static const MV_OP_PARAMS mtiCpuPcsLbTx2RxSeqParams[] = {            
    {MTI_CPU_EXT_UNIT, MTIP_CPU_EXT_PORT_CONTROL,  0x8000,     0x8000},      /* CPU port is more tricky, because clock mux is needed */
    {MTI_CPU_MAC_UNIT, MTI_MAC_TX_IPG_LENGTH,      0xA0000,    0xFFFF0000},  /* reduce IPG and provide slight speed up (arbitrary small value) */
    {MTI_CPU_PCS_UNIT, MTI_PCS_CONTROL1,           0x4000,     0x4000}
    };

/* seqId to params array mapping */
const MV_MAC_PCS_CFG_SEQ hwsMtiCpuPcsSeqDb[] =
{
    {mtiCpuPcsResetSeqParams,           MV_SEQ_SIZE(mtiCpuPcsResetSeqParams)},          /* MTI_CPU_PCS_RESET_SEQ            */
    {mtiCpuPcsUnresetSeqParams,         MV_SEQ_SIZE(mtiCpuPcsUnresetSeqParams)},        /* MTI_CPU_PCS_UNRESET_SEQ          */
    {mtiCpuPcsPowerDownSeqParams,       MV_SEQ_SIZE(mtiCpuPcsPowerDownSeqParams)},      /* MTI_CPU_PCS_POWER_DOWN_SEQ       */
    {mtiCpuPcsXgModeSeqParams,          MV_SEQ_SIZE(mtiCpuPcsXgModeSeqParams)},         /* MTI_CPU_PCS_XG_MODE_SEQ          */
    {mtiCpuPcsXlgModeSeqParams,         MV_SEQ_SIZE(mtiCpuPcsXlgModeSeqParams)},        /* MTI_CPU_PCS_XLG_MODE_SEQ         */
    {NULL,                              MV_SEQ_SIZE(NULL)},                             /* MTI_CPU_PCS_START_SEND_FAULT_SEQ */
    {NULL,                              MV_SEQ_SIZE(NULL)},                             /* MTI_CPU_PCS_STOP_SEND_FAULT_SEQ  */
    {mtiCpuPcsLbNormalSeqParams,        MV_SEQ_SIZE(mtiCpuPcsLbNormalSeqParams)},       /* MTI_CPU_PCS_LPBK_NORMAL_SEQ      */
    {mtiCpuPcsLbTx2RxSeqParams,         MV_SEQ_SIZE(mtiCpuPcsLbTx2RxSeqParams)},        /* MTI_CPU_PCS_LPBK_TX2RX_SEQ       */

};

GT_STATUS hwsMtiCpuPcsSeqGet(MV_HWS_MTI_CPU_PCS_SUB_SEQ seqType, MV_OP_PARAMS *seqLine, GT_U32 lineNum)
{
  if (seqType >= MTI_CPU_PCS_LAST_SEQ)
  {
      return GT_NO_SUCH;
  }

  *seqLine = hwsMtiCpuPcsSeqDb[seqType].cfgSeq[lineNum];
  return GT_OK;
}
