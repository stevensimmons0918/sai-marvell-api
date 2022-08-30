/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
*******************************************************************************/

/**
********************************************************************************
* @file cpssHalPlatform.c
*
* @version   01
********************************************************************************
*/

#include "cpssHalPlatform.h"
#include "prvCpssBindFunc.h"
#include "cpssHalUtil.h"
#include "gtGenTypes.h"
#include "cpssHalDevice.h"
#include "xpsCommon.h"
#include "cpssDriver/pp/hardware/cpssDriverPpHw.h"
#include "cpssHalSysKern.h"
#define GPIO_DATA_OUT_ENABLE_REG  0x18104
#define GPIO_DATA_OUT_REG  0x18100
#define GPIO25 25
#define GPIO26 26
#define GPIO21 21
#define GPIO22 22
#define SET_BIT(p,n) (p |= 1UL << n)
#define CLR_BIT(p,n) (p &= ~(1UL << n))

GT_STATUS cpssHalGpioRegisterWrite(GT_U8 devNum, GT_U8 gpioPinPosition,
                                   GT_BOOL set)
{
    GT_STATUS cpssRet;
    GT_U32 data;
    //enable data out
    cpssRet = cpssDrvPpHwInternalPciRegRead(devNum,
                                            CPSS_PORT_GROUP_UNAWARE_MODE_CNS, GPIO_DATA_OUT_ENABLE_REG, &data);
    if (cpssRet != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "cpssDrvPpHwInternalPciRegRead dev %d failed(%d)", devNum, cpssRet);
        return cpssRet;
    }
    CLR_BIT(data, gpioPinPosition);
    cpssRet = cpssDrvPpHwInternalPciRegWrite(devNum,
                                             CPSS_PORT_GROUP_UNAWARE_MODE_CNS, GPIO_DATA_OUT_ENABLE_REG, data);
    if (cpssRet != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "cpssDrvPpHwInternalPciRegWrite dev %d  failed(%d)", devNum, cpssRet);
        return cpssRet;
    }
    //write data.
    cpssRet = cpssDrvPpHwInternalPciRegRead(devNum,
                                            CPSS_PORT_GROUP_UNAWARE_MODE_CNS, GPIO_DATA_OUT_REG, &data);
    if (cpssRet != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "cpssDrvPpHwInternalPciRegRead dev %d failed(%d)", devNum, cpssRet);
        return cpssRet;
    }

    set ? (SET_BIT(data, gpioPinPosition)) : (CLR_BIT(data, gpioPinPosition));
    cpssRet = cpssDrvPpHwInternalPciRegWrite(devNum,
                                             CPSS_PORT_GROUP_UNAWARE_MODE_CNS, GPIO_DATA_OUT_REG, data);
    if (cpssRet != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "cpssDrvPpHwInternalPciRegWrite dev %d  failed(%d)", devNum, cpssRet);
        return cpssRet;
    }

    return GT_OK;
}


GT_STATUS cpssHalPlatformInit(GT_U8 devNum, XP_DEV_TYPE_T devType)
{
    GT_STATUS rc;
    if (devType==ALDB2B && devNum == 0)
    {
#if 0
        //enable fan led
        rc = cpssHalGpioRegisterWrite(devNum,  GPIO25, GT_TRUE);//set GPIO25 to 1
        if (rc != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_TRACE,
                  "cpssDrvPpHwInternalPciRegWrite  failed(%d)", rc);
        }
        rc =  cpssHalGpioRegisterWrite(devNum,  GPIO26, GT_FALSE);//set GPIO26 to 0
        if (rc != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_TRACE,
                  "cpssDrvPpHwInternalPciRegWrite  failed(%d)", rc);
        }
        //enable PSU led
        rc = cpssHalGpioRegisterWrite(devNum,  GPIO22, GT_TRUE);//set GPIO22 to 1
        if (rc != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_TRACE,
                  "cpssDrvPpHwInternalPciRegWrite  failed(%d)", rc);
        }
        rc =  cpssHalGpioRegisterWrite(devNum,  GPIO21, GT_FALSE);//set GPIO21 to 0
        if (rc != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_TRACE,
                  "cpssDrvPpHwInternalPciRegWrite  failed(%d)", rc);
        }
#else
        // Force remove  if loaded
        rc = remove_kernel_module(PLATFORM_GPIO_MODNAME);

        rc = load_kernel_module(PLATFORM_GPIO_DRIVER, "");
        if (rc != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "load_kernel_module %s failed(%d)", PLATFORM_GPIO_DRIVER, rc);
        }
        else
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_TRACE,
                  "load_kernel_module %s success(%d)", PLATFORM_GPIO_DRIVER, rc);
        }
#endif
    }
    return GT_OK;

}


GT_STATUS cpssHalPlatformDeInit(GT_U8 cpssDevNum)
{
    GT_STATUS rc;
    if (cpssDevNum == 0)
    {
        rc = remove_kernel_module(PLATFORM_GPIO_MODNAME);
        if (rc != GT_OK)
        {
            printf("remove_kernel_module %s failed(%d)", PLATFORM_GPIO_MODNAME, rc);
        }
        else
        {
            printf("remove_kernel_module %s success(%d)", PLATFORM_GPIO_MODNAME, rc);
        }
    }
    return GT_OK;
}
