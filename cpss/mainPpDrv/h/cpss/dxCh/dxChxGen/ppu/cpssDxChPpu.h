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
* @file cpssDxChPpu.h
*
* @brief CPSS DXCH PPU lib API declaration.
*
* @version   1
********************************************************************************
*/
#ifndef __cpssDxChPpuh
#define __cpssDxChPpuh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#include <cpss/dxCh/dxChxGen/ppu/cpssDxChPpuTypes.h>
#include <cpss/dxCh/dxChxGen/pcl/cpssDxChPcl.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwTables.h>
#include <cpss/dxCh/dxChxGen/tcam/private/prvCpssDxChTcam.h>

/**
* @internal cpssDxChPpuEnableSet function
* @endinternal
*
* @brief    This function enables or disables PPU block
*
* @note   APPLICABLE DEVICES:  AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; BobCat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                 - device number
* @param[in] enable                 - 0 = disable, 1 = enable.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on failure
*/
GT_STATUS cpssDxChPpuEnableSet
(
    IN  GT_U8       devNum,
    IN  GT_BOOL     enable
);

/**
* @internal cpssDxChPpuEnableGet function
* @endinternal
*
* @brief    This function gets ppu enable status
*
* @note   APPLICABLE DEVICES:  AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; BobCat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                 - device number
* @param[out] enablePtr             - pointer to enable status. 0 = disable, 1 = enable.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on failure
*/
GT_STATUS cpssDxChPpuEnableGet
(
    IN  GT_U8       devNum,
    OUT GT_BOOL     *enablePtr
);

/**
* @internal cpssDxChPpuMaxLoopbackSet function
* @endinternal
*
* @brief    This function sets the maximum allowed loopbacks
*
* @note   APPLICABLE DEVICES:  AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; BobCat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                 - device number
* @param[in] maxLoopback                    - maximum number of loopbacks allowed
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on failure
*/
GT_STATUS cpssDxChPpuMaxLoopbackSet
(
    IN  GT_U8       devNum,
    IN  GT_U32      maxLoopback
);

/**
* @internal cpssDxChPpuMaxLoopbackGet function
* @endinternal
*
* @brief    This function gets the maximum allowed loopbacks
*
* @note   APPLICABLE DEVICES:  AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; BobCat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                 - device number
* @param[out] maxLoopbackPtr        - pointer to maximum number of loopbacks allowed
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on failure
*/
GT_STATUS cpssDxChPpuMaxLoopbackGet
(
    IN  GT_U8       devNum,
    OUT GT_U32      *maxLoopbackPtr
);

/**
* @internal cpssDxChPpuGlobalConfigSet function
* @endinternal
*
* @brief    This function sets the Global configuration Register
*
* @note   APPLICABLE DEVICES:  AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; BobCat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                 - device number
* @param[in] ppuGlobalConfigPtr     - pointer to global configuration
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note NONE
*
*/
GT_STATUS cpssDxChPpuGlobalConfigSet
(
    IN  GT_U8                           devNum,
    IN  CPSS_DXCH_PPU_GLOBAL_CONFIG_STC *ppuGlobalConfigPtr
);

/**
* @internal cpssDxChPpuGlobalConfigGet function
* @endinternal
*
* @brief    This function gets the Global configuration Register
*
* @note   APPLICABLE DEVICES:  AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; BobCat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                 - device number
* @param[out] ppuGlobalConfigPtr    - pointer to global configuration
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note NONE
*
*/
GT_STATUS cpssDxChPpuGlobalConfigGet
(
    IN  GT_U8                            devNum,
    OUT CPSS_DXCH_PPU_GLOBAL_CONFIG_STC  *ppuGlobalConfigPtr
);

/**
* @internal cpssDxChPpuTtiActionProfileIndexSet function
* @endinternal
*
* @brief    set ppu profile index in tti action
*
* @note   APPLICABLE DEVICES:  AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; BobCat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                 - device number
* @param[in] ttiRuleIndex    - PPU profile index pointer by TTI action.
* @param[in] ppuProfileNum          - PPU profile table index
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note NONE
*
*/
GT_STATUS cpssDxChPpuTtiActionProfileIndexSet
(
    IN  GT_U8                               devNum,
    IN  GT_U32                              ttiRuleIndex,
    IN  GT_U32                              profileNum
);

/**
* @internal cpssDxChPpuTtiActionProfileIndexGet function
* @endinternal
*
* @brief    Get ppu profile index in tti action
*
* @note   APPLICABLE DEVICES:  AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; BobCat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                 - device number
* @param[in] ttiRuleIndex           - PPU profile index pointer by TTI action.
* @param[out] profileNumPtr         - pointer to PPU profile table index
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note NONE
*
*/
GT_STATUS cpssDxChPpuTtiActionProfileIndexGet
(
    IN  GT_U8                               devNum,
    IN  GT_U32                              ttiRuleIndex,
    OUT  GT_U32                              *profileNumPtr
);

/**
* @internal cpssDxChPpuSrcPortProfileIndexSet function
* @endinternal
*
* @brief  Set default PPU profile index for srouce port
*
* @note   APPLICABLE DEVICES:  AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; BobCat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                 - device number
* @param[in] srcPortNum             - source port number
* @param[in] profileNum             - ppu profile index.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note NONE
*
*/
GT_STATUS cpssDxChPpuSrcPortProfileIndexSet
(
    IN  GT_U8                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                srcPortNum,
    IN  GT_U32                              profileNum
);

/**
* @internal cpssDxChPpuSrcPortProfileIndexGet function
* @endinternal
*
* @brief  Get default PPU profile index for srouce port
*
* @note   APPLICABLE DEVICES:  AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; BobCat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                 - device number
* @param[in] srcPortNum             - source port number
* @param[out] profileNumPtr          - pointer to ppu profile index.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note NONE
*
*/
GT_STATUS cpssDxChPpuSrcPortProfileIndexGet
(
    IN  GT_U8                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                srcPortNum,
    OUT GT_U32                              *profileNumPtr
);

/**
* @internal cpssDxChPpuProfileSet function
* @endinternal
*
* @brief    This function sets PPU profile table entry for a given profile index.
*
* @note   APPLICABLE DEVICES:  AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; BobCat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                 - device number
* @param[in] profileIdx             - PPU profile index
* @param[in] ppuProfilePtr          - pointer to PPU profile.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note NONE
*
*/
GT_STATUS cpssDxChPpuProfileSet
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          profileIdx,
    IN  CPSS_DXCH_PPU_PROFILE_ENTRY_STC *ppuProfilePtr
);

/**
* @internal cpssDxChPpuProfileGet function
* @endinternal
*
* @brief    This function gets PPU profile table entry for a given profile index.
*
* @note   APPLICABLE DEVICES:  AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; BobCat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                 - device number
* @param[in] profileIdx             - PPU profile index
* @param[out] ppuProfilePtr         - pointer to PPU profile.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note NONE
*
*/
GT_STATUS cpssDxChPpuProfileGet
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          profileIdx,
    OUT CPSS_DXCH_PPU_PROFILE_ENTRY_STC *ppuProfilePtr
);

/**
* @internal cpssDxChPpuSpBusDefaultProfileSet function
* @endinternal
*
* @brief    This function sets SP_BUS default profile for a given profile number.
*
* @note   APPLICABLE DEVICES:  AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; BobCat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                 - device number
* @param[in] profileNum             - profile number
* @param[in] spBusProfilePtr        - pointer to SP_BUS profile data.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note NONE
*
*/
GT_STATUS cpssDxChPpuSpBusDefaultProfileSet
(
    IN  GT_U8                    devNum,
    IN  GT_U32                   profileNum,
    IN  CPSS_DXCH_PPU_SP_BUS_STC *spBusProfilePtr
);

/**
* @internal cpssDxChPpuSpBusDefaultProfileGet function
* @endinternal
*
* @brief    This function gets SP_BUS default profile for a given profile number.
*
* @note   APPLICABLE DEVICES:  AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; BobCat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                 - device number
* @param[in] profileNum             - profile number
* @param[out] spBusProfilePtr       - pointer to SP_BUS profile data.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note NONE
*
*/
GT_STATUS cpssDxChPpuSpBusDefaultProfileGet
(
    IN  GT_U8                    devNum,
    IN  GT_U32                   profileNum,
    OUT CPSS_DXCH_PPU_SP_BUS_STC *spBusProfile
);

/**
* @internal cpssDxChPpuKstgKeyGenProfileSet function
* @endinternal
*
* @brief    This function sets K_STG key generation profile for a given profile index
*
* @note   APPLICABLE DEVICES:  AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; BobCat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                 - device number
* @param[in] kstgNum                - K_stg number
* @param[in] profileIdx             - Key generation profile index
* @param[in] keygenProfilePtr       - pointer to key generation profile
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note NONE
*
*/
GT_STATUS cpssDxChPpuKstgKeyGenProfileSet
(
    IN  GT_U8                                   devNum,
    IN  GT_U32                                  kstgNum,
    IN  GT_U32                                  profileIdx,
    IN  CPSS_DXCH_PPU_KSTG_KEY_GEN_PROFILE_STC  *keygenProfilePtr
);

/**
* @internal cpssDxChPpuKstgKeyGenProfileGet function
* @endinternal
*
* @brief    This function gets K_STG key generation profile for a given profile number.
*
* @note   APPLICABLE DEVICES:  AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; BobCat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                 - device number
* @param[in] kstgNum                - K_stg number
* @param[in] profileIdx             - Key generation profile index
* @param[out] keygenProfilePtr      - pointer to key generation profile
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note NONE
*
*/
GT_STATUS cpssDxChPpuKstgKeyGenProfileGet
(
    IN  GT_U8                                   devNum,
    IN  GT_U32                                  kstgNum,
    IN  GT_U32                                  profileIdx,
    OUT CPSS_DXCH_PPU_KSTG_KEY_GEN_PROFILE_STC  *keygenProfilePtr
);

/**
* @internal cpssDxChPpuKstgTcamEntrySet function
* @endinternal
*
* @brief    This function sets K_STG TCAM entry for a given index
*
* @note   APPLICABLE DEVICES:  AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; BobCat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                 - device number
* @param[in] kstgNum                - K_stg number
* @param[in] index                  - tcam entry index
* @param[in] tcamEntryPtr           - pointer to tcam entry
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note NONE
*
*/
GT_STATUS cpssDxChPpuKstgTcamEntrySet
(
    IN  GT_U8                               devNum,
    IN  GT_U32                              kstgNum,
    IN  GT_U32                              profileIdx,
    IN  CPSS_DXCH_PPU_KSTG_TCAM_ENTRY_STC   *tcamEntryPtr
);

/**
* @internal cpssDxChPpuKstgTcamEntryGet function
* @endinternal
*
* @brief    This function gets K_STG TCAM entry for a given index
*
* @note   APPLICABLE DEVICES:  AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; BobCat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                 - device number
* @param[in] kstgNum                - K_stg number
* @param[in] index                  - tcam entry index
* @param[out] tcamEntryPtr          - pointer to tcam entry
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note NONE
*
*/
GT_STATUS cpssDxChPpuKstgTcamEntryGet
(
    IN  GT_U8                               devNum,
    IN  GT_U32                              kstgNum,
    IN  GT_U32                              profileIdx,
    OUT CPSS_DXCH_PPU_KSTG_TCAM_ENTRY_STC   *tcamEntryPtr
);

/**
* @internal cpssDxChPpuActionTableEntrySet function
* @endinternal
*
* @brief    This function writes PPU action table entry.
*
* @note   APPLICABLE DEVICES:  AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; BobCat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                 - device number
* @param[in] kstgNum                - K_stg number
* @param[in] index                  - entry index
* @param[in] ppuActionEntryPtr      - pointer to PPU action profile entry.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note NONE
*
*/
GT_STATUS cpssDxChPpuActionTableEntrySet
(
    IN  GT_U8                               devNum,
    IN  GT_U32                              kstgNum,
    IN  GT_U32                              index,
    IN  CPSS_DXCH_PPU_KSTG_ACTION_ENTRY_STC *ppuActionProfilePtr
);

/**
* @internal cpssDxChPpuActionTableEntryGet function
* @endinternal
*
* @brief    This function reads PPU action table entry.
*
* @note   APPLICABLE DEVICES:  AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; BobCat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                 - device number
* @param[in] kstgNum                - K_stg number
* @param[in] index                  - entry index
* @param[out] ppuActionEntryPtr     - pointer to PPU action profile entry.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note NONE
*
*/
GT_STATUS cpssDxChPpuActionTableEntryGet
(
    IN  GT_U8                               devNum,
    IN  GT_U32                              kstgNum,
    IN  GT_U32                              index,
    OUT CPSS_DXCH_PPU_KSTG_ACTION_ENTRY_STC *ppuActionEntryPtr
);

/**
* @internal  cpssDxChPpuDauProfileEntrySet function
* @endinternal
*
* @brief    This function sets DAU profile entry.
*
* @note   APPLICABLE DEVICES:  AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; BobCat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                 - device number
* @param[in] profileIndex           - DAU profile index
* @param[in] dauProfilePtr          - pointer to PPU DAU profile entry data.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note NONE
*
*/
GT_STATUS cpssDxChPpuDauProfileEntrySet
(
    IN  GT_U8                               devNum,
    IN  GT_U32                              profileIndex,
    IN  CPSS_DXCH_PPU_DAU_PROFILE_ENTRY_STC *dauProfilePtr
);

/**
* @internal cpssDxChPpuDauProfileEntryGet function
* @endinternal
*
* @brief    This function reads DAU profile table entry.
*
* @note   APPLICABLE DEVICES:  AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; BobCat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                 - device number
* @param[in] profileIndex           - DAU profile index
* @param[out] dauProfilePtr         - pointer to PPU DAU profile entry data;
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note NONE
*
*/
GT_STATUS cpssDxChPpuDauProfileEntryGet
(
    IN  GT_U8                               devNum,
    IN  GT_U32                              profileIndex,
    OUT CPSS_DXCH_PPU_DAU_PROFILE_ENTRY_STC *dauProfilePtr
);

/**
* @internal  cpssDxChPpuDauProtectedWindowSet function
* @endinternal
*
* @brief    This function sets DAU protected window table entry.
*
* @note   APPLICABLE DEVICES:  AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; BobCat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                 - device number
* @param[in] index                  - index to DAU protected window table entry.
* @param[in] protWinPtr             - pointer to PPU DAU protected window table entry.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note NONE
*
*/
GT_STATUS cpssDxChPpuDauProtectedWindowSet
(
    IN  GT_U8                                       devNum,
    IN  GT_U32                                      index,
    IN  CPSS_DXCH_PPU_DAU_DATA_PROTECTED_WINDOW_STC *protWinPtr
);

/**
* @internal  cpssDxChPpuDauProtectedWindowGet function
* @endinternal
*
* @brief    This function gets DAU protected window table entry.
*
* @note   APPLICABLE DEVICES:  AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; BobCat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                 - device number
* @param[in] index                  - index to DAU protected window table entry.
* @param[out] protWinPtr            - pointer to PPU DAU protected window table entry.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note NONE
*
*/
GT_STATUS cpssDxChPpuDauProtectedWindowGet
(
    IN  GT_U8                                       devNum,
    IN  GT_U32                                      index,
    OUT CPSS_DXCH_PPU_DAU_DATA_PROTECTED_WINDOW_STC *protWinPtr
);

/**
* @internal cpssDxChPpuErrorProfileSet function
* @endinternal
*
* @brief    This function gets error profile for a given profile and descriptor field.
*
* @note   APPLICABLE DEVICES:  AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; BobCat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                 - device number
* @param[in] profileNum             - profile number
* @param[in] errorProfilePtr       - pointer to PPU error profile.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note NONE
*
*/
GT_STATUS cpssDxChPpuErrorProfileSet
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          profileNum,
    IN CPSS_DXCH_PPU_ERROR_PROFILE_STC  *errorProfilePtr
);

/**
* @internal cpssDxChPpuErrorProfileGet function
* @endinternal
*
* @brief    This function gets error profile for a given profile and descriptor field.
*
* @note   APPLICABLE DEVICES:  AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; BobCat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                 - device number
* @param[in] profileNum             - profile number
* @param[out] errorProfilePtr       - pointer to PPU error profile.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note NONE
*
*/
GT_STATUS cpssDxChPpuErrorProfileGet
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          profileNum,
    OUT CPSS_DXCH_PPU_ERROR_PROFILE_STC *errorProfilePtr
);

/**
* @internal cpssDxChPpuDebugCountersGet function
* @endinternal
*
* @brief    This function reads the debug counters.
*
* @note   APPLICABLE DEVICES:  AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; BobCat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in]  devNum               - device number
* @param[out] dbgCountersPtr       - pointer to debug counters
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note NONE
*
*/
GT_STATUS cpssDxChPpuDebugCountersGet
(
    IN  GT_U8                            devNum,
    OUT CPSS_DXCH_PPU_DEBUG_COUNTERS_STC *dbgCountersPtr
);

/**
* @internal cpssDxChPpuDebugCountersClear function
* @endinternal
*
* @brief    This function clears debug counters and sets to zero.
*
* @note   APPLICABLE DEVICES:  AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; BobCat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                 - device number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note NONE
*
*/
GT_STATUS cpssDxChPpuDebugCountersClear
(
    IN  GT_U8   devNum
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssDxChPpuh */


