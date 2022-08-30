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
* @file cpssDxChPpu.c
*
* @brief CPSS DXCH PPU API implementation
*
* @version   1
********************************************************************************
*/
#define CPSS_LOG_IN_MODULE_ENABLE
#include <cpss/dxCh/dxChxGen/ppu/private/prvCpssDxChPpu.h>
#include <cpss/dxCh/dxChxGen/ppu/private/prvCpssDxChPpuLog.h>
#include <cpss/dxCh/dxChxGen/ppu/cpssDxChPpu.h>
#include <cpss/dxCh/dxChxGen/ppu/cpssDxChPpuTypes.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/networkIf/private/prvCpssDxChNetIf.h>
#include <cpss/dxCh/dxChxGen/private/routerTunnelTermTcam/prvCpssDxChRouterTunnelTermTcam.h>
#include <cpss/dxCh/dxChxGen/diag/cpssDxChDiagDataIntegrity.h>
#include <cpssDriver/pp/interrupts/generic/prvCpssDrvInterrupts.h>
#include <cpss/generic/private/prvCpssCommonDefs.h>
#include <cpssDriver/pp/prvCpssDrvPpDefs.h>
#include <cpssCommon/cpssFormatConvert.h>
#include <cpss/dxCh/dxChxGen/tcam/private/prvCpssDxChTcam.h>
#include <cpss/dxCh/dxChxGen/private/lpm/hw/prvCpssDxChLpmHw.h>
#include <cpss/dxCh/dxChxGen/mirror/private/prvCpssDxChMirror.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

#define CHECK_IF_PPU_UNIT_SUPPORTED_MAC(_devNum)                                \
    if(PRV_CPSS_SIP_6_10_CHECK_MAC(_devNum) == GT_FALSE ||                      \
       (GT_TRUE == PRV_CPSS_DXCH_PP_MAC(_devNum)->hwInfo.ppuInfo.notSupported)) \
    {                                                                   \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE,         \
            "The device [%d] not supports the PPU unit",                \
            _devNum);                                                   \
    }

/**
* @internal internal_cpssDxChPpuEnableSet function
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
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS internal_cpssDxChPpuEnableSet
(
    IN  GT_U8       devNum,
    IN  GT_BOOL     enable
)
{
    GT_STATUS rc;

    /* Check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    CHECK_IF_PPU_UNIT_SUPPORTED_MAC(devNum);

    rc = prvCpssDxChPpuEnableSet(devNum, enable);

    return rc;
}

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
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPpuEnableSet
(
    IN  GT_U8       devNum,
    IN  GT_BOOL     enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPpuEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, enable));

    rc = internal_cpssDxChPpuEnableSet(devNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPpuEnableGet function
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
GT_STATUS internal_cpssDxChPpuEnableGet
(
    IN  GT_U8       devNum,
    OUT GT_BOOL     *enablePtr
)
{
    GT_STATUS   rc;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    CHECK_IF_PPU_UNIT_SUPPORTED_MAC(devNum);

    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    rc = prvCpssDxChPpuEnableGet(devNum, enablePtr);

    return rc;
}

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
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPpuEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, enablePtr));

    rc = internal_cpssDxChPpuEnableGet(devNum, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPpuMaxLoopbackSet function
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
GT_STATUS internal_cpssDxChPpuMaxLoopbackSet
(
    IN  GT_U8       devNum,
    IN  GT_U32      maxLoopback
)
{
    GT_STATUS   rc;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    CHECK_IF_PPU_UNIT_SUPPORTED_MAC(devNum);

    CPSS_DATA_CHECK_MAX_MAC(maxLoopback, (CPSS_DXCH_PPU_MAX_LOOPBACK_NUM_CNS + 1));

    rc = prvCpssDxChPpuMaxLoopbackSet(devNum, maxLoopback);

    return rc;
}

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
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPpuMaxLoopbackSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, maxLoopback));

    rc = internal_cpssDxChPpuMaxLoopbackSet(devNum, maxLoopback);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, maxLoopback));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPpuMaxLoopbackGet function
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
GT_STATUS internal_cpssDxChPpuMaxLoopbackGet
(
    IN  GT_U8       devNum,
    OUT GT_U32      *maxLoopbackPtr
)
{
    GT_STATUS   rc;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    CHECK_IF_PPU_UNIT_SUPPORTED_MAC(devNum);

    CPSS_NULL_PTR_CHECK_MAC(maxLoopbackPtr);

    rc = prvCpssDxChPpuMaxLoopbackGet(devNum, maxLoopbackPtr);

    return rc;
}

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
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPpuMaxLoopbackGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, maxLoopbackPtr));

    rc = internal_cpssDxChPpuMaxLoopbackGet(devNum, maxLoopbackPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, maxLoopbackPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPpuGlobalConfigSet function
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
GT_STATUS internal_cpssDxChPpuGlobalConfigSet
(
    IN  GT_U8                               devNum,
    IN  CPSS_DXCH_PPU_GLOBAL_CONFIG_STC     *ppuGlobalConfigPtr
)
{
    GT_STATUS   rc;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    CHECK_IF_PPU_UNIT_SUPPORTED_MAC(devNum);

    CPSS_NULL_PTR_CHECK_MAC(ppuGlobalConfigPtr);

    rc = prvCpssDxChPpuGlobalConfigSet(devNum, ppuGlobalConfigPtr);

    return rc;
}

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
    IN  GT_U8                               devNum,
    IN  CPSS_DXCH_PPU_GLOBAL_CONFIG_STC     *ppuGlobalConfigPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPpuGlobalConfigSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, ppuGlobalConfigPtr));

    rc = internal_cpssDxChPpuGlobalConfigSet(devNum, ppuGlobalConfigPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, ppuGlobalConfigPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPpuGlobalConfigGet function
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
GT_STATUS internal_cpssDxChPpuGlobalConfigGet
(
    IN  GT_U8                           devNum,
    OUT CPSS_DXCH_PPU_GLOBAL_CONFIG_STC *ppuGlobalConfigPtr
)
{
    GT_STATUS   rc;


    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    CHECK_IF_PPU_UNIT_SUPPORTED_MAC(devNum);

    CPSS_NULL_PTR_CHECK_MAC(ppuGlobalConfigPtr);

    rc = prvCpssDxChPpuGlobalConfigGet(devNum, ppuGlobalConfigPtr);

    return rc;
}

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
    IN  GT_U8                           devNum,
    OUT CPSS_DXCH_PPU_GLOBAL_CONFIG_STC *ppuGlobalConfigPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPpuGlobalConfigGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, ppuGlobalConfigPtr));

    rc = internal_cpssDxChPpuGlobalConfigGet(devNum, ppuGlobalConfigPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, ppuGlobalConfigPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPpuTtiActionProfileIndexSet function
* @endinternal
*
* @brief    set ppu profile index in tti action.
*
* @note   APPLICABLE DEVICES:  AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; BobCat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                 - device number
* @param[in] ttiRuleIndex    - PPU profile index pointer by TTI action.
* @param[in] profileNum             - PPU profile table index
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
GT_STATUS internal_cpssDxChPpuTtiActionProfileIndexSet
(
    IN  GT_U8                               devNum,
    IN  GT_U32                              ttiRuleIndex,
    IN  GT_U32                              profileNum
)
{
    GT_STATUS   rc;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    CHECK_IF_PPU_UNIT_SUPPORTED_MAC(devNum);

    PRV_CPSS_DXCH_TTI_INDEX_CHECK_MAC(devNum, ttiRuleIndex);
    CPSS_DATA_CHECK_MAX_MAC(profileNum, CPSS_DXCH_PPU_PROFILE_ENTRIES_MAX_CNS);

    rc = prvCpssDxChPpuTtiActionProfileIndexSet(devNum, ttiRuleIndex, profileNum);

    return rc;
}

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
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPpuTtiActionProfileIndexSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, ttiRuleIndex, profileNum));

    rc = internal_cpssDxChPpuTtiActionProfileIndexSet(devNum, ttiRuleIndex, profileNum);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, ttiRuleIndex, profileNum));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPpuTtiActionProfileIndexGet function
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
GT_STATUS internal_cpssDxChPpuTtiActionProfileIndexGet
(
    IN  GT_U8                               devNum,
    IN  GT_U32                              ttiRuleIndex,
    OUT GT_U32                              *profileNumPtr
)
{
    GT_STATUS   rc;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    CHECK_IF_PPU_UNIT_SUPPORTED_MAC(devNum);

    PRV_CPSS_DXCH_TTI_INDEX_CHECK_MAC(devNum, ttiRuleIndex);
    CPSS_NULL_PTR_CHECK_MAC(profileNumPtr);

    rc = prvCpssDxChPpuTtiActionProfileIndexGet(devNum, ttiRuleIndex, profileNumPtr);

    return rc;
}

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
    OUT GT_U32                              *profileNumPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPpuTtiActionProfileIndexGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, ttiRuleIndex, profileNumPtr));

    rc = internal_cpssDxChPpuTtiActionProfileIndexGet(devNum, ttiRuleIndex, profileNumPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, ttiRuleIndex, profileNumPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssDxChPpuSrcPortProfileIndexSet function
* @endinternal
*
* @brief    Set default profile profile index of a src port.
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
GT_STATUS internal_cpssDxChPpuSrcPortProfileIndexSet
(
    IN  GT_U8                               devNum,
    IN  GT_U32                              srcPortNum,
    IN  GT_U32                              profileNum
)
{
    GT_STATUS   rc;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    CHECK_IF_PPU_UNIT_SUPPORTED_MAC(devNum);

    PRV_CPSS_DXCH_PHY_PORT_NUM_CHECK_MAC(devNum, srcPortNum);
    CPSS_DATA_CHECK_MAX_MAC(profileNum, CPSS_DXCH_PPU_PROFILE_ENTRIES_MAX_CNS);

    rc = prvcpssDxChPpuSrcPortProfileIndexSet(devNum, srcPortNum, profileNum);

    return rc;
}

/**
* @internal cpssDxChPpuSrcPortProfileIndexSet function
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
GT_STATUS cpssDxChPpuSrcPortProfileIndexSet
(
    IN  GT_U8                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                srcPortNum,
    IN  GT_U32                              profileNum
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPpuSrcPortProfileIndexSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, srcPortNum, profileNum));

    rc = internal_cpssDxChPpuSrcPortProfileIndexSet(devNum, srcPortNum, profileNum);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, srcPortNum, profileNum));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPpuSrcPortProfileIndexGet  function
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
GT_STATUS internal_cpssDxChPpuSrcPortProfileIndexGet
(
    IN  GT_U8                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                srcPortNum,
    OUT GT_U32                              *profileNumPtr
)
{
    GT_STATUS   rc;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    CHECK_IF_PPU_UNIT_SUPPORTED_MAC(devNum);

    PRV_CPSS_DXCH_PHY_PORT_NUM_CHECK_MAC(devNum, srcPortNum);
    CPSS_NULL_PTR_CHECK_MAC(profileNumPtr);

    rc = prvCpssDxChPpuSrcPortProfileIndexGet (devNum, srcPortNum, profileNumPtr);

    return rc;
}

/**
* @internal cpssDxChPpuSrcPortProfileIndexGet  function
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
GT_STATUS cpssDxChPpuSrcPortProfileIndexGet
(
    IN  GT_U8                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                srcPortNum,
    OUT GT_U32                              *profileNumPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPpuSrcPortProfileIndexGet );

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, srcPortNum, profileNumPtr));

    rc = internal_cpssDxChPpuSrcPortProfileIndexGet (devNum, srcPortNum, profileNumPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, srcPortNum, profileNumPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPpuProfileSet function
* @endinternal
*
* @brief    This function sets PPU profile table entry for a given profile index.
*
* @note   APPLICABLE DEVICES:  AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; BobCat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                 - device number
* @param[in] profileNum             - PPU profile index
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
GT_STATUS internal_cpssDxChPpuProfileSet
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          profileNum,
    IN  CPSS_DXCH_PPU_PROFILE_ENTRY_STC *ppuProfilePtr
)
{
    GT_STATUS   rc;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    CHECK_IF_PPU_UNIT_SUPPORTED_MAC(devNum);

    CPSS_DATA_CHECK_MAX_MAC(profileNum, CPSS_DXCH_PPU_PROFILE_ENTRIES_MAX_CNS);

    CPSS_NULL_PTR_CHECK_MAC(ppuProfilePtr);

    rc = prvCpssDxChPpuProfileSet(devNum, profileNum, ppuProfilePtr);

    return rc;
}

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
* @param[in] profileNum             - PPU profile index
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
    IN  GT_U32                          profileNum,
    IN  CPSS_DXCH_PPU_PROFILE_ENTRY_STC *ppuProfilePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPpuProfileSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, profileNum, ppuProfilePtr));

    rc = internal_cpssDxChPpuProfileSet(devNum, profileNum, ppuProfilePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, profileNum, ppuProfilePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPpuProfileGet function
* @endinternal
*
* @brief    This function gets PPU profile table entry for a given profile index.
*
* @note   APPLICABLE DEVICES:  AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; BobCat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                 - device number
* @param[in] profileNum             - PPU profile index
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
GT_STATUS internal_cpssDxChPpuProfileGet
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          profileNum,
    OUT CPSS_DXCH_PPU_PROFILE_ENTRY_STC *ppuProfilePtr
)
{
    GT_STATUS   rc;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    CHECK_IF_PPU_UNIT_SUPPORTED_MAC(devNum);

    CPSS_DATA_CHECK_MAX_MAC(profileNum, CPSS_DXCH_PPU_PROFILE_ENTRIES_MAX_CNS);

    CPSS_NULL_PTR_CHECK_MAC(ppuProfilePtr);

    rc = prvCpssDxChPpuProfileGet(devNum, profileNum, ppuProfilePtr);

    return rc;
}

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
* @param[in] profileNum             - PPU profile index
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
    IN  GT_U32                          profileNum,
    OUT CPSS_DXCH_PPU_PROFILE_ENTRY_STC *ppuProfilePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPpuProfileGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, profileNum, ppuProfilePtr));

    rc = internal_cpssDxChPpuProfileGet(devNum, profileNum, ppuProfilePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, profileNum, ppuProfilePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPpuSpBusDefaultProfileSet function
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
GT_STATUS internal_cpssDxChPpuSpBusDefaultProfileSet
(
    IN  GT_U8                    devNum,
    IN  GT_U32                   profileNum,
    IN  CPSS_DXCH_PPU_SP_BUS_STC *spBusProfilePtr
)
{
    GT_STATUS   rc;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    CHECK_IF_PPU_UNIT_SUPPORTED_MAC(devNum);

    CPSS_DATA_CHECK_MAX_MAC(profileNum, CPSS_DXCH_PPU_SP_BUS_PROFILE_ENTRIES_MAX_CNS);

    CPSS_NULL_PTR_CHECK_MAC(spBusProfilePtr);

    rc = prvCpssDxChPpuSpBusDefaultProfileSet(devNum, profileNum, spBusProfilePtr);

    return rc;
}

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
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPpuSpBusDefaultProfileSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, profileNum, spBusProfilePtr));

    rc = internal_cpssDxChPpuSpBusDefaultProfileSet(devNum, profileNum, spBusProfilePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, profileNum, spBusProfilePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPpuSpBusDefaultProfileGet function
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
GT_STATUS internal_cpssDxChPpuSpBusDefaultProfileGet
(
    IN  GT_U8                    devNum,
    IN  GT_U32                   profileNum,
    OUT CPSS_DXCH_PPU_SP_BUS_STC *spBusProfilePtr
)
{
    GT_STATUS   rc;

    /* check params */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    CHECK_IF_PPU_UNIT_SUPPORTED_MAC(devNum);

    CPSS_DATA_CHECK_MAX_MAC(profileNum, CPSS_DXCH_PPU_SP_BUS_PROFILE_ENTRIES_MAX_CNS);

    CPSS_NULL_PTR_CHECK_MAC(spBusProfilePtr);

    rc = prvCpssDxChPpuSpBusDefaultProfileGet(devNum, profileNum, spBusProfilePtr);

    return rc;
}

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
    OUT CPSS_DXCH_PPU_SP_BUS_STC *spBusProfilePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPpuSpBusDefaultProfileGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, profileNum, spBusProfilePtr));

    rc = internal_cpssDxChPpuSpBusDefaultProfileGet(devNum, profileNum, spBusProfilePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, profileNum, spBusProfilePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPpuKstgKeyGenProfileSet function
* @endinternal
*
* @brief    This function sets K_STG key generation profile for a given profile index
*
* @note   APPLICABLE DEVICES:  AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; BobCat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                 - device number
* @param[in] kstgNum                - K_stg number
* @param[in] profileNum             - Key generation profile index
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
GT_STATUS internal_cpssDxChPpuKstgKeyGenProfileSet
(
    IN  GT_U8                                   devNum,
    IN  GT_U32                                  kstgNum,
    IN  GT_U32                                  profileNum,
    IN  CPSS_DXCH_PPU_KSTG_KEY_GEN_PROFILE_STC  *keygenProfilePtr
)
{
    GT_STATUS   rc;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    CHECK_IF_PPU_UNIT_SUPPORTED_MAC(devNum);

    CPSS_DATA_CHECK_MAX_MAC(kstgNum, CPSS_DXCH_PPU_KSTGS_MAX_CNS);

    CPSS_DATA_CHECK_MAX_MAC(profileNum, CPSS_DXCH_PPU_KSTG_KEY_GEN_PROFILES_MAX_CNS);

    CPSS_NULL_PTR_CHECK_MAC(keygenProfilePtr);

    rc = prvCpssDxChPpuKstgKeyGenProfileSet(devNum, kstgNum, profileNum, keygenProfilePtr);

    return rc;
}

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
* @param[in] profileNum             - Key generation profile index
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
    IN  GT_U32                                  profileNum,
    IN  CPSS_DXCH_PPU_KSTG_KEY_GEN_PROFILE_STC  *keygenProfilePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPpuKstgKeyGenProfileSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, kstgNum, profileNum, keygenProfilePtr));

    rc = internal_cpssDxChPpuKstgKeyGenProfileSet(devNum, kstgNum, profileNum, keygenProfilePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, kstgNum, profileNum, keygenProfilePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPpuKstgKeyGenProfileGet function
* @endinternal
*
* @brief    This function gets K_STG key generation profile for a given profile number.
*
* @note   APPLICABLE DEVICES:  AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; BobCat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                 - device number
* @param[in] kstgNum                - K_stg number
* @param[in] profileNum             - Key generation profile index
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
GT_STATUS internal_cpssDxChPpuKstgKeyGenProfileGet
(
    IN  GT_U8                                   devNum,
    IN  GT_U32                                  kstgNum,
    IN  GT_U32                                  profileNum,
    OUT CPSS_DXCH_PPU_KSTG_KEY_GEN_PROFILE_STC  *keygenProfilePtr
)
{
    GT_STATUS   rc;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    CHECK_IF_PPU_UNIT_SUPPORTED_MAC(devNum);

    CPSS_DATA_CHECK_MAX_MAC(kstgNum, CPSS_DXCH_PPU_KSTGS_MAX_CNS);

    CPSS_DATA_CHECK_MAX_MAC(profileNum, CPSS_DXCH_PPU_KSTG_KEY_GEN_PROFILES_MAX_CNS);

    CPSS_NULL_PTR_CHECK_MAC(keygenProfilePtr);

    rc = prvCpssDxChPpuKstgKeyGenProfileGet(devNum, kstgNum, profileNum, keygenProfilePtr);

    return rc;
}

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
* @param[in] profileNum             - Key generation profile index
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
    IN  GT_U32                                  profileNum,
    OUT CPSS_DXCH_PPU_KSTG_KEY_GEN_PROFILE_STC  *keygenProfilePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPpuKstgKeyGenProfileGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, kstgNum, profileNum, keygenProfilePtr));

    rc = internal_cpssDxChPpuKstgKeyGenProfileGet(devNum, kstgNum, profileNum, keygenProfilePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, kstgNum, profileNum, keygenProfilePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPpuKstgTcamEntrySet function
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
GT_STATUS internal_cpssDxChPpuKstgTcamEntrySet
(
    IN  GT_U8                               devNum,
    IN  GT_U32                              kstgNum,
    IN  GT_U32                              profileNum,
    IN  CPSS_DXCH_PPU_KSTG_TCAM_ENTRY_STC   *tcamEntryPtr
)
{
    GT_STATUS   rc;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    CHECK_IF_PPU_UNIT_SUPPORTED_MAC(devNum);

    CPSS_DATA_CHECK_MAX_MAC(kstgNum, CPSS_DXCH_PPU_KSTGS_MAX_CNS);

    CPSS_DATA_CHECK_MAX_MAC(profileNum, CPSS_DXCH_PPU_KSTG_TCAM_ENTRIES_MAX_CNS);

    CPSS_NULL_PTR_CHECK_MAC(tcamEntryPtr);

    rc = prvCpssDxChPpuKstgTcamEntrySet(devNum, kstgNum, profileNum, tcamEntryPtr);

    return rc;
}

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
    IN  GT_U32                              profileNum,
    IN  CPSS_DXCH_PPU_KSTG_TCAM_ENTRY_STC   *tcamEntryPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPpuKstgTcamEntrySet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, kstgNum, profileNum, tcamEntryPtr));

    rc = internal_cpssDxChPpuKstgTcamEntrySet(devNum, kstgNum, profileNum, tcamEntryPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, kstgNum, profileNum, tcamEntryPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPpuKstgTcamEntryGet function
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
GT_STATUS internal_cpssDxChPpuKstgTcamEntryGet
(
    IN  GT_U8                               devNum,
    IN  GT_U32                              kstgNum,
    IN  GT_U32                              profileNum,
    OUT CPSS_DXCH_PPU_KSTG_TCAM_ENTRY_STC   *tcamEntryPtr
)
{
    GT_STATUS   rc;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    CHECK_IF_PPU_UNIT_SUPPORTED_MAC(devNum);

    CPSS_DATA_CHECK_MAX_MAC(kstgNum, CPSS_DXCH_PPU_KSTGS_MAX_CNS);

    CPSS_DATA_CHECK_MAX_MAC(profileNum, CPSS_DXCH_PPU_KSTG_TCAM_ENTRIES_MAX_CNS);

    CPSS_NULL_PTR_CHECK_MAC(tcamEntryPtr);

    rc = prvCpssDxChPpuKstgTcamEntryGet(devNum, kstgNum, profileNum, tcamEntryPtr);

    return rc;
}

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
    IN  GT_U32                              profileNum,
    OUT CPSS_DXCH_PPU_KSTG_TCAM_ENTRY_STC   *tcamEntryPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPpuKstgTcamEntryGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, kstgNum, profileNum, tcamEntryPtr));

    rc = internal_cpssDxChPpuKstgTcamEntryGet(devNum, kstgNum, profileNum, tcamEntryPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, kstgNum, profileNum, tcamEntryPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPpuActionTableEntrySet function
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
GT_STATUS internal_cpssDxChPpuActionTableEntrySet
(
    IN  GT_U8                               devNum,
    IN  GT_U32                              kstgNum,
    IN  GT_U32                              index,
    IN  CPSS_DXCH_PPU_KSTG_ACTION_ENTRY_STC *ppuActionEntryPtr
)
{
    GT_STATUS   rc;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    CHECK_IF_PPU_UNIT_SUPPORTED_MAC(devNum);

    CPSS_DATA_CHECK_MAX_MAC(kstgNum, CPSS_DXCH_PPU_KSTGS_MAX_CNS);

    CPSS_NULL_PTR_CHECK_MAC(ppuActionEntryPtr);

    rc = prvCpssDxChPpuActionTableEntrySet(devNum, kstgNum, index, ppuActionEntryPtr);

    return rc;
}

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
    IN  CPSS_DXCH_PPU_KSTG_ACTION_ENTRY_STC *ppuActionEntryPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPpuActionTableEntrySet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, kstgNum, index, ppuActionEntryPtr));

    rc = internal_cpssDxChPpuActionTableEntrySet(devNum, kstgNum, index, ppuActionEntryPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, kstgNum, index, ppuActionEntryPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPpuActionTableEntryGet function
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
GT_STATUS internal_cpssDxChPpuActionTableEntryGet
(
    IN  GT_U8                               devNum,
    IN  GT_U32                              kstgNum,
    IN  GT_U32                              index,
    OUT CPSS_DXCH_PPU_KSTG_ACTION_ENTRY_STC *ppuActionEntryPtr
)
{
    GT_STATUS   rc;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    CHECK_IF_PPU_UNIT_SUPPORTED_MAC(devNum);

    CPSS_DATA_CHECK_MAX_MAC(kstgNum, CPSS_DXCH_PPU_KSTGS_MAX_CNS);

    CPSS_NULL_PTR_CHECK_MAC(ppuActionEntryPtr);

    rc = prvCpssDxChPpuActionTableEntryGet(devNum, kstgNum, index, ppuActionEntryPtr);

    return rc;
}

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
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPpuActionTableEntryGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, kstgNum, index, ppuActionEntryPtr));

    rc = internal_cpssDxChPpuActionTableEntryGet(devNum, kstgNum, index, ppuActionEntryPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, kstgNum, index, ppuActionEntryPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal  internal_cpssDxChPpuDauProfileEntrySet function
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
GT_STATUS internal_cpssDxChPpuDauProfileEntrySet
(
    IN  GT_U8                               devNum,
    IN  GT_U32                              profileIndex,
    IN  CPSS_DXCH_PPU_DAU_PROFILE_ENTRY_STC *dauProfilePtr
)
{
    GT_STATUS   rc;
    GT_U32      i;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    CHECK_IF_PPU_UNIT_SUPPORTED_MAC(devNum);

    CPSS_DATA_CHECK_MAX_MAC(profileIndex, CPSS_DXCH_PPU_DAU_PROFILE_ENTRIES_MAX_CNS);

    CPSS_NULL_PTR_CHECK_MAC(dauProfilePtr);

    for (i=0; i<CPSS_DXCH_PPU_DAU_DESC_BYTE_SET_MAX_CNS; i++)
    {
        if (dauProfilePtr->setDescBits[i].spByteWriteEnable == GT_TRUE)
        {
            CPSS_DATA_CHECK_MAX_MAC(dauProfilePtr->setDescBits[i].spByteTargetOffset, CPSS_DXCH_PPU_DAU_DESCR_SIZE_IN_BITS);
            CPSS_DATA_CHECK_MAX_MAC(dauProfilePtr->setDescBits[i].spByteSrcOffset, CPSS_DXCH_PPU_SP_BUS_BYTES_MAX_CNS*8);
            CPSS_DATA_CHECK_MAX_MAC(dauProfilePtr->setDescBits[i].spByteNumBits, 8);
        }
    }

    rc = prvCpssDxChPpuDauProfileEntrySet(devNum, profileIndex, dauProfilePtr);

    return rc;
}

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
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPpuDauProfileEntrySet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, profileIndex, dauProfilePtr));

    rc = internal_cpssDxChPpuDauProfileEntrySet(devNum, profileIndex, dauProfilePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, profileIndex, dauProfilePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPpuDauProfileEntryGet function
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
GT_STATUS internal_cpssDxChPpuDauProfileEntryGet
(
    IN  GT_U8                               devNum,
    IN  GT_U32                              profileIndex,
    OUT CPSS_DXCH_PPU_DAU_PROFILE_ENTRY_STC *dauProfilePtr
)
{
    GT_STATUS   rc;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    CHECK_IF_PPU_UNIT_SUPPORTED_MAC(devNum);

    CPSS_DATA_CHECK_MAX_MAC(profileIndex, CPSS_DXCH_PPU_DAU_PROFILE_ENTRIES_MAX_CNS);

    CPSS_NULL_PTR_CHECK_MAC(dauProfilePtr);

    rc = prvCpssDxChPpuDauProfileEntryGet(devNum, profileIndex, dauProfilePtr);

    return rc;
}

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
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPpuDauProfileEntryGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, profileIndex, dauProfilePtr));

    rc = internal_cpssDxChPpuDauProfileEntryGet(devNum, profileIndex, dauProfilePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, profileIndex, dauProfilePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal  internal_cpssDxChPpuDauProtectedWindowSet function
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
GT_STATUS internal_cpssDxChPpuDauProtectedWindowSet
(
    IN  GT_U8                                       devNum,
    IN  GT_U32                                      index,
    IN  CPSS_DXCH_PPU_DAU_DATA_PROTECTED_WINDOW_STC *protWinPtr
)
{
    GT_STATUS   rc;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    CHECK_IF_PPU_UNIT_SUPPORTED_MAC(devNum);

    CPSS_DATA_CHECK_MAX_MAC(index, CPSS_DXCH_PPU_DAU_DATA_PROTECTED_WINDOW_MAX_CNS);

    CPSS_NULL_PTR_CHECK_MAC(protWinPtr);

    rc = prvCpssDxChPpuDauProtectedWindowSet(devNum, index, protWinPtr);

    return rc;
}

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
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPpuDauProtectedWindowSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, index, protWinPtr));

    rc = internal_cpssDxChPpuDauProtectedWindowSet(devNum, index, protWinPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, index, protWinPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal  internal_cpssDxChPpuDauProtectedWindowGet function
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
GT_STATUS internal_cpssDxChPpuDauProtectedWindowGet
(
    IN  GT_U8                                       devNum,
    IN  GT_U32                                      index,
    OUT CPSS_DXCH_PPU_DAU_DATA_PROTECTED_WINDOW_STC *protWinPtr
)
{
    GT_STATUS   rc;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    CHECK_IF_PPU_UNIT_SUPPORTED_MAC(devNum);

    CPSS_DATA_CHECK_MAX_MAC(index, CPSS_DXCH_PPU_DAU_DATA_PROTECTED_WINDOW_MAX_CNS);

    CPSS_NULL_PTR_CHECK_MAC(protWinPtr);

    rc = prvCpssDxChPpuDauProtectedWindowGet(devNum, index, protWinPtr);

    return rc;
}

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
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPpuDauProtectedWindowGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, index, protWinPtr));

    rc = internal_cpssDxChPpuDauProtectedWindowGet(devNum, index, protWinPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, index, protWinPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPpuErrorProfileSet function
* @endinternal
*
* @brief    This function gets error profile for a given profile and descriptor field.
*
* @note   APPLICABLE DEVICES:  AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; BobCat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                - device number
* @param[in] profileNum            - profile number
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
GT_STATUS internal_cpssDxChPpuErrorProfileSet
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          profileNum,
    IN  CPSS_DXCH_PPU_ERROR_PROFILE_STC *errorProfilePtr
)
{
    GT_STATUS   rc;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    CHECK_IF_PPU_UNIT_SUPPORTED_MAC(devNum);

    CPSS_DATA_CHECK_MAX_MAC(profileNum, CPSS_DXCH_PPU_ERROR_PROFILES_MAX_CNS);

    CPSS_NULL_PTR_CHECK_MAC(errorProfilePtr);

    rc = prvCpssDxChPpuErrorProfileSet(devNum, profileNum, errorProfilePtr);

    return rc;
}

/**
* @internal cpssDxChPpuErrorProfileSet function
* @endinternal
*
* @brief    This function gets error profile for a given profile and descriptor field.
*
* @note   APPLICABLE DEVICES:  AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; BobCat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                - device number
* @param[in] profileNum            - profile number
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
    IN  CPSS_DXCH_PPU_ERROR_PROFILE_STC *errorProfilePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPpuErrorProfileSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, profileNum, errorProfilePtr));

    rc = internal_cpssDxChPpuErrorProfileSet(devNum, profileNum, errorProfilePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, profileNum, errorProfilePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPpuErrorProfileGet function
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
GT_STATUS internal_cpssDxChPpuErrorProfileGet
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          profileNum,
    OUT CPSS_DXCH_PPU_ERROR_PROFILE_STC *errorProfilePtr
)
{
    GT_STATUS   rc;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    CHECK_IF_PPU_UNIT_SUPPORTED_MAC(devNum);

    CPSS_DATA_CHECK_MAX_MAC(profileNum, CPSS_DXCH_PPU_ERROR_PROFILES_MAX_CNS);

    CPSS_NULL_PTR_CHECK_MAC(errorProfilePtr);

    rc = prvCpssDxChPpuErrorProfileGet(devNum, profileNum, errorProfilePtr);

    return rc;
}

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
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPpuErrorProfileGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, profileNum, errorProfilePtr));

    rc = internal_cpssDxChPpuErrorProfileGet(devNum, profileNum, errorProfilePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, profileNum, errorProfilePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPpuDebugCountersGet function
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
GT_STATUS internal_cpssDxChPpuDebugCountersGet
(
    IN  GT_U8                            devNum,
    OUT CPSS_DXCH_PPU_DEBUG_COUNTERS_STC *dbgCountersPtr
)
{
    GT_STATUS   rc;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    CHECK_IF_PPU_UNIT_SUPPORTED_MAC(devNum);

    CPSS_NULL_PTR_CHECK_MAC(dbgCountersPtr);

    rc = prvCpssDxChPpuDebugCountersGet(devNum, dbgCountersPtr);

    return rc;
}

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
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPpuDebugCountersGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, dbgCountersPtr));

    rc = internal_cpssDxChPpuDebugCountersGet(devNum, dbgCountersPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, dbgCountersPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPpuDebugCountersClear function
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
GT_STATUS internal_cpssDxChPpuDebugCountersClear
(
    IN  GT_U8   devNum
)
{
    GT_STATUS   rc;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    CHECK_IF_PPU_UNIT_SUPPORTED_MAC(devNum);

    rc = prvCpssDxChPpuDebugCountersClear(devNum);

    return rc;
}

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
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPpuDebugCountersClear);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum));

    rc = internal_cpssDxChPpuDebugCountersClear(devNum);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

