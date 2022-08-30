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
* @file cpssPxIngress.c
*
* @brief CPSS PX implementation for Ingress Processing.
*
*
* @version   1
********************************************************************************
*/

#define CPSS_LOG_IN_MODULE_ENABLE
#include <cpss/px/cpssPxTypes.h>
#include <cpss/generic/log/prvCpssPxGenDbLog.h>
#include <cpss/px/ingress/private/prvCpssPxIngressLog.h>
#include <cpss/px/ingress/cpssPxIngress.h>
#include <cpss/px/config/private/prvCpssPxInfo.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

/* Maximal TPID size in bytes */
#define CPSS_PX_INGRESS_TPID_BYTE_SIZE_MAX_CNS                      24
/* Number of the Source/Destination packet type format table entries */
#define CPSS_PX_INGRESS_PACKET_TYPE_FORMAT_TABLE_ENTRIES_MAX_CNS    32

/**************************** Private macro definitions ***********************/
/* macro sets UDBP data/mask */
#define PRV_CPSS_PX_UDBP_REG_FIELD_SET_MAC(udbp, reg) \
    U32_SET_FIELD_MAC(reg, 0,  8, udbp.udb[1]); \
    U32_SET_FIELD_MAC(reg, 8,  8, udbp.udb[0]);

/* macro gets UDBP data/mask */
#define PRV_CPSS_PX_UDBP_REG_FIELD_GET_MAC(udbp, reg) \
    udbp.udb[1] = U32_GET_FIELD_MAC(reg, 0,  8); \
    udbp.udb[0] = U32_GET_FIELD_MAC(reg, 8,  8);

/* macro gets MAC DA 2 LSB bytes data from register value */
#define PRV_CPSS_PX_MAC_DA_TWO_LSB_BYTES_REG_GET_MAC(macDa, reg) \
    macDa.arEther[5] = U32_GET_FIELD_MAC(reg, 0, 8); \
    macDa.arEther[4] = U32_GET_FIELD_MAC(reg, 8, 8);

/* macro gets MAC DA 4 MSB bytes data from register value */
#define PRV_CPSS_PX_MAC_DA_FOUR_MSB_BYTES_GET_MAC(macDa, reg) \
    macDa.arEther[3] = U32_GET_FIELD_MAC(reg,  0, 8); \
    macDa.arEther[2] = U32_GET_FIELD_MAC(reg,  8, 8); \
    macDa.arEther[1] = U32_GET_FIELD_MAC(reg, 16, 8); \
    macDa.arEther[0] = U32_GET_FIELD_MAC(reg, 24, 8);


/* macro checks UDBP values */
#define PRV_CPSS_PX_UDB_PAIR_KEY_CHECK_MAC(udbp) \
    if ((udbp).udbByteOffset >= BIT_6) { \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_ONE_PARAM_FORMAT_MAC((udbp).udbByteOffset)); \
    } \
    switch ((udbp).udbAnchorType){ \
        case CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_L2_E: \
        case CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_L3_E: \
            break; \
        default: \
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_ONE_PARAM_FORMAT_MAC((udbp).udbAnchorType)); \
    }

/*  Converts 32 bits negative signed index constant to 12/13 bits value and vice versa.
   The constant 0xFFFFFFFD should be converted to 0x1FFD for destination map table */
static GT_VOID prvCpssPxSignedIndexConstConvert
(
    IN CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_ENT tableType,
    IN GT_32                                   indexConst,
    OUT GT_32                                 *indexConstPtr
)
{
    GT_U32 signBitMask, valueMask;

    /* Original value */
    *indexConstPtr = indexConst;

    signBitMask = (tableType == CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_PORT_FILTERING_E) ? BIT_11 : BIT_12;
    if ((indexConst & signBitMask) == 0)
    {
        /* Value is positive */
        return;
    }

    /* Set mask for signed negative value including sign bit */
    valueMask = (signBitMask << 1) - 1;
    if (indexConst < 0)
    {
        /* Get 11 or 12 LSB + 1 MSB signed bit */
        *indexConstPtr = indexConst & valueMask;
    }
    else
    {
        /* Convert value to 32 bits signed negative value */
        *indexConstPtr = indexConst | ~valueMask;
    }

    return;
}


/* macro checks packet type format indexes */
#define PRV_CPSS_PX_PACKET_TYPE_FORMAT_INDEXES_CHECK_MAC(tableType, packetTypeFormatPtr) \
    if ((tableType) == CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_PORT_FILTERING_E){ \
        if ((packetTypeFormatPtr)->indexMax >= BIT_12) {\
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_ONE_PARAM_FORMAT_MAC((packetTypeFormatPtr)->indexMax)); \
        } \
        if (((packetTypeFormatPtr)->indexConst < -2048) || ((packetTypeFormatPtr)->indexConst > 2047)) {\
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_ONE_PARAM_FORMAT_MAC((packetTypeFormatPtr)->indexConst)); \
        } \
    } else { \
        if ((packetTypeFormatPtr)->indexMax >= BIT_13) {\
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_ONE_PARAM_FORMAT_MAC((packetTypeFormatPtr)->indexMax)); \
        } \
        if (((packetTypeFormatPtr)->indexConst < -4096) || ((packetTypeFormatPtr)->indexConst > 4095)) {\
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_ONE_PARAM_FORMAT_MAC((packetTypeFormatPtr)->indexConst)); \
        } \
    }

/**
* @internal internal_cpssPxIngressTpidEntrySet function
* @endinternal
*
* @brief   Set global TPID table entry.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] entryIndex               - entry index for TPID table (APPLICABLE RANGES: 0..3).
* @param[in] tpidEntryPtr             - (pointer to) global TPID entry
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxIngressTpidEntrySet
(
    IN  GT_SW_DEV_NUM                   devNum,
    IN  GT_U32                          entryIndex,
    IN  CPSS_PX_INGRESS_TPID_ENTRY_STC  *tpidEntryPtr
)
{
    GT_U32 regAddr;         /* register address */
    GT_U32 regValue = 0;    /* register value */
    GT_U32 regWriteBits;    /* number of bits to be written to register */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(tpidEntryPtr);
    if(entryIndex >= CPSS_PX_INGRESS_TPID_TABLE_ENTRIES_MAX_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_ONE_PARAM_FORMAT_MAC(entryIndex));
    }
    if (tpidEntryPtr->size & 1)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_ONE_PARAM_FORMAT_MAC(tpidEntryPtr->size));
    }
    if ((tpidEntryPtr->size == 0) || (tpidEntryPtr->size > CPSS_PX_INGRESS_TPID_BYTE_SIZE_MAX_CNS))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_ONE_PARAM_FORMAT_MAC(tpidEntryPtr->size));
    }

    /* valid TPID field */
    U32_SET_FIELD_MAC(regValue,  0,  1, tpidEntryPtr->valid);
    /* value TPID field */
    U32_SET_FIELD_MAC(regValue,  1, 16, tpidEntryPtr->val);
    /* size TPID field */
    U32_SET_FIELD_MAC(regValue, 17,  4, tpidEntryPtr->size);
    /* Write bits */
    regWriteBits = 21;

    regAddr = PRV_PX_REG1_UNIT_PCP_MAC(devNum).PRS.PCPTPIDTable[entryIndex];

    return prvCpssHwPpSetRegField(CAST_SW_DEVNUM(devNum), regAddr, 0, regWriteBits, regValue);
}

/**
* @internal cpssPxIngressTpidEntrySet function
* @endinternal
*
* @brief   Set global TPID table entry.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] entryIndex               - entry index for TPID table (APPLICABLE RANGES: 0..3).
* @param[in] tpidEntryPtr             - (pointer to) global TPID entry
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxIngressTpidEntrySet
(
    IN  GT_SW_DEV_NUM                   devNum,
    IN  GT_U32                          entryIndex,
    IN  CPSS_PX_INGRESS_TPID_ENTRY_STC  *tpidEntryPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxIngressTpidEntrySet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, entryIndex, tpidEntryPtr));

    rc = internal_cpssPxIngressTpidEntrySet(devNum, entryIndex, tpidEntryPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, entryIndex, tpidEntryPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxIngressTpidEntryGet function
* @endinternal
*
* @brief   Get global TPID table entry.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] entryIndex               - entry index for TPID table (APPLICABLE RANGES: 0..3).
*
* @param[out] tpidEntryPtr             - (pointer to) global TPID entry
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxIngressTpidEntryGet
(
    IN  GT_SW_DEV_NUM                   devNum,
    IN  GT_U32                          entryIndex,
    OUT CPSS_PX_INGRESS_TPID_ENTRY_STC  *tpidEntryPtr
)
{
    GT_STATUS rc;           /* return status */
    GT_U32 regAddr;         /* register address */
    GT_U32 regValue = 0;    /* register value */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(tpidEntryPtr);
    if(entryIndex >= CPSS_PX_INGRESS_TPID_TABLE_ENTRIES_MAX_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_ONE_PARAM_FORMAT_MAC(entryIndex));
    }

    regAddr = PRV_PX_REG1_UNIT_PCP_MAC(devNum).PRS.PCPTPIDTable[entryIndex];
    rc = prvCpssHwPpReadRegister(CAST_SW_DEVNUM(devNum), regAddr, &regValue);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* valid TPID field */
    tpidEntryPtr->valid = U32_GET_FIELD_MAC(regValue,  0,  1);
    /* value TPID field */
    tpidEntryPtr->val   = U32_GET_FIELD_MAC(regValue,  1, 16);
    /* size TPID field */
    tpidEntryPtr->size  = U32_GET_FIELD_MAC(regValue, 17,  4);

    return GT_OK;
}

/**
* @internal cpssPxIngressTpidEntryGet function
* @endinternal
*
* @brief   Get global TPID table entry.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] entryIndex               - entry index for TPID table (APPLICABLE RANGES: 0..3).
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The packet tags identified by it's TPID parsed over to locate
*       the Ethernet EtherType/Len field. The parser can parse up 4 tags,
*       where the tag size is determined by the TPID.
*
*/
GT_STATUS cpssPxIngressTpidEntryGet
(
    IN  GT_SW_DEV_NUM                   devNum,
    IN  GT_U32                          entryIndex,
    OUT CPSS_PX_INGRESS_TPID_ENTRY_STC  *tpidEntryPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxIngressTpidEntryGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, entryIndex, tpidEntryPtr));

    rc = internal_cpssPxIngressTpidEntryGet(devNum, entryIndex, tpidEntryPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, , entryIndex, tpidEntryPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/* Get Ethertype register info per config type */
static GT_STATUS prvCpssPxIngressEtherTypeConfigGet
(
    IN  GT_SW_DEV_NUM                   devNum,
    IN  CPSS_PX_INGRESS_ETHERTYPE_ENT   configType,
    OUT GT_U32                          *regAddrPtr,
    OUT GT_U32                          *etherTypeSizePtr
)
{
    *etherTypeSizePtr = 16;
    switch (configType)
    {
        case CPSS_PX_INGRESS_ETHERTYPE_IPV4_E:
            *regAddrPtr = PRV_PX_REG1_UNIT_PCP_MAC(devNum).PRS.PCPGlobalEtherTypeConfig0;
            break;
        case CPSS_PX_INGRESS_ETHERTYPE_IPV6_E:
            *regAddrPtr = PRV_PX_REG1_UNIT_PCP_MAC(devNum).PRS.PCPGlobalEtherTypeConfig1;
            break;
        case CPSS_PX_INGRESS_ETHERTYPE_MPLS1_E:
            *regAddrPtr = PRV_PX_REG1_UNIT_PCP_MAC(devNum).PRS.PCPGlobalEtherTypeConfig2;
            break;
        case CPSS_PX_INGRESS_ETHERTYPE_MPLS2_E:
            *regAddrPtr = PRV_PX_REG1_UNIT_PCP_MAC(devNum).PRS.PCPGlobalEtherTypeConfig3;
            break;
        case CPSS_PX_INGRESS_ETHERTYPE_IPV6_EH_E:
            *regAddrPtr = PRV_PX_REG1_UNIT_PCP_MAC(devNum).PRS.PCPGlobalEtherTypeConfig4;
            *etherTypeSizePtr = 8;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_ONE_PARAM_FORMAT_MAC(configType));
    }

    return GT_OK;
}

/**
* @internal internal_cpssPxIngressEtherTypeSet function
* @endinternal
*
* @brief   Set global ethertype configuration.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] configType               - type of ethertype to be configured
* @param[in] etherType                - EtherType for protocol recognition.
* @param[in] valid                    - EtherType entry status
*                                      GT_TRUE     - EtherType is valid,
*                                      GT_FALSE    - EtherType is not valid.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxIngressEtherTypeSet
(
    IN  GT_SW_DEV_NUM                   devNum,
    IN  CPSS_PX_INGRESS_ETHERTYPE_ENT   configType,
    IN  GT_U16                          etherType,
    IN  GT_BOOL                         valid
)
{
    GT_STATUS rc;           /* return status */
    GT_U32 regAddr;         /* register address */
    GT_U32 regValue = 0;    /* register value */
    GT_U32 ethTypeSize;     /* ethertype size in bits */
    GT_U32 regWriteBits;    /* number of bits to be written to register */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);

    rc = prvCpssPxIngressEtherTypeConfigGet(devNum, configType, &regAddr, &ethTypeSize);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* Set ethertype valid bit */
    U32_SET_FIELD_MAC(regValue, 0,  1, BOOL2BIT_MAC(valid));
    /* Set ethertype value */
    U32_SET_FIELD_MAC(regValue, 1, ethTypeSize, etherType);
    /* Write bits */
    regWriteBits = ethTypeSize + 1;

    return prvCpssHwPpSetRegField(CAST_SW_DEVNUM(devNum), regAddr, 0, regWriteBits, regValue);
}

/**
* @internal cpssPxIngressEtherTypeSet function
* @endinternal
*
* @brief   Set global ethertype configuration.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] configType               - type of ethertype to be configured
* @param[in] etherType                - EtherType for protocol recognition.
* @param[in] valid                    - EtherType entry status
*                                      GT_TRUE     - EtherType is valid,
*                                      GT_FALSE    - EtherType is not valid.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxIngressEtherTypeSet
(
    IN  GT_SW_DEV_NUM                   devNum,
    IN  CPSS_PX_INGRESS_ETHERTYPE_ENT   configType,
    IN  GT_U16                          etherType,
    IN  GT_BOOL                         valid
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxIngressEtherTypeSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, configType, etherType, valid));

    rc = internal_cpssPxIngressEtherTypeSet(devNum, configType, etherType, valid);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, configType, etherType, valid));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxIngressEtherTypeGet function
* @endinternal
*
* @brief   Get global ethertype configuration.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] configType               - type of ethertype
*
* @param[out] etherTypePtr             - (pointer to) EtherType for protocol recognition.
* @param[out] validPtr                 - (pointer to) EtherType entry status
*                                      GT_TRUE     - EtherType is valid,
*                                      GT_FALSE    - EtherType is not valid.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxIngressEtherTypeGet
(
    IN  GT_SW_DEV_NUM                   devNum,
    IN  CPSS_PX_INGRESS_ETHERTYPE_ENT   configType,
    OUT GT_U16                          *etherTypePtr,
    OUT GT_BOOL                         *validPtr
)
{
    GT_STATUS rc;           /* return status */
    GT_U32 regAddr;         /* register address */
    GT_U32 regValue;        /* register value */
    GT_U32 ethTypeSize;     /* ethertype size in bits */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(etherTypePtr);
    CPSS_NULL_PTR_CHECK_MAC(validPtr);

    rc = prvCpssPxIngressEtherTypeConfigGet(devNum, configType, &regAddr, &ethTypeSize);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = prvCpssHwPpReadRegister(CAST_SW_DEVNUM(devNum), regAddr, &regValue);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* Get ethertype valid bit */
    *validPtr = BIT2BOOL_MAC(U32_GET_FIELD_MAC(regValue,  0,  1));
    /* Get ethertype size */
    *etherTypePtr = (GT_U16)U32_GET_FIELD_MAC(regValue,  1,  ethTypeSize);

    return GT_OK;
}

/**
* @internal cpssPxIngressEtherTypeGet function
* @endinternal
*
* @brief   Get global ethertype configuration.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] configType               - type of ethertype
*
* @param[out] etherTypePtr             - (pointer to) EtherType for protocol recognition.
* @param[out] validPtr                 - (pointer to) EtherType entry status
*                                      GT_TRUE     - EtherType is valid,
*                                      GT_FALSE    - EtherType is not valid.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxIngressEtherTypeGet
(
    IN  GT_SW_DEV_NUM                   devNum,
    IN  CPSS_PX_INGRESS_ETHERTYPE_ENT   configType,
    OUT GT_U16                          *etherTypePtr,
    OUT GT_BOOL                         *validPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxIngressEtherTypeGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, configType, etherTypePtr, validPtr));

    rc = internal_cpssPxIngressEtherTypeGet(devNum, configType, etherTypePtr, validPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, configType, etherTypePtr, validPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxIngressPortMacDaByteOffsetSet function
* @endinternal
*
* @brief   Set byte offset of MAC DA on specified port.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - physical port number.
* @param[in] offset                   - byte  of MAC DA.
*                                      Only even values are applicable.
*                                      (APPLICABLE RANGES: 0..30)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxIngressPortMacDaByteOffsetSet
(
    IN  GT_SW_DEV_NUM                   devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  GT_U32                          offset
)
{
    GT_U32 regAddr;             /* register address */
    GT_U32 regValue;            /* register value */
    GT_U32 regWriteBits;        /* number of bits to be written to register */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PHY_PORT_NUM_CHECK_MAC(devNum, portNum);
    if (offset & 1)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_ONE_PARAM_FORMAT_MAC(offset));
    }
    if (offset > CPSS_PX_INGRESS_MAC_DA_BYTE_OFFSET_MAX_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_ONE_PARAM_FORMAT_MAC(offset));
    }

    regAddr = PRV_PX_REG1_UNIT_PCP_MAC(devNum).PRS.PCPPortDaOffsetConfig[portNum];
    /* DA offset */
    regValue = offset;
    regWriteBits = 5;

    return prvCpssHwPpSetRegField(CAST_SW_DEVNUM(devNum), regAddr, 0, regWriteBits, regValue);
}

/**
* @internal cpssPxIngressPortMacDaByteOffsetSet function
* @endinternal
*
* @brief   Set byte offset of MAC DA on specified port.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - physical port number.
* @param[in] offset                   - byte  of MAC DA.
*                                      Only even values are applicable.
*                                      (APPLICABLE RANGES: 0..30)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxIngressPortMacDaByteOffsetSet
(
    IN  GT_SW_DEV_NUM                   devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  GT_U32                          offset
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxIngressPortMacDaByteOffsetSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, offset));

    rc = internal_cpssPxIngressPortMacDaByteOffsetSet(devNum, portNum, offset);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, offset));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxIngressPortMacDaByteOffsetGet function
* @endinternal
*
* @brief   Get byte offset of MAC DA for specified port.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - physical port number.
*
* @param[out] offsetPtr                - (pointer to) byte offset of MAC DA.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxIngressPortMacDaByteOffsetGet
(
    IN  GT_SW_DEV_NUM                   devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    OUT GT_U32                          *offsetPtr
)
{
    GT_U32 regAddr;         /* register address */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PHY_PORT_NUM_CHECK_MAC(devNum, portNum);
    CPSS_NULL_PTR_CHECK_MAC(offsetPtr);

    regAddr = PRV_PX_REG1_UNIT_PCP_MAC(devNum).PRS.PCPPortDaOffsetConfig[portNum];
    /* DA offset */
    return prvCpssHwPpGetRegField(CAST_SW_DEVNUM(devNum), regAddr, 0, 5, offsetPtr);
}

/**
* @internal cpssPxIngressPortMacDaByteOffsetGet function
* @endinternal
*
* @brief   Get byte offset of MAC DA for specified port.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - physical port number.
*
* @param[out] offsetPtr                - (pointer to) byte offset of MAC DA.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxIngressPortMacDaByteOffsetGet
(
    IN  GT_SW_DEV_NUM                   devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    OUT GT_U32                          *offsetPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxIngressPortMacDaByteOffsetGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, offsetPtr));

    rc = internal_cpssPxIngressPortMacDaByteOffsetGet(devNum, portNum, offsetPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, offsetPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxIngressPortPacketTypeKeySet function
* @endinternal
*
* @brief   Set port packet type key generation info:
*         - source profile
*         - UDB pairs information
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - physical port number.
* @param[in] portKeyPtr               - (pointer to) port packet type key.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxIngressPortPacketTypeKeySet
(
    IN  GT_SW_DEV_NUM                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                        portNum,
    IN  CPSS_PX_INGRESS_PORT_PACKET_TYPE_KEY_STC    *portKeyPtr
)
{
    GT_STATUS rc;           /* return status */
    GT_U32 regAddr;         /* register address */
    GT_U32 regValue;        /* register value */
    GT_U32 i, step;         /* UDBP index, register field step */
    GT_U32 regWriteBits;    /* number of bits to be written to register */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PHY_PORT_NUM_CHECK_MAC(devNum, portNum);
    CPSS_NULL_PTR_CHECK_MAC(portKeyPtr);

    if (portKeyPtr->srcPortProfile >= CPSS_PX_INGRESS_SOURCE_PORT_PROFILE_MAX_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_ONE_PARAM_FORMAT_MAC(portKeyPtr->srcPortProfile));
    }

    /* PCP Port Packet Type Key Table <<%n>> */
    regAddr = PRV_PX_REG1_UNIT_PCP_MAC(devNum).PRS.PCPPortPktTypeKeyTable[portNum];

    regValue = 0;
    /* Source profile */
    U32_SET_FIELD_MAC(regValue, 0, 7, portKeyPtr->srcPortProfile);
    /* UDBP0..2 */
    for (i = 0; i < 3; i++)
    {
        PRV_CPSS_PX_UDB_PAIR_KEY_CHECK_MAC(portKeyPtr->portUdbPairArr[i]);

        step = 7 * i;
        /* Anchor Type */
        U32_SET_FIELD_MAC(regValue,  7 + step,  1, portKeyPtr->portUdbPairArr[i].udbAnchorType);
        /* Byte Offset */
        U32_SET_FIELD_MAC(regValue,  8 + step,  6, portKeyPtr->portUdbPairArr[i].udbByteOffset);
    }

    PRV_CPSS_PX_UDB_PAIR_KEY_CHECK_MAC(portKeyPtr->portUdbPairArr[3]);

    /* Write bits */
    regWriteBits = 28;

    rc = prvCpssHwPpSetRegField(CAST_SW_DEVNUM(devNum), regAddr, 0, regWriteBits, regValue);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* PCP Port Packet Type Key Table ext<<%n>> */
    regAddr = PRV_PX_REG1_UNIT_PCP_MAC(devNum).PRS.PCPPortPktTypeKeyTableExt[portNum];

    regValue = 0;
    /* UDBP3 */
    /* Anchor Type */
    U32_SET_FIELD_MAC(regValue,  0,  1, portKeyPtr->portUdbPairArr[3].udbAnchorType);
    /* Byte Offset */
    U32_SET_FIELD_MAC(regValue,  1,  6, portKeyPtr->portUdbPairArr[3].udbByteOffset);
    /* Write bits */
    regWriteBits = 7;

    return prvCpssHwPpSetRegField(CAST_SW_DEVNUM(devNum), regAddr, 0, regWriteBits, regValue);
}

/**
* @internal cpssPxIngressPortPacketTypeKeySet function
* @endinternal
*
* @brief   Set port packet type key generation info:
*         - source profile
*         - UDB pairs information
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - physical port number.
* @param[in] portKeyPtr               - (pointer to) port packet type key.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxIngressPortPacketTypeKeySet
(
    IN  GT_SW_DEV_NUM                       devNum,
    IN  GT_PHYSICAL_PORT_NUM                portNum,
    IN  CPSS_PX_INGRESS_PORT_PACKET_TYPE_KEY_STC    *portKeyPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxIngressPortPacketTypeKeySet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, portKeyPtr));

    rc = internal_cpssPxIngressPortPacketTypeKeySet(devNum, portNum, portKeyPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, portKeyPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxIngressPortPacketTypeKeyGet function
* @endinternal
*
* @brief   Get packet type key generation info:
*         - source profile
*         - UDB pairs information
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - physical port number.
*
* @param[out] portKeyPtr               - (pointer to) packet type key.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxIngressPortPacketTypeKeyGet
(
    IN  GT_SW_DEV_NUM                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                        portNum,
    OUT CPSS_PX_INGRESS_PORT_PACKET_TYPE_KEY_STC    *portKeyPtr
)
{
    GT_STATUS rc;           /* return status */
    GT_U32 regAddr;         /* register address */
    GT_U32 regValue;        /* register value */
    GT_U32 i, step;         /* UDBP index, register field step */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PHY_PORT_NUM_CHECK_MAC(devNum, portNum);
    CPSS_NULL_PTR_CHECK_MAC(portKeyPtr);

    /* PCP Port Packet Type Key Table <<%n>> */
    regAddr = PRV_PX_REG1_UNIT_PCP_MAC(devNum).PRS.PCPPortPktTypeKeyTable[portNum];
    rc = prvCpssHwPpReadRegister(CAST_SW_DEVNUM(devNum), regAddr, &regValue);
    if (rc != GT_OK)
    {
        return rc;
    }
    /* Source profile */
    portKeyPtr->srcPortProfile = U32_GET_FIELD_MAC(regValue,  0,  7);
    /* UDBP0..2 */
    for (i = 0; i < 3; i++)
    {
        step = 7 * i;
        /* Anchor Type */
        portKeyPtr->portUdbPairArr[i].udbAnchorType = U32_GET_FIELD_MAC(regValue,  7 + step,  1);
        /* Byte Offset */
        portKeyPtr->portUdbPairArr[i].udbByteOffset = U32_GET_FIELD_MAC(regValue,  8 + step,  6);
    }

    /* PCP Port Packet Type Key Table ext <<%n>> */
    regAddr = PRV_PX_REG1_UNIT_PCP_MAC(devNum).PRS.PCPPortPktTypeKeyTableExt[portNum];
    rc = prvCpssHwPpReadRegister(CAST_SW_DEVNUM(devNum), regAddr, &regValue);
    if (rc != GT_OK)
    {
        return rc;
    }
    /* UDBP3 */
    /* Anchor Type */
    portKeyPtr->portUdbPairArr[3].udbAnchorType = U32_GET_FIELD_MAC(regValue,  0,  1);
    /* Byte Offset */
    portKeyPtr->portUdbPairArr[3].udbByteOffset = U32_GET_FIELD_MAC(regValue,  1,  6);

    return GT_OK;
}

/**
* @internal cpssPxIngressPortPacketTypeKeyGet function
* @endinternal
*
* @brief   Get packet type key generation info:
*         - source profile
*         - UDB pairs information
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - physical port number.
*
* @param[out] portKeyPtr               - (pointer to) packet type key.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxIngressPortPacketTypeKeyGet
(
    IN  GT_SW_DEV_NUM                       devNum,
    IN  GT_PHYSICAL_PORT_NUM                portNum,
    OUT CPSS_PX_INGRESS_PORT_PACKET_TYPE_KEY_STC    *portKeyPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxIngressPortPacketTypeKeyGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, portKeyPtr));

    rc = internal_cpssPxIngressPortPacketTypeKeyGet(devNum, portNum, portKeyPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, portKeyPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/* Write Packet type register data and mask registers */
GT_STATUS prvCpssPxIngressPacketTypeRegisterWrite
(
    IN GT_SW_DEV_NUM    devNum,
    IN GT_U32           regDataAddr,
    IN GT_U32           regMaskAddr,
    IN GT_U32           regDataValue,
    IN GT_U32           regMaskValue,
    IN GT_U32           regWriteBits
)
{
    GT_STATUS rc;

    /* Write data register */
    rc = prvCpssHwPpSetRegField(CAST_SW_DEVNUM(devNum), regDataAddr, 0, regWriteBits, regDataValue);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* According to the functional spec API uses AND mask logic. But HW uses inverted one.
       Need to invert data before writing into HW */
    regMaskValue = ~regMaskValue;

    /* Write mask register */
    return prvCpssHwPpSetRegField(CAST_SW_DEVNUM(devNum), regMaskAddr, 0, regWriteBits, regMaskValue);
}

/* Set Packet type MAC data and mask */
static GT_STATUS prvCpssPxIngressPacketTypeMacSet
(
    IN GT_SW_DEV_NUM                                devNum,
    IN  CPSS_PX_PACKET_TYPE                         packetType,
    IN  CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC  *keyDataPtr,
    IN  CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC  *keyMaskPtr

)
{
    GT_STATUS rc;               /* return status */
    GT_U32 regDataAddr;         /* register data address */
    GT_U32 regDataValue;        /* register data value */
    GT_U32 regMaskAddr;         /* register mask address */
    GT_U32 regMaskValue;        /* register mask value */
    GT_U32 regWriteBits;        /* number of bits to be written to register */

    regDataAddr = PRV_PX_REG1_UNIT_PCP_MAC(devNum).PRS.PCPPktTypeKeyTableMACDA2LSBEntry[packetType];
    regMaskAddr = PRV_PX_REG1_UNIT_PCP_MAC(devNum).PRS.PCPPktTypeKeyTableMACDA2LSBEntryMask[packetType];

    regDataValue = 0;
    regMaskValue = 0;
    /* 2 MAC DA LSB   */
    U32_SET_FIELD_MAC(regDataValue, 0, 16, GT_HW_MAC_LOW16(&(keyDataPtr->macDa)));
    U32_SET_FIELD_MAC(regMaskValue, 0, 16, GT_HW_MAC_LOW16(&(keyMaskPtr->macDa)));
    /* Write bits */
    regWriteBits = 16;

    rc = prvCpssPxIngressPacketTypeRegisterWrite(devNum,
                                                 regDataAddr, regMaskAddr,
                                                 regDataValue, regMaskValue,
                                                 regWriteBits);
    if (rc != GT_OK)
    {
        return rc;
    }

    regDataAddr = PRV_PX_REG1_UNIT_PCP_MAC(devNum).PRS.PCPPktTypeKeyTableMACDA4MSBEntry[packetType];
    regMaskAddr = PRV_PX_REG1_UNIT_PCP_MAC(devNum).PRS.PCPPktTypeKeyTableMACDA4MSBEntryMask[packetType];

    /* 4 MAC DA MSB   */
    regDataValue = GT_HW_MAC_HIGH32(&(keyDataPtr->macDa));
    regMaskValue = GT_HW_MAC_HIGH32(&(keyMaskPtr->macDa));
    /* Write bits */
    regWriteBits = 32;

    return prvCpssPxIngressPacketTypeRegisterWrite(devNum,
                                                   regDataAddr, regMaskAddr,
                                                   regDataValue, regMaskValue,
                                                   regWriteBits);
}

/* Set Packet type UDBP data and mask */
static GT_STATUS prvCpssPxIngressPacketTypeUdbpSet
(
    IN GT_SW_DEV_NUM                                devNum,
    IN  CPSS_PX_PACKET_TYPE                         packetType,
    IN  CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC  *keyDataPtr,
    IN  CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC  *keyMaskPtr

)
{
    GT_STATUS rc;               /* return status */
    GT_U32 regDataAddr;         /* register data address */
    GT_U32 regDataValue;        /* register data value */
    GT_U32 regMaskAddr;         /* register mask address */
    GT_U32 regMaskValue;        /* register mask value */
    GT_U32 regWriteBits;        /* number of bits to be written to register */
    GT_U32  i;

    /* UDBP0..3 */
    for (i = 0; i < 4; i++)
    {
        regDataAddr = PRV_PX_REG1_UNIT_PCP_MAC(devNum).PRS.PCPPktTypeKeyTableUDBPEntry[packetType][i];
        regMaskAddr = PRV_PX_REG1_UNIT_PCP_MAC(devNum).PRS.PCPPktTypeKeyTableUDBPEntryMask[packetType][i];

        regDataValue = 0;
        regMaskValue = 0;

        /* UDBP Data */
        PRV_CPSS_PX_UDBP_REG_FIELD_SET_MAC(keyDataPtr->udbPairsArr[i], regDataValue);
        /* UDBP Mask */
        PRV_CPSS_PX_UDBP_REG_FIELD_SET_MAC(keyMaskPtr->udbPairsArr[i], regMaskValue);
        /* Write bits */
        regWriteBits = 16;

        rc = prvCpssPxIngressPacketTypeRegisterWrite(devNum,
                                                     regDataAddr, regMaskAddr,
                                                     regDataValue, regMaskValue,
                                                     regWriteBits);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    return GT_OK;
}

/* Set Packet type TPID EtherType data and mask */
static GT_STATUS prvCpssPxIngressPacketTypeTpidEtherTypeSet
(
    IN GT_SW_DEV_NUM                                devNum,
    IN  CPSS_PX_PACKET_TYPE                         packetType,
    IN  CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC  *keyDataPtr,
    IN  CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC  *keyMaskPtr

)
{
    GT_U32 regDataAddr;         /* register data address */
    GT_U32 regDataValue;        /* register data value */
    GT_U32 regMaskAddr;         /* register mask address */
    GT_U32 regMaskValue;        /* register mask value */
    GT_U32 regWriteBits;        /* number of bits to be written to register */

    /* TPID EtherType */
    regDataAddr = PRV_PX_REG1_UNIT_PCP_MAC(devNum).PRS.PCPPktTypeKeyTableTPIDEtherTypeEntry[packetType];
    regMaskAddr = PRV_PX_REG1_UNIT_PCP_MAC(devNum).PRS.PCPPktTypeKeyTableTPIDEtherTypeEntryMask[packetType];

    regDataValue = 0;
    regMaskValue = 0;

    U32_SET_FIELD_MAC(regDataValue, 0, 16, keyDataPtr->etherType);
    U32_SET_FIELD_MAC(regMaskValue, 0, 16, keyMaskPtr->etherType);
    /* Write bits */
    regWriteBits = 16;

    return prvCpssPxIngressPacketTypeRegisterWrite(devNum,
                                                   regDataAddr, regMaskAddr,
                                                   regDataValue, regMaskValue,
                                                   regWriteBits);
}

/* Set Packet type Source port profile data and mask */
static GT_STATUS prvCpssPxIngressPacketTypeSourcePortProfileSet
(
    IN GT_SW_DEV_NUM                                devNum,
    IN  CPSS_PX_PACKET_TYPE                         packetType,
    IN  CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC  *keyDataPtr,
    IN  CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC  *keyMaskPtr

)
{
    GT_U32 regDataAddr;         /* register data address */
    GT_U32 regDataValue;        /* register data value */
    GT_U32 regMaskAddr;         /* register mask address */
    GT_U32 regMaskValue;        /* register mask value */
    GT_U32 regWriteBits;        /* number of bits to be written to register */

    /* Source port profile */
    regDataAddr = PRV_PX_REG1_UNIT_PCP_MAC(devNum).PRS.PCPPktTypeKeyTableSRCPortProfileEntry[packetType];
    regMaskAddr = PRV_PX_REG1_UNIT_PCP_MAC(devNum).PRS.PCPPktTypeKeyTableSRCPortProfileEntryMask[packetType];

    regDataValue = 0;
    regMaskValue = 0;

    U32_SET_FIELD_MAC(regDataValue, 0, 7, keyDataPtr->profileIndex);
    U32_SET_FIELD_MAC(regMaskValue, 0, 7, keyMaskPtr->profileIndex);
    /* Write bits */
    regWriteBits = 7;

    return prvCpssPxIngressPacketTypeRegisterWrite(devNum,
                                                   regDataAddr, regMaskAddr,
                                                   regDataValue, regMaskValue,
                                                   regWriteBits);
}

/* Set Packet type LLC-Non-Snap data and mask */
static GT_STATUS prvCpssPxIngressPacketTypeLlcNonSnapSet
(
    IN GT_SW_DEV_NUM                                devNum,
    IN  CPSS_PX_PACKET_TYPE                         packetType,
    IN  CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC  *keyDataPtr,
    IN  CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC  *keyMaskPtr

)
{
    GT_U32 regDataAddr;         /* register data address */
    GT_U32 regDataValue;        /* register data value */
    GT_U32 regMaskAddr;         /* register mask address */
    GT_U32 regMaskValue;        /* register mask value */
    GT_U32 regWriteBits;        /* number of bits to be written to register */

    /* LLC-non-SNAP */
    regDataAddr = PRV_PX_REG1_UNIT_PCP_MAC(devNum).PRS.PCPPktTypeKeyTableIsLLCNonSnapEntry[packetType];
    regMaskAddr = PRV_PX_REG1_UNIT_PCP_MAC(devNum).PRS.PCPPktTypeKeyTableIsLLCNonSnapEntryMask[packetType];

    regDataValue = 0;
    regMaskValue = 0;

    U32_SET_FIELD_MAC(regDataValue, 0, 1, keyDataPtr->isLLCNonSnap);
    U32_SET_FIELD_MAC(regMaskValue, 0, 1, keyMaskPtr->isLLCNonSnap);
    /* Write bits */
    regWriteBits = 1;

    return prvCpssPxIngressPacketTypeRegisterWrite(devNum,
                                                   regDataAddr, regMaskAddr,
                                                   regDataValue, regMaskValue,
                                                   regWriteBits);
}

/* Set Packet type IP2ME data and mask */
static GT_STATUS prvCpssPxIngressPacketTypeIp2meIndexSet
(
    IN  GT_SW_DEV_NUM                               devNum,
    IN  CPSS_PX_PACKET_TYPE                         packetType,
    IN  CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC  *keyDataPtr,
    IN  CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC  *keyMaskPtr

)
{
    GT_U32 regDataAddr;         /* register data address */
    GT_U32 regDataValue;        /* register data value */
    GT_U32 regMaskAddr;         /* register mask address */
    GT_U32 regMaskValue;        /* register mask value */
    GT_U32 regWriteBits;        /* number of bits to be written to register */

    /* IP2ME */
    regDataAddr = PRV_PX_REG1_UNIT_PCP_MAC(devNum).PRS.
        PCPPacketTypeKeyTableIP2MEMatchIndexEntry[packetType];
    regMaskAddr = PRV_PX_REG1_UNIT_PCP_MAC(devNum).PRS.
        PCPPacketTypeKeyTableIP2MEMatchIndexEntryMask[packetType];

    regDataValue = 0;
    regMaskValue = 0;

    U32_SET_FIELD_MAC(regDataValue, 0, 3, keyDataPtr->ip2meIndex);
    U32_SET_FIELD_MAC(regMaskValue, 0, 3, keyMaskPtr->ip2meIndex);
    /* Write bits */
    regWriteBits = 3;

    return prvCpssPxIngressPacketTypeRegisterWrite(devNum,
                                                   regDataAddr, regMaskAddr,
                                                   regDataValue, regMaskValue,
                                                   regWriteBits);
}


/* Read Packet type register data and mask registers */
GT_STATUS prvCpssPxIngressPacketTypeRegisterRead
(
    IN GT_SW_DEV_NUM    devNum,
    IN GT_U32           regDataAddr,
    IN GT_U32           regMaskAddr,
    IN GT_U32           *regDataValuePtr,
    IN GT_U32           *regMaskValuePtr
)
{
    GT_STATUS rc;

    /* Read data register */
    rc = prvCpssHwPpReadRegister(CAST_SW_DEVNUM(devNum), regDataAddr, regDataValuePtr);
    if (rc != GT_OK)
    {
        return rc;
    }
    /* Read mask register */
    rc = prvCpssHwPpReadRegister(CAST_SW_DEVNUM(devNum), regMaskAddr, regMaskValuePtr);

    /* According to the functional spec API uses AND mask logic. But HW uses inverted one.
       Need to invert data after reading from HW */
    *regMaskValuePtr = ~(*regMaskValuePtr);

    return rc;
}

/* Get Packet type MAC data and mask */
static GT_STATUS prvCpssPxIngressPacketTypeMacGet
(
    IN GT_SW_DEV_NUM                                devNum,
    IN  CPSS_PX_PACKET_TYPE                         packetType,
    IN  CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC  *keyDataPtr,
    IN  CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC  *keyMaskPtr

)
{
    GT_STATUS rc;
    GT_U32  regDataAddr;
    GT_U32  regDataValue;
    GT_U32  regMaskAddr;
    GT_U32  regMaskValue;

    /* 2 MAC DA LSB   */
    regDataAddr = PRV_PX_REG1_UNIT_PCP_MAC(devNum).PRS.PCPPktTypeKeyTableMACDA2LSBEntry[packetType];
    regMaskAddr = PRV_PX_REG1_UNIT_PCP_MAC(devNum).PRS.PCPPktTypeKeyTableMACDA2LSBEntryMask[packetType];
    rc = prvCpssPxIngressPacketTypeRegisterRead(devNum,
                                                regDataAddr, regMaskAddr,
                                                &regDataValue, &regMaskValue);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* MAC DA data */
    PRV_CPSS_PX_MAC_DA_TWO_LSB_BYTES_REG_GET_MAC(keyDataPtr->macDa, regDataValue);
    /* MAC DA mask */
    PRV_CPSS_PX_MAC_DA_TWO_LSB_BYTES_REG_GET_MAC(keyMaskPtr->macDa, regMaskValue);

    /* 4 MAC DA MSB   */
    regDataAddr = PRV_PX_REG1_UNIT_PCP_MAC(devNum).PRS.PCPPktTypeKeyTableMACDA4MSBEntry[packetType];
    regMaskAddr = PRV_PX_REG1_UNIT_PCP_MAC(devNum).PRS.PCPPktTypeKeyTableMACDA4MSBEntryMask[packetType];
    rc = prvCpssPxIngressPacketTypeRegisterRead(devNum,
                                                regDataAddr, regMaskAddr,
                                                &regDataValue, &regMaskValue);

    /* MAC DA data */
    PRV_CPSS_PX_MAC_DA_FOUR_MSB_BYTES_GET_MAC(keyDataPtr->macDa, regDataValue);
    /* MAC DA mask */
    PRV_CPSS_PX_MAC_DA_FOUR_MSB_BYTES_GET_MAC(keyMaskPtr->macDa, regMaskValue);

    return GT_OK;
}

/* Get Packet type UDBP data and mask */
static GT_STATUS prvCpssPxIngressPacketTypeUdbpGet
(
    IN GT_SW_DEV_NUM                                devNum,
    IN  CPSS_PX_PACKET_TYPE                         packetType,
    IN  CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC  *keyDataPtr,
    IN  CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC  *keyMaskPtr

)
{
    GT_STATUS rc;
    GT_U32  regDataAddr;
    GT_U32  regDataValue;
    GT_U32  regMaskAddr;
    GT_U32  regMaskValue;
    GT_U32  i;

    /* UDBP0..3 */
    for (i = 0; i < 4; i++)
    {
        regDataAddr = PRV_PX_REG1_UNIT_PCP_MAC(devNum).PRS.PCPPktTypeKeyTableUDBPEntry[packetType][i];
        regMaskAddr = PRV_PX_REG1_UNIT_PCP_MAC(devNum).PRS.PCPPktTypeKeyTableUDBPEntryMask[packetType][i];
        rc = prvCpssPxIngressPacketTypeRegisterRead(devNum,
                                                    regDataAddr, regMaskAddr,
                                                    &regDataValue, &regMaskValue);
        if (rc != GT_OK)
        {
            return rc;
        }

        /* UDBP Data */
        PRV_CPSS_PX_UDBP_REG_FIELD_GET_MAC(keyDataPtr->udbPairsArr[i], regDataValue);
        /* UDBP Mask */
        PRV_CPSS_PX_UDBP_REG_FIELD_GET_MAC(keyMaskPtr->udbPairsArr[i], regMaskValue);
    }

    return GT_OK;
}

/* Get Packet type TPID EtherType data and mask */
static GT_STATUS prvCpssPxIngressPacketTypeTpidEtherTypeGet
(
    IN GT_SW_DEV_NUM                                devNum,
    IN  CPSS_PX_PACKET_TYPE                         packetType,
    IN  CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC  *keyDataPtr,
    IN  CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC  *keyMaskPtr

)
{
    GT_STATUS rc;
    GT_U32  regDataAddr;
    GT_U32  regDataValue;
    GT_U32  regMaskAddr;
    GT_U32  regMaskValue;

    /* TPID EtherType */
    regDataAddr = PRV_PX_REG1_UNIT_PCP_MAC(devNum).PRS.PCPPktTypeKeyTableTPIDEtherTypeEntry[packetType];
    regMaskAddr = PRV_PX_REG1_UNIT_PCP_MAC(devNum).PRS.PCPPktTypeKeyTableTPIDEtherTypeEntryMask[packetType];

    rc = prvCpssPxIngressPacketTypeRegisterRead(devNum,
                                                regDataAddr, regMaskAddr,
                                                &regDataValue, &regMaskValue);
    if (rc != GT_OK)
    {
        return rc;
    }

    keyDataPtr->etherType = U32_GET_FIELD_MAC(regDataValue, 0, 16);
    keyMaskPtr->etherType = U32_GET_FIELD_MAC(regMaskValue, 0, 16);

    return GT_OK;
}

/* Get Packet type Source port profile data and mask */
static GT_STATUS prvCpssPxIngressPacketTypeSourcePortProfileGet
(
    IN GT_SW_DEV_NUM                                devNum,
    IN  CPSS_PX_PACKET_TYPE                         packetType,
    IN  CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC  *keyDataPtr,
    IN  CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC  *keyMaskPtr

)
{
    GT_STATUS rc;
    GT_U32  regDataAddr;
    GT_U32  regDataValue;
    GT_U32  regMaskAddr;
    GT_U32  regMaskValue;

    /* Source port profile */
    regDataAddr = PRV_PX_REG1_UNIT_PCP_MAC(devNum).PRS.PCPPktTypeKeyTableSRCPortProfileEntry[packetType];
    regMaskAddr = PRV_PX_REG1_UNIT_PCP_MAC(devNum).PRS.PCPPktTypeKeyTableSRCPortProfileEntryMask[packetType];

    rc = prvCpssPxIngressPacketTypeRegisterRead(devNum,
                                                regDataAddr, regMaskAddr,
                                                &regDataValue, &regMaskValue);
    if (rc != GT_OK)
    {
        return rc;
    }

    keyDataPtr->profileIndex = U32_GET_FIELD_MAC(regDataValue, 0, 7);
    keyMaskPtr->profileIndex = U32_GET_FIELD_MAC(regMaskValue, 0, 7);

    return GT_OK;
}

/* Get Packet type LLC-Non-Snap data and mask */
static GT_STATUS prvCpssPxIngressPacketTypeLlcNonSnapGet
(
    IN GT_SW_DEV_NUM                                devNum,
    IN  CPSS_PX_PACKET_TYPE                         packetType,
    IN  CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC  *keyDataPtr,
    IN  CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC  *keyMaskPtr

)
{
    GT_STATUS rc;
    GT_U32  regDataAddr;
    GT_U32  regDataValue;
    GT_U32  regMaskAddr;
    GT_U32  regMaskValue;

    /* Source port profile */
    regDataAddr = PRV_PX_REG1_UNIT_PCP_MAC(devNum).PRS.PCPPktTypeKeyTableIsLLCNonSnapEntry[packetType];
    regMaskAddr = PRV_PX_REG1_UNIT_PCP_MAC(devNum).PRS.PCPPktTypeKeyTableIsLLCNonSnapEntryMask[packetType];

    rc = prvCpssPxIngressPacketTypeRegisterRead(devNum,
                                                regDataAddr, regMaskAddr,
                                                &regDataValue, &regMaskValue);
    if (rc != GT_OK)
    {
        return rc;
    }

    keyDataPtr->isLLCNonSnap = U32_GET_FIELD_MAC(regDataValue, 0, 1);
    keyMaskPtr->isLLCNonSnap = U32_GET_FIELD_MAC(regMaskValue, 0, 1);

    return GT_OK;
}

/* Get Packet type IP2ME data and mask */
static GT_STATUS prvCpssPxIngressPacketTypeIp2meIndexGet
(
    IN GT_SW_DEV_NUM                                devNum,
    IN  CPSS_PX_PACKET_TYPE                         packetType,
    IN  CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC  *keyDataPtr,
    IN  CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC  *keyMaskPtr

)
{
    GT_STATUS rc;
    GT_U32  regDataAddr;
    GT_U32  regDataValue;
    GT_U32  regMaskAddr;
    GT_U32  regMaskValue;

    /* IP2ME */
    regDataAddr = PRV_PX_REG1_UNIT_PCP_MAC(devNum).PRS.
        PCPPacketTypeKeyTableIP2MEMatchIndexEntry[packetType];
    regMaskAddr = PRV_PX_REG1_UNIT_PCP_MAC(devNum).PRS.
        PCPPacketTypeKeyTableIP2MEMatchIndexEntryMask[packetType];


    rc = prvCpssPxIngressPacketTypeRegisterRead(devNum,
                                                regDataAddr, regMaskAddr,
                                                &regDataValue, &regMaskValue);
    if (rc != GT_OK)
    {
        return rc;
    }

    keyDataPtr->ip2meIndex = U32_GET_FIELD_MAC(regDataValue, 0, 3);
    keyMaskPtr->ip2meIndex = U32_GET_FIELD_MAC(regMaskValue, 0, 3);

    return GT_OK;
}

/**
* @internal internal_cpssPxIngressPacketTypeKeyEntrySet function
* @endinternal
*
* @brief   Set packet type key table entry.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] packetType               - index of the packet type key in the table.
*                                      (APPLICABLE RANGES: 0..31)
* @param[in] keyDataPtr               - (pointer to) 17 bytes packet type key data.
* @param[in] keyMaskPtr               - (pointer to) 17 bytes packet type key mask.
*                                      The packet type key mask is AND styled one.
*                                      Mask bit's 0 means don't care bit (corresponding bit in
*                                      the pattern is not using in the TCAM like lookup).
*                                      Mask bit's 1 means that corresponding bit in the
*                                      pattern is using in the TCAM like lookup.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxIngressPacketTypeKeyEntrySet
(
    IN  GT_SW_DEV_NUM                               devNum,
    IN  CPSS_PX_PACKET_TYPE                         packetType,
    IN  CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC  *keyDataPtr,
    IN  CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC  *keyMaskPtr
)
{
    GT_STATUS rc;           /* return status */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(keyDataPtr);
    CPSS_NULL_PTR_CHECK_MAC(keyMaskPtr);

    if (packetType >= CPSS_PX_INGRESS_PACKET_TYPE_KEY_TABLE_ENTRIES_MAX_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_ONE_PARAM_FORMAT_MAC(packetType));
    }

    if (keyDataPtr->profileIndex >= CPSS_PX_INGRESS_SOURCE_PORT_PROFILE_MAX_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_ONE_PARAM_FORMAT_MAC(keyDataPtr->profileIndex));
    }

    if (keyMaskPtr->profileIndex >= CPSS_PX_INGRESS_SOURCE_PORT_PROFILE_MAX_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_ONE_PARAM_FORMAT_MAC(keyMaskPtr->profileIndex));
    }

    if (PRV_CPSS_PX_A1_AND_ABOVE_CHECK_MAC(devNum))
    {
        if(keyDataPtr->ip2meIndex > 7)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_ONE_PARAM_FORMAT_MAC(keyDataPtr->ip2meIndex));
        }

        if(keyMaskPtr->ip2meIndex > 7)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_ONE_PARAM_FORMAT_MAC(keyMaskPtr->ip2meIndex));
        }
    }

    /* Packet type MAC DA */
    rc = prvCpssPxIngressPacketTypeMacSet(devNum, packetType, keyDataPtr, keyMaskPtr);
    if (rc != GT_OK)
    {
        return rc;
    }
    /* Packet type UDBP */
    rc = prvCpssPxIngressPacketTypeUdbpSet(devNum, packetType, keyDataPtr, keyMaskPtr);
    if (rc != GT_OK)
    {
        return rc;
    }
    /* Packet type TPID EtherType */
    rc = prvCpssPxIngressPacketTypeTpidEtherTypeSet(devNum, packetType, keyDataPtr, keyMaskPtr);
    if (rc != GT_OK)
    {
        return rc;
    }
    /* Packet type Source Port Profile */
    rc = prvCpssPxIngressPacketTypeSourcePortProfileSet(devNum, packetType, keyDataPtr, keyMaskPtr);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* Packet type LLC-Non-SNAP */
    rc = prvCpssPxIngressPacketTypeLlcNonSnapSet(devNum, packetType, keyDataPtr, keyMaskPtr);
    if (rc != GT_OK)
    {
        return rc;
    }

    if (PRV_CPSS_PX_A1_AND_ABOVE_CHECK_MAC(devNum))
    {
        /* Packet type IP2ME index */
        rc = prvCpssPxIngressPacketTypeIp2meIndexSet(devNum, packetType, keyDataPtr, keyMaskPtr);
        if (rc != GT_OK)
        {
            return rc;
        }
    }
    return GT_OK;
}

/**
* @internal cpssPxIngressPacketTypeKeyEntrySet function
* @endinternal
*
* @brief   Set packet type key table entry.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] packetType               - index of the packet type key in the table.
*                                      (APPLICABLE RANGES: 0..31)
* @param[in] keyDataPtr               - (pointer to) 17 bytes packet type key data.
* @param[in] keyMaskPtr               - (pointer to) 17 bytes packet type key mask.
*                                      The packet type key mask is AND styled one.
*                                      Mask bit's 0 means don't care bit (corresponding bit in
*                                      the pattern is not using in the TCAM like lookup).
*                                      Mask bit's 1 means that corresponding bit in the
*                                      pattern is using in the TCAM like lookup.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxIngressPacketTypeKeyEntrySet
(
    IN  GT_SW_DEV_NUM                               devNum,
    IN  CPSS_PX_PACKET_TYPE                         packetType,
    IN  CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC  *keyDataPtr,
    IN  CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC  *keyMaskPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxIngressPacketTypeKeyEntrySet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, packetType, keyDataPtr, keyMaskPtr));

    rc = internal_cpssPxIngressPacketTypeKeyEntrySet(devNum, packetType, keyDataPtr, keyMaskPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, packetType, keyDataPtr, keyMaskPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxIngressPacketTypeKeyEntryGet function
* @endinternal
*
* @brief   Get packet type key table entry.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] packetType               - index of the packet type key in the table.
*                                      (APPLICABLE RANGES: 0..31)
*
* @param[out] keyDataPtr               - (pointer to) 17 bytes packet type key pattern.
* @param[out] keyMaskPtr               - (pointer to) 17 bytes packet type key mask.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxIngressPacketTypeKeyEntryGet
(
    IN  GT_SW_DEV_NUM                               devNum,
    IN  CPSS_PX_PACKET_TYPE                         packetType,
    OUT CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC  *keyDataPtr,
    OUT CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC  *keyMaskPtr
)
{
    GT_STATUS rc;

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(keyDataPtr);
    CPSS_NULL_PTR_CHECK_MAC(keyMaskPtr);

    if (packetType >= CPSS_PX_INGRESS_PACKET_TYPE_KEY_TABLE_ENTRIES_MAX_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_ONE_PARAM_FORMAT_MAC(packetType));
    }

    /* MAC DA */
    rc = prvCpssPxIngressPacketTypeMacGet(devNum, packetType, keyDataPtr, keyMaskPtr);
    if (rc != GT_OK)
    {
        return rc;
    }
    /* UDBP */
    rc = prvCpssPxIngressPacketTypeUdbpGet(devNum, packetType, keyDataPtr, keyMaskPtr);
    if (rc != GT_OK)
    {
        return rc;
    }
    /* TPID EtherType */
    rc = prvCpssPxIngressPacketTypeTpidEtherTypeGet(devNum, packetType, keyDataPtr, keyMaskPtr);
    if (rc != GT_OK)
    {
        return rc;
    }
    /* Source port profile */
    rc = prvCpssPxIngressPacketTypeSourcePortProfileGet(devNum, packetType, keyDataPtr, keyMaskPtr);
    if (rc != GT_OK)
    {
        return rc;
    }

    if (PRV_CPSS_PX_A1_AND_ABOVE_CHECK_MAC(devNum))
    {
        /* IP2ME index */
        rc = prvCpssPxIngressPacketTypeIp2meIndexGet(devNum, packetType, keyDataPtr, keyMaskPtr);
        if (rc != GT_OK)
        {
            return rc;
        }
    }
    else
    {
        keyDataPtr->ip2meIndex = 0;
        keyMaskPtr->ip2meIndex = 0;
    }

    /* LLC-non-SNAP */
    return prvCpssPxIngressPacketTypeLlcNonSnapGet(devNum, packetType, keyDataPtr, keyMaskPtr);
}

/**
* @internal cpssPxIngressPacketTypeKeyEntryGet function
* @endinternal
*
* @brief   Get packet type key table entry.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
*                                      entryIndex          - index of the packet type key in the table.
*                                      (APPLICABLE RANGES: 0..31)
*
* @param[out] keyDataPtr               - (pointer to) 17 bytes packet type key pattern.
* @param[out] keyMaskPtr               - (pointer to) 17 bytes packet type key mask.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxIngressPacketTypeKeyEntryGet
(
    IN  GT_SW_DEV_NUM                               devNum,
    IN  CPSS_PX_PACKET_TYPE                         packetType,
    OUT CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC  *keyDataPtr,
    OUT CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC  *keyMaskPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxIngressPacketTypeKeyEntryGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, packetType, keyDataPtr, keyMaskPtr));

    rc = internal_cpssPxIngressPacketTypeKeyEntryGet(devNum, packetType, keyDataPtr, keyMaskPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, packetType, keyDataPtr, keyMaskPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/* Set Packet Type Bit Field info */
static GT_STATUS prvCpssPxIngressPortMapPacketTypeFormatBitFieldInfoSet
(
    IN  GT_SW_DEV_NUM                                        devNum,
    IN  GT_U32                                               regAddr,
    IN  CPSS_PX_INGRESS_PACKET_BIT_FIELD_OFFSET_STC          *bitFieldArrPtr
)
{
    GT_STATUS regValue = 0;         /* register value */
    GT_U32 i, step;
    GT_U32 regWriteBits;            /* number of bits to be written to register */
    GT_U32 HW_startBit;

    for (i = 0; i < 2; i++)
    {
        if (bitFieldArrPtr[i].byteOffset >= BIT_6)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_ONE_PARAM_FORMAT_MAC(bitFieldArrPtr[i].byteOffset));
        }
        if (bitFieldArrPtr[i].startBit >= BIT_3)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_ONE_PARAM_FORMAT_MAC(bitFieldArrPtr[i].startBit));
        }
        if (bitFieldArrPtr[i].numBits > BIT_3)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_ONE_PARAM_FORMAT_MAC(bitFieldArrPtr[i].numBits));
        }
        if ((bitFieldArrPtr[i].startBit + bitFieldArrPtr[i].numBits) > BIT_3)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_ONE_PARAM_FORMAT_MAC((bitFieldArrPtr[i].startBit + bitFieldArrPtr[i].numBits)));
        }

        step = i * 13;
        /* PTS_Bit_Field<i>_Byte_Offset */
        U32_SET_FIELD_MAC(regValue, 0 + step, 6, bitFieldArrPtr[i].byteOffset);
        /* PTS_Bit_Field<i>_Start_Bit */
        /* NOTE: the HW hold ALL the bits in reverse order */
        HW_startBit = 0x7 & (8 - (bitFieldArrPtr[i].startBit + bitFieldArrPtr[i].numBits));
        U32_SET_FIELD_MAC(regValue, 6 + step, 3, HW_startBit);
        /* PTS_Bit_Field<i>_Number_Bits */
        U32_SET_FIELD_MAC(regValue, 9 + step, 4, bitFieldArrPtr[i].numBits);
    }
    /* Write bits */
    regWriteBits = 26;

    return prvCpssHwPpSetRegField(CAST_SW_DEVNUM(devNum), regAddr, 0, regWriteBits, regValue);
}

/* Get Packet Type Bit Field info */
static GT_STATUS prvCpssPxIngressPortMapPacketTypeFormatBitFieldInfoGet
(
    IN  GT_SW_DEV_NUM                                        devNum,
    IN  GT_U32                                               regAddr,
    OUT CPSS_PX_INGRESS_PACKET_BIT_FIELD_OFFSET_STC          *bitFieldArrPtr
)
{
    GT_STATUS rc;                   /* return status */
    GT_STATUS regValue;             /* register value */
    GT_U32 i, step;
    GT_U32 HW_startBit;

    rc = prvCpssHwPpReadRegister(CAST_SW_DEVNUM(devNum), regAddr, &regValue);
    if (rc != GT_OK)
    {
        return rc;
    }

    for (i = 0; i < 2; i++)
    {
        step = i * 13;
        /* PTS_Bit_Field<i>_Byte_Offset */
        bitFieldArrPtr[i].byteOffset = U32_GET_FIELD_MAC(regValue, 0 + step, 6);
        /* PTS_Bit_Field<i>_Start_Bit */
        HW_startBit = U32_GET_FIELD_MAC(regValue, 6 + step, 3);
        /* PTS_Bit_Field<i>_Number_Bits */
        bitFieldArrPtr[i].numBits = U32_GET_FIELD_MAC(regValue, 9 + step, 4);

        /* NOTE: the HW hold ALL the bits in reverse order */
        bitFieldArrPtr[i].startBit = 0x7 & (8 - (bitFieldArrPtr[i].numBits + HW_startBit));
    }

    return GT_OK;
}

/* Set Packet Type Source Format entry */
static GT_STATUS prvCpssPxIngressPortMapPacketTypeSourceFormatEntrySet
(
    IN GT_SW_DEV_NUM                                        devNum,
    IN  CPSS_PX_PACKET_TYPE                                 packetType,
    IN  CPSS_PX_INGRESS_PORT_MAP_PACKET_TYPE_FORMAT_STC     *packetTypeFormatPtr
)
{
    GT_STATUS rc;               /* return status */
    GT_U32 regAddr;             /* register address */
    GT_U32 regValue;            /* register value */
    GT_U32 regWriteBits;        /* number of bits to be written to register */
    GT_32  indexConst;          /* two's complement signed index constant */

    PRV_CPSS_PX_PACKET_TYPE_FORMAT_INDEXES_CHECK_MAC(CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_PORT_FILTERING_E, packetTypeFormatPtr);

    /* Packet Type Source Format 0 entry <<%n>> */
    regAddr = PRV_PX_REG1_UNIT_PCP_MAC(devNum).PORTMAP.PCPPktTypeSourceFormat0Entry[packetType];

    prvCpssPxSignedIndexConstConvert(CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_PORT_FILTERING_E, packetTypeFormatPtr->indexConst, &indexConst);

    regValue = 0;
    /* PTS_Src_Idx_Constant */
    U32_SET_FIELD_MAC(regValue, 0,  12, indexConst);
    /* PTS_Max_Src_Idx */
    U32_SET_FIELD_MAC(regValue, 12, 12, packetTypeFormatPtr->indexMax);
    /* Write bits */
    regWriteBits = 24;

    rc = prvCpssHwPpSetRegField(CAST_SW_DEVNUM(devNum), regAddr, 0, regWriteBits, regValue);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* Packet Type Source Format 1 entry <<%n>> */
    regAddr = PRV_PX_REG1_UNIT_PCP_MAC(devNum).PORTMAP.PCPPktTypeSourceFormat1Entry[packetType];
    rc = prvCpssPxIngressPortMapPacketTypeFormatBitFieldInfoSet(devNum, regAddr,
                                                                &packetTypeFormatPtr->bitFieldArr[0]);
    if (rc != GT_OK)
    {
        return rc;
    }
    /* Packet Type Source Format 2 entry <<%n>> */
    regAddr = PRV_PX_REG1_UNIT_PCP_MAC(devNum).PORTMAP.PCPPktTypeSourceFormat2Entry[packetType];


    return prvCpssPxIngressPortMapPacketTypeFormatBitFieldInfoSet(devNum, regAddr,
                                                                  &packetTypeFormatPtr->bitFieldArr[2]);
}

/* Set Packet Type Destination Format entry */
static GT_STATUS prvCpssPxIngressPortMapPacketTypeDestinationFormatEntrySet
(
    IN GT_SW_DEV_NUM                                        devNum,
    IN  CPSS_PX_PACKET_TYPE                                 packetType,
    IN  CPSS_PX_INGRESS_PORT_MAP_PACKET_TYPE_FORMAT_STC     *packetTypeFormatPtr

)
{
    GT_STATUS rc;               /* return status */
    GT_U32 regAddr;             /* register address */
    GT_U32 regValue;            /* register value */
    GT_U32 regWriteBits;             /* number of bits to be written to register */
    GT_32  indexConst;          /* two's complement signed index constant */

    PRV_CPSS_PX_PACKET_TYPE_FORMAT_INDEXES_CHECK_MAC(CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_DESTINATION_E, packetTypeFormatPtr);

    /* Packet Type Destination Format 0 entry <<%n>> */
    regAddr = PRV_PX_REG1_UNIT_PCP_MAC(devNum).PORTMAP.PCPPktTypeDestinationFormat0Entry[packetType];

    prvCpssPxSignedIndexConstConvert(CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_DESTINATION_E, packetTypeFormatPtr->indexConst, &indexConst);

    regValue = 0;
    /* PTS_Dst_Idx_Constant */
    U32_SET_FIELD_MAC(regValue, 0,  13, indexConst);
    /* PTS_Max_Dst_Idx */
    U32_SET_FIELD_MAC(regValue, 13, 13, packetTypeFormatPtr->indexMax);
    /* Write bits */
    regWriteBits = 26;

    rc = prvCpssHwPpSetRegField(CAST_SW_DEVNUM(devNum), regAddr, 0, regWriteBits, regValue);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* Packet Type Destination Format 1 entry <<%n>> */
    regAddr = PRV_PX_REG1_UNIT_PCP_MAC(devNum).PORTMAP.PCPPktTypeDestinationFormat1Entry[packetType];
    rc = prvCpssPxIngressPortMapPacketTypeFormatBitFieldInfoSet(devNum, regAddr,
                                                                &packetTypeFormatPtr->bitFieldArr[0]);
    if (rc != GT_OK)
    {
        return rc;
    }


    /* Packet Type Destination Format 2 entry <<%n>> */
    regAddr = PRV_PX_REG1_UNIT_PCP_MAC(devNum).PORTMAP.PCPPktTypeDestinationFormat2Entry[packetType];

    return prvCpssPxIngressPortMapPacketTypeFormatBitFieldInfoSet(devNum, regAddr,
                                                                &packetTypeFormatPtr->bitFieldArr[2]);
}

/* Get Packet Type Source Format entry */
static GT_STATUS prvCpssPxIngressPortMapPacketTypeSourceFormatEntryGet
(
    IN  GT_SW_DEV_NUM                                       devNum,
    IN  CPSS_PX_PACKET_TYPE                                 packetType,
    OUT CPSS_PX_INGRESS_PORT_MAP_PACKET_TYPE_FORMAT_STC     *packetTypeFormatPtr
)
{
    GT_STATUS rc;
    GT_U32  regAddr;
    GT_U32  regValue;
    GT_32   indexConst;          /* two's complement signed index constant */

    /* Packet Type Source Format 0 entry <<%n>> */
    regAddr = PRV_PX_REG1_UNIT_PCP_MAC(devNum).PORTMAP.PCPPktTypeSourceFormat0Entry[packetType];
    rc = prvCpssHwPpReadRegister(CAST_SW_DEVNUM(devNum), regAddr, &regValue);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* PTS_Src_Idx_Constant */
    indexConst = U32_GET_FIELD_MAC(regValue, 0,  12);
    /* PTS_Max_Src_Idx */
    packetTypeFormatPtr->indexMax = U32_GET_FIELD_MAC(regValue, 12, 12);

    prvCpssPxSignedIndexConstConvert(CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_PORT_FILTERING_E, indexConst, &packetTypeFormatPtr->indexConst);

    /* Packet Type Source Format 1 entry <<%n>> */
    regAddr = PRV_PX_REG1_UNIT_PCP_MAC(devNum).PORTMAP.PCPPktTypeSourceFormat1Entry[packetType];
    rc = prvCpssPxIngressPortMapPacketTypeFormatBitFieldInfoGet(devNum, regAddr,
                                                                &packetTypeFormatPtr->bitFieldArr[0]);
    if (rc != GT_OK)
    {
        return rc;
    }
    /* Packet Type Source Format 2 entry <<%n>> */
    regAddr = PRV_PX_REG1_UNIT_PCP_MAC(devNum).PORTMAP.PCPPktTypeSourceFormat2Entry[packetType];


    return prvCpssPxIngressPortMapPacketTypeFormatBitFieldInfoGet(devNum, regAddr,
                                                                  &packetTypeFormatPtr->bitFieldArr[2]);
}

/* Get Packet Type Destination Format entry */
static GT_STATUS prvCpssPxIngressPortMapPacketTypeDestinationFormatEntryGet
(
    IN  GT_SW_DEV_NUM                                       devNum,
    IN  CPSS_PX_PACKET_TYPE                                 packetType,
    OUT CPSS_PX_INGRESS_PORT_MAP_PACKET_TYPE_FORMAT_STC     *packetTypeFormatPtr
)
{
    GT_STATUS rc;
    GT_U32  regAddr;
    GT_U32  regValue;
    GT_32   indexConst;          /* two's complement signed index constant */

    /* Packet Type Destination Format 0 entry <<%n>> */
    regAddr = PRV_PX_REG1_UNIT_PCP_MAC(devNum).PORTMAP.PCPPktTypeDestinationFormat0Entry[packetType];
    rc = prvCpssHwPpReadRegister(CAST_SW_DEVNUM(devNum), regAddr, &regValue);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* PTS_Src_Idx_Constant */
    indexConst = U32_GET_FIELD_MAC(regValue, 0,  13);
    /* PTS_Max_Src_Idx */
    packetTypeFormatPtr->indexMax = U32_GET_FIELD_MAC(regValue, 13, 13);

    prvCpssPxSignedIndexConstConvert(CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_DESTINATION_E, indexConst, &packetTypeFormatPtr->indexConst);

    /* Packet Type Destination Format 1 entry <<%n>> */
    regAddr = PRV_PX_REG1_UNIT_PCP_MAC(devNum).PORTMAP.PCPPktTypeDestinationFormat1Entry[packetType];
    rc = prvCpssPxIngressPortMapPacketTypeFormatBitFieldInfoGet(devNum, regAddr,
                                                                &packetTypeFormatPtr->bitFieldArr[0]);
    if (rc != GT_OK)
    {
        return rc;
    }
    /* Packet Type Destination Format 2 entry <<%n>> */
    regAddr = PRV_PX_REG1_UNIT_PCP_MAC(devNum).PORTMAP.PCPPktTypeDestinationFormat2Entry[packetType];


    return prvCpssPxIngressPortMapPacketTypeFormatBitFieldInfoGet(devNum, regAddr,
                                                                  &packetTypeFormatPtr->bitFieldArr[2]);
}

/**
* @internal internal_cpssPxIngressPortMapPacketTypeFormatEntrySet function
* @endinternal
*
* @brief   Set Source/Destination information for packet type in the format table,
*         used to extract the information from the packet forwarding tag (DSA or E-Tag).
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] tableType                - Source/Destination port map table.
* @param[in] packetType               - index of the packet type format.
*                                      in the Source/Destination format table.
*                                      (APPLICABLE RANGES: 0..31)
* @param[in] packetTypeFormatPtr      - (pointer to) Source/Destination packet type format
*                                      in the Source/Destination format table.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxIngressPortMapPacketTypeFormatEntrySet
(
    IN  GT_SW_DEV_NUM                                       devNum,
    IN  CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_ENT             tableType,
    IN  CPSS_PX_PACKET_TYPE                                 packetType,
    IN  CPSS_PX_INGRESS_PORT_MAP_PACKET_TYPE_FORMAT_STC     *packetTypeFormatPtr
)
{
    GT_STATUS rc;           /* return status */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(packetTypeFormatPtr);

    if (packetType >= CPSS_PX_INGRESS_PACKET_TYPE_FORMAT_TABLE_ENTRIES_MAX_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_ONE_PARAM_FORMAT_MAC(packetType));
    }

    switch (tableType)
    {
        case CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_PORT_FILTERING_E:
            rc = prvCpssPxIngressPortMapPacketTypeSourceFormatEntrySet(devNum, packetType, packetTypeFormatPtr);
            if (rc != GT_OK)
            {
                return rc;
            }
            break;
        case CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_DESTINATION_E:
            rc = prvCpssPxIngressPortMapPacketTypeDestinationFormatEntrySet(devNum, packetType, packetTypeFormatPtr);
            if (rc != GT_OK)
            {
                return rc;
            }
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_ONE_PARAM_FORMAT_MAC(tableType));
    }

    return GT_OK;
}

/**
* @internal cpssPxIngressPortMapPacketTypeFormatEntrySet function
* @endinternal
*
* @brief   Set Source/Destination information for packet type in the format table,
*         used to extract the information from the packet forwarding tag (DSA or E-Tag).
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] tableType                - Source/Destination port map table.
* @param[in] packetType               - index of the packet type format.
*                                      in the Source/Destination format table.
*                                      (APPLICABLE RANGES: 0..31)
* @param[in] packetTypeFormatPtr      - (pointer to) Source/Destination packet type format
*                                      in the Source/Destination format table.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxIngressPortMapPacketTypeFormatEntrySet
(
    IN  GT_SW_DEV_NUM                                       devNum,
    IN  CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_ENT             tableType,
    IN  CPSS_PX_PACKET_TYPE                                 packetType,
    IN  CPSS_PX_INGRESS_PORT_MAP_PACKET_TYPE_FORMAT_STC     *packetTypeFormatPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxIngressPortMapPacketTypeFormatEntrySet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, tableType, packetType, packetTypeFormatPtr));

    rc = internal_cpssPxIngressPortMapPacketTypeFormatEntrySet(devNum, tableType, packetType, packetTypeFormatPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, tableType, packetType, packetTypeFormatPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxIngressPortMapPacketTypeFormatEntryGet function
* @endinternal
*
* @brief   Get Source/Destination information for packet type in the format table entry.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] tableType                - Source/Destination port map table.
* @param[in] packetType               - index of the packet type format
*                                      in the Source/Destination format table.
*                                      (APPLICABLE RANGES: 0..31)
*
* @param[out] packetTypeFormatPtr      - (pointer to) Source/Destination packet type format
*                                      in the Source/Destination format table.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxIngressPortMapPacketTypeFormatEntryGet
(
    IN  GT_SW_DEV_NUM                                       devNum,
    IN  CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_ENT             tableType,
    IN  CPSS_PX_PACKET_TYPE                                 packetType,
    OUT CPSS_PX_INGRESS_PORT_MAP_PACKET_TYPE_FORMAT_STC     *packetTypeFormatPtr
)
{
    GT_STATUS rc;           /* return status */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(packetTypeFormatPtr);

    if (packetType >= CPSS_PX_INGRESS_PACKET_TYPE_FORMAT_TABLE_ENTRIES_MAX_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_ONE_PARAM_FORMAT_MAC(packetType));
    }

    switch (tableType)
    {
        case CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_PORT_FILTERING_E:
            rc = prvCpssPxIngressPortMapPacketTypeSourceFormatEntryGet(devNum, packetType, packetTypeFormatPtr);
            if (rc != GT_OK)
            {
                return rc;
            }
            break;
        case CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_DESTINATION_E:
            rc = prvCpssPxIngressPortMapPacketTypeDestinationFormatEntryGet(devNum, packetType, packetTypeFormatPtr);
            if (rc != GT_OK)
            {
                return rc;
            }
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_ONE_PARAM_FORMAT_MAC(tableType));
    }

    return GT_OK;
}

/**
* @internal cpssPxIngressPortMapPacketTypeFormatEntryGet function
* @endinternal
*
* @brief   Get Source/Destination information for packet type in the format table entry.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] tableType                - Source/Destination port map table.
* @param[in] packetType               - index of the packet type format
*                                      in the Source/Destination format table.
*                                      (APPLICABLE RANGES: 0..31)
*
* @param[out] packetTypeFormatPtr      - (pointer to) Source/Destination packet type format
*                                      in the Source/Destination format table.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxIngressPortMapPacketTypeFormatEntryGet
(
    IN  GT_SW_DEV_NUM                                       devNum,
    IN  CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_ENT             tableType,
    IN  CPSS_PX_PACKET_TYPE                                 packetType,
    OUT CPSS_PX_INGRESS_PORT_MAP_PACKET_TYPE_FORMAT_STC     *packetTypeFormatPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxIngressPortMapPacketTypeFormatEntryGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, tableType, packetType, packetTypeFormatPtr));

    rc = internal_cpssPxIngressPortMapPacketTypeFormatEntryGet(devNum, tableType, packetType, packetTypeFormatPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, tableType, packetType, packetTypeFormatPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxIngressPortMapEntrySet function
* @endinternal
*
* @brief   Set Source/Destination port map table entry.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] tableType                - Source/Destination port map table.
* @param[in] entryIndex               - port map entry index.
*                                      Index calculated by packet type source/destination format.
*                                      Destination table -
*                                      (APPLICABLE RANGES: 0..8191)
*                                      Source table -
*                                      (APPLICABLE RANGES: 0..4095)
* @param[in] portsBmp                 - the bitmap of ports.
* @param[in] bypassLagDesignatedBitmap
*                                      - enables bypass of LAG Designated Port selection for this entry.
*                                      Relevant only for destination port map table.
*                                      GT_TRUE     -  The LAG Designated Port selection is bypassed for this entry
*                                      GT_FALSE    -  The LAG Designated Port selection is applied for this entry
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Source, Destination port map tables, LAG designated table and Port Enable
*       map table are used in calculation of egress ports
*       to which the unicast or multicast packet is forwarded.
*
*/
static GT_STATUS internal_cpssPxIngressPortMapEntrySet
(
    IN  GT_SW_DEV_NUM                               devNum,
    IN  CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_ENT     tableType,
    IN  GT_U32                                      entryIndex,
    IN  CPSS_PX_PORTS_BMP                           portsBmp,
    IN  GT_BOOL                                     bypassLagDesignatedBitmap
)
{
    GT_U32  regValue = 0;

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_BMP_CHECK_MAC(devNum, portsBmp);

    /* SRC/DST Port Map Table Entry */
    U32_SET_FIELD_MAC(regValue,  0,  17, portsBmp);
    switch (tableType)
    {
        case CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_PORT_FILTERING_E:
            return prvCpssPxWriteTableEntry(devNum,
                                            CPSS_PX_TABLE_PCP_PORT_FILTERING_TABLE_E,
                                            entryIndex, &regValue);
        case CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_DESTINATION_E:
            if (PRV_CPSS_PX_A1_AND_ABOVE_CHECK_MAC(devNum))
            {
                /* Bypass LAG Designated Portmap */
                U32_SET_FIELD_MAC(regValue,  17,  1, BOOL2BIT_MAC(bypassLagDesignatedBitmap));
            }
            return prvCpssPxWriteTableEntry(devNum,
                                            CPSS_PX_TABLE_PCP_DST_PORT_MAP_TABLE_E,
                                            entryIndex, &regValue);
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_ONE_PARAM_FORMAT_MAC(tableType));
    }

    /*return GT_OK; warning C4702: unreachable code*/
}

/**
* @internal cpssPxIngressPortMapEntrySet function
* @endinternal
*
* @brief   Set Source/Destination port map table entry.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] tableType                - Source/Destination port map table.
* @param[in] entryIndex               - port map entry index.
*                                      Index calculated by packet type source/destination format.
*                                      Destination table -
*                                      (APPLICABLE RANGES: 0..8191)
*                                      Source table -
*                                      (APPLICABLE RANGES: 0..4095)
* @param[in] portsBmp                 - the bitmap of ports.
* @param[in] bypassLagDesignatedBitmap
*                                      - enables bypass of LAG Designated Port selection for this entry.
*                                      Relevant only for destination port map table.
*                                      GT_TRUE     -  The LAG Designated Port selection is bypassed for this entry
*                                      GT_FALSE    -  The LAG Designated Port selection is applied for this entry
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxIngressPortMapEntrySet
(
    IN  GT_SW_DEV_NUM                               devNum,
    IN  CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_ENT     tableType,
    IN  GT_U32                                      entryIndex,
    IN  CPSS_PX_PORTS_BMP                           portsBmp,
    IN  GT_BOOL                                     bypassLagDesignatedBitmap
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxIngressPortMapEntrySet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, tableType, entryIndex, portsBmp, bypassLagDesignatedBitmap));

    rc = internal_cpssPxIngressPortMapEntrySet(devNum, tableType, entryIndex, portsBmp, bypassLagDesignatedBitmap);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, tableType, entryIndex, portsBmp, bypassLagDesignatedBitmap));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxIngressPortMapEntryGet function
* @endinternal
*
* @brief   Gets Source/Destination port map table entry.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] tableType                - Source/Destination port map table.
* @param[in] entryIndex               - port map entry index.
*                                      Index calculated by packet type source/destination format.
*                                      Destination table -
*                                      (APPLICABLE RANGES: 0..8191)
*                                      Source table -
*                                      (APPLICABLE RANGES: 0..4095)
*
* @param[out] portsBmpPtr              - (pointer to) the bitmap of ports.
* @param[out] bypassLagDesignatedBitmapPtr
*                                      - (pointer to) the bypass status of LAG Designated Port selection for this entry.
*                                      Relevant only for destination port map table.
*                                      GT_TRUE     -  The LAG Designated Port selection is bypassed for this entry
*                                      GT_FALSE    -  The LAG Designated Port selection is applied for this entry
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Source, Destination port map tables, LAG designated table and Port Enable
*       map table are used in calculation of egress ports
*       to which the unicast or multicast packet is forwarded.
*
*/
static GT_STATUS internal_cpssPxIngressPortMapEntryGet
(
    IN  GT_SW_DEV_NUM                               devNum,
    IN  CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_ENT     tableType,
    IN  GT_U32                                      entryIndex,
    OUT CPSS_PX_PORTS_BMP                           *portsBmpPtr,
    OUT GT_BOOL                                     *bypassLagDesignatedBitmapPtr
)
{
    GT_STATUS rc;
    GT_U32 regVal = 0;

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(portsBmpPtr);
    CPSS_NULL_PTR_CHECK_MAC(bypassLagDesignatedBitmapPtr);

    switch (tableType)
    {
        case CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_PORT_FILTERING_E:
            rc = prvCpssPxReadTableEntry(devNum, CPSS_PX_TABLE_PCP_PORT_FILTERING_TABLE_E,
                                    entryIndex, &regVal);
            if (rc != GT_OK)
            {
                return rc;
            }
            break;
        case CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_DESTINATION_E:
            rc = prvCpssPxReadTableEntry(devNum, CPSS_PX_TABLE_PCP_DST_PORT_MAP_TABLE_E,
                                         entryIndex, &regVal);
            if (rc != GT_OK)
            {
                return rc;
            }

            if (PRV_CPSS_PX_A1_AND_ABOVE_CHECK_MAC(devNum))
            {
                /* Bypass LAG Designated Portmap */
                *bypassLagDesignatedBitmapPtr =
                    BIT2BOOL_MAC(U32_GET_FIELD_MAC(regVal,  17,  1));
            }
            else
            {
                *bypassLagDesignatedBitmapPtr = 0;
            }
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_ONE_PARAM_FORMAT_MAC(tableType));
    }

    /* SRC/DST Port Map Table Entry */
    *portsBmpPtr = U32_GET_FIELD_MAC(regVal,  0,  17);

    return GT_OK;
}

/**
* @internal cpssPxIngressPortMapEntryGet function
* @endinternal
*
* @brief   Gets Source/Destination port map table entry.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] tableType                - Source/Destination port map table.
* @param[in] entryIndex               - port map entry index.
*                                      Index calculated by packet type source/destination format.
*                                      Destination table -
*                                      (APPLICABLE RANGES: 0..8191)
*                                      Source table -
*                                      (APPLICABLE RANGES: 0..4095)
*
* @param[out] portsBmpPtr              - (pointer to) the bitmap of ports.
* @param[out] bypassLagDesignatedBitmapPtr
*                                      - (pointer to) the bypass status of LAG Designated Port selection for this entry.
*                                      Relevant only for destination port map table.
*                                      GT_TRUE     -  The LAG Designated Port selection is bypassed for this entry
*                                      GT_FALSE    -  The LAG Designated Port selection is applied for this entry
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxIngressPortMapEntryGet
(
    IN  GT_SW_DEV_NUM                               devNum,
    IN  CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_ENT     tableType,
    IN  GT_U32                                      entryIndex,
    OUT CPSS_PX_PORTS_BMP                           *portsBmpPtr,
    OUT GT_BOOL                                     *bypassLagDesignatedBitmapPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxIngressPortMapEntryGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, tableType, entryIndex, portsBmpPtr, bypassLagDesignatedBitmapPtr));

    rc = internal_cpssPxIngressPortMapEntryGet(devNum, tableType, entryIndex, portsBmpPtr, bypassLagDesignatedBitmapPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, tableType, entryIndex, portsBmpPtr, bypassLagDesignatedBitmapPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxIngressPortTargetEnableSet function
* @endinternal
*
* @brief   Enable/disable forwarding to the target port.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - physical port number.
* @param[in] enable                   -  or disable forwarding to the target port.
*                                      GT_TRUE - Enables packet forwarding to the target port,
*                                      GT_FALSE - Filters all traffic to the target port.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxIngressPortTargetEnableSet
(
    IN  GT_SW_DEV_NUM                   devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  GT_BOOL                         enable
)
{
    GT_U32 regAddr;             /* register address */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PHY_PORT_NUM_CHECK_MAC(devNum, portNum);

    /* PCP Ports Enable Config */
    regAddr = PRV_PX_REG1_UNIT_PCP_MAC(devNum).PORTMAP.PCPPortsEnableConfig;
    /* PCP Enable Port Config <portNum> */
    return prvCpssHwPpSetRegField(CAST_SW_DEVNUM(devNum), regAddr, portNum, 1, BOOL2BIT_MAC(enable));
}

/**
* @internal cpssPxIngressPortTargetEnableSet function
* @endinternal
*
* @brief   Enable/disable forwarding to the target port.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - physical port number.
* @param[in] enable                   -  or disable forwarding to the target port.
*                                      GT_TRUE - Enables packet forwarding to the target port,
*                                      GT_FALSE - Filters all traffic to the target port.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxIngressPortTargetEnableSet
(
    IN  GT_SW_DEV_NUM                   devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  GT_BOOL                         enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxIngressPortTargetEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, enable));

    rc = internal_cpssPxIngressPortTargetEnableSet(devNum, portNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxIngressPortTargetEnableGet function
* @endinternal
*
* @brief   Get state of the target port.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - physical port number.
*
* @param[out] enablePtr                - (pointer to) state of the target port.
*                                      GT_TRUE - Enables packet forwarding to the target port,
*                                      GT_FALSE - Filters all traffic to the target port.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxIngressPortTargetEnableGet
(
    IN  GT_SW_DEV_NUM                   devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    OUT GT_BOOL                         *enablePtr
)
{
    GT_STATUS rc;
    GT_U32  regAddr;
    GT_U32  fieldValue;

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PHY_PORT_NUM_CHECK_MAC(devNum, portNum);
    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    /* PCP Ports Enable Config */
    regAddr = PRV_PX_REG1_UNIT_PCP_MAC(devNum).PORTMAP.PCPPortsEnableConfig;
    /* PCP Enable Port Config <portNum> */
    rc = prvCpssHwPpGetRegField(CAST_SW_DEVNUM(devNum), regAddr, portNum, 1, &fieldValue);
    if (rc != GT_OK)
    {
        return rc;
    }
    *enablePtr = BIT2BOOL_MAC(fieldValue);

    return GT_OK;
}

/**
* @internal cpssPxIngressPortTargetEnableGet function
* @endinternal
*
* @brief   Get state of the target port.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - physical port number.
*
* @param[out] enablePtr                - (pointer to) state of the target port.
*                                      GT_TRUE - Enables packet forwarding to the target port,
*                                      GT_FALSE - Filters all traffic to the target port.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxIngressPortTargetEnableGet
(
    IN  GT_SW_DEV_NUM                   devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    OUT GT_BOOL                         *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxIngressPortTargetEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enablePtr));

    rc = internal_cpssPxIngressPortTargetEnableGet(devNum, portNum, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/* Get miss Packet type MAC */
static GT_STATUS prvCpssPxIngressMissPacketTypeMacGet
(
    IN GT_SW_DEV_NUM                                devNum,
    IN CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC   *errorKeyPtr
)
{
    GT_STATUS rc;
    GT_U32  regAddr;
    GT_U32  regValue;

    /* 2 MAC DA LSB   */
    regAddr = PRV_PX_REG1_UNIT_PCP_MAC(devNum).PRS.PCPPktTypeKeyMACDA2LSBMissValue;
    rc = prvCpssHwPpReadRegister(CAST_SW_DEVNUM(devNum), regAddr, &regValue);
    if (rc != GT_OK)
    {
        return rc;
    }
    PRV_CPSS_PX_MAC_DA_TWO_LSB_BYTES_REG_GET_MAC(errorKeyPtr->macDa, regValue);

    /* 4 MAC DA MSB   */
    regAddr = PRV_PX_REG1_UNIT_PCP_MAC(devNum).PRS.PCPPktTypeKeyMACDA4MSBMissValue;
    rc = prvCpssHwPpReadRegister(CAST_SW_DEVNUM(devNum), regAddr, &regValue);
    if (rc != GT_OK)
    {
        return rc;
    }
    PRV_CPSS_PX_MAC_DA_FOUR_MSB_BYTES_GET_MAC(errorKeyPtr->macDa, regValue);

    return GT_OK;
}

/* Get miss Packet type UDBP data and mask */
static GT_STATUS prvCpssPxIngressMissPacketTypeUdbpGet
(
    IN GT_SW_DEV_NUM                                devNum,
    IN  CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC  *errorKeyPtr
)
{
    GT_STATUS rc;
    GT_U32  regAddr;
    GT_U32  regValue;
    GT_U32  i;

    /* UDBP0..3 miss value */
    for (i = 0; i < 4; i++)
    {
        regAddr = PRV_PX_REG1_UNIT_PCP_MAC(devNum).PRS.PCPPktTypeKeyUDBPMissValue[i];
        /* Read data register */
        rc = prvCpssHwPpReadRegister(CAST_SW_DEVNUM(devNum), regAddr, &regValue);
        if (rc != GT_OK)
        {
            return rc;
        }

        /* UDBP Data */
        PRV_CPSS_PX_UDBP_REG_FIELD_GET_MAC(errorKeyPtr->udbPairsArr[i], regValue);
    }

    return GT_OK;
}

/* Get miss Packet type TPID EtherType data and mask */
static GT_STATUS prvCpssPxIngressMissPacketTypeTpidEtherTypeGet
(
    IN GT_SW_DEV_NUM                                devNum,
    IN  CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC  *errorKeyPtr

)
{
    GT_STATUS rc;
    GT_U32  regAddr;
    GT_U32  regValue;

    /* TPID EtherType miss value */
    regAddr = PRV_PX_REG1_UNIT_PCP_MAC(devNum).PRS.PCPPktTypeKeyTPIDEtherTypeMissValue;

    /* Read data register */
    rc = prvCpssHwPpReadRegister(CAST_SW_DEVNUM(devNum), regAddr, &regValue);
    if (rc != GT_OK)
    {
        return rc;
    }

    errorKeyPtr->etherType = U32_GET_FIELD_MAC(regValue, 0, 16);

    return GT_OK;
}

/* Get miss Packet type Source port profile data and mask */
static GT_STATUS prvCpssPxIngressMissPacketTypeSourcePortProfileGet
(
    IN GT_SW_DEV_NUM                                devNum,
    IN  CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC  *errorKeyPtr

)
{
    GT_STATUS rc;
    GT_U32  regAddr;
    GT_U32  regValue;

    /* Source port profile miss value */
    regAddr = PRV_PX_REG1_UNIT_PCP_MAC(devNum).PRS.PCPPktTypeKeySRCPortProfileMissValue;

    /* Read data register */
    rc = prvCpssHwPpReadRegister(CAST_SW_DEVNUM(devNum), regAddr, &regValue);
    if (rc != GT_OK)
    {
        return rc;
    }

    errorKeyPtr->profileIndex = U32_GET_FIELD_MAC(regValue, 0, 7);

    return GT_OK;
}

/* Get miss Packet type LLC-Non-Snap data and mask */
static GT_STATUS prvCpssPxIngressMissPacketTypeLlcNonSnapGet
(
    IN GT_SW_DEV_NUM                                devNum,
    IN  CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC  *errorKeyPtr

)
{
    GT_STATUS rc;
    GT_U32  regAddr;
    GT_U32  regValue;

    /* LLS-non-SNAP miss value */
    regAddr = PRV_PX_REG1_UNIT_PCP_MAC(devNum).PRS.PCPPktTypeKeyIsLLCNonSnapMissValue;

    /* Read data register */
    rc = prvCpssHwPpReadRegister(CAST_SW_DEVNUM(devNum), regAddr, &regValue);
    if (rc != GT_OK)
    {
        return rc;
    }

    errorKeyPtr->isLLCNonSnap = U32_GET_FIELD_MAC(regValue, 0, 1);

    return GT_OK;
}

/**
* @internal internal_cpssPxIngressPacketTypeErrorGet function
* @endinternal
*
* @brief   Get unmatched packet type error counter and key.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
*
* @param[out] errorCounterPtr          - (pointer to) unmatched packet type counter.
* @param[out] errorKeyPtr              - (pointer to) unmatched packet type key.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxIngressPacketTypeErrorGet
(
    IN  GT_SW_DEV_NUM                               devNum,
    OUT GT_U32                                      *errorCounterPtr,
    OUT CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC  *errorKeyPtr
)
{
    GT_STATUS rc;
    GT_U32  regAddr;
    GT_U32  regValue;

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(errorCounterPtr);
    CPSS_NULL_PTR_CHECK_MAC(errorKeyPtr);

    /* PCP Packet Type Key Look Up Miss Detected */
    regAddr = PRV_PX_REG1_UNIT_PCP_MAC(devNum).PRS.PCPPktTypeKeyLookUpMissDetected;
    rc  = prvCpssHwPpReadRegister(CAST_SW_DEVNUM(devNum), regAddr, &regValue);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* Miss_detected */
    if ((regValue & 1) == 0)
    {
        *errorCounterPtr = 0;
        return GT_OK;
    }

    /* PCP Packet Type Key Look Up Miss Counter */
    regAddr = PRV_PX_REG1_UNIT_PCP_MAC(devNum).PRS.PCPPktTypeKeyLookUpMissCntr;
    rc  = prvCpssHwPpReadRegister(CAST_SW_DEVNUM(devNum), regAddr, errorCounterPtr);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* MAC DA */
    rc = prvCpssPxIngressMissPacketTypeMacGet(devNum, errorKeyPtr);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* UDBP */
    rc = prvCpssPxIngressMissPacketTypeUdbpGet(devNum, errorKeyPtr);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* TPID EtherType */
    rc = prvCpssPxIngressMissPacketTypeTpidEtherTypeGet(devNum, errorKeyPtr);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* Source port profile */
    rc = prvCpssPxIngressMissPacketTypeSourcePortProfileGet(devNum, errorKeyPtr);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* LLC-non-SNAP */
    return prvCpssPxIngressMissPacketTypeLlcNonSnapGet(devNum, errorKeyPtr);
}

/**
* @internal cpssPxIngressPacketTypeErrorGet function
* @endinternal
*
* @brief   Get unmatched packet type error counter and key.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
*
* @param[out] errorCounterPtr          - (pointer to) unmatched packet type counter.
* @param[out] errorKeyPtr              - (pointer to) unmatched packet type key.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxIngressPacketTypeErrorGet
(
    IN  GT_SW_DEV_NUM                               devNum,
    OUT GT_U32                                      *errorCounterPtr,
    OUT CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC  *errorKeyPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxIngressPacketTypeErrorGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, errorCounterPtr, errorKeyPtr));

    rc = internal_cpssPxIngressPacketTypeErrorGet(devNum, errorCounterPtr, errorKeyPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, errorCounterPtr, errorKeyPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxIngressForwardingPortMapExceptionSet function
* @endinternal
*
* @brief   Set exception Source/Destination port map.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] tableType                - Source/Destination port map table.
* @param[in] portsBmp                 - the bitmap of ports.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxIngressForwardingPortMapExceptionSet
(
    IN  GT_SW_DEV_NUM                               devNum,
    IN  CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_ENT     tableType,
    IN  CPSS_PX_PORTS_BMP                           portsBmp
)
{
    GT_U32 regAddr;             /* register address */
    GT_U32 regWriteBits;             /* number of bits to be written to register */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_BMP_CHECK_MAC(devNum, portsBmp);

    switch (tableType)
    {
        case CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_PORT_FILTERING_E:
            regAddr = PRV_PX_REG1_UNIT_PCP_MAC(devNum).PORTMAP.PCPSRCExceptionForwardingPortMap;
            break;
        case CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_DESTINATION_E:
            regAddr = PRV_PX_REG1_UNIT_PCP_MAC(devNum).PORTMAP.PCPDSTExceptionForwardingPortMap;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_ONE_PARAM_FORMAT_MAC(tableType));
    }
    /* Write bits */
    regWriteBits = 17;

    return prvCpssHwPpSetRegField(CAST_SW_DEVNUM(devNum), regAddr, 0, regWriteBits, portsBmp);
}

/**
* @internal cpssPxIngressForwardingPortMapExceptionSet function
* @endinternal
*
* @brief   Set exception Source/Destination port map.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] tableType                - Source/Destination port map table.
* @param[in] portsBmp                 - the bitmap of ports.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxIngressForwardingPortMapExceptionSet
(
    IN  GT_SW_DEV_NUM                               devNum,
    IN  CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_ENT     tableType,
    IN  CPSS_PX_PORTS_BMP                           portsBmp
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxIngressForwardingPortMapExceptionSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, tableType, portsBmp));

    rc = internal_cpssPxIngressForwardingPortMapExceptionSet(devNum, tableType, portsBmp);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, tableType, portsBmp));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxIngressForwardingPortMapExceptionGet function
* @endinternal
*
* @brief   Get exception Source/Destination port map.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] tableType                - Source/Destination port map table.
*
* @param[out] portsBmpPtr              - (pointer to) the bitmap of ports.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxIngressForwardingPortMapExceptionGet
(
    IN  GT_SW_DEV_NUM                               devNum,
    IN  CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_ENT     tableType,
    OUT CPSS_PX_PORTS_BMP                           *portsBmpPtr
)
{
    GT_U32  regAddr;

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(portsBmpPtr);

    switch (tableType)
    {
        case CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_PORT_FILTERING_E:
            regAddr = PRV_PX_REG1_UNIT_PCP_MAC(devNum).PORTMAP.PCPSRCExceptionForwardingPortMap;
            break;
        case CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_DESTINATION_E:
            regAddr = PRV_PX_REG1_UNIT_PCP_MAC(devNum).PORTMAP.PCPDSTExceptionForwardingPortMap;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_ONE_PARAM_FORMAT_MAC(tableType));
    }

    return prvCpssHwPpReadRegister(CAST_SW_DEVNUM(devNum), regAddr, portsBmpPtr);
}

/**
* @internal cpssPxIngressForwardingPortMapExceptionGet function
* @endinternal
*
* @brief   Get exception Source/Destination port map.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] tableType                - Source/Destination port map table.
*
* @param[out] portsBmpPtr              - (pointer to) the bitmap of ports.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxIngressForwardingPortMapExceptionGet
(
    IN  GT_SW_DEV_NUM                               devNum,
    IN  CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_ENT     tableType,
    OUT CPSS_PX_PORTS_BMP                           *portsBmpPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxIngressForwardingPortMapExceptionGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, tableType, portsBmpPtr));

    rc = internal_cpssPxIngressForwardingPortMapExceptionGet(devNum, tableType, portsBmpPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, tableType, portsBmpPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxIngressPortMapExceptionCounterGet function
* @endinternal
*
* @brief   Get Source/Destination index exception counter.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] tableType                - Source/Destination port map table.
*
* @param[out] errorCounterPtr          - (pointer to) Source/Destination index exception counter.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxIngressPortMapExceptionCounterGet
(
    IN  GT_SW_DEV_NUM                               devNum,
    IN  CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_ENT     tableType,
    OUT GT_U32                                      *errorCounterPtr
)
{
    GT_U32  regAddr;

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(errorCounterPtr);

    switch (tableType)
    {
        case CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_PORT_FILTERING_E:
            regAddr = PRV_PX_REG1_UNIT_PCP_MAC(devNum).PORTMAP.PCPSrcIdxExceptionCntr;
            break;
        case CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_DESTINATION_E:
            regAddr = PRV_PX_REG1_UNIT_PCP_MAC(devNum).PORTMAP.PCPDstIdxExceptionCntr;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_ONE_PARAM_FORMAT_MAC(tableType));
    }

    return prvCpssHwPpReadRegister(CAST_SW_DEVNUM(devNum), regAddr, errorCounterPtr);
}

/**
* @internal cpssPxIngressPortMapExceptionCounterGet function
* @endinternal
*
* @brief   Get Source/Destination index exception counter.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] tableType                - Source/Destination port map table.
*
* @param[out] errorCounterPtr          - (pointer to) Source/Destination index exception counter.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxIngressPortMapExceptionCounterGet
(
    IN  GT_SW_DEV_NUM                               devNum,
    IN  CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_ENT     tableType,
    OUT GT_U32                                      *errorCounterPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxIngressPortMapExceptionCounterGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, tableType, errorCounterPtr));

    rc = internal_cpssPxIngressPortMapExceptionCounterGet(devNum, tableType, errorCounterPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, tableType, errorCounterPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxIngressPacketTypeKeyEntryEnableSet function
* @endinternal
*
* @brief   Enable/disable packet type key table entry.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] packetType               - index of the packet type key in the table.
*                                      (APPLICABLE RANGES: 0..31)
* @param[in] enable                   - Enable or disable packet type key table entry
*                                      GT_TRUE - enable packet type key table entry,
*                                      GT_FALSE - disable packet type key table entry.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxIngressPacketTypeKeyEntryEnableSet
(
    IN  GT_SW_DEV_NUM                               devNum,
    IN  CPSS_PX_PACKET_TYPE                         packetType,
    IN  GT_BOOL                                     enable
)
{
    GT_U32 regAddr;         /* register address */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    if (packetType >= CPSS_PX_INGRESS_PACKET_TYPE_KEY_TABLE_ENTRIES_MAX_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_ONE_PARAM_FORMAT_MAC(packetType));
    }

    /*         PCP Packet Type Key Table Entries Enable */
    regAddr = PRV_PX_REG1_UNIT_PCP_MAC(devNum).PRS.PCPPktTypeKeyTableEntriesEnable;
    /* Packet Type Key Enable <packetType> */
    return prvCpssHwPpSetRegField(CAST_SW_DEVNUM(devNum), regAddr, packetType, 1, BOOL2BIT_MAC(enable));
}

/**
* @internal cpssPxIngressPacketTypeKeyEntryEnableSet function
* @endinternal
*
* @brief   Enable/disable packet type key table entry.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] packetType               - index of the packet type key in the table.
*                                      (APPLICABLE RANGES: 0..31)
* @param[in] enable                   - Enable or disable packet type key table entry
*                                      GT_TRUE - enable packet type key table entry,
*                                      GT_FALSE - disable packet type key table entry.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxIngressPacketTypeKeyEntryEnableSet
(
    IN  GT_SW_DEV_NUM                               devNum,
    IN  CPSS_PX_PACKET_TYPE                         packetType,
    IN  GT_BOOL                                     enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxIngressPacketTypeKeyEntryEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, packetType, enable));

    rc = internal_cpssPxIngressPacketTypeKeyEntryEnableSet(devNum, packetType, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, packetType, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxIngressPacketTypeKeyEntryEnableGet function
* @endinternal
*
* @brief   Get status of the packet type key table entry.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] packetType               - index of the packet type key in the table.
*                                      (APPLICABLE RANGES: 0..31)
*
* @param[out] enablePtr                - (pointer to) state of the packet type key table entry
*                                      GT_TRUE - enable packet type key table entry,
*                                      GT_FALSE - disable packet type key table entry.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxIngressPacketTypeKeyEntryEnableGet
(
    IN  GT_SW_DEV_NUM                               devNum,
    IN  CPSS_PX_PACKET_TYPE                         packetType,
    OUT GT_BOOL                                     *enablePtr
)
{
    GT_U32 regAddr;         /* register address */
    GT_STATUS rc;           /* return status */
    GT_U32 regValue;        /* register value */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    if (packetType >= CPSS_PX_INGRESS_PACKET_TYPE_KEY_TABLE_ENTRIES_MAX_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_ONE_PARAM_FORMAT_MAC(packetType));
    }

    /*         PCP Packet Type Key Table Entries Enable */
    regAddr = PRV_PX_REG1_UNIT_PCP_MAC(devNum).PRS.PCPPktTypeKeyTableEntriesEnable;
    /* Packet Type Key Enable <packetType> */
    rc = prvCpssHwPpGetRegField(CAST_SW_DEVNUM(devNum), regAddr, packetType, 1, &regValue);
    if (rc != GT_OK)
    {
        return rc;
    }

    *enablePtr = BIT2BOOL_MAC(regValue);

    return GT_OK;
}

/**
* @internal cpssPxIngressPacketTypeKeyEntryEnableGet function
* @endinternal
*
* @brief   Get status of the packet type key table entry.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] packetType               - index of the packet type key in the table.
*                                      (APPLICABLE RANGES: 0..31)
*
* @param[out] enablePtr                - (pointer to) state of the packet type key table entry
*                                      GT_TRUE - enable packet type key table entry,
*                                      GT_FALSE - disable packet type key table entry.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxIngressPacketTypeKeyEntryEnableGet
(
    IN  GT_SW_DEV_NUM                               devNum,
    IN  CPSS_PX_PACKET_TYPE                         packetType,
    OUT GT_BOOL                                     *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxIngressPacketTypeKeyEntryEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, packetType, enablePtr));

    rc = internal_cpssPxIngressPacketTypeKeyEntryEnableGet(devNum, packetType, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, packetType, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxIngressPortDsaTagEnableSet function
* @endinternal
*
* @brief   Enable/disable the source port for DSA-tagged packets.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - physical port number.
* @param[in] enable                   -  or disable the source port for DSA-tagged packets
*                                      GT_TRUE   - enable the port for DSA-tagged packets,
*                                      GT_FALSE  - disable the port for DSA-tagged packets
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxIngressPortDsaTagEnableSet
(
    IN  GT_SW_DEV_NUM                   devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  GT_BOOL                         enable
)
{
    GT_U32 regAddr;             /* register address */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PHY_PORT_NUM_CHECK_MAC(devNum, portNum);

    /* SCDMA <portNum> Configuration 0 */
    regAddr = PRV_PX_REG1_UNIT_RXDMA_MAC(devNum).singleChannelDMAConfigs.SCDMAConfig0[portNum];
    /* Cascade Enable <portNum>  */
    return prvCpssHwPpSetRegField(CAST_SW_DEVNUM(devNum), regAddr, 3, 1, BOOL2BIT_MAC(enable));
}

/**
* @internal cpssPxIngressPortDsaTagEnableSet function
* @endinternal
*
* @brief   Enable/disable the source port for DSA-tagged packets.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - physical port number.
* @param[in] enable                   -  or disable the source port for DSA-tagged packets
*                                      GT_TRUE   - enable the port for DSA-tagged packets,
*                                      GT_FALSE  - disable the port for DSA-tagged packets
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxIngressPortDsaTagEnableSet
(
    IN  GT_SW_DEV_NUM                   devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  GT_BOOL                         enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxIngressPortDsaTagEnableSet);

    CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enable));

    rc = internal_cpssPxIngressPortDsaTagEnableSet(devNum, portNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enable));
    CPSS_API_UNLOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxIngressPortDsaTagEnableGet function
* @endinternal
*
* @brief   Get DSA-tagged packets receiving status on the source port.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - physical port number.
*
* @param[out] enablePtr                - (pointer to) DSA-tagged packets receiving status on the source port
*                                      GT_TRUE   - enable the port for DSA-tagged packets,
*                                      GT_FALSE  - disable the port for DSA-tagged packets
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxIngressPortDsaTagEnableGet
(
    IN  GT_SW_DEV_NUM                   devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    OUT GT_BOOL                         *enablePtr
)
{
    GT_U32 regAddr;         /* register address */
    GT_STATUS rc;           /* return status */
    GT_U32 regValue;        /* register value */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PHY_PORT_NUM_CHECK_MAC(devNum, portNum);
    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    /* SCDMA <portNum> Configuration 0 */
    regAddr = PRV_PX_REG1_UNIT_RXDMA_MAC(devNum).singleChannelDMAConfigs.SCDMAConfig0[portNum];
    /* Cascade Enable <portNum>  */
    rc = prvCpssHwPpGetRegField(CAST_SW_DEVNUM(devNum), regAddr, 3, 1, &regValue);
    if (rc != GT_OK)
    {
        return rc;
    }

    *enablePtr = BIT2BOOL_MAC(regValue);

    return GT_OK;
}

/**
* @internal cpssPxIngressPortDsaTagEnableGet function
* @endinternal
*
* @brief   Get DSA-tagged packets receiving status on the source port.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - physical port number.
*
* @param[out] enablePtr                - (pointer to) DSA-tagged packets receiving status on the source port
*                                      GT_TRUE   - enable the port for DSA-tagged packets,
*                                      GT_FALSE  - disable the port for DSA-tagged packets
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxIngressPortDsaTagEnableGet
(
    IN  GT_SW_DEV_NUM                   devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    OUT GT_BOOL                         *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxIngressPortDsaTagEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enablePtr));

    rc = internal_cpssPxIngressPortDsaTagEnableGet(devNum, portNum, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxIngressPortRedirectSet function
* @endinternal
*
* @brief   Set redirecting of the all packets from ingress port to list of
*         egress ports.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - ingress physical port number.
* @param[in] targetPortsBmp           - the bitmap of egress ports.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxIngressPortRedirectSet
(
    IN  GT_SW_DEV_NUM                   devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  CPSS_PX_PORTS_BMP               targetPortsBmp
)
{
    GT_STATUS               rc;                     /* return status */
    GT_U32                  srcProfileIdx;
    GT_U32                  pktType;
    GT_U32                  portMapEntryIdx;
    GT_PHYSICAL_PORT_NUM    tmpPortNum;
    GT_BOOL                 forwardingEn;
    CPSS_PX_INGRESS_PORT_PACKET_TYPE_KEY_STC        portKey;
    CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC      keyData;
    CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC      keyMask;
    CPSS_PX_INGRESS_PORT_MAP_PACKET_TYPE_FORMAT_STC packetTypeFormat;
    CPSS_PX_PORTS_BMP       sourcePortsBmp;

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PHY_PORT_NUM_CHECK_MAC(devNum, portNum);
    PRV_CPSS_PX_PORT_BMP_CHECK_MAC(devNum, targetPortsBmp);


    srcProfileIdx = portNum;

    /* 1. Set Port Packet Type Key Table for portNum */
    cpssOsMemSet(&portKey, 0, sizeof(CPSS_PX_INGRESS_PORT_PACKET_TYPE_KEY_STC));
    portKey.srcPortProfile = srcProfileIdx;
    rc = cpssPxIngressPortPacketTypeKeySet(devNum, portNum, &portKey);
    if (rc != GT_OK)
        return rc;

    pktType = portNum;

    /* 2. Set Packet Type #pktType for upstream traffic */
    cpssOsMemSet(&keyData, 0, sizeof(CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC));
    cpssOsMemSet(&keyMask, 0, sizeof(CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC));

    /* set that each 'packet type' will match the src port profile */
    keyMask.profileIndex = 0x7F;
    keyData.profileIndex = srcProfileIdx;

    rc = cpssPxIngressPacketTypeKeyEntrySet(devNum, pktType, &keyData, &keyMask);
    if (rc != GT_OK)
        return rc;


    portMapEntryIdx = 0xFF + (portNum);
    /* 3. Set Packet Type #pktType destination format entry for upstream traffic */
    cpssOsMemSet(&packetTypeFormat, 0,
                  sizeof(CPSS_PX_INGRESS_PORT_MAP_PACKET_TYPE_FORMAT_STC));

    packetTypeFormat.indexMax = BIT_12-1;
    packetTypeFormat.indexConst = portMapEntryIdx; /* destination index */
    rc = cpssPxIngressPortMapPacketTypeFormatEntrySet(devNum,
                            CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_DESTINATION_E,
                            pktType, &packetTypeFormat);
    if (rc != GT_OK)
        return rc;

    /* 4. Set Destination port map table entry for target bitmaps */
    rc = cpssPxIngressPortMapEntrySet(devNum,
                            CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_DESTINATION_E,
                            portMapEntryIdx, targetPortsBmp, GT_FALSE);


    /* 5. Set Packet Type #pktType source format entry to prevent source port filtering */
    portMapEntryIdx = 0xFF + (portNum * 0x10);
    packetTypeFormat.indexMax = BIT_11-1;/*11 bits*/
    packetTypeFormat.indexConst = portMapEntryIdx; /* source index */

    rc = cpssPxIngressPortMapPacketTypeFormatEntrySet(devNum,
                            CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_PORT_FILTERING_E,
                            pktType, &packetTypeFormat);
    if (rc != GT_OK)
        return rc;

    /* 6. Set Source port map table entry for source and destination ports */
    sourcePortsBmp = targetPortsBmp | (1 << portNum);
    rc = cpssPxIngressPortMapEntrySet(devNum,
                            CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_PORT_FILTERING_E,
                            portMapEntryIdx, sourcePortsBmp, GT_FALSE);

    /* 7. Enable Packet Type #pktType  */
    rc = cpssPxIngressPacketTypeKeyEntryEnableSet(devNum, pktType, GT_TRUE);
    if (rc != GT_OK)
        return rc;

    /* 8. Enable forwarding for target bitmaps
           (except CPU --> CPU port is already enabled)  */
    for(tmpPortNum = 0; tmpPortNum < PRV_CPSS_PP_MAC(devNum)->numOfPorts; tmpPortNum++)
    {
        forwardingEn = ((1<<tmpPortNum) & targetPortsBmp) ? GT_TRUE : GT_FALSE;

        if (forwardingEn == GT_FALSE)
            continue;

        rc = cpssPxIngressPortTargetEnableSet(devNum, tmpPortNum, forwardingEn);
        if (rc != GT_OK)
            return rc;
    }

    return rc;
}

/**
* @internal cpssPxIngressPortRedirectSet function
* @endinternal
*
* @brief   Set redirecting of the all packets from ingress port to list of
*         egress ports.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - ingress physical port number.
* @param[in] targetPortsBmp           - the bitmap of egress ports.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxIngressPortRedirectSet
(
    IN  GT_SW_DEV_NUM                   devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  CPSS_PX_PORTS_BMP               targetPortsBmp
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxIngressPortRedirectSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, targetPortsBmp));

    rc = internal_cpssPxIngressPortRedirectSet(devNum, portNum, targetPortsBmp);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, targetPortsBmp));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxIngressFilteringEnableSet function
* @endinternal
*
* @brief   Set port filtering enabled state.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] direction                - direction.
* @param[in] packetType               - packet type.
* @param[in] enable                   -  or disable port filtering
*                                      GT_TRUE   - port filtering is enabled
*                                      GT_FALSE  - port filtering is disabled
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Applicable starting from revision A1.
*
*/
static GT_STATUS internal_cpssPxIngressFilteringEnableSet
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  CPSS_DIRECTION_ENT      direction,
    IN  CPSS_PX_PACKET_TYPE     packetType,
    IN  GT_BOOL                 enable
)
{
    GT_U32 regAddr;
    GT_STATUS rc;

    PRV_CPSS_PX_A1_DEV_CHECK_MAC(devNum);

    if (packetType >= CPSS_PX_INGRESS_PACKET_TYPE_KEY_TABLE_ENTRIES_MAX_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_ONE_PARAM_FORMAT_MAC(packetType));
    }

    switch(direction)
    {
        case CPSS_DIRECTION_BOTH_E:
            regAddr = PRV_PX_REG1_UNIT_PCP_MAC(devNum).PORTMAP.
                PCPPacketTypeEnableEgressPortFiltering;
            rc = prvCpssHwPpSetRegField(CAST_SW_DEVNUM(devNum), regAddr,
                packetType, 1, BOOL2BIT_MAC(enable));
            if(GT_OK != rc)
            {
                return rc;
            }
            /* Fallthrough to configure the feature for ingress */
            GT_ATTR_FALLTHROUGH;
        case CPSS_DIRECTION_INGRESS_E:
            regAddr = PRV_PX_REG1_UNIT_PCP_MAC(devNum).PORTMAP.
                PCPPacketTypeEnableIngressPortFiltering;
            break;
        case CPSS_DIRECTION_EGRESS_E:
            regAddr = PRV_PX_REG1_UNIT_PCP_MAC(devNum).PORTMAP.
                PCPPacketTypeEnableEgressPortFiltering;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    rc = prvCpssHwPpSetRegField(CAST_SW_DEVNUM(devNum), regAddr, packetType, 1,
        BOOL2BIT_MAC(enable));

    return rc;
}

/**
* @internal cpssPxIngressFilteringEnableSet function
* @endinternal
*
* @brief   Set port filtering enabled state.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] direction                - direction.
* @param[in] packetType               - packet type.
* @param[in] enable                   -  or disable port filtering
*                                      GT_TRUE   - port filtering is enabled
*                                      GT_FALSE  - port filtering is disabled
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Applicable starting from revision A1.
*
*/
GT_STATUS cpssPxIngressFilteringEnableSet
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  CPSS_DIRECTION_ENT      direction,
    IN  CPSS_PX_PACKET_TYPE     packetType,
    IN  GT_BOOL                 enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxIngressFilteringEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, direction, packetType, enable));

    rc = internal_cpssPxIngressFilteringEnableSet(devNum, direction, packetType, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, direction, packetType, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxIngressFilteringEnableGet function
* @endinternal
*
* @brief   Get port filtering enabled state.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] direction                - direction: ingress or egress.
* @param[in] packetType               - packet type.
*
* @param[out] enablePtr                - (pointer to) enabled state
*                                      GT_TRUE   - port filtering is enabled
*                                      GT_FALSE  - port filtering is disabled
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Applicable starting from revision A1.
*
*/
static GT_STATUS internal_cpssPxIngressFilteringEnableGet
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  CPSS_DIRECTION_ENT      direction,
    IN  CPSS_PX_PACKET_TYPE     packetType,
    OUT GT_BOOL                *enablePtr
)
{
    GT_U32 regAddr;
    GT_U32 regVal;
    GT_STATUS rc;

    PRV_CPSS_PX_A1_DEV_CHECK_MAC(devNum);

    if (packetType >= CPSS_PX_INGRESS_PACKET_TYPE_KEY_TABLE_ENTRIES_MAX_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_ONE_PARAM_FORMAT_MAC(packetType));
    }

    switch(direction)
    {
        case CPSS_DIRECTION_INGRESS_E:
            regAddr = PRV_PX_REG1_UNIT_PCP_MAC(devNum).PORTMAP.
                PCPPacketTypeEnableIngressPortFiltering;
            break;
        case CPSS_DIRECTION_EGRESS_E:
            regAddr = PRV_PX_REG1_UNIT_PCP_MAC(devNum).PORTMAP.
                PCPPacketTypeEnableEgressPortFiltering;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    rc = prvCpssHwPpGetRegField(CAST_SW_DEVNUM(devNum), regAddr, packetType, 1,
        &regVal);
    if (rc != GT_OK)
    {
        return rc;
    }

    *enablePtr = BIT2BOOL_MAC(regVal);

    return rc;
}

/**
* @internal cpssPxIngressFilteringEnableGet function
* @endinternal
*
* @brief   Get port filtering enabled state.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] direction                - direction: ingress or egress.
* @param[in] packetType               - packet type.
*
* @param[out] enablePtr                - (pointer to) enabled state
*                                      GT_TRUE   - port filtering is enabled
*                                      GT_FALSE  - port filtering is disabled
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Applicable starting from revision A1.
*
*/
GT_STATUS cpssPxIngressFilteringEnableGet
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  CPSS_DIRECTION_ENT      direction,
    IN  CPSS_PX_PACKET_TYPE     packetType,
    OUT GT_BOOL                 *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxIngressFilteringEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, direction, packetType, enablePtr));

    rc = internal_cpssPxIngressFilteringEnableGet(devNum, direction, packetType, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, direction, packetType, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxIngressIp2MeEntrySet function
* @endinternal
*
* @brief   Set IP2ME table entry.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] entryIndex               - entry index for IP2ME table (APPLICABLE RANGES: 1..7).
* @param[in] ip2meEntryPtr            - (pointer to) IP2ME entry
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Applicable starting from revision A1.
*
*/
static GT_STATUS internal_cpssPxIngressIp2MeEntrySet
(
    IN  GT_SW_DEV_NUM                   devNum,
    IN  GT_U32                          entryIndex,
    IN  CPSS_PX_INGRESS_IP2ME_ENTRY_STC *ip2meEntryPtr
)
{
    GT_U32 regAddr;
    GT_U32 regVal;
    GT_U32 ii;
    GT_STATUS rc;

    PRV_CPSS_PX_A1_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(ip2meEntryPtr);

    if ((0 == entryIndex) || (entryIndex > 7))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_ONE_PARAM_FORMAT_MAC(entryIndex));
    }

    if(0 == ip2meEntryPtr->prefixLength)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_ONE_PARAM_FORMAT_MAC(ip2meEntryPtr->prefixLength));
    }

    if((GT_TRUE == ip2meEntryPtr->isIpv6) && (ip2meEntryPtr->prefixLength > 128))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_ONE_PARAM_FORMAT_MAC(ip2meEntryPtr->prefixLength));
    }

    if((GT_FALSE == ip2meEntryPtr->isIpv6) && (ip2meEntryPtr->prefixLength > 32))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_ONE_PARAM_FORMAT_MAC(ip2meEntryPtr->prefixLength));
    }

    regAddr = PRV_PX_REG1_UNIT_PCP_IP2ME_MAC(devNum).PCPIP2MEControlBitsEntry[entryIndex-1];
    regVal = 0;
    U32_SET_FIELD_MAC(regVal, 0, 1, BOOL2BIT_MAC(ip2meEntryPtr->valid));
    U32_SET_FIELD_MAC(regVal, 1, 7, ip2meEntryPtr->prefixLength - 1);
    U32_SET_FIELD_MAC(regVal, 8, 1, BOOL2BIT_MAC(ip2meEntryPtr->isIpv6));
    rc = prvCpssHwPpWriteRegister(CAST_SW_DEVNUM(devNum), regAddr, regVal);
    if(GT_OK != rc)
    {
        return rc;
    }

    if(GT_TRUE == ip2meEntryPtr->isIpv6)
    {
        for(ii = 0; ii < 4; ii++)
        {
            regAddr = PRV_PX_REG1_UNIT_PCP_IP2ME_MAC(devNum).PCPIP2MEIpPart[ii][entryIndex-1];

            regVal =  (ip2meEntryPtr->ipAddr.ipv6Addr.arIP[((3 - ii) * 4)]     << 24) |
                      (ip2meEntryPtr->ipAddr.ipv6Addr.arIP[((3 - ii) * 4) + 1] << 16) |
                      (ip2meEntryPtr->ipAddr.ipv6Addr.arIP[((3 - ii) * 4) + 2] << 8)  |
                      (ip2meEntryPtr->ipAddr.ipv6Addr.arIP[((3 - ii) * 4) + 3]);

            rc = prvCpssHwPpWriteRegister(CAST_SW_DEVNUM(devNum), regAddr, regVal);
            if(GT_OK != rc)
            {
                return rc;
            }
        }
    }
    else
    {
        regAddr = PRV_PX_REG1_UNIT_PCP_IP2ME_MAC(devNum).PCPIP2MEIpPart[3][entryIndex-1];

        regVal = ((ip2meEntryPtr->ipAddr.ipv4Addr.arIP[0] << 24) |
                  (ip2meEntryPtr->ipAddr.ipv4Addr.arIP[1] << 16) |
                  (ip2meEntryPtr->ipAddr.ipv4Addr.arIP[2] << 8) |
                  (ip2meEntryPtr->ipAddr.ipv4Addr.arIP[3]));

        rc = prvCpssHwPpWriteRegister(CAST_SW_DEVNUM(devNum), regAddr, regVal);
    }

    return rc;
}

/**
* @internal cpssPxIngressIp2MeEntrySet function
* @endinternal
*
* @brief   Set IP2ME table entry.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] entryIndex               - entry index for IP2ME table (APPLICABLE RANGES: 1..7).
* @param[in] ip2meEntryPtr            - (pointer to) IP2ME entry
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Applicable starting from revision A1.
*
*/
GT_STATUS cpssPxIngressIp2MeEntrySet
(
    IN  GT_SW_DEV_NUM                   devNum,
    IN  GT_U32                          entryIndex,
    IN  CPSS_PX_INGRESS_IP2ME_ENTRY_STC *ip2meEntryPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxIngressIp2MeEntrySet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, entryIndex, ip2meEntryPtr));

    rc = internal_cpssPxIngressIp2MeEntrySet(devNum, entryIndex, ip2meEntryPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, entryIndex, ip2meEntryPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxIngressIp2MeEntryGet function
* @endinternal
*
* @brief   Get IP2ME table entry.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] entryIndex               - entry index for IP2ME table (APPLICABLE RANGES: 1..7).
*
* @param[out] ip2meEntryPtr            - (pointer to) IP2ME entry.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Applicable starting from revision A1.
*
*/
static GT_STATUS internal_cpssPxIngressIp2MeEntryGet
(
    IN  GT_SW_DEV_NUM                   devNum,
    IN  GT_U32                          entryIndex,
    OUT CPSS_PX_INGRESS_IP2ME_ENTRY_STC *ip2meEntryPtr
)
{
    GT_U32 regAddr;
    GT_U32 regVal;
    GT_U32 ii;
    GT_STATUS rc;

    PRV_CPSS_PX_A1_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(ip2meEntryPtr);

    if ((0 == entryIndex) || (entryIndex > 7))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_ONE_PARAM_FORMAT_MAC(entryIndex));
    }

    regAddr = PRV_PX_REG1_UNIT_PCP_IP2ME_MAC(devNum).PCPIP2MEControlBitsEntry[entryIndex-1];
    rc = prvCpssHwPpReadRegister(CAST_SW_DEVNUM(devNum), regAddr, &regVal);
    if(GT_OK != rc)
    {
        return rc;
    }

    ip2meEntryPtr->valid = BIT2BOOL_MAC(U32_GET_FIELD_MAC(regVal, 0, 1));
    ip2meEntryPtr->prefixLength = U32_GET_FIELD_MAC(regVal, 1, 7) + 1;
    ip2meEntryPtr->isIpv6 = BIT2BOOL_MAC(U32_GET_FIELD_MAC(regVal, 8, 1));


    if(GT_TRUE == ip2meEntryPtr->isIpv6)
    {
        for(ii = 0; ii < 4; ii++)
        {
            regAddr = PRV_PX_REG1_UNIT_PCP_IP2ME_MAC(devNum).PCPIP2MEIpPart[ii][entryIndex-1];
            rc = prvCpssHwPpReadRegister(CAST_SW_DEVNUM(devNum), regAddr, &regVal);
            if(GT_OK != rc)
            {
                return rc;
            }
            ip2meEntryPtr->ipAddr.ipv6Addr.arIP[((3 - ii) * 4)] = (regVal >> 24) & 0xFF;
            ip2meEntryPtr->ipAddr.ipv6Addr.arIP[((3 - ii) * 4) + 1] = (regVal >> 16) & 0xFF;
            ip2meEntryPtr->ipAddr.ipv6Addr.arIP[((3 - ii) * 4) + 2] = (regVal >> 8)  & 0xFF;
            ip2meEntryPtr->ipAddr.ipv6Addr.arIP[((3 - ii) * 4) + 3] = regVal & 0xFF;

        }
    }
    else
    {
        regAddr = PRV_PX_REG1_UNIT_PCP_IP2ME_MAC(devNum).PCPIP2MEIpPart[3][entryIndex-1];
        rc = prvCpssHwPpReadRegister(CAST_SW_DEVNUM(devNum), regAddr, &regVal);
        if(GT_OK != rc)
        {
            return rc;
        }
        ip2meEntryPtr->ipAddr.ipv4Addr.arIP[0] = (regVal >> 24) & 0xFF;
        ip2meEntryPtr->ipAddr.ipv4Addr.arIP[1] = (regVal >> 16) & 0xFF;
        ip2meEntryPtr->ipAddr.ipv4Addr.arIP[2] = (regVal >> 8) & 0xFF;
        ip2meEntryPtr->ipAddr.ipv4Addr.arIP[3] = regVal & 0xFF;
    }

    return rc;
}

/**
* @internal cpssPxIngressIp2MeEntryGet function
* @endinternal
*
* @brief   Get IP2ME table entry.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] entryIndex               - entry index for IP2ME table (APPLICABLE RANGES: 1..7).
*
* @param[out] ip2meEntryPtr            - (pointer to) IP2ME entry.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Applicable starting from revision A1.
*
*/
GT_STATUS cpssPxIngressIp2MeEntryGet
(
    IN  GT_SW_DEV_NUM                   devNum,
    IN  GT_U32                          entryIndex,
    OUT CPSS_PX_INGRESS_IP2ME_ENTRY_STC *ip2meEntryPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxIngressIp2MeEntryGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, entryIndex, ip2meEntryPtr));

    rc = internal_cpssPxIngressIp2MeEntryGet(devNum, entryIndex, ip2meEntryPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, entryIndex, ip2meEntryPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

