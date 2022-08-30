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
* @file cpssDxChTcam.h
*
* @brief CPSS TCAM declarations.
*
* @version   11
********************************************************************************
*/

#ifndef __cpssDxChTcamh
#define __cpssDxChTcamh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/cpssTypes.h>
#include <cpss/generic/cpssCommonDefs.h>

/**
* General NOTEs about portGroupsBmp In TCAM related APIs:
*   1. In Falcon :
*     the TCAM is shared between 2 port groups.
*     the caller need to use 'representative' port groups , for example:
*     value 0x00000014 hold bits 2,4 represent TCAMs of port groups 2,3 and 4,5
*     value 0x00000041 hold bits 0,6 represent TCAMs of port groups 0,1 and 6,7
*     Value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.(represent all port groups)
*
**/

/* max number of floors in TCAM */
#define CPSS_DXCH_TCAM_MAX_NUM_FLOORS_CNS  16 /* AC5P has 16 floors */

/* max number of banks in TCAM */
#define CPSS_DXCH_TCAM_MAX_NUM_BANKS_CNS   12

/* max number of rules per bank in TCAM */
#define CPSS_DXCH_TCAM_MAX_NUM_RULES_PER_BANKS_CNS   256

/* max number of blocks in TCAM */
#define CPSS_DXCH_TCAM_MAX_NUM_BLOCKS_CNS   6

/* max number of rules per half floor in TCAM (1536) */
#define CPSS_DXCH_TCAM_MAX_NUM_RULES_PER_HALF_FLOOR_CNS   (CPSS_DXCH_TCAM_MAX_NUM_RULES_PER_BANKS_CNS*(CPSS_DXCH_TCAM_MAX_NUM_BANKS_CNS / 2))

/* max number of rules per floor in TCAM (3072) */
#define CPSS_DXCH_TCAM_MAX_NUM_RULES_PER_FLOOR_CNS   (CPSS_DXCH_TCAM_MAX_NUM_RULES_PER_BANKS_CNS*CPSS_DXCH_TCAM_MAX_NUM_BANKS_CNS)

/* max number of rules in TCAM (36864) */
#define CPSS_DXCH_TCAM_MAX_NUM_RULES_CNS   (CPSS_DXCH_TCAM_MAX_NUM_RULES_PER_BANKS_CNS*CPSS_DXCH_TCAM_MAX_NUM_FLOORS_CNS*CPSS_DXCH_TCAM_MAX_NUM_BANKS_CNS)

/* Old CPSS versions used obsolete name CPSS_DXCH_TCAM_HALF_FLOOR_INFO_STC. New versions use new name CPSS_DXCH_TCAM_BLOCK_INFO_STC. */
#define CPSS_DXCH_TCAM_HALF_FLOOR_INFO_STC CPSS_DXCH_TCAM_BLOCK_INFO_STC

/* the number of rules derived from the 'max' of 'TTI'/'PCL' rules defined */
/* as we need to support the Ironman-S that hold no TTI connection to the TCAM ! */
#define NUM_TCAM_RULES_MAC(_devNum) \
    ((PCL_NUM_RULES_MAX_AS_TTI_AUTO_CALC != PRV_CPSS_DXCH_PP_MAC(_devNum)->fineTuning.tableSize.policyTcamRaws) ? \
        (MAX(                           \
         PRV_CPSS_DXCH_PP_MAC(_devNum)->fineTuning.tableSize.policyTcamRaws , \
         PRV_CPSS_DXCH_PP_MAC(_devNum)->fineTuning.tableSize.tunnelTerm)):    \
         PRV_CPSS_DXCH_PP_MAC(_devNum)->fineTuning.tableSize.tunnelTerm)

/* macro to validate the value of global line index in TCAM
    use fineTuning.tableSize.tunnelTerm for the check
*/
#define PRV_CPSS_DXCH_TCAM_CHECK_LINE_INDEX_MAC(devNum,index)      \
    if ((index) >= NUM_TCAM_RULES_MAC(devNum)) \
    {                                                              \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "tcam 'index' must be in range [0..%d] but got[%d]" ,   \
            NUM_TCAM_RULES_MAC(devNum) - 1 , index); \
    }

/* Converts rule size to number of TCAM banks value */
#define PRV_CPSS_DXCH_TCAM_CONVERT_RULE_SIZE_TO_NUM_OF_BANKS_VAL_MAC(_numOfBanks, _ruleSize)\
    switch (_ruleSize)                                                                      \
    {                                                                                       \
        case CPSS_DXCH_TCAM_RULE_SIZE_10_B_E:                                               \
            _numOfBanks = 1;                                                                \
            break;                                                                          \
        case CPSS_DXCH_TCAM_RULE_SIZE_20_B_E:                                               \
            _numOfBanks = 2;                                                                \
            break;                                                                          \
        case CPSS_DXCH_TCAM_RULE_SIZE_30_B_E:                                               \
            _numOfBanks = 3;                                                                \
            break;                                                                          \
        case CPSS_DXCH_TCAM_RULE_SIZE_40_B_E:                                               \
            _numOfBanks = 4;                                                                \
            break;                                                                          \
        case CPSS_DXCH_TCAM_RULE_SIZE_50_B_E:                                               \
            _numOfBanks = 5;                                                                \
            break;                                                                          \
        case CPSS_DXCH_TCAM_RULE_SIZE_60_B_E:                                               \
            _numOfBanks = 6;                                                                \
            break;                                                                          \
        case CPSS_DXCH_TCAM_RULE_SIZE_80_B_E:                                               \
            _numOfBanks = 8;                                                                \
            break;                                                                          \
        default:                                                                            \
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);                                                            \
    }

