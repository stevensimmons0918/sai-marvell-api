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
* @file prvTgfPacketAnalyzerDebugUcTraffic.h
*
* @brief Test Packet Analyzer functionality when sending UC trafffic
*
* @version   1
********************************************************************************
*/
#ifndef __prvTgfPacketAnalyzerDebugUcTraffich
#define __prvTgfPacketAnalyzerDebugUcTraffich

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/dxCh/dxChxGen/packetAnalyzer/cpssDxChPacketAnalyzer.h>

/* gets random MangerId */
#define PRV_CPSS_PACKET_ANALYZER_GET_MANAGER_ID_MAC(managerId)        \
        managerId = (prvUtfSeedFromStreamNameGet() % PRV_CPSS_DXCH_PACKET_ANALYZER_MAX_MANAGERS_NUM_CNS) + 1;


/**
* @internal GT_VOID prvTgfPacketAnalyzerUnknownUcTrafficGenerate
*           function
* @endinternal
*
* @brief   Generate traffic
*/
GT_VOID prvTgfPacketAnalyzerUnknownUcTrafficGenerate
(
     GT_BOOL invalidateVlan,
     GT_BOOL useCustomPort,
     GT_U32  customPortIndex
);

/**
* @internal GT_VOID prvTgfPacketAnalyzerUnknownUcRestore
*           function
* @endinternal
*
* @brief   Restore Packet Analyzer test Configurations
*/
GT_VOID prvTgfPacketAnalyzerUnknownUcRestore
(
    GT_BOOL restoreVlan
);

GT_VOID prvTgfPacketAnalyzerGeneralConfigSet
(
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT             *stagesArr,
    CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT                     *fieldsArr,
    CPSS_DXCH_PACKET_ANALYZER_FIELD_VALUE_STC               *fieldsValueArr,
    GT_U32                                                  *numOfStagesPtr,
    GT_U32                                                   numOfFields,
    GT_CHAR_PTR                                              ruleName
);

GT_VOID prvTgfPacketAnalyzerCreateLogicalStagesArrWithFields
(
    OUT GT_U32                                                  *numOfStagesOut,
    OUT CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT             *stagesArrOut
);

GT_VOID prvTgfPacketAnalyzerGeneralResultsGet
(

    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT              *matchedStagesArr,
    GT_U32                                                   numOfMatchedStages,
    GT_U32                                                   numOfFields,
    GT_U32                                                   *expectedNumOfHits,
    GT_U32                                                   *expectedNumOfSampleFields,
    CPSS_DXCH_PACKET_ANALYZER_FIELD_VALUE_STC                *expectedSampleFieldsValueArr[]
);

GT_VOID  prvTgfPacketAnalyzerLogicalKeyFieldsPerStageGet
(
    IN  CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT         stageId,
    INOUT GT_U32                                            *numOfFieldsPtr,
    OUT   CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT               fieldsArr[]
);

GT_VOID  prvTgfPacketAnalyzerEgressAggregatorCatchPort
(
    GT_U32 targetPortIndex
);

GT_STATUS  prvTgfPacketAnalyzerTestAddExpectedResultStage
(
    IN CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT *matchedStagesArr,
    IN GT_U32 index,
    IN CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT stageId,
    IN GT_U32 totalStages
);



GT_STATUS  prvTgfPacketAnalyzerTestAddExpectedResultFieldsToStage
(
    IN GT_U32 *numOfHits,
    IN GT_U32 *numOfSampleFields,
    IN GT_U32 index,
    IN GT_U32 expectedHits,
    IN GT_U32 expectedHitFieldsAmount,
    IN GT_U32 totalStages
);

GT_VOID  prvTgfPacketAnalyzerIngressAggregatorCatchPort
(
    GT_U32 sendPortIndex
);

GT_VOID  prvTgfPacketAnalyzerPreQTest
(
    GT_U32 byteCount,
    GT_U32 expectedNumOfHits
);

GT_VOID  prvTgfPacketAnalyzerDebugModeEnableSet
(
    GT_BOOL enable
);

GT_BOOL  prvTgfPacketAnalyzerDebugModeEnableGet
(
    GT_VOID
);



GT_VOID  prvTgfPacketAnalyzerSip6SanityTest
(
    GT_U32 byteCount,
    GT_U32 srcDevNum,
    GT_U32 queueOffset,
    GT_U32 expectedNumOfHits
);

/**
* @internal GT_VOID prvTgfPacketAnalyzerGeneralResults2Get
*           function
* @endinternal
*
* @brief   Check Packet Analyzer test Results
*/
GT_VOID prvTgfPacketAnalyzerGeneralResults2Get
(
   IN GT_U32                                                numOfStages,
   IN CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT           stagesArr[],
   IN GT_U32                                                numOfHitPerStageArr[],
   IN GT_U32                                                numOfFieldPerStage[],
   IN CPSS_DXCH_PACKET_ANALYZER_FIELD_VALUE_STC             *fieldsValuePerStageArr[]
);

/**
* @internal GT_VOID prvTgfPacketAnalyzerGeneralResultsByAttributeGet
*           function
* @endinternal
*
* @brief   Check Packet Analyzer test Results by search Attribute for Falcon
*/
GT_VOID prvTgfPacketAnalyzerGeneralResultsByAttributeGet
(
   IN GT_U32                                                numOfStages,
   IN CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT           stagesArr[],
   IN CPSS_DXCH_PACKET_ANALYZER_SEARCH_ATTRIBUTE_STC        *searchAttributePtr,
   IN GT_U32                                                numOfHitPerStageArr[],
   IN GT_U32                                                numOfFieldPerStage[],
   IN CPSS_DXCH_PACKET_ANALYZER_FIELD_VALUE_STC             *fieldsValuePerStageArr[]
);

/**
* @internal GT_VOID prvTgfPacketAnalyzerGeneralCountersClear
*           function
* @endinternal
*
* @brief   Clear Sampling Data and Counters for packet analyzer rule.
*/
GT_VOID prvTgfPacketAnalyzerGeneralCountersClear
(
    GT_VOID
);

GT_VOID prvTgfPacketAnalyzerGeneralSwitchMuxStages
(
    IN CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT stageOut,
    IN CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT stageIn
);

/**
* @internal GT_VOID prvTgfPacketAnalyzerGeneralSetVlan
*           function
* @endinternal
*
* @brief   Replace Vlan content of packet analyzer rule.
*/
GT_VOID prvTgfPacketAnalyzerGeneralSetVlan
(
   GT_U32                 vlan ,
   GT_BOOL                setCustomVlan,
   GT_BOOL                setVlan2system
);

/**
* @internal GT_VOID prvTgfPacketAnalyzerGeneralChangeBindStatus
*           function
* @endinternal
*
* @brief   Replace bind state for stage.
*/
GT_VOID prvTgfPacketAnalyzerGeneralChangeBindStatus
(
   CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT           stage,
   GT_BOOL                                               toBind
);

/**
* @internal GT_VOID prvTgfPacketAnalyzerGeneralUpdateRule
*           function
* @endinternal
*
* @brief   change field parms in rule
*/
GT_VOID prvTgfPacketAnalyzerGeneralUpdateRule
(
    CPSS_DXCH_PACKET_ANALYZER_FIELD_VALUE_STC               *fieldsValueArr
);

/**
* @internal GT_VOID prvTgfPacketAnalyzerGeneralEnableSampling
*           function
* @endinternal
*
* @brief   enable/disable sampling
*/
GT_VOID prvTgfPacketAnalyzerGeneralEnableSampling
(
    GT_BOOL           enable
);
#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif
