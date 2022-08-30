/* eip164_types.h
 *
 * EIP-164 Driver Library Common Types Definitions
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

#ifndef EIP164_TYPES_H_
#define EIP164_TYPES_H_

/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

/* Default configuration */
#include <Kit/EIP164/incl/c_eip164.h>

/* Driver Framework Basic Definitions API */
#include <Kit/DriverFramework/incl/basic_defs.h>      /* uint32_t */


/*----------------------------------------------------------------------------
 * Definitions and macros
 */

/** Counter increment disable control values\n
    Each bit disables all the counters of one counter type.
    Counter types:
        - SA counters
        - Interface counters
        - Interface1 counters
        - SecY counters
        - Channel counters
        - RxCAM counters */
#define EIP164_SECY_SA_COUNT_INC_DIS       BIT_0
#define EIP164_SECY_IFC_COUNT_INC_DIS      BIT_1
#define EIP164_SECY_IFC1_COUNT_INC_DIS     BIT_2
#define EIP164_SECY_SECY_COUNT_INC_DIS     BIT_3
#define EIP164_SECY_RXCAM_COUNT_INC_DIS    BIT_5

/* I/O Area size for one device instance */
#define EIP164_IOAREA_REQUIRED_SIZE               (sizeof(void*)+8*sizeof(int))

/* Maximum number of packet classification rules */
/* using MAC destination address and Ethernet type */
#define EIP164_CFYE_MAC_DA_ET_MATCH_RULES_COUNT   10

/* Maximum number of packet classification rules */
/* using just MAC EtherType */
#define EIP164_CFYE_MAC_ET_MATCH_RULES_COUNT      8

#define EIP164_CHANNEL_WORDS 2

/** Channel Mask */
typedef struct {
    uint32_t ch_bitmask[EIP164_CHANNEL_WORDS];
} EIP164_Ch_Mask_t;

typedef struct
{
    uint32_t low;
    uint32_t hi;
} EIP164_UI64_t;

/*----------------------------------------------------------------------------
 * EIP164_Error_t
 *
 * Status (error) code type returned by these API functions
 * See each function "Return value" for details.
 *
 * EIP164_NO_ERROR : successful completion of the call.
 * EIP164_UNSUPPORTED_FEATURE_ERROR : not supported by the device.
 * EIP164_ARGUMENT_ERROR :  invalid argument for a function parameter.
 * EIP164_BUSY_RETRY_LATER : device is busy.
 * EIP164_HW_CONFIGURATION_MISMATCH : HW configuration does not support
 *                                    requested operation.
 * Any other integer value: Error return from device read or write function.
 *
 * Note: any functions that access hardware can return error codes
 *       returned by device read or write functions.
 */
enum
{
    EIP164_NO_ERROR = 0,
    EIP164_UNSUPPORTED_FEATURE_ERROR,
    EIP164_ARGUMENT_ERROR,
    EIP164_BUSY_RETRY_LATER,
    EIP164_HW_CONFIGURATION_MISMATCH
};
typedef int EIP164_Error_t;

/* place holder for device specific internal data */
typedef struct
{
    void * placeholder[(EIP164_IOAREA_REQUIRED_SIZE + sizeof(void *) - 1)/sizeof(void *)];
} EIP164_IOArea_t;

typedef enum
{
    EIP164_MODE_EGRESS,
    EIP164_MODE_INGRESS,
    EIP164_MODE_EGRESS_INGRESS
} EIP164_Device_Mode_t;

/* Validate frame values, */
typedef enum
{
    EIP164_VALIDATE_FRAME_DISABLED,
    EIP164_VALIDATE_FRAME_CHECK,
    EIP164_VALIDATE_FRAME_STRICT
} EIP164_ValidateFrame_t;

/* SecY Packet Number Threshold configuration */
typedef struct
{
    /* Outbound sequence number threshold value */
    /* When non-0 the SecY device will generate a notification to indicate */
    /* the threshold event which can be used to start the re-keying procedure. */
    /* The notification will be generated only if it is requested. */
    /* If set to zero then only the sequence number roll-over notification */
    /* will be generated. */
    uint32_t SeqNrThreshold;

    /* Outbound sequence number threshold value for 64-bit packet numbering */
    EIP164_UI64_t SeqNrThreshold64;
} EIP164_PN_Threshold_t;

/* Generic EIP HW version */
typedef struct
{
    /* The basic EIP number. */
    uint8_t EipNumber;

    /* The complement of the basic EIP number. */
    uint8_t ComplmtEipNumber;

    /* Hardware Patch Level. */
    uint8_t HWPatchLevel;

    /* Minor Hardware revision. */
    uint8_t MinHWRevision;

    /* Major Hardware revision. */
    uint8_t MajHWRevision;
} EIP164_Version_t;

/* Configuration options of EIP-164 HW */
typedef struct
{
    /* Number of supported SA's */
    uint16_t    SA_Count;

    /* Number of supported channels */
    uint8_t     Channel_Count;

    /* Number of supported vPorts */
    uint16_t    vPort_Count;

    /* Number of supported Secure Channels */
    uint16_t    SC_Count;

    /* Hardware has external RxCAM (applies to ingress only) */
    bool        fExternalRxCAM;

    /* Spare register available */
    bool        fSpareReg;

    /* Specialize the engine for ingress functionality and counters only, */
    /* true for -i, false for -e, false for -ie. */
    bool        fIngressOnly;

    /* Specialize the engine for egress functionality and counters only, */
    /* false for -i, true for -e, false for -ie. */
    bool        fEgressOnly;
} EIP164_Options_t;


/* Configuration options of EIP-164 HW */
typedef struct
{
    /* Set if the core is egress-only, cleared for ingress or ingress-egress. */
    bool        fEgressOnly;

    /* Set if the core is ingress-only, cleared for egress or ingress-egress. */
    bool        fIngressOnly;

    /* Number of channels supported by the core. */
    uint8_t     ChannelCount;

    /* Number of processing pipelines. */
    uint8_t     PipeCount;
} EIP66_Options_t;


/* Capabilities structure for EIP-164 HW */
typedef struct
{
    /* EIP-164 */
    EIP164_Options_t    EIP164_Options;
    EIP164_Version_t       EIP164_Version;

    /* Processing Engine */
    EIP66_Options_t     EIP66_Options;
    EIP164_Version_t       EIP66_Version;
} EIP164_Capabilities_t;

/* Types of ports */
typedef enum
{
    EIP164_PORT_COMMON,
    EIP164_PORT_RESERVED,
    EIP164_PORT_CONTROLLED,
    EIP164_PORT_UNCONTROLLED
} EIP164_PortType_t;

/*----------------------------------------------------------------------------
 * EIP164_SecY_DropType_t
 *
 * SA drop type:
 *      bypass with CRC error,
 *      bypass with packet error,
 *      internal drop by crypto device.
 *
 */
typedef enum
{
    EIP164_SECY_SA_DROP_CRC_ERROR,
    EIP164_SECY_SA_DROP_PKT_ERROR,
    EIP164_SECY_SA_DROP_INTERNAL,
    EIP164_SECY_SA_DO_NOT_DROP          /* For debug purpose only */
} EIP164_DropType_t;

/* MACsec SecTAG parsing rule, */
/* This rule classifies each packet into one of four categories, */
/* see also the EIP164_Rules_SA_NonMatch_t data structure: */
/* 1) Untagged, the packet has no MACsec tag, */
/*    i.e. the Ether-type differs from 0x88E5. */
/* 2) Bad tag, the packet has an invalid MACsec tag */
/* 3) KaY tag, the packet has a KaY tag. These packets are generated and/or */
/*    handled by application software and no MACsec processing is performed */
/*    for them by the Classification device except for straight bypass. */
/* 4) Tagged, the packet has a valid MACsec tag that is not KaY. */
typedef struct
{
    /* Compare Ether-type in packet against EtherType value. */
    /* false - all packets are treated as MACsec */
    /*         (no packets are classified as untagged). */
    /* default is true */
    bool fCompEType;

    /* true - check V bit to be 0 */
    /* default is true */
    bool fCheckV;

    /* true - check if this is a KaY packet (C and E bits) */
    /* default is true */
    bool fCheckKay;

    /* true - check illegal C and E bits combination (C=1 and E=0) */
    /* default is true */
    bool fCheckCE;

    /* true - check illegal SC/ES/SCB bits combinations */
    /* default is true */
    bool fCheckSC;

    /* true - check if SL (Short Length) field value is out of range */
    /* default is true */
    bool fCheckSL;

    /* true - check PN (Packet Number) is non-zero */
    /* default is true */
    bool fCheckPN;

    /* true- check if SL (Short Length) field value is consistent with */
    /* packet length */
    bool fCheckSLExt;


    /* Ether-type value used for MACsec tag type comparison, */
    /* default is 0x88E5 */
    uint16_t EtherType;
} EIP164_Channel_Rule_SecTAG_t;

/* Actions per packet that did not match any SA */
typedef struct
{
    /* Flow action type */
    /* true - bypass, */
    /* false - perform drop action, see EIP164_SecY_DropType_t */
    bool fBypass;

    /* Packet drop type, see EIP164_SecY_DropType_t */
    EIP164_DropType_t DropType;

    /* Ingress only */
    /* Perform drop action if packet is not from the reserved port */
    bool fDropNonReserved;

    EIP164_PortType_t DestPort;

} EIP164_Rules_NonSA_t;

/* Actions per packet type for packets which did not match any SA, */
/* see EIP164_Channel_Rule_SecTAG_t rules which are used to classify packets into these */
/* categories */
typedef struct
{
    EIP164_Rules_NonSA_t Untagged;
    EIP164_Rules_NonSA_t Tagged;
    EIP164_Rules_NonSA_t BadTag;
    EIP164_Rules_NonSA_t KaY;
} EIP164_Rules_SA_NonMatch_t;

/* Device settings */
typedef struct
{
    /* Device operation mode */
    EIP164_Device_Mode_t Mode;

    /* When true the statistics counters are automatically reset */
    /* on a read operation */
    bool fAutoStatCntrsReset;

    /* This mask specifies which SA-related counter increments are regarded a */
    /* security fail event - bit [0] is for the first 64-bits counter of an */
    /* SA-related counter set. */
    /* Note: Actual width depends on the number of SA counters implemented. */
    uint16_t SA_SecFail_Mask;

    /* This mask specifies which SecY counter increments are regarded a */
    /* security fail event - bit [0] is for the first 64-bits global counter. */
    /* Note: Actual width depends on the number of SecY counters implemented. */
    uint32_t SecY_SecFail_Mask;

    /* This mask specifies which global counter increments are regarded a */
    /* security fail event - bit [0] is for the first 64-bits global counter. */
    /* Note: Actual width depends on the number of global counters implemented. */
    uint32_t Global_SecFail_Mask;

    /* The fixed packet latency, if set to 0 then no fixed latency will be used */
    /* Latency + 4 = engine clock cycles */
    /* NOTE: do not set this value above 26 when setting the fStaticBypass. */
    uint16_t Latency;

    /* Disable the MACsec crypto-core (EIP-66), */
    /* send the packets around it to minimize latency */
    bool fStaticBypass;

    /* Outbound sequence number threshold value (one global for all SA's) */
    /* When non-0 the device will generate an interrupt to indicate */
    /* the threshold event which can be used to start the re-keying procedure */
    /* If set to zero then only the sequence number roll-over interrupt */
    /* will be generated. */
    uint32_t SeqNrThreshold;

    /* Outbound sequence number threshold value for 64-bit packet numbering */
    uint32_t SeqNrThreshold64Lo;
    uint32_t SeqNrThreshold64Hi;

    /* Threshold for the frame counters */
    uint32_t SACountFrameThrLo;
    uint32_t SACountFrameThrHi;
    uint32_t SecYCountFrameThrLo;
    uint32_t SecYCountFrameThrHi;
    uint32_t IFCCountFrameThrLo;
    uint32_t IFCCountFrameThrHi;
    uint32_t IFC1CountFrameThrLo;
    uint32_t IFC1CountFrameThrHi;
    uint32_t RxCAMCountFrameThrLo;
    uint32_t RxCAMCountFrameThrHi;

    /* Threshold for the octet counters */
    uint32_t SACountOctetThrLo;
    uint32_t SACountOctetThrHi;
    uint32_t IFCCountOctetThrLo;
    uint32_t IFCCountOctetThrHi;
    uint32_t IFC1CountOctetThrLo;
    uint32_t IFC1CountOctetThrHi;

    /* EOP timeout settings */
    unsigned int EOPTimeoutVal;
    EIP164_Ch_Mask_t EOPTimeoutCtrl;

    /* Threshold for ECC settings. */
    unsigned int ECCCorrectableThr;
    unsigned int ECCUncorrectableThr;

    /* Counter increment enable control */
    uint8_t CountIncDisCtrl;

    /* Enable the Crypt-authenticate operation (all vPorts) */
    bool fFlowCyptAuth;

    /* Initial processing rules for default/bypass flows (all vPorts) */
    EIP164_Rules_NonSA_t DropBypass;

} EIP164_Settings_t;

typedef struct
{
    /* When true the statistics counters are automatically reset */
    /* on a read operation */
    bool fAutoStatCntrsReset;

    /* Outbound sequence number threshold value (one global for all SA's) */
    /* When non-0 the SecY device will generate a notification to indicate */
    /* the threshold event which can be used to start the re-keying procedure. */
    /* The notification will be generated only if it is requested. */
    /* If set to zero then only the sequence number roll-over notification */
    /* will be generated. */
    /* Note: This is a global parameter for all the SA's added to */
    /*       one SecY device. */
    uint32_t SeqNrThreshold;

    /* Outbound sequence number threshold value for 64-bit packet numbering */
    EIP164_UI64_t SeqNrThreshold64;

    /* Threshold for the frame counters */
    EIP164_UI64_t SACountFrameThr;
    EIP164_UI64_t SecYCountFrameThr;
    EIP164_UI64_t IFCCountFrameThr;
    EIP164_UI64_t IFC1CountFrameThr;
    EIP164_UI64_t RxCAMCountFrameThr;

    /* Threshold for the octet counters */
    EIP164_UI64_t SACountOctetThr;
    EIP164_UI64_t IFCCountOctetThr;
    EIP164_UI64_t IFC1CountOctetThr;

} EIP164_Statistics_Control_t;


#endif /* EIP164_TYPES_H_ */


/* end of file eip164_types.h */
