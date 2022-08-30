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
* @file tgfTcamGen.h
*
* @brief Generic API for TCAM
*
* @version   2
********************************************************************************
*/
#ifndef __tgfTcamGenh
#define __tgfTcamGenh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifdef CHX_FAMILY
    #include <cpss/dxCh/dxChxGen/tcam/cpssDxChTcam.h>
#endif /* CHX_FAMILY */

#include <common/tgfCommon.h>
#include <utf/private/prvUtfExtras.h>

#define  HARD_WIRE_TCAM_MAC(dev) \
    ((PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_BOBCAT3_E) ? 1 : 0)

/******************************************************************************\
 *                          Private type definitions                          *
\******************************************************************************/

/**
* @enum PRV_TGF_TCAM_CLIENT_ENT
 *
 * @brief TCAM clients.
*/
typedef enum{

    /** TCAM client ingress policy 0 (first lookup) */
    PRV_TGF_TCAM_IPCL_0_E,

    /** TCAM client ingress policy 1 (second lookup) */
    PRV_TGF_TCAM_IPCL_1_E,

    /** TCAM client ingress policy 2 (third lookup) */
    PRV_TGF_TCAM_IPCL_2_E,

    /** TCAM client egress policy */
    PRV_TGF_TCAM_EPCL_E,

    /** TCAM client tunnel termination */
    PRV_TGF_TCAM_TTI_E

} PRV_TGF_TCAM_CLIENT_ENT;

/**
* @struct PRV_TGF_TCAM_BLOCK_INFO_STC
 *
 * @brief Defines the group id and lookup Number for TCAM block.
 * Block size - 6 banks per block -> 2 blocks per floor
 * APPLICABLE DEVICES: Bobcat2.
 * Block size - 2 banks per block -> 6 blocks per floor
 * APPLICABLE DEVICES: Caelum, Aldrin, AC3X.
*/
typedef struct{

    /** TCAM group ID (APPLICABLE RANGES: Bobcat2, Caelum, Aldrin, AC3X: 0..2, Bobcat3; Aldrin2: 0..4) */
    GT_U32 group;

    /** lookup number (APPLICABLE RANGES: 0..3) */
    GT_U32 hitNum;

} PRV_TGF_TCAM_BLOCK_INFO_STC;

/**
* @enum CPSS_DXCH_TCAM_RULE_SIZE_ENT
 *
 * @brief TCAM rule size.
*/
typedef enum{

    /** TCAM rule size to be used 10 Bytes */
    PRV_TGF_TCAM_RULE_SIZE_10_B_E,

    /** TCAM rule size to be used 20 Bytes */
    PRV_TGF_TCAM_RULE_SIZE_20_B_E,

    /** TCAM rule size to be used 30 Bytes */
    PRV_TGF_TCAM_RULE_SIZE_30_B_E,

    /** TCAM rule size to be used 40 Bytes */
    PRV_TGF_TCAM_RULE_SIZE_40_B_E,

    /** TCAM rule size to be used 50 Bytes */
    PRV_TGF_TCAM_RULE_SIZE_50_B_E,

    /** TCAM rule size to be used 60 Bytes */
    PRV_TGF_TCAM_RULE_SIZE_60_B_E,

    /** TCAM rule size to be used 80 Bytes */
    PRV_TGF_TCAM_RULE_SIZE_80_B_E

} PRV_TGF_TCAM_RULE_SIZE_ENT;


#define PRV_TGF_TCAM_MAX_TCAM_BLOCKS_CNS    CPSS_DXCH_TCAM_MAX_NUM_BLOCKS_CNS
/**
* @internal prvTgfTcamClientGroupSet function
* @endinternal
*
* @brief   Enable/Disable client per TCAM group.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; AC5; Lion; xCat2; Lion2.
*
* @param[in] tcamClient               - TCAM client.
* @param[in] tcamGroup                - TCAM group id (APPLICABLE RANGES: Bobcat2, Caelum, Aldrin, AC3X: 0..2, Bobcat3; Aldrin2: 0..4)
* @param[in] enable                   - GT_TRUE: TCAM client is enabled
*                                      GT_FALSE: TCAM client is disabled
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_RANGE          - parameter not in valid range.
*/
GT_STATUS prvTgfTcamClientGroupSet
(
    IN  PRV_TGF_TCAM_CLIENT_ENT tcamClient,
    IN  GT_U32                  tcamGroup,
    IN  GT_BOOL                 enable
);

/**
* @internal prvTgfTcamClientGroupGet function
* @endinternal
*
* @brief   Gets status (Enable/Disable) for client per TCAM group.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; AC5; Lion; xCat2; Lion2.
*
* @param[in] tcamClient               - TCAM client.
*
* @param[out] tcamGroupPtr             - (pointer to) TCAM group id (APPLICABLE RANGES: Bobcat2, Caelum, Aldrin, AC3X: 0..2, Bobcat3; Aldrin2: 0..4)
* @param[out] enablePtr                - (pointer to) TCAM client status.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_RANGE          - parameter not in valid range.
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS prvTgfTcamClientGroupGet
(
    IN  PRV_TGF_TCAM_CLIENT_ENT tcamClient,
    OUT GT_U32                  *tcamGroupPtr,
    OUT GT_BOOL                 *enablePtr
);

/**
* @internal prvTgfTcamIndexRangeHitNumAndGroupSet function
* @endinternal
*
* @brief   Select for each TCAM index which TCAM group and lookup number
*         is served by TCAM.
*         APPLICABLE DEVICES:
*         Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*         NOT APPLICABLE DEVICES:
*         DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; AC5; Lion; xCat2; Lion2.
*         INPUTS:
*         floorIndex     - floor index (APPLICABLE RANGES: 0..11)
*         floorInfoArr    - array of group ID and lookup number for all TCAM blocks in floor.
*         Index 0 represent banks 0-5 and index 1 represent banks 6-11.
*         APPLICABLE DEVICES: Bobcat2, Bobcat3, Aldrin2, Falcon, AC5P, AC5X, Harrier, Ironman.
*         Index 0 represent banks 0-1, index 1 represent banks 2-3,
*         index 2 represent banks 4-5, index 3 represent banks 6-7,
*         index 4 represent banks 8-9, index 5 represent banks 10-11.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; AC5; Lion; xCat2; Lion2.
*
* @param[in] floorIndex               - floor index (APPLICABLE RANGES: 0..11)
* @param[in] floorInfoArr[PRV_TGF_TCAM_MAX_TCAM_BLOCKS_CNS] - array of group ID and lookup number for all TCAM blocks in floor.
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
GT_STATUS prvTgfTcamIndexRangeHitNumAndGroupSet
(
    IN GT_U32                           floorIndex,
    IN PRV_TGF_TCAM_BLOCK_INFO_STC      floorInfoArr[PRV_TGF_TCAM_MAX_TCAM_BLOCKS_CNS]
);

/**
* @internal prvTgfTcamIndexRangeHitNumAndGroupGet function
* @endinternal
*
* @brief   Select for each TCAM index which TCAM group and lookup number
*         is served by TCAM.
*         APPLICABLE DEVICES:
*         Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*         NOT APPLICABLE DEVICES:
*         DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; AC5; Lion; xCat2; Lion2.
*         INPUTS:
*         floorIndex     - floor index (APPLICABLE RANGES: 0..11)
*         OUTPUTS:
*         floorInfoArr    - array of group ID and lookup number for all TCAM blocks in floor.
*         Index 0 represent banks 0-5 and index 1 represent banks 6-11.
*         APPLICABLE DEVICES: Bobcat2, Bobcat3, Aldrin2, Falcon, AC5P, AC5X, Harrier, Ironman.
*         Index 0 represent banks 0-1, index 1 represent banks 2-3,
*         index 2 represent banks 4-5, index 3 represent banks 6-7,
*         index 4 represent banks 8-9, index 5 represent banks 10-11.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; AC5; Lion; xCat2; Lion2.
*
* @param[in] floorIndex               - floor index (APPLICABLE RANGES: 0..11)
*
* @param[out] floorInfoArr[PRV_TGF_TCAM_MAX_TCAM_BLOCKS_CNS] - array of group ID and lookup number for all TCAM blocks in floor.
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
GT_STATUS prvTgfTcamIndexRangeHitNumAndGroupGet
(
    IN GT_U32                               floorIndex,
    OUT PRV_TGF_TCAM_BLOCK_INFO_STC         floorInfoArr[PRV_TGF_TCAM_MAX_TCAM_BLOCKS_CNS]
);

/**
* @internal prvTgfTcamPortGroupCpuLookupTriggerSet function
* @endinternal
*
* @brief   Sets lookup data and triggers CPU lookup in the TCAM.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
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
GT_STATUS prvTgfTcamPortGroupCpuLookupTriggerSet
(
    IN  GT_U8                               devNum,
    IN  GT_PORT_GROUPS_BMP                  portGroupsBmp,
    IN  GT_U32                              group,
    IN  CPSS_DXCH_TCAM_RULE_SIZE_ENT        size,
    IN  GT_U32                              tcamKeyArr[],
    IN  GT_U32                              subKeyProfile
);

/**
* @internal prvTgfTcamPortGroupCpuLookupTriggerGet function
* @endinternal
*
* @brief   Gets lookup data and key size from CPU lookup in the TCAM.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
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
* @param[out] subKeyProfilePtr         - (pointer to) TCAM profile ID
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
GT_STATUS prvTgfTcamPortGroupCpuLookupTriggerGet
(
    IN  GT_U8                               devNum,
    IN  GT_PORT_GROUPS_BMP                  portGroupsBmp,
    IN  GT_U32                              group,
    OUT CPSS_DXCH_TCAM_RULE_SIZE_ENT        *sizePtr,
    OUT GT_U32                              tcamKeyArr[],
    OUT GT_U32                              *subKeyProfilePtr
);

/**
* @internal prvTgfTcamFloorsNumGet function
* @endinternal
*
* @brief   Return number of TCAM floors
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; AC5; Lion; xCat2; Lion2.
*
*                                       number of TCAM floors
*/
GT_U32 prvTgfTcamFloorsNumGet
(
    GT_VOID
);

/**
* @internal prvTgfTcamCpuLookupResultsGet function
* @endinternal
*
* @brief   Gets CPU lookup results.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
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
GT_STATUS prvTgfTcamCpuLookupResultsGet
(
    IN  GT_U8                               devNum,
    IN  GT_PORT_GROUPS_BMP                  portGroupsBmp,
    IN  GT_U32                              group,
    IN  GT_U32                              hitNum,
    OUT GT_BOOL                            *isValidPtr,
    OUT GT_BOOL                            *isHitPtr,
    OUT GT_U32                             *hitIndexPtr
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __tgfTcamGenh */

