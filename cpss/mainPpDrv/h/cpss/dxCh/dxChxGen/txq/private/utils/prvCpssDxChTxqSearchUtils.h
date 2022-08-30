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

#ifndef __prvCpssDxChTxqSearchUtils
#define __prvCpssDxChTxqSearchUtils

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/**
 * @internal prvCpssSip6TxqAnodeToPnodeIndexGet function
 * @endinternal
 *
 * @brief   Find P node by A node
 *
 * @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
 * @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
 *
 * @param[in] devNum                   -PP's device number.
 * @param[in] tileNum                  - tile number to look for.(APPLICABLE RANGES:0..3)
 * @param[in] aNodeIndex                  -index of A node
 * @param[out] pNodePtr                  -(pointer to)index of A node
 *
 * @retval GT_OK                    - on success.
 * @retval GT_BAD_PARAM             - wrong tile number.
 * @retval GT_HW_ERROR              - on writing to HW error.
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 */
GT_STATUS prvCpssSip6TxqAnodeToPnodeIndexGet
(
    IN  GT_U8  devNum,
    IN  GT_U32 tileNum,
    IN  GT_U32 aNodeIndex,
    OUT GT_U32 *pNodePtr
);
/**
 * @internal prvCpssSip6TxqQnodeToAnodeIndexGet function
 * @endinternal
 *
 * @brief   Find A node by Q node.
 *
 * @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
 * @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
 *
 * @param[in] devNum                   -PP's device number.
 * @param[in] tileNum                  - tile number to look for.(APPLICABLE RANGES:0..3)
 * @param[in] aNodeIndex                  -index of A node
 * @param[out] pNodePtr                  -(pointer to)index of A node
 *
 * @retval GT_OK                    - on success.
 * @retval GT_BAD_PARAM             - wrong tile number.
 * @retval GT_HW_ERROR              - on writing to HW error.
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 */
GT_STATUS prvCpssSip6TxqQnodeToAnodeIndexGet
(
    IN  GT_U8  devNum,
    IN  GT_U32 tileNum,
    IN  GT_U32 qNodeIndex,
    OUT GT_U32 *aNodePtr
);

/**
 * @internal prvCpssSip6TxqGoQToPhysicalPortGet function
 * @endinternal
 *
 * @brief   Find physical port  by GoQ  index.
 *
 * @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
 * @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
 *
 * @param[in] devNum                   -PP's device number.
 * @param[in] tileNum                  - tile number to look for.(APPLICABLE RANGES:0..3)
 * @param[in] aNodeIndex                  -index of A node
 * @param[out] pNodePtr                  -(pointer to)index of A node
 *
 * @retval GT_OK                    - on success.
 * @retval GT_BAD_PARAM             - wrong tile number.
 * @retval GT_HW_ERROR              - on writing to HW error.
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 */
GT_STATUS prvCpssSip6TxqGoQToPhysicalPortGet
(
    IN  GT_U8  devNum,
    IN  GT_U32 tileNum,
    IN  GT_U32 goQIndex,
    OUT GT_U32 *physicalPortPtr
);

/*******************************************************************************
* prvCpssFalconTxqUtilsPhysicalNodeToAnodeMappingGet
*
* DESCRIPTION:
*      Read entry from physical port to Anode index data base
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* INPUTS:
*       devNum        -  device number
*       dmaNum       - Global DMA number(0..263).
*       tileNum         - traffic class queue on this device (0..7).
*
* OUTPUTS:
*      aNodeNumPtr   - Index of A node
*      tileNumPtr   - Index of tile
*
* RETURNS:
*       GT_OK                    - on success.
*       GT_BAD_PARAM             - wrong sdq number.
*       GT_HW_ERROR              - on writing to HW error.
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*
*
*******************************************************************************/
GT_STATUS prvCpssFalconTxqUtilsPhysicalNodeToAnodeMappingGet
(
    IN GT_U8 devNum,
    IN GT_U32 physicalPortNumber,
    OUT GT_U32   * tileNumPtr,
    OUT GT_U32   * aNodeNumPtr
);



/**
 * @internal prvCpssFalconTxqUtilsFindAnodeByPhysicalPortNumber function
 * @endinternal
 *
 * @brief   Find A node by physical port number
 *
 * @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
 * @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
 *
 * @param[in] devNum                   -PP's device number.
 * @param[in] tileNum                  - tile number to look for.(APPLICABLE RANGES:0..3)
 * @param[in] portNum                  - physical port number
 *
 * @retval GT_OK                    - on success.
 * @retval GT_BAD_PARAM             - wrong tile number.
 * @retval GT_HW_ERROR              - on writing to HW error.
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 */
GT_STATUS prvCpssFalconTxqUtilsFindAnodeByPhysicalPortNumber
(
    IN GT_U8 devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    OUT PRV_CPSS_DXCH_TXQ_SIP_6_A_NODE ** outPtr
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvCpssDxChTxqSearchUtils */

