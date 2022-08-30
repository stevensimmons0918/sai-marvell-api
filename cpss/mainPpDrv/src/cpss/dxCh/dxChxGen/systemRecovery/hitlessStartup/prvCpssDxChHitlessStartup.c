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
* @file prvCpssDxChHitlessStartup.c
*
* @brief private CPSS DxCh hitless startup facility API.
*
* @version   1
********************************************************************************
*/

#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/systemRecovery/hitlessStartup/private/prvCpssDxChHitlessStartup.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>
#include <cpss/dxCh/dxChxGen/bootChannel/private/prvCpssDxChBootChannelHandler.h>

#define PRV_SERVICE_CPU_GLOBAL_CONFIGURATION_OFFSET 0x500 /*offset is in bytes*/

extern GT_U32 prvCpssDrPpConInitMg0UnitBaseAddressGet
(
     IN GT_U8      devNum
);

/*set value to init state register */
GT_STATUS prvHitlessStartupInitStateSet
(
    GT_U8 devNum,
    GT_U32 value
)
{
    GT_STATUS rc ;
    GT_U32 regAddr;

    regAddr = PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->DFXServerUnitsDeviceSpecificRegs.deviceCtrl2;
    rc = prvCpssDrvHwPpResetAndInitControllerWriteReg(devNum,regAddr,value);
    if (rc != GT_OK)
    {
        return rc ;
    }
    return GT_OK ;
}

/*get value of init state register*/
GT_STATUS prvHitlessStartupInitStateGet
(
    GT_U8 devNum,
    GT_U32 *valuePtr
)
{
    GT_STATUS rc ;
    GT_U32 regAddr ;
    regAddr = PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->DFXServerUnitsDeviceSpecificRegs.deviceCtrl2;
    rc = prvCpssDrvHwPpResetAndInitControllerReadReg(devNum,regAddr,valuePtr);
    if (rc != GT_OK)
    {
        return rc ;
    }
    return GT_OK ;
}

/**
* @internal prvCpssDxChHitlessStartupStateCheckAndSet function
* @endinternal
*
* @brief   This function checks expected Init State register value and set new value
*/
GT_STATUS prvCpssDxChHitlessStartupStateCheckAndSet
(
    GT_U8 devNum,
    PRV_CPSS_HITLESS_STARTUP_SOFTWARE_INIT_STATE_ENT expValue,
    PRV_CPSS_HITLESS_STARTUP_SOFTWARE_INIT_STATE_ENT newValue
)
{
    GT_STATUS rc ;
    GT_U32 value = 0;

    if (expValue > PRV_CPSS_HITLESS_STARTUP_SOFTWARE_INIT_STATE_HS_DONE_E ||
        newValue > PRV_CPSS_HITLESS_STARTUP_SOFTWARE_INIT_STATE_HS_DONE_E ||
        newValue < expValue)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    rc = prvHitlessStartupInitStateGet(devNum, &value);
    if (rc != GT_OK)
    {
        return rc;
    }

    if (PRV_NON_SHARED_GLOBAL_VAR_SYSTEM_RECOVERY_DEBUG.hsSkipDfxInitStateCheck == GT_FALSE)
    {
        if (expValue != value)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
        }
    }
    else
    {
        cpssOsPrintf("\n skip HS DFX init State check - debug mode\n");
    }

    rc = prvHitlessStartupInitStateSet(devNum, newValue);
    if (rc != GT_OK)
    {
        return rc;
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChHitlessStartupPortConfiguredCheck function
* @endinternal
*
* @brief   This function checks if port configured by MI
*/
GT_STATUS prvCpssDxChHitlessStartupPortConfiguredCheck
(
    GT_U8                                 devNum,
    GT_PHYSICAL_PORT_NUM                  portNum,
    GT_BOOL                               *isConfigure
)
{
    GT_STATUS rc ;
    PRV_CPSS_BOOT_CH_PORT_STATUS_STC      portInfo;

    *isConfigure = GT_FALSE;
    rc = prvCpssDxChBootChannelHandlerPortStatusGet(devNum, portNum, &portInfo);
    if (rc != GT_OK)
    {
        /*in case of port not configured by MI GT_NOT_FOUND is returned */
        if (rc == GT_NOT_FOUND)
        {
            return GT_OK;
        }
        else
            return rc;
    }

    *isConfigure = GT_TRUE;
    return GT_OK;
}

/**
* @internal prvHitlessStartupMiClose function
* @endinternal
*
* @brief   This function close MI process after HS process finished
*/
GT_STATUS prvHitlessStartupMiClose
(
    GT_U8 devNum
)
{
    GT_STATUS rc ;
    GT_U32 regAddr , value, initStatus;

    regAddr = prvCpssDrPpConInitMg0UnitBaseAddressGet(devNum) + PRV_SERVICE_CPU_GLOBAL_CONFIGURATION_OFFSET ;
    rc = cpssDrvPpHwRegisterRead(devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,regAddr,&value);
    if (rc != GT_OK)
    {
        return rc ;
    }

    rc = prvHitlessStartupInitStateGet(devNum, &initStatus);
    if (rc != GT_OK)
    {
        return rc;
    }

    /*check that HS done and MI can be close */
    if (initStatus != PRV_CPSS_HITLESS_STARTUP_SOFTWARE_INIT_STATE_HS_DONE_E)
    {
        CPSS_LOG_INFORMATION_MAC("\nprvHitlessStartupMiClose return without reset ,initStatus != PRV_CPSS_HITLESS_STARTUP_SOFTWARE_INIT_STATE_HS_DONE_E");
        return GT_OK;
    }

    /*reset bits 28,29*/
    value = value & 0xCFFFFFFF;
    rc = cpssDrvPpHwRegisterWrite(devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,regAddr,value);
    if (rc != GT_OK)
    {
        return rc ;
    }

    CPSS_LOG_INFORMATION_MAC("\n close MI process");
    return GT_OK ;
}
