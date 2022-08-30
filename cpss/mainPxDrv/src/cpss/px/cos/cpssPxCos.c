/*******************************************************************************
*              (c), Copyright 2006, Marvell International Ltd.                 *
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
* @file cpssPxCos.c
*
* @brief Implementation of CPSS PX CoS API.
*
*
* @version   1
********************************************************************************
*/
#define CPSS_LOG_IN_MODULE_ENABLE
#include <cpss/px/cos/private/prvCpssPxCosLog.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

#include <cpss/common/cpssTypes.h>
#include <cpss/px/cpssPxTypes.h>
#include <cpss/px/config/private/prvCpssPxInfo.h>
#include <cpss/common/cos/cpssCosTypes.h>
#include <cpss/px/cos/cpssPxCos.h>

/**
* @internal prvCpssPxBitOffsetsConvert function
* @endinternal
*
* @brief   Converts bit offset from CPSS API to HW values and vice versa.
*
* @param[in,out] bitOffsetPtr             - (pointer to) HW/SW bit offset that shoul be converted
*                                      before write into or after read from register
* @param[in] numOfBits                - number of read/write bits
* @param[in,out] bitOffsetPtr             - (pointer to) HW/SW bit offset after conversion
*
* @retval GT_TRUE                  - Byte offset should be aligned after bit offset convertion
* @retval GT_FALSE                 - Byte offset should not be aligned after bit offset convertion
*/
static GT_BOOL prvCpssPxBitOffsetsConvert
(
    INOUT GT_U32    *bitOffsetPtr,
    IN    GT_U32    numOfBits
)
{
    GT_BOOL needByteConvert;
    GT_U32  bitOffset = *bitOffsetPtr;
    GT_U32  endBit;

    if (numOfBits == 0)
    {
        /* Bit field not used */
        return GT_FALSE;
    }

    endBit = bitOffset + numOfBits - 1;
    if (endBit / 8 > 0) 
    {
        /* Calculate HW/SW bit offset*/
        bitOffset = 0x7 & (8 - ((bitOffset + numOfBits) % 8));
        /* Byte offset should be converted */
        needByteConvert = GT_TRUE;
    }
    else
    {
        /* Calculate HW/SW bit offset*/
        bitOffset = 0x7 & (8 - (bitOffset + numOfBits));
        /* Byte offset should be converted */
        needByteConvert = GT_FALSE;
    }

    *bitOffsetPtr = bitOffset;

    return needByteConvert;
}

/**
* @internal internal_cpssPxCosFormatEntrySet function
* @endinternal
*
* @brief   Set CoS Mode and Attributes per Packet Type.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - Device number.
* @param[in] packetType               - Packet Type.
*                                      (APPLICABLE RANGE: 0..31)
* @param[in] cosFormatEntryPtr        - (Pointer to) CoS Format Entry for this packet type.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong parameter.
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note packetType is the one obtained from Packet Type lookup.
*
*/
static GT_STATUS internal_cpssPxCosFormatEntrySet
(
    IN  GT_SW_DEV_NUM                       devNum,
    IN  CPSS_PX_PACKET_TYPE                 packetType,
    IN  CPSS_PX_COS_FORMAT_ENTRY_STC        *cosFormatEntryPtr
)
{
    GT_STATUS rc;
    GT_U32 regAddr;         /* register address */
    GT_U32 regValue = 0;    /* register value */
    CPSS_PX_COS_FORMAT_ENTRY_STC   cosFormatEntryTemp;
    CPSS_PX_COS_FORMAT_ENTRY_STC   *cosFormatEntryTempPtr;

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(cosFormatEntryPtr);
    CPSS_PX_PACKET_TYPE_CHECK_MAC(packetType);

    /* Copy entry to temporary instance, original values could be changed */
    cosFormatEntryTemp = *cosFormatEntryPtr;
    /* Set pointer to copy instance */
    cosFormatEntryTempPtr = &cosFormatEntryTemp;

    switch (cosFormatEntryTempPtr->cosMode)
    {
        case CPSS_PX_COS_MODE_PORT_E:
            /* Set CoS Mode field */
            U32_SET_FIELD_MAC(regValue,  0,  2, 0);
            break;

        case CPSS_PX_COS_MODE_PACKET_DSA_E:
        case CPSS_PX_COS_MODE_PACKET_L2_E:
        case CPSS_PX_COS_MODE_PACKET_L3_E:
        case CPSS_PX_COS_MODE_PACKET_MPLS_E:
            /* Check fields */
            CPSS_PARAM_CHECK_MAX_MAC(cosFormatEntryTempPtr->cosByteOffset,BIT_5);
            CPSS_PARAM_CHECK_MAX_MAC(cosFormatEntryTempPtr->cosBitOffset,BIT_3);
            CPSS_PARAM_CHECK_MAX_MAC(cosFormatEntryTempPtr->cosNumOfBits-1,
                (cosFormatEntryTempPtr->cosMode == CPSS_PX_COS_MODE_PACKET_DSA_E) ? 7 :
                (cosFormatEntryTempPtr->cosMode == CPSS_PX_COS_MODE_PACKET_L2_E) ? 4 :
                (cosFormatEntryTempPtr->cosMode == CPSS_PX_COS_MODE_PACKET_L3_E) ? 6 :
                /* (cosFormatEntryPtr->cosMode == CPSS_PX_COS_MODE_PACKET_MPLS_E) ? */ 3);
            /* Set CoS Mode field */
            U32_SET_FIELD_MAC(regValue,  0,  2, 1);
            /* Set CoS Map Type field */
            U32_SET_FIELD_MAC(regValue,  2,  2,
                (cosFormatEntryTempPtr->cosMode == CPSS_PX_COS_MODE_PACKET_DSA_E) ? 0 :
                (cosFormatEntryTempPtr->cosMode == CPSS_PX_COS_MODE_PACKET_L2_E) ? 1 :
                (cosFormatEntryTempPtr->cosMode == CPSS_PX_COS_MODE_PACKET_L3_E) ? 2 :
                /* (cosFormatEntryPtr->cosMode == CPSS_PX_COS_MODE_PACKET_MPLS_E) ? */ 3);

            /* Convert byte and bit offsets to HW values */
            if (prvCpssPxBitOffsetsConvert(&cosFormatEntryTempPtr->cosBitOffset, 
                                            cosFormatEntryTempPtr->cosNumOfBits) == GT_TRUE) 
            {
                cosFormatEntryTempPtr->cosByteOffset--;
            }

            /* Set Offset and Length fields */
            U32_SET_FIELD_MAC(regValue,  4,  5, cosFormatEntryTempPtr->cosByteOffset);
            U32_SET_FIELD_MAC(regValue,  9,  3, cosFormatEntryTempPtr->cosBitOffset);
            U32_SET_FIELD_MAC(regValue, 12,  3, cosFormatEntryTempPtr->cosNumOfBits);
            break;

        case CPSS_PX_COS_MODE_FORMAT_ENTRY_E:
            /* Check fields */
            CPSS_PARAM_CHECK_MAX_MAC(cosFormatEntryTempPtr->cosAttributes.trafficClass,BIT_3);
            CPSS_PARAM_CHECK_MAX_MAC(cosFormatEntryTempPtr->cosAttributes.dropPrecedence,CPSS_DP_LAST_E);
            CPSS_PARAM_CHECK_MAX_MAC(cosFormatEntryTempPtr->cosAttributes.userPriority,BIT_3);
            CPSS_PARAM_CHECK_MAX_MAC(cosFormatEntryTempPtr->cosAttributes.dropEligibilityIndication,BIT_1);
            /* Set CoS Mode field */
            U32_SET_FIELD_MAC(regValue,  0,  2, 2);
            /* Set CoS Attributes fields */
            U32_SET_FIELD_MAC(regValue, 21,  3, cosFormatEntryTempPtr->cosAttributes.trafficClass);
            U32_SET_FIELD_MAC(regValue, 19,  2, cosFormatEntryTempPtr->cosAttributes.dropPrecedence);
            U32_SET_FIELD_MAC(regValue, 16,  3, cosFormatEntryTempPtr->cosAttributes.userPriority);
            U32_SET_FIELD_MAC(regValue, 15,  1, cosFormatEntryTempPtr->cosAttributes.dropEligibilityIndication);
            break;

        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,
                    LOG_ERROR_ONE_PARAM_FORMAT_MAC(cosFormatEntryTempPtr->cosMode));
    }

    /* Write bits */
    regAddr = PRV_PX_REG1_UNIT_PCP_MAC(devNum).COS.PCPCoSFormatTableEntry[packetType];

    rc = prvCpssHwPpSetRegField(CAST_SW_DEVNUM(devNum), regAddr, 0, 32, regValue);

    return rc;
}


/**
* @internal cpssPxCosFormatEntrySet function
* @endinternal
*
* @brief   Set CoS Mode and Attributes per Packet Type.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - Device number.
* @param[in] packetType               - Packet Type.
*                                      (APPLICABLE RANGE: 0..31)
* @param[in] cosFormatEntryPtr        - (Pointer to) CoS Format Entry for this packet type.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong parameter.
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note packetType is the one obtained from Packet Type lookup.
*
*/
GT_STATUS cpssPxCosFormatEntrySet
(
    IN  GT_SW_DEV_NUM                       devNum,
    IN  CPSS_PX_PACKET_TYPE                 packetType,
    IN  CPSS_PX_COS_FORMAT_ENTRY_STC        *cosFormatEntryPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxCosFormatEntrySet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, packetType, cosFormatEntryPtr));

    rc = internal_cpssPxCosFormatEntrySet(devNum, packetType, cosFormatEntryPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, packetType, cosFormatEntryPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssPxCosFormatEntryGet function
* @endinternal
*
* @brief   Get CoS Mode and Attributes per Packet Type.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - Device number.
* @param[in] packetType               - Packet Type.
*                                      (APPLICABLE RANGE: 0..31)
*
* @param[out] cosFormatEntryPtr        - (Pointer to) CoS Format Entry for this packet type.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong parameter.
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note packetType is the one obtained from Packet Type lookup.
*
*/
static GT_STATUS internal_cpssPxCosFormatEntryGet
(
    IN  GT_SW_DEV_NUM                       devNum,
    IN  CPSS_PX_PACKET_TYPE                 packetType,
    OUT CPSS_PX_COS_FORMAT_ENTRY_STC        *cosFormatEntryPtr
)
{
    GT_STATUS rc;
    GT_U32 regAddr;         /* register address */
    GT_U32 regValue = 0;    /* register value */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(cosFormatEntryPtr);
    CPSS_PX_PACKET_TYPE_CHECK_MAC(packetType);

    /* Read bits */
    regAddr = PRV_PX_REG1_UNIT_PCP_MAC(devNum).COS.PCPCoSFormatTableEntry[packetType];
    rc = prvCpssHwPpGetRegField(CAST_SW_DEVNUM(devNum), regAddr, 0, 32, &regValue);
    if (rc != GT_OK)
    {
        return rc;
    }

    switch (U32_GET_FIELD_MAC(regValue,  0,  2))
    {
    case 0:
        cosFormatEntryPtr->cosMode = CPSS_PX_COS_MODE_PORT_E;
        break;

    case 1:
        switch (U32_GET_FIELD_MAC(regValue,  2,  2))
        {
        case 0:
            cosFormatEntryPtr->cosMode = CPSS_PX_COS_MODE_PACKET_DSA_E;
            break;
        case 1:
            cosFormatEntryPtr->cosMode = CPSS_PX_COS_MODE_PACKET_L2_E;
            break;
        case 2:
            cosFormatEntryPtr->cosMode = CPSS_PX_COS_MODE_PACKET_L3_E;
            break;
        case 3:
            cosFormatEntryPtr->cosMode = CPSS_PX_COS_MODE_PACKET_MPLS_E;
            break;
        default:
            break;
        }
        break;
    case 2:
        cosFormatEntryPtr->cosMode = CPSS_PX_COS_MODE_FORMAT_ENTRY_E;
        break;

    default:
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE,
                LOG_ERROR_ONE_PARAM_FORMAT_MAC(regValue));
    }

    /* Get Offset and Length fields */
    cosFormatEntryPtr->cosByteOffset = U32_GET_FIELD_MAC(regValue,  4,  5);
    cosFormatEntryPtr->cosBitOffset = U32_GET_FIELD_MAC(regValue,  9,  3);
    cosFormatEntryPtr->cosNumOfBits = U32_GET_FIELD_MAC(regValue, 12,  3);

    /* Convert byte and bit offsets to software values */
    if (prvCpssPxBitOffsetsConvert(&cosFormatEntryPtr->cosBitOffset, 
                                    cosFormatEntryPtr->cosNumOfBits) == GT_TRUE) 
    {
        cosFormatEntryPtr->cosByteOffset++;
    }

    /* Get CoS Attributes fields */
    cosFormatEntryPtr->cosAttributes.trafficClass = U32_GET_FIELD_MAC(regValue, 21,  3);
    cosFormatEntryPtr->cosAttributes.dropPrecedence = U32_GET_FIELD_MAC(regValue, 19,  2);
    cosFormatEntryPtr->cosAttributes.userPriority = U32_GET_FIELD_MAC(regValue, 16,  3);
    cosFormatEntryPtr->cosAttributes.dropEligibilityIndication = U32_GET_FIELD_MAC(regValue, 15,  1);

    return rc;
}

/**
* @internal cpssPxCosFormatEntryGet function
* @endinternal
*
* @brief   Get CoS Mode and Attributes per Packet Type.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - Device number.
* @param[in] packetType               - Packet Type.
*                                      (APPLICABLE RANGE: 0..31)
*
* @param[out] cosFormatEntryPtr        - (Pointer to) CoS Format Entry for this packet type.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong parameter.
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note packetType is the one obtained from Packet Type lookup.
*
*/
GT_STATUS cpssPxCosFormatEntryGet
(
    IN  GT_SW_DEV_NUM                       devNum,
    IN  CPSS_PX_PACKET_TYPE                 packetType,
    OUT CPSS_PX_COS_FORMAT_ENTRY_STC        *cosFormatEntryPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxCosFormatEntryGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, packetType, cosFormatEntryPtr));

    rc = internal_cpssPxCosFormatEntryGet(devNum, packetType, cosFormatEntryPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, packetType, cosFormatEntryPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal prvCpssPxCosAttributesSet function
* @endinternal
*
* @brief   Generic Set CoS Attributes.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - Device number.
* @param[in] regAddr                  - Register address to be set.
* @param[in] cosAttributesPtr         - (Pointer to) CoS Attributes.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong parameter.
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS prvCpssPxCosAttributesSet
(
    IN  GT_SW_DEV_NUM               devNum,
    IN  GT_U32                      regAddr,
    IN  CPSS_PX_COS_ATTRIBUTES_STC  *cosAttributesPtr
)
{
    GT_STATUS rc;
    GT_U32 regValue = 0;    /* register value */
    GT_U32 regWriteBits;    /* number of bits to be written to register */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(cosAttributesPtr);

    /* Check fields */
    CPSS_PARAM_CHECK_MAX_MAC(cosAttributesPtr->trafficClass,BIT_3);
    CPSS_PARAM_CHECK_MAX_MAC(cosAttributesPtr->dropPrecedence,CPSS_DP_LAST_E);
    CPSS_PARAM_CHECK_MAX_MAC(cosAttributesPtr->userPriority,BIT_3);
    CPSS_PARAM_CHECK_MAX_MAC(cosAttributesPtr->dropEligibilityIndication,BIT_1);
    /* Set CoS Attributes fields */
    U32_SET_FIELD_MAC(regValue,  0,  3, cosAttributesPtr->trafficClass);
    U32_SET_FIELD_MAC(regValue,  3,  2, cosAttributesPtr->dropPrecedence);
    U32_SET_FIELD_MAC(regValue,  5,  3, cosAttributesPtr->userPriority);
    U32_SET_FIELD_MAC(regValue,  8,  1, cosAttributesPtr->dropEligibilityIndication);

    /* Write bits */
    regWriteBits = 32;
    rc = prvCpssHwPpSetRegField(CAST_SW_DEVNUM(devNum), regAddr, 0, regWriteBits, regValue);

    return rc;
}

/**
* @internal prvCpssPxCosAttributesGet function
* @endinternal
*
* @brief   Generic Get CoS Attributes.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - Device number.
* @param[in] regAddr                  - Register address to be set.
*
* @param[out] cosAttributesPtr         - (Pointer to) CoS Attributes.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong parameter.
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS prvCpssPxCosAttributesGet
(
    IN  GT_SW_DEV_NUM               devNum,
    IN  GT_U32                      regAddr,
    OUT CPSS_PX_COS_ATTRIBUTES_STC  *cosAttributesPtr
)
{
    GT_STATUS rc;
    GT_U32 regValue = 0;    /* register value */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(cosAttributesPtr);

    /* Read bits */
    rc = prvCpssHwPpGetRegField(CAST_SW_DEVNUM(devNum), regAddr, 0, 32, &regValue);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* Get CoS Attributes fields */
    cosAttributesPtr->trafficClass = U32_GET_FIELD_MAC(regValue,  0,  3);
    cosAttributesPtr->dropPrecedence = U32_GET_FIELD_MAC(regValue,  3,  2);
    cosAttributesPtr->userPriority = U32_GET_FIELD_MAC(regValue,  5,  3);
    cosAttributesPtr->dropEligibilityIndication = U32_GET_FIELD_MAC(regValue,  8,  1);

    return rc;
}

/**
* @internal internal_cpssPxCosPortAttributesSet function
* @endinternal
*
* @brief   Set CoS Attributes per Port.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - Device number.
* @param[in] portNum                  - Source port number.
* @param[in] cosAttributesPtr         - (Pointer to) CoS Attributes for this source port.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong parameter.
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Mapping used on packet types whose CoS mode is 'Port'.
*
*/
static GT_STATUS internal_cpssPxCosPortAttributesSet
(
    IN  GT_SW_DEV_NUM               devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    IN  CPSS_PX_COS_ATTRIBUTES_STC  *cosAttributesPtr
)
{
    GT_STATUS rc;
    GT_U32 regAddr;         /* register address */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PHY_PORT_NUM_CHECK_MAC(devNum, portNum);

    regAddr = PRV_PX_REG1_UNIT_PCP_MAC(devNum).COS.PCPPortCoSAttributes[portNum];

    rc = prvCpssPxCosAttributesSet(devNum, regAddr, cosAttributesPtr);

    return rc;
}

/**
* @internal cpssPxCosPortAttributesSet function
* @endinternal
*
* @brief   Set CoS Attributes per Port.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - Device number.
* @param[in] portNum                  - Source port number.
* @param[in] cosAttributesPtr         - (Pointer to) CoS Attributes for this source port.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong parameter.
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Mapping used on packet types whose CoS mode is 'Port'.
*
*/
GT_STATUS cpssPxCosPortAttributesSet
(
    IN  GT_SW_DEV_NUM               devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    IN  CPSS_PX_COS_ATTRIBUTES_STC  *cosAttributesPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxCosPortAttributesSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, cosAttributesPtr));

    rc = internal_cpssPxCosPortAttributesSet(devNum, portNum, cosAttributesPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, cosAttributesPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssPxCosPortAttributesGet function
* @endinternal
*
* @brief   Get CoS Attributes per Port.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - Device number.
* @param[in] portNum                  - Source port number.
*
* @param[out] cosAttributesPtr         - (Pointer to) CoS Attributes for this source port.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong parameter.
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Mapping used on packet types whose CoS mode is 'Port'.
*
*/
static GT_STATUS internal_cpssPxCosPortAttributesGet
(
    IN  GT_SW_DEV_NUM               devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    OUT CPSS_PX_COS_ATTRIBUTES_STC  *cosAttributesPtr
)
{
    GT_STATUS rc;
    GT_U32 regAddr;         /* register address */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PHY_PORT_NUM_CHECK_MAC(devNum, portNum);

    regAddr = PRV_PX_REG1_UNIT_PCP_MAC(devNum).COS.PCPPortCoSAttributes[portNum];

    rc = prvCpssPxCosAttributesGet(devNum, regAddr, cosAttributesPtr);

    return rc;
}

/**
* @internal cpssPxCosPortAttributesGet function
* @endinternal
*
* @brief   Get CoS Attributes per Port.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - Device number.
* @param[in] portNum                  - Source port number.
*
* @param[out] cosAttributesPtr         - (Pointer to) CoS Attributes for this source port.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong parameter.
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Mapping used on packet types whose CoS mode is 'Port'.
*
*/
GT_STATUS cpssPxCosPortAttributesGet
(
    IN  GT_SW_DEV_NUM               devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    OUT CPSS_PX_COS_ATTRIBUTES_STC  *cosAttributesPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxCosPortAttributesGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, cosAttributesPtr));

    rc = internal_cpssPxCosPortAttributesGet(devNum, portNum, cosAttributesPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, cosAttributesPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssPxCosPortL2MappingSet function
* @endinternal
*
* @brief   Set CoS L2 Attributes mapping per Port.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - Device number.
* @param[in] portNum                  - Source port number.
* @param[in] l2Index                  - L2 CoS bits extracted from packet.
*                                      (APPLICABLE RANGE: 0..15)
* @param[in] cosAttributesPtr         - (Pointer to) CoS L2 Attributes for this source port.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong parameter.
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Mapping used on packet types whose CoS mode is 'Packet_L2'.
*
*/
static GT_STATUS internal_cpssPxCosPortL2MappingSet
(
    IN  GT_SW_DEV_NUM               devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    IN  GT_U32                      l2Index,
    IN  CPSS_PX_COS_ATTRIBUTES_STC  *cosAttributesPtr
)
{
    GT_STATUS rc;
    GT_U32 regAddr;         /* register address */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PHY_PORT_NUM_CHECK_MAC(devNum, portNum);
    CPSS_PARAM_CHECK_MAX_MAC(l2Index,BIT_4);

    regAddr = PRV_PX_REG1_UNIT_PCP_MAC(devNum).COS.PCPPortL2CoSMapEntry[portNum][l2Index];

    rc = prvCpssPxCosAttributesSet(devNum, regAddr, cosAttributesPtr);

    return rc;
}

/**
* @internal cpssPxCosPortL2MappingSet function
* @endinternal
*
* @brief   Set CoS L2 Attributes mapping per Port.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - Device number.
* @param[in] portNum                  - Source port number.
* @param[in] l2Index                  - L2 CoS bits extracted from packet.
*                                      (APPLICABLE RANGE: 0..15)
* @param[in] cosAttributesPtr         - (Pointer to) CoS L2 Attributes for this source port.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong parameter.
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Mapping used on packet types whose CoS mode is 'Packet_L2'.
*
*/
GT_STATUS cpssPxCosPortL2MappingSet
(
    IN  GT_SW_DEV_NUM               devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    IN  GT_U32                      l2Index,
    IN  CPSS_PX_COS_ATTRIBUTES_STC  *cosAttributesPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxCosPortL2MappingSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, l2Index, cosAttributesPtr));

    rc = internal_cpssPxCosPortL2MappingSet(devNum, portNum, l2Index, cosAttributesPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, l2Index, cosAttributesPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssPxCosPortL2MappingGet function
* @endinternal
*
* @brief   Get CoS L2 Attributes mapping per Port.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - Device number.
* @param[in] portNum                  - Source port number.
* @param[in] l2Index                  - L2 CoS bits extracted from packet.
*                                      (APPLICABLE RANGE: 0..15)
*
* @param[out] cosAttributesPtr         - (Pointer to) CoS L2 Attributes for this source port.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong parameter.
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Mapping used on packet types whose CoS mode is 'Packet_L2'.
*
*/
static GT_STATUS internal_cpssPxCosPortL2MappingGet
(
    IN  GT_SW_DEV_NUM               devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    IN  GT_U32                      l2Index,
    OUT CPSS_PX_COS_ATTRIBUTES_STC  *cosAttributesPtr
)
{
    GT_STATUS rc;
    GT_U32 regAddr;         /* register address */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PHY_PORT_NUM_CHECK_MAC(devNum, portNum);
    CPSS_PARAM_CHECK_MAX_MAC(l2Index,BIT_4);

    regAddr = PRV_PX_REG1_UNIT_PCP_MAC(devNum).COS.PCPPortL2CoSMapEntry[portNum][l2Index];

    rc = prvCpssPxCosAttributesGet(devNum, regAddr, cosAttributesPtr);

    return rc;
}

/**
* @internal cpssPxCosPortL2MappingGet function
* @endinternal
*
* @brief   Get CoS L2 Attributes mapping per Port.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - Device number.
* @param[in] portNum                  - Source port number.
* @param[in] l2Index                  - L2 CoS bits extracted from packet.
*                                      (APPLICABLE RANGE: 0..15)
*
* @param[out] cosAttributesPtr         - (Pointer to) CoS L2 Attributes for this source port.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong parameter.
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Mapping used on packet types whose CoS mode is 'Packet_L2'.
*
*/
GT_STATUS cpssPxCosPortL2MappingGet
(
    IN  GT_SW_DEV_NUM               devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    IN  GT_U32                      l2Index,
    OUT CPSS_PX_COS_ATTRIBUTES_STC  *cosAttributesPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxCosPortL2MappingGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, l2Index, cosAttributesPtr));

    rc = internal_cpssPxCosPortL2MappingGet(devNum, portNum, l2Index, cosAttributesPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, l2Index, cosAttributesPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssPxCosL3MappingSet function
* @endinternal
*
* @brief   Set CoS Attributes mapping per L3 DSCP.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - Device number.
* @param[in] l3Index                  - L3 DSCP bits extracted from packet.
*                                      (APPLICABLE RANGE: 0..63)
* @param[in] cosAttributesPtr         - (Pointer to) CoS Attributes for this L3 DSCP index.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong parameter.
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Mapping used on packet types whose CoS mode is 'Packet_L3'.
*
*/
static GT_STATUS internal_cpssPxCosL3MappingSet
(
    IN  GT_SW_DEV_NUM               devNum,
    IN  GT_U32                      l3Index,
    IN  CPSS_PX_COS_ATTRIBUTES_STC  *cosAttributesPtr
)
{
    GT_STATUS rc;
    GT_U32 regAddr;         /* register address */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    CPSS_DATA_CHECK_MAX_MAC(l3Index,BIT_6);

    regAddr = PRV_PX_REG1_UNIT_PCP_MAC(devNum).COS.PCPL3CoSMapEntry[l3Index];

    rc = prvCpssPxCosAttributesSet(devNum, regAddr, cosAttributesPtr);

    return rc;
}

/**
* @internal cpssPxCosL3MappingSet function
* @endinternal
*
* @brief   Set CoS Attributes mapping per L3 DSCP.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - Device number.
* @param[in] l3Index                  - L3 DSCP bits extracted from packet.
*                                      (APPLICABLE RANGE: 0..63)
* @param[in] cosAttributesPtr         - (Pointer to) CoS Attributes for this L3 DSCP index.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong parameter.
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Mapping used on packet types whose CoS mode is 'Packet_L3'.
*
*/
GT_STATUS cpssPxCosL3MappingSet
(
    IN  GT_SW_DEV_NUM               devNum,
    IN  GT_U32                      l3Index,
    IN  CPSS_PX_COS_ATTRIBUTES_STC  *cosAttributesPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxCosL3MappingSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, l3Index, cosAttributesPtr));

    rc = internal_cpssPxCosL3MappingSet(devNum, l3Index, cosAttributesPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, l3Index, cosAttributesPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssPxCosL3MappingGet function
* @endinternal
*
* @brief   Get CoS Attributes mapping per L3 DSCP.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - Device number.
* @param[in] l3Index                  - L3 DSCP bits extracted from packet.
*                                      (APPLICABLE RANGE: 0..63)
*
* @param[out] cosAttributesPtr         - (Pointer to) CoS Attributes for this L3 DSCP index.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong parameter.
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Mapping used on packet types whose CoS mode is 'Packet_L3'.
*
*/
static GT_STATUS internal_cpssPxCosL3MappingGet
(
    IN  GT_SW_DEV_NUM               devNum,
    IN  GT_U32                      l3Index,
    OUT CPSS_PX_COS_ATTRIBUTES_STC  *cosAttributesPtr
)
{
    GT_STATUS rc;
    GT_U32 regAddr;         /* register address */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    CPSS_DATA_CHECK_MAX_MAC(l3Index,BIT_6);

    regAddr = PRV_PX_REG1_UNIT_PCP_MAC(devNum).COS.PCPL3CoSMapEntry[l3Index];

    rc = prvCpssPxCosAttributesGet(devNum, regAddr, cosAttributesPtr);

    return rc;
}

/**
* @internal cpssPxCosL3MappingGet function
* @endinternal
*
* @brief   Get CoS Attributes mapping per L3 DSCP.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - Device number.
* @param[in] l3Index                  - L3 DSCP bits extracted from packet.
*                                      (APPLICABLE RANGE: 0..63)
*
* @param[out] cosAttributesPtr         - (Pointer to) CoS Attributes for this L3 DSCP index.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong parameter.
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Mapping used on packet types whose CoS mode is 'Packet_L3'.
*
*/
GT_STATUS cpssPxCosL3MappingGet
(
    IN  GT_SW_DEV_NUM               devNum,
    IN  GT_U32                      l3Index,
    OUT CPSS_PX_COS_ATTRIBUTES_STC  *cosAttributesPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxCosL3MappingGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, l3Index, cosAttributesPtr));

    rc = internal_cpssPxCosL3MappingGet(devNum, l3Index, cosAttributesPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, l3Index, cosAttributesPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssPxCosMplsMappingSet function
* @endinternal
*
* @brief   Set CoS Attributes mapping per MPLS EXP.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - Device number.
* @param[in] mplsIndex                - MPLS EXP bits extracted from packet.
*                                      (APPLICABLE RANGE: 0..7)
* @param[in] cosAttributesPtr         - (Pointer to) CoS Attributes for this MPLS EXP index.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong parameter.
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Mapping used on packet types whose CoS mode is 'Packet_MPLS'.
*
*/
static GT_STATUS internal_cpssPxCosMplsMappingSet
(
    IN  GT_SW_DEV_NUM               devNum,
    IN  GT_U32                      mplsIndex,
    IN  CPSS_PX_COS_ATTRIBUTES_STC  *cosAttributesPtr
)
{
    GT_STATUS rc;
    GT_U32 regAddr;         /* register address */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    CPSS_DATA_CHECK_MAX_MAC(mplsIndex,BIT_3);

    regAddr = PRV_PX_REG1_UNIT_PCP_MAC(devNum).COS.PCPMPLSCoSMapEntry[mplsIndex];

    rc = prvCpssPxCosAttributesSet(devNum, regAddr, cosAttributesPtr);

    return rc;
}

/**
* @internal cpssPxCosMplsMappingSet function
* @endinternal
*
* @brief   Set CoS Attributes mapping per MPLS EXP.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - Device number.
* @param[in] mplsIndex                - MPLS EXP bits extracted from packet.
*                                      (APPLICABLE RANGE: 0..7)
* @param[in] cosAttributesPtr         - (Pointer to) CoS Attributes for this MPLS EXP index.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong parameter.
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Mapping used on packet types whose CoS mode is 'Packet_MPLS'.
*
*/
GT_STATUS cpssPxCosMplsMappingSet
(
    IN  GT_SW_DEV_NUM               devNum,
    IN  GT_U32                      mplsIndex,
    IN  CPSS_PX_COS_ATTRIBUTES_STC  *cosAttributesPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxCosMplsMappingSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, mplsIndex, cosAttributesPtr));

    rc = internal_cpssPxCosMplsMappingSet(devNum, mplsIndex, cosAttributesPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, mplsIndex, cosAttributesPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssPxCosMplsMappingGet function
* @endinternal
*
* @brief   Get CoS Attributes mapping per MPLS EXP.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - Device number.
* @param[in] mplsIndex                - MPLS EXP bits extracted from packet.
*                                      (APPLICABLE RANGE: 0..7)
*
* @param[out] cosAttributesPtr         - (Pointer to) CoS Attributes for this MPLS EXP index.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong parameter.
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Mapping used on packet types whose CoS mode is 'Packet_MPLS'.
*
*/
static GT_STATUS internal_cpssPxCosMplsMappingGet
(
    IN  GT_SW_DEV_NUM               devNum,
    IN  GT_U32                      mplsIndex,
    OUT CPSS_PX_COS_ATTRIBUTES_STC  *cosAttributesPtr
)
{
    GT_STATUS rc;
    GT_U32 regAddr;         /* register address */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    CPSS_DATA_CHECK_MAX_MAC(mplsIndex,BIT_3);

    regAddr = PRV_PX_REG1_UNIT_PCP_MAC(devNum).COS.PCPMPLSCoSMapEntry[mplsIndex];

    rc = prvCpssPxCosAttributesGet(devNum, regAddr, cosAttributesPtr);

    return rc;
}

/**
* @internal cpssPxCosMplsMappingGet function
* @endinternal
*
* @brief   Get CoS Attributes mapping per MPLS EXP.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - Device number.
* @param[in] mplsIndex                - MPLS EXP bits extracted from packet.
*                                      (APPLICABLE RANGE: 0..7)
*
* @param[out] cosAttributesPtr         - (Pointer to) CoS Attributes for this MPLS EXP index.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong parameter.
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Mapping used on packet types whose CoS mode is 'Packet_MPLS'.
*
*/
GT_STATUS cpssPxCosMplsMappingGet
(
    IN  GT_SW_DEV_NUM               devNum,
    IN  GT_U32                      mplsIndex,
    OUT CPSS_PX_COS_ATTRIBUTES_STC  *cosAttributesPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxCosMplsMappingGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, mplsIndex, cosAttributesPtr));

    rc = internal_cpssPxCosMplsMappingGet(devNum, mplsIndex, cosAttributesPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, mplsIndex, cosAttributesPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssPxCosDsaMappingSet function
* @endinternal
*
* @brief   Set CoS Attributes mapping from DSA header.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - Device number.
* @param[in] dsaIndex                 - CoS bits extracted from packet DSA header.
*                                      (APPLICABLE RANGE: 0..127)
* @param[in] cosAttributesPtr         - (Pointer to) CoS Attributes for this index.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong parameter.
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Mapping used on packet types whose CoS mode is 'Packet_DSA'.
*
*/
static GT_STATUS internal_cpssPxCosDsaMappingSet
(
    IN  GT_SW_DEV_NUM               devNum,
    IN  GT_U32                      dsaIndex,
    IN  CPSS_PX_COS_ATTRIBUTES_STC  *cosAttributesPtr
)
{
    GT_STATUS rc;
    GT_U32 regAddr;         /* register address */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    CPSS_DATA_CHECK_MAX_MAC(dsaIndex,BIT_7);

    regAddr = PRV_PX_REG1_UNIT_PCP_MAC(devNum).COS.PCPDsaCoSMapEntry[dsaIndex];

    rc = prvCpssPxCosAttributesSet(devNum, regAddr, cosAttributesPtr);

    return rc;
}

/**
* @internal cpssPxCosDsaMappingSet function
* @endinternal
*
* @brief   Set CoS Attributes mapping from DSA header.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - Device number.
* @param[in] dsaIndex                 - CoS bits extracted from packet DSA header.
*                                      (APPLICABLE RANGE: 0..127)
* @param[in] cosAttributesPtr         - (Pointer to) CoS Attributes for this index.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong parameter.
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Mapping used on packet types whose CoS mode is 'Packet_DSA'.
*
*/
GT_STATUS cpssPxCosDsaMappingSet
(
    IN  GT_SW_DEV_NUM               devNum,
    IN  GT_U32                      dsaIndex,
    IN  CPSS_PX_COS_ATTRIBUTES_STC  *cosAttributesPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxCosDsaMappingSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, dsaIndex, cosAttributesPtr));

    rc = internal_cpssPxCosDsaMappingSet(devNum, dsaIndex, cosAttributesPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, dsaIndex, cosAttributesPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssPxCosDsaMappingGet function
* @endinternal
*
* @brief   Get CoS Attributes mapping from DSA header.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - Device number.
* @param[in] dsaIndex                 - CoS bits extracted from packet DSA header.
*                                      (APPLICABLE RANGE: 0..127)
*
* @param[out] cosAttributesPtr         - (Pointer to) CoS Attributes for this index.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong parameter.
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Mapping used on packet types whose CoS mode is 'Packet_DSA'.
*
*/
static GT_STATUS internal_cpssPxCosDsaMappingGet
(
    IN  GT_SW_DEV_NUM               devNum,
    IN  GT_U32                      dsaIndex,
    OUT CPSS_PX_COS_ATTRIBUTES_STC        *cosAttributesPtr
)
{
    GT_STATUS rc;
    GT_U32 regAddr;         /* register address */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    CPSS_DATA_CHECK_MAX_MAC(dsaIndex,BIT_7);

    regAddr = PRV_PX_REG1_UNIT_PCP_MAC(devNum).COS.PCPDsaCoSMapEntry[dsaIndex];

    rc = prvCpssPxCosAttributesGet(devNum, regAddr, cosAttributesPtr);

    return rc;
}

/**
* @internal cpssPxCosDsaMappingGet function
* @endinternal
*
* @brief   Get CoS Attributes mapping from DSA header.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - Device number.
* @param[in] dsaIndex                 - CoS bits extracted from packet DSA header.
*                                      (APPLICABLE RANGE: 0..127)
*
* @param[out] cosAttributesPtr         - (Pointer to) CoS Attributes for this index.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong parameter.
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Mapping used on packet types whose CoS mode is 'Packet_DSA'.
*
*/
GT_STATUS cpssPxCosDsaMappingGet
(
    IN  GT_SW_DEV_NUM               devNum,
    IN  GT_U32                      dsaIndex,
    OUT CPSS_PX_COS_ATTRIBUTES_STC        *cosAttributesPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxCosDsaMappingGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, dsaIndex, cosAttributesPtr));

    rc = internal_cpssPxCosDsaMappingGet(devNum, dsaIndex, cosAttributesPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, dsaIndex, cosAttributesPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}



