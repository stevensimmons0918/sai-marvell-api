/** @file cs_hwpal.h
 *
 * @brief Configuration Settings for Driver Framework Implementation
 * for the SafeXcel-IP-164 Driver.
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

#ifndef INCLUDE_GUARD_CS_HWPAL_H
#define INCLUDE_GUARD_CS_HWPAL_H

/* Top-level Driver configuration */
#include <Kit/DriverFramework/incl/cs_driver.h>

/* Top-level Adapter configuration */
#include <Kit/DriverFramework/incl/cs_adapter.h>

/* Host hardware platform specific extensions */
#include <Kit/DriverFramework/incl/cs_hwpal_ext.h>

/** log level for the entire adapter (for now) choose from:\n
     - LOG_SEVERITY_INFO\n
     - LOG_SEVERITY_WARN\n
     - LOG_SEVERITY_CRIT */
#ifdef ONLY_FOR_DOXYGEN
#define LOG_SEVERITY_MAX
#else
#undef LOG_SEVERITY_MAX
#endif
/** Trace level for log messages for the Driver Framework modules.\n
    Default value is LOG_SEVERITY_CRITICAL which means that only
    critical messages will be logged. It can be set to
    LOG_SEVERITY_INFO (log informational and critical messages), which
    will give more traces but will also slow down the Driver operation. */
#if defined DRIVER_PERFORMANCE || defined ONLY_FOR_DOXYGEN
#define HWPAL_LOG_SEVERITY  LOG_SEVERITY_CRITICAL
#else /* DRIVER_PERFORMANCE */
#define HWPAL_LOG_SEVERITY  LOG_SEVERITY_INFO
/*#define HWPAL_LOG_SEVERITY  LOG_SEVERITY_WARN */
#endif /* DRIVER_PERFORMANCE */

/** Maximum number of characters (bytes) that can be used for a device
    name as used in the HWPAL_DEVICES list of devices. */
#define HWPAL_MAX_DEVICE_NAME_LENGTH 64

/** Optional magic number used to validate the device administration data. */
#define HWPAL_DEVICE_MAGIC   54333

/** Specifies the driver name used for reporting in the log files and
    operating system. */
#ifdef DRIVER_NAME
#define HWPAL_DRIVER_NAME DRIVER_NAME
#endif

/** Define this if the Driver is to be compiled for a platform that
    has 64-bit addresses. Leave this undefined for a 32-bit platform.
    This option is normally passed via the make file from the DDK_HOST64
    environment variable. */
#if defined DRIVER_64BIT_HOST || defined ONLY_FOR_DOXYGEN
#define HWPAL_64BIT_HOST
/* Is device 64-bit? Only makes sense on 64-bit host. */
#endif  /* DRIVER_64BIT_HOST */

/** Only define this if the platform hardware guarantees cache coherence
    of DMA buffers, i.e. when SW does not need to control the coherence
    management. */
#if defined ARCH_X86 || defined ONLY_FOR_DOXYGEN
#define HWPAL_ARCH_COHERENT
#else
#undef HWPAL_ARCH_COHERENT
#endif

/** Enables strict argument checking for the input parameters in all the
    Driver Framework API functions. */
#if !defined DRIVER_PERFORMANCE || defined ONLY_FOR_DOXYGEN
#define HWPAL_STRICT_ARGS_CHECK
#if defined ARCH_X86 || defined ONLY_FOR_DOXYGEN
/* Enabled for DMAResource API implementation on x86 debugging purposes only */
#undef HWPAL_ARCH_COHERENT
#endif
#define HWPAL_TRACE_DEVICE_FIND
#define HWPAL_TRACE_DEVICE_READ
#define HWPAL_TRACE_DEVICE_WRITE
#endif

/** Enables the endianness conversion by the host CPU when there is a
    different endianness between the host and the device (little endian). */
#ifdef DRIVER_SWAPENDIAN
#define HWPAL_DEVICE_ENABLE_SWAP
#endif /* DRIVER_SWAPENDIAN */

/** Use sleepable or non-sleepable lock */
#ifdef ONLY_FOR_DOXYGEN
#define HWPAL_LOCK_SLEEPABLE
#else
/*#define HWPAL_LOCK_SLEEPABLE */
#endif

/** Use direct I/O bypassing the OS functions,
    I/O device must swap bytes in words */
#if defined DRIVER_ENABLE_SWAP_SLAVE || defined ONLY_FOR_DOXYGEN
#define HWPAL_DEVICE_DIRECT_MEMIO
#endif

/** EIP-164 device 0 name */
#ifdef DRIVER_DEVICE_NAME_0
#define HWPAL_EIP164_NAME_0   DRIVER_DEVICE_NAME_0
#else
#error "DRIVER_DEVICE_NAME_0 not defined"
#endif

/** EIP-164 device 1 name */
#ifdef DRIVER_DEVICE_NAME_1
#define HWPAL_EIP164_NAME_1   DRIVER_DEVICE_NAME_1
#else
#error "DRIVER_DEVICE_NAME_1 not defined"
#endif

/** EIP-164 Global EIP-201 Interrupt Controller (IC) device 0 name */
#ifdef DRIVER_IC_NAME_0
#define HWPAL_IC_NAME_0   DRIVER_IC_NAME_0
#else
#error "DRIVER_IC_NAME_0 not defined"
#endif

/** EIP-164 Packet Engine EIP-201 Interrupt Controller (IC) device 0 name */
#ifdef DRIVER_PE_IC_NAME_0
#define HWPAL_PE_IC_NAME_0   DRIVER_PE_IC_NAME_0
#else
#error "DRIVER_PE_IC_NAME_0 not defined"
#endif

/* EIP-164 Channel (0..19) EIP-201 Interrupt Controller (IC) device 0 name */
#if defined(DRIVER_IC_NAME_0_CH)
/** Name  of the EIP-201 device 0 for channel 0.19 */
#define HWPAL_IC_NAME_0_CH(n)   DRIVER_IC_NAME_0_CH(n)
#else
#error "DRIVER_IC_NAME_0_CH not defined"
#endif

/** Name  of the EIP-201 device 1. */
#ifdef DRIVER_IC_NAME_1
#define HWPAL_IC_NAME_1   DRIVER_IC_NAME_1
#else
#error "DRIVER_IC_NAME_1 not defined"
#endif

/** EIP-164 Packet Engine EIP-201 Interrupt Controller (IC) device 1 name */
#ifdef DRIVER_PE_IC_NAME_1
#define HWPAL_PE_IC_NAME_1   DRIVER_PE_IC_NAME_1
#else
#error "DRIVER_PE_IC_NAME_1 not defined"
#endif


/* EIP-164 Channel (0..19) EIP-201 Interrupt Controller (IC) device 1 name */
#if defined(DRIVER_IC_NAME_1_CH)
/** Name  of the EIP-201 device 1 for channel 0..19 */
#define HWPAL_IC_NAME_1_CH(n)   DRIVER_IC_NAME_1_CH(n)
#else
#error "DRIVER_IC_NAME_1_CH not defined"
#endif

/** EIP-163 device 0 name */
#ifdef DRIVER_CE_DEVICE_NAME_0
#define HWPAL_EIP163_NAME_0   DRIVER_CE_DEVICE_NAME_0
#else
#error "DRIVER_CE_DEVICE_NAME_0 not defined"
#endif

/** EIP-163 device 1 name */
#ifdef DRIVER_CE_DEVICE_NAME_1
#define HWPAL_EIP163_NAME_1   DRIVER_CE_DEVICE_NAME_1
#else
#error "DRIVER_CE_DEVICE_NAME_1 not defined"
#endif

/** EIP-163 Global EIP-201 Interrupt Controller (IC) device 0 name */
#ifdef DRIVER_CE_IC_NAME_0
#define HWPAL_CE_IC_NAME_0   DRIVER_CE_IC_NAME_0
#else
#error "DRIVER_CE_IC_NAME_0 not defined"
#endif

/* EIP-163 Channel (0..19) EIP-201 Interrupt Controller (IC) device 0 name */
#if defined(DRIVER_CE_IC_NAME_0_CH)
/** EIP-163 Channel 0..19 EIP-201 Interrupt Controller (IC) device 0 name */
#define HWPAL_CE_IC_NAME_0_CH(n)   DRIVER_CE_IC_NAME_0_CH(n)
#else
#error "DRIVER_CE_IC_NAME_0_CH not defined"
#endif

/** EIP-163 Global EIP-201 Interrupt Controller (IC) device 1 name */
#ifdef DRIVER_CE_IC_NAME_1
#define HWPAL_CE_IC_NAME_1   DRIVER_CE_IC_NAME_1
#else
#error "DRIVER_CE_IC_NAME_1 not defined"
#endif

/* EIP-163 Channel (0..3) EIP-201 Interrupt Controller (IC) device 1 name */
#if defined(DRIVER_CE_IC_NAME_1_CH)
/** EIP-163 Channel 0 EIP-201 Interrupt Controller (IC) device 1 name */
#define HWPAL_CE_IC_NAME_1_CH(n)   DRIVER_CE_IC_NAME_1_CH(n)
#else
#error "DRIVER_CE_IC_NAME_1_CH not defined"
#endif


#ifdef ONLY_FOR_DOXYGEN
/** List of devices with their names and their addresses ranges. All addresses
    of all devices accessed by the driver are assumed to lie in a single
    address range, e.g. a single BAR on a PCI device or a single address range
    as specified in the Open Firmware device tree. The start and end addresses
    are specified as byte offsets with respect to the start of that range.\n
    Normally the EIP-201 devices all have a fixed offset with respect to the
    EIP-164 or EIP-163 devices they belong to, but the address ranges of
    the EIP-164 or EIP-163 devices may start at different offsets and need to
    be customized. */
#define HWPAL_DEVICES

/** Index of the IRQ used to query the physical device interrupt number via the
    Linux platform kernel API. */
#define HWPAL_PLATFORM_IRQ_IDX

/** These are only used for a PCI (or PCIe) device and they specify the device
    of the device. Note that similar parameters must also be specified in the
    configuration of the Kernel Support Driver in cs_umdevxs_ext.h */
#define HWPAL_DEVICE_ID

/** These are only used for a PCI (or PCIe) device and they specify the vendor
    of the device. Note that similar parameters must also be specified in the
    configuration of the Kernel Support Driver in cs_umdevxs_ext.h */
#define HWPAL_VENDOR_ID

/** This is only used for a PCI (or PCIe) device. It specifies whether the
    device must use MSI interrupts. Note that a similar parameter must also
    be specified in the configuration of the Kernel Support Driver in
    cs_umdevxs_ext.h */
#define HWPAL_USE_MSI

/** This is only used for an Open Firmware device. It specifies the name of
    the device as specified in the Open Firmware device tree.\n
    Note that a similar parameter must also be specified in the
    configuration of the Kernel Support Driver in cs_umdevxs_ext.h */
#define HWPAL_PLATFORM_DEVICE_NAME  "safexcel-ip-164"

/** Device_Read32() function will busy wait for the specified by this parameter
    number of microseconds before actually performing the read operation.\n
    This is a workaround for postedwrites implementation of the MMIO register
    slave interface when too fast write-read sequence does not work. */
#define HWPAL_DEVICE_READ_DELAY_US    100


#else /* ONLY_FOR_DOXYGEN */
/* Maximum number of devices in the device list */
#define HWPAL_DEVICE_COUNT          \
        (DRIVER_OPTIONAL_DEVICE_COUNT + \
         DRIVER_MAX_NOF_EIP164_DEVICES + \
         DRIVER_MAX_NOF_EIP164_IC_DEVICES + \
         DRIVER_MAX_NOF_EIP163_DEVICES + \
         DRIVER_MAX_NOF_EIP163_IC_DEVICES)


/* Device order rules for the HWPAL_DEVICES device list in cs_hwpal_ext.h: */
/* - First (HWPAL_OPTIONAL_DEVICES_COUNT) optional devices, */
/* - Then mandatory devices, e.g. pairs of EIP-164 and EIP-163 device chains, */
/*   EIP-164 chains should always precede the corresponding EIP-163 chain, */
/*   one port requires one pair of the EIP-164 and EIP-163 chains */
/*   for the ingress processing and another pair for the egress processing. */
/* - Egress should follow ingress data path. */
/* - There may be no two devices in the list with the same name. */

/* Note: Device names in the list must be unique. */

/* Helper macros derived from driver config to define all devices required */
/* for an EIP164/EIP163 data path. */
#undef DRIVER_DEVICE_ADD
#define DRIVER_DEVICE_ADD HWPAL_DEVICE_ADD

/* Ingress data path, */
/* one port consists of both ingress and egress data path, */
/* first ingress followed by egress data path in the device list per port */
#define HWPAL_DATAPATH_EGRESS DRIVER_DATAPATH
#define HWPAL_DATAPATH_INGRESS DRIVER_DATAPATH

#endif /* ONLY_FOR_DOXYGEN */


#endif /* INCLUDE_GUARD_CS_HWPAL_H */


/* end of file cs_hwpal.h */
