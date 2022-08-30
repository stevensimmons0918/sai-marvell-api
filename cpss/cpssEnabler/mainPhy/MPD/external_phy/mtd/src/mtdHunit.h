/*******************************************************************************
Copyright (C) 2014 - 2021, Marvell International Ltd. and its affiliates
If you received this File from Marvell and you have entered into a commercial
license agreement (a "Commercial License") with Marvell, the File is licensed
to you under the terms of the applicable Commercial License.
*******************************************************************************/

/********************************************************************
This file contains functions prototypes and global defines/data for
higher-level functions of the Host Interface Unit (H Unit) of 
the Marvell 88X32X0, 88X33X0, 88X35X0, 88E20X0 and 88E21X0 ethernet PHYs.
********************************************************************/
#ifndef MTDHUNIT_H
#define MTDHUNIT_H
#if C_LINKAGE
#if defined __cplusplus 
    extern "C" { 
#endif 
#endif

/******************************************************************************
 Mac Interface functions
******************************************************************************/

/* for macType X32X0/X33X0/E20X0,  please read the options carefully not all are available on */
/* all devices. NOT E21X0 devices. For E21X0 please see below. */
#define MTD_MAC_TYPE_RXAUI_SGMII_AN_EN  (0x0) /* X32X0/X33x0, but not E20x0 */
#define MTD_MAC_TYPE_RXAUI_SGMII_AN_DIS (0x1) /* X32x0/X3340/X3320, but not X3310/E2010 */
#define MTD_MAC_TYPE_XAUI_RATE_ADAPT    (0x1) /* X3310,E2010 only */
#define MTD_MAC_TYPE_RXAUI_RATE_ADAPT   (0x2)
#define MTD_MAC_TYPE_XAUI               (0x3) /* X3310,E2010 only */  
#define MTD_MAC_TYPE_XFI_SGMII_AN_EN    (0x4) /* XFI at 10G, X33x0/E20x0 also use 5GBASE-R/2500BASE-X */
#define MTD_MAC_TYPE_XFI_SGMII_AN_DIS   (0x5) /* XFI at 10G, X33x0/E20x0 also use 5GBASE-R/2500BASE-X */
#define MTD_MAC_TYPE_XFI_RATE_ADAPT     (0x6)
#define MTD_MAC_TYPE_USXGMII            (0x7) /* X33X0 only */
#define MTD_MAC_LEAVE_UNCHANGED         (0x8) /* use this option to not touch these bits */

/* for macType E21X0 devices */
/* NOTE: DXGMII and QXGMII have limitation changing into/out of via software. See datasheet for details. */
#define MTD_MAC_TYPE_SXGMII             (0x0) /* single port USXGMII */
#define MTD_MAC_TYPE_DXGMII             (0x1) /* dual port USXGMII THIS CANNOT BE CHANGED THROUGH API - STRAP ONLY */
#define MTD_MAC_TYPE_QXGMII             (0x2) /* quad port USXGMII THIS CANNOT BE CHANGED THROUGH API - STRAP ONLY */
#define MTD_MAC_TYPE_5BR_SGMII_AN_EN    (0x4) /* 5GBASE-R/2500BASE-X/SGMII AN En for 1G and below */
#define MTD_MAC_TYPE_5BR_SGMII_AN_DIS   (0x5) /* 5GBASE-R/2500BASE-X/SGMII AN Dis for 1G and below */
#define MTD_MAC_TYPE_10GR_RATE_ADAP     (0x6) /* 10GR/5GR/2.5GX Mac Interface, rate matching to same/lower speed copper */
/* MTD_MAC_LEAVE_UNCHANGED used to leave as-is, check datasheet for allowed override options, some changes are limited */

/* for macType X35X0/E25X0 devices */
/*      MTD_MAC_TYPE_SXGMII             (0x0)    Single port USXGMII, same as E21X0 */
#define MTD_MAC_TYPE_10G_DXGMII         (0x1) /* 10GBASE-R dual ports USXGMII, two copper ports 5G max each */
#define MTD_MAC_TYPE_10G_QXGMII         (0x2) /* 10GBASE-R quad ports USXGMII, four copper ports 2.5G max each */
#define MTD_MAC_TYPE_20G_DXGMII         (0x9) /* 20GBASE-R dual ports USXGMII, two copper ports 10G max each */
#define MTD_MAC_TYPE_20G_QXGMII         (0xA) /* 20GBASE-R quad ports USXGMII, four copper ports 5G max each */

/* For OXGMII, 1st PHY Port 2 need to be connected to 2nd PHY QXGMII Port 0 */
#define MTD_MAC_TYPE_OXGMII             (0x3) /* Octal ports USXGMII, 20GBASE-R only, four copper ports 2.5G max each */
#define MTD_MAC_TYPE_OXGMII_PARTNER     (0xB) /* Link partner mode for OXGMII mode, four copper ports 2.5G max each */

#define MTD_MAC_TYPE_10GR_SGMII_AN_EN   (0x4) /* 10GBASE-R/5GBASE-R/2.5GBASE-X, or SGMII AN enabled for 1G and below */
#define MTD_MAC_TYPE_10GR_1000BX_AP_AN  (0x5) /* 10GBASE-KR/5GBASE-KR/2.5GBASE-KX, or 1000BASE-KX (AP AN enabled) */
                                              /* 100M/10M are not supported */
#define MTD_MAC_TYPE_10GR_RM_AMD_OFF    (0x6) /* Rate matching with Auto-Media Detect Off */
#define MTD_MAC_TYPE_10GR_RM_AMD_ON     (0xE) /* Rate matching with Auto-Media Detect On */
                                              /* X35X0 only supports 10GBASE-R rate matching to 5GBR/2.5GBX, not to 1G/100M/10M */                                              
/* MTD_MAC_LEAVE_UNCHANGED used to leave as-is */

/* for macIfSnoopSel */
#define MTD_MAC_SNOOP_FROM_NETWORK      (0x2)
#define MTD_MAC_SNOOP_FROM_HOST         (0x3)
#define MTD_MAC_SNOOP_OFF               (0x0)
#define MTD_MAC_SNOOP_LEAVE_UNCHANGED   (0x4) /* use this option to not touch these bits */

/* for macLinkDownSpeed */
#define MTD_MAC_SPEED_10_MBPS            MTD_CU_SPEED_10_MBPS 
#define MTD_MAC_SPEED_100_MBPS           MTD_CU_SPEED_100_MBPS
#define MTD_MAC_SPEED_1000_MBPS          MTD_CU_SPEED_1000_MBPS
#define MTD_MAC_SPEED_10_GBPS            MTD_CU_SPEED_10_GBPS
#define MTD_MAC_SPEED_LEAVE_UNCHANGED    (0x4)
#define MTD_MAC_SPEED_NOT_APPLICABLE     (0X4) /* for X35X0/E25X0 MP-USXGMII modes */

/* For macMaxIfSpeed, not available for X32X0 */
/* On X3540/E2540 this cannot be set, for these devices it is set according */
/* to macType ( only 10G or 20G is chosen according to chosen macType above) */
#define MTD_MAX_MAC_SPEED_10G             (0x0)
#define MTD_MAX_MAC_SPEED_5G              (0x2) /* Not applicable on X3540 */
#define MTD_MAX_MAC_SPEED_20G             (0x2) /* Only applicable on X3540 */
#define MTD_MAX_MAC_SPEED_2P5G            (0x3) /* Not applicable on X3540 */
#define MTD_MAX_MAC_SPEED_LEAVE_UNCHANGED (0x4)
#define MTD_MAX_MAC_SPEED_NOT_APPLICABLE  (0x4) /* for X32X0 and X3540/E2540 devices pass this on set */

/******************************************************************************
 MTD_STATUS mtdSetMacInterfaceControl
 (
     IN MTD_DEV_PTR devPtr,
     IN MTD_U16 port,
     IN MTD_U16 macType,
     IN MTD_BOOL macIfPowerDown,
     IN MTD_U16 macIfSnoopSel,
     IN MTD_U16 macIfActiveLaneSelect,
     IN MTD_U16 macLinkDownSpeed,
     IN MTD_U16 macMaxIfSpeed,
     IN MTD_BOOL doSwReset,
     IN MTD_BOOL rerunSerdesInitialization
 );

 Inputs:
    devPtr - pointer to MTD_DEV initialized by mtdLoadDriver() call
    port - port number, 0-31
    macType - the type of MAC interface being used (the hardware interface).
        For X32X0/X33X0/E20X0:
        One of the following (not all valid for all devices):
        MTD_MAC_TYPE_RXAUI_SGMII_AN_EN - selects RXAUI with SGMII AN enabled
        MTD_MAC_TYPE_RXAUI_SGMII_AN_DIS - selects RXAUI with SGMII AN disabled (not valid on X3310)
        MTD_MAC_TYPE_XAUI_RATE_ADAPT - selects XAUI with rate matching (only valid on X3310)
        MTD_MAC_TYPE_RXAUI_RATE_ADAPT  - selects RXAUI with rate matching
        MTD_MAC_TYPE_XAUI - selects XAUI (only valid on X3310/E2010)
        MTD_MAC_TYPE_XFI_SGMII_AN_EN - selects XFI with SGMII AN enabled
        MTD_MAC_TYPE_XFI_SGMII_AN_DIS - selects XFI with SGMII AN disabled
        MTD_MAC_TYPE_XFI_RATE_ADAPT  - selects XFI with rate matching
        MTD_MAC_TYPE_USXGMII - selects USXGMII (only valid on X33X0 devices)
        MTD_MAC_LEAVE_UNCHANGED - option to leave this parameter unchanged/as it is  

        For E21X0 and X35X0/E25X0 devices mactype, please refer to mtdSetMacInterfaceCopperOnlyPhy().

    macIfPowerDown - MTD_TRUE if the host interface is always to be powered up
                     MTD_FALSE if the host interface can be powered down under 
                     certain circumstances (see datasheet)
        For E21X0 and X35X0 devices, MTD_FALSE not allowed for all MP-USXGMII macTypes

    macIfSnoopSel - If snooping is requested on the other lane, selects the source
        MTD_MAC_SNOOP_FROM_NETWORK - source of snooped data is to come from the network
        MTD_MAC_SNOOP_FROM_HOST - source of snooped data is to come from the host
        MTD_MAC_SNOOP_OFF - snooping is to be turned off
        MTD_MAC_SNOOP_LEAVE_UNCHANGED - option to leave this parameter unchanged/as it is

        Ignored for copper-only devices

    macIfActiveLaneSelect - For redundant host mode, this selects the active lane. 0 or 1
        only. 0 selects 0 as the active lane and 1 as the standby. 1 selects the other way.

        Ignored for copper-only devices
        
    macLinkDownSpeed - The speed the mac interface should run when the media side is
        link down. One of the following:
            MTD_MAC_SPEED_10_MBPS
            MTD_MAC_SPEED_100_MBPS
            MTD_MAC_SPEED_1000_MBPS
            MTD_MAC_SPEED_10_GBPS  
            MTD_MAC_SPEED_LEAVE_UNCHANGED
            MTD_MAC_SPEED_NOT_APPLICABLE (for X35X0/E25X0 devices pass this on set)

        For E21X0 device, this parameter is only valid with macType 
        MTD_MAC_TYPE_5BR_SGMII_AN_EN and MTD_MAC_TYPE_5BR_SGMII_AN_DIS to set
        link down speed to 1000 Mbps and below. Other macTypes must use
        MTD_MAC_SPEED_10_GBPS or MTD_MAC_SPEED_LEAVE_UNCHANGED.
 
        On E21X0 device to set link down speed to 5G or 2.5G, please set 
        this speed to MTD_MAC_SPEED_10_GBPS and uses macMaxIfSpeed to 
        limit the speed to 5G or 2.5G.

        On X3540/E2540 devices link down speed follows the last link up speed and the power-on default
        is MTD_MAC_SPEED_10_GBPS.

    macMaxIfSpeed - Can be used to limit the Mac interface speed. NOT settable on X32X0 or X3540/E2540.
        On the X3540/E2540 device, it is automatically set to 10G unless MACType is
        MTD_MAC_TYPE_20G_DXGMII, MTD_MAC_TYPE_20G_QXGMII or MTD_MAC_TYPE_OXGMII, then it
        is set to 20G.
   
        For other devices:
        MTD_MAX_MAC_SPEED_10G 
        MTD_MAX_MAC_SPEED_5G 
        MTD_MAX_MAC_SPEED_2P5G (on E21X0 device not allowed for macType SXGMII)
        MTD_MAX_MAC_SPEED_NOT_APPLICABLE (for X32X0 and X3540/E2540 devices pass this on set)
        MTD_MAX_MAC_SPEED_LEAVE_UNCHANGED

    doSwReset - MTD_TRUE if a software reset (31.F001.15) should be done after these changes
        have been made, or MTD_FALSE otherwise. See note below.

        Ignored for E21X0 devices. E21X0 devices will always do a software reset.
        
    rerunSerdesInitialization - MTD_TRUE if any parameter that is likely to change the speed
        of the serdes interface was performed like macLinkDownSpeed or macType will attempt
        to reset the H unit serdes (this needs to be done AFTER the soft reset, so if doSwReset
        is passed as MTD_FALSE, host must later call 
        mtdRerunSerdesAutoInitUseAutoMode() eventually to re-init the serdes).

        Ignored for E21X0 devices. E21X0 devices will always re-initializes the serdes.
        
 Outputs:
    None

 Returns:
    MTD_OK or MTD_FAIL if a bad parameter was passed, or an IO error occurs.

 Description:
    Changes the above parameters as indicated in 31.F000/F001/F0A8/F007 and
    optionally does a software reset afterwards for those bits which require a 
    software reset to take effect.
        X32X0 - Uses 31.F000/F001
        X33X0/E20X0
              - Uses 31.F000/F001/F0A8
        X35X0/E25X0 - Uses 31.F000/F001/F007
   
    For E21X0 devices uses 31.F000/31.F007/31.F0A8/1.C04A and always does a software reset
    and re-initializes the serdes after changing these options.

 Side effects:
    None

 Notes/Warnings:
    Applicable to X32X0/X33X0/E20X0 devices:
    These bits are actually in the C unit, but pertain to the host interface
    control so the API called was placed here.

    Changes to the MAC type (31.F001.2:0) do not take effect until a software
    reset is performed on the port.

    Changes to macLinkDownSpeed (31.F001.7:6) require 2 software resets to
    take effect. This function will do 2 resets if doSwReset is MTD_TRUE
    and macLinkDownSpeed is being changed.

    IMPORTANT: the readback reads back the last written value following
    a software reset. Writes followed by reads without an intervening
    software reset will read back the old bit value for all those bits
    requiring a software reset.

    Because of this, read-modify-writes to different bitfields must have an
    intervening software reset to pick up the latest value before doing
    another read-modify-write to the register, otherwise the bitfield
    may lose the value.

    Suggest always setting doSwReset to MTD_TRUE to avoid problems of
    possibly losing changes.

    Applicable to E21X0/X35X0/E25X0 devices:
    There is a simpler API available with fewer parameters to configure 
    the E21X0 device. mtdSetMacInterfaceControl() API calls that one.
    The host may use that simpler API directly. These PHYs always does a
    software reset and re-initialize the serdes after making changes.
    See mtdSetMacInterfaceCopperOnlyPhy().

    E21X0 PHY will return an error if the macType is DXGMII/QXGMII, which
    are STRAP ONLY.

******************************************************************************/
MTD_STATUS mtdSetMacInterfaceControl
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_U16 macType,
    IN MTD_BOOL macIfPowerDown,
    IN MTD_U16 macIfSnoopSel,
    IN MTD_U16 macIfActiveLaneSelect,
    IN MTD_U16 macLinkDownSpeed,
    IN MTD_U16 macMaxIfSpeed,
    IN MTD_BOOL doSwReset,
    IN MTD_BOOL rerunSerdesInitialization
);

/******************************************************************************
 MTD_STATUS mtdGetMacInterfaceControl
 (
     IN MTD_DEV_PTR devPtr,
     IN MTD_U16 port,
     OUT MTD_U16 *macType,
     OUT MTD_BOOL *macIfPowerDown,
     OUT MTD_U16 *macIfSnoopSel,
     OUT MTD_U16 *macIfActiveLaneSelect,
     OUT MTD_U16 *macLinkDownSpeed,
     OUT MTD_U16 *macMaxIfSpeed
 );

 Inputs:
    devPtr - pointer to MTD_DEV initialized by mtdLoadDriver() call
    port - port number, 0-31
    
 Outputs:
    macType - the type of MAC interface being used (the hardware interface).
        For X32X0/X33X0/E20X0:
        One of the following (not all valid for all devices):
        MTD_MAC_TYPE_RXAUI_SGMII_AN_EN - selects RXAUI with SGMII AN enabled
        MTD_MAC_TYPE_RXAUI_SGMII_AN_DIS - selects RXAUI with SGMII AN disabled (not valid on X3310)
        MTD_MAC_TYPE_XAUI_RATE_ADAPT - selects XAUI with rate matching (only valid on X3310)
        MTD_MAC_TYPE_RXAUI_RATE_ADAPT  - selects RXAUI with rate matching
        MTD_MAC_TYPE_XAUI - selects XAUI (only valid on X3310/E2010)
        MTD_MAC_TYPE_XFI_SGMII_AN_EN - selects XFI with SGMII AN enabled
        MTD_MAC_TYPE_XFI_SGMII_AN_DIS - selects XFI with SGMII AN disabled
        MTD_MAC_TYPE_XFI_RATE_ADAPT  - selects XFI with rate matching
        MTD_MAC_TYPE_USXGMII - selects USXGMII (only valid on X33X0 devices)

        For E21X0 and X35X0/E25X0 devices mactype, please refer to mtdGetMacInterfaceCopperOnlyPhy().

    macIfPowerDown - MTD_TRUE if the host interface is always to be powered up
                     MTD_FALSE if the host interface can be powered down under 
                     certain circumstances (see datasheet)
        For E21X0 and X35X0 devices, MTD_FALSE not applicable for all MP-USXGMII macTypes

    macIfSnoopSel - Indicates if snooping is turned on and source
        MTD_MAC_SNOOP_FROM_NETWORK - source of snooped data is to come from the network        
        MTD_MAC_SNOOP_FROM_HOST - source of snooped data is to come from the host
        MTD_MAC_SNOOP_OFF - snooping is to be turned off        

        Not applicable to copper-only devices

    macIfActiveLaneSelect - For redundant host mode, this indicates the active lane. 0 or 1
        only. 0 indicates 0 as the active lane and 1 as the standby. 
        1 indicates the other way.

        Not applicable to copper-only devices
        
    macLinkDownSpeed - The speed the mac interface should run when the media side is
        link down. One of the following:
            MTD_MAC_SPEED_10_MBPS
            MTD_MAC_SPEED_100_MBPS
            MTD_MAC_SPEED_1000_MBPS
            MTD_MAC_SPEED_10_GBPS

    macMaxIfSpeed - Can be used to limit the Mac interface speed. NOT available on X32X0.
        On X3540 it is set automatically, the return value tells you what it was
        set to.
        
        For X3540 device:
        MTD_MAX_MAC_SPEED_20G   (only applicable to DXGMII/QXGMII/OXGMII)
        MTD_MAX_MAC_SPEED_10G   (all other mac types)
   
        For other devices:
        MTD_MAX_MAC_SPEED_10G 
        MTD_MAX_MAC_SPEED_5G 
        MTD_MAX_MAC_SPEED_2P5G (on E21X0 device not allowed for macType SXGMII)
        MTD_MAX_MAC_SPEED_NOT_APPLICABLE (for X32X0 and X3540/E2540 devices pass this on set)

 Returns:
    MTD_OK or MTD_FAIL if query failed (for example an IO error occurred).

 Description:
    Reads parameters as currently selected in 31.F000/F001/F0A8/F007, 1.C04A 
    based on device type and returns them.
        X32X0 - Reads 31.F000/F001
        X33X0/E20X0
              - Reads 31.F000/F001/F0A8
        E21X0 - Reads 31.F000/F0A8/F007, 1.C04A
        X35X0/E25X0 - Reads 31.F000/F001/F007

 Side effects:
    None

 Notes/Warnings:
    These bits are actually in the C unit, but pertain to the host interface
    control so the API called was placed here.

    Changes to the MAC type (31.F001.2:0) and link down speed (31.F000.7:6) 
    do not take effect until a software reset is performed on the port. Changes
    on link down speed need TWO software reset to take effect.

    This function returns what's been written to the register, but may not 
    indicate what is currently being used if a software reset was not 
    performed after the MAC type was changed).

    There is a simpler API available with fewer parameters to read the
    configuration of the E21X0 device. mtdGetMacInterfaceControl()
    API calls that one. The host may use that simpler API directly.
    See mtdGetMacInterfaceCopperOnlyPhy().

******************************************************************************/
MTD_STATUS mtdGetMacInterfaceControl
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    OUT MTD_U16 *macType,
    OUT MTD_BOOL *macIfPowerDown,
    OUT MTD_U16 *macIfSnoopSel,
    OUT MTD_U16 *macIfActiveLaneSelect,
    OUT MTD_U16 *macLinkDownSpeed,
    OUT MTD_U16 *macMaxIfSpeed
);


/******************************************************************************
 For Copper-Only PHYs - Simplified Control/Status Interface
******************************************************************************/

/******************************************************************************
 MTD_STATUS mtdSetMacInterfaceCopperOnlyPhy
 (
     IN MTD_DEV_PTR devPtr,
     IN MTD_U16 port,
     IN MTD_U16 macType,
     IN MTD_BOOL macIfPowerDown,
     IN MTD_U16 macLinkDownSpeed,
     IN MTD_U16 macMaxIfSpeed
 );

 Inputs:
    devPtr - pointer to MTD_DEV initialized by mtdLoadDriver() call
    port - port number, 0-31, MUST be the first(Master) of the grouped port (P0-1, P2-3, P0-3)
           for X35X0/E25X0 DXGMII/QXGMII/OXGMII modes
    macType - the type of MAC interface being used (the hardware interface). 
        For E21X0 devices:
        MTD_MAC_TYPE_SXGMII - single port USXGMII
        MTD_MAC_TYPE_DXGMII - NOT ALLOWED (must be selected by strap and cannot be changed through software)
        MTD_MAC_TYPE_QXGMII - NOT ALLOWED (must be selected by strap and cannot be changed through software)
        MTD_MAC_TYPE_5BR_SGMII_AN_EN - 5GBASE-R/2500BASE-X/SGMII AN En for 1G and below
        MTD_MAC_TYPE_5BR_SGMII_AN_DIS - 5GBASE-R/2500BASE-X/SGMII AN Dis for 1G and below
        MTD_MAC_TYPE_10GR_RATE_ADAP - 10GR/5GR/2.5GX Mac Interface, rate matching to same/lower speed copper
        MTD_MAC_LEAVE_UNCHANGED        - used to leave as-is

        For X35X0/E25X0 devices:
        MTD_MAC_TYPE_SXGMII            - Single port USXGMII
        MTD_MAC_TYPE_10G_DXGMII        - 10GBASE-R dual ports USXGMII, two copper ports of 5G max each
        MTD_MAC_TYPE_10G_QXGMII        - 10GBASE-R quad ports USXGMII, four copper ports of 2.5G max each
        MTD_MAC_TYPE_20G_DXGMII        - 20GBASE-R dual ports USXGMII, two copper ports of 10G max each
        MTD_MAC_TYPE_20G_QXGMII        - 20GBASE-R quad ports USXGMII, four copper ports of 5G max each
        
        - OXGMII 1st PHY Port 2 need to be connected to 2nd phy QXGMII Port 1
        - The 2nd X35X0/E25X0 PHY should be configured to MTD_MAC_TYPE_OXGMII_PARTNER, which runs 10G QXGMII
        MTD_MAC_TYPE_OXGMII            - Octal ports USXGMII, four copper ports of 2.5G max each
        MTD_MAC_TYPE_OXGMII_PARTNER    - Link partner mode for OXGMII mode, four copper ports of 2.5G max each
        
        MTD_MAC_TYPE_10GR_SGMII_AN_EN  - 10GBASE-R/5GBASE-R/2.5GBASE-X, or SGMII AN enabled for 1G and below
        MTD_MAC_TYPE_10GR_1000BX_AP_AN - 10GBASE-KR/5GBASE-KR/2.5GBASE-KX, or 1000BASE-KX (CL73 AN enabled)
                                       - 100M/10M are not supported
        MTD_MAC_TYPE_10GR_RM_AMD_OFF   - Rate matching with Auto-Media Detect Off
        MTD_MAC_TYPE_10GR_RM_AMD_ON    - Rate matching with Auto-Media Detect On
                                         X35X0 only supports 10GBASE-R rate matching to 5GBR/2.5GBX, not to 1G/100M/10M
        MTD_MAC_LEAVE_UNCHANGED        - used to leave as-is

        For X32X0/X33X0/E20X0 devices mactype, please refer to mtdSetMacInterfaceControl().

    macIfPowerDown - MTD_TRUE if the host interface is always to be powered up
                     MTD_FALSE if the host interface can be powered down under 
                     certain circumstances (see datasheet)  
        For E21X0 and X35X0/E25X0 devices, must be MTD_TRUE for all MP-USXGMII macTypes

    macLinkDownSpeed - The speed the mac interface should run when the media side is
                       link down. One of the following: 
        MTD_MAC_SPEED_10_MBPS
        MTD_MAC_SPEED_100_MBPS
        MTD_MAC_SPEED_1000_MBPS
        MTD_MAC_SPEED_10_GBPS
        MTD_MAC_SPEED_LEAVE_UNCHANGED
        MTD_MAC_SPEED_NOT_APPLICABLE (for X35X0/E25X0 devices pass this on set)

        For E21X0 device, always pass MTD_MAC_SPEED_10_GBPS or MTD_MAC_SPEED_LEAVE_UNCHANGED
        unless you want to set the link down speed to 1000 Mbps or below. However,
        the link down speed may only be set to 1000 Mbps and below on Mac Types
        MTD_MAC_TYPE_5BR_SGMII_AN_EN and MTD_MAC_TYPE_5BR_SGMII_AN_DIS.
                            
        On E21X0 device in order to set the link down speed to 5G or 2.5G, please set 
        this speed to MTD_MAC_SPEED_10_GBPS and uses macMaxIfSpeed to 
        limit the speed to 5G or 2.5G.

        On X3540/E2540 devices link down speed follows the last link up speed and the power-on default
        is MTD_MAC_SPEED_10_GBPS.

    macMaxIfSpeed - Can be used to limit the Mac interface speed. NOT settable on X3540/E2540.
        On the X3540/E2540 device, it is automatically set to 10G unless MACType is
        MTD_MAC_TYPE_20G_DXGMII, MTD_MAC_TYPE_20G_QXGMII or MTD_MAC_TYPE_OXGMII, then it
        is set to 20G.

        For other devices:
            MTD_MAX_MAC_SPEED_10G
            MTD_MAX_MAC_SPEED_5G
            MTD_MAX_MAC_SPEED_2P5G (on E21X0 device not allowed for macType SXGMII)
            MTD_MAX_MAC_SPEED_LEAVE_UNCHANGED
            MTD_MAX_MAC_SPEED_NOT_APPLICABLE (for X3540/E2540 devices pass this on set)

 Outputs:
    None

 Returns:
    MTD_OK or MTD_FAIL if a bad parameter was passed, or an IO error occurs.

 Description:
    This is a simplified version of mtdSetMacInterfaceControl(). For copper-only PHYs,
    either function may be used.

    OXGMII uses 8 ports but X3540 has only 4 ports. OXGMII configuration requires another
    X3540 or other QXGMII compatible device. When X3540 is configured to OXGMII, the Port2
    need to be connected to QXGMII primary host port of the second device which is
    configured as QXGMII mode. If the 2nd board is X3540, user MUST use this API to configure
    it to MTD_MAC_TYPE_OXGMII_PARTNER mode, which runs 10G QXGMII with extra settings.

    Uses different registers to control copper only PHYs MAC interface.
        E20X0 - uses 31.F000/F001/F0A8
        E21X0 - uses 31.F000/F0A8/F007, 1.C04A
        X35X0/E25X0 - uses 31.F000/F001/F007

    The API always does a software reset and re-initializes the serdes after
    changing these options.

 Side effects:
    None

 Notes/Warnings:
    Must pass in the first(Master) of the grouped port (P0-1, P2-3, P0-3) for
    X35X0/E25X0 DXGMII/QXGMII/OXGMII/OXGMII_PARTNER modes.

    If this is a X35X0/E25X0 device that used for OXGMII partner, user must call this API to configure
    it to MTD_MAC_TYPE_OXGMII_PARTNER mode, which runs 10G QXGMII with extra settings.

    If X35X0 device was previously configured to MTD_MAC_TYPE_OXGMII_PARTNER mode, and
    then a port is changed to a different mode, all ports in that 4-port group  will have
    their MP-USXGMII partner bits cleared.

    For X3580, user must configure first 4 ports and second four ports to fully configure
    the device.

    The link will drop when this function is called.

    See the notes for mtdSetMacInterfaceControl() for more details.

    E21X0 PHY will return an error if the macType is DXGMII/QXGMII, which
    are STRAP ONLY.

    Regarding copper speed advertisement: The maximum copper speed per port of 
    the MBASE-T interface is based on the total bandwidth of the host interface. 
    Call mtdEnableSpeeds() to disable higher speeds as needed to insure the total 
    bandwidth of the copper ports does not exceed the host interface bandwidth.
    For example, MTD_MAC_TYPE_10G_DXGMII, the maximum copper speed is 5G. See
    above for each macType what the combination of speeds is allowed.

******************************************************************************/
MTD_STATUS mtdSetMacInterfaceCopperOnlyPhy
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_U16 macType,
    IN MTD_BOOL macIfPowerDown,
    IN MTD_U16 macLinkDownSpeed,
    IN MTD_U16 macMaxIfSpeed
);


/******************************************************************************
 MTD_STATUS mtdGetMacInterfaceCopperOnlyPhy
 (
     IN MTD_DEV_PTR devPtr,
     IN MTD_U16 port,
     OUT MTD_U16 *macType,
     OUT MTD_BOOL *macIfPowerDown,
     OUT MTD_U16 *macLinkDownSpeed,
     OUT MTD_U16 *macMaxIfSpeed 
 );

 Inputs:
    devPtr - pointer to MTD_DEV initialized by mtdLoadDriver() call
    port - port number, 0-31
    
 Outputs:
    macType - the type of MAC interface being selected. 
        For E21X0 devices:
        MTD_MAC_TYPE_SXGMII - single port USXGMII
        MTD_MAC_TYPE_DXGMII - NOT ALLOWED (must be selected by strap and cannot be changed through software)
        MTD_MAC_TYPE_QXGMII - NOT ALLOWED (must be selected by strap and cannot be changed through software)
        MTD_MAC_TYPE_5BR_SGMII_AN_EN - 5GBASE-R/2500BASE-X/SGMII AN En for 1G and below
        MTD_MAC_TYPE_5BR_SGMII_AN_DIS - 5GBASE-R/2500BASE-X/SGMII AN Dis for 1G and below
        MTD_MAC_TYPE_10GR_RATE_ADAP - 10GR/5GR/2.5GX Mac Interface, rate matching to same/lower speed copper

        For X35X0/E25X0 devices:
        MTD_MAC_TYPE_SXGMII            - Single port USXGMII
        MTD_MAC_TYPE_10G_DXGMII        - 10GBASE-R dual ports USXGMII
        MTD_MAC_TYPE_10G_QXGMII        - 10GBASE-R quad ports USXGMII
        MTD_MAC_TYPE_20G_DXGMII        - 20GBASE-R dual ports USXGMII, supports two ports of 10GBASE-T, X3540 only
        MTD_MAC_TYPE_20G_QXGMII        - 20GBASE-R quad ports USXGMII, supports four ports of 2.5/5GBASE-T, X3540 only
        MTD_MAC_TYPE_OXGMII            - Octal ports USXGMII, 20GBASE-R only, X3540 only
        MTD_MAC_TYPE_OXGMII_PARTNER    - Link partner mode for OXGMII mode, X3540 only
        MTD_MAC_TYPE_10GR_SGMII_AN_EN  - 10GBASE-R/5GBASE-R/2.5GBASE-X, or SGMII AN enabled for 1G and below
        MTD_MAC_TYPE_10GR_1000BX_AP_AN - 10GBASE-KR/5GBASE-KR/2.5GBASE-KX, or 1000BASE-KX (CL73 AN enabled)
                                       - 100M/10M are not supported
        MTD_MAC_TYPE_10GR_RM_AMD_OFF   - Rate matching with Auto-Media Detect Off
        MTD_MAC_TYPE_10GR_RM_AMD_ON    - Rate matching with Auto-Media Detect On
                                         X35X0 only supports 10GBASE-R rate matching to 5GBR/2.5GBX, not to 1G/100M/10M

        For E20X0 devices mactype, please refer to mtdSetMacInterfaceControl().

    macIfPowerDown - MTD_TRUE if the host interface is always to be powered up
                     MTD_FALSE if the host interface can be powered down under 
                     certain circumstances (see datasheet)
        For E21X0 devices, MTD_FALSE not allowed for all MP-USXGMII macTypes

    macLinkDownSpeed - The speed the mac interface should run when the media side is
        link down. One of the following:
            MTD_MAC_SPEED_10_MBPS
            MTD_MAC_SPEED_100_MBPS
            MTD_MAC_SPEED_1000_MBPS
            MTD_MAC_SPEED_10_GBPS

    macMaxIfSpeed -
        On X3540 it is set automatically, the return value tells you what it was
        set to.
  
        For X3540 device:
        MTD_MAX_MAC_SPEED_20G   (only applicable to DXGMII/QXGMII/OXGMII)
        MTD_MAX_MAC_SPEED_10G   (all other mac types)
  
        For other devices:
        MTD_MAX_MAC_SPEED_10G
        MTD_MAX_MAC_SPEED_5G
        MTD_MAX_MAC_SPEED_2P5G (on E21X0 device not allowed for macType SXGMII)

 Returns:
    MTD_OK or MTD_FAIL if query failed (for example an IO error occurred).

 Description:
    Reads parameters as currently selected in 31.F000/F001/F0A8/F007, 1.C04A
    based on device type and returns them.
        E20X0 - Reads 31.F000/F001/F0A8
        E21X0 - Reads 31.F000/F0A8/F007, 1.C04A
        X35X0/E25X0 - Reads 31.F000/F001/F007

    This is a simplified version of mtdGetMacInterfaceControl() for the
    copper-only PHYs, which has fewer parameters to return.

 Side effects:
    None

 Notes/Warnings:
    See notes for mtdGetMacInterfaceControl() for more details.

    On X3540/E2540, user should always call this API on the Master port of grouped port
    for DXGMII/QXGMII/OXGMII/OXGMII_PARTNER modes. For example, P0/P2 is the Master port of 
    DXGMII grouped ports (P0-P1, P2-P3), P0 is the Master of QXGMII/OXGMII/OXGMII_PARTNER mode.
    Calling this API on Slave ports may return invalid results.

    See E21X0 datasheet for a list of allowed from/to Mac Types via
    register modification. Not all combinations are allowed to be set
    via register write and some have special setup requirements. This function
    checks for those and handles them automatically. It will return MTD_FAIL
    if a change is requested that is not allowed.

******************************************************************************/
MTD_STATUS mtdGetMacInterfaceCopperOnlyPhy
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    OUT MTD_U16 *macType,
    OUT MTD_BOOL *macIfPowerDown,
    OUT MTD_U16 *macLinkDownSpeed,
    OUT MTD_U16 *macMaxIfSpeed 
);

/******************************************************************************
 For X35X0/E25X0 and E21X0 only
******************************************************************************/

/* For rateMatchingSpeed */
#define MTD_RM_MAC_SPEED_10G  (0) 
#define MTD_RM_MAC_SPEED_5G   (2)
#define MTD_RM_MAC_SPEED_2P5G (3)
/******************************************************************************
 MTD_STATUS mtdSetMacInterfaceRateMatchingSpeedCopperOnlyPhy
 (
     IN MTD_DEV_PTR devPtr,
     IN MTD_U16 port,
     IN MTD_U16 rateMatchingSpeed    
 );

 Inputs:
    devPtr - pointer to MTD_DEV initialized by mtdLoadDriver() call
    port - port number, 0-31
    
    rateMatchingSpeed - One of the following: 
            MTD_RM_MAC_SPEED_10G 
            MTD_RM_MAC_SPEED_5G
            MTD_RM_MAC_SPEED_2P5G
 Outputs:
    None
             

 Returns:
    MTD_OK or MTD_FAIL if query failed (for example an IO error occurred).

 Description:
    Sets the mac interface speed for the rate matching mode
    for the supported PHY. Does a software reset after writing the
    new value. 

 Side effects:
    None

 Notes/Warnings:
    This setting is only applicable to rate matching modes and only 
    applicable to the supported PHY.

******************************************************************************/
MTD_STATUS mtdSetMacInterfaceRateMatchingSpeedCopperOnlyPhy
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_U16 rateMatchingSpeed    
);

/******************************************************************************
 MTD_STATUS mtdGetMacInterfaceRateMatchingSpeedCopperOnlyPhy
 (
     IN MTD_DEV_PTR devPtr,
     IN MTD_U16 port,
     OUT MTD_U16 *rateMatchingSpeed    
 );


 Inputs:
    devPtr - pointer to MTD_DEV initialized by mtdLoadDriver() call
    port - port number, 0-31
    
 Outputs:
    rateMatchingSpeed - One of the following: 
            MTD_RM_MAC_SPEED_10G 
            MTD_RM_MAC_SPEED_5G
            MTD_RM_MAC_SPEED_2P5G
             

 Returns:
    MTD_OK or MTD_FAIL if query failed (for example an IO error occurred).

 Description:
    Reads and returns the current setting for the rate matching speed
    for the supported PHY.

 Side effects:
    None

 Notes/Warnings:
    This setting is only applicable to rate matching modes and only applicable
    to the supported PHY.

******************************************************************************/
MTD_STATUS mtdGetMacInterfaceRateMatchingSpeedCopperOnlyPhy
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    OUT MTD_U16 *rateMatchingSpeed    
);

/******************************************************************************
 USXGMII functions
******************************************************************************/

/******************************************************************************
 MTD_STATUS mtdGetMultiUSXGMIIConfigurationCopperOnlyPhy
 (
     IN MTD_DEV_PTR devPtr,
     IN MTD_U16 port,
     OUT MTD_U16 *usxgmiiPortId,
     OUT MTD_BOOL *usxgmiiPrimaryPort
 );


 Inputs:
    devPtr - pointer to MTD_DEV initialized by mtdLoadDriver() call
    port - port number, 0-31
    
 Outputs:
    usxgmiiPortId - port id for DXGMII or QXGMII mode
    usxgmiiPrimaryPort - MTD_TRUE if this is the primary (also
        sometimes called the Master) port or MTD_FALSE if not
             

 Returns:
    MTD_OK or MTD_FAIL if query failed (for example an IO error occurred).

 Description:
    Reads and returns the settings for DXGMII or QXGMII mode. usxgmiiPortId
    is the port id that identifies the order for muxing the data to the
    USXGMII primary port. usxgmiiPrimaryPort identifies the port that
    is the actual port that should be physically connected to the host
    serdes.

 Side effects:
    None

 Notes/Warnings:
    This function is only applicable to the E21X0 PHY (where X is 2 or
    greater, i.e. E2120, E2140, E2180).

******************************************************************************/
MTD_STATUS mtdGetMultiUSXGMIIConfigurationCopperOnlyPhy
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    OUT MTD_U16 *usxgmiiPortId,
    OUT MTD_BOOL *usxgmiiPrimaryPort
);



/******************************************************************************
 SGMII functions
******************************************************************************/

/******************************************************************************
 MTD_STATUS mtdGetSGMIIAutoneg_Advertisement
 (
     IN MTD_DEV_PTR devPtr,
     IN MTD_U16 port,
     OUT MTD_BOOL *link_status,
     OUT MTD_U16 *speed,
     OUT MTD_U16 *full_duplex,
     OUT MTD_BOOL *tx_pause,
     OUT MTD_BOOL *rx_pause,
     OUT MTD_BOOL *fiber_media,
     OUT MTD_BOOL *eee_enabled,
     OUT MTD_BOOL *clock_stop_lpi
 );


 Inputs:
    devPtr - pointer to MTD_DEV initialized by mtdLoadDriver() call
    port - port number, 0-31
    link_status - MTD_TRUE if PHY link is up, MTD_FALSE if link is down
    full_duplex - MTD_TRUE if PHY resolved to full duplex, MTD_FALSE if
                  PHY resolved to half duplex
    speed - one of the following:
       MTD_SGMII_SPEED_10M
       MTD_SGMII_SPEED_100M  
       MTD_SGMII_SPEED_1G    
    tx_pause - MTD_TRUE if transmit pause is enabled, MTD_FALSE if not
    rx_pause - MTD_TRUE if receive pause is enabled, MTD_FALSE if not
    fiber_media - MTD_TRUE if resolved to fiber media, MTD_FALSE if copper
    eee_enabled - MTD_TRUE if PHY resolved to use EEE, MTD_FALSE if not
    clock_stop_lpi - MTD_TRUE if the clock stops during LPI for EEE,
                     MTD_FALSE if not

 Outputs:
    None

 Returns:
    MTD_OK or MTD_FAIL

 Description:
    This function queries the SGMII autoneg result in 4.2004 (H unit
    auto-negotiation advertisement register) and returns the results.

    This function should be called after autonegotiation is complete to
    read the result.

 Side effects:
    None

 Notes/Warnings:
    This call is also used for 2.5G operation on all PHYs except X32X0.

******************************************************************************/
MTD_STATUS mtdGetSGMIIAutoneg_Advertisement
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    OUT MTD_BOOL *link_status,
    OUT MTD_BOOL *full_duplex,
    OUT MTD_U16 *speed,
    OUT MTD_BOOL *tx_pause,
    OUT MTD_BOOL *rx_pause,
    OUT MTD_BOOL *fiber_media,
    OUT MTD_BOOL *eee_enabled,
    OUT MTD_BOOL *clock_stop_lpi
);

#if MTD_ORIGSERDES

/******************************************************************************
 XFI functions
******************************************************************************/

/******************************************************************************
 MTD_STATUS mtdSetANKRControl
 (
     IN MTD_DEV_PTR devPtr,
     IN MTD_U16 port,
     IN MTD_BOOL enableANKR
 );

 Inputs:
    devPtr - pointer to MTD_DEV initialized by mtdLoadDriver() call
    port - port number, 0-31
    enableANKR - MTD_TRUE to enable AN+KR, MTD_FALSE to disable AN+KR

 Outputs:
    None

 Returns:
    MTD_OK or MTD_FAIL

 Description:
    This function enables or disables AN+KR on the H unit. The serdes link
    will drop and retrain when this function is executed.

 Side effects:
    None

 Notes/Warnings:
    None

******************************************************************************/
MTD_STATUS mtdSetANKRControl
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_BOOL enableANKR
);

/******************************************************************************
 MTD_STATUS mtdGetANKRControl
 (
     IN MTD_DEV_PTR devPtr,
     IN MTD_U16 port,
     OUT MTD_BOOL *enableANKR
 );

 Inputs:
    devPtr - pointer to MTD_DEV initialized by mtdLoadDriver() call
    port - port number, 0-31

 Outputs:
    enableANKR - MTD_TRUE if enabled AN+KR, MTD_FALSE if disabled AN+KR

 Returns:
    MTD_OK or MTD_FAIL

 Description:
    This function checks if AN+KR is enabled or disabled.

 Side effects:
    None

 Notes/Warnings:
    None

******************************************************************************/
MTD_STATUS mtdGetANKRControl
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    OUT MTD_BOOL *enableANKR
);


/******************************************************************************
 X2 (RXAUI) functions
******************************************************************************/

/******************************************************************************
 MTD_STATUS mtdSetX2SerdesLanePolarity
 (
     IN MTD_DEV_PTR devPtr,
     IN MTD_U16 port,
     IN MTD_BOOL invert_input_pol_l0,
     IN MTD_BOOL invert_output_pol_l0,
     IN MTD_BOOL invert_input_pol_l1,
     IN MTD_BOOL invert_output_pol_l1
 );

 Inputs:
    devPtr - pointer to MTD_DEV initialized by mtdLoadDriver() call
    port - port number, 0-31
    invert_input_pol_l0 - MTD_TRUE to invert the serdes input polarity, 
                          MTD_FALSE to leave it as-is (lane 0)
    invert_output_pol_l0 - MTD_TRUE to invert the serdes output polarity, 
                           MTD_FALSE to leave it as-is (lane 0)
    invert_input_pol_l1 - MTD_TRUE to invert the serdes input polarity, 
                          MTD_FALSE to leave it as-is (lane 1)
    invert_output_pol_l1 - MTD_TRUE to invert the serdes output polarity, 
                           MTD_FALSE to leave it as-is (lane 1)

 Outputs:
    None

 Returns:
    MTD_OK or MTD_FAIL

 Description:
    This function can be used to invert the input or output polarity of
    lane 0 or lane 1 in 10GBASE-X2 (RXAUI) mode.

 Side effects:
    None

 Notes/Warnings:
    In 10GBASE-X2 (RXAUI) mode, both lanes are controlled by 4.F004.15:12.    

    Call mtdSetSerdesLanePolarity() to control the polarity of the
    serdes lanes in 10GBASE-R, 1000BASE-X/SGMII modes.

******************************************************************************/
MTD_STATUS mtdSetX2SerdesLanePolarity
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_BOOL invert_input_pol_l0,
    IN MTD_BOOL invert_output_pol_l0,
    IN MTD_BOOL invert_input_pol_l1,
    IN MTD_BOOL invert_output_pol_l1
);


/******************************************************************************
 MTD_STATUS mtdGetX2SerdesLanePolarity
 (
     IN MTD_DEV_PTR devPtr,
     IN MTD_U16 port,
     IN MTD_BOOL *invert_input_pol_l0,
     IN MTD_BOOL *invert_output_pol_l0,
     IN MTD_BOOL *invert_input_pol_l1,
     IN MTD_BOOL *invert_output_pol_l1    
 );

 Inputs:
    devPtr - pointer to MTD_DEV initialized by mtdLoadDriver() call
    port - port number, 0-31
    HorXunit - MTD_H_UNIT or MTD_X_UNIT, depending on which lane is
                being gotten (see description below)

 Outputs:
    invert_input_pol - MTD_TRUE the serdes input polarity is inverted, 
                       MTD_FALSE it's not inverted
    invert_output_pol - MTD_TRUE the serdes output polarity is inverted, 
                       MTD_FALSE it's not inverted

 Returns:
    MTD_OK or MTD_FAIL

 Description:
    This function can be called to read the current polarity setting
    in 10GBASE-X2 (RXAUI) mode.

 Side effects:
    None

 Notes/Warnings:
    In 10GBASE-X2 (RXAUI) mode, both lanes are controlled by 4.F004.15:12.    

    Call mtdGetSerdesLanePolarity() to get the polarity of the
    serdes lanes in 10GBASE-R, 1000BASE-X/SGMII modes.

******************************************************************************/
MTD_STATUS mtdGetX2SerdesLanePolarity
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_BOOL *invert_input_pol_l0,
    IN MTD_BOOL *invert_output_pol_l0,
    IN MTD_BOOL *invert_input_pol_l1,
    IN MTD_BOOL *invert_output_pol_l1    
);


/******************************************************************************
 MTD_STATUS mtdSetX4SerdesLanePolarity
 (
     IN MTD_DEV_PTR devPtr,
     IN MTD_U16 port,
     IN MTD_U16 inputPolarityMask,
     IN MTD_U16 outputPolarityMask
 );

 Inputs:
    devPtr - pointer to MTD_DEV initialized by mtdLoadDriver() call
    port - port number, 0-31
    inputPolarityMask - LSB to bit 3 maps to lanes 0-3, set a 1
        to invert the input polarity or leave 0 for normal polarity
    outputPolarityMask - LSB to bit 3 maps to lanes 0-3, set a 1
        to invert the input polarity or leave 0 for normal polarity        

 Outputs:
    None

 Returns:
    MTD_OK or MTD_FAIL

 Description:
    This function can be used to invert the input or output polarity of
    lanes 0-3 for XAUI mode on X3310 devices.

 Side effects:
    None

 Notes/Warnings:
    In XAUI mode, all lanes are controlled by 4.F004. 

    Examples:
    mtdSetX4SerdesLanePolarity(devPtr,port,3,3); inverts input and output
                                                 polarity on lanes 0-1
    

******************************************************************************/
MTD_STATUS mtdSetX4SerdesLanePolarity
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_U16 inputPolarityMask,
    IN MTD_U16 outputPolarityMask
);


/******************************************************************************
 MTD_STATUS mtdGetX4SerdesLanePolarity
 (
     IN MTD_DEV_PTR devPtr,
     IN MTD_U16 port,
     IN MTD_U16 *inputPolarityMask,
     IN MTD_U16 *outputPolarityMask
 );


 Inputs:
    devPtr - pointer to MTD_DEV initialized by mtdLoadDriver() call
    port - port number, 0-31

 Outputs:
    inputPolarityMask - LSB to bit 3 maps to lanes 0-3, 1 indicates
        that lane's polarity is inverted
    outputPolarityMask - LSB to bit 3 maps to lanes 0-3, 1 indicates
        that lane's polarity is inverted        

 Returns:
    MTD_OK or MTD_FAIL

 Description:
    This function can be used to read the polarity setting for the
    input and output polarity of lanes 0-3 on the X3310 device
    in XAUI mode.

 Side effects:
    None

 Notes/Warnings:
    In XAUI mode, all lanes are controlled by 4.F004. 

    See example in mtdSetX4SerdesLanePolarity().

******************************************************************************/
MTD_STATUS mtdGetX4SerdesLanePolarity
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_U16 *inputPolarityMask,
    IN MTD_U16 *outputPolarityMask
);

#endif /* MTD_ORIGSERDES */


#if C_LINKAGE
#if defined __cplusplus
}
#endif
#endif


#endif

