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
* @file prvTgfCncEgrPacketType.h
*
* @brief Centralized Counters (Cnc)
*
* @version  1
********************************************************************************
*/
#ifndef __prvTgfCncEgrPacketType
#define __prvTgfCncEgrPacketType

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define PRV_TGF_CNC_TEST_VID 5

#define PRV_TGF_CNC_SEND_PORT_INDEX_CNS 1

#define PRV_TGF_CNC_RECEIVE_PORT_INDEX_CNS 2

/* tested block index */
#define PRV_TGF_CNC_TESTED_BLOCK_NUM_MAC() (PRV_TGF_CNC_BLOCKS_NUM_MAC() - 1)

/**
* @internal prvTgfCncTestVlanL2L3TrafficGenerateAndCheck function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port given packet:
*         Check counters.
*/
GT_VOID prvTgfCncTestEgrPacketTypeTrafficGenerateAndCheck
(
    GT_VOID
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfCncEgrPacketType */


