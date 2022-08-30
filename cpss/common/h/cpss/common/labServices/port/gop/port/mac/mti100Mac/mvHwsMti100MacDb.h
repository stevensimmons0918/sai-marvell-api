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
* mvHwsMti100MacDb.h
*
* DESCRIPTION:
*
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*
*******************************************************************************/

#ifndef __mvhwsMti100MacDbIf_H
#define __mvhwsMti100MacDbIf_H

#ifdef __cplusplus
extern "C" {
#endif

/* General H Files */
#include <gtOs/gtGenTypes.h>
#include <cpss/common/labServices/port/gop/common/configElementDb/mvCfgElementDb.h>

typedef enum
{
    HWS_MTI_MAC100_XG_MODE_SEQ,                      /* 10G - 25G */
    HWS_MTI_MAC100_XG_25RSFEC_MODE_SEQ,              /* 25G rsfec*/
    HWS_MTI_MAC100_XLG_MODE_SEQ,                     /* 50R1 */
    HWS_MTI_MAC100_XLG_NRZ_MODE_SEQ,                 /* 40R4, 50R2, 100R2 */
    HWS_MTI_MAC100_XLG_NRZ_SPECIAL_SPEEDS_SEQ,       /* 106G R4, 42G R4, 53G R2 */
    HWS_MTI_MAC100_XLG_POWER_DOWN_SEQ,
    HWS_MTI_MAC100_XLG_UNRESET_SEQ,
    HWS_MTI_MAC100_LAST_SEQ
}HWS_MTI_MAC100_SUB_SEQ;

/* MTI MAC sequences */
extern const MV_MAC_PCS_CFG_SEQ hwsMti100MacSeqDb[HWS_MTI_MAC100_LAST_SEQ];

GT_STATUS hwsMti100MacSeqGet(HWS_MTI_MAC100_SUB_SEQ seqType, MV_OP_PARAMS *seqLine, GT_U32 lineNum);

#ifdef __cplusplus
}
#endif

#endif /* __mvhwsMti100MacDbIf_H */

