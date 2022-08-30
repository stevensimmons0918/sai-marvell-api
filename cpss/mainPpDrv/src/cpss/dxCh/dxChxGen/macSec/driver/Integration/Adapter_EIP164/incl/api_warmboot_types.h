/** @file  api_warmboot_types.h
 *
 * @brief Warm Boot API, implementation-specific device types.
 */

/*****************************************************************************
* Copyright (c) 2016-2017 INSIDE Secure B.V. All Rights Reserved.
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

#ifndef API_WARMBOOT_TYPES_H_
#define API_WARMBOOT_TYPESH_

/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */


/*----------------------------------------------------------------------------
 * Definitions and macros
 */


/** Type describing the device */
typedef enum
{
    WARMBOOT_DEVICE_TYPE_SECY_E,
    WARMBOOT_DEVICE_TYPE_SECY_I,
    WARMBOOT_DEVICE_TYPE_CFYE_E,
    WARMBOOT_DEVICE_TYPE_CFYE_I,
    WARMBOOT_DEVICE_TYPE_DF,
} WarmBoot_DeviceType_t;


#endif /* API_WARMBOOT_TYPESH_ */


/* end of file api_warmboot_types.h */
