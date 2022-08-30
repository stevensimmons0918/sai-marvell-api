/* eip164_device.c
 *
 * EIP-164 Driver Library Device implementation
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

/*----------------------------------------------------------------------------
 * This module implements (provides) the following interface(s):
 */

#include <Kit/DriverFramework/incl/clib.h>

#include <Kit/EIP164/incl/eip164_secy.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

/* Default configuration */
#include <Kit/EIP164/incl/c_eip164.h>

/* Driver Framework Basic Definitions API */
#include <Kit/DriverFramework/incl/basic_defs.h>             /* IDENTIFIER_NOT_USED, bool, uint32_t */

/* Driver Framework Device API */
#include <Kit/DriverFramework/incl/device_types.h>           /* Device_Handle_t */

/* EIP-66 register interface */
#include <Kit/EIP164/incl/eip66_level0.h>

/* EIP-164 register interface */
#include <Kit/EIP164/incl/eip164_level0.h>

/* EIP-164 Internal interface */
#include <Kit/EIP164/incl/eip164_internal.h>

/* EIP-164 Internal Packet Classification Rules interface */
#include <Kit/EIP164/incl/eip164_rules.h>

/* EIP-164 HW interface */
#include <Kit/EIP164/incl/eip164_hw_interface.h>


/*----------------------------------------------------------------------------
 * Definitions and macros
 */
#define HIGH_CH_BIT(ChannelCount)  (1 << ((ChannelCount + 31) % 32))
#define PREP_MASK(ChannelCount)   ((HIGH_CH_BIT(ChannelCount))| (HIGH_CH_BIT(ChannelCount) - 1))
#define MASK_UNUSED_CH_BITS(ChannelCount, ChannelMask) (PREP_MASK(ChannelCount) & (ChannelMask))


/*----------------------------------------------------------------------------
 * Local variables
 */

/*----------------------------------------------------------------------------
 * Local Functions
 */

/*----------------------------------------------------------------------------
 * MaskUnusedChBits
 *
 * Masks the Most Significant Channel bits.
 */
static void
MaskUnusedChBits(
    EIP164_Ch_Mask_t *const ChMask_p,
    const uint16_t ChannelCount)
{
    uint8_t i, k = 0;
    uint32_t val = 0;

    if (0 == ChannelCount) return;

    k = (ChannelCount + 31) / 32;

    /** Get the Channel Mask representing the higher channels */
    val = ChMask_p->ch_bitmask[k - 1];

    ChMask_p->ch_bitmask[k - 1] = MASK_UNUSED_CH_BITS(ChannelCount, val);

    for (i = k; i < EIP164_CHANNEL_WORDS; i++)
        ChMask_p->ch_bitmask[i] = 0;
}


/*----------------------------------------------------------------------------
 * EIP164Lib_Detect
 *
 * Checks the presence of EIP-164 hardware. Returns true when found.
 */
static EIP164_Error_t
EIP164Lib_Detect(
        const Device_Handle_t Device)
{
    uint32_t Value;
    int rc;

    rc = EIP164_Read32(Device, EIP164_REG_VERSION, &Value);
    if (rc) return rc;

    if (!EIP164_REV_SIGNATURE_MATCH( Value ))
        return EIP164_UNSUPPORTED_FEATURE_ERROR;

    /* read-write test one of the registers */

    /* Set 0xF0F0F0F0 bits of the EIP164_REG_XFORM_REC register */
    rc = EIP164_Write32(Device, EIP164_REG_XFORM_REC(0), 0xF0F0F0F0);
    if (rc) return rc;

    rc = EIP164_Read32(Device, EIP164_REG_XFORM_REC(0), &Value);
    if (rc) return rc;

    if (Value != 0xF0F0F0F0)
        return EIP164_UNSUPPORTED_FEATURE_ERROR;

    /* Clear MASK_4_BITS bits of the EIP164_REG_XFORM_REC register */
    rc = EIP164_Write32(Device, EIP164_REG_XFORM_REC(0), 0);
    if (rc) return rc;

    rc = EIP164_Read32(Device, EIP164_REG_XFORM_REC(0), &Value);
    if (rc) return rc;

    if (Value != 0)
       return EIP164_UNSUPPORTED_FEATURE_ERROR;

    return EIP164_NO_ERROR;
}


/*----------------------------------------------------------------------------
 * EIP66Lib_Detect
 *
 * Checks the presence of EIP-6x hardware. Returns true when found.
 */
static EIP164_Error_t
EIP66Lib_Detect(
        const Device_Handle_t Device)
{
    uint32_t RegVal;
    int rc;
    rc = EIP66_Read32(Device, EIP66_REG_VERSION, &RegVal);
    if (rc) return rc;

    if (!EIP66_REV_SIGNATURE_MATCH(RegVal))
        return EIP164_UNSUPPORTED_FEATURE_ERROR;

    return EIP164_NO_ERROR;
}


/*----------------------------------------------------------------------------
 * EIP164Lib_HWRevision_Get
 */
static int
EIP164Lib_HWRevision_Get(
        const Device_Handle_t Device,
        EIP164_Options_t * const Options_p,
        EIP164_Version_t * const Version_p)
{
    int rc = 0;

    rc = EIP164_EIP_REV_RD(Device,
                           &Version_p->EipNumber,
                           &Version_p->ComplmtEipNumber,
                           &Version_p->HWPatchLevel,
                           &Version_p->MinHWRevision,
                           &Version_p->MajHWRevision);
    if (rc) return rc;

    rc = EIP164_CONFIG_RD(Device,
                          &Options_p->SA_Count,
                          &Options_p->vPort_Count,
                          &Options_p->fIngressOnly,
                          &Options_p->fEgressOnly);
    if (rc) return rc;

    return EIP164_CONFIG2_RD(Device,
                             &Options_p->SC_Count,
                             &Options_p->Channel_Count,
                             &Options_p->fSpareReg,
                             &Options_p->fExternalRxCAM);
}


/*----------------------------------------------------------------------------
 * EIP66Lib_HWRevision_Get
 */
static int
EIP66Lib_HWRevision_Get(
        const Device_Handle_t Device,
        EIP66_Options_t * const Options_p,
        EIP164_Version_t * const Version_p)
{
    int rc = 0;

    rc = EIP66_EIP_REV_RD(Device,
                          &Version_p->EipNumber,
                          &Version_p->ComplmtEipNumber,
                          &Version_p->HWPatchLevel,
                          &Version_p->MinHWRevision,
                          &Version_p->MajHWRevision);
    if (rc) return rc;

    return EIP66_CONFIG_RD(Device,
                         &Options_p->fEgressOnly,
                         &Options_p->fIngressOnly,
                         &Options_p->ChannelCount,
                         &Options_p->PipeCount);
}


/*----------------------------------------------------------------------------
 * EIP164Lib_Device_Is_Init_Done
 */
static EIP164_Error_t
EIP164Lib_Device_Is_Init_Done(
        const Device_Handle_t Device,
        bool fExternalRxCAM)
{
    bool fResetAllSA, fResetAllSecY, fResetAllIFC, fResetAllIFC1, fResetAllRXCAM;
    bool fSaturateCtrs, fAutoCtrReset, fResetSummary;
    int rc = 0;

    rc = EIP164_SA_COUNT_CONTROL_RD(Device,
                                    &fResetAllSA,
                                    &fSaturateCtrs,
                                    &fAutoCtrReset,
                                    &fResetSummary);
    if (rc) return rc;

    rc = EIP164_SECY_COUNT_CONTROL_RD(Device,
                                      &fResetAllSecY,
                                      &fSaturateCtrs,
                                      &fAutoCtrReset,
                                      &fResetSummary);
    if (rc) return rc;

    rc = EIP164_IFC_COUNT_CONTROL_RD(Device,
                                     &fResetAllIFC,
                                     &fSaturateCtrs,
                                     &fAutoCtrReset,
                                     &fResetSummary);
    if (rc) return rc;

    rc = EIP164_IFC1_COUNT_CONTROL_RD(Device,
                                      &fResetAllIFC1,
                                      &fSaturateCtrs,
                                      &fAutoCtrReset,
                                      &fResetSummary);
    if (rc) return rc;

    rc = EIP164_RXCAM_COUNT_CONTROL_RD(Device,
                                       &fResetAllRXCAM,
                                       &fSaturateCtrs,
                                       &fAutoCtrReset,
                                       &fResetSummary);
    if (rc) return rc;

    /* CDS point #1: device initialization is done */

    IDENTIFIER_NOT_USED(fSaturateCtrs);
    IDENTIFIER_NOT_USED(fAutoCtrReset);
    IDENTIFIER_NOT_USED(fResetSummary);

    /* Wait till this falls to false indicating that */
    /* the device initialization is completed */
    if (fResetAllSA || fResetAllSecY ||
        fResetAllIFC || fResetAllIFC1 ||
        fResetAllRXCAM)
    {
        return EIP164_BUSY_RETRY_LATER;
    }

    if (fExternalRxCAM)
    {
        bool fWriteBusy,fReadBusy,fDisableBusy,fEnabled;

        rc = EIP164_RXSC_CAM_STATUS_RD(Device,
                                       &fWriteBusy,
                                       &fReadBusy,
                                       &fDisableBusy,
                                       &fEnabled);
        if (rc) return rc;

        if (fDisableBusy)
            return EIP164_BUSY_RETRY_LATER;
    }

    return EIP164_NO_ERROR;
}


/*----------------------------------------------------------------------------
 * EIP164Lib_Device_Is_Sync_Done
 */
static EIP164_Error_t
EIP164Lib_Device_Is_Sync_Done(
        const Device_Handle_t Device)
{
    bool fLoadUnsafe;
    uint8_t Unsafe, InFlight;
    int rc = 0;

    rc = EIP164_SAM_IN_FLIGHT_RD(Device,
                                 &Unsafe,
                                 &InFlight,
                                 &fLoadUnsafe);
    if (rc) return rc;

    IDENTIFIER_NOT_USED(InFlight);
    IDENTIFIER_NOT_USED(fLoadUnsafe);

    /* CDS point #2: device synchronization w.r.t. the requested packets */
    /* processing completion is done */

    /* Wait till this field falls to 0 indicating that */
    /* the device synchronization is completed */
    if (Unsafe == 0) return EIP164_NO_ERROR;

    return EIP164_BUSY_RETRY_LATER;
}



/*----------------------------------------------------------------------------
 * EIP164_Device_Init
 */
EIP164_Error_t
EIP164_Device_Init(
        EIP164_IOArea_t * const IOArea_p,
        const Device_Handle_t Device,
        const EIP164_Settings_t * const Settings_p)
{
    EIP164_Capabilities_t Capabilities;
    volatile EIP164_True_IOArea_t * const TrueIOArea_p = IOAREA_INOUT(IOArea_p);
    int rc = 0;

#ifndef EIP164_DEV_INIT_DISABLE
    unsigned int i;
#endif

    EIP164_CHECK_POINTER(IOArea_p);
    EIP164_CHECK_POINTER(Settings_p);

    /* Detect presence of EIP-164 HW hardware */
    rc = EIP164Lib_Detect(Device);
    if (rc) return rc;
    rc = EIP66Lib_Detect(Device);
    if (rc) return rc;

    /* Initialize the IO Area */
    TrueIOArea_p->Device = Device;

    rc = EIP164Lib_HWRevision_Get(Device,
                                  &Capabilities.EIP164_Options,
                                  &Capabilities.EIP164_Version);

    if (rc) return rc;

    /* Check compatibility between requested mode and HW configuration */
    if (Settings_p->Mode == EIP164_MODE_EGRESS &&
        Capabilities.EIP164_Options.fIngressOnly)
    {
        return EIP164_HW_CONFIGURATION_MISMATCH;
    }

    /* Check compatibility between requested mode and HW configuration */
    if (Settings_p->Mode == EIP164_MODE_INGRESS &&
        Capabilities.EIP164_Options.fEgressOnly)
    {
        return EIP164_HW_CONFIGURATION_MISMATCH;
    }

    /* Check compatibility between requested mode and HW configuration */
    if (Settings_p->Mode == EIP164_MODE_EGRESS_INGRESS &&
        (Capabilities.EIP164_Options.fEgressOnly ||
         Capabilities.EIP164_Options.fIngressOnly))
    {
        return EIP164_HW_CONFIGURATION_MISMATCH;
    }

    TrueIOArea_p->Mode = Settings_p->Mode;

    /* Remember the EIP-164 HW configuration, e, i or ie */
    if (Capabilities.EIP164_Options.fEgressOnly)
    {
        TrueIOArea_p->HWConf = EIP164_HW_CONF_E;
    }
    else if (Capabilities.EIP164_Options.fIngressOnly)
    {
        TrueIOArea_p->HWConf = EIP164_HW_CONF_I;
    }
    else
    {
        TrueIOArea_p->HWConf = EIP164_HW_CONF_IE;
    }

    /* Store the maximum number of Channels, SA's, Secure Channels and vPorts */
    /* that the device supports */
    TrueIOArea_p->MaxChCount = Capabilities.EIP164_Options.Channel_Count;
    TrueIOArea_p->MaxSACount = Capabilities.EIP164_Options.SA_Count;
    TrueIOArea_p->MaxSCCount = Capabilities.EIP164_Options.SC_Count;
    TrueIOArea_p->MaxvPortCount = Capabilities.EIP164_Options.vPort_Count;
    TrueIOArea_p->fExternalRxCAM = Capabilities.EIP164_Options.fExternalRxCAM;
    TrueIOArea_p->fReadPending = false;

#ifndef EIP164_DEV_INIT_DISABLE
    if (TrueIOArea_p->fExternalRxCAM)
    {
        /* Clear all RxCAM entries. */
        rc = EIP164_RXSC_ENTRY_ENABLE_CTRL_WR(Device,
                                              0, false, false,
                                              0, false, true);
        if (rc) return rc;
    }

    /* Program the sequence number threshold */
    if (Settings_p->SeqNrThreshold)
        for (i = 0; i < TrueIOArea_p->MaxChCount; i++)
        {
            rc = EIP66_SEQ_NR_THRESH_WR(Device, i, Settings_p->SeqNrThreshold);
            if (rc) return rc;
        }

    if (Settings_p->SeqNrThreshold64Lo || Settings_p->SeqNrThreshold64Hi)
        for (i = 0; i < TrueIOArea_p->MaxChCount; i++)
        {
            rc = EIP66_SEQ_NR_THRESH_64_WR(Device,
                                           i,
                                           Settings_p->SeqNrThreshold64Lo,
                                           Settings_p->SeqNrThreshold64Hi);

            if (rc) return rc;
        }

    /* Threshold for the frame counters */
    rc = EIP164_SA_COUNT_FRAME_THR_WR(Device,
                                      Settings_p->SACountFrameThrLo,
                                      Settings_p->SACountFrameThrHi);
    if (rc) return rc;

    rc = EIP164_SECY_COUNT_FRAME_THR_WR(Device,
                                        Settings_p->SecYCountFrameThrLo,
                                        Settings_p->SecYCountFrameThrHi);
    if (rc) return rc;

    rc = EIP164_IFC_COUNT_FRAME_THR_WR(Device,
                                       Settings_p->IFCCountFrameThrLo,
                                       Settings_p->IFCCountFrameThrHi);
    if (rc) return rc;

    rc = EIP164_IFC1_COUNT_FRAME_THR_WR(Device,
                                        Settings_p->IFC1CountFrameThrLo,
                                        Settings_p->IFC1CountFrameThrHi);
    if (rc) return rc;

    rc = EIP164_RXCAM_COUNT_FRAME_THR_WR(Device,
                                         Settings_p->RxCAMCountFrameThrLo,
                                         Settings_p->RxCAMCountFrameThrHi);
    if (rc) return rc;

    /* Threshold for the octet counters */
    rc = EIP164_SA_COUNT_OCTET_THR_WR(Device,
                                      Settings_p->SACountOctetThrLo,
                                      Settings_p->SACountOctetThrHi);
    if (rc) return rc;

    rc = EIP164_IFC_COUNT_OCTET_THR_WR(Device,
                                       Settings_p->IFCCountOctetThrLo,
                                       Settings_p->IFCCountOctetThrHi);
    if (rc) return rc;

    rc = EIP164_IFC1_COUNT_OCTET_THR_WR(Device,
                                        Settings_p->IFC1CountOctetThrLo,
                                        Settings_p->IFC1CountOctetThrHi);
    if (rc) return rc;

    /* Reset all statistics counters and threshold summary registers */
    rc = EIP164_SA_COUNT_CONTROL_WR(Device, true, false, false, false);
    if (rc) return rc;

    rc = EIP164_SECY_COUNT_CONTROL_WR(Device, true, false, false, false);
    if (rc) return rc;

    rc = EIP164_IFC_COUNT_CONTROL_WR(Device, true, false, false, false);
    if (rc) return rc;

    rc = EIP164_IFC1_COUNT_CONTROL_WR(Device, true, false, false, false);
    if (rc) return rc;

    rc = EIP164_RXCAM_COUNT_CONTROL_WR(Device, true, false, false, false);
    if (rc) return rc;

    /* Configure the statistics module to clear-on-read and saturating */
    rc = EIP164_SA_COUNT_CONTROL_WR(Device,
                               false,       /* Do not reset */
                               true,        /* Saturate counters */
                               Settings_p->fAutoStatCntrsReset,
                               false);      /* Do not reset */
    if (rc) return rc;

    rc = EIP164_SECY_COUNT_CONTROL_WR(Device,
                                 false,     /* Do not reset */
                                 true,      /* Saturate counters */
                                 Settings_p->fAutoStatCntrsReset,
                                 false);    /* Do not reset */
    if (rc) return rc;

    rc = EIP164_IFC_COUNT_CONTROL_WR(Device,
                                false,      /* Do not reset */
                                true,       /* Saturate counters */
                                Settings_p->fAutoStatCntrsReset,
                                false);     /* Do not reset */
    if (rc) return rc;

    rc = EIP164_IFC1_COUNT_CONTROL_WR(Device,
                                 false,     /* Do not reset */
                                 true,      /* Saturate counters */
                                 Settings_p->fAutoStatCntrsReset,
                                 false);    /* Do not reset */
    if (rc) return rc;

    rc = EIP164_RXCAM_COUNT_CONTROL_WR(Device,
                                  false,    /* Do not reset */
                                  true,     /* Saturate counters */
                                  Settings_p->fAutoStatCntrsReset,
                                  false);   /* Do not reset */
    if (rc) return rc;

    {
        uint16_t SACntrMask, NumSACntrs;
        uint16_t IFCCntrMask, NumIFCCntrs;
        uint16_t SecYCntrMask, NumSecYCntrs;
        uint16_t RxCAMCntrMask, NumRxCAMCntrs;

        NumRxCAMCntrs = sizeof(EIP164_SecY_RxCAM_Stat_t) /
                                sizeof(EIP164_UI64_t);
        if (TrueIOArea_p->Mode == EIP164_MODE_INGRESS)
        {
            NumSACntrs = sizeof(EIP164_SecY_SA_Stat_I_t) /
                                sizeof(EIP164_UI64_t);
            NumIFCCntrs = sizeof(EIP164_SecY_Ifc_Stat_I_t) /
                                sizeof(EIP164_UI64_t);
            NumSecYCntrs = sizeof(EIP164_SecY_Statistics_I_t) /
                                sizeof(EIP164_UI64_t);
        } else {
            NumSACntrs = sizeof(EIP164_SecY_SA_Stat_E_t) /
                                sizeof(EIP164_UI64_t);
            NumIFCCntrs = sizeof(EIP164_SecY_Ifc_Stat_E_t) /
                                sizeof(EIP164_UI64_t);
            NumSecYCntrs = sizeof(EIP164_SecY_Statistics_E_t) /
                                sizeof(EIP164_UI64_t);
        }

        /* Calculate masks for every statistics module to cover all counters */
        SACntrMask = ((1 << NumSACntrs) - 1);
        IFCCntrMask = ((1 << NumIFCCntrs) - 1);
        SecYCntrMask = ((1 << NumSecYCntrs) - 1);
        RxCAMCntrMask = ((1 << NumRxCAMCntrs) - 1);

        /* Enable or disable counters, depending on the value of */
        /* Settings_p->CountIncDisCtrl */
        rc = EIP164_SA_COUNT_INCEN1_WR(Device, SACntrMask *
            (!(Settings_p->CountIncDisCtrl & EIP164_SECY_SA_COUNT_INC_DIS)));
        if (rc) return rc;

        rc = EIP164_IFC_COUNT_INCEN1_WR(Device, IFCCntrMask *
            (!(Settings_p->CountIncDisCtrl & EIP164_SECY_IFC_COUNT_INC_DIS)));
        if (rc) return rc;

        rc = EIP164_IFC1_COUNT_INCEN1_WR(Device, IFCCntrMask *
            (!(Settings_p->CountIncDisCtrl & EIP164_SECY_IFC1_COUNT_INC_DIS)));
        if (rc) return rc;

        rc = EIP164_SECY_COUNT_INCEN1_WR(Device,
            (SecYCntrMask * (!(Settings_p->CountIncDisCtrl &
                               EIP164_SECY_SECY_COUNT_INC_DIS))));
        if (rc) return rc;

        rc = EIP164_RXCAM_COUNT_INCEN1_WR(Device, RxCAMCntrMask *
            (!(Settings_p->CountIncDisCtrl & EIP164_SECY_RXCAM_COUNT_INC_DIS)));
        if (rc) return rc;
    }

    /* Write the PACK_TIMING registers */
    rc = EIP164_SA_COUNT_PACK_TIMING_WR(Device,
                                        EIP164_PACK_TIMEOUT_VALUE,
                                        EIP164_PACK_TIMEOUT_ENABLE);
    if (rc) return rc;

    rc = EIP164_IFC_COUNT_PACK_TIMING_WR(Device,
                                         EIP164_PACK_TIMEOUT_VALUE,
                                         EIP164_PACK_TIMEOUT_ENABLE);
    if (rc) return rc;

    rc = EIP164_IFC1_COUNT_PACK_TIMING_WR(Device,
                                          EIP164_PACK_TIMEOUT_VALUE,
                                          EIP164_PACK_TIMEOUT_ENABLE);
    if (rc) return rc;

    rc = EIP164_SECY_COUNT_PACK_TIMING_WR(Device,
                                          EIP164_PACK_TIMEOUT_VALUE,
                                          EIP164_PACK_TIMEOUT_ENABLE);
    if (rc) return rc;

    rc = EIP164_RXCAM_COUNT_PACK_TIMING_WR(Device,
                                           EIP164_PACK_TIMEOUT_VALUE,
                                           EIP164_PACK_TIMEOUT_ENABLE);
    if (rc) return rc;

    /* Initialize all channels */
    for (i = 0; i < TrueIOArea_p->MaxChCount; i++)
    {
        /* Set channel by default in MACsec mode */
        rc = EIP164_CHANNEL_CTRL_WR(Device,
                                    i,
                                    0,
                                    0,
                                    false,
                                    false);
        if (rc) return rc;

        /* Configure the EIP-66 context control */
        /* Note: For the EIP-164, correct usage according to the manual would */
        /*       be strictly equal comparison (true) focusing on egress, */
        /*       however greater or equal comparison (false) works better for */
        /*       both egress and ingress. */
        rc = EIP66_CTRL_WR(Device,
                           i,         /* channel index */
                           false,     /* Packet Number threshold comparison */
                           0x88E5     /* EtherType for MACSec */);

        if (rc) return rc;

        /* Configured context update on externally bad packets */
        if (TrueIOArea_p->Mode == EIP164_MODE_INGRESS)
        {
            /* To be MACsec compliant disable context update */
            rc = EIP66_CTX_BLK_UPD_CTRL_WR(Device, i, 3);
            if (rc) return rc;
        }
        else
        {
            /* For security reasons enable context update */
            rc = EIP66_CTX_BLK_UPD_CTRL_WR(Device, i, 0);
            if (rc) return rc;
        }
    }

    /* Set device latency */
    rc = EIP164_LATENCY_CTRL_WR(Device, Settings_p->Latency);
    if (rc) return rc;

    /* Set initial processing rules for packets not matching SA flows */
    for (i = 0; i < TrueIOArea_p->MaxvPortCount; i++)
    {
        if (Settings_p->DropBypass.fBypass)
        {
            rc = EIP164_SAM_FLOW_CTRL1_BYPASS_WR(Device,
                                            i,      /* vPort index */
                                            Settings_p->DropBypass.DestPort,
                                            Settings_p->DropBypass.fDropNonReserved,
                                            Settings_p->fFlowCyptAuth,
                                            Settings_p->DropBypass.DropType);
            if (rc) return rc;
        }
        else
        {
            rc = EIP164_SAM_FLOW_CTRL1_DROP_WR(Device,
                                          i,        /* vPort index */
                                          Settings_p->DropBypass.DestPort,
                                          Settings_p->DropBypass.fDropNonReserved,
                                          Settings_p->fFlowCyptAuth,
                                          Settings_p->DropBypass.DropType);
            if (rc) return rc;
        }
    }

    /* Configure SA counters increments as security fail events */
    rc = EIP164_COUNT_SECFAIL1_WR(Device,
                                  Settings_p->SA_SecFail_Mask,
                                  EIP66_FATAL_ERROR_MASK);
    if (rc) return rc;

    /* Configure SecY counters increments as security fail events */
    rc = EIP164_COUNT_SECFAIL2_WR(Device, Settings_p->SecY_SecFail_Mask);
    if (rc) return rc;

    rc = EIP164_Device_EOP_Configure(IOArea_p,
                                     Settings_p->EOPTimeoutVal,
                                     &Settings_p->EOPTimeoutCtrl);
    if (rc) return rc;

    rc = EIP164_Device_ECC_Configure(IOArea_p,
                                     Settings_p->ECCCorrectableThr,
                                     Settings_p->ECCUncorrectableThr);
    if (rc) return rc;
    return EIP164Lib_Device_Is_Init_Done(Device, TrueIOArea_p->fExternalRxCAM);
#else
    return EIP164_NO_ERROR;
#endif /* EIP164_DEV_INIT_DISABLE */
}


/*----------------------------------------------------------------------------
 * EIP164_Device_Is_Init_Done
 *
 */
EIP164_Error_t
EIP164_Device_Is_Init_Done(
        const EIP164_IOArea_t * const IOArea_p)
{
    Device_Handle_t Device;
    const volatile EIP164_True_IOArea_t * const TrueIOArea_p = IOAREA_IN(IOArea_p);

    EIP164_CHECK_POINTER(IOArea_p);

    Device = TrueIOArea_p->Device;

    return EIP164Lib_Device_Is_Init_Done(Device, TrueIOArea_p->fExternalRxCAM);
}


/*----------------------------------------------------------------------------
 * EIP164_Device_Uninit
 */
EIP164_Error_t
EIP164_Device_Uninit(
        EIP164_IOArea_t * const IOArea_p)
{
    Device_Handle_t Device;
    const volatile EIP164_True_IOArea_t * const TrueIOArea_p = IOAREA_IN(IOArea_p);
    unsigned int i;
    int rc = 0;

    EIP164_CHECK_POINTER(IOArea_p);

    Device = TrueIOArea_p->Device;

    for (i = 0; i < TrueIOArea_p->MaxChCount; i++)
    {
        rc = EIP66_SEQ_NR_THRESH_WR(Device, i, 0);
        if (rc) return rc;

        rc = EIP66_SEQ_NR_THRESH_64_WR(Device, i, 0, 0);
        if (rc) return rc;

        /* Set channel by default in MACsec mode */
        rc = EIP164_CHANNEL_CTRL_WR(Device,
                                    i,
                                    0,
                                    0,
                                    false,
                                    false);
        if (rc) return rc;
    }

    /* Threshold for the frame counters */
    rc = EIP164_SA_COUNT_FRAME_THR_WR(Device, 0, 0);
    if (rc) return rc;

    rc = EIP164_SECY_COUNT_FRAME_THR_WR(Device, 0, 0);
    if (rc) return rc;

    rc = EIP164_IFC_COUNT_FRAME_THR_WR(Device, 0, 0);
    if (rc) return rc;

    rc = EIP164_IFC1_COUNT_FRAME_THR_WR(Device, 0, 0);
    if (rc) return rc;

    rc = EIP164_RXCAM_COUNT_FRAME_THR_WR(Device, 0, 0);
    if (rc) return rc;

    /* Threshold for the octet counters */
    rc = EIP164_SA_COUNT_OCTET_THR_WR(Device, 0, 0);
    if (rc) return rc;

    rc = EIP164_IFC_COUNT_OCTET_THR_WR(Device, 0, 0);
    if (rc) return rc;

    rc = EIP164_IFC1_COUNT_OCTET_THR_WR(Device, 0, 0);
    if (rc) return rc;

    /* Reset all statistics counters and threshold summary registers */
    rc = EIP164_SA_COUNT_CONTROL_WR(Device, true, false, false, false);
    if (rc) return rc;

    rc = EIP164_SECY_COUNT_CONTROL_WR(Device, true, false, false, false);
    if (rc) return rc;

    rc = EIP164_IFC_COUNT_CONTROL_WR(Device, true, false, false, false);
    if (rc) return rc;

    rc = EIP164_IFC1_COUNT_CONTROL_WR(Device, true, false, false, false);
    if (rc) return rc;

    rc = EIP164_RXCAM_COUNT_CONTROL_WR(Device, true, false, false, false);
    if (rc) return rc;

    return EIP164Lib_Device_Is_Init_Done(Device, TrueIOArea_p->fExternalRxCAM);
}


/*----------------------------------------------------------------------------
 * EIP164_HWRevision_Get
 */
EIP164_Error_t
EIP164_HWRevision_Get(
        const Device_Handle_t Device,
        EIP164_Capabilities_t * const Capabilities_p)
{
    int rc = 0;

    EIP164_CHECK_POINTER(Capabilities_p);

    rc = EIP164Lib_HWRevision_Get(Device,
                             &Capabilities_p->EIP164_Options,
                             &Capabilities_p->EIP164_Version);
    if (rc) return rc;

    rc = EIP66Lib_HWRevision_Get(Device,
                            &Capabilities_p->EIP66_Options,
                            &Capabilities_p->EIP66_Version);
    if (rc) return rc;

    return EIP164_NO_ERROR;
}


/*----------------------------------------------------------------------------
 * EIP164_Device_Sync
 *
 */
EIP164_Error_t
EIP164_Device_Sync(
        EIP164_IOArea_t * const IOArea_p)
{
    Device_Handle_t Device;
    volatile EIP164_True_IOArea_t * const TrueIOArea_p = IOAREA_INOUT(IOArea_p);
    int rc = 0;

    EIP164_CHECK_POINTER(IOArea_p);

    Device = TrueIOArea_p->Device;

    /* CDS point #2 */
    /* Request device synchronization w.r.t. packets currently available */
    /* in the EIP-164 pipeline */
    rc = EIP164_SAM_IN_FLIGHT_WR(Device, true);
    if (rc) return rc;

    return EIP164Lib_Device_Is_Sync_Done(Device);
}


/*----------------------------------------------------------------------------
 * EIP164_Device_Is_Sync_Done
 *
 */
EIP164_Error_t
EIP164_Device_Is_Sync_Done(
        EIP164_IOArea_t * const IOArea_p)
{
    Device_Handle_t Device;
    volatile EIP164_True_IOArea_t * const TrueIOArea_p = IOAREA_INOUT(IOArea_p);

    EIP164_CHECK_POINTER(IOArea_p);

    Device = TrueIOArea_p->Device;

    return EIP164Lib_Device_Is_Sync_Done(Device);
}


/*----------------------------------------------------------------------------
 * EIP164_Device_Restore
 */
EIP164_Error_t
EIP164_Device_Restore(
        EIP164_IOArea_t * const IOArea_p,
        const Device_Handle_t Device,
        const EIP164_Device_Mode_t Mode)
{
    EIP164_Capabilities_t Capabilities;
    volatile EIP164_True_IOArea_t * const TrueIOArea_p = IOAREA_INOUT(IOArea_p);
    int rc = 0;

    EIP164_CHECK_POINTER(IOArea_p);

    /* Initialize the IO Area */
    TrueIOArea_p->Device = Device;

    rc = EIP164Lib_HWRevision_Get(Device,
                                  &Capabilities.EIP164_Options,
                                  &Capabilities.EIP164_Version);
    if (rc) return rc;

    TrueIOArea_p->Mode = Mode;

        /* Remember the EIP-164 HW configuration, e, i or ie */
    if (Capabilities.EIP164_Options.fEgressOnly)
    {
        TrueIOArea_p->HWConf = EIP164_HW_CONF_E;
    }
    else if (Capabilities.EIP164_Options.fIngressOnly)
    {
        TrueIOArea_p->HWConf = EIP164_HW_CONF_I;
    }
    else
    {
        TrueIOArea_p->HWConf = EIP164_HW_CONF_IE;
    }

    /* Store the maximum number of Channels, SA's, Secure Channels and vPorts */
    /* that the device supports */
    TrueIOArea_p->MaxChCount = Capabilities.EIP164_Options.Channel_Count;
    TrueIOArea_p->MaxSACount = Capabilities.EIP164_Options.SA_Count;
    TrueIOArea_p->MaxSCCount = Capabilities.EIP164_Options.SC_Count;
    TrueIOArea_p->MaxvPortCount = Capabilities.EIP164_Options.vPort_Count;
    TrueIOArea_p->fExternalRxCAM = Capabilities.EIP164_Options.fExternalRxCAM;
    TrueIOArea_p->fReadPending = false;

    return EIP164_NO_ERROR;
}


/*----------------------------------------------------------------------------
 * EIP164_Device_ChannelControl_Read
 */
EIP164_Error_t
EIP164_Device_ChannelControl_Read(
        const EIP164_IOArea_t * const IOArea_p,
        const unsigned int ChannelId,
        uint16_t * const Latency_p,
        uint8_t * const BurstLimit_p,
        bool * const fLatencyEnable_p,
        bool * const fLowLatencyBypass_p)
{
    Device_Handle_t Device;
    const volatile EIP164_True_IOArea_t * const TrueIOArea_p = IOAREA_IN(IOArea_p);
    int rc = 0;

    EIP164_CHECK_POINTER(IOArea_p);
    EIP164_CHECK_POINTER(Latency_p);
    EIP164_CHECK_POINTER(BurstLimit_p);
    EIP164_CHECK_POINTER(fLatencyEnable_p);
    EIP164_CHECK_POINTER(fLowLatencyBypass_p);
    EIP164_CHECK_INT_ATMOST(ChannelId, (TrueIOArea_p->MaxChCount - 1));

    Device = TrueIOArea_p->Device;

    rc = EIP164_CHANNEL_CTRL_RD(Device,
                                ChannelId,
                                Latency_p,
                                BurstLimit_p,
                                fLatencyEnable_p,
                                fLowLatencyBypass_p);
    if (rc) return rc;

    return EIP164_NO_ERROR;
}


/*----------------------------------------------------------------------------
 * EIP164_Device_ChannelControl_Write
 */
EIP164_Error_t
EIP164_Device_ChannelControl_Write(
        const EIP164_IOArea_t * const IOArea_p,
        const unsigned int ChannelId,
        const uint16_t Latency,
        const uint8_t BurstLimit,
        const bool fLatencyEnable,
        const bool fLowLatencyBypass)
{
    Device_Handle_t Device;
    const volatile EIP164_True_IOArea_t * const TrueIOArea_p = IOAREA_IN(IOArea_p);
    int rc = 0;

    EIP164_CHECK_POINTER(IOArea_p);
    EIP164_CHECK_INT_ATMOST(ChannelId, (TrueIOArea_p->MaxChCount - 1));

    Device = TrueIOArea_p->Device;

    rc = EIP164_CHANNEL_CTRL_WR(Device,
                                ChannelId,
                                Latency,
                                BurstLimit,
                                fLatencyEnable,
                                fLowLatencyBypass);

    if (rc) return rc;

    return EIP164_NO_ERROR;
}


/*----------------------------------------------------------------------------
 * EIP164_Device_Channel_CtxControl_Write
 */
EIP164_Error_t
EIP164_Device_Channel_CtxControl_Write(
        const EIP164_IOArea_t * const IOArea_p,
        const unsigned int ChannelId,
        const bool fPktNumThrMode,
        const uint16_t EtherType)
{
    Device_Handle_t Device;
    const volatile EIP164_True_IOArea_t * const TrueIOArea_p = IOAREA_IN(IOArea_p);
    int rc = 0;

    EIP164_CHECK_POINTER(IOArea_p);
    EIP164_CHECK_INT_ATMOST(ChannelId, (TrueIOArea_p->MaxChCount - 1));

    Device = TrueIOArea_p->Device;

    /* Re-configure the EIP-66 context control */
    rc = EIP66_CTRL_WR(Device,
                       ChannelId,      /* channel index */
                       fPktNumThrMode, /* Packet Number threshold comparison */
                       EtherType);

    if (rc) return rc;

    return EIP164_NO_ERROR;
}


/*----------------------------------------------------------------------------
 * EIP164_Device_Channel_CtxControl_Read
 */
EIP164_Error_t
EIP164_Device_Channel_CtxControl_Read(
        const EIP164_IOArea_t * const IOArea_p,
        const unsigned int ChannelId,
        bool * const fPktNumThrMode_p,
        uint16_t *const EtherType_p)
{
    Device_Handle_t Device;
    const volatile EIP164_True_IOArea_t * const TrueIOArea_p = IOAREA_IN(IOArea_p);
    int rc = 0;

    EIP164_CHECK_POINTER(IOArea_p);
    EIP164_CHECK_INT_ATMOST(ChannelId, (TrueIOArea_p->MaxChCount - 1));

    Device = TrueIOArea_p->Device;

    /* Read the EIP-66 context control */
    rc = EIP66_CTRL_RD(Device,
                       ChannelId,        /* channel index */
                       fPktNumThrMode_p, /* Packet Number threshold comparison */
                       EtherType_p);      /* EtherType for MACSec */
    if (rc) return rc;

    return EIP164_NO_ERROR;
}


/*----------------------------------------------------------------------------
 * EIP164_Device_PN_Threshold_Control_Write
 */
EIP164_Error_t
EIP164_Device_PN_Threshold_Control_Write(
    const EIP164_IOArea_t * const IOArea_p,
    const unsigned int ChannelId,
    const EIP164_PN_Threshold_t * const PN_Threshold_p)
{
    Device_Handle_t Device;
    const volatile EIP164_True_IOArea_t * const TrueIOArea_p = IOAREA_IN(IOArea_p);
    int rc = 0;

    EIP164_CHECK_POINTER(IOArea_p);
    EIP164_CHECK_POINTER(PN_Threshold_p);
    EIP164_CHECK_INT_ATMOST(ChannelId, (TrueIOArea_p->MaxChCount - 1));

    Device = TrueIOArea_p->Device;

    /* Program the 32-bit sequence number threshold */
    rc = EIP66_SEQ_NR_THRESH_WR(Device,
                                ChannelId,
                                PN_Threshold_p->SeqNrThreshold);
    if (rc) return rc;

    /* Program the 64-bit sequence number threshold */
    rc = EIP66_SEQ_NR_THRESH_64_WR(Device,
                                   ChannelId,
                                   PN_Threshold_p->SeqNrThreshold64.low,
                                   PN_Threshold_p->SeqNrThreshold64.hi);
    if (rc) return rc;

    return EIP164_NO_ERROR;
}


/*----------------------------------------------------------------------------
 * EIP164_Device_PN_Threshold_Control_Read
 */
EIP164_Error_t
EIP164_Device_PN_Threshold_Control_Read(
    const EIP164_IOArea_t * const IOArea_p,
    const unsigned int ChannelId,
    EIP164_PN_Threshold_t * const PN_Threshold_p)
{
    Device_Handle_t Device;
    const volatile EIP164_True_IOArea_t * const TrueIOArea_p = IOAREA_IN(IOArea_p);
    int rc = 0;

    EIP164_CHECK_POINTER(IOArea_p);
    EIP164_CHECK_POINTER(PN_Threshold_p);
    EIP164_CHECK_INT_ATMOST(ChannelId, (TrueIOArea_p->MaxChCount - 1));

    Device = TrueIOArea_p->Device;

    /* Program the 32-bit sequence number threshold */
    rc = EIP66_SEQ_NR_THRESH_RD(Device,
                                ChannelId,
                                &PN_Threshold_p->SeqNrThreshold);
    if (rc) return rc;

    /* Program the 64-bit sequence number threshold */
    rc = EIP66_SEQ_NR_THRESH_64_RD(Device,
                                   ChannelId,
                                   &PN_Threshold_p->SeqNrThreshold64.low,
                                   &PN_Threshold_p->SeqNrThreshold64.hi);
    if (rc) return rc;

    return EIP164_NO_ERROR;
}


/*----------------------------------------------------------------------------
 * EIP164_Device_Channel_PacketsInFlight_read
 */
EIP164_Error_t
EIP164_Device_Channel_PacketsInFlight_Read(
        const EIP164_IOArea_t * const IOArea_p,
        const unsigned int ChannelId,
        uint32_t * const InFlight_p)
{
    Device_Handle_t Device;
    const volatile EIP164_True_IOArea_t * const TrueIOArea_p = IOAREA_IN(IOArea_p);
    int rc = 0;

    EIP164_CHECK_POINTER(IOArea_p);

    Device = TrueIOArea_p->Device;

    rc = EIP164_CHANNEL_IN_FLIGHT_RD(Device, ChannelId, InFlight_p);
    if (rc) return rc;

    return EIP164_NO_ERROR;
}


/*----------------------------------------------------------------------------
 * EIP164_Device_Clocks_On
 */
EIP164_Error_t
EIP164_Device_Clocks_On(
        const EIP164_IOArea_t * const IOArea_p,
        const uint32_t ClockMask,
        const bool fAlwaysOn)
{
    Device_Handle_t Device;
    const volatile EIP164_True_IOArea_t * const TrueIOArea_p = IOAREA_IN(IOArea_p);
    int rc = 0;

    EIP164_CHECK_POINTER(IOArea_p);

    Device = TrueIOArea_p->Device;

    rc = EIP164_FORCE_CLOCK_ON_WR(Device,
                                  fAlwaysOn ? (ClockMask & MASK_6_BITS) : 0);
    if (rc) return rc;

    rc = EIP66_FORCE_CLOCK_ON_WR(Device,
                            fAlwaysOn ? ((ClockMask >> 16) & MASK_5_BITS) : 0);
    if (rc) return rc;

    return EIP164_NO_ERROR;
}


/*----------------------------------------------------------------------------
 * EIP164_Device_Clocks_Off
 */
EIP164_Error_t
EIP164_Device_Clocks_Off(
        const EIP164_IOArea_t * const IOArea_p,
        const uint32_t ClockMask)
{
    Device_Handle_t Device;
    const volatile EIP164_True_IOArea_t * const TrueIOArea_p = IOAREA_IN(IOArea_p);
    int rc = 0;

    EIP164_CHECK_POINTER(IOArea_p);

    Device = TrueIOArea_p->Device;

    rc = EIP164_FORCE_CLOCK_OFF_WR(Device, ClockMask);
    if (rc) return rc;

    rc = EIP66_FORCE_CLOCK_OFF_WR(Device, (ClockMask >> 16) & MASK_5_BITS);
    if (rc) return rc;

    return EIP164_NO_ERROR;
}


/*----------------------------------------------------------------------------
 * EIP164_Device_EOP_Configure
 */
EIP164_Error_t
EIP164_Device_EOP_Configure(
        const EIP164_IOArea_t * const IOArea_p,
        const unsigned int EOPTimeoutVal,
        const EIP164_Ch_Mask_t * const EOPTimeoutCtrl_p)
{
    Device_Handle_t Device;
    const volatile EIP164_True_IOArea_t * const TrueIOArea_p = IOAREA_IN(IOArea_p);
    uint16_t ChannelCount = TrueIOArea_p->MaxChCount;
    EIP164_Ch_Mask_t EOPTimeoutCtrl;
    int rc = 0;

    EIP164_CHECK_POINTER(IOArea_p);
    EIP164_CHECK_POINTER(EOPTimeoutCtrl_p);

    Device = TrueIOArea_p->Device;

    rc = EIP164_RX_TIMEOUT_VAL_WR(Device, EOPTimeoutVal);
    if (rc) return rc;

    cpssOsMemCpy(&EOPTimeoutCtrl, EOPTimeoutCtrl_p, sizeof(EIP164_Ch_Mask_t));
    MaskUnusedChBits(&EOPTimeoutCtrl, ChannelCount);

    rc = EIP164_RX_TIMEOUT_CTRL_WR(Device, ChannelCount, &EOPTimeoutCtrl);
    if (rc) return rc;

    return EIP164_NO_ERROR;
}


/*----------------------------------------------------------------------------
 * EIP164_Device_EOP_Configuration_Get
 */
EIP164_Error_t
EIP164_Device_EOP_Configuration_Get(
        const EIP164_IOArea_t * const IOArea_p,
        unsigned int * const EOPTimeoutVal_p,
        EIP164_Ch_Mask_t * const EOPTimeoutCtrl_p)
{
    Device_Handle_t Device;
    const volatile EIP164_True_IOArea_t * const TrueIOArea_p = IOAREA_IN(IOArea_p);
    uint16_t ChannelCount = TrueIOArea_p->MaxChCount;
    int rc = 0;

    EIP164_CHECK_POINTER(IOArea_p);
    EIP164_CHECK_POINTER(EOPTimeoutVal_p);
    EIP164_CHECK_POINTER(EOPTimeoutCtrl_p);

    Device = TrueIOArea_p->Device;

    rc = EIP164_RX_TIMEOUT_VAL_RD(Device, EOPTimeoutVal_p);
    if (rc) return rc;

    rc = EIP164_RX_TIMEOUT_CTRL_RD(Device, ChannelCount, EOPTimeoutCtrl_p);
    if (rc) return rc;

    MaskUnusedChBits(EOPTimeoutCtrl_p, ChannelCount);

    return EIP164_NO_ERROR;
}


/*----------------------------------------------------------------------------
 * EIP164_Device_InsertSOP
 */
EIP164_Error_t
EIP164_Device_InsertSOP(
        const EIP164_IOArea_t * const IOArea_p,
        const EIP164_Ch_Mask_t * const ChannelMask_p)
{
    Device_Handle_t Device;
    const volatile EIP164_True_IOArea_t * const TrueIOArea_p = IOAREA_IN(IOArea_p);
    uint16_t ChannelCount = TrueIOArea_p->MaxChCount;
    EIP164_Ch_Mask_t Ch_Mask;
    int rc = 0;

    EIP164_CHECK_POINTER(IOArea_p);
    EIP164_CHECK_POINTER(ChannelMask_p);

    Device = TrueIOArea_p->Device;

    cpssOsMemCpy(&Ch_Mask, ChannelMask_p, sizeof(EIP164_Ch_Mask_t));
    MaskUnusedChBits(&Ch_Mask, ChannelCount);

    rc = EIP164_RX_INSERT_SOP_WR(Device, ChannelCount, &Ch_Mask);
    if (rc) return rc;

    return EIP164_NO_ERROR;
}


/*----------------------------------------------------------------------------
 * EIP164_Device_InsertEOP
 */
EIP164_Error_t
EIP164_Device_InsertEOP(
        const EIP164_IOArea_t * const IOArea_p,
        const EIP164_Ch_Mask_t * const ChannelMask_p)
{
    Device_Handle_t Device;
    const volatile EIP164_True_IOArea_t * const TrueIOArea_p = IOAREA_IN(IOArea_p);
    uint16_t ChannelCount = TrueIOArea_p->MaxChCount;
    EIP164_Ch_Mask_t Ch_Mask;
    int rc = 0;

    EIP164_CHECK_POINTER(IOArea_p);
    EIP164_CHECK_POINTER(ChannelMask_p);
    if (0 == ChannelCount) return EIP164_ARGUMENT_ERROR;

    Device = TrueIOArea_p->Device;

    cpssOsMemCpy(&Ch_Mask, ChannelMask_p, sizeof(EIP164_Ch_Mask_t));
    MaskUnusedChBits(&Ch_Mask, ChannelCount);

    rc = EIP164_RX_INSERT_EOP_WR(Device, ChannelCount, &Ch_Mask);
    if (rc) return rc;

    return EIP164_NO_ERROR;
}


/*----------------------------------------------------------------------------
 * EIP164_Device_ECC_Configure
 */
EIP164_Error_t
EIP164_Device_ECC_Configure(
        const EIP164_IOArea_t * const IOArea_p,
        const unsigned int CorrectableThr,
        const unsigned int UncorrectableThr)
{
    Device_Handle_t Device;
    const volatile EIP164_True_IOArea_t * const TrueIOArea_p = IOAREA_IN(IOArea_p);
    unsigned int CorrThrValue, DerrThrValue;
    uint32_t CorrMaskValue, DerrMaskValue;
    int rc = 0;

    EIP164_CHECK_POINTER(IOArea_p);

    Device = TrueIOArea_p->Device;

    if (CorrectableThr > 0)
    {
        CorrMaskValue = MASK_18_BITS;
        CorrThrValue = CorrectableThr - 1;
    }
    else
    {
        CorrMaskValue = 0;
        CorrThrValue = 0xffff;
    }

    if (UncorrectableThr > 0)
    {
        DerrMaskValue = MASK_18_BITS;
        DerrThrValue = UncorrectableThr - 1;
    }
    else
    {
        DerrMaskValue = 0;
        DerrThrValue = 0xffff;
    }

    rc = EIP164_ECC_THRESHOLDS_WR(Device, CorrThrValue, DerrThrValue);
    if (rc) return rc;

    rc = EIP164_ECC_CORR_ENABLE_WR(Device, CorrMaskValue);
    if (rc) return rc;

    rc = EIP164_ECC_DERR_ENABLE_WR(Device, DerrMaskValue);
    if (rc) return rc;

    return EIP164_NO_ERROR;
}


/*----------------------------------------------------------------------------
 * EIP164_Device_ECC_Configuration_Get
 */
EIP164_Error_t
EIP164_Device_ECC_Configuration_Get(
        const EIP164_IOArea_t * const IOArea_p,
        unsigned int * const CorrectableThr_p,
        unsigned int * const UncorrectableThr_p)
{
    Device_Handle_t Device;
    const volatile EIP164_True_IOArea_t * const TrueIOArea_p = IOAREA_IN(IOArea_p);
    unsigned int CorrThrValue, DerrThrValue;
    int rc = 0;

    EIP164_CHECK_POINTER(IOArea_p);

    Device = TrueIOArea_p->Device;

    rc = EIP164_ECC_THRESHOLDS_RD(Device, &CorrThrValue, &DerrThrValue);
    if (rc) return rc;

    if (CorrThrValue == 0xffff)
    {
        *CorrectableThr_p = 0;
    }
    else
    {
        *CorrectableThr_p = CorrThrValue + 1;
    }

    if (DerrThrValue == 0xffff)
    {
        *UncorrectableThr_p = 0;
    }
    else
    {
        *UncorrectableThr_p = DerrThrValue + 1;
    }

    return EIP164_NO_ERROR;
}


/*----------------------------------------------------------------------------
 * EIP164_Device_ECCStatus_Get
  */
EIP164_Error_t
EIP164_Device_ECCStatus_Get(
        const EIP164_IOArea_t * const IOArea_p,
        EIP164_Device_ECCStatus_t * const ECCStatus_p)
{
    Device_Handle_t Device;
    const volatile EIP164_True_IOArea_t * const TrueIOArea_p = IOAREA_IN(IOArea_p);
    unsigned int i;
    uint32_t CorrStatus, DerrStatus;
    int rc = 0;

    EIP164_CHECK_POINTER(IOArea_p);
    EIP164_CHECK_POINTER(ECCStatus_p);

    Device = TrueIOArea_p->Device;

    rc = EIP164_ECC_THR_CORR_STAT_RD(Device, &CorrStatus);
    if (rc) return rc;

    rc = EIP164_ECC_THR_DERR_STAT_RD(Device, &DerrStatus);
    if (rc) return rc;

    for (i = 0; i < EIP164_ECC_NOF_STATUS_COUNTERS; i++)
    {
        unsigned int CorrValue,DerrValue;
        rc = EIP164_ECC_COUNTER_RAM_RD(Device, i, &CorrValue, &DerrValue);
        if (rc) return rc;

        rc = EIP164_ECC_COUNTER_RAM_WR(Device, i, CorrValue, DerrValue);
        if (rc) return rc;

        ECCStatus_p[i].CorrectableCount = CorrValue;
        ECCStatus_p[i].UncorrectableCount = DerrValue;
        ECCStatus_p[i].fCorrectableThr = (CorrStatus & (1<<i)) != 0;
        ECCStatus_p[i].fUncorrectableThr = (DerrStatus & (1<<i)) != 0;
    }

    return EIP164_NO_ERROR;
}



/*----------------------------------------------------------------------------
 * EIP164_Device_PktProcessDebug_Get
 */
EIP164_Error_t
EIP164_Device_PktProcessDebug_Get(
        const EIP164_IOArea_t * const IOArea_p,
        EIP164_Device_PktProcessDebug_t * const PktProcessDebug_p)
{
    Device_Handle_t Device;
    const volatile EIP164_True_IOArea_t * const TrueIOArea_p = IOAREA_IN(IOArea_p);
    int rc = 0;

    EIP164_CHECK_POINTER(IOArea_p);
    EIP164_CHECK_POINTER(PktProcessDebug_p);

    Device = TrueIOArea_p->Device;

    rc = EIP164_DBG_PARSED_DA_RD(Device,
                            &PktProcessDebug_p->ParsedDALo,
                            &PktProcessDebug_p->ParsedDAHi);
    if (rc) return rc;

    rc = EIP164_DBG_PARSED_SA_RD(Device,
                            &PktProcessDebug_p->ParsedSALo,
                            &PktProcessDebug_p->ParsedSAHi);
    if (rc) return rc;

    rc = EIP164_DBG_PARSED_SECTAG_RD(Device,
                            &PktProcessDebug_p->ParsedSecTAGLo,
                            &PktProcessDebug_p->ParsedSecTAGHi);
    if (rc) return rc;

    rc = EIP164_DBG_PARSED_SCI_RD(Device,
                            &PktProcessDebug_p->ParsedSCILo,
                            &PktProcessDebug_p->ParsedSCIHi);
    if (rc) return rc;

    rc = EIP164_SECTAG_DEBUG_RD(Device, &PktProcessDebug_p->SecTAGDebug);
    if (rc) return rc;

    rc = EIP164_DBG_RXCAM_SCI_RD(Device,
                            &PktProcessDebug_p->RxCAMSCILo,
                            &PktProcessDebug_p->RxCAMSCIHi);
    if (rc) return rc;

    rc = EIP164_PARSER_IN_DEBUG_RD(Device, &PktProcessDebug_p->ParserInDebug);
    if (rc) return rc;

    return EIP164_NO_ERROR;
}


/*----------------------------------------------------------------------------
 * EIP164_Device_SA_Statistics_Enable
 */
EIP164_Error_t
EIP164_Device_SA_Statistics_Enable(
        const EIP164_IOArea_t * const IOArea_p,
        const uint32_t Mask)
{
    Device_Handle_t Device;
    const volatile EIP164_True_IOArea_t * const TrueIOArea_p = IOAREA_IN(IOArea_p);
    int rc = 0;

    EIP164_CHECK_POINTER(IOArea_p);

    Device = TrueIOArea_p->Device;

    rc = EIP164_SA_COUNT_INCEN1_WR(Device, Mask);
    if (rc) return rc;

    return EIP164_NO_ERROR;
}


/*----------------------------------------------------------------------------
 * EIP164_Device_SA_Statistics_Enabled_Get
 */
EIP164_Error_t
EIP164_Device_SA_Statistics_Enabled_Get(
        const EIP164_IOArea_t * const IOArea_p,
        uint32_t * const  Mask_p)
{
    Device_Handle_t Device;
    const volatile EIP164_True_IOArea_t * const TrueIOArea_p = IOAREA_IN(IOArea_p);
    int rc = 0;

    EIP164_CHECK_POINTER(IOArea_p);
    EIP164_CHECK_POINTER(Mask_p);

    Device = TrueIOArea_p->Device;

    rc = EIP164_SA_COUNT_INCEN1_RD(Device, Mask_p);
    if (rc) return rc;

    return EIP164_NO_ERROR;
}


/*----------------------------------------------------------------------------
 * EIP164_Device_SecY_Statistics_Enable
 */
EIP164_Error_t
EIP164_Device_SecY_Statistics_Enable(
        const EIP164_IOArea_t * const IOArea_p,
        const uint32_t Mask)
{
    Device_Handle_t Device;
    const volatile EIP164_True_IOArea_t * const TrueIOArea_p = IOAREA_IN(IOArea_p);
    int rc = 0;

    EIP164_CHECK_POINTER(IOArea_p);

    Device = TrueIOArea_p->Device;

    rc = EIP164_SECY_COUNT_INCEN1_WR(Device, Mask);
    if (rc) return rc;

    return EIP164_NO_ERROR;
}


/*----------------------------------------------------------------------------
 * EIP164_Device_SecY_Statistics_Enabled_Get
 */
EIP164_Error_t
EIP164_Device_SecY_Statistics_Enabled_Get(
        const EIP164_IOArea_t * const IOArea_p,
        uint32_t * const Mask_p)
{
    Device_Handle_t Device;
    const volatile EIP164_True_IOArea_t * const TrueIOArea_p = IOAREA_IN(IOArea_p);
    int rc = 0;

    EIP164_CHECK_POINTER(IOArea_p);
    EIP164_CHECK_POINTER(Mask_p);

    Device = TrueIOArea_p->Device;

    rc = EIP164_SECY_COUNT_INCEN1_RD(Device, Mask_p);
    if (rc) return rc;

    return EIP164_NO_ERROR;
}


/*----------------------------------------------------------------------------
 * EIP164_Device_IFC_Statistics_Enable
 */
EIP164_Error_t
EIP164_Device_IFC_Statistics_Enable(
        const EIP164_IOArea_t * const IOArea_p,
        const uint32_t Mask)
{
    Device_Handle_t Device;
    const volatile EIP164_True_IOArea_t * const TrueIOArea_p = IOAREA_IN(IOArea_p);
    int rc = 0;

    EIP164_CHECK_POINTER(IOArea_p);

    Device = TrueIOArea_p->Device;

    rc = EIP164_IFC_COUNT_INCEN1_WR(Device, Mask);
    if (rc) return rc;

    return EIP164_NO_ERROR;
}


/*----------------------------------------------------------------------------
 * EIP164_Device_IFC_Statistics_Enabled_Get
 */
EIP164_Error_t
EIP164_Device_IFC_Statistics_Enabled_Get(
        const EIP164_IOArea_t * const IOArea_p,
        uint32_t * const Mask_p)
{
    Device_Handle_t Device;
    const volatile EIP164_True_IOArea_t * const TrueIOArea_p = IOAREA_IN(IOArea_p);
    int rc = 0;

    EIP164_CHECK_POINTER(IOArea_p);
    EIP164_CHECK_POINTER(Mask_p);

    Device = TrueIOArea_p->Device;

    rc = EIP164_IFC_COUNT_INCEN1_RD(Device, Mask_p);
    if (rc) return rc;

    return EIP164_NO_ERROR;
}


/*----------------------------------------------------------------------------
 * EIP164_Device_IFC1_Statistics_Enable
 */
EIP164_Error_t
EIP164_Device_IFC1_Statistics_Enable(
        const EIP164_IOArea_t * const IOArea_p,
        const uint32_t Mask)
{
    Device_Handle_t Device;
    const volatile EIP164_True_IOArea_t * const TrueIOArea_p = IOAREA_IN(IOArea_p);
    int rc = 0;

    EIP164_CHECK_POINTER(IOArea_p);

    Device = TrueIOArea_p->Device;

    rc = EIP164_IFC1_COUNT_INCEN1_WR(Device, Mask);
    if (rc) return rc;

    return EIP164_NO_ERROR;
}


/*----------------------------------------------------------------------------
 * EIP164_Device_IFC1_Statistics_Enabled_Get
 */
EIP164_Error_t
EIP164_Device_IFC1_Statistics_Enabled_Get(
        const EIP164_IOArea_t * const IOArea_p,
        uint32_t * const Mask_p)
{
    Device_Handle_t Device;
    const volatile EIP164_True_IOArea_t * const TrueIOArea_p = IOAREA_IN(IOArea_p);
    int rc = 0;

    EIP164_CHECK_POINTER(IOArea_p);
    EIP164_CHECK_POINTER(Mask_p);

    Device = TrueIOArea_p->Device;

    rc = EIP164_IFC1_COUNT_INCEN1_RD(Device, Mask_p);
    if (rc) return rc;

    return EIP164_NO_ERROR;

}


/*----------------------------------------------------------------------------
 * EIP164_Device_RxCAM_Statistics_Enable
 */
EIP164_Error_t
EIP164_Device_RxCAM_Statistics_Enable(
        const EIP164_IOArea_t * const IOArea_p,
        const uint32_t Mask)
{
    Device_Handle_t Device;
    const volatile EIP164_True_IOArea_t * const TrueIOArea_p = IOAREA_IN(IOArea_p);
    int rc = 0;

    EIP164_CHECK_POINTER(IOArea_p);

    Device = TrueIOArea_p->Device;

    rc = EIP164_RXCAM_COUNT_INCEN1_WR(Device, Mask);
    if (rc) return rc;

    return EIP164_NO_ERROR;
}


/*----------------------------------------------------------------------------
 * EIP164_Device_RxCAM_Statistics_Enabled_Get
 */
EIP164_Error_t
EIP164_Device_RxCAM_Statistics_Enabled_Get(
        const EIP164_IOArea_t * const IOArea_p,
        uint32_t * const Mask_p)
{
    Device_Handle_t Device;
    const volatile EIP164_True_IOArea_t * const TrueIOArea_p = IOAREA_IN(IOArea_p);
    int rc = 0;

    EIP164_CHECK_POINTER(IOArea_p);
    EIP164_CHECK_POINTER(Mask_p);

    Device = TrueIOArea_p->Device;

    rc = EIP164_RXCAM_COUNT_INCEN1_RD(Device, Mask_p);
    if (rc) return rc;

    return EIP164_NO_ERROR;
}


/*----------------------------------------------------------------------------
 * EIP164_StatisticsControl_Update
 */
EIP164_Error_t
EIP164_StatisticsControl_Update(
    const EIP164_IOArea_t * const IOArea_p,
    const EIP164_Statistics_Control_t * const StatCtrl_p)
{
    unsigned int i;
    Device_Handle_t Device;
    const volatile EIP164_True_IOArea_t * const TrueIOArea_p =
                                                        IOAREA_IN(IOArea_p);
    int rc = 0;

    EIP164_CHECK_POINTER(IOArea_p);
    EIP164_CHECK_POINTER(StatCtrl_p);

    Device = TrueIOArea_p->Device;

    /* Program the sequence number threshold */
    for (i = 0; i < TrueIOArea_p->MaxChCount; i++)
    {
        rc = EIP66_SEQ_NR_THRESH_WR(Device, i, StatCtrl_p->SeqNrThreshold);
        if (rc) return rc;
    }

    for (i = 0; i < TrueIOArea_p->MaxChCount; i++)
    {
        rc = EIP66_SEQ_NR_THRESH_64_WR(Device,
                                       i,
                                       StatCtrl_p->SeqNrThreshold64.low,
                                       StatCtrl_p->SeqNrThreshold64.hi);
        if (rc) return rc;
    }

    /* Threshold for the frame counters */
    rc = EIP164_SA_COUNT_FRAME_THR_WR(Device,
                                      StatCtrl_p->SACountFrameThr.low,
                                      StatCtrl_p->SACountFrameThr.hi);
    if (rc) return rc;

    rc = EIP164_SECY_COUNT_FRAME_THR_WR(Device,
                                        StatCtrl_p->SecYCountFrameThr.low,
                                        StatCtrl_p->SecYCountFrameThr.hi);
    if (rc) return rc;

    rc = EIP164_IFC_COUNT_FRAME_THR_WR(Device,
                                       StatCtrl_p->IFCCountFrameThr.low,
                                       StatCtrl_p->IFCCountFrameThr.hi);
    if (rc) return rc;

    rc = EIP164_IFC1_COUNT_FRAME_THR_WR(Device,
                                        StatCtrl_p->IFC1CountFrameThr.low,
                                        StatCtrl_p->IFC1CountFrameThr.hi);
    if (rc) return rc;

    rc = EIP164_RXCAM_COUNT_FRAME_THR_WR(Device,
                                         StatCtrl_p->RxCAMCountFrameThr.low,
                                         StatCtrl_p->RxCAMCountFrameThr.hi);
    if (rc) return rc;

    /* Threshold for the octet counters */
    rc = EIP164_SA_COUNT_OCTET_THR_WR(Device,
                                      StatCtrl_p->SACountOctetThr.low,
                                      StatCtrl_p->SACountOctetThr.hi);
    if (rc) return rc;

    rc = EIP164_IFC_COUNT_OCTET_THR_WR(Device,
                                       StatCtrl_p->IFCCountOctetThr.low,
                                       StatCtrl_p->IFCCountOctetThr.hi);
    if (rc) return rc;

    rc = EIP164_IFC1_COUNT_OCTET_THR_WR(Device,
                                        StatCtrl_p->IFC1CountOctetThr.low,
                                        StatCtrl_p->IFC1CountOctetThr.hi);
    if (rc) return rc;

    /* Reset all statistics counters and threshold summary registers */
    rc = EIP164_SA_COUNT_CONTROL_WR(Device, true, false, false, false);
    if (rc) return rc;

    rc = EIP164_SECY_COUNT_CONTROL_WR(Device, true, false, false, false);
    if (rc) return rc;

    rc = EIP164_IFC_COUNT_CONTROL_WR(Device, true, false, false, false);
    if (rc) return rc;

    rc = EIP164_IFC1_COUNT_CONTROL_WR(Device, true, false, false, false);
    if (rc) return rc;

    rc = EIP164_RXCAM_COUNT_CONTROL_WR(Device, true, false, false, false);
    if (rc) return rc;

    /* Configure the statistics module to clear-on-read and saturating */
    rc = EIP164_SA_COUNT_CONTROL_WR(Device,
                               false,       /* Do not reset */
                               true,        /* Saturate counters */
                               StatCtrl_p->fAutoStatCntrsReset,
                               false);      /* Do not reset */
    if (rc) return rc;
    rc = EIP164_SECY_COUNT_CONTROL_WR(Device,
                                 false,     /* Do not reset */
                                 true,      /* Saturate counters */
                                 StatCtrl_p->fAutoStatCntrsReset,
                                 false);    /* Do not reset */
    if (rc) return rc;
    rc = EIP164_IFC_COUNT_CONTROL_WR(Device,
                                false,      /* Do not reset */
                                true,       /* Saturate counters */
                                StatCtrl_p->fAutoStatCntrsReset,
                                false);     /* Do not reset */
    if (rc) return rc;
    rc = EIP164_IFC1_COUNT_CONTROL_WR(Device,
                                 false,     /* Do not reset */
                                 true,      /* Saturate counters */
                                 StatCtrl_p->fAutoStatCntrsReset,
                                 false);    /* Do not reset */
    if (rc) return rc;

    rc = EIP164_RXCAM_COUNT_CONTROL_WR(Device,
                                  false,    /* Do not reset */
                                  true,     /* Saturate counters */
                                  StatCtrl_p->fAutoStatCntrsReset,
                                  false);   /* Do not reset */
    if (rc) return rc;

    return EIP164Lib_Device_Is_Init_Done(Device, TrueIOArea_p->fExternalRxCAM);
}


/*----------------------------------------------------------------------------
 * EIP164_StatisticsControl_Read
 */
EIP164_Error_t
EIP164_StatisticsControl_Read(
        const EIP164_IOArea_t * const IOArea_p,
        EIP164_Statistics_Control_t * const StatCtrl_p)
{
    Device_Handle_t Device;
    bool fResetAll,fSaturate,fResetSummary;
    const volatile EIP164_True_IOArea_t * const TrueIOArea_p =
                                                        IOAREA_IN(IOArea_p);
    int rc = 0;

    EIP164_CHECK_POINTER(IOArea_p);
    EIP164_CHECK_POINTER(StatCtrl_p);

    Device = TrueIOArea_p->Device;

    /* Read the sequence number threshold */
    rc = EIP66_SEQ_NR_THRESH_RD(Device, 0, &StatCtrl_p->SeqNrThreshold);
    if (rc) return rc;

    rc = EIP66_SEQ_NR_THRESH_64_RD(Device,
                                   0,
                                   &StatCtrl_p->SeqNrThreshold64.low,
                                   &StatCtrl_p->SeqNrThreshold64.hi);
    if (rc) return rc;

    /* Threshold for the frame counters */
    rc = EIP164_SA_COUNT_FRAME_THR_RD(Device,
                                      &StatCtrl_p->SACountFrameThr.low,
                                      &StatCtrl_p->SACountFrameThr.hi);
    if (rc) return rc;

    rc = EIP164_SECY_COUNT_FRAME_THR_RD(Device,
                                        &StatCtrl_p->SecYCountFrameThr.low,
                                        &StatCtrl_p->SecYCountFrameThr.hi);
    if (rc) return rc;

    rc = EIP164_IFC_COUNT_FRAME_THR_RD(Device,
                                       &StatCtrl_p->IFCCountFrameThr.low,
                                       &StatCtrl_p->IFCCountFrameThr.hi);
    if (rc) return rc;

    rc = EIP164_IFC1_COUNT_FRAME_THR_RD(Device,
                                        &StatCtrl_p->IFC1CountFrameThr.low,
                                        &StatCtrl_p->IFC1CountFrameThr.hi);
    if (rc) return rc;

    rc = EIP164_RXCAM_COUNT_FRAME_THR_RD(Device,
                                        &StatCtrl_p->RxCAMCountFrameThr.low,
                                        &StatCtrl_p->RxCAMCountFrameThr.hi);
    if (rc) return rc;

    /* Threshold for the octet counters */
    rc = EIP164_SA_COUNT_OCTET_THR_RD(Device,
                                      &StatCtrl_p->SACountOctetThr.low,
                                      &StatCtrl_p->SACountOctetThr.hi);
    if (rc) return rc;

    rc = EIP164_IFC_COUNT_OCTET_THR_RD(Device,
                                       &StatCtrl_p->IFCCountOctetThr.low,
                                       &StatCtrl_p->IFCCountOctetThr.hi);
    if (rc) return rc;

    rc = EIP164_IFC1_COUNT_OCTET_THR_RD(Device,
                                        &StatCtrl_p->IFC1CountOctetThr.low,
                                        &StatCtrl_p->IFC1CountOctetThr.hi);
    if (rc) return rc;

    /* Configure the statistics module to clear-on-read and saturating */
    rc = EIP164_SA_COUNT_CONTROL_RD(Device,
                                    &fResetAll,
                                    &fSaturate,
                                    &StatCtrl_p->fAutoStatCntrsReset,
                                    &fResetSummary);
    if (rc) return rc;

    return EIP164Lib_Device_Is_Init_Done(Device, TrueIOArea_p->fExternalRxCAM);
}

/*----------------------------------------------------------------------------
 * EIP164_Device_SAPnThrSummary_Read
 */
EIP164_Error_t
EIP164_Device_SAPnThrSummary_Read(
        const EIP164_IOArea_t * const IOArea_p,
        const unsigned int Index,
        uint32_t * const SAPnThrSummary_p)
{
    Device_Handle_t Device;
    const volatile EIP164_True_IOArea_t * const TrueIOArea_p = IOAREA_IN(IOArea_p);
    int rc = 0;

    EIP164_CHECK_POINTER(IOArea_p);
    EIP164_CHECK_INT_ATMOST(Index, ((TrueIOArea_p->MaxSACount + 31) / 32));

    Device = TrueIOArea_p->Device;

    rc = EIP164_SA_PN_THR_SUMMARY_RD(Device, Index, SAPnThrSummary_p);
    if (rc) return rc;

    return EIP164_NO_ERROR;
}


/*----------------------------------------------------------------------------
 * EIP164_Device_SAPnThrSummary_Clear
 */
EIP164_Error_t
EIP164_Device_SAPnThrSummary_Clear(
        const EIP164_IOArea_t * const IOArea_p,
        const unsigned int Index,
        const uint32_t SAPnThrSummary)
{
    Device_Handle_t Device;
    const volatile EIP164_True_IOArea_t * const TrueIOArea_p = IOAREA_IN(IOArea_p);
    int rc = 0;

    EIP164_CHECK_POINTER(IOArea_p);
    EIP164_CHECK_INT_ATMOST(Index, ((TrueIOArea_p->MaxSACount + 31) / 32));

    Device = TrueIOArea_p->Device;

    rc = EIP164_SA_PN_THR_SUMMARY_WR(Device, Index, SAPnThrSummary);
    if (rc) return rc;

    return EIP164_NO_ERROR;
}


/*----------------------------------------------------------------------------
 * EIP164_Device_SAExpiredSummary_Read
 */
EIP164_Error_t
EIP164_Device_SAExpiredSummary_Read(
        const EIP164_IOArea_t * const IOArea_p,
        const unsigned int Index,
        uint32_t * const SAExpiredSummary_p)
{
    Device_Handle_t Device;
    const volatile EIP164_True_IOArea_t * const TrueIOArea_p = IOAREA_IN(IOArea_p);
    int rc = 0;

    EIP164_CHECK_POINTER(IOArea_p);
    EIP164_CHECK_INT_ATMOST(Index, ((TrueIOArea_p->MaxSACount + 31) / 32));

    Device = TrueIOArea_p->Device;

    rc = EIP164_SA_EXP_SUMMARY_RD(Device, Index, SAExpiredSummary_p);
    if (rc) return rc;

    return EIP164_NO_ERROR;
}


/*----------------------------------------------------------------------------
 * EIP164_Device_SAExpiredSummary_Clear
 */
EIP164_Error_t
EIP164_Device_SAExpiredSummary_Clear(
        const EIP164_IOArea_t * const IOArea_p,
        const unsigned int Index,
        const uint32_t SAExpiredSummary)
{
    Device_Handle_t Device;
    const volatile EIP164_True_IOArea_t * const TrueIOArea_p = IOAREA_IN(IOArea_p);
    int rc = 0;

    EIP164_CHECK_POINTER(IOArea_p);
    EIP164_CHECK_INT_ATMOST(Index, ((TrueIOArea_p->MaxSACount + 31) / 32));

    Device = TrueIOArea_p->Device;

    rc = EIP164_SA_EXP_SUMMARY_WR(Device, Index, SAExpiredSummary);
    if (rc) return rc;

    return EIP164_NO_ERROR;
}


/*----------------------------------------------------------------------------
 * EIP164_Device_CountSummary_PSA_Read
 */
EIP164_Error_t
EIP164_Device_CountSummary_PSA_Read(
        const EIP164_IOArea_t * const IOArea_p,
        const unsigned int Index,
        uint32_t * const CountSummaryPSA_p)
{
    Device_Handle_t Device;
    const volatile EIP164_True_IOArea_t * const TrueIOArea_p = IOAREA_IN(IOArea_p);
    int rc = 0;

    EIP164_CHECK_POINTER(IOArea_p);
    EIP164_CHECK_INT_INRANGE(Index, 1, ((TrueIOArea_p->MaxSACount + 31) / 32));

    Device = TrueIOArea_p->Device;

    rc = EIP164_SA_COUNT_SUMMARY_P_RD(Device, Index, CountSummaryPSA_p);
    if (rc) return rc;

    return EIP164_NO_ERROR;
}


/*----------------------------------------------------------------------------
 * EIP164_Device_CountSummary_PSA_Clear
 */
EIP164_Error_t
EIP164_Device_CountSummary_PSA_Clear(
        const EIP164_IOArea_t * const IOArea_p,
        const unsigned int Index,
        const uint32_t CountSummaryPSA)
{
    Device_Handle_t Device;
    const volatile EIP164_True_IOArea_t * const TrueIOArea_p = IOAREA_IN(IOArea_p);
    int rc = 0;

    EIP164_CHECK_POINTER(IOArea_p);
    EIP164_CHECK_INT_INRANGE(Index, 1, ((TrueIOArea_p->MaxSACount + 31) / 32));

    Device = TrueIOArea_p->Device;

    rc = EIP164_SA_COUNT_SUMMARY_P_WR(Device, Index, CountSummaryPSA);
    if (rc) return rc;

    return EIP164_NO_ERROR;
}


#ifdef EIP164_PERCOUNTER_SUMMARY_ENABLE
/*----------------------------------------------------------------------------
 * EIP164_Device_CountSummary_SA_Read
 */
EIP164_Error_t
EIP164_Device_CountSummary_SA_Read(
        const EIP164_IOArea_t * const IOArea_p,
        const unsigned int Index,
        uint32_t * const CountSummarySA_p)
{
    Device_Handle_t Device;
    const volatile EIP164_True_IOArea_t * const TrueIOArea_p = IOAREA_IN(IOArea_p);
    int rc = 0;

    EIP164_CHECK_POINTER(IOArea_p);
    EIP164_CHECK_INT_ATMOST(Index, (TrueIOArea_p->MaxSACount - 1));

    Device = TrueIOArea_p->Device;

    rc = EIP164_SA_COUNT_SUMMARY_RD(Device, Index, CountSummarySA_p);
    if (rc) return rc;

    return EIP164_NO_ERROR;
}


/*----------------------------------------------------------------------------
 * EIP164_Device_CountSummary_SA_Clear
 */
EIP164_Error_t
EIP164_Device_CountSummary_SA_Clear(
        const EIP164_IOArea_t * const IOArea_p,
        const unsigned int Index,
        const uint32_t CountSummarySA)
{
    Device_Handle_t Device;
    const volatile EIP164_True_IOArea_t * const TrueIOArea_p = IOAREA_IN(IOArea_p);
    int rc = 0;

    EIP164_CHECK_POINTER(IOArea_p);
    EIP164_CHECK_INT_ATMOST(Index, (TrueIOArea_p->MaxSACount - 1));

    Device = TrueIOArea_p->Device;

    rc = EIP164_SA_COUNT_SUMMARY_WR(Device, Index, CountSummarySA);
    if (rc) return rc;

    return EIP164_NO_ERROR;
}
#endif

/*----------------------------------------------------------------------------
 * EIP164_Device_CountSummary_PSecY_Read
 */
EIP164_Error_t
EIP164_Device_CountSummary_PSecY_Read(
        const EIP164_IOArea_t * const IOArea_p,
        const unsigned int Index,
        uint32_t * const CountSummaryPSecY_p)
{
    Device_Handle_t Device;
    const volatile EIP164_True_IOArea_t * const TrueIOArea_p = IOAREA_IN(IOArea_p);
    int rc = 0;

    EIP164_CHECK_POINTER(IOArea_p);
    EIP164_CHECK_INT_INRANGE(Index, 1, ((TrueIOArea_p->MaxvPortCount + 31) / 32));

    Device = TrueIOArea_p->Device;

    rc = EIP164_SECY_COUNT_SUMMARY_P_RD(Device, Index, CountSummaryPSecY_p);
    if (rc) return rc;

    return EIP164_NO_ERROR;
}


/*----------------------------------------------------------------------------
 * EIP164_Device_CountSummary_PSecY_Clear
 */
EIP164_Error_t
EIP164_Device_CountSummary_PSecY_Clear(
        const EIP164_IOArea_t * const IOArea_p,
        const unsigned int Index,
        const uint32_t CountSummaryPSecY)
{
    Device_Handle_t Device;
    const volatile EIP164_True_IOArea_t * const TrueIOArea_p = IOAREA_IN(IOArea_p);
    int rc = 0;

    EIP164_CHECK_POINTER(IOArea_p);
    EIP164_CHECK_INT_INRANGE(Index, 1, ((TrueIOArea_p->MaxvPortCount + 31) / 32));

    Device = TrueIOArea_p->Device;

    rc = EIP164_SECY_COUNT_SUMMARY_P_WR(Device, Index, CountSummaryPSecY);
    if (rc) return rc;

    return EIP164_NO_ERROR;
}


#ifdef EIP164_PERCOUNTER_SUMMARY_ENABLE
/*----------------------------------------------------------------------------
 * EIP164_Device_CountSummary_SecY_Read
 */
EIP164_Error_t
EIP164_Device_CountSummary_SecY_Read(
        const EIP164_IOArea_t * const IOArea_p,
        const unsigned int Index,
        uint32_t * const CountSummarySecY_p)
{
    Device_Handle_t Device;
    const volatile EIP164_True_IOArea_t * const TrueIOArea_p = IOAREA_IN(IOArea_p);
    int rc = 0;

    EIP164_CHECK_POINTER(IOArea_p);
    EIP164_CHECK_INT_ATMOST(Index, (TrueIOArea_p->MaxvPortCount - 1));

    Device = TrueIOArea_p->Device;

    rc = EIP164_SECY_COUNT_SUMMARY_RD(Device, Index, CountSummarySecY_p);
    if (rc) return rc;

    return EIP164_NO_ERROR;
}


/*----------------------------------------------------------------------------
 * EIP164_Device_CountSummary_SecY_Clear
 */
EIP164_Error_t
EIP164_Device_CountSummary_SecY_Clear(
        const EIP164_IOArea_t * const IOArea_p,
        const unsigned int Index,
        const uint32_t CountSummarySecY)
{
    Device_Handle_t Device;
    const volatile EIP164_True_IOArea_t * const TrueIOArea_p = IOAREA_IN(IOArea_p);
    int rc = 0;

    EIP164_CHECK_POINTER(IOArea_p);
    EIP164_CHECK_INT_ATMOST(Index, (TrueIOArea_p->MaxvPortCount - 1));

    Device = TrueIOArea_p->Device;

    rc = EIP164_SECY_COUNT_SUMMARY_WR(Device, Index, CountSummarySecY);
    if (rc) return rc;

    return EIP164_NO_ERROR;
}
#endif

/*----------------------------------------------------------------------------
 * EIP164_Device_CountSummary_PIFC_Read
 */
EIP164_Error_t
EIP164_Device_CountSummary_PIFC_Read(
        const EIP164_IOArea_t * const IOArea_p,
        const unsigned int Index,
        uint32_t * const CountSummaryPIFC_p)
{
    Device_Handle_t Device;
    const volatile EIP164_True_IOArea_t * const TrueIOArea_p = IOAREA_IN(IOArea_p);
    int rc = 0;

    EIP164_CHECK_POINTER(IOArea_p);
    EIP164_CHECK_INT_INRANGE(Index, 1, ((TrueIOArea_p->MaxvPortCount + 31) / 32));

    Device = TrueIOArea_p->Device;

    rc = EIP164_IFC_COUNT_SUMMARY_P_RD(Device, Index, CountSummaryPIFC_p);
    if (rc) return rc;

    return EIP164_NO_ERROR;
}


/*----------------------------------------------------------------------------
 * EIP164_Device_CountSummary_PIFC_Clear
 */
EIP164_Error_t
EIP164_Device_CountSummary_PIFC_Clear(
        const EIP164_IOArea_t * const IOArea_p,
        const unsigned int Index,
        const uint32_t CountSummaryPIFC)
{
    Device_Handle_t Device;
    const volatile EIP164_True_IOArea_t * const TrueIOArea_p = IOAREA_IN(IOArea_p);
    int rc = 0;

    EIP164_CHECK_POINTER(IOArea_p);
    EIP164_CHECK_INT_INRANGE(Index, 1, ((TrueIOArea_p->MaxvPortCount + 31) / 32));

    Device = TrueIOArea_p->Device;

    rc = EIP164_IFC_COUNT_SUMMARY_P_WR(Device, Index, CountSummaryPIFC);
    if (rc) return rc;

    return EIP164_NO_ERROR;
}


#ifdef EIP164_PERCOUNTER_SUMMARY_ENABLE
/*----------------------------------------------------------------------------
 * EIP164_Device_CountSummary_IFC_Read
 */
EIP164_Error_t
EIP164_Device_CountSummary_IFC_Read(
        const EIP164_IOArea_t * const IOArea_p,
        const unsigned int Index,
        uint32_t * const CountSummaryIFC_p)
{
    Device_Handle_t Device;
    const volatile EIP164_True_IOArea_t * const TrueIOArea_p = IOAREA_IN(IOArea_p);
    int rc = 0;

    EIP164_CHECK_POINTER(IOArea_p);
    EIP164_CHECK_INT_ATMOST(Index, (TrueIOArea_p->MaxvPortCount - 1));

    Device = TrueIOArea_p->Device;

    rc = EIP164_IFC_COUNT_SUMMARY_RD(Device, Index, CountSummaryIFC_p);
    if (rc) return rc;

    return EIP164_NO_ERROR;
}


/*----------------------------------------------------------------------------
 * EIP164_Device_CountSummary_IFC_Clear
 */
EIP164_Error_t
EIP164_Device_CountSummary_IFC_Clear(
        const EIP164_IOArea_t * const IOArea_p,
        const unsigned int Index,
        const uint32_t CountSummaryIFC)
{
    Device_Handle_t Device;
    const volatile EIP164_True_IOArea_t * const TrueIOArea_p = IOAREA_IN(IOArea_p);
    int rc = 0;

    EIP164_CHECK_POINTER(IOArea_p);
    EIP164_CHECK_INT_ATMOST(Index, (TrueIOArea_p->MaxvPortCount - 1));

    Device = TrueIOArea_p->Device;

    rc = EIP164_IFC_COUNT_SUMMARY_WR(Device, Index, CountSummaryIFC);
    if (rc) return rc;

    return EIP164_NO_ERROR;
}
#endif

/*----------------------------------------------------------------------------
 * EIP164_Device_CountSummary_PIFC1_Read
 */
EIP164_Error_t
EIP164_Device_CountSummary_PIFC1_Read(
        const EIP164_IOArea_t * const IOArea_p,
        const unsigned int Index,
        uint32_t * const CountSummaryPIFC1_p)
{
    Device_Handle_t Device;
    const volatile EIP164_True_IOArea_t * const TrueIOArea_p = IOAREA_IN(IOArea_p);
    int rc = 0;

    EIP164_CHECK_POINTER(IOArea_p);
    EIP164_CHECK_INT_ATMOST(Index, ((TrueIOArea_p->MaxvPortCount + 31) / 32));

    Device = TrueIOArea_p->Device;

    rc = EIP164_IFC1_COUNT_SUMMARY_P_RD(Device, Index, CountSummaryPIFC1_p);
    if (rc) return rc;

    return EIP164_NO_ERROR;
}


/*----------------------------------------------------------------------------
 * EIP164_Device_CountSummary_PIFC1_Clear
 */
EIP164_Error_t
EIP164_Device_CountSummary_PIFC1_Clear(
        const EIP164_IOArea_t * const IOArea_p,
        const unsigned int Index,
        const uint32_t CountSummaryPIFC1)
{
    Device_Handle_t Device;
    const volatile EIP164_True_IOArea_t * const TrueIOArea_p = IOAREA_IN(IOArea_p);
    int rc = 0;

    EIP164_CHECK_POINTER(IOArea_p);
    EIP164_CHECK_INT_INRANGE(Index, 1, ((TrueIOArea_p->MaxvPortCount + 31) / 32));

    Device = TrueIOArea_p->Device;

    rc = EIP164_IFC1_COUNT_SUMMARY_P_WR(Device, Index, CountSummaryPIFC1);
    if (rc) return rc;

    return EIP164_NO_ERROR;
}


#ifdef EIP164_PERCOUNTER_SUMMARY_ENABLE
/*----------------------------------------------------------------------------
 * EIP164_Device_CountSummary_IFC1_Read
 */
EIP164_Error_t
EIP164_Device_CountSummary_IFC1_Read(
        const EIP164_IOArea_t * const IOArea_p,
        const unsigned int Index,
        uint32_t * const CountSummaryIFC1_p)
{
    Device_Handle_t Device;
    const volatile EIP164_True_IOArea_t * const TrueIOArea_p = IOAREA_IN(IOArea_p);
    int rc = 0;

    EIP164_CHECK_POINTER(IOArea_p);
    EIP164_CHECK_INT_ATMOST(Index, (TrueIOArea_p->MaxvPortCount - 1));

    Device = TrueIOArea_p->Device;

    rc = EIP164_IFC1_COUNT_SUMMARY_RD(Device, Index, CountSummaryIFC1_p);
    if (rc) return rc;

    return EIP164_NO_ERROR;
}

/*----------------------------------------------------------------------------
 * EIP164_Device_CountSummary_IFC1_Clear
 */
EIP164_Error_t
EIP164_Device_CountSummary_IFC1_Clear(
        const EIP164_IOArea_t * const IOArea_p,
        const unsigned int Index,
        const uint32_t CountSummaryIFC1)
{
    Device_Handle_t Device;
    const volatile EIP164_True_IOArea_t * const TrueIOArea_p = IOAREA_IN(IOArea_p);
    int rc = 0;

    EIP164_CHECK_POINTER(IOArea_p);
    EIP164_CHECK_INT_ATMOST(Index, (TrueIOArea_p->MaxvPortCount - 1));

    Device = TrueIOArea_p->Device;

    rc = EIP164_IFC1_COUNT_SUMMARY_WR(Device, Index, CountSummaryIFC1);
    if (rc) return rc;

    return EIP164_NO_ERROR;
}
#endif

/*----------------------------------------------------------------------------
 * EIP164_Device_CountSummary_PRxCAM_Read
 */
EIP164_Error_t
EIP164_Device_CountSummary_PRxCAM_Read(
        const EIP164_IOArea_t * const IOArea_p,
        const unsigned int Index,
        uint32_t * const CountSummary_p)
{
    Device_Handle_t Device;
    const volatile EIP164_True_IOArea_t * const TrueIOArea_p = IOAREA_IN(IOArea_p);
    int rc = 0;

    EIP164_CHECK_POINTER(IOArea_p);
    EIP164_CHECK_INT_INRANGE(Index, 1, ((TrueIOArea_p->MaxSCCount + 31) / 32));

    Device = TrueIOArea_p->Device;

    rc = EIP164_RXCAM_COUNT_SUMMARY_P_RD(Device, Index, CountSummary_p);
    if (rc) return rc;

    return EIP164_NO_ERROR;
}


/*----------------------------------------------------------------------------
 * EIP164_Device_CountSummary_PRxCAM_Clear
 */
EIP164_Error_t
EIP164_Device_CountSummary_PRxCAM_Clear(
        const EIP164_IOArea_t * const IOArea_p,
        const unsigned int Index,
        const uint32_t CountSummary)
{
    Device_Handle_t Device;
    const volatile EIP164_True_IOArea_t * const TrueIOArea_p = IOAREA_IN(IOArea_p);
    int rc = 0;

    EIP164_CHECK_POINTER(IOArea_p);
    EIP164_CHECK_INT_INRANGE(Index, 1, ((TrueIOArea_p->MaxSCCount + 31) / 32));

    Device = TrueIOArea_p->Device;

    rc = EIP164_RXCAM_COUNT_SUMMARY_P_WR(Device, Index, CountSummary);
    if (rc) return rc;

    return EIP164_NO_ERROR;
}


/* end of file eip164_device.c */

