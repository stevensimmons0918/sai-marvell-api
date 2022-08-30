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
* mvHwsMtiCpuMacDb.h
*
* DESCRIPTION:
*
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*
*******************************************************************************/

#ifndef __mvhwsMtiCpuMacDbIf_H
#define __mvhwsMtiCpuMacDbIf_H

#ifdef __cplusplus
extern "C" {
#endif

/* General H Files */
#include <gtOs/gtGenTypes.h>
#include <cpss/common/labServices/port/gop/common/configElementDb/mvCfgElementDb.h>

typedef enum
{
    MTI_CPU_MAC_XLG_MODE_SEQ,       /* 10G */
    MTI_CPU_MAC_XLG_POWER_DOWN_SEQ,
    MTI_CPU_MAC_XLG_UNRESET_SEQ,
    MTI_CPU_MAC_LAST_SEQ
}HWS_MTI_CPU_MAC_SUB_SEQ;

/* MTI MAC sequences */
extern const MV_MAC_PCS_CFG_SEQ hwsMtiCpuMacSeqDb[MTI_CPU_MAC_LAST_SEQ];

GT_STATUS hwsMtiCpuMacSeqGet(HWS_MTI_CPU_MAC_SUB_SEQ seqType, MV_OP_PARAMS *seqLine, GT_U32 lineNum);

#ifdef __cplusplus
}
#endif

#endif /* __mvhwsMtiCpuMacDbIf_H */

