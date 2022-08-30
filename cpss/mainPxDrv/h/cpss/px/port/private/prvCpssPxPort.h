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
* @file prvCpssPxPort.h
*
* @brief Includes structures definition for the use of CPSS Px Port lib .
*
*
* @version   58
********************************************************************************
*/
#ifndef __prvCpssPxPorth
#define __prvCpssPxPorth

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/cpssTypes.h>
#include <cpss/px/port/cpssPxPortCtrl.h>
#include <cpss/common/port/private/prvCpssPortTypes.h>

/* get the register address - port Descriptors Counter Register */
#define PRV_CPSS_PX_PORT_DESCR_COUNT_REG_MAC(devNum,portNum,regAddrPtr)\
    if(GT_OK != prvCpssPxPortTxDebugRegisterAddrGet(devNum,portNum,0,PRV_CPSS_PX_PORT_TX_REG_TYPE_DESCR_COUNTER_E,regAddrPtr))\
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG)

typedef enum{
    PRV_CPSS_PX_PORT_TX_REG_TYPE_TX_CONFIG_E,
    PRV_CPSS_PX_PORT_TX_REG_TYPE_TOKEN_BUCK_E,
    PRV_CPSS_PX_PORT_TX_REG_TYPE_TOKEN_BUCK_LEN_E,
    PRV_CPSS_PX_PORT_TX_REG_TYPE_PRIO_TOKEN_BUCK_E,
    PRV_CPSS_PX_PORT_TX_REG_TYPE_PRIO_TOKEN_BUCK_LEN_E,
    PRV_CPSS_PX_PORT_TX_REG_TYPE_DESCR_COUNTER_E,
    PRV_CPSS_PX_PORT_TX_REG_TYPE_BUFFER_COUNTER_E,
    PRV_CPSS_PX_PORT_TX_REG_TYPE_TC_DESCR_COUNTER_E,
    PRV_CPSS_PX_PORT_TX_REG_TYPE_TC_BUFFER_COUNTER_E,

    PRV_CPSS_PX_PROFILE_TX_REG_TYPE_WRR_WEIGHTS_0_E,
    PRV_CPSS_PX_PROFILE_TX_REG_TYPE_WRR_WEIGHTS_1_E,
    PRV_CPSS_PX_PROFILE_TX_REG_TYPE_WRR_STRICT_PRIO_E,
    PRV_CPSS_PX_PROFILE_TX_REG_TYPE_WRR_STRICT_PRIO_EN_E,
    PRV_CPSS_PX_PROFILE_TX_REG_TYPE_DESCR_LIMIT_E
}PRV_CPSS_PX_PORT_TX_REG_TYPE_ENT;

/* type for pointer to ...SerdesPowerUpSequence matrix */
typedef GT_U32 PRV_CPSS_PX_PORT_SERDES_POWER_UP_ARRAY[CPSS_PORT_SERDES_SPEED_NA_E+2];

/* type for pointer to lpSerdesExtConfig... matrix */
typedef GT_U32 PRV_CPSS_PX_PORT_SERDES_EXT_CFG_ARRAY[2];


/**
* @internal prvCpssPxPortTxDebugRegisterAddrGet function
* @endinternal
*
* @brief   get register address for 'txq per port' .
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
 *
* @param[in] devNum                   - device number.
* @param[in] primaryIndex             - primary index , can be used as :
*                                      port number (also CPU port)
*                                      or as profile number
* @param[in] secondaryIndex           - secondary index , can be used as TC
* @param[in] registerType             - register type
*
* @param[out] regAddrPtr               - (pointer to) register address
*                                       GT_OK
*/
GT_STATUS prvCpssPxPortTxDebugRegisterAddrGet
(
    IN  GT_SW_DEV_NUM    devNum,
    IN  GT_U32           primaryIndex,
    IN  GT_U32           secondaryIndex,
    IN  PRV_CPSS_PX_PORT_TX_REG_TYPE_ENT  registerType,
    OUT GT_U32           *regAddrPtr
);

/******************************************************************************
* PRV_CPSS_DXCH_PORT_SERDES_POWER_STATUS_SET_FUN
*
* DESCRIPTION:
*       Sets power state of SERDES port lanes according to port capabilities.
*       XG / XAUI ports: All 4 (TX or RX) lanes are set simultanuously.
*       HX / QX ports: lanes 0,1 or TX and RX may be set separately.
*
* APPLICABLE DEVICES:
*        Pipe.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       devNum    - physical device number
*       portNum   - physical port number
*       powerUp   - GT_TRUE  = power up, GT_FALSE = power down
*
* OUTPUTS:
*       None.
*
* RETURNS:
*     GT_OK            - on success.
*     GT_BAD_PARAM     - on bad parameters
*     GT_FAIL          - on error
*     GT_NOT_SUPPORTED - HW does not support the requested operation
*     GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*
******************************************************************************/
typedef GT_STATUS (*PRV_CPSS_PX_PORT_SERDES_POWER_STATUS_SET_FUN)
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_BOOL                 powerUp
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvCpssPxPorth */


