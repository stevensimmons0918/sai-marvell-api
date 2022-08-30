/* cs_intdispatch_umdevxs.h
 *
 * Configuration Settings for the Interrupt Dispatcher.
 */

/*****************************************************************************
* Copyright (c) 2015-2017 INSIDE Secure B.V. All Rights Reserved.
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

#ifndef INCLUDE_GUARD_CS_INTDISPATCH_UMDEVXS_H
#define INCLUDE_GUARD_CS_INTDISPATCH_UMDEVXS_H

#include <Kit/DriverFramework/incl/cs_driver.h>

/* logging level for Interrupt Dispatcher */
/* Choose from LOG_SEVERITY_INFO, LOG_SEVERITY_WARN, LOG_SEVERITY_CRIT */
#undef LOG_SEVERITY_MAX
#ifdef DRIVER_PERFORMANCE
#define INTDISPATCH_LOG_SEVERITY  LOG_SEVERITY_CRITICAL
#else
#define INTDISPATCH_LOG_SEVERITY  LOG_SEVERITY_WARN
#endif

#if defined(DRIVER_MAX_NOF_EIP163_IC_DEVICES) && defined(DRIVER_MAX_NOF_EIP164_IC_DEVICES)
#define INTDISPATCH_MAX_NOF_RESOURCES   (DRIVER_MAX_NOF_EIP163_IC_DEVICES + \
                                         DRIVER_MAX_NOF_EIP164_IC_DEVICES)
#else
#error "DRIVER_MAX_NOF_EIP16[3|4]_IC_DEVICES not defined"
#endif

/* Calculate AIC device ID from its parent EIP-164 device ID */
#define INTDISPATCH_ICDEV_ID         DRIVER_ICDEV_ID

/* Calculate EIP-164 interrupt handler ID from EIP-201 AIC device ID */
#define INTDISPATCH_INT_HANDLER_ID   DRIVER_INT_HANDLER_ID


#endif /* Include Guard */


/* end of file cs_intdispatch_umdevxs.h */
