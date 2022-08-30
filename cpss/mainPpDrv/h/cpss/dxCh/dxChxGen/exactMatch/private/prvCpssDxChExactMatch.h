/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
*
*/
/**
********************************************************************************
* @file prvCpssDxChExactMatch.h
*
* @brief Common private Exact Match declarations.
*
* @version   1
********************************************************************************
*/
#ifndef __prvCpssDxChExactMatchh
#define __prvCpssDxChExactMatchh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#include <cpss/common/cpssTypes.h>
#include <cpss/dxCh/dxChxGen/exactMatch/cpssDxChExactMatch.h>

/* min/max profileId value */
#define PRV_CPSS_DXCH_EXACT_MATCH_MIN_PROFILE_ID_VAL_CNS                    1
#define PRV_CPSS_DXCH_EXACT_MATCH_MAX_PROFILE_ID_VAL_CNS                    15

/* min/max expander action index value */
#define PRV_CPSS_DXCH_EXACT_MATCH_MIN_EXPANDED_ACTION_INDEX_VAL_CNS         0
#define PRV_CPSS_DXCH_EXACT_MATCH_MAX_EXPANDED_ACTION_INDEX_VAL_CNS         15

#define PRV_CPSS_DXCH_EXACT_MATCH_EXPANDED_ACTION_NUM_OF_ENTRIES_VAL_CNS    16

/* the size of an action in words */
#define PRV_CPSS_DXCH_EXACT_MATCH_ACTION_SIZE_CNS                           8

/* the max size of exact match entry in words 4 banks each with 115 bits
   each bank wide is 4 words */
#define PRV_CPSS_DXCH_EXACT_MATCH_ENTRY_MAX_SIZE_CNS                        16

/* the size of reduced action in bytes */
#define PRV_CPSS_DXCH_EXACT_MATCH_REDUCED_ACTION_SIZE_CNS                   8

/* each bank wide is 4 words */
#define PRV_CPSS_DXCH_EXACT_MATCH_ENTRY_BANK_SIZE_IN_WORDS_CNS              4

/* the size of default action in words */
#define PRV_CPSS_DXCH_EXACT_MATCH_DEFAULT_ACTION_SIZE_CNS                   8

/* max number of port groups */
#define PRV_CPSS_DXCH_EXACT_MATCH_MAX_NUM_PORT_GROUPS_CNS                   32

/* macro to validate the value of profile ID value */
#define PRV_CPSS_DXCH_EXACT_MATCH_CHECK_PROFILE_ID_VALUE_MAC(profileId)      \
    if(((profileId) < PRV_CPSS_DXCH_EXACT_MATCH_MIN_PROFILE_ID_VAL_CNS) ||   \
       ((profileId) > PRV_CPSS_DXCH_EXACT_MATCH_MAX_PROFILE_ID_VAL_CNS))     \
    {                                                                        \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);       \
    }

/* macro to validate the value of expanded action index */
#define PRV_CPSS_DXCH_EXACT_MATCH_CHECK_EXPANDED_ACTION_INDEX_VALUE_MAC(expandedActionIndex)    \
    if((expandedActionIndex) > PRV_CPSS_DXCH_EXACT_MATCH_MAX_EXPANDED_ACTION_INDEX_VAL_CNS)     \
    {                                                                                           \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);                          \
    }

#define PRV_CPSS_DXCH_EXACT_MATCH_INDEX_CHECK_MAC(devNum,index)                     \
    if(index >= PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.tableSize.emNum)           \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE,                              \
            "PRV_CPSS_DXCH_EXACT_MATCH_INDEX_CHECK_MAC: Device %d, EXACT MATCH index %d is out of range\n", devNum, index);

/* Exact Match number of banks  (4,8,16)  */
#define PRV_CPSS_DXCH_EXACT_MATCH_NUM_OF_BANKS_GET_MAC(dev,swValue)    \
     swValue = PRV_CPSS_DXCH_PP_MAC(dev)->exactMatchInfo.exactMatchNumOfBanks

/* Converts key size to number of Exact Match banks value */
#define PRV_CPSS_DXCH_EXACT_MATCH_CONVERT_KEY_SIZE_TO_NUM_OF_BANKS_VAL_MAC(_numOfBanks, _keySize)\
    switch (_keySize)                                                                   \
    {                                                                                   \
        case CPSS_DXCH_EXACT_MATCH_KEY_SIZE_5B_E:                                       \
            _numOfBanks = 1;                                                            \
            break;                                                                      \
        case CPSS_DXCH_EXACT_MATCH_KEY_SIZE_19B_E:                                      \
            _numOfBanks = 2;                                                            \
            break;                                                                      \
        case CPSS_DXCH_EXACT_MATCH_KEY_SIZE_33B_E:                                      \
            _numOfBanks = 3;                                                            \
            break;                                                                      \
        case CPSS_DXCH_EXACT_MATCH_KEY_SIZE_47B_E:                                      \
            _numOfBanks = 4;                                                            \
            break;                                                                      \
        default:                                                                        \
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);              \
    }

