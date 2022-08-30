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
* @file prvCpssDxChLpmRamDbg.h
*
* @brief Private CPSS DXCH LPM RAM debug functions
*
* @version   2
********************************************************************************
*/
#ifndef __prvCpssDxChLpmRamDbgh
#define __prvCpssDxChLpmRamDbgh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/cpssTypes.h>
#include <cpss/generic/cpssCommonDefs.h>
#include <cpss/dxCh/dxChxGen/private/lpm/ram/prvCpssDxChLpmRamTypes.h>

/**
* @internal prvCpssDxChLpmRamDbgDump function
* @endinternal
*
* @brief   This function is intended to do hardware LPM dump
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5.
*
* @param[in] devNum                   - physical device number.
* @param[in] vrId                     - virtual router ID
* @param[in] protocol                 - the protocol
* @param[in] prefixType               - the prefix type (Unicast or Multicast)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong devNum or vrId.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChLpmRamDbgDump
(
    IN  GT_U8                                   devNum,
    IN  GT_U32                                  vrId,
    IN  PRV_CPSS_DXCH_LPM_PROTOCOL_STACK_ENT    protocol,
    IN  CPSS_UNICAST_MULTICAST_ENT              prefixType
);

/**
* @internal prvCpssDxChLpmRamDbgHwValidation function
* @endinternal
*
* @brief   This function is intended to do hardware LPM validation
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5.
*
* @param[in] devNum                   - physical device number.
* @param[in] vrId                     - virtual router ID
* @param[in] protocol                 - the protocol
* @param[in] prefixType               - the prefix type (Unicast or Multicast)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong devNum or vrId.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note All the checks
*       For every LPM LINE
*       Bucket type (bits 30-31) - all values valid
*       If Bucket type NH:
*       Bits (28-29) next hop type: only 3 valid values (check 4th value is not there)
*       If NH type is regular, block size must be 0 (on ecmp and qos, all block sizes are valid)
*       For all NH types, the range [NH_pointer - NH_pointer+block_size] must be within the NH table (the size of the NH table is initialized at init and can be retrieved from CPSS tables)
*       If bucket type regular bits 0-29 is next bucket pointer (index of LPM line, offset from LPM mem start)
*       Can check validity of the regular bitmap of the next bucket (see later)
*       If bucket type compress 1, bits 0-29 is next bucket pointer
*       Can check validity of compress1 bitmap of the next bucket (see later)
*       If bucket type is compress 2, bits 0-21 is next bucket point
*       Can check validity of compress 2 bitmap (and pass it 5th range as parameter)
*       Regular bitmap
*       --------------
*       For first word, sum must be 0
*       For each word in the bitmap other than first
*       Sum = previous_sum + number of bits in previous word
*       On last word, last 8 bits must be 0 (those do not count as 1124 = 264 = 256 + 8)
*       Regular bucket is for 11-255 ranges so there must be at least 11 bits != 0 (exception here for first MC source bucket)
*       Bit 0 in word 0 must be set because first range always starts in 0
*       Compressed 1 bitmap (ranges are 0 based)
*       ----------------------------------------
*       Bitmap has 4 bytes [range_4_start ; range_3_start ; range_2_start ; range_1_start]
*       Range 0 start is always 0 (so no need to add to bitmap).
*       While range x start != 0
*       Current range start > previous range start
*       After first 0, all must be 0
*       Compressed 1 must contain 2-5 ranges so range_1_start (second range) must be != 0
*       Compressed 2 bitmap (ranges are 0 based)
*       ----------------------------------------
*       Word 1: [range_4_start ; range_3_start ; range_2_start ; range_1_start]
*       Word 2: [range_9_start ; range_8_start ; range_7_start ; range_6_start]
*       range_5_start is carried from the pointer to this bucket
*       for each word:
*       each range start > previous range start
*       for word 1:
*       all ranges start must be != 0 (if one is 0, then it should be compressed 1)
*       for word 2:
*       all must be bigger than range_5_start or 0!!
*       Range_5_start must be != 0 (if 0, then should use compressed 1)
*       Tree based checks
*       -----------------
*       IPv4 UC
*       Max tree depth is 4 so from depth 4 all LPM lines must be NH bucket type
*       IPv6 UC
*       Same but depth is 16
*       IPv4 MC SRC
*       Max MC group depth is 4 and each group points to root of sources tree (with max depth of 4). So total max depth is 8.
*       IPv4 MC source can't start with compressed 2 bucket (need to check if that is true or applies only to IPv6 MC).
*       If there are sources for the group then group must be exact match and have depth of 5 and above. So level of group end can be detected and verify that root of source is not compressed 2.
*       If you reach depth 8, then all LPM lines must be pointing to NH.
*
*/
GT_STATUS prvCpssDxChLpmRamDbgHwValidation
(
    IN  GT_U8                                   devNum,
    IN  GT_U32                                  vrId,
    IN  PRV_CPSS_DXCH_LPM_PROTOCOL_STACK_ENT    protocol,
    IN  CPSS_UNICAST_MULTICAST_ENT              prefixType
);

