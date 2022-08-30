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
* @file mvHwsCgPcsDb.h
*
* @brief
*
* @version   1
********************************************************************************
*/

#ifndef __mvHwsCgPcsDbIf_H
#define __mvHwsCgPcsDbIf_H

#ifdef __cplusplus
extern "C" {
#endif

/* General H Files */
#include <gtOs/gtGenTypes.h>
#include <cpss/common/labServices/port/gop/common/configElementDb/mvCfgElementDb.h>


typedef enum
{
    CGPCS_RESET_SEQ,
    CGPCS_UNRESET_SEQ,

    CGPCS_MODE_MISC_SEQ,
    CGPCS_MODE_10_LANE_SEQ,
    CGPCS_MODE_12_LANE_SEQ,

    CGPCS_LPBK_NORMAL_SEQ,
    CGPCS_LPBK_TX2RX_SEQ,

    CGPCS_MODE_1_LANE_0_SEQ,
    CGPCS_MODE_1_LANE_1_SEQ,
    CGPCS_MODE_1_LANE_2_SEQ,
    CGPCS_MODE_1_LANE_3_SEQ,
    CGPCS_MODE_4_LANE_SEQ,
    CGPCS_RS_FEC_4_LANE_SEQ,
    CGPCS_RESET_1_LANE_0_SEQ,
    CGPCS_RESET_1_LANE_1_SEQ,
    CGPCS_RESET_1_LANE_2_SEQ,
    CGPCS_RESET_1_LANE_3_SEQ,
    CGPCS_RESET_4_LANE_SEQ,
    CGPCS_UNRESET_1_LANE_0_SEQ,
    CGPCS_UNRESET_1_LANE_1_SEQ,
    CGPCS_UNRESET_1_LANE_2_SEQ,
    CGPCS_UNRESET_1_LANE_3_SEQ,
    CGPCS_UNRESET_4_LANE_SEQ,
    CGPCS_POWER_DOWN_1_LANE_0_SEQ,
    CGPCS_POWER_DOWN_1_LANE_1_SEQ,
    CGPCS_POWER_DOWN_1_LANE_2_SEQ,
    CGPCS_POWER_DOWN_1_LANE_3_SEQ,
    CGPCS_POWER_DOWN_4_LANE_SEQ,
    CGPCS_REDUCED_POWER_DOWN_1_LANE_0_SEQ,
    CGPCS_REDUCED_POWER_DOWN_1_LANE_1_SEQ,
    CGPCS_REDUCED_POWER_DOWN_1_LANE_2_SEQ,
    CGPCS_REDUCED_POWER_DOWN_1_LANE_3_SEQ,
#if !defined(ALDRIN_DEV_SUPPORT) && !defined(BOBK_DEV_SUPPORT) && !defined(BC2_DEV_SUPPORT) && !defined(AC5_DEV_SUPPORT) && !defined(MV_HWS_REDUCED_BUILD_EXT_CM3)
    CGPCS_MODE_4_LANE_MLG_SEQ,
    CGPCS_RESET_4_LANE_MLG_SEQ,
    CGPCS_POWER_DOWN_4_LANE_MLG_SEQ,
    CGPCS_UNRESET_4_LANE_MLG_SEQ,
#endif
    CGPCS_START_SEND_FAULT_SEQ,
    CGPCS_STOP_SEND_FAULT_SEQ,
#if !defined(MV_HWS_REDUCED_BUILD)
    CGPCS_UNRESET_4_LANE_UPDATE_SEQ,
#endif

    MV_CGPCS_LAST_SEQ
}MV_HWS_CGPCS_SUB_SEQ;

extern const MV_MAC_PCS_CFG_SEQ *hwsCgPcsPscSeqDb;

GT_STATUS hwsCgPcsSeqGet(MV_HWS_CGPCS_SUB_SEQ seqType, MV_OP_PARAMS *seqLine, GT_U32 lineNum);
GT_STATUS hwsCgPcsSeqSet(GT_BOOL firstLine, MV_HWS_CGPCS_SUB_SEQ seqType, MV_OP_PARAMS *seqLine, GT_U32 lineNum);

#ifdef __cplusplus
}
#endif

#endif /* __mvHwsXPcsDbIf_H */


