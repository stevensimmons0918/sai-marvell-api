/* adapter_cfye_ext.c
 *
 * Extended CfyE API implementation
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

/*----------------------------------------------------------------------------
 * This module implements (provides) the following interface(s):
 */

/* Internal adapter API */
#include <Integration/Adapter_EIP163/incl/adapter_cfye_support.h>

#include <Integration/Adapter_EIP163/incl/api_cfye_ext.h>           /* Extended CfyE API */

/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

/* Logging API */
#include <Kit/Log/incl/log.h>                    /* LOG_* */

/* Driver Framework Run-Time C Library Abstraction API */
#include <Kit/DriverFramework/incl/clib.h>       /* ZEROINIT() */

/* Sleep API */
#include <Integration/Adapter_EIP164/incl/adapter_sleep.h>

#define true GT_TRUE
#define false GT_FALSE
#define __func__    __FUNCTION__


/*----------------------------------------------------------------------------
 * Definitions and macros
 */



/*----------------------------------------------------------------------------
 * Local variables
 */

/* Names of MAC addresses in the Control packet structure */
static const char *CfyE_MACAddrNames[] = {"ET Addr 0", "ET Addr 1",
                                          "ET Addr 2", "ET Addr 3",
                                          "ET Addr 4", "ET Addr 5",
                                          "ET Addr 6", "ET Addr 7",
                                          "ET Rng S0", "ET Rng E0",
                                          "ET Rng S1", "ET Rng E1",
                                          "Range Sta", "Range End",
                                          "DA 44bit ", "DA 48bit "};


/*----------------------------------------------------------------------------
 * CfyE_Device_Role_Get
 */
CfyE_Status_t
CfyE_Device_Role_Get(
        const unsigned int DeviceId,
        CfyE_Role_t * const Role_p)
{
    LOG_INFO("\n %s \n", __func__);

    ADAPTER_EIP163_CHECK_INT_ATMOST(DeviceId,
                                    ADAPTER_EIP163_MAX_NOF_DEVICES - 1);
    ADAPTER_EIP163_CHECK_POINTER(Role_p);

    CfyELib_Device_Lock(DeviceId);

    if (!CfyELib_Initialized_Check(DeviceId, __func__, true))
    {
        CfyELib_Device_Unlock(DeviceId);
        return CFYE_ERROR_INTERNAL;
    }

    *Role_p = PRV_SHARED_MACSEC_CFYE_GLOBAL_VAR_GET(CfyE_Devices)[DeviceId]->Role;

    CfyELib_Device_Unlock(DeviceId);

    LOG_INFO("\n %s done \n", __func__);

    return CFYE_STATUS_OK;
}


/**----------------------------------------------------------------------------
 * CfyE_vPort_Read
 */
CfyE_Status_t
CfyE_vPort_Read(
        const unsigned int DeviceId,
        const CfyE_vPortHandle_t vPortHandle,
        CfyE_vPort_t * const vPort_p)
{
    EIP163_Error_t EIP163_Rc;
    EIP163_vPortPolicy_t DevicePort;

    /* Fill in device-specific vPort data structure for packet classification */
    ZEROINIT(DevicePort);

    LOG_INFO("\n %s \n", __func__);

    ADAPTER_EIP163_CHECK_INT_ATMOST(DeviceId,
                                    ADAPTER_EIP163_MAX_NOF_DEVICES - 1);
    ADAPTER_EIP163_CHECK_POINTER(vPort_p);

    if (!CfyELib_vPortHandle_IsValid(vPortHandle))
        return CFYE_ERROR_BAD_PARAMETER;

    CfyELib_Device_Lock(DeviceId);

    if (!CfyELib_Initialized_Check(DeviceId, __func__, true))
    {
        CfyELib_Device_Unlock(DeviceId);
        return CFYE_ERROR_INTERNAL;
    }

    /* Add the vPort to the Classification device */
    EIP163_Rc = EIP163_vPortPolicy_Read(&PRV_SHARED_MACSEC_CFYE_GLOBAL_VAR_GET(CfyE_Devices)[DeviceId]->IOArea,
                                        CfyELib_vPortId_Get(vPortHandle),
                                        &DevicePort);
    if (EIP163_Rc != EIP163_NO_ERROR)
    {
        LOG_CRIT("%s: Failed to read vPort %p (id=%d) "
                 "for EIP-163 device %d (%s), error %d\n",
                 __func__,
                 vPortHandle,
                 CfyELib_vPortId_Get(vPortHandle),
                 DeviceId,
                 ADAPTER_EIP163_DEVICE_NAME(DeviceId),
                 EIP163_Rc);
        CfyELib_Device_Unlock(DeviceId);
        return CFYE_DLIB_ERROR(EIP163_Rc);
    }

    vPort_p->PktExtension      = DevicePort.PktExtension;
    vPort_p->SecTagOffset      = DevicePort.SecTagOffset;
    CfyELib_Device_Unlock(DeviceId);

    LOG_INFO("\n %s done \n", __func__);

    return CFYE_STATUS_OK;
}


/*----------------------------------------------------------------------------
 * CfyE_vPort_Next_Get
 */
CfyE_Status_t
CfyE_vPort_Next_Get(
        const unsigned int DeviceId,
        const CfyE_vPortHandle_t CurrentvPortHandle,
        CfyE_vPortHandle_t * const NextvPortHandle_p)
{
    unsigned int vPortId;
    LOG_INFO("\n %s \n", __func__);

    ADAPTER_EIP163_CHECK_INT_ATMOST(DeviceId,
                                    ADAPTER_EIP163_MAX_NOF_DEVICES - 1);
    ADAPTER_EIP163_CHECK_POINTER(NextvPortHandle_p);

    CfyELib_Device_Lock(DeviceId);

    if (!CfyELib_Initialized_Check(DeviceId, __func__, true))
    {
        CfyELib_Device_Unlock(DeviceId);
        return CFYE_ERROR_INTERNAL;
    }

    if (CfyELib_vPortHandle_IsSame(CurrentvPortHandle, CfyE_vPortHandle_NULL))
    {
        vPortId= 0;
    }
    else
    {
        vPortId = CfyELib_vPortId_Get(CurrentvPortHandle);
        vPortId++;
    }

    for ( ; vPortId < PRV_SHARED_MACSEC_CFYE_GLOBAL_VAR_GET(CfyE_Devices)[DeviceId]->vPortCount; vPortId++)
    {
        if (PRV_SHARED_MACSEC_CFYE_GLOBAL_VAR_GET(CfyE_Devices)[DeviceId]->vPortDscr_p[vPortId].Magic ==
            ADAPTER_EIP163_VPORT_DSCR_MAGIC)
        {

            CfyELib_Device_Unlock(DeviceId);
            return CfyE_vPortHandle_Get(DeviceId, vPortId, NextvPortHandle_p);
        }
    }

    *NextvPortHandle_p = CfyE_vPortHandle_NULL;
    CfyELib_Device_Unlock(DeviceId);
    return CFYE_STATUS_OK;
}


/*-----------------------------------------------------------------------------
 * CfyE_Rule_Read
 */
CfyE_Status_t
CfyE_Rule_Read(
        const unsigned int DeviceId,
        const CfyE_RuleHandle_t RuleHandle,
        CfyE_Rule_t * const Rule_p,
        bool * const fEnabled_p)
{
    unsigned int RuleId;

    LOG_INFO("\n %s \n", __func__);

    ADAPTER_EIP163_CHECK_INT_ATMOST(DeviceId,
                                    ADAPTER_EIP163_MAX_NOF_DEVICES - 1);
    ADAPTER_EIP163_CHECK_POINTER(Rule_p);

    if (!CfyELib_RuleHandle_IsValid(RuleHandle))
        return CFYE_ERROR_BAD_PARAMETER;

    CfyELib_Device_Lock(DeviceId);

    if (!CfyELib_Initialized_Check(DeviceId, __func__, true))
    {
        CfyELib_Device_Unlock(DeviceId);
        return CFYE_ERROR_INTERNAL;
    }

    RuleId = CfyELib_RuleId_Get(RuleHandle);

    if (PRV_SHARED_MACSEC_CFYE_GLOBAL_VAR_GET(CfyE_Devices)[DeviceId]->RuleDscr_p[RuleId].u.InUse.vPortDscr_p == NULL)
    {
        LOG_CRIT("%s: Failed, missing vPort for rule %p (id=%d) "
                 "for EIP-163 device %d (%s)\n",
                 __func__,
                 RuleHandle,
                 RuleId,
                 DeviceId,
                 ADAPTER_EIP163_DEVICE_NAME(DeviceId));
        CfyELib_Device_Unlock(DeviceId);
        return CFYE_ERROR_INTERNAL;
    }

    {
        EIP163_Error_t EIP163_Rc;
        EIP163_Rule_t DeviceRule;

        /* Fill in device-specific rule data structure for packet classification */
        ZEROINIT(DeviceRule);

        {
            unsigned int LoopCounter = ADAPTER_EIP163_MAX_NOF_SYNC_RETRY_COUNT;

            for(;;)
            {
                LoopCounter--;

                LOG_INFO("\n\t EIP163_Rule_Read \n");

                /* Read rule information */
                EIP163_Rc = EIP163_Rule_Read(&PRV_SHARED_MACSEC_CFYE_GLOBAL_VAR_GET(CfyE_Devices)[DeviceId]->IOArea,
                                             RuleId,
                                             &DeviceRule,
                                             fEnabled_p);
                if (EIP163_Rc == EIP163_NO_ERROR)
                {
                    break;
                }
                else if (EIP163_Rc != EIP163_BUSY_RETRY_LATER ||
                         LoopCounter == 0)
                {
                    LOG_CRIT("%s: Failed to update rule %p (id=%d) "
                             "for EIP-163 device %d (%s), error %d\n",
                             __func__,
                             RuleHandle,
                             RuleId,
                             DeviceId,
                             ADAPTER_EIP163_DEVICE_NAME(DeviceId),
                             EIP163_Rc);
                    CfyELib_Device_Unlock(DeviceId);
                    return CFYE_DLIB_ERROR(EIP163_Rc);
                }
                Adapter_SleepMS(ADAPTER_EIP163_SYNC_RETRY_TIMEOUT_MS);
            }
        }
        /* Copy rule key, mask and packet data */
        Rule_p->Key.NumTags     = DeviceRule.Key.NumTags;
        Rule_p->Key.PacketType  = DeviceRule.Key.PacketType;
        Rule_p->Key.ChannelID   = DeviceRule.Key.ChannelID;

        cpssOsMemCpy(Rule_p->Data,
               DeviceRule.Data,
               EIP163_RULE_NON_CTRL_WORD_COUNT * sizeof(uint32_t));

        Rule_p->Mask.NumTags    = DeviceRule.Mask.NumTags;
        Rule_p->Mask.PacketType = DeviceRule.Mask.PacketType;
        Rule_p->Mask.ChannelID  = DeviceRule.Mask.ChannelID;

        cpssOsMemCpy(Rule_p->DataMask,
               DeviceRule.DataMask,
               EIP163_RULE_NON_CTRL_WORD_COUNT * sizeof(uint32_t));

        /* Copy matching rule policy data */
        Rule_p->Policy.Priority       = DeviceRule.Policy.Priority;
        CfyELib_vPortIndexToHandle(DeviceId, DeviceRule.Policy.vPortId,
                                   &Rule_p->Policy.vPortHandle);
        Rule_p->Policy.fDrop          = DeviceRule.Policy.fDrop;
        Rule_p->Policy.fControlPacket = DeviceRule.Policy.fControlPacket;

        CfyELib_Device_Unlock(DeviceId);
        return CFYE_STATUS_OK;
    }
}


/*----------------------------------------------------------------------------
 * CfyE_Rule_Next_Get
 */
CfyE_Status_t
CfyE_Rule_Next_Get(
        const unsigned int DeviceId,
        const CfyE_RuleHandle_t CurrentRuleHandle,
        CfyE_RuleHandle_t * const NextRuleHandle_p)
{
    unsigned int RuleId;
    LOG_INFO("\n %s \n", __func__);

    ADAPTER_EIP163_CHECK_INT_ATMOST(DeviceId,
                                    ADAPTER_EIP163_MAX_NOF_DEVICES - 1);
    ADAPTER_EIP163_CHECK_POINTER(NextRuleHandle_p);

    CfyELib_Device_Lock(DeviceId);

    if (!CfyELib_Initialized_Check(DeviceId, __func__, true))
    {
        CfyELib_Device_Unlock(DeviceId);
        return CFYE_ERROR_INTERNAL;
    }

    if (CfyELib_RuleHandle_IsSame(CurrentRuleHandle, CfyE_RuleHandle_NULL))
    {
        RuleId= 0;
    }
    else
    {
        RuleId = CfyELib_RuleId_Get(CurrentRuleHandle);
        RuleId++;
    }

    for ( ; RuleId < PRV_SHARED_MACSEC_CFYE_GLOBAL_VAR_GET(CfyE_Devices)[DeviceId]->RulesCount; RuleId++)
    {
        if (PRV_SHARED_MACSEC_CFYE_GLOBAL_VAR_GET(CfyE_Devices)[DeviceId]->RuleDscr_p[RuleId].Magic ==
            ADAPTER_EIP163_RULE_DSCR_MAGIC)
        {

            CfyELib_Device_Unlock(DeviceId);
            return CfyE_RuleHandle_Get(DeviceId, RuleId, NextRuleHandle_p);
        }
    }

    *NextRuleHandle_p = CfyE_RuleHandle_NULL;
    CfyELib_Device_Unlock(DeviceId);
    return CFYE_STATUS_OK;
}


/*----------------------------------------------------------------------------
 * CfyE_Rule_vPort_Next_Get
 */
CfyE_Status_t
CfyE_Rule_vPort_Next_Get(
        const unsigned int DeviceId,
        const CfyE_vPortHandle_t vPortHandle,
        const CfyE_RuleHandle_t CurrentRuleHandle,
        CfyE_RuleHandle_t * const NextRuleHandle_p)
{
    unsigned int RuleId;
    CfyE_vPort_Descriptor_Internal_t * vPortDscr_p =
                                (CfyE_vPort_Descriptor_Internal_t*)vPortHandle;
    LOG_INFO("\n %s \n", __func__);

    ADAPTER_EIP163_CHECK_INT_ATMOST(DeviceId,
                                    ADAPTER_EIP163_MAX_NOF_DEVICES - 1);
    ADAPTER_EIP163_CHECK_POINTER(NextRuleHandle_p);

    CfyELib_Device_Lock(DeviceId);

    if (!CfyELib_Initialized_Check(DeviceId, __func__, true))
    {
        CfyELib_Device_Unlock(DeviceId);
        return CFYE_ERROR_INTERNAL;
    }

    if (CfyELib_RuleHandle_IsSame(CurrentRuleHandle, CfyE_RuleHandle_NULL))
    {
        RuleId= 0;
    }
    else
    {
        RuleId = CfyELib_RuleId_Get(CurrentRuleHandle);
        RuleId++;
    }

    for ( ; RuleId < PRV_SHARED_MACSEC_CFYE_GLOBAL_VAR_GET(CfyE_Devices)[DeviceId]->RulesCount; RuleId++)
    {
        if (PRV_SHARED_MACSEC_CFYE_GLOBAL_VAR_GET(CfyE_Devices)[DeviceId]->RuleDscr_p[RuleId].Magic ==
            ADAPTER_EIP163_RULE_DSCR_MAGIC &&
            PRV_SHARED_MACSEC_CFYE_GLOBAL_VAR_GET(CfyE_Devices)[DeviceId]->RuleDscr_p[RuleId].u.InUse.vPortDscr_p ==
            vPortDscr_p)
        {

            CfyELib_Device_Unlock(DeviceId);
            return CfyE_RuleHandle_Get(DeviceId, RuleId, NextRuleHandle_p);
        }
    }

    *NextRuleHandle_p = CfyE_RuleHandle_NULL;
    CfyELib_Device_Unlock(DeviceId);
    return CFYE_STATUS_OK;
}


/*----------------------------------------------------------------------------
 * CfyE_Diag_Device_Dump
 */
CfyE_Status_t
CfyE_Diag_Device_Dump(
        const unsigned int DeviceId)
{
    CfyE_Status_t CfyE_Rc;
    unsigned int RuleCount;
    unsigned int MaxCfyEChannels = 0;

    /* CfyE_Device_Limits */
    {
        CfyE_Role_t Role;
        CfyE_Device_Limits_t Device_Limits;

        CfyE_Rc = CfyE_Device_Role_Get(DeviceId, &Role);
        if (CfyE_Rc != CFYE_STATUS_OK)
        {
            LOG_CRIT("CfyE_Device_Role_Get returned error %d\n",CfyE_Rc);
            return CfyE_Rc;
        }

        CfyE_Rc = CfyE_Device_Limits_Get(DeviceId,
                                         &Device_Limits);
        if (CfyE_Rc != CFYE_STATUS_OK)
        {
            LOG_CRIT("CfyE_Device_Limits returned error %d\n",CfyE_Rc);
            return CfyE_Rc;
        }
        RuleCount = Device_Limits.rule_count;
        Log_FormattedMessage("CfyE_Diag_Device Role=%s version=%d.%d.%d"
                             "Channels=%d vPorts=%d Rules=%d\n",
                             Role==CFYE_ROLE_INGRESS?"INGRESS":
                             (Role==CFYE_ROLE_EGRESS?"EGRESS":"INGRESS-EGRESS"),
                             Device_Limits.major_version,
                             Device_Limits.minor_version,
                             Device_Limits.patch_level,
                             Device_Limits.channel_count,
                             Device_Limits.vport_count,
                             Device_Limits.rule_count);
    }

    /* CfyE_Device_Config_Get */
    {
        CfyE_Device_t DeviceParams;
        CfyE_Statistics_Control_t StatsParams;
        CfyE_EOPConf_t EOPParams;
        CfyE_ECCConf_t ECCParams;
        uint16_t i = 0;

        CfyE_Rc = CfyE_Device_Limits(DeviceId,
                                     &MaxCfyEChannels,
                                     NULL,
                                     NULL);
        if (CfyE_Rc != CFYE_STATUS_OK)
        {
            LOG_CRIT("CfyE_Device_Limits returned error %d\n", CfyE_Rc);

            return CfyE_Rc;
        }

        ZEROINIT(DeviceParams);
        DeviceParams.StatControl_p = &StatsParams;
        DeviceParams.EOPConf_p = &EOPParams;
        DeviceParams.ECCConf_p = &ECCParams;

        CfyE_Rc = CfyE_Device_Config_Get(DeviceId,
                                         0, /*ignored, non-channel fields*/
                                         &DeviceParams);
        if (CfyE_Rc != CFYE_STATUS_OK)
        {
            LOG_CRIT("CfyE_Device_Config_Get returned error %d\n",CfyE_Rc);
            return CfyE_Rc;
        }

        Log_FormattedMessage(
            "Statistics Control: fAutoStatCntrsReset=%d CountIncDisCtrl=0x%x\n",
            StatsParams.fAutoStatCntrsReset,
            StatsParams.CountIncDisCtrl);
        Log_FormattedMessage("CountFrameThr (lo=%10u,hi=%10u)\n",
                             StatsParams.CountFrameThr.Lo,
                             StatsParams.CountFrameThr.Hi);
        Log_FormattedMessage("ChanCountFrameThr (lo=%10u,hi=%10u)\n",
                             StatsParams.ChanCountFrameThr.Lo,
                             StatsParams.ChanCountFrameThr.Hi);

        Log_FormattedMessage("EOP config: EOPTimeoutVal = %u\n",
                             EOPParams.EOPTimeoutVal);

        for (i = 0; i < ((MaxCfyEChannels + 31) / 32); i++)
        {
            Log_FormattedMessage("EOP config: EOPTimeoutCtrl=0x%x\n",
                                 EOPParams.EOPTimeoutCtrl.ch_bitmask[i]);
        }

        Log_FormattedMessage(
            "ECC config: ECCCorrectable=%d ECCUncorrectable=%d\n",
            ECCParams.ECCCorrectableThr,
            ECCParams.ECCUncorrectableThr);
    }

    /* CfyE_Device_Status_Get */
    {
        CfyE_ECCStatus_t ECCStatus;
        CfyE_PktProcessDebug_t PktProcessDebug;
        CfyE_DeviceStatus_t DeviceStatus;
        unsigned int i;
        bool fExternalTCAM;

        ZEROINIT(DeviceStatus);
        ZEROINIT(PktProcessDebug);
        DeviceStatus.PktProcessDebug_p = &PktProcessDebug;
        DeviceStatus.ECCStatus_p = &ECCStatus;
        DeviceStatus.fExternalTCAM_p = &fExternalTCAM;
        CfyE_Rc = CfyE_Device_Status_Get(
            DeviceId,
            &DeviceStatus);
        if (CfyE_Rc != CFYE_STATUS_OK)
        {
            LOG_CRIT("CfyE_Device_Status_Get returned error %d\n",CfyE_Rc);
            return CfyE_Rc;
        }
        for (i = 0; i < CFYE_ECC_NOF_STATUS_COUNTERS; i++)
        {
            Log_FormattedMessage(
                "ECC counter %u: correctable=%u thr=%d"
                " uncorrectable=%u thr=%d\n",
                i,
                ECCStatus.Counters[i].CorrectableCount,
                ECCStatus.Counters[i].fCorrectableThr,
                ECCStatus.Counters[i].UncorrectableCount,
                ECCStatus.Counters[i].fUncorrectableThr);
        }
        Log_FormattedMessage(
            "PktProcess CPMatchDeb=0x%08x TCAMDeb=0x%08x SecTAGDeb=0x%08x\n"
            "           SAMPPDeb1=0x%08x SAMPPDeb2=0x%08x SAMPPDeb3=0x%08x\n"
            "           ParsedDALo=0x%08x ParsedDAHi=0x%08x\n"
            "           ParsedSALo=0x%08x ParsedSAHi=0x%08x\n"
            "           ParsedSecTAGLo=0x%08x ParsedSecTAGHi=0x%08x\n"
            "           DebFlowLookup=0x%08x\n",
            PktProcessDebug.CPMatchDebug,
            PktProcessDebug.TCAMDebug,
            PktProcessDebug.SecTAGDebug,
            PktProcessDebug.SAMPPDebug1,
            PktProcessDebug.SAMPPDebug2,
            PktProcessDebug.SAMPPDebug3,
            PktProcessDebug.ParsedDALo,
            PktProcessDebug.ParsedDAHi,
            PktProcessDebug.ParsedSALo,
            PktProcessDebug.ParsedSAHi,
            PktProcessDebug.ParsedSecTAGLo,
            PktProcessDebug.ParsedSecTAGHi,
            PktProcessDebug.DebugFlowLookup);
        Log_FormattedMessage(
            "External TCAM: %s\n",fExternalTCAM?"Yes":"No");
    }

    /* Summary registers. */
    {
        CfyE_Ch_Mask_t ChannelSummary;
        uint32_t TCAMSummary[32];
        unsigned int SummaryCount = MIN(RuleCount/32, 32);
        uint16_t i = 0;

        ZEROINIT(ChannelSummary);

        CfyE_Rc = CfyE_Statistics_Summary_Channel_Read(DeviceId,
                                                       &ChannelSummary);
        if (CfyE_Rc != CFYE_STATUS_OK)
        {
            LOG_CRIT("CfyE_Statistics_Summary_Channel_Read returned error %d\n",
                     CfyE_Rc);
            return CfyE_Rc;
        }

        for (i = 0; i < ((MaxCfyEChannels + 31) / 32); i++)
        {
            Log_FormattedMessage("Channel Summary 0x%08x\n",
                                 ChannelSummary.ch_bitmask[i]);
        }

        CfyE_Rc = CfyE_Statistics_Summary_TCAM_Read(DeviceId,
                                                    0,
                                                    TCAMSummary,
                                                    SummaryCount);
        if (CfyE_Rc != CFYE_STATUS_OK)
        {
            LOG_CRIT("CfyE_Statistics_Summary_TCAM_Read returned error %d\n",
                     CfyE_Rc);
            return CfyE_Rc;
        }

        Log_HexDump32("TCAM summary", 0, TCAMSummary, SummaryCount);
    }

    return CFYE_STATUS_OK;
}

/*----------------------------------------------------------------------------
 * CfyE_Diag_Channel_Dump
 */
CfyE_Status_t
CfyE_Diag_Channel_Dump(
        const unsigned int DeviceId,
        const unsigned int ChannelId,
        const bool fAllChannels)
{
    CfyE_Status_t CfyE_Rc;
    if (fAllChannels)
    {
        unsigned int ChannelCount;
        unsigned int i;
        CfyE_Rc = CfyE_Device_Limits(DeviceId,
                                     &ChannelCount,
                                     NULL,
                                     NULL);
        if (CfyE_Rc != CFYE_STATUS_OK)
        {
            LOG_CRIT("CfyE_Device_Limits returned error %d\n", CfyE_Rc);
            return CfyE_Rc;
        }
        for (i=0; i<ChannelCount; i++)
        {
            CfyE_Rc = CfyE_Diag_Channel_Dump(DeviceId, i, false);
            if (CfyE_Rc != CFYE_STATUS_OK)
            {
                LOG_CRIT("CfyE_Diag_Channel_Dump"
                         " returned error %d for channel %d\n", CfyE_Rc,i);
                return CfyE_Rc;
            }
        }
    }
    else
    {
        CfyE_Device_t DeviceParams;
        CfyE_Device_Control_t DevCtrl;
        CfyE_Device_Exceptions_t DevExceptions;

        CfyE_ControlPacket_t CP;
        uint8_t EtherAddr[16][6];

        CfyE_HeaderParser_t HeaderParser;
        CfyE_SecTAG_Parser_t SecTAG_Parser;
        CfyE_VLAN_Parser_t VLAN_Parser;
        CfyE_Statistics_Channel_t Chan_Stat;
        unsigned int i;
        ZEROINIT(SecTAG_Parser);

        ZEROINIT(DeviceParams);
        ZEROINIT(EtherAddr);
        ZEROINIT(CP);
        ZEROINIT(HeaderParser);
        ZEROINIT(VLAN_Parser);
        ZEROINIT(Chan_Stat);
        ZEROINIT(DevExceptions);

        DeviceParams.Control_p = &DevCtrl;
        DevCtrl.Exceptions_p = &DevExceptions;


        /* Make all Ethernet address fields in CP structure point to
           valid buffers */
        for (i=0; i<8; i++)
            CP.MAC_DA_ET_Rules[i].MAC_DA_p = EtherAddr[i];

        CP.MAC_DA_ET_Range[0].Range.MAC_DA_Start_p = EtherAddr[8];
        CP.MAC_DA_ET_Range[0].Range.MAC_DA_End_p = EtherAddr[9];
        CP.MAC_DA_ET_Range[1].Range.MAC_DA_Start_p = EtherAddr[10];
        CP.MAC_DA_ET_Range[1].Range.MAC_DA_End_p = EtherAddr[11];
        CP.MAC_DA_Range.MAC_DA_Start_p = EtherAddr[12];
        CP.MAC_DA_Range.MAC_DA_End_p = EtherAddr[13];
        CP.MAC_DA_44Bit_Const_p = EtherAddr[14];
        CP.MAC_DA_48Bit_Const_p = EtherAddr[15];
        DeviceParams.CP_p = &CP;

        DeviceParams.HeaderParser_p = &HeaderParser;
        HeaderParser.SecTAG_Parser_p = &SecTAG_Parser;

        HeaderParser.VLAN_Parser_p = &VLAN_Parser;
        Log_FormattedMessage("CfyE_Diag_Channel device=%d channel=%d\n",
                             DeviceId, ChannelId);

        CfyE_Rc = CfyE_Device_Config_Get(DeviceId,
                                         ChannelId,
                                         &DeviceParams);
        if (CfyE_Rc != CFYE_STATUS_OK)
        {
            LOG_CRIT("CfyE_Device_Config_Get returned error %d\n",CfyE_Rc);
            return CfyE_Rc;
        }
        Log_FormattedMessage(
            "DevCtrl fLowLatencyBypass=%d fDefaultVPortValid=%d"
            " DefaultVPort=%u\n"
            "fForceDrop=%d  Dropaction=%s\n",
            DevCtrl.fLowLatencyBypass,
            DevExceptions.fDefaultVPortValid,
            DevExceptions.DefaultVPort,
            DevExceptions.fForceDrop,
            DevExceptions.DropAction==CFYE_DROP_CRC_ERROR?"CRC ERR":
            (DevExceptions.DropAction==CFYE_DROP_PKT_ERROR?"PKT ERR":
             (DevExceptions.DropAction==CFYE_DROP_INTERNAL?"INTERNAL":"NONE")));
        Log_FormattedMessage(
            "CtrlPkt  CPMatchModeMask=0x%x  CPMatchEnableMask 0x%x\n",
            CP.CPMatchModeMask, CP.CPMatchEnableMask);
        for (i=0; i<16; i++)
        {
            Log_HexDump(CfyE_MACAddrNames[i],
                        0,
                        EtherAddr[i],
                        6);
        }
        for(i=0; i<8; i++)
        {
            Log_FormattedMessage("        EType%d = 0x%04x\n",
                                 i,
                                 CP.MAC_DA_ET_Rules[i].EtherType);
        }
        for(i=0; i<2; i++)
        {
            Log_FormattedMessage(" Range ETypee%d = 0x%04x\n",
                                 i,
                                 CP.MAC_DA_ET_Range[i].EtherType);
        }

        Log_FormattedMessage(
            "SecTAG Parser fCompType=%d fCheckVersion=%d fCheckKay=%d\n"
            " fLookupUseSCI=%d MACsecTagValue=0x%04x\n",
            SecTAG_Parser.fCompType,
            SecTAG_Parser.fCheckVersion,
            SecTAG_Parser.fCheckKay,
            SecTAG_Parser.fLookupUseSCI,
            SecTAG_Parser.MACsecTagValue);

        Log_FormattedMessage(
            "VLAN Parser DefaultUp=%d QTag=0x%04x"
            " STag1=0x%04x STag2=0x%04x STag3=0x%04x\n"
            "CP.fParseQinQ=%d CP.fParseQTag=%d\n"
            "CP.fParseStag1=%d CP.fParseStag2=%d CP.fParseStag3=%d\n"
            "fQTagUpEnable=%d fSTagUpEnable=%d\n",
            VLAN_Parser.DefaultUp,
            VLAN_Parser.QTag,
            VLAN_Parser.STag1,
            VLAN_Parser.STag2,
            VLAN_Parser.STag3,
            VLAN_Parser.CP.fParseQinQ,
            VLAN_Parser.CP.fParseQTag,
            VLAN_Parser.CP.fParseStag1,
            VLAN_Parser.CP.fParseStag2,
            VLAN_Parser.CP.fParseStag3,
            VLAN_Parser.fQTagUpEnable,
            VLAN_Parser.fSTagUpEnable);

        for (i = 0; i < CFYE_VLAN_UP_MAX_COUNT; i++)
        {
            Log_FormattedMessage("VLAN %d UpTable1=%d UpTable2 = %d\n",
                                 i,
                                 VLAN_Parser.UpTable1[i],
                                 VLAN_Parser.UpTable2[i]);
        }

        CfyE_Rc = CfyE_Statistics_Channel_Get(DeviceId,
                                              ChannelId,
                                              &Chan_Stat,
                                              false);
        if (CfyE_Rc != CFYE_STATUS_OK)
        {
            LOG_CRIT("CfyE_Statistics_Channel_Get returned error %d\n",
                     CfyE_Rc);
            return CfyE_Rc;
        }

        Log_FormattedMessage(
            "Chan Stats TCAMHitMultiple         lo=%10u hi=%10u\n"
            "           HeaderParserDroppedPkts lo=%10u hi=%10u\n"
            "           TCAMMiss                lo=%10u hi=%10u\n"
            "           PktsCtrl                lo=%10u hi=%10u\n"
            "           PktsData                lo=%10u hi=%10u\n"
            "           PktsDropped             lo=%10u hi=%10u\n"
            "           PktsErrIn               lo=%10u hi=%10u\n",
            Chan_Stat.TCAMHitMultiple.Lo,
            Chan_Stat.TCAMHitMultiple.Hi,
            Chan_Stat.HeaderParserDroppedPkts.Lo,
            Chan_Stat.HeaderParserDroppedPkts.Hi,
            Chan_Stat.TCAMMiss.Lo,
            Chan_Stat.TCAMMiss.Hi,
            Chan_Stat.PktsCtrl.Lo,
            Chan_Stat.PktsCtrl.Hi,
            Chan_Stat.PktsData.Lo,
            Chan_Stat.PktsData.Hi,
            Chan_Stat.PktsDropped.Lo,
            Chan_Stat.PktsDropped.Hi,
            Chan_Stat.PktsErrIn.Lo,
            Chan_Stat.PktsErrIn.Hi);
    }

    return CFYE_STATUS_OK;
}


/*----------------------------------------------------------------------------
 * CfyE_Diag_vPort_Dump
 */
CfyE_Status_t
CfyE_Diag_vPort_Dump(
        const unsigned int DeviceId,
        const CfyE_vPortHandle_t vPortHandle,
        const bool fAllvPorts,
        const bool fIncludeRule)
{
    CfyE_Status_t CfyE_Rc;

    if (fAllvPorts)
    {
        unsigned int i, vPortCount;
        CfyE_vPortHandle_t vPortHandle1,vPortHandle2;
        CfyE_Rc = CfyE_Device_Limits(DeviceId,
                                     NULL,
                                     &vPortCount,
                                     NULL);
        if (CfyE_Rc != CFYE_STATUS_OK)
        {
            LOG_CRIT("CfyE_Device_Limits returned error %d\n", CfyE_Rc);
            return CfyE_Rc;
        }

        vPortHandle1 = CfyE_vPortHandle_NULL;
        vPortHandle2 = CfyE_vPortHandle_NULL;

        for (i=0; i<vPortCount; i++)
        {
            CfyE_Rc = CfyE_vPort_Next_Get(DeviceId,
                                          vPortHandle1,
                                          &vPortHandle2);

            if (CfyE_Rc != CFYE_STATUS_OK)
            {
                LOG_CRIT("CfyE_vPort_Next_Get returned error %d\n",CfyE_Rc);
                return CfyE_Rc;
            }

            if (CfyE_vPortHandle_IsSame(vPortHandle2, CfyE_vPortHandle_NULL))
            {
                break;
            }

            CfyE_Rc = CfyE_Diag_vPort_Dump(DeviceId,
                                           vPortHandle2,
                                           false,
                                           fIncludeRule);
             if (CfyE_Rc != CFYE_STATUS_OK)
            {
                LOG_CRIT("CfyE_Diag_vPort_Dump returned error %d\n", CfyE_Rc);
                return CfyE_Rc;
            }

            vPortHandle1 = vPortHandle2;
        }
    }
    else
    {
        unsigned int vPortId;
        CfyE_vPort_t vPort;

        CfyE_Rc = CfyE_vPortIndex_Get(vPortHandle, &vPortId);
        if (CfyE_Rc != CFYE_STATUS_OK)
        {
            LOG_CRIT("CfyE_Diag_vPort_Dump returned error %d\n", CfyE_Rc);
            return CfyE_Rc;
        }

        Log_FormattedMessage("CfyE_Diag_vPort #%d\n", vPortId);

        CfyE_Rc = CfyE_vPort_Read(DeviceId, vPortHandle, &vPort);
        if (CfyE_Rc != CFYE_STATUS_OK)
        {
            LOG_CRIT("CfyE_vPort_Read returned error %d\n", CfyE_Rc);
            return CfyE_Rc;
        }

        Log_FormattedMessage(" PktExtension=%d SecTagOffset=%d\n",
                             vPort.PktExtension, vPort.SecTagOffset);


        if (fIncludeRule)
        {
            CfyE_RuleHandle_t RuleHandle1, RuleHandle2;
            RuleHandle1 = CfyE_RuleHandle_NULL;

            for (;;)
            {
                CfyE_Rc = CfyE_Rule_vPort_Next_Get(DeviceId,
                                                   vPortHandle,
                                                   RuleHandle1,
                                                   &RuleHandle2);
                if (CfyE_Rc != CFYE_STATUS_OK)
                {
                    LOG_CRIT("CfyE_Rule_Next_Get returned error %d\n", CfyE_Rc);
                    return CfyE_Rc;
                }
                if (CfyE_RuleHandle_IsSame(RuleHandle2, CfyE_RuleHandle_NULL))
                {
                    break;
                }
                CfyE_Rc = CfyE_Diag_Rule_Dump(DeviceId, RuleHandle2, false);
                if (CfyE_Rc != CFYE_STATUS_OK)
                {
                    LOG_CRIT("CfyE_Diag_Rule_Dump returned error %d\n",
                             CfyE_Rc);
                    return CfyE_Rc;
                }

                RuleHandle1 = RuleHandle2;
            }
        }
    }
    return CFYE_STATUS_OK;
}


/*----------------------------------------------------------------------------
 * CfyE_Diag_Rule_Dump
 */
CfyE_Status_t
CfyE_Diag_Rule_Dump(
        const unsigned int DeviceId,
        const CfyE_RuleHandle_t RuleHandle,
        const bool fAllRules)
{
    CfyE_Status_t CfyE_Rc;
    if (fAllRules)
    {
        unsigned int i, RuleCount;
        CfyE_RuleHandle_t RuleHandle1,RuleHandle2;
        CfyE_Rc = CfyE_Device_Limits(DeviceId,
                                     NULL,
                                     NULL,
                                     &RuleCount);
        if (CfyE_Rc != CFYE_STATUS_OK)
        {
            LOG_CRIT("CfyE_Device_Limits returned error %d\n", CfyE_Rc);
            return CfyE_Rc;
        }

        RuleHandle1 = CfyE_RuleHandle_NULL;

        for (i=0; i<RuleCount; i++)
        {
            CfyE_Rc = CfyE_Rule_Next_Get(DeviceId,
                                         RuleHandle1,
                                         &RuleHandle2);
            if (CfyE_Rc != CFYE_STATUS_OK)
            {
                LOG_CRIT("CfyE_Rule_Next_Get returned error %d\n", CfyE_Rc);
                return CfyE_Rc;
            }
            if (CfyE_RuleHandle_IsSame(RuleHandle2, CfyE_RuleHandle_NULL))
            {
                break;
            }
            CfyE_Rc = CfyE_Diag_Rule_Dump(DeviceId, RuleHandle2, false);
            if (CfyE_Rc != CFYE_STATUS_OK)
            {
                LOG_CRIT("CfyE_Diag_Rule_Dump returned error %d\n", CfyE_Rc);
                return CfyE_Rc;
            }

            RuleHandle1 = RuleHandle2;
        }
    }
    else
    {
        unsigned int RuleId = 0, vPortId = 0;
        CfyE_Statistics_TCAM_t TCAM_Stat;
        CfyE_Rule_t Rule;
        bool fEnabled = false;

        ZEROINIT(Rule);

        CfyE_Rc = CfyE_RuleIndex_Get(RuleHandle, &RuleId);
        if (CfyE_Rc != CFYE_STATUS_OK)
        {
            LOG_CRIT("CfyE_RuleIndex_Get returned error %d\n", CfyE_Rc);
            return CfyE_Rc;
        }
        Log_FormattedMessage("CfyE_Diag_Rule #%d\n", RuleId);
        CfyE_Rc = CfyE_Rule_Read(DeviceId, RuleHandle, &Rule, &fEnabled);
        if (CfyE_Rc != CFYE_STATUS_OK)
        {
            LOG_CRIT("CfyE_Rule_Read returned error %d\n", CfyE_Rc);
            return CfyE_Rc;
        }
        CfyE_Rc = CfyE_vPortIndex_Get(Rule.Policy.vPortHandle, &vPortId);
        if (CfyE_Rc != CFYE_STATUS_OK)
        {
            LOG_CRIT("CfyE_vPortIndex_Get returned error %d\n", CfyE_Rc);
            return CfyE_Rc;
        }
        Log_FormattedMessage("Rule enabled=%d Policy vPortId=%d"
                             " Priority=%d fDrop=%d fControlPacket=%d\n",
                             fEnabled,
                             vPortId,
                             Rule.Policy.Priority,
                             Rule.Policy.fDrop,
                             Rule.Policy.fControlPacket);
        Log_FormattedMessage(
            "Packet type=%s (mask=0x%x) \n"
            " NumTags=0x%02x (mask=0x%02x) \n"
            " ChannelID=%d (mask=0x%02x) \n",
            Rule.Key.PacketType==CFYE_RULE_PKT_TYPE_OTHER? "OTHER":
            (Rule.Key.PacketType==CFYE_RULE_PKT_TYPE_PBB? "PBB":
             (Rule.Key.PacketType==CFYE_RULE_PKT_TYPE_MPLS? "MPLS" : "MACSEC")),
            Rule.Mask.PacketType,
            Rule.Key.NumTags,
            Rule.Mask.NumTags,
            Rule.Key.ChannelID,
            Rule.Mask.ChannelID);
        Log_HexDump32("Data     ",
                      0,
                      Rule.Data,
                      CFYE_RULE_NON_CTRL_WORD_COUNT);
        Log_HexDump32("DataMask ",
                      0,
                      Rule.DataMask,
                      CFYE_RULE_NON_CTRL_WORD_COUNT);

        CfyE_Rc = CfyE_Statistics_TCAM_Get(DeviceId,
                                           RuleId,
                                           &TCAM_Stat,
                                           false);
        if (CfyE_Rc != CFYE_STATUS_OK)
        {
            LOG_CRIT("CfyE_Statistics_TCAM_Get returned error %d\n", CfyE_Rc);
            return CfyE_Rc;
        }
        Log_FormattedMessage("TCAM Stats Counter lo=%10u hi=%10u\n",
                             TCAM_Stat.Counter.Lo,
                             TCAM_Stat.Counter.Hi);
    }

    return CFYE_STATUS_OK;
}


/* end of file adapter_cfye_ext.c */
