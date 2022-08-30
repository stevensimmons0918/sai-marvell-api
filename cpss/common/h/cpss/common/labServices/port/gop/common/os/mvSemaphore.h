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
* mvCfgElementDb.h
*
* DESCRIPTION:
*
*
* FILE REVISION NUMBER:
*       $Revision: 12 $
*
*******************************************************************************/
#ifndef _mvSemaphore_h
#define _mvSemaphore_h

#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>

#define MV_SEMA_REG_BASE_CNM_PCI   ( 0x20500 )
#define MV_SEMA_REG_BASE_SWITCH_MG ( 0x800 )
#define MV_MAX_SEMA             128
#define MV_SEMA_AVAGO           0
#define MV_SEMA_SERDES          0   /* Avago for older devices, serdes for newer devices comphy SDs, so they can use the same bit */
#define MV_SEMA_CG              1
#define MV_SEMA_CM3             2
#define MV_SEMA_PM_CM3          3
#define MV_SEMA_LAST            4

GT_BOOL mvSemaLock(GT_U8 devNum
#if !defined MV_HWS_REDUCED_BUILD_EXT_CM3 || defined (FALCON_DEV_SUPPORT)
                   , GT_U32 chipIndex
#endif
                   , GT_U32 num);

GT_BOOL mvSemaTryLock(GT_U8 devNum, GT_U32 num);
GT_BOOL mvSemaUnlock(GT_U8 devNum
#if !defined MV_HWS_REDUCED_BUILD_EXT_CM3 || defined (FALCON_DEV_SUPPORT)
                     , GT_U32 chipIndex
#endif
                     , GT_U32 num);

GT_STATUS mvSemaOwnerGet(GT_U8 devNum
#ifndef MV_HWS_REDUCED_BUILD_EXT_CM3
                         , GT_U32 chipIndex
#endif
                         , GT_U32 *semaOwner);
#endif /* _mvSemaphore_h */
