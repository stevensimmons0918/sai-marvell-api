/* adapter_warmboot_support.h
 *
 * WarmBoot support interface.
 */

/*****************************************************************************
* Copyright (c) 2017 INSIDE Secure B.V. All Rights Reserved.
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

#ifndef INCLUDE_GUARD_ADAPTER_WARMBOOT_SUPPORT_H
#define INCLUDE_GUARD_ADAPTER_WARMBOOT_SUPPORT_H

/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

/* Driver Framework Basic Definitions API */
#include <Kit/DriverFramework/incl/basic_defs.h>    /* bool */

/*----------------------------------------------------------------------------
 * Definitions and macros
 */
#define restrict  /* restrict keyword may not be accepted by some compilers. */


/*----------------------------------------------------------------------------
 * WarmBootLib_Alloc
 *
 * This function allocates a storage area of the desired size in
 * a memory area that will be preserved across CPU reboots. The allocated
 * area is filled entirely with bytes of value zero. The returned AreaId
 * represents a valid storage area. Each allocated (and not yet freed) storage
 * area has a unique AreaId.
 *
 * StorageByteCount (input)
 *     Size of the storage area in bytes.
 *
 * AreaId_p (output)
 *     ID of the allocated storage area.
 *
 * Return value:
 *     true: success.
 *     false: failure
 */
bool
WarmBootLib_Alloc(
        const unsigned int StorageByteCount,
        unsigned int * const AreaId_p);


/*----------------------------------------------------------------------------
 * WarmBootLib_Free
 *
 * This function frees a storage area previously allocated by
 * the WarnBoot_Alloc callback.  After this call, AreaId no longer corresponds
 * to a valid storage area and it may no longer be passed to callbacks of this
 * API. Future calls to the WarmBoot_Alloc callback may re-use this AreaId.
 *
 * AreaId (input)
 *     ID of the storage area to be freed.
 *
 * Return value:
 *     true: success.
 *     false: failure
 */
bool
WarmBootLib_Free(
        const unsigned int AreaId);


/*----------------------------------------------------------------------------
 * WarmBootLib_Write
 *
 * This function writes data to the storage area described by
 * the AreaId. It overwrites the data starting at ByteOffset and with size
 * ByteCount. Other bytes stored in this area are unchanged.
 *
 * AreaId (input)
 *     ID of the storage area to write to.
 * Data_p (input)
 *     Pointer to the data to be written.
 * ByteOffset (input)
 *     Offset from the start of the storage area where the first data
 *     byte will be written (ByteOffset == 0 corresponds to start of area).
 * ByteCount (input)
 *     Number of bytes to write.
 *
 * Note: data may be partially written in case of an unexpected reboot.
 *       The last byte will be written last, so it can be used as a flag to
 *       indicate validity.
 *
 * Return value:
 *     true: success.
 *     false: failure
 */
bool
WarmBootLib_Write(
        const unsigned int AreaId,
        const unsigned char * const restrict  Data_p,
        const unsigned int ByteOffset,
        const unsigned int ByteCount);


/*----------------------------------------------------------------------------
 * WarmBootLib_Read
 *
 *
 * This function reads data from the storage area described by
 * the AreaId. It reads the data starting at ByteOffset and with size
 * ByteCount
 *
 * AreaId (input)
 *     ID of the storage area to read from.
 * Data_p (output)
 *     Pointer an area where the read data will be stored.
 * ByteOffset (input)
 *     Offset from the start of the storage area where the first data
 *     byte will be read (ByteOffset == 0 corresponds to start of area).
 * ByteCount (input)
 *     Number of bytes to read.
 *
 * Return value:
 *     true: success.
 *     false: failure
 */
bool
WarmBootLib_Read(
        const unsigned int AreaId,
        unsigned char * const restrict Data_p,
        const unsigned int ByteOffset,
        const unsigned int ByteCount);


#endif /* INCLUDE_GUARD_ADAPTER_WARMBOOT_SUPPORT_H */


/* end of file adapter_warmboot_support.h */
