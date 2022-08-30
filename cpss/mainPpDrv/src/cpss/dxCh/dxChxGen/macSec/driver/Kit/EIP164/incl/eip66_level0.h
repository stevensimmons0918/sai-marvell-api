/* eip66_level0.h
 *
 * EIP-66 Engine Level0 Internal interface
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

#ifndef EIP66_LEVEL0_H_
#define EIP66_LEVEL0_H_


/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

/* Default configuration */
#include <Kit/EIP164/incl/c_eip66.h>

/* Driver Framework Basic Definitions API */
#include <Kit/DriverFramework/incl/basic_defs.h>         /* BIT definitions, bool, uint32_t */

/* Driver Framework Device API */
#include <Kit/DriverFramework/incl/device_types.h>       /* Device_Handle_t */
#include <Kit/DriverFramework/incl/device_rw.h>          /* Read32, Write32 */


/*----------------------------------------------------------------------------
 * Definitions and macros
 */

/* Read/Write register constants */

/*****************************************************************************
 * Byte offsets of the EIP-66 Packet Engine registers
 *****************************************************************************/
/* EIP-66 EIP number (0x42) and complement (0xBD) */
#define EIP66_SIGNATURE                   ((uint16_t)0xBD42)

#define EIP66_REG_OFFS                     4
#define EIP66_REG_CHANNEL_CTRL_STAT_OFFS   0x20


#define EIP66_REG_CTRL(n)                  (EIP66_CONF_BASE + 0x0000 + \
                                            (0x10000 * ((n) >> 5)) + \
                                            (EIP66_REG_CHANNEL_CTRL_STAT_OFFS * ((n)& 0x1F)))
#define EIP66_REG_STAT(n)                  (EIP66_CONF_BASE + 0x0004 + \
                                                (0x10000 * ((n)>> 5)) + \
                                            (EIP66_REG_CHANNEL_CTRL_STAT_OFFS * ((n)& 0x1F)))
#define EIP66_REG_SEQ_NR_THRESH(n)         (EIP66_CONF_BASE + 0x0008 + \
                                                (0x10000 * ((n)>> 5)) + \
                                            (EIP66_REG_CHANNEL_CTRL_STAT_OFFS * ((n)& 0x1F)))
#define EIP66_REG_SEQ_NR_THRESH_64_LO(n)   (EIP66_CONF_BASE + 0x000C + \
                                                (0x10000 * ((n)>> 5)) + \
                                            (EIP66_REG_CHANNEL_CTRL_STAT_OFFS * ((n)& 0x1F)))
#define EIP66_REG_SEQ_NR_THRESH_64_HI(n)   (EIP66_CONF_BASE + 0x0010 + \
                                            (0x10000 * ((n)>> 5)) + \
                                            (EIP66_REG_CHANNEL_CTRL_STAT_OFFS * ((n)& 0x1F)))
#define EIP66_REG_CTX_BLK_UPD_CTRL(n)      (EIP66_CONF_BASE + 0x0014 + \
                                            (0x10000 * ((n)>> 5)) + \
                                            (EIP66_REG_CHANNEL_CTRL_STAT_OFFS * ((n)& 0x1F)))

#define EIP66_REG_NEXTPN_LO                (EIP66_CONF_BASE + 0x0780)
#define EIP66_REG_NEXTPN_HI                (EIP66_CONF_BASE + 0x0784)
#define EIP66_REG_NEXTPN_CTX_ADDR          (EIP66_CONF_BASE + 0x078C)
#define EIP66_REG_NEXTPN_CTRL              (EIP66_CONF_BASE + 0x0790)

#define EIP66_REG_CTX_UPD_CTRL             (EIP66_CONF_BASE + 0x07C0)

#define EIP66_REG_CLOCK_STATE              (EIP66_CONF_BASE + 0x07E8)
#define EIP66_REG_FORCE_CLOCK_ON           (EIP66_CONF_BASE + 0x07EC)
#define EIP66_REG_FORCE_CLOCK_OFF          (EIP66_CONF_BASE + 0x07F0)

#define EIP66_REG_CONFIG                   (EIP66_CONF_BASE + 0x07F8)
#define EIP66_REG_VERSION                  (EIP66_CONF_BASE + 0x07FC)

/* As per CONTEXT STATUS register */
#define EIP66_FATAL_ERROR_MASK             (MASK_7_BITS | BIT_13)


/*----------------------------------------------------------------------------
 * EIP66_Swap16
 *
 * Swaps 2 bytes in a 2-byte word
 */
static inline uint16_t
EIP66_Swap16(
        const uint16_t Value16)
{
    return ((Value16 & MASK_8_BITS) << 8) | ((Value16 >> 8) & MASK_8_BITS);
}


/*----------------------------------------------------------------------------
 * EIP66_Read32
 *
 * This routine writes to a Register location in the EIP-66.
 */
static inline int
EIP66_Read32(
        Device_Handle_t Device,
        const unsigned int Offset,
        uint32_t * const Value_p)
{
    return Device_Read32Check(Device, Offset, Value_p);
}


/*----------------------------------------------------------------------------
 * EIP66_Write32
 *
 * This routine writes to a Register location in the EIP-66.
 */
static inline int
EIP66_Write32(
        Device_Handle_t Device,
        const unsigned int Offset,
        const uint32_t Value)
{
    return Device_Write32(Device, Offset, Value);
}


static inline bool
EIP66_REV_SIGNATURE_MATCH(
        uint32_t RegVal)
{
    return (((uint16_t)RegVal) == EIP66_SIGNATURE);
}


static inline int
EIP66_EIP_REV_RD(
        Device_Handle_t Device,
        uint8_t * const EipNumber,
        uint8_t * const ComplmtEipNumber,
        uint8_t * const HWPatchLevel,
        uint8_t * const MinHWRevision,
        uint8_t * const MajHWRevision)
{
    int rc = 0;
    uint32_t RegVal = EIP66_Read32(Device, EIP66_REG_VERSION, &RegVal);

    if (!rc)
    {
        *MajHWRevision    = (uint8_t)((RegVal >> 24) & MASK_4_BITS);
        *MinHWRevision    = (uint8_t)((RegVal >> 20) & MASK_4_BITS);
        *HWPatchLevel     = (uint8_t)((RegVal >> 16) & MASK_4_BITS);
        *ComplmtEipNumber = (uint8_t)((RegVal >>  8) & MASK_8_BITS);
        *EipNumber        = (uint8_t)((RegVal)       & MASK_8_BITS);
    }

    return rc;
}


static inline int
EIP66_CONFIG_RD(
        Device_Handle_t Device,
        bool * const fEgressOnly,
        bool * const fIngressOnly,
        uint8_t * const ChannelCount,
        uint8_t * const PipeCount)
{
    uint32_t RegVal = 0;
    int rc = 0;

    rc = EIP66_Read32(Device, EIP66_REG_CONFIG, &RegVal);

    if (!rc)
    {
        *fEgressOnly  = ((RegVal & BIT_31) != 0);
        *fIngressOnly = ((RegVal & BIT_30) != 0);

        *ChannelCount = ((((RegVal >> 16) & MASK_2_BITS) << 6) |
                         ((RegVal >> 24) & MASK_6_BITS));
        *PipeCount = RegVal & MASK_5_BITS;
    }

    return rc;
}


static inline int
EIP66_CTRL_WR(
        Device_Handle_t Device,
        unsigned int Channel,
        const bool fPktNumThrMode,
        const uint16_t EtherType)
{
    uint32_t RegVal = 0;

    RegVal = (((uint32_t)EIP66_Swap16(EtherType)) & MASK_16_BITS) << 16;

    if(fPktNumThrMode)
        RegVal |= BIT_10;

    return EIP66_Write32(Device, EIP66_REG_CTRL(Channel), RegVal);
}


static inline int
EIP66_CTRL_RD(
        Device_Handle_t Device,
        unsigned int Channel,
        bool * const fPktNumThrMode_p,
        uint16_t * const EtherType_p)
{
    int rc = 0;
    uint32_t RegVal = 0;

    rc = EIP66_Read32(Device, EIP66_REG_CTRL(Channel), &RegVal);

    if (!rc)
    {
        *fPktNumThrMode_p   = (RegVal & BIT_10) != 0;
        *EtherType_p        = EIP66_Swap16(RegVal >> 16);
    }

    return rc;
}


static inline int
EIP66_CTX_BLK_UPD_CTRL_WR(
        Device_Handle_t Device,
        unsigned int Channel,
        uint8_t BlockContextUpdate)
{
    uint32_t RegVal = ((uint32_t)BlockContextUpdate) & MASK_2_BITS;

    return EIP66_Write32(Device, EIP66_REG_CTX_BLK_UPD_CTRL(Channel), RegVal);
}


static inline int
EIP66_SEQ_NR_THRESH_WR(
        Device_Handle_t Device,
        unsigned int Channel,
        uint32_t Threshold)
{
    return EIP66_Write32(Device, EIP66_REG_SEQ_NR_THRESH(Channel), Threshold);
}


static inline int
EIP66_SEQ_NR_THRESH_RD(
        Device_Handle_t Device,
        unsigned int Channel,
        uint32_t * Threshold_p)
{
    return EIP66_Read32(Device, EIP66_REG_SEQ_NR_THRESH(Channel), Threshold_p);
}


static inline int
EIP66_SEQ_NR_THRESH_64_WR(
        Device_Handle_t Device,
        unsigned int Channel,
        uint32_t Threshold64Lo,
        uint32_t Threshold64Hi)
{
    int rc = 0;

    rc = EIP66_Write32(Device,
                       EIP66_REG_SEQ_NR_THRESH_64_LO(Channel),
                       Threshold64Lo);
    if (rc) return rc;

    return EIP66_Write32(Device,
                         EIP66_REG_SEQ_NR_THRESH_64_HI(Channel),
                         Threshold64Hi);
}


static inline int
EIP66_SEQ_NR_THRESH_64_RD(
        Device_Handle_t Device,
        unsigned int Channel,
        uint32_t * Threshold64Lo,
        uint32_t * Threshold64Hi)
{
    int rc = 0;

    rc = EIP66_Read32(Device,
                      EIP66_REG_SEQ_NR_THRESH_64_LO(Channel),
                      Threshold64Lo);
    if (rc) return rc;

    return EIP66_Read32(Device,
                        EIP66_REG_SEQ_NR_THRESH_64_HI(Channel),
                        Threshold64Hi);
}


static inline int
EIP66_NEXTPN_LO_WR(
        Device_Handle_t Device,
        uint32_t NextPN_Lo)
{
    return EIP66_Write32(Device, EIP66_REG_NEXTPN_LO, NextPN_Lo);
}

static inline int
EIP66_NEXTPN_HI_WR(
        Device_Handle_t Device,
        uint32_t NextPN_Hi)
{
    return EIP66_Write32(Device, EIP66_REG_NEXTPN_HI, NextPN_Hi);
}

static inline int
EIP66_NEXTPN_CTX_ADDR_WR(
        Device_Handle_t Device,
        uint32_t CtxAddr)
{
    return EIP66_Write32(Device, EIP66_REG_NEXTPN_CTX_ADDR, CtxAddr);
}

static inline int
EIP66_NEXTPN_CTRL_WR(
        Device_Handle_t Device)
{
    uint32_t RegVal = BIT_0;

    return EIP66_Write32(Device, EIP66_REG_NEXTPN_CTRL, RegVal);
}

static inline int
EIP66_NEXTPN_CTRL_RD(
        Device_Handle_t Device,
        bool * const fNextPNWritten_p,
        bool * const fEnableUpdate_p,
        bool * const fECCError_p)
{
    uint32_t RegVal = 0;
    int rc = 0;

    rc = EIP66_Read32(Device, EIP66_REG_NEXTPN_CTRL, &RegVal);

    if (!rc)
    {
        *fECCError_p = ((RegVal & BIT_2) != 0);
        *fNextPNWritten_p = ((RegVal & BIT_1) != 0);
        *fEnableUpdate_p = ((RegVal & BIT_0) != 0);
    }

    return rc;
}

static inline int
EIP66_CTX_UPD_CTRL_WR(
        Device_Handle_t Device,
        bool fCompareMoreThanEqual,
        uint8_t CompareValue)
{
    uint32_t RegVal = ((uint32_t)CompareValue) & MASK_8_BITS;

    if(fCompareMoreThanEqual)
        RegVal |= BIT_31;

    return EIP66_Write32(Device, EIP66_REG_CTX_UPD_CTRL, RegVal);
}

static inline int
EIP66_FORCE_CLOCK_ON_WR(
        Device_Handle_t Device,
        uint8_t ClockMask)
{
    uint32_t RegVal = ClockMask;
    return EIP66_Write32(Device, EIP66_REG_FORCE_CLOCK_ON, RegVal);
}


static inline int
EIP66_FORCE_CLOCK_OFF_WR(
        Device_Handle_t Device,
        uint8_t ClockMask)
{
    uint32_t RegVal = ClockMask;

    return EIP66_Write32(Device, EIP66_REG_FORCE_CLOCK_OFF, RegVal);
}



#endif /* EIP66_LEVEL0_H_ */


/* end of file eip66_level0.h */
