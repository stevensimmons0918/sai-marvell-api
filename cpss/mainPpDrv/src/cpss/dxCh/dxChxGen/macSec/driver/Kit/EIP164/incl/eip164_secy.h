/* eip164_secy.h
 *
 * EIP-164 Driver Library MACsec SecY API
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

#ifndef EIP164_SECY_H_
#define EIP164_SECY_H_

/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

/* Driver Framework Basic Definitions API */
#include <Kit/DriverFramework/incl/basic_defs.h>             /* uint32_t */

/* Driver Framework Device API */
#include <Kit/DriverFramework/incl/device_types.h>           /* Device_Handle_t */

#include <Kit/EIP164/incl/eip164_types.h>


/*----------------------------------------------------------------------------
 * Definitions and macros
 */

/*
 * Interrupt Sources Mask for EIP-66 Device
 */
#define EIP164_SECY_INT_LENGTH_ERR0         BIT_0   /* Length error interrupt (e0) */
#define EIP164_SECY_INT_LENGTH_ERR1         BIT_1   /* Length error interrupt (e1) */
#define EIP164_SECY_INT_LENGTH_ERR2         BIT_2   /* Length error interrupt  (e2) */
#define EIP164_SECY_INT_CTR_ERR             BIT_3   /* AES counter error interrupt (e3) */
#define EIP164_SECY_INT_TOKEN_ERR           BIT_4   /* Token error interrupt (e4) */
#define EIP164_SECY_INT_CTX_ERR             BIT_5   /* Context error interrupt (e5) */
#define EIP164_SECY_INT_ECC_ERR             BIT_6   /* Context ECC error interrupt (e6) */
#define EIP164_SECY_INT_IB_ZERO_PKTNUM      BIT_7   /* Inbound zero packet number (e7). */
#define EIP164_SECY_INT_OB_SEQNR_THRSH      BIT_8   /* Outbound sequence number threshold interrupt (e8) */
#define EIP164_SECY_INT_IB_AUTH_FAIL        BIT_9    /* Inbound authentication failure (e9) */
#define EIP164_SECY_INT_OB_SEQNR_RLOVR      BIT_10   /* Outbound sequence number roll-over interrupt (e10) */
#define EIP164_SECY_INT_IB_SL               BIT_11   /* Inbound short length interrupt (e11) */
#define EIP164_SECY_INT_OB_MTU_CHECK        BIT_12   /* Outbound MTU check interrupt (e12) */
#define EIP164_SECY_INT_OB_EXPANSION_ERR    BIT_13   /* Outbound packet expansion error (e13) */


/* Clock bitmask specification for clock control */
#define EIP164_SECY_DEVICE_PE_CLOCK         BIT_0   /* Packet transform engine */
#define EIP164_SECY_DEVICE_CE_CLOCK         BIT_1   /* Classification engine */
#define EIP164_SECY_DEVICE_IPB_CLOCK        BIT_2   /* Input packet buffer */
#define EIP164_SECY_DEVICE_RXCAM_CLOCK      BIT_3   /* RX CAM */
#define EIP164_SECY_DEVICE_IDB_CLOCK        BIT_4   /* Input delay buffer */
#define EIP164_SECY_DEVICE_ODB_CLOCK        BIT_5   /* Output delay buffer */


/*  Clock bitmask specification for packet engine clock control */
#define EIP164_SECY_DEVICE_PE_AES_KS_CLOCK  BIT_16  /* AES Key schedule */
#define EIP164_SECY_DEVICE_PE_AES_CLOCK     BIT_17  /* AES core */
#define EIP164_SECY_DEVICE_PE_GHASH_CLOCK   BIT_18  /* GHASH core. */
#define EIP164_SECY_DEVICE_PE_HKEY_CLOCK    BIT_19   /* HKEY */
#define EIP164_SECY_DEVICE_PE_DPATH_CLOCK   BIT_20   /* Data patch */

/* SecY Egress Statistics */
typedef struct
{
    /* Packet counters */
    EIP164_UI64_t TransformErrorPkts;

    EIP164_UI64_t OutPktsCtrl;
    EIP164_UI64_t OutPktsUntagged;
} EIP164_SecY_Statistics_E_t;


/* SecY Ingress Statistics */
typedef struct
{
    /* Packet counters */
    EIP164_UI64_t TransformErrorPkts;

    EIP164_UI64_t InPktsCtrl;
    EIP164_UI64_t InPktsUntagged;
    EIP164_UI64_t InPktsNoTag;
    EIP164_UI64_t InPktsBadTag;
    EIP164_UI64_t InPktsNoSCI;
    EIP164_UI64_t InPktsUnknownSCI;
    EIP164_UI64_t InPktsTaggedCtrl;
} EIP164_SecY_Statistics_I_t;

/* SecY Statistics */
typedef union
{
    EIP164_SecY_Statistics_E_t Egress;
    EIP164_SecY_Statistics_I_t Ingress;
} EIP164_SecY_Statistics_t;

/*----------------------------------------------------------------------------
 * EIP164_SecY_SA_ActionType_t
 *
 * SA action type:
 *      bypass,
 *      drop,
 *      MACsec ingress (do not use for an egress only device),
 *      MACsec egress (do not use for an ingress only device).
 *
 */
typedef enum
{
    EIP164_SECY_SA_ACTION_BYPASS,
    EIP164_SECY_SA_ACTION_DROP,
    EIP164_SECY_SA_ACTION_INGRESS,
    EIP164_SECY_SA_ACTION_EGRESS,
    EIP164_SECY_SA_ACTION_CRYPT_AUTH
} EIP164_SecY_SA_ActionType_t;

typedef enum
{
    EIP164_CHANNEL_MODE_MACSEC,
    EIP164_CHANNEL_MODE_INVALID,
} EIP164_ChannelMode_t;

/* SA parameters for Egress action type */
typedef struct
{
    /* The number of bytes (in the range of 0-127) that are authenticated but */
    /* not encrypted following the SecTAG in the encrypted packet. Values */
    /* 65-127 are reserved in HW < 4.0 and should not be used there. */
    uint8_t ConfidentialityOffset;

    /* true - enable frame protection, */
    /* false - bypass frame through device */
    bool fProtectFrames;

    /* true - inserts explicit SCI in the packet, */
    /* false - use implicit SCI (not transferred) */
    bool fIncludeSCI;

    /* true - enable ES bit in the generated SecTAG */
    /* false - disable ES bit in the generated SecTAG */
    bool fUseES;

    /* true - enable SCB bit in the generated SecTAG */
    /* false - disable SCB bit in the generated SecTAG */
    bool fUseSCB;

    /* true - enable confidentiality protection */
    /* false - disable confidentiality protection */
    bool fConfProtect;

    /* true - allow data (non-control) packets. */
    /* false - drop data packets. */
    bool fAllowDataPkts;

    /* Specifies number of bytes from the start of the packet to be bypassed
     * without MACsec protection.
     */
    uint8_t PreSecTagAuthStart;

    /* Specifies number of bytes to be authenticated in pre-SecTAG area */
    uint8_t PreSecTagAuthLength;
} EIP164_SecY_SA_E_t;

/* SA parameters for Ingress action type */
typedef struct
{
    /* The number of bytes (in the range of 0-127) that are authenticated but */
    /* not encrypted following the SecTAG in the encrypted packet. */
    uint8_t ConfidentialityOffset;

    /* true - enable replay protection */
    /* false - disable replay protection */
    bool fReplayProtect;

    /* MACsec frame validation level */
    EIP164_ValidateFrame_t ValidateFramesTagged;

    /* true - allow tagged packets. */
    /* false - drop tagged packets. */
    bool fAllowTagged;

    /* true - allow untagged packets. */
    /* false - drop untagged packets. */
    bool fAllowUntagged;

    /* true - enabled validate untagged packets. */
    /* false - disabled validate untagged packets. */
    bool fValidateUntagged;

    uint8_t PreSecTagAuthStart;
    uint8_t PreSecTagAuthLength;

    /* For situations when RxSC is not found or SAinUse=0 with validation level */
    /* that allows packet to be sent to the Controlled port with the SecTAG/ICV */
    /* removed, this flag represents a policy to allow SecTAG retaining. */
    /* true - SecTAG is retained. */
    bool fRetainSecTAG;

    /* true - ICV is retained (allowed only when fRetainSecTAG is true). */
    bool fRetainICV;
} EIP164_SecY_SA_I_t;

/* SA parameters for Bypass/Drop action type */
typedef struct
{
    int dummy;
} EIP164_SecY_SA_BD_t;

/* SA parameters for Crypt-authenticate action type */
typedef struct
{
    /* true - message has length 0 */
    /* false - message has length > 0 */
    bool fZeroLengthMessage;

    /* The number of bytes (in the range of 0-255) that are authenticated but */
    /* not encrypted (AAD length). */
    uint8_t ConfidentialityOffset;

    /* IV loading mode: */
    /* 0: The IV is fully loaded via the transform record. */
    /* 1: The full IV is loaded via the input frame. This IV is located in */
    /*    front of the frame and is considered to be part of the bypass data, */
    /*    however it is not part to the result frame. */
    /* 2: The full IV is loaded via the input frame. This IV is located at the */
    /*    end of the bypass data and is considered to be part of the bypass */
    /*    data, and it also part to the result frame. */
    /* 3: The first three IV words are loaded via the input frame, the counter */
    /*    value of the IV is set to one. The three IV words are located in */
    /*    front of the frame and are considered to be part of the bypass data, */
    /*    however it is not part to the result frame. */
    uint8_t IVMode;

    /* true - append the calculated ICV */
    /* false - don't append the calculated ICV */
    bool fICVAppend;

    /* true - enable ICV verification */
    /* false - disable ICV verification */
    bool fICVVerify;

    /* true - enable confidentiality protection (AES-GCM/CTR operation) */
    /* false - disable confidentiality protection (AES-GMAC operation) */
    bool fConfProtect;
} EIP164_SecY_SA_CA_t;

/*----------------------------------------------------------------------------
 * EIP164_SecY_SA_t
 *
 * SecY SA data structure that contains data required to add a new SA.
 *
 */
typedef struct
{
    /* SA parameters */
    union
    {
        EIP164_SecY_SA_E_t       Egress;
        EIP164_SecY_SA_I_t       Ingress;
        EIP164_SecY_SA_BD_t      BypassDrop;
        EIP164_SecY_SA_CA_t      CryptAuth;
    } Params;

    /* SA action type, see EIP164_SecY_SA_ActionType_t */
    EIP164_SecY_SA_ActionType_t ActionType;

    /* SA drop type, see EIP164_SecY_DropType_t */
    EIP164_DropType_t DropType;

    /* Egress only */
    /* Perform drop action if packet is not from the reserved port */
    bool fDropNonReserved;

    EIP164_PortType_t DestPort;

} EIP164_SecY_SA_t;

/* SA statistics */
typedef struct
{
    /* Octet counters */
    EIP164_UI64_t OutOctetsEncryptedProtected;

    /* Packet counters */
    EIP164_UI64_t OutPktsEncryptedProtected;
    EIP164_UI64_t OutPktsTooLong;
    EIP164_UI64_t OutPktsSANotInUse;
} EIP164_SecY_SA_Stat_E_t;

typedef struct
{
    /* Octet counters */
    EIP164_UI64_t InOctetsDecrypted;
    EIP164_UI64_t InOctetsValidated;

    /* Packet counters */
    EIP164_UI64_t InPktsUnchecked;
    EIP164_UI64_t InPktsDelayed;
    EIP164_UI64_t InPktsLate;
    EIP164_UI64_t InPktsOK;
    EIP164_UI64_t InPktsInvalid;
    EIP164_UI64_t InPktsNotValid;
    EIP164_UI64_t InPktsNotUsingSA;
    EIP164_UI64_t InPktsUnusedSA;
} EIP164_SecY_SA_Stat_I_t;


typedef union
{
    EIP164_SecY_SA_Stat_E_t Egress;
    EIP164_SecY_SA_Stat_I_t Ingress;
} EIP164_SecY_SA_Stat_t;

/* IFC/IFC1 statistics */
typedef struct
{
    /* Octet counters */
    EIP164_UI64_t OutOctetsUncontrolled;
    EIP164_UI64_t OutOctetsControlled;
    EIP164_UI64_t OutOctetsCommon;

    /* Packet counters */
    EIP164_UI64_t OutPktsUnicastUncontrolled;
    EIP164_UI64_t OutPktsMulticastUncontrolled;
    EIP164_UI64_t OutPktsBroadcastUncontrolled;

    EIP164_UI64_t OutPktsUnicastControlled;
    EIP164_UI64_t OutPktsMulticastControlled;
    EIP164_UI64_t OutPktsBroadcastControlled;
} EIP164_SecY_Ifc_Stat_E_t;

typedef struct
{
    /* Octet counters */
    EIP164_UI64_t InOctetsUncontrolled;
    EIP164_UI64_t InOctetsControlled;

    /* Packet counters */
    EIP164_UI64_t InPktsUnicastUncontrolled;
    EIP164_UI64_t InPktsMulticastUncontrolled;
    EIP164_UI64_t InPktsBroadcastUncontrolled;

    EIP164_UI64_t InPktsUnicastControlled;
    EIP164_UI64_t InPktsMulticastControlled;
    EIP164_UI64_t InPktsBroadcastControlled;
} EIP164_SecY_Ifc_Stat_I_t;

typedef union
{
    EIP164_SecY_Ifc_Stat_E_t Egress;
    EIP164_SecY_Ifc_Stat_I_t Ingress;
} EIP164_SecY_Ifc_Stat_t;


/* RxCAM hit statistics */
typedef struct
{
    /* Packet counter */
    EIP164_UI64_t CAMHit;
} EIP164_SecY_RxCAM_Stat_t;

/* Number of ECC status counters. */
#define EIP164_ECC_NOF_STATUS_COUNTERS 19


/* Debug registers for packet classification. */
typedef struct
{
    uint32_t ParsedDALo;
    uint32_t ParsedDAHi;
    uint32_t ParsedSALo;
    uint32_t ParsedSAHi;
    uint32_t ParsedSecTAGLo;
    uint32_t ParsedSecTAGHi;
    uint32_t ParsedSCILo;
    uint32_t ParsedSCIHi;
    uint32_t SecTAGDebug;
    uint32_t RxCAMSCILo;
    uint32_t RxCAMSCIHi;
    uint32_t ParserInDebug;
} EIP164_Device_PktProcessDebug_t;

/* ECC Error status */
typedef struct
{
    unsigned int CorrectableCount;
    unsigned int UncorrectableCount;
    bool fCorrectableThr;        /* Threshold for correctable errors exceeded */
    bool fUncorrectableThr;      /* Threshold for uncorrectable errors exceeded */
} EIP164_Device_ECCStatus_t;
/*----------------------------------------------------------------------------
 * EIP164_Device_Init
 *
 * This function starts the initialization of an EIP-164 device.
 *
 * This function returns the EIP164_HW_CONFIGURATION_MISMATCH error code
 * when it detects a mismatch in the EIP-164 Driver Library configuration.
 *
 * Note: This function should be called after the EIP-164 HW Reset.
 *
 * API use order:
 *      This function must be executed before any of the EIP164_*() functions
 *      can be called except the EIP164_HWRevision_Get() function.
 *      If this function returns EIP164_BUSY_RETRY_LATER then
 *      the EIP164_Device_Is_Init_Done() should be called afterwards until it
 *      returns EIP164_NO_ERROR. Only then the device initialization can be
 *      considered completed and the other API functions may be called for
 *      this device.
 *
 * IOArea_p (input/output)
 *      Pointer to the Device instance I/O area in memory that will be used to
 *      identify the EIP-164 device.
 *
 * Device (input)
 *      Handle for the Device instance returned by Device_Find.
 *
 * Settings_p (input)
 *      Pointer to the data structure that contains device configuration
 *      parameters.
 *
 * This function is NOT re-entrant for the same Device.
 * This function is re-entrant for different Devices.
 *
 * Return value
 *      EIP164_NO_ERROR : Operation is completed.
 *      EIP164_UNSUPPORTED_FEATURE_ERROR : Not supported by the device.
 *      EIP164_ARGUMENT_ERROR : Passed wrong argument.
 *      EIP164_BUSY_RETRY_LATER : Device initialization is not completed yet.
 *      EIP164_HW_CONFIGURATION_MISMATCH : HW configuration does not support
 *                                        requested operation.
 */
EIP164_Error_t
EIP164_Device_Init(
        EIP164_IOArea_t * const IOArea_p,
        const Device_Handle_t Device,
        const EIP164_Settings_t * const Settings_p);


/*----------------------------------------------------------------------------
 * EIP164_Device_Is_Init_Done
 *
 * This function completes the initialization of an EIP-164 device.
 *
 * API use order:
 *      This function should be called after the EIP164_Device_Init() function
 *      returned EIP164_BUSY_RETRY_LATER. This function should be called
 *      repeatedly until it returns EIP164_NO_ERROR. Only then the device
 *      initialization can be considered completed and other API functions may
 *      be called for this device.
 *
 * IOArea_p (input)
 *      Pointer to the Device instance I/O area in memory that identifies the
 *      EIP-164 device to be used.
 *
 * This function is NOT re-entrant for the same Device.
 * This function is re-entrant for different Devices.
 *
 * Return value
 *      EIP164_NO_ERROR : Operation is completed.
 *      EIP164_ARGUMENT_ERROR : Passed wrong argument.
 *      EIP164_BUSY_RETRY_LATER : Device initialization is not completed yet.
 */
EIP164_Error_t
EIP164_Device_Is_Init_Done(
        const EIP164_IOArea_t * const IOArea_p);


/*----------------------------------------------------------------------------
 * EIP164_Device_Uninit
 *
 * This function starts the uninitialization of an EIP-164 device.
 *
 * API use order:
 *      After this function is executed none of the EIP164_*() functions
 *      can be called for this device. If this function returns
 *      EIP164_BUSY_RETRY_LATER then the EIP164_Device_Is_Init_Done() should
 *      be called afterwards until it returns EIP164_NO_ERROR. Only then
 *      the device uninitialization can be considered completed and
 *      the other API functions may be called for this device.
 *
 * IOArea_p (input/output)
 *      Pointer to the Device instance I/O area in memory that is used to
 *      identify the EIP-164 device.
 *
 * This function is NOT re-entrant for the same Device.
 * This function is re-entrant for different Devices.
 *
 * Return value
 *      EIP164_NO_ERROR : Operation is completed.
 *      EIP164_ARGUMENT_ERROR : Passed wrong argument.
 *      EIP164_BUSY_RETRY_LATER : Device uninitialization is not completed yet.
 */
EIP164_Error_t
EIP164_Device_Uninit(
        EIP164_IOArea_t * const IOArea_p);


/*----------------------------------------------------------------------------
 * EIP164_HWRevision_Get
 *
 * This function retrieves EIP-164 device capabilities and version information.
 *
 * Device (input)
 *      Handle for the Device instance returned by Device_Find.
 *
 * Capabilities_p (output)
 *      Pointer to the data structure where the capabilities and version
 *      parameters will be stored
 *
 * This function is re-entrant for any Device.
 *
 * This function can be called concurrently with any other EIP164_CfyE_*()
 * or EIP164_SecY_*() API function for the same or different Device.
 *
 * Return value:
 *      EIP164_NO_ERROR : Operation is completed.
 *      EIP164_UNSUPPORTED_FEATURE_ERROR : Not supported by the device.
 *      EIP164_ARGUMENT_ERROR : Passed wrong argument.
 */
EIP164_Error_t
EIP164_HWRevision_Get(
        const Device_Handle_t Device,
        EIP164_Capabilities_t * const Capabilities_p);


/*----------------------------------------------------------------------------
 * EIP164_Device_Sync
 *
 * This function starts the host synchronization with an EIP-164 device.
 * When this function is called it makes the EIP-164 device to remember
 * the number of packets currently available in its pipeline.
 * The synchronization is considered completed when all these remembered
 * packets are processed. This does not mean, however, that the processing
 * of other packets does not start.
 *
 * API use order:
 *      This function should be called before the EIP-164 SA, SecY, IFC or
 *      IFC1 statistics can be read by the host. If this function returns
 *      EIP164_BUSY_RETRY_LATER then the EIP164_SecY_Is_Sync_Done() should
 *      be called afterwards until it returns EIP164_NO_ERROR. Only then the
 *      device synchronization with the host can be considered completed and
 *      other API functions may be called for this device.
 *
 * IOArea_p (input/output)
 *      Pointer to the Device instance I/O area in memory that identifies the
 *      EIP-164 device to be used.
 *
 * This function is NOT re-entrant for the same Device.
 * This function is re-entrant for different Devices.
 *
 * This function cannot be called concurrently with the function
 * EIP164_SecY_Device_Is_Sync_Done() for the same Device.
 * This function can be called concurrently with the function
 * EIP164_SecY_Device_Is_Sync_Done() for the different Devices or other
 * functions except EIP164_SecY_Device_Is_Sync_Done() for the same Device.
 *
 * Return value
 *      EIP164_NO_ERROR : Operation is completed.
 *      EIP164_ARGUMENT_ERROR : Passed wrong argument.
 *      EIP164_BUSY_RETRY_LATER : Device synchronization is not completed yet.
 */
EIP164_Error_t
EIP164_Device_Sync(
        EIP164_IOArea_t * const IOArea_p);


/*----------------------------------------------------------------------------
 * EIP164_Device_Is_Sync_Done
 *
 * This function completes the host synchronization with an EIP-164 device.
 *
 * API use order:
 *      This function should be called after the EIP164_SecY_Device_Sync()
 *      function returned EIP164_BUSY_RETRY_LATER. It should be called
 *      repeatedly until it returns EIP164_NO_ERROR. Only then the device
 *      synchronization with can be considered completed and the SA, SecY, IFC
 *      or IFC1 statistics can be read by the host.
 *
 * IOArea_p (input/output)
 *      Pointer to the Device instance I/O area in memory that identifies the
 *      EIP-164 device to be used.
 *
 * This function is NOT re-entrant for the same Device.
 * This function is re-entrant for different Devices.
 *
 * This function cannot be called concurrently with the function
 * EIP164_SecY_Device_Sync() for the same Device.
 * This function can be called concurrently with the EIP164_SecY_Device_Sync()
 * function for different Devices or other functions except
 * EIP164_SecY_Device_Sync() for the same Device.
 *
 * Return value
 *     EIP164_NO_ERROR : Operation is completed.
 *     EIP164_ARGUMENT_ERROR : Passed wrong argument.
 *     EIP164_BUSY_RETRY_LATER : Device synchronization is not completed yet.
 */
EIP164_Error_t
EIP164_Device_Is_Sync_Done(
        EIP164_IOArea_t * const IOArea_p);


/*----------------------------------------------------------------------------
 * EIP164_Device_Restore
 *
 * This function restores the IOArea without altering the (hardware) state
 * of the device. Call this function instead of EIP164_Device_Init when
 * the software is restarted, but the device is still in a well-defined state.
 *
 * Note: This function should be called after the EIP-164 SW Reset.
 *
 * API use order:
 *      This function must be executed before any of the EIP164_*() functions
 *      can be called after the SW reset except the EIP164_HWRevision_Get()
 *      function.
 *
 * IOArea_p (output)
 *      Pointer to the Device instance I/O area in memory that will be used to
 *      identify the EIP-164 device.
 *
 * Device (input)
 *      Handle for the Device instance returned by Device_Find.
 *
 * Mode (input)
 *      Mode (egress, ingress, combined) that the device is in.
 *
 * This function is re-entrant for the same or different Devices.
 *
 * Return value:
 *      EIP164_NO_ERROR : Operation is completed
 *      EIP164_UNSUPPORTED_FEATURE_ERROR : Not supported by the device.
 *      EIP164_ARGUMENT_ERROR : Passed wrong argument.
 */
EIP164_Error_t
EIP164_Device_Restore(
        EIP164_IOArea_t * const IOArea_p,
        const Device_Handle_t Device,
        const EIP164_Device_Mode_t Mode);


/*----------------------------------------------------------------------------
 * EIP164_Device_ChannelControl_Read
 *
 * This function reads the control setting for the specified channel of an
 * EIP-164 device.
 *
 * IOArea_p (input)
 *      Pointer to the Device instance I/O area in memory that identifies the
 *      EIP-164 device to be used.
 *
 * ChannelId (input)
 *      The channel index (number) to read the control setting from.
 *
 * Latency_p (output)
 *      Pointer to the channel-specific latency value.
 *
 * BurstLimit_p (output)
 *      Pointer to the channel-specific burst limit value.
 *
 * fLatencyEnable_p (output)
 *      Pointer to the indication if the channel-specific latency value
 *      is applied for the channel identified by ChannelId.
 *
 * fLowLatencyBypass (output)
 *      Indication if the low latency bypass must be enabled (true)
 *      or the MACsec mode (false).
 *
 * Return value
 *      EIP164_NO_ERROR : Operation is completed.
 *      EIP164_ARGUMENT_ERROR : Passed wrong argument.
 */
EIP164_Error_t
EIP164_Device_ChannelControl_Read(
        const EIP164_IOArea_t * const IOArea_p,
        const unsigned int ChannelId,
        uint16_t * const Latency_p,
        uint8_t * const BurstLimit_p,
        bool * const fLatencyEnable_p,
        bool * const fLowLatencyBypass_p);


/*----------------------------------------------------------------------------
 * EIP164_Device_ChannelControl_Write
 *
 * This function writes the control setting for the specified channel of an
 * EIP-164 device.
 *
 * IOArea_p (input)
 *      Pointer to the Device instance I/O area in memory that identifies the
 *      EIP-164 device to be used.
 *
 * ChannelId (input)
 *      The channel index (number) to write the control setting from.
 *
 * Latency (input)
 *      Channel-specific latency value for MACsec channels, e.g. channels
 *      with fLowLatencyBypass set to false. Ignored if fLowLatencyBypass is
 *      set to true for this channel.
 *
 * BurstLimit (input)
 *      Channel-specific burst limit value.
 *
 * fLatencyEnable (input)
 *      Indication if the channel-specific latency value must be applied
 *      for the channel identified by ChannelId.
 *
 * fLowLatencyBypass (input)
 *      Indication if the low latency bypass must be enabled (true)
 *      or the MACsec mode (false).
 *
 *
 * Return value
 *      EIP164_NO_ERROR : Operation is completed.
 *      EIP164_ARGUMENT_ERROR : Passed wrong argument.
 */
EIP164_Error_t
EIP164_Device_ChannelControl_Write(
        const EIP164_IOArea_t * const IOArea_p,
        const unsigned int ChannelId,
        const uint16_t Latency,
        const uint8_t BurstLimit,
        const bool fLatencyEnable,
        const bool fLowLatencyBypass);


/*----------------------------------------------------------------------------
 * EIP164_Device_Channel_CtxControl_Write
 *
 * This function writes the context control settings for the specified channel
 * of an EIP-164 device.
 *
 * IOArea_p (input)
 *      Pointer to the Device instance I/O area in memory that identifies the
 *      EIP-164 device to be used.
 *
 * ChannelId (input)
 *      The channel index (number) to write the control setting from.
 *
 * fPktNumThrMode (input)
 *      Mode for packet number threshold comparison:
 *          false - comparison is greater or equal.
 *          true  - comparison is strictly equal.
 *
 * EtherType (input)
 *      EtherType field for MACsec to be inserted in the SecTAG for outbound
 *      MACsec packet processing. Default (reset) value is 0x88E5.
 *
 * Return value
 *      EIP164_NO_ERROR : Operation is completed.
 *      EIP164_ARGUMENT_ERROR : Passed wrong argument.
 */
EIP164_Error_t
EIP164_Device_Channel_CtxControl_Write(
        const EIP164_IOArea_t * const IOArea_p,
        const unsigned int ChannelId,
        const bool fPktNumThrMode,
        const uint16_t EtherType);


/*----------------------------------------------------------------------------
 * EIP164_Device_Channel_CtxControl_Read
 *
 * This function reads the context control settings for the specified channel
 * of an EIP-164 device.
 *
 * IOArea_p (input)
 *      Pointer to the Device instance I/O area in memory that identifies the
 *      EIP-164 device to be used.
 *
 * ChannelId (input)
 *      The channel index (number) to read the control setting from.
 *
 * fPktNumThrMode_p (output)
 *      Mode for packet number threshold comparison
 *          false - comparison is greater or equal
 *          true  - comparison is strictly equal
 *
 * EtherType_p (output)
 *      EtherType field for MACsec to be inserted in the SecTAG for outbound
 *      MACsec packet processing. Default (reset) value is 0x88E5.
 * Return value
 *      EIP164_NO_ERROR : Operation is completed.
 *      EIP164_ARGUMENT_ERROR : Passed wrong argument.
 */
EIP164_Error_t
EIP164_Device_Channel_CtxControl_Read(
        const EIP164_IOArea_t * const IOArea_p,
        const unsigned int ChannelId,
        bool * const fPktNumThrMode_p,
        uint16_t *const EtherType_p);


/*----------------------------------------------------------------------------
 * EIP164_Device_PN_Threshold_Control_Write
 *
 * This function writes the statistics control settings for the specified
 * channel of an EIP-164 device.
 *
 * IOArea_p (input)
 *      Pointer to the Device instance I/O area in memory that identifies the
 *      EIP-164 device to be used.
 *
 * ChannelId (input)
 *      The channel index (number) to write the control setting from.
 *
 * PN_Threshold_p (input)
 *      Pointer to the memory location where the statistics control settings
 *      are stored for channel specified by ChannelId.
 *
 * Return value
 *      EIP164_NO_ERROR : Operation is completed.
 *      EIP164_ARGUMENT_ERROR : Passed wrong argument.
 */
EIP164_Error_t
EIP164_Device_PN_Threshold_Control_Write(
    const EIP164_IOArea_t * const IOArea_p,
    const unsigned int ChannelId,
    const EIP164_PN_Threshold_t * const PN_Threshold_p);


/*----------------------------------------------------------------------------
 * EIP164_Device_PN_Threshold_Control_Read
 *
 * This function reads the statistics control settings for the specified
 * channel of an EIP-164 device.
 *
 * IOArea_p (input)
 *      Pointer to the Device instance I/O area in memory that identifies the
 *      EIP-164 device to be used.
 *
 * ChannelId (input)
 *      The channel index (number) to read the control setting from.
 *
 * PN_Threshold_p (input)
 *      Pointer to the memory location where the statistics control settings
 *      are stored for channel specified by ChannelId.
 *
 * Return value
 *      EIP164_NO_ERROR : Operation is completed.
 *      EIP164_ARGUMENT_ERROR : Passed wrong argument.
 */
EIP164_Error_t
EIP164_Device_PN_Threshold_Control_Read(
    const EIP164_IOArea_t * const IOArea_p,
    const unsigned int ChannelId,
    EIP164_PN_Threshold_t * const PN_Threshold_p);


/*----------------------------------------------------------------------------
 * EIP164_Device_Channel_PacketsInFlight_Read
 *
 * This function reads which channels have packets in flight (are processing
 * packets) for the specified EIP-164 device.
 *
 * IOArea_p (input)
 *      Pointer to the Device instance I/O area in memory that identifies the
 *      EIP-164 device to be used.
 *
 * ChannelId (input)
 *      The channel index (number) to read the control setting from.
 *
 * InFlight_p (output)
 *      Pointer to an indication in which the in flight packets information
 *      must be returned. Each bit represents if channel that is still
 *      processing packets and false if the channel is idle.
 *
 * Return value
 *      EIP164_NO_ERROR : Operation is completed.
 *      EIP164_ARGUMENT_ERROR : Passed wrong argument.
 */
EIP164_Error_t
EIP164_Device_Channel_PacketsInFlight_Read(
        const EIP164_IOArea_t * const IOArea_p,
        const unsigned int ChannelId,
        uint32_t * const InFlight_p);


/*----------------------------------------------------------------------------
 * EIP164_Device_Clocks_On
 *
 * This function switches the specified clocks of the EIP-164 device on.
 *
 * IOArea_p (input)
 *      Pointer to the Device instance I/O area in memory that identifies the
 *      EIP-164 device to be used.
 *
 * Mask (input)
 *      Mask that specifies the clocks to be switched off.
 *      See EIP164_SECY_DEVICE_*_CLOCK for clock signal details.
 *
 * fAlwaysOn (input)
 *      If true then the speficied by the ClockMask clocks will be always
 *      enabled. If false then the clocks will be enabled only when needed,
 *      the device will manage the clocks dynamically.
 *
 * This function is NOT re-entrant for the same Device.
 * This function is re-entrant for different Devices.
 *
 * Return value
 *      EIP164_NO_ERROR : Operation is completed.
 *      EIP164_ARGUMENT_ERROR : Passed wrong argument.
 */
EIP164_Error_t
EIP164_Device_Clocks_On(
        const EIP164_IOArea_t * const IOArea_p,
        const uint32_t ClockMask,
        const bool fAlwaysOn);


/*----------------------------------------------------------------------------
 * EIP164_Device_Clocks_Off
 *
 * This function switches the specified clocks of the EIP-164 device off.
 *
 * IOArea_p (input)
 *      Pointer to the Device instance I/O area in memory that identifies the
 *      EIP-164 device to be used.
 *
 * Mask (input)
 *      Mask that specifies the clocks to be switched off.
 *      See EIP164_SECY_DEVICE_*_CLOCK for clock signal details.
 *
 * This function is NOT re-entrant for the same Device.
 * This function is re-entrant for different Devices.
 *
 * Return value
 *      EIP164_NO_ERROR : Operation is completed.
 *      EIP164_ARGUMENT_ERROR : Passed wrong argument.
 */
EIP164_Error_t
EIP164_Device_Clocks_Off(
        const EIP164_IOArea_t * const IOArea_p,
        const uint32_t ClockMask);


/*----------------------------------------------------------------------------
 * EIP164_Device_EOP_Configure
 *
 * Configure automatic EOP configuration
 *
 * IOArea_p (input)
 *      Pointer to the Device instance I/O area in memory that identifies the
 *      EIP-164 device to be used.
 *
 * EOPTimeoutVal (input)
 *      Timeout value for automatic EOP insertion.
 *
 * EOPTimeoutCtrl_p (input)
 *      Bit mask that specifies for which channels EOP insertion is applied.
 *      A '1' in each bit enables automatic EOP insertion in the corresponding
 *      channel.
 *
 * This function is NOT re-entrant for the same Device.
 * This function is re-entrant for different Devices.
 * This function cannot be called concurrently with
 * EIP164_Device_EOP_Confgiuration_Get() for the same device.
 * This function can be called concurrently with any other function for
 * the same or different Devices.
 *
 * Return value
 *      EIP164_NO_ERROR : Operation is completed.
 *      EIP164_ARGUMENT_ERROR : Passed wrong argument.
 */
EIP164_Error_t
EIP164_Device_EOP_Configure(
        const EIP164_IOArea_t * const IOArea_p,
        const unsigned int EOPTimeoutVal,
        const EIP164_Ch_Mask_t * const EOPTimeoutCtrl_p);


/*----------------------------------------------------------------------------
 * EIP164_Device_EOP_Configuration_Get
 *
 * Read automatic EOP configuration
 *
 * IOArea_p (input)
 *      Pointer to the Device instance I/O area in memory that identifies the
 *      EIP-164 device to be used.
 *
 * EOPTimeoutVal_p (output)
 *      Timeout value for automatic EOP insertion.
 *
 * EOPTimeoutCtrl_p (output)
 *      Bit mask that specifies for which channels EOP insertion is applied.
 *      A '1' in each bit enables automatic EOP insertion in the corresponding
 *      channel.
 *
 * This function is re-entrant for the same or different Devices.
 * This function cannot be called concurrently with
 * EIP164_Device_EOP_Confgiure() for the same device.
 * This function can be called concurrently with any other function for
 * the same or different Devices.
 *
 * Return value
 *      EIP164_NO_ERROR : Operation is completed.
 *      EIP164_ARGUMENT_ERROR : Passed wrong argument.
 */
EIP164_Error_t
EIP164_Device_EOP_Configuration_Get(
        const EIP164_IOArea_t * const IOArea_p,
        unsigned int * const EOPTimeoutVal_p,
        EIP164_Ch_Mask_t * const EOPTimeoutCtrl_p);


/*----------------------------------------------------------------------------
 * EIP164_Device_InsertSOP
 *
 * Insert an SOP signal on one or more channels.
 *
 * IOArea_p (input)
 *      Pointer to the Device instance I/O area in memory that identifies the
 *      EIP-164 device to be used.
 *
 * ChannelMask_p (input)
 *      Bit mask that specifies for which channels SOP insertion is applied.
 *      A '1' in each bit inserts an SOP in the corresponding channel.
 *
 * This function is re-entrant for the same or different Devices.
 * This function can be called concurrently with any other function for
 * the same or different Devices.
 *
 * Return value
 *      EIP164_NO_ERROR : Operation is completed.
 *      EIP164_ARGUMENT_ERROR : Passed wrong argument.
 */
EIP164_Error_t
EIP164_Device_InsertSOP(
        const EIP164_IOArea_t * const IOArea_p,
        const EIP164_Ch_Mask_t * const ChannelMask_p);


/*----------------------------------------------------------------------------
 * EIP164_Device_InsertEOP
 *
 * Insert an SOP signal on one or more channels.
 *
 * IOArea_p (input)
 *      Pointer to the Device instance I/O area in memory that identifies the
 *      EIP-164 device to be used.
 *
 * ChannelMask_p (input)
 *      Bit mask that specifies for which channels EOP insertion is applied.
 *      A '1' in each bit inserts an EOP in the corresponding channel.
 *
 * This function is re-entrant for the same or different Devices.
 * This function can be called concurrently with any other function for
 * the same or different Devices.
 *
 * Return value
 *      EIP164_NO_ERROR : Operation is completed.
 *      EIP164_ARGUMENT_ERROR : Passed wrong argument.
 */
EIP164_Error_t
EIP164_Device_InsertEOP(
        const EIP164_IOArea_t * const IOArea_p,
        const EIP164_Ch_Mask_t * const ChannelMask_p);


/*----------------------------------------------------------------------------
 * EIP164_Device_ECC_Configure
 *
 * Configure ECC error checking.
 *
 * IOArea_p (input)
 *      Pointer to the Device instance I/O area in memory that identifies the
 *      EIP-164 device to be used.
 *
 * CorrectableThr (input)
 *      Threshold for the number of correctable errors to be reported as an
 *      event. Set to 0 to disable reporting an event.
 *
 * UncorrectableThr (input)
 *      Threshold for the number of uncorrectable errors to be reported as an
 *      event. Set to 0 to disable reporting an event.
 *
 * This function is NOT re-entrant for the same Device.
 * This function is re-entrant for different Devices.
 *
 * This function cannot be called concurrently with the function
 * EIP164_Device_ECCStatus_Get() or EIP164_Device_ECC_Configuration_Get()
 * for the same Device.
 * This function can be called concurrently with
 * the EIP164_Device_ECCStatus_Get() function or other functions for different
 * Devices.
 *
 * Return value
 *      EIP164_NO_ERROR : Operation is completed.
 *      EIP164_ARGUMENT_ERROR : Passed wrong argument.
 */
EIP164_Error_t
EIP164_Device_ECC_Configure(
        const EIP164_IOArea_t * const IOArea_p,
        const unsigned int CorrectableThr,
        const unsigned int UncorrectableThr);


/*----------------------------------------------------------------------------
 * EIP164_Device_ECC_Configuration_Get
 *
 * Read the ECC error checking configuration.
 *
 * IOArea_p (input)
 *      Pointer to the Device instance I/O area in memory that identifies the
 *      EIP-164 device to be used.
 *
 * CorrectableThr_p (output)
 *      Threshold for the number of correctable errors to be reported as an
 *      event. Set to 0 to disable reporting an event.
 *
 * UncorrectableThr_p (output)
 *      Threshold for the number of uncorrectable errors to be reported as an
 *      event. Set to 0 to disable reporting an event.
 *
 * This function is re-entrant for the same or different Devices.
 * This function cannot be called concurrently with the function
 * EIP164_Device_ECCS_Configure() for the same Device.
 *
 * This function cannot be called concurrently with the function
 * EIP164_Device_ECCStatus_Get() for the same Device.
 * This function can be called concurrently with
 * the EIP164_Device_ECCStatus_Get() function or other functions for different
 * Devices.
 *
 * Return value
 *      EIP164_NO_ERROR : Operation is completed.
 *      EIP164_ARGUMENT_ERROR : Passed wrong argument.
 */
EIP164_Error_t
EIP164_Device_ECC_Configuration_Get(
        const EIP164_IOArea_t * const IOArea_p,
        unsigned int * const CorrectableThr_p,
        unsigned int * const UncorrectableThr_p);


/*----------------------------------------------------------------------------
 * EIP164_Device_ECCStatus_Get
 *
 * Read the number of correctable and uncorrectable ECC errors and clear
 * the counters.
 *
 * IOArea_p (input)
 *      Pointer to the Device instance I/O area in memory that identifies the
 *      EIP-164 device to be used.
 *
 * ECCStatus_p (output)
 *      Array of ECC status structures, one for each RAM for which
 *      ECC statistics are kept. The number of items in the array is
 *      EIP164_ECC_NOF_STATUS_COUNTERS.
 *
 * This function is re-entrant for the same or different Devices.
 *
 * This function cannot be called concurrently with the function
 * EIP164_Device_ECC_Configure() for the same Device.
 * This function can be called concurrently with
 * the EIP164_Device_ECC_Configure() function or other functions for different
 * Devices.
 *
 * Return value
 *      EIP164_NO_ERROR : Operation is completed.
 *      EIP164_ARGUMENT_ERROR : Passed wrong argument.
 */
EIP164_Error_t
EIP164_Device_ECCStatus_Get(
        const EIP164_IOArea_t * const IOArea_p,
        EIP164_Device_ECCStatus_t * const ECCStatus_p);


/*----------------------------------------------------------------------------
 * EIP164_Device_PktProcessDebug_Get
 *
 * Read the debug registers related to packet classification in the EIP164.
 *
 * IOArea_p (input)
 *      Pointer to the Device instance I/O area in memory that identifies the
 *      EIP-164 device to be used.
 *
 * PktProcessDebug_p (output)
 *      Structure containing the contents of the debug registers.
 *
 * This function is re-entrant for the same or different Devices.
 *
 * This function can be called concurrently with any other functions
 * for the same ordifferent Devices.
 *
 * Return value
 *      EIP164_NO_ERROR : Operation is completed.
 *      EIP164_ARGUMENT_ERROR : Passed wrong argument.
 */
EIP164_Error_t
EIP164_Device_PktProcessDebug_Get(
        const EIP164_IOArea_t * const IOArea_p,
        EIP164_Device_PktProcessDebug_t * const PktProcessDebug_p);


/*----------------------------------------------------------------------------
 * EIP164_Device_SecY_Statistics_Enable
 *
 * This function enables or disables the SecY statistics counters for an
 * EIP-164 device. Note that the statistics counters are enabled by default.
 *
 * IOArea_p (input)
 *      Pointer to the Device instance I/O area in memory that identifies the
 *      EIP-164 device to be used.
 *
 * Mask (input)
 *      Mask that enables the SecY statistics counters. Mask for
 *      - Egress:
 *          BIT 0    TransformErrPkts
 *          BIT 1    OutPktsCtrl
 *          BIT 2    OutPktsUntagged
 *      - Ingress:
 *          BIT 0    TransformErrPkts
 *          BIT 1    InPktsCtrl
 *          BIT 2    InPktsUntagged
 *          BIT 3    InPktsNoTag
 *          BIT 4    InPktsBadTag
 *          BIT 5    InPktsNoSCI
 *          BIT 6    InPktsUnknownSCI
 *          BIT 7    InPktsTaggedCtrl
 *
 * This function is re-entrant for the same or different Devices.
 * This function can be called concurrently with any other function for
 * the same or different Devices.
 *
 * Return value
 *      EIP164_NO_ERROR : Operation is completed.
 *      EIP164_ARGUMENT_ERROR : Passed wrong argument.
 */
EIP164_Error_t
EIP164_Device_SecY_Statistics_Enable(
        const EIP164_IOArea_t * const IOArea_p,
        const uint32_t Mask);


/*----------------------------------------------------------------------------
 * EIP164_Device_SecY_Statistics_Enabled_Get
 *
 * This function reads the enabled state of the SecY statistics
 * counters for an EIP-164 device.
 *
 * IOArea_p (input)
 *      Pointer to the Device instance I/O area in memory that identifies the
 *      EIP-164 device to be used.
 *
 * Mask_p (output)
 *      Mask that enables the SecY statistics counters. Mask for
 *      - Egress:
 *          BIT 0    TransformErrPkts
 *          BIT 1    OutPktsCtrl
 *          BIT 2    OutPktsUntagged
 *      - Ingress:
 *          BIT 0    TransformErrPkts
 *          BIT 1    InPktsCtrl
 *          BIT 2    InPktsUntagged
 *          BIT 3    InPktsNoTag
 *          BIT 4    InPktsBadTag
 *          BIT 5    InPktsNoSCI
 *          BIT 6    InPktsUnknownSCI
 *          BIT 7    InPktsTaggedCtrl
 *
 * This function is re-entrant for the same or different Devices.
 * This function can be called concurrently with any other function for
 * the same or different Devices.
 *
 * Return value
 *      EIP164_NO_ERROR : Operation is completed.
 *      EIP164_ARGUMENT_ERROR : Passed wrong argument.
 */
EIP164_Error_t
EIP164_Device_SecY_Statistics_Enabled_Get(
        const EIP164_IOArea_t * const IOArea_p,
        uint32_t * const Mask_p);


/*----------------------------------------------------------------------------
 * EIP164_Device_SA_Statistics_Enable
 *
 * This function enables or disables the SA statistics counters for an
 * EIP-164 device. Note that the statistics counters are enabled by default.
 *
 * IOArea_p (input)
 *      Pointer to the Device instance I/O area in memory that identifies the
 *      EIP-164 device to be used.
 *
 *  Mask (input)
 *     Mask that enables the SA statistics counters. Mask for
 *     - Egress:
 *         BIT 0    OutOctetsEncrypted/OutOctetsProtected
 *         BIT 1    OutPktsEncrypted/OutPktsProtected.
 *         BIT 2    OutPktsTooLong
 *         BIT 3    OutPktSANotInUse
 *     - Ingress:
 *         BIT 0    InOctetsDecrypted
 *         BIT 1    InOctetsValidated
 *         BIT 2    InPktsUnchecked
 *         BIT 3    InPktsDelayed
 *         BIT 4    InPktsDelayed
 *         BIT 5    InPktsOk
 *         BIT 6    InPktsInvalid
 *         BIT 7    InPktsNotValid
 *         BIT 8    InPktsNotUsingSA
 *         BIT 9    InPktsUnusedSA
 *
 * This function is re-entrant for the same or different Devices.
 * This function can be called concurrently with any other function for
 * the same or different Devices.
 *
 * Return value
 *     EIP164_NO_ERROR : Operation is completed.
 *     EIP164_ARGUMENT_ERROR : Passed wrong argument.
 */
EIP164_Error_t
EIP164_Device_SA_Statistics_Enable(
        const EIP164_IOArea_t * const IOArea_p,
        const uint32_t Mask);


/*----------------------------------------------------------------------------
 * EIP164_Device_SA_Statistics_Enabled_Get
 *
 * This function reads the enabled state of the SA statistics counters
 * for an EIP-164 device.
 *
 * IOArea_p (input)
 *      Pointer to the Device instance I/O area in memory that identifies the
 *      EIP-164 device to be used.
 *
 *  Mask_p (output)
 *     Mask that enables the SA statistics counters. Mask for
 *     - Egress:
 *         BIT 0    OutOctetsEncrypted/OutOctetsProtected
 *         BIT 1    OutPktsEncrypted/OutPktsProtected.
 *         BIT 2    OutPktsTooLong
 *         BIT 3    OutPktSANotInUse
 *     - Ingress:
 *         BIT 0    InOctetsDecrypted
 *         BIT 1    InOctetsValidated
 *         BIT 2    InPktsUnchecked
 *         BIT 3    InPktsDelayed
 *         BIT 4    InPktsDelayed
 *         BIT 5    InPktsOk
 *         BIT 6    InPktsInvalid
 *         BIT 7    InPktsNotValid
 *         BIT 8    InPktsNotUsingSA
 *         BIT 9    InPktsUnusedSA
 *
 * This function is re-entrant for the same or different Devices.
 * This function can be called concurrently with any other function for
 * the same or different Devices.
 *
 * Return value
 *     EIP164_NO_ERROR : Operation is completed.
 *     EIP164_ARGUMENT_ERROR : Passed wrong argument.
 */
EIP164_Error_t
EIP164_Device_SA_Statistics_Enabled_Get(
        const EIP164_IOArea_t * const IOArea_p,
        uint32_t * const  Mask_p);


/*----------------------------------------------------------------------------
 * EIP164_Device_IFC_Statistics_Enable
 *
 * This function enables or disables the IFC statistics counters for an
 * EIP-164 device. Note that the statistics counters are enabled by default.
 *
 * IOArea_p (input)
 *      Pointer to the Device instance I/O area in memory that identifies the
 *      EIP-164 device to be used.
 *
 * Mask (input)
 *      Mask that enables the IFC statistics counters. Mask for
 *      - Egress:
 *          BIT 0    ifOutOctetsUncontrolled
 *          BIT 1    ifOutOctetsControlled
 *          BIT 2    ifOutUcastPktsUncontrolled
 *          BIT 3    ifOutMulticastPktsUncontrolled
 *          BIT 4    ifOutBroadcastPktsUncontrolled
 *      - Ingress:
 *          BIT 0    ifInOctetsUncontrolled
 *          BIT 1    ifInUcastPktsUncontrolled
 *          BIT 2    ifInMulticastPktsUncontrolled
 *          BIT 3    ifInBroadcastPktsUncontrolled
 *
 * This function is re-entrant for the same or different Devices.
 * This function can be called concurrently with any other function for
 * the same or different Devices.
 *
 * Return value
 *      EIP164_NO_ERROR : Operation is completed.
 *      EIP164_ARGUMENT_ERROR : Passed wrong argument.
 */
EIP164_Error_t
EIP164_Device_IFC_Statistics_Enable(
        const EIP164_IOArea_t * const IOArea_p,
        const uint32_t Mask);


/*----------------------------------------------------------------------------
 * EIP164_Device_IFC_Statistics_Enabled_Get
 *
 * This function reads the enabled state of the IFC statistics
 * counters for an EIP-164 device.
 *
 * IOArea_p (input)
 *      Pointer to the Device instance I/O area in memory that identifies the
 *      EIP-164 device to be used.
 *
 * Mask_p (output)
 *      Mask that enables the IFC statistics counters. Mask for
 *      - Egress:
 *          BIT 0    ifOutOctetsUncontrolled
 *          BIT 1    ifOutOctetsControlled
 *          BIT 2    ifOutUcastPktsUncontrolled
 *          BIT 3    ifOutMulticastPktsUncontrolled
 *          BIT 4    ifOutBroadcastPktsUncontrolled
 *      - Ingress:
 *          BIT 0    ifInOctetsUncontrolled
 *          BIT 1    ifInUcastPktsUncontrolled
 *          BIT 2    ifInMulticastPktsUncontrolled
 *          BIT 3    ifInBroadcastPktsUncontrolled
 *
 * This function is re-entrant for the same or different Devices.
 * This function can be called concurrently with any other function for
 * the same or different Devices.
 *
 * Return value
 *      EIP164_NO_ERROR : Operation is completed.
 *      EIP164_ARGUMENT_ERROR : Passed wrong argument.
 */
EIP164_Error_t
EIP164_Device_IFC_Statistics_Enabled_Get(
        const EIP164_IOArea_t * const IOArea_p,
        uint32_t * const Mask_p);


/*----------------------------------------------------------------------------
 * EIP164_Device_IFC1_Statistics_Enable
 *
 * This function enables or disables the IFC1 statistics counters for an
 * EIP-164 device. Note that the statistics counters are enabled by default.
 *
 * IOArea_p (input)
 *      Pointer to the Device instance I/O area in memory that identifies the
 *      EIP-164 device to be used.
 *
 * Mask (input)
 *      Mask that enables the IFC1 statistics counters. Mask for
 *      - Egress:
 *          BIT 0    ifOutOctetsCommon
 *          BIT 1    ifOutUcastPktsControlled
 *          BIT 2    ifOutMulticastPktsControlled
 *          BIT 3    ifOutBroadcastPktsControlled
 *      - Ingress:
 *          BIT 0    ifInOctetsControlled
 *          BIT 1    ifInUcastPktsControlled
 *          BIT 2    ifInMulticastPktsControlled
 *          BIT 3    ifInBroadcastPktsControlled
 *
 * This function is re-entrant for the same or different Devices.
 * This function can be called concurrently with any other function for
 * the same or different Devices.
 *
 * Return value
 *      EIP164_NO_ERROR : Operation is completed.
 *      EIP164_ARGUMENT_ERROR : Passed wrong argument.
 */
EIP164_Error_t
EIP164_Device_IFC1_Statistics_Enable(
        const EIP164_IOArea_t * const IOArea_p,
        const uint32_t Mask);


/*----------------------------------------------------------------------------
 * EIP164_Device_IFC1_Statistics_Enabled_Get
 *
 * This function reads the enabled state of the IFC1 statistics
 * counters for an EIP-164 device.
 *
 * IOArea_p (input)
 *      Pointer to the Device instance I/O area in memory that identifies the
 *      EIP-164 device to be used.
 *
 * Mask_p (output)
 *      Mask that enables the IFC1 statistics counters. Mask for
 *      - Egress:
 *          BIT 0    ifOutOctetsCommon
 *          BIT 1    ifOutUcastPktsControlled
 *          BIT 2    ifOutMulticastPktsControlled
 *          BIT 3    ifOutBroadcastPktsControlled
 *      - Ingress:
 *          BIT 0    ifInOctetsControlled
 *          BIT 1    ifInUcastPktsControlled
 *          BIT 2    ifInMulticastPktsControlled
 *          BIT 3    ifInBroadcastPktsControlled
 *
 * This function is re-entrant for the same or different Devices.
 * This function can be called concurrently with any other function for
 * the same or different Devices.
 *
 * Return value
 *      EIP164_NO_ERROR : Operation is completed.
 *      EIP164_ARGUMENT_ERROR : Passed wrong argument.
 */
EIP164_Error_t
EIP164_Device_IFC1_Statistics_Enabled_Get(
        const EIP164_IOArea_t * const IOArea_p,
        uint32_t * const Mask_p);


/*----------------------------------------------------------------------------
 * EIP164_Device_RxCAM_Statistics_Enable
 *
 * This function enables or disables the RxCAM statistics counters for an
 * EIP-164 device (ingress only). Note that the statistics counters are
 * enabled by default.
 *
 * IOArea_p (input)
 *      Pointer to the Device instance I/O area in memory that identifies the
 *      EIP-164 device to be used.
 *
 * Mask (input)
 *      Mask that enables the RxCAM statistics counters. Mask for
 *      - Egress:    N/A
 *      - Ingress:
 *          BIT 0    RxCAMHit
 *
 * This function is re-entrant for the same or different Devices.
 * This function can be called concurrently with any other function for
 * the same or different Devices.
 *
 * Return value
 *      EIP164_NO_ERROR : Operation is completed.
 *      EIP164_ARGUMENT_ERROR : Passed wrong argument.
 */
EIP164_Error_t
EIP164_Device_RxCAM_Statistics_Enable(
        const EIP164_IOArea_t * const IOArea_p,
        const uint32_t Mask);


/*----------------------------------------------------------------------------
 * EIP164_Device_RxCAM_Statistics_Enabled_Get
 *
 * This function reads the enabled state of the RxCAM statistics
 * counters for an EIP-164 device (ingress only).
 *
 * IOArea_p (input)
 *      Pointer to the Device instance I/O area in memory that identifies the
 *      EIP-164 device to be used.
 *
 * Mask_p (output)
 *      Mask that enables the RxCAM statistics counters. Mask for
 *      - Egress:    N/A
 *      - Ingress:
 *          BIT 0    RxCAMHit
 *
 * This function is re-entrant for the same or different Devices.
 * This function can be called concurrently with any other function for
 * the same or different Devices.
 *
 * Return value
 *      EIP164_NO_ERROR : Operation is completed.
 *      EIP164_ARGUMENT_ERROR : Passed wrong argument.
 */
EIP164_Error_t
EIP164_Device_RxCAM_Statistics_Enabled_Get(
        const EIP164_IOArea_t * const IOArea_p,
        uint32_t * const Mask_p);


/*----------------------------------------------------------------------------
 * EIP164_StatisticsControl_Update
 *
 * This function updates the EIP-164 device statistics module settings.
 * It can be used to configure the SA/SecY/IFC/IFC1/RxCAM statistics
 * threshold global (stat_xxx_thr) and channel (stat_chan_thr) interrupts.
 *
 * API use order:
 *      If this function returns EIP164_BUSY_RETRY_LATER then
 *      the EIP164_Device_Is_Init_Done() should be called afterwards until it
 *      returns EIP164_NO_ERROR. Only then the statistics control update can be
 *      considered completed and  the other API functions may be called
 *      for this device.
 *
 * IOArea_p (input/output)
 *      Pointer to the Device instance I/O area in memory that is used to
 *      identify the EIP-164 device.
 *
 * StatCtrl_p (input)
 *      Pointer to the EIP-164 device statistics settings in memory.
 *
 * This function is NOT re-entrant for the same Device.
 * This function is re-entrant for different Devices.
 * This function cannot be called concurrently with
 * EiP164_StatisticsCoontrol_Read() for the same device.
 *
 * Return value
 *      EIP164_NO_ERROR : Operation is completed.
 *      EIP164_ARGUMENT_ERROR : Passed wrong argument.
 *      EIP164_BUSY_RETRY_LATER : Update is not completed yet.
 */
EIP164_Error_t
EIP164_StatisticsControl_Update(
        const EIP164_IOArea_t * const IOArea_p,
        const EIP164_Statistics_Control_t * const StatCtrl_p);


/*----------------------------------------------------------------------------
 * EIP164_StatisticsControl_Read
 *
 * This function reads the EIP-164 device statistics module settings.
 * It can be used to read out the SA/SecY/IFC/IFC1/RxCAM statistics
 * threshold global (stat_xxx_thr) and channel (stat_chan_thr) interrupts.
 *
 * IOArea_p (input/output)
 *      Pointer to the Device instance I/O area in memory that is used to
 *      identify the EIP-164 device.
 *
 * StatCtrl_p (output)
 *      Pointer to the EIP-164 device statistics settings in memory.
 *
 * This function is re-entrant for the same or different Devices.
 * This function cannot be called concurrently with
 * EiP164_StatisticsCoontrol_Update() for the same device.
 *
 * Return value
 *      EIP164_NO_ERROR : Operation is completed.
 *      EIP164_ARGUMENT_ERROR : Passed wrong argument.
 */
EIP164_Error_t
EIP164_StatisticsControl_Read(
        const EIP164_IOArea_t * const IOArea_p,
        EIP164_Statistics_Control_t * const StatCtrl_p);


/*----------------------------------------------------------------------------
 * EIP164_SecY_CryptAuth_BypassLen_Update
 *
 * Updates the length of the bypass data during crypt-authenticate operation.
 * The bypass data should contain the MAC header to be able to classify the
 * packet into the corresponding crypto-authenticate flow control register and
 * additionally it might include the IV used in the transformation.
 *
 * IOArea_p (input)
 *      Pointer to the Device instance I/O area in memory that identifies the
 *      EIP-164 device to be used.
 *
 * BypassLength (input)
 *      The length in bytes of the bypass data.
 *
 * This function is re-entrant for the same or different Devices.
 *
 * This function can be called concurrently with any other function for the
 * same or different Devices.
 *
 * Return value:
 *      EIP164_NO_ERROR : Operation is completed
 *      EIP164_ARGUMENT_ERROR : Passed wrong argument.
 */
EIP164_Error_t
EIP164_SecY_CryptAuth_BypassLen_Update(
        const EIP164_IOArea_t * const IOArea_p,
        const unsigned int BypassLength);


/*----------------------------------------------------------------------------
 * EIP164_SecY_CryptAuth_BypassLen_Get
 *
 * Reads the length of the bypass data during crypt-authenticate operation.
 * The bypass data should contain the MAC header to be able to classify the
 * packet into the corresponding crypto-authenticate flow control register and
 * additionally it might include the IV used in the transformation.
 *
 * IOArea_p (input)
 *      Pointer to the Device instance I/O area in memory that identifies the
 *      EIP-164 device to be used.
 *
 * BypassLength_p (output)
 *      The length in bytes of the bypass data.
 *
 * This function is re-entrant for the same or different Devices.
 *
 * This function can be called concurrently with any other function for the
 * same or different Devices.
 *
 * Return value:
 *      EIP164_NO_ERROR : Operation is completed
 *      EIP164_ARGUMENT_ERROR : Passed wrong argument.
 */
EIP164_Error_t
EIP164_SecY_CryptAuth_BypassLen_Get(
        const EIP164_IOArea_t * const IOArea_p,
        unsigned int * const BypassLength_p);

/*----------------------------------------------------------------------------
 * EIP164_SecY_SA_Write
 *
 * Writes the transform record of an SA to the device at a given index.
 *
 * API use order:
 *      This function must be called after EIP164_Device_Init(). It
 *      must be called before the SA is made available via
 *      EIP164_SecY_SC_SA_Map_E_Update() or
 *      EIP164_SecY_SC_SA_Map_I_Update().
 *
 * IOArea_p (input)
 *      Pointer to the Device instance I/O area in memory that identifies the
 *      EIP-164 device to be used.
 *
 * SAIndex (input)
 *      Index of SA to be written must be less than the maximum number of SA's
 *      supported by the device. The user must keep track of free and used SA's.
 *
 * Transform_p (input)
 *      Pointer to the memory location where the WordCount 32-bit words
 *      of the SA transform record are stored.
 *
 * WordCount (input)
 *      Number of words which must be written to the SA transform record.
 *
 * This function is NOT re-entrant for the same SA of the same Device.
 * This function is re-entrant for different Devices or different SA's
 * of the same Device.
 *
 * This function can be called concurrently with any other API function for
 * different SA's of the same or different Devices provided
 * the use order of the API functions is followed.
 *
 * Return value:
 *      EIP164_NO_ERROR : Operation is completed
 *      EIP164_ARGUMENT_ERROR : Passed wrong argument.
 */
EIP164_Error_t
EIP164_SecY_SA_Write(
        const EIP164_IOArea_t * const IOArea_p,
        const unsigned int SAIndex,
        const uint32_t * Transform_p,
        const unsigned int WordCount);


/*----------------------------------------------------------------------------
 * EIP164_SecY_SC_SA_Map_E_Update
 *
 * The function updates the Egress SC to SA mapping entry.
 *
 * API use order:
 *      The function can only be called after the SA was written with
 *      valid data using EIP164_SecY_SA_Write(). Also call
 *      EIP164_SecY_SAMFlow_Write() to make the SA active.
 *
 * IOArea_p (input)
 *      Pointer to the Device instance I/O area in memory that identifies the
 *      EIP-164 device to be used.
 *
 * SCIndex (input)
 *      Index of SC to be written must be less than the maximum number of SC's
 *      supported by the device. The user must keep track of free and used SC's.
 *
 * SAIndex (input)
 *      Index of SA to be written must be less than the maximum number of SA's
 *      supported by the device. The user must keep track of free and used SA's.
 *
 * fSAInUse (input)
 *      Mark the SA as valid (in use).
 *
 * fIndexUpdate (input)
 *      Update only the SA Index, not the sa_in_use flag.
 *
 * This function is NOT re-entrant for the same Device.
 * This function is re-entrant for different Devices.
 *
 * Return value:
 *      EIP164_NO_ERROR : Operation is completed
 *      EIP164_ARGUMENT_ERROR : Passed wrong argument.
 */
EIP164_Error_t
EIP164_SecY_SC_SA_Map_E_Update(
            const EIP164_IOArea_t * const IOArea_p,
            const unsigned int SCIndex,
            const unsigned int SAIndex,
            const bool fSAInUse,
            const bool fIndexUpdate);


/*----------------------------------------------------------------------------
 * EIP164_SecY_SC_SA_Map_I_Update
 *
 * The function updates the Ingress SC to SA mapping entry.
 *
 * API use order:
 *      If fSaInUse is true, the function can only be called after the SA
 *      was written with valid data using EIP164_SecY_SA_Write().
 *      This function, EIP164_SecY_RxCAM_Add() and EIP164_SecY_SAMFlow_Write()
 *      must all be called (in any order) to make an ingress SA active.
 *
 * IOArea_p (input)
 *      Pointer to the Device instance I/O area in memory that identifies the
 *      EIP-164 device to be used.
 *
 * SCIndex (input)
 *      Index of SC to be written must be less than the maximum number of SC's
 *      supported by the device. The user must keep track of free and used SC's.
 *
 * AN (input)
 *      Association number of the SA to be updated. Only applies to ingress
 *      configuration, where each SC maps to up to 4 AN's.
 *
 * SAIndex (input)
 *      Index of SA to be written must be less than the maximum number of SA's
 *      supported by the device. The user must keep track of free and used SA's.
 *
 * fSAInUse (input)
 *      Mark the SA as valid (in use).
 *
 * This function is NOT re-entrant for the same Device.
 * This function is re-entrant for different Devices.
 *
 * Return value:
 *      EIP164_NO_ERROR : Operation is completed
 *      EIP164_ARGUMENT_ERROR : Passed wrong argument.
 */
EIP164_Error_t
EIP164_SecY_SC_SA_Map_I_Update(
            const EIP164_IOArea_t * const IOArea_p,
            const unsigned int SCIndex,
            const unsigned int AN,
            const unsigned int SAIndex,
            const bool fSAInUse);


/*----------------------------------------------------------------------------
 * EIP164_SecY_RxCAM_Remove
 *
 * Remove entry from RxCAM
 *
 * API use order:
 *      The function can only be used for an RxCAM entry previously added with
 *      EIP164_SecY_RxCAM_Add().
 *
 * IOArea_p (input)
 *      Pointer to the Device instance I/O area in memory that identifies the
 *      EIP-164 device to be used.
 *
 * SCIndex (input)
 *      Index of SC to be written must be less than the maximum number of SC's
 *      supported by the device. The user must keep track of free and used SC's.
 *
 * This function is NOT re-entrant for the same Device.
 * This function is re-entrant for different Devices.
 *
 * This function cannot be called concurrently with function
 * EIP164_SecY_RxCAM_Read() or EIP164_SecY_RxCAM_Add() for the same
 * Device and the same SCIndex.
 *
 * For hardware with an Internal TCAM only, this function can be called
 * concurrently with the EIP164_SecY_RxCAM_Read() and
 * EIP164_SecY_RxCAM_Add() functions for the same Device and a
 * different SCIndex .
 *
 * This function can be called concurrently with the
 * EIP164_SecY_RxCAM_Read() and EIP164_SecY_RxCAM_Add() functions for
 * a different device.
 *
 * This function can be called concurrently with any other functions for the
 * same or different device.
 *
 * Return value:
 *      EIP164_NO_ERROR : Operation is completed
 *      EIP164_ARGUMENT_ERROR : Passed wrong argument.
 *      EIP164_BUSY_RETRY_LATER : Device initialization is not completed yet.
 */
EIP164_Error_t
EIP164_SecY_RxCAM_Remove(
            const EIP164_IOArea_t * const IOArea_p,
            const unsigned int SCIndex);


/*----------------------------------------------------------------------------
 * EIP164_SecY_RxCAM_Add
 *
 * Add entry to RXCAM to look up an SC from a given vPort and SCI for
 * an inbound SA.
 *
 * API use order:
 *      This function, EIP164_SecY_SA_Write(),
 *      EIP164_SecY_SC_SA_Map_I_Update() and EIP164_SecY_SAMFlow_Write()
 *      must all be called to create an active ingress SA, where
 *      EIP164_SecY_SA_Write() may not be called last.
 *
 * IOArea_p (input)
 *      Pointer to the Device instance I/O area in memory that identifies the
 *      EIP-164 device to be used.
 *
 * SCIndex (input)
 *      Index of SC to be written must be less than the maximum number of SC's
 *      supported by the device. The user must keep track of free and used SC's.
 *
 * SCI_Lo (input)
 *      Lower part of SCi for the match entry
 *
 * SCI_Hi (input)
 *      Higher part of SCi for the match entry
 *
 * vPort (input)
 *      vPort for the match entry
 *
 * This function is NOT re-entrant for the same Device.
 * This function is re-entrant for different Devices.
 *
 * This function cannot be called concurrently with function
 * EIP164_SecY_RxCAM_Read() or EIP164_SecY_RxCAM_Add() for the same
 * Device and the same SCIndex.
 *
 * For hardware with an Internal TCAM only, this function can be called
 * concurrently with the EIP164_SecY_RxCAM_Read() and
 * EIP164_SecY_RxCAM_Add() functions for the same Device and a
 * different SCIndex .
 *
 * This function can be called concurrently with the
 * EIP164_SecY_RxCAM_Read() and EIP164_SecY_RxCAM_Add() functions for
 * a different device.
 *
 * This function can be called concurrently with any other functions for the
 * same or different device.
 *
 * Return value:
 *      EIP164_NO_ERROR : Operation is completed
 *      EIP164_ARGUMENT_ERROR : Passed wrong argument.
 *      EIP164_BUSY_RETRY_LATER : Device initialization is not completed yet.
 */
EIP164_Error_t
EIP164_SecY_RxCAM_Add(
        const EIP164_IOArea_t * const IOArea_p,
        const unsigned int SCIndex,
        const uint32_t SCI_Lo,
        const uint32_t SCI_Hi,
        const unsigned int vPort);


/*----------------------------------------------------------------------------
 * EIP164_SecY_RxCAM_Read
 *
 * Read entry from RxCAM.
 *
 * API use order:
 *      This function will return *fEnabled_p == true if it is called for
 *      an entry previously added with EIP164_SecY_RxCAM_Add() and not yet
 *      removed with EIP164_SecY_RxCAM_Remove(). In other cases it will
 *      *fEnabled_p == false and the SCI_Lo_p, SCI_Hi_p and vPoort_p outputs
 *      are undefined.
 *
 * IOArea_p (input)
 *      Pointer to the Device instance I/O area in memory that identifies the
 *      EIP-164 device to be used.
 *
 * SCIndex (input)
 *      Index of SC to be read must be less than the maximum number of SC's
 *      supported by the device. The user must keep track of free and used SC's.
 *
 * SCI_Lo_p (output)
 *      Lower part of SCi for the match entry
 *
 * SCI_Hi_p (output)
 *      Higher part of SCi for the match entry
 *
 * vPort_p (output)
 *      vPort for the match entry
 *
 * fEnabled_p (output)
 *      true if the RxCAM entry is enabled.
 *
 * This function is re-entrant for the same or different Devices for hardware
 * with an internal TCAM.
 * For hardware with an external TCAM, this function is not re-entrant for
 * the same device..
 *
 * This function cannot be called concurrently with function
 * EIP164_SecY_RxCAM_Read() or EIP164_SecY_RxCAM_Add() for the same
 * Device and the same SCIndex.
 *
 * For hardware with an Internal TCAM only, this function can be called
 * concurrently with the EIP164_SecY_RxCAM_Read() and
 * EIP164_SecY_RxCAM_Add() functions for the same Device and a
 * different SCIndex .
 *
 * This function can be called concurrently with the
 * EIP164_SecY_RxCAM_Read() and EIP164_SecY_RxCAM_Add() functions for
 * a different device.
 *
 * This function can be called concurrently with any other functions for the
 * same or different device.
 *
 * Return value:
 *      EIP164_NO_ERROR : Operation is completed
 *      EIP164_ARGUMENT_ERROR : Passed wrong argument.
 *      EIP164_BUSY_RETRY_LATER : Device initialization is not completed yet.
 */
EIP164_Error_t
EIP164_SecY_RxCAM_Read(
        EIP164_IOArea_t * const IOArea_p,
        const unsigned int SCIndex,
        uint32_t * const SCI_Lo_p,
        uint32_t * const SCI_Hi_p,
        unsigned int * vPort_p,
        bool *fEnabled_p);


/*----------------------------------------------------------------------------
 * EIP164_SecY_SAMFlow_Clear
 *
 * Clear a SAMFlow entry, setting it to bypass.
 *
 * API use order:
 *      This function can be used at all times after
 *      EIP164_Device_Init() and it can be used to invalidate an entry
 *      written with EIP164_SAMFlow_Write().
 *
 * IOArea_p (input)
 *      Pointer to the Device instance I/O area in memory that identifies the
 *      EIP-164 device to be used.
 *
 * vPort (input)
 *      vPort number of the entry to be cleared.
 *
 * This function is re-entrant for the same or different Devices.
 *
 * This function cannot be called concurrently with functions
 * EIP164_SecY_SAMFlow_Write() or EIP164_SecY_SAMFlow_Read()
 * for the same Device and the same vPort.
 * This function can be called concurrently with
 * the EIP164_SecY_SAMFlow_Write() or EIP164_SecY_SAMFlow_Read() functions or
 * other functions for different Devices or different vPort.
 *
 * Return value:
 *      EIP164_NO_ERROR : Operation is completed
 *      EIP164_ARGUMENT_ERROR : Passed wrong argument.
 */
EIP164_Error_t
EIP164_SecY_SAMFlow_Clear(
            const EIP164_IOArea_t * const IOArea_p,
            const unsigned int vPort);


/*----------------------------------------------------------------------------
 * EIP164_SecY_SAMFlow_Write
 *
 * Write a SAMFlow entry.
 *
 * API use order:
 *      For bypass/drop: call this function alone.
 *      For egress: call EIP164_SecY_SA_Write(), EIP164_SecY_SC_SA_E_Map() and
 *                  this function to make an egress SA active.
 *      For ingress: call EIP164_SecY_SA_Write(), EIP164_RxCAM_Add(),
 *                  EIP164_SecY_SC_SA_I_Map() and this function to make an
 *                  ingress SA active.
 *
 * IOArea_p (input)
 *      Pointer to the Device instance I/O area in memory that identifies the
 *      EIP-164 device to be used.
 *
 * vPort (input)
 *      vPort number of the entry to be cleared.
 *
 * SA_p (input)
 *      Pointer to the parameters of the entry to be written.
 *
 * This function is NOT re-entrant for the same Device.
 * This function is re-entrant for different Devices.
 *
 * This function cannot be called concurrently with functions
 * EIP164_SecY_SAMFlow_Clear() or EIP164_SecY_SAMFlow_Read()
 * for the same Device and the same vPort.
 * This function can be called concurrently with
 * the EIP164_SecY_SAMFlow_Clear() or EIP164_SecY_SAMFlow_Read() functions or
 * other functions for different Devices or different vPort.
 *
 * Return value:
 *      EIP164_NO_ERROR : Operation is completed
 *      EIP164_ARGUMENT_ERROR : Passed wrong argument.
 */
EIP164_Error_t
EIP164_SecY_SAMFlow_Write(
        const EIP164_IOArea_t * const IOArea_p,
        const unsigned int vPort,
        const EIP164_SecY_SA_t * const SA_p
        );


/*----------------------------------------------------------------------------
 * EIP164_SecY_SAMFlow_Read
 *
 * Read a SAMFlow entry.
 *
 * API use order:
 *      This function can be used at all times after
 *      EIP164_Device_Init(). When called after EIP164_SecY_SAMFlow_Write()
 *      for the same vPort it will return the input parameters of that call.
 *
 * IOArea_p (input)
 *      Pointer to the Device instance I/O area in memory that identifies the
 *      EIP-164 device to be used.
 *
 * vPort (input)
 *      vPort number of the entry to be cleared.
 *
 * SA_p (output)
 *      Pointer to the parameters of the entry to be written.
 *
 * This function is re-entrant for the same or different Devices.
 *
 * This function cannot be called concurrently with functions
 * EIP164_SecY_SAMFlow_Write() or EIP164_SecY_SAMFlow_Clear()
 * for the same Device and the same vPort.
 * This function can be called concurrently with
 * the EIP164_SecY_SAMFlow_Write() or EIP164_SecY_SAMFlow_Clear() functions or
 * other functions for different Devices or different vPort.
 *
 * Return value:
 *      EIP164_NO_ERROR : Operation is completed
 *      EIP164_ARGUMENT_ERROR : Passed wrong argument.
 */
EIP164_Error_t
EIP164_SecY_SAMFlow_Read(
        const EIP164_IOArea_t * const IOArea_p,
        const unsigned int vPort,
        EIP164_SecY_SA_t * const SA_p
        );

/*----------------------------------------------------------------------------
 * EIP164_SecY_SA_NextPN_Update
 *
 * Updates the NextPN field of an ingress (inboud) SA for an EIP-164 device
 * instance identified by the IOArea_p parameter.
 *
 * API use order:
 *      This function may be called after the EIP164_SecY_SA_Write() function
 *      for the same SAIndex of the same Device.
 *      After calling this function, call EIP164_SecY_SA_NextPN_Status_Get()
 *      repeatedly until it reports a non-busy status.
 *
 * IOArea_p (input)
 *      Pointer to the Device instance I/O area in memory that identifies the
 *      EIP-164 device to be used.
 *
 * SAIndex (input)
 *      Index of SA to be updated must be less than the maximum number of SA's
 *      supported by the device. The user must keep track of free and used SA's.
 *
 * NextPN_Lo (input)
 *      Least 32 bits of new NextPN value.
 *
 * NextPN_Hi (input)
 *      Most 32 bits of new NextPN value.
 *
 * CtxId (input)
 *      Context ID field of SA to be updated.
 *
 * fSeqNum64 (input)
 *      is true if the sequence number is 64-bit.
 *
 * This function is NOT re-entrant for the same SA of the same Device.
 * This function is re-entrant for different Devices or different SA's
 * of the same Device.
 *
 * This function cannot be called concurrently with
 * any EIP164_SecY_SA_*() function except for EIP164_SecY_SA_Read() for
 * the same Device.
 * This function can be called concurrently with any other EIP164_CfyE_*() or
 * EIP164_SecY_*() function for the different SA's of the same Device or
 * different Devices.
 *
 * Return value:
 *      EIP164_NO_ERROR : Operation is completed
 *      EIP164_UNSUPPORTED_FEATURE_ERROR : Not supported by the device.
 *      EIP164_ARGUMENT_ERROR : Passed wrong argument.
 *      EIP164_HW_CONFIGURATION_MISMATCH : HW configuration does not support
 *                                         requested operation.
 */
EIP164_Error_t
EIP164_SecY_SA_NextPN_Update(
       const EIP164_IOArea_t * const IOArea_p,
       const unsigned int SAIndex,
       const uint32_t NextPN_Lo,
       const uint32_t NextPN_Hi,
       const bool fSeqNum64);


/*----------------------------------------------------------------------------
 * EIP164_SecY_SA_NextPN_Status_Get
 *
 * Reads the status of the last EIP164_SecY_SA_Update_NextPN operation.
 *
 * API use order:
 *      Call this function repeatedly after EIP164_SecY_SA_NextPN_Update()
 *      until the returned busy status is false.
 *
 * IOArea_p (input)
 *      Pointer to the Device instance I/O area in memory that identifies the
 *      EIP-164 device to be used.
 *
 * fNextPNWritten_p (output)
 *      The last EIP164_SecY_SA_Update_NextPN operation resulted in an actual
 *      update of the NextPN field in the SA.
 *
 * fBusy_p (output)
 *      The last EIP164_SecY_SA_Update_NextPN operation is still in progress
 *      and another one may not be started yet.
 *
 * This function is re-entrant for the same SA of the same Device.
 * This function is re-entrant for different Devices or different SA's
 * of the same Device.
 *
 * This function can be called concurrently with any EIP164_CfyE_*() or
 * EIP164_SecY_*() function for the different SA's of the same Device or
 * different Devices.
 *
 * Return value:
 *      EIP164_NO_ERROR : Operation is completed
 *      EIP164_UNSUPPORTED_FEATURE_ERROR : Not supported by the device.
 *      EIP164_ARGUMENT_ERROR : Passed wrong argument.
 *      EIP164_HW_CONFIGURATION_MISMATCH : HW configuration does not support
 *                                         requested operation.
 */
EIP164_Error_t
EIP164_SecY_SA_NextPN_Status_Get(
       const EIP164_IOArea_t * const IOArea_p,
       bool * const fNextPNWritten_p,
       bool * const fBusy_p);


/*----------------------------------------------------------------------------
 * EIP164_SecY_SA_Read
 *
 * Reads (a part of) a transform record of an already added SA from an EIP-164
 * device instance identified by the IOArea_p parameter.
 *
 * API use order:
 *      This function may be called after the EIP164_SecY_SA_Write() function
 *      for the same SAIndex of the same Device.
 *
 * IOArea_p (input)
 *      Pointer to the Device instance I/O area in memory that identifies the
 *      EIP-164 device to be used.
 *
 * SAIndex (input)
 *      Index of SA to be read must be less than the maximum number of SA's
 *      supported by the device. The user must keep track of free and used SA's.
 *
 * WordOffset (input)
 *      Word offset in transform record where the data should be read from.
 *
 * WordCount (input)
 *      Number of words which must be read from the SA transform record.
 *
 * Transform_p (output)
 *      Pointer to the memory location where the WordCount 32-bit words
 *      of the SA transform record will be stored.
 *
 * This function is re-entrant.
 *
 * This function can be called concurrently with any other API function for
 * the same or different SA's of the same or different Devices provided
 * the use order of the API functions is followed.
 *
 * Return value:
 *      EIP164_NO_ERROR : Operation is completed
 *      EIP164_UNSUPPORTED_FEATURE_ERROR : Not supported by the device.
 *      EIP164_ARGUMENT_ERROR : Passed wrong argument.
 *      EIP164_HW_CONFIGURATION_MISMATCH : HW configuration does not support
 *                                         requested operation.
 */
EIP164_Error_t
EIP164_SecY_SA_Read(
        const EIP164_IOArea_t * const IOArea_p,
        const unsigned int SAIndex,
        const unsigned int WordOffset,
        const unsigned int WordCount,
        uint32_t * Transform_p);


/*----------------------------------------------------------------------------
 * EIP164_SecY_SA_Update_Control_Word_Update
 *
 * Updates any single 32-bit word for an already added SA for an
 * EIP-164 device instance identified by the IOArea_p parameter. This
 * function is intended to be used to update the SA Update control
 * word, the ingress anti replay window size or the egress MTU of an existing
 * SA.
 *
 * API use order:
 *      This function may be called after the EIP164_SecY_SA_Write() function
 *      for the same SAIndex of the same Device.
 *
 * IOArea_p (input)
 *      Pointer to the Device instance I/O area in memory that identifies the
 *      EIP-164 device to be used.
 *
 * SAIndex (input)
 *      Index of SA to be updated.
 *
 * Offset (input)
 *      Offset of SA Update Control word.
 *
 * SAUpdateCtrWord (input)
 *      New value to be stored in SA Update Control word.
 *
 * This function is re-entrant for the same SA of the same Device.
 * This function is re-entrant for different Devices or different SA's
 * of the same Device.
 *
 * This function cannot be called concurrently with
 * any EIP164_SecY_SA_*() function except for EIP164_SecY_SA_Read() for
 * the same SA of the same Device.
 * This function can be called concurrently with any other EIP164_CfyE_*() or
 * EIP164_SecY_*() function for the different SA's of the same Device or
 * different Devices.
 *
 * Return value:
 *      EIP164_NO_ERROR : Operation is completed
 *      EIP164_ARGUMENT_ERROR : Passed wrong argument.
 */
EIP164_Error_t
EIP164_SecY_SA_Update_Control_Word_Update(
        const EIP164_IOArea_t * const IOArea_p,
        const unsigned int SAIndex,
        const unsigned int Offset,
        const uint32_t UpdateCtrlWord);

/*----------------------------------------------------------------------------
 * EIP164_SecY_SA_Installation_Verify
 *
 * Returns the active egress SA for a given SC along with the sa_in_use flag.
 * This can be used to verify if an egress SA is added or switched correctly.
 *
 * IOArea_p (input)
 *      Pointer to the Device instance I/O area in memory that identifies the
 *      EIP-164 device to be used.
 *
 * SAMFlowCtrlIndex (input)
 *      Index of the flow control register.
 *
 * SAIndex_p (output)
 *      Pointer to a memory location where the SA index is stored
 *
 * fSAInUse_p (output)
 *      Pointer to a memory location where the in-use flag is stored.
 *
 * This function is re-entrant for the same SA of the same Device.
 * This function is re-entrant for different Devices or different SA's
 * of the same Device.
 *
 * Return value:
 *      EIP164_NO_ERROR : Operation is completed
 *      EIP164_ARGUMENT_ERROR : Passed wrong argument.
 */
EIP164_Error_t
EIP164_SecY_SA_Installation_Verify(
        const EIP164_IOArea_t * const IOArea_p,
        const unsigned int SAMFlowCtrlIndex,
        unsigned int * const SAIndex_p,
        bool * const fSAInUse_p);



/*----------------------------------------------------------------------------
 * EIP164_SecY_SA_Active_I_Get
 *
 * Return the four ingress SA indexes corresponding to an SC along with
 * flags indicating their validity.
 *
 * IOArea_p (input)
 *      Pointer to the Device instance I/O area in memory that identifies the
 *      EIP-164 device to be used.
 *
 * SCIndex (input)
 *      Index of the flow control register.
 *
 * SAIndex_p (output)
 *      Pointer to an array of four SA index values
 *
 * fSAInUse_p (output)
 *      Pointer to an array of four SA index values.
 *
 * This function is re-entrant for the same SA of the same Device.
 * This function is re-entrant for different Devices or different SA's
 * of the same Device.
 *
 * Return value:
 *      EIP164_NO_ERROR : Operation is completed
 *      EIP164_ARGUMENT_ERROR : Passed wrong argument.
 */
EIP164_Error_t
EIP164_SecY_SA_Active_I_Get(
        const EIP164_IOArea_t * const IOArea_p,
        const unsigned int SCIndex,
        unsigned int * const SAIndex_p,
        bool * const fSAInUse_p);


/*----------------------------------------------------------------------------
 * EIP164_SecY_Stat_E_Clear
 *
 * Clear the egress SecY statistics for an EIP-164 device instance
 * identified by the IOArea_p and SecY_Index parameters.
 *
 * Note: This function can be called only for the device initialized
 *       for the EIP164_MODE_EGRESS or EIP164_MODE_EGRESS_INGRESS mode.
 *
 * IOArea_p (input)
 *      Pointer to the Device instance I/O area in memory that identifies the
 *      EIP-164 device to be used.
 *
 * SecY_Index (input)
 *      SecY index for which the SecY statistics must be cleared
 *
 * This function is re-entrant for the same or different Devices.
 *
 * This function cannot be called concurrently with function
 * EIP164_SecY_Stat_E_Get() for the same Device and the same SecY index.
 * This function can be called concurrently with
 * the EIP164_SecY_Stat_E_Get() function or other functions
 * for different Devices or different SecY index.
 *
 * Return value:
 *      EIP164_NO_ERROR : Operation is completed
 *      EIP164_UNSUPPORTED_FEATURE_ERROR : Not supported by the device.
 *      EIP164_ARGUMENT_ERROR : Passed wrong argument.
 *      EIP164_HW_CONFIGURATION_MISMATCH : HW configuration does not support
 *                                         requested operation.
 */
EIP164_Error_t
EIP164_SecY_Stat_E_Clear(
        const EIP164_IOArea_t * const IOArea_p,
        const unsigned int SecY_Index);


/*----------------------------------------------------------------------------
 * EIP164_SecY_Stat_I_Clear
 *
 * Clear the ingress SecY statistics for an EIP-164 device instance
 * identified by the IOArea_p and SecY_Index parameters.
 *
 * Note: This function can be called only for the device initialized
 *       for the EIP164_MODE_INGRESS or EIP164_MODE_EGRESS_INGRESS mode.
 *
 * IOArea_p (input)
 *      Pointer to the Device instance I/O area in memory that identifies the
 *      EIP-164 device to be used.
 *
 * SecY_Index (input)
 *      SecY index for which the SecY statistics must be cleared
 *
 * This function is re-entrant for the same or different Devices.
 *
 * This function cannot be called concurrently with function
 * EIP164_SecY_Stat_I_Get() for the same Device and the same SecY index.
 * This function can be called concurrently with
 * the EIP164_SecY_Stat_I_Get() function or other functions
 * for different Devices or different SecY index.
 *
 * Return value:
 *      EIP164_NO_ERROR : Operation is completed
 *      EIP164_UNSUPPORTED_FEATURE_ERROR : Not supported by the device.
 *      EIP164_ARGUMENT_ERROR : Passed wrong argument.
 *      EIP164_HW_CONFIGURATION_MISMATCH : HW configuration does not support
 *                                         requested operation.
 */
EIP164_Error_t
EIP164_SecY_Stat_I_Clear(
        const EIP164_IOArea_t * const IOArea_p,
        const unsigned int SecY_Index);


/*----------------------------------------------------------------------------
 * EIP164_SecY_SA_Stat_E_Clear
 *
 * Clear the egress SA statistics for an EIP-164 device instance
 * identified by the IOArea_p and SAIndex parameters.
 *
 * Note: This function can be called only for the device initialized
 *       for the EIP164_MODE_EGRESS or EIP164_MODE_EGRESS_INGRESS mode.
 *
 * IOArea_p (input)
 *      Pointer to the Device instance I/O area in memory that identifies the
 *      EIP-164 device to be used.
 *
 * SAIndex (input)
 *      SA index for which the SA statistics must be cleared
 *
 * This function is re-entrant for the same or different Devices.
 *
 * This function cannot be called concurrently with function
 * EIP164_SecY_SA_Stat_E_Get() for the same Device and the same SA index.
 * This function can be called concurrently with
 * the EIP164_SecY_SA_Stat_E_Get() function or other functions
 * for different Devices or different SA indices.
 *
 * Return value:
 *      EIP164_NO_ERROR : Operation is completed
 *      EIP164_UNSUPPORTED_FEATURE_ERROR : Not supported by the device.
 *      EIP164_ARGUMENT_ERROR : Passed wrong argument.
 *      EIP164_HW_CONFIGURATION_MISMATCH : HW configuration does not support
 *                                         requested operation.
 */
EIP164_Error_t
EIP164_SecY_SA_Stat_E_Clear(
        const EIP164_IOArea_t * const IOArea_p,
        const unsigned int SAIndex);


/*----------------------------------------------------------------------------
 * EIP164_SecY_SA_Stat_I_Clear
 *
 * Clear the ingress SecY statistics for an EIP-164 device instance
 * identified by the IOArea_p and SAIndex parameters.
 *
 * Note: This function can be called only for the device initialized
 *       for the EIP164_MODE_INGRESS or EIP164_MODE_EGRESS_INGRESS mode.
 *
 * IOArea_p (input)
 *      Pointer to the Device instance I/O area in memory that identifies the
 *      EIP-164 device to be used.
 *
 * SAIndex (input)
 *      SA index for which the SA statistics must be cleared
 *
 * This function is re-entrant for the same or different Devices.
 *
 * This function cannot be called concurrently with function
 * EIP164_SecY_SA_Stat_I_Get() for the same Device and the same SA index.
 * This function can be called concurrently with
 * the EIP164_SecY_SA_Stat_I_Get() function or other functions
 * for different Devices or different SA indices.
 *
 * Return value:
 *      EIP164_NO_ERROR : Operation is completed
 *      EIP164_UNSUPPORTED_FEATURE_ERROR : Not supported by the device.
 *      EIP164_ARGUMENT_ERROR : Passed wrong argument.
 *      EIP164_HW_CONFIGURATION_MISMATCH : HW configuration does not support
 *                                         requested operation.
 */
EIP164_Error_t
EIP164_SecY_SA_Stat_I_Clear(
        const EIP164_IOArea_t * const IOArea_p,
        const unsigned int SAIndex);


/*----------------------------------------------------------------------------
 * EIP164_SecY_Ifc_Stat_E_Clear
 *
 * Clear the egress Interface statistics for an EIP-164 device instance
 * identified by the IOArea_p and IfcIndex parameters.
 *
 * Note: This function can be called only for the device initialized
 *       for the EIP164_MODE_EGRESS or EIP164_MODE_EGRESS_INGRESS mode.
 *
 * IOArea_p (input)
 *      Pointer to the Device instance I/O area in memory that identifies the
 *      EIP-164 device to be used.
 *
 * IfcIndex (input)
 *      Interface index for which the Ifc statistics must be cleared
 *
 * This function is re-entrant for the same or different Devices.
 *
 * This function cannot be called concurrently with function
 * EIP164_SecY_Ifc_Stat_E_Get() for the same Device and the same Ifc index.
 * This function can be called concurrently with
 * the EIP164_SecY_Ifc_Stat_E_Get() function or other functions
 * for different Devices or different Ifc indices.
 *
 * Return value:
 *      EIP164_NO_ERROR : Operation is completed
 *      EIP164_UNSUPPORTED_FEATURE_ERROR : Not supported by the device.
 *      EIP164_ARGUMENT_ERROR : Passed wrong argument.
 *      EIP164_HW_CONFIGURATION_MISMATCH : HW configuration does not support
 *                                         requested operation.
 */
EIP164_Error_t
EIP164_SecY_Ifc_Stat_E_Clear(
        const EIP164_IOArea_t * const IOArea_p,
        const unsigned int IfcIndex);


/*----------------------------------------------------------------------------
 * EIP164_SecY_Ifc_Stat_I_Clear
 *
 * Clear the ingress Interface statistics for an EIP-164 device instance
 * identified by the IOArea_p and IfcIndex parameters.
 *
 * Note: This function can be called only for the device initialized
 *       for the EIP164_MODE_INGRESS or EIP164_MODE_EGRESS_INGRESS mode.
 *
 * IOArea_p (input)
 *      Pointer to the Device instance I/O area in memory that identifies the
 *      EIP-164 device to be used.
 *
 * IfcIndex (input)
 *      Interface index for which the Ifc statistics must be cleared
 *
 * This function is re-entrant for the same or different Devices.
 *
 * This function cannot be called concurrently with function
 * EIP164_SecY_Ifc_Stat_I_Get() for the same Device and the same Ifc index.
 * This function can be called concurrently with
 * the EIP164_SecY_Ifc_Stat_I_Get() function or other functions
 * for different Devices or different Ifc indices.
 *
 * Return value:
 *      EIP164_NO_ERROR : Operation is completed
 *      EIP164_UNSUPPORTED_FEATURE_ERROR : Not supported by the device.
 *      EIP164_ARGUMENT_ERROR : Passed wrong argument.
 *      EIP164_HW_CONFIGURATION_MISMATCH : HW configuration does not support
 *                                         requested operation.
 */
EIP164_Error_t
EIP164_SecY_Ifc_Stat_I_Clear(
        const EIP164_IOArea_t * const IOArea_p,
        const unsigned int IfcIndex);


/*----------------------------------------------------------------------------
 * EIP164_SecY_Stat_E_Get
 *
 * Retrieves the egress SecY statistics for an EIP-164 device instance
 * identified by the IOArea_p and SecY_Index parameters.
 *
 * Note: This function can be called only for the device initialized
 *       for the EIP164_MODE_EGRESS or EIP164_MODE_EGRESS_INGRESS mode.
 *
 * API use order:
 *      For packet-precise statistics ensure the host-device synchronization
 *      before reading its statistics. Use EIP164_SecY_Device_Sync() and
 *      EIP164_SecY_Device_Is_Sync_Done() functions for this.
 *
 * IOArea_p (input)
 *      Pointer to the Device instance I/O area in memory that identifies the
 *      EIP-164 device to be used.
 *
 * SecY_Index (input)
 *      SecY index for which the SecY statistics must be retrieved
 *
 * Stats_p (output)
 *     Pointer to the data structure where the SecY egress device statistics
 *     parameters will be stored
 *
 * fClear (input)
 *      Use write action to clear statistics registers
 *
 * This function is NOT re-entrant for the same SecY index of the same Device.
 * This function is re-entrant for different Devices or different SecY indices
 * of the same Device.
 *
 * This function cannot be called concurrently with function
 * EIP164_SecY_Stat_E_Clear() for the same Device and the same SecY index.
 * This function can be called concurrently with
 * the EIP164_SecY_Stat_E_Clear() function or other functions
 * for different Devices or different SecY indices.
 *
 * Return value:
 *      EIP164_NO_ERROR : Operation is completed
 *      EIP164_UNSUPPORTED_FEATURE_ERROR : Not supported by the device.
 *      EIP164_ARGUMENT_ERROR : Passed wrong argument.
 *      EIP164_HW_CONFIGURATION_MISMATCH : HW configuration does not support
 *                                         requested operation.
 */
EIP164_Error_t
EIP164_SecY_Stat_E_Get(
        const EIP164_IOArea_t * const IOArea_p,
        const unsigned int SecY_Index,
        EIP164_SecY_Statistics_E_t * const Stats_p,
        const bool fClear);


/*----------------------------------------------------------------------------
 * EIP164_SecY_Stat_I_Get
 *
 * Retrieves the ingress SecY statistics for an EIP-164 device instance
 * identified by the IOArea_p and SecY_Index parameters.
 *
 * Note: This function can be called only for the device initialized
 *       for the EIP164_MODE_INGRESS or EIP164_MODE_EGRESS_INGRESS mode.
 *
 * API use order:
 *      For packet-precise statistics ensure the host-device synchronization
 *      before reading its statistics. Use EIP164_SecY_Device_Sync() and
 *      EIP164_SecY_Device_Is_Sync_Done() functions for this.
 *
 * IOArea_p (input)
 *      Pointer to the Device instance I/O area in memory that identifies the
 *      EIP-164 device to be used.
 *
 * SecY_Index (input)
 *      SecY index for which the SecY statistics must be retrieved
 *
 * Stats_p (output)
 *      Pointer to the data structure where the SecY ingress device statistics
 *      parameters will be stored
 *
 * fClear (input)
 *      Use write action to clear statistics registers
 *
 * This function is NOT re-entrant for the same SecY index of the same Device.
 * This function is re-entrant for different Devices or different SecY indices
 * of the same Device.
 *
 * This function cannot be called concurrently with function
 * EIP164_SecY_Stat_I_Clear() for the same Device and the same SecY index.
 * This function can be called concurrently with
 * the EIP164_SecY_Stat_I_Clear() function or other functions
 * for different Devices or different SecY indices.
 *
 * Return value:
 *      EIP164_NO_ERROR : Operation is completed
 *      EIP164_UNSUPPORTED_FEATURE_ERROR : Not supported by the device.
 *      EIP164_ARGUMENT_ERROR : Passed wrong argument.
 *      EIP164_HW_CONFIGURATION_MISMATCH : HW configuration does not support
 *                                         requested operation.
 */
EIP164_Error_t
EIP164_SecY_Stat_I_Get(
        const EIP164_IOArea_t * const IOArea_p,
        const unsigned int SecY_Index,
        EIP164_SecY_Statistics_I_t * const Stats_p,
        const bool fClear);


/*----------------------------------------------------------------------------
 * EIP164_SecY_SA_Stat_E_Get
 *
 * Reads egress SA statistics from an EIP-164 device instance identified
 * by the IOArea_p and SAIndex parameters.
 *
 * Note: This function can be called only for the device initialized
 *       for the EIP164_MODE_EGRESS or EIP164_MODE_EGRESS_INGRESS mode.
 *
 * API use order:
 *      For packet-precise statistics ensure the host-device synchronization
 *      after removing an SA before reading its statistics.
 *      Use EIP164_SecY_Device_Sync() and EIP164_SecY_Device_Is_Sync_Done()
 *      functions for this.
 *
 * IOArea_p (input)
 *      Pointer to the Device instance I/O area in memory that identifies the
 *      EIP-164 device to be used.
 *
 * SAIndex (input)
 *      SA index to be added must be less than the maximum number of SA's
 *      supported by the device. The user must keep track of free and used SA's.
 *
 * Stat_p (output)
 *      Pointer to a memory location where the SA statistics will be stored.
 *
 * fClear (input)
 *      Use write action to clear statistics registers
 *
 * This function is NOT re-entrant for the same SA of the same Device.
 * This function is re-entrant for different Devices or different SA's
 * of the same Device.
 *
 * This function cannot be called concurrently with function
 * EIP164_SecY_SA_Stat_E_Clear() for the same Device and the same SA index.
 * This function can be called concurrently with
 * the EIP164_SecY_SA_Stat_E_Clear() function or other functions
 * for different Devices or different SA indices.
 *
 * Return value:
 *      EIP164_NO_ERROR : Operation is completed
 *      EIP164_UNSUPPORTED_FEATURE_ERROR : Not supported by the device.
 *      EIP164_ARGUMENT_ERROR : Passed wrong argument.
 *      EIP164_HW_CONFIGURATION_MISMATCH : HW configuration does not support
 *                                         requested operation.
 */
EIP164_Error_t
EIP164_SecY_SA_Stat_E_Get(
        const EIP164_IOArea_t * const IOArea_p,
        const unsigned int SAIndex,
        EIP164_SecY_SA_Stat_E_t * const Stat_p,
        const bool fClear);

/*----------------------------------------------------------------------------
 * EIP164_SecY_SA_Stat_I_Get
 *
 * Reads ingress SA statistics from an EIP-164 device instance identified
 * by the IOArea_p and SAIndex parameters.
 *
 * Note: This function can be called only for the device initialized
 *       for the EIP164_MODE_INGRESS or EIP164_MODE_EGRESS_INGRESS mode.
 *
 * API use order:
 *      For packet-precise statistics ensure the host-device synchronization
 *      after removing an SA before reading its statistics.
 *      Use EIP164_SecY_Device_Sync() and EIP164_SecY_Device_Is_Sync_Done()
 *      functions for this.
 *
 * IOArea_p (input)
 *      Pointer to the Device instance I/O area in memory that identifies the
 *      EIP-164 device to be used.
 *
 * SAIndex (input)
 *      SA index to be added must be less than the maximum number of SA's
 *      supported by the device. The user must keep track of free and used SA's.
 *
 * Stat_p (output)
 *      Pointer to a memory location where the SA statistics will be stored.
 *
 * fClear (input)
 *      Use write action to clear statistics registers
 *
 * This function is NOT re-entrant for the same SA of the same Device.
 * This function is re-entrant for different Devices or different SA's
 * of the same Device.
 *
 * This function cannot be called concurrently with function
 * EIP164_SecY_SA_Stat_I_Clear() for the same Device and the same SA index.
 * This function can be called concurrently with
 * the EIP164_SecY_SA_Stat_I_Clear() function or other functions
 * for different Devices or different SA indices.
 *
 * Return value:
 *      EIP164_NO_ERROR : Operation is completed
 *      EIP164_UNSUPPORTED_FEATURE_ERROR : Not supported by the device.
 *      EIP164_ARGUMENT_ERROR : Passed wrong argument.
 *      EIP164_HW_CONFIGURATION_MISMATCH : HW configuration does not support
 *                                         requested operation.
 */
