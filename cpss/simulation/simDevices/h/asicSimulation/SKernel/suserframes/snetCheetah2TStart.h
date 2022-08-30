/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* snetCht2TStart.h
*
* DESCRIPTION:
*       Cheetah2 Asic Simulation .
*       Tunnel Termination Engine processing for incoming frame.
*       header file.
*
* DEPENDENCIES:
*       None.
*
* FILE REVISION NUMBER:
*       $Revision: 5 $
*
*******************************************************************************/
#ifndef snetCht2TStart
#define __snetCht2TStart



#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */



/**
* @internal snetCht2HaTunnelStart function
* @endinternal
*
* @brief   snetCht2TStart Engine processing for outgoing frame on Cheetah2
*         asic simulation.
*         snetCht2TStart processing , tt assignment ,key forming , 1 Lookup ,
*         actions to descriptor processing
* @param[in] devObjPtr                - pointer to device object.
* @param[in,out] descrPtr                 - pointer to frame data buffer Id
* @param[in] haInfoPtr                - HA internal info
* @param[in] egressPort               - the local egress port (not global port)
* @param[in,out] descrPtr                 - pointer to updated frame data buffer Id
*                                      RETURN:
*                                      COMMENTS:
*/
GT_VOID snetCht2HaTunnelStart
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN HA_INTERNAL_INFO_STC*     haInfoPtr,
    IN GT_U32   egressPort
);

/**
* @internal snetLion3HaTunnelStart function
* @endinternal
*
* @brief   tunnel start for SIP5
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] descrPtr                 - pointer to the frame's descriptor.
* @param[in,out] haInfoPtr                - ha internal info
* @param[in] egressPort               - the local egress port (not global port)
*
* @param[out] descrPtr                 - pointer to updated frame data buffer Id
* @param[in,out] haInfoPtr                - ha internal info
*                                      RETURN:
*/
GT_VOID snetLion3HaTunnelStart
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    INOUT HA_INTERNAL_INFO_STC*     haInfoPtr,
    IN GT_U32   egressPort
);



#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif  /* __snetCheetah2EPclh */