/**
* @internal prvCpssDxChLpmRamDbgDbHwMemPrint function
* @endinternal
*
* @brief   Function Relevant mode : High Level API modes
*         This function print LPM debug information
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5.
*
* @param[in] lpmDbPtr                 - The LPM DB.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - otherwise
*
* @note none.
*
*/
GT_STATUS prvCpssDxChLpmRamDbgDbHwMemPrint
(
    IN PRV_CPSS_DXCH_LPM_RAM_SHADOWS_DB_STC  *lpmDbPtr
);

/**
* @internal prvCpssDxChLpmRamDbgShadowValidityCheck function
* @endinternal
*
* @brief   Validation function for the LPM shadow
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5.
*
* @param[in] lpmDbPtr                 - LPM DB
* @param[in] vrId                     - virtual router id, 4096 means "all vrIds"
* @param[in] protocolBitmap           - protocols bitmap
* @param[in] prefixType               - UC/MC/both prefix type
* @param[in] returnOnFailure          - GT_TRUE: the validation check stops at first failure
*                                      GT_FALSE: continue with the test on failure
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on illegal input parameter/s
* @retval GT_NOT_FOUND             - LPM DB was not found
* @retval GT_NOT_INITIALIZED       - LPM DB is not initialized
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note IPv6 MC validation is not implemented yet
*
*/
GT_STATUS prvCpssDxChLpmRamDbgShadowValidityCheck
(
    IN PRV_CPSS_DXCH_LPM_RAM_SHADOWS_DB_STC     *lpmDbPtr,
    IN GT_U32                                   vrId,
    IN PRV_CPSS_DXCH_LPM_PROTOCOL_BMP           protocolBitmap,
    IN CPSS_UNICAST_MULTICAST_ENT               prefixType,
    IN GT_BOOL                                  returnOnFailure
);

/**
* @internal prvCpssDxChLpmRamDbgHwShadowSyncValidityCheck function
* @endinternal
*
* @brief   This function validates synchronization between the SW and HW of the LPM
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5.
*
* @param[in] devNum                   - The device number
* @param[in] lpmDbPtr                 - LPM DB
* @param[in] vrId                     - virtual router id, 256 means "all vrIds"
* @param[in] protocolBitmap           - protocols bitmap
* @param[in] prefixType               - UC/MC/both prefix type
* @param[in] returnOnFailure          - GT_TRUE: the validation check stops at first failure
*                                      GT_FALSE: continue with the test on failure
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on illegal input parameter/s
* @retval GT_NOT_FOUND             - LPM DB was not found
* @retval GT_NOT_INITIALIZED       - LPM DB is not initialized
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChLpmRamDbgHwShadowSyncValidityCheck
(
    IN GT_U8                                    devNum,
    IN PRV_CPSS_DXCH_LPM_RAM_SHADOWS_DB_STC     *lpmDbPtr,
    IN GT_U32                                   vrId,
    IN PRV_CPSS_DXCH_LPM_PROTOCOL_BMP           protocolBitmap,
    IN CPSS_UNICAST_MULTICAST_ENT               prefixType,
    IN GT_BOOL                                  returnOnFailure
);

/**
* @internal prvCpssDxChLpmRamDbgHwOctetsToBlockMappingInfoPrint function
* @endinternal
*
* @brief   Print Octet to Block mapping debug information for a specific LPM DB
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - otherwise
* @retval GT_NOT_FOUND             - if can't find the lpm DB
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note none.
*
*/
GT_STATUS prvCpssDxChLpmRamDbgHwOctetsToBlockMappingInfoPrint
(
    IN GT_U32                           lpmDbId
);

/**
* @internal prvCpssDxChLpmDbgHwBlockInfoPrint function
* @endinternal
*
* @brief   Print Octet per Block debug information for a specific LPM DB
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
*                                       None.
*/
GT_VOID prvCpssDxChLpmDbgHwBlockInfoPrint
(
    IN PRV_CPSS_DXCH_LPM_RAM_SHADOWS_DB_STC      *lpmDbPtr
);

/**
* @internal prvCpssDxChLpmDbgHwOctetPerBlockPrint function
* @endinternal
*
* @brief   Print Octet per Block debug information for a specific LPM DB
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
*                                       None.
*/
GT_VOID prvCpssDxChLpmDbgHwOctetPerBlockPrint
(
    IN PRV_CPSS_DXCH_LPM_RAM_SHADOWS_DB_STC      *lpmDbPtr
);

