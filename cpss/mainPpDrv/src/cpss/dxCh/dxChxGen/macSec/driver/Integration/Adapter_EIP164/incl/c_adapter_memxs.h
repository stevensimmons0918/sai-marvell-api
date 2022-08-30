/* c_adapter_memxs.h
 *
 * Default MemXS Adapter Module Configuration
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

#ifndef C_ADAPTER_MEMXS_H_
#define C_ADAPTER_MEMXS_H_

/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

/* Top-level Driver Framework configuration */
#include <Kit/DriverFramework/incl/cs_hwpal.h>


/*----------------------------------------------------------------------------
 * Definitions and macros
 */

/* Here is the dependency on the Driver Framework configuration */
/* via the MemXS configuration */
#ifndef HWPAL_DEVICES
#error "Expected HWPAL_DEVICES defined by cs_memxs.h"
#endif


#endif /* C_ADAPTER_MEMXS_H_ */


/* end of file c_adapter_memxs.h */

