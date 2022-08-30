/* eip163.h
 *
 * EIP-163 Driver Library Classification Engine API
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

#ifndef EIP163_H_
#define EIP163_H_

/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

/* Driver Framework Basic Definitions API */
#include <Kit/DriverFramework/incl/basic_defs.h>

/* Driver Framework Device API */
#include <Kit/DriverFramework/incl/device_types.h>           /* Device_Handle_t */


/*----------------------------------------------------------------------------
 * Definitions and macros
 */

/** Counter increment disable control values\n
    Each bit disables all the counters of one counter type.
    Counter types:
        - TCAM counters
        - Channel counters */
#define EIP163_CFYE_TCAM_COUNT_INC_DIS     BIT_0
#define EIP163_CFYE_CHAN_COUNT_INC_DIS     BIT_1

/* I/O Area size for one device instance */
#define EIP163_IOAREA_REQUIRED_SIZE                 (sizeof(void*)+10*sizeof(int))

/* Clock bitmask specification for clock control */
#define EIP163_DEVICE_ICE_CLOCK                     BIT_0   /* Classification */
                                                            /* engine */
#define EIP163_DEVICE_TCAM_CLOCK                    BIT_1   /* TCAM */
#define EIP163_DEVICE_ICE_DATA_CLOCK                BIT_2   /* ICE Data path */
#define EIP163_DEVICE_STATS_CLOCK                   BIT_3   /* Statistics */

/* Maximum number of non-control words in one TCAM entry (key or mask) */
#define EIP163_RULE_NON_CTRL_WORD_COUNT             4


/* Maximum number of VLAN user priority values */
#define EIP163_VLAN_UP_MAX_COUNT                    8

#define EIP163_MAC_DA_ET_MATCH_RULES_COUNT          8
#define EIP163_MAC_DA_ET_RANGE_MATCH_RULES_COUNT    2

/* Classification device role: either Egress only or Ingress only */
typedef enum
{
    EIP163_ROLE_EGRESS,
    EIP163_ROLE_INGRESS
} EIP163_Role_t;

/* Drop type */
typedef enum
{
    EIP163_DROP_CRC_ERROR,  /* bypass with CRC error */
    EIP163_DROP_PKT_ERROR,  /* bypass with packet error */
    EIP163_DROP_INTERNAL,   /* internal drop by external logic */
    EIP163_DO_NOT_DROP      /* do not drop, for debug purposes only */
} EIP163_DropType_t;

/*----------------------------------------------------------------------------
 * EIP163_Error_t
 *
 * Status (error) code type returned by these API functions
 * See each function "Return value" for details.
 *
 * EIP163_NO_ERROR : successful completion of the call.
 * EIP163_UNSUPPORTED_FEATURE_ERROR : not supported by the device.
 * EIP163_ARGUMENT_ERROR :  invalid argument for a function parameter.
 * EIP163_BUSY_RETRY_LATER : device is busy.
 * EIP163_HW_CONFIGURATION_MISMATCH : HW configuration does not support
 *                                    requested operation.
 * Any other integer value: Error return from device read or write function.
 *
 * Note: any functions that access hardware can return error codes
 *       returned by device read or write functions.
 */
enum
{
    EIP163_NO_ERROR = 0,
    EIP163_UNSUPPORTED_FEATURE_ERROR,
    EIP163_ARGUMENT_ERROR,
    EIP163_BUSY_RETRY_LATER,
    EIP163_HW_CONFIGURATION_MISMATCH
};

typedef int EIP163_Error_t;

#define EIP163_CHANNEL_WORDS 2

/** Channel Mask */
typedef struct {
    uint32_t ch_bitmask[EIP163_CHANNEL_WORDS];
} EIP163_Ch_Mask_t;

/* place holder for device specific internal data */
typedef struct
{
    void * placeholder[(EIP163_IOAREA_REQUIRED_SIZE + sizeof(void*) - 1)/sizeof(void*)];
} EIP163_IOArea_t;

/* EIP-163 device mode: egress or ingress */
typedef enum
{
    EIP163_MODE_EGRESS,
    EIP163_MODE_INGRESS
} EIP163_Device_Mode_t;

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
} EIP163_Version_t;

/* Configuration options of EIP-163 device */
typedef struct
{
    /** Number of supported channels */
    uint8_t     Channel_Count;

    /** Number of supported TCAM rules */
    uint16_t    vPort_Count;

    /** Number of supported vPorts */
    uint16_t    Rules_Count;

    /* Specialize the engine for ingress functionality and counters only, */
    /* true for -i, false for -e, false for -ie. */
    bool        fIngressOnly;

    /* Specialize the engine for egress functionality and counters only, */
    /* false for -i, true for -e, false for -ie. */
    bool        fEgressOnly;

    /* When true then the SecTAG lookup operation is supported */
    bool        fLookupSCI;

    /* When true the TCAM is external. */
    bool        fExternalTCAM;

} EIP163_Options_t;

/* Configuration options of EIP-217 device */
typedef struct
{
    /* Number of supported TCAM hit counters */
    uint16_t    TCAMHitCounters_Count;

    /* TCAM hit counters width (in bits) */
    uint8_t     TCAMHitCountersWidth_BitCount;

    /* Number of supported TCAM hit packet counters */
    uint8_t     TCAMHitPktCounters_Count;
} EIP163_TCAM_Options_t;

/* Capabilities structure for EIP-217 device (statistics module) */
typedef struct
{
    /* EIP-163 */
    EIP163_Options_t       EIP163_Options;
    EIP163_Version_t       EIP163_Version;

    /* EIP-217 TCAM */
    EIP163_TCAM_Options_t  TCAM_Options;
    EIP163_Version_t       TCAM_Version;
} EIP163_Capabilities_t;

/* TCAM initialization data structure */
typedef struct
{
    /* Pointer to memory location where an array of 32-bit words */
    /* is stored. This array will be copied to the TCAM memory during */
    /* the EIP-163 device initialization. */
    uint32_t * InitData_p;

    /* Size of the InitData_p array in 32-bit words */
    unsigned int InitData_WordCount;

    /* 32-bit word offset in TCAM where the InitData_p array must be written */
    unsigned int WordOffset;
} EIP163_TCAM_Init_t;

/* Device settings */
typedef struct
{
    /* Device operation mode */
    EIP163_Device_Mode_t Mode;

    /* When true the statistics counters are automatically reset */
    /* on a read operation */
    bool fAutoStatCntrsReset;

    /* When true all channels will be initialized for low-latency bypass mode */
    bool fLowLatencyBypass;

    /* TCAM initialization data */
    EIP163_TCAM_Init_t TCAMInit;

    /* Threshold for the packet counters */
    uint32_t CountFrameThrLo;
    uint32_t CountFrameThrHi;

    /* Threshold for the per-channel packet counters */
    uint32_t ChanCountFrameThrLo;
    uint32_t ChanCountFrameThrHi;

    /* EOP timeout settings */
    unsigned int EOPTimeoutVal;
    EIP163_Ch_Mask_t EOPTimeoutCtrl;

    /* Threshold for ECC settings. */
    unsigned int ECCCorrectableThr;
    unsigned int ECCUncorrectableThr;

    /* Counter increment enable control */
    uint8_t CountIncDisCtrl;

} EIP163_Settings_t;

/* Exception cases control */
typedef struct
{
    /* Defines the way the drop operation is performed if the TCAM is missed */
    EIP163_DropType_t DropAction;

    /* Facilitates channel reset sequence and allows in single write drop all */
    /* incoming packets for a channel */
    /* true  - force dropping for all packets for a given channel */
    /* false - do not force dropping */
    bool fForceDrop;

    /* Allows passing packets that missed the TCAM through, for this DropAction */
    /* must not be set to EIP163_DROP_INTERNAL */
    /* true -  allow assigning a default vPort for packet that missed vPort */
    /*         matching */
    /* false - do not change the results of the vPort matching */
    bool fDefaultVPortValid;

    /* Default vPort value, ignored if fDefaultVPortValid is set to false */
    uint16_t DefaultVPort;
} EIP163_Device_Exceptions_t;

/* Device control */
typedef struct
{
    /* Indication if the low latency bypass must be enabled (true) */
    /* or the MACsec mode (false). */
    bool fLowLatencyBypass;

    /* Policy used to configure actions for exception cases */
    /* Set to NULL if no update is required */
    EIP163_Device_Exceptions_t * Exceptions_p;
} EIP163_Device_Control_t;


/* MACsec SecTAG parser control */
typedef struct
{
    /* true - compare Ether-type in packet against MACsecTagValue value */
    bool fCompType;

    /* true - check V (Version) bit to be 0 */
    bool fCheckVersion;

    /* true - check if packet is meant to be handled by KaY (C & E bits) */
    bool fCheckKay;

    /* This flag controls key generation for packets with SecTAG after MAC SA. */
    /* true - enable generation of TCAM key with packet type MACsec (TCI, SCI) */
    /* false - always generate normal TCAM keys */
    bool fLookupUseSCI;

    /* Ether-type value used for MACsec tag type comparison (stored byte swapped) */
    /* Default value 0x88E5 (stored as 0xE588) is the standard MACsec tag value */
    uint16_t MACsecTagValue;
} EIP163_SecTAG_Parser_t;

typedef struct
{
    /* Enable detection of VLAN tags matching QTag value */
    bool fParseQTag;

    /* Enable detection of VLAN tags matching STag1 value */
    bool fParseStag1;

    /* Enable detection of VLAN tags matching STag2 value */
    bool fParseStag2;

    /* Enable detection of VLAN tags matching STag3 value */
    bool fParseStag3;

    /* Enable detection of multiple back-2-back VLAN tags (Q-in-Q and beyond) */
    bool fParseQinQ;
} EIP163_VLAN_ParseTag_t;

/* VLAN parsing control */
/* The EIP-163 is able to detect up to five VLAN tags following */
/* the outer MAC_SA and parse the first two for packet classification */
typedef struct
{
    /* 1. Per-channel VLAN parsing settings */

    /* Input header parser VLAN tags parsing settings */
    EIP163_VLAN_ParseTag_t CP;

    /* true - update the secondary header parser VLAN tags parsing settings */
    /*        with values from the SCP data structure member */
    /* false - no update is required */
    bool fSCP;

    /* Secondary header parser VLAN tags parsing settings */
    EIP163_VLAN_ParseTag_t SCP;

    /* 2. Global VLAN parsing settings */

    /* Enable user priority processing for '802.1s' packets. If set, */
    /* when a '802.1s' tag is found, take the user priority directly from */
    /* the PCP field, otherwise take the default user priority. */
    bool fSTagUpEnable;

    /* Enable user priority processing for '802.1Q' packets. If set, */
    /* when a '802.1Q' tag is found, translate the PCP field using UpTable1, */
    /* otherwise take the default user priority. */
    bool fQTagUpEnable;

    /* Default user priority, assigned to non-VLAN packets and */
    /* to VLAN packets for which the VLAN user priority processing is disabled */
    uint8_t DefaultUp; /* Allowed values in range 0 .. 7! */

    /* Translation tables to derive the user priority from the PCP field */
    /* in '802.1Q' tags. If the PCP field is 0 then take UpTable[0] as the user */
    /* priority, if PCP=1 then take UpTable[1], etc. */

    /* Translation table for 1st '802.1Q' tag, allowed values in range 0 .. 7! */
    uint8_t UpTable1[EIP163_VLAN_UP_MAX_COUNT];

    /* Translation table for 2nd '802.1Q' tag, allowed values in range 0 .. 7! */
    uint8_t UpTable2[EIP163_VLAN_UP_MAX_COUNT];

    /* Ethertype value used for '802.1Q' tag type comparison */
    uint16_t QTag;

    /* Ethertype value used for '802.1s' tag type comparison */
    uint16_t STag1;
    uint16_t STag2;
    uint16_t STag3;
} EIP163_VLAN_Parser_t;

/* Header Parser control */
typedef struct
{
    /* MACsec SecTAG parser control settings. */
    /* Set to NULL if no update is required */
    EIP163_SecTAG_Parser_t *SecTAG_Parser_p;

    /* VLAN header settings. Set to NULL if no update is required */
    EIP163_VLAN_Parser_t *VLAN_Parser_p;
} EIP163_HeaderParser_t;

/* Packet match rule using MAC destination address and EtherType */
typedef struct
{
    /* MAC destination address, 6 bytes */
    uint8_t * MAC_DA_p;

    /* EtherType */
    uint16_t EtherType;
} EIP163_MAC_DA_ET_MatchRule_t;

/* Packet match rule using range of MAC destination addresses */
typedef struct
{
    /* Start MAC destination address, 6 bytes */
    uint8_t * MAC_DA_Start_p;

    /* End MAC destination address, 6 bytes */
    uint8_t * MAC_DA_End_p;
} EIP163_MAC_DA_Range_MatchRule_t;

/* Packet match rule using range of MAC destination addresses and EtherType */
typedef struct
{
    /* Range of MAC destination addresses, start and end */
    EIP163_MAC_DA_Range_MatchRule_t Range;

    /* EtherType */
    uint16_t EtherType;
} EIP163_MAC_DA_ET_Range_MatchRule_t;

/* Control Packet Detector settings */
typedef struct
{
    /* MAC destination address and EtherType */
    /* Set MAC_DA_ET_Rules[n].MAC_DA_p to NULL if no update is required, */
    /* corresponding bits in CPMatchEnableMask and SCPMatchEnableMask should */
    /* be set to 0 */
    EIP163_MAC_DA_ET_MatchRule_t MAC_DA_ET_Rules[EIP163_MAC_DA_ET_MATCH_RULES_COUNT];

    /* Range of MAC destination address used with EtherType */
    /* Set MAC_DA_ET_Range[n].Range.MAC_DA_Start_p and */
    /* MAC_DA_ET_Range[n].Range.MAC_DA_End_p to NULL if no update is required, */
    /* corresponding bits in CPMatchEnableMask and SCPMatchEnableMask should */
    /* be set to 0 */
    EIP163_MAC_DA_ET_Range_MatchRule_t MAC_DA_ET_Range[EIP163_MAC_DA_ET_RANGE_MATCH_RULES_COUNT];

    /* Range of MAC destination address (no EtherType) */
    /* Set MAC_DA_Range[n].MAC_DA_Start_p and MAC_DA_Range[n].MAC_DA_End_p to NULL */
    /* if no update is required, corresponding bits in CPMatchEnableMask and */
    /* SCPMatchEnableMask should be set to 0 */
    EIP163_MAC_DA_Range_MatchRule_t MAC_DA_Range;

    /* 44-bit MAC destination address constant, 6 bytes */
    /* Set to NULL if no updated is required, corresponding bits in */
    /* CPMatchEnableMask and SCPMatchEnableMask should be set to 0 */
    uint8_t * MAC_DA_44Bit_Const_p;

    /* 48-bit MAC destination address constant, 6 bytes */
    /* Set to NULL if no updated is required */
    /* Set to NULL if no updated is required, corresponding bits in */
    /* CPMatchEnableMask and SCPMatchEnableMask should be set to 0 */
    uint8_t * MAC_DA_48Bit_Const_p;

    /* Match mask used to select which EtherType to compare with the EtherType */
    /* value in the matching rule */
    /* bit i (0-7): 1 enables packet MAC destination address comparison with */
    /*              the respective MAC_DA_ET_Rules[i].EtherType address */
    /* bit j (8-9): 1 enables packet MAC destination address comparison with */
    /*              the respective MAC_DA_ET_Range[j-8].EtherType address */
    uint32_t CPMatchModeMask;  /* Primary Control Packet Detector */
    uint32_t SCPMatchModeMask; /* Secondary Control Packet Detector */

    /* Match mask which can be used to enable/disable control packet */
    /* matching rules */
    /* bit i (0-7): 1 enables packet MAC destination address comparison with */
    /*              the respective MAC_DA_ET_Rules[i].MAC_DA_p address */
    /* bit j (8-15): 1 enables packet Ethernet Type value comparison with */
    /*               the respective MAC_DA_ET_Rules[j-8].EtherType value */
    /* bit 16: 1 enables range of packet MAC destination address as well as */
    /*         Ethernet Type value comparison with the respective */
    /*         MAC_DA_ET_Range[0].Range.MAC_DA_Start_p and */
    /*         MAC_DA_ET_Range[0].Range.MAC_DA_End_p values as well as */
    /*         MAC_DA_ET_Range[0].EtherType value */
    /* bit 17: 1 enables range of packet MAC destination address as well as */
    /*         Ethernet Type value comparison with the respective */
    /*         MAC_DA_ET_Range[1].Range.MAC_DA_Start_p and */
    /*         MAC_DA_ET_Range[1].Range.MAC_DA_End_p values as well as */
    /*         MAC_DA_ET_Range[1].EtherType value */
    /* bit 18: 1 enables range of packet MAC destination addresses comparison */
    /*         with MAC_DA_Range.MAC_DA_Start_p and MAC_DA_Range.MAC_DA_End_p */
    /* bit 19: 1 enables packet MAC destination addresses comparison with */
    /*         MAC_DA_44Bit_Const_p constant */
    /* bit 20: 1 enables packet MAC destination addresses comparison with */
    /*         MAC_DA_48Bit_Const_p constant */
    /* bit 31: 1 enables MACsec KaY packets as control packets */
    uint32_t CPMatchEnableMask;  /* Primary Control Packet Detector */
    uint32_t SCPMatchEnableMask; /* Secondary Control Packet Detector */
} EIP163_ControlPacket_t;

/* vPort policy data structure */
typedef struct
{
    /* SecTag offset */
    /* Width and function of this field depends on direction: */
    /* - Egress, 6-bit wide: SecTAG location relative to the start of the frame. */
    /* - Ingress: not applicable. */
    uint8_t SecTagOffset;

    /* Packet expansion indication */
    /* 00b - 0 bytes (no expansion) */
    /* 01b - Reserved */
    /* 10b - 24 bytes */
    /* 11b - 32 bytes */
    /* If the packet is classified as a 'control' packet, this field is ignored */
    /* and no expansion is indicated on the output interface. */

    /* Note: For egress only, not used for ingress configurations. */
    uint8_t PktExtension;
} EIP163_vPortPolicy_t;

/* Packet types */
typedef enum
{
    EIP163_RULE_PKT_TYPE_OTHER  = 0, /* untagged, VLAN, etc */
    EIP163_RULE_PKT_TYPE_MPLS   = 1,
    EIP163_RULE_PKT_TYPE_PBB    = 2,
    EIP163_RULE_PKT_TYPE_MACSEC = 3  /* ingress direction only */
} EIP163_Rule_PacketType_t;

/* vPort matching rule Key/Mask data structure */
typedef struct
{
    EIP163_Rule_PacketType_t PacketType;

    /* If PacketType = EIP163_RULE_PKT_TYPE_MACSEC then */
    /*     Bits [5:0] = SecTAG TCI (if supported by configuration) */

    /* If PacketType = EIP163_RULE_PKT_TYPE_MPLS or */
    /*    PacketType = EIP163_RULE_PKT_TYPE_OTHER then */
    /*     Bit 0 = 1 : No VLAN tags and no MPLS labels */
    /*     Bit 1 = 1 : 1 VLAN tag or 1 MPLS label */
    /*     Bit 2 = 1 : 2 VLAN tags or 2 MPLS labels */
    /*     Bit 3 = 1 : 3 VLAN tags or 3 MPLS labels */
    /*     Bit 4 = 1 : 4 VLAN tags or 4 MPLS labels */
    /*     Bit 5 = 1 : 5 MPLS labels */
    /*     Bit 6 = 1 : >4 VLAN tags or >5 MPLS labels */
    uint8_t NumTags; /* bit mask, only 7 bits [6:0] are used, see above how */

    uint16_t ChannelID;
} EIP163_Rule_KeyMask_t;

/* vPort matching rule policy */
typedef struct
{
    /* vPort number that will be assigned to the packet */
    uint16_t vPortId;

    /* Priority value that is used to resolve multiple rule matches. */
    /* When multiple rules are hit by a packet simultaneously, the rule with */
    /* the higher priority value will be returned. If multiple rules with */
    /* an identical priority value are hit, the rule with the lowest rule index */
    /* is used. */
    uint8_t Priority;

    /* true : drop the packet */
    bool fDrop;

    /* true : process the packet as controlled */
    bool fControlPacket;
} EIP163_Rule_Policy_t;

/* vPort matching rule data structure */
typedef struct
{
    /* Sets matching values as specified in EIP163_Rule_KeyMask_t */
    EIP163_Rule_KeyMask_t Key;

    /* Mask for matching values, can be used to mask out irrelevant Key bits */
    EIP163_Rule_KeyMask_t Mask;

    /* Data[0] : MAC Destination Address least significant bytes (3..0) */
    /* Data[1] : MAC Destination Address most significant bytes (5, 4) */
    /* Data[2] : Packet data (EtherType, VLAN tag, MPLS label) */
    /* Data[3] : Packet data (EtherType, VLAN tag, MPLS label) */
    /* See TCAM packet data fields description in the EIP-163 Programmer Manual */
    uint32_t Data[EIP163_RULE_NON_CTRL_WORD_COUNT];

    /* Mask for data values, can be used to mask out irrelevant Data bits */
    uint32_t DataMask[EIP163_RULE_NON_CTRL_WORD_COUNT];

    /* Rule policy */
    EIP163_Rule_Policy_t Policy;
} EIP163_Rule_t;

typedef struct
{
    uint32_t low;
    uint32_t hi;
} EIP163_UI64_t;

typedef struct
{
    /* When true the statistics counters are automatically reset */
    /* on a read operation */
    bool fAutoStatCntrsReset;

    /* Threshold for the packet counters */
    EIP163_UI64_t CountFrameThr;

    /* Threshold for the per-channel packet counters */
    EIP163_UI64_t ChanCountFrameThr;

    /* Counter increment enable control */
    uint8_t CountIncDisCtrl;
} EIP163_Statistics_Control_t;

typedef struct
{
    /* TCAM hit counter */
    EIP163_UI64_t   Counter;
} EIP163_Statistics_TCAM_t;


typedef struct
{
    EIP163_UI64_t TCAMHitMultiple;
    EIP163_UI64_t HeaderParserDroppedPkts;
    EIP163_UI64_t TCAMMiss;
    EIP163_UI64_t PktsCtrl;
    EIP163_UI64_t PktsData;
    EIP163_UI64_t PktsDropped;
    EIP163_UI64_t PktsErrIn;
} EIP163_Statistics_Channel_t;



/* Number of ECC status counters. */
#define EIP163_ECC_NOF_STATUS_COUNTERS 11

/* Debug registers for packet classification. */
typedef struct
{
    uint32_t CPMatchDebug;
    uint32_t TCAMDebug;
    uint32_t SecTAGDebug;
    uint32_t SAMPPDebug1;
    uint32_t SAMPPDebug2;
    uint32_t SAMPPDebug3;
    uint32_t ParsedDALo;
    uint32_t ParsedDAHi;
    uint32_t ParsedSALo;
    uint32_t ParsedSAHi;
    uint32_t ParsedSecTAGLo;
    uint32_t ParsedSecTAGHi;
    uint32_t DebugFlowLookup;
} EIP163_Device_PktProcessDebug_t;


/* ECC Error status */
typedef struct
{
    unsigned int CorrectableCount;
    unsigned int UncorrectableCount;
    bool fCorrectableThr;        /* Threshold for correctable errors exceeded */
    bool fUncorrectableThr;      /* Threshold for uncorrectable errors exceeded */
} EIP163_Device_ECCStatus_t;


/*----------------------------------------------------------------------------
 * EIP163_HWRevision_Get
 *
 * Retrieves EIP-163 device capabilities and version information.
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
 * This function can be called concurrently with any other EIP163_*() API
 * function for the same or different Device.
 *
 * Return value:
 *      EIP163_NO_ERROR : Operation is completed.
 *      EIP163_UNSUPPORTED_FEATURE_ERROR : Not supported by the device.
 *      EIP163_ARGUMENT_ERROR : Passed wrong argument.
 */
EIP163_Error_t
EIP163_HWRevision_Get(
        const Device_Handle_t Device,
        EIP163_Capabilities_t * const Capabilities_p);


/*----------------------------------------------------------------------------
 * EIP163_Device_Init
 *
 * This function starts the initialization of an EIP-163 device.
 *
 * This function returns the EIP163_HW_CONFIGURATION_MISMATCH error code
 * when it detects a mismatch in the EIP-163 Driver Library configuration.
 *
 * Note: This function should be called after the EIP-163 HW Reset.
 *
 * API use order:
 *      This function must be executed before any of the EIP163_*() functions
 *      can be called except the EIP164_HWRevision_Get() function.
 *      If this function returns EIP163_BUSY_RETRY_LATER then
 *      the EIP163_Device_Is_Init_Done() should be called afterwards until it
 *      returns EIP163_NO_ERROR. Only then the device initialization can be
 *      considered completed and the other API functions may be called for
 *      this device.
 *
 * IOArea_p (input/output)
 *      Pointer to the Device instance I/O area in memory that is used to
 *      identify the EIP-163 device.
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
 *      EIP163_NO_ERROR : Operation is completed.
 *      EIP163_UNSUPPORTED_FEATURE_ERROR : Not supported by the device.
 *      EIP163_ARGUMENT_ERROR : Passed wrong argument.
 *      EIP163_BUSY_RETRY_LATER : Device initialization is not completed yet.
 *      EIP163_HW_CONFIGURATION_MISMATCH : HW configuration does not support
 *                                         requested operation.
 */
EIP163_Error_t
EIP163_Device_Init(
        EIP163_IOArea_t * const IOArea_p,
        const Device_Handle_t Device,
        const EIP163_Settings_t * const Settings_p);


/*----------------------------------------------------------------------------
 * EIP163_Device_Is_Done
 *
 * This function completes the EIP-163 device operation.
 *
 * API use order:
 *      This function should be called after the EIP163_Device_Init() function
 *      returned EIP163_BUSY_RETRY_LATER. This function should be called
 *      repeatedly until it returns EIP163_NO_ERROR. Only then the device
 *      initialization can be considered completed and other API functions may
 *      be called for this device.
 *
 * IOArea_p (input)
 *      Pointer to the Device instance I/O area in memory that identifies the
 *      EIP-163 device to be used.
 *
 * This function is NOT re-entrant for the same Device.
 * This function is re-entrant for different Devices.
 *
 * Return value
 *      EIP163_NO_ERROR : Operation is completed.
 *      EIP163_ARGUMENT_ERROR : Passed wrong argument.
 *      EIP163_BUSY_RETRY_LATER : Device initialization is not completed yet.
 */
EIP163_Error_t
EIP163_Device_Is_Done(
        const EIP163_IOArea_t * const IOArea_p);


/*----------------------------------------------------------------------------
 * EIP163_Device_Uninit
 *
 * This function starts the uninitialization of an EIP-163 device.
 *
 * API use order:
 *      After this function is executed none of the EIP163_*() functions
 *      can be called for this device. If this function returns
 *      EIP163_BUSY_RETRY_LATER then the EIP163_Device_Is_Done() should
 *      be called afterwards until it returns EIP163_NO_ERROR. Only then
 *      the device uninitialization can be considered completed and
 *      the other API functions may be called for this device.
 *
 * IOArea_p (input/output)
 *      Pointer to the Device instance I/O area in memory that is used to
 *      identify the EIP-163 device.
 *
 * This function is NOT re-entrant for the same Device.
 * This function is re-entrant for different Devices.
 *
 * Return value
 *      EIP163_NO_ERROR : Operation is completed.
 *      EIP163_ARGUMENT_ERROR : Passed wrong argument.
 *      EIP163_BUSY_RETRY_LATER : Device uninitialization is not completed yet.
 */
EIP163_Error_t
EIP163_Device_Uninit(
        EIP163_IOArea_t * const IOArea_p);


/*----------------------------------------------------------------------------
 * EIP163_Device_Sync
 *
 * This function starts the host synchronization with an EIP-163 device.
 * When this function is called it makes the EIP-163 device to remember
 * the number of packets currently available in its pipeline.
 * The synchronization is considered completed when all these remembered
 * packets are processed. This does not mean, however, that the processing
 * of other packets does not start.
 *
 * API use order:
 *      This function should be called before the EIP-163 statistics can be read
 *      by the host. If this function returns
 *      EIP163_BUSY_RETRY_LATER then the EIP163_SecY_Is_Sync_Done() should
 *      be called afterwards until it returns EIP163_NO_ERROR. Only then the
 *      device synchronization with the host can be considered completed and
 *      other API functions may be called for this device.
 *
 * IOArea_p (input/output)
 *      Pointer to the Device instance I/O area in memory that identifies the
 *      EIP-163 device to be used.
 *
 * This function is NOT re-entrant for the same Device.
 * This function is re-entrant for different Devices.
 *
 * This function cannot be called concurrently with the function
 * EIP163_Device_Is_Sync_Done() for the same Device.
 * This function can be called concurrently with the function
 * EIP163_Device_Is_Sync_Done() for the different Devices or other
 * functions except EIP163_Device_Is_Sync_Done() for the same Device.
 *
 * Return value
 *      EIP163_NO_ERROR : Operation is completed.
 *      EIP163_ARGUMENT_ERROR : Passed wrong argument.
 *      EIP163_BUSY_RETRY_LATER : Device synchronization is not completed yet.
 */
EIP163_Error_t
EIP163_Device_Sync(
        EIP163_IOArea_t * const IOArea_p);


/*----------------------------------------------------------------------------
 * EIP163_Device_Is_Sync_Done
 *
 * This function completes the host synchronization with an EIP-163 device.
 *
 * API use order:
 *      This function should be called after the EIP163_Device_Sync()
 *      function returned EIP163_BUSY_RETRY_LATER. It should be called
 *      repeatedly until it returns EIP163_NO_ERROR. Only then the device
 *      synchronization with can be considered completed and the statistics
 *      can be read by the host.
 *
 * IOArea_p (input/output)
 *      Pointer to the Device instance I/O area in memory that identifies the
 *      EIP-163 device to be used.
 *
 * This function is NOT re-entrant for the same Device.
 * This function is re-entrant for different Devices.
 *
 * This function cannot be called concurrently with the function
 * EIP163_Device_Sync() for the same Device.
 * This function can be called concurrently with the EIP163_Device_Sync()
 * function for different Devices or other functions except
 * EIP163_Device_Sync() for the same Device.
 *
 * Return value
 *     EIP163_NO_ERROR : Operation is completed.
 *     EIP163_ARGUMENT_ERROR : Passed wrong argument.
 *     EIP163_BUSY_RETRY_LATER : Device synchronization is not completed yet.
 */
EIP163_Error_t
EIP163_Device_Is_Sync_Done(
        EIP163_IOArea_t * const IOArea_p);


/*----------------------------------------------------------------------------
 * EIP163_Device_Clocks_On
 *
 * This function switches specified the clocks of the EIP-163 device on.
 *
 * IOArea_p (input)
 *      Pointer to the Device instance I/O area in memory that identifies the
 *      EIP-163 device to be used.
 *
 * Mask (input)
 *      Mask that specifies the clocks to be switched off.
 *      See EIP163_DEVICE_*_CLOCK for clock signal details.
 *
 * fAlwaysOn (input)
 *      If true then the specified by the ClockMask clocks will be always
 *      enabled. If false then the clocks will be enabled only when needed,
 *      the device will manage the clocks dynamically.
 *
 * This function is NOT re-entrant for the same Device.
 * This function is re-entrant for different Devices.
 *
 * Return value
 *      EIP163_NO_ERROR : Operation is completed.
 *      EIP163_ARGUMENT_ERROR : Passed wrong argument.
 */
EIP163_Error_t
EIP163_Device_Clocks_On(
        const EIP163_IOArea_t * const IOArea_p,
        const uint32_t ClockMask,
        const bool fAlwaysOn);


/*----------------------------------------------------------------------------
 * EIP163_Device_Clocks_Off
 *
 * This function switches specified the clocks of the EIP-163 device off.
 *
 * IOArea_p (input)
 *      Pointer to the Device instance I/O area in memory that identifies the
 *      EIP-163 device to be used.
 *
 * Mask (input)
 *      Mask that specifies the clocks to be switched off.
 *      See EIP163_DEVICE_*_CLOCK for clock signal details.
 *
 * This function is NOT re-entrant for the same Device.
 * This function is re-entrant for different Devices.
 *
 * Return value
 *      EIP163_NO_ERROR : Operation is completed.
 *      EIP163_ARGUMENT_ERROR : Passed wrong argument.
 */
EIP163_Error_t
EIP163_Device_Clocks_Off(
        const EIP163_IOArea_t * const IOArea_p,
        const uint32_t ClockMask);


/*----------------------------------------------------------------------------
 * EIP163_Device_Update
 *
 * This function updates the EIP-163 device control settings for the specified
 * channel.
 *
 * IOArea_p (input)
 *      Pointer to the Device instance I/O area in memory that identifies the
 *      EIP-163 device to be used.
 *
 * ChannelId (input)
 *      The channel index (number) to write the control setting from.
 *
 * Control_p (input)
 *      Pointer to the memory location where the control parameters
 *      are stored, see EIP163_Device_Control_t.
 *
 * This function is NOT re-entrant for the same channel of the same Device.
 * This function is re-entrant for different Devices or different channels
 * of the same Device.
 *
 * Return value
 *      EIP163_NO_ERROR : Operation is completed.
 *      EIP163_ARGUMENT_ERROR : Passed wrong argument.
 */
EIP163_Error_t
EIP163_Device_Update(
        const EIP163_IOArea_t * const IOArea_p,
        unsigned int ChannelId,
        const EIP163_Device_Control_t * const Control_p);


/*----------------------------------------------------------------------------
 * EIP163_Device_Control_Read
 *
 * This function reads the EIP-163 device control settings for the specified
 * channel.
 *
 * IOArea_p (input)
 *      Pointer to the Device instance I/O area in memory that identifies the
 *      EIP-163 device to be used.
 *
 * ChannelId (input)
 *      The channel index (number) to write the control setting from.
 *
 * Control_p (output)
 *      Pointer to the memory location where the control parameters
 *      are stored, see EIP163_Device_Control_t.
 *
 * This function is re-entrant for the same or different Devices.
 *
 * Return value
 *      EIP163_NO_ERROR : Operation is completed.
 *      EIP163_ARGUMENT_ERROR : Passed wrong argument.
 */
EIP163_Error_t
EIP163_Device_Control_Read(
        const EIP163_IOArea_t * const IOArea_p,
        unsigned int ChannelId,
        EIP163_Device_Control_t * const Control_p);


/*----------------------------------------------------------------------------
 * EIP163_Device_Restore
 *
 * This function restores the IOArea without altering the (hardware) state
 * of the device. Call this function instead of EIP163_Device_Init when
 * the software is restarted, but the device is still in a well-defined state.
 *
 * API use order:
 *      This function must be executed before any of the EIP163_*() functions
 *      can be called after the SW reset except the EIP163_HWRevision_Get()
 *      function.
 *
 * IOArea_p (output)
 *      Pointer to the Device instance I/O area in memory that is used to
 *      identify the EIP-163 device.
 *
 * Device (input)
 *      Handle for the Device instance returned by Device_Find.
 *
 * Mode (input)
 *      Mode of the device (ingress or egress).
 *
 * This function is re-entrant for the same or different Devices.
 *
 * This function can be called concurrently with any other function
 * provided it is not using the same IOArea.
 *
 * Return value
 *      EIP163_NO_ERROR : Operation is completed.
 *      EIP163_UNSUPPORTED_FEATURE_ERROR : Not supported by the device.
 *      EIP163_ARGUMENT_ERROR : Passed wrong argument.
 */
EIP163_Error_t
EIP163_Device_Restore(
        EIP163_IOArea_t * const IOArea_p,
        const Device_Handle_t Device,
        const EIP163_Device_Mode_t Mode);


/*----------------------------------------------------------------------------
 * EIP163_Device_ECC_Configure
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
 * EIP163_Device_ECCStatus_Get() or EIP163_Device_ECC_Configuration_Get()
 * for the same Device.
 * This function can be called concurrently with
 * the EIP163_Device_ECCStatus_Get() function or other functions for different
 * Devices.
 *
 * Return value
 *      EIP163_NO_ERROR : Operation is completed.
 *      EIP163_ARGUMENT_ERROR : Passed wrong argument.
 */
EIP163_Error_t
EIP163_Device_ECC_Configure(
        const EIP163_IOArea_t * const IOArea_p,
        const unsigned int CorrectableThr,
        const unsigned int UncorrectableThr);


/*----------------------------------------------------------------------------
 * EIP163_Device_ECC_Configuration_Get
 *
 * Read the configuration ECC error checking.
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
 *
 * This function cannot be called concurrently with the function
 * EIP163_Device_ECC_Configure() for the same Device.
 * This function can be called concurrently with
 * the EIP163_Device_ECCStatus_Get() function or other functions for different
 * Devices.
 *
 * Return value
 *      EIP163_NO_ERROR : Operation is completed.
 *      EIP163_ARGUMENT_ERROR : Passed wrong argument.
 */
EIP163_Error_t
EIP163_Device_ECC_Configuration_Get(
        const EIP163_IOArea_t * const IOArea_p,
        unsigned int * const CorrectableThr_p,
        unsigned int * const UncorrectableThr_p);


/*----------------------------------------------------------------------------
 * EIP163_Device_EOP_Configure
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
 * EIP163_Device_EOP_Configuration_Get() or the same device.
 * This function can be called concurrently with any other function for
 * the same or different Devices.
 *
 * Return value
 *      EIP163_NO_ERROR : Operation is completed.
 *      EIP163_ARGUMENT_ERROR : Passed wrong argument.
 */
EIP163_Error_t
EIP163_Device_EOP_Configure(
        const EIP163_IOArea_t * const IOArea_p,
        const unsigned int EOPTimeoutVal_p,
        const EIP163_Ch_Mask_t * const EOPTimeoutCtrl_p);


/*----------------------------------------------------------------------------
 * EIP163_Device_EOP_Configuration_Get
 *
 * Read the configuration for automatic EOP configuration
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
 * EIP163_Device_EOP_Configure() or the same device.
 * This function can be called concurrently with any other function for
 * the same or different Devices.
 *
 * Return value
 *      EIP163_NO_ERROR : Operation is completed.
 *      EIP163_ARGUMENT_ERROR : Passed wrong argument.
 */
EIP163_Error_t
EIP163_Device_EOP_Configuration_Get(
        const EIP163_IOArea_t * const IOArea_p,
        unsigned int * const EOPTimeoutVal_p,
        EIP163_Ch_Mask_t * const EOPTimeoutCtrl_p);


/*----------------------------------------------------------------------------
 * EIP163_Device_InsertSOP
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
 *      EIP163_NO_ERROR : Operation is completed.
 *      EIP163_ARGUMENT_ERROR : Passed wrong argument.
 */
EIP163_Error_t
EIP163_Device_InsertSOP(
        const EIP163_IOArea_t * const IOArea_p,
        const EIP163_Ch_Mask_t * const ChannelMask_p);


/*----------------------------------------------------------------------------
 * EIP163_Device_InsertEOP
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
 *      EIP163_NO_ERROR : Operation is completed.
 *      EIP163_ARGUMENT_ERROR : Passed wrong argument.
 */
EIP163_Error_t
EIP163_Device_InsertEOP(
        const EIP163_IOArea_t * const IOArea_p,
        const EIP163_Ch_Mask_t * const ChannelMask_p);

/*----------------------------------------------------------------------------
 * EIP163_Device_ECCStatus_Get
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
 *      EIP163_ECC_NOF_STATUS_COUNTERS.
 *
 * This function is re-entrant for the same or different Devices.
 *
 * This function cannot be called concurrently with the function
 * EIP163_Device_ECC_Configure() for the same Device.
 * This function can be called concurrently with
 * the EIP163_Device_ECC_Configure() function or other functions for different
 * Devices.
 *
 * Return value
 *      EIP163_NO_ERROR : Operation is completed.
 *      EIP163_ARGUMENT_ERROR : Passed wrong argument.
 */
EIP163_Error_t
EIP163_Device_ECCStatus_Get(
        const EIP163_IOArea_t * const IOArea_p,
        EIP163_Device_ECCStatus_t * const ECCStatus_p);


/*----------------------------------------------------------------------------
 * EIP163_Device_PktProcessDebug_Get
 *
 * Read the debug registers related to packet classification in the EIP163
 *
 * IOArea_p (input)
 *      Pointer to the Device instance I/O area in memory that identifies the
 *      EIP-163 device to be used.
 *
 * PktProcessDebug_p (output)
 *      Structure containing the contents of the debug registers.
 *
 * This function is re-entrant for the same or different Devices.
 *
 * This function can be called concurrently with any other functions
 * for the same or different Devices.
 *
 * Return value
 *      EIP163_NO_ERROR : Operation is completed.
 *      EIP163_ARGUMENT_ERROR : Passed wrong argument.
 */
EIP163_Error_t
EIP163_Device_PktProcessDebug_Get(
        const EIP163_IOArea_t * const IOArea_p,
        EIP163_Device_PktProcessDebug_t * const PktProcessDebug_p);


/*----------------------------------------------------------------------------
 * EIP163_HeaderParser_Update
 *
 * This function updates the EIP-163 header parser control settings
 * for the specified channel.
 *
 * IOArea_p (input)
 *      Pointer to the Device instance I/O area in memory that identifies the
 *      EIP-163 device to be used.
 *
 * ChannelId (input)
 *      The channel index (number) to write the control settings for.
 *
 * HdrParser_p (input)
 *      Pointer to the memory location where the control settings
 *      are stored, see EIP163_HeaderParser_t.
 *
 * This function is NOT re-entrant for the same Device.
 * This function is re-entrant for different Devices.
 * This function cannot be called concurrently with EIP163_HeaderParser_Read()
 * for the same device.
 * This function can be called concurrently with any other function for
 * the same or different Devices.
 *
 * Return value
 *     EIP163_NO_ERROR : Operation is completed
 *     EIP163_ARGUMENT_ERROR : Passed wrong argument.
 */
EIP163_Error_t
EIP163_HeaderParser_Update(
        const EIP163_IOArea_t * const IOArea_p,
        unsigned int ChannelId,
        const EIP163_HeaderParser_t * const HdrParser_p);


/*----------------------------------------------------------------------------
 * EIP163_HeaderParser_Read
 *
 * This function reads the EIP-163 header parser control settings
 * for the specified channel.
 *
 * IOArea_p (input)
 *      Pointer to the Device instance I/O area in memory that identifies the
 *      EIP-163 device to be used.
 *
 * ChannelId (input)
 *      The channel index (number) to write the control settings for.
 *
 * HdrParser_p (output)
 *      Pointer to the memory location where the control settings
 *      are stored, see EIP163_HeaderParser_t.
 *
 * This function is re-entrant for the same or different Devices.
 * This function cannot be called concurrently with EIP163_HeaderParser_Update()
 * for the same device.
 * This function can be called concurrently with any other function for
 * the same or different Devices.
 *
 * Return value
 *     EIP163_NO_ERROR : Operation is completed
 *     EIP163_ARGUMENT_ERROR : Passed wrong argument.
 */
EIP163_Error_t
EIP163_HeaderParser_Read(
        const EIP163_IOArea_t * const IOArea_p,
        unsigned int ChannelId,
        EIP163_HeaderParser_t * const HdrParser_p);


/*----------------------------------------------------------------------------
 * EIP163_ControlPacket_Update
 *
 * This function updates rules the EIP-163 control packet detector settings
 * for the specified channel.
 *
 * IOArea_p (input)
 *      Pointer to the Device instance I/O area in memory that identifies the
 *      EIP-163 device to be used.
 *
 * ChannelId (input)
 *      The channel index (number) to write the control settings for.
 *
 * Cp_p (input)
 *      Pointer to the memory location where the control settings
 *      are stored, see EIP163_ControlPacket_t.
 *
 * This function is NOT re-entrant for the same Device.
 * This function is re-entrant for different Devices.
 * This function cannot be called concurrently with EIP163_ControlPacket_Read()
 * for the same device.
 * This function can be called concurrently with any other function for
 * the same or different Devices.
 *
 * Return value
 *     EIP163_NO_ERROR : Operation is completed
 *     EIP163_ARGUMENT_ERROR : Passed wrong argument.
 */
EIP163_Error_t
EIP163_ControlPacket_Update(
        const EIP163_IOArea_t * const IOArea_p,
        unsigned int ChannelId,
        const EIP163_ControlPacket_t * const CP_p);


/*----------------------------------------------------------------------------
 * EIP163_ControlPacket_Read
 *
 * This function reads the EIP-163 control packet detector settings
 * for the specified channel.
 *
 * IOArea_p (input)
 *      Pointer to the Device instance I/O area in memory that identifies the
 *      EIP-163 device to be used.
 *
 * ChannelId (input)
 *      The channel index (number) to write the control settings for.
 *
 * Cp_p (output)
 *      Pointer to the memory location where the control settings
 *      are stored, see EIP163_ControlPacket_t.
 *
 * This function is re-entrant for the same or different Devices.
 * This function cannot be called concurrently with
 * EIP163_ControlPacket_Update() for the same device.
 * This function can be called concurrently with any other function for
 * the same or different Devices.
 *
 * Return value
 *     EIP163_NO_ERROR : Operation is completed
 *     EIP163_ARGUMENT_ERROR : Passed wrong argument.
 */
EIP163_Error_t
EIP163_ControlPacket_Read(
        const EIP163_IOArea_t * const IOArea_p,
        unsigned int ChannelId,
        EIP163_ControlPacket_t * const CP_p);


/*----------------------------------------------------------------------------
 * EIP163_StatisticsControl_Update
 *
 * This function updates the EIP-163 device statistics module settings.
 * It can be used to configure the TCAM hit statistics threshold global
 * (stat_tcam_thr) and channel (stat_channel_thr) interrupts.
 *
 * API use order:
 *      If this function returns EIP163_BUSY_RETRY_LATER then
 *      the EIP163_Device_Is_Done() should be called afterwards until it
 *      returns EIP163_NO_ERROR. Only then the statistics control update can be
 *      considered completed and  the other API functions may be called
 *      for this device.
 *
 * IOArea_p (input/output)
 *      Pointer to the Device instance I/O area in memory that is used to
 *      identify the EIP-163 device.
 *
 * StatCtrl_p (input)
 *      Pointer to the EIP-163 device statistics settings in memory.
 *
 * This function is NOT re-entrant for the same Device.
 * This function is re-entrant for different Devices.
 *
 * This function cannot be called concurrently with the function
 * EIP163_Statistics_TCAM_Get() or EIP163_Statistics_Channel_Get()
 * of EIP163_StatisticsControl_Reead() for the same Device.
 * This function can be called concurrently with the function
 * EIP163_Statistics_TCAM_Get() or EIP163_Statistics_Channel_Get()
 * for the different Devices or other functions for the same Device.
 *
 * Return value
 *      EIP163_NO_ERROR : Operation is completed.
 *      EIP163_ARGUMENT_ERROR : Passed wrong argument.
 *      EIP163_BUSY_RETRY_LATER : Update is not completed yet.
 */
EIP163_Error_t
EIP163_StatisticsControl_Update(
        const EIP163_IOArea_t * const IOArea_p,
        const EIP163_Statistics_Control_t * const StatCtrl_p);


/*----------------------------------------------------------------------------
 * EIP163_StatisticsControl_Read
 *
 * This function reads the EIP-163 device statistics module settings.
 *
 * IOArea_p (input/output)
 *      Pointer to the Device instance I/O area in memory that is used to
 *      identify the EIP-163 device.
 *
 * StatCtrl_p (output)
 *      Pointer to the EIP-163 device statistics settings in memory.
 *
 * This function is re-entrant for the same or different Devices.
 *
 * This function cannot be called concurrently with the function
 * EIP163_StatisticsControl_Update() for the same Device.
 * This function can be called concurrently with the function
 * EIP163_Statistics_TCAM_Get() or EIP163_Statistics_Channel_Get()
 * for the different Devices or other functions for the same Device.
 *
 * Return value
 *      EIP163_NO_ERROR : Operation is completed.
 *      EIP163_ARGUMENT_ERROR : Passed wrong argument.
 */
EIP163_Error_t
EIP163_StatisticsControl_Read(
        const EIP163_IOArea_t * const IOArea_p,
        EIP163_Statistics_Control_t * const StatCtrl_p);


/*----------------------------------------------------------------------------
 * EIP163_Statistics_TCAM_Get
 *
 * This function reads the EIP-163 device TCAM statistics.
 *
 * IOArea_p (input/output)
 *      Pointer to the Device instance I/O area in memory that is used to
 *      identify the EIP-163 device.
 *
 * StatIndex (input)
 *      The statistics counter index (number) to read the statistics for.
 *
 * Stat_p (output)
 *      Pointer to the memory location where the EIP-163 device statistics
 *      counter identified by StatIndex will be stored.
 *
 * fClear (input)
 *      When true then the read statistics counter will be cleared.
 *
 * This function is NOT re-entrant for the same Device and the same statistics
 * counter index.
 * This function is re-entrant for different Devices or different statistics
 * counter indices.
 *
 * This function cannot be called concurrently with the function
 * EIP163_StatisticsControl_Update() for the same Device.
 * This function can be called concurrently with the function
 * EIP163_StatisticsControl_Update() for the different Devices or
 * other functions for the same Device.
 *
 * Return value
 *      EIP163_NO_ERROR : Operation is completed.
 *      EIP163_ARGUMENT_ERROR : Passed wrong argument.
 */
EIP163_Error_t
EIP163_Statistics_TCAM_Get(
        const EIP163_IOArea_t * const IOArea_p,
        const unsigned int StatIndex,
        EIP163_Statistics_TCAM_t * const Stat_p,
        const bool fClear);


/*----------------------------------------------------------------------------
 * EIP163_Statistics_Channel_Get
 *
 * This function reads the EIP-163 per-channel device statistics.
 *
 * IOArea_p (input/output)
 *      Pointer to the Device instance I/O area in memory that is used to
 *      identify the EIP-163 device.
 *
 * StatIndex (input)
 *      The statistics counter index (number) to read the statistics for.
 *
 * Stat_p (output)
 *      Pointer to the memory location where the EIP-163 device statistics
 *      counter identified by StatIndex will be stored.
 *
 * fClear (input)
 *      When true then the read statistics counter will be cleared.
 *
 * This function is NOT re-entrant for the same Device and the same statistics
 * counter index.
 * This function is re-entrant for different Devices or different statistics
 * counter indices.
 *
 * This function cannot be called concurrently with the function
 * EIP163_StatisticsControl_Update() for the same Device.
 * This function can be called concurrently with the function
 * EIP163_StatisticsControl_Update() for the different Devices or
 * other functions for the same Device.
 *
 * Return value
 *      EIP163_NO_ERROR : Operation is completed.
 *      EIP163_ARGUMENT_ERROR : Passed wrong argument.
 */
EIP163_Error_t
EIP163_Statistics_Channel_Get(
        const EIP163_IOArea_t * const IOArea_p,
        const unsigned int StatIndex,
        EIP163_Statistics_Channel_t * const Stat_p,
        const bool fClear);


/*----------------------------------------------------------------------------
 * EIP163_Statistics_Summary_TCAM_Read
 *
 * Reads the values of TCAM counter summary registers.
 * Each bit tells if one corresponding TCAM rule has its statistics counter
 * crossed the configured threshold.
 *
 * IOArea_p (input)
 *      Pointer to the Device instance I/O area in memory that identifies the
 *      EIP-163 device to be used.
 *
 * StartOffset (input)
 *      Start offset of the TCAM counter summary register to start reading from.
 *
 * Summary_p (output)
 *      Pointer to a memory location where the content of the TCAM summary
 *      register s will be stored.
 *
 * StartOffset (input)
 *      Start offset of the TCAM counter summary register to start reading from.
 *
 * Count (input)
 *      Number of summary registers to read.
 *
 * This function is re-entrant for the same or different Devices.
 *
 * This function can be called concurrently with any other function provided
 * the API use order is followed.
 *
 * Return value:
 *      EIP163_NO_ERROR : Operation is completed
 *      EIP163_ARGUMENT_ERROR : Passed wrong argument.
 */
EIP163_Error_t
EIP163_Statistics_Summary_TCAM_Read(
        const EIP163_IOArea_t * const IOArea_p,
        const unsigned int StartOffset,
        uint32_t * const Summary_p,
        const unsigned int Count);


/*----------------------------------------------------------------------------
 * EIP163_Statistics_Summary_TCAM_Write
 *
 * Writes the values for TCAM counter summary registers. Each bit clears one
 * corresponding TCAM rule event indicating that this rule has its statistics
 * counter crossed the configured threshold. This function will also clear
 * the stat_tcam_thr interrupt when all the summary bits are cleared.
 *
 * IOArea_p (input/output)
 *      Pointer to the Device instance I/O area in memory that is used to
 *      identify the EIP-163 device.
 *
 * StartOffset (input)
 *      Start offset of the TCAM counter summary register to start writing to.
 *
 * Stat_p (output)
 *      Pointer to the memory location where the EIP-163 device statistics
 *      counter values to be written are stored.
 *
 * Count (input)
 *      Number of summary registers to write.
 *
 * This function is NOT re-entrant for the same Device with overlapping
 * summary registers.
 * This function is re-entrant for different Devices or the same Device with
 * non-overlapping summary registers.
 *
 * Return value
 *      EIP163_NO_ERROR : Operation is completed.
 *      EIP163_ARGUMENT_ERROR : Passed wrong argument.
 */
EIP163_Error_t
EIP163_Statistics_Summary_TCAM_Write(
        const EIP163_IOArea_t * const IOArea_p,
        const unsigned int StartOffset,
        const uint32_t * const Summary_p,
        const unsigned int Count);


/*----------------------------------------------------------------------------
 * EIP163_Statistics_Summary_Channel_Read
 *
 * Reads the per-channel statistics summary.
 *
 * IOArea_p (input)
 *      Pointer to the Device instance I/O area in memory that identifies the
 *      EIP-163 device to be used.
 *
 * ChSummary_p (output)
 *      Pointer to a memory location where the channel statistics summary
 *      will be stored.
 *
 * This function is re-entrant for the same or different Devices.
 *
 * This function can be called concurrently with any other function provided
 * the API use order is followed.
 *
 * Return value:
 *      EIP163_NO_ERROR : Operation is completed
 *      EIP163_ARGUMENT_ERROR : Passed wrong argument.
 */
EIP163_Error_t
EIP163_Statistics_Summary_Channel_Read(
        const EIP163_IOArea_t * const IOArea_p,
        EIP163_Ch_Mask_t * const ChSummary_p);

/*----------------------------------------------------------------------------
 * EIP163_Statistics_Summary_Channel_Write
 *
 * Writes the channel statistics summary. This function will
 * also clear the stat_channel_thr interrupt when all the summary bits are
 * cleared.
 *
 * IOArea_p (input/output)
 *      Pointer to the Device instance I/O area in memory that is used to
 *      identify the EIP-163 device.
 *
 * ChSummary_p (input)
 *      Pointer to the memory location where the channel statistics summary
 *      to be written are stored.
 *
 * This function is NOT re-entrant for the same Device with overlapping
 * summary registers.
 * This function is re-entrant for different Devices or the same Device with
 * non-overlapping summary registers.
 *
 * Return value
 *      EIP163_NO_ERROR : Operation is completed.
 *      EIP163_ARGUMENT_ERROR : Passed wrong argument.
 */
EIP163_Error_t
EIP163_Statistics_Summary_Channel_Write(
        const EIP163_IOArea_t * const IOArea_p,
        const EIP163_Ch_Mask_t * const ChSummary_p);


/*----------------------------------------------------------------------------
 * EIP163_vPortPolicy_Update
 *
 * Updates a vPort policy (or adds a new one) for one EIP-163 device instance
 * identified by IOArea_p parameter.
 *
 * API use order:
 *      A vPort policy can be updated for a EIP-163 device instance only after
 *      this device has been initialized via the EIP163_Device_Init() function.
 *
 * IOArea_p (input)
 *      Handle for the Device instance I/O area identifying of the EIP-163
 *      device to be used for packet classification.
 *
 * PolicyIndex (input)
 *      Policy index to be updated must be less than the maximum number of
 *      policies supported by the device. The user must keep track of free and
 *      used vPort policies.
 *
 * vPortPolicy_p (input)
 *      Pointer to a memory location where the data for the vPort policy
 *      is stored. This data will be used for the update operation.
 *
 * This function is re-entrant for the same or different Devices for the same
 * or different policies.
 *
 * This function can be called concurrently with any other function provided
 * the API use order is followed.
 *
 * Return value:
 *     EIP163_NO_ERROR : Operation is completed
 *     EIP163_ARGUMENT_ERROR : Passed wrong argument.
 */
EIP163_Error_t
EIP163_vPortPolicy_Update(
        const EIP163_IOArea_t * const IOArea_p,
        const unsigned int PolicyIndex,
        const EIP163_vPortPolicy_t * const vPortPolicy_p);


/*----------------------------------------------------------------------------
 * EIP163_vPortPolicy_Read
 *
 * Read a vPort policy for one EIP-163 device instance identified by
 * IOArea_p parameter.
 *
 * API use order:
 *      A vPort policy can be read for a EIP-163 device instance only after
 *      this device has been initialized via the EIP163_Device_Init() function.
 *
 * IOArea_p (input)
 *      Handle for the Device instance I/O area identifying of the EIP-163
 *      device to be used for packet classification.
 *
 * PolicyIndex (input)
 *      Policy index to be read must be less than the maximum number of
 *      policies supported by the device.
 *
 * vPortPolicy_p (input)
 *      Pointer to a memory location where the data for the vPort policy
 *      is stored.
 *
 * This function is re-entrant for the same or different Devices for the same
 * or different policies.
 *
 * This function can be called concurrently with any other function provided
 * the API use order is followed.
 *
 * Return value:
 *     EIP163_NO_ERROR : Operation is completed
 *     EIP163_ARGUMENT_ERROR : Passed wrong argument.
 */
EIP163_Error_t
EIP163_vPortPolicy_Read(
        const EIP163_IOArea_t * const IOArea_p,
        const unsigned int PolicyIndex,
        EIP163_vPortPolicy_t * const vPortPolicy_p);


/*----------------------------------------------------------------------------
 * EIP163_vPortPolicy_Remove
 *
 * Removes an already existing vPort policy from one EIP-163 device instance
 * identified by IOArea_p parameter.
 *
 * IOArea_p (input)
 *      Handle for the Device instance I/O area identifying of the EIP-163
 *      device to be used for packet classification.
 *
 * PolicyIndex (input)
 *      Policy index to be removed must be less than the maximum number of
 *      policies supported by the device. The user must keep track of free and
 *      used vPort policies.
 *
 * This function is re-entrant for the same or different Devices for the same
 * or different policies.
 *
 * This function can be called concurrently with any other function provided
 * the API use order is followed.
 *
 * Return value:
 *     EIP163_NO_ERROR : Operation is completed
 *     EIP163_ARGUMENT_ERROR : Passed wrong argument.
 */
EIP163_Error_t
EIP163_vPortPolicy_Remove(
        const EIP163_IOArea_t * const IOArea_p,
        const unsigned int PolicyIndex);


/*----------------------------------------------------------------------------
 * EIP163_Rule_Update
 *
 * Updates a vPort matching rule (or adds a new one) for one EIP-163 device
 * instance identified by IOArea_p parameter.
 *
 * API use order:
 *      A rule can be updated for a EIP-163 device instance only after
 *      the matching vPort policy has been added to this device instance
 *      via the EIP163_vPortPolicy_Update() function.
 *
 * IOArea_p (input)
 *      Handle for the Device instance I/O area identifying of the EIP-163
 *      device to be used for packet classification.
 *
 * RuleIndex (input)
 *      Rule index to be updated must be less than the maximum number of
 *      rules supported by the device. The user must keep track of free and
 *      used rules.
 *
 * Rule_p (input)
 *      Pointer to a memory location where the data for the Rule
 *      is stored. This data will be used for the update operation.
 *
 * This function is NOT re-entrant for the same rule of the same Device.
 * This function is re-entrant for different Devices or different rules
 * of the same Device.
 *
 * This function cannot be called concurrently with the function
 * EIP163_Rule_Read() or EIP163_Rule_Remove() for the same Device.  On
 * hardware with an external TCAM it also cannot be called
 * concurrently with EIP163_Rule_Disable() or EIP163_Rule_EnableDisable().
 *
 * This function can be called concurrently with
 * the EIP163_Rule_Read() or EIP163_Rule_Remove() function or other functions
 * for different Devices.
 *
 * Return value:
 *     EIP163_NO_ERROR : Operation is completed
 *     EIP163_ARGUMENT_ERROR : Passed wrong argument.
 *     EIP163_BUSY_RETRY_LATER : device is busy.
 */
EIP163_Error_t
EIP163_Rule_Update(
        const EIP163_IOArea_t * const IOArea_p,
        const unsigned int RuleIndex,
        const EIP163_Rule_t * const Rule_p);


/*----------------------------------------------------------------------------
 * EIP163_Rule_Read
 *
 * Read a vPort matching rule for one EIP-163 device instance
 * identified by IOArea_p parameter.
 *
 * API use order:
 *      A rule can be read for a EIP-163 device instance only after
 *      the matching vPort policy has been added to this device instance
 *      via the EIP163_vPortPolicy_Update() function.
 *
 * IOArea_p (input)
 *      Handle for the Device instance I/O area identifying of the EIP-163
 *      device to be used for packet classification.
 *
 * RuleIndex (input)
 *      Rule index to be read must be less than the maximum number of
 *      rules supported by the device.
 *
 * Rule_p (output)
 *      Pointer to a memory location where the data for the Rule
 *      is stored.
 *
 * fEnabled_p (output)
 *      Pointer to a memory location where to store the flag that indicates
 *      whether the rules is enabled.
 *
 * On hardware with an internal TCAM this function is re-entrant.
 * On hardware with an external TCAM this function is not re-entrant.
 *
 * This function cannot be called concurrently with the function
 * EIP163_Rule_Update() or EIP163_Rule_Remove() for the same Device.
 * On hardware with an external TCAM this function also cannot be called
 * concurrently with EIP163_Rule_Disable() or EIP163_Rule_EnableDisable().
 *
 * This function can be called concurrently with
 * the EIP163_Rule_Update() or EIP163_Rule_Remove() function or other functions
 * for different Devices.
 *
 * Return value:
 *     EIP163_NO_ERROR : Operation is completed
 *     EIP163_ARGUMENT_ERROR : Passed wrong argument.
 *     EIP163_BUSY_RETRY_LATER : device is busy.
 */
EIP163_Error_t
EIP163_Rule_Read(
        EIP163_IOArea_t * const IOArea_p,
        const unsigned int RuleIndex,
        EIP163_Rule_t * const Rule_p,
        bool * const fEnabled_p);


/*----------------------------------------------------------------------------
 * EIP163_Rule_Remove
 *
 * Removes a vPort matching rule for one EIP-163 device instance identified
 * by IOArea_p parameter.
 *
 * API use order:
 *      A rule can be removed from a EIP-163 device instance only after
 *      the matching rule policy has been added (updated) to this device
 *      instance via the EIP163_Rule_Update() function.
 *
 * IOArea_p (input)
 *      Handle for the Device instance I/O area identifying of the EIP-163
 *      device to be used for packet classification.
 *
 * RuleIndex (input)
 *      Rule index to be removed must be less than the maximum number of
 *      rules supported by the device.
 *
 * This function is re-entrant.
 *
 * This function cannot be called concurrently with the function
 * EIP163_Rule_Update() or EIP163_Rule_Read() for the same Device.
 * This function can be called concurrently with
 * the EIP163_Rule_Update() or EIP163_Rule_Read() function or other functions
 * for different Devices.
 *
 * This function is a no-op on hardware with an external TCAM.
 *
 * Return value:
 *     EIP163_NO_ERROR : Operation is completed
 *     EIP163_ARGUMENT_ERROR : Passed wrong argument.
 */
EIP163_Error_t
EIP163_Rule_Remove(
        const EIP163_IOArea_t * const IOArea_p,
        const unsigned int RuleIndex);


/*----------------------------------------------------------------------------
 * EIP163_Rule_Enable
 *
 * Enables a rule for one EIP-163 device instance identified by
 * IOArea_p parameter.
 *
 * API use order:
 *      A rule can be enabled for an EIP-163 device instance only after
 *      the rule has been added to this device instance
 *      via the EIP163_Rule_Add() function.
 *
 * IOArea_p (input)
 *      Handle for the Device instance I/O area identifying of the EIP-163
 *      device to be used for packet classification.
 *
 * RuleIndex (input)
 *      Rule index to be enabled must be less than the maximum number of
 *      rules supported by the device.
 *
 * This function is re-entrant.
 *
 * This function can be called concurrently with any other function provided
 * the API use order is followed.
 *
 * This function is a no-op on hardware with an external TCAM.
 *
 * Return value:
 *     EIP163_NO_ERROR : Operation is completed
 *     EIP163_ARGUMENT_ERROR : Passed wrong argument.
 */
EIP163_Error_t
EIP163_Rule_Enable(
        const EIP163_IOArea_t * const IOArea_p,
        const unsigned int RuleIndex);


/*----------------------------------------------------------------------------
 * EIP163_Rule_Disable
 *
 * Disables a rule for one EIP-163 device instance identified by
 * IOArea_p parameter.
 *
 * API use order:
 *      A rule can be disabled for an EIP-163 device instance only after
 *      the rule has been added to this device instance
 *      via the EIP163_Rule_Add() function.
 *
 * IOArea_p (input)
 *      Handle for the Device instance I/O area identifying of the EIP-163
 *      device to be used for packet classification.
 *
 * RuleIndex (input)
 *      Rule index to be disabled must be less than the maximum number of
 *      rules supported by the device.
 *
 * On hardware with an internal TCAM this function is re-entrant
 * This function can be called concurrently with any other function provided
 * the API use order is followed.
 *
 * On hardware with an external TCAM this function is not re-entrant
 * This function cannot be called concurrently with EIP164_Rule_Update() or
 * EIP163_Rule_Read() or EIP163_Rule_EnableDisable() on the same device.
 *
 * Return value:
 *     EIP163_NO_ERROR : Operation is completed
 *     EIP163_ARGUMENT_ERROR : Passed wrong argument.
 *     EIP163_BUSY_RETRY_LATER : device is busy.
 */
EIP163_Error_t
EIP163_Rule_Disable(
        const EIP163_IOArea_t * const IOArea_p,
        const unsigned int RuleIndex);


/*----------------------------------------------------------------------------
 * EIP163_Rule_EnableDisable
 *
 * Enables and/or disables a rule for one EIP-163 device instance identified by
 * IOArea_p parameter.
 *
 * API use order:
 *      A rule can be enabled/disabled for an EIP-163 device instance only after
 *      the rule has been added to this device instance
 *      via the EIP163_Rule_Add() function.
 *
 * IOArea_p (input)
 *      Handle for the Device instance I/O area identifying of the EIP-163
 *      device to be used for packet classification.
 *
 * IndexEnable (input)
 *      Index of the rule to enable.
 *
 * IndexDisable (input)
 *      Index of the rule to disable.
 *
 * EnableSingle (input)
 *      When set to true the rule indicated by IndexEnable will be
 *      enabled.
 *
 * DisableSingle (input)
 *      When set to true the rule indicated by IndexEnable will be
 *      disabled.
 *
 * EnableAll (input)
 *      When set to true all rules will be enabled.
 *      Note: Do not set true if DisableAll is true.
 *
 * DisableAll (input)
 *      When set to true all rules will be disabled.
 *      Note: Do not set true if EnableAll is true.
 *
 * On hardware with an internal TCAM this function is re-entrant
 * This function can be called concurrently with any other function provided
 * the API use order is followed.
 *
 * On hardware with an external TCAM this function is not re-entrant
 * This function cannot be called concurrently with EIP164_Rule_Update() or
 * EIP163_Rule_Read() and EIP163_Rule_Disable() on the same device.
 *
 * On hardware with an external TCAM this function can only disable, but
 * not enable rules.
 *
 * Return value:
 *     EIP163_NO_ERROR : Operation is completed
 *     EIP163_ARGUMENT_ERROR : Passed wrong argument.
 *     EIP163_BUSY_RETRY_LATER : device is busy.
 */
EIP163_Error_t
EIP163_Rule_EnableDisable(
        const EIP163_IOArea_t * const IOArea_p,
        const unsigned int IndexEnable,
        const unsigned int IndexDisable,
        const bool EnableSingle,
        const bool DisableSingle,
        const bool EnableAll,
        const bool DisableAll);


/*----------------------------------------------------------------------------
 * EIP163_Rule_Policy_Read
 *
 * Read the policy associated with a given rule.
 *
 * IOArea_p (input)
 *      Handle for the Device instance I/O area identifying of the EIP-163
 *      device to be used for packet classification.
 *
 * RuleIndex (input)
 *      Index of the rule.
 *
 * Policy_p (output)
 *      Rule policy as read from the EIP163 device.
 *
 * fEnabled_p (output)
 *      True if the rule is enabled.
 *
 * This function is re-entrant.
 *
 * This function can be called concurrently with any other function provided
 * the API use order is followed.
 *
 * Return value:
 *     EIP163_NO_ERROR : Operation is completed
 *     EIP163_ARGUMENT_ERROR : Passed wrong argument.
 */
EIP163_Error_t
EIP163_Rule_Policy_Read(
        const EIP163_IOArea_t * const IOArea_p,
        const unsigned int RuleIndex,
        EIP163_Rule_Policy_t * const Policy_p,
        bool * const fEnabled_p);


#endif /* EIP163_H_ */


/* end of file eip163.h */
