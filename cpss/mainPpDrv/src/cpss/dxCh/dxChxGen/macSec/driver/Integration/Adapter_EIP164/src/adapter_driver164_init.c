/* adapter_driver164_init.c
 *
 * Adapter top level module, SafeXcel-IP-164 driver's entry point.
 */

/*****************************************************************************
* Copyright (c) 2015-2017 INSIDE Secure B.V. All Rights Reserved.
*
* This confidential and proprietary software may be used only as authorized
* by a licensing agreement from INSIDE Secure.
*
* The entire notice above must be reproduced on all authorized copies that
* may only be made to the extent permitted by a licensing agreement from
* INSIDE Secure.
*
* For more information or support, please go to our online support system at
* https://customersupport.insidesecure.com.
* In case you do not have an account for this system, please send an e-mail
* to ESSEmbeddedHW-Support@insidesecure.com.
*****************************************************************************/

/*----------------------------------------------------------------------------
 * This module implements (provides) the following interface(s):
 */


/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

/* Top-level Adapter configuration */
#include <Kit/DriverFramework/incl/cs_adapter.h>             /* ADAPTER_DRIVER_NAME */

/* Adapter Initialization API */
#include <Integration/Adapter_EIP164/incl/adapter_init.h>           /* Adapter_* */

/* Logging API */
#include <Kit/Log/incl/log.h>       /* LOG_INFO */

/* Driver Framework C Run-time Library API */
#include <Kit/DriverFramework/incl/clib.h> 

/* Driver Framework Device API */
#include <Kit/DriverFramework/incl/device_mgmt.h>

/* WarmBoot init API */
#include <Integration/Adapter_EIP164/incl/adapter_init_warmboot.h>

/* Adapter init support API. */
#include <Integration/Adapter_EIP164/incl/adapter_init_support.h>

/*----------------------------------------------------------------------------
 * Driver164_Init
 */
GT_32
Driver164_Init(GT_U8 devNum, GT_U32 unitBmp)
{

    Adapter_Report_Build_Params();

    if (!Adapter_Init(devNum, unitBmp))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }
#ifdef ADAPTER_WARMBOOT_ENABLE
    if (!Adapter_Init_WarmBoot_Init())
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }
#endif

    return GT_OK;   /* success */
}


/*----------------------------------------------------------------------------
 * Driver164_Exit
 */
void
Driver164_Exit(GT_U8 devNum, GT_U32 unitBmp, GT_BOOL lastDevice)
{
#ifdef ADAPTER_WARMBOOT_ENABLE
    Adapter_Init_WarmBoot_Uninit();
#endif
    Adapter_UnInit(devNum, unitBmp, lastDevice);
}


/*----------------------------------------------------------------------------
 * Driver164_Device_Add
 */
int
Driver164_Device_Add(
        const unsigned int Index,
        const Driver164_Device_t * const Device_p)
{
    int Rc;
    Device_Properties_t Props;

    LOG_INFO("\n\t %s \n", __func__);

    ZEROINIT(Props);

    Props.Flags             = Device_p->Flags;
    Props.Name_p            = Device_p->DeviceName_p;
    Props.StartByteOffset   = Device_p->StartByteOffset;
    Props.LastByteOffset    = Device_p->LastByteOffset;

    Rc = Device_Add(Index, &Props);

#ifdef ADAPTER_WARMBOOT_ENABLE
    if (Rc == 0)
    {
        Adapter_Init_WarmBoot_Device_Update(Index, &Props);
    }
#endif

    LOG_INFO("\n\t %s done \n", __func__);

    return Rc;
}


/*----------------------------------------------------------------------------
 * Driver164_Device_Remove
 */
int
Driver164_Device_Remove(
        const unsigned int Index)
{
    int Rc;

    LOG_INFO("\n\t %s \n", __func__);

    Rc = Device_Remove(Index);

    LOG_INFO("\n\t %s done \n", __func__);

#ifdef ADAPTER_WARMBOOT_ENABLE
    if (Rc == 0)
    {
        Adapter_Init_WarmBoot_Device_Update(Index, NULL);
    }
#endif

    return Rc;
}


/*-----------------------------------------------------------------------------
 * Driver164_Device_GetCount
 */
unsigned int
Driver164_Device_GetCount(void)
{
    unsigned int Count;

    LOG_INFO("\n\t %s \n", __func__);

    Count = Device_GetCount();

    LOG_INFO("\n\t %s done \n", __func__);

    return Count;
}


/*----------------------------------------------------------------------------
 * Driver164_DataPath_Add
 */
int
Driver164_DataPath_Add(
        const unsigned int Index,
        const Driver164_DataPath_t * const DataPath_p)
{
    int Rc;

    LOG_INFO("\n\t %s \n", __func__);

    if (Index >= ADAPTER_EIP164_MAX_NOF_DEVICES)
        return -1;

    Rc = Adapter_Init_DataPath_Add(Index, DataPath_p);

#ifdef ADAPTER_WARMBOOT_ENABLE
    if (Rc == 0)
    {
        Adapter_Init_WarmBoot_DataPath_Update(Index, DataPath_p);
    }
#endif

    LOG_INFO("\n\t %s done \n", __func__);

    return Rc;
}


/*----------------------------------------------------------------------------
 * Driver164_DataPath_Remove
 */
int
Driver164_DataPath_Remove(
        const unsigned int Index)
{
    int Rc;

    LOG_INFO("\n\t %s \n", __func__);

    if (Index >= ADAPTER_EIP164_MAX_NOF_DEVICES)
        return -1;

    Rc = Adapter_Init_DataPath_Remove(Index);

    LOG_INFO("\n\t %s done \n", __func__);

#ifdef ADAPTER_WARMBOOT_ENABLE
    if (Rc == 0)
    {
        Adapter_Init_WarmBoot_DataPath_Update(Index, NULL);
    }
#endif

    return Rc;
}


/*-----------------------------------------------------------------------------
 * Driver164_DataPath_GetCount
 */
unsigned int
Driver164_DataPath_GetCount(void)
{
    return ADAPTER_EIP164_MAX_NOF_DEVICES;
}




/* end of file adapter_driver164_init.c */
