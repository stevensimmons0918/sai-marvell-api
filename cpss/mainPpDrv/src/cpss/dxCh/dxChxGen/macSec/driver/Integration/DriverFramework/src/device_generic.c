/* device_generic.c
 *
 * This is the generic Driver Framework v4 Device API implementation.
 */

/*****************************************************************************
* Copyright (c) 2017-2019 INSIDE Secure B.V. All Rights Reserved.
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

/* Driver Framework Device API */
#include <Kit/DriverFramework/incl/device_mgmt.h>            /* API to implement */


/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

/* Driver Framework Basic Defs API */
#include <Kit/DriverFramework/incl/basic_defs.h>             /* IDENTIFIER_NOT_USED, NULL */

/* Driver Framework Device Internal interface */
#include <Integration/DriverFramework/src/device_internal.h>        /* Device_Internal_* */


/*----------------------------------------------------------------------------
 * Definitions and macros
 */

/* Default configuration */
#include <Integration/DriverFramework/src/c_device_generic.h>

/* Logging API */
#include <Kit/Log/incl/log.h>

#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

/*----------------------------------------------------------------------------
 * Local variables
 */


/*-----------------------------------------------------------------------------
 * DeviceLib_Device_Exists
 *
 * Checks if a device with DeviceName_p is already present in the device list
 *
 */
static bool
DeviceLib_Device_Exists(const GT_CHAR * DeviceName_p)
{
    GT_U32 i = 0;
    GT_U32 DevCount = Device_Internal_Static_Count_Get();
    Device_Admin_t ** DevAdmin_pp = Device_Internal_Admin_Get();

    if (DeviceName_p == NULL)
        return false;

    while(DevAdmin_pp[i] && (i < DevCount))
    {
        if (cpssOsStrCmp(DevAdmin_pp[i]->DevName, DeviceName_p) == 0)
            return true;
        i++;
    }

    return false;
}



/*-----------------------------------------------------------------------------
 * device_mgmt API
 *
 * These functions support finding a device given its name.
 * A handle is returned that is needed in the device_rw API
 * to read or write the device
 */

/*-----------------------------------------------------------------------------
 * Device_Initialize
 */
GT_32
Device_Initialize(GT_U8 devNum, GT_U32 unitBmp)
{
    GT_BOOL didErr;      /* error indication */
    GT_STATUS rc;
    GT_U32 res;
    GT_U32 unitBaseAddr; /* sevice base address */
    const Device_Admin_Static_t * DevStatAdmin_p = Device_Internal_Admin_Static_Get();
    Device_Admin_t ** DevAdmin_pp = Device_Internal_Admin_Get();
    Device_Global_Admin_t * DevGlobalAdmin_p = Device_Internal_Admin_Global_Get();
    GT_U32 devStart; /* Device specific start entry to fill MACSec units DB */
    GT_U32 devEnd;   /* Device specific end entry covering up to 8 MACSec units */

    /* DB tables hold information for up to 8 MACSec units per device.
       First 4 units represent DP0 and last 4 units represent DP1.
       Need to check if device is single DP or dual DP and initialize DB tables accordingly */
    if (PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.macSecInfo.macSecDpBmp == 3)
    {
        /* Dual DP therefore need to initialize up to 8 MACSec units of DP0 and DP1
           Three options:
           unitBmp:0x1 => only DP0, threfore need to initialize only first 4 MACSec units per device
           unitBmp:0x2 => only DP1, threfore need to initialize only last 4 MACSec units per device
           unitBmp:0x3 => DP0 & DP1, threfore need to initialize 8 MACSec units per device */
        devStart = (devNum*8) + (4 - 4*(0x1 & unitBmp));
        devEnd = devStart + 4*(0x1 & unitBmp) + 4*((0x2 & unitBmp)>>1) ;
    }
    else
    {
        /* Single DP therefore need to initialize only 4 MACSec units of DP0 */
        devStart = (devNum*8) ;
        devEnd = devStart + 4 ;
    }

    /* Go over static devices list and copy devices information */
    for (res = devStart; res < devEnd; res++)
    {
        /* Checks if current device name is already presented in the device list */
        if (DeviceLib_Device_Exists(DevStatAdmin_p[res].unitName))
        {
            goto error_exit;
        }

        /* Allocate memory for device administration data */
        DevAdmin_pp[res] = cpssOsMalloc(sizeof(Device_Admin_t));
        if (DevAdmin_pp[res] == NULL)
        {
            goto error_exit;
        }

        /* Allocate and copy device name */
        DevAdmin_pp[res]->DevName = cpssOsMalloc((GT_U32)cpssOsStrlen(DevStatAdmin_p[res].unitName)+1);
        if (DevAdmin_pp[res]->DevName == NULL)
        {
            goto error_exit;
        }
        /* Copy device name from static list */
        cpssOsStrCpy(DevAdmin_pp[res]->DevName, DevStatAdmin_p[res].unitName);

        /* Get base address of current device */
        unitBaseAddr = prvCpssDxChHwUnitBaseAddrGet(devNum,DevStatAdmin_p[res].unitId,&didErr);
        if(didErr == GT_TRUE)
        {
            goto error_exit;
        }

        /* Copy the rest of device data */
        DevAdmin_pp[res]->Flags    = DevStatAdmin_p[res].flags;
        DevAdmin_pp[res]->DeviceId = res;
        DevAdmin_pp[res]->FirstOfs = unitBaseAddr;
        DevAdmin_pp[res]->LastOfs  = unitBaseAddr + DevStatAdmin_p[res].unitOfsEnd;
        DevAdmin_pp[res]->devNum   = devNum;
    }

    /* Indicate that all devices were initialized */
    DevGlobalAdmin_p->fInitialized = true;
    return GT_OK; /* success */

error_exit:
    /* Free all allocated memory */
    for (res = devStart; res < devEnd; res++)
        if (DevAdmin_pp[res])
        {
            cpssOsFree(DevAdmin_pp[res]->DevName);
            cpssOsFree(DevAdmin_pp[res]);
            DevAdmin_pp[res] = NULL;
        }
    rc = GT_ERROR;
    return rc; /* failed */
}



/*-----------------------------------------------------------------------------
 * Device_UnInitialize
 */
void
Device_UnInitialize(GT_U8 devNum, GT_U32 unitBmp, GT_BOOL lastDevice)
{
    Device_Admin_t ** DevAdmin_pp = Device_Internal_Admin_Get();
    Device_Global_Admin_t * DevGlobalAdmin_p = Device_Internal_Admin_Global_Get();
    GT_U32 devStart; /* Device specific start entry to fill MACSec units DB */
    GT_U32 devEnd;   /* Device specific end entry covering up to 8 MACSec units */

    /* DB tables hold information for up to 8 MACSec units per device.
       First 4 units represent DP0 and last 4 units represent DP1.
       Need to check if device is single DP or dual DP and uninitialize DB tables accordingly */
    if (PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.macSecInfo.macSecDpBmp == 3)
    {
        /* Dual DP therefore need to initialize up to 8 MACSec units of DP0 and DP1
           Three options:
           unitBmp:0x1 => only DP0, threfore need to uninitialize only first 4 MACSec units per device
           unitBmp:0x2 => only DP1, threfore need to uninitialize only last 4 MACSec units per device
           unitBmp:0x3 => DP0 & DP1, threfore need to uninitialize 8 MACSec units per device */
        devStart = (devNum*8) + (4 - 4*(0x1 & unitBmp));
        devEnd = devStart + 4*(0x1 & unitBmp) + 4*((0x2 & unitBmp)>>1) ;
    }
    else
    {
        /* Single DP therefore need to uninitialize only 4 MACSec units of DP0 */
        devStart = (devNum*8) ;
        devEnd = devStart + 4 ;
    }

    if (DevGlobalAdmin_p->fInitialized)
    {
        GT_U32 i;

        Device_Internal_UnInitialize();

        /* Free all allocated memory */
        for (i = devStart; i < devEnd; i++)
            if (DevAdmin_pp[i])
            {
                cpssOsFree(DevAdmin_pp[i]->DevName);
                cpssOsFree(DevAdmin_pp[i]);
                DevAdmin_pp[i] = NULL;
            }

        /* Set to false only if this is the last device working with MACSec */
        if (lastDevice)
        {
            DevGlobalAdmin_p->fInitialized = false;
        }
    }
}


/*-----------------------------------------------------------------------------
 * Device_Find
 */
Device_Handle_t
Device_Find(const GT_CHAR * DeviceName_p)
{
    GT_U32 i;
    GT_U32 DevCount = Device_Internal_Static_Count_Get();
    Device_Admin_t ** DevAdmin_pp = Device_Internal_Admin_Get();
    Device_Global_Admin_t * DevGlobalAdmin_p = Device_Internal_Admin_Global_Get();

    if (!DevGlobalAdmin_p->fInitialized)
    {
        return NULL;
    }

#ifdef HWPAL_STRICT_ARGS_CHECK
    if (DeviceName_p == NULL)
    {
        return NULL; /* not supported, thus not found */
    }
#endif

    /* walk through the device list and compare the device name */
    for (i = 0; i < DevCount; i++)
        if (DevAdmin_pp[i] &&
            cpssOsStrCmp(DeviceName_p, DevAdmin_pp[i]->DevName) == 0)
            /* Return the device handle */
            return Device_Internal_Find(DeviceName_p, i);


    return NULL;
}


/*----------------------------------------------------------------------------
 * Device_GetProperties
 */
GT_32
Device_GetProperties(
        const GT_U32 Index,
        Device_Properties_t * const Props_p,
        bool * const fValid_p)
{
    Device_Admin_t ** DevAdmin_pp = Device_Internal_Admin_Get();
    Device_Global_Admin_t * DevGlobalAdmin_p = Device_Internal_Admin_Global_Get();

    if (!DevGlobalAdmin_p->fInitialized)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_ERROR, LOG_ERROR_NO_MSG);
    }

#ifdef HWPAL_STRICT_ARGS_CHECK
    if (Index >= Device_Internal_Count_Get())
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_ERROR, LOG_ERROR_NO_MSG);
    }

    if (Props_p == NULL || fValid_p == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_ERROR, LOG_ERROR_NO_MSG);
    }
#endif

    if (!DevAdmin_pp[Index])
    {
        *fValid_p = false;
    }
    else
    {
        Props_p->Name_p          = DevAdmin_pp[Index]->DevName;
        Props_p->StartByteOffset = DevAdmin_pp[Index]->FirstOfs;
        Props_p->LastByteOffset  = DevAdmin_pp[Index]->LastOfs;
        Props_p->Flags           = (char)DevAdmin_pp[Index]->Flags;
        Props_p->Flags           = (char)DevAdmin_pp[Index]->DeviceId;
        Props_p->Flags           = (char)DevAdmin_pp[Index]->devNum;
        *fValid_p = true;
    }
    return GT_OK;
}



/*----------------------------------------------------------------------------
 * Device_Add
 */
GT_32
Device_Add(
        const GT_U32 Index,
        const Device_Properties_t * const Props_p)
{
    Device_Admin_t ** DevAdmin_pp = Device_Internal_Admin_Get();
    Device_Global_Admin_t * DevGlobalAdmin_p = Device_Internal_Admin_Global_Get();

    if (!DevGlobalAdmin_p->fInitialized)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_ERROR, LOG_ERROR_NO_MSG);
    }

#ifdef HWPAL_STRICT_ARGS_CHECK
    if (Index >= Device_Internal_Count_Get())
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_ERROR, LOG_ERROR_NO_MSG);
    }

    if (Props_p == NULL || Props_p->Name_p == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_ERROR, LOG_ERROR_NO_MSG);
    }
#endif

    if (DevAdmin_pp[Index])
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_ERROR, LOG_ERROR_NO_MSG);
    }

    if (DeviceLib_Device_Exists(Props_p->Name_p))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_ERROR, LOG_ERROR_NO_MSG);
    }

    /* Allocate memory for device administration data */
    DevAdmin_pp[Index] = cpssOsMalloc(sizeof(Device_Admin_t));
    if (DevAdmin_pp[Index] == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_ERROR, LOG_ERROR_NO_MSG);
    }

    /* Allocate and copy device name */
    DevAdmin_pp[Index]->DevName = cpssOsMalloc((GT_U32)cpssOsStrlen(Props_p->Name_p)+1);
    if (DevAdmin_pp[Index]->DevName == NULL)
    {
        Device_Internal_Free(DevAdmin_pp[Index]);
        DevAdmin_pp[Index] = NULL;
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_ERROR, LOG_ERROR_NO_MSG);
    }
    cpssOsStrCpy(DevAdmin_pp[Index]->DevName, Props_p->Name_p);

    /* Copy the rest */
    DevAdmin_pp[Index]->FirstOfs  = Props_p->StartByteOffset;
    DevAdmin_pp[Index]->LastOfs   = Props_p->LastByteOffset;
    DevAdmin_pp[Index]->Flags     = (GT_U32)Props_p->Flags;
    DevAdmin_pp[Index]->DeviceId  = Index;

    return GT_OK; /* success */
}


/*----------------------------------------------------------------------------
 * Device_Remove
 */
GT_32
Device_Remove(const GT_U32 Index)
{
    Device_Admin_t ** DevAdmin_pp = Device_Internal_Admin_Get();
    Device_Global_Admin_t * DevGlobalAdmin_p = Device_Internal_Admin_Global_Get();

    if (!DevGlobalAdmin_p->fInitialized)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_ERROR, LOG_ERROR_NO_MSG);
    }

#ifdef HWPAL_STRICT_ARGS_CHECK
    if (Index >= Device_Internal_Count_Get())
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_ERROR, LOG_ERROR_NO_MSG);
    }
#endif

    if (!DevAdmin_pp[Index])
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_ERROR, LOG_ERROR_NO_MSG);
    }
    else
    {
        /* Free device memory */
        cpssOsFree(DevAdmin_pp[Index]->DevName);
        cpssOsFree(DevAdmin_pp[Index]);
        DevAdmin_pp[Index] = NULL;
    }

    return GT_OK; /* success */
}


/*-----------------------------------------------------------------------------
 * Device_GetName
 */
char *
Device_GetName(const GT_U32 Index)
{
    Device_Admin_t ** DevAdmin_pp = Device_Internal_Admin_Get();
    Device_Global_Admin_t * DevGlobalAdmin_p = Device_Internal_Admin_Global_Get();

    if (!DevGlobalAdmin_p->fInitialized)
    {
        return NULL;
    }

#ifdef HWPAL_STRICT_ARGS_CHECK
    if (Index >= Device_Internal_Count_Get())
    {
        return NULL;
    }
#endif

    if (!DevAdmin_pp[Index])
    {
        return NULL;
    }

    return DevAdmin_pp[Index]->DevName; /* success */
}


/*-----------------------------------------------------------------------------
 * Device_GetIndex
 */
GT_32
Device_GetIndex(const Device_Handle_t Device)
{
    Device_Global_Admin_t * DevGlobalAdmin_p = Device_Internal_Admin_Global_Get();

    if (!DevGlobalAdmin_p->fInitialized)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_ERROR, LOG_ERROR_NO_MSG);
    }

    return Device_Internal_GetIndex(Device);
}


/*-----------------------------------------------------------------------------
 * Device_GetCount
 */
GT_U32
Device_GetCount(void)
{
    return Device_Internal_Count_Get();
}


/* end of file device_generic.c */
