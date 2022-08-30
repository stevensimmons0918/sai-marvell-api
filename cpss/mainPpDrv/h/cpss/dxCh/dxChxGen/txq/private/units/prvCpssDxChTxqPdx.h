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
* @file prvCpssDxChTxqPdx.h
*
* @brief CPSS SIP6 TXQ PDX low level configurations.
*
* @version   1
********************************************************************************
*/

#ifndef __prvCpssDxChTxqPdx
#define __prvCpssDxChTxqPdx

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

 typedef struct
 {
     GT_U32 dpCoreLocalTrgPort;  /*## attribute dpCoreLocalTrgPort  */
     GT_U32 queueBase;           /*## attribute queueBase           */
     GT_U32 queuePdsIndex;       /*## attribute queuePdsIndex       */
     GT_U32 queuePdxIndex;       /*## attribute queuePdxIndex       */
 } PRV_CPSS_DXCH_FALCON_TXQ_PDX_PHYSICAL_PORT_MAP_STC;

/**
* @internal prvCpssDxChTxqFalconPdxQueueGroupMapSet function
* @endinternal
*
* @brief   Set entry at PDX DX_QGRPMAP table
 *
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - PP's device number.
* @param[in] queueGroupIndex       - queue group number , entry index
* @param[in] txPortMap             - entry to be written to HW
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong pdx number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChTxqFalconPdxQueueGroupMapSet
(
    IN GT_U8 devNum,
    IN GT_U32 queueGroupIndex,
    IN PRV_CPSS_DXCH_FALCON_TXQ_PDX_PHYSICAL_PORT_MAP_STC txPortMap
);


/**
* @internal prvCpssDxChTxqFalconPdxQueueGroupMapGet function
* @endinternal
*
* @brief  Read entry from PDX DX_QGRPMAP table
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                -PP's device number.
* @param[in] queueGroupIndex       - queue group number , entry index
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong pdx number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChTxqFalconPdxQueueGroupMapGet
(
    IN GT_U8 devNum,
    IN GT_U32 queueGroupIndex,
    OUT PRV_CPSS_DXCH_FALCON_TXQ_PDX_PHYSICAL_PORT_MAP_STC * txPortMapPtr
);

/**
* @internal prvCpssDxChTxqFalconPdxBurstFifoPdsArbitrationMapGet function
* @endinternal
*
* @brief   Read arbitration PIZZA on PDS BurstFifo at PDX
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                 - device number
* @param[in] pdxNum                 - current PDX id(APPLICABLE RANGES:0..3).
* @param[in] pdsNum                 - PDS burst fifo number (APPLICABLE RANGES:0..7).
*
* @param[out] activeSliceNumPtr     - the   number of active slices
* @param[out] activeSliceMapArr     - PIZZA configuration
* @param[out] sliceValidArr         - array that mark used slices
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong pdx number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Currently only one tile is supported
*/
GT_STATUS prvCpssDxChTxqFalconPdxBurstFifoPdsArbitrationMapGet
(
    IN GT_U8 devNum,
    IN GT_U32 pdxNum,
    IN GT_U32 pdsNum,
    OUT GT_U32* activeSliceNumPtr,
    OUT GT_U32* activeSliceMapArr,
    OUT GT_BOOL * sliceValidArr
);


/**
* @internal prvCpssDxChTxqFalconPdxInterfaceMapSet function
* @endinternal
*
* @brief   Set current PDX to another PDX/s interface mapping
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number
* @param[in] pdxNum                - current PDX id(APPLICABLE RANGES:0..3).
* @param[in] connectedPdxId        - the mapping of connected PDXs
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong pdx number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Currently only one tile is supported
*/
GT_STATUS prvCpssDxChTxqFalconPdxInterfaceMapSet
(
   IN GT_U8 devNum,
   IN GT_U32 pdxNum,
   IN GT_U32 * connectedPdxId
);

/**
* @internal prvCpssDxChTxqFalconPdxBurstFifoLoadNewPizza function
* @endinternal
*
* @brief   Triger loading of a new pizza into arbiter
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum          - device number
* @param[in] pdxNum          - current PDX id(APPLICABLE RANGES:0..3).
* @param[in] pdsNum          - DP number(APPLICABLE RANGES:0..7).
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong pdx number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note
*
*/
GT_STATUS prvCpssDxChTxqFalconPdxBurstFifoLoadNewPizza
(
    IN GT_U8 devNum,
    IN GT_U32 pdxNum,
    IN GT_U32 pdsNum
);

/**
* @internal prvCpssDxChTxqFalconPdxBurstFifoSetMaxSlice function
* @endinternal
*
* @brief   Set maximal numer of slices per DP at PDX arbiter
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum      - device number
* @param[in] pdxNum      - current PDX id(APPLICABLE RANGES:0..3).
* @param[in] pdsNum      - DP number(APPLICABLE RANGES:0..7).
* @param[in] maxSlice    - the number of slices used(APPLICABLE RANGES:8..33).
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong pdx number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS prvCpssDxChTxqFalconPdxBurstFifoSetMaxSlice
(
    IN GT_U8 devNum,
    IN GT_U32 pdxNum,
    IN GT_U32 pdsNum,
    IN GT_U32  maxSlice
);

/**
* @internal prvCpssDxChTxqFalconPdxPacPortDescCounterGet function
* @endinternal
*
* @brief   Get descriptor counter per port at PDX PAC unit
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - PP's device number.
* @param[in] pdxNum                - tile number
* @param[in] portNum               - global port num (0..71)
* @param[out ] descNumPtr          - number of descriptors
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong pdx number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Currently only one tile is supported
*
*/
GT_STATUS prvCpssDxChTxqFalconPdxPacPortDescCounterGet
(
    IN GT_U8 devNum,
    IN GT_U32 pdxNum,
    IN GT_U32 portNum,
    OUT GT_U32* descNumPtr
);

/**
* @internal prvCpssDxChTxqFalconPdxBurstFifoInit function
* @endinternal
*
* @brief  Initialize PDX burst fifo thresholds
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - PP's device number.
* @param[in] pdxNum                - tile number
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong pdx number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS prvCpssDxChTxqFalconPdxBurstFifoInit
(
   IN GT_U8 devNum,
   IN GT_U32 pdxNum
);

/**
 * @internal prvCpssDxChTxqSip6_10PdxPreemptionEnableSet function
 * @endinternal
 *
 * @brief  Enable/disable preemption at PDX glue
 *
 * @note   APPLICABLE DEVICES:      AC5P
 * @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;Falcon; AC5X; Harrier; Ironman.
 *
 * @param[in] devNum                - PP's device number.
 * @param[in] groupOfQueues         - group of queues
 * @param[in] enable                - if equal GT_TRUE then preemption is enabled in PDX,
 *                                    GT_FALSE otherwise
 *
 * @retval GT_OK                    - on success.
 * @retval GT_BAD_PARAM             - wrong pdx number.
 * @retval GT_HW_ERROR              - on writing to HW error.
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 *
 */
GT_STATUS prvCpssDxChTxqSip6_10PdxPreemptionEnableSet
(
    IN GT_U8   devNum,
    IN GT_U32  groupOfQueues,
    IN GT_BOOL enable
);


/**
 * @internal prvCpssDxChTxqSip6_10PdxPreemptionEnableSet function
 * @endinternal
 *
 * @brief  Get enable/disable preemption at PDX glue
 *
 * @note   APPLICABLE DEVICES:      AC5P
 * @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;Falcon; AC5X; Harrier; Ironman.
 *
 * @param[in] devNum                - PP's device number.
 * @param[in] groupOfQueues         - group of queues
 * @param[out] enablePtr            -(pointer to)if equal GT_TRUE then preemption is enabled in PDX,
 *                                    GT_FALSE otherwise
 *
 * @retval GT_OK                    - on success.
 * @retval GT_BAD_PARAM             - wrong pdx number.
 * @retval GT_HW_ERROR              - on writing to HW error.
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 *
 */
GT_STATUS prvCpssDxChTxqSip6_10PdxPreemptionEnableGet
(
    IN GT_U8   devNum,
    IN GT_U32  groupOfQueues,
    OUT GT_BOOL *enablePtr
);

/**
* @internal prvCpssSip6TxqPdxDebugInterruptDisableSet function
* @endinternal
*
* @brief   Debug function that disable iterrupt in PDX
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
GT_STATUS prvCpssSip6TxqPdxDebugInterruptDisableSet
(
    IN GT_U8  devNum,
    IN GT_U32 tileNum
);

/**
* @internal prvCpssSip6TxqPdxDebugInterruptGet function
* @endinternal
*
* @brief   Get interrupt cause for PDX
*
* @note   APPLICABLE DEVICES: Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number
* @param[in] tileNum               - The number of tile (Applicable range 0..3)
* @param[in] qfcNum                - data path index
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong sdq number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssSip6TxqPdxDebugInterruptGet
(
    IN GT_U8  devNum,
    IN GT_U32 tileNum,
    OUT GT_U32 *itxFunctionalPtr,
    OUT GT_U32 *itxDebugPtr,
    OUT GT_U32 *pac0FunctionalPtr,
    OUT GT_U32 *pac1FunctionalPtr
);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssDxChTxqPdx */
