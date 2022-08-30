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
* @file prvCpssDxChTxqPsi.h
*
* @brief CPSS SIP6 TXQ PSI low level configurations.
*
* @version   1
********************************************************************************
*/

#ifndef __prvCpssDxChTxqPsi
#define __prvCpssDxChTxqPsi

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
* @internal prvCpssFalconPsiMapPortToPdqQueuesSet function
* @endinternal
*
* @brief   Map port to pdq queues
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number
* @param[in] psiNum                - current PSI id(APPLICABLE RANGES:0..3).
* @param[in] pNodeIndex            - index of Pnode representing the local port
* @param[in] firstQueueInDp        - the first pdq queue of DP
* @param[in] dp                    - data path index[0..3] (APPLICABLE DEVICES: AC5P; AC5X; Harrier; Ironman).
* @param[in] localPort             - local DP port[0..26] (APPLICABLE DEVICES: AC5P; AC5X; Harrier; Ironman).
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong pdx number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Currently only one tile is supported
*
*/
GT_STATUS prvCpssFalconPsiMapPortToPdqQueuesSet
(
    IN  GT_U8  devNum,
    IN  GT_U32 psiNum,
    IN  GT_U32 pNodeIndex,
    IN  GT_U32 firstQueueInDp,
    IN  GT_U32 dp,
    IN  GT_U32 localPort
);
/**
* @internal prvCpssFalconPsiMapPortToPdqQueuesGet function
* @endinternal
*
* @brief   Read sdq to pdq queues mapping
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                -  device number
* @param[in] psiNum                -  current PSI id(APPLICABLE RANGES:0..3).
* @param[in] sdqNum                -  SDQ that being mapped number (APPLICABLE RANGES:0..7).
* @param[in] localPortNum          -  local port number (APPLICABLE RANGES:0..8).
*
* @param[out] pdqLowQueueNumPtr    - (pointer to)the first pdq queue
* @param[out] dpPtr                - (pointer to)data path index[0..3] (APPLICABLE DEVICES: AC5P; AC5X; Harrier; Ironman).
* @param[out] localPortPtr         - (pointer to)local DP port[0..26] (APPLICABLE DEVICES: AC5P; AC5X; Harrier; Ironman).
* @param[out] validPtr             - (pointer to) GT_TRUE: the pNode is mapped to a queue
*                                                 GT_False: the pNode is NOT mapped to a queue
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong pdx number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS prvCpssFalconPsiMapPortToPdqQueuesGet
(
    IN   GT_U8  devNum,
    IN   GT_U32 psiNum,
    IN   GT_U32 sdqNum,
    IN   GT_U32 localPortNum,
    OUT  GT_U32 *pdqLowQueueNumPtr,
    OUT  GT_U32 *dpPtr,
    OUT  GT_U32 *localPortPtr,
    OUT  GT_U32 *validPtr
);


/**
* @internal prvCpssFalconPsiCreditValueSet function
* @endinternal
*
* @brief   Set credit value that is being given to SDQ from PDQ
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number
* @param[in] psiNum                - current PSI id(APPLICABLE RANGES:0..3).
* @param[in] creditValue           - credit value
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong pdx number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Currently only one tile is supported
*
*/
GT_STATUS  prvCpssFalconPsiCreditValueSet
(
    IN  GT_U8  devNum,
    IN  GT_U32 psiNum,
    IN  GT_U32 creditValue
);


/**
* @internal prvCpssSip6PsiClockGatingEnableSet function
* @endinternal
*
* @brief   Enable/disable clock gating .(reducing dynamic power dissipation, by removing the clock signal when the circuit is not in use)
*
* @note   APPLICABLE DEVICES:      AC5X;Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;Falcon; AC5P.
*
* @param[in] devNum               -  device number
* @param[in] clockGateEnable      -  Enable/disable clock gating
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong pdx number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS  prvCpssSip6PsiClockGatingEnableSet
(
    IN  GT_U8  devNum,
    IN  GT_BOOL clockGateEnable
);

/**
* @internal prvCpssSip6TxqPsiDebugInterruptDisableSet function
* @endinternal
*
* @brief   Debug function that disable iterrupt in PSI
*
* @note   APPLICABLE DEVICES: Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number
* @param[in] tileNum               - The number of tile (Applicable range 0..3)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong sdq number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssSip6TxqPsiDebugInterruptDisableSet
(
    IN GT_U8 devNum,
    IN GT_U32 tileNum
);

/**
* @internal prvCpssSip6TxqSdqDebugInterruptGet function
* @endinternal
*
* @brief   Get interrupt cause for PSI
*
* @note   APPLICABLE DEVICES: Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number
* @param[in] tileNum               - The number of tile (Applicable range 0..3)
* @param[in] sdqNum                - data path index
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong sdq number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssSip6TxqPsiDebugInterruptGet
(
    IN GT_U8  devNum,
    IN GT_U32 tileNum,
    OUT GT_U32 *interruptPtr
);

GT_U32 prvCpssSip6TxqPsiSchedErrorBmp
(
    IN GT_U8  devNum
);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssDxChTxqPsi */

