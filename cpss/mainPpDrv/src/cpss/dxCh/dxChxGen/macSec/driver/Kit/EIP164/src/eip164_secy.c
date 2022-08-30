/* eip164_secy_sa.c
 *
 * EIP-164 Driver Library SecY SA implementation
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

#include <Kit/EIP164/incl/eip164_secy.h>


/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

/* Default configuration */
#include <Kit/EIP164/incl/c_eip164.h>

/* Driver Framework Basic Definitions API */
#include <Kit/DriverFramework/incl/basic_defs.h>             /* IDENTIFIER_NOT_USED, bool, uint32_t */

/* Driver Framework Device API */
#include <Kit/DriverFramework/incl/device_types.h>           /* Device_Handle_t */

/* EIP-164 register interface */
#include <Kit/EIP164/incl/eip164_level0.h>

/* EIP-66 register interface */
#include <Kit/EIP164/incl/eip66_level0.h>

/* EIP-164 Internal interface */
#include <Kit/EIP164/incl/eip164_internal.h>


/*----------------------------------------------------------------------------
 * Definitions and macros
 */


/*----------------------------------------------------------------------------
 * Local variables
 */

/*----------------------------------------------------------------------------
 * EIP164Lib_Counter_Read
 */
static int
EIP164Lib_Counter_Read(
        Device_Handle_t Device,
        unsigned int Offset,
        unsigned int Index,
        uint32_t * const CountLo_p,
        uint32_t * const CountHi_p,
        const bool fClear)
{
    int rc = EIP217_COUNTER_64_RD(Device,
                                  Offset,
                                  Index,
                                  CountLo_p,
                                  CountHi_p);
    if (rc) return rc;

    if(fClear && (*CountLo_p != 0 || *CountHi_p != 0))
    {
        /* Write the same value back when the counter is nonzero, this
           will decrement the counter by that amount. */
        rc = EIP217_COUNTER_64_WR(Device,
                                  Offset,
                                  Index,
                                  *CountLo_p,
                                  *CountHi_p);
    }
    return rc;
}


/*----------------------------------------------------------------------------
 * EIP164_SecY_SA_Write
 */
EIP164_Error_t
EIP164_SecY_SA_Write(
        const EIP164_IOArea_t * const IOArea_p,
        const unsigned int SAIndex,
        const uint32_t * Transform_p,
        const unsigned int WordCount)
{
    Device_Handle_t Device;
    const volatile EIP164_True_IOArea_t * const TrueIOArea_p = IOAREA_IN(IOArea_p);
    int rc = 0;

    EIP164_CHECK_POINTER(IOArea_p);
    EIP164_CHECK_POINTER(Transform_p);
    EIP164_CHECK_INT_ATMOST(SAIndex + 1, TrueIOArea_p->MaxSACount);

    Device = TrueIOArea_p->Device;

    rc = EIP164_XFORM_REC_WR(Device,
                             SAIndex,
                             Transform_p,
                             WordCount);

    if (rc) return rc;

    return EIP164_NO_ERROR;
}

/*----------------------------------------------------------------------------
 * EIP164_SecY_SC_SA_MAP_E_Update
 */
EIP164_Error_t
EIP164_SecY_SC_SA_Map_E_Update(
        const EIP164_IOArea_t * const IOArea_p,
        const unsigned int SCIndex,
        const unsigned int SAIndex,
        const bool fSAInUse,
        const bool fIndexUpdate)
{
    Device_Handle_t Device;
    const volatile EIP164_True_IOArea_t * const TrueIOArea_p = IOAREA_IN(IOArea_p);
    int rc = 0;

    EIP164_CHECK_POINTER(IOArea_p);
    EIP164_CHECK_INT_ATMOST(SAIndex + 1, TrueIOArea_p->MaxSACount);
    EIP164_CHECK_INT_ATMOST(SCIndex + 1, TrueIOArea_p->MaxSCCount);

    Device = TrueIOArea_p->Device;

    rc = EIP164_SC_SA_MAP1_WR(Device,
                              SCIndex,
                              SAIndex,
                              fSAInUse,
                              fIndexUpdate,
                              0,
                              false,
                              false);
    if (rc) return rc;

    return EIP164_NO_ERROR;
}


/*----------------------------------------------------------------------------
 * EIP164_SecY_SC_SA_Map_I_Update
 */
EIP164_Error_t
EIP164_SecY_SC_SA_Map_I_Update(
        const EIP164_IOArea_t * const IOArea_p,
        const unsigned int SCIndex,
        const unsigned int AN,
        const unsigned int SAIndex,
        const bool fSAInUse)
{
    Device_Handle_t Device;
    const volatile EIP164_True_IOArea_t * const TrueIOArea_p = IOAREA_IN(IOArea_p);
    unsigned int SAIndex0, SAIndex1;
    bool fSAInUse0, fSAInUse1;
    int rc = 0;

    EIP164_CHECK_POINTER(IOArea_p);
    EIP164_CHECK_INT_ATMOST(SAIndex + 1, TrueIOArea_p->MaxSACount);
    EIP164_CHECK_INT_ATMOST(SCIndex + 1, TrueIOArea_p->MaxSCCount);

    Device = TrueIOArea_p->Device;
    switch(AN)
    {
    case 0:
        rc = EIP164_SC_SA_MAP1_RD(Device,
                                  SCIndex,
                                  &SAIndex0,
                                  &fSAInUse0,
                                  &SAIndex1,
                                  &fSAInUse1);
        if (rc) return rc;

        SAIndex0 = SAIndex;
        fSAInUse0 = fSAInUse;

        rc = EIP164_SC_SA_MAP1_WR(Device,
                                  SCIndex,
                                  SAIndex0,
                                  fSAInUse0,
                                  false,
                                  SAIndex1,
                                  fSAInUse1,
                                  false);
        if (rc) return rc;
        break;

    case 1:
        rc = EIP164_SC_SA_MAP1_RD(Device,
                                  SCIndex,
                                  &SAIndex0,
                                  &fSAInUse0,
                                  &SAIndex1,
                                  &fSAInUse1);
        if (rc) return rc;

        SAIndex1 = SAIndex;
        fSAInUse1 = fSAInUse;

        rc = EIP164_SC_SA_MAP1_WR(Device,
                                  SCIndex,
                                  SAIndex0,
                                  fSAInUse0,
                                  false,
                                  SAIndex1,
                                  fSAInUse1,
                                  false);
        if (rc) return rc;
        break;

    case 2:
        rc = EIP164_SC_SA_MAP2_RD(Device,
                                  SCIndex,
                                  &SAIndex0,
                                  &fSAInUse0,
                                  &SAIndex1,
                                  &fSAInUse1);

        if (rc) return rc;

        SAIndex0 = SAIndex;
        fSAInUse0 = fSAInUse;

        rc = EIP164_SC_SA_MAP2_WR(Device,
                                  SCIndex,
                                  SAIndex0,
                                  fSAInUse0,
                                  false,
                                  SAIndex1,
                                  fSAInUse1,
                                  false);
        if (rc) return rc;
        break;

    case 3:
        rc = EIP164_SC_SA_MAP2_RD(Device,
                                  SCIndex,
                                  &SAIndex0,
                                  &fSAInUse0,
                                  &SAIndex1,
                                  &fSAInUse1);
        if (rc) return rc;

        SAIndex1 = SAIndex;
        fSAInUse1 = fSAInUse;

        rc = EIP164_SC_SA_MAP2_WR(Device,
                                  SCIndex,
                                  SAIndex0,
                                  fSAInUse0,
                                  false,
                                  SAIndex1,
                                  fSAInUse1,
                                  false);
        if (rc) return rc;
        break;

    default:
        return EIP164_ARGUMENT_ERROR;
    }

    return EIP164_NO_ERROR;
}


#ifdef EIP164_CRYPT_AUTH_ENABLE
/*----------------------------------------------------------------------------
 * EIP164_SecY_SA_CryptAuth_BypassLen_Update
 */
EIP164_Error_t
EIP164_SecY_CryptAuth_BypassLen_Update(
        const EIP164_IOArea_t * const IOArea_p,
        const unsigned int BypassLength)
{
    Device_Handle_t Device;
    const volatile EIP164_True_IOArea_t * const TrueIOArea_p = IOAREA_IN(IOArea_p);
    int rc = 0;

    EIP164_CHECK_POINTER(IOArea_p);
    EIP164_CHECK_INT_ATMOST(BypassLength,
                            EIP164_SECY_CRYPT_AUTH_MAX_BYPASS_LEN);

    Device = TrueIOArea_p->Device;

    rc = EIP164_CRYPT_AUTH_CTRL_WR(Device, BypassLength);
    if (rc) return rc;

    return EIP164_NO_ERROR;
}


/*----------------------------------------------------------------------------
 * EIP164_SecY_SA_CryptAuth_BypassLen_Get
 */
EIP164_Error_t
EIP164_SecY_CryptAuth_BypassLen_Get(
        const EIP164_IOArea_t * const IOArea_p,
        unsigned int * const BypassLength_p)
{
    Device_Handle_t Device;
    const volatile EIP164_True_IOArea_t * const TrueIOArea_p = IOAREA_IN(IOArea_p);
    uint8_t BypassLength;
    int rc = 0;

    EIP164_CHECK_POINTER(IOArea_p);
    EIP164_CHECK_POINTER(BypassLength_p);
    Device = TrueIOArea_p->Device;

    rc = EIP164_CRYPT_AUTH_CTRL_RD(Device, &BypassLength);
    if (rc) return rc;

    *BypassLength_p = BypassLength;

    return EIP164_NO_ERROR;
}
#endif /* EIP164_CRYPT_AUTH_ENABLE */

/*----------------------------------------------------------------------------
 * EIP164_SecY_RxCAM_Add
 */
EIP164_Error_t
EIP164_SecY_RxCAM_Add(
        const EIP164_IOArea_t * const IOArea_p,
        const unsigned int SCIndex,
        const uint32_t SCI_Lo,
        const uint32_t SCI_Hi,
        const unsigned int vPort)
{
    Device_Handle_t Device;
    const volatile EIP164_True_IOArea_t * const TrueIOArea_p = IOAREA_IN(IOArea_p);
    int rc = 0;

    EIP164_CHECK_POINTER(IOArea_p);
    EIP164_CHECK_INT_ATMOST(SCIndex + 1, TrueIOArea_p->MaxSCCount);
    EIP164_CHECK_INT_ATMOST(vPort + 1, TrueIOArea_p->MaxvPortCount);

    Device = TrueIOArea_p->Device;
    if (TrueIOArea_p->fExternalRxCAM)
    {
        bool fWriteBusy,fReadBusy,fDisableBusy,fEnabled;

        rc = EIP164_RXSC_CAM_STATUS_RD(Device,
                                       &fWriteBusy,
                                       &fReadBusy,
                                       &fDisableBusy,
                                       &fEnabled);
        if (rc) return rc;

        if (fReadBusy || fWriteBusy)
            return EIP164_BUSY_RETRY_LATER;
    }

    /* Add an entry to the RX CAM (translate Vport+SCI->SCIndex) */
    rc = EIP164_RXSC_CAM_SCI_LO_WR(Device, SCIndex, SCI_Lo);
    if (rc) return rc;

    rc = EIP164_RXSC_CAM_SCI_HI_WR(Device, SCIndex, SCI_Hi);
    if (rc) return rc;

    rc = EIP164_RXSC_CAM_CTRL_WR(Device, SCIndex, vPort);
    if (rc) return rc;

    if (!TrueIOArea_p->fExternalRxCAM)
    {
        rc = EIP164_RXSC_ENTRY_ENABLE_CTRL_WR(Device,
                                              SCIndex, true, false,
                                              0, false, false);
        if (rc) return rc;
    }

    return EIP164_NO_ERROR;
}


/*----------------------------------------------------------------------------
 * EIP164_SecY_RxCAM_Read
 */
EIP164_Error_t
EIP164_SecY_RxCAM_Read(
        EIP164_IOArea_t * const IOArea_p,
        const unsigned int SCIndex,
        uint32_t * const SCI_Lo_p,
        uint32_t * const SCI_Hi_p,
        unsigned int * vPort_p,
        bool *fEnabled_p)
{
    Device_Handle_t Device;
    volatile EIP164_True_IOArea_t * const TrueIOArea_p = IOAREA_INOUT(IOArea_p);
    int rc = 0;

    EIP164_CHECK_POINTER(IOArea_p);
    EIP164_CHECK_INT_ATMOST(SCIndex + 1, TrueIOArea_p->MaxSCCount);
    EIP164_CHECK_POINTER(SCI_Lo_p);
    EIP164_CHECK_POINTER(SCI_Hi_p);
    EIP164_CHECK_POINTER(vPort_p);
    EIP164_CHECK_POINTER(fEnabled_p);

    Device = TrueIOArea_p->Device;

    if (TrueIOArea_p->fExternalRxCAM)
    {
        bool fWriteBusy,fReadBusy,fDisableBusy,fEnabled;
        uint32_t Dummy;

        rc = EIP164_RXSC_CAM_STATUS_RD(Device,
                                  &fWriteBusy,
                                  &fReadBusy, &
                                  fDisableBusy,
                                  &fEnabled);
        if (rc) return rc;

        if (fReadBusy || fWriteBusy)
        {
            return EIP164_BUSY_RETRY_LATER;
        }
        else if (!TrueIOArea_p->fReadPending)
        {
            /* Trigger the read operation. */
            rc = EIP164_RXSC_CAM_SCI_LO_RD(Device, SCIndex, &Dummy);
            if (rc) return rc;

            TrueIOArea_p->fReadPending = true;
            return EIP164_BUSY_RETRY_LATER;
        }
        *fEnabled_p = fEnabled;
        TrueIOArea_p->fReadPending = false;
    }
    else
    {
        rc = EIP164_RXSC_ENTRY_ENABLE_RD(Device, SCIndex, fEnabled_p);
        if (rc) return rc;
    }

    rc = EIP164_RXSC_CAM_SCI_LO_RD(Device, SCIndex, SCI_Lo_p);
    if (rc) return rc;

    rc = EIP164_RXSC_CAM_SCI_HI_RD(Device, SCIndex, SCI_Hi_p);
    if (rc) return rc;

    rc = EIP164_RXSC_CAM_CTRL_RD(Device, SCIndex, vPort_p);
    if (rc) return rc;

    return EIP164_NO_ERROR;
}

/*----------------------------------------------------------------------------
 * EIP164_SecY_RxCAM_Remove
 */
EIP164_Error_t
EIP164_SecY_RxCAM_Remove(
            const EIP164_IOArea_t * const IOArea_p,
            const unsigned int SCIndex)
{
    Device_Handle_t Device;
    const volatile EIP164_True_IOArea_t * const TrueIOArea_p = IOAREA_IN(IOArea_p);
    int rc = 0;

    EIP164_CHECK_POINTER(IOArea_p);
    EIP164_CHECK_INT_ATMOST(SCIndex + 1, TrueIOArea_p->MaxSCCount);

    Device = TrueIOArea_p->Device;

    if (TrueIOArea_p->fExternalRxCAM)
    {
        bool fWriteBusy,fReadBusy,fDisableBusy,fEnabled;

        rc = EIP164_RXSC_CAM_STATUS_RD(Device,
                                       &fWriteBusy,
                                       &fReadBusy, &
                                       fDisableBusy,
                                       &fEnabled);
        if (rc) return rc;

        if (fDisableBusy)
            return EIP164_BUSY_RETRY_LATER;
    }

    rc = EIP164_SC_SA_MAP1_WR(Device, SCIndex, 0, false, false, 0, false, false);
    if (rc) return rc;

    rc = EIP164_SC_SA_MAP2_WR(Device, SCIndex, 0, false, false, 0, false, false);
    if (rc) return rc;

    rc = EIP164_RXSC_ENTRY_ENABLE_CTRL_WR(Device,
                                          0, false, false,
                                          SCIndex, true,
                                          TrueIOArea_p->fExternalRxCAM);
    if (rc) return rc;

    return EIP164_NO_ERROR;
}


#ifdef EIP164_DEVICE_MODE_INGRESS
/*----------------------------------------------------------------------------
 * EIP164_SecY_RxCAM_Stat_Get
 */
EIP164_Error_t
EIP164_SecY_RxCAM_Stat_Get(
        const EIP164_IOArea_t * const IOArea_p,
        const unsigned int SCIndex,
        EIP164_SecY_RxCAM_Stat_t * const Stats_p,
        const bool fClear)
{
    Device_Handle_t Device;
    const volatile EIP164_True_IOArea_t * const TrueIOArea_p = IOAREA_IN(IOArea_p);
    int rc = 0;

    EIP164_CHECK_POINTER(IOArea_p);
    EIP164_CHECK_POINTER(Stats_p);
    EIP164_CHECK_INT_ATMOST(SCIndex + 1, TrueIOArea_p->MaxSCCount);

    if ((TrueIOArea_p->Mode != EIP164_MODE_INGRESS) &&
        (TrueIOArea_p->Mode != EIP164_MODE_EGRESS_INGRESS))
    {
        return EIP164_HW_CONFIGURATION_MISMATCH;
    }

    Device = TrueIOArea_p->Device;

    /* RxCAM statistics */
    rc = EIP164Lib_Counter_Read(Device,
                              EIP164_REG_RXCAM_STATISTICS(SCIndex),
                              EIP164_RXCAM_STAT_HIT,
                              &Stats_p->CAMHit.low,
                              &Stats_p->CAMHit.hi,
                              fClear);
    if (rc) return rc;

    return EIP164_NO_ERROR;
}


/*----------------------------------------------------------------------------
 * EIP164_SecY_RxCAM_Stat_Clear
 */
EIP164_Error_t
EIP164_SecY_RxCAM_Stat_Clear(
        const EIP164_IOArea_t * const IOArea_p,
        const unsigned int SCIndex)
{
    Device_Handle_t Device;
    const volatile EIP164_True_IOArea_t * const TrueIOArea_p = IOAREA_IN(IOArea_p);
    int rc = 0;

    EIP164_CHECK_POINTER(IOArea_p);
    EIP164_CHECK_INT_ATMOST(SCIndex + 1, TrueIOArea_p->MaxSCCount);

    if ((TrueIOArea_p->Mode != EIP164_MODE_INGRESS) &&
        (TrueIOArea_p->Mode != EIP164_MODE_EGRESS_INGRESS))
    {
        return EIP164_HW_CONFIGURATION_MISMATCH;
    }

    Device = TrueIOArea_p->Device;

    /* RxCAM statistics */
    rc = EIP164_COUNTER_64_DEFAULT_WR(Device,
                                      EIP164_REG_RXCAM_STATISTICS(SCIndex),
                                      EIP164_RXCAM_STAT_HIT);
    if (rc) return rc;

    return EIP164_NO_ERROR;
}
#endif /* EIP164_DEVICE_MODE_INGRESS */


/*----------------------------------------------------------------------------
 * EIP164_SecY_SAMFlow_Clear
 */
EIP164_Error_t
EIP164_SecY_SAMFlow_Clear(
            const EIP164_IOArea_t * const IOArea_p,
            const unsigned int vPort)
{
    Device_Handle_t Device;
    const volatile EIP164_True_IOArea_t * const TrueIOArea_p = IOAREA_IN(IOArea_p);
    int rc = 0;

    EIP164_CHECK_POINTER(IOArea_p);
    EIP164_CHECK_INT_ATMOST(vPort + 1, TrueIOArea_p->MaxvPortCount);

    Device = TrueIOArea_p->Device;

    rc = EIP164_SAM_FLOW_CTRL1_BYPASS_WR(Device, vPort, 0, false, false, 0);
    if (rc) return rc;

    return EIP164_NO_ERROR;
}


/*----------------------------------------------------------------------------
 * EIP164_SecY_SAMFlow_Write
 */
EIP164_Error_t
EIP164_SecY_SAMFlow_Write(
        const EIP164_IOArea_t * const IOArea_p,
        const unsigned int vPort,
        const EIP164_SecY_SA_t * const SA_p
        )
{
    Device_Handle_t Device;
    const volatile EIP164_True_IOArea_t * const TrueIOArea_p = IOAREA_IN(IOArea_p);
    int rc = 0;

    EIP164_CHECK_POINTER(IOArea_p);
    EIP164_CHECK_POINTER(SA_p);
    EIP164_CHECK_INT_ATMOST(vPort + 1, TrueIOArea_p->MaxvPortCount);

    Device = TrueIOArea_p->Device;

    {
        switch (SA_p->ActionType)
        {
        case EIP164_SECY_SA_ACTION_EGRESS:
            /* Check if the SA type is supported in the device mode */
            if (TrueIOArea_p->Mode == EIP164_MODE_INGRESS)
            {
                return EIP164_HW_CONFIGURATION_MISMATCH;
            }

            /* Enable egress SA */
            rc = EIP164_SAM_FLOW_CTRL1_EGRESS_WR(Device,
                                                 vPort,
                                                 SA_p->DestPort,
                                                 false, /* fFlowCyptAuth */
                                                 SA_p->DropType,
                                                 SA_p->Params.Egress.fProtectFrames,
                                                 SA_p->Params.Egress.fConfProtect,
                                                 SA_p->Params.Egress.fIncludeSCI,
                                                 SA_p->Params.Egress.fUseES,
                                                 SA_p->Params.Egress.fUseSCB,
                                                 SA_p->Params.Egress.fAllowDataPkts);
            if (rc) return rc;

            rc = EIP164_SAM_FLOW_CTRL2_WR(Device,
                                     vPort,
                                     SA_p->Params.Egress.ConfidentialityOffset,
                                     SA_p->Params.Egress.PreSecTagAuthStart,
                                     SA_p->Params.Egress.PreSecTagAuthLength,
                                     false); /* Don't retains ICV for egress */
            if (rc) return rc;

            break;

        case EIP164_SECY_SA_ACTION_INGRESS:
            /* Check if the SA type is supported in the device mode */
            if (TrueIOArea_p->Mode == EIP164_MODE_EGRESS)
            {
                return EIP164_HW_CONFIGURATION_MISMATCH;
            }

            if (SA_p->Params.Ingress.fRetainICV &&
                    !SA_p->Params.Ingress.fRetainSecTAG)
            {
                return EIP164_UNSUPPORTED_FEATURE_ERROR;
            }

            /* Enable ingress SA */
            rc = EIP164_SAM_FLOW_CTRL1_INGRESS_WR(Device,
                                             vPort,
                                             SA_p->DestPort,
                                             SA_p->Params.Ingress.fRetainSecTAG,
                                             false, /* fFlowCyptAuth */
                                             SA_p->DropType,
                                             SA_p->Params.Ingress.fReplayProtect,
                                             SA_p->Params.Ingress.fAllowTagged,
                                             SA_p->Params.Ingress.fAllowUntagged,
                                             SA_p->Params.Ingress.ValidateFramesTagged,
                                             SA_p->Params.Ingress.fValidateUntagged);
            if (rc) return rc;
            rc = EIP164_SAM_FLOW_CTRL2_WR(Device,
                                     vPort,
                                     SA_p->Params.Ingress.ConfidentialityOffset,
                                     SA_p->Params.Ingress.PreSecTagAuthStart,
                                     SA_p->Params.Ingress.PreSecTagAuthLength,
                                     SA_p->Params.Ingress.fRetainICV);
            if (rc) return rc;
            break;

        case EIP164_SECY_SA_ACTION_CRYPT_AUTH:
            /* Enable Crypt-authenticate SA */
            rc = EIP164_SAM_FLOW_CTRL1_CRYPTAUTH_WR(Device,
                                               vPort,
                                               SA_p->DestPort,
                                               SA_p->fDropNonReserved,
                                               true, /* fFlowCryptAuth */
                                               SA_p->DropType,
                                               SA_p->Params.CryptAuth.fConfProtect,
                                               SA_p->Params.CryptAuth.fICVAppend,
                                               SA_p->Params.CryptAuth.IVMode,
                                               SA_p->Params.CryptAuth.fICVVerify,
                                               /* Enabled SC lookup for ingress configuration */
                                               TrueIOArea_p->Mode == EIP164_MODE_INGRESS);
            if (rc) return rc;

            {
                uint8_t ConfOffset = SA_p->Params.CryptAuth.ConfidentialityOffset;

                if (SA_p->Params.CryptAuth.fZeroLengthMessage)
                {
                    if (SA_p->Params.CryptAuth.fICVVerify)
                    {
                        ConfOffset = BIT_6;
                    }
                }

                rc = EIP164_SAM_FLOW_CTRL2_WR(Device,
                                              vPort,
                                              ConfOffset,
                                              0,
                                              0,
                                              false);
                if (rc) return rc;
            }
            break;

        case EIP164_SECY_SA_ACTION_BYPASS:
            /* Enable non-MACsec SA */
            rc = EIP164_SAM_FLOW_CTRL1_BYPASS_WR(Device,
                                            vPort,
                                            SA_p->DestPort,
                                            SA_p->fDropNonReserved,
                                            false, /* fFlowCyptAuth */
                                            SA_p->DropType);
            if (rc) return rc;
            break;

        default:
        case EIP164_SECY_SA_ACTION_DROP:
            /* Enable non-MACsec SA */
            rc = EIP164_SAM_FLOW_CTRL1_DROP_WR(Device,
                                          vPort,
                                          SA_p->DestPort,
                                          SA_p->fDropNonReserved,
                                          false, /* fFlowCyptAuth */
                                          SA_p->DropType);
            if (rc) return rc;
            break;
        }
    }

    return EIP164_NO_ERROR;
}


/*----------------------------------------------------------------------------
 * EIP164_SecY_SAMFlow_Read
 */
EIP164_Error_t
EIP164_SecY_SAMFlow_Read(
        const EIP164_IOArea_t * const IOArea_p,
        const unsigned int vPort,
        EIP164_SecY_SA_t * const SA_p
        )
{
    Device_Handle_t Device;
    const volatile EIP164_True_IOArea_t * const TrueIOArea_p = IOAREA_IN(IOArea_p);
    bool fDummy;
    int rc = 0;
    uint8_t Dummy1 = 0, DropType = 0, DestPort = 0, FlowType = 0, ValidateFramesTagged = 0;

    EIP164_CHECK_POINTER(IOArea_p);
    EIP164_CHECK_POINTER(SA_p);
    EIP164_CHECK_INT_ATMOST(vPort + 1, TrueIOArea_p->MaxvPortCount);

    Device = TrueIOArea_p->Device;

    rc = EIP164_SAM_FLOW_CTRL1_TYPE_RD(Device, vPort, &FlowType);
    if (rc) return rc;

    SA_p->ActionType = FlowType;

    {
        switch (SA_p->ActionType)
        {
        case EIP164_SECY_SA_ACTION_EGRESS:
            /* Check if the SA type is supported in the device mode */
            if (TrueIOArea_p->Mode == EIP164_MODE_INGRESS)
            {
                return EIP164_HW_CONFIGURATION_MISMATCH;
            }

            /* Enable egress SA */
            rc = EIP164_SAM_FLOW_CTRL1_EGRESS_RD(Device,
                                            vPort,
                                            &DestPort,
                                            &fDummy, /* fFlowCyptAuth */
                                            &DropType,
                                            &SA_p->Params.Egress.fProtectFrames,
                                            &SA_p->Params.Egress.fConfProtect,
                                            &SA_p->Params.Egress.fIncludeSCI,
                                            &SA_p->Params.Egress.fUseES,
                                            &SA_p->Params.Egress.fUseSCB,
                                            &SA_p->Params.Egress.fAllowDataPkts);
            if (rc) return rc;

            rc = EIP164_SAM_FLOW_CTRL2_RD(Device,
                                     vPort,
                                     &SA_p->Params.Egress.ConfidentialityOffset,
                                     &SA_p->Params.Egress.PreSecTagAuthStart,
                                     &SA_p->Params.Egress.PreSecTagAuthLength,
                                     &fDummy); /* Don't retains ICV for egress */
            if (rc) return rc;
            break;

        case EIP164_SECY_SA_ACTION_INGRESS:
            /* Check if the SA type is supported in the device mode */
            if (TrueIOArea_p->Mode == EIP164_MODE_EGRESS)
            {
                return EIP164_HW_CONFIGURATION_MISMATCH;
            }

            /* Enable ingress SA */
            rc = EIP164_SAM_FLOW_CTRL1_INGRESS_RD(Device,
                                             vPort,
                                             &DestPort,
                                             &SA_p->Params.Ingress.fRetainSecTAG,
                                             &fDummy, /* fFlowCyptAuth */
                                             &DropType,
                                             &SA_p->Params.Ingress.fReplayProtect,
                                             &SA_p->Params.Ingress.fAllowTagged,
                                             &SA_p->Params.Ingress.fAllowUntagged,
                                             &ValidateFramesTagged,
                                             &SA_p->Params.Ingress.fValidateUntagged);
            if (rc) return rc;

            SA_p->Params.Ingress.ValidateFramesTagged = ValidateFramesTagged;
            rc = EIP164_SAM_FLOW_CTRL2_RD(Device,
                                     vPort,
                                     &SA_p->Params.Ingress.ConfidentialityOffset,
                                     &SA_p->Params.Ingress.PreSecTagAuthStart,
                                     &SA_p->Params.Ingress.PreSecTagAuthLength,
                                     &SA_p->Params.Ingress.fRetainICV);
            if (rc) return rc;
            break;

        case EIP164_SECY_SA_ACTION_CRYPT_AUTH:
            /* Enable Crypt-authenticate SA */
            rc = EIP164_SAM_FLOW_CTRL1_CRYPTAUTH_RD(Device,
                                               vPort,
                                               &DestPort,
                                               &SA_p->fDropNonReserved,
                                               &fDummy, /* fFlowCryptAuth */
                                               &DropType,
                                               &SA_p->Params.CryptAuth.fConfProtect,
                                               &SA_p->Params.CryptAuth.fICVAppend,
                                               &SA_p->Params.CryptAuth.IVMode,
                                               &SA_p->Params.CryptAuth.fICVVerify,
                                               &fDummy);
            if (rc) return rc;
            {
                uint8_t ConfOffset;

                rc = EIP164_SAM_FLOW_CTRL2_RD(Device,
                                         vPort,
                                         &ConfOffset,
                                         &Dummy1,
                                         &Dummy1,
                                         &fDummy);
                if (rc) return rc;
                if ( (ConfOffset & BIT_6) != 0)
                {
                    SA_p->Params.CryptAuth.ConfidentialityOffset = 0;
                    SA_p->Params.CryptAuth.fZeroLengthMessage = true;
                }
                else
                {
                    SA_p->Params.CryptAuth.ConfidentialityOffset = ConfOffset;
                    SA_p->Params.CryptAuth.fZeroLengthMessage = false;
                }
            }
            break;

        case EIP164_SECY_SA_ACTION_BYPASS:
            /* Enable non-MACsec SA */
            rc = EIP164_SAM_FLOW_CTRL1_BYPASS_RD(Device,
                                                 vPort,
                                                 &DestPort,
                                                 &SA_p->fDropNonReserved,
                                                 &fDummy, /* fFlowCyptAuth */
                                                 &DropType);
            if (rc) return rc;
            break;

        default:
        case EIP164_SECY_SA_ACTION_DROP:
            /* Enable non-MACsec SA */
            rc = EIP164_SAM_FLOW_CTRL1_DROP_RD(Device,
                                               vPort,
                                               &DestPort,
                                               &SA_p->fDropNonReserved,
                                               &fDummy, /* fFlowCyptAuth */
                                               &DropType);
            if (rc) return rc;
            break;
       }
    }
    SA_p->DropType = DropType;
    SA_p->DestPort = DestPort;

    return EIP164_NO_ERROR;
}


/*----------------------------------------------------------------------------
 * EIP164_SecY_SA_NextPN_Update
 */
EIP164_Error_t
EIP164_SecY_SA_NextPN_Update(
       const EIP164_IOArea_t * const IOArea_p,
       const unsigned int SAIndex,
       const uint32_t NextPN_Lo,
       const uint32_t NextPN_Hi,
       const bool fSeqNum64)
{
    Device_Handle_t Device;
    uint32_t ContextAddress;
    const volatile EIP164_True_IOArea_t * const TrueIOArea_p = IOAREA_IN(IOArea_p);
    int rc = 0;

    EIP164_CHECK_POINTER(IOArea_p);
    EIP164_CHECK_INT_ATMOST(SAIndex + 1, TrueIOArea_p->MaxSACount);

    Device = TrueIOArea_p->Device;

    ContextAddress = SAIndex;

    rc = EIP66_NEXTPN_LO_WR(Device, NextPN_Lo);
    if (rc) return rc;
    if (fSeqNum64)
    {
        rc = EIP66_NEXTPN_HI_WR(Device, NextPN_Hi);
        if (rc) return rc;
    }
    rc = EIP66_NEXTPN_CTX_ADDR_WR(Device, ContextAddress);
    if (rc) return rc;
    rc = EIP66_NEXTPN_CTRL_WR(Device);
    if (rc) return rc;

    return EIP164_NO_ERROR;
}


/*----------------------------------------------------------------------------
 * EIP164_SecY_SA_NextPN_Status_Get
 */
EIP164_Error_t
EIP164_SecY_SA_NextPN_Status_Get(
       const EIP164_IOArea_t * const IOArea_p,
       bool * const fNextPNWritten_p,
       bool * const fBusy_p)
{
    Device_Handle_t Device;
    const volatile EIP164_True_IOArea_t * const TrueIOArea_p = IOAREA_IN(IOArea_p);
    int rc = 0;

    bool fECCError;

    EIP164_CHECK_POINTER(IOArea_p);
    EIP164_CHECK_POINTER(fNextPNWritten_p);
    EIP164_CHECK_POINTER(fBusy_p);

    Device = TrueIOArea_p->Device;
    rc = EIP66_NEXTPN_CTRL_RD(Device, fNextPNWritten_p, fBusy_p, &fECCError);
    if (rc) return rc;
    return EIP164_NO_ERROR;
}


/*----------------------------------------------------------------------------
 * EIP164_SecY_SA_Read
 */
EIP164_Error_t
EIP164_SecY_SA_Read(
        const EIP164_IOArea_t * const IOArea_p,
        const unsigned int SAIndex,
        const unsigned int WordOffset,
        const unsigned int WordCount,
        uint32_t * Transform_p)
{
    Device_Handle_t Device;
    const volatile EIP164_True_IOArea_t * const TrueIOArea_p = IOAREA_IN(IOArea_p);
    int rc = 0;

    EIP164_CHECK_POINTER(IOArea_p);
    EIP164_CHECK_INT_ATMOST(SAIndex + 1, TrueIOArea_p->MaxSACount);
    EIP164_CHECK_INT_ATMOST(WordOffset + 1, EIP164_XFORM_REC_WORD_COUNT);
    EIP164_CHECK_INT_ATMOST(WordOffset + WordCount, EIP164_XFORM_REC_WORD_COUNT);
    EIP164_CHECK_POINTER(Transform_p);

    Device = TrueIOArea_p->Device;

    /* Read (a part of) the SA transform record */
    rc = EIP164_XFORM_REC_RD(Device, SAIndex, WordOffset, WordCount, Transform_p);
    if (rc) return rc;

    return EIP164_NO_ERROR;
}


/*----------------------------------------------------------------------------
 * EIP164_SecY_SA_Update_Control_Word_Update
 */
EIP164_Error_t
EIP164_SecY_SA_Update_Control_Word_Update(
        const EIP164_IOArea_t * const IOArea_p,
        const unsigned int SAIndex,
        const unsigned int Offset,
        const uint32_t UpdateCtrlWord)
{
    Device_Handle_t Device;
    const volatile EIP164_True_IOArea_t * const TrueIOArea_p = IOAREA_IN(IOArea_p);
    int rc = 0;

    EIP164_CHECK_POINTER(IOArea_p);
    EIP164_CHECK_INT_ATMOST(SAIndex + 1, TrueIOArea_p->MaxSACount);

    Device = TrueIOArea_p->Device;

    /* Write control word to the SA transform record */
    rc = EIP164_XFORM_REC_WORD_WR(Device,
                                  SAIndex,
                                  Offset,
                                  UpdateCtrlWord);
    if (rc) return rc;

    return EIP164_NO_ERROR;
}
#ifdef EIP164_DEVICE_MODE_EGRESS
/*----------------------------------------------------------------------------
 * EIP164_SecY_SA_Installation_Verify
 */
EIP164_Error_t
EIP164_SecY_SA_Installation_Verify(
        const EIP164_IOArea_t * const IOArea_p,
        const unsigned int SAMFlowCtrlIndex,
        unsigned int * const SAIndex_p,
        bool * const fSAInUse_p)
{
    Device_Handle_t Device;
    unsigned int SAIndex0,SAIndex1;
    bool fSAInUse0, fSAInUse1;
    const volatile EIP164_True_IOArea_t * const TrueIOArea_p = IOAREA_IN(IOArea_p);
    int rc = 0;

    EIP164_CHECK_POINTER(IOArea_p);
    EIP164_CHECK_INT_ATMOST(SAMFlowCtrlIndex + 1, TrueIOArea_p->MaxvPortCount);
    EIP164_CHECK_POINTER(fSAInUse_p);

    Device = TrueIOArea_p->Device;

    rc = EIP164_SC_SA_MAP1_RD(Device, SAMFlowCtrlIndex, &SAIndex0, &fSAInUse0,
                         &SAIndex1, &fSAInUse1);
    if (rc) return rc;

    *fSAInUse_p = fSAInUse0;
    *SAIndex_p = SAIndex0;

    return EIP164_NO_ERROR;
}
#endif

#ifdef EIP164_DEVICE_MODE_INGRESS
/*----------------------------------------------------------------------------
 * EIP164_SecY_SA_Active_I_Get
 */
EIP164_Error_t
EIP164_SecY_SA_Active_I_Get(
        const EIP164_IOArea_t * const IOArea_p,
        const unsigned int SCIndex,
        unsigned int * const SAIndex_p,
        bool * const fSAInUse_p)
{
    Device_Handle_t Device;
    const volatile EIP164_True_IOArea_t * const TrueIOArea_p = IOAREA_IN(IOArea_p);
    int rc = 0;

    EIP164_CHECK_POINTER(IOArea_p);
    EIP164_CHECK_INT_ATMOST(SCIndex + 1, TrueIOArea_p->MaxSCCount);
    EIP164_CHECK_POINTER(fSAInUse_p);

    Device = TrueIOArea_p->Device;

    rc = EIP164_SC_SA_MAP1_RD(Device,
                              SCIndex,
                              &SAIndex_p[0],
                              &fSAInUse_p[0],
                              &SAIndex_p[1],
                              &fSAInUse_p[1]);
    if (rc) return rc;

    rc = EIP164_SC_SA_MAP2_RD(Device,
                              SCIndex,
                              &SAIndex_p[2],
                              &fSAInUse_p[2],
                              &SAIndex_p[3],
                              &fSAInUse_p[3]);
    if (rc) return rc;

    return EIP164_NO_ERROR;
}
#endif


#ifdef EIP164_DEVICE_MODE_EGRESS
/*----------------------------------------------------------------------------
 * EIP164_SecY_Stat_E_Clear
 */
EIP164_Error_t
EIP164_SecY_Stat_E_Clear(
        const EIP164_IOArea_t * const IOArea_p,
        const unsigned int SecY_Index)
{
    Device_Handle_t Device;
    const volatile EIP164_True_IOArea_t * const TrueIOArea_p = IOAREA_IN(IOArea_p);
    int rc = 0;

    EIP164_CHECK_POINTER(IOArea_p);
    EIP164_CHECK_INT_ATMOST(SecY_Index + 1, TrueIOArea_p->MaxvPortCount);

    if ((TrueIOArea_p->Mode != EIP164_MODE_EGRESS) &&
        (TrueIOArea_p->Mode != EIP164_MODE_EGRESS_INGRESS))
    {
        return EIP164_HW_CONFIGURATION_MISMATCH;
    }

    Device = TrueIOArea_p->Device;

    /* Egress SecY statistics */
    rc = EIP164_COUNTER_64_DEFAULT_WR(Device,
                                 EIP164_REG_SECY_STATISTICS(SecY_Index),
                                 EIP164_SECY_STAT_TRANSFORM_ERROR_PKTS);
    if (rc) return rc;
    rc = EIP164_COUNTER_64_DEFAULT_WR(Device,
                                 EIP164_REG_SECY_STATISTICS(SecY_Index),
                                 EIP164_SECY_STAT_E_OUT_PKTS_CTRL);
    if (rc) return rc;
    rc = EIP164_COUNTER_64_DEFAULT_WR(Device,
                                 EIP164_REG_SECY_STATISTICS(SecY_Index),
                                 EIP164_SECY_STAT_E_OUT_PKTS_UNTAGGED);
    if (rc) return rc;

    return EIP164_NO_ERROR;
}
#endif


#ifdef EIP164_DEVICE_MODE_INGRESS
/*----------------------------------------------------------------------------
 * EIP164_SecY_Stat_I_Clear
 */
EIP164_Error_t
EIP164_SecY_Stat_I_Clear(
        const EIP164_IOArea_t * const IOArea_p,
        const unsigned int SecY_Index)
{
    Device_Handle_t Device;
    const volatile EIP164_True_IOArea_t * const TrueIOArea_p = IOAREA_IN(IOArea_p);
    int rc = 0;

    EIP164_CHECK_POINTER(IOArea_p);
    EIP164_CHECK_INT_ATMOST(SecY_Index + 1, TrueIOArea_p->MaxvPortCount);

    if ((TrueIOArea_p->Mode != EIP164_MODE_INGRESS) &&
        (TrueIOArea_p->Mode != EIP164_MODE_EGRESS_INGRESS))
    {
        return EIP164_HW_CONFIGURATION_MISMATCH;
    }

    Device = TrueIOArea_p->Device;

    /* Ingress SecY statistics */
    rc = EIP164_COUNTER_64_DEFAULT_WR(Device,
                                 EIP164_REG_SECY_STATISTICS(SecY_Index),
                                 EIP164_SECY_STAT_TRANSFORM_ERROR_PKTS);
    if (rc) return rc;
    rc = EIP164_COUNTER_64_DEFAULT_WR(Device,
                                 EIP164_REG_SECY_STATISTICS(SecY_Index),
                                 EIP164_SECY_STAT_I_IN_PKTS_CTRL);
    if (rc) return rc;
    rc = EIP164_COUNTER_64_DEFAULT_WR(Device,
                                 EIP164_REG_SECY_STATISTICS(SecY_Index),
                                 EIP164_SECY_STAT_I_IN_PKTS_UNTAGGED);
    if (rc) return rc;
    rc = EIP164_COUNTER_64_DEFAULT_WR(Device,
                                 EIP164_REG_SECY_STATISTICS(SecY_Index),
                                 EIP164_SECY_STAT_I_IN_PKTS_NOTAG);
    if (rc) return rc;
    rc = EIP164_COUNTER_64_DEFAULT_WR(Device,
                                 EIP164_REG_SECY_STATISTICS(SecY_Index),
                                 EIP164_SECY_STAT_I_IN_PKTS_BADTAG);
    if (rc) return rc;
    rc = EIP164_COUNTER_64_DEFAULT_WR(Device,
                                 EIP164_REG_SECY_STATISTICS(SecY_Index),
                                 EIP164_SECY_STAT_I_IN_PKTS_NOSCI);
    if (rc) return rc;
    rc = EIP164_COUNTER_64_DEFAULT_WR(Device,
                                 EIP164_REG_SECY_STATISTICS(SecY_Index),
                                 EIP164_SECY_STAT_I_IN_PKTS_UNKNOWNSCI);
    if (rc) return rc;
    rc = EIP164_COUNTER_64_DEFAULT_WR(Device,
                                 EIP164_REG_SECY_STATISTICS(SecY_Index),
                                 EIP164_SECY_STAT_I_IN_PKTS_TAGGEDCTRL);
    if (rc) return rc;

    return EIP164_NO_ERROR;
}
#endif


#ifdef EIP164_DEVICE_MODE_EGRESS
/*----------------------------------------------------------------------------
 * EIP164_SecY_SA_Stat_E_Clear
 */
EIP164_Error_t
EIP164_SecY_SA_Stat_E_Clear(
        const EIP164_IOArea_t * const IOArea_p,
        const unsigned int SAIndex)
{
    Device_Handle_t Device;
    const volatile EIP164_True_IOArea_t * const TrueIOArea_p = IOAREA_IN(IOArea_p);
    int rc = 0;

    EIP164_CHECK_POINTER(IOArea_p);
    EIP164_CHECK_INT_ATMOST(SAIndex + 1, TrueIOArea_p->MaxSACount);

    if ((TrueIOArea_p->Mode != EIP164_MODE_EGRESS) &&
        (TrueIOArea_p->Mode != EIP164_MODE_EGRESS_INGRESS))
    {
        return EIP164_HW_CONFIGURATION_MISMATCH;
    }

    Device = TrueIOArea_p->Device;

    /* Egress SA statistics */
    rc = EIP164_COUNTER_64_DEFAULT_WR(Device,
                                 EIP164_REG_SA_STATISTICS(SAIndex),
                                 EIP164_SA_STAT_E_OUT_OCTETS_ENC_PROT);
    if (rc) return rc;
    rc = EIP164_COUNTER_64_DEFAULT_WR(Device,
                                 EIP164_REG_SA_STATISTICS(SAIndex),
                                 EIP164_SA_STAT_E_OUT_PKTS_ENC_PROT);
    if (rc) return rc;
    rc = EIP164_COUNTER_64_DEFAULT_WR(Device,
                                 EIP164_REG_SA_STATISTICS(SAIndex),
                                 EIP164_SA_STAT_E_OUT_PKTS_TOO_LONG);
    if (rc) return rc;
    rc = EIP164_COUNTER_64_DEFAULT_WR(Device,
                                 EIP164_REG_SA_STATISTICS(SAIndex),
                                 EIP164_SA_STAT_E_OUT_PKTS_SA_NOT_IN_USE);
    if (rc) return rc;

    return EIP164_NO_ERROR;
}
#endif


#ifdef EIP164_DEVICE_MODE_INGRESS
/*----------------------------------------------------------------------------
 * EIP164_SecY_SA_Stat_I_Clear
 */
EIP164_Error_t
EIP164_SecY_SA_Stat_I_Clear(
        const EIP164_IOArea_t * const IOArea_p,
        const unsigned int SAIndex)
{
    Device_Handle_t Device;
    const volatile EIP164_True_IOArea_t * const TrueIOArea_p = IOAREA_IN(IOArea_p);
    int rc = 0;

    EIP164_CHECK_POINTER(IOArea_p);
    EIP164_CHECK_INT_ATMOST(SAIndex + 1, TrueIOArea_p->MaxSACount);

    if ((TrueIOArea_p->Mode != EIP164_MODE_INGRESS) &&
        (TrueIOArea_p->Mode != EIP164_MODE_EGRESS_INGRESS))
    {
        return EIP164_HW_CONFIGURATION_MISMATCH;
    }

    Device = TrueIOArea_p->Device;

    /* Egress SA statistics */
    rc = EIP164_COUNTER_64_DEFAULT_WR(Device,
                                 EIP164_REG_SA_STATISTICS(SAIndex),
                                 EIP164_SA_STAT_I_IN_OCTETS_DEC);
    if (rc) return rc;
    rc = EIP164_COUNTER_64_DEFAULT_WR(Device,
                                 EIP164_REG_SA_STATISTICS(SAIndex),
                                 EIP164_SA_STAT_I_IN_OCTETS_VALIDATED);
    if (rc) return rc;
    rc = EIP164_COUNTER_64_DEFAULT_WR(Device,
                                 EIP164_REG_SA_STATISTICS(SAIndex),
                                 EIP164_SA_STAT_I_IN_PKTS_UNCHECKED);
    if (rc) return rc;
    rc = EIP164_COUNTER_64_DEFAULT_WR(Device,
                                 EIP164_REG_SA_STATISTICS(SAIndex),
                                 EIP164_SA_STAT_I_IN_PKTS_DELAYED);
    if (rc) return rc;
    rc = EIP164_COUNTER_64_DEFAULT_WR(Device,
                                 EIP164_REG_SA_STATISTICS(SAIndex),
                                 EIP164_SA_STAT_I_IN_PKTS_LATE);
    if (rc) return rc;
    rc = EIP164_COUNTER_64_DEFAULT_WR(Device,
                                 EIP164_REG_SA_STATISTICS(SAIndex),
                                 EIP164_SA_STAT_I_IN_PKTS_OK);
    if (rc) return rc;
    rc = EIP164_COUNTER_64_DEFAULT_WR(Device,
                                 EIP164_REG_SA_STATISTICS(SAIndex),
                                 EIP164_SA_STAT_I_IN_PKTS_INVALID);
    if (rc) return rc;
    rc = EIP164_COUNTER_64_DEFAULT_WR(Device,
                                 EIP164_REG_SA_STATISTICS(SAIndex),
                                 EIP164_SA_STAT_I_IN_PKTS_NOT_VALID);
    if (rc) return rc;
    rc = EIP164_COUNTER_64_DEFAULT_WR(Device,
                                 EIP164_REG_SA_STATISTICS(SAIndex),
                                 EIP164_SA_STAT_I_IN_PKTS_NOT_USING_SA);
    if (rc) return rc;

    rc = EIP164_COUNTER_64_DEFAULT_WR(Device,
                                      EIP164_REG_SA_STATISTICS(SAIndex),
                                      EIP164_SA_STAT_I_IN_PKTS_UNUSED_SA);
    if (rc) return rc;

    return EIP164_NO_ERROR;
}
#endif


#ifdef EIP164_DEVICE_MODE_EGRESS
/*----------------------------------------------------------------------------
 * EIP164_SecY_Ifc_Stat_E_Clear
 */
EIP164_Error_t
EIP164_SecY_Ifc_Stat_E_Clear(
        const EIP164_IOArea_t * const IOArea_p,
        const unsigned int IfcIndex)
{
    Device_Handle_t Device;
    const volatile EIP164_True_IOArea_t * const TrueIOArea_p = IOAREA_IN(IOArea_p);
    int rc = 0;

    EIP164_CHECK_POINTER(IOArea_p);
    EIP164_CHECK_INT_ATMOST(IfcIndex + 1, TrueIOArea_p->MaxvPortCount);

    if ((TrueIOArea_p->Mode != EIP164_MODE_EGRESS) &&
        (TrueIOArea_p->Mode != EIP164_MODE_EGRESS_INGRESS))
    {
        return EIP164_HW_CONFIGURATION_MISMATCH;
    }

    Device = TrueIOArea_p->Device;

    /* Egress IFC statistics */
    rc = EIP164_COUNTER_64_DEFAULT_WR(Device,
                                 EIP164_REG_IFC_UNCTL_STATISTICS(IfcIndex),
                                 EIP164_IFC_STAT_E_OUT_OCTETS_UNCONTROLLED);
    if (rc) return rc;
    rc = EIP164_COUNTER_64_DEFAULT_WR(Device,
                                 EIP164_REG_IFC_UNCTL_STATISTICS(IfcIndex),
                                 EIP164_IFC_STAT_E_OUT_OCTETS_CONTROLLED);
    if (rc) return rc;
    rc = EIP164_COUNTER_64_DEFAULT_WR(Device,
                                 EIP164_REG_IFC_UNCTL_STATISTICS(IfcIndex),
                                 EIP164_IFC_STAT_E_OUT_UCAST_PKTS_UNCONTROLLED);
    if (rc) return rc;
    rc = EIP164_COUNTER_64_DEFAULT_WR(Device,
                                 EIP164_REG_IFC_UNCTL_STATISTICS(IfcIndex),
                                 EIP164_IFC_STAT_E_OUT_MCAST_PKTS_UNCONTROLLED);
    if (rc) return rc;
    rc = EIP164_COUNTER_64_DEFAULT_WR(Device,
                                 EIP164_REG_IFC_UNCTL_STATISTICS(IfcIndex),
                                 EIP164_IFC_STAT_E_OUT_BCAST_PKTS_UNCONTROLLED);
    if (rc) return rc;

    /* Egress IFC1 statistics */
    rc = EIP164_COUNTER_64_DEFAULT_WR(Device,
                                 EIP164_REG_IFC_CTL_STATISTICS(IfcIndex),
                                 EIP164_IFC_STAT_E_OUT_OCTETS_COMMON);
    if (rc) return rc;
    rc = EIP164_COUNTER_64_DEFAULT_WR(Device,
                                 EIP164_REG_IFC_CTL_STATISTICS(IfcIndex),
                                 EIP164_IFC_STAT_E_OUT_UCAST_PKTS_CONTROLLED);
    if (rc) return rc;
    rc = EIP164_COUNTER_64_DEFAULT_WR(Device,
                                 EIP164_REG_IFC_CTL_STATISTICS(IfcIndex),
                                 EIP164_IFC_STAT_E_OUT_MCAST_PKTS_CONTROLLED);
    if (rc) return rc;
    rc = EIP164_COUNTER_64_DEFAULT_WR(Device,
                                 EIP164_REG_IFC_CTL_STATISTICS(IfcIndex),
                                 EIP164_IFC_STAT_E_OUT_BCAST_PKTS_CONTROLLED);
    if (rc) return rc;

    return EIP164_NO_ERROR;
}
#endif


#ifdef EIP164_DEVICE_MODE_INGRESS
/*----------------------------------------------------------------------------
 * EIP164_SecY_Ifc_Stat_I_Clear
 */
EIP164_Error_t
EIP164_SecY_Ifc_Stat_I_Clear(
        const EIP164_IOArea_t * const IOArea_p,
        const unsigned int IfcIndex)
{
    Device_Handle_t Device;
    const volatile EIP164_True_IOArea_t * const TrueIOArea_p = IOAREA_IN(IOArea_p);
    int rc = 0;

    EIP164_CHECK_POINTER(IOArea_p);
    EIP164_CHECK_INT_ATMOST(IfcIndex + 1, TrueIOArea_p->MaxvPortCount);

    if ((TrueIOArea_p->Mode != EIP164_MODE_INGRESS) &&
        (TrueIOArea_p->Mode != EIP164_MODE_EGRESS_INGRESS))
    {
        return EIP164_HW_CONFIGURATION_MISMATCH;
    }

    Device = TrueIOArea_p->Device;

    /* Ingress IFC statistics */
    rc = EIP164_COUNTER_64_DEFAULT_WR(Device,
                                 EIP164_REG_IFC_UNCTL_STATISTICS(IfcIndex),
                                 EIP164_IFC_STAT_I_IN_OCTETS_UNCONTROLLED);
    if (rc) return rc;
    rc = EIP164_COUNTER_64_DEFAULT_WR(Device,
                                 EIP164_REG_IFC_UNCTL_STATISTICS(IfcIndex),
                                 EIP164_IFC_STAT_I_IN_UCAST_PKTS_UNCONTROLLED);
    if (rc) return rc;
    rc = EIP164_COUNTER_64_DEFAULT_WR(Device,
                                 EIP164_REG_IFC_UNCTL_STATISTICS(IfcIndex),
                                 EIP164_IFC_STAT_I_IN_MCAST_PKTS_UNCONTROLLED);
    if (rc) return rc;
    rc = EIP164_COUNTER_64_DEFAULT_WR(Device,
                                 EIP164_REG_IFC_UNCTL_STATISTICS(IfcIndex),
                                 EIP164_IFC_STAT_I_IN_BCAST_PKTS_UNCONTROLLED);
    if (rc) return rc;

    /* Egress IFC1 statistics */
    rc = EIP164_COUNTER_64_DEFAULT_WR(Device,
                                 EIP164_REG_IFC_CTL_STATISTICS(IfcIndex),
                                 EIP164_IFC_STAT_I_IN_OCTETS_CONTROLLED);
    if (rc) return rc;
    rc = EIP164_COUNTER_64_DEFAULT_WR(Device,
                                 EIP164_REG_IFC_CTL_STATISTICS(IfcIndex),
                                 EIP164_IFC_STAT_I_IN_UCAST_PKTS_CONTROLLED);
    if (rc) return rc;
    rc = EIP164_COUNTER_64_DEFAULT_WR(Device,
                                 EIP164_REG_IFC_CTL_STATISTICS(IfcIndex),
                                 EIP164_IFC_STAT_I_IN_MCAST_PKTS_CONTROLLED);
    if (rc) return rc;
    rc = EIP164_COUNTER_64_DEFAULT_WR(Device,
                                 EIP164_REG_IFC_CTL_STATISTICS(IfcIndex),
                                 EIP164_IFC_STAT_I_IN_BCAST_PKTS_CONTROLLED);
    if (rc) return rc;

    return EIP164_NO_ERROR;
}
#endif


#ifdef EIP164_DEVICE_MODE_EGRESS
/*----------------------------------------------------------------------------
 * EIP164_SecY_Stat_E_Get
 */
EIP164_Error_t
EIP164_SecY_Stat_E_Get(
        const EIP164_IOArea_t * const IOArea_p,
        const unsigned int SecY_Index,
        EIP164_SecY_Statistics_E_t * const Stats_p,
        const bool fClear)
{
    Device_Handle_t Device;
    const volatile EIP164_True_IOArea_t * const TrueIOArea_p = IOAREA_IN(IOArea_p);
    int rc = 0;

    EIP164_CHECK_POINTER(IOArea_p);
    EIP164_CHECK_POINTER(Stats_p);
    EIP164_CHECK_INT_ATMOST(SecY_Index + 1, TrueIOArea_p->MaxvPortCount);

    if ((TrueIOArea_p->Mode != EIP164_MODE_EGRESS) &&
        (TrueIOArea_p->Mode != EIP164_MODE_EGRESS_INGRESS))
    {
        return EIP164_HW_CONFIGURATION_MISMATCH;
    }

    Device = TrueIOArea_p->Device;

    /* Egress SecY statistics */
    rc = EIP164Lib_Counter_Read(Device,
                              EIP164_REG_SECY_STATISTICS(SecY_Index),
                              EIP164_SECY_STAT_TRANSFORM_ERROR_PKTS,
                              &Stats_p->TransformErrorPkts.low,
                              &Stats_p->TransformErrorPkts.hi,
                              fClear);
    if (rc) return rc;
    rc = EIP164Lib_Counter_Read(Device,
                              EIP164_REG_SECY_STATISTICS(SecY_Index),
                              EIP164_SECY_STAT_E_OUT_PKTS_CTRL,
                              &Stats_p->OutPktsCtrl.low,
                              &Stats_p->OutPktsCtrl.hi,
                              fClear);
    if (rc) return rc;
    rc = EIP164Lib_Counter_Read(Device,
                              EIP164_REG_SECY_STATISTICS(SecY_Index),
                              EIP164_SECY_STAT_E_OUT_PKTS_UNTAGGED,
                              &Stats_p->OutPktsUntagged.low,
                              &Stats_p->OutPktsUntagged.hi,
                              fClear);
    if (rc) return rc;

    return EIP164_NO_ERROR;
}

#endif /* EIP164_DEVICE_MODE_EGRESS */


#ifdef EIP164_DEVICE_MODE_INGRESS
/*----------------------------------------------------------------------------
 * EIP164_SecY_Stat_I_Get
 */
EIP164_Error_t
EIP164_SecY_Stat_I_Get(
        const EIP164_IOArea_t * const IOArea_p,
        const unsigned int SecY_Index,
        EIP164_SecY_Statistics_I_t * const Stats_p,
        const bool fClear)
{
    Device_Handle_t Device;
    const volatile EIP164_True_IOArea_t * const TrueIOArea_p = IOAREA_IN(IOArea_p);
    int rc = 0;

    EIP164_CHECK_POINTER(IOArea_p);
    EIP164_CHECK_POINTER(Stats_p);
    EIP164_CHECK_INT_ATMOST(SecY_Index + 1, TrueIOArea_p->MaxvPortCount);

    if ((TrueIOArea_p->Mode != EIP164_MODE_INGRESS) &&
        (TrueIOArea_p->Mode != EIP164_MODE_EGRESS_INGRESS))
    {
        return EIP164_HW_CONFIGURATION_MISMATCH;
    }

    Device = TrueIOArea_p->Device;

    /* Ingress SecY statistics */
    rc = EIP164Lib_Counter_Read(Device,
                              EIP164_REG_SECY_STATISTICS(SecY_Index),
                              EIP164_SECY_STAT_TRANSFORM_ERROR_PKTS,
                              &Stats_p->TransformErrorPkts.low,
                              &Stats_p->TransformErrorPkts.hi,
                              fClear);
    if (rc) return rc;
    rc = EIP164Lib_Counter_Read(Device,
                              EIP164_REG_SECY_STATISTICS(SecY_Index),
                              EIP164_SECY_STAT_I_IN_PKTS_CTRL,
                              &Stats_p->InPktsCtrl.low,
                              &Stats_p->InPktsCtrl.hi,
                              fClear);
    if (rc) return rc;
    rc = EIP164Lib_Counter_Read(Device,
                              EIP164_REG_SECY_STATISTICS(SecY_Index),
                              EIP164_SECY_STAT_I_IN_PKTS_UNTAGGED,
                              &Stats_p->InPktsUntagged.low,
                              &Stats_p->InPktsUntagged.hi,
                              fClear);
    if (rc) return rc;
    rc = EIP164Lib_Counter_Read(Device,
                              EIP164_REG_SECY_STATISTICS(SecY_Index),
                              EIP164_SECY_STAT_I_IN_PKTS_NOTAG,
                              &Stats_p->InPktsNoTag.low,
                              &Stats_p->InPktsNoTag.hi,
                              fClear);
    if (rc) return rc;
    rc = EIP164Lib_Counter_Read(Device,
                              EIP164_REG_SECY_STATISTICS(SecY_Index),
                              EIP164_SECY_STAT_I_IN_PKTS_BADTAG,
                              &Stats_p->InPktsBadTag.low,
                              &Stats_p->InPktsBadTag.hi,
                              fClear);
    if (rc) return rc;
    rc = EIP164Lib_Counter_Read(Device,
                              EIP164_REG_SECY_STATISTICS(SecY_Index),
                              EIP164_SECY_STAT_I_IN_PKTS_NOSCI,
                              &Stats_p->InPktsNoSCI.low,
                              &Stats_p->InPktsNoSCI.hi,
                              fClear);
    if (rc) return rc;
    rc = EIP164Lib_Counter_Read(Device,
                              EIP164_REG_SECY_STATISTICS(SecY_Index),
                              EIP164_SECY_STAT_I_IN_PKTS_UNKNOWNSCI,
                              &Stats_p->InPktsUnknownSCI.low,
                              &Stats_p->InPktsUnknownSCI.hi,
                              fClear);
    if (rc) return rc;
    rc = EIP164Lib_Counter_Read(Device,
                              EIP164_REG_SECY_STATISTICS(SecY_Index),
                              EIP164_SECY_STAT_I_IN_PKTS_TAGGEDCTRL,
                              &Stats_p->InPktsTaggedCtrl.low,
                              &Stats_p->InPktsTaggedCtrl.hi,
                              fClear);
    if (rc) return rc;

    return EIP164_NO_ERROR;
}

#endif /* EIP164_DEVICE_MODE_INGRESS */


#ifdef EIP164_DEVICE_MODE_EGRESS
/*----------------------------------------------------------------------------
 * EIP164_SecY_SA_Stat_E_Get
 */
EIP164_Error_t
EIP164_SecY_SA_Stat_E_Get(
        const EIP164_IOArea_t * const IOArea_p,
        const unsigned int SAIndex,
        EIP164_SecY_SA_Stat_E_t * const Stat_p,
        const bool fClear)
{
    Device_Handle_t Device;
    const volatile EIP164_True_IOArea_t * const TrueIOArea_p = IOAREA_IN(IOArea_p);
    int rc = 0;

    EIP164_CHECK_POINTER(IOArea_p);
    EIP164_CHECK_POINTER(Stat_p);
    EIP164_CHECK_INT_ATMOST(SAIndex + 1, TrueIOArea_p->MaxSACount);

    if ((TrueIOArea_p->Mode != EIP164_MODE_EGRESS) &&
        (TrueIOArea_p->Mode != EIP164_MODE_EGRESS_INGRESS))
    {
        return EIP164_HW_CONFIGURATION_MISMATCH;
    }

    Device = TrueIOArea_p->Device;

    /* Egress SA statistics */
    rc = EIP164Lib_Counter_Read(Device,
                              EIP164_REG_SA_STATISTICS(SAIndex),
                              EIP164_SA_STAT_E_OUT_OCTETS_ENC_PROT,
                              &Stat_p->OutOctetsEncryptedProtected.low,
                              &Stat_p->OutOctetsEncryptedProtected.hi,
                              fClear);
    if (rc) return rc;
    rc = EIP164Lib_Counter_Read(Device,
                              EIP164_REG_SA_STATISTICS(SAIndex),
                              EIP164_SA_STAT_E_OUT_PKTS_ENC_PROT,
                              &Stat_p->OutPktsEncryptedProtected.low,
                              &Stat_p->OutPktsEncryptedProtected.hi,
                              fClear);
    if (rc) return rc;
    rc = EIP164Lib_Counter_Read(Device,
                              EIP164_REG_SA_STATISTICS(SAIndex),
                              EIP164_SA_STAT_E_OUT_PKTS_TOO_LONG,
                              &Stat_p->OutPktsTooLong.low,
                              &Stat_p->OutPktsTooLong.hi,
                              fClear);
    if (rc) return rc;
    rc = EIP164Lib_Counter_Read(Device,
                              EIP164_REG_SA_STATISTICS(SAIndex),
                              EIP164_SA_STAT_E_OUT_PKTS_SA_NOT_IN_USE,
                              &Stat_p->OutPktsSANotInUse.low,
                              &Stat_p->OutPktsSANotInUse.hi,
                              fClear);
    if (rc) return rc;

    return EIP164_NO_ERROR;
}
#endif /* EIP164_DEVICE_MODE_EGRESS */


#ifdef EIP164_DEVICE_MODE_INGRESS
/*----------------------------------------------------------------------------
 * EIP164_SecY_SA_Stat_I_Get
 */
EIP164_Error_t
EIP164_SecY_SA_Stat_I_Get(
        const EIP164_IOArea_t * const IOArea_p,
        const unsigned int SAIndex,
        EIP164_SecY_SA_Stat_I_t * const Stat_p,
        const bool fClear)
{
    Device_Handle_t Device;
    const volatile EIP164_True_IOArea_t * const TrueIOArea_p = IOAREA_IN(IOArea_p);
    int rc = 0;

    EIP164_CHECK_POINTER(IOArea_p);
    EIP164_CHECK_POINTER(Stat_p);
    EIP164_CHECK_INT_ATMOST(SAIndex + 1, TrueIOArea_p->MaxSACount);

    Device = TrueIOArea_p->Device;

    if ((TrueIOArea_p->Mode != EIP164_MODE_INGRESS) &&
        (TrueIOArea_p->Mode != EIP164_MODE_EGRESS_INGRESS))
    {
        return EIP164_HW_CONFIGURATION_MISMATCH;
    }

    /* Ingress SA statistics */
    rc = EIP164Lib_Counter_Read(Device,
                              EIP164_REG_SA_STATISTICS(SAIndex),
                              EIP164_SA_STAT_I_IN_OCTETS_DEC,
                              &Stat_p->InOctetsDecrypted.low,
                              &Stat_p->InOctetsDecrypted.hi,
                              fClear);
    if (rc) return rc;
    rc = EIP164Lib_Counter_Read(Device,
                              EIP164_REG_SA_STATISTICS(SAIndex),
                              EIP164_SA_STAT_I_IN_OCTETS_VALIDATED,
                              &Stat_p->InOctetsValidated.low,
                              &Stat_p->InOctetsValidated.hi,
                              fClear);
    if (rc) return rc;
    rc = EIP164Lib_Counter_Read(Device,
                              EIP164_REG_SA_STATISTICS(SAIndex),
                              EIP164_SA_STAT_I_IN_PKTS_UNCHECKED,
                              &Stat_p->InPktsUnchecked.low,
                              &Stat_p->InPktsUnchecked.hi,
                              fClear);
    if (rc) return rc;
    rc = EIP164Lib_Counter_Read(Device,
                              EIP164_REG_SA_STATISTICS(SAIndex),
                              EIP164_SA_STAT_I_IN_PKTS_DELAYED,
                              &Stat_p->InPktsDelayed.low,
                              &Stat_p->InPktsDelayed.hi,
                              fClear);
    if (rc) return rc;
    rc = EIP164Lib_Counter_Read(Device,
                              EIP164_REG_SA_STATISTICS(SAIndex),
                              EIP164_SA_STAT_I_IN_PKTS_LATE,
                              &Stat_p->InPktsLate.low,
                              &Stat_p->InPktsLate.hi,
                              fClear);
    if (rc) return rc;
    rc = EIP164Lib_Counter_Read(Device,
                              EIP164_REG_SA_STATISTICS(SAIndex),
                              EIP164_SA_STAT_I_IN_PKTS_OK,
                              &Stat_p->InPktsOK.low,
                              &Stat_p->InPktsOK.hi,
                              fClear);
    if (rc) return rc;
    rc = EIP164Lib_Counter_Read(Device,
                              EIP164_REG_SA_STATISTICS(SAIndex),
                              EIP164_SA_STAT_I_IN_PKTS_INVALID,
                              &Stat_p->InPktsInvalid.low,
                              &Stat_p->InPktsInvalid.hi,
                              fClear);
    if (rc) return rc;
    rc = EIP164Lib_Counter_Read(Device,
                              EIP164_REG_SA_STATISTICS(SAIndex),
                              EIP164_SA_STAT_I_IN_PKTS_NOT_VALID,
                              &Stat_p->InPktsNotValid.low,
                              &Stat_p->InPktsNotValid.hi,
                              fClear);
    if (rc) return rc;
    rc = EIP164Lib_Counter_Read(Device,
                              EIP164_REG_SA_STATISTICS(SAIndex),
                              EIP164_SA_STAT_I_IN_PKTS_NOT_USING_SA,
                              &Stat_p->InPktsNotUsingSA.low,
                              &Stat_p->InPktsNotUsingSA.hi,
                              fClear);
    if (rc) return rc;
    rc = EIP164Lib_Counter_Read(Device,
                              EIP164_REG_SA_STATISTICS(SAIndex),
                              EIP164_SA_STAT_I_IN_PKTS_UNUSED_SA,
                              &Stat_p->InPktsUnusedSA.low,
                              &Stat_p->InPktsUnusedSA.hi,
                              fClear);
    if (rc) return rc;

    return EIP164_NO_ERROR;
}
#endif /* EIP164_DEVICE_MODE_INGRESS */


#ifdef EIP164_DEVICE_MODE_EGRESS
/*----------------------------------------------------------------------------
 * EIP164_SecY_Ifc_Stat_E_Get
 */
EIP164_Error_t
EIP164_SecY_Ifc_Stat_E_Get(
        const EIP164_IOArea_t * const IOArea_p,
        const unsigned int IfcIndex,
        EIP164_SecY_Ifc_Stat_E_t * const Stat_p,
        const bool fClear)
{
    Device_Handle_t Device;
    const volatile EIP164_True_IOArea_t * const TrueIOArea_p = IOAREA_IN(IOArea_p);
    int rc = 0;

    EIP164_CHECK_POINTER(IOArea_p);
    EIP164_CHECK_POINTER(Stat_p);
    EIP164_CHECK_INT_ATMOST(IfcIndex + 1, TrueIOArea_p->MaxvPortCount);

    if ((TrueIOArea_p->Mode != EIP164_MODE_EGRESS) &&
        (TrueIOArea_p->Mode != EIP164_MODE_EGRESS_INGRESS))
    {
        return EIP164_HW_CONFIGURATION_MISMATCH;
    }

    Device = TrueIOArea_p->Device;

    /* Egress IFC statistics */
    rc = EIP164Lib_Counter_Read(Device,
                              EIP164_REG_IFC_UNCTL_STATISTICS(IfcIndex),
                              EIP164_IFC_STAT_E_OUT_OCTETS_UNCONTROLLED,
                              &Stat_p->OutOctetsUncontrolled.low,
                              &Stat_p->OutOctetsUncontrolled.hi,
                              fClear);
    if (rc) return rc;

    rc = EIP164Lib_Counter_Read(Device,
                              EIP164_REG_IFC_UNCTL_STATISTICS(IfcIndex),
                              EIP164_IFC_STAT_E_OUT_OCTETS_CONTROLLED,
                              &Stat_p->OutOctetsControlled.low,
                              &Stat_p->OutOctetsControlled.hi,
                              fClear);
    if (rc) return rc;

    rc = EIP164Lib_Counter_Read(Device,
                              EIP164_REG_IFC_UNCTL_STATISTICS(IfcIndex),
                              EIP164_IFC_STAT_E_OUT_UCAST_PKTS_UNCONTROLLED,
                              &Stat_p->OutPktsUnicastUncontrolled.low,
                              &Stat_p->OutPktsUnicastUncontrolled.hi,
                              fClear);
    if (rc) return rc;

    rc = EIP164Lib_Counter_Read(Device,
                              EIP164_REG_IFC_UNCTL_STATISTICS(IfcIndex),
                              EIP164_IFC_STAT_E_OUT_MCAST_PKTS_UNCONTROLLED,
                              &Stat_p->OutPktsMulticastUncontrolled.low,
                              &Stat_p->OutPktsMulticastUncontrolled.hi,
                              fClear);
    if (rc) return rc;

    rc = EIP164Lib_Counter_Read(Device,
                              EIP164_REG_IFC_UNCTL_STATISTICS(IfcIndex),
                              EIP164_IFC_STAT_E_OUT_BCAST_PKTS_UNCONTROLLED,
                              &Stat_p->OutPktsBroadcastUncontrolled.low,
                              &Stat_p->OutPktsBroadcastUncontrolled.hi,
                              fClear);
    if (rc) return rc;

    /* Egress IFC1 statistics */
    rc = EIP164Lib_Counter_Read(Device,
                              EIP164_REG_IFC_CTL_STATISTICS(IfcIndex),
                              EIP164_IFC_STAT_E_OUT_OCTETS_COMMON,
                              &Stat_p->OutOctetsCommon.low,
                              &Stat_p->OutOctetsCommon.hi,
                              fClear);
    if (rc) return rc;

    rc = EIP164Lib_Counter_Read(Device,
                              EIP164_REG_IFC_CTL_STATISTICS(IfcIndex),
                              EIP164_IFC_STAT_E_OUT_UCAST_PKTS_CONTROLLED,
                              &Stat_p->OutPktsUnicastControlled.low,
                              &Stat_p->OutPktsUnicastControlled.hi,
                              fClear);
    if (rc) return rc;

    rc = EIP164Lib_Counter_Read(Device,
                              EIP164_REG_IFC_CTL_STATISTICS(IfcIndex),
                              EIP164_IFC_STAT_E_OUT_MCAST_PKTS_CONTROLLED,
                              &Stat_p->OutPktsMulticastControlled.low,
                              &Stat_p->OutPktsMulticastControlled.hi,
                              fClear);
    if (rc) return rc;

    rc = EIP164Lib_Counter_Read(Device,
                              EIP164_REG_IFC_CTL_STATISTICS(IfcIndex),
                              EIP164_IFC_STAT_E_OUT_BCAST_PKTS_CONTROLLED,
                              &Stat_p->OutPktsBroadcastControlled.low,
                              &Stat_p->OutPktsBroadcastControlled.hi,
                              fClear);
    if (rc) return rc;

    return EIP164_NO_ERROR;
}
#endif /* EIP164_DEVICE_MODE_EGRESS */


#ifdef EIP164_DEVICE_MODE_INGRESS
/*----------------------------------------------------------------------------
 * EIP164_SecY_Ifc_Stat_I_Get
 */
EIP164_Error_t
EIP164_SecY_Ifc_Stat_I_Get(
        const EIP164_IOArea_t * const IOArea_p,
        const unsigned int IfcIndex,
        EIP164_SecY_Ifc_Stat_I_t * const Stat_p,
        const bool fClear)
{
    Device_Handle_t Device;
    const volatile EIP164_True_IOArea_t * const TrueIOArea_p = IOAREA_IN(IOArea_p);
    int rc = 0;

    EIP164_CHECK_POINTER(IOArea_p);
    EIP164_CHECK_POINTER(Stat_p);
    EIP164_CHECK_INT_ATMOST(IfcIndex + 1, TrueIOArea_p->MaxvPortCount);

    Device = TrueIOArea_p->Device;

    if ((TrueIOArea_p->Mode != EIP164_MODE_INGRESS) &&
        (TrueIOArea_p->Mode != EIP164_MODE_EGRESS_INGRESS))
    {
        return EIP164_HW_CONFIGURATION_MISMATCH;
    }

    /* Ingress IFC statistics */
    rc = EIP164Lib_Counter_Read(Device,
                              EIP164_REG_IFC_UNCTL_STATISTICS(IfcIndex),
                              EIP164_IFC_STAT_I_IN_OCTETS_UNCONTROLLED,
                              &Stat_p->InOctetsUncontrolled.low,
                              &Stat_p->InOctetsUncontrolled.hi,
                              fClear);
    if (rc) return rc;

    rc = EIP164Lib_Counter_Read(Device,
                              EIP164_REG_IFC_UNCTL_STATISTICS(IfcIndex),
                              EIP164_IFC_STAT_I_IN_UCAST_PKTS_UNCONTROLLED,
                              &Stat_p->InPktsUnicastUncontrolled.low,
                              &Stat_p->InPktsUnicastUncontrolled.hi,
                              fClear);
    if (rc) return rc;

    rc = EIP164Lib_Counter_Read(Device,
                              EIP164_REG_IFC_UNCTL_STATISTICS(IfcIndex),
                              EIP164_IFC_STAT_I_IN_MCAST_PKTS_UNCONTROLLED,
                              &Stat_p->InPktsMulticastUncontrolled.low,
                              &Stat_p->InPktsMulticastUncontrolled.hi,
                              fClear);
    if (rc) return rc;

    rc = EIP164Lib_Counter_Read(Device,
                              EIP164_REG_IFC_UNCTL_STATISTICS(IfcIndex),
                              EIP164_IFC_STAT_I_IN_BCAST_PKTS_UNCONTROLLED,
                              &Stat_p->InPktsBroadcastUncontrolled.low,
                              &Stat_p->InPktsBroadcastUncontrolled.hi,
                              fClear);
    if (rc) return rc;

    /* Ingress IFC1 statistics */
    rc = EIP164Lib_Counter_Read(Device,
                              EIP164_REG_IFC_CTL_STATISTICS(IfcIndex),
                              EIP164_IFC_STAT_I_IN_OCTETS_CONTROLLED,
                              &Stat_p->InOctetsControlled.low,
                              &Stat_p->InOctetsControlled.hi,
                              fClear);
    if (rc) return rc;

    rc = EIP164Lib_Counter_Read(Device,
                              EIP164_REG_IFC_CTL_STATISTICS(IfcIndex),
                              EIP164_IFC_STAT_I_IN_UCAST_PKTS_CONTROLLED,
                              &Stat_p->InPktsUnicastControlled.low,
                              &Stat_p->InPktsUnicastControlled.hi,
                              fClear);
    if (rc) return rc;

    rc = EIP164Lib_Counter_Read(Device,
                              EIP164_REG_IFC_CTL_STATISTICS(IfcIndex),
                              EIP164_IFC_STAT_I_IN_MCAST_PKTS_CONTROLLED,
                              &Stat_p->InPktsMulticastControlled.low,
                              &Stat_p->InPktsMulticastControlled.hi,
                              fClear);
    if (rc) return rc;

    rc = EIP164Lib_Counter_Read(Device,
                              EIP164_REG_IFC_CTL_STATISTICS(IfcIndex),
                              EIP164_IFC_STAT_I_IN_BCAST_PKTS_CONTROLLED,
                              &Stat_p->InPktsBroadcastControlled.low,
                              &Stat_p->InPktsBroadcastControlled.hi,
                              fClear);
    if (rc) return rc;

    return EIP164_NO_ERROR;
}
#endif /* EIP164_DEVICE_MODE_INGRESS */

/* end of file eip164_secy_sa.c */
