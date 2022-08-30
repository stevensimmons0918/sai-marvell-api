/* hwpal_device_umdevxs.c
 *
 * This is the Linux User-mode Driver Framework v4 Device API
 * implementation for UMDevXS. The implementation is device-agnostic and
 * receives configuration details from the cs_hwpal_umdevxs.h file.
 * This implementation supports mapping two UMDevXS devices, to for example
 * allow control of the FPGA via another device.
 *
 * This implementation uses the UMDevXS Proxy and Kernel Driver to map a
 * memory region with the device registers into user space memory.
 */

/*****************************************************************************
* Copyright (c) 2009-2019 INSIDE Secure B.V. All Rights Reserved.
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
#include <Kit/DriverFramework/incl/device_rw.h>              /* API to implement */
#include <Kit/DriverFramework/incl/device_swap.h>

/* Driver Framework Device Internal interface */
#include <Integration/DriverFramework/src/device_internal.h>        /* Device_Internal_* */


/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

/* Default configuration */
#include <Integration/DriverFramework/src/c_hwpal_device_umdevxs.h> /* get the configuration options */

/* Driver Framework Basic Defs API */
#include <Kit/DriverFramework/incl/basic_defs.h>             /* uint32_t, NULL, inline, etc. */

/* Standard functions API */
#include <Kit/DriverFramework/incl/clib.h>                   /* cpssMemCmp */

/* Driver Framework Device Platform interface */
#include <Integration/DriverFramework/src/device_platform.h>        /* Device_Platform_* */

/* Logging API */
#include <Kit/Log/incl/log.h>

#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChRegsVer1.h>

#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>
#include <cpss/dxCh/dxChxGen/macSec/private/prvCpssDxChMacSecGlobalDb.h>

/*global variables macros*/
#define PRV_SHARED_MACSEC_DRIVER_DIR_HWPAL_DEVICE_UMDEVXS_SRC_GLOBAL_VAR_GET(_var)\
    (((PRV_CPSS_DXCH_MACSEC_GLOBAL_DB_STC *)PRV_SHARED_GLOBAL_VAR_GET(mainPpDrvMod.macSecDriverDir.dbPtr))->hwpalDeviceUmdevxsSrc._var)

/*----------------------------------------------------------------------------
 * Definitions and macros
 */
/* definition of the Flags */
#define HWPAL_FLAGS_READ   BIT_0
#define HWPAL_FLAGS_WRITE  BIT_1



/*----------------------------------------------------------------------------
 * Local variables
 */

static const Device_Admin_Static_t HWPALLib_Devices_Static[] =
{
    HWPAL_DEVICES
};



/*----------------------------------------------------------------------------
 * HWPALLib_IsValid
 *
 * This function checks that the parameters are valid to make the access.
 *
 * Device_p is valid
 * ByteOffset is 32-bit aligned
 * ByteOffset is inside device memory range
 */
static inline GT_BOOL
HWPALLib_IsValid(
        const Device_Admin_t * const device_p,
        const GT_U32 byteOffset)
{
    if (device_p == NULL)
        return false;

    if (byteOffset & 3)
        return false;

    if (device_p->FirstOfs + byteOffset > device_p->LastOfs)
        return false;

    return true;
}




/*-----------------------------------------------------------------------------
 * device_internal interface
 *
 */

/*----------------------------------------------------------------------------
 * Device_Internal_Static_Count_Get
 */
GT_U32
Device_Internal_Static_Count_Get(void)
{
    return HWPAL_DEVICE_STATIC_COUNT;
}


/*----------------------------------------------------------------------------
 * Device_Internal_Count_Get
 */
GT_U32
Device_Internal_Count_Get(void)
{
    return HWPAL_DEVICE_COUNT;
}


/*----------------------------------------------------------------------------
 * Device_Internal_Admin_Static_Get
 */
const Device_Admin_Static_t *
Device_Internal_Admin_Static_Get(void)
{
    return HWPALLib_Devices_Static;
}


/*----------------------------------------------------------------------------
 * Device_Internal_Admin_Get
 *
 * Returns pointer to the memory location where the device list is stored.
 *
 */
Device_Admin_t **
Device_Internal_Admin_Get(void)
{
    return PRV_SHARED_MACSEC_DRIVER_DIR_HWPAL_DEVICE_UMDEVXS_SRC_GLOBAL_VAR_GET(HWPALLib_Devices_p);
}


/*----------------------------------------------------------------------------
 * Device_Internal_Admin_Global_Get
 */
Device_Global_Admin_t *
Device_Internal_Admin_Global_Get(void)
{
    return &(PRV_SHARED_MACSEC_DRIVER_DIR_HWPAL_DEVICE_UMDEVXS_SRC_GLOBAL_VAR_GET(HWPALLib_Device_Global));
}

/*----------------------------------------------------------------------------
 * Device_Internal_Alloc
 */
void *
Device_Internal_Alloc(
        unsigned int byteCount)
{
    return cpssOsMalloc(byteCount);
}


/*----------------------------------------------------------------------------
 * Device_Internal_Free
 */
void
Device_Internal_Free(
        void * ptr)
{
    cpssOsFree(ptr);
}



/*-----------------------------------------------------------------------------
 * Device_Internal_UnInitialize
 */
void
Device_Internal_UnInitialize(void)
{
}


/*-----------------------------------------------------------------------------
 * Device_Internal_Find
 */
Device_Handle_t
Device_Internal_Find(
        const char * deviceName_p,
        const GT_U32 index)
{
    IDENTIFIER_NOT_USED(deviceName_p);

    /* Return the device handle */
    return (Device_Handle_t)PRV_SHARED_MACSEC_DRIVER_DIR_HWPAL_DEVICE_UMDEVXS_SRC_GLOBAL_VAR_GET(HWPALLib_Devices_p)[index];
}


/*-----------------------------------------------------------------------------
 * Device_Internal_GetIndex
 */
GT_32
Device_Internal_GetIndex(
        const Device_Handle_t device)
{
    Device_Admin_t * device_p;

    device_p = device;

#ifdef HWPAL_STRICT_ARGS_CHECK
    if (!HWPALLib_IsValid(device_p, 0))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_ERROR, LOG_ERROR_NO_MSG);
    }
#endif

    return (GT_32)device_p->DeviceId;
}


/*-----------------------------------------------------------------------------
 * device_rw API
 *
 * These functions can be used to transfer a single 32bit word or an array of
 * 32bit words to or from a device.
 * Endianness swapping is performed on the fly based on the configuration for
 * this device.
 *
 */

/*-----------------------------------------------------------------------------
 * Device_Read32
 */
GT_U32
Device_Read32(
        const Device_Handle_t device,
        const GT_U32 byteOffset)
{
    GT_U32 value = 0;
    Device_Read32Check(device, byteOffset, &value);
    return value;
}


/*-----------------------------------------------------------------------------
 * Device_Read32Check
 */
GT_32
Device_Read32Check(
        const Device_Handle_t device,
        const GT_U32 byteOffset,
        GT_U32 * const value_p)
{
    Device_Admin_t * device_p;
    GT_U32 wordRead;
    GT_U32 devRegAddr;
    GT_STATUS rc;

    if (!PRV_SHARED_MACSEC_DRIVER_DIR_HWPAL_DEVICE_UMDEVXS_SRC_GLOBAL_VAR_GET(HWPALLib_Device_Global).fInitialized)
    {
        return (GT_32)DEVICE_NOT_INITIALIZED_ERROR;
    }

    /* Device handle of Device_Admin_t type */
    device_p = device;

#ifdef HWPAL_STRICT_ARGS_CHECK
    if (!HWPALLib_IsValid(device_p, byteOffset))
    {
        return DEVICE_RW_PARAM_ERROR;
    }
#endif

    /* Get register address */
    devRegAddr = device_p->FirstOfs + byteOffset;

#ifdef HWPAL_DEVICE_READ_DELAY_US
    /* Convert to nanosec and call cpssOsDelay */
    cpssOsDelay(HWPAL_DEVICE_READ_DELAY_US*1000);
#endif

    /* Perform register read */
    rc = prvCpssDrvHwPpReadRegister(device_p->devNum ,devRegAddr, &wordRead);
    if( rc != GT_OK )
    {
        return rc;
    }

    *value_p  = wordRead;

    return GT_OK;
}


/*-----------------------------------------------------------------------------
 * Device_Write32
 */
GT_32
Device_Write32(
        const Device_Handle_t device,
        const GT_U32 byteOffset,
        const GT_U32 value)
{
    Device_Admin_t * device_p;
    GT_U32 devRegAddr;
    GT_STATUS rc;
    GT_U32 wordWrite = value;

    if (!PRV_SHARED_MACSEC_DRIVER_DIR_HWPAL_DEVICE_UMDEVXS_SRC_GLOBAL_VAR_GET(HWPALLib_Device_Global).fInitialized)
    {
        return DEVICE_RW_PARAM_ERROR;
    }

    /* Device handle of Device_Admin_t type */
    device_p = device;

#ifdef HWPAL_STRICT_ARGS_CHECK
    if (!HWPALLib_IsValid(device_p, byteOffset))
    {
        return DEVICE_RW_PARAM_ERROR;
    }
#endif


    /* Get register address */
    devRegAddr = device_p->FirstOfs + byteOffset;

    /* Perform register write */
    rc = prvCpssDrvHwPpWriteRegister(device_p->devNum ,devRegAddr, wordWrite);
    if( rc != GT_OK )
    {
        return rc;
    }

    return GT_OK;
}


/*-----------------------------------------------------------------------------
 * Device_Read32Array
 */
GT_32
Device_Read32Array(
        const Device_Handle_t device,
        const GT_U32 startByteOffset,
        GT_U32 * memoryDst_p,
        const GT_32 count)
{
    Device_Admin_t * device_p;
    GT_U32 wordRead;
    GT_U32 devRegAddrStart;
    GT_32 nWords;
    GT_STATUS rc;

    if (!PRV_SHARED_MACSEC_DRIVER_DIR_HWPAL_DEVICE_UMDEVXS_SRC_GLOBAL_VAR_GET(HWPALLib_Device_Global).fInitialized)
    {
        return DEVICE_RW_PARAM_ERROR;
    }

    /* Device handle of Device_Admin_t type */
    device_p = device;

    if (count == 0)
    {
        /* avoid that `count-1' goes negative in test below */
        return 0;
    }

#ifdef HWPAL_STRICT_ARGS_CHECK
    if ((count < 0) ||
        !HWPALLib_IsValid(device_p, startByteOffset) ||
        !HWPALLib_IsValid(device_p, startByteOffset + (((GT_U32)count - 1) * (GT_U32)sizeof(GT_U32))))
    {
        return DEVICE_RW_PARAM_ERROR;
    }
#endif

    /* Get register address */
    devRegAddrStart = device_p->FirstOfs + startByteOffset;

    for (nWords = 0; nWords < count; nWords++)
    {
        /* Perform register read */
        rc = prvCpssDrvHwPpReadRegister(device_p->devNum ,devRegAddrStart + nWords*4, &wordRead);
        if( rc != GT_OK )
        {
            return rc;
        }

        memoryDst_p[nWords] = wordRead;

    }

    return GT_OK;
}


/*-----------------------------------------------------------------------------
 * Device_Read32Array
 */
GT_32
Device_Write32Array(
        const Device_Handle_t device,
        const GT_U32 startByteOffset,
        const GT_U32 * memorySrc_p,
        const GT_32 count)
{
    Device_Admin_t * device_p;
    GT_U32 wordWrite;
    GT_32 nWords;
    GT_U32 devRegAddrStart;
    GT_STATUS rc;

    if (!PRV_SHARED_MACSEC_DRIVER_DIR_HWPAL_DEVICE_UMDEVXS_SRC_GLOBAL_VAR_GET(HWPALLib_Device_Global).fInitialized)
    {
        return DEVICE_RW_PARAM_ERROR;
    }

    /* Device handle of Device_Admin_t type */
    device_p = device;

    if (count == 0)
        return 0; /* avoid that `Count-1' goes negative in test below */

#ifdef HWPAL_STRICT_ARGS_CHECK
    if ((count < 0) ||
        !HWPALLib_IsValid(device_p, startByteOffset) ||
        !HWPALLib_IsValid(device_p, startByteOffset + (((GT_U32)count - 1) *  (GT_U32)sizeof(GT_U32))))
    {
        return DEVICE_RW_PARAM_ERROR;
    }
#endif

    devRegAddrStart = device_p->FirstOfs + startByteOffset;
    for (nWords = 0; nWords < count; nWords++)
    {
        /* Get the value you want to write */
        wordWrite = memorySrc_p[nWords];

        /* Perform register write */
        rc = prvCpssDrvHwPpWriteRegister(device_p->devNum, devRegAddrStart + nWords*4, wordWrite);
        if( rc != GT_OK )
        {
            return rc;
        }

    }

    return GT_OK;
}


/* end of file hwpal_device_umdevxs.c */
