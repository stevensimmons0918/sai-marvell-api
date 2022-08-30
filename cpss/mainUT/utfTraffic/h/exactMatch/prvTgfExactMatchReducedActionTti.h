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
* @file prvTgfExactMatchReducedActionTti.h
*
* @brief Test Exact Match Expanded Action functionality with TTI Action Type
*
* @version   1
********************************************************************************
*/
#ifndef __prvTgfExactMatchReducedActionTtih
#define __prvTgfExactMatchReducedActionTtih

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <common/tgfExactMatchGen.h>
#include <utf/private/prvUtfExtras.h>


/**
* @internal prvTgfExactMatchReducedActionTtiGenericConfigSet function
* @endinternal
*
* @brief   Set TTI generic test configuration, not related to Exact Match
*
* @note Used 7 first bytes of anchor 'L3 offset - 2'.
*
*/
GT_VOID prvTgfExactMatchReducedActionTtiGenericConfigSet
(
    GT_VOID
);

/**
* @internal prvTgfExactMatchReducedActionTtiConfigSet function
* @endinternal
*
* @brief Set TTI test configuration related to Exact Match Reduced Action
* @param[in] flowIdMaskEnable   - enable masking of flowId
*                                 GT_TRUE  - flowId is taken from Expabder and Reduced action
*                                 GT_FALSE - flowId is fully taken from the Expander
*/
GT_VOID prvTgfExactMatchReducedActionTtiConfigSet
(
    GT_BOOL flowIdMaskEnable
);

/**
* @internal prvTgfExactMatchReducedActionTtiExactMatchPriorityConfig function
* @endinternal
*
* @brief   Set TCAM Priority over Exact Match
* @param[in] exactMatchOverTtiEn - GT_FALSE: TCAM priority over Exact Match
*                                - GT_TRUE : Exact Match priority over TCAM
*/
GT_VOID prvTgfExactMatchReducedActionTtiExactMatchPriorityConfig
(
    GT_BOOL exactMatchOverTtiEn
);


/**
* @internal prvTgfExactMatchReducedActionTtiReducedActionSet function
* @endinternal
*
* @brief Set to Reduced Action diffrent vaule from expanded Action
*        -set command PRV_TGF_PACKET_CMD_TRAP_TO_CPU_E
*        -set userDefinedCpuCode to 505
*
* @param[in] flowIdMaskEnable   - enable masking of flowId
*                                 GT_TRUE  - flowId is taken from Expabder and Reduced action
*                                 GT_FALSE - flowId is fully
*                                 taken from the Expander
*/
GT_VOID prvTgfExactMatchReducedActionTtiReducedActionSet
(
    GT_BOOL flowIdMaskEnable
);

/**
* @internal prvTgfExactMatchReducedActionTtiTrafficGenerate function
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
* @param[in] flowIdMaskEnable     - enable masking of flowId
*                                 GT_TRUE  - flowId is taken from Expabder and Reduced action
*                                 GT_FALSE - flowId is fully taken from the Expander*
*/
GT_VOID prvTgfExactMatchReducedActionTtiTrafficGenerate
(
    GT_BOOL     expectForwardTraffic,
    GT_BOOL     expectTrapTraffic,
    GT_U32      expectCPUcode,
    GT_BOOL     flowIdMaskEnable
);

/**
* @internal prvTgfExactMatchBasicExpandedActionGenericConfigRestore function
* @endinternal
*
* @brief   Restore TTI generic test configuration, not related to Exact Match
*/
GT_VOID prvTgfExactMatchReducedActionTtiGenericConfigRestore
(
    GT_VOID
);

/**
* @internal prvTgfExactMatchReducedActionTtiConfigRestore function
* @endinternal
*
* @brief   Restore TTI test configuration related to Exact Match
*          Reduced Action
*/
GT_VOID prvTgfExactMatchReducedActionTtiConfigRestore
(
    GT_VOID
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfExactMatchReducedActionTtih */


