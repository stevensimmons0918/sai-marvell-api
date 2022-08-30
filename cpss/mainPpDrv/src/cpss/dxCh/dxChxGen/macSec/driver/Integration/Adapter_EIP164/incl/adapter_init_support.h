/* adapter_init_support.h
 *
 * Internal support API for adapter init functions.
 */

/*****************************************************************************
* Copyright (c) 2017-2018 INSIDE Secure B.V. All Rights Reserved.
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

#ifndef INCLUDE_GUARD_ADAPTER_INIT_SUPPORT_H
#define INCLUDE_GUARD_ADAPTER_INIT_SUPPORT_H

/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

/* Driver Framework Basic Definitions API */
#include <Kit/DriverFramework/incl/basic_defs.h>  /* bool */

#include <Integration/Adapter_EIP164/incl/api_driver164_init.h> /* Driver164_Device_t */

/*----------------------------------------------------------------------------
 * Definitions and macros
 */


/*----------------------------------------------------------------------------
 * Adapter_Init_DataPath_Add
 *
 * Adds all devices for a complete data path to the driver device list.
 *
 * This function must be called before any other driver function can reference
 * this data path. It can be used as an alternative or in combination with
 * static device list configuration in the driver.
 *
 * Index (input)
 *     Data path index of the data path that must be added in the device list
 *
 * DataPath_p (input)
 *     Pointer to memory location where data path properties are stored, may
 *     not be NULL
 *
 * Returns 0 for success and -1 for failure.
 */
int
Adapter_Init_DataPath_Add(
        const unsigned int Index,
        const Driver164_DataPath_t * const DataPath_p);


/*----------------------------------------------------------------------------
 * Adapter_Init_DataPath_Remove
 *
 * Removes all devices of a data path from the driver device list at
 * the requested index, the data path must be previously added either
 * statically or via a call to the Driver164_DataPath_Add() function.
 *
 * This function must be called when no other driver function can reference
 * this device.
 *
 * Index (input)
 *     Data path index of the data path that must be removed from the device
 *     list
 *
 * Returns 0 for success and -1 for failure.
 */
int
Adapter_Init_DataPath_Remove(
        const unsigned int Index);


/*----------------------------------------------------------------------------
 * Adapter_Init_DataPath_GetProperties
 *
 * This function reads the properties of any installed data path.
 *
 * Index (input)
 *     Data path index of the data path whose properties are read.
 *
 * DataPaths_p (output)
 *     Pointer to a memory location where data path properties are stored.
 *     These properties can be used to add the same data path later via
 *     Adapter_Init_DataPath_Add().
 *
 * fValid_p (output)
 *     Flag that indicates whether a valid data path exists at the given index.
 *
 * Returns 0 for success and -1 for failure.
 */
int
Adapter_Init_DataPath_GetProperties(
        unsigned int Index,
        Driver164_DataPath_t * const DataPath_p,
        bool * const fValid_p);


/*----------------------------------------------------------------------------
 * Adapter_Init_DataPath_GetIndexRange
 *
 * This function returns the range of the device indexes used by a given
 * data path.
 *
 * Index (input)
 *     Data path index of the data path whose properties are read.
 *
 * FirstIndex_p (output)
 *     Device index of the first device in the data path.
 *
 * LastIndex_p (output)
 *     Device index of the last device in the data path + 1.
 *
 * Returns 0 for success and -1 for failure.
 */
int
Adapter_Init_DataPath_GetIndexRange(
        unsigned int Index,
        unsigned int * const FirstIndex_p,
        unsigned int * const LastIndex_p);


#endif /* INCLUDE_GUARD_ADAPTER_INIT_SUPPORT_H */


/* end of file adapter_init_support.h */
