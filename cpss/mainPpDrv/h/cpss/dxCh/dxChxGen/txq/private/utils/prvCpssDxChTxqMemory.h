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
* @file prvCpssDxChTxqMemory.h
*
* @brief CPSS SIP6 TXQ  memory operation functions
*
* @version   1
********************************************************************************

*/

#ifndef __prvCpssDxChTxqMemory
#define __prvCpssDxChTxqMemory

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
* @internal prvCpssSip6TxqUtilsDevClose function
* @endinternal
*
* @brief  Release dynamic memory allocation for TxQ scheduler
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   -         `                          physical device number
*
* @retval GT_OK                    -           on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*
*/
GT_STATUS prvCpssSip6TxqUtilsDevClose
(
    IN GT_U8                                devNum
);


/**
 * @internal prvCpssDxChTxqSip6PnodeAllocateAndInit function
 * @endinternal
 *       NONE
 *
 * @brief   Allocate Initialize P  Node structure
 *
 * @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman;AC3X;
 * @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin;  Bobcat3; Aldrin2.
 *
 * @param[in] pNodePtrPtr                   - pointer to pointer toP node structure
  * @param[in] numberOfAnodes           -number of A nodes to initilize
 *
 * @retval GT_OK                    - on success.
 * @retval GT_BAD_PTR          - NULL pointer used
  * @retval GT_OUT_OF_CPU_MEM          -memory allocation failure
 */
GT_STATUS  prvCpssDxChTxqSip6PnodeAllocateAndInit
(
    PRV_CPSS_DXCH_TXQ_SIP_6_P_NODE ** pNodePtrPtr,
    GT_U32                           numberOfAnodes
);

/**
 * @internal prvCpssDxChTxqSip6PnodeArrayAllocateAndInit function
 * @endinternal
 *       NONE
 *
 * @brief   Allocate Initialize P  Node array
 *
 * @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman;AC3X;
 * @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin;  Bobcat3; Aldrin2.
 *
 * @param[in] pNodePtrPtrPtr                   - pointer to pointer to pointer to P node structure
 * @param[in] numberOfAnodes           -number of A nodes to initilize
 *
 * @retval GT_OK                    - on success.
 * @retval GT_BAD_PTR          - NULL pointer used
  * @retval GT_OUT_OF_CPU_MEM          -memory allocation failure
 */
GT_STATUS  prvCpssDxChTxqSip6PnodeArrayAllocateAndInit
(
    PRV_CPSS_DXCH_TXQ_SIP_6_P_NODE *** pNodePtrPtrPtr,
    GT_U32                             arraySize
);

/**
 * @internal prvCpssDxChTxqSip6SearchArrayAllocateAndInit function
 * @endinternal
 *       NONE
 *
 * @brief   Allocate and initialize  search array
 *
 * @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman;AC3X;
 * @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin;  Bobcat3; Aldrin2.
 *
 * @param[in] pNodePtrPtrPtr                   - pointer to pointer to pointer to P node structure
 * @param[in] numberOfAnodes           -number of A nodes to initilize
 *
 * @retval GT_OK                    - on success.
 * @retval GT_BAD_PTR          - NULL pointer used
  * @retval GT_OUT_OF_CPU_MEM          -memory allocation failure
 */
GT_STATUS  prvCpssDxChTxqSip6SearchArrayAllocateAndInit
(
    GT_U32                             ** aNodeToPnodePtrPtr,
    GT_U32                             *localMappingPtr,
    GT_U32                             arraySize
);



#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvCpssDxChTxqMemory */

