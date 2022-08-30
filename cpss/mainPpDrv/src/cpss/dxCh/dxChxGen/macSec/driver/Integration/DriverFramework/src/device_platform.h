/* device_platform.h
 *
 * Driver Framework platform-specific interface,
 * Linux user-space UMDevXS.
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

#ifndef DEVICE_PLATFORM_H_
#define DEVICE_PLATFORM_H_

/*----------------------------------------------------------------------------
 * This module implements (provides) the following interface(s):
 */



/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */


/*----------------------------------------------------------------------------
 * Definitions and macros
 */

/* Internal global device administration data */
typedef struct
{
    unsigned int Reserverd; /* not used */

} Device_Platform_Global_t;

/* Internal device administration data */
typedef struct
{
    /* Pointer to mapped area */
    uint32_t * Mem32_p;

    /* Offsets w.r.t. mapped area */
    unsigned int FirstOfs;
    unsigned int LastOfs;

} Device_Platform_t;


/*----------------------------------------------------------------------------
 * Local variables
 */


#endif /* DEVICE_PLATFORM_H_ */


/* end of file device_platform.h */

