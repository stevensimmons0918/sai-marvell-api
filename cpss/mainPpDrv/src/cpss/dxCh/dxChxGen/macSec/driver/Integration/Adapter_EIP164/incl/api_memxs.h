/* api_memxs.h
 *
 * Low-level Memory Access (MemXS) API
 *
 * This API should be used for debugging purposes only.
 *
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


#ifndef API_MEMXS_H_
#define API_MEMXS_H_


/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

/* Driver Framework Basic Definitions API */
#include <Kit/DriverFramework/incl/basic_defs.h>


/*----------------------------------------------------------------------------
 * Definitions and macros
 */

typedef enum
{
    MEMXS_STATUS_OK,              /* Operation is successful */
    MEMXS_INVALID_PARAMETER,      /* Invalid input parameter */
    MEMXS_UNSUPPORTED_FEATURE,    /* Feature is not implemented */
    MEMXS_ERROR                   /* Operation has failed */
} MemXS_Status_t;

/*----------------------------------------------------------------------------
 * MemXS_Handle_t
 *
 * This handle is a reference to a memory resource.
 *
 * The handle is set to NULL when MemXS_Handle_t handle.p is equal to NULL.
 *
 */
typedef struct
{
    void * p;
} MemXS_Handle_t;

/*----------------------------------------------------------------------------
 * MemXS_DevInfo_t
 *
 * Memory device data structure.
 *
 */
typedef struct
{
    /* Memory device handle */
    MemXS_Handle_t Handle;

    /* Device name (zero-terminated string) */
    const char * Name_p;

    /* Device index */
    unsigned int Index;

    /* Device start offset */
    unsigned int FirstOfs;

    /* Device end offset */
    unsigned int LastOfs;

} MemXS_DevInfo_t;

/*----------------------------------------------------------------------------
 * MemXS_NULLHandle
 *
 * This handle can be assigned to a variable of type MemXS_Handle_t.
 *
 */
extern const MemXS_Handle_t MemXS_NULLHandle;


/*----------------------------------------------------------------------------
 * MemXS_Init
 *
 * An application must call this API once prior to using any other API
 * functions. During this call the service ensures that the Additional Address
 * Space and Device are accessible for read/write operations.
 *
 * Return value:
 *      true:  Initialization was successful, the other APIs can now be used.
 *      false: Initialization failed, the other APIs cannot be used.
 */
bool
MemXS_Init (void);


/*----------------------------------------------------------------------------
 * MemXS_Handle_IsSame
 *
 * Check whether provided Handle1 is equal to provided Handle2.
 *
 * Handle1_p
 *      First handle
 *
 * Handle2_p
 *      Second handle
 *
 * Return values
 *      true:  provided handles are equal
 *      false: provided handles are not equal
 *
 */
bool
MemXS_Handle_IsSame(
        const MemXS_Handle_t * const Handle1_p,
        const MemXS_Handle_t * const Handle2_p);


/*-----------------------------------------------------------------------------
 * MemXS_Device_Count_Get
 *
 * Returns the number of device which memory (MMIO, internal RAM) can be
 * accessed via this API.
 *
 * DeviceCount_p (output)
 *      Pointer to a memory location where the number of supported devices
 *      will be stored.
 *
 * Return:
 *     MEMXS_STATUS_OK
 *     MEMXS_INVALID_PARAMETER
 *     MEMXS_ERROR
 */
MemXS_Status_t
MemXS_Device_Count_Get(
        unsigned int * const DeviceCount_p);


/*-----------------------------------------------------------------------------
 * MemXS_Device_Info_Get
 *
 * Returns the data structure that describes the device identified by
 * DeviceIndex parameter.
 *
 * DeviceIndex (input)
 *      Device index, from 0 to the number obtained via
 *      the MemXS_Device_Count_Get() function.
 *
 * DeviceInfo_p (output)
 *      Pointer to a memory location where the number of supported devices
 *      will be stored.
 *
 * Return:
 *     MEMXS_STATUS_OK
 *     MEMXS_INVALID_PARAMETER
 *     MEMXS_ERROR
 */
MemXS_Status_t
MemXS_Device_Info_Get(
        const unsigned int DeviceIndex,
        MemXS_DevInfo_t * const DeviceInfo_p);


/*----------------------------------------------------------------------------
 * MemXS_Read32
 *
 * This function can be used to read one static 32bit resource inside a device
 * (typically a register or memory location). Since reading registers can have
 * side effects, the implementation must guarantee that the resource will be
 * read only once and no neighboring resources will be accessed.
 *
 * If required (decided based on internal configuration), on the fly endianness
 * swapping of the value read will be performed before it is returned to the
 * caller.
 *
 * Handle (input)
 *     Handle for the device instance which memory must be read.
 *
 * ByteOffset (input)
 *     The byte offset within the device for the resource to read.
 *
 * Return Value
 *     The value read.
 *
 * When the Handle or Offset parameters are invalid, the implementation will
 * return an unspecified value.
 */
uint32_t
MemXS_Read32(
        const MemXS_Handle_t Handle,
        const unsigned int ByteOffset);


/*----------------------------------------------------------------------------
 * MemXS_Write32
 *
 * This function can be used to write one static 32bit resource inside a
 * device (typically a register or memory location). Since writing registers
 * can have side effects, the implementation must guarantee that the resource
 * will be written exactly once and no neighboring resources will be
 * accessed.
 *
 * If required (decided based on internal configuration), on the fly endianness
 * swapping of the value to be written will be performed.
 *
 * Handle (input)
 *     Handle for the device instance which memory must be read.
 *
 * ByteOffset (input)
 *     The byte offset within the device for the resource to write.
 *
 * Value (input)
 *     The 32bit value to write.
 *
 * Return Value
 *     None
 *
 * The write can only be successful when the Handle and ByteOffset parameters
 * are valid.
 */
void
MemXS_Write32(
        const MemXS_Handle_t Handle,
        const unsigned int ByteOffset,
        const uint32_t Value);


/*----------------------------------------------------------------------------
 * MemXS_Read32Array
 *
 * This function perform the same task as MemXS_Read32 for an array of
 * consecutive 32bit words, allowing the implementation to use a more optimal
 * burst-read (if available).
 *
 * See MemXS_Read32 for pre-conditions and a more detailed description.
 *
 * Handle (input)
 *     Handle for the device instance which memory must be read.
 *
 * StartByteOffset (input)
 *     Byte offset of the first resource to read.
 *     This value is incremented by 4 for each following resource.
 *
 * MemoryDst_p (output)
 *     Pointer to the memory where the retrieved words will be stored.
 *
 * Count (input)
 *     The number of 32bit words to transfer.
 *
 * Return Value
 *     None.
 */
void
MemXS_Read32Array(
        const MemXS_Handle_t Handle,
        const unsigned int StartByteOffset,
        uint32_t * MemoryDst_p,
        const int Count);


/*----------------------------------------------------------------------------
 * MemXS_Write32Array
 *
 * This function perform the same task as MemXS_Write32 for an array of
 * consecutive 32bit words, allowing the implementation to use a more optimal
 * burst-write (if available).
 *
 * See MemXS_Write32 for pre-conditions and a more detailed description.
 *
 * Handle (input)
 *     Handle for the device instance which memory must be read.
 *
 * StartByteOffset (input)
 *     Byte offset of the first resource to write.
 *     This value is incremented by 4 for each following resource.
 *
 * MemorySrc_p (input)
 *     Pointer to the memory where the values to be written are located.
 *
 * Count (input)
 *     The number of 32bit words to transfer.
 *
 * Return Value
 *     None.
 */
void
MemXS_Write32Array(
        const MemXS_Handle_t Handle,
        const unsigned int StartByteOffset,
        const uint32_t * MemorySrc_p,
        const int Count);


#endif /* API_MEMXS_H_ */


/* end of file api_memxs.h */
