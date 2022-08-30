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
* @file cpssDxChStream.c
*
* @brief CPSS DxCh Stream configuration API
*   NOTE: some of the function or fields are HSR/PRP some are for the 802.1cb
*
* @version   1
********************************************************************************
*/
#define CPSS_LOG_IN_MODULE_ENABLE

#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/stream/private/prvCpssDxChStreamLog.h>
#include <cpss/dxCh/dxChxGen/stream/cpssDxChStream.h>

#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

/* MACRO to check the SMU_SNG_IRF index */
#define SMU_SNG_IRF_INDEX_CHECK_MAC(_devNum,_index)                                                          \
    if(_index >= PRV_CPSS_DXCH_PP_MAC(_devNum)->hwInfo.parametericTables.numEntriesStream ||                 \
       _index >= STREAM_SNG_NUM_CNS)                                                                         \
    {                                                                                                        \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Stream index [0x%4.4x] is not in range of [0x0..0x%4.4x]", \
            _index,                                                                                          \
            PRV_CPSS_DXCH_PP_MAC(_devNum)->hwInfo.parametericTables.numEntriesStream-1);                     \
    }

/* MACRO to check the SRF index */
#define SMU_SRF_INDEX_CHECK_MAC(_devNum,_index)                                                              \
    if(_index >= PRV_CPSS_DXCH_PP_MAC(_devNum)->hwInfo.parametericTables.numEntriesStreamSrf ||              \
       _index >= STREAM_PREQ_SRF_NUM)                                                                        \
    {                                                                                                        \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "SRF index [0x%4.4x] is not in range of [0x0..0x%4.4x]", \
            _index,                                                                                          \
            PRV_CPSS_DXCH_PP_MAC(_devNum)->hwInfo.parametericTables.numEntriesStreamSrf-1);                  \
    }

/* MACRO to check the SRF SRF History buffer index */
#define SMU_SRF_BUFFER_INDEX_CHECK_MAC(_devNum,_index)                                                       \
    if(_index >= PRV_CPSS_DXCH_PP_MAC(_devNum)->hwInfo.parametericTables.numEntriesStreamSrfHist)            \
    {                                                                                                        \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "SRF buffer index [0x%4.4x] is not in range of [0x0..0x%4.4x]", \
            _index,                                                                                          \
            PRV_CPSS_DXCH_PP_MAC(_devNum)->hwInfo.parametericTables.numEntriesStreamSrfHist-1);              \
    }

/* MACRO to check the SRF zero bit vector index */
#define SMU_SRF_ZER_BIT_VECTOR_INDEX_CHECK_MAC(_devNum,_index)                              \
    SMU_SRF_BUFFER_INDEX_CHECK_MAC(_devNum,_index);                                         \
    if((_index) & 0x1f)                                                                     \
    {                                                                                       \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,                                         \
            "SRF buffer zero bit vector [0x%4.4x] must be in steps of 32 (0,32,64...(%d))", \
            startIndex,                                                                     \
            PRV_CPSS_DXCH_PP_MAC(_devNum)->hwInfo.parametericTables.numEntriesStreamSrfHist - 32); \
    }

/**
* @internal internal_cpssDxChStreamSngEnableSet function
* @endinternal
*
* @brief  Set Enable/Disable the SNG (Sequence Number Generation).
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum         - device number.
* @param[in] enable         - Indication to Enable/Disable the SNG.
*                             GT_TRUE  - Enable.
*                             GT_FALSE - Disable.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
static GT_STATUS internal_cpssDxChStreamSngEnableSet
(
    IN  GT_U8                           devNum,
    IN  GT_BOOL                         enable
)
{
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E |
        CPSS_BOBCAT2_E | CPSS_BOBCAT3_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E |
        CPSS_ALDRIN2_E | CPSS_FALCON_E |  CPSS_AC5P_E |  CPSS_AC5X_E | CPSS_HARRIER_E);

    /* set <Sequence Number Generation Enable> in SMU Unit */
    return prvCpssHwPpSetRegField(devNum,
        PRV_DXCH_REG1_UNIT_SMU_MAC(devNum).irfSng.irfSngGlobalConfig,
        0,1,BOOL2BIT_MAC(enable));
}

/**
* @internal cpssDxChStreamSngEnableSet function
* @endinternal
*
* @brief  Set Enable/Disable the SNG (Sequence Number Generation).
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum         - device number.
* @param[in] enable         - Indication to Enable/Disable the SNG.
*                             GT_TRUE  - Enable.
*                             GT_FALSE - Disable.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChStreamSngEnableSet
(
    IN  GT_U8                           devNum,
    IN  GT_BOOL                         enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChStreamSngEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, enable));

    rc = internal_cpssDxChStreamSngEnableSet(devNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChStreamSngEnableGet function
* @endinternal
*
* @brief  Get the Enable/Disable of the SNG (Sequence Number Generation).
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum         - device number.
*
* @param[out] enablePtr     - (pointer to) Indication to Enable/Disable the SNG.
*                             GT_TRUE  - Enable.
*                             GT_FALSE - Disable.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
static GT_STATUS internal_cpssDxChStreamSngEnableGet
(
    IN  GT_U8                           devNum,
    OUT GT_BOOL                         *enablePtr
)
{
    GT_STATUS   rc;      /* return code */
    GT_U32      hwValue; /* value read from HW */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E |
        CPSS_BOBCAT2_E | CPSS_BOBCAT3_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E |
        CPSS_ALDRIN2_E | CPSS_FALCON_E |  CPSS_AC5P_E |  CPSS_AC5X_E | CPSS_HARRIER_E);
    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    /* set <Sequence Number Generation Enable> in SMU Unit */
    rc = prvCpssHwPpGetRegField(devNum,
        PRV_DXCH_REG1_UNIT_SMU_MAC(devNum).irfSng.irfSngGlobalConfig,
        0,1,&hwValue);

    *enablePtr = BIT2BOOL_MAC(hwValue);

    return rc;
}

/**
* @internal cpssDxChStreamSngEnableGet function
* @endinternal
*
* @brief  Get the Enable/Disable of the SNG (Sequence Number Generation).
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum         - device number.
*
* @param[out] enablePtr     - (pointer to) Indication to Enable/Disable the SNG.
*                             GT_TRUE  - Enable.
*                             GT_FALSE - Disable.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChStreamSngEnableGet
(
    IN  GT_U8                           devNum,
    OUT GT_BOOL                         *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChStreamSngEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, enablePtr));

    rc = internal_cpssDxChStreamSngEnableGet(devNum, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal cpssDxChStreamIrfEnableSet function
* @endinternal
*
* @brief  Set Enable/Disable the IRF (Individual Recovery Function).
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum         - device number.
* @param[in] enable         - Indication to Enable/Disable the IRF.
*                             GT_TRUE  - Enable.
*                             GT_FALSE - Disable.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
static GT_STATUS internal_cpssDxChStreamIrfEnableSet
(
    IN  GT_U8                           devNum,
    IN  GT_BOOL                         enable
)
{
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E |
        CPSS_BOBCAT2_E | CPSS_BOBCAT3_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E |
        CPSS_ALDRIN2_E | CPSS_FALCON_E |  CPSS_AC5P_E |  CPSS_AC5X_E | CPSS_HARRIER_E);

    /* set <Individual Recovery Function Global Enable> in SMU Unit */
    return prvCpssHwPpSetRegField(devNum,
        PRV_DXCH_REG1_UNIT_SMU_MAC(devNum).irfSng.irfSngGlobalConfig,
        1,1,BOOL2BIT_MAC(enable));
}

/**
* @internal cpssDxChStreamIrfEnableSet function
* @endinternal
*
* @brief  Set Enable/Disable the IRF (Individual Recovery Function).
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum         - device number.
* @param[in] enable         - Indication to Enable/Disable the IRF.
*                             GT_TRUE  - Enable.
*                             GT_FALSE - Disable.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChStreamIrfEnableSet
(
    IN  GT_U8                           devNum,
    IN  GT_BOOL                         enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChStreamIrfEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, enable));

    rc = internal_cpssDxChStreamIrfEnableSet(devNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChStreamIrfEnableGet function
* @endinternal
*
* @brief  Get the Enable/Disable of the IRF (Individual Recovery Function).
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum         - device number.
*
* @param[out] enablePtr     - (pointer to) Indication to Enable/Disable the IRF.
*                             GT_TRUE  - Enable.
*                             GT_FALSE - Disable.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
static GT_STATUS internal_cpssDxChStreamIrfEnableGet
(
    IN  GT_U8                           devNum,
    OUT GT_BOOL                         *enablePtr
)
{
    GT_STATUS   rc;      /* return code */
    GT_U32      hwValue; /* value read from HW */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E |
        CPSS_BOBCAT2_E | CPSS_BOBCAT3_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E |
        CPSS_ALDRIN2_E | CPSS_FALCON_E |  CPSS_AC5P_E |  CPSS_AC5X_E | CPSS_HARRIER_E);
    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    /* set <Individual Recovery Function Global Enable> in SMU Unit */
    rc = prvCpssHwPpGetRegField(devNum,
        PRV_DXCH_REG1_UNIT_SMU_MAC(devNum).irfSng.irfSngGlobalConfig,
        1,1,&hwValue);

    *enablePtr = BIT2BOOL_MAC(hwValue);

    return rc;
}


/**
* @internal cpssDxChStreamIrfEnableGet function
* @endinternal
*
* @brief  Get the Enable/Disable of the IRF (Individual Recovery Function).
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum         - device number.
*
* @param[out] enablePtr     - (pointer to) Indication to Enable/Disable the IRF.
*                             GT_TRUE  - Enable.
*                             GT_FALSE - Disable.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChStreamIrfEnableGet
(
    IN  GT_U8                           devNum,
    OUT GT_BOOL                         *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChStreamIrfEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, enablePtr));

    rc = internal_cpssDxChStreamIrfEnableGet(devNum, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal exceptionRegInfoGet function
* @endinternal
*
* @brief  Get the register address and start bit to specific exception type.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum         - device number.
* @param[in] type           - The type of exception.
*
* @param[out] command_regAddrPtr        - (pointer to) the packet command register address.
*                                           if NULL ignored
* @param[out] command_startBitPtr       - (pointer to) the packet command start bit.
*                                           if NULL ignored
* @param[out] code_regAddrPtr           - (pointer to) the cpu code register address.
*                                           if NULL ignored
* @param[out] code_startBitPtr          - (pointer to) the cpu code start bit.
*                                           if NULL ignored
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong type
*
*/
static GT_STATUS exceptionRegInfoGet(
    IN GT_U8                                    devNum,
    IN CPSS_DXCH_STREAM_EXCEPTION_TYPE_ENT     type,
    OUT GT_U32      *command_regAddrPtr,
    OUT GT_U32      *command_startBitPtr,
    OUT GT_U32      *code_regAddrPtr,
    OUT GT_U32      *code_startBitPtr
)
{
    GT_U32      regAddr1  = GT_NA;
    GT_U32      startBit1 = 0;
    GT_U32      regAddr2  = GT_NA;
    GT_U32      startBit2 = 0;

    switch(type)
    {
        case CPSS_DXCH_STREAM_EXCEPTION_TYPE_IRF_SAME_SEQ_ID_E:
            regAddr1  = PRV_DXCH_REG1_UNIT_SMU_MAC(devNum).irfSng.irfSngGlobalConfig;
            startBit1 = 2;/*command*/
            regAddr2  = regAddr1;
            startBit2 = 5;/*code*/
            break;
        case CPSS_DXCH_STREAM_EXCEPTION_TYPE_SRF_TAGLESS_E:
            regAddr1  = PRV_DXCH_REG1_UNIT_PREQ_MAC(devNum).FRE.freSrfGlobalConfig1;
            startBit1 = 0;/*command*/
            regAddr2  = regAddr1;
            startBit2 = 3;/*code*/
            break;
        case CPSS_DXCH_STREAM_EXCEPTION_TYPE_SRF_ROGUE_E:
            regAddr1  = PRV_DXCH_REG1_UNIT_PREQ_MAC(devNum).FRE.freSrfGlobalConfig0;
            startBit1 = 11;/*command*/
            regAddr2  = regAddr1;
            startBit2 = 14;/*code*/
            break;
        case CPSS_DXCH_STREAM_EXCEPTION_TYPE_SRF_DUPLICATED_E:
            regAddr1  = PRV_DXCH_REG1_UNIT_PREQ_MAC(devNum).FRE.freSrfGlobalConfig0;
            startBit1 = 0;/*command*/
            regAddr2  = regAddr1;
            startBit2 = 3;/*code*/
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(type);
    }

    if(command_regAddrPtr)
    {
        *command_regAddrPtr  = regAddr1;
        *command_startBitPtr = startBit1;
    }

    if(code_regAddrPtr)
    {
        *code_regAddrPtr  = regAddr2;
        *code_startBitPtr = startBit2;
    }

    return GT_OK;
}

/**
* @internal internal_cpssDxChStreamExceptionCommandSet function
* @endinternal
*
* @brief  Set the packet command to specific exception type.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum         - device number.
* @param[in] type           - The type of exception.
* @param[in] command        - the packet command.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum/type/command
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
static GT_STATUS internal_cpssDxChStreamExceptionCommandSet
(
    IN GT_U8                                    devNum,
    IN CPSS_DXCH_STREAM_EXCEPTION_TYPE_ENT      type,
    IN CPSS_PACKET_CMD_ENT                      command
)
{
    GT_STATUS   rc;      /* return code */
    GT_U32      regAddr; /* register address */
    GT_U32      startBit;/* start bit in the register */
    GT_U32      hwValue; /* value to write to HW */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E |
        CPSS_BOBCAT2_E | CPSS_BOBCAT3_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E |
        CPSS_ALDRIN2_E | CPSS_FALCON_E |  CPSS_AC5P_E |  CPSS_AC5X_E | CPSS_HARRIER_E);

    /* get info about the register address and the start bit */
    rc = exceptionRegInfoGet(devNum,type,&regAddr,&startBit,NULL,NULL);
    if(rc != GT_OK)
    {
        return rc;
    }

    switch(command)
    {
        case CPSS_PACKET_CMD_FORWARD_E      :
        case CPSS_PACKET_CMD_MIRROR_TO_CPU_E:
        case CPSS_PACKET_CMD_TRAP_TO_CPU_E  :
        case CPSS_PACKET_CMD_DROP_HARD_E    :
        case CPSS_PACKET_CMD_DROP_SOFT_E    :
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(command);
    }

    hwValue = command;

    return prvCpssHwPpSetRegField(devNum, regAddr, startBit, 3, hwValue);
}

/**
* @internal cpssDxChStreamExceptionCommandSet function
* @endinternal
*
* @brief  Set the packet command to specific exception type.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum         - device number.
* @param[in] type           - The type of exception.
* @param[in] command        - the packet command.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum/type/command
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChStreamExceptionCommandSet
(
    IN GT_U8                                    devNum,
    IN CPSS_DXCH_STREAM_EXCEPTION_TYPE_ENT      type,
    IN CPSS_PACKET_CMD_ENT                      command
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChStreamExceptionCommandSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, type, command));

    rc = internal_cpssDxChStreamExceptionCommandSet(devNum, type, command);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, type, command));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal cpssDxChStreamExceptionCommandGet function
* @endinternal
*
* @brief  Get the packet command for specific exception type.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum         - device number.
* @param[in] type           - The type of exception.
*
* @param[out] commandPtr    - (pointer to) the packet command.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum/type
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
static GT_STATUS internal_cpssDxChStreamExceptionCommandGet
(
    IN GT_U8                                    devNum,
    IN CPSS_DXCH_STREAM_EXCEPTION_TYPE_ENT      type,
    OUT CPSS_PACKET_CMD_ENT                     *commandPtr
)
{
    GT_STATUS   rc;      /* return code */
    GT_U32      regAddr; /* register address */
    GT_U32      startBit;/* start bit in the register */
    GT_U32      hwValue; /* value read from HW */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E |
        CPSS_BOBCAT2_E | CPSS_BOBCAT3_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E |
        CPSS_ALDRIN2_E | CPSS_FALCON_E |  CPSS_AC5P_E |  CPSS_AC5X_E | CPSS_HARRIER_E);
    CPSS_NULL_PTR_CHECK_MAC(commandPtr);

    /* get info about the register address and the start bit */
    rc = exceptionRegInfoGet(devNum,type,&regAddr,&startBit,NULL,NULL);
    if(rc != GT_OK)
    {
        return rc;
    }

    rc = prvCpssHwPpGetRegField(devNum, regAddr, startBit, 3, &hwValue);
    if(rc != GT_OK)
    {
        return rc;
    }

    *commandPtr = hwValue;
    switch(hwValue)
    {
        case CPSS_PACKET_CMD_FORWARD_E      :
        case CPSS_PACKET_CMD_MIRROR_TO_CPU_E:
        case CPSS_PACKET_CMD_TRAP_TO_CPU_E  :
        case CPSS_PACKET_CMD_DROP_HARD_E    :
        case CPSS_PACKET_CMD_DROP_SOFT_E    :
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE,"unknown packet command [%d] in the register",
                hwValue);
    }

    return GT_OK;
}

/**
* @internal cpssDxChStreamExceptionCommandGet function
* @endinternal
*
* @brief  Get the packet command for specific exception type.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum         - device number.
* @param[in] type           - The type of exception.
*
* @param[out] commandPtr    - (pointer to) the packet command.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum/type
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChStreamExceptionCommandGet
(
    IN GT_U8                                    devNum,
    IN CPSS_DXCH_STREAM_EXCEPTION_TYPE_ENT      type,
    OUT CPSS_PACKET_CMD_ENT                     *commandPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChStreamExceptionCommandGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, type, commandPtr));

    rc = internal_cpssDxChStreamExceptionCommandGet(devNum, type, commandPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, type, commandPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssDxChStreamExceptionCpuCodeSet function
* @endinternal
*
* @brief  Set the cpu code to specific exception type.
*         Note : Relevant when the packet command for the exception is not 'FORWARD'
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum         - device number.
* @param[in] type           - The type of exception.
* @param[in] cpuCode        - the cpu code.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum/type/cpuCode
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
static GT_STATUS internal_cpssDxChStreamExceptionCpuCodeSet
(
    IN  GT_U8                                    devNum,
    IN  CPSS_DXCH_STREAM_EXCEPTION_TYPE_ENT      type,
    IN  CPSS_NET_RX_CPU_CODE_ENT                 cpuCode
)
{
    GT_STATUS   rc;      /* return code */
    GT_U32      regAddr; /* register address */
    GT_U32      startBit;/* start bit in the register */
    GT_U32      hwValue; /* value to write to HW */
    PRV_CPSS_DXCH_NET_DSA_TAG_CPU_CODE_ENT dsaCpuCode;/* HW cpu code enumeration */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E |
        CPSS_BOBCAT2_E | CPSS_BOBCAT3_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E |
        CPSS_ALDRIN2_E | CPSS_FALCON_E |  CPSS_AC5P_E |  CPSS_AC5X_E | CPSS_HARRIER_E);

    /* get info about the register address and the start bit */
    rc = exceptionRegInfoGet(devNum,type,NULL,NULL,&regAddr,&startBit);
    if(rc != GT_OK)
    {
        return rc;
    }
    /* convert generic cpu code into 'hw cpu code' */
    rc = prvCpssDxChNetIfCpuToDsaCode(cpuCode, &dsaCpuCode);
    if (rc != GT_OK)
    {
        return rc;
    }

    hwValue = dsaCpuCode;

    return prvCpssHwPpSetRegField(devNum, regAddr, startBit, 8, hwValue);
}

/**
* @internal cpssDxChStreamExceptionCpuCodeSet function
* @endinternal
*
* @brief  Set the cpu code to specific exception type.
*         Note : Relevant when the packet command for the exception is not 'FORWARD'
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum         - device number.
* @param[in] type           - The type of exception.
* @param[in] cpuCode        - the cpu code.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum/type/cpuCode
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChStreamExceptionCpuCodeSet
(
    IN  GT_U8                                    devNum,
    IN  CPSS_DXCH_STREAM_EXCEPTION_TYPE_ENT      type,
    IN  CPSS_NET_RX_CPU_CODE_ENT                 cpuCode
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChStreamExceptionCpuCodeSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, type, cpuCode));

    rc = internal_cpssDxChStreamExceptionCpuCodeSet(devNum, type, cpuCode);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, type, cpuCode));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChStreamExceptionCpuCodeGet function
* @endinternal
*
* @brief  Get the cpu code for specific exception type.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum         - device number.
* @param[in] type           - The type of exception.
*
* @param[out] cpuCodePtr    - (pointer to) The cpu code.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum/type
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
static GT_STATUS internal_cpssDxChStreamExceptionCpuCodeGet
(
    IN  GT_U8                                    devNum,
    IN  CPSS_DXCH_STREAM_EXCEPTION_TYPE_ENT      type,
    OUT CPSS_NET_RX_CPU_CODE_ENT                 *cpuCodePtr
){
    GT_STATUS   rc;      /* return code */
    GT_U32      regAddr; /* register address */
    GT_U32      startBit;/* start bit in the register */
    GT_U32      hwValue; /* value read from HW */
    PRV_CPSS_DXCH_NET_DSA_TAG_CPU_CODE_ENT dsaCpuCode;/* HW cpu code enumeration */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E |
        CPSS_BOBCAT2_E | CPSS_BOBCAT3_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E |
        CPSS_ALDRIN2_E | CPSS_FALCON_E |  CPSS_AC5P_E |  CPSS_AC5X_E | CPSS_HARRIER_E);
    CPSS_NULL_PTR_CHECK_MAC(cpuCodePtr);

    /* get info about the register address and the start bit */
    rc = exceptionRegInfoGet(devNum,type,NULL,NULL,&regAddr,&startBit);
    if(rc != GT_OK)
    {
        return rc;
    }

    rc = prvCpssHwPpGetRegField(devNum, regAddr, startBit, 8, &hwValue);
    if(rc != GT_OK)
    {
        return rc;
    }

    dsaCpuCode = hwValue;

    /* convert 'hw cpu code' into generic cpu code*/
    rc = prvCpssDxChNetIfDsaToCpuCode(dsaCpuCode,cpuCodePtr);
    if (rc != GT_OK)
    {
        return rc;
    }

    return GT_OK;
}

/**
* @internal cpssDxChStreamExceptionCpuCodeGet function
* @endinternal
*
* @brief  Get the cpu code for specific exception type.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum         - device number.
* @param[in] type           - The type of exception.
*
* @param[out] cpuCodePtr    - (pointer to) The cpu code.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum/type
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChStreamExceptionCpuCodeGet
(
    IN  GT_U8                                    devNum,
    IN  CPSS_DXCH_STREAM_EXCEPTION_TYPE_ENT      type,
    OUT CPSS_NET_RX_CPU_CODE_ENT                 *cpuCodePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChStreamExceptionCpuCodeGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, type, cpuCodePtr));

    rc = internal_cpssDxChStreamExceptionCpuCodeGet(devNum, type, cpuCodePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, type, cpuCodePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChStreamCountingModeSet function
* @endinternal
*
* @brief   Set the counting mode for streams.
*   NOTE: the feature not relate to the CNC unit.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum                - device number
* @param[in] irfMode               - The mode for the IRF byte counting (L2/L3 counting)
* @param[in] srfTaglessEnable      - enable/disable the SRF tagless counting.
*                                    GT_TRUE  - enable  the SRF tagless counting
*                                    GT_FALSE - disable the SRF tagless counting
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad parameter error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
static GT_STATUS internal_cpssDxChStreamCountingModeSet
(
    IN  GT_U8                                   devNum,
    IN  CPSS_DXCH_CNC_BYTE_COUNT_MODE_ENT       irfMode,
    IN  GT_BOOL                                 srfTaglessEnable
)
{
    GT_STATUS   rc;         /* return code                  */
    GT_U32      regAddr;    /* register address             */
    GT_U32      hwValue;    /* value to write to the register */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E |
        CPSS_AC5P_E |  CPSS_AC5X_E | CPSS_HARRIER_E);

    regAddr = PRV_DXCH_REG1_UNIT_SMU_MAC(devNum).irfSng.irfSngGlobalConfig;

    switch(irfMode)
    {
        case CPSS_DXCH_CNC_BYTE_COUNT_MODE_L2_E:
            hwValue = 0;
            break;
        case CPSS_DXCH_CNC_BYTE_COUNT_MODE_L3_E:
            hwValue = 1;
            break;
        default :
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    rc = prvCpssHwPpSetRegField(devNum, regAddr, 17, 1, hwValue);
    if(rc != GT_OK)
    {
        return rc;
    }

    hwValue = 1 - BOOL2BIT_MAC(srfTaglessEnable);

    regAddr = PRV_DXCH_REG1_UNIT_PREQ_MAC(devNum).FRE.freSrfGlobalConfig1;
    return prvCpssHwPpSetRegField(devNum, regAddr, 16, 1, hwValue);

}

/**
* @internal cpssDxChStreamCountingModeSet function
* @endinternal
*
* @brief   Set the counting mode for streams.
*   NOTE: the feature not relate to the CNC unit.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum                - device number
* @param[in] irfMode               - The mode for the IRF byte counting (L2/L3 counting)
* @param[in] srfTaglessEnable      - enable/disable the SRF tagless counting.
*                                    GT_TRUE  - enable  the SRF tagless counting
*                                    GT_FALSE - disable the SRF tagless counting
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad parameter error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChStreamCountingModeSet
(
    IN  GT_U8                                   devNum,
    IN  CPSS_DXCH_CNC_BYTE_COUNT_MODE_ENT       irfMode,
    IN  GT_BOOL                                 srfTaglessEnable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChStreamCountingModeSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, irfMode,srfTaglessEnable));

    rc = internal_cpssDxChStreamCountingModeSet(devNum, irfMode,srfTaglessEnable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, irfMode,srfTaglessEnable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChStreamCountingModeGet function
* @endinternal
*
* @brief   Get the counting mode for streams.
*   NOTE: the feature not relate to the CNC unit.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum                - device number
*
* @param[out] irfModePtr           - (pointer to) The mode for the IRF byte counting (L2/L3 counting)
* @param[out] irfModePtr           - (pointer to) enable/disable the SRF tagless counting.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad parameter error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
static GT_STATUS internal_cpssDxChStreamCountingModeGet
(
    IN  GT_U8                                   devNum,
    OUT CPSS_DXCH_CNC_BYTE_COUNT_MODE_ENT       *irfModePtr,
    OUT GT_BOOL                                 *srfTaglessEnablePtr
)
{
    GT_STATUS   rc;         /* return code                    */
    GT_U32      regAddr;    /* register address               */
    GT_U32      hwValue;    /* value to write to the register */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E |
        CPSS_AC5P_E |  CPSS_AC5X_E | CPSS_HARRIER_E);

    CPSS_NULL_PTR_CHECK_MAC(irfModePtr);
    CPSS_NULL_PTR_CHECK_MAC(srfTaglessEnablePtr);

    regAddr = PRV_DXCH_REG1_UNIT_SMU_MAC(devNum).irfSng.irfSngGlobalConfig;

    rc = prvCpssHwPpGetRegField(devNum, regAddr, 17, 1, &hwValue);
    if(rc != GT_OK)
    {
        return rc;
    }

    switch(hwValue)
    {
        case 0:
            *irfModePtr = CPSS_DXCH_CNC_BYTE_COUNT_MODE_L2_E;
            break;
        case 1:
        default:
            *irfModePtr = CPSS_DXCH_CNC_BYTE_COUNT_MODE_L3_E;
            break;
    }

    regAddr = PRV_DXCH_REG1_UNIT_PREQ_MAC(devNum).FRE.freSrfGlobalConfig1;
    rc = prvCpssHwPpGetRegField(devNum, regAddr, 16, 1, &hwValue);
    if(rc != GT_OK)
    {
        return rc;
    }

    *srfTaglessEnablePtr = BIT2BOOL_MAC(1-hwValue);

    return GT_OK;
}

/**
* @internal cpssDxChStreamCountingModeGet function
* @endinternal
*
* @brief   Get the counting mode for streams.
*   NOTE: the feature not relate to the CNC unit.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum                - device number
*
* @param[out] irfModePtr           - (pointer to) The mode for the IRF byte counting (L2/L3 counting)
* @param[out] irfModePtr           - (pointer to) enable/disable the SRF tagless counting.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad parameter error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChStreamCountingModeGet
(
    IN  GT_U8                                   devNum,
    OUT CPSS_DXCH_CNC_BYTE_COUNT_MODE_ENT       *irfModePtr,
    OUT GT_BOOL                                 *srfTaglessEnablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChStreamCountingModeGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, irfModePtr,srfTaglessEnablePtr));

    rc = internal_cpssDxChStreamCountingModeGet(devNum, irfModePtr,srfTaglessEnablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, irfModePtr,srfTaglessEnablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChStreamSrfDaemonInfoSet function
* @endinternal
*
* @brief   Set the SRF daemon info.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum                - device number
* @param[in] infoPtr               - (pointer to) The info about the SRF daemon
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad parameter error
* @retval GT_OUT_OF_RANGE          - on out of range parameter error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
static GT_STATUS internal_cpssDxChStreamSrfDaemonInfoSet
(
    IN  GT_U8                                   devNum,
    IN  CPSS_DXCH_STREAM_SRF_DAEMON_INFO_STC    *infoPtr
)
{
    GT_STATUS   rc;         /* return error                   */
    GT_U32      regAddr;    /* register address               */
    GT_U32      hwValue;    /* value to write to the register */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E |
        CPSS_AC5P_E |  CPSS_AC5X_E | CPSS_HARRIER_E);

    CPSS_NULL_PTR_CHECK_MAC(infoPtr);

    /* 23 bits in HW for <Restart Period> */
    if( ((BIT_23-1) < infoPtr->restartPeriodTimeInMicroSec) ||
        (((BIT_23-1) / PRV_CPSS_PP_MAC(devNum)->coreClock) < infoPtr->restartPeriodTimeInMicroSec))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE,
            "The restartPeriodTimeInMicroSec[%d] is more than MAX[%d]",
            infoPtr->restartPeriodTimeInMicroSec,
            ((BIT_23-1) / PRV_CPSS_PP_MAC(devNum)->coreClock));
    }

    /* 26 bits in HW for <Latent Error Period> */
    if( ((BIT_26-1) < infoPtr->latentErrorDetectionPeriodTimeInMicroSec) ||
        (((BIT_26-1) / PRV_CPSS_PP_MAC(devNum)->coreClock) < infoPtr->latentErrorDetectionPeriodTimeInMicroSec))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE,
            "The latentErrorDetectionPeriodTimeInMicroSec[%d] is more than MAX[%d]",
            infoPtr->latentErrorDetectionPeriodTimeInMicroSec,
            ((BIT_26-1) / PRV_CPSS_PP_MAC(devNum)->coreClock));
    }

    /* hwValue = restartPeriodTimeInMicroSec * coreClock (in MHz)
        NOTE: the divide of micro and MHz becomes 1
    */
    hwValue = infoPtr->restartPeriodTimeInMicroSec *
        PRV_CPSS_PP_MAC(devNum)->coreClock;
    regAddr = PRV_DXCH_REG1_UNIT_PREQ_MAC(devNum).FRE.RestartPeriod;
    /* <Restart Period> */
    rc = prvCpssHwPpSetRegField(devNum, regAddr, 0, 23, hwValue);
    if(rc != GT_OK)
    {
        return rc;
    }

    hwValue = infoPtr->latentErrorDetectionPeriodTimeInMicroSec *
        PRV_CPSS_PP_MAC(devNum)->coreClock;
    regAddr = PRV_DXCH_REG1_UNIT_PREQ_MAC(devNum).FRE.LatentErrorPeriod;
    /* <Latent Error Period>*/
    rc = prvCpssHwPpSetRegField(devNum, regAddr, 0, 26, hwValue);
    if(rc != GT_OK)
    {
        return rc;
    }

    regAddr = PRV_DXCH_REG1_UNIT_PREQ_MAC(devNum).FRE.freGlobalConfig;
    hwValue = BOOL2BIT_MAC(infoPtr->latentErrorDetectionAlgorithmEnable)  | /*bit 0*/
              BOOL2BIT_MAC(infoPtr->latentErrorDetectionResetEnable) << 1 | /*bit 1*/
              BOOL2BIT_MAC(infoPtr->latentErrorDaemonEnable)         << 2 | /*bit 2*/
              BOOL2BIT_MAC(infoPtr->restartDaemonEnable)             << 3 ; /*bit 3*/

    /* set bits 0..3*/
    rc = prvCpssHwPpSetRegField(devNum, regAddr, 0, 4, hwValue);
    if(rc != GT_OK)
    {
        return rc;
    }

    return GT_OK;
}

/**
* @internal cpssDxChStreamSrfDaemonInfoSet function
* @endinternal
*
* @brief   Set the SRF daemon info.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum                - device number
* @param[in] infoPtr               - (pointer to) The info about the SRF daemon
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad parameter error
* @retval GT_OUT_OF_RANGE          - on out of range parameter error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChStreamSrfDaemonInfoSet
(
    IN  GT_U8                                   devNum,
    IN  CPSS_DXCH_STREAM_SRF_DAEMON_INFO_STC    *infoPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChStreamSrfDaemonInfoSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, infoPtr));

    rc = internal_cpssDxChStreamSrfDaemonInfoSet(devNum, infoPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, infoPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssDxChStreamSrfDaemonInfoGet function
* @endinternal
*
* @brief   Get the SRF daemon info.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum                - device number
*
* @param[out] infoPtr              - (pointer to) The info about the SRF daemon
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad parameter error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
static GT_STATUS internal_cpssDxChStreamSrfDaemonInfoGet
(
    IN  GT_U8                                   devNum,
    OUT CPSS_DXCH_STREAM_SRF_DAEMON_INFO_STC    *infoPtr
)
{
    GT_STATUS   rc;         /* return error                   */
    GT_U32      regAddr;    /* register address               */
    GT_U32      hwValue;    /* value to write to the register */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E |
        CPSS_AC5P_E |  CPSS_AC5X_E | CPSS_HARRIER_E);

    CPSS_NULL_PTR_CHECK_MAC(infoPtr);

    regAddr = PRV_DXCH_REG1_UNIT_PREQ_MAC(devNum).FRE.RestartPeriod;
    /* <Restart Period> */
    rc = prvCpssHwPpGetRegField(devNum, regAddr, 0, 23, &hwValue);
    if(rc != GT_OK)
    {
        return rc;
    }

    infoPtr->restartPeriodTimeInMicroSec = hwValue / PRV_CPSS_PP_MAC(devNum)->coreClock;

    regAddr = PRV_DXCH_REG1_UNIT_PREQ_MAC(devNum).FRE.LatentErrorPeriod;
    /* <Latent Error Period>*/
    rc = prvCpssHwPpGetRegField(devNum, regAddr, 0, 26, &hwValue);
    if(rc != GT_OK)
    {
        return rc;
    }

    infoPtr->latentErrorDetectionPeriodTimeInMicroSec = hwValue / PRV_CPSS_PP_MAC(devNum)->coreClock;

    regAddr = PRV_DXCH_REG1_UNIT_PREQ_MAC(devNum).FRE.freGlobalConfig;
    /* Get bits 0..3 */
    rc = prvCpssHwPpGetRegField(devNum, regAddr, 0, 4, &hwValue);
    if(rc != GT_OK)
    {
        return rc;
    }

    infoPtr->latentErrorDetectionAlgorithmEnable = BIT2BOOL_MAC(hwValue        & 0x1); /*bit 0*/
    infoPtr->latentErrorDetectionResetEnable     = BIT2BOOL_MAC((hwValue >> 1) & 0x1); /*bit 1*/
    infoPtr->latentErrorDaemonEnable             = BIT2BOOL_MAC((hwValue >> 2) & 0x1); /*bit 2*/
    infoPtr->restartDaemonEnable                 = BIT2BOOL_MAC((hwValue >> 3) & 0x1); /*bit 3*/

    return GT_OK;
}

/**
* @internal cpssDxChStreamSrfDaemonInfoGet function
* @endinternal
*
* @brief   Get the SRF daemon info.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum                - device number
*
* @param[out] infoPtr              - (pointer to) The info about the SRF daemon
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad parameter error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChStreamSrfDaemonInfoGet
(
    IN  GT_U8                                   devNum,
    OUT CPSS_DXCH_STREAM_SRF_DAEMON_INFO_STC    *infoPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChStreamSrfDaemonInfoGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, infoPtr));

    rc = internal_cpssDxChStreamSrfDaemonInfoGet(devNum, infoPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, infoPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}



/**
* @internal internal_cpssDxChStreamEntrySet function
* @endinternal
*
* @brief  Set the stream entry info.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum         - device number.
* @param[in] index          - The index. (streamId)
*                             APPLICABLE RANGE: 0..(2K-1)
* @param[in] sngIrfInfoPtr  - (pointer to) The sng/irf entry info.
*                               ignored if NULL
* @param[in] srfMappingInfoPtr - (pointer to) The SRF mapping entry.
*                               ignored if NULL
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or index
* @retval GT_OUT_OF_RANGE          - on out of range parameter error
* @retval GT_BAD_PTR               - if ALL the pointers are NULL
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
static GT_STATUS internal_cpssDxChStreamEntrySet
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          index,
    IN  CPSS_DXCH_STREAM_SNG_IRF_ENTRY_STC          *sngIrfInfoPtr,
    IN  CPSS_DXCH_STREAM_SRF_MAPPING_ENTRY_STC      *srfMappingInfoPtr
)
{
    GT_STATUS   rc;         /* return code */
    GT_U32      hwValue[8]; /* value write/read to/from HW */
    GT_U32      fieldValue;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E |
        CPSS_BOBCAT2_E | CPSS_BOBCAT3_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E |
        CPSS_ALDRIN2_E | CPSS_FALCON_E |  CPSS_AC5P_E |  CPSS_AC5X_E | CPSS_HARRIER_E);

    SMU_SNG_IRF_INDEX_CHECK_MAC(devNum,index);

    if(sngIrfInfoPtr == NULL &&
       srfMappingInfoPtr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PTR,
            "The Api not support NULL on all the pointers (at least one must be valid)");
    }


    if(srfMappingInfoPtr)
    {
        if(srfMappingInfoPtr->srfIndex >=
            PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.parametericTables.numEntriesStreamSrf)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, "SRF index [0x%4.4x] is out of range of [0x0..0x%4.4x]",
                srfMappingInfoPtr->srfIndex,
                PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.parametericTables.numEntriesStreamSrf-1);
        }

        /* check that the device supports all the physical ports that defined in the BMP */
        rc = prvCpssDxChTablePortsBmpCheck(devNum , &srfMappingInfoPtr->egressPortsBmp);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    if(sngIrfInfoPtr)
    {
        /* start by read the entry*/
        rc = prvCpssDxChReadTableEntry(devNum,
                CPSS_DXCH_SIP6_30_TABLE_SMU_IRF_SNG_E,
                index,hwValue);
        if(rc != GT_OK)
        {
            return rc;
        }

        /* build the fields */

        SIP6_30_SMU_IRF_SNG_FIELD_SET_MAC(devNum,hwValue,
            SIP6_30_SMU_IRF_SNG_TABLE_FIELDS_SNG_SEQUENCE_NUMBER_ASSIGNMENT_E,
            sngIrfInfoPtr->seqNumAssignmentEnable);

        SIP6_30_SMU_IRF_SNG_FIELD_SET_MAC(devNum,hwValue,
            SIP6_30_SMU_IRF_SNG_TABLE_FIELDS_SNG_RESET_PACKET_COUNTER_E,
            sngIrfInfoPtr->resetSeqNum);

        /* NOTE: SIP6_30_SMU_IRF_SNG_TABLE_FIELDS_SNG_PACKET_COUNTER_E is read only*/

        /* write back the entry */
        rc = prvCpssDxChWriteTableEntry(devNum,
                CPSS_DXCH_SIP6_30_TABLE_SMU_IRF_SNG_E,
                index,hwValue);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    if(srfMappingInfoPtr)
    {
        /* read the entry */
        rc = prvCpssDxChReadTableEntry(devNum,
                CPSS_DXCH_SIP6_30_TABLE_PREQ_SRF_MAPPING_E,
                index,hwValue);
        if(rc != GT_OK)
        {
            return rc;
        }

        fieldValue = BOOL2BIT_MAC(srfMappingInfoPtr->srfEnable);
        SIP6_30_PREQ_SRF_MAPPING_FIELD_SET_MAC(devNum,hwValue,
            SIP6_30_PREQ_SRF_MAPPING_TABLE_FIELDS_SRF_ENABLED_E,
            fieldValue);

        fieldValue = BOOL2BIT_MAC(srfMappingInfoPtr->isMulticast);
        SIP6_30_PREQ_SRF_MAPPING_FIELD_SET_MAC(devNum,hwValue,
            SIP6_30_PREQ_SRF_MAPPING_TABLE_FIELDS_STREAM_TYPE_E,
            fieldValue);

        fieldValue = srfMappingInfoPtr->srfIndex;
        SIP6_30_PREQ_SRF_MAPPING_FIELD_SET_MAC(devNum,hwValue,
            SIP6_30_PREQ_SRF_MAPPING_TABLE_FIELDS_SRF_NUMBER_E,
            fieldValue);

        U_ANY_SET_FIELD_BY_ID_MAC(hwValue,
            PRV_TABLE_FORMAT_INFO(devNum)[PRV_CPSS_DXCH_TABLE_FORMAT_PREQ_SRF_MAPPING_E].fieldsInfoPtr,
            SIP6_30_PREQ_SRF_MAPPING_TABLE_FIELDS_EGRESS_PORTS_BITMAP_E,
            &srfMappingInfoPtr->egressPortsBmp.ports[0]);

        /* write back the entry */
        rc = prvCpssDxChWriteTableEntry(devNum,
                CPSS_DXCH_SIP6_30_TABLE_PREQ_SRF_MAPPING_E,
                index,hwValue);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    return GT_OK;
}

/**
* @internal cpssDxChStreamEntrySet function
* @endinternal
*
* @brief  Set the stream entry info.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum         - device number.
* @param[in] index          - The index. (streamId)
*                             APPLICABLE RANGE: 0..(2K-1)
* @param[in] sngIrfInfoPtr  - (pointer to) The sng/irf entry info.
*                               ignored if NULL
* @param[in] srfMappingInfoPtr - (pointer to) The SRF mapping entry.
*                               ignored if NULL
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or index
* @retval GT_OUT_OF_RANGE          - on out of range parameter error
* @retval GT_BAD_PTR               - if ALL the pointers are NULL
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChStreamEntrySet
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          index,
    IN  CPSS_DXCH_STREAM_SNG_IRF_ENTRY_STC          *sngIrfInfoPtr,
    IN  CPSS_DXCH_STREAM_SRF_MAPPING_ENTRY_STC      *srfMappingInfoPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChStreamEntrySet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, index , sngIrfInfoPtr,srfMappingInfoPtr));

    rc = internal_cpssDxChStreamEntrySet(devNum, index , sngIrfInfoPtr,srfMappingInfoPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, index , sngIrfInfoPtr,srfMappingInfoPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssDxChStreamEntryGet function
* @endinternal
*
* @brief  Get the stream entry info.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum         - device number.
* @param[in] index          - The index. (streamId)
*                             APPLICABLE RANGE: 0..(2K-1)
*
* @param[out] sngIrfInfoPtr - (pointer to) The sng/irf entry info.
*                               ignored if NULL
* @param[out] irfCountersPtr - (pointer to) The irf counters entry.
*                               ignored if NULL
* @param[out] srfMappingInfoPtr - (pointer to) The SRF mapping entry.
*                               ignored if NULL
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or index
* @retval GT_BAD_PTR               - if ALL the pointers are NULL
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
static GT_STATUS internal_cpssDxChStreamEntryGet
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          index,
    OUT CPSS_DXCH_STREAM_SNG_IRF_ENTRY_STC          *sngIrfInfoPtr,
    OUT CPSS_DXCH_STREAM_IRF_COUNTERS_ENTRY_STC     *irfCountersPtr,
    OUT CPSS_DXCH_STREAM_SRF_MAPPING_ENTRY_STC      *srfMappingInfoPtr
)
{
    GT_STATUS   rc;         /* return code */
    GT_U32      hwValue[8]; /* value read from HW */
    GT_U32      fieldValue; /* value read from HW */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E |
        CPSS_BOBCAT2_E | CPSS_BOBCAT3_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E |
        CPSS_ALDRIN2_E | CPSS_FALCON_E |  CPSS_AC5P_E |  CPSS_AC5X_E | CPSS_HARRIER_E);

    SMU_SNG_IRF_INDEX_CHECK_MAC(devNum,index);

    if(sngIrfInfoPtr == NULL &&
       irfCountersPtr == NULL &&
       srfMappingInfoPtr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PTR,
            "The Api not support NULL on all the pointers (at least one must be valid)");
    }

    if(sngIrfInfoPtr)
    {
        /* read the entry */
        rc = prvCpssDxChReadTableEntry(devNum,
                CPSS_DXCH_SIP6_30_TABLE_SMU_IRF_SNG_E,
                index,hwValue);
        if(rc != GT_OK)
        {
            return rc;
        }

        /* parse the fields */

        SIP6_30_SMU_IRF_SNG_FIELD_GET_MAC(devNum,hwValue,
            SIP6_30_SMU_IRF_SNG_TABLE_FIELDS_SNG_SEQUENCE_NUMBER_ASSIGNMENT_E,
            fieldValue);
        sngIrfInfoPtr->seqNumAssignmentEnable = BIT2BOOL_MAC(fieldValue);

        SIP6_30_SMU_IRF_SNG_FIELD_GET_MAC(devNum,hwValue,
            SIP6_30_SMU_IRF_SNG_TABLE_FIELDS_SNG_RESET_PACKET_COUNTER_E,
            fieldValue);
        sngIrfInfoPtr->resetSeqNum = BIT2BOOL_MAC(fieldValue);

        SIP6_30_SMU_IRF_SNG_FIELD_GET_MAC(devNum,hwValue,
            SIP6_30_SMU_IRF_SNG_TABLE_FIELDS_SNG_PACKET_COUNTER_E,
            fieldValue);
        sngIrfInfoPtr->currentSeqNum = fieldValue;
    }

    if(irfCountersPtr)
    {
        /* read the entry */
        rc = prvCpssDxChReadTableEntry(devNum,
                CPSS_DXCH_SIP6_30_TABLE_SMU_IRF_COUNTING_E,
                index,hwValue);
        if(rc != GT_OK)
        {
            return rc;
        }

        /*prvCpssPortGroupsBmpCounterSummary*/

        SIP6_30_SMU_IRF_COUNTERS_FIELD_GET_MAC(devNum,hwValue,
            SIP6_30_SMU_IRF_COUNTERS_TABLE_FIELDS_IRF_PASSED_PACKETS_COUNTER_E,
            fieldValue);

        irfCountersPtr->passedPacketCounter.l[0] = fieldValue;
        irfCountersPtr->passedPacketCounter.l[1] = 0;

        SIP6_30_SMU_IRF_COUNTERS_FIELD_GET_MAC(devNum,hwValue,
            SIP6_30_SMU_IRF_COUNTERS_TABLE_FIELDS_IRF_DISCARDED_PACKETS_COUNTER_E,
            fieldValue);

        irfCountersPtr->discardedPacketsOrSameIdCounter.l[0] = fieldValue;
        irfCountersPtr->discardedPacketsOrSameIdCounter.l[1] = 0;

        SIP6_30_SMU_IRF_COUNTERS_FIELD_GET_MAC(devNum,hwValue,
            SIP6_30_SMU_IRF_COUNTERS_TABLE_FIELDS_IRF_OUT_OF_ORDER_PACKETS_COUNTER_E,
            fieldValue);

        irfCountersPtr->outOfOrderPacketsCounter.l[0] = fieldValue;
        irfCountersPtr->outOfOrderPacketsCounter.l[1] = 0;

        SIP6_30_SMU_IRF_COUNTERS_FIELD_GET_MAC(devNum,hwValue,
            SIP6_30_SMU_IRF_COUNTERS_TABLE_FIELDS_IRF_NUMBER_OF_TAGLESS_PACKETS_COUNTER_E,
            fieldValue);

        irfCountersPtr->taglessPacketsCounter.l[0] = fieldValue;
        irfCountersPtr->taglessPacketsCounter.l[1] = 0;

        U_ANY_GET_FIELD_BY_ID_MAC(hwValue,
            PRV_TABLE_FORMAT_INFO(devNum)[PRV_CPSS_DXCH_TABLE_FORMAT_SMU_IRF_COUNTERS_E].fieldsInfoPtr,
            SIP6_30_SMU_IRF_COUNTERS_TABLE_FIELDS_IRF_BYTE_COUNT_E,
            &irfCountersPtr->taglessPacketsCounter.l[0]);
    }

    if(srfMappingInfoPtr)
    {
        /* read the entry */
        rc = prvCpssDxChReadTableEntry(devNum,
                CPSS_DXCH_SIP6_30_TABLE_PREQ_SRF_MAPPING_E,
                index,hwValue);
        if(rc != GT_OK)
        {
            return rc;
        }

        SIP6_30_PREQ_SRF_MAPPING_FIELD_GET_MAC(devNum,hwValue,
            SIP6_30_PREQ_SRF_MAPPING_TABLE_FIELDS_SRF_ENABLED_E,
            fieldValue);

        srfMappingInfoPtr->srfEnable = BIT2BOOL_MAC(fieldValue);

        SIP6_30_PREQ_SRF_MAPPING_FIELD_GET_MAC(devNum,hwValue,
            SIP6_30_PREQ_SRF_MAPPING_TABLE_FIELDS_STREAM_TYPE_E,
            fieldValue);

        srfMappingInfoPtr->isMulticast = BIT2BOOL_MAC(fieldValue);

        SIP6_30_PREQ_SRF_MAPPING_FIELD_GET_MAC(devNum,hwValue,
            SIP6_30_PREQ_SRF_MAPPING_TABLE_FIELDS_SRF_NUMBER_E,
            fieldValue);

        srfMappingInfoPtr->srfIndex = fieldValue;

        U_ANY_GET_FIELD_BY_ID_MAC(hwValue,
            PRV_TABLE_FORMAT_INFO(devNum)[PRV_CPSS_DXCH_TABLE_FORMAT_PREQ_SRF_MAPPING_E].fieldsInfoPtr,
            SIP6_30_PREQ_SRF_MAPPING_TABLE_FIELDS_EGRESS_PORTS_BITMAP_E,
            &srfMappingInfoPtr->egressPortsBmp.ports[0]);
    }

    return GT_OK;
}

