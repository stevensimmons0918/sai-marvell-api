/* c_list.h
 *
 * Default configuration for the List module
 */

/*****************************************************************************
* Copyright (c) 2012-2013 INSIDE Secure B.V. All Rights Reserved.
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

#ifndef C_LIST_H_
#define C_LIST_H_


/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

/* Top-level configuration */
#include <Kit/DriverFramework/incl/cs_list.h>


/*----------------------------------------------------------------------------
 * Definitions and macros
 */

/* Maximum supported number of the list instances */
#ifndef LIST_MAX_NOF_INSTANCES
#define LIST_MAX_NOF_INSTANCES      2
#endif /* LIST_MAX_NOF_INSTANCES */

/* Strict argument checking */
/*#define LIST_STRICT_ARGS */


#endif /* C_LIST_H_ */


/* end of file c_list.h */
