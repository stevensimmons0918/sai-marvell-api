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
* @file prvTgfExactMatchTtiPclFullPath.h
*
* @brief Test Exact Match Expanded Action functionality with TTI and PCL
*        Configuration
*
* @version   1
********************************************************************************
*/
#ifndef __prvTgfExactMatchTtiPclFullPathh
#define __prvTgfExactMatchTtiPclFullPathh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <common/tgfExactMatchGen.h>


/**
* @internal prvTgfExactMatchTtiPclFullPath_TTI_GenericConfigSet function
* @endinternal
*
* @brief   Set TTI Generic configuration not related to Exact Match
*
* @param[in] firstCall            - GT_TRUE: calling this API for the first time
*                                   GT_FALSE: calling this API for the second time
*/
GT_VOID prvTgfExactMatchTtiPclFullPath_TTI_GenericConfigSet
(
    IN GT_BOOL  firstCall
);
/**
* @internal prvTgfExactMatchTtiPclFullPath_TTI_GenericConfigRestore function
* @endinternal
*
* @brief   Restore TTI Generic configuration not related to Exact Match
*/
GT_VOID prvTgfExactMatchTtiPclFullPath_TTI_GenericConfigRestore
(
    GT_VOID
);
/**
* @internal prvTgfExactMatchTtiPclFullPath_PCL_GenericConfigSet function
* @endinternal
*
* @brief   Set PCL Generic configuration not related to Exact Match
*
*/
GT_VOID prvTgfExactMatchTtiPclFullPath_PCL_GenericConfigSet
(
    GT_U32      subProfileId
);
/**
* @internal prvTgfExactMatchTtiPclFullPath_PCL_GenericConfigRestore function
* @endinternal
*
* @brief   Restore PCL Generic configuration not related to Exact Match
*/
GT_VOID prvTgfExactMatchTtiPclFullPath_PCL_GenericConfigRestore
(
    GT_VOID
);
/**
* @internal prvTgfExactMatchTtiPclFullPathInvalidateRule function
* @endinternal
*
* @brief   Invalidate PCL Rule
*/
GT_VOID prvTgfExactMatchTtiPclFullPathInvalidateRule
(
    GT_VOID
);
/**
* @internal GT_VOID prvTgfExactMatchTtiPclFullPathConfigSet function
* @endinternal
*
* @brief   Set PCL test configuration related to Exact Match Expanded Action
*
* @param[in] pktCmd   - packet command to assign in Exact Match Action
*
*/
GT_VOID prvTgfExactMatchTtiPclFullPathConfigSet
(
     CPSS_PACKET_CMD_ENT pktCmd
);

/**
* @internal GT_VOID prvTgfExactMatchTtiPclFullPathConfigSet function
* @endinternal
*
* @brief   Set PCL test configuration related to Exact Match Expanded Action
*          and Reduced Action
*
* @param[in] pktCmd   - packet command to assign in Exact Match Action
*
*/
GT_VOID prvTgfExactMatchTtiPclFullPathReducedConfigSet
(
     CPSS_PACKET_CMD_ENT pktCmd
);

/**
* @internal GT_VOID
*           prvTgfExactMatchTtiPclFullPathReducedConfigMacSaSet
*           function
* @endinternal
*
* @brief   Set PCL test configuration related to Exact Match Expanded Action
*          and Reduced Action
*
* @param[in] pktCmd   - packet command to assign in Exact Match Action
*
*/
GT_VOID prvTgfExactMatchTtiPclFullPathReducedConfigMacSaSet
(
     CPSS_PACKET_CMD_ENT pktCmd
);

/**
* @internal prvTgfExactMatchTtiPclFullPathNewKeySizeConfigSet function
* @endinternal
*
* @brief   Set Exact Match keyParams.keySize to be != 0
*
*/
GT_VOID prvTgfExactMatchTtiPclFullPathNewKeySizeConfigSet
(
     GT_VOID
);
/**
* @internal prvTgfExactMatchTtiPclFullPathNewKeyMaskConfigSet function
* @endinternal
*
* @brief   Set Exact Match keyParams.keyMask to be != 0xFF
*
*/
GT_VOID prvTgfExactMatchTtiPclFullPathNewKeyMaskConfigSet
(
     GT_VOID
);
/**
* @internal prvTgfExactMatchTtiPclFullPathUdb47ConfigSet function
* @endinternal
*
* @brief   Set Exact Match keyParams.keySize to be PRV_TGF_EXACT_MATCH_KEY_SIZE_47B_E
*
*/
GT_VOID prvTgfExactMatchTtiPclFullPathUdb47ConfigSet
(
     GT_VOID
);

/**
* @internal prvTgfExactMatchTtiPclFullPathUdbAllConfigSet
*           function
* @endinternal
*
* @brief   Set Exact Match keyParams
*
* @param[in] keySize  - key size to assign to Exact Match
*                       Profile ID
*/
GT_VOID prvTgfExactMatchTtiPclFullPathUdbAllConfigSet
(
     PRV_TGF_EXACT_MATCH_KEY_SIZE_ENT keySize
);

/**
* @internal prvTgfExactMatchTtiPclFullPathActivitySet function
* @endinternal
*
* @brief   Set Expected Activity
*
*/
GT_VOID prvTgfExactMatchTtiPclFullPathActivitySet
(
     GT_BOOL    expectedActivityHit
);
/**
* @internal GT_VOID prvTgfExactMatchTtiPclFullPathConfigRestore function
* @endinternal
*
* @brief   Restore PCL test configuration related to Exact Match Expanded Action
*/
GT_VOID prvTgfExactMatchTtiPclFullPathConfigRestore
(
    GT_VOID
);
/**
* @internal prvTgfExactMatchTtiPclFullPathInvalidateEmEntry function
* @endinternal
*
* @brief   Invalidate Exact Match Entry
*/
GT_VOID prvTgfExactMatchTtiPclFullPathInvalidateEmEntry
(
    GT_VOID
);
/**
* @internal prvTgfExactMatchTtiPclFullPathInvalidateSecondEmEntry function
* @endinternal
*
* @brief   Invalidate Second Exact Match Entry
*/
GT_VOID prvTgfExactMatchTtiPclFullPathInvalidateSecondEmEntry
(
    GT_VOID
);
/*
* @internal prvTgfExactMatchTtiPclFullPathTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic, expect traffic to be trapped/dropped/forwarded
*
* @param[in] expectNoTraffic   - GT_TRUE:  No traffic
*                                GT_FALSE: Expect Traffic
* @param[in] expectFdbForwarding - GT_TRUE:  FBB forwarding
*                                GT_FALSE: NO FBB forwarding
*/
GT_VOID prvTgfExactMatchTtiPclFullPathTrafficGenerate
(
    GT_BOOL     expectNoTraffic,
    GT_BOOL     expectFdbForwarding
);

/**
* @internal prvTgfExactMatchTtiPclFullPathUdbAllConfigRestore
*           function
* @endinternal
*
* @brief   Return to default value of mac SA
*/
GT_VOID prvTgfExactMatchTtiPclFullPathUdbAllConfigRestore();

/**
* @internal prvTgfExactMatchTtiPclFullPathMultipleTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic, expect traffic to be trapped/dropped/forwarded
*
* @param[in] expectTrapTraffic   - GT_FALSE: expect no traffic
*                                - GT_TRUE: expect traffic to be trapped
*
* @param[in] expectForwardTraffic - GT_FALSE: expect no traffic
*                                 - GT_TRUE: expect traffic to be forwarded
*
* @param[in] numberOfPacketsToSend - number of packets to send,
*                                    each with a different Dst MAC LSB
*/
GT_VOID prvTgfExactMatchTtiPclFullPathMultipleTrafficGenerate
(
    GT_BOOL     expectTrapTraffic,
    GT_BOOL     expectForwardTraffic,
    GT_U32      numberOfPacketsToSend
);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif
