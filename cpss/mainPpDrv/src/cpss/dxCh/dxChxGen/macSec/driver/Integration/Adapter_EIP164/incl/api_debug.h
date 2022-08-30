/* api_debug.h
 *
 * Driver Debugging API
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

#ifndef API_DEBUG_H_
#define API_DEBUG_H_

/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

/* Driver Framework Device API */
#include <Kit/DriverFramework/incl/device_mgmt.h>   /* SecY_SAHandle_t, SecY_Rules_* */


/*----------------------------------------------------------------------------
 * Definitions and macros
 */


/*----------------------------------------------------------------------------
 * Debug_Device_Read32
 *
 * Reads a device static resource such as register or internal memory
 *
 * API use order:
 *      None
 *
 * DeviceName_p (input)
 *      Device name to be read
 *
 * ReadOffset (input)
 *      Resource read offset within the device memory map
 *
 * Value_p (output)
 *      Pointer to a memory location where the read result will be stored.
 *
 * This function is re-entrant for the same or different DeviceName_p.
 *
 * This function can be called concurrently with any other Debug API
 * function for the same or different DeviceId provided the API use order
 * is followed.
 *
 * Return value:
 *     0 :  device resource is read
 *     <0 : failure
 */
int
Debug_Device_Read32(
        const char * const DeviceName_p,
        const unsigned int ReadOffset,
        uint32_t * Value_p);


#endif /* API_DEBUG_H_ */


/* end of file api_debug.h */
