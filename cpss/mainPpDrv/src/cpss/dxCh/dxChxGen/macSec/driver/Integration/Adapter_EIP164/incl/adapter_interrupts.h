/* adapter_interrupts.h
 *
 * Adapter Interrupts interface
 */

/*****************************************************************************
* Copyright (c) 2010-2019 INSIDE Secure B.V. All Rights Reserved.
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

#ifndef INCLUDE_GUARD_ADAPTER_INTERRUPTS_H
#define INCLUDE_GUARD_ADAPTER_INTERRUPTS_H

/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

/* Driver Framework Basic Defs API */
#include <Kit/DriverFramework/incl/basic_defs.h>     /* bool */


/*----------------------------------------------------------------------------
 * Definitions and macros
 */

typedef void (* Adapter_InterruptHandler_t)(const int nIRQ, const unsigned int Flags);


/*----------------------------------------------------------------------------
 *                           Adapter_Interrupts
 *----------------------------------------------------------------------------
 */

int
Adapter_Interrupts_Init(
        const int nIRQ);

int
Adapter_Interrupts_UnInit(
        const int nIRQ);

int
Adapter_Interrupt_SetHandler(
        const int nIRQ,
        Adapter_InterruptHandler_t HandlerFunction);

int
Adapter_Interrupt_Enable(
        const int nIRQ,
        const unsigned int Flags);

/*
 * This function enables execution context (allows it to run) that services
 * the nIRQ interrupt.
 *
 * Note: this function is different from the Adapter_Interrupt_Enable()
 *       function which only enables the nIRQ interrupt event at an interrupt
 *       controller. The interrupt can be dispatched to some execution context
 *       (for example, a thread) that can be also enabled or disabled.
 */
int
Adapter_Interrupt_Context_Enable(
        const int nIRQ,
        const unsigned int Flags);

int
Adapter_Interrupt_Clear(
        const int nIRQ,
        const unsigned int Flags);

int
Adapter_Interrupt_ClearAndEnable(
        const int nIRQ,
        const unsigned int Flags);

int
Adapter_Interrupt_Disable(
        const int nIRQ,
        const unsigned int Flags);

/*
 * This function disables execution context (allows it to run) that services
 * the nIRQ interrupt.
 *
 * Note: this function is different from the Adapter_Interrupt_Disable()
 *       function which only disables the nIRQ interrupt event at an interrupt
 *       controller. The interrupt can be dispatched to some execution context
 *       (for example, a thread) that can be also enabled or disabled.
 */
int
Adapter_Interrupt_Context_Disable(
        const int nIRQ,
        const unsigned int Flags);

int
Adapter_Interrupts_Resume(void);


/* Host hardware specific extensions */
#include <Integration/Adapter_EIP164/incl/adapter_interrupts_ext.h>


#endif /* Include Guard */


/* end of file adapter_interrupts.h */
