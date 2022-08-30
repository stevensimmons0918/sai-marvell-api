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
* @file prvCpssDxChPpu.h
*
* @brief CPSS DXCH private PPU lib API declaration.
*
* @version   50
********************************************************************************
*/
#ifndef __prvCpssDxChPpuh
#define __prvCpssDxChPpuh

#include <cpss/dxCh/dxChxGen/pcl/cpssDxChPcl.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwTables.h>
#include <cpss/dxCh/dxChxGen/tcam/private/prvCpssDxChTcam.h>
#include <cpss/dxCh/dxChxGen/ppu/cpssDxChPpuTypes.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
* @internal prvCpssDxChPpuEnableSet function
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
GT_STATUS prvCpssDxChPpuEnableSet
(
    IN  GT_U8       devNum,
    IN  GT_BOOL     enable
);

/**
* @internal prvCpssDxChPpuEnableGet function
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
GT_STATUS prvCpssDxChPpuEnableGet
(
    IN  GT_U8       devNum,
    OUT GT_BOOL     *enablePtr
);

/**
* @internal prvCpssDxChPpuMaxLoopbackSet function
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
GT_STATUS prvCpssDxChPpuMaxLoopbackSet
(
    IN  GT_U8       devNum,
    IN  GT_U32      maxLoopback
);

/**
* @internal prvCpssDxChPpuMaxLoopbackGet function
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
GT_STATUS prvCpssDxChPpuMaxLoopbackGet
(
    IN  GT_U8       devNum,
    OUT GT_U32      *maxLoopbackPtr
);

/**
* @internal prvCpssDxChPpuTtiActionProfileIndexSet function
* @endinternal
*
* @brief    set ppu profile index in tti action
*
* @note   APPLICABLE DEVICES:  AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; BobCat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                 - device number
* @param[in] ttiActionEntryIndex    - PPU profile index pointer by TTI action.
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
GT_STATUS prvCpssDxChPpuTtiActionProfileIndexSet
(
    IN  GT_U8                               devNum,
    IN  GT_U32                              ttiActionEntryIndex,
    IN  GT_U32                              profileNum
);

/**
* @internal prvCpssDxChPpuTtiActionProfileIndexGet function
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
GT_STATUS prvCpssDxChPpuTtiActionProfileIndexGet
(
    IN  GT_U8                               devNum,
    IN  GT_U32                              ttiRuleIndex,
    OUT GT_U32                              *profileNumPtr
);

/**
* @internal prvcpssDxChPpuSrcPortProfileIndexSet function
* @endinternal
*
* @brief  Set default PPU profile index for source port
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
GT_STATUS prvcpssDxChPpuSrcPortProfileIndexSet
(
    IN  GT_U8                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                srcPortNum,
    IN  GT_U32                              profileNum
);

/**
* @internal prvCpssDxChPpuSrcPortProfileIndexGet  function
* @endinternal
*
* @brief  Get default PPU profile index for source port
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
GT_STATUS prvCpssDxChPpuSrcPortProfileIndexGet
(
    IN  GT_U8                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                srcPortNum,
    OUT GT_U32                              *profileNumPtr
);

/**
* @internal prvCpssDxChPpuGlobalConfigSet function
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
GT_STATUS prvCpssDxChPpuGlobalConfigSet
(
    IN  GT_U8                           devNum,
    IN  CPSS_DXCH_PPU_GLOBAL_CONFIG_STC *ppuGlobalConfigPtr
);

/**
* @internal prvCpssDxChPpuGlobalConfigGet function
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
GT_STATUS prvCpssDxChPpuGlobalConfigGet
(
    IN  GT_U8                            devNum,
    OUT CPSS_DXCH_PPU_GLOBAL_CONFIG_STC  *ppuGlobalConfigPtr
);

/**
* @internal prvCpssDxChPpuProfileSet function
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
GT_STATUS prvCpssDxChPpuProfileSet
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          profileIdx,
    IN  CPSS_DXCH_PPU_PROFILE_ENTRY_STC *ppuProfilePtr
);

/**
* @internal prvCpssDxChPpuProfileGet function
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
GT_STATUS prvCpssDxChPpuProfileGet
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          profileIdx,
    OUT CPSS_DXCH_PPU_PROFILE_ENTRY_STC *ppuProfilePtr
);

/**
* @internal prvCpssDxChPpuSpBusDefaultProfileSet function
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
GT_STATUS prvCpssDxChPpuSpBusDefaultProfileSet
(
    IN  GT_U8                    devNum,
    IN  GT_U32                   profileNum,
    IN  CPSS_DXCH_PPU_SP_BUS_STC *spBusProfilePtr
);

/**
* @internal prvCpssDxChPpuSpBusDefaultProfileGet function
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
GT_STATUS prvCpssDxChPpuSpBusDefaultProfileGet
(
    IN  GT_U8                    devNum,
    IN  GT_U32                   profileNum,
    OUT CPSS_DXCH_PPU_SP_BUS_STC *spBusProfile
);

/**
* @internal prvCpssDxChPpuKstgKeyGenProfileSet function
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
GT_STATUS prvCpssDxChPpuKstgKeyGenProfileSet
(
    IN  GT_U8                                   devNum,
    IN  GT_U32                                  kstgNum,
    IN  GT_U32                                  profileIdx,
    IN  CPSS_DXCH_PPU_KSTG_KEY_GEN_PROFILE_STC  *keygenProfilePtr
);

/**
* @internal prvCpssDxChPpuKstgKeyGenProfileGet function
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
GT_STATUS prvCpssDxChPpuKstgKeyGenProfileGet
(
    IN  GT_U8                                   devNum,
    IN  GT_U32                                  kstgNum,
    IN  GT_U32                                  profileIdx,
    OUT CPSS_DXCH_PPU_KSTG_KEY_GEN_PROFILE_STC  *keygenProfilePtr
);

/**
* @internal prvCpssDxChPpuKstgTcamEntrySet function
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
GT_STATUS prvCpssDxChPpuKstgTcamEntrySet
(
    IN  GT_U8                               devNum,
    IN  GT_U32                              kstgNum,
    IN  GT_U32                              profileIdx,
    IN  CPSS_DXCH_PPU_KSTG_TCAM_ENTRY_STC   *tcamEntryPtr
);

/**
* @internal prvCpssDxChPpuKstgTcamEntryGet function
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
GT_STATUS prvCpssDxChPpuKstgTcamEntryGet
(
    IN  GT_U8                               devNum,
    IN  GT_U32                              kstgNum,
    IN  GT_U32                              profileIdx,
    OUT CPSS_DXCH_PPU_KSTG_TCAM_ENTRY_STC   *tcamEntryPtr
);

/**
* @internal prvCpssDxChPpuActionTableEntrySet function
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
GT_STATUS prvCpssDxChPpuActionTableEntrySet
(
    IN  GT_U8                               devNum,
    IN  GT_U32                              kstgNum,
    IN  GT_U32                              index,
    IN  CPSS_DXCH_PPU_KSTG_ACTION_ENTRY_STC *ppuActionProfilePtr
);

/**
* @internal prvCpssDxChPpuActionTableEntryGet function
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
GT_STATUS prvCpssDxChPpuActionTableEntryGet
(
    IN  GT_U8                               devNum,
    IN  GT_U32                              kstgNum,
    IN  GT_U32                              index,
    OUT CPSS_DXCH_PPU_KSTG_ACTION_ENTRY_STC *ppuActionEntryPtr
);

/**
* @internal  prvCpssDxChPpuDauProfileEntrySet function
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
GT_STATUS prvCpssDxChPpuDauProfileEntrySet
(
    IN  GT_U8                               devNum,
    IN  GT_U32                              profileIndex,
    IN  CPSS_DXCH_PPU_DAU_PROFILE_ENTRY_STC *dauProfilePtr
);

/**
* @internal prvCpssDxChPpuDauProfileEntryGet function
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
GT_STATUS prvCpssDxChPpuDauProfileEntryGet
(
    IN  GT_U8                               devNum,
    IN  GT_U32                              profileIndex,
    OUT CPSS_DXCH_PPU_DAU_PROFILE_ENTRY_STC *dauProfilePtr
);

/**
* @internal  prvCpssDxChPpuDauProtectedWindowSet function
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
GT_STATUS prvCpssDxChPpuDauProtectedWindowSet
(
    IN  GT_U8                                       devNum,
    IN  GT_U32                                      index,
    IN  CPSS_DXCH_PPU_DAU_DATA_PROTECTED_WINDOW_STC *protWinPtr
);

/**
* @internal  prvCpssDxChPpuDauProtectedWindowGet function
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
GT_STATUS prvCpssDxChPpuDauProtectedWindowGet
(
    IN  GT_U8                                       devNum,
    IN  GT_U32                                      index,
    OUT CPSS_DXCH_PPU_DAU_DATA_PROTECTED_WINDOW_STC *protWinPtr
);

/**
* @internal prvCpssDxChPpuErrorProfileSet function
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
GT_STATUS prvCpssDxChPpuErrorProfileSet
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          profileNum,
    IN CPSS_DXCH_PPU_ERROR_PROFILE_STC  *errorProfilePtr
);

/**
* @internal prvCpssDxChPpuErrorProfileGet function
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
GT_STATUS prvCpssDxChPpuErrorProfileGet
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          profileNum,
    OUT CPSS_DXCH_PPU_ERROR_PROFILE_STC *errorProfilePtr
);

/**
* @internal prvCpssDxChPpuDebugCountersGet function
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
GT_STATUS prvCpssDxChPpuDebugCountersGet
(
    IN  GT_U8                            devNum,
    OUT CPSS_DXCH_PPU_DEBUG_COUNTERS_STC *dbgCountersPtr
);

/**
* @internal prvCpssDxChPpuDebugCountersClear function
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
GT_STATUS prvCpssDxChPpuDebugCountersClear
(
    IN  GT_U8                           devNum
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvCpssDxChPclh */


