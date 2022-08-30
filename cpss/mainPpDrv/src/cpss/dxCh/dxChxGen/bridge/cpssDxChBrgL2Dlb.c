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
* @file cpssDxChBrgL2Dlb.c
*
* @brief L2 DLB facility CPSS DxCh implementation.
*
*
* @version   1
********************************************************************************
*/
#define CPSS_LOG_IN_MODULE_ENABLE
#include <cpss/dxCh/dxChxGen/bridge/private/prvCpssDxChBrgLog.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgL2Dlb.h>
#include <cpss/dxCh/dxChxGen/trunk/private/prvCpssDxChTrunk.h>
#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgL2Ecmp.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

/* check that the index not over it's boundaries */
/* Note that the l2ecmp get 'only' lower part of the 'Hosting table'
  the other part is for the trunk usage */
#define ECMP_MEMBER_INDEX_CHECK_MAC(_devNum,_index)    \
    if((_index) >= ((PRV_CPSS_DXCH_PP_MAC(_devNum)->bridge.l2EcmpNumMembers - \
                     PRV_CPSS_DXCH_PP_MAC(_devNum)->bridge.l2EcmpNumMembers_usedByTrunk)))   \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG)

static const CPSS_DXCH_TABLE_ENT pathUtilizationTable[] = { CPSS_DXCH_SIP6_TABLE_EQ_L2ECMP_PATH_UTILIZATION_TABLE0_E,
                                                      CPSS_DXCH_SIP6_TABLE_EQ_L2ECMP_PATH_UTILIZATION_TABLE1_E,
                                                      CPSS_DXCH_SIP6_TABLE_EQ_L2ECMP_PATH_UTILIZATION_TABLE2_E,
                                                      CPSS_DXCH_SIP6_TABLE_EQ_L2ECMP_PATH_UTILIZATION_TABLE3_E,
                                                      CPSS_DXCH_TABLE_LAST_E
                                                    };

#define DLB_PATH_UTILIZATION_TABLE_INDEX_CHECK(_index, _target) \
    if(_index >= _target)                                              \
    {                                                                  \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG); \
    }

/* Reference clock frequency for ePort DLB current timer
 * 25 cycles of ref clock increments the ePort DLB current timer by 1 */
#define PRV_DLB_LTT_REF_CLK_CNS                     25

/* Macro to access erratum data structure */
#define PRV_DLB_LTT_WA_ERRTA_MAC(_dev)          \
    PRV_CPSS_DXCH_PP_MAC(_dev)->errata.info_PRV_CPSS_DXCH_BRG_L2_DLB_TIMER_COUNTER_WA_E

/* Macro to calculate the primary block size */
#define PRV_DLB_LTT_PRIMARY_BLOCK_SIZE_MAC(_dev)\
    (PRV_DLB_LTT_WA_ERRTA_MAC(_dev).secondaryLttBaseIndex - PRV_DLB_LTT_WA_ERRTA_MAC(_dev).primaryLttBaseIndex)

/* Macro to calculate the offset for primary LTT entry, based on current block in use(blockInUse) */
#define PRV_DLB_LTT_PRIMARY_SHIFT(_dev) \
    (PRV_DLB_LTT_PRIMARY_BLOCK_SIZE_MAC(_dev) * PRV_DLB_LTT_WA_ERRTA_MAC(_dev).blockInUse)

/* Macro to calculate the offset for secondary LTT entry, based on current block in use(blockInUse) */
#define PRV_DLB_LTT_SECONDARY_SHIFT(_dev) \
    (PRV_DLB_LTT_PRIMARY_BLOCK_SIZE_MAC(_dev) * (PRV_DLB_LTT_WA_ERRTA_MAC(_dev).blockInUse^1))

/* Macro to find out last secondary base index, based on current block in use(blockInUse)
Base eport when primary region is use   = primary_base_ePort
Base eport when secondary region is use = primary_base_ePort - size_of_primary_regioin
Logic : forwarding engine matches with index (targetEport - LTT base index)
*/
#define PRV_DLB_LTT_LAST_SECONDARY_BASE_MAC(_dev) \
    ((PRV_DLB_LTT_WA_ERRTA_MAC(_dev).primaryLttBaseIndex) + (PRV_DLB_LTT_PRIMARY_BLOCK_SIZE_MAC(_dev) * (PRV_DLB_LTT_WA_ERRTA_MAC(_dev).blockInUse - 1)))

/*
* @internal internal_cpssDxChBrgL2DlbEnableSet function
* @endinternal
*
* @brief   Set the global enable/disable Dynamic Load Balancing(DLB).
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*                                  xCat3; AC5; Lion2.
*
* @param[in] devNum              - device number
* @param[in] enable              - DLB status:
*                                  GT_TRUE : DLB is enabled
*                                  GT_FALSE: DLB is disabled
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgL2DlbEnableSet
(
    IN  GT_U8       devNum,
    IN GT_BOOL      enable
)
{
    GT_U32      regAddr;
    GT_U32      hwData;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
          CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_XCAT3_E | CPSS_AC5_E | CPSS_BOBCAT3_E | CPSS_CAELUM_E |
          CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_ALDRIN2_E);

    regAddr = PRV_DXCH_REG1_UNIT_EQ_MAC(devNum).L2ECMP.globalEportDLBConfig;

    hwData = BOOL2BIT_MAC(enable);
    return prvCpssHwPpSetRegField(devNum, regAddr, 12, 1, hwData);
}

/*
* @internal cpssDxChBrgL2DlbEnableSet function
* @endinternal
*
* @brief   Set the global enable/disable Dynamic Load Balancing(DLB).
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*                                  xCat3; AC5; Lion2.
*
* @param[in] devNum              - device number
* @param[in] enable              - DLB status:
*                                  GT_TRUE : DLB is enabled
*                                  GT_FALSE: DLB is disabled
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgL2DlbEnableSet
(
    IN  GT_U8       devNum,
    IN GT_BOOL      enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgL2DlbEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, enable));

    rc = internal_cpssDxChBrgL2DlbEnableSet(devNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgL2DlbEnableGet function
* @endinternal
*
* @brief   Get the global enable/disable Dynamic Load Balancing(DLB).
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*                                  xCat3; AC5; Lion2.
*
* @param[in] devNum              - device number
* @param[out] enablePtr          - (pointer to)DLB status:
*                                  GT_TRUE : DLB is enabled
*                                  GT_FALSE: DLB is disabled
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgL2DlbEnableGet
(
    IN  GT_U8       devNum,
    OUT GT_BOOL     *enablePtr
)
{
    GT_U32      regAddr;
    GT_U32      hwData;
    GT_STATUS   rc;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
          CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_XCAT3_E | CPSS_AC5_E | CPSS_BOBCAT3_E | CPSS_CAELUM_E |
          CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_ALDRIN2_E);

    CPSS_NULL_PTR_CHECK_MAC(enablePtr);
    regAddr = PRV_DXCH_REG1_UNIT_EQ_MAC(devNum).L2ECMP.globalEportDLBConfig;

    rc = prvCpssHwPpGetRegField(devNum, regAddr, 12, 1, &hwData);
    *enablePtr = BIT2BOOL_MAC(hwData);
    return rc;
}

/**
* @internal cpssDxChBrgL2DlbEnableGet function
* @endinternal
*
* @brief   Get the global enable/disable Dynamic Load Balancing(DLB).
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*                                  xCat3; AC5; Lion2.
*
* @param[in] devNum              - device number
* @param[out] enablePtr          - (pointer to)DLB status:
*                                  GT_TRUE : DLB is enabled
*                                  GT_FALSE: DLB is disabled
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgL2DlbEnableGet
(
    IN  GT_U8       devNum,
    OUT GT_BOOL     *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgL2DlbEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, enablePtr));

    rc = internal_cpssDxChBrgL2DlbEnableGet(devNum, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    return rc;
}

/**
* @internal internal_cpssDxChBrgL2DlbLttIndexBaseEportSet function
* @endinternal
*
* @brief  Defines the first ePort number in the L2 DLB ePort number range.
*         The index to the L2ECMP LTT Table is <Target ePort>-<DLB LTT Base ePort>.
*
* @note   APPLICABLE DEVICES:     Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*                                 xCat3; AC5; Lion2.
*
* @param[in] devNum                - device number
* @param[in] dlbIndexBaseEport     - the ePort number
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgL2DlbLttIndexBaseEportSet
(
    IN GT_U8            devNum,
    IN GT_PORT_NUM      dlbIndexBaseEport
)
{
    GT_U32      regAddr;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
          CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_XCAT3_E | CPSS_AC5_E | CPSS_BOBCAT3_E | CPSS_CAELUM_E |
          CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_ALDRIN2_E);

    if(dlbIndexBaseEport > PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_E_PORT_MAC(devNum))
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

    if(PRV_DLB_LTT_WA_ERRTA_MAC(devNum).dlbWaEnable == GT_TRUE)
    {
        /* Update the primary region base index, required for the WA */
        PRV_DLB_LTT_WA_ERRTA_MAC(devNum).primaryLttBaseIndex = dlbIndexBaseEport;
    }

    regAddr = PRV_DXCH_REG1_UNIT_EQ_MAC(devNum).L2ECMP.dlbLTTIndexBaseEport;

    return prvCpssHwPpSetRegField(devNum, regAddr, 0, 15, dlbIndexBaseEport);
}

/**
* @internal cpssDxChBrgL2DlbLttIndexBaseEportSet function
* @endinternal
*
* @brief  Defines the first ePort number in the L2 DLB ePort number range.
*         The index to the L2ECMP LTT Table is <Target ePort>-<DLB LTT Base ePort>.
*
* @note   APPLICABLE DEVICES:     Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*                                 xCat3; AC5; Lion2.
*
* @param[in] devNum                - device number
* @param[in] dlbIndexBaseEport     - the ePort number
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgL2DlbLttIndexBaseEportSet
(
    IN GT_U8            devNum,
    IN GT_PORT_NUM      dlbIndexBaseEport
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgL2DlbLttIndexBaseEportSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, dlbIndexBaseEport));

    rc = internal_cpssDxChBrgL2DlbLttIndexBaseEportSet(devNum, dlbIndexBaseEport);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, dlbIndexBaseEport));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    return rc;
}

/**
* @internal internal_cpssDxChBrgL2DlbLttIndexBaseEportGet function
* @endinternal
*
* @brief  Get the first ePort number in the L2 DLB ePort number range.
*         The index to the L2 DLB LTT is <Target ePort>-<DLB LTT Base ePort>.
*
* @note   APPLICABLE DEVICES:     Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*                                 xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[out]dlbIndexBaseEportPtr     - (pointer to)the ePort number
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_BAD_PTR               - one of the parameter is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgL2DlbLttIndexBaseEportGet
(
    IN GT_U8            devNum,
    OUT GT_PORT_NUM     *dlbIndexBaseEportPtr
)
{
    GT_STATUS   rc;
    GT_U32      hwData;
    GT_U32      regAddr;

    /* check parameters */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
          CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_XCAT3_E | CPSS_AC5_E | CPSS_BOBCAT3_E | CPSS_CAELUM_E |
          CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_ALDRIN2_E);

    CPSS_NULL_PTR_CHECK_MAC(dlbIndexBaseEportPtr);

    regAddr = PRV_DXCH_REG1_UNIT_EQ_MAC(devNum).L2ECMP.dlbLTTIndexBaseEport;
    rc = prvCpssHwPpGetRegField(devNum,regAddr ,0 ,15 ,&hwData);

    *dlbIndexBaseEportPtr = hwData;

    return rc;
}

/**
* @internal cpssDxChBrgL2DlbLttIndexBaseEportGet function
* @endinternal
*
* @brief  Get the first ePort number in the L2 DLB ePort number range.
*         The index to the L2 DLB LTT is <Target ePort>-<DLB LTT Base ePort>.
*
* @note   APPLICABLE DEVICES:     Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*                                 xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[out]dlbIndexBaseEportPtr     - (pointer to)the ePort number
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_BAD_PTR               - one of the parameter is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgL2DlbLttIndexBaseEportGet
(
    IN GT_U8            devNum,
    OUT GT_PORT_NUM     *dlbIndexBaseEportPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgL2DlbLttIndexBaseEportGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, dlbIndexBaseEportPtr));

    rc = internal_cpssDxChBrgL2DlbLttIndexBaseEportGet(devNum, dlbIndexBaseEportPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, dlbIndexBaseEportPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    return rc;
}

/**
* @internal internal_cpssDxChBrgL2DlbTableEntrySet function
* @endinternal
*
* @brief   Set L2 DLB entry info.
*
* @note   APPLICABLE DEVICES:     Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; xCat3; AC5; Lion2.
*
* @param[in] devNum                - device number
* @param[in] index                 - the DLB entry index
*
* @param[in] dlbEntryPtr           - (pointer to) L2 DLB entry.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong parameters
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_OUT_OF_RANGE          - parameter out of range.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note 1. Both ECMP and DLB feature use same ECMP Table
*       2. Index according to index from L2 ECMP LTT
*       3. When trunk mode is 'CPSS_DXCH_TRUNK_MEMBERS_MODE_FLEX_E' the CPSS allows
*       access to all table entries.
*       Otherwise the CPSS allows access to ONLY to lower half of table entries.
*
*/
static GT_STATUS internal_cpssDxChBrgL2DlbTableEntrySet
(
    IN  GT_U8                                devNum,
    IN  GT_U32                               index,
    IN  CPSS_DXCH_BRG_L2_DLB_ENTRY_STC       *dlbEntryPtr
)
{
    GT_STATUS rc;
    GT_U32    hwFormat=0;

    /* check parameters */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
          CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_XCAT3_E | CPSS_AC5_E | CPSS_BOBCAT3_E | CPSS_CAELUM_E |
          CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_ALDRIN2_E);

    CPSS_NULL_PTR_CHECK_MAC(dlbEntryPtr);

    /* check that index not exceed the table */
    ECMP_MEMBER_INDEX_CHECK_MAC(devNum,index);

    if(dlbEntryPtr->pathId > CPSS_DXCH_BRG_L2_DLB_MAX_PATH)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    if(dlbEntryPtr->lastSeenTimeStamp > 0x7FFFF)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    U32_SET_FIELD_MAC(hwFormat,0 ,19,dlbEntryPtr->lastSeenTimeStamp);
    U32_SET_FIELD_MAC(hwFormat,19,4,dlbEntryPtr->pathId);

    rc = prvCpssDxChPortGroupWriteTableEntry(devNum,
                                             CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                             CPSS_DXCH_SIP5_TABLE_EQ_L2_ECMP_E,
                                             index,
                                             &hwFormat);
    return rc;
}

/**
* @internal cpssDxChBrgL2DlbTableEntrySet function
* @endinternal
*
* @brief   Set L2 DLB entry info.
*
* @note   APPLICABLE DEVICES:     Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; xCat3; AC5; Lion2.
*
* @param[in] devNum                - device number
* @param[in] index                 - the DLB entry index
*
* @param[in] dlbEntryPtr           - (pointer to) L2 DLB entry.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong parameters
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_OUT_OF_RANGE          - parameter out of range.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note 1. Both ECMP and DLB feature use same ECMP Table
*       2. Index according to index from L2 ECMP LTT
*       3. When trunk mode is 'CPSS_DXCH_TRUNK_MEMBERS_MODE_FLEX_E' the CPSS allows
*       access to all table entries.
*       Otherwise the CPSS allows access to ONLY to lower half of table entries.
*
*/
GT_STATUS cpssDxChBrgL2DlbTableEntrySet
(
    IN  GT_U8                                devNum,
    IN  GT_U32                               index,
    IN  CPSS_DXCH_BRG_L2_DLB_ENTRY_STC       *dlbEntryPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgL2DlbTableEntrySet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, index, dlbEntryPtr));

    rc = internal_cpssDxChBrgL2DlbTableEntrySet(devNum, index, dlbEntryPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, index, dlbEntryPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    return rc;
}

/**
* @internal internal_cpssDxChBrgL2DlbTableEntryGet function
* @endinternal
*
* @brief   Get L2 DLB entry info.
*
* @note   APPLICABLE DEVICES:     Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; xCat3; AC5; Lion2.
*
* @param[in] devNum                - device number
* @param[in] index                 - the DLB entry index
*
* @param[out] dlbEntryPtr          - (pointer to) L2 DLB entry.
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
static GT_STATUS internal_cpssDxChBrgL2DlbTableEntryGet
(
    IN  GT_U8                                devNum,
    IN  GT_U32                               index,
    OUT CPSS_DXCH_BRG_L2_DLB_ENTRY_STC     *dlbEntryPtr
)
{
    GT_STATUS rc;               /* return code          */
    GT_U32    hwFormat=0;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(dlbEntryPtr);

    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
          CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_XCAT3_E | CPSS_AC5_E | CPSS_BOBCAT3_E | CPSS_CAELUM_E |
          CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_ALDRIN2_E);

    /* check that index not exceed the table */
    ECMP_MEMBER_INDEX_CHECK_MAC(devNum,index);

    rc = prvCpssDxChPortGroupReadTableEntry(devNum,
                                         CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                         CPSS_DXCH_SIP5_TABLE_EQ_L2_ECMP_E,
                                         index,
                                         &hwFormat);
    if(rc != GT_OK)
        return rc;

    dlbEntryPtr->lastSeenTimeStamp  = U32_GET_FIELD_MAC(hwFormat,0,19);
    dlbEntryPtr->pathId             = U32_GET_FIELD_MAC(hwFormat,19,4);

    return rc;
}

/**
* @internal cpssDxChBrgL2DlbTableEntryGet function
* @endinternal
*
* @brief   Get L2 DLB entry info.
*
* @note   APPLICABLE DEVICES:     Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; xCat3; AC5; Lion2.
*
* @param[in] devNum                - device number
* @param[in] index                 - the DLB entry index
*
* @param[out] dlbEntryPtr          - (pointer to) L2 DLB entry.
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
GT_STATUS cpssDxChBrgL2DlbTableEntryGet
(
    IN  GT_U8                                devNum,
    IN  GT_U32                               index,
    OUT CPSS_DXCH_BRG_L2_DLB_ENTRY_STC       *dlbEntryPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgL2DlbTableEntryGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, index, dlbEntryPtr));

    rc = internal_cpssDxChBrgL2DlbTableEntryGet(devNum, index, dlbEntryPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, index, dlbEntryPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    return rc;
}


/**
* @internal internal_cpssDxChBrgL2DlbPathUtilizationScaleModeSet function
* @endinternal
*
* @brief  Configure the path utilization scale mode for DLB.
*
* @note   APPLICABLE DEVICES:     Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*                                 xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] scaleMode                - path utilization scaling mode.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgL2DlbPathUtilizationScaleModeSet
(
    IN GT_U8                                devNum,
    IN CPSS_DXCH_BRG_L2_DLB_SCALE_MODE_ENT  scaleMode
)
{
    GT_U32    regAddr;
    GT_U32    hwData=0;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
          CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_XCAT3_E | CPSS_AC5_E | CPSS_BOBCAT3_E | CPSS_CAELUM_E |
          CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_ALDRIN2_E);

    switch(scaleMode)
    {
        case CPSS_DXCH_BRG_L2_DLB_16_PATHS_MODE_E:
            hwData = 0;
            break;
        case CPSS_DXCH_BRG_L2_DLB_8_PATHS_MODE_E:
            hwData = 1;
            break;
        case CPSS_DXCH_BRG_L2_DLB_4_PATHS_MODE_E:
            hwData = 2;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    regAddr = PRV_DXCH_REG1_UNIT_EQ_MAC(devNum).L2ECMP.globalEportDLBConfig;

    return prvCpssHwPpSetRegField(devNum, regAddr, 0, 2, hwData);
}

/**
* @internal cpssDxChBrgL2DlbPathUtilizationScaleModeSet function
* @endinternal
*
* @brief  Configure the path utilization scale mode for DLB.
*
* @note   APPLICABLE DEVICES:     Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*                                 xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] scaleMode                - path utilization scaling mode.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgL2DlbPathUtilizationScaleModeSet
(
    IN GT_U8                                devNum,
    IN CPSS_DXCH_BRG_L2_DLB_SCALE_MODE_ENT  scaleMode
)
{

    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgL2DlbPathUtilizationScaleModeSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, scaleMode));

    rc = internal_cpssDxChBrgL2DlbPathUtilizationScaleModeSet(devNum, scaleMode);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, scaleMode));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    return rc;
}

/**
* @internal internal_cpssDxChBrgL2DlbPathUtilizationScaleModeGet function
* @endinternal
*
* @brief  Get the path utilization scale mode for DLB.
*
* @note   APPLICABLE DEVICES:     Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*                                 xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[out] scaleModePtr            - (pointer to)path utilization scaling mode.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_STATE             - on illegal value from hw.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgL2DlbPathUtilizationScaleModeGet
(
    IN GT_U8                                 devNum,
    OUT CPSS_DXCH_BRG_L2_DLB_SCALE_MODE_ENT  *scaleModePtr
)
{
    GT_STATUS rc;               /* return code */
    GT_U32    regAddr;
    GT_U32    hwData=0;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(scaleModePtr);

    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
          CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_XCAT3_E | CPSS_AC5_E | CPSS_BOBCAT3_E | CPSS_CAELUM_E |
          CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_ALDRIN2_E);


    regAddr = PRV_DXCH_REG1_UNIT_EQ_MAC(devNum).L2ECMP.globalEportDLBConfig;

    rc = prvCpssHwPpGetRegField(devNum, regAddr, 0, 2, &hwData);

    if( rc != GT_OK)
    {
        return rc;
    }

    switch(hwData)
    {
        case 0:
            *scaleModePtr = CPSS_DXCH_BRG_L2_DLB_16_PATHS_MODE_E;
            break;
        case 1:
            *scaleModePtr = CPSS_DXCH_BRG_L2_DLB_8_PATHS_MODE_E;
            break;
        case 2:
            *scaleModePtr = CPSS_DXCH_BRG_L2_DLB_4_PATHS_MODE_E;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }
    return rc;
}

/**
* @internal cpssDxChBrgL2DlbPathUtilizationScaleModeGet function
* @endinternal
*
* @brief  Get the path utilization scale mode for DLB.
*
* @note   APPLICABLE DEVICES:     Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*                                 xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[out] scaleModePtr            - (pointer to)path utilization scaling mode.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgL2DlbPathUtilizationScaleModeGet
(
    IN GT_U8                                 devNum,
    OUT CPSS_DXCH_BRG_L2_DLB_SCALE_MODE_ENT  *scaleModePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgL2DlbPathUtilizationScaleModeGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, scaleModePtr));

    rc = internal_cpssDxChBrgL2DlbPathUtilizationScaleModeGet(devNum, scaleModePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, scaleModePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    return rc;
}

/**
* @internal internal_cpssDxChBrgL2DlbMinimumFlowletIpgSet function
* @endinternal
*
* @brief  Configure the minimum inter-packet gap(IPG) for flowlet boundary identification.
*
* @note   APPLICABLE DEVICES:     Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*                                 xCat3; AC5; Lion2.
*
* @param[in] devNum                - device number
* @param[in] minimumIpg            - Minimum Inter-packet Gap in micro-second.
*                                    Applicable range(0 - 0x7FFFF)

* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_OUT_OF_RANGE          -  parameter out of range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS internal_cpssDxChBrgL2DlbMinimumFlowletIpgSet
(
    IN GT_U8    devNum,
    IN GT_U32   minimumIpg
)
{
    GT_U32      regAddr;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
          CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_XCAT3_E | CPSS_AC5_E | CPSS_BOBCAT3_E | CPSS_CAELUM_E |
          CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_ALDRIN2_E);

    if(PRV_DLB_LTT_WA_ERRTA_MAC(devNum).dlbWaEnable)
    {
        /* dlbWaEnable - Updates DLB clock cycle to 10micro Sec(Default 1micro Sec) */
        minimumIpg /=10;
    }
    if(minimumIpg >= BIT_19)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);

    regAddr = PRV_DXCH_REG1_UNIT_EQ_MAC(devNum).L2ECMP.dlbMinimumFlowletIpg;

    return prvCpssHwPpSetRegField(devNum, regAddr, 0, 19, minimumIpg);

}

/**
* @internal cpssDxChBrgL2DlbMinimumFlowletIpgSet function
* @endinternal
*
* @brief  Configure the minimum inter-packet gap(IPG) for flowlet boundary identification.
*
* @note   APPLICABLE DEVICES:     Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*                                 xCat3; AC5; Lion2.
*
* @param[in] devNum                - device number
* @param[in] minimumIpg            - Minimum Inter-packet Gap in micro-second.
*                                    Applicable range(0 - 0x7FFFF)

* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_OUT_OF_RANGE          -  parameter out of range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgL2DlbMinimumFlowletIpgSet
(
    IN GT_U8    devNum,
    IN GT_U32   minimumIpg
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgL2DlbMinimumFlowletIpgSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, minimumIpg));

    rc = internal_cpssDxChBrgL2DlbMinimumFlowletIpgSet(devNum, minimumIpg);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, minimumIpg));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    return rc;
}

/**
* @internal internal_cpssDxChBrgL2DlbMinimumFlowletIpgGet function
* @endinternal
*
* @brief  Get the minimum inter-packet gap(IPG) for flowlet boundary identification.
*
* @note   APPLICABLE DEVICES:     Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*                                 xCat3; AC5; Lion2.
*
* @param[in] devNum                - device number
* @param[out] minimumIpgPtr        - Inter-packet Gap.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS internal_cpssDxChBrgL2DlbMinimumFlowletIpgGet
(
    IN GT_U8                          devNum,
    OUT GT_U32                        *minimumIpgPtr
)
{
    GT_STATUS   rc;
    GT_U32      regAddr;
    GT_U32      hwData;
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(minimumIpgPtr);

    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
          CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_XCAT3_E | CPSS_AC5_E | CPSS_BOBCAT3_E | CPSS_CAELUM_E |
          CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_ALDRIN2_E);

    regAddr = PRV_DXCH_REG1_UNIT_EQ_MAC(devNum).L2ECMP.dlbMinimumFlowletIpg;

    rc = prvCpssHwPpGetRegField(devNum, regAddr, 0, 19, &hwData);

    /* dlbWaEnable - Updates DLB clock cycle to 10micro Sec(Default 1micro Sec) */
    *minimumIpgPtr = (PRV_DLB_LTT_WA_ERRTA_MAC(devNum).dlbWaEnable)?hwData*10:hwData;

    return rc;
}

/**
* @internal cpssDxChBrgL2DlbMinimumFlowletIpgGet function
* @endinternal
*
* @brief  Get the minimum inter-packet gap(IPG) for flowlet boundary identification.
*
* @note   APPLICABLE DEVICES:     Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*                                 xCat3; AC5; Lion2.
*
* @param[in] devNum                - device number
* @param[out] minimumIpgPtr        - Inter-packet Gap.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters.
* @retval GT_HW_ERROR              - failed to write to hardware.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgL2DlbMinimumFlowletIpgGet
(
    IN GT_U8                          devNum,
    OUT GT_U32                        *minimumIpgPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgL2DlbMinimumFlowletIpgGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, minimumIpgPtr));

    rc = internal_cpssDxChBrgL2DlbMinimumFlowletIpgGet(devNum, minimumIpgPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, minimumIpgPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    return rc;
}

/**
* @internal dlbPathTargetGet function
* @endinternal
*
* @brief  DLB path weight calculation.
*
* @note   APPLICABLE DEVICES:     Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*                                 xCat3; AC5; Lion2.
*
* @param[in] devNum                - device number
* @param[in] dlbEntryArr           - DLB path utilization table entry array.
*                                    Array indexes are used based on path utilization scale mode as below.
* @param[in] numOfPaths            - number of path selected for DLB.
* @param[in] maxTarget             - max number of target supported.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters.
* @retval GT_HW_ERROR              - failed to write to hardware.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - on error.
*/
static GT_STATUS dlbPathTargetGet
(
    IN GT_U8                               devNum,
    OUT GT_U32                             *numOfPaths,
    OUT GT_U32                             *maxTarget
)
{
    GT_STATUS rc;
    CPSS_DXCH_BRG_L2_DLB_SCALE_MODE_ENT scaleMode;

    rc = cpssDxChBrgL2DlbPathUtilizationScaleModeGet(devNum, &scaleMode);

    if( rc != GT_OK)
    {
        return rc;
    }

    switch(scaleMode)
    {
        case CPSS_DXCH_BRG_L2_DLB_16_PATHS_MODE_E:
            *numOfPaths = 16;
            *maxTarget  = 512;
            break;
        case CPSS_DXCH_BRG_L2_DLB_8_PATHS_MODE_E:
            *numOfPaths = 8;
            *maxTarget  = 1024;
            break;
        case CPSS_DXCH_BRG_L2_DLB_4_PATHS_MODE_E:
            *numOfPaths = 4;
            *maxTarget  = 2048;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    return rc;
}

/**
* @internal internal_cpssDxChBrgL2DlbPathUtilizationTableEntrySet function
* @endinternal
*
* @brief  Configure the Path Utilization Table entry(Target ePort/Device, Weight).
*         Path weight is assigned in proportion to the utilizable percentage of bandwidth.
*         i.e 100% utilization percentage.
*                * Weight 100 equals full BW available.
*                * Weight 50 is assigned for 50% available BM
*                * Weight 25 is assigned for available 25% BW
*                * Weight 0 is assigned for path which has no BW.
*         Relative weight is normalized to span the range [0,1024]
*         such that the sum of all the path weights is equal '1024'
*
* @note   APPLICABLE DEVICES:     Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*                                 xCat3; AC5; Lion2.
*
* @param[in] devNum                - device number
* @param[in] index                 - the DLB entry Index.
* @param[in] dlbEntryArr           - DLB path utilization table entry array.
*                                    Array indexes are used based on path utilization scale mode as below.
*                                       512 x 16 path  - 16 indexes are used
*                                                 |table3|table2|table1|table0|
*                                       1K x 8 path    - Only first 8 indexes are used.
*                                                   |table2|table0|
*                                                   |table3|table1|
*                                       2K x 4 path    - Only first 4 indexes are used
*                                                   |table0|
*                                                   |table1|
*                                                   |table2|
*                                                   |table3|
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_OUT_OF_RANGE          - parameter out of range.
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgL2DlbPathUtilizationTableEntrySet
(
    IN GT_U8                               devNum,
    IN GT_U32                              index,
    IN CPSS_DXCH_BRG_L2_DLB_PATH_ENTRY_STC dlbEntryArr[CPSS_DXCH_BRG_L2_DLB_MAX_PATH]
)
{
    GT_STATUS  rc;
    GT_U32     hwData[5] = {0,0,0,0,0};
    GT_U32     i,j;
    GT_U32     weight = 0;
    GT_U32     numOfPaths, maxTarget;
    GT_U32     offset, tableId;
    GT_FLOAT32 tmpValue;
    GT_U32     hwWeight = 0;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
          CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_XCAT3_E | CPSS_AC5_E | CPSS_BOBCAT3_E | CPSS_CAELUM_E |
          CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_ALDRIN2_E);

    rc = dlbPathTargetGet(devNum, &numOfPaths, &maxTarget);
    if( rc != GT_OK)
    {
        return rc;
    }

    for( i = 0; i < numOfPaths; i++)
    {
        weight = weight + dlbEntryArr[i].weight;
    }
    /* Sum of path weight in percentage */
    if ((weight != 0) && (weight != 100))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,
               "sum of path weights:%d should be equal to 100% : ", weight);
    }

    DLB_PATH_UTILIZATION_TABLE_INDEX_CHECK(index, maxTarget);

    tableId = (index / 512) ;

    /* 512 entries per table */
    index  = index%512;
    offset = (CPSS_DXCH_BRG_L2_DLB_MAX_PATH)/numOfPaths;
    weight = 0;
    for(i = 0 ; i < numOfPaths/4 ; i++)
    {
        for( j =0 ; j < 4 ; j++)
        {
            weight   = weight + dlbEntryArr[4*i+j].weight;
            tmpValue = (GT_FLOAT32)((1.0*weight *1024)/100);
            hwWeight = (GT_U32)(0.5 + tmpValue);
            U32_SET_FIELD_IN_ENTRY_MAC(hwData,0 + j*34, 11, hwWeight);

            if((dlbEntryArr[4*i+j].targetHwDevice > PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_HW_DEV_NUM_MAC(devNum)) ||
               (dlbEntryArr[4*i+j].targetEport    > PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_E_PORT_MAC(devNum)))
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
            U32_SET_FIELD_IN_ENTRY_MAC(hwData,11+ j*34, 10, dlbEntryArr[4*i+j].targetHwDevice);
            U32_SET_FIELD_IN_ENTRY_MAC(hwData,21 +j*34, 13, dlbEntryArr[4*i+j].targetEport);
        }

        rc = prvCpssDxChWriteTableEntry(devNum,
             pathUtilizationTable[tableId + i*offset],
             index,
             hwData);

        if( rc != GT_OK)
        {
            return rc;
        }
    }
    return rc;
}

/**
* @internal cpssDxChBrgL2DlbPathUtilizationTableEntrySet function
* @endinternal
*
* @brief  Configure the Path Utilization Table entry(Target ePort/Device, Weight).
*         Path weight is assigned in proportion to the utilizable percentage of bandwidth
*         i.e 100% utilization percentage.
*                * Weight 100 equals full BW available
*                * Weight 50 is assigned for 50% available BM
*                * Weight 25 is assigned for available 25% BW
*                * Weight 0 is assigned for path which has no BW.
*         Relative weight is normalized to span the range [0,1024]
*         such that the sum of all the path weights is equal '1024'
*
* @note   APPLICABLE DEVICES:     Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*                                 xCat3; AC5; Lion2.
*
* @param[in] devNum                - device number
* @param[in] index                 - the DLB entry Index.
* @param[in] dlbEntryArr           - DLB path utilization table entry array.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_OUT_OF_RANGE          - parameter out of range.
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgL2DlbPathUtilizationTableEntrySet
(
    IN GT_U8                               devNum,
    IN GT_U32                              index,
    IN CPSS_DXCH_BRG_L2_DLB_PATH_ENTRY_STC dlbEntryArr[CPSS_DXCH_BRG_L2_DLB_MAX_PATH]
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgL2DlbPathUtilizationTableEntrySet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, index, dlbEntryArr));

    rc = internal_cpssDxChBrgL2DlbPathUtilizationTableEntrySet(devNum, index, dlbEntryArr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, index, dlbEntryArr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    return rc;
}

/**
* @internal internal_cpssDxChBrgL2DlbPathUtilizationTableEntryGet function
* @endinternal
*
* @brief  Get the Path Utilization Table entry(Target ePort/Device, Weight).
*         Path weight is assigned in proportion to the utilizable percentage of bandwidth
*         i.e 100% utilization percentage.
*                * Weight 100 equals full BW available
*                * Weight 50 is assigned for 50% available BM
*                * Weight 25 is assigned for available 25% BW
*                * Weight 0 is assigned for path which has no BW.
*         Relative weight is normalized to span the range [0,1024]
*         such that the sum of all the path weights is equal '1024'
*
* @note   APPLICABLE DEVICES:     Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*                                 xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] index                    - the DLB entry Index.
* @param[out] dlbEntryArr             - (pointer to) DLB path utilization table entry array.
*                                       Array indexes are used based on path utilization scale mode as below.
*                                       512 x 16 path  - 16 indexes are used
*                                                 |table3|table2|table1|table0|
*                                       1K x 8 path    - Only first 8 indexes are used.
*                                                   |table2|table0|
*                                                   |table3|table1|
*                                       2K x 4 path    - Only first 4 indexes are used
*                                                   |table0|
*                                                   |table1|
*                                                   |table2|
*                                                   |table3|
* @retval GT_OK                       - on success.
* @retval GT_BAD_PARAM                - on wrong input parameters
* @retval GT_HW_ERROR                 - failed to write to hardware
* @retval GT_BAD_PTR                  - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE    - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgL2DlbPathUtilizationTableEntryGet
(
    IN GT_U8                                devNum,
    IN GT_U32                               index,
    OUT CPSS_DXCH_BRG_L2_DLB_PATH_ENTRY_STC dlbEntryArr[CPSS_DXCH_BRG_L2_DLB_MAX_PATH]
)
{
    GT_STATUS  rc;
    GT_U32     hwData[5] = {0,0,0,0,0};
    GT_U32     i,j;
    GT_U32     weight = 0;
    GT_U32     numOfPaths, maxTarget;
    GT_U32     offset, tableId;
    GT_FLOAT32 tmpValue;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
          CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_XCAT3_E | CPSS_AC5_E | CPSS_BOBCAT3_E | CPSS_CAELUM_E |
          CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_ALDRIN2_E);
     CPSS_NULL_PTR_CHECK_MAC(dlbEntryArr);

    rc = dlbPathTargetGet(devNum, &numOfPaths, &maxTarget);
    if( rc != GT_OK)
    {
        return rc;
    }

    /* max supported target is 2K */
    DLB_PATH_UTILIZATION_TABLE_INDEX_CHECK(index, maxTarget);

    tableId = (index / 512) ;

    /* 512 entries per table */
    index  = index%512;
    offset = (CPSS_DXCH_BRG_L2_DLB_MAX_PATH)/numOfPaths;

    for(i = 0 ; i < numOfPaths/4 ; i++)
    {
        rc = prvCpssDxChReadTableEntry(devNum,
             pathUtilizationTable[tableId + i*offset],
             index,
             hwData);

        if(rc != GT_OK)
        {
            return rc;
        }

        for( j =0 ; j < 4 ; j++)
        {
            U32_GET_FIELD_IN_ENTRY_MAC(hwData,0 + j*34, 11, weight);
            U32_GET_FIELD_IN_ENTRY_MAC(hwData,11+ j*34, 10, dlbEntryArr[4*i+j].targetHwDevice);
            U32_GET_FIELD_IN_ENTRY_MAC(hwData,21 +j*34, 13, dlbEntryArr[4*i+j].targetEport);
            tmpValue = (GT_FLOAT32)((1.0*weight*100)/1024);
            dlbEntryArr[4*i+j].weight   = (GT_U32)(0.5 + tmpValue);
        }
    }
    return rc;
}

/**
* @internal cpssDxChBrgL2DlbPathUtilizationTableEntryGet function
* @endinternal
*
* @brief  Get the Path Utilization Table entry(Target ePort/Device, Weight).
*         Path weight is assigned in proportion to the utilizable percentage of bandwidth
*         i.e 100% utilization percentage.
*                * Weight 100 equals full BW available
*                * Weight 50 is assigned for 50% available BM
*                * Weight 25 is assigned for available 25% BW
*                * Weight 0 is assigned for path which has no BW.
*         Relative weight is normalized to span the range [0,1024]
*         such that the sum of all the path weights is equal '1024'
*
* @note   APPLICABLE DEVICES:     Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*                                 xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] index                    - the DLB entry Index.
* @param[out] dlbEntryArr             - (pointer to) DLB path utilization table entry array.
*
* @retval GT_OK                       - on success.
* @retval GT_BAD_PARAM                - on wrong input parameters
* @retval GT_HW_ERROR                 - failed to write to hardware
* @retval GT_BAD_PTR                  - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE    - on not applicable device
*/
GT_STATUS cpssDxChBrgL2DlbPathUtilizationTableEntryGet
(
    IN GT_U8                                devNum,
    IN GT_U32                               index,
    OUT CPSS_DXCH_BRG_L2_DLB_PATH_ENTRY_STC dlbEntryArr[CPSS_DXCH_BRG_L2_DLB_MAX_PATH]
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgL2DlbPathUtilizationTableEntryGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, index, dlbEntryArr));

    rc = internal_cpssDxChBrgL2DlbPathUtilizationTableEntryGet(devNum, index, dlbEntryArr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, index, dlbEntryArr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    return rc;
}

/**
* @internal prvCpssDxChBrgL2DlbWaInit function
* @endinternal
*
* @brief  Allocates the required data structure.
*         Configures the maximum granularity for the DLB clock.
*
* @note   APPLICABLE DEVICES:     Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES: xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                - device number
* @param[in] secondaryLttBaseIndex - Base index to the secondary region of LTT table.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChBrgL2DlbWaInit
(
    IN GT_U8                               devNum,
    IN GT_U32                              secondaryLttBaseIndex
)
{
    GT_STATUS                                   rc;
    GT_U32                                      regAddr;
    GT_U32                                      hwData;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
          CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_XCAT3_E | CPSS_AC5_E | CPSS_BOBCAT3_E | CPSS_CAELUM_E |
          CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_ALDRIN2_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E);

    if(PRV_DLB_LTT_WA_ERRTA_MAC(devNum).dlbWaEnable == GT_TRUE)
    {
        return GT_OK;
    }

    /* Update the primary region base index */
    rc = internal_cpssDxChBrgL2DlbLttIndexBaseEportGet(devNum, &hwData);
    if(rc != GT_OK)
    {
        return rc;
    }
    PRV_DLB_LTT_WA_ERRTA_MAC(devNum).primaryLttBaseIndex = hwData;

    /* AAC global control */
    regAddr = PRV_DXCH_REG1_UNIT_SIP6_CNM_AAC_MAC(devNum).AACControl;
    rc = prvCpssHwPpSetRegField(devNum, regAddr, 0, 1, 1);
    if(rc != GT_OK)
    {
        return rc;
    }

    /*  AAC Engine <<%n>> Control for channel - AAC address mode - indirect */
    regAddr = PRV_DXCH_REG1_UNIT_SIP6_CNM_AAC_MAC(devNum).AACEngineControl[PRV_CPSS_DXCH_AAC_CHANNEL_DLB_AND_PHA_E];
    rc = prvCpssHwPpSetRegField(devNum, regAddr, 0, 1, 1);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* Increase the incremental time factor to 10 micro sec (To prevent the wrap around till 5.22sec) */
    regAddr = PRV_DXCH_REG1_UNIT_EQ_MAC(devNum).L2ECMP.globalEportDLBConfig;
    rc = prvCpssHwPpSetRegField(devNum, regAddr, 4, 8, (PRV_DLB_LTT_REF_CLK_CNS * 10));
    if(rc != GT_OK)
    {
        return rc;
    }

    PRV_DLB_LTT_WA_ERRTA_MAC(devNum).dlbWaEnable            = GT_TRUE;
    PRV_DLB_LTT_WA_ERRTA_MAC(devNum).secondaryLttBaseIndex  = secondaryLttBaseIndex;
    return GT_OK;
}

/**
* @internal internal_cpssDxChBrgL2DlbWaLttInfoSet function
* @endinternal
*
* @brief  Updates LTT entry with base index of secondary region.
*
* @note   APPLICABLE DEVICES:     Falcon.
* @note   NOT APPLICABLE DEVICES: xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                - device number
* @param[in] lttIndex              - LTT Index.
* @param[in] secondaryDlbBaseIndex - Base index to the secondary region of DLB table
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgL2DlbWaLttInfoSet
(
    IN GT_U8                                devNum,
    IN GT_U32                               lttIndex,
    IN GT_U32                               secondaryDlbBaseIndex
)
{
    GT_STATUS                                   rc;
    CPSS_DXCH_BRG_L2_ECMP_LTT_ENTRY_STC         ecmpLttInfo;
    PRV_CPSS_DXCH_BRG_L2_DLB_WA_LTT_USAGE_STC   *dlbWaUsageNodePtr;
    PRV_CPSS_DXCH_BRG_L2_DLB_WA_LTT_USAGE_STC   *dlbWaUsageLastNodePtr;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
          CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_XCAT3_E | CPSS_AC5_E | CPSS_BOBCAT3_E | CPSS_CAELUM_E |
          CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_ALDRIN2_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E);

    /* Input validation check */
    if((secondaryDlbBaseIndex > PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_E_PORT_MAC(devNum)) ||
       ((PRV_DLB_LTT_PRIMARY_BLOCK_SIZE_MAC(devNum) + lttIndex) > PRV_CPSS_DXCH_PP_HW_INFO_LAST_INDEX_IN_L2_LTT_ECMP_MAC(devNum)))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* Get ECMP LTT table entry from primary region */
    rc = cpssDxChBrgL2EcmpLttTableGet(devNum,
            lttIndex,
            &ecmpLttInfo);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* Configure L2ECMP  LTT table secondary region */
    ecmpLttInfo.ecmpStartIndex = secondaryDlbBaseIndex;
    rc = cpssDxChBrgL2EcmpLttTableSet(devNum,
            PRV_DLB_LTT_PRIMARY_BLOCK_SIZE_MAC(devNum) + lttIndex,
            &ecmpLttInfo);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* Update secondaryDlbBaseIndex for the LTT Entry in CPSS DB */
    dlbWaUsageNodePtr     = PRV_DLB_LTT_WA_ERRTA_MAC(devNum).headOfDlbWaUsageList;
    dlbWaUsageLastNodePtr = dlbWaUsageNodePtr;
    while(dlbWaUsageNodePtr)
    {
        /* Update existing list */
        if(dlbWaUsageNodePtr->lttIndex == lttIndex)
        {
            dlbWaUsageNodePtr->secondaryDlbBaseIndex = secondaryDlbBaseIndex;
            return GT_OK;
        }
        dlbWaUsageLastNodePtr = dlbWaUsageNodePtr;
        dlbWaUsageNodePtr     = dlbWaUsageNodePtr->nextPointer;
    }

    /* New Entry (Last entry to the list) */
    if((dlbWaUsageLastNodePtr != NULL) && (dlbWaUsageLastNodePtr->nextPointer != NULL))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    dlbWaUsageNodePtr = cpssOsMalloc(sizeof(PRV_CPSS_DXCH_BRG_L2_DLB_WA_LTT_USAGE_STC));
    if(dlbWaUsageNodePtr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM,
                "Memory allocation for L2 DLB WA Failed, devNum[%d]", devNum);
    }
    dlbWaUsageNodePtr->lttIndex              = lttIndex;
    dlbWaUsageNodePtr->secondaryDlbBaseIndex = secondaryDlbBaseIndex;
    dlbWaUsageNodePtr->nextPointer           = NULL;

    /* Add New node as Head */
    if(dlbWaUsageLastNodePtr == NULL)
    {
        PRV_DLB_LTT_WA_ERRTA_MAC(devNum).headOfDlbWaUsageList = dlbWaUsageNodePtr;
    }
    /* Add New node at the end */
    else
    {
        dlbWaUsageLastNodePtr->nextPointer = dlbWaUsageNodePtr;
    }
    return GT_OK;
}

/**
* @internal cpssDxChBrgL2DlbWaLttInfoSet function
* @endinternal
*
* @brief  Updates LTT entry with base index of secondary region.
*
* @note   APPLICABLE DEVICES:     Falcon.
* @note   NOT APPLICABLE DEVICES: xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                - device number
* @param[in] lttIndex              - LTT Index.
* @param[in] secondaryDlbBaseIndex - Base index to the secondary region of DLB table
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgL2DlbWaLttInfoSet
(
    IN GT_U8                                devNum,
    IN GT_U32                               lttIndex,
    IN GT_U32                               secondaryDlbBaseIndex
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgL2DlbWaLttInfoSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, lttIndex, secondaryDlbBaseIndex));

    rc = internal_cpssDxChBrgL2DlbWaLttInfoSet(devNum, lttIndex, secondaryDlbBaseIndex);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, lttIndex, secondaryDlbBaseIndex));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    return rc;
}

/**
* @internal internal_cpssDxChBrgL2DlbWaExecute function
* @endinternal
*
* @brief  Performs DLB WA.
*
* @note   APPLICABLE DEVICES:     Falcon.
* @note   NOT APPLICABLE DEVICES: xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                - device number
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgL2DlbWaExecute
(
    IN GT_U8                                devNum
)
{
    GT_STATUS                                   rc;
    GT_U32                                      currIpg;         /* Restore IPG at END */
    GT_U32                                      timeFactor;      /* Restore timeFactor at END */
    GT_U32                                      i;
    CPSS_DXCH_BRG_L2_DLB_ENTRY_STC              dlbEntry;
    CPSS_DXCH_BRG_L2_ECMP_LTT_ENTRY_STC         ecmpLttInfo_primary, ecmpLttInfo_secondary;
    PRV_CPSS_DXCH_BRG_L2_DLB_WA_LTT_USAGE_STC   *dlbWaUsageNodePtr;
    GT_U32                                      regAddr;
    GT_U32                                      hwData;
    GT_U32                                      wrrArbiterWeight;/* Restore WrrArbiterWeight at END */
    GT_U32                                      portGroupId;
    GT_U32                                      offsetAdd;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
          CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_XCAT3_E | CPSS_AC5_E | CPSS_BOBCAT3_E | CPSS_CAELUM_E |
          CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_ALDRIN2_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E);

    /* Return if WA is disabled, or no DLB entry to work on */
    dlbWaUsageNodePtr = PRV_DLB_LTT_WA_ERRTA_MAC(devNum).headOfDlbWaUsageList;
    if((PRV_DLB_LTT_WA_ERRTA_MAC(devNum).dlbWaEnable  == GT_FALSE) ||
            (dlbWaUsageNodePtr == NULL))
    {
        return GT_OK;
    }

    /* Stop timer (Reset incremental factor [4:11]bits to 0) */
    regAddr = PRV_DXCH_REG1_UNIT_EQ_MAC(devNum).L2ECMP.globalEportDLBConfig;
    rc = prvCpssHwPpGetRegField(devNum, regAddr, 4, 8, &timeFactor);
    if(rc != GT_OK)
    {
        return rc;
    }
    rc = prvCpssHwPpSetRegField(devNum, regAddr, 4, 8, 0/* Reset to 0 */);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* Prevent re-balancing by setting IPG to MAX value,
     * keep current IPG for restore at end */
    rc = internal_cpssDxChBrgL2DlbMinimumFlowletIpgGet(devNum, &currIpg);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* WaInit changed the DLB clock freq to 10 micro sec, so multiplying the counter */
    rc = internal_cpssDxChBrgL2DlbMinimumFlowletIpgSet(devNum, (0x7FFFF * 10));
    if(rc != GT_OK)
    {
        return rc;
    }

    /* Copy DLB table entries from primary to secondary region */
    while(dlbWaUsageNodePtr)
    {
        /* Get the L2 ECMP LTT entry - from primary region(Reading region) */
        rc = cpssDxChBrgL2EcmpLttTableGet(devNum,
                PRV_DLB_LTT_PRIMARY_SHIFT(devNum) + dlbWaUsageNodePtr->lttIndex,
                &ecmpLttInfo_primary);
        if(rc != GT_OK)
        {
            return rc;
        }

        /* Get the L2 ECMP LTT entry - from secondary region(Writing region) */
        rc = cpssDxChBrgL2EcmpLttTableGet(devNum,
                PRV_DLB_LTT_SECONDARY_SHIFT(devNum) + dlbWaUsageNodePtr->lttIndex,
                &ecmpLttInfo_secondary);
        if(rc != GT_OK)
        {
            return rc;
        }

        /* Clear lastSeenTimeStamp for all ECMP entry and write back to secondary region */
        for(i = 0; i < ecmpLttInfo_primary.ecmpNumOfPaths; i++)
        {
            /* Read from the primary region */
            rc = internal_cpssDxChBrgL2DlbTableEntryGet(devNum,
                    ecmpLttInfo_primary.ecmpStartIndex+i,
                    &dlbEntry);
            if(rc != GT_OK)
            {
                return rc;
            }
            dlbEntry.lastSeenTimeStamp = 0;
            /* Write back to secondary block(Last iteration's secondary, current iteration primary) */
            internal_cpssDxChBrgL2DlbTableEntrySet(devNum,
                    ecmpLttInfo_secondary.ecmpStartIndex+i,
                    &dlbEntry);
            if(rc != GT_OK)
            {
                return rc;
            }
        }
        dlbWaUsageNodePtr = dlbWaUsageNodePtr->nextPointer;
    }

    /* Next 2 Steps, must performed very fast, to prevent with high probability re-order.
     * 1. Clear current time
     * 2. Change base of DLB region
     */
    PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_MAC(devNum,portGroupId)
    {
        offsetAdd = FALCON_CONTROL_PIPE_OFFSET_CNS*(portGroupId&1) + FALCON_TILE_OFFSET_CNS*((portGroupId>>1)&3);

        /* BackUp wrrArbiterWeight to restore */
        regAddr = PRV_DXCH_REG1_UNIT_EQ_MAC(devNum).preEgrEngineGlobalConfig.incomingWrrArbiterWeights + offsetAdd;
        rc = prvCpssDrvHwPpReadRegister(devNum, regAddr, &wrrArbiterWeight);
        if(rc != GT_OK)
        {
            return rc;
        }

        /* Check that previous access done by busy-wait polling before trigger next access. */
        regAddr = PRV_DXCH_REG1_UNIT_SIP6_CNM_AAC_MAC(devNum).AACEnginesStatus;
        rc = prvCpssPortGroupBusyWait(devNum, portGroupId, regAddr,
                PRV_CPSS_DXCH_AAC_CHANNEL_DLB_AND_PHA_E, GT_FALSE/*busyWait*/);
        if(rc != GT_OK)
        {
            return rc;
        }

        /* AAC Channel counter reset */
        regAddr = PRV_DXCH_REG1_UNIT_SIP6_CNM_AAC_MAC(devNum).AACEngineControl[PRV_CPSS_DXCH_AAC_CHANNEL_DLB_AND_PHA_E];
        rc = prvCpssDrvHwPpWriteRegister(devNum, regAddr, 0xF3);
        if(rc != GT_OK)
        {
            return rc;
        }

        /* AAC channel direct mode */
        rc = prvCpssDrvHwPpWriteRegister(devNum, regAddr, 0xF1);
        if(rc != GT_OK)
        {
            return rc;
        }

        /* Data register for AAC DLB Channel */
        regAddr = PRV_DXCH_REG1_UNIT_SIP6_CNM_AAC_MAC(devNum).AACEngineData[PRV_CPSS_DXCH_AAC_CHANNEL_DLB_AND_PHA_E];

        /* Prevent packet processing from Queue - setting WrrArbiterWeights to 0(Ingress pipe weight[0:4]) */
        hwData  = PRV_DXCH_REG1_UNIT_EQ_MAC(devNum).preEgrEngineGlobalConfig.incomingWrrArbiterWeights + offsetAdd;
        rc = prvCpssDrvHwPpWriteRegister(devNum, regAddr, hwData);            /* address */
        if(rc != GT_OK)
        {
            return rc;
        }
        rc = prvCpssDrvHwPpWriteRegister(devNum, regAddr, (wrrArbiterWeight | 0xFFFFFFE0));/* data */
        if(rc != GT_OK)
        {
            return rc;
        }

        /* Set the primary region with the base index, which was "last time secondary" */
        hwData  = PRV_DXCH_REG1_UNIT_EQ_MAC(devNum).L2ECMP.dlbLTTIndexBaseEport + offsetAdd;
        rc = prvCpssDrvHwPpWriteRegister(devNum, regAddr, hwData);            /* address */
        if(rc != GT_OK)
        {
            return rc;
        }
        hwData  = PRV_DLB_LTT_LAST_SECONDARY_BASE_MAC(devNum);
        rc = prvCpssDrvHwPpWriteRegister(devNum, regAddr, hwData);            /* data */
        if(rc != GT_OK)
        {
            return rc;
        }

        /* Clear the current time register */
        hwData  = PRV_DXCH_REG1_UNIT_EQ_MAC(devNum).L2ECMP.ePortDLBCurrentTime + offsetAdd;
        rc = prvCpssDrvHwPpWriteRegister(devNum, regAddr, hwData);            /* address */
        if(rc != GT_OK)
        {
            return rc;
        }
        rc = prvCpssDrvHwPpWriteRegister(devNum, regAddr, 0x0);               /* data */
        if(rc != GT_OK)
        {
            return rc;
        }

        /* Revert back the WrrArbiterWeights (Ingress pipe weight[0:4] - default 0x1F) */
        hwData  = PRV_DXCH_REG1_UNIT_EQ_MAC(devNum).preEgrEngineGlobalConfig.incomingWrrArbiterWeights + offsetAdd;
        rc = prvCpssDrvHwPpWriteRegister(devNum, regAddr, hwData);           /* address */
        if(rc != GT_OK)
        {
            return rc;
        }
        rc = prvCpssDrvHwPpWriteRegister(devNum, regAddr, wrrArbiterWeight); /* data */
        if(rc != GT_OK)
        {
            return rc;
        }

        /* Address register for AAC DLB Channel - AAC Trigger */
        regAddr = PRV_DXCH_REG1_UNIT_SIP6_CNM_AAC_MAC(devNum).AACEngineAddress[PRV_CPSS_DXCH_AAC_CHANNEL_DLB_AND_PHA_E];
        rc = prvCpssDrvHwPpWriteRegister(devNum, regAddr, 0x0);              /* trigger */
        if(rc != GT_OK)
        {
            return rc;
        }
    }
    PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_MAC(devNum,portGroupId)

    /* Restore back IPG */
    rc = internal_cpssDxChBrgL2DlbMinimumFlowletIpgSet(devNum, currIpg);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* Restore back timer incremental factor */
    regAddr = PRV_DXCH_REG1_UNIT_EQ_MAC(devNum).L2ECMP.globalEportDLBConfig;
    rc = prvCpssHwPpSetRegField(devNum, regAddr, 4, 8, timeFactor);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* Update the block in use for next WA call */
    PRV_DLB_LTT_WA_ERRTA_MAC(devNum).blockInUse ^= 1;
    return rc;
}

/**
* @internal cpssDxChBrgL2DlbWaExecute function
* @endinternal
*
* @brief  Performs DLB WA.
*
* @note   APPLICABLE DEVICES:     Falcon.
* @note   NOT APPLICABLE DEVICES: xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                - device number
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgL2DlbWaExecute
(
    IN GT_U8                                devNum
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgL2DlbWaExecute);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum));

    rc = internal_cpssDxChBrgL2DlbWaExecute(devNum);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    return rc;
}