/* Converts number of Exact Match banks to key size value */
#define PRV_CPSS_DXCH_EXACT_MATCH_CONVERT_NUM_OF_BANKS_TO_KEY_SIZE_VAL_MAC(_keySize, _numOfBanks)\
    switch (_numOfBanks)                                                                \
    {                                                                                   \
        case 1:                                                                         \
            _keySize = CPSS_DXCH_EXACT_MATCH_KEY_SIZE_5B_E;                             \
            break;                                                                      \
        case 2:                                                                         \
            _keySize = CPSS_DXCH_EXACT_MATCH_KEY_SIZE_19B_E;                            \
            break;                                                                      \
        case 3:                                                                         \
            _keySize = CPSS_DXCH_EXACT_MATCH_KEY_SIZE_33B_E;                            \
            break;                                                                      \
        case 4:                                                                         \
            _keySize = CPSS_DXCH_EXACT_MATCH_KEY_SIZE_47B_E;                            \
            break;                                                                      \
        default:                                                                        \
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_VALUE, LOG_ERROR_NO_MSG);              \
    }

/* Converts key size to HW value */
#define PRV_CPSS_DXCH_EXACT_MATCH_CONVERT_KEY_SIZE_TO_HW_VAL_MAC(_hwVal, _keySize)      \
    switch (_keySize)                                                                   \
    {                                                                                   \
        case CPSS_DXCH_EXACT_MATCH_KEY_SIZE_5B_E:                                       \
            _hwVal = 0;                                                                 \
            break;                                                                      \
        case CPSS_DXCH_EXACT_MATCH_KEY_SIZE_19B_E:                                      \
            _hwVal = 1;                                                                 \
            break;                                                                      \
        case CPSS_DXCH_EXACT_MATCH_KEY_SIZE_33B_E:                                      \
            _hwVal = 2;                                                                 \
            break;                                                                      \
        case CPSS_DXCH_EXACT_MATCH_KEY_SIZE_47B_E:                                      \
            _hwVal = 3;                                                                 \
            break;                                                                      \
        default:                                                                        \
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);              \
    }

/* Converts HW value key size value */
#define PRV_CPSS_DXCH_EXACT_MATCH_CONVERT_HW_TO_KEY_SIZE_VAL_MAC(_keySize, _hwVal)      \
    switch (_hwVal)                                                                     \
    {                                                                                   \
        case 0:                                                                         \
            _keySize = CPSS_DXCH_EXACT_MATCH_KEY_SIZE_5B_E;                             \
            break;                                                                      \
        case 1:                                                                         \
            _keySize = CPSS_DXCH_EXACT_MATCH_KEY_SIZE_19B_E;                            \
            break;                                                                      \
        case 2:                                                                         \
            _keySize = CPSS_DXCH_EXACT_MATCH_KEY_SIZE_33B_E;                            \
            break;                                                                      \
        case 3:                                                                         \
            _keySize = CPSS_DXCH_EXACT_MATCH_KEY_SIZE_47B_E;                            \
            break;                                                                      \
        default:                                                                        \
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_VALUE, LOG_ERROR_NO_MSG);              \
    }

/**
* @struct PRV_CPSS_DXCH_EXACT_MATCH_EXPANDER_INFO_STC
 *
*  @brief Structure represent the DB of the CPSS for exact match
*  Expander Table in that device INFO "PER DEVICE"
*/
typedef struct{

    /** @brief entry index
     */
    GT_U32 expandedActionValid;

    /** @brief entry index
     */
    GT_U32 expandedActionIndex;

    /** @brief the action type TTI/PCL/EPCL
     */
    CPSS_DXCH_EXACT_MATCH_ACTION_TYPE_ENT   actionType;

    /** @brief the action data to be used in case the useExpanded
     *         flag is false.
     */
    CPSS_DXCH_EXACT_MATCH_ACTION_UNT    actionData;

    /** @brief if useExpanded of a field is true then the data will
     *         be taken from actionData, else it will be taken from
     *         the reduced entry in the Exact Match Table
     */
    CPSS_DXCH_EXACT_MATCH_EXPANDED_ACTION_ORIGIN_UNT expandedActionOrigin;

    /** @brief Reduced mask to be used when configuring the Exact
     *         Match Entry.
     */
    GT_U32   reducedMaskArr[PRV_CPSS_DXCH_EXACT_MATCH_ACTION_SIZE_CNS];

    /** @brief Specifies the bit offset of the Flow-ID field in the EM compressed action.
     *        The Flow-ID is 16 bits long. This offset specifies where the beginning of the Flow-ID resides.
     *        This field should be configured to a value that is less than or equal to 48.
     *        (APPLICABLE DEVICES: AC5P; AC5X; Harrier; Ironman)
     */
    GT_U32  flowIdBitOffset;
} PRV_CPSS_DXCH_EXACT_MATCH_EXPANDER_INFO_STC;

/**
* @struct PRV_CPSS_DXCH_EXACT_MATCH_DB_INFO_PER_PORT_GROUP_STC
 *
*  @brief structure that holds Exact Match info per port group.
*        (APPLICABLE DEVICES: AC5P; AC5X; Harrier; Ironman)
*/
typedef struct{

    /** @brief counter that hold the number of profile id's
     *   enabled for auto learning.
     */
    GT_U32 numAutoLearningEnablePerProfileId;

    /** @brief boolean that hold whether the user read first auto
     *         learned entry.
     */
    GT_BOOL isFirstPointer;

    /** @brief pointer to last read Flow ID by application.
     */
    GT_U32 flowIdPointer;

} PRV_CPSS_DXCH_EXACT_MATCH_DB_INFO_PER_PORT_GROUP_STC;

/**
* @struct PRV_CPSS_DXCH_EXACT_MATCH_DB_INFO_STC
 *
*  @brief Structure represent the DB of the CPSS for exact match
*  in that device INFO "PER DEVICE" and "PER PORT GROUP".
*/
typedef struct{

    /** @brief init should be done only once
     */
    GT_BOOL initDone;

    /** @brief array with 16 elements for each Entry in the
     *  Expander Action Table
     */
    PRV_CPSS_DXCH_EXACT_MATCH_EXPANDER_INFO_STC  ExpanderInfoArr[PRV_CPSS_DXCH_EXACT_MATCH_EXPANDED_ACTION_NUM_OF_ENTRIES_VAL_CNS];

    /** @brief holds the number of Exact Match banks
     */
    GT_U32 exactMatchNumOfBanks;

    /** @brief structure that holds Exact Match info per port group
     *         (APPLICABLE DEVICES: AC5P; AC5X; Harrier; Ironman)
     */
    PRV_CPSS_DXCH_EXACT_MATCH_DB_INFO_PER_PORT_GROUP_STC  exactMatchInfoPerPortGroup[PRV_CPSS_DXCH_EXACT_MATCH_MAX_NUM_PORT_GROUPS_CNS];

} PRV_CPSS_DXCH_EXACT_MATCH_DB_INFO_STC;

/**
* @internal prvCpssDxChExpandedActionToHwformat function
* @endinternal
*
* @brief   Convert SW configuration to HW values.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum;
*                                  Aldrin; AC3X; Lion2; Bobcat3;
*                                  Aldrin2.
*
* @param[in] devNum            - device number
* @param[in] expandedActionIndex -Exact Match Expander table
*                               index (APPLICABLE RANGES:0..15)
* @param[in] actionType        - Exact Match Action Type
* @param[in] actionPtr         - (pointer to)Exact Match Action
* @param[in] expandedActionOriginPtr - (pointer to) Whether to use
*                   the action attributes from the Exact Match rule
*                   action or from the profile
* @param[out] hwValuesArr      - values to set to HW
* @param[out] hwReducedMaskArr - reduce entry mask to keep in DB
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong param
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChExpandedActionToHwformat
(
    IN  GT_U8                                               devNum,
    IN GT_U32                                               expandedActionIndex,
    IN  CPSS_DXCH_EXACT_MATCH_ACTION_TYPE_ENT               actionType,
    IN  CPSS_DXCH_EXACT_MATCH_ACTION_UNT                    *actionPtr,
    IN  CPSS_DXCH_EXACT_MATCH_EXPANDED_ACTION_ORIGIN_UNT    *expandedActionOriginPtr,
    OUT GT_U32                                              hwValuesArr[],
    OUT GT_U32                                              hwReducedMaskArr[]
);

/**
* @internal prvCpssDxChExactMatchDbInit function
* @endinternal
*
* @brief   the function init Exact Match Shadow DB
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum;
*                                  Aldrin; AC3X; Lion2; Bobcat3;
*                                  Aldrin2.
*
* @param[in] devNum            - device number
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong param
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChExactMatchDbInit
(
    IN GT_U8                                            devNum
);

/**
* @internal prvCpssDxChCfgEmSizeSet function
* @endinternal
*
* @brief   function to set the Exact Match size.
*
*   NOTE: function based on prvCpssDxChBrgFdbSizeSet(...)
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number.
* @param[in] emSize                   - the Exact Match size to set.
*                                      (each device with it's own relevant values)
*                                       NOTE: value 0 is supported.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on bad device or Exact Match table size
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChCfgEmSizeSet
(
    IN  GT_U8   devNum,
    IN  GT_U32  emSize
);
/**
* @internal prvCpssDxChCfgEmMhtSet function
* @endinternal
*
* @brief   function to set the Exact Match MHT (Number of Multiple Hash Tables).
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number.
* @param[in] mht                      - the Exact Match MHT (Number of Multiple Hash Tables)
*                                       4/8/16
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on bad device or Exact Match table size
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChCfgEmMhtSet
(
    IN  GT_U8   devNum,
    IN  GT_U32  mht
);

/**
* @internal prvCpssDxChTtiActionType2Logic2HwFormat function
* @endinternal
*
* @brief   Converts a given tti action type 2 from logic format to hardware format.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @param[in] devNum                   - device number
* @param[in] logicFormatPtr           - points to tti action in logic format
* @param[in] ttiUseExpandedPtr        -  points to tti use
*                                        Expander flags
*
* @param[out] hwFormatArray            - tti action in hardware format (5 words)
* @param[out] exactMatchReducedHwMaskArray  - tti reduced action
*       mask used for exact match entry (5 words)
* @param[out] flowIdLocationByteStartPtr - points to byte location of flow-id
*                                          in reduced action per client type
* @param[out] flowIdFoundPtr             - points to whether flow-id was configured
*                                          in reduced action
* @param[out] flowIdLocationBitStartInBytePtr - point to bit location in byte of
*                                               flow-id in reduced action per client type
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter.
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS prvCpssDxChTtiActionType2Logic2HwFormatAndExactMatchReducedMask
(
    IN  GT_U8                                                devNum,
    IN  CPSS_DXCH_TTI_ACTION_STC                             *logicFormatPtr,
    IN  CPSS_DXCH_EXACT_MATCH_EXPANDED_TTI_ACTION_ORIGIN_STC *ttiUseExpandedPtr,
    OUT GT_U32                                               *hwFormatArray,
    OUT GT_U32                                               *exactMatchReducedHwMaskArray,
    OUT GT_U32                                               *flowIdLocationByteStartPtr,
    OUT GT_BOOL                                              *flowIdFoundPtr,
    OUT GT_U32                                               *flowIdLocationBitStartInBytePtr
);

/**
* @internal prvCpssDxChTtiActionType2Logic2HwFormat function
* @endinternal
*
* @brief   Converts a given tti action type 2 from logic format to hardware format.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @param[in] devNum                   - device number
* @param[in] logicFormatPtr           - points to tti action in logic format
*
* @param[out] hwFormatArray            - tti action in hardware format (5 words)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter.
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS prvCpssDxChTtiActionType2Logic2HwFormat
(
    IN  GT_U8                               devNum,
    IN  CPSS_DXCH_TTI_ACTION_STC           *logicFormatPtr,
    OUT GT_U32                             *hwFormatArray
);

/**
* @internal prvCpssDxChTtiActionType2Hw2LogicFormat function
* @endinternal
*
* @brief   Converts a given tti action type 2 from hardware
*          format to logic format.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @param[in] devNum          - device number
* @param[in] hwFormatArray   - tti action in hardware format
*                              (5 words)
*
* @param[out] logicFormatPtr - points to tti action in logic
*                              format
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter.
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS prvCpssDxChTtiActionType2Hw2LogicFormat
(
    IN  GT_U8                               devNum,
    IN  GT_U32                             *hwFormatArray,
    OUT CPSS_DXCH_TTI_ACTION_STC         *logicFormatPtr
);

/**
* @internal prvCpssDxChPclActionLogic2HwFormatAndExactMatchReducedMask
*           function
* @endinternal
*
* @brief   Converts a given pcl action from logic format to
*          hardware format.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2;
*         Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*
* @param[in] devNum                     - device number
* @param[in] logicFormatPtr             - points to pcl action in logic
*                                       format
* @param[in] pclUseExpandedPtr          - points to pcl use Expander
*                                         flags
* @param[out] hwFormatArray            - pcl action in hardware
*                                        format (5 words)
* @param[out] exactMatchReducedHwMaskArray  - pcl reduced action
*                                       mask used for exact match entry (5 words)
* @param[out] flowIdLocationByteStartPtr - points to byte location of flow-id
*                                          in reduced action per client type
* @param[out] flowIdFoundPtr             - points to whether flow-id was configured
*                                          in reduced action
* @param[out] flowIdLocationBitStartInBytePtr - point to bit location in byte of
*                                               flow-id in reduced action per client type
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter.
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS prvCpssDxChPclActionLogic2HwFormatAndExactMatchReducedMask
(
    IN  GT_U8                                                devNum,
    IN  CPSS_DXCH_PCL_ACTION_STC                             *logicFormatPtr,
    IN  CPSS_DXCH_EXACT_MATCH_EXPANDED_PCL_ACTION_ORIGIN_STC *pclUseExpandedPtr,
    OUT GT_U32                                               *hwFormatArray,
    OUT GT_U32                                               *exactMatchReducedHwMaskArray,
    OUT GT_U32                                               *flowIdLocationByteStartPtr,
    OUT GT_BOOL                                              *flowIdFoundPtr,
    OUT GT_U32                                               *flowIdLocationBitStartInBytePtr
);

/**
* @internal prvCpssDxChExactMatchPortGroupTtiProfileIdModePacketTypeSet function
* @endinternal
*
* @brief   Sets the Exact Match Profile Id for TTI keyType
*
* NOTE: Client lookup for given ttiLookupNum MUST be configured
*       before this API is called (cpssDxChExactMatchClientLookupSet).
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2;
*         Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*
* @param[in] devNum         - device number
* @param[in] portGroupsBmp  - bitmap of Port Groups.
*                             NOTEs:
*                             1. for non multi-port groups device this parameter is IGNORED.
*                             2. for multi-port groups device :
*                             (APPLICABLE DEVICES  Falcon; AC5P; AC5X; Harrier; Ironman)
*                             bitmap must be set with at least one bit representing
*                             valid port group(s). If a bit of non valid port group
*                             is set then function returns GT_BAD_PARAM.
*                             value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] keyType        - TTI key type
* @param[in] ttiLookupNum   - TTI lookup num
* @param[in] enableExactMatchLookup- enable Exact Match lookup
*                           GT_TRUE: trigger Exact Match lookup.
*                           GT_FALSE: Do not trigger Exact Match
*                           lookup.
* @param[in] profileId    - Exact Match profile identifier
*                           (APPLICABLE RANGES: 1..15)
* @retval GT_OK                 - on success
* @retval GT_HW_ERROR           - on hardware error
* @retval GT_OUT_OF_RANGE       - parameter not in valid range.
* @retval GT_BAD_PARAM          - on wrong param
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* NOTE: legacy key type not supported:
*       CPSS_DXCH_TTI_KEY_IPV4_E,
*       CPSS_DXCH_TTI_KEY_MPLS_E,
*       CPSS_DXCH_TTI_KEY_ETH_E,
*       CPSS_DXCH_TTI_KEY_MIM_E
*
*/
GT_STATUS prvCpssDxChExactMatchPortGroupTtiProfileIdModePacketTypeSet
(
    IN  GT_U8                               devNum,
    IN  GT_PORT_GROUPS_BMP                  portGroupsBmp,
    IN  CPSS_DXCH_TTI_KEY_TYPE_ENT          keyType,
    IN  CPSS_DXCH_EXACT_MATCH_LOOKUP_ENT    ttiLookupNum,
    IN  GT_BOOL                             enableExactMatchLookup,
    IN  GT_U32                              profileId
);
/**
* @internal prvCpssDxChExactMatchPortGroupTtiProfileIdModePacketTypeGet function
* @endinternal
*
* @brief   Gets the  Exact Match Profile Id from TTI keyType
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2;
*         Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*
* @param[in] devNum         - device number
* @param[in] portGroupsBmp  - bitmap of Port Groups.
*                             NOTEs:
*                             1. for non multi-port groups device this parameter is IGNORED.
*                             2. for multi-port groups device :
*                             (APPLICABLE DEVICES  Falcon)
*                             bitmap must be set with at least one bit representing
*                             valid port group(s). If a bit of non valid port group
*                             is set then function returns GT_BAD_PARAM.
*                             value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] keyType        - TTI key type
* @param[in] ttiLookupNum   - TTI lookup num
* @param[out] enableExactMatchLookupPtr- (pointer to)enable
*                           Exact Match lookup
*                           GT_TRUE: trigger Exact Match lookup.
*                           GT_FALSE: Do not trigger Exact Match
*                           lookup.
* @param[out] profileIdPtr  - (pointer to) Exact Match profile
*                             identifier
*                           (APPLICABLE RANGES: 1..15)
* @retval GT_OK                  - on success
* @retval GT_HW_ERROR            - on hardware error
* @retval GT_OUT_OF_RANGE        - parameter not in valid range.
* @retval GT_BAD_PARAM           - on wrong param
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChExactMatchPortGroupTtiProfileIdModePacketTypeGet
(
    IN  GT_U8                               devNum,
    IN  GT_PORT_GROUPS_BMP                  portGroupsBmp,
    IN  CPSS_DXCH_TTI_KEY_TYPE_ENT          keyType,
    IN  CPSS_DXCH_EXACT_MATCH_LOOKUP_ENT    ttiLookupNum,
    OUT GT_BOOL                             *enableExactMatchLookupPtr,
    OUT GT_U32                              *profileIdPtr
);

/**
* @internal prvCpssDxChExactMatchPortGroupPclProfileIdSet function
* @endinternal
*
* @brief  Sets the Exact Match Profile Id for PCL packet type
*
* NOTE: Client lookup for given pclLookupNum MUST be configured
*       before this API is called (cpssDxChExactMatchClientLookupSet).
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum;
*                               Aldrin; AC3X; Lion2; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES  Falcon; AC5P; AC5X; Harrier; Ironman)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] direction                - ingress/egress
* @param[in] packetType               - PCL packet type
* @param[in] subProfileId             - pcl sub profile
*       (APPLICABLE RANGES: Falcon - 0..7, AC5P; AC5X - 0..15)
* @param[in] pclLookupNum             - pcl lookup number
* @param[in] enableExactMatchLookup   - (pointer to)enable Exact Match lookup
*                                   GT_TRUE: trigger Exact Match lookup.
*                                   GT_FALSE: Do not trigger Exact Match lookup.
* @param[in] profileId - Exact Match profile identifier
*       (APPLICABLE RANGES: 1..15)
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - parameter not in valid range.
* @retval GT_BAD_PARAM             - on wrong param
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS prvCpssDxChExactMatchPortGroupPclProfileIdSet
(
   IN  GT_U8                               devNum,
   IN  GT_PORT_GROUPS_BMP                  portGroupsBmp,
   IN  CPSS_PCL_DIRECTION_ENT              direction,
   IN  CPSS_DXCH_PCL_PACKET_TYPE_ENT       packetType,
   IN  GT_U32                              subProfileId,
   IN  CPSS_DXCH_EXACT_MATCH_LOOKUP_ENT    pclLookupNum,
   IN  GT_BOOL                             enableExactMatchLookup,
   IN  GT_U32                              profileId
);
/**
* @internal prvCpssDxChExactMatchPortGroupPclProfileIdGet function
* @endinternal
*
* @brief  Gets the Exact Match Profile Id from PCL packet type
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; Bobcat2; Caelum;
*                               Aldrin; AC3X; Lion2; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[in] portGroupsBmp        - bitmap of Port Groups.
*                                   NOTEs:
*                                   1. for non multi-port groups device this parameter is IGNORED.
*                                   2. for multi-port groups device :
*                                   (APPLICABLE DEVICES  Falcon)
*                                   bitmap must be set with at least one bit representing
*                                   valid port group(s). If a bit of non valid port group
*                                   is set then function returns GT_BAD_PARAM.
*                                   value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] direction                - ingress/egress
* @param[in] packetType               - PCL packet type
* @param[in] subProfileId             - pcl sub profile
*       (APPLICABLE RANGES: Falcon - 0..7, AC5P - 0..15)
* @param[in] pclLookupNum             - pcl lookup number
* @param[out] enableExactMatchLookupPtr- (pointer to)enable
*                           Exact Match lookup
*                           GT_TRUE: trigger Exact Match lookup.
*                           GT_FALSE: Do not trigger Exact Match
*                           lookup.
* @param[in] profileIdPtr - (pointer to)Exact Match profile
*       identifier (APPLICABLE RANGES: 1..15)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - parameter not in valid range.
* @retval GT_BAD_PARAM             - on wrong param
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS prvCpssDxChExactMatchPortGroupPclProfileIdGet
(
   IN  GT_U8                               devNum,
   IN  GT_PORT_GROUPS_BMP                  portGroupsBmp,
   IN  CPSS_PCL_DIRECTION_ENT              direction,
   IN  CPSS_DXCH_PCL_PACKET_TYPE_ENT       packetType,
   IN  GT_U32                              subProfileId,
   IN  CPSS_DXCH_EXACT_MATCH_LOOKUP_ENT    pclLookupNum,
   OUT GT_BOOL                             *enableExactMatchLookupPtr,
   OUT GT_U32                              *profileIdPtr
);
/**
* @internal prvCpssDxChExactMatchPortGroupClientLookupSet function
* @endinternal
*
* @brief   Set global configuration to determine the client of
*          the first Exact Match lookup and the client of the
*          second Exact Match lookup.
*
*   NOTE: this API should be called before configuring
*      Exact Match Profile Id for TTI keyType
*      (cpssDxChExactMatchTtiProfileIdModePacketTypeSet) or
*      Exact Match Profile Id for PCL/EPCL packet type
*      (cpssDxChExactMatchPclProfileIdSet)
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum;
*                                  Aldrin; AC3X; Lion2; Bobcat3;
*                                  Aldrin2.
*
* @param[in] devNum               - the device number
* @param[in] portGroupsBmp        - bitmap of Port Groups.
*                                   NOTEs:
*                                   1. for non multi-port groups device this parameter is IGNORED.
*                                   2. for multi-port groups device :
*                                   (APPLICABLE DEVICES  Falcon; AC5P; AC5X; Harrier; Ironman)
*                                   bitmap must be set with at least one bit representing
*                                   valid port group(s). If a bit of non valid port group
*                                   is set then function returns GT_BAD_PARAM.
*                                   value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] exactMatchLookupNum  - exact match lookup number
* @param[in] clientType           - client type (TTI/EPCL/IPCL)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChExactMatchPortGroupClientLookupSet
(
    IN  GT_U8                                       devNum,
    IN  GT_PORT_GROUPS_BMP                          portGroupsBmp,
    IN  CPSS_DXCH_EXACT_MATCH_LOOKUP_ENT            exactMatchLookupNum,
    IN  CPSS_DXCH_EXACT_MATCH_CLIENT_ENT            clientType
);

/*
* @internal prvCpssDxChExactMatchPortGroupClientLookupGet function
* @endinternal
*
* @brief   Get global configuration that determine the client of
*          the first Exact Match lookup and the client of the
*          second Exact Match lookup.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; Bobcat2; Caelum;
*                                  Aldrin; AC3X; Lion2; Bobcat3;
*                                  Aldrin2.
*
* @param[in] devNum               - the device number
* @param[in] portGroupsBmp        - bitmap of Port Groups.
*                                   NOTEs:
*                                   1. for non multi-port groups device this parameter is IGNORED.
*                                   2. for multi-port groups device :
*                                   (APPLICABLE DEVICES  Falcon)
*                                   bitmap must be set with at least one bit representing
*                                   valid port group(s). If a bit of non valid port group
*                                   is set then function returns GT_BAD_PARAM.
*                                   value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] exactMatchLookupNum  - exact match lookup number
* @param[in] clientTypePtr        - (pointer to)client type (TTI/EPCL/IPCL)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChExactMatchPortGroupClientLookupGet
(
    IN  GT_U8                                       devNum,
    IN  GT_PORT_GROUPS_BMP                          portGroupsBmp,
    IN  CPSS_DXCH_EXACT_MATCH_LOOKUP_ENT            exactMatchLookupNum,
    OUT CPSS_DXCH_EXACT_MATCH_CLIENT_ENT            *clientTypePtr
);

/**
* @internal prvCpssDxChExactMatchPortGroupActivityBitEnableSet function
* @endinternal
*
* @brief   Enables/disables the refresh of the Exact Match
*          activity bit.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2;
*         Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*
* @param[in] devNum             - device number
* @param[in] portGroupsBmp      - bitmap of Port Groups.
*                                 NOTEs:
*                                 1. for non multi-port groups device this parameter is IGNORED.
*                                 2. for multi-port groups device :
*                                 (APPLICABLE DEVICES  Falcon; AC5P; AC5X; Harrier; Ironman)
*                                 bitmap must be set with at least one bit representing
*                                 valid port group(s). If a bit of non valid port group
*                                 is set then function returns GT_BAD_PARAM.
*                                 value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.*
* @param[in] enable             - GT_TRUE - enable refreshing
*                                 GT_FALSE - disable refreshing
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS prvCpssDxChExactMatchPortGroupActivityBitEnableSet
(
    IN GT_U8                devNum,
    IN GT_PORT_GROUPS_BMP   portGroupsBmp,
    IN GT_BOOL              enable
);

/**
* @internal prvCpssDxChExactMatchPortGroupProfileKeyParamsSet function
* @endinternal
*
* @brief   Sets the Exact Match Profile key parameters
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2;
*         Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*
* @param[in] devNum            - device number
* @param[in] portGroupsBmp     - bitmap of Port Groups.
*                                NOTEs:
*                                1. for non multi-port groups device this parameter is IGNORED.
*                                2. for multi-port groups device :
*                                (APPLICABLE DEVICES  Falcon; AC5P; AC5X; Harrier; Ironman)
*                                bitmap must be set with at least one bit representing
*                                valid port group(s). If a bit of non valid port group
*                                is set then function returns GT_BAD_PARAM.
*                                value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] exactMatchProfileIndex - Exact Match
*            profile identifier (APPLICABLE RANGES: 1..15)
* @param[in] keyParamsPtr      - (pointer to)Exact Match
*                                profile key parameters
*
* @retval GT_OK                - on success
* @retval GT_HW_ERROR          - on hardware error
* @retval GT_BAD_PARAM         - on wrong param
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS prvCpssDxChExactMatchPortGroupProfileKeyParamsSet
(
    IN GT_U8                                            devNum,
    IN GT_PORT_GROUPS_BMP                               portGroupsBmp,
    IN GT_U32                                           exactMatchProfileIndex,
    IN CPSS_DXCH_EXACT_MATCH_PROFILE_KEY_PARAMS_STC     *keyParamsPtr
);
/**
* @internal prvCpssDxChExactMatchPortGroupProfileKeyParamsGet function
* @endinternal
*
* @brief   Gets the Exact Match Profile key parameters
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2;
*         Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*
* @param[in] devNum            - device number
* @param[in] portGroupsBmp     - bitmap of Port Groups.
*                                NOTEs:
*                                1. for non multi-port groups device this parameter is IGNORED.
*                                2. for multi-port groups device :
*                                (APPLICABLE DEVICES  Falcon)
*                                bitmap must be set with at least one bit representing
*                                valid port group(s). If a bit of non valid port group
*                                is set then function returns GT_BAD_PARAM.
*                                value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] exactMatchProfileIndex - Exact Match profile
*                 identifier (APPLICABLE RANGES:1..15)
* @param[in] keyParamsPtr      - (pointer to)Exact Match
*                                profile key parameters
*
* @retval GT_OK                - on success
* @retval GT_HW_ERROR          - on hardware error
* @retval GT_BAD_PARAM         - on wrong param
* @retval GT_BAD_PTR           - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS prvCpssDxChExactMatchPortGroupProfileKeyParamsGet
(
    IN  GT_U8                                           devNum,
    IN  GT_PORT_GROUPS_BMP                              portGroupsBmp,
    IN  GT_U32                                          exactMatchProfileIndex,
    OUT CPSS_DXCH_EXACT_MATCH_PROFILE_KEY_PARAMS_STC    *keyParamsPtr
);
/**
* @internal prvCpssDxChExactMatchPortGroupProfileDefaultActionSet function
* @endinternal
*
* @brief   Sets the default Action in case there is no match in the
*          Exact Match lookup and in the TCAM lookup
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2;
*         Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*
* @param[in] devNum             - device number
* @param[in] portGroupsBmp      - bitmap of Port Groups.
*                                 NOTEs:
*                                 1. for non multi-port groups device this parameter is IGNORED.
*                                 2. for multi-port groups device :
*                                 (APPLICABLE DEVICES  Falcon; AC5P; AC5X; Harrier; Ironman)
*                                 bitmap must be set with at least one bit representing
*                                 valid port group(s). If a bit of non valid port group
*                                 is set then function returns GT_BAD_PARAM.
*                                 value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.*
* @param[in] exactMatchProfileIndex - Exact Match profile
*                    identifier (APPLICABLE RANGES:1..15)
* @param[in] actionType         - Exact Match Action Type
* @param[in] actionPtr          - (pointer to)Exact Match Action
* @param[in] defActionEn        - Enable using Profile
*                               Table default Action in case
*                               there is no match in the Exact
*                               Match lookup and in the TCAM
*                               lookup
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong param
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS prvCpssDxChExactMatchPortGroupProfileDefaultActionSet
(
    IN GT_U8                                    devNum,
    IN GT_PORT_GROUPS_BMP                       portGroupsBmp,
    IN GT_U32                                   exactMatchProfileIndex,
    IN CPSS_DXCH_EXACT_MATCH_ACTION_TYPE_ENT    actionType,
    IN CPSS_DXCH_EXACT_MATCH_ACTION_UNT         *actionPtr,
    IN GT_BOOL                                  defActionEn
);
/**
* @internal prvCpssDxChExactMatchPortGroupProfileDefaultActionGet function
* @endinternal
*
* @brief   Gets the default Action in case there is no match in
*          the Exact Match lookup and in the TCAM lookup
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2;
*         Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*
* @param[in] devNum             - device number
* @param[in] portGroupsBmp     - bitmap of Port Groups.
*                                NOTEs:
*                                1. for non multi-port groups device this parameter is IGNORED.
*                                2. for multi-port groups device :
*                                (APPLICABLE DEVICES  Falcon)
*                                bitmap must be set with at least one bit representing
*                                valid port group(s). If a bit of non valid port group
*                                is set then function returns GT_BAD_PARAM.
*                                value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] exactMatchProfileIndex - Exact Match profile
*                     identifier (APPLICABLE RANGES:1..15)
* @param[in] actionType         - Exact Match Action Type
* @param[in] actionPtr          - (pointer to)Exact Match Action
* @param[in] defActionEnPtr     - (pointer to) Enable using
*                               Profile Table default Action in
*                               case there is no match in the
*                               Exact Match lookup and in the
*                               TCAM lookup
*
* @retval GT_OK                 - on success
* @retval GT_HW_ERROR           - on hardware error
* @retval GT_BAD_PARAM          - on wrong param
* @retval GT_BAD_PTR            - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS prvCpssDxChExactMatchPortGroupProfileDefaultActionGet
(
    IN  GT_U8                                    devNum,
    IN  GT_PORT_GROUPS_BMP                       portGroupsBmp,
    IN  GT_U32                                   exactMatchProfileIndex,
    IN  CPSS_DXCH_EXACT_MATCH_ACTION_TYPE_ENT    actionType,
    OUT CPSS_DXCH_EXACT_MATCH_ACTION_UNT         *actionPtr,
    OUT GT_BOOL                                  *defActionEnPtr
);
/**
* @internal prvCpssDxChExactMatchPortGroupExpandedActionSet function
* @endinternal
*
* @brief  Sets the expanded action for Exact Match Profile. The
*         API also sets for each action attribute whether to
*         take it from the Exact Match entry action or from
*         the Exact Match profile
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2;
*         Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*
* @param[in] devNum            - device number
* @param[in] portGroupsBmp    - bitmap of Port Groups.
*                               NOTEs:
*                               1. for non multi-port groups device this parameter is IGNORED.
*                               2. for multi-port groups device :
*                               (APPLICABLE DEVICES  Falcon)
*                               bitmap must be set with at least one bit representing
*                               valid port group(s). If a bit of non valid port group
*                               is set then function returns GT_BAD_PARAM.
*                               value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] expandedActionIndex - Exact Match Expander table
*                               index (APPLICABLE RANGES:0..15)
* @param[in] actionType        - Exact Match Action Type
* @param[in] actionPtr         - (pointer to)Exact Match Action
* @param[in] expandedActionOriginPtr - (pointer to) Whether to use
*                   the action attributes from the Exact Match rule
*                   action or from the profile
* @param[in] hwWriteBlock      -if true -skip hw write
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong param
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* NOTE: This API must be called before configuration of
*         Exact Match Entry by API cpssDxChExactMatchEntrySet
*
*         The Expander Action Entry cannot be changed if an
*         Exact Match Rule is using this entry
*/
GT_STATUS prvCpssDxChExactMatchPortGroupExpandedActionSet
(
    IN GT_U8                                            devNum,
    IN GT_PORT_GROUPS_BMP                               portGroupsBmp,
    IN GT_U32                                           expandedActionIndex,
    IN CPSS_DXCH_EXACT_MATCH_ACTION_TYPE_ENT            actionType,
    IN CPSS_DXCH_EXACT_MATCH_ACTION_UNT                 *actionPtr,
    IN CPSS_DXCH_EXACT_MATCH_EXPANDED_ACTION_ORIGIN_UNT *expandedActionOriginPtr,
    IN GT_BOOL                                          hwWriteBlock
);

/**
* @internal prvCpssDxChExactMatchReducedToHwformat function
* @endinternal
*
* @brief    build the reduce entry to set into HW
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum;
*                                  Aldrin; AC3X; Lion2; Bobcat3;
*                                  Aldrin2.
*
* @param[in] devNum                  - device number
* @param[in] exactMatchHwActionArray - Exact match HW Action
* @param[in] reducedMaskArr          - Reduced mask to be used when
*                                      configuring the Exact Match Entry
* @param[out]reducedHwActionArr - Exact match HW Reduced Action
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on fail
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong param
* @retval GT_BAD_VALUE             - on wrong HW value
*/
GT_STATUS prvCpssDxChExactMatchReducedToHwformat
(
    IN  GT_U8        devNum,
    IN  GT_U32       exactMatchHwActionArray[],
    IN  GT_U32       reducedMaskArr[],
    OUT GT_U8        reducedHwActionArr[]
);

/**
* @internal prvCpssDxChExactMatchEntryToHwformat function
* @endinternal
*
* @brief  build entry hw format key + ReducedAction
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum;
*                                  Aldrin; AC3X; Lion2; Bobcat3;
*                                  Aldrin2.
*
* @param[in] devNum              - device number
* @param[in] expandedActionIndex - Expanded entry index
* @param[in] entryPtr         - (pointer to)Exact Match SW entry
* @param[in]reducedHwActionArr- Exact match HW Reduced Action
* @param[out]hwDataArr        - Exact match HW entry
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on fail
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong param
* @retval GT_BAD_VALUE             - on wrong HW value
*/
GT_STATUS prvCpssDxChExactMatchEntryToHwformat
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          expandedActionIndex,
    IN  CPSS_DXCH_EXACT_MATCH_ENTRY_STC *entryPtr,
    IN  GT_U8                           reducedHwActionArr[],
    OUT GT_U32                          hwDataArr[]
);

/**
* @internal prvCpssDxChExactMatchIndexValidity function
* @endinternal
*
* @brief   check validity of Exact Match index
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; Bobcat2; Caelum;
*                                  Aldrin; AC3X; Lion2; Bobcat3;
*                                  Aldrin2.
*
* @param[in] devNum                - device number
* @param[in] index                 - Exact match entry index
* @param[in] keySize               - Exact match key size
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on fail
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong param
* @retval GT_BAD_VALUE             - on wrong HW value
*/
GT_STATUS prvCpssDxChExactMatchIndexValidity
(
    IN GT_U8                                devNum,
    IN GT_U32                               index,
    IN CPSS_DXCH_EXACT_MATCH_KEY_SIZE_ENT   keySize
);

/**
* @internal prvCpssDxChExpandedActionInvalidateShadow function
* @endinternal
*
* @brief   Invalidate SW values in shadow
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum;
*                                  Aldrin; AC3X; Lion2; Bobcat3;
*                                  Aldrin2.
*
* @param[in] devNum              - device number
* @param[in] expandedActionIndex - Exact Match Expander table
*                               index (APPLICABLE RANGES:1..15)
*
* @retval GT_OK                    - on success
*/
GT_STATUS prvCpssDxChExpandedActionInvalidateShadow
(
    IN GT_U8        devNum,
    IN GT_U32       expandedActionIndex
);
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvCpssDxChExactMatchh */

