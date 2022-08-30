/* adapter_secy.c
 *
 * SecY API and Adapter internal SecY API implementation
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

#include <Integration/Adapter_EIP164/incl/api_secy.h>      /* SecY API */


/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

/* Top-level Adapter configuration */
#include <Integration/Adapter_EIP164/incl/c_adapter_eip164.h>             /* ADAPTER_EIP164_DRIVER_NAME */

#include <Integration/Adapter_EIP164/incl/adapter_secy_support.h>  /* Adapter internal SecY API */
#ifdef ADAPTER_EIP164_WARMBOOT_ENABLE
#include <Integration/Adapter_EIP164/incl/adapter_secy_warmboot.h>
#endif

/* Logging API */
#include <Kit/Log/incl/log.h>                /* LOG_* */

/* Driver Framework Device API */
#include <Kit/DriverFramework/incl/device_types.h>
#include <Kit/DriverFramework/incl/device_mgmt.h>

/* Driver Framework C Library Abstraction API */
#include <Kit/DriverFramework/incl/clib.h>               /* ZEROINIT() */

/* EIP-164 Driver Library SecY API */
#include <Kit/EIP164/incl/eip164_types.h>
#include <Kit/EIP164/incl/eip164_secy.h>
#include <Kit/EIP164/incl/eip164_rules.h>

/* Adapter Sleep API */
#include <Integration/Adapter_EIP164/incl/adapter_sleep.h>      /* Adapter_SleepMS() */

/* Adapter Alloc API */
#include <Integration/Adapter_EIP164/incl/adapter_alloc.h>      /* Adapter_Alloc()/_Free() */

/* Adapter Interrupts API */
#include <Integration/Adapter_EIP164/incl/adapter_interrupts.h>

/* List API */
#include <Kit/List/incl/list.h>


/*----------------------------------------------------------------------------
 * Definitions and macros
 */

#define EIP164_SECY_ALL_CLOCKS (EIP164_SECY_DEVICE_PE_CLOCK | \
                                EIP164_SECY_DEVICE_CE_CLOCK | \
                                EIP164_SECY_DEVICE_IPB_CLOCK | \
                                EIP164_SECY_DEVICE_RXCAM_CLOCK | \
                                EIP164_SECY_DEVICE_PE_AES_KS_CLOCK | \
                                EIP164_SECY_DEVICE_PE_AES_CLOCK | \
                                EIP164_SECY_DEVICE_PE_GHASH_CLOCK | \
                                EIP164_SECY_DEVICE_PE_HKEY_CLOCK | \
                                EIP164_SECY_DEVICE_PE_DPATH_CLOCK)


#if (LOG_SEVERITY_MAX != LOG_SEVERITY_NO_OUTPUT)
/*----------------------------------------------------------------------------
 * YesNo
 */
static const char *
YesNo(
        const bool b)
{
    if (b)
        return "Yes";
    else
        return "No";
}
#endif


/*----------------------------------------------------------------------------
 * SecYLib_NonSA_Rules_Copy
 */
static inline void
SecYLib_NonSA_Rules_Copy(
        const SecY_Rules_NonSA_t * const Src_p,
        EIP164_Rules_NonSA_t * const Dst_p)
{
    Dst_p->DestPort         = (EIP164_PortType_t)Src_p->DestPort;
    Dst_p->DropType         = (EIP164_DropType_t)Src_p->DropType;
    Dst_p->fBypass          = Src_p->fBypass;
    Dst_p->fDropNonReserved = Src_p->fDropNonReserved;
}


/*----------------------------------------------------------------------------
 * SecYLib_Channel_Config_Set
 */
static SecY_Status_t
SecYLib_Channel_Config_Set(
        const unsigned int DeviceId,
        const SecY_ChannelConf_t * const ChannelConfig_p)
{
    EIP164_Error_t Rc;

    /* Configure individual channels */
    if (ChannelConfig_p->Params.ChannelCount)
    {
        unsigned int i;
        unsigned int ChannelCount = ChannelConfig_p->Params.ChannelCount;

        EIP164_Channel_Rule_SecTAG_t Dev_SecTag_Rules;
        EIP164_PN_Threshold_t PN_Threshold_Cfg;

        /* Configure each channel, */
        /* channel indices (ChannelId) must correspond to the new channel map */
        for (i = 0; i < ChannelCount; i++)
        {
            SecY_Channel_t * ch_p = &ChannelConfig_p->Params.Channel_p[i];

            LOG_INFO("\n\t EIP164_Device_ChannelControl_Write \n");

            /* Enable or disable low latency bypass */
            Rc = EIP164_Device_ChannelControl_Write(
                                        &PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->IOArea,
                                        ch_p->ChannelId,
                                        ch_p->Latency,
                                        ch_p->BurstLimit,
                                        ch_p->fLatencyEnable,
                                        ch_p->fLowLatencyBypass);

            if (Rc != EIP164_NO_ERROR)
                return SECY_DLIB_ERROR(Rc);

            /* Skip MACsec-specific configuration for non-MACsec bypass channel */
            if (ch_p->fLowLatencyBypass)
                continue;

            ZEROINIT(PN_Threshold_Cfg);

            PN_Threshold_Cfg.SeqNrThreshold       = ch_p->StatCtrl.SeqNrThreshold;
            PN_Threshold_Cfg.SeqNrThreshold64.low =
                                    ch_p->StatCtrl.SeqNrThreshold64.Lo;
            PN_Threshold_Cfg.SeqNrThreshold64.hi  =
                                    ch_p->StatCtrl.SeqNrThreshold64.Hi;

            LOG_INFO("\n\t EIP164_Device_PN_Threshold_Control_Write \n");

            /* Configure SA sequence numbers check thresholds */
            Rc = EIP164_Device_PN_Threshold_Control_Write(
                                                &PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->IOArea,
                                                ch_p->ChannelId,
                                                &PN_Threshold_Cfg);
            if (Rc != EIP164_NO_ERROR)
                return SECY_DLIB_ERROR(Rc);

            LOG_INFO("\n\t EIP164_Device_Channel_CtxControl_Write \n");

            /* Configure SA sequence number threshold check type and */
            /* EtherType */
            Rc = EIP164_Device_Channel_CtxControl_Write(
                                                &PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->IOArea,
                                                ch_p->ChannelId,
                                                ch_p->fPktNumThrMode,
                                                ch_p->EtherType);

            if (Rc != EIP164_NO_ERROR)
                return SECY_DLIB_ERROR(Rc);

            ZEROINIT(Dev_SecTag_Rules);

            Dev_SecTag_Rules.EtherType       = ch_p->RuleSecTAG.EtherType;
            Dev_SecTag_Rules.fCheckCE        = ch_p->RuleSecTAG.fCheckCE;
            Dev_SecTag_Rules.fCheckKay       = ch_p->RuleSecTAG.fCheckKay;
            Dev_SecTag_Rules.fCheckPN        = ch_p->RuleSecTAG.fCheckPN;
            Dev_SecTag_Rules.fCheckSC        = ch_p->RuleSecTAG.fCheckSC;
            Dev_SecTag_Rules.fCheckSL        = ch_p->RuleSecTAG.fCheckSL;
            Dev_SecTag_Rules.fCheckV         = ch_p->RuleSecTAG.fCheckV;
            Dev_SecTag_Rules.fCheckSLExt     = false;
            Dev_SecTag_Rules.fCompEType      = ch_p->RuleSecTAG.fCompEType;

            LOG_INFO("\n\t EIP164_Rules_SecTag_Update \n");

            /* Configure SecTAG parsing and checking rules */
            Rc = EIP164_Rules_SecTag_Update(&PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->IOArea,
                                            ch_p->ChannelId,
                                            &Dev_SecTag_Rules);
            if (Rc != EIP164_NO_ERROR)
                return SECY_DLIB_ERROR(Rc);
        } /* for */
    }

    return SECY_STATUS_OK;
}


/*----------------------------------------------------------------------------
 * SecYLib_Channel_Config_Get
 */
static SecY_Status_t
SecYLib_Channel_Config_Get(
        const unsigned int DeviceId,
        const unsigned int ChannelId,
        SecY_Channel_t * const ChannelConfig_p)
{
    EIP164_Error_t Rc;
    EIP164_Channel_Rule_SecTAG_t Dev_SecTag_Rules;
    EIP164_PN_Threshold_t PN_Threshold_Cfg;

    ChannelConfig_p->ChannelId = ChannelId;

    LOG_INFO("\n\t EIP164_Device_ChannelControl_Read \n");

    /* Enable or disable low latency bypass */
    Rc = EIP164_Device_ChannelControl_Read(&PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->IOArea,
                                           ChannelId,
                                           &ChannelConfig_p->Latency,
                                           &ChannelConfig_p->BurstLimit,
                                           &ChannelConfig_p->fLatencyEnable,
                                           &ChannelConfig_p->fLowLatencyBypass);

    if (Rc != EIP164_NO_ERROR)
    {
        return SECY_DLIB_ERROR(Rc);
    }

    LOG_INFO("\n\t EIP164_Rules_SecTag_Read \n");

    /* Read the per-channel SecTAG rule. */
    Rc = EIP164_Rules_SecTag_Read(&PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->IOArea,
                                  ChannelId,
                                  &Dev_SecTag_Rules);
    if (Rc != EIP164_NO_ERROR)
    {
        return SECY_DLIB_ERROR(Rc);
    }

    ChannelConfig_p->RuleSecTAG.EtherType = Dev_SecTag_Rules.EtherType;
    ChannelConfig_p->RuleSecTAG.fCompEType = Dev_SecTag_Rules.fCompEType;
    ChannelConfig_p->RuleSecTAG.fCheckCE = Dev_SecTag_Rules.fCheckCE;
    ChannelConfig_p->RuleSecTAG.fCheckKay = Dev_SecTag_Rules.fCheckKay;
    ChannelConfig_p->RuleSecTAG.fCheckPN = Dev_SecTag_Rules.fCheckPN;
    ChannelConfig_p->RuleSecTAG.fCheckSC = Dev_SecTag_Rules.fCheckSC;
    ChannelConfig_p->RuleSecTAG.fCheckSL = Dev_SecTag_Rules.fCheckSL;
    ChannelConfig_p->RuleSecTAG.fCheckV = Dev_SecTag_Rules.fCheckV;

    /* Get per channel context control. */
    Rc = EIP164_Device_Channel_CtxControl_Read(&PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->IOArea,
                                               ChannelId,
                                               &ChannelConfig_p->fPktNumThrMode,
                                               &ChannelConfig_p->EtherType);
    if (Rc != EIP164_NO_ERROR)
    {
        return SECY_DLIB_ERROR(Rc);
    }

    /* Get the statistics control settings. */
    Rc = EIP164_Device_PN_Threshold_Control_Read(
        &PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->IOArea,
        ChannelId,
        &PN_Threshold_Cfg);

    if (Rc != EIP164_NO_ERROR)
    {
        return SECY_DLIB_ERROR(Rc);
    }

    ChannelConfig_p->StatCtrl.SeqNrThreshold = PN_Threshold_Cfg.SeqNrThreshold;
    ChannelConfig_p->StatCtrl.SeqNrThreshold64.Lo = PN_Threshold_Cfg.SeqNrThreshold64.low;
    ChannelConfig_p->StatCtrl.SeqNrThreshold64.Hi = PN_Threshold_Cfg.SeqNrThreshold64.hi;

    return SECY_STATUS_OK;
}


/*----------------------------------------------------------------------------
 * SecY_Device_Init
 */
SecY_Status_t
SecY_Device_Init(
        const unsigned int DeviceId,
        const SecY_Role_t Role,
        const SecY_Settings_t * const Settings_p)
{
    unsigned int i;
    SecY_Status_t SecY_Rc = SECY_STATUS_OK;
    Device_Handle_t Device;
    EIP164_Error_t Rc;
    EIP164_Settings_t Settings;
    EIP164_Capabilities_t Cp;

    LOG_INFO("\n %s \n", __func__);

#ifdef ADAPTER_EIP164_STRICT_ARGS
    if (DeviceId >= ADAPTER_EIP164_MAX_NOF_DEVICES)
        return SECY_ERROR_BAD_PARAMETER;

    if (Settings_p == NULL)
        return SECY_ERROR_BAD_PARAMETER;
#endif /* ADAPTER_EIP164_STRICT_ARGS */

    SecYLib_Device_Lock(DeviceId);

    /* Check if the device has been already initialized */
    if (PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device_StaticFields)[DeviceId].fInitialized)
    {
        LOG_WARN("%s: EIP-164 device already initialized for device %d\n",
                 __func__, DeviceId);
        SecYLib_Device_Unlock(DeviceId);
        return SECY_STATUS_OK;
    }

    ADAPTER_EIP164_DEVICE_NAME(DeviceId) =
                    Device_GetName(ADAPTER_EIP164_DEV_ID(DeviceId));

    /* Find the EIP-164 device */
    Device = Device_Find(ADAPTER_EIP164_DEVICE_NAME(DeviceId));
    if (Device == NULL)
    {
        LOG_CRIT("%s: Failed to locate EIP-164 device for device %d (%s)\n",
                 __func__, DeviceId, ADAPTER_EIP164_DEVICE_NAME(DeviceId));
        SecYLib_Device_Unlock(DeviceId);
        return SECY_ERROR_INTERNAL;
    }

    PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_SET(SecY_Device[DeviceId],Adapter_Alloc(sizeof(SecY_Device_t)));
    if (PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId] == NULL)
    {
        LOG_CRIT("%s: Failed to allocate data for SecY device %d (%s)\n",
                 __func__, DeviceId, ADAPTER_EIP164_DEVICE_NAME(DeviceId));
        SecYLib_Device_Unlock(DeviceId);
        return SECY_ERROR_INTERNAL;
    }

    ZEROINIT(Settings);

    switch (Role)
    {
    case SECY_ROLE_EGRESS:
        Settings.Mode = EIP164_MODE_EGRESS;
        break;
    case SECY_ROLE_INGRESS:
        Settings.Mode = EIP164_MODE_INGRESS;
        break;
    case SECY_ROLE_EGRESS_INGRESS:
    default:
        /* Assume Egress-Ingress */
        Settings.Mode = EIP164_MODE_EGRESS_INGRESS;
        break;
    }

    Settings.SeqNrThreshold     = Settings_p->SeqNrThreshold;
    Settings.SeqNrThreshold64Lo = Settings_p->SeqNrThreshold64Lo;
    Settings.SeqNrThreshold64Hi = Settings_p->SeqNrThreshold64Hi;

    Settings.SACountFrameThrLo    = Settings_p->SACountFrameThrLo;
    Settings.SACountFrameThrHi    = Settings_p->SACountFrameThrHi;
    Settings.SecYCountFrameThrLo  = Settings_p->SecYCountFrameThrLo;
    Settings.SecYCountFrameThrHi  = Settings_p->SecYCountFrameThrHi;
    Settings.IFCCountFrameThrLo   = Settings_p->IFCCountFrameThrLo;
    Settings.IFCCountFrameThrHi   = Settings_p->IFCCountFrameThrHi;
    Settings.IFC1CountFrameThrLo  = Settings_p->IFC1CountFrameThrLo;
    Settings.IFC1CountFrameThrHi  = Settings_p->IFC1CountFrameThrHi;
    Settings.RxCAMCountFrameThrLo = Settings_p->RxCAMCountFrameThrLo;
    Settings.RxCAMCountFrameThrHi = Settings_p->RxCAMCountFrameThrHi;

    Settings.SACountOctetThrLo   = Settings_p->SACountOctetThrLo;
    Settings.SACountOctetThrHi   = Settings_p->SACountOctetThrHi;
    Settings.IFCCountOctetThrLo  = Settings_p->IFCCountOctetThrLo;
    Settings.IFCCountOctetThrHi  = Settings_p->IFCCountOctetThrHi;
    Settings.IFC1CountOctetThrLo = Settings_p->IFC1CountOctetThrLo;
    Settings.IFC1CountOctetThrHi = Settings_p->IFC1CountOctetThrHi;

    Settings.EOPTimeoutVal = Settings_p->EOPTimeoutVal;

    cpssOsMemCpy(&Settings.EOPTimeoutCtrl, &Settings_p->EOPTimeoutCtrl, sizeof(SecY_Ch_Mask_t));

    Settings.ECCCorrectableThr = Settings_p->ECCCorrectableThr;
    Settings.ECCUncorrectableThr = Settings_p->ECCUncorrectableThr;

    Settings.CountIncDisCtrl     = Settings_p->CountIncDisCtrl;

    LOG_CRIT("%s device statistics counter increment enable settings:\n"
             "\tSA counters: %s\n"
             "\tIFC counters: %s\n"
             "\tIFC1 counters: %s\n"
             "\tSecY counters: %s\n"
             "\tRxCAM counters: %s\n"
             "\tChannel counters: %s\n",
             ADAPTER_EIP164_DEVICE_NAME(DeviceId),
             (Settings_p->CountIncDisCtrl & SECY_SA_COUNT_INC_DIS) ?
                     "Disabled" : "Enabled",
             (Settings_p->CountIncDisCtrl & SECY_IFC_COUNT_INC_DIS) ?
                     "Disabled" : "Enabled",
             (Settings_p->CountIncDisCtrl & SECY_IFC1_COUNT_INC_DIS) ?
                     "Disabled" : "Enabled",
             (Settings_p->CountIncDisCtrl & SECY_SECY_COUNT_INC_DIS) ?
                     "Disabled" : "Enabled",
             (Settings_p->CountIncDisCtrl & SECY_RXCAM_COUNT_INC_DIS) ?
                     "Disabled" : "Enabled",
             (Settings_p->CountIncDisCtrl & SECY_CHAN_COUNT_INC_DIS) ?
                     "Disabled" : "Enabled");

    Settings.SA_SecFail_Mask     = ADAPTER_EIP164_SA_SECFAIL_MASK;
    Settings.SecY_SecFail_Mask   = ADAPTER_EIP164_SECY_SECFAIL_MASK;

    Settings.fAutoStatCntrsReset = ADAPTER_EIP164_CLEAR_ON_READ;
    Settings.Latency             = ADAPTER_EIP164_FIXED_LATENCY;
    Settings.fStaticBypass       = ADAPTER_EIP164_STATIC_BYPASS;

    /* Copy the default processing flow rules */
    Settings.fFlowCyptAuth =  Settings_p->fFlowCyptAuth;
    SecYLib_NonSA_Rules_Copy(&Settings_p->DropBypass, &Settings.DropBypass);

    LOG_INFO("\n\t EIP164_Device_Init \n");

    /* Request the device initialization is done */
    Rc = EIP164_Device_Init(&PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->IOArea, Device, &Settings);
    if ((Rc != EIP164_NO_ERROR) && (Rc != EIP164_BUSY_RETRY_LATER))
    {
        LOG_CRIT("%s: Failed to init EIP-164 device for device %d (%s), "
                 "mode %s, error %d\n",
                 __func__, DeviceId, ADAPTER_EIP164_DEVICE_NAME(DeviceId),
                 (Settings.Mode == EIP164_MODE_EGRESS) ? "egress" : "ingress",
                 Rc);
        Adapter_Free(PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]);
        PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_SET(SecY_Device[DeviceId],NULL);

        SecYLib_Device_Unlock(DeviceId);
        return SECY_DLIB_ERROR(Rc);
    }

    /* Wait till the device initialization is done */
    if (Rc == EIP164_BUSY_RETRY_LATER)
    {
        unsigned int LoopCounter = ADAPTER_EIP164_MAX_NOF_INIT_RETRY_COUNT;

        do
        {
            LOG_INFO("\n\t EIP164_Device_Is_Init_Done \n");

            Rc = EIP164_Device_Is_Init_Done(&PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->IOArea);
            if (Rc != EIP164_BUSY_RETRY_LATER)
            {
                break;
            }

            Adapter_SleepMS(ADAPTER_EIP164_INIT_RETRY_TIMEOUT_MS);
        } while (--LoopCounter);

        if (Rc != EIP164_NO_ERROR)
        {
            LOG_CRIT("%s: Failed to complete EIP-164 device initialization "
                     "for device %d\n",
                     __func__, DeviceId);
            Adapter_Free(PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]);
            PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_SET(SecY_Device[DeviceId],NULL);
            SecYLib_Device_Unlock(DeviceId);
            return SECY_DLIB_ERROR(Rc);
        }
    }

    ZEROINIT(Cp);

    LOG_INFO("\n\t EIP164_HWRevision_Get \n");

    /* Retrieve device capabilities */
    Rc = EIP164_HWRevision_Get(Device, &Cp);
    if (Rc != EIP164_NO_ERROR)
    {
        LOG_CRIT("%s: Failed to get EIP-164 device capabilities for device %d,"
                 " error %d\n",
                 __func__, DeviceId, Rc);
        Adapter_Free(PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]);
        PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_SET(SecY_Device[DeviceId],NULL);
        SecYLib_Device_Unlock(DeviceId);
        return SECY_DLIB_ERROR(Rc);
    }

    /* Show those capabilities not propagated to higher layer. */
    LOG_CRIT("%s device version major/minor/patch: %d/%d/%d\n",
              ADAPTER_EIP164_DEVICE_NAME(DeviceId),
              Cp.EIP164_Version.MajHWRevision,
              Cp.EIP164_Version.MinHWRevision,
              Cp.EIP164_Version.HWPatchLevel);

    LOG_CRIT("%s device capabilities\n", ADAPTER_EIP164_DEVICE_NAME(DeviceId));

    LOG_CRIT("\tEgress only:               %s\n"
             "\tIngress only:              %s\n"
             "\tSpare register:            %s\n"
             "\tExternal RxCAM:            %s\n"
             "\tNof SA's:                  %d\n"
             "\tNof SC's:                  %d\n"
             "\tNof channels:              %d\n"
             "\tNof vPorts:                %d\n",
             YesNo(Cp.EIP164_Options.fEgressOnly),
             YesNo(Cp.EIP164_Options.fIngressOnly),
             YesNo(Cp.EIP164_Options.fSpareReg),
             YesNo(Cp.EIP164_Options.fExternalRxCAM),
             Cp.EIP164_Options.SA_Count,
             Cp.EIP164_Options.SC_Count,
             Cp.EIP164_Options.Channel_Count,
             Cp.EIP164_Options.vPort_Count);

    LOG_CRIT("%s device (EIP-66) version major/minor/patch: %d/%d/%d\n",
              ADAPTER_EIP164_DEVICE_NAME(DeviceId),
              Cp.EIP66_Version.MajHWRevision,
              Cp.EIP66_Version.MinHWRevision,
              Cp.EIP66_Version.HWPatchLevel);

    LOG_CRIT("%s device (EIP-66) capabilities\n",
             ADAPTER_EIP164_DEVICE_NAME(DeviceId));

    LOG_CRIT("\tEgress only:               %s\n"
             "\tIngress only:              %s\n"
             "\tNumber of channels:        %d\n"
             "\tNumber of pipes:           %d\n",
             YesNo(Cp.EIP66_Options.fEgressOnly),
             YesNo(Cp.EIP66_Options.fIngressOnly),
             Cp.EIP66_Options.ChannelCount,
             Cp.EIP66_Options.PipeCount);

#ifndef ADAPTER_EIP164_DEV_INIT_DISABLE
    /* These clock control functions can be used to fine-grained */
    /* clock control, for now only OFF/ON sequence is done */

    LOG_INFO("\n\t EIP164_Device_Clocks_Off \n");

    /* Clock control, disable all EIP-164 clock signals */
    Rc = EIP164_Device_Clocks_Off(&PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->IOArea,
                             EIP164_SECY_ALL_CLOCKS);
    if (Rc != EIP164_NO_ERROR)
    {
        Adapter_Free(PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]);
        PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_SET(SecY_Device[DeviceId],NULL);
        SecYLib_Device_Unlock(DeviceId);
        return SECY_DLIB_ERROR(Rc);
    }

    LOG_INFO("\n\t EIP164_Device_Clocks_Off \n");

    /* Clock control, restore all EIP-164 clock signals, */
    /* e.g. go back to the dynamic clock control */
    Rc = EIP164_Device_Clocks_Off(&PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->IOArea, 0);
    if (Rc != EIP164_NO_ERROR)
    {
        Adapter_Free(PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]);
        PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_SET(SecY_Device[DeviceId],NULL);
        SecYLib_Device_Unlock(DeviceId);
        return SECY_DLIB_ERROR(Rc);
    }

    LOG_INFO("\n\t EIP164_Device_Clocks_On \n");

    /* Clock control, enable all EIP-164 clock signals */
    Rc = EIP164_Device_Clocks_On(&PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->IOArea,
                             EIP164_SECY_ALL_CLOCKS,
                             1);

    if (Rc != EIP164_NO_ERROR)
    {
        Adapter_Free(PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]);
        PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_SET(SecY_Device[DeviceId],NULL);
        SecYLib_Device_Unlock(DeviceId);
        return SECY_DLIB_ERROR(Rc);
    }
    LOG_INFO("\n\t EIP164_Device_Clocks_On \n");

    /* Clock control, restore all EIP-164 clock signals, */
    /* e.g. go back to the dynamic clock control, */
    /* for this ADAPTER_EIP164_DEVICE_CLOCK_CONTROL must be set to 0 */
    Rc = EIP164_Device_Clocks_On(&PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->IOArea,
                            EIP164_SECY_ALL_CLOCKS,
                            ADAPTER_EIP164_DEVICE_CLOCK_CONTROL);
    if (Rc != EIP164_NO_ERROR)
    {
        Adapter_Free(PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]);
        PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_SET(SecY_Device[DeviceId],NULL);
        SecYLib_Device_Unlock(DeviceId);
        return SECY_DLIB_ERROR(Rc);
    }
#endif /* !ADAPTER_EIP164_DEV_INIT_DISABLE */

    if ((Cp.EIP164_Options.SA_Count == 0) ||
        (Cp.EIP164_Options.SC_Count == 0) ||
        (Cp.EIP164_Options.Channel_Count == 0) ||
        (Cp.EIP164_Options.vPort_Count == 0))
    {
        LOG_CRIT("%s: Failed, the number of SA's, SC's, Channels or vPorts "
                 "cannot be zero for device %d\n",
                 __func__, DeviceId);
        Adapter_Free(PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]);
        PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_SET(SecY_Device[DeviceId],NULL);
        SecYLib_Device_Unlock(DeviceId);
        return SECY_ERROR_INTERNAL;
    }
    PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_SET(SecY_Device[DeviceId]->SACount,Cp.EIP164_Options.SA_Count);
    PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_SET(SecY_Device[DeviceId]->SCCount,Cp.EIP164_Options.SC_Count);
    PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_SET(SecY_Device[DeviceId]->ChannelCount,Cp.EIP164_Options.Channel_Count);
    PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_SET(SecY_Device[DeviceId]->vPortCount,Cp.EIP164_Options.vPort_Count);

    /* Restrict any counts to the values specified by Settings_p */
    if (Settings_p->MaxChannelCount > 0 &&
        Settings_p->MaxChannelCount < PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->ChannelCount)
    {
        LOG_CRIT("%s: restricting ChannelCount to %u\n",
                 __func__,Settings_p->MaxChannelCount);
        PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_SET(SecY_Device[DeviceId]->ChannelCount,Settings_p->MaxChannelCount);
    }
    if (Settings_p->MaxvPortCount > 0 &&
        Settings_p->MaxvPortCount < PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->vPortCount)
    {
        LOG_CRIT("%s: restricting vPortCount to %u\n",
                 __func__,Settings_p->MaxvPortCount);
        PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_SET(SecY_Device[DeviceId]->vPortCount,Settings_p->MaxvPortCount);
    }
    if (Settings_p->MaxSCCount > 0 &&
        Settings_p->MaxSCCount < PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->SCCount)
    {
        LOG_CRIT("%s: restricting SCCount to %u\n",
                 __func__,Settings_p->MaxSCCount);
        PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_SET(SecY_Device[DeviceId]->SCCount,Settings_p->MaxSCCount);
    }
    if (Settings_p->MaxSACount > 0 &&
        Settings_p->MaxSACount < PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->SACount)
    {
        LOG_CRIT("%s: restricting SACount to %u\n",
                 __func__,Settings_p->MaxSACount);
        PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_SET(SecY_Device[DeviceId]->SACount,Settings_p->MaxSACount);
    }

    PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_SET(SecY_Device[DeviceId]->Role,Role);

#ifdef ADAPTER_EIP164_WARMBOOT_ENABLE
    if ( !SecYLib_WarmBoot_Device_Init(DeviceId))
    {
        LOG_CRIT("%s: WarmBoot initialization failed\n",__func__);
        Adapter_Free(PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]);
        PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_SET(SecY_Device[DeviceId],NULL);
        SecYLib_Device_Unlock(DeviceId);
        return SECY_ERROR_INTERNAL;
    }
#endif

    /* Initialize the SA free list for this device */
    SecY_Rc = SECY_ERROR_INTERNAL;
    {
        List_Status_t List_Rc;

        /* Allocate SA free list */
        PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_SET(SecY_Device[DeviceId]->SAFL_p,Adapter_Alloc(List_GetInstanceByteCount()));
        if (PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->SAFL_p == NULL)
        {
            LOG_CRIT("%s: Failed to allocate SA free list "
                     "for EIP-164 device %d\n",
                     __func__,
                     DeviceId);
            Adapter_Free(PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]);
            PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_SET(SecY_Device[DeviceId],NULL);
            SecYLib_Device_Unlock(DeviceId);
            return SECY_ERROR_INTERNAL;
        }

        List_Rc = List_Init(0, PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->SAFL_p);
        if (List_Rc != LIST_STATUS_OK)
        {
            LOG_CRIT("%s: Failed to initialize SA free list for EIP-164 "
                     "device for device %d\n",
                     __func__,
                     DeviceId);
            goto lError;
        }

        /* Allocate SA descriptors for this device */
        PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_SET(SecY_Device[DeviceId]->SADscr_p,
                                              Adapter_Alloc(PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->SACount *
                                                                                            sizeof (SecY_SA_Descriptor_t)));
        if (PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->SADscr_p == NULL)
        {
            LOG_CRIT("%s: Failed to allocate SA descriptors for EIP-164 "
                     "device for device %d\n",
                     __func__,
                     DeviceId);
            goto lError;
        }

        for (i = 0; i < PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->SACount; i++)
        {
            PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_SET(SecY_Device[DeviceId]->SADscr_p[i].Magic,0);

            /* Add the SA descriptor as an element to the SA free list */
            PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_SET(SecY_Device[DeviceId]->SADscr_p[i].u.free.DataObject_p,
                                                  &PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->SADscr_p[i]);

            List_Rc = List_AddToHead(0,
                                     PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->SAFL_p,
                                     &PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->SADscr_p[i].u.free);
            if (List_Rc != LIST_STATUS_OK)
            {
                LOG_CRIT("%s: Failed to create SA free list for EIP-164 "
                         "device for device %d\n",
                         __func__,
                         DeviceId);
                goto lError;
            }
        } /* for (SA count) */
    } /* SA free list initialization done */

    /* Initialize the SC free list for this device */
    {
        List_Status_t List_Rc;

        /* Allocate SA free list */
        PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_SET(SecY_Device[DeviceId]->SCFL_p,Adapter_Alloc(List_GetInstanceByteCount()));
        if (PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->SCFL_p == NULL)
        {
            LOG_CRIT("%s: Failed to allocate SC free list "
                     "for EIP-164 device %d\n",
                     __func__,
                     DeviceId);
            goto lError;
        }

        List_Rc = List_Init(0, PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->SCFL_p);
        if (List_Rc != LIST_STATUS_OK)
        {
            LOG_CRIT("%s: Failed to initialize SC free list for EIP-164 "
                     "device for device %d\n",
                     __func__,
                     DeviceId);
            goto lError;
        }

        /* Allocate SC descriptors for this device */
        PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_SET(SecY_Device[DeviceId]->SCDscr_p,
                                              Adapter_Alloc(PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->SCCount *
                                                                                                 sizeof (SecY_SC_Descriptor_t)));
        if (PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->SCDscr_p == NULL)
        {
            LOG_CRIT("%s: Failed to allocate SC descriptors for EIP-164 "
                     "device for device %d\n",
                     __func__,
                     DeviceId);
            goto lError;
        }

        /* Allocate array of vPort descriptors. */
        PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_SET(SecY_Device[DeviceId]->vPortDscr_p,
                                              Adapter_Alloc(PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->vPortCount *
                                                                                                          sizeof(SecY_vPort_Descriptor_t)));

        if (PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->vPortDscr_p == NULL)
        {
            LOG_CRIT("%s: Failed to allocate vPort descriptor array for "
                     "EIP-164 device for device %d\n",
                     __func__,
                     DeviceId);
            goto lError;
        }

        PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_SET(SecY_Device[DeviceId]->vPortListHeads_p,
                                              Adapter_Alloc(PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->vPortCount *
                                                                                                              List_GetInstanceByteCount()));
        if (PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->vPortListHeads_p == NULL)
        {
            LOG_CRIT("%s: Failed to allocate vPort SC list array for EIP-164 "
                     "device for device %d\n",
                     __func__,
                     DeviceId);
            goto lError;
        }

        for (i = 0; i < PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->vPortCount; i++)
        {
            PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_SET(SecY_Device[DeviceId]->vPortDscr_p[i].SACount,0);
            PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_SET(SecY_Device[DeviceId]->vPortDscr_p[i].MTU,0);
            PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_SET(SecY_Device[DeviceId]->vPortDscr_p[i].SCList,
                                                  PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->vPortListHeads_p +
                                                                                                       i * List_GetInstanceByteCount());
            List_Rc = List_Init(0, PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->vPortDscr_p[i].SCList);
            if (List_Rc != LIST_STATUS_OK)
            {
                LOG_CRIT("%s: Failed to initialize SC lookup list for EIP-164 "
                         "device for device %d\n",
                         __func__, DeviceId);
                goto lError;
            }
        }

        for (i = 0; i < PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->SCCount; i++)
        {
            /* Set MapType to DETACHED */
            PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_SET(SecY_Device[DeviceId]->SCDscr_p[i].MapType,SECY_SA_MAP_DETACHED);

#ifdef ADAPTER_EIP164_DBG
            /* SC descriptor magic number */
            PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_SET(SecY_Device[DeviceId]->SCDscr_p[i].Magic,ADAPTER_EIP164_SA_DSCR_MAGIC);
#endif /* ADAPTER_EIP164_DBG */
            PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_SET(SecY_Device[DeviceId]->SCDscr_p[i].SCIndex,i);

            /* Add the SC descriptor as an element to the SC free list */
            PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_SET(SecY_Device[DeviceId]->SCDscr_p[i].OnList.DataObject_p,
                                                  &PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->SCDscr_p[i]);

            List_Rc = List_AddToHead(0,
                                     PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->SCFL_p,
                                     &PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->SCDscr_p[i].OnList);
            if (List_Rc != LIST_STATUS_OK)
            {
                LOG_CRIT("%s: Failed to add descriptor to SC lookup list "
                         "for EIP-164 device for device %d\n",
                         __func__,
                         DeviceId);
                goto lError;
            }
        } /* for (vPorts count) */
    } /* SC free list initialization done */

#ifdef ADAPTER_EIP164_INTERRUPTS_ENABLE
    {
        int rc;
        /* Initialize Global AIC */
        rc = Adapter_Interrupts_Init(SecYLib_ICDeviceID_Get(DeviceId, NULL));
        if (rc) goto irq_failed;

        rc = Adapter_Interrupt_SetHandler(SecYLib_ICDeviceID_Get(DeviceId, NULL),
                                     SecYLib_InterruptHandler);
        if (rc) goto irq_failed;
        /* Initialize Packet Engine AIC. */
        rc = Adapter_Interrupts_Init(SecYLib_ICDeviceID_Get(DeviceId, NULL) + 1);
        if (rc) goto irq_failed;
        rc = Adapter_Interrupt_SetHandler(SecYLib_ICDeviceID_Get(DeviceId, NULL) + 1,
                                 SecYLib_InterruptHandler);
        if (rc) goto irq_failed;

        /* Initialize Channel AIC's */
        for (i = 0; i < PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->ChannelCount; i++)
        {
            rc = Adapter_Interrupts_Init(SecYLib_ICDeviceID_Get(DeviceId, &i));

            if (rc) goto irq_failed;
            rc = Adapter_Interrupt_SetHandler(SecYLib_ICDeviceID_Get(DeviceId, &i),
                                              SecYLib_InterruptHandler);
            if (rc) goto irq_failed;
        }
    irq_failed:
        if (rc)
        {
            /* Uninitialize Global AIC */
            Adapter_Interrupts_UnInit(SecYLib_ICDeviceID_Get(DeviceId, NULL));
            Adapter_Interrupts_UnInit(SecYLib_ICDeviceID_Get(DeviceId, NULL) + 1);

            /* Uninitialize Channel AIC's */
            for (i = 0; i < PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->ChannelCount; i++)
            {
                Adapter_Interrupts_UnInit(SecYLib_ICDeviceID_Get(DeviceId, &i));
            }

            SecY_Rc = SECY_DLIB_ERROR(rc);
            goto lError;
        }
    }
#endif /* ADAPTER_EIP164_INTERRUPTS_ENABLE */

    SecY_Rc = SECY_STATUS_OK;

lError:
    if (SecY_Rc != SECY_STATUS_OK)
    {
        /* Free vPort descriptors. */
        Adapter_Free(PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->vPortDscr_p);
        PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_SET(SecY_Device[DeviceId]->vPortDscr_p,NULL);

        /* Free vPort List heads */
        Adapter_Free(PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->vPortListHeads_p);
        PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_SET(SecY_Device[DeviceId]->vPortListHeads_p,NULL);

        /* Free SC descriptors */
        Adapter_Free(PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->SCDscr_p);
        PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_SET(SecY_Device[DeviceId]->SCDscr_p,NULL);

        if (PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->SCFL_p)
        {
            List_Uninit(0, PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->SCFL_p);
            Adapter_Free(PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->SCFL_p);
            PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_SET(SecY_Device[DeviceId]->SCFL_p,NULL);
        }

        /* Free SA descriptors */
        Adapter_Free(PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->SADscr_p);
        PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_SET(SecY_Device[DeviceId]->SADscr_p,NULL);

        if (PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->SAFL_p)
        {
            List_Uninit(0, PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->SAFL_p);
            Adapter_Free(PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->SAFL_p);
            PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_SET(SecY_Device[DeviceId]->SAFL_p,NULL);
        }

#ifdef ADAPTER_EIP164_WARMBOOT_ENABLE
        SecYLib_WarmBoot_Device_Uninit(DeviceId);
#endif
        Adapter_Free(PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]);
        PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_SET(SecY_Device[DeviceId],NULL);
        SecYLib_Device_Unlock(DeviceId);
        return SecY_Rc;
    }


    PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_SET(SecY_Device_StaticFields[DeviceId].fInitialized,true);

    SecYLib_Device_Unlock(DeviceId);
    LOG_INFO("%s: device for device %d successfully initialized\n",
             __func__, DeviceId);

    return SECY_STATUS_OK;
}


/*----------------------------------------------------------------------------
 * SecY_Device_Uninit
 */
SecY_Status_t
SecY_Device_Uninit(
        const unsigned int DeviceId)
{
    List_Status_t List_Rc;

#ifndef ADAPTER_EIP164_DEV_UNINIT_DISABLE
    EIP164_Error_t Rc;
#endif

    LOG_INFO("\n %s \n", __func__);

#ifdef ADAPTER_EIP164_STRICT_ARGS
    if (DeviceId >= ADAPTER_EIP164_MAX_NOF_DEVICES)
        return SECY_ERROR_BAD_PARAMETER;
#endif

    SecYLib_Device_Lock(DeviceId);

    if (!PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device_StaticFields)[DeviceId].fInitialized)
    {
        LOG_CRIT("%s: failed, EIP-164 device %d already uninitialized\n",
                 __func__,
                 DeviceId);
        SecYLib_Device_Unlock(DeviceId);
        return SECY_ERROR_INTERNAL;
    }

#ifdef ADAPTER_EIP164_INTERRUPTS_ENABLE
    {
        unsigned int i, NotifyId, PE_ICDeviceId;

        LOG_INFO("%s: Uninitialize Global AIC\n", __func__);
        Adapter_Interrupts_UnInit(SecYLib_ICDeviceID_Get(DeviceId, NULL));

        NotifyId = ADAPTER_EIP164_NOTIFY_ID(
                            SecYLib_ICDeviceID_Get(DeviceId, NULL));
        Adapter_Free(SecY_Notify[NotifyId]);
        SecY_Notify[NotifyId] = NULL;

        LOG_INFO("%s: Uninitialize PE AIC\n", __func__);
        PE_ICDeviceId = ADAPTER_EIP164_ICDEV_ID(
                            ADAPTER_EIP164_DEV_ID(DeviceId), false, true, 0);
        Adapter_Interrupt_Clear(PE_ICDeviceId, 0xffff);
        Adapter_Interrupt_Disable(PE_ICDeviceId, 0xffff);
        Adapter_Interrupts_UnInit(PE_ICDeviceId);

        NotifyId = ADAPTER_EIP164_NOTIFY_ID(PE_ICDeviceId);
        Adapter_Free(SecY_Notify[NotifyId]);
        SecY_Notify[NotifyId] = NULL;

        for (i = 0; i < PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->ChannelCount; i++)
        {
            LOG_INFO("%s: Uninitialize AIC for channel %d\n", __func__, i);
            Adapter_Interrupts_UnInit(SecYLib_ICDeviceID_Get(DeviceId, &i));

            NotifyId = ADAPTER_EIP164_NOTIFY_ID(
                                    SecYLib_ICDeviceID_Get(DeviceId, &i));
            Adapter_Free(SecY_Notify[NotifyId]);
            SecY_Notify[NotifyId] = NULL;
        }
    }
#endif /* ADAPTER_EIP164_INTERRUPTS_ENABLE */

    /* Uninitialize SA free list */
    List_Rc = List_Uninit(0, PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->SAFL_p);
    if (List_Rc != LIST_STATUS_OK)
    {
        LOG_CRIT("%s: Failed to uninit SA free list for "
                 "EIP-164 device for device %d\n",
                 __func__,
                 DeviceId);
    }
    Adapter_Free(PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->SAFL_p);
    PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_SET(SecY_Device[DeviceId]->SAFL_p,NULL);

    /* Free SA descriptors */
    Adapter_Free(PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->SADscr_p);
    PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_SET(SecY_Device[DeviceId]->SADscr_p,NULL);

    /* Uninitialize SC free list */
    List_Rc = List_Uninit(0, PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->SCFL_p);
    if (List_Rc != LIST_STATUS_OK)
    {
        LOG_CRIT("%s: Failed to uninit SC free list for "
                 "EIP-164 device for device %d\n",
                 __func__,
                 DeviceId);
    }
    Adapter_Free(PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->SCFL_p);
    PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_SET(SecY_Device[DeviceId]->SCFL_p,NULL);

    /* Free SC descriptors */
    Adapter_Free(PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->SCDscr_p);
    PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_SET(SecY_Device[DeviceId]->SCDscr_p,NULL);

    Adapter_Free(PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->vPortDscr_p);
    PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_SET(SecY_Device[DeviceId]->vPortDscr_p,NULL);

    Adapter_Free(PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->vPortListHeads_p);
    PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_SET(SecY_Device[DeviceId]->vPortListHeads_p,NULL);

#ifndef ADAPTER_EIP164_DEV_UNINIT_DISABLE
    LOG_INFO("\n\t EIP164_Device_Uninit \n");

    Rc = EIP164_Device_Uninit(&PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->IOArea);
    if (Rc != EIP164_NO_ERROR && Rc != EIP164_BUSY_RETRY_LATER)
    {
        LOG_CRIT("%s: Failed to uninit EIP-164 device %d (%s), error %d\n",
                 __func__,
                 DeviceId,
                 ADAPTER_EIP164_DEVICE_NAME(DeviceId),
                 Rc);
        PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_SET(SecY_Device_StaticFields[DeviceId].fInitialized,false);
        SecYLib_Device_Unlock(DeviceId);
#ifdef ADAPTER_EIP164_WARMBOOT_ENABLE
        SecYLib_WarmBoot_Device_Uninit(DeviceId);
#endif
        Adapter_Free(PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]);
        PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_SET(SecY_Device[DeviceId],NULL);
        return SECY_DLIB_ERROR(Rc);
    }

#if ADAPTER_EIP164_MAX_NOF_UNINIT_RETRY_COUNT > 0
    /* Wait till the SecY device uninitialization is done */
    if (Rc == EIP164_BUSY_RETRY_LATER)
    {
        unsigned int LoopCounter = ADAPTER_EIP164_MAX_NOF_UNINIT_RETRY_COUNT;

        do
        {
            LOG_INFO("\n\t EIP164_Device_Is_Init_Done \n");

            Rc = EIP164_Device_Is_Init_Done(&PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->IOArea);
            if (Rc != EIP164_BUSY_RETRY_LATER)
            {
                break;
            }

            Adapter_SleepMS(ADAPTER_EIP164_UNINIT_RETRY_TIMEOUT_MS);
        } while (--LoopCounter);

        if (Rc != EIP164_NO_ERROR)
        {
            LOG_CRIT(
              "%s: Failed to complete EIP-164 device %d uninitialization\n",
              __func__,
              DeviceId);
            PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_SET(SecY_Device_StaticFields[DeviceId].fInitialized,false);
            SecYLib_Device_Unlock(DeviceId);
#ifdef ADAPTER_EIP164_WARMBOOT_ENABLE
            SecYLib_WarmBoot_Device_Uninit(DeviceId);
#endif
            Adapter_Free(PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]);
            PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_SET(SecY_Device[DeviceId],NULL);
            return SECY_DLIB_ERROR(Rc);
        }
    }
#endif /* ADAPTER_EIP164_MAX_NOF_UNINIT_RETRY_COUNT > 0 */
#endif /* !ADAPTER_EIP164_DEV_UNINIT_DISABLE */

    PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_SET(SecY_Device_StaticFields[DeviceId].fInitialized,false);

    SecYLib_Device_Unlock(DeviceId);

    LOG_INFO("%s: device for device %d successfully uninitialized\n",
             __func__,
             DeviceId);
#ifdef ADAPTER_EIP164_WARMBOOT_ENABLE
    SecYLib_WarmBoot_Device_Uninit(DeviceId);
#endif

    Adapter_Free(PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]);
    PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_SET(SecY_Device[DeviceId],NULL);

    return SECY_STATUS_OK;
}


/*----------------------------------------------------------------------------
 * SecY_Device_Limits
 */
SecY_Status_t
SecY_Device_Limits(
        const unsigned int DeviceId,
        unsigned int * const MaxChannelCount_p,
        unsigned int * const MaxvPortCount_p,
        unsigned int * const MaxSACount_p,
        unsigned int * const MaxSCCount_p)
{
    LOG_INFO("\n %s \n", __func__);

#ifdef ADAPTER_EIP164_STRICT_ARGS
    if (DeviceId >= ADAPTER_EIP164_MAX_NOF_DEVICES)
        return SECY_ERROR_BAD_PARAMETER;
#endif
    SecYLib_Device_Lock(DeviceId);

    if (!PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device_StaticFields)[DeviceId].fInitialized)
    {
        SecYLib_Device_Unlock(DeviceId);
        return SECY_ERROR_INTERNAL;
    }

    if (MaxChannelCount_p != NULL)
        *MaxChannelCount_p = PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->ChannelCount;

    if (MaxvPortCount_p != NULL)
        *MaxvPortCount_p = PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->vPortCount;

    if (MaxSACount_p != NULL)
        *MaxSACount_p = PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->SACount;

    if (MaxSCCount_p != NULL)
        *MaxSCCount_p = PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->SCCount;

    LOG_INFO("\n %s done \n", __func__);

    SecYLib_Device_Unlock(DeviceId);

    return SECY_STATUS_OK;
}


/*----------------------------------------------------------------------------
 * SecY_Device_Update
 */
SecY_Status_t
SecY_Device_Update(
        const unsigned int DeviceId,
        const SecY_Device_Params_t * const Device_p)
{
    EIP164_Error_t EIP164_Rc = 0;
    SecY_Status_t Rc;
    unsigned int ErrorType; /* Index in ErrorString */
    const char * ErrorString[] = {"statistics",
                                  "statistics sync",
                                  "channels"};

    LOG_INFO("\n %s \n", __func__);

#ifdef ADAPTER_EIP164_STRICT_ARGS
    if (DeviceId >= ADAPTER_EIP164_MAX_NOF_DEVICES)
        return SECY_ERROR_BAD_PARAMETER;

    if (Device_p == NULL)
        return SECY_ERROR_BAD_PARAMETER;
#endif /* ADAPTER_EIP164_STRICT_ARGS */

    SecYLib_Device_Lock(DeviceId);

    if (!PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device_StaticFields)[DeviceId].fInitialized)
    {
        SecYLib_Device_Unlock(DeviceId);
        return SECY_ERROR_INTERNAL;
    }

    if (Device_p->StatControl_p)
    {
        EIP164_Statistics_Control_t StatControl;
        SecY_Statistics_Control_t * StatCtrl_p = Device_p->StatControl_p;
        uint16_t SACntrMask, NumSACntrs;
        uint16_t IFCCntrMask, NumIFCCntrs;
        uint16_t SecYCntrMask, NumSecYCntrs;
        uint16_t RxCAMCntrMask, NumRxCAMCntrs;

        ZEROINIT(StatControl);
        StatControl.fAutoStatCntrsReset     = StatCtrl_p->fAutoStatCntrsReset;
        StatControl.SeqNrThreshold          = StatCtrl_p->SeqNrThreshold;
        StatControl.SeqNrThreshold64.hi     = StatCtrl_p->SeqNrThreshold64.Hi;
        StatControl.SeqNrThreshold64.low    = StatCtrl_p->SeqNrThreshold64.Lo;
        StatControl.SACountFrameThr.hi      = StatCtrl_p->SACountFrameThr.Hi;
        StatControl.SACountFrameThr.low     = StatCtrl_p->SACountFrameThr.Lo;
        StatControl.SecYCountFrameThr.hi    = StatCtrl_p->SecYCountFrameThr.Hi;
        StatControl.SecYCountFrameThr.low   = StatCtrl_p->SecYCountFrameThr.Lo;
        StatControl.IFCCountFrameThr.hi     = StatCtrl_p->IFCCountFrameThr.Hi;
        StatControl.IFCCountFrameThr.low    = StatCtrl_p->IFCCountFrameThr.Lo;
        StatControl.IFC1CountFrameThr.hi    = StatCtrl_p->IFC1CountFrameThr.Hi;
        StatControl.IFC1CountFrameThr.low   = StatCtrl_p->IFC1CountFrameThr.Lo;
        StatControl.RxCAMCountFrameThr.hi   = StatCtrl_p->RxCAMCountFrameThr.Hi;
        StatControl.RxCAMCountFrameThr.low  = StatCtrl_p->RxCAMCountFrameThr.Lo;
        StatControl.SACountOctetThr.hi      = StatCtrl_p->SACountOctetThr.Hi;
        StatControl.SACountOctetThr.low     = StatCtrl_p->SACountOctetThr.Lo;
        StatControl.IFCCountOctetThr.hi     = StatCtrl_p->IFCCountOctetThr.Hi;
        StatControl.IFCCountOctetThr.low    = StatCtrl_p->IFCCountOctetThr.Lo;
        StatControl.IFC1CountOctetThr.hi    = StatCtrl_p->IFC1CountOctetThr.Hi;
        StatControl.IFC1CountOctetThr.low   = StatCtrl_p->IFC1CountOctetThr.Lo;

        LOG_INFO("\n\t EIP164_StatisticsControl_Update \n");

        /* Update the SecY statistics control settings */
        EIP164_Rc = EIP164_StatisticsControl_Update(&PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->IOArea,
                                                    &StatControl);
        if ((EIP164_Rc != EIP164_NO_ERROR) &&
                            (EIP164_Rc != EIP164_BUSY_RETRY_LATER))
        {
            ErrorType = 0; /* index in ErrorString */
            goto error_exit;
        }

        if (EIP164_Rc == EIP164_BUSY_RETRY_LATER)
        {
            unsigned int LoopCounter = ADAPTER_EIP164_MAX_NOF_INIT_RETRY_COUNT;

            do
            {
                LOG_INFO("\n\t EIP164_Device_Is_Init_Done \n");

                EIP164_Rc =
                        EIP164_Device_Is_Init_Done(&PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->IOArea);
                if (EIP164_Rc != EIP164_BUSY_RETRY_LATER)
                {
                    break;
                }

                Adapter_SleepMS(ADAPTER_EIP164_INIT_RETRY_TIMEOUT_MS);
            } while (--LoopCounter || EIP164_Rc != EIP164_NO_ERROR);

            if (EIP164_Rc != EIP164_NO_ERROR)
            {
                ErrorType = 1; /* index in ErrorString */
                goto error_exit;
            }
        }

        LOG_CRIT("%s device statistics counter increment enable settings:\n"
                 "\tSA counters: %s\n"
                 "\tIFC counters: %s\n"
                 "\tIFC1 counters: %s\n"
                 "\tSecY counters: %s\n"
                 "\tRxCAM counters: %s\n"
                 "\tChannel counters: %s\n",
                 ADAPTER_EIP164_DEVICE_NAME(DeviceId),
                 (StatCtrl_p->CountIncDisCtrl & SECY_SA_COUNT_INC_DIS) ?
                         "Disabled" : "Enabled",
                 (StatCtrl_p->CountIncDisCtrl & SECY_IFC_COUNT_INC_DIS) ?
                         "Disabled" : "Enabled",
                 (StatCtrl_p->CountIncDisCtrl & SECY_IFC1_COUNT_INC_DIS) ?
                         "Disabled" : "Enabled",
                 (StatCtrl_p->CountIncDisCtrl & SECY_SECY_COUNT_INC_DIS) ?
                         "Disabled" : "Enabled",
                 (StatCtrl_p->CountIncDisCtrl & SECY_RXCAM_COUNT_INC_DIS) ?
                         "Disabled" : "Enabled",
                 (StatCtrl_p->CountIncDisCtrl & SECY_CHAN_COUNT_INC_DIS) ?
                         "Disabled" : "Enabled");

        NumRxCAMCntrs = sizeof(SecY_RxCAM_Stat_t) /
                                sizeof(SecY_Stat_Counter_t);

        if (PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->Role == SECY_ROLE_INGRESS)
        {
            NumSACntrs = sizeof(SecY_SA_Stat_I_t) /
                                sizeof(SecY_Stat_Counter_t);
            NumIFCCntrs = sizeof(SecY_Ifc_Stat_I_t) /
                                sizeof(SecY_Stat_Counter_t);
            NumSecYCntrs = sizeof(SecY_SecY_Stat_I_t) /
                                sizeof(SecY_Stat_Counter_t);
        } else {
            NumSACntrs = sizeof(SecY_SA_Stat_E_t) /
                                sizeof(SecY_Stat_Counter_t);
            NumIFCCntrs = sizeof(SecY_Ifc_Stat_E_t) /
                                sizeof(SecY_Stat_Counter_t);
            NumSecYCntrs = sizeof(SecY_SecY_Stat_E_t) /
                                sizeof(SecY_Stat_Counter_t);
        }

        /* Calculate masks for every statistics module to cover all counters */
        SACntrMask = ((1 << NumSACntrs) - 1);
        IFCCntrMask = ((1 << NumIFCCntrs) - 1);
        SecYCntrMask = ((1 << NumSecYCntrs) - 1);
        RxCAMCntrMask = ((1 << NumRxCAMCntrs) - 1);

        LOG_INFO("\n\t EIP164_Device_SA_Statistics_Enable \n");

        /* Enable or disable counters, depending on the value of */
        /* Settings_p->CountIncDisCtrl */
        EIP164_Rc = EIP164_Device_SA_Statistics_Enable(&PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->IOArea,
                SACntrMask *
                (!(StatCtrl_p->CountIncDisCtrl & SECY_SA_COUNT_INC_DIS)));
        if (EIP164_Rc != EIP164_NO_ERROR)
        {
            ErrorType = 0; /* index in ErrorString */
            goto error_exit;
        }

        LOG_INFO("\n\t EIP164_Device_IFC_Statistics_Enable \n");

        EIP164_Rc =
            EIP164_Device_IFC_Statistics_Enable(&PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->IOArea,
                IFCCntrMask *
                (!(StatCtrl_p->CountIncDisCtrl & SECY_IFC_COUNT_INC_DIS)));
        if (EIP164_Rc != EIP164_NO_ERROR)
        {
            ErrorType = 0; /* index in ErrorString */
            goto error_exit;
        }

        LOG_INFO("\n\t EIP164_Device_IFC1_Statistics_Enable \n");

        EIP164_Rc =
            EIP164_Device_IFC1_Statistics_Enable(&PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->IOArea,
                IFCCntrMask *
                (!(StatCtrl_p->CountIncDisCtrl & SECY_IFC1_COUNT_INC_DIS)));
        if (EIP164_Rc != EIP164_NO_ERROR)
        {
            ErrorType = 0; /* index in ErrorString */
            goto error_exit;
        }

        LOG_INFO("\n\t EIP164_Device_SecY_Statistics_Enable \n");

        EIP164_Rc =
            EIP164_Device_SecY_Statistics_Enable(&PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->IOArea,
                (SecYCntrMask * (!(StatCtrl_p->CountIncDisCtrl &
                                   SECY_SECY_COUNT_INC_DIS))));
        if (EIP164_Rc != EIP164_NO_ERROR)
        {
            ErrorType = 0; /* index in ErrorString */
            goto error_exit;
        }

        LOG_INFO("\n\t EIP164_Device_RxCAM_Statistics_Enable \n");

        EIP164_Rc =
            EIP164_Device_RxCAM_Statistics_Enable(&PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->IOArea,
                RxCAMCntrMask *
                (!(StatCtrl_p->CountIncDisCtrl & SECY_RXCAM_COUNT_INC_DIS)));
        if (EIP164_Rc != EIP164_NO_ERROR)
        {
            ErrorType = 0; /* index in ErrorString */
            goto error_exit;
        }
    }

    /* Configure channels if required */
    if (Device_p->ChConf_p)
    {
        Rc = SecYLib_Channel_Config_Set(DeviceId, Device_p->ChConf_p);
        if (Rc != SECY_STATUS_OK)
        {
            ErrorType = 2; /* index in ErrorString */
            goto error_exit;
        }
    }

    if (Device_p->EOPConf_p)
    {
        EIP164_Error_t Rc;

        Rc = EIP164_Device_EOP_Configure(&PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->IOArea,
               Device_p->EOPConf_p->EOPTimeoutVal,
               (const EIP164_Ch_Mask_t * const)&Device_p->EOPConf_p->EOPTimeoutCtrl);

        if (Rc != EIP164_NO_ERROR)
        {
            LOG_CRIT("%s: Failed to configure EOP EIP-164 device for device %d "
                 "(%s), error %d\n",
                 __func__,
                 DeviceId,
                     ADAPTER_EIP164_DEVICE_NAME(DeviceId),
                     Rc);
            return SECY_DLIB_ERROR(Rc);
        }
    }

    if (Device_p->ECCConf_p)
    {
        EIP164_Error_t Rc;

        Rc = EIP164_Device_ECC_Configure(&PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->IOArea,
                                         Device_p->ECCConf_p->ECCCorrectableThr,
                                         Device_p->ECCConf_p->ECCUncorrectableThr);
        if (Rc != EIP164_NO_ERROR)
        {
            LOG_CRIT("%s: Failed to configure ECC EIP-164 device for device %d "
                 "(%s), error %d\n",
                 __func__,
                 DeviceId,
                     ADAPTER_EIP164_DEVICE_NAME(DeviceId),
                     Rc);
            return SECY_DLIB_ERROR(Rc);
        }
    }

    SecYLib_Device_Unlock(DeviceId);

    LOG_INFO("\n %s done \n", __func__);

    IDENTIFIER_NOT_USED_LOG_OFF(ErrorType);
    IDENTIFIER_NOT_USED_LOG_OFF(ErrorString);

    return SECY_STATUS_OK;

error_exit:
    LOG_CRIT("%s: Failed to update EIP-164 device %d (%s) for %s, "
             "error %d\n",
             __func__,
             DeviceId,
             ADAPTER_EIP164_DEVICE_NAME(DeviceId),
             ErrorString[ErrorType],
             EIP164_Rc);

    SecYLib_Device_Unlock(DeviceId);

    return SECY_DLIB_ERROR(EIP164_Rc);
}


/*-----------------------------------------------------------------------------
 * SecY_Device_Config_Get
 */
SecY_Status_t
SecY_Device_Config_Get(
        const unsigned int DeviceId,
        SecY_Device_Params_t * const Control_p)
{
    EIP164_Error_t Rc;
    SecY_Statistics_Control_t * StatControl_p;
    SecY_ChannelConf_t * ChConf_p;
    SecY_EOPConf_t * EOPConf_p;
    SecY_ECCConf_t * ECCConf_p;

#ifdef ADAPTER_EIP164_STRICT_ARGS
    if (DeviceId >= ADAPTER_EIP164_MAX_NOF_DEVICES)
        return SECY_ERROR_BAD_PARAMETER;

    if (Control_p == NULL)
        return SECY_ERROR_BAD_PARAMETER;
#endif

    SecYLib_Device_Lock(DeviceId);

    if (!PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device_StaticFields)[DeviceId].fInitialized)
    {
        SecYLib_Device_Unlock(DeviceId);
        return SECY_ERROR_INTERNAL;
    }

    StatControl_p = Control_p->StatControl_p;
    if (StatControl_p)
    {
        EIP164_Statistics_Control_t StatControl;
        uint32_t Mask;
        StatControl_p->CountIncDisCtrl = 0;
        Rc = EIP164_StatisticsControl_Read(&PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->IOArea,
                                           &StatControl);
        if (Rc != EIP164_NO_ERROR)
        {
            LOG_CRIT("%s: Failed to read statistics control EIP-164 device for device %d "
                     "(%s), error %d\n",
                     __func__,
                     DeviceId,
                     ADAPTER_EIP164_DEVICE_NAME(DeviceId),
                     Rc);
            SecYLib_Device_Unlock(DeviceId);
            return SECY_DLIB_ERROR(Rc);
        }
        StatControl_p->fAutoStatCntrsReset = StatControl.fAutoStatCntrsReset;
        StatControl_p->SeqNrThreshold = StatControl.SeqNrThreshold;
        StatControl_p->SeqNrThreshold64.Hi = StatControl.SeqNrThreshold64.hi;
        StatControl_p->SeqNrThreshold64.Lo = StatControl.SeqNrThreshold64.low;
        StatControl_p->SACountFrameThr.Hi = StatControl.SACountFrameThr.hi;
        StatControl_p->SACountFrameThr.Lo = StatControl.SACountFrameThr.low;
        StatControl_p->SecYCountFrameThr.Hi = StatControl.SecYCountFrameThr.hi;
        StatControl_p->SecYCountFrameThr.Lo = StatControl.SecYCountFrameThr.low;
        StatControl_p->IFCCountFrameThr.Hi = StatControl.IFCCountFrameThr.hi;
        StatControl_p->IFCCountFrameThr.Lo = StatControl.IFCCountFrameThr.low;
        StatControl_p->IFC1CountFrameThr.Hi = StatControl.IFC1CountFrameThr.hi;
        StatControl_p->IFC1CountFrameThr.Lo = StatControl.IFC1CountFrameThr.low;
        StatControl_p->RxCAMCountFrameThr.Hi = StatControl.RxCAMCountFrameThr.hi;
        StatControl_p->RxCAMCountFrameThr.Lo = StatControl.RxCAMCountFrameThr.low;
        StatControl_p->SACountOctetThr.Hi = StatControl.SACountOctetThr.hi;
        StatControl_p->SACountOctetThr.Lo = StatControl.SACountOctetThr.low;
        StatControl_p->IFCCountOctetThr.Hi = StatControl.IFCCountOctetThr.hi;
        StatControl_p->IFCCountOctetThr.Lo = StatControl.IFCCountOctetThr.low;
        StatControl_p->IFC1CountOctetThr.Hi = StatControl.IFC1CountOctetThr.hi;
        StatControl_p->IFC1CountOctetThr.Lo = StatControl.IFC1CountOctetThr.low;

        Rc = EIP164_Device_SA_Statistics_Enabled_Get(&PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->IOArea,
                                           &Mask);
        if (Rc != EIP164_NO_ERROR)
        {
            LOG_CRIT("%s: Failed to read statistics enabled EIP-164 device for device %d "
                     "(%s), error %d\n",
                     __func__,
                     DeviceId,
                     ADAPTER_EIP164_DEVICE_NAME(DeviceId),
                     Rc);
            SecYLib_Device_Unlock(DeviceId);
            return SECY_DLIB_ERROR(Rc);
        }
        if (Mask == 0)
        {
            StatControl_p->CountIncDisCtrl |= SECY_SA_COUNT_INC_DIS;
        }

        Rc = EIP164_Device_SecY_Statistics_Enabled_Get(&PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->IOArea,
                                           &Mask);
        if (Rc != EIP164_NO_ERROR)
        {
            LOG_CRIT("%s: Failed to read statistics enabled EIP-164 device for device %d "
                     "(%s), error %d\n",
                     __func__,
                     DeviceId,
                     ADAPTER_EIP164_DEVICE_NAME(DeviceId),
                     Rc);
            SecYLib_Device_Unlock(DeviceId);
            return SECY_DLIB_ERROR(Rc);
        }
        if (Mask == 0)
        {
            StatControl_p->CountIncDisCtrl |= SECY_SECY_COUNT_INC_DIS;
        }

        Rc = EIP164_Device_IFC_Statistics_Enabled_Get(&PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->IOArea,
                                           &Mask);
        if (Rc != EIP164_NO_ERROR)
        {
            LOG_CRIT("%s: Failed to read statistics enabled EIP-164 device for device %d "
                     "(%s), error %d\n",
                     __func__,
                     DeviceId,
                     ADAPTER_EIP164_DEVICE_NAME(DeviceId),
                     Rc);
            SecYLib_Device_Unlock(DeviceId);
            return SECY_DLIB_ERROR(Rc);
        }
        if (Mask == 0)
        {
            StatControl_p->CountIncDisCtrl |= SECY_IFC_COUNT_INC_DIS;
        }

        Rc = EIP164_Device_IFC1_Statistics_Enabled_Get(&PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->IOArea,
                                           &Mask);
        if (Rc != EIP164_NO_ERROR)
        {
            LOG_CRIT("%s: Failed to read statistics enabled EIP-164 device for device %d "
                     "(%s), error %d\n",
                     __func__,
                     DeviceId,
                     ADAPTER_EIP164_DEVICE_NAME(DeviceId),
                     Rc);
            SecYLib_Device_Unlock(DeviceId);
            return SECY_DLIB_ERROR(Rc);
        }
        if (Mask == 0)
        {
            StatControl_p->CountIncDisCtrl |= SECY_IFC1_COUNT_INC_DIS;
        }

        Rc = EIP164_Device_RxCAM_Statistics_Enabled_Get(&PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->IOArea,
                                           &Mask);
        if (Rc != EIP164_NO_ERROR)
        {
            LOG_CRIT("%s: Failed to read statistics enabled EIP-164 device for device %d "
                     "(%s), error %d\n",
                     __func__,
                     DeviceId,
                     ADAPTER_EIP164_DEVICE_NAME(DeviceId),
                     Rc);
            SecYLib_Device_Unlock(DeviceId);
            return SECY_DLIB_ERROR(Rc);
        }
        if (Mask == 0)
        {
            StatControl_p->CountIncDisCtrl |= SECY_RXCAM_COUNT_INC_DIS;
        }
    }

    ChConf_p = Control_p->ChConf_p;
    if (ChConf_p)
    {
        unsigned int i;
        SecY_Status_t SecY_Rc;
        ChConf_p->Params.ChannelCount = PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->ChannelCount;
        if (ChConf_p->Params.Channel_p)
        {
            for (i = 0; i < ChConf_p->Params.ChannelCount; i++)
            {
                SecY_Rc = SecYLib_Channel_Config_Get(DeviceId,
                                                     i,
                                                     &ChConf_p->Params.Channel_p[i]);
                if (SecY_Rc != SECY_STATUS_OK)
                {

                    SecYLib_Device_Unlock(DeviceId);
                    return SecY_Rc;
                }
            }
        }
    }

    EOPConf_p = Control_p->EOPConf_p;
    if (EOPConf_p)
    {
        Rc = EIP164_Device_EOP_Configuration_Get(&PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->IOArea,
                 &EOPConf_p->EOPTimeoutVal,
                 (EIP164_Ch_Mask_t * const) &EOPConf_p->EOPTimeoutCtrl);
        if (Rc != EIP164_NO_ERROR)
        {
            LOG_CRIT("%s: Failed to read EOP conf EIP-164 device for device %d "
                     "(%s), error %d\n",
                     __func__,
                     DeviceId,
                     ADAPTER_EIP164_DEVICE_NAME(DeviceId),
                     Rc);
            SecYLib_Device_Unlock(DeviceId);
            return SECY_DLIB_ERROR(Rc);
        }
    }

    ECCConf_p = Control_p->ECCConf_p;
    if (ECCConf_p)
    {
        Rc = EIP164_Device_ECC_Configuration_Get(&PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->IOArea,
                                                 &ECCConf_p->ECCCorrectableThr,
                                                 &ECCConf_p->ECCUncorrectableThr);
        if (Rc != EIP164_NO_ERROR)
        {
            LOG_CRIT("%s: Failed to read ECC conf EIP-164 device for device %d "
                     "(%s), error %d\n",
                     __func__,
                     DeviceId,
                     ADAPTER_EIP164_DEVICE_NAME(DeviceId),
                     Rc);
            SecYLib_Device_Unlock(DeviceId);
            return SECY_DLIB_ERROR(Rc);
        }
    }


    SecYLib_Device_Unlock(DeviceId);

    return SECY_STATUS_OK;
}


/*-----------------------------------------------------------------------------
 * SecY_Device_Status_Get
 */
SecY_Status_t
SecY_Device_Status_Get(
        const unsigned int DeviceId,
        SecY_DeviceStatus_t * const DeviceStatus_p)
{
    EIP164_Error_t Rc;
    EIP164_Device_ECCStatus_t ECCStatus[EIP164_ECC_NOF_STATUS_COUNTERS];
    EIP164_Device_PktProcessDebug_t PktProcessDebug;
    unsigned int i;
    SecY_ECCStatus_t *ECCStatus_p;
    SecY_PktProcessDebug_t * PktProcessDebug_p;

#ifdef ADAPTER_EIP164_STRICT_ARGS
    if (DeviceId >= ADAPTER_EIP164_MAX_NOF_DEVICES)
        return SECY_ERROR_BAD_PARAMETER;

    if (DeviceStatus_p == NULL)
        return SECY_ERROR_BAD_PARAMETER;
#endif

    SecYLib_Device_Lock(DeviceId);

    if (!PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device_StaticFields)[DeviceId].fInitialized)
    {
        SecYLib_Device_Unlock(DeviceId);
        return SECY_ERROR_INTERNAL;
    }

    ECCStatus_p = DeviceStatus_p->ECCStatus_p;
    if (ECCStatus_p)
    {
        Rc = EIP164_Device_ECCStatus_Get(&PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->IOArea,
                                         ECCStatus);
        if (Rc != EIP164_NO_ERROR)
        {
            LOG_CRIT("%s: Failed to read ECC status EIP-164 device for device %d "
                     "(%s), error %d\n",
                     __func__,
                     DeviceId,
                     ADAPTER_EIP164_DEVICE_NAME(DeviceId),
                     Rc);
            SecYLib_Device_Unlock(DeviceId);
            return SECY_DLIB_ERROR(Rc);
        }

        for (i=0; i<EIP164_ECC_NOF_STATUS_COUNTERS; i++)
        {
            ECCStatus_p->Counters[i].CorrectableCount = ECCStatus[i].CorrectableCount;
            ECCStatus_p->Counters[i].UncorrectableCount = ECCStatus[i].UncorrectableCount;
            ECCStatus_p->Counters[i].fCorrectableThr = ECCStatus[i].fCorrectableThr;
            ECCStatus_p->Counters[i].fUncorrectableThr = ECCStatus[i].fUncorrectableThr;
        }
    }

    PktProcessDebug_p = DeviceStatus_p->PktProcessDebug_p;
    if (PktProcessDebug_p)
    {
        Rc = EIP164_Device_PktProcessDebug_Get(&PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->IOArea,
                                               &PktProcessDebug);
        if (Rc != EIP164_NO_ERROR)
        {
            LOG_CRIT("%s: Failed to read packet process debug EIP-164 device for device %d "
                     "(%s), error %d\n",
                     __func__,
                     DeviceId,
                     ADAPTER_EIP164_DEVICE_NAME(DeviceId),
                     Rc);
            SecYLib_Device_Unlock(DeviceId);
            return SECY_DLIB_ERROR(Rc);
        }
        PktProcessDebug_p->ParsedDALo = PktProcessDebug.ParsedDALo;
        PktProcessDebug_p->ParsedDAHi = PktProcessDebug.ParsedDAHi;
        PktProcessDebug_p->ParsedSALo = PktProcessDebug.ParsedSALo;
        PktProcessDebug_p->ParsedSAHi = PktProcessDebug.ParsedSAHi;
        PktProcessDebug_p->ParsedSecTAGLo = PktProcessDebug.ParsedSecTAGLo;
        PktProcessDebug_p->ParsedSecTAGHi = PktProcessDebug.ParsedSecTAGHi;
        PktProcessDebug_p->ParsedSCILo = PktProcessDebug.ParsedSCILo;
        PktProcessDebug_p->ParsedSCIHi = PktProcessDebug.ParsedSCIHi;
        PktProcessDebug_p->SecTAGDebug = PktProcessDebug.SecTAGDebug;
        PktProcessDebug_p->RxCAMSCILo = PktProcessDebug.RxCAMSCILo;
        PktProcessDebug_p->RxCAMSCIHi = PktProcessDebug.RxCAMSCIHi;
        PktProcessDebug_p->ParserInDebug = PktProcessDebug.ParserInDebug;
    }
    SecYLib_Device_Unlock(DeviceId);

    return SECY_STATUS_OK;
}


/*-----------------------------------------------------------------------------
 * SecY_Device_InsertSOP
 */
SecY_Status_t
SecY_Device_InsertSOP(
        const unsigned int DeviceId,
        const SecY_Ch_Mask_t * const ChannelMask_p)
{
    EIP164_Error_t Rc;

    LOG_INFO("\n %s \n", __func__);

#ifdef ADAPTER_EIP164_STRICT_ARGS
    if (DeviceId >= ADAPTER_EIP164_MAX_NOF_DEVICES)
        return SECY_ERROR_BAD_PARAMETER;

    if (ChannelMask_p == NULL)
        return SECY_ERROR_BAD_PARAMETER;
#endif

    SecYLib_Device_Lock(DeviceId);

    if (!PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device_StaticFields)[DeviceId].fInitialized)
    {
        SecYLib_Device_Unlock(DeviceId);
        return SECY_ERROR_INTERNAL;
    }

    LOG_INFO("\n\t EIP164_Device_InsertSOP\n");

    Rc = EIP164_Device_InsertSOP(&PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->IOArea,
                                 (const EIP164_Ch_Mask_t * const) ChannelMask_p);
    if (Rc != EIP164_NO_ERROR)
    {
        LOG_CRIT("%s: Failed to insert SOP "
                 "for EIP-164 device for device %d\n",
                 __func__, DeviceId);
        SecYLib_Device_Unlock(DeviceId);
        return SECY_DLIB_ERROR(Rc);
    }

    SecYLib_Device_Unlock(DeviceId);

    LOG_INFO("\n %s done \n", __func__);

    return SECY_STATUS_OK;
}


/*-----------------------------------------------------------------------------
 * SecY_Device_InsertEOP
 */
SecY_Status_t
SecY_Device_InsertEOP(
        const unsigned int DeviceId,
        const SecY_Ch_Mask_t * const ChannelMask_p)
{
    EIP164_Error_t Rc;

    LOG_INFO("\n %s \n", __func__);

#ifdef ADAPTER_EIP164_STRICT_ARGS
    if (DeviceId >= ADAPTER_EIP164_MAX_NOF_DEVICES)
        return SECY_ERROR_BAD_PARAMETER;

    if (ChannelMask_p == NULL)
        return SECY_ERROR_BAD_PARAMETER;
#endif

    SecYLib_Device_Lock(DeviceId);

    if (!PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device_StaticFields)[DeviceId].fInitialized)
    {
        SecYLib_Device_Unlock(DeviceId);
        return SECY_ERROR_INTERNAL;
    }

    LOG_INFO("\n\t EIP164_Device_InsertEOP\n");

    Rc = EIP164_Device_InsertEOP(&PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->IOArea,
                                 (const EIP164_Ch_Mask_t * const) ChannelMask_p);
    if (Rc != EIP164_NO_ERROR)
    {
        LOG_CRIT("%s: Failed to insert EOP "
                 "for EIP-164 device for device %d\n",
                 __func__, DeviceId);
        SecYLib_Device_Unlock(DeviceId);
        return SECY_DLIB_ERROR(Rc);
    }

    SecYLib_Device_Unlock(DeviceId);

    LOG_INFO("\n %s done \n", __func__);

    return SECY_STATUS_OK;
}


#ifdef ADAPTER_EIP164_CRYPT_AUTH_ENABLE
/*----------------------------------------------------------------------------
 * SecY_CryptAuth_BypassLen_Update
 */
SecY_Status_t
SecY_CryptAuth_BypassLen_Update(
        const unsigned int DeviceId,
        const unsigned int BypassLength)
{
    EIP164_Error_t Rc;

    LOG_INFO("\n %s \n", __func__);

#ifdef ADAPTER_EIP164_STRICT_ARGS
    if (DeviceId >= ADAPTER_EIP164_MAX_NOF_DEVICES)
        return SECY_ERROR_BAD_PARAMETER;
#endif

    SecYLib_Device_Lock(DeviceId);

    if (!PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device_StaticFields)[DeviceId].fInitialized)
    {
        SecYLib_Device_Unlock(DeviceId);
        return SECY_ERROR_INTERNAL;
    }

    LOG_INFO("\n\t EIP164_SecY_CryptAuth_BypassLen_Update \n");

    Rc = EIP164_SecY_CryptAuth_BypassLen_Update(&PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->IOArea,
                                                BypassLength);
    if (Rc != EIP164_NO_ERROR)
    {
        LOG_CRIT("%s: Failed to update the crypt-authenticate bypass length "
                 "for EIP-164 device for device %d\n",
                 __func__, DeviceId);
        SecYLib_Device_Unlock(DeviceId);
        return SECY_DLIB_ERROR(Rc);
    }

    SecYLib_Device_Unlock(DeviceId);

    LOG_INFO("\n %s done \n", __func__);

    return SECY_STATUS_OK;
}


/*----------------------------------------------------------------------------
 * SecY_CryptAuth_BypassLen_Get
 */
SecY_Status_t
SecY_CryptAuth_BypassLen_Get(
        const unsigned int DeviceId,
        unsigned int * const BypassLength_p)
{
    EIP164_Error_t Rc;

    LOG_INFO("\n %s \n", __func__);

#ifdef ADAPTER_EIP164_STRICT_ARGS
    if (DeviceId >= ADAPTER_EIP164_MAX_NOF_DEVICES)
        return SECY_ERROR_BAD_PARAMETER;
#endif

    SecYLib_Device_Lock(DeviceId);

    if (!PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device_StaticFields)[DeviceId].fInitialized)
    {
        SecYLib_Device_Unlock(DeviceId);
        return SECY_ERROR_INTERNAL;
    }

    LOG_INFO("\n\t EIP164_SecY_CryptAuth_BypassLen_Get \n");

    Rc = EIP164_SecY_CryptAuth_BypassLen_Get(&PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->IOArea,
                                             BypassLength_p);
    if (Rc != EIP164_NO_ERROR)
    {
        LOG_CRIT("%s: Failed to get the crypt-authenticate bypass length "
                 "for EIP-164 device for device %d\n",
                 __func__, DeviceId);
        SecYLib_Device_Unlock(DeviceId);
        return SECY_DLIB_ERROR(Rc);
    }

    SecYLib_Device_Unlock(DeviceId);

    LOG_INFO("\n %s done \n", __func__);

    return SECY_STATUS_OK;
}
#endif /* ADAPTER_EIP164_CRYPT_AUTH_ENABLE */


/*----------------------------------------------------------------------------
 * SecY_Rules_SecTag_Update
 */
SecY_Status_t
SecY_Rules_SecTag_Update(
        const unsigned int DeviceId,
        const unsigned int ChannelId,
        const SecY_Channel_Rule_SecTAG_t * const SecTag_Rules_p)
{
    EIP164_Error_t Rc;
    EIP164_Channel_Rule_SecTAG_t Dev_SecTag_Rules_p;

    LOG_INFO("\n %s \n", __func__);

#ifdef ADAPTER_EIP164_STRICT_ARGS
    if (DeviceId >= ADAPTER_EIP164_MAX_NOF_DEVICES)
        return SECY_ERROR_BAD_PARAMETER;

    if (ChannelId >= PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->ChannelCount)
        return SECY_ERROR_BAD_PARAMETER;

    if (SecTag_Rules_p == NULL)
        return SECY_ERROR_BAD_PARAMETER;
#endif

    SecYLib_Device_Lock(DeviceId);

    if (!PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device_StaticFields)[DeviceId].fInitialized)
    {
        SecYLib_Device_Unlock(DeviceId);
        return SECY_ERROR_INTERNAL;
    }

    ZEROINIT(Dev_SecTag_Rules_p);

    Dev_SecTag_Rules_p.EtherType       = SecTag_Rules_p->EtherType;
    Dev_SecTag_Rules_p.fCheckCE        = SecTag_Rules_p->fCheckCE;
    Dev_SecTag_Rules_p.fCheckKay       = SecTag_Rules_p->fCheckKay;
    Dev_SecTag_Rules_p.fCheckPN        = SecTag_Rules_p->fCheckPN;
    Dev_SecTag_Rules_p.fCheckSC        = SecTag_Rules_p->fCheckSC;
    Dev_SecTag_Rules_p.fCheckSLExt     = false;
    Dev_SecTag_Rules_p.fCheckSL        = SecTag_Rules_p->fCheckSL;
    Dev_SecTag_Rules_p.fCheckV         = SecTag_Rules_p->fCheckV;
    Dev_SecTag_Rules_p.fCompEType      = SecTag_Rules_p->fCompEType;

    LOG_INFO("\n\t EIP164_Rules_SecTag_Update \n");

    Rc = EIP164_Rules_SecTag_Update(&PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->IOArea,
                                    ChannelId,
                                    &Dev_SecTag_Rules_p);
    if (Rc != EIP164_NO_ERROR)
    {
        SecYLib_Device_Unlock(DeviceId);
        return SECY_DLIB_ERROR(Rc);
    }

    SecYLib_Device_Unlock(DeviceId);

    LOG_INFO("\n %s done \n", __func__);

    return SECY_STATUS_OK;
}


/*----------------------------------------------------------------------------
 * SecY_Channel_Bypass_Get
 */
SecY_Status_t
SecY_Channel_Bypass_Get(
        const unsigned int DeviceId,
        const unsigned int ChannelId,
        bool * const fBypass_p)
{
    EIP164_Error_t Rc;
    uint16_t Latency;
    uint8_t BurstLimit;
    bool fLatencyEnable;


    LOG_INFO("\n %s \n", __func__);

#ifdef ADAPTER_EIP164_STRICT_ARGS
    if (DeviceId >= ADAPTER_EIP164_MAX_NOF_DEVICES)
        return SECY_ERROR_BAD_PARAMETER;

    if (fBypass_p == NULL)
        return SECY_ERROR_BAD_PARAMETER;

    if (ChannelId >= PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->ChannelCount)
        return SECY_ERROR_BAD_PARAMETER;
#endif

    SecYLib_Device_Lock(DeviceId);

    if (!PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device_StaticFields)[DeviceId].fInitialized)
    {
        SecYLib_Device_Unlock(DeviceId);
        return SECY_ERROR_INTERNAL;
    }

    LOG_INFO("\n\t EIP164_Device_ChannelControl_Read \n");

    /* Read low latency bypass setting */
    Rc = EIP164_Device_ChannelControl_Read(&PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->IOArea,
                                           ChannelId,
                                           &Latency,
                                           &BurstLimit,
                                           &fLatencyEnable,
                                           fBypass_p);

    if (Rc != EIP164_NO_ERROR)
    {
        SecYLib_Device_Unlock(DeviceId);
        return SECY_DLIB_ERROR(Rc);
    }

    SecYLib_Device_Unlock(DeviceId);

    IDENTIFIER_NOT_USED(Latency);
    IDENTIFIER_NOT_USED(BurstLimit);
    IDENTIFIER_NOT_USED(fLatencyEnable);

    LOG_INFO("\n %s done \n", __func__);

    return SECY_STATUS_OK;
}


/*----------------------------------------------------------------------------
 * SecY_Channel_Bypass_Set
 */
SecY_Status_t
SecY_Channel_Bypass_Set(
        const unsigned int DeviceId,
        const unsigned int ChannelId,
        const bool fBypass)
{
    EIP164_Error_t Rc;

    LOG_INFO("\n %s \n", __func__);

#ifdef ADAPTER_EIP164_STRICT_ARGS
    if (DeviceId >= ADAPTER_EIP164_MAX_NOF_DEVICES)
        return SECY_ERROR_BAD_PARAMETER;

    if (ChannelId >= PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->ChannelCount)
        return SECY_ERROR_BAD_PARAMETER;
#endif

    SecYLib_Device_Lock(DeviceId);

    if (!PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device_StaticFields)[DeviceId].fInitialized)
    {
        SecYLib_Device_Unlock(DeviceId);
        return SECY_ERROR_INTERNAL;
    }

    LOG_INFO("\n\t EIP164_Device_ChannelControl_Write \n");

    if (fBypass)
    {
        /* Enable low latency bypass, channel-specific latency will be ignored */
        Rc = EIP164_Device_ChannelControl_Write(&PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->IOArea,
                                                ChannelId,
                                                0,     /* disable latency */
                                                0,     /* disable burst limit. */
                                                false, /* specific for channel */
                                                true   /* enable bypass */);

        if (Rc != EIP164_NO_ERROR)
        {
            SecYLib_Device_Unlock(DeviceId);
            return SECY_DLIB_ERROR(Rc);
        }
    }
    else
    {
        uint16_t Latency;
        uint8_t BurstLimit;
        bool fLatencyEnable;
        bool fTempBypass;

        /* Read channel latency settings */
        Rc = EIP164_Device_ChannelControl_Read(&PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->IOArea,
                                               ChannelId,
                                               &Latency,
                                               &BurstLimit,
                                               &fLatencyEnable,
                                               &fTempBypass);

        if (Rc != EIP164_NO_ERROR)
        {
            SecYLib_Device_Unlock(DeviceId);
            return SECY_DLIB_ERROR(Rc);
        }

        /* Disable low latency bypass, keep channel-specific latency settings */
        Rc = EIP164_Device_ChannelControl_Write(&PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->IOArea,
                                                ChannelId,
                                                Latency,
                                                BurstLimit,
                                                fLatencyEnable,
                                                false /* disable bypass */);

        if (Rc != EIP164_NO_ERROR)
        {
            SecYLib_Device_Unlock(DeviceId);
            return SECY_DLIB_ERROR(Rc);
        }
    }

    SecYLib_Device_Unlock(DeviceId);

    LOG_INFO("\n %s done \n", __func__);

    return SECY_STATUS_OK;
}


/*----------------------------------------------------------------------------
 * SecY_Channel_PacketsInflight_Get
 */
SecY_Status_t
SecY_Channel_PacketsInflight_Get(
        const unsigned int DeviceId,
        const unsigned int ChannelId,
        bool * const fInflight_p,
        uint32_t * const Mask_p)
{
    EIP164_Error_t Rc;
    uint32_t Inflight;

    LOG_INFO("\n %s \n", __func__);

#ifdef ADAPTER_EIP164_STRICT_ARGS
    if (DeviceId >= ADAPTER_EIP164_MAX_NOF_DEVICES)
        return SECY_ERROR_BAD_PARAMETER;

#endif

    SecYLib_Device_Lock(DeviceId);

    if (!PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device_StaticFields)[DeviceId].fInitialized)
    {
        SecYLib_Device_Unlock(DeviceId);
        return SECY_ERROR_INTERNAL;
    }

    LOG_INFO("\n\t EIP164_Device_Channel_PacketsInFlight_Read \n");

    /* Read channel in flight information */
    Rc = EIP164_Device_Channel_PacketsInFlight_Read(
                                 &PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->IOArea,
                                 ChannelId,
                                 &Inflight);
    if (Rc != EIP164_NO_ERROR)
    {
        SecYLib_Device_Unlock(DeviceId);
        return SECY_DLIB_ERROR(Rc);
    }

    if (fInflight_p != NULL)
    {
        *fInflight_p = (Inflight & (1<<ChannelId)) != 0;
    }

    if (Mask_p != NULL)
    {
        *Mask_p = Inflight;
    }

    SecYLib_Device_Unlock(DeviceId);

    LOG_INFO("\n %s done \n", __func__);

    return SECY_STATUS_OK;
}


/*----------------------------------------------------------------------------
 * SecY_Channel_Config_Get
 */
SecY_Status_t
SecY_Channel_Config_Get(
        const unsigned int DeviceId,
        const unsigned int ChannelId,
        SecY_Channel_t * const ChannelConfig_p)
{
    SecY_Status_t Rc;

    LOG_INFO("\n %s \n", __func__);

#ifdef ADAPTER_EIP164_STRICT_ARGS
    if (DeviceId >= ADAPTER_EIP164_MAX_NOF_DEVICES)
        return SECY_ERROR_BAD_PARAMETER;

    if (ChannelConfig_p == NULL)
        return SECY_ERROR_BAD_PARAMETER;
#endif

    SecYLib_Device_Lock(DeviceId);

    if (!PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device_StaticFields)[DeviceId].fInitialized)
    {
        SecYLib_Device_Unlock(DeviceId);
        return SECY_ERROR_INTERNAL;
    }

    Rc = SecYLib_Channel_Config_Get(DeviceId, ChannelId, ChannelConfig_p);
    if (Rc != SECY_STATUS_OK)
    {
        SecYLib_Device_Unlock(DeviceId);
        return Rc;
    }

    SecYLib_Device_Unlock(DeviceId);

    LOG_INFO("\n %s done \n", __func__);

    return SECY_STATUS_OK;
}


/*----------------------------------------------------------------------------
 * SecY_Channel_Config_Set
 */
SecY_Status_t
SecY_Channel_Config_Set(
        const unsigned int DeviceId,
        const SecY_ChannelConf_t * const ChannelConfig_p)
{
    SecY_Status_t Rc;

    LOG_INFO("\n %s \n", __func__);

#ifdef ADAPTER_EIP164_STRICT_ARGS
    if (DeviceId >= ADAPTER_EIP164_MAX_NOF_DEVICES)
        return SECY_ERROR_BAD_PARAMETER;

    if (ChannelConfig_p == NULL)
        return SECY_ERROR_BAD_PARAMETER;
#endif

    SecYLib_Device_Lock(DeviceId);

    if (!PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device_StaticFields)[DeviceId].fInitialized)
    {
        SecYLib_Device_Unlock(DeviceId);
        return SECY_ERROR_INTERNAL;
    }

    Rc = SecYLib_Channel_Config_Set(DeviceId, ChannelConfig_p);
    if (Rc != SECY_STATUS_OK)
    {
        SecYLib_Device_Unlock(DeviceId);
        return Rc;
    }

    SecYLib_Device_Unlock(DeviceId);

    LOG_INFO("\n %s done \n", __func__);

    return SECY_STATUS_OK;
}


/* end of file adapter_secy.c */
