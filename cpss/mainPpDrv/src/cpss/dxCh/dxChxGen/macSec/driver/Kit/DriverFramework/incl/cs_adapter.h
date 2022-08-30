/** @file cs_adapter.h
 *
 * @brief Configuration Settings for the Adapter module.
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
#ifndef INCLUDE_GUARD_CS_ADAPTER_H
#define INCLUDE_GUARD_CS_ADAPTER_H


/* we accept a few settings from the top-level configuration file */
#include <Kit/DriverFramework/incl/cs_driver.h>

#include <Kit/DriverFramework/incl/cs_driver_ext.h>

/****************************************************************************
 * Generic adapter configuration parameters
 */

/** Trace level (for informational messages) for the Adapter module.
    Default value is LOG_SEVERITY_CRITICAL. It can be set to
    LOG_SEVERITY_INFO, which will give more traces but will also
    slow down the Driver operation. */
#ifndef LOG_SEVERITY_MAX
    #ifdef DRIVER_PERFORMANCE
        #ifndef CPSS_LOG_ENABLE
            #define LOG_SEVERITY_MAX  LOG_SEVERITY_NO_OUTPUT
        #else
            #define LOG_SEVERITY_MAX  LOG_SEVERITY_CRITICAL
        #endif
    #else
        #ifndef CPSS_LOG_ENABLE
            #define LOG_SEVERITY_MAX  LOG_SEVERITY_NO_OUTPUT
        #else
            #define LOG_SEVERITY_MAX  LOG_SEVERITY_INFO /*LOG_SEVERITY_WARN*/
        #endif
    #endif
#endif


/****************************************************************************
 * Adapter EIP-163 configuration parameters
 */

#if !defined DRIVER_PERFORMANCE || defined ONLY_FOR_DOXYGEN
/** Enables additional debugging mechanisms. */
#define ADAPTER_EIP163_DBG
/** Enables strict argument checking for the input parameters in all
    the driver API functions implemented in the EIP-160 Adapter module. */
#define ADAPTER_EIP163_STRICT_ARGS
#endif

/** Specifies the driver name used for reporting in the log files and
    operating system. */
/** Adapter driver name */
#ifndef ADAPTER_EIP163_DRIVER_NAME
#define ADAPTER_EIP163_DRIVER_NAME      "SafeXcel-IP-163"
#endif

/** Maximum number of EIP-163 devices supported by the driver. */
#ifdef DRIVER_MAX_NOF_EIP163_DEVICES
#define ADAPTER_EIP163_MAX_NOF_DEVICES   DRIVER_MAX_NOF_EIP163_DEVICES
#endif

/** The Driver can be used with or without interrupts. It provides an
    API function that can be used to register a callback to be invoked on
    receiving device interrupts. These callbacks typically read the device
    status or trigger this functionality in other execution contexts.\n
    When interrupts are off, the Driver is said to be in 'polling mode'.\n
    In this mode, the caller of the Driver API should poll the device
    registers for the device status. When interrupts are enabled, result
    notification callbacks are invoked asynchronously with respect to
    the main application. */
#ifdef DRIVER_INTERRUPTS
#define ADAPTER_EIP163_INTERRUPTS_ENABLE
#endif


/** Maximum number of attempts for the Driver during the device uninitialization
    to get the device status initialized. */
#ifdef DRIVER_MAX_NOF_INIT_RETRY_COUNT
#define ADAPTER_EIP163_MAX_NOF_INIT_RETRY_COUNT DRIVER_MAX_NOF_INIT_RETRY_COUNT
#endif

/** Timeout in milliseconds between two consecutive status polls for the
    Driver during the device initialization. */
#ifdef DRIVER_INIT_RETRY_TIMEOUT_MS
#define ADAPTER_EIP163_INIT_RETRY_TIMEOUT_MS    DRIVER_INIT_RETRY_TIMEOUT_MS
#endif

/** Maximum number of attempts for the Driver during the device uninitialization
    to get the device status uninitialized. */
#ifdef DRIVER_MAX_NOF_UNINIT_RETRY_COUNT
#define ADAPTER_EIP163_MAX_NOF_UNINIT_RETRY_COUNT DRIVER_MAX_NOF_UNINIT_RETRY_COUNT
#endif

/** Timeout in milliseconds between two consecutive status polls for the
    Driver during the device uninitialization. */
#ifdef DRIVER_UNINIT_RETRY_TIMEOUT_MS
#define ADAPTER_EIP163_UNINIT_RETRY_TIMEOUT_MS    DRIVER_UNINIT_RETRY_TIMEOUT_MS
#endif

/** Maximum number of status polls for the Driver for the
    host-device synchronization. */
#ifdef DRIVER_MAX_NOF_SYNC_RETRY_COUNT
#define ADAPTER_EIP163_MAX_NOF_SYNC_RETRY_COUNT DRIVER_MAX_NOF_SYNC_RETRY_COUNT
#endif

/** Timeout in milliseconds between two consecutive status polls for the
    Driver during the hostdevice synchronization. */
#ifdef DRIVER_SYNC_RETRY_TIMEOUT_MS
#define ADAPTER_EIP163_SYNC_RETRY_TIMEOUT_MS    DRIVER_SYNC_RETRY_TIMEOUT_MS
#endif

/* EIP-163 Global AIC interrupt resources */
/** EIP-163 Statistics TCAM Threshold interrupt number */
#define ADAPTER_EIP163_PHY_STAT_TCAM_THR_IRQ      0
/** EIP-160 Statistics TCAM Threshold interrupt name */
#define ADAPTER_EIP163_STAT_TCAM_THR_INT_NAME     "EIP163-STAT-TCAM-THR"

/** EIP-163 Statistics Channel Threshold interrupt number */
#define ADAPTER_EIP163_PHY_STAT_CHAN_THR_IRQ      1
/** EIP-160 Statistics Channel Threshold interrupt name */
#define ADAPTER_EIP163_STAT_CHAN_THR_INT_NAME     "EIP163-STAT-CHAN-THR"

/** EIP-163 Statistics ECC Error interrupt number */
#define ADAPTER_EIP163_PHY_ECC_ERR_IRQ            8
/** EIP-160 Statistics ECC Error interrupt name */
#define ADAPTER_EIP163_ECC_ERR_INT_NAME           "EIP163-ECC-ERR"

/* EIP-163 Channel AIC interrupt resources */
/** EIP-163 Channel Error SOP without EOP interrupt number */
#define ADAPTER_EIP163_CHAN_ERR_SOP_WO_EOP_IRQ       0
/** EIP-163 Channel Error SOP without EOP interrupt name */
#define ADAPTER_EIP163_CHAN_ERR_SOP_WO_EOP_INT_NAME  "EIP163-CHAN-ERR-SOP-WO-EOP"

/** EIP-163 Channel Error EOP without SO interrupt number */
#define ADAPTER_EIP163_CHAN_ERR_EOP_WO_SOP_IRQ       1
/** EIP-163 Channel Error EOP without SO interrupt name */
#define ADAPTER_EIP163_CHAN_ERR_EOP_WO_SOP_INT_NAME  "EIP163-CHAN-ERR-EOP-WO-SOP"

/** EIP-163 Channel Error Transfer without SOP interrupt number */
#define ADAPTER_EIP163_CHAN_ERR_XFER_WO_SOP_IRQ      2
/** EIP-163 Channel Error Transfer without SOP interrupt name */
#define ADAPTER_EIP163_CHAN_ERR_XFER_WO_SOP_INT_NAME "EIP163-CHAN-ERR-XFER-WO-SOP"

/** EIP-163 Channel Error SLOT SOP interrupt number */
#define ADAPTER_EIP163_CHAN_ERR_SLOT_SOP_IRQ         3
/** EIP-163 Channel Error SLOT SOP interrupt name */
#define ADAPTER_EIP163_CHAN_ERR_SLOT_SOP_INT_NAME    "EIP163-CHAN-ERR-SLOT-SOP"

/** EIP-163 Channel Error SLOT ChannelId interrupt number */
#define ADAPTER_EIP163_CHAN_ERR_SLOT_CHID_IRQ        4
/** EIP-163 Channel Error SLOT ChannelId interrupt name */
#define ADAPTER_EIP163_CHAN_ERR_SLOT_CHID_INT_NAME   "EIP163-CHAN-ERR-SLOT-CHID"

/** EIP-163 Channel Error NOT B2B interrupt number */
#define ADAPTER_EIP163_CHAN_ERR_NOT_B2B_IRQ          5
/** EIP-163 Channel Error NOT B2B interrupt name */
#define ADAPTER_EIP163_CHAN_ERR_NOT_B2B_INT_NAME    "EIP163-CHAN-ERR-NOT-B2B"

/** EIP-163 Channel Control Packet interrupt number */
#define ADAPTER_EIP163_CHAN_CTRL_PKT_IRQ             6
/** EIP-163 Channel Control Packet interrupt name */
#define ADAPTER_EIP163_CHAN_CTRL_PKT_INT_NAME        "EIP163-CHAN-CTRL-PKT"

/** EIP-163 Channel Data Packet interrupt number */
#define ADAPTER_EIP163_CHAN_DATA_PKT_IRQ             7
/** EIP-163 Channel Data Packet interrupt name */
#define ADAPTER_EIP163_CHAN_DATA_PKT_INT_NAME        "EIP163-CHAN-DATA-PKT"

/** EIP-163 Channel DROP Packet interrupt number */
#define ADAPTER_EIP163_CHAN_DROP_PKT_IRQ             8
/** EIP-163 Channel DROP Packet interrupt name */
#define ADAPTER_EIP163_CHAN_DROP_PKT_INT_NAME        "EIP163-CHAN-DROP-PKT"

/** EIP-163 Channel TCAM Multi Hit interrupt number */
#define ADAPTER_EIP163_CHAN_TCAM_HIT_MULT_IRQ        9
/** EIP-163 Channel TCAM Multi Hit interrupt name */
#define ADAPTER_EIP163_CHAN_TCAM_HIT_MULT_INT_NAME   "EIP163-CHAN-TCAM-HIT-MULT"

/** EIP-163 Channel TCAM Miss interrupt number */
#define ADAPTER_EIP163_CHAN_TCAM_MISS_IRQ            10
/** EIP-163 Channel TCAM Miss interrupt name */
#define ADAPTER_EIP163_CHAN_TCAM_MISS_INT_NAME       "EIP163-CHAN-TCAM-MISS"

/** EIP-163 Channel ErrSlot2Pkts interrupt number */
#define ADAPTER_EIP163_CHAN_ERR_SLOT_2PKTS_IRQ       11
/** EIP-163 Channel ErrSlot2Pkts interrupt name */
#define ADAPTER_EIP163_CHAN_ERR_SLOT_2PKTS_INT_NAME  "EIP163-CHAN-ERR_SLOT_2PKTS"

/** List of all interrupts used by the adapter with the interrupt controllers
    they are served by, their physical numbers and their type and polarity.
    The interrupt sources is a bitmask of interrupt sources
    Refer to the data sheet of device for the correct values\n
    Config: Choose from RISING_EDGE, FALLING_EDGE, ACTIVE_HIGH, ACTIVE_LOW\n
    Interrupt sources:     Name, Inputs,   Config */
#define ADAPTER_EIP163_INT_RESOURCES                                         \
    /* EIP-163 Global AIC interrupt resources (2), level sensitive */        \
    ADAPTER_EIP164_INT_RESOURCE_ADD(ADAPTER_EIP163_STAT_TCAM_THR_INT_NAME,   \
                                    1<<ADAPTER_EIP163_PHY_STAT_TCAM_THR_IRQ, \
                                    ACTIVE_HIGH),                            \
    ADAPTER_EIP164_INT_RESOURCE_ADD(ADAPTER_EIP163_STAT_CHAN_THR_INT_NAME,   \
                                    1<<ADAPTER_EIP163_PHY_STAT_CHAN_THR_IRQ, \
                                    ACTIVE_HIGH),                            \
    ADAPTER_EIP164_INT_RESOURCE_ADD(ADAPTER_EIP163_ECC_ERR_INT_NAME,   \
                                    1<<ADAPTER_EIP163_PHY_ECC_ERR_IRQ, \
                                    ACTIVE_HIGH),                            \
    /* EIP-163 Channel AIC interrupt resources (11), edge sensitive */           \
    ADAPTER_EIP164_INT_RESOURCE_ADD(ADAPTER_EIP163_CHAN_ERR_SOP_WO_EOP_INT_NAME, \
                                    1<<ADAPTER_EIP163_CHAN_ERR_SOP_WO_EOP_IRQ,   \
                                    RISING_EDGE),                                \
    ADAPTER_EIP164_INT_RESOURCE_ADD(ADAPTER_EIP163_CHAN_ERR_EOP_WO_SOP_INT_NAME, \
                                    1<<ADAPTER_EIP163_CHAN_ERR_SOP_WO_EOP_IRQ,   \
                                    RISING_EDGE),                                \
    ADAPTER_EIP164_INT_RESOURCE_ADD(ADAPTER_EIP163_CHAN_ERR_XFER_WO_SOP_INT_NAME,\
                                    1<<ADAPTER_EIP163_CHAN_ERR_XFER_WO_SOP_IRQ,  \
                                    RISING_EDGE),                                \
    ADAPTER_EIP164_INT_RESOURCE_ADD(ADAPTER_EIP163_CHAN_ERR_SLOT_SOP_INT_NAME,   \
                                    1<<ADAPTER_EIP163_CHAN_ERR_SLOT_SOP_IRQ,     \
                                    RISING_EDGE),                                \
    ADAPTER_EIP164_INT_RESOURCE_ADD(ADAPTER_EIP163_CHAN_ERR_SLOT_CHID_INT_NAME,  \
                                    1<<ADAPTER_EIP163_CHAN_ERR_SLOT_CHID_IRQ,    \
                                    RISING_EDGE),                                \
    ADAPTER_EIP164_INT_RESOURCE_ADD(ADAPTER_EIP163_CHAN_ERR_NOT_B2B_INT_NAME,    \
                                    1<<ADAPTER_EIP163_CHAN_ERR_NOT_B2B_IRQ,      \
                                    RISING_EDGE),                                \
    ADAPTER_EIP164_INT_RESOURCE_ADD(ADAPTER_EIP163_CHAN_CTRL_PKT_INT_NAME,       \
                                    1<<ADAPTER_EIP163_CHAN_CTRL_PKT_IRQ,         \
                                    RISING_EDGE),                                \
    ADAPTER_EIP164_INT_RESOURCE_ADD(ADAPTER_EIP163_CHAN_DATA_PKT_INT_NAME,       \
                                    1<<ADAPTER_EIP163_CHAN_DATA_PKT_IRQ,         \
                                    RISING_EDGE),                                \
    ADAPTER_EIP164_INT_RESOURCE_ADD(ADAPTER_EIP163_CHAN_DROP_PKT_INT_NAME,       \
                                    1<<ADAPTER_EIP163_CHAN_DROP_PKT_IRQ,         \
                                    RISING_EDGE),                                \
    ADAPTER_EIP164_INT_RESOURCE_ADD(ADAPTER_EIP163_CHAN_TCAM_HIT_MULT_INT_NAME,  \
                                    1<<ADAPTER_EIP163_CHAN_TCAM_HIT_MULT_IRQ,    \
                                    RISING_EDGE),                                \
    ADAPTER_EIP164_INT_RESOURCE_ADD(ADAPTER_EIP163_CHAN_TCAM_MISS_INT_NAME,      \
                                    1<<ADAPTER_EIP163_CHAN_TCAM_MISS_IRQ,        \
                                    RISING_EDGE),                                \
    ADAPTER_EIP164_INT_RESOURCE_ADD(ADAPTER_EIP163_CHAN_ERR_SLOT_2PKTS_NAME,      \
                                    1<<ADAPTER_EIP163_CHAN_ERR_SLOT_2PKTS_IRQ,    \
                                    RISING_EDGE)

/** Select Egress mode */
#ifdef DRIVER_MODE_EGRESS
#define ADAPTER_EIP163_MODE_EGRESS
#endif

/** Select Ingress mode */
#ifdef DRIVER_MODE_INGRESS
#define ADAPTER_EIP163_MODE_INGRESS
#endif

/** Name of the EIP-163 device 0. */
#define ADAPTER_EIP163_DEVICE_NAME_0      DRIVER_CE_DEVICE_NAME_0

/** Name of the EIP-163 device 1. */
#define ADAPTER_EIP163_DEVICE_NAME_1      DRIVER_CE_DEVICE_NAME_1

