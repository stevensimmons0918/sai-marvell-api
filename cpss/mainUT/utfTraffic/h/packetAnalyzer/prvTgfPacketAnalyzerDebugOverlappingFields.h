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
* @file prvTgfPacketAnalyzerDebugOverlappingFields.h
*
* @brief Test Packet Analyzer functionality when sending UC trafffic
*
* @version   1
********************************************************************************
*/
#ifndef __prvTgfPacketAnalyzerDebugOverlappingFieldsh
#define __prvTgfPacketAnalyzerDebugOverlappingFieldsh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
* @internal GT_VOID prvTgfPacketAnalyzerDebugOverlappingFieldsTrafficGenerate
*           function
* @endinternal
*
* @brief   Generate traffic
*/
GT_VOID prvTgfPacketAnalyzerDebugOverlappingFieldsTrafficGenerate
(
     GT_U32         packetToSend
);

/**
* @internal GT_VOID prvTgfPacketAnalyzerDebugOverlappingFieldsConfigSet
*           function
* @endinternal
*
* @brief   Set Packet Analyzer test configuration
*/
GT_VOID prvTgfPacketAnalyzerDebugOverlappingFieldsConfigSet
(
    GT_VOID
);

/**
* @internal GT_VOID prvTgfPacketAnalyzerDebugOverlappingFieldsChangeField
*           function
* @endinternal
*
* @brief   change rule field and set Sampling Enable.
*/
GT_VOID prvTgfPacketAnalyzerDebugOverlappingFieldsChangeField
(
   GT_U32                                     numberOfFields,
   CPSS_DXCH_PACKET_ANALYZER_FIELD_VALUE_STC  fieldsValueArr[]
);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif
