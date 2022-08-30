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
* @file mvHwsMtiPcsDb.h
*
* @brief
*
* @version   1
********************************************************************************
*/

#ifndef __mvHwsMtiPcsDb_H
#define __mvHwsMtiPcsDb_H

#ifdef __cplusplus
extern "C" {
#endif

/* General H Files */
#include <cpss/common/labServices/port/gop/common/configElementDb/mvCfgElementDb.h>

typedef enum
{
    MTI_PCS_RESET_SEQ,
    MTI_PCS_UNRESET_SEQ,
    MTI_PCS_POWER_DOWN_SEQ,

    MTI_PCS_XG_MODE_SEQ,                /* 5G - 25G */
    MTI_PCS_XG_25_MODE_SEQ,             /* 25G with NO RS-FEC */
    MTI_PCS_XG_25_RS_FEC_MODE_SEQ,      /* 25G with RS-FEC 528 */
    MTI_PCS_XLG_50R1_MODE_SEQ,          /* 50G R1 */
    MTI_PCS_XLG_50R2_RS_FEC_MODE_SEQ,   /* 50G R2 with RS-FEC 528*/
    MTI_PCS_XLG_MODE_SEQ,               /* 40G - 50G all other*/
    MTI_PCS_CG_MODE_SEQ,                /* 100G  */

    MTI_PCS_200R4_MODE_SEQ,             /* 200G  4 lanes*/
    MTI_PCS_200R8_MODE_SEQ,             /* 200G  8 lanes*/
    MTI_PCS_400R8_MODE_SEQ,             /* 400G  8 lanes*/

    MTI_PCS_START_SEND_FAULT_SEQ,
    MTI_PCS_STOP_SEND_FAULT_SEQ,

    MTI_PCS_LPBK_NORMAL_SEQ,
    MTI_PCS_LPBK_TX2RX_SEQ,

    MTI_PCS_200G_R4_POWER_DOWN_SEQ,

    MTI_PCS_POWER_DOWN_NO_RS_FEC_SEQ,   /* 10G, 25G_NO_FEC, 25_FC_FEC, 40G_R4, all mode without RS_FEC*/
#if (!defined(MV_HWS_REDUCED_BUILD_EXT_CM3)) || defined (FALCON_DEV_SUPPORT)
    MTI_PCS_LPBK_TX2RX_WA_SEQ,          /* WA to raise link change interrupt on internal loopback. */
#endif
    MTI_PCS_LAST_SEQ

}MV_HWS_MTI_PCS_SUB_SEQ;

typedef enum
{
    MTI_CPU_PCS_RESET_SEQ,
    MTI_CPU_PCS_UNRESET_SEQ,
    MTI_CPU_PCS_POWER_DOWN_SEQ,
    MTI_CPU_PCS_XG_MODE_SEQ,                /* 10G */
    MTI_CPU_PCS_XLG_MODE_SEQ,               /* 25G */
    MTI_CPU_PCS_START_SEND_FAULT_SEQ,
    MTI_CPU_PCS_STOP_SEND_FAULT_SEQ,

    MTI_CPU_PCS_LPBK_NORMAL_SEQ,
    MTI_CPU_PCS_LPBK_TX2RX_SEQ,

    MTI_CPU_PCS_LAST_SEQ

}MV_HWS_MTI_CPU_PCS_SUB_SEQ;

typedef enum
{
    MTI_LOW_SPEED_PCS_1000_BASE_X_MODE_SEQ,
    MTI_LOW_SPEED_PCS_1000_BASE_X_POWER_DOWN_SEQ,
    MTI_LOW_SPEED_PCS_1000_BASE_X_RESET_SEQ,
    MTI_LOW_SPEED_PCS_1000_BASE_X_UNRESET_SEQ,
    MTI_LOW_SPEED_PCS_LPBK_NORMAL_SEQ,
    MTI_LOW_SPEED_PCS_LPBK_TX2RX_SEQ,

    MTI_LOW_SPEED_PCS_LAST_SEQ

}MV_HWS_MTI_LOW_SPEED_PCS_SUB_SEQ;


/* PCS reset sequences */
extern const MV_MAC_PCS_CFG_SEQ hwsMtiPcs400SeqDb[MTI_PCS_LAST_SEQ];
extern const MV_MAC_PCS_CFG_SEQ hwsMtiPcs200SeqDb[MTI_PCS_LAST_SEQ];
extern const MV_MAC_PCS_CFG_SEQ hwsMtiPcs100SeqDb[MTI_PCS_LAST_SEQ];
extern const MV_MAC_PCS_CFG_SEQ hwsMtiPcs50SeqDb[MTI_PCS_LAST_SEQ];
extern const MV_MAC_PCS_CFG_SEQ hwsMtiCpuPcsSeqDb[MTI_CPU_PCS_LAST_SEQ];
extern const MV_MAC_PCS_CFG_SEQ hwsMtiLowSpeedPcsSeqDb[MTI_LOW_SPEED_PCS_LAST_SEQ];

GT_STATUS hwsMtiPcs400SeqGet(MV_HWS_MTI_PCS_SUB_SEQ seqType, MV_OP_PARAMS *seqLine, GT_U32 lineNum);
GT_STATUS hwsMtiPcs200SeqGet(MV_HWS_MTI_PCS_SUB_SEQ seqType, MV_OP_PARAMS *seqLine, GT_U32 lineNum);
GT_STATUS hwsMtiPcs100SeqGet(MV_HWS_MTI_PCS_SUB_SEQ seqType, MV_OP_PARAMS *seqLine, GT_U32 lineNum);
GT_STATUS hwsMtiPcs50SeqGet(MV_HWS_MTI_PCS_SUB_SEQ seqType, MV_OP_PARAMS *seqLine, GT_U32 lineNum);
GT_STATUS hwsMtiCpuPcsSeqGet(MV_HWS_MTI_CPU_PCS_SUB_SEQ seqType, MV_OP_PARAMS *seqLine, GT_U32 lineNum);
GT_STATUS hwsMtiLowSpeedPcsSeqGet(MV_HWS_MTI_LOW_SPEED_PCS_SUB_SEQ seqType, MV_OP_PARAMS *seqLine, GT_U32 lineNum);

#ifdef __cplusplus
}
#endif

#endif /* __mvHwsMtiPcsDb_H */


