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
* @file prvTgfPacketAnalyzerDebugManagerReset.h
*
* @brief Test Packet Analyzer functionality when sending UC trafffic
*
* @version   1
********************************************************************************
*/
#ifndef __prvTgfPacketAnalyzerDebugManagerReseth
#define __prvTgfPacketAnalyzerDebugManagerReseth

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
* @internal GT_VOID prvTgfPacketAnalyzerDebugManagerResetConfigSet
*           function
* @endinternal
*
* @brief   Set Packet Analyzer test configuration
*/
GT_VOID prvTgfPacketAnalyzerDebugManagerResetConfigSet
(
    GT_BOOL           skipManagerCreatAndCountersClear
);

/**
* @internal GT_VOID prvTgfPacketAnalyzerDebugManagerResetTrafficGenerate
*           function
* @endinternal
*
* @brief   Generate traffic
*/
GT_VOID prvTgfPacketAnalyzerDebugManagerResetTrafficGenerate
(
     GT_U32         packetToSend
);

/**
* @internal GT_VOID prvTgfPacketAnalyzerDebugManagerResetChangeBinding
*           function
* @endinternal
*
* @brief   Replace bind state for stage.
*/
GT_VOID prvTgfPacketAnalyzerDebugManagerResetChangeBinding
(
   CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT           stage,
   GT_BOOL                                               isBind
);

/**
* @internal GT_VOID prvTgfPacketAnalyzerDebugManagerResetToDefaults
*           function
* @endinternal
*
* @brief   reset manager to defaults values.
*/
GT_VOID prvTgfPacketAnalyzerDebugManagerResetToDefaults
(
    GT_VOID
);

/**
* @internal GT_VOID prvTgfPacketAnalyzerDebugInterfaceReset
*           function
* @endinternal
*
* @brief   reset Interface.
*/
GT_VOID prvTgfPacketAnalyzerDebugInterfaceReset
(
    GT_BOOL                 reset
);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif
