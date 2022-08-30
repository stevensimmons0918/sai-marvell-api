/* adapter_interrupts_ext.h
 *
 * Host hardware specific extensions for the Adapter Interrupts interface
 * EIP-164 hardware
 */

/*****************************************************************************
* Copyright (c) 2012-2019 INSIDE Secure B.V. All Rights Reserved.
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

#ifndef ADAPTER_INTERRUPTS_EXT_H_
#define ADAPTER_INTERRUPTS_EXT_H_

/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */


/*----------------------------------------------------------------------------
 * Definitions and macros
 */

/* Adapter logical EIP-164 SecY Device interrupts */
/* Each EIP-164 device has just one physical interrupt output line */
/* and each device has its own EIP-201 Advanced Interrupt Controller */

#define IRQ_LAST_LINE   1

int
Adapter_Interrupt_EnabledStatus_Get(const int nIRQ, unsigned int *Status_p);


#endif /* ADAPTER_INTERRUPTS_EXT_H_ */


/* end of file adapter_interrupts_ext.h */
