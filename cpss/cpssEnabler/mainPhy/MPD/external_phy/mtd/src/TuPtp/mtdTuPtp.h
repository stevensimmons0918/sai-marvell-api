/*******************************************************************************
Copyright (C) 2014 - 2020, Marvell International Ltd. and its affiliates
If you received this File from Marvell and you have entered into a commercial
license agreement (a "Commercial License") with Marvell, the File is licensed
to you under the terms of the applicable Commercial License.
*******************************************************************************/

/********************************************************************
This file contains functions, definitions and data structures for  
Precision Time Protocol(PTP) feature for the 88X35X0 Ethernet PHYs.
********************************************************************/

#ifndef MTD_TU_PTP_H
#define MTD_TU_PTP_H

#if MTD_TU_PTP

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
MTD_STATUS mtdTuPTPConfigPTPBlock
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_U16 enablePTPBlock,
    IN MTD_BOOL doSwReset
);

 Inputs:
    devPtr - pointer to MTD_DEV initialized by mtdLoadDriver() call
    port - MDIO port address, 0-31
    enablePTPBlock - MTD_ENABLE to enable the PTP block or MTD_DISABLE to disable it
    doSwReset - MTD_TRUE to perform a T-unit soft reset and PTP block soft reset
                to apply changes, or MTD_FALSE otherwise. See note below.

 Outputs:
    None.

 Returns:
    MTD_OK   - on success
    MTD_FAIL - on error

 Description:
    The PTP block needs to be configured and enabled for PTP protocol operations. This API 
    will set all the required registers needed to enable or disable the PTP block, including
    disabling the PTP bypass when enabled the PTP.

 Side effects:
    None.

 Notes/Warnings:
    mtdTuPTPConfigPTPBlock() must be called at PTP initial configuration for
    the PTP protocol.
    T-unit soft reset and PTP soft reset are needed for changes to be taken effect.
   
    If doSwReset is MTD_FALSE, mtdSoftwareReset() should be called to issue
    a T-unit soft reset. mtdTuPTPSoftReset() should be called to issue a PTP soft reset.

    Suggest always setting doSwReset to MTD_TRUE to avoid problems of
    possibly losing changes.
******************************************************************************/
MTD_STATUS mtdTuPTPConfigPTPBlock
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_U16 enablePTPBlock,
    IN MTD_BOOL doSwReset
);


/*******************************************************************************
MTD_STATUS mtdTuPTPSoftReset
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port
);

 Inputs:
    devPtr - pointer to MTD_DEV initialized by mtdLoadDriver() call
    port - MDIO port address, 0-31

 Outputs:
    None

 Returns:
    MTD_OK   - on success
    MTD_FAIL - on error

 Description:
    Performs a soft reset to PTP block, reset the PTP logic.

 Side effects:
    None

 Notes/Warnings:
    None
*******************************************************************************/
MTD_STATUS mtdTuPTPSoftReset
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port
);


/*******************************************************************************
MTD_STATUS mtdTuPTPSetBypass
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_U16 ptpBypass 
);

 Inputs:
    devPtr - pointer to MTD_DEV initialized by mtdLoadDriver() call
    port - MDIO port address, 0-31
    ptpBypass - 0: no PTP bypass; 1:enable PTP bypass
                 
 Outputs:
    None

 Returns:
    MTD_OK   - on success
    MTD_FAIL - on error

 Description:
    This PTP bypass option will bypass the PTP path if the ptpBypass is set to 1
    Enabling this option will make XGMII/GMII path to bypass PTP 

 Side effects:
    None

 Notes/Warnings:
    None
*******************************************************************************/
MTD_STATUS mtdTuPTPSetBypass
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_U16 ptpBypass
);


/*******************************************************************************
MTD_STATUS mtdTuPTPGetBypass
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    OUT MTD_U16 *ptpBypass 
);

 Inputs:
    devPtr - pointer to MTD_DEV initialized by mtdLoadDriver() call
    port - MDIO port address, 0-31

 Outputs:
    ptpDisable - whether PTP is bypassed
      0 - no PTP bypass
      1 - enable PTP bypass

 Returns:
    MTD_OK   - on success
    MTD_FAIL - on error

 Description:
    This API returns the PTP bypass option. This PTP bypass option will 
    bypass the PTP path if the ptpBypass is set to 1

 Side effects:
    None

 Notes/Warnings:
    None
*******************************************************************************/
MTD_STATUS mtdTuPTPGetBypass
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    OUT MTD_U16 *ptpBypass /* 0:set PTP bypass; 1:disable PTP bypass */
);


/*******************************************************************************
MTD_STATUS mtdTuPTPSetEtherType
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_U16 etherType
);

 Inputs:
    devPtr - pointer to MTD_DEV initialized by mtdLoadDriver() call
    port - MDIO port address, 0-31
    etherType  - PTP Ether Type

 Outputs:
   None.

 Returns:
    MTD_OK   - on success
    MTD_FAIL - on error

 Description:
    This routine sets the Precise Time Protocol EtherType(PTPEType) expected in the packet.
    All layer 2 PTP frames are recognized using a combination of a specific EtherType 
    and MessageType values (part of the PTP Common Header). This field is used to 
    identify the EtherType on these frames.

    Call mtdTuPTPGetMsgTSEnable() to get the types of frames that the hardware need
    to time stamp.

    For IEEE 802.1AS and IEEE1588 over Layer 2 Ethernet, the EtherType is expected to 
    be programmed to 0x88F7.

    Side effects:
    None

 Notes/Warnings:
    None
*******************************************************************************/
MTD_STATUS mtdTuPTPSetEtherType
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_U16 etherType
);


/*******************************************************************************
MTD_STATUS mtdTuPTPGetEtherType
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    OUT MTD_U16 *etherType
);

 Inputs:
    devPtr - pointer to MTD_DEV initialized by mtdLoadDriver() call
    port - MDIO port address, 0-31

 Outputs:
    etherType  - PTP Ether Type

 Returns:
    MTD_OK   - on success
    MTD_FAIL - on error

 Description:
    This routine gets the Precise Time Protocol EtherType(PTPEType).
    All layer 2 PTP frames are recognized using a combination of a specific EtherType 
    and MessageType values (part of the PTP Common Header). This field is used to 
    identify the EtherType on these frames.

    Call mtdTuPTPGetMsgTSEnable() to get the types of frames that the hardware need
    to time stamp.

    For IEEE 802.1AS and IEEE1588 over Layer 2 Ethernet, the EtherType is expected to 
    be programmed to 0x88F7.

 Side effects:
    None

 Notes/Warnings:
    None
*******************************************************************************/
MTD_STATUS mtdTuPTPGetEtherType
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    OUT MTD_U16 *etherType
);


/*******************************************************************************
MTD_STATUS mtdTuPTPSetMsgTSEnable
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_U16 msgTSEnable
);

 Inputs:
    devPtr - pointer to MTD_DEV initialized by mtdLoadDriver() call
    port - MDIO port address, 0-31
    msgTSEnable  - Message Type Time Stamp Enable, see the description below

 Outputs:
   None.

 Returns:
    MTD_OK   - on success
    MTD_FAIL - on error

 Description:
    The API sets the Message Type Time Stamp Enable.

    MessageType is part of the PTP common header, which identifies the PTP frame types
    that need to be time stamped for frames that are not hardware accelerated.

    The MessageType read from PTP frames is vectorized and mapped to the parameter
    msgTSEnable in the register MTD_TU_PTP_GLOBAL_MSG_TYPE. If the selected bit is set
    to 1 then that frame type will be time stamped both in ingress and egress,
    otherwise the frame type will not be time stamped.

    For example if MessageType field (in the PTP common header) with a value of 0x4 
    needs to be time stamped in hardware then bit 4 of msgTSEnable should be set to 1.

    Call mtdTuPTPSetTimeStampArrivalPtr() then set bit 4 of tsArrPtr to select if PTPArr0Time
    or PTPArr1Time should be used to store the incoming PTP frames' time stamp.
    All outgoing PTP frames with the MessageType field of 0x4 will be timestamped into the
    Port's PTPDepTime capture register. See mtdTuPTPGetTimeStamp().

 Side effects:
    None

 Notes/Warnings:
    None
*******************************************************************************/
MTD_STATUS mtdTuPTPSetMsgTSEnable
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_U16 msgTSEnable
);


/*******************************************************************************
MTD_STATUS mtdTuPTPGetMsgTSEnable
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    OUT MTD_U16 *msgTSEnable
);

 Inputs:
    devPtr - pointer to MTD_DEV initialized by mtdLoadDriver() call
    port - MDIO port address, 0-31

 Outputs:
    msgTSEnable  - Message Type Time Stamp Enable

 Returns:
    MTD_OK   - on success
    MTD_FAIL - on error

 Description:
    The API gets the Message Type Time Stamp Enable.

    MessageType is part of the PTP common header, which identifies the PTP frame types
    that need to be time stamped for frames that are not hardware accelerated.

    The MessageType read from PTP frames is vectorized and mapped to the parameter
    msgTSEnable in the register MTD_TU_PTP_GLOBAL_MSG_TYPE. If the selected bit is set
    to 1 then that frame type will be time stamped both in ingress and egress,
    otherwise the frame type will not be time stamped.

    For example if MessageType field (in the PTP common header) with a value of 0x4 
    needs to be time stamped in hardware then bit 4 of msgTSEnable should be set to 1.

    Call mtdTuPTPSetTimeStampArrivalPtr() then set bit 4 of tsArrPtr to select if PTPArr0Time
    or PTPArr1Time should be used to store the incoming PTP frames' time stamp.
    All outgoing PTP frames with the MessageType field of 0x4 will be timestamped into the
    Port's PTPDepTime capture register. See mtdTuPTPGetTimeStamp().
 
 Side effects:
    None

 Notes/Warnings:
    None
*******************************************************************************/
MTD_STATUS mtdTuPTPGetMsgTSEnable
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    OUT MTD_U16 *msgTSEnable
);


/*******************************************************************************
MTD_STATUS mtdTuPTPSetAltScheme
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_U16 altScheme
);

 Inputs:
    devPtr - pointer to MTD_DEV initialized by mtdLoadDriver() call
    port - MDIO port address, 0-31
    altScheme - Alternate Scheme 

 Outputs:
       None.

 Returns:
    MTD_OK   - on success
    MTD_FAIL - on error

 Description:
    This API sets the PTP Alternate Scheme.
   
    The Alternate Scheme defines the values that are returned in the 
    requestReceiptTimestamp field in IEEE 1588 pDelay Response messages
    and the correctionField & responseOriginTimestamp in pDelay Response
   
    Follow Up messages as follows:
       0 - requestReceiptTimestamp = t2,
           responseOriginTimestamp = t3
           correctionField = 0 (fractional nano sec)
       1 - requestReceiptTimestamp = 0
           responseOriginTimestamp = 0
           correctionField = turn around time

 Side effects:
    None

 Notes/Warnings:
    This bit has no effect if the PTP frame is an IEEE 802.1AS frame as
    indicated by the frame's TransSpec field, read by mtdTuPTPGetPortConfig().
*******************************************************************************/
MTD_STATUS mtdTuPTPSetAltScheme
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_U16 altScheme
);


/*******************************************************************************
MTD_STATUS mtdTuPTPGetAltScheme
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    OUT MTD_U16 *altScheme
);

 Inputs:
    devPtr - pointer to MTD_DEV initialized by mtdLoadDriver() call
    port - MDIO port address, 0-31

 Outputs:
    altScheme - Alternate Scheme 

 Returns:
    MTD_OK   - on success
    MTD_FAIL - on error

 Description:
    This API gets the PTP Alternate Scheme. 
   
    The Alternate Scheme defines the values that are returned in the
    requestReceiptTimestamp field in IEEE 1588 pDelay Response messages
    and the correctionField & responseOriginTimestamp in pDelay Response
   
    Follow Up messages as follows:
        0 - requestReceiptTimestamp = t2,
            responseOriginTimestamp = t3
            correctionField = 0 (fractional nano sec)
        1 - requestReceiptTimestamp = 0
            responseOriginTimestamp = 0
            correctionField = turn around time

 Side effects:
    None

 Notes/Warnings:
    This bit has no effect if the PTP frame is an IEEE 802.1AS frame as
    indicated by the frame's TransSpec field, read by mtdTuPTPGetPortConfig().
*******************************************************************************/
MTD_STATUS mtdTuPTPGetAltScheme
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    OUT MTD_U16 *altScheme
);


/*******************************************************************************
MTD_STATUS mtdTuPTPSetGrandMaster
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_U16 grandMaster
);

 Inputs:
    devPtr - pointer to MTD_DEV initialized by mtdLoadDriver() call
    port - MDIO port address, 0-31
    grandMaster - Grand Master Enable 
        0 - Hardware accelerate with this device NOT being the Grand Master
        1 - Hardware accelerate with this device being the Grand Master

 Outputs:
       None.

 Returns:
    MTD_OK   - on success
    MTD_FAIL - on error

 Description:
    This API sets the Grand Master Enable. 
    By enabling this hardware support for the port, the port will act
    as the PTP Grand Master.
    
 Side effects:
    None

 Notes/Warnings:
    None
*******************************************************************************/
MTD_STATUS mtdTuPTPSetGrandMaster
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_U16 grandMaster
);


/*******************************************************************************
MTD_STATUS mtdTuPTPGetGrandMaster
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    OUT MTD_U16 *grandMaster
);

 Inputs:
    devPtr - pointer to MTD_DEV initialized by mtdLoadDriver() call
    port - MDIO port address, 0-31

 Outputs:
    grandMaster - Grand Master Enable 
        0 - Hardware accelerate with this device NOT being the Grand Master
        1 - Hardware accelerate with this device being the Grand Master

 Returns:
    MTD_OK   - on success
    MTD_FAIL - on error

 Description:
    This API gets the Grand Master Enable. 
    If this hardware support is enabled on the given port, the port will act
    as the PTP Grand Master.

 Side effects:
    None

 Notes/Warnings:
*******************************************************************************/
MTD_STATUS mtdTuPTPGetGrandMaster
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    OUT MTD_U16 *grandMaster
);


/*******************************************************************************
MTD_STATUS mtdTuPTPSetPTPMode
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_U16 ptpMode
);

 Inputs:
    devPtr - pointer to MTD_DEV initialized by mtdLoadDriver() call
    port - MDIO port address, 0-31
    ptpMode - PTP Mode:
        00 - Boundary Clock
        01 - Peer to Peer Transparent Clock
        10 - End to End Transparent Clock
        11 - Reserved for future use

 Outputs:
       None.

 Returns:
    MTD_OK   - on success
    MTD_FAIL - on error

 Description:
    This API sets the PTP Mode on the given port.
   
    For these settings to have an effect, call mtdTuPTPTODStoreOperation()/
    mtdTuPTPSetPulse() to set at configure one Time Array to be used.
    This mode will then take effect on the ports whose Hardware Acceleration
    is enabled by mtdTuPTPSetCfgHWAccel().
    
 Side effects:
    None

 Notes/Warnings:
    None
*******************************************************************************/
MTD_STATUS mtdTuPTPSetPTPMode
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_U16 ptpMode
);


/*******************************************************************************
MTD_STATUS mtdTuPTPGetPTPMode
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    OUT MTD_U16 *ptpMode
);

 Inputs:
    devPtr - pointer to MTD_DEV initialized by mtdLoadDriver() call
    port - MDIO port address, 0-31

 Outputs:
    ptpMode - PTP Mode:
        00 - Boundary Clock
        01 - Peer to Peer Transparent Clock
        10 - End to End Transparent Clock
        11 - Reserved for future use

 Returns:
    MTD_OK   - on success
    MTD_FAIL - on error

 Description:
    This API gets the PTP Mode on the given port.
   
    The mode will take effect on the ports whose Hardware Acceleration
    is enabled by mtdTuPTPSetCfgHWAccel().
    
 Side effects:
    None

 Notes/Warnings:
    None
*******************************************************************************/
MTD_STATUS mtdTuPTPGetPTPMode
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    OUT MTD_U16 *ptpMode
);


/*******************************************************************************
MTD_STATUS mtdTuPTPSetOneStep
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_U16 oneStep
);

 Inputs:
    devPtr - pointer to MTD_DEV initialized by mtdLoadDriver() call
    port - MDIO port address, 0-31
    oneStep - PTP OneStep Enable:
        0 - Hardware accelerate using AutoFollowUp Two Step frame formats
        1 - Hardware accelerate using One Step frame formats

 Outputs:
       None.

 Returns:
    MTD_OK   - on success
    MTD_FAIL - on error

 Description:
    This API sets whether the port should use One Step frame formats or
    Two Step frame formats.

 Side effects:
    None

 Notes/Warnings:
    X3540 does not support the receiving of One Step frames
    and the hardware conversion of these frames into Two Step when the
    PTPMode set by mtdTuPTPSetPTPMode() is 0b10(End to End Transparent Clock).
*******************************************************************************/
MTD_STATUS mtdTuPTPSetOneStep
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_U16 oneStep
);


/*******************************************************************************
MTD_STATUS mtdTuPTPGetOneStep
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    OUT MTD_U16 *oneStep
);

 Inputs:
    devPtr - pointer to MTD_DEV initialized by mtdLoadDriver() call
    port - MDIO port address, 0-31

 Outputs:
    oneStep - PTP OneStep Enable:
        0 - Hardware accelerate using AutoFollowUp Two Step frame formats
        1 - Hardware accelerate using One Step frame formats

 Returns:
    MTD_OK   - on success
    MTD_FAIL - on error

 Description:
    This API gets the PTP OneStep Enable config, which indicate whether
    the port should use One Step frame formats or Two Step frame formats.

 Side effects:
    None

 Notes/Warnings:
    X3540 does not support the receiving of One Step frames
    and the hardware conversion of these frames into Two Step when the
    PTPMode set by mtdTuPTPSetPTPMode() is 0b10(End to End Transparent Clock).
*******************************************************************************/
MTD_STATUS mtdTuPTPGetOneStep
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    OUT MTD_U16 *oneStep
);


/*******************************************************************************
MTD_STATUS mtdTuPTPSetCfgTransSpec
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_U16 transSpec
);

 Inputs:
    devPtr - pointer to MTD_DEV initialized by mtdLoadDriver() call
    port - MDIO port address, 0-31
    transSpec - Transport Specific bits present in PTP Common header

 Outputs:
    None

 Returns:
    MTD_OK   - on success
    MTD_FAIL - on error

 Description:
  The Transport Specific bits present in PTP Common header are used to differentiate 
  between IEEE1588, IEEE802.1AS etc. frames. This is to differentiate between 
  various timing protocols running on either Layer2 or higher protocol layers.

  For IEEE 1588 networks this is expected to be configured to a 0x0 and for IEEE 802.1AS 
  networks this is expected to be configured to 0x1.

 Side effects:
    None

 Notes/Warnings:
    The only valid TransSpec values for PTP hardware acceleration are 0x0 and 0x1.
*******************************************************************************/
MTD_STATUS mtdTuPTPSetCfgTransSpec
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_U16 transSpec
);


/*******************************************************************************
MTD_STATUS mtdTuPTPSetCfgDisableTSpecCheck
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_U16 disableTSpecCheck
);

 Inputs:
    devPtr - pointer to MTD_DEV initialized by mtdLoadDriver() call
    port - MDIO port address, 0-31
    disableTSpecCheck -
        0 = enable checking for Transport Spec
        1 = disable checking for Transport Spec
 Outputs:
    None

 Returns:
    MTD_OK   - on success
    MTD_FAIL - on error

 Description:
    When disableTSpecCheck is set to 0, the Transport Spec part of the PTP Common header of
    incoming frames must match the configured TransSpec in order for time stamping
    to occur (PTP hardware accelerated or not). This allows time stamping to be limited
    to only IEEE 1588 or to only IEEE 802.1AS per their frame's Transport Specs
    (assuming their value is contained in the TransSpec register, which
    set by mtdTuPTPSetCfgTransSpec).
  
    When disableTSpecCheck is set to 1 the, the Transport Spec checking of the PTP frames
    is not performed before time stamping occurs.
    This setting allows PTP time stamping for all TransSpec values
    Noticed that PTP hardware acceleration only works on IEEE 1588 and IEEE 802.1AS
    TransSpec values.
  
 Side effects:
    None

 Notes/Warnings:
    None
*******************************************************************************/
MTD_STATUS mtdTuPTPSetCfgDisableTSpecCheck
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_U16 disableTSpecCheck
);


/*******************************************************************************
MTD_STATUS mtdTuPTPSetCfgDisTimeStmpCntOverwrite
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_U16 disTimeStmpCntOverwrite 
);

 Inputs:
    devPtr - pointer to MTD_DEV initialized by mtdLoadDriver() call
    port - MDIO port address, 0-31
    disTimeStmpCntOverwrite
        0 -Overwrite unread Time Stamps in the registers with new data
        1 -Keep unread Time Stamps in the registers until read
 Outputs:
    None

 Returns:
    MTD_OK   - on success
    MTD_FAIL - on error

 Description:
    Overwrite unread time stamps with new data.

    When this disTimeStmpCntOverwrite is set to 0, PTPArr0Time, PTPArr1Time and
    PTPDepTime get overwritten even though their corresponding status are not cleared.
    See mtdTuPTPGetTimeStamp() for other details.
  
 Side effects:
    None

 Notes/Warnings:
    None
*******************************************************************************/
MTD_STATUS mtdTuPTPSetCfgDisTimeStmpCntOverwrite
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_U16 disTimeStmpCntOverwrite 
);


/*******************************************************************************
MTD_STATUS mtdTuPTPSetCfgIPJump
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_U16 ipJump
);

  Inputs:
    devPtr - pointer to MTD_DEV initialized by mtdLoadDriver() call
    port - MDIO port address, 0-31
    ipJump - specifies how many bytes to skip starting at the first byte of the 
             frame's EtherType 

    Outputs:
    None

 Returns:
    MTD_OK   - on success
    MTD_FAIL - on error

 Description:
    Set IPJump to point to the start of the frame's IP Version byte (802.1Q tagged frames 
    are automatically compensated by ETJump).
  
    This field specifies how many bytes to skip starting at the first byte of the frame's EtherType 
    (i.e., where ETJump left off) in order to jump to the beginning of the IPv4 or IPv6 
    headers in the frame. If an IPv4 or IPv6 version is found at this location of the frame, 
    Layer 4 PTP processing occurs from there.
  
    This allows flexibility in the hardware to skip past the protocol chains that are specific to 
    customer networks including MPLS, etc.
  
    For example if ETJump is programmed to 0xC and IPJump is programmed to 0x16, this indicates to 
    hardware to skip 0x22 bytes in order to get to the IP header. It can either be IPv4 or IPv6 
    header.
  
 Side effects:
    None

 Notes/Warnings:
    A value of 0x0 (default) is a special case that prevents further frame searching if 
    ETJump did not find a match (i.e., a zero value in IPJump prevents the IPJump mechanism from 
    searching further).
*******************************************************************************/
MTD_STATUS mtdTuPTPSetCfgIPJump
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_U16 ipJump
);


/*******************************************************************************
MTD_STATUS mtdTuPTPSetCfgETJump
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_U16 etJump
);

 Inputs:
    devPtr - pointer to MTD_DEV initialized by mtdLoadDriver() call
    port - MDIO port address, 0-31
    etJump - specifies how many bytes to skip starting from the start of the MAC-DA 
             of the frame

    Outputs:
    None

 Returns:
    MTD_OK   - on success
    MTD_FAIL - on error

 Description:
    ETJump specifies how many bytes to skip starting from the start of the MAC-DA of the frame 
    in order to get to the first byte of the EtherType of the frame. Frames found with an 0x8100 
    EtherType (802.1Q tag) at this location are automatically searched 4 bytes further into the 
    frame for the next EtherType to compare (this extension is done once).

    This allows flexibility in the hardware to skip past the protocol chains that are specific to 
    customer networks including DSA-Tag, IEEE802.1Q tag, Provider tag etc.
  
 Side effects:
    None

 Notes/Warnings:
    None
*******************************************************************************/
MTD_STATUS mtdTuPTPSetCfgETJump
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_U16 etJump
);


/*******************************************************************************
MTD_STATUS mtdTuPTPSetCfgArrTSMode
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_U16 arrivalTSMode
);

 Inputs:
    devPtr - pointer to MTD_DEV initialized by mtdLoadDriver() call
    port - MDIO port address, 0-31
    arrivalTSMode - configures the Arrival Time Stamp mode
        On X3540 and other devices:
        0x00: Arrival Time Stamp frame modification is disabled.
        0x10: Overwrite the location of PTPArr0Time or PTPArr1Time time stamp insertion,
              place the time stamp from the 16th to 19th bytes in the PTP Common Header,
              without increasing the frame's size.

        Not applicable to X3540:
        0x01: Add the PTPArr0Time or PTPArr1Time time stamp at the end of the frame,
                      increasing the frame's size by 4 bytes.

        0x11 to 0xEF: same as value 0x10, overwrite the time stamp and place it at arrivalTSMode
                      bytes past the start of the PTP Common Header.

                      If the end of the frame is reached prior to the completion of
                      this overwrite, the time stamp will be placed at the end of the
                      frame and increase the frame's size by enough bytes for it to fit.

        Others: Reserved.

        This field must be set to 0 if External Hardware Acceleration(ExtHwAccel) is enabled,
        which can be configured by mtdTuPTPSetExtHwAccEnable()

    Outputs:
    None

 Returns:
    MTD_OK   - on success
    MTD_FAIL - on error

 Description:
    This field is used to configure the Arrival Time Stamp Mode as note in the arrivalTSMode
    input option.

    All frames will be processed using the above settings unless the frame
    Hardware Acceleration(HWAccel) is enabled, which can be configured by mtdTuPTPSetCfgHWAccel().

    Added bytes that increase the frame's size are included in the MIB counters and 
    policy is performed on the resulting frame (e.g., TCAM matching and frame size checking).

 Side effects:
    None

 Notes/Warnings:
    This API only works when a port is in the Disabled Port State (Port offset 0x04).
*******************************************************************************/
MTD_STATUS mtdTuPTPSetCfgArrTSMode
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_U16 arrivalTSMode
);


/*******************************************************************************
MTD_STATUS mtdTuPTPSetCfgHWAccel
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_U16 hwAccel
);

  Inputs:
    devPtr - pointer to MTD_DEV initialized by mtdLoadDriver() call
    port - MDIO port address, 0-31
    hwAccel - configure if both ingress and engress path use PTP hardware acceleration
        0 - Only Ingress PTP hardware acceleration
        1 - Ingress and Egress PTP hardware acceleration are both enabled

        Do not set this to 1 if External Hardware Acceleration(ExtHWAccel) is enabled.
        See mtdTuPTPSetExtHwAccEnable().

  Outputs:
    None

 Returns:
    MTD_OK   - on success
    MTD_FAIL - on error

 Description:
    This API configure the PTP hardware acceleration on the given port. PTP hardware 
    acceleration will automatically occur in the selected PTPMode(set by mtdTuPTPSetPTPMode)
    for the enabled PTP Domains once a Time Array is configured and enabled.
    Even then PTP hardware acceleration will only occur for the Transport Specs enabled
    by mtdTuPTPSetCfgTransSpec(). 

    When hwAccel is 1, any frame that cannot be hardware accelerated will be processed using
    the settings defined by Arrival Time Stamp Mode(ArrTSMode), also for the cases when hwAccel is 0.

    Notes: PTP Hardware Acceleration requires that the Time Stamping Clock Period (TAI offset 0x01, 3.D941)
    be 8000 pico seconds (8ns) +/- 100 PPM. If the PTP_EXTCLK (external clock) is used 
    (TAI offset 0x1E, 3.D95E) it must also be within this range before enabling PTP Hardware Acceleration.
 
 Side effects:
    None

 Notes/Warnings:
    This API only works when a port is in the Disabled Port State (Port offset 0x04).
*******************************************************************************/
MTD_STATUS mtdTuPTPSetCfgHWAccel
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_U16 hwAccel
);


/*******************************************************************************
MTD_STATUS mtdTuPTPSetKeepSourceAddr
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_U16 keepSA
);

  Inputs:
    devPtr - pointer to MTD_DEV initialized by mtdLoadDriver() call
    port - MDIO port address, 0-31
    keepSA - configure 
        0 - Place Port's SA into modified egressing PTP frames
        1 - Keep the frame's SA even for modified egressing PTP frames

  Outputs:
    None

 Returns:
    MTD_OK   - on success
    MTD_FAIL - on error

 Description:
    If keepSA is set as 0, the address of the modifying entity is placed into
    the Source Address (SA) field of egressing frames when the Data portion of
    a frame (the part of the frame between the EtherType and the CRC) is modified.

    The SA used in the frame is determined by the Switch MAC registers (Global 2 offset 0x0D).

    If keepSA is set as 1, the SA portion of PTP frames is not modified.
 
 Side effects:
    None

 Notes/Warnings:
    None
*******************************************************************************/
MTD_STATUS mtdTuPTPSetKeepSourceAddr
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_U16 keepSA
);


/*******************************************************************************
MTD_STATUS mtdTuPTPSetExtHwAccEnable
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_U16 extHwAccEnable
);

  Inputs:
    devPtr - pointer to MTD_DEV initialized by mtdLoadDriver() call
    port - MDIO port address, 0-31
    extHwAccEnable - enable External Hardware Acceleration
        0 - Port has no External PHY or its PTP hardware acceleration is off
        1 - Port's External PHY is enabled with PTP hardware acceleration

        Do not set this to 1 if PTP Hardware Acceleration(HWAccel) is enabled, or if the
        ArrTSMode is set to an non-zero value.
        See mtdTuPTPSetCfgHWAccel() and mtdTuPTPSetCfgArrTSMode().

  Outputs:
    None

 Returns:
    MTD_OK   - on success
    MTD_FAIL - on error

 Description:
  Set extHwAccEnable to 1 enables PTP hardware accelerated frames to be mapped to this 
  port and then to the External PHY's hardware acceleration logic. No time stamping or frame
  modification is done on the switch's side of this port as it is expected to be done in the 
  External PHY in this case.

  Set extHwAccEnable to 1 0 prevents the mapping of Hardware Accelerated PTP frames to this
  port.
 
 Side effects:
    None

 Notes/Warnings:
    This API only works when a port is in the Disabled Port State (Port offset 0x04).
*******************************************************************************/
MTD_STATUS mtdTuPTPSetExtHwAccEnable
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_U16 extHwAccEnable
);

/*******************************************************************************
MTD_STATUS mtdTuPTPSetDepartureIntEn
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_U16 departureIntEn
);

  Inputs:
    devPtr - pointer to MTD_DEV initialized by mtdLoadDriver() call
    port - MDIO port address, 0-31
    departureIntEn - one of the following
        0 - Disable PTP Departure capture interrupts
        1 - Enable PTP Departure capture interrupts

  Outputs:
    None

 Returns:
    MTD_OK   - on success
    MTD_FAIL - on error

 Description:
    This API enables the per-port interrupt for outgoing PTP frame from this port.

    The PTP interrupt status can be read by calling mtdTuPTPGetIntStatus() if the departure
    time stamp is valid.

    Note that hardware logic only time stamps the PTP frames when configured to do
    so, see mtdTuPTPSetMsgTSEnable().
 
 Side effects:
    None

 Notes/Warnings:
    None
*******************************************************************************/
MTD_STATUS mtdTuPTPSetDepartureIntEn
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_U16 departureIntEn
);


/*******************************************************************************
MTD_STATUS mtdTuPTPSetArrivalIntEn
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_U16 arrivalIntEn
);

  Inputs:
    devPtr - pointer to MTD_DEV initialized by mtdLoadDriver() call
    port - MDIO port address, 0-31
    arrivalIntEn - one of the following
        0 - Disable PTP Arrival capture interrupts
        1 - Enable PTP Arrival capture interrupts

  Outputs:
    None

 Returns:
    MTD_OK   - on success
    MTD_FAIL - on error

 Description:
    This field enabled the per-port interrupt for incoming PTP frames from this port.

    The PTP interrupt status can be read by calling mtdTuPTPGetIntStatus() if the departure
    time stamp is valid.

    Note that hardware logic only time stamps the PTP frames when configured to do
    so, see mtdTuPTPSetMsgTSEnable().
 
 Side effects:
    None

 Notes/Warnings:
    None
*******************************************************************************/
MTD_STATUS mtdTuPTPSetArrivalIntEn
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_U16 arrivalIntEn
);


/*******************************************************************************
MTD_STATUS mtdTuPTPGetPortConfig
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    OUT MTD_TU_PTP_PORT_CONFIG *ptpData
);

 Inputs:
    devPtr - pointer to MTD_DEV initialized by mtdLoadDriver() call
    port - MDIO port address, 0-31

 Outputs:
      ptpData - PTP port configuration parameters.

 Returns:
    MTD_OK   - on success
    MTD_FAIL - on error

 Description:
    This routine gets PTP port configuration in the MTD_TU_PTP_PORT_CONFIG structure.
  
    MTD_TU_PTP_PORT_CONFIG: 
     disablePTP     - enable/disable PTP port; 0=enable; 1=disable 
     transSpec      - This is to differentiate between various timing protocols.
     disTSpec       - Disable Transport specific check
     etJump         - offset to Ether type start address in bytes
     ipJump         - offset to IP header start address counting from Ether type offset
     ptpArrIntEn    - PTP port arrival interrupt enable
     ptpDepIntEn    - PTP port departure interrupt enable
     disTSOverwrite - disable time stamp counter overwriting until the corresponding
                      timer valid bit is cleared.
     filterAct      - Filter 802.1 Act from LEDs
     hwAccPtp       - Hardware Accelerated PTP
     keepFmSA      - KS = Keep Frame source addr
     ExthwAcc       - External Hardware Acceleration enable
     arrTSMode      - Arrival Time Stamp mode
     arrLedCtrl     - LED control for packets entering the device.
     depLedCtrl     - LED control for packets departing the device.

 Notes/Warnings:
    Call mtdTuPTPGetPortPTPState() if only the PTP state is needed. 
*******************************************************************************/
MTD_STATUS mtdTuPTPGetPortConfig
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    OUT MTD_TU_PTP_PORT_CONFIG *ptpData
);


/*******************************************************************************
MTD_STATUS mtdTuPTPGetTimeStamp
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_TU_PTP_TIME timeToRead,
    OUT MTD_TU_PTP_TS_STATUS *ptpStatus
);

 Inputs:
    devPtr - pointer to MTD_DEV initialized by mtdLoadDriver() call
    port - MDIO port address, 0-31
    timeToRead - option to read the PTP arrival0, arrival1 and Departure time
        MTD_TU_PTP_ARR0_TIME = 0x0       - PTPArr0Time
        MTD_TU_PTP_ARR1_TIME = 0x1       - PTPArr1Time
        MTD_TU_PTP_DEP_TIME = 0x2        - PTPDepTime


 Outputs:
    ptpStatus    - PTP port status

 Returns:
    MTD_OK   - on success
    MTD_FAIL - on error

 Description:
    This routine retrieves the PTP port status that includes time stamp value
    and sequence Id that are captured by PTP logic for a PTP frame that needs
    to be time stamped.

 Side effects:
    None

 Notes/Warnings:
    None
*******************************************************************************/
MTD_STATUS mtdTuPTPGetTimeStamp
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_TU_PTP_TIME timeToRead,
    OUT MTD_TU_PTP_TS_STATUS *ptpStatus
);

/*******************************************************************************
MTD_STATUS mtdTuGetPTPTSValidSt
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_TU_PTP_TIME timeToRead,
    OUT MTD_BOOL *isValid
);

 Inputs:
    devPtr - pointer to MTD_DEV initialized by mtdLoadDriver() call
    port - MDIO port address, 0-31
    timeToRead - selection for the PTP arrival0, arrival1 and Departure time
        MTD_TU_PTP_ARR0_TIME = 0x0       - PTPArr0Time
        MTD_TU_PTP_ARR1_TIME = 0x1       - PTPArr1Time
        MTD_TU_PTP_DEP_TIME = 0x2        - PTPDepTime

 Outputs:
    isValid - MTD_TRUE: valid TS, MTD_FALSE: invalid TS.

 Returns:
    MTD_OK   - on success
    MTD_FAIL - on error

 Description:
       This routine gets PTP Time valid bit.

 Side effects:
    None

 Notes/Warnings:
    None
*******************************************************************************/
MTD_STATUS mtdTuGetPTPTSValidSt
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_TU_PTP_TIME timeToRead,
    OUT MTD_BOOL *isValid
);


/*******************************************************************************
MTD_STATUS mtdTuPTPTimeStampReset
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_TU_PTP_TIME timeToReset
);

 Inputs:
    devPtr - pointer to MTD_DEV initialized by mtdLoadDriver() call
    port - MDIO port address, 0-31
    timeToReset    - selection for the PTP arrival0, arrival1 and Departure time
        MTD_TU_PTP_ARR0_TIME = 0x0       - PTPArr0Time
        MTD_TU_PTP_ARR1_TIME = 0x1       - PTPArr1Time
        MTD_TU_PTP_DEP_TIME = 0x2        - PTPDepTime

 Outputs:
       None.

 Returns:
    MTD_OK   - on success
    MTD_FAIL - on error

 Description:
       This routine resets PTP Time valid bit so that PTP logic can time stamp
       a next PTP frame that needs to be time stamped.

 Side effects:
    None

 Notes/Warnings:
    None
*******************************************************************************/
MTD_STATUS mtdTuPTPTimeStampReset
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_TU_PTP_TIME timeToReset
);

/*******************************************************************************
MTD_STATUS mtdTuPTPSetTimeStampArrivalPtr
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_U16 tsArrPtr
);

 Inputs:
    devPtr - pointer to MTD_DEV initialized by mtdLoadDriver() call
    port - MDIO port address, 0-31
    tsArrPtr  - Time stamp arrival time counter pointer (either Arr0Time or Arr1Time)

 Outputs:
   None.

 Returns:
    MTD_OK   - on success
    MTD_FAIL - on error

 Description:
    The API sets the Time Stamp Arrival Time Capture Pointer.
    If the incoming PTP frame needs to be time stamped (see mtdTuPTPSetMsgTSEnable),
    this field determines whether the hardware logic should use PTPArr0Time or PTPArr1Time
    for storing the arriving frames' time stamp information.

    This field corresponds to the sixteen combinations of the vectorized MessageType.
    For example if bit 2 of tsArrPtr is set to 1 it indicates to the hardware that if
    bit 2 of msgTSEnable is set to 1 then PTP frames with MessageType = 0x2 will use
    PTPArr1Time counter for storing the incoming PTP frames' time stamp.

    On the contrary if bit 2 of tsArrPtr is cleared to 0, it indicates that PTP frames
    with MessageType = 0x2 will use PTPArr0Time counter for storing the incoming
    PTP frames' time stamp.

 Side effects:
    None

 Notes/Warnings:
    None
*******************************************************************************/
MTD_STATUS mtdTuPTPSetTimeStampArrivalPtr
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_U16 tsArrPtr
);

/*******************************************************************************
MTD_STATUS mtdTuPTPGetTimeStampArrivalPtr
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    OUT MTD_U16 *tsArrPtr
);

 Inputs:
    devPtr - pointer to MTD_DEV initialized by mtdLoadDriver() call
    port - MDIO port address, 0-31

 Outputs:
    tsArrPtr  - Time stamp arrival time counter pointer (either Arr0Time or Arr1Time)

 Returns:
    MTD_OK   - on success
    MTD_FAIL - on error

 Description:
    The API gets the Time Stamp Arrival Time Capture Pointer.
   
    If the incoming PTP frame needs to be time stamped (see mtdTuPTPSetMsgTSEnable),
    this field determines whether the hardware logic should use PTPArr0Time or PTPArr1Time
    for storing the arriving frames' time stamp information.

    This field corresponds to the sixteen combinations of the vectorized MessageType.
    For example if bit 2 of tsArrPtr is set to 1 it indicates to the hardware that if
    bit 2 of msgTSEnable is set to 1 then PTP frames with MessageType = 0x2 will use
    PTPArr1Time counter for storing the incoming PTP frames' time stamp.

    On the contrary if bit 2 of tsArrPtr is cleared to 0, it indicates that PTP frames
    with MessageType = 0x2 will use PTPArr0Time counter for storing the incoming
    PTP frames' time stamp.

 Side effects:
    None

 Notes/Warnings:
    None
*******************************************************************************/
MTD_STATUS mtdTuPTPGetTimeStampArrivalPtr
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    OUT MTD_U16 *tsArrPtr
);


/*******************************************************************************
MTD_STATUS mtdTuPTPGetIntStatus
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    OUT MTD_TU_PTP_INTERRUPT_STATUS *ptpIntSt
);

 Inputs:
    devPtr - pointer to MTD_DEV initialized by mtdLoadDriver() call
    port - MDIO port address, 0-31

 Outputs:
    ptpIntSt - interrupt status for each port (bit 0 for port 0, bit 1 for port 1, etc.)
       ptpTrigGenInt         - Trigger generate mode Interrupt
       ptpEventInt           - Event Capture Interrupt
       ptpUpperPortInt       - Upper port Interrupt
       ptpIntStLowerPortVect - Precise Time Protocol Interrupt for Ports 0 - 10.
       ptpIntStUpperPortVect - Precise Time Protocol Interrupt for Ports 11 - 26.

 Returns:
    MTD_OK   - on success
    MTD_FAIL - on error

 Description:
    This routine gets PTP interrupt status for a port.

 Side effects:
    None

 Notes/Warnings:
    None
*******************************************************************************/
MTD_STATUS mtdTuPTPGetIntStatus
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    OUT MTD_TU_PTP_INTERRUPT_STATUS *ptpIntSt
);


/*******************************************************************************
MTD_STATUS mtdTuPTPSetIgrMeanPathDelay
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_U16 delay
);

 Inputs:
    devPtr - pointer to MTD_DEV initialized by mtdLoadDriver() call
    port - MDIO port address, 0-31
    delay - cable delay between this port and its link partner in unsigned
            nano seconds

 Outputs:
       None.

 Returns:
    MTD_OK   - on success
    MTD_FAIL - on error

 Description:
    This routine sets ingress mean path delay on a port.

 Side effects:
    None

 Notes/Warnings:
    None
*******************************************************************************/
MTD_STATUS mtdTuPTPSetIgrMeanPathDelay
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_U16 delay
);


/*******************************************************************************
MTD_STATUS mtdTuPTPGetIgrMeanPathDelay
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    OUT MTD_U16 *delay
);

 Inputs:
    devPtr - pointer to MTD_DEV initialized by mtdLoadDriver() call
    port - MDIO port address, 0-31

 Outputs:
    delay - cable delay between this port and its link partner in unsigned
            nano seconds

 Returns:
    MTD_OK   - on success
    MTD_FAIL - on error

 Description:
    This routine gets ingress mean path delay on a port.

 Side effects:
    None

 Notes/Warnings:
    None
*******************************************************************************/
MTD_STATUS mtdTuPTPGetIgrMeanPathDelay
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    OUT MTD_U16 *delay
);


/*******************************************************************************
MTD_STATUS mtdTuPTPSetIgrPathDelayAsym
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_U16 asym
);

 Inputs:
    devPtr - pointer to MTD_DEV initialized by mtdLoadDriver() call
    port - MDIO port address, 0-31
    asym - configure the delay asymmetry
        bit 15 is path delay asymmetry sign:
            0 - the path delay asymmetry number is added
            1 - the path delay asymmetry number is subtracted

        bit 14:0 indicates the asymmetry value beyond the mean path delay
        that needs to be added or subtracted for more accurate cable
        measurements.

 Outputs:
    None.

 Returns:
    MTD_OK   - on success
    MTD_FAIL - on error

 Description:
    This routine sets ingress path delay asymmetry on a port.

 Side effects:
    None

 Notes/Warnings:
    None
*******************************************************************************/
MTD_STATUS mtdTuPTPSetIgrPathDelayAsym
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_U16 asym
);


/*******************************************************************************
MTD_STATUS mtdTuPTPGetIgrPathDelayAsym
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    OUT MTD_U16 *asym
);

 Inputs:
    devPtr - pointer to MTD_DEV initialized by mtdLoadDriver() call
    port - MDIO port address, 0-31

 Outputs:
    asym - configure the delay asymmetry
        bit 15 is path delay asymmetry sign:
            0 - the path delay asymmetry number is added
            1 - the path delay asymmetry number is subtracted

        bit 14:0 indicates the asymmetry value beyond the mean path delay
        that needs to be added or subtracted for more accurate cable
        measurements.

 Returns:
    MTD_OK   - on success
    MTD_FAIL - on error

 Description:
    This routine gets ingress path delay asymmetry on a port.

 Side effects:
    None

 Notes/Warnings:
    None
*******************************************************************************/
MTD_STATUS mtdTuPTPGetIgrPathDelayAsym
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    OUT MTD_U16 *asym
);


/*******************************************************************************
MTD_STATUS mtdTuPTPEgrPathDelayAsymSet
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_U16 asym
);

 Inputs:
    devPtr - pointer to MTD_DEV initialized by mtdLoadDriver() call
    port - MDIO port address, 0-31
    asym - configure the delay asymmetry
        bit 15 is path delay asymmetry sign:
            0 - the path delay asymmetry number is added
            1 - the path delay asymmetry number is subtracted

        bit 14:0 indicates the asymmetry value beyond the mean path delay
        that needs to be added or subtracted for more accurate cable
        measurements.

 Outputs:
       None.
 Returns:
    MTD_OK   - on success
    MTD_FAIL - on error

 Description:
       This routine sets egress path delay asymmetry on a port.

 Side effects:
    None

 Notes/Warnings:
    Call mtdTuPTPSetPortPTPState() to enable/disable PTP port. 
*******************************************************************************/
MTD_STATUS mtdTuPTPEgrPathDelayAsymSet
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_U16 asym
);


/*******************************************************************************
MTD_STATUS mtdTuPTPEgrPathDelayAsymGet
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    OUT MTD_U16 *asym
);

 Inputs:
    devPtr - pointer to MTD_DEV initialized by mtdLoadDriver() call
    port - MDIO port address, 0-31

 Outputs:
    asym - configure the delay asymmetry
        bit 15 is path delay asymmetry sign:
            0 - the path delay asymmetry number is added
            1 - the path delay asymmetry number is subtracted

        bit 14:0 indicates the asymmetry value beyond the mean path delay
        that needs to be added or subtracted for more accurate cable
        measurements.

 Returns:
    MTD_OK   - on success
    MTD_FAIL - on error

 Description:
       This routine gets egress path delay asymmetry on a port.

 Side effects:
    None

 Notes/Warnings:
    None
*******************************************************************************/
MTD_STATUS mtdTuPTPEgrPathDelayAsymGet
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    OUT MTD_U16 *asym
);


/*******************************************************************************
MTD_STATUS mtdTuPTPGetTODBusyBitStatus
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    OUT MTD_BOOL *busyStatus
);

 Inputs:
    devPtr - pointer to MTD_DEV initialized by mtdLoadDriver() call
    port - MDIO port address, 0-31

 Outputs:
    busyStatus - one of the following
       MTD_TRUE - Hardware is busy, should wait. 
       MTD_FALSE - busy bit has been self clear, can do other operation

 Returns:
    MTD_OK   - on success
    MTD_FAIL - on error

 Description:
    Check the busy bit to see if the hardware is busy. Make sure the busy status is 
    False before any other ToD operation

 Side effects:
    None

 Notes/Warnings:
    None
*******************************************************************************/
MTD_STATUS mtdTuPTPGetTODBusyBitStatus
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    OUT MTD_BOOL *busyStatus
);


/*******************************************************************************
MTD_STATUS mtdTuPTPTODStoreOperation
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_TU_PTP_TOD_STORE_OPERATION storeOp,
    IN MTD_U8 timeArrayIndex,
    IN MTD_TU_PTP_TIME_ARRAY *timeArray
);

 Inputs:
    devPtr - pointer to MTD_DEV initialized by mtdLoadDriver() call
    port - MDIO port address, 0-31
    timeArrayIndex - Time array index
    storeOp - MTD_TU_PTP_TOD_STORE_COMP: Store Compensation registers to selected TimeArray
              MTD_TU_PTP_TOD_STORE_ALL: Store all registers to selected TimeArray
    timeArray -    TOD load pointer
                   10 byte ToD time,
                   8 byte 1722 time,
                   4 byte Compensation,
                   1 byte Domain Number,
                   And a 1 bit Clock Valid.

 Outputs:
    None.

 Returns:
    MTD_OK   - on success
    MTD_FAIL - on error

 Description:
    This API Time of Day(ToD) Store All Registers operation is used to determine the instant 
    in time that the selected Time Array is loaded. The load occurs at the instant the 
    PTP Global Time (TAI Global offset 0x0E & 0x0F) matches the contents of this register.
 
 Side effects:
    None

 Notes/Warnings:
    None
*******************************************************************************/
MTD_STATUS mtdTuPTPTODStoreOperation
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_TU_PTP_TOD_STORE_OPERATION storeOp,
    IN MTD_U8 timeArrayIndex,
    IN MTD_TU_PTP_TIME_ARRAY *timeArray
);


/*******************************************************************************
MTD_STATUS mtdTuPTPTODCaptureAll
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_U8 timeArrayIndex,
    OUT MTD_TU_PTP_TIME_ARRAY *timeArray
);

 Inputs:
    devPtr - pointer to MTD_DEV initialized by mtdLoadDriver() call
    port - MDIO port address, 0-31
    timeArrayIndex - Time array index

 Outputs:
    timeArray -    TOD load pointer
                   10 byte ToD time,
                   8 byte 1722 time,
                   4 byte Compensation,
                   1 byte Domain Number,
                   And a 1 bit Clock Valid.

 Returns:
    MTD_OK   - on success
    MTD_FAIL - on error

 Description:
    This API Time of Day(ToD)Capture All is used to capture the instant in 
    time that the Capture occurred. On each ToD Capture, the contents of 
    this register will be loaded with the current value contained in the 
    PTP Global Time (TAI Global offsets 0x0E & 0x0F).

 Side effects:
    None

 Notes/Warnings:
    None
*******************************************************************************/
MTD_STATUS mtdTuPTPTODCaptureAll
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_U8 timeArrayIndex,
    OUT MTD_TU_PTP_TIME_ARRAY *timeArray
);


/*******************************************************************************
MTD_STATUS mtdTuPTPSetPulse
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_TU_PTP_PULSE_STRUCT *pulsePara
);

 Inputs:
    devPtr - pointer to MTD_DEV initialized by mtdLoadDriver() call
    port - MDIO port address, 0-31
    pulsePara - following parameters:
        ptpPulseWidth;
        ptpAltTCAMTimeSel;
        ptp1ppsWidthRange;
        ptpTCAMTimeSel
        ptp1ppsPhase;
        ptp1ppsSelect;

 Outputs:
    None.

 Returns:
    MTD_OK   - on success
    MTD_FAIL - on error

 Description:
    Set all the PTP Pulse fields in the MTD_TU_PTP_PULSE_STRUCT. Refer to the 
    device datasheet on each of these fields.  The following fields are defined 
    in the MTD_TU_PTP_PULSE_STRUCT structure:
        ptpPulseWidth;
        ptpAltTCAMTimeSel;
        ptp1ppsWidthRange;
        ptpTCAMTimeSel
        ptp1ppsPhase;
        ptp1ppsSelect;

 Side effects:
    None

 Notes/Warnings:
    None
*******************************************************************************/
MTD_STATUS mtdTuPTPSetPulse
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_TU_PTP_PULSE_STRUCT *pulsePara
);


/*******************************************************************************
MTD_STATUS mtdTuPTPGetPulse
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    OUT MTD_TU_PTP_PULSE_STRUCT *pulsePara
);

 Inputs:
    devPtr - pointer to MTD_DEV initialized by mtdLoadDriver() call
    port - MDIO port address, 0-31

 Outputs:
    pulsePara - following parameters:
        ptpPulseWidth;
        ptpAltTCAMTimeSel;
        ptp1ppsWidthRange;
        ptpTCAMTimeSel;
        ptp1ppsPhase;
        ptp1ppsSelect;

 Returns:
    MTD_OK   - on success
    MTD_FAIL - on error

 Description:
    Retrieves all the PTP Pulse fields in the MTD_TU_PTP_PULSE_STRUCT. Refer to the 
    device datasheet on each of these fields.  The following fields are defined 
    in the MTD_TU_PTP_PULSE_STRUCT structure:
      ptpPulseWidth;
      ptpAltTCAMTimeSel;
      ptp1ppsWidthRange;
      ptpTCAMTimeSel
      ptp1ppsPhase;
      ptp1ppsSelect;

 Side effects:
    None

 Notes/Warnings:
    None
*******************************************************************************/
MTD_STATUS mtdTuPTPGetPulse
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    OUT MTD_TU_PTP_PULSE_STRUCT *pulsePara
);


/*******************************************************************************
MTD_STATUS mtdTuPTPGetPTPGlobalTime
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    OUT MTD_U32 *ptpGlobalTime
);

 Inputs:
    devPtr - pointer to MTD_DEV initialized by mtdLoadDriver() call
    port - MDIO port address, 0-31

 Outputs:
    ptpTime    - PTP global time

 Returns:
    MTD_OK   - on success
    MTD_FAIL - on error

 Description:
    This routine gets the global timer value that is running off of the free
    running clock. The global timer value that is running off of the free running 
    switch core clock. This counter wraps around in hardware.

    To support synchronization of PTP Global Time between multiple devices in a 
    system, this register gets updated with the value specified in TrigGenAmt 
    when MultiPTPSync is set to a one (3.D940.2, see mtdTuPTPGetTAIGlobalCfg) and an
    EventReq occurs (see EventPhase at 3.D940.13).
 
 Side effects:
    None

 Notes/Warnings:
    None
*******************************************************************************/
MTD_STATUS mtdTuPTPGetPTPGlobalTime
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    OUT MTD_U32 *ptpGlobalTime
);


/*******************************************************************************
MTD_STATUS mtdTuPTPEventCaptureTime
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    OUT MTD_U32 *evtCaptureTime
);

 Inputs:
    devPtr - pointer to MTD_DEV initialized by mtdLoadDriver() call
    port - MDIO port address, 0-31

 Outputs:
    evtCaptureTime - PTP Event Capture Time

 Returns:
    MTD_OK   - on success
    MTD_FAIL - on error

 Description:
    The Event Capture Time captures the value of the PTP Global Timer 
    (TAI offsets 0x0E & 0x0F) when an EventReq (see EventPhase, 3.D940.13) has occurred.

    If the EventCapOv (3.D940.15, see mtdTuPTPGetTAIGlobalCfg) is set to 1, then evtCaptureTime
    indicates the time captured for the last event. When EventCapErr(TAI offset 0x09, 3.D949) is set 
    to 1, the evtCaptureTime no longer represent the time of the first event.

    Note that the maximum jitter for the EventCapRegister time amount with respect 
    to the EventReq on a GPIO pin is one TSClkPer amount.

    Note that the minimum EventReq GPIO input signal high or low width has to be 
    equal to or greater than 1.5 times the TSClkPer amount.

    Note that in order for hardware to capture the EventReq on the GPIO input signal, 
    the minimum gap between two consecutive events has to be 150 plus 5 times the 
    TSClkPer amount.

 Side effects:
    None

 Notes/Warnings:
    None
*******************************************************************************/
MTD_STATUS mtdTuPTPEventCaptureTime
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    OUT MTD_U32 *evtCaptureTime
);

/*******************************************************************************
MTD_STATUS mtdTuPTPGetEventCaptureCounter
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    OUT MTD_U16 *eventCaptureCounter
);

 Inputs:
    devPtr - pointer to MTD_DEV initialized by mtdLoadDriver() call
    port - MDIO port address, 0-31

 Outputs:
    eventCaptureCounter - PTP event capture counter

 Returns:
    MTD_OK   - on success
    MTD_FAIL - on error

 Description:
    This API reads the PTP event capture counter. This counter is incremented once by each 
    EventReq (see EventPhase in PTP TAI offset 0x00) as long as EventCtrStart (PTP TAI offset 0x00) 
    is set to one. This counter wraps around and can be cleared by calling
    mtdTuPTPClearEventCaptureCounter API

 Side effects:
    None

 Notes/Warnings:
    None
*******************************************************************************/
MTD_STATUS mtdTuPTPGetEventCaptureCounter
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    OUT MTD_U16 *eventCaptureCounter
);

/*******************************************************************************
MTD_STATUS mtdTuPTPClearEventCaptureCounter
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port
);

 Inputs:
    devPtr - pointer to MTD_DEV initialized by mtdLoadDriver() call
    port - MDIO port address, 0-31

 Outputs:
    None

 Returns:
    MTD_OK   - on success
    MTD_FAIL - on error

 Description:
    This API clears the event capture counter. This counter wraps around and can be 
    cleared by writing zeros to it.

 Side effects:
    None

 Notes/Warnings:
    None
*******************************************************************************/
MTD_STATUS mtdTuPTPClearEventCaptureCounter
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port
);

/*******************************************************************************
MTD_STATUS mtdTuPTPGetEventCaptureValid
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    OUT MTD_U16 *eventCaptureValid
);

 Inputs:
    devPtr - pointer to MTD_DEV initialized by mtdLoadDriver() call
    port - MDIO port address, 0-31

 Outputs:
    eventCaptureValid - Event Capture Valid flag bit

 Returns:
    MTD_OK   - on success
    MTD_FAIL - on error

 Description:
     This API read the Event Capture Valid flag. This flag bit is set to a one whenever 
     the EventCap (Event Capture - TAI offsets 0x0A & 0x0B) register contains the time of 
     a captured event. Software needs to clear this bit to a zero to enable the EventCap 
     Register to be able to acquire a subsequent event if the EventCapOv (Event Capture 
     Override - TAI offset 0x00) is not enabled. Clearing this bit to a zero also clears 
     the EventInt (Event Capture Interrupt - PTP Global offset 0x08).

 Side effects:
    None

 Notes/Warnings:
    None
*******************************************************************************/
MTD_STATUS mtdTuPTPGetEventCaptureValid
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    OUT MTD_U16 *eventCaptureValid
);

/*******************************************************************************
MTD_STATUS mtdTuPTPGetEventCaptureErr
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    OUT MTD_U16 *eventCaptureErr
);

 Inputs:
    devPtr - pointer to MTD_DEV initialized by mtdLoadDriver() call
    port - MDIO port address, 0-31

 Outputs:
    eventCaptureErr - Event Capture Error flag bit

 Returns:
    MTD_OK   - on success
    MTD_FAIL - on error

 Description:
     This API read the Event Capture Error bit. This bit is set by the hardware logic when 
     an EventReq has occurred (see EventPhase in TAI offset 0x00) where the EventCapValid 
     bit, below, is already set to a one and the EventCapOv bit (TAI offset 0x00) is cleared 
     to a zero. This condition could happen if the EventReqs are occurring faster than 
     the local CPU can process them (and clear the EventCapValid bit before the next
     EventReq).

 Side effects:
    None

 Notes/Warnings:
    None
*******************************************************************************/
MTD_STATUS mtdTuPTPGetEventCaptureErr
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    OUT MTD_U16 *eventCaptureErr
);

/*******************************************************************************
MTD_STATUS mtdTuPTPClearEventCaptureErr
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port
);

 Inputs:
    devPtr - pointer to MTD_DEV initialized by mtdLoadDriver() call
    port - MDIO port address, 0-31

 Outputs:
    None

 Returns:
    MTD_OK   - on success
    MTD_FAIL - on error

 Description:
     This API clears the Event Capture Error bit. This bit is set by the hardware logic when 
     an EventReq has occurred (see EventPhase in TAI offset 0x00) where the EventCapValid 
     bit, below, is already set to a one and the EventCapOv bit (TAI offset 0x00) is cleared 
     to a zero. This condition could happen if the EventReqs are occurring faster than 
     the local CPU can process them (and clear the EventCapValid bit before the next
     EventReq).

 Side effects:
    None

 Notes/Warnings:
    None
*******************************************************************************/
MTD_STATUS mtdTuPTPClearEventCaptureErr
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port
);

/*******************************************************************************
MTD_STATUS mtdTuPTPSetEventCaptureTrigger
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_U16 eventCaptureTrigger
);

 Inputs:
    devPtr - pointer to MTD_DEV initialized by mtdLoadDriver() call
    port - MDIO port address, 0-31
    eventCaptureTrigger - Capture Event Trigger
                             0 = Capture PTP_EVREQ pin events
                             1 = Capture PTP_TRIG internal events

 Outputs:
    None

 Returns:
    MTD_OK   - on success
    MTD_FAIL - on error

 Description:
     This API sets the Capture Event Trigger option. When this bit is cleared to a 
     zero the Event Capture register looks at events on the PTP_EVREQ pin. When this 
     bit is set to a one the Event Capture register looks at events from the waveform 
     generated by PTP_TRIG. This allows observing the rising or falling edge of the 
     PTP_TRIG (the EventPhase register is still active, PTP TAI offset 0x00) without 
     the need of using pins. This is used to insure the edges have not drifted over 
     time so they can be re-aligned if needed.

 Side effects:
    None

 Notes/Warnings:
    None
*******************************************************************************/
MTD_STATUS mtdTuPTPSetEventCaptureTrigger
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_U16 eventCaptureTrigger
);

/*******************************************************************************
MTD_STATUS mtdTuPTPGetEventCaptureTrigger
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    OUT MTD_U16 *eventCaptureTrigger
);

 Inputs:
    devPtr - pointer to MTD_DEV initialized by mtdLoadDriver() call
    port - MDIO port address, 0-31
    eventCaptureTrigger - Capture Event Trigger bit
                             0 = Capture PTP_EVREQ pin events
                             1 = Capture PTP_TRIG internal events

 Outputs:

 Returns:
    MTD_OK   - on success
    MTD_FAIL - on error

 Description:
     This API read the Capture Event Trigger bit.  When this bit is cleared to a 
     zero the Event Capture register looks at events on the PTP_EVREQ pin. When 
     this bit is set to a one the Event Capture register looks at events from the 
     waveform generated by PTP_TRIG. This allows observing the rising or falling 
     edge of the PTP_TRIG (the EventPhase register is still active, PTP TAI 
     offset 0x00) without the need of using pins. This is used to insure the 
     edges have not drifted over time so they can be re-aligned if needed.

 Side effects:
    None

 Notes/Warnings:
    None
*******************************************************************************/
MTD_STATUS mtdTuPTPGetEventCaptureTrigger
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    OUT MTD_U16 *eventCaptureTrigger
);

/*******************************************************************************
MTD_STATUS mtdTuPTPSetDriftComp
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_U16 enableDriftComp
);

 Inputs:
    devPtr - pointer to MTD_DEV initialized by mtdLoadDriver() call
    port - MDIO port address, 0-31
    enableDriftComp  - Enable Drift Comp bit - MTD_ENABLE or MTD_DISABLE

 Outputs:
    None

 Returns:
    MTD_OK   - on success
    MTD_FAIL - on error

 Description:
    Enable Drift Comp bit enables frequency drift compensation function for 
    Time of Day (TOD)

 Side effects:
    None

 Notes/Warnings:
    None
*******************************************************************************/
MTD_STATUS mtdTuPTPSetDriftComp
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_U16 enableDriftComp
);

/*******************************************************************************
MTD_STATUS mtdTuPTPGetDriftComp
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    OUT MTD_U16 *enableDriftComp  -  MTD_ENABLE or MTD_DISABLE 
);

 Inputs:
    devPtr - pointer to MTD_DEV initialized by mtdLoadDriver() call
    port - MDIO port address, 0-31

 Outputs:
    enableDriftComp  - Enable Drift Comp bit - MTD_ENABLE or MTD_DISABLE

 Returns:
    MTD_OK   - on success
    MTD_FAIL - on error

 Description:
    Get the enable Drift Comp bit that enables frequency drift compensation 
    function for Time of Day (TOD)

 Side effects:
    None

 Notes/Warnings:
    None
*******************************************************************************/
MTD_STATUS mtdTuPTPGetDriftComp
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    OUT MTD_U16 *enableDriftComp  /* MTD_ENABLE or MTD_DISABLE */
);


/*******************************************************************************
MTD_STATUS mtdTuPTPSetTodClkPeriod
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_U16 todClkPeriod 
);

 Inputs:
    devPtr - pointer to MTD_DEV initialized by mtdLoadDriver() call
    port - MDIO port address, 0-31
    todClkPeriod  - Clock_Period 
        0x0 - 125 ms
        0x1 - 250 ms
        0x2 - 500 ms
        0x3 - 1.0 sec

 Outputs:
    None

 Returns:
    MTD_OK   - on success
    MTD_FAIL - on error

 Description:
    TOD Clock Period bit determines the period of TOD alignment clock 

 Side effects:
    None

 Notes/Warnings:
    None
*******************************************************************************/
MTD_STATUS mtdTuPTPSetTodClkPeriod
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_U16 todClkPeriod 
);


/*******************************************************************************
MTD_STATUS mtdTuPTPGetTodClkPeriod
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    OUT MTD_U16 *todClkPeriod
);

 Inputs:
    devPtr - pointer to MTD_DEV initialized by mtdLoadDriver() call
    port - MDIO port address, 0-31

 Outputs:
    todClkPeriod  - Clock_Period 
       0x0 - 125 ms
       0x1 - 250 ms
       0x2 - 500 ms
       0x3 - 1.0 sec

 Returns:
    MTD_OK   - on success
    MTD_FAIL - on error

 Description:
    Get the TOD Clock Period bit that determines the period of TOD 
    alignment clock 

 Side effects:
    None

 Notes/Warnings:
    None
*******************************************************************************/
MTD_STATUS mtdTuPTPGetTodClkPeriod
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    OUT MTD_U16 *todClkPeriod
);


/*******************************************************************************
MTD_STATUS mtdTuPTPSetEnableTodClk
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_U16 enableTodClk  - 1:enable; 0:disable
);

 Inputs:
    devPtr - pointer to MTD_DEV initialized by mtdLoadDriver() call
    port - MDIO port address, 0-31
    enableTodClk  - enable/disable TAI Master to generate a periodic clock

 Outputs:
    None

 Returns:
    MTD_OK   - on success
    MTD_FAIL - on error

 Description:
    Enables TAI Master to generate a periodic clock

 Side effects:
    None

 Notes/Warnings:
    None
*******************************************************************************/
MTD_STATUS mtdTuPTPSetEnableTodClk
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_U16 enableTodClk /* 1: enable; 0:disable */
);


/*******************************************************************************
MTD_STATUS mtdTuPTPGetEnableTodClk
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    OUT MTD_U16 *enableTodClk - 1:enable; 0:disable
)

 Inputs:
    devPtr - pointer to MTD_DEV initialized by mtdLoadDriver() call
    port - MDIO port address, 0-31

 Outputs:
    enableTodClk  - enable/disable TAI Master to generate a periodic clock

 Returns:
    MTD_OK   - on success
    MTD_FAIL - on error

 Description:
    Gets the enable/disable state of TAI Master that generates a periodic clock

 Side effects:
    None

 Notes/Warnings:
    None
*******************************************************************************/
MTD_STATUS mtdTuPTPGetEnableTodClk
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    OUT MTD_U16 *enableTodClk /* 1: enable; 0: disable */
);


/*******************************************************************************
MTD_STATUS mtdTuPTPSetSyncTod
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_U16 enableSyncTod - 1: enable; 0: disable
);

 Inputs:
    devPtr - pointer to MTD_DEV initialized by mtdLoadDriver() call
    port - MDIO port address, 0-31
    enableSyncTod  - enable/disable TAI slave device to sync-up its TOD 
                     with TAI Master's TOD

 Outputs:
    None

 Returns:
    MTD_OK   - on success
    MTD_FAIL - on error

 Description:
    Enables TAI slave device to sync-up its TOD with TAI Master's TOD by 
    updating its TOD with PTP Global Time Array register values 
   
    It is recommended that the sync_tod bit is set and master and slave's 
    TOD's sync-up operation is performed at the very beginning of the 
    frequency drift compensation operation such that master and slave's 
    TOD's are aligned before the start of the frequency drift compensation.

 Side effects:
    None

 Notes/Warnings:
    None
*******************************************************************************/
MTD_STATUS mtdTuPTPSetSyncTod
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_U16 enableSyncTod /* 1: enable; 0: disable */
);


/*******************************************************************************
MTD_STATUS mtdTuPTPGetSyncTod
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    OUT MTD_U16 *enableSyncTod - 1: enable; 0: disable
);

 Inputs:
    devPtr - pointer to MTD_DEV initialized by mtdLoadDriver() call
    port - MDIO port address, 0-31

 Outputs:
    enableSyncTod  - enable/disable TAI slave device to sync-up its TOD 
                     with TAI Master's TOD

 Returns:
    MTD_OK   - on success
    MTD_FAIL - on error

 Description:
    This API gets the enable TAI slave device status to sync-up its TOD 
    with TAI Master's TOD by updating its TOD with PTP Global Time 
    Array register values 
   
    It is recommended that the sync_tod bit is set and master and slave's 
    TOD's sync-up operation is performed at the very beginning of the 
    frequency drift compensation operation such that master and slave's 
    TOD's are aligned before the start of the frequency drift compensation.

 Side effects:
    None

 Notes/Warnings:
    None
*******************************************************************************/
MTD_STATUS mtdTuPTPGetSyncTod
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    OUT MTD_U16 *enableSyncTod /* 1: enable; 0:disable */
);


/*******************************************************************************
MTD_STATUS mtdTuPTPSetTodTrigRequest
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_U16 enableTodTrigRequest - 1: enable; 0: disable
);

 Inputs:
    devPtr - pointer to MTD_DEV initialized by mtdLoadDriver() call
    port - MDIO port address, 0-31
    enableTodTrigRequest  - enable/disable TOD Trigger Request 

 Outputs:
    None

 Returns:
    MTD_OK   - on success
    MTD_FAIL - on error

 Description:
    TOD Trigger Request enables TAI Master to generate a single pulse 
    at "tod_trig_out" pin when master's TOD time matches that of PTP Global 
    Time Array register. The pulse width is controlled by the Pulse Width and 
    Pulse Width Range 
   
    This bit is self-cleared after the detection of a pulse at "tod_trig_out" pin.

 Side effects:
    None

 Notes/Warnings:
    None
*******************************************************************************/
MTD_STATUS mtdTuPTPSetTodTrigRequest
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_U16 enableTodTrigRequest /* 1: enable; 0: disable */
);


/*******************************************************************************
MTD_STATUS mtdTuPTPGetTodTrigRequest
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    OUT MTD_U16 *enableTodTrigRequest - 1: enable; 0: disable
);

 Inputs:
    devPtr - pointer to MTD_DEV initialized by mtdLoadDriver() call
    port - MDIO port address, 0-31

 Outputs:
    enableTodTrigRequest - enable/disable TOD Trigger Request 

 Returns:
    MTD_OK   - on success
    MTD_FAIL - on error

 Description:
    This API gets the TOD Trigger Request state that enables TAI Master 
    to generate a single pulse at "tod_trig_out" pin when master's TOD 
    time matches that of PTP Global Time Array register. The pulse width 
    is controlled by the Pulse Width and Pulse Width Range 
   
    This bit is self-cleared after the detection of a pulse at 
    "tod_trig_out" pin.

 Side effects:
    None

 Notes/Warnings:
    None
*******************************************************************************/
MTD_STATUS mtdTuPTPGetTodTrigRequest
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    OUT MTD_U16 *enableTodTrigRequest /* 1: enable; 0: disable */
);


/*******************************************************************************
MTD_STATUS mtdTuPTPSetMinDriftThreshold
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_U16 minDriftThreshold
);

 Inputs:
    devPtr - pointer to MTD_DEV initialized by mtdLoadDriver() call
    port - MDIO port address, 0-31
    minDriftThreshold - Minimum Drift Threshold 
                        The unit for this register is in 1ns. 
                        The default is set to 2ns (=1 clk_ptp_ts (500MHz) cycle)

 Outputs:
    None

 Returns:
    MTD_OK   - on success
    MTD_FAIL - on error

 Description:
    Minimum Drift Threshold defines the lower threshold. If frequency 
    drift measured between two consecutive alignment clock edges is equal 
    or below this threshold, no frequency drift compensation be performed.

    The unit for this register is in 1ns. 
    The default is set to 2ns (=1 clk_ptp_ts (500MHz) cycle)

 Side effects:
    None

 Notes/Warnings:
    None
*******************************************************************************/
MTD_STATUS mtdTuPTPSetMinDriftThreshold
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_U16 minDriftThreshold
);


/*******************************************************************************
MTD_STATUS mtdTuPTPGetMinDriftThreshold
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    OUT MTD_U16 *minDriftThreshold
);

 Inputs:
    devPtr - pointer to MTD_DEV initialized by mtdLoadDriver() call
    port - MDIO port address, 0-31

 Outputs:
    minDriftThreshold - Minimum Drift Threshold 
                        The unit for this register is in 1ns. 
                        The default is set to 2ns (=1 clk_ptp_ts (500MHz) cycle)

 Returns:
    MTD_OK   - on success
    MTD_FAIL - on error

 Description:
    Gets the Minimum Drift Threshold that defines the lower threshold. If  
    frequency drift measured between two consecutive alignment clock edges
    is equal or below this threshold, no frequency drift compensation be 
    performed.

    The unit for this register is in 1ns. 
    The default is set to 2ns (=1 clk_ptp_ts (500MHz) cycle)

 Side effects:
    None

 Notes/Warnings:
    None
*******************************************************************************/
MTD_STATUS mtdTuPTPGetMinDriftThreshold
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    OUT MTD_U16 *minDriftThreshold
);


/*******************************************************************************
MTD_STATUS mtdTuPTPSetMaxDriftThreshold
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_U16 maxDriftThreshold
);

 Inputs:
    devPtr - pointer to MTD_DEV initialized by mtdLoadDriver() call
    port - MDIO port address, 0-31
    maxDriftThreshold - Maximum Drift Threshold 
                        The unit for this register is in 1us. 
                        The default is set to 0x64 (or 100us).

 Outputs:
    None

 Returns:
    MTD_OK   - on success
    MTD_FAIL - on error

 Description:
    Maximum Drift Threshold defines the upper threshold. If frequency drift 
    measured between two consecutive alignment clock edges exceeds this 
    threshold, no frequency drift compensation be performed.

    The unit for this register is in 1us. 
    The default is set to 0x64 (or 100us). 

    For instance, the largest possible setting of 0x3FF (=1,023us) is bigger 
    than 1000ppm difference between master and slave's crystals over 1 second 
    period.

 Side effects:
    None

 Notes/Warnings:
    None
*******************************************************************************/
MTD_STATUS mtdTuPTPSetMaxDriftThreshold
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_U16 maxDriftThreshold
);


/*******************************************************************************
MTD_STATUS mtdTuPTPGetMaxDriftThreshold
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    OUT MTD_U16 *maxDriftThreshold
);

 Inputs:
    devPtr - pointer to MTD_DEV initialized by mtdLoadDriver() call
    port - MDIO port address, 0-31

 Outputs:
    maxDriftThreshold - Maximum Drift Threshold 
                        The unit for this register is in 1us. 
                        The default is set to 0x64 (or 100us).

 Returns:
    MTD_OK   - on success
    MTD_FAIL - on error

 Description:
    Gets the Maximum Drift Threshold that defines the upper threshold. 
    If frequency drift measured between two consecutive alignment clock 
    edges exceeds this threshold, no frequency drift compensation be 
    performed.

    The unit for this register is in 1us. 
    The default is set to 0x64 (or 100us). 

    For instance, the largest possible setting of 0x3FF (=1,023us) is bigger 
    than 1000ppm difference between master and slave's crystals over 1 second 
    period.
  
 Side effects:
    None

 Notes/Warnings:
    None
*******************************************************************************/
MTD_STATUS mtdTuPTPGetMaxDriftThreshold
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    OUT MTD_U16 *maxDriftThreshold
);


/*******************************************************************************
MTD_STATUS mtdTuPTPGetFreqDriftIntr
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    OUT MTD_U16 *freqDriftIntr
);

 Inputs:
    devPtr - pointer to MTD_DEV initialized by mtdLoadDriver() call
    port - MDIO port address, 0-31

 Outputs:
    freqDriftIntr - Frequency Drift Interrupt 

 Returns:
    MTD_OK   - on success
    MTD_FAIL - on error

 Description:
    Frequency Drift Interrupt is asserted when the frequency drift 
    exceeds maximum drift threshold.
  
 Side effects:
    None

 Notes/Warnings:
    None
*******************************************************************************/
MTD_STATUS mtdTuPTPGetFreqDriftIntr
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    OUT MTD_U16 *freqDriftIntr
);


/*******************************************************************************
MTD_STATUS mtdTuPTPGetClkGenIntr
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    OUT MTD_U16 *clkGenIntr
);

 Inputs:
    devPtr - pointer to MTD_DEV initialized by mtdLoadDriver() call
    port - MDIO port address, 0-31

 Outputs:
    clkGenIntr - Clock Generation Interrupt 

 Returns:
    MTD_OK   - on success
    MTD_FAIL - on error

 Description:
    Clock Generation Interrupt is asserted when rising edge of TOD 
    alignment clock is generated at the 'tod_clk_out' pin.
  
 Side effects:
    None

 Notes/Warnings:
    None
*******************************************************************************/
MTD_STATUS mtdTuPTPGetClkGenIntr
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    OUT MTD_U16 *clkGenIntr
);


/* PTP TAI Interrupt Mask Register */
#define MTD_PTP_INT_MASK_TRIG_GEN   0x0001   /* Trigger Generation Interrupt Mask */
#define MTD_PTP_INT_MASK_PULSE_IN   0x0002   /* Pulse In Interrupt. Mask          */
#define MTD_PTP_INT_MASK_CLK_GEN    0x0004   /* Clock Generation Interrupt Mask   */
#define MTD_PTP_INT_MASK_FREQ_DRIFT 0x0008   /* Frequency Drift Interrupt Mask    */
/*******************************************************************************
MTD_STATUS mtdTuPTPSetMaskFreqDriftInt
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_U16 maskFreqDriftInt
);

 Inputs:
    devPtr - pointer to MTD_DEV initialized by mtdLoadDriver() call
    port - MDIO port address, 0-31
    maskFreqDriftInt - MTD_PTP_INT_MASK_TRIG_GEN   - Trigger Generation Interrupt Mask
                       MTD_PTP_INT_MASK_PULSE_IN   - Pulse In Interrupt. Mask         
                       MTD_PTP_INT_MASK_CLK_GEN    - Clock Generation Interrupt Mask  
                       MTD_PTP_INT_MASK_FREQ_DRIFT - Frequency Drift Interrupt Mask   
 Outputs:
    None

 Returns:
    MTD_OK   - on success
    MTD_FAIL - on error

 Description:
    This API set the mask for the requested hardware frequency drift interrupts.
    The following interrupts are available:
    MTD_PTP_INT_MASK_TRIG_GEN   - Trigger Generation Interrupt Mask
    MTD_PTP_INT_MASK_PULSE_IN   - Pulse In Interrupt. Mask         
    MTD_PTP_INT_MASK_CLK_GEN    - Clock Generation Interrupt Mask  
    MTD_PTP_INT_MASK_FREQ_DRIFT - Frequency Drift Interrupt Mask   

    Calling mtdTuPTPGetFreqDriftIntTriggered API will retrieve the trigger interrupts  

 Side effects:
    None

 Notes/Warnings:
    None
*******************************************************************************/
MTD_STATUS mtdTuPTPSetMaskFreqDriftInt
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_U16 maskFreqDriftInt
);

/*******************************************************************************
MTD_STATUS mtdTuPTPGetMaskFreqDriftInt
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    OUT MTD_U16 *maskFreqDriftInt
);

 Inputs:
    devPtr - pointer to MTD_DEV initialized by mtdLoadDriver() call
    port - MDIO port address, 0-31

 Outputs:
    maskFreqDriftInt - MTD_PTP_INT_MASK_TRIG_GEN   - Trigger Generation Interrupt Mask
                       MTD_PTP_INT_MASK_PULSE_IN   - Pulse In Interrupt. Mask         
                       MTD_PTP_INT_MASK_CLK_GEN    - Clock Generation Interrupt Mask  
                       MTD_PTP_INT_MASK_FREQ_DRIFT - Frequency Drift Interrupt Mask   

 Returns:
    MTD_OK   - on success
    MTD_FAIL - on error

 Description:
    This API reads the hardware frequency drift interrupts mask that is set with the 
    mtdTuPTPSetMaskFreqDriftInt()

    Calling mtdTuPTPGetFreqDriftIntTriggered API will retrieve the trigger interrupts  
    
 Side effects:
    None

 Notes/Warnings:
    None
*******************************************************************************/
MTD_STATUS mtdTuPTPGetMaskFreqDriftInt
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    OUT MTD_U16 *maskFreqDriftInt
);

/*******************************************************************************
MTD_STATUS mtdTuPTPGetFreqDriftIntTriggered
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    OUT MTD_U16 *maskFreqDriftIntTriggered
);

 Inputs:
    devPtr - pointer to MTD_DEV initialized by mtdLoadDriver() call
    port - MDIO port address, 0-31

 Outputs:
    maskFreqDriftIntTriggered - MTD_PTP_INT_MASK_TRIG_GEN   - Trigger Generation Interrupt Mask
                                MTD_PTP_INT_MASK_PULSE_IN   - Pulse In Interrupt. Mask         
                                MTD_PTP_INT_MASK_CLK_GEN    - Clock Generation Interrupt Mask  
                                MTD_PTP_INT_MASK_FREQ_DRIFT - Frequency Drift Interrupt Mask   

 Returns:
    MTD_OK   - on success
    MTD_FAIL - on error

 Description:
    This API returns the maskFreqDriftIntTriggered mask bit for the hardware frequency drift interrupt 
    that are triggered. This is a self clearing bit which once read, the bit will be reset.

    Bit 0 - MTD_PTP_INT_MASK_TRIG_GEN: Trigger Generation Interrupt is asserted when trigger pulse is 
            generated at the tod_trig_out pin.
    Bit 1 - MTD_PTP_INT_MASK_PULSE_IN: Pulse In Interrupt is asserted when rising edge of trigger pulse 
            is detected at the tod_trig_in pin.
    Bit 2 - MTD_PTP_INT_MASK_CLK_GEN: Clock Generation Interrupt is asserted when rising edge of TOD 
            alignment clock is generated at the tod_clk_out pin.
    Bit 3 - MTD_PTP_INT_MASK_FREQ_DRIFT: Frequency Drift Interrupt is asserted when the frequency drift 
            exceeds maximum drift threshold.
    
 Side effects:
    None

 Notes/Warnings:
    None
*******************************************************************************/
MTD_STATUS mtdTuPTPGetFreqDriftIntTriggered
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    OUT MTD_U16 *maskFreqDriftIntTriggered
);

/*******************************************************************************
MTD_STATUS mtdTuPTPSetTAIGlobalCfg
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_TU_PTP_TAI_CONFIG taiCfgOptionBit,
    IN MTD_U16 taiCfgVal
);

 Inputs:
    devPtr - pointer to MTD_DEV initialized by mtdLoadDriver() call
    port - MDIO port address, 0-31
    taiCfgOptionBit - option in MTD_TU_PTP_TAI_CONFIG
    taiCfgVal - option value 

 Outputs:
    None

 Returns:
    MTD_OK   - on success
    MTD_FAIL - on error

 Description:
    This API configures the PTP Time Application Interface(TAI) Configuration 
    PTP Event Request Capture, PTP Trigger Generate, PTP Global time 
    increment/decrement, multi-ptp device time sync function etc.
  
    MTD_TU_PTP_TAI_CONFIG fields:
       MTD_TU_PTP_TAI_TRIG_GENREQ     - Trigger Generation Request
       MTD_TU_PTP_TAI_TRIG_MODE       - Trigger Mode
       MTD_TU_PTP_TAI_MULT_SYNC       - Multi PTP device Sync Mode
       MTD_TU_PTP_TAI_BLOCK_UPDATE    - Block Update
       MTD_TU_PTP_TAI_TRIG_LOCK       - Trigger Lock Enable
       MTD_TU_PTP_TAI_EVENT_CAP_INTR  - Event Capture Interrupt Enable
       MTD_TU_PTP_TAI_TRIG_GEN_INTR   - Trigger Generator Interrupt
       MTD_TU_PTP_TAI_TIME_INCDEC_EN  - Time Increment Decrement Enable 
       MTD_TU_PTP_TAI_EVENT_CNT_START - Event Counter Start 
       MTD_TU_PTP_TAI_EVENT_CAP_OW    - Event Capture Overwrite

       Refers to the device datasheet for detail information on these fields

 Side effects:
    None

 Notes/Warnings:
    None
*******************************************************************************/
MTD_STATUS mtdTuPTPSetTAIGlobalCfg
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_TU_PTP_TAI_CONFIG taiCfgOptionBit,
    IN MTD_U16 taiCfgVal
);

/*******************************************************************************
MTD_STATUS mtdTuPTPGetTAIGlobalCfg
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_TU_PTP_TAI_CONFIG taiCfgOptionBit,
    OUT MTD_U16 *taiCfgVal
);

 Inputs:
    devPtr - pointer to MTD_DEV initialized by mtdLoadDriver() call
    port - MDIO port address, 0-31
    taiCfgOptionBit - option in MTD_TU_PTP_TAI_CONFIG

 Outputs:
    taiCfgVal - option value 

 Returns:
    MTD_OK   - on success
    MTD_FAIL - on error

 Description:
    This API retrieves the configuration value of the TAI Configuration 
    PTP Event Request Capture, PTP Trigger Generate, PTP Global time 
    increment/decrement, multi-ptp device time sync function etc.
  
    MTD_TU_PTP_TAI_CONFIG fields:
       MTD_TU_PTP_TAI_TRIG_GENREQ     - Trigger Generation Request
       MTD_TU_PTP_TAI_TRIG_MODE       - Trigger Mode
       MTD_TU_PTP_TAI_MULT_SYNC       - Multi PTP device Sync Mode
       MTD_TU_PTP_TAI_BLOCK_UPDATE    - Block Update
       MTD_TU_PTP_TAI_TRIG_LOCK       - Trigger Lock Enable
       MTD_TU_PTP_TAI_EVENT_CAP_INTR  - Event Capture Interrupt Enable
       MTD_TU_PTP_TAI_TRIG_GEN_INTR   - Trigger Generator Interrupt
       MTD_TU_PTP_TAI_TIME_INCDEC_EN  - Time Increment Decrement Enable 
       MTD_TU_PTP_TAI_EVENT_CNT_START - Event Counter Start 
       MTD_TU_PTP_TAI_EVENT_CAP_OW    - Event Capture Overwrite

       Refers to the device datasheet for detail information on these fields

 Side effects:
    None

 Notes/Warnings:
    None
*******************************************************************************/
MTD_STATUS mtdTuPTPGetTAIGlobalCfg
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_TU_PTP_TAI_CONFIG taiCfgOptionBit,
    OUT MTD_U16 *taiCfgVal
);

/*******************************************************************************
MTD_STATUS mtdTuPTPGetTimeStampClkPeriod 
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    OUT MTD_U16 *tsClkPeriod
);

 Inputs:
    devPtr - pointer to MTD_DEV initialized by mtdLoadDriver() call
    port - MDIO port address, 0-31

 Outputs:
    tsClkPeriod - Time Stamping Clock Period in pico seconds 

 Returns:
    MTD_OK   - on success
    MTD_FAIL - on error

 Description:
    This API reads the Time Stamping Clock Period in pico seconds.
    This field specifies the clock period for the time stamping clock supplied 
    to the PTP hardware.

 Side effects:
    None

 Notes/Warnings:
    None
*******************************************************************************/
MTD_STATUS mtdTuPTPGetTimeStampClkPeriod 
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    OUT MTD_U16 *tsClkPeriod
);

/*******************************************************************************
MTD_STATUS mtdTuPTPSetTrigGenAmt
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_U32 trigGenAmt
);

 Inputs:
    devPtr - pointer to MTD_DEV initialized by mtdLoadDriver() call
    port - MDIO port address, 0-31
    trigGenAmt - Trigger Generation Amount bits in 32-bit
                     Bit 0-15  in MTD_TU_PTP_TAI_GLOBAL_TRIG_GEN
                     Bit 16-31 in MTD_TU_PTP_TAI_GLOBAL_TRIG_GEN+1

 Outputs:
    None

 Returns:
    MTD_OK   - on success
    MTD_FAIL - on error

 Description:
    This API sets the PTP Time Application Interface trigger generation time amount.

    When TrigMode is set, the value specified in this field is compared with 
    the PTP Global Timer (TAI offset 0x09 & 0x0A) and when it matches the first time, 
    a pulse is generated on PTP_TRIG whose width is controlled by PulseWidth (TAI 
    offset 0x05). In this mode there is an internal delay of three TSClkPer before 
    the leading edge of the pulse will be seen on the PTP_TRIG output pin.

    When TrigMode is cleared to zero, the value in this field is used as a clock period in 
    TSClkPer increments (TAI offset 0x01) to generate an output clock on the PTP_TRIG signal 
    (see TrigPhase in TAI offset 0x00). In this mode the TrigClkComp amount (TAI offset 0x04) 
    and TrigClkCompSubPs (TAI offset 0x05) gets accumulated once per TrigGenAmt cycle and 
    when this accumulated value exceeds the value specified in TSClkPer, one TSClkPer amount 
    gets added to or subtracted from the next trailing edge of PTP_TRIG clock output.

 Side effects:
    None

 Notes/Warnings:
    None
*******************************************************************************/
MTD_STATUS mtdTuPTPSetTrigGenAmt
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_U32 trigGenAmt
);

/*******************************************************************************
MTD_STATUS mtdTuPTPSetTrigCompensateDir
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_U16 trigCompensateDir
);

 Inputs:
    devPtr - pointer to MTD_DEV initialized by mtdLoadDriver() call
    port - MDIO port address, 0-31
    trigCompensateDir - Trig Clock Compensation Direction
                        0 = Add one TSClkPer to the next PTP_TRIG cycle
                        1 = Subtract one TSClkPer from the next PTP_TRIG cycle

 Outputs:
    None

 Returns:
    MTD_OK   - on success
    MTD_FAIL - on error

 Description:
    This API sets the Trigger Clock Compensation Direction

    When the accumulated TrigClkComp amount mtdTuPTPGetTrigCompensate() exceeds the 
    value in TSClkPer (TAI offset 0x01), one TSClkPer amount gets added to or 
    subtracted from the next PTP_TRIG clock output. This bit determines which as follows:
    0 = Add one TSClkPer to the next PTP_TRIG cycle
    1 = Subtract one TSClkPer from the next PTP_TRIG cycle

 Side effects:
    None

 Notes/Warnings:
    None
*******************************************************************************/
MTD_STATUS mtdTuPTPSetTrigCompensateDir
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_U16 trigCompensateDir
);

/*******************************************************************************
MTD_STATUS mtdTuPTPGetTrigCompensateDir
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    OUT MTD_U16 *trigCompensateDir
);

 Inputs:
    devPtr - pointer to MTD_DEV initialized by mtdLoadDriver() call
    port - MDIO port address, 0-31

 Outputs:
    trigCompensateDir - Trig Clock Compensation Direction
                        0 = Add one TSClkPer to the next PTP_TRIG cycle
                        1 = Subtract one TSClkPer from the next PTP_TRIG cycle

 Returns:
    MTD_OK   - on success
    MTD_FAIL - on error

 Description:
    This API reads the Trigger Clock Compensation Direction
    0 = Add one TSClkPer to the next PTP_TRIG cycle
    1 = Subtract one TSClkPer from the next PTP_TRIG cycle

 Side effects:
    None

 Notes/Warnings:
    None
*******************************************************************************/
MTD_STATUS mtdTuPTPGetTrigCompensateDir
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    OUT MTD_U16 *trigCompensateDir
);

/*******************************************************************************
MTD_STATUS mtdTuPTPSetTrigCompensate
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_U16 trigCompensate
);

 Inputs:
    devPtr - pointer to MTD_DEV initialized by mtdLoadDriver() call
    port - MDIO port address, 0-31
    trigCompensate - Trigger mode Clock Compensation Amount in pico seconds as an unsigned number
                     15-bit unsigned int. Bit 16 is used for compensation direction

 Outputs:
    None

 Returns:
    MTD_OK   - on success
    MTD_FAIL - on error

 Description:
    This API sets the Trigger mode Clock Compensation Amount.
    
    This field is used in 50% duty cycle clock mode only (when TrigMode is cleared to zero 
    and TrigGenReq is set to one).

    This field specifies the remainder amount in pico secs for the clock that is being generated 
    with a period specified by the TrigGenAmt (TAI offset 0x02 & 0x03). This field must be 
    set as an absolute error number in pico secs (in other words it is a magnitude difference) 
    regardless if the local clock is too fast or too slow compared to the reference clock. 

    The direction of the clock compensation is configured in the CompDir bit above.
    In the 50% duty cycle clock mode this register gets accumulated once per TrigGenAmt 
    cycle and when this accumulated value exceeds the value specified in TSClkPer (TAI 
    offset 0x01), one TSClkPer amount gets added to or subtracted from the next PTP_TRIG 
    clock output.

    This requires that the absolute value of TrigClkComp must not exceed the size of the 
    TSClkPer. If it does, the TSClkPer needs to be adjusted in size (either up or down) 
    until the remainder that remains is less that the TSClkPer and that value gets put 
    into this register.

 Side effects:
    None

 Notes/Warnings:
    None
*******************************************************************************/
MTD_STATUS mtdTuPTPSetTrigCompensate
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_U16 trigCompensate
);

/*******************************************************************************
MTD_STATUS mtdTuPTPGetTrigCompensate
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    OUT MTD_U16 *trigCompensate
);

 Inputs:
    devPtr - pointer to MTD_DEV initialized by mtdLoadDriver() call
    port - MDIO port address, 0-31

 Outputs:
    trigCompensate - Trigger mode Clock Compensation Amount in pico seconds as an unsigned number
                     15-bit unsigned int. Bit 16 is used for compensation direction

 Returns:
    MTD_OK   - on success
    MTD_FAIL - on error

 Description:
    This API reads the Trigger mode Clock Compensation Amount.
    
    Refer to mtdTuPTPSetTrigCompensate() for more details on this trigger compensation value.

 Side effects:
    None

 Notes/Warnings:
    None
*******************************************************************************/
MTD_STATUS mtdTuPTPGetTrigCompensate
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    OUT MTD_U16 *trigCompensate
);

/*******************************************************************************
MTD_STATUS mtdTuPTPSetTrigCompensatePS
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_U16 trigCompensateSubPS
);

 Inputs:
    devPtr - pointer to MTD_DEV initialized by mtdLoadDriver() call
    port - MDIO port address, 0-31
    trigCompensateSubPS - Trigger mode Clock Compensation Amount in sub pico seconds as an unsigned number
                          8-bit unsigned int

 Outputs:
    None

 Returns:
    MTD_OK   - on success
    MTD_FAIL - on error

 Description:
    This API sets the Trigger mode Clock Compensation Amount. This value is in Sub Pico seconds 
    as an unsigned number.
    
    This field is used in 50% duty cycle clock mode only (when TrigMode is cleared to zero and 
    TrigGenReq is set to one).

    This field specifies the remainder amount in sub pico sec increments for the clock that is being 
    generated with a period specified by the TrigGenAmt (TAI offset 0x02 & 0x03). This field 
    must be set as an absolute error number in Sub pico sec (in other words it is a magnitude difference) 
    regardless if the local clock is too fast or too slow compared to the reference clock. The 
    direction of the clock compensation is configured in the CompDir bit above. Each unit in this 
    register is 1/256th of a pico sec or approximately 4 femto seconds (actual number is 3.90625 femto 
    seconds per unit).

 Side effects:
    None

 Notes/Warnings:
    None
*******************************************************************************/
MTD_STATUS mtdTuPTPSetTrigCompensatePS
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_U16 trigCompensateSubPS
);

/*******************************************************************************
MTD_STATUS mtdTuPTPGetTrigCompensatePS
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    OUT MTD_U16 *trigCompensateSubPS
);

 Inputs:
    devPtr - pointer to MTD_DEV initialized by mtdLoadDriver() call
    port - MDIO port address, 0-31

 Outputs:
    trigCompensateSubPS - Trigger mode Clock Compensation Amount in sub pico seconds as an 
                          8-bit unsigned number

 Returns:
    MTD_OK   - on success
    MTD_FAIL - on error

 Description:
    This API reads the Trigger mode Clock Compensation Amount sub pico seconds.
    
    Refer to mtdTuPTPSetTrigCompensatePS() for more details on this trigger compensation value.

 Side effects:
    None

 Notes/Warnings:
    None
*******************************************************************************/
MTD_STATUS mtdTuPTPGetTrigCompensatePS
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    OUT MTD_U16 *trigCompensateSubPS
);

/*******************************************************************************
MTD_STATUS mtdTuPTPSetTrigPulseWidth
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_U16 trigPulseWidth
);

 Inputs:
    devPtr - pointer to MTD_DEV initialized by mtdLoadDriver() call
    port - MDIO port address, 0-31
    trigPulseWidth - Pulse width for PTP_TRIG; 4-bit length

 Outputs:
    None

 Returns:
    MTD_OK   - on success
    MTD_FAIL - on error

 Description:
    This API sets the Pulse width for PTP_TRIG.
    
    This pulse width is in units of TSClkPer (TAI offset 0x01). This specifies the width 
    of the pulse that gets generated on PTP_TRIG (see TrigPhase in TAI offset 0x00) when 
    the one shot pulse

 Side effects:
    None

 Notes/Warnings:
    None
*******************************************************************************/
MTD_STATUS mtdTuPTPSetTrigPulseWidth
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_U16 trigPulseWidth
);

/*******************************************************************************
MTD_STATUS mtdTuPTPGetTrigPulseWidth
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    OUT MTD_U16 *trigPulseWidth
);

 Inputs:
    devPtr - pointer to MTD_DEV initialized by mtdLoadDriver() call
    port - MDIO port address, 0-31

 Outputs:
    trigPulseWidth - Pulse width for PTP_TRIG; 4-bit length

 Returns:
    MTD_OK   - on success
    MTD_FAIL - on error

 Description:
    This API reads the Pulse width for PTP_TRIG.
    
    This pulse width is in units of TSClkPer (TAI offset 0x01). This specifies the width 
    of the pulse that gets generated on PTP_TRIG (see TrigPhase in TAI offset 0x00) when 
    the one shot pulse

 Side effects:
    None

 Notes/Warnings:
    None
*******************************************************************************/
MTD_STATUS mtdTuPTPGetTrigPulseWidth
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    OUT MTD_U16 *trigPulseWidth
);

/*******************************************************************************
MTD_STATUS mtdTuPTPSetTrigPulseWidthRange
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_U16 trigPulseWidthRange
);

 Inputs:
    devPtr - pointer to MTD_DEV initialized by mtdLoadDriver() call
    port - MDIO port address, 0-31
    trigPulseWidthRange - Pulse Width Range for the PTP_TRIG signal
                          3-bit length

 Outputs:
    None

 Returns:
    MTD_OK   - on success
    MTD_FAIL - on error

 Description:
    This API sets the Pulse Width Range for the PTP_TRIG signal.

    This register selects the units of time used to define the Pulse Width as follows 
    (each higher numbered selection generates units that are 8x larger than the previous 
    lower numbered selection):
    0x0 = 8 nSec units for a 125 MHz PTP clock or 1 x TSClkPer
    0x1 = 64 nSec units for a 125 MHz PTP clock or 8 x TSClkPer
    0x2 = 512 nSec units for a 125 MHz PTP clock or 64 x TSClkPer
    0x3 = 4,096 nSec units for a 125 MHz PTP clock or 512 x TSClkPer
    0x4 = 32.768 uSec units for a 125 MHz PTP clock or 4,096 x TSClkPer
    0x5 = 262.144 uSec units for a 125 MHz PTP clock or 32,768 x TSClkPer
    0x6 = 2.097 mSec units for a 125 MHz PTP clock or 262,144 x TSClkPer
    0x7 = 16.777 mSec units for a 125 MHz PTP clock or 2,097,152 x TSClkPer

    The narrowest width is 8 nSec (assuming a 125 MHz PTP clock) or one TSClkPer 
    (by setting this register to 0x0 and the Pulse Width register, above, to 0x1). 
    The widest width is 251 mSec (assuming a 125 MHz PTP clock) or just a bit over 
    1/4 second (by setting this register to 0x7 and the Pulse Width register to 0xF). 
    The maximum number is 31,457,280 TSClkPer.
 
 Side effects:
    None

 Notes/Warnings:
    None
*******************************************************************************/
MTD_STATUS mtdTuPTPSetTrigPulseWidthRange
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_U16 trigPulseWidthRange
);

/*******************************************************************************
MTD_STATUS mtdTuPTPGetTrigPulseWidthRange
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    OUT MTD_U16 *trigPulseWidthRange
);

 Inputs:
    devPtr - pointer to MTD_DEV initialized by mtdLoadDriver() call
    port - MDIO port address, 0-31

 Outputs:
    trigPulseWidthRange - Pulse Width Range for the PTP_TRIG signal
                          3-bit length

 Returns:
    MTD_OK   - on success
    MTD_FAIL - on error

 Description:
    This API reads the Pulse Width Range for the PTP_TRIG signal.

    This register selects the units of time used to define the Pulse Width as follows 
    (each higher numbered selection generates units that are 8x larger than the previous 
    lower numbered selection):
    0x0 = 8 nSec units for a 125 MHz PTP clock or 1 x TSClkPer
    0x1 = 64 nSec units for a 125 MHz PTP clock or 8 x TSClkPer
    0x2 = 512 nSec units for a 125 MHz PTP clock or 64 x TSClkPer
    0x3 = 4,096 nSec units for a 125 MHz PTP clock or 512 x TSClkPer
    0x4 = 32.768 uSec units for a 125 MHz PTP clock or 4,096 x TSClkPer
    0x5 = 262.144 uSec units for a 125 MHz PTP clock or 32,768 x TSClkPer
    0x6 = 2.097 mSec units for a 125 MHz PTP clock or 262,144 x TSClkPer
    0x7 = 16.777 mSec units for a 125 MHz PTP clock or 2,097,152 x TSClkPer

    The narrowest width is 8 nSec (assuming a 125 MHz PTP clock) or one TSClkPer 
    (by setting this register to 0x0 and the Pulse Width register, above, to 0x1). 
    The widest width is 251 mSec (assuming a 125 MHz PTP clock) or just a bit over 
    1/4 second (by setting this register to 0x7 and the Pulse Width register to 0xF). 
    The maximum number is 31,457,280 TSClkPer.
 
 Side effects:
    None

 Notes/Warnings:
    None
*******************************************************************************/
MTD_STATUS mtdTuPTPGetTrigPulseWidthRange
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    OUT MTD_U16 *trigPulseWidthRange
);

/*******************************************************************************
MTD_STATUS mtdTuPTPSetTrigGenTime
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_U32 trigGenTime
);

 Inputs:
    devPtr - pointer to MTD_DEV initialized by mtdLoadDriver() call
    port - MDIO port address, 0-31
    trigGenTime - Trigger Generation Time 32-bit length

 Outputs:
    None

 Returns:
    MTD_OK   - on success
    MTD_FAIL - on error

 Description:
    This API sets the Trigger Generation Time.

    This field specifies the PTP Global Time (TAI offsets 0x0E & 0x0F) where the 
    1st leading edge of PTP_TRIG will occur (with a three TSClkPer latency) when 
    PTP Trig is in the continuous square wave mode (i.e, when TrigMode is 0x0, 
    offset 0x00 above) as long as this register's value is non-zero. If its value 
    is zero, the 1st leading edge of PTP_TRIG will occur when TrigGenReg is set 
    to a one (TAI offset 0x00).
 
 Side effects:
    None

 Notes/Warnings:
    None
*******************************************************************************/
MTD_STATUS mtdTuPTPSetTrigGenTime
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_U32 trigGenTime
);

/*******************************************************************************
MTD_STATUS mtdTuPTPGetTrigGenTime
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_U32 *trigGenTime
);

 Inputs:
    devPtr - pointer to MTD_DEV initialized by mtdLoadDriver() call
    port - MDIO port address, 0-31

 Outputs:
    trigGenTime - Trigger Generation Time 32-bit length

 Returns:
    MTD_OK   - on success
    MTD_FAIL - on error

 Description:
    This API reads the Trigger Generation Time.

    This field specifies the PTP Global Time (TAI offsets 0x0E & 0x0F) where the 
    1st leading edge of PTP_TRIG will occur (with a three TSClkPer latency) when 
    PTP Trig is in the continuous square wave mode (i.e, when TrigMode is 0x0, 
    offset 0x00 above) as long as this register's value is non-zero. If its value 
    is zero, the 1st leading edge of PTP_TRIG will occur when TrigGenReg is set 
    to a one (TAI offset 0x00).
 
 Side effects:
    None

 Notes/Warnings:
    None
*******************************************************************************/
MTD_STATUS mtdTuPTPGetTrigGenTime
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_U32 *trigGenTime
);

#ifdef __cplusplus
}
#endif

#endif /* MTD_TU_PTP */

#endif /* MTD_TU_PTP_H */
