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
* @file prvTgfExactMatchAutoLearnTti.h
*
* @brief Test Exact Match Expanded Action functionality with TTI Action Type
*
* @version   1
********************************************************************************
*/
#ifndef __prvTgfExactMatchAutoLearnTtih
#define __prvTgfExactMatchAutoLearnTtih

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <common/tgfExactMatchGen.h>
#include <utf/private/prvUtfExtras.h>


/**
* @internal prvTgfExactMatchAutoLearnTtiGenericConfigSet function
* @endinternal
*
* @brief   Set TTI generic test configuration, not related to Exact Match
*
* @note Used 7 first bytes of anchor 'L3 offset - 2'.
*
*/
GT_VOID prvTgfExactMatchAutoLearnTtiGenericConfigSet
(
    GT_VOID
);

/**
* @internal prvTgfExactMatchAutoLearnTtiConfigSet function
* @endinternal
*
* @brief Set TTI test configuration related to Exact Match Reduced Action
*
*/
GT_VOID prvTgfExactMatchAutoLearnTtiConfigSet
(
    GT_VOID
);

/**
* @internal prvTgfExactMatchAutoLearnTtiExactMatchPriorityConfig function
* @endinternal
*
* @brief   Set TCAM Priority over Exact Match
* @param[in] exactMatchOverTtiEn - GT_FALSE: TCAM priority over Exact Match
*                                - GT_TRUE : Exact Match priority over TCAM
*/
GT_VOID prvTgfExactMatchAutoLearnTtiExactMatchPriorityConfig
(
    GT_BOOL exactMatchOverTtiEn
);

/**
* @internal prvTgfExactMatchAutoLearnTtiTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*
* @param[in] expectForwardTraffic - GT_FALSE: expect no traffic
*                                 - GT_TRUE: expect traffic to be forwarded
* @param[in] expectTrapTraffic    - GT_FALSE: expect no traffic
*                                 - GT_TRUE: expect traffic to be trapped
* @param[in] expectCPUcode        - if expectForwardTraffic set to GT_FALSE
*                                   Compar expectCPUcode to what we got
*/
GT_VOID prvTgfExactMatchAutoLearnTtiTrafficGenerate
(
    GT_BOOL     expectForwardTraffic,
    GT_BOOL     expectTrapTraffic,
    GT_U32      expectCPUcode
);

/**
* @internal prvTgfExactMatchBasicExpandedActionGenericConfigRestore function
* @endinternal
*
* @brief   Restore TTI generic test configuration, not related to Exact Match
*/
GT_VOID prvTgfExactMatchAutoLearnTtiGenericConfigRestore
(
    GT_VOID
);

/**
* @internal prvTgfExactMatchAutoLearnTtiConfigRestore function
* @endinternal
*
* @brief   Restore TTI test configuration related to Exact Match
*          Reduced Action
*/
GT_VOID prvTgfExactMatchAutoLearnTtiConfigRestore
(
    GT_VOID
);

/**
* @internal prvTgfExactMatchAutoLearnIpfixtInit function
* @endinternal
*
* @brief  Set test configuration: Enable the Ingress Policy Engine
*/
GT_VOID prvTgfExactMatchAutoLearnIpfixtInit
(
    GT_VOID
);

/**
* @internal prvTgfExactMatchAutoLearnIpfixtRestore function
* @endinternal
*
* [in] numberOfTiggers - number of tiggers to do, each trigger
*                        free one flowId
*
* @brief  Restore test configuration for IPFIX
*/
GT_VOID prvTgfExactMatchAutoLearnIpfixtRestore
(
    GT_U32 numberOfTigger
);

/**
* @internal prvTgfExactMatchAutoLearnConfigSet function
* @endinternal
*
* @brief  Set test configuration for Auto Learn
*
* @param[in] flowIdMaxNum        - maximum number of flow-IDs
*
*/
GT_VOID prvTgfExactMatchAutoLearnConfigSet
(
    GT_U32 flowIdMaxNum
);

/**
* @internal prvTgfExactMatchAutoLearnConfigSet function
* @endinternal
*
* @brief  Set test configuration for Auto Learn
*
* @param[in] flowIdMaxNum        - maximum number of flow-IDs
*
*/
GT_VOID prvTgfExactMatchAutoLearnMaxConfigSet
(
    GT_VOID
);

/**
* @internal prvTgfExactMatchAutoLearnConfigRestore function
* @endinternal
*
* @brief  Restore test configuration for Auto Learn
*/
GT_VOID prvTgfExactMatchAutoLearnConfigRestore
(
    GT_VOID
);

/**
* @internal GT_VOID prvTgfExactMatchAutoLearnTtiPclFullPathReducedConfigSet function
* @endinternal
*
* @brief   Set PCL test configuration related to Exact Match Expanded Action
*          and Reduced Action
*
* @param[in] pktCmd   - packet command to assign in Exact Match Action
* @param[in] keySize  - keySize for profile configuration
*
*/
GT_VOID prvTgfExactMatchAutoLearnTtiPclFullPathReducedConfigSet
(
     CPSS_PACKET_CMD_ENT                pktCmd,
     PRV_TGF_EXACT_MATCH_KEY_SIZE_ENT   keySize
);
/**
* @internal GT_VOID prvTgfExactMatchAutoLearnTtiPclFullPathConfigRestore function
* @endinternal
*
* @brief   Restore PCL test configuration related to Exact Match Expanded Action
*/
GT_VOID prvTgfExactMatchAutoLearnTtiPclFullPathConfigRestore
(
    GT_VOID
);

/**
* @internal prvTgfExactMatchAutoLearnIpfixCountersCheckAndTrafficSend function
*
* @endinternal
*
* @brief
*      Sent traffic and check IPFIX counters
*
* @param[in] expectedIpfixPacketCounterValue - the expected
*       value of the ipfix packet counter
*/
GT_VOID prvTgfExactMatchAutoLearnIpfixCountersCheckAndTrafficSend
(
    GT_U32 expectedIpfixPacketCounterValue
);

/**
* @internal prvTgfExactMatchAutoLearnIpfixCountersCheckAndTrafficMultipleSend function
*
* @endinternal
*
* @brief
*      Sent traffic and check IPFIX counters
*
* @param[in] expectedIpfixPacketCounterValue - the expected
*       value of the ipfix packet counter
* @param[in] numberOfPacketsToSend - number of packets to send
*/
GT_VOID prvTgfExactMatchAutoLearnIpfixCountersCheckAndTrafficMultipleSend
(
    GT_U32  expectedIpfixPacketCounterValue,
    GT_U32  numberOfPacketsToSend
);

/**
* @internal prvTgfExactMatchAutoLearnTtiPclFullPathInvalidateEmEntry function
* @endinternal
*
* @brief   Invalidate the Auto Learned Exact Match Entry
*
* @param[in] numberOfEntriesToInvalidate - number of EM entries to delete
*/
GT_VOID prvTgfExactMatchAutoLearnTtiPclFullPathInvalidateEmEntry
(
    GT_U32 numberOfEntriesToInvalidate
);

/**
*   @internal prvTgfExactMatchAutoLearnTtiPclFullPathCheckFailCounter function
*
* @endinternal
*
* @brief   Counts the number of flows EM didn't learn due to
*          lack of availables flowID's
*
* @param[in] expectedFlowIdFailCounterValue - number of EM entries didn't learn due to lack of flowIDs
* @param[in] expectedIndexFailCounterValue - number of EM entries didn't learn due to lack of free entries
*/
GT_VOID prvTgfExactMatchAutoLearnTtiPclFullPathCheckFailCounter
(
    GT_U32 expectedFlowIdFailCounterValue,
    GT_U32 expectedIndexFailCounterValue
);
/**
*   @internal prvTgfExactMatchAutoLearnTtiPclFullPathCheckCollision function
*
* @endinternal
*
* @brief   Gets the Exact Match Auto Learn collision key parameters
*
*/
GT_VOID prvTgfExactMatchAutoLearnTtiPclFullPathCheckCollision
(
    GT_VOID
);
/**
*   @internal prvTgfExactMatchAutoLearnAddStaticEntry function
*
* @endinternal
*
* @brief  Set Exact Match static entry
*
*/
GT_VOID prvTgfExactMatchAutoLearnAddStaticEntry
(
    GT_VOID
);
/**
* @internal prvTgfExactMatchAutoLearnAddStaticEntryInvalidate function
* @endinternal
*
* @brief   Invalidate Exact Match Entry
*/
GT_VOID prvTgfExactMatchAutoLearnAddStaticEntryInvalidate
(
    GT_VOID
);

/**
* @internal prvTgfExactMatchAutoLearnTtiPclFullPathCheckEventInterrupt function
* @endinternal
*
* @brief   Check/Clear Auto Learn interrupts
*
* @param[in] clearInterrupts - GT_TRUE: clear the interrupts, do not check counter
*                              GT_FALSE: Read the interrupts and check counter
* @param[in] expectedFlowAllocationFailEvents - number of "flow allocation fail" events expected
* @param[in] expectedRangeFullEvents - number of "range full" events expected
* @param[in] expectedThresholdCrossedEvents - number of "threshold crossed" events expected
* @param[in] expectedFlowIdEmptyEvents - number of "flow Id empty" events expected
* @param[in] expectedCollisionEvents - number of "auto learn Collision" events expected
*
*/
GT_VOID prvTgfExactMatchAutoLearnTtiPclFullPathCheckEventInterrupt
(
    GT_BOOL     clearInterrupts,
    GT_U32      expectedFlowAllocationFailEvents,
    GT_U32      expectedRangeFullEvents,
    GT_U32      expectedThresholdCrossedEvents,
    GT_U32      expectedFlowIdEmptyEvents,
    GT_U32      expectedCollisionEvents
);

/**
* @internal tgfExactMatchAutoLearnTtiPclFullPathRedirectToPortReducedMultipleLearningCheckPointersSenarios function
* @endinternal
*
* @param[in] maxNumber - max number of EM AutoLearn entries in DB
*
*/
GT_VOID tgfExactMatchAutoLearnTtiPclFullPathRedirectToPortReducedMultipleLearningCheckPointersSenarios
(
    GT_U32 maxNumber
);

/**
* @internal
*           tgfExactMatchAutoLearnTtiPclFullPathRedirectToPortReducedMultipleLearningCheckPointersSenarios2
*           function
* @endinternal
*
* @param[in] maxNumber - max number of EM AutoLearn entries in DB
*
*/
GT_VOID tgfExactMatchAutoLearnTtiPclFullPathRedirectToPortReducedMultipleLearningCheckPointersSenarios2
(
    GT_U32 maxNumber
);

/**
* @internal
*           tgfExactMatchAutoLearnTtiPclFullPathRedirectToPortReducedMultipleLearningCheckPointersSenarios3
*           function
* @endinternal
*
* @param[in] maxNumber - max number of EM AutoLearn entries in DB
*
*/
GT_VOID tgfExactMatchAutoLearnTtiPclFullPathRedirectToPortReducedMultipleLearningCheckPointersSenarios3
(
    GT_U32 maxNumber
);

/**
*   @internal prvTgfExactMatchAutoLearnDebugPrint function
*
* @endinternal
*
* @brief  Print Exact Match Debug Info
*
*/
GT_STATUS prvTgfExactMatchAutoLearnDebugPrint(GT_VOID);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfExactMatchAutoLearnTtih */


