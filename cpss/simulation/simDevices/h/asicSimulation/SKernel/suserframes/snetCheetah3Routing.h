/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* snetCheetahRouting.h
*
* DESCRIPTION:
*       This is a external API definition for Router Engine Processing
*
* DEPENDENCIES:
*       None.
*
* FILE REVISION NUMBER:
*       $Revision: 7 $
*
*******************************************************************************/
#ifndef __snetCheetah3Routingh
#define __snetCheetah3Routingh

#include <asicSimulation/SKernel/smain/smain.h>
#include <asicSimulation/SKernel/suserframes/snetCheetah2Routing.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
* @internal snetCht3UpdateRpfCheckMode function
* @endinternal
*
* @brief   updates rpf check mode (relevant for cheetah 3 and above)
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] descrPtr                 - pointer to the frame's descriptor.
*                                      rpfCheckModePtr  - weather to make rpfCheck
*/
GT_VOID snetCht3UpdateRpfCheckMode
(
    IN    SKERNEL_DEVICE_OBJECT           *devObjPtr,
    IN    SKERNEL_FRAME_CHEETAH_DESCR_STC *descrPtr,
    INOUT SNET_RPF_CHECK_MODE_ENT         *unicastRpfCheckModePtr
);

/**
* @internal snetCht3UpdateSipSaCheckEnable function
* @endinternal
*
* @brief   updates sip sa check enable (relevant for cheetah 3 and above)
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] descrPtr                 - pointer to the frame's descriptor.
* @param[in,out] unicastSipSaCheckPtr     - weather to make sip sa check
* @param[in,out] unicastSipSaCheckPtr     - weather to make sip sa check
*/
GT_VOID snetCht3UpdateSipSaCheckEnable
(
    IN    SKERNEL_DEVICE_OBJECT           *devObjPtr,
    IN    SKERNEL_FRAME_CHEETAH_DESCR_STC *descrPtr,
    INOUT GT_U32                          *unicastSipSaCheckPtr
);

/**
* @internal snetCht3L3iFetchRouteEntry function
* @endinternal
*
* @brief   Fetch the lookup translation table entry .
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] descrPtr                 - pointer to frame descriptor.
* @param[in] matchIndexPtr            - pointer to the matching index.
*
* @param[out] routeIndexPtr            - pointer to the matching table.
* @param[out] ipSecurChecksInfoPtr     - routing security checks information
*                                      RETURN:
*                                      COMMENTS:
*                                      Router LTT Table:
*                                      CH3:  LTT holds up to 5K lines.
*                                      XCAT: LTT holds up to 3.25K lines.
*
* @note Router LTT Table:
*       CH3: LTT holds up to 5K lines.
*       XCAT: LTT holds up to 3.25K lines.
*
*/
GT_VOID snetCht3L3iFetchRouteEntry
(
    IN  SKERNEL_DEVICE_OBJECT               *devObjPtr,
    IN  SKERNEL_FRAME_CHEETAH_DESCR_STC     *descrPtr,
    IN  GT_U32                              *matchIndexPtr,
    OUT GT_U32                              *routeIndexPtr,
    OUT SNET_ROUTE_SECURITY_CHECKS_INFO_STC *ipSecurChecksInfoPtr
);

/**
* @internal snetCht3L3iCounters function
* @endinternal
*
* @brief   Update Bridge counters
*/
GT_VOID snetCht3L3iCounters
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN SNET_CHEETAH2_L3_CNTRL_PACKET_INFO * cntrlPcktInfoPtr ,
    IN GT_U32 * routeIndexPtr
);

/**
* @internal snetCht3L3iTcamLookUp function
* @endinternal
*
* @brief   Tcam lookup for IPv4/6 address .
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] descrPtr                 - pointer to frame descriptor.
*
* @param[out] matchIndex               - pointer to the matching index.
*                                      RETURN:
*                                      COMMENTS:
*                                      C.12.8.4  Router Tcam Table :   TCAM holds 5119 entries
*                                      of 32 bits for IPv4 or 128 bits for IPv6.
*
* @note C.12.8.4 Router Tcam Table :  TCAM holds 5119 entries
*       of 32 bits for IPv4 or 128 bits for IPv6.
*
*/
GT_VOID snetCht3L3iTcamLookUp
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr  ,
    OUT GT_U32 *matchIndex
);


#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif  /* __snetCheetah3Routing */










