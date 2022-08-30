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
* @file cpssDxChHsrPrp.c
*
* @brief CPSS DxCh HSR and PRP configuration API
*   NOTE: some of the function or fields are only for HSR or for PRP.
*
* @version   1
********************************************************************************
*/
#define CPSS_LOG_IN_MODULE_ENABLE

#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/hsrPrp/private/prvCpssDxChHsrPrpLog.h>
#include <cpss/dxCh/dxChxGen/hsrPrp/cpssDxChHsrPrp.h>
/* needed for function prvCpssDxChBrgFdbHashDdeByParamsCalc(...) */
#include <cpss/dxCh/dxChxGen/bridge/private/prvCpssDxChBrgFdbHashCrc.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>


#define PORTS_BMP_CHECK_MAC(_bmp)    \
    if(_bmp >= BIT_10)               \
    {                                \
        /* the support is only for 10 bits (ports) */ \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, "[%s] bitmap of ports [0x%8.8x] hold bits above bit 9 (allowed bits 0..9)",\
            #_bmp,_bmp);             \
    }

/* MACRO to check the DDE index */
#define DDE_INDEX_CHECK_MAC(_devNum,_index)                                                                  \
    if(GT_FALSE == PRV_CPSS_DXCH_PP_MAC(_devNum)->hsrPrp.ddeInfoValid)                                       \
    {                                                                                                        \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,                                                          \
            "The DDE table can't be accesses (at any index) as partition mode was not set for it in the FDB table"); \
    }                                                                                                        \
    if(0 == PRV_CPSS_DXCH_PP_MAC(_devNum)->hsrPrp.ddeInfo.ddeNumEntriesInFdbTable)                           \
    {                                                                                                        \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,                                                          \
            "The DDE table can't be accesses (at any index) as partition size is '0' for it in the FDB table"); \
    }                                                                                                        \
    if(_index >= PRV_CPSS_DXCH_PP_MAC(_devNum)->hsrPrp.ddeInfo.ddeNumEntriesInFdbTable)                      \
    {                                                                                                        \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "DDE index [0x%5.5x] is not in range of [0x0..0x%5.5x]", \
            _index,                                                                                          \
            PRV_CPSS_DXCH_PP_MAC(_devNum)->hsrPrp.ddeInfo.ddeNumEntriesInFdbTable-1);                        \
    }

/* MACRO to check number of DDEs */
#define DDE_NUM_OF_ENTRIES_CHECK_MAC(_devNum,_numOfEntries)                                                  \
    if(_numOfEntries == GT_NA)                                                                               \
    {                                                                                                        \
        _numOfEntries = PRV_CPSS_DXCH_PP_MAC(_devNum)->hsrPrp.ddeInfo.ddeNumEntriesInFdbTable;               \
    }                                                                                                        \
    else                                                                                                     \
    if(_numOfEntries == 0 || _numOfEntries > PRV_CPSS_DXCH_PP_MAC(_devNum)->hsrPrp.ddeInfo.ddeNumEntriesInFdbTable)                \
    {                                                                                                        \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "numOfEntries [0x%5.5x] is not in range of [0x1..0x%5.5x] and not[0xFFFFFFFF]", \
            _numOfEntries,                                                                                   \
            PRV_CPSS_DXCH_PP_MAC(_devNum)->hsrPrp.ddeInfo.ddeNumEntriesInFdbTable);                          \
    }

/* MACRO to check the DDE index */
#define LSDU_OFFSET_CHECK_MAC(_lsduOffset)                \
    if(_lsduOffset >= BIT_6)                              \
    {                                                     \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE,    \
            "lsduOffset [%d] is not in range of [0..%d]", \
            _lsduOffset,63);                              \
    }

/* number of words in the mac entry */
#define SIP_6_30_FDB_ENTRY_WORDS_SIZE_CNS   4

/* the 'entry type' of DDE in the FDB table */
#define HW_FDB_DDE_ENTRY_TYPE_CNS           0x7

/* number of banks for FDB DDE hash */
#define NUM_BANKS_MULTI_HASH_CNS     16


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
    IN CPSS_DXCH_HSR_PRP_EXCEPTION_TYPE_ENT     type,
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
        case CPSS_DXCH_HSR_PRP_EXCEPTION_TYPE_HSR_SIZE_MISMATCH_E:
            regAddr1  = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).TTIUnitGlobalConfigs.hsrPrpGlobalConfig0;
            startBit1 = 16;/*command*/
            regAddr2  = regAddr1;
            startBit2 = 19;/*code*/
            break;
        case CPSS_DXCH_HSR_PRP_EXCEPTION_TYPE_PNT_FULL_UNTAGGED_E:
            regAddr1  = PRV_DXCH_REG1_UNIT_L2I_MAC(devNum).hsrPrp.hsrPrpGlobalConfig1;
            startBit1 = 16;/*command*/
            regAddr2  = regAddr1;
            startBit2 = 19;/*code*/
            break;
        case CPSS_DXCH_HSR_PRP_EXCEPTION_TYPE_PNT_FULL_TAGGED_E:
            regAddr1  = PRV_DXCH_REG1_UNIT_L2I_MAC(devNum).hsrPrp.hsrPrpGlobalConfig2;
            startBit1 = 0;/*command*/
            regAddr2  = regAddr1;
            startBit2 = 3;/*code*/
            break;
        case CPSS_DXCH_HSR_PRP_EXCEPTION_TYPE_ZERO_BMP_E:
            regAddr1  = PRV_DXCH_REG1_UNIT_L2I_MAC(devNum).hsrPrp.hsrPrpGlobalConfig2;
            startBit1 = 11;/*command*/
            regAddr2  = regAddr1;
            startBit2 = 14;/*code*/
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
* @internal internal_cpssDxChHsrPrpExceptionCommandSet function
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
static GT_STATUS internal_cpssDxChHsrPrpExceptionCommandSet
(
    IN GT_U8                                    devNum,
    IN CPSS_DXCH_HSR_PRP_EXCEPTION_TYPE_ENT     type,
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
* @internal cpssDxChHsrPrpExceptionCommandSet function
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
GT_STATUS cpssDxChHsrPrpExceptionCommandSet
(
    IN GT_U8                                    devNum,
    IN CPSS_DXCH_HSR_PRP_EXCEPTION_TYPE_ENT     type,
    IN CPSS_PACKET_CMD_ENT                      command
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChHsrPrpExceptionCommandSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, type, command));

    rc = internal_cpssDxChHsrPrpExceptionCommandSet(devNum, type, command);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, type, command));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChHsrPrpExceptionCommandGet function
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
static GT_STATUS internal_cpssDxChHsrPrpExceptionCommandGet
(
    IN GT_U8                                    devNum,
    IN CPSS_DXCH_HSR_PRP_EXCEPTION_TYPE_ENT     type,
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
* @internal cpssDxChHsrPrpExceptionCommandSet function
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
GT_STATUS cpssDxChHsrPrpExceptionCommandGet
(
    IN GT_U8                                    devNum,
    IN CPSS_DXCH_HSR_PRP_EXCEPTION_TYPE_ENT     type,
    OUT CPSS_PACKET_CMD_ENT                     *commandPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChHsrPrpExceptionCommandGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, type, commandPtr));

    rc = internal_cpssDxChHsrPrpExceptionCommandGet(devNum, type, commandPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, type, commandPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChHsrPrpExceptionCpuCodeSet function
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
static GT_STATUS internal_cpssDxChHsrPrpExceptionCpuCodeSet
(
    IN  GT_U8                                    devNum,
    IN  CPSS_DXCH_HSR_PRP_EXCEPTION_TYPE_ENT     type,
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
* @internal cpssDxChHsrPrpExceptionCpuCodeSet function
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
GT_STATUS cpssDxChHsrPrpExceptionCpuCodeSet
(
    IN  GT_U8                                    devNum,
    IN  CPSS_DXCH_HSR_PRP_EXCEPTION_TYPE_ENT     type,
    IN  CPSS_NET_RX_CPU_CODE_ENT                 cpuCode
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChHsrPrpExceptionCpuCodeSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, type, cpuCode));

    rc = internal_cpssDxChHsrPrpExceptionCpuCodeSet(devNum, type, cpuCode);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, type, cpuCode));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssDxChHsrPrpExceptionCpuCodeGet function
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
static GT_STATUS internal_cpssDxChHsrPrpExceptionCpuCodeGet
(
    IN  GT_U8                                    devNum,
    IN  CPSS_DXCH_HSR_PRP_EXCEPTION_TYPE_ENT     type,
    OUT CPSS_NET_RX_CPU_CODE_ENT                 *cpuCodePtr
)
{
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
* @internal cpssDxChHsrPrpExceptionCpuCodeGet function
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
GT_STATUS cpssDxChHsrPrpExceptionCpuCodeGet
(
    IN  GT_U8                                    devNum,
    IN  CPSS_DXCH_HSR_PRP_EXCEPTION_TYPE_ENT     type,
    OUT CPSS_NET_RX_CPU_CODE_ENT                 *cpuCodePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChHsrPrpExceptionCpuCodeGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, type, cpuCodePtr));

    rc = internal_cpssDxChHsrPrpExceptionCpuCodeGet(devNum, type, cpuCodePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, type, cpuCodePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChHsrPrpPortPrpEnableSet function
* @endinternal
*
* @brief  Enable/Disable the PRP on specific port.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum         - device number.
* @param[in] portNum        - The port number.
*                               APPLICABLE RANGE: 'Default eport' range.
* @param[in] enable         - Indication to Enable/Disable the PRP.
*                             GT_TRUE  - Enable.
*                             GT_FALSE - Disable.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or portNum
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
static GT_STATUS internal_cpssDxChHsrPrpPortPrpEnableSet
(
    IN  GT_U8                    devNum,
    IN  GT_PORT_NUM              portNum,
    IN  GT_BOOL                  enable
)
{
    GT_STATUS   rc;      /* return code */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E |
        CPSS_BOBCAT2_E | CPSS_BOBCAT3_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E |
        CPSS_ALDRIN2_E | CPSS_FALCON_E |  CPSS_AC5P_E |  CPSS_AC5X_E | CPSS_HARRIER_E);

    PRV_CPSS_DXCH_DEFAULT_EPORT_CHECK_MAC(devNum, portNum);

    /* write to post-tti-lookup-ingress-eport table */
    rc = prvCpssDxChWriteTableEntryField(devNum,
            CPSS_DXCH_SIP5_TABLE_PRE_TTI_LOOKUP_INGRESS_EPORT_E,
            portNum,
            PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
            SIP6_30_TTI_DEFAULT_EPORT_TABLE_FIELDS_PRP_ENABLED_E, /* field name */
            PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
            BOOL2BIT_MAC(enable));

    return rc;
}

/**
* @internal cpssDxChHsrPrpPortPrpEnableSet function
* @endinternal
*
* @brief  Enable/Disable the PRP on specific port.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum         - device number.
* @param[in] portNum        - The port number.
*                               APPLICABLE RANGE: 'Default eport' range.
* @param[in] enable         - Indication to Enable/Disable the PRP.
*                             GT_TRUE  - Enable.
*                             GT_FALSE - Disable.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or portNum
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChHsrPrpPortPrpEnableSet
(
    IN  GT_U8                    devNum,
    IN  GT_PORT_NUM              portNum,
    IN  GT_BOOL                  enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChHsrPrpPortPrpEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enable));

    rc = internal_cpssDxChHsrPrpPortPrpEnableSet(devNum, portNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssDxChHsrPrpPortPrpEnableGet function
* @endinternal
*
* @brief  Get the Enable/Disable of PRP on specific port.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum         - device number.
* @param[in] portNum        - The port number.
*                               APPLICABLE RANGE: 'Default eport' range.
*
* @param[out] enablePtr     - (pointer to) Indication to Enable/Disable the PRP.
*                             GT_TRUE  - Enable.
*                             GT_FALSE - Disable.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or portNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
static GT_STATUS internal_cpssDxChHsrPrpPortPrpEnableGet
(
    IN  GT_U8                    devNum,
    IN  GT_PORT_NUM              portNum,
    OUT GT_BOOL                  *enablePtr
)
{
    GT_STATUS rc;
    GT_U32      hwValue; /* value read from HW */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E |
        CPSS_BOBCAT2_E | CPSS_BOBCAT3_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E |
        CPSS_ALDRIN2_E | CPSS_FALCON_E |  CPSS_AC5P_E |  CPSS_AC5X_E | CPSS_HARRIER_E);
    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    PRV_CPSS_DXCH_DEFAULT_EPORT_CHECK_MAC(devNum, portNum);

    /* read from post-tti-lookup-ingress-eport table */
    rc = prvCpssDxChReadTableEntryField(devNum,
            CPSS_DXCH_SIP5_TABLE_PRE_TTI_LOOKUP_INGRESS_EPORT_E,
            portNum,
            PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
            SIP6_30_TTI_DEFAULT_EPORT_TABLE_FIELDS_PRP_ENABLED_E, /* field name */
            PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
            &hwValue);

    *enablePtr = BIT2BOOL_MAC(hwValue);

    return rc;
}

/**
* @internal cpssDxChHsrPrpPortPrpEnableGet function
* @endinternal
*
* @brief  Get the Enable/Disable of PRP on specific port.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum         - device number.
* @param[in] portNum        - The port number.
*                               APPLICABLE RANGE: 'Default eport' range.
*
* @param[out] enablePtr     - (pointer to) Indication to Enable/Disable the PRP.
*                             GT_TRUE  - Enable.
*                             GT_FALSE - Disable.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or portNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChHsrPrpPortPrpEnableGet
(
    IN  GT_U8                    devNum,
    IN  GT_PORT_NUM              portNum,
    OUT GT_BOOL                  *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChHsrPrpPortPrpEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enablePtr));

    rc = internal_cpssDxChHsrPrpPortPrpEnableGet(devNum, portNum, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssDxChHsrPrpPortPrpLanIdSet function
* @endinternal
*
* @brief  Set the PRP LAN ID on specific port.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum         - device number.
* @param[in] portNum        - The port number.
*                             For Ingress direction (and 'Both directions') :
*                               APPLICABLE RANGE: 'Default eport' range.
*                             For Egress direction (and 'Both directions') :
*                               APPLICABLE RANGE: eport range.
* @param[in] direction      - select direction (ingress/egress/Both)
* @param[in] lanId          - The LAN ID.
*                             APPLICABLE RANGE: 0..15.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or portNum/direction
* @retval GT_OUT_OF_RANGE          - out of range lanId
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
static GT_STATUS internal_cpssDxChHsrPrpPortPrpLanIdSet
(
    IN  GT_U8                    devNum,
    IN  GT_PORT_NUM              portNum,
    IN  CPSS_DIRECTION_ENT       direction,
    IN  GT_U32                   lanId
)
{
    GT_STATUS   rc;      /* return code */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E |
        CPSS_BOBCAT2_E | CPSS_BOBCAT3_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E |
        CPSS_ALDRIN2_E | CPSS_FALCON_E |  CPSS_AC5P_E |  CPSS_AC5X_E | CPSS_HARRIER_E);

    switch(direction)
    {
        case CPSS_DIRECTION_INGRESS_E:
            PRV_CPSS_DXCH_DEFAULT_EPORT_CHECK_MAC(devNum, portNum);
            break;
        case CPSS_DIRECTION_EGRESS_E:
            PRV_CPSS_DXCH_PORT_CHECK_MAC(devNum, portNum);
            break;
        case CPSS_DIRECTION_BOTH_E:
            PRV_CPSS_DXCH_DEFAULT_EPORT_CHECK_MAC(devNum, portNum);
            PRV_CPSS_DXCH_PORT_CHECK_MAC(devNum, portNum);
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(direction);
    }

    if(direction == CPSS_DIRECTION_INGRESS_E ||
       direction == CPSS_DIRECTION_BOTH_E)
    {
        PRV_CPSS_DXCH_DEFAULT_EPORT_CHECK_MAC(devNum, portNum);
    }

    if(direction == CPSS_DIRECTION_EGRESS_E ||
       direction == CPSS_DIRECTION_BOTH_E)
    {
    }

    if(direction == CPSS_DIRECTION_INGRESS_E ||
       direction == CPSS_DIRECTION_BOTH_E)
    {
        /* write to post-tti-lookup-ingress-eport table */
        rc = prvCpssDxChWriteTableEntryField(devNum,
                CPSS_DXCH_SIP5_TABLE_PRE_TTI_LOOKUP_INGRESS_EPORT_E,
                portNum,
                PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                SIP6_30_TTI_DEFAULT_EPORT_TABLE_FIELDS_PRT_PORT_LAN_ID_E, /* field name */
                PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                lanId);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    if(direction == CPSS_DIRECTION_EGRESS_E ||
       direction == CPSS_DIRECTION_BOTH_E)
    {
        rc = prvCpssDxChWriteTableEntryField(devNum,
                CPSS_DXCH_SIP5_TABLE_HA_EGRESS_EPORT_1_E,
                portNum,
                PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                SIP6_30_HA_EPORT_TABLE_1_FIELDS_HSR_PRP_LAN_ID_E, /* field name */
                PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                lanId);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    return GT_OK;
}

/**
* @internal cpssDxChHsrPrpPortPrpLanIdSet function
* @endinternal
*
* @brief  Set the PRP LAN ID on specific port.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum         - device number.
* @param[in] portNum        - The port number.
*                             For Ingress direction (and 'Both directions') :
*                               APPLICABLE RANGE: 'Default eport' range.
*                             For Egress direction (and 'Both directions') :
*                               APPLICABLE RANGE: eport range.
* @param[in] direction      - select direction (ingress/egress/Both)
* @param[in] lanId          - The LAN ID.
*                             APPLICABLE RANGE: 0..15.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or portNum/direction
* @retval GT_OUT_OF_RANGE          - out of range lanId
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChHsrPrpPortPrpLanIdSet
(
    IN  GT_U8                    devNum,
    IN  GT_PORT_NUM              portNum,
    IN  CPSS_DIRECTION_ENT       direction,
    IN  GT_U32                   lanId
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChHsrPrpPortPrpLanIdSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, direction ,lanId));

    rc = internal_cpssDxChHsrPrpPortPrpLanIdSet(devNum, portNum, direction ,lanId);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, direction ,lanId));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChHsrPrpPortPrpLanIdGet function
* @endinternal
*
* @brief  Get the PRP LAN ID of specific port.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum         - device number.
* @param[in] portNum        - The port number.
*                             For Ingress direction :
*                               APPLICABLE RANGE: 'Default eport' range.
*                             For Egress direction :
*                               APPLICABLE RANGE: eport range.
* @param[in] direction      - select direction (ingress/egress/Both)
*
* @param[out] lanIdPtr      - (pointer to) The LAN ID.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or portNum/direction
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
static GT_STATUS internal_cpssDxChHsrPrpPortPrpLanIdGet
(
    IN  GT_U8                    devNum,
    IN  GT_PORT_NUM              portNum,
    IN  CPSS_DIRECTION_ENT       direction,
    OUT GT_U32                   *lanIdPtr
)
{
    GT_STATUS   rc;      /* return code */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E |
        CPSS_BOBCAT2_E | CPSS_BOBCAT3_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E |
        CPSS_ALDRIN2_E | CPSS_FALCON_E |  CPSS_AC5P_E |  CPSS_AC5X_E | CPSS_HARRIER_E);
    CPSS_NULL_PTR_CHECK_MAC(lanIdPtr);

    switch(direction)
    {
        case CPSS_DIRECTION_INGRESS_E:
            PRV_CPSS_DXCH_DEFAULT_EPORT_CHECK_MAC(devNum, portNum);
            /* read the post-tti-lookup-ingress-eport table */
            rc = prvCpssDxChReadTableEntryField(devNum,
                    CPSS_DXCH_SIP5_TABLE_PRE_TTI_LOOKUP_INGRESS_EPORT_E,
                    portNum,
                    PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                    SIP6_30_TTI_DEFAULT_EPORT_TABLE_FIELDS_PRT_PORT_LAN_ID_E, /* field name */
                    PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                    lanIdPtr);
            break;
        case CPSS_DIRECTION_EGRESS_E:
            PRV_CPSS_DXCH_PORT_CHECK_MAC(devNum, portNum);
            rc = prvCpssDxChReadTableEntryField(devNum,
                    CPSS_DXCH_SIP5_TABLE_HA_EGRESS_EPORT_1_E,
                    portNum,
                    PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                    SIP6_30_HA_EPORT_TABLE_1_FIELDS_HSR_PRP_LAN_ID_E, /* field name */
                    PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                    lanIdPtr);
            break;
        default: /* not support 'both' on 'Get' API */
            CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(direction);
    }

    return rc;
}

/**
* @internal cpssDxChHsrPrpPortPrpLanIdGet function
* @endinternal
*
* @brief  Get the PRP LAN ID of specific port.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum         - device number.
* @param[in] portNum        - The port number.
*                             For Ingress direction :
*                               APPLICABLE RANGE: 'Default eport' range.
*                             For Egress direction :
*                               APPLICABLE RANGE: eport range.
* @param[in] direction      - select direction (ingress/egress/Both)
*
* @param[out] lanIdPtr      - (pointer to) The LAN ID.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or portNum/direction
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChHsrPrpPortPrpLanIdGet
(
    IN  GT_U8                    devNum,
    IN  GT_PORT_NUM              portNum,
    IN  CPSS_DIRECTION_ENT       direction,
    OUT GT_U32                   *lanIdPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChHsrPrpPortPrpLanIdGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, direction ,lanIdPtr));

    rc = internal_cpssDxChHsrPrpPortPrpLanIdGet(devNum, portNum, direction ,lanIdPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, direction ,lanIdPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal cpssDxChHsrPrpPortPrpTreatWrongLanIdAsRctExistsSet function
* @endinternal
*
* @brief  Enable/Disable the PRP treat wrong LanId as RCT exists on specific port.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum         - device number.
* @param[in] portNum        - The port number.
*                               APPLICABLE RANGE: 'Default eport' range.
* @param[in] enable         - Indication to treat wrong LanId as RCT exists.
*                             GT_TRUE  - Do    treat wrong LanId as RCT exists.
*                             GT_FALSE - Don't treat wrong LanId as RCT exists.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or portNum
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
static GT_STATUS internal_cpssDxChHsrPrpPortPrpTreatWrongLanIdAsRctExistsSet
(
    IN  GT_U8                    devNum,
    IN  GT_PORT_NUM              portNum,
    IN  GT_BOOL                  enable
)
{
    GT_STATUS   rc;      /* return code */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E |
        CPSS_BOBCAT2_E | CPSS_BOBCAT3_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E |
        CPSS_ALDRIN2_E | CPSS_FALCON_E |  CPSS_AC5P_E |  CPSS_AC5X_E | CPSS_HARRIER_E);

    PRV_CPSS_DXCH_DEFAULT_EPORT_CHECK_MAC(devNum, portNum);

    /* write to post-tti-lookup-ingress-eport table */
    rc = prvCpssDxChWriteTableEntryField(devNum,
            CPSS_DXCH_SIP5_TABLE_PRE_TTI_LOOKUP_INGRESS_EPORT_E,
            portNum,
            PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
            SIP6_30_TTI_DEFAULT_EPORT_TABLE_FIELDS_PRT_TREAT_WRONG_LAN_ID_AS_RCT_EXISTS_E, /* field name */
            PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
            BOOL2BIT_MAC(enable));

    return rc;
}
/**
* @internal cpssDxChHsrPrpPortPrpTreatWrongLanIdAsRctExistsSet function
* @endinternal
*
* @brief  Enable/Disable the PRP treat wrong LanId as RCT exists on specific port.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum         - device number.
* @param[in] portNum        - The port number.
*                               APPLICABLE RANGE: 'Default eport' range.
* @param[in] enable         - Indication to treat wrong LanId as RCT exists.
*                             GT_TRUE  - Do    treat wrong LanId as RCT exists.
*                             GT_FALSE - Don't treat wrong LanId as RCT exists.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or portNum
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChHsrPrpPortPrpTreatWrongLanIdAsRctExistsSet
(
    IN  GT_U8                    devNum,
    IN  GT_PORT_NUM              portNum,
    IN  GT_BOOL                  enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChHsrPrpPortPrpTreatWrongLanIdAsRctExistsSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enable));

    rc = internal_cpssDxChHsrPrpPortPrpTreatWrongLanIdAsRctExistsSet(devNum, portNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal cpssDxChHsrPrpPortPrpTreatWrongLanIdAsRctExistsGet function
* @endinternal
*
* @brief  Get Enable/Disable for the PRP treat wrong LanId as RCT exists on specific port.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum         - device number.
* @param[in] portNum        - The port number.
*                               APPLICABLE RANGE: 'Default eport' range.
*
* @param[out] enablePtr     - (pointer to) Indication to treat wrong LanId as RCT exists.
*                             GT_TRUE  - Do    treat wrong LanId as RCT exists.
*                             GT_FALSE - Don't treat wrong LanId as RCT exists.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or portNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
static GT_STATUS internal_cpssDxChHsrPrpPortPrpTreatWrongLanIdAsRctExistsGet
(
    IN  GT_U8                    devNum,
    IN  GT_PORT_NUM              portNum,
    OUT GT_BOOL                  *enablePtr
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

    PRV_CPSS_DXCH_DEFAULT_EPORT_CHECK_MAC(devNum, portNum);

    /* read from post-tti-lookup-ingress-eport table */
    rc = prvCpssDxChReadTableEntryField(devNum,
            CPSS_DXCH_SIP5_TABLE_PRE_TTI_LOOKUP_INGRESS_EPORT_E,
            portNum,
            PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
            SIP6_30_TTI_DEFAULT_EPORT_TABLE_FIELDS_PRT_TREAT_WRONG_LAN_ID_AS_RCT_EXISTS_E, /* field name */
            PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
            &hwValue);

    *enablePtr = BIT2BOOL_MAC(hwValue);

    return rc;
}

/**
* @internal cpssDxChHsrPrpPortPrpTreatWrongLanIdAsRctExistsGet function
* @endinternal
*
* @brief  Get Enable/Disable for the PRP treat wrong LanId as RCT exists on specific port.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum         - device number.
* @param[in] portNum        - The port number.
*                               APPLICABLE RANGE: 'Default eport' range.
*
* @param[out] enablePtr     - (pointer to) Indication to treat wrong LanId as RCT exists.
*                             GT_TRUE  - Do    treat wrong LanId as RCT exists.
*                             GT_FALSE - Don't treat wrong LanId as RCT exists.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or portNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChHsrPrpPortPrpTreatWrongLanIdAsRctExistsGet
(
    IN  GT_U8                    devNum,
    IN  GT_PORT_NUM              portNum,
    OUT GT_BOOL                  *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChHsrPrpPortPrpTreatWrongLanIdAsRctExistsGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enablePtr));

    rc = internal_cpssDxChHsrPrpPortPrpTreatWrongLanIdAsRctExistsGet(devNum, portNum, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal portsBmpInfoGet function
* @endinternal
*
* @brief  Get the register address and start bit to specific bmpType type.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum         - device number.
* @param[in] type           - The type of bmpType.
*
* @param[out] regAddrPtr    - (pointer to) the register address.
* @param[out] startBitPtr   - (pointer to) the start bit.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong bmpType
*
*/
static GT_STATUS portsBmpInfoGet(
    IN GT_U8                               devNum,
    IN CPSS_DXCH_HSR_PRP_PORTS_BMP_ID_ENT  bmpType,
    OUT GT_U32      *regAddrPtr,
    OUT GT_U32      *startBitPtr
)
{
    GT_U32      regAddr  = GT_NA;
    GT_U32      startBit = 0;

    switch(bmpType)
    {
        case CPSS_DXCH_HSR_PRP_PORTS_BMP_FILTER_ID_1_E:
            regAddr  = PRV_DXCH_REG1_UNIT_L2I_MAC(devNum).hsrPrp.hsrPrpGlobalConfig0;
            startBit = 1;
            break;
        case CPSS_DXCH_HSR_PRP_PORTS_BMP_FILTER_ID_2_E:
            regAddr  = PRV_DXCH_REG1_UNIT_L2I_MAC(devNum).hsrPrp.hsrPrpGlobalConfig0;
            startBit = 11;
            break;
        case CPSS_DXCH_HSR_PRP_PORTS_BMP_FILTER_ID_3_E:
            regAddr  = PRV_DXCH_REG1_UNIT_L2I_MAC(devNum).hsrPrp.hsrPrpGlobalConfig0;
            startBit = 21;
            break;
        case CPSS_DXCH_HSR_PRP_PORTS_BMP_IGNORE_DDL_DECISION_E:
            regAddr  = PRV_DXCH_REG1_UNIT_L2I_MAC(devNum).hsrPrp.hsrPrpGlobalConfig3;
            startBit = 0;
            break;
        case CPSS_DXCH_HSR_PRP_PORTS_BMP_IGNORE_SOURCE_DUPLICATION_DISCARD_E:
            regAddr  = PRV_DXCH_REG1_UNIT_L2I_MAC(devNum).hsrPrp.hsrPrpGlobalConfig3;
            startBit = 10;
            break;
        case CPSS_DXCH_HSR_PRP_PORTS_BMP_SOURCE_IS_ME_CLEANING_E:
            regAddr  = PRV_DXCH_REG1_UNIT_L2I_MAC(devNum).hsrPrp.hsrPrpGlobalConfig3;
            startBit = 20;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(bmpType);
    }

    *regAddrPtr  = regAddr;
    *startBitPtr = startBit;

    return GT_OK;
}
/**
* @internal internal_cpssDxChHsrPrpPortsBmpSet function
* @endinternal
*
* @brief  Set ports bitmap for specific bitmap type.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum         - device number.
* @param[in] bmpType        - The bitmap type.
* @param[in] bmp            - The ports bitmap.
*                             APPLICABLE RANGE: combination of bits 0..9. (0..1023)
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum/bmpType
* @retval GT_OUT_OF_RANGE          - wrong bmp
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
static GT_STATUS internal_cpssDxChHsrPrpPortsBmpSet
(
    IN  GT_U8                               devNum,
    IN  CPSS_DXCH_HSR_PRP_PORTS_BMP_ID_ENT  bmpType,
    IN  CPSS_PORTS_SHORT_BMP_STC            bmp
)
{
    GT_STATUS   rc;      /* return code */
    GT_U32      regAddr; /* register address */
    GT_U32      startBit;/* start bit in the register */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E |
        CPSS_BOBCAT2_E | CPSS_BOBCAT3_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E |
        CPSS_ALDRIN2_E | CPSS_FALCON_E |  CPSS_AC5P_E |  CPSS_AC5X_E | CPSS_HARRIER_E);

    /* get info about the register address and the start bit */
    rc = portsBmpInfoGet(devNum,bmpType,&regAddr,&startBit);
    if(rc != GT_OK)
    {
        return rc;
    }

    PORTS_BMP_CHECK_MAC(bmp);

    return prvCpssHwPpSetRegField(devNum, regAddr, startBit, 10, bmp);
}

/**
* @internal cpssDxChHsrPrpPortsBmpSet function
* @endinternal
*
* @brief  Set ports bitmap for specific bitmap type.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum         - device number.
* @param[in] bmpType        - The bitmap type.
* @param[in] bmp            - The ports bitmap.
*                             APPLICABLE RANGE: combination of bits 0..9. (0..1023)
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum/bmpType
* @retval GT_OUT_OF_RANGE          - wrong bmp
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChHsrPrpPortsBmpSet
(
    IN  GT_U8                               devNum,
    IN  CPSS_DXCH_HSR_PRP_PORTS_BMP_ID_ENT  bmpType,
    IN  CPSS_PORTS_SHORT_BMP_STC            bmp
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChHsrPrpPortsBmpSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, bmpType, bmp));

    rc = internal_cpssDxChHsrPrpPortsBmpSet(devNum, bmpType, bmp);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, bmpType, bmp));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChHsrPrpPortsBmpGet function
* @endinternal
*
* @brief  Get ports bitmap for specific bitmap type.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum         - device number.
* @param[in] bmpType        - The bitmap type.
*
* @param[out] bmpPtr        - (pointer to)The ports bitmap.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum/bmpType
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
static GT_STATUS internal_cpssDxChHsrPrpPortsBmpGet
(
    IN  GT_U8                               devNum,
    IN  CPSS_DXCH_HSR_PRP_PORTS_BMP_ID_ENT  bmpType,
    OUT CPSS_PORTS_SHORT_BMP_STC            *bmpPtr
)
{
    GT_STATUS   rc;      /* return code */
    GT_U32      regAddr; /* register address */
    GT_U32      startBit;/* start bit in the register */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E |
        CPSS_BOBCAT2_E | CPSS_BOBCAT3_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E |
        CPSS_ALDRIN2_E | CPSS_FALCON_E |  CPSS_AC5P_E |  CPSS_AC5X_E | CPSS_HARRIER_E);
    CPSS_NULL_PTR_CHECK_MAC(bmpPtr);

    /* get info about the register address and the start bit */
    rc = portsBmpInfoGet(devNum,bmpType,&regAddr,&startBit);
    if(rc != GT_OK)
    {
        return rc;
    }

    return prvCpssHwPpGetRegField(devNum, regAddr, startBit, 10, bmpPtr);
}

/**
* @internal cpssDxChHsrPrpPortsBmpGet function
* @endinternal
*
* @brief  Get ports bitmap for specific bitmap type.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum         - device number.
* @param[in] bmpType        - The bitmap type.
*
* @param[out] bmpPtr        - (pointer to)The ports bitmap.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum/bmpType
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChHsrPrpPortsBmpGet
(
    IN  GT_U8                               devNum,
    IN  CPSS_DXCH_HSR_PRP_PORTS_BMP_ID_ENT  bmpType,
    OUT CPSS_PORTS_SHORT_BMP_STC            *bmpPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChHsrPrpPortsBmpGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, bmpType, bmpPtr));

    rc = internal_cpssDxChHsrPrpPortsBmpGet(devNum, bmpType, bmpPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, bmpType, bmpPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChHsrPrpHPortSrcEPortBaseSet function
* @endinternal
*
* @brief  Set source eport base for HPorts.
*         Defines the range of source ePorts that are mapped to HSR/PRP port.
*         The HSR/PRP port number is the 4 lsb of the source ePort number
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum         - device number.
* @param[in] baseEport      - The eport base.
*                             APPLICABLE RANGE: 0..65520 . limitation : in steps of 16 (0,16,32,..65520)
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum
*                                    on baseEport in range but violate the limitation.
* @retval GT_OUT_OF_RANGE          - wrong baseEport
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
static GT_STATUS internal_cpssDxChHsrPrpHPortSrcEPortBaseSet
(
    IN  GT_U8               devNum,
    IN  GT_PORT_NUM         baseEport
)
{
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E |
        CPSS_BOBCAT2_E | CPSS_BOBCAT3_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E |
        CPSS_ALDRIN2_E | CPSS_FALCON_E |  CPSS_AC5P_E |  CPSS_AC5X_E | CPSS_HARRIER_E);

    if(baseEport >= BIT_16)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, "baseEport[0x%8.8x] limited to [0..0x%8.8x]",
            baseEport,(BIT_16-1));
    }

    if(baseEport & 0xF)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "baseEport [0x%8.8x] must be in steps of 16 (0x10)",
            baseEport);
    }

    return prvCpssHwPpSetRegField(devNum,
        PRV_DXCH_REG1_UNIT_L2I_MAC(devNum).hsrPrp.hsrPrpGlobalConfig1,
        0,16, baseEport);
}

/**
* @internal cpssDxChHsrPrpHPortSrcEPortBaseSet function
* @endinternal
*
* @brief  Set source eport base for HPorts.
*         Defines the range of source ePorts that are mapped to HSR/PRP port.
*         The HSR/PRP port number is the 4 lsb of the source ePort number
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum         - device number.
* @param[in] baseEport      - The eport base.
*                             APPLICABLE RANGE: 0..65520 . limitation : in steps of 16 (0,16,32,..65520)
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum
*                                    on baseEport in range but violate the limitation.
* @retval GT_OUT_OF_RANGE          - wrong baseEport
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChHsrPrpHPortSrcEPortBaseSet
(
    IN  GT_U8               devNum,
    IN  GT_PORT_NUM         baseEport
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChHsrPrpHPortSrcEPortBaseSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, baseEport));

    rc = internal_cpssDxChHsrPrpHPortSrcEPortBaseSet(devNum, baseEport);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, baseEport));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChHsrPrpHPortSrcEPortBaseGet function
* @endinternal
*
* @brief  Get source eport base for HPorts.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum         - device number.
*
* @param[out] baseEportPtr  - (pointer to)The eport base.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
static GT_STATUS internal_cpssDxChHsrPrpHPortSrcEPortBaseGet
(
    IN  GT_U8               devNum,
    OUT GT_PORT_NUM         *baseEportPtr
)
{
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E |
        CPSS_BOBCAT2_E | CPSS_BOBCAT3_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E |
        CPSS_ALDRIN2_E | CPSS_FALCON_E |  CPSS_AC5P_E |  CPSS_AC5X_E | CPSS_HARRIER_E);
    CPSS_NULL_PTR_CHECK_MAC(baseEportPtr);

    return prvCpssHwPpGetRegField(devNum,
        PRV_DXCH_REG1_UNIT_L2I_MAC(devNum).hsrPrp.hsrPrpGlobalConfig1,
        0,16, baseEportPtr);
}

/**
* @internal cpssDxChHsrPrpHPortSrcEPortBaseGet function
* @endinternal
*
* @brief  Get source eport base for HPorts.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum         - device number.
*
* @param[out] baseEportPtr  - (pointer to)The eport base.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChHsrPrpHPortSrcEPortBaseGet
(
    IN  GT_U8               devNum,
    OUT GT_PORT_NUM         *baseEportPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChHsrPrpHPortSrcEPortBaseGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, baseEportPtr));

    rc = internal_cpssDxChHsrPrpHPortSrcEPortBaseGet(devNum, baseEportPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, baseEportPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChHsrPrpCounterGet function
* @endinternal
*
* @brief  Get specific global Counter.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum         - device number.
* @param[in] counterType    - The counter type.
*
* @param[out] counterPtr    - (pointer to)The counter.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum/counterType
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
static GT_STATUS internal_cpssDxChHsrPrpCounterGet
(
    IN  GT_U8                               devNum,
    IN  CPSS_DXCH_HSR_PRP_COUNTER_TYPE_ENT  counterType,
    OUT GT_U64                              *counterPtr
)
{
    GT_U32      regAddr; /* register address */
    GT_U32      startBit;/* start bit in the register */
    GT_U32      numOfBits;/* number of bits for the field */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E |
        CPSS_BOBCAT2_E | CPSS_BOBCAT3_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E |
        CPSS_ALDRIN2_E | CPSS_FALCON_E |  CPSS_AC5P_E |  CPSS_AC5X_E | CPSS_HARRIER_E);
    CPSS_NULL_PTR_CHECK_MAC(counterPtr);

    switch(counterType)
    {
        case CPSS_DXCH_HSR_PRP_COUNTER_TYPE_PNT_NUM_OF_READY_ENTRIES_E:
            regAddr  = PRV_DXCH_REG1_UNIT_L2I_MAC(devNum).hsrPrp.hsrPrpPntCounter;
            startBit = 0;
            numOfBits = 10;
            break;
        case CPSS_DXCH_HSR_PRP_COUNTER_OVERRIDEN_NON_EXPIRED_ENTRIES_E:
            regAddr  = PRV_DXCH_REG1_UNIT_FDB_MAC(devNum).FDBCore.hsrPrp.hsrPrpOverridenNonExpiredCounter;
            startBit = 0;
            numOfBits = 32;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(counterType);
    }

    return prvCpssPortGroupsBmpCounterSummary(devNum, CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                                  regAddr, startBit, numOfBits,
                                                  NULL, counterPtr);
}

/**
* @internal cpssDxChHsrPrpCounterGet function
* @endinternal
*
* @brief  Get specific global Counter.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum         - device number.
* @param[in] counterType    - The counter type.
*
* @param[out] counterPtr    - (pointer to)The counter.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum/counterType
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChHsrPrpCounterGet
(
    IN  GT_U8                               devNum,
    IN  CPSS_DXCH_HSR_PRP_COUNTER_TYPE_ENT  counterType,
    OUT GT_U64                              *counterPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChHsrPrpCounterGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, counterType , counterPtr));

    rc = internal_cpssDxChHsrPrpCounterGet(devNum, counterType , counterPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, counterType , counterPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChHsrPrpPntNumOfReadyEntriesThresholdSet function
* @endinternal
*
* @brief  Set threshold for number of ready entries in PNT.
*         The event CPSS_PP_HSR_PRP_PNT_NUM_OF_READY_ENTRIES_E is triggered when
*         <PNT Number of Ready Entries> is equal or below this value.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum         - device number.
* @param[in] threshold      - The threshold.
*                             APPLICABLE RANGE: 0..1023
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_OUT_OF_RANGE          - out of range threshold
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
static GT_STATUS internal_cpssDxChHsrPrpPntNumOfReadyEntriesThresholdSet
(
    IN  GT_U8               devNum,
    IN  GT_U32              threshold
)
{
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E |
        CPSS_BOBCAT2_E | CPSS_BOBCAT3_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E |
        CPSS_ALDRIN2_E | CPSS_FALCON_E |  CPSS_AC5P_E |  CPSS_AC5X_E | CPSS_HARRIER_E);

    if(threshold >= BIT_10)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, "threshold[0x%8.8x] limited to [0..0x%8.8x]",
            threshold,(BIT_10-1));
    }

    return prvCpssHwPpSetRegField(devNum,
        PRV_DXCH_REG1_UNIT_L2I_MAC(devNum).hsrPrp.hsrPrpPntCounterThreshold,
        0,10, threshold);
}


/**
* @internal cpssDxChHsrPrpPntNumOfReadyEntriesThresholdSet function
* @endinternal
*
* @brief  Set threshold for number of ready entries in PNT.
*         The event CPSS_PP_HSR_PRP_PNT_NUM_OF_READY_ENTRIES_E is triggered when
*         <PNT Number of Ready Entries> is equal or below this value.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum         - device number.
* @param[in] threshold      - The threshold.
*                             APPLICABLE RANGE: 0..1023
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_OUT_OF_RANGE          - out of range threshold
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChHsrPrpPntNumOfReadyEntriesThresholdSet
(
    IN  GT_U8               devNum,
    IN  GT_U32              threshold
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChHsrPrpPntNumOfReadyEntriesThresholdSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, threshold));

    rc = internal_cpssDxChHsrPrpPntNumOfReadyEntriesThresholdSet(devNum, threshold);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, threshold));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChHsrPrpPntNumOfReadyEntriesThresholdGet function
* @endinternal
*
* @brief  Get threshold for number of ready entries in PNT.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum         - device number.
*
* @param[out] thresholdPtr  - (pointer to) The threshold.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
static GT_STATUS internal_cpssDxChHsrPrpPntNumOfReadyEntriesThresholdGet
(
    IN  GT_U8               devNum,
    OUT GT_U32              *thresholdPtr
)
{
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E |
        CPSS_BOBCAT2_E | CPSS_BOBCAT3_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E |
        CPSS_ALDRIN2_E | CPSS_FALCON_E |  CPSS_AC5P_E |  CPSS_AC5X_E | CPSS_HARRIER_E);
    CPSS_NULL_PTR_CHECK_MAC(thresholdPtr);

    return prvCpssHwPpGetRegField(devNum,
        PRV_DXCH_REG1_UNIT_L2I_MAC(devNum).hsrPrp.hsrPrpPntCounterThreshold,
        0,10, thresholdPtr);
}


/**
* @internal cpssDxChHsrPrpPntNumOfReadyEntriesThresholdGet function
* @endinternal
*
* @brief  Get threshold for number of ready entries in PNT.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum         - device number.
*
* @param[out] thresholdPtr  - (pointer to) The threshold.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChHsrPrpPntNumOfReadyEntriesThresholdGet
(
    IN  GT_U8               devNum,
    OUT GT_U32              *thresholdPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChHsrPrpPntNumOfReadyEntriesThresholdGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, thresholdPtr));

    rc = internal_cpssDxChHsrPrpPntNumOfReadyEntriesThresholdGet(devNum, thresholdPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, thresholdPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChHsrPrpHPortEntrySet function
* @endinternal
*
* @brief  Set the HPort entry info.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum         - device number.
* @param[in] hPort          - The HPort index.
*                             APPLICABLE RANGE: 0..9
* @param[in] infoPtr        - (pointer to) The entry info.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or hPort or other wrong parameter
* @retval GT_OUT_OF_RANGE          - out of range parameter
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
static GT_STATUS internal_cpssDxChHsrPrpHPortEntrySet
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          hPort,
    IN  CPSS_DXCH_HSR_PRP_HPORT_ENTRY_STC    *infoPtr
)
{
    GT_STATUS   rc;      /* return code */
    GT_U32      hwValue[2]; /* value to write to HW */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E |
        CPSS_BOBCAT2_E | CPSS_BOBCAT3_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E |
        CPSS_ALDRIN2_E | CPSS_FALCON_E |  CPSS_AC5P_E |  CPSS_AC5X_E | CPSS_HARRIER_E);
    CPSS_NULL_PTR_CHECK_MAC(infoPtr);

    if(hPort >= H_PORTS_NUM_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "hPort[%d] limited to [0..%d]",
            hPort,H_PORTS_NUM_CNS-1);
    }

    hwValue[0] = 0;
    hwValue[1] = 0;

    /* L2i : start 'register 0' of the entry */

    switch(infoPtr->pntLookupMode)
    {
        case CPSS_DXCH_HSR_PRP_PNT_LOOKUP_MODE_SKIP_E              :
            /* value 0 */
            break;
        case CPSS_DXCH_HSR_PRP_PNT_LOOKUP_MODE_SRC_AND_LEARN_E     :
            hwValue[0] |= 1;
            break;
        case CPSS_DXCH_HSR_PRP_PNT_LOOKUP_MODE_DST_AND_SRC_E       :
            hwValue[0] |= 2;
            break;
        case CPSS_DXCH_HSR_PRP_PNT_LOOKUP_MODE_SRC_E               :
            hwValue[0] |= 3;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(infoPtr->pntLookupMode);
    }

    switch(infoPtr->fdbDdlMode)
    {
        case CPSS_DXCH_HSR_PRP_FDB_DDL_MODE_SKIP_E:
            /* value 0 */
            break;
        case CPSS_DXCH_HSR_PRP_FDB_DDL_MODE_DDL_E:
            hwValue[0] |= 1 << 2;
            break;
        case CPSS_DXCH_HSR_PRP_FDB_DDL_MODE_FDB_E:
            hwValue[0] |= 2 << 2;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(infoPtr->fdbDdlMode);
    }

    PORTS_BMP_CHECK_MAC(infoPtr->defaultForwardingBmp);
    hwValue[0] |= infoPtr->defaultForwardingBmp << 4;/* 10 bits */

    if(GT_TRUE == infoPtr->pntLearnTaggedPackets)
    {
        hwValue[0] |= 1 << 14;
    }

    if(GT_TRUE == infoPtr->acceptSamePort)
    {
        hwValue[0] |= 1 << 15;
    }

    switch(infoPtr->lreRingPortType)
    {
        case CPSS_DXCH_HSR_PRP_LRE_PORT_TYPE_A_E:
            /* value 0 */
            break;
        case CPSS_DXCH_HSR_PRP_LRE_PORT_TYPE_B_E:
            hwValue[0] |= 1 << 16;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(infoPtr->lreRingPortType);
    }

    if(infoPtr->lreInstance >= BIT_2)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, "lreInstance[%d] limited to [0..%d]",
            infoPtr->lreInstance,(BIT_2-1));
    }
    hwValue[0] |= infoPtr->lreInstance << 17;

    /* L2i : start 'register 1' of the entry */

    PORTS_BMP_CHECK_MAC(infoPtr->srcIsMeBmp);
    hwValue[1] |= infoPtr->srcIsMeBmp;/* 10 bits */

    PORTS_BMP_CHECK_MAC(infoPtr->srcDuplicateDiscardBmp);
    hwValue[1] |= infoPtr->srcDuplicateDiscardBmp << 10;/* 10 bits */

    PORTS_BMP_CHECK_MAC(infoPtr->destToMeForwardingBmp);
    hwValue[1] |= infoPtr->destToMeForwardingBmp << 20;/* 10 bits */

    /* now we will write the 2 registers */
    rc = prvCpssHwPpWriteRegister(devNum,
        PRV_DXCH_REG1_UNIT_L2I_MAC(devNum).hsrPrp.hsrPrpSourcePortConfig0[hPort],
        hwValue[0]);
    if(rc != GT_OK)
    {
        return rc;
    }

    return prvCpssHwPpWriteRegister(devNum,
        PRV_DXCH_REG1_UNIT_L2I_MAC(devNum).hsrPrp.hsrPrpSourcePortConfig1[hPort],
        hwValue[1]);
}

/**
* @internal cpssDxChHsrPrpHPortEntrySet function
* @endinternal
*
* @brief  Set the HPort entry info.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum         - device number.
* @param[in] hPort          - The HPort index.
*                             APPLICABLE RANGE: 0..9
* @param[in] infoPtr        - (pointer to) The entry info.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or hPort or other wrong parameter
* @retval GT_OUT_OF_RANGE          - out of range parameter
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChHsrPrpHPortEntrySet
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          hPort,
    IN  CPSS_DXCH_HSR_PRP_HPORT_ENTRY_STC    *infoPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChHsrPrpHPortEntrySet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, hPort , infoPtr));

    rc = internal_cpssDxChHsrPrpHPortEntrySet(devNum, hPort , infoPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, hPort , infoPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChHsrPrpHPortEntryGet function
* @endinternal
*
* @brief  Get the HPort entry info and counters.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum         - device number.
* @param[in] hPort          - The HPort index.
*                             APPLICABLE RANGE: 0..9
*
* @param[out] infoPtr        - (pointer to) The entry info.
*                               NOTE: if NULL ignored. allow to get only counter(s).
* @param[out] firstCounterPtr - (pointer to) The 'first' counter.
*                               NOTE: if NULL ignored. allow to skip the counter.
*                               The counter is Read only and clear after read
* @param[out] secondCounterPtr - (pointer to) The 'second' counter.
*                               NOTE: if NULL ignored. allow to skip the counter.
*                               The counter is Read only and clear after read
* @param[out] thirdCounterPtr - (pointer to) The 'third' counter.
*                               NOTE: if NULL ignored. allow to skip the counter.
*                               The counter is Read only and clear after read
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or hPort or other wrong parameter
* @retval GT_BAD_PTR               - if ALL the pointers are NULL
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
static GT_STATUS internal_cpssDxChHsrPrpHPortEntryGet
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          hPort,
    OUT CPSS_DXCH_HSR_PRP_HPORT_ENTRY_STC    *infoPtr,
    OUT GT_U64                          *firstCounterPtr,
    OUT GT_U64                          *secondCounterPtr,
    OUT GT_U64                          *thirdCounterPtr
)
{
    GT_STATUS   rc;      /* return code */
    GT_U32      hwValue[2]; /* value to read from HW */
    GT_U32      fieldValue;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E |
        CPSS_BOBCAT2_E | CPSS_BOBCAT3_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E |
        CPSS_ALDRIN2_E | CPSS_FALCON_E |  CPSS_AC5P_E |  CPSS_AC5X_E | CPSS_HARRIER_E);

    if(infoPtr == NULL          && firstCounterPtr == NULL &&
       secondCounterPtr == NULL && thirdCounterPtr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PTR,
            "The Api not support NULL on all the pointers (at least one must be valid)");
    }

    if(hPort >= H_PORTS_NUM_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "hPort[%d] limited to [0..%d]",
            hPort,H_PORTS_NUM_CNS-1);
    }

    if(infoPtr)
    {
        /* now we will read the 2 registers */
        rc = prvCpssHwPpReadRegister(devNum,
            PRV_DXCH_REG1_UNIT_L2I_MAC(devNum).hsrPrp.hsrPrpSourcePortConfig0[hPort],
            &hwValue[0]);
        if(rc != GT_OK)
        {
            return rc;
        }

        rc = prvCpssHwPpReadRegister(devNum,
            PRV_DXCH_REG1_UNIT_L2I_MAC(devNum).hsrPrp.hsrPrpSourcePortConfig1[hPort],
            &hwValue[1]);
        if(rc != GT_OK)
        {
            return rc;
        }

        fieldValue = hwValue[0] & 0x3;
        infoPtr->pntLookupMode = fieldValue;

        fieldValue = (hwValue[0]>>2) & 0x3;
        infoPtr->fdbDdlMode = fieldValue;

        if(fieldValue == 3)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE,
                "unexpected value[%d] for the field[infoPtr->fdbDdlMode]",
                fieldValue);
        }

        fieldValue = (hwValue[0]>>4) & 0x3FF;/* 10 bits */
        infoPtr->defaultForwardingBmp = fieldValue;

        fieldValue = (hwValue[0]>>14) & 0x1;
        infoPtr->pntLearnTaggedPackets = fieldValue;

        fieldValue = (hwValue[0]>>15) & 0x1;
        infoPtr->acceptSamePort = fieldValue;

        fieldValue = (hwValue[0]>>16) & 0x1;
        infoPtr->lreRingPortType = fieldValue;

        fieldValue = (hwValue[0]>>17) & 0x3;
        infoPtr->lreInstance  = fieldValue;

        /* L2i : start 'register 1' of the entry */
        fieldValue = hwValue[1] & 0x3FF;/* 10 bits */
        infoPtr->srcIsMeBmp = fieldValue;

        fieldValue = (hwValue[1]>>10) & 0x3FF;/* 10 bits */
        infoPtr->srcDuplicateDiscardBmp = fieldValue;

        fieldValue = (hwValue[1]>>20) & 0x3FF;/* 10 bits */
        infoPtr->destToMeForwardingBmp = fieldValue;
    }

    if(firstCounterPtr)
    {
        rc = prvCpssPortGroupsBmpCounterSummary(devNum, CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
            PRV_DXCH_REG1_UNIT_FDB_MAC(devNum).FDBCore.hsrPrp.hsrPrpCountersFirst[hPort],
            0, 32, NULL, firstCounterPtr);
        if(rc != GT_OK)
        {
            return rc;
        }
    }
    if(secondCounterPtr)
    {
        rc = prvCpssPortGroupsBmpCounterSummary(devNum, CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
            PRV_DXCH_REG1_UNIT_FDB_MAC(devNum).FDBCore.hsrPrp.hsrPrpCountersSecond[hPort],
            0, 32, NULL, secondCounterPtr);
        if(rc != GT_OK)
        {
            return rc;
        }
    }
    if(thirdCounterPtr)
    {
        rc = prvCpssPortGroupsBmpCounterSummary(devNum, CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
            PRV_DXCH_REG1_UNIT_FDB_MAC(devNum).FDBCore.hsrPrp.hsrPrpCountersThird[hPort],
            0, 32, NULL, thirdCounterPtr);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    return GT_OK;
}

/**
* @internal cpssDxChHsrPrpHPortEntryGet function
* @endinternal
*
* @brief  Get the HPort entry info and counters.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum         - device number.
* @param[in] hPort          - The HPort index.
*                             APPLICABLE RANGE: 0..9
*
* @param[out] infoPtr        - (pointer to) The entry info.
*                               NOTE: if NULL ignored. allow to get only counter(s).
* @param[out] firstCounterPtr - (pointer to) The 'first' counter.
*                               NOTE: if NULL ignored. allow to skip the counter.
*                               The counter is Read only and clear after read
* @param[out] secondCounterPtr - (pointer to) The 'second' counter.
*                               NOTE: if NULL ignored. allow to skip the counter.
*                               The counter is Read only and clear after read
* @param[out] thirdCounterPtr - (pointer to) The 'third' counter.
*                               NOTE: if NULL ignored. allow to skip the counter.
*                               The counter is Read only and clear after read
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or hPort or other wrong parameter
* @retval GT_BAD_PTR               - if ALL the pointers are NULL
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChHsrPrpHPortEntryGet
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          hPort,
    OUT CPSS_DXCH_HSR_PRP_HPORT_ENTRY_STC    *infoPtr,
    OUT GT_U64                          *firstCounterPtr,
    OUT GT_U64                          *secondCounterPtr,
    OUT GT_U64                          *thirdCounterPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChHsrPrpHPortEntryGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, hPort , infoPtr , firstCounterPtr , secondCounterPtr , thirdCounterPtr));

    rc = internal_cpssDxChHsrPrpHPortEntryGet(devNum, hPort , infoPtr , firstCounterPtr , secondCounterPtr , thirdCounterPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, hPort , infoPtr , firstCounterPtr , secondCounterPtr , thirdCounterPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal pnpCheckIsInvalidEntry function
* @endinternal
*
* @brief  internal function to check if entry is invalid.
*         need to check the valid bit and the age bit
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum         - device number.
* @param[in] index          - The index in the table.
*                             APPLICABLE RANGE: 0..127
* @param[out] isInvalidPtr  - (pointer to) indication that the entry is invalid
*                             GT_FALSE - the entry is   valid (not invalid)
*                             GT_TRUE  - the entry is invalid
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
*
*/
static GT_STATUS pnpCheckIsInvalidEntry(
    IN  GT_U8               devNum,
    IN  GT_U32              index,
    OUT GT_BOOL             *isInvalidPtr
)
{
    GT_STATUS   rc;         /* return code */
    GT_U32      fieldValue;

    *isInvalidPtr = GT_FALSE;

    /* read the valid bit */
    rc = prvCpssHwPpReadRegister(devNum,
        PRV_DXCH_REG1_UNIT_L2I_MAC(devNum).hsrPrp.hsrPrpPntValid[index],
        &fieldValue);
    if(rc != GT_OK)
    {
        return rc;
    }

    if(fieldValue & 0x1)
    {
        /* valid bit is set */
        return GT_OK;
    }

    /* read the age bit */
    rc = prvCpssHwPpReadRegister(devNum,
        PRV_DXCH_REG1_UNIT_L2I_MAC(devNum).hsrPrp.hsrPrpPntEntryPart1[index],
        &fieldValue);
    if(rc != GT_OK)
    {
        return rc;
    }

    if((fieldValue >> 16) & 0x1)
    {
        /* age bit is set , meaning that invalid entry hit traffic and is still active ! */
        return GT_OK;
    }

    *isInvalidPtr = GT_TRUE;
    return GT_OK;
}

/**
* @internal pntInvalidateEntry function
* @endinternal
*
* @brief  internal function to set entry as invalid.
*         it require specific sequence , that may result in additional iteration.
*         as update entry under traffic may re-alive an invalid entry
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum         - device number.
* @param[in] index          - The index in the table.
*                             APPLICABLE RANGE: 0..127
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
*
*/
static GT_STATUS pntInvalidateEntry(
    IN  GT_U8               devNum,
    IN  GT_U32              index
)
{
    GT_STATUS   rc;         /* return code */
    GT_U32      fieldValue;

    /* Set set ready = 0 , valid = 0 then age = 0 */
    fieldValue = 0;

    /* set the ready bit to 0 (reset the full word) */
    rc = prvCpssHwPpWriteRegister(devNum,
        PRV_DXCH_REG1_UNIT_L2I_MAC(devNum).hsrPrp.hsrPrpPntReady[index],
        fieldValue);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* set the valid bit to 0 (reset the full word) */
    rc = prvCpssHwPpWriteRegister(devNum,
        PRV_DXCH_REG1_UNIT_L2I_MAC(devNum).hsrPrp.hsrPrpPntValid[index],
        fieldValue);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* write the age bit (reset the full word) */
    rc = prvCpssHwPpWriteRegister(devNum,
        PRV_DXCH_REG1_UNIT_L2I_MAC(devNum).hsrPrp.hsrPrpPntEntryPart1[index],
        fieldValue);

    return rc;
}


/**
* @internal pnpSetReadyEntry function
* @endinternal
*
* @brief  internal function to set the streamId and to set the entry as 'ready'
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum         - device number.
* @param[in] index          - The index in the table.
*                             APPLICABLE RANGE: 0..127
* @param[in] streamId      - The stream ID of the entry
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
*
*/
static GT_STATUS pnpSetReadyEntry(
    IN  GT_U8               devNum,
    IN  GT_U32              index,
    IN  GT_U32              streamId
)
{
    GT_STATUS   rc;         /* return code */

    /* set the streamId */
    rc = prvCpssHwPpWriteRegister(devNum,
        PRV_DXCH_REG1_UNIT_L2I_MAC(devNum).hsrPrp.hsrPrpPntEntryPart2[index],
        streamId);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* set the ready bit to 1 */
    return prvCpssHwPpWriteRegister(devNum,
        PRV_DXCH_REG1_UNIT_L2I_MAC(devNum).hsrPrp.hsrPrpPntReady[index],
        1);
}
/**
* @internal pnpDoAgingEntry function
* @endinternal
*
* @brief  internal function to age PNT entry :
*       if entry already aged (age bit == 0) --> invalidate it
*       else age the entry (set age bit = 0)
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum         - device number.
* @param[in] index          - The index in the table.
*                             APPLICABLE RANGE: 0..127
*
* @param[out] agedOutPtr    - (pointer to) Indication that the entry was invalidated (aged out)
*                              ignored if NULL.
*                              relevant when the operation == 'DO_AGING'.
*                              GT_TRUE  - the entry was     aged out. (not valid any more)
*                              GT_FALSE - the entry was not aged out. (still valid)
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_TIMEOUT               - failed on timeout to invalidate the entry.
*                                    relevant when operation == 'INVALIDATE'
*
*/
static GT_STATUS pnpDoAgingEntry(
    IN  GT_U8               devNum,
    IN  GT_U32              index,
    OUT GT_BOOL             *agedOutPtr
)
{
    GT_STATUS   rc;         /* return code */
    GT_U32      hwValue;    /* value to read/write from/to HW */

    *agedOutPtr = GT_FALSE;
    /* read the ageBit */
    rc = prvCpssHwPpReadRegister(devNum,
        PRV_DXCH_REG1_UNIT_L2I_MAC(devNum).hsrPrp.hsrPrpPntEntryPart1[index],
        &hwValue);
    if(rc != GT_OK)
    {
        return rc;
    }

    if((hwValue >> 16) & 0x1)
    {
        /* set to 0 the bit */
        hwValue &= ~BIT_16;

        /* write the bit back */
        return prvCpssHwPpWriteRegister(devNum,
            PRV_DXCH_REG1_UNIT_L2I_MAC(devNum).hsrPrp.hsrPrpPntEntryPart1[index],
            hwValue);
    }

    /* the ageBit is already 0 --> need to age it out */

    /* set the valid bit to 0 (reset the full word) */
    rc = prvCpssHwPpWriteRegister(devNum,
        PRV_DXCH_REG1_UNIT_L2I_MAC(devNum).hsrPrp.hsrPrpPntValid[index],
        0);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* read the ageBit */
    rc = prvCpssHwPpReadRegister(devNum,
        PRV_DXCH_REG1_UNIT_L2I_MAC(devNum).hsrPrp.hsrPrpPntEntryPart1[index],
        &hwValue);
    if(rc != GT_OK)
    {
        return rc;
    }

    if((hwValue >> 16) & 0x1)
    {
        /* The entry is still alive , the traffic just now refreshed it ! */
        /* set the valid bit to 1 !!! */
        rc = prvCpssHwPpWriteRegister(devNum,
            PRV_DXCH_REG1_UNIT_L2I_MAC(devNum).hsrPrp.hsrPrpPntValid[index],
            1);
        if(rc != GT_OK)
        {
            return rc;
        }
    }
    else
    {
        *agedOutPtr = GT_TRUE;
    }

    return GT_OK;
}

/**
* @internal pnpSetEntry function
* @endinternal
*
* @brief  internal function to fill PNT entry info , and to validate the entry.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum         - device number.
* @param[in] index          - The index in the table.
*                             APPLICABLE RANGE: 0..127
* @param[in] infoPtr        - (pointer to) The entry info.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
*
*/
static GT_STATUS pnpSetEntry(
    IN  GT_U8                           devNum,
    IN  GT_U32                          index,
    IN  CPSS_DXCH_HSR_PRP_PNT_ENTRY_STC      *infoPtr
)
{
    GT_STATUS   rc;         /* return code */
    GT_U32      hwValue[3]; /* value to write to HW */

    /*full word*/
    hwValue[0] = GT_HW_MAC_LOW32(&infoPtr->macAddr);
    /* lower 16 bits */
    hwValue[1]  = GT_HW_MAC_HIGH16 (&infoPtr->macAddr);
    /* higher 16 bits */
    hwValue[1] |= BOOL2BIT_MAC(infoPtr->ageBit) << 16;
    hwValue[1] |= infoPtr->lreInstance << 17;
    hwValue[1] |= infoPtr->destBmp << 19;
    hwValue[1] |= BOOL2BIT_MAC(infoPtr->untagged) << 29;

    /* only 16 bits */
    hwValue[2] = infoPtr->streamId;

    /* write the 3 words */
    rc = prvCpssHwPpWriteRegister(devNum,
        PRV_DXCH_REG1_UNIT_L2I_MAC(devNum).hsrPrp.hsrPrpPntEntryPart0[index],
        hwValue[0]);
    if(rc != GT_OK)
    {
        return rc;
    }

    rc = prvCpssHwPpWriteRegister(devNum,
        PRV_DXCH_REG1_UNIT_L2I_MAC(devNum).hsrPrp.hsrPrpPntEntryPart1[index],
        hwValue[1]);
    if(rc != GT_OK)
    {
        return rc;
    }

    rc = prvCpssHwPpWriteRegister(devNum,
        PRV_DXCH_REG1_UNIT_L2I_MAC(devNum).hsrPrp.hsrPrpPntEntryPart2[index],
        hwValue[2]);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* validate the entry */
    return prvCpssHwPpWriteRegister(devNum,
        PRV_DXCH_REG1_UNIT_L2I_MAC(devNum).hsrPrp.hsrPrpPntValid[index],
        1);
}

/**
* @internal internal_cpssDxChHsrPrpPntEntrySet function
* @endinternal
*
* @brief  Set the PNT entry info.
*       NOTE: for 'Aging' entries , see API : cpssDxChHsrPrpPntAgingApply(...)
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum         - device number.
* @param[in] index          - The index in the table.
*                             APPLICABLE RANGE: 0..127
* @param[in] infoPtr        - (pointer to) The entry info.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or index or other wrong parameter
* @retval GT_OUT_OF_RANGE          - out of range parameter
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_ALLOWED           - the operation not allowed.
*                                    relevant when operation == 'SET_READY' but entry is not invalid.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
static GT_STATUS internal_cpssDxChHsrPrpPntEntrySet
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          index,
    IN  CPSS_DXCH_HSR_PRP_PNT_OPER_ENT       operation,
    IN  CPSS_DXCH_HSR_PRP_PNT_ENTRY_STC      *infoPtr
)
{
    GT_STATUS   rc;         /* return code */
    GT_BOOL     isInvalid;/* indication that entry is invalid */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E |
        CPSS_BOBCAT2_E | CPSS_BOBCAT3_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E |
        CPSS_ALDRIN2_E | CPSS_FALCON_E |  CPSS_AC5P_E |  CPSS_AC5X_E | CPSS_HARRIER_E);

    CPSS_NULL_PTR_CHECK_MAC(infoPtr);

    if(index >= PNT_NUM_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "index[%d] limited to [0..%d]",
            index,PNT_NUM_CNS-1);
    }

    switch(operation)
    {
        case CPSS_DXCH_HSR_PRP_PNT_OPER_SET_READY_E:
            /* check if the entry is invalid */
            rc = pnpCheckIsInvalidEntry(devNum,index,&isInvalid);
            if(rc != GT_OK)
            {
                return rc;
            }

            if(isInvalid == GT_FALSE)
            {
                /* valid bit is set -->  we can't make the entry 'ready' */
                /* application need to choose other entry , or invalidate this entry */
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_ALLOWED, "can't set 'Ready' on 'Valid' entry");
            }

            if(infoPtr->streamId >= BIT_16)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, "infoPtr->streamId[%d] limited to [0..%d]",
                    infoPtr->streamId,BIT_16-1);
            }

            rc = pnpSetReadyEntry(devNum,index,infoPtr->streamId);
            break;

        case CPSS_DXCH_HSR_PRP_PNT_OPER_SET_INVALID_E:
            rc = pntInvalidateEntry(devNum,index);
            break;

        case CPSS_DXCH_HSR_PRP_PNT_OPER_SET_VALID_E:
            if(infoPtr->streamId >= BIT_16)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, "infoPtr->streamId[%d] limited to [0..%d]",
                    infoPtr->streamId,BIT_16-1);
            }
            if(infoPtr->destBmp >= BIT_10)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, "infoPtr->destBmp[0x%4.4x] limited to [0..0x%4.4x]",
                    infoPtr->destBmp,BIT_10-1);
            }
            if(infoPtr->lreInstance >= BIT_2)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, "infoPtr->lreInstance[%d] limited to [0..%d]",
                    infoPtr->lreInstance,BIT_2-1);
            }

            rc = pntInvalidateEntry(devNum,index);
            if(rc != GT_OK)
            {
                return rc;
            }

            rc = pnpSetEntry(devNum,index,infoPtr);
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(operation);
    }

    return rc;
}

/**
* @internal cpssDxChHsrPrpPntEntrySet function
* @endinternal
*
* @brief  Set the PNT entry info.
*       NOTE: for 'Aging' entries , see API : cpssDxChHsrPrpPntAgingApply(...)
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum         - device number.
* @param[in] index          - The index in the table.
*                             APPLICABLE RANGE: 0..127
* @param[in] infoPtr        - (pointer to) The entry info.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or index or other wrong parameter
* @retval GT_OUT_OF_RANGE          - out of range parameter
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_ALLOWED           - the operation not allowed.
*                                    relevant when operation == 'SET_READY' but entry is not invalid.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChHsrPrpPntEntrySet
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          index,
    IN  CPSS_DXCH_HSR_PRP_PNT_OPER_ENT       operation,
    IN  CPSS_DXCH_HSR_PRP_PNT_ENTRY_STC      *infoPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChHsrPrpPntEntrySet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, index , operation , infoPtr));

    rc = internal_cpssDxChHsrPrpPntEntrySet(devNum, index , operation , infoPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, index , operation , infoPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChHsrPrpPntEntryGet function
* @endinternal
*
* @brief  Get the PNT entry info.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum         - device number.
* @param[in] index          - The index in the table.
*                             APPLICABLE RANGE: 0..127
*
* @param[out] statePtr      - (pointer to) The state of the entry.
* @param[out] infoPtr       - (pointer to) The entry info.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or index
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
static GT_STATUS internal_cpssDxChHsrPrpPntEntryGet
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          index,
    OUT CPSS_DXCH_HSR_PRP_PNT_STATE_ENT      *statePtr,
    OUT CPSS_DXCH_HSR_PRP_PNT_ENTRY_STC      *infoPtr
)
{
    GT_STATUS   rc;         /* return code */
    GT_U32      hwValue[3]; /* value to write to HW */
    GT_U32      ready,valid;/* ready,valid values read from HW */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E |
        CPSS_BOBCAT2_E | CPSS_BOBCAT3_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E |
        CPSS_ALDRIN2_E | CPSS_FALCON_E |  CPSS_AC5P_E |  CPSS_AC5X_E | CPSS_HARRIER_E);

    CPSS_NULL_PTR_CHECK_MAC(statePtr);
    CPSS_NULL_PTR_CHECK_MAC(infoPtr);

    if(index >= PNT_NUM_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "index[%d] limited to [0..%d]",
            index,PNT_NUM_CNS-1);
    }
    /* check the 'valid bit' */
    rc = prvCpssHwPpReadRegister(devNum,
        PRV_DXCH_REG1_UNIT_L2I_MAC(devNum).hsrPrp.hsrPrpPntValid[index],
        &valid);
    if(rc != GT_OK)
    {
        return rc;
    }

    if(0x1 == (valid & 0x1))
    {
        *statePtr = CPSS_DXCH_HSR_PRP_PNT_STATE_VALID_E;
    }
    else
    {
        /* check the 'ready bit' */
        rc = prvCpssHwPpReadRegister(devNum,
            PRV_DXCH_REG1_UNIT_L2I_MAC(devNum).hsrPrp.hsrPrpPntReady[index],
            &ready);
        if(rc != GT_OK)
        {
            return rc;
        }

        if(0x1 == (ready & 0x1))
        {
            *statePtr = CPSS_DXCH_HSR_PRP_PNT_STATE_READY_E;
        }
        else
        {
            *statePtr = CPSS_DXCH_HSR_PRP_PNT_STATE_NOT_VALID_E;
        }
    }

    /* read the 3 words of the entry */
    rc = prvCpssHwPpReadRegister(devNum,
        PRV_DXCH_REG1_UNIT_L2I_MAC(devNum).hsrPrp.hsrPrpPntEntryPart0[index],
        &hwValue[0]);
    if(rc != GT_OK)
    {
        return rc;
    }

    rc = prvCpssHwPpReadRegister(devNum,
        PRV_DXCH_REG1_UNIT_L2I_MAC(devNum).hsrPrp.hsrPrpPntEntryPart1[index],
        &hwValue[1]);
    if(rc != GT_OK)
    {
        return rc;
    }

    rc = prvCpssHwPpReadRegister(devNum,
        PRV_DXCH_REG1_UNIT_L2I_MAC(devNum).hsrPrp.hsrPrpPntEntryPart2[index],
        &hwValue[2]);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* word 0 */
    infoPtr->macAddr.arEther[2] = (GT_U8)(hwValue[0] >> 24);
    infoPtr->macAddr.arEther[3] = (GT_U8)(hwValue[0] >> 16);
    infoPtr->macAddr.arEther[4] = (GT_U8)(hwValue[0] >>  8);
    infoPtr->macAddr.arEther[5] = (GT_U8)(hwValue[0]);
    /* word 1 */
    infoPtr->macAddr.arEther[0] = (GT_U8)(hwValue[1] >>  8);
    infoPtr->macAddr.arEther[1] = (GT_U8)(hwValue[1]);

    infoPtr->ageBit      = BIT2BOOL_MAC((hwValue[1]>>16) & 0x1);
    infoPtr->lreInstance = (hwValue[1]>>17) & 0x3;
    infoPtr->destBmp     = (hwValue[1]>>19) & 0x3ff;
    infoPtr->untagged    = BIT2BOOL_MAC((hwValue[1]>>29) & 0x1);

    /* word 2 */
    infoPtr->streamId = hwValue[2] & 0xFFFF;

    return GT_OK;
}

/**
* @internal cpssDxChHsrPrpPntEntryGet function
* @endinternal
*
* @brief  Get the PNT entry info.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum         - device number.
* @param[in] index          - The index in the table.
*                             APPLICABLE RANGE: 0..127
*
* @param[out] statePtr      - (pointer to) The state of the entry.
* @param[out] infoPtr       - (pointer to) The entry info.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or index
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChHsrPrpPntEntryGet
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          index,
    OUT CPSS_DXCH_HSR_PRP_PNT_STATE_ENT      *statePtr,
    OUT CPSS_DXCH_HSR_PRP_PNT_ENTRY_STC      *infoPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChHsrPrpPntEntryGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, index , statePtr , infoPtr));

    rc = internal_cpssDxChHsrPrpPntEntryGet(devNum, index , statePtr , infoPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, index , statePtr , infoPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChHsrPrpPntAgingApply function
* @endinternal
*
* @brief  Age a range of entries in the PNT.
*       for each entry the operation is :
*        Ask to age the entry :
*        If the entry is not valid the operation is ignored
*        if the entry is fresh     (aged bit == 1) the entry will be aged (aged bit = 0).
*        if the entry already aged (aged bit == 0) the entry will be invalid (aged out). and counted.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum         - device number.
* @param[in] index          - The start index in the table.
*                             APPLICABLE RANGE: 0..127
* @param[in] numOfEntries   - The number of entries in the table to apply
*                             aging on , starting from 'index' (with wraparound support)
*                             NOTE: value 0xFFFFFFFF means 'Full Table' , starting from 'index'
*                               (not implicitly from index 0)
*                             APPLICABLE RANGE: 1..128 , 0xFFFFFFFF
*
* @param[out] numOfEntriesDeletedPtr   - (pointer to) The number of entries that
*                               deleted (invalidated) due to the aging.
*                               NOTE: if NULL ignored.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or index or numOfEntries
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
static GT_STATUS internal_cpssDxChHsrPrpPntAgingApply
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          index,
    IN  GT_U32                          numOfEntries,
    OUT GT_U32                          *numOfEntriesDeletedPtr
)
{
    GT_STATUS   rc;         /* return code */
    GT_U32  numEntriesAgingDone , numOfEntriesDeleted;
    GT_BOOL isInvalid,agedOut;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E |
        CPSS_BOBCAT2_E | CPSS_BOBCAT3_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E |
        CPSS_ALDRIN2_E | CPSS_FALCON_E |  CPSS_AC5P_E |  CPSS_AC5X_E | CPSS_HARRIER_E);

    if(index >= PNT_NUM_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "index[%d] limited to [0..%d]",
            index,PNT_NUM_CNS-1);
    }

    if(numOfEntries == 0 ||
       (numOfEntries > PNT_NUM_CNS &&
        numOfEntries != GT_NA) /* FULL table */)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "numOfEntries[%d] limited to [1..%d] or [0x%x]",
            index,PNT_NUM_CNS,GT_NA);
    }

    if(numOfEntries == GT_NA) /* FULL table */
    {
        numOfEntries = PNT_NUM_CNS;
    }

    numOfEntriesDeleted = 0;/* counter of aged out (deleted) due to aging */

    for(numEntriesAgingDone = 0; numEntriesAgingDone < numOfEntries; index++,numEntriesAgingDone++)
    {
        index %= PNT_NUM_CNS;

        rc = pnpCheckIsInvalidEntry(devNum ,index ,&isInvalid);
        if(rc != GT_OK)
        {
            continue;
        }

        if(isInvalid == GT_TRUE)
        {
            continue;
        }


        rc = pnpDoAgingEntry(devNum ,index ,&agedOut);
        if(rc != GT_OK)
        {
            continue;
        }

        if(agedOut == GT_TRUE)
        {
            numOfEntriesDeleted ++;
        }
    }

    if(numOfEntriesDeletedPtr)
    {
        *numOfEntriesDeletedPtr = numOfEntriesDeleted;
    }

    return GT_OK;
}

/**
* @internal cpssDxChHsrPrpPntAgingApply function
* @endinternal
*
* @brief  Age a range of entries in the PNT.
*       for each entry the operation is :
*        Ask to age the entry :
*        If the entry is not valid the operation is ignored
*        if the entry is fresh     (aged bit == 1) the entry will be aged (aged bit = 0).
*        if the entry already aged (aged bit == 0) the entry will be invalid (aged out). and counted.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum         - device number.
* @param[in] index          - The start index in the table.
*                             APPLICABLE RANGE: 0..127
* @param[in] numOfEntries   - The number of entries in the table to apply
*                             aging on , starting from 'index' (with wraparound support)
*                             NOTE: value 0xFFFFFFFF means 'Full Table' , starting from 'index'
*                               (not implicitly from index 0)
*                             APPLICABLE RANGE: 1..128 , 0xFFFFFFFF
*
* @param[out] numOfEntriesDeletedPtr   - (pointer to) The number of entries that
*                               deleted (invalidated) due to the aging.
*                               NOTE: if NULL ignored.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or index or numOfEntries
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChHsrPrpPntAgingApply
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          index,
    IN  GT_U32                          numOfEntries,
    OUT GT_U32                          *numOfEntriesDeletedPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChHsrPrpPntAgingApply);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, index , numOfEntries , numOfEntriesDeletedPtr));

    rc = internal_cpssDxChHsrPrpPntAgingApply(devNum, index , numOfEntries , numOfEntriesDeletedPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, index , numOfEntries , numOfEntriesDeletedPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChHsrPrpLreInstanceEntrySet function
* @endinternal
*
* @brief  Set the LRE instance entry info.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum         - device number.
* @param[in] index          - The index (instance) in the table.
*                             APPLICABLE RANGE: 0..3
* @param[in] infoPtr        - (pointer to) The entry info.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or index
* @retval GT_OUT_OF_RANGE          - out of range parameter
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
static GT_STATUS internal_cpssDxChHsrPrpLreInstanceEntrySet
(
    IN  GT_U8                               devNum,
    IN  GT_U32                              index,
    IN  CPSS_DXCH_HSR_PRP_LRE_INSTANCE_ENTRY_STC *infoPtr
)
{
    GT_U32      hwValue; /* value to write to HW */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E |
        CPSS_BOBCAT2_E | CPSS_BOBCAT3_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E |
        CPSS_ALDRIN2_E | CPSS_FALCON_E |  CPSS_AC5P_E |  CPSS_AC5X_E | CPSS_HARRIER_E);

    CPSS_NULL_PTR_CHECK_MAC(infoPtr);

    if(index >= LRE_NUM_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "index[%d] limited to [0..%d]",
            index,LRE_NUM_CNS-1);
    }

    if(infoPtr->numOfPorts > H_PORTS_NUM_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, "infoPtr->numOfPorts[%d] limited to [0..%d]",
            infoPtr->numOfPorts,H_PORTS_NUM_CNS);
    }

    if(infoPtr->lowestHPort >= H_PORTS_NUM_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, "infoPtr->lowestHPort[%d] limited to [0..%d]",
            infoPtr->lowestHPort,H_PORTS_NUM_CNS-1);
    }

    if((infoPtr->numOfPorts + infoPtr->lowestHPort) > H_PORTS_NUM_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, "(infoPtr->numOfPorts[%d] + infoPtr->lowestHPort[%d]) limited to [0..%d]",
            infoPtr->numOfPorts , infoPtr->lowestHPort,H_PORTS_NUM_CNS);
    }

    /*
        NOTE: we limit to 13 bits of HW limit , but should be less if the max 'eVIDX' is less.
        but we don't know the mac valid eVIDX because it depends from 'base' of the L2MLL LTT settings.
    */
    if ((infoPtr->eVidxMappingBase + (1 << infoPtr->numOfPorts)) >= BIT_13)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE,"(infoPtr->eVidxMappingBase[0x%8.8x] + 2^infoPtr->numOfPorts[0x%x]  limited to [0..0x%8.8x]",
            infoPtr->eVidxMappingBase , (1 << infoPtr->numOfPorts) ,
            BIT_13-1);
    }


    hwValue =  infoPtr->numOfPorts |
               infoPtr->lowestHPort << 4 |
               infoPtr->eVidxMappingBase << 8;

    return prvCpssHwPpWriteRegister(devNum,
        PRV_DXCH_REG1_UNIT_L2I_MAC(devNum).hsrPrp.hsrPrpLreInstanceTable[index],
        hwValue);

}

/**
* @internal cpssDxChHsrPrpLreInstanceEntrySet function
* @endinternal
*
* @brief  Set the LRE instance entry info.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum         - device number.
* @param[in] index          - The index (instance) in the table.
*                             APPLICABLE RANGE: 0..3
* @param[in] infoPtr        - (pointer to) The entry info.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or index
* @retval GT_OUT_OF_RANGE          - out of range parameter
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChHsrPrpLreInstanceEntrySet
(
    IN  GT_U8                               devNum,
    IN  GT_U32                              index,
    IN  CPSS_DXCH_HSR_PRP_LRE_INSTANCE_ENTRY_STC *infoPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChHsrPrpLreInstanceEntrySet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, index , infoPtr));

    rc = internal_cpssDxChHsrPrpLreInstanceEntrySet(devNum, index , infoPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, index , infoPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssDxChHsrPrpLreInstanceEntryGet function
* @endinternal
*
* @brief  Get the LRE instance entry info and counters.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum         - device number.
* @param[in] index          - The index (instance) in the table.
*                             APPLICABLE RANGE: 0..3
*
* @param[out] infoPtr       - (pointer to) The entry info.
*                               NOTE: if NULL ignored. allow to get only counter(s).
* @param[out] lreADiscardCounterPtr - (pointer to)The LRE 'A' discard counter.
*                               NOTE: if NULL ignored. allow to skip the counter.
*                               The counter is Read only and clear after read
* @param[out] lreBDiscardCounterPtr - (pointer to)The LRE 'B' discard counter.
*                               NOTE: if NULL ignored. allow to skip the counter.
*                               The counter is Read only and clear after read
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or index
* @retval GT_BAD_PTR               - if ALL the pointers are NULL
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
static GT_STATUS internal_cpssDxChHsrPrpLreInstanceEntryGet
(
    IN  GT_U8                               devNum,
    IN  GT_U32                              index,
    OUT CPSS_DXCH_HSR_PRP_LRE_INSTANCE_ENTRY_STC *infoPtr,
    OUT GT_U64                              *lreADiscardCounterPtr,
    OUT GT_U64                              *lreBDiscardCounterPtr
)
{
    GT_STATUS   rc;      /* return code */
    GT_U32      hwValue; /* value to write to HW */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E |
        CPSS_BOBCAT2_E | CPSS_BOBCAT3_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E |
        CPSS_ALDRIN2_E | CPSS_FALCON_E |  CPSS_AC5P_E |  CPSS_AC5X_E | CPSS_HARRIER_E);

    if(index >= LRE_NUM_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "index[%d] limited to [0..%d]",
            index,LRE_NUM_CNS-1);
    }

    if(infoPtr == NULL               &&
       lreADiscardCounterPtr == NULL &&
       lreBDiscardCounterPtr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PTR,
            "The Api not support NULL on all the pointers (at least one must be valid)");
    }

    if(infoPtr)
    {
        rc = prvCpssHwPpReadRegister(devNum,
            PRV_DXCH_REG1_UNIT_L2I_MAC(devNum).hsrPrp.hsrPrpLreInstanceTable[index],
            &hwValue);
        if(rc != GT_OK)
        {
            return rc;
        }

        infoPtr->numOfPorts       = hwValue & 0xF;
        infoPtr->lowestHPort      = (hwValue >> 4) & 0xF;
        infoPtr->eVidxMappingBase = (hwValue >> 8) & 0x1FFF;
    }

    if(lreADiscardCounterPtr)
    {
        rc = prvCpssPortGroupsBmpCounterSummary(devNum, CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
            PRV_DXCH_REG1_UNIT_L2I_MAC(devNum).hsrPrp.hsrPrpPntDiscardPacketsLreACounter[index],
            0, 32, NULL, lreADiscardCounterPtr);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    if(lreBDiscardCounterPtr)
    {
        rc = prvCpssPortGroupsBmpCounterSummary(devNum, CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
            PRV_DXCH_REG1_UNIT_L2I_MAC(devNum).hsrPrp.hsrPrpPntDiscardPacketsLreBCounter[index],
            0, 32, NULL, lreBDiscardCounterPtr);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    return GT_OK;
}

/**
* @internal cpssDxChHsrPrpLreInstanceEntryGet function
* @endinternal
*
* @brief  Get the LRE instance entry info and counters.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum         - device number.
* @param[in] index          - The index (instance) in the table.
*                             APPLICABLE RANGE: 0..3
*
* @param[out] infoPtr       - (pointer to) The entry info.
*                               NOTE: if NULL ignored. allow to get only counter(s).
* @param[out] lreADiscardCounterPtr - (pointer to)The LRE 'A' discard counter.
*                               NOTE: if NULL ignored. allow to skip the counter.
*                               The counter is Read only and clear after read
* @param[out] lreBDiscardCounterPtr - (pointer to)The LRE 'B' discard counter.
*                               NOTE: if NULL ignored. allow to skip the counter.
*                               The counter is Read only and clear after read
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or index
* @retval GT_BAD_PTR               - if ALL the pointers are NULL
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChHsrPrpLreInstanceEntryGet
(
    IN  GT_U8                               devNum,
    IN  GT_U32                              index,
    OUT CPSS_DXCH_HSR_PRP_LRE_INSTANCE_ENTRY_STC *infoPtr,
    OUT GT_U64                              *lreADiscardCounterPtr,
    OUT GT_U64                              *lreBDiscardCounterPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChHsrPrpLreInstanceEntryGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, index , infoPtr , lreADiscardCounterPtr , lreBDiscardCounterPtr));

    rc = internal_cpssDxChHsrPrpLreInstanceEntryGet(devNum, index , infoPtr , lreADiscardCounterPtr , lreBDiscardCounterPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, index , infoPtr , lreADiscardCounterPtr , lreBDiscardCounterPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChHsrPrpFdbPartitionModeSet function
* @endinternal
*
* @brief  Set the FDB partition with the DDE.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum         - device number.
* @param[in] mode           - The partition mode.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or mode
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
static GT_STATUS internal_cpssDxChHsrPrpFdbPartitionModeSet
(
    IN  GT_U8                               devNum,
    IN  CPSS_DXCH_HSR_PRP_FDB_PARTITION_MODE_ENT mode
)
{
    GT_STATUS   rc;      /* return code */
    GT_U32      hwValue; /* value to write to HW */
    GT_U32      fdbSize;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E |
        CPSS_BOBCAT2_E | CPSS_BOBCAT3_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E |
        CPSS_ALDRIN2_E | CPSS_FALCON_E |  CPSS_AC5P_E |  CPSS_AC5X_E | CPSS_HARRIER_E);

    switch(mode)
    {
        case CPSS_DXCH_HSR_PRP_FDB_PARTITION_MODE_DDE_NONE_E:
            hwValue = 0x0;
            break;
        case CPSS_DXCH_HSR_PRP_FDB_PARTITION_MODE_DDE_1_2_E:
            hwValue = 0x1;
            break;
        case CPSS_DXCH_HSR_PRP_FDB_PARTITION_MODE_DDE_3_4_E:
            hwValue = 0x2;
            break;
        case CPSS_DXCH_HSR_PRP_FDB_PARTITION_MODE_DDE_7_8_E:
            hwValue = 0x3;
            break;
        case CPSS_DXCH_HSR_PRP_FDB_PARTITION_MODE_DDE_ALL_E:
            hwValue = 0x7;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(mode);
    }

    fdbSize = PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.tableSize.fdb;

    if(mode != CPSS_DXCH_HSR_PRP_FDB_PARTITION_MODE_DDE_ALL_E)
    {
        fdbSize >>= hwValue;
    }
    else
    {
        fdbSize = 0;
    }

    /* update FDB hash parameters  */
    switch(fdbSize)
    {
        case   0:
            PRV_CPSS_DXCH_PP_MAC(devNum)->bridge.fdbHashParams.size =
                    CPSS_DXCH_BRG_FDB_TBL_SIZE_0_E;
            break;
        case _4K:
            PRV_CPSS_DXCH_PP_MAC(devNum)->bridge.fdbHashParams.size =
                    CPSS_DXCH_BRG_FDB_TBL_SIZE_4K_E;
            break;
        case _8K:
            PRV_CPSS_DXCH_PP_MAC(devNum)->bridge.fdbHashParams.size =
                    CPSS_DXCH_BRG_FDB_TBL_SIZE_8K_E;
            break;
        case _16K:
            PRV_CPSS_DXCH_PP_MAC(devNum)->bridge.fdbHashParams.size =
                    CPSS_DXCH_BRG_FDB_TBL_SIZE_16K_E;
            break;
        case _32K:
            PRV_CPSS_DXCH_PP_MAC(devNum)->bridge.fdbHashParams.size =
                    CPSS_DXCH_BRG_FDB_TBL_SIZE_32K_E;
            break;
        case _64K:
            PRV_CPSS_DXCH_PP_MAC(devNum)->bridge.fdbHashParams.size =
                    CPSS_DXCH_BRG_FDB_TBL_SIZE_64K_E;
            break;
        case _128K:
            PRV_CPSS_DXCH_PP_MAC(devNum)->bridge.fdbHashParams.size =
                    CPSS_DXCH_BRG_FDB_TBL_SIZE_128K_E;
            break;
        case _256K:
            PRV_CPSS_DXCH_PP_MAC(devNum)->bridge.fdbHashParams.size =
                    CPSS_DXCH_BRG_FDB_TBL_SIZE_256K_E;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_ONE_PARAM_FORMAT_MAC(fdbSize));
    }

    PRV_CPSS_DXCH_PP_MAC(devNum)->hsrPrp.ddeInfoValid = GT_TRUE;

    /* the DDEs start after the partition of the mac/ip entries */
    PRV_CPSS_DXCH_PP_MAC(devNum)->hsrPrp.ddeInfo.ddeBaseIndexInFdbTable  = fdbSize;
    /* the DDE get the 'rest' of the table */
    PRV_CPSS_DXCH_PP_MAC(devNum)->hsrPrp.ddeInfo.ddeNumEntriesInFdbTable =
            PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.parametericTables.numEntriesFdb - fdbSize;
    /* save the mode for DDE hash calculations */
    PRV_CPSS_DXCH_PP_MAC(devNum)->hsrPrp.ddeInfo.fdbPartitionMode = mode;

    /* set <HSR PRP FDB Mode> in FDB Unit */
    rc = prvCpssHwPpSetRegField(devNum,
        PRV_DXCH_REG1_UNIT_FDB_MAC(devNum).FDBCore.hsrPrp.hsrPrpGlobalConfig,
        25,3,hwValue);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* set <Enable HSR/PRP Processing> in L2i Unit */
    return prvCpssHwPpSetRegField(devNum,
        PRV_DXCH_REG1_UNIT_L2I_MAC(devNum).hsrPrp.hsrPrpGlobalConfig0,
        0,1,(hwValue == 0) ? 0 : 1);

}

/**
* @internal cpssDxChHsrPrpFdbPartitionModeSet function
* @endinternal
*
* @brief  Set the FDB partition with the DDE.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum         - device number.
* @param[in] mode           - The partition mode.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or mode
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChHsrPrpFdbPartitionModeSet
(
    IN  GT_U8                               devNum,
    IN  CPSS_DXCH_HSR_PRP_FDB_PARTITION_MODE_ENT mode
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChHsrPrpFdbPartitionModeSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, mode));

    rc = internal_cpssDxChHsrPrpFdbPartitionModeSet(devNum, mode);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, mode));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssDxChHsrPrpFdbPartitionModeGet function
* @endinternal
*
* @brief  Get the FDB partition with the DDE.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum         - device number.
*
* @param[out] modePtr       - (pointer to) The partition mode.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
static GT_STATUS internal_cpssDxChHsrPrpFdbPartitionModeGet
(
    IN  GT_U8                               devNum,
    OUT CPSS_DXCH_HSR_PRP_FDB_PARTITION_MODE_ENT *modePtr
)
{
    GT_STATUS   rc;      /* return code */
    GT_U32      hwValue; /* value to write to HW */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E |
        CPSS_BOBCAT2_E | CPSS_BOBCAT3_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E |
        CPSS_ALDRIN2_E | CPSS_FALCON_E |  CPSS_AC5P_E |  CPSS_AC5X_E | CPSS_HARRIER_E);

    CPSS_NULL_PTR_CHECK_MAC(modePtr);

    rc = prvCpssHwPpGetRegField(devNum,
        PRV_DXCH_REG1_UNIT_FDB_MAC(devNum).FDBCore.hsrPrp.hsrPrpGlobalConfig,
        25,3,&hwValue);
    if(rc != GT_OK)
    {
        return rc;
    }

    switch(hwValue)
    {
        case 0x0:
            *modePtr = CPSS_DXCH_HSR_PRP_FDB_PARTITION_MODE_DDE_NONE_E;
            break;
        case 0x1:
            *modePtr = CPSS_DXCH_HSR_PRP_FDB_PARTITION_MODE_DDE_1_2_E;
            break;
        case 0x2:
            *modePtr = CPSS_DXCH_HSR_PRP_FDB_PARTITION_MODE_DDE_3_4_E;
            break;
        case 0x3:
            *modePtr = CPSS_DXCH_HSR_PRP_FDB_PARTITION_MODE_DDE_7_8_E;
            break;
        case 0x7:
            *modePtr = CPSS_DXCH_HSR_PRP_FDB_PARTITION_MODE_DDE_ALL_E;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE,"The read Value[0x%x] is unknown",
                hwValue);
    }

    return GT_OK;
}

/**
* @internal cpssDxChHsrPrpFdbPartitionModeGet function
* @endinternal
*
* @brief  Get the FDB partition with the DDE.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum         - device number.
*
* @param[out] modePtr       - (pointer to) The partition mode.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChHsrPrpFdbPartitionModeGet
(
    IN  GT_U8                               devNum,
    OUT CPSS_DXCH_HSR_PRP_FDB_PARTITION_MODE_ENT *modePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChHsrPrpFdbPartitionModeGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, modePtr));

    rc = internal_cpssDxChHsrPrpFdbPartitionModeGet(devNum, modePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, modePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal ddeTimerFactorGet function
* @endinternal
*
* @brief  internal function to Get the DDE timer factor in micro seconds.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum         - device number.
*
* @param[out] factorPtr     - (pointer to) The factor.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
*
*/
static GT_STATUS ddeTimerFactorGet
(
    IN  GT_U8    devNum,
    OUT GT_U32   *factorPtr
)
{
    GT_STATUS   rc;      /* return code */
    GT_U32      hwValue; /* value to write to HW */

    /* get <Timer Tick Time> in FDB Unit */
    rc = prvCpssHwPpGetRegField(devNum,
        PRV_DXCH_REG1_UNIT_FDB_MAC(devNum).FDBCore.hsrPrp.hsrPrpGlobalConfig,
        0,2,&hwValue);

    *factorPtr = 80 >> hwValue;/* 80 / 40 / 20 / 10 */

    return rc;
}

/**
* @internal internal_cpssDxChHsrPrpDdeTimerGranularityModeSet function
* @endinternal
*
* @brief  Set the DDE timer granularity mode.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum         - device number.
* @param[in] mode           - The granularity mode.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or mode
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
static GT_STATUS internal_cpssDxChHsrPrpDdeTimerGranularityModeSet
(
    IN  GT_U8                                       devNum,
    IN  CPSS_DXCH_HSR_PRP_DDE_TIMER_GRANULARITY_MODE_ENT mode
)
{
    GT_U32      hwValue; /* value to write to HW */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E |
        CPSS_BOBCAT2_E | CPSS_BOBCAT3_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E |
        CPSS_ALDRIN2_E | CPSS_FALCON_E |  CPSS_AC5P_E |  CPSS_AC5X_E | CPSS_HARRIER_E);

    switch(mode)
    {
        case CPSS_DXCH_HSR_PRP_DDE_TIMER_GRANULARITY_MODE_10_MICRO_SEC_E:
            hwValue = 0x3;
            break;
        case CPSS_DXCH_HSR_PRP_DDE_TIMER_GRANULARITY_MODE_20_MICRO_SEC_E:
            hwValue = 0x2;
            break;
        case CPSS_DXCH_HSR_PRP_DDE_TIMER_GRANULARITY_MODE_40_MICRO_SEC_E:
            hwValue = 0x1;
            break;
        case CPSS_DXCH_HSR_PRP_DDE_TIMER_GRANULARITY_MODE_80_MICRO_SEC_E:
            hwValue = 0x0;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(mode);
    }

    /* NOTE: The uSec granularity arrives from reference clock (25MHz),
       so independent on core_clk per project */

    /* set <Timer Tick Time> in FDB Unit */
    return prvCpssHwPpSetRegField(devNum,
        PRV_DXCH_REG1_UNIT_FDB_MAC(devNum).FDBCore.hsrPrp.hsrPrpGlobalConfig,
        0,2,hwValue);
}

/**
* @internal cpssDxChHsrPrpDdeTimerGranularityModeSet function
* @endinternal
*
* @brief  Set the DDE timer granularity mode.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum         - device number.
* @param[in] mode           - The granularity mode.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or mode
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChHsrPrpDdeTimerGranularityModeSet
(
    IN  GT_U8                                       devNum,
    IN  CPSS_DXCH_HSR_PRP_DDE_TIMER_GRANULARITY_MODE_ENT mode
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChHsrPrpDdeTimerGranularityModeSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, mode));

    rc = internal_cpssDxChHsrPrpDdeTimerGranularityModeSet(devNum, mode);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, mode));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChHsrPrpDdeTimerGranularityModeGet function
* @endinternal
*
* @brief  Get the DDE timer granularity mode.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum         - device number.
*
* @param[out] modePtr       - (pointer to)The granularity mode.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
static GT_STATUS internal_cpssDxChHsrPrpDdeTimerGranularityModeGet
(
    IN  GT_U8                                       devNum,
    OUT CPSS_DXCH_HSR_PRP_DDE_TIMER_GRANULARITY_MODE_ENT *modePtr
)
{
    GT_STATUS   rc;      /* return code */
    GT_U32      hwValue; /* value to write to HW */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E |
        CPSS_BOBCAT2_E | CPSS_BOBCAT3_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E |
        CPSS_ALDRIN2_E | CPSS_FALCON_E |  CPSS_AC5P_E |  CPSS_AC5X_E | CPSS_HARRIER_E);

    CPSS_NULL_PTR_CHECK_MAC(modePtr);

    rc = prvCpssHwPpGetRegField(devNum,
        PRV_DXCH_REG1_UNIT_FDB_MAC(devNum).FDBCore.hsrPrp.hsrPrpGlobalConfig,
        0,2,&hwValue);
    if(rc != GT_OK)
    {
        return rc;
    }

    switch(hwValue)
    {
        case 0x0:
            *modePtr = CPSS_DXCH_HSR_PRP_DDE_TIMER_GRANULARITY_MODE_80_MICRO_SEC_E;
            break;
        case 0x1:
            *modePtr = CPSS_DXCH_HSR_PRP_DDE_TIMER_GRANULARITY_MODE_40_MICRO_SEC_E;
            break;
        case 0x2:
            *modePtr = CPSS_DXCH_HSR_PRP_DDE_TIMER_GRANULARITY_MODE_20_MICRO_SEC_E;
            break;
        case 0x3:
        default:
            *modePtr = CPSS_DXCH_HSR_PRP_DDE_TIMER_GRANULARITY_MODE_10_MICRO_SEC_E;
            break;
    }

    return GT_OK;
}
/**
* @internal cpssDxChHsrPrpDdeTimerGranularityModeGet function
* @endinternal
*
* @brief  Get the DDE timer granularity mode.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum         - device number.
*
* @param[out] modePtr       - (pointer to)The granularity mode.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChHsrPrpDdeTimerGranularityModeGet
(
    IN  GT_U8                                       devNum,
    OUT CPSS_DXCH_HSR_PRP_DDE_TIMER_GRANULARITY_MODE_ENT *modePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChHsrPrpDdeTimerGranularityModeGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, modePtr));

    rc = internal_cpssDxChHsrPrpDdeTimerGranularityModeGet(devNum, modePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, modePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal ddeTimeFieldInfoGet function
* @endinternal
*
* @brief  Get the register address and start bit to specific DDE time field.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum         - device number.
* @param[in] field           - The type of DDE time field.
*
* @param[out] regAddrPtr        - (pointer to) the register address.
* @param[out] startBitPtr       - (pointer to) the start bit.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong field
*
*/
static GT_STATUS ddeTimeFieldInfoGet(
    IN GT_U8                            devNum,
    IN  CPSS_DXCH_HSR_PRP_DDE_TIME_FIELD_ENT field,
    OUT GT_U32      *regAddrPtr,
    OUT GT_U32      *startBitPtr
)
{
    GT_U32      regAddr  = GT_NA;
    GT_U32      startBit = 0;

    switch(field)
    {
        case CPSS_DXCH_HSR_PRP_DDE_TIME_FIELD_SAME_KEY_EXPIRATION_E:
            regAddr  = PRV_DXCH_REG1_UNIT_FDB_MAC(devNum).FDBCore.hsrPrp.hsrPrpGlobalConfig;
            startBit = 2;
            break;
        case CPSS_DXCH_HSR_PRP_DDE_TIME_FIELD_EXPIRATION_E:
            regAddr  = PRV_DXCH_REG1_UNIT_FDB_MAC(devNum).FDBCore.hsrPrp.ddeExpirationTimeConfig;
            startBit = 0;
            break;
        case CPSS_DXCH_HSR_PRP_DDE_TIME_FIELD_TIMER_E:
            regAddr  = PRV_DXCH_REG1_UNIT_FDB_MAC(devNum).FDBCore.hsrPrp.hsrPrpTimer;
            startBit = 0;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(field);
    }

    *regAddrPtr  = regAddr;
    *startBitPtr = startBit;

    return GT_OK;
}

/**
* @internal cpssDxChHsrPrpDdeTimeFieldSet function
* @endinternal
*
* @brief  Set time for a specific DDE time field.
*
*       NOTEs:
*       1. The time is in micro-seconds.
*       2. The time range and granularity depends on 'DdeTimerGranularity' set by :
*          cpssDxChHsrPrpDdeTimerGranularityModeSet(...)
*       3. the function round the time to the nearest value supported by the granularity.
*          the actual value can be retrieved by the 'Get' function.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum         - device number.
* @param[in] field          - The time field.
* @param[in] timeInMicroSec - The time in micro-seconds.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or field
* @retval GT_OUT_OF_RANGE          - out of range timeInMicroSec
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
static GT_STATUS internal_cpssDxChHsrPrpDdeTimeFieldSet
(
    IN  GT_U8                           devNum,
    IN  CPSS_DXCH_HSR_PRP_DDE_TIME_FIELD_ENT field,
    IN  GT_U32                          timeInMicroSec
)
{
    GT_STATUS   rc;      /* return code */
    GT_U32      regAddr; /* register address */
    GT_U32      startBit;/* start bit in the register */
    GT_U32      hwValue; /* value to write to HW */
    GT_U32      factor;  /* the time factor from HW value to 'micro-seconds' */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E |
        CPSS_BOBCAT2_E | CPSS_BOBCAT3_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E |
        CPSS_ALDRIN2_E | CPSS_FALCON_E |  CPSS_AC5P_E |  CPSS_AC5X_E | CPSS_HARRIER_E);

    if(field == CPSS_DXCH_HSR_PRP_DDE_TIME_FIELD_TIMER_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,"The 'TIMER' is 'Read Only' and can't be 'Set'");
    }

    /* get info about the register address and the start bit */
    rc = ddeTimeFieldInfoGet(devNum,field,&regAddr,&startBit);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* get info about the time factor */
    rc = ddeTimerFactorGet(devNum,&factor);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* do  '+ (factor / 2)' to round the value to nearest integer
       after dividing by 'factor' */
    hwValue = (timeInMicroSec + (factor / 2)) / factor;

    /* check if the value can fit into the 22 bits of the register */
    if(hwValue >= BIT_22)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, "timeInMicroSec[0x%8.8x] limited to [0..0x%8.8x]",
            timeInMicroSec,(BIT_22-1)*factor + (factor / 2) - 1);
    }

    return prvCpssHwPpSetRegField(devNum, regAddr, startBit, 22, hwValue);
}

/**
* @internal cpssDxChHsrPrpDdeTimeFieldSet function
* @endinternal
*
* @brief  Set time for a specific DDE time field.
*
*       NOTEs:
*       1. The time is in micro-seconds.
*       2. The time range and granularity depends on 'DdeTimerGranularity' set by :
*          cpssDxChHsrPrpDdeTimerGranularityModeSet(...)
*       3. the function round the time to the nearest value supported by the granularity.
*          the actual value can be retrieved by the 'Get' function.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum         - device number.
* @param[in] field          - The time field.
* @param[in] timeInMicroSec - The time in micro-seconds.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or field
* @retval GT_OUT_OF_RANGE          - out of range timeInMicroSec
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChHsrPrpDdeTimeFieldSet
(
    IN  GT_U8                           devNum,
    IN  CPSS_DXCH_HSR_PRP_DDE_TIME_FIELD_ENT field,
    IN  GT_U32                          timeInMicroSec
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChHsrPrpDdeTimeFieldSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, field , timeInMicroSec));

    rc = internal_cpssDxChHsrPrpDdeTimeFieldSet(devNum, field , timeInMicroSec);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, field , timeInMicroSec));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal cpssDxChHsrPrpDdeTimeFieldGet function
* @endinternal
*
* @brief  Get time for a specific DDE time field.
*
*       NOTEs:
*       1. The time is in micro-seconds.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum         - device number.
* @param[in] field          - The time field.
*
* @param[out] timeInMicroSecPtr - (pointer to)The time in micro-seconds.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or field
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
static GT_STATUS internal_cpssDxChHsrPrpDdeTimeFieldGet
(
    IN  GT_U8                           devNum,
    IN  CPSS_DXCH_HSR_PRP_DDE_TIME_FIELD_ENT field,
    OUT GT_U32                          *timeInMicroSecPtr
)
{
    GT_STATUS   rc;      /* return code */
    GT_U32      regAddr; /* register address */
    GT_U32      startBit;/* start bit in the register */
    GT_U32      hwValue; /* value to write to HW */
    GT_U32      factor;  /* the time factor from HW value to 'micro-seconds' */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E |
        CPSS_BOBCAT2_E | CPSS_BOBCAT3_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E |
        CPSS_ALDRIN2_E | CPSS_FALCON_E |  CPSS_AC5P_E |  CPSS_AC5X_E | CPSS_HARRIER_E);

    CPSS_NULL_PTR_CHECK_MAC(timeInMicroSecPtr);

    /* get info about the register address and the start bit */
    rc = ddeTimeFieldInfoGet(devNum,field,&regAddr,&startBit);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* get info about the time factor */
    rc = ddeTimerFactorGet(devNum,&factor);
    if(rc != GT_OK)
    {
        return rc;
    }

    rc = prvCpssHwPpGetRegField(devNum, regAddr, startBit, 22, &hwValue);

    *timeInMicroSecPtr = hwValue * factor;

    return rc;
}

/**
* @internal cpssDxChHsrPrpDdeTimeFieldGet function
* @endinternal
*
* @brief  Get time for a specific DDE time field.
*
*       NOTEs:
*       1. The time is in micro-seconds.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum         - device number.
* @param[in] field          - The time field.
*
* @param[out] timeInMicroSecPtr - (pointer to)The time in micro-seconds.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or field
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChHsrPrpDdeTimeFieldGet
(
    IN  GT_U8                           devNum,
    IN  CPSS_DXCH_HSR_PRP_DDE_TIME_FIELD_ENT field,
    OUT GT_U32                          *timeInMicroSecPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChHsrPrpDdeTimeFieldGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, field , timeInMicroSecPtr));

    rc = internal_cpssDxChHsrPrpDdeTimeFieldGet(devNum, field , timeInMicroSecPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, field , timeInMicroSecPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChHsrPrpPortPrpTrailerActionSet function
* @endinternal
*
* @brief  Set the PRP trailer action on specific port.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum         - device number.
* @param[in] portNum        - The port number.
*                               APPLICABLE RANGE: physical port range.
* @param[in] action         - The trailer action.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or portNum/action
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
static GT_STATUS internal_cpssDxChHsrPrpPortPrpTrailerActionSet
(
    IN  GT_U8                                    devNum,
    IN  GT_PHYSICAL_PORT_NUM                     portNum,
    IN  CPSS_DXCH_HSR_PRP_PRP_TRAILER_ACTION_ENT action
)
{
    GT_U32      hwValue; /* value to write to HW */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E |
        CPSS_BOBCAT2_E | CPSS_BOBCAT3_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E |
        CPSS_ALDRIN2_E | CPSS_FALCON_E |  CPSS_AC5P_E |  CPSS_AC5X_E | CPSS_HARRIER_E);

    PRV_CPSS_DXCH_PHY_PORT_NUM_CHECK_MAC(devNum, portNum);

    switch(action)
    {
        case CPSS_DXCH_HSR_PRP_PRP_TRAILER_ACTION_NONE_E:
            hwValue = 0x0;
            break;
        case CPSS_DXCH_HSR_PRP_PRP_TRAILER_ACTION_ADD_PRP_E:
            hwValue = 0x1;
            break;
        case CPSS_DXCH_HSR_PRP_PRP_TRAILER_ACTION_REMOVE_PRP_E:
            hwValue = 0x2;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(action);
    }

    /* write to HA physical port 1 table */
    return prvCpssDxChWriteTableEntryField(devNum,
            CPSS_DXCH_SIP5_TABLE_HA_PHYSICAL_PORT_1_E,
            portNum,
            PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
            SIP6_30_HA_PHYSICAL_PORT_TABLE_1_FIELDS_PRP_TRAILER_ACTION_E, /* field name */
            PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
            hwValue);
}

/**
* @internal cpssDxChHsrPrpPortPrpTrailerActionSet function
* @endinternal
*
* @brief  Set the PRP trailer action on specific port.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum         - device number.
* @param[in] portNum        - The port number.
*                               APPLICABLE RANGE: physical port range.
* @param[in] action         - The trailer action.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or portNum/action
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChHsrPrpPortPrpTrailerActionSet
(
    IN  GT_U8                                    devNum,
    IN  GT_PHYSICAL_PORT_NUM                     portNum,
    IN  CPSS_DXCH_HSR_PRP_PRP_TRAILER_ACTION_ENT action
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChHsrPrpPortPrpTrailerActionSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum , action));

    rc = internal_cpssDxChHsrPrpPortPrpTrailerActionSet(devNum, portNum , action);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum , action));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChHsrPrpPortPrpTrailerActionGet function
* @endinternal
*
* @brief  Get the PRP trailer action on specific port.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum         - device number.
* @param[in] portNum        - The port number.
*                               APPLICABLE RANGE: physical port range.
*
* @param[out] actionPtr     - (pointer to)The trailer action.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or portNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
static GT_STATUS internal_cpssDxChHsrPrpPortPrpTrailerActionGet
(
    IN  GT_U8                                    devNum,
    IN  GT_PHYSICAL_PORT_NUM                     portNum,
    OUT CPSS_DXCH_HSR_PRP_PRP_TRAILER_ACTION_ENT *actionPtr
)
{
    GT_STATUS   rc;      /* return code */
    GT_U32      hwValue; /* value to read from HW */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E |
        CPSS_BOBCAT2_E | CPSS_BOBCAT3_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E |
        CPSS_ALDRIN2_E | CPSS_FALCON_E |  CPSS_AC5P_E |  CPSS_AC5X_E | CPSS_HARRIER_E);

    PRV_CPSS_DXCH_PHY_PORT_NUM_CHECK_MAC(devNum, portNum);
    CPSS_NULL_PTR_CHECK_MAC(actionPtr);

    /* read from HA physical port 1 table */
    rc = prvCpssDxChReadTableEntryField(devNum,
            CPSS_DXCH_SIP5_TABLE_HA_PHYSICAL_PORT_1_E,
            portNum,
            PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
            SIP6_30_HA_PHYSICAL_PORT_TABLE_1_FIELDS_PRP_TRAILER_ACTION_E, /* field name */
            PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
            &hwValue);

    switch(hwValue)
    {
        case 0x0:
            *actionPtr = CPSS_DXCH_HSR_PRP_PRP_TRAILER_ACTION_NONE_E;
            break;
        case 0x1:
            *actionPtr = CPSS_DXCH_HSR_PRP_PRP_TRAILER_ACTION_ADD_PRP_E;
            break;
        case 0x2:
            *actionPtr = CPSS_DXCH_HSR_PRP_PRP_TRAILER_ACTION_REMOVE_PRP_E;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE,"The read Value[0x%x] is unknown",
                hwValue);
    }

    return rc;
}

/**
* @internal cpssDxChHsrPrpPortPrpTrailerActionGet function
* @endinternal
*
* @brief  Get the PRP trailer action on specific port.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum         - device number.
* @param[in] portNum        - The port number.
*                               APPLICABLE RANGE: physical port range.
*
* @param[out] actionPtr     - (pointer to)The trailer action.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or portNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChHsrPrpPortPrpTrailerActionGet
(
    IN  GT_U8                                    devNum,
    IN  GT_PHYSICAL_PORT_NUM                     portNum,
    OUT CPSS_DXCH_HSR_PRP_PRP_TRAILER_ACTION_ENT *actionPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChHsrPrpPortPrpTrailerActionGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum , actionPtr));

    rc = internal_cpssDxChHsrPrpPortPrpTrailerActionGet(devNum, portNum , actionPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum , actionPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChHsrPrpPortPrpPaddingModeSet function
* @endinternal
*
* @brief  Set the PRP padding mode on specific port.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum         - device number.
* @param[in] portNum        - The port number.
*                               APPLICABLE RANGE: physical port range.
* @param[in] mode           - The padding mode.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or portNum/mode
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
static GT_STATUS internal_cpssDxChHsrPrpPortPrpPaddingModeSet
(
    IN  GT_U8                                    devNum,
    IN  GT_PHYSICAL_PORT_NUM                     portNum,
    IN  CPSS_DXCH_HSR_PRP_PRP_PADDING_MODE_ENT   mode
)
{
    GT_U32      hwValue; /* value to write to HW */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E |
        CPSS_BOBCAT2_E | CPSS_BOBCAT3_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E |
        CPSS_ALDRIN2_E | CPSS_FALCON_E |  CPSS_AC5P_E |  CPSS_AC5X_E | CPSS_HARRIER_E);

    PRV_CPSS_DXCH_PHY_PORT_NUM_CHECK_MAC(devNum, portNum);

    switch(mode)
    {
        case CPSS_DXCH_HSR_PRP_PRP_PADDING_MODE_PAD_UNTAGGED_TO_60_OR_VLAN_TAGGED_TO_64_E:
            hwValue = 0x0;
            break;
        case CPSS_DXCH_HSR_PRP_PRP_PADDING_MODE_PAD_TO_60_E:
            hwValue = 0x1;
            break;
        case CPSS_DXCH_HSR_PRP_PRP_PADDING_MODE_PAD_TO_64_E:
            hwValue = 0x2;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(mode);
    }

    /* write to HA physical port 1 table */
    return prvCpssDxChWriteTableEntryField(devNum,
            CPSS_DXCH_SIP5_TABLE_HA_PHYSICAL_PORT_1_E,
            portNum,
            PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
            SIP6_30_HA_PHYSICAL_PORT_TABLE_1_FIELDS_PRP_PADDING_SIZE_E, /* field name */
            PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
            hwValue);
}

/**
* @internal cpssDxChHsrPrpPortPrpPaddingModeSet function
* @endinternal
*
* @brief  Set the PRP padding mode on specific port.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum         - device number.
* @param[in] portNum        - The port number.
*                               APPLICABLE RANGE: physical port range.
* @param[in] mode           - The padding mode.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or portNum/mode
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChHsrPrpPortPrpPaddingModeSet
(
    IN  GT_U8                                    devNum,
    IN  GT_PHYSICAL_PORT_NUM                     portNum,
    IN  CPSS_DXCH_HSR_PRP_PRP_PADDING_MODE_ENT   mode
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChHsrPrpPortPrpPaddingModeSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum , mode));

    rc = internal_cpssDxChHsrPrpPortPrpPaddingModeSet(devNum, portNum , mode);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum , mode));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChHsrPrpPortPrpPaddingModeGet function
* @endinternal
*
* @brief  Get the PRP padding mode on specific port.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum         - device number.
* @param[in] portNum        - The port number.
*                               APPLICABLE RANGE: physical port range.
*
* @param[out] modePtr       - (pointer to)The padding mode.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or portNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
static GT_STATUS internal_cpssDxChHsrPrpPortPrpPaddingModeGet
(
    IN  GT_U8                                    devNum,
    IN  GT_PHYSICAL_PORT_NUM                     portNum,
    OUT CPSS_DXCH_HSR_PRP_PRP_PADDING_MODE_ENT   *modePtr
)
{
    GT_STATUS   rc;      /* return code */
    GT_U32      hwValue; /* value to read from HW */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E |
        CPSS_BOBCAT2_E | CPSS_BOBCAT3_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E |
        CPSS_ALDRIN2_E | CPSS_FALCON_E |  CPSS_AC5P_E |  CPSS_AC5X_E | CPSS_HARRIER_E);

    PRV_CPSS_DXCH_PHY_PORT_NUM_CHECK_MAC(devNum, portNum);
    CPSS_NULL_PTR_CHECK_MAC(modePtr);

    /* read from HA physical port 1 table */
    rc = prvCpssDxChReadTableEntryField(devNum,
            CPSS_DXCH_SIP5_TABLE_HA_PHYSICAL_PORT_1_E,
            portNum,
            PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
            SIP6_30_HA_PHYSICAL_PORT_TABLE_1_FIELDS_PRP_PADDING_SIZE_E, /* field name */
            PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
            &hwValue);

    switch(hwValue)
    {
        case 0x0:
            *modePtr = CPSS_DXCH_HSR_PRP_PRP_PADDING_MODE_PAD_UNTAGGED_TO_60_OR_VLAN_TAGGED_TO_64_E;
            break;
        case 0x1:
            *modePtr = CPSS_DXCH_HSR_PRP_PRP_PADDING_MODE_PAD_TO_60_E;
            break;
        case 0x2:
            *modePtr = CPSS_DXCH_HSR_PRP_PRP_PADDING_MODE_PAD_TO_64_E;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE,"The read Value[0x%x] is unknown",
                hwValue);
    }

    return rc;
}

/**
* @internal cpssDxChHsrPrpPortPrpPaddingModeGet function
* @endinternal
*
* @brief  Get the PRP padding mode on specific port.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum         - device number.
* @param[in] portNum        - The port number.
*                               APPLICABLE RANGE: physical port range.
*
* @param[out] modePtr       - (pointer to)The padding mode.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or portNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChHsrPrpPortPrpPaddingModeGet
(
    IN  GT_U8                                    devNum,
    IN  GT_PHYSICAL_PORT_NUM                     portNum,
    OUT CPSS_DXCH_HSR_PRP_PRP_PADDING_MODE_ENT   *modePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChHsrPrpPortPrpPaddingModeGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum , modePtr));

    rc = internal_cpssDxChHsrPrpPortPrpPaddingModeGet(devNum, portNum , modePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum , modePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal ddeEntryInvalidate function
* @endinternal
*
* @brief   Invalidate an entry in DDE table in specified index.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum                - device number
* @param[in] index                 - The index of the entry.
*                                    APPLICABLE RANGE: 0..max
*                                    NOTE: 1. 'max' index depends on the mode selected
*                                             by CPSS_DXCH_HSR_PRP_FDB_PARTITION_MODE_ENT
*                                          2. The index can be calculated by cpssDxChHsrPrpDdeHashCalc(...)
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum,index
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
static GT_STATUS ddeEntryInvalidate
(
    IN  GT_U8         devNum,
    IN  GT_U32        hwIndex,
    IN  GT_U32        oldTimeStamp,
    OUT GT_BOOL       *isDeletedPtr
)
{
    GT_STATUS   rc;      /* return code */
    GT_U32      hwValue; /* value to write to HW */
    GT_U32      regAddr; /* register address */
    GT_U32      bankId,indexInBank;
    GT_U32      newTimestamp;

    *isDeletedPtr = GT_FALSE;

    /* we can invalidate the entry only after we 'lock' this index , so the HW will not update it */
    regAddr  = PRV_DXCH_REG1_UNIT_FDB_MAC(devNum).FDBCore.hsrPrp.hsrPrpAging;

    bankId      = hwIndex % PRV_CPSS_DXCH_PP_MAC(devNum)->bridge.fdbHashParams.numOfBanks;
    indexInBank = hwIndex / PRV_CPSS_DXCH_PP_MAC(devNum)->bridge.fdbHashParams.numOfBanks;

    hwValue = 0x1 /*<DDE Enable To Be Deleted FDB Entry>*/ |
              (bankId << 1) |  /* bits [4:1] are the bank    */
              indexInBank << 5;/* bits[20:5] are the address */

    rc = prvCpssHwPpWriteRegister(devNum, regAddr, hwValue);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* get newTimestamp */
    rc = prvCpssDxChReadTableEntryField(devNum,
            CPSS_DXCH_TABLE_FDB_E,
            hwIndex,
            PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
            SIP6_30_FDB_FDB_TABLE_FIELDS_DDE_TIMESTAMP_E, /* field name */
            PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
            &newTimestamp);
    if(rc != GT_OK)
    {
        return rc;
    }

    if(newTimestamp == oldTimeStamp)
    {
        /* the entry was not refreshed by the traffic between the time we read the
           entry and the time we set the <DDE Enable To Be Deleted FDB Entry> */

        /* write 0 to first word */
        rc = prvCpssDxChWriteTableEntryField(devNum,
                CPSS_DXCH_TABLE_FDB_E,
                hwIndex,
                0,/*word 0*/
                0,/*start bit 0*/
                32,/*32 bits*/
                0/*reset the first word*/);
        if(rc != GT_OK)
        {
            return rc;
        }

        *isDeletedPtr = GT_TRUE;
    }

    /* release the LOCK */
    return prvCpssHwPpWriteRegister(devNum, regAddr, 0);
}
/**
* @internal internal_cpssDxChHsrPrpDdeEntryInvalidate function
* @endinternal
*
* @brief   Invalidate an entry in DDE table in specified index.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum                - device number
* @param[in] index                 - The index of the entry.
*                                    APPLICABLE RANGE: 0..max
*                                    NOTE: 1. 'max' index depends on the mode selected
*                                             by CPSS_DXCH_HSR_PRP_FDB_PARTITION_MODE_ENT
*                                          2. The index can be calculated by cpssDxChHsrPrpDdeHashCalc(...)
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum,index
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
static GT_STATUS internal_cpssDxChHsrPrpDdeEntryInvalidate
(
    IN  GT_U8         devNum,
    IN  GT_U32        index
)
{
    GT_STATUS   rc;      /* return code */
    GT_U32      hwIndex; /* index to the FDB table of the HW */
    GT_U32      oldTimeStamp;
    GT_BOOL     isDeleted;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E |
        CPSS_BOBCAT2_E | CPSS_BOBCAT3_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E |
        CPSS_ALDRIN2_E | CPSS_FALCON_E |  CPSS_AC5P_E |  CPSS_AC5X_E | CPSS_HARRIER_E);

    /* check the DDE index */
    DDE_INDEX_CHECK_MAC(devNum,index);

    /* write to the FDB table at the 'HW index' */
    hwIndex = index + PRV_CPSS_DXCH_PP_MAC(devNum)->hsrPrp.ddeInfo.ddeBaseIndexInFdbTable;

    /* the logic of this function :
       If the software wants to age out an entry it should:
            1: Read the entry timestamp
            2: Set the global configuration that the entry is going to be deleted
            3: Read the entry timestamp again
            4: If the timestamp is same, then delete the entry. Else don’t delete the entry
       If the HW adds a new key between step 1 and step 3 the SW would not override it
       The HW would not add new key after step 2
    */

    /* get oldTimeStamp */
    rc = prvCpssDxChReadTableEntryField(devNum,
            CPSS_DXCH_TABLE_FDB_E,
            hwIndex,
            PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
            SIP6_30_FDB_FDB_TABLE_FIELDS_DDE_TIMESTAMP_E, /* field name */
            PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
            &oldTimeStamp);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* call to invalidate the entry */
    return ddeEntryInvalidate(devNum,hwIndex,oldTimeStamp,&isDeleted);
}

/**
* @internal cpssDxChHsrPrpDdeEntryInvalidate function
* @endinternal
*
* @brief   Invalidate an entry in DDE table in specified index.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum                - device number
* @param[in] index                 - The index of the entry.
*                                    APPLICABLE RANGE: 0..max
*                                    NOTE: 1. 'max' index depends on the mode selected
*                                             by CPSS_DXCH_HSR_PRP_FDB_PARTITION_MODE_ENT
*                                          2. The index can be calculated by cpssDxChHsrPrpDdeHashCalc(...)
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum,index
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChHsrPrpDdeEntryInvalidate
(
    IN  GT_U8         devNum,
    IN  GT_U32        index
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChHsrPrpDdeEntryInvalidate);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, index));

    rc = internal_cpssDxChHsrPrpDdeEntryInvalidate(devNum, index);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, index));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssDxChHsrPrpDdeEntryWrite function
* @endinternal
*
* @brief   Write an entry in DDE table in specified index.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum                - device number
* @param[in] index                 - The index of the entry.
*                                    APPLICABLE RANGE: 0..max
*                                    NOTE: 1. 'max' index depends on the mode selected
*                                             by CPSS_DXCH_HSR_PRP_FDB_PARTITION_MODE_ENT
*                                          2. The index can be calculated by cpssDxChHsrPrpDdeHashCalc(...)
* @param[in] infoPtr               - (pointer to) The entry info.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum,index
* @retval GT_OUT_OF_RANGE          - on out of range parameters in entryPtr :
*                                    ddeKey.seqNum , ddeKey.lreInstance , destBmp  , srcHPort  ,
*                                    timeInMicroSec , destHportCounters[]
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
static GT_STATUS internal_cpssDxChHsrPrpDdeEntryWrite
(
    IN  GT_U8                        devNum,
    IN  GT_U32                       index,
    IN  CPSS_DXCH_HSR_PRP_DDE_ENTRY_STC   *entryPtr
)
{
    GT_STATUS   rc;      /* return code */
    GT_U32      hwIndex; /* index to the FDB table of the HW */
    GT_U32      hwDataArr[SIP_6_30_FDB_ENTRY_WORDS_SIZE_CNS];
    GT_U32      hwValue; /* value to write to HW */
    GT_U32      factor;  /* the time factor from HW value to 'micro-seconds' */
    GT_U32      ii;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E |
        CPSS_BOBCAT2_E | CPSS_BOBCAT3_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E |
        CPSS_ALDRIN2_E | CPSS_FALCON_E |  CPSS_AC5P_E |  CPSS_AC5X_E | CPSS_HARRIER_E);

    CPSS_NULL_PTR_CHECK_MAC(entryPtr);

    /* check the DDE index */
    DDE_INDEX_CHECK_MAC(devNum,index);

    /* reset all words in hwDataArr */
    cpssOsMemSet(&hwDataArr[0], 0,
             SIP_6_30_FDB_ENTRY_WORDS_SIZE_CNS * sizeof(GT_U32));

    SIP6_30_FDB_DDE_ENTRY_FIELD_MAC_ADDR_SET_MAC(devNum, hwDataArr,
        &entryPtr->ddeKey.macAddr.arEther[0]);

    hwValue = BOOL2BIT_MAC(entryPtr->ageBit);
    SIP6_FDB_ENTRY_FIELD_SET_MAC(devNum,hwDataArr,
        SIP5_FDB_FDB_TABLE_FIELDS_AGE_E,
        hwValue);

    hwValue = HW_FDB_DDE_ENTRY_TYPE_CNS;/* DDE entry type ! */
    SIP6_FDB_ENTRY_FIELD_SET_MAC(devNum,hwDataArr,
        SIP5_FDB_FDB_TABLE_FIELDS_FDB_ENTRY_TYPE_E,
        hwValue);

    hwValue = 0x1;
    SIP6_FDB_ENTRY_FIELD_SET_MAC(devNum,hwDataArr,
        SIP5_FDB_FDB_TABLE_FIELDS_VALID_E,
        hwValue);

    if(entryPtr->ddeKey.seqNum >= BIT_16)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, "entryPtr->ddeKey.seqNum[0x%8.8x] limited to [0..0x%8.8x]",
            entryPtr->ddeKey.seqNum,
            (BIT_16-1));
    }

    hwValue = entryPtr->ddeKey.seqNum;
    SIP6_FDB_ENTRY_FIELD_SET_MAC(devNum,hwDataArr,
        SIP6_30_FDB_FDB_TABLE_FIELDS_DDE_SEQUENCE_NUMBER_E,
        hwValue);

    if(entryPtr->ddeKey.lreInstance >= BIT_2)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, "entryPtr->ddeKey.lreInstance[%d] limited to [0..%d]",
            entryPtr->ddeKey.lreInstance,
            (BIT_2-1));
    }
    hwValue = entryPtr->ddeKey.lreInstance;
    SIP6_FDB_ENTRY_FIELD_SET_MAC(devNum,hwDataArr,
        SIP6_30_FDB_FDB_TABLE_FIELDS_DDE_TARGET_INSTANCE_E,
        hwValue);

    if(entryPtr->destBmp >= BIT_10)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, "entryPtr->destBmp[0x%4.4x] limited to [0..0x%4.4x]",
            entryPtr->destBmp,BIT_10-1);
    }
    hwValue = entryPtr->destBmp;
    SIP6_FDB_ENTRY_FIELD_SET_MAC(devNum,hwDataArr,
        SIP6_30_FDB_FDB_TABLE_FIELDS_DDE_TARGET_PORT_BITMAP_E,
        hwValue);

    if(entryPtr->srcHPort >= H_PORTS_NUM_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, "entryPtr->srcHPort[%d] limited to [0..%d]",
            entryPtr->srcHPort,H_PORTS_NUM_CNS-1);
    }
    hwValue = entryPtr->srcHPort;
    SIP6_FDB_ENTRY_FIELD_SET_MAC(devNum,hwDataArr,
        SIP6_30_FDB_FDB_TABLE_FIELDS_DDE_TARGET_SOURCE_HPORT_E,
        hwValue);

    /* get info about the time factor */
    rc = ddeTimerFactorGet(devNum,&factor);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* do  '+ (factor / 2)' to round the value to nearest integer
       after dividing by 'factor' */
    hwValue = (entryPtr->timeInMicroSec + (factor / 2)) / factor;
    /* check if the value can fit into the 22 bits of the register */
    if(hwValue >= BIT_22)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, "entryPtr->timeInMicroSec[0x%8.8x] limited to [0..0x%8.8x]",
            entryPtr->timeInMicroSec,(BIT_22-1)*factor + (factor / 2) - 1);
    }

    SIP6_FDB_ENTRY_FIELD_SET_MAC(devNum,hwDataArr,
        SIP6_30_FDB_FDB_TABLE_FIELDS_DDE_TIMESTAMP_E,
        hwValue);

    hwValue = 0;
    for(ii = 0 ; ii < CPSS_DXCH_HSR_PRP_H_PORTS_NUM_CNS; ii++)
    {
        if(entryPtr->destHportCounters[ii] >= BIT_2)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, "entryPtr->destHportCounters[%d]=[%d] limited to [0..3]",
                (GT_U32)entryPtr->destHportCounters[ii]);
        }

        hwValue |= entryPtr->destHportCounters[ii] << (2*ii);
    }

    SIP6_FDB_ENTRY_FIELD_SET_MAC(devNum,hwDataArr,
        SIP6_30_FDB_FDB_TABLE_FIELDS_DDE_TARGET_COUNTERS_E,
        hwValue);

    /* write to the FDB table at the 'HW index' */
    hwIndex = index + PRV_CPSS_DXCH_PP_MAC(devNum)->hsrPrp.ddeInfo.ddeBaseIndexInFdbTable;

    /* write the entry to the FDB table */
    return prvCpssDxChWriteTableEntry(devNum,
         CPSS_DXCH_TABLE_FDB_E,
         hwIndex,
         &hwDataArr[0]);
}

/**
* @internal cpssDxChHsrPrpDdeEntryWrite function
* @endinternal
*
* @brief   Write an entry in DDE table in specified index.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum                - device number
* @param[in] index                 - The index of the entry.
*                                    APPLICABLE RANGE: 0..max
*                                    NOTE: 1. 'max' index depends on the mode selected
*                                             by CPSS_DXCH_HSR_PRP_FDB_PARTITION_MODE_ENT
*                                          2. The index can be calculated by cpssDxChHsrPrpDdeHashCalc(...)
* @param[in] infoPtr               - (pointer to) The entry info.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum,index
* @retval GT_OUT_OF_RANGE          - on out of range parameters in entryPtr :
*                                    ddeKey.seqNum , ddeKey.lreInstance , destBmp  , srcHPort  ,
*                                    timeInMicroSec , destHportCounters[]
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChHsrPrpDdeEntryWrite
(
    IN  GT_U8                        devNum,
    IN  GT_U32                       index,
    IN  CPSS_DXCH_HSR_PRP_DDE_ENTRY_STC   *entryPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChHsrPrpDdeEntryWrite);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, index, entryPtr));

    rc = internal_cpssDxChHsrPrpDdeEntryWrite(devNum, index, entryPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, index, entryPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssDxChHsrPrpDdeEntryRead function
* @endinternal
*
* @brief   Read an entry from DDE table in specified index.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum                - device number
* @param[in] index                 - The index of the entry.
*                                    APPLICABLE RANGE: 0..max
*                                    NOTE: 1. 'max' index depends on the mode selected
*                                             by CPSS_DXCH_HSR_PRP_FDB_PARTITION_MODE_ENT
*                                          2. The index can be calculated by cpssDxChHsrPrpDdeHashCalc(...)
*
* @param[out] infoPtr              - (pointer to) The entry info.
* @param[out] isValidPtr           - (pointer to) Is the entry valid.
*                                    if the entry is not valid (valid bit unset or not 'DDE' type) ,
*                                    the values are not parsed into infoPtr.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum,index
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
static GT_STATUS internal_cpssDxChHsrPrpDdeEntryRead
(
    IN  GT_U8                        devNum,
    IN  GT_U32                       index,
    OUT CPSS_DXCH_HSR_PRP_DDE_ENTRY_STC  *entryPtr,
    OUT GT_BOOL                     *isValidPtr
)
{
    GT_STATUS   rc;      /* return code */
    GT_U32      hwIndex; /* index to the FDB table of the HW */
    GT_U32      hwDataArr[SIP_6_30_FDB_ENTRY_WORDS_SIZE_CNS + 1];/* the 'indirect read' reads 5 words !(common with like sip5) */
    GT_U32      hwValue; /* value to write to HW */
    GT_U32      factor;  /* the time factor from HW value to 'micro-seconds' */
    GT_U32      ii;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E |
        CPSS_BOBCAT2_E | CPSS_BOBCAT3_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E |
        CPSS_ALDRIN2_E | CPSS_FALCON_E |  CPSS_AC5P_E |  CPSS_AC5X_E | CPSS_HARRIER_E);

    CPSS_NULL_PTR_CHECK_MAC(entryPtr);
    CPSS_NULL_PTR_CHECK_MAC(isValidPtr);

    /* check the DDE index */
    DDE_INDEX_CHECK_MAC(devNum,index);

    /* read from the FDB table at the 'HW index' */
    hwIndex = index + PRV_CPSS_DXCH_PP_MAC(devNum)->hsrPrp.ddeInfo.ddeBaseIndexInFdbTable;

    /* read the entry from the FDB table */
    rc = prvCpssDxChReadTableEntry(devNum,
         CPSS_DXCH_TABLE_FDB_E,
         hwIndex,
         &hwDataArr[0]);
    if(rc != GT_OK)
    {
        return rc;
    }

    SIP6_FDB_ENTRY_FIELD_GET_MAC(devNum,hwDataArr,
        SIP5_FDB_FDB_TABLE_FIELDS_VALID_E,
        hwValue);
    *isValidPtr = BIT2BOOL_MAC(hwValue);
    if(GT_FALSE == *isValidPtr)
    {
        /* we not parse non-valid entry */
        return GT_OK;
    }
    SIP6_FDB_ENTRY_FIELD_GET_MAC(devNum,hwDataArr,
        SIP5_FDB_FDB_TABLE_FIELDS_FDB_ENTRY_TYPE_E,
        hwValue);
    if(hwValue != HW_FDB_DDE_ENTRY_TYPE_CNS)/*'DDE' entry type*/
    {
        *isValidPtr = GT_FALSE;
        /* we not parse non-DDE entry */
        return GT_OK;
    }

    SIP6_30_FDB_DDE_ENTRY_FIELD_MAC_ADDR_GET_MAC(devNum, hwDataArr,
        &entryPtr->ddeKey.macAddr.arEther[0]);

    SIP6_FDB_ENTRY_FIELD_GET_MAC(devNum,hwDataArr,
        SIP5_FDB_FDB_TABLE_FIELDS_AGE_E,
        hwValue);
    entryPtr->ageBit = BIT2BOOL_MAC(hwValue);

    SIP6_FDB_ENTRY_FIELD_GET_MAC(devNum,hwDataArr,
        SIP6_30_FDB_FDB_TABLE_FIELDS_DDE_SEQUENCE_NUMBER_E,
        hwValue);
    entryPtr->ddeKey.seqNum = hwValue;

    SIP6_FDB_ENTRY_FIELD_GET_MAC(devNum,hwDataArr,
        SIP6_30_FDB_FDB_TABLE_FIELDS_DDE_TARGET_INSTANCE_E,
        hwValue);
    entryPtr->ddeKey.lreInstance = hwValue;

    SIP6_FDB_ENTRY_FIELD_GET_MAC(devNum,hwDataArr,
        SIP6_30_FDB_FDB_TABLE_FIELDS_DDE_TARGET_PORT_BITMAP_E,
        hwValue);
    entryPtr->destBmp = hwValue;

    SIP6_FDB_ENTRY_FIELD_GET_MAC(devNum,hwDataArr,
        SIP6_30_FDB_FDB_TABLE_FIELDS_DDE_TARGET_SOURCE_HPORT_E,
        hwValue);
    entryPtr->srcHPort = hwValue;

    /* get info about the time factor */
    rc = ddeTimerFactorGet(devNum,&factor);
    if(rc != GT_OK)
    {
        return rc;
    }

    SIP6_FDB_ENTRY_FIELD_GET_MAC(devNum,hwDataArr,
        SIP6_30_FDB_FDB_TABLE_FIELDS_DDE_TIMESTAMP_E,
        hwValue);
    entryPtr->timeInMicroSec = hwValue * factor;

    SIP6_FDB_ENTRY_FIELD_GET_MAC(devNum,hwDataArr,
        SIP6_30_FDB_FDB_TABLE_FIELDS_DDE_TARGET_COUNTERS_E,
        hwValue);

    for(ii = 0 ; ii < CPSS_DXCH_HSR_PRP_H_PORTS_NUM_CNS; ii++)
    {
        entryPtr->destHportCounters[ii] = (hwValue >> (2*ii)) & 0x3;
    }

    return GT_OK;
}

/**
* @internal cpssDxChHsrPrpDdeEntryRead function
* @endinternal
*
* @brief   Read an entry from DDE table in specified index.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum                - device number
* @param[in] index                 - The index of the entry.
*                                    APPLICABLE RANGE: 0..max
*                                    NOTE: 1. 'max' index depends on the mode selected
*                                             by CPSS_DXCH_HSR_PRP_FDB_PARTITION_MODE_ENT
*                                          2. The index can be calculated by cpssDxChHsrPrpDdeHashCalc(...)
*
* @param[out] infoPtr              - (pointer to) The entry info.
* @param[out] isValidPtr           - (pointer to) Is the entry valid.
*                                    if the entry is not valid (valid bit unset or not 'DDE' type) ,
*                                    the values are not parsed into infoPtr.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum,index
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChHsrPrpDdeEntryRead
(
    IN  GT_U8                        devNum,
    IN  GT_U32                       index,
    OUT CPSS_DXCH_HSR_PRP_DDE_ENTRY_STC  *entryPtr,
    OUT GT_BOOL                     *isValidPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChHsrPrpDdeEntryRead);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, index, entryPtr, isValidPtr));

    rc = internal_cpssDxChHsrPrpDdeEntryRead(devNum, index, entryPtr, isValidPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, index, entryPtr, isValidPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChHsrPrpDdeAgingApply function
* @endinternal
*
* @brief  Age a range of entries in the DDE table.
*       for each entry the operation is :
*        Ask to age the entry :
*        If the entry is not valid the operation is ignored
*        if the entry is fresh     (aged bit == 1) the entry will be aged (aged bit = 0).
*        if the entry already aged (aged bit == 0) the entry will be invalid (aged out). and counted.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum         - device number.
* @param[in] index          - The index of the entry.
*                             APPLICABLE RANGE: 0..max
*                             NOTE: 1. 'max' index depends on the mode selected
*                                      by CPSS_DXCH_HSR_PRP_FDB_PARTITION_MODE_ENT
*                                   2. The index can be calculated by cpssDxChHsrPrpDdeHashCalc(...)
* @param[in] numOfEntries   - The number of entries in the table to apply
*                             aging on , starting from 'index' (with wraparound support)
*                             NOTE: value 0xFFFFFFFF means 'Full Table' , starting from 'index'
*                               (not implicitly from index 0)
*                             APPLICABLE RANGE: 1..max , 0xFFFFFFFF
*
* @param[out] numOfEntriesDeletedPtr   - (pointer to) The number of entries that
*                               deleted (invalidated) due to the aging.
*                               NOTE: if NULL ignored.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or index or numOfEntries
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
static GT_STATUS internal_cpssDxChHsrPrpDdeAgingApply
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          index,
    IN  GT_U32                          numOfEntries,
    OUT GT_U32                          *numOfEntriesDeletedPtr
)
{
    GT_STATUS   rc;      /* return code */
    GT_U32      hwIndex; /* index to the FDB table of the HW */
    GT_U32      hwDataArr[SIP_6_30_FDB_ENTRY_WORDS_SIZE_CNS + 1];/* the 'indirect read' reads 5 words !(common with like sip5) */
    GT_U32      hwValue; /* value to write to HW */
    GT_U32      regAddr; /* register address */
    GT_U32      numEntriesAgingDone , numOfEntriesDeleted;
    GT_U32      bankId,indexInBank;
    GT_U32      oldTimeStamp;
    GT_BOOL     isDeleted;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E |
        CPSS_BOBCAT2_E | CPSS_BOBCAT3_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E |
        CPSS_ALDRIN2_E | CPSS_FALCON_E |  CPSS_AC5P_E |  CPSS_AC5X_E | CPSS_HARRIER_E);

    /* check the DDE index */
    DDE_INDEX_CHECK_MAC(devNum,index);
    /* check the number of entries to age */
    DDE_NUM_OF_ENTRIES_CHECK_MAC(devNum,numOfEntries);

    /* we can invalidate the entry only after we 'lock' this index , so the HW will not update it */
    regAddr  = PRV_DXCH_REG1_UNIT_FDB_MAC(devNum).FDBCore.hsrPrp.hsrPrpAging;

    numOfEntriesDeleted = 0;/* counter of aged out (deleted) due to aging */

    for(numEntriesAgingDone = 0; numEntriesAgingDone < numOfEntries; index++,numEntriesAgingDone++)
    {
        index %= PRV_CPSS_DXCH_PP_MAC(devNum)->hsrPrp.ddeInfo.ddeNumEntriesInFdbTable;

        /* read from the FDB table at the 'HW index' */
        hwIndex = index + PRV_CPSS_DXCH_PP_MAC(devNum)->hsrPrp.ddeInfo.ddeBaseIndexInFdbTable;

        bankId      = hwIndex % PRV_CPSS_DXCH_PP_MAC(devNum)->bridge.fdbHashParams.numOfBanks;
        indexInBank = hwIndex / PRV_CPSS_DXCH_PP_MAC(devNum)->bridge.fdbHashParams.numOfBanks;

        hwValue = 0x1 /*<DDE Enable To Be Deleted FDB Entry>*/ |
                  (bankId << 1) |  /* bits [4:1] are the bank    */
                  indexInBank << 5;/* bits[20:5] are the address */

        rc = prvCpssHwPpWriteRegister(devNum, regAddr, hwValue);
        if(rc != GT_OK)
        {
            return rc;
        }

        /* read the entry from the FDB table */
        rc = prvCpssDxChReadTableEntry(devNum,
             CPSS_DXCH_TABLE_FDB_E,
             hwIndex,
             &hwDataArr[0]);
        if(rc != GT_OK)
        {
            return rc;
        }

        SIP6_FDB_ENTRY_FIELD_GET_MAC(devNum,hwDataArr,
            SIP5_FDB_FDB_TABLE_FIELDS_VALID_E,
            hwValue);
        if(0 == hwValue)
        {
            /* the entry is not valid */
            continue;
        }
        SIP6_FDB_ENTRY_FIELD_GET_MAC(devNum,hwDataArr,
            SIP5_FDB_FDB_TABLE_FIELDS_FDB_ENTRY_TYPE_E,
            hwValue);
        if(hwValue != HW_FDB_DDE_ENTRY_TYPE_CNS)/*'DDE' entry type*/
        {
            /* we not care about this 'non-initialized' entry */
            continue;
        }

        SIP6_FDB_ENTRY_FIELD_GET_MAC(devNum,hwDataArr,
            SIP5_FDB_FDB_TABLE_FIELDS_AGE_E,
            hwValue);
        if(hwValue) /*AGE bit*/
        {
            /* set aging to 0 */
            SIP6_FDB_ENTRY_FIELD_SET_MAC(devNum,hwDataArr,
                SIP5_FDB_FDB_TABLE_FIELDS_AGE_E,
                0x0);
            /* update the entry in HW */
            rc = prvCpssDxChWriteTableEntry(devNum,
                 CPSS_DXCH_TABLE_FDB_E,
                 hwIndex,
                 &hwDataArr[0]);
            if(rc != GT_OK)
            {
                return rc;
            }
        }
        else
        {
            SIP6_FDB_ENTRY_FIELD_GET_MAC(devNum,hwDataArr,
                SIP6_30_FDB_FDB_TABLE_FIELDS_DDE_TIMESTAMP_E,
                oldTimeStamp);
            /* call to invalidate the entry */
            rc = ddeEntryInvalidate(devNum,hwIndex,oldTimeStamp,&isDeleted);
            if(rc != GT_OK)
            {
                return rc;
            }

            if(isDeleted)
            {
                /*count the deleted entry*/
                numOfEntriesDeleted++;
            }
        }
    }

    if(numOfEntriesDeletedPtr)
    {
        *numOfEntriesDeletedPtr = numOfEntriesDeleted;
    }

    /* release the LOCK */
    return prvCpssHwPpWriteRegister(devNum, regAddr, 0);

}

/**
* @internal cpssDxChHsrPrpDdeAgingApply function
* @endinternal
*
* @brief  Age a range of entries in the DDE table.
*       for each entry the operation is :
*        Ask to age the entry :
*        If the entry is not valid the operation is ignored
*        if the entry is fresh     (aged bit == 1) the entry will be aged (aged bit = 0).
*        if the entry already aged (aged bit == 0) the entry will be invalid (aged out). and counted.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum         - device number.
* @param[in] index          - The index of the entry.
*                             APPLICABLE RANGE: 0..max
*                             NOTE: 1. 'max' index depends on the mode selected
*                                      by CPSS_DXCH_HSR_PRP_FDB_PARTITION_MODE_ENT
*                                   2. The index can be calculated by cpssDxChHsrPrpDdeHashCalc(...)
* @param[in] numOfEntries   - The number of entries in the table to apply
*                             aging on , starting from 'index' (with wraparound support)
*                             NOTE: value 0xFFFFFFFF means 'Full Table' , starting from 'index'
*                               (not implicitly from index 0)
*                             APPLICABLE RANGE: 1..max , 0xFFFFFFFF
*
* @param[out] numOfEntriesDeletedPtr   - (pointer to) The number of entries that
*                               deleted (invalidated) due to the aging.
*                               NOTE: if NULL ignored.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or index or numOfEntries
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChHsrPrpDdeAgingApply
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          index,
    IN  GT_U32                          numOfEntries,
    OUT GT_U32                          *numOfEntriesDeletedPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChHsrPrpDdeAgingApply);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, index, numOfEntries, numOfEntriesDeletedPtr));

    rc = internal_cpssDxChHsrPrpDdeAgingApply(devNum, index, numOfEntries, numOfEntriesDeletedPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, index, numOfEntries, numOfEntriesDeletedPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssDxChHsrPrpDdeHashCalc function
* @endinternal
*
* @brief   This function calculates the hash index for the DDE key (into the DDE table).
*         NOTE: the function do not access the HW , and do only SW calculations.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum                - device number
* @param[in] entryKeyPtr           - (pointer to) The key of the entry.
* @param[in] startBank             - the first bank for hash result.
*                                   (APPLICABLE RANGES: 0..15)
* @param[in] numOfBanks            - number of banks for the hash result.
*                                   this value indicates the number of elements that will be
*                                   retrieved by hashArr[]
*                                   restriction of (numOfBanks + startBank) <= 16 .
*                                   (APPLICABLE RANGES: 1..16)
*
* @param[out] hashArr[]            - (array of) the hash results. index in this array is 'bank Id'
*                                   (index 0 will hold value relate to bank startBank).
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum or parameters in entryKeyPtr :
*                                    seqNum , lreInstance.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
static GT_STATUS internal_cpssDxChHsrPrpDdeHashCalc
(
    IN  GT_U8                           devNum,
    IN  CPSS_DXCH_HSR_PRP_DDE_HASH_KEY_STC   *entryKeyPtr,
    IN  GT_U32                          startBank,
    IN  GT_U32                          numOfBanks,
    OUT GT_U32                          hashArr[] /*maxArraySize=16*/
)
{
    GT_STATUS   rc;
    GT_U32  crcMultiHashArr[NUM_BANKS_MULTI_HASH_CNS];
    GT_U16  up1,cfi1,vid1,fid;
    GT_U32  ii;
    GT_U32  rightShift , factor;
    GT_U32  numOfMultipleHashes;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E |
        CPSS_BOBCAT2_E | CPSS_BOBCAT3_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E |
        CPSS_ALDRIN2_E | CPSS_FALCON_E |  CPSS_AC5P_E |  CPSS_AC5X_E | CPSS_HARRIER_E);

    CPSS_NULL_PTR_CHECK_MAC(entryKeyPtr);
    CPSS_NULL_PTR_CHECK_MAC(hashArr);

    if(PRV_CPSS_DXCH_PP_MAC(devNum)->hsrPrp.ddeInfoValid == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE,
            "The DDE table can't be accesses (at any index) as partition mode was not set for it in the FDB table");
    }
    switch(PRV_CPSS_DXCH_PP_MAC(devNum)->hsrPrp.ddeInfo.fdbPartitionMode)
    {
        case CPSS_DXCH_HSR_PRP_FDB_PARTITION_MODE_DDE_NONE_E:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE,
                "There is no DDE Partition in the FDB table , can't calculate hash index");
        case CPSS_DXCH_HSR_PRP_FDB_PARTITION_MODE_DDE_1_2_E:
            rightShift = 6;
            factor     = 1;
            break;
        case CPSS_DXCH_HSR_PRP_FDB_PARTITION_MODE_DDE_3_4_E:
            rightShift = 8;
            factor     = 6;
            break;
        case CPSS_DXCH_HSR_PRP_FDB_PARTITION_MODE_DDE_7_8_E:
            rightShift = 8;
            factor     = 7;
            break;
        case CPSS_DXCH_HSR_PRP_FDB_PARTITION_MODE_DDE_ALL_E:
            rightShift = 5;
            factor     = 1;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(PRV_CPSS_DXCH_PP_MAC(devNum)->hsrPrp.ddeInfo.fdbPartitionMode);
    }

    if(entryKeyPtr->seqNum >= BIT_16)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "entryKeyPtr->seqNum[0x%8.8x] limited to [0..0x%8.8x]",
            entryKeyPtr->seqNum,
            (BIT_16-1));
    }

    if(entryKeyPtr->lreInstance >= BIT_2)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "entryKeyPtr->lreInstance[%d] limited to [0..%d]",
            entryKeyPtr->lreInstance,
            (BIT_2-1));
    }

    /* the 3 fields that 'hide' in the seqNum */
    up1  = (entryKeyPtr->seqNum >> 13) &   0x7;/* 3 bits*/
    cfi1 = (entryKeyPtr->seqNum >> 12) &   0x1;/* 1 bit */
    vid1 =  entryKeyPtr->seqNum        & 0xFFF;/*12 bits*/

    /* <Instance>, Desc<UP1>, Desc<CFI1>} */
    fid  = (GT_U16)(cfi1 | up1 << 1 | entryKeyPtr->lreInstance << 4);

    rc = prvCpssDxChBrgFdbHashDdeByParamsCalc(
        entryKeyPtr->macAddr.arEther,fid,vid1,
        startBank,numOfBanks,crcMultiHashArr);

    if(rc != GT_OK)
    {
        return rc;
    }

    switch(PRV_CPSS_DXCH_PP_MAC(devNum)->bridge.fdbNumOfBitsPerBank)
    {
        case 2:
            numOfMultipleHashes = 4;
            break;
        case 3:
            numOfMultipleHashes = 8;
            break;
        default:
            numOfMultipleHashes = 16;
            break;
    }

    for(ii = 0 ; ii < numOfBanks ; ii++)
    {
        hashArr[ii] = (crcMultiHashArr[ii] * factor) >> rightShift;
        hashArr[ii] *= numOfMultipleHashes;
        hashArr[ii] += ii;
    }

    return GT_OK;
}

/**
* @internal cpssDxChHsrPrpDdeHashCalc function
* @endinternal
*
* @brief   This function calculates the hash index for the DDE key (into the DDE table).
*         NOTE: the function do not access the HW , and do only SW calculations.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum                - device number
* @param[in] entryKeyPtr           - (pointer to) The key of the entry.
* @param[in] startBank             - the first bank for hash result.
*                                   (APPLICABLE RANGES: 0..15)
* @param[in] numOfBanks            - number of banks for the hash result.
*                                   this value indicates the number of elements that will be
*                                   retrieved by hashArr[]
*                                   restriction of (numOfBanks + startBank) <= 16 .
*                                   (APPLICABLE RANGES: 1..16)
*
* @param[out] hashArr[]            - (array of) the hash results. index in this array is 'bank Id'
*                                   (index 0 will hold value relate to bank startBank).
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum or parameters in entryKeyPtr :
*                                    seqNum , lreInstance.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChHsrPrpDdeHashCalc
(
    IN  GT_U8                           devNum,
    IN  CPSS_DXCH_HSR_PRP_DDE_HASH_KEY_STC   *entryKeyPtr,
    IN  GT_U32                          startBank,
    IN  GT_U32                          numOfBanks,
    OUT GT_U32                          hashArr[] /*maxArraySize=16*/
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChHsrPrpDdeHashCalc);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, entryKeyPtr , startBank , numOfBanks , hashArr ));

    rc = internal_cpssDxChHsrPrpDdeHashCalc(devNum, entryKeyPtr , startBank , numOfBanks , hashArr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, entryKeyPtr , startBank , numOfBanks , hashArr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChHsrPrpPortTrgPrpBaseLsduOffsetSet function
* @endinternal
*
* @brief  Set the PRP LSDU offset on specific target physical port.
*         Offset to the LSDU as defined by the PRP standard for untagged VLAN packets
*         that are also without Timestamp tag.
*         The device automatically takes into account the VLAN tag size and
*         Timestamp tag in the LSDU size calculation.
*         The device doesn't automatically take into account the eDSA tag size
*         and the pushed tag size. In case the packets are sent with eDSA tag or
*         pushed tag this parameter that is per target port, should be set to
*         14 + the size of the tag.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum         - device number.
* @param[in] portNum        - The port number.
*                               APPLICABLE RANGE: physical port range.
* @param[in] lsduOffset     - The PRP lsdu Offset.
*                               APPLICABLE RANGE: 0..63
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or portNum
* @retval GT_OUT_OF_RANGE          - out of range lsduOffset
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
static GT_STATUS internal_cpssDxChHsrPrpPortTrgPrpBaseLsduOffsetSet
(
    IN  GT_U8                    devNum,
    IN  GT_PHYSICAL_PORT_NUM     portNum,
    IN  GT_U32                   lsduOffset
)
{
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E |
        CPSS_BOBCAT2_E | CPSS_BOBCAT3_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E |
        CPSS_ALDRIN2_E | CPSS_FALCON_E |  CPSS_AC5P_E |  CPSS_AC5X_E | CPSS_HARRIER_E);

    PRV_CPSS_DXCH_PHY_PORT_NUM_CHECK_MAC(devNum, portNum);

    LSDU_OFFSET_CHECK_MAC(lsduOffset);

    /* write to HA physical port 1 table */
    return prvCpssDxChWriteTableEntryField(devNum,
            CPSS_DXCH_SIP5_TABLE_HA_PHYSICAL_PORT_1_E,
            portNum,
            PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
            SIP6_30_HA_PHYSICAL_PORT_TABLE_1_FIELDS_PRP_BASE_LSDU_OFFSET_E, /* field name */
            PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
            lsduOffset);
}

/**
* @internal cpssDxChHsrPrpPortTrgPrpBaseLsduOffsetSet function
* @endinternal
*
* @brief  Set the PRP LSDU offset on specific target physical port.
*         Offset to the LSDU as defined by the PRP standard for untagged VLAN packets
*         that are also without Timestamp tag.
*         The device automatically takes into account the VLAN tag size and
*         Timestamp tag in the LSDU size calculation.
*         The device doesn't automatically take into account the eDSA tag size
*         and the pushed tag size. In case the packets are sent with eDSA tag or
*         pushed tag this parameter that is per target port, should be set to
*         14 + the size of the tag.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum         - device number.
* @param[in] portNum        - The port number.
*                               APPLICABLE RANGE: physical port range.
* @param[in] lsduOffset     - The PRP lsdu Offset.
*                               APPLICABLE RANGE: 0..63
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or portNum
* @retval GT_OUT_OF_RANGE          - out of range lsduOffset
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChHsrPrpPortTrgPrpBaseLsduOffsetSet
(
    IN  GT_U8                    devNum,
    IN  GT_PHYSICAL_PORT_NUM     portNum,
    IN  GT_U32                   lsduOffset
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChHsrPrpPortTrgPrpBaseLsduOffsetSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum , lsduOffset ));

    rc = internal_cpssDxChHsrPrpPortTrgPrpBaseLsduOffsetSet(devNum, portNum , lsduOffset);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum , lsduOffset));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChHsrPrpPortTrgPrpBaseLsduOffsetGet function
* @endinternal
*
* @brief  Get the PRP LSDU offset on specific target physical port.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum         - device number.
* @param[in] portNum        - The port number.
*                               APPLICABLE RANGE: physical port range.
*
* @param[out] lsduOffsetPtr - (pointer to) The PRP lsdu Offset.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or portNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
static GT_STATUS internal_cpssDxChHsrPrpPortTrgPrpBaseLsduOffsetGet
(
    IN  GT_U8                    devNum,
    IN  GT_PHYSICAL_PORT_NUM     portNum,
    OUT GT_U32                   *lsduOffsetPtr
)
{
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E |
        CPSS_BOBCAT2_E | CPSS_BOBCAT3_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E |
        CPSS_ALDRIN2_E | CPSS_FALCON_E |  CPSS_AC5P_E |  CPSS_AC5X_E | CPSS_HARRIER_E);

    PRV_CPSS_DXCH_PHY_PORT_NUM_CHECK_MAC(devNum, portNum);
    CPSS_NULL_PTR_CHECK_MAC(lsduOffsetPtr);

    /* read from HA physical port 1 table */
    return prvCpssDxChReadTableEntryField(devNum,
            CPSS_DXCH_SIP5_TABLE_HA_PHYSICAL_PORT_1_E,
            portNum,
            PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
            SIP6_30_HA_PHYSICAL_PORT_TABLE_1_FIELDS_PRP_BASE_LSDU_OFFSET_E, /* field name */
            PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
            lsduOffsetPtr);
}

/**
* @internal cpssDxChHsrPrpPortTrgPrpBaseLsduOffsetGet function
* @endinternal
*
* @brief  Get the PRP LSDU offset on specific target physical port.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum         - device number.
* @param[in] portNum        - The port number.
*                               APPLICABLE RANGE: physical port range.
*
* @param[out] lsduOffsetPtr - (pointer to) The PRP lsdu Offset.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or portNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChHsrPrpPortTrgPrpBaseLsduOffsetGet
(
    IN  GT_U8                    devNum,
    IN  GT_PHYSICAL_PORT_NUM     portNum,
    OUT GT_U32                   *lsduOffsetPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChHsrPrpPortTrgPrpBaseLsduOffsetGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum , lsduOffsetPtr ));

    rc = internal_cpssDxChHsrPrpPortTrgPrpBaseLsduOffsetGet(devNum, portNum , lsduOffsetPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum , lsduOffsetPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssDxChHsrPrpPortSrcHsrBaseLsduOffsetSet function
* @endinternal
*
* @brief  Set the HSR LSDU offset on specific source physical port.
*         offset to the LSDU as defined by HSR standard for untagged VLAN packets
*         that are also without Timestamp tag.
*         The device automatically takes into account the VLAN tag size and Timestamp tag
*         in the LSDU size calculation.
*         The device doesn't automatically take into account the eDSA tag size
*         and the pushed tag size.
*         in case the packet are received with eDSA tag or pushed tag over the
*         source port, this parameter that is per source port should be set to
*         14+the size of the tag.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum         - device number.
* @param[in] portNum        - The port number.
*                             APPLICABLE RANGE: physical port range.
* @param[in] lsduOffset     - The HSR LSDU Offset.
*                             APPLICABLE RANGE: 0..63
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or portNum
* @retval GT_OUT_OF_RANGE          - out of range lsduOffset
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
static GT_STATUS internal_cpssDxChHsrPrpPortSrcHsrBaseLsduOffsetSet
(
    IN  GT_U8                    devNum,
    IN  GT_PHYSICAL_PORT_NUM     portNum,
    IN  GT_U32                   lsduOffset
)
{
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E |
        CPSS_BOBCAT2_E | CPSS_BOBCAT3_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E |
        CPSS_ALDRIN2_E | CPSS_FALCON_E |  CPSS_AC5P_E |  CPSS_AC5X_E | CPSS_HARRIER_E);

    PRV_CPSS_DXCH_PHY_PORT_NUM_CHECK_MAC(devNum, portNum);

    LSDU_OFFSET_CHECK_MAC(lsduOffset);

    /* write to HA physical port 2 table */
    return prvCpssDxChWriteTableEntryField(devNum,
            CPSS_DXCH_SIP5_TABLE_HA_PHYSICAL_PORT_2_E,
            portNum,
            PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
            SIP6_30_HA_PHYSICAL_PORT_TABLE_2_FIELDS_HSR_BASE_LSDU_OFFSET_E, /* field name */
            PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
            lsduOffset);
}

/**
* @internal cpssDxChHsrPrpPortSrcHsrBaseLsduOffsetSet function
* @endinternal
*
* @brief  Set the HSR LSDU offset on specific source physical port.
*         offset to the LSDU as defined by HSR standard for untagged VLAN packets
*         that are also without Timestamp tag.
*         The device automatically takes into account the VLAN tag size and Timestamp tag
*         in the LSDU size calculation.
*         The device doesn't automatically take into account the eDSA tag size
*         and the pushed tag size.
*         in case the packet are received with eDSA tag or pushed tag over the
*         source port, this parameter that is per source port should be set to
*         14+the size of the tag.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum         - device number.
* @param[in] portNum        - The port number.
*                             APPLICABLE RANGE: physical port range.
* @param[in] lsduOffset     - The HSR LSDU Offset.
*                             APPLICABLE RANGE: 0..63
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or portNum
* @retval GT_OUT_OF_RANGE          - out of range lsduOffset
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChHsrPrpPortSrcHsrBaseLsduOffsetSet
(
    IN  GT_U8                    devNum,
    IN  GT_PHYSICAL_PORT_NUM     portNum,
    IN  GT_U32                   lsduOffset
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChHsrPrpPortSrcHsrBaseLsduOffsetSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum , lsduOffset ));

    rc = internal_cpssDxChHsrPrpPortSrcHsrBaseLsduOffsetSet(devNum, portNum , lsduOffset);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum , lsduOffset));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChHsrPrpPortSrcHsrBaseLsduOffsetGet function
* @endinternal
*
* @brief  Get the HSR LSDU offset on specific source physical port.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum         - device number.
* @param[in] portNum        - The port number.
*                             APPLICABLE RANGE: physical port range.
*
* @param[out] lsduOffsetPtr - (pointer to) The HSR LSDU Offset.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or portNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
static GT_STATUS internal_cpssDxChHsrPrpPortSrcHsrBaseLsduOffsetGet
(
    IN  GT_U8                    devNum,
    IN  GT_PHYSICAL_PORT_NUM     portNum,
    OUT GT_U32                   *lsduOffsetPtr
)
{
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E |
        CPSS_BOBCAT2_E | CPSS_BOBCAT3_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E |
        CPSS_ALDRIN2_E | CPSS_FALCON_E |  CPSS_AC5P_E |  CPSS_AC5X_E | CPSS_HARRIER_E);

    PRV_CPSS_DXCH_PHY_PORT_NUM_CHECK_MAC(devNum, portNum);
    CPSS_NULL_PTR_CHECK_MAC(lsduOffsetPtr);

    /* read from HA physical port 2 table */
    return prvCpssDxChReadTableEntryField(devNum,
            CPSS_DXCH_SIP5_TABLE_HA_PHYSICAL_PORT_2_E,
            portNum,
            PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
            SIP6_30_HA_PHYSICAL_PORT_TABLE_2_FIELDS_HSR_BASE_LSDU_OFFSET_E, /* field name */
            PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
            lsduOffsetPtr);
}

/**
* @internal cpssDxChHsrPrpPortSrcHsrBaseLsduOffsetGet function
* @endinternal
*
* @brief  Get the HSR LSDU offset on specific source physical port.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum         - device number.
* @param[in] portNum        - The port number.
*                             APPLICABLE RANGE: physical port range.
*
* @param[out] lsduOffsetPtr - (pointer to) The HSR LSDU Offset.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or portNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChHsrPrpPortSrcHsrBaseLsduOffsetGet
(
    IN  GT_U8                    devNum,
    IN  GT_PHYSICAL_PORT_NUM     portNum,
    OUT GT_U32                   *lsduOffsetPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChHsrPrpPortSrcHsrBaseLsduOffsetGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum , lsduOffsetPtr ));

    rc = internal_cpssDxChHsrPrpPortSrcHsrBaseLsduOffsetGet(devNum, portNum , lsduOffsetPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum , lsduOffsetPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}



