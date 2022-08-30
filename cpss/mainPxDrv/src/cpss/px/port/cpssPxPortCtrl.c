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
* @file cpssPxPortCtrl.c
*
* @brief CPSS implementation for Port configuration and control facility.
*
*
* @version   1
********************************************************************************
*/
#define CPSS_LOG_IN_MODULE_ENABLE
#include <cpss/px/port/private/prvCpssPxPortLog.h>
#include <cpss/px/port/private/prvCpssPxPortCtrl.h>
#include <cpss/px/port/private/prvCpssPxPortMacCtrl.h>
#include <cpss/px/port/PizzaArbiter/prvCpssPxPortPizzaArbiter.h>
#include <cpss/px/port/PortMapping/prvCpssPxPortMappingShadowDB.h>
#include <cpss/px/port/cpssPxPortCtrl.h>
#include <cpss/px/port/cpssPxPortAp.h>
#include <cpss/px/port/cpssPxPortTxDebug.h>
#include <cpss/px/port/cpssPxPortTxShaper.h>
#include <cpss/px/config/private/prvCpssPxInfo.h>
#include <cpss/px/cpssHwInit/private/prvCpssPxRegsVer1.h>
#include <cpss/px/diag/cpssPxDiag.h>
#include <cpss/px/port/private/prvCpssPxPortIfModeCfgPipeResource.h>
#include <cpss/px/port/cpssPxPortPfc.h>
#include <cpss/px/port/private/prvCpssPxPortCtrl.h>

#include <cpss/common/systemRecovery/cpssGenSystemRecovery.h>
#include <cpss/common/labServices/port/gop/port/mvHwsPortMiscIf.h>
#include <cpss/common/labServices/port/gop/port/mvPortModeElements.h>
#include <cpss/common/labServices/port/gop/port/silicon/general/mvHwsGeneralCpll.h>
#include <cpss/common/labServices/port/gop/port/mac/mvHwsMacIf.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>

#include <cpss/common/labServices/port/gop/port/mac/cgMac/mvHwsCgMac28nmIf.h>
#include <cpss/common/labServices/port/gop/port/pcs/mmPcs/mvHwsMMPcs28nmIf.h>

#include <cpss/common/port/private/prvCpssPortManagerTypes.h>
#include <cpss/common/labServices/port/gop/port/mvHwsPortInitIf.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

/* length of system tick in millisecond */
static GT_U32 tickTimeMsec;


PRV_CPSS_PX_PORT_CPLL_CONFIG_STC prvCpssPxPortCpllConfigArr[PRV_CPSS_PX_SERDES_NUM_CNS];


/**
* @internal prvCpssPxPortMacConfigurationClear function
* @endinternal
*
* @brief   Clear array of registers data
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in,out] regDataArray             - "clean" array of register's data
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL ptr
*/
GT_STATUS prvCpssPxPortMacConfigurationClear
(
    INOUT PRV_CPSS_PORT_REG_CONFIG_STC   *regDataArray
)
{
    PRV_CPSS_PORT_TYPE_ENT  portMacType;

    CPSS_NULL_PTR_CHECK_MAC(regDataArray);

    for (portMacType = PRV_CPSS_PORT_GE_E; portMacType < PRV_CPSS_PORT_NOT_APPLICABLE_E; portMacType++)
    {
        regDataArray[portMacType].regAddr = PRV_CPSS_SW_PTR_ENTRY_UNUSED;
    }

    return GT_OK;
}

/**
* @internal prvCpssPxPortMacConfiguration function
* @endinternal
*
* @brief   Write value to register field and duplicate it to other members of SW
*         combo if needed
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] regDataArray             - array of register's address/offset/field lenght/value
*                                      to write
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL ptr
* @retval GT_HW_ERROR              - if write failed
*/
GT_STATUS prvCpssPxPortMacConfiguration
(
    IN  GT_SW_DEV_NUM                           devNum,
    IN  GT_PHYSICAL_PORT_NUM                    portNum,
    IN  const PRV_CPSS_PORT_REG_CONFIG_STC   *regDataArray
)
{
    GT_STATUS               rc;
    GT_U32                  portGroupId;  /* port group id */
    PRV_CPSS_PORT_TYPE_ENT  portMacType;  /* MAC unit of port */
    GT_PHYSICAL_PORT_NUM    portMacNum;   /* MAC number of given physical port */

    CPSS_NULL_PTR_CHECK_MAC(regDataArray);
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);
    portGroupId = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portMacNum);

    for(portMacType = PRV_CPSS_PORT_GE_E; portMacType < PRV_CPSS_PORT_NOT_APPLICABLE_E; portMacType++)
    {
        if (regDataArray[portMacType].regAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
        {
            if(PRV_CPSS_PORT_CG_E == portMacType)
                mvHwsCgMac28nmAccessLock(devNum, portMacNum);

            rc = prvCpssDrvHwPpPortGroupSetRegField(CAST_SW_DEVNUM(devNum), portGroupId,
                        regDataArray[portMacType].regAddr,
                        regDataArray[portMacType].fieldOffset,
                        regDataArray[portMacType].fieldLength,
                        regDataArray[portMacType].fieldData);

            if(PRV_CPSS_PORT_CG_E == portMacType)
                mvHwsCgMac28nmAccessUnlock(devNum, portMacNum);

            if(rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
            }
        }
    }

    return GT_OK;
}

/**
* @internal internal_cpssPxPortForceLinkPassEnableSet function
* @endinternal
*
* @brief   Enable/disable Force Link Pass on specified port on specified device.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] state                    - GT_TRUE for force link pass, GT_FALSE otherwise
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPortForceLinkPassEnableSet
(
    IN  GT_SW_DEV_NUM            devNum,
    IN  GT_PHYSICAL_PORT_NUM     portNum,
    IN  GT_BOOL   state
)
{
    GT_U32  regAddr;    /* register address */
    GT_U32  portMacNum; /* MAC number */
    GT_U32  value;      /* data to write to register */
    PRV_CPSS_PORT_REG_CONFIG_STC   regDataArray[PRV_CPSS_PORT_NOT_APPLICABLE_E];
    PRV_CPSS_PORT_TYPE_ENT  portMacType; /* port MAC type */
    GT_U32                  portGroupId; /* port group id */
    GT_U32                  macCtrlReg4Addr;
    GT_U32                  idleCheck;
    GT_STATUS               rc;

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);


    if(PRV_CPSS_PX_PP_MAC(devNum)->genInfo.hitlessWriteMethodEnable)
    {
        GT_STATUS   rc;
        GT_BOOL     currentState;

        rc = cpssPxPortForceLinkPassEnableGet(devNum,portNum,&currentState);
        if(rc != GT_OK)
        {
            return rc;
        }
        if(state == currentState)
        {
            return GT_OK;
        }
    }

    value = BOOL2BIT_MAC(state);

    if(prvCpssPxPortMacConfigurationClear(regDataArray) != GT_OK)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_INIT_ERROR, LOG_ERROR_NO_MSG);

    PRV_CPSS_PX_PORT_AUTO_NEG_CTRL_REG_MAC(devNum,portMacNum,&regAddr);
    if(regAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        regDataArray[PRV_CPSS_PORT_GE_E].regAddr = regAddr;
        regDataArray[PRV_CPSS_PORT_GE_E].fieldData = value;
        regDataArray[PRV_CPSS_PORT_GE_E].fieldLength = 1;
        regDataArray[PRV_CPSS_PORT_GE_E].fieldOffset = 1;
    }

    PRV_CPSS_PX_PORT_MAC_CTRL_REG_MAC(devNum,portMacNum,PRV_CPSS_PORT_XG_E,&regAddr);
    if(regAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        regDataArray[PRV_CPSS_PORT_XG_E].regAddr = regAddr;
        regDataArray[PRV_CPSS_PORT_XG_E].fieldData = value;
        regDataArray[PRV_CPSS_PORT_XG_E].fieldLength = 1;
        regDataArray[PRV_CPSS_PORT_XG_E].fieldOffset = 3;
    }

    PRV_CPSS_PX_PORT_MAC_CTRL_REG_MAC(devNum,portMacNum,PRV_CPSS_PORT_XLG_E,&regAddr);
    if(regAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        regDataArray[PRV_CPSS_PORT_XLG_E].regAddr = regAddr;
        regDataArray[PRV_CPSS_PORT_XLG_E].fieldData = value;
        regDataArray[PRV_CPSS_PORT_XLG_E].fieldLength = 1;
        regDataArray[PRV_CPSS_PORT_XLG_E].fieldOffset = 3;
    }

    PRV_CPSS_PX_REG1_CG_CONVERTERS_CTRL0_REG_MAC(devNum,portMacNum,&regAddr);
    if(regAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        regDataArray[PRV_CPSS_PORT_CG_E].regAddr = regAddr;
        regDataArray[PRV_CPSS_PORT_CG_E].fieldData = value;
        regDataArray[PRV_CPSS_PORT_CG_E].fieldLength = 1;
        regDataArray[PRV_CPSS_PORT_CG_E].fieldOffset = 27;
    }

    /* Disable the idle_check_for_link bit #14 in XLG-MAC Control_Register4 */
    /* Enable this bit is mandatory for linkDown indication between 10G port to
       2.5G/20G/40G ports, but in Force Link Pass operation it should be disabled */
    portMacType = PRV_CPSS_PX_PORT_TYPE_MAC(devNum, portMacNum);
    portGroupId = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portMacNum);
    if(portMacType == PRV_CPSS_PORT_XLG_E)
    {
        PRV_CPSS_PX_PORT_MAC_CTRL4_REG_MAC(devNum, portMacNum, PRV_CPSS_PORT_XLG_E, &macCtrlReg4Addr);
        if (macCtrlReg4Addr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
        {
            idleCheck = (state == GT_TRUE) ? 0 : 1;
            if (prvCpssDrvHwPpPortGroupSetRegField(CAST_SW_DEVNUM(devNum), portGroupId, macCtrlReg4Addr, 14, 1, idleCheck) != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);
            }
        }
    }

    rc = prvCpssPxPortMacConfiguration(devNum, portNum, regDataArray);
    if(rc != GT_OK)
    {
        return rc;
    }
    return GT_OK;
}

/**
* @internal cpssPxPortForceLinkPassEnableSet function
* @endinternal
*
* @brief   Enable/disable Force Link Pass on specified port on specified device.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] state                    - GT_TRUE for force link pass, GT_FALSE otherwise
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortForceLinkPassEnableSet
(
    IN  GT_SW_DEV_NUM            devNum,
    IN  GT_PHYSICAL_PORT_NUM     portNum,
    IN  GT_BOOL                  state
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortForceLinkPassEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, state));

    rc = internal_cpssPxPortForceLinkPassEnableSet(devNum, portNum, state);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, state));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortForceLinkPassEnableGet function
* @endinternal
*
* @brief   Get Force Link Pass on specified port on specified device.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] statePtr                 - (ptr to) current force link pass state:
*                                      GT_TRUE for force link pass, GT_FALSE otherwise
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - NULL pointer in statePtr
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPortForceLinkPassEnableGet
(
    IN  GT_SW_DEV_NUM            devNum,
    IN  GT_PHYSICAL_PORT_NUM     portNum,
    OUT GT_BOOL                 *statePtr
)
{
    GT_U32 regAddr;         /* register address */
    GT_U32 value;           /* value to write into the register */
    GT_U32 offset;          /* bit number inside register       */
    GT_U32 portGroupId;     /*the port group Id - support multi-port-groups device */
    PRV_CPSS_PORT_TYPE_ENT portMacType; /* mac type of port */
    GT_U32 portMacNum; /* MAC number */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);
    CPSS_NULL_PTR_CHECK_MAC(statePtr);

    portMacType = PRV_CPSS_PX_PORT_TYPE_MAC(devNum,portMacNum);
    if (portMacType >= PRV_CPSS_PORT_XG_E)
    {
        if (portMacType == PRV_CPSS_PORT_CG_E)
        {
            PRV_CPSS_PX_REG1_CG_CONVERTERS_CTRL0_REG_MAC(devNum,portMacNum,&regAddr);
            offset =27;
        }
        else
        {
            PRV_CPSS_PX_PORT_MAC_CTRL_REG_MAC(devNum,portMacNum,portMacType,&regAddr);
            offset = 3;
        }
    }
    else
    {
        PRV_CPSS_PX_PORT_AUTO_NEG_CTRL_REG_MAC(devNum,portMacNum,&regAddr);
        offset = 1;
    }
    if(PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);

    /* get force link pass bit */
    portGroupId = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portMacNum);
    if(prvCpssDrvHwPpPortGroupGetRegField(CAST_SW_DEVNUM(devNum), portGroupId,regAddr, offset, 1, &value) != GT_OK)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);

    *statePtr = BIT2BOOL_MAC(value);
    return GT_OK;
}

/**
* @internal cpssPxPortForceLinkPassEnableGet function
* @endinternal
*
* @brief   Get Force Link Pass on specified port on specified device.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] statePtr                 - (ptr to) current force link pass state:
*                                      GT_TRUE for force link pass, GT_FALSE otherwise
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - NULL pointer in statePtr
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortForceLinkPassEnableGet
(
    IN  GT_SW_DEV_NUM            devNum,
    IN  GT_PHYSICAL_PORT_NUM     portNum,
    OUT GT_BOOL                 *statePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortForceLinkPassEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, statePtr));

    rc = internal_cpssPxPortForceLinkPassEnableGet(devNum, portNum, statePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, statePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal prvCpssPxPortForceLinkDownEnableSet function
* @endinternal
*
* @brief   Enable/disable Force Link Down on specified port on specified device.
*         Note: tx_rem_fault takes precedence over tx_loc_fault. for
*         CG mac, make sure to disable tx_rem_fault, otherwise
*         this function won't work.
*
* @note   APPLICABLE DEVICES:     Pipe.
* @note   NOT APPLICABLE DEVICES: None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number (or CPU port)
* @param[in] state                    - GT_TRUE for force link down, GT_FALSE otherwise
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssPxPortForceLinkDownEnableSet
(
    IN  GT_SW_DEV_NUM     devNum,
    IN  GT_PHYSICAL_PORT_NUM     portNum,
    IN  GT_BOOL   state
)
{
    GT_U32  forceLinkDownRegAddr; /* register address */
    GT_U32  value;                /* value to write into the register */
    GT_U32  portMacNum; /* MAC number */
    PRV_CPSS_PORT_REG_CONFIG_STC   regDataArray[PRV_CPSS_PORT_NOT_APPLICABLE_E];
    GT_STATUS   rc;

    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

    if(PRV_CPSS_PX_PP_MAC(devNum)->genInfo.hitlessWriteMethodEnable)
    {
        GT_STATUS   rc;
        GT_BOOL     currentState;

        rc = cpssPxPortForceLinkDownEnableGet(devNum,portNum,&currentState);
        if(rc != GT_OK)
        {
            return rc;
        }
        if(state == currentState)
        {
            return GT_OK;
        }
    }

    value = BOOL2BIT_MAC(state);

    if(prvCpssPxPortMacConfigurationClear(regDataArray) != GT_OK)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_INIT_ERROR, LOG_ERROR_NO_MSG);

    PRV_CPSS_PX_PORT_AUTO_NEG_CTRL_REG_MAC(devNum, portMacNum, &forceLinkDownRegAddr);
    if(forceLinkDownRegAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        regDataArray[PRV_CPSS_PORT_GE_E].regAddr = forceLinkDownRegAddr;
        regDataArray[PRV_CPSS_PORT_GE_E].fieldData = value;
        regDataArray[PRV_CPSS_PORT_GE_E].fieldLength = 1;
        regDataArray[PRV_CPSS_PORT_GE_E].fieldOffset = 0;
    }

    PRV_CPSS_PX_PORT_MAC_CTRL_REG_MAC(devNum, portMacNum, PRV_CPSS_PORT_XG_E,
                                                        &forceLinkDownRegAddr);
    if(forceLinkDownRegAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        regDataArray[PRV_CPSS_PORT_XG_E].regAddr = forceLinkDownRegAddr;
        regDataArray[PRV_CPSS_PORT_XG_E].fieldData = value;
        regDataArray[PRV_CPSS_PORT_XG_E].fieldLength = 1;
        regDataArray[PRV_CPSS_PORT_XG_E].fieldOffset = 2;
    }

    PRV_CPSS_PX_PORT_MAC_CTRL_REG_MAC(devNum, portMacNum, PRV_CPSS_PORT_XLG_E,
                                                        &forceLinkDownRegAddr);
    if(forceLinkDownRegAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        regDataArray[PRV_CPSS_PORT_XLG_E].regAddr = forceLinkDownRegAddr;
        regDataArray[PRV_CPSS_PORT_XLG_E].fieldData = value;
        regDataArray[PRV_CPSS_PORT_XLG_E].fieldLength = 1;
        regDataArray[PRV_CPSS_PORT_XLG_E].fieldOffset = 2;
    }

    /* WA for CG port type, in CG there is no force link down.
    Instead, using Local Fault bit in Control 0 register.
    Note: tx_rem_fault takes precedence over tx_loc_fault, make sure to disable tx_rem_fault to use tx_loc_fault bit.
     */
    PRV_CPSS_PX_REG1_CG_CONVERTERS_CTRL0_REG_MAC(devNum,portMacNum, &forceLinkDownRegAddr);
    if(forceLinkDownRegAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        regDataArray[PRV_CPSS_PORT_CG_E].regAddr = forceLinkDownRegAddr;
        regDataArray[PRV_CPSS_PORT_CG_E].fieldData = value;
        regDataArray[PRV_CPSS_PORT_CG_E].fieldLength = 1;
        regDataArray[PRV_CPSS_PORT_CG_E].fieldOffset = 0;
    }

    rc = prvCpssPxPortMacConfiguration(devNum, portNum, regDataArray);
    if(rc != GT_OK)
    {
        return rc;
    }
    CPSS_PORTS_BMP_PORT_ENABLE_MAC(&PRV_CPSS_PX_PP_MAC(devNum)->port.portForceLinkDownBmp, portNum, state);

    return GT_OK;
}
/**
* @internal internal_cpssPxPortForceLinkDownEnableSet function
* @endinternal
*
* @brief   Enable/disable Force Link Down on specified port on specified device.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] state                    - GT_TRUE for force link down, GT_FALSE otherwise
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPortForceLinkDownEnableSet
(
    IN  GT_SW_DEV_NUM            devNum,
    IN  GT_PHYSICAL_PORT_NUM     portNum,
    IN  GT_BOOL   state
)
{
    GT_U32      portMacNum; /* MAC number */
    GT_STATUS   rc;
    CPSS_PORT_INTERFACE_MODE_ENT ifMode;/* current interface of port */
    GT_BOOL     apEnabled = GT_FALSE;
    CPSS_PX_PORT_AP_PARAMS_STC apParams;

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

    ifMode = PRV_CPSS_PX_PORT_IFMODE_MAC(devNum, portMacNum);

    /* check if AP is enabled */
    rc = cpssPxPortApEnableGet(devNum, &apEnabled);
    if (rc != GT_OK)
    {
        return rc;
    }

    if (apEnabled)
    {
        /* check if AP is enabled on specific port */
        rc = cpssPxPortApPortConfigGet(CAST_SW_DEVNUM(devNum), portNum, &apEnabled, &apParams);
        if (rc != GT_OK)
        {
            return rc;
        }

        if (apEnabled && (ifMode == CPSS_PORT_INTERFACE_MODE_1000BASE_X_E))
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, "This api does not support force link down on an AP port");
        }
    }

    return prvCpssPxPortForceLinkDownEnableSet(devNum, portNum, state);
}

/**
* @internal cpssPxPortForceLinkDownEnableSet function
* @endinternal
*
* @brief   Enable/disable Force Link Down on specified port on specified device.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] state                    - GT_TRUE for force link down, GT_FALSE otherwise
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortForceLinkDownEnableSet
(
    IN  GT_SW_DEV_NUM            devNum,
    IN  GT_PHYSICAL_PORT_NUM     portNum,
    IN  GT_BOOL                  state
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortForceLinkDownEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, state));

    rc = internal_cpssPxPortForceLinkDownEnableSet(devNum, portNum, state);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, state));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortForceLinkDownEnableGet function
* @endinternal
*
* @brief   Get Force Link Down on specified port on specified device.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] statePtr                 - (ptr to) current force link down state:
*                                      GT_TRUE for force link down, GT_FALSE otherwise
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - NULL pointer in statePtr
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPortForceLinkDownEnableGet
(
    IN  GT_SW_DEV_NUM            devNum,
    IN  GT_PHYSICAL_PORT_NUM     portNum,
    OUT GT_BOOL                 *statePtr
)
{
    GT_STATUS       rc;                     /* return code */
    GT_U32          forceLinkDownRegAddr; /* register address */
    GT_U32          value;                /* value to write into the register */
    GT_U32          forceLinkDownOffset;  /* bit number inside register       */
    PRV_CPSS_PORT_TYPE_ENT portMacType;
    GT_U32          portMacNum; /* MAC number */
    GT_BOOL         apEnabled = GT_FALSE;
    CPSS_PX_PORT_AP_PARAMS_STC apParams;
    GT_BOOL         isCgMac;
    CPSS_SYSTEM_RECOVERY_INFO_STC tempSystemRecovery_Info;

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);
    CPSS_NULL_PTR_CHECK_MAC(statePtr);

    /* check if port is AP port */
    rc = cpssPxPortApPortConfigGet(CAST_SW_DEVNUM(devNum), portNum, &apEnabled, &apParams);
    if ((rc != GT_OK) && (rc != GT_BAD_PARAM) && (rc != GT_NOT_APPLICABLE_DEVICE))
    {
        return rc;
    }

    if (apEnabled)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, "This api does not support force link down on an AP port");
    }

    /* Get the port type */
    portMacType = PRV_CPSS_PX_PORT_TYPE_MAC(devNum,portMacNum);
    isCgMac = (portMacType == PRV_CPSS_PORT_CG_E);
    if (isCgMac)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }

    rc = cpssSystemRecoveryStateGet(&tempSystemRecovery_Info);
    if (rc != GT_OK)
    {
        return rc;
    }
    if(CPSS_SYSTEM_RECOVERY_PROCESS_NOT_ACTIVE_E == tempSystemRecovery_Info.systemRecoveryProcess)
    {/* read from SW DB only if not during some recovery */
        *statePtr = CPSS_PORTS_BMP_IS_PORT_SET_MAC(&PRV_CPSS_PX_PP_MAC(devNum)->port.portForceLinkDownBmp,portNum);
        return GT_OK;
    }

    /* In CG unit we do not have force link down ability */
    if ( portMacType == PRV_CPSS_PORT_CG_E)
    {
        *statePtr = GT_FALSE;
    }
    else
    {
        /* for 10 Gb interface and up */
        if(portMacType >= PRV_CPSS_PORT_XG_E)
        {
            PRV_CPSS_PX_PORT_MAC_CTRL_REG_MAC(devNum,portMacNum, portMacType, &forceLinkDownRegAddr);
            forceLinkDownOffset = 2;
        }
        else
        {
            PRV_CPSS_PX_PORT_AUTO_NEG_CTRL_REG_MAC(devNum, portMacNum, &forceLinkDownRegAddr);
            forceLinkDownOffset = 0;
        }
        if(PRV_CPSS_SW_PTR_ENTRY_UNUSED == forceLinkDownRegAddr)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);

        if (prvCpssDrvHwPpPortGroupGetRegField(CAST_SW_DEVNUM(devNum), CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                                forceLinkDownRegAddr, forceLinkDownOffset, 1, &value) != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);
        }
        *statePtr = BIT2BOOL_MAC(value);
    }
    return GT_OK;
}

/**
* @internal cpssPxPortForceLinkDownEnableGet function
* @endinternal
*
* @brief   Get Force Link Down on specified port on specified device.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] statePtr                 - (ptr to) current force link down state:
*                                      GT_TRUE for force link down, GT_FALSE otherwise
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - NULL pointer in statePtr
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortForceLinkDownEnableGet
(
    IN  GT_SW_DEV_NUM            devNum,
    IN  GT_PHYSICAL_PORT_NUM     portNum,
    OUT GT_BOOL                 *statePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortForceLinkDownEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, statePtr));

    rc = internal_cpssPxPortForceLinkDownEnableGet(devNum, portNum, statePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, statePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/*******************************************************************************
* internal_cpssPxPortMruSet
*
* DESCRIPTION:
*       Sets the Maximal Receive Packet size for specified port
*       on specified device.
*
* APPLICABLE DEVICES:
*        Pipe.
*
* NOT APPLICABLE DEVICES:
*        None.
*
*
* INPUTS:
*       devNum     - physical device number
*       portNum    - physical port number
*       mruSize    - max receive packet size in bytes.
*                    (APPLICABLE RANGES: 0..10304) - jumbo frame + 64 (including
*                    4 bytes CRC). Value must be even.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - on wrong port number or device or
*                          odd value of mruSize
*       GT_HW_ERROR              - on hardware error
*       GT_OUT_OF_RANGE          - mruSize > 10304
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*
*******************************************************************************/
#define PRV_CPSS_PX_PORT_MRU_FIELD_LENGTH_CNS 13
#define PRV_CPSS_PX_PORT_MRU_FIELD_LENGTH_CG_CNS 16
#define PRV_CPSS_PX_PORT_MRU_FIELD_OFFSET_GE_CNS 2
#define PRV_CPSS_PX_PORT_MRU_FIELD_OFFSET_XG_CNS 0
#define PRV_CPSS_PX_PORT_MRU_FIELD_OFFSET_CG_CNS 0
static GT_STATUS internal_cpssPxPortMruSet
(
    IN  GT_SW_DEV_NUM         devNum,
    IN  GT_PHYSICAL_PORT_NUM  portNum,
    IN  GT_U32                mruSize
)
{
    GT_STATUS rc = GT_OK;
    GT_U32  portMacNum;      /* MAC number */
    GT_U32  valueGranularity1B;
    GT_U32  valueGranularity2B;
    GT_U32  regAddr;
    PRV_CPSS_PORT_REG_CONFIG_STC   regDataArray[PRV_CPSS_PORT_NOT_APPLICABLE_E];

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum,portMacNum);

    /* The resolution of this field is 2 bytes and
     * the default value is 1522 bytes => 0x2f9
     */
    if(mruSize & 1)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "MRU must be 'even' value but got [%d]",mruSize);
    }

    /* 10KB + 64 max frame size supported */
    CPSS_DATA_CHECK_MAX_MAC(mruSize, (CPSS_PX_PORT_MAX_MRU_CNS+1))


    /* In units prior to CG, the MRU size is in granularity of 2B, so the requested number
     should be devided by two before written to register. In CG unit, the granularity is 1
     so the size should be as requested. */
    valueGranularity2B = mruSize >> 1;
    valueGranularity1B = mruSize;

    if(prvCpssPxPortMacConfigurationClear(regDataArray) != GT_OK)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_INIT_ERROR, LOG_ERROR_NO_MSG);

    PRV_CPSS_PX_PORT_MAC_CTRL_REG_MAC(devNum, portMacNum, PRV_CPSS_PORT_GE_E,
                                                                        &regAddr);
    if(regAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        regDataArray[PRV_CPSS_PORT_GE_E].regAddr = regAddr;
        regDataArray[PRV_CPSS_PORT_GE_E].fieldOffset = PRV_CPSS_PX_PORT_MRU_FIELD_OFFSET_GE_CNS;
        regDataArray[PRV_CPSS_PORT_GE_E].fieldLength = PRV_CPSS_PX_PORT_MRU_FIELD_LENGTH_CNS;
        regDataArray[PRV_CPSS_PORT_GE_E].fieldData = valueGranularity2B;
    }

    PRV_CPSS_PX_PORT_MAC_CTRL1_REG_MAC(devNum, portMacNum, PRV_CPSS_PORT_XG_E,
                                                                        &regAddr);
    if(regAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        regDataArray[PRV_CPSS_PORT_XG_E].regAddr = regAddr;
        regDataArray[PRV_CPSS_PORT_XG_E].fieldOffset = PRV_CPSS_PX_PORT_MRU_FIELD_OFFSET_XG_CNS;
        regDataArray[PRV_CPSS_PORT_XG_E].fieldLength = PRV_CPSS_PX_PORT_MRU_FIELD_LENGTH_CNS;
        regDataArray[PRV_CPSS_PORT_XG_E].fieldData = valueGranularity2B;
    }

    PRV_CPSS_PX_PORT_MAC_CTRL1_REG_MAC(devNum, portMacNum, PRV_CPSS_PORT_XLG_E,
                                                                        &regAddr);
    if(regAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        regDataArray[PRV_CPSS_PORT_XLG_E].regAddr = regAddr;
        regDataArray[PRV_CPSS_PORT_XLG_E].fieldOffset = PRV_CPSS_PX_PORT_MRU_FIELD_OFFSET_XG_CNS;
        regDataArray[PRV_CPSS_PORT_XLG_E].fieldLength = PRV_CPSS_PX_PORT_MRU_FIELD_LENGTH_CNS;
        regDataArray[PRV_CPSS_PORT_XLG_E].fieldData = valueGranularity2B;
    }

    PRV_CPSS_PX_REG1_CG_PORT_MAC_FRM_LNGTH_REG_MAC(devNum, portMacNum, &regAddr);
    if(regAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        regDataArray[PRV_CPSS_PORT_CG_E].regAddr = regAddr;
        regDataArray[PRV_CPSS_PORT_CG_E].fieldOffset = PRV_CPSS_PX_PORT_MRU_FIELD_OFFSET_CG_CNS;
        regDataArray[PRV_CPSS_PORT_CG_E].fieldLength = PRV_CPSS_PX_PORT_MRU_FIELD_LENGTH_CG_CNS;
        regDataArray[PRV_CPSS_PORT_CG_E].fieldData = valueGranularity1B;
    }

    rc = prvCpssPxPortMacConfiguration(devNum, portNum, regDataArray);

    return rc;
}

/**
* @internal cpssPxPortMruSet function
* @endinternal
*
* @brief   Sets the Maximal Receive Packet size for specified port
*         on specified device.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] mruSize                  - max receive packet size in bytes.
*                                      (APPLICABLE RANGES: 0..10304) - jumbo frame + 64 (including
*                                      4 bytes CRC). Value must be even.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device or
*                                       odd value of mruSize
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - mruSize > 10304
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortMruSet
(
    IN  GT_SW_DEV_NUM         devNum,
    IN  GT_PHYSICAL_PORT_NUM  portNum,
    IN  GT_U32                mruSize
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortMruSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, mruSize));

    rc = internal_cpssPxPortMruSet(devNum, portNum, mruSize);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, mruSize));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortMruGet function
* @endinternal
*
* @brief   Gets the Maximal Receive Packet size for specified port
*         on specified device.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] mruSizePtr               - (pointer to) max receive packet size in bytes. 10K+64 (including 4 bytes CRC)
*                                      value must be even
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device or
*                                       odd value of mruSize
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - mruSize > 10304
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPortMruGet
(
    IN  GT_SW_DEV_NUM         devNum,
    IN  GT_PHYSICAL_PORT_NUM  portNum,
    OUT GT_U32                *mruSizePtr
)
{
    GT_STATUS rc = GT_OK;
    GT_U32  portMacNum;      /* MAC number */
    GT_U32 value;           /* value to read from the register  */
    GT_U32 offset;          /* bit number inside register       */
    GT_U32 regAddr;         /* register address                 */
    PRV_CPSS_PORT_TYPE_ENT portMacType;

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum,portMacNum);
    CPSS_NULL_PTR_CHECK_MAC(mruSizePtr);

    /* cpssPxPortMruSet updates all available MAC's - here enough to read */
    /* one of them */
    portMacType = PRV_CPSS_PX_PORT_TYPE_MAC(devNum,portMacNum);

    if(portMacType >= PRV_CPSS_PORT_XG_E)
    {
        if (portMacType == PRV_CPSS_PORT_CG_E)
        {
            PRV_CPSS_PX_REG1_CG_PORT_MAC_FRM_LNGTH_REG_MAC(devNum, portMacNum, &regAddr);
            offset = PRV_CPSS_PX_PORT_MRU_FIELD_OFFSET_CG_CNS;
        }
        else
        {
            PRV_CPSS_PX_PORT_MAC_CTRL1_REG_MAC(devNum, portMacNum, portMacType, &regAddr);
            offset = PRV_CPSS_PX_PORT_MRU_FIELD_OFFSET_XG_CNS;
        }
    }
    else
    {
        PRV_CPSS_PX_PORT_MAC_CTRL_REG_MAC(devNum,portMacNum,portMacType,&regAddr);
        offset = PRV_CPSS_PX_PORT_MRU_FIELD_OFFSET_GE_CNS;
    }
    if(PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);

    if (portMacType == PRV_CPSS_PORT_CG_E)
    {
        mvHwsCgMac28nmAccessLock(CAST_SW_DEVNUM(devNum), portMacNum);
        rc = prvCpssDrvHwPpPortGroupGetRegField(CAST_SW_DEVNUM(devNum),
                PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(CAST_SW_DEVNUM(devNum), portMacNum),
                PRV_PX_REG1_UNIT_CG_CONVERTERS_MAC(CAST_SW_DEVNUM(devNum), portMacNum).CGMAConvertersResets,
                26, 1, &value);
        if (0 == value)
        {/* if CG MAC in reset, return HW default, because access to MAC at this
            time causes hang */
            *mruSizePtr = 0x600;
        }
        else
        {
            rc = prvCpssDrvHwPpPortGroupGetRegField(CAST_SW_DEVNUM(devNum),
                    PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(CAST_SW_DEVNUM(devNum), portMacNum),
                regAddr, offset, PRV_CPSS_PX_PORT_MRU_FIELD_LENGTH_CG_CNS, &value);
            /*  In CG unit, the granularity of MRU size is 1 so no further handling
            is needed on this value. */
            *mruSizePtr = value;
        }
        mvHwsCgMac28nmAccessUnlock(CAST_SW_DEVNUM(devNum), portMacNum);
    }
    else
    {
        rc = prvCpssDrvHwPpPortGroupGetRegField(CAST_SW_DEVNUM(devNum),
            PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(CAST_SW_DEVNUM(devNum),portMacNum),
            regAddr, offset, PRV_CPSS_PX_PORT_MRU_FIELD_LENGTH_CNS, &value);
        /* In units prior to CG, the MRU size is in granularity of 2B, so the value in the
        register was devided by two before written to register and need to be multiplied
        by two upon import */
        *mruSizePtr = value<<1;
    }

    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);
    }

    return rc;
}

/**
* @internal cpssPxPortMruGet function
* @endinternal
*
* @brief   Gets the Maximal Receive Packet size for specified port
*         on specified device.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] mruSizePtr               - (pointer to) max receive packet size in bytes. 10K+64 (including 4 bytes CRC)
*                                      value must be even
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device or
*                                       odd value of mruSize
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - mruSize > 10304
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortMruGet
(
    IN  GT_SW_DEV_NUM         devNum,
    IN  GT_PHYSICAL_PORT_NUM  portNum,
    OUT GT_U32                *mruSizePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortMruGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, mruSizePtr));

    rc = internal_cpssPxPortMruGet(devNum, portNum, mruSizePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, mruSizePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortLinkStatusGet function
* @endinternal
*
* @brief   Gets Link Status of specified port on specified device.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] isLinkUpPtr              - GT_TRUE for link up, GT_FALSE otherwise
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPortLinkStatusGet
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_BOOL                 *isLinkUpPtr
)
{
    GT_U32 portGroupId;
    GT_U32 portMacNum; /* MAC number */
    GT_STATUS   rc; /* return code */
    GT_U32      phyPortNum; /* port number in local core */
    MV_HWS_PORT_STANDARD    portMode;
    CPSS_PORT_INTERFACE_MODE_ENT    cpssIfMode;
    CPSS_PORT_SPEED_ENT             cpssSpeed;
#ifdef TBD_PIPE_COMBO
    CPSS_DXCH_PORT_COMBO_PARAMS_STC *comboParamsPtr;
#endif

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum,portMacNum);
    CPSS_NULL_PTR_CHECK_MAC(isLinkUpPtr);

    portGroupId = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portMacNum);
    cpssIfMode = PRV_CPSS_PX_PORT_IFMODE_MAC(devNum, portMacNum);
    cpssSpeed = PRV_CPSS_PX_PORT_SPEED_MAC(devNum, portMacNum);

    if((CPSS_PORT_INTERFACE_MODE_NA_E == cpssIfMode) ||
       (CPSS_PORT_SPEED_NA_E == cpssSpeed))
    {
        *isLinkUpPtr = GT_FALSE;
        return GT_OK;
    }

    portMode = NON_SUP_MODE;
    phyPortNum = portMacNum;
#ifdef TBD_PIPE_COMBO
    comboParamsPtr = &PRV_CPSS_PX_PP_MAC(devNum)->port.comboPortsInfoArray[portMacNum];

    if(comboParamsPtr->macArray[0].macNum != CPSS_DXCH_PORT_COMBO_NA_MAC_CNS)
    { /* if it's combo port */
        CPSS_DXCH_PORT_MAC_PARAMS_STC   mac;
        rc = cpssPxPortComboPortActiveMacGet(devNum, portNum, &mac);
        if(rc != GT_OK)
            return rc;
        if (mac.macNum != phyPortNum)
        {
            switch(cpssIfMode)
            {
                case CPSS_PORT_INTERFACE_MODE_SR_LR_E:
                case CPSS_PORT_INTERFACE_MODE_KR_E:
                    portMode = _100GBase_KR10;
                    break;
                case CPSS_PORT_INTERFACE_MODE_SGMII_E:
                    portMode = QSGMII;
                    break;
                case CPSS_PORT_INTERFACE_MODE_1000BASE_X_E:
                    portMode = _100Base_FX;
                    break;
                default:
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
            }
        }
    }
#endif
    if (NON_SUP_MODE == portMode)
    {
        rc = prvCpssCommonPortIfModeToHwsTranslate(devNum,
                                    PRV_CPSS_PX_PORT_IFMODE_MAC(devNum, portMacNum),
                                    PRV_CPSS_PX_PORT_SPEED_MAC(devNum, portMacNum),
                                    &portMode);
        if(rc != GT_OK)
            return rc;
    }
    CPSS_LOG_INFORMATION_MAC("Calling: mvHwsPortLinkStatusGet(devNum[%d], portGroupId[%d], phyPortNum[%d], portMode[%d], *isLinkUpPtr)",devNum, portGroupId, phyPortNum, portMode);
    rc = mvHwsPortLinkStatusGet(devNum, portGroupId, phyPortNum, portMode,
                                isLinkUpPtr);
    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"mvHwsPortLinkStatusGet : Hws Failed");
    }

    return GT_OK;
}

/**
* @internal cpssPxPortLinkStatusGet function
* @endinternal
*
* @brief   Gets Link Status of specified port on specified device.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] isLinkUpPtr              - GT_TRUE for link up, GT_FALSE otherwise
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortLinkStatusGet
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_BOOL                 *isLinkUpPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortLinkStatusGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, isLinkUpPtr));

    rc = internal_cpssPxPortLinkStatusGet(devNum, portNum, isLinkUpPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, isLinkUpPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssPxPortInternalLoopbackEnableSet function
* @endinternal
*
* @brief   Set the internal Loopback state in the packet processor MAC port.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] enable                   - If GT_TRUE,  loopback
*                                      If GT_FALSE, disable loopback
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPortInternalLoopbackEnableSet
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_BOOL                 enable
)
{
    GT_U32 portMacNum;

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

    return prvCpssPxPortInternalLoopbackEnableSet(devNum, portNum, enable);
}

/**
* @internal cpssPxPortInternalLoopbackEnableSet function
* @endinternal
*
* @brief   Set the internal Loopback state in the packet processor MAC port.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] enable                   - If GT_TRUE,  loopback
*                                      If GT_FALSE, disable loopback
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortInternalLoopbackEnableSet
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_BOOL                 enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortInternalLoopbackEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enable));

    rc = internal_cpssPxPortInternalLoopbackEnableSet(devNum, portNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortInternalLoopbackEnableGet function
* @endinternal
*
* @brief   Get the internal Loopback state in the packet processor MAC port.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] enablePtr                - Pointer to the Loopback state.
*                                      If GT_TRUE, loopback is enabled
*                                      If GT_FALSE, loopback is disabled
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPortInternalLoopbackEnableGet
(
    IN  GT_SW_DEV_NUM        devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_BOOL              *enablePtr
)
{
    GT_STATUS               status;      /* execution status */
    GT_U32                  portGroupId; /* core number of port */
    GT_U32                  portMacNum;  /* MAC number */
    MV_HWS_PORT_STANDARD    portMode;    /* port interface in HWS format */
    MV_HWS_PORT_LB_TYPE     lbType;      /* loopback type in HWS format */
    MV_HWS_UNIT             lpPlace;     /* which unit to read for lb state */
    CPSS_PORT_INTERFACE_MODE_ENT ifMode; /* current interface of port */
    CPSS_PORT_SPEED_ENT          speed;  /* current speed of port */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);
    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    *enablePtr = GT_FALSE;
    portGroupId = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portMacNum);

    lpPlace = HWS_MAC;
    ifMode = PRV_CPSS_PX_PORT_IFMODE_MAC(devNum, portMacNum);
    speed = PRV_CPSS_PX_PORT_SPEED_MAC(devNum, portMacNum);
    if((CPSS_PORT_INTERFACE_MODE_NA_E == ifMode) || (CPSS_PORT_SPEED_NA_E == speed))
    {/* loopback on port which interface not defined yet - forbidden */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    status = prvCpssCommonPortIfModeToHwsTranslate(devNum,ifMode, speed, &portMode);
    if(status != GT_OK)
    {
        return status;
    }
    /* on CG mac the loopback is configured over the PCS*/
    if(PRV_CPSS_PORT_CG_E == PRV_CPSS_PX_PORT_TYPE_MAC(devNum,portMacNum))
    {
        lpPlace = HWS_PCS;
    }
    CPSS_LOG_INFORMATION_MAC("Calling: mvHwsPortLoopbackStatusGet(devNum[%d], portGroupId[%d], phyPortNum[%d], portMode[%d], lpPlace[%d], *lbType)", devNum, portGroupId, portMacNum, portMode, lpPlace);
    status = mvHwsPortLoopbackStatusGet(devNum, portGroupId, portMacNum, portMode,
                                        lpPlace, &lbType);
    if (status != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(status,"mvHwsPortLoopbackStatusGet : Hws Failed");
    }

    *enablePtr = (TX_2_RX_LB == lbType);
    return GT_OK;
}

/**
* @internal cpssPxPortInternalLoopbackEnableGet function
* @endinternal
*
* @brief   Get the internal Loopback state in the packet processor MAC port.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] enablePtr                - Pointer to the Loopback state.
*                                      If GT_TRUE, loopback is enabled
*                                      If GT_FALSE, loopback is disabled
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortInternalLoopbackEnableGet
(
    IN  GT_SW_DEV_NUM        devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_BOOL              *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortInternalLoopbackEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enablePtr));

    rc = internal_cpssPxPortInternalLoopbackEnableGet(devNum, portNum, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal prvCpssPxPortDisableWait function
* @endinternal
*
* @brief   Wait for queues empty on disable port.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number, CPU port number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS prvCpssPxPortDisableWait
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum
)
{
    GT_STATUS   rc;         /* return code */
    GT_U32      regAddr;    /* register address */
    GT_U32      portMacNum; /* MAC number */
    CPSS_PX_DETAILED_PORT_MAP_STC *portMapShadowPtr;

    GT_U16      portTxqDescNum = (GT_U16) -1; /* number of not treated TXQ descriptors */
    GT_U16      prevPortTxqDescNum; /* previous number of not treated TXQ descriptors */
    GT_U32      timeout;        /* resources free timeout counter */
    GT_U32      portTxdmaNum;   /* TXDMA allocated for port */
    GT_U32      scdmaTxFifoCounters;    /* value of both Header(bits 10-19)
                                and Payload(bits 0-9) scdma TxFifo Counters */
    GT_U8       tcQueue;
    GT_BOOL     queueEn;
    GT_U32      portGroupId;/* group number of port (in multi-port-group device)*/
    CPSS_SYSTEM_RECOVERY_INFO_STC tempSystemRecovery_Info;

    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

    /* ensure last packets passed pipe */
    rc = prvCpssPxPortPhysicalPortMapShadowDBGet(devNum, portNum, &portMapShadowPtr);
    if(rc != GT_OK)
    {
        return rc;
    }

    portGroupId = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portMacNum);
    if (portMapShadowPtr->portMap.txqNum != GT_PX_NA) /* extender cascade port has no TXQ (GT_NA in TXQ at mapping) therefore no sense in test */
    {
        for (tcQueue = 0; tcQueue < CPSS_TC_RANGE_CNS; tcQueue++)
        {
            rc = cpssPxPortTxDebugQueueTxEnableGet(devNum, portNum, tcQueue,
                                                &queueEn);
            if(rc != GT_OK)
            {
                return rc;
            }
            if (!queueEn)
            {/* if at least one of queues disabled, exit without wait,
                we suppose that user debugging system and descriptors left in
                pipe no matter */
                return GT_OK;
            }
        }

        for(timeout = 100; timeout > 0; timeout--)
        {
            do
            {
                prevPortTxqDescNum = portTxqDescNum;
                rc = cpssPxPortTxDebugDescNumberGet(devNum, portNum, &portTxqDescNum);
                if(rc != GT_OK)
                {
                    return rc;
                }

                if(0 == portTxqDescNum)
                {
                    break;
                }
                else
                {
                    cpssOsTimerWkAfter(10);
                }
            } while (portTxqDescNum < prevPortTxqDescNum);
            if(0 == portTxqDescNum)
            {
                break;
            }

            if(0 == timeout)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_TIMEOUT, LOG_ERROR_NO_MSG);
            }
        }
    }
    rc = cpssSystemRecoveryStateGet(&tempSystemRecovery_Info);
    if (rc != GT_OK)
    {
        return rc;
    }

    if(tempSystemRecovery_Info.systemRecoveryProcess != CPSS_SYSTEM_RECOVERY_PROCESS_HA_E)
    {
        rc = prvCpssPxPortPhysicalPortMapCheckAndConvert(devNum, portNum,
                                                           PRV_CPSS_PX_PORT_TYPE_TxDMA_E,
                                                           &portTxdmaNum);
        if(GT_OK != rc)
        {
            return rc;
        }
        regAddr = PRV_PX_REG1_UNIT_TXDMA_MAC(devNum).txDMADebug.
                                informativeDebug.SCDMAStatusReg1[portTxdmaNum];
        for(timeout = 100; timeout > 0; timeout--)
        {
            rc = prvCpssHwPpPortGroupGetRegField(CAST_SW_DEVNUM(devNum), portGroupId, regAddr, 0,
                                                 20, &scdmaTxFifoCounters);
            if(rc != GT_OK)
            {
                return rc;
            }
#ifdef GM_USED
            /* the registers don't exist in GM ... and return '0xbadad' */
            scdmaTxFifoCounters = 0;
#endif /*GM_USED*/
            if(0 == scdmaTxFifoCounters)
                break;
            else
                cpssOsTimerWkAfter(10);
        }

        if(0 == timeout)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_TIMEOUT, LOG_ERROR_NO_MSG);
        }
    }

    return GT_OK;
}

/**
* @internal prvCpssPxPortEnableSet function
* @endinternal
*
* @brief   Enable/disable a specified port on specified device.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number, CPU port number
* @param[in] enable                   - GT_TRUE:   port,
*                                      GT_FALSE: disable port.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssPxPortEnableSet
(
    IN  GT_SW_DEV_NUM     devNum,
    IN  GT_PHYSICAL_PORT_NUM     portNum,
    IN  GT_BOOL   enable
)
{
    GT_U32      regAddr;    /* register address */
    GT_U32      resetData, resetMask, value;      /* value to write */
    GT_STATUS   rc;         /* return code */
    GT_STATUS   rc1 = GT_OK;    /* alternative return code */
    GT_U32      portMacNum; /* MAC number */
    PRV_CPSS_PORT_REG_CONFIG_STC regDataArray[PRV_CPSS_PORT_NOT_APPLICABLE_E];
    GT_U8       tcQueue;
    GT_BOOL     isPortTxShaperEnable = GT_FALSE;
    GT_BOOL     isPortTxQShaperEnable[CPSS_TC_RANGE_CNS] = {GT_FALSE};
    GT_BOOL     isPortTxShaperAvbEnable = GT_FALSE;
    GT_BOOL     isPortTxQShaperAvbEnable[CPSS_TC_RANGE_CNS] = {GT_FALSE};
    GT_BOOL     reset;

    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

    if(enable)
    {/* Unreset PTP before port state is enabled */
#ifndef TBD_PIPE_PTP /*to remove when ptp is supported*/
        /*rc = prvCpssPxPortPtpReset(devNum, portNum, GT_FALSE, GT_FALSE);*/
        reset = GT_FALSE;
        regAddr = PRV_PX_REG1_UNIT_GOP_PTP_MAC(devNum, portMacNum).PTPGeneralCtrl;
        resetData = 0;
        /* Any PP: bit1 - PTP reset */
        resetMask = 2;
        if (reset == GT_FALSE)
        {
            resetData |= 2;
        }
        /* PTP Rx reset - bit8 */
        resetMask |= 0x100;
        if (reset == GT_FALSE)
        {
            resetData |= 0x100;
        }
        /* Set PTP unit software reset bit/bits */
        rc = prvCpssDrvHwPpWriteRegBitMask(CAST_SW_DEVNUM(devNum), regAddr, resetMask, resetData);
        if(rc != GT_OK)
        {
            return rc;
        }
#endif
    }
    else
    {
        rc = cpssPxIngressPortTargetEnableSet(devNum,portNum,GT_FALSE);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    if(prvCpssPxPortMacConfigurationClear(regDataArray) != GT_OK)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_INIT_ERROR, LOG_ERROR_NO_MSG);

    value = BOOL2BIT_MAC(enable);

    PRV_CPSS_PX_PORT_MAC_CTRL_REG_MAC(devNum,portMacNum,PRV_CPSS_PORT_GE_E,&regAddr);
    if(regAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        regDataArray[PRV_CPSS_PORT_GE_E].regAddr = regAddr;
        regDataArray[PRV_CPSS_PORT_GE_E].fieldData = value;
        regDataArray[PRV_CPSS_PORT_GE_E].fieldOffset = 0;
        regDataArray[PRV_CPSS_PORT_GE_E].fieldLength = 1;
    }

    PRV_CPSS_PX_PORT_MAC_CTRL_REG_MAC(devNum,portMacNum,PRV_CPSS_PORT_XG_E,&regAddr);
    if(regAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        regDataArray[PRV_CPSS_PORT_XG_E].regAddr = regAddr;
        regDataArray[PRV_CPSS_PORT_XG_E].fieldData = value;
        regDataArray[PRV_CPSS_PORT_XG_E].fieldOffset = 0;
        regDataArray[PRV_CPSS_PORT_XG_E].fieldLength = 1;
    }

    PRV_CPSS_PX_PORT_MAC_CTRL_REG_MAC(devNum,portMacNum,PRV_CPSS_PORT_XLG_E,&regAddr);
    if(regAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        regDataArray[PRV_CPSS_PORT_XLG_E].regAddr = regAddr;
        regDataArray[PRV_CPSS_PORT_XLG_E].fieldData = value;
        regDataArray[PRV_CPSS_PORT_XLG_E].fieldOffset = 0;
        regDataArray[PRV_CPSS_PORT_XLG_E].fieldLength = 1;
    }

    PRV_CPSS_PX_REG1_CG_CONVERTERS_CTRL0_REG_MAC(devNum,portMacNum,&regAddr);
    if(regAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        regDataArray[PRV_CPSS_PORT_CG_E].regAddr = regAddr;
        regDataArray[PRV_CPSS_PORT_CG_E].fieldData = value;
        regDataArray[PRV_CPSS_PORT_CG_E].fieldOffset = 20;
        regDataArray[PRV_CPSS_PORT_CG_E].fieldLength = 1;
    }

    rc = prvCpssPxPortMacConfiguration(devNum, portNum, regDataArray);
    if(rc != GT_OK)
    {
        return rc;
    }

    if(!enable)
    {
        /* Check which port shapers are enabled */
        rc1 = cpssPxPortTxShaperEnableGet(devNum,portNum,
                    &isPortTxShaperEnable,
                    &isPortTxShaperAvbEnable);
        /* Ignore rc1 */
        for (tcQueue = 0; tcQueue < CPSS_TC_RANGE_CNS; tcQueue++)
        {
            rc1 = cpssPxPortTxShaperQueueEnableGet(devNum,portNum,tcQueue,
                                                &isPortTxQShaperEnable[tcQueue],
                                                &isPortTxQShaperAvbEnable[tcQueue]);
            /* Ignore rc1 */
        }

        /* Disable port and per-TC shapers */
        if (isPortTxShaperEnable)
        {
            rc1 = cpssPxPortTxShaperEnableSet(devNum,portNum,
                            GT_FALSE, GT_FALSE);
            /* Ignore rc1 */
        }
        for (tcQueue = 0; tcQueue < CPSS_TC_RANGE_CNS; tcQueue++)
        {
            if (isPortTxQShaperEnable[tcQueue])
            {
                rc1 = cpssPxPortTxShaperQueueEnableSet(devNum,portNum,tcQueue,
                            GT_FALSE, GT_FALSE);
                /* Ignore rc1 */
            }
        }

        /* Wait for queues to empty */
        rc = prvCpssPxPortDisableWait(devNum, portNum);
        /* rc to be returned at end of function */

        /* Re-enable port and per-TC shapers - independent of rc/rc1 */
        for (tcQueue = 0; tcQueue < CPSS_TC_RANGE_CNS; tcQueue++)
        {
            if (isPortTxQShaperEnable[tcQueue])
            {
                rc1 = cpssPxPortTxShaperQueueEnableSet(devNum,portNum,tcQueue,
                            GT_TRUE, isPortTxQShaperAvbEnable[tcQueue]);
                /* Ignore rc1 */
            }
        }
        if (isPortTxShaperEnable)
        {
            rc1 = cpssPxPortTxShaperEnableSet(devNum,portNum,
                            GT_TRUE, isPortTxShaperAvbEnable);
            /* Ignore rc1 */
        }
    }

    if(!enable)
    {/* Reset PTP unit after port state is disabled */
#ifndef TBD_PIPE_PTP /*to remove when ptp is supported*/
        /*rc = prvCpssPxPortPtpReset(devNum, portNum, GT_TRUE, GT_TRUE);*/
        reset = GT_TRUE;
        regAddr = PRV_PX_REG1_UNIT_GOP_PTP_MAC(devNum, portMacNum).PTPGeneralCtrl;
        resetData = 0;
        /* Any PP: bit1 - PTP reset */
        resetMask = 2;
        if (reset == GT_FALSE)
        {
            resetData |= 2;
        }
        /* PTP Rx reset - bit8 */
        resetMask |= 0x100;
        if (reset == GT_FALSE)
        {
            resetData |= 0x100;
        }
        /* Set PTP unit software reset bit/bits */
        rc1 = prvCpssDrvHwPpWriteRegBitMask(CAST_SW_DEVNUM(devNum), regAddr, resetMask, resetData);
#endif
    }

    if (rc != GT_OK)
    {
        return rc;
    }
    else
    {
        return rc1;
    }
}

/**
* @internal internal_cpssPxPortEnableSet function
* @endinternal
*
* @brief   Enable/disable a specified port on specified device.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] enable                   - GT_TRUE:   port,
*                                      GT_FALSE: disable port.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPortEnableSet
(
    IN  GT_SW_DEV_NUM     devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  GT_BOOL   enable
)
{
    GT_STATUS rc = GT_OK;
    GT_U32 portMacNum; /* MAC number */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum,portMacNum);

    rc = prvCpssPxPortEnableSet(devNum, portNum, enable);
    if(rc!=GT_OK)
    {
        return rc;
    }

    return rc;
}
/**
* @internal cpssPxPortEnableSet function
* @endinternal
*
* @brief   Enable/disable a specified port on specified device.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] enable                   - GT_TRUE:   port,
*                                      GT_FALSE: disable port.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortEnableSet
(
    IN  GT_SW_DEV_NUM     devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  GT_BOOL   enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enable));

    rc = internal_cpssPxPortEnableSet(devNum, portNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal prvCpssPxPortEnableGet function
* @endinternal
*
* @brief   Get the Enable/Disable status of a specified port on specified device.
*
* @note   APPLICABLE DEVICES:     Pipe.
* @note   NOT APPLICABLE DEVICES: None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] statePtr                 - Pointer to the Get Enable/disable state of the port.
*                                      GT_TRUE for enabled port, GT_FALSE otherwise
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssPxPortEnableGet
(
    IN   GT_SW_DEV_NUM     devNum,
    IN   GT_PHYSICAL_PORT_NUM     portNum,
    OUT  GT_BOOL   *statePtr
)
{
    GT_U32 regAddr;
    GT_U32 value;
    GT_U32 portMacNum; /* MAC number */
    PRV_CPSS_PORT_TYPE_ENT macType;
    GT_STATUS rc = GT_OK;

    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

    macType = PRV_CPSS_PX_PORT_TYPE_MAC(devNum,portMacNum);

    /* store value of port state */
    if (macType == PRV_CPSS_PORT_CG_E)
    {
        PRV_CPSS_PX_REG1_CG_CONVERTERS_CTRL0_REG_MAC(devNum, portMacNum, &regAddr);
        if(PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);

        rc = prvCpssDrvHwPpPortGroupGetRegField(CAST_SW_DEVNUM(devNum),
                PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(CAST_SW_DEVNUM(devNum), portMacNum),
                    regAddr, 20, 1, &value);
    }
    else
    {
        PRV_CPSS_PX_PORT_MAC_CTRL_REG_MAC(devNum, portMacNum, macType, &regAddr);
        if(PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);

        rc = prvCpssDrvHwPpPortGroupGetRegField(CAST_SW_DEVNUM(devNum),
                PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portMacNum),
                    regAddr, 0, 1, &value);
    }
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);
    }

    *statePtr = (value == 1) ? GT_TRUE : GT_FALSE;
    return GT_OK;
}

/**
* @internal internal_cpssPxPortEnableGet function
* @endinternal
*
* @brief   Get the Enable/Disable status of a specified port on specified device.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] statePtr                 - Pointer to the Get Enable/disable state of the port.
*                                      GT_TRUE for enabled port, GT_FALSE otherwise
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPortEnableGet
(
    IN   GT_SW_DEV_NUM     devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT  GT_BOOL   *statePtr
)
{
    GT_STATUS rc = GT_OK;
    GT_U32 portMacNum; /* MAC number */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum,portMacNum);
    CPSS_NULL_PTR_CHECK_MAC(statePtr);

    rc = prvCpssPxPortEnableGet(devNum,portNum,statePtr);
    if(rc!=GT_OK)
    {
        return rc;
    }

    return GT_OK;
}

/**
* @internal cpssPxPortEnableGet function
* @endinternal
*
* @brief   Get the Enable/Disable status of a specified port on specified device.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] statePtr                 - Pointer to the Get Enable/disable state of the port.
*                                      GT_TRUE for enabled port, GT_FALSE otherwise
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortEnableGet
(
    IN   GT_SW_DEV_NUM     devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT  GT_BOOL   *statePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, statePtr));

    rc = internal_cpssPxPortEnableGet(devNum, portNum, statePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, statePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

GT_BOOL pipeDebugCpllInitFlag = GT_TRUE;

GT_VOID pipeDebugCpllInitFlagEnableSet()
{
    pipeDebugCpllInitFlag = GT_TRUE;

    return;
}

GT_VOID pipeDebugCpllInitFlagDisableSet()
{
    pipeDebugCpllInitFlag = GT_FALSE;

    return;
}


/**
* @internal prvCpssPxPortCpllDbInit function
* @endinternal
*
* @brief   CPLL database initialization
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on fail
*/
static GT_STATUS prvCpssPxPortCpllDbInit
(
    void
)
{
    GT_U32      serdesNum;  /* serdes loop iterator */

    for(serdesNum = 0; serdesNum < PRV_CPSS_PX_SERDES_NUM_CNS; serdesNum++)
    {
        prvCpssPxPortCpllConfigArr[serdesNum].outFreq = MV_HWS_312MHz_OUT;
        prvCpssPxPortCpllConfigArr[serdesNum].inputSrcClk = MV_HWS_CPLL;
        /* CPLL in receives input clock 25Mhz */
        prvCpssPxPortCpllConfigArr[serdesNum].inFreq = MV_HWS_25MHzIN;
        prvCpssPxPortCpllConfigArr[serdesNum].valid = GT_TRUE;

        if(cpssDeviceRunCheck_onEmulator())
        {
            /* The emulator not supports CPLL registers */
            prvCpssPxPortCpllConfigArr[serdesNum].valid = GT_FALSE;
        }
    }

    return GT_OK;
}

/**
* @internal prvCpssPxPortCpllConfig function
* @endinternal
*
* @brief   CPLL initialization
*
* @param[in] devNum                   - device number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on fail
*/
GT_STATUS prvCpssPxPortCpllConfig
(
    IN GT_SW_DEV_NUM devNum
)
{
    GT_STATUS   rc;                         /* return code */
    GT_U32      serdesNum;                  /* serdes iterator */
    GT_U32      regAddr;                    /* register address */
    CPSS_SYSTEM_RECOVERY_INFO_STC tempSystemRecovery_Info;

    rc = prvCpssPxPortCpllDbInit();
    if(GT_OK != rc)
    {
        return rc;
    }
    rc = cpssSystemRecoveryStateGet(&tempSystemRecovery_Info);
    if (rc != GT_OK)
    {
        return rc;
    }
    if ((tempSystemRecovery_Info.systemRecoveryProcess == CPSS_SYSTEM_RECOVERY_PROCESS_FAST_BOOT_E) ||
        ((tempSystemRecovery_Info.systemRecoveryProcess == CPSS_SYSTEM_RECOVERY_PROCESS_HA_E) &&
         (tempSystemRecovery_Info.systemRecoveryState == CPSS_SYSTEM_RECOVERY_INIT_STATE_E))
       )
    {
        return GT_OK;
    }
    /************************************************************************
     * Internal CPLL initialization:
     *      if CPLL_x (x may be 0,1,2) was not initialized by internal CPU,
     *      it should be done before HWS initialization, because Serdes FW
     *      upload needs clock from CPLL if external reference clock does
     *      not exists.
     ************************************************************************/

    if(pipeDebugCpllInitFlag != GT_TRUE)
    {
        /* configure all serdeses to use external ref clock */
        for (serdesNum = 0; serdesNum < PRV_CPSS_PX_SERDES_NUM_CNS; serdesNum++)
        {
            if(prvCpssPxPortCpllConfigArr[serdesNum].valid == GT_TRUE)
            {
                regAddr = PRV_CPSS_PX_DEV_REGS_VER1_MAC(devNum)->serdesConfig[serdesNum].serdesExternalReg1;
                rc = prvCpssDrvHwPpSetRegField(CAST_SW_DEVNUM(devNum),regAddr,8,1,1);
                if (rc != GT_OK)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
                }
            }
        }
        return GT_OK;
    }
    else
    {
        for (serdesNum = 0; serdesNum < PRV_CPSS_PX_SERDES_NUM_CNS; serdesNum++)
        {
            if(prvCpssPxPortCpllConfigArr[serdesNum].valid == GT_TRUE)
            {
                regAddr = PRV_CPSS_PX_DEV_REGS_VER1_MAC(devNum)->serdesConfig[serdesNum].serdesExternalReg1;
                rc = prvCpssDrvHwPpSetRegField(CAST_SW_DEVNUM(devNum),regAddr,11,2,2);
                if (rc != GT_OK)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
                }
            }
        }
    }

    for (serdesNum = 0; serdesNum < PRV_CPSS_PX_SERDES_NUM_CNS; serdesNum++)
    {
        if(prvCpssPxPortCpllConfigArr[serdesNum].valid == GT_TRUE)
        {
            rc = mvHwsSerdesClockGlobalControl(CAST_SW_DEVNUM(devNum), 0, serdesNum,
                                               prvCpssPxPortCpllConfigArr[serdesNum].inputSrcClk,
                                               prvCpssPxPortCpllConfigArr[serdesNum].inFreq,
                                               prvCpssPxPortCpllConfigArr[serdesNum].outFreq);
            if (rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
            }
        }
    }

    return GT_OK;
}

/**
* @internal internal_cpssPxPortCpllCfgInit function
* @endinternal
*
* @brief   CPLL unit initialization routine.
*
* @note   APPLICABLE DEVICES:     Pipe.
* @note   NOT APPLICABLE DEVICES: None.
*
* @param[in] devNum                   - physical device number
* @param[in] cpllNum                  - CPLL unit index
*                                      (APPLICABLE RANGES: 0)
* @param[in] inputFreq                - CPLL input frequency
* @param[in] outputFreq               - CPLL output frequency
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note API should be called before any port with CPLL as reference clock
*       was created.
*
*/
static GT_STATUS internal_cpssPxPortCpllCfgInit
(
    IN GT_SW_DEV_NUM                              devNum,
    IN GT_U32                                     cpllNum,
    IN CPSS_PX_PORT_CPLL_INPUT_FREQUENCY_ENT      inputFreq,
    IN CPSS_PX_PORT_CPLL_OUTPUT_FREQUENCY_ENT     outputFreq
)
{
    GT_STATUS                       rc;         /* return code */
    MV_HWS_CPLL_INPUT_FREQUENCY     inFreq;     /* input frequency in HWS format */
    MV_HWS_CPLL_OUTPUT_FREQUENCY    outFreq;    /* output frequency in HWS format */
    GT_U32                          serdesNum;  /* serdes loop iterator */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);

    switch(inputFreq)
    {
        case CPSS_PX_PORT_CPLL_INPUT_FREQUENCY_25M_E:
            inFreq = MV_HWS_25MHzIN;
            break;
        case CPSS_PX_PORT_CPLL_INPUT_FREQUENCY_156M_E:
            inFreq = MV_HWS_156MHz_IN;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    switch(outputFreq)
    {
        case CPSS_PX_PORT_CPLL_OUTPUT_FREQUENCY_78M_E:
            outFreq = MV_HWS_78MHz_OUT;
            break;
        case CPSS_PX_PORT_CPLL_OUTPUT_FREQUENCY_156M_E:
            outFreq = MV_HWS_156MHz_OUT;
            break;
        case CPSS_PX_PORT_CPLL_OUTPUT_FREQUENCY_200M_E:
            outFreq = MV_HWS_200MHz_OUT;
            break;
        case CPSS_PX_PORT_CPLL_OUTPUT_FREQUENCY_312M_E:
            outFreq = MV_HWS_312MHz_OUT;
            break;
        case CPSS_PX_PORT_CPLL_OUTPUT_FREQUENCY_161M_E:
            outFreq = MV_HWS_161MHz_OUT;
            break;
        case CPSS_PX_PORT_CPLL_OUTPUT_FREQUENCY_164M_E:
            outFreq = MV_HWS_164MHz_OUT;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if(cpllNum != 0)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    rc = mvHwsGeneralSrcClockModeConfigSet(devNum, 0, 0, 0, 0, CPLL0, inFreq, outFreq);
    if(GT_OK != rc)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }


    for(serdesNum = 0; serdesNum < PRV_CPSS_PX_SERDES_NUM_CNS; serdesNum++)
    {
        if(prvCpssPxPortCpllConfigArr[serdesNum].valid != GT_TRUE)
        {
            continue;
        }
        prvCpssPxPortCpllConfigArr[serdesNum].inFreq = inFreq;
        prvCpssPxPortCpllConfigArr[serdesNum].outFreq = outFreq;
    }

    return GT_OK;
}

/**
* @internal cpssPxPortCpllCfgInit function
* @endinternal
*
* @brief   CPLL unit initialization routine.
*
* @note   APPLICABLE DEVICES:     Pipe.
* @note   NOT APPLICABLE DEVICES: None.
*
* @param[in] devNum                   - physical device number
* @param[in] cpllNum                  - CPLL unit index
*                                      (APPLICABLE RANGES: 0)
* @param[in] inputFreq                - CPLL input frequency
* @param[in] outputFreq               - CPLL output frequency
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note API should be called before any port with CPLL as reference clock
*       was created.
*
*/
GT_STATUS cpssPxPortCpllCfgInit
(
    IN GT_SW_DEV_NUM                              devNum,
    IN GT_U32                                     cpllNum,
    IN CPSS_PX_PORT_CPLL_INPUT_FREQUENCY_ENT      inputFreq,
    IN CPSS_PX_PORT_CPLL_OUTPUT_FREQUENCY_ENT     outputFreq
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortCpllCfgInit);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, cpllNum, inputFreq, outputFreq));

    rc = internal_cpssPxPortCpllCfgInit(devNum, cpllNum, inputFreq, outputFreq);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, cpllNum, inputFreq, outputFreq));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

static GT_STATUS prvCpssPxPortDevInfo
(
    IN  GT_U8  devNum,
    OUT GT_U32 *devId,
    OUT GT_U32 *revNum
)
{
    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);

    *devId = PRV_CPSS_PP_MAC(devNum)->devType;
    *revNum = PRV_CPSS_PP_MAC(devNum)->revision;
    return GT_OK;
}


/* TBD port manager PX */
/**
* @internal prvCpssPxPortCheckAndGetMacNumberWrapper function
* @endinternal
*
* @brief   Wrapper function for CPSS macro PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC
*         in order to use in in Common code.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] portMacNumPtr            - (pointer to) mac number for the given port number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, physicalPortNum
* @retval GT_BAD_PTR               - on bad pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssPxPortCheckAndGetMacNumberWrapper
(
    IN  GT_SW_DEV_NUM        devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_U32               *portMacNumPtr
)
{

    CPSS_LOG_INFORMATION_MAC("inside PortCheckAndGetMacNumberWrapper function wrapper");

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(portMacNumPtr);

    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, *portMacNumPtr);

    return GT_OK;
}
/**
* @internal prvCpssPxCgConvertersRegAddrGetWrapper function
* @endinternal
*
* @brief   Wrapper function for CPSS macro PRV_CPSS_PX_REG1_CG_CONVERTERS_IP_STATUS_REG_MAC
*         in order to use in in Common code.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  none.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] regAddrPtr               - (pointer to) register address of CG mac converters registers
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, physicalPortNum
* @retval GT_BAD_PTR               - on bad pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssPxCgConvertersRegAddrGetWrapper
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_U32                  *regAddrPtr
)
{
    GT_U32 portMacNum;

    CPSS_LOG_INFORMATION_MAC("inside CgConvertersRegAddrGetWrapper function wrapper");

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(regAddrPtr);

    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

    PRV_CPSS_PX_REG1_CG_CONVERTERS_IP_STATUS_REG_MAC(devNum, portMacNum, regAddrPtr);

    return GT_OK;
}

/**
* @internal
*           prvCpssPxCgConvertersStatus2RegAddrGetWrapperfunction
*
* @endinternal
*
* @brief   Wrapper function for CPSS macro
*         PRV_CPSS_PX_REG1_CG_CONVERTERS_IP_STATUS2_REG_MAC in
*         order to use in in Common code.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  none.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] regAddrPtr               - (pointer to) register address of CG mac converters registers
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, physicalPortNum
* @retval GT_BAD_PTR               - on bad pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssPxCgConvertersStatus2RegAddrGetWrapper
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_U32                  *regAddrPtr
)
{
    GT_U32 portMacNum;

    CPSS_LOG_INFORMATION_MAC("inside CgConvertersStatus2RegAddrGetWrapper function wrapper");

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(regAddrPtr);

    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

    PRV_CPSS_PX_REG1_CG_CONVERTERS_IP_STATUS2_REG_MAC(devNum, portMacNum, regAddrPtr);

    return GT_OK;
}


/**
* @internal prvCpssPxPortManagerDbGetWrapper function
* @endinternal
*
* @brief   Wrapper function for getting PRV_CPSS_PORT_MNG_DB_STC structure pointer from
*         CPSS PX pp structure.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] portManagerDbPtr         - (pointer to) pointer to port manager DB.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, physicalPortNum
* @retval GT_BAD_PTR               - on bad pointer
*/
GT_STATUS prvCpssPxPortManagerDbGetWrapper
(
    IN  GT_SW_DEV_NUM            devNum,
    OUT PRV_CPSS_PORT_MNG_DB_STC **portManagerDbPtr
)
{

    CPSS_LOG_INFORMATION_MAC("inside PortManagerDbGetWrapper function wrapper");

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(portManagerDbPtr);

    *portManagerDbPtr = (PRV_CPSS_PORT_MNG_DB_STC* )(&(PRV_CPSS_PX_PP_MAC(devNum)->port.portManagerDb));

    return GT_OK;
}

/**
* @internal prvCpssPxMPCS40GRegAddrGetWrapper function
* @endinternal
*
* @brief   Wrapper function for getting the MPCS 40G common status register address.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  none.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] regAddrPtr               - (pointer to) register address of CG mac converters registers
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, physicalPortNum
* @retval GT_BAD_PTR               - on bad pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssPxMPCS40GRegAddrGetWrapper
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_U32                  *regAddrPtr
)
{
    GT_U32 portMacNum;

    CPSS_LOG_INFORMATION_MAC("inside MPCS40GRegAddrGetWrapper function wrapper");

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

    *regAddrPtr = PRV_CPSS_PX_DEV_REGS_VER1_MAC(devNum)->GOP.perPortRegs[portMacNum].mpcs40GCommonStatus;

    return GT_OK;
}


/**
* @internal prvCpssPxMacDmaModeRegAddrGetWrapper function
* @endinternal
*
* @brief   Wrapper function for getting XLG DMA mode register address
*         in order to use in in Common code.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  none.
*
* @param[in] devNum                   - device number
*                                      portNum         - port number
*
* @param[out] regAddrPtr               - (pointer to) register address of CG mac converters registers
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, physicalPortNum
* @retval GT_BAD_PTR               - on bad pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssPxMacDmaModeRegAddrGetWrapper
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  GT_U32                  portMacNum,
    OUT GT_U32                  *regAddrPtr
)
{
    PRV_CPSS_PORT_TYPE_ENT  portMacType;

    CPSS_LOG_INFORMATION_MAC("inside MacDmaModeRegAddrGetWrapper function wrapper");

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(regAddrPtr);

    portMacType = PRV_CPSS_PX_PORT_TYPE_MAC(devNum,portMacNum);
    PRV_CPSS_PX_PORT_MAC_CTRL4_REG_MAC(devNum, portMacNum, portMacType, regAddrPtr);

    return GT_OK;
}

/**
* @internal prvCpssPxLinkStatusChangedWAs function
* @endinternal
*
* @brief   Wrapper function for handling WAs related to link status change event.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  none.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] portType                 - AP or Regular
* @param[in] LinkUp                   - Link up status
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, physicalPortNum
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note This API is currently implementing nothing and is defined for future use.
*
*/
GT_U32 prvCpssPxLinkStatusChangedWAs
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  CPSS_PORT_MANAGER_PORT_TYPE_ENT portType,
    IN  GT_BOOL                 linkUp
)
{
    GT_STATUS rc;
    GT_BOOL isPortInBiodirectionalMode;/*is port of a 'link down' in mode of 'Biodirectional' */
    GT_BOOL                     apEnable;
    CPSS_PX_PORT_AP_PARAMS_STC  apParams;

    GT_UNUSED_PARAM(portType);
#if 0
    /* we got valid physical port for our MAC number */
    rc = cpssPxPortLinkStatusGet(devNum, portNum, &linkUp);
    if (rc != GT_OK)
    {
        return rc;
    }
#endif
    rc = cpssPxPortApPortConfigGet(devNum, portNum, &apEnable, &apParams);
    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"CPSS_PP_PORT_LINK_STATUS_CHANGED_E - cpssPxPortApPortConfigGet:rc=%d,portNum=%d\n",rc, portNum);
    }

    if (apEnable)
    {
        CPSS_PX_DETAILED_PORT_MAP_STC   portMapShadow;
        GT_U32                          macNum;

        rc = cpssPxPortPhysicalPortDetailedMapGet(devNum, portNum, &portMapShadow);
        if (rc != GT_OK)
        {
           CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"cpssPxPortPhysicalPortDetailedMapGet:error!devNum=%d,port=%d,rc=%d\n",
                    devNum, portNum, rc);
        }

        macNum = portMapShadow.portMap.macNum;

        if (PRV_CPSS_PORT_CG_E == PRV_CPSS_PP_MAC(devNum)->phyPortInfoArray[macNum].portType)
        {
            GT_U32  regAddr;
            GT_U32  data = 0;

            /* AP state machine sends this interrupt on end of treat of port up/down,
                so I'm sure I get here right value */
            regAddr = PRV_PX_REG1_UNIT_CG_CONVERTERS_MAC(devNum, macNum).CGMAConvertersResets;
            if(regAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
            {
                rc = cpssDrvPpHwRegBitMaskRead(devNum, 0, regAddr, 0xffffffff, &data);
                 if (rc != GT_OK)
                 {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"cpssDrvPpHwRegBitMaskRead: error, devNum=%d, port=%d, rc=%d\n",
                                    devNum, portNum, rc);
                 }
            }

            if (((data>>26)&0x1) == 0)
            {
                /* update port DB with the default port type value in order to prevent using CG MAC
                   when it is disabled  */
                PRV_CPSS_PP_MAC(devNum)->phyPortInfoArray[macNum].portType = PRV_CPSS_PORT_XG_E;

                /* init CG port register database */
                rc = prvCpssPxCgPortDbInvalidate(devNum, macNum, GT_TRUE);
                if (rc != GT_OK)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"pipe_linkChange:prvCpssPxCgPortDbInvalidate:rc=%d,portNum=%d,GT_TRUE\n",rc, portNum);
                }
            }
        }
        else
        {
            CPSS_PORT_SPEED_ENT speed;
            CPSS_PORT_INTERFACE_MODE_ENT ifMode;
            GT_BOOL             isCgUnitInUse;

            /* Check current mode */
            rc = cpssPxPortSpeedGet(devNum, portNum, &speed);
            if(rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"pipe_linkChange:cpssPxPortSpeedGet:rc=%d,portNum=%d\n", rc, portNum);
            }
            rc = cpssPxPortInterfaceModeGet(devNum, portNum, &ifMode);
            if(rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"pipe_linkChange:cpssPxPortInterfaceModeGet:rc=%d,portNum=%d\n", rc, portNum);
            }

            rc = prvCpssPxIsCgUnitInUse(devNum, portNum, ifMode, speed, &isCgUnitInUse);
            if(rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"pipe_linkChange:prvCpssPxIsCgUnitInUse:rc=%d,portNum=%d,ifMode=%d,speed=%d\n",
                                rc, portNum, ifMode, speed);
            }

            if(isCgUnitInUse)
            {/* must restore CG DB, because if we here it means link was restored after fail */
                PRV_CPSS_PP_MAC(devNum)->phyPortInfoArray[macNum].portType = PRV_CPSS_PORT_CG_E;

                /* init CG port register database */
                rc = prvCpssPxCgPortDbInvalidate(devNum, macNum, GT_FALSE);
                if (rc != GT_OK)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"pipe_linkChange:prvCpssPxCgPortDbInvalidate:rc=%d,portNum=%d,GT_FALSE\n", rc, portNum);
                }
            }
        }
    }

    isPortInBiodirectionalMode = GT_FALSE;

    if(linkUp == GT_FALSE && isPortInBiodirectionalMode == GT_TRUE)
    {
        /* !!! do not modify the filter !!! keep it as 'link up' */
        return GT_OK;
    }

    /* need to set the ingress link status filter according to new state of the
       port */
    rc = cpssPxIngressPortTargetEnableSet(devNum, portNum,linkUp);
    if (rc != GT_OK)
    {
        return rc;
    }

    return GT_OK;
}

/**
* @internal prvCpssPxXlgMacIntMaskGet function
* @endinternal
*
* @brief   Get the address of the XLG mac interrupt mask register.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portMacNum               - port mac number
*
* @param[out] regAddrPtr               - address of the XLG mac interrupt mask register
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL ptr
*/
GT_STATUS prvCpssPxXlgMacIntMaskGet
(
    IN  GT_SW_DEV_NUM   devNum,
    IN  GT_U32          portMacNum,
    OUT GT_U32          *regAddrPtr
)
{
    GT_U32 regAddr;

    CPSS_NULL_PTR_CHECK_MAC(regAddrPtr);

    PRV_CPSS_PX_PORT_MAC_INT_MASK_REG_MAC(devNum, portMacNum, PRV_CPSS_PORT_XLG_E, &regAddr);

    *regAddrPtr = regAddr;

    return GT_OK;
}


/**
* @internal internal_cpssPxPortFaultSendSet function
* @endinternal
*
* @brief   Configure the port to start or stop sending fault signals to partner.
*         When port is configured to send, link on both sides will be down.
*         Note: tx_rem_fault takes precedence over
*         tx_loc_fault. for CG mac, know that using tx_rem_fault
*         will cancel tx_loc_fault effect.
*
* @note   APPLICABLE DEVICES:      Pipe;
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] ifMode                   - port interface mode
* @param[in] speed                    - port speed
* @param[in] send                     - or stop sending
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPortFaultSendSet
(
    IN  GT_SW_DEV_NUM        devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  CPSS_PORT_INTERFACE_MODE_ENT  ifMode,
    IN  CPSS_PORT_SPEED_ENT           speed,
    IN  GT_BOOL                       send
)
{
    GT_STATUS rc;
    GT_U32    portGroupId;
    GT_U32    portMacNum;      /* MAC number */
    MV_HWS_PORT_STANDARD    portMode;
    GT_U32    localPort;

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

    /* Get port parameters */
    portGroupId = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portMacNum);
    localPort = portMacNum;

    /* Sanity check */
    if((CPSS_PORT_INTERFACE_MODE_NA_E == ifMode) || (CPSS_PORT_SPEED_NA_E == speed))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    /* Getting HWS supported port mode */
    rc = prvCpssCommonPortIfModeToHwsTranslate(devNum,ifMode, speed, &portMode);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* Configure hws to start ot stop sending fault signals */
    rc = hwsPortFaultSendSet(devNum, portGroupId, localPort, portMode, send);

    if (rc!=GT_OK)
    {
        return rc;
    }

    return GT_OK;
}

/**
* @internal cpssPxPortFaultSendSet function
* @endinternal
*
* @brief   Configure the port to start or stop sending fault signals to partner.
*         When port is configured to send, link on both sides will be down.
*
* @note   APPLICABLE DEVICES:      Pipe;
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] ifMode                   - port interface mode
* @param[in] speed                    - port speed
* @param[in] send                     - or stop sending
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortFaultSendSet
(
    IN  GT_SW_DEV_NUM        devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  CPSS_PORT_INTERFACE_MODE_ENT  ifMode,
    IN  CPSS_PORT_SPEED_ENT           speed,
    IN  GT_BOOL                       send
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortFaultSendSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum));

    rc = internal_cpssPxPortFaultSendSet(devNum, portNum, ifMode, speed, send);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal prvCpssPxPortProprietaryHighSpeedSet function
* @endinternal
*
* @brief   Configure the CPU ports resources so High speed
*         ports will have enough bencdwitdh.
*
* @note   APPLICABLE DEVICES:      Pipe;
* @param[in] devNum                   - physical device number
*
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssPxPortProprietaryHighSpeedSet
(
    IN  GT_SW_DEV_NUM        devNum,
    IN  GT_BOOL              enable
)
{
    GT_STATUS rc;

    devNum = devNum;
    enable = enable;
   /* Currently not supported for PX */
    rc = GT_NOT_APPLICABLE_DEVICE;

    return rc;
}

/**
* @internal prvCpssPxPortMgrInit function
* @endinternal
*
* @brief   Init Port manager
*
* @note   APPLICABLE DEVICES:      Pipe;
* @param[in] devNum                   - physical device number
*
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssPxPortMgrInit
(
    IN  GT_SW_DEV_NUM               devNum,
    IN  PRV_CPSS_PORT_PM_FUNC_PTRS *cpssPmFuncPtr

)
{
    GT_STATUS rc;
    GT_U32 port;
    GT_BOOL isCpu = GT_FALSE;
    GT_BOOL ingressStatus;
    GT_UNUSED_PARAM(cpssPmFuncPtr);
    for(port = 0 ;port < PRV_CPSS_PP_MAC(devNum)->numOfPorts ;port++)
    {
        rc = cpssPxPortPhysicalPortMapIsCpuGet(CAST_SW_DEVNUM(devNum), port, &isCpu);
        if(rc != GT_OK)
        {
            return rc;
        }
        ingressStatus = isCpu;
        rc = cpssPxIngressPortTargetEnableSet(devNum,port,ingressStatus);
        if(rc != GT_OK)
        {
            return rc;
        }
    }
    return GT_OK;
}

/**
* @internal prvCpssPxPortMngEngineInit function
* @endinternal
*
* @brief   Initialize port manager related databases and
*          function pointers
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @retval GT_OK                    - on success
* @retval GT_OUT_OF_CPU_MEM        - port object allocation failed
* @retval GT_FAIL                  - wrong devFamily
*/
GT_STATUS prvCpssPxPortMngEngineInit
(
    IN GT_U8    devNum
)
{
    GT_STATUS rc;
    PRV_CPSS_PORT_MNG_DB_STC *tmpPortManagerDbPtr;

    PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppCheckAndGetMacFunc = prvCpssPxPortCheckAndGetMacNumberWrapper;
    PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppConvertMacToPortFunc = NULL;
    PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppLinkStatusGetFunc = cpssPxPortLinkStatusGet;
    PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppRemoteFaultSetFunc = cpssPxPortFaultSendSet;
    PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppForceLinkDownSetFunc = cpssPxPortForceLinkDownEnableSet;
    PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppPortMacEnableGetFunc = prvCpssPxPortEnableGet;
    PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppPortMacEnableSetFunc = prvCpssPxPortEnableSet;
    PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppCgConvertersGetFunc = prvCpssPxCgConvertersRegAddrGetWrapper;
    PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppCgConvertersStatus2GetFunc = prvCpssPxCgConvertersStatus2RegAddrGetWrapper;
    PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppSpeedGetFromExtFunc = cpssPxPortSpeedGet;
    PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppPortManagerDbGetFunc = prvCpssPxPortManagerDbGetWrapper;
    PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppMpcs40GCommonStatusRegGetFunc = prvCpssPxMPCS40GRegAddrGetWrapper;
    PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppMacDmaModeRegAddrGetFunc = prvCpssPxMacDmaModeRegAddrGetWrapper;
    PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppLinkStatusChangedNotifyFunc = prvCpssPxLinkStatusChangedWAs;
    PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppXlgMacMaskAddrGetFunc = prvCpssPxXlgMacIntMaskGet;
    PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppMacConfigClearFunc = prvCpssPxPortMacConfigurationClear;
    PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppMacConfigurationFunc = prvCpssPxPortMacConfiguration;
    /* common functions bind - (currently for Port Manager use) - HwInit and PortMapping related*/
    PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppNumOfSerdesGetFunc = prvCpssPxHwInitNumOfSerdesGet;
    PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppMacConvertFunc = prvCpssPxPortEventPortMapConvert;
    PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppPortRemoteFaultConfigGetFunc = prvCpssPxPortRemoteFaultConfigGet;
    PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppPortLkbSetFunc = prvCpssPxPortLkbSet;
    /* AP port APIs */
    PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppPortApConfigSetFunc = prvCpssPxPortApPortConfigSetConvert;
    PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppPortApStatusGetFunc = prvCpssPxPortApPortStatusGetConvert;
    PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppPortApSerdesTxParamsOffsetSetFunc = cpssPxPortApSerdesTxParametersOffsetSet;

    PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppPortMacLoopbackModeSetFunc = cpssPxPortInternalLoopbackEnableSet;
    PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppPortPcsLoopbackModeSetFunc = prvCpssPxPortPcsLoopbackModeSetWrapper;
    PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppPortSerdesLoopbackModeSetFunc = prvCpssPxPortSerdesLoopbackModeSetWrapper;
    PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppPortSerdesPolaritySetFunc = prvCpssPxPortSerdesLanePolaritySetWrapper;
    PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppPortProprietaryHighSpeedPortsSetFunc = prvCpssPxPortProprietaryHighSpeedSet;
    PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppPortSerdesTuneResultGetFunc = prvCpssPxPortSerdesAutoTuneResultsGetWrapper;
    PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppAutoNeg1GSgmiiFunc = prvCpssPxAutoNeg1GSgmiiWrapper;
    PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppUsxReplicationSetFunc = NULL;
    PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppPortMgrInitFunc = prvCpssPxPortMgrInit;
    PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppPortFastLinkDownSetFunc = NULL;
    PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppPortExtraOperationsSetFunc = NULL;
    PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppRemotePortCheckFunc = NULL;
    PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppPortPreemptionParamsSetFunc = NULL;
    PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppPortForceLinkPassSetFunc = NULL;
    PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppPortForceLinkPassGetFunc = NULL;
    PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppPortEnableWaWithLinkStatusSet = NULL;

    rc = prvCpssPxPortManagerDbGetWrapper(devNum,&tmpPortManagerDbPtr);
    if (rc != GT_OK) {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }
      /*global parameters*/
    tmpPortManagerDbPtr->globalParamsCfg.signalDetectDbCfg.sdChangeInterval = 5;
    tmpPortManagerDbPtr->globalParamsCfg.signalDetectDbCfg.sdChangeMinWindowSize = 50;
    tmpPortManagerDbPtr->globalParamsCfg.signalDetectDbCfg.sdCheckTimeExpired = 100;

    return GT_OK;
}
/**
* @internal prvCpssPxPortIfCfgInit function
* @endinternal
*
* @brief   Initialize port interface mode configuration method in device object
*
* @note   APPLICABLE DEVICES:     Pipe.
* @note   NOT APPLICABLE DEVICES: None.
*
* @param[in] devNum                   - device number
*
* @retval GT_OK                    - on success
* @retval GT_OUT_OF_CPU_MEM        - port object allocation failed
* @retval GT_FAIL                  - wrong devFamily
*/
GT_STATUS prvCpssPxPortIfCfgInit
(
    IN GT_SW_DEV_NUM    devNum,
    IN GT_BOOL          allowHwAccessOnly
)
{
    GT_STATUS rc;
    GT_U32  ticks;  /* system clock rate - ticks per second */
    HWS_OS_FUNC_PTR funcPtrsStc; /* pointers to OS/CPU dependent system calls */
#ifdef TBD_PIPE_COMBO
    CPSS_DXCH_PORT_COMBO_PARAMS_STC *comboPortsInfoArray; /* ptr to combo
                            ports array for shorten code and quicker access */
#endif
    GT_U32  i, j;   /* iterators */

    /* calculate length of tick in mSec for exact delay function */
    cpssOsGetSysClockRate(&ticks);
    tickTimeMsec = 1000/ticks;

    /* TBD port manager PX */
    /* port ifMode function obj init*/
    rc = prvCpssPxPortIfFunctionsObjInit(devNum);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"PortIfFunctionsObjInit failed=%d", rc);
    }
    /* port Serdes function obj init*/
    rc = prvCpssPxPortSerdesFunctionsObjInit(devNum);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"PortSerdesFunctionsObjInit failed=%d", rc);
    }

    /* common functions bind - (currently for Port Manager use) - porCtrl related*/
    rc = prvCpssPxPortMngEngineInit(devNum);
    if (rc != GT_OK) {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    cpssOsMemSet(&funcPtrsStc, 0, sizeof(funcPtrsStc));

    funcPtrsStc.osExactDelayPtr = prvCpssCommonPortTraceDelay;/*common code for DX,PX*/
    funcPtrsStc.osTimerWkPtr    = cpssOsTimerWkAfter;
    funcPtrsStc.osMemSetPtr     = cpssOsMemSet;
#ifdef OS_MALLOC_MEMORY_LEAKAGE_DBG
    funcPtrsStc.osFreePtr       = myCpssOsFree;
    funcPtrsStc.osMallocPtr     = myCpssOsMalloc;
#else  /*! OS_MALLOC_MEMORY_LEAKAGE_DBG */
    funcPtrsStc.osFreePtr       = cpssOsFree;
    funcPtrsStc.osMallocPtr     = cpssOsMalloc;
#endif /*! OS_MALLOC_MEMORY_LEAKAGE_DBG */
    funcPtrsStc.osMemCopyPtr    = (MV_OS_MEM_COPY_FUNC)cpssOsMemCpy;
    funcPtrsStc.sysDeviceInfo   = prvCpssPxPortDevInfo;
    funcPtrsStc.serverRegSetAccess = cpssDrvHwPpResetAndInitControllerWriteReg;
    funcPtrsStc.serverRegGetAccess = cpssDrvHwPpResetAndInitControllerReadReg;
    funcPtrsStc.serverRegFieldSetAccess = cpssDrvHwPpResetAndInitControllerSetRegField;
    funcPtrsStc.serverRegFieldGetAccess = cpssDrvHwPpResetAndInitControllerGetRegField;
    funcPtrsStc.timerGet = cpssOsTimeRT;
    funcPtrsStc.osStrCatPtr = cpssOsStrCat;
    funcPtrsStc.serdesRegGetAccess =
        (MV_SERDES_REG_ACCESS_GET)prvCpssGenPortGroupSerdesReadRegBitMask;
    funcPtrsStc.serdesRegSetAccess =
        (MV_SERDES_REG_ACCESS_SET)prvCpssGenPortGroupSerdesWriteRegBitMask;
    (GT_VOID)multiLaneConfigSupport(GT_FALSE);

    funcPtrsStc.registerSetAccess = (MV_REG_ACCESS_SET)genRegisterSet;
    funcPtrsStc.registerGetAccess = (MV_REG_ACCESS_GET)genRegisterGet;
    funcPtrsStc.coreClockGetPtr   = (MV_CORE_CLOCK_GET)prvCpssPxHwCoreClockGetWrapper;


    CPSS_LOG_INFORMATION_MAC("Calling: hwsPipeIfPreInit(devNum[%d], *funcPtr)", devNum);

    rc = hwsPipeIfPreInit(CAST_SW_DEVNUM(devNum), &funcPtrsStc);
    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    rc = prvCpssPxPortCpllConfig(devNum);
    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    rc = hwsPipeIfInit(CAST_SW_DEVNUM(devNum), &funcPtrsStc);
    if((rc != GT_OK) && (rc != GT_ALREADY_EXIST))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }


    /* initialize only cpssDriver for HW access, skip the rest */
    if (allowHwAccessOnly == GT_TRUE)
        return GT_OK;


#ifdef TBD_PIPE_COMBO
    /* init combo ports array */
    comboPortsInfoArray = PRV_CPSS_DXCH_PP_MAC(devNum)->port.comboPortsInfoArray;
    for(i = 0; i < PRV_CPSS_MAX_PP_PORTS_NUM_CNS; i++)
    {
        comboPortsInfoArray[i].preferredMacIdx = CPSS_DXCH_PORT_COMBO_NA_MAC_CNS;
        for(j = 0; j < CPSS_DXCH_PORT_MAX_MACS_IN_COMBO_CNS; j++)
        {
            comboPortsInfoArray[i].macArray[j].macNum = CPSS_DXCH_PORT_COMBO_NA_MAC_CNS;
        }
    }
#endif
    /* init PRBS modes ports array */
    for(i = 0; i < PRV_CPSS_MAX_PP_PORTS_NUM_CNS; i++)
    {
        PRV_CPSS_PX_PP_MAC(devNum)->port.prbsMode[i] = CPSS_PX_DIAG_TRANSMIT_MODE_REGULAR_E;
    }

    /* init PRBS Serdes Counters Clear On Read Enable */
    for(i = 0; i < PRV_CPSS_MAX_MAC_PORTS_NUM_CNS; i++)
    {
        for(j = 0; j < PRV_CPSS_MAX_PORT_LANES_CNS; j++)
        {
            PRV_CPSS_PX_PP_MAC(devNum)->port.prbsSerdesCountersClearOnReadEnable[i][j] = GT_TRUE;
        }
    }

    return GT_OK;
}



/**
* @internal prvCpssPxPortStateDisableSet function
* @endinternal
*
* @brief   Disable a specified port on specified device.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] portStatePtr             - (pointer to) the state (en/dis) of port before calling this function
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
*/
static GT_STATUS prvCpssPxPortStateDisableSet
(
    IN GT_SW_DEV_NUM         devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_BOOL              *portStatePtr
)
{
    GT_STATUS   rc; /* return code */

    rc = internal_cpssPxPortEnableGet(devNum, portNum, portStatePtr);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* disable port if we need */
    if (GT_TRUE == *portStatePtr)
    {
        rc = internal_cpssPxPortEnableSet(devNum, portNum, GT_FALSE);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    return GT_OK;
}


/**
* @internal prvCpssPxPortStateEnableSet function
* @endinternal
*
* @brief   Enable a specified port on specified device.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] portState                - the state (en/dis) of port before calling the port disable
*                                      function
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
*/
static GT_STATUS prvCpssPxPortStateEnableSet
(
    IN GT_SW_DEV_NUM        devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN GT_BOOL              portState
)
{
    /* enable port if we need */
    if (portState == GT_TRUE)
    {
        return cpssPxPortEnableSet(devNum, portNum, GT_TRUE);
    }

    return GT_OK;
}

/**
* @internal prvCpssPxPortStateDisableAndGet function
* @endinternal
*
* @brief   Disable port and get current port parameters that should be
*         restored after port configuration.
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] portStateStcPtr          - (pointer to) the port state struct for saving parameters
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS prvCpssPxPortStateDisableAndGet
(
    IN  GT_SW_DEV_NUM               devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    OUT PRV_CPSS_PX_PORT_STATE_STC  *portStateStcPtr
)
{
    GT_STATUS   rc;

    CPSS_NULL_PTR_CHECK_MAC(portStateStcPtr);

    rc = cpssPxIngressPortTargetEnableGet(devNum, portNum, &portStateStcPtr->egfPortLinkStatusState);
    if(GT_OK != rc)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }


    rc = cpssPxIngressPortTargetEnableSet(devNum, portNum, GT_FALSE);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    rc = prvCpssPxPortStateDisableSet(devNum, portNum, &portStateStcPtr->portEnableState);
    if(GT_OK != rc)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    return GT_OK;
}


/**
* @internal prvCpssPxPortStateRestore function
* @endinternal
*
* @brief   Restore port parameters that was saved before port configuration.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] portStateStcPtr          - (pointer to) the port state struct for restoring parameters
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS prvCpssPxPortStateRestore
(
    IN  GT_SW_DEV_NUM               devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    IN  PRV_CPSS_PX_PORT_STATE_STC  *portStateStcPtr
)
{
    GT_STATUS rc;

    CPSS_NULL_PTR_CHECK_MAC(portStateStcPtr);

    rc = prvCpssPxPortStateEnableSet(devNum, portNum, portStateStcPtr->portEnableState);
    if(GT_OK != rc)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    rc = cpssPxIngressPortTargetEnableSet(devNum, portNum, portStateStcPtr->egfPortLinkStatusState);
    if(GT_OK != rc)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    return GT_OK;
}


/**
* @internal prvCpssPxPortInternalLoopbackEnableSet function
* @endinternal
*
* @brief   Configure MAC and PCS TX2RX loopbacks on port.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number (not CPU port)
* @param[in] enable                   - If GT_TRUE,  loopback
*                                      If GT_FALSE, disable loopback
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssPxPortInternalLoopbackEnableSet
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_BOOL                 enable
)
{
    GT_STATUS                   rc; /* return code */
    GT_U32                      portGroupId;    /* core number of port */
    MV_HWS_PORT_STANDARD        portMode;   /* port interface in HWS format */

    MV_HWS_PORT_LB_TYPE         lbType;     /* loopback type in HWS format */
    PRV_CPSS_PX_PORT_STATE_STC  portStateStc;       /* current port state */
    CPSS_PORT_INTERFACE_MODE_ENT ifMode; /* current interface of port */
    CPSS_PORT_SPEED_ENT         speed;  /* current speed of port */
    GT_U32                      portMacNum;      /* MAC number */
#ifdef TBD_PIPE_COMBO
    MV_HWS_PORT_STANDARD        portModeCombo;   /* port interface in HWS format for non-active combo MAC member*/
#endif

    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);
    ifMode = PRV_CPSS_PX_PORT_IFMODE_MAC(devNum, portMacNum);
    speed = PRV_CPSS_PX_PORT_SPEED_MAC(devNum, portMacNum);
    if((CPSS_PORT_INTERFACE_MODE_NA_E == ifMode) || (CPSS_PORT_SPEED_NA_E == speed))
    {/* loopback on port which interface not defined yet forbidden */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }
    portGroupId = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portMacNum);

    lbType = (enable) ? TX_2_RX_LB : DISABLE_LB;

    rc = prvCpssCommonPortIfModeToHwsTranslate(devNum,ifMode, speed, &portMode);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* disable port always because loopback procedure resets and unreset
       port's MAC. Traffic should be avoided during reset/unreset of MAC to
       avoid corrupted packets */
    if (prvCpssPxPortStateDisableAndGet(devNum, portNum, &portStateStc) != GT_OK)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);


    if(portMode != HGL)  /* HGL doesn't support MAC loopback */
    {
        CPSS_LOG_INFORMATION_MAC("Calling: mvHwsPortLoopbackSet(devNum[%d], portGroupId[%d], phyPortNum[%d], portMode[%d], lpPlace[%d], lbType[%d])",devNum, portGroupId, portMacNum, portMode, HWS_MAC, lbType);
        rc = mvHwsPortLoopbackSet(devNum, portGroupId, portMacNum, portMode,
                                  HWS_MAC, lbType);
        if(rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"mvHwsPortLoopbackSet : Hws Failed");
        }

        /* set Rate limit */
        rc = prvCpssPxPortResourcesRateLimitSet(devNum, portNum, speed, (enable? GT_FALSE:GT_TRUE));
        if(rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"prvCpssPxPipePortResourcesRateLimitSet : Failed");
        }
    }
    CPSS_LOG_INFORMATION_MAC("Calling: mvHwsPortLoopbackSet(devNum[%d], portGroupId[%d], phyPortNum[%d], portMode[%d], lpPlace[%d], lbType[%d])",devNum, portGroupId, portMacNum, portMode, HWS_PCS, lbType);
    rc = mvHwsPortLoopbackSet(devNum, portGroupId, portMacNum, portMode,
                              HWS_PCS, lbType);
    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"mvHwsPortLoopbackSet : Hws Failed");
    }

#ifdef TBD_PIPE_COMBO
    if((_10GBase_KR == portMode) || (SGMII == portMode) || (_1000Base_X == portMode))
    {
        CPSS_DXCH_PORT_COMBO_PARAMS_STC *comboParamsPtr;

        comboParamsPtr = &PRV_CPSS_DXCH_PP_MAC(devNum)->port.comboPortsInfoArray[portNum];
        if(comboParamsPtr->macArray[0].macNum != CPSS_DXCH_PORT_COMBO_NA_MAC_CNS)
        { /* if it's combo port */
            if (_10GBase_KR == portMode)
            {
                portModeCombo = _100GBase_KR10;
            }
            else if ((SGMII == portMode) || (_1000Base_X == portMode))
            {
                portModeCombo = (SGMII == portMode) ? QSGMII : _100Base_FX;
            }
            else
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
            }

            CPSS_LOG_INFORMATION_MAC("Calling: mvHwsPortLoopbackSet(devNum[%d], portGroupId[%d], phyPortNum[%d], portMode[%d], lpPlace[%d], lbType[%d])",devNum, portGroupId, portMacNum, portModeCombo, HWS_MAC, lbType);
            rc = mvHwsPortLoopbackSet(devNum, portGroupId, portMacNum, portModeCombo,
                                      HWS_MAC, lbType);
            if(rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"mvHwsPortLoopbackSet : Hws Failed");
            }

            CPSS_LOG_INFORMATION_MAC("Calling: mvHwsPortLoopbackSet(devNum[%d], portGroupId[%d], phyPortNum[%d], portMode[%d], lpPlace[%d], lbType[%d])",devNum, portGroupId, portMacNum, portModeCombo, HWS_PCS, lbType);
            rc = mvHwsPortLoopbackSet(devNum, portGroupId, portMacNum, portModeCombo,
                                      HWS_PCS, lbType);
            if(rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"mvHwsPortLoopbackSet : Hws Failed");
            }
        }
    }
#endif

    /* enable port if we need */
    if (prvCpssPxPortStateRestore(devNum,portNum, &portStateStc) != GT_OK)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);

    return GT_OK;
}

/**
* @internal prvCpssPxVntLastReadTimeStampGet function
* @endinternal
*
* @brief   Gets the last-read-time-stamp counter value that represent the exact
*         timestamp of the last read operation to the PP registers.
*
* @note   APPLICABLE DEVICES:     Pipe.
* @note   NOT APPLICABLE DEVICES: None.
*
* @param[in] devNum                   - device number
*                                      portGroupsBmp  - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion; Lion2)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*                                      - read only from first active port group of the bitmap.
*
* @param[out] timeStampValuePtr        - pointer to timestamp of the last read operation.
*                                      200  MHz:   granularity - 5 nSec
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or portGroupsBmp
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssPxVntLastReadTimeStampGet
(
    IN  GT_U8                   devNum,
    OUT GT_U32                  *timeStampValuePtr
)
{
    GT_U32    regAddr;       /* register address   */
    GT_U32    portGroupId;   /* the port group Id  */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(timeStampValuePtr);

    portGroupId = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;
    regAddr = PRV_CPSS_PX_DEV_REGS_VER1_MAC(devNum)->MG.globalRegs.lastReadTimeStampReg;

    /* Gets the last-read-time-stamp counter value */
    return prvCpssHwPpPortGroupReadRegister(devNum, portGroupId,
                                               regAddr, timeStampValuePtr);

}

/**
* @internal prvCpssPxPortTraceDelay function
* @endinternal
*
* @brief   Make delay and trace it
*
* @param[in] devNum                   - PP device number
* @param[in] portGroupId              - ports group number
* @param[in] millisec                 -  the delay in millisec
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on fail
*
*   NOTE: called from prvCpssCommonPortTraceDelay(...)
*/
GT_STATUS prvCpssPxPortTraceDelay
(
    IN GT_U8       devNum,
    IN GT_U32      portGroupId,
    IN GT_U32      millisec
)
{
    if(millisec > tickTimeMsec/2)
    {/* if required delay more than half of tick time do OS delay */
        HW_WAIT_MILLISECONDS_MAC(devNum,portGroupId,millisec);
    }
    else
    {/* if delay is short do busy wait */
        if (prvCpssDrvTraceHwDelay[devNum] == GT_TRUE)
        {
            cpssTraceHwAccessDelay(devNum,portGroupId,millisec);
        }
        {
            GT_STATUS   rc; /* return code */
            GT_U32  startTimeStamp, /* value of time stamp register on delay start */
                    currentTimeStamp;/* current value of time stamp register */
            GT_U32  diff; /* emount of core clocks passed */
            GT_U32  passed; /* delay time passed in millisec */
            GT_U32  value;  /* dummy - user defined register value */
            GT_U32  coreClockTime; /* in millisec */
            GT_U32  regAddr;    /* address of register to read */

            coreClockTime = PRV_CPSS_PP_MAC(devNum)->coreClock * 1000;
            /* register must be not in MG unit */
            regAddr = PRV_PX_REG1_UNIT_PCP_MAC(devNum).general.PCPDebug;
            if(PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
            }
            /* read register to cause time stamp register update */
            if (prvCpssDrvHwPpPortGroupReadRegister(CAST_SW_DEVNUM(devNum), portGroupId,
                    regAddr, &value) != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);
            }

            rc = prvCpssPxVntLastReadTimeStampGet(devNum, &startTimeStamp);
            if(rc != GT_OK)
            {
                return rc;
            }

            do
            {
                if (prvCpssDrvHwPpPortGroupReadRegister(CAST_SW_DEVNUM(devNum), portGroupId,
                        regAddr, &value) != GT_OK)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);
                }

                rc = prvCpssPxVntLastReadTimeStampGet(devNum, &currentTimeStamp);
                if(rc != GT_OK)
                {
                    return rc;
                }

                /* calculate difference between current and start */
                if (currentTimeStamp > startTimeStamp)
                {
                    diff = currentTimeStamp - startTimeStamp;
                }
                else
                {/* take care of wraparound */
                   diff = (0xFFFFFFFFU - startTimeStamp) + currentTimeStamp;
                }

                passed = diff/coreClockTime;

                #ifdef ASIC_SIMULATION
                    /* single iteration is enough ... as we 'checked' that above
                       logic access existing registers */
                    passed = millisec;
                #endif /*ASIC_SIMULATION*/
            }while(passed < millisec);
        }
    }

    return GT_OK;
}

/**
* @internal prvCpssPxPortDuplexAutoNegEnableSet function
* @endinternal
*
* @brief   Enable/disable an Auto-Negotiation for duplex mode on specified port on
*         specified device.
*         When duplex Auto-Negotiation is enabled, the port's duplex mode is
*         resolved via Auto-Negotiation. The Auto-Negotiation advertised duplex
*         mode is full-duplex.
*         When duplex Auto-Negotiation is disabled, the port's duplex mode is set
*         via cpssPxPortDuplexModeSet
*
* @note   APPLICABLE DEVICES:      pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] state                    - GT_TRUE for enable Auto-Negotiation for duplex mode,
*                                      GT_FALSE otherwise
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - on not supported state
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note CPU port not supports the duplex auto negotiation
*
*/
static GT_STATUS prvCpssPxPortDuplexAutoNegEnableSet
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_BOOL                 state
)
{
    GT_STATUS                   rc;         /* return code */
    PRV_CPSS_PX_PORT_STATE_STC  portStateStc;       /* current port state */
    GT_U32                      dupledAutoNegRegAddr; /* address of reg. for duplex auto-neg. config. */
    PRV_CPSS_PORT_TYPE_ENT      portMacType;
    GT_U32                      portMacNum; /* MAC number */
    PRV_CPSS_PORT_REG_CONFIG_STC regDataArray[PRV_CPSS_PORT_NOT_APPLICABLE_E];

    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

    portMacType = PRV_CPSS_PX_PORT_TYPE_MAC(devNum,portMacNum);
    if(portMacType >= PRV_CPSS_PORT_XG_E)
    {/* check if auto-neg. supported for current mode/port*/
        if (state == GT_TRUE)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
        }
    }

    PRV_CPSS_PX_PORT_AUTO_NEG_CTRL_REG_MAC(devNum,portMacNum,&dupledAutoNegRegAddr);
    if(dupledAutoNegRegAddr == PRV_CPSS_SW_PTR_ENTRY_UNUSED)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);

    if(portMacType <= PRV_CPSS_PORT_GE_E)
    {/* disable port if we need */
        rc = prvCpssPxPortStateDisableAndGet(devNum,portNum,&portStateStc);
        if(rc != GT_OK)
            return rc;
    }

    if(prvCpssPxPortMacConfigurationClear(regDataArray) != GT_OK)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_INIT_ERROR, LOG_ERROR_NO_MSG);

    regDataArray[PRV_CPSS_PORT_GE_E].regAddr = dupledAutoNegRegAddr;
    regDataArray[PRV_CPSS_PORT_GE_E].fieldData = BOOL2BIT_MAC(state);
    regDataArray[PRV_CPSS_PORT_GE_E].fieldOffset = 13;
    regDataArray[PRV_CPSS_PORT_GE_E].fieldLength = 1;

    rc = prvCpssPxPortMacConfiguration(devNum, portNum, regDataArray);
    if(rc != GT_OK)
    {
        return rc;
    }

    if(portMacType <= PRV_CPSS_PORT_GE_E)
    {/* restore port enable state */
        rc = prvCpssPxPortStateRestore(devNum,portNum, &portStateStc);
        if(rc != GT_OK)
            return rc;
    }

    return GT_OK;
}

/**
* @internal internal_cpssPxPortDuplexAutoNegEnableSet function
* @endinternal
*
* @brief   Enable/disable an Auto-Negotiation for duplex mode on specified port on
*         specified device.
*         When duplex Auto-Negotiation is enabled, the port's duplex mode is
*         resolved via Auto-Negotiation. The Auto-Negotiation advertised duplex
*         mode is full-duplex.
*         When duplex Auto-Negotiation is disabled, the port's duplex mode is set
*         via cpssPxPortDuplexModeSet
*
* @note   APPLICABLE DEVICES:      pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] state                    - GT_TRUE for enable Auto-Negotiation for duplex mode,
*                                      GT_FALSE otherwise
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - on not supported state
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note 1.CPU port not supports the duplex auto negotiation
*
*/
static GT_STATUS internal_cpssPxPortDuplexAutoNegEnableSet
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_BOOL                 state
)
{
    GT_STATUS rc = GT_OK;
    GT_BOOL targetPortState = state;
    GT_U32 portMacNum; /* MAC number */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

    if(PRV_CPSS_PX_PP_MAC(devNum)->genInfo.hitlessWriteMethodEnable)
    {
        GT_BOOL   currentState;

        rc = cpssPxPortDuplexAutoNegEnableGet(devNum, portNum, &currentState);
        if(rc != GT_OK)
        {
            return rc;
        }
        if(state == currentState)
        {
            return GT_OK;
        }
    }

    rc = prvCpssPxPortDuplexAutoNegEnableSet(devNum,portNum,targetPortState);
    if(rc!=GT_OK)
    {
        return rc;
    }
    return rc;
}

/**
* @internal cpssPxPortDuplexAutoNegEnableSet function
* @endinternal
*
* @brief   Enable/disable an Auto-Negotiation for duplex mode on specified port on
*         specified device.
*         When duplex Auto-Negotiation is enabled, the port's duplex mode is
*         resolved via Auto-Negotiation. The Auto-Negotiation advertised duplex
*         mode is full-duplex.
*         When duplex Auto-Negotiation is disabled, the port's duplex mode is set
*         via cpssPxPortDuplexModeSet
*
* @note   APPLICABLE DEVICES:      pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] state                    - GT_TRUE for enable Auto-Negotiation for duplex mode,
*                                      GT_FALSE otherwise
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - on not supported state
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note 1.CPU port not supports the duplex auto negotiation
*
*/
GT_STATUS cpssPxPortDuplexAutoNegEnableSet
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_BOOL                 state
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortDuplexAutoNegEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, state));

    rc = internal_cpssPxPortDuplexAutoNegEnableSet(devNum, portNum, state);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, state));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal prvCpssPxPortDuplexAutoNegEnableGet function
* @endinternal
*
* @brief   Get enable/disable status of an Auto-Negotiation for MAC duplex mode
*         per port.
*         When duplex Auto-Negotiation is enabled, the port's duplex mode is
*         resolved via Auto-Negotiation. The Auto-Negotiation advertised duplex
*         mode is full-duplex.
*         When duplex Auto-Negotiation is disabled, the port's duplex mode is set
*         via cpssPxPortDuplexModeSet
*
* @note   APPLICABLE DEVICES:      pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] statePtr                 - GT_TRUE for enable Auto-Negotiation for duplex mode,
*                                      GT_FALSE otherwise
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note CPU port not supports the duplex auto negotiation
*
*/
static GT_STATUS prvCpssPxPortDuplexAutoNegEnableGet
(
    IN  GT_SW_DEV_NUM        devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_BOOL              *statePtr
)
{
    GT_U32 value;       /* value of duplex auto-neg. status */
    GT_U32 portGroupId; /* number of port group for mlti-port group dev's */
    GT_U32 dupledAutoNegRegAddr; /* address of duplex auto-neg. config. reg. */
    GT_U32 portMacNum; /* MAC number */

    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

    /* can not be changed for 10 Gb interface or CPU port */
    if(PRV_CPSS_PX_PORT_TYPE_MAC(devNum,portMacNum) >= PRV_CPSS_PORT_XG_E)
    {
        *statePtr = GT_FALSE;
        return GT_OK;
    }

    PRV_CPSS_PX_PORT_AUTO_NEG_CTRL_REG_MAC(devNum,portMacNum,&dupledAutoNegRegAddr);

    portGroupId = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portMacNum);
    if (prvCpssDrvHwPpPortGroupGetRegField(CAST_SW_DEVNUM(devNum), portGroupId, dupledAutoNegRegAddr,
                                            13, 1, &value) != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);
    }

    *statePtr = BIT2BOOL_MAC(value);

    return GT_OK;
}

/**
* @internal internal_cpssPxPortDuplexAutoNegEnableGet function
* @endinternal
*
* @brief   Get enable/disable status of an Auto-Negotiation for MAC duplex mode
*         per port.
*         When duplex Auto-Negotiation is enabled, the port's duplex mode is
*         resolved via Auto-Negotiation. The Auto-Negotiation advertised duplex
*         mode is full-duplex.
*         When duplex Auto-Negotiation is disabled, the port's duplex mode is set
*         via cpssPxPortDuplexModeSet
*
* @note   APPLICABLE DEVICES:      pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] statePtr                 - GT_TRUE for enable Auto-Negotiation for duplex mode,
*                                      GT_FALSE otherwise
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPortDuplexAutoNegEnableGet
(
    IN  GT_SW_DEV_NUM        devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_BOOL              *statePtr
)
{
    GT_STATUS rc = GT_OK;
    GT_U32 portMacNum; /* MAC number */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(statePtr);
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

    rc = prvCpssPxPortDuplexAutoNegEnableGet(devNum,portNum,statePtr);
    if(rc!=GT_OK)
    {
        return rc;
    }

    return rc;
}

/**
* @internal cpssPxPortDuplexAutoNegEnableGet function
* @endinternal
*
* @brief   Get enable/disable status of an Auto-Negotiation for MAC duplex mode
*         per port.
*         When duplex Auto-Negotiation is enabled, the port's duplex mode is
*         resolved via Auto-Negotiation. The Auto-Negotiation advertised duplex
*         mode is full-duplex.
*         When duplex cpssPxPortDuplexModeSet
*
* @note   APPLICABLE DEVICES:      pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] statePtr                 - GT_TRUE for enable Auto-Negotiation for duplex mode,
*                                      GT_FALSE otherwise
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortDuplexAutoNegEnableGet
(
    IN  GT_SW_DEV_NUM        devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_BOOL              *statePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortDuplexAutoNegEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, statePtr));

    rc = internal_cpssPxPortDuplexAutoNegEnableGet(devNum, portNum, statePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, statePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssPxPortFlowCntrlAutoNegEnableSet function
* @endinternal
*
* @brief   Enable/disable an Auto-Negotiation for Flow Control on
*         specified port on specified device.
*
* @note   APPLICABLE DEVICES:      pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] state                    - GT_TRUE for enable Auto-Negotiation for Flow Control
*                                      GT_FALSE otherwise
* @param[in] pauseAdvertise           - Advertise symmetric flow control support in
*                                      Auto-Negotiation. 0 = Disable, 1 = Enable.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - on not supported state
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note 1.CPU port not supports the flow control auto negotiation
*
*/
static GT_STATUS internal_cpssPxPortFlowCntrlAutoNegEnableSet
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_BOOL                 state,
    IN  GT_BOOL                 pauseAdvertise
)
{
    GT_STATUS   rc = GT_OK;
    GT_U32       portMacNum;      /* MAC number */
    GT_U32      stateValue;      /* value to write into the register */
    GT_U32      pauseAdvValue;   /* value to write into the register */
    PRV_CPSS_PX_PORT_STATE_STC portStateStc;       /* current port state */
    GT_U32      fcAutoNegRegAddr;/* config. reg. address */
    PRV_CPSS_PORT_TYPE_ENT portMacType;
    GT_U32      fcaRegAddr = PRV_CPSS_SW_PTR_ENTRY_UNUSED;  /* register address */
    GT_U32      fcaValue = 0;    /* value to write into the FCA register */
    PRV_CPSS_PORT_REG_CONFIG_STC   regDataArray[PRV_CPSS_PORT_NOT_APPLICABLE_E];

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

    portMacType = PRV_CPSS_PX_PORT_TYPE_MAC(devNum, portMacNum);
    if((portMacType >= PRV_CPSS_PORT_XG_E) || (portNum == CPSS_CPU_PORT_NUM_CNS))
    {/* check if auto-neg. supported for current mode/port*/
        if ((state == GT_TRUE) || (pauseAdvertise == GT_TRUE))
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
        }
    }

    PRV_CPSS_PX_PORT_AUTO_NEG_CTRL_REG_MAC(devNum, portMacNum, &fcAutoNegRegAddr);
    if(fcAutoNegRegAddr == PRV_CPSS_SW_PTR_ENTRY_UNUSED)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);

    if(portMacType <= PRV_CPSS_PORT_GE_E)
    {/* disable port if we need */
        rc = prvCpssPxPortStateDisableAndGet(devNum, portNum, &portStateStc);
        if(rc != GT_OK)
            return rc;
    }

    if(prvCpssPxPortMacConfigurationClear(regDataArray) != GT_OK)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_INIT_ERROR, LOG_ERROR_NO_MSG);

    stateValue = BOOL2BIT_MAC(state);
    pauseAdvValue = BOOL2BIT_MAC(pauseAdvertise);
    if(fcAutoNegRegAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        regDataArray[PRV_CPSS_PORT_GE_E].regAddr = fcAutoNegRegAddr;
        regDataArray[PRV_CPSS_PORT_GE_E].fieldData = stateValue;
        regDataArray[PRV_CPSS_PORT_GE_E].fieldOffset = 11;     /* AnFCEn Enables Auto-Negotiation for Flow */
        regDataArray[PRV_CPSS_PORT_GE_E].fieldLength = 1;
    }

    rc = prvCpssPxPortMacConfiguration(devNum, portNum, regDataArray);
    if(rc != GT_OK)
    {
        return rc;
    }

    if(fcAutoNegRegAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        regDataArray[PRV_CPSS_PORT_GE_E].regAddr = fcAutoNegRegAddr;
        regDataArray[PRV_CPSS_PORT_GE_E].fieldData = pauseAdvValue;  /* PauseAdv This is the Flow Control advertise  */
        regDataArray[PRV_CPSS_PORT_GE_E].fieldOffset = 9;
        regDataArray[PRV_CPSS_PORT_GE_E].fieldLength = 1;
    }

    rc = prvCpssPxPortMacConfiguration(devNum, portNum, regDataArray);
    if(rc != GT_OK)
    {
        return rc;
    }

    if(portMacType <= PRV_CPSS_PORT_GE_E)
    {
        /* restore port enable state */
        rc = prvCpssPxPortStateRestore(devNum, portNum, &portStateStc);
        if(rc != GT_OK)
            return rc;
    }

    fcaRegAddr = PRV_PX_REG1_UNIT_GOP_FCA_MAC(devNum, portMacNum).FCACtrl;
    /* <FCA <enable/disable> bit configuration */
    if((GT_TRUE == state) && (GT_TRUE == pauseAdvertise))
    {
        fcaValue = 0;
        if (prvCpssDrvHwPpPortGroupSetRegField(CAST_SW_DEVNUM(devNum),
                                               PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portMacNum),
                                               fcaRegAddr, 1, 1, fcaValue) != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);
        }
    }

    return GT_OK;
}

/**
* @internal cpssPxPortFlowCntrlAutoNegEnableSet function
* @endinternal
*
* @brief   Enable/disable an Auto-Negotiation for Flow Control on
*         specified port on specified device.
*
* @note   APPLICABLE DEVICES:      pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] state                    - GT_TRUE for enable Auto-Negotiation for Flow Control
*                                      GT_FALSE otherwise
* @param[in] pauseAdvertise           - Advertise symmetric flow control support in
*                                      Auto-Negotiation. 0 = Disable, 1 = Enable.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - on not supported state
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note 1.CPU port not supports the flow control auto negotiation
*
*/
GT_STATUS cpssPxPortFlowCntrlAutoNegEnableSet
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_BOOL                 state,
    IN  GT_BOOL                 pauseAdvertise
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortFlowCntrlAutoNegEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, state, pauseAdvertise));

    rc = internal_cpssPxPortFlowCntrlAutoNegEnableSet(devNum, portNum, state, pauseAdvertise);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, state, pauseAdvertise));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortFlowCntrlAutoNegEnableGet function
* @endinternal
*
* @brief   Get Auto-Negotiation enable/disable state for Flow Control per port
*
* @note   APPLICABLE DEVICES:      pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] statePtr                 -   GT_TRUE for enable Auto-Negotiation for Flow Control
*                                      GT_FALSE otherwise
* @param[out] pauseAdvertisePtr        - Advertise symmetric flow control support in
*                                      Auto-Negotiation. 0 = Disable, 1 = Enable.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note CPU port not supports the flow control auto negotiation
*
*/
static GT_STATUS internal_cpssPxPortFlowCntrlAutoNegEnableGet
(
    IN   GT_SW_DEV_NUM          devNum,
    IN   GT_PHYSICAL_PORT_NUM   portNum,
    OUT  GT_BOOL                *statePtr,
    OUT  GT_BOOL                *pauseAdvertisePtr
)
{
    GT_STATUS   rc = GT_OK;
    GT_U32      portMacNum;      /* MAC number */
    GT_U32      portGroupId;
    GT_U32      fcAutoNegRegAddr;/* config. reg. address */
    GT_U32      value = 0;

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(statePtr);
    CPSS_NULL_PTR_CHECK_MAC(pauseAdvertisePtr);
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

    portGroupId = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portMacNum);

    if((PRV_CPSS_PX_PORT_TYPE_MAC(devNum,portMacNum) >= PRV_CPSS_PORT_XG_E) ||
       (portNum == CPSS_CPU_PORT_NUM_CNS))
    {
        *statePtr = GT_FALSE;
        *pauseAdvertisePtr = GT_FALSE;
        return GT_OK;
    }

    PRV_CPSS_PX_PORT_AUTO_NEG_CTRL_REG_MAC(devNum, portMacNum, &fcAutoNegRegAddr);
    if (prvCpssDrvHwPpPortGroupGetRegField(CAST_SW_DEVNUM(devNum), portGroupId, fcAutoNegRegAddr,
                                            11, 1, &value) != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);
    }

    *statePtr = BIT2BOOL_MAC(value);

    if (prvCpssDrvHwPpPortGroupGetRegField(CAST_SW_DEVNUM(devNum), portGroupId, fcAutoNegRegAddr,
                                            9, 1, &value) != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);
    }

    *pauseAdvertisePtr = BIT2BOOL_MAC(value);

    return rc;
}

/**
* @internal cpssPxPortFlowCntrlAutoNegEnableGet function
* @endinternal
*
* @brief   Get Auto-Negotiation enable/disable state for Flow Control per port
*
* @note   APPLICABLE DEVICES:      pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] statePtr                 -   GT_TRUE for enable Auto-Negotiation for Flow Control
*                                      GT_FALSE otherwise
* @param[out] pauseAdvertisePtr        - Advertise symmetric flow control support in
*                                      Auto-Negotiation. 0 = Disable, 1 = Enable.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note CPU port not supports the flow control auto negotiation
*
*/
GT_STATUS cpssPxPortFlowCntrlAutoNegEnableGet
(
    IN   GT_SW_DEV_NUM          devNum,
    IN   GT_PHYSICAL_PORT_NUM   portNum,
    OUT  GT_BOOL                *statePtr,
    OUT  GT_BOOL                *pauseAdvertisePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortFlowCntrlAutoNegEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, statePtr, pauseAdvertisePtr));

    rc = internal_cpssPxPortFlowCntrlAutoNegEnableGet(devNum, portNum, statePtr, pauseAdvertisePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, statePtr, pauseAdvertisePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssPxPortFlowControlEnableSet function
* @endinternal
*
* @brief   Enable/disable receiving and transmission of 802.3x Flow Control frames
*         in full duplex on specified port on specified device.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number (or CPU port)
* @param[in] state                    - Flow Control state: Both disabled,
*                                      Both enabled, Only Rx or Only Tx enabled.
*                                      Note: only XG ports can be configured in all 4 options,
*                                      Tri-Speed and FE ports may use only first two.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number, state or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPortFlowControlEnableSet
(
    IN  GT_SW_DEV_NUM               devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    IN  CPSS_PORT_FLOW_CONTROL_ENT  state
)
{
    GT_STATUS   rc = GT_OK;
    GT_U32      portMacNum;      /* MAC number */
    GT_U32      regAddr;         /* register address */
    GT_U32      fcaRegAddr = PRV_CPSS_SW_PTR_ENTRY_UNUSED; /* register address */
    GT_U32      value;           /* value to write into the register */
    GT_U32      fcaValue = 0;    /* value to write into the FCA register */
    GT_U32      offset;          /* bit number inside register       */
    GT_U32      length;          /* number of bits to be written to the register */
    PRV_CPSS_PX_PORT_STATE_STC       portStateStc;       /* current port state */
    PRV_CPSS_PORT_REG_CONFIG_STC  regDataArray[PRV_CPSS_PORT_NOT_APPLICABLE_E];

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

    if(PRV_CPSS_PX_PORT_TYPE_MAC(devNum,portMacNum) <= PRV_CPSS_PORT_GE_E)
    {
        /* disable port if current interface is FE or GE */
        if (prvCpssPxPortStateDisableAndGet(devNum, portNum, &portStateStc) != GT_OK)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);
    }

    if(prvCpssPxPortMacConfigurationClear(regDataArray) != GT_OK)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_INIT_ERROR, LOG_ERROR_NO_MSG);

    fcaRegAddr = PRV_PX_REG1_UNIT_GOP_FCA_MAC(devNum, portMacNum).FCACtrl;

    /* FCA unit handles TX FCA. '1' is Bypass and means disable TX FC. '0' means enable TX FCA. */
    switch (state)
    {
        case CPSS_PORT_FLOW_CONTROL_DISABLE_E:
            fcaValue = 1;
            break;
        case CPSS_PORT_FLOW_CONTROL_RX_TX_E:
            fcaValue = 0;
            break;
        case CPSS_PORT_FLOW_CONTROL_RX_ONLY_E:
            fcaValue = 1;
            break;
        case CPSS_PORT_FLOW_CONTROL_TX_ONLY_E:
            fcaValue = 0;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* <FCA TX disable> bit configuration */
    if (prvCpssDrvHwPpPortGroupSetRegField(CAST_SW_DEVNUM(devNum),
                                           PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portMacNum),
                                           fcaRegAddr, 1, 1, fcaValue) != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);
    }

    PRV_CPSS_PX_PORT_MAC_CTRL4_REG_MAC(devNum,portMacNum,PRV_CPSS_PORT_GE_E,&regAddr);
    offset = 3;
    length = 2;

    switch (state)
    {
        case CPSS_PORT_FLOW_CONTROL_DISABLE_E:
            value = 0;
            break;
        case CPSS_PORT_FLOW_CONTROL_RX_TX_E:
            value = 3;
            break;
        case CPSS_PORT_FLOW_CONTROL_RX_ONLY_E:
            value = 1;
            break;
        case CPSS_PORT_FLOW_CONTROL_TX_ONLY_E:
            value = 2;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if(regAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        regDataArray[PRV_CPSS_PORT_GE_E].regAddr = regAddr;
        regDataArray[PRV_CPSS_PORT_GE_E].fieldData = value;
        regDataArray[PRV_CPSS_PORT_GE_E].fieldOffset = offset;
        regDataArray[PRV_CPSS_PORT_GE_E].fieldLength = length;
    }

    rc = prvCpssPxPortMacConfiguration(devNum, portNum, regDataArray);
    if(rc != GT_OK)
    {
        return rc;
    }

    offset = 7;
    value &= 0x1; /* for XLG ports RX is configured via MAC - TX is controlled by FCA unit */
    length = 2;

    PRV_CPSS_PX_PORT_MAC_CTRL_REG_MAC(devNum,portMacNum,PRV_CPSS_PORT_XG_E,&regAddr);
    if(regAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        regDataArray[PRV_CPSS_PORT_XG_E].regAddr = regAddr;
        regDataArray[PRV_CPSS_PORT_XG_E].fieldData = value;
        regDataArray[PRV_CPSS_PORT_XG_E].fieldOffset = offset;
        regDataArray[PRV_CPSS_PORT_XG_E].fieldLength = length;
    }

    PRV_CPSS_PX_PORT_MAC_CTRL_REG_MAC(devNum,portMacNum,PRV_CPSS_PORT_XLG_E,&regAddr);
    if(regAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        regDataArray[PRV_CPSS_PORT_XLG_E].regAddr = regAddr;
        regDataArray[PRV_CPSS_PORT_XLG_E].fieldData = value;
        regDataArray[PRV_CPSS_PORT_XLG_E].fieldOffset = offset;
        regDataArray[PRV_CPSS_PORT_XLG_E].fieldLength = length;
    }

    regAddr = PRV_PX_REG1_UNIT_CG_CONVERTERS_MAC(devNum,portMacNum).CGMAConvertersFcControl0;
    if(regAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        regDataArray[PRV_CPSS_PORT_CG_E].regAddr = regAddr;
        regDataArray[PRV_CPSS_PORT_CG_E].fieldData = value;
        regDataArray[PRV_CPSS_PORT_CG_E].fieldLength = 1;
        regDataArray[PRV_CPSS_PORT_CG_E].fieldOffset = 15;/*<Enable 802.3x FC Tx halting>*/
    }

    rc = prvCpssPxPortMacConfiguration(devNum, portNum, regDataArray);
    if(rc != GT_OK)
    {
        return rc;
    }

    if(PRV_CPSS_PX_PORT_TYPE_MAC(devNum,portMacNum) <= PRV_CPSS_PORT_GE_E)
    {
        /* restore port enable state */
        if (prvCpssPxPortStateRestore(devNum, portNum, &portStateStc) != GT_OK)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);
    }

    return GT_OK;
}

/**
* @internal cpssPxPortFlowControlEnableSet function
* @endinternal
*
* @brief   Enable/disable receiving and transmission of 802.3x Flow Control frames
*         in full duplex on specified port on specified device.
*
* @note   APPLICABLE DEVICES:      pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number (or CPU port)
* @param[in] state                    - Flow Control state: Both disabled,
*                                      Both enabled, Only Rx or Only Tx enabled.
*                                      Note: only XG ports can be configured in all 4 options,
*                                      Tri-Speed  ports may use only first two.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number, state or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Before calling cpssPxPortFlowControlEnableSet,
*       cpssPxPortPeriodicFlowControlCounterSet should be called
*       to set the interval between the transmission of two consecutive
*       Flow Control packets according to port speed
*
*/
GT_STATUS cpssPxPortFlowControlEnableSet
(
    IN  GT_SW_DEV_NUM               devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    IN  CPSS_PORT_FLOW_CONTROL_ENT  state
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortFlowControlEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, state));

    rc = internal_cpssPxPortFlowControlEnableSet(devNum, portNum, state);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, state));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortFlowControlEnableGet function
* @endinternal
*
* @brief   Get status of 802.3x Flow Control on specific logical port.
*
* @note   APPLICABLE DEVICES:      pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number (or CPU port)
*
* @param[out] statePtr                 - Pointer to Flow Control state: Both disabled,
*                                      Both enabled, Only Rx or Only Tx enabled.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPortFlowControlEnableGet
(
    IN  GT_SW_DEV_NUM                   devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    OUT CPSS_PORT_FLOW_CONTROL_ENT      *statePtr
)
{
    GT_U32      portMacNum;      /* MAC number */
    GT_U32      value;
    GT_U32      fcaValue = 1;
    GT_U32      regAddr;
    GT_U32      fcaRegAddr;
    GT_U32      fieldOffset;
    GT_U32      fieldLength;
    GT_U32      portGroup;
    PRV_CPSS_PORT_TYPE_ENT portMacType;


    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum,portMacNum);
    CPSS_NULL_PTR_CHECK_MAC(statePtr);

    portMacType = PRV_CPSS_PX_PORT_TYPE_MAC(devNum,portMacNum);
    portGroup = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portMacNum);

    fcaRegAddr = PRV_PX_REG1_UNIT_GOP_FCA_MAC(devNum, portMacNum).FCACtrl;
    if (prvCpssDrvHwPpPortGroupGetRegField(CAST_SW_DEVNUM(devNum), portGroup, fcaRegAddr, 1,
                                           1, &fcaValue) != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);
    }

    fieldLength = 2;
    if (portMacType >= PRV_CPSS_PORT_XG_E)
    {
        if(portMacType == PRV_CPSS_PORT_CG_E)
        {
            regAddr = PRV_PX_REG1_UNIT_CG_CONVERTERS_MAC(devNum,portMacNum).CGMAConvertersFcControl0;
            fieldOffset = 15;/*<Forward 802.3x FC Enable>*/
            fieldLength = 1;
        }
        else
        {
            PRV_CPSS_PX_PORT_MAC_CTRL_REG_MAC(devNum,portMacNum,portMacType,&regAddr);
            fieldOffset = 7;
        }
    }
    else
    {
        PRV_CPSS_PX_PORT_STATUS_CTRL_REG_MAC(devNum, portMacNum, &regAddr);
        fieldOffset = 4;
    }

    if(PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);

    if (prvCpssDrvHwPpPortGroupGetRegField(CAST_SW_DEVNUM(devNum), portGroup, regAddr,
                                           fieldOffset, fieldLength, &value) != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);
    }

    /* combine Rx and Tx values */
    value |= ((~fcaValue & 0x1) << 1);

    switch (value)
    {
        case 0:
            *statePtr = CPSS_PORT_FLOW_CONTROL_DISABLE_E;
            break;
        case 1:
            *statePtr = CPSS_PORT_FLOW_CONTROL_RX_ONLY_E;
            break;
        case 2:
            *statePtr = CPSS_PORT_FLOW_CONTROL_TX_ONLY_E;
            break;
        case 3:
            *statePtr = CPSS_PORT_FLOW_CONTROL_RX_TX_E;
            break;
        default:
            /* no chance getting here */
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    return GT_OK;
}

/**
* @internal cpssPxPortFlowControlEnableGet function
* @endinternal
*
* @brief   Get status of 802.3x Flow Control on specific logical port.
*
* @note   APPLICABLE DEVICES:      pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number (or CPU port)
*
* @param[out] statePtr                 - Pointer to Flow Control state: Both disabled,
*                                      Both enabled, Only Rx or Only Tx enabled.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortFlowControlEnableGet
(
    IN  GT_SW_DEV_NUM                   devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    OUT CPSS_PORT_FLOW_CONTROL_ENT      *statePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortFlowControlEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, statePtr));

    rc = internal_cpssPxPortFlowControlEnableGet(devNum, portNum, statePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, statePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssPxPortFlowControlModeSet function
* @endinternal
*
* @brief   Sets Flow Control mode on given port.
*
* @note   APPLICABLE DEVICES:      pipe.
* @param[in] devNum                   - device number.
* @param[in] portNum                  - port number, CPU port number.
* @param[in] fcMode                   - flow control mode.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note This function also configures insertion of DSA tag for PFC frames.
*
*/
static GT_STATUS internal_cpssPxPortFlowControlModeSet
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  CPSS_PX_PORT_FC_MODE_ENT fcMode
)
{
    GT_U32      regAddr;    /* register address */
    GT_U32      fieldOffset;/* the start bit number in the register */
    GT_U32      value, oppcodeValue = 0;      /* register value */
    GT_STATUS   rc;      /* function return value */
    GT_U32      portMacNum; /* MAC number */
    PRV_CPSS_PORT_REG_CONFIG_STC   regDataArray[PRV_CPSS_PORT_NOT_APPLICABLE_E];
    GT_BOOL     enablePfcCascade;


    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

    switch (fcMode)
    {
        case CPSS_PX_PORT_FC_MODE_802_3X_E:
            value = 0;
            oppcodeValue = 0x1;
            break;

        case CPSS_PX_PORT_FC_MODE_PFC_E:
            value = 1;
            oppcodeValue = 0x101;
            break;

        case CPSS_PX_PORT_FC_MODE_LL_FC_E:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
            break;

        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* use FCA unit for TX FC mode configuration (only if supported)
    Note: RX FC mode is configured to the same value via MAC registers */
    regAddr = PRV_PX_REG1_UNIT_GOP_FCA_MAC(devNum, portMacNum).FCACtrl;
    fieldOffset = 2;
    /* set FC_MODE */
    rc =  prvCpssHwPpSetRegField(CAST_SW_DEVNUM(devNum), regAddr, fieldOffset, 2, value);
    if (rc != GT_OK)
    {
        return rc;
    }

    regAddr = PRV_PX_REG1_UNIT_GOP_FCA_MAC(devNum, portMacNum).L2CtrlReg1;
    fieldOffset = 0;
    /* set OPPCODE */
    rc =  prvCpssHwPpSetRegField(CAST_SW_DEVNUM(devNum), regAddr, fieldOffset, 16, oppcodeValue);
    if (rc != GT_OK)
    {
        return rc;
    }

    if(prvCpssPxPortMacConfigurationClear(regDataArray) != GT_OK)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_INIT_ERROR, LOG_ERROR_NO_MSG);

    PRV_CPSS_PX_PORT_MAC_CTRL2_REG_MAC(devNum,portMacNum,PRV_CPSS_PORT_GE_E,&regAddr);
    if(regAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        regDataArray[PRV_CPSS_PORT_GE_E].regAddr = regAddr;
        regDataArray[PRV_CPSS_PORT_GE_E].fieldData = value;
        regDataArray[PRV_CPSS_PORT_GE_E].fieldLength = 2;
        regDataArray[PRV_CPSS_PORT_GE_E].fieldOffset = 1;
    }

    PRV_CPSS_PX_PORT_MAC_CTRL2_REG_MAC(devNum,portMacNum,PRV_CPSS_PORT_XG_E,&regAddr);
    if(regAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        regDataArray[PRV_CPSS_PORT_XG_E].regAddr = regAddr;
        regDataArray[PRV_CPSS_PORT_XG_E].fieldData = value;
        regDataArray[PRV_CPSS_PORT_XG_E].fieldLength = 2;
        regDataArray[PRV_CPSS_PORT_XG_E].fieldOffset = 14;
    }

    PRV_CPSS_PX_PORT_MAC_CTRL2_REG_MAC(devNum,portMacNum,PRV_CPSS_PORT_XLG_E,&regAddr);
    if(regAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        regDataArray[PRV_CPSS_PORT_XLG_E].regAddr = regAddr;
        regDataArray[PRV_CPSS_PORT_XLG_E].fieldData = value;
        regDataArray[PRV_CPSS_PORT_XLG_E].fieldLength = 2;
        regDataArray[PRV_CPSS_PORT_XLG_E].fieldOffset = 14;
    }

    rc = prvCpssPxPortMacConfiguration(devNum, portNum, regDataArray);
    if (rc != GT_OK)
    {
        return rc;
    }
    /* for cascade port and pfc mode, enable insertion of DSA tag to PFC packets */
    if (fcMode == CPSS_PX_PORT_FC_MODE_PFC_E)
    {
        /* get cascade port configuration */
        rc = cpssPxIngressPortDsaTagEnableGet(devNum,portNum,&enablePfcCascade);
        if (rc != GT_OK)
        {
            return rc;
        }
        rc = prvCpssPxPortPfcCascadeEnableSet(devNum,portNum,enablePfcCascade);
    }
   return rc;
}

/**
* @internal cpssPxPortFlowControlModeSet function
* @endinternal
*
* @brief   Sets Flow Control mode on given port.
*
* @note   APPLICABLE DEVICES:      pipe.
* @param[in] devNum                   - device number.
* @param[in] portNum                  - port number, CPU port number.
* @param[in] fcMode                   - flow control mode.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note This function also configures insertion of DSA tag for PFC frames.
*
*/
GT_STATUS cpssPxPortFlowControlModeSet
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  CPSS_PX_PORT_FC_MODE_ENT fcMode
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortFlowControlModeSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, fcMode));

    rc = internal_cpssPxPortFlowControlModeSet(devNum, portNum, fcMode);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, fcMode));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssPxPortFlowControlModeGet function
* @endinternal
*
* @brief   Gets Flow Control mode on given port.
*
* @note   APPLICABLE DEVICES:      pipe.
* @param[in] devNum                   - device number.
* @param[in] portNum                  - port number, CPU port number.
*
* @param[out] fcModePtr                - flow control mode.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPortFlowControlModeGet
(
    IN  GT_SW_DEV_NUM               devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    OUT CPSS_PX_PORT_FC_MODE_ENT    *fcModePtr
)
{

    GT_U32 regAddr;    /* register address */
    GT_U32 fieldOffset;/* the start bit number in the register */
    GT_U32 value;      /* register value */
    GT_STATUS rc;
    GT_U32 portMacNum; /* MAC number */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);
    CPSS_NULL_PTR_CHECK_MAC(fcModePtr);

    /* use FCA unit (instead of MAC) for FC mode configuration (only if supported) */
    regAddr = PRV_PX_REG1_UNIT_GOP_FCA_MAC(devNum, portMacNum).FCACtrl;
    fieldOffset = 2;

    /* get FC_MODE */
    rc =  prvCpssHwPpGetRegField(CAST_SW_DEVNUM(devNum), regAddr, fieldOffset, 2, &value);
    if (rc != GT_OK)
    {
        return rc;
    }

    *fcModePtr = (CPSS_PX_PORT_FC_MODE_ENT)value;

    return GT_OK;
}

/**
* @internal cpssPxPortFlowControlModeGet function
* @endinternal
*
* @brief   Gets Flow Control mode on given port.
*
* @note   APPLICABLE DEVICES:      pipe.
* @param[in] devNum                   - device number.
* @param[in] portNum                  - port number, CPU port number.
*
* @param[out] fcModePtr                - flow control mode.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortFlowControlModeGet
(
    IN  GT_SW_DEV_NUM               devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    OUT CPSS_PX_PORT_FC_MODE_ENT  *fcModePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortFlowControlModeGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, fcModePtr));

    rc = internal_cpssPxPortFlowControlModeGet(devNum, portNum, fcModePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, fcModePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/*******************************************************************************
* prvCpssPxPortDuplexModeSet
*
* DESCRIPTION:
*       Set the port mode to half- or full-duplex mode when duplex autonegotiation is disabled.
*
* APPLICABLE DEVICES:
*        pipe.
*
* NOT APPLICABLE DEVICES:
*        None.
*
*
* INPUTS:
*       devNum   - physical device number
*       portNum  - physical port number or CPU port
*       dMode    - duplex mode.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - on wrong port number or device or dMode
*       GT_HW_ERROR              - on hardware error
*       GT_NOT_SUPPORTED         - on not supported duplex mode
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       None.
*
*******************************************************************************/
#define PRV_CPSS_PX_PORT_MAC_DUPLEX_FLD_LEN_CNS   1
#define PRV_CPSS_PX_PORT_MAC_DUPLEX_FLD_OFFSET_GE_CNS   12
static GT_STATUS prvCpssPxPortDuplexModeSet
(
    IN  GT_SW_DEV_NUM         devNum,
    IN  GT_PHYSICAL_PORT_NUM  portNum,
    IN  CPSS_PORT_DUPLEX_ENT  dMode
)
{
    GT_STATUS       rc;         /* return code */
    PRV_CPSS_PX_PORT_STATE_STC portStateStc;       /* current port state */
    GT_U32          duplexModeRegAddr; /* config. reg. address */
    PRV_CPSS_PORT_TYPE_ENT portMacType; /* mac type to use */
    GT_U32          portMacNum; /* MAC number */
    PRV_CPSS_PORT_REG_CONFIG_STC   regDataArray[PRV_CPSS_PORT_NOT_APPLICABLE_E];

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum,portMacNum);
    if (dMode != CPSS_PORT_FULL_DUPLEX_E &&
        dMode != CPSS_PORT_HALF_DUPLEX_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    portMacType = PRV_CPSS_PX_PORT_TYPE_MAC(devNum,portMacNum);
    if(portMacType >= PRV_CPSS_PORT_XG_E)
    {
        if (dMode == CPSS_PORT_HALF_DUPLEX_E)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
        }
    }

    if(prvCpssPxPortMacConfigurationClear(regDataArray) != GT_OK)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_INIT_ERROR, LOG_ERROR_NO_MSG);

    PRV_CPSS_PX_PORT_AUTO_NEG_CTRL_REG_MAC(devNum,portMacNum,&duplexModeRegAddr);
    if(duplexModeRegAddr == PRV_CPSS_SW_PTR_ENTRY_UNUSED)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);

    if(PRV_CPSS_PX_PORT_TYPE_MAC(devNum,portMacNum) <= PRV_CPSS_PORT_GE_E)
    {/* disable port if we need */
        rc = prvCpssPxPortStateDisableAndGet(devNum,portNum,&portStateStc);
        if(rc != GT_OK)
            return rc;
    }

    regDataArray[PRV_CPSS_PORT_GE_E].regAddr = duplexModeRegAddr;
    regDataArray[PRV_CPSS_PORT_GE_E].fieldData = (dMode == CPSS_PORT_FULL_DUPLEX_E) ? 1 : 0;
    regDataArray[PRV_CPSS_PORT_GE_E].fieldOffset = PRV_CPSS_PX_PORT_MAC_DUPLEX_FLD_OFFSET_GE_CNS;
    regDataArray[PRV_CPSS_PORT_GE_E].fieldLength = PRV_CPSS_PX_PORT_MAC_DUPLEX_FLD_LEN_CNS;

    rc = prvCpssPxPortMacConfiguration(devNum, portNum, regDataArray);
    if(rc != GT_OK)
    {
        return rc;
    }

    if(PRV_CPSS_PX_PORT_TYPE_MAC(devNum,portMacNum) <= PRV_CPSS_PORT_GE_E)
    {/* restore port enable state */
        rc = prvCpssPxPortStateRestore(devNum,portNum, &portStateStc);
        if(rc != GT_OK)
            return rc;
    }

    return GT_OK;
}

/**
* @internal internal_cpssPxPortDuplexModeSet function
* @endinternal
*
* @brief   Set the port mode to half- or full-duplex mode when duplex autonegotiation is disabled.
*
* @note   APPLICABLE DEVICES:      pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number or CPU port
* @param[in] dMode                    - duplex mode.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device or dMode
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - on not supported duplex mode
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
static GT_STATUS internal_cpssPxPortDuplexModeSet
(
    IN  GT_SW_DEV_NUM         devNum,
    IN  GT_PHYSICAL_PORT_NUM  portNum,
    IN  CPSS_PORT_DUPLEX_ENT  dMode
)
{
    CPSS_PORT_DUPLEX_ENT  targetdMode = dMode;   /* mode value - set by callback */
    GT_STATUS rc = GT_OK;
    GT_U32 portMacNum; /* MAC number */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

    if (dMode != CPSS_PORT_FULL_DUPLEX_E &&
        dMode != CPSS_PORT_HALF_DUPLEX_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if(PRV_CPSS_PX_PP_MAC(devNum)->genInfo.hitlessWriteMethodEnable)
    {
        CPSS_PORT_DUPLEX_ENT   currentState;

        rc = cpssPxPortDuplexModeGet(devNum, portNum, &currentState);
        if(rc != GT_OK)
        {
            return rc;
        }
        if(dMode == currentState)
        {
            return GT_OK;
        }
    }

    /* switch MAC configuration */
    rc = prvCpssPxPortDuplexModeSet(devNum, portNum, targetdMode);
    if(rc!=GT_OK)
    {
        return rc;
    }
    return rc;
}

/**
* @internal cpssPxPortDuplexModeSet function
* @endinternal
*
* @brief   Set the port mode to half- or full-duplex mode when duplex autonegotiation is disabled.
*
* @note   APPLICABLE DEVICES:      pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number or CPU port
* @param[in] dMode                    - duplex mode.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device or dMode
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - on not supported duplex mode
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssPxPortDuplexModeSet
(
    IN  GT_SW_DEV_NUM         devNum,
    IN  GT_PHYSICAL_PORT_NUM  portNum,
    IN  CPSS_PORT_DUPLEX_ENT  dMode
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortDuplexModeSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, dMode));

    rc = internal_cpssPxPortDuplexModeSet(devNum, portNum, dMode);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, dMode));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal prvCpssPxPortDuplexModeGet function
* @endinternal
*
* @brief   Gets duplex mode for specified port on specified device.
*
* @note   APPLICABLE DEVICES:      pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number (or CPU port)
*
* @param[out] dModePtr                 - duplex mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS prvCpssPxPortDuplexModeGet
(
    IN  GT_SW_DEV_NUM         devNum,
    IN  GT_PHYSICAL_PORT_NUM  portNum,
    OUT CPSS_PORT_DUPLEX_ENT  *dModePtr
)
{
    GT_U32 value;
    GT_U32 offset;          /* bit number inside register       */
    GT_U32 duplexModeRegAddr;
    GT_STATUS rc;
    CPSS_PORT_SPEED_ENT speed;
    GT_U32 portMacNum; /* MAC number */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum,portMacNum);
    CPSS_NULL_PTR_CHECK_MAC(dModePtr);

     /* can not be changed for 1G and above */
    if((rc = cpssPxPortSpeedGet(devNum, portNum, &speed)) != GT_OK)
        return rc;

    if(speed > CPSS_PORT_SPEED_1000_E)
    {
        *dModePtr = CPSS_PORT_FULL_DUPLEX_E;
        return GT_OK;
    }

    PRV_CPSS_PX_PORT_STATUS_CTRL_REG_MAC(devNum, portMacNum, &duplexModeRegAddr);
    if(PRV_CPSS_SW_PTR_ENTRY_UNUSED == duplexModeRegAddr)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);

    offset = 3;

    if (prvCpssDrvHwPpPortGroupGetRegField(CAST_SW_DEVNUM(devNum),
                                           PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portMacNum),
                                           duplexModeRegAddr,
                                           offset,
                                           1,
                                           &value) != GT_OK)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);

    *dModePtr = (value == 1) ? CPSS_PORT_FULL_DUPLEX_E : CPSS_PORT_HALF_DUPLEX_E;

    return GT_OK;
}

/**
* @internal internal_cpssPxPortDuplexModeGet function
* @endinternal
*
* @brief   Gets duplex mode for specified port on specified device.
*
* @note   APPLICABLE DEVICES:      pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] dModePtr                 - duplex mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPortDuplexModeGet
(
    IN  GT_SW_DEV_NUM         devNum,
    IN  GT_PHYSICAL_PORT_NUM  portNum,
    OUT CPSS_PORT_DUPLEX_ENT  *dModePtr
)
{
    GT_STATUS rc = GT_OK;
    GT_U32 portMacNum; /* MAC number */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);
    CPSS_NULL_PTR_CHECK_MAC(dModePtr);

/* get value from switch MAC */
    rc = prvCpssPxPortDuplexModeGet(devNum, portNum, dModePtr);
    if(rc!=GT_OK)
    {
        return rc;
    }

    return rc;
}

/**
* @internal cpssPxPortDuplexModeGet function
* @endinternal
*
* @brief   Gets duplex mode for specified port on specified device.
*
* @note   APPLICABLE DEVICES:      pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number (or CPU port)
*
* @param[out] dModePtr                 - duplex mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortDuplexModeGet
(
    IN  GT_SW_DEV_NUM         devNum,
    IN  GT_PHYSICAL_PORT_NUM  portNum,
    OUT CPSS_PORT_DUPLEX_ENT  *dModePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortDuplexModeGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, dModePtr));

    rc = internal_cpssPxPortDuplexModeGet(devNum, portNum, dModePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, dModePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortAttributesOnPortGet function
* @endinternal
*
* @brief   Gets port attributes for particular logical port.
*
* @note   APPLICABLE DEVICES:      pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number (or CPU port)
*
* @param[out] portAttributSetArrayPtr  - Pointer to attributes values array.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPortAttributesOnPortGet
(
    IN    GT_SW_DEV_NUM             devNum,
    IN    GT_PHYSICAL_PORT_NUM      portNum,
    OUT   CPSS_PORT_ATTRIBUTES_STC  *portAttributSetArrayPtr
)
{
    GT_BOOL              portLinkUp;      /* port attributes */
    CPSS_PORT_SPEED_ENT  portSpeed;       /* port attributes */
    CPSS_PORT_DUPLEX_ENT portDuplexity;   /* port attributes */
    GT_STATUS            rc;              /* return code     */
    GT_U32               portMacNum;      /* MAC number */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);
    CPSS_NULL_PTR_CHECK_MAC(portAttributSetArrayPtr);

    /* get port attributes */
    rc = cpssPxPortLinkStatusGet(devNum, portNum, &portLinkUp);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = cpssPxPortSpeedGet(devNum, portNum, &portSpeed);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = cpssPxPortDuplexModeGet(devNum ,portNum ,&portDuplexity);
    if (rc != GT_OK)
    {
        return rc;
    }

    portAttributSetArrayPtr->portLinkUp     = portLinkUp;
    portAttributSetArrayPtr->portSpeed      = portSpeed;
    portAttributSetArrayPtr->portDuplexity  = portDuplexity;

    return GT_OK;
}

/**
* @internal cpssPxPortAttributesOnPortGet function
* @endinternal
*
* @brief   Gets port attributes for particular logical port.
*
* @note   APPLICABLE DEVICES:      pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number (or CPU port)
*
* @param[out] portAttributSetArrayPtr  - Pointer to attributes values array.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortAttributesOnPortGet
(
    IN    GT_SW_DEV_NUM             devNum,
    IN    GT_PHYSICAL_PORT_NUM      portNum,
    OUT   CPSS_PORT_ATTRIBUTES_STC  *portAttributSetArrayPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortAttributesOnPortGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, portAttributSetArrayPtr));

    rc = internal_cpssPxPortAttributesOnPortGet(devNum, portNum, portAttributSetArrayPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, portAttributSetArrayPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortSpeedAutoNegEnableSet function
* @endinternal
*
* @brief   Enable/disable an Auto-Negotiation of interface speed on specified port
*         on specified device.
*
* @note   APPLICABLE DEVICES:      pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] state                    - GT_TRUE for enable Auto-Negotiation of interface speed,
*                                      GT_FALSE otherwise
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - on not supported state
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note 1.CPU port not supports the speed auto negotiation
*
*/
static GT_STATUS internal_cpssPxPortSpeedAutoNegEnableSet
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_BOOL                 state
)
{
    GT_STATUS   rc = GT_OK;
    GT_U32      portMacNum;      /* MAC number */
    GT_U32      speedAutoNegRegAddr; /* address of speed auto-neg. config. reg. */
    PRV_CPSS_PX_PORT_STATE_STC  portStateStc;       /* current port state */
    PRV_CPSS_PORT_TYPE_ENT       portMacType; /* port mac unit to use */
    PRV_CPSS_PORT_REG_CONFIG_STC   regDataArray[PRV_CPSS_PORT_NOT_APPLICABLE_E];

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

    if(PRV_CPSS_PX_PP_MAC(devNum)->genInfo.hitlessWriteMethodEnable)
    {
        GT_BOOL   currentState;

        rc = cpssPxPortSpeedAutoNegEnableGet(devNum, portNum, &currentState);
        if(rc != GT_OK)
        {
            return rc;
        }
        if(state == currentState)
        {
            return GT_OK;
        }
    }

    portMacType = PRV_CPSS_PX_PORT_TYPE_MAC(devNum,portMacNum);
    if(portMacType >= PRV_CPSS_PORT_XG_E)
    {/* check if auto-neg. supported for current mode/port*/
        if (state == GT_TRUE)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
        }
    }

    PRV_CPSS_PX_PORT_AUTO_NEG_CTRL_REG_MAC(devNum, portMacNum, &speedAutoNegRegAddr);
    if(speedAutoNegRegAddr == PRV_CPSS_SW_PTR_ENTRY_UNUSED)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);

    if(portMacType <= PRV_CPSS_PORT_GE_E)
    {/* disable port if we need */
        rc = prvCpssPxPortStateDisableAndGet(devNum,portNum,&portStateStc);
        if(rc != GT_OK)
            return rc;
    }

    if(prvCpssPxPortMacConfigurationClear(regDataArray) != GT_OK)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_INIT_ERROR, LOG_ERROR_NO_MSG);

    if(speedAutoNegRegAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        regDataArray[PRV_CPSS_PORT_GE_E].regAddr = speedAutoNegRegAddr;
        regDataArray[PRV_CPSS_PORT_GE_E].fieldData = BOOL2BIT_MAC(state);
        regDataArray[PRV_CPSS_PORT_GE_E].fieldOffset = 7;
        regDataArray[PRV_CPSS_PORT_GE_E].fieldLength = 1;
    }

    rc = prvCpssPxPortMacConfiguration(devNum, portNum, regDataArray);
    if(rc != GT_OK)
    {
        return rc;
    }

    if(portMacType <= PRV_CPSS_PORT_GE_E)
    {/* restore port enable state */
        rc = prvCpssPxPortStateRestore(devNum,portNum, &portStateStc);
        if(rc != GT_OK)
            return rc;
    }

    return GT_OK;
}

/**
* @internal cpssPxPortSpeedAutoNegEnableSet function
* @endinternal
*
* @brief   Enable/disable an Auto-Negotiation of interface speed on specified port
*         on specified device.
*
* @note   APPLICABLE DEVICES:      pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] state                    - GT_TRUE for enable Auto-Negotiation of interface speed,
*                                      GT_FALSE otherwise
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - on not supported state
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note 1.CPU port not supports the speed auto negotiation
*
*/
GT_STATUS cpssPxPortSpeedAutoNegEnableSet
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_BOOL                 state
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortSpeedAutoNegEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, state));

    rc = internal_cpssPxPortSpeedAutoNegEnableSet(devNum, portNum, state);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, state));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortSpeedAutoNegEnableGet function
* @endinternal
*
* @brief   Get status of Auto-Negotiation enable on specified port
*
* @note   APPLICABLE DEVICES:      pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] statePtr                 - GT_TRUE for enabled Auto-Negotiation of interface speed,
*                                      GT_FALSE otherwise
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note 1. CPU port not supports the speed auto negotiation
*
*/
static GT_STATUS internal_cpssPxPortSpeedAutoNegEnableGet
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT  GT_BOOL                *statePtr
)
{
    GT_U32      portMacNum;      /* MAC number */
    GT_U32      value;               /* current speed auto-neg. status */
    GT_U32      speedAutoNegRegAddr; /* speed auto-neg. reg. address */
    GT_U32      portGroupId;         /* port group num. for multi-port group dev's */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(statePtr);
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

    if(PRV_CPSS_PX_PORT_TYPE_MAC(devNum, portMacNum) >= PRV_CPSS_PORT_XG_E)
    {
        *statePtr = GT_FALSE;
        return GT_OK;
    }

    PRV_CPSS_PX_PORT_AUTO_NEG_CTRL_REG_MAC(devNum, portMacNum, &speedAutoNegRegAddr);
    if(PRV_CPSS_SW_PTR_ENTRY_UNUSED == speedAutoNegRegAddr)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);

    portGroupId = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portMacNum);
    if (prvCpssDrvHwPpPortGroupGetRegField(CAST_SW_DEVNUM(devNum), portGroupId, speedAutoNegRegAddr,
                                            7, 1, &value) != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);
    }

    *statePtr = BIT2BOOL_MAC(value);

    return GT_OK;
}

/**
* @internal cpssPxPortSpeedAutoNegEnableGet function
* @endinternal
*
* @brief   Get status of Auto-Negotiation enable on specified port
*
* @note   APPLICABLE DEVICES:      pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] statePtr                 - GT_TRUE for enabled Auto-Negotiation of interface speed,
*                                      GT_FALSE otherwise
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note 1. CPU port not supports the speed auto negotiation
*
*/
GT_STATUS cpssPxPortSpeedAutoNegEnableGet
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT  GT_BOOL                *statePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortSpeedAutoNegEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, statePtr));

    rc = internal_cpssPxPortSpeedAutoNegEnableGet(devNum, portNum, statePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, statePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortBackPressureEnableSet function
* @endinternal
*
* @brief   Enable/disable of Back Pressure in half-duplex on specified port
*         on specified device.
*
* @note   APPLICABLE DEVICES:      pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number (or CPU port)
* @param[in] state                    - GT_TRUE for enable Back Pressure, GT_FALSE otherwise
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_SUPPORTED         - if feature not supported by port
*
* @note 1. Although for XGMII (10 Gbps) ports feature is not supported the
*       function let application to configure it, in case GE mac unit exists
*       for this port
*
*/
static GT_STATUS internal_cpssPxPortBackPressureEnableSet
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_BOOL                 state
)
{
    GT_U32      portMacNum;      /* MAC number */
    GT_U32      backPressureRegAddr; /* register address */
    PRV_CPSS_PORT_REG_CONFIG_STC   regDataArray[PRV_CPSS_PORT_NOT_APPLICABLE_E];

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum,portMacNum);

    /* set BackPressure Enable bit */
    /* Serial Parameters Register is one for 6 ports set */
    backPressureRegAddr = PRV_CPSS_PX_DEV_REGS_VER1_MAC(devNum)->GOP.perPortRegs[portNum].serialParameters;
    if(PRV_CPSS_SW_PTR_ENTRY_UNUSED == backPressureRegAddr)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);

    if(prvCpssPxPortMacConfigurationClear(regDataArray) != GT_OK)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_INIT_ERROR, LOG_ERROR_NO_MSG);

    regDataArray[PRV_CPSS_PORT_GE_E].regAddr = backPressureRegAddr;
    regDataArray[PRV_CPSS_PORT_GE_E].fieldData = BOOL2BIT_MAC(state);
    regDataArray[PRV_CPSS_PORT_GE_E].fieldLength = 1;
    regDataArray[PRV_CPSS_PORT_GE_E].fieldOffset = 4;

    return prvCpssPxPortMacConfiguration(devNum, portNum, regDataArray);
}

/**
* @internal cpssPxPortBackPressureEnableSet function
* @endinternal
*
* @brief   Enable/disable of Back Pressure in half-duplex on specified port
*         on specified device.
*
* @note   APPLICABLE DEVICES:      pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number (or CPU port)
* @param[in] state                    - GT_TRUE for enable Back Pressure, GT_FALSE otherwise
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_SUPPORTED         - if feature not supported by port
*
* @note 1. Although for XGMII (10 Gbps) ports feature is not supported the
*       function let application to configure it, in case GE mac unit exists
*       for this port
*
*/
GT_STATUS cpssPxPortBackPressureEnableSet
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_BOOL                 state
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortBackPressureEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, state));

    rc = internal_cpssPxPortBackPressureEnableSet(devNum, portNum, state);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, state));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortBackPressureEnableGet function
* @endinternal
*
* @brief   Gets the state of Back Pressure in half-duplex on specified port
*         on specified device.
*
* @note   APPLICABLE DEVICES:      pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number (or CPU port)
*
* @param[out] enablePtr                - pointer to back pressure enable/disable state:
*                                      - GT_TRUE to enable Back Pressure
*                                      - GT_FALSE to disable Back Pressure.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note 1. Although for XGMII (10 Gbps) ports feature is not supported the
*       function let application to configure it, in case GE mac unit exists
*       for this port
*
*/
static GT_STATUS internal_cpssPxPortBackPressureEnableGet
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_BOOL                 *enablePtr
)
{
    GT_U32 portMacNum;      /* MAC number */
    GT_U32 regAddr;         /* register address            */
    GT_U32 value;           /* value to write into the register */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);
    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    /* get BackPressure Enable bit */
    /* Serial Parameters Register is one for 6 ports set */
    regAddr = PRV_CPSS_PX_DEV_REGS_VER1_MAC(devNum)->GOP.perPortRegs[portMacNum].serialParameters;
    if(PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);

    if (prvCpssDrvHwPpPortGroupGetRegField(CAST_SW_DEVNUM(devNum),
        PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(CAST_SW_DEVNUM(devNum),portMacNum),
                                                          regAddr, 4, 1, &value) != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);
    }

    *enablePtr = BIT2BOOL_MAC(value);

    return GT_OK;
}

/**
* @internal cpssPxPortBackPressureEnableGet function
* @endinternal
*
* @brief   Gets the state of Back Pressure in half-duplex on specified port
*         on specified device.
*
* @note   APPLICABLE DEVICES:      pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number (or CPU port)
*
* @param[out] enablePtr                - pointer to back pressure enable/disable state:
*                                      - GT_TRUE to enable Back Pressure
*                                      - GT_FALSE to disable Back Pressure.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note 1. Although for XGMII (10 Gbps) ports feature is not supported the
*       function let application to configure it, in case GE mac unit exists
*       for this port
*
*/
GT_STATUS cpssPxPortBackPressureEnableGet
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_BOOL                 *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortBackPressureEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enablePtr));

    rc = internal_cpssPxPortBackPressureEnableGet(devNum, portNum, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortCrcCheckEnableSet function
* @endinternal
*
* @brief   Enable/Disable 32-bit the CRC checking.
*
* @note   APPLICABLE DEVICES:      pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number (or CPU port)
* @param[in] enable                   - If GT_TRUE,  CRC checking
*                                      If GT_FALSE, disable CRC checking
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPortCrcCheckEnableSet
(
    IN  GT_SW_DEV_NUM        devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  GT_BOOL              enable
)
{
    GT_U32  portMacNum;      /* MAC number */
    GT_U32 regAddr;         /* register address */
    GT_U32 value;           /* value to write into the register */
    GT_U32 offset;          /* bit number inside register       */
    PRV_CPSS_PORT_REG_CONFIG_STC   regDataArray[PRV_CPSS_PORT_NOT_APPLICABLE_E];

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum,portMacNum);

    value = (enable == GT_TRUE) ? 1 : 0;

    if(prvCpssPxPortMacConfigurationClear(regDataArray) != GT_OK)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_INIT_ERROR, LOG_ERROR_NO_MSG);

    /* convert the 'Physical port' to portGroupId,local port -- supporting multi-port-groups device */
    PRV_CPSS_PX_PORT_MAC_CTRL1_REG_MAC(devNum,portMacNum,PRV_CPSS_PORT_GE_E,&regAddr);
    if(regAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        regDataArray[PRV_CPSS_PORT_GE_E].regAddr = regAddr;
        regDataArray[PRV_CPSS_PORT_GE_E].fieldData = value;
        regDataArray[PRV_CPSS_PORT_GE_E].fieldLength = 1;
        regDataArray[PRV_CPSS_PORT_GE_E].fieldOffset = 0;
    }

    offset = 9;

    PRV_CPSS_PX_PORT_MAC_CTRL_REG_MAC(devNum,portMacNum,PRV_CPSS_PORT_XG_E,&regAddr);
    if(regAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        regDataArray[PRV_CPSS_PORT_XG_E].regAddr = regAddr;
        regDataArray[PRV_CPSS_PORT_XG_E].fieldData = value;
        regDataArray[PRV_CPSS_PORT_XG_E].fieldLength = 1;
        regDataArray[PRV_CPSS_PORT_XG_E].fieldOffset = offset;
    }

    PRV_CPSS_PX_PORT_MAC_CTRL_REG_MAC(devNum,portMacNum,PRV_CPSS_PORT_XLG_E,&regAddr);
    if(regAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        regDataArray[PRV_CPSS_PORT_XLG_E].regAddr = regAddr;
        regDataArray[PRV_CPSS_PORT_XLG_E].fieldData = value;
        regDataArray[PRV_CPSS_PORT_XLG_E].fieldLength = 1;
        regDataArray[PRV_CPSS_PORT_XLG_E].fieldOffset = offset;
    }

    PRV_CPSS_PX_REG1_CG_PORT_MAC_RX_CRC_OPT_REG_MAC(devNum,portMacNum,&regAddr);
    if(regAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        /* In CG unit the value configures the disable operation(reversed operation, hence (1 - value) ) */
        value = 1 - value;
        regDataArray[PRV_CPSS_PORT_CG_E].regAddr = regAddr;
        regDataArray[PRV_CPSS_PORT_CG_E].fieldData = value;
        regDataArray[PRV_CPSS_PORT_CG_E].fieldLength = 1;
        regDataArray[PRV_CPSS_PORT_CG_E].fieldOffset = 16;
    }

    return prvCpssPxPortMacConfiguration(devNum, portNum, regDataArray);
}

/**
* @internal cpssPxPortCrcCheckEnableSet function
* @endinternal
*
* @brief   Enable/Disable 32-bit the CRC checking.
*
* @note   APPLICABLE DEVICES:      pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number (or CPU port)
* @param[in] enable                   - If GT_TRUE,  CRC checking
*                                      If GT_FALSE, disable CRC checking
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortCrcCheckEnableSet
(
    IN  GT_SW_DEV_NUM        devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  GT_BOOL              enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortCrcCheckEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enable));

    rc = internal_cpssPxPortCrcCheckEnableSet(devNum, portNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortCrcCheckEnableGet function
* @endinternal
*
* @brief   Get CRC checking (Enable/Disable) state for received packets.
*
* @note   APPLICABLE DEVICES:      pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number (or CPU port)
*
* @param[out] enablePtr                - Pointer to the CRS checking state :
*                                      GT_TRUE  - CRC checking is enable,
*                                      GT_FALSE - CRC checking is disable.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPortCrcCheckEnableGet
(
    IN  GT_SW_DEV_NUM        devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_BOOL              *enablePtr
)
{
    GT_STATUS rc = GT_OK;
    GT_U32  portMacNum;      /* MAC number */
    GT_U32 regAddr;         /* register address */
    GT_U32 value;           /* value to write into the register */
    GT_U32 offset;          /* bit number inside register       */
    GT_U32  portGroupId;/*the port group Id - support multi-port-groups device */
    PRV_CPSS_PORT_TYPE_ENT portMacType;

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum,portMacNum);
    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    /* convert the 'Physical port' to portGroupId,local port -- supporting multi-port-groups device */
    portGroupId = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portMacNum);
    portMacType = PRV_CPSS_PX_PORT_TYPE_MAC(devNum,portMacNum);

    if(portMacType >= PRV_CPSS_PORT_XG_E)
    {
        if (portMacType == PRV_CPSS_PORT_CG_E)
        {
            PRV_CPSS_PX_REG1_CG_PORT_MAC_RX_CRC_OPT_REG_MAC(devNum, portMacNum, &regAddr);
            offset = 16;
        }
        else
        {
            PRV_CPSS_PX_PORT_MAC_CTRL_REG_MAC(devNum, portMacNum, portMacType, &regAddr);
            offset = 9;
        }
    }
    else
    {
        PRV_CPSS_PX_PORT_MAC_CTRL1_REG_MAC(devNum, portMacNum, portMacType, &regAddr);
        offset = 0;
    }

    if(PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);

    /* get state */
    rc = prvCpssDrvHwPpPortGroupGetRegField(CAST_SW_DEVNUM(devNum), portGroupId, regAddr, offset, 1, &value);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);
    }

    if (portMacType == PRV_CPSS_PORT_CG_E)
    {
        *enablePtr = (value == 1) ? GT_FALSE : GT_TRUE;
    }
    else
    {
        *enablePtr = (value == 1) ? GT_TRUE : GT_FALSE;
    }

    return GT_OK;
}

/**
* @internal cpssPxPortCrcCheckEnableGet function
* @endinternal
*
* @brief   Get CRC checking (Enable/Disable) state for received packets.
*
* @note   APPLICABLE DEVICES:      pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number (or CPU port)
*
* @param[out] enablePtr                - Pointer to the CRS checking state :
*                                      GT_TRUE  - CRC checking is enable,
*                                      GT_FALSE - CRC checking is disable.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortCrcCheckEnableGet
(
    IN  GT_SW_DEV_NUM        devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_BOOL              *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortCrcCheckEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enablePtr));

    rc = internal_cpssPxPortCrcCheckEnableGet(devNum, portNum, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortExcessiveCollisionDropEnableSet function
* @endinternal
*
* @brief   Enable/Disable excessive collision packets drop.
*         In half duplex mode if a collision occurs the device tries to transmit
*         the packet again. If the number of collisions on the same packet is 16
*         and excessive collision packets drop is enabled,
*         the packet is dropped.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number (or CPU port).
* @param[in] enable                   - GT_TRUE  - if the number of collisions on the same packet
*                                      is 16 the packet is dropped.
*                                      - GT_FALSE - A collided packet will be retransmitted by device
*                                      until it is transmitted without collisions,
*                                      regardless of the number of collisions on the packet.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_FAIL                  - on error
*
* @note The setting is not relevant in full duplex mode
*
*/
static GT_STATUS internal_cpssPxPortExcessiveCollisionDropEnableSet
(
    IN  GT_SW_DEV_NUM        devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  GT_BOOL              enable
)
{
    GT_U32  portMacNum;  /* MAC number */
    GT_U32  regAddr; /* register address */
    PRV_CPSS_PORT_REG_CONFIG_STC   regDataArray[PRV_CPSS_PORT_NOT_APPLICABLE_E];

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum,portMacNum);

    PRV_CPSS_PX_PORT_MAC_CTRL1_REG_MAC(devNum,portMacNum,PRV_CPSS_PORT_GE_E,&regAddr);
    if(PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);

    if(prvCpssPxPortMacConfigurationClear(regDataArray) != GT_OK)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_INIT_ERROR, LOG_ERROR_NO_MSG);

    if(regAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        regDataArray[PRV_CPSS_PORT_GE_E].regAddr = regAddr;
        regDataArray[PRV_CPSS_PORT_GE_E].fieldData = (enable == GT_TRUE) ? 0 : 1;
        regDataArray[PRV_CPSS_PORT_GE_E].fieldLength = 1;
        regDataArray[PRV_CPSS_PORT_GE_E].fieldOffset = 4;
    }

    return prvCpssPxPortMacConfiguration(devNum, portNum, regDataArray);
}

/**
* @internal cpssPxPortExcessiveCollisionDropEnableSet function
* @endinternal
*
* @brief   Enable/Disable excessive collision packets drop.
*         In half duplex mode if a collision occurs the device tries to transmit
*         the packet again. If the number of collisions on the same packet is 16
*         and excessive collision packets drop is enabled,
*         the packet is dropped.
*
* @note   APPLICABLE DEVICES:      pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number (or CPU port).
* @param[in] enable                   - GT_TRUE  - if the number of collisions on the same packet
*                                      is 16 the packet is dropped.
*                                      - GT_FALSE - A collided packet will be retransmitted by device
*                                      until it is transmitted without collisions,
*                                      regardless of the number of collisions on the packet.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_FAIL                  - on error
*
* @note The setting is not relevant in full duplex mode
*
*/
GT_STATUS cpssPxPortExcessiveCollisionDropEnableSet
(
    IN  GT_SW_DEV_NUM        devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  GT_BOOL              enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortExcessiveCollisionDropEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enable));

    rc = internal_cpssPxPortExcessiveCollisionDropEnableSet(devNum, portNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortExcessiveCollisionDropEnableGet function
* @endinternal
*
* @brief   Gets status of excessive collision packets drop.
*         In half duplex mode if a collision occurs the device tries to transmit
*         the packet again. If the number of collisions on the same packet is 16
*         and excessive collision packets drop is enabled,
*         the packet is dropped.
*
* @note   APPLICABLE DEVICES:      pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number (or CPU port).
*
* @param[out] enablePtr                - pointer to status of excessive collision packets drop.
*                                      - GT_TRUE  - if the number of collisions on the same packet
*                                      is 16 the packet is dropped.
*                                      - GT_FALSE - A collided packet will be retransmitted by
*                                      device until it is transmitted
*                                      without collisions, regardless of the number
*                                      of collisions on the packet.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*
* @note 1. Not relevant in full duplex mode
*
*/
static GT_STATUS internal_cpssPxPortExcessiveCollisionDropEnableGet
(
    IN  GT_SW_DEV_NUM        devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_BOOL              *enablePtr
)
{
    GT_STATUS   rc = GT_OK;
    GT_U32      portMacNum;      /* MAC number */
    GT_U32      regAddr; /* register address */
    GT_U32      value;   /* register field value */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum,portMacNum);
    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    PRV_CPSS_PX_PORT_MAC_CTRL1_REG_MAC(devNum, portMacNum, PRV_CPSS_PORT_GE_E, &regAddr);
    if(PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);

    rc = prvCpssDrvHwPpPortGroupGetRegField(CAST_SW_DEVNUM(devNum),
            PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portMacNum),
            regAddr, 4, 1, &value);
    if(rc != GT_OK)
    {
        return rc;
    }

    *enablePtr = (value == 0) ? GT_TRUE : GT_FALSE;
    return GT_OK;
}

/**
* @internal cpssPxPortExcessiveCollisionDropEnableGet function
* @endinternal
*
* @brief   Gets status of excessive collision packets drop.
*         In half duplex mode if a collision occurs the device tries to transmit
*         the packet again. If the number of collisions on the same packet is 16
*         and excessive collision packets drop is enabled,
*         the packet is dropped.
*
* @note   APPLICABLE DEVICES:      pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number (or CPU port).
*
* @param[out] enablePtr                - pointer to status of excessive collision packets drop.
*                                      - GT_TRUE  - if the number of collisions on the same packet
*                                      is 16 the packet is dropped.
*                                      - GT_FALSE - A collided packet will be retransmitted by
*                                      device until it is transmitted
*                                      without collisions, regardless of the number
*                                      of collisions on the packet.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*
* @note 1. Not relevant in full duplex mode
*
*/
GT_STATUS cpssPxPortExcessiveCollisionDropEnableGet
(
    IN  GT_SW_DEV_NUM        devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_BOOL              *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortExcessiveCollisionDropEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enablePtr));

    rc = internal_cpssPxPortExcessiveCollisionDropEnableGet(devNum, portNum, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssPxPortInBandAutoNegBypassEnableSet function
* @endinternal
*
* @brief   Enable/Disable Auto-Negotiation by pass.
*         If the link partner doesn't respond to Auto-Negotiation process,
*         the link is established by bypassing the Auto-Negotiation procedure.
*
* @note   APPLICABLE DEVICES:      pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number.
* @param[in] enable                   - GT_TRUE  - Auto-Negotiation can't be bypassed.
*                                      - GT_FALSE - Auto-Negotiation is bypassed.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_NOT_SUPPORTED         - The feature is not supported the port/device
* @retval GT_FAIL                  - on error
*
* @note Relevant when Inband Auto-Negotiation is enabled.
*       (See cpssPxPortInbandAutoNegEnableSet.)
*       Although for XGMII (10 Gbps) ports feature is not supported the function
*       lets application to configure it
*
*/
static GT_STATUS internal_cpssPxPortInBandAutoNegBypassEnableSet
(
    IN  GT_SW_DEV_NUM        devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  GT_BOOL              enable
)
{
    GT_U32      regAddr; /* register address */
    GT_U32      portMacNum; /* MAC number */
    PRV_CPSS_PORT_REG_CONFIG_STC   regDataArray[PRV_CPSS_PORT_NOT_APPLICABLE_E];

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

    PRV_CPSS_PX_PORT_AUTO_NEG_CTRL_REG_MAC(devNum,portMacNum,&regAddr);
    if(PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);

    if(prvCpssPxPortMacConfigurationClear(regDataArray) != GT_OK)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_INIT_ERROR, LOG_ERROR_NO_MSG);

    regDataArray[PRV_CPSS_PORT_GE_E].regAddr = regAddr;
    regDataArray[PRV_CPSS_PORT_GE_E].fieldData = BOOL2BIT_MAC(enable);
    regDataArray[PRV_CPSS_PORT_GE_E].fieldLength = 1;
    regDataArray[PRV_CPSS_PORT_GE_E].fieldOffset = 3;

    return prvCpssPxPortMacConfiguration(devNum, portNum, regDataArray);
}

/**
* @internal cpssPxPortInBandAutoNegBypassEnableSet function
* @endinternal
*
* @brief   Enable/Disable Auto-Negotiation by pass.
*         If the link partner doesn't respond to Auto-Negotiation process,
*         the link is established by bypassing the Auto-Negotiation procedure.
*
* @note   APPLICABLE DEVICES:      pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number.
* @param[in] enable                   - GT_TRUE  - Auto-Negotiation can't be bypassed.
*                                      - GT_FALSE - Auto-Negotiation is bypassed.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_NOT_SUPPORTED         - The feature is not supported the port/device
* @retval GT_FAIL                  - on error
*
* @note Relevant when Inband Auto-Negotiation is enabled.
*       (See cpssPxPortInbandAutoNegEnableSet.)
*       Although for XGMII (10 Gbps) ports feature is not supported the function
*       lets application to configure it
*
*/
GT_STATUS cpssPxPortInBandAutoNegBypassEnableSet
(
    IN  GT_SW_DEV_NUM        devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  GT_BOOL              enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortInBandAutoNegBypassEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enable));

    rc = internal_cpssPxPortInBandAutoNegBypassEnableSet(devNum, portNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortInBandAutoNegBypassEnableGet function
* @endinternal
*
* @brief   Gets Auto-Negotiation by pass status.
*         If the link partner doesn't respond to Auto-Negotiation process,
*         the link is established by bypassing the Auto-Negotiation procedure.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number.
*
* @param[out] enablePtr                - pointer to Auto-Negotiation by pass status.
*                                      - GT_TRUE  - Auto-Negotiation can't be bypassed.
*                                      - GT_FALSE - Auto-Negotiation is bypassed.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_NOT_SUPPORTED         - The feature is not supported the port/device
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*
* @note Relevant when Inband Auto-Negotiation is enabled.
*       (See cpssPxPortInbandAutoNegEnableSet.)
*       Although for XGMII (10 Gbps) ports feature is not supported the function
*       lets application to configure it
*
*/
static GT_STATUS internal_cpssPxPortInBandAutoNegBypassEnableGet
(
    IN  GT_SW_DEV_NUM        devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_BOOL              *enablePtr
)
{
    GT_U32      regAddr; /* register address */
    GT_U32      value;   /* register field value */
    GT_STATUS   rc;      /* return status */
    GT_U32      portMacNum; /* MAC number */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);
    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    PRV_CPSS_PX_PORT_AUTO_NEG_CTRL_REG_MAC(devNum, portMacNum, &regAddr);
    if(PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);

    rc = prvCpssDrvHwPpPortGroupGetRegField(CAST_SW_DEVNUM(devNum),
            PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portMacNum),
            regAddr, 3, 1, &value);

    if(rc != GT_OK)
    {
        return rc;
    }
    *enablePtr = BIT2BOOL_MAC(value);
    return GT_OK;
}

/**
* @internal cpssPxPortInBandAutoNegBypassEnableGet function
* @endinternal
*
* @brief   Gets Auto-Negotiation by pass status.
*         If the link partner doesn't respond to Auto-Negotiation process,
*         the link is established by bypassing the Auto-Negotiation procedure.
*
* @note   APPLICABLE DEVICES:      pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number.
*
* @param[out] enablePtr                - pointer to Auto-Negotiation by pass status.
*                                      - GT_TRUE  - Auto-Negotiation can't be bypassed.
*                                      - GT_FALSE - Auto-Negotiation is bypassed.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_NOT_SUPPORTED         - The feature is not supported the port/device
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*
* @note Relevant when Inband Auto-Negotiation is enabled.
*       (See cpssPxPortInbandAutoNegEnableSet.)
*       Although for XGMII (10 Gbps) ports feature is not supported the function
*       lets application to configure it
*
*/
GT_STATUS cpssPxPortInBandAutoNegBypassEnableGet
(
    IN  GT_SW_DEV_NUM        devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_BOOL              *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortInBandAutoNegBypassEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enablePtr));

    rc = internal_cpssPxPortInBandAutoNegBypassEnableGet(devNum, portNum, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal prvCpssPxPortMacResetStateGet function
* @endinternal
*
* @brief   Get MAC Reset state on specified port on specified device.
*
* @note   APPLICABLE DEVICES:      pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number (including CPU)
*
* @param[out] statePtr                 - Reset state
*                                      GT_TRUE   - Port MAC is under Reset
*                                      GT_FALSE - Port MAC is Not under Reset, normal operation
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - statePtr == NULL
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssPxPortMacResetStateGet
(
    IN  GT_SW_DEV_NUM        devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_BOOL              *statePtr
)
{
    GT_STATUS rc;
    GT_U32 regAddr;         /* register address */
    GT_U32 value;           /* value to write into the register */
    GT_U32 offset;          /* bit number inside register       */
    GT_U32  portGroupId;/*the port group Id - support multi-port-groups device */
    PRV_CPSS_PORT_TYPE_ENT portMacType; /* MAC unit used by port */
    GT_U32  portMacNum;      /* MAC number */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum, portMacNum);

    portMacType = PRV_CPSS_PX_PORT_TYPE_MAC(devNum,portMacNum);
    if(PRV_CPSS_PORT_NOT_EXISTS_E == portMacType)
    {
        *statePtr = GT_TRUE;
        return GT_OK;
    }

    if(portMacType >= PRV_CPSS_PORT_XG_E)
    {
        PRV_CPSS_PX_PORT_MAC_CTRL_REG_MAC(devNum, portMacNum, portMacType, &regAddr);
        offset = 1;
    }
    else
    {
        PRV_CPSS_PX_PORT_MAC_CTRL2_REG_MAC(devNum, portMacNum, portMacType, &regAddr);
        offset = 6;
    }
    if(PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);

    /* get MACResetn bit */
    portGroupId = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portMacNum);
    if((rc = prvCpssDrvHwPpPortGroupGetRegField(CAST_SW_DEVNUM(devNum), portGroupId, regAddr,
                                                offset, 1, &value)) != GT_OK)
    {
        return rc;
    }

    if(portMacType >= PRV_CPSS_PORT_XG_E)
    {
        *statePtr = (1 == value) ? GT_FALSE : GT_TRUE;
    }
    else
    {
        *statePtr = BIT2BOOL_MAC(value);
    }
    return GT_OK;
}

/**
* @internal prvCpssPxPortMacResetStateSet function
* @endinternal
*
* @brief   Set MAC and XPCS Reset state on specified port on specified device.
*
* @note   APPLICABLE DEVICES:      pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number (or CPU port)
* @param[in] state                    - Reset state
*                                      GT_TRUE   - Port MAC is under Reset
*                                      GT_FALSE - Port MAC is Not under Reset, normal operation
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssPxPortMacResetStateSet
(
    IN  GT_SW_DEV_NUM        devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  GT_BOOL              state
)
{
    GT_STATUS                           rc;
    PRV_CPSS_PORT_REG_CONFIG_STC     regDataArray[PRV_CPSS_PORT_NOT_APPLICABLE_E];
    MV_HWS_PORT_INIT_PARAMS             curPortParams;
    GT_U32                              portGroup;      /* port group number for multi-port-group devs */
    MV_HWS_PORT_STANDARD                portMode;       /* port interface in HWS format */
    GT_U32                              portMacNum;     /* MAC number */
    CPSS_PORT_INTERFACE_MODE_ENT        ifMode;         /* interface mode to configure */
    CPSS_PORT_SPEED_ENT                 speed;          /* current speed of port */
    GT_U32                              localPort=0;    /* port number in local core */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum,portMacNum);

    if (prvCpssPxPortMacConfigurationClear(regDataArray) != GT_OK)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_INIT_ERROR, LOG_ERROR_NO_MSG);

    ifMode      = PRV_CPSS_PX_PORT_IFMODE_MAC(devNum, portMacNum);
    speed       = PRV_CPSS_PX_PORT_SPEED_MAC(devNum, portMacNum);

    if ((CPSS_PORT_INTERFACE_MODE_NA_E != ifMode) && (CPSS_PORT_SPEED_NA_E != speed) && (portNum != CPSS_CPU_PORT_NUM_CNS))
    {
        portGroup = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portMacNum);
        localPort = portMacNum;

        prvCpssCommonPortIfModeToHwsTranslate(devNum,ifMode, speed, &portMode);


        if (GT_OK != hwsPortModeParamsGetToBuffer(CAST_SW_DEVNUM(devNum), portGroup, localPort, portMode, &curPortParams))
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
        }

        if ((curPortParams.portMacType != LAST_MAC) && (curPortParams.portMacType != MAC_NA))
        {
            if (ifMode == CPSS_PORT_INTERFACE_MODE_QSGMII_E)
            {
                /* for QSGMII mode mvHwsMacReset shoud get the first portNum in the GOP */
                localPort = localPort - localPort % 4;
            }

            rc = mvHwsMacReset(CAST_SW_DEVNUM(devNum), portGroup, localPort, portMode, curPortParams.portMacType, ((state == GT_TRUE) ? RESET : UNRESET));
            if (rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "mvHwsMacReset: error for portNum=%d\n", localPort);
            }
        }
    }
    return GT_OK;
}

/**
* @internal internal_cpssPxPortMacResetStateSet function
* @endinternal
*
* @brief   Set MAC Reset state on specified port on specified device.
*
* @note   APPLICABLE DEVICES:      pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number (or CPU port)
* @param[in] state                    - Reset state
*                                      GT_TRUE   - Port MAC is under Reset
*                                      GT_FALSE - Port MAC is Not under Reset, normal operation
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPortMacResetStateSet
(
    IN  GT_SW_DEV_NUM        devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  GT_BOOL   state
)
{
    return prvCpssPxPortMacResetStateSet(devNum, portNum, state);
}

/**
* @internal cpssPxPortMacResetStateSet function
* @endinternal
*
* @brief   Set MAC Reset state on specified port on specified device.
*
* @note   APPLICABLE DEVICES:      pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number (or CPU port)
* @param[in] state                    - Reset state
*                                      GT_TRUE   - Port MAC is under Reset
*                                      GT_FALSE - Port MAC is Not under Reset, normal operation
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortMacResetStateSet
(
    IN  GT_SW_DEV_NUM        devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  GT_BOOL              state
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortMacResetStateSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, state));

    rc = internal_cpssPxPortMacResetStateSet(devNum, portNum, state);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, state));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssPxPortForward802_3xEnableSet function
* @endinternal
*
* @brief   Enable/disable forwarding of 802.3x Flow Control frames to the ingress
*         pipeline of a specified port. Processing of 802.3x Flow Control frames
*         is done like regular data frames if forwarding enabled.
*
* @note   APPLICABLE DEVICES:      pipe.
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number (CPU port not supported)
* @param[in] enable                   - GT_TRUE:  forward 802.3x frames to the ingress pipe,
*                                      GT_FALSE: do not forward 802.3x frames to the ingress pipe.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note A packet is considered a valid Flow Control packet (i.e., it may be used
*       to halt the port's packet transmission if it is an XOFF packet, or to
*       resume the port's packets transmission, if it is an XON packet) if all of
*       the following are true:
*       - Packet's Length/EtherType field is 88-08
*       - Packet's OpCode field is 00-01
*       - Packet's MAC DA is 01-80-C2-00-00-01 or the port's configured MAC Address
*
*/
static GT_STATUS internal_cpssPxPortForward802_3xEnableSet
(
    IN  GT_SW_DEV_NUM        devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  GT_BOOL              enable
)
{
    GT_U32  regAddr;    /* register address */
    GT_U32  value;      /* register value */
    GT_U32  offset;     /* bit number inside register       */
    GT_U32  portMacNum; /* MAC number */
    PRV_CPSS_PORT_REG_CONFIG_STC   regDataArray[PRV_CPSS_PORT_NOT_APPLICABLE_E];

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

    value = BOOL2BIT_MAC(enable);
    regAddr = PRV_CPSS_PX_DEV_REGS_VER1_MAC(devNum)->GOP.perPortRegs[portMacNum].serialParameters;
    offset = 3;

    if(prvCpssPxPortMacConfigurationClear(regDataArray) != GT_OK)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_INIT_ERROR, LOG_ERROR_NO_MSG);

    if(regAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        regDataArray[PRV_CPSS_PORT_GE_E].regAddr = regAddr;
        regDataArray[PRV_CPSS_PORT_GE_E].fieldData = value;
        regDataArray[PRV_CPSS_PORT_GE_E].fieldLength = 1;
        regDataArray[PRV_CPSS_PORT_GE_E].fieldOffset = offset;
    }

    PRV_CPSS_PX_PORT_MAC_CTRL4_REG_MAC(devNum, portMacNum, PRV_CPSS_PORT_XG_E, &regAddr);
    offset = 5;

    if(regAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        regDataArray[PRV_CPSS_PORT_XG_E].regAddr = regAddr;
        regDataArray[PRV_CPSS_PORT_XG_E].fieldData = value;
        regDataArray[PRV_CPSS_PORT_XG_E].fieldLength = 1;
        regDataArray[PRV_CPSS_PORT_XG_E].fieldOffset = offset;
    }
    PRV_CPSS_PX_PORT_MAC_CTRL4_REG_MAC(devNum, portMacNum, PRV_CPSS_PORT_XLG_E, &regAddr);
    offset = 5;

    if(regAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        regDataArray[PRV_CPSS_PORT_XLG_E].regAddr = regAddr;
        regDataArray[PRV_CPSS_PORT_XLG_E].fieldData = value;
        regDataArray[PRV_CPSS_PORT_XLG_E].fieldLength = 1;
        regDataArray[PRV_CPSS_PORT_XLG_E].fieldOffset = offset;
    }

    regAddr = PRV_PX_REG1_UNIT_CG_CONVERTERS_MAC(devNum,portMacNum).CGMAConvertersFcControl0;
    if(regAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        regDataArray[PRV_CPSS_PORT_CG_E].regAddr = regAddr;
        regDataArray[PRV_CPSS_PORT_CG_E].fieldData = value;
        regDataArray[PRV_CPSS_PORT_CG_E].fieldLength = 1;
        regDataArray[PRV_CPSS_PORT_CG_E].fieldOffset = 16;/*<Forward 802.3x FC Enable>*/
    }

    return prvCpssPxPortMacConfiguration(devNum, portNum, regDataArray);
}

/**
* @internal cpssPxPortForward802_3xEnableSet function
* @endinternal
*
* @brief   Enable/disable forwarding of 802.3x Flow Control frames to the ingress
*         pipeline of a specified port. Processing of 802.3x Flow Control frames
*         is done like regular data frames if forwarding enabled.
*
* @note   APPLICABLE DEVICES:      pipe.
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number (CPU port not supported)
* @param[in] enable                   - GT_TRUE:  forward 802.3x frames to the ingress pipe,
*                                      GT_FALSE: do not forward 802.3x frames to the ingress pipe.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note A packet is considered a valid Flow Control packet (i.e., it may be used
*       to halt the port?s packet transmission if it is an XOFF packet, or to
*       resume the port?s packets transmission, if it is an XON packet) if all of
*       the following are true:
*       - Packet?s Length/EtherType field is 88-08
*       - Packet?s OpCode field is 00-01
*       - Packet?s MAC DA is 01-80-C2-00-00-01 or the port?s configured MAC Address
*
*/
GT_STATUS cpssPxPortForward802_3xEnableSet
(
    IN  GT_SW_DEV_NUM        devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  GT_BOOL              enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortForward802_3xEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enable));

    rc = internal_cpssPxPortForward802_3xEnableSet(devNum, portNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssPxPortForward802_3xEnableGet function
* @endinternal
*
* @brief   Get status of 802.3x frames forwarding on a specified port
*
* @note   APPLICABLE DEVICES:      pipe.
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number (CPU port not supported)
*
* @param[out] enablePtr                - status of 802.3x frames forwarding
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL pointer
*
* @note A packet is considered a valid Flow Control packet (i.e., it may be used
*       to halt the port?s packet transmission if it is an XOFF packet, or to
*       resume the port?s packets transmission, if it is an XON packet) if all of
*       the following are true:
*       - Packet?s Length/EtherType field is 88-08
*       - Packet?s OpCode field is 00-01
*       - Packet?s MAC DA is 01-80-C2-00-00-01 or the port?s configured MAC Address
*
*/
static GT_STATUS internal_cpssPxPortForward802_3xEnableGet
(
    IN  GT_SW_DEV_NUM        devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_BOOL              *enablePtr
)
{
    GT_U32      regAddr;    /* register address */
    GT_U32      portGroupId;/*the port group Id - support multi-port-groups device */
    GT_U32      value;      /* register value */
    GT_STATUS   rc;      /* return code */
    GT_U32      offset;     /* bit number inside register       */
    PRV_CPSS_PORT_TYPE_ENT portMacType; /* type of mac unit of port */
    GT_U32      portMacNum; /* MAC number */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);
    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    portMacType = PRV_CPSS_PX_PORT_TYPE_MAC(devNum, portMacNum);
    if(portMacType >= PRV_CPSS_PORT_XG_E)
    {
        if(portMacType == PRV_CPSS_PORT_CG_E)
        {
            regAddr = PRV_PX_REG1_UNIT_CG_CONVERTERS_MAC(devNum,portMacNum).CGMAConvertersFcControl0;
            offset  = 16;/*<Forward 802.3x FC Enable>*/
        }
        else
        {
            PRV_CPSS_PX_PORT_MAC_CTRL4_REG_MAC(devNum,portMacNum,portMacType,&regAddr);
            offset = 5;
        }
    }
    else
    {
        regAddr = PRV_CPSS_PX_DEV_REGS_VER1_MAC(devNum)->GOP.perPortRegs[portMacNum].serialParameters;
        offset = 3;
    }
    if(PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);

    /* convert the 'Physical port' to portGroupId,local port -- supporting multi-port-groups device */
    portGroupId = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portMacNum);
    if((rc = prvCpssDrvHwPpPortGroupGetRegField(CAST_SW_DEVNUM(devNum), portGroupId, regAddr, offset, 1, &value)) != GT_OK)
        return rc;

    *enablePtr = BIT2BOOL_MAC(value);
    return GT_OK;
}

/**
* @internal cpssPxPortForward802_3xEnableGet function
* @endinternal
*
* @brief   Get status of 802.3x frames forwarding on a specified port
*
* @note   APPLICABLE DEVICES:      pipe.
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number (CPU port not supported)
*
* @param[out] enablePtr                - status of 802.3x frames forwarding
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL pointer
*
* @note A packet is considered a valid Flow Control packet (i.e., it may be used
*       to halt the port?s packet transmission if it is an XOFF packet, or to
*       resume the port?s packets transmission, if it is an XON packet) if all of
*       the following are true:
*       - Packet?s Length/EtherType field is 88-08
*       - Packet?s OpCode field is 00-01
*       - Packet?s MAC DA is 01-80-C2-00-00-01 or the port?s configured MAC Address
*
*/
GT_STATUS cpssPxPortForward802_3xEnableGet
(
    IN  GT_SW_DEV_NUM        devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_BOOL              *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortForward802_3xEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enablePtr));

    rc = internal_cpssPxPortForward802_3xEnableGet(devNum, portNum, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortForwardUnknownMacControlFramesEnableSet function
* @endinternal
*
* @brief   Enable/disable forwarding of unknown MAC control frames to the ingress
*         pipeline of a specified port. Processing of unknown MAC control frames
*         is done like regular data frames if forwarding enabled.
*
* @note   APPLICABLE DEVICES:      pipe.
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number (CPU port not supported)
* @param[in] enable                   - GT_TRUE:  forward unknown MAC control frames to the ingress pipe,
*                                      GT_FALSE: do not forward unknown MAC control frames to
*                                      the ingress pipe.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note A packet is considered as an unknown MAC control frame if all of
*       the following are true:
*       - Packet?s Length/EtherType field is 88-08
*       - Packet?s OpCode field is not 00-01 and not 01-01
*       OR
*       Packet?s MAC DA is not 01-80-C2-00-00-01 and not the port?s configured
*       MAC Address
*
*/
static GT_STATUS internal_cpssPxPortForwardUnknownMacControlFramesEnableSet
(
    IN  GT_SW_DEV_NUM        devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  GT_BOOL              enable
)
{
    GT_U32 regAddr;    /* register address */
    GT_U32 value;      /* register value */
    GT_U32 portMacNum; /* MAC number */
    PRV_CPSS_PORT_REG_CONFIG_STC   regDataArray[PRV_CPSS_PORT_NOT_APPLICABLE_E];

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

    value = BOOL2BIT_MAC(enable);
    regAddr = PRV_CPSS_PX_DEV_REGS_VER1_MAC(devNum)->GOP.perPortRegs[portMacNum].serialParameters1;
    if(prvCpssPxPortMacConfigurationClear(regDataArray) != GT_OK)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_INIT_ERROR, LOG_ERROR_NO_MSG);

    if(regAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        regDataArray[PRV_CPSS_PORT_GE_E].regAddr = regAddr;
        regDataArray[PRV_CPSS_PORT_GE_E].fieldData = value;
        regDataArray[PRV_CPSS_PORT_GE_E].fieldLength = 1;
        regDataArray[PRV_CPSS_PORT_GE_E].fieldOffset = 2;
    }

    PRV_CPSS_PX_PORT_MAC_CTRL4_REG_MAC(devNum,portMacNum,PRV_CPSS_PORT_XG_E,&regAddr);
    if(regAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        regDataArray[PRV_CPSS_PORT_XG_E].regAddr = regAddr;
        regDataArray[PRV_CPSS_PORT_XG_E].fieldData = value;
        regDataArray[PRV_CPSS_PORT_XG_E].fieldLength = 1;
        regDataArray[PRV_CPSS_PORT_XG_E].fieldOffset = 7;
    }

    PRV_CPSS_PX_PORT_MAC_CTRL4_REG_MAC(devNum,portMacNum,PRV_CPSS_PORT_XLG_E,&regAddr);
    if(regAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        regDataArray[PRV_CPSS_PORT_XLG_E].regAddr = regAddr;
        regDataArray[PRV_CPSS_PORT_XLG_E].fieldData = value;
        regDataArray[PRV_CPSS_PORT_XLG_E].fieldLength = 1;
        regDataArray[PRV_CPSS_PORT_XLG_E].fieldOffset = 7;
    }

    regAddr = PRV_PX_REG1_UNIT_CG_CONVERTERS_MAC(devNum,portMacNum).CGMAConvertersFcControl0;
    if(regAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        regDataArray[PRV_CPSS_PORT_CG_E].regAddr = regAddr;
        regDataArray[PRV_CPSS_PORT_CG_E].fieldData = value;
        regDataArray[PRV_CPSS_PORT_CG_E].fieldLength = 1;
        regDataArray[PRV_CPSS_PORT_CG_E].fieldOffset = 18;/*<Forward UnKnown FC Enable>*/
    }

    return prvCpssPxPortMacConfiguration(devNum, portNum, regDataArray);
}

/**
* @internal cpssPxPortForwardUnknownMacControlFramesEnableSet function
* @endinternal
*
* @brief   Enable/disable forwarding of unknown MAC control frames to the ingress
*         pipeline of a specified port. Processing of unknown MAC control frames
*         is done like regular data frames if forwarding enabled.
*
* @note   APPLICABLE DEVICES:      pipe.
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number (CPU port not supported)
* @param[in] enable                   - GT_TRUE:  forward unknown MAC control frames to the ingress pipe,
*                                      GT_FALSE: do not forward unknown MAC control frames to
*                                      the ingress pipe.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note A packet is considered as an unknown MAC control frame if all of
*       the following are true:
*       - Packet?s Length/EtherType field is 88-08
*       - Packet?s OpCode field is not 00-01 and not 01-01
*       OR
*       Packet?s MAC DA is not 01-80-C2-00-00-01 and not the port?s configured
*       MAC Address
*
*/
GT_STATUS cpssPxPortForwardUnknownMacControlFramesEnableSet
(
    IN  GT_SW_DEV_NUM        devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  GT_BOOL              enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortForwardUnknownMacControlFramesEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enable));

    rc = internal_cpssPxPortForwardUnknownMacControlFramesEnableSet(devNum, portNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssPxPortForwardUnknownMacControlFramesEnableGet function
* @endinternal
*
* @brief   Get current status of unknown MAC control frames
*         forwarding on a specified port
*
* @note   APPLICABLE DEVICES:      pipe.
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number (CPU port not supported)
*
* @param[out] enablePtr                - status of unknown MAC control frames forwarding
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL pointer
*
* @note A packet is considered as an unknown MAC control frame if all of
*       the following are true:
*       - Packet?s Length/EtherType field is 88-08
*       - Packet?s OpCode field is not 00-01 and not 01-01
*       OR
*       Packet?s MAC DA is not 01-80-C2-00-00-01 and not the port?s configured
*       MAC Address
*
*/
static GT_STATUS internal_cpssPxPortForwardUnknownMacControlFramesEnableGet
(
    IN  GT_SW_DEV_NUM        devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_BOOL              *enablePtr
)
{
    GT_U32      regAddr;    /* register address */
    GT_U32      portGroupId;/*the port group Id - support multi-port-groups device */
    GT_U32      value;      /* register value */
    GT_STATUS   rc;      /* return code */
    GT_U32      offset;     /* bit number inside register       */
    PRV_CPSS_PORT_TYPE_ENT portMacType; /* type of mac unit of port */
    GT_U32      portMacNum; /* MAC number */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);
    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    /* convert the 'Physical port' to portGroupId,local port -- supporting multi-port-groups device */
    portGroupId = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portMacNum);
    portMacType = PRV_CPSS_PX_PORT_TYPE_MAC(devNum,portMacNum);
    if(portMacType < PRV_CPSS_PORT_XG_E)
    {
        offset = 2;
        regAddr = PRV_CPSS_PX_DEV_REGS_VER1_MAC(devNum)->GOP.perPortRegs[portMacNum].serialParameters1;
    }
    else
    {
        if(portMacType == PRV_CPSS_PORT_CG_E)
        {
            regAddr = PRV_PX_REG1_UNIT_CG_CONVERTERS_MAC(devNum,portMacNum).CGMAConvertersFcControl0;
            offset  = 18;/*<Forward UnKnown FC Enable>*/
        }
        else
        {
            PRV_CPSS_PX_PORT_MAC_CTRL4_REG_MAC(devNum, portMacNum, portMacType, &regAddr);
            if(portMacType >= PRV_CPSS_PORT_XG_E)
            {
                offset = 7;
            }
            else
            {
                offset = 2;
            }
        }
    }

    if(PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);

    if((rc = prvCpssDrvHwPpPortGroupGetRegField(CAST_SW_DEVNUM(devNum), portGroupId, regAddr, offset, 1, &value)) != GT_OK)
        return rc;

    *enablePtr = BIT2BOOL_MAC(value);
    return GT_OK;
}

/**
* @internal cpssPxPortForwardUnknownMacControlFramesEnableGet function
* @endinternal
*
* @brief   Get current status of unknown MAC control frames
*         forwarding on a specified port
*
* @note   APPLICABLE DEVICES:      pipe.
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number (CPU port not supported)
*
* @param[out] enablePtr                - status of unknown MAC control frames forwarding
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL pointer
*
* @note A packet is considered as an unknown MAC control frame if all of
*       the following are true:
*       - Packet?s Length/EtherType field is 88-08
*       - Packet?s OpCode field is not 00-01 and not 01-01
*       OR
*       Packet?s MAC DA is not 01-80-C2-00-00-01 and not the port?s configured
*       MAC Address
*
*/
GT_STATUS cpssPxPortForwardUnknownMacControlFramesEnableGet
(
    IN  GT_SW_DEV_NUM        devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_BOOL              *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortForwardUnknownMacControlFramesEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enablePtr));

    rc = internal_cpssPxPortForwardUnknownMacControlFramesEnableGet(devNum, portNum, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal prvCpssPxPortInbandAutonegMode function
* @endinternal
*
* @brief   Set inband autoneg mode accordingly to required ifMode
*
* @note   APPLICABLE DEVICES:      pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number (CPU port not supported)
* @param[in] ifMode                   - port interface mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Relevant when <InBandAnEn> is set to 1.
*       Not relevant for the CPU port.
*       This field may only be changed when the port link is down.
*       In existing devices inband auto-neg. disabled by default.
*
*/
GT_STATUS prvCpssPxPortInbandAutonegMode
(
    IN GT_SW_DEV_NUM        devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN CPSS_PORT_INTERFACE_MODE_ENT ifMode
)
{
    GT_U32      regAddr;/* register address */
    GT_U32      value;  /* register value */
    GT_U32      portMacNum;      /* MAC number */
    PRV_CPSS_PORT_REG_CONFIG_STC   regDataArray[PRV_CPSS_PORT_NOT_APPLICABLE_E];

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

    switch(ifMode)
    {
        case CPSS_PORT_INTERFACE_MODE_SGMII_E:
        case CPSS_PORT_INTERFACE_MODE_QSGMII_E:
            value = 1;
            break;

        case CPSS_PORT_INTERFACE_MODE_1000BASE_X_E:
            value = 0;
            break;

        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    PRV_CPSS_PX_PORT_MAC_CTRL2_REG_MAC(devNum, portMacNum, PRV_CPSS_PORT_GE_E, &regAddr);
    if(PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);

    if(prvCpssPxPortMacConfigurationClear(regDataArray) != GT_OK)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_INIT_ERROR, LOG_ERROR_NO_MSG);

    regDataArray[PRV_CPSS_PORT_GE_E].regAddr = regAddr;
    regDataArray[PRV_CPSS_PORT_GE_E].fieldData = value;
    regDataArray[PRV_CPSS_PORT_GE_E].fieldLength = 1;
    regDataArray[PRV_CPSS_PORT_GE_E].fieldOffset = 0;

    return prvCpssPxPortMacConfiguration(devNum, portNum, regDataArray);
}


/**
* @internal internal_cpssPxPortInbandAutoNegEnableSet function
* @endinternal
*
* @brief   Configure Auto-Negotiation mode of MAC for a port.
*         The Tri-Speed port MAC may operate in one of the following two modes:
*         - SGMII Mode - In this mode, Auto-Negotiation may be performed
*         out-of-band via the device's Master SMI interface or in-band.
*         The function sets the mode of Auto-Negotiation to in-band or
*         out-of-band.
*         - 1000BASE-X mode - In this mode, the port operates at 1000 Mbps,
*         full-duplex only and supports in-band Auto-Negotiation for link and
*         for Flow Control.
*         The function set in-band Auto-Negotiation mode only.
*
* @note   APPLICABLE DEVICES:      pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] enable                   - GT_TRUE  - Auto-Negotiation works in in-band mode.
*                                      GT_FALSE - Auto-Negotiation works in out-of-band via
*                                      the device's Master SMI interface mode.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - The feature is not supported the port/device
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Not supported for CPU port.
*       SGMII port In-band Auto-Negotiation is performed by the PCS layer to
*       establish link, speed, and duplex mode.
*       1000BASE-X port In-band Auto-Negotiation is performed by the PCS layer
*       to establish link and flow control support.
*       The change of the Auto-Negotiation causes temporary change of the link
*       to down and up for ports with link up.
*       Although for XGMII (10 Gbps) ports feature is not supported the function
*       lets application to configure it
*
*/
static GT_STATUS internal_cpssPxPortInbandAutoNegEnableSet
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_BOOL                 enable
)
{
    GT_STATUS   rc;         /* return code */
    GT_U32      regAddr;         /* register address */
    GT_U32      portState;       /* current port state (enabled/disabled) */
    GT_U32      portGroupId;/*the port group Id - support multi-port-groups device */
    GT_U32      portMacNum;      /* MAC number */
    PRV_CPSS_PORT_REG_CONFIG_STC   regDataArray[PRV_CPSS_PORT_NOT_APPLICABLE_E];

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

    /* convert the 'Physical port' to portGroupId,local port -- supporting multi-port-groups device */
    portGroupId = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portMacNum);

    /* related to GE interfaces only */
    PRV_CPSS_PX_PORT_MAC_CTRL_REG_MAC(CAST_SW_DEVNUM(devNum),portMacNum,PRV_CPSS_PORT_GE_E,&regAddr);
    if(PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);

    /* store value of port state */
    if (prvCpssDrvHwPpPortGroupGetRegField(CAST_SW_DEVNUM(devNum), portGroupId,regAddr, 0, 1, &portState) != GT_OK)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);

    /* Disable port if it's enabled */
    if (portState == 1)
    {
        if (prvCpssDrvHwPpPortGroupSetRegField(CAST_SW_DEVNUM(devNum), portGroupId,regAddr, 0, 1, 0) != GT_OK)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);
    }

    PRV_CPSS_PX_PORT_AUTO_NEG_CTRL_REG_MAC(CAST_SW_DEVNUM(devNum), portMacNum, &regAddr);
    if(PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);

    if(GT_TRUE == enable)
    {
        CPSS_PORT_INTERFACE_MODE_ENT ifMode = PRV_CPSS_PX_PORT_IFMODE_MAC(devNum,portMacNum);
        /* the only modes that the function supports */
        if(ifMode == CPSS_PORT_INTERFACE_MODE_SGMII_E ||
           ifMode == CPSS_PORT_INTERFACE_MODE_QSGMII_E  ||
           ifMode == CPSS_PORT_INTERFACE_MODE_1000BASE_X_E)
        {
            /* no need to check result, application may call this function prior
               to GE ifMode configuration, then this function will be engaged
               during ifModeSet */
            (GT_VOID)prvCpssPxPortInbandAutonegMode(devNum, portNum, ifMode);
        }
    }

    if(prvCpssPxPortMacConfigurationClear(regDataArray) != GT_OK)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_INIT_ERROR, LOG_ERROR_NO_MSG);

    /* Set inband auto-negotiation */
    regDataArray[PRV_CPSS_PORT_GE_E].regAddr = regAddr;
    regDataArray[PRV_CPSS_PORT_GE_E].fieldData = BOOL2BIT_MAC(enable);
    regDataArray[PRV_CPSS_PORT_GE_E].fieldLength = 1;
    regDataArray[PRV_CPSS_PORT_GE_E].fieldOffset = 2;

    rc = prvCpssPxPortMacConfiguration(devNum, portNum, regDataArray);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* Restore original port state */
    if (portState == 1)
    {
        PRV_CPSS_PX_PORT_MAC_CTRL_REG_MAC(devNum,portMacNum,PRV_CPSS_PORT_GE_E,&regAddr);

        if (prvCpssDrvHwPpPortGroupSetRegField(CAST_SW_DEVNUM(devNum), portGroupId,regAddr, 0, 1, 1) != GT_OK)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);
    }

    return GT_OK;
}

/**
* @internal cpssPxPortInbandAutoNegEnableSet function
* @endinternal
*
* @brief   Configure Auto-Negotiation mode of MAC for a port.
*         The Tri-Speed port MAC may operate in one of the following two modes:
*         - SGMII Mode - In this mode, Auto-Negotiation may be performed
*         out-of-band via the device's Master SMI interface or in-band.
*         The function sets the mode of Auto-Negotiation to in-band or
*         out-of-band.
*         - 1000BASE-X mode - In this mode, the port operates at 1000 Mbps,
*         full-duplex only and supports in-band Auto-Negotiation for link and
*         for Flow Control.
*         The function set in-band Auto-Negotiation mode only.
*
* @note   APPLICABLE DEVICES:      pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] enable                   - GT_TRUE  - Auto-Negotiation works in in-band mode.
*                                      GT_FALSE - Auto-Negotiation works in out-of-band via
*                                      the device's Master SMI interface mode.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - The feature is not supported the port/device
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Not supported for CPU port.
*       SGMII port In-band Auto-Negotiation is performed by the PCS layer to
*       establish link, speed, and duplex mode.
*       1000BASE-X port In-band Auto-Negotiation is performed by the PCS layer
*       to establish link and flow control support.
*       The change of the Auto-Negotiation causes temporary change of the link
*       to down and up for ports with link up.
*       Although for XGMII (10 Gbps) ports feature is not supported the function
*       lets application to configure it
*
*/
GT_STATUS cpssPxPortInbandAutoNegEnableSet
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_BOOL                 enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortInbandAutoNegEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enable));

    rc = internal_cpssPxPortInbandAutoNegEnableSet(devNum, portNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortInbandAutoNegEnableGet function
* @endinternal
*
* @brief   Gets Auto-Negotiation mode of MAC for a port.
*         The Tri-Speed port MAC may operate in one of the following two modes:
*         - SGMII Mode - In this mode, Auto-Negotiation may be performed
*         out-of-band via the device's Master SMI interface or in-band.
*         The function sets the mode of Auto-Negotiation to in-band or
*         out-of-band.
*         - 1000BASE-X mode - In this mode, the port operates at 1000 Mbps,
*         full-duplex only and supports in-band Auto-Negotiation for link and
*         for Flow Control.
*         The function set in-band Auto-Negotiation mode only.
*
* @note   APPLICABLE DEVICES:      pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] enablePtr                - GT_TRUE  - Auto-Negotiation works in in-band mode.
*                                      GT_FALSE - Auto-Negotiation works in out-of-band via
*                                      the device's Master SMI interface mode.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - The feature is not supported the port/device
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Not supported for CPU port.
*       SGMII port In-band Auto-Negotiation is performed by the PCS layer to
*       establish link, speed, and duplex mode.
*       1000BASE-X port In-band Auto-Negotiation is performed by the PCS layer
*       to establish link and flow control support.
*       Although for XGMII (10 Gbps) ports feature is not supported the function
*       lets application to configure it
*
*/
static GT_STATUS internal_cpssPxPortInbandAutoNegEnableGet
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_BOOL                 *enablePtr
)
{
    GT_U32 regAddr;         /* register address */
    GT_U32 value;           /* value to write into the register */
    GT_STATUS rc;
    GT_U32      portMacNum;      /* MAC number */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);
    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    PRV_CPSS_PX_PORT_AUTO_NEG_CTRL_REG_MAC(devNum,portMacNum,&regAddr);
    if(PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);

    /* store value of port state */
    rc = prvCpssDrvHwPpPortGroupGetRegField(CAST_SW_DEVNUM(devNum), PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portMacNum),
            regAddr, 2, 1, &value);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);
    }

    *enablePtr = (value == 1) ? GT_TRUE : GT_FALSE;

    return GT_OK;
}

/**
* @internal cpssPxPortInbandAutoNegEnableGet function
* @endinternal
*
* @brief   Gets Auto-Negotiation mode of MAC for a port.
*         The Tri-Speed port MAC may operate in one of the following two modes:
*         - SGMII Mode - In this mode, Auto-Negotiation may be performed
*         out-of-band via the device's Master SMI interface or in-band.
*         The function sets the mode of Auto-Negotiation to in-band or
*         out-of-band.
*         - 1000BASE-X mode - In this mode, the port operates at 1000 Mbps,
*         full-duplex only and supports in-band Auto-Negotiation for link and
*         for Flow Control.
*         The function set in-band Auto-Negotiation mode only.
*
* @note   APPLICABLE DEVICES:      pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] enablePtr                - GT_TRUE  - Auto-Negotiation works in in-band mode.
*                                      GT_FALSE - Auto-Negotiation works in out-of-band via
*                                      the device's Master SMI interface mode.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - The feature is not supported the port/device
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Not supported for CPU port.
*       SGMII port In-band Auto-Negotiation is performed by the PCS layer to
*       establish link, speed, and duplex mode.
*       1000BASE-X port In-band Auto-Negotiation is performed by the PCS layer
*       to establish link and flow control support.
*       Although for XGMII (10 Gbps) ports feature is not supported the function
*       lets application to configure it
*
*/
GT_STATUS cpssPxPortInbandAutoNegEnableGet
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_BOOL                 *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortInbandAutoNegEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enablePtr));

    rc = internal_cpssPxPortInbandAutoNegEnableGet(devNum, portNum, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortMacTypeGet function
* @endinternal
*
* @brief   Get port MAC type.
*
* @note   APPLICABLE DEVICES:      pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] portMacTypePtr           - (pointer to) port MAC type
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPortMacTypeGet
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT CPSS_PORT_MAC_TYPE_ENT  *portMacTypePtr
)
{
    GT_U32                  portMacNum;      /* MAC number */
    PRV_CPSS_PORT_TYPE_ENT  prvPortMacType;

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);
    CPSS_NULL_PTR_CHECK_MAC(portMacTypePtr);

    prvPortMacType = PRV_CPSS_PX_PORT_TYPE_MAC(devNum, portMacNum);

    switch(prvPortMacType)
    {
        case PRV_CPSS_PORT_NOT_EXISTS_E:
            *portMacTypePtr = CPSS_PORT_MAC_TYPE_NOT_EXISTS_E;
            break;

        case PRV_CPSS_PORT_GE_E:
            *portMacTypePtr = CPSS_PORT_MAC_TYPE_GE_E;
            break;

        case PRV_CPSS_PORT_XG_E:
            *portMacTypePtr = CPSS_PORT_MAC_TYPE_XG_E;
            break;

        case PRV_CPSS_PORT_XLG_E:
            *portMacTypePtr = CPSS_PORT_MAC_TYPE_XLG_E;
            break;

        case PRV_CPSS_PORT_HGL_E:
            *portMacTypePtr = CPSS_PORT_MAC_TYPE_HGL_E;
            break;

        case PRV_CPSS_PORT_CG_E:
            *portMacTypePtr = CPSS_PORT_MAC_TYPE_CG_E;
            break;

        case PRV_CPSS_PORT_ILKN_E:
            *portMacTypePtr = CPSS_PORT_MAC_TYPE_ILKN_E;
            break;

        case PRV_CPSS_PORT_NOT_APPLICABLE_E:
            *portMacTypePtr = CPSS_PORT_MAC_TYPE_NOT_APPLICABLE_E;
            break;

        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }
    return GT_OK;
}

/**
* @internal cpssPxPortMacTypeGet function
* @endinternal
*
* @brief   Get port MAC type.
*
* @note   APPLICABLE DEVICES:      pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] portMacTypePtr           - (pointer to) port MAC type
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortMacTypeGet
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT CPSS_PORT_MAC_TYPE_ENT  *portMacTypePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortMacTypeGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, portMacTypePtr));

    rc = internal_cpssPxPortMacTypeGet(devNum, portNum, portMacTypePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, portMacTypePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssPxPortIpgSet function
* @endinternal
*
* @brief   Sets the Inter-Packet Gap (IPG) interval of a tri-speed physical port.
*         Using this API may be required to enable wire-speed in traffic paths
*         that include cascading ports, where it may not be feasible to reduce
*         the preamble length.
*
* @note   APPLICABLE DEVICES:      pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical or CPU port number
* @param[in] ipg                      - IPG in bytes, acceptable range:
*                                      (APPLICABLE RANGES: 0..511) (APPLICABLE DEVICES pipe)
*                                      Default HW value is 12 bytes.
*                                       for CG MAC -->
*                                       1-8 means IPG depend on
*                                       packet size, move
*                                       between 1byte to 8byte
*                                       12 -> mechanism of IPG
*                                       is enabled to create
*                                       average IPG of 12 any
*                                       other setting are
*                                       discarded
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - on wrong port type
* @retval GT_OUT_OF_RANGE          - ipg value out of range
* @retval GT_BAD_PARAM             - on bad parameter
*/
static GT_STATUS internal_cpssPxPortIpgSet
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_U32                  ipg
)
{
    GT_STATUS rc = GT_OK;
    GT_U32  portMacNum;      /* MAC number */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum,portMacNum);

    rc = prvCpssPxPortMacIPGLengthSet(devNum,portNum,ipg);
    return rc;
}

/**
* @internal cpssPxPortIpgSet function
* @endinternal
*
* @brief   Sets the Inter-Packet Gap (IPG) interval of a tri-speed physical port.
*         Using this API may be required to enable wire-speed in traffic paths
*         that include cascading ports, where it may not be feasible to reduce
*         the preamble length.
*
* @note   APPLICABLE DEVICES:      pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical or CPU port number
* @param[in] ipg                      - IPG in bytes, acceptable range:
*                                      (APPLICABLE RANGES: 0..511) (APPLICABLE DEVICES pipe)
*                                      Default HW value is 12 bytes.
*                                       for CG MAC -->
*                                       1-8 means IPG depend on
*                                       packet size, move
*                                       between 1byte to 8byte
*                                       12 -> mechanism of IPG
*                                       is enabled to create
*                                       average IPG of 12 any
*                                       other setting are
*                                       discarded
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - on wrong port type
* @retval GT_OUT_OF_RANGE          - ipg value out of range
* @retval GT_BAD_PARAM             - on bad parameter
*/
GT_STATUS cpssPxPortIpgSet
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_U32                  ipg
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortIpgSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, ipg));

    rc = internal_cpssPxPortIpgSet(devNum, portNum, ipg);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, ipg));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortIpgGet function
* @endinternal
*
* @brief   Gets the Inter-Packet Gap (IPG) interval of a tri-speed physical port.
*
* @note   APPLICABLE DEVICES:      pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical or CPU port number
*
* @param[out] ipgPtr                   - (pointer to) IPG value in bytes
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - on wrong port type
* @retval GT_BAD_PARAM             - on bad parameter
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
static GT_STATUS internal_cpssPxPortIpgGet
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT  GT_U32                 *ipgPtr
)
{
    GT_STATUS   rc = GT_OK;
    GT_U32      portMacNum;      /* MAC number */
    PRV_CPSS_PORT_TYPE_ENT portMacType;

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);
    CPSS_NULL_PTR_CHECK_MAC(ipgPtr);

    *ipgPtr = 0;
    portMacType = PRV_CPSS_PX_PORT_TYPE_MAC(devNum, portMacNum);
    rc = prvCpssPxPortMacIPGLengthGet(devNum, portNum, portMacType,/*OUT*/ipgPtr);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    return GT_OK;
}

/**
* @internal cpssPxPortIpgGet function
* @endinternal
*
* @brief   Gets the Inter-Packet Gap (IPG) interval of a tri-speed physical port.
*
* @note   APPLICABLE DEVICES:      pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical or CPU port number
*
* @param[out] ipgPtr                   - (pointer to) IPG value in bytes
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - on wrong port type
* @retval GT_BAD_PARAM             - on bad parameter
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
GT_STATUS cpssPxPortIpgGet
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT  GT_U32                 *ipgPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortIpgGet);
    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, ipgPtr));

    rc = internal_cpssPxPortIpgGet(devNum, portNum, ipgPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, ipgPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortIpgBaseSet function
* @endinternal
*
* @brief   Sets IPG base for fixed IPG mode on XG ports.
*
* @note   APPLICABLE DEVICES:      pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] ipgBase                  - IPG base
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number, device or ipgBase
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - on not supported feature for a port
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Relevant only when XG Port is in CPSS_PORT_XGMII_FIXED_E mode.
*
*/
static GT_STATUS internal_cpssPxPortIpgBaseSet
(
    IN  GT_SW_DEV_NUM               devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    IN  CPSS_PORT_XG_FIXED_IPG_ENT  ipgBase
)
{
    GT_U32 value;   /* value to write into the register */
    GT_U32 offset;  /* bit number inside register       */
    GT_U32 regAddr;    /* register address */
    GT_U32 portMacNum; /* MAC number */
    PRV_CPSS_PORT_REG_CONFIG_STC   regDataArray[PRV_CPSS_PORT_NOT_APPLICABLE_E];

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

    offset = 9;
    switch(ipgBase)
    {
        case CPSS_PORT_XG_FIXED_IPG_12_BYTES_E:
            value = 0;
            break;
        case CPSS_PORT_XG_FIXED_IPG_8_BYTES_E:
            value = 1;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if(prvCpssPxPortMacConfigurationClear(regDataArray) != GT_OK)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_INIT_ERROR, LOG_ERROR_NO_MSG);

    PRV_CPSS_PX_PORT_MAC_CTRL2_REG_MAC(devNum, portMacNum, PRV_CPSS_PORT_XG_E, &regAddr);
    if(regAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        regDataArray[PRV_CPSS_PORT_XG_E].regAddr = regAddr;
        regDataArray[PRV_CPSS_PORT_XG_E].fieldData = value;
        regDataArray[PRV_CPSS_PORT_XG_E].fieldLength = 1;
        regDataArray[PRV_CPSS_PORT_XG_E].fieldOffset = offset;
    }

    PRV_CPSS_PX_PORT_MAC_CTRL2_REG_MAC(devNum, portMacNum, PRV_CPSS_PORT_XLG_E, &regAddr);
    if(regAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        regDataArray[PRV_CPSS_PORT_XLG_E].regAddr = regAddr;
        regDataArray[PRV_CPSS_PORT_XLG_E].fieldData = value;
        regDataArray[PRV_CPSS_PORT_XLG_E].fieldLength = 1;
        regDataArray[PRV_CPSS_PORT_XLG_E].fieldOffset = offset;
    }

    return prvCpssPxPortMacConfiguration(devNum, portNum, regDataArray);
}

/**
* @internal cpssPxPortIpgBaseSet function
* @endinternal
*
* @brief   Sets IPG base for fixed IPG mode on XG ports.
*
* @note   APPLICABLE DEVICES:      pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] ipgBase                  - IPG base
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number, device or ipgBase
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - on not supported feature for a port
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Relevant only when XG Port is in CPSS_PORT_XGMII_FIXED_E mode.
*
*/
GT_STATUS cpssPxPortIpgBaseSet
(
    IN  GT_SW_DEV_NUM               devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    IN  CPSS_PORT_XG_FIXED_IPG_ENT  ipgBase
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortIpgBaseSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, ipgBase));

    rc = internal_cpssPxPortIpgBaseSet(devNum, portNum, ipgBase);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, ipgBase));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortIpgBaseGet function
* @endinternal
*
* @brief   Gets IPG base for fixed IPG mode on XG ports.
*
* @note   APPLICABLE DEVICES:      pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] ipgBasePtr               - pointer to IPG base
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - on not supported feature for a port
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*
* @note Relevant only when XG Port is in CPSS_PORT_XGMII_FIXED_E mode.
*
*/
static GT_STATUS internal_cpssPxPortIpgBaseGet
(
    IN  GT_SW_DEV_NUM               devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    OUT CPSS_PORT_XG_FIXED_IPG_ENT  *ipgBasePtr
)
{
    GT_U32  value;   /* value read from the register */
    GT_U32  offset;  /* bit number inside register   */
    PRV_CPSS_PORT_TYPE_ENT portMacType;
    GT_U32  regAddr;         /* register address */
    GT_U32  portMacNum;      /* MAC number */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);
    CPSS_NULL_PTR_CHECK_MAC(ipgBasePtr);

    portMacType = PRV_CPSS_PX_PORT_TYPE_MAC(devNum, portMacNum);

    if(portMacType < PRV_CPSS_PORT_XG_E || portMacType == PRV_CPSS_PORT_CG_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }

    offset = 9;
    PRV_CPSS_PX_PORT_MAC_CTRL2_REG_MAC(devNum, portMacNum, portMacType, &regAddr);
    if(PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);

    /* set xgmii mode */
    if (prvCpssDrvHwPpPortGroupGetRegField(CAST_SW_DEVNUM(devNum),
                PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portMacNum),
                regAddr, offset, 1, &value) != GT_OK)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);

    *ipgBasePtr = (value == 0) ? CPSS_PORT_XG_FIXED_IPG_12_BYTES_E :
                                 CPSS_PORT_XG_FIXED_IPG_8_BYTES_E;

    return GT_OK;
}

/**
* @internal cpssPxPortIpgBaseGet function
* @endinternal
*
* @brief   Gets IPG base for fixed IPG mode on XG ports.
*
* @note   APPLICABLE DEVICES:      pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] ipgBasePtr               - pointer to IPG base
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - on not supported feature for a port
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*
* @note Relevant only when XG Port is in CPSS_PORT_XGMII_FIXED_E mode.
*
*/
GT_STATUS cpssPxPortIpgBaseGet
(
    IN  GT_SW_DEV_NUM               devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    OUT CPSS_PORT_XG_FIXED_IPG_ENT  *ipgBasePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortIpgBaseGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, ipgBasePtr));

    rc = internal_cpssPxPortIpgBaseGet(devNum, portNum, ipgBasePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, ipgBasePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortPreambleLengthSet function
* @endinternal
*
* @brief   Set the port with preamble length for Rx or Tx or both directions.
*
* @note   APPLICABLE DEVICES:      pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - physical port number (or CPU port)
* @param[in] direction                - Rx or Tx or both directions
*                                      only XG ports support Rx direction and "both directions"
*                                      options (GE ports support only Tx direction)
* @param[in] length                   -  of preamble in bytes
*                                      support only values of 4,8
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device number or wrong physical port number or
*                                       wrong direction or wrong length
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPortPreambleLengthSet
(
    IN GT_SW_DEV_NUM            devNum,
    IN GT_PHYSICAL_PORT_NUM     portNum,
    IN CPSS_PORT_DIRECTION_ENT  direction,
    IN GT_U32                   length
)
{
    GT_U32  portMacNum;      /* MAC number */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum,portMacNum);

    return prvCpssPxPortMacPreambleLengthSet(devNum, portNum, direction, length);

}

/**
* @internal cpssPxPortPreambleLengthSet function
* @endinternal
*
* @brief   Set the port with preamble length for Rx or Tx or both directions.
*
* @note   APPLICABLE DEVICES:      pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - physical port number (or CPU port)
* @param[in] direction                - Rx or Tx or both directions
*                                      only XG ports support Rx direction and "both directions"
*                                      options (GE ports support only Tx direction)
* @param[in] length                   -  of preamble in bytes
*                                      support only values of 4,8
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device number or wrong physical port number or
*                                       wrong direction or wrong length
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortPreambleLengthSet
(
    IN GT_SW_DEV_NUM            devNum,
    IN GT_PHYSICAL_PORT_NUM     portNum,
    IN CPSS_PORT_DIRECTION_ENT  direction,
    IN GT_U32                   length
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortPreambleLengthSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, direction, length));

    rc = internal_cpssPxPortPreambleLengthSet(devNum, portNum, direction, length);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, direction, length));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortPreambleLengthGet function
* @endinternal
*
* @brief   Get the port with preamble length for Rx or Tx or both directions.
*
* @note   APPLICABLE DEVICES:      pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - physical port number (or CPU port)
* @param[in] direction                - Rx or Tx or both directions
*                                      only XG ports support Rx direction
*                                      GE ports support only Tx direction.
*
* @param[out] lengthPtr                - pointer to preamble length in bytes :
*                                      supported length values are : 4,8.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong device number or wrong physical port number or
*                                       wrong direction or wrong length
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPortPreambleLengthGet
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  CPSS_PORT_DIRECTION_ENT direction,
    OUT GT_U32                  *lengthPtr
)
{
    GT_STATUS   rc = GT_OK;
    GT_U32      portMacNum;      /* MAC number */
    PRV_CPSS_PORT_TYPE_ENT portMacType;

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);
    CPSS_NULL_PTR_CHECK_MAC(lengthPtr);

    portMacType = PRV_CPSS_PX_PORT_TYPE_MAC(devNum, portMacNum);
    *lengthPtr = 0;
    rc = prvCpssPxPortMacPreambleLengthGet(devNum, portNum, portMacType, direction, lengthPtr);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    return GT_OK;
}

/**
* @internal cpssPxPortPreambleLengthGet function
* @endinternal
*
* @brief   Get the port with preamble length for Rx or Tx or both directions.
*
* @note   APPLICABLE DEVICES:      pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - physical port number (or CPU port)
* @param[in] direction                - Rx or Tx or both directions
*                                      only XG ports support Rx direction
*                                      GE ports support only Tx direction.
*
* @param[out] lengthPtr                - pointer to preamble length in bytes :
*                                      supported length values are : 4,8.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong device number or wrong physical port number or
*                                       wrong direction or wrong length
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortPreambleLengthGet(
    IN  GT_SW_DEV_NUM           devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  CPSS_PORT_DIRECTION_ENT direction,
    OUT GT_U32                  *lengthPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortPreambleLengthGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, direction, lengthPtr));

    rc = internal_cpssPxPortPreambleLengthGet(devNum, portNum, direction, lengthPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, direction, lengthPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortPeriodicFcEnableSet function
* @endinternal
*
* @brief   Enable/Disable transmits of periodic 802.3x flow control.
*
* @note   APPLICABLE DEVICES:      pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number (or CPU port)
* @param[in] enable                   - periodic 802.3x flow control tramsition state
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_SUPPORTED         - on unsupported request
*
* @note Note: The gig port cannot disable the xoff, therefore the configuration for the gig port:
*       CPSS_PX_PORT_PERIODIC_FLOW_CONTROL_DISABLE_E = xoff only
*       CPSS_PX_PORT_PERIODIC_FLOW_CONTROL_XON_ONLY_E = both xoff and xon
*       Note: CPSS_PX_PORT_PERIODIC_FLOW_CONTROL_XON_ONLY_E
*       option is not supported.
*
*/
static GT_STATUS internal_cpssPxPortPeriodicFcEnableSet
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  CPSS_PORT_PERIODIC_FLOW_CONTROL_TYPE_ENT   enable
)
{
    GT_U32      portMacNum;      /* MAC number */
    GT_U32      regAddr; /* register address               */
    GT_U32      regValue=0, valueOn=0,valueOff=0;           /* value to write into the register    */
    GT_U32      portGroupId;/*the port group Id - support multi-port-groups device */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

    /* convert the 'Physical port' to portGroupId,local port -- supporting multi-port-groups device */
    portGroupId = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portMacNum);

    switch (enable) {
    case CPSS_PORT_PERIODIC_FLOW_CONTROL_DISABLE_E:
        break;
    case CPSS_PORT_PERIODIC_FLOW_CONTROL_XON_XOFF_E:
        valueOn=1;
        valueOff=1;
        break;
    case CPSS_PORT_PERIODIC_FLOW_CONTROL_XON_ONLY_E:
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
        break;
    case CPSS_PORT_PERIODIC_FLOW_CONTROL_XOFF_ONLY_E:
        valueOff=1;
        break;
    default:
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    regAddr = PRV_PX_REG1_UNIT_GOP_FCA_MAC(devNum, portMacNum).FCACtrl;
    regValue = (valueOff << 11) | (valueOn << 7);
    return prvCpssDrvHwPpPortGroupWriteRegBitMask(CAST_SW_DEVNUM(devNum), portGroupId, regAddr, (BIT_11 | BIT_7), regValue);

}

/**
* @internal cpssPxPortPeriodicFcEnableSet function
* @endinternal
*
* @brief   Enable/Disable transmits of periodic 802.3x flow control.
*
* @note   APPLICABLE DEVICES:      pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] enable                   - periodic 802.3x flow control tramsition state
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_SUPPORTED         - on unsupported request
*
* @note Note: The gig port cannot disable the xoff, therefore the configuration for the gig port:
*       CPSS_PORT_PERIODIC_FLOW_CONTROL_DISABLE_E = xoff only
*       CPSS_PORT_PERIODIC_FLOW_CONTROL_XON_ONLY_E = both xoff and xon
*       Note: CPSS_PORT_PERIODIC_FLOW_CONTROL_XON_ONLY_E
*       option is not supported.
*
*/
GT_STATUS cpssPxPortPeriodicFcEnableSet
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  CPSS_PORT_PERIODIC_FLOW_CONTROL_TYPE_ENT   enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortPeriodicFcEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enable));

    rc = internal_cpssPxPortPeriodicFcEnableSet(devNum, portNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortPeriodicFcEnableGet function
* @endinternal
*
* @brief   Get status of periodic 802.3x flow control transmition.
*
* @note   APPLICABLE DEVICES:      pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number (or CPU port)
*
* @param[out] enablePtr                - Pointer to the periodic 802.3x flow control
*                                      tramsition state.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Note: The gig port cannot disable the xoff, therefore the configuration for the gig port:
*       CPSS_PORT_PERIODIC_FLOW_CONTROL_DISABLE_E = xoff only
*       CPSS_PORT_PERIODIC_FLOW_CONTROL_XON_ONLY_E = both xoff and xon
*
*/
static GT_STATUS internal_cpssPxPortPeriodicFcEnableGet
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT CPSS_PORT_PERIODIC_FLOW_CONTROL_TYPE_ENT   *enablePtr
)
{
    GT_STATUS   rc = GT_OK;
    GT_U32      portMacNum;                     /* MAC number */
    GT_U32      regAddr; /* register address       */
    GT_U32      value;     /* value read from register    */
    GT_U32      portGroupId;/*the port group Id - support multi-port-groups device */

    CPSS_NULL_PTR_CHECK_MAC(enablePtr);
    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum,portMacNum);

    /* convert the 'Physical port' to portGroupId,local port -- supporting multi-port-groups device */
    portGroupId = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portMacNum);

    regAddr = PRV_PX_REG1_UNIT_GOP_FCA_MAC(devNum, portMacNum).FCACtrl;
    rc = prvCpssDrvHwPpPortGroupReadRegBitMask(CAST_SW_DEVNUM(devNum), portGroupId, regAddr, (BIT_11 | BIT_7), &value);
    if(GT_OK != rc)
    {
        return rc;
    }

    switch(value)
    {
        case (0):
            *enablePtr = CPSS_PORT_PERIODIC_FLOW_CONTROL_DISABLE_E;
            break;
        case (BIT_7):
            *enablePtr = CPSS_PORT_PERIODIC_FLOW_CONTROL_XON_ONLY_E;
            break;
        case (BIT_11):
            *enablePtr = CPSS_PORT_PERIODIC_FLOW_CONTROL_XOFF_ONLY_E;
            break;
        case (BIT_7 | BIT_11):
            *enablePtr = CPSS_PORT_PERIODIC_FLOW_CONTROL_XON_XOFF_E;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);
    }
    return GT_OK;
}

/**
* @internal cpssPxPortPeriodicFcEnableGet function
* @endinternal
*
* @brief   Get status of periodic 802.3x flow control transmition.
*
* @note   APPLICABLE DEVICES:      pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number (or CPU port)
*
* @param[out] enablePtr                - Pointer to the periodic 802.3x flow control
*                                      tramsition state.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Note: The gig port cannot disable the xoff, therefore the configuration for the gig port:
*       CPSS_PX_PORT_PERIODIC_FLOW_CONTROL_DISABLE_E = xoff only
*       CPSS_PX_PORT_PERIODIC_FLOW_CONTROL_XON_ONLY_E = both xoff and xon
*
*/
GT_STATUS cpssPxPortPeriodicFcEnableGet
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT CPSS_PORT_PERIODIC_FLOW_CONTROL_TYPE_ENT   *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortPeriodicFcEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enablePtr));

    rc = internal_cpssPxPortPeriodicFcEnableGet(devNum, portNum, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssPxPortPeriodicFlowControlCounterSet function
* @endinternal
*
* @brief   Set Periodic Flow Control interval. The interval in microseconds
*         between two successive Flow Control frames.
*
* @note   APPLICABLE DEVICES:      pipe.
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - port number
* @param[in] value                    - The interval in microseconds between two successive
*                                      Flow Control frames that are sent periodically by the port.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad devNum, portNum
* @retval GT_OUT_OF_RANGE          - on bad value
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPortPeriodicFlowControlCounterSet
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_U32                  value
)
{
    GT_STATUS   rc;         /* return code */
    GT_U32      regAddr;    /* register address */
    GT_U32      regValue;   /* register value */
    GT_U32      portMacNum; /* MAC number */
    GT_U32      periodicEnable; /* surrent status of periodic FC per port */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

    /* store <periodic_enable> field */
    regAddr = PRV_PX_REG1_UNIT_GOP_FCA_MAC(devNum, portMacNum).FCACtrl;
    rc = prvCpssDrvHwPpGetRegField(CAST_SW_DEVNUM(devNum), regAddr, 11, 1, &periodicEnable);
    if(GT_OK != rc)
    {
        return rc;
    }

    /* disable <periodic_enable> field */
    rc = prvCpssDrvHwPpSetRegField(CAST_SW_DEVNUM(devNum), regAddr, 11, 1, 0);
    if(GT_OK != rc)
    {
        return rc;
    }

    if(value > (0xFFFFFFFF / PRV_CPSS_PP_MAC(devNum)->coreClock))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    regValue = value * PRV_CPSS_PP_MAC(devNum)->coreClock;

    regAddr = PRV_PX_REG1_UNIT_GOP_FCA_MAC(devNum, portMacNum).periodicCntrLSB;
    rc = prvCpssDrvHwPpSetRegField(CAST_SW_DEVNUM(devNum), regAddr, 0, 16, U32_GET_FIELD_MAC(regValue, 0, 16));
    if(GT_OK != rc)
    {
        return rc;
    }


    regAddr = PRV_PX_REG1_UNIT_GOP_FCA_MAC(devNum, portMacNum).periodicCntrMSB;
    rc = prvCpssDrvHwPpSetRegField(CAST_SW_DEVNUM(devNum), regAddr, 0, 16, U32_GET_FIELD_MAC(regValue, 16, 16));
    if(GT_OK != rc)
    {
        return rc;
    }

    /* restore <periodic_enable> field */
    regAddr = PRV_PX_REG1_UNIT_GOP_FCA_MAC(devNum, portMacNum).FCACtrl;
    rc = prvCpssDrvHwPpSetRegField(CAST_SW_DEVNUM(devNum), regAddr, 11, 1, periodicEnable);
    return rc;
}

/**
* @internal cpssPxPortPeriodicFlowControlCounterSet function
* @endinternal
*
* @brief   Set Periodic Flow Control interval. The interval in microseconds
*         between two successive Flow Control frames.
*
* @note   APPLICABLE DEVICES:      pipe.
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - port number
* @param[in] value                    - The interval in microseconds between two successive
*                                      Flow Control frames that are sent periodically by the port.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad devNum, portNum
* @retval GT_OUT_OF_RANGE          - on bad value
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The interval in micro seconds between transmission of two consecutive
*       Flow Control packets recommended interval is calculated by the following formula:
*       period (micro seconds) = 33553920 / speed(M)
*       Exception: for 10M, 100M and 10000M Flow Control packets recommended interval is 33500
*       Following are recommended intervals in micro seconds for common port speeds:
*       33500  for speed 10M
*       33500  for speed 100M
*       33500  for speed 1G
*       13421  for speed 2.5G
*       6710  for speed 5G
*       3355  for speed 10G
*       2843  for speed 11.8G
*       2796  for speed 12G
*       2467  for speed 13.6G
*       2236  for speed 15G
*       2097  for speed 16G
*       1677  for speed 20G
*       838   for speed 40G
*       710   for speed 47.2G
*       671   for speed 50G
*       447   for speed 75G
*       335   for speed 100G
*       239   for speed 140G
*
*/
GT_STATUS cpssPxPortPeriodicFlowControlCounterSet
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_U32                  value
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortPeriodicFlowControlCounterSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, value));

    rc = internal_cpssPxPortPeriodicFlowControlCounterSet(devNum, portNum, value);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, value));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssPxPortPeriodicFlowControlCounterGet function
* @endinternal
*
* @brief   Get Periodic Flow Control interval. The interval in microseconds
*         between two successive Flow Control frames.
*
* @note   APPLICABLE DEVICES:      pipe.
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - port number
*
* @param[out] valuePtr                 - (pointer to) The interval in microseconds between two successive
*                                      Flow Control frames that are sent periodically by the port.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad devNum, portNum
* @retval GT_OUT_OF_RANGE          - on bad value
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPortPeriodicFlowControlCounterGet
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_U32                  *valuePtr
)
{
    GT_STATUS   rc;         /* return code */
    GT_U32      regAddr;    /* register address */
    GT_U32      regValue;   /* register value */
    GT_U32      portMacNum; /* MAC number */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);
    CPSS_NULL_PTR_CHECK_MAC(valuePtr);

    regAddr = PRV_PX_REG1_UNIT_GOP_FCA_MAC(devNum, portMacNum).periodicCntrLSB;
    rc = prvCpssDrvHwPpGetRegField(CAST_SW_DEVNUM(devNum), regAddr, 0, 16, &regValue);
    if(GT_OK != rc)
    {
        return rc;
    }

    *valuePtr = regValue;

    regAddr = PRV_PX_REG1_UNIT_GOP_FCA_MAC(devNum, portMacNum).periodicCntrMSB;
    rc = prvCpssDrvHwPpGetRegField(CAST_SW_DEVNUM(devNum), regAddr, 0, 16, &regValue);
    if(GT_OK != rc)
    {
        return rc;
    }

    *valuePtr |= (regValue << 16);
    *valuePtr = *valuePtr / PRV_CPSS_PP_MAC(devNum)->coreClock;
    return GT_OK;
}

/**
* @internal cpssPxPortPeriodicFlowControlCounterGet function
* @endinternal
*
* @brief   Get Periodic Flow Control interval. The interval in microseconds
*         between two successive Flow Control frames.
*
* @note   APPLICABLE DEVICES:      pipe.
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - port number
*
* @param[out] valuePtr                 - (pointer to) The interval in microseconds between two successive
*                                      Flow Control frames that are sent periodically by the port.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad devNum, portNum
* @retval GT_OUT_OF_RANGE          - on bad value
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortPeriodicFlowControlCounterGet
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_U32                  *valuePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortPeriodicFlowControlCounterGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, valuePtr));

    rc = internal_cpssPxPortPeriodicFlowControlCounterGet(devNum, portNum, valuePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, valuePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

#define PRV_CPSS_PX_PORT_MAC_SA_LSB_FLD_LEN_CNS   8
#define PRV_CPSS_PX_PORT_MAC_SA_LSB_FLD_OFFSET_GE_CNS   7
#define PRV_CPSS_PX_PORT_MAC_SA_LSB_FLD_OFFSET_XG_CNS   0
#define PRV_CPSS_PX_PORT_MAC_SA_LSB_FLD_OFFSET_CG_CNS   0
/**
* @internal internal_cpssPxPortMacSaLsbSet function
* @endinternal
*
* @brief   Set the least significant byte of the MAC SA of specified port on
*         specified device.
*
* @note   APPLICABLE DEVICES:      pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] macSaLsb                 - The ls byte of the MAC SA
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The upper 40 bits of the MAC Address are the same for all ports in the
*       device and the lower eight bits are unique per port. The upper 40 bits
*       are configured by cpssPxPortMacSaBaseSet.
*
*/
static GT_STATUS internal_cpssPxPortMacSaLsbSet
(
    IN GT_SW_DEV_NUM            devNum,
    IN GT_PHYSICAL_PORT_NUM     portNum,
    IN GT_U8                    macSaLsb
)
{
    GT_STATUS       rc;              /* return code */
    GT_U32          regAddr;         /* register address */
    GT_U32          value;           /* value to write into the register */
    GT_U32          portMacNum;      /* MAC number */
    PRV_CPSS_PORT_REG_CONFIG_STC   regDataArray[PRV_CPSS_PORT_NOT_APPLICABLE_E];

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

    value = macSaLsb;
    regAddr = PRV_PX_REG1_UNIT_GOP_FCA_MAC(devNum, portMacNum).MACSA0To15;
    rc = prvCpssHwPpSetRegField(CAST_SW_DEVNUM(devNum), regAddr, 0, 8, value);
    if(GT_OK != rc)
    {
        return rc;
    }

    if(prvCpssPxPortMacConfigurationClear(regDataArray) != GT_OK)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_INIT_ERROR, LOG_ERROR_NO_MSG);

    PRV_CPSS_PX_PORT_MAC_CTRL1_REG_MAC(devNum, portMacNum, PRV_CPSS_PORT_GE_E, &regAddr);
    if(regAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        regDataArray[PRV_CPSS_PORT_GE_E].regAddr = regAddr;
        regDataArray[PRV_CPSS_PORT_GE_E].fieldData = value;
        regDataArray[PRV_CPSS_PORT_GE_E].fieldLength = PRV_CPSS_PX_PORT_MAC_SA_LSB_FLD_LEN_CNS;
        regDataArray[PRV_CPSS_PORT_GE_E].fieldOffset = PRV_CPSS_PX_PORT_MAC_SA_LSB_FLD_OFFSET_GE_CNS;
    }

    PRV_CPSS_PX_PORT_MAC_CTRL2_REG_MAC(devNum,portMacNum,PRV_CPSS_PORT_XG_E,&regAddr);
    if(regAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        regDataArray[PRV_CPSS_PORT_XG_E].regAddr = regAddr;
        regDataArray[PRV_CPSS_PORT_XG_E].fieldData = value;
        regDataArray[PRV_CPSS_PORT_XG_E].fieldLength = PRV_CPSS_PX_PORT_MAC_SA_LSB_FLD_LEN_CNS;
        regDataArray[PRV_CPSS_PORT_XG_E].fieldOffset = PRV_CPSS_PX_PORT_MAC_SA_LSB_FLD_OFFSET_XG_CNS;
    }

    PRV_CPSS_PX_PORT_MAC_CTRL2_REG_MAC(devNum, portMacNum, PRV_CPSS_PORT_XLG_E, &regAddr);
    if(regAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        regDataArray[PRV_CPSS_PORT_XLG_E].regAddr = regAddr;
        regDataArray[PRV_CPSS_PORT_XLG_E].fieldData = value;
        regDataArray[PRV_CPSS_PORT_XLG_E].fieldLength = PRV_CPSS_PX_PORT_MAC_SA_LSB_FLD_LEN_CNS;
        regDataArray[PRV_CPSS_PORT_XLG_E].fieldOffset = PRV_CPSS_PX_PORT_MAC_SA_LSB_FLD_OFFSET_XG_CNS;
    }

    /* In CG unit devices, there is a dedicated 32bit register for the LSBs (and another 32 bit
       register for the MSB bites) */
    PRV_CPSS_PX_REG1_CG_PORT_MAC_ADDR0_REG_MAC(devNum,portMacNum,&regAddr);
    if(regAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        /* Setting the dedicated 32 LSBs register */
        regDataArray[PRV_CPSS_PORT_CG_E].regAddr = regAddr;
        regDataArray[PRV_CPSS_PORT_CG_E].fieldData = value;
        regDataArray[PRV_CPSS_PORT_CG_E].fieldLength = PRV_CPSS_PX_PORT_MAC_SA_LSB_FLD_LEN_CNS;
        regDataArray[PRV_CPSS_PORT_CG_E].fieldOffset = PRV_CPSS_PX_PORT_MAC_SA_LSB_FLD_OFFSET_CG_CNS;
    }

    rc = prvCpssPxPortMacConfiguration(devNum, portNum, regDataArray);

    return rc;
}

/**
* @internal cpssPxPortMacSaLsbSet function
* @endinternal
*
* @brief   Set the least significant byte of the MAC SA of specified port on
*         specified device.
*
* @note   APPLICABLE DEVICES:      pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] macSaLsb                 - The ls byte of the MAC SA
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The upper 40 bits of the MAC Address are the same for all ports in the
*       device and the lower eight bits are unique per port. The upper 40 bits
*       are configured by cpssPxPortMacSaBaseSet.
*
*/
GT_STATUS cpssPxPortMacSaLsbSet
(
    IN GT_SW_DEV_NUM            devNum,
    IN GT_PHYSICAL_PORT_NUM     portNum,
    IN GT_U8                    macSaLsb
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortMacSaLsbSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, macSaLsb));

    rc = internal_cpssPxPortMacSaLsbSet(devNum, portNum, macSaLsb);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, macSaLsb));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortMacSaLsbGet function
* @endinternal
*
* @brief   Get the least significant byte of the MAC SA of specified port on
*         specified device.
*
* @note   APPLICABLE DEVICES:      pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] macSaLsbPtr              - (pointer to) The ls byte of the MAC SA
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The upper 40 bits of the MAC Address are the same for all ports in the
*       device and the lower eight bits are unique per port.The upper 40 bits
*       are configured by cpssPxPortMacSaBaseSet.
*
*/
static GT_STATUS internal_cpssPxPortMacSaLsbGet
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_U8                   *macSaLsbPtr
)
{
    GT_U32          regAddr;         /* register address */
    GT_U32          value;
    GT_U32          fieldOffset;     /* start to write register at this bit */
    GT_STATUS       rc = GT_OK;
    PRV_CPSS_PORT_TYPE_ENT portMacType;
    GT_U32          portMacNum;      /* MAC number */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);
    CPSS_NULL_PTR_CHECK_MAC(macSaLsbPtr);

   /* cpssPxPortMacSaLsbSet updates all available MAC's - here enough to read */
    /* one of them */
    portMacType = PRV_CPSS_PX_PORT_TYPE_MAC(devNum,portMacNum);

    if (portMacType >= PRV_CPSS_PORT_XG_E)
    {
        if (portMacType == PRV_CPSS_PORT_CG_E )
        {
            PRV_CPSS_PX_REG1_CG_PORT_MAC_ADDR0_REG_MAC(devNum, portMacNum, &regAddr);
            fieldOffset = PRV_CPSS_PX_PORT_MAC_SA_LSB_FLD_OFFSET_CG_CNS;
        }
        else
        {
            PRV_CPSS_PX_PORT_MAC_CTRL2_REG_MAC(devNum, portMacNum, portMacType, &regAddr);
            fieldOffset = PRV_CPSS_PX_PORT_MAC_SA_LSB_FLD_OFFSET_XG_CNS;
        }
    }
    else
    {
        PRV_CPSS_PX_PORT_MAC_CTRL1_REG_MAC(devNum,portMacNum,portMacType,&regAddr);
        fieldOffset = PRV_CPSS_PX_PORT_MAC_SA_LSB_FLD_OFFSET_GE_CNS;
    }
    if(PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);

    rc = prvCpssDrvHwPpPortGroupGetRegField(CAST_SW_DEVNUM(devNum),
            PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portMacNum),
            regAddr, fieldOffset, PRV_CPSS_PX_PORT_MAC_SA_LSB_FLD_LEN_CNS, &value);
    if(rc != GT_OK)
        return rc;

    *macSaLsbPtr = (GT_U8)value;
    return rc;
}

/**
* @internal cpssPxPortMacSaLsbGet function
* @endinternal
*
* @brief   Get the least significant byte of the MAC SA of specified port on
*         specified device.
*
* @note   APPLICABLE DEVICES:      pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] macSaLsbPtr              - (pointer to) The ls byte of the MAC SA
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The upper 40 bits of the MAC Address are the same for all ports in the
*       device and the lower eight bits are unique per port.The upper 40 bits
*       are configured by cpssPxPortMacSaBaseSet.
*
*/
GT_STATUS cpssPxPortMacSaLsbGet
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_U8                   *macSaLsbPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortMacSaLsbGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, macSaLsbPtr));

    rc = internal_cpssPxPortMacSaLsbGet(devNum, portNum, macSaLsbPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, macSaLsbPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssPxPortMacSaBaseSet function
* @endinternal
*
* @brief   Sets the base part(40 upper bits) of all device's ports MAC addresses.
*         Port MAC addresses are used as the MAC SA for Flow Control Packets
*         transmitted by the device. In addition these addresses can be used as
*         MAC DA for Flow Control packets received by these ports.
*
* @note   APPLICABLE DEVICES:      pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] macPtr                   - (pointer to)The system Mac address to set.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The upper 40 bits of the MAC Address are the same for all ports in the
*       device and the lower eight bits are unique per port.
*
*/
static GT_STATUS internal_cpssPxPortMacSaBaseSet
(
    IN  GT_SW_DEV_NUM   devNum,
    IN  GT_ETHERADDR    *macPtr
)
{
    GT_STATUS rc;               /* Return code */
    GT_U32  regAddr;            /* register address */
    GT_U32  portMacNum;         /* MAC number */
    GT_U32  maxMacNum;          /* max MAC number */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(macPtr);

    for(portMacNum = 0, maxMacNum = 16; portMacNum < maxMacNum; portMacNum++)
    {
        /* MAC SA - bits [15:8] */
        regAddr = PRV_PX_REG1_UNIT_GOP_FCA_MAC(devNum, portMacNum).MACSA0To15;
        rc = prvCpssHwPpSetRegField(CAST_SW_DEVNUM(devNum), regAddr, 8, 8, macPtr->arEther[4]);
        if(GT_OK != rc)
        {
            return rc;
        }

        /* MAC SA - bits [31:16] */
        regAddr = PRV_PX_REG1_UNIT_GOP_FCA_MAC(devNum, portMacNum).MACSA16To31;
        rc = prvCpssHwPpSetRegField(CAST_SW_DEVNUM(devNum), regAddr, 0, 16, ((macPtr->arEther[3]) | (macPtr->arEther[2] << 8)));
        if(GT_OK != rc)
        {
            return rc;
        }

        /* MAC SA - bits [48:32] */
        regAddr = PRV_PX_REG1_UNIT_GOP_FCA_MAC(devNum, portMacNum).MACSA32To47;
        rc = prvCpssHwPpSetRegField(CAST_SW_DEVNUM(devNum), regAddr, 0, 16, ((macPtr->arEther[1]) | (macPtr->arEther[0] << 8)));
        if(GT_OK != rc)
        {
            return rc;
        }
    }
    return GT_OK;
}

/**
* @internal cpssPxPortMacSaBaseSet function
* @endinternal
*
* @brief   Sets the base part(40 upper bits) of all device's ports MAC addresses.
*         Port MAC addresses are used as the MAC SA for Flow Control Packets
*         transmitted by the device. In addition these addresses can be used as
*         MAC DA for Flow Control packets received by these ports.
*
* @note   APPLICABLE DEVICES:      pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] macPtr                   - (pointer to)The system Mac address to set.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The upper 40 bits of the MAC Address are the same for all ports in the
*       device and the lower eight bits are unique per port.
*
*/
GT_STATUS cpssPxPortMacSaBaseSet
(
    IN  GT_SW_DEV_NUM   devNum,
    IN  GT_ETHERADDR    *macPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortMacSaBaseSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, macPtr));

    rc = internal_cpssPxPortMacSaBaseSet(devNum, macPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, macPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssPxPortMacSaBaseGet function
* @endinternal
*
* @brief   Gets the base part (40 upper bits) of all device's ports MAC addresses.
*         Port MAC addresses are used as the MAC SA for Flow Control Packets
*         transmitted by the device.In addition these addresses can be used as
*         MAC DA for Flow Control packets received by these ports.
*
* @note   APPLICABLE DEVICES:      pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] macPtr                   - (pointer to)The system Mac address.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The upper 40 bits of the MAC Address are the same for all ports in the
*       device and the lower eight bits are unique per port.
*
*/
static GT_STATUS internal_cpssPxPortMacSaBaseGet
(
    IN  GT_SW_DEV_NUM   devNum,
    OUT GT_ETHERADDR    *macPtr
)
{
    GT_STATUS rc;               /* Return code */
    GT_U32  regAddr;            /* register address */
    GT_U32  portMacNum;         /* MAC number */
    GT_U32  maxMacNum;          /* max MAC number */
    GT_U32  value;              /* value from register */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(macPtr);

    for(portMacNum = 0, maxMacNum = 16; portMacNum < maxMacNum; portMacNum++)
    {
        /* support not continues MACs of bobk */
        PRV_CPSS_SKIP_NOT_EXIST_PORT_MAC(devNum, portMacNum);

        regAddr = PRV_PX_REG1_UNIT_GOP_FCA_MAC(devNum, portMacNum).MACSA0To15;
        rc = prvCpssHwPpGetRegField(CAST_SW_DEVNUM(devNum), regAddr, 8, 8, &value);
        if(GT_OK != rc)
        {
            return rc;
        }
        macPtr->arEther[4] = (GT_U8)value;

        /* MAC SA - bits [31:16] */
        regAddr = PRV_PX_REG1_UNIT_GOP_FCA_MAC(CAST_SW_DEVNUM(devNum), portMacNum).MACSA16To31;
        rc = prvCpssHwPpGetRegField(CAST_SW_DEVNUM(devNum), regAddr, 0, 16, &value);
        if(GT_OK != rc)
        {
            return rc;
        }
        macPtr->arEther[2] = (GT_U8)(value>>8);
        macPtr->arEther[3] = (GT_U8)value;

        /* MAC SA - bits [48:32] */
        regAddr = PRV_PX_REG1_UNIT_GOP_FCA_MAC(CAST_SW_DEVNUM(devNum), portMacNum).MACSA32To47;
        rc = prvCpssHwPpGetRegField(CAST_SW_DEVNUM(devNum), regAddr, 0, 16, &value);
        if(GT_OK != rc)
        {
            return rc;
        }

        macPtr->arEther[0] = (GT_U8)(value>>8);
        macPtr->arEther[1] = (GT_U8)value;

        /* stop after first valid port */
        return GT_OK;
    }

    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);/* error ... no valid ports */
}

/**
* @internal cpssPxPortMacSaBaseGet function
* @endinternal
*
* @brief   Gets the base part (40 upper bits) of all device's ports MAC addresses.
*         Port MAC addresses are used as the MAC SA for Flow Control Packets
*         transmitted by the device.In addition these addresses can be used as
*         MAC DA for Flow Control packets received by these ports.
*
* @note   APPLICABLE DEVICES:      pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] macPtr                   - (pointer to)The system Mac address.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The upper 40 bits of the MAC Address are the same for all ports in the
*       device and the lower eight bits are unique per port.
*
*/
GT_STATUS cpssPxPortMacSaBaseGet
(
    IN  GT_SW_DEV_NUM   devNum,
    OUT GT_ETHERADDR    *macPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortMacSaBaseGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, macPtr));

    rc = internal_cpssPxPortMacSaBaseGet(devNum, macPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, macPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortXGmiiModeSet function
* @endinternal
*
* @brief   Sets XGMII mode on specified device.
*
* @note   APPLICABLE DEVICES:      pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] mode                     - XGMII mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - on not supported feature for a port
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPortXGmiiModeSet
(
    IN  GT_SW_DEV_NUM               devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    IN  CPSS_PORT_XGMII_MODE_ENT    mode
)
{
    GT_U32  value;          /* value to write into the register */
    GT_U32  offset;         /* field offset */
    GT_U32  regAddr;        /* register address */
    GT_U32  portMacNum;     /* MAC number */
    PRV_CPSS_PORT_REG_CONFIG_STC   regDataArray[PRV_CPSS_PORT_NOT_APPLICABLE_E];

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

    switch(mode)
    {
        case CPSS_PORT_XGMII_LAN_E:
            value = 0;
            break;
        case CPSS_PORT_XGMII_WAN_E:
            value = 1;
            break;
        case CPSS_PORT_XGMII_FIXED_E:
            value = 2;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    offset = 5;
    if(prvCpssPxPortMacConfigurationClear(regDataArray) != GT_OK)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_INIT_ERROR, LOG_ERROR_NO_MSG);

    /* Setting the XGMII Transmit Inter-Packet Gap (IPG) mode */
    PRV_CPSS_PX_PORT_MAC_CTRL_REG_MAC(devNum, portMacNum, PRV_CPSS_PORT_XG_E, &regAddr);
    if(regAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        regDataArray[PRV_CPSS_PORT_XG_E].regAddr = regAddr;
        regDataArray[PRV_CPSS_PORT_XG_E].fieldData = value;
        regDataArray[PRV_CPSS_PORT_XG_E].fieldLength = 2;
        regDataArray[PRV_CPSS_PORT_XG_E].fieldOffset = offset;
    }

    PRV_CPSS_PX_PORT_MAC_CTRL_REG_MAC(devNum, portMacNum, PRV_CPSS_PORT_XLG_E, &regAddr);
    if(regAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        regDataArray[PRV_CPSS_PORT_XLG_E].regAddr = regAddr;
        regDataArray[PRV_CPSS_PORT_XLG_E].fieldData = value;
        regDataArray[PRV_CPSS_PORT_XLG_E].fieldLength = 2;
        regDataArray[PRV_CPSS_PORT_XLG_E].fieldOffset = offset;
    }

    return prvCpssPxPortMacConfiguration(devNum, portNum, regDataArray);
}

/**
* @internal cpssPxPortXGmiiModeSet function
* @endinternal
*
* @brief   Sets XGMII mode on specified device.
*
* @note   APPLICABLE DEVICES:      pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] mode                     - XGMII mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - on not supported feature for a port
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortXGmiiModeSet
(
    IN  GT_SW_DEV_NUM               devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    IN  CPSS_PORT_XGMII_MODE_ENT    mode
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortXGmiiModeSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, mode));

    rc = internal_cpssPxPortXGmiiModeSet(devNum, portNum, mode);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, mode));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortXGmiiModeGet function
* @endinternal
*
* @brief   Gets XGMII mode on specified device.
*
* @note   APPLICABLE DEVICES:      pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] modePtr                  - Pointer to XGMII mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - on not supported feature for a port
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPortXGmiiModeGet
(
    IN  GT_SW_DEV_NUM               devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    OUT CPSS_PORT_XGMII_MODE_ENT    *modePtr
)
{
    GT_U32  value;   /* value read from the register */
    GT_U32  offset;  /* field offset */
    GT_U32  regAddr;         /* register address */
    GT_U32  portMacNum;      /* MAC number */
    PRV_CPSS_PORT_TYPE_ENT portMacType;

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);
    CPSS_NULL_PTR_CHECK_MAC(modePtr);

    portMacType = PRV_CPSS_PX_PORT_TYPE_MAC(devNum,portMacNum);

    if(portMacType < PRV_CPSS_PORT_XG_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }

    offset = 5;

    /* CG unit supports only LAN PHY mode */
    if (portMacType == PRV_CPSS_PORT_CG_E)
    {
        *modePtr = CPSS_PORT_XGMII_LAN_E;
        return GT_OK;
    }

    /* get xgmii mode */
    PRV_CPSS_PX_PORT_MAC_CTRL_REG_MAC(devNum, portMacNum, portMacType, &regAddr);
    if(PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);

    if (prvCpssDrvHwPpPortGroupGetRegField(CAST_SW_DEVNUM(devNum),
            PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portMacNum),
            regAddr, offset, 2, &value) != GT_OK)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);

    switch(value)
    {
        case 0:
            *modePtr = CPSS_PORT_XGMII_LAN_E;
            break;
        case 1:
            *modePtr = CPSS_PORT_XGMII_WAN_E;
            break;
        case 2:
            *modePtr = CPSS_PORT_XGMII_FIXED_E;
            break;
        default:
            break;
    }

    return GT_OK;
}

/**
* @internal cpssPxPortXGmiiModeGet function
* @endinternal
*
* @brief   Gets XGMII mode on specified device.
*
* @note   APPLICABLE DEVICES:      pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] modePtr                  - Pointer to XGMII mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - on not supported feature for a port
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortXGmiiModeGet
(
    IN  GT_SW_DEV_NUM               devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    OUT CPSS_PORT_XGMII_MODE_ENT    *modePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortXGmiiModeGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, modePtr));

    rc = internal_cpssPxPortXGmiiModeGet(devNum, portNum, modePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, modePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortExtraIpgSet function
* @endinternal
*
* @brief   Sets the number of 32-bit words to add to the 12-byte IPG.
*         Hence, 12+4ExtraIPG is the basis for the entire IPG calculation.
*
* @note   APPLICABLE DEVICES:      pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] number                   -   of words
*                                      (APPLICABLE RANGES: 0..127)
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - the number is out of range
* @retval GT_NOT_SUPPORTED         - on not supported feature for a port
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPortExtraIpgSet
(
    IN  GT_SW_DEV_NUM        devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  GT_U8                number
)
{
    GT_U32 value;           /* value to write into the register */
    GT_U32  regAddr;    /* register address */
    GT_U32 offset;      /* bit number inside register       */
    GT_U32 sizeOfValue; /* number of bits to write to the register       */
    GT_U32  portMacNum;      /* MAC number */
    PRV_CPSS_PORT_REG_CONFIG_STC   regDataArray[PRV_CPSS_PORT_NOT_APPLICABLE_E];

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

    value = number;

    /* set */
    if(value >= BIT_7)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }
    sizeOfValue=7;
    offset=6;

    if(prvCpssPxPortMacConfigurationClear(regDataArray) != GT_OK)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_INIT_ERROR, LOG_ERROR_NO_MSG);

    PRV_CPSS_PX_PORT_MAC_CTRL3_REG_MAC(devNum, portMacNum, PRV_CPSS_PORT_XG_E, &regAddr);
    if(regAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        regDataArray[PRV_CPSS_PORT_XG_E].regAddr = regAddr;
        regDataArray[PRV_CPSS_PORT_XG_E].fieldData = value;
        regDataArray[PRV_CPSS_PORT_XG_E].fieldLength = sizeOfValue;
        regDataArray[PRV_CPSS_PORT_XG_E].fieldOffset = offset;
    }

    PRV_CPSS_PX_PORT_MAC_CTRL3_REG_MAC(devNum, portMacNum, PRV_CPSS_PORT_XLG_E, &regAddr);
    if(regAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        regDataArray[PRV_CPSS_PORT_XLG_E].regAddr = regAddr;
        regDataArray[PRV_CPSS_PORT_XLG_E].fieldData = value;
        regDataArray[PRV_CPSS_PORT_XLG_E].fieldLength = sizeOfValue;
        regDataArray[PRV_CPSS_PORT_XLG_E].fieldOffset = offset;
    }

    return prvCpssPxPortMacConfiguration(devNum, portNum, regDataArray);
}

/**
* @internal cpssPxPortExtraIpgSet function
* @endinternal
*
* @brief   Sets the number of 32-bit words to add to the 12-byte IPG.
*         Hence, 12+4ExtraIPG is the basis for the entire IPG calculation.
*
* @note   APPLICABLE DEVICES:      pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] number                   -   of words
*                                      (APPLICABLE RANGES: 0..127)
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - the number is out of range
* @retval GT_NOT_SUPPORTED         - on not supported feature for a port
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortExtraIpgSet
(
    IN  GT_SW_DEV_NUM        devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  GT_U8                number
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortExtraIpgSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, number));

    rc = internal_cpssPxPortExtraIpgSet(devNum, portNum, number);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, number));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortExtraIpgGet function
* @endinternal
*
* @brief   Gets the number of 32-bit words to add to the 12-byte IPG.
*         Hence, 12+4ExtraIPG is the basis for the entire IPG calculation.
*
* @note   APPLICABLE DEVICES:      pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] numberPtr                -  pointer to number of words
*                                      (APPLICABLE RANGES: 0..127)
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - the number is out of range
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_SUPPORTED         - on not supported feature for a port
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPortExtraIpgGet
(
    IN  GT_SW_DEV_NUM        devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_U8                *numberPtr
)
{
    GT_U32 value;           /* value to write into the register */
    GT_U32  regAddr;
    GT_STATUS rc;
    GT_U32  portGroupId;/*the port group Id - support multi-port-groups device */
    PRV_CPSS_PORT_TYPE_ENT portMacType;
    GT_U32  portMacNum;      /* MAC number */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);
    CPSS_NULL_PTR_CHECK_MAC(numberPtr);

    /* convert the 'Physical port' to portGroupId,local port -- supporting multi-port-groups device */
    portGroupId = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portMacNum);
    portMacType = PRV_CPSS_PX_PORT_TYPE_MAC(devNum,portMacNum);

    if(portMacType < PRV_CPSS_PORT_XG_E || portMacType == PRV_CPSS_PORT_CG_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }

    PRV_CPSS_PX_PORT_MAC_CTRL3_REG_MAC(devNum, portMacNum, portMacType, &regAddr);
    if(PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);

    rc = prvCpssDrvHwPpPortGroupGetRegField(CAST_SW_DEVNUM(devNum), portGroupId, regAddr, 6, 7, &value);
    if (rc != GT_OK)
        return rc;

    *numberPtr = (GT_U8)value;

    return GT_OK;
}

/**
* @internal cpssPxPortExtraIpgGet function
* @endinternal
*
* @brief   Gets the number of 32-bit words to add to the 12-byte IPG.
*         Hence, 12+4ExtraIPG is the basis for the entire IPG calculation.
*
* @note   APPLICABLE DEVICES:      pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] numberPtr                -  pointer to number of words
*                                      (APPLICABLE RANGES: 0..127)
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - the number is out of range
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_SUPPORTED         - on not supported feature for a port
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortExtraIpgGet
(
    IN  GT_SW_DEV_NUM        devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_U8                *numberPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortExtraIpgGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, numberPtr));

    rc = internal_cpssPxPortExtraIpgGet(devNum, portNum, numberPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, numberPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal prvCpssPxPortRemoteFaultConfigGet function
* @endinternal
*
* @brief   Reads the remote fault confuguration from mac control register.
*
* @note   APPLICABLE DEVICES:      pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] isRemoteFaultPtr        - GT_TRUE, if bit is set or
*                                       GT_FALSE otherwise
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - on not supported MAC (other than XLG or CG)
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssPxPortRemoteFaultConfigGet
(
    IN  GT_SW_DEV_NUM        devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_BOOL              *isRemoteFaultPtr
)
{
    GT_U32                  portMacNum, regAddr, value;
    PRV_CPSS_PORT_TYPE_ENT  portMacType;

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);
    CPSS_NULL_PTR_CHECK_MAC(isRemoteFaultPtr);

    portMacType = PRV_CPSS_PX_PORT_TYPE_MAC(devNum,portMacNum);
    if(portMacType == PRV_CPSS_PORT_CG_E)
    {
        /*PRV_CPSS_PX_PORT_MAC_CTRL_REG_MAC()??*/
        PRV_CPSS_PX_REG1_CG_CONVERTERS_CTRL0_REG_MAC(devNum,portMacNum,&regAddr);
        if(PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);

        if (prvCpssDrvHwPpPortGroupGetRegField(devNum,
                                               PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portMacNum),
                                               regAddr, 1, 1, &value) != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);
        }
    }
    else if(portMacType == PRV_CPSS_PORT_XLG_E)
    {
        PRV_CPSS_PX_PORT_MAC_CTRL2_REG_MAC(devNum,portMacNum,portMacType,&regAddr);

        if(PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);

        if (prvCpssDrvHwPpPortGroupGetRegField(CAST_SW_DEVNUM(devNum),
                                               PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portMacNum),
                                               regAddr, 12, 1, &value) != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);
        }
    }
    else
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }

    *isRemoteFaultPtr = BIT2BOOL_MAC(value);
    return GT_OK;
}

/**
* @internal internal_cpssPxPortXgmiiLocalFaultGet function
* @endinternal
*
* @brief   Reads bit then indicate if the XGMII RS has detected local
*         fault messages.
*
* @note   APPLICABLE DEVICES:      pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] isLocalFaultPtr          - GT_TRUE, if bit set or
*                                      GT_FALSE otherwise
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - on not supported feature for a port
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPortXgmiiLocalFaultGet
(
    IN  GT_SW_DEV_NUM        devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_BOOL              *isLocalFaultPtr
)
{
    GT_U32 value = 0;
    GT_U32  regAddr;
    GT_U32  portMacNum;      /* MAC number */
    GT_U32  offset;
    GT_U32  length;
    GT_BOOL canReadCg;

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);
    CPSS_NULL_PTR_CHECK_MAC(isLocalFaultPtr);

    if(PRV_CPSS_PX_PORT_TYPE_MAC(devNum,portMacNum) < PRV_CPSS_PORT_XG_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }

    if(PRV_CPSS_PX_PORT_TYPE_MAC(devNum,portMacNum) == PRV_CPSS_PORT_CG_E)
    {
        PRV_CPSS_PX_REG1_CG_PORT_MAC_STATUS_MAC(devNum,portMacNum,&regAddr);
        offset = 0;
        length = 1;
        if(PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);

        /* check if cg is in reset and with no clk */
        mvHwsCgMac28nmAccessLock(devNum, portMacNum);
        canReadCg = mvHwsCgMac28nmAccessGet(devNum, PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portMacNum), portMacNum);
        if (canReadCg == GT_TRUE)
        {
            if (prvCpssDrvHwPpPortGroupGetRegField(CAST_SW_DEVNUM(devNum),
                PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum,portMacNum),
                regAddr, offset, length, &value) != GT_OK)
            {
                mvHwsCgMac28nmAccessUnlock(devNum, portMacNum);
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);
            }
        }
        mvHwsCgMac28nmAccessUnlock(devNum, portMacNum);
    }
    else
    {
        PRV_CPSS_PX_PORT_STATUS_CTRL_REG_MAC(devNum, portMacNum, &regAddr);
        offset = 2;
        length = 1;

        if(PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);

        if (prvCpssDrvHwPpPortGroupGetRegField(CAST_SW_DEVNUM(devNum),
                                               PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portMacNum),
                                               regAddr, offset, length, &value) != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);
        }
    }

    *isLocalFaultPtr = BIT2BOOL_MAC(value);

    return GT_OK;
}

/**
* @internal cpssPxPortXgmiiLocalFaultGet function
* @endinternal
*
* @brief   Reads bit then indicate if the XGMII RS has detected local
*         fault messages.
*
* @note   APPLICABLE DEVICES:      pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] isLocalFaultPtr          - GT_TRUE, if bit set or
*                                      GT_FALSE otherwise
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - on not supported feature for a port
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortXgmiiLocalFaultGet
(
    IN  GT_SW_DEV_NUM        devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_BOOL              *isLocalFaultPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortXgmiiLocalFaultGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, isLocalFaultPtr));

    rc = internal_cpssPxPortXgmiiLocalFaultGet(devNum, portNum, isLocalFaultPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, isLocalFaultPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortXgmiiRemoteFaultGet function
* @endinternal
*
* @brief   Reads bit then indicate if the XGMII RS has detected remote
*         fault messages.
*
* @note   APPLICABLE DEVICES:      pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] isRemoteFaultPtr         - GT_TRUE, if bit set or
*                                      GT_FALSE otherwise
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - on not supported feature for a port
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPortXgmiiRemoteFaultGet
(
    IN  GT_U8       devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_BOOL     *isRemoteFaultPtr
)
{
    GT_U32 value = 0;
    GT_U32  regAddr;
    GT_U32  portMacNum;      /* MAC number */
    GT_BOOL canReadCg;

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);
    CPSS_NULL_PTR_CHECK_MAC(isRemoteFaultPtr);

    if(PRV_CPSS_PX_PORT_TYPE_MAC(devNum,portMacNum) < PRV_CPSS_PORT_XG_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }

    if(PRV_CPSS_PX_PORT_TYPE_MAC(devNum,portMacNum) == PRV_CPSS_PORT_CG_E)
    {
        PRV_CPSS_PX_REG1_CG_PORT_MAC_STATUS_MAC(devNum,portMacNum,&regAddr);
        if(PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);

        /* check if cg is in reset and with no clk */
        mvHwsCgMac28nmAccessLock(devNum, portMacNum);
        canReadCg = mvHwsCgMac28nmAccessGet(devNum, PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portMacNum), portMacNum);
        if (canReadCg == GT_TRUE)
        {
            if (prvCpssDrvHwPpPortGroupGetRegField(devNum,
                    PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portMacNum),
                        regAddr, 1, 1, &value) != GT_OK)
            {
                mvHwsCgMac28nmAccessUnlock(devNum, portMacNum);
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);
            }
        }
        mvHwsCgMac28nmAccessUnlock(devNum, portMacNum);
    }
    else
    {
        PRV_CPSS_PX_PORT_STATUS_CTRL_REG_MAC(devNum, portMacNum, &regAddr);

        if(PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);

        if (prvCpssDrvHwPpPortGroupGetRegField(CAST_SW_DEVNUM(devNum),
                                               PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portMacNum),
                                               regAddr, 1, 1, &value) != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);
        }
    }

    *isRemoteFaultPtr = BIT2BOOL_MAC(value);
    return GT_OK;
}

/**
* @internal cpssPxPortXgmiiRemoteFaultGet function
* @endinternal
*
* @brief   Reads bit then indicate if the XGMII RS has detected remote
*         fault messages.
*
* @note   APPLICABLE DEVICES:      pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] isRemoteFaultPtr         - GT_TRUE, if bit set or
*                                      GT_FALSE otherwise
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - on not supported feature for a port
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortXgmiiRemoteFaultGet
(
    IN  GT_SW_DEV_NUM        devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_BOOL              *isRemoteFaultPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortXgmiiRemoteFaultGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, isRemoteFaultPtr));

    rc = internal_cpssPxPortXgmiiRemoteFaultGet(devNum, portNum, isRemoteFaultPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, isRemoteFaultPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortMacStatusGet function
* @endinternal
*
* @brief   Reads bits that indicate different problems on specified port.
*
* @note   APPLICABLE DEVICES:      pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] portMacStatusPtr         - info about port MAC
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPortMacStatusGet
(
    IN  GT_SW_DEV_NUM              devNum,
    IN  GT_PHYSICAL_PORT_NUM       portNum,
    OUT CPSS_PORT_MAC_STATUS_STC   *portMacStatusPtr
)
{
    GT_U32 regAddr;         /* register address */
    GT_U32 value;
    GT_U32 cgValue1,cgValue2,cgValue3;
    GT_U32 portMacNum;      /* MAC number */
    GT_U32 xoff_status = 1; /* The status is XOFF when xoff_status = 0 */
    GT_BOOL canReadCg; /* check if cg is in reset and with no clk */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);
    CPSS_NULL_PTR_CHECK_MAC(portMacStatusPtr);

    value = 0;
    cgValue1 = 0;
    cgValue2 = 0;
    cgValue3 = 0;

    if (PRV_CPSS_PX_PORT_TYPE_MAC(devNum, portMacNum) != PRV_CPSS_PORT_CG_E)
    {
        PRV_CPSS_PX_PORT_STATUS_CTRL_REG_MAC(devNum, portMacNum, &regAddr);
        if(PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);

        if (prvCpssDrvHwPpPortGroupReadRegister(CAST_SW_DEVNUM(devNum),
                                                PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portMacNum),
                                                regAddr, &value) != GT_OK)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);
    }
    else /* For CG MAC the status bits are in different registers */
    {
        PRV_CPSS_PX_REG1_CG_CONVERTERS_STATUS_REG_MAC(devNum, portMacNum, &regAddr);
        if(PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);

        if (prvCpssDrvHwPpPortGroupReadRegister(CAST_SW_DEVNUM(devNum),
                                                PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portMacNum),
                                                regAddr, &cgValue1) != GT_OK)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);

        PRV_CPSS_PX_REG1_CG_PORT_MAC_STATUS_MAC(devNum,portMacNum,&regAddr);
        if(PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);

        mvHwsCgMac28nmAccessLock(devNum, portMacNum);
        canReadCg = mvHwsCgMac28nmAccessGet(devNum, PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portMacNum), portMacNum);
        if (canReadCg == GT_FALSE)
        {
            cgValue2 = 0;
        }
        else
        {
        if (prvCpssDrvHwPpPortGroupReadRegister(CAST_SW_DEVNUM(devNum),
                                                PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portMacNum),
                                                regAddr, &cgValue2) != GT_OK)
            {
                mvHwsCgMac28nmAccessUnlock(devNum, portMacNum);
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);
            }
        }
        mvHwsCgMac28nmAccessUnlock(devNum, portMacNum);

        PRV_CPSS_PX_REG1_CG_CONVERTERS_IP_STATUS_REG_MAC(devNum, portMacNum, &regAddr);
        if(PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);

        if (prvCpssDrvHwPpPortGroupReadRegister(CAST_SW_DEVNUM(devNum),
                                                PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portMacNum),
                                                regAddr, &cgValue3) != GT_OK)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);
    }

    /* Read XOFF status from FCA DB register0 */
    if(prvCpssHwPpGetRegField(CAST_SW_DEVNUM(devNum),
                              PRV_PX_REG1_UNIT_GOP_FCA_MAC(devNum, portMacNum).DBReg0, 0, 1, &xoff_status) != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);
    }

    /* CG unit */
    if (PRV_CPSS_PX_PORT_TYPE_MAC(devNum,portMacNum) == PRV_CPSS_PORT_CG_E)
    {
        portMacStatusPtr->isPortRxPause  = GT_FALSE;

        if(xoff_status == 0)
        {
            portMacStatusPtr->isPortTxPause = GT_TRUE;
        }
        else
        {
            portMacStatusPtr->isPortTxPause = GT_FALSE;
        }
        portMacStatusPtr->isPortBackPres =    GT_FALSE;
        portMacStatusPtr->isPortBufFull  =    (cgValue1 & 0x1) ? GT_TRUE : GT_FALSE;/* Bit#0 */
        portMacStatusPtr->isPortSyncFail =    (cgValue3 & (0x1 << 29)) ? GT_FALSE : GT_TRUE; /* bit#29 -> SyncFail */
        portMacStatusPtr->isPortHiErrorRate = GT_FALSE;
        portMacStatusPtr->isPortAnDone =      GT_FALSE;
        portMacStatusPtr->isPortFatalError =  (cgValue2 & 0x3) ? GT_TRUE : GT_FALSE; /* Bit#0 Or Bit#1 */
    }
    else{
        /*  10 Gb interface */
        if(PRV_CPSS_PX_PORT_TYPE_MAC(devNum,portMacNum) >= PRV_CPSS_PORT_XG_E)
        {
            portMacStatusPtr->isPortRxPause  =    (value & 0x40) ? GT_TRUE : GT_FALSE;
            portMacStatusPtr->isPortTxPause  =    (value & 0x80) ? GT_TRUE : GT_FALSE;

            /* Return XOFF status from FCA DB register0 */
            if(xoff_status == 0){
                portMacStatusPtr->isPortTxPause = GT_TRUE;
            }
            else
            {
                portMacStatusPtr->isPortTxPause = GT_FALSE;
            }

            portMacStatusPtr->isPortBackPres =    GT_FALSE;
            portMacStatusPtr->isPortBufFull  =    (value & 0x100) ? GT_TRUE : GT_FALSE;/* Bit#8 */
            portMacStatusPtr->isPortSyncFail =    (value & 0x1) ? GT_FALSE : GT_TRUE; /* value 0 -> SyncFail */
            portMacStatusPtr->isPortHiErrorRate = GT_FALSE;
            portMacStatusPtr->isPortAnDone =      GT_FALSE;
            portMacStatusPtr->isPortFatalError =  (value & 0x6) ? GT_TRUE : GT_FALSE; /* Bit#1 Or Bit#2 */
        }
        else
        {
            portMacStatusPtr->isPortRxPause = (value & 0x40) ? GT_TRUE : GT_FALSE;
            portMacStatusPtr->isPortTxPause = (value & 0x80) ? GT_TRUE : GT_FALSE;
            portMacStatusPtr->isPortBackPres = (value & 0x100) ? GT_TRUE : GT_FALSE;
            portMacStatusPtr->isPortBufFull = (value & 0x200) ? GT_TRUE : GT_FALSE;
            portMacStatusPtr->isPortSyncFail = (value & 0x400) ? GT_TRUE : GT_FALSE;
            portMacStatusPtr->isPortAnDone = (value & 0x800) ? GT_TRUE : GT_FALSE;

            portMacStatusPtr->isPortHiErrorRate = GT_FALSE;
            portMacStatusPtr->isPortFatalError = GT_FALSE;
        }
    }

    return GT_OK;
}

/**
* @internal cpssPxPortMacStatusGet function
* @endinternal
*
* @brief   Reads bits that indicate different problems on specified port.
*
* @note   APPLICABLE DEVICES:      pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] portMacStatusPtr         - info about port MAC
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortMacStatusGet
(
    IN  GT_SW_DEV_NUM              devNum,
    IN  GT_PHYSICAL_PORT_NUM       portNum,
    OUT CPSS_PORT_MAC_STATUS_STC   *portMacStatusPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortMacStatusGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, portMacStatusPtr));

    rc = internal_cpssPxPortMacStatusGet(devNum, portNum, portMacStatusPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, portMacStatusPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortPaddingEnableSet function
* @endinternal
*
* @brief   Enable/Disable padding of transmitted packets shorter than 64B.
*
* @note   APPLICABLE DEVICES:      pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number (or CPU port).
* @param[in] enable                   - GT_TRUE  - Pad short packet in Tx.
*                                      - GT_FALSE - No padding in short packets.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_FAIL                  - on error
*/
static GT_STATUS internal_cpssPxPortPaddingEnableSet
(
    IN  GT_SW_DEV_NUM        devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  GT_BOOL              enable
)
{
    GT_U32      portMacNum;      /* MAC number */
    GT_U32      regAddr; /* register address */
    GT_U32      value;   /* register field value */
    GT_U32      offset;  /* field offset */
    PRV_CPSS_PORT_REG_CONFIG_STC   regDataArray[PRV_CPSS_PORT_NOT_APPLICABLE_E];

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

    if(prvCpssPxPortMacConfigurationClear(regDataArray) != GT_OK)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_INIT_ERROR, LOG_ERROR_NO_MSG);

    value = (enable == GT_TRUE) ? 0 : 1;
    PRV_CPSS_PX_PORT_MAC_CTRL2_REG_MAC(devNum, portMacNum, PRV_CPSS_PORT_GE_E, &regAddr);
    if(regAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        regDataArray[PRV_CPSS_PORT_GE_E].regAddr = regAddr;
        regDataArray[PRV_CPSS_PORT_GE_E].fieldData = value;
        regDataArray[PRV_CPSS_PORT_GE_E].fieldLength = 1;
        regDataArray[PRV_CPSS_PORT_GE_E].fieldOffset = 5;
    }

    PRV_CPSS_PX_PORT_MAC_CTRL_REG_MAC(devNum, portMacNum, PRV_CPSS_PORT_XG_E, &regAddr);
    offset = 13;
    if(regAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        regDataArray[PRV_CPSS_PORT_XG_E].regAddr = regAddr;
        regDataArray[PRV_CPSS_PORT_XG_E].fieldData = value;
        regDataArray[PRV_CPSS_PORT_XG_E].fieldLength = 1;
        regDataArray[PRV_CPSS_PORT_XG_E].fieldOffset = offset;
    }

    PRV_CPSS_PX_PORT_MAC_CTRL_REG_MAC(devNum, portMacNum, PRV_CPSS_PORT_XLG_E, &regAddr);
    if(regAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        regDataArray[PRV_CPSS_PORT_XLG_E].regAddr = regAddr;
        regDataArray[PRV_CPSS_PORT_XLG_E].fieldData = value;
        regDataArray[PRV_CPSS_PORT_XLG_E].fieldLength = 1;
        regDataArray[PRV_CPSS_PORT_XLG_E].fieldOffset = offset;
    }

    return prvCpssPxPortMacConfiguration(devNum, portNum, regDataArray);
}

/**
* @internal cpssPxPortPaddingEnableSet function
* @endinternal
*
* @brief   Enable/Disable padding of transmitted packets shorter than 64B.
*
* @note   APPLICABLE DEVICES:      pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number (or CPU port).
* @param[in] enable                   - GT_TRUE  - Pad short packet in Tx.
*                                      - GT_FALSE - No padding in short packets.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_FAIL                  - on error
*/
GT_STATUS cpssPxPortPaddingEnableSet
(
    IN  GT_SW_DEV_NUM        devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  GT_BOOL              enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortPaddingEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enable));

    rc = internal_cpssPxPortPaddingEnableSet(devNum, portNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortPaddingEnableGet function
* @endinternal
*
* @brief   Gets padding status of transmitted packets shorter than 64B.
*
* @note   APPLICABLE DEVICES:      pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number (or CPU port).
*
* @param[out] enablePtr                - pointer to packet padding status.
*                                      - GT_TRUE  - Pad short packet in Tx.
*                                      - GT_FALSE - No padding in short packets.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
static GT_STATUS internal_cpssPxPortPaddingEnableGet
(
    IN  GT_SW_DEV_NUM        devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_BOOL              *enablePtr
)
{
    GT_STATUS   rc = GT_OK;                      /* return status */
    GT_U32      portMacNum;      /* MAC number */
    GT_U32      regAddr; /* register address */
    GT_U32      value;   /* register field value */
    GT_U32      offset;  /* field offset */
    PRV_CPSS_PORT_TYPE_ENT portMacType;

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);
    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    portMacType = PRV_CPSS_PX_PORT_TYPE_MAC(devNum, portMacNum);

    if(portMacType < PRV_CPSS_PORT_XG_E)
    {
        PRV_CPSS_PX_PORT_MAC_CTRL2_REG_MAC(devNum, portMacNum, portMacType, &regAddr);
        offset = 5;
    }
    else if(portMacType < PRV_CPSS_PORT_CG_E)
    {
        PRV_CPSS_PX_PORT_MAC_CTRL_REG_MAC(devNum,portMacNum,portMacType,&regAddr);
        offset = 13;
    }
    else
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }
    if(PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);

    rc = prvCpssDrvHwPpPortGroupGetRegField(CAST_SW_DEVNUM(devNum),
                                            PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portMacNum),
                                            regAddr, offset, 1, &value);

    if(rc != GT_OK)
    {
        return rc;
    }

    *enablePtr = (value == 0) ? GT_TRUE : GT_FALSE;
    return GT_OK;
}

/**
* @internal cpssPxPortPaddingEnableGet function
* @endinternal
*
* @brief   Gets padding status of transmitted packets shorter than 64B.
*
* @note   APPLICABLE DEVICES:      pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number (or CPU port).
*
* @param[out] enablePtr                - pointer to packet padding status.
*                                      - GT_TRUE  - Pad short packet in Tx.
*                                      - GT_FALSE - No padding in short packets.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
GT_STATUS cpssPxPortPaddingEnableGet
(
    IN  GT_SW_DEV_NUM        devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_BOOL              *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortPaddingEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enablePtr));

    rc = internal_cpssPxPortPaddingEnableGet(devNum, portNum, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssPxPortCtleBiasOverrideEnableSet function
* @endinternal
*
* @brief   Set the override mode and the value of the CTLE Bias parameter per port.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - Device Number.
* @param[in] portNum                  - port Number.
* @param[in] overrideEnable           - override the CTLE default value
* @param[in] ctleBiasValue            - value of Ctle Bias [0..1]
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad devNum
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPortCtleBiasOverrideEnableSet
(
    IN  GT_U8                              devNum,
    IN  GT_PHYSICAL_PORT_NUM               portNum,
    IN  GT_BOOL                            overrideEnable,
    IN  CPSS_PX_PORT_CTLE_BIAS_MODE_ENT  ctleBiasValue
)
{
   GT_U32  portMacNum;      /* MAC number */
   GT_STATUS rc;

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);

    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

    rc = mvHwsPortCtleBiasOverrideEnableSet(devNum, portMacNum, (GT_U32)overrideEnable,(GT_U32)ctleBiasValue);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "mvHwsPortCtleBiasOverrideEnableSet : Hws Failed");
    }

    return GT_OK;

}

/**
* @internal cpssPxPortCtleBiasOverrideEnableSet function
* @endinternal
*
* @brief   Set the override mode and the value of the CTLE Bias parameter per port.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - Device Number.
* @param[in] portNum                  - port Number.
* @param[in] overrideEnable           - override the CTLE default value
* @param[in] ctleBiasValue            - value of Ctle Bias [0..1]
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad devNum
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortCtleBiasOverrideEnableSet
(
    IN  GT_U8                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                portNum,
    IN  GT_BOOL                             overrideEnable,
    IN  CPSS_PX_PORT_CTLE_BIAS_MODE_ENT     ctleBiasValue
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortCtleBiasOverrideEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, overrideEnable, ctleBiasValue));

    rc = internal_cpssPxPortCtleBiasOverrideEnableSet(devNum, portNum, overrideEnable, ctleBiasValue);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, overrideEnable, clteBiasValue));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;

}

/**
* @internal internal_cpssPxPortCtleBiasOverrideEnableGet function
* @endinternal
*
* @brief   Set the override mode and the value of the CTLE Bias parameter per port.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - Device Number.
* @param[in] portNum                  - port Number.
*
* @param[out] overrideEnablePtr        - pointer to override mode
* @param[out] ctleBiasValuePtr         - pointer to value of Ctle Bias [0..1]
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad devNum
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPortCtleBiasOverrideEnableGet
(
    IN  GT_SW_DEV_NUM                         devNum,
    IN  GT_PHYSICAL_PORT_NUM                  portNum,
    IN  GT_BOOL                               *overrideEnablePtr,
    IN  CPSS_PX_PORT_CTLE_BIAS_MODE_ENT      *ctleBiasValuePtr
)
{
   GT_U32  portMacNum;      /* MAC number */
   GT_STATUS rc;

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

    rc = mvHwsPortCtleBiasOverrideEnableGet(devNum, portMacNum, (GT_U32*)overrideEnablePtr,(GT_U32*)ctleBiasValuePtr);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "mvHwsPortCtleBiasOverrideEnableGet : Hws Failed");
    }

    return GT_OK;
}

/**
* @internal cpssPxPortCtleBiasOverrideEnableGet function
* @endinternal
*
* @brief   Set the override mode and the value of the CTLE Bias parameter per port.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - Device Number.
* @param[in] portNum                  - port Number.
*
* @param[out] overrideEnablePtr        - pointer to override mode
* @param[out] ctleBiasValuePtr         - pointer to value of Ctle Bias [0..1]
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad devNum
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortCtleBiasOverrideEnableGet
(
    IN  GT_SW_DEV_NUM                       devNum,
    IN  GT_PHYSICAL_PORT_NUM                portNum,
    IN  GT_BOOL                             *overrideEnablePtr,
    IN  CPSS_PX_PORT_CTLE_BIAS_MODE_ENT     *ctleBiasValuePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortCtleBiasOverrideEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, overrideEnablePtr, ctleBiasValuePtr));

    rc = internal_cpssPxPortCtleBiasOverrideEnableGet(devNum, portNum, overrideEnablePtr, ctleBiasValuePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, overrideEnablePtr, clteBiasValuePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;

}

/**
* @internal internal_cpssPxPortVosOverrideControlModeSet function
* @endinternal
*
* @brief   Set the override mode of the VOS parameters for all ports. If the override mode
*         is set to true, the VOS parameters will be overriden.
*
* @note   APPLICABLE DEVICES:      pipe
* @param[in] devNum                   - Device Number
* @param[in] vosOverride              - GT_TRUE means to override the VOS parameters for the device, GT_FALSE otherwisw.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad devNum
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPortVosOverrideControlModeSet
(
    IN  GT_SW_DEV_NUM   devNum,
    IN  GT_BOOL         vosOverride
)
{

    GT_STATUS   rc = GT_OK;

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);

    CPSS_LOG_INFORMATION_MAC("Calling: mvHwsPortVosOverrideControlModeSet(devNum[%d], vosOverride[%d])", devNum, vosOverride);
    rc = mvHwsPortVosOverrideControlModeSet(devNum, 0, vosOverride);

    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "hwsSerdesVosOverrideControlModeSet : Hws Failed");
    }

    return rc;
}

/**
* @internal cpssPxPortVosOverrideControlModeSet function
* @endinternal
*
* @brief   Set the override mode of the VOS parameters for all ports. If the override mode
*         is set to true, the VOS parameters will be overriden.
*
* @note   APPLICABLE DEVICES:      pipe
* @param[in] devNum                   - Device Number
* @param[in] vosOverride              - GT_TRUE means to override the VOS parameters for the device, GT_FALSE otherwisw.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad devNum
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortVosOverrideControlModeSet
(
    IN  GT_SW_DEV_NUM   devNum,
    IN  GT_BOOL         vosOverride
)
{
    GT_STATUS rc = GT_OK;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortVosOverrideControlModeSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, vosOverride));

    rc = internal_cpssPxPortVosOverrideControlModeSet(devNum, vosOverride);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, vosOverride));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortVosOverrideControlModeGet function
* @endinternal
*
* @brief   Get the override mode of the VOS parameters for all ports.
*
* @note   APPLICABLE DEVICES:      pipe
* @param[in] devNum                   - Device Number
*
* @param[out] vosOverridePtr           - (pointer to) current VOS override mode.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad devNum
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - if vosOverride is NULL pointer
*/
static GT_STATUS internal_cpssPxPortVosOverrideControlModeGet
(
    IN   GT_SW_DEV_NUM  devNum,
    OUT  GT_BOOL        *vosOverridePtr
)
{
    GT_STATUS        rc = GT_OK;

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(vosOverridePtr);

    CPSS_LOG_INFORMATION_MAC("Calling: mvHwsPortVosOverrideControlModeGet(devNum[%d])", devNum);
    rc = mvHwsPortVosOverrideControlModeGet(devNum, vosOverridePtr);

    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "hwsSerdesVosOverrideControlModeGet : Hws Failed");
    }
    return rc;
}

/**
* @internal cpssPxPortVosOverrideControlModeGet function
* @endinternal
*
* @brief   Get the override mode of the VOS parameters for all ports.
*
* @note   APPLICABLE DEVICES:      pipe
* @param[in] devNum                   - Device Number
*
* @param[out] vosOverridePtr           - (pointer to) current VOS override mode.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad devNum
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - if vosOverride is NULL pointer
*/
GT_STATUS cpssPxPortVosOverrideControlModeGet
(
    IN   GT_SW_DEV_NUM  devNum,
    OUT  GT_BOOL        *vosOverridePtr
)
{
    GT_STATUS rc = GT_OK;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortVosOverrideControlModeGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, vosOverridePtr));

    rc = internal_cpssPxPortVosOverrideControlModeGet(devNum, vosOverridePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, vosOverridePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortXlgReduceAverageIPGSet function
* @endinternal
*
* @brief   Configure Reduce Average IPG in XLG MAC.
*
* @note   APPLICABLE DEVICES:      pipe.
* @param[in] devNum                   - physical device number.
* @param[in] portNum                  - physical port number.
* @param[in] value                    -  to set to the XLG MAC DIC_PPM_ IPG_Reduce Register (APPLICABLE RANGES: 0..0xFFFF)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
* @retval GT_HW_ERROR              - on hardware error
*                                       GT_BAD_PARAM ? on bad parameters
*/
GT_STATUS internal_cpssPxPortXlgReduceAverageIPGSet
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  GT_U32                          value
)
{
    GT_STATUS rc = GT_OK; /* return code */
    GT_U32    portMacNum; /* port Mac number */
    GT_U32    regAddr;    /* register address */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    if(value > 0xFFFF)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);
    regAddr = PRV_CPSS_PX_DEV_REGS_VER1_MAC(devNum)->GOP.perPortRegs[portMacNum].xlgDicPpmIpgReduce;
    if(regAddr == PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
         CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    rc = prvCpssHwPpSetRegField(CAST_SW_DEVNUM(devNum), regAddr, 0, 16, value);

    return rc;
}

/**
* @internal cpssPxPortXlgReduceAverageIPGSet function
* @endinternal
*
* @brief   Configure Reduce Average IPG in XLG MAC.
*
* @note   APPLICABLE DEVICES:      pipe.
* @param[in] devNum                   - physical device number.
* @param[in] portNum                  - physical port number.
* @param[in] value                    -  to set to the XLG MAC DIC_PPM_ IPG_Reduce Register (APPLICABLE RANGES: 0..0xFFFF)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
* @retval GT_HW_ERROR              - on hardware error
*                                       GT_BAD_PARAM ? on bad parameters
*/
GT_STATUS cpssPxPortXlgReduceAverageIPGSet
(
    IN  GT_SW_DEV_NUM                   devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  GT_U32                          value
)
{
    GT_STATUS rc = GT_OK;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortXlgReduceAverageIPGSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, value));

    rc = internal_cpssPxPortXlgReduceAverageIPGSet(devNum, portNum, value);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, value));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortXlgReduceAverageIPGGet function
* @endinternal
*
* @brief   Get Reduce Average IPG value in XLG MAC.
*
* @note   APPLICABLE DEVICES:      pipe.
* @param[in] devNum                   - physical device number.
* @param[in] portNum                  - physical port number.
*
* @param[out] valuePtr                 -  pointer to value ? content of the XLG MAC DIC_PPM_IPG_Reduce register
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
* @retval GT_HW_ERROR              - on hardware error
*                                       GT_BAD_PARAM ? on bad parameters
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS internal_cpssPxPortXlgReduceAverageIPGGet
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_U32                 *valuePtr
)
{
    GT_STATUS rc = GT_OK; /* return code */
    GT_U32    portMacNum; /* port Mac number */
    GT_U32    regAddr;    /* register address */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(valuePtr);
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);
    regAddr = PRV_CPSS_PX_DEV_REGS_VER1_MAC(devNum)->GOP.perPortRegs[portMacNum].xlgDicPpmIpgReduce;
    if(regAddr == PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
         CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    rc = prvCpssHwPpGetRegField(CAST_SW_DEVNUM(devNum), regAddr, 0, 16, valuePtr);

    return rc;
}

/**
* @internal cpssPxPortXlgReduceAverageIPGGet function
* @endinternal
*
* @brief   Get Reduce Average IPG value in XLG MAC.
*
* @note   APPLICABLE DEVICES:      pipe.
* @param[in] devNum                   - physical device number.
* @param[in] portNum                  - physical port number.
*
* @param[out] valuePtr                 -  pointer to value ? content of the XLG MAC DIC_PPM_IPG_Reduce register
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
* @retval GT_HW_ERROR              - on hardware error
*                                       GT_BAD_PARAM ? on bad parameters
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS cpssPxPortXlgReduceAverageIPGGet
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_U32                 *valuePtr
)
{
    GT_STATUS rc = GT_OK;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortXlgReduceAverageIPGGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, valuePtr));

    rc = internal_cpssPxPortXlgReduceAverageIPGGet(devNum, portNum, valuePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, valuePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssPxPortCrcNumBytesSet function
* @endinternal
*
* @brief   Set CRC num of bytes
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  none
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - port number
* @param[in] portDirection            - TX/RX cascade port direction
* @param[in] numCrcBytes              - number of crc bytes
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPortCrcNumBytesSet
(
    IN GT_SW_DEV_NUM                     devNum,
    IN GT_PHYSICAL_PORT_NUM              portNum,
    IN CPSS_PORT_DIRECTION_ENT           portDirection,
    IN GT_U32                            numCrcBytes
)
{
    GT_STATUS status;               /* return status */
    GT_U32 hwIvalidCrcMode;         /* invalid CRC mode value */
    GT_U32 rxDmaRecalcCrc = 0;      /* RxDMA CRC recalculation configuration */
    GT_U32 regAddr;                 /* register address */
    GT_U32 portMacNum;              /* MAC number */
    GT_U32 portRxDmaNum;

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);

    status = prvCpssPxPortMacCrcModeSet(devNum, portNum, portDirection, numCrcBytes);
    if (status != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(status, LOG_ERROR_NO_MSG);
    }
    if (numCrcBytes > 4) {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_RXDMA_NUM_GET_MAC(devNum, portNum, portRxDmaNum);

    rxDmaRecalcCrc = (numCrcBytes < 2) ? 1 : 0;
    hwIvalidCrcMode = (numCrcBytes == 4) ? 0 : 1;
    if (portDirection != CPSS_PORT_DIRECTION_TX_E)
    {
        regAddr = PRV_CPSS_PX_DEV_REGS_VER1_MAC(devNum)->PHA.pha_regs.portInvalidCRCMode;
        status = prvCpssDrvHwPpSetRegField(CAST_SW_DEVNUM(devNum), regAddr, portNum, 1, hwIvalidCrcMode);
        if(GT_OK != status)
        {
            return status;
        }

        regAddr = PRV_CPSS_PX_DEV_REGS_VER1_MAC(devNum)->rxDMA.singleChannelDMAConfigs.SCDMAConfig0[portRxDmaNum];
        status = prvCpssDrvHwPpSetRegField(CAST_SW_DEVNUM(devNum), regAddr, 4, 1, rxDmaRecalcCrc);
        if(GT_OK != status)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(status, LOG_ERROR_NO_MSG);
        }
    }
    return GT_OK;
}

/**
* @internal cpssPxPortCrcNumBytesSet function
* @endinternal
*
* @brief   Set CRC num of bytes .
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  none.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - port number
* @param[in] portDirection            - TX/RX cascade port direction
* @param[in] numCrcBytes              - number of crc bytes
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortCrcNumBytesSet
(
    IN GT_SW_DEV_NUM                     devNum,
    IN GT_PHYSICAL_PORT_NUM              portNum,
    IN CPSS_PORT_DIRECTION_ENT           portDirection,
    IN GT_U32                            numCrcBytes
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortCrcNumBytesSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, portDirection, numCrcBytes));

    rc = internal_cpssPxPortCrcNumBytesSet(devNum, portNum, portDirection, numCrcBytes);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, portDirection, numCrcBytes));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortCrcNumBytesGet function
* @endinternal
*
* @brief   Get CRC num of bytes
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  none.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - port number
* @param[in] portDirection            - TX/RX cascade port direction (ingress or egress)
*
* @param[out] numCrcBytesPtr           -  CRC bytes
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL ptr
*/
static GT_STATUS internal_cpssPxPortCrcNumBytesGet
(
    IN  GT_SW_DEV_NUM                     devNum,
    IN  GT_PHYSICAL_PORT_NUM              portNum,
    IN  CPSS_PORT_DIRECTION_ENT           portDirection,
    OUT GT_U32                            *numCrcBytesPtr
)
{
    GT_STATUS rc;         /* return code */
    GT_U32    portMacNum; /* MAC number */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);
    CPSS_NULL_PTR_CHECK_MAC(numCrcBytesPtr);

    if (portDirection != CPSS_PORT_DIRECTION_RX_E && portDirection != CPSS_PORT_DIRECTION_TX_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    rc = prvCpssPxPortMacCrcModeGet(devNum,portNum,portDirection,/*OUT*/numCrcBytesPtr);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    return GT_OK;
}

/**
* @internal cpssPxPortCrcNumBytesGet function
* @endinternal
*
* @brief   Get CRC num of bytes.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  none.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - port number
* @param[in] portDirection            - TX/RX cascade port direction (ingress or egress)
*
* @param[out] numCrcBytesPtr           - num of crc bytes
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL ptr
*/
GT_STATUS cpssPxPortCrcNumBytesGet
(
    IN  GT_SW_DEV_NUM                     devNum,
    IN  GT_PHYSICAL_PORT_NUM              portNum,
    IN  CPSS_PORT_DIRECTION_ENT           portDirection,
    OUT GT_U32                            *numCrcBytesPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortCrcNumBytesGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, portDirection, numCrcBytesPtr));

    rc = internal_cpssPxPortCrcNumBytesGet(devNum, portNum, portDirection, numCrcBytesPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, portDirection, numCrcBytesPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal prvCpssPxPortLkbSet function
* @endinternal
*
* @brief   Sets Link Binding on port
*
* @note   APPLICABLE DEVICES:      Pipe.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] portMode                 - HWS port mode
* @param[in] pairPortNum              - physical pair port number
* @param[in] enable                   - GT_TRUE for enable, GT_FALSE for disable
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - on not supported feature for a port
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssPxPortLkbSet
(
    IN  GT_SW_DEV_NUM        devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  MV_HWS_PORT_STANDARD portMode,
    IN  GT_PHYSICAL_PORT_NUM pairPortNum,
    IN  GT_BOOL              enable
)
{
    GT_U32 portMacNum, portGroupId;

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);
    portGroupId = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portMacNum);

    if(mvHwsPortLkbPortSet(devNum, portGroupId, (GT_U16)portNum, portMode, (GT_U16)pairPortNum, enable) != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    return GT_OK;
}

/**
* @internal internal_cpssPxFcFecCounterGet function
* @endinternal
*
* @brief   Return the FC-FEC counters
*
* @note   APPLICABLE DEVICES:      Pipe.
*
* @param[in] devNum                   - system device number
* @param[in] portNum                  - port number
*
* @param[out] fcfecCountersPtr        - pointer to struct that
*                                       will contain the FC-FEC
*                                       counters
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS internal_cpssPxFcFecCounterGet
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    OUT CPSS_FCFEC_COUNTERS_STC        *fcfecCountersPtr
)
{
    GT_U32                      portMacNum;
    GT_STATUS                   rc;
    GT_U32                      portGroup;
    MV_HWS_FCFEC_COUNTERS_STC   result;

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum,portMacNum);
    CPSS_NULL_PTR_CHECK_MAC(fcfecCountersPtr);

    cpssOsMemSet(&result, 0, sizeof(result));

    portGroup = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portMacNum);

    rc = mvHwsMMPcs28nmFcFecCorrectedError(devNum,portGroup,portMacNum,&result);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "calling mvHwsMMPcs28nmFcFecCorrectedError failed=%d");
    }

    fcfecCountersPtr->numReceivedBlocks = result.numReceivedBlocks;
    fcfecCountersPtr->numReceivedBlocksNoError = result.numReceivedBlocksNoError;
    fcfecCountersPtr->numReceivedBlocksCorrectedError = result.numReceivedBlocksCorrectedError;
    fcfecCountersPtr->numReceivedBlocksUncorrectedError = result.numReceivedBlocksUncorrectedError;
    fcfecCountersPtr->numReceivedCorrectedErrorBits = result.numReceivedCorrectedErrorBits;
    fcfecCountersPtr->numReceivedUncorrectedErrorBits = result.numReceivedUncorrectedErrorBits;

    cpssOsMemCpy(fcfecCountersPtr->blocksCorrectedError,result.blocksCorrectedError,  sizeof(fcfecCountersPtr->blocksCorrectedError));
    cpssOsMemCpy(fcfecCountersPtr->blocksUncorrectedError,result.blocksUncorrectedError,  sizeof(fcfecCountersPtr->blocksUncorrectedError));
    return GT_OK;
}

/**
* @internal cpssPxFcFecCounterGet function
* @endinternal
*
* @brief   Return the FC-FEC counters
*
* @note   APPLICABLE DEVICES:      Pipe.
*
* @param[in] devNum                   - system device number
* @param[in] portNum                  - port number
*
* @param[out] fcfecCountersPtr        - pointer to struct that
*                                       will contain the FC-FEC
*                                       counters
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS cpssPxFcFecCounterGet
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    OUT CPSS_FCFEC_COUNTERS_STC        *fcfecCountersPtr
)
{
    GT_STATUS rc = GT_OK;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxFcFecCounterGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, fcfecCountersPtr));

    rc = internal_cpssPxFcFecCounterGet(devNum, portNum, fcfecCountersPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, fcfecCountersPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxRsFecCounterGet function
* @endinternal
*
* @brief   Return the RS-FEC counters
*
* @note   APPLICABLE DEVICES:      Pipe.
*
* @param[in] devNum                   - system device number
* @param[in] portNum                  - port number
*
* @param[out] rsfecCountersPtr        - pointer to struct that
*                                       will contain the RS-FEC
*                                       counters
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS internal_cpssPxRsFecCounterGet
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    OUT CPSS_RSFEC_COUNTERS_STC       *rsfecCountersPtr
)
{
    GT_U32                      portMacNum;
    GT_STATUS                   rc;
    GT_U32                      portGroup;
    MV_HWS_RSFEC_COUNTERS_STC   result;

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum,portMacNum);
    CPSS_NULL_PTR_CHECK_MAC(rsfecCountersPtr);

    cpssOsMemSet(&result, 0, sizeof(result));

    portGroup = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portMacNum);

    rc = mvHwsRsFecCorrectedError(devNum,portGroup,portMacNum,&result);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "calling mvHwsRsFecCorrectedError failed=%d");
    }

    rsfecCountersPtr->correctedFecCodeword = result.correctedFecCodeword;
    rsfecCountersPtr->uncorrectedFecCodeword = result.uncorrectedFecCodeword;

    cpssOsMemCpy(rsfecCountersPtr->symbolError,result.symbolError,  sizeof(rsfecCountersPtr->symbolError));
    return GT_OK;
}

/**
* @internal cpssPxRsFecCounterGet function
* @endinternal
*
* @brief   Return the RS-FEC counters
*
* @note   APPLICABLE DEVICES:      Pipe.
*
* @param[in] devNum                   - system device number
* @param[in] portNum                  - port number
*
* @param[out] rsfecCountersPtr        - pointer to struct that
*                                       will contain the RS-FEC
*                                       counters
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS cpssPxRsFecCounterGet
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    OUT CPSS_RSFEC_COUNTERS_STC        *rsfecCountersPtr
)
{
    GT_STATUS rc = GT_OK;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxRsFecCounterGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, rsfecCountersPtr));

    rc = internal_cpssPxRsFecCounterGet(devNum, portNum, rsfecCountersPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, rsfecCountersPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}
