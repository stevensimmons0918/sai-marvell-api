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
* @file simLogInfoTypeTcam.h
*
* @brief simulation logger tcam functions
*
* @version   2
********************************************************************************
*/
#ifndef __simLogTcam_h__
#define __simLogTcam_h__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <os/simTypes.h>

/**
* @internal simLogTcamTTNotMatch function
* @endinternal
*
* @brief   log tcam key info
*
* @param[in] devObjPtr                - device object pointer
* @param[in] ttSearchKey16Bits        - tcam key (16 bits)
* @param[in] ttSearchKey32Bits        - tcam key (32 bits)
* @param[in] xdataPtr                 - pointer to routing TCAM data X entry
* @param[in] xctrlPtr                 - pointer to routing TCAM ctrl X entry
* @param[in] ydataPtr                 - pointer to routing TCAM data Y entry
* @param[in] yctrlPtr                 - pointer to routing TCAM ctrl Y entry
*                                       None.
*/
GT_VOID simLogTcamTTNotMatch
(
    IN SKERNEL_DEVICE_OBJECT  *devObjPtr,
    IN GT_U32                  ttSearchKey16Bits,
    IN GT_U32                  ttSearchKey32Bits,
    IN GT_U32                 *xdataPtr,
    IN GT_U32                 *ydataPtr,
    IN GT_U32                 *xctrlPtr,
    IN GT_U32                 *yctrlPtr
);

/**
* @internal simLogTcamTTKey function
* @endinternal
*
* @brief   log tcam key info
*
* @param[in] devObjPtr                - device object pointer
* @param[in] ttSearchKey16Bits        - tcam key (16 bits)
* @param[in] ttSearchKey32Bits        - tcam key (32 bits)
*                                       None.
*/
GT_VOID simLogTcamTTKey
(
    IN SKERNEL_DEVICE_OBJECT *devObjPtr,
    IN GT_U32                 ttSearchKey16Bits,
    IN GT_U32                 ttSearchKey32Bits
);

/**
* @internal simLogTcamBitsCausedNoMatchInTheEntry function
* @endinternal
*
* @brief   log tcam bits caused no match in the entry
*
* @param[in] devObjPtr                - device object pointer
* @param[in] bits                     -  caused no match in the entry
*                                       None.
*/
GT_VOID simLogTcamBitsCausedNoMatchInTheEntry
(
    IN SKERNEL_DEVICE_OBJECT *devObjPtr,
    IN GT_U32                 bits
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __simLogTcam_h__ */


