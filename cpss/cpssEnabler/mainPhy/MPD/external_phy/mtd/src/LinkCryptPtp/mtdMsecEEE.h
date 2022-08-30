/*******************************************************************************
Copyright (C) 2014 - 2019, Marvell International Ltd. and its affiliates
If you received this File from Marvell and you have entered into a commercial
license agreement (a "Commercial License") with Marvell, the File is licensed
to you under the terms of the applicable Commercial License.
*******************************************************************************/

/********************************************************************
This file contains functions and global data for
higher-level functions using MDIO access to control and read 
status of the MACsec Legacy Mode Buffer energy efficient ethernet (EEE) 
functions of the Marvell 88X32X0, 88X33X0 and 88E20X0 ethernet PHYs.
********************************************************************/

#ifndef MTD_MSEC_EEE_H
#define MTD_MSEC_EEE_H

#if C_LINKAGE
#if defined __cplusplus
    extern "C" {
#endif
#endif

#if MTD_LINKCRYPT_MSECPTP

/******************************************************************************
 MTD_STATUS mtdMACSECEeeBufferConfig
 (
     IN MTD_DEV_PTR devPtr,
     IN MTD_U16 port,
     IN MTD_U8 EEE_Mode_Control, 
     IN MTD_U8 XGMII_Enter_Timeout, 
     IN MTD_U8 GMII_Enter_Timeout, 
     IN MTD_U8 MII_Enter_Timeout, 
     IN MTD_U8 XGMII_Exit_Timeout, 
     IN MTD_U8 GMII_Exit_Timeout, 
     IN MTD_U8 MII_Exit_Timeout, 
     IN MTD_BOOL Force_Sys_LPI, 
     IN MTD_BOOL Force_Wre_LPI 
 );


 Inputs: 
    devPtr - pointer to MTD_DEV initialized by mtdLoadDriver() call
    port - MDIO port address, 0-31

    EEE_Mode_Control - One of the following:
       MTD_EEE_MODE_DISABLE, to disable EEE
       MTD_EEE_MODE_ENABLE_NO_LEGACY, for MACs which are capable of generating LPI
       MTD_EEE_MODE_ENABLE_WITH_LEGACY, for MACs which are not capable of generating LPI

    XGMII_Enter_Timeout - Sets the delay in 100ns steps from the start of idle
      until the buffer begins sending Low Power Idle in 10G mode. Default is
      0xFF (26 us). 0-255, set to 0 or 0xFF to get default setting.
      
    GMII_Enter_Timeout - Sets the delay in microseconds from the start of idle
      until the buffer begins sending Low Power Idle in 1G mode. Default is
      18 us. 0-255. Set to 0 or 18 to get default setting.

    MII_Enter_Timeout - Sets the delay in microseconds from the start of idle
      until the buffer begins sending Low Power Idle in 100M mode. Default is
      32 us. 0-255. Set to 0 or 32 to get default setting.

    XGMII_Exit_Timeout - Sets the delay in 100ns steps from the first frame 
      received until the buffer sends the frame in 10G mode. Default is
      0x4B (7.5 us). 0-255, set to 0 or 0x4B to get default setting.

    GMII_Exit_Timeout - Sets the delay in microseconds from the first frame 
      received until the buffer sends the frame in 1G mode. Default is
      18 us. 0-255. Set to 0 or 18 to get default setting.

    MII_Exit_Timeout - Sets the delay in microseconds from the first frame 
      received until the buffer sends the frame in 100M mode. Default is
      32 us. 0-255. Set to 0 or 32 to get default setting.

    Force_Sys_LPI - MTD_TRUE forces LPI symbols towards the System (MAC)

    Force_Wre_LPI - MTD_TRUE forces LPI symbols towards the Wire (PHY)

 Outputs:
    None

 Returns:
    MTD_OK if action was successfully taken, MTD_FAIL if not

 Description:
    This function sets the MACSEC EEE Legacy Mode buffer configurations in 
    31.809C to 31.809F.
    Enables EEE mode in the MACSEC so that LPI symbols may be passed to the 
    T unit.
    Optionally enables the EEE Legacy Mode buffer so that EEE may be used with a 
    MAC that is not capable of sourcing the Low Power Idle symbols. During periods 
    without transmit activity the EEE buffer will convert idle symbols to 
    low power idle symbols. The buffer will wait 'enter timer' microseconds 
    and begin sending LPI. Any frame received will be buffered until the exit 
    from LPI. Upon receiving a frame the buffer will send Idle for 'exit timer'
    microseconds before sending the frame.

 Side effects:
    None

 Notes/Warnings:
    mtdMACSECEeeBufferConfig() is applicable only for the case when MACSEC is enabled,
     (31.F000.13:12 = 11). If MACSEC is not enabled, this function will return MTD_FAIL.

    Per IEEE 802.3 78.1.2.1.2, the EEE Buffer must be disabled for 1 second after 
    initial link up and may then be enabled.
        **This function must be called EVERY time the link transitions from down
        to up for speeds where EEE was resolved as supported after a 1 second delay.
        ** Note that the XGMII_Enter_Timeout and MII_Exit_Timeout timers are set
        in 100ns steps, the other timers are set in 1us steps.

    If EEE is disabled, be certain to turn off advertisement by calling
    mtdAdvert_EEE() and restarting AN, otherwise the link may not be stable.

    Usage Notes:

    The calls
    mtdMACSECEeeBufferConfig(devPtr,port,MTD_EEE_MODE_DISABLE,0,0,0,0,0,0,
                             MTD_FALSE,MTD_FALSE)
    will disable the EEE and set the timeouts to the suggested values.

    mtdMACSECEeeBufferConfig(devPtr,port,MTD_EEE_MODE_ENABLE_NO_LEGACY,0,0,0,0,0,0,
                             MTD_FALSE,MTD_FALSE)
    will enable the EEE and set the timeouts to the suggested values. This
    is the call for a MAC which supports sending its own LPI signalling to/through
    the MACSEC.

    mtdMACSECEeeBufferConfig(devPtr,port,MTD_EEE_MODE_ENABLE_NO_LEGACY,0,0,0,0,0,0,
                             MTD_FALSE,MTD_FALSE)
    will enable the EEE and set the timeouts to the suggested values. This
    is the call for a MAC which does not support sending LPI signalling, and
    is requesting the MACSEC to do it when the MACSEC internal data buffer is 
    empty.

    To force LPI in one direction or the other, change one or both of the above
    MTD_FALSE to MTD_TRUE.

******************************************************************************/
MTD_STATUS mtdMACSECEeeBufferConfig
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_U8 EEE_Mode_Control, 
    IN MTD_U8 XGMII_Enter_Timeout, 
    IN MTD_U8 GMII_Enter_Timeout, 
    IN MTD_U8 MII_Enter_Timeout, 
    IN MTD_U8 XGMII_Exit_Timeout, 
    IN MTD_U8 GMII_Exit_Timeout, 
    IN MTD_U8 MII_Exit_Timeout, 
    IN MTD_BOOL Force_Sys_LPI, 
    IN MTD_BOOL Force_Wre_LPI 
);

/******************************************************************************
 MTD_STATUS mtdMACSECEeeBufferConfig5G_2P5G
 (
     IN MTD_DEV_PTR devPtr,
     IN MTD_U16 port,
     IN MTD_U8 FiveG_Enter_Timeout, 
     IN MTD_U8 TwoP5G_Enter_Timeout, 
     IN MTD_U8 FiveG_Exit_Timeout, 
     IN MTD_U8 TwoP5G_Exit_Timeout
 );


 Inputs: 
    devPtr - pointer to MTD_DEV initialized by mtdLoadDriver() call
    port - MDIO port address, 0-31

    FiveG_Enter_Timeout - Sets the delay in 100ns steps from the start of idle
      until the buffer begins sending Low Power Idle in 10G mode. Default is
      0xB4 (18 us). 0-255, set to 0 or 0xB4 to get default setting.
      
    TwoP5G_Enter_Timeout - Sets the delay in microseconds from the start of idle
      until the buffer begins sending Low Power Idle in 1G mode. Default is
      18 us. 0-255. Set to 0 or 18 to get default setting.

    FiveG_Exit_Timeout - Sets the delay in 100ns steps from the first frame 
      received until the buffer sends the frame in 10G mode. Default is
      0x96 (15 us). 0-255, set to 0 or 0x96 to get default setting.

    TwoP5G_Exit_Timeout - Sets the delay in microseconds from the first frame 
      received until the buffer sends the frame in 1G mode. Default is
      30 us. 0-255. Set to 0 or 30 to get default setting.

 Outputs:
    None

 Returns:
    MTD_OK if action was successfully taken, MTD_FAIL if not

 Description:
    Modifies the enter/exit timers for 5G/2.5G EEE. Pass 0 for a parameter
    to use the default setting.

    See notes below for usage instructions.

 Side effects:
    None

 Notes/Warnings:
    Use mtdMACSECEeeBufferConfig() first to configure MacSec EEE buffer,
    then call this function only to change the 5G/2.5G Enter/Exit timers
    on X33x0/E20x0/E21x0 devices.

    This function is only applicable to X33x0/E20x0/E21x0 devices.

******************************************************************************/
MTD_STATUS mtdMACSECEeeBufferConfig5G_2P5G
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_U8 FiveG_Enter_Timeout, 
    IN MTD_U8 TwoP5G_Enter_Timeout, 
    IN MTD_U8 FiveG_Exit_Timeout, 
    IN MTD_U8 TwoP5G_Exit_Timeout
);

/******************************************************************************
 MTD_STATUS mtdGetMACSECEeeBufferConfig
 (
     IN MTD_DEV_PTR devPtr,
     IN MTD_U16 port,
     OUT MTD_U8 *EEE_Mode_Control, 
     OUT MTD_U8 *XGMII_Enter_Timeout, 
     OUT MTD_U8 *GMII_Enter_Timeout, 
     OUT MTD_U8 *MII_Enter_Timeout, 
     OUT MTD_U8 *XGMII_Exit_Timeout, 
     OUT MTD_U8 *GMII_Exit_Timeout, 
     OUT MTD_U8 *MII_Exit_Timeout, 
     OUT MTD_BOOL *Force_Sys_LPI, 
     OUT MTD_BOOL *Force_Wre_LPI 
 );


 Inputs: 
    devPtr - pointer to MTD_DEV initialized by mtdLoadDriver() call
    port - MDIO port address, 0-31

 Outputs:
    EEE_Mode_Control - One of the following:
       MTD_EEE_MODE_DISABLE, to disable EEE
       MTD_EEE_MODE_ENABLE_NO_LEGACY, for MACs which are capable of generating LPI
       MTD_EEE_MODE_ENABLE_WITH_LEGACY, for MACs which are not capable of generating LPI

    XGMII_Enter_Timeout - delay in 100ns steps from the start of idle
      until the buffer begins sending Low Power Idle in 10G mode. 
      
    GMII_Enter_Timeout - delay in microseconds from the start of idle
      until the buffer begins sending Low Power Idle in 1G mode. 

    MII_Enter_Timeout - delay in microseconds from the start of idle
      until the buffer begins sending Low Power Idle in 100M mode. 

    XGMII_Exit_Timeout - delay in 100ns steps from the first frame 
      received until the buffer sends the frame in 10G mode. 

    GMII_Exit_Timeout - delay in microseconds from the first frame 
      received until the buffer sends the frame in 1G mode. 

    MII_Exit_Timeout - delay in microseconds from the first frame 
      received until the buffer sends the frame in 100M mode. 

    Force_Sys_LPI - MTD_TRUE/MTD_FALSE

    Force_Wre_LPI - MTD_TRUE/MTD_FALSE

 Returns:
    MTD_OK if query was successful, MTD_FAIL if not

 Description:
    This function returns the MACSEC EEE Mode buffer configurations as set in 
    31.809C to 31.809F by call to mtdMACSECEeeBufferConfig()

 Side effects:
    None

 Notes/Warnings:
    MACSEC must be enabled to read the configuration. This function will return
    MTD_FAIL if MACSEC is disabled (31.F000.13:12 is not 11b).

******************************************************************************/
MTD_STATUS mtdGetMACSECEeeBufferConfig
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    OUT MTD_U8 *EEE_Mode_Control, 
    OUT MTD_U8 *XGMII_Enter_Timeout, 
    OUT MTD_U8 *GMII_Enter_Timeout, 
    OUT MTD_U8 *MII_Enter_Timeout, 
    OUT MTD_U8 *XGMII_Exit_Timeout, 
    OUT MTD_U8 *GMII_Exit_Timeout, 
    OUT MTD_U8 *MII_Exit_Timeout, 
    OUT MTD_BOOL *Force_Sys_LPI, 
    OUT MTD_BOOL *Force_Wre_LPI 
);


/******************************************************************************
 MTD_STATUS mtdGetMACSECEeeBufferConfig5G_2P5G
 (
     IN MTD_DEV_PTR devPtr,
     IN MTD_U16 port,
     OUT MTD_U8 *FiveG_Enter_Timeout, 
     OUT MTD_U8 *TwoP5G_Enter_Timeout, 
     OUT MTD_U8 *FiveG_Exit_Timeout, 
     OUT MTD_U8 *TwoP5G_Exit_Timeout
 );


 Inputs: 
    devPtr - pointer to MTD_DEV initialized by mtdLoadDriver() call
    port - MDIO port address, 0-31

 Outputs:
    FiveG_Enter_Timeout - delay in 100ns steps from the start of idle
      until the buffer begins sending Low Power Idle in 10G mode. 
      
    TwoP5G_Enter_Timeout - delay in microseconds from the start of idle
      until the buffer begins sending Low Power Idle in 1G mode. 

    FiveG_Exit_Timeout - delay in 100ns steps from the first frame 
      received until the buffer sends the frame in 10G mode. 

    TwoP5G_Exit_Timeout - delay in microseconds from the first frame 
      received until the buffer sends the frame in 1G mode. 

 Returns:
    MTD_OK if action was successfully taken, MTD_FAIL if not

 Description:
    Reads the 5G/2.5G enter/exit timer values from the MacSec
    EEE buffer configuration registers.

 Side effects:
    None

 Notes/Warnings:
    This function is only applicable to X33x0/E20x0/E21x0 devices.

******************************************************************************/
MTD_STATUS mtdGetMACSECEeeBufferConfig5G_2P5G
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    OUT MTD_U8 *FiveG_Enter_Timeout, 
    OUT MTD_U8 *TwoP5G_Enter_Timeout, 
    OUT MTD_U8 *FiveG_Exit_Timeout, 
    OUT MTD_U8 *TwoP5G_Exit_Timeout
);

#endif   /* MTD_LINKCRYPT_MSECPTP */

#if C_LINKAGE
#if defined __cplusplus
}
#endif
#endif

#endif /* MTD_MSEC_EEE_H */
