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
* @file snetGm.h
*
* @brief This is a external API definition for snet Gm module of SKernel.
*
* @version   2
********************************************************************************
*/
#ifndef __snetGmh
#define __snetGmh


#include <asicSimulation/SKernel/smain/smain.h>
#include <common/Utils/FrameInfo/sframeInfoAddr.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
* @internal snetGmProcessInit function
* @endinternal
*
* @brief   Init module.
*
* @param[in] devObjPtr                - pointer to device object.
*/
void snetGmProcessInit
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr
);

/**
* @internal snetGmProcessFrameFromSlan function
* @endinternal
*
* @brief   Process the frame, get and do actions for a frame
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] bufferId                 - frame data buffer Id
* @param[in] srcPort                  - source port number
*
* @retval GT_OK                    - when the data is sent successfully.
* @retval GT_FAIL                  - in other case.
*
* @note The function is used by the FA and by the PP.
*
*/
GT_VOID snetGmProcessFrameFromSlan
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SBUF_BUF_ID bufferId,
    IN GT_U32 srcPort
);

/**
* @internal snetGmLinkStateNotify function
* @endinternal
*
* @brief   Notify devices database that link state changed
*/
GT_VOID snetGmLinkStateNotify
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32 port,
    IN GT_U32 linkState
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif  /* __snetGmh */
