/* cs_hwpal_ext.h
 *
 * SafeXcel-IP-164 (HAWK) chip specific configuration parameters
 */

/*****************************************************************************
* Copyright (c) 2015-2018 INSIDE Secure B.V. All Rights Reserved.
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

#ifndef CS_HWPAL_EXT_H_
#define CS_HWPAL_EXT_H_

#include <Kit/DriverFramework/incl/cs_eip164.h>
#include <Kit/DriverFramework/incl/cs_eip163.h>
#include <Kit/DriverFramework/incl/cs_eip66.h>

#include <Integration/DriverFramework/src/device_internal.h>

/*----------------------------------------------------------------------------
 * Definitions and macros
 */

/* For obtaining the IRQ number */
#ifdef DRIVER_INTERRUPTS
#define HWPAL_INTERRUPTS
#endif


/* Disable PCI Configuration Space support support */
#define HWPAL_REMOVE_DEVICE_PCICONFIGSPACE

/* Device name in the Device Tree Structure */
#define HWPAL_PLATFORM_DEVICE_NAME  "safexcel-ip-164"

/* Index of the IRQ in the "interrupts" property of the Open Firmware device */
/* tree entry. 0 is the first IRQ listed, 1 is the second IRQ listed, etc. */
#define HWPAL_PLATFORM_IRQ_IDX 0

#define HWPAL_REMAP_ADDRESSES   ;

#define HWPAL_PLATFORM_SPECIFIC_DEVICES
#define HWPAL_PLATFORM_SPECIFIC_DEVICES_COUNT   0



/* Optional devices in HWPAL_DEVICES, see below */
/*#define HWPAL_OPTIONAL_DEVICES_COUNT (HWPAL_PLATFORM_SPECIFIC_DEVICES_COUNT + 2)*/
#define HWPAL_OPTIONAL_DEVICES_COUNT (HWPAL_PLATFORM_SPECIFIC_DEVICES_COUNT + 0) /*doronf*/

#if HWPAL_OPTIONAL_DEVICES_COUNT != DRIVER_OPTIONAL_DEVICE_COUNT
#error "Optional devices configuration mismatch"
#endif


/* Definition of static resources (devices), */
/* refer to the data sheet of device for the correct values */


/* Note: EIP-164 register memory map size is 0xp0000 and end address 0xp0000-1 */
/*       where p = (MaxSA_Count >> 7), */
/*       EIP-163 register memory map size is 0xp0000 and end address 0xp0000-1 */
/*       where p = (MaxTCAMRules_Count >> 9), */
/*       see EIP-164 and EIP-163 Programmer Manuals for details */

/* Flags: */
/*   bit0 = Trace Device_Read32(), requires HWPAL_TRACE_DEVICE_READ */
/*   bit1 = Trace Device_Write32(), requires HWPAL_TRACE_DEVICE_WRITE */
/*   bit2 = Swap word endianness (requires HWPAL_DEVICE_ENABLE_SWAP) */

/* Represents 8 MACSec units per device for total of 128 devices */

#define HWPAL_DEVICES                                                                                                        \
/* Fields per unit:      unit name      ,  HW unit ID                                  ,  unit offset end     ,  flags */      \
/* Device #0 */                                                                                                              \
    /* EIP-164 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_0" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_0" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_1" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_1" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_2" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_2" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_1_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_3" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_3" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_1_E ,  EIP163_REG_IF_END   ,  3        ), \
/* Device #1 */                                                                                                              \
    /* EIP-164 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_4" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_4" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_5" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_5" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_6" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_6" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_1_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_7" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_7" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_1_E ,  EIP163_REG_IF_END   ,  3        ), \
/* Device #2 */                                                                                                              \
    /* EIP-164 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_8" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_8" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_9" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_9" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_10" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_10" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_1_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_11" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_11" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_1_E ,  EIP163_REG_IF_END   ,  3        ),\
/* Device #3 */                                                                                                              \
    /* EIP-164 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_12" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_12" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_13" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_13" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_14" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_14" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_1_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_15" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_15" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_1_E ,  EIP163_REG_IF_END   ,  3        ),\
/* Device #4 */                                                                                                              \
    /* EIP-164 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_16" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_16" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_17" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_17" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_18" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_18" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_1_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_19" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_19" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_1_E ,  EIP163_REG_IF_END   ,  3        ),\
/* Device #5 */                                                                                                              \
    /* EIP-164 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_20" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_20" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_21" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_21" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_22" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_22" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_1_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_23" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_23" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_1_E ,  EIP163_REG_IF_END   ,  3        ),\
/* Device #6 */                                                                                                              \
    /* EIP-164 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_24" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_24" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_25" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_25" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_26" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_26" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_1_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_27" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_27" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_1_E ,  EIP163_REG_IF_END   ,  3        ),\
/* Device #7 */                                                                                                              \
    /* EIP-164 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_28" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_28" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_29" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_29" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_30" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_30" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_1_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_31" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_31" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_1_E ,  EIP163_REG_IF_END   ,  3        ),\
/* Device #8 */                                                                                                              \
    /* EIP-164 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_32" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_32" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_33" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_33" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_34" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_34" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_1_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_35" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_35" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_1_E ,  EIP163_REG_IF_END   ,  3        ),\
/* Device #9 */                                                                                                              \
    /* EIP-164 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_36" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_36" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_37" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_37" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_38" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_38" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_1_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_39" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_39" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_1_E ,  EIP163_REG_IF_END   ,  3        ),\
/* Device #10 */                                                                                                             \
    /* EIP-164 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_40" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_40" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_41" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_41" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_42" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_42" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_1_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_43" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_43" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_1_E ,  EIP163_REG_IF_END   ,  3        ),\
/* Device #11 */                                                                                                             \
    /* EIP-164 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_44" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_44" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_45" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_45" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_46" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_46" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_1_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_47" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_47" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_1_E ,  EIP163_REG_IF_END   ,  3        ),\
/* Device #12 */                                                                                                             \
    /* EIP-164 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_48" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_48" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_49" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_49" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_50" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_50" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_1_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_51" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_51" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_1_E ,  EIP163_REG_IF_END   ,  3        ),\
/* Device #13 */                                                                                                             \
    /* EIP-164 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_52" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_52" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_53" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_53" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_54" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_54" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_1_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_55" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_55" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_1_E ,  EIP163_REG_IF_END   ,  3        ),\
/* Device #14 */                                                                                                             \
    /* EIP-164 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_56" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_56" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_57" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_57" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_58" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_58" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_1_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_59" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_59" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_1_E ,  EIP163_REG_IF_END   ,  3        ),\
/* Device #15 */                                                                                                             \
    /* EIP-164 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_60" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_60" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_61" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_61" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_62" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_62" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_1_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_63" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_63" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_1_E ,  EIP163_REG_IF_END   ,  3        ),\
/* Device #16 */                                                                                                             \
    /* EIP-164 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_64" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_64" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_65" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_65" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_66" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_66" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_1_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_67" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_67" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_1_E ,  EIP163_REG_IF_END   ,  3        ),\
/* Device #17 */                                                                                                             \
    /* EIP-164 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_68" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_68" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_69" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_69" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_70" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_70" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_1_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_71" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_71" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_1_E ,  EIP163_REG_IF_END   ,  3        ),\
/* Device #18 */                                                                                                             \
    /* EIP-164 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_72" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_72" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_73" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_73" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_74" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_74" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_1_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_75" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_75" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_1_E ,  EIP163_REG_IF_END   ,  3        ),\
/* Device #19 */                                                                                                             \
    /* EIP-164 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_76" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_76" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_77" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_77" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_78" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_78" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_1_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_79" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_79" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_1_E ,  EIP163_REG_IF_END   ,  3        ),\
/* Device #20 */                                                                                                             \
    /* EIP-164 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_80" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_80" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_81" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_81" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_82" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_82" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_1_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_83" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_83" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_1_E ,  EIP163_REG_IF_END   ,  3        ),\
/* Device #21 */                                                                                                             \
    /* EIP-164 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_84" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_84" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_85" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_85" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_86" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_86" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_1_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_87" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_87" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_1_E ,  EIP163_REG_IF_END   ,  3        ),\
/* Device #22 */                                                                                                             \
    /* EIP-164 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_88" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_88" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_89" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_89" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_90" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_90" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_1_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_91" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_91" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_1_E ,  EIP163_REG_IF_END   ,  3        ),\
/* Device #23 */                                                                                                             \
    /* EIP-164 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_92" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_92" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_93" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_93" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_94" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_94" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_1_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_95" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_95" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_1_E ,  EIP163_REG_IF_END   ,  3        ),\
/* Device #23 */                                                                                                             \
    /* EIP-164 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_96" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_96" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_97" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_97" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_98" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_98" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_1_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_99" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_99" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_1_E ,  EIP163_REG_IF_END   ,  3        ),\
/* Device #24 */                                                                                                             \
    /* EIP-164 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_100" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_100" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_101" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_101" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_102" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_102" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_1_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_103" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_103" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_1_E ,  EIP163_REG_IF_END   ,  3        ),\
/* Device #25 */                                                                                                              \
    /* EIP-164 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_104" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_104" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_105" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_105" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_106" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_106" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_1_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_107" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_107" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_1_E ,  EIP163_REG_IF_END   ,  3        ),\
/* Device #26 */                                                                                                              \
    /* EIP-164 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_108" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_108" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_109" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_109" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_110" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_110" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_1_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_111" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_111" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_1_E ,  EIP163_REG_IF_END   ,  3        ),\
/* Device #27 */                                                                                                             \
    /* EIP-164 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_112" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_112" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_113" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_113" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_114" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_114" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_1_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_115" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_115" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_1_E ,  EIP163_REG_IF_END   ,  3        ),\
/* Device #28 */                                                                                                              \
    /* EIP-164 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_116" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_116" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_117" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_117" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_118" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_118" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_1_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_119" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_119" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_1_E ,  EIP163_REG_IF_END   ,  3        ),\
/* Device #29 */                                                                                                              \
    /* EIP-164 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_120" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_120" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_121" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_121" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_122" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_122" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_1_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_123" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_123" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_1_E ,  EIP163_REG_IF_END   ,  3        ),\
/* Device #30 */                                                                                                              \
    /* EIP-164 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_124" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_124" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_125" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_125" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_126" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_126" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_1_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_127" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_127" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_1_E ,  EIP163_REG_IF_END   ,  3        ),\
/* Device #31 */                                                                                                             \
    /* EIP-164 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_128" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_128" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_129" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_129" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_130" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_130" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_1_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_131" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_131" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_1_E ,  EIP163_REG_IF_END   ,  3        ),\
/* Device #32 */                                                                                                              \
    /* EIP-164 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_132" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_132" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_133" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_133" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_134" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_134" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_1_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_135" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_135" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_1_E ,  EIP163_REG_IF_END   ,  3        ),\
/* Device #32 */                                                                                                             \
    /* EIP-164 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_136" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_136" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_137" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_137" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_138" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_138" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_1_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_139" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_139" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_1_E ,  EIP163_REG_IF_END   ,  3        ),\
/* Device #33 */                                                                                                             \
    /* EIP-164 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_140" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_140" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_141" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_141" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_142" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_142" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_1_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_143" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_143" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_1_E ,  EIP163_REG_IF_END   ,  3        ),\
/* Device #34 */                                                                                                             \
    /* EIP-164 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_144" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_144" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_145" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_145" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_146" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_146" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_1_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_147" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_147" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_1_E ,  EIP163_REG_IF_END   ,  3        ),\
/* Device #35 */                                                                                                             \
    /* EIP-164 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_148" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_148" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_149" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_149" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_150" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_150" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_1_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_151" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_151" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_1_E ,  EIP163_REG_IF_END   ,  3        ),\
/* Device #36 */                                                                                                              \
    /* EIP-164 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_152" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_152" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_153" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_153" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_154" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_154" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_1_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_155" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_155" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_1_E ,  EIP163_REG_IF_END   ,  3        ),\
/* Device #37 */                                                                                                             \
    /* EIP-164 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_156" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_156" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_157" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_157" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_158" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_158" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_1_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_159" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_159" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_1_E ,  EIP163_REG_IF_END   ,  3        ),\
/* Device #38 */                                                                                                             \
    /* EIP-164 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_160" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_160" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_161" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_161" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_162" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_162" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_1_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_163" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_163" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_1_E ,  EIP163_REG_IF_END   ,  3        ),\
/* Device #39 */                                                                                                             \
    /* EIP-164 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_164" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_164" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_165" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_165" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_166" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_166" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_1_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_167" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_167" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_1_E ,  EIP163_REG_IF_END   ,  3        ),\
/* Device #40 */                                                                                                             \
    /* EIP-164 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_168" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_168" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_169" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_169" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_170" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_170" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_1_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_171" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_171" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_1_E ,  EIP163_REG_IF_END   ,  3        ),\
/* Device #41 */                                                                                                             \
    /* EIP-164 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_172" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_172" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_173" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_173" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_174" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_174" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_1_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_175" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_175" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_1_E ,  EIP163_REG_IF_END   ,  3        ),\
/* Device #42 */                                                                                                              \
    /* EIP-164 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_176" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_176" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_177" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_177" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_178" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_178" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_1_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_179" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_179" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_1_E ,  EIP163_REG_IF_END   ,  3        ),\
/* Device #43 */                                                                                                             \
    /* EIP-164 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_180" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_180" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_181" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_181" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_182" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_182" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_1_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_183" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_183" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_1_E ,  EIP163_REG_IF_END   ,  3        ),\
/* Device #44 */                                                                                                             \
    /* EIP-164 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_184" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_184" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_185" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_185" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_186" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_186" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_1_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_187" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_187" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_1_E ,  EIP163_REG_IF_END   ,  3        ),\
/* Device #45 */                                                                                                             \
    /* EIP-164 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_188" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_188" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_189" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_189" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_190" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_190" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_1_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_191" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_191" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_1_E ,  EIP163_REG_IF_END   ,  3        ),\
/* Device #46 */                                                                                                            \
    /* EIP-164 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_192" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_192" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_193" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_193" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_194" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_194" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_1_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_195" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_195" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_1_E ,  EIP163_REG_IF_END   ,  3        ),\
/* Device #47 */                                                                                                              \
    /* EIP-164 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_196" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_196" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_197" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_197" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_198" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_198" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_1_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_199" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_199" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_1_E ,  EIP163_REG_IF_END   ,  3        ),\
/* Device #48 */                                                                                                             \
    /* EIP-164 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_200" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_200" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_201" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_201" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_202" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_202" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_1_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_203" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_203" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_1_E ,  EIP163_REG_IF_END   ,  3        ),\
/* Device #49 */                                                                                                             \
    /* EIP-164 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_204" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_204" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_205" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_205" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_206" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_206" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_1_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_207" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_207" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_1_E ,  EIP163_REG_IF_END   ,  3        ),\
/* Device #50 */                                                                                                             \
    /* EIP-164 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_208" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_208" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_209" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_209" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_210" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_210" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_1_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_211" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_211" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_1_E ,  EIP163_REG_IF_END   ,  3        ),\
/* Device #51 */                                                                                                             \
    /* EIP-164 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_212" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_212" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_213" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_213" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_214" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_214" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_1_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_215" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_215" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_1_E ,  EIP163_REG_IF_END   ,  3        ),\
/* Device #52 */                                                                                                            \
    /* EIP-164 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_216" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_216" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_217" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_217" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_218" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_218" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_1_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_219" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_219" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_1_E ,  EIP163_REG_IF_END   ,  3        ),\
/* Device #53 */                                                                                                           \
    /* EIP-164 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_220" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_220" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_221" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_221" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_222" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_222" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_1_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_223" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_223" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_1_E ,  EIP163_REG_IF_END   ,  3        ),\
/* Device #54 */                                                                                                                \
    /* EIP-164 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_224" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_224" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_225" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_225" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_226" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_226" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_1_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_227" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_227" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_1_E ,  EIP163_REG_IF_END   ,  3        ),\
/* Device #55 */                                                                                                             \
    /* EIP-164 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_228" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_228" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_229" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_229" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_230" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_230" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_1_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_231" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_231" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_1_E ,  EIP163_REG_IF_END   ,  3        ),\
/* Device #56 */                                                                                                              \
    /* EIP-164 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_232" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_232" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_233" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_233" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_234" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_234" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_1_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_235" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_235" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_1_E ,  EIP163_REG_IF_END   ,  3        ),\
/* Device #57 */                                                                                                             \
    /* EIP-164 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_236" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_236" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_237" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_237" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_238" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_238" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_1_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_239" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_239" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_1_E ,  EIP163_REG_IF_END   ,  3        ),\
/* Device #58 */                                                                                                             \
    /* EIP-164 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_240" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_240" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_241" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_241" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_242" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_242" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_1_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_243" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_243" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_1_E ,  EIP163_REG_IF_END   ,  3        ),\
/* Device #59 */                                                                                                             \
    /* EIP-164 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_244" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_244" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_245" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_245" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_246" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_246" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_1_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_247" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_247" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_1_E ,  EIP163_REG_IF_END   ,  3        ),\
/* Device #60 */                                                                                                              \
    /* EIP-164 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_248" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_248" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_249" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_249" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_250" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_250" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_1_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_251" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_251" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_1_E ,  EIP163_REG_IF_END   ,  3        ),\
/* Device #61 */                                                                                                             \
    /* EIP-164 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_252" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_252" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_253" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_253" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_254" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_254" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_1_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_255" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_255" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_1_E ,  EIP163_REG_IF_END   ,  3        ),\
/* Device #62 */                                                                                                              \
    /* EIP-164 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_256" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_256" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_257" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_257" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_258" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_258" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_1_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_259" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_259" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_1_E ,  EIP163_REG_IF_END   ,  3        ),\
/* Device #63 */                                                                                                              \
    /* EIP-164 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_260" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_260" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_261" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_261" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_262" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_262" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_1_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_263" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_263" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_1_E ,  EIP163_REG_IF_END   ,  3        ),\
/* Device #64 */                                                                                                             \
    /* EIP-164 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_264" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_264" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_265" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_265" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_266" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_266" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_1_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_267" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_267" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_1_E ,  EIP163_REG_IF_END   ,  3        ),\
/* Device #65 */                                                                                                              \
    /* EIP-164 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_268" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_268" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_269" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_269" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_270" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_270" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_1_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_271" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_271" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_1_E ,  EIP163_REG_IF_END   ,  3        ),\
/* Device #66 */                                                                                                             \
    /* EIP-164 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_272" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_272" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_273" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_273" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_274" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_274" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_1_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_275" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_275" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_1_E ,  EIP163_REG_IF_END   ,  3        ),\
/* Device #67 */                                                                                                              \
    /* EIP-164 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_276" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_276" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_277" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_277" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_278" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_278" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_1_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_279" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_279" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_1_E ,  EIP163_REG_IF_END   ,  3        ),\
/* Device #68 */                                                                                                              \
    /* EIP-164 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_280" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_280" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_281" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_281" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_282" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_282" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_1_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_283" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_283" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_1_E ,  EIP163_REG_IF_END   ,  3        ),\
/* Device #69 */                                                                                                             \
    /* EIP-164 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_284" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_284" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_285" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_285" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_286" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_286" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_1_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_287" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_287" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_1_E ,  EIP163_REG_IF_END   ,  3        ),\
/* Device #70 */                                                                                                              \
    /* EIP-164 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_288" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_288" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_289" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_289" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_290" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_290" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_1_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_291" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_291" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_1_E ,  EIP163_REG_IF_END   ,  3        ),\
/* Device #71 */                                                                                                             \
    /* EIP-164 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_292" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_292" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_293" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_293" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_294" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_294" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_1_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_295" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_295" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_1_E ,  EIP163_REG_IF_END   ,  3        ),\
/* Device #71 */                                                                                                             \
    /* EIP-164 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_296" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_296" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_297" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_297" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_298" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_298" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_1_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_299" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_299" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_1_E ,  EIP163_REG_IF_END   ,  3        ),\
/* Device #72 */                                                                                                              \
    /* EIP-164 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_300" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_300" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_301" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_301" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_302" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_302" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_1_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_303" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_303" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_1_E ,  EIP163_REG_IF_END   ,  3        ),\
/* Device #73 */                                                                                                             \
    /* EIP-164 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_304" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_304" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_305" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_305" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_306" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_306" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_1_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_307" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_307" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_1_E ,  EIP163_REG_IF_END   ,  3        ),\
/* Device #74 */                                                                                                              \
    /* EIP-164 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_308" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_308" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_309" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_309" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_310" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_310" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_1_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_311" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_311" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_1_E ,  EIP163_REG_IF_END   ,  3        ),\
/* Device #75 */                                                                                                             \
    /* EIP-164 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_312" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_312" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_313" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_313" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_314" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_314" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_1_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_315" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_315" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_1_E ,  EIP163_REG_IF_END   ,  3        ),\
/* Device #76 */                                                                                                             \
    /* EIP-164 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_316" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_316" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_317" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_317" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_318" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_318" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_1_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_319" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_319" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_1_E ,  EIP163_REG_IF_END   ,  3        ),\
/* Device #77 */                                                                                                             \
    /* EIP-164 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_320" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_320" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_321" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_321" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_322" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_322" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_1_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_323" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_323" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_1_E ,  EIP163_REG_IF_END   ,  3        ),\
/* Device #78 */                                                                                                             \
    /* EIP-164 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_324" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_324" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_325" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_325" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_326" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_326" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_1_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_327" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_327" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_1_E ,  EIP163_REG_IF_END   ,  3        ),\
/* Device #79 */                                                                                                              \
    /* EIP-164 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_328" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_328" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_329" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_329" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_330" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_330" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_1_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_331" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_331" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_1_E ,  EIP163_REG_IF_END   ,  3        ),\
/* Device #80 */                                                                                                             \
    /* EIP-164 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_332" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_332" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_333" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_333" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_334" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_334" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_1_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_335" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_335" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_1_E ,  EIP163_REG_IF_END   ,  3        ),\
/* Device #81 */                                                                                                              \
    /* EIP-164 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_336" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_336" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_337" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_337" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_338" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_338" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_1_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_339" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_339" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_1_E ,  EIP163_REG_IF_END   ,  3        ),\
/* Device #82 */                                                                                                             \
    /* EIP-164 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_340" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_340" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_341" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_341" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_342" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_342" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_1_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_343" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_343" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_1_E ,  EIP163_REG_IF_END   ,  3        ),\
/* Device #83 */                                                                                                             \
    /* EIP-164 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_344" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_344" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_345" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_345" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_346" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_346" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_1_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_347" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_347" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_1_E ,  EIP163_REG_IF_END   ,  3        ),\
/* Device #84 */                                                                                                             \
    /* EIP-164 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_348" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_348" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_349" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_349" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_350" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_350" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_1_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_351" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_351" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_1_E ,  EIP163_REG_IF_END   ,  3        ),\
/* Device #85 */                                                                                                             \
    /* EIP-164 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_352" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_352" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_353" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_353" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_354" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_354" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_1_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_355" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_355" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_1_E ,  EIP163_REG_IF_END   ,  3        ),\
/* Device #86 */                                                                                                             \
    /* EIP-164 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_356" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_356" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_357" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_357" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_358" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_358" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_1_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_359" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_359" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_1_E ,  EIP163_REG_IF_END   ,  3        ),\
/* Device #87 */                                                                                                             \
    /* EIP-164 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_360" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_360" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_361" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_361" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_362" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_362" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_1_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_363" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_363" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_1_E ,  EIP163_REG_IF_END   ,  3        ),\
/* Device #88 */                                                                                                            \
    /* EIP-164 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_364" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_364" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_365" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_365" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_366" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_366" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_1_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_367" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_367" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_1_E ,  EIP163_REG_IF_END   ,  3        ),\
/* Device #89 */                                                                                                             \
    /* EIP-164 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_368" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_368" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_369" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_369" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_370" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_370" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_1_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_371" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_371" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_1_E ,  EIP163_REG_IF_END   ,  3        ),\
/* Device #90 */                                                                                                             \
    /* EIP-164 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_372" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_372" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_373" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_373" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_374" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_374" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_1_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_375" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_375" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_1_E ,  EIP163_REG_IF_END   ,  3        ),\
/* Device #91 */                                                                                                             \
    /* EIP-164 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_376" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_376" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_377" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_377" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_378" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_378" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_1_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_379" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_379" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_1_E ,  EIP163_REG_IF_END   ,  3        ),\
/* Device #92 */                                                                                                              \
    /* EIP-164 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_380" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_380" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_381" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_381" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_382" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_382" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_1_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_383" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_383" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_1_E ,  EIP163_REG_IF_END   ,  3        ),\
/* Device #93 */                                                                                                              \
    /* EIP-164 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_384" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_384" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_385" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_385" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_386" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_386" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_1_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_387" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_387" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_1_E ,  EIP163_REG_IF_END   ,  3        ),\
/* Device #94 */                                                                                                             \
    /* EIP-164 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_388" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_388" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_389" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_389" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_390" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_390" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_1_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_391" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_391" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_1_E ,  EIP163_REG_IF_END   ,  3        ),\
/* Device #95 */                                                                                                             \
    /* EIP-164 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_392" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_392" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_393" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_393" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_394" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_394" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_1_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_395" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_395" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_1_E ,  EIP163_REG_IF_END   ,  3        ),\
/* Device #96 */                                                                                                             \
    /* EIP-164 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_396" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_396" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_397" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_397" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_398" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_398" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_1_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_399" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_399" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_1_E ,  EIP163_REG_IF_END   ,  3        ),\
/* Device #97 */                                                                                                             \
    /* EIP-164 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_400" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_400" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_401" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_401" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_402" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_402" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_1_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_403" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_403" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_1_E ,  EIP163_REG_IF_END   ,  3        ),\
/* Device #98 */                                                                                                             \
    /* EIP-164 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_404" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_404" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_405" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_405" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_406" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_406" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_1_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_407" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_407" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_1_E ,  EIP163_REG_IF_END   ,  3        ),\
/* Device #99 */                                                                                                             \
    /* EIP-164 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_408" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_408" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_409" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_409" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_410" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_410" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_1_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_411" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_411" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_1_E ,  EIP163_REG_IF_END   ,  3        ),\
/* Device #100 */                                                                                                            \
    /* EIP-164 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_412" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_412" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_413" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_413" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_414" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_414" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_1_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_415" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_415" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_1_E ,  EIP163_REG_IF_END   ,  3        ),\
/* Device #101 */                                                                                                            \
    /* EIP-164 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_416" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_416" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_417" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_417" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_418" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_418" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_1_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_419" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_419" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_1_E ,  EIP163_REG_IF_END   ,  3        ),\
/* Device #102 */                                                                                                            \
    /* EIP-164 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_420" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_420" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_421" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_421" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_422" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_422" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_1_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_423" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_423" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_1_E ,  EIP163_REG_IF_END   ,  3        ),\
/* Device #103 */                                                                                                            \
    /* EIP-164 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_424" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_424" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_425" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_425" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_426" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_426" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_1_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_427" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_427" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_1_E ,  EIP163_REG_IF_END   ,  3        ),\
/* Device #104 */                                                                                                            \
    /* EIP-164 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_428" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_428" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_429" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_429" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_430" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_430" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_1_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_431" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_431" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_1_E ,  EIP163_REG_IF_END   ,  3        ),\
/* Device #105 */                                                                                                            \
    /* EIP-164 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_432" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_432" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_433" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_433" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_434" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_434" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_1_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_435" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_435" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_1_E ,  EIP163_REG_IF_END   ,  3        ),\
/* Device #106 */                                                                                                            \
    /* EIP-164 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_436" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_436" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_437" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_437" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_438" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_438" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_1_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_439" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_439" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_1_E ,  EIP163_REG_IF_END   ,  3        ),\
/* Device #107 */                                                                                                            \
    /* EIP-164 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_440" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_440" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_441" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_441" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_442" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_442" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_1_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_443" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_443" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_1_E ,  EIP163_REG_IF_END   ,  3        ),\
/* Device #108 */                                                                                                            \
    /* EIP-164 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_444" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_444" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_445" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_445" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_446" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_446" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_1_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_447" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_447" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_1_E ,  EIP163_REG_IF_END   ,  3        ),\
/* Device #109 */                                                                                                            \
    /* EIP-164 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_448" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_448" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_449" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_449" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_450" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_450" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_1_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_451" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_451" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_1_E ,  EIP163_REG_IF_END   ,  3        ),\
/* Device #110 */                                                                                                            \
    /* EIP-164 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_452" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_452" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_453" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_453" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_454" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_454" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_1_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_455" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_455" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_1_E ,  EIP163_REG_IF_END   ,  3        ),\
/* Device #111 */                                                                                                                             \
    /* EIP-164 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_456" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_456" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_457" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_457" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_458" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_458" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_1_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_459" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_459" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_1_E ,  EIP163_REG_IF_END   ,  3        ),\
/* Device #112 */                                                                                                            \
    /* EIP-164 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_460" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_460" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_461" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_461" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_462" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_462" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_1_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_463" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_463" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_1_E ,  EIP163_REG_IF_END   ,  3        ),\
/* Device #113 */                                                                                                            \
    /* EIP-164 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_464" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_464" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_465" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_465" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_466" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_466" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_1_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_467" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_467" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_1_E ,  EIP163_REG_IF_END   ,  3        ),\
/* Device #114 */                                                                                                            \
    /* EIP-164 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_468" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_468" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_469" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_469" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_470" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_470" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_1_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_471" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_471" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_1_E ,  EIP163_REG_IF_END   ,  3        ),\
/* Device #115 */                                                                                                            \
    /* EIP-164 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_472" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_472" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_473" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_473" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_474" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_474" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_1_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_475" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_475" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_1_E ,  EIP163_REG_IF_END   ,  3        ),\
/* Device #116 */                                                                                                            \
    /* EIP-164 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_476" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_476" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_477" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_477" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_478" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_478" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_1_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_479" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_479" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_1_E ,  EIP163_REG_IF_END   ,  3        ),\
/* Device #117 */                                                                                                            \
    /* EIP-164 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_480" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_480" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_481" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_481" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_482" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_482" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_1_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_483" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_483" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_1_E ,  EIP163_REG_IF_END   ,  3        ),\
/* Device #118 */                                                                                                            \
    /* EIP-164 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_484" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_484" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_485" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_485" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_486" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_486" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_1_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_487" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_487" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_1_E ,  EIP163_REG_IF_END   ,  3        ),\
/* Device #119 */                                                                                                                             \
    /* EIP-164 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_488" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_488" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_489" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_489" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_490" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_490" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_1_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_491" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_491" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_1_E ,  EIP163_REG_IF_END   ,  3        ),\
/* Device #120 */                                                                                                            \
    /* EIP-164 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_492" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_492" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_493" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_493" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_494" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_494" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_1_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_495" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_495" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_1_E ,  EIP163_REG_IF_END   ,  3        ),\
/* Device #121 */                                                                                                                             \
    /* EIP-164 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_496" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_496" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_497" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_497" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_498" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_498" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_1_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_499" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_499" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_1_E ,  EIP163_REG_IF_END   ,  3        ),\
/* Device #122 */                                                                                                            \
    /* EIP-164 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_500" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_500" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_501" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_501" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_502" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_502" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_1_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_503" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_503" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_1_E ,  EIP163_REG_IF_END   ,  3        ),\
/* Device #123 */                                                                                                            \
    /* EIP-164 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_504" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_504" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_505" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_505" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_506" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_506" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_1_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_507" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_507" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_1_E ,  EIP163_REG_IF_END   ,  3        ),\
/* Device #124 */                                                                                                            \
    /* EIP-164 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_508" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_508" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_509" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_509" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_510" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_510" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_1_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_511" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_511" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_1_E ,  EIP163_REG_IF_END   ,  3        ),\
/* Device #125 */                                                                                                           \
    /* EIP-164 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_512" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_512" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_513" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_513" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_514" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_514" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_1_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_515" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_515" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_1_E ,  EIP163_REG_IF_END   ,  3        ),\
/* Device #126 */                                                                                                            \
    /* EIP-164 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_516" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_516" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_517" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_517" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_518" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_518" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_1_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_519" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_519" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_1_E ,  EIP163_REG_IF_END   ,  3        ),\
/* Device #127 */                                                                                                             \
    /* EIP-164 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_520" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 0  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_520" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_521" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_0_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 0 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_521" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_0_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_522" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Egress D-path 1  */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_522" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_1_E ,  EIP163_REG_IF_END   ,  3        ), \
    /* EIP-164 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP164_523" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_1_E ,  EIP164_REG_IF_END   ,  3        ), \
    /* EIP-163 Ingress D-path 1 */                                                                                           \
    HWPAL_DEVICE_ADD(    "DEV_EIP163_523" ,  PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_1_E ,  EIP163_REG_IF_END   ,  3        )

#endif /* CS_HWPAL_EXT_H_ */


/* end of file cs_hwpal_ext.h */
