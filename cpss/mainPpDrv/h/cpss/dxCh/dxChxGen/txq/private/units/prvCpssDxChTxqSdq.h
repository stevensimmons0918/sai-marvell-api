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
* @file prvCpssDxChTxqSdq.h
*
* @brief CPSS SIP6 TXQ SDQ low level configurations.
*
* @version   1
********************************************************************************
*/

#ifndef __prvCpssDxChTxqSdq
#define __prvCpssDxChTxqSdq


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define PRV_TH_GRANULARITY_SHIFT 8 /* 256*/
#define PRV_ELIG_PRIO_TH_GRANULARITY_SHIFT 13 /* 8K*/
#define PRV_QDRYER_SELIG_TH 0x00000800 /* 2K*/
#define PRV_QDRYER_GRANULARITY_SHIFT  9/*512*/

#define PRV_SDQ_SIP_6_20_DEV_DEP_FIELD_GET_MAC(_dev,_fieldName) \
    (PRV_CPSS_SIP_6_30_CHECK_MAC(_dev)?TXQ_IRONMAN_SDQ_##_fieldName:TXQ_HARRIER_SDQ_##_fieldName)

#define PRV_SDQ_SIP_6_15_DEV_DEP_FIELD_GET_MAC(_dev,_fieldName) \
    (PRV_CPSS_SIP_6_20_CHECK_MAC(_dev)? \
     (PRV_SDQ_SIP_6_20_DEV_DEP_FIELD_GET_MAC(_dev,_fieldName)):TXQ_PHOENIX_SDQ_##_fieldName)


#define PRV_SDQ_SIP_6_10_DEV_DEP_FIELD_GET_MAC(_dev,_fieldName) \
    (PRV_CPSS_SIP_6_15_CHECK_MAC(_dev)?\
     (PRV_SDQ_SIP_6_15_DEV_DEP_FIELD_GET_MAC(_dev,_fieldName)):TXQ_HAWK_SDQ_##_fieldName)

#define TXQ_SDQ_FIELD_GET(_dev,_fieldName) (PRV_CPSS_SIP_6_10_CHECK_MAC(_dev)?\
    (PRV_SDQ_SIP_6_10_DEV_DEP_FIELD_GET_MAC(_dev,_fieldName)):\
    (TXQ_SDQ_##_fieldName))

#define TXQ_SDQ_FIELD_GET_ADV(_dev,_fieldName) \
    PRV_SDQ_SIP_6_10_DEV_DEP_FIELD_GET_MAC(_dev,_fieldName)

/**
* @struct PRV_CPSS_DXCH_SIP6_TXQ_SDQ_PORT_ATTRIBUTES
 *
 * @brief SDQ port thresholds.Controls when the credits are requested for port from PDQ
*/
typedef struct
{
    /** @brief  Port Back pressure low threshold
     */
    GT_U32 portBackPressureLowThreshold;

    /** @brief  Port Back pressure high threshold
     */
    GT_U32 portBackPressureHighThreshold;
}
PRV_CPSS_DXCH_SIP6_TXQ_SDQ_PORT_ATTRIBUTES;


/*## type cpssFalconTxqSdqQueueAttributesStruct */
typedef struct
{
    GT_BOOL enable;                 /*## attribute enable */
    /* Aging threshold in clock cycles */
    GT_U32 agingThreshold;          /*## attribute agingThreshold */
    GT_U32 highCreditThreshold;     /*## attribute highCreditThreshold */
    GT_U32 lowCreditTheshold;       /*## attribute lowCreditTheshold */
    /* Minimum credit budget for selection. */
    GT_U32 negativeCreditThreshold; /*## attribute negativeCreditThreshold */
    GT_U32 tc;                      /*## attribute tc */
    GT_U32 semiEligThreshold;
}
PRV_CPSS_DXCH_SIP6_TXQ_SDQ_QUEUE_ATTRIBUTES;

typedef struct
{
    /* Port high range queue */
    GT_U32 hiQueueNumber;       /*## attribute hiQueueNumber */
    /* Port low range queue */
    GT_U32 lowQueueNumber;      /*## attribute lowQueueNumber */
} PRV_CPSS_DXCH_SIP6_TXQ_SDQ_PORT_QUEUE_RANGE;

typedef struct
{
    GT_U32 prio0HighLimit;
    GT_U32 prio0LowLimit;
    GT_U32 prio1HighLimit;
    GT_U32 prio1LowLimit;
} PRV_CPSS_DXCH_SIP6_TXQ_SDQ_SEL_LIST_RANGE;

GT_STATUS prvCpssSip6TxqSdqLocalPortQcnFactorSet
(
    IN GT_U8                                        devNum,
    IN GT_U32                                       tileNum,
    IN GT_U32                                       sdqNum,
    IN GT_U32                                       startQ,
    IN GT_U32                                       endQ,
    IN GT_U32                                       qcnFactor
);

GT_STATUS prvCpssSip6TxqSdqLocalPortQcnFactorGet
(
    IN GT_U8                                        devNum,
    IN GT_U32                                       tileNum,
    IN GT_U32                                       sdqNum,
    IN GT_U32                                       queue,
    IN GT_U32                                       *qcnFactorPtr
);



/**
* @internal prvCpssFalconTxqSdqQueueAttributesSet function
* @endinternal
*
* @brief   Queue attributes set
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[in] tileNum                  - the tile number  (APPLICABLE RANGES:0..3).
* @param[in] sdqNum                   - the local number of SDQ (APPLICABLE RANGES:0..7).
* @param[in] queueNumber              - queue number(APPLICABLE RANGES:0..399).
* @param[in] speed                    - speed of port that contain the queue
* @param[in] enable                   - queue enable/disable
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong sdq number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/

GT_STATUS prvCpssFalconTxqSdqQueueAttributesSet
(
    IN GT_U8                devNum,
    IN GT_U32               tileNum,
    IN GT_U32               sdqNum,
    IN GT_U32               queueNumber,
    IN CPSS_PORT_SPEED_ENT  speed,
    IN GT_BOOL              semiEligEnable
);

/**
* @internal prvCpssFalconTxqSdqLocalPortQueueRangeSet function
* @endinternal
*
* @brief   Map port to queue
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[in] tileNum                  - the tile number  (APPLICABLE RANGES:0..3).
* @param[in] sdqNum                   - the local number of SDQ (APPLICABLE RANGES:0..7).
* @param[in] localPortNum             - local port (APPLICABLE RANGES:0..8).
* @param[in] range                    - the  of the queues(APPLICABLE RANGES:0..399).
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong sdq number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssFalconTxqSdqLocalPortQueueRangeSet
(
    IN GT_U8                                        devNum,
    IN GT_U32                                       tileNum,
    IN GT_U32                                       sdqNum,
    IN GT_U32                                       localPortNum,
    IN PRV_CPSS_DXCH_SIP6_TXQ_SDQ_PORT_QUEUE_RANGE  range
);

/**
* @internal prvCpssFalconTxqSdqQueueAttributesGet function
* @endinternal
*
* @brief   Read Queue attributes
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[in] tileNum                  - the tile number  (APPLICABLE RANGES:0..3).
* @param[in] sdqNum                   - the global number of SDQ (APPLICABLE RANGES:0..7).
* @param[in] queueNumber              - queue number(APPLICABLE RANGES:0..399).
*
* @param[out] queueAttributesPtr      - pointer to queue attributes
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong sdq number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/

GT_STATUS prvCpssFalconTxqSdqQueueAttributesGet
(
    IN GT_U8                                        devNum,
    IN GT_U32                                       tileNum,
    IN GT_U32                                       sdqNum,
    IN GT_U32                                       queueNumber,
    OUT PRV_CPSS_DXCH_SIP6_TXQ_SDQ_QUEUE_ATTRIBUTES * queueAttributesPtr
);

/**
* @internal prvCpssFalconTxqSdqQueueEnableSet function
* @endinternal
*
* @brief   Enable/disable queue
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number
* @param[in] tileNum               - the tile number  (APPLICABLE RANGES:0..3).
* @param[in] sdqNum                - the global number of SDQ (APPLICABLE RANGES:0..31).
* @param[in] enable                - if GT_TRUE credit aging is enabled, otherwise disabled
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong sdq number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssFalconTxqSdqQueueEnableSet
(
 IN GT_U8 devNum,
 IN GT_U32 tileNum,
 IN GT_U32 sdqNum,
 IN GT_U32 localPortNumber,
 IN GT_U32 tc,
 IN GT_BOOL enable
);


/**
* @internal prvCpssFalconTxqSdqQueueEnableGet function
* @endinternal
*
* @brief   Get Enable/disable queue status
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[in] tileNum                  - the tile number  (APPLICABLE RANGES:0..3).
* @param[in] sdqNum                   - the global number of SDQ (APPLICABLE RANGES:0..7).
*
* @param[out] enablePtr                - if GT_TRUE queue  is enabled , otherwise disabled
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong sdq number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssFalconTxqSdqQueueEnableGet
(
 IN GT_U8   devNum,
 IN GT_U32  tileNum,
 IN GT_U32  sdqNum,
 IN GT_U32  localPortNumber,
 IN GT_U32  queueNumber,
 IN GT_BOOL * enablePtr
);

/**
* @internal prvCpssFalconTxqSdqLocalPortQueueRangeGet function
* @endinternal
*
* @brief   Read port to queue from specific SDQ
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[in] tileNum                  - the tile number  (APPLICABLE RANGES:0..3).
* @param[in] sdqNum                   - the global number of SDQ (APPLICABLE RANGES:0..31).
* @param[in] localPortNum             - local port (APPLICABLE RANGES:0..8).
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong sdq number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssFalconTxqSdqLocalPortQueueRangeGet
(
    IN GT_U8                                            devNum,
    IN GT_U32                                           tileNum,
    IN GT_U32                                           sdqNum,
    IN GT_U32                                           localPortNum,
    OUT PRV_CPSS_DXCH_SIP6_TXQ_SDQ_PORT_QUEUE_RANGE     * rangePtr
);


/**
* @internal prvCpssFalconTxqSdqLocalPortEnableSet function
* @endinternal
*
* @brief   Set port to enable state in SDQ
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number
* @param[in] tileNum               - Then number of tile (Applicable range 0..3)
* @param[in] sdqNum                - the local number of SDQ (APPLICABLE RANGES:0..7).
* @param[in] localPortNum          - local port (APPLICABLE RANGES:0..8).
* @param[in] enable                - If equal GT_TRUE port is enabled at SDQ, else otherwise
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong sdq number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/

GT_STATUS prvCpssFalconTxqSdqLocalPortEnableSet
(
 IN GT_U8 devNum,
 IN GT_U32 tileNum,
 IN GT_U32 sdqNum,
 IN GT_U32 localPortNum,
 IN GT_BOOL enable
);


/**
* @internal prvCpssFalconTxqSdqLocalPortEnableGet function
* @endinternal
*
* @brief   Get port to enable state in SDQ
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[in] tileNum                  - Then number of tile (Applicable range 0..3)
* @param[in] sdqNum                   - the global number of SDQ (APPLICABLE RANGES:0..31).
* @param[in] localPortNum             - local port (APPLICABLE RANGES:0..8).
*
* @param[out] enablePtr               - If equal GT_TRUE port is enabled at SDQ,else otherwise
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong sdq number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/

GT_STATUS prvCpssFalconTxqSdqLocalPortEnableGet
(
 IN GT_U8       devNum,
 IN GT_U32      tileNum,
 IN GT_U32      sdqNum,
 IN GT_U32      localPortNum,
 OUT GT_BOOL    * enablePtr
);

/**
* @internal prvCpssFalconTxqSdqPortAttributesSet function
* @endinternal
*
* @brief   Get port thresholds
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number
* @param[in] tileNum               - Then number of tile (Applicable range 0..3)
* @param[in] sdqNum                - the global number of SDQ (APPLICABLE RANGES:0..31).
* @param[in] localPortNum          - local port (APPLICABLE RANGES:0..8).
*
* @param[in] speed                 - port speed
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong sdq number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssFalconTxqSdqPortAttributesSet
(
    IN GT_U8                devNum,
    IN GT_U32               tileNum,
    IN GT_U32               sdqNum,
    IN GT_U32               localPortNum,
    IN CPSS_PORT_SPEED_ENT  speed
);

/**
* @internal prvCpssFalconTxqSdqPortAttributesGet function
* @endinternal
*
* @brief   Get port thresholds
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[in] tileNum                  - Then number of tile (Applicable range 0..3)
* @param[in] sdqNum                   - the global number of SDQ (APPLICABLE RANGES:0..31).
* @param[in] localPortNum             - local port (APPLICABLE RANGES:0..8).
*
* @param[out] portAttributesPtr       - (pointer to) port thresholds
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong sdq number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/

GT_STATUS prvCpssFalconTxqSdqPortAttributesGet
(
    IN GT_U8                                        devNum,
    IN GT_U32                                       tileNum,
    IN GT_U32                                       sdqNum,
    IN GT_U32                                       localPortNum,
    OUT PRV_CPSS_DXCH_SIP6_TXQ_SDQ_PORT_ATTRIBUTES  * portAttributesPtr
);

GT_STATUS prvCpssFalconTxqSdqQueueStrictPrioritySet
(
    IN GT_U8 devNum,
    IN GT_U32 tileNum,
    IN GT_U32 sdqNum,
    IN GT_U32 queueNumber,
    IN GT_BOOL sp
);


GT_STATUS prvCpssFalconTxqSdqSelectListSet
(
    IN GT_U8 devNum,
    IN GT_U32 tileNum,
    IN GT_U32 sdqNum,
    IN GT_U32 localPortNum,
    IN PRV_CPSS_DXCH_SIP6_TXQ_SDQ_SEL_LIST_RANGE * rangePtr
);


/**
* @internal prvCpssFalconTxqSdqSelectListGet function
* @endinternal
*
* @brief   Initialize queue thresholds for given speed
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] tileNum           - the tile number  (APPLICABLE RANGES:0..3).
* @param[in] sdqNum            - the local number of SDQ (APPLICABLE RANGES:0..7).
* @param[in] localPortNum      - local port (APPLICABLE RANGES:0..8).
* @param[in] range             - the  range of the queues
*
* @retval GT_OK                - on success.
* @retval GT_FAIL              - no thresholds are configured for this speed
*/
GT_STATUS prvCpssFalconTxqSdqSelectListGet
(
    IN GT_U8                                        devNum,
    IN GT_U32                                       tileNum,
    IN GT_U32                                       sdqNum,
    IN GT_U32                                       localPortNum,
    IN PRV_CPSS_DXCH_SIP6_TXQ_SDQ_SEL_LIST_RANGE    * rangePtr
);



GT_STATUS prvCpssFalconTxqSdqQCreditBalanceGet
(
    IN GT_U8        devNum,
    IN GT_U32       tileNum,
    IN GT_U32       sdqNum,
    IN GT_U32       queueNum,
    OUT  GT_U32     * balancePtr
);

GT_STATUS prvCpssFalconTxqSdqEligStateGet
(
    IN GT_U8        devNum,
    IN GT_U32       tileNum,
    IN GT_U32       sdqNum,
    IN GT_U32       queueNum,
    OUT  GT_U32     * eligStatePtr
);
/**
* @internal prvCpssFalconTxqSdqPfcControlEnableSet function
* @endinternal
*
* @brief   Enable /Disable  PFC response per port/TC.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - physical device number
* @param[in] tileNum               - the tile numebr (APPLICABLE RANGES:0..3).
* @param[in] sdqNum                - the local number of SDQ (APPLICABLE RANGES:0..7).
* @param[in] localPortNum          - local port (APPLICABLE RANGES:0..8).
* @param[in] queueOffset           - queue offset[0..15]
* @param[in] enable                - port/tc PFC responce enable option.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*
*/
GT_STATUS prvCpssFalconTxqSdqPfcControlEnableSet
(
    IN GT_U8   devNum,
    IN GT_U32  tileNum,
    IN GT_U32  sdqNum,
    IN GT_U32  localPortNumber,
    IN GT_U32  queueOffset,
    IN GT_BOOL enable
);
/**
* @internal prvCpssFalconTxqSdqPfcControlEnableGet function
* @endinternal
*
* @brief   Get enable /Disable  PFC response per port/TC.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - physical device number
* @param[in] tileNum               - the tile numebr (APPLICABLE RANGES:0..3).
* @param[in] sdqNum                - the local number of SDQ (APPLICABLE RANGES:0..7).
* @param[in] localPortNum          - local port (APPLICABLE RANGES:0..8).
* @param[in] queueOffset           - queue offset[0..15]
* @param[in] enablePtr             - (pointer to)port/tc PFC responce enable option.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*
*/
GT_STATUS prvCpssFalconTxqSdqPfcControlEnableGet
(
    IN GT_U8   devNum,
    IN GT_U32  tileNum,
    IN GT_U32  sdqNum,
    IN GT_U32  localPortNumber,
    IN GT_U32  queueNumber,
    IN GT_BOOL *enablePtr
);

/**
* @internal prvCpssFalconTxqSdqQueueTcSet function
* @endinternal
*
* @brief   Sets PFC TC to queue map. Meaning this table define
*          which Q should be paused on reception of perticular TC
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - physical device number
* @param[in] tileNum                  - the tile numebr (APPLICABLE RANGES:0..3).
* @param[in] sdqNum                   - the local number of SDQ (APPLICABLE RANGES:0..7).
* @param[in] localPortNum             - local port (APPLICABLE RANGES:0..8).
* @param[in] queueOffset              - queue offset[0..15]
* @param[in] tc                       - tc that is mapped to.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS prvCpssFalconTxqSdqQueueTcSet
(
    IN GT_U8  devNum,
    IN GT_U32 tileNum,
    IN GT_U32 sdqNum,
    IN GT_U32 queueNumber,
    IN GT_U32 tc
);

/**
* @internal prvCpssFalconTxqSdqPfcControlEnableBitmapGet  function
* @endinternal
*
* @brief   Get Enable /Disable  PFC response per port
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - physical device number
* @param[in] tileNum               - the tile numebr (APPLICABLE RANGES:0..3).
* @param[in] sdqNum                - the local number of SDQ (APPLICABLE RANGES:0..7).
* @param[in] localPortNum          - local port (APPLICABLE RANGES:0..8).
* @param[in] queueOffset           - queue offset[0..15]
* @param[in] enableBtmpPtr         - (pointer to)port PFC responce enable bitmap option.(each bit represent queue)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS prvCpssFalconTxqSdqPfcControlEnableBitmapGet
(
    IN GT_U8   devNum,
    IN GT_U32  tileNum,
    IN GT_U32  sdqNum,
    IN GT_U32  localPortNumber,
    IN GT_U32 *enableBtmpPtr
);

/**
* @internal prvCpssFalconTxqSdqPfcControlEnableBitmapSet function
* @endinternal
*
* @brief   Enable /Disable  PFC response per port
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - physical device number
* @param[in] tileNum               - the tile numebr (APPLICABLE RANGES:0..3).
* @param[in] sdqNum                - the local number of SDQ (APPLICABLE RANGES:0..7).
* @param[in] localPortNum          - local port (APPLICABLE RANGES:0..8).
* @param[in] queueOffset           - queue offset[0..15]
* @param[in] enableBtmp            - port PFC responce enable bitmap option.(each bit represent queue)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*
*/
GT_STATUS prvCpssFalconTxqSdqPfcControlEnableBitmapSet
(
    IN GT_U8   devNum,
    IN GT_U32  tileNum,
    IN GT_U32  sdqNum,
    IN GT_U32  localPortNumber,
    IN GT_U32  enableBtmp
);

/**
* @internal prvCpssFalconTxqSdqLocalPortFlushSet function
* @endinternal
*
* @brief  Set port to "credit ignore" mode.This make port to transmit whenever there is data to send ignoring credits.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - physical device number
* @param[in] tileNum               - the tile numebr (APPLICABLE RANGES:0..3).
* @param[in] sdqNum                - the local number of SDQ (APPLICABLE RANGES:0..7).
* @param[in] localPortNum          - local port (APPLICABLE RANGES:0..8).
* @param[in] enable                - Enable/disable "credit ignore" mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS prvCpssFalconTxqSdqLocalPortFlushSet
(
 IN GT_U8   devNum,
 IN GT_U32  tileNum,
 IN GT_U32  sdqNum,
 IN GT_U32  localPortNum,
 IN GT_BOOL enable
);

/**
* @internal prvCpssFalconTxqSdqQueueStatusDump function
* @endinternal
*
* @brief  Dump queue eligeble status in parsed format
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum           - physical device number
* @param[in] tileNum          - the tile numebr (APPLICABLE RANGES:0..3).
* @param[in] sdqNum           - the local number of SDQ (APPLICABLE RANGES:0..7).
* @param[in] startQ           - first queue to dump
* @param[in] size             - number of queues to dump.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS prvCpssFalconTxqSdqQueueStatusDump
(
    IN  GT_U8                   devNum,
    IN  GT_U32                  tileNumber,
    IN  GT_U32                  sdqNum,
    IN  GT_U32                  startQ,
    IN  GT_U32                  size,
    IN  CPSS_OS_FILE_TYPE_STC   * file
);

/**
* @internal prvCpssFalconTxqSdqQueueStrictPriorityGet function
* @endinternal
*
* @brief   Get queue strict priority bit
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number
* @param[in] tileNum               - Then number of tile (Applicable range 0..3)
* @param[in] sdqNum                - the global number of SDQ (APPLICABLE RANGES:0..31).
* @param[in] queueNumber           - local queue number  (APPLICABLE RANGES:0..399).
*
* @param[out] spPtr                - (pointer to)strict priority bit
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong sdq number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssFalconTxqSdqQueueStrictPriorityGet
(
    IN GT_U8    devNum,
    IN GT_U32   tileNum,
    IN GT_U32   sdqNum,
    IN GT_U32   queueNumber,
    IN GT_BOOL  *spPtr
);

/**
* @internal prvCpssFalconTxqSdqSemiEligEnableSet function
* @endinternal
*
* @brief   Enable/disable semi elig feature
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[in] tileNum                  - the tile number  (APPLICABLE RANGES:0..3).
* @param[in] sdqNum                   - the global number of SDQ (APPLICABLE RANGES:0..31).
* @param[in] seligEnable              - if GT_TRUE semi elig feature is enabled, otherwise disabled
* @param[in] selig4DryEnable          - if GT_TRUE Enable selection slow down when semi elig by entering queue to the qdryer after selection,
*                                       otherwise disabled
* @param[in] granularity1kEnable      - Change Semi Elig threshold granule to 1KB. In that case, Semi Elig Threshold range will be 1KB-128KB
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong sdq number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssFalconTxqSdqSemiEligEnableSet
(
    IN GT_U8   devNum,
    IN GT_U32  tileNum,
    IN GT_U32  sdqNum,
    IN GT_BOOL seligEnable,
    IN GT_BOOL selig4DryEnable,
    IN GT_BOOL granularity1kEnable
);

/**
 * @internal prvCpssSip6_10TxqSdqSelectListEnableSet function
 * @endinternal
 *
 * @brief   Enable/disable select list range
 *
 * @note   APPLICABLE DEVICES: AC5P; AC5X; Harrier; Ironman
 * @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
 *
 * @param[in] devNum                - device number
 * @param[in] tileNum               - Then number of tile (Applicable range 0..3)
 * @param[in] sdqNum                - the global number of SDQ (APPLICABLE RANGES:0..31).
 * @param[in] localPortNum          - local port number  (APPLICABLE RANGES:0..26).
 * @param[in] lowPriority           - Defines the range.If equal GT_TRUE then range=0 ,else range=1.
 * @param[in] enable                - Enable/Disable select list range(0,1)
 *
 * @retval GT_OK                    - on success.
 * @retval GT_BAD_PARAM             - wrong sdq number.
 * @retval GT_HW_ERROR              - on writing to HW error.
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 */
 GT_STATUS prvCpssSip6_10TxqSdqSelectListEnableSet
 (
     IN GT_U8   devNum,
     IN GT_U32  tileNum,
     IN GT_U32  sdqNum,
     IN GT_U32  localPortNum,
     IN GT_BOOL lowPriority,
     IN GT_BOOL enable
 );
/**
  * @internal prvCpssFalconTxqSdqSelectListEnableGet function
  * @endinternal
  *
  * @brief   Get enable/disable select list range
  *
  * @note   APPLICABLE DEVICES: AC5P; AC5X; Harrier; Ironman
  * @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
  *
  * @param[in] devNum                - device number
  * @param[in] tileNum               - Then number of tile (Applicable range 0..3)
  * @param[in] sdqNum                - the global number of SDQ (APPLICABLE RANGES:0..31).
  * @param[in] localPortNum          - local port number  (APPLICABLE RANGES:0..26).
  * @param[in] lowPriority           - Defines the range.If equal GT_TRUE then range=0 ,else range=1.
  * @param[out] enablePtr            -(pointer to)Enable/Disable select list range(0,1)
  *
  * @retval GT_OK                    - on success.
  * @retval GT_BAD_PARAM             - wrong sdq number.
  * @retval GT_HW_ERROR              - on writing to HW error.
  * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
  */
 GT_STATUS prvCpssFalconTxqSdqSelectListEnableGet
 (
     IN  GT_U8   devNum,
     IN  GT_U32  tileNum,
     IN  GT_U32  sdqNum,
     IN  GT_U32  localPortNum,
     IN  GT_BOOL lowPriority,
     OUT GT_BOOL *enablePtr
 );

GT_STATUS prvCpssFalconTxqSdqPortCreditBalanceGet
(
    IN GT_U8     devNum,
    IN GT_U32    tileNum,
    IN GT_U32    sdqNum,
    IN GT_U32    portNum,
    OUT  GT_U32  * balancePtr,
    OUT  GT_U32  * qCountPtr
);

GT_STATUS prvCpssFalconTxqSdqSelectListPtrGet
(
     IN  GT_U8   devNum,
     IN  GT_U32  tileNum,
     IN  GT_U32  sdqNum,
     IN  GT_U32  ind,
     OUT GT_U32  *value
);
/**
* @internal prvCpssSip6TxqRegisterFieldWrite function
* @endinternal
*
* @brief   Function write to SDQ register.Also check that register is defined add tile offset
*
* @note   APPLICABLE DEVICES: Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number
* @param[in] tileNum               - The number of tile (Applicable range 0..3)
* @param[in] regAddr               - Adress of the register in SDQ
* @param[in] fieldOffset           - The start bit number in the register.
* @param[in] fieldLength           - The number of bits to be written to register.
* @param[in] fieldData             - Data to be written into the register.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong sdq number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssSip6TxqRegisterFieldWrite
(
     IN  GT_U8    devNum,
     IN  GT_U32   tileNum,
     IN PRV_CPSS_DXCH_TXQ_SIP_6_REG_LOG_STC * logDataPtr,
     IN  GT_U32   regAddr,
     IN  GT_U32   fieldOffset,
     IN  GT_U32   fieldLength,
     IN  GT_U32   fieldData
);
/**
* @internal prvCpssSip6TxqRegisterFieldRead function
* @endinternal
*
* @brief   Function read from  SDQ register.Also check that register is defined add tile offset.
*
* @note   APPLICABLE DEVICES: Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number
* @param[in] tileNum               - The number of tile (Applicable range 0..3)
* @param[in] regAddr               - Adress of the register in SDQ
* @param[in] fieldOffset           - The start bit number in the register.
* @param[in] fieldLength           - The number of bits to be read.
*
* @param[out] fieldDataPtr         - (pointer to) Data to read from the register.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong sdq number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssSip6TxqRegisterFieldRead
(
     IN   GT_U8    devNum,
     IN   GT_U32   tileNum,
     IN   GT_U32   regAddr,
     IN   GT_U32   fieldOffset,
     IN   GT_U32   fieldLength,
     OUT  GT_U32   *fieldDataPtr
);

GT_STATUS prvCpssSip6TxqWriteTableEntry
(
    IN GT_U8                  devNum,
    IN PRV_CPSS_DXCH_TXQ_SIP_6_TABLE_LOG_STC *logDataPtr,
    IN CPSS_DXCH_TABLE_ENT    tableType,
    IN GT_U32                 entryIndex,
    IN GT_U32                  *entryValuePtr

);

/**
* @internal prvCpssSip6TxqSdqDebugInterruptDisableSet function
* @endinternal
*
* @brief   Debug function that disable iterrupt in SDQ
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
GT_STATUS prvCpssSip6TxqSdqDebugInterruptDisableSet
(
    IN GT_U8  devNum,
    IN GT_U32 tileNum,
    IN GT_U32 sdqNum
);

/**
* @internal prvCpssSip6TxqSdqDebugInterruptGet function
* @endinternal
*
* @brief   Get interrupt cause for SDQ
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
GT_STATUS prvCpssSip6TxqSdqDebugInterruptGet
(
    IN GT_U8    devNum,
    IN GT_U32   tileNum,
    IN GT_U32   sdqNum,
    OUT GT_U32  *functionalPtr,
    OUT GT_U32  *debugPtr
);

GT_U32 prvCpssSip6TxqSdqUnMapPortErrorBmp
(
    IN GT_U8  devNum
);


/**
* @internal prvCpssSip6TxqSdqErrorCaptureGet function
* @endinternal
*
* @brief  Capture SDQ errors
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum               -  physical device number
* @param[in] tileNum              -  the tile numebr (APPLICABLE RANGES:0..3).
* @param[in] sdqNum               -  the local number of SDQ (APPLICABLE RANGES:0..7).
* @param[in] selectErrorValidPtr  -  (pointer to)GT_TRUE if select error is valid,GT_FALSE otherwise
* @param[in] selectErrorPortPtr   -  (pointer to) port that caused select error
* @param[in] selectErrorPortPtr   -  (pointer to) elig error raw format
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS prvCpssSip6TxqSdqErrorCaptureGet
(
 IN  GT_U8      devNum,
 IN  GT_U32     tileNum,
 IN  GT_U32     sdqNum,
 OUT GT_BOOL    *selectErrorValidPtr,
 OUT GT_U32     *selectErrorPortPtr,
 OUT GT_U32     *eligPtr
);

GT_STATUS prvCpssSip6TxqSdqErrorCaptureDump
(
 IN  GT_U8  devNum,
 IN  GT_U32 tileNum,
 IN  GT_U32 sdqNum
);

/**
* @internal prvCpssSip6TxqSdqPausedTcBmpSet function
* @endinternal
*
* @brief   Pause queues for local port
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number
* @param[in] tileNum               - the tile number  (APPLICABLE RANGES:0..3).
* @param[in] localPortNum          - local port number of SDQ
* @param[in] pausedTcBmp           - bitmap of paused queues
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong sdq number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssSip6TxqSdqPausedTcBmpSet
(
 IN  GT_U8   devNum,
 IN  GT_U32  tileNum,
 IN  GT_U32  sdqNum,
 IN  GT_U32  localPortNum,
 IN  GT_U32  pausedTcBmp
);
/**
* @internal prvCpssSip6TxqSdqPausedTcBmpSet function
* @endinternal
*
* @brief   Pause queues for local port
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number
* @param[in] tileNum               - the tile number  (APPLICABLE RANGES:0..3).
* @param[in] localPortNum          - local port number of SDQ
* @param[out] pausedTcBmpPtr       -(pointer to )bitmap of paused queues
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong sdq number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssSip6TxqSdqPausedTcBmpGet
(
 IN  GT_U8   devNum,
 IN  GT_U32  tileNum,
 IN  GT_U32  sdqNum,
 IN  GT_U32  localPortNum,
 OUT GT_U32  *pausedTcBmpPtr
);


/**
* @internal prvCpssSip6_30TxqSdqQbvScanEnableSet function
* @endinternal
*
* @brief   Set QBV scan
*
* @note   APPLICABLE DEVICES: Ironman L
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum                - device number
* @param[in] tileNum               - The number of tile (Applicable range 0..3)
* @param[in] sdqNum                - data path index
* @param[in] enable                -enable/disable qbv scan
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong sdq number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/

GT_STATUS prvCpssSip6_30TxqSdqQbvScanEnableSet
(
    IN GT_U8   devNum,
    IN GT_U32  tileNum,
    IN GT_U32  sdqNum,
    IN GT_BOOL enable
);

/**
 * @internal prvCpssSip6TxqSdqLocalPortConfigGet function
 * @endinternal
 *
 * @brief   Get port configuration in raw format
 *
 * @note   APPLICABLE DEVICES: Falcon; AC5P; AC5X; Harrier; Ironman.
 * @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
 *
 * @param[in] devNum                - device number
 * @param[in] tileNum               - The number of tile (Applicable range 0..3)
 * @param[in] sdqNum                - data path index
 * @param[out] dataPtr                -(pointer to)value of port configuration register
 *
 * @retval GT_OK                    - on success.
 * @retval GT_BAD_PARAM             - wrong sdq number.
 * @retval GT_HW_ERROR              - on writing to HW error.
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 */
GT_STATUS prvCpssSip6TxqSdqLocalPortConfigGet
(
 IN GT_U8       devNum,
 IN GT_U32      tileNum,
 IN GT_U32      sdqNum,
 IN GT_U32      localPortNum,
 OUT GT_U32    * dataPtr
);


/**
* @internal prvCpssSip6_30TxqSdqLocalPortPreemptionTypeSet function
* @endinternal
*
* @brief   Set preemption type for SDQ port(required for hold/release feature configuration)
*
* @note   APPLICABLE DEVICES: Ironman L
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum                - device number
* @param[in] tileNum               - The number of tile (Applicable range 0..3)
* @param[in] sdqNum                - data path index
* @param[in] preemptive                - act as preemptive channel
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong sdq number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/

GT_STATUS prvCpssSip6_30TxqSdqLocalPortPreemptionTypeSet
(
 IN GT_U8   devNum,
 IN GT_U32  tileNum,
 IN GT_U32  sdqNum,
 IN GT_U32  localPortNum,
 IN GT_BOOL preemptive
);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssDxChTxqSdq */

