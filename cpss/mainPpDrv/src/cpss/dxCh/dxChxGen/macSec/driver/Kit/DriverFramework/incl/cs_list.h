/* cs_list.h
 *
 * Top-level configuration for the List module
 */

/*****************************************************************************
* Copyright (c) 2015 INSIDE Secure B.V. All Rights Reserved.
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

#ifndef CS_LIST_H_
#define CS_LIST_H_


/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

/* Top-level configuration */
#include <Kit/DriverFramework/incl/cs_driver.h>


/*----------------------------------------------------------------------------
 * Definitions and macros
 */

/* Maximum supported number of the list instances */
#ifdef DRIVER_LIST_MAX_NOF_INSTANCES
#define LIST_MAX_NOF_INSTANCES      DRIVER_LIST_MAX_NOF_INSTANCES
#endif

/* Strict argument checking */
#define LIST_STRICT_ARGS

/* Use all API functions. */
#define LIST_FULL_API

#endif /* CS_LIST_H_ */


/* end of file cs_list.h */
