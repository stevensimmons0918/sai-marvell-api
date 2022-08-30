/* eip217_level0.h
 *
 * EIP-217 Statistics module Level0 Internal interface
 */

/*****************************************************************************
* Copyright (c) 2015-2019 INSIDE Secure B.V. All Rights Reserved.
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

#ifndef EIP217_LEVEL0_H_
#define EIP217_LEVEL0_H_


/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

/* Default configuration */
#include <Kit/EIP217/src/c_eip217.h>

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
 * Byte offsets of the EIP-217 device registers
 *****************************************************************************/
/* EIP-217 EIP number (0xD9) and complement (0x26) */
#define EIP217_SIGNATURE                   ((uint16_t)0x26D9)

#define EIP217_REG_OFFS                     4

/* EIP-217 has 5 register banks: */
/* - global (not used yet) */
/* - 64-bit statistics counters */
/* - statistics controls, options & version */
/* - (per-channel) 64-bit counters */
/* - per-counter summary */

/* EIP-217 register bank: 64-bit statistics counters (n) */
#define EIP217_REG_COUNTER_OFFS             8
#define EIP217_REG_COUNTER_LO(n)            (EIP217_CONF_BASE + \
                                             EIP217_REG_COUNTER_OFFS * n)
#define EIP217_REG_COUNTER_HI(n)            (EIP217_CONF_BASE + \
                                             EIP217_REG_OFFS + \
                                             EIP217_REG_COUNTER_OFFS * n)

/* EIP-217 register bank: statistics controls */
#define EIP217_REG_COUNT_CONTROL            (EIP217_CONF_BASE + 0x10)
#define EIP217_REG_COUNT_INCEN1             (EIP217_CONF_BASE + 0x14)
#define EIP217_REG_COUNT_INCEN2             (EIP217_CONF_BASE + 0x18)
#define EIP217_REG_COUNT_PACK_TIMING        (EIP217_CONF_BASE + 0x1C)
#define EIP217_REG_COUNT_FRAME_THR1         (EIP217_CONF_BASE + 0x20)
#define EIP217_REG_COUNT_FRAME_THR2         (EIP217_CONF_BASE + 0x24)
#define EIP217_REG_COUNT_OCTET_THR1         (EIP217_CONF_BASE + 0x30)
#define EIP217_REG_COUNT_OCTET_THR2         (EIP217_CONF_BASE + 0x34)
#define EIP217_REG_COUNT_POSTED_CTRL        (EIP217_CONF_BASE + 0x38)
#define EIP217_REG_COUNT_SUMMARY_GL         (EIP217_CONF_BASE + 0x40)
#define EIP217_REG_COUNT_SUMMARY_CHAN1      (EIP217_CONF_BASE + 0x44)
#define EIP217_REG_COUNT_SUMMARY_P(n,s)     (EIP217_CONF_BASE +  \
                                             0x10000 * ((n)>>(s)) +     \
                                             EIP217_REG_OFFS * ((n) & ((1<<(s))-1)))
#define EIP217_REG_OPTIONS2                 (EIP217_CONF_BASE + 0x74)
#define EIP217_REG_OPTIONS                  (EIP217_CONF_BASE + 0x78)
#define EIP217_REG_VERSION                  (EIP217_CONF_BASE + 0x7C)

/* EIP-217 register bank: per-counter summary */
#define EIP217_REG_COUNT_SUMMARY(n,s)         (EIP217_CONF_BASE +  \
                                               0x10000 * ((n)>>(s)) +   \
                                               EIP217_REG_OFFS * ((n) & ((1<<(s))-1)))

/* EIP-217 register bank: (per-channel) 64-bit counters */
/* Use macros from statistics counters! */

/* Register default (reset) values */
#define EIP217_REG_COUNT_CONTROL_DEFAULT    0x00000001
#define EIP217_REG_COUNT_FRAME_THR1_DEFAULT 0x00000000
#define EIP217_REG_COUNT_FRAME_THR2_DEFAULT 0x00000000
#define EIP217_REG_COUNTER_LO_DEFAULT       0x00000000
#define EIP217_REG_COUNTER_HI_DEFAULT       0x00000000


/*----------------------------------------------------------------------------
 * EIP217_Read32
 *
 * This routine writes to a Register location in the EIP-217.
 */
static inline int
EIP217_Read32(
        Device_Handle_t Device,
        const unsigned int Offset,
        uint32_t * const Value_p)
{
    return Device_Read32Check(Device, Offset, Value_p);
}


/*----------------------------------------------------------------------------
 * EIP217_Write32
 *
 * This routine writes to a Register location in the EIP-217.
 */
static inline int
EIP217_Write32(
        Device_Handle_t Device,
        const unsigned int Offset,
        const uint32_t Value)
{
    return Device_Write32(Device, Offset, Value);
}


/*----------------------------------------------------------------------------
 * EIP217_Read32Array
 *
 * This routine reads MemoryDst_p array of Count 32-bit words from a Register
 * location in the EIP-217 at Offset.
 */
static inline int
EIP217_Read32Array(
        Device_Handle_t Device,
        const unsigned int Offset,
        uint32_t * MemoryDst_p,
        const int Count)
{
    return Device_Read32Array(Device, Offset, MemoryDst_p, Count);
}


/*----------------------------------------------------------------------------
 * EIP217_Write32Array
 *
 * This routine writes MemorySrc_p array of Count 32-bit words to a Register
 * location in the EIP-217 at Offset.
 */
static inline int
EIP217_Write32Array(
        Device_Handle_t Device,
        const unsigned int Offset,
        const uint32_t * MemorySrc_p,
        const int Count)
{
    return Device_Write32Array(Device, Offset, MemorySrc_p, Count);
}


static inline bool
EIP217_REV_SIGNATURE_MATCH(
        const uint32_t Rev)
{
    return (((uint16_t)Rev) == EIP217_SIGNATURE);
}


static inline int
EIP217_EIP_REV_RD(
        Device_Handle_t Device,
        const unsigned int Offset,
        uint8_t * const EipNumber,
        uint8_t * const ComplmtEipNumber,
        uint8_t * const HWPatchLevel,
        uint8_t * const MinHWRevision,
        uint8_t * const MajHWRevision)
{
    uint32_t RegVal;
    int rc = 0;

    rc = EIP217_Read32(Device, Offset + EIP217_REG_VERSION, &RegVal);

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
EIP217_OPTIONS_RD(
        Device_Handle_t Device,
        const unsigned int Offset,
        uint8_t * const CountersCount_p)
{
    uint32_t RegVal;

    int rc = EIP217_Read32(Device, Offset + EIP217_REG_OPTIONS, &RegVal);

    if (!rc)
    {
        *CountersCount_p = (uint8_t)((RegVal) & MASK_5_BITS);
    }

    return rc;
}


static inline int
EIP217_OPTIONS2_RD(
        Device_Handle_t Device,
        const unsigned int Offset,
        uint16_t * const Counters_p,
        uint8_t * const DW_BitCount_p)
{
    uint32_t RegVal = 0;
    int rc = 0;

    rc = EIP217_Read32(Device, Offset + EIP217_REG_OPTIONS2, &RegVal);

    if (!rc)
    {
        *DW_BitCount_p  = (uint8_t)((RegVal >> 16) & MASK_8_BITS);
        *Counters_p     = (uint16_t)((RegVal)      & MASK_14_BITS);
    }

    return rc;
}


static inline int
EIP217_COUNT_SUMMARY_P_WR(
        Device_Handle_t Device,
        const unsigned int Offset,
        const unsigned int StartOffset,
        const uint32_t * Summary_p,
        const unsigned int Count,
        const unsigned int MaskBits)
{
    unsigned int i;
    int rc = 0;

    for (i=0; i<Count; i++)
    {
        rc = EIP217_Write32(Device,
                            Offset + EIP217_REG_COUNT_SUMMARY_P(StartOffset+i,MaskBits),
                            Summary_p[i]);
        if (rc) return rc;
    }

    return 0;
}


static inline int
EIP217_COUNT_SUMMARY_P_RD(
        Device_Handle_t Device,
        const unsigned int Offset,
        const unsigned int StartOffset,
        uint32_t * Summary_p,
        const unsigned int Count,
        const unsigned int MaskBits)
{
    unsigned int i;
    int rc = 0;

    for (i=0; i<Count; i++)
    {
        rc = EIP217_Read32(Device,
                           Offset + EIP217_REG_COUNT_SUMMARY_P(StartOffset+i, MaskBits),
                           &Summary_p[i]);

        if (rc) return rc;
    }

    return 0;
}


static inline int
EIP217_COUNTER_64_WR(
        Device_Handle_t Device,
        unsigned int Offset,
        unsigned int Index,
        uint32_t CountLo,
        uint32_t CountHi)
{
    int rc = 0;

    rc = EIP217_Write32(Device,
                        Offset + EIP217_REG_COUNTER_LO(Index),
                        CountLo);

    if (rc) return rc;

    return EIP217_Write32(Device,
                          Offset + EIP217_REG_COUNTER_HI(Index),
                          CountHi);
}


static inline int
EIP217_COUNTER_64_RD(
        Device_Handle_t Device,
        unsigned int Offset,
        unsigned int Index,
        uint32_t * const CountLo_p,
        uint32_t * const CountHi_p)
{
    int rc = EIP217_Read32(Device, Offset + EIP217_REG_COUNTER_LO(Index), CountLo_p);

    if(rc) return rc;

    return EIP217_Read32(Device, Offset + EIP217_REG_COUNTER_HI(Index), CountHi_p);
}


static inline int
EIP217_COUNTER_64_DEFAULT_WR(
        Device_Handle_t Device,
        unsigned int Offset,
        unsigned int Index)
{
    int rc = 0;

    rc = EIP217_Write32(Device,
                        Offset + EIP217_REG_COUNTER_LO(Index),
                        EIP217_REG_COUNTER_LO_DEFAULT);

    if (rc) return rc;

    return EIP217_Write32(Device,
                          Offset + EIP217_REG_COUNTER_HI(Index),
                          EIP217_REG_COUNTER_HI_DEFAULT);
}


static inline int
EIP217_COUNT_CONTROL_DEFAULT_WR(
        Device_Handle_t Device,
        unsigned int Offset)
{
    return EIP217_Write32(Device,
                          Offset + EIP217_REG_COUNT_CONTROL,
                          EIP217_REG_COUNT_CONTROL_DEFAULT);
}


static inline int
EIP217_COUNT_CONTROL_WR(
        Device_Handle_t Device,
        unsigned int Offset,
        bool fResetAll,
        bool fSaturateCtrs,
        bool fAutoCtrReset,
        bool fResetSummary,
        bool fIgnoreECC)
{
    uint32_t RegVal = EIP217_REG_COUNT_CONTROL_DEFAULT;

    if(fResetAll)
        RegVal |= BIT_0;
    else
        RegVal &= ~BIT_0;

    if(fSaturateCtrs)
        RegVal |= BIT_2;
    else
        RegVal &= ~BIT_2;

    if(fAutoCtrReset)
        RegVal |= BIT_3;
    else
        RegVal &= ~BIT_3;

    if(fResetSummary)
        RegVal |= BIT_4;
    else
        RegVal &= ~BIT_4;

    if(fIgnoreECC)
        RegVal |= BIT_5;
    else
        RegVal &= ~BIT_5;


    return EIP217_Write32(Device, Offset + EIP217_REG_COUNT_CONTROL, RegVal);
}


static inline int
EIP217_COUNT_CONTROL_RD(
        Device_Handle_t Device,
        unsigned int Offset,
        bool * const fResetAll,
        bool * const fSaturateCtrs,
        bool * const fAutoCtrReset,
        bool * const fResetSummary,
        bool * const fIgnoreECC)
{
    uint32_t RegVal = 0;
    int rc = EIP217_Read32(Device, Offset + EIP217_REG_COUNT_CONTROL, &RegVal);

    if (!rc)
    {
        *fResetAll     = ((RegVal & BIT_0) != 0);
        *fSaturateCtrs = ((RegVal & BIT_2) != 0);
        *fAutoCtrReset = ((RegVal & BIT_3) != 0);
        *fResetSummary = ((RegVal & BIT_4) != 0);
        *fIgnoreECC    = ((RegVal & BIT_5) != 0);
    }

    return rc;
}


static inline int
EIP217_COUNT_INCEN1_WR(
        Device_Handle_t Device,
        unsigned int Offset,
        const uint32_t Mask)
{
    return EIP217_Write32(Device, Offset + EIP217_REG_COUNT_INCEN1, Mask);
}


static inline int
EIP217_COUNT_INCEN1_RD(
        Device_Handle_t Device,
        unsigned int Offset,
        uint32_t * const Mask)
{
    return EIP217_Read32(Device, Offset + EIP217_REG_COUNT_INCEN1, Mask);
}


static inline int
EIP217_COUNT_INCEN2_WR(
        Device_Handle_t Device,
        unsigned int Offset,
        const uint32_t Mask)
{
    return EIP217_Write32(Device, Offset + EIP217_REG_COUNT_INCEN2, Mask);
}


static inline int
EIP217_COUNT_PACK_TIMING_WR(
        Device_Handle_t Device,
        unsigned int Offset,
        uint8_t PackTimeout,
        bool fEnable)
{
    uint32_t RegVal = 0;
    RegVal = PackTimeout;

    if (fEnable)
        RegVal |= BIT_31;

    return EIP217_Write32(Device, Offset + EIP217_REG_COUNT_PACK_TIMING, RegVal);
}

static inline int
EIP217_COUNT_FRAME_THR_64_DEFAULT_WR(
        Device_Handle_t Device,
        unsigned int Offset)
{
    int rc = 0;

    rc = EIP217_Write32(Device,
                        Offset + EIP217_REG_COUNT_FRAME_THR1,
                        EIP217_REG_COUNT_FRAME_THR1_DEFAULT);
    if (rc) return rc;

    return EIP217_Write32(Device,
                          Offset + EIP217_REG_COUNT_FRAME_THR2,
                          EIP217_REG_COUNT_FRAME_THR2_DEFAULT);
}


static inline int
EIP217_COUNT_FRAME_THR_64_WR(
        Device_Handle_t Device,
        unsigned int Offset,
        const uint32_t CountFrameThr_Lo,
        const uint32_t CountFrameThr_Hi)
{
    int rc = 0;

    rc = EIP217_Write32(Device,
                        Offset + EIP217_REG_COUNT_FRAME_THR1,
                        CountFrameThr_Lo);
    if (rc) return rc;

    return EIP217_Write32(Device,
                          Offset + EIP217_REG_COUNT_FRAME_THR2,
                          CountFrameThr_Hi);
}


static inline int
EIP217_COUNT_FRAME_THR_64_RD(
        Device_Handle_t Device,
        unsigned int Offset,
        uint32_t * const CountFrameThr_Lo,
        uint32_t * const CountFrameThr_Hi)
{
    int rc = 0;

    rc = EIP217_Read32(Device,
                       Offset + EIP217_REG_COUNT_FRAME_THR1,
                       CountFrameThr_Lo);
    if (rc) return rc;

    return EIP217_Read32(Device,
                         Offset + EIP217_REG_COUNT_FRAME_THR2,
                         CountFrameThr_Hi);
}


static inline int
EIP217_COUNT_OCTET_THR_64_WR(
        Device_Handle_t Device,
        unsigned int Offset,
        const uint32_t CountOctetThr_Lo,
        const uint32_t CountOctetThr_Hi)
{
    int rc = 0;

    rc = EIP217_Write32(Device,
                        Offset + EIP217_REG_COUNT_OCTET_THR1,
                        CountOctetThr_Lo);
    if (rc) return rc;

    return EIP217_Write32(Device,
                          Offset + EIP217_REG_COUNT_OCTET_THR2,
                          CountOctetThr_Hi);
}


static inline int
EIP217_COUNT_OCTET_THR_64_RD(
        Device_Handle_t Device,
        unsigned int Offset,
        uint32_t * const  CountOctetThr_Lo,
        uint32_t * const CountOctetThr_Hi)
{
    int rc = 0;

    rc = EIP217_Read32(Device,
                       Offset + EIP217_REG_COUNT_OCTET_THR1,
                       CountOctetThr_Lo);
    if (rc) return rc;

    return EIP217_Read32(Device,
                         Offset + EIP217_REG_COUNT_OCTET_THR2,
                         CountOctetThr_Hi);
}


static inline int
EIP217_COUNT_POSTED_CTRL_WR(
        Device_Handle_t Device,
        const unsigned int Offset,
        const bool fEnable,
        const bool fWrite,
        const uint16_t Index,
        const uint16_t Mask,
        const bool fAllCounters)
{
    uint32_t RegVal = 0;

    RegVal = ((Index & MASK_10_BITS) << 3) | ((Mask & MASK_15_BITS) << 16);

    if (fEnable)
        RegVal |= BIT_0;

    if (fWrite)
        RegVal |= BIT_1;

    if (fAllCounters)
        RegVal |= BIT_31;

    return EIP217_Write32(Device,
                          Offset + EIP217_REG_COUNT_POSTED_CTRL,
                          RegVal);
}

static inline int
EIP217_COUNT_POSTED_CTRL_RD(
        Device_Handle_t Device,
        const unsigned int Offset,
        bool * const fEnable,
        bool * const fWrite,
        uint16_t * const Index,
        uint16_t * const Mask,
        bool * const fAllCounters)
{
    uint32_t RegVal = 0;
    int rc = 0;

    rc = EIP217_Read32(Device,
                       Offset + EIP217_REG_COUNT_POSTED_CTRL,
                       &RegVal);

    if (!rc)
    {
        *Index = (RegVal >> 3) & MASK_10_BITS;
        *Mask = (RegVal >> 16) & MASK_15_BITS;
        *fEnable = (RegVal & BIT_0) != 0;
        *fWrite = (RegVal & BIT_1) != 0;
        *fAllCounters = (RegVal & BIT_31) != 0;
    }

    return rc;
}


static inline int
EIP217_COUNT_SUMMARY_WR(
        Device_Handle_t Device,
        const unsigned int Offset,
        const unsigned int StartOffset,
        const uint32_t * Summary_p,
        const unsigned int Count,
        const unsigned int MaskBits)
{
    unsigned int i;
    int rc = 0;

    for (i=0; i<Count; i++)
    {
        rc = EIP217_Write32(Device,
                            Offset + EIP217_REG_COUNT_SUMMARY(StartOffset+i,MaskBits),
                            Summary_p[i]);
        if (rc) return rc;
    }

    return 0;
}


static inline int
EIP217_COUNT_SUMMARY_RD(
        Device_Handle_t Device,
        const unsigned int Offset,
        const unsigned int StartOffset,
        uint32_t * Summary_p,
        const unsigned int Count,
        const unsigned int MaskBits)
{
    unsigned int i;
    int rc = 0;

    for (i=0; i<Count; i++)
    {
        rc = EIP217_Read32(Device,
                          Offset + EIP217_REG_COUNT_SUMMARY(StartOffset+i, MaskBits),
                          &Summary_p[i]);
        if (rc) return rc;
    }

    return 0;
}


static inline int
EIP217_COUNT_SUMMARY_GL_WR(
        Device_Handle_t Device,
        const unsigned int Offset,
        const uint32_t Summary)
{
    return EIP217_Write32(Device,
                          Offset + EIP217_REG_COUNT_SUMMARY_GL,
                          Summary);
}


static inline int
EIP217_COUNT_SUMMARY_GL_RD(
        Device_Handle_t Device,
        const unsigned int Offset,
        uint32_t * Summary_p)
{
    return EIP217_Read32(Device,
                         Offset + EIP217_REG_COUNT_SUMMARY_GL,
                         Summary_p);
}


static inline int
EIP217_COUNT_SUMMARY_CHAN_WR(
        Device_Handle_t Device,
        const unsigned int Offset,
        const unsigned int StartOffset,
        const uint32_t * Summary_p,
        const unsigned int Count)
{
    return EIP217_Write32Array(Device,
                               Offset +
                               EIP217_REG_COUNT_SUMMARY_CHAN1 +
                               EIP217_REG_COUNT_SUMMARY(StartOffset,1),
                               Summary_p,
                               Count);
}


static inline int
EIP217_COUNT_SUMMARY_CHAN_RD(
        Device_Handle_t Device,
        const unsigned int Offset,
        const unsigned int StartOffset,
        uint32_t * Summary_p,
        const unsigned int Count)
{
    return EIP217_Read32Array(Device,
                              Offset +
                              EIP217_REG_COUNT_SUMMARY_CHAN1 +
                              EIP217_REG_COUNT_SUMMARY(StartOffset,1),
                              Summary_p,
                              Count);
}


#endif /* EIP217_LEVEL0_H_ */


/* end of file eip217_level0.h */