/** List of all EIP-163 devices with their names. */
#define ADAPTER_EIP163_DEVICES \
    ADAPTER_EIP163_DEVICE(ADAPTER_EIP163_DEVICE_NAME_0), \
    ADAPTER_EIP163_DEVICE(ADAPTER_EIP163_DEVICE_NAME_1)

/** Name of EIP-163 device #0 Global AIC's */
#define ADAPTER_EIP163_IC_NAME_0          DRIVER_CE_IC_NAME_0

/** Name of EIP-163 device #1 Global AIC's */
#define ADAPTER_EIP163_IC_NAME_1          DRIVER_CE_IC_NAME_1

/** name of EIP-163 device #0 Channel 0..64 AIC */
#define ADAPTER_EIP163_CE_IC_NAME_0_CH(n)   DRIVER_CE_IC_NAME_0_CH(n)

/** name of EIP-163 device #1 Channel 0..64 AIC */
#define ADAPTER_EIP163_CE_IC_NAME_1_CH(n)   DRIVER_CE_IC_NAME_1_CH(n)

/** Maximum number of EIP-201 devices in the EIP-163 device
    supported by the driver. */
#define ADAPTER_EIP163_MAX_IC_COUNT       DRIVER_MAX_NOF_EIP163_IC_DEVICES

/** Calculate Adapter EIP-163 notification ID from EIP-201 AIC device ID,
    see Note 1 in cs_driver.h. One notification is used per one EIP-163 AIC
    device. */
#define ADAPTER_EIP163_NOTIFY_ID(ICDevID) \
    ((((((ICDevID - DRIVER_OPTIONAL_DEVICE_COUNT) / \
        (3 + DRIVER_MAX_NOF_CHANNEL_IC_DEVICES)) - 1) / 2) * \
          (2 + DRIVER_MAX_NOF_CHANNEL_IC_DEVICES)) + \
            ((ICDevID - DRIVER_OPTIONAL_DEVICE_COUNT) % \
               (3 + DRIVER_MAX_NOF_CHANNEL_IC_DEVICES)) - 1)

/** Calculate device ID in the device list from its CfyE API (EIP-163) device ID,
    see Note 1 in cs_driver.h. The same device ID is used for the CfyE and SecY
    devices in the same egress or ingress datapath. */
#define ADAPTER_EIP163_DEV_ID(CfyE_DevID)   ( (CfyE_DevID * 2) + 1 )

/** Calculate AIC device ID from its parent EIP-163 device ID */
#define ADAPTER_EIP163_ICDEV_ID(DevID, fGlobal, fPE, ChID) \
                DRIVER_ICDEV_ID(DevID, fGlobal, fPE, ChID)

/** Make an adapter-local name for DRIVER_DEVICE_ADD in the expansion of
    the ADAPTER_EIP164_DATAPATH macro */
#define DRIVER_DEVICE_ADD ADAPTER_EIP164_DEVICE_ADD
/** Adapter EIP-164 datapath definition macro */
#define ADAPTER_EIP164_DATAPATH DRIVER_DATAPATH

/** Adapter EIP-164 name */
#define ADAPTER_EIP164_NAME DRIVER_EIP164_NAME
/** Adapter EIP-163 name */
#define ADAPTER_EIP163_NAME DRIVER_EIP163_NAME

/** Maximum length in bytes of the device name in the device list */
#define ADAPTER_EIP164_MAX_DEVICE_NAME_LENGTH DRIVER_MAX_DEVICE_NAME_LENGTH

/** Maximum length in bytes of the device name in the device list */
#define ADAPTER_MAX_DEVICE_NAME_LENGTH DRIVER_MAX_DEVICE_NAME_LENGTH


/**  Disable EIP-163 device initialization register writes */
#ifdef DRIVER_DEV_INIT_DISABLE
#define ADAPTER_EIP163_DEV_INIT_DISABLE
#endif

/** Disable EIP-163 device uninitialization register writes */
#ifdef DRIVER_DEV_UNINIT_DISABLE
#define ADAPTER_EIP163_DEV_UNINIT_DISABLE
#endif


/****************************************************************************
 * Adapter EIP-164 configuration parameters
 */

/* enable debug checks */
#if !defined DRIVER_PERFORMANCE || defined ONLY_FOR_DOXYGEN
/** Enables additional debugging mechanisms. */
#define ADAPTER_EIP164_DBG
/** Enables strict argument checking for the input parameters in all
    the driver API functions implemented in the EIP-160 Adapter module. */
#define ADAPTER_EIP164_STRICT_ARGS
#endif

/** Specifies the driver name used for reporting in the log files and
    operating system. */
#ifdef DRIVER_NAME
#define ADAPTER_EIP164_DRIVER_NAME     DRIVER_NAME
#else
#define ADAPTER_EIP164_DRIVER_NAME     "SafeXcel"
#endif

/** Specifies the driver license type for the driver registration
    inside the Linux kernel. */
#ifdef DRIVER_LICENSE
#define ADAPTER_EIP164_LICENSE         DRIVER_LICENSE
#else
#define ADAPTER_EIP164_LICENSE         "Proprietary"
#endif

/** Does device support per-counter summary? */
#if defined DRIVER_PERCOUNTER_SUMMARY || defined ONLY_FOR_DOXYGEN
#define ADAPTER_EIP164_PERCOUNTER_SUMMARY_ENABLE
#endif


/** PCI configuration value: Cache Line Size, in 32bit words\n
    Advised value: 1 */
#define ADAPTER_PCICONFIG_CACHELINESIZE   1

/** PCI configuration value: Master Latency Timer, in PCI bus clocks\n
    Advised value: 0xf8 */
#ifdef ONLY_FOR_DOXYGEN
#define ADAPTER_PCICONFIG_MASTERLATENCYTIMER 0xf8
#else
/* #define ADAPTER_PCICONFIG_MASTERLATENCYTIMER 0xf8 */
#endif

/** Define this if the Driver is to be compiled for a platform that
    has 64-bit addresses. Leave this undefined for a 32-bit platform.
    This option is normally passed via the make file from the
    DDK_HOST64 environment variable. */
#if defined DRIVER_64BIT_HOST || defined ONLY_FOR_DOXYGEN
#define ADAPTER_EIP164_64BIT_HOST
#endif  /* DRIVER_64BIT_HOST */

/** Maximum number of EIP-164 devices supported by the driver. */
#ifdef DRIVER_MAX_NOF_EIP164_DEVICES
#define ADAPTER_EIP164_MAX_NOF_DEVICES DRIVER_MAX_NOF_EIP164_DEVICES
#endif

/** Name of the EIP-164 device 0. */
#define ADAPTER_EIP164_DEVICE_NAME_0   DRIVER_DEVICE_NAME_0
/** Name of the EIP-164 device 1. */
#define ADAPTER_EIP164_DEVICE_NAME_1   DRIVER_DEVICE_NAME_1

/** List of all EIP-164 devices with their names. */
#define ADAPTER_EIP164_DEVICES \
    ADAPTER_EIP164_DEVICE(ADAPTER_EIP164_DEVICE_NAME_0), \
    ADAPTER_EIP164_DEVICE(ADAPTER_EIP164_DEVICE_NAME_1)


/** EIP-164 device #0 Global AIC */
#define ADAPTER_EIP164_IC_NAME_0       DRIVER_IC_NAME_0

/** EIP-164 device #1 Global AIC */
#define ADAPTER_EIP164_IC_NAME_1       DRIVER_IC_NAME_1

/** EIP-164 device #0 PE AIC */
#define ADAPTER_EIP164_PE_IC_NAME_0       DRIVER_PE_IC_NAME_0

/** EIP-164 device #1 PE AIC */
#define ADAPTER_EIP164_PE_IC_NAME_1       DRIVER_PE_IC_NAME_1


/** EIP-164 device #0 Channel 0..64 AIC */
#define ADAPTER_EIP164_IC_NAME_0_CH(n)   DRIVER_IC_NAME_0_CH(n)

/** EIP-164 device #1 Channel 0..64 AIC */
#define ADAPTER_EIP164_IC_NAME_1_CH(n)   DRIVER_IC_NAME_1_CH(n)

/** Maximum number of EIP-201 devices in the EIP-164 device
    supported by the driver. */
#define ADAPTER_EIP164_MAX_IC_COUNT    DRIVER_MAX_NOF_EIP164_IC_DEVICES

/** List of all EIP-201 AIC devices used by the driver together with
    the ranges of interrupt numbers served by each of them.
    @note: The device order in the list is important,
    first Global AIC followed by by Packet Engine AIC then
    Channel AIC's for EIP-164 then
    Global AIC followed by Channel AIC's for EIP-163!
    Interrupt sources:     Name, Offset,  Count */
#ifdef ADAPTER_EIP163_MAX_IC_COUNT
/* EIP-201 Advanced Interrupt Controllers configuration */
/* Note: the device order in the list is important, */
/*       first Global AIC followed by Packet Engine AIC then */
/*       Channel AIC's for EIP-164 then */
/*       Global AIC followed by Channel AIC's for EIP-163! */
/*         Interrupt sources:     Name                   Offset Count */
/*               from ADAPTER_EIP164_INT_RESOURCES list (see below) */
#define ADAPTER_EIP164_IC_DEVICES                                         \
        /* EIP-164 Device Id 0, Global EIP-201 AIC */                     \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_0,      0, 12),      \
                                                                          \
        /* EIP-164 Device Id 0, Packet_Engine  EIP-201 AIC */             \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_PE_IC_NAME_0,   12, 0),      \
                                                                          \
        /* EIP-164 Device Id 0, Channel EIP-201 AIC's */                  \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_0_CH(0), 12, 6), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_0_CH(1), 12, 6), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_0_CH(2), 12, 6), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_0_CH(3), 12, 6), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_0_CH(4), 12, 6), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_0_CH(5), 12, 6), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_0_CH(6), 12, 6), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_0_CH(7), 12, 6), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_0_CH(8), 12, 6), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_0_CH(9), 12, 6), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_0_CH(10), 12, 6), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_0_CH(11), 12, 6), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_0_CH(12), 12, 6), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_0_CH(13), 12, 6), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_0_CH(14), 12, 6), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_0_CH(15), 12, 6), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_0_CH(16), 12, 6), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_0_CH(17), 12, 6), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_0_CH(18), 12, 6), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_0_CH(19), 12, 6), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_0_CH(20), 12, 6), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_0_CH(21), 12, 6), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_0_CH(22), 12, 6), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_0_CH(23), 12, 6), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_0_CH(24), 12, 6), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_0_CH(25), 12, 6), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_0_CH(26), 12, 6), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_0_CH(27), 12, 6), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_0_CH(28), 12, 6), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_0_CH(29), 12, 6), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_0_CH(30), 12, 6), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_0_CH(31), 12, 6), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_0_CH(32), 12, 6), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_0_CH(33), 12, 6), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_0_CH(34), 12, 6), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_0_CH(35), 12, 6), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_0_CH(36), 12, 6), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_0_CH(37), 12, 6), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_0_CH(38), 12, 6), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_0_CH(39), 12, 6), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_0_CH(40), 12, 6), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_0_CH(41), 12, 6), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_0_CH(42), 12, 6), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_0_CH(43), 12, 6), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_0_CH(44), 12, 6), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_0_CH(45), 12, 6), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_0_CH(46), 12, 6), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_0_CH(47), 12, 6), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_0_CH(48), 12, 6), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_0_CH(49), 12, 6), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_0_CH(50), 12, 6), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_0_CH(51), 12, 6), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_0_CH(52), 12, 6), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_0_CH(53), 12, 6), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_0_CH(54), 12, 6), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_0_CH(55), 12, 6), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_0_CH(56), 12, 6), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_0_CH(57), 12, 6), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_0_CH(58), 12, 6), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_0_CH(59), 12, 6), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_0_CH(60), 12, 6), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_0_CH(61), 12, 6), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_0_CH(62), 12, 6), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_0_CH(63), 12, 6), \
                                                                          \
        /* EIP-164 Device Id 1, Global EIP-201 AIC  */                    \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_1,     0, 12),       \
                                                                          \
        /* EIP-164 Device Id 1, Packet_Engine  EIP-201 AIC */             \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_PE_IC_NAME_1,  12, 0),       \
                                                                          \
        /* EIP-164 Device Id 1, Channel EIP-201 AIC's */                  \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_1_CH(0), 12, 6), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_1_CH(1), 12, 6), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_1_CH(2), 12, 6), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_1_CH(3), 12, 6), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_1_CH(4), 12, 6), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_1_CH(5), 12, 6), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_1_CH(6), 12, 6), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_1_CH(7), 12, 6), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_1_CH(8), 12, 6), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_1_CH(9), 12, 6), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_1_CH(10), 12, 6), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_1_CH(11), 12, 6), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_1_CH(12), 12, 6), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_1_CH(13), 12, 6), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_1_CH(14), 12, 6), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_1_CH(15), 12, 6), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_1_CH(16), 12, 6), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_1_CH(17), 12, 6), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_1_CH(18), 12, 6), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_1_CH(19), 12, 6), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_1_CH(20), 12, 6), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_1_CH(21), 12, 6), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_1_CH(22), 12, 6), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_1_CH(23), 12, 6), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_1_CH(24), 12, 6), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_1_CH(25), 12, 6), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_1_CH(26), 12, 6), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_1_CH(27), 12, 6), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_1_CH(28), 12, 6), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_1_CH(29), 12, 6), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_1_CH(30), 12, 6), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_1_CH(31), 12, 6), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_1_CH(32), 12, 6), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_1_CH(33), 12, 6), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_1_CH(34), 12, 6), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_1_CH(35), 12, 6), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_1_CH(36), 12, 6), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_1_CH(37), 12, 6), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_1_CH(38), 12, 6), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_1_CH(39), 12, 6), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_1_CH(40), 12, 6), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_1_CH(41), 12, 6), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_1_CH(42), 12, 6), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_1_CH(43), 12, 6), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_1_CH(44), 12, 6), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_1_CH(45), 12, 6), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_1_CH(46), 12, 6), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_1_CH(47), 12, 6), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_1_CH(48), 12, 6), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_1_CH(49), 12, 6), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_1_CH(50), 12, 6), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_1_CH(51), 12, 6), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_1_CH(52), 12, 6), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_1_CH(53), 12, 6), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_1_CH(54), 12, 6), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_1_CH(55), 12, 6), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_1_CH(56), 12, 6), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_1_CH(57), 12, 6), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_1_CH(58), 12, 6), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_1_CH(59), 12, 6), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_1_CH(60), 12, 6), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_1_CH(61), 12, 6), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_1_CH(62), 12, 6), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_1_CH(63), 12, 6), \
                                                                          \
        /* EIP-163 Device Id 0, Global EIP-201 AIC  */                    \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP163_IC_NAME_0,        18,  3),   \
                                                                          \
        /* EIP-163 Device Id 0, Channel EIP-201 AIC's */                  \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP163_CE_IC_NAME_0_CH(0), 21, 12), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP163_CE_IC_NAME_0_CH(1), 21, 12), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP163_CE_IC_NAME_0_CH(2), 21, 12), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP163_CE_IC_NAME_0_CH(3), 21, 12), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP163_CE_IC_NAME_0_CH(4), 21, 12), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP163_CE_IC_NAME_0_CH(5), 21, 12), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP163_CE_IC_NAME_0_CH(6), 21, 12), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP163_CE_IC_NAME_0_CH(7), 21, 12), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP163_CE_IC_NAME_0_CH(8), 21, 12), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP163_CE_IC_NAME_0_CH(9), 21, 12), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP163_CE_IC_NAME_0_CH(10), 21, 12), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP163_CE_IC_NAME_0_CH(11), 21, 12), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP163_CE_IC_NAME_0_CH(12), 21, 12), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP163_CE_IC_NAME_0_CH(13), 21, 12), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP163_CE_IC_NAME_0_CH(14), 21, 12), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP163_CE_IC_NAME_0_CH(15), 21, 12), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP163_CE_IC_NAME_0_CH(16), 21, 12), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP163_CE_IC_NAME_0_CH(17), 21, 12), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP163_CE_IC_NAME_0_CH(18), 21, 12), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP163_CE_IC_NAME_0_CH(19), 21, 12), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP163_CE_IC_NAME_0_CH(20), 21, 12), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP163_CE_IC_NAME_0_CH(21), 21, 12), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP163_CE_IC_NAME_0_CH(22), 21, 12), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP163_CE_IC_NAME_0_CH(23), 21, 12), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP163_CE_IC_NAME_0_CH(24), 21, 12), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP163_CE_IC_NAME_0_CH(25), 21, 12), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP163_CE_IC_NAME_0_CH(26), 21, 12), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP163_CE_IC_NAME_0_CH(27), 21, 12), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP163_CE_IC_NAME_0_CH(28), 21, 12), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP163_CE_IC_NAME_0_CH(29), 21, 12), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP163_CE_IC_NAME_0_CH(30), 21, 12), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP163_CE_IC_NAME_0_CH(31), 21, 12), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP163_CE_IC_NAME_0_CH(32), 21, 12), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP163_CE_IC_NAME_0_CH(33), 21, 12), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP163_CE_IC_NAME_0_CH(34), 21, 12), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP163_CE_IC_NAME_0_CH(35), 21, 12), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP163_CE_IC_NAME_0_CH(36), 21, 12), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP163_CE_IC_NAME_0_CH(37), 21, 12), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP163_CE_IC_NAME_0_CH(38), 21, 12), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP163_CE_IC_NAME_0_CH(39), 21, 12), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP163_CE_IC_NAME_0_CH(40), 21, 12), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP163_CE_IC_NAME_0_CH(41), 21, 12), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP163_CE_IC_NAME_0_CH(42), 21, 12), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP163_CE_IC_NAME_0_CH(43), 21, 12), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP163_CE_IC_NAME_0_CH(44), 21, 12), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP163_CE_IC_NAME_0_CH(45), 21, 12), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP163_CE_IC_NAME_0_CH(46), 21, 12), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP163_CE_IC_NAME_0_CH(47), 21, 12), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP163_CE_IC_NAME_0_CH(48), 21, 12), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP163_CE_IC_NAME_0_CH(49), 21, 12), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP163_CE_IC_NAME_0_CH(50), 21, 12), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP163_CE_IC_NAME_0_CH(51), 21, 12), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP163_CE_IC_NAME_0_CH(52), 21, 12), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP163_CE_IC_NAME_0_CH(53), 21, 12), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP163_CE_IC_NAME_0_CH(54), 21, 12), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP163_CE_IC_NAME_0_CH(55), 21, 12), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP163_CE_IC_NAME_0_CH(56), 21, 12), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP163_CE_IC_NAME_0_CH(57), 21, 12), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP163_CE_IC_NAME_0_CH(58), 21, 12), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP163_CE_IC_NAME_0_CH(59), 21, 12), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP163_CE_IC_NAME_0_CH(60), 21, 12), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP163_CE_IC_NAME_0_CH(61), 21, 12), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP163_CE_IC_NAME_0_CH(62), 21, 12), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP163_CE_IC_NAME_0_CH(63), 21, 12), \
                                                                          \
        /* EIP-163 Device Id 1, Global EIP-201 AIC  */                    \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP163_IC_NAME_1,        18,  3),   \
                                                                          \
        /* EIP-163 Device Id 1, Channel EIP-201 AIC's */                  \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP163_CE_IC_NAME_1_CH(0), 21, 12),\
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP163_CE_IC_NAME_1_CH(1), 21, 12),\
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP163_CE_IC_NAME_1_CH(2), 21, 12),\
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP163_CE_IC_NAME_1_CH(3), 21, 12),\
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP163_CE_IC_NAME_1_CH(4), 21, 12),\
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP163_CE_IC_NAME_1_CH(5), 21, 12),\
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP163_CE_IC_NAME_1_CH(6), 21, 12),\
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP163_CE_IC_NAME_1_CH(7), 21, 12),\
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP163_CE_IC_NAME_1_CH(8), 21, 12),\
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP163_CE_IC_NAME_1_CH(9), 21, 12),\
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP163_CE_IC_NAME_1_CH(10), 21, 12),\
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP163_CE_IC_NAME_1_CH(11), 21, 12),\
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP163_CE_IC_NAME_1_CH(12), 21, 12),\
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP163_CE_IC_NAME_1_CH(13), 21, 12),\
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP163_CE_IC_NAME_1_CH(14), 21, 12),\
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP163_CE_IC_NAME_1_CH(15), 21, 12),\
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP163_CE_IC_NAME_1_CH(16), 21, 12),\
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP163_CE_IC_NAME_1_CH(17), 21, 12),\
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP163_CE_IC_NAME_1_CH(18), 21, 12),\
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP163_CE_IC_NAME_1_CH(19), 21, 12),\
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP163_CE_IC_NAME_1_CH(20), 21, 12),\
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP163_CE_IC_NAME_1_CH(21), 21, 12),\
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP163_CE_IC_NAME_1_CH(22), 21, 12),\
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP163_CE_IC_NAME_1_CH(23), 21, 12),\
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP163_CE_IC_NAME_1_CH(24), 21, 12),\
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP163_CE_IC_NAME_1_CH(25), 21, 12),\
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP163_CE_IC_NAME_1_CH(26), 21, 12),\
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP163_CE_IC_NAME_1_CH(27), 21, 12),\
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP163_CE_IC_NAME_1_CH(28), 21, 12),\
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP163_CE_IC_NAME_1_CH(29), 21, 12),\
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP163_CE_IC_NAME_1_CH(30), 21, 12),\
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP163_CE_IC_NAME_1_CH(31), 21, 12),\
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP163_CE_IC_NAME_1_CH(32), 21, 12),\
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP163_CE_IC_NAME_1_CH(33), 21, 12),\
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP163_CE_IC_NAME_1_CH(34), 21, 12),\
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP163_CE_IC_NAME_1_CH(35), 21, 12),\
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP163_CE_IC_NAME_1_CH(36), 21, 12),\
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP163_CE_IC_NAME_1_CH(37), 21, 12),\
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP163_CE_IC_NAME_1_CH(38), 21, 12),\
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP163_CE_IC_NAME_1_CH(39), 21, 12),\
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP163_CE_IC_NAME_1_CH(40), 21, 12),\
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP163_CE_IC_NAME_1_CH(41), 21, 12),\
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP163_CE_IC_NAME_1_CH(42), 21, 12),\
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP163_CE_IC_NAME_1_CH(43), 21, 12),\
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP163_CE_IC_NAME_1_CH(44), 21, 12),\
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP163_CE_IC_NAME_1_CH(45), 21, 12),\
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP163_CE_IC_NAME_1_CH(46), 21, 12),\
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP163_CE_IC_NAME_1_CH(47), 21, 12),\
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP163_CE_IC_NAME_1_CH(48), 21, 12),\
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP163_CE_IC_NAME_1_CH(49), 21, 12),\
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP163_CE_IC_NAME_1_CH(50), 21, 12),\
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP163_CE_IC_NAME_1_CH(51), 21, 12),\
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP163_CE_IC_NAME_1_CH(52), 21, 12),\
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP163_CE_IC_NAME_1_CH(53), 21, 12),\
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP163_CE_IC_NAME_1_CH(54), 21, 12),\
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP163_CE_IC_NAME_1_CH(55), 21, 12),\
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP163_CE_IC_NAME_1_CH(56), 21, 12),\
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP163_CE_IC_NAME_1_CH(57), 21, 12),\
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP163_CE_IC_NAME_1_CH(58), 21, 12),\
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP163_CE_IC_NAME_1_CH(59), 21, 12),\
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP163_CE_IC_NAME_1_CH(60), 21, 12),\
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP163_CE_IC_NAME_1_CH(61), 21, 12),\
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP163_CE_IC_NAME_1_CH(62), 21, 12),\
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP163_CE_IC_NAME_1_CH(63), 21, 12)
#else /* ADAPTER_EIP163_MAX_IC_COUNT */
/* EIP-201 Advanced Interrupt Controllers configuration */
/* Note: the device order in the list is important, */
/*       first Global AIC followed by Channel AIC's for EIP-164! */
#define ADAPTER_EIP164_IC_DEVICES                                     \
        /* EIP-164 Device Id 0, Global EIP-201 AIC */                 \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_0,      0, 12),  \
                                                                      \
        /* EIP-164 Device Id 0, Packet_Engine  EIP-201 AIC */         \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_PE_IC_NAME_0,  12, 0),   \
                                                                      \
        /* EIP-164 Device Id 0, Channel EIP-201 AIC's */              \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_0_CH(0), 12, 6), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_0_CH(1), 12, 6), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_0_CH(2), 12, 6), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_0_CH(3), 12, 6), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_0_CH(4), 12, 6), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_0_CH(5), 12, 6), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_0_CH(6), 12, 6), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_0_CH(7), 12, 6), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_0_CH(8), 12, 6), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_0_CH(9), 12, 6), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_0_CH(10), 12, 6), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_0_CH(11), 12, 6), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_0_CH(12), 12, 6), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_0_CH(13), 12, 6), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_0_CH(14), 12, 6), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_0_CH(15), 12, 6), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_0_CH(16), 12, 6), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_0_CH(17), 12, 6), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_0_CH(18), 12, 6), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_0_CH(19), 12, 6), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_0_CH(20), 12, 6), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_0_CH(21), 12, 6), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_0_CH(22), 12, 6), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_0_CH(23), 12, 6), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_0_CH(24), 12, 6), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_0_CH(25), 12, 6), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_0_CH(26), 12, 6), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_0_CH(27), 12, 6), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_0_CH(28), 12, 6), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_0_CH(29), 12, 6), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_0_CH(30), 12, 6), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_0_CH(31), 12, 6), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_0_CH(32), 12, 6), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_0_CH(33), 12, 6), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_0_CH(34), 12, 6), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_0_CH(35), 12, 6), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_0_CH(36), 12, 6), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_0_CH(37), 12, 6), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_0_CH(38), 12, 6), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_0_CH(39), 12, 6), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_0_CH(40), 12, 6), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_0_CH(41), 12, 6), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_0_CH(42), 12, 6), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_0_CH(43), 12, 6), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_0_CH(44), 12, 6), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_0_CH(45), 12, 6), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_0_CH(46), 12, 6), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_0_CH(47), 12, 6), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_0_CH(48), 12, 6), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_0_CH(49), 12, 6), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_0_CH(50), 12, 6), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_0_CH(51), 12, 6), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_0_CH(52), 12, 6), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_0_CH(53), 12, 6), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_0_CH(54), 12, 6), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_0_CH(55), 12, 6), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_0_CH(56), 12, 6), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_0_CH(57), 12, 6), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_0_CH(58), 12, 6), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_0_CH(59), 12, 6), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_0_CH(60), 12, 6), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_0_CH(61), 12, 6), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_0_CH(62), 12, 6), \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_0_CH(63), 12, 6), \
                                                                      \
        /* EIP-164 Device Id 1, Global EIP-201 AIC  */                \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_1,     0, 12),   \
                                                                      \
        /* EIP-164 Device Id 1, Packet_Engine  EIP-201 AIC */         \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_PE_IC_NAME_1,  12, 0),   \
                                                                      \
        /* EIP-164 Device Id 1, Channel EIP-201 AIC's */              \
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_1_CH(0), 12, 6),\
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_1_CH(1), 12, 6),\
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_1_CH(2), 12, 6),\
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_1_CH(3), 12, 6),\
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_1_CH(4), 12, 6),\
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_1_CH(5), 12, 6),\
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_1_CH(6), 12, 6),\
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_1_CH(7), 12, 6),\
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_1_CH(8), 12, 6),\
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_1_CH(9), 12, 6),\
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_1_CH(10), 12, 6),\
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_1_CH(11), 12, 6),\
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_1_CH(12), 12, 6),\
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_1_CH(13), 12, 6),\
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_1_CH(14), 12, 6),\
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_1_CH(15), 12, 6),\
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_1_CH(16), 12, 6),\
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_1_CH(17), 12, 6),\
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_1_CH(18), 12, 6),\
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_1_CH(19), 12, 6),\
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_1_CH(20), 12, 6),\
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_1_CH(21), 12, 6),\
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_1_CH(22), 12, 6),\
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_1_CH(23), 12, 6),\
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_1_CH(24), 12, 6),\
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_1_CH(25), 12, 6),\
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_1_CH(26), 12, 6),\
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_1_CH(27), 12, 6),\
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_1_CH(28), 12, 6),\
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_1_CH(29), 12, 6),\
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_1_CH(30), 12, 6),\
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_1_CH(31), 12, 6),\
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_1_CH(32), 12, 6),\
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_1_CH(33), 12, 6),\
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_1_CH(34), 12, 6),\
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_1_CH(35), 12, 6),\
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_1_CH(36), 12, 6),\
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_1_CH(37), 12, 6),\
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_1_CH(38), 12, 6),\
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_1_CH(39), 12, 6),\
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_1_CH(40), 12, 6),\
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_1_CH(41), 12, 6),\
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_1_CH(42), 12, 6),\
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_1_CH(43), 12, 6),\
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_1_CH(44), 12, 6),\
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_1_CH(45), 12, 6),\
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_1_CH(46), 12, 6),\
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_1_CH(47), 12, 6),\
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_1_CH(48), 12, 6),\
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_1_CH(49), 12, 6),\
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_1_CH(50), 12, 6),\
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_1_CH(51), 12, 6),\
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_1_CH(52), 12, 6),\
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_1_CH(53), 12, 6),\
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_1_CH(54), 12, 6),\
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_1_CH(55), 12, 6),\
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_1_CH(56), 12, 6),\
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_1_CH(57), 12, 6),\
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_1_CH(58), 12, 6),\
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_1_CH(59), 12, 6),\
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_1_CH(60), 12, 6),\
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_1_CH(61), 12, 6),\
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_1_CH(62), 12, 6),\
        ADAPTER_EIP164_IC_ADD(ADAPTER_EIP164_IC_NAME_1_CH(63), 12, 6)
#endif /* !ADAPTER_EIP163_MAX_IC_COUNT */

/** Determines the trace filter for the device interrupts. 0x3FFFF will
    trace all the device interrupts. */
#define ADAPTER_EIP164_INTERRUPTS_TRACEFILTER 0x00000FFF

/** The Driver can be used with or without interrupts. It provides an
    API function that can be used to register a callback to be invoked on
    receiving device interrupts. These callbacks typically read the device
    status or trigger this functionality in other execution contexts.\n
    When interrupts are off, the Driver is said to be in 'polling mode'.\n
    In this mode, the caller of the Driver API should poll the device
    registers for the device status. When interrupts are enabled, result
    notification callbacks are invoked asynchronously with respect to
    the main application. */
#ifdef DRIVER_INTERRUPTS
#define ADAPTER_EIP164_INTERRUPTS_ENABLE
#endif

/** Clear the statistics counters after reading in one of two ways:
    clear on read or decrement on write */
#ifdef DRIVER_WRITE_TO_DECREMENT
/** EIP-163 Clear-on read for statistics */
#define ADAPTER_EIP163_CLEAR_ON_READ            0
/** EIP-164 Clear-on read for statistics */
#define ADAPTER_EIP164_CLEAR_ON_READ            0
/** EIP-163 Write-to-decrement  for statistics */
#define ADAPTER_EIP163_WRITE_TO_DECREMENT       1
/** EIP-164 Write-to-decrement for statistics */
#define ADAPTER_EIP164_WRITE_TO_DECREMENT       1
#else
/** EIP-163 Clear-on read for statistics */
#define ADAPTER_EIP163_CLEAR_ON_READ            1
/** EIP-164 Clear-on read for statistics */
#define ADAPTER_EIP164_CLEAR_ON_READ            1
/** EIP-163 Write-to-decrement  for statistics */
#define ADAPTER_EIP163_WRITE_TO_DECREMENT       0
/** EIP-164 Write-to-decrement for statistics */
#define ADAPTER_EIP164_WRITE_TO_DECREMENT       0
#endif

/* EIP-164 Global AIC interrupt resources */
/** EIP-164 Drop Class interrupt number */
#define ADAPTER_EIP164_PHY_DROP_CLASS_IRQ       0
/** EIP-164 Drop Class interrupt name */
#define ADAPTER_EIP164_DROP_CLASS_INT_NAME      "EIP164-DROP-CLASS"

/** EIP-164 Drop PP interrupt number */
#define ADAPTER_EIP164_PHY_DROP_PP_IRQ          1
/** EIP-164 Drop PP interrupt name */
#define ADAPTER_EIP164_DROP_PP_INT_NAME         "EIP164-DROP-PP"

/** EIP-164 Drop MTU interrupt number */
#define ADAPTER_EIP164_PHY_DROP_MTU_IRQ         2
/** EIP-164 Drop MTU interrupt name */
#define ADAPTER_EIP164_DROP_MTU_INT_NAME        "EIP164-DROP-MTU"

/** EIP-164 - EIP66 interrupt number */
#define ADAPTER_EIP66_PHY_IRQ                   3
/** EIP-164 - EIP66 interrupt name */
#define ADAPTER_EIP66_INT_NAME                  "EIP66-IRQ"

/** EIP-164 Statistics SA Threashold interrupt number */
#define ADAPTER_EIP164_PHY_STAT_SA_THR_IRQ      4
/** EIP-164 Statistics SA Threashold interrupt name */
#define ADAPTER_EIP164_STAT_SA_THR_INT_NAME     "EIP164-STAT-SA-THR"

/** EIP-164 Statistics SecY Threashold interrupt number */
#define ADAPTER_EIP164_PHY_STAT_SECY_THR_IRQ    5
/** EIP-164 Statistics SecY Threashold interrupt name */
#define ADAPTER_EIP164_STAT_SECY_THR_INT_NAME   "EIP164-STAT-SECY-THR"

/** EIP-164 Statistics IFC Threashold interrupt number */
#define ADAPTER_EIP164_PHY_STAT_IFC_THR_IRQ     6
/** EIP-164 Statistics IFC Threashold interrupt name */
#define ADAPTER_EIP164_STAT_IFC_THR_INT_NAME    "EIP164-STAT-IFC-THR"

/** EIP-164 Statistics IFC1 Threshold interrupt number */
#define ADAPTER_EIP164_PHY_STAT_IFC1_THR_IRQ    7
/** EIP-164 Statistics IFC1 Threshold interrupt name */
#define ADAPTER_EIP164_STAT_IFC1_THR_INT_NAME   "EIP164-STAT-IFC1-THR"

/** EIP-164 Statistics RxCAM Threshold interrupt number */
#define ADAPTER_EIP164_PHY_STAT_RXCAM_THR_IRQ   8
/** EIP-164 Statistics RxCAM Threshold interrupt name */
#define ADAPTER_EIP164_STAT_RXCAM_THR_INT_NAME  "EIP164-STAT-RXCAM-THR"

/** EIP-164 SA Packet Number Threshold interrupt number */
#define ADAPTER_EIP164_PHY_SA_PN_THR_IRQ        10
/** EIP-164 SA Packet Number Threshold interrupt name */
#define ADAPTER_EIP164_SA_PN_THR_INT_NAME       "EIP164-SA-PN-THR"

/** EIP-164 Statistics SA Expired interrupt number */
#define ADAPTER_EIP164_PHY_SA_EXPIRED_IRQ       11
/** EIP-164 Statistics SA Expired interrupt name */
#define ADAPTER_EIP164_SA_EXPIRED_INT_NAME      "EIP164-SA-EXPIRED"


/** EIP-164 ECC Error  interrupt number */
#define ADAPTER_EIP164_PHY_ECC_ERR_IRQ          12
/** EIP-164 ECC Error interrupt name */
#define ADAPTER_EIP164_ECC_ERR_INT_NAME         "EIP164-ECC-ERR"

/* EIP-164 Channel AIC interrupt resources */
/** EIP-164 Channel Error SOP without EOP interrupt number */
#define ADAPTER_EIP164_CHAN_ERR_SOP_WO_EOP_IRQ       0
/** EIP-164 Statistics interrupt name */
#define ADAPTER_EIP164_CHAN_ERR_SOP_WO_EOP_INT_NAME  "EIP164-CHAN-ERR-SOP-WO-EOP"

/** EIP-164 Channel Error EOP without SOP interrupt number */
#define ADAPTER_EIP164_CHAN_ERR_EOP_WO_SOP_IRQ       1
/** EIP-164 Channel Error EOP without SOP interrupt name */
#define ADAPTER_EIP164_CHAN_ERR_EOP_WO_SOP_INT_NAME  "EIP164-CHAN-ERR-EOP-WO-SOP"

/** EIP-164 Channel Error Transfer without SOP interrupt number */
#define ADAPTER_EIP164_CHAN_ERR_XFER_WO_SOP_IRQ      2
/** EIP-164 Channel Error Transfer without SOP interrupt name */
#define ADAPTER_EIP164_CHAN_ERR_XFER_WO_SOP_INT_NAME "EIP164-CHAN-ERR-XFER-WO-SOP"

/** EIP-164 Channel RxCAM Multiple Hit interrupt number */
#define ADAPTER_EIP164_CHAN_RXCAM_HIT_MULT_IRQ       6
/** EIP-164 Channel RxCAM Multiple Hit interrupt name */
#define ADAPTER_EIP164_CHAN_RXCAM_HIT_MULT_INT_NAME  "EIP164-CHAN-RXCAM-HIT-MULT"

/** EIP-164 Channel RxCAM Miss interrupt number */
#define ADAPTER_EIP164_CHAN_RXCAM_MISS_IRQ           7
/** EIP-164 Channel RxCAM Miss interrupt name */
#define ADAPTER_EIP164_CHAN_RXCAM_MISS_INT_NAME      "EIP164-CHAN-RXCAM-MISS"

/** EIP-164 Channel Data Packet Overrun interrupt number */
#define ADAPTER_EIP164_CHAN_PKT_DATA_OVERRUN_IRQ       8
/** EIP-164 Channel Data Packet Overrun interrupt name */
#define ADAPTER_EIP164_CHAN_PKT_DATA_OVERRUN_INT_NAME  "EIP164-CHAN-PKT-DATA-OVERRUN"


/** List of all interrupts used by the adapter with the interrupt controllers
    they are served by, their physical numbers and their type and polarity.
    The interrupt sources is a bitmask of interrupt sources
    Refer to the data sheet of device for the correct values\n
    Config: Choose from RISING_EDGE, FALLING_EDGE, ACTIVE_HIGH, ACTIVE_LOW\n
    Interrupt sources:     Name, Inputs,   Config */
#define ADAPTER_EIP164_INT_RESOURCES \
    /* EIP-164 Global AIC interrupt resources (12), level and edge sensitive */ \
    ADAPTER_EIP164_INT_RESOURCE_ADD(ADAPTER_EIP164_DROP_CLASS_INT_NAME, \
                                    1<<ADAPTER_EIP164_PHY_DROP_CLASS_IRQ, \
                                    RISING_EDGE), \
    ADAPTER_EIP164_INT_RESOURCE_ADD(ADAPTER_EIP164_DROP_PP_INT_NAME, \
                                    1<<ADAPTER_EIP164_PHY_DROP_PP_IRQ, \
                                    RISING_EDGE), \
    ADAPTER_EIP164_INT_RESOURCE_ADD(ADAPTER_EIP164_DROP_MTU_INT_NAME, \
                                    1<<ADAPTER_EIP164_PHY_DROP_MTU_IRQ, \
                                    RISING_EDGE), \
    ADAPTER_EIP164_INT_RESOURCE_ADD(ADAPTER_EIP66_INT_NAME, \
                                    1<<ADAPTER_EIP66_PHY_IRQ, \
                                    RISING_EDGE), \
    ADAPTER_EIP164_INT_RESOURCE_ADD(ADAPTER_EIP164_STAT_SA_THR_INT_NAME, \
                                    1<<ADAPTER_EIP164_PHY_STAT_SA_THR_IRQ, \
                                    ACTIVE_HIGH), \
    ADAPTER_EIP164_INT_RESOURCE_ADD(ADAPTER_EIP164_STAT_SECY_THR_INT_NAME, \
                                    1<<ADAPTER_EIP164_PHY_STAT_SECY_THR_IRQ, \
                                    ACTIVE_HIGH), \
    ADAPTER_EIP164_INT_RESOURCE_ADD(ADAPTER_EIP164_STAT_IFC_THR_INT_NAME, \
                                    1<<ADAPTER_EIP164_PHY_STAT_IFC_THR_IRQ, \
                                    ACTIVE_HIGH), \
    ADAPTER_EIP164_INT_RESOURCE_ADD(ADAPTER_EIP164_STAT_IFC1_THR_INT_NAME, \
                                    1<<(ADAPTER_EIP164_PHY_STAT_IFC1_THR_IRQ), \
                                    ACTIVE_HIGH), \
    ADAPTER_EIP164_INT_RESOURCE_ADD(ADAPTER_EIP164_STAT_RXCAM_THR_INT_NAME, \
                                    1<<(ADAPTER_EIP164_PHY_STAT_RXCAM_THR_IRQ), \
                                    ACTIVE_HIGH), \
    ADAPTER_EIP164_INT_RESOURCE_ADD(ADAPTER_EIP164_SA_PN_THR_INT_NAME, \
                                    1<<(ADAPTER_EIP164_PHY_SA_PN_THR_IRQ), \
                                    ACTIVE_HIGH), \
    ADAPTER_EIP164_INT_RESOURCE_ADD(ADAPTER_EIP164_SA_EXPIRED_INT_NAME, \
                                    1<<(ADAPTER_EIP164_PHY_SA_EXPIRED_IRQ), \
                                    ACTIVE_HIGH), \
    ADAPTER_EIP164_INT_RESOURCE_ADD(ADAPTER_EIP164_ECC_ERR_INT_NAME, \
                                    1<<(ADAPTER_EIP164_PHY_ECC_ERR_IRQ), \
                                    ACTIVE_HIGH), \
    /* EIP-164 Channel AIC interrupt resources (10), edge sensitive */ \
    ADAPTER_EIP164_INT_RESOURCE_ADD(ADAPTER_EIP164_CHAN_ERR_SOP_WO_EOP_INT_NAME, \
                                    1<<ADAPTER_EIP164_CHAN_ERR_SOP_WO_EOP_IRQ, \
                                    RISING_EDGE), \
    ADAPTER_EIP164_INT_RESOURCE_ADD(ADAPTER_EIP164_CHAN_ERR_EOP_WO_SOP_INT_NAME, \
                                    1<<ADAPTER_EIP164_CHAN_ERR_EOP_WO_SOP_IRQ, \
                                    RISING_EDGE), \
    ADAPTER_EIP164_INT_RESOURCE_ADD(ADAPTER_EIP164_CHAN_ERR_XFER_WO_SOP_INT_NAME, \
                                    1<<ADAPTER_EIP164_CHAN_ERR_XFER_WO_SOP_IRQ, \
                                    RISING_EDGE), \
    ADAPTER_EIP164_INT_RESOURCE_ADD(ADAPTER_EIP164_CHAN_RXCAM_HIT_MULT_INT_NAME, \
                                    1<<ADAPTER_EIP164_CHAN_RXCAM_HIT_MULT_IRQ, \
                                    RISING_EDGE), \
    ADAPTER_EIP164_INT_RESOURCE_ADD(ADAPTER_EIP164_CHAN_RXCAM_MISS_INT_NAME, \
                                    1<<ADAPTER_EIP164_CHAN_RXCAM_MISS_IRQ, \
                                    RISING_EDGE), \
    ADAPTER_EIP164_INT_RESOURCE_ADD(ADAPTER_EIP164_CHAN_PKT_DATA_OVERRUN_INT_NAME, \
                                    1<<ADAPTER_EIP164_CHAN_PKT_DATA_OVERRUN_IRQ, \
                                    RISING_EDGE), \
    ADAPTER_EIP163_INT_RESOURCES

/** Maximum number of EIP-201 devices in the EIP-163 device
    supported by the driver. */
#ifdef ADAPTER_EIP163_MAX_IC_COUNT
#define ADAPTER_INTERRUPTS_MAX_COUNT    (ADAPTER_EIP164_MAX_IC_COUNT + \
                                         ADAPTER_EIP163_MAX_IC_COUNT)
#endif

/** Select Egress mode */
#ifdef DRIVER_MODE_EGRESS
#define ADAPTER_EIP164_MODE_EGRESS
#endif

/** Select Ingress mode */
#ifdef DRIVER_MODE_INGRESS
#define ADAPTER_EIP164_MODE_INGRESS
#endif

/** Maximum number of attempts for the Driver during the device initialization
    to get the device status initialized. */
#ifdef DRIVER_MAX_NOF_INIT_RETRY_COUNT
#define ADAPTER_EIP164_MAX_NOF_INIT_RETRY_COUNT DRIVER_MAX_NOF_INIT_RETRY_COUNT
#endif

/** Timeout in milliseconds between two consecutive status polls for the
    Driver during the device initialization. */
#ifdef DRIVER_INIT_RETRY_TIMEOUT_MS
#define ADAPTER_EIP164_INIT_RETRY_TIMEOUT_MS    DRIVER_INIT_RETRY_TIMEOUT_MS
#endif

/** Maximum number of attempts for the Driver during the device uninitialization
    to get the device status uninitialized. */
#ifdef DRIVER_MAX_NOF_UNINIT_RETRY_COUNT
#define ADAPTER_EIP164_MAX_NOF_UNINIT_RETRY_COUNT DRIVER_MAX_NOF_UNINIT_RETRY_COUNT
#endif

/** Timeout in milliseconds between two consecutive status polls for the
    Driver during the device uninitialization. */
#ifdef DRIVER_UNINIT_RETRY_TIMEOUT_MS
#define ADAPTER_EIP164_UNINIT_RETRY_TIMEOUT_MS    DRIVER_UNINIT_RETRY_TIMEOUT_MS
#endif

/** Maximum number of status polls for the Driver for the
    host-device synchronization. */
#ifdef DRIVER_MAX_NOF_SYNC_RETRY_COUNT
#define ADAPTER_EIP164_MAX_NOF_SYNC_RETRY_COUNT DRIVER_MAX_NOF_SYNC_RETRY_COUNT
#endif

/** Timeout in milliseconds between two consecutive status polls for the
    Driver during the hostdevice synchronization. */
#ifdef DRIVER_SYNC_RETRY_TIMEOUT_MS
#define ADAPTER_EIP164_SYNC_RETRY_TIMEOUT_MS    DRIVER_SYNC_RETRY_TIMEOUT_MS
#endif

#if defined DRIVER_WARMBOOT || defined ONLY_FOR_DOXYGEN
/** Enable overall WarmBoot functionality in the adapter */
#define ADAPTER_WARMBOOT_ENABLE
/** Enable CfyE Adapter */
#define ADAPTER_WARMBOOT_CFYE_ENABLE
/** Enable WarmBoot in the EIP164 Adapter */
#define ADAPTER_EIP164_WARMBOOT_ENABLE
#if defined ADAPTER_WARMBOOT_CFYE_ENABLE || ONLY_FOR_DOXYGEN
/** Enable WarmBoot in the EIP163 Adapter */
#define ADAPTER_EIP163_WARMBOOT_ENABLE
#endif
#endif

/** If this parameter is defined, the driver will support basic
    crypt-authenticate operations (nonMACsec), which can be used for test purposes. */
#define ADAPTER_EIP164_CRYPT_AUTH_ENABLE


/** Calculate Adapter EIP-164 notification ID from EIP-201 AIC device ID,
 * see Note 1 in cs_driver.h. One notification is used per one EIP-164 AIC
 * device.*/
#define ADAPTER_EIP164_NOTIFY_ID(ICDevID) \
    (((((ICDevID - DRIVER_OPTIONAL_DEVICE_COUNT) / \
       (3 + DRIVER_MAX_NOF_CHANNEL_IC_DEVICES)) / 2) * \
         (2 + DRIVER_MAX_NOF_CHANNEL_IC_DEVICES)) + \
           ((ICDevID - DRIVER_OPTIONAL_DEVICE_COUNT) % \
               (3 + DRIVER_MAX_NOF_CHANNEL_IC_DEVICES)) - 1)

/** Calculate device ID in the device list from its SecY API (EIP-164) device ID,
 * see Note 1 in cs_driver.h. The same device ID is used for the CfyE and SecY
 * devices in the same egress or ingress datapath. */
#define ADAPTER_EIP164_DEV_ID(SecY_DevID)   ( SecY_DevID * 2 )



/** Calculate AIC device ID from its parent EIP-164 device ID */
#define ADAPTER_EIP164_ICDEV_ID         DRIVER_ICDEV_ID

/** Calculate Adapter EIP-164 interrupt handler ID from EIP-201 AIC device ID */
#define ADAPTER_EIP164_INT_HANDLER_ID   DRIVER_INT_HANDLER_ID

/** Disable EIP-164 device initialization register writes */
#ifdef DRIVER_DEV_INIT_DISABLE
#define ADAPTER_EIP164_DEV_INIT_DISABLE
#endif

/** Disable EIP-164 device uninitialization register writes */
#ifdef DRIVER_DEV_UNINIT_DISABLE
#define ADAPTER_EIP164_DEV_UNINIT_DISABLE
#endif

#endif /* INCLUDE_GUARD_CS_ADAPTER_H */

/* end of file cs_adapter.h */
