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
* @file cpssPxIngressHash.c
*
* @brief CPSS PX implementation for packet hash calculation.
*
*
* @version   1
********************************************************************************
*/

#define CPSS_LOG_IN_MODULE_ENABLE
#include <cpss/generic/log/prvCpssPxGenDbLog.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>
#include <cpss/px/ingress/private/prvCpssPxIngressLog.h>
#include <cpss/px/ingress/cpssPxIngressHash.h>
#include <cpss/px/config/private/prvCpssPxInfo.h>

/**
* @internal internal_cpssPxIngressHashUdeEthertypeSet function
* @endinternal
*
* @brief   Set user-defined ethertypes for hash packet classificator.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] hashPacketType           - hash packet type. (APPLICABLE VALUES:
*                                      CPSS_PX_INGRESS_HASH_PACKET_TYPE_UDE1_E;
*                                      CPSS_PX_INGRESS_HASH_PACKET_TYPE_UDE2_E)
* @param[in] etherType                - ethertype.
* @param[in] valid                    - validity of the record.
*                                      GT_TRUE  - ethertype is valid.
*                                      GT_FALSE - ethertype is invalid.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxIngressHashUdeEthertypeSet
(
    IN GT_SW_DEV_NUM                        devNum,
    IN CPSS_PX_INGRESS_HASH_PACKET_TYPE_ENT hashPacketType,
    IN GT_U16                               etherType,
    IN GT_BOOL                              valid
)
{
    GT_U32    etherTypeReg; /* address of UDE register */
    GT_U32    value;        /* value to write in register field */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);

    switch(hashPacketType)
    {
        case CPSS_PX_INGRESS_HASH_PACKET_TYPE_UDE1_E:
            etherTypeReg =
                PRV_PX_REG1_UNIT_PCP_HASH_MAC(devNum).
                PCPHashPktTypeUDE1EtherType;
            break;
        case CPSS_PX_INGRESS_HASH_PACKET_TYPE_UDE2_E:
            etherTypeReg =
                PRV_PX_REG1_UNIT_PCP_HASH_MAC(devNum).
                PCPHashPktTypeUDE2EtherType;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    value = (etherType << 1) | BOOL2BIT_MAC(valid);
    return prvCpssHwPpSetRegField(CAST_SW_DEVNUM(devNum), etherTypeReg, 0, 17,
        value);
}

/**
* @internal cpssPxIngressHashUdeEthertypeSet function
* @endinternal
*
* @brief   Set user-defined ethertypes for hash packet classificator.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] hashPacketType           - hash packet type. (APPLICABLE VALUES:
*                                      CPSS_PX_INGRESS_HASH_PACKET_TYPE_UDE1_E;
*                                      CPSS_PX_INGRESS_HASH_PACKET_TYPE_UDE2_E)
* @param[in] etherType                - ethertype.
* @param[in] valid                    - validity of the record.
*                                      GT_TRUE  - ethertype is valid.
*                                      GT_FALSE - ethertype is invalid.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxIngressHashUdeEthertypeSet
(
    IN GT_SW_DEV_NUM                        devNum,
    IN CPSS_PX_INGRESS_HASH_PACKET_TYPE_ENT hashPacketType,
    IN GT_U16                               etherType,
    IN GT_BOOL                              valid
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxIngressHashUdeEthertypeSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, hashPacketType, etherType, valid));

    rc = internal_cpssPxIngressHashUdeEthertypeSet(devNum, hashPacketType, etherType, valid);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, hashPacketType, etherType, valid));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxIngressHashUdeEthertypeGet function
* @endinternal
*
* @brief   Get user-defined ethertypes (in addition to predefined types) for hash packet classificator.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] hashPacketType           - hash packet type. (APPLICABLE VALUES:
*                                      CPSS_PX_INGRESS_HASH_PACKET_TYPE_UDE1_E;
*                                      CPSS_PX_INGRESS_HASH_PACKET_TYPE_UDE2_E)
*
* @param[out] etherTypePtr             - (pointer to) ethertype.
* @param[out] validPtr                 - (pointer to) validity of the ethertype.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxIngressHashUdeEthertypeGet
(
    IN  GT_SW_DEV_NUM                        devNum,
    IN  CPSS_PX_INGRESS_HASH_PACKET_TYPE_ENT hashPacketType,
    OUT GT_U16                               *etherTypePtr,
    OUT GT_BOOL                              *validPtr
)
{
    GT_U32    etherTypeReg; /* address of UDE register */
    GT_U32    value;        /* value of field */
    GT_STATUS rc;           /* return code */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(etherTypePtr);
    CPSS_NULL_PTR_CHECK_MAC(validPtr);

    switch(hashPacketType)
    {
        case CPSS_PX_INGRESS_HASH_PACKET_TYPE_UDE1_E:
            etherTypeReg =
                PRV_PX_REG1_UNIT_PCP_HASH_MAC(devNum).
                PCPHashPktTypeUDE1EtherType;
            break;
        case CPSS_PX_INGRESS_HASH_PACKET_TYPE_UDE2_E:
            etherTypeReg =
                PRV_PX_REG1_UNIT_PCP_HASH_MAC(devNum).
                PCPHashPktTypeUDE2EtherType;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    rc = prvCpssHwPpGetRegField(CAST_SW_DEVNUM(devNum), etherTypeReg, 0, 17,
        &value);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    *validPtr = BIT2BOOL_MAC(value & 0x1);
    *etherTypePtr = (GT_U16)((value >> 1) & 0xFFFF);
    return GT_OK;
}

/**
* @internal cpssPxIngressHashUdeEthertypeGet function
* @endinternal
*
* @brief   Get user-defined ethertypes (in addition to predefined types) for hash packet classificator.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] hashPacketType           - hash packet type. (APPLICABLE VALUES:
*                                      CPSS_PX_INGRESS_HASH_PACKET_TYPE_UDE1_E;
*                                      CPSS_PX_INGRESS_HASH_PACKET_TYPE_UDE2_E)
*
* @param[out] etherTypePtr             - (pointer to) ethertype.
* @param[out] validPtr                 - (pointer to) validity of the ethertype.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxIngressHashUdeEthertypeGet
(
    IN  GT_SW_DEV_NUM                        devNum,
    IN  CPSS_PX_INGRESS_HASH_PACKET_TYPE_ENT hashPacketType,
    OUT GT_U16                               *etherTypePtr,
    OUT GT_BOOL                              *validPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxIngressHashUdeEthertypeGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, hashPacketType, etherTypePtr, validPtr));

    rc = internal_cpssPxIngressHashUdeEthertypeGet(devNum, hashPacketType, etherTypePtr, validPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, hashPacketType, etherTypePtr, validPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal prvCpssPxIngressHashPacketHashTypeToHwVal function
* @endinternal
*
* @brief   Get configuration register address for selected user-defined byte pair
*         and hash packet type.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] hashPacketType           - hash packet type.
*
* @param[out] hwVal                    - (pointer to) hardware value of hash packet type
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer
*/
static GT_STATUS prvCpssPxIngressHashPacketHashTypeToHwVal
(
    IN  CPSS_PX_INGRESS_HASH_PACKET_TYPE_ENT    hashPacketType,
    OUT GT_U32                                  *hwVal
)
{
    CPSS_NULL_PTR_CHECK_MAC(hwVal);

    switch(hashPacketType)
    {
        case CPSS_PX_INGRESS_HASH_PACKET_TYPE_IPV4_TCP_UDP_E:
            *hwVal = 0;
            break;
        case CPSS_PX_INGRESS_HASH_PACKET_TYPE_IPV6_TCP_UDP_E:
            *hwVal = 1;
            break;
        case CPSS_PX_INGRESS_HASH_PACKET_TYPE_IPV4_NO_TCP_UDP_E:
            *hwVal = 2;
            break;
        case CPSS_PX_INGRESS_HASH_PACKET_TYPE_IPV6_NO_TCP_UDP_E:
            *hwVal = 3;
            break;
        case CPSS_PX_INGRESS_HASH_PACKET_TYPE_MPLS_SINGLE_LABEL_E:
            *hwVal = 4;
            break;
        case CPSS_PX_INGRESS_HASH_PACKET_TYPE_MPLS_MULTI_LABEL_E:
            *hwVal = 5;
            break;
        case CPSS_PX_INGRESS_HASH_PACKET_TYPE_UDE1_E:
            *hwVal = 6;
            break;
        case CPSS_PX_INGRESS_HASH_PACKET_TYPE_UDE2_E:
            *hwVal = 7;
            break;
        case CPSS_PX_INGRESS_HASH_PACKET_TYPE_ETHERNET_E:
            *hwVal = 8;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    return GT_OK;
}

/**
* @internal internal_cpssPxIngressHashPacketTypeEntrySet function
* @endinternal
*
* @brief   Sets up packet hash key configuration for selected packet type
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] hashPacketType           - hash packet type.
* @param[in] udbpArr[CPSS_PX_INGRESS_HASH_UDBP_MAX_CNS] - user-defined byte pairs array of 21 elements.
* @param[in] sourcePortEnable         - enable usage of source port in packet hash.
*                                      GT_TRUE  - source port is used in hash
*                                      calculation.
*                                      GT_FALSE - source port is not used in hash
*                                      calculation.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_RANGE          - on offset in udbpArr is out of range
*/
static GT_STATUS internal_cpssPxIngressHashPacketTypeEntrySet
(
    IN  GT_SW_DEV_NUM                        devNum,
    IN  CPSS_PX_INGRESS_HASH_PACKET_TYPE_ENT hashPacketType,
    IN  CPSS_PX_INGRESS_HASH_UDBP_STC        udbpArr[CPSS_PX_INGRESS_HASH_UDBP_MAX_CNS],
    IN  GT_BOOL                              sourcePortEnable
)
{
    GT_STATUS   rc;               /* return code */
    GT_U32      ii;               /* iterator */
    GT_U32      registerAddr;     /* address of register */
    GT_U32      value;            /* value of field */
    GT_U32      hashPacketTypeHw; /* hash packet type HW value */
    /* array of HW values for anchor */
    GT_U32      anchorsHw[CPSS_PX_INGRESS_HASH_UDBP_MAX_CNS];

    /* Validate parameters */
    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(udbpArr);

    for(ii = 0; ii < CPSS_PX_INGRESS_HASH_UDBP_MAX_CNS; ii++)
    {
        if(PRV_CPSS_PX_A1_AND_ABOVE_CHECK_MAC(devNum))
        {
            if(udbpArr[ii].offset > 63)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
            }
        }
        else
        {
            if(udbpArr[ii].offset > 15)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
            }
        }

        if(NULL == udbpArr[ii].nibbleMaskArr)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PTR, LOG_ERROR_NO_MSG);
        }

        switch(udbpArr[ii].anchor)
        {
            case CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_L2_E:
                anchorsHw[ii] = 0;
                break;
            case CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_L3_E:
                anchorsHw[ii] = 1;
                break;
            case CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_L4_E:
                anchorsHw[ii] = 2;
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }

    /* Check validity of hashPacketType and convert it to HW value */
    rc = prvCpssPxIngressHashPacketHashTypeToHwVal(hashPacketType,
        &hashPacketTypeHw);
    if(GT_OK != rc)
    {
        return rc;
    }

    /* Write UDBP configurations */
    for (ii = 0 ; ii < CPSS_PX_INGRESS_HASH_UDBP_MAX_CNS; ii++)
    {
        registerAddr = PRV_PX_REG1_UNIT_PCP_HASH_MAC(devNum).
            PCPPktTypeHashConfigUDBPEntry[ii][hashPacketTypeHw];
        if(PRV_CPSS_PX_A1_AND_ABOVE_CHECK_MAC(devNum))
        {
            value = BOOL2BIT_MAC(udbpArr[ii].nibbleMaskArr[3]) << 8 |
                    BOOL2BIT_MAC(udbpArr[ii].nibbleMaskArr[2]) << 9 |
                    BOOL2BIT_MAC(udbpArr[ii].nibbleMaskArr[1]) << 10 |
                    BOOL2BIT_MAC(udbpArr[ii].nibbleMaskArr[0]) << 11 |
                    udbpArr[ii].offset << 2 | anchorsHw[ii];

            rc = prvCpssHwPpSetRegField(CAST_SW_DEVNUM(devNum), registerAddr, 0,
                12, value);
        }
        else
        {
            value = BOOL2BIT_MAC(udbpArr[ii].nibbleMaskArr[3]) << 6 |
                    BOOL2BIT_MAC(udbpArr[ii].nibbleMaskArr[2]) << 7 |
                    BOOL2BIT_MAC(udbpArr[ii].nibbleMaskArr[1]) << 8 |
                    BOOL2BIT_MAC(udbpArr[ii].nibbleMaskArr[0]) << 9 |
                    udbpArr[ii].offset << 2 | anchorsHw[ii];

            rc = prvCpssHwPpSetRegField(CAST_SW_DEVNUM(devNum), registerAddr, 0,
                10, value);
        }
        if(GT_OK != rc)
        {
            return rc;
        }
    }

    /* Enable source port usage */
    registerAddr = PRV_PX_REG1_UNIT_PCP_HASH_MAC(devNum).
        PCPPktTypeHashConfigIngrPort[hashPacketTypeHw];

    return prvCpssHwPpSetRegField(CAST_SW_DEVNUM(devNum), registerAddr, 0, 1,
        BOOL2BIT_MAC(sourcePortEnable));
}

/**
* @internal cpssPxIngressHashPacketTypeEntrySet function
* @endinternal
*
* @brief   Sets up packet hash key configuration for selected packet type
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] hashPacketType           - hash packet type.
* @param[in] udbpArr[CPSS_PX_INGRESS_HASH_UDBP_MAX_CNS] - user-defined byte pairs array of 21 elements.
* @param[in] sourcePortEnable         - enable usage of source port in packet hash.
*                                      GT_TRUE  - source port is used in hash
*                                      calculation.
*                                      GT_FALSE - source port is not used in hash
*                                      calculation.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_RANGE          - on offset in udbpArr is out of range
*/
GT_STATUS cpssPxIngressHashPacketTypeEntrySet
(
    IN  GT_SW_DEV_NUM                        devNum,
    IN  CPSS_PX_INGRESS_HASH_PACKET_TYPE_ENT hashPacketType,
    IN  CPSS_PX_INGRESS_HASH_UDBP_STC        udbpArr[CPSS_PX_INGRESS_HASH_UDBP_MAX_CNS],
    IN  GT_BOOL                              sourcePortEnable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxIngressHashPacketTypeEntrySet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, hashPacketType, udbpArr, sourcePortEnable));

    rc = internal_cpssPxIngressHashPacketTypeEntrySet(devNum, hashPacketType, udbpArr, sourcePortEnable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, hashPacketType, udbpArr, sourcePortEnable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxIngressHashPacketTypeEntryGet function
* @endinternal
*
* @brief   Get packet hash key configuration for selected packet type
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] hashPacketType           - hash packet type.
*
* @param[out] udbpArr[CPSS_PX_INGRESS_HASH_UDBP_MAX_CNS] - user-defined byte pairs array of 21 elements.
* @param[out] sourcePortEnablePtr      - (pointer to) enable usage of source port in
*                                      packet hash.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_STATE             - on invalid anchor field read
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxIngressHashPacketTypeEntryGet
(
    IN  GT_SW_DEV_NUM                        devNum,
    IN  CPSS_PX_INGRESS_HASH_PACKET_TYPE_ENT hashPacketType,
    OUT CPSS_PX_INGRESS_HASH_UDBP_STC        udbpArr[CPSS_PX_INGRESS_HASH_UDBP_MAX_CNS],
    OUT GT_BOOL                              *sourcePortEnablePtr
)
{
    GT_STATUS   rc;               /* return code */
    GT_U32      ii;               /* iterator */
    GT_U32      registerAddr;     /* address of register */
    GT_U32      value;            /* register field value */
    GT_U32      hashPacketTypeHw; /* hash packet type HW value */

    /* Validate parameters */
    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(sourcePortEnablePtr);
    CPSS_NULL_PTR_CHECK_MAC(udbpArr);

    /* Check validity of hashPacketType and convert it to HW value */
    rc = prvCpssPxIngressHashPacketHashTypeToHwVal(hashPacketType,
        &hashPacketTypeHw);
    if(GT_OK != rc)
    {
        return rc;
    }

    /* Write UDBP configurations */
    for (ii = 0 ; ii < CPSS_PX_INGRESS_HASH_UDBP_MAX_CNS; ii++)
    {
        registerAddr = PRV_PX_REG1_UNIT_PCP_HASH_MAC(devNum).
            PCPPktTypeHashConfigUDBPEntry[ii][hashPacketTypeHw];
        if(PRV_CPSS_PX_A1_AND_ABOVE_CHECK_MAC(devNum))
        {
            rc = prvCpssHwPpGetRegField(CAST_SW_DEVNUM(devNum), registerAddr, 0, 12,
               &value);
            if(GT_OK != rc)
            {
                return rc;
            }
            udbpArr[ii].nibbleMaskArr[3] = BIT2BOOL_MAC((value >> 8) & 0x1);
            udbpArr[ii].nibbleMaskArr[2] = BIT2BOOL_MAC((value >> 9) & 0x1);
            udbpArr[ii].nibbleMaskArr[1] = BIT2BOOL_MAC((value >> 10) & 0x1);
            udbpArr[ii].nibbleMaskArr[0] = BIT2BOOL_MAC((value >> 11) & 0x1);
            udbpArr[ii].offset = (value >> 2) & 0x3F;
        }
        else
        {
            rc = prvCpssHwPpGetRegField(CAST_SW_DEVNUM(devNum), registerAddr, 0, 10,
               &value);
            if(GT_OK != rc)
            {
                return rc;
            }
            udbpArr[ii].nibbleMaskArr[3] = BIT2BOOL_MAC((value >> 6) & 0x1);
            udbpArr[ii].nibbleMaskArr[2] = BIT2BOOL_MAC((value >> 7) & 0x1);
            udbpArr[ii].nibbleMaskArr[1] = BIT2BOOL_MAC((value >> 8) & 0x1);
            udbpArr[ii].nibbleMaskArr[0] = BIT2BOOL_MAC((value >> 9) & 0x1);
            udbpArr[ii].offset = (value >> 2) & 0xF;
        }
        switch(value & 0x3)
        {
            case 0:
                udbpArr[ii].anchor = CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_L2_E;
                break;
            case 1:
                udbpArr[ii].anchor = CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_L3_E;
                break;
            case 2:
                udbpArr[ii].anchor = CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_L4_E;
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
        }
    }

    /* Check source port usage */
    registerAddr = PRV_PX_REG1_UNIT_PCP_HASH_MAC(devNum).
        PCPPktTypeHashConfigIngrPort[hashPacketTypeHw];

    rc = prvCpssHwPpGetRegField(CAST_SW_DEVNUM(devNum), registerAddr, 0, 1,
        &value);
    *sourcePortEnablePtr = BIT2BOOL_MAC(value);
    return rc;
}

/**
* @internal cpssPxIngressHashPacketTypeEntryGet function
* @endinternal
*
* @brief   Get packet hash key configuration for selected packet type
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] hashPacketType           - hash packet type.
*
* @param[out] udbpArr[CPSS_PX_INGRESS_HASH_UDBP_MAX_CNS] - user-defined byte pairs array of 21 elements.
* @param[out] sourcePortEnablePtr      - (pointer to) enable usage of source port in
*                                      packet hash.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_STATE             - on invalid anchor field read
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxIngressHashPacketTypeEntryGet
(
    IN  GT_SW_DEV_NUM                        devNum,
    IN  CPSS_PX_INGRESS_HASH_PACKET_TYPE_ENT hashPacketType,
    OUT CPSS_PX_INGRESS_HASH_UDBP_STC        udbpArr[CPSS_PX_INGRESS_HASH_UDBP_MAX_CNS],
    OUT GT_BOOL                              *sourcePortEnablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxIngressHashPacketTypeEntryGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, hashPacketType, udbpArr, sourcePortEnablePtr));

    rc = internal_cpssPxIngressHashPacketTypeEntryGet(devNum, hashPacketType, udbpArr, sourcePortEnablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, hashPacketType, udbpArr, sourcePortEnablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxIngressHashPacketTypeHashModeSet function
* @endinternal
*
* @brief   Sets up hash calculation mode.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] hashPacketType           - hash packet type.
* @param[in] hashMode                 - hash calculation mode
* @param[in] bitOffsetsArr[CPSS_PX_INGRESS_HASH_SELECTED_BITS_MAX_CNS] - array of offsets (8 elements) of hash bits to be
*                                      extracted from 43 bytes of hash key. Relevant only
*                                      when hashMode == 'SELECTED BITS'
*                                      (APPLICABLE RANGES: 0..343)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_RANGE          - bitOffsetsArr member is out of range
*/
static GT_STATUS internal_cpssPxIngressHashPacketTypeHashModeSet
(
    IN  GT_SW_DEV_NUM                        devNum,
    IN  CPSS_PX_INGRESS_HASH_PACKET_TYPE_ENT hashPacketType,
    IN  CPSS_PX_INGRESS_HASH_MODE_ENT        hashMode,
    IN  GT_U32                               bitOffsetsArr[CPSS_PX_INGRESS_HASH_SELECTED_BITS_MAX_CNS]
)
{
    GT_STATUS rc;               /* return code */
    GT_U32    ii;               /* iterator */
    GT_U32    registerAddr;     /* address of register */
    GT_U32    value;            /* value of field */
    GT_U32    hashPacketTypeHw; /* hash packet type HW value */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);

    /* Check validity of hashPacketType and convert it to HW value */
    rc = prvCpssPxIngressHashPacketHashTypeToHwVal(hashPacketType,
        &hashPacketTypeHw);
    if(GT_OK != rc)
    {
        return rc;
    }

    /* Check validity of hashMode */
    switch(hashMode)
    {
        case CPSS_PX_INGRESS_HASH_MODE_CRC32_E:
        case CPSS_PX_INGRESS_HASH_MODE_SELECTED_BITS_E:
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if(CPSS_PX_INGRESS_HASH_MODE_CRC32_E == hashMode)
    {
        registerAddr = PRV_PX_REG1_UNIT_PCP_HASH_MAC(devNum).
            PCPPktTypeHashMode0Entry[hashPacketTypeHw];
        rc = prvCpssHwPpSetRegField(CAST_SW_DEVNUM(devNum), registerAddr,
            0, 1, 0);
        return rc;
    }

    CPSS_NULL_PTR_CHECK_MAC(bitOffsetsArr);

    for(ii = 0; ii < CPSS_PX_INGRESS_HASH_SELECTED_BITS_MAX_CNS; ii++)
    {
        if(bitOffsetsArr[ii] > 343)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }

    registerAddr = PRV_PX_REG1_UNIT_PCP_HASH_MAC(devNum).
                PCPPktTypeHashMode0Entry[hashPacketTypeHw];
    value = (bitOffsetsArr[2] << 19) | (bitOffsetsArr[1] << 10) |
            (bitOffsetsArr[0] << 1) | 0x1;
    rc = prvCpssHwPpSetRegField(CAST_SW_DEVNUM(devNum), registerAddr, 0, 28,
        value);
    if(GT_OK != rc)
    {
        return rc;
    }

    registerAddr = PRV_PX_REG1_UNIT_PCP_HASH_MAC(devNum).
                PCPPktTypeHashMode1Entry[hashPacketTypeHw];
    value = (bitOffsetsArr[5] << 18) | (bitOffsetsArr[4] << 9) |
             bitOffsetsArr[3];
    rc = prvCpssHwPpSetRegField(CAST_SW_DEVNUM(devNum), registerAddr, 0, 27,
        value);
    if(GT_OK != rc)
    {
        return rc;
    }

    registerAddr = PRV_PX_REG1_UNIT_PCP_HASH_MAC(devNum).
                PCPPktTypeHashMode2Entry[hashPacketTypeHw];

    if (PRV_CPSS_PX_A1_AND_ABOVE_CHECK_MAC(devNum) == 0)
    {
        value = bitOffsetsArr[6];
        rc = prvCpssHwPpSetRegField(CAST_SW_DEVNUM(devNum), registerAddr, 0, 9,
            value);
    }
    else
    {
        value = bitOffsetsArr[7] << 9 | bitOffsetsArr[6];
        rc = prvCpssHwPpSetRegField(CAST_SW_DEVNUM(devNum), registerAddr, 0, 18,
            value);
    }

    return rc;
}

/**
* @internal cpssPxIngressHashPacketTypeHashModeSet function
* @endinternal
*
* @brief   Sets up hash calculation mode.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] hashPacketType           - hash packet type.
* @param[in] hashMode                 - hash calculation mode
* @param[in] bitOffsetsArr[CPSS_PX_INGRESS_HASH_SELECTED_BITS_MAX_CNS] - array of offsets (8 elements) of hash bits to be
*                                      extracted from 43 bytes of hash key. Relevant only
*                                      when hashMode == 'SELECTED BITS'
*                                      (APPLICABLE RANGES: 0..343)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_RANGE          - bitOffsetsArr member is out of range
*/
GT_STATUS cpssPxIngressHashPacketTypeHashModeSet
(
    IN  GT_SW_DEV_NUM                        devNum,
    IN  CPSS_PX_INGRESS_HASH_PACKET_TYPE_ENT hashPacketType,
    IN  CPSS_PX_INGRESS_HASH_MODE_ENT        hashMode,
    IN  GT_U32                               bitOffsetsArr[CPSS_PX_INGRESS_HASH_SELECTED_BITS_MAX_CNS]
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxIngressHashPacketTypeHashModeSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, hashPacketType, hashMode, bitOffsetsArr));

    rc = internal_cpssPxIngressHashPacketTypeHashModeSet(devNum, hashPacketType, hashMode, bitOffsetsArr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, hashPacketType, hashMode, bitOffsetsArr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxIngressHashPacketTypeHashModeGet function
* @endinternal
*
* @brief   Gets hash calculation mode.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] hashPacketType           - hash packet type.
*
* @param[out] hashModePtr              - (pointer to) hash calculation mode.
* @param[out] bitOffsetsArr[CPSS_PX_INGRESS_HASH_SELECTED_BITS_MAX_CNS] - (pointer to) array of offsets (8 elements) of hash
*                                      bits to be extracted from 43 bytes of hash key.
*                                      Relevant only when hashMode == 'SELECTED_BITS'.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxIngressHashPacketTypeHashModeGet
(
    IN  GT_SW_DEV_NUM                        devNum,
    IN  CPSS_PX_INGRESS_HASH_PACKET_TYPE_ENT hashPacketType,
    OUT CPSS_PX_INGRESS_HASH_MODE_ENT        *hashModePtr,
    OUT GT_U32                               bitOffsetsArr[CPSS_PX_INGRESS_HASH_SELECTED_BITS_MAX_CNS]
)
{
    GT_U32      registerAddr;     /* address of register */
    GT_U32      value;            /* value of field */
    GT_STATUS   rc;               /* return code */
    GT_U32      hashPacketTypeHw; /* hash packet type HW value */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(hashModePtr);
    CPSS_NULL_PTR_CHECK_MAC(bitOffsetsArr);

    /* Check validity of hashPacketType and convert it to HW value */
    rc = prvCpssPxIngressHashPacketHashTypeToHwVal(hashPacketType,
        &hashPacketTypeHw);
    if(GT_OK != rc)
    {
        return rc;
    }

    registerAddr = PRV_PX_REG1_UNIT_PCP_HASH_MAC(devNum).
                PCPPktTypeHashMode0Entry[hashPacketTypeHw];
    rc = prvCpssHwPpGetRegField(CAST_SW_DEVNUM(devNum), registerAddr, 0, 28,
        &value);
    if(GT_OK != rc)
    {
        return rc;
    }
    *hashModePtr = (value & 0x1) ? CPSS_PX_INGRESS_HASH_MODE_SELECTED_BITS_E:
                                   CPSS_PX_INGRESS_HASH_MODE_CRC32_E;
    if (*hashModePtr == CPSS_PX_INGRESS_HASH_MODE_CRC32_E)
    {
        return rc;
    }
    bitOffsetsArr[0] = (value >> 1) & 0x1FF;
    bitOffsetsArr[1] = (value >> 10) & 0x1FF;
    bitOffsetsArr[2] = (value >> 19) & 0x1FF;

    registerAddr = PRV_PX_REG1_UNIT_PCP_HASH_MAC(devNum).
                PCPPktTypeHashMode1Entry[hashPacketTypeHw];
    rc = prvCpssHwPpGetRegField(CAST_SW_DEVNUM(devNum), registerAddr, 0, 27,
        &value);
    if(GT_OK != rc)
    {
        return rc;
    }
    bitOffsetsArr[3] = value & 0x1FF;
    bitOffsetsArr[4] = (value >> 9) & 0x1FF;
    bitOffsetsArr[5] = (value >> 18) & 0x1FF;

    registerAddr = PRV_PX_REG1_UNIT_PCP_HASH_MAC(devNum).
                PCPPktTypeHashMode2Entry[hashPacketTypeHw];

    if (PRV_CPSS_PX_A1_AND_ABOVE_CHECK_MAC(devNum) == 0)
    {
        rc = prvCpssHwPpGetRegField(CAST_SW_DEVNUM(devNum), registerAddr, 0, 9,
            &value);
        if(GT_OK != rc)
        {
            return rc;
        }
        bitOffsetsArr[6] = value & 0x1FF;
    }
    else
    {
        rc = prvCpssHwPpGetRegField(CAST_SW_DEVNUM(devNum), registerAddr, 0, 18,
            &value);
        if(GT_OK != rc)
        {
            return rc;
        }
        bitOffsetsArr[6] = value & 0x1FF;
        bitOffsetsArr[7] = (value >> 9) & 0x1FF;
    }
    return rc;
}

/**
* @internal cpssPxIngressHashPacketTypeHashModeGet function
* @endinternal
*
* @brief   Gets hash calculation mode.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] hashPacketType           - hash packet type.
*
* @param[out] hashModePtr              - (pointer to) hash calculation mode.
* @param[out] bitOffsetsArr[CPSS_PX_INGRESS_HASH_SELECTED_BITS_MAX_CNS] - (pointer to) array of offsets (8 elements) of hash
*                                      bits to be extracted from 43 bytes of hash key.
*                                      Relevant only when hashMode == 'SELECTED_BITS'.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxIngressHashPacketTypeHashModeGet
(
    IN  GT_SW_DEV_NUM                        devNum,
    IN  CPSS_PX_INGRESS_HASH_PACKET_TYPE_ENT hashPacketType,
    OUT CPSS_PX_INGRESS_HASH_MODE_ENT        *hashModePtr,
    OUT GT_U32                               bitOffsetsArr[CPSS_PX_INGRESS_HASH_SELECTED_BITS_MAX_CNS]
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxIngressHashPacketTypeHashModeGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, hashPacketType, hashModePtr, bitOffsetsArr));

    rc = internal_cpssPxIngressHashPacketTypeHashModeGet(devNum, hashPacketType, hashModePtr, bitOffsetsArr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, hashPacketType, hashModePtr, bitOffsetsArr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxIngressHashSeedSet function
* @endinternal
*
* @brief   Sets up CRC32 seed for hash calculation
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] crc32Seed                - seed for CRC32 packet hash calculation.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxIngressHashSeedSet
(
    IN  GT_SW_DEV_NUM                        devNum,
    IN  GT_U32                               crc32Seed
)
{
    GT_U32      registerAddr; /* address of register */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);

    registerAddr = PRV_PX_REG1_UNIT_PCP_HASH_MAC(devNum).PCPHashCRC32Seed;
    return prvCpssHwPpWriteRegister(CAST_SW_DEVNUM(devNum), registerAddr,
        crc32Seed);
}

/**
* @internal cpssPxIngressHashSeedSet function
* @endinternal
*
* @brief   Sets up CRC32 seed for hash calculation
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] crc32Seed                - seed for CRC32 packet hash calculation.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxIngressHashSeedSet
(
    IN  GT_SW_DEV_NUM                        devNum,
    IN  GT_U32                               crc32Seed
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxIngressHashSeedSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, crc32Seed));

    rc = internal_cpssPxIngressHashSeedSet(devNum, crc32Seed);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, crc32Seed));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxIngressHashSeedGet function
* @endinternal
*
* @brief   Get CRC32 seed for hash calculation
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
*
* @param[out] crc32SeedPtr             - (pointer to) seed for CRC32 packet hash
*                                      calculation.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxIngressHashSeedGet
(
    IN  GT_SW_DEV_NUM                        devNum,
    OUT GT_U32                               *crc32SeedPtr
)
{
    GT_U32      registerAddr; /* address of register */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(crc32SeedPtr);

    registerAddr = PRV_PX_REG1_UNIT_PCP_HASH_MAC(devNum).PCPHashCRC32Seed;
    return prvCpssHwPpReadRegister(CAST_SW_DEVNUM(devNum), registerAddr,
        crc32SeedPtr);
}

/**
* @internal cpssPxIngressHashSeedGet function
* @endinternal
*
* @brief   Get CRC32 seed for hash calculation
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
*
* @param[out] crc32SeedPtr             - (pointer to) seed for CRC32 packet hash
*                                      calculation.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxIngressHashSeedGet
(
    IN  GT_SW_DEV_NUM                        devNum,
    OUT GT_U32                               *crc32SeedPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxIngressHashSeedGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, crc32SeedPtr));

    rc = internal_cpssPxIngressHashSeedGet(devNum, crc32SeedPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, crc32SeedPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxIngressHashPortIndexModeSet function
* @endinternal
*
* @brief   Sets up LAG designated port table indexing mode for selected source
*         port.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - port number.
* @param[in] indexMode                - designated port table indexing mode.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxIngressHashPortIndexModeSet
(
    IN  GT_SW_DEV_NUM                                                 devNum,
    IN  GT_PHYSICAL_PORT_NUM                                          portNum,
    IN  CPSS_PX_INGRESS_HASH_PORT_INDEX_MODE_ENT                      indexMode
)
{
    GT_U32      registerAddr; /* address of register */
    GT_U32      indexModeHw;  /* indexing mode hardware value */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PHY_PORT_NUM_CHECK_MAC(devNum, portNum);

    switch(indexMode)
    {
        case CPSS_PX_INGRESS_HASH_PORT_INDEX_MODE_HASH_E:
            indexModeHw = 0;
            break;
        case CPSS_PX_INGRESS_HASH_PORT_INDEX_MODE_PRNG_E:
            indexModeHw = 1;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    registerAddr = PRV_PX_REG1_UNIT_PCP_HASH_MAC(devNum).PCPLagTableIndexMode;

    return prvCpssHwPpSetRegField(CAST_SW_DEVNUM(devNum), registerAddr,
        portNum, 1, indexModeHw);
}

/**
* @internal cpssPxIngressHashPortIndexModeSet function
* @endinternal
*
* @brief   Sets up LAG designated port table indexing mode for selected source
*         port.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - port number.
* @param[in] indexMode                - designated port table indexing mode.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxIngressHashPortIndexModeSet
(
    IN  GT_SW_DEV_NUM                                                 devNum,
    IN  GT_PHYSICAL_PORT_NUM                                          portNum,
    IN  CPSS_PX_INGRESS_HASH_PORT_INDEX_MODE_ENT                      indexMode
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxIngressHashPortIndexModeSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, indexMode));

    rc = internal_cpssPxIngressHashPortIndexModeSet(devNum, portNum, indexMode);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, indexMode));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxIngressHashPortIndexModeGet function
* @endinternal
*
* @brief   Gets LAG designated port table indexing mode for selected source
*         port.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - port number.
*
* @param[out] indexModePtr             - designated port table indexing mode.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxIngressHashPortIndexModeGet
(
    IN  GT_SW_DEV_NUM                                              devNum,
    IN  GT_PHYSICAL_PORT_NUM                                       portNum,
    OUT CPSS_PX_INGRESS_HASH_PORT_INDEX_MODE_ENT                   *indexModePtr
)
{
    GT_U32 registerAddr; /* address of register */
    GT_U32 rc;           /* return code*/
    GT_U32 indexModeHw;  /* indexing mode hardware value */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PHY_PORT_NUM_CHECK_MAC(devNum, portNum);
    CPSS_NULL_PTR_CHECK_MAC(indexModePtr);

    registerAddr = PRV_PX_REG1_UNIT_PCP_HASH_MAC(devNum).PCPLagTableIndexMode;

    rc = prvCpssHwPpGetRegField(CAST_SW_DEVNUM(devNum), registerAddr,
        portNum, 1, &indexModeHw);

    *indexModePtr = (0 == indexModeHw) ?
        CPSS_PX_INGRESS_HASH_PORT_INDEX_MODE_HASH_E:
        CPSS_PX_INGRESS_HASH_PORT_INDEX_MODE_PRNG_E;

    return rc;
}

/**
* @internal cpssPxIngressHashPortIndexModeGet function
* @endinternal
*
* @brief   Gets LAG designated port table indexing mode for selected source
*         port.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - port number.
*
* @param[out] indexModePtr             - designated port table indexing mode.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxIngressHashPortIndexModeGet
(
    IN  GT_SW_DEV_NUM                                              devNum,
    IN  GT_PHYSICAL_PORT_NUM                                       portNum,
    OUT CPSS_PX_INGRESS_HASH_PORT_INDEX_MODE_ENT                   *indexModePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxIngressHashPortIndexModeGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, indexModePtr));

    rc = internal_cpssPxIngressHashPortIndexModeGet(devNum, portNum, indexModePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, indexModePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxIngressHashDesignatedPortsEntrySet function
* @endinternal
*
* @brief   Set entry in LAG designated ports table.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] entryIndex               - entry index (APPLICABLE RANGES: 0..255).
* @param[in] portsBmp                 - member port bitmap.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxIngressHashDesignatedPortsEntrySet
(
    IN  GT_SW_DEV_NUM                                            devNum,
    IN  GT_U32                                                   entryIndex,
    IN  CPSS_PX_PORTS_BMP                                        portsBmp
)
{
    GT_U32  registerAddr; /* address of register */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_BMP_CHECK_MAC(devNum, portsBmp);
    if (PRV_CPSS_PX_A1_AND_ABOVE_CHECK_MAC(devNum) == 0)
    {
        CPSS_PARAM_CHECK_MAX_MAC(entryIndex,BIT_7);
    }
    else
    {
        CPSS_PARAM_CHECK_MAX_MAC(entryIndex,BIT_8);
    }

    registerAddr = PRV_PX_REG1_UNIT_PCP_MAC(devNum).PORTMAP.
        PCPLagDesignatedPortEntry[entryIndex];

    return prvCpssHwPpSetRegField(CAST_SW_DEVNUM(devNum), registerAddr, 0, 17,
        portsBmp);
}

/**
* @internal cpssPxIngressHashDesignatedPortsEntrySet function
* @endinternal
*
* @brief   Set entry in LAG designated ports table.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] entryIndex               - entry index (APPLICABLE RANGES: 0..255).
* @param[in] portsBmp                 - member port bitmap.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxIngressHashDesignatedPortsEntrySet
(
    IN  GT_SW_DEV_NUM                                            devNum,
    IN  GT_U32                                                   entryIndex,
    IN  CPSS_PX_PORTS_BMP                                        portsBmp
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxIngressHashDesignatedPortsEntrySet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, entryIndex, portsBmp));

    rc = internal_cpssPxIngressHashDesignatedPortsEntrySet(devNum, entryIndex, portsBmp);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, entryIndex, portsBmp));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxIngressHashDesignatedPortsEntryGet function
* @endinternal
*
* @brief   Get entry in LAG designated ports table.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] entryIndex               - entry index (APPLICABLE RANGES: 0..255).
*
* @param[out] portsBmpPtr              - (pointer to) member port bitmap.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxIngressHashDesignatedPortsEntryGet
(
    IN  GT_SW_DEV_NUM                                            devNum,
    IN  GT_U32                                                   entryIndex,
    OUT CPSS_PX_PORTS_BMP                                        *portsBmpPtr
)
{
    GT_U32 registerAddr; /* address of register */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    if (PRV_CPSS_PX_A1_AND_ABOVE_CHECK_MAC(devNum) == 0)
    {
        CPSS_PARAM_CHECK_MAX_MAC(entryIndex, BIT_7);
    }
    else
    {
        CPSS_PARAM_CHECK_MAX_MAC(entryIndex, BIT_8);
    }
    CPSS_NULL_PTR_CHECK_MAC(portsBmpPtr);

    registerAddr = PRV_PX_REG1_UNIT_PCP_MAC(devNum).PORTMAP.
        PCPLagDesignatedPortEntry[entryIndex];

    return prvCpssHwPpGetRegField(CAST_SW_DEVNUM(devNum), registerAddr, 0, 17,
        portsBmpPtr);
}

/**
* @internal cpssPxIngressHashDesignatedPortsEntryGet function
* @endinternal
*
* @brief   Get entry in LAG designated ports table.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] entryIndex               - entry index (APPLICABLE RANGES: 0..255).
*
* @param[out] portsBmpPtr              - (pointer to) member port bitmap.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxIngressHashDesignatedPortsEntryGet
(
    IN  GT_SW_DEV_NUM                                            devNum,
    IN  GT_U32                                                   entryIndex,
    OUT CPSS_PX_PORTS_BMP                                        *portsBmpPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxIngressHashDesignatedPortsEntryGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, entryIndex, portsBmpPtr));

    rc = internal_cpssPxIngressHashDesignatedPortsEntryGet(devNum, entryIndex, portsBmpPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, entryIndex, portsBmpPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxIngressHashLagTableModeSet function
* @endinternal
*
* @brief   Sets global LAG Designated Port Table mode
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] lagTableMode             - LAG table mode.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Applicable starting from revision A1
*
*/
static GT_STATUS internal_cpssPxIngressHashLagTableModeSet
(
    IN GT_SW_DEV_NUM                                devNum,
    IN CPSS_PX_INGRESS_HASH_LAG_TABLE_MODE_ENT      lagTableMode
)
{
    GT_U32 regAddr;
    GT_U32 regVal;

    PRV_CPSS_PX_A1_DEV_CHECK_MAC(devNum);

    switch (lagTableMode) 
    {
        case CPSS_PX_INGRESS_HASH_LAG_TABLE_SINGLE_MODE_E:
            regVal = 0;
            break;
        case CPSS_PX_INGRESS_HASH_LAG_TABLE_DUAL_MODE_E:
            regVal = 1;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* PCP Global configuration */
    regAddr = PRV_PX_REG1_UNIT_PCP_MAC(devNum).general.PCPGlobalConfiguration;

    /* LAG Table Mode */
    return prvCpssHwPpSetRegField(CAST_SW_DEVNUM(devNum), regAddr, 0, 1, regVal);
}

/**
* @internal cpssPxIngressHashLagTableModeSet function
* @endinternal
*
* @brief   Sets global LAG Designated Port Table mode
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] lagTableMode             - LAG table mode.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Applicable starting from revision A1
*
*/
GT_STATUS cpssPxIngressHashLagTableModeSet
(
    IN GT_SW_DEV_NUM                                devNum,
    IN CPSS_PX_INGRESS_HASH_LAG_TABLE_MODE_ENT      lagTableMode
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxIngressHashLagTableModeSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, lagTableMode));

    rc = internal_cpssPxIngressHashLagTableModeSet(devNum, lagTableMode);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, lagTableMode));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxIngressHashLagTableModeGet function
* @endinternal
*
* @brief   Gets global LAG Designated Port Table mode
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
*
* @param[out] lagTableModePtr          - (pointer to) LAG table mode.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Applicable starting from revision A1
*
*/
static GT_STATUS internal_cpssPxIngressHashLagTableModeGet
(
    IN GT_SW_DEV_NUM                                devNum,
    OUT CPSS_PX_INGRESS_HASH_LAG_TABLE_MODE_ENT    *lagTableModePtr
)
{
    GT_U32 regAddr;
    GT_U32 regVal;
    GT_STATUS rc;

    PRV_CPSS_PX_A1_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(lagTableModePtr);

    /* PCP Global configuration */
    regAddr = PRV_PX_REG1_UNIT_PCP_MAC(devNum).general.PCPGlobalConfiguration;
    /* LAG Table Mode */
    rc = prvCpssHwPpGetRegField(CAST_SW_DEVNUM(devNum), regAddr, 0, 1, &regVal);
    if (rc != GT_OK)
    {
        return rc;
    }
    switch (regVal) 
    {
        case 0:
            *lagTableModePtr = CPSS_PX_INGRESS_HASH_LAG_TABLE_SINGLE_MODE_E;
            break;
        case 1:
            *lagTableModePtr = CPSS_PX_INGRESS_HASH_LAG_TABLE_DUAL_MODE_E;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    return GT_OK;
}

/**
* @internal cpssPxIngressHashLagTableModeGet function
* @endinternal
*
* @brief   Gets global LAG Designated Port Table mode
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
*
* @param[out] lagTableModePtr          - (pointer to) LAG table mode.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Applicable starting from revision A1
*
*/
GT_STATUS cpssPxIngressHashLagTableModeGet
(
    IN GT_SW_DEV_NUM                                devNum,
    OUT CPSS_PX_INGRESS_HASH_LAG_TABLE_MODE_ENT    *lagTableModePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxIngressHashLagTableModeGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, lagTableModePtr));

    rc = internal_cpssPxIngressHashLagTableModeGet(devNum, lagTableModePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, lagTableModePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxIngressHashPacketTypeLagTableNumberSet function
* @endinternal
*
* @brief   Sets LAG table number for specific packet type
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] packetType               - index of the packet type key in the table.
*                                      (APPLICABLE RANGES: 0..31)
* @param[in] lagTableNumber           - the LAG table number.
*                                      (APPLICABLE RANGES: 0..1)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Applicable starting from revision A1
*
*/
static GT_STATUS internal_cpssPxIngressHashPacketTypeLagTableNumberSet
(
    IN GT_SW_DEV_NUM                                devNum,
    IN CPSS_PX_PACKET_TYPE                          packetType,
    IN GT_U32                                       lagTableNumber
)
{
    GT_U32  regAddr;

    PRV_CPSS_PX_A1_DEV_CHECK_MAC(devNum);
    CPSS_PARAM_CHECK_MAX_MAC(packetType, BIT_5);
    CPSS_PARAM_CHECK_MAX_MAC(lagTableNumber, BIT_1);

    /* PCP Packet Type LAG table number */
    regAddr = PRV_PX_REG1_UNIT_PCP_HASH_MAC(devNum).PCPPacketTypeLagTableNumber;

    /* Packet Type LAG Table Number */
    return prvCpssHwPpSetRegField(CAST_SW_DEVNUM(devNum), regAddr, packetType, 1, 
                                  lagTableNumber);
}

/**
* @internal cpssPxIngressHashPacketTypeLagTableNumberSet function
* @endinternal
*
* @brief   Sets LAG table number for specific packet type
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] packetType               - index of the packet type key in the table.
*                                      (APPLICABLE RANGES: 0..31)
* @param[in] lagTableNumber           - the LAG table number.
*                                      (APPLICABLE RANGES: 0..1)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Applicable starting from revision A1
*
*/
GT_STATUS cpssPxIngressHashPacketTypeLagTableNumberSet
(
    IN GT_SW_DEV_NUM                                devNum,
    IN CPSS_PX_PACKET_TYPE                          packetType,
    IN GT_U32                                       lagTableNumber
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxIngressHashPacketTypeLagTableNumberSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, packetType, lagTableNumber));

    rc = internal_cpssPxIngressHashPacketTypeLagTableNumberSet(devNum, packetType, lagTableNumber);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, packetType, lagTableNumber));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxIngressHashPacketTypeLagTableNumberGet function
* @endinternal
*
* @brief   Gets LAG table number for specific packet type
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] packetType               - index of the packet type key in the table.
*                                      (APPLICABLE RANGES: 0..31)
*
* @param[out] lagTableNumberPtr        - (pointer to) the LAG table number.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxIngressHashPacketTypeLagTableNumberGet
(
    IN GT_SW_DEV_NUM                                devNum,
    IN CPSS_PX_PACKET_TYPE                          packetType,
    OUT GT_U32                                     *lagTableNumberPtr
)
{
    GT_U32  regAddr;
    GT_U32  fieldVal;
    GT_STATUS   rc;

    PRV_CPSS_PX_A1_DEV_CHECK_MAC(devNum);
    CPSS_PARAM_CHECK_MAX_MAC(packetType, BIT_5);
    
    /*  PCP Packet Type LAG table number */
    regAddr = PRV_PX_REG1_UNIT_PCP_HASH_MAC(devNum).PCPPacketTypeLagTableNumber;

    /* Packet Type LAG Table Number */
    rc = prvCpssHwPpGetRegField(CAST_SW_DEVNUM(devNum), regAddr, packetType, 1, 
                                &fieldVal);
    if (rc != GT_OK)
    {
        return rc;
    }

    *lagTableNumberPtr = fieldVal;

    return GT_OK;
}

/**
* @internal cpssPxIngressHashPacketTypeLagTableNumberGet function
* @endinternal
*
* @brief   Gets LAG table number for specific packet type
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] packetType               - index of the packet type key in the table.
*                                      (APPLICABLE RANGES: 0..31)
*
* @param[out] lagTableNumberPtr        - (pointer to) the LAG table number.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxIngressHashPacketTypeLagTableNumberGet
(
    IN GT_SW_DEV_NUM                                devNum,
    IN CPSS_PX_PACKET_TYPE                          packetType,
    OUT GT_U32                                     *lagTableNumberPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxIngressHashPacketTypeLagTableNumberGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, packetType, lagTableNumberPtr));

    rc = internal_cpssPxIngressHashPacketTypeLagTableNumberGet(devNum, packetType, lagTableNumberPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, packetType, lagTableNumberPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


