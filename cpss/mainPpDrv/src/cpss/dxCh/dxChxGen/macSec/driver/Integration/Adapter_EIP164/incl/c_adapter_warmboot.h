/* c_adapter_warmboot.h
 *
 * Default WarmBoot Adapter Module Configuration
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

#ifndef C_ADAPTER_WARMBOOT_H_
#define C_ADAPTER_WARMBOOT_H_

/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

/* Top-level Adapter configuration */
#include <Kit/DriverFramework/incl/cs_adapter.h>


/*----------------------------------------------------------------------------
 * Definitions and macros
 */
/* Define this to include WarmBoot functionality. */
/*#define ADAPTER_WARMBOOT_ENABLE */

/* Is CfYE Adapter present? */
/*#define ADAPTER_WARMBOOT_CFYE_ENABLE */

#endif /* C_ADAPTER_WARMBOOT_H_ */


/* end of file c_adapter_warmboot.h */