/**
* @internal prvCpssDxChIpLpmDbgLastNeededMemInfoPrint function
* @endinternal
*
* @brief   Print Needed memory information for last prefix addition
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*                                  Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2;
*
* @param[in] shadowPtr  - (pointer to ) LPM shadow db
*
*/
GT_VOID prvCpssDxChIpLpmDbgLastNeededMemInfoPrint
(
    IN    PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC  *shadowPtr
);

/**
* @internal prvCpssDxChLpmDbgHwOctetPerProtocolPrint function
* @endinternal
*
* @brief   Print Octet per Protocol debug information for a specific LPM DB
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
*                                       None.
*/
GT_VOID prvCpssDxChLpmDbgHwOctetPerProtocolPrint
(
    IN PRV_CPSS_DXCH_LPM_RAM_SHADOWS_DB_STC      *lpmDbPtr
);

/**
* @internal prvCpssDxChLpmDbgHwOctetPerProtocolPrintLpmLinesCounters function
* @endinternal
*
* @brief   Print Octet per Protocol LPM lines debug information for a specific LPM DB
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
*                                       None.
*/
GT_VOID prvCpssDxChLpmDbgHwOctetPerProtocolPrintLpmLinesCounters
(
    IN PRV_CPSS_DXCH_LPM_RAM_SHADOWS_DB_STC      *lpmDbPtr
);

GT_U32 getNumOfRangesFromHW(CPSS_DXCH_LPM_NEXT_POINTER_TYPE_ENT type,GT_U32 *hwData, GT_U32 bucketPtr);
GT_U32 getBaseAddrFromHW(CPSS_DXCH_LPM_NEXT_POINTER_TYPE_ENT type,
                         GT_U32 index,
                         GT_U32 *hwData,GT_U32 bucketPtr);
GT_STATUS compareBucket(GT_U8 devNum,GT_U32 hwAddr,GT_U32 *firstBucketDataPtr ,GT_U32 * secondBucketDataPtr,
                                   GT_U32 numOfRanges,CPSS_DXCH_LPM_NEXT_POINTER_TYPE_ENT     nodeType ,GT_BOOL ignoreAgingBit);
GT_STATUS validateBitVectorOfRegularBucket(GT_U32 *bucketDataPtr, GT_U32 hwAddr, GT_U32 basePrefix);

GT_STATUS getNumOfRangesFromHWSip6(PRV_CPSS_DXCH_LPM_PROTOCOL_STACK_ENT protocol,
                                   GT_U32 depth,
                                   PRV_CPSS_DXCH_LPM_CHILD_TYPE_ENT nodeType,
                                   GT_U32 *hwData,
                                   GT_U32 hwAddr,
                                   GT_U32 nodeChildAddressesArr[6],
                                   GT_U32 nodeTotalChildTypesArr[6][3],
                                   GT_U32 nodeTotalLinesPerTypeArr[6][3],
                                   GT_U32 nodeTotalBucketPerTypesArr[6][3],
                                   GT_U32 *numOfRangesPtr,
                                   GT_U32 *rangesPtr,
                                   GT_U32 *rangesTypePtr,
                                   GT_U32 *rangesTypeIndexPtr,
                                   GT_U32 *rangesBitLinePtr);
/**
* @internal
*           prvCpssDxChLpmRamDbgGetLeafDataSip6 function
* @endinternal
*
* @brief   get the leaf data from HW
*
* @param[in] devNum                   - The device number
* @param[in] bucketPtr                - pointer to the bucket
* @param[in] hwBucketDataArr          - array holding hw data.
*                                       in case of root this is
*                                       a null pointer
* @param[in] returnOnFailure          - GT_TRUE: the validation check stops on
*                                      first failure
*                                      GT_FALSE: continue with the test on
*                                      failure
* @param[out] nextNodeTypePtr         - pointer to the next node
*                                       entry type
* @param[out] nhAdditionalDataPtr     - pointer to a set of UC
*                                       security check enablers
*                                       and IPv6 MC scope level
* @param[out] nextBucketPointerPtr    - pointer to the next
*                                       bucket on the (G,S)
*                                       lookup
* @param[out] nhPointerPtr            - Pointer to the Next Hop
*                                       Table or the ECMP Table,
*                                       based on the Leaf Type
* @param[out] leafTypePtr             - pointer to The leaf
*                                       entry type
* @param[out] entryTypePtr            - pointer to entry type:
*                                       Leaf ot Trigger
* @param[out] lpmOverEmPriorityPtr    - pointer to the resolution
*                                       priority between LPM and
*                                       Exact Match results
* @param[out]                         - the HW address of the leaf line
* @param[out] offsetOfLeafInLine      - pointer to the offset in HW were the 20 bits for
*                                       the pointer to the next bucket on the (G,S) lookup
*                                       starts this value can be:
*                                       for a line leaf structure: 2 for Leaf0, 25 for Leaf1,
*                                       48 for Leaf2, 71 for Leaf3, 94 for Leaf4 (92-23*leafOffsetInLine)+2
*                                       for a line embedded leaf structure:
*                                       91, 68, 45  (89 - leafOffsetInLine*23)+2
*
* @retval GT_FAIL                  - on failure
* @retval GT_OK                    - on success
*/
GT_STATUS prvCpssDxChLpmRamDbgGetLeafDataSip6
(
    IN  GT_U8                                       devNum,
    IN  PRV_CPSS_DXCH_LPM_RAM_BUCKET_SHADOW_STC     *bucketPtr,
    IN  GT_U32                                      hwBucketGonAddr,
    IN  GT_U32                                      hwBucketDataArr[],
    IN  GT_U32                                      hwRangeType,
    IN  GT_U32                                      hwRangeTypeIndex,
    IN  GT_U32                                      nodeChildAddress,
    IN  GT_U32                                      nodeTotalChildTypesArr[],
    IN  GT_BOOL                                     returnOnFailure,
    OUT GT_U32                                      *nextNodeTypePtr,
    OUT GT_U32                                      *nhAdditionalDataPtr,
    OUT GT_U32                                      *nextBucketPointerPtr,
    OUT GT_U32                                      *nhPointerPtr,
    OUT GT_U32                                      *leafTypePtr,
    OUT GT_U32                                      *entryTypePtr,
    OUT GT_U32                                      *lpmOverEmPriorityPtr,
    OUT GT_U32                                      *leafLineHwAddr,
    OUT GT_U32                                      *offsetOfLeafInLine
);

/**
* @internal
*           prvCpssDxChLpmRamDbgGetSrcBucketPointerSip6 function
* @endinternal
*
* @brief   get the head of SRC tree Bucket pointer
*
* @param[in] bucketPtr   - pointer to the bucket
* @param[in] rangeIndex  - index of the range we are looking for
* @param[in] returnOnFailure          - GT_TRUE: the validation check stops on
*                                      first failure
*                                      GT_FALSE: continue with the test on
*                                      failure
*
* @param[out]rangeSrcBucketPointerPtr - the root of the SRS tree
*                                       pointer address
*
* @retval GT_FAIL                  - on failure
* @retval GT_OK                    - on success
*/
GT_STATUS prvCpssDxChLpmRamDbgGetSrcBucketPointerSip6
(
    IN  PRV_CPSS_DXCH_LPM_RAM_BUCKET_SHADOW_STC     *bucketPtr,
    IN  GT_U32                                      rangeIndex,
    OUT GT_UINTPTR                                  *rangeSrcBucketPointerPtr
);

/**
* @internal
*           prvCpssDxChLpmRamDbgBucketShadowGetNumOfTotalRangesNonRealIncluddedSip6
*           function
* @endinternal
*
* @brief  function that goes over the range list and count non
*         real ranges. relevant for regular buckets
*
* @param[in] bucketPtr            - pointer to the bucket
*
* @param[out] totalNumOfRangesPtr  - pointer to all
*                             ranges includded the hidden ones
*            totalRangesPtr       - pointer to the ranges values
*            totalRangesTypesPtr  - pointer to the ranges types
*            totalRangesIsHiddenPtr - pointer to flags for
*                                     hidden ranges
*
*
* @retval GT_FAIL                  - on failure
* @retval GT_OK                    - on success
*/
GT_STATUS prvCpssDxChLpmRamDbgBucketShadowGetNumOfTotalRangesNonRealIncluddedSip6
(
    IN  PRV_CPSS_DXCH_LPM_RAM_BUCKET_SHADOW_STC   *bucketPtr,
    OUT GT_U32                                    *totalNumOfRangesPtr,
    OUT GT_U32                                    *totalRangesPtr,
    OUT GT_U32                                    *totalRangesTypesPtr,
    OUT GT_BOOL                                   *totalRangesIsHiddenPtr
);

/* the child address calculation differs according to the child type.
   The group of child nodes are organized so that all child nodes
   of the same type are grouped together*/
GT_STATUS getFromTheGonOneNodeAddrAndSize
(
    GT_U32 gonBaseAddr,
    GT_U32 *totalChildsTypeArr,
    GT_U32 rangeType,
    GT_U32 rangeTypeIndex,
    GT_U32 *gonNodeAddrPtr,
    GT_U32 *gonNodeSizePtr
);
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvCpssDxChLpmRamDbgh */


