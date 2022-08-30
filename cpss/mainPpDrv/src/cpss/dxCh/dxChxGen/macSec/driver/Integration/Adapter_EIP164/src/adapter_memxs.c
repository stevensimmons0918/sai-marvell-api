/* adapter_memxs.c
 *
 * Low-level Memory Access API implementation.
 */

/*****************************************************************************
* Copyright (c) 2012-2016 INSIDE Secure B.V. All Rights Reserved.
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

/* MemXS API */
#include <Integration/Adapter_EIP164/incl/api_memxs.h>


/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

/* Default configuration */
#include <Integration/Adapter_EIP164/incl/c_adapter_memxs.h>

/* Driver Framework Basic Definitions API */
#include <Kit/DriverFramework/incl/basic_defs.h>

/* Driver Framework Device API */
#include <Kit/DriverFramework/incl/device_types.h>       /* Device_Handle_t */
#include <Kit/DriverFramework/incl/device_rw.h>         /* Device_Read/Write */
#include <Kit/DriverFramework/incl/device_mgmt.h>


/*----------------------------------------------------------------------------
 * Definitions and macros
 */

/*----------------------------------------------------------------------------
 * MemXS_DeviceAdmin_t
 *
 * MemXS device record
 */
typedef struct
{
    const char *        DevName;

    unsigned int        FirstOfs;

    unsigned int        LastOfs;

    Device_Handle_t     MemXS_Device;

} MemXS_DeviceAdmin_t;


/*----------------------------------------------------------------------------
 * Local variables
 */

/* Here is the dependency on the Driver Framework configuration */
/* via the MemXS configuration */
#ifndef HWPAL_DEVICES
#error "Expected HWPAL_DEVICES defined by cs_memxs.h"
#endif

/* Here is the dependency on the Driver Framework configuration */
/* via the MemXS configuration */
#undef HWPAL_DEVICE_ADD
#define HWPAL_DEVICE_ADD(_name, _firstofs, _lastofs, _flags) \
           { _name, _firstofs, _lastofs, NULL }

static MemXS_DeviceAdmin_t MemXS_Devices[] =
{
    HWPAL_DEVICES
};

static const unsigned int MemXS_Device_Count =
        (sizeof(MemXS_Devices) / sizeof(MemXS_DeviceAdmin_t));


/*----------------------------------------------------------------------------
 * MemXS_NULLHandle
 *
 */
const MemXS_Handle_t MemXS_NULLHandle = { 0 };


/*----------------------------------------------------------------------------
 * MemXS_Init
 */
bool
MemXS_Init (void)
{
    unsigned int i;

    for(i = 0; i < MemXS_Device_Count; i++)
    {
        MemXS_Devices[i].MemXS_Device = NULL;

        MemXS_Devices[i].MemXS_Device =
                Device_Find (MemXS_Devices[i].DevName);

        if (MemXS_Devices[i].MemXS_Device == NULL)
        {
            return false;
        }
    } /* for */

    return true;
}


/*-----------------------------------------------------------------------------
 * MemXS_Handle_IsSame
 */
bool
MemXS_Handle_IsSame(
        const MemXS_Handle_t * const Handle1_p,
        const MemXS_Handle_t * const Handle2_p)
{
    return Handle1_p->p == Handle2_p->p;
}


/*-----------------------------------------------------------------------------
 * MemXS_Device_Count_Get
 */
MemXS_Status_t
MemXS_Device_Count_Get(
        unsigned int * const DeviceCount_p)
{
    if (DeviceCount_p == NULL)
        return MEMXS_INVALID_PARAMETER;

    *DeviceCount_p = MemXS_Device_Count;

    return MEMXS_STATUS_OK;
}


/*-----------------------------------------------------------------------------
 * MemXS_Device_Info_Get
 */
MemXS_Status_t
MemXS_Device_Info_Get(
        const unsigned int DeviceIndex,
        MemXS_DevInfo_t * const DeviceInfo_p)
{
    if (DeviceInfo_p == NULL)
        return MEMXS_INVALID_PARAMETER;

    if (DeviceIndex > MemXS_Device_Count)
        return MEMXS_INVALID_PARAMETER;

    DeviceInfo_p->Index    = DeviceIndex;
    DeviceInfo_p->Handle.p = MemXS_Devices[DeviceIndex].MemXS_Device;
    DeviceInfo_p->Name_p   = MemXS_Devices[DeviceIndex].DevName;
    DeviceInfo_p->FirstOfs = MemXS_Devices[DeviceIndex].FirstOfs;
    DeviceInfo_p->LastOfs  = MemXS_Devices[DeviceIndex].LastOfs;

    return MEMXS_STATUS_OK;
}


/*----------------------------------------------------------------------------
 * MemXS_Read32
 */
uint32_t
MemXS_Read32(
        const MemXS_Handle_t Handle,
        const unsigned int ByteOffset)
{
    Device_Handle_t Device = (Device_Handle_t)Handle.p;

    return Device_Read32(Device, ByteOffset);
}


/*----------------------------------------------------------------------------
 * MemXS_Write32
 */
void
MemXS_Write32(
        const MemXS_Handle_t Handle,
        const unsigned int ByteOffset,
        const uint32_t Value)
{
    Device_Handle_t Device = (Device_Handle_t)Handle.p;

    Device_Write32(Device, ByteOffset, Value);
}


/*----------------------------------------------------------------------------
 * MemXS_Read32Array
 */
void
MemXS_Read32Array(
        const MemXS_Handle_t Handle,
        const unsigned int ByteOffset,
        uint32_t * MemoryDst_p,
        const int Count)
{
    Device_Handle_t Device = (Device_Handle_t)Handle.p;

    Device_Read32Array(Device, ByteOffset, MemoryDst_p, Count);
}


/*----------------------------------------------------------------------------
 * MemXS_Write32Array
 */
void
MemXS_Write32Array(
        const MemXS_Handle_t Handle,
        const unsigned int ByteOffset,
        const uint32_t * MemorySrc_p,
        const int Count)
{
    Device_Handle_t Device = (Device_Handle_t)Handle.p;

    Device_Write32Array(Device, ByteOffset, MemorySrc_p, Count);
}


/* end of file adapter_memxs.c */
