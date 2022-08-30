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
* @file prvCpssDxChLpmTcamMgm.h
*
* @brief This file includes Rules alloction manegment for TCAM IP routing
*
* @version   17
********************************************************************************
*/

#ifndef __prvCpssDxChLpmTcamMgmh
#define __prvCpssDxChLpmTcamMgmh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/cpssTypes.h>
#include <cpss/dxCh/dxChxGen/ipLpmEngine/cpssDxChIpLpmTypes.h>
#include <cpss/dxCh/dxChxGen/private/lpm/tcam/prvCpssDxChLpmTcamTypes.h>

struct PRV_CPSS_DXCH_LPM_TCAM_PAT_TRIE_NODE_STCT;
/*
 * typedef: struct PRV_CPSS_DXCH_LPM_TCAM_RULE_NODE_STC
 *
 * Description:
 *      Tcam Routing Rule Node.
 *
 * Fields:
 *      valid - wheather this rule is a valid one.
 *      offset - the rule offset in the TCAM.
 *      trieNode - the trie node that points to this node (the prefix)
 *
 *      redirectOffset - for policy-based routing, free redirect index in routing
 *                       lookup table.
 * Comments:
 *
 */
typedef struct PRV_CPSS_DXCH_LPM_TCAM_RULE_NODE_STCT
{
    GT_BOOL                                           valid;
    GT_U32                                            offset;
    struct PRV_CPSS_DXCH_LPM_TCAM_PAT_TRIE_NODE_STCT  *trieNode;

    GT_U32                                      redirectOffset;
} PRV_CPSS_DXCH_LPM_TCAM_RULE_NODE_STC;



/*
 * typedef: struct PRV_CPSS_DXCH_LPM_TCAM_TOKEN_NODE_STCT
 *
 * Description:
 *      TCAM Routing Rule Node.
 *
 * Fields:
 *      tcamManagerEntryToken - token repersenting allocated entry.
 *      trieNode              - the trie node that points to this node (the prefix)
 * Comments:
 *
 */
typedef struct PRV_CPSS_DXCH_LPM_TCAM_TOKEN_NODE_STCT
{
    GT_UINTPTR                                           tcamManagerEntryToken;
    struct PRV_CPSS_DXCH_LPM_TCAM_PAT_TRIE_NODE_STCT     *trieNode;
} PRV_CPSS_DXCH_LPM_TCAM_TOKEN_NODE_STC;
/**
* @enum PRV_CPSS_DXCH_LPM_TCAM_MGM_ALLOC_METHOD_ENT
 *
 * @brief TCAM Managment allocation method.
*/
typedef enum{

    /** allocate minimal index. */
    PRV_CPSS_DXCH_LPM_TCAM_MGM_ALLOC_MIN_E,

    /** allocate near middle index. */
    PRV_CPSS_DXCH_LPM_TCAM_MGM_ALLOC_MID_E,

    /** allocate maximal index. */
    PRV_CPSS_DXCH_LPM_TCAM_MGM_ALLOC_MAX_E

} PRV_CPSS_DXCH_LPM_TCAM_MGM_ALLOC_METHOD_ENT;

/*******************************************************************************
* PRV_CPSS_DXCH_LPM_TCAM_MGM_POOL_INDEX_CONVERT_FUN
*
* DESCRIPTION:
*       converts the pool index to the interconnected pool index
*
* INPUTS:
*       originalIndex    - the original pool's rule index.
* OUTPUTS:
*       None.
*
* RETURNS:
*       the interconnected pool's rule index
*
* COMMENTS:
*      None.
*
*******************************************************************************/
typedef GT_U32 (*PRV_CPSS_DXCH_LPM_TCAM_MGM_POOL_INDEX_CONVERT_FUN)
(
    IN GT_U32 originalIndex
);

/*
 * typedef: struct PRV_CPSS_DXCH_LPM_TCAM_MGM_POOL_STC
 *
 * Description:
 *      TCAM Routing Pool Manager header.
 *
 * Fields:
 *      *coreIpTcamRulesAllocArray  - the Ip Tcam Rules array.
 *      coreIpNumOfIpTcamRules      - the Ip Tcam Rules array size.
 *      *coreIpTcamRulesAllocBitMap - the Ip Tcam Rules Bit map array.
 *      coreIpTcamBitMapSize        - the ip pcl rules bit map array size.
 *      coreIpTcamNumOfAlloc        - the number of allocated rules in the pool.
 *      interConnectedPoolPtr       - another interconnected pool pointer that
 *                                    shared the rules with this pool in some
 *                                    way, the way to convert the indexes is
 *                                    using the interConnectedPoolIndexConFun
 *                                    if = NULL then there is no inter connected
 *                                    pool.
 *      interConnectedPoolIndexConFun - the inter connected pool index convert
 *                                      function.
 *
 * Comments:
 *
 */
typedef struct PRV_CPSS_DXCH_LPM_TCAM_MGM_POOL_STCT
{
    PRV_CPSS_DXCH_LPM_TCAM_RULE_NODE_STC              *coreIpTcamRulesAllocArray;
    GT_U32                                            coreIpNumOfIpTcamRules;
    GT_U32                                            *coreIpTcamRulesAllocBitMap;
    GT_U32                                            coreIpTcamBitMapSize;
    GT_U32                                            coreIpTcamNumOfAlloc;
    struct PRV_CPSS_DXCH_LPM_TCAM_MGM_POOL_STCT       **interConnectedPoolPtr;
    PRV_CPSS_DXCH_LPM_TCAM_MGM_POOL_INDEX_CONVERT_FUN interConnectedPoolIndexConFun;
}PRV_CPSS_DXCH_LPM_TCAM_MGM_POOL_STC;

#define PRV_CPSS_DXCH_LPM_TCAM_MGM_POOL_NUM_OF_PROTOCOLS_CNS    2

#define PRV_CPSS_DXCH_LPM_TCAM_MGM_POOL_ROUTER_TCAM_INDEX_CNS   0
#define PRV_CPSS_DXCH_LPM_TCAM_MGM_POOL_PCL_TCAM_INDEX_CNS      1
#define PRV_CPSS_DXCH_LPM_TCAM_MGM_POOL_NUM_OF_TCAMS_CNS        2

/*
 * typedef: struct PRV_CPSS_DXCH_LPM_TCAM_MGM_MULTIPLE_POOLS_STCT
 *
 * Description:
 *      TCAM Pool Manager header for several tcams, per stack protocol and
 *      per tcam (pcl, router)
 *
 * Fields:
 *      ipTcamPool - IP Tcam Pool
 *
 * Comments:
 *
 */
typedef struct PRV_CPSS_DXCH_LPM_TCAM_MGM_MULTIPLE_POOLS_STCT
{
    PRV_CPSS_DXCH_LPM_TCAM_MGM_POOL_STC *tcamMgm[PRV_CPSS_DXCH_LPM_TCAM_MGM_POOL_NUM_OF_TCAMS_CNS][PRV_CPSS_DXCH_LPM_TCAM_MGM_POOL_NUM_OF_PROTOCOLS_CNS];
}PRV_CPSS_DXCH_LPM_TCAM_MGM_MULTIPLE_POOLS_STC;


/*
 * typedef: struct PRV_CPSS_DXCH_LPM_TCAM_MGM_POOL_STC_IDX_SEG_STC
 *
 * Description:
 *      TCAM Routing Pool Manager Index Segment.
 *
 * Fields:
 *      startIndex      - the segment's first index.
 *      endIndex        - the segment's last index.
 *      numOfIndexes    - number of indexes in range, since range can be empty
 *
 * Comments:
 *
 */
typedef struct PRV_CPSS_DXCH_LPM_TCAM_MGM_POOL_STC_IDX_SEG_STCT
{
    GT_U32                  startIndex;
    GT_U32                  endIndex;
    GT_U32                  numOfIndexes;
} PRV_CPSS_DXCH_LPM_TCAM_MGM_POOL_STC_IDX_SEG_STC;

/*
 * typedef: struct PRV_CPSS_DXCH_LPM_TCAM_MGM_POOL_ROUTER_IDX_SEG_STC
 *
 * Description:
 *      TCAM Routing Pool Manager Index Segment for 5 columns in Router Tcam.
 *
 * Fields:
 *      segment - represent continuous block of indexes in column.
 *
 * Comments:
 *
 */
typedef struct PRV_CPSS_DXCH_LPM_TCAM_MGM_POOL_ROUTER_IDX_SEG_STCT
{
    PRV_CPSS_DXCH_LPM_TCAM_MGM_POOL_STC_IDX_SEG_STC segment[5];
} PRV_CPSS_DXCH_LPM_TCAM_MGM_POOL_ROUTER_IDX_SEG_STC;

/**
* @internal prvCpssDxChLpmTcamMgmGetRuleIdx function
* @endinternal
*
* @brief   returns the rule index in the rules array
*
* @note   APPLICABLE DEVICES:      xCat3; AC5.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] pIpTcamPoolPtr           - the pool pointer
* @param[in] allocNodePtr             - the rule to get the idx for.
*
* @param[out] ruleIdxPtr               - the rule idx.
*
* @retval GT_OK                    - if success
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChLpmTcamMgmGetRuleIdx
(
    IN  PRV_CPSS_DXCH_LPM_TCAM_MGM_POOL_STC    *pIpTcamPoolPtr,
    IN  PRV_CPSS_DXCH_LPM_TCAM_RULE_NODE_STC   *allocNodePtr,
    OUT GT_32                                  *ruleIdxPtr
);

/**
* @internal prvCpssDxChLpmTcamMgmGetRuleByRuleIdx function
* @endinternal
*
* @brief   returns the rule node according to the internal rule index
*
* @note   APPLICABLE DEVICES:      xCat3; AC5.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] pIpTcamPoolPtr           - the pool pointer
* @param[in] ruleIdx                  - the rule idx.
*
* @param[out] ruleNodePtrPtr           - the rule node of this idx.
*
* @retval GT_OK                    - if success
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChLpmTcamMgmGetRuleByRuleIdx
(
    IN  PRV_CPSS_DXCH_LPM_TCAM_MGM_POOL_STC    *pIpTcamPoolPtr,
    IN  GT_32                                  ruleIdx,
    OUT PRV_CPSS_DXCH_LPM_TCAM_RULE_NODE_STC   **ruleNodePtrPtr
);

/*******************************************************************************
* prvCpssDxChLpmTcamMgmCreate
*
* DESCRIPTION:
*       Creates a new IP TCAM rules pool manager
*
* APPLICABLE DEVICES:       xCat3; AC5.
* NOT APPLICABLE DEVICES:   None.
*
* INPUTS:
*       numOfIpTcamRules            - the number of ip Tcam rules.
*       segmentsArrayPtr            - the array of index segments that
*                                     represents the physical indexes of the
*                                     rules in this pool.
*       numOfSegments               - the number of segments
*       redirectSegmentsArrayPtr    - If != NULL, redirect info for policy-based
*                                     routing. Must be the same size as segmentsArray.
*       numOfRedirectSegments       - the number of redirect segments.
*       interConnectedPoolPtrPtr    - another interconnected pool pointer that
*                                     shared the rules with this pool in some
*                                     way, the way to convert the indexes is
*                                     using the interConnectedPoolIndexConFun
*                                     if = NULL then there is no inter connected
*                                     pool.
*       interConnectedPoolIndexConFun - the inter connected pool index convert
*                                       function.
*
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       the pool pointer if succeeded,
*       NULL otherwise.
*
* COMMENTS:
*       numOfIpTcamRules != 0!
*
*******************************************************************************/
PRV_CPSS_DXCH_LPM_TCAM_MGM_POOL_STC* prvCpssDxChLpmTcamMgmCreate
(
    IN GT_U32                                               numOfIpTcamRules,
    IN PRV_CPSS_DXCH_LPM_TCAM_MGM_POOL_STC_IDX_SEG_STC      *segmentsArrayPtr,
    IN GT_U32                                               numOfSegments,
    IN PRV_CPSS_DXCH_LPM_TCAM_MGM_POOL_STC_IDX_SEG_STC      *redirectSegmentsArrayPtr,
    IN GT_U32                                               numOfRedirectSegments,
    IN PRV_CPSS_DXCH_LPM_TCAM_MGM_POOL_STC                  **interConnectedPoolPtrPtr,
    IN PRV_CPSS_DXCH_LPM_TCAM_MGM_POOL_INDEX_CONVERT_FUN    interConnectedPoolIndexConFun
);

/**
* @internal prvCpssDxChLpmTcamMgmDestroy function
* @endinternal
*
* @brief   Destroys a IP Tcam rules pool manager
*
* @note   APPLICABLE DEVICES:      xCat3; AC5.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] pIpTcamPoolPtr           - the pool pointer
*
* @retval GT_OK                    - if susccessed
* @retval GT_FAIL                  - otherwise
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChLpmTcamMgmDestroy
(
    IN PRV_CPSS_DXCH_LPM_TCAM_MGM_POOL_STC    *pIpTcamPoolPtr
);

/**
* @internal prvCpssDxChLpmTcamMgmAlloc function
* @endinternal
*
* @brief   allocates a rule of the pool by the method given
*
* @note   APPLICABLE DEVICES:      xCat3; AC5.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] pIpTcamPoolPtr           - the pool pointer
* @param[in] lowerBoundPtr            - the lower bound rule to allocate between.
*                                      if = NULL the use the lowest index.
* @param[in] upperBoundPtr            - the upper bound rule to allocate between.
*                                      if = NULL the use the max index.
* @param[in] allocMethod              - the allocation method in which to allocate
*
* @param[out] allocNodePtrPtr          - the allocated rule node.
*                                       GT_OK if allocation succedded.
*
* @retval GT_OUT_OF_PP_MEM         - if there is no space left between the upper and lower
*                                       bounds.
* @retval GT_OUT_OF_RANGE          - if the indexes were out of range of the pool.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChLpmTcamMgmAlloc
(
    IN PRV_CPSS_DXCH_LPM_TCAM_MGM_POOL_STC         *pIpTcamPoolPtr,
    IN PRV_CPSS_DXCH_LPM_TCAM_RULE_NODE_STC        *lowerBoundPtr,
    IN PRV_CPSS_DXCH_LPM_TCAM_RULE_NODE_STC        *upperBoundPtr,
    IN PRV_CPSS_DXCH_LPM_TCAM_MGM_ALLOC_METHOD_ENT allocMethod,
    OUT PRV_CPSS_DXCH_LPM_TCAM_RULE_NODE_STC       **allocNodePtrPtr
);

/**
* @internal prvCpssDxChLpmTcamMgmAllocByGivenRule function
* @endinternal
*
* @brief   allocates a given rule.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] pIpTcamPoolPtr           - the pool pointer
* @param[in] allocRulePtr             - the rule to allocate .
*
* @param[out] allocNodePtrPtr          - the allocated rule node.
*                                       GT_OK if allocation succedded.
*
* @retval GT_ALREADY_EXIST         - if the rule is already allocated.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChLpmTcamMgmAllocByGivenRule
(
    IN PRV_CPSS_DXCH_LPM_TCAM_MGM_POOL_STC    *pIpTcamPoolPtr,
    IN PRV_CPSS_DXCH_LPM_TCAM_RULE_NODE_STC   *allocRulePtr,
    OUT PRV_CPSS_DXCH_LPM_TCAM_RULE_NODE_STC  **allocNodePtrPtr
);

/**
* @internal prvCpssDxChLpmTcamMgmFree function
* @endinternal
*
* @brief   frees a rule.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] pIpTcamPoolPtr           - the pool pointer
* @param[in] allocNodePtr             - the rule to free.
*                                       GT_OK if allocation succedded.
*
* @retval GT_NOT_FOUND             - if the rule wasn't found.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChLpmTcamMgmFree
(
    IN PRV_CPSS_DXCH_LPM_TCAM_MGM_POOL_STC    *pIpTcamPoolPtr,
    IN PRV_CPSS_DXCH_LPM_TCAM_RULE_NODE_STC   *allocNodePtr
);

/*******************************************************************************
* prvCpssDxChLpmTcamMgmSetRealIdxAsAlloc
*
* DESCRIPTION:
*       sets a real hw index as allocated.
*
* APPLICABLE DEVICES:       xCat3; AC5.
* NOT APPLICABLE DEVICES:   None.
*
* INPUTS:
*       pIpTcamPoolPtr   - the pool pointer
*       hwIndex          - the hwRule index.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       NULL - if the rule is already allocated or it's not in the pool
*       or the rule pointer, if we managed to allocate it.
*
*
* COMMENTS:
*       None.
*
*******************************************************************************/
PRV_CPSS_DXCH_LPM_TCAM_RULE_NODE_STC *prvCpssDxChLpmTcamMgmSetRealIdxAsAlloc
(
    IN PRV_CPSS_DXCH_LPM_TCAM_MGM_POOL_STC    *pIpTcamPoolPtr,
    IN GT_U32                                 hwIndex
);

