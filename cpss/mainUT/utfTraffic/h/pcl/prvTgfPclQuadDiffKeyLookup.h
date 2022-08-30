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
* @file prvTgfPclQuadDiffKeyLookup.h
*
* @brief PCL Quad lookup of parallel different keys test header file
*
* @version   1
********************************************************************************
*/
#ifndef __prvTgfPclQuadDiffKeyLookup
#define __prvTgfPclQuadDiffKeyLookup

#include <trafficEngine/tgfTrafficGenerator.h>

#include <common/tgfCommon.h>
#include <common/tgfPclGen.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
* @internal prvTgfPclQuadDiffKeySendAndCheckPacket function
* @endinternal
*
* @brief   Send packet and check VID in egress packet.
*
* @param[in] prvTgfPacketInfoPtr  -  pattern packet to send
* @param[in] macDaBits_15_0       -  MAC DA bits[15:0] to modify before send
* @param[in] newVid               -  VID to check in egress packet VLAN tag
*
*/
GT_VOID prvTgfPclQuadDiffKeySendAndCheckPacket
(
    IN  TGF_PACKET_STC   *prvTgfPacketInfoPtr,
    IN  GT_U16           macDaBits_15_0,
    IN  GT_U16           newVid
);

/**
* @internal prvTgfPclQuadDiffKeyLookupIPCL0Test function
* @endinternal
*
* @brief   IPCL Quad lookup of parallel different keys test
*/
GT_VOID prvTgfPclQuadDiffKeyLookupIPCL0Test
(
    GT_VOID
);

/**
* @internal prvTgfPclQuadDiffKeyLookupIPCL1Test function
* @endinternal
*
* @brief   IPCL Quad lookup of parallel different keys test
*/
GT_VOID prvTgfPclQuadDiffKeyLookupIPCL1Test
(
    GT_VOID
);

/**
* @internal prvTgfPclQuadDiffKeyLookupIPCL2Test function
* @endinternal
*
* @brief   IPCL Quad lookup of parallel different keys test
*/
GT_VOID prvTgfPclQuadDiffKeyLookupIPCL2Test
(
    GT_VOID
);

/**
* @internal prvTgfPclQuadDiffKeyLookupEPCLTest function
* @endinternal
*
* @brief   IPCL Quad lookup of parallel different keys test
*/
GT_VOID prvTgfPclQuadDiffKeyLookupEPCLTest
(
    GT_VOID
);

/**
* @internal prvTgfPclQuadDiffKeyLookupTTIPortModeTest function
* @endinternal
*
* @brief   TTI Quad lookup of parallel different keys test
*/
GT_VOID prvTgfPclQuadDiffKeyLookupTTIPortModeTest
(
    GT_VOID
);

/**
* @internal prvTgfPclQuadDiffKeyLookupTTIPktTypeModeTest function
* @endinternal
*
* @brief   TTI Quad lookup of parallel different keys test
*/
GT_VOID prvTgfPclQuadDiffKeyLookupTTIPktTypeModeTest
(
    GT_VOID
);

/**
* @internal prvTgfPclQuadDiffKeyLookupTTIPortAndPktTypeModeTest function
* @endinternal
*
* @brief   TTI Quad lookup of parallel different keys test
*/
GT_VOID prvTgfPclQuadDiffKeyLookupTTIPortAndPktTypeModeTest
(
    GT_VOID
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfPclQuadDiffKeyLookup */

