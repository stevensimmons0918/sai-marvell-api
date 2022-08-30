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
* @file prvCpssDxChPortTxPizzaResourceHawk.h
*
* @brief  CPSS Hawk Data Path Pizza and resource configuration.
*
* @version   0
********************************************************************************
*/
#ifndef __prvCpssDxChPortTxPizzaResourceHawk_h
#define __prvCpssDxChPortTxPizzaResourceHawk_h

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/port/cpssPortCtrl.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/common/port/private/prvCpssPortTypes.h>

/* SIP_6_10 PIZZA ARBITER Algorithm types */

/**
* @enum PRV_CPSS_DXCH_PIZZA_ARBITER_PROJECT_ENT
 *
 * @brief Enumeration of pizza configuration projects
*/
typedef enum
{
    /** default Hawk arbiters configuration */
    PRV_CPSS_DXCH_PIZZA_ARBITER_PROJECT_HAWK_DEFAULT_E,

    /** default Phoenix configuration */
    PRV_CPSS_DXCH_PIZZA_ARBITER_PROJECT_PHOENIX_E,

    /** default Harrier configuration */
    PRV_CPSS_DXCH_PIZZA_ARBITER_PROJECT_HARRIER_E,

    /** default IRONMAN_L configuration */
    PRV_CPSS_DXCH_PIZZA_ARBITER_PROJECT_IRONMAN_L_E,

} PRV_CPSS_DXCH_PIZZA_ARBITER_PROJECT_ENT;

/**
* @enum PRV_CPSS_DXCH_PIZZA_ARBITER_UNIT_ENT
 *
 * @brief Enumeration of pizza configuration types
*/
typedef enum
{
    /** TX_DMA */
    PRV_CPSS_DXCH_PIZZA_ARBITER_UNIT_TX_DMA_E,

    /** TX_FIFO */
    PRV_CPSS_DXCH_PIZZA_ARBITER_UNIT_TX_FIFO_E,

    /** TX_PB_GPC_PKT_READ */
    PRV_CPSS_DXCH_PIZZA_ARBITER_UNIT_PB_GPC_PKT_READ_E,

    /** PCA RX */
    PRV_CPSS_DXCH_PIZZA_ARBITER_UNIT_PCA_RX_E,

    /** PCA TX */
    PRV_CPSS_DXCH_PIZZA_ARBITER_UNIT_PCA_TX_E,

    /** PCA Bridge RX */
    PRV_CPSS_DXCH_PIZZA_ARBITER_UNIT_PCA_BRG_RX_E,

    /** PCA Bridge TX */
    PRV_CPSS_DXCH_PIZZA_ARBITER_UNIT_PCA_BRG_TX_E,

    /** PCA SFF FIFO elements rings */
    PRV_CPSS_DXCH_PIZZA_ARBITER_UNIT_PCA_SFF_E

} PRV_CPSS_DXCH_PIZZA_ARBITER_UNIT_ENT;

/**
* @enum PRV_CPSS_DXCH_PIZZA_ARBITER_PREEMTION_ENT
 *
 * @brief Enumeration of channel preemption types
*/
typedef enum
{
    /** channel out of preemption pair */
    PRV_CPSS_DXCH_PIZZA_ARBITER_NOPREEMTION_E,

    /** express channel in preemption pair */
    PRV_CPSS_DXCH_PIZZA_ARBITER_EXPRESS_E,

    /** preemtive channel in preemption pair */
    PRV_CPSS_DXCH_PIZZA_ARBITER_PREEMPTIVE_E

} PRV_CPSS_DXCH_PIZZA_ARBITER_PREEMTION_ENT;

/* resource is a set of pizza arbiter slots having ID       */
/* maximal resource ID and amount of words im bitmap of IDs */
#define PRV_CPSS_DXCH_PIZZA_ARBITER_MAX_RESOURCE_ID_CNS 128
#define PRV_CPSS_DXCH_PIZZA_ARBITER_WORDS_IN_RESOURCE_ID_BMP_CNS \
    ((PRV_CPSS_DXCH_PIZZA_ARBITER_MAX_RESOURCE_ID_CNS + 31) / 32)
#define PRV_CPSS_DXCH_PIZZA_ARBITER_MAX_SLOTS_CNS 256
#define PRV_CPSS_DXCH_PIZZA_ARBITER_WORDS_IN_SLOTS_BMP_CNS \
    ((PRV_CPSS_DXCH_PIZZA_ARBITER_MAX_SLOTS_CNS + 31) / 32)

/**
* @enum PRV_CPSS_DXCH_PIZZA_ARBITER_SPEED_ID_TO_VAL_STC
*
*  @brief the structure describes value per speed id range
*  the found value is a pair of the least speedIdBound greater than the given speed Id
*/
typedef struct{
    /** bitmap of channels relevant to the entry */
    GT_U32             channelBitmapArr[2];
    /** the lowest speed Id relevant to the value, 0xFFFFFFFF for entry marking end of table */
    GT_U32             speedIdLowest;
    /** value */
    GT_U32             value;
} PRV_CPSS_DXCH_PIZZA_ARBITER_SPEED_ID_TO_VAL_STC;

/**
* @enum PRV_CPSS_DXCH_PIZZA_ARBITER_CHANNEL_RESOURCES_STC
 *
 * @brief the structure describes pattern for pizza arbiter configuration
*/
typedef struct{
    /** channel Id, 0xFFFFFFFF for entry marking end of table */
    GT_U32             channelId;
    /** speed Id Low */
    GT_U32             speedIdLow;
    /** speed Id High */
    GT_U32             speedIdHigh;
    /** resource Ids bitmap */
    GT_U32             resourceIdsBmp[PRV_CPSS_DXCH_PIZZA_ARBITER_WORDS_IN_RESOURCE_ID_BMP_CNS];
} PRV_CPSS_DXCH_PIZZA_ARBITER_CHANNEL_RESOURCES_STC;

/**
* @enum PRV_CPSS_DXCH_PIZZA_ARBITER_SPEED_TO_PROFILE_STC
 *
 * @brief the structure describes entry of speedId to profile table
*/
typedef struct{
    /** speed Id, Low 0xFFFFFFFF for entry marking end of table */
    GT_U32             speedIdLow;
    /** speed Id High */
    GT_U32             speedIdHigh;
    /** profile Id */
    GT_U32             profileId;
} PRV_CPSS_DXCH_PIZZA_ARBITER_SPEED_TO_PROFILE_STC;

/**
* @enum PRV_CPSS_DXCH_PIZZA_ARBITER_SPEED_UP_PROFILE_CFG_STC
 *
 * @brief the structure describes entry of speed up profile configuration table
*/
typedef struct{
    /** speedId 0xFFFFFFFF for entry marking end of table */
    GT_U32             speedId;
    /** profile Id */
    GT_U32             profileId;
    /** pattern profile Id */
    GT_U32             patternProfileId;
    /** TX DMA idle rate inc */
    GT_U32             txDmaIdleRateIncrement;
    /** PB GPR shaper leak value */
    GT_U32             pbGprShaperLeakValue;
} PRV_CPSS_DXCH_PIZZA_ARBITER_SPEED_UP_PROFILE_CFG_STC;

/**
* @enum PRV_CPSS_DXCH_PIZZA_ARBITER_PATTERN_STC
 *
 * @brief the structure describes pattern for pizza arbiter configuration
*/
typedef struct{
    /** number of slots in arbiter */
    GT_U32             numOfSlots;
    /** Pizza offset for PCA arbiter for inactive MACSEC */
    GT_U32             pizzaOffsetMacSecOff;
    /** Pizza offset for PCA arbiter  for active MACSEC */
    GT_U32             pizzaOffsetMacSecOn;
    /** number of channels in unit */
    GT_U32             numOfChannels;
    /** default number of slot in disabled arbiter slot register in unit */
    GT_U32             defaultSlotNumber;
    /** table of ID of resource containing the slot per slot */
    const GT_U32       *resourceIdPerSlotPtr;
    /** table of resources per all supported pairs channel and speed                    */
    /** can be common for different units, entries with channel > numOfChannels ignored */
    const PRV_CPSS_DXCH_PIZZA_ARBITER_CHANNEL_RESOURCES_STC *resourcesPerChannelSpeedPtr;
    /** speedId to Profile Table */
    const PRV_CPSS_DXCH_PIZZA_ARBITER_SPEED_TO_PROFILE_STC *speedIdToProfileTablePtr;
} PRV_CPSS_DXCH_PIZZA_ARBITER_PATTERN_STC;

/**
* @enum PRV_CPSS_DXCH_PIZZA_ARBITER_HW_INFO_STC
 *
 * @brief the structure describes HW info of pizza arbiter
*/
typedef struct{
    /** control register address */
    GT_U32             ctrlRegAddr;
    /** control register enable bit  */
    GT_U32             ctrlRegEnableBit;
    /** control register size field position  */
    GT_U32             ctrlRegSizeFldPos;
    /** control register size field length  */
    GT_U32             ctrlRegSizeFldLen;
    /** control register Pizza Offset field position  */
    GT_U32             ctrlRegPizzaOffsetFldPos;
    /** control register Pizza Offset field length  */
    GT_U32             ctrlRegPizzaOffsetFldLen;
    /** pointer to array of slot register addresses */
    const GT_U32       *slotRegAddrArrPtr;
    /** slot register valid bit  */
    GT_U32             slotRegValidBit;
    /** slot register channel field position  */
    GT_U32             slotRegChannelFldPos;
    /** slot register channel field length  */
    GT_U32             slotRegChannelFldLen;
    /** pointer to array of channel speed profile register addresses */
    const GT_U32       *channelSpeedProfileRegAddrArrPtr;
    /** channel speed profile field position */
    GT_U32             channelSpeedProfileFldPos;
    /** channel speed profile field length */
    GT_U32             channelSpeedProfileFldLen;
    /** pointer to array of channel reset register addresses */
    const GT_U32       *channelResetRegAddrArrPtr;
    /** channel reset number of writes in sequence */
    GT_U32             channelResetNumOfWrites;
    /** channel reset number of writes in sequence */
    GT_U32             channelResetBitsToWriteWrites;
    /** cpu Channel local Index */
    GT_U32             cpuChannelLocalIndex;
    /** cpu Channel global Index ( For TX_DMA and TX_FIFO regDB - 0xFFFFFFFF - use regular array)*/
    GT_U32             cpuChannelGlobalIndex;

} PRV_CPSS_DXCH_PIZZA_ARBITER_HW_INFO_STC;


/**
* @enum PRV_CPSS_DXCH_PIZZA_ARBITER_CHANNEL_INIT_SPEED_STC
 *
 * @brief the structure is an entry in the table of initial arbiter state
*/
typedef struct{
    /** channel Id, 0xFFFFFFFF for entry marking end of table */
    GT_U32                          channelId;
    /** speedEnum */
    CPSS_PORT_SPEED_ENT             speedEnum;
} PRV_CPSS_DXCH_PIZZA_ARBITER_CHANNEL_INIT_SPEED_STC;

/**
* @enum PRV_CPSS_DXCH_PIZZA_ARBITER_CHANNEL_INIT_PORT_TYPE_SPEED_STC
 *
 * @brief the structure is an entry in the table of initial arbiter state
*/
typedef struct{
    /** channel Id, 0xFFFFFFFF for entry marking end of table */
    GT_U32                          channelId;
    /** portType */
    PRV_CPSS_PORT_TYPE_ENT          portType;
    /** speedEnum */
    CPSS_PORT_SPEED_ENT             speedEnum;
} PRV_CPSS_DXCH_PIZZA_ARBITER_CHANNEL_INIT_PORT_TYPE_SPEED_STC;

/**
* @internal prvCpssDxChTxPortSpeedPizzaResourcesTxChannelAndIsPcaFromPortNum function
* @endinternal
*
* @brief   Gets speed profile for specified port speed on specified device.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                 - physical device number
* @param[in] portNum                - physical port number
*
* @param[out] txDmaIndexPtr       - (pointer to)  Tx global
*       index (0..3)
* @param[out] channelIndexPtr     - (pointer to)  channel local
*      index in DP (0..26)
* @param[out] isPca               - (pointer to)  1 - PCA, 0 - SDMA
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_NOT_SUPPORTED         - wrong media interface mode
*         value received
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChTxPortSpeedPizzaResourcesTxChannelAndIsPcaFromPortNum
(
    IN  GT_U8                        devNum,
    IN  GT_PHYSICAL_PORT_NUM         portNum,
    OUT GT_U32                       *txDmaIndexPtr,
    OUT GT_U32                       *channelIndexPtr,
    OUT GT_U32                       *isPca
);

/**
* @internal prvCpssDxChTxPortSpeedPizzaResourcesUnitIdToName function
* @endinternal
*
* @brief    Convert unit id to unit printable name.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] unitType               - id of unit
*
* @retval unit name or UNKNOWN
*/
const char* prvCpssDxChTxPortSpeedPizzaResourcesUnitIdToName
(
    IN  PRV_CPSS_DXCH_PIZZA_ARBITER_UNIT_ENT      unitType
);

/**
* @internal prvCpssDxChTxPortSpeedPizzaProjectGet function
* @endinternal
*
* @brief    Get device projecty Id.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in]  devNum               - physical device number
* @param[out] projectPtr           - (pointer to)project Id.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad parameter value
* @retval GT_NOT_APPLICABLE_DEVICE - not applicable device
*/
GT_STATUS prvCpssDxChTxPortSpeedPizzaProjectGet
(
    IN  GT_U8                                          devNum,
    OUT  PRV_CPSS_DXCH_PIZZA_ARBITER_PROJECT_ENT       *projectPtr
);

/**
* @internal prvCpssDxChTxPortSpeedPizzaResourcesValueBySpeedIdGet function
* @endinternal
*
* @brief    Get get the most exact value found by speed id.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] valueTablePtr          - table of pairs <speedIdBound, value>.
* @param[in] valueTableSize         - size of table
* @param[in] channelId              - channel Id for search
* @param[in] speedId                - speed Id for search
*
* @retval the most exact value found by speed id, NOT_VALID_CNS if not found
*/
GT_U32 prvCpssDxChTxPortSpeedPizzaResourcesValueBySpeedIdGet
(
    IN  const PRV_CPSS_DXCH_PIZZA_ARBITER_SPEED_ID_TO_VAL_STC   *valueTablePtr,
    IN  const GT_U32                                            valueTableSize,
    IN  GT_U32                                                  channelId,
    IN  GT_U32                                                  speedId
);

/**
* @internal prvCpssDxChTxPortSpeedPizzaResourcesPatternGet function
* @endinternal
*
* @brief    Get pattern of pizza arbiter.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] project                - project Id.
* @param[in] unitType               - id of unit containig pizza arbiter
* @param[in]  unitIndex             - unit index
* @retval pointer to pizza arbiter pattern on success or NULL
*/
const PRV_CPSS_DXCH_PIZZA_ARBITER_PATTERN_STC* prvCpssDxChTxPortSpeedPizzaResourcesPatternGet
(
    IN  PRV_CPSS_DXCH_PIZZA_ARBITER_PROJECT_ENT   project,
    IN  PRV_CPSS_DXCH_PIZZA_ARBITER_UNIT_ENT      unitType,
    IN  GT_U32                                    unitIndex
);

/**
* @internal prvCpssDxChTxPortSpeedPizzaResourcesSlotBitmapGet function
* @endinternal
*
* @brief    Get bitmap of pizza arbiter slots for given Tx channel and speed.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] project                - project Id.
* @param[in] unitType               - id of unit containig pizza arbiter
* @param[in] unitIndex              - unut index
* @param[in] channel                - local number of channel
* @param[in] portSpeed              - port speed
* @param[in] slotBmpPtr             - (pointer to)bitmap should be cleared
* @param[out] slotBmpPtr            - (pointer to)bitmap of arbiter slots for channel
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad parameter value
* @retval GT_BAD_PTR               - on null pointer
*/
GT_STATUS prvCpssDxChTxPortSpeedPizzaResourcesSlotBitmapGet
(
    IN    PRV_CPSS_DXCH_PIZZA_ARBITER_PROJECT_ENT   project,
    IN    PRV_CPSS_DXCH_PIZZA_ARBITER_UNIT_ENT      unitType,
    IN    GT_U32                                    unitIndex,
    IN    GT_U32                                    channel,
    IN    CPSS_PORT_SPEED_ENT                       portSpeed,
    INOUT GT_U32                                    *slotBmpPtr
);

/**
* @internal prvCpssDxChTxPortSpeedPizzaResourcesHwInfoGet function
* @endinternal
*
* @brief    Get pizza arbiter HW info.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in]  devNum               - physical device number
* @param[in]  unitType             - type of unit containig pizza arbiter
* @param[in]  unitIndex            - index of unit containig pizza arbiter
* @param[out] hwInfoPtr            - (pointer to) HW info of pizza arbiter
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad parameter value
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - not applicable device
*/
GT_STATUS prvCpssDxChTxPortSpeedPizzaResourcesHwInfoGet
(
    IN  GT_U8                                     devNum,
    IN  PRV_CPSS_DXCH_PIZZA_ARBITER_UNIT_ENT      unitType,
    IN  GT_U32                                    unitIndex,
    OUT PRV_CPSS_DXCH_PIZZA_ARBITER_HW_INFO_STC   *hwInfoPtr
);

/**
* @internal prvCpssDxChTxPortSpeedPizzaResourcesInitSpeedsGet function
* @endinternal
*
* @brief    Get pattern of initial speeds of arbiter channels.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] project                - project Id.
* @param[in] unitType               - id of unit containig pizza arbiter
*
* @retval pointer to pizza arbiter pattern on success or NULL
*/
const PRV_CPSS_DXCH_PIZZA_ARBITER_CHANNEL_INIT_SPEED_STC* prvCpssDxChTxPortSpeedPizzaResourcesInitSpeedsGet
(
    IN  PRV_CPSS_DXCH_PIZZA_ARBITER_PROJECT_ENT   project,
    IN  PRV_CPSS_DXCH_PIZZA_ARBITER_UNIT_ENT      unitType
);

/**
* @internal prvCpssDxChTxPortSpeedPizzaResourcesChannelArbiterConfigure function
* @endinternal
*
* @brief    Configure arbiter for given channel and speed.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                 - physical device number
* @param[in] project                - project Id.
* @param[in] unitType               - id of unit containig pizza arbiter
* @param[in] unitIndex              - index of unit containig pizza arbiter
* @param[in] channel                - local number of channel
* @param[in] portSpeed              - port speed
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad parameter value
* @retval GT_BAD_STATE             - other channel slots overlapped
*/
GT_STATUS prvCpssDxChTxPortSpeedPizzaResourcesChannelArbiterConfigure
(
    IN    GT_U8                                     devNum,
    IN    PRV_CPSS_DXCH_PIZZA_ARBITER_PROJECT_ENT   project,
    IN    PRV_CPSS_DXCH_PIZZA_ARBITER_UNIT_ENT      unitType,
    IN    GT_U32                                    unitIndex,
    IN    GT_U32                                    channel,
    IN    CPSS_PORT_SPEED_ENT                       portSpeed
);

/**
* @internal prvCpssDxChTxPortSpeedPizzaResourcesChannelArbiterInvalidate function
* @endinternal
*
* @brief    Invalidate arbiter for given channel.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in]  devNum                - physical device number
* @param[in] project                - project Id.
* @param[in] unitType               - id of unit containig pizza arbiter
* @param[in] unitIndex              - index of unit containig pizza arbiter
* @param[in] channel                - local number of channel
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad parameter value
* @retval GT_BAD_PTR               - on null pointer
*/
GT_STATUS prvCpssDxChTxPortSpeedPizzaResourcesChannelArbiterInvalidate
(
    IN    GT_U8                                     devNum,
    IN    PRV_CPSS_DXCH_PIZZA_ARBITER_PROJECT_ENT   project,
    IN    PRV_CPSS_DXCH_PIZZA_ARBITER_UNIT_ENT      unitType,
    IN    GT_U32                                    unitIndex,
    IN    GT_U32                                    channel
);

/**
* @internal prvCpssDxChTxPortSpeedPizzaResourcesChannelProfileSet function
* @endinternal
*
* @brief    Set profile for given channel and speed.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                 - physical device number
* @param[in] project                - project Id.
* @param[in] unitType               - id of unit containig pizza arbiter
* @param[in] unitIndex              - index of unit containig pizza arbiter
* @param[in] channel                - local number of channel
* @param[in] portSpeed              - port speed
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad parameter value
* @retval GT_BAD_STATE             - other channel slots overlapped
*/
GT_STATUS prvCpssDxChTxPortSpeedPizzaResourcesChannelProfileSet
(
    IN    GT_U8                                     devNum,
    IN    PRV_CPSS_DXCH_PIZZA_ARBITER_PROJECT_ENT   project,
    IN    PRV_CPSS_DXCH_PIZZA_ARBITER_UNIT_ENT      unitType,
    IN    GT_U32                                    unitIndex,
    IN    GT_U32                                    channel,
    IN    CPSS_PORT_SPEED_ENT                       portSpeed
);

/**
* @internal prvCpssDxChTxPortSpeedPizzaResourcesTxDmaSpeedUpProfileConfigure function
* @endinternal
*
* @brief    Configure TX_DMA Speed Up profile for as modified pattern profile.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                 - physical device number
* @param[in] unitIndex              - index of unit
* @param[in] profileId              - Id of configured profile
* @param[in] patternProfileId       - Id of pattern profile
* @param[in] txDmaIdleRateIncrement - value of modified field Idle Rate Increment
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad parameter value
*/
GT_STATUS prvCpssDxChTxPortSpeedPizzaResourcesTxDmaSpeedUpProfileConfigure
(
    IN    GT_U8                                     devNum,
    IN    GT_U32                                    unitIndex,
    IN    GT_U32                                    profileId,
    IN    GT_U32                                    patternProfileId,
    IN    GT_U32                                    txDmaIdleRateIncrement
);

/**
* @internal prvCpssDxChTxPortSpeedPizzaResourcesPbGprSpeedUpProfileConfigure function
* @endinternal
*
* @brief    Configure PB_GPR Speed Up profile for as modified pattern profile.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                 - physical device number
* @param[in] unitIndex              - index of unit
* @param[in] profileId              - Id of configured profile
* @param[in] patternProfileId       - Id of pattern profile
* @param[in] pbGprShaperLeakValue  - value of modified field Shaper Leak Value
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad parameter value
*/
GT_STATUS prvCpssDxChTxPortSpeedPizzaResourcesPbGprSpeedUpProfileConfigure
(
    IN    GT_U8                                     devNum,
    IN    GT_U32                                    unitIndex,
    IN    GT_U32                                    profileId,
    IN    GT_U32                                    patternProfileId,
    IN    GT_U32                                    pbGprShaperLeakValue
);

/**
* @internal prvCpssDxChTxPortSpeedPizzaResourcesSpeedUpConfigure function
* @endinternal
*
* @brief    Configure speed up profiles for given speeed in DP units.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                 - physical device number
* @param[in] unitIndex              - Datapath index (index of units of differnt types)
* @param[in] speedUpCfgTablePtr     - (pointer to) table of speed up parameters per speed
* @param[in] portSpeed              - port speed
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad parameter value
*/
GT_STATUS prvCpssDxChTxPortSpeedPizzaResourcesSpeedUpConfigure
(
    IN    GT_U8                                                         devNum,
    IN    GT_U32                                                        unitIndex,
    IN    const PRV_CPSS_DXCH_PIZZA_ARBITER_SPEED_UP_PROFILE_CFG_STC    *speedUpCfgTablePtr,
    IN    CPSS_PORT_SPEED_ENT                                           portSpeed
);

/**
* @internal prvCpssDxChTxPortSpeedPizzaResourcesTxDmaProfileDump function
* @endinternal
*
* @brief    Dump TX_DMA profile.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                 - physical device number
* @param[in] unitIndex              - index of unit
* @param[in] profileId              - Id of profile
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad parameter value
*/
GT_STATUS prvCpssDxChTxPortSpeedPizzaResourcesTxDmaProfileDump
(
    IN    GT_U8                                     devNum,
    IN    GT_U32                                    unitIndex,
    IN    GT_U32                                    profileId
);

/**
* @internal prvCpssDxChTxPortSpeedPizzaResourcesPbGprProfileDump function
* @endinternal
*
* @brief    Dump PB_GPR profile.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                 - physical device number
* @param[in] unitIndex              - index of unit
* @param[in] profileId              - Id of profile
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad parameter value
*/
GT_STATUS prvCpssDxChTxPortSpeedPizzaResourcesPbGprProfileDump
(
    IN    GT_U8                                     devNum,
    IN    GT_U32                                    unitIndex,
    IN    GT_U32                                    profileId
);

/**
* @internal prvCpssDxChTxPortSpeedPizzaResourcesChannelReset function
* @endinternal
*
* @brief    Reset given channel.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                 - physical device number
* @param[in] project                - project Id.
* @param[in] unitType               - id of unit containig pizza arbiter
* @param[in] unitIndex              - index of unit containig pizza arbiter
* @param[in] channel                - local number of channel
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad parameter value
* @retval GT_BAD_STATE             - other channel slots overlapped
*/
GT_STATUS prvCpssDxChTxPortSpeedPizzaResourcesChannelReset
(
    IN    GT_U8                                     devNum,
    IN    PRV_CPSS_DXCH_PIZZA_ARBITER_PROJECT_ENT   project,
    IN    PRV_CPSS_DXCH_PIZZA_ARBITER_UNIT_ENT      unitType,
    IN    GT_U32                                    unitIndex,
    IN    GT_U32                                    channel
);

/**
* @internal prvCpssDxChTxPortSpeedPizzaResourcesInit function
* @endinternal
*
* @brief    Initialyze pizza arbiter and profiles of default channels and resets them.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in]  devNum                - physical device number
* @param[in] project                - project Id.
* @param[in] unitType               - id of unit containig pizza arbiter
* @param[in] unitIndex              - index of unit containig pizza arbiter
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad parameter value
* @retval GT_BAD_PTR               - on null pointer
*/
GT_STATUS prvCpssDxChTxPortSpeedPizzaResourcesInit
(
    IN    GT_U8                                     devNum,
    IN    PRV_CPSS_DXCH_PIZZA_ARBITER_PROJECT_ENT   project,
    IN    PRV_CPSS_DXCH_PIZZA_ARBITER_UNIT_ENT      unitType,
    IN    GT_U32                                    unitIndex
);

/**
* @internal prvCpssDxChTxPortSpeedPizzaResourcesDump function
* @endinternal
*
* @brief    Dump pizza arbiter and profiles of all channels.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in]  devNum                - physical device number
* @param[in] project                - project Id.
* @param[in] unitType               - id of unit containig pizza arbiter
* @param[in] unitIndex              - index of unit containig pizza arbiter
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad parameter value
* @retval GT_BAD_PTR               - on null pointer
*/
GT_STATUS prvCpssDxChTxPortSpeedPizzaResourcesDump
(
    IN    GT_U8                                     devNum,
    IN    PRV_CPSS_DXCH_PIZZA_ARBITER_UNIT_ENT      unitType,
    IN    GT_U32                                    unitIndex
);

/**
* @internal prvCpssDxChTxPortSpeedPizzaResourcesRxChannelSourcePortSet function
* @endinternal
*
* @brief   Set source physical port for the given RX Channel.
*
* @note   APPLICABLE DEVICES:     Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES: xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X  Lion2; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[in] portGlobalRxNum          - global Rx DMA port number including CPU port.
* @param[in] sorcePortNum             - source physical port number.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum or portNum
* @retval GT_OUT_OF_RANGE          - on wrong minCtByteCount
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS prvCpssDxChTxPortSpeedPizzaResourcesRxChannelSourcePortSet
(
    IN GT_U8                 devNum,
    IN GT_U32                portGlobalRxNum,
    IN GT_PHYSICAL_PORT_NUM  sorcePortNum
);

/**
* @internal prvCpssDxChTxPortSpeedPizzaResourcesPortMiscConfigure function
* @endinternal
*
* @brief    Configure miscelanous features for given port.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                 - physical device number
* @param[in] portNum                - physical port number
* @param[in] portSpeed              - port speed
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad parameter value
* @retval GT_BAD_STATE             - other channel slots overlapped
*/
GT_STATUS prvCpssDxChTxPortSpeedPizzaResourcesPortMiscConfigure
(
    IN    GT_U8                                devNum,
    IN    GT_PHYSICAL_PORT_NUM                 portNum,
    IN    CPSS_PORT_SPEED_ENT                  portSpeed
);

/**
* @internal prvCpssDxChTxPortSpeedPizzaResourcesPortMiscDown function
* @endinternal
*
* @brief    Power down miscelanous features for given port.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                 - physical device number
* @param[in] portNum                - physical port number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad parameter value
* @retval GT_BAD_STATE             - other channel slots overlapped
*/
GT_STATUS prvCpssDxChTxPortSpeedPizzaResourcesPortMiscDown
(
    IN    GT_U8                                devNum,
    IN    GT_PHYSICAL_PORT_NUM                 portNum
);

/* Full Units Init, DP channel Power up and Power down functions */

/**
* @internal prvCpssDxChTxPortSpeedPizzaResourcesDeviceGlobalInit function
* @endinternal
*
* @brief    Initialyze global resources of device.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in]  devNum                - physical device number
* @param[in] project                - project Id.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad parameter value
* @retval GT_BAD_PTR               - on null pointer
*/
GT_STATUS prvCpssDxChTxPortSpeedPizzaResourcesDeviceGlobalInit
(
    IN    GT_U8                                     devNum,
    IN    PRV_CPSS_DXCH_PIZZA_ARBITER_PROJECT_ENT   project
);

/**
* @internal prvCpssDxChTxPortSpeedPizzaResourcesDeviceDpInit function
* @endinternal
*
* @brief    Initialyze pizza arbiters and profiles for default channels for all DP units.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in]  devNum                - physical device number
* @param[in] project                - project Id.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad parameter value
* @retval GT_BAD_PTR               - on null pointer
*/
GT_STATUS prvCpssDxChTxPortSpeedPizzaResourcesDeviceDpInit
(
    IN    GT_U8                                     devNum,
    IN    PRV_CPSS_DXCH_PIZZA_ARBITER_PROJECT_ENT   project
);

/**
* @internal prvCpssDxChTxPortSpeedPizzaResourcesPortDpConfigure function
* @endinternal
*
* @brief    Configure pizzas and profile and reset channel in all units.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                 - physical device number
* @param[in] portNum                - physical port number
* @param[in] project                - project Id.
* @param[in] ifMode                 - Interface mode
*                                     not relevant SDMA, PCA units not configured
* @param[in] portSpeed              - port speed
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad parameter value
* @retval GT_BAD_STATE             - other channel slots overlapped
*/
GT_STATUS prvCpssDxChTxPortSpeedPizzaResourcesPortDpConfigure
(
    IN    GT_U8                                     devNum,
    IN    GT_PHYSICAL_PORT_NUM                      portNum,
    IN    PRV_CPSS_DXCH_PIZZA_ARBITER_PROJECT_ENT   project,
    IN    CPSS_PORT_INTERFACE_MODE_ENT              ifMode,
    IN    CPSS_PORT_SPEED_ENT                       portSpeed
);

/**
* @internal prvCpssDxChTxPortSpeedPizzaResourcesChannelDpConfigure function
* @endinternal
*
* @brief    Configure pizzas and profile and reset channel in all units.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                 - physical device number
* @param[in] unitIndex              - Datapath index (index of units of differnt types)
* @param[in] channel                - channel local number in Datapath
* @param[in] isPca                  - 0 - PCA units not used (SDMA), other PCA units used (network port)
* @param[in] portType               - port MAC type, relevant only for PCA ports (network ports)
* @param[in] preempionType          - channel preemption type
* @param[in] project                - project Id.
* @param[in] portSpeed              - port speed
* @param[in] usxPortMaxSpeed        - speed to configure all Pizzas and registers in PCA.
*                                     Maximal supported for USX port, for others same as portSpeed.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad parameter value
* @retval GT_BAD_STATE             - other channel slots overlapped
*/
GT_STATUS prvCpssDxChTxPortSpeedPizzaResourcesChannelDpConfigure
(
    IN    GT_U8                                      devNum,
    IN    GT_U32                                     unitIndex,
    IN    GT_U32                                     channel,
    IN    GT_U32                                     isPca,
    IN    PRV_CPSS_PORT_TYPE_ENT                     portType,
    IN    PRV_CPSS_DXCH_PIZZA_ARBITER_PREEMTION_ENT  preempionType,
    IN    PRV_CPSS_DXCH_PIZZA_ARBITER_PROJECT_ENT    project,
    IN    CPSS_PORT_SPEED_ENT                        portSpeed,
    IN    CPSS_PORT_SPEED_ENT                        usxPortMaxSpeed
);

/**
* @internal prvCpssDxChTxPortSpeedPizzaResourcesChannelDpDown function
* @endinternal
*
* @brief    Free pizzas slots of channel in all units.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                 - physical device number
* @param[in] unitIndex              - Datapath index (index of units of differnt types)
* @param[in] channel                - channel local number in Datapath
* @param[in] isPca                  - 0 - PCA units not used (SDMA), other PCA units used (network port)
* @param[in] project                - project Id.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad parameter value
* @retval GT_BAD_STATE             - other channel slots overlapped
*/
GT_STATUS prvCpssDxChTxPortSpeedPizzaResourcesChannelDpDown
(
    IN    GT_U8                                     devNum,
    IN    GT_U32                                    unitIndex,
    IN    GT_U32                                    channel,
    IN    GT_U32                                    isPca,
    IN    PRV_CPSS_DXCH_PIZZA_ARBITER_PROJECT_ENT   project
);

/**
* @internal prvCpssDxChTxPortSpeedPizzaResourcesPortDpDown function
* @endinternal
*
* @brief    Free pizzas slots of channel in all units.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                 - physical device number
* @param[in] project                - project Id.
* @param[in] portNum                - physical port number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad parameter value
* @retval GT_BAD_STATE             - other channel slots overlapped
*/
GT_STATUS prvCpssDxChTxPortSpeedPizzaResourcesPortDpDown
(
    IN    GT_U8                                     devNum,
    IN    GT_PHYSICAL_PORT_NUM                      portNum,
    IN    PRV_CPSS_DXCH_PIZZA_ARBITER_PROJECT_ENT   project
);

/**
* @internal prvCpssDxChTxPortSpeedPizzaResourcesPcaSffFifoWidthGet function
* @endinternal
*
* @brief    Get FIFO width of PCA SFF unit.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
*
* @param[in]  project               - project Id.
* @param[in]  unitIndex             - SFF unit index
* @param[in]  channel               - local number of channel
* @param[in]  portType              - MAC type/mode
* @param[in]  preempionType         - channel preemption type
* @param[in]  portSpeed             - port speed
* @param[out] fifo_widthPtr         - (pointer to) FIFO width
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad parameter value
*/
GT_STATUS prvCpssDxChTxPortSpeedPizzaResourcesPcaSffFifoWidthGet
(
    IN    PRV_CPSS_DXCH_PIZZA_ARBITER_PROJECT_ENT    project,
    IN    GT_U32                                     unitIndex,
    IN    GT_U32                                     channel,
    IN    PRV_CPSS_PORT_TYPE_ENT                     portType,
    IN    PRV_CPSS_DXCH_PIZZA_ARBITER_PREEMTION_ENT  preempionType,
    IN    CPSS_PORT_SPEED_ENT                        portSpeed,
    OUT   GT_U32                                     *fifo_widthPtr
);

/**
* @internal prvCpssDxChTxPortSpeedPizzaResourcesPcaSffMaxOfTokensGet function
* @endinternal
*
* @brief    Get Max of Tokens of PCA SFF unit.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
*
* @param[in]  project               - project Id.
* @param[in]  unitIndex             - SFF unit index
* @param[in]  channel               - local number of channel
* @param[in]  portType              - MAC type/mode
* @param[in]  portSpeed             - port speed
* @param[out] max_of_tokensPtr      - (pointer to) Max of Tokens
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad parameter value
*/
GT_STATUS prvCpssDxChTxPortSpeedPizzaResourcesPcaSffMaxOfTokensGet
(
    IN    PRV_CPSS_DXCH_PIZZA_ARBITER_PROJECT_ENT   project,
    IN    GT_U32                                    unitIndex,
    IN    GT_U32                                    channel,
    IN    PRV_CPSS_PORT_TYPE_ENT                    portType,
    IN    CPSS_PORT_SPEED_ENT                       portSpeed,
    OUT   GT_U32                                    *max_of_tokensPtr
);

/**
* @internal prvCpssDxChTxPortSpeedPizzaResourcesPcaSffMaxSlotsGet function
* @endinternal
*
* @brief    Get maximum slots for channel of PCA SFF unit.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
*
* @param[in]  project               - project Id.
* @param[in]  unitIndex             - SFF unit index
* @param[in]  channel               - local number of channel
* @param[in]  portSpeed             - port speed
* @param[out] maxSlotsPtr           - (pointer to)Max slots for channel
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad parameter value
*/
GT_STATUS prvCpssDxChTxPortSpeedPizzaResourcesPcaSffMaxSlotsGet
(
    IN    PRV_CPSS_DXCH_PIZZA_ARBITER_PROJECT_ENT   project,
    IN    GT_U32                                    unitIndex,
    IN    GT_U32                                    channel,
    IN    CPSS_PORT_SPEED_ENT                       portSpeed,
    OUT   GT_U32                                    *maxSlotsPtr
);

/**
* @internal prvCpssDxChTxPortSpeedPizzaResourcesPcaBrgTxCreditsAllocGet function
* @endinternal
*
* @brief    Get TX Credits Alloc PCA BRG unit.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
*
* @param[in]  project               - project Id.
* @param[in]  unitIndex             - SFF unit index
* @param[in]  channel               - local number of channel
* @param[in]  portSpeed             - port speed
* @param[out] brg_tx_creditsPtr      - (pointer to) TX Credits
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad parameter value
*/
GT_STATUS prvCpssDxChTxPortSpeedPizzaResourcesPcaBrgTxCreditsAllocGet
(
    IN    PRV_CPSS_DXCH_PIZZA_ARBITER_PROJECT_ENT   project,
    IN    GT_U32                                    unitIndex,
    IN    GT_U32                                    channel,
    IN    CPSS_PORT_SPEED_ENT                       portSpeed,
    OUT   GT_U32                                    *brg_tx_credits_allocPtr
);

/* PCA SFF unit */

/**
* @internal prvCpssDxChTxPortSpeedPizzaResourcesPcaSffChannelConfigure function
* @endinternal
*
* @brief    Configure Channel of Hawk PCA SFF unit.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in]  devNum                - physical device number
* @param[in]  project               - project Id.
* @param[in]  unitIndex             - SFF unit index
* @param[in]  channel               - local number of channel
* @param[in]  portType              - MAC type/mode
* @param[in]  preempionType         - channel preemption type
* @param[in]  portSpeed             - port speed
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad parameter value
* @retval GT_BAD_PTR               - on null pointer
*/
GT_STATUS prvCpssDxChTxPortSpeedPizzaResourcesPcaSffChannelConfigure
(
    IN    GT_U8                                      devNum,
    IN    PRV_CPSS_DXCH_PIZZA_ARBITER_PROJECT_ENT    project,
    IN    GT_U32                                     unitIndex,
    IN    GT_U32                                     channel,
    IN    PRV_CPSS_PORT_TYPE_ENT                     portType,
    IN    PRV_CPSS_DXCH_PIZZA_ARBITER_PREEMTION_ENT  preempionType,
    IN    CPSS_PORT_SPEED_ENT                        portSpeed
);

/**
* @internal prvCpssDxChTxPortSpeedPizzaResourcesPcaSffChannelDown function
* @endinternal
*
* @brief    Disable Channel of Hawk PCA SFF unit.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in]  devNum                - physical device number
* @param[in]  project               - project Id.
* @param[in]  unitIndex             - SFF unit index
* @param[in]  channel               - local number of channel
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad parameter value
* @retval GT_BAD_PTR               - on null pointer
*/
GT_STATUS prvCpssDxChTxPortSpeedPizzaResourcesPcaSffChannelDown
(
    IN    GT_U8                                     devNum,
    IN    PRV_CPSS_DXCH_PIZZA_ARBITER_PROJECT_ENT   project,
    IN    GT_U32                                    unitIndex,
    IN    GT_U32                                    channel
);

/**
* @internal prvCpssDxChTxPortSpeedPizzaResourcesPcaSffInit function
* @endinternal
*
* @brief    Initialyze Hawk PCA SFF unit.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in]  devNum                - physical device number
* @param[in]  project               - project Id.
* @param[in]  unitIndex             - SFF unit index
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad parameter value
* @retval GT_BAD_PTR               - on null pointer
*/
GT_STATUS prvCpssDxChTxPortSpeedPizzaResourcesPcaSffInit
(
    IN    GT_U8                                     devNum,
    IN    PRV_CPSS_DXCH_PIZZA_ARBITER_PROJECT_ENT   project,
    IN    GT_U32                                    unitIndex
);

/**
* @internal prvCpssDxChTxPortSpeedPizzaResourcesPcaSffDump function
* @endinternal
*
* @brief    Dump SFF Fifo of all channels.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in]  devNum                - physical device number
* @param[in] unitIndex              - index of unit containig pizza arbiter
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad parameter value
* @retval GT_BAD_PTR               - on null pointer
*/
GT_STATUS prvCpssDxChTxPortSpeedPizzaResourcesPcaSffDump
(
    IN    GT_U8                                     devNum,
    IN    GT_U32                                    unitIndex
);

/**
* @internal prvCpssDxChTxPortSpeedPizzaResourcesPcaBrgTxChannelConfigure function
* @endinternal
*
* @brief    Configure Channel of Hawk TX PCA BRG unit.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in]  devNum                - physical device number
* @param[in]  project               - project Id.
* @param[in]  unitIndex             - BRG unit index
* @param[in]  channel               - local number of channel
* @param[in]  portSpeed             - port speed
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad parameter value
* @retval GT_BAD_PTR               - on null pointer
*/
GT_STATUS prvCpssDxChTxPortSpeedPizzaResourcesPcaBrgTxChannelConfigure
(
    IN    GT_U8                                     devNum,
    IN    PRV_CPSS_DXCH_PIZZA_ARBITER_PROJECT_ENT   project,
    IN    GT_U32                                    unitIndex,
    IN    GT_U32                                    channel,
    IN    CPSS_PORT_SPEED_ENT                       portSpeed
);

/**
* @internal prvCpssDxChTxPortSpeedPizzaResourcesPcaBrgRxChannelConfigure function
* @endinternal
*
* @brief    Configure Channel of Hawk PCA BRG RX unit.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in]  devNum                - physical device number
* @param[in]  project               - project Id.
* @param[in]  unitIndex             - BRG unit index
* @param[in]  channel               - local number of channel
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad parameter value
* @retval GT_BAD_PTR               - on null pointer
*/
GT_STATUS prvCpssDxChTxPortSpeedPizzaResourcesPcaBrgRxChannelConfigure
(
    IN    GT_U8                                     devNum,
    IN    PRV_CPSS_DXCH_PIZZA_ARBITER_PROJECT_ENT   project,
    IN    GT_U32                                    unitIndex,
    IN    GT_U32                                    channel
);

/**
* @internal prvCpssDxChTxPortSpeedPizzaResourcesPcaBrgTxChannelDown function
* @endinternal
*
* @brief    Disable Channel of PCA BRG TX unit.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in]  devNum                - physical device number
* @param[in]  project               - project Id.
* @param[in]  unitIndex             - BRG unit index
* @param[in]  channel               - local number of channel
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad parameter value
* @retval GT_BAD_PTR               - on null pointer
*/
GT_STATUS prvCpssDxChTxPortSpeedPizzaResourcesPcaBrgTxChannelDown
(
    IN    GT_U8                                     devNum,
    IN    PRV_CPSS_DXCH_PIZZA_ARBITER_PROJECT_ENT   project,
    IN    GT_U32                                    unitIndex,
    IN    GT_U32                                    channel
);

/**
* @internal prvCpssDxChTxPortSpeedPizzaResourcesPcaBrgRxChannelDown function
* @endinternal
*
* @brief    Disable Channel of PCA BRG RX unit.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in]  devNum                - physical device number
* @param[in]  project               - project Id.
* @param[in]  unitIndex             - BRG unit index
* @param[in]  channel               - local number of channel
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad parameter value
* @retval GT_BAD_PTR               - on null pointer
*/
GT_STATUS prvCpssDxChTxPortSpeedPizzaResourcesPcaBrgRxChannelDown
(
    IN    GT_U8                                     devNum,
    IN    PRV_CPSS_DXCH_PIZZA_ARBITER_PROJECT_ENT   project,
    IN    GT_U32                                    unitIndex,
    IN    GT_U32                                    channel
);

/**
* @internal prvCpssDxChTxPortSpeedPizzaResourcesPcaBrgTxInit function
* @endinternal
*
* @brief    Initialyze PCA BRG TX unit.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in]  devNum                - physical device number
* @param[in]  project               - project Id.
* @param[in]  unitIndex             - BRG unit index
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad parameter value
* @retval GT_BAD_PTR               - on null pointer
*/
GT_STATUS prvCpssDxChTxPortSpeedPizzaResourcesPcaBrgTxInit
(
    IN    GT_U8                                     devNum,
    IN    PRV_CPSS_DXCH_PIZZA_ARBITER_PROJECT_ENT   project,
    IN    GT_U32                                    unitIndex
);

/**
* @internal prvCpssDxChTxPortSpeedPizzaResourcesPcaBrgRxInit function
* @endinternal
*
* @brief    Initialyze PCA BRG RX unit.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in]  devNum                - physical device number
* @param[in]  project               - project Id.
* @param[in]  unitIndex             - BRG unit index
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad parameter value
* @retval GT_BAD_PTR               - on null pointer
*/
GT_STATUS prvCpssDxChTxPortSpeedPizzaResourcesPcaBrgRxInit
(
    IN    GT_U8                                     devNum,
    IN    PRV_CPSS_DXCH_PIZZA_ARBITER_PROJECT_ENT   project,
    IN    GT_U32                                    unitIndex
);

/* HAWK specific */

/**
* @internal prvCpssDxChTxPortSpeedPizzaResourcesPcaBrgTxDump function
* @endinternal
*
* @brief    Dump BRG TX all channels.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in]  devNum                - physical device number
* @param[in] unitIndex              - index of unit
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad parameter value
* @retval GT_BAD_PTR               - on null pointer
*/
GT_STATUS prvCpssDxChTxPortSpeedPizzaResourcesPcaBrgTxDump
(
    IN    GT_U8                                     devNum,
    IN    GT_U32                                    unitIndex
);

/**
* @internal prvCpssDxChTxPortSpeedPizzaResourcesPcaBrgRxDump function
* @endinternal
*
* @brief    Dump BRG RX all channels.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in]  devNum                - physical device number
* @param[in] unitIndex              - index of unit
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad parameter value
* @retval GT_BAD_PTR               - on null pointer
*/
GT_STATUS prvCpssDxChTxPortSpeedPizzaResourcesPcaBrgRxDump
(
    IN    GT_U8                                     devNum,
    IN    GT_U32                                    unitIndex
);

/**
* @internal prvCpssDxChTxPortSpeedPizzaResourcesDeviceInit function
* @endinternal
*
* @brief    Initialyze Hawk DP units.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in]  devNum                - physical device number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad parameter value
* @retval GT_BAD_PTR               - on null pointer
*/
GT_STATUS prvCpssDxChTxPortSpeedPizzaResourcesDeviceInit
(
    IN    GT_U8                                devNum
);

/**
* @internal prvCpssDxChTxPortSpeedPizzaResourcesPortConfigure function
* @endinternal
*
* @brief    Configure all DP units for given port.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                 - physical device number
* @param[in] portNum                - physical port number
* @param[in] ifMode                 - Interface mode
*                                     not relevant SDMA, PCA units not configured
* @param[in] portSpeed              - port speed
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad parameter value
* @retval GT_BAD_STATE             - other channel slots overlapped
*/
GT_STATUS prvCpssDxChTxPortSpeedPizzaResourcesPortConfigure
(
    IN    GT_U8                                devNum,
    IN    GT_PHYSICAL_PORT_NUM                 portNum,
    IN    CPSS_PORT_INTERFACE_MODE_ENT         ifMode,
    IN    CPSS_PORT_SPEED_ENT                  portSpeed
);

/**
* @internal prvCpssDxChTxPortSpeedPizzaResourcesPortDown function
* @endinternal
*
* @brief    Free pizzas slots of port in all units.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                 - physical device number
* @param[in] portNum                - physical port number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad parameter value
* @retval GT_BAD_STATE             - other channel slots overlapped
*/
GT_STATUS prvCpssDxChTxPortSpeedPizzaResourcesPortDown
(
    IN    GT_U8                                devNum,
    IN    GT_PHYSICAL_PORT_NUM                 portNum
);

/**
* @internal prvCpssDxChTxPortSpeedPizzaResourcesChannelSpeedByTxDmaProfileGet function
* @endinternal
*
* @brief    Get channel and speed by Tx DMA profile.
*           Not exact - can be used only for speed save and restore in tests.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                 - physical device number
* @param[in] project                - project Id.
* @param[in] unitIndex              - index of unit containig pizza arbiter
* @param[in] channel                - local number of channel
* @param[out] portSpeed             - pointer to port speed
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad parameter value
* @retval GT_BAD_STATE             - convertion profile id to speed failed
*/
GT_STATUS prvCpssDxChTxPortSpeedPizzaResourcesChannelSpeedByTxDmaProfileGet
(
    IN    GT_U8                                     devNum,
    IN    PRV_CPSS_DXCH_PIZZA_ARBITER_PROJECT_ENT   project,
    IN    GT_U32                                    unitIndex,
    IN    GT_U32                                    channel,
    OUT   CPSS_PORT_SPEED_ENT                       *portSpeedPtr
);

/*****************************************************************************/

/**
* @internal prvCpssDxChTxPortSpeedPizzaResourcesPcaTxArbiterReinitEmpty function
* @endinternal
*
* @brief    Reinitialize PCA TX pizza arbiter - all slots invalid.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; Ironman.
*
* @param[in] devNum                 - physical device number
* @param[in] unitIndex              - index of PCA TX Arbier unit
* @param[in] usedSlots              - amount of slots used by PCA TX pizza arbiter
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad parameter value
* @retval GT_BAD_PTR               - on null pointer
*/
GT_STATUS prvCpssDxChTxPortSpeedPizzaResourcesPcaTxArbiterReinitEmpty
(
    IN    GT_U8                                     devNum,
    IN    GT_U32                                    unitIndex,
    IN    GT_U32                                    usedSlots
);

/**
* @internal prvCpssDxChTxPortSpeedPizzaResourcesPcaTxArbiterChannelSlotsSetCyclic function
* @endinternal
*
* @brief    Set slots of PCA Tx arbiter by given channel and slots cyclic sequence.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum              - physical device number
* @param[in] unitIndex           - index of PCA TX pizza arbiter
* @param[in] channel             - local number of channel
* @param[in] startSlot           - start slot of the sequence
* @param[in] slotsStep           - step of the sequence in slots
* @param[in] slotsAmount         - amount of slots of the sequence
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad parameter value
*/
GT_STATUS prvCpssDxChTxPortSpeedPizzaResourcesPcaTxArbiterChannelSlotsSetCyclic
(
    IN    GT_U8                                     devNum,
    IN    GT_U32                                    unitIndex,
    IN    GT_U32                                    channel,
    IN    GT_U32                                    startSlot,
    IN    GT_U32                                    slotsStep,
    IN    GT_U32                                    slotsAmount
);

/**
* @internal prvCpssDxChTxPortSpeedPizzaResourcesPcaTxArbiterRestore function
* @endinternal
*
* @brief    Restore PCA TX pizza arbiter - all channel-mapped slots restored.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; Ironman.
*
* @param[in] devNum                 - physical device number
* @param[in] unitIndex              - index of PCA TX Arbier unit
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad parameter value
* @retval GT_BAD_PTR               - on null pointer
*/
GT_STATUS prvCpssDxChTxPortSpeedPizzaResourcesPcaTxArbiterRestore
(
    IN    GT_U8                                     devNum,
    IN    GT_U32                                    unitIndex
);

/**
* @internal prvCpssDxChTxPortSpeedPizzaResourcesPcaTxArbiterChannelSlotsBySpeedSet function
* @endinternal
*
* @brief    Set slots of PCA Tx arbiter by given channel and speed.
*
* @note   APPLICABLE DEVICES:      Harrier.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*                                  Falcon; AC5P; AC5X; Ironman.
*
* @param[in] devNum                 - physical device number
* @param[in] unitIndex              - index of PCA TX pizza arbiter
* @param[in] channel                - local number of channel
* @param[in] speedInMbps            - speed in mega-bits per second.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad parameter value
* @retval GT_BAD_STATE             - other channel slots overlapped
* @retval GT_NOT_APPLICABLE_DEVICE - not applicable device
*/
GT_STATUS prvCpssDxChTxPortSpeedPizzaResourcesPcaTxArbiterChannelSlotsBySpeedSet
(
    IN    GT_U8   devNum,
    IN    GT_U32  unitIndex,
    IN    GT_U32  channel,
    IN    GT_U32  speedInMbps
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /*__prvCpssDxChPortTxPizzaResourceHawk_h*/

