/* eip163_level0.h
 *
 * EIP-163 Classification Engine Level0 Internal interface
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

#ifndef EIP163_LEVEL0_H_
#define EIP163_LEVEL0_H_


/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

/* Default configuration */
#include <Kit/EIP163/incl/c_eip163.h>

/* Driver Framework Basic Definitions API */
#include <Kit/DriverFramework/incl/basic_defs.h>         /* BIT definitions, bool, uint32_t */

/* Driver Framework Device API */
#include <Kit/DriverFramework/incl/device_types.h>       /* Device_Handle_t */
#include <Kit/DriverFramework/incl/device_rw.h>          /* Read32, Write32 */

/* EIP-163 register interface */
#include <Kit/EIP163/incl/eip163_hw_interface.h>


/*----------------------------------------------------------------------------
 * Definitions and macros
 */

/*----------------------------------------------------------------------------
 * EIP163_Swap16
 *
 * Swaps 2 bytes in a 2-byte word
 */
static inline uint16_t
EIP163_Swap16(
        const uint16_t Value16)
{
    return ((Value16 & MASK_8_BITS) << 8) | ((Value16 >> 8) & MASK_8_BITS);
}


/*----------------------------------------------------------------------------
 * EIP163_Read32
 *
 * This routine reads a 32-bit word from a Register location at Offset
 * in the EIP-163.
 */
static inline int
EIP163_Read32(
        Device_Handle_t Device,
        const unsigned int Offset,
        uint32_t * const Value_p)
{
    return Device_Read32Check(Device, Offset, Value_p);
}


/*----------------------------------------------------------------------------
 * EIP163_Write32
 *
 * This routine writes to a Register location in the EIP-163.
 */
static inline int
EIP163_Write32(
        Device_Handle_t Device,
        const unsigned int Offset,
        const uint32_t Value)
{
   return  Device_Write32(Device, Offset, Value);
}


/*----------------------------------------------------------------------------
 * EIP163_Read32Array
 *
 * This routine reads MemoryDst_p array of Count 32-bit words from a Register
 * location in the EIP-163 at Offset.
 */
static inline int
EIP163_Read32Array(
        Device_Handle_t Device,
        const unsigned int Offset,
        uint32_t * MemoryDst_p,
        const int Count)
{
    return Device_Read32Array(Device, Offset, MemoryDst_p, Count);
}


/*----------------------------------------------------------------------------
 * EIP163_Write32Array
 *
 * This routine writes MemorySrc_p array of Count 32-bit words to a Register
 * location at Offset in the EIP-163.
 */
static inline int
EIP163_Write32Array(
        Device_Handle_t Device,
        const unsigned int Offset,
        const uint32_t * MemorySrc_p,
        const int Count)
{
    return Device_Write32Array(Device, Offset, MemorySrc_p, Count);
}


static inline bool
EIP163_REV_SIGNATURE_MATCH(
        const uint32_t Rev)
{
    return (((uint16_t)Rev) == EIP163_SIGNATURE);
}


static inline int
EIP163_EIP_REV_RD(
        Device_Handle_t Device,
        uint8_t * const EipNumber,
        uint8_t * const ComplmtEipNumber,
        uint8_t * const HWPatchLevel,
        uint8_t * const MinHWRevision,
        uint8_t * const MajHWRevision)
{
    uint32_t RegVal = 0;
    int rc = 0;

    rc = EIP163_Read32(Device, EIP163_REG_VERSION, &RegVal);

    if (!rc)
    {
        *MajHWRevision    = (uint8_t)((RegVal >> 24) & MASK_4_BITS);
        *MinHWRevision    = (uint8_t)((RegVal >> 20) & MASK_4_BITS);
        *HWPatchLevel     = (uint8_t)((RegVal >> 16) & MASK_4_BITS);
        *ComplmtEipNumber = (uint8_t)((RegVal >> 8)  & MASK_8_BITS);
        *EipNumber        = (uint8_t)((RegVal)       & MASK_8_BITS);
    }

    return rc;
}


static inline int
EIP163_CONFIG_RD(
        Device_Handle_t Device,
        uint16_t * const vPortCount_p,
        uint16_t * const RulesCount_p,
        bool * const fIngressOnly,
        bool * const fEgressOnly)
{
    uint32_t RegVal =  0;
    int rc = 0;
    unsigned int ShiftCount;

    rc = EIP163_Read32(Device, EIP163_REG_CONFIG, &RegVal);

    if (!rc)
    {
        ShiftCount = RegVal & MASK_3_BITS;
        *fEgressOnly  = ((RegVal & BIT_31) != 0);
        *fIngressOnly = ((RegVal & BIT_30) != 0);
        *RulesCount_p = (uint16_t)((RegVal >> 20) & MASK_10_BITS);

        if (*RulesCount_p == 0)
            *RulesCount_p = 1024;

        *RulesCount_p <<= ShiftCount;
        *vPortCount_p = (uint16_t)((RegVal >> 10) & MASK_10_BITS);
        *vPortCount_p <<= ShiftCount;
    }

    return rc;
}


static inline int
EIP163_CONFIG2_RD(
        Device_Handle_t Device,
        uint8_t * const Channel_Count_p,
        bool * const fLookupSCI_p,
        bool * const fExternalTCAM_p)
{
    uint32_t RegVal =  0;
    int rc = 0;

    rc = EIP163_Read32(Device, EIP163_REG_CONFIG2, &RegVal);

    if (!rc)
    {
        *fLookupSCI_p    = ((RegVal & BIT_31) != 0);
        *Channel_Count_p = (uint8_t)((RegVal >> 24) & MASK_7_BITS);
        *fExternalTCAM_p = ((RegVal & BIT_22) != 0);
    }

    return rc;
}


static inline int
EIP163_CP_MATCH_DEBUG_RD(
        Device_Handle_t Device,
        uint32_t * const Value_p)
{
    return EIP163_Read32(Device, EIP163_REG_CP_MATCH_DEBUG, Value_p);
}


static inline int
EIP163_TCAM_DEBUG_RD(
        Device_Handle_t Device,
        uint32_t * const Value_p)
{
    return EIP163_Read32(Device, EIP163_REG_TCAM_DEBUG, Value_p);
}


static inline int
EIP163_SECTAG_DEBUG_RD(
        Device_Handle_t Device,
        uint32_t * const Value_p)
{
    return EIP163_Read32(Device, EIP163_REG_SECTAG_DEBUG, Value_p);
}


static inline int
EIP163_SAMPP_DEBUG_RD(
        Device_Handle_t Device,
        uint32_t * const Value1_p,
        uint32_t * const Value2_p,
        uint32_t * const Value3_p)
{
    int rc = 0;

     rc = EIP163_Read32(Device, EIP163_REG_SAMPP_DEBUG1, Value1_p);
     if (rc) return rc;

     rc = EIP163_Read32(Device, EIP163_REG_SAMPP_DEBUG2, Value2_p);
     if (rc) return rc;

     return EIP163_Read32(Device, EIP163_REG_SAMPP_DEBUG2, Value3_p);
}


static inline int
EIP163_DBG_PARSED_DA_RD(
        Device_Handle_t Device,
        uint32_t * const Lo_p,
        uint32_t * const Hi_p)
{
     int rc = 0;

     rc = EIP163_Read32(Device, EIP163_REG_DBG_PARSED_DA_LO, Lo_p);
     if (rc) return rc;

     return EIP163_Read32(Device, EIP163_REG_DBG_PARSED_DA_HI, Hi_p);
}


static inline int
EIP163_DBG_PARSED_SA_RD(
        Device_Handle_t Device,
        uint32_t * const Lo_p,
        uint32_t * const Hi_p)
{
     int rc =0;

     rc = EIP163_Read32(Device, EIP163_REG_DBG_PARSED_SA_LO, Lo_p);
     if (rc) return rc;

     return EIP163_Read32(Device, EIP163_REG_DBG_PARSED_SA_HI, Hi_p);
}


static inline int
EIP163_DBG_PARSED_SECTAG_RD(
        Device_Handle_t Device,
        uint32_t * const Lo_p,
        uint32_t * const Hi_p)
{
     int rc = 0;

     rc = EIP163_Read32(Device, EIP163_REG_DBG_PARSED_SECTAG_LO, Lo_p);
     if (rc) return rc;

     return EIP163_Read32(Device, EIP163_REG_DBG_PARSED_SECTAG_HI, Hi_p);
}


static inline int
EIP163_DEBUG_FLOW_LOOKUP_RD(
        Device_Handle_t Device,
        uint32_t * const Value_p)
{
     return EIP163_Read32(Device, EIP163_REG_DEBUG_FLOW_LOOKUP, Value_p);
}


static inline int
EIP163_TCAM_WR(
        Device_Handle_t Device,
        const unsigned int OffsetInTCAM,
        const uint32_t * Word32_p,
        const uint32_t WordCount,
        const uint32_t MinWordCount)
{
    unsigned int i;
    int rc;

    rc = EIP163_Write32Array(Device,
                             EIP163_REG_TCAM_KEY(0, 0) + /* TCAM start + */
                               OffsetInTCAM,             /* offset in TCAM */
                             Word32_p,
                             WordCount);

    if (rc) return rc;

    if (MinWordCount > WordCount)
    {
        for (i=0; i<MinWordCount - WordCount; i++)
        {
            rc = EIP163_Write32(Device,
                                EIP163_REG_TCAM_KEY(0, 0) + /* TCAM start + */
                                OffsetInTCAM +             /* offset in TCAM */
                                WordCount*4 + i*4,
                                0);
            if (rc) return rc;
        }
    }
    return 0;
}


static inline int
EIP163_TCAM_RD(
        Device_Handle_t Device,
        const unsigned int OffsetInTCAM,
        uint32_t * Word32_p,
        const uint32_t WordCount)
{
    return EIP163_Read32Array(Device,
                              EIP163_REG_TCAM_KEY(0, 0) + /* TCAM start + */
                              OffsetInTCAM,               /* offset in TCAM */
                              Word32_p,
                              WordCount);
}


static inline uint32_t
EIP163_TCAM_CTRL_FR(
        const uint8_t NumTags,
        const uint8_t PacketType,
        const uint16_t ChannelID)
{
    uint32_t RegVal = EIP163_REG_TCAM_CTRL_KEY_DEFAULT;

    RegVal |= ((ChannelID  & MASK_7_BITS) << 16);
    RegVal |= ((PacketType & MASK_2_BITS)  << 8);
    RegVal |= (NumTags     & MASK_7_BITS);

    return RegVal;
}

static inline void
EIP163_TCAM_CTRL_SPLIT(
        const uint32_t RegVal,
        uint8_t * const NumTags,
        uint8_t * const PacketType,
        uint16_t * const ChannelID)
{
    *NumTags    = RegVal & MASK_7_BITS;
    *PacketType = (RegVal >> 8) & MASK_2_BITS;
    *ChannelID  = (RegVal >> 16) & MASK_7_BITS;
}

static inline int
EIP163_TCAM_CTRL_WR(
        Device_Handle_t Device,
        const unsigned int Offset,
        const uint8_t NumTags,
        const uint8_t PacketType,
        const uint16_t ChannelID)
{
    return EIP163_Write32(Device,
                          Offset,
                          EIP163_TCAM_CTRL_FR(NumTags,
                                              PacketType,
                                              ChannelID));
}

static inline int
EIP163_TCAM_CTRL_RD(
        Device_Handle_t Device,
        const unsigned int Offset,
        uint8_t * const NumTags,
        uint8_t * const PacketType,
        uint16_t * const ChannelID)
{
    uint32_t RegVal =  0;
    int rc = 0;

    rc = EIP163_Read32(Device, Offset, &RegVal);

    if (!rc)
    {
        EIP163_TCAM_CTRL_SPLIT(RegVal,
                               NumTags,
                               PacketType,
                               ChannelID);
    }

    return rc;
}


static inline int
EIP163_TCAM_CTRL_KEY_DEFAULT_WR(
        Device_Handle_t Device,
        const unsigned int TCAMEntryID)
{
    return EIP163_Write32(Device,
                          EIP163_REG_TCAM_CTRL_KEY(TCAMEntryID),
                          EIP163_REG_TCAM_CTRL_KEY_DEFAULT);
}


static inline int
EIP163_TCAM_CTRL_KEY_WR(
        Device_Handle_t Device,
        const unsigned int TCAMEntryID,
        const uint8_t NumTags,
        const uint8_t PacketType,
        const uint16_t ChannelID)
{
    return EIP163_TCAM_CTRL_WR(Device,
                               EIP163_REG_TCAM_CTRL_KEY(TCAMEntryID),
                               NumTags,
                               PacketType,
                               ChannelID);
}


static inline int
EIP163_TCAM_CTRL_KEY_RD(
        Device_Handle_t Device,
        const unsigned int TCAMEntryID,
        uint8_t * const NumTags,
        uint8_t * const PacketType,
        uint16_t * const ChannelID)
{
    return EIP163_TCAM_CTRL_RD(Device,
                               EIP163_REG_TCAM_CTRL_KEY(TCAMEntryID),
                               NumTags,
                               PacketType,
                               ChannelID);
}


static inline int
EIP163_TCAM_CTRL_MASK_DEFAULT_WR(
        Device_Handle_t Device,
        const unsigned int TCAMEntryID)
{
    return EIP163_Write32(Device,
                          EIP163_REG_TCAM_CTRL_MASK(TCAMEntryID),
                          EIP163_REG_TCAM_CTRL_MASK_DEFAULT);
}


static inline int
EIP163_TCAM_CTRL_MASK_WR(
        Device_Handle_t Device,
        const unsigned int TCAMEntryID,
        const uint8_t NumTagsMask,
        const uint8_t PacketTypeMask,
        const uint16_t ChannelIDMask)
{
    return EIP163_TCAM_CTRL_WR(Device,
                               EIP163_REG_TCAM_CTRL_MASK(TCAMEntryID),
                               NumTagsMask,
                               PacketTypeMask,
                               ChannelIDMask);
}


static inline int
EIP163_TCAM_CTRL_MASK_RD(
        Device_Handle_t Device,
        const unsigned int TCAMEntryID,
        uint8_t * const NumTagsMask,
        uint8_t * const PacketTypeMask,
        uint16_t * const ChannelIDMask)
{
    return EIP163_TCAM_CTRL_RD(Device,
                               EIP163_REG_TCAM_CTRL_MASK(TCAMEntryID),
                               NumTagsMask,
                               PacketTypeMask,
                               ChannelIDMask);
}


static inline int
EIP163_TCAM_STATUS_RD(
        Device_Handle_t Device,
        bool * const fWriteBusy,
        bool * const fReadBusy,
        bool * const fDisableBusy,
        bool * const fEnabled)
{
    uint32_t RegVal = 0;
    int rc = 0;

    rc = EIP163_Read32(Device, EIP163_REG_TCAM_STATUS, &RegVal);

    if (!rc)
    {
        *fWriteBusy   = (RegVal & BIT_0) != 0;
        *fReadBusy    = (RegVal & BIT_1) != 0;
        *fDisableBusy = (RegVal & BIT_2) != 0;
        *fEnabled     = (RegVal & BIT_8) != 0;
    }

    return rc;
}


static inline int
EIP163_TCAM_ENTRY_SET_WR(
        Device_Handle_t Device,
        const uint8_t Index,
        const uint32_t Mask)
{
    return EIP163_Write32(Device, EIP163_REG_TCAM_ENTRY_SET(Index), Mask);
}


static inline int
EIP163_TCAM_ENTRY_CLEAR_WR(
        Device_Handle_t Device,
        const uint8_t Index,
        const uint32_t Mask)
{
    return EIP163_Write32(Device, EIP163_REG_TCAM_ENTRY_CLEAR(Index), Mask);
}


static inline int
EIP163_TCAM_ENTRY_ENABLE_CTRL_WR(
        Device_Handle_t Device,
        const unsigned int IndexSet,
        const bool fSetEnable,
        const bool fEnableAll,
        const unsigned int IndexClear,
        const bool fClearEnable,
        const bool fClearAll)
{
    uint32_t RegVal = EIP163_REG_TCAM_ENTRY_ENABLE_CTRL_DEFAULT;

    if(fSetEnable)
        RegVal |= BIT_14;

    if(fEnableAll)
        RegVal |= BIT_15;

    if(fClearEnable)
        RegVal |= BIT_30;

    if(fClearAll)
        RegVal |= BIT_31;

    RegVal |= ((IndexClear  & MASK_14_BITS) << 16);
    RegVal |= (IndexSet     & MASK_14_BITS);

    return EIP163_Write32(Device, EIP163_REG_TCAM_ENTRY_ENABLE_CTRL, RegVal);
}

static inline int
EIP163_TCAM_ENTRY_ENABLE_RD(
        Device_Handle_t Device,
        const unsigned int RuleIndex,
        bool * const fEnabled_p)
{
    unsigned int RegIndex = RuleIndex / 32;
    uint32_t RegVal;
    uint32_t RegMask = 1 << (RuleIndex % 32);
    int rc = 0;

    rc = EIP163_Read32(Device, EIP163_REG_TCAM_ENTRY_ENABLE(RegIndex), &RegVal);

    if (!rc)
    {
      *fEnabled_p = (RegVal & RegMask) != 0;
    }

    return rc;
}


static inline int
EIP163_SAM_IN_FLIGHT_WR(
        Device_Handle_t Device,
        const bool fLoadUnsafe)
{
    uint32_t RegVal = EIP163_REG_SAM_IN_FLIGHT_DEFAULT;

    if(fLoadUnsafe)
        RegVal |= BIT_31;

    return EIP163_Write32(Device, EIP163_REG_SAM_IN_FLIGHT, RegVal);
}


static inline int
EIP163_SAM_IN_FLIGHT_RD(
        Device_Handle_t Device,
        uint8_t * const Unsafe,
        uint8_t * const InFlight,
        bool * const fLoadUnsafe)
{
    uint32_t RegVal = 0;
    int rc = 0;

    rc = EIP163_Read32(Device, EIP163_REG_SAM_IN_FLIGHT, &RegVal);

    if (!rc)
    {
        *Unsafe      = (uint8_t)((RegVal >> 0) & MASK_6_BITS);
        *InFlight    = (uint8_t)((RegVal >> 8) & MASK_6_BITS);
        *fLoadUnsafe = ((RegVal & BIT_31) != 0);
    }

    return rc;
}


static inline int
EIP163_VPORT_POLICY_DEFAULT_WR(
        Device_Handle_t Device,
        const unsigned int Index)
{
    return EIP163_Write32(Device,
                   EIP163_REG_VPORT_POLICY(Index),
                   EIP163_REG_VPORT_POLICY_DEFAULT);
}


static inline int
EIP163_VPORT_POLICY_WR(
        Device_Handle_t Device,
        const unsigned int Index,
        const uint8_t SecTagOffset,
        const uint8_t PktExtension)
{
    uint32_t RegVal = EIP163_REG_VPORT_POLICY_DEFAULT;

    RegVal |= ((PktExtension & MASK_2_BITS) << 10);
    RegVal |= (SecTagOffset  & MASK_7_BITS);

    return EIP163_Write32(Device, EIP163_REG_VPORT_POLICY(Index), RegVal);
}


static inline int
EIP163_VPORT_POLICY_RD(
        Device_Handle_t Device,
        const unsigned int Index,
        uint8_t * const  SecTagOffset_p,
        uint8_t * const PktExtension_p)
{
    uint32_t RegVal = 0;
    int rc = 0;

    rc = EIP163_Read32(Device, EIP163_REG_VPORT_POLICY(Index), &RegVal);

    if (!rc)
    {
      *PktExtension_p = (RegVal >> 10) & MASK_2_BITS;
      *SecTagOffset_p = RegVal & MASK_7_BITS;
    }

    return rc;
}
static inline int
EIP163_TCAM_POLICY_DEFAULT_WR(
        Device_Handle_t Device,
        const unsigned int Index)
{
    return EIP163_Write32(Device,
                          EIP163_REG_TCAM_POLICY(Index),
                          EIP163_REG_TCAM_POLICY_DEFAULT);
}


static inline int
EIP163_TCAM_POLICY_WR(
        Device_Handle_t Device,
        const unsigned int Index,
        const uint16_t vPortIndex,
        const uint8_t Priority,
        const bool fDrop,
        const bool fControlPkt)
{
    uint32_t RegVal = EIP163_REG_TCAM_POLICY_DEFAULT;

    if(fControlPkt)
        RegVal |= BIT_31;

    if(fDrop)
        RegVal |= BIT_30;

    RegVal |= ((Priority  & MASK_3_BITS)   << 27);
    RegVal |= (vPortIndex & MASK_16_BITS);

    return EIP163_Write32(Device, EIP163_REG_TCAM_POLICY(Index), RegVal);
}


static inline int
EIP163_TCAM_POLICY_RD(
        Device_Handle_t Device,
        const unsigned int Index,
        uint16_t * const vPortIndex_p,
        uint8_t * const  Priority_p,
        bool * fDrop_p,
        bool * fControlPkt_p)
{
    uint32_t RegVal = 0;
    int rc = 0;

    rc = EIP163_Read32(Device, EIP163_REG_TCAM_POLICY(Index), &RegVal);

    if (!rc)
    {
      *fControlPkt_p  = (RegVal & BIT_31) != 0;
      *fDrop_p = (RegVal & BIT_30) != 0;

      *Priority_p = (RegVal >> 27) & MASK_3_BITS;
      *vPortIndex_p = RegVal & MASK_16_BITS;
    }

    return rc;
}

static inline int
EIP163_SAM_NM_PARAMS_WR(
        Device_Handle_t Device,
        const unsigned int ChannelId,
        const bool fCompEtype,
        const bool fCheckVersion,
        const bool fCheckKay,
        const bool fLookupUseSCI,
        const uint16_t MACsecTagValue)
{
    uint32_t RegVal = EIP163_REG_SAM_NM_PARAMS_DEFAULT;

    if(fCompEtype)
        RegVal |= BIT_0;
    else
        RegVal &= ~BIT_0;

    if(fCheckVersion)
        RegVal |= BIT_1;
    else
        RegVal &= ~BIT_1;

    if(fCheckKay)
        RegVal |= BIT_2;
    else
        RegVal &= ~BIT_2;

    if(fLookupUseSCI)
        RegVal |= BIT_15;
    else
        RegVal &= ~BIT_15;

    RegVal |= ((EIP163_Swap16(MACsecTagValue) & MASK_16_BITS) << 16);

    return EIP163_Write32(Device, EIP163_REG_SAM_NM_PARAMS(ChannelId), RegVal);
}


static inline int
EIP163_SAM_NM_PARAMS_RD(
        Device_Handle_t Device,
        const unsigned int ChannelId,
        bool * const fCompEtype,
        bool * const fCheckVersion,
        bool * const fCheckKay,
        bool * const fLookupUseSCI,
        uint16_t * const MACsecTagValue)
{
    uint32_t RegVal = 0;
    int rc = 0;

    rc = EIP163_Read32(Device, EIP163_REG_SAM_NM_PARAMS(ChannelId), &RegVal);

    if (!rc)
    {
        *fCompEtype = (RegVal & BIT_0) != 0;
        *fCheckVersion = (RegVal & BIT_1) != 0;
        *fCheckKay = (RegVal & BIT_2) != 0;
        *fLookupUseSCI = (RegVal & BIT_15) != 0;
        *MACsecTagValue = EIP163_Swap16((RegVal >> 16) & MASK_16_BITS);
    }

    return rc;
}


static inline int
EIP163_CP_MATCH_MODE_WR(
        Device_Handle_t Device,
        const unsigned int ChannelId,
        const uint32_t MatchMode)
{
    uint32_t RegVal = (MatchMode & EIP163_REG_CP_MATCH_MODE_MASK) << 8;

    return EIP163_Write32(Device, EIP163_REG_CP_MATCH_MODE(ChannelId), RegVal);
}


static inline int
EIP163_CP_MATCH_MODE_RD(
        Device_Handle_t Device,
        const unsigned int ChannelId,
        uint32_t * const MatchMode)
{
    uint32_t RegVal = 0;
    int rc = 0;

    rc = EIP163_Read32(Device, EIP163_REG_CP_MATCH_MODE(ChannelId), &RegVal);

    if (!rc)
    {
        *MatchMode = (RegVal >> 8) & EIP163_REG_CP_MATCH_MODE_MASK;
    }

    return rc;
}


static inline int
EIP163_CP_MATCH_ENABLE_WR(
        Device_Handle_t Device,
        const unsigned int ChannelId,
        const uint32_t MatchEnable)
{
    uint32_t RegVal = MatchEnable & EIP163_REG_CP_MATCH_ENABLE_MASK;

    return EIP163_Write32(Device, EIP163_REG_CP_MATCH_ENABLE(ChannelId), RegVal);
}


static inline int
EIP163_CP_MATCH_ENABLE_RD(
        Device_Handle_t Device,
        const unsigned int ChannelId,
        uint32_t * const MatchEnable)
{
    uint32_t RegVal = 0;
    int rc = 0;

    rc = EIP163_Read32(Device, EIP163_REG_CP_MATCH_ENABLE(ChannelId), &RegVal);

    if (!rc)
    {
        *MatchEnable = RegVal & EIP163_REG_CP_MATCH_ENABLE_MASK;
    }

    return rc;
}

static inline int
EIP163_SAM_POLICY_WR(
        Device_Handle_t Device,
        const unsigned int ChannelId,
        const uint8_t DropAction,
        const bool fForceDrop,
        const bool fDefaultVPortValid,
        const uint16_t DefaultVPort)
{
    uint32_t RegVal = EIP163_REG_SAM_POLICY_DEFAULT;

    if(fForceDrop)
        RegVal |= BIT_2;

    if(fDefaultVPortValid)
        RegVal |= BIT_3;

    RegVal |= ((DefaultVPort & MASK_16_BITS) << 16);
    RegVal |= (DropAction    & MASK_2_BITS);

    return EIP163_Write32(Device, EIP163_REG_SAM_POLICY(ChannelId), RegVal);
}


static inline int
EIP163_SAM_POLICY_RD(
        Device_Handle_t Device,
        const unsigned int ChannelId,
        uint8_t * const DropAction,
        bool * const fForceDrop,
        bool *const fDefaultVPortValid,
        uint16_t *const DefaultVPort)
{
    uint32_t RegVal = 0;
    int rc = 0;

    rc = EIP163_Read32(Device, EIP163_REG_SAM_POLICY(ChannelId), &RegVal);

    if (!rc)
    {
        *fForceDrop = (RegVal & BIT_2) != 0;
        *fDefaultVPortValid = (RegVal & BIT_3) != 0;
        *DefaultVPort = (RegVal>>16) & MASK_16_BITS;
        *DropAction = RegVal & MASK_2_BITS;
    }

    return rc;
}

static inline int
EIP163_SAM_PP_TAGS_WR(
        Device_Handle_t Device,
        const uint16_t QTag,
        const uint16_t STag)
{
    uint32_t RegVal = 0; /* EIP163_REG_SAM_PP_TAGS_DEFAULT; */

    RegVal |= ((EIP163_Swap16(STag) & MASK_16_BITS) << 16);
    RegVal |= (EIP163_Swap16(QTag)  & MASK_16_BITS);

    return EIP163_Write32(Device, EIP163_REG_SAM_PP_TAGS, RegVal);
}


static inline int
EIP163_SAM_PP_TAGS_RD(
        Device_Handle_t Device,
        uint16_t * const QTag,
        uint16_t * const STag)
{
    uint32_t RegVal = 0;
    int rc = 0;

    rc = EIP163_Read32(Device, EIP163_REG_SAM_PP_TAGS, &RegVal);

    if (!rc)
    {
        *QTag = EIP163_Swap16(RegVal & MASK_16_BITS);
        *STag = EIP163_Swap16((RegVal >> 16) & MASK_16_BITS);
    }

    return rc;
}


static inline int
EIP163_SAM_PP_TAGS2_WR(
        Device_Handle_t Device,
        const uint16_t STag2,
        const uint16_t STag3)
{
    uint32_t RegVal = 0; /* EIP163_REG_SAM_PP_TAGS2_DEFAULT; */

    RegVal |= ((EIP163_Swap16(STag3) & MASK_16_BITS) << 16);
    RegVal |= (EIP163_Swap16(STag2)  & MASK_16_BITS);

    return EIP163_Write32(Device, EIP163_REG_SAM_PP_TAGS2, RegVal);
}


static inline int
EIP163_SAM_PP_TAGS2_RD(
        Device_Handle_t Device,
        uint16_t * const STag2,
        uint16_t * const STag3)
{
    uint32_t RegVal = 0;
    int rc = 0;

    rc = EIP163_Read32(Device, EIP163_REG_SAM_PP_TAGS2, &RegVal);

    if (!rc)
    {
        *STag2 = EIP163_Swap16(RegVal & MASK_16_BITS);
        *STag3 = EIP163_Swap16((RegVal >> 16) & MASK_16_BITS);
    }

    return rc;
}

static inline int
EIP163_SAM_CP_TAG_ENB_WR(
        Device_Handle_t Device,
        const unsigned int ChannelId,
        const bool fParseQtag,
        const bool fParseStag1,
        const bool fParseStag2,
        const bool fParseStag3,
        const bool fParseQinQ)
{
    uint32_t RegVal = EIP163_REG_SAM_CP_TAG_ENB_DEFAULT;

    if(fParseQinQ)
        RegVal |= BIT_31;

    if(fParseStag3)
        RegVal |= BIT_3;

    if(fParseStag2)
        RegVal |= BIT_2;

    if(fParseStag1)
        RegVal |= BIT_1;

    if(fParseQtag)
        RegVal |= BIT_0;

    return EIP163_Write32(Device, EIP163_REG_SAM_CP_TAG_ENB(ChannelId), RegVal);
}

static inline int
EIP163_SAM_CP_TAG_ENB_RD(
        Device_Handle_t Device,
        const unsigned int ChannelId,
        bool * const fParseQtag,
        bool * const fParseStag1,
        bool * const fParseStag2,
        bool * const fParseStag3,
        bool * const fParseQinQ)
{
    uint32_t RegVal =  0;
    int rc = 0;

    rc = EIP163_Read32(Device, EIP163_REG_SAM_CP_TAG_ENB(ChannelId), &RegVal);

    if (!rc)
    {
        *fParseQinQ  = (RegVal & BIT_31) != 0;
        *fParseStag3 = (RegVal & BIT_3) != 0;
        *fParseStag2 = (RegVal & BIT_2) != 0;
        *fParseStag1 = (RegVal & BIT_1) != 0;
        *fParseQtag  = (RegVal & BIT_0) != 0;
    }

    return rc;
}

static inline int
EIP163_CP_MAC_DA_MATCH_WR(
        Device_Handle_t Device,
        const unsigned int Index,
        const uint8_t DA_Byte1,
        const uint8_t DA_Byte2,
        const uint8_t DA_Byte3,
        const uint8_t DA_Byte4)
{
    uint32_t RegVal = 0; /* EIP163_REG_CP_MAC_DA_MATCH_DEFAULT; */

    RegVal |= ((DA_Byte4   & MASK_8_BITS) << 24);
    RegVal |= ((DA_Byte3   & MASK_8_BITS) << 16);
    RegVal |= ((DA_Byte2   & MASK_8_BITS) << 8);
    RegVal |= (DA_Byte1    & MASK_8_BITS);

    return EIP163_Write32(Device, EIP163_REG_CP_MAC_DA_MATCH(Index), RegVal);
}


static inline int
EIP163_CP_MAC_DA_MATCH_RD(
        Device_Handle_t Device,
        const unsigned int Index,
        uint8_t * const DA_Byte1,
        uint8_t * const DA_Byte2,
        uint8_t * const DA_Byte3,
        uint8_t * const DA_Byte4)
{
    uint32_t RegVal =  0;
    int rc = 0;

    rc = EIP163_Read32(Device, EIP163_REG_CP_MAC_DA_MATCH(Index), &RegVal);

    if (!rc)
    {
        *DA_Byte1 = RegVal & MASK_8_BITS;
        *DA_Byte2 = (RegVal >> 8) & MASK_8_BITS;
        *DA_Byte3 = (RegVal >> 16) & MASK_8_BITS;
        *DA_Byte4 = (RegVal >> 24) & MASK_8_BITS;
    }

    return rc;
}


static inline int
EIP163_CP_MAC_DA_ET_MATCH_WR(
        Device_Handle_t Device,
        const unsigned int Index,
        const uint8_t DA_Byte5,
        const uint8_t DA_Byte6,
        const uint16_t EtherType)
{
    uint32_t RegVal = 0; /* EIP163_REG_CP_MAC_DA_ET_MATCH_DEFAULT; */
    uint32_t Swapped = EIP163_Swap16(EtherType);

    RegVal |= ((Swapped    & MASK_16_BITS) << 16);
    RegVal |= ((DA_Byte6   & MASK_8_BITS)  << 8);
    RegVal |= (DA_Byte5    & MASK_8_BITS);

    return EIP163_Write32(Device, EIP163_REG_CP_MAC_DA_ET_MATCH(Index), RegVal);
}


static inline int
EIP163_CP_MAC_DA_ET_MATCH_RD(
        Device_Handle_t Device,
        const unsigned int Index,
        uint8_t * const DA_Byte5,
        uint8_t * const DA_Byte6,
        uint16_t * const EtherType)
{
    uint32_t RegVal =  0;
    int rc = 0;

    rc = EIP163_Read32(Device, EIP163_REG_CP_MAC_DA_ET_MATCH(Index), &RegVal);

    if (!rc)
    {
        *DA_Byte5 = RegVal & MASK_8_BITS;
        *DA_Byte6 = (RegVal >> 8) & MASK_8_BITS;
        *EtherType = EIP163_Swap16(RegVal >> 16);
    }

    return rc;
}


static inline int
EIP163_CP_MAC_DA_START_LO_WR(
        Device_Handle_t Device,
        const unsigned int Index,
        const uint8_t DA_Byte1,
        const uint8_t DA_Byte2,
        const uint8_t DA_Byte3,
        const uint8_t DA_Byte4)
{
    uint32_t RegVal = 0; /* EIP163_REG_CP_MAC_DA_MATCH_DEFAULT; */

    RegVal |= ((DA_Byte4   & MASK_8_BITS) << 24);
    RegVal |= ((DA_Byte3   & MASK_8_BITS) << 16);
    RegVal |= ((DA_Byte2   & MASK_8_BITS) << 8);
    RegVal |= (DA_Byte1    & MASK_8_BITS);

    return EIP163_Write32(Device, EIP163_REG_CP_MAC_DA_START_LO(Index), RegVal);
}


static inline int
EIP163_CP_MAC_DA_START_LO_RD(
        Device_Handle_t Device,
        const unsigned int Index,
        uint8_t * const DA_Byte1,
        uint8_t * const DA_Byte2,
        uint8_t * const DA_Byte3,
        uint8_t * const DA_Byte4)
{
    uint32_t RegVal =  0;
    int rc = 0;

    rc = EIP163_Read32(Device, EIP163_REG_CP_MAC_DA_START_LO(Index), &RegVal);

    if (!rc)
    {
        *DA_Byte1 = RegVal & MASK_8_BITS;
        *DA_Byte2 = (RegVal >> 8) & MASK_8_BITS;
        *DA_Byte3 = (RegVal >> 16) & MASK_8_BITS;
        *DA_Byte4 = (RegVal >> 24) & MASK_8_BITS;
    }

    return rc;
}


static inline int
EIP163_CP_MAC_DA_START_HI_WR(
        Device_Handle_t Device,
        const unsigned int Index,
        const uint8_t DA_Byte5,
        const uint8_t DA_Byte6,
        const uint16_t EtherType)
{
    uint32_t RegVal = 0; /* EIP163_REG_CP_MAC_DA_ET_MATCH_DEFAULT; */
    uint32_t Swapped = EIP163_Swap16(EtherType);

    RegVal |= ((Swapped    & MASK_16_BITS) << 16);
    RegVal |= ((DA_Byte6   & MASK_8_BITS) << 8);
    RegVal |= (DA_Byte5    & MASK_8_BITS);

    return EIP163_Write32(Device, EIP163_REG_CP_MAC_DA_START_HI(Index), RegVal);
}

static inline int
EIP163_CP_MAC_DA_START_HI_RD(
        Device_Handle_t Device,
        const unsigned int Index,
        uint8_t * const DA_Byte5,
        uint8_t * const DA_Byte6,
        uint16_t * const EtherType)
{
    uint32_t RegVal =  0;
    int rc = 0;

    rc = EIP163_Read32(Device, EIP163_REG_CP_MAC_DA_START_HI(Index), &RegVal);

    if (!rc)
    {
        *DA_Byte5 = RegVal & MASK_8_BITS;
        *DA_Byte6 = (RegVal >> 8) & MASK_8_BITS;
        *EtherType = EIP163_Swap16(RegVal >> 16);
    }

    return rc;
}


static inline int
EIP163_CP_MAC_DA_END_LO_WR(
        Device_Handle_t Device,
        const unsigned int Index,
        const uint8_t DA_Byte1,
        const uint8_t DA_Byte2,
        const uint8_t DA_Byte3,
        const uint8_t DA_Byte4)
{
    uint32_t RegVal = 0; /* EIP163_REG_CP_MAC_DA_MATCH_DEFAULT; */

    RegVal |= ((DA_Byte4   & MASK_8_BITS) << 24);
    RegVal |= ((DA_Byte3   & MASK_8_BITS) << 16);
    RegVal |= ((DA_Byte2   & MASK_8_BITS) << 8);
    RegVal |= (DA_Byte1    & MASK_8_BITS);

    return EIP163_Write32(Device, EIP163_REG_CP_MAC_DA_END_LO(Index), RegVal);
}


static inline int
EIP163_CP_MAC_DA_END_LO_RD(
        Device_Handle_t Device,
        const unsigned int Index,
        uint8_t * const DA_Byte1,
        uint8_t * const DA_Byte2,
        uint8_t * const DA_Byte3,
        uint8_t * const DA_Byte4)
{
    uint32_t RegVal =  0;
    int rc = 0;

    rc = EIP163_Read32(Device, EIP163_REG_CP_MAC_DA_END_LO(Index), &RegVal);

    if (!rc)
    {
        *DA_Byte1 = RegVal & MASK_8_BITS;
        *DA_Byte2 = (RegVal >> 8) & MASK_8_BITS;
        *DA_Byte3 = (RegVal >> 16) & MASK_8_BITS;
        *DA_Byte4 = (RegVal >> 24) & MASK_8_BITS;
    }

    return rc;
}


static inline int
EIP163_CP_MAC_DA_END_HI_WR(
        Device_Handle_t Device,
        const unsigned int Index,
        const uint8_t DA_Byte5,
        const uint8_t DA_Byte6)
{
    uint32_t RegVal = 0; /* EIP163_REG_CP_MAC_DA_ET_MATCH_DEFAULT; */

    RegVal |= ((DA_Byte6   & MASK_8_BITS) << 8);
    RegVal |= (DA_Byte5    & MASK_8_BITS);

    return EIP163_Write32(Device, EIP163_REG_CP_MAC_DA_END_HI(Index), RegVal);
}


static inline int
EIP163_CP_MAC_DA_END_HI_RD(
        Device_Handle_t Device,
        const unsigned int Index,
        uint8_t * const DA_Byte5,
        uint8_t * const DA_Byte6)
{
    uint32_t RegVal =  0;
    int rc = 0;
    rc = EIP163_Read32(Device, EIP163_REG_CP_MAC_DA_END_HI(Index), &RegVal);

    if (!rc)
    {
        *DA_Byte5 = RegVal & MASK_8_BITS;
        *DA_Byte6 = (RegVal >> 8) & MASK_8_BITS;
    }

    return rc;
}


static inline int
EIP163_CP_MAC_DA_START_RO_LO_WR(
        Device_Handle_t Device,
        const uint8_t DA_Byte1,
        const uint8_t DA_Byte2,
        const uint8_t DA_Byte3,
        const uint8_t DA_Byte4)
{
    uint32_t RegVal = 0; /* EIP163_REG_CP_MAC_DA_MATCH_DEFAULT; */

    RegVal |= ((DA_Byte4   & MASK_8_BITS) << 24);
    RegVal |= ((DA_Byte3   & MASK_8_BITS) << 16);
    RegVal |= ((DA_Byte2   & MASK_8_BITS) << 8);
    RegVal |= (DA_Byte1    & MASK_8_BITS);

    return EIP163_Write32(Device, EIP163_REG_CP_MAC_DA_START_RO_LO, RegVal);
}


static inline int
EIP163_CP_MAC_DA_START_RO_LO_RD(
        Device_Handle_t Device,
        uint8_t * const DA_Byte1,
        uint8_t * const DA_Byte2,
        uint8_t * const DA_Byte3,
        uint8_t * const DA_Byte4)
{
    uint32_t RegVal =  0;
    int rc = 0;

    rc = EIP163_Read32(Device, EIP163_REG_CP_MAC_DA_START_RO_LO, &RegVal);

    if (!rc)
    {
        *DA_Byte1 = RegVal & MASK_8_BITS;
        *DA_Byte2 = (RegVal >> 8) & MASK_8_BITS;
        *DA_Byte3 = (RegVal >> 16) & MASK_8_BITS;
        *DA_Byte4 = (RegVal >> 24) & MASK_8_BITS;
    }

    return rc;
}


static inline int
EIP163_CP_MAC_DA_START_RO_HI_WR(
        Device_Handle_t Device,
        const uint8_t DA_Byte5,
        const uint8_t DA_Byte6)
{
    uint32_t RegVal = 0; /* EIP163_REG_CP_MAC_DA_ET_MATCH_DEFAULT; */

    RegVal |= ((DA_Byte6   & MASK_8_BITS) << 8);
    RegVal |= (DA_Byte5    & MASK_8_BITS);

    return EIP163_Write32(Device, EIP163_REG_CP_MAC_DA_START_RO_HI, RegVal);
}


static inline int
EIP163_CP_MAC_DA_START_RO_HI_RD(
        Device_Handle_t Device,
        uint8_t * const DA_Byte5,
        uint8_t * const DA_Byte6)
{
    uint32_t RegVal = 0;
    int rc = 0;

    rc = EIP163_Read32(Device, EIP163_REG_CP_MAC_DA_START_RO_HI, &RegVal);

    if (!rc)
    {
        *DA_Byte5 = RegVal & MASK_8_BITS;
        *DA_Byte6 = (RegVal >> 8) & MASK_8_BITS;
    }

    return rc;
}


static inline int
EIP163_CP_MAC_DA_END_RO_LO_WR(
        Device_Handle_t Device,
        const uint8_t DA_Byte1,
        const uint8_t DA_Byte2,
        const uint8_t DA_Byte3,
        const uint8_t DA_Byte4)
{
    uint32_t RegVal = 0; /* EIP163_REG_CP_MAC_DA_MATCH_DEFAULT; */

    RegVal |= ((DA_Byte4   & MASK_8_BITS) << 24);
    RegVal |= ((DA_Byte3   & MASK_8_BITS) << 16);
    RegVal |= ((DA_Byte2   & MASK_8_BITS) << 8);
    RegVal |= (DA_Byte1    & MASK_8_BITS);

    return EIP163_Write32(Device, EIP163_REG_CP_MAC_DA_END_RO_LO, RegVal);
}


static inline int
EIP163_CP_MAC_DA_END_RO_LO_RD(
        Device_Handle_t Device,
        uint8_t * const DA_Byte1,
        uint8_t * const DA_Byte2,
        uint8_t * const DA_Byte3,
        uint8_t * const DA_Byte4)
{
    uint32_t RegVal =  0;
    int rc = 0;

    rc = EIP163_Read32(Device, EIP163_REG_CP_MAC_DA_END_RO_LO, &RegVal);

    if (!rc)
    {
        *DA_Byte1 = RegVal & MASK_8_BITS;
        *DA_Byte2 = (RegVal >> 8) & MASK_8_BITS;
        *DA_Byte3 = (RegVal >> 16) & MASK_8_BITS;
        *DA_Byte4 = (RegVal >> 24) & MASK_8_BITS;
    }

    return rc;
}


static inline int
EIP163_CP_MAC_DA_END_RO_HI_WR(
        Device_Handle_t Device,
        const uint8_t DA_Byte5,
        const uint8_t DA_Byte6)
{
    uint32_t RegVal = 0; /* EIP163_REG_CP_MAC_DA_ET_MATCH_DEFAULT; */

    RegVal |= ((DA_Byte6   & MASK_8_BITS) << 8);
    RegVal |= (DA_Byte5    & MASK_8_BITS);

    return EIP163_Write32(Device, EIP163_REG_CP_MAC_DA_END_RO_HI, RegVal);
}


static inline int
EIP163_CP_MAC_DA_END_RO_HI_RD(
        Device_Handle_t Device,
        uint8_t * const DA_Byte5,
        uint8_t * const DA_Byte6)
{
    uint32_t RegVal = 0;
    int rc = 0;

    rc = EIP163_Read32(Device, EIP163_REG_CP_MAC_DA_END_RO_HI, &RegVal);

    if (!rc)
    {
        *DA_Byte5 = RegVal & MASK_8_BITS;
        *DA_Byte6 = (RegVal >> 8) & MASK_8_BITS;
    }

    return rc;
}


static inline int
EIP163_CP_MAC_DA_44_BITS_LO_WR(
        Device_Handle_t Device,
        const uint8_t DA_Byte1,
        const uint8_t DA_Byte2,
        const uint8_t DA_Byte3,
        const uint8_t DA_Byte4)
{
    uint32_t RegVal = 0; /* EIP163_REG_CP_MAC_DA_44_BITS_LO_DEFAULT; */

    RegVal |= ((DA_Byte4   & MASK_8_BITS) << 24);
    RegVal |= ((DA_Byte3   & MASK_8_BITS) << 16);
    RegVal |= ((DA_Byte2   & MASK_8_BITS) << 8);
    RegVal |= (DA_Byte1    & MASK_8_BITS);

    return EIP163_Write32(Device, EIP163_REG_CP_MAC_DA_44_BITS_LO, RegVal);
}


static inline int
EIP163_CP_MAC_DA_44_BITS_LO_RD(
        Device_Handle_t Device,
        uint8_t * const DA_Byte1,
        uint8_t * const DA_Byte2,
        uint8_t * const DA_Byte3,
        uint8_t * const DA_Byte4)
{
    uint32_t RegVal =  0;
    int rc = 0;

    rc = EIP163_Read32(Device, EIP163_REG_CP_MAC_DA_44_BITS_LO, &RegVal);

    if (!rc)
    {
        *DA_Byte1 = RegVal & MASK_8_BITS;
        *DA_Byte2 = (RegVal >> 8) & MASK_8_BITS;
        *DA_Byte3 = (RegVal >> 16) & MASK_8_BITS;
        *DA_Byte4 = (RegVal >> 24) & MASK_8_BITS;
    }

    return rc;
}


static inline int
EIP163_CP_MAC_DA_44_BITS_HI_WR(
        Device_Handle_t Device,
        const uint8_t DA_Byte5,
        const uint8_t DA_Byte6)
{
    uint32_t RegVal = 0; /* EIP163_REG_CP_MAC_DA_44_BITS_HI_DEFAULT; */

    RegVal |= ((DA_Byte6 &        0xF0) << 8);
    RegVal |= (DA_Byte5  & MASK_8_BITS);

    return EIP163_Write32(Device, EIP163_REG_CP_MAC_DA_44_BITS_HI, RegVal);
}


static inline int
EIP163_CP_MAC_DA_44_BITS_HI_RD(
        Device_Handle_t Device,
        uint8_t * const DA_Byte5,
        uint8_t * const DA_Byte6)
{
    uint32_t RegVal = 0;
    int rc = 0;

    rc = EIP163_Read32(Device, EIP163_REG_CP_MAC_DA_44_BITS_HI, &RegVal);

    if (!rc)
    {
        *DA_Byte5 = RegVal & MASK_8_BITS;
        *DA_Byte6 = (RegVal >> 8) & MASK_8_BITS;
    }

    return rc;
}


static inline int
EIP163_CP_MAC_DA_48_BITS_LO_WR(
        Device_Handle_t Device,
        const uint8_t DA_Byte1,
        const uint8_t DA_Byte2,
        const uint8_t DA_Byte3,
        const uint8_t DA_Byte4)
{
    uint32_t RegVal = 0; /* EIP163_REG_CP_MAC_DA_48_BITS_LO_DEFAULT; */

    RegVal |= ((DA_Byte4   & MASK_8_BITS) << 24);
    RegVal |= ((DA_Byte3   & MASK_8_BITS) << 16);
    RegVal |= ((DA_Byte2   & MASK_8_BITS) << 8);
    RegVal |= (DA_Byte1    & MASK_8_BITS);

    return EIP163_Write32(Device, EIP163_REG_CP_MAC_DA_48_BITS_LO, RegVal);
}


static inline int
EIP163_CP_MAC_DA_48_BITS_LO_RD(
        Device_Handle_t Device,
        uint8_t * const DA_Byte1,
        uint8_t * const DA_Byte2,
        uint8_t * const DA_Byte3,
        uint8_t * const DA_Byte4)
{
    uint32_t RegVal =  0;
    int rc = 0;

    rc = EIP163_Read32(Device, EIP163_REG_CP_MAC_DA_48_BITS_LO, &RegVal);

    if (!rc)
    {
        *DA_Byte1 = RegVal & MASK_8_BITS;
        *DA_Byte2 = (RegVal >> 8) & MASK_8_BITS;
        *DA_Byte3 = (RegVal >> 16) & MASK_8_BITS;
        *DA_Byte4 = (RegVal >> 24) & MASK_8_BITS;
    }

    return rc;
}


static inline int
EIP163_CP_MAC_DA_48_BITS_HI_WR(
        Device_Handle_t Device,
        const uint8_t DA_Byte5,
        const uint8_t DA_Byte6)
{
    uint32_t RegVal = 0; /* EIP163_REG_CP_MAC_DA_48_BITS_HI_DEFAULT; */

    RegVal |= ((DA_Byte6   & MASK_8_BITS) << 8);
    RegVal |= (DA_Byte5    & MASK_8_BITS);

    return EIP163_Write32(Device, EIP163_REG_CP_MAC_DA_48_BITS_HI, RegVal);
}


static inline int
EIP163_CP_MAC_DA_48_BITS_HI_RD(
        Device_Handle_t Device,
        uint8_t * const DA_Byte5,
        uint8_t * const DA_Byte6)
{
    uint32_t RegVal = 0;
    int rc = 0;

    rc = EIP163_Read32(Device, EIP163_REG_CP_MAC_DA_48_BITS_HI, &RegVal);

    if (!rc)
    {
        *DA_Byte5 = RegVal & MASK_8_BITS;
        *DA_Byte6 = (RegVal >> 8) & MASK_8_BITS;
    }

    return rc;
}


static inline int
EIP163_SAM_CP_TAG_WR(
        Device_Handle_t Device,
        const uint8_t UP0,
        const uint8_t UP1,
        const uint8_t UP2,
        const uint8_t UP3,
        const uint8_t UP4,
        const uint8_t UP5,
        const uint8_t UP6,
        const uint8_t UP7,
        const uint8_t DefaultUP,
        const bool fSTagUPEnable,
        const bool fQTagUPEnable)
{
    uint32_t RegVal = EIP163_REG_SAM_CP_TAG_DEFAULT;

    if(fQTagUPEnable)
        RegVal |= BIT_28;

    if(fSTagUPEnable)
        RegVal |= BIT_27;

    RegVal |= ((DefaultUP & MASK_3_BITS)  << 24);
    RegVal |= ((UP7       & MASK_3_BITS)  << 21);
    RegVal |= ((UP6       & MASK_3_BITS)  << 18);
    RegVal |= ((UP5       & MASK_3_BITS)  << 15);
    RegVal |= ((UP4       & MASK_3_BITS)  << 12);
    RegVal |= ((UP3       & MASK_3_BITS)  << 9);
    RegVal |= ((UP2       & MASK_3_BITS)  << 6);
    RegVal |= ((UP1       & MASK_3_BITS)  << 3);
    RegVal |= (UP0        & MASK_3_BITS);

    return EIP163_Write32(Device, EIP163_REG_SAM_CP_TAG, RegVal);
}


static inline int
EIP163_SAM_CP_TAG_RD(
        Device_Handle_t Device,
        uint8_t * const UP0,
        uint8_t * const UP1,
        uint8_t * const UP2,
        uint8_t * const UP3,
        uint8_t * const UP4,
        uint8_t * const UP5,
        uint8_t * const UP6,
        uint8_t * const UP7,
        uint8_t * const DefaultUP,
        bool * const fSTagUPEnable,
        bool * const fQTagUPEnable)
{
    uint32_t RegVal = 0;
    int rc = 0;

    rc = EIP163_Read32(Device, EIP163_REG_SAM_CP_TAG, &RegVal);

    if (!rc)
    {
      *fQTagUPEnable = (RegVal & BIT_28) != 0;
      *fSTagUPEnable = (RegVal & BIT_27) != 0;

      *DefaultUP = (RegVal >> 24) & MASK_3_BITS;
      *UP7       = (RegVal >> 21) & MASK_3_BITS;
      *UP6       = (RegVal >> 18) & MASK_3_BITS;
      *UP5       = (RegVal >> 15) & MASK_3_BITS;
      *UP4       = (RegVal >> 12) & MASK_3_BITS;
      *UP3       = (RegVal >>  9) & MASK_3_BITS;
      *UP2       = (RegVal >>  6) & MASK_3_BITS;
      *UP1       = (RegVal >>  3) & MASK_3_BITS;
      *UP0       =  RegVal        & MASK_3_BITS;
    }

    return rc;
}


static inline int
EIP163_SAM_CP_TAG2_WR(
        Device_Handle_t Device,
        const uint8_t UP0,
        const uint8_t UP1,
        const uint8_t UP2,
        const uint8_t UP3,
        const uint8_t UP4,
        const uint8_t UP5,
        const uint8_t UP6,
        const uint8_t UP7)
{
    uint32_t RegVal = EIP163_REG_SAM_CP_TAG2_DEFAULT;

    RegVal |= ((UP7       & MASK_3_BITS)  << 21);
    RegVal |= ((UP6       & MASK_3_BITS)  << 18);
    RegVal |= ((UP5       & MASK_3_BITS)  << 15);
    RegVal |= ((UP4       & MASK_3_BITS)  << 12);
    RegVal |= ((UP3       & MASK_3_BITS)  << 9);
    RegVal |= ((UP2       & MASK_3_BITS)  << 6);
    RegVal |= ((UP1       & MASK_3_BITS)  << 3);
    RegVal |= (UP0        & MASK_3_BITS);

    return EIP163_Write32(Device, EIP163_REG_SAM_CP_TAG2, RegVal);
}


static inline int
EIP163_SAM_CP_TAG2_RD(
        Device_Handle_t Device,
        uint8_t * const UP0,
        uint8_t * const UP1,
        uint8_t * const UP2,
        uint8_t * const UP3,
        uint8_t * const UP4,
        uint8_t * const UP5,
        uint8_t * const UP6,
        uint8_t * const UP7)
{
    uint32_t RegVal = 0;
    int rc = 0;

    rc = EIP163_Read32(Device, EIP163_REG_SAM_CP_TAG2, &RegVal);

    if (!rc)
    {
      *UP7 = (RegVal >> 21) & MASK_3_BITS;
      *UP6 = (RegVal >> 18) & MASK_3_BITS;
      *UP5 = (RegVal >> 15) & MASK_3_BITS;
      *UP4 = (RegVal >> 12) & MASK_3_BITS;
      *UP3 = (RegVal >>  9) & MASK_3_BITS;
      *UP2 = (RegVal >>  6) & MASK_3_BITS;
      *UP1 = (RegVal >>  3) & MASK_3_BITS;
      *UP0 =  RegVal        & MASK_3_BITS;
    }

    return rc;
}



static inline int
EIP163_CHANNEL_CTRL_RD(
        Device_Handle_t Device,
        const unsigned int ChannelId,
        bool * const fBypassMode)
{
    uint32_t RegVal = 0;
    int rc = 0;

    rc = EIP163_Read32(Device, EIP163_REG_CHANNEL_CTRL(ChannelId), &RegVal);

    if (!rc)
    {
      *fBypassMode = ((RegVal & BIT_0) != 0);
    }

    return rc;
}


static inline int
EIP163_CHANNEL_CTRL_DEFAULT_WR(
        Device_Handle_t Device,
        const unsigned int ChannelId)
{
    return EIP163_Write32(Device,
                   EIP163_REG_CHANNEL_CTRL(ChannelId),
                   EIP163_REG_CHANNEL_CTRL_DEFAULT);
}


static inline int
EIP163_CHANNEL_CTRL_WR(
        Device_Handle_t Device,
        const unsigned int Index,
        const bool fBypassMode)
{
    uint32_t RegVal = EIP163_REG_CHANNEL_CTRL_DEFAULT;

    if (fBypassMode)
        RegVal |= BIT_0;
    else
        RegVal &= ~BIT_0;

    return EIP163_Write32(Device, EIP163_REG_CHANNEL_CTRL(Index), RegVal);
}


static inline int
EIP163_FORCE_CLOCK_ON_WR(
        Device_Handle_t Device,
        const uint32_t ClockMask)
{
    uint32_t RegVal = (ClockMask & MASK_4_BITS);

    return EIP163_Write32(Device, EIP163_REG_FORCE_CLOCK_ON, RegVal);
}


static inline int
EIP163_FORCE_CLOCK_OFF_WR(
        Device_Handle_t Device,
        const uint32_t ClockMask)
{
    uint32_t RegVal = (ClockMask & MASK_4_BITS);

    return EIP163_Write32(Device, EIP163_REG_FORCE_CLOCK_OFF, RegVal);
}


static inline int
EIP163_LOW_LAT_CONFIG_WR(
        Device_Handle_t Device,
        bool fLowLatencyBypass)
{
    return EIP163_Write32(Device, EIP163_REG_LOW_LAT_CONFIG,
                   fLowLatencyBypass ? 1 : 0);
}


static inline int
EIP163_RX_INSERT_SOP_WR(
        Device_Handle_t Device,
        const uint16_t ChannelCount,
        const EIP163_Ch_Mask_t * const ChannelMask_p)
{
    int rc = 0;
    uint16_t i = 0;

    for (i = 0; i < ((ChannelCount + 31) / 32); i++)
    {
         rc = EIP163_Write32(Device,
                             EIP163_REG_RX_INSERT_SOP(i),
                             ChannelMask_p->ch_bitmask[i]);
        if (rc) return rc;
    }

    return rc;
}


static inline int
EIP163_RX_INSERT_EOP_WR(
        Device_Handle_t Device,
        const uint16_t ChannelCount,
        const EIP163_Ch_Mask_t * const ChannelMask_p)
{
    int rc = 0;
    uint16_t i = 0;

    for (i = 0; i < ((ChannelCount + 31) / 32); i++)
    {
        rc = EIP163_Write32(Device,
                            EIP163_REG_RX_INSERT_EOP(i),
                            ChannelMask_p->ch_bitmask[i]);
        if (rc) return rc;
    }

    return rc;
}


static inline int
EIP163_RX_TIMEOUT_CTRL_WR(
        Device_Handle_t Device,
        const uint16_t ChannelCount,
        const EIP163_Ch_Mask_t * const ChannelMask_p)
{
    uint16_t i = 0;
    int rc = 0;

    for (i = 0; i < (ChannelCount + 31) / 32; i++)
    {
        rc = EIP163_Write32(Device,
                            EIP163_REG_RX_TIMEOUT_CTRL(i),
                            ChannelMask_p->ch_bitmask[i]);
        if (rc) return rc;
    }

    return rc;
}


static inline int
EIP163_RX_TIMEOUT_CTRL_RD(
        Device_Handle_t Device,
        const uint16_t ChannelCount,
        EIP163_Ch_Mask_t * const ChannelMask_p)
{
    uint16_t i = 0;
    int rc = 0;

    for (i = 0; i < ((ChannelCount + 31) / 32); i++)
    {
        rc = EIP163_Read32(Device,
                           EIP163_REG_RX_TIMEOUT_CTRL(i),
                           &ChannelMask_p->ch_bitmask[i]);
        if (rc) return rc;
    }

    return rc;
}

static inline int
EIP163_RX_TIMEOUT_VAL_WR(
        Device_Handle_t Device,
        unsigned int TimeoutVal)
{
    uint32_t RegVal = TimeoutVal & MASK_16_BITS;

    return EIP163_Write32(Device, EIP163_REG_RX_TIMEOUT_VAL, RegVal);
}

static inline int
EIP163_RX_TIMEOUT_VAL_RD(
        Device_Handle_t Device,
        unsigned int * TimeoutVal)
{
    uint32_t RegVal = 0;
    int rc = 0;

    rc = EIP163_Read32(Device, EIP163_REG_RX_TIMEOUT_VAL, &RegVal);

    if (!rc)
    {
      *TimeoutVal = RegVal & MASK_16_BITS;
    }

    return rc;
}

static inline int
EIP163_ECC_COUNTER_RAM_RD(
        Device_Handle_t Device,
        unsigned int Index,
        unsigned int *CorrValue,
        unsigned int *DerrValue)
{
    uint32_t RegVal = 0;
    int rc = 0;

    rc = EIP163_Read32(Device, EIP163_REG_ECC_COUNTER_RAM(Index), &RegVal);

    if (!rc)
    {
       *CorrValue = (RegVal >> 16) & MASK_16_BITS;
       *DerrValue = RegVal & MASK_16_BITS;
    }

    return rc;
}

static inline int
EIP163_ECC_COUNTER_RAM_WR(
        Device_Handle_t Device,
        unsigned int Index,
        unsigned int CorrValue,
        unsigned int DerrValue)
{

    uint32_t RegVal = ((CorrValue & MASK_16_BITS) << 16) |
                      (DerrValue & MASK_16_BITS);

    return EIP163_Write32(Device, EIP163_REG_ECC_COUNTER_RAM(Index), RegVal);
}


static inline int
EIP163_ECC_THR_CORR_STAT_RD(
        Device_Handle_t Device,
        uint32_t *ThrMask)
{
    return EIP163_Read32(Device, EIP163_REG_ECC_THR_CORR_STAT, ThrMask);
}

static inline int
EIP163_ECC_THR_DERR_STAT_RD(
        Device_Handle_t Device,
        uint32_t *ThrMask)
{
    return EIP163_Read32(Device, EIP163_REG_ECC_THR_DERR_STAT, ThrMask);
}


static inline int
EIP163_ECC_CORR_ENABLE_WR(
        Device_Handle_t Device,
        uint32_t EnableMask)
{
    return EIP163_Write32(Device, EIP163_REG_ECC_CORR_ENABLE, EnableMask);
}

static inline int
EIP163_ECC_DERR_ENABLE_WR(
        Device_Handle_t Device,
        uint32_t EnableMask)
{
    return EIP163_Write32(Device, EIP163_REG_ECC_DERR_ENABLE, EnableMask);
}

static inline int
EIP163_ECC_THRESHOLDS_WR(
        Device_Handle_t Device,
        unsigned int CorrValue,
        unsigned int DerrValue)
{
    uint32_t RegVal = ((CorrValue & MASK_16_BITS) << 16) |
                      (DerrValue & MASK_16_BITS);

    return EIP163_Write32(Device, EIP163_REG_ECC_THRESHOLDS, RegVal);
}

static inline int
EIP163_ECC_THRESHOLDS_RD(
        Device_Handle_t Device,
        unsigned int * CorrValue,
        unsigned int * DerrValue)
{
    uint32_t RegVal = 0;
    int rc = 0;

    rc = EIP163_Read32(Device, EIP163_REG_ECC_THRESHOLDS, &RegVal);

    if (!rc)
    {
      *CorrValue = (RegVal >> 16) & MASK_16_BITS;
      *DerrValue = RegVal & MASK_16_BITS;
    }

    return rc;
}

#endif /* EIP163_LEVEL0_H_ */


/* end of file eip163_level0.h */
