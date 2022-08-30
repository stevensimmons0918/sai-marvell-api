/* c_eip164.h
 *
 * EIP-164 Driver Library Default Configuration.
 */

/* -------------------------------------------------------------------------- */
/*                                                                            */
/*   Module        : ddk164                                                   */
/*   Version       : 2.5                                                      */
/*   Configuration : DDK-164                                                  */
/*                                                                            */
/*   Date          : 2020-Feb-27                                              */
/*                                                                            */
/* Copyright (c) 2008-2020 INSIDE Secure B.V. All Rights Reserved             */
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

#ifndef C_EIP164_H_
#define C_EIP164_H_


/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

/* Top-level configuration */
#include <Kit/DriverFramework/incl/cs_eip164.h>

/* One device instance (core) mode of operation */
/*#define EIP164_DEVICE_MODE_EGRESS */
/*#define EIP164_DEVICE_MODE_INGRESS */

/* Base offset used for all the EIP-164 registers */
#ifndef EIP164_CONF_BASE
#define EIP164_CONF_BASE                    0x00
#endif /* EIP164_CONF_BASE */

/* Number of read operations required for the 0 unsafe counter */
/* for proper CDS */
#ifndef EIP164_SECY_MAX_SYNC_COUNT
#define EIP164_SECY_MAX_SYNC_COUNT          3
#endif

/* Maximum value for crypt-authenticate bypass length */
#ifndef EIP164_SECY_CRYPT_AUTH_MAX_BYPASS_LEN
#define EIP164_SECY_CRYPT_AUTH_MAX_BYPASS_LEN            255
#endif

/* Enable PACK timeout counter of statistics modules, */
/* granting the host access after a certain number of cycles. */
#ifndef EIP164_PACK_TIMEOUT_ENABLE
#define EIP164_PACK_TIMEOUT_ENABLE          1
#endif

/* Number of cycles for PACK timeout. */
#ifndef EIP164_PACK_TIMEOUT_VALUE
#define EIP164_PACK_TIMEOUT_VALUE           1
#endif


/* Enable EIP-164 Crypt-Authenticate feature */
/*#define EIP164_CRYPT_AUTH_ENABLE */

/* Strict argument checking for the input parameters */
/* If required then define this parameter in the top-level configuration */
/*#define EIP164_STRICT_ARGS */

/* Enable per-counter summary */
/* #define EIP164_PERCOUNTER_SUMMARY_ENABLE */

/* Disable EIP-164 device initialization register writes, */
/* this can be used to speed up initialization by offloading these register */
/* writes to another processor with lower EIP-164 register write overhead */
/*#define EIP164_DEV_INIT_DISABLE */


#endif /* C_EIP164_H_ */


/* end of file c_eip164.h */
