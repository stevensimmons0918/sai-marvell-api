/* cs_eip66.h
 *
 * Top-level configuration parameters
 * for the EIP-6X Driver Library
 *
 */

/*****************************************************************************
* Copyright (c) 2016 INSIDE Secure B.V. All Rights Reserved.
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

#ifndef CS_EIP66_H_
#define CS_EIP66_H_


/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */
#include <Kit/DriverFramework/incl/cs_driver.h>

/*----------------------------------------------------------------------------
 * Definitions and macros
 */

/* Base offset used for all the EIP-6X registers */
#define EIP66_CONF_BASE 0x5800

/* Start address of EIP-66 Interrupt Controller  */
#define EIP66_AIC_START 0x5C00

/* End address of EIP-66 Interrupt Controller    */
#define EIP66_AIC_END  (EIP66_AIC_START + 0x3f)


#endif /* CS_EIP66_H_ */


/* end of file cs_eip66.h */
