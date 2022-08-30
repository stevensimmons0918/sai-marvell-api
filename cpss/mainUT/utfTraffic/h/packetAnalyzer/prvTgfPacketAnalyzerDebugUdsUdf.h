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
* @file prvTgfPacketAnalyzerDebugUdsUdf.h
*
* @brief Test Packet Analyzer functionality when sending UC trafffic
*
* @version   1
********************************************************************************
*/
#ifndef __prvTgfPacketAnalyzerDebugUdsUdfh
#define __prvTgfPacketAnalyzerDebugUdsUdfh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
* @internal GT_VOID prvTgfPacketAnalyzerDebugUdsUdfConfigSet
*           function
* @endinternal
*
* @brief   Set Packet Analyzer test configuration
*/
GT_VOID prvTgfPacketAnalyzerDebugUdsUdfConfigSet
(
    GT_VOID
);

/**
* @internal GT_VOID prvTgfPacketAnalyzerDebugUdsUdfTrafficGenerate
*           function
* @endinternal
*
* @brief   Generate traffic
*/
GT_VOID prvTgfPacketAnalyzerDebugUdsUdfTrafficGenerate
(
     GT_U32         packetToSend
);

/**
* @internal GT_VOID prvTgfPacketAnalyzerDebugUdsUdfUpdateRule
*           function
* @endinternal
*
* @brief   change rule data and set Sampling Enable.
*/
GT_VOID prvTgfPacketAnalyzerDebugUdsUdfUpdateRule
(
   GT_U32                                     numberOfFields,
   CPSS_DXCH_PACKET_ANALYZER_FIELD_VALUE_STC  fieldsValueArr[],
   GT_BOOL                                    setField
);

/**
* @internal GT_VOID prvTgfPacketAnalyzerDebugUdsConfigSet
*           function
* @endinternal
*
* @brief   Set Packet Analyzer test configuration
*/
GT_VOID prvTgfPacketAnalyzerDebugUdsConfigSet
(
    GT_BOOL concatinatedInterface
);

/**
* @internal GT_VOID prvTgfPacketAnalyzerDebugUdsResultsGet
*           function
* @endinternal
*
* @brief   Check Packet Analyzer test Results
*/
GT_VOID prvTgfPacketAnalyzerDebugUdsResultsGet
(
   IN GT_U32                                                numOfHitArr[],
   IN GT_U32                                                expSample[],
   IN GT_BOOL                                               concatinated
);

/**
* @internal GT_VOID prvTgfPacketAnalyzerDebugUdfConfigSet
*           function
* @endinternal
*
* @brief   Set Packet Analyzer test configuration
*/
GT_VOID prvTgfPacketAnalyzerDebugUdfConfigSet
(
    GT_VOID
);

GT_VOID prvTgfPacketAnalyzerDebugAsymmetricalInstancesConfigSet
(
    GT_VOID
);
#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif
