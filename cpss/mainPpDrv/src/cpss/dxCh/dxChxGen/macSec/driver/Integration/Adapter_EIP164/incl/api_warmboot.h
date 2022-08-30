/** @file api_warmboot.h
 *
 * @brief Warm Boot API
 *
 * This API implements Warm Boot functionality, this is saving the
 * state of the the software driver in order to restart the CPU while
 * the hardware is still configured and running and restoring the
 * driver's state to a saved state before the CPU was restarted.
 */

/*****************************************************************************
* Copyright (c) 2016-2019 INSIDE Secure B.V. All Rights Reserved.
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

#ifndef API_WARMBOOT_H_
#define API_WARMBOOT_H_

/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

#include <Integration/Adapter_EIP164/incl/api_warmboot_types.h> /* WarmBoot_DeviceType_t; */

#define restrict  /* restrict keyword may not be accepted by some compilers. */

/**---------------------------------------------------------------------------
 * Assumptions about the use of the WarmBoot API.
 *
 * 1 Non-volatile memory (NVM) content: constants and code
 *   segments. The reboot does not change the content of these
 *   segments.
 * 2 Volatile memory (VM) content: heap, stack, bss segments. The
 *   reboot erases the content of these segments. The driver is responsible
 *   for preserving and restoring all essential data that is stored in
 *   these segments via the WarmBoot callbacks. It is the responsibility of
 *   the OS (or application) to preserve whatever data was saved through
 *   the callback functions.
 *   Data object addresses are not guaranteed to be the same after a reboot,
 *   hence it is not allowed to store values of pointers to data objects
 *   in the saved state.
 * 3 Instruction pointer. The reboot restores it not to a default
 *   reset value but to some warm boot value. This eventually causes
 *   the application to call the WarmBoot_Restore() function of the
 *   driver. The exact mechanism is out of the scope of the driver.
 * 4 OS (or application) and other drivers are just as much
 *   responsible for restoring their software state as the driver is,
 *   so the driver does not have to be re-initialized in any way other
 *   than via the WB API to help the OS and the other drivers to
 *   restore their state which they have to do via some sort of WB
 *   API; and the hardware does not need to be re-initialized because
 *   it is supposed to keep its state during the host processor reset.
 *
 * Note:
 * Various device driver APIs return allocated items via handles.
 * Such handles do not survive a warm boot correctly. Index values
 * will survive warm boots. Therefore the application must use the
 * appropriate *_Index_Get() to convert any handles to index values if
 * they need to survive a warm boot. After the warm boot it can use
 * *_Handle_Get() functions to convert them back to handles.
 *
 * API Use Order:
 *
 * After a cold boot before initializing the driver. No valid data items as
 * allocated by this API shall exist immediately after a cold boot.
 * - optionally use WarmBoot_MaxSize_Get() to obtain the maximum size per data
 *   item.
 * - WarmBoot_Register()
 *        to register callbacks with the API.
 * When the driver is initialized and runs, it uses the provided callback
 * functions to allocate space to store driver state and to write updated
 * driver state. The callback functions are responsible for maintaining
 * the stored state in a memory that will be preserved when the CPU is
 * restarted.
 *
 * Warm boot: the hardware device remains operational, while the CPU running
 * the software is restarted. This restart can be either planned or unexpected.
 *
 * For a planned warm boot, the application is supposed to call
 * WarmBoot_Shutdown() for each of the data items allocated by the
 * driver through this API. This allows the driver to relinquish
 * software resources without shutting down the hardware devices, so
 * the CPU can restart in an orderly manner.
 * For an unexpected warm boot, WarmBoot_Shutdown() will not be called.
 *
 * After a warm boot:
 * - WarmBoot_Register()
 *        to register callbacks with the API.
 * - For each of the data items allocated by the driver through this API:
 *   - WarmBoot_Restore()
 *        to restore all driver internal state, which corresponds to the
 *        state of the running hardware.
 * No other driver API calls are permitted before the last WarmBoot_Restore()
 * call returns.
 */

/*----------------------------------------------------------------------------
 * Definitions and macros
 */

/** Status returned by each of the functions in this API*/
enum
{
    WARMBOOT_STATUS_OK,
    WARMBOOT_ERROR_BAD_PARAMETER,
    WARMBOOT_ERROR_ALLOCATION,
    WARMBOOT_INTERNAL_ERROR,
};
/** Return type for WarmBoot API functions. above enum values and any other
    integer values for device read/write errors */
typedef int WarmBoot_Status_t;


/**---------------------------------------------------------------------------
 * @typedef WarmBoot_Alloc_Callback_t
 *
 * This callback function allocates a storage area of the desired size in
 * a memory area that will be preserved across CPU reboots. The allocated
 * area is filled entirely with bytes of value zero. The returned AreaId
 * represents a valid storage area. Each allocated (and not yet freed) storage
 * area has a unique AreaId.
 *
 * @param [in] StorageByteCount
 *     Size of the storage area in bytes.
 *
 * @param [out] AreaId_p
 *     ID of the allocated storage area.
 *
 * @return WARMBOOT_STATUS_OK : success
 * @return WARMBOOT_ERROR_BAD_PARAMETER : incorrect input parameter. AreaId_p is
 * @return NULL or StorageByteCount exceeds area size limit imposed by system.
 * @return WARMBOOT_ERROR_ALLOCATION : no space available for the requested
 *         storage area.
 * @return WARMBOOT_INTERNAL_ERROR : other failure
 */
typedef WarmBoot_Status_t (*WarmBoot_Alloc_Callback_t)(
        const unsigned int StorageByteCount,
        unsigned int * const AreaId_p);


/**---------------------------------------------------------------------------
 * @typedef WarmBoot_Free_Callback_t
 *
 * This callback function frees a storage area previously allocated by
 * the WarnBoot_Alloc callback.  After this call, AreaId no longer corresponds
 * to a valid storage area and it may no longer be passed to callbacks of this
 * API. Future calls to the WarmBoot_Alloc callback may re-use this AreaId.
 *
 * @param [in] AreaId
 *     ID of the storage area to be freed.
 *
 * @return WARMBOOT_STATUS_OK : success
 * @return WARMBOOT_ERROR_BAD_PARAMETER : incorrect input parameter. AreaId does
 *         not represent a valid storage area.
 * @return WARMBOOT_INTERNAL_ERROR : other failure
 */
typedef WarmBoot_Status_t (*WarmBoot_Free_Callback_t)(
        const unsigned int AreaId);


/**---------------------------------------------------------------------------
 * @typedef WarmBoot_Write_Callback_t
 *
 * This callback function writes data to the storage area described by
 * the AreaId. It overwrites the data starting at ByteOffset and with size
 * ByteCount. Other bytes stored in this area are unchanged.
 *
 * @param [in] AreaId
 *     ID of the storage area to write to.
 * @param [in] Data_p
 *     Pointer to the data to be written.
 * @param [in] ByteOffset
 *     Offset from the start of the storage area where the first data
 *     byte will be written (ByteOffset == 0 corresponds to start of area).
 * @param [in] ByteCount
 *     Number of bytes to write.
 *
 * Note: data may be partially written in case of an unexpected reboot.
 *       The last byte will be written last, so it can be used as a flag to
 *       indicate validity.
 *
 * Return value:
 * @return WARMBOOT_STATUS_OK : success
 * @return WARMBOOT_ERROR_BAD_PARAMETER : incorrect input parameter. AreaId does
 *         not represent a valid storage area, Data_p is NULL,
 *         ByteOffset + ByteCount greater than area size.
 * @return WARMBOOT_INTERNAL_ERROR : other failure
 */
typedef WarmBoot_Status_t (*WarmBoot_Write_Callback_t)(
        const unsigned int AreaId,
        const unsigned char * const restrict  Data_p,
        const unsigned int ByteOffset,
        const unsigned int ByteCount);


/**---------------------------------------------------------------------------
 * @typedef WarmBoot_Read_Callback_t
 *
 * This callback function reads data from the storage area described by
 * the AreaId. It reads the data starting at ByteOffset and with size
 * ByteCount
 *
 * @param [in] AreaId
 *     ID of the storage area to read from.
 * @param [out] Data_p
 *     Pointer an area where the read data will be stored.
 * @param [in] ByteOffset
 *     Offset from the start of the storage area where the first data
 *     byte will be read (ByteOffset == 0 corresponds to start of area).
 * @param [in] ByteCount
 *     Number of bytes to read.
 *
 * Return value:
 * @return WARMBOOT_STATUS_OK : success
 * @return WARMBOOT_ERROR_BAD_PARAMETER : incorrect input parameter. AreaId does
 *         not represent a valid storage area, Data_p is NULL,
 *         ByteOffset + ByteCount greater than area size.
 * @return WARMBOOT_INTERNAL_ERROR : other failure
 */
typedef WarmBoot_Status_t (*WarmBoot_Read_Callback_t)(
        const unsigned int AreaId,
        unsigned char * const restrict Data_p,
        const unsigned int ByteOffset,
        const unsigned int ByteCount);


/**---------------------------------------------------------------------------
 * @fn WarmBoot_MaxSize_Get(
 *       const WarmBoot_DeviceType_t DeviceType,
 *       unsigned int * const restrict MaxByteCount_p);
 *
 * This function computes the maximum amount of storage that will be reserved
 * for each device using the WarmBoot API. If the application must reserve a
 * fixed size storage area before any callback functions can be used, it
 * can use this function.
 *
 * @param [in] DeviceType
 *     Implementation-specific device type.
 *
 * @param [out] MaxByteCount_p
 *     Maximum size of the storage area in bytes that will be reserved
 *     for this device type.
 *
 * @return WARMBOOT_STATUS_OK : success
 * @return WARMBOOT_ERROR_BAD_PARAMETER : incorrect input parameter, any pointer
 *         is NULL.
 */
WarmBoot_Status_t
WarmBoot_MaxSize_Get(
        const WarmBoot_DeviceType_t DeviceType,
        unsigned int * const restrict MaxByteCount_p);


/**---------------------------------------------------------------------------
 * @fn WarmBoot_Register(
 *       WarmBoot_Alloc_Callback_t Alloc_CB,
 *       WarmBoot_Free_Callback_t Free_CB,
 *       WarmBoot_Write_Callback_t Write_CB,
 *       WarmBoot_Read_Callback_t Read_CB);
 *
 * This function must be called prior to any other driver API function.
 * It registers the application-provided callback functions with the driver.
 * This function must be called both after a cold boot (hardware needs
 * to be configured) and after a warm boot (hardware is already configured).
 *
 * @param [in] Alloc_CB
 *     WarmBoot_Alloc function callback.
 * @param [in] Free_CB
 *     WarmBoot_Free function callback. This is allowed to be NULL.
 * @param [in] Write_CB
 *     Warmboot_Write function callback.
 * @param [in] Read_CB
 *     WarmBoot_Read function callback.
 *
 * @return WARMBOOT_STATUS_OK : success
 * @return WARMBOOT_ERROR_BAD_PARAMETER : incorrect input parameter. Any of
 *       the function pointers other than Free_CB is NULL.
 * @return WARMBOOT_INTERNAL_ERROR : other failure
 */
WarmBoot_Status_t
WarmBoot_Register(
        WarmBoot_Alloc_Callback_t Alloc_CB,
        WarmBoot_Free_Callback_t Free_CB,
        WarmBoot_Write_Callback_t Write_CB,
        WarmBoot_Read_Callback_t Read_CB);


/**---------------------------------------------------------------------------
 * @fn WarmBoot_Shutdown(
 *       const unsigned int AreaId);
 *
 * This function can be called as the last driver function just prior
 * to a planned warm boot. For every storage area allocated by a
 * WarmBoot_Alloc callback (and not yet freed by the WarmBoot_Free
 * callback) the application should make a call to WarmBoot_Shutdown (the
 * order in which the calls are made is unspecified). No calls to other
 * driver API functions are allowed after calling this function.
 *
 * The implementation of this function in the Driver will typically free
 * any software resources belonging to the corresponding device, so the
 * system can be shut down in an orderly manner.
 * The implementation of this function in the Driver must not shut down
 * the corresponding hardware device.
 * The implementation of this function in the Driver must not free
 * the storage area allocated through the WarmBoot_Alloc callback.
 *
 * @param [in] AreaId
 *      ID of the storage area containing the driver state information.
 *
 * @return WARMBOOT_STATUS_OK : success
 * @return WARMBOOT_ERROR_BAD_PARAMETER : incorrect input parameter, AreaID is
 *           not the ID of a valid storage area.
 * @return WARMBOOT_INTERNAL_ERROR : failure
 * @return other values: device read/write errors.
 */
WarmBoot_Status_t
WarmBoot_Shutdown(
        const unsigned int AreaId);


/**---------------------------------------------------------------------------
 * WarmBoot_Restore(
 *       const unsigned int AreaId);
 *
 * This function must be called as the first function after
 * WarmBoot_Register after a warm boot. For every storage area
 * allocated by a WarmBoot_Alloc callback (and not yet freed by the
 * WarmBoot_Free callback) the application must make a call to
 * WarmBoot_Restore (the order in which the calls are made is
 * unspecified). The allocated storage areas remain valid after a warm
 * boot and after WarmBoot_Restore returns.
 *
 * The implementation of this function in the Driver will typically
 * use the WarmBoot_Read callback to read (part of) the contents of
 * the storage area and then call a device-specific restore function
 * to restore the state of the driver corresponding to the device
 * whose state was stored in this area.
 *
 * @param [in] AreaId
 *      ID of the storage area containing the driver state information.
 *
 * @return WARMBOOT_STATUS_OK : success
 * @return WARMBOOT_ERROR_BAD_PARAMETER : incorrect input parameter, AreaID is
 *           not the ID of a valid storage area.
 * @return WARMBOOT_INTERNAL_ERROR : failure
 * @return other values: device read/write errors.
 */
WarmBoot_Status_t
WarmBoot_Restore(
        const unsigned int AreaId);


#endif /* API_WARMBOOT_H_ */


/* end of file api_warmboot.h */
