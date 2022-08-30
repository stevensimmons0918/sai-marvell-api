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
* @file prvTgfCncEgressQuePassDropRecudedMode.h
*
* @brief Centralized Counters (CNC) - test for Egress Queue CNC client in reduced mode with configured Queue ID.
*
* @version  1
********************************************************************************
*/
#ifndef __prvTgfCncEgressQuePassDropRecudedMode
#define __prvTgfCncEgressQuePassDropRecudedMode

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
* @internal prvTgfCncEgressPassDropQueIdTest function 
* @endinternal
*
* @brief    Configure Queue group base for specific port and CNC client. 
*           Generate traffic: Send to device's port given packet: Check
*           counters.
*/
GT_VOID prvTgfCncEgressPassDropRecudedModeQueIdTest
(
    GT_VOID
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfCncEgressQuePassDropRecudedMode */




