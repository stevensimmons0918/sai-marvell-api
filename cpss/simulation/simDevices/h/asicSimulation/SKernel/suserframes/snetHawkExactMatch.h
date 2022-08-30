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
* @file snetHawkExactMatch.h
*
* @brief This is a external API definition for SIP6_10 Exact Match
*
* @version   1
********************************************************************************
*/
#ifndef __snetHawkExactMatchh
#define __snetHawkExactMatchh

#include <asicSimulation/SKernel/smain/smain.h>
#include <asicSimulation/SKernel/suserframes/snetCheetah3TTermination.h>
#include <asicSimulation/SKernel/suserframes/snetXCatPcl.h>
#include <asicSimulation/SKernel/suserframes/snetFalconExactMatch.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* each bank wide is 4 words */
#define SMEM_EXACT_MATCH_ENTRY_BANK_SIZE_IN_WORDS_CNS              4

/**
* @internal sip6_10_ExactMatchAutoLearnHitNumEnable function
* @endinternal
*
* @brief   return GT_TRUE incase the hitNum is enabled for Auto Learn
*
* @param[in] devObjPtr                  - (pointer to) the device object
* @param[in] hitNum                     - hit number (0,1)
*/
GT_BOOL sip6_10_ExactMatchAutoLearnHitNumEnable
(
    IN  SKERNEL_DEVICE_OBJECT                *devObjPtr,
    IN  GT_U32                                hitNum
);
/**
* @internal sip6_10_ExactMatchAutoLearnStatusCountersEnableGet function
* @endinternal
*
* @brief   return GT_TRUE if EM status counters are enabled else GT_FALSE
*
* @param[in] devObjPtr                  - (pointer to) the device object
*
*/
GT_BOOL sip6_10_ExactMatchAutoLearnStatusCountersEnableGet
(
    IN  SKERNEL_DEVICE_OBJECT                *devObjPtr
);
/**
* @internal sip6_10_ExactMatchAutoLearnProfileIndexEnable function
* @endinternal
*
* @brief   return GT_TRUE incase the profileIndex is enabled for Auto Learn
*
* @param[in] devObjPtr                  - (pointer to) the device object
* @param[in] exactMatchProfileIndex     - profile index (0,15)
*/
GT_BOOL sip6_10_ExactMatchAutoLearnProfileIndexEnable
(
    IN  SKERNEL_DEVICE_OBJECT                *devObjPtr,
    IN  GT_U32                                exactMatchProfileIndex
);

/**
* @internal sip6_10_ExactMatchAutoLearnProfileFlowIdBitOffset function
* @endinternal
*
* @brief   return the flowId bit offset for the given profileIndex
*
* @param[in] devObjPtr                  - (pointer to) the device object
* @param[in] exactMatchProfileIndex     - profile index (0,15)
*/
GT_U32 sip6_10_ExactMatchAutoLearnProfileFlowIdBitOffset
(
    IN  SKERNEL_DEVICE_OBJECT                *devObjPtr,
    IN  GT_U32                                exactMatchProfileIndex
);

/**
* @internal sip6_10_ExactMatchFreeIndex function
* @endinternal
*
* @brief   return GT_TRUE incase there is a free index in the
*          Exact match table that can fit the given key
*
* @param[in] devObjPtr                  - (pointer to) the device object
* @param[in] hitNum                     - hit number (0,1)
* @param[in] exactMatchNewKeyDataPtr    - EM-Key data
* @param[in] exactMatchNewKeySizePtr    - EM-Key size
* @param[out] freeIndexPtr              - pointer to free Index
* @param[out] bankNumPtr                - pointer to bank of the free Index
* @param[out] numberOfBanksPtr          - the number of valid banks in the system
*/
GT_BOOL sip6_10_ExactMatchFreeIndex
(
    IN  SKERNEL_DEVICE_OBJECT                *devObjPtr,
    IN  GT_U32                                hitNum,
    IN  GT_U32                               *exactMatchNewKeyDataPtr,
    IN  SIP6_EXACT_MATCH_KEY_SIZE_ENT         exactMatchNewKeySize,
    OUT GT_U32                               *freeIndexPtr,
    OUT GT_U32                               *bankNumPtr,
    OUT GT_U32                               *numberOfBanks
);

/**
* @internal sip6_10_ExactMatchBuildAutoLearnEntry function
* @endinternal
*
* @brief   return GT_TRUE if a new auto learn entry was added correctly
*
* @param[in] devObjPtr                  - (pointer to) the device object
* @param[in] hitNum                     - hit number (0,1)
* @param[in] exactMatchProfileIndex     - profile index (0,15)
* @param[in] exactMatchNewKeyDataPtr    - EM-Key data
* @param[in] exactMatchNewKeySizePtr    - EM-Key size
* @param[in] newFlowId                  - flowId value to add in the reduced entry
* @param[in] entryIndex                 - the Index for the new learned entry
*/
GT_BOOL sip6_10_ExactMatchBuildAutoLearnEntry
(
    IN  SKERNEL_DEVICE_OBJECT                   *devObjPtr,
    IN  GT_U32                                  hitNum,
    IN  GT_U32                                  exactMatchProfileIndex,
    IN  GT_U32                                  *exactMatchNewKeyDataPtr,
    IN  SIP6_EXACT_MATCH_KEY_SIZE_ENT           exactMatchNewKeySize,
    IN  GT_U32                                  newFlowId,
    IN  GT_U32                                  entryIndex
);

/**
* @internal sip6_10_ExactMatchFreeFlowId function
* @endinternal
*
* @brief   return GT_TRUE if a free flowId was found
*
* @param[in] devObjPtr                  - (pointer to) the device object
* @param[out] freeFlowIdPtr             - free flowId value
*/
GT_BOOL sip6_10_ExactMatchFreeFlowId
(
    IN  SKERNEL_DEVICE_OBJECT                   *devObjPtr,
    OUT GT_U32                                  *freeFlowIdPtr
);

/**
* @internal sip6_10_ExactMatchRecycleFlowId function
* @endinternal
*
* @brief   When bit is set by the CPU, it causes the <First Flow ID> to
*          be marked as free, and this bit is automatically cleared.
*          return GT_TRUE if recycle was successful
*
* @param[in] devObjPtr                  - (pointer to) the device object
*/
GT_BOOL sip6_10_ExactMatchRecycleFlowId
(
    IN  SKERNEL_DEVICE_OBJECT                   *devObjPtr
);

/**
* @internal sip6_10_ExactMatchAutoLearnEntryIndexSet function
* @endinternal
*
* @brief   Set the index of the exact mathc entry bind to the
*          flowId
*
* @param[in] devObjPtr              - (pointer to) the device object
* @param[in] freeFlowIdPtr          - free flowId value,index to the table
* @param[in] exactMatchIndex        - value of the learnd exact match index
* @param[in] bankNum                - value of the learnd exact match bank
*/
GT_VOID sip6_10_ExactMatchAutoLearnEntryIndexSet
(
    IN  SKERNEL_DEVICE_OBJECT                   *devObjPtr,
    IN  GT_U32                                  freeFlowId,
    IN  GT_U32                                  exactMatchIndex,
    IN  GT_U32                                  bankNum
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif  /* __snetHawkExactMatchh */

