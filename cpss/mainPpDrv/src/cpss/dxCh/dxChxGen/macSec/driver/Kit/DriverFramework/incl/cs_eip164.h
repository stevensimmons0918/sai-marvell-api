/** @file cs_eip164.h
 *
 * @brief Top-level configuration parameters
 * for the EIP-164 Driver Library
 *
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

#ifndef CS_EIP164_H_
#define CS_EIP164_H_


/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */
#include <Kit/DriverFramework/incl/cs_driver.h>

#include <Kit/EIP164/incl/eip164_hw_interface.h>

/*----------------------------------------------------------------------------
 * Definitions and macros
 */

/* One device instance (core) mode of operation */
/** EIP-164 Egress device mode */
#ifdef DRIVER_MODE_EGRESS
#define EIP164_DEVICE_MODE_EGRESS
#endif

/** EIP-164 Ingress device mode */
#ifdef DRIVER_MODE_INGRESS
#define EIP164_DEVICE_MODE_INGRESS
#endif

/** If this parameter is defined, the driver will support basic
    crypt-authenticate  operations (nonMACsec), which can be used for
    test purposes. */
#define EIP164_CRYPT_AUTH_ENABLE

/** Enables strict argument checking for the input parameters in all the
    EIP-164 Driver API functions */
#define EIP164_STRICT_ARGS

/** Enable PACK timeout counter of statistics modules,
    granting the host access after a certain number of cycles. */
#define EIP164_PACK_TIMEOUT_ENABLE          1

/** Number of cycles for PACK timeout. */
#define EIP164_PACK_TIMEOUT_VALUE           1

/** Does device support per-counter summary? */
#if defined DRIVER_PERCOUNTER_SUMMARY || defined ONLY_FOR_DOXYGEN
#define EIP164_PERCOUNTER_SUMMARY_ENABLE
#endif


#ifdef ONLY_FOR_DOXYGEN

/** If this configuration option is set, the strict argument checking of the
    input parameters is enabled and can detect some programming errors.
    If not set, the performance is increased and code size is reduced. */
#define EIP164_CONF_BASE                    0x00

/** This configuration option sets (defines) the maximum crypt-authenticate
    bypass length. If not set, the default will be 255. */
#define EIP164_SECY_CRYPT_AUTH_MAX_BYPASS_LEN

/** This configuration option sets (defines) the base offset for all the
    EIP-66 registers. If not set, the default will be 0. */
#define EIP66_CONF_BASE

/** This configuration option sets (defines) the base offset for all the EIP-217
    registers. If not set, the default will be 0. */
#define EIP217_CONF_BASE

#endif /* ONLY_FOR_DOXYGEN */



/* End address of EIP-164 HW registers area */
#define EIP164_REG_IF_END (0x3EDFF)

/* Start/End address of EIP-201 (global AIC) in EIP-164 device  */
#define EIP164_GLOBAL_AIC_IF_START  EIP164_REG_AIC_POL_CTRL
#define EIP164_GLOBAL_AIC_IF_END  (EIP164_GLOBAL_AIC_IF_START + 0x3F)

/* Start/End address of Channel 0 Interrupt Controller in EIP-164 device  */
#define EIP164_REG_CHn_AIC_END(n)   (EIP164_CONF_BASE + 0xF43f + (0x10000 * ((n)>> 5)) + \
                                      (EIP164_REG_CHn_AIC_BASE_OFFS * ((n)& 0x1F)))

#endif /* CS_EIP164_H_ */


/* end of file cs_eip164.h */