/**
* @internal cpssDxChStreamEntryGet function
* @endinternal
*
* @brief  Get the stream entry info.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum         - device number.
* @param[in] index          - The index. (streamId)
*                             APPLICABLE RANGE: 0..(2K-1)
*
* @param[out] sngIrfInfoPtr - (pointer to) The sng/irf entry info.
*                               ignored if NULL
* @param[out] irfCountersPtr - (pointer to) The irf counters entry.
*                               ignored if NULL
* @param[out] srfMappingInfoPtr - (pointer to) The SRF mapping entry.
*                               ignored if NULL
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or index
* @retval GT_BAD_PTR               - if ALL the pointers are NULL
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChStreamEntryGet
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          index,
    OUT CPSS_DXCH_STREAM_SNG_IRF_ENTRY_STC          *sngIrfInfoPtr,
    OUT CPSS_DXCH_STREAM_IRF_COUNTERS_ENTRY_STC     *irfCountersPtr,
    OUT CPSS_DXCH_STREAM_SRF_MAPPING_ENTRY_STC      *srfMappingInfoPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChStreamEntryGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, index , sngIrfInfoPtr,irfCountersPtr,srfMappingInfoPtr));

    rc = internal_cpssDxChStreamEntryGet(devNum, index , sngIrfInfoPtr,irfCountersPtr,srfMappingInfoPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, index , sngIrfInfoPtr,irfCountersPtr,srfMappingInfoPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChStreamAgingGroupSet function
* @endinternal
*
* @brief  Set group of 32 bits bitmap of aging bits (for 32 consecutive streams).
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum         - device number.
* @param[in] startIndex     - The start index. (streamId)
*                             in steps of 32 : 0,32,64 .. (2K-32) for corresponding indexes in cpssDxChStreamEntrySet/Get
*                             APPLICABLE RANGE: 0,32,64 .. (2K-32)
* @param[in] agingGroupBmp  - 32 bits bitmap of aging bits from startIndex .. (startIndex + 31)
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or startIndex
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
static GT_STATUS internal_cpssDxChStreamAgingGroupSet
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          startIndex,
    IN  GT_U32                          agingGroupBmp
)
{
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E |
        CPSS_BOBCAT2_E | CPSS_BOBCAT3_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E |
        CPSS_ALDRIN2_E | CPSS_FALCON_E |  CPSS_AC5P_E |  CPSS_AC5X_E | CPSS_HARRIER_E);

    SMU_SNG_IRF_INDEX_CHECK_MAC(devNum,startIndex);

    if(startIndex & 0x1f)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,
            "Stream index [0x%4.4x] must be in steps of 32 (0,32,64...(%d))",
            startIndex,
            PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.parametericTables.numEntriesStream - 32);
    }

    return prvCpssHwPpWriteRegister(devNum,
        PRV_DXCH_REG1_UNIT_SMU_MAC(devNum).irfSng.irfAgeBit[startIndex / 32],
        agingGroupBmp);
}

/**
* @internal cpssDxChStreamAgingGroupSet function
* @endinternal
*
* @brief  Set group of 32 bits bitmap of aging bits (for 32 consecutive streams).
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum         - device number.
* @param[in] startIndex     - The start index. (streamId)
*                             in steps of 32 : 0,32,64 .. (2K-32) for corresponding indexes in cpssDxChStreamEntrySet/Get
*                             APPLICABLE RANGE: 0,32,64 .. (2K-32)
* @param[in] agingGroupBmp  - 32 bits bitmap of aging bits from startIndex .. (startIndex + 31)
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or startIndex
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChStreamAgingGroupSet
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          startIndex,
    IN  GT_U32                          agingGroupBmp
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChStreamAgingGroupSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, startIndex , agingGroupBmp));

    rc = internal_cpssDxChStreamAgingGroupSet(devNum, startIndex , agingGroupBmp);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, startIndex , agingGroupBmp));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChStreamAgingGroupGet function
* @endinternal
*
* @brief  Get group of 32 bits bitmap of aging bits (for 32 consecutive streams).
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum         - device number.
* @param[in] startIndex     - The start index. (streamId)
*                             in steps of 32 : 0,32,64 .. (2K-32) for corresponding indexes in cpssDxChStreamEntrySet/Get
*                             APPLICABLE RANGE: 0,32,64 .. (2K-32)
*
* @param[out] agingGroupBmpPtr  - (pointer to) 32 bits bitmap of aging bits from startIndex .. (startIndex + 31)
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or startIndex
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
static GT_STATUS internal_cpssDxChStreamAgingGroupGet
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          startIndex,
    OUT GT_U32                          *agingGroupBmpPtr
)
{
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E |
        CPSS_BOBCAT2_E | CPSS_BOBCAT3_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E |
        CPSS_ALDRIN2_E | CPSS_FALCON_E |  CPSS_AC5P_E |  CPSS_AC5X_E | CPSS_HARRIER_E);

    SMU_SNG_IRF_INDEX_CHECK_MAC(devNum,startIndex);
    CPSS_NULL_PTR_CHECK_MAC(agingGroupBmpPtr);

    if(startIndex & 0x1f)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,
            "Stream index [0x%4.4x] must be in steps of 32 (0,32,64...(%d))",
            startIndex,
            PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.parametericTables.numEntriesStream - 32);
    }

    return prvCpssHwPpReadRegister(devNum,
        PRV_DXCH_REG1_UNIT_SMU_MAC(devNum).irfSng.irfAgeBit[startIndex / 32],
        agingGroupBmpPtr);
}
/**
* @internal cpssDxChStreamAgingGroupGet function
* @endinternal
*
* @brief  Get group of 32 bits bitmap of aging bits (for 32 consecutive streams).
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum         - device number.
* @param[in] startIndex     - The start index. (streamId)
*                             in steps of 32 : 0,32,64 .. (2K-32) for corresponding indexes in cpssDxChStreamEntrySet/Get
*                             APPLICABLE RANGE: 0,32,64 .. (2K-32)
*
* @param[out] agingGroupBmpPtr  - (pointer to) 32 bits bitmap of aging bits from startIndex .. (startIndex + 31)
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or startIndex
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChStreamAgingGroupGet
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          startIndex,
    OUT GT_U32                          *agingGroupBmpPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChStreamAgingGroupGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, startIndex , agingGroupBmpPtr));

    rc = internal_cpssDxChStreamAgingGroupGet(devNum, startIndex , agingGroupBmpPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, startIndex , agingGroupBmpPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChStreamSrfEntrySet function
* @endinternal
*
* @brief  Set the stream SRF entry info.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum         - device number.
* @param[in] index          - The SRF index. (pointed by srfIndex)
*                             APPLICABLE RANGE: 0..(1K-1)
* @param[in] srfConfigInfoPtr - (pointer to) The SRF config entry info.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or index
* @retval GT_OUT_OF_RANGE          - on out of range parameter error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
static GT_STATUS internal_cpssDxChStreamSrfEntrySet
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          index,
    IN  CPSS_DXCH_STREAM_SRF_CONFIG_ENTRY_STC       *srfConfigInfoPtr
)
{
    GT_STATUS   rc;         /* return code */
    GT_U32      hwValue[4]; /* value to write to HW (size according to widest table) */
    GT_U32      fieldValue; /* value to write to HW */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E |
        CPSS_BOBCAT2_E | CPSS_BOBCAT3_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E |
        CPSS_ALDRIN2_E | CPSS_FALCON_E |  CPSS_AC5P_E |  CPSS_AC5X_E | CPSS_HARRIER_E);

    SMU_SRF_INDEX_CHECK_MAC(devNum,index);

    CPSS_NULL_PTR_CHECK_MAC(srfConfigInfoPtr);

    cpssOsMemSet(hwValue,0,sizeof(hwValue));

    /* 16 bits in HW for <Restart Threshold> */
    if(BIT_16 <= srfConfigInfoPtr->restartThreshold)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE,
            "The restartThreshold[%d] is more than MAX[%d]",
            srfConfigInfoPtr->restartThreshold,
            BIT_16-1);
    }

    if(srfConfigInfoPtr->leResetTimeEnabled != GT_FALSE &&
       (srfConfigInfoPtr->leResetTimeInSeconds < 1 ||
        srfConfigInfoPtr->leResetTimeInSeconds > 31))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE,
            "leResetTimeInSeconds [%d] is out of range [1..31]",
            srfConfigInfoPtr->leResetTimeInSeconds);
    }

    if(srfConfigInfoPtr->firstHistoryBuffer >= _1K)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE,
            "vectorSequenceHistoryLength [%d] is out of range [0..1023]",
            srfConfigInfoPtr->firstHistoryBuffer);
    }

    if(srfConfigInfoPtr->useVectorRecoveryAlgorithm == GT_FALSE)
    {
        SIP6_30_PREQ_SRF_CONFIG_FIELD_SET_MAC(devNum,hwValue,
            SIP6_30_PREQ_SRF_CONFIG_TABLE_FIELDS_SEQ_HIST_LENGTH_E,
            0);
    }
    else
    {

        if(srfConfigInfoPtr->vectorSequenceHistoryLength < 2 ||
           srfConfigInfoPtr->vectorSequenceHistoryLength > _2K)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE,
                "vectorSequenceHistoryLength [%d] is out of range [2..2K]",
                srfConfigInfoPtr->vectorSequenceHistoryLength);
        }

        SIP6_30_PREQ_SRF_CONFIG_FIELD_SET_MAC(devNum,hwValue,
            SIP6_30_PREQ_SRF_CONFIG_TABLE_FIELDS_SEQ_HIST_LENGTH_E,
            srfConfigInfoPtr->vectorSequenceHistoryLength - 1);/*HW range is 1..(2k-1)*/
    }

    SIP6_30_PREQ_SRF_CONFIG_FIELD_SET_MAC(devNum,hwValue,
        SIP6_30_PREQ_SRF_CONFIG_TABLE_FIELDS_TAKE_NO_SEQUENCE_E,
        BOOL2BIT_MAC(srfConfigInfoPtr->takeNoSeqNum));


    SIP6_30_PREQ_SRF_CONFIG_FIELD_SET_MAC(devNum,hwValue,
        SIP6_30_PREQ_SRF_CONFIG_TABLE_FIELDS_FIRST_BUFFER_E,
        srfConfigInfoPtr->firstHistoryBuffer);


    if(srfConfigInfoPtr->numOfBuffers < 1 ||
       srfConfigInfoPtr->numOfBuffers > 16)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE,
            "numOfBuffers [%d] is out of range [1..16]",
            srfConfigInfoPtr->numOfBuffers);
    }

    SIP6_30_PREQ_SRF_CONFIG_FIELD_SET_MAC(devNum,hwValue,
        SIP6_30_PREQ_SRF_CONFIG_TABLE_FIELDS_NUMBER_OF_BUFFERS_E,
        srfConfigInfoPtr->numOfBuffers - 1);/*HW range is 0..15 */

    /* write the entry (ignore 'read only' fields) */
    rc = prvCpssDxChWriteTableEntry(devNum,
            CPSS_DXCH_SIP6_30_TABLE_PREQ_SRF_CONFIG_E,
            index,hwValue);
    if(rc != GT_OK)
    {
        return rc;
    }

    cpssOsMemSet(hwValue,0,sizeof(hwValue));


    SIP6_30_PREQ_SRF_DAEMON_FIELD_SET_MAC(devNum,hwValue,
        SIP6_30_PREQ_SRF_DAEMON_TABLE_FIELDS_LE_ENABLE_E,
        BOOL2BIT_MAC(srfConfigInfoPtr->leEnable));

    if(srfConfigInfoPtr->leResetTimeEnabled == GT_FALSE)
    {
        SIP6_30_PREQ_SRF_DAEMON_FIELD_SET_MAC(devNum,hwValue,
            SIP6_30_PREQ_SRF_DAEMON_TABLE_FIELDS_LE_RESET_TIME_E,
            0);
    }
    else
    {
        SIP6_30_PREQ_SRF_DAEMON_FIELD_SET_MAC(devNum,hwValue,
            SIP6_30_PREQ_SRF_DAEMON_TABLE_FIELDS_LE_RESET_TIME_E,
            srfConfigInfoPtr->leResetTimeInSeconds);/*HW range is 1..31 */
    }

    /* write the entry (ignore 'read only' fields) */
    rc = prvCpssDxChWriteTableEntry(devNum,
            CPSS_DXCH_SIP6_30_TABLE_PREQ_SRF_DAEMON_E,
            index,hwValue);
    if(rc != GT_OK)
    {
        return rc;
    }

    fieldValue = srfConfigInfoPtr->restartThreshold;

    /* write the field (single field as all others are 'read only counters') */
    rc = prvCpssDxChWriteTableEntryField(devNum,
            CPSS_DXCH_SIP6_30_TABLE_PREQ_SRF_COUNTERS_E,
            index,
            PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
            SIP6_30_PREQ_SRF_COUNTERS_TABLE_FIELDS_RESTART_THRESHOLD_E, /* field name */
            PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
            fieldValue);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* treat <take any> as field in the 'SRF entry' */
    return prvCpssHwPpSetRegField(devNum,
        PRV_DXCH_REG1_UNIT_PREQ_MAC(devNum).FRE.TakeAny[index >> 5],
        index & 0x1f , 1 ,
        BOOL2BIT_MAC(srfConfigInfoPtr->takeAnySeqNum));

}

/**
* @internal cpssDxChStreamSrfEntrySet function
* @endinternal
*
* @brief  Set the stream SRF entry info.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum         - device number.
* @param[in] index          - The SRF index. (pointed by srfIndex)
*                             APPLICABLE RANGE: 0..(1K-1)
* @param[in] srfConfigInfoPtr - (pointer to) The SRF config entry info.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or index
* @retval GT_OUT_OF_RANGE          - on out of range parameter error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChStreamSrfEntrySet
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          index,
    IN  CPSS_DXCH_STREAM_SRF_CONFIG_ENTRY_STC       *srfConfigInfoPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChStreamSrfEntrySet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, index , srfConfigInfoPtr));

    rc = internal_cpssDxChStreamSrfEntrySet(devNum, index , srfConfigInfoPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, index , srfConfigInfoPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChStreamSrfEntryGet function
* @endinternal
*
* @brief  Get the stream SRF entry info.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum         - device number.
* @param[in] index          - The SRF index. (pointed by srfIndex)
*                             APPLICABLE RANGE: 0..(1K-1)
*
* @param[out] srfConfigInfoPtr - (pointer to) The SRF config entry info.
*                               ignored if NULL
* @param[out] srfCountersPtr   - (pointer to) The SRF counters entry.
*                               ignored if NULL
* @param[out] srfStatusInfoPtr - (pointer to) The SRF status entry info.
*                               ignored if NULL
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or index
* @retval GT_BAD_PTR               - if ALL the pointers are NULL
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
static GT_STATUS internal_cpssDxChStreamSrfEntryGet
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          index,
    OUT CPSS_DXCH_STREAM_SRF_CONFIG_ENTRY_STC       *srfConfigInfoPtr,
    OUT CPSS_DXCH_STREAM_SRF_COUNTERS_ENTRY_STC     *srfCountersPtr,
    OUT CPSS_DXCH_STREAM_SRF_STATUS_ENTRY_STC       *srfStatusInfoPtr
)
{
    GT_STATUS   rc;         /* return code */
    GT_U32      hwValue[8]; /* value read from HW (size according to widest table) */
    GT_U32      fieldValue; /* value read from HW */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E |
        CPSS_BOBCAT2_E | CPSS_BOBCAT3_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E |
        CPSS_ALDRIN2_E | CPSS_FALCON_E |  CPSS_AC5P_E |  CPSS_AC5X_E | CPSS_HARRIER_E);

    SMU_SRF_INDEX_CHECK_MAC(devNum,index);

    if(srfConfigInfoPtr == NULL && srfCountersPtr == NULL && srfStatusInfoPtr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PTR,
            "The Api not support NULL on all the pointers (at least one must be valid)");
    }

    cpssOsMemSet(hwValue,0,sizeof(hwValue));

    /* read the config entry */
    rc = prvCpssDxChReadTableEntry(devNum,
            CPSS_DXCH_SIP6_30_TABLE_PREQ_SRF_CONFIG_E,
            index,hwValue);
    if(rc != GT_OK)
    {
        return rc;
    }

    if(srfConfigInfoPtr)
    {
        SIP6_30_PREQ_SRF_CONFIG_FIELD_GET_MAC(devNum,hwValue,
            SIP6_30_PREQ_SRF_CONFIG_TABLE_FIELDS_SEQ_HIST_LENGTH_E,
            fieldValue);

        if(fieldValue == 0)
        {
            srfConfigInfoPtr->useVectorRecoveryAlgorithm = GT_FALSE;
            srfConfigInfoPtr->vectorSequenceHistoryLength = 0;
        }
        else
        {
            srfConfigInfoPtr->useVectorRecoveryAlgorithm = GT_TRUE;
            srfConfigInfoPtr->vectorSequenceHistoryLength = fieldValue + 1;
        }

        SIP6_30_PREQ_SRF_CONFIG_FIELD_GET_MAC(devNum,hwValue,
            SIP6_30_PREQ_SRF_CONFIG_TABLE_FIELDS_TAKE_NO_SEQUENCE_E,
            fieldValue);
        srfConfigInfoPtr->takeNoSeqNum = BIT2BOOL_MAC(fieldValue);

        SIP6_30_PREQ_SRF_CONFIG_FIELD_GET_MAC(devNum,hwValue,
            SIP6_30_PREQ_SRF_CONFIG_TABLE_FIELDS_FIRST_BUFFER_E,
            fieldValue);
        srfConfigInfoPtr->firstHistoryBuffer = fieldValue;

        SIP6_30_PREQ_SRF_CONFIG_FIELD_GET_MAC(devNum,hwValue,
            SIP6_30_PREQ_SRF_CONFIG_TABLE_FIELDS_NUMBER_OF_BUFFERS_E,
            fieldValue);

        srfConfigInfoPtr->numOfBuffers = fieldValue + 1;
    }

    if(srfCountersPtr)
    {
        SIP6_30_PREQ_SRF_CONFIG_FIELD_GET_MAC(devNum,hwValue,
            SIP6_30_PREQ_SRF_CONFIG_TABLE_FIELDS_ROGUE_PKTS_CNT_E,
            fieldValue);
        srfCountersPtr->roguePacketCounter.l[0] = fieldValue;
    }

    if(srfStatusInfoPtr)
    {
        SIP6_30_PREQ_SRF_CONFIG_FIELD_GET_MAC(devNum,hwValue,
            SIP6_30_PREQ_SRF_CONFIG_TABLE_FIELDS_RECOV_SEQ_NUM_E,
            fieldValue);
        srfStatusInfoPtr->recoverySeqNum = fieldValue;

        SIP6_30_PREQ_SRF_CONFIG_FIELD_GET_MAC(devNum,hwValue,
            SIP6_30_PREQ_SRF_CONFIG_TABLE_FIELDS_CUR_BUFFER_E,
            fieldValue);
        srfStatusInfoPtr->currHistoryBuffer = fieldValue;

        SIP6_30_PREQ_SRF_CONFIG_FIELD_GET_MAC(devNum,hwValue,
            SIP6_30_PREQ_SRF_CONFIG_TABLE_FIELDS_CUR_BIT_E,
            fieldValue);
        srfStatusInfoPtr->currHistoryBufferBit = fieldValue;
    }

    /* read the daemon entry */
    rc = prvCpssDxChReadTableEntry(devNum,
            CPSS_DXCH_SIP6_30_TABLE_PREQ_SRF_DAEMON_E,
            index,hwValue);
    if(rc != GT_OK)
    {
        return rc;
    }

    if(srfConfigInfoPtr)
    {
        SIP6_30_PREQ_SRF_DAEMON_FIELD_GET_MAC(devNum,hwValue,
            SIP6_30_PREQ_SRF_DAEMON_TABLE_FIELDS_LE_ENABLE_E,
            fieldValue);
        srfConfigInfoPtr->leEnable = BIT2BOOL_MAC(fieldValue);

        SIP6_30_PREQ_SRF_DAEMON_FIELD_GET_MAC(devNum,hwValue,
            SIP6_30_PREQ_SRF_DAEMON_TABLE_FIELDS_LE_RESET_TIME_E,
            fieldValue);

        if(fieldValue == 0)
        {
            srfConfigInfoPtr->leResetTimeEnabled = GT_FALSE;
            srfConfigInfoPtr->leResetTimeInSeconds = 0;
        }
        else
        {
            srfConfigInfoPtr->leResetTimeEnabled = GT_TRUE;
            srfConfigInfoPtr->leResetTimeInSeconds = fieldValue;
        }
    }

    if(srfStatusInfoPtr)
    {
        SIP6_30_PREQ_SRF_DAEMON_FIELD_GET_MAC(devNum,hwValue,
            SIP6_30_PREQ_SRF_DAEMON_TABLE_FIELDS_NUMBER_OF_RESETS_E,
            fieldValue);
        srfStatusInfoPtr->numberOfResets = fieldValue;

        SIP6_30_PREQ_SRF_DAEMON_FIELD_GET_MAC(devNum,hwValue,
            SIP6_30_PREQ_SRF_DAEMON_TABLE_FIELDS_LE_BASE_DIFFERENCE_E,
            fieldValue);
        srfStatusInfoPtr->leBaseDifference = fieldValue;

        SIP6_30_PREQ_SRF_DAEMON_FIELD_GET_MAC(devNum,hwValue,
            SIP6_30_PREQ_SRF_DAEMON_TABLE_FIELDS_LE_TIME_SINCE_LAST_RESET_E,
            fieldValue);
        srfStatusInfoPtr->leTimeInSecSinceLastReset = fieldValue;

    }

    /* read the counters entry */
    rc = prvCpssDxChReadTableEntry(devNum,
            CPSS_DXCH_SIP6_30_TABLE_PREQ_SRF_COUNTERS_E,
            index,hwValue);
    if(rc != GT_OK)
    {
        return rc;
    }

    if(srfConfigInfoPtr)
    {
        SIP6_30_PREQ_SRF_COUNTERS_FIELD_GET_MAC(devNum,hwValue,
            SIP6_30_PREQ_SRF_COUNTERS_TABLE_FIELDS_RESTART_THRESHOLD_E,
            fieldValue);
        srfConfigInfoPtr->restartThreshold = fieldValue;
    }

    if(srfCountersPtr)
    {
        SIP6_30_PREQ_SRF_COUNTERS_FIELD_GET_MAC(devNum,hwValue,
            SIP6_30_PREQ_SRF_COUNTERS_TABLE_FIELDS_PASSED_PKTS_CNT_E,
            fieldValue);
        srfCountersPtr->passedPacketsCounter.l[0] = fieldValue;

        SIP6_30_PREQ_SRF_COUNTERS_FIELD_GET_MAC(devNum,hwValue,
            SIP6_30_PREQ_SRF_COUNTERS_TABLE_FIELDS_DISCARDED_PKTS_CNT_E,
            fieldValue);
        srfCountersPtr->discardedPacketsCounter.l[0] = fieldValue;

        SIP6_30_PREQ_SRF_COUNTERS_FIELD_GET_MAC(devNum,hwValue,
            SIP6_30_PREQ_SRF_COUNTERS_TABLE_FIELDS_OUT_OF_ORDER_PKTS_CNT_E,
            fieldValue);
        srfCountersPtr->outOfOrderPacketsCounter.l[0] = fieldValue;

        SIP6_30_PREQ_SRF_COUNTERS_FIELD_GET_MAC(devNum,hwValue,
            SIP6_30_PREQ_SRF_COUNTERS_TABLE_FIELDS_CLEARED_ZERO_HIST_CNT_E,
            fieldValue);
        srfCountersPtr->clearedZeroHistoryBitsCounter.l[0] = fieldValue;
    }

    if(srfStatusInfoPtr)
    {
        SIP6_30_PREQ_SRF_COUNTERS_FIELD_GET_MAC(devNum,hwValue,
            SIP6_30_PREQ_SRF_COUNTERS_TABLE_FIELDS_RESTART_COUNTER_E,
            fieldValue);
        srfStatusInfoPtr->untillRestartTimeInMicroSec =
            fieldValue / PRV_CPSS_PP_MAC(devNum)->coreClock;
    }

    if(srfConfigInfoPtr)
    {
        /* treat <take any> as field in the 'SRF entry' */
        rc = prvCpssHwPpGetRegField(devNum,
            PRV_DXCH_REG1_UNIT_PREQ_MAC(devNum).FRE.TakeAny[index >> 5],
            index & 0x1f , 1 ,
            &fieldValue);
        if(rc != GT_OK)
        {
            return rc;
        }

        srfConfigInfoPtr->takeAnySeqNum = BIT2BOOL_MAC(fieldValue);
    }

    return GT_OK;
}

/**
* @internal cpssDxChStreamSrfEntryGet function
* @endinternal
*
* @brief  Get the stream SRF entry info.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum         - device number.
* @param[in] index          - The SRF index. (pointed by srfIndex)
*                             APPLICABLE RANGE: 0..(1K-1)
*
* @param[out] srfConfigInfoPtr - (pointer to) The SRF config entry info.
*                               ignored if NULL
* @param[out] srfCountersPtr   - (pointer to) The SRF counters entry.
*                               ignored if NULL
* @param[out] srfStatusInfoPtr - (pointer to) The SRF status entry info.
*                               ignored if NULL
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or index
* @retval GT_BAD_PTR               - if ALL the pointers are NULL
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChStreamSrfEntryGet
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          index,
    OUT CPSS_DXCH_STREAM_SRF_CONFIG_ENTRY_STC       *srfConfigInfoPtr,
    OUT CPSS_DXCH_STREAM_SRF_COUNTERS_ENTRY_STC     *srfCountersPtr,
    OUT CPSS_DXCH_STREAM_SRF_STATUS_ENTRY_STC       *srfStatusInfoPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChStreamSrfEntryGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, index , srfConfigInfoPtr,srfCountersPtr,srfStatusInfoPtr));

    rc = internal_cpssDxChStreamSrfEntryGet(devNum, index , srfConfigInfoPtr,srfCountersPtr,srfStatusInfoPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, index , srfConfigInfoPtr,srfCountersPtr,srfStatusInfoPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssDxChStreamSrfLatentErrorDetectedGroupGet function
* @endinternal
*
* @brief  Get group of 32 bits bitmap of error bits (for 32 consecutive SRF indexes).
*   NOTE: 1. can call this function on event of type CPSS_PP_STREAM_LATENT_ERROR_E
*         to get the streams with latent error
*         2. the registers are Read only and clear after read.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum         - device number.
* @param[in] startIndex     - The start SRF index. (pointed by srfIndex)
*                             in steps of 32  : 0,32,64 .. (1K-32) for corresponding indexes in cpssDxChStreamSrfEntrySet/Get
*                             APPLICABLE RANGE: 0,32,64 .. (1K-32)
*
* @param[out] errorGroupBmpPtr  - (pointer to) 32 bits bitmap of error bits from startIndex .. (startIndex + 31)
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or startIndex
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
static GT_STATUS internal_cpssDxChStreamSrfLatentErrorDetectedGroupGet
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          startIndex,
    OUT GT_U32                          *errorGroupBmpPtr
)
{
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E |
        CPSS_BOBCAT2_E | CPSS_BOBCAT3_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E |
        CPSS_ALDRIN2_E | CPSS_FALCON_E |  CPSS_AC5P_E |  CPSS_AC5X_E | CPSS_HARRIER_E);

    SMU_SRF_INDEX_CHECK_MAC(devNum,startIndex);
    CPSS_NULL_PTR_CHECK_MAC(errorGroupBmpPtr);

    if(startIndex & 0x1f)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,
            "SRF index [0x%4.4x] must be in steps of 32 (0,32,64...(%d))",
            startIndex,
            PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.parametericTables.numEntriesStreamSrf - 32);
    }

    return prvCpssHwPpReadRegister(devNum,
        PRV_DXCH_REG1_UNIT_PREQ_MAC(devNum).FRE.ErrorDetected[startIndex / 32],
        errorGroupBmpPtr);
}

/**
* @internal cpssDxChStreamSrfLatentErrorDetectedGroupGet function
* @endinternal
*
* @brief  Get group of 32 bits bitmap of error bits (for 32 consecutive SRF indexes).
*   NOTE: 1. can call this function on event of type CPSS_PP_STREAM_LATENT_ERROR_E
*         to get the streams with latent error
*         2. the registers are Read only and clear after read.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum         - device number.
* @param[in] startIndex     - The start SRF index. (pointed by srfIndex)
*                             in steps of 32  : 0,32,64 .. (1K-32) for corresponding indexes in cpssDxChStreamSrfEntrySet/Get
*                             APPLICABLE RANGE: 0,32,64 .. (1K-32)
*
* @param[out] errorGroupBmpPtr  - (pointer to) 32 bits bitmap of error bits from startIndex .. (startIndex + 31)
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or startIndex
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChStreamSrfLatentErrorDetectedGroupGet
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          startIndex,
    OUT GT_U32                          *errorGroupBmpPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChStreamSrfLatentErrorDetectedGroupGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, startIndex , errorGroupBmpPtr));

    rc = internal_cpssDxChStreamSrfLatentErrorDetectedGroupGet(devNum, startIndex , errorGroupBmpPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, startIndex , errorGroupBmpPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChStreamSrfHistoryBufferEntrySet function
* @endinternal
*
* @brief  Set the stream SRF history buffer entry info. (bitmap of 128 history bits)
*         Needed only for debug purposes.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum         - device number.
* @param[in] index          - The SRF History buffer index. (pointed by 'currHistoryBuffer' or 'firstHistoryBuffer')
*                             APPLICABLE RANGE: 0..(1K-1)
* @param[in] histBuffPtr    - (pointer to) The entry info. (bitmap of 128 history bits)
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or index
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
static GT_STATUS internal_cpssDxChStreamSrfHistoryBufferEntrySet
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          index,
    IN  CPSS_DXCH_STREAM_SRF_HISTORY_BUFFER_ENTRY_STC   *histBuffPtr
)
{
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E |
        CPSS_BOBCAT2_E | CPSS_BOBCAT3_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E |
        CPSS_ALDRIN2_E | CPSS_FALCON_E |  CPSS_AC5P_E |  CPSS_AC5X_E | CPSS_HARRIER_E);

    SMU_SRF_BUFFER_INDEX_CHECK_MAC(devNum,index);

    CPSS_NULL_PTR_CHECK_MAC(histBuffPtr);

    /* write the entry */
    return prvCpssDxChWriteTableEntry(devNum,
            CPSS_DXCH_SIP6_30_TABLE_PREQ_SRF_HISTORY_BUFFER_E,
            index,histBuffPtr->historyBuffer);
}

/**
* @internal cpssDxChStreamSrfHistoryBufferEntrySet function
* @endinternal
*
* @brief  Set the stream SRF history buffer entry info. (bitmap of 128 history bits)
*         Needed only for debug purposes.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum         - device number.
* @param[in] index          - The SRF History buffer index. (pointed by 'currHistoryBuffer' or 'firstHistoryBuffer')
*                             APPLICABLE RANGE: 0..(1K-1)
* @param[in] histBuffPtr    - (pointer to) The entry info. (bitmap of 128 history bits)
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or index
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChStreamSrfHistoryBufferEntrySet
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          index,
    IN  CPSS_DXCH_STREAM_SRF_HISTORY_BUFFER_ENTRY_STC   *histBuffPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChStreamSrfHistoryBufferEntrySet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, index , histBuffPtr));

    rc = internal_cpssDxChStreamSrfHistoryBufferEntrySet(devNum, index , histBuffPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, index , histBuffPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChStreamSrfHistoryBufferEntryGet function
* @endinternal
*
* @brief  Get the stream SRF history buffer entry info. (bitmap of 128 history bits)
*         Needed only for debug purposes.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum         - device number.
* @param[in] index          - The SRF History buffer index. (pointed by 'currHistoryBuffer' or 'firstHistoryBuffer')
*                             APPLICABLE RANGE: 0..(1K-1)
*
* @param[out] histBuffPtr    - (pointer to) The entry info. (bitmap of 128 history bits)
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or index
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
static GT_STATUS internal_cpssDxChStreamSrfHistoryBufferEntryGet
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          index,
    OUT CPSS_DXCH_STREAM_SRF_HISTORY_BUFFER_ENTRY_STC   *histBuffPtr
)
{
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E |
        CPSS_BOBCAT2_E | CPSS_BOBCAT3_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E |
        CPSS_ALDRIN2_E | CPSS_FALCON_E |  CPSS_AC5P_E |  CPSS_AC5X_E | CPSS_HARRIER_E);

    SMU_SRF_BUFFER_INDEX_CHECK_MAC(devNum,index);

    CPSS_NULL_PTR_CHECK_MAC(histBuffPtr);

    /* write the entry */
    return prvCpssDxChReadTableEntry(devNum,
            CPSS_DXCH_SIP6_30_TABLE_PREQ_SRF_HISTORY_BUFFER_E,
            index,histBuffPtr->historyBuffer);
}
/**
* @internal cpssDxChStreamSrfHistoryBufferEntryGet function
* @endinternal
*
* @brief  Get the stream SRF history buffer entry info. (bitmap of 128 history bits)
*         Needed only for debug purposes.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum         - device number.
* @param[in] index          - The SRF History buffer index. (pointed by 'currHistoryBuffer' or 'firstHistoryBuffer')
*                             APPLICABLE RANGE: 0..(1K-1)
*
* @param[out] histBuffPtr    - (pointer to) The entry info. (bitmap of 128 history bits)
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or index
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChStreamSrfHistoryBufferEntryGet
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          index,
    OUT CPSS_DXCH_STREAM_SRF_HISTORY_BUFFER_ENTRY_STC   *histBuffPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChStreamSrfHistoryBufferEntryGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, index , histBuffPtr));

    rc = internal_cpssDxChStreamSrfHistoryBufferEntryGet(devNum, index , histBuffPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, index , histBuffPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChStreamSrfZeroBitVectorGroupEntrySet function
* @endinternal
*
* @brief  Set the stream SRF Zero bit vector entry info. (bitmap of 32 bits)
*         Needed only for debug purposes.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum         - device number.
* @param[in] startIndex     - The SRF History buffer index. (pointed by 'currHistoryBuffer' or 'firstHistoryBuffer')
*                             in steps of 32 : 0,32,64 .. (1K-32) for corresponding indexes of 'currHistoryBuffer' or 'firstHistoryBuffer'
*                             APPLICABLE RANGE: 0,32,64 .. (1K-32)
* @param[in] vector          - the vector of the 32 bits
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or startIndex
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
static GT_STATUS internal_cpssDxChStreamSrfZeroBitVectorGroupEntrySet
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          startIndex,
    IN  GT_U32                          vector
)
{
    GT_STATUS           rc;     /*return code*/
    GT_U32              value;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E |
        CPSS_BOBCAT2_E | CPSS_BOBCAT3_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E |
        CPSS_ALDRIN2_E | CPSS_FALCON_E |  CPSS_AC5P_E |  CPSS_AC5X_E | CPSS_HARRIER_E);

    SMU_SRF_ZER_BIT_VECTOR_INDEX_CHECK_MAC(devNum,startIndex);

    value = vector & 0xFFFF;

    rc = prvCpssDxChWriteTableEntry(devNum,
        CPSS_DXCH_SIP6_30_TABLE_PREQ_SRF_ZERO_BIT_VECTOR_0_E,
        startIndex / 32,
        &value);
    if(rc != GT_OK)
    {
        return rc;
    }

    value = vector >> 16;

    rc = prvCpssDxChWriteTableEntry(devNum,
        CPSS_DXCH_SIP6_30_TABLE_PREQ_SRF_ZERO_BIT_VECTOR_1_E,
        startIndex / 32,
        &value);
    return rc;
}

/**
* @internal cpssDxChStreamSrfZeroBitVectorGroupEntrySet function
* @endinternal
*
* @brief  Set the stream SRF Zero bit vector entry info. (bitmap of 32 bits)
*         Needed only for debug purposes.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum         - device number.
* @param[in] startIndex     - The SRF History buffer index. (pointed by 'currHistoryBuffer' or 'firstHistoryBuffer')
*                             in steps of 32 : 0,32,64 .. (1K-32) for corresponding indexes of 'currHistoryBuffer' or 'firstHistoryBuffer'
*                             APPLICABLE RANGE: 0,32,64 .. (1K-32)
* @param[in] vector          - the vector of the 32 bits
*                             APPLICABLE RANGE: 0..0xffff
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or startIndex
* @retval GT_OUT_OF_RANGE          - out of range vector error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChStreamSrfZeroBitVectorGroupEntrySet
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          startIndex,
    IN  GT_U32                          vector
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChStreamSrfZeroBitVectorGroupEntrySet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, startIndex , vector));

    rc = internal_cpssDxChStreamSrfZeroBitVectorGroupEntrySet(devNum, startIndex , vector);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, startIndex , vector));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChStreamSrfZeroBitVectorGroupEntryGet function
* @endinternal
*
* @brief  Get the stream SRF Zero bit vector entry info. (bitmap of 32 bits)
*         Needed only for debug purposes.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum         - device number.
* @param[in] startIndex     - The SRF History buffer index. (pointed by 'currHistoryBuffer' or 'firstHistoryBuffer')
*                             in steps of 32 : 0,32,64 .. (1K-32) for corresponding indexes of 'currHistoryBuffer' or 'firstHistoryBuffer'
*                             APPLICABLE RANGE: 0,32,64 .. (1K-32)
*
* @param[out] vectorPtr       - (point to) the vector of the 32 bits
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or startIndex
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
static GT_STATUS internal_cpssDxChStreamSrfZeroBitVectorGroupEntryGet
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          startIndex,
    OUT GT_U32                          *vectorPtr
)
{
    GT_STATUS           rc;     /*return code*/
    GT_U32              value0,value1;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E |
        CPSS_BOBCAT2_E | CPSS_BOBCAT3_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E |
        CPSS_ALDRIN2_E | CPSS_FALCON_E |  CPSS_AC5P_E |  CPSS_AC5X_E | CPSS_HARRIER_E);

    SMU_SRF_ZER_BIT_VECTOR_INDEX_CHECK_MAC(devNum,startIndex);
    CPSS_NULL_PTR_CHECK_MAC(vectorPtr);

    rc = prvCpssDxChReadTableEntry(devNum,
        CPSS_DXCH_SIP6_30_TABLE_PREQ_SRF_ZERO_BIT_VECTOR_0_E,
        startIndex / 32,
        &value0);
    if(rc != GT_OK)
    {
        return rc;
    }

    rc = prvCpssDxChReadTableEntry(devNum,
        CPSS_DXCH_SIP6_30_TABLE_PREQ_SRF_ZERO_BIT_VECTOR_1_E,
        startIndex / 32,
        &value1);
    if(rc != GT_OK)
    {
        return rc;
    }

    *vectorPtr = value0 | (value1 << 16);

    return GT_OK;
}

/**
* @internal cpssDxChStreamSrfZeroBitVectorGroupEntryGet function
* @endinternal
*
* @brief  Get the stream SRF Zero bit vector entry info. (bitmap of 32 bits)
*         Needed only for debug purposes.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum         - device number.
* @param[in] startIndex     - The SRF History buffer index. (pointed by 'currHistoryBuffer' or 'firstHistoryBuffer')
*                             in steps of 32 : 0,32,64 .. (1K-32) for corresponding indexes of 'currHistoryBuffer' or 'firstHistoryBuffer'
*                             APPLICABLE RANGE: 0,32,64 .. (1K-32)
*
* @param[out] vectorPtr       - (point to) the vector of the 32 bits
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or startIndex
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChStreamSrfZeroBitVectorGroupEntryGet
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          startIndex,
    OUT GT_U32                          *vectorPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChStreamSrfZeroBitVectorGroupEntryGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, startIndex , vectorPtr));

    rc = internal_cpssDxChStreamSrfZeroBitVectorGroupEntryGet(devNum, startIndex , vectorPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, startIndex , vectorPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChStreamSrfTrgPortMappedMulticastIndexSet function
* @endinternal
*
* @brief  Set the mapped target physical port used for calculating the SRF index.
*         Relevant for SRF Multicast Streams only.
*         For LAGs, all the ports in the LAG should be mapped to a single port L
*         in the LAG.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum         - device number.
* @param[in] portNum        - The target physical port number.
* @param[in] index          - The index that represent the physical port in multicast stream.
*                             APPLICABLE RANGES : 0..63 (according to range of physical port)
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or or portNum
* @retval GT_OUT_OF_RANGE          - on out of range parameter error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
static GT_STATUS internal_cpssDxChStreamSrfTrgPortMappedMulticastIndexSet
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  GT_U32                          index
)
{
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E |
        CPSS_BOBCAT2_E | CPSS_BOBCAT3_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E |
        CPSS_ALDRIN2_E | CPSS_FALCON_E |  CPSS_AC5P_E |  CPSS_AC5X_E | CPSS_HARRIER_E);

    PRV_CPSS_DXCH_PHY_PORT_NUM_CHECK_MAC(devNum,portNum);

    return prvCpssDxChWriteTableEntryField(
        devNum,
        CPSS_DXCH_SIP6_TABLE_PREQ_TARGET_PHYSICAL_PORT_E,
        portNum,
        PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
        SIP6_30_PREQ_TARGET_PHYSICAL_PORT_TABLE_FIELDS_SRF_TRG_PHY_PORT_E, /* field name */
        PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
        index);
}

/**
* @internal cpssDxChStreamSrfTrgPortMappedMulticastIndexSet function
* @endinternal
*
* @brief  Set the mapped target physical port used for calculating the SRF index.
*         Relevant for SRF Multicast Streams only.
*         For LAGs, all the ports in the LAG should be mapped to a single port L
*         in the LAG.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum         - device number.
* @param[in] portNum        - The target physical port number.
* @param[in] index          - The index that represent the physical port in multicast stream.
*                             APPLICABLE RANGES : 0..63 (according to range of physical port)
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or or portNum
* @retval GT_OUT_OF_RANGE          - on out of range parameter error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChStreamSrfTrgPortMappedMulticastIndexSet
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  GT_U32                          index
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChStreamSrfTrgPortMappedMulticastIndexSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum , index));

    rc = internal_cpssDxChStreamSrfTrgPortMappedMulticastIndexSet(devNum, portNum , index);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum , index));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChStreamSrfTrgPortMappedMulticastIndexGet function
* @endinternal
*
* @brief  Get the mapped target physical port used for calculating the SRF index.
*         Relevant for SRF Multicast Streams only.
*         For LAGs, all the ports in the LAG should be mapped to a single port L
*         in the LAG.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum         - device number.
* @param[in] portNum        - The target physical port number.
*
* @param[out] indexPtr      - (pointer to) The index that represent the physical port in multicast stream.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or or portNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
static GT_STATUS internal_cpssDxChStreamSrfTrgPortMappedMulticastIndexGet
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    OUT GT_U32                          *indexPtr
)
{
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E |
        CPSS_BOBCAT2_E | CPSS_BOBCAT3_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E |
        CPSS_ALDRIN2_E | CPSS_FALCON_E |  CPSS_AC5P_E |  CPSS_AC5X_E | CPSS_HARRIER_E);

    PRV_CPSS_DXCH_PHY_PORT_NUM_CHECK_MAC(devNum,portNum);
    CPSS_NULL_PTR_CHECK_MAC(indexPtr);

    return prvCpssDxChReadTableEntryField(
        devNum,
        CPSS_DXCH_SIP6_TABLE_PREQ_TARGET_PHYSICAL_PORT_E,
        portNum,
        PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
        SIP6_30_PREQ_TARGET_PHYSICAL_PORT_TABLE_FIELDS_SRF_TRG_PHY_PORT_E, /* field name */
        PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
        indexPtr);
}

/**
* @internal cpssDxChStreamSrfTrgPortMappedMulticastIndexGet function
* @endinternal
*
* @brief  Get the mapped target physical port used for calculating the SRF index.
*         Relevant for SRF Multicast Streams only.
*         For LAGs, all the ports in the LAG should be mapped to a single port L
*         in the LAG.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum         - device number.
* @param[in] portNum        - The target physical port number.
*
* @param[out] indexPtr      - (pointer to) The index that represent the physical port in multicast stream.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or or portNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChStreamSrfTrgPortMappedMulticastIndexGet
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    OUT GT_U32                          *indexPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChStreamSrfTrgPortMappedMulticastIndexGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum , indexPtr));

    rc = internal_cpssDxChStreamSrfTrgPortMappedMulticastIndexGet(devNum, portNum , indexPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum , indexPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal prvCpssDxChStreamSrfUse802_1cbdb function
* @endinternal
*
* @brief   Set the 802.1cbdb mode or the 802.1cb.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum                - device number
* @param[in] enable802_1cbdb       - enable/disable the 802.1cbdb
*                                    GT_TRUE  - enable  the 802.1cbdb
*                                    GT_FALSE - disable the 802.1cbdb (use 802.1cb)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad parameter error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS prvCpssDxChStreamSrfUse802_1cbdb
(
    IN  GT_U8         devNum,
    IN  GT_BOOL       enable802_1cbdb
)
{
    GT_U32      regAddr;    /* register address             */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E |
        CPSS_AC5P_E |  CPSS_AC5X_E | CPSS_HARRIER_E);


    regAddr = PRV_DXCH_REG1_UNIT_PREQ_MAC(devNum).FRE.freSrfGlobalConfig1;
    return prvCpssHwPpSetRegField(devNum, regAddr, 18, 1, BIT2BOOL_MAC(enable802_1cbdb));

}

