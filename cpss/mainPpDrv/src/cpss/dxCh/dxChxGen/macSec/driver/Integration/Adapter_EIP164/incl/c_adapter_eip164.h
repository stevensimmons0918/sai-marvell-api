/** @file c_adapter_eip164.h
 *
 * @brief Default Adapter EIP-164 configuration
 */

/*****************************************************************************
* Copyright (c) 2015-2019 INSIDE Secure B.V. All Rights Reserved.
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

#ifndef INCLUDE_GUARD_C_ADAPTER_EIP164_H
#define INCLUDE_GUARD_C_ADAPTER_EIP164_H

/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

/* Top-level Adapter configuration */
#include <Kit/DriverFramework/incl/cs_adapter.h>

/* log level for the entire adapter (for now) */
/* choose from LOG_SEVERITY_INFO, LOG_SEVERITY_WARN, LOG_SEVERITY_CRIT */
/*#define LOG_SEVERITY_MAX  LOG_SEVERITY_CRITICAL */

/* enable debug checks */
/*#define ADAPTER_EIP164_DBG */
/*#define ADAPTER_EIP164_STRICT_ARGS */

#ifndef ONLY_FOR_DOXYGEN

/** Adapter EIP-164 driver name */
#ifndef ADAPTER_EIP164_DRIVER_NAME
#define ADAPTER_EIP164_DRIVER_NAME     "SafeXcel"
#endif

/** Adapter EIP-164 driver license */
#ifndef ADAPTER_EIP164_LICENSE
#define ADAPTER_EIP164_LICENSE         "Proprietary"
#endif

/** PCI configuration value: Cache Line Size, in 32bit words
    Advised value: 1 */
#ifndef ADAPTER_PCICONFIG_CACHELINESIZE
#define ADAPTER_PCICONFIG_CACHELINESIZE   1
#endif

/** PCI configuration value: Master Latency Timer, in PCI bus clocks
    Advised value: 0xf8 */
#ifndef ADAPTER_PCICONFIG_MASTERLATENCYTIMER
#define ADAPTER_PCICONFIG_MASTERLATENCYTIMER 0xf8
#endif

/* Is host platform 64-bit? */
/*#define ADAPTER_EIP164_64BIT_HOST */

/** Maximum of 4 EIP-164 units per device (Ingress/Egress + DP0/DP1) */
#ifndef ADAPTER_EIP164_MAX_NOF_DEVICES
#define ADAPTER_EIP164_MAX_NOF_DEVICES           4*PRV_CPSS_MAX_PP_DEVICES_CNS
#endif

/** Adapter EIP-164 devices configuration */
#ifndef ADAPTER_EIP164_DEVICES
#error "ADAPTER_EIP164_DEVICES must be defined"
#endif

/* Enables interrupt support when defined */
/*#define ADAPTER_EIP164_INTERRUPTS_ENABLE */

/** Filter for tracing interrupts */
#ifdef ADAPTER_EIP164_INTERRUPTS_ENABLE
#ifndef ADAPTER_EIP164_INTERRUPTS_TRACEFILTER
#define ADAPTER_EIP164_INTERRUPTS_TRACEFILTER 0x00000FFF
#endif
#endif /* ADAPTER_EIP164_INTERRUPTS_ENABLE */

/** Default 1 IC device for one EIP-164 device */
#ifndef ADAPTER_EIP164_MAX_IC_COUNT
#define ADAPTER_EIP164_MAX_IC_COUNT              ADAPTER_EIP164_MAX_NOF_DEVICES
#endif

/** Total number of supported IC devices */
#ifndef ADAPTER_INTERRUPTS_MAX_COUNT
#define ADAPTER_INTERRUPTS_MAX_COUNT             ADAPTER_EIP164_MAX_IC_COUNT
#endif

#ifndef ADAPTER_EIP164_PHY_DROP_CLASS_IRQ
#define ADAPTER_EIP164_PHY_DROP_CLASS_IRQ        0
#endif

#ifndef ADAPTER_EIP164_DROP_CLASS_INT_NAME
#define ADAPTER_EIP164_DROP_CLASS_INT_NAME       "EIP164-DROP-CLASS"
#endif

#ifndef ADAPTER_EIP164_PHY_DROP_PP_IRQ
#define ADAPTER_EIP164_PHY_DROP_PP_IRQ           1
#endif

#ifndef ADAPTER_EIP164_DROP_PP_INT_NAME
#define ADAPTER_EIP164_DROP_PP_INT_NAME          "EIP164-DROP-PP"
#endif

#ifndef ADAPTER_EIP164_PHY_DROP_MTU_IRQ
#define ADAPTER_EIP164_PHY_DROP_MTU_IRQ          2
#endif

#ifndef ADAPTER_EIP164_DROP_MTU_INT_NAME
#define ADAPTER_EIP164_DROP_MTU_INT_NAME         "EIP164-DROP-MTU"
#endif

#ifndef ADAPTER_EIP66_PHY_IRQ
#define ADAPTER_EIP66_PHY_IRQ                    3
#endif

#ifndef ADAPTER_EIP66_INT_NAME
#define ADAPTER_EIP66_INT_NAME                   "EIP66-IRQ"
#endif

#ifndef ADAPTER_EIP164_PHY_STAT_SA_THR_IRQ
#define ADAPTER_EIP164_PHY_STAT_SA_THR_IRQ       4
#endif

#ifndef ADAPTER_EIP164_STAT_SA_THR_INT_NAME
#define ADAPTER_EIP164_STAT_SA_THR_INT_NAME      "EIP164-STAT-SA-THR"
#endif

#ifndef ADAPTER_EIP164_PHY_STAT_GLOBAL_THR_IRQ
#define ADAPTER_EIP164_PHY_STAT_GLOBAL_THR_IRQ   5
#endif

#ifndef ADAPTER_EIP164_STAT_GLOBAL_THR_INT_NAME
#define ADAPTER_EIP164_STAT_GLOBAL_THR_INT_NAME  "EIP164-STAT-GLOBAL-THR"
#endif

#ifndef ADAPTER_EIP164_PHY_STAT_VLAN_THR_IRQ
#define ADAPTER_EIP164_PHY_STAT_VLAN_THR_IRQ     6
#endif

#ifndef ADAPTER_EIP164_STAT_VLAN_THR_INT_NAME
#define ADAPTER_EIP164_STAT_VLAN_THR_INT_NAME    "EIP164-STAT-VLAN-THR"
#endif

#ifndef ADAPTER_EIP164_PHY_CC_IRQ
#define ADAPTER_EIP164_PHY_CC_IRQ                7
#endif

#ifndef ADAPTER_EIP164_CC_INT_NAME
#define ADAPTER_EIP164_CC_INT_NAME               "EIP164-CC"
#endif

#ifndef ADAPTER_EIP164_PHY_SA_PN_THR_IRQ
#define ADAPTER_EIP164_PHY_SA_PN_THR_IRQ         8
#endif

#ifndef ADAPTER_EIP164_SA_PN_THR_INT_NAME
#define ADAPTER_EIP164_SA_PN_THR_INT_NAME        "EIP164-SA-PN-THR"
#endif

#ifndef ADAPTER_EIP164_PHY_SA_EXPIRED_IRQ
#define ADAPTER_EIP164_PHY_SA_EXPIRED_IRQ        9
#endif

#ifndef ADAPTER_EIP164_SA_EXPIRED_INT_NAME
#define ADAPTER_EIP164_SA_EXPIRED_INT_NAME       "EIP164-SA-EXPIRED"
#endif

#ifndef ADAPTER_EIP164_INT_RESOURCES
#error "ADAPTER_EIP164_INT_RESOURCES not defined"
#endif

/** List of all EIP-201 AIC devices used by the driver together with
    the ranges of interrupt numbers served by each of them */
#ifndef ADAPTER_EIP164_IC_DEVICES
#error "ADAPTER_EIP164_IC_DEVICES not defined"
#endif

/* SecY device mode */
/*#define ADAPTER_EIP164_MODE_EGRESS */
/*#define ADAPTER_EIP164_MODE_INGRESS */

/** Maximum number of initialization status polls */
#ifndef ADAPTER_EIP164_MAX_NOF_INIT_RETRY_COUNT
#define ADAPTER_EIP164_MAX_NOF_INIT_RETRY_COUNT 100
#endif

/** Timeout in milliseconds between two consecutive initialization
    status polls */
#ifndef ADAPTER_EIP164_INIT_RETRY_TIMEOUT_MS
#define ADAPTER_EIP164_INIT_RETRY_TIMEOUT_MS    10
#endif

/** Maximum number of uninitialization status polls */
#ifndef ADAPTER_EIP164_MAX_NOF_UNINIT_RETRY_COUNT
#define ADAPTER_EIP164_MAX_NOF_UNINIT_RETRY_COUNT 100
#endif

/** Timeout in milliseconds between two consecutive uninitialization
    status polls */
#ifndef ADAPTER_EIP164_UNINIT_RETRY_TIMEOUT_MS
#define ADAPTER_EIP164_UNINIT_RETRY_TIMEOUT_MS    10
#endif

/** Maximum number of synchronization status polls */
#ifndef ADAPTER_EIP164_MAX_NOF_SYNC_RETRY_COUNT
#define ADAPTER_EIP164_MAX_NOF_SYNC_RETRY_COUNT 100
#endif

/** Timeout in milliseconds between two consecutive synchronization
    status polls */
#ifndef ADAPTER_EIP164_SYNC_RETRY_TIMEOUT_MS
#define ADAPTER_EIP164_SYNC_RETRY_TIMEOUT_MS    10
#endif

#endif  /* ONLY_FOR_DOXYGEN */

/** By default the fixed latency for EIP-164 device is disabled,
    each packet may have its own processing time in the device */
#ifndef ADAPTER_EIP164_FIXED_LATENCY
#define ADAPTER_EIP164_FIXED_LATENCY    0
#endif

/** By default the static bypass is switched off */
#ifndef ADAPTER_EIP164_STATIC_BYPASS
#define ADAPTER_EIP164_STATIC_BYPASS    0
#endif

/** 1 - perform drop action if packet is not from the reserved port\n
    0 - do not drop */
#ifndef ADAPTER_EIP164_SEC_DROP_NON_RESERVED
#define ADAPTER_EIP164_SEC_DROP_NON_RESERVED    0
#endif


/** This mask specifies which SA-related counter increments are regarded
    a security fail event. Bit 0 is for the first 40-bits counter of an
    SA-related counter set.\n
    Ingress device has 11 SA counters - bits [14:11] must be kept zero.
    Egress device has 4 SA counters - bits [14:4] must be kept zero.
    Combined device has 11 SA counters - bits [14:11] must be kept zero. */
#ifndef ADAPTER_EIP164_SA_SECFAIL_MASK
#define ADAPTER_EIP164_SA_SECFAIL_MASK          0
#endif

/** This mask specifies which SecY counter increments are regarded a
    security fail event - bit [0] is for the first 64-bits global counter.\n
    @note: Actual width depends on the number of SecY counters implemented. */
#ifndef ADAPTER_EIP164_SECY_SECFAIL_MASK
#define ADAPTER_EIP164_SECY_SECFAIL_MASK        0
#endif

/** The statistics "clear on read" EIP-164 device feature should not be enabled
    in a system that performs speculative reads, i.e. one that reads ahead
    "just in case the next words are needed too" */
#ifndef ADAPTER_EIP164_CLEAR_ON_READ
#define ADAPTER_EIP164_CLEAR_ON_READ         1
#endif

/** Cleare statistics by writing the read value to decrement */
#ifndef ADAPTER_EIP164_WRITE_TO_DECREMENT
#define ADAPTER_EIP164_WRITE_TO_DECREMENT       0
#endif

/* Enable EIP-164 Crypt-Authenticate feature */
/*#define ADAPTER_EIP164_CRYPT_AUTH_ENABLE */

/** Clock control: 1 - clocks are always enabled,\n
                   0 - dynamic clock control by device, e.g. only when needed */
#ifndef ADAPTER_EIP164_DEVICE_CLOCK_CONTROL
#define ADAPTER_EIP164_DEVICE_CLOCK_CONTROL   0
#endif

#ifdef ONLY_FOR_DOXYGEN
/** Enable per-counter summary registers. */
#define ADAPTER_EIP164_PERCOUNTER_SUMMARY_ENABLE

/** Enable WarmBoot */
#define ADAPTER_EIP164_WARMBOOT_ENABLE
#endif


/* List of all devices in a data path. */
#ifndef ADAPTER_EIP164_DATAPATH
#error "ADAPTER_EIP164_DATAPATH undefined"
#endif

/* Device name of the EIP164 device */
#ifndef ADAPTER_EIP164_NAME
#error "ADAPTER_EIP164_NAME undefined"
#endif

/* Device name of the EIP163 device */
#ifndef ADAPTER_EIP163_NAME
#error "ADAPTER_EIP163_NAME undefined"
#endif

#endif /* Include Guard */


/* end of file c_adapter_eip164.h */