/**
* @internal prvCpssDxChLpmTcamMgmCheckAvailbleMem function
* @endinternal
*
* @brief   Checks whether this poll has the availble memory
*
* @note   APPLICABLE DEVICES:      xCat3; AC5.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] pIpTcamPoolPtr           - the pool pointer
* @param[in] numOfRulesNeeded         - number of tcam rules needed
*                                       GT_OK if there is room
*                                       GT_OUT_OF_PP_MEM otherwise
*
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note numOfIpTcamRules != 0!
*
*/
GT_STATUS prvCpssDxChLpmTcamMgmCheckAvailbleMem
(
    IN PRV_CPSS_DXCH_LPM_TCAM_MGM_POOL_STC  *pIpTcamPoolPtr,
    IN GT_U32                               numOfRulesNeeded
);

/**
* @internal prvCpssDxChLpmTcamSetRuleData function
* @endinternal
*
* @brief   set a rule data by rule index, only for valid rules!
*
* @note   APPLICABLE DEVICES:      xCat3; AC5.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] pIpTcamPoolPtr           - the pool pointer
* @param[in] ruleIdx                  - the rule index (pce num)
* @param[in] ruleData                 - the data to set for that rule
*                                       GT_OK if allocation succedded.
*
* @retval GT_ERROR                 - for trying to set an invalid rule.
* @retval GT_OUT_OF_RANGE          - if the index is out of range of the pool.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChLpmTcamSetRuleData
(
    IN PRV_CPSS_DXCH_LPM_TCAM_MGM_POOL_STC       *pIpTcamPoolPtr,
    IN GT_U32                                    ruleIdx,
    IN GT_PTR                                    ruleData
);

/**
* @internal prvCpssDxChLpmTcamCompRulesPos function
* @endinternal
*
* @brief   compares two rules in terms of position
*
* @note   APPLICABLE DEVICES:      xCat3; AC5.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] allocNode1Ptr            - the first rule to compare.
* @param[in] allocNode2Ptr            - the secone rule to compare.
*                                       comapre result GT_EQUAL , GT_GREATER , or GT_SMALLER.
*/
GT_COMP_RES prvCpssDxChLpmTcamCompRulesPos
(
    IN PRV_CPSS_DXCH_LPM_TCAM_RULE_NODE_STC   *allocNode1Ptr,
    IN PRV_CPSS_DXCH_LPM_TCAM_RULE_NODE_STC   *allocNode2Ptr
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvCpssDxChLpmTcamMgmh */


