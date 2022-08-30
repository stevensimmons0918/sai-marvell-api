/* eip164_level0.h
 *
 * EIP-164 Engine Level0 Internal interface
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

#ifndef EIP164_LEVEL0_H_
#define EIP164_LEVEL0_H_


/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

/* Default configuration */
#include <Kit/EIP164/incl/c_eip164.h>

/* Driver Framework Basic Definitions API */
#include <Kit/DriverFramework/incl/basic_defs.h>         /* BIT definitions, bool, uint32_t */

/* Driver Framework Device API */
#include <Kit/DriverFramework/incl/device_types.h>       /* Device_Handle_t */
#include <Kit/DriverFramework/incl/device_rw.h>          /* Read32, Write32 */

/* EIP-164 Hardware Register Internal interface */
#include <Kit/EIP164/incl/eip164_hw_interface.h> /* EIP164_REG_* */

/* EIP-217 Level0 interface */
#include <Kit/EIP217/src/eip217_level0.h>


/*----------------------------------------------------------------------------
 * Definitions and macros
 */

typedef struct
{
    /* Flow type */
    /* true - bypass */
    /* false - perform drop action */
    bool fBypass;

    bool fDropNonReserved;
    uint8_t DestPort;
    uint8_t DropAction;
} EIP164_Level0_NonSA_Rules_t;


/*----------------------------------------------------------------------------
 * EIP164_Swap16
 *
 * Swaps 2 bytes in a 2-byte word
 */
static inline uint16_t
EIP164_Swap16(
        const uint16_t Value16)
{
    return ((Value16 & MASK_8_BITS) << 8) | ((Value16 >> 8) & MASK_8_BITS);
}


/*----------------------------------------------------------------------------
 * EIP164_Read32
 *
 * This routine reads a 32-bit word from a Register location at Offset
 * in the EIP-164.
 */
static inline int
EIP164_Read32(
        Device_Handle_t Device,
        const unsigned int Offset,
        uint32_t * const Value_p)
{
    return Device_Read32Check(Device, Offset, Value_p);
}


/*----------------------------------------------------------------------------
 * EIP164_Write32
 *
 * This routine writes to a Register location in the EIP-164.
 */
static inline int
EIP164_Write32(
        Device_Handle_t Device,
        const unsigned int Offset,
        const uint32_t Value)
{
    return  Device_Write32(Device, Offset, Value);
}


/*----------------------------------------------------------------------------
 * EIP164_Read32Array
 *
 * This routine reads MemoryDst_p array of Count 32-bit words from a Register
 * location in the EIP-164 at Offset.
 */
static inline int
EIP164_Read32Array(
        Device_Handle_t Device,
        const unsigned int Offset,
        uint32_t * MemoryDst_p,
        const int Count)
{
    return Device_Read32Array(Device, Offset, MemoryDst_p, Count);
}


/*----------------------------------------------------------------------------
 * EIP164_Write32Array
 *
 * This routine writes MemorySrc_p array of Count 32-bit words to a Register
 * location at Offset in the EIP-164.
 */
static inline int
EIP164_Write32Array(
        Device_Handle_t Device,
        const unsigned int Offset,
        const uint32_t * MemorySrc_p,
        const int Count)
{
    return Device_Write32Array(Device, Offset, MemorySrc_p, Count);
}


static inline bool
EIP164_REV_SIGNATURE_MATCH(
        const uint32_t Rev)
{
    return (((uint16_t)Rev) == EIP164_SIGNATURE);
}


static inline int
EIP164_EIP_REV_RD(
        Device_Handle_t Device,
        uint8_t * const EipNumber,
        uint8_t * const ComplmtEipNumber,
        uint8_t * const HWPatchLevel,
        uint8_t * const MinHWRevision,
        uint8_t * const MajHWRevision)
{
    uint32_t RegVal = 0;
    int rc = 0;

    rc = EIP164_Read32(Device, EIP164_REG_VERSION, &RegVal);

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
EIP164_CONFIG_RD(
        Device_Handle_t Device,
        uint16_t * const SA_Count,
        uint16_t * const vPort_Count,
        bool * const fIngressOnly,
        bool * const fEgressOnly)
{
    uint32_t RegVal = 0;
    int rc = 0;

    rc = EIP164_Read32(Device, EIP164_REG_CONFIG, &RegVal);

    if (!rc)
    {
        *SA_Count     = (uint16_t)((RegVal >> 16) & MASK_14_BITS);
        *vPort_Count  = (uint16_t)((RegVal >> 0) & MASK_14_BITS);
        *fIngressOnly = ((RegVal & BIT_30) != 0);
        *fEgressOnly  = ((RegVal & BIT_31) != 0);
    }

    return rc;
}


static inline int
EIP164_CONFIG2_RD(
        Device_Handle_t Device,
        uint16_t * const SC_Count,
        uint8_t * const Channel_Count,
        bool * const fSpareReg,
        bool * const fExternalRxCAM)
{
    uint32_t RegVal = 0;
    int rc = 0;

    rc = EIP164_Read32(Device, EIP164_REG_CONFIG2, &RegVal);

    if (!rc)
    {
        *fExternalRxCAM = ((RegVal & BIT_22) != 0);
        *fSpareReg      = ((RegVal & BIT_23) != 0);
        *SC_Count      = (uint16_t)(RegVal & MASK_14_BITS);
        *Channel_Count = (uint8_t)((RegVal >> 24) & MASK_8_BITS);
    }

    return rc;
}


static inline int
EIP164_DBG_PARSED_DA_RD(
        Device_Handle_t Device,
        uint32_t * const Lo_p,
        uint32_t * const Hi_p)
{
    int rc = 0;

    rc = EIP164_Read32(Device, EIP164_REG_DBG_PARSED_DA_LO, Lo_p);
    if (rc) return rc;

    return EIP164_Read32(Device, EIP164_REG_DBG_PARSED_DA_HI, Hi_p);
}


static inline int
EIP164_DBG_PARSED_SA_RD(
        Device_Handle_t Device,
        uint32_t * const Lo_p,
        uint32_t * const Hi_p)
{
    int rc = 0;

    rc = EIP164_Read32(Device, EIP164_REG_DBG_PARSED_SA_LO, Lo_p);
    if (rc) return rc;

    return EIP164_Read32(Device, EIP164_REG_DBG_PARSED_SA_HI, Hi_p);
}


static inline int
EIP164_DBG_PARSED_SECTAG_RD(
        Device_Handle_t Device,
        uint32_t * const Lo_p,
        uint32_t * const Hi_p)
{
    int rc = 0;

    rc = EIP164_Read32(Device, EIP164_REG_DBG_PARSED_SECTAG_LO, Lo_p);
    if (rc) return rc;

    return EIP164_Read32(Device, EIP164_REG_DBG_PARSED_SECTAG_HI, Hi_p);
}


static inline int
EIP164_DBG_PARSED_SCI_RD(
        Device_Handle_t Device,
        uint32_t * const Lo_p,
        uint32_t * const Hi_p)
{
    int rc = 0;

    rc = EIP164_Read32(Device, EIP164_REG_DBG_PARSED_SCI_LO, Lo_p);
    if (rc) return rc;

    return EIP164_Read32(Device, EIP164_REG_DBG_PARSED_SCI_HI, Hi_p);
}


static inline int
EIP164_SECTAG_DEBUG_RD(
        Device_Handle_t Device,
        uint32_t * const Value_p)
{
    return EIP164_Read32(Device, EIP164_REG_SECTAG_DEBUG, Value_p);
}


static inline int
EIP164_PARSER_IN_DEBUG_RD(
        Device_Handle_t Device,
        uint32_t * const Value_p)
{
    return EIP164_Read32(Device, EIP164_REG_PARSER_IN_DEBUG, Value_p);
}


static inline int
EIP164_DBG_RXCAM_SCI_RD(
        Device_Handle_t Device,
        uint32_t * const Lo_p,
        uint32_t * const Hi_p)
{
    int rc = 0;

    rc = EIP164_Read32(Device, EIP164_REG_DBG_RXCAM_SCI_LO, Lo_p);
    if (rc) return rc;

    return EIP164_Read32(Device, EIP164_REG_DBG_RXCAM_SCI_HI, Hi_p);
}


static inline int
EIP164_SC_SA_MAP1_WR(
        Device_Handle_t Device,
        const unsigned int SC_Index,
        const unsigned int SA_Index0,
        const bool fSA_InUse0,
        const bool fSA_IndexUpdate0,
        const unsigned int SA_Index1,
        const bool fSA_InUse1,
        const bool fSA_IndexUpdate1)
{
    uint32_t RegVal = EIP164_REG_SC_SA_MAP1_DEFAULT;

    if (fSA_IndexUpdate0)
        RegVal |= BIT_13;
    else
        RegVal &= ~BIT_13;

    if (fSA_InUse0)
        RegVal |= BIT_15;
    else
        RegVal &= ~BIT_15;

    if (fSA_IndexUpdate1)
        RegVal |= BIT_29;
    else
        RegVal &= ~BIT_29;

    if (fSA_InUse1)
        RegVal |= BIT_31;
    else
        RegVal &= ~BIT_31;


    RegVal |= (SA_Index0 & MASK_13_BITS);
    RegVal |= (SA_Index1 & MASK_13_BITS) << 16;

    return EIP164_Write32(Device, EIP164_REG_SC_SA_MAP1(SC_Index), RegVal);
}


static inline int
EIP164_SC_SA_MAP1_RD(
        Device_Handle_t Device,
        const unsigned int SC_Index,
        unsigned int *SA_Index0,
        bool *fSA_InUse0,
        unsigned int *SA_Index1,
        bool *fSA_InUse1)
{
    uint32_t RegVal = 0;
    int rc = 0;

    rc = EIP164_Read32(Device, EIP164_REG_SC_SA_MAP1(SC_Index), &RegVal);

    if (!rc)
    {
        *SA_Index0  = RegVal & MASK_13_BITS;
        *fSA_InUse0 = (RegVal >> 15) & MASK_1_BIT;
        *SA_Index1  = (RegVal >> 16) & MASK_13_BITS;
        *fSA_InUse1 = (RegVal >> 31) & MASK_1_BIT;
    }

    return rc;
}


static inline int
EIP164_SC_SA_MAP2_WR(
        Device_Handle_t Device,
        const unsigned int SC_Index,
        const unsigned int SA_Index2,
        const bool fSA_InUse2,
        const bool fSA_IndexUpdate2,
        const unsigned int SA_Index3,
        const bool fSA_InUse3,
        const bool fSA_IndexUpdate3)
{
    uint32_t RegVal = EIP164_REG_SC_SA_MAP2_DEFAULT;

    if (fSA_IndexUpdate2)
        RegVal |= BIT_13;
    else
        RegVal &= ~BIT_13;

    if (fSA_InUse2)
        RegVal |= BIT_15;
    else
        RegVal &= ~BIT_15;

    if (fSA_IndexUpdate3)
        RegVal |= BIT_29;
    else
        RegVal &= ~BIT_29;

    if (fSA_InUse3)
        RegVal |= BIT_31;
    else
        RegVal &= ~BIT_31;

    RegVal |= (SA_Index2 & MASK_13_BITS);
    RegVal |= (SA_Index3 & MASK_13_BITS) << 16;

    return EIP164_Write32(Device, EIP164_REG_SC_SA_MAP2(SC_Index), RegVal);
}


static inline int
EIP164_SC_SA_MAP2_RD(
        Device_Handle_t Device,
        const unsigned int SC_Index,
        unsigned int *SA_Index2,
        bool *fSA_InUse2,
        unsigned int *SA_Index3,
        bool *fSA_InUse3)
{
    uint32_t RegVal = 0;
    int rc = 0;

    rc = EIP164_Read32(Device, EIP164_REG_SC_SA_MAP2(SC_Index), &RegVal);

    if (!rc)
    {
        *SA_Index2  = RegVal & MASK_13_BITS;
        *fSA_InUse2 = (RegVal >> 15) & MASK_1_BIT;
        *SA_Index3  = (RegVal >> 16) & MASK_13_BITS;
        *fSA_InUse3 = (RegVal >> 31) & MASK_1_BIT;
    }

    return rc;
}


static inline int
EIP164_RXSC_CAM_SCI_LO_WR(
        Device_Handle_t Device,
        const unsigned int SC_Index,
        const uint32_t Value)
{
    return EIP164_Write32(Device, EIP164_REG_RXSC_CAM_SCI_LO(SC_Index), Value);
}


static inline int
EIP164_RXSC_CAM_SCI_HI_WR(
        Device_Handle_t Device,
        const unsigned int SC_Index,
        const uint32_t Value)
{
    return EIP164_Write32(Device, EIP164_REG_RXSC_CAM_SCI_HI(SC_Index), Value);
}


static inline int
EIP164_RXSC_CAM_CTRL_WR(
        Device_Handle_t Device,
        const unsigned int SC_Index,
        const uint32_t Value)
{
    return EIP164_Write32(Device, EIP164_REG_RXSC_CAM_CTRL(SC_Index), Value);
}

static inline int
EIP164_RXSC_CAM_SCI_LO_RD(
        Device_Handle_t Device,
        const unsigned int SC_Index,
        uint32_t * const Value_p)
{
    return EIP164_Read32(Device, EIP164_REG_RXSC_CAM_SCI_LO(SC_Index), Value_p);
}


static inline int
EIP164_RXSC_CAM_SCI_HI_RD(
        Device_Handle_t Device,
        const unsigned int SC_Index,
        uint32_t * const Value_p)
{
    return EIP164_Read32(Device, EIP164_REG_RXSC_CAM_SCI_HI(SC_Index), Value_p);
}

static inline int
EIP164_RXSC_CAM_CTRL_RD(
        Device_Handle_t Device,
        const unsigned int SC_Index,
        uint32_t * const Value_p)
{
    return EIP164_Read32(Device, EIP164_REG_RXSC_CAM_CTRL(SC_Index), Value_p);
}


static inline int
EIP164_RXSC_ENTRY_ENABLE_CTRL_WR(
        Device_Handle_t Device,
        const unsigned int SC_IndexSet,
        const bool fSetEnable,
        const bool fSetAll,
        const unsigned int SC_ClearIndex,
        const bool fClearEnable,
        const bool fClearAll)
{
    uint32_t RegVal = EIP164_REG_RXSC_ENTRY_ENABLE_CTRL_DEFAULT;

    if (fSetEnable)
        RegVal |= BIT_14;
    else
        RegVal &= ~BIT_14;

    if (fSetAll)
        RegVal |= BIT_15;
    else
        RegVal &= ~BIT_15;

    if (fClearEnable)
        RegVal |= BIT_30;
    else
        RegVal &= ~BIT_30;

    if (fClearAll)
        RegVal |= BIT_31;
    else
        RegVal &= ~BIT_31;

    RegVal |= (SC_IndexSet   & MASK_13_BITS);
    RegVal |= (SC_ClearIndex & MASK_13_BITS) << 16;

    return EIP164_Write32(Device, EIP164_REG_RXSC_ENTRY_ENABLE_CTRL, RegVal);
}

static inline int
EIP164_RXSC_ENTRY_ENABLE_RD(
        Device_Handle_t Device,
        const unsigned int SC_Index,
        bool * const fEnabled)
{
    unsigned int RegIndex = SC_Index / 32 + 1;
    unsigned int RegMask = 1 << (SC_Index % 32);
    uint32_t RegVal = 0;
    int rc = 0;

    rc = EIP164_Read32(Device,
                       EIP164_REG_RXSC_ENTRY_ENABLE(RegIndex),
                       &RegVal);

   if (!rc)
   {
      *fEnabled = (RegVal & RegMask) != 0;
   }

    return rc;
}


static inline int
EIP164_RXSC_CAM_STATUS_RD(
        Device_Handle_t Device,
        bool * const fWriteBusy,
        bool * const fReadBusy,
        bool * const fDisableBusy,
        bool * const fEnabled)
{
    uint32_t RegVal = 0;
    int rc = 0;

    rc = EIP164_Read32(Device, EIP164_REG_RXSC_STATUS, &RegVal);

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
EIP164_SA_EXP_SUMMARY_WR(
        Device_Handle_t Device,
        const unsigned int Index,
        const uint32_t Bits_Clear)
{
    return EIP164_Write32(Device, EIP164_REG_SA_EXP_SUMMARY(Index), Bits_Clear);
}


static inline int
EIP164_SA_EXP_SUMMARY_RD(
        Device_Handle_t Device,
        const unsigned int Index,
        uint32_t * const Bits_Read)
{
    return EIP164_Read32(Device, EIP164_REG_SA_EXP_SUMMARY(Index), Bits_Read);
}


static inline int
EIP164_SA_PN_THR_SUMMARY_WR(
        Device_Handle_t Device,
        const unsigned int Index,
        const uint32_t Bits_Clear)
{
    return EIP164_Write32(Device,
                          EIP164_REG_SA_PN_THR_SUMMARY(Index),
                          Bits_Clear);
}


static inline int
EIP164_SA_PN_THR_SUMMARY_RD(
        Device_Handle_t Device,
        const unsigned int Index,
        uint32_t * const Bits_Read)
{
    return EIP164_Read32(Device, EIP164_REG_SA_PN_THR_SUMMARY(Index), Bits_Read);
}


static inline int
EIP164_LATENCY_CTRL_WR(
        Device_Handle_t Device,
        uint16_t Latency)
{
    uint32_t RegVal = EIP164_REG_LATENCY_CTRL_DEFAULT;

    RegVal &= ~MASK_16_BITS;
    RegVal |= (uint32_t)((((uint32_t)Latency) & MASK_16_BITS));

    return EIP164_Write32(Device, EIP164_REG_LATENCY_CTRL, RegVal);
}


static inline int
EIP164_SA_COUNT_CONTROL_WR(
        Device_Handle_t Device,
        bool fResetAll,
        bool fSaturateCtrs,
        bool fAutoCtrReset,
        bool fResetSummary)
{
    return EIP217_COUNT_CONTROL_WR(Device,
                                   EIP164_REG_SA_STATISTICS_CONTROLS,
                                   fResetAll,
                                   fSaturateCtrs,
                                   fAutoCtrReset,
                                   fResetSummary,
                                   false);
}


static inline int
EIP164_SECY_COUNT_CONTROL_WR(
        Device_Handle_t Device,
        bool fResetAll,
        bool fSaturateCtrs,
        bool fAutoCtrReset,
        bool fResetSummary)
{
    return EIP217_COUNT_CONTROL_WR(Device,
                                   EIP164_REG_SECY_STATISTICS_CONTROLS,
                                   fResetAll,
                                   fSaturateCtrs,
                                   fAutoCtrReset,
                                   fResetSummary,
                                   false);
}


static inline int
EIP164_IFC_COUNT_CONTROL_WR(
        Device_Handle_t Device,
        bool fResetAll,
        bool fSaturateCtrs,
        bool fAutoCtrReset,
        bool fResetSummary)
{
    return EIP217_COUNT_CONTROL_WR(Device,
                                   EIP164_REG_IFC_STATISTICS_CONTROLS,
                                   fResetAll,
                                   fSaturateCtrs,
                                   fAutoCtrReset,
                                   fResetSummary,
                                   false);
}


static inline int
EIP164_IFC1_COUNT_CONTROL_WR(
        Device_Handle_t Device,
        bool fResetAll,
        bool fSaturateCtrs,
        bool fAutoCtrReset,
        bool fResetSummary)
{
    return EIP217_COUNT_CONTROL_WR(Device,
                                   EIP164_REG_IFC1_STATISTICS_CONTROLS,
                                   fResetAll,
                                   fSaturateCtrs,
                                   fAutoCtrReset,
                                   fResetSummary,
                                   false);
}


static inline int
EIP164_RXCAM_COUNT_CONTROL_WR(
        Device_Handle_t Device,
        bool fResetAll,
        bool fSaturateCtrs,
        bool fAutoCtrReset,
        bool fResetSummary)
{
    return EIP217_COUNT_CONTROL_WR(Device,
                                   EIP164_REG_RXCAM_STATISTICS_CONTROLS,
                                   fResetAll,
                                   fSaturateCtrs,
                                   fAutoCtrReset,
                                   fResetSummary,
                                   false);
}


static inline int
EIP164_SA_COUNT_CONTROL_RD(
        Device_Handle_t Device,
        bool * const fResetAll,
        bool * const fSaturateCtrs,
        bool * const fAutoCtrReset,
        bool * const fResetSummary)
{
    bool dummy;
    return EIP217_COUNT_CONTROL_RD(Device,
                                   EIP164_REG_SA_STATISTICS_CONTROLS,
                                   fResetAll,
                                   fSaturateCtrs,
                                   fAutoCtrReset,
                                   fResetSummary,
                                   &dummy);
}


static inline int
EIP164_SECY_COUNT_CONTROL_RD(
        Device_Handle_t Device,
        bool * const fResetAll,
        bool * const fSaturateCtrs,
        bool * const fAutoCtrReset,
        bool * const fResetSummary)
{
    bool dummy;
    return EIP217_COUNT_CONTROL_RD(Device,
                                   EIP164_REG_SECY_STATISTICS_CONTROLS,
                                   fResetAll,
                                   fSaturateCtrs,
                                   fAutoCtrReset,
                                   fResetSummary,
                                   &dummy);
}


static inline int
EIP164_IFC_COUNT_CONTROL_RD(
        Device_Handle_t Device,
        bool * const fResetAll,
        bool * const fSaturateCtrs,
        bool * const fAutoCtrReset,
        bool * const fResetSummary)
{
    bool dummy;
    return EIP217_COUNT_CONTROL_RD(Device,
                                   EIP164_REG_IFC_STATISTICS_CONTROLS,
                                   fResetAll,
                                   fSaturateCtrs,
                                   fAutoCtrReset,
                                   fResetSummary,
                                   &dummy);
}


static inline int
EIP164_IFC1_COUNT_CONTROL_RD(
        Device_Handle_t Device,
        bool * const fResetAll,
        bool * const fSaturateCtrs,
        bool * const fAutoCtrReset,
        bool * const fResetSummary)
{
    bool dummy;
    return EIP217_COUNT_CONTROL_RD(Device,
                                   EIP164_REG_IFC1_STATISTICS_CONTROLS,
                                   fResetAll,
                                   fSaturateCtrs,
                                   fAutoCtrReset,
                                   fResetSummary,
                                   &dummy);
}


static inline int
EIP164_RXCAM_COUNT_CONTROL_RD(
        Device_Handle_t Device,
        bool * const fResetAll,
        bool * const fSaturateCtrs,
        bool * const fAutoCtrReset,
        bool * const fResetSummary)
{
    bool dummy;
    return EIP217_COUNT_CONTROL_RD(Device,
                                   EIP164_REG_RXCAM_STATISTICS_CONTROLS,
                                   fResetAll,
                                   fSaturateCtrs,
                                   fAutoCtrReset,
                                   fResetSummary,
                                   &dummy);
}


static inline int
EIP164_SA_COUNT_INCEN1_WR(
        Device_Handle_t Device,
        const uint32_t Mask)
{
    return EIP217_COUNT_INCEN1_WR(Device,
                                  EIP164_REG_SA_STATISTICS_CONTROLS,
                                  Mask);
}


static inline int
EIP164_SA_COUNT_INCEN1_RD(
        Device_Handle_t Device,
        uint32_t * const Mask)
{
    return EIP217_COUNT_INCEN1_RD(Device,
                                  EIP164_REG_SA_STATISTICS_CONTROLS,
                                  Mask);
}


static inline int
EIP164_SECY_COUNT_INCEN1_WR(
        Device_Handle_t Device,
        const uint32_t Mask)
{
    return EIP217_COUNT_INCEN1_WR(Device,
                                  EIP164_REG_SECY_STATISTICS_CONTROLS,
                                  Mask);
}


static inline int
EIP164_SECY_COUNT_INCEN1_RD(
        Device_Handle_t Device,
        uint32_t * const Mask)
{
    return EIP217_COUNT_INCEN1_RD(Device,
                                  EIP164_REG_SECY_STATISTICS_CONTROLS,
                                  Mask);
}


static inline int
EIP164_IFC_COUNT_INCEN1_WR(
        Device_Handle_t Device,
        const uint32_t Mask)
{
    return EIP217_COUNT_INCEN1_WR(Device,
                                  EIP164_REG_IFC_STATISTICS_CONTROLS,
                                  Mask);
}


static inline int
EIP164_IFC_COUNT_INCEN1_RD(
        Device_Handle_t Device,
        uint32_t * const Mask)
{
    return EIP217_COUNT_INCEN1_RD(Device, EIP164_REG_IFC_STATISTICS_CONTROLS, Mask);
}


static inline int
EIP164_IFC1_COUNT_INCEN1_WR(
        Device_Handle_t Device,
        const uint32_t Mask)
{
    return EIP217_COUNT_INCEN1_WR(Device, EIP164_REG_IFC1_STATISTICS_CONTROLS, Mask);
}


static inline int
EIP164_IFC1_COUNT_INCEN1_RD(
        Device_Handle_t Device,
        uint32_t * const Mask)
{
    return EIP217_COUNT_INCEN1_RD(Device, EIP164_REG_IFC1_STATISTICS_CONTROLS, Mask);
}


static inline int
EIP164_RXCAM_COUNT_INCEN1_WR(
        Device_Handle_t Device,
        const uint32_t Mask)
{
    return EIP217_COUNT_INCEN1_WR(Device, EIP164_REG_RXCAM_STATISTICS_CONTROLS, Mask);
}


static inline int
EIP164_RXCAM_COUNT_INCEN1_RD(
        Device_Handle_t Device,
        uint32_t * const Mask)
{
    return EIP217_COUNT_INCEN1_RD(Device, EIP164_REG_RXCAM_STATISTICS_CONTROLS, Mask);
}


static inline int
EIP164_SA_COUNT_FRAME_THR_WR(
        Device_Handle_t Device,
        const uint32_t CountFrameThr_Lo,
        const uint32_t CountFrameThr_Hi)
{
    return EIP217_COUNT_FRAME_THR_64_WR(Device,
                                        EIP164_REG_SA_STATISTICS_CONTROLS,
                                        CountFrameThr_Lo,
                                        CountFrameThr_Hi);
}


static inline int
EIP164_SA_COUNT_FRAME_THR_RD(
        Device_Handle_t Device,
        uint32_t * const CountFrameThr_Lo,
        uint32_t * const CountFrameThr_Hi)
{
    return EIP217_COUNT_FRAME_THR_64_RD(Device,
                                        EIP164_REG_SA_STATISTICS_CONTROLS,
                                        CountFrameThr_Lo,
                                        CountFrameThr_Hi);
}


static inline int
EIP164_SECY_COUNT_FRAME_THR_WR(
        Device_Handle_t Device,
        const uint32_t CountFrameThr_Lo,
        const uint32_t CountFrameThr_Hi)
{
    return EIP217_COUNT_FRAME_THR_64_WR(Device,
                                        EIP164_REG_SECY_STATISTICS_CONTROLS,
                                        CountFrameThr_Lo,
                                        CountFrameThr_Hi);
}


static inline int
EIP164_SECY_COUNT_FRAME_THR_RD(
        Device_Handle_t Device,
        uint32_t * const CountFrameThr_Lo,
        uint32_t * const CountFrameThr_Hi)
{
    return EIP217_COUNT_FRAME_THR_64_RD(Device,
                                        EIP164_REG_SECY_STATISTICS_CONTROLS,
                                        CountFrameThr_Lo,
                                        CountFrameThr_Hi);
}


static inline int
EIP164_IFC_COUNT_FRAME_THR_WR(
        Device_Handle_t Device,
        const uint32_t CountFrameThr_Lo,
        const uint32_t CountFrameThr_Hi)
{
    return EIP217_COUNT_FRAME_THR_64_WR(Device,
                                        EIP164_REG_IFC_STATISTICS_CONTROLS,
                                        CountFrameThr_Lo,
                                        CountFrameThr_Hi);
}


static inline int
EIP164_IFC_COUNT_FRAME_THR_RD(
        Device_Handle_t Device,
        uint32_t * const CountFrameThr_Lo,
        uint32_t * const CountFrameThr_Hi)
{
    return EIP217_COUNT_FRAME_THR_64_RD(Device,
                                        EIP164_REG_IFC_STATISTICS_CONTROLS,
                                        CountFrameThr_Lo,
                                        CountFrameThr_Hi);
}


static inline int
EIP164_IFC1_COUNT_FRAME_THR_WR(
        Device_Handle_t Device,
        const uint32_t CountFrameThr_Lo,
        const uint32_t CountFrameThr_Hi)
{
    return EIP217_COUNT_FRAME_THR_64_WR(Device,
                                        EIP164_REG_IFC1_STATISTICS_CONTROLS,
                                        CountFrameThr_Lo,
                                        CountFrameThr_Hi);
}


static inline int
EIP164_IFC1_COUNT_FRAME_THR_RD(
        Device_Handle_t Device,
        uint32_t * const CountFrameThr_Lo,
        uint32_t * const CountFrameThr_Hi)
{
    return EIP217_COUNT_FRAME_THR_64_RD(Device,
                                        EIP164_REG_IFC1_STATISTICS_CONTROLS,
                                        CountFrameThr_Lo,
                                        CountFrameThr_Hi);
}


static inline int
EIP164_RXCAM_COUNT_FRAME_THR_WR(
        Device_Handle_t Device,
        const uint32_t CountFrameThr_Lo,
        const uint32_t CountFrameThr_Hi)
{
    return EIP217_COUNT_FRAME_THR_64_WR(Device,
                                        EIP164_REG_RXCAM_STATISTICS_CONTROLS,
                                        CountFrameThr_Lo,
                                        CountFrameThr_Hi);
}


static inline int
EIP164_RXCAM_COUNT_FRAME_THR_RD(
        Device_Handle_t Device,
        uint32_t * const CountFrameThr_Lo,
        uint32_t * const CountFrameThr_Hi)
{
    return EIP217_COUNT_FRAME_THR_64_RD(Device,
                                         EIP164_REG_RXCAM_STATISTICS_CONTROLS,
                                         CountFrameThr_Lo,
                                         CountFrameThr_Hi);
}


static inline int
EIP164_SA_COUNT_OCTET_THR_WR(
        Device_Handle_t Device,
        const uint32_t CountOctetThr_Lo,
        const uint32_t CountOctetThr_Hi)
{
     return EIP217_COUNT_OCTET_THR_64_WR(Device,
                                         EIP164_REG_SA_STATISTICS_CONTROLS,
                                         CountOctetThr_Lo,
                                         CountOctetThr_Hi);
}


static inline int
EIP164_SA_COUNT_OCTET_THR_RD(
        Device_Handle_t Device,
        uint32_t * const CountOctetThr_Lo,
        uint32_t * const CountOctetThr_Hi)
{
   return EIP217_COUNT_OCTET_THR_64_RD(Device,
                                       EIP164_REG_SA_STATISTICS_CONTROLS,
                                       CountOctetThr_Lo,
                                       CountOctetThr_Hi);
}


static inline int
EIP164_IFC_COUNT_OCTET_THR_WR(
        Device_Handle_t Device,
        const uint32_t CountOctetThr_Lo,
        const uint32_t CountOctetThr_Hi)
{
    return EIP217_COUNT_OCTET_THR_64_WR(Device,
                                        EIP164_REG_IFC_STATISTICS_CONTROLS,
                                        CountOctetThr_Lo,
                                        CountOctetThr_Hi);
       }


static inline int
EIP164_IFC_COUNT_OCTET_THR_RD(
        Device_Handle_t Device,
        uint32_t * const CountOctetThr_Lo,
        uint32_t * const CountOctetThr_Hi)
{
    return EIP217_COUNT_OCTET_THR_64_RD(Device,
                                        EIP164_REG_IFC_STATISTICS_CONTROLS,
                                        CountOctetThr_Lo,
                                        CountOctetThr_Hi);
}


static inline int
EIP164_IFC1_COUNT_OCTET_THR_WR(
        Device_Handle_t Device,
        const uint32_t CountOctetThr_Lo,
        const uint32_t CountOctetThr_Hi)
{
    return EIP217_COUNT_OCTET_THR_64_WR(Device,
                                       EIP164_REG_IFC1_STATISTICS_CONTROLS,
                                       CountOctetThr_Lo,
                                       CountOctetThr_Hi);
}


static inline int
EIP164_IFC1_COUNT_OCTET_THR_RD(
        Device_Handle_t Device,
        uint32_t * const CountOctetThr_Lo,
        uint32_t * const CountOctetThr_Hi)
{
    return EIP217_COUNT_OCTET_THR_64_RD(Device,
                                        EIP164_REG_IFC1_STATISTICS_CONTROLS,
                                        CountOctetThr_Lo,
                                        CountOctetThr_Hi);
}


static inline int
EIP164_SA_COUNT_PACK_TIMING_WR(
        Device_Handle_t Device,
        uint8_t PackTimeout,
        bool fEnable)
{
    return EIP217_COUNT_PACK_TIMING_WR(Device,
                                       EIP164_REG_SA_STATISTICS_CONTROLS,
                                       PackTimeout,
                                       fEnable);
}

static inline int
EIP164_IFC_COUNT_PACK_TIMING_WR(
        Device_Handle_t Device,
        uint8_t PackTimeout,
        bool fEnable)
{
    return EIP217_COUNT_PACK_TIMING_WR(Device,
                                       EIP164_REG_IFC_STATISTICS_CONTROLS,
                                       PackTimeout,
                                       fEnable);
}

static inline int
EIP164_IFC1_COUNT_PACK_TIMING_WR(
        Device_Handle_t Device,
        uint8_t PackTimeout,
        bool fEnable)
{
    return EIP217_COUNT_PACK_TIMING_WR(Device,
                                       EIP164_REG_IFC1_STATISTICS_CONTROLS,
                                       PackTimeout,
                                       fEnable);
}

static inline int
EIP164_SECY_COUNT_PACK_TIMING_WR(
        Device_Handle_t Device,
        uint8_t PackTimeout,
        bool fEnable)
{
    return EIP217_COUNT_PACK_TIMING_WR(Device,
                                       EIP164_REG_SECY_STATISTICS_CONTROLS,
                                       PackTimeout,
                                       fEnable);
}

static inline int
EIP164_RXCAM_COUNT_PACK_TIMING_WR(
        Device_Handle_t Device,
        uint8_t PackTimeout,
        bool fEnable)
{
    return EIP217_COUNT_PACK_TIMING_WR(Device,
                                       EIP164_REG_RXCAM_STATISTICS_CONTROLS,
                                       PackTimeout,
                                       fEnable);
}


static inline int
EIP164_COUNT_SECFAIL1_WR(
        Device_Handle_t Device,
        const uint16_t SASecfailMask,
        const uint16_t ErrorMask)
{
    uint32_t RegVal = EIP164_REG_COUNT_SECFAIL1_DEFAULT;

    RegVal &= ~((MASK_15_BITS <<17) | MASK_16_BITS);
    RegVal |= (uint32_t)((((uint32_t)SASecfailMask) & MASK_16_BITS));
    RegVal |= (uint32_t)((((uint32_t)ErrorMask)     & MASK_15_BITS) << 17);

    return EIP164_Write32(Device, EIP164_REG_COUNT_SECFAIL1, RegVal);
}


static inline int
EIP164_COUNT_SECFAIL2_WR(
        Device_Handle_t Device,
        const uint32_t SecYSecfailMask)
{
    return EIP164_Write32(Device, EIP164_REG_COUNT_SECFAIL2, SecYSecfailMask);
}


static inline int
EIP164_COUNTER_64_DEFAULT_WR(
        Device_Handle_t Device,
        unsigned int Offset,
        unsigned int Index)
{
    return EIP217_COUNTER_64_DEFAULT_WR(Device, Offset, Index);
}


static inline int
EIP164_XFORM_REC_WR(
        Device_Handle_t Device,
        const unsigned int XformIndex,
        const uint32_t * MemorySrc_p,
        const unsigned int Count)
{
    int i;
    int rc = 0;

    rc = EIP164_Write32Array(Device,
                             EIP164_REG_XFORM_REC(XformIndex),
                             MemorySrc_p,
                             Count);
    if (rc) return rc;

    /* Write the rest of the transform record with zero. */
    for (i = Count; i < EIP164_XFORM_REC_WORD_COUNT; i++)
    {
        rc = EIP164_Write32(Device,
                            EIP164_REG_XFORM_REC(XformIndex) +
                            (i * EIP164_REG_OFFS),
                            0);
        if (rc) return rc;
    }

    return rc;
}

static inline int
EIP164_XFORM_REC_WORD_WR(
        Device_Handle_t Device,
        const unsigned int XformIndex,
        const unsigned int WordOffsetInXform,
        const uint32_t DataWord)
{
    return EIP164_Write32(Device,
                          EIP164_REG_XFORM_REC(XformIndex) +
                          (WordOffsetInXform * EIP164_REG_OFFS),
                          DataWord);
}

static inline int
EIP164_XFORM_REC_RD(
        Device_Handle_t Device,
        const unsigned int XformIndex,
        const unsigned int WordOffsetInXform,
        const int WordCount,
        uint32_t * MemoryDst_p)
{
    return EIP164_Read32Array(Device,
                              EIP164_REG_XFORM_REC(XformIndex) +
                              (WordOffsetInXform * EIP164_REG_OFFS),
                              MemoryDst_p,
                              WordCount);
}
static inline int
EIP164_SAM_IN_FLIGHT_WR(
        Device_Handle_t Device,
        const bool fLoadUnsafe)
{
    uint32_t RegVal = EIP164_REG_SAM_IN_FLIGHT_DEFAULT;

    if(fLoadUnsafe)
        RegVal |= BIT_31;
    else
        RegVal &= ~BIT_31;

    return EIP164_Write32(Device, EIP164_REG_SAM_IN_FLIGHT, RegVal);
}


static inline int
EIP164_SAM_IN_FLIGHT_RD(
        Device_Handle_t Device,
        uint8_t * const Unsafe,
        uint8_t * const InFlight,
        bool * const fLoadUnsafe)
{
    uint32_t RegVal = 0;
    int rc = 0;

    rc = EIP164_Read32(Device, EIP164_REG_SAM_IN_FLIGHT, &RegVal);

    if (!rc)
    {
        *Unsafe      = (uint8_t)((RegVal >> 0) & MASK_6_BITS);
        *InFlight    = (uint8_t)((RegVal >> 8) & MASK_6_BITS);
        *fLoadUnsafe = ((RegVal & BIT_31) != 0);
    }

    return rc;
}


static inline int
EIP164_SAM_FLOW_CTRL1_INGRESS_WR(
        Device_Handle_t Device,
        const unsigned int Index,
        const uint8_t DestPort,
        const bool fRetainSecTAG,
        const bool fFlowCryptAuth,
        const uint8_t DropAction,
        const bool fReplayProtect,
        const bool fAllowTaggedData,
        const bool fAllowUntaggedData,
        const uint8_t ValidateFramesTagged,
        const bool ValidateFramesUntagged)
{
    uint32_t RegVal = 0x2;              /* Ingress */

    /* General MACsec SecY part */
    RegVal |= (uint32_t)((((uint32_t)DestPort) & MASK_2_BITS) << 2);

    if(fRetainSecTAG)
        RegVal |= BIT_4;

    if(fFlowCryptAuth)
        RegVal |= BIT_5;

    RegVal |= (uint32_t)((((uint32_t)DropAction) & MASK_2_BITS) << 6);

    /* Specific MACsec SecY part */
    if(fReplayProtect)
        RegVal |= BIT_16;

    if(fAllowTaggedData)
        RegVal |= BIT_17;

    if(fAllowUntaggedData)
        RegVal |= BIT_18;

    RegVal |= (uint32_t)((((uint32_t)ValidateFramesTagged) & MASK_2_BITS) << 19);

    if(ValidateFramesUntagged)
        RegVal |= BIT_21;

    return EIP164_Write32(Device, EIP164_REG_SAM_FLOW_CTRL1(Index), RegVal);
}


static inline int
EIP164_SAM_FLOW_CTRL1_EGRESS_WR(
        Device_Handle_t Device,
        const unsigned int Index,
        const uint8_t DestPort,
        const bool fFlowCryptAuth,
        const uint8_t DropAction,
        const bool fProtectFrames,
        const bool fConfProtect,
        const bool fIncludeSCI,
        const bool fUseES,
        const bool fUseSCB,
        const bool fAllowDataPkts)
{
    uint32_t RegVal = 0x3;              /* Egress */

    /* General MACsec SecY part */
    RegVal |= (uint32_t)((((uint32_t)DestPort) & MASK_2_BITS) << 2);

    if(fFlowCryptAuth)
        RegVal |= BIT_5;

    RegVal |= (uint32_t)((((uint32_t)DropAction) & MASK_2_BITS) << 6);

    /* Specific MACsec SecY part */
    if(fProtectFrames)
        RegVal |= BIT_16;

    if(fConfProtect)
        RegVal |= BIT_17;

    if(fIncludeSCI)
        RegVal |= BIT_18;

    if(fUseES)
        RegVal |= BIT_19;

    if(fUseSCB)
        RegVal |= BIT_20;

    if(fAllowDataPkts)
        RegVal |= BIT_21;

    return EIP164_Write32(Device, EIP164_REG_SAM_FLOW_CTRL1(Index), RegVal);
}

static inline int
EIP164_SAM_FLOW_CTRL1_CRYPTAUTH_WR(
        Device_Handle_t Device,
        const unsigned int Index,
        const uint8_t DestPort,
        const bool fDropNonReserved,
        const bool fFlowCryptAuth,
        const uint8_t DropAction,
        const bool fConfProtect,
        const bool fIcvAppend,
        const uint8_t IV,
        const bool fIcvVerify,
        const bool fUseSCLookup)
{
    uint32_t RegVal = 0;

    RegVal |= (uint32_t)((((uint32_t)DestPort) & MASK_2_BITS) << 2);

    if(fDropNonReserved)
        RegVal |= BIT_4;

    if(fFlowCryptAuth)
        RegVal |= BIT_5;

    RegVal |= (uint32_t)((((uint32_t)DropAction) & MASK_2_BITS) << 6);

    if(fUseSCLookup)
        RegVal |= BIT_16;

    if(fConfProtect)
        RegVal |= BIT_17;

    if(fIcvAppend)
        RegVal |= BIT_18;

    RegVal |= (uint32_t)((((uint32_t)IV) & MASK_2_BITS) << 19);

    if(fIcvVerify)
        RegVal |= BIT_21;

    return EIP164_Write32(Device, EIP164_REG_SAM_FLOW_CTRL1(Index), RegVal);
}


static inline int
EIP164_SAM_FLOW_CTRL1_BYPASS_WR(
        Device_Handle_t Device,
        const unsigned int Index,
        const uint8_t DestPort,
        const bool fDropNonReserved,
        const bool fFlowCryptAuth,
        const uint8_t DropAction)
{
    uint32_t RegVal = 0;

    RegVal |= (uint32_t)((((uint32_t)DestPort) & MASK_2_BITS) << 2);

    if(fDropNonReserved)
        RegVal |= BIT_4;

    if(fFlowCryptAuth)
        RegVal |= BIT_5;

    RegVal |= (uint32_t)((((uint32_t)DropAction) & MASK_2_BITS) << 6);

    return EIP164_Write32(Device, EIP164_REG_SAM_FLOW_CTRL1(Index), RegVal);
}


static inline int
EIP164_SAM_FLOW_CTRL1_DROP_WR(
        Device_Handle_t Device,
        const unsigned int Index,
        const uint8_t DestPort,
        const bool fDropNonReserved,
        const bool fFlowCryptAuth,
        const uint8_t DropAction)
{
    uint32_t RegVal = 1;

    RegVal |= (uint32_t)((((uint32_t)DestPort) & MASK_2_BITS) << 2);

    if(fDropNonReserved)
        RegVal |= BIT_4;

    if(fFlowCryptAuth)
        RegVal |= BIT_5;

    RegVal |= (uint32_t)((((uint32_t)DropAction) & MASK_2_BITS) << 6);

    return EIP164_Write32(Device, EIP164_REG_SAM_FLOW_CTRL1(Index), RegVal);
}



static inline int
EIP164_SAM_FLOW_CTRL1_INGRESS_RD(
        Device_Handle_t Device,
        const unsigned int Index,
        uint8_t * const DestPort,
        bool * const fRetainSecTAG,
        bool * const fFlowCryptAuth,
        uint8_t * const DropAction,
        bool * const fReplayProtect,
        bool * const fAllowTaggedData,
        bool * const fAllowUntaggedData,
        uint8_t * const ValidateFramesTagged,
        bool * const ValidateFramesUntagged)
{
    uint32_t RegVal = 0;
    int rc = 0;

    rc = EIP164_Read32(Device, EIP164_REG_SAM_FLOW_CTRL1(Index), &RegVal);

    if (!rc)
    {
        /* General MACsec SecY part */
        *DestPort = (RegVal >> 2) & MASK_2_BITS;

        *fRetainSecTAG = (RegVal & BIT_4) != 0;

        *fFlowCryptAuth = (RegVal & BIT_5) != 0;

        *DropAction = (RegVal >> 6) & MASK_2_BITS;

        /* Specific MACsec SecY part */
        *fReplayProtect = (RegVal & BIT_16) != 0;

        *fAllowTaggedData = (RegVal & BIT_17) != 0;

        *fAllowUntaggedData = (RegVal & BIT_18) != 0;

        *ValidateFramesTagged = (RegVal >> 19) & MASK_2_BITS;

        *ValidateFramesUntagged = (RegVal & BIT_21) != 0;
    }

    return rc;
}


static inline int
EIP164_SAM_FLOW_CTRL1_EGRESS_RD(
        Device_Handle_t Device,
        const unsigned int Index,
        uint8_t * const DestPort,
        bool * const fFlowCryptAuth,
        uint8_t * const DropAction,
        bool * const fProtectFrames,
        bool * const fConfProtect,
        bool * const fIncludeSCI,
        bool * const fUseES,
        bool * const fUseSCB,
        bool * const fAllowDataPkts)
{
    uint32_t RegVal = 0;
    int rc = 0;

    rc = EIP164_Read32(Device, EIP164_REG_SAM_FLOW_CTRL1(Index), &RegVal);

    if (!rc)
    {
        /* General MACsec SecY part */
        *DestPort = (RegVal >> 2) & MASK_2_BITS;

        *fFlowCryptAuth = (RegVal & BIT_5) != 0;

        *DropAction = (RegVal >> 6) & MASK_2_BITS;

        /* Specific MACsec SecY part */
        *fProtectFrames = (RegVal & BIT_16) != 0;

        *fConfProtect = (RegVal & BIT_17) != 0;

        *fIncludeSCI = (RegVal & BIT_18) != 0;

        *fUseES = (RegVal & BIT_19) != 0;

        *fUseSCB = (RegVal & BIT_20) != 0;

        *fAllowDataPkts = (RegVal & BIT_21) != 0;
    }

    return rc;
}


static inline int
EIP164_SAM_FLOW_CTRL1_CRYPTAUTH_RD(
        Device_Handle_t Device,
        const unsigned int Index,
        uint8_t * const DestPort,
        bool * const fDropNonReserved,
        bool * const fFlowCryptAuth,
        uint8_t * const DropAction,
        bool * const fConfProtect,
        bool * const fIcvAppend,
        uint8_t * const IV,
        bool * const fIcvVerify,
        bool * const fUseSCLookup)
{
    uint32_t RegVal = 0;
    int rc = 0;

    rc = EIP164_Read32(Device, EIP164_REG_SAM_FLOW_CTRL1(Index), &RegVal);

    if (!rc)
    {
        *DestPort = (RegVal >> 2) & MASK_2_BITS;

        *fDropNonReserved = (RegVal & BIT_4) != 0;
        *fFlowCryptAuth = (RegVal & BIT_5) != 0;

        *DropAction = (RegVal >> 6) & MASK_2_BITS;

        *fUseSCLookup = (RegVal & BIT_16) != 0;

        *fConfProtect = (RegVal & BIT_17) != 0;

        *fIcvAppend = (RegVal & BIT_18) != 0;

        *IV = (RegVal >> 19) & MASK_2_BITS;

        *fIcvVerify = (RegVal & BIT_21) != 0;
    }

    return rc;
}


static inline int
EIP164_SAM_FLOW_CTRL1_BYPASS_RD(
        Device_Handle_t Device,
        const unsigned int Index,
        uint8_t * const DestPort,
        bool * const fDropNonReserved,
        bool * const fFlowCryptAuth,
        uint8_t * const  DropAction)
{
    uint32_t RegVal = 0;
    int rc = 0;

    rc = EIP164_Read32(Device, EIP164_REG_SAM_FLOW_CTRL1(Index), &RegVal);

    if (!rc)
    {
        *DestPort = (RegVal >> 2) & MASK_2_BITS;

        *fDropNonReserved = (RegVal & BIT_4) != 0;
        *fFlowCryptAuth = (RegVal & BIT_5) != 0;

        *DropAction = (RegVal >> 6) & MASK_2_BITS;
    }

    return rc;
}


static inline int
EIP164_SAM_FLOW_CTRL1_DROP_RD(
        Device_Handle_t Device,
        const unsigned int Index,
        uint8_t * const DestPort,
        bool * const fDropNonReserved,
        bool * const fFlowCryptAuth,
        uint8_t * const  DropAction)
{
    uint32_t RegVal = 0;
    int rc = 0;

    rc = EIP164_Read32(Device, EIP164_REG_SAM_FLOW_CTRL1(Index), &RegVal);

    if (!rc)
    {
        *DestPort = (RegVal >> 2) & MASK_2_BITS;

        *fDropNonReserved = (RegVal & BIT_4) != 0;
        *fFlowCryptAuth = (RegVal & BIT_5) != 0;

        *DropAction = (RegVal >> 6) & MASK_2_BITS;
    }

    return rc;
}

static inline int
EIP164_SAM_FLOW_CTRL1_TYPE_RD(
        Device_Handle_t Device,
        const unsigned int Index,
        uint8_t * const ActionType)
{
    uint32_t RegVal = 0;
    int rc = 0;

    rc = EIP164_Read32(Device, EIP164_REG_SAM_FLOW_CTRL1(Index), &RegVal);

    if (!rc)
    {
        if (RegVal & BIT_5)
        {
            *ActionType = 4;
        }
        else
        {
            *ActionType = (RegVal & MASK_2_BITS);
        }
    }

    return rc;
}


static inline int
EIP164_SAM_FLOW_CTRL2_WR(
        Device_Handle_t Device,
        const unsigned int Index,
        const uint8_t ConfidentialityOffset,
        const uint8_t PreSecTagAuthStart,
        const uint8_t PreSecTagAuthLen,
        const bool fRetainICV)
{
    uint32_t RegVal = 0;

    if (fRetainICV)
        RegVal |= BIT_31;

    RegVal |= (PreSecTagAuthStart    & MASK_7_BITS);
    RegVal |= (PreSecTagAuthLen      & MASK_6_BITS) << 8;
    RegVal |= (ConfidentialityOffset & MASK_6_BITS) << 24;


    return EIP164_Write32(Device, EIP164_REG_SAM_FLOW_CTRL2(Index), RegVal);
}


static inline int
EIP164_SAM_FLOW_CTRL2_RD(
        Device_Handle_t Device,
        const unsigned int Index,
        uint8_t * const ConfidentialityOffset,
        uint8_t * const PreSecTagAuthStart,
        uint8_t * const PreSecTagAuthLen,
        bool    * const fRetainICV)
{
    uint32_t RegVal = Device_Read32(Device, EIP164_REG_SAM_FLOW_CTRL2(Index));

    *fRetainICV = (RegVal & BIT_31) != 0;

    *PreSecTagAuthStart = (RegVal & MASK_7_BITS);
    *PreSecTagAuthLen   = (RegVal >> 8) & MASK_6_BITS;
    *ConfidentialityOffset = (RegVal >> 24) & MASK_6_BITS;

    return EIP164_Write32(Device, EIP164_REG_SAM_FLOW_CTRL2(Index), RegVal);
}


static inline int
EIP164_SAM_FLOW_CTRL_CLEAR(
        Device_Handle_t Device,
        const unsigned int Index)
{
    int rc = 0;

    rc = EIP164_Write32(Device, EIP164_REG_SAM_FLOW_CTRL1(Index), 0);
    if (rc) return rc;

    return EIP164_Write32(Device, EIP164_REG_SAM_FLOW_CTRL2(Index), 0);
}
static inline int
EIP164_SAM_NM_PARAMS_WR(
        Device_Handle_t Device,
        const unsigned int Channel,
        const bool fCompEType,
        const bool fCheckV,
        const bool fCheckKay,
        const bool fCheckCE,
        const bool fCheckSC,
        const bool fCheckSL,
        const bool fCheckPN,
        const bool fCheckSLExt,
        const uint16_t EtherType)
{
    uint32_t RegVal = EIP164_REG_SAM_NM_PARAMS_DEFAULT;

    if(fCompEType)
        RegVal |= BIT_0;
    else
        RegVal &= ~BIT_0;

    if(fCheckV)
        RegVal |= BIT_1;
    else
        RegVal &= ~BIT_1;

    if(fCheckKay)
        RegVal |= BIT_2;
    else
        RegVal &= ~BIT_2;

    if(fCheckCE)
        RegVal |= BIT_3;
    else
        RegVal &= ~BIT_3;

    if(fCheckSC)
        RegVal |= BIT_4;
    else
        RegVal &= ~BIT_4;

    if(fCheckSL)
        RegVal |= BIT_5;
    else
        RegVal &= ~BIT_5;

    if(fCheckPN)
        RegVal |= BIT_6;
    else
        RegVal &= ~BIT_6;

    if(fCheckSLExt)
        RegVal |= BIT_7;
    else
        RegVal &= ~BIT_7;

    RegVal &= ~(MASK_16_BITS << 16);
    RegVal |= ((uint32_t)EIP164_Swap16(EtherType)) << 16;

    return EIP164_Write32(Device, EIP164_REG_SAM_NM_PARAMS(Channel), RegVal);
}


static inline int
EIP164_SAM_NM_PARAMS_RD(
        Device_Handle_t Device,
        const unsigned int Channel,
        bool * const fCompEType,
        bool * const fCheckV,
        bool * const fCheckKay,
        bool * const fCheckCE,
        bool * const fCheckSC,
        bool * const fCheckSL,
        bool * const fCheckPN,
        bool * const fCheckSLExt,
        uint16_t *const  EtherType)
{
    uint32_t RegVal = 0;
    int rc = 0;

    rc = EIP164_Read32(Device, EIP164_REG_SAM_NM_PARAMS(Channel), &RegVal);

    if (!rc)
    {
        *fCompEType = (RegVal & BIT_0) != 0;
        *fCheckV = (RegVal & BIT_1) != 0;
        *fCheckKay = (RegVal & BIT_2) != 0;
        *fCheckCE = (RegVal & BIT_3) != 0;
        *fCheckSC = (RegVal & BIT_4) != 0;
        *fCheckSL = (RegVal & BIT_5) != 0;
        *fCheckPN = (RegVal & BIT_6) != 0;
        *fCheckSLExt = (RegVal & BIT_7) != 0;

        *EtherType = EIP164_Swap16(RegVal >> 16);
    }

    return rc;
}


static inline int
EIP164_SA_COUNT_SUMMARY_P_WR(
        Device_Handle_t Device,
        const unsigned int Index,
        const uint32_t Bits_Clear)
{
    return EIP217_COUNT_SUMMARY_P_WR(Device,
                                     EIP164_REG_SA_SUMMARY_P,
                                     Index - 1,
                                     &Bits_Clear,
                                     1,
                                     6);
}


static inline int
EIP164_SA_COUNT_SUMMARY_P_RD(
        Device_Handle_t Device,
        const unsigned int Index,
        uint32_t * const Bits_Read)
{
    return EIP217_COUNT_SUMMARY_P_RD(Device,
                                     EIP164_REG_SA_SUMMARY_P,
                                     Index - 1,
                                     Bits_Read,
                                     1,
                                     6);
}


static inline int
EIP164_SA_COUNT_SUMMARY_WR(
        Device_Handle_t Device,
        const unsigned int Index,
        const uint32_t Bits_Clear)
{
    return EIP217_COUNT_SUMMARY_WR(Device,
                                   EIP164_REG_SA_COUNT_SUMMARY_(0),
                                   Index,
                                   &Bits_Clear,
                                   1,
                                   6);
}


static inline int
EIP164_SA_COUNT_SUMMARY_RD(
        Device_Handle_t Device,
        const unsigned int Index,
        uint32_t * const Bits_Read)
{
    return EIP217_COUNT_SUMMARY_RD(Device,
                                   EIP164_REG_SA_COUNT_SUMMARY_(0),
                                   Index,
                                   Bits_Read,
                                   1,
                                   6);
}


static inline int
EIP164_SECY_COUNT_SUMMARY_P_WR(
        Device_Handle_t Device,
        const unsigned int Index,
        const uint32_t Bits_Clear)
{
    return EIP217_COUNT_SUMMARY_P_WR(Device,
                                     EIP164_REG_SECY_SUMMARY_P,
                                     Index - 1,
                                     &Bits_Clear,
                                     1,
                                     5);
}


static inline int
EIP164_SECY_COUNT_SUMMARY_P_RD(
        Device_Handle_t Device,
        const unsigned int Index,
        uint32_t * const Bits_Read)
{
    return EIP217_COUNT_SUMMARY_P_RD(Device,
                                     EIP164_REG_SECY_SUMMARY_P,
                                     Index - 1,
                                     Bits_Read,
                                     1,
                                     5);
}


static inline int
EIP164_SECY_COUNT_SUMMARY_WR(
        Device_Handle_t Device,
        const unsigned int Index,
        const uint32_t Bits_Clear)
{
    return EIP217_COUNT_SUMMARY_WR(Device,
                                   EIP164_REG_SECY_COUNT_SUMMARY_(0),
                                   Index,
                                   &Bits_Clear,
                                   1,
                                   5);
}


static inline int
EIP164_SECY_COUNT_SUMMARY_RD(
        Device_Handle_t Device,
        const unsigned int Index,
        uint32_t * const Bits_Read)
{
    return EIP217_COUNT_SUMMARY_RD(Device,
                                   EIP164_REG_SECY_COUNT_SUMMARY_(0),
                                   Index,
                                   Bits_Read,
                                   1,
                                   5);
}


static inline int
EIP164_SECY_COUNT_SUMMARY_CHAN_WR(
        Device_Handle_t Device,
        const unsigned int Index,
        const uint32_t Bits_Clear)
{
    return EIP217_COUNT_SUMMARY_CHAN_WR(Device,
                                        EIP164_REG_SECY_STATISTICS_CONTROLS,
                                        Index - 1,
                                        &Bits_Clear,
                                        1);
}


static inline int
EIP164_SECY_COUNT_SUMMARY_CHAN_RD(
        Device_Handle_t Device,
        const unsigned int Index,
        uint32_t * const Bits_Read)
{
    return EIP217_COUNT_SUMMARY_CHAN_RD(Device,
                                        EIP164_REG_SECY_STATISTICS_CONTROLS,
                                        Index - 1,
                                        Bits_Read,
                                        1);
}


static inline int
EIP164_IFC_COUNT_SUMMARY_P_WR(
        Device_Handle_t Device,
        const unsigned int Index,
        const uint32_t Bits_Clear)
{
    return EIP217_COUNT_SUMMARY_P_WR(Device,
                                     EIP164_REG_IFC_SUMMARY_P,
                                     Index - 1,
                                     &Bits_Clear,
                                     1,
                                     5);
}


static inline int
EIP164_IFC_COUNT_SUMMARY_P_RD(
        Device_Handle_t Device,
        const unsigned int Index,
        uint32_t * const Bits_Read)
{
    return EIP217_COUNT_SUMMARY_P_RD(Device,
                                     EIP164_REG_IFC_SUMMARY_P,
                                     Index - 1,
                                     Bits_Read,
                                     1,
                                     5);
}


static inline int
EIP164_IFC_COUNT_SUMMARY_WR(
        Device_Handle_t Device,
        const unsigned int Index,
        const uint32_t Bits_Clear)
{
    return EIP217_COUNT_SUMMARY_WR(Device,
                                   EIP164_REG_IFC_COUNT_SUMMARY_(0),
                                   Index,
                                   &Bits_Clear,
                                   1,
                                   5);
}


static inline int
EIP164_IFC_COUNT_SUMMARY_RD(
        Device_Handle_t Device,
        const unsigned int Index,
        uint32_t * const Bits_Read)
{
    return EIP217_COUNT_SUMMARY_RD(Device,
                                   EIP164_REG_IFC_COUNT_SUMMARY_(0),
                                   Index,
                                   Bits_Read,
                                   1,
                                   5);
}


static inline int
EIP164_IFC1_COUNT_SUMMARY_P_WR(
        Device_Handle_t Device,
        const unsigned int Index,
        const uint32_t Bits_Clear)
{
    return EIP217_COUNT_SUMMARY_P_WR(Device,
                                     EIP164_REG_IFC1_SUMMARY_P,
                                     Index - 1,
                                     &Bits_Clear,
                                     1,
                                     5);
}


static inline int
EIP164_IFC1_COUNT_SUMMARY_P_RD(
        Device_Handle_t Device,
        const unsigned int Index,
        uint32_t * const Bits_Read)
{
    return EIP217_COUNT_SUMMARY_P_RD(Device,
                                     EIP164_REG_IFC1_SUMMARY_P,
                                     Index - 1,
                                     Bits_Read,
                                     1,
                                     5);
}


static inline int
EIP164_IFC1_COUNT_SUMMARY_WR(
        Device_Handle_t Device,
        const unsigned int Index,
        const uint32_t Bits_Clear)
{
    return EIP217_COUNT_SUMMARY_WR(Device,
                                   EIP164_REG_IFC1_COUNT_SUMMARY_(0),
                                   Index,
                                   &Bits_Clear,
                                   1,
                                   5);
}


static inline int
EIP164_IFC1_COUNT_SUMMARY_RD(
        Device_Handle_t Device,
        const unsigned int Index,
        uint32_t * const Bits_Read)
{
    return EIP217_COUNT_SUMMARY_RD(Device,
                                   EIP164_REG_IFC1_COUNT_SUMMARY_(0),
                                   Index,
                                   Bits_Read,
                                   1,
                                   5);
}


static inline int
EIP164_RXCAM_COUNT_SUMMARY_P_WR(
        Device_Handle_t Device,
        const unsigned int Index,
        const uint32_t Bits_Clear)
{
    return EIP217_COUNT_SUMMARY_P_WR(Device,
                                     EIP164_REG_RXCAM_SUMMARY_P,
                                     Index - 1,
                                     &Bits_Clear,
                                     1,
                                     5);
}


static inline int
EIP164_RXCAM_COUNT_SUMMARY_P_RD(
        Device_Handle_t Device,
        const unsigned int Index,
        uint32_t * const Bits_Read)
{
    return EIP217_COUNT_SUMMARY_P_RD(Device,
                                     EIP164_REG_RXCAM_SUMMARY_P,
                                     Index - 1,
                                     Bits_Read,
                                     1,
                                     5);
}


#ifdef EIP164_CRYPT_AUTH_ENABLE
static inline int
EIP164_CRYPT_AUTH_CTRL_WR(
        Device_Handle_t Device,
        uint8_t CryptAuthBypassLength)
{
    uint32_t RegVal = EIP164_REG_CRYPT_AUTH_CTRL_DEFAULT;

    RegVal &= ~MASK_8_BITS;
    RegVal |= (uint32_t)(((uint32_t)CryptAuthBypassLength) & MASK_8_BITS);

    return EIP164_Write32(Device, EIP164_REG_CRYPT_AUTH_CTRL, RegVal);
}


static inline int
EIP164_CRYPT_AUTH_CTRL_RD(
        Device_Handle_t Device,
        uint8_t *CryptAuthBypassLength)
{
    uint32_t RegVal = 0;
    int rc = 0;

    rc = EIP164_Read32(Device, EIP164_REG_CRYPT_AUTH_CTRL, &RegVal);

    if (!rc)
    {
        *CryptAuthBypassLength = (uint8_t)(RegVal & MASK_8_BITS);
    }

    return rc;
}
#endif /* EIP164_CRYPT_AUTH_ENABLE */


static inline int
EIP164_CHANNEL_CTRL_RD(
        Device_Handle_t Device,
        const unsigned int Index,
        uint16_t * const Latency_p,
        uint8_t * const BurstLimit_p,
        bool * const fLatencyEnable_p,
        bool * const fBypass_p)
{
    uint32_t RegVal = 0;
    int rc = 0;

    rc = EIP164_Read32(Device, EIP164_REG_CHANNEL_CTRL(Index), &RegVal);

    if (!rc)
    {
        *Latency_p        = (uint16_t)((RegVal >> 8) & MASK_16_BITS);
        *BurstLimit_p     = 0;
        *fLatencyEnable_p = ((RegVal & BIT_1) != 0);
        *fBypass_p = ((RegVal & BIT_0) != 0);
    }

    return rc;
}


static inline int
EIP164_CHANNEL_CTRL_WR(
        Device_Handle_t Device,
        const unsigned int Index,
        const uint16_t Latency,
        const uint8_t BurstLimit,
        const bool fLatencyEnable,
        const bool fBypass)
{
    uint32_t RegVal = EIP164_REG_CHANNEL_CTRL_DEFAULT;

    IDENTIFIER_NOT_USED(BurstLimit);

    if(fBypass)
        RegVal |= BIT_0;
    else
        RegVal &= ~BIT_0;

    if(fLatencyEnable)
        RegVal |= BIT_1;
    else
        RegVal &= ~BIT_1;

    RegVal |= (Latency & MASK_16_BITS) << 8;

    return EIP164_Write32(Device, EIP164_REG_CHANNEL_CTRL(Index), RegVal);
}


static inline int
EIP164_CHANNEL_IN_FLIGHT_RD(
        Device_Handle_t Device,
        const unsigned int ChannelId,
        uint32_t * const InFlight_p)
{
    uint32_t RegVal = 0;
    int rc = 0;

    rc = EIP164_Read32(Device, EIP164_REG_IN_FLIGHT_GL(ChannelId), &RegVal);

    if (!rc)
    {
        *InFlight_p = RegVal;
    }

    return rc;
}


static inline int
EIP164_FORCE_CLOCK_ON_WR(
        Device_Handle_t Device,
        const uint32_t ClockMask)
{
    uint32_t RegVal = EIP164_REG_FORCE_CLOCK_ON_DEFAULT;

    RegVal &= ~MASK_8_BITS;
    RegVal |= (ClockMask & MASK_8_BITS);

    return EIP164_Write32(Device, EIP164_REG_FORCE_CLOCK_ON, RegVal);
}


static inline int
EIP164_FORCE_CLOCK_OFF_WR(
        Device_Handle_t Device,
        const uint32_t ClockMask)
{
    uint32_t RegVal = EIP164_REG_FORCE_CLOCK_OFF_DEFAULT;

    RegVal &= ~MASK_8_BITS;
    RegVal |= (ClockMask & MASK_8_BITS);

    return EIP164_Write32(Device, EIP164_REG_FORCE_CLOCK_OFF, RegVal);
}


static inline int
EIP164_RX_INSERT_SOP_WR(
        Device_Handle_t Device,
        const uint16_t ChannelCount,
        const EIP164_Ch_Mask_t * const ChannelMask_p)
{
    int rc = 0;
    uint16_t i = 0;

    for (i = 0; i < ((ChannelCount + 31) / 32); i++)
    {
         rc = EIP164_Write32(Device,
                             EIP164_REG_RX_INSERT_SOP(i),
                             ChannelMask_p->ch_bitmask[i]);
        if (rc) return rc;
    }

    return rc;
}


static inline int
EIP164_RX_INSERT_EOP_WR(
        Device_Handle_t Device,
        const uint16_t ChannelCount,
        const EIP164_Ch_Mask_t * const ChannelMask_p)
{
    int rc = 0;
    uint16_t i = 0;

    for (i = 0; i < ((ChannelCount + 31) / 32); i++)
    {
         rc = EIP164_Write32(Device,
                             EIP164_REG_RX_INSERT_EOP(i),
                             ChannelMask_p->ch_bitmask[i]);
        if (rc) return rc;
    }

    return rc;
}


static inline int
EIP164_RX_TIMEOUT_CTRL_WR(
        Device_Handle_t Device,
        const uint16_t ChannelCount,
        const EIP164_Ch_Mask_t * const ChannelMask_p)
{
    int rc = 0;
    uint16_t i = 0;

    for (i = 0; i < ((ChannelCount + 31) / 32); i++)
    {
         rc = EIP164_Write32(Device,
                             EIP164_REG_RX_TIMEOUT_CTRL(i),
                             ChannelMask_p->ch_bitmask[i]);
        if (rc) return rc;
    }

    return rc;
}

static inline int
EIP164_RX_TIMEOUT_CTRL_RD(
        Device_Handle_t Device,
        const uint16_t ChannelCount,
        EIP164_Ch_Mask_t * const ChannelMask_p)
{
    int rc = 0;
    uint16_t i = 0;

    for (i = 0; i < ((ChannelCount + 31) / 32); i++)
    {
         rc = EIP164_Read32(Device,
                            EIP164_REG_RX_TIMEOUT_CTRL(i),
                            &ChannelMask_p->ch_bitmask[i]);
        if (rc) return rc;
    }

    return rc;
}


static inline int
EIP164_RX_TIMEOUT_VAL_WR(
        Device_Handle_t Device,
        unsigned int TimeoutVal)
{
    uint32_t RegVal = TimeoutVal & MASK_16_BITS;

    return EIP164_Write32(Device,
                          EIP164_REG_RX_TIMEOUT_VAL,
                          RegVal);
}


static inline int
EIP164_RX_TIMEOUT_VAL_RD(
        Device_Handle_t Device,
        unsigned int * TimeoutVal)
{
    uint32_t RegVal = 0;
    int rc = 0;

    rc = EIP164_Read32(Device, EIP164_REG_RX_TIMEOUT_VAL, &RegVal);

    if (!rc)
    {
        *TimeoutVal = RegVal & MASK_16_BITS;
    }

    return rc;
}


static inline int
EIP164_ECC_COUNTER_RAM_RD(
        Device_Handle_t Device,
        unsigned int Index,
        unsigned int *CorrValue,
        unsigned int *DerrValue)
{
    uint32_t RegVal = 0;
    int rc = 0;

    rc = EIP164_Read32(Device, EIP164_REG_ECC_COUNTER_RAM(Index), &RegVal);

    if (!rc)
    {
        *CorrValue = (RegVal >> 16) & MASK_16_BITS;
        *DerrValue = RegVal & MASK_16_BITS;
    }

    return rc;
}

static inline int
EIP164_ECC_THR_CORR_STAT_RD(
        Device_Handle_t Device,
        uint32_t *ThrMask)
{
    return EIP164_Read32(Device, EIP164_REG_ECC_THR_CORR_STAT, ThrMask);
}

static inline int
EIP164_ECC_THR_DERR_STAT_RD(
        Device_Handle_t Device,
        uint32_t *ThrMask)
{
    return EIP164_Read32(Device, EIP164_REG_ECC_THR_DERR_STAT, ThrMask);
}


static inline int
EIP164_ECC_COUNTER_RAM_WR(
        Device_Handle_t Device,
        unsigned int Index,
        unsigned int CorrValue,
        unsigned int DerrValue)
{

    uint32_t RegVal = ((CorrValue & MASK_16_BITS) << 16) |
                      (DerrValue & MASK_16_BITS);
    return EIP164_Write32(Device, EIP164_REG_ECC_COUNTER_RAM(Index), RegVal);
}

static inline int
EIP164_ECC_CORR_ENABLE_WR(
        Device_Handle_t Device,
        uint32_t EnableMask)
{
    return EIP164_Write32(Device, EIP164_REG_ECC_CORR_ENABLE, EnableMask);
}

static inline int
EIP164_ECC_DERR_ENABLE_WR(
        Device_Handle_t Device,
        uint32_t EnableMask)
{
    return EIP164_Write32(Device, EIP164_REG_ECC_DERR_ENABLE, EnableMask);
}

static inline int
EIP164_ECC_THRESHOLDS_WR(
        Device_Handle_t Device,
        unsigned int CorrValue,
        unsigned int DerrValue)
{
    uint32_t RegVal = ((CorrValue & MASK_16_BITS) << 16) |
                      (DerrValue & MASK_16_BITS);

    return EIP164_Write32(Device, EIP164_REG_ECC_THRESHOLDS, RegVal);
}


static inline int
EIP164_ECC_THRESHOLDS_RD(
        Device_Handle_t Device,
        unsigned int * CorrValue,
        unsigned int * DerrValue)
{
    uint32_t RegVal = 0;
    int rc = 0;

    rc = EIP164_Read32(Device, EIP164_REG_ECC_THRESHOLDS, &RegVal);

    if (!rc)
    {
        *CorrValue = (RegVal >> 16) & MASK_16_BITS;
        *DerrValue = RegVal & MASK_16_BITS;
    }

    return rc;
}

#endif /* EIP164_LEVEL0_H_ */


/* end of file eip164_level0.h */
