/* adapter_debug.c
 *
 * Driver Debugging API implementation.
 *
 */

/*****************************************************************************
* Copyright (c) 2012-2013 INSIDE Secure B.V. All Rights Reserved.
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

#include <Integration/Adapter_EIP164/incl/api_debug.h>


/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

/* Top-level Adapter configuration */
/*#include "c_adapter_debug.h" */

/* Driver Framework Device API */
#include <Kit/DriverFramework/incl/device_mgmt.h> /* Device_Find */
#include <Kit/DriverFramework/incl/device_rw.h>   /* Device_Read32, Device_Write32 */

/*----------------------------------------------------------------------------
 * Local variables
 */


/*----------------------------------------------------------------------------
 * Debug_Device_Read32
 */
int
Debug_Device_Read32(
        const char * const DeviceName_p,
        const unsigned int ReadOffset,
        uint32_t * Value_p)
{
    Device_Handle_t Device = Device_Find(DeviceName_p);

    if (Device == NULL)
        return -1; /* failure, device not found */

    *Value_p = Device_Read32(Device, ReadOffset);

    return 0; /* success */
}


/* end of file adapter_debug.c */
