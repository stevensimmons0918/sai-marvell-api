/* device_mgmt.h
 *
 * Driver Framework, Device API, Management functions
 *
 * The document "Driver Framework Porting Guide" contains the detailed
 * specification of this API. The information contained in this header file
 * is for reference only.
 */

/*****************************************************************************
* Copyright (c) 2007-2017 INSIDE Secure B.V. All Rights Reserved.
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

#ifndef INCLUDE_GUARD_DEVICE_MGMT_H
#define INCLUDE_GUARD_DEVICE_MGMT_H

/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

/* Driver Framework Device API API */
#include <Kit/DriverFramework/incl/device_types.h>   /* Device_Handle_t, Device_Reference_t */

/* Driver Framework Basic Definitions API */
#include <Kit/DriverFramework/incl/basic_defs.h>    /* bool, uint32_t, inline */


/*----------------------------------------------------------------------------
 * Definitions and macros
 */


/*----------------------------------------------------------------------------
 * Device_Initialize
 *
 * This function must be called exactly once to initialize the Device
 * implementation before any other API function may be used.
 *
 * devNum
 *     This anonymous parameter can be used to pass information from the caller
 *     to the driver framework implementation.
 *
 * Return Value
 *     0    Success
 *     <0   Error code (implementation specific)
 */
GT_32
Device_Initialize(GT_U8 devNum, GT_U32 unitBmp);


/*----------------------------------------------------------------------------
 * Device_UnInitialize
 *
 * This function can be called to shut down the Device implementation. The
 * caller must make sure none of the other API functions are called after or
 * during the invocation of this UnInitialize function. After this call
 * returns the API state is back in "uninitialized" and the Device_Initialize
 * function may be called anew.
 *
 * Return Value
 *     None
 */
void
Device_UnInitialize(GT_U8 devNum, GT_U32 unitBmp, GT_BOOL lastDevice);


/*----------------------------------------------------------------------------
 * Device_Find
 *
 * This function must be used to retrieve a handle for a certain device that
 * is identified by a string. The exact strings supported is implementation
 * specific and will differ from product to product.
 * Note that this function may be called more than once to retrieve the same
 * handle for the same device.
 *
 * DeviceName_p (input)
 *     Pointer to the (zero-terminated) string that represents the device.
 *
 * Return Value
 *     NULL    No device found with requested
 *     !NULL   Device handle that can be used in the Device API
 */
Device_Handle_t
Device_Find(
        const GT_CHAR * szDeviceName_p);


/*-----------------------------------------------------------------------------
 * Device_GetReference
 *
 * This function can be used to obtain the implementation-specific device
 * reference
 *
 * Device (input)
 *     Handle for the device instance as returned by Device_Find for which
 *     the reference must be obtained.
 *
 * Data_p (output)
 *     Pointer to memory location where device data will be stored.
 *     If not used then set to NULL.
 *
 * Return Value
 *     NULL    No reference found for the requested device instance
 *     !NULL   Device reference
 */
Device_Reference_t
Device_GetReference(
        const Device_Handle_t Device,
        Device_Data_t * const Data_p);


/*-----------------------------------------------------------------------------
 * Device_GetName
 *
 * This function returns the device name for the provided index.
 *
 * Index (input)
 *     Device index in the device list for which the name must be obtained
 *
 * Return Value
 *     NULL    No valid device found in the device list at the provided index
 *     !NULL   Device name
 */
GT_CHAR *
Device_GetName(
        const GT_U32 Index);


/*-----------------------------------------------------------------------------
 * Device_GetIndex
 *
 * This function returns the device index for the provided device handle.
 *
 * Device (input)
 *     Handle for the device instance as returned by Device_Find for which
 *     the reference must be obtained.
 *
 * Return Value
 *     >= 0 Device index
 *     <0   Error
 */
GT_32
Device_GetIndex(
        const Device_Handle_t Device);


/*-----------------------------------------------------------------------------
 * Device_GetCount
 *
 * This function returns the number of devices present in the device list.
 *
 * Return Value
 *     Device count.
 */
GT_U32
Device_GetCount(void);


/*----------------------------------------------------------------------------
 * Device_GetProperties
 *
 * Retrieve the properties of a device, the same as used in Device_Add
 *
 * Index (input)
 *     Device index indicating which device to read the properties from.
 *
 * Props_p (output)
 *     Pointer to memory location where device properties are stored,
 *     may not be NULL.
 *
 * fValid_p (output)
 *     Flag indicating if there is a valid device at this index.
 *
 * Return Value
 *     0    success
 *     -1   failure
 */
GT_32
Device_GetProperties(
        const GT_U32 Index,
        Device_Properties_t * const Props_p,
        bool * const fValid_p);


/*----------------------------------------------------------------------------
 * Device_Add
 *
 * Adds a new device to the device list.
 *
 * This function must be called before any other function can reference
 * this device.
 *
 * Index (input)
 *     Device index where the device must be added in the device list
 *
 * Props_p (input)
 *     Pointer to memory location where device properties are stored,
 *     may not be NULL
 *
 * Return Value
 *     0    success
 *     -1   failure
 */
GT_32
Device_Add(
        const GT_U32 Index,
        const Device_Properties_t * const Props_p);


/*----------------------------------------------------------------------------
 * Device_Remove
 *
 * Removes device from the device list at the requested index,
 * the device must be previously added either statically or via a call
 * to the Device_Add() function.
 *
 * This function must be called when no other driver function can reference
 * this device.
 *
 * Index (input)
 *     Device index where the device must be added in the device list
 *
 * Return Value
 *     0    success
 *     -1   failure
 */
GT_32
Device_Remove(
        const GT_U32 Index);


#endif /* Include Guard */


/* end of file device_mgmt.h */