/* Converts number of TCAM banks to rule size value */
#define PRV_CPSS_DXCH_TCAM_CONVERT_NUM_OF_BANKS_TO_RULE_SIZE_VAL_MAC(_ruleSize, _numOfBanks)\
    switch (_numOfBanks)                                                                    \
    {                                                                                       \
        case 1:                                                                             \
            _ruleSize = CPSS_DXCH_TCAM_RULE_SIZE_10_B_E;                                    \
            break;                                                                          \
        case 2:                                                                             \
            _ruleSize = CPSS_DXCH_TCAM_RULE_SIZE_20_B_E;                                    \
            break;                                                                          \
        case 3:                                                                             \
            _ruleSize = CPSS_DXCH_TCAM_RULE_SIZE_30_B_E;                                    \
            break;                                                                          \
        case 4:                                                                             \
            _ruleSize = CPSS_DXCH_TCAM_RULE_SIZE_40_B_E;                                    \
            break;                                                                          \
        case 5:                                                                             \
            _ruleSize = CPSS_DXCH_TCAM_RULE_SIZE_50_B_E;                                    \
            break;                                                                          \
        case 6:                                                                             \
            _ruleSize = CPSS_DXCH_TCAM_RULE_SIZE_60_B_E;                                    \
            break;                                                                          \
        case 8:                                                                             \
            _ruleSize = CPSS_DXCH_TCAM_RULE_SIZE_80_B_E;                                    \
            break;                                                                          \
        default:                                                                            \
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);                                                            \
    }

/* Converts rule size to key size value */
#define PRV_CPSS_DXCH_TCAM_CONVERT_RULE_SIZE_TO_KEY_SIZE_VAL_MAC(_keySize, _ruleSize)       \
    switch (_ruleSize)                                                                      \
    {                                                                                       \
        case CPSS_DXCH_TCAM_RULE_SIZE_10_B_E:                                               \
            _keySize = 0;                                                                   \
            break;                                                                          \
        case CPSS_DXCH_TCAM_RULE_SIZE_20_B_E:                                               \
            _keySize = 1;                                                                   \
            break;                                                                          \
        case CPSS_DXCH_TCAM_RULE_SIZE_30_B_E:                                               \
            _keySize = 2;                                                                   \
            break;                                                                          \
        case CPSS_DXCH_TCAM_RULE_SIZE_40_B_E:                                               \
            _keySize = 3;                                                                   \
            break;                                                                          \
        case CPSS_DXCH_TCAM_RULE_SIZE_50_B_E:                                               \
            _keySize = 4;                                                                   \
            break;                                                                          \
        case CPSS_DXCH_TCAM_RULE_SIZE_60_B_E:                                               \
            _keySize = 5;                                                                   \
            break;                                                                          \
        case CPSS_DXCH_TCAM_RULE_SIZE_80_B_E:                                               \
            _keySize = 7;                                                                   \
            break;                                                                          \
        default:                                                                            \
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);                                                            \
    }

/* Converts key size to rule size value */
#define PRV_CPSS_DXCH_TCAM_CONVERT_KEY_SIZE_TO_RULE_SIZE_VAL_MAC_EX(_ruleSize, _keySize, _rc) \
    switch (_keySize)                                                                         \
    {                                                                                         \
        case 0:                                                                               \
            _ruleSize = CPSS_DXCH_TCAM_RULE_SIZE_10_B_E;                                      \
            break;                                                                            \
        case 1:                                                                               \
            _ruleSize = CPSS_DXCH_TCAM_RULE_SIZE_20_B_E;                                      \
            break;                                                                            \
        case 2:                                                                               \
            _ruleSize = CPSS_DXCH_TCAM_RULE_SIZE_30_B_E;                                      \
            break;                                                                            \
        case 3:                                                                               \
            _ruleSize = CPSS_DXCH_TCAM_RULE_SIZE_40_B_E;                                      \
            break;                                                                            \
        case 4:                                                                               \
            _ruleSize = CPSS_DXCH_TCAM_RULE_SIZE_50_B_E;                                      \
            break;                                                                            \
        case 5:                                                                               \
            _ruleSize = CPSS_DXCH_TCAM_RULE_SIZE_60_B_E;                                      \
            break;                                                                            \
        case 7:                                                                               \
            _ruleSize = CPSS_DXCH_TCAM_RULE_SIZE_80_B_E;                                      \
            break;                                                                            \
        default:                                                                              \
            CPSS_LOG_ERROR_AND_RETURN_MAC(_rc, LOG_ERROR_NO_MSG);                             \
    }

#define PRV_CPSS_DXCH_TCAM_CONVERT_KEY_SIZE_TO_RULE_SIZE_VAL_MAC(_ruleSize, _keySize)       \
            PRV_CPSS_DXCH_TCAM_CONVERT_KEY_SIZE_TO_RULE_SIZE_VAL_MAC_EX(_ruleSize, _keySize, GT_BAD_PARAM)

/* Converts TCAM client to client id value */
#define PRV_CPSS_DXCH_TCAM_CONVERT_CLIENT_TO_CLIENT_ID_VAL_MAC(_devNum,_clientId,_tcamClient)\
    if (PRV_CPSS_DXCH_PP_MAC(_devNum)->hwInfo.pcl.iPcl0Bypass == GT_FALSE)              \
    {                                                                                   \
        switch (_tcamClient)                                                              \
        {                                                                                 \
            case CPSS_DXCH_TCAM_IPCL_0_E:                                                 \
                _clientId = 1;                                                            \
                break;                                                                    \
            case CPSS_DXCH_TCAM_IPCL_1_E:                                                 \
                _clientId = 2;                                                            \
                break;                                                                    \
            case CPSS_DXCH_TCAM_IPCL_2_E:                                                 \
                _clientId = 3;                                                            \
                break;                                                                    \
            case CPSS_DXCH_TCAM_EPCL_E:                                                   \
                _clientId = 4;                                                            \
                break;                                                                    \
            case CPSS_DXCH_TCAM_TTI_E:                                                    \
                _clientId = 0;                                                            \
                break;                                                                    \
            default:                                                                      \
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);            \
        }                                                                               \
    }                                                                                   \
    else                                                                                \
    {                                                                                   \
        switch (_tcamClient)                                                              \
        {                                                                                 \
            case CPSS_DXCH_TCAM_IPCL_0_E:                                                 \
                _clientId = 1;                                                            \
                break;                                                                    \
            case CPSS_DXCH_TCAM_IPCL_1_E:                                                 \
                _clientId = 2;                                                            \
                break;                                                                    \
            case CPSS_DXCH_TCAM_EPCL_E:                                                   \
                _clientId = 3;                                                            \
                break;                                                                    \
            case CPSS_DXCH_TCAM_TTI_E:                                                    \
                _clientId = 0;                                                            \
                break;                                                                    \
            default:                                                                      \
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);            \
        }                                                                               \
    }

/* check that TCAM group id is in valid range (APPLICABLE RANGES: Bobcat2, Caelum, Aldrin, AC3X, AC5X, Harrier, Ironman: 0..2, Bobcat3, Aldrin2, AC5P: 0..4, Falcon: 0..3)   */
#define PRV_CPSS_DXCH_TCAM_GROUP_ID_DEV_CHECK_MAC(_devNum,_tcamGroup)                 \
    if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))                                              \
    {                                                                                 \
        if (_tcamGroup >= PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.tcam.sip6maxTcamGroupId) \
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, "[%s][%d] >= max TCAM groups[%d]",\
                #_tcamGroup,_tcamGroup,PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.tcam.sip6maxTcamGroupId);        \
    }                                                                                     \
    else                                                                                  \
    {                                                                                     \
        switch (PRV_CPSS_PP_MAC(_devNum)->devFamily)                                      \
        {                                                                                 \
        case CPSS_PP_FAMILY_DXCH_BOBCAT2_E:                                               \
        case CPSS_PP_FAMILY_DXCH_ALDRIN_E:                                                \
        case CPSS_PP_FAMILY_DXCH_AC3X_E:                                                  \
            if (_tcamGroup > 2)                                                           \
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);        \
            break;                                                                        \
        case CPSS_PP_FAMILY_DXCH_BOBCAT3_E:                                               \
        case CPSS_PP_FAMILY_DXCH_ALDRIN2_E:                                               \
            if (_tcamGroup > 4)                                                           \
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);        \
            break;                                                                        \
        default:                                                                          \
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);                \
        }                                                                                 \
    }

/* check TCAM hit number range   */
#define PRV_CPSS_DXCH_TCAM_HIT_NUM_CHECK_MAC(_devNum,_hitNum)                                 \
    if(PRV_CPSS_DXCH_PP_MAC(_devNum)->hwInfo.tcam.limitedNumOfParrallelLookups &&             \
       _hitNum >= PRV_CPSS_DXCH_PP_MAC(_devNum)->hwInfo.tcam.limitedNumOfParrallelLookups )   \
    {                                                                                         \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "hitNum[%d] >= 'max'[%d]",                \
            _hitNum,                                                                          \
            PRV_CPSS_DXCH_PP_MAC(_devNum)->hwInfo.tcam.limitedNumOfParrallelLookups);         \
    }                                                                                         \
    else                                                                                      \
    if (_hitNum > 3)                                                                  \
    {                                                                                 \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);                \
    }


/**
* @enum CPSS_DXCH_TCAM_RULE_SIZE_ENT
 *
 * @brief TCAM rule size.
*/
typedef enum{

    /** TCAM rule size to be used 10 Bytes */
    CPSS_DXCH_TCAM_RULE_SIZE_10_B_E,

    /** TCAM rule size to be used 20 Bytes */
    CPSS_DXCH_TCAM_RULE_SIZE_20_B_E,

    /** TCAM rule size to be used 30 Bytes */
    CPSS_DXCH_TCAM_RULE_SIZE_30_B_E,

    /** TCAM rule size to be used 40 Bytes */
    CPSS_DXCH_TCAM_RULE_SIZE_40_B_E,

    /** TCAM rule size to be used 50 Bytes */
    CPSS_DXCH_TCAM_RULE_SIZE_50_B_E,

    /** TCAM rule size to be used 60 Bytes */
    CPSS_DXCH_TCAM_RULE_SIZE_60_B_E,

    /** TCAM rule size to be used 80 Bytes */
    CPSS_DXCH_TCAM_RULE_SIZE_80_B_E

} CPSS_DXCH_TCAM_RULE_SIZE_ENT;

/**
* @enum CPSS_DXCH_TCAM_CLIENT_ENT
 *
 * @brief TCAM clients.
*/
typedef enum{

    /** TCAM client ingress policy 0 (first lookup) */
    CPSS_DXCH_TCAM_IPCL_0_E,

    /** TCAM client ingress policy 1 (second lookup) */
    CPSS_DXCH_TCAM_IPCL_1_E,

    /** TCAM client ingress policy 2 (third lookup) */
    /* (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; BobCat3; Aldrin2) */
    CPSS_DXCH_TCAM_IPCL_2_E,

    /** TCAM client egress policy */
    CPSS_DXCH_TCAM_EPCL_E,

    /** TCAM client tunnel termination */
    CPSS_DXCH_TCAM_TTI_E

} CPSS_DXCH_TCAM_CLIENT_ENT;


/**
* @struct CPSS_DXCH_TCAM_BLOCK_INFO_STC
 *
 * @brief Defines the group id and lookup Number for TCAM block
 * (Bobcat2, Bobcat3, Falcon: banks 0-5 or 6-11,
 * Caelum, Aldrin, AC3X: banks 0-1, 2-3, 4-5 .. 10-11)
*/
typedef struct{

    /** TCAM group id (APPLICABLE RANGES: Bobcat2, Caelum, Aldrin, AC3X: 0..2, Bobcat3; Aldrin2: 0..4, Falcon: 0..3) */
    GT_U32 group;

    /** lookup Number (APPLICABLE RANGES: 0..3) */
    GT_U32 hitNum;

} CPSS_DXCH_TCAM_BLOCK_INFO_STC;

/**
* @struct CPSS_DXCH_TCAM_LOOKUP_PROFILE_CFG_STC
 *
*  @brief Configuration per Parallel Lookups Profile
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
*
*/
typedef struct{

    /** Sizes of subkeys for parallel lookups 0-3 */
    CPSS_DXCH_TCAM_RULE_SIZE_ENT subKey0_3_Sizes[4];

    /** Mux Table Line Indexes for parallel lookups 1-3 (APPLICABLE RANGES: 0..47)
        0-value means that key will be built without muxing */
    GT_U32 subKey1_3_MuxTableLineIndexes[3];

    /** Mux Table Line Offsets for parallel lookups 1-3 (APPLICABLE RANGES: 0..5) in 10-byte resolution */
    GT_U32 subKey1_3_MuxTableLineOffsets[3];

} CPSS_DXCH_TCAM_LOOKUP_PROFILE_CFG_STC;

/**
* @struct CPSS_DXCH_TCAM_LOOKUP_MUX_TABLE_LINE_CFG_STC
 *
*  @brief Configuration per Parallel Lookups Muxing Table Line
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
*
*/
typedef struct{

    /** subKey 2-byte-units offsets to copy from super-key
     *  subKey first 60 bytes (or less if size less from 60 bytes)
     *  built by copying up to 30 2-byte units from super key.
     *  The offsets of the copyed units also specified in 2-byte resolution.
     *  (APPLICABLE RANGES: 0..29) */
    GT_U8 superKeyTwoByteUnitsOffsets[30];

} CPSS_DXCH_TCAM_LOOKUP_MUX_TABLE_LINE_CFG_STC;

/* Defines the bmp banks power up. up to 12 banks*/
typedef GT_U32  GT_FLOOR_BANK_BMP;
/**
* @internal cpssDxChTcamPortGroupRuleWrite function
* @endinternal
*
* @brief   Writes the TCAM mask and key. Writing operation is preformed on all
*         bits in the line.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - the device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Falcon)
*                                      Bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*                                      3. see general note about portGroupsBmp in TCAM APIs for Falcon
* @param[in] index                    - global line  in TCAM to write to.
* @param[in] valid                    - whether TCAM entry is  or not
* @param[in] ruleSize                 - size of rule
* @param[in] tcamPatternPtr           - points to the TCAM rule's pattern.
* @param[in] tcamMaskPtr              - points to the TCAM rule's mask.
*                                      The rule mask is "AND STYLED ONE".
*                                      Mask bit's 0 and pattern bit's 0 means don't care bit
*                                      (corresponding bit in the pattern is not used in the TCAM lookup).
*                                      Mask bit's 0 and pattern bit's 1 means ALWAYS MISS (full entry miss).
*                                      Mask bit's 1 means that corresponding bit in the pattern
*                                      is using in the TCAM lookup.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter, and when the rule cannot start in a given index.
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTcamPortGroupRuleWrite
(
    IN  GT_U8                               devNum,
    IN  GT_PORT_GROUPS_BMP                  portGroupsBmp,
    IN  GT_U32                              index,
    IN  GT_BOOL                             valid,
    IN  CPSS_DXCH_TCAM_RULE_SIZE_ENT        ruleSize,
    IN  GT_U32                              *tcamPatternPtr,
    IN  GT_U32                              *tcamMaskPtr
);

/**
* @internal cpssDxChTcamPortGroupRuleRead function
* @endinternal
*
* @brief   Reads the TCAM mask and key.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - the device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Falcon)
*                                      Bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*                                      3. see general note about portGroupsBmp in TCAM APIs for Falcon
* @param[in] index                    - global line  in TCAM to write to.
*
* @param[out] validPtr                 - (pointer to) whether TCAM entry is valid or not
* @param[out] ruleSizePtr              - (pointer to) size of rule
* @param[out] tcamPatternPtr           - points to the TCAM rule's pattern.
* @param[out] tcamMaskPtr              - points to the TCAM rule's mask.
*                                      The rule mask is "AND STYLED ONE".
*                                      Mask bit's 0 and pattern bit's 0 means don't care bit
*                                      (corresponding bit in the pattern is not used in the TCAM lookup).
*                                      Mask bit's 0 and pattern bit's 1 means ALWAYS MISS (full entry miss).
*                                      Mask bit's 1 means that corresponding bit in the pattern
*                                      is using in the TCAM lookup.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter, and when the rule cannot start in a given index.
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTcamPortGroupRuleRead
(
    IN  GT_U8                               devNum,
    IN  GT_PORT_GROUPS_BMP                  portGroupsBmp,
    IN  GT_U32                              index,
    OUT GT_BOOL                             *validPtr,
    OUT CPSS_DXCH_TCAM_RULE_SIZE_ENT        *ruleSizePtr,
    OUT GT_U32                              *tcamPatternPtr,
    OUT GT_U32                              *tcamMaskPtr
);

/**
* @internal cpssDxChTcamPortGroupRuleValidStatusSet function
* @endinternal
*
* @brief   Validates/Invalidates the TCAM rule.
*         The validation of the rule is performed by next steps:
*         1. Retrieve the content of the rule from PP TCAM
*         2. Write content back to TCAM with Valid indication set.
*         The function does not check content of the rule before
*         write it back to TCAM
*         The invalidation of the rule is performed by next steps:
*         1. Retrieve the content of the rule from PP TCAM
*         2. Write content back to TCAM with Invalid indication set.
*         If the given the rule found already in needed valid state
*         no write done. If the given the rule found with size
*         different from the given rule-size an error code returned.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - the device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Falcon)
*                                      Bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*                                      3. see general note about portGroupsBmp in TCAM APIs for Falcon
* @param[in] index                    - global line  in TCAM to write to.
* @param[in] valid                    - whether TCAM entry is  or not
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter, and when the rule cannot start in a given index.
* @retval GT_BAD_STATE             - if in TCAM found rule of size different from the specified
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTcamPortGroupRuleValidStatusSet
(
    IN  GT_U8                               devNum,
    IN  GT_PORT_GROUPS_BMP                  portGroupsBmp,
    IN  GT_U32                              index,
    IN  GT_BOOL                             valid
);

/**
* @internal cpssDxChTcamPortGroupRuleValidStatusGet function
* @endinternal
*
* @brief   Gets validity of the TCAM rule.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - the device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Falcon)
*                                      Bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*                                      3. see general note about portGroupsBmp in TCAM APIs for Falcon
* @param[in] index                    - global line  in TCAM to write to.
*
* @param[out] validPtr                 - (pointer to) whether TCAM entry is valid or not
* @param[out] ruleSizePtr              - (pointer to) size of rule
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter, and when the rule cannot start in a given index.
* @retval GT_BAD_STATE             - if in TCAM found rule of size different from the specified
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChTcamPortGroupRuleValidStatusGet
(
    IN  GT_U8                               devNum,
    IN  GT_PORT_GROUPS_BMP                  portGroupsBmp,
    IN  GT_U32                              index,
    OUT GT_BOOL                             *validPtr,
    OUT CPSS_DXCH_TCAM_RULE_SIZE_ENT        *ruleSizePtr
);

/**
* @internal cpssDxChTcamPortGroupClientGroupSet function
* @endinternal
*
* @brief   Enable/Disable client per TCAM group.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat3.
*
* @param[in] devNum                   - the device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Falcon)
*                                      Bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*                                      3. see general note about portGroupsBmp in TCAM APIs for Falcon
* @param[in] tcamClient               - TCAM client.
* @param[in] tcamGroup                - TCAM group id (APPLICABLE RANGES: Bobcat2, Caelum, Aldrin, AC3X: 0..2, Falcon: 0..3)
* @param[in] enable                   - GT_TRUE: TCAM client is enabled
*                                      GT_FALSE: TCAM client is disabled
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_RANGE          - parameter not in valid range.
*/
GT_STATUS cpssDxChTcamPortGroupClientGroupSet
(
    IN  GT_U8                               devNum,
    IN  GT_PORT_GROUPS_BMP                  portGroupsBmp,
    IN  CPSS_DXCH_TCAM_CLIENT_ENT           tcamClient,
    IN  GT_U32                              tcamGroup,
    IN  GT_BOOL                             enable
);

/**
* @internal cpssDxChTcamPortGroupClientGroupGet function
* @endinternal
*
* @brief   Gets status (Enable/Disable) for client per TCAM group.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - the device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Falcon)
*                                      Bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*                                      3. see general note about portGroupsBmp in TCAM APIs for Falcon
* @param[in] tcamClient               - TCAM client.
*
* @param[out] tcamGroupPtr             - (pointer to) TCAM group id (APPLICABLE RANGES: Bobcat2, Caelum, Aldrin, AC3X: 0..2)
* @param[out] enablePtr                - (pointer to) TCAM client status.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_RANGE          - parameter not in valid range.
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS cpssDxChTcamPortGroupClientGroupGet

(
    IN  GT_U8                               devNum,
    IN  GT_PORT_GROUPS_BMP                  portGroupsBmp,
    IN  CPSS_DXCH_TCAM_CLIENT_ENT           tcamClient,
    OUT GT_U32                              *tcamGroupPtr,
    OUT GT_BOOL                             *enablePtr
);

/**
* @internal cpssDxChTcamPortGroupIndexRangeHitNumAndGroupSet function
* @endinternal
*
* @brief   Select for each TCAM index which TCAM group and lookup number
*         is served by TCAM.
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - the device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                       NOTEs:
*                                       1. for non multi-port groups device this parameter is IGNORED.
*                                       (APPLICABLE DEVICES Falcon)
*                                       Bitmap must be set with at least one bit representing
*                                       valid port group(s). If a bit of non valid port group
*                                       is set then function returns GT_BAD_PARAM.
*                                       value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*                                       2. see general note about portGroupsBmp in TCAM APIs for Falcon
* @param[in] floorIndex               - floor index (APPLICABLE RANGES: 0..11)
* @param[in] floorInfoArr[CPSS_DXCH_TCAM_MAX_NUM_BLOCKS_CNS] - holds group id and lookup Number for each half floor.
*                                       Index 0 represent banks 0-5 and index 1 represent  banks  6-11.
*                                       APPLICABLE DEVICES: Bobcat2, Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*                                       Index 0 represent banks 0-1, index 1 represent  banks  2-3,
*                                       index 2 represent banks 4-5, index 3 represent  banks  6-7,
*                                       index 4 represent banks 8-9, index 5 represent  banks  10-11.
*                                       APPLICABLE DEVICES: Caelum, Aldrin, AC3X.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_RANGE          - parameter not in valid range.
*/
GT_STATUS cpssDxChTcamPortGroupIndexRangeHitNumAndGroupSet
(
    IN  GT_U8                            devNum,
    IN  GT_PORT_GROUPS_BMP               portGroupsBmp,
    IN  GT_U32                           floorIndex,
    IN  CPSS_DXCH_TCAM_BLOCK_INFO_STC    floorInfoArr[CPSS_DXCH_TCAM_MAX_NUM_BLOCKS_CNS]
);

/**
* @internal cpssDxChTcamIndexRangeHitNumAndGroupSet function
* @endinternal
*
* @brief   Select for each TCAM index which TCAM group and lookup number
*         is served by TCAM.
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - the device number
* @param[in] floorIndex               - floor index (APPLICABLE RANGES: 0..11)
* @param[in] floorInfoArr[CPSS_DXCH_TCAM_MAX_NUM_BLOCKS_CNS] - holds group id and lookup Number for each half floor.
*                                      Index 0 represent banks 0-5 and index 1 represent  banks  6-11.
*                                      APPLICABLE DEVICES: Bobcat2, Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*                                      Index 0 represent banks 0-1, index 1 represent  banks  2-3,
*                                      index 2 represent banks 4-5, index 3 represent  banks  6-7,
*                                      index 4 represent banks 8-9, index 5 represent  banks  10-11.
*                                      APPLICABLE DEVICES: Caelum, Aldrin, AC3X.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_RANGE          - parameter not in valid range.
*/
GT_STATUS cpssDxChTcamIndexRangeHitNumAndGroupSet
(
    IN GT_U8            devNum,
    IN GT_U32           floorIndex,
    IN CPSS_DXCH_TCAM_BLOCK_INFO_STC          floorInfoArr[CPSS_DXCH_TCAM_MAX_NUM_BLOCKS_CNS]
);

/**
* @internal cpssDxChTcamPortGroupIndexRangeHitNumAndGroupGet function
* @endinternal
*
* @brief   Select for each TCAM index which TCAM group and lookup number
*         is served by TCAM.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum          - the device number
* @param[in] portGroupsBmp   - bitmap of Port Groups.
*                              NOTEs:
*                              1. for non multi-port groups device this parameter is IGNORED.
*                              (APPLICABLE DEVICES Falcon)
*                              Bitmap must be set with at least one bit representing
*                              valid port group(s). If a bit of non valid port group
*                              is set then function returns GT_BAD_PARAM.
*                              value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*                              2. see general note about portGroupsBmp in TCAM APIs for Falcon
* @param[in] floorIndex      - floor index (APPLICABLE RANGES: 0..11)
*
* @param[out] floorInfoArr[CPSS_DXCH_TCAM_MAX_NUM_BLOCKS_CNS] - group id and lookup Number
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_RANGE          - parameter not in valid range.
*/
GT_STATUS cpssDxChTcamPortGroupIndexRangeHitNumAndGroupGet
(
    IN  GT_U8                            devNum,
    IN  GT_PORT_GROUPS_BMP               portGroupsBmp,
    IN  GT_U32                           floorIndex,
    OUT CPSS_DXCH_TCAM_BLOCK_INFO_STC    floorInfoArr[CPSS_DXCH_TCAM_MAX_NUM_BLOCKS_CNS]
);

/**
* @internal cpssDxChTcamIndexRangeHitNumAndGroupGet function
* @endinternal
*
* @brief   Select for each TCAM index which TCAM group and lookup number
*         is served by TCAM.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - the device number
* @param[in] floorIndex               - floor index (APPLICABLE RANGES: 0..11)
*
* @param[out] floorInfoArr[CPSS_DXCH_TCAM_MAX_NUM_BLOCKS_CNS] - group id and lookup Number
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_RANGE          - parameter not in valid range.
*/
GT_STATUS cpssDxChTcamIndexRangeHitNumAndGroupGet
(
    IN GT_U8            devNum,
    IN GT_U32           floorIndex,
    OUT CPSS_DXCH_TCAM_BLOCK_INFO_STC          floorInfoArr[CPSS_DXCH_TCAM_MAX_NUM_BLOCKS_CNS] /*maxArraySize=6*/
);

/**
* @internal cpssDxChTcamPortGroupActiveFloorsSet function
* @endinternal
*
* @brief   Function sets number of active floors at TCAM array. The non-active
*         floors will be the upper floors and will be at power down mode.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - the device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                       NOTEs:
*                                       1. for non multi-port groups device this parameter is IGNORED.
*                                       2. for multi-port groups device :
*                                       (APPLICABLE DEVICES Lion2; Bobcat3; Falcon)
*                                       bitmap must be set with at least one bit representing
*                                       valid port group(s). If a bit of non valid port group
*                                       is set then function returns GT_BAD_PARAM.
*                                       value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] numOfActiveFloors        - number of active floors (APPLICABLE RANGES: 0..12)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_RANGE          - parameter not in valid range.
*/
GT_STATUS cpssDxChTcamPortGroupActiveFloorsSet
(
    IN  GT_U8                     devNum,
    IN  GT_PORT_GROUPS_BMP        portGroupsBmp,
    IN  GT_U32                    numOfActiveFloors
);

/**
* @internal cpssDxChTcamActiveFloorsSet function
* @endinternal
*
* @brief   Function sets number of active floors at TCAM array. The non-active
*         floors will be the upper floors and will be at power down mode.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - the device number
* @param[in] numOfActiveFloors        - number of active floors (APPLICABLE RANGES: 0..12)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_RANGE          - parameter not in valid range.
*/
GT_STATUS cpssDxChTcamActiveFloorsSet
(
    IN  GT_U8           devNum,
    IN  GT_U32          numOfActiveFloors
);

/**
* @internal cpssDxChTcamPortGroupActiveFloorsGet function
* @endinternal
*
* @brief   Function gets number of active floors at TCAM array. The non-active
*         floors will be the upper floors and will be at power down mode.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - the device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                       NOTEs:
*                                       1. for non multi-port groups device this parameter is IGNORED.
*                                       2. for multi-port groups device :
*                                       (APPLICABLE DEVICES Lion2; Bobcat3; Falcon)
*                                       bitmap must be set with at least one bit representing
*                                       valid port group(s). If a bit of non valid port group
*                                       is set then function returns GT_BAD_PARAM.
*                                       value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[out] numOfActiveFloorsPtr     - number of active floors
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_RANGE          - parameter not in valid range.
*/
GT_STATUS cpssDxChTcamPortGroupActiveFloorsGet
(
    IN  GT_U8                     devNum,
    IN  GT_PORT_GROUPS_BMP        portGroupsBmp,
    OUT  GT_U32                   *numOfActiveFloorsPtr
);

/**
* @internal cpssDxChTcamActiveFloorsGet function
* @endinternal
*
* @brief   Function gets number of active floors at TCAM array. The non-active
*         floors will be the upper floors and will be at power down mode.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - the device number
*
* @param[out] numOfActiveFloorsPtr     - number of active floors
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_RANGE          - parameter not in valid range.
*/
GT_STATUS cpssDxChTcamActiveFloorsGet
(
    IN   GT_U8           devNum,
    OUT  GT_U32          *numOfActiveFloorsPtr
);

/**
* @internal cpssDxChTcamPortGroupCpuLookupTriggerSet function
* @endinternal
*
* @brief   Sets lookup data and triggers CPU lookup in the TCAM.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - the device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      (APPLICABLE DEVICES Falcon)
*                                      Bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*                                      3. see general note about portGroupsBmp in TCAM APIs for Falcon
* @param[in] group                    - TCAM  id (APPLICABLE RANGES: Bobcat2, Caelum, Aldrin, AC3X: 0..2, Bobcat3; Aldrin2: 0..4)
* @param[in] size                     - key  in TCAM
* @param[in] tcamKeyArr               - (pointer to) key array in the TCAM
* @param[in] subKeyProfile            - TCAM profile ID
*                                       (APPLICABLE DEVICES: AC5P; AC5X; Harrier; Ironman)
*                                       (APPLICABLE RANGES: 0..63)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_RANGE          - parameter not in valid range.
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS cpssDxChTcamPortGroupCpuLookupTriggerSet
(
    IN  GT_U8                               devNum,
    IN  GT_PORT_GROUPS_BMP                  portGroupsBmp,
    IN  GT_U32                              group,
    IN  CPSS_DXCH_TCAM_RULE_SIZE_ENT        size,
    IN  GT_U32                              tcamKeyArr[],
    IN  GT_U32                              subKeyProfile
);

/**
* @internal cpssDxChTcamPortGroupCpuLookupTriggerGet function
* @endinternal
*
* @brief   Gets lookup data and key size from CPU lookup in the TCAM.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - the device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Falcon)
*                                      Bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*                                      3. see general note about portGroupsBmp in TCAM APIs for Falcon
* @param[in] group                    - TCAM  id (APPLICABLE RANGES: Bobcat2, Caelum, Aldrin, AC3X: 0..2, Bobcat3; Aldrin2: 0..4)
*
* @param[out] sizePtr                  - (pointer to) key size in TCAM
* @param[out] tcamKeyArr               - (pointer to) key array in the TCAM
* @param[out]  subKeyProfilePtr        - (pointer to) TCAM profile ID
*                                       (APPLICABLE DEVICES: AC5P; AC5X; Harrier; Ironman)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_RANGE          - parameter not in valid range.
* @retval GT_BAD_STATE             - if in TCAM found rule with different sizes
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS cpssDxChTcamPortGroupCpuLookupTriggerGet
(
    IN  GT_U8                               devNum,
    IN  GT_PORT_GROUPS_BMP                  portGroupsBmp,
    IN  GT_U32                              group,
    OUT CPSS_DXCH_TCAM_RULE_SIZE_ENT       *sizePtr,
    OUT GT_U32                              tcamKeyArr[] /*maxArraySize=20*/,
    OUT GT_U32                             *subKeyProfilePtr
);

/**
* @internal cpssDxChTcamCpuLookupResultsGet function
* @endinternal
*
* @brief   Gets CPU lookup results.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - the device number
* @param[in] group                    - TCAM  id (APPLICABLE RANGES: Bobcat2, Caelum, Aldrin, AC3X: 0..2, Bobcat3; Aldrin2: 0..4)
* @param[in] hitNum                   - hit number in TCAM (APPLICABLE RANGES: 0..3)
* @param[in] portGroupsBmp            -
*
* @param[out] isValidPtr               - (pointer to) whether results are valid for prev trigger set.
* @param[out] isHitPtr                 - (pointer to) whether there was hit in the TCAM. Valid only when isValidPtr == GT_TRUE
* @param[out] hitIndexPtr              - (pointer to) global hit index in the TCAM. Valid only when isHitPtr == GT_TRUE
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_STATE             - if CPU lookup results are not valid
*/
GT_STATUS cpssDxChTcamCpuLookupResultsGet
(
    IN  GT_U8                               devNum,
    IN  GT_PORT_GROUPS_BMP                  portGroupsBmp,
    IN  GT_U32                              group,
    IN  GT_U32                              hitNum,
    OUT GT_BOOL                            *isValidPtr,
    OUT GT_BOOL                            *isHitPtr,
    OUT GT_U32                             *hitIndexPtr
);

/**
* @internal cpssDxChTcamLookupProfileCfgSet function
* @endinternal
*
* @brief   Sets Parallel lookups per Profile configuration.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - the device number
* @param[in] tcamProfileId            - TCAM Profile Id (APPLICABLE RANGES: 1..63).
*                                       TCAM Profile 0 has no configuration and implemented by passing super-key to all lookups.
* @param[in] tcamLookupProfileCfgPtr  - (pointer to) configuration per TCAM profile.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - one of parameters value invalid
* @retval GT_OUT_OF_RANGE          - one of parameters value out-of-range
*/
GT_STATUS cpssDxChTcamLookupProfileCfgSet
(
    IN   GT_U8                                  devNum,
    IN   GT_U32                                 tcamProfileId,
    IN   CPSS_DXCH_TCAM_LOOKUP_PROFILE_CFG_STC  *tcamLookupProfileCfgPtr
);

/**
* @internal cpssDxChTcamLookupProfileCfgGet function
* @endinternal
*
* @brief   Gets Parallel lookups per Profile configuration.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - the device number
* @param[in] tcamProfileId            - TCAM Profile Id (APPLICABLE RANGES: 1..63).
*                                       TCAM Profile 0 has no configuration and implemented by passing super-key to all lookups.
* @param[out] tcamLookupProfileCfgPtr - (pointer to) configuration per TCAM profile.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - one of parameters value invalid
* @retval GT_BAD_STATE             - one HW fields out of range
*/
GT_STATUS cpssDxChTcamLookupProfileCfgGet
(
    IN   GT_U8                                  devNum,
    IN   GT_U32                                 tcamProfileId,
    OUT  CPSS_DXCH_TCAM_LOOKUP_PROFILE_CFG_STC  *tcamLookupProfileCfgPtr
);

/**
* @internal cpssDxChTcamLookupMuxTableLineSet function
* @endinternal
*
* @brief   Sets configuration of TCAM sub-key Mux Table Entry.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                        - the device number
* @param[in] entryIndex                    - TCAM sub-key Mux Table Entry index (APPLICABLE RANGES: 1..47).
*                                            Index 0 used for truncating super-key without muxing.
* @param[in] tcamLookupMuxTableLineCfgPtr  - (pointer to) configuration of TCAM sub-key Mux Table Entry.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - one of parameters value invalid
* @retval GT_OUT_OF_RANGE          - one of parameters value out-of-range
*/
GT_STATUS cpssDxChTcamLookupMuxTableLineSet
(
    IN   GT_U8                                         devNum,
    IN   GT_U32                                        entryIndex,
    IN   CPSS_DXCH_TCAM_LOOKUP_MUX_TABLE_LINE_CFG_STC  *tcamLookupMuxTableLineCfgPtr
);

/**
* @internal cpssDxChTcamLookupMuxTableLineGet function
* @endinternal
*
* @brief   Gets configuration of TCAM sub-key Mux Table Entry.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                        - the device number
* @param[in] entryIndex                    - TCAM sub-key Mux Table Entry index (APPLICABLE RANGES: 1..47).
*                                            Index 0 used for truncating super-key without muxing.
* @param[out] tcamLookupMuxTableLineCfgPtr - (pointer to) configuration of TCAM sub-key Mux Table Entry.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - one of parameters value invalid
*/
GT_STATUS cpssDxChTcamLookupMuxTableLineGet
(
    IN   GT_U8                                         devNum,
    IN   GT_U32                                        entryIndex,
    OUT  CPSS_DXCH_TCAM_LOOKUP_MUX_TABLE_LINE_CFG_STC  *tcamLookupMuxTableLineCfgPtr
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssDxChTcamh */

