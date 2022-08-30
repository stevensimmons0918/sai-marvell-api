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
* @file tgfBridgeL2EcmpGen.c
*
* @brief Generic API implementation for Bridge L2 ECMP
*
* @version   8
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>

#include <utf/private/prvUtfHelpers.h>
#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>

#include <trafficEngine/tgfTrafficEngine.h>
#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>

#include <common/tgfCommon.h>
#include <common/tgfBridgeL2EcmpGen.h>

#ifdef CHX_FAMILY

#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgL2Ecmp.h>

/**
* @internal prvTgfConvertGenericToDxChL2EcmpLttEntry function
* @endinternal
*
* @brief   Convert generic into device specific L2 ECMP LTT entry
*
* @param[in] l2EcmpLttEntryPtr        - (pointer to) L2 ECMP LTT entry
*
* @param[out] dxChL2EcmpLttEntryPtr    - (pointer to) DxCh L2 ECMP LTT entry parameters
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
static GT_STATUS prvTgfConvertGenericToDxChL2EcmpLttEntry
(
    IN  PRV_TGF_BRG_L2_ECMP_LTT_ENTRY_STC    *l2EcmpLttEntryPtr,
    OUT CPSS_DXCH_BRG_L2_ECMP_LTT_ENTRY_STC  *dxChL2EcmpLttEntryPtr
)
{
    cpssOsMemSet(dxChL2EcmpLttEntryPtr, 0, sizeof(CPSS_DXCH_BRG_L2_ECMP_LTT_ENTRY_STC));

    /* convert entry info into device specific format */
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChL2EcmpLttEntryPtr, l2EcmpLttEntryPtr, ecmpStartIndex);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChL2EcmpLttEntryPtr, l2EcmpLttEntryPtr, ecmpNumOfPaths);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChL2EcmpLttEntryPtr, l2EcmpLttEntryPtr, ecmpEnable);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChL2EcmpLttEntryPtr, l2EcmpLttEntryPtr, hashBitSelectionProfile);

    return GT_OK;
}

/**
* @internal prvTgfConvertDxChToGenericL2EcmpLttEntry function
* @endinternal
*
* @brief   Convert device specific L2 ECMP LTT entry into generic
*
* @param[in] dxChL2EcmpLttEntryPtr    - (pointer to) DxCh L2 ECMP LTT entry parameters
*
* @param[out] l2EcmpLttEntryPtr        - (pointer to) L2 ECMP LTT entry
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
static GT_STATUS prvTgfConvertDxChToGenericL2EcmpLttEntry
(
    IN CPSS_DXCH_BRG_L2_ECMP_LTT_ENTRY_STC  *dxChL2EcmpLttEntryPtr,
    OUT PRV_TGF_BRG_L2_ECMP_LTT_ENTRY_STC   *l2EcmpLttEntryPtr
)
{
    cpssOsMemSet(l2EcmpLttEntryPtr, 0, sizeof(PRV_TGF_BRG_L2_ECMP_LTT_ENTRY_STC));

    /* convert entry info into generic format */
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChL2EcmpLttEntryPtr, l2EcmpLttEntryPtr, ecmpStartIndex);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChL2EcmpLttEntryPtr, l2EcmpLttEntryPtr, ecmpNumOfPaths);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChL2EcmpLttEntryPtr, l2EcmpLttEntryPtr, ecmpEnable);

    return GT_OK;
}

/**
* @internal prvTgfConvertGenericToDxChL2EcmpEntry function
* @endinternal
*
* @brief   Convert generic into device specific L2 ECMP entry
*
* @param[in] l2EcmpEntryPtr           - (pointer to) L2 ECMP entry
*
* @param[out] dxChL2EcmpEntryPtr       - (pointer to) DxCh L2 ECMP entry parameters
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
static GT_STATUS prvTgfConvertGenericToDxChL2EcmpEntry
(
    IN  PRV_TGF_BRG_L2_ECMP_ENTRY_STC    *l2EcmpEntryPtr,
    OUT CPSS_DXCH_BRG_L2_ECMP_ENTRY_STC  *dxChL2EcmpEntryPtr
)
{
    cpssOsMemSet(dxChL2EcmpEntryPtr, 0, sizeof(CPSS_DXCH_BRG_L2_ECMP_ENTRY_STC));

    /* convert entry info into device specific format */
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChL2EcmpEntryPtr, l2EcmpEntryPtr, targetEport);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChL2EcmpEntryPtr, l2EcmpEntryPtr, targetHwDevice);

    /* covert the SW device to HW devNum */
    return prvUtfHwDeviceNumberGet((GT_U8)dxChL2EcmpEntryPtr->targetHwDevice,
                                   &dxChL2EcmpEntryPtr->targetHwDevice);
}

/**
* @internal prvTgfConvertDxChToGenericL2EcmpEntry function
* @endinternal
*
* @brief   Convert device specific L2 ECMP entry into generic
*
* @param[in] dxChL2EcmpEntryPtr       - (pointer to) DxCh L2 ECMP entry parameters
*
* @param[out] l2EcmpEntryPtr           - (pointer to) L2 ECMP entry
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
static GT_STATUS prvTgfConvertDxChToGenericL2EcmpEntry
(
    IN CPSS_DXCH_BRG_L2_ECMP_ENTRY_STC  *dxChL2EcmpEntryPtr,
    OUT PRV_TGF_BRG_L2_ECMP_ENTRY_STC   *l2EcmpEntryPtr
)
{
    cpssOsMemSet(l2EcmpEntryPtr, 0, sizeof(PRV_TGF_BRG_L2_ECMP_ENTRY_STC));

    /* convert entry info into generic format */
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChL2EcmpEntryPtr, l2EcmpEntryPtr, targetEport);
    PRV_TGF_STC_D2S_FIELD_COPY_MAC(dxChL2EcmpEntryPtr, l2EcmpEntryPtr, targetHwDevice);

    /* covert the HW devNum to SW device */
    return prvUtfSwFromHwDeviceNumberGet(l2EcmpEntryPtr->targetHwDevice,
                                         &l2EcmpEntryPtr->targetHwDevice);
}

/**
* @internal prvTgfConvertGenericToDxChHashBitSelectionProfileEntry function
* @endinternal
*
* @brief   Convert generic into device specific hash bit selection profile entry
*
* @param[in] hashBitSelectionEntryPtr        - (pointer to) Hash bit selection entry
*
* @param[out] dxChHashBitSelectionEntryPtr    - (pointer to) DxCh Hash bit selection entry parameters
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
static GT_STATUS prvTgfConvertGenericToDxChHashBitSelectionProfileEntry
(
    IN  PRV_TGF_HASH_BIT_SELECTION_PROFILE_STC    *hashBitSelectionEntryPtr,
    OUT CPSS_DXCH_HASH_BIT_SELECTION_PROFILE_STC  *dxChHashBitSelectionEntryPtr
)
{
    cpssOsMemSet(dxChHashBitSelectionEntryPtr, 0, sizeof(CPSS_DXCH_HASH_BIT_SELECTION_PROFILE_STC));

    /* convert entry info into device specific format */
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChHashBitSelectionEntryPtr, hashBitSelectionEntryPtr, firstBit);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChHashBitSelectionEntryPtr, hashBitSelectionEntryPtr, lastBit);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChHashBitSelectionEntryPtr, hashBitSelectionEntryPtr, salt);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChHashBitSelectionEntryPtr, hashBitSelectionEntryPtr, srcPortHashEnable);

    return GT_OK;
}

/**
* @internal prvTgfConvertGenericToDxChHashBitSelectionProfileEntry function
* @endinternal
*
* @brief   Convert generic into device specific hash bit selection profile entry
*
* @param[in] dxChHashBitSelectionEntryPtr    - (pointer to) DxCh Hash bit selection entry parameters
*
* @param[out] hashBitSelectionEntryPtr        - (pointer to) Hash bit selection entry
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
static GT_STATUS prvTgfConvertDxChToGenericHashBitSelectionProfileEntry
(
    IN  CPSS_DXCH_HASH_BIT_SELECTION_PROFILE_STC  *dxChHashBitSelectionEntryPtr,
    OUT PRV_TGF_HASH_BIT_SELECTION_PROFILE_STC   *hashBitSelectionEntryPtr
)
{
    cpssOsMemSet(hashBitSelectionEntryPtr, 0, sizeof(PRV_TGF_HASH_BIT_SELECTION_PROFILE_STC));

    /* convert entry info into device specific format */
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(hashBitSelectionEntryPtr, dxChHashBitSelectionEntryPtr, firstBit);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(hashBitSelectionEntryPtr, dxChHashBitSelectionEntryPtr, lastBit);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(hashBitSelectionEntryPtr, dxChHashBitSelectionEntryPtr, salt);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(hashBitSelectionEntryPtr, dxChHashBitSelectionEntryPtr, srcPortHashEnable);

    return GT_OK;
}

#endif /*CHX_FAMILY*/

/**
* @internal prvTgfBrgL2EcmpEnableSet function
* @endinternal
*
* @brief   Globally enable/disable L2 ECMP (AKA ePort ECMP)
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; AC5; Lion; xCat2; Lion2.
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
GT_STATUS prvTgfBrgL2EcmpEnableSet
(
    IN  GT_U8       devNum,
    IN  GT_BOOL     enable
)
{
    PRV_UTF_LOG2_MAC("prvTgfBrgL2EcmpEnableSet (%d,%d) \n",
        devNum,enable);

#ifdef CHX_FAMILY
    {
        GT_STATUS   rc = GT_OK;

        /* call device specific API */
        rc = cpssDxChBrgL2EcmpEnableSet(devNum, enable);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChBrgL2EcmpEnableSet FAILED, rc = [%d]", rc);

            return rc;
        }

        return rc;
    }
#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfBrgL2EcmpEnableGet function
* @endinternal
*
* @brief   Get the global enable/disable L2 ECMP (AKA ePort ECMP)
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; AC5; Lion; xCat2; Lion2.
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
GT_STATUS prvTgfBrgL2EcmpEnableGet
(
    IN  GT_U8       devNum,
    OUT GT_BOOL     *enablePtr
)
{
    PRV_UTF_LOG1_MAC("prvTgfBrgL2EcmpEnableGet (%d) \n",
        devNum);

#ifdef CHX_FAMILY
    {
        GT_STATUS   rc = GT_OK;

        /* call device specific API */
        rc = cpssDxChBrgL2EcmpEnableGet(devNum, enablePtr);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: cpssDxChBrgL2EcmpEnableGet FAILED, rc = [%d]", rc);

            return rc;
        }

        return rc;
    }
#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(enablePtr);
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfBrgL2EcmpIndexBaseEportSet function
* @endinternal
*
* @brief   Defines the first ePort number in the L2 ECMP ePort number range.
*         The index to the L2 ECMP LTT is <Target ePort>-<Base ePort>.
* @param[in] devNum                   - device number
* @param[in] ecmpIndexBaseEport       - the ePort number
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgL2EcmpIndexBaseEportSet
(
    IN GT_U8            devNum,
    IN GT_PORT_NUM      ecmpIndexBaseEport
)
{
#ifdef CHX_FAMILY
    GT_STATUS   rc = GT_OK;

    /* call device specific API */
    rc = cpssDxChBrgL2EcmpIndexBaseEportSet(devNum, ecmpIndexBaseEport);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChBrgL2EcmpIndexBaseEportSet FAILED, rc = [%d]", rc);

        return rc;
    }

    return rc;
#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfBrgL2EcmpIndexBaseEportGet function
* @endinternal
*
* @brief   Return the first ePort number in the L2 ECMP ePort number range.
*         The index to the L2 ECMP LTT is <Target ePort>-<Base ePort>.
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
GT_STATUS prvTgfBrgL2EcmpIndexBaseEportGet
(
    IN  GT_U8           devNum,
    OUT GT_PORT_NUM     *ecmpIndexBaseEportPtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS   rc = GT_OK;

    /* call device specific API */
    rc = cpssDxChBrgL2EcmpIndexBaseEportGet(devNum, ecmpIndexBaseEportPtr);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChBrgL2EcmpIndexBaseEportGet FAILED, rc = [%d]", rc);

        return rc;
    }

    return rc;
#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfBrgL2EcmpLttTableSet function
* @endinternal
*
* @brief   Set L2 ECMP LTT entry info.
*
* @param[in] devNum                   - device number
* @param[in] index                    - the LTT index
* @param[in] ecmpLttInfoPtr           - (pointer to) L2 ECMP LTT Information
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The index to the LTT is the result of InDesc<TRGePort>-<Base ePort>.
*
*/
GT_STATUS prvTgfBrgL2EcmpLttTableSet
(
    IN GT_U8                                devNum,
    IN GT_U32                               index,
    IN PRV_TGF_BRG_L2_ECMP_LTT_ENTRY_STC   *ecmpLttInfoPtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS   rc = GT_OK;
    CPSS_DXCH_BRG_L2_ECMP_LTT_ENTRY_STC     dxChEcmpLttEntry;

    /* convert L2 ECMP LTT entry into device specific format */
    rc = prvTgfConvertGenericToDxChL2EcmpLttEntry(ecmpLttInfoPtr, &dxChEcmpLttEntry);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChL2EcmpLttEntry FAILED, rc = [%d]", rc);

        return rc;
    }

    rc = cpssDxChBrgL2EcmpLttTableSet(devNum, index, &dxChEcmpLttEntry);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfBrgL2EcmpLttTableSet FAILED, rc = [%d]", rc);

        return rc;
    }

    return rc;
#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfBrgL2EcmpLttTableGet function
* @endinternal
*
* @brief   Get L2 ECMP LTT entry info.
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
* @note The index to the LTT is the result of InDesc<TRGePort>-<Base ePort>.
*
*/
GT_STATUS prvTgfBrgL2EcmpLttTableGet
(
    IN  GT_U8                                devNum,
    IN  GT_U32                               index,
    OUT PRV_TGF_BRG_L2_ECMP_LTT_ENTRY_STC   *ecmpLttInfoPtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS   rc = GT_OK;
    CPSS_DXCH_BRG_L2_ECMP_LTT_ENTRY_STC     dxChEcmpLttEntry;

    rc = cpssDxChBrgL2EcmpLttTableGet(devNum, index, &dxChEcmpLttEntry);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChBrgL2EcmpLttTableGet FAILED, rc = [%d]", rc);

        return rc;
    }

    /* convert L2 ECMP LTT entry into generic format */
    rc = prvTgfConvertDxChToGenericL2EcmpLttEntry(&dxChEcmpLttEntry, ecmpLttInfoPtr);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertDxChToGenericL2EcmpLttEntry FAILED, rc = [%d]", rc);

        return rc;
    }

    return rc;
#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfBrgL2EcmpTableSet function
* @endinternal
*
* @brief   Set L2 ECMP entry info.
*
* @param[in] devNum                   - device number
* @param[in] index                    - the ECMP entry index
* @param[in] ecmpEntryPtr             - (pointer to) L2 ECMP entry.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Index according to index from L2 ECMP LTT
*
*/
GT_STATUS prvTgfBrgL2EcmpTableSet
(
    IN GT_U8                                devNum,
    IN GT_U32                               index,
    IN PRV_TGF_BRG_L2_ECMP_ENTRY_STC       *ecmpEntryPtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS   rc = GT_OK;
    CPSS_DXCH_BRG_L2_ECMP_ENTRY_STC         dxChEcmpEntry;

    /* convert L2 ECMP entry into device specific format */
    rc = prvTgfConvertGenericToDxChL2EcmpEntry(ecmpEntryPtr, &dxChEcmpEntry);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChL2EcmpEntry FAILED, rc = [%d]", rc);

        return rc;
    }

    rc = cpssDxChBrgL2EcmpTableSet(devNum, index, &dxChEcmpEntry);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChBrgL2EcmpTableSet FAILED, rc = [%d]", rc);

        return rc;
    }

    return rc;
#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfBrgL2EcmpTableGet function
* @endinternal
*
* @brief   Get L2 ECMP entry info.
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
* @note Index according to index from L2 ECMP LTT
*
*/
GT_STATUS prvTgfBrgL2EcmpTableGet
(
    IN  GT_U8                                devNum,
    IN  GT_U32                               index,
    OUT PRV_TGF_BRG_L2_ECMP_ENTRY_STC       *ecmpEntryPtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS   rc = GT_OK;
    CPSS_DXCH_BRG_L2_ECMP_ENTRY_STC         dxChEcmpEntry;

    rc = cpssDxChBrgL2EcmpTableGet(devNum, index, &dxChEcmpEntry);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChBrgL2EcmpTableGet FAILED, rc = [%d]", rc);

        return rc;
    }

    /* convert L2 ECMP entry into generic format */
    rc = prvTgfConvertDxChToGenericL2EcmpEntry(&dxChEcmpEntry, ecmpEntryPtr);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertDxChToGenericL2EcmpEntry FAILED, rc = [%d]", rc);

        return rc;
    }

    return rc;
#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfBrgL2EcmpMemberSelectionModeSet function
* @endinternal
*
* @brief   Set mode used to calculate the Index of the secondary ePort (L2 ECMP member)
*
* @param[in] devNum                   - device number
* @param[in] selectionMode            - member selection mode
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgL2EcmpMemberSelectionModeSet
(
    IN GT_U8                                    devNum,
    IN PRV_TGF_TRUNK_MEMBER_SELECTION_MODE_ENT  selectionMode
)
{
#ifdef CHX_FAMILY
    GT_STATUS   rc = GT_OK;
    CPSS_DXCH_MEMBER_SELECTION_MODE_ENT         dxChSelectionMode;
    switch(selectionMode)
    {
        PRV_TGF_SWITCH_CASE_MAC(dxChSelectionMode,
                                PRV_TGF_TRUNK_MEMBER_SELECTION_MODE_12_BITS_E,
                                CPSS_DXCH_MEMBER_SELECTION_MODE_12_BITS_E);
        PRV_TGF_SWITCH_CASE_MAC(dxChSelectionMode,
                                PRV_TGF_TRUNK_MEMBER_SELECTION_MODE_6_LSB_E,
                                CPSS_DXCH_MEMBER_SELECTION_MODE_6_LSB_E);
        PRV_TGF_SWITCH_CASE_MAC(dxChSelectionMode,
                                PRV_TGF_TRUNK_MEMBER_SELECTION_MODE_6_MSB_E,
                                CPSS_DXCH_MEMBER_SELECTION_MODE_6_MSB_E);
        default:
            return GT_BAD_PARAM;
    }

    /* call device specific API */
    rc = cpssDxChBrgL2EcmpMemberSelectionModeSet(devNum, dxChSelectionMode);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChBrgL2EcmpMemberSelectionModeSet FAILED, rc = [%d]", rc);

        return rc;
    }

    return rc;
#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfBrgL2EcmpMemberSelectionModeGet function
* @endinternal
*
* @brief   Get mode used to calculate the Index of the secondary ePort (L2 ECMP member)
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
GT_STATUS prvTgfBrgL2EcmpMemberSelectionModeGet
(
    IN  GT_U8                                   devNum,
    OUT PRV_TGF_TRUNK_MEMBER_SELECTION_MODE_ENT *selectionModePtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS   rc = GT_OK;
    CPSS_DXCH_MEMBER_SELECTION_MODE_ENT         dxChSelectionMode;

    /* call device specific API */
    rc = cpssDxChBrgL2EcmpMemberSelectionModeGet(devNum, &dxChSelectionMode);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChBrgL2EcmpMemberSelectionModeGet FAILED, rc = [%d]", rc);

        return rc;
    }

    switch(dxChSelectionMode)
    {
        PRV_TGF_SWITCH_CASE_MAC(*selectionModePtr,
                                CPSS_DXCH_MEMBER_SELECTION_MODE_12_BITS_E,
                                PRV_TGF_TRUNK_MEMBER_SELECTION_MODE_12_BITS_E);
        PRV_TGF_SWITCH_CASE_MAC(*selectionModePtr,
                                CPSS_DXCH_MEMBER_SELECTION_MODE_6_LSB_E,
                                PRV_TGF_TRUNK_MEMBER_SELECTION_MODE_6_LSB_E);
        PRV_TGF_SWITCH_CASE_MAC(*selectionModePtr,
                                CPSS_DXCH_MEMBER_SELECTION_MODE_6_MSB_E,
                                PRV_TGF_TRUNK_MEMBER_SELECTION_MODE_6_MSB_E);
        default:
            return GT_BAD_PARAM;
    }

    return rc;
#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfBrgL2EcmpAndLttEntriesClear function
* @endinternal
*
* @brief   Clear ECMP MLL & LTT entries.
*
* @param[in] lttIndexArr              - Array of LTT indexes to clear.
* @param[in] numOfLttIndexes          - Number of valid entries in <lttIndexArr>.
* @param[in] ecmpIndexArr             - Array of ECMP indexes to clear.
* @param[in] numOfEcmpIndexes         - Number of valid entries in <ecmpIndexArr>.
*                                       None
*/
GT_VOID prvTgfBrgL2EcmpAndLttEntriesClear
(
    IN GT_U32 *lttIndexArr,
    IN GT_U32  numOfLttIndexes,
    IN GT_U32 *ecmpIndexArr,
    IN GT_U32  numOfEcmpIndexes
)
{
    GT_STATUS rc;
    PRV_TGF_BRG_L2_ECMP_LTT_ENTRY_STC           ecmpLttEntry;
    PRV_TGF_BRG_L2_ECMP_ENTRY_STC               ecmpEntry;
    GT_U32  i;

    /* clear L2 ECMP LTT entries */
    cpssOsMemSet(&ecmpLttEntry, 0, sizeof(PRV_TGF_BRG_L2_ECMP_LTT_ENTRY_STC));
    ecmpLttEntry.ecmpNumOfPaths = 1;

    for ( i = 0 ; i < numOfLttIndexes ; i++)
    {
        rc = prvTgfBrgL2EcmpLttTableSet(prvTgfDevNum, lttIndexArr[i], &ecmpLttEntry);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgL2EcmpLttTableSet: index %d",
                                     lttIndexArr[i]);
    }

    /* clear L2 ECMP entries */
    cpssOsMemSet(&ecmpEntry, 0, sizeof(PRV_TGF_BRG_L2_ECMP_ENTRY_STC));

    for ( i = 0 ; i < numOfEcmpIndexes ; i++)
    {
        rc = prvTgfBrgL2EcmpTableSet(prvTgfDevNum, ecmpIndexArr[i], &ecmpEntry);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgL2EcmpTableSet: index %d",
                                     ecmpIndexArr[i]);
    }
}
/**
* @internal prvTgfBrgL2EcmpEntryClear function
* @endinternal
*
* @brief   Clear ECMP MLL & LTT entries.
*
* @param[in] lttIndex                 - LTT indexes to clear.
* @param[in] resetMemebers            - indication to clear all the members that are pointed
*                                      from the LTT in the ECMP table.
*                                      GT_TRUE  - clear members too.
*                                      GT_FALSE - not clear members (only LTT entry).
*                                       None
*/
GT_VOID prvTgfBrgL2EcmpEntryClear
(
    IN GT_U32  lttIndex,
    IN GT_BOOL resetMemebers
)
{
    GT_STATUS rc;
    PRV_TGF_BRG_L2_ECMP_LTT_ENTRY_STC           ecmpLttEntry;
    PRV_TGF_BRG_L2_ECMP_LTT_ENTRY_STC           resetEcmpLttEntry;
    PRV_TGF_BRG_L2_ECMP_ENTRY_STC               resetEcmpEntry;
    GT_U32  ii;
    GT_U32  index;

    PRV_UTF_LOG1_MAC("prvTgfBrgL2EcmpEntryClear (%d) \n",
        lttIndex);

    cpssOsMemSet(&ecmpLttEntry, 0, sizeof(ecmpLttEntry));

    rc = prvTgfBrgL2EcmpLttTableGet(prvTgfDevNum, lttIndex, &ecmpLttEntry);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgL2EcmpLttTableGet: index %d",
                                 lttIndex);

    /* clear L2 ECMP LTT entries */
    cpssOsMemSet(&resetEcmpLttEntry, 0, sizeof(resetEcmpLttEntry));
    resetEcmpLttEntry.ecmpNumOfPaths = 1;
    rc = prvTgfBrgL2EcmpLttTableSet(prvTgfDevNum, lttIndex, &resetEcmpLttEntry);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgL2EcmpLttTableGet: index %d",
                                 lttIndex);

    if(resetMemebers == GT_TRUE)
    {
        /* clear L2 ECMP entries */
        cpssOsMemSet(&resetEcmpEntry, 0, sizeof(resetEcmpEntry));

        index = ecmpLttEntry.ecmpStartIndex;
        for(ii = 0 ; ii < ecmpLttEntry.ecmpNumOfPaths; ii++ , index++)
        {
            rc = prvTgfBrgL2EcmpTableSet(prvTgfDevNum, index, &resetEcmpEntry);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgL2EcmpTableSet: index %d",
                                         index);
        }
    }


    return;
}

/**
* @internal prvTgfBrgL2EcmpHashBitSelectionProfileSet function
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
*/
GT_STATUS prvTgfBrgL2EcmpHashBitSelectionProfileSet
(
    IN GT_U8                                     devNum,
    IN GT_U32                                    profileIndex,
    IN PRV_TGF_HASH_BIT_SELECTION_PROFILE_STC    *hashBitSelectionProfileInfoPtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS   rc = GT_OK;

    CPSS_DXCH_HASH_BIT_SELECTION_PROFILE_STC    dxChHashBitSelectionProfileEntry;

    /* convert hash bit selection entry into device specific format */
    rc = prvTgfConvertGenericToDxChHashBitSelectionProfileEntry(hashBitSelectionProfileInfoPtr, &dxChHashBitSelectionProfileEntry);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChHashBitSelectionProfileEntry FAILED, rc = [%d]", rc);

        return rc;
    }

    /* call device specific API */
    rc = cpssDxChBrgL2EcmpHashBitSelectionProfileSet(devNum, profileIndex, &dxChHashBitSelectionProfileEntry);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChBrgL2EcmpHashBitSelectionProfileSet FAILED, rc = [%d]", rc);

        return rc;
    }

    return rc;
#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfBrgL2EcmpHashBitSelectionProfileGet function
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
GT_STATUS prvTgfBrgL2EcmpHashBitSelectionProfileGet
(
    IN   GT_U8                                     devNum,
    IN   GT_U32                                    profileIndex,
    OUT  PRV_TGF_HASH_BIT_SELECTION_PROFILE_STC    *hashBitSelectionProfileInfoPtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS   rc = GT_OK;

    CPSS_DXCH_HASH_BIT_SELECTION_PROFILE_STC    dxChHashBitSelectionProfileEntry;

    /* call device specific API */
    rc = cpssDxChBrgL2EcmpHashBitSelectionProfileGet(devNum, profileIndex, &dxChHashBitSelectionProfileEntry);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChBrgL2EcmpHashBitSelectionProfileGet FAILED, rc = [%d]", rc);

        return rc;
    }

    /* convert hash bit selection profile */
    rc = prvTgfConvertDxChToGenericHashBitSelectionProfileEntry(&dxChHashBitSelectionProfileEntry, hashBitSelectionProfileInfoPtr);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertDxChToGenericHashBitSelectionProfileEntry FAILED, rc = [%d]", rc);

        return rc;
    }


    return rc;
#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfBrgL2EcmpPortHashSet function
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
GT_STATUS prvTgfBrgL2EcmpPortHashSet
(
    IN GT_U8                        devNum,
    IN GT_PHYSICAL_PORT_NUM         portNum,
    IN GT_U32                       srcPortHash
)
{
#ifdef CHX_FAMILY
    GT_STATUS   rc = GT_OK;

    /* call device specific API */
    rc = cpssDxChBrgL2EcmpPortHashSet(devNum, portNum, srcPortHash);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChBrgL2EcmpPortHashSet FAILED, rc = [%d]", rc);

        return rc;
    }

    return rc;
#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfBrgL2EcmpPortHashGet function
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
GT_STATUS prvTgfBrgL2EcmpPortHashGet
(
    IN  GT_U8                        devNum,
    IN  GT_PHYSICAL_PORT_NUM         portNum,
    OUT GT_U32                       *srcPortHashPtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS   rc = GT_OK;

    /* call device specific API */
    rc = cpssDxChBrgL2EcmpPortHashGet(devNum, portNum, srcPortHashPtr);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChBrgL2EcmpPortHashGet FAILED, rc = [%d]", rc);

        return rc;
    }

    return rc;
#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfBrgL2EcmpPortHashBitSelectionSet function
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
GT_STATUS prvTgfBrgL2EcmpPortHashBitSelectionSet
(
    IN  GT_U8      devNum,
    IN  GT_U32     numBitsInHash
)
{
#ifdef CHX_FAMILY
    GT_STATUS   rc = GT_OK;

    /* call device specific API */
    rc = cpssDxChBrgL2EcmpPortHashBitSelectionSet(devNum, numBitsInHash);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChBrgL2EcmpPortHashBitSelectionSet FAILED, rc = [%d]", rc);

        return rc;
    }

    return rc;
#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfBrgL2EcmpPortHashBitSelectionGet function
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
GT_STATUS prvTgfBrgL2EcmpPortHashBitSelectionGet
(
    IN  GT_U8      devNum,
    OUT GT_U32     *numBitsInHashPtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS   rc = GT_OK;

    /* call device specific API */
    rc = cpssDxChBrgL2EcmpPortHashBitSelectionGet(devNum, numBitsInHashPtr);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChBrgL2EcmpPortHashBitSelectionGet FAILED, rc = [%d]", rc);

        return rc;
    }

    return rc;
#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfBrgL2EcmpMllReplicationConfigSet function
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
GT_STATUS prvTgfBrgL2EcmpMllReplicationConfigSet
(
    IN  GT_U8                                           devNum,
    IN  CPSS_DXCH_BRG_L2_ECMP_MLL_REPLICATION_MODE_ENT  replicationMode,
    IN  CPSS_NET_RX_CPU_CODE_ENT                        replicationDropCode
)
{
#ifdef CHX_FAMILY
    GT_STATUS   rc = GT_OK;

    /* call device specific API */
    rc = cpssDxChBrgL2EcmpMllReplicationConfigSet(devNum, replicationMode, replicationDropCode);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChBrgL2EcmpMllReplicationConfigSet FAILED, rc = [%d]", rc);

        return rc;
    }

    return rc;
#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}

/**
* @internal prvTgfBrgL2EcmpMllReplicationConfigGet function
* @endinternal
*
* @brief   Get L2 ECMP MLL Replication Mode and Replication Drop Code.
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
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, replicationMode and replicationDropCode
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfBrgL2EcmpMllReplicationConfigGet
(
    IN  GT_U8                                            devNum,
    OUT CPSS_DXCH_BRG_L2_ECMP_MLL_REPLICATION_MODE_ENT   *replicationModePtr,
    OUT CPSS_NET_RX_CPU_CODE_ENT                         *replicationDropCodePtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS   rc = GT_OK;

    /* call device specific API */
    rc = cpssDxChBrgL2EcmpMllReplicationConfigGet(devNum, replicationModePtr, replicationDropCodePtr);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChBrgL2EcmpMllReplicationConfigGet FAILED, rc = [%d]", rc);

        return rc;
    }

    return rc;
#endif /* CHX_FAMILY */

#if !(defined CHX_FAMILY)
    return GT_BAD_STATE;
#endif /* !(defined CHX_FAMILY) */
}
