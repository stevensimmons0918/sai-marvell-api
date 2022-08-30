/*******************************************************************************
*              (c), Copyright 2017, Marvell International Ltd.                 *
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
* @file cpssPxPortTxTailDrop.c
*
* @brief CPSS implementation for configuring Tail Drop of the Physical Port Tx
* Traffic Class Queues.
*
* @version   1
********************************************************************************
*/
#define CPSS_LOG_IN_MODULE_ENABLE

#include <cpss/px/cpssHwInit/private/prvCpssPxRegsVer1.h>
#include <cpss/px/config/private/prvCpssPxInfo.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>
#include <cpss/px/port/cpssPxPortTxTailDrop.h>
#include <cpss/px/cos/private/prvCpssPxCoS.h>
#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/px/port/private/prvCpssPxPortLog.h>

/* Check the Tail Drop DBA DP Queue Alpha enum value */
#define PRV_CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_CHECK_MAC(alpha)              \
    switch (alpha)                                                            \
    {                                                                         \
        case CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_0_E:                         \
        case CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_0_25_E:                      \
        case CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_0_5_E:                       \
        case CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_1_E:                         \
        case CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_2_E:                         \
        case CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_4_E:                         \
            break;                                                            \
        default:                                                              \
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);    \
    }

/* Convert CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_ENT to HW value */
#define PRV_CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_TO_HW_CONVERT_MAC(alpha, hwValue) \
    switch (alpha)                                                            \
    {                                                                         \
        case CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_0_E:                         \
            hwValue = 0;                                                      \
            break;                                                            \
        case CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_0_25_E:                      \
            hwValue = 1;                                                      \
            break;                                                            \
        case CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_0_5_E:                       \
            hwValue = 2;                                                      \
            break;                                                            \
        case CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_1_E:                         \
            hwValue = 3;                                                      \
            break;                                                            \
        case CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_2_E:                         \
            hwValue = 4;                                                      \
            break;                                                            \
        case CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_4_E:                         \
            hwValue = 5;                                                      \
            break;                                                            \
        default:                                                              \
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);    \
    }

/* Convert HW value to CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_ENT */
#define PRV_CPSS_PX_PORT_TX_TAIL_DROP_DBA_HW_TO_ALPHA_CONVERT_MAC(hwValue, alpha) \
    switch (hwValue)                                                          \
    {                                                                         \
        case 0:                                                               \
            alpha = CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_0_E;                  \
            break;                                                            \
        case 1:                                                               \
            alpha = CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_0_25_E;               \
            break;                                                            \
        case 2:                                                               \
            alpha = CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_0_5_E;                \
            break;                                                            \
        case 3:                                                               \
            alpha = CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_1_E;                  \
            break;                                                            \
        case 4:                                                               \
            alpha = CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_2_E;                  \
            break;                                                            \
        case 5:                                                               \
            alpha = CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_4_E;                  \
            break;                                                            \
        default:                                                              \
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);         \
    }

/* Check the Tail Drop Shared Pool Number range */
#define PRV_CPSS_PX_PORT_TX_TAIL_DROP_POOL_NUM_CHECK_MAC(poolNum)       \
    if ((poolNum) >= PRV_CPSS_PX_SHARED_POOL_NUM_CNS)                   \
    {                                                                   \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);  \
    }

/* Macro to get the bit mask which is set */
#define PRV_CPSS_PX_PORT_TX_TAIL_DROP_BIT_MASK_GET_MAC(_msbNum, _mask)  \
    _msbNum = 0;                                                        \
    while (_mask != 0)                                                  \
    {                                                                   \
        _msbNum++;                                                      \
        _mask = _mask >> 1;                                             \
    }


/**
* @internal prvCpssPxPortTxTailDropProfileTcParamCheck function
* @endinternal
*
* @brief   Check tail drop profiles limits for particular TC.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] paramsPtr                - the Drop Profile Parameters to associate with the
*                                      Traffic Class in this Profile set.
*
* @retval GT_OK                    - on success
* @retval GT_OUT_OF_RANGE          - on out of range value
*/
static GT_STATUS prvCpssPxPortTxTailDropProfileTcParamCheck
(
    IN  CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_TC_STC    *paramsPtr
)
{
    if ((paramsPtr->dp0MaxDescNum >= BIT_16) ||
        (paramsPtr->dp0MaxBuffNum >= BIT_20) ||
        (paramsPtr->dp1MaxDescNum >= BIT_16) ||
        (paramsPtr->dp1MaxBuffNum >= BIT_20) ||
        (paramsPtr->dp2MaxDescNum >= BIT_16) ||
        (paramsPtr->dp2MaxBuffNum >= BIT_20) ||
        (paramsPtr->tcMaxBuffNum >= BIT_20) ||
        (paramsPtr->tcMaxDescNum >= BIT_16))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    PRV_CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_CHECK_MAC(paramsPtr->dp0QueueAlpha);
    PRV_CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_CHECK_MAC(paramsPtr->dp1QueueAlpha);
    PRV_CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_CHECK_MAC(paramsPtr->dp2QueueAlpha);

    return GT_OK;
}

/**
* @internal prvCpssPxPortTxTailDropProfileTcWriteTableEntry function
* @endinternal
*
* @brief   Write a whole entry to the table.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] tableType                - the specific table name
*                                      entryIndex    - index in the table
* @param[in] countOfField             - count of fields that will be set.
*                                      ARG[3i + 0]  - (GT_U32) offset of field[i]
*                                      ARG[3i + 1]  - (GT_U32) length of field[i]
*                                      ARG[3i + 2]  - (GT_U32) value of field[i]
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS prvCpssPxPortTxTailDropProfileTcWriteTableEntry
(
    IN  GT_SW_DEV_NUM       devNum,
    IN  GT_U32              tableIndex,
    IN  CPSS_PX_TABLE_ENT   tableType,
    IN  GT_U32              countOfField,
    IN  ...
)
{
    va_list ap;             /* argument list pointer */

    GT_U32  hwArray[2];     /* HW table */
    GT_U32  fieldOffset;    /* field offset */
    GT_U32  fieldLength;    /* field length */
    GT_U32  fieldValue;     /* field value */
    GT_U32  i;              /* iterator */

    va_start(ap, countOfField);

    /* Clear HW table data */
    hwArray[0] = 0;
    hwArray[1] = 0;

    for (i = 0; i < countOfField; i++)
    {
        fieldOffset = va_arg(ap, GT_U32);
        fieldLength = va_arg(ap, GT_U32);
        fieldValue  = va_arg(ap, GT_U32);

        U32_SET_FIELD_IN_ENTRY_MAC(hwArray, fieldOffset, fieldLength, fieldValue);
    }

    va_end(ap);

    return prvCpssPxWriteTableEntry(devNum, tableType, tableIndex, hwArray);
}

/**
* @internal prvCpssPxPortTxTailDropProfileTcReadTableEntry function
* @endinternal
*
* @brief   Write a whole entry to the table.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] tableType                - the specific table name
*                                      entryIndex    - index in the table
* @param[in] countOfField             - count of fields that will be set.
*                                      ARG[3i + 0]  - (GT_U32) offset of field[i]
*                                      ARG[3i + 1]  - (GT_U32) length of field[i]
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS prvCpssPxPortTxTailDropProfileTcReadTableEntry
(
    IN  GT_SW_DEV_NUM       devNum,
    IN  GT_U32              tableIndex,
    IN  CPSS_PX_TABLE_ENT   tableType,
    IN  GT_U32              countOfField,
    INOUT   ...
)
{
    va_list     ap;             /* argument list pointer */

    GT_STATUS   rc;             /* return code */
    GT_U32      hwArray[2];     /* HW table */
    GT_U32      fieldOffset;    /* field offset */
    GT_U32      fieldLength;    /* field length */
    GT_U32_PTR  fieldValue;     /* field value */
    GT_U32      i;              /* iterator */

    va_start(ap, countOfField);

    /* Read HW table data */
    rc = prvCpssPxReadTableEntry(devNum, tableType, tableIndex, hwArray);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* Read fields */
    for (i = 0; i < countOfField; i++)
    {
        fieldOffset = va_arg(ap, GT_U32);
        fieldLength = va_arg(ap, GT_U32);
        fieldValue  = va_arg(ap, GT_U32_PTR);

        U32_GET_FIELD_IN_ENTRY_MAC(hwArray, fieldOffset, fieldLength, (*fieldValue));
    }

    va_end(ap);

    return GT_OK;
}

/**
* @internal internal_cpssPxPortTxTailDropProfileIdSet function
* @endinternal
*
* @brief   Bind a Physical Port to a specific Drop Profile Set.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] profileSet               - the Profile Set in which the Traffic Class Drop Parameters
*                                      is associated
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
*/
static GT_STATUS internal_cpssPxPortTxTailDropProfileIdSet
(
    IN  GT_SW_DEV_NUM                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                        portNum,
    IN  CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_SET_ENT   profileSet
)
{
    GT_U32      txqPortNum; /* TxQ port number */
    GT_U32      regAddr;    /* register address */
    GT_U32      regValue;   /* register value */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_SET_CHECK_MAC(profileSet);
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_TXQ_PORT_GET_MAC(devNum, portNum, txqPortNum);

    /* <Port<n> Tail Drop/CN Profile> register */
    regAddr = PRV_PX_REG1_UNIT_TXQ_Q_MAC(devNum).tailDrop.tailDropConfig.
              portTailDropCNProfile[txqPortNum];

    regValue = (GT_U32) profileSet;

    return prvCpssHwPpSetRegField(CAST_SW_DEVNUM(devNum), regAddr, 0, 4, regValue);
}

/**
* @internal cpssPxPortTxTailDropProfileIdSet function
* @endinternal
*
* @brief   Bind a Physical Port to a specific Drop Profile Set.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] profileSet               - the Profile Set in which the Traffic Class Drop Parameters
*                                      is associated
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS cpssPxPortTxTailDropProfileIdSet
(
    IN  GT_SW_DEV_NUM                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                        portNum,
    IN  CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_SET_ENT   profileSet
)
{
    GT_STATUS rc;

    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortTxTailDropProfileIdSet);
    CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, profileSet));

    rc = internal_cpssPxPortTxTailDropProfileIdSet(devNum, portNum, profileSet);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, profileSet));
    CPSS_API_UNLOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortTxTailDropProfileIdGet function
* @endinternal
*
* @brief   Get Drop Profile Set according to a Physical Port.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
*
* @param[out] profileSetPtr            - pointer to the Profile Set in which the Traffic Class
*                                      Drop Parameters is associated
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPortTxTailDropProfileIdGet
(
    IN  GT_SW_DEV_NUM                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                        portNum,
    OUT CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_SET_ENT   *profileSetPtr
)
{
    GT_STATUS   rc;         /* return code */
    GT_U32      txqPortNum; /* TxQ port number */
    GT_U32      regAddr;    /* register address */
    GT_U32      regValue;   /* register value */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_TXQ_PORT_GET_MAC(devNum, portNum, txqPortNum);
    CPSS_NULL_PTR_CHECK_MAC(profileSetPtr);

    /* <Port<n> Tail Drop/CN Profile> register */
    regAddr = PRV_PX_REG1_UNIT_TXQ_Q_MAC(devNum).tailDrop.tailDropConfig.
              portTailDropCNProfile[txqPortNum];

    rc = prvCpssHwPpGetRegField(CAST_SW_DEVNUM(devNum), regAddr, 0, 4, &regValue);
    if (rc != GT_OK)
    {
        return rc;
    }

    *profileSetPtr = (CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_SET_ENT) regValue;

    return GT_OK;
}

/**
* @internal cpssPxPortTxTailDropProfileIdGet function
* @endinternal
*
* @brief   Get Drop Profile Set according to a Physical Port.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
*
* @param[out] profileSetPtr            - pointer to the Profile Set in which the Traffic Class
*                                      Drop Parameters is associated
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortTxTailDropProfileIdGet
(
    IN  GT_SW_DEV_NUM                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                        portNum,
    OUT CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_SET_ENT   *profileSetPtr
)
{
    GT_STATUS rc;

    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortTxTailDropProfileIdGet);
    CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, profileSetPtr));

    rc = internal_cpssPxPortTxTailDropProfileIdGet(devNum, portNum, profileSetPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, profileSetPtr));
    CPSS_API_UNLOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortTxTailDropUcEnableSet function
* @endinternal
*
* @brief   Enable/Disable tail-dropping for all packets based on the profile limits.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   -  device number.
* @param[in] enable                   -  GT_TRUE  - Tail Drop is enabled.
*                                      GT_FALSE - The Tail Drop limits for all packets are
*                                      ignored and packet is dropped only
*                                      when the Tx Queue has reached its global
*                                      descriptors limit.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPortTxTailDropUcEnableSet
(
    IN  GT_SW_DEV_NUM   devNum,
    IN  GT_BOOL         enable
)
{
    GT_U32      regAddr;    /* register address */
    GT_U32      regValue;   /* register value */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);

    regValue = BOOL2BIT_MAC(enable);

    /* <Global Tail Drop Configuration> register */
    regAddr = PRV_PX_REG1_UNIT_TXQ_Q_MAC(devNum).tailDrop.tailDropConfig.
              globalTailDropConfig;

    /* Set <Tail Drop Enable> field of <Global Tail Drop Configuration> register */
    return prvCpssHwPpSetRegField(CAST_SW_DEVNUM(devNum), regAddr, 7, 1, regValue);
}

/**
* @internal cpssPxPortTxTailDropUcEnableSet function
* @endinternal
*
* @brief   Enable/Disable tail-dropping for all packets based on the profile limits.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   -  device number.
* @param[in] enable                   -  GT_TRUE  - Tail Drop is enabled.
*                                      GT_FALSE - The Tail Drop limits for all packets are
*                                      ignored and packet is dropped only
*                                      when the Tx Queue has reached its global
*                                      descriptors limit.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortTxTailDropUcEnableSet
(
    IN  GT_SW_DEV_NUM   devNum,
    IN  GT_BOOL         enable
)
{
    GT_STATUS rc;

    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortTxTailDropUcEnableSet);
    CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, enable));

    rc = internal_cpssPxPortTxTailDropUcEnableSet(devNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, enable));
    CPSS_API_UNLOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortTxTailDropUcEnableGet function
* @endinternal
*
* @brief   Get enable/disable tail-dropping for all packets based on the profile
*         limits.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
*
* @param[out] enablePtr                - pointer to tail drop status:
*                                      GT_TRUE  - Tail Drop is enabled.
*                                      GT_FALSE - The Tail Drop limits for all packets are ignored
*                                      and packet is dropped only when the Tx Queue has
*                                      reached its global descriptors limit.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPortTxTailDropUcEnableGet
(
    IN  GT_SW_DEV_NUM   devNum,
    OUT GT_BOOL         *enablePtr
)
{
    GT_STATUS   rc;         /* return code */
    GT_U32      regAddr;    /* register address */
    GT_U32      regValue;   /* register value */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    /* <Global Tail Drop Configuration> register */
    regAddr = PRV_PX_REG1_UNIT_TXQ_Q_MAC(devNum).tailDrop.tailDropConfig.
              globalTailDropConfig;

    /* Get <Tail Drop Enable> field of <Global Tail Drop Configuration> register */
    rc = prvCpssHwPpGetRegField(CAST_SW_DEVNUM(devNum), regAddr, 7, 1, &regValue);
    if (rc != GT_OK)
    {
        return rc;
    }

    *enablePtr = BIT2BOOL_MAC(regValue);

    return GT_OK;
}

/**
* @internal cpssPxPortTxTailDropUcEnableGet function
* @endinternal
*
* @brief   Get enable/disable tail-dropping for all packets based on the profile
*         limits.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
*
* @param[out] enablePtr                - pointer to tail drop status:
*                                      GT_TRUE  - Tail Drop is enabled.
*                                      GT_FALSE - The Tail Drop limits for all packets are ignored
*                                      and packet is dropped only when the Tx Queue has
*                                      reached its global descriptors limit.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortTxTailDropUcEnableGet
(
    IN  GT_SW_DEV_NUM   devNum,
    OUT GT_BOOL         *enablePtr
)
{
    GT_STATUS rc;

    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortTxTailDropUcEnableGet);
    CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, enablePtr));

    rc = internal_cpssPxPortTxTailDropUcEnableGet(devNum, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortTxTailDropProfileSet function
* @endinternal
*
* @brief   Set maximal port's limits of buffers and descriptors.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] profileSet               - the Profile Set in which the Traffic Class Drop
*                                      Parameters is associated
* @param[in] portMaxBuffLimit         - maximal number of buffers for a port.
*                                      (APPLICABLE RANGES: 0..0x0FFFFF).
* @param[in] portMaxDescLimit         - maximal number of descriptors for a port.
*                                      (APPLICABLE RANGES: 0..0xFFFF).
* @param[in] portAlpha                - ratio of the free buffers used for the port
*                                      thresholds.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number, profile set or
*                                       portAlpha
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - on out of range value
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPortTxTailDropProfileSet
(
    IN  GT_SW_DEV_NUM                               devNum,
    IN  CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_SET_ENT   profileSet,
    IN  GT_U32                                      portMaxBuffLimit,
    IN  GT_U32                                      portMaxDescLimit,
    IN  CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_ENT     portAlpha
)
{
    GT_STATUS   rc;             /* return code */
    GT_U32      regAddr;        /* register address */
    GT_U32      portAlphaNum;   /* port alpha */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_SET_CHECK_MAC(profileSet);
    PRV_CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_TO_HW_CONVERT_MAC(portAlpha, portAlphaNum);

    if ((portMaxBuffLimit >= BIT_20) || (portMaxDescLimit >= BIT_16))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    /* <Profile<n> Port Buffer Limits> register */
    regAddr = PRV_PX_REG1_UNIT_TXQ_Q_MAC(devNum).tailDrop.tailDropLimits.
              profilePortBufferLimits[profileSet];

    /* Set <Port Buffer Limit Profile <n>> field of  */
    /* <Profile<n> Port Buffer Limits> register      */
    rc = prvCpssHwPpSetRegField(CAST_SW_DEVNUM(devNum), regAddr, 0, 20,
                                portMaxBuffLimit);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* <Profile<n> Port Descriptor Limits> register */
    regAddr = PRV_PX_REG1_UNIT_TXQ_Q_MAC(devNum).tailDrop.tailDropLimits.
              profilePortDescLimits[profileSet];

    /* Set <PortDescLimit Profile <n>> field of     */
    /* <Profile<n> Port Descriptor Limits> register */
    rc = prvCpssHwPpSetRegField(CAST_SW_DEVNUM(devNum), regAddr, 0, 16,
                                portMaxDescLimit);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* <Profile<n> Port Buffer Limits> register */
    regAddr = PRV_PX_REG1_UNIT_TXQ_Q_MAC(devNum).tailDrop.tailDropLimits.
              profilePortBufferLimits[profileSet];

    /* Set <Port Alpha Profile <N>> field of     */
    /* <Profile<n> Port Buffer Limits> register  */
    return prvCpssHwPpSetRegField(CAST_SW_DEVNUM(devNum), regAddr, 20, 3,
                                  portAlphaNum);
}

/**
* @internal cpssPxPortTxTailDropProfileSet function
* @endinternal
*
* @brief   Set maximal port's limits of buffers and descriptors.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] profileSet               - the Profile Set in which the Traffic Class Drop
*                                      Parameters is associated
* @param[in] portMaxBuffLimit         - maximal number of buffers for a port.
*                                      (APPLICABLE RANGES: 0..0x0FFFFF).
* @param[in] portMaxDescLimit         - maximal number of descriptors for a port.
*                                      (APPLICABLE RANGES: 0..0xFFFF).
* @param[in] portAlpha                - ratio of the free buffers used for the port
*                                      thresholds.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number, profile set or
*                                       portAlpha
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - on out of range value
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortTxTailDropProfileSet
(
    IN  GT_SW_DEV_NUM                               devNum,
    IN  CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_SET_ENT   profileSet,
    IN  GT_U32                                      portMaxBuffLimit,
    IN  GT_U32                                      portMaxDescLimit,
    IN  CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_ENT     portAlpha
)
{
    GT_STATUS rc;

    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortTxTailDropProfileSet);
    CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, profileSet, portMaxBuffLimit, portMaxDescLimit, portAlpha));

    rc = internal_cpssPxPortTxTailDropProfileSet(devNum, profileSet,
            portMaxBuffLimit, portMaxDescLimit, portAlpha);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, profileSet, portMaxBuffLimit, portMaxDescLimit, portAlpha));
    CPSS_API_UNLOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortTxTailDropProfileGet function
* @endinternal
*
* @brief   Get maximal port's limits of buffers and descriptors.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] profileSet               - the Profile Set in which the Traffic Class Drop Parameters
*                                      is associated.
*
* @param[out] portMaxBuffLimitPtr      - pointer to maximal number of buffers for a port.
* @param[out] portMaxDescLimitPtr      - pointer to maximal number of descriptors for a
*                                      port.
* @param[out] portAlphaPtr             - pointer to the ratio of the free buffers used for
*                                      the port thresholds.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number or profile set
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPortTxTailDropProfileGet
(
    IN  GT_SW_DEV_NUM                               devNum,
    IN  CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_SET_ENT   profileSet,
    OUT GT_U32                                      *portMaxBuffLimitPtr,
    OUT GT_U32                                      *portMaxDescLimitPtr,
    OUT CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_ENT     *portAlphaPtr
)
{
    GT_STATUS   rc;         /* return code */
    GT_U32      regAddr;    /* register address */
    GT_U32      regValue;   /* register value */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_SET_CHECK_MAC(profileSet);
    CPSS_NULL_PTR_CHECK_MAC(portMaxBuffLimitPtr);
    CPSS_NULL_PTR_CHECK_MAC(portMaxDescLimitPtr);
    CPSS_NULL_PTR_CHECK_MAC(portAlphaPtr);

    /* <Profile<n> Port Buffer Limits> register */
    regAddr = PRV_PX_REG1_UNIT_TXQ_Q_MAC(devNum).tailDrop.tailDropLimits.
              profilePortBufferLimits[profileSet];

    /* Get <Port Buffer Limit Profile <n>> field of  */
    /* <Profile<n> Port Buffer Limits> register      */
    rc = prvCpssHwPpGetRegField(CAST_SW_DEVNUM(devNum), regAddr, 0, 20, &regValue);
    if (rc != GT_OK)
    {
        return rc;
    }

    *portMaxBuffLimitPtr = regValue;

    /* <Profile<n> Port Descriptor Limits> register */
    regAddr = PRV_PX_REG1_UNIT_TXQ_Q_MAC(devNum).tailDrop.tailDropLimits.
              profilePortDescLimits[profileSet];

    /* Get <PortDescLimit Profile <n>> field of     */
    /* <Profile<n> Port Descriptor Limits> register */
    rc = prvCpssHwPpGetRegField(CAST_SW_DEVNUM(devNum), regAddr, 0, 16, &regValue);
    if (rc != GT_OK)
    {
        return rc;
    }

    *portMaxDescLimitPtr = regValue;

    /* <Profile<n> Port Buffer Limits> register */
    regAddr = PRV_PX_REG1_UNIT_TXQ_Q_MAC(devNum).tailDrop.tailDropLimits.
              profilePortBufferLimits[profileSet];

    /* Get <Port Alpha Profile <N>> field of     */
    /* <Profile<n> Port Buffer Limits> register  */
    rc = prvCpssHwPpGetRegField(CAST_SW_DEVNUM(devNum), regAddr, 20, 3, &regValue);
    if (rc != GT_OK)
    {
        return rc;
    }

    PRV_CPSS_PX_PORT_TX_TAIL_DROP_DBA_HW_TO_ALPHA_CONVERT_MAC(regValue, *portAlphaPtr);

    return GT_OK;
}

/**
* @internal cpssPxPortTxTailDropProfileGet function
* @endinternal
*
* @brief   Get maximal port's limits of buffers and descriptors.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] profileSet               - the Profile Set in which the Traffic Class Drop Parameters
*                                      is associated.
*
* @param[out] portMaxBuffLimitPtr      - pointer to maximal number of buffers for a port.
* @param[out] portMaxDescLimitPtr      - pointer to maximal number of descriptors for a
*                                      port.
* @param[out] portAlphaPtr             - pointer to the ratio of the free buffers used for
*                                      the port thresholds.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number or profile set
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortTxTailDropProfileGet
(
    IN  GT_SW_DEV_NUM                               devNum,
    IN  CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_SET_ENT   profileSet,
    OUT GT_U32                                      *portMaxBuffLimitPtr,
    OUT GT_U32                                      *portMaxDescLimitPtr,
    OUT CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_ENT     *portAlphaPtr
)
{
    GT_STATUS rc;

    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortTxTailDropProfileGet);
    CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, profileSet, portMaxBuffLimitPtr, portMaxDescLimitPtr, portAlphaPtr));

    rc = internal_cpssPxPortTxTailDropProfileGet(devNum, profileSet,
            portMaxBuffLimitPtr, portMaxDescLimitPtr, portAlphaPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, profileSet, portMaxBuffLimitPtr, portMaxDescLimitPtr, portAlphaPtr));
    CPSS_API_UNLOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortTxTailDropPacketModeLengthSet function
* @endinternal
*
* @brief   Configure the packet length used for Tail Drop in packet mode.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] length                   - Defines the number of buffers consumed by a packet for
*                                      Tail Drop.
*                                      (APPLICABLE RANGES: 0..63).
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - on length out of range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The function should be called only when traffic disabled
*       Mode configuration for Pipe performed per profile.
*
*/
static GT_STATUS internal_cpssPxPortTxTailDropPacketModeLengthSet
(
    IN  GT_SW_DEV_NUM   devNum,
    IN  GT_U32          length
)
{
    GT_STATUS   rc;         /* return code */
    GT_U32      regAddr;    /* register address */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);

    if (length >= BIT_6)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    /*
        Enqueue packet lenght
    */
    /* <Packet Length for Tail Drop - Enqueue> register  */
    regAddr = PRV_PX_REG1_UNIT_TXQ_Q_MAC(devNum).tailDrop.tailDropConfig.
              pktLengthForTailDropEnqueue;

    /* Set <EnqConstPacketLength> field of               */
    /* <Packet Length for Tail Drop - Enqueue> register  */
    rc = prvCpssHwPpSetRegField(CAST_SW_DEVNUM(devNum), regAddr, 1, 6, length);
    if (rc != GT_OK)
    {
        return rc;
    }

    /*
        Dequeue packet lenght
    */
    /* <Packet Length for Tail Drop - Dequeue> register  */
    regAddr = PRV_PX_REG1_UNIT_TXQ_DQ_MAC(devNum).scheduler.schedulerConfig.
              pktLengthForTailDropDequeue;

    /* Set <DqConstPacketLength> field of               */
    /* <Packet Length for Tail Drop - Dequeue> register  */
    return prvCpssHwPpSetRegField(CAST_SW_DEVNUM(devNum), regAddr, 1, 6, length);
}

/**
* @internal cpssPxPortTxTailDropPacketModeLengthSet function
* @endinternal
*
* @brief   Configure the packet length used for Tail Drop in packet mode.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
*                                      lenght  - Defines the number of buffers consumed by a packet for
*                                      Tail Drop.
*                                      (APPLICABLE RANGES: 0..63).
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - on length out of range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The function should be called only when traffic disabled
*       Mode configuration for Pipe performed per profile.
*
*/
GT_STATUS cpssPxPortTxTailDropPacketModeLengthSet
(
    IN  GT_SW_DEV_NUM   devNum,
    IN  GT_U32          length
)
{
    GT_STATUS rc;

    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortTxTailDropPacketModeLengthSet);
    CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, length));

    rc = internal_cpssPxPortTxTailDropPacketModeLengthSet(devNum, length);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, length));
    CPSS_API_UNLOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortTxTailDropPacketModeLengthGet function
* @endinternal
*
* @brief   Get the packet length used for Tail Drop in packet mode.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
*
* @param[out] lengthPtr                - (pointer to) the number of buffers consumed by a packet for
*                                      Tail Drop.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPortTxTailDropPacketModeLengthGet
(
    IN  GT_SW_DEV_NUM   devNum,
    OUT GT_U32          *lengthPtr
)
{
    GT_STATUS   rc;         /* return code */
    GT_U32      regAddr;    /* register address */
    GT_U32      regValue;   /* register value */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(lengthPtr);

    /* <Packet Length for Tail Drop - Enqueue> register  */
    regAddr = PRV_PX_REG1_UNIT_TXQ_Q_MAC(devNum).tailDrop.tailDropConfig.
              pktLengthForTailDropEnqueue;

    /* Get <EnqConstPacketLength> field of               */
    /* <Packet Length for Tail Drop - Enqueue> register  */
    rc = prvCpssHwPpGetRegField(CAST_SW_DEVNUM(devNum), regAddr, 1, 6, &regValue);
    if (rc != GT_OK)
    {
        return rc;
    }

    *lengthPtr = regValue;

    return GT_OK;
}

/**
* @internal cpssPxPortTxTailDropPacketModeLengthGet function
* @endinternal
*
* @brief   Get the packet length used for Tail Drop in packet mode.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
*
* @param[out] lengthPtr                - (pointer to) the number of buffers consumed by a packet for
*                                      Tail Drop.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortTxTailDropPacketModeLengthGet
(
    IN  GT_SW_DEV_NUM   devNum,
    OUT GT_U32          *lengthPtr
)
{
    GT_STATUS rc;

    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortTxTailDropPacketModeLengthGet);
    CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, lengthPtr));

    rc = internal_cpssPxPortTxTailDropPacketModeLengthGet(devNum, lengthPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, lengthPtr));
    CPSS_API_UNLOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortTxTailDropProfileBufferConsumptionModeSet function
* @endinternal
*
* @brief   Sets the packet buffer consumption mode per Tail Drop profile.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] profile                  - tail drop profile
* @param[in] mode                     - buffer consumption mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device, profile or wrong mode
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPortTxTailDropProfileBufferConsumptionModeSet
(
    IN  GT_SW_DEV_NUM                                           devNum,
    IN  CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_SET_ENT               profile,
    IN  CPSS_PX_PORT_TX_TAIL_DROP_BUFFER_CONSUMPTION_MODE_ENT   mode
)
{
    GT_STATUS   rc;         /* return code */
    GT_U32      regAddr;    /* register address */
    GT_U32      regValue;   /* register value */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_SET_CHECK_MAC(profile);

    switch (mode)
    {
        case CPSS_PX_PORT_TX_TAIL_DROP_BUFFER_CONSUMPTION_MODE_BYTE_E:
            regValue = 0;
            break;
        case CPSS_PX_PORT_TX_TAIL_DROP_BUFFER_CONSUMPTION_MODE_PACKET_E:
            regValue = 1;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /*
        Dequeue packet lenght mode profile
    */
    /* <Profile <p> Byte Count Modification> register */
    regAddr = PRV_PX_REG1_UNIT_TXQ_DQ_MAC(devNum).global.globalDQConfig.
              profileByteCountModification[profile];

    /* Set <DqPacketLengthModeProfile<p>> field of     */
    /* <Profile <p> Byte Count Modification> register  */
    rc = prvCpssHwPpSetRegField(CAST_SW_DEVNUM(devNum), regAddr, 0, 1, regValue);
    if (rc != GT_OK)
    {
        return rc;
    }

    /*
        Enqueue packet lenght mode profile
    */
    /* <Profile <p> Tail Drop Configuration> register */
    regAddr = PRV_PX_REG1_UNIT_TXQ_Q_MAC(devNum).tailDrop.tailDropConfig.
              profileTailDropConfig[profile];

    /* Set <EnqPacketLengthModeProfile<p>> field of    */
    /* <Profile <p> Tail Drop Configuration> register  */
    return prvCpssHwPpSetRegField(CAST_SW_DEVNUM(devNum), regAddr, 1, 1, regValue);
}

/**
* @internal cpssPxPortTxTailDropProfileBufferConsumptionModeSet function
* @endinternal
*
* @brief   Sets the packet buffer consumption mode per Tail Drop profile.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] profile                  - tail drop profile
* @param[in] mode                     - buffer consumption mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device, profile or wrong mode
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortTxTailDropProfileBufferConsumptionModeSet
(
    IN  GT_SW_DEV_NUM                                           devNum,
    IN  CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_SET_ENT               profile,
    IN  CPSS_PX_PORT_TX_TAIL_DROP_BUFFER_CONSUMPTION_MODE_ENT   mode
)
{
    GT_STATUS rc;

    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortTxTailDropProfileBufferConsumptionModeSet);
    CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, profile, mode));

    rc = internal_cpssPxPortTxTailDropProfileBufferConsumptionModeSet(devNum,
            profile, mode);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, profile, mode));
    CPSS_API_UNLOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortTxTailDropProfileBufferConsumptionModeGet function
* @endinternal
*
* @brief   Gets the packet buffer consumption mode per Tail Drop profile.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] profile                  - tail drop profile
*
* @param[out] modePtr                  - (pointer to) buffer consumption mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device, profile
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
*/
static GT_STATUS internal_cpssPxPortTxTailDropProfileBufferConsumptionModeGet
(
    IN  GT_SW_DEV_NUM                                           devNum,
    IN  CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_SET_ENT               profile,
    OUT CPSS_PX_PORT_TX_TAIL_DROP_BUFFER_CONSUMPTION_MODE_ENT   *modePtr
)
{
    GT_STATUS   rc;         /* return code */
    GT_U32      regAddr;    /* register address */
    GT_U32      regValue;   /* register value */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_SET_CHECK_MAC(profile);
    CPSS_NULL_PTR_CHECK_MAC(modePtr);

    /* <Profile <p> Byte Count Modification> register */
    regAddr = PRV_PX_REG1_UNIT_TXQ_DQ_MAC(devNum).global.globalDQConfig.
              profileByteCountModification[profile];

    /* Get <DqPacketLengthModeProfile<p>> field of     */
    /* <Profile <p> Byte Count Modification> register  */
    rc = prvCpssHwPpGetRegField(CAST_SW_DEVNUM(devNum), regAddr, 0, 1, &regValue);
    if (rc != GT_OK)
    {
        return rc;
    }

    *modePtr = (regValue == 0)
               ? CPSS_PX_PORT_TX_TAIL_DROP_BUFFER_CONSUMPTION_MODE_BYTE_E
               : CPSS_PX_PORT_TX_TAIL_DROP_BUFFER_CONSUMPTION_MODE_PACKET_E;

    return GT_OK;
}

/**
* @internal cpssPxPortTxTailDropProfileBufferConsumptionModeGet function
* @endinternal
*
* @brief   Gets the packet buffer consumption mode per Tail Drop profile.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] profile                  - tail drop profile
*
* @param[out] modePtr                  - (pointer to) buffer consumption mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device, profile
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
*/
GT_STATUS cpssPxPortTxTailDropProfileBufferConsumptionModeGet
(
    IN  GT_SW_DEV_NUM                                           devNum,
    IN  CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_SET_ENT               profile,
    OUT CPSS_PX_PORT_TX_TAIL_DROP_BUFFER_CONSUMPTION_MODE_ENT   *modePtr
)
{
    GT_STATUS rc;

    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortTxTailDropProfileBufferConsumptionModeGet);
    CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, profile, modePtr));

    rc = internal_cpssPxPortTxTailDropProfileBufferConsumptionModeGet(devNum,
            profile, modePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, profile, modePtr));
    CPSS_API_UNLOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortTxTailDropMaskSharedBuffEnableSet function
* @endinternal
*
* @brief   Enable/Disable shared buffer indication masking. When enabled, the tail
*         drop mechanism will ignore the buffer fill level indication.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] enable                   - GT_TRUE  - Enable buffer fill level masking.
*                                      GT_FALSE - Disable buffer fill level masking.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPortTxTailDropMaskSharedBuffEnableSet
(
    IN  GT_SW_DEV_NUM   devNum,
    IN  GT_BOOL         enable
)
{
    GT_U32  regAddr;    /* register address */
    GT_U32  regValue;   /* register value */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);

    regValue = BOOL2BIT_MAC(enable);

    /* <Global Tail Drop Configuration> register */
    regAddr = PRV_PX_REG1_UNIT_TXQ_Q_MAC(devNum).tailDrop.tailDropConfig.
              globalTailDropConfig;

    /* Set <Mask Shared Buff Indication From BM> field of  */
    /* <Global Tail Drop Configuration> register           */
    return prvCpssHwPpSetRegField(CAST_SW_DEVNUM(devNum), regAddr, 12, 1, regValue);
}

/**
* @internal cpssPxPortTxTailDropMaskSharedBuffEnableSet function
* @endinternal
*
* @brief   Enable/Disable shared buffer indication masking. When enabled, the tail
*         drop mechanism will ignore the buffer fill level indication.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] enable                   - GT_TRUE  - Enable buffer fill level masking.
*                                      GT_FALSE - Disable buffer fill level masking.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortTxTailDropMaskSharedBuffEnableSet
(
    IN  GT_SW_DEV_NUM   devNum,
    IN  GT_BOOL         enable
)
{
    GT_STATUS rc;

    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortTxTailDropMaskSharedBuffEnableSet);
    CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, enable));

    rc = internal_cpssPxPortTxTailDropMaskSharedBuffEnableSet(devNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, enable));
    CPSS_API_UNLOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortTxTailDropMaskSharedBuffEnableGet function
* @endinternal
*
* @brief   Gets status of shared buffer indication masking. When enabled, the tail
*         drop mechanism will ignore the buffer fill level indication.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                - (pointer to) shared buffer indication masking.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
*/
static GT_STATUS internal_cpssPxPortTxTailDropMaskSharedBuffEnableGet
(
    IN  GT_SW_DEV_NUM   devNum,
    OUT GT_BOOL         *enablePtr
)
{
    GT_STATUS   rc;         /* return code */
    GT_U32      regAddr;    /* register address */
    GT_U32      regValue;   /* register value */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    /* <Global Tail Drop Configuration> register */
    regAddr = PRV_PX_REG1_UNIT_TXQ_Q_MAC(devNum).tailDrop.tailDropConfig.
              globalTailDropConfig;

    /* Get <Mask Shared Buff Indication From BM> field of  */
    /* <Global Tail Drop Configuration> register           */
    rc = prvCpssHwPpGetRegField(CAST_SW_DEVNUM(devNum), regAddr, 12, 1, &regValue);
    if (rc != GT_OK)
    {
        return rc;
    }

    *enablePtr = BIT2BOOL_MAC(regValue);

    return GT_OK;
}

/**
* @internal cpssPxPortTxTailDropMaskSharedBuffEnableGet function
* @endinternal
*
* @brief   Gets status of shared buffer indication masking. When enabled, the tail
*         drop mechanism will ignore the buffer fill level indication.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                - (pointer to) shared buffer indication masking.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
*/
GT_STATUS cpssPxPortTxTailDropMaskSharedBuffEnableGet
(
    IN  GT_SW_DEV_NUM   devNum,
    OUT GT_BOOL         *enablePtr
)
{
    GT_STATUS rc;

    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortTxTailDropMaskSharedBuffEnableGet);
    CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, enablePtr));

    rc = internal_cpssPxPortTxTailDropMaskSharedBuffEnableGet(devNum, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortTxTailDropSharedBuffMaxLimitSet function
* @endinternal
*
* @brief   Set max shared buffer limit value.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] maxSharedBufferLimit     - maximal shared buffer limit.
*                                      (APPLICABLE RANGES: 0..0xFFFF).
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_OUT_OF_RANGE          - on illegal maxSharedBufferLimit
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPortTxTailDropSharedBuffMaxLimitSet
(
    IN  GT_SW_DEV_NUM   devNum,
    IN  GT_U32          maxSharedBufferLimit
)
{
    GT_U32  regAddr;    /* register address */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);

    if (maxSharedBufferLimit > 0xFFFF)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    /* <BM Buffer Limit Configuration 1> register */
    regAddr = PRV_PX_REG1_UNIT_BM_MAC(devNum).BMGlobalConfigs.BMBufferLimitConfig1;

    /* Set <Max Shared Buffer Limit> field of      */
    /* <BM Buffer Limit Configuration 1> register  */
    return prvCpssHwPpSetRegField(CAST_SW_DEVNUM(devNum), regAddr, 16, 16,
                                  maxSharedBufferLimit);
}

/**
* @internal cpssPxPortTxTailDropSharedBuffMaxLimitSet function
* @endinternal
*
* @brief   Set max shared buffer limit value.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] maxSharedBufferLimit     - maximal shared buffer limit.
*                                      (APPLICABLE RANGES: 0..0xFFFF).
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_OUT_OF_RANGE          - on illegal maxSharedBufferLimit
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortTxTailDropSharedBuffMaxLimitSet
(
    IN  GT_SW_DEV_NUM   devNum,
    IN  GT_U32          maxSharedBufferLimit
)
{
    GT_STATUS rc;

    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortTxTailDropSharedBuffMaxLimitSet);
    CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, maxSharedBufferLimit));

    rc = internal_cpssPxPortTxTailDropSharedBuffMaxLimitSet(devNum,
            maxSharedBufferLimit);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, maxSharedBufferLimit));
    CPSS_API_UNLOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortTxTailDropSharedBuffMaxLimitGet function
* @endinternal
*
* @brief   Get max shared buffer limit value.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] maxSharedBufferLimitPtr  - (pointer to) maximal shared buffer limit
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
*/
static GT_STATUS internal_cpssPxPortTxTailDropSharedBuffMaxLimitGet
(
    IN  GT_SW_DEV_NUM   devNum,
    OUT GT_U32          *maxSharedBufferLimitPtr
)
{
    GT_STATUS   rc;         /* return code */
    GT_U32      regAddr;    /* register address */
    GT_U32      regValue;   /* register value */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(maxSharedBufferLimitPtr);

    /* <BM Buffer Limit Configuration 1> register */
    regAddr = PRV_PX_REG1_UNIT_BM_MAC(devNum).BMGlobalConfigs.BMBufferLimitConfig1;

    /* Get <Max Shared Buffer Limit> field of      */
    /* <BM Buffer Limit Configuration 1> register  */
    rc = prvCpssHwPpGetRegField(CAST_SW_DEVNUM(devNum), regAddr, 16, 16, &regValue);
    if (rc != GT_OK)
    {
        return rc;
    }

    *maxSharedBufferLimitPtr = regValue;

    return GT_OK;
}

/**
* @internal cpssPxPortTxTailDropSharedBuffMaxLimitGet function
* @endinternal
*
* @brief   Get max shared buffer limit value.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] maxSharedBufferLimitPtr  - (pointer to) maximal shared buffer limit
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
*/
GT_STATUS cpssPxPortTxTailDropSharedBuffMaxLimitGet
(
    IN  GT_SW_DEV_NUM   devNum,
    OUT GT_U32          *maxSharedBufferLimitPtr
)
{
    GT_STATUS rc;

    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortTxTailDropSharedBuffMaxLimitGet);
    CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, maxSharedBufferLimitPtr));

    rc = internal_cpssPxPortTxTailDropSharedBuffMaxLimitGet(devNum,
            maxSharedBufferLimitPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, maxSharedBufferLimitPtr));
    CPSS_API_UNLOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortTxTailDropProfileWeightedRandomTailDropEnableSet function
* @endinternal
*
* @brief   Enable/disable Weighted Random Tail Drop Threshold to overcome
*         synchronization.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] profile                  - the Profile Set in which the Traffic Class Drop Parameters
*                                      is associated.
* @param[in] dp                       - Drop Precedence.
*                                      (APPLICABLE RANGES: 0..2).
* @param[in] tc                       - the Traffic Class.
*                                      (APPLICABLE RANGES: 0..7).
* @param[in] enablerPtr               - (pointer to) Tail Drop limits enabling for Weigthed Random
*                                      Tail Drop
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on invalid input paramters value
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPortTxTailDropProfileWeightedRandomTailDropEnableSet
(
    IN  GT_SW_DEV_NUM                               devNum,
    IN  CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_SET_ENT   profile,
    IN  GT_U32                                      dp,
    IN  GT_U32                                      tc,
    IN  CPSS_PX_PORT_TX_TAIL_DROP_WRTD_ENABLER_STC  *enablerPtr
)
{
    GT_STATUS   rc;         /* return code */
    GT_U32      regAddr;    /* register address */
    GT_U32      regValue;   /* register value */
    GT_U32      offset;     /* bit offset inside register */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_SET_CHECK_MAC(profile);
    PRV_CPSS_PX_COS_CHECK_TC_MAC(tc);
    CPSS_NULL_PTR_CHECK_MAC(enablerPtr);

    if (dp > 2)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    offset = (dp << 3) + tc;


    /* <Profile <p> Shared Pool WRTD En> register */
    regAddr = PRV_PX_REG1_UNIT_TXQ_Q_MAC(devNum).tailDrop.tailDropConfig.
              profilePoolWRTDEn[profile];

    regValue = BOOL2BIT_MAC(enablerPtr->sharedPoolLimit);

    /* Set <Profile<p>Dp<X>Tc<X>SharedPoolWRTDEn> field of  */
    /* <Profile <p> Shared Pool WRTD En> register           */
    rc = prvCpssHwPpSetRegField(CAST_SW_DEVNUM(devNum), regAddr, offset, 1,
                                regValue);
    if (GT_OK != rc)
    {
        return rc;
    }


    /* <Profile <p> Max Queue WRTD En> register */
    regAddr = PRV_PX_REG1_UNIT_TXQ_Q_MAC(devNum).tailDrop.tailDropConfig.
              profileMaxQueueWRTDEn[profile];

    regValue = BOOL2BIT_MAC(enablerPtr->tcLimit);

    /* Set <Profile<p>DP<X>TC<X>MaxQueueWRTDEn> field of  */
    /* <Profile <p> Max Queue WRTD En> register           */
    rc = prvCpssHwPpSetRegField(CAST_SW_DEVNUM(devNum), regAddr, offset, 1,
                                regValue);
    if (GT_OK != rc)
    {
        return rc;
    }


    /* <Profile <p> Port WRTD En> register */
    regAddr = PRV_PX_REG1_UNIT_TXQ_Q_MAC(devNum).tailDrop.tailDropConfig.
              profilePortWRTDEn[profile];

    regValue = BOOL2BIT_MAC(enablerPtr->portLimit);

    /* Set <Profile<p>Dp<X>Tc<X>PortWRTDEn> field of  */
    /* <Profile <p> Port WRTD En> register            */
    rc = prvCpssHwPpSetRegField(CAST_SW_DEVNUM(devNum), regAddr, offset, 1,
                                regValue);
    if (GT_OK != rc)
    {
        return rc;
    }


    /* <Profile <p> Queue WRTD En> register */
    regAddr = PRV_PX_REG1_UNIT_TXQ_Q_MAC(devNum).tailDrop.tailDropConfig.
              profileQueueWRTDEn[profile];

    regValue = BOOL2BIT_MAC(enablerPtr->tcDpLimit);

    /* Set <Profile<p>Dp<X>Tc<X>QueueWRTDEn> field of  */
    /* <Profile <p> Queue WRTD En> register            */
    return prvCpssHwPpSetRegField(CAST_SW_DEVNUM(devNum), regAddr, offset, 1,
                                  regValue);
}

/**
* @internal cpssPxPortTxTailDropProfileWeightedRandomTailDropEnableSet function
* @endinternal
*
* @brief   Enable/disable Weighted Random Tail Drop Threshold to overcome
*         synchronization.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] profile                  - the Profile Set in which the Traffic Class Drop Parameters
*                                      is associated.
* @param[in] dp                       - Drop Precedence.
*                                      (APPLICABLE RANGES: 0..2).
* @param[in] tc                       - the Traffic Class.
*                                      (APPLICABLE RANGES: 0..7).
* @param[in] enablerPtr               - (pointer to) Tail Drop limits enabling for Weigthed Random
*                                      Tail Drop
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on invalid input paramters value
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortTxTailDropProfileWeightedRandomTailDropEnableSet
(
    IN  GT_SW_DEV_NUM                               devNum,
    IN  CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_SET_ENT   profile,
    IN  GT_U32                                      dp,
    IN  GT_U32                                      tc,
    IN  CPSS_PX_PORT_TX_TAIL_DROP_WRTD_ENABLER_STC  *enablerPtr
)
{
    GT_STATUS   rc;

    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortTxTailDropProfileWeightedRandomTailDropEnableSet);
    CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, profile, dp, tc, enablerPtr));

    rc = internal_cpssPxPortTxTailDropProfileWeightedRandomTailDropEnableSet(devNum,
            profile, dp, tc, enablerPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, profile, dp, tc, enablerPtr));
    CPSS_API_UNLOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortTxTailDropProfileWeightedRandomTailDropEnableGet function
* @endinternal
*
* @brief   Get Random Tail drop Threshold status.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] profile                  - the Profile Set in which the Traffic Class Drop Parameters
*                                      is associated.
* @param[in] dp                       - Drop Precedence
*                                      (APPLICABLE RANGES: 0..2).
* @param[in] tc                       - the Traffic Class.
*                                      (APPLICABLE RANGES: 0..7).
*
* @param[out] enablerPtr               - (pointer to) Tail Drop limits enabling for Weigthed Random
*                                      Tail Drop.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on invalid input paramters value
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPortTxTailDropProfileWeightedRandomTailDropEnableGet
(
    IN  GT_SW_DEV_NUM                               devNum,
    IN  CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_SET_ENT   profile,
    IN  GT_U32                                      dp,
    IN  GT_U32                                      tc,
    OUT CPSS_PX_PORT_TX_TAIL_DROP_WRTD_ENABLER_STC  *enablerPtr
)
{
    GT_STATUS   rc;         /* return code */
    GT_U32      regAddr;    /* register address */
    GT_U32      regValue;   /* register value */
    GT_U32      offset;     /* bit offset inside register */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_SET_CHECK_MAC(profile);
    PRV_CPSS_PX_COS_CHECK_TC_MAC(tc);
    CPSS_NULL_PTR_CHECK_MAC(enablerPtr);

    if (dp > 2)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    offset = (dp << 3) + tc;


    /* <Profile <p> Shared Pool WRTD En> register */
    regAddr = PRV_PX_REG1_UNIT_TXQ_Q_MAC(devNum).tailDrop.tailDropConfig.
              profilePoolWRTDEn[profile];

    /* Get <Profile<p>Dp<X>Tc<X>SharedPoolWRTDEn> field of  */
    /* <Profile <p> Shared Pool WRTD En> register           */
    rc = prvCpssHwPpGetRegField(CAST_SW_DEVNUM(devNum), regAddr, offset, 1,
                                &regValue);
    if (GT_OK != rc)
    {
        return rc;
    }

    enablerPtr->sharedPoolLimit = BIT2BOOL_MAC(regValue);


    /* <Profile <p> Max Queue WRTD En> register */
    regAddr = PRV_PX_REG1_UNIT_TXQ_Q_MAC(devNum).tailDrop.tailDropConfig.
              profileMaxQueueWRTDEn[profile];

    /* Get <Profile<p>DP<X>TC<X>MaxQueueWRTDEn> field of  */
    /* <Profile <p> Max Queue WRTD En> register           */
    rc = prvCpssHwPpGetRegField(CAST_SW_DEVNUM(devNum), regAddr, offset, 1,
                                &regValue);
    if (GT_OK != rc)
    {
        return rc;
    }

    enablerPtr->tcLimit = BIT2BOOL_MAC(regValue);


    /* <Profile <p> Port WRTD En> register */
    regAddr = PRV_PX_REG1_UNIT_TXQ_Q_MAC(devNum).tailDrop.tailDropConfig.
              profilePortWRTDEn[profile];

    /* Get <Profile<p>Dp<X>Tc<X>PortWRTDEn> field of  */
    /* <Profile <p> Port WRTD En> register            */
    rc = prvCpssHwPpGetRegField(CAST_SW_DEVNUM(devNum), regAddr, offset, 1,
                                &regValue);
    if (GT_OK != rc)
    {
        return rc;
    }

    enablerPtr->portLimit = BIT2BOOL_MAC(regValue);


    /* <Profile <p> Queue WRTD En> register */
    regAddr = PRV_PX_REG1_UNIT_TXQ_Q_MAC(devNum).tailDrop.tailDropConfig.
              profileQueueWRTDEn[profile];

    /* Get <Profile<p>Dp<X>Tc<X>QueueWRTDEn> field of  */
    /* <Profile <p> Queue WRTD En> register            */
    rc = prvCpssHwPpGetRegField(CAST_SW_DEVNUM(devNum), regAddr, offset, 1,
                                &regValue);
    if (rc != GT_OK)
    {
        return rc;
    }

    enablerPtr->tcDpLimit = BIT2BOOL_MAC(regValue);

    return GT_OK;
}

/**
* @internal cpssPxPortTxTailDropProfileWeightedRandomTailDropEnableGet function
* @endinternal
*
* @brief   Get Random Tail drop Threshold status.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] profile                  - the Profile Set in which the Traffic Class Drop Parameters
*                                      is associated.
* @param[in] dp                       - Drop Precedence
*                                      (APPLICABLE RANGES: 0..2).
* @param[in] tc                       - the Traffic Class.
*                                      (APPLICABLE RANGES: 0..7).
*
* @param[out] enablerPtr               - (pointer to) Tail Drop limits enabling for Weigthed Random
*                                      Tail Drop.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on invalid input paramters value
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortTxTailDropProfileWeightedRandomTailDropEnableGet
(
    IN  GT_SW_DEV_NUM                               devNum,
    IN  CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_SET_ENT   profile,
    IN  GT_U32                                      dp,
    IN  GT_U32                                      tc,
    OUT CPSS_PX_PORT_TX_TAIL_DROP_WRTD_ENABLER_STC  *enablerPtr
)
{
    GT_STATUS   rc;

    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortTxTailDropProfileWeightedRandomTailDropEnableGet);
    CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, profile, dp, tc, enablerPtr));

    rc = internal_cpssPxPortTxTailDropProfileWeightedRandomTailDropEnableGet(devNum,
            profile, dp, tc, enablerPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, profile, dp, tc, enablerPtr));
    CPSS_API_UNLOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortTxTailDropWrtdMasksSet function
* @endinternal
*
* @brief   Sets Weighted Random Tail Drop (WRTD) masks.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] maskLsbPtr               - WRTD masked least significant bits.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_OUT_OF_RANGE          - on out of range value
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPortTxTailDropWrtdMasksSet
(
    IN  GT_SW_DEV_NUM                               devNum,
    IN  CPSS_PX_PORT_TX_TAIL_DROP_WRTD_MASK_LSB_STC *maskLsbPtr
)
{
    GT_STATUS   rc;         /* return code */
    GT_U32      regAddr;    /* register address */
    GT_U32      regValue;   /* register value */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(maskLsbPtr);

    if ((maskLsbPtr->port > 20) || (maskLsbPtr->tcDp > 20) ||
        (maskLsbPtr->pool > 20) || (maskLsbPtr->tc > 20))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }


    /* <SharedPoolWRTD Masks> register */
    regAddr = PRV_PX_REG1_UNIT_TXQ_Q_MAC(devNum).tailDrop.tailDropConfig.
              poolWRTDMasks;

    regValue = BIT_MASK_0_31_MAC(maskLsbPtr->pool);

    /* Set <SharedPoolWRTD_Mask> field of <SharedPoolWRTD Mask> register */
    rc = prvCpssHwPpSetRegField(CAST_SW_DEVNUM(devNum), regAddr, 0, 20, regValue);
    if (GT_OK != rc)
    {
        return rc;
    }


    /* <Port WRTD Masks> register */
    regAddr = PRV_PX_REG1_UNIT_TXQ_Q_MAC(devNum).tailDrop.tailDropConfig.
              portWRTDMasks;

    regValue = BIT_MASK_0_31_MAC(maskLsbPtr->port);

    /* Set <PortWRDT_Mask> field of <Port WRTD Masks> register */
    rc = prvCpssHwPpSetRegField(CAST_SW_DEVNUM(devNum), regAddr, 0, 20, regValue);
    if (GT_OK != rc)
    {
        return rc;
    }


    /* <MaxQueueWRTD Masks> register */
    regAddr = PRV_PX_REG1_UNIT_TXQ_Q_MAC(devNum).tailDrop.tailDropConfig.
              maxQueueWRTDMasks;

    regValue = BIT_MASK_0_31_MAC(maskLsbPtr->tc);

    /* Set <MaxQueueWRTD_Mask> field of <MaxQueueWRTD Masks> register */
    rc = prvCpssHwPpSetRegField(CAST_SW_DEVNUM(devNum), regAddr, 0, 20, regValue);
    if (GT_OK != rc)
    {
        return rc;
    }


    /* <QueueWRTD Masks> register */
    regAddr = PRV_PX_REG1_UNIT_TXQ_Q_MAC(devNum).tailDrop.tailDropConfig.
              queueWRTDMasks;

    regValue = BIT_MASK_0_31_MAC(maskLsbPtr->tcDp);

    /* Set <QueueWRTD_Mask> field of <QueueWRTD Masks> register */
    return prvCpssHwPpSetRegField(CAST_SW_DEVNUM(devNum), regAddr, 0, 20, regValue);
}

/**
* @internal cpssPxPortTxTailDropWrtdMasksSet function
* @endinternal
*
* @brief   Sets Weighted Random Tail Drop (WRTD) masks.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] maskLsbPtr               - WRTD masked least significant bits.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_OUT_OF_RANGE          - on out of range value
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortTxTailDropWrtdMasksSet
(
    IN  GT_SW_DEV_NUM                               devNum,
    IN  CPSS_PX_PORT_TX_TAIL_DROP_WRTD_MASK_LSB_STC *maskLsbPtr
)
{
    GT_STATUS rc;

    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortTxTailDropWrtdMasksSet);
    CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, maskLsbPtr));

    rc = internal_cpssPxPortTxTailDropWrtdMasksSet(devNum, maskLsbPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, maskLsbPtr));
    CPSS_API_UNLOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortTxTailDropWrtdMasksGet function
* @endinternal
*
* @brief   Gets Weighted Random Tail Drop (WRTD) masks.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] maskLsbPtr               - (pointer to) WRTD masked least significant bits.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPortTxTailDropWrtdMasksGet
(
    IN  GT_SW_DEV_NUM                               devNum,
    OUT CPSS_PX_PORT_TX_TAIL_DROP_WRTD_MASK_LSB_STC *maskLsbPtr
)
{
    GT_STATUS   rc;         /* return code */
    GT_U32      regAddr;    /* register address */
    GT_U32      regValue;   /* register value */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(maskLsbPtr);


    /* <SharedPoolWRTD Masks> register */
    regAddr = PRV_PX_REG1_UNIT_TXQ_Q_MAC(devNum).tailDrop.tailDropConfig.
              poolWRTDMasks;

    /* Get <SharedPoolWRTD_Mask> field of <SharedPoolWRTD Mask> register */
    rc = prvCpssHwPpGetRegField(CAST_SW_DEVNUM(devNum), regAddr, 0, 20, &regValue);
    if (GT_OK != rc)
    {
        return rc;
    }

    PRV_CPSS_PX_PORT_TX_TAIL_DROP_BIT_MASK_GET_MAC(maskLsbPtr->pool, regValue);


    /* <Port WRTD Masks> register */
    regAddr = PRV_PX_REG1_UNIT_TXQ_Q_MAC(devNum).tailDrop.tailDropConfig.
              portWRTDMasks;

    /* Get <PortWRDT_Mask> field of <Port WRTD Masks> register */
    rc = prvCpssHwPpGetRegField(CAST_SW_DEVNUM(devNum), regAddr, 0, 20, &regValue);
    if (GT_OK != rc)
    {
        return rc;
    }

    PRV_CPSS_PX_PORT_TX_TAIL_DROP_BIT_MASK_GET_MAC(maskLsbPtr->port, regValue);


    /* <MaxQueueWRTD Masks> register */
    regAddr = PRV_PX_REG1_UNIT_TXQ_Q_MAC(devNum).tailDrop.tailDropConfig.
              maxQueueWRTDMasks;

    /* Get <MaxQueueWRTD_Mask> field of <MaxQueueWRTD Masks> register */
    rc = prvCpssHwPpGetRegField(CAST_SW_DEVNUM(devNum), regAddr, 0, 20, &regValue);
    if (GT_OK != rc)
    {
        return rc;
    }

    PRV_CPSS_PX_PORT_TX_TAIL_DROP_BIT_MASK_GET_MAC(maskLsbPtr->tc, regValue);


    /* <QueueWRTD Masks> register */
    regAddr = PRV_PX_REG1_UNIT_TXQ_Q_MAC(devNum).tailDrop.tailDropConfig.
              queueWRTDMasks;

    /* Get <QueueWRTD_Mask> field of <QueueWRTD Masks> register */
    rc = prvCpssHwPpGetRegField(CAST_SW_DEVNUM(devNum), regAddr, 0, 20, &regValue);
    if (rc != GT_OK)
    {
        return rc;
    }

    PRV_CPSS_PX_PORT_TX_TAIL_DROP_BIT_MASK_GET_MAC(maskLsbPtr->tcDp, regValue);


    return GT_OK;
}

/**
* @internal cpssPxPortTxTailDropWrtdMasksGet function
* @endinternal
*
* @brief   Gets Weighted Random Tail Drop (WRTD) masks.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] maskLsbPtr               - (pointer to) WRTD masked least significant bits.
* @param[out] devNum
*                                      GT_OK                    - on success
*                                      GT_FAIL                  - on error
*                                      GT_BAD_PARAM             - on wrong device number
*                                      GT_HW_ERROR              - on hardware error
*                                      GT_BAD_PTR               - one of the parameters is NULL pointer
*                                      GT_NOT_APPLICABLE_DEVICE - on not applicable device
*                                      COMMENTS:
*                                      None.
*/
GT_STATUS cpssPxPortTxTailDropWrtdMasksGet
(
    IN  GT_SW_DEV_NUM                               devNum,
    OUT CPSS_PX_PORT_TX_TAIL_DROP_WRTD_MASK_LSB_STC *maskLsbPtr
)
{
    GT_STATUS rc;

    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortTxTailDropWrtdMasksGet);
    CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, maskLsbPtr));

    rc = internal_cpssPxPortTxTailDropWrtdMasksGet(devNum, maskLsbPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, maskLsbPtr));
    CPSS_API_UNLOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortTxTailDropProfileTcSharingSet function
* @endinternal
*
* @brief   Enable/Disable usage of the shared descriptors / buffer pool for
*         packets with the traffic class (tc) that are transmited via a port that
*         is associated with the Profile (profile). Sets the shared pool
*         associated for traffic class and profile.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] profile                  - the Profile Set in which the Traffic Class Drop Parameters
*                                      is associated.
* @param[in] tc                       - the Traffic Class.
*                                      (APPLICABLE RANGES: 0..7).
* @param[in] enableMode               - Drop Precedence (DPs) enabled mode for sharing.
* @param[in] poolNum                  - shared pool associated.
*                                      (APPLICABLE RANGES: 0..7).
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number, profile
* @retval GT_OUT_OF_RANGE          - on out of range value
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPortTxTailDropProfileTcSharingSet
(
    IN  GT_SW_DEV_NUM                                   devNum,
    IN  CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_SET_ENT       profile,
    IN  GT_U32                                          tc,
    IN  CPSS_PX_PORT_TX_TAIL_DROP_SHARED_DP_MODE_ENT    enableMode,
    IN  GT_U32                                          poolNum
)
{
    GT_STATUS   rc;         /* return code */
    GT_U32      regAddr;    /* register address */
    GT_U32      regValue;   /* register value */
    GT_U32      offset;     /* register offset */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_SET_CHECK_MAC(profile);
    PRV_CPSS_PX_COS_CHECK_TC_MAC(tc);
    PRV_CPSS_PX_PORT_TX_TAIL_DROP_POOL_NUM_CHECK_MAC(poolNum);

    switch (enableMode)
    {
        case CPSS_PX_PORT_TX_TAIL_DROP_SHARED_DP_MODE_DP0_E:
            regValue = 0;
            break;
        case CPSS_PX_PORT_TX_TAIL_DROP_SHARED_DP_MODE_DP0_DP1_E:
            regValue = 1;
            break;
        case CPSS_PX_PORT_TX_TAIL_DROP_SHARED_DP_MODE_ALL_E:
            regValue = 2;
            break;
        case CPSS_PX_PORT_TX_TAIL_DROP_SHARED_DP_MODE_DISABLE_E:
            regValue = 3;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }


    /* <Profile PriorityQueue<t> Enable Shared Pool Usage> register */
    regAddr = PRV_PX_REG1_UNIT_TXQ_Q_MAC(devNum).tailDrop.tailDropConfig.
              profilePriorityQueueEnablePoolUsage[tc];

    offset = 2 * ((GT_U32) profile);

    /* Set <Profile<X>PriorityQueue<t>EnSharedPoolUsage> field of    */
    /* <Profile PriorityQueue<t> Enable Shared Pool Usage> register  */
    rc = prvCpssHwPpSetRegField(CAST_SW_DEVNUM(devNum), regAddr, offset, 2, regValue);
    if (rc != GT_OK)
    {
        return rc;
    }


    /* <Profile<p> Priority Queue to Shared Pool Association> register */
    regAddr = PRV_PX_REG1_UNIT_TXQ_Q_MAC(devNum).tailDrop.tailDropConfig.
              profilePriorityQueueToPoolAssociation[profile];

    offset = 3 * tc;

    /* Set <Queue Profile<p> Priority Queue<X> to Shared Pool> field of  */
    /* <Profile<p> Priority Queue to Shared Pool Association> register   */
    return prvCpssHwPpSetRegField(CAST_SW_DEVNUM(devNum), regAddr, offset, 3, poolNum);
}

/**
* @internal cpssPxPortTxTailDropProfileTcSharingSet function
* @endinternal
*
* @brief   Enable/Disable usage of the shared descriptors / buffer pool for
*         packets with the traffic class (tc) that are transmited via a port that
*         is associated with the Profile (profile). Sets the shared pool
*         associated for traffic class and profile.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] profile                  - the Profile Set in which the Traffic Class Drop Parameters
*                                      is associated.
* @param[in] tc                       - the Traffic Class.
*                                      (APPLICABLE RANGES: 0..7).
* @param[in] enableMode               - Drop Precedence (DPs) enabled mode for sharing.
* @param[in] poolNum                  - shared pool associated.
*                                      (APPLICABLE RANGES: 0..7).
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number, profile
* @retval GT_OUT_OF_RANGE          - on out of range value
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortTxTailDropProfileTcSharingSet
(
    IN  GT_SW_DEV_NUM                                   devNum,
    IN  CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_SET_ENT       profile,
    IN  GT_U32                                          tc,
    IN  CPSS_PX_PORT_TX_TAIL_DROP_SHARED_DP_MODE_ENT    enableMode,
    IN  GT_U32                                          poolNum
)
{
    GT_STATUS rc;

    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortTxTailDropProfileTcSharingSet);
    CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, profile, tc, enableMode, poolNum));

    rc = internal_cpssPxPortTxTailDropProfileTcSharingSet(devNum, profile, tc,
                                                          enableMode, poolNum);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, profile, tc, enableMode, poolNum));
    CPSS_API_UNLOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortTxTailDropProfileTcSharingGet function
* @endinternal
*
* @brief   Get usage of the shared descriptors / buffer pool status for packets
*         with the traffic class (tc) that are transmited via a port that is
*         associated with the Profile (profile). Get the shared pool associated
*         for traffic class and Profile.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] profile                  - the Profile Set in which the Traffic Class Drop
*                                      Parameters is associated.
* @param[in] tc                       - the Traffic Class.
*                                      (APPLICABLE RANGES: 0..7).
*
* @param[out] enableModePtr            - (pointer to) Drop Precedence (DPs) enabled mode for
*                                      sharing.
* @param[out] poolNumPtr               - (pointer to) shared pool associated.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number, profile
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_OUT_OF_RANGE          - on out of range value
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPortTxTailDropProfileTcSharingGet
(
    IN  GT_SW_DEV_NUM                                   devNum,
    IN  CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_SET_ENT       profile,
    IN  GT_U32                                          tc,
    OUT CPSS_PX_PORT_TX_TAIL_DROP_SHARED_DP_MODE_ENT    *enableModePtr,
    OUT GT_U32                                          *poolNumPtr
)
{
    GT_STATUS   rc;         /* return code */
    GT_U32      regAddr;    /* register address */
    GT_U32      regValue;   /* register value */
    GT_U32      offset;     /* register offset */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_SET_CHECK_MAC(profile);
    PRV_CPSS_PX_COS_CHECK_TC_MAC(tc);
    CPSS_NULL_PTR_CHECK_MAC(enableModePtr);
    CPSS_NULL_PTR_CHECK_MAC(poolNumPtr);

    /* <Profile PriorityQueue<t> Enable Shared Pool Usage> register */
    regAddr = PRV_PX_REG1_UNIT_TXQ_Q_MAC(devNum).tailDrop.tailDropConfig.
              profilePriorityQueueEnablePoolUsage[tc];

    offset = 2 * ((GT_U32) profile);

    /* Get <Profile<X>PriorityQueue<t>EnSharedPoolUsage> field of    */
    /* <Profile PriorityQueue<t> Enable Shared Pool Usage> register  */
    rc = prvCpssHwPpGetRegField(CAST_SW_DEVNUM(devNum), regAddr, offset, 2,
                                &regValue);
    if (rc != GT_OK)
    {
        return rc;
    }

    switch (regValue)
    {
        case 0:
            *enableModePtr = CPSS_PX_PORT_TX_TAIL_DROP_SHARED_DP_MODE_DP0_E;
            break;
        case 1:
            *enableModePtr = CPSS_PX_PORT_TX_TAIL_DROP_SHARED_DP_MODE_DP0_DP1_E;
            break;
        case 2:
            *enableModePtr = CPSS_PX_PORT_TX_TAIL_DROP_SHARED_DP_MODE_ALL_E;
            break;
        case 3:
            *enableModePtr = CPSS_PX_PORT_TX_TAIL_DROP_SHARED_DP_MODE_DISABLE_E;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    };


    /* <Profile<p> Priority Queue to Shared Pool Association> register */
    regAddr = PRV_PX_REG1_UNIT_TXQ_Q_MAC(devNum).tailDrop.tailDropConfig.
              profilePriorityQueueToPoolAssociation[profile];

    offset = 3 * tc;

    /* Get <Queue Profile<p> Priority Queue<X> to Shared Pool> field of  */
    /* <Profile<p> Priority Queue to Shared Pool Association> register   */
    rc = prvCpssHwPpGetRegField(CAST_SW_DEVNUM(devNum), regAddr, offset, 3,
                                &regValue);
    if (rc != GT_OK)
    {
        return rc;
    }

    *poolNumPtr = regValue;

    return GT_OK;
}

/**
* @internal cpssPxPortTxTailDropProfileTcSharingGet function
* @endinternal
*
* @brief   Get usage of the shared descriptors / buffer pool status for packets
*         with the traffic class (tc) that are transmited via a port that is
*         associated with the Profile (profile). Get the shared pool associated
*         for traffic class and Profile.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] profile                  - the Profile Set in which the Traffic Class Drop
*                                      Parameters is associated.
* @param[in] tc                       - the Traffic Class.
*                                      (APPLICABLE RANGES: 0..7).
*
* @param[out] enableModePtr            - (pointer to) Drop Precedence (DPs) enabled mode for
*                                      sharing.
* @param[out] poolNumPtr               - (pointer to) shared pool associated.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number, profile
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_OUT_OF_RANGE          - on out of range value
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortTxTailDropProfileTcSharingGet
(
    IN  GT_SW_DEV_NUM                                   devNum,
    IN  CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_SET_ENT       profile,
    IN  GT_U32                                          tc,
    OUT CPSS_PX_PORT_TX_TAIL_DROP_SHARED_DP_MODE_ENT    *enableModePtr,
    OUT GT_U32                                          *poolNumPtr
)
{
    GT_STATUS rc;

    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortTxTailDropProfileTcSharingGet);
    CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, profile, tc, enableModePtr, poolNumPtr));

    rc = internal_cpssPxPortTxTailDropProfileTcSharingGet(devNum, profile, tc,
                                                          enableModePtr, poolNumPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, profile, tc, enableModePtr, poolNumPtr));
    CPSS_API_UNLOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortTxTailDropProfileTcSet function
* @endinternal
*
* @brief   Set tail drop profiles limits for particular TC.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] profile                  - the Profile Set in which the Traffic Class
*                                      Drop Parameters is associated.
* @param[in] tc                       - the Traffic Class associated with this set
*                                      of Drop Parameters.
*                                      (APPLICABLE RANGES: 0..7).
* @param[in] tailDropProfileParamsPtr - the Drop Profile Parameters to associate
*                                      with the Traffic Class in this Profile set.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number, profile set or
*                                       traffic class
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_OUT_OF_RANGE          - on out of range value
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPortTxTailDropProfileTcSet
(
    IN  GT_SW_DEV_NUM                               devNum,
    IN  CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_SET_ENT   profile,
    IN  GT_U32                                      tc,
    IN  CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_TC_STC    *tailDropProfileParamsPtr
)
{
    GT_STATUS   rc;             /* return code */
    GT_U32      tableIndex;     /* table index */
    GT_U32      hwQueueAlpha;   /* HW value of dp<N>QueueAlpha */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_SET_CHECK_MAC(profile);
    PRV_CPSS_PX_COS_CHECK_TC_MAC(tc);
    CPSS_NULL_PTR_CHECK_MAC(tailDropProfileParamsPtr);

    /* Check parameters that we will set */
    rc = prvCpssPxPortTxTailDropProfileTcParamCheck(tailDropProfileParamsPtr);
    if (rc != GT_OK)
    {
        return rc;
    }

    tableIndex = (profile << 3) + tc;

    PRV_CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_TO_HW_CONVERT_MAC(
            tailDropProfileParamsPtr->dp0QueueAlpha, hwQueueAlpha);

    /* Set <Descriptors Limit>, <Buffer Limit> and <DP Queue Alpha>  */
    /* fields of <Queue Limits DP0 - Enqueue> entry                  */
    rc = prvCpssPxPortTxTailDropProfileTcWriteTableEntry(devNum, tableIndex,
            CPSS_PX_TABLE_TXQ_QUEUE_TAIL_DROP_LIMITS_DP0_E,
            3,  /* count of fields */
            0,  16, tailDropProfileParamsPtr->dp0MaxDescNum, /* <Descriptors Limit> */
            32, 20, tailDropProfileParamsPtr->dp0MaxBuffNum, /* <Buffer Limit> */
            52, 3,  hwQueueAlpha);                           /* <DP Queue Alpha> */
    if (rc != GT_OK)
    {
        return rc;
    }

    /* Set <DP0MCBufferLimit> field of <Queue Buffer Limits - Dequeue> entry */
    rc = prvCpssPxPortTxTailDropProfileTcWriteTableEntry(devNum, tableIndex,
            CPSS_PX_TABLE_TXQ_QUEUE_TAIL_DROP_BUF_LIMITS_E,
            1,  /* count of fields */
            16, 20, tailDropProfileParamsPtr->dp0MaxBuffNum);/* <DP0MCBufferLimit> */
    if (rc != GT_OK)
    {
        return rc;
    }

    /* Set <DP0DescLimit> field of <Queue Descriptor Limits - Dequeue> entry */
    rc = prvCpssPxPortTxTailDropProfileTcWriteTableEntry(devNum, tableIndex,
            CPSS_PX_TABLE_TXQ_QUEUE_TAIL_DROP_DESC_LIMITS_E,
            1,  /* count of fields */
            0,  16, tailDropProfileParamsPtr->dp0MaxDescNum);/* <DP0DescLimit> */
    if (rc != GT_OK)
    {
        return rc;
    }

    PRV_CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_TO_HW_CONVERT_MAC(
            tailDropProfileParamsPtr->dp1QueueAlpha, hwQueueAlpha);

    /* Set <Descriptors Limit>, <Buffer Limit> and <DP Queue Alpha>    */
    /* fields of <Queue Limits DP12 - Enqueue> entry (entries 0..127)  */
    rc = prvCpssPxPortTxTailDropProfileTcWriteTableEntry(devNum, tableIndex,
            CPSS_PX_TABLE_TXQ_QUEUE_TAIL_DROP_LIMITS_DP1_E,
            3,  /* count of fields */
            0,  16, tailDropProfileParamsPtr->dp1MaxDescNum, /* <Descriptors Limit> */
            32, 20, tailDropProfileParamsPtr->dp1MaxBuffNum, /* <Buffer Limit> */
            52, 3,  hwQueueAlpha);                           /* <DP Queue Alpha> */
    if (rc != GT_OK)
    {
        return rc;
    }

    PRV_CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_TO_HW_CONVERT_MAC(
            tailDropProfileParamsPtr->dp2QueueAlpha, hwQueueAlpha);

    /* Set <Descriptors Limit>, <Buffer Limit> and <DP Queue Alpha>      */
    /* fields of <Queue Limits DP12 - Enqueue> entry (entries 128..255)  */
    rc = prvCpssPxPortTxTailDropProfileTcWriteTableEntry(devNum, tableIndex,
            CPSS_PX_TABLE_TXQ_QUEUE_TAIL_DROP_LIMITS_DP2_E,
            3,  /* count of fields */
            0,  16, tailDropProfileParamsPtr->dp2MaxDescNum, /* <Descriptors Limit> */
            32, 20, tailDropProfileParamsPtr->dp2MaxBuffNum, /* <Buffer Limit> */
            52, 3,  hwQueueAlpha);                           /* <DP Queue Alpha> */
    if (rc != GT_OK)
    {
        return rc;
    }

    /* Set <Maximum Queue Desc Limit> and <Maximum Queue Buffer Limit> */
    /* fields of <Maximum Queue Limits> entry                          */
    rc = prvCpssPxPortTxTailDropProfileTcWriteTableEntry(devNum, tableIndex,
            CPSS_PX_TABLE_TXQ_QUEUE_TAIL_DROP_MAX_QUEUE_LIMITS_E,
            2,  /* count of fields */
            0,  16, tailDropProfileParamsPtr->tcMaxDescNum, /* <Maximum Queue Desc Limit> */
            16, 20, tailDropProfileParamsPtr->tcMaxBuffNum);/* <Maximum Queue Buffer Limit> */

    return rc;
}

/**
* @internal cpssPxPortTxTailDropProfileTcSet function
* @endinternal
*
* @brief   Set tail drop profiles limits for particular TC.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] profile                  - the Profile Set in which the Traffic Class
*                                      Drop Parameters is associated.
* @param[in] tc                       - the Traffic Class associated with this set
*                                      of Drop Parameters.
*                                      (APPLICABLE RANGES: 0..7).
* @param[in] tailDropProfileParamsPtr - the Drop Profile Parameters to associate
*                                      with the Traffic Class in this Profile set.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number, profile set or
*                                       traffic class
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_OUT_OF_RANGE          - on out of range value
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortTxTailDropProfileTcSet
(
    IN  GT_SW_DEV_NUM                               devNum,
    IN  CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_SET_ENT   profile,
    IN  GT_U32                                      tc,
    IN  CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_TC_STC    *tailDropProfileParamsPtr
)
{
    GT_STATUS rc;

    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortTxTailDropProfileTcSet);
    CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, profile, tc, tailDropProfileParamsPtr));

    rc = internal_cpssPxPortTxTailDropProfileTcSet(devNum, profile, tc,
                                                   tailDropProfileParamsPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, profile, tc, tailDropProfileParamsPtr));
    CPSS_API_UNLOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortTxTailDropProfileTcGet function
* @endinternal
*
* @brief   Get tail drop profiles limits for particular TC.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] profile                  - the Profile Set in which the Traffic Class
*                                      Drop Parameters is associated.
* @param[in] tc                       - the Traffic Class associated with this set
*                                      of Drop Parameters.
*                                      (APPLICABLE RANGES: 0..7).
*
* @param[out] tailDropProfileParamsPtr - (pointer to) the Drop Profile Parameters to
*                                      associate with the Traffic Class in this
*                                      Profile.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number, profile set or
*                                       traffic class
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPortTxTailDropProfileTcGet
(
    IN  GT_SW_DEV_NUM                               devNum,
    IN  CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_SET_ENT   profile,
    IN  GT_U32                                      tc,
    OUT CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_TC_STC    *tailDropProfileParamsPtr
)
{
    GT_STATUS   rc;             /* return code */
    GT_U32      tableIndex;     /* table index */
    GT_U32      hwPortAlpha;    /* HW value of port alpha */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_SET_CHECK_MAC(profile);
    PRV_CPSS_PX_COS_CHECK_TC_MAC(tc);
    CPSS_NULL_PTR_CHECK_MAC(tailDropProfileParamsPtr);

    tableIndex = (profile << 3) + tc;

    /* Get <Descriptors Limit>, <Buffer Limit> and <DP Queue Alpha>  */
    /* fields of <Queue Limits DP0 - Enqueue> entry                  */
    rc = prvCpssPxPortTxTailDropProfileTcReadTableEntry(devNum, tableIndex,
            CPSS_PX_TABLE_TXQ_QUEUE_TAIL_DROP_LIMITS_DP0_E,
            3,  /* count of fields */
            0,  16, &(tailDropProfileParamsPtr->dp0MaxDescNum), /* <Descriptors Limit> */
            32, 20, &(tailDropProfileParamsPtr->dp0MaxBuffNum), /* <Buffer Limit> */
            52, 3,  &hwPortAlpha);                              /* <DP Queue Alpha> */
    if (rc != GT_OK)
    {
        return rc;
    }

    PRV_CPSS_PX_PORT_TX_TAIL_DROP_DBA_HW_TO_ALPHA_CONVERT_MAC(hwPortAlpha,
            tailDropProfileParamsPtr->dp0QueueAlpha);

    /* Get <Descriptors Limit>, <Buffer Limit> and <DP Queue Alpha>    */
    /* fields of <Queue Limits DP12 - Enqueue> entry (entries 0..127)  */
    rc = prvCpssPxPortTxTailDropProfileTcReadTableEntry(devNum, tableIndex,
            CPSS_PX_TABLE_TXQ_QUEUE_TAIL_DROP_LIMITS_DP1_E,
            3,  /* count of fields */
            0,  16, &(tailDropProfileParamsPtr->dp1MaxDescNum), /* <Descriptors Limit> */
            32, 20, &(tailDropProfileParamsPtr->dp1MaxBuffNum), /* <Buffer Limit> */
            52, 3,  &hwPortAlpha);                              /* <DP Queue Alpha> */
    if (rc != GT_OK)
    {
        return rc;
    }

    PRV_CPSS_PX_PORT_TX_TAIL_DROP_DBA_HW_TO_ALPHA_CONVERT_MAC(hwPortAlpha,
            tailDropProfileParamsPtr->dp1QueueAlpha);

    /* Get <Descriptors Limit>, <Buffer Limit> and <DP Queue Alpha>      */
    /* fields of <Queue Limits DP12 - Enqueue> entry (entries 128..255)  */
    rc = prvCpssPxPortTxTailDropProfileTcReadTableEntry(devNum, tableIndex,
            CPSS_PX_TABLE_TXQ_QUEUE_TAIL_DROP_LIMITS_DP2_E,
            3,  /* count of fields */
            0,  16, &(tailDropProfileParamsPtr->dp2MaxDescNum), /* <Descriptors Limit> */
            32, 20, &(tailDropProfileParamsPtr->dp2MaxBuffNum), /* <Buffer Limit> */
            52, 3,  &hwPortAlpha);                              /* <DP Queue Alpha> */
    if (rc != GT_OK)
    {
        return rc;
    }

    PRV_CPSS_PX_PORT_TX_TAIL_DROP_DBA_HW_TO_ALPHA_CONVERT_MAC(hwPortAlpha,
            tailDropProfileParamsPtr->dp2QueueAlpha);

    /* Get <Maximum Queue Desc Limit> and <Maximum Queue Buffer Limit> */
    /* fields of <Maximum Queue Limits> entry                          */
    rc = prvCpssPxPortTxTailDropProfileTcReadTableEntry(devNum, tableIndex,
            CPSS_PX_TABLE_TXQ_QUEUE_TAIL_DROP_MAX_QUEUE_LIMITS_E,
            2,  /* count of fields */
            0,  16, &(tailDropProfileParamsPtr->tcMaxDescNum), /* <Maximum Queue Desc Limit> */
            16, 20, &(tailDropProfileParamsPtr->tcMaxBuffNum));/* <Maximum Queue Buffer Limit> */

    return rc;
}

/**
* @internal cpssPxPortTxTailDropProfileTcGet function
* @endinternal
*
* @brief   Get tail drop profiles limits for particular TC.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] profile                  - the Profile Set in which the Traffic Class
*                                      Drop Parameters is associated.
* @param[in] tc                       - the Traffic Class associated with this set
*                                      of Drop Parameters.
*                                      (APPLICABLE RANGES: 0..7).
*
* @param[out] tailDropProfileParamsPtr - (pointer to) the Drop Profile Parameters to
*                                      associate with the Traffic Class in this
*                                      Profile.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number, profile set or
*                                       traffic class
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortTxTailDropProfileTcGet
(
    IN  GT_SW_DEV_NUM                               devNum,
    IN  CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_SET_ENT   profile,
    IN  GT_U32                                      tc,
    OUT CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_TC_STC    *tailDropProfileParamsPtr
)
{
    GT_STATUS rc;

    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortTxTailDropProfileTcGet);
    CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, profile, tc, tailDropProfileParamsPtr));

    rc = internal_cpssPxPortTxTailDropProfileTcGet(devNum, profile, tc,
                                                   tailDropProfileParamsPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, profile, tc, tailDropProfileParamsPtr));
    CPSS_API_UNLOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortTxTailDropTcBuffNumberGet function
* @endinternal
*
* @brief   Gets the current number of buffers allocated on specified port
*         for specified Traffic Class queues.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] tc                       - traffic class.
*                                      (APPLICABLE RANGES: 0..7).
*
* @param[out] numberPtr                - (pointer to) the number of buffers
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPortTxTailDropTcBuffNumberGet
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_U32                  tc,
    OUT GT_U32                  *numberPtr
)
{
    GT_STATUS   rc;         /* return code */
    GT_U32      txqPortNum; /* TxQ port number */
    GT_U32      tableIndex; /* table index */
    GT_U32      value;      /* value that we will read from table */
    CPSS_PX_TABLE_ENT   tableType;  /* table type */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_TXQ_PORT_GET_MAC(devNum, portNum, txqPortNum);
    PRV_CPSS_PX_COS_CHECK_TC_MAC(tc);
    CPSS_NULL_PTR_CHECK_MAC(numberPtr);

    /* <Queue Maintenance Buffers> entry */
    tableType = CPSS_PX_TABLE_TXQ_QUEUE_TAIL_DROP_COUNTERS_Q_MAIN_BUFF_E;

    tableIndex = (txqPortNum << 3) | tc;

    /* Get <BufferCounter> field of <Queue Maintenance Buffers> entry */
    rc = prvCpssPxReadTableEntry(devNum, tableType, tableIndex, &value);
    if (rc != GT_OK)
    {
        return rc;
    }

    *numberPtr = value;

    return GT_OK;
}

/**
* @internal cpssPxPortTxTailDropTcBuffNumberGet function
* @endinternal
*
* @brief   Gets the current number of buffers allocated on specified port
*         for specified Traffic Class queues.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] tc                       - traffic class.
*                                      (APPLICABLE RANGES: 0..7).
*
* @param[out] numberPtr                - (pointer to) the number of buffers
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortTxTailDropTcBuffNumberGet
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_U32                  tc,
    OUT GT_U32                  *numberPtr
)
{
    GT_STATUS rc;

    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortTxTailDropTcBuffNumberGet);
    CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, tc, numberPtr));

    rc = internal_cpssPxPortTxTailDropTcBuffNumberGet(devNum, portNum, tc, numberPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, tc, numberPtr));
    CPSS_API_UNLOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortTxTailDropMcastPcktDescLimitSet function
* @endinternal
*
* @brief   Set maximal descriptors limits for multicast packets.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] mcastMaxDescNum          - The number of descriptors allocated for multicast
*                                      packets.
*                                      (APPLICABLE RANGES: 0..0xFFFF).
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPortTxTailDropMcastPcktDescLimitSet
(
    IN  GT_SW_DEV_NUM   devNum,
    IN  GT_U32          mcastMaxDescNum
)
{
    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);

    if (mcastMaxDescNum >= BIT_16)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    /* Set <Multicast Descriptors Limit> field of  */
    /* <Multicast Descriptors Limit> register      */
    return prvCpssHwPpSetRegField(CAST_SW_DEVNUM(devNum),
            PRV_PX_REG1_UNIT_TXQ_Q_MAC(devNum).tailDrop.mcFilterLimits.mcDescsLimit,
            0, 16, mcastMaxDescNum);
}

/**
* @internal cpssPxPortTxTailDropMcastPcktDescLimitSet function
* @endinternal
*
* @brief   Set maximal descriptors limits for multicast packets.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] mcastMaxDescNum          - The number of descriptors allocated for multicast
*                                      packets.
*                                      (APPLICABLE RANGES: 0..0xFFFF).
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortTxTailDropMcastPcktDescLimitSet
(
    IN  GT_SW_DEV_NUM   devNum,
    IN  GT_U32          mcastMaxDescNum
)
{
    GT_STATUS rc;

    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortTxTailDropMcastPcktDescLimitSet);
    CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, mcastMaxDescNum));

    rc = internal_cpssPxPortTxTailDropMcastPcktDescLimitSet(devNum, mcastMaxDescNum);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, mcastMaxDescNum));
    CPSS_API_UNLOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortTxTailDropMcastPcktDescLimitGet function
* @endinternal
*
* @brief   Get maximal descriptors limits for multicast packets.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] mcastMaxDescNumPtr       - (pointer to) the number of descriptors allocated
*                                      for multicast packets.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPortTxTailDropMcastPcktDescLimitGet
(
    IN  GT_SW_DEV_NUM   devNum,
    OUT GT_U32          *mcastMaxDescNumPtr
)
{
    GT_STATUS   rc;         /* return code */
    GT_U32      regValue;   /* register value */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(mcastMaxDescNumPtr);

    /* Get <Multicast Descriptors Limit> field of  */
    /* <Multicast Descriptors Limit> register      */
    rc = prvCpssHwPpGetRegField(CAST_SW_DEVNUM(devNum),
            PRV_PX_REG1_UNIT_TXQ_Q_MAC(devNum).tailDrop.mcFilterLimits.mcDescsLimit,
            0, 16, &regValue);
    if (rc != GT_OK)
    {
        return rc;
    }

    *mcastMaxDescNumPtr = regValue;

    return GT_OK;
}

/**
* @internal cpssPxPortTxTailDropMcastPcktDescLimitGet function
* @endinternal
*
* @brief   Get maximal descriptors limits for multicast packets.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] mcastMaxDescNumPtr       - (pointer to) the number of descriptors allocated
*                                      for multicast packets.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortTxTailDropMcastPcktDescLimitGet
(
    IN  GT_SW_DEV_NUM   devNum,
    OUT GT_U32          *mcastMaxDescNumPtr
)
{
    GT_STATUS rc;

    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortTxTailDropMcastPcktDescLimitGet);
    CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, mcastMaxDescNumPtr));

    rc = internal_cpssPxPortTxTailDropMcastPcktDescLimitGet(devNum, mcastMaxDescNumPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, mcastMaxDescNumPtr));
    CPSS_API_UNLOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortTxTailDropMcastBuffersLimitSet function
* @endinternal
*
* @brief   Set maximal buffers limits for multicast packets.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] mcastMaxBuffNum          - The number of buffers allocated for multicast packets.
*                                      (APPLICABLE RANGES: 0..0xFFFFF).
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - on out of range mcastMaxBuffNum
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPortTxTailDropMcastBuffersLimitSet
(
    IN  GT_SW_DEV_NUM   devNum,
    IN  GT_U32          mcastMaxBuffNum
)
{
    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);

    if (mcastMaxBuffNum >= BIT_20)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    /* Set <Multicast Buffers Limit> field of  */
    /* <Multicast Buffers Limit> register      */
    return prvCpssHwPpSetRegField(CAST_SW_DEVNUM(devNum),
            PRV_PX_REG1_UNIT_TXQ_Q_MAC(devNum).tailDrop.mcFilterLimits.mcBuffersLimit,
            0, 20, mcastMaxBuffNum);
}

/**
* @internal cpssPxPortTxTailDropMcastBuffersLimitSet function
* @endinternal
*
* @brief   Set maximal buffers limits for multicast packets.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] mcastMaxBuffNum          - The number of buffers allocated for multicast packets.
*                                      (APPLICABLE RANGES: 0..0xFFFFF).
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - on out of range mcastMaxBuffNum
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortTxTailDropMcastBuffersLimitSet
(
    IN  GT_SW_DEV_NUM   devNum,
    IN  GT_U32          mcastMaxBuffNum
)
{
    GT_STATUS rc;

    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortTxTailDropMcastBuffersLimitSet);
    CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, mcastMaxBuffNum));

    rc = internal_cpssPxPortTxTailDropMcastBuffersLimitSet(devNum, mcastMaxBuffNum);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, mcastMaxBuffNum));
    CPSS_API_UNLOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortTxTailDropMcastBuffersLimitGet function
* @endinternal
*
* @brief   Get maximal buffers limits for multicast packets.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] mcastMaxBuffNumPtr       - (pointer to) the number of buffers allocated for
*                                      multicast packets.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPortTxTailDropMcastBuffersLimitGet
(
    IN  GT_SW_DEV_NUM   devNum,
    OUT GT_U32          *mcastMaxBuffNumPtr
)
{
    GT_STATUS   rc;         /* return code */
    GT_U32      regValue;   /* register value */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(mcastMaxBuffNumPtr);

    /* Get <Multicast Buffers Limit> field of  */
    /* <Multicast Buffers Limit> register      */
    rc = prvCpssHwPpGetRegField(CAST_SW_DEVNUM(devNum),
            PRV_PX_REG1_UNIT_TXQ_Q_MAC(devNum).tailDrop.mcFilterLimits.mcBuffersLimit,
            0, 20, &regValue);
    if (rc != GT_OK)
    {
        return rc;
    }

    *mcastMaxBuffNumPtr = regValue;

    return GT_OK;
}

/**
* @internal cpssPxPortTxTailDropMcastBuffersLimitGet function
* @endinternal
*
* @brief   Get maximal buffers limits for multicast packets.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] mcastMaxBuffNumPtr       - (pointer to) the number of buffers allocated for
*                                      multicast packets.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortTxTailDropMcastBuffersLimitGet
(
    IN  GT_SW_DEV_NUM   devNum,
    OUT GT_U32          *mcastMaxBuffNumPtr
)
{
    GT_STATUS rc;

    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortTxTailDropMcastBuffersLimitGet);
    CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, mcastMaxBuffNumPtr));

    rc = internal_cpssPxPortTxTailDropMcastBuffersLimitGet(devNum, mcastMaxBuffNumPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, mcastMaxBuffNumPtr));
    CPSS_API_UNLOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortTxTailDropMcastDescNumberGet function
* @endinternal
*
* @brief   Gets total number of multi-target descriptors allocated.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] numberPtr                - number of descriptors
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Multi-target descriptors are descriptors that are duplicated to
*       multiple egress targets, e.g. Multicast, Broadcast, or unknown Unicast
*       flooded packets.
*
*/
static GT_STATUS internal_cpssPxPortTxTailDropMcastDescNumberGet
(
    IN  GT_SW_DEV_NUM   devNum,
    OUT GT_U32          *numberPtr
)
{
    GT_STATUS   rc;         /* return code */
    GT_U32      regValue;   /* register value */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(numberPtr);

    rc = prvCpssHwPpGetRegField(CAST_SW_DEVNUM(devNum),
            PRV_PX_REG1_UNIT_TXQ_Q_MAC(devNum).tailDrop.muliticastFilterCntrs.mcDescsCntr,
            0, 16, &regValue);
    if (rc != GT_OK)
    {
        return rc;
    }

    *numberPtr = regValue;

    return GT_OK;
}

/**
* @internal cpssPxPortTxTailDropMcastDescNumberGet function
* @endinternal
*
* @brief   Gets total number of multi-target descriptors allocated.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] numberPtr                - number of descriptors
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Multi-target descriptors are descriptors that are duplicated to
*       multiple egress targets, e.g. Multicast, Broadcast, or unknown Unicast
*       flooded packets.
*
*/
GT_STATUS cpssPxPortTxTailDropMcastDescNumberGet
(
    IN  GT_SW_DEV_NUM   devNum,
    OUT GT_U32          *numberPtr
)
{
    GT_STATUS rc;

    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortTxTailDropMcastDescNumberGet);
    CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, numberPtr));

    rc = internal_cpssPxPortTxTailDropMcastDescNumberGet(devNum, numberPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, numberPtr));
    CPSS_API_UNLOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortTxTailDropMcastBuffNumberGet function
* @endinternal
*
* @brief   Gets total number of multi-target buffers allocated (virtual buffers).
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] numberPtr                - number of buffers
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The buffers associated with enqueued multi-target descriptors are
*       counted multiple times, once for each multi-target descriptor instance.
*       Multi-target descriptors are descriptors that are duplicated to
*       multiple egress targets, e.g. Multicast, Broadcast, or unknown Unicast
*       flooded packets.
*
*/
static GT_STATUS internal_cpssPxPortTxTailDropMcastBuffNumberGet
(
    IN  GT_SW_DEV_NUM   devNum,
    OUT GT_U32          *numberPtr
)
{
    GT_STATUS   rc;         /* return code */
    GT_U32      regValue;   /* register value */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(numberPtr);

    rc = prvCpssHwPpGetRegField(CAST_SW_DEVNUM(devNum),
            PRV_PX_REG1_UNIT_TXQ_Q_MAC(devNum).tailDrop.muliticastFilterCntrs.mcBuffersCntr,
            0, 20, &regValue);
    if (rc != GT_OK)
    {
        return rc;
    }

    *numberPtr = regValue;

    return GT_OK;
}

/**
* @internal cpssPxPortTxTailDropMcastBuffNumberGet function
* @endinternal
*
* @brief   Gets total number of multi-target buffers allocated (virtual buffers).
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] numberPtr                - number of buffers
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The buffers associated with enqueued multi-target descriptors are
*       counted multiple times, once for each multi-target descriptor instance.
*       Multi-target descriptors are descriptors that are duplicated to
*       multiple egress targets, e.g. Multicast, Broadcast, or unknown Unicast
*       flooded packets.
*
*/
GT_STATUS cpssPxPortTxTailDropMcastBuffNumberGet
(
    IN  GT_SW_DEV_NUM   devNum,
    OUT GT_U32          *numberPtr
)
{
    GT_STATUS rc;

    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortTxTailDropMcastBuffNumberGet);
    CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, numberPtr));

    rc = internal_cpssPxPortTxTailDropMcastBuffNumberGet(devNum, numberPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, numberPtr));
    CPSS_API_UNLOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortTxTailDropSharedPolicySet function
* @endinternal
*
* @brief   Sets shared pool allocation policy for enqueuing of packets.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] policy                   - shared pool allocation policy.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device or wrong policy
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPortTxTailDropSharedPolicySet
(
    IN  GT_SW_DEV_NUM                               devNum,
    IN  CPSS_PX_PORT_TX_TAIL_DROP_SHARED_POLICY_ENT policy
)
{
    GT_U32  regAddr;    /* register address */
    GT_U32  regValue;   /* register value */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);

    switch (policy)
    {
        case CPSS_PX_PORT_TX_TAIL_DROP_SHARED_POLICY_UNCONSTRAINED_E:
            regValue = 0;
            break;
        case CPSS_PX_PORT_TX_TAIL_DROP_SHARED_POLICY_CONSTRAINED_E:
            regValue = 1;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* <Global Tail Drop Configuration> register */
    regAddr = PRV_PX_REG1_UNIT_TXQ_Q_MAC(devNum).tailDrop.tailDropConfig.
              globalTailDropConfig;

    /* Set <Constrained Resource Sharing> field of  */
    /* <Global Tail Drop Configuration> register    */
    return prvCpssHwPpSetRegField(CAST_SW_DEVNUM(devNum), regAddr, 6, 1, regValue);
}

/**
* @internal cpssPxPortTxTailDropSharedPolicySet function
* @endinternal
*
* @brief   Sets shared pool allocation policy for enqueuing of packets.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] policy                   - shared pool allocation policy.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device or wrong policy
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortTxTailDropSharedPolicySet
(
    IN  GT_SW_DEV_NUM                               devNum,
    IN  CPSS_PX_PORT_TX_TAIL_DROP_SHARED_POLICY_ENT policy
)
{
    GT_STATUS rc;

    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortTxTailDropSharedPolicySet);
    CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, policy));

    rc = internal_cpssPxPortTxTailDropSharedPolicySet(devNum, policy);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, policy));
    CPSS_API_UNLOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortTxTailDropSharedPolicyGet function
* @endinternal
*
* @brief   Gets shared pool allocation policy for enqueuing of packets.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
*
* @param[out] policyPtr                - (pointer to) shared pool allocation policy.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPortTxTailDropSharedPolicyGet
(
    IN  GT_SW_DEV_NUM                               devNum,
    OUT CPSS_PX_PORT_TX_TAIL_DROP_SHARED_POLICY_ENT *policyPtr
)
{
    GT_STATUS   rc;         /* return code */
    GT_U32      regAddr;    /* register address */
    GT_U32      regValue;   /* register value */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(policyPtr);

    /* <Global Tail Drop Configuration> register */
    regAddr = PRV_PX_REG1_UNIT_TXQ_Q_MAC(devNum).tailDrop.tailDropConfig.
              globalTailDropConfig;

    /* Get <Constrained Resource Sharing> field of  */
    /* <Global Tail Drop Configuration> register    */
    rc = prvCpssHwPpGetRegField(CAST_SW_DEVNUM(devNum), regAddr, 6, 1, &regValue);
    if (rc != GT_OK)
    {
        return rc;
    }

    *policyPtr = (regValue == 0)
                 ? CPSS_PX_PORT_TX_TAIL_DROP_SHARED_POLICY_UNCONSTRAINED_E
                 : CPSS_PX_PORT_TX_TAIL_DROP_SHARED_POLICY_CONSTRAINED_E;

    return GT_OK;
}

/**
* @internal cpssPxPortTxTailDropSharedPolicyGet function
* @endinternal
*
* @brief   Gets shared pool allocation policy for enqueuing of packets.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
*
* @param[out] policyPtr                - (pointer to) shared pool allocation policy.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortTxTailDropSharedPolicyGet
(
    IN  GT_SW_DEV_NUM                               devNum,
    OUT CPSS_PX_PORT_TX_TAIL_DROP_SHARED_POLICY_ENT *policyPtr
)
{
    GT_STATUS rc;

    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortTxTailDropSharedPolicyGet);
    CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, policyPtr));

    rc = internal_cpssPxPortTxTailDropSharedPolicyGet(devNum, policyPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, policyPtr));
    CPSS_API_UNLOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortTxTailDropSharedPoolLimitsSet function
* @endinternal
*
* @brief   Set maximal descriptors and buffers limits for shared pool.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] poolNum                  - Shared pool number.
*                                      (APPLICABLE RANGES: 0..7).
* @param[in] maxBuffNum               - The number of buffers allocated for a shared pool.
*                                      (APPLICABLE RANGES: 0..0xFFFFF).
* @param[in] maxDescNum               - The number of descriptors allocated for a shared pool.
*                                      (APPLICABLE RANGES: 0..0xFFFF).
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_RANGE          - on out of range value
*/
static GT_STATUS internal_cpssPxPortTxTailDropSharedPoolLimitsSet
(
    IN  GT_SW_DEV_NUM   devNum,
    IN  GT_U32          poolNum,
    IN  GT_U32          maxBuffNum,
    IN  GT_U32          maxDescNum
)
{
    GT_STATUS   rc;         /* return code */
    GT_U32      regAddr;    /* register address */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_TX_TAIL_DROP_POOL_NUM_CHECK_MAC(poolNum);

    if ((maxBuffNum >= BIT_20) || (maxDescNum >= BIT_16))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }


    /* <Shared Pool<t> Descriptor Limits> register */
    regAddr = PRV_PX_REG1_UNIT_TXQ_Q_MAC(devNum).tailDrop.tailDropLimits.
              poolDescLimits[poolNum];

    /* Set <SharedPoolDescLimit> field of           */
    /* <Shared Pool<t> Descriptor Limits> register  */
    rc = prvCpssHwPpSetRegField(CAST_SW_DEVNUM(devNum), regAddr, 0, 16, maxDescNum);
    if (rc != GT_OK)
    {
        return rc;
    }


    /* <Shared Pool<t> Buffer Limits> register */
    regAddr = PRV_PX_REG1_UNIT_TXQ_Q_MAC(devNum).tailDrop.tailDropLimits.
              poolBufferLimits[poolNum];

    /* Set <SharedPoolBuffLimit> field of <Shared Pool<t> Buffer Limits> register */
    return prvCpssHwPpSetRegField(CAST_SW_DEVNUM(devNum), regAddr, 0, 20, maxBuffNum);
}

/**
* @internal cpssPxPortTxTailDropSharedPoolLimitsSet function
* @endinternal
*
* @brief   Set maximal descriptors and buffers limits for shared pool.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] poolNum                  - Shared pool number.
*                                      (APPLICABLE RANGES: 0..7).
* @param[in] maxBuffNum               - The number of buffers allocated for a shared pool.
*                                      (APPLICABLE RANGES: 0..0xFFFFF).
* @param[in] maxDescNum               - The number of descriptors allocated for a shared pool.
*                                      (APPLICABLE RANGES: 0..0xFFFF).
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_RANGE          - on out of range value
*/
GT_STATUS cpssPxPortTxTailDropSharedPoolLimitsSet
(
    IN  GT_SW_DEV_NUM   devNum,
    IN  GT_U32          poolNum,
    IN  GT_U32          maxBuffNum,
    IN  GT_U32          maxDescNum
)
{
    GT_STATUS rc;

    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortTxTailDropSharedPoolLimitsSet);
    CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, poolNum, maxBuffNum, maxDescNum));

    rc = internal_cpssPxPortTxTailDropSharedPoolLimitsSet(devNum, poolNum,
                                                          maxBuffNum, maxDescNum);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, poolNum, maxBuffNum, maxDescNum));
    CPSS_API_UNLOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortTxTailDropSharedPoolLimitsGet function
* @endinternal
*
* @brief   Get maximal descriptors and buffers limits for shared pool.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] poolNum                  - Shared pool number.
*                                      (APPLICABLE RANGES: 0..7).
*
* @param[out] maxBuffNumPtr            - (pointer to) the number of buffers allocated for a
*                                      shared pool.
* @param[out] maxDescNumPtr            - (pointer to) the number of descriptors allocated
*                                      for a shared pool.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPortTxTailDropSharedPoolLimitsGet
(
    IN  GT_SW_DEV_NUM   devNum,
    IN  GT_U32          poolNum,
    OUT GT_U32          *maxBuffNumPtr,
    OUT GT_U32          *maxDescNumPtr
)
{
    GT_STATUS   rc;         /* return code */
    GT_U32      regAddr;    /* register address */
    GT_U32      regValue;   /* register value */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_TX_TAIL_DROP_POOL_NUM_CHECK_MAC(poolNum);
    CPSS_NULL_PTR_CHECK_MAC(maxBuffNumPtr);
    CPSS_NULL_PTR_CHECK_MAC(maxDescNumPtr);


    /* <Shared Pool<t> Descriptor Limits> register */
    regAddr = PRV_PX_REG1_UNIT_TXQ_Q_MAC(devNum).tailDrop.tailDropLimits.
              poolDescLimits[poolNum];

    /* Get <SharedPoolDescLimit> field of           */
    /* <Shared Pool<t> Descriptor Limits> register  */
    rc = prvCpssHwPpGetRegField(CAST_SW_DEVNUM(devNum), regAddr, 0, 16, &regValue);
    if (rc != GT_OK)
    {
        return rc;
    }

    *maxDescNumPtr = regValue;


    /* <Shared Pool<t> Buffer Limits> register */
    regAddr = PRV_PX_REG1_UNIT_TXQ_Q_MAC(devNum).tailDrop.tailDropLimits.
              poolBufferLimits[poolNum];

    /* Get <SharedPoolBuffLimit> field of       */
    /* <Shared Pool<t> Buffer Limits> register  */
    rc = prvCpssHwPpGetRegField(CAST_SW_DEVNUM(devNum), regAddr, 0, 20, &regValue);
    if (rc != GT_OK)
    {
        return rc;
    }

    *maxBuffNumPtr = regValue;

    return GT_OK;
}

/**
* @internal cpssPxPortTxTailDropSharedPoolLimitsGet function
* @endinternal
*
* @brief   Get maximal descriptors and buffers limits for shared pool.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] poolNum                  - Shared pool number.
*                                      (APPLICABLE RANGES: 0..7).
*
* @param[out] maxBuffNumPtr            - (pointer to) the number of buffers allocated for a
*                                      shared pool.
* @param[out] maxDescNumPtr            - (pointer to) the number of descriptors allocated
*                                      for a shared pool.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortTxTailDropSharedPoolLimitsGet
(
    IN  GT_SW_DEV_NUM   devNum,
    IN  GT_U32          poolNum,
    OUT GT_U32          *maxBuffNumPtr,
    OUT GT_U32          *maxDescNumPtr
)
{
    GT_STATUS rc;

    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortTxTailDropSharedPoolLimitsGet);
    CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, poolNum, maxBuffNumPtr, maxDescNumPtr));

    rc = internal_cpssPxPortTxTailDropSharedPoolLimitsGet(devNum, poolNum,
            maxBuffNumPtr, maxDescNumPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, poolNum, maxBuffNumPtr, maxDescNumPtr));
    CPSS_API_UNLOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortTxTailDropSharedResourceDescNumberGet function
* @endinternal
*
* @brief   Gets total number of descriptors allocated from the shared descriptors
*         pool.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] tcQueue                  - traffic class queue.
*                                      (APPLICABLE RANGES: 0..7).
*
* @param[out] numberPtr                - number of descriptors
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong poolNum or devNum
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPortTxTailDropSharedResourceDescNumberGet
(
    IN  GT_SW_DEV_NUM   devNum,
    IN  GT_U32          tcQueue,
    OUT GT_U32          *numberPtr
)
{
    GT_STATUS   rc;         /* return code */
    GT_U32      regAddr;    /* register address */
    GT_U32      regValue;   /* register value */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_COS_CHECK_TC_MAC(tcQueue);
    CPSS_NULL_PTR_CHECK_MAC(numberPtr);

    /* <Priority<t> Shared Desc Counter> register */
    regAddr = PRV_PX_REG1_UNIT_TXQ_Q_MAC(devNum).tailDrop.tailDropCntrs.
              priorityDescCntr[tcQueue];

    /* Get value of <Priority<t> Shared Desc Counter> register */
    rc = prvCpssHwPpGetRegField(CAST_SW_DEVNUM(devNum), regAddr, 0, 16, &regValue);
    if (rc != GT_OK)
    {
        return rc;
    }

    *numberPtr = regValue;

    return GT_OK;
}

/**
* @internal cpssPxPortTxTailDropSharedResourceDescNumberGet function
* @endinternal
*
* @brief   Gets total number of descriptors allocated from the shared descriptors
*         pool.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] tcQueue                  - traffic class queue.
*                                      (APPLICABLE RANGES: 0..7).
*
* @param[out] numberPtr                - number of descriptors
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong poolNum or devNum
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortTxTailDropSharedResourceDescNumberGet
(
    IN  GT_SW_DEV_NUM   devNum,
    IN  GT_U32          tcQueue,
    OUT GT_U32          *numberPtr
)
{
    GT_STATUS rc;

    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortTxTailDropSharedResourceDescNumberGet);
    CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, tcQueue, numberPtr));

    rc = internal_cpssPxPortTxTailDropSharedResourceDescNumberGet(devNum, tcQueue, numberPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, tcQueue, numberPtr));
    CPSS_API_UNLOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortTxTailDropSharedResourceBuffNumberGet function
* @endinternal
*
* @brief   Gets the number of virtual buffers enqueued from the shared buffer pool.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] tcQueue                  - traffic class queue.
*                                      (APPLICABLE RANGES: 0..7).
*
* @param[out] numberPtr                - number of buffers
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong poolNum or devNum
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPortTxTailDropSharedResourceBuffNumberGet
(
    IN  GT_SW_DEV_NUM   devNum,
    IN  GT_U32          tcQueue,
    OUT GT_U32          *numberPtr
)
{
    GT_STATUS   rc;         /* return code */
    GT_U32      regAddr;    /* register address */
    GT_U32      regValue;   /* register value */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_COS_CHECK_TC_MAC(tcQueue);
    CPSS_NULL_PTR_CHECK_MAC(numberPtr);

    /* <Priority<t> Shared Desc Counter> register */
    regAddr = PRV_PX_REG1_UNIT_TXQ_Q_MAC(devNum).tailDrop.tailDropCntrs.
              priorityBuffersCntr[tcQueue];

    /* Get value of <Priority<t> Shared Desc Counter> register */
    rc = prvCpssHwPpGetRegField(CAST_SW_DEVNUM(devNum), regAddr, 0, 20, &regValue);
    if (rc != GT_OK)
    {
        return rc;
    }

    *numberPtr = regValue;

    return GT_OK;
}

/**
* @internal cpssPxPortTxTailDropSharedResourceBuffNumberGet function
* @endinternal
*
* @brief   Gets the number of virtual buffers enqueued from the shared buffer pool.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] tcQueue                  - traffic class queue.
*                                      (APPLICABLE RANGES: 0..7).
*
* @param[out] numberPtr                - number of buffers
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong poolNum or devNum
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortTxTailDropSharedResourceBuffNumberGet
(
    IN  GT_SW_DEV_NUM   devNum,
    IN  GT_U32          tcQueue,
    OUT GT_U32          *numberPtr
)
{
    GT_STATUS rc;

    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortTxTailDropSharedResourceBuffNumberGet);
    CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, tcQueue, numberPtr));

    rc = internal_cpssPxPortTxTailDropSharedResourceBuffNumberGet(devNum, tcQueue, numberPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, tcQueue, numberPtr));
    CPSS_API_UNLOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortTxTailDropGlobalDescNumberGet function
* @endinternal
*
* @brief   Gets total number of descriptors allocated.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] numberPtr                - number of descriptors
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPortTxTailDropGlobalDescNumberGet
(
    IN  GT_SW_DEV_NUM   devNum,
    OUT GT_U32          *numberPtr
)
{
    GT_STATUS   rc;         /* return code */
    GT_U32      regAddr;    /* register address */
    GT_U32      regValue;   /* register value */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(numberPtr);

    /* <Total Desc Counter> register */
    regAddr = PRV_PX_REG1_UNIT_TXQ_Q_MAC(devNum).tailDrop.tailDropCntrs.
              totalDescCntr;

    /* Set <TotalDescCnt> field of <Total Desc Counter> register */
    rc = prvCpssHwPpGetRegField(CAST_SW_DEVNUM(devNum), regAddr, 0, 16, &regValue);
    if (rc != GT_OK)
    {
        return rc;
    }

    *numberPtr = regValue;

    return GT_OK;
}

/**
* @internal cpssPxPortTxTailDropGlobalDescNumberGet function
* @endinternal
*
* @brief   Gets total number of descriptors allocated.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] numberPtr                - number of descriptors
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortTxTailDropGlobalDescNumberGet
(
    IN  GT_SW_DEV_NUM   devNum,
    OUT GT_U32          *numberPtr
)
{
    GT_STATUS rc;

    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortTxTailDropGlobalDescNumberGet);
    CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, numberPtr));

    rc = internal_cpssPxPortTxTailDropGlobalDescNumberGet(devNum, numberPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, numberPtr));
    CPSS_API_UNLOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortTxTailDropGlobalBuffNumberGet function
* @endinternal
*
* @brief   Gets total number of virtual buffers enqueued.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] numberPtr                - number of buffers
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The buffers associated with enqueued multi-target descriptors are
*       counted multiple times, once for each multi-target descriptor instance.
*
*/
static GT_STATUS internal_cpssPxPortTxTailDropGlobalBuffNumberGet
(
    IN  GT_SW_DEV_NUM   devNum,
    OUT GT_U32          *numberPtr
)
{
    GT_STATUS   rc;         /* return code */
    GT_U32      regAddr;    /* register address */
    GT_U32      regValue;   /* register value */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(numberPtr);

    /* <Total Buffers Counter> register */
    regAddr = PRV_PX_REG1_UNIT_TXQ_Q_MAC(devNum).tailDrop.tailDropCntrs.
              totalBuffersCntr;

    /* Set <TotalBuffersCnt> field of <Total Buffers Counter> register */
    rc = prvCpssHwPpGetRegField(CAST_SW_DEVNUM(devNum), regAddr, 0, 20, &regValue);
    if (rc != GT_OK)
    {
        return rc;
    }

    *numberPtr = regValue;

    return GT_OK;
}

/**
* @internal cpssPxPortTxTailDropGlobalBuffNumberGet function
* @endinternal
*
* @brief   Gets total number of virtual buffers enqueued.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] numberPtr                - number of buffers
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The buffers associated with enqueued multi-target descriptors are
*       counted multiple times, once for each multi-target descriptor instance.
*
*/
GT_STATUS cpssPxPortTxTailDropGlobalBuffNumberGet
(
    IN  GT_SW_DEV_NUM   devNum,
    OUT GT_U32          *numberPtr
)
{
    GT_STATUS rc;

    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortTxTailDropGlobalBuffNumberGet);
    CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, numberPtr));

    rc = internal_cpssPxPortTxTailDropGlobalBuffNumberGet(devNum, numberPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, numberPtr));
    CPSS_API_UNLOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortTxTailDropDescNumberGet function
* @endinternal
*
* @brief   Gets the current number of descriptors allocated per specified port.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
*
* @param[out] numberPtr                - number of descriptors
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPortTxTailDropDescNumberGet
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_U32                  *numberPtr
)
{
    GT_STATUS   rc;         /* return code */
    GT_U32      txqPortNum; /* TxQ port number */
    GT_U32      regAddr;    /* register address */
    GT_U32      regValue;   /* register value */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_TXQ_PORT_GET_MAC(devNum, portNum, txqPortNum);
    CPSS_NULL_PTR_CHECK_MAC(numberPtr);

    /* <Port<n> Desc Counter> register */
    regAddr = PRV_PX_REG1_UNIT_TXQ_Q_MAC(devNum).tailDrop.tailDropCntrs.
              portDescCntr[txqPortNum];

    /* Get <Port<n> DescCnt> field of <Port<n> Desc Counter> register */
    rc = prvCpssHwPpGetRegField(CAST_SW_DEVNUM(devNum), regAddr, 0, 16, &regValue);
    if (rc != GT_OK)
    {
        return rc;
    }

    *numberPtr = regValue;

    return GT_OK;
}

/**
* @internal cpssPxPortTxTailDropDescNumberGet function
* @endinternal
*
* @brief   Gets the current number of descriptors allocated per specified port.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
*
* @param[out] numberPtr                - number of descriptors
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortTxTailDropDescNumberGet
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_U32                  *numberPtr
)
{
    GT_STATUS rc;

    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortTxTailDropDescNumberGet);
    CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, numberPtr));

    rc = internal_cpssPxPortTxTailDropDescNumberGet(devNum, portNum, numberPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, numberPtr));
    CPSS_API_UNLOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortTxTailDropBuffNumberGet function
* @endinternal
*
* @brief   Gets the current number of buffers allocated per specified port.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
*
* @param[out] numberPtr                - number of buffers
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPortTxTailDropBuffNumberGet
(
    IN   GT_SW_DEV_NUM          devNum,
    IN   GT_PHYSICAL_PORT_NUM   portNum,
    OUT  GT_U32                 *numberPtr
)
{
    GT_STATUS   rc;         /* return code */
    GT_U32      txqPortNum; /* TxQ port number */
    GT_U32      regAddr;    /* register address */
    GT_U32      regValue;   /* register value */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_TXQ_PORT_GET_MAC(devNum, portNum, txqPortNum);
    CPSS_NULL_PTR_CHECK_MAC(numberPtr);

    /* <Port<n> Buffers Counter> register */
    regAddr = PRV_PX_REG1_UNIT_TXQ_Q_MAC(devNum).tailDrop.tailDropCntrs.
              portBuffersCntr[txqPortNum];

    /* Get <Port BuffersCnt> field of <Port<n> Buffers Counter> register */
    rc = prvCpssHwPpGetRegField(CAST_SW_DEVNUM(devNum), regAddr, 0, 20, &regValue);
    if (rc != GT_OK)
    {
        return rc;
    }

    *numberPtr = regValue;

    return GT_OK;
}

/**
* @internal cpssPxPortTxTailDropBuffNumberGet function
* @endinternal
*
* @brief   Gets the current number of buffers allocated per specified port.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
*
* @param[out] numberPtr                - number of buffers
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortTxTailDropBuffNumberGet
(
    IN   GT_SW_DEV_NUM          devNum,
    IN   GT_PHYSICAL_PORT_NUM   portNum,
    OUT  GT_U32                 *numberPtr
)
{
    GT_STATUS rc;

    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortTxTailDropBuffNumberGet);
    CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, numberPtr));

    rc = internal_cpssPxPortTxTailDropBuffNumberGet(devNum, portNum, numberPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, numberPtr));
    CPSS_API_UNLOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortTxTailDropDbaModeEnableSet function
* @endinternal
*
* @brief   Enable/disable Dynamic Buffers Allocation (DBA) mode of resource
*         management.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] enable                   - GT_TRUE -  DBA mode enabled
*                                      GT_FALSE - DBA mode disabled.
*/
static GT_STATUS internal_cpssPxPortTxTailDropDbaModeEnableSet
(
    IN  GT_SW_DEV_NUM   devNum,
    IN  GT_BOOL         enable
)
{
    GT_U32  regAddr;    /* register address */
    GT_U32  regValue;   /* register value */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    regValue = BOOL2BIT_MAC(enable);

    /* <Global Tail Drop Configuration> register */
    regAddr = PRV_PX_REG1_UNIT_TXQ_Q_MAC(devNum).tailDrop.tailDropConfig.
              globalTailDropConfig;

    /* Set <Enable dynamic buffer mode> field of  */
    /* <Global Tail Drop Configuration> register  */
    return prvCpssHwPpSetRegField(CAST_SW_DEVNUM(devNum), regAddr, 17, 1, regValue);
}

/**
* @internal cpssPxPortTxTailDropDbaModeEnableSet function
* @endinternal
*
* @brief   Enable/disable Dynamic Buffers Allocation (DBA) mode of resource
*         management.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] enable                   - GT_TRUE -  DBA mode enabled
*                                      GT_FALSE - DBA mode disabled.
*/
GT_STATUS cpssPxPortTxTailDropDbaModeEnableSet
(
    IN  GT_SW_DEV_NUM   devNum,
    IN  GT_BOOL         enable
)
{
    GT_STATUS rc;

    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortTxTailDropDbaModeEnableSet);
    CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, enable));

    rc = internal_cpssPxPortTxTailDropDbaModeEnableSet(devNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, enable));
    CPSS_API_UNLOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortTxTailDropDbaModeEnableGet function
* @endinternal
*
* @brief   Get state of Dynamic Buffers Allocation (DBA) mode of resource
*         management.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                - pointer to DBA mode state.
*                                      RETURN:
*                                      GT_OK                    - on success
*                                      GT_FAIL                  - on error
*                                      GT_BAD_PTR               - on NULL pointer
*                                      GT_BAD_PARAM             - on wrong device number
*                                      GT_HW_ERROR              - on hardware error
*                                      GT_NOT_APPLICABLE_DEVICE - on not applicable device
*                                      COMMENTS:
*                                      None.
*/
static GT_STATUS internal_cpssPxPortTxTailDropDbaModeEnableGet
(
    IN  GT_SW_DEV_NUM   devNum,
    OUT GT_BOOL         *enablePtr
)
{
    GT_STATUS   rc;         /* return code */
    GT_U32      regAddr;    /* register address */
    GT_U32      regValue;   /* register value */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    /* <Global Tail Drop Configuration> register */
    regAddr = PRV_PX_REG1_UNIT_TXQ_Q_MAC(devNum).tailDrop.tailDropConfig.
              globalTailDropConfig;

    /* Get <Enable dynamic buffer mode> field of  */
    /* <Global Tail Drop Configuration> register  */
    rc = prvCpssHwPpGetRegField(CAST_SW_DEVNUM(devNum), regAddr, 17, 1, &regValue);
    if (rc != GT_OK)
    {
        return rc;
    }

    *enablePtr = BIT2BOOL_MAC(regValue);

    return GT_OK;
}

/**
* @internal cpssPxPortTxTailDropDbaModeEnableGet function
* @endinternal
*
* @brief   Get state of Dynamic Buffers Allocation (DBA) mode of resource
*         management.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                - pointer to DBA mode state.
*                                      RETURN:
*                                      GT_OK                    - on success
*                                      GT_FAIL                  - on error
*                                      GT_BAD_PTR               - on NULL pointer
*                                      GT_BAD_PARAM             - on wrong device number
*                                      GT_HW_ERROR              - on hardware error
*                                      GT_NOT_APPLICABLE_DEVICE - on not applicable device
*                                      COMMENTS:
*                                      None.
*/
GT_STATUS cpssPxPortTxTailDropDbaModeEnableGet
(
    IN  GT_SW_DEV_NUM   devNum,
    OUT GT_BOOL         *enablePtr
)
{
    GT_STATUS rc;

    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortTxTailDropDbaModeEnableGet);
    CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, enablePtr));

    rc = internal_cpssPxPortTxTailDropDbaModeEnableGet(devNum, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortTxTailDropDbaAvailableBuffSet function
* @endinternal
*
* @brief   Set amount of buffers available for dynamic allocation.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] availableBuff            - amount of buffers available for dynamic allocation.
*                                      (APPLICABLE RANGES: 0..0x0FFFFF).
*
* @note Valid only if DBA mode enabled.
*
*/
static GT_STATUS internal_cpssPxPortTxTailDropDbaAvailableBuffSet
(
    IN  GT_SW_DEV_NUM   devNum,
    IN  GT_U32          availableBuff
)
{
    GT_U32  regAddr;    /* register address */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);

    if (availableBuff >= BIT_20)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    /* <Dynamic Tail Drop Available Buffers> register */
    regAddr = PRV_PX_REG1_UNIT_TXQ_Q_MAC(devNum).tailDrop.tailDropConfig.
              dynamicAvailableBuffers;

    /* Set <td_available_buffs> field of               */
    /* <Dynamic Tail Drop Available Buffers> register  */
    return prvCpssHwPpSetRegField(CAST_SW_DEVNUM(devNum), regAddr, 0, 20,
                                  availableBuff);
}

/**
* @internal cpssPxPortTxTailDropDbaAvailableBuffSet function
* @endinternal
*
* @brief   Set amount of buffers available for dynamic allocation.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] availableBuff            - amount of buffers available for dynamic allocation.
*                                      (APPLICABLE RANGES: 0..0x0FFFFF).
*
* @note Valid only if DBA mode enabled.
*
*/
GT_STATUS cpssPxPortTxTailDropDbaAvailableBuffSet
(
    IN  GT_SW_DEV_NUM   devNum,
    IN  GT_U32          availableBuff
)
{
    GT_STATUS rc;

    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortTxTailDropDbaAvailableBuffSet);
    CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, availableBuff));

    rc = internal_cpssPxPortTxTailDropDbaAvailableBuffSet(devNum, availableBuff);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, availableBuff));
    CPSS_API_UNLOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortTxTailDropDbaAvailableBuffGet function
* @endinternal
*
* @brief   Get amount of buffers available for dynamic allocation.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] availableBuffPtr         - pointer to amount of buffers available for dynamic
*                                      allocation.
*                                      RETURN:
*                                      GT_OK                    - on success
*                                      GT_FAIL                  - on error
*                                      GT_BAD_PTR               - on NULL pointer
*                                      GT_BAD_PARAM             - on wrong device number
*                                      GT_HW_ERROR              - on hardware error
*                                      GT_NOT_APPLICABLE_DEVICE - on not applicable device
*                                      COMMENTS:
*                                      None.
*/
static GT_STATUS internal_cpssPxPortTxTailDropDbaAvailableBuffGet
(
    IN  GT_SW_DEV_NUM   devNum,
    OUT GT_U32          *availableBuffPtr
)
{
    GT_STATUS   rc;         /* return code */
    GT_U32      regAddr;    /* register address */
    GT_U32      regValue;   /* register value */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(availableBuffPtr);

    /* <Dynamic Tail Drop Available Buffers> register */
    regAddr = PRV_PX_REG1_UNIT_TXQ_Q_MAC(devNum).tailDrop.tailDropConfig.
              dynamicAvailableBuffers;

    /* Get <td_available_buffs> field of               */
    /* <Dynamic Tail Drop Available Buffers> register  */
    rc = prvCpssHwPpGetRegField(CAST_SW_DEVNUM(devNum), regAddr, 0, 20, &regValue);
    if (rc != GT_OK)
    {
        return rc;
    }

    *availableBuffPtr = regValue;

    return GT_OK;
}

/**
* @internal cpssPxPortTxTailDropDbaAvailableBuffGet function
* @endinternal
*
* @brief   Get amount of buffers available for dynamic allocation.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] availableBuffPtr         - pointer to amount of buffers available for dynamic
*                                      allocation.
*                                      RETURN:
*                                      GT_OK                    - on success
*                                      GT_FAIL                  - on error
*                                      GT_BAD_PTR               - on NULL pointer
*                                      GT_BAD_PARAM             - on wrong device number
*                                      GT_HW_ERROR              - on hardware error
*                                      GT_NOT_APPLICABLE_DEVICE - on not applicable device
*                                      COMMENTS:
*                                      None.
*/
GT_STATUS cpssPxPortTxTailDropDbaAvailableBuffGet
(
    IN  GT_SW_DEV_NUM   devNum,
    OUT GT_U32          *availableBuffPtr
)
{
    GT_STATUS rc;

    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortTxTailDropDbaAvailableBuffGet);
    CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, availableBuffPtr));

    rc = internal_cpssPxPortTxTailDropDbaAvailableBuffGet(devNum, availableBuffPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, availableBuffPtr));
    CPSS_API_UNLOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