EIP164_Error_t
EIP164_SecY_SA_Stat_I_Get(
        const EIP164_IOArea_t * const IOArea_p,
        const unsigned int SAIndex,
        EIP164_SecY_SA_Stat_I_t * const Stat_p,
        const bool fClear);


/*----------------------------------------------------------------------------
 * EIP164_SecY_Ifc_Stat_E_Get
 *
 * Reads egress IFC/IFC1 statistics from an EIP-164 device instance identified
 * by the IOArea_p and IfcIndex parameters.
 *
 * Note: This function can be called only for the device initialized
 *       for the EIP164_MODE_EGRESS or EIP164_MODE_EGRESS_INGRESS mode.
 *
 * API use order:
 *      For packet-precise statistics ensure the host-device synchronization
 *      before reading its statistics. Use EIP164_SecY_Device_Sync() and
 *      EIP164_SecY_Device_Is_Sync_Done() functions for this.
 *
 * IOArea_p (input)
 *      Pointer to the Device instance I/O area in memory that identifies the
 *      EIP-164 device to be used.
 *
 * IfcIndex (input)
 *      IFC/IFC1 index to be added must be less than the maximum number of
 *      IFC's supported by the device. The user must keep track of free and
 *      used IFC's.
 *
 * Stat_p (output)
 *      Pointer to a memory location where the IFC/IFC1 statistics will be
 *      stored.
 *
 * fClear (input)
 *      Use write action to clear statistics registers
 *
 * This function is NOT re-entrant for the same IFC/IFC1 of the same Device.
 * This function is re-entrant for different Devices or different IFC/IFC1's
 * of the same Device.
 *
 * This function cannot be called concurrently with function
 * EIP164_SecY_Ifc_Stat_E_Clear() for the same Device and the same IFC index.
 * This function can be called concurrently with
 * the EIP164_SecY_Ifc_Stat_E_Clear() function or other functions
 * for different Devices or different IFC indices.
 *
 * Return value:
 *      EIP164_NO_ERROR : Operation is completed
 *      EIP164_UNSUPPORTED_FEATURE_ERROR : Not supported by the device.
 *      EIP164_ARGUMENT_ERROR : Passed wrong argument.
 *      EIP164_HW_CONFIGURATION_MISMATCH : HW configuration does not support
 *                                         requested operation.
 */
EIP164_Error_t
EIP164_SecY_Ifc_Stat_E_Get(
        const EIP164_IOArea_t * const IOArea_p,
        const unsigned int IfcIndex,
        EIP164_SecY_Ifc_Stat_E_t * const Stat_p,
        const bool fClear);


/*----------------------------------------------------------------------------
 * EIP164_SecY_Ifc_Stat_I_Get
 *
 * Reads ingress IFC/IFC1 statistics from an EIP-164 device instance
 * identified by the IOArea_p and IfcIndex parameters.
 *
 * Note: This function can be called only for the device initialized
 *       for the EIP164_MODE_INGRESS or EIP164_MODE_EGRESS_INGRESS mode.
 *
 * API use order:
 *      For packet-precise statistics ensure the host-device synchronization
 *      before reading its statistics. Use EIP164_SecY_Device_Sync() and
 *      EIP164_SecY_Device_Is_Sync_Done() functions for this.
 *
 * IOArea_p (input)
 *      Pointer to the Device instance I/O area in memory that identifies the
 *      EIP-164 device to be used.
 *
 * IfcIndex (input)
 *      IFC/IFC1 index to be added must be less than the maximum number of
 *      IFC's supported by the device. The user must keep track of free and
 *      used IFC's.
 *
 * Stat_p (output)
 *      Pointer to a memory location where the IFC/IFC1 statistics will be
 *      stored.
 *
 * fClear (input)
 *      Use write action to clear statistics registers
 *
 * This function is NOT re-entrant for the same IFC/IFC1 of the same Device.
 * This function is re-entrant for different Devices or different IFC/IFC1's
 * of the same Device.
 *
 * This function cannot be called concurrently with function
 * EIP164_SecY_Ifc_Stat_I_Clear() for the same Device and the same IFC index.
 * This function can be called concurrently with
 * the EIP164_SecY_Ifc_Stat_I_Clear() function or other functions
 * for different Devices or different IFC indices.
 *
 * Return value:
 *      EIP164_NO_ERROR : Operation is completed
 *      EIP164_UNSUPPORTED_FEATURE_ERROR : Not supported by the device.
 *      EIP164_ARGUMENT_ERROR : Passed wrong argument.
 *      EIP164_HW_CONFIGURATION_MISMATCH : HW configuration does not support
 *                                         requested operation.
 */
EIP164_Error_t
EIP164_SecY_Ifc_Stat_I_Get(
        const EIP164_IOArea_t * const IOArea_p,
        const unsigned int IfcIndex,
        EIP164_SecY_Ifc_Stat_I_t * const Stat_p,
        const bool fClear);


/*----------------------------------------------------------------------------
 * EIP164_SecY_RxCAM_Stat_Get
 *
 * Reads RxCAM statistics (ingress only) from an EIP-164 device instance
 * identified by the IOArea_p and SCIndex parameters.
 *
 * API use order:
 *      For packet-precise statistics ensure the host-device synchronization
 *      before reading its statistics. Use EIP164_SecY_Device_Sync() and
 *      EIP164_SecY_Device_Is_Sync_Done() functions for this.
 *
 * IOArea_p (input)
 *      Pointer to the Device instance I/O area in memory that identifies the
 *      EIP-164 device to be used.
 *
 * SCIndex (input)
 *      Secure Channel index from which the statistics must be read.
 *
 * Stat_p (output)
 *      Pointer to a memory location where the RxCAM statistics must be
 *      stored.
 *
 * fClear (input)
 *      Use write action to clear statistics registers
 *
 * This function is NOT re-entrant for the same SCIndex of the same Device.
 * This function is re-entrant for different Devices or different SCIndex
 * of the same Device.
 *
 * This function cannot be called concurrently with function
 * EIP164_SecY_RxCAM_Stat_Clear() for the same Device and the same SCIndex.
 * This function can be called concurrently with
 * the EIP164_SecY_RxCAM_Stat_Clear() function or other functions
 * for different Devices or different SCIndex.
 *
 * Return value:
 *      EIP164_NO_ERROR : Operation is completed
 *      EIP164_UNSUPPORTED_FEATURE_ERROR : Not supported by the device.
 *      EIP164_ARGUMENT_ERROR : Passed wrong argument.
 */
EIP164_Error_t
EIP164_SecY_RxCAM_Stat_Get(
        const EIP164_IOArea_t * const IOArea_p,
        const unsigned int SCIndex,
        EIP164_SecY_RxCAM_Stat_t * const Stats_p,
        const bool fClear);


/*----------------------------------------------------------------------------
 * EIP164_SecY_RxCAM_Stat_Clear
 *
 * Clear RxCAM statistics (ingress only) from an EIP-164 device instance
 * identified by the IOArea_p and SCIndex parameters.
 *
 * Note: This function can be called only for the device initialized
 *       for the EIP164_MODE_INGRESS or EIP164_MODE_EGRESS_INGRESS mode.
 *
 * IOArea_p (input)
 *      Pointer to the Device instance I/O area in memory that identifies the
 *      EIP-164 device to be used.
 *
 * SCIndex (input)
 *      Secure Channel index from which the statistics must be cleared.
 *
 * This function is re-entrant for the same or different Devices.
 *
 * This function cannot be called concurrently with function
 * EIP164_SecY_RxCAM_Stat_Get() for the same Device and the same SCIndex.
 * This function can be called concurrently with
 * the EIP164_SecY_RxCAM_Stat_Get() function or other functions
 * for different Devices or different SCIndex.
 *
 * Return value:
 *      EIP164_NO_ERROR : Operation is completed
 *      EIP164_UNSUPPORTED_FEATURE_ERROR : Not supported by the device.
 *      EIP164_ARGUMENT_ERROR : Passed wrong argument.
 */
EIP164_Error_t
EIP164_SecY_RxCAM_Stat_Clear(
        const EIP164_IOArea_t * const IOArea_p,
        const unsigned int SCIndex);


/*----------------------------------------------------------------------------
 * EIP164_Device_SAPnThrSummary_Read
 *
 * This function retrieves the value of the specified SA packet number
 * threshold summary register. Each bit represents a SA if set the SA has
 * processed more packets than the threshold.
 *
 * IOArea_p (input)
 *      Pointer to the Device instance I/O area in memory that identifies the
 *      EIP-164 device to be used.
 *
 * Index (input)
 *      Index of SA PnThrSummary register to be read.
 *
 * SAPnThrSummary_p (output)
 *      Pointer to a memory location where the a set of bits in one SA
 *      PnThrSummary register, each of them represents one SA. If a bit is 1,
 *      it means the SA it represents processed more than the threshold packets.
 *
 * Return value:
 *      EIP164_NO_ERROR : Operation is completed
 *      EIP164_UNSUPPORTED_FEATURE_ERROR : Not supported by the device.
 *      EIP164_ARGUMENT_ERROR : Passed wrong argument.
 */
EIP164_Error_t
EIP164_Device_SAPnThrSummary_Read(
        const EIP164_IOArea_t * const IOArea_p,
        const unsigned int Index,
        uint32_t * const SAPnThrSummary_p);


/*----------------------------------------------------------------------------
 * EIP164_Device_SAPnThrSummary_Clear
 *
 * This function clears the specified bits of the specified SA packet number
 * threshold summary register. Note that only the set bits will be cleared.
 *
 * IOArea_p (input)
 *      Pointer to the Device instance I/O area in memory that identifies the
 *      EIP-164 device to be used.
 *
 * Index (input)
 *      Index of SA PnThrSummary register to be cleared.
 *
 * SAPnThrSummary (input)
 *      A set of bits in one SA PnThrSummary register, each of them represents
 *      one SA. If a bit is 1, it will be cleared.
 *
 * This function is NOT re-entrant for the same channel of the same Device.
 * This function is re-entrant for different Devices or different channel's
 * of the same Device.
 *
 * Return value:
 *      EIP164_NO_ERROR : Operation is completed
 *      EIP164_UNSUPPORTED_FEATURE_ERROR : Not supported by the device.
 *      EIP164_ARGUMENT_ERROR : Passed wrong argument.
 */
EIP164_Error_t
EIP164_Device_SAPnThrSummary_Clear(
        const EIP164_IOArea_t * const IOArea_p,
        const unsigned int Index,
        const uint32_t SAPnThrSummary);


/*----------------------------------------------------------------------------
 * EIP164_Device_SAExpiredSummary_Read
 *
 * This function retrieves the value of the specified SA expired summary
 * register. Each bit represents a SA if set the SA has expired.
 *
 * IOArea_p (input)
 *      Pointer to the Device instance I/O area in memory that identifies the
 *      EIP-164 device to be used.
 *
 * Index (input)
 *      Index of SA expired summary register to be read.
 *
 * SAExpiredSummary_p (output)
 *      Pointer to a memory location where the a set of bits in one SA
 *      expired summary register, each of them represents one SA. If a bit is 1,
 *      it means the SA it represents expired.
 *
 * Return value:
 *      EIP164_NO_ERROR : Operation is completed
 *      EIP164_UNSUPPORTED_FEATURE_ERROR : Not supported by the device.
 *      EIP164_ARGUMENT_ERROR : Passed wrong argument.
 */
EIP164_Error_t
EIP164_Device_SAExpiredSummary_Read(
        const EIP164_IOArea_t * const IOArea_p,
        const unsigned int Index,
        uint32_t * const SAExpiredSummary_p);


/*----------------------------------------------------------------------------
 * EIP164_Device_SAExpiredSummary_Clear
 *
 * This function clears the specified bits of the specified SA expired summary
 * register. Note that only the set bits will be cleared.
 *
 * IOArea_p (input)
 *      Pointer to the Device instance I/O area in memory that identifies the
 *      EIP-164 device to be used.
 *
 * Index (input)
 *      Index of SA expired summary register to be cleared.
 *
 * SAExpiredSummary (input)
 *      A set of bits in one SA expired summary register, each of them
 *      represents one SA. If a bit is 1, it will be cleared.
 *
 * This function is NOT re-entrant for the same channel of the same Device.
 * This function is re-entrant for different Devices or different channel's
 * of the same Device.
 *
 * Return value:
 *      EIP164_NO_ERROR : Operation is completed
 *      EIP164_UNSUPPORTED_FEATURE_ERROR : Not supported by the device.
 *      EIP164_ARGUMENT_ERROR : Passed wrong argument.
 */
EIP164_Error_t
EIP164_Device_SAExpiredSummary_Clear(
        const EIP164_IOArea_t * const IOArea_p,
        const unsigned int Index,
        const uint32_t SAExpiredSummary);


/*----------------------------------------------------------------------------
 * EIP164_Device_CountSummary_PSA_Read
 *
 * This function retrieves the value of the specified per-SA counter summary
 * register. Each bit represents a SA if set the one or more SA statistics
 * counters crossed the threshold.
 *
 * IOArea_p (input)
 *      Pointer to the Device instance I/O area in memory that identifies the
 *      EIP-164 device to be used.
 *
 * Index (input)
 *      Index of per-SA counter summary register to be read.
 *
 * CountSummaryPSA_p (output)
 *      Pointer to a memory location where the a set of bits in one per-SA
 *      count summary register, each of them represents one SA. If a bit is 1,
 *      it means at least one of the SA counters crossed the threshold.
 *
 * Return value:
 *      EIP164_NO_ERROR : Operation is completed
 *      EIP164_UNSUPPORTED_FEATURE_ERROR : Not supported by the device.
 *      EIP164_ARGUMENT_ERROR : Passed wrong argument.
 */
EIP164_Error_t
EIP164_Device_CountSummary_PSA_Read(
        const EIP164_IOArea_t * const IOArea_p,
        const unsigned int Index,
        uint32_t * const CountSummaryPSA_p);


/*----------------------------------------------------------------------------
 * EIP164_Device_CountSummary_PSA_Clear
 *
 * This function clears the specified bits of the specified per-SA counter
 * summary register. Note that only the set bits will be cleared.
 *
 * IOArea_p (input)
 *      Pointer to the Device instance I/O area in memory that identifies the
 *      EIP-164 device to be used.
 *
 * Index (input)
 *      Index of per-SA counter summary register to be cleared.
 *
 * CountSummaryPSA (input)
 *      A set of bits in one per-SA counter summary register, each of them
 *      represents one SA. If a bit is 1, it will be cleared.
 *
 * This function is NOT re-entrant for the same channel of the same Device.
 * This function is re-entrant for different Devices or different channel's
 * of the same Device.
 *
 * Return value:
 *      EIP164_NO_ERROR : Operation is completed
 *      EIP164_UNSUPPORTED_FEATURE_ERROR : Not supported by the device.
 *      EIP164_ARGUMENT_ERROR : Passed wrong argument.
 */
EIP164_Error_t
EIP164_Device_CountSummary_PSA_Clear(
        const EIP164_IOArea_t * const IOArea_p,
        const unsigned int Index,
        const uint32_t CountSummaryPSA);


/*----------------------------------------------------------------------------
 * EIP164_Device_CountSummary_SA_Read
 *
 * This function retrieves the value of the specified SA counter summary
 * register. Each bit represents a SA statistics counter if set the
 * corresponding SA statistics counter crossed the threshold.
 *
 * IOArea_p (input)
 *      Pointer to the Device instance I/O area in memory that identifies the
 *      EIP-164 device to be used.
 *
 * Index (input)
 *      Index of SA counter summary register to be read.
 *
 * CountSummarySA_p (output)
 *      Pointer to a memory location where the a set of bits in one SA
 *      count summary register, each of them represents one SA counter.
 *      If a bit is 1, it means one of the SA counters crossed the threshold.
 *
 * Return value:
 *      EIP164_NO_ERROR : Operation is completed
 *      EIP164_UNSUPPORTED_FEATURE_ERROR : Not supported by the device.
 *      EIP164_ARGUMENT_ERROR : Passed wrong argument.
 */
EIP164_Error_t
EIP164_Device_CountSummary_SA_Read(
        const EIP164_IOArea_t * const IOArea_p,
        const unsigned int Index,
        uint32_t * const CountSummarySA_p);


/*----------------------------------------------------------------------------
 * EIP164_Device_CountSummary_SA_Clear
 *
 * This function clears the specified bits of the specified SA counter
 * summary register. Note that only the set bits will be cleared.
 *
 * IOArea_p (input)
 *      Pointer to the Device instance I/O area in memory that identifies the
 *      EIP-164 device to be used.
 *
 * Index (input)
 *      Index of SA counter summary register to be cleared.
 *
 * CountSummarySA (input)
 *      A set of bits in one SA counter summary register, each of them
 *      represents one SA counter. If a bit is 1, it will be cleared.
 *
 * This function is NOT re-entrant for the same channel of the same Device.
 * This function is re-entrant for different Devices or different channel's
 * of the same Device.
 *
 * Return value:
 *      EIP164_NO_ERROR : Operation is completed
 *      EIP164_UNSUPPORTED_FEATURE_ERROR : Not supported by the device.
 *      EIP164_ARGUMENT_ERROR : Passed wrong argument.
 */
EIP164_Error_t
EIP164_Device_CountSummary_SA_Clear(
        const EIP164_IOArea_t * const IOArea_p,
        const unsigned int Index,
        const uint32_t CountSummarySA);


/*----------------------------------------------------------------------------
 * EIP164_Device_CountSummary_PSecY_Read
 *
 * This function retrieves the value of the specified per-SecY counter summary
 * register. Each bit represents a SecY if set the one or more SecY statistics
 * counters crossed the threshold.
 *
 * IOArea_p (input)
 *      Pointer to the Device instance I/O area in memory that identifies the
 *      EIP-164 device to be used.
 *
 * Index (input)
 *      Index of per-SecY counter summary register to be read.
 *
 * CountSummaryPSecY_p (output)
 *      Pointer to a memory location where the a set of bits in one per-SecY
 *      count summary register, each of them represents one SecY. If a bit is 1,
 *      it means at least one of the SecY counters crossed the threshold.
 *
 * Return value:
 *      EIP164_NO_ERROR : Operation is completed
 *      EIP164_UNSUPPORTED_FEATURE_ERROR : Not supported by the device.
 *      EIP164_ARGUMENT_ERROR : Passed wrong argument.
 */
EIP164_Error_t
EIP164_Device_CountSummary_PSecY_Read(
        const EIP164_IOArea_t * const IOArea_p,
        const unsigned int Index,
        uint32_t * const CountSummaryPSecY_p);


/*----------------------------------------------------------------------------
 * EIP164_Device_CountSummary_PSecY_Clear
 *
 * This function clears the specified bits of the specified per-SecY counter
 * summary register. Note that only the set bits will be cleared.
 *
 * IOArea_p (input)
 *      Pointer to the Device instance I/O area in memory that identifies the
 *      EIP-164 device to be used.
 *
 * Index (input)
 *      Index of per-SecY counter summary register to be cleared.
 *
 * CountSummaryPSecY (input)
 *      A set of bits in one per-SecY counter summary register, each of them
 *      represents one SecY. If a bit is 1, it will be cleared.
 *
 * This function is NOT re-entrant for the same channel of the same Device.
 * This function is re-entrant for different Devices or different channel's
 * of the same Device.
 *
 * Return value:
 *      EIP164_NO_ERROR : Operation is completed
 *      EIP164_UNSUPPORTED_FEATURE_ERROR : Not supported by the device.
 *      EIP164_ARGUMENT_ERROR : Passed wrong argument.
 */
EIP164_Error_t
EIP164_Device_CountSummary_PSecY_Clear(
        const EIP164_IOArea_t * const IOArea_p,
        const unsigned int Index,
        const uint32_t CountSummaryPSecY);


/*----------------------------------------------------------------------------
 * EIP164_Device_CountSummary_SecY_Read
 *
 * This function retrieves the value of the specified SecY counter summary
 * register. Each bit represents a SecY statistics counter if set the
 * corresponding SecY statistics counter crossed the threshold.
 *
 * IOArea_p (input)
 *      Pointer to the Device instance I/O area in memory that identifies the
 *      EIP-164 device to be used.
 *
 * Index (input)
 *      Index of SecY counter summary register to be cleared.
 *
 * CountSummarySecY_p (output)
 *      Pointer to a memory location where the a set of bits in the SecY
 *      count summary register, each of them represents one counter.
 *      If a bit is 1, it means the counter it represents crossed the threshold.
 *
 * Return value:
 *      EIP164_NO_ERROR : Operation is completed
 *      EIP164_UNSUPPORTED_FEATURE_ERROR : Not supported by the device.
 *      EIP164_ARGUMENT_ERROR : Passed wrong argument.
 */
EIP164_Error_t
EIP164_Device_CountSummary_SecY_Read(
        const EIP164_IOArea_t * const IOArea_p,
        const unsigned int Index,
        uint32_t * const CountSummarySecY_p);


/*----------------------------------------------------------------------------
 * EIP164_Device_CountSummary_SecY_Clear
 *
 * This function clears the specified bits of the specified SecY counter
 * summary register. Note that only the set bits will be cleared.
 *
 * IOArea_p (input)
 *      Pointer to the Device instance I/O area in memory that identifies the
 *      EIP-164 device to be used.
 *
 * Index (input)
 *      Index of SecY counter summary register to be cleared.
 *
 * CountSummarySecY (input)
 *      A set of bits in the SecY counter summary register, each of them
 *      represents one counter. If a bit is 1, it will be cleared.
 *
 * This function is NOT re-entrant for the same channel of the same Device.
 * This function is re-entrant for different Devices or different channel's
 * of the same Device.
 *
 * Return value:
 *      EIP164_NO_ERROR : Operation is completed
 *      EIP164_UNSUPPORTED_FEATURE_ERROR : Not supported by the device.
 *      EIP164_ARGUMENT_ERROR : Passed wrong argument.
 */
EIP164_Error_t
EIP164_Device_CountSummary_SecY_Clear(
        const EIP164_IOArea_t * const IOArea_p,
        const unsigned int Index,
        const uint32_t CountSummarySecY);


/*----------------------------------------------------------------------------
 * EIP164_Device_CountSummary_PIFC_Read
 *
 * This function retrieves the value of the specified per-IFC counter summary
 * register. Each bit represents an IFC if set the one or more IFC statistics
 * counters crossed the threshold.
 *
 * IOArea_p (input)
 *      Pointer to the Device instance I/O area in memory that identifies the
 *      EIP-164 device to be used.
 *
 * Index (input)
 *      Index of per-IFC counter summary register to be read.
 *
 * CountSummaryPIFC_p (output)
 *      Pointer to a memory location where the a set of bits in one per-IFC
 *      count summary register, each of them represents one IFC. If a bit is 1,
 *      it means at least one of the IFC counters crossed the threshold.
 *
 * Return value:
 *      EIP164_NO_ERROR : Operation is completed
 *      EIP164_UNSUPPORTED_FEATURE_ERROR : Not supported by the device.
 *      EIP164_ARGUMENT_ERROR : Passed wrong argument.
 */
EIP164_Error_t
EIP164_Device_CountSummary_PIFC_Read(
        const EIP164_IOArea_t * const IOArea_p,
        const unsigned int Index,
        uint32_t * const CountSummaryPIFC_p);


/*----------------------------------------------------------------------------
 * EIP164_Device_CountSummary_PIFC_Clear
 *
 * This function clears the specified bits of the specified per-IFC counter
 * summary register. Note that only the set bits will be cleared.
 *
 * IOArea_p (input)
 *      Pointer to the Device instance I/O area in memory that identifies the
 *      EIP-164 device to be used.
 *
 * Index (input)
 *      Index of per-IFC counter summary register to be cleared.
 *
 * CountSummaryPIFC (input)
 *      A set of bits in one per-IFC counter summary register, each of them
 *      represents one IFC. If a bit is 1, it will be cleared.
 *
 * This function is NOT re-entrant for the same channel of the same Device.
 * This function is re-entrant for different Devices or different channel's
 * of the same Device.
 *
 * Return value:
 *      EIP164_NO_ERROR : Operation is completed
 *      EIP164_UNSUPPORTED_FEATURE_ERROR : Not supported by the device.
 *      EIP164_ARGUMENT_ERROR : Passed wrong argument.
 */
EIP164_Error_t
EIP164_Device_CountSummary_PIFC_Clear(
        const EIP164_IOArea_t * const IOArea_p,
        const unsigned int Index,
        const uint32_t CountSummaryPIFC);


/*----------------------------------------------------------------------------
 * EIP164_Device_CountSummary_IFC_Read
 *
 * This function retrieves the value of the specified IFC counter summary
 * register. Each bit represents an IFC statistics counter if set the
 * corresponding IFC statistics counter crossed the threshold.
 *
 * IOArea_p (input)
 *      Pointer to the Device instance I/O area in memory that identifies the
 *      EIP-164 device to be used.
 *
 * Index (input)
 *      Index of IFC counter summary register to be cleared.
 *
 * CountSummaryIFC_p (output)
 *      Pointer to a memory location where the a set of bits in the IFC
 *      count summary register, each of them represents one counter.
 *      If a bit is 1, it means the counter it represents crossed the threshold.
 *
 * Return value:
 *      EIP164_NO_ERROR : Operation is completed
 *      EIP164_UNSUPPORTED_FEATURE_ERROR : Not supported by the device.
 *      EIP164_ARGUMENT_ERROR : Passed wrong argument.
 */
EIP164_Error_t
EIP164_Device_CountSummary_IFC_Read(
        const EIP164_IOArea_t * const IOArea_p,
        const unsigned int Index,
        uint32_t * const CountSummaryIFC_p);


/*----------------------------------------------------------------------------
 * EIP164_Device_CountSummary_IFC_Clear
 *
 * This function clears the specified bits of the specified IFC counter
 * summary register. Note that only the set bits will be cleared.
 *
 * IOArea_p (input)
 *      Pointer to the Device instance I/O area in memory that identifies the
 *      EIP-164 device to be used.
 *
 * Index (input)
 *      Index of IFC counter summary register to be cleared.
 *
 * CountSummaryIFC (input)
 *      A set of bits in the IFC counter summary register, each of them
 *      represents one counter. If a bit is 1, it will be cleared.
 *
 * This function is NOT re-entrant for the same channel of the same Device.
 * This function is re-entrant for different Devices or different channel's
 * of the same Device.
 *
 * Return value:
 *      EIP164_NO_ERROR : Operation is completed
 *      EIP164_UNSUPPORTED_FEATURE_ERROR : Not supported by the device.
 *      EIP164_ARGUMENT_ERROR : Passed wrong argument.
 */
EIP164_Error_t
EIP164_Device_CountSummary_IFC_Clear(
        const EIP164_IOArea_t * const IOArea_p,
        const unsigned int Index,
        const uint32_t CountSummaryIFC);


/*----------------------------------------------------------------------------
 * EIP164_Device_CountSummary_PIFC1_Read
 *
 * This function retrieves the value of the specified per-IFC1 counter summary
 * register. Each bit represents an IFC1 if set the one or more IFC1 statistics
 * counters crossed the threshold.
 *
 * IOArea_p (input)
 *      Pointer to the Device instance I/O area in memory that identifies the
 *      EIP-164 device to be used.
 *
 * Index (input)
 *      Index of per-IFC1 counter summary register to be read.
 *
 * CountSummaryPIFC1_p (output)
 *      Pointer to a memory location where the a set of bits in one per-IFC1
 *      count summary register, each of them represents one IFC1. If a bit is 1,
 *      it means at least one of the IFC1 counters crossed the threshold.
 *
 * Return value:
 *      EIP164_NO_ERROR : Operation is completed
 *      EIP164_UNSUPPORTED_FEATURE_ERROR : Not supported by the device.
 *      EIP164_ARGUMENT_ERROR : Passed wrong argument.
 */
EIP164_Error_t
EIP164_Device_CountSummary_PIFC1_Read(
        const EIP164_IOArea_t * const IOArea_p,
        const unsigned int Index,
        uint32_t * const CountSummaryPIFC1_p);


/*----------------------------------------------------------------------------
 * EIP164_Device_CountSummary_PIFC1_Clear
 *
 * This function clears the specified bits of the specified per-IFC1 counter
 * summary register. Note that only the set bits will be cleared.
 *
 * IOArea_p (input)
 *      Pointer to the Device instance I/O area in memory that identifies the
 *      EIP-164 device to be used.
 *
 * Index (input)
 *      Index of per-IFC1 counter summary register to be cleared.
 *
 * CountSummaryPIFC1 (input)
 *      A set of bits in one per-IFC1 counter summary register, each of them
 *      represents one IFC1. If a bit is 1, it will be cleared.
 *
 * This function is NOT re-entrant for the same channel of the same Device.
 * This function is re-entrant for different Devices or different channel's
 * of the same Device.
 *
 * Return value:
 *      EIP164_NO_ERROR : Operation is completed
 *      EIP164_UNSUPPORTED_FEATURE_ERROR : Not supported by the device.
 *      EIP164_ARGUMENT_ERROR : Passed wrong argument.
 */
EIP164_Error_t
EIP164_Device_CountSummary_PIFC1_Clear(
        const EIP164_IOArea_t * const IOArea_p,
        const unsigned int Index,
        const uint32_t CountSummaryPIFC1);


/*----------------------------------------------------------------------------
 * EIP164_Device_CountSummary_IFC1_Read
 *
 * This function retrieves the value of the specified IFC1 counter summary
 * register. Each bit represents an IFC1 statistics counter if set the
 * corresponding IFC1 statistics counter crossed the threshold.
 *
 * IOArea_p (input)
 *      Pointer to the Device instance I/O area in memory that identifies the
 *      EIP-164 device to be used.
 *
 * Index (input)
 *      Index of IFC1 counter summary register to be cleared.
 *
 * CountSummaryIFC_p (output)
 *      Pointer to a memory location where the a set of bits in the IFC1
 *      count summary register, each of them represents one counter.
 *      If a bit is 1, it means the counter it represents crossed the threshold.
 *
 * Return value:
 *      EIP164_NO_ERROR : Operation is completed
 *      EIP164_UNSUPPORTED_FEATURE_ERROR : Not supported by the device.
 *      EIP164_ARGUMENT_ERROR : Passed wrong argument.
 */
EIP164_Error_t
EIP164_Device_CountSummary_IFC1_Read(
        const EIP164_IOArea_t * const IOArea_p,
        const unsigned int Index,
        uint32_t * const CountSummaryIFC1_p);


/*----------------------------------------------------------------------------
 * EIP164_Device_CountSummary_IFC1_Clear
 *
 * This function clears the specified bits of the specified IFC1 counter
 * summary register. Note that only the set bits will be cleared.
 *
 * IOArea_p (input)
 *      Pointer to the Device instance I/O area in memory that identifies the
 *      EIP-164 device to be used.
 *
 * Index (input)
 *      Index of IFC1 counter summary register to be cleared.
 *
 * CountSummaryIFC1 (input)
 *      A set of bits in the IFC1 counter summary register, each of them
 *      represents one counter. If a bit is 1, it will be cleared.
 *
 * This function is NOT re-entrant for the same channel of the same Device.
 * This function is re-entrant for different Devices or different channel's
 * of the same Device.
 *
 * Return value:
 *      EIP164_NO_ERROR : Operation is completed
 *      EIP164_UNSUPPORTED_FEATURE_ERROR : Not supported by the device.
 *      EIP164_ARGUMENT_ERROR : Passed wrong argument.
 */
EIP164_Error_t
EIP164_Device_CountSummary_IFC1_Clear(
        const EIP164_IOArea_t * const IOArea_p,
        const unsigned int Index,
        const uint32_t CountSummaryIFC1);


/*----------------------------------------------------------------------------
 * EIP164_Device_CountSummary_PRxCAM_Read
 *
 * This function retrieves the value of the specified per-RxCAM counter summary
 * register. Each bit represents a RxCAM hit counter if set the statistics
 * counter crossed the threshold.
 *
 * IOArea_p (input)
 *      Pointer to the Device instance I/O area in memory that identifies the
 *      EIP-164 device to be used.
 *
 * Index (input)
 *      Index of per-RxCAM counter summary register to be read.
 *
 * CountSummary_p (output)
 *      Pointer to a memory location where the a set of bits in one per-RxCAM
 *      count summary register, each of them represents one RxCAM hit counter.
 *      If a bit is 1, it means that the RxCAM hit counter has crossed the
 *      threshold.
 *
 * Return value:
 *      EIP164_NO_ERROR : Operation is completed
 *      EIP164_UNSUPPORTED_FEATURE_ERROR : Not supported by the device.
 *      EIP164_ARGUMENT_ERROR : Passed wrong argument.
 */
EIP164_Error_t
EIP164_Device_CountSummary_PRxCAM_Read(
        const EIP164_IOArea_t * const IOArea_p,
        const unsigned int Index,
        uint32_t * const CountSummary_p);


/*----------------------------------------------------------------------------
 * EIP164_Device_CountSummary_PRxCAM_Clear
 *
 * This function clears the specified bits of the specified per-RxCAM counter
 * summary register. Note that only the set bits will be cleared.
 *
 * IOArea_p (input)
 *      Pointer to the Device instance I/O area in memory that identifies the
 *      EIP-164 device to be used.
 *
 * Index (input)
 *      Index of per-RxCAM counter summary register to be cleared.
 *
 * CountSummary (input)
 *      A set of bits in one per-RxCAM counter summary register, each of them
 *      represents one RxCAM hit counter. If a bit is 1, it will be cleared.
 *
 * This function is NOT re-entrant for the same channel of the same Device.
 * This function is re-entrant for different Devices or different channel's
 * of the same Device.
 *
 * Return value:
 *      EIP164_NO_ERROR : Operation is completed
 *      EIP164_UNSUPPORTED_FEATURE_ERROR : Not supported by the device.
 *      EIP164_ARGUMENT_ERROR : Passed wrong argument.
 */
EIP164_Error_t
EIP164_Device_CountSummary_PRxCAM_Clear(
        const EIP164_IOArea_t * const IOArea_p,
        const unsigned int Index,
        const uint32_t CountSummary);


#endif /* EIP164_SECY_H_ */


/* end of file eip164_secy.h */
