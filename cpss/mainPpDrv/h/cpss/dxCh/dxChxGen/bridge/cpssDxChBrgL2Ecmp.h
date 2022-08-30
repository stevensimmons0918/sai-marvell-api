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
* @file cpssDxChBrgL2Ecmp.h
*
* @brief L2 ECMP facility CPSS DxCh implementation.
*
* @version   10
********************************************************************************
*/
#ifndef __cpssDxChBrgL2Ecmph
#define __cpssDxChBrgL2Ecmph

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/cpssTypes.h>
#include <cpss/dxCh/dxChxGen/cpssDxChTypes.h>


/**
* @struct CPSS_DXCH_BRG_L2_ECMP_LTT_ENTRY_STC
 *
 * @brief L2 ECMP LTT Entry
*/
typedef struct{

    /** @brief Determines the start index of the L2 ECMP block.
     *  (APPLICABLE RANGES: 0..8191)
     *  NOTE: When trunk mode is 'CPSS_DXCH_TRUNK_MEMBERS_MODE_FLEX_E' the CPSS allows
     *  to point to all 'L2 ECMP' table entries.
     *  Otherwise the CPSS allows to point ONLY to the lower half of 'L2 ECMP' table entries.
     */
    GT_U32 ecmpStartIndex;

    /** @brief The number of paths in the ECMP block:
     *  0x1 = 1 path, 0x2 = 2 paths and so on.
     *  (APPLICABLE RANGES: 1..4096)
     */
    GT_U32 ecmpNumOfPaths;

    /** Determines whether this packet is load balanced over an ECMP group. */
    GT_BOOL ecmpEnable;

    /** allows random load balance */
    GT_BOOL ecmpRandomPathEnable;

    /** @brief  Defines index of the Hash Bit Selection Profile
     *         (APPLICABLE DEVICES: AC5P, AC5X, Harrier, Ironman)
     *         (APPLICABLE RANGES: 0..15)
     */
    GT_U32 hashBitSelectionProfile;

} CPSS_DXCH_BRG_L2_ECMP_LTT_ENTRY_STC;

/**
* @struct CPSS_DXCH_BRG_L2_ECMP_ENTRY_STC
 *
 * @brief L2 ECMP Entry
*/
typedef struct{

    /** target ePort */
    GT_PORT_NUM targetEport;

    /** target HW device */
    GT_HW_DEV_NUM targetHwDevice;

} CPSS_DXCH_BRG_L2_ECMP_ENTRY_STC;

/**
* @enum CPSS_DXCH_BRG_L2_ECMP_MLL_REPLICATION_MODE_ENT
 *
 * @brief Enumeration describes MLL L2 ECMP Replication Mode
*/
typedef enum{

    /** @brief MLL replication is forwarded to the L2 ECMP member
     *  selected regardless of the target device.
     */
    CPSS_DXCH_BRG_L2_ECMP_MLL_REPLICATION_MODE_NORMAL_E,

    /** @brief MLL replication is dropped if the target device
     *  of the L2 ECMP member selected is not the local device
     */
    CPSS_DXCH_BRG_L2_ECMP_MLL_REPLICATION_MODE_CASCADED_SYSTEM_E

} CPSS_DXCH_BRG_L2_ECMP_MLL_REPLICATION_MODE_ENT;

/**
* @internal cpssDxChBrgL2EcmpEnableSet function
* @endinternal
*
* @brief   Globally enable/disable L2 ECMP (AKA ePort ECMP)
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] enable                   - L2 ECMP status:
*                                      GT_TRUE: enable L2 ECMP
*                                      GT_FALSE: disable L2 ECMP
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgL2EcmpEnableSet
(
    IN  GT_U8       devNum,
    IN  GT_BOOL     enable
);

/**
* @internal cpssDxChBrgL2EcmpEnableGet function
* @endinternal
*
* @brief   Get the global enable/disable L2 ECMP (AKA ePort ECMP)
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                - (pointer to) L2 ECMP  status:
*                                      GT_TRUE : L2 ECMP is enabled
*                                      GT_FALSE: L2 ECMP is disabled
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgL2EcmpEnableGet
(
    IN  GT_U8       devNum,
    OUT GT_BOOL     *enablePtr
);


/**
* @internal cpssDxChBrgL2EcmpIndexBaseEportSet function
* @endinternal
*
* @brief   Defines the first ePort number in the L2 ECMP ePort number range.
*         The index to the L2 ECMP LTT is <Target ePort>-<Base ePort>.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] ecmpIndexBaseEport       - the ePort number
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgL2EcmpIndexBaseEportSet
(
    IN GT_U8            devNum,
    IN GT_PORT_NUM      ecmpIndexBaseEport
);

/**
* @internal cpssDxChBrgL2EcmpIndexBaseEportGet function
* @endinternal
*
* @brief   Return the first ePort number in the L2 ECMP ePort number range.
*         The index to the L2 ECMP LTT is <Target ePort>-<Base ePort>.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
*
* @param[out] ecmpIndexBaseEportPtr    - (pointer to) the ePort number
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgL2EcmpIndexBaseEportGet
(
    IN  GT_U8           devNum,
    OUT GT_PORT_NUM     *ecmpIndexBaseEportPtr
);

/**
* @internal cpssDxChBrgL2EcmpLttTableSet function
* @endinternal
*
* @brief   Set L2 ECMP LTT entry info.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] index                    - the LTT index
* @param[in] ecmpLttInfoPtr           - (pointer to) L2 ECMP LTT Information
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong parameters
* @retval GT_OUT_OF_RANGE          - on out of range hashBitSelectionProfile, ecmpStartIndex or ecmpNumOfPaths
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The index to the LTT is the result of Target ePort - Base ePort.
*       Base ePort may be configured by cpssDxChBrgL2EcmpIndexBaseEportSet.
*
*/
GT_STATUS cpssDxChBrgL2EcmpLttTableSet
(
    IN GT_U8                                devNum,
    IN GT_U32                               index,
    IN CPSS_DXCH_BRG_L2_ECMP_LTT_ENTRY_STC  *ecmpLttInfoPtr
);

/**
* @internal cpssDxChBrgL2EcmpLttTableGet function
* @endinternal
*
* @brief   Get L2 ECMP LTT entry info.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] index                    - the LTT index
*
* @param[out] ecmpLttInfoPtr           - (pointer to) L2 ECMP LTT Information
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong parameters
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The index to the LTT is the result of Target ePort - Base ePort.
*       Base ePort may be configured by cpssDxChBrgL2EcmpIndexBaseEportSet.
*
*/
GT_STATUS cpssDxChBrgL2EcmpLttTableGet
(
    IN  GT_U8                                devNum,
    IN  GT_U32                               index,
    OUT CPSS_DXCH_BRG_L2_ECMP_LTT_ENTRY_STC  *ecmpLttInfoPtr
);

/**
* @internal cpssDxChBrgL2EcmpTableSet function
* @endinternal
*
* @brief   Set L2 ECMP entry info.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] index                    - the ECMP entry index
* @param[in] ecmpEntryPtr             - (pointer to) L2 ECMP entry.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong parameters
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note 1. Index according to index from L2 ECMP LTT
*       2. When trunk mode is 'CPSS_DXCH_TRUNK_MEMBERS_MODE_FLEX_E' the CPSS allows
*       access to all table entries.
*       Otherwise the CPSS allows access to ONLY to lower half of table entries.
*
*/
GT_STATUS cpssDxChBrgL2EcmpTableSet
(
    IN GT_U8                                devNum,
    IN GT_U32                               index,
    IN CPSS_DXCH_BRG_L2_ECMP_ENTRY_STC     *ecmpEntryPtr
);

/**
* @internal cpssDxChBrgL2EcmpTableGet function
* @endinternal
*
* @brief   Get L2 ECMP entry info.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] index                    - the ECMP entry index
*
* @param[out] ecmpEntryPtr             - (pointer to) L2 ECMP entry.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong parameters
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note 1. Index according to index from L2 ECMP LTT
*       2. When trunk mode is 'CPSS_DXCH_TRUNK_MEMBERS_MODE_FLEX_E' the CPSS allows
*       access to all table entries.
*       Otherwise the CPSS allows access to ONLY to lower half of table entries.
*
*/
GT_STATUS cpssDxChBrgL2EcmpTableGet
(
    IN  GT_U8                                devNum,
    IN  GT_U32                               index,
    OUT CPSS_DXCH_BRG_L2_ECMP_ENTRY_STC     *ecmpEntryPtr
);

/**
* @internal cpssDxChBrgL2EcmpMemberSelectionModeSet function
* @endinternal
*
* @brief   Set mode used to calculate the Index of the secondary ePort (L2 ECMP member)
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] selectionMode            - member selection mode
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The selection mode configuration may be changed by:
*       - cpssDxChTrunkHashNumBitsSet.
*       - cpssDxChTrunkHashGlobalModeSet.
*       - cpssDxChTrunkInit.
*       - cpssDxChTrunkHashCrcParametersSet.
*
*/
GT_STATUS cpssDxChBrgL2EcmpMemberSelectionModeSet
(
    IN GT_U8                                    devNum,
    IN CPSS_DXCH_MEMBER_SELECTION_MODE_ENT      selectionMode
);

/**
* @internal cpssDxChBrgL2EcmpMemberSelectionModeGet function
* @endinternal
*
* @brief   Get mode used to calculate the Index of the secondary ePort (L2 ECMP member)
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
*
* @param[out] selectionModePtr         - (pointer to) member selection mode
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgL2EcmpMemberSelectionModeGet
(
    IN  GT_U8                                   devNum,
    OUT CPSS_DXCH_MEMBER_SELECTION_MODE_ENT     *selectionModePtr
);

/**
* @internal cpssDxChBrgL2EcmpHashBitSelectionProfileSet function
* @endinternal
*
* @brief   Set L2 ECMP hash bit selection profile information.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                          - device number
* @param[in] profileIndex                    - the profile index
*                                              (APPLICABLE RANGES: 0..15)
* @param[in] hashBitSelectionProfileInfoPtr  - (pointer to) L2 ECMP hash bit selection profile information
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong parameters
* @retval GT_OUT_OF_RANGE          - on out of range firstBit, lastBit or salt
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note  Hash bit selection profile index can also be changed by:
*        cpssDxChBrgL2EcmpLttTableSet for L2 ECMP Client
*        cpssDxChTrunkEcmpLttTableSet  and cpssDxChTrunkHashBitSelectionProfileIndexSet
*        for trunk ECMP client.
*
*/
GT_STATUS cpssDxChBrgL2EcmpHashBitSelectionProfileSet
(
    IN GT_U8                                    devNum,
    IN GT_U32                                   profileIndex,
    IN CPSS_DXCH_HASH_BIT_SELECTION_PROFILE_STC  *hashBitSelectionProfileInfoPtr
);

/**
* @internal cpssDxChBrgL2EcmpHashBitSelectionProfileGet function
* @endinternal
*
* @brief   Get L2 ECMP hash bit selection profile information.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                           - device number
* @param[in] profileIndex                     - the profile index
*                                               (APPLICABLE RANGES: 0..15)
*
* @param[out] hashBitSelectionProfileInfoPtr  - (pointer to) L2 ECMP hash bit selection profile information
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChBrgL2EcmpHashBitSelectionProfileGet
(
    IN   GT_U8                                     devNum,
    IN   GT_U32                                    profileIndex,
    OUT  CPSS_DXCH_HASH_BIT_SELECTION_PROFILE_STC  *hashBitSelectionProfileInfoPtr
);

/**
* @internal cpssDxChBrgL2EcmpPortHashSet function
* @endinternal
*
* @brief  Set hash value per port
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2;Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                  - device number
* @param[in] portNum                 - port number
* @param[in] srcPortHash             - hash value to be used
*                                      (APPLICABLE RANGES: 0..0xFFFF)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_OUT_OF_RANGE          - out of range value for srcPortHash
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgL2EcmpPortHashSet
(
    IN GT_U8                        devNum,
    IN GT_PHYSICAL_PORT_NUM         portNum,
    IN GT_U32                       srcPortHash
);

/**
* @internal cpssDxChBrgL2EcmpPortHashGet function
* @endinternal
*
* @brief  Get hash value per port
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2;Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                - device number
* @param[in] portNum               - port number
*
* @param[out] srcPortHashPtr       - (pointer to)source port hash value
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgL2EcmpPortHashGet
(
    IN  GT_U8                        devNum,
    IN  GT_PHYSICAL_PORT_NUM         portNum,
    OUT GT_U32                       *srcPortHashPtr
);

/**
* @internal cpssDxChBrgL2EcmpPortHashBitSelectionSet function
* @endinternal
*
* @brief   Set the number of bits to be used for load balancing selection computation.
*          This indicates the number of right shift bits needed when
*          <source_port_hash_enable> = TRUE
*
* @note   APPLICABLE DEVICES:     AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES: xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                      - device number
* @param[in] numBitsInHash               - number of bits to be selected
*                                          (APPLICABLE RANGES: 0..31)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_OUT_OF_RANGE          - on out of range value for numBitsInHash
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgL2EcmpPortHashBitSelectionSet
(
    IN  GT_U8      devNum,
    IN  GT_U32     numBitsInHash
);

/**
* @internal cpssDxChBrgL2EcmpPortHashBitSelectionGet function
* @endinternal
*
* @brief         Get the number of bits to be used for load balancing selection computation.
*                This indicates the number of right shift bits needed when
*                <source_port_hash_enable> = TRUE
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2;Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                - device number
*
* @param[out] numBitsInHashPtr     - (pointer to) number of bits to be selected
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgL2EcmpPortHashBitSelectionGet
(
    IN  GT_U8      devNum,
    OUT GT_U32     *numBitsInHashPtr
);

/**
* @internal cpssDxChBrgL2EcmpMllReplicationConfigSet function
* @endinternal
*
* @brief   Set L2 ECMP MLL Replication Mode and Replication Drop Code.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2;Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                - device number
*
* @param[in] replicationMode       - MLL L2 ECMP Replication Mode
*
* @param[in] replicationDropCode   - The Drop Code assignment if the <MLL L2 ECMP Replication Mode> is set
*                                    to Mode CPSS_DXCH_BRG_L2_ECMP_MLL_REPLICATION_MODE_CASCADED_SYSTEM_E
*                                    and L2 ECMP member selection has a Target Device != Local Device
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, replicationMode and replicationDropCode
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgL2EcmpMllReplicationConfigSet
(
    IN  GT_U8                                            devNum,
    IN  CPSS_DXCH_BRG_L2_ECMP_MLL_REPLICATION_MODE_ENT   replicationMode,
    IN  CPSS_NET_RX_CPU_CODE_ENT                         replicationDropCode
);

/**
* @internal cpssDxChBrgL2EcmpMllReplicationConfigGet function
* @endinternal
*
* @brief   Get status of L2 ECMP MLL Replication Mode and Replication Drop Code.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2;Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                    - device number
*
* @param[out] replicationModePtr       - (pointer to) MLL L2 ECMP Replication Mode
*
* @param[out] replicationDropCodePtr   - (pointer to) The Drop Code assignment if the <MLL L2 ECMP Replication Mode> is set
*                                                     to Mode CPSS_DXCH_BRG_L2_ECMP_MLL_REPLICATION_MODE_CASCADED_SYSTEM_E
*                                                     and L2 ECMP member selection has a Target Device != Local Device
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgL2EcmpMllReplicationConfigGet
(
    IN   GT_U8                                            devNum,
    OUT  CPSS_DXCH_BRG_L2_ECMP_MLL_REPLICATION_MODE_ENT   *replicationModePtr,
    OUT  CPSS_NET_RX_CPU_CODE_ENT                         *replicationDropCodePtr
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssDxChBrgL2Ecmph */


