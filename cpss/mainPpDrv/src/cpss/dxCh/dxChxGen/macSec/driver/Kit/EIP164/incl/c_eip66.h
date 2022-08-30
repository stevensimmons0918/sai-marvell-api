/* c_eip66.h
 *
 * EIP-6x Driver Library Default Configuration.
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

#ifndef C_EIP66_H_
#define C_EIP66_H_


/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

/* Top-level configuration */
#include <Kit/DriverFramework/incl/cs_eip66.h>

/* Base offset used for all the EIP-66 registers */
#ifndef EIP66_CONF_BASE
#define EIP66_CONF_BASE                0x00
#endif /* EIP66_CONF_BASE */


#endif /* C_EIP66_H_ */


/* end of file c_eip66.h */
