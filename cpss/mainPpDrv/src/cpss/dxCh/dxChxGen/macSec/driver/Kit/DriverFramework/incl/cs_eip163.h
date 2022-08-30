/** @file cs_eip163.h
 *
 * @brief EIP-163 Driver Library Top-Level Configuration.
 */

/* -------------------------------------------------------------------------- */
/*                                                                            */
/*   Module        : ddk164                                                   */
/*   Version       : 2.4                                                      */
/*   Configuration : DDK-164                                                  */
/*                                                                            */
/*   Date          : 2019-Oct-10                                              */
/*                                                                            */
/* Copyright (c) 2008-2019 INSIDE Secure B.V. All Rights Reserved             */
/*                                                                            */
/* This confidential and proprietary software may be used only as authorized  */
/* by a licensing agreement from INSIDE Secure.                               */
/*                                                                            */
/* The entire notice above must be reproduced on all authorized copies that   */
/* may only be made to the extent permitted by a licensing agreement from     */
/* INSIDE Secure.                                                             */
/*                                                                            */
/* For more information or support, please go to our online support system at */
/* https://customersupport.insidesecure.com.                                  */
/* In case you do not have an account for this system, please send an e-mail  */
/* to ESSEmbeddedHW-Support@insidesecure.com.                                 */
/* -------------------------------------------------------------------------- */

#ifndef CS_EIP163_H_
#define CS_EIP163_H_


/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

/* Top-level driver configuration */
#include <Kit/DriverFramework/incl/cs_driver.h>

#include <Kit/EIP163/incl/eip163_hw_interface.h>

/*----------------------------------------------------------------------------
 * Definitions and macros
 */

/** EIP-163 Egress device mode */
#ifdef DRIVER_MODE_EGRESS
#define EIP163_DEVICE_MODE_EGRESS
#endif

/** EIP-163 Ingress device mode */
#ifdef DRIVER_MODE_INGRESS
#define EIP163_DEVICE_MODE_INGRESS
#endif

/* Base offset used for all the EIP-163 registers */
/*#define EIP163_CONF_BASE                    0x80000 */

/** Enables strict argument checking for the input parameters in all the
    EIP-163 Driver API functions */
#if !defined DRIVER_PERFORMANCE || defined ONLY_FOR_DOXYGEN
#define EIP163_STRICT_ARGS
#endif

/** Enable PACK timeout counter of statistics modules,
    granting the host access after a certain number of cycles. */
#define EIP163_PACK_TIMEOUT_ENABLE          1

/** Number of cycles for PACK timeout. */
#define EIP163_PACK_TIMEOUT_VALUE           1


#ifdef ONLY_FOR_DOXYGEN

/** If this configuration option is set, the strict argument checking of the
    input parameters is enabled and can detect some programming errors.
    If not set, the performance is increased and code size is reduced. */
#define EIP163_CONF_BASE                    0x00

/** Number of read operations required for the 0 unsafe counter for proper CDS */
#define EIP163_MAX_SYNC_COUNT               3

/** This configuration option sets (defines) the base offset for all the EIP-217
    registers. If not set, the default will be 0. */
#define EIP217_CONF_BASE

#endif /* ONLY_FOR_DOXYGEN */


/** Number of non-control words in each TCAM entry. The actual width of
    TCAM words may be larger than what is required by the rules.*/
#define EIP163_TCAM_NON_CTRL_WORD_COUNT 4

/* End address of EIP-163 HW registers area - defined by AC5X address space */
#define EIP163_REG_IF_END (0x1FFE4)

#endif /* CS_EIP163_H_ */


/* end of file cs_eip163.h */
