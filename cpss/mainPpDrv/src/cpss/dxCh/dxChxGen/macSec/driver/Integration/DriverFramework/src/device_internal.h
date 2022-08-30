/* device_internal.h
 *
 * Driver Framework Device Internal interface.
 *
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

#ifndef DEVICE_INTERNAL_H_
#define DEVICE_INTERNAL_H_

/*----------------------------------------------------------------------------
 * This module implements (provides) the following interface(s):
 */


/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

/* Default configuration */
#include <Integration/DriverFramework/src/c_device_generic.h>

/* Driver Framework Device API */
#include <Kit/DriverFramework/incl/device_types.h>           /* Device_Handle_t */

/* Driver Framework Device Platform interface */
#include <Integration/DriverFramework/src/device_platform.h>        /* Device_Platform_* */


/*----------------------------------------------------------------------------
 * Definitions and macros
 */

/* Represents 8 MACSec units per device for total of 128 devices */
#define HWPAL_DEVICE_STATIC_COUNT  (8*128)


/* Global administration data */
typedef struct
{
    /* Initialization flag */
    bool fInitialized;

    Device_Platform_Global_t Platform; /* platform-specific global data */
} Device_Global_Admin_t;

/* Internal statically configured device administration data */
typedef struct
{
    const GT_CHAR          *unitName;
    PRV_CPSS_DXCH_UNIT_ENT unitId;
    GT_U32                 unitOfsEnd;
    GT_U32                 flags; /* swapping, tracing */
} Device_Admin_Static_t;

/* Internal device administration data */
typedef struct
{
    GT_CHAR * DevName;
    GT_U32 DeviceNr;
    GT_U32 FirstOfs;
    GT_U32 LastOfs;
    GT_U32 Flags;    /* swapping, tracing */
    GT_U32 DeviceId; /* index in the device list */
    GT_U8 devNum;    /* device number: Hawk,Phenix... */

} Device_Admin_t;

#define HWPAL_DEVICE_ADD(_name, _unitId, _lastofs, _flags) \
                        {_name, _unitId, _lastofs, _flags}

/*----------------------------------------------------------------------------
 * Local variables
 */


/*----------------------------------------------------------------------------
 * Device_Internal_Static_Count_Get
 *
 * Returns number of statically configured devices in the HWPAL_DEVICES device
 * list.
 *
 */
GT_U32
Device_Internal_Static_Count_Get(void);


/*----------------------------------------------------------------------------
 * Device_Internal_Count_Get
 *
 * Returns number of maximum supported devices in the device list.
 *
 */
GT_U32
Device_Internal_Count_Get(void);


/*----------------------------------------------------------------------------
 * Device_Internal_Admin_Static_Get
 *
 * Returns pointer to the memory location where the statically configured
 * device list is stored.
 *
 */
const Device_Admin_Static_t *
Device_Internal_Admin_Static_Get(void);


/*----------------------------------------------------------------------------
 * Device_Internal_Admin_Get
 *
 * Returns pointer to the memory location where the device list is stored.
 *
 */
Device_Admin_t **
Device_Internal_Admin_Get(void);


/*----------------------------------------------------------------------------
 * Device_Internal_Admin_Global_Get
 *
 * Returns pointer to the memory location where the global device
 * administration data is stored.
 *
 */
Device_Global_Admin_t *
Device_Internal_Admin_Global_Get(void);


/*----------------------------------------------------------------------------
 * Device_Internal_Alloc
 *
 * Returns a pointer to a newly allocated block ByteCount bytes long, or a null
 * pointer if the block could not be allocated.
 *
 */
void *
Device_Internal_Alloc(
        GT_U32 ByteCount);


/*----------------------------------------------------------------------------
 * Device_Internal_Free
 *
 * Deallocates the block of memory pointed at by Ptr.
 *
 */
void
Device_Internal_Free(
        void * Ptr);


/*----------------------------------------------------------------------------
 * Device_Internal_Initialize
 *
 * See Device_Initialize() description.
 *
 */
GT_32
Device_Internal_Initialize(
        void * CustomInitData_p);


/*----------------------------------------------------------------------------
 * Device_Internal_UnInitialize
 *
 * See Device_UnInitialize() description.
 *
 */
void
Device_Internal_UnInitialize(void);


/*-----------------------------------------------------------------------------
 * Device_Internal_Find
 *
 * See Device_Find() description.
 *
 * Index (input)
 *      Device  index in the device list.
 *
 */
Device_Handle_t
Device_Internal_Find(
        const GT_CHAR * DeviceName_p,
        const GT_U32 Index);


/*-----------------------------------------------------------------------------
 * Device_Internal_GetIndex
 *
 * See Device_GetIndex() description.
 *
 */
GT_32
Device_Internal_GetIndex(
        const Device_Handle_t Device);





#endif /* DEVICE_INTERNAL_H_ */


/* end of file device_internal.h */

