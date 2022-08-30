/** @file api_driver164_init.h
 *
 * @brief SafeXcel-IP-164 MACsec Driver Initialization API.
 *
 * This API specifies the driver entry and exit points.
 *
 * Note: one driver instance can support multiple SafeXcel-IP-164 MACsec
 *       hardware acceleration devices.
 */

/*****************************************************************************
* Copyright (c) 2015-2018 INSIDE Secure B.V. All Rights Reserved.
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

#ifndef DRIVER164_INIT_H_
#define DRIVER164_INIT_H_


/* Driver Framework Basic Definitions API */
#include <Kit/DriverFramework/incl/basic_defs.h>

/*----------------------------------------------------------------------------
 * Definitions and macros
 */

/** Device administration structure */
typedef struct
{
    /** Device name */
    char * DeviceName_p;

    /** Device start offset in the device register range in system memory map */
    unsigned int StartByteOffset;

    /** Device end offset in the device register range in system memory map */
    unsigned int LastByteOffset;

    /** Implementation specific device flags */
    char Flags;

} Driver164_Device_t;


/** Data path administration structure */
typedef struct
{
    /**  Device start offset  inside system memory map of first sub-device
         chain.*/
    unsigned int StartByteOffset1;
    /**  Device last offset  inside system memory map of second sub-device
         chain.*/
    unsigned int LastByteOffset1;

    /**  Device start offset  inside system memory map of first sub-device
         chain.*/
    unsigned int StartByteOffset2;
    /**  Device last offset  inside system memory map of second sub-device
         chain.*/
    unsigned int LastByteOffset2;

    /** Implementation specific device flags for first chain */
    char Flags1;

    /** Implementation specific device flags for second chain */
    char Flags2;

} Driver164_DataPath_t;


/**---------------------------------------------------------------------------
 * @fn Driver164_Init(void)
 *
 * Initialize the driver. This function must be called before any other
 * driver API function can be called.
 *
 * @return 0 : success
 * @return -1: failure
 */
GT_32
Driver164_Init(GT_U8 devNum, GT_U32 unitBmp);


/**---------------------------------------------------------------------------
 * @fn Driver164_Exit(void)
 *
 * Initialize the driver. After this function is called no other driver API
 * function can be called except Driver164_Init().
 */
void
Driver164_Exit(GT_U8 devNum, GT_U32 unitBmp, GT_BOOL lastDevice);


/**---------------------------------------------------------------------------
 * @fn Driver164_Device_Add(
 *     const unsigned int Index,
 *     const Driver164_Device_t * const Device_p)
 *
 * Adds a new device to the driver device list.
 * It can be used as an alternative or in combination with static device list
 * configuration in the driver.
 *
 * @pre This function must be called before any other driver function can
 *      reference this device.
 *
 * @param [in] Index
 *     Device index where the device must be added in the device list
 *
 * @param [in] Device_p
 *     Pointer to memory location where device data is stored, may not be NULL
 *
 * @return 0 : success
 * @return -1: failure
 */
int
Driver164_Device_Add(
        const unsigned int Index,
        const Driver164_Device_t * const Device_p);


/**---------------------------------------------------------------------------
 * @fn Driver164_Device_Remove(
 *     const unsigned int Index)
 *
 * Removes device from the driver device list at the requested index,
 * the device must be previously added either statically or via a call
 * to the Driver164_Device_Add() function.
 *
 * @pre This function must be called when no other driver function can reference
 *      this device.
 *
 * @param [in] Index
 *     Device index where the device must be added in the device list
 *
 * @return 0 : success
 * @return -1: failure
 */
int
Driver164_Device_Remove(
        const unsigned int Index);


/**----------------------------------------------------------------------------
 * @fn Driver164_Device_GetCount(void)
 *
 * This function returns the number of devices present in the device list.
 *
 * @return : device count.
 */
unsigned int
Driver164_Device_GetCount(void);


/**---------------------------------------------------------------------------
 * @fn Driver164_DataPath_Add(
 *       const unsigned int Index,
 *       const Driver164_DataPath_t * const DataPath_p);
 *
 * Adds all devices for a complete data path to the driver device list.
 *
 * This function must be called before any other driver function can reference
 * this data path. It can be used as an alternative or in combination with
 * static device list configuration in the driver.
 *
 * @param [in] Index
 *     Data path index of the data path that must be added in the device list
 *
 * @param [in] DataPath_p
 *     Pointer to memory location where data path properties are stored, may
 *     not be NULL
 *
 * @return 0 : success
 * @return -1: failure
 */
int
Driver164_DataPath_Add(
        const unsigned int Index,
        const Driver164_DataPath_t * const DataPath_p);


/**---------------------------------------------------------------------------
 * @fn Driver164_DataPath_Remove(
 *        const unsigned int Index);
 *
 * Removes all devices of a data path from the driver device list at
 * the requested index, the data path must be previously added either
 * statically or via a call to the Driver164_DataPath_Add() function.
 *
 * This function must be called when no other driver function can reference
 * this device.
 *
 * @param [in] Index
 *     Data path index of the data path that must be removed from the device
 *     list
 *
 * @return 0 : success
 * @return -1: failure
 */
int
Driver164_DataPath_Remove(
        const unsigned int Index);


/**----------------------------------------------------------------------------
 * @fn Driver164_DataPath_GetCount(void)
 *
 * This function returns the number of data paths supported by the Driver.
 *
 * @return: Data path count.
 */
unsigned int
Driver164_DataPath_GetCount(void);


#endif /* DRIVER164_INIT_H_ */


/* end of file api_driver164_init.h */
