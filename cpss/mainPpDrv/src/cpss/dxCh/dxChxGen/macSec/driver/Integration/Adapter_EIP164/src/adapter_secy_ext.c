/* adapter_secy_ext.c
 *
 * SecY Extended API Adapter implementation.
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

#include <Integration/Adapter_EIP164/incl/api_secy_ext.h>  /* Extended SecY API */

#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>
/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

/* Top-level Adapter configuration */
#include <Integration/Adapter_EIP164/incl/c_adapter_eip164.h>             /* ADAPTER_EIP164_DRIVER_NAME */

#include <Integration/Adapter_EIP164/incl/api_secy.h>                     /* SecY API */


/* Logging API */
#include <Kit/Log/incl/log.h>                /* LOG_* */

/* Driver Framework C Library Abstraction API */
#include <Kit/DriverFramework/incl/clib.h>               /* cpssOsMemSet()/memcpy() */

/* SA Builder API */
#include <Kit/SABuilder_MACsec/incl/sa_builder_macsec.h>

/* EIP-164 Driver Library SecY API */
#include <Kit/EIP164/incl/eip164_types.h>
#include <Kit/EIP164/incl/eip164_secy.h>

/* List API */
#include <Kit/List/incl/list.h>

/* Adapter Alloc API */
#include <Integration/Adapter_EIP164/incl/adapter_alloc.h>      /* Adapter_Alloc()/_Free() */

#include <Integration/Adapter_EIP164/incl/adapter_secy_support.h>  /* Adapter internal SecY API */

/*----------------------------------------------------------------------------
 * Definitions and macros
 */
#define SC_INDEX_INVALID 0xffffffff
#define SA_INDEX_INVALID 0xffffffff

/*----------------------------------------------------------------------------
 * SecYLib_DeviceSA_Flow_Copy
 */
static void
SecYLib_DeviceSA_Flow_Copy(
        const EIP164_SecY_SA_t * const DeviceSA_p,
        SecY_SA_t * const SA_p)
{
    switch(DeviceSA_p->DestPort)
    {
    default:
    case EIP164_PORT_COMMON:
        SA_p->DestPort = SECY_PORT_COMMON;
        break;

    case EIP164_PORT_CONTROLLED:
        SA_p->DestPort = SECY_PORT_CONTROLLED;
        break;

    case EIP164_PORT_UNCONTROLLED:
        SA_p->DestPort = SECY_PORT_UNCONTROLLED;
        break;
    }

    /* Action type */
    if (DeviceSA_p->ActionType == EIP164_SECY_SA_ACTION_EGRESS)
    {
        SA_p->ActionType = SECY_SA_ACTION_EGRESS;

        SA_p->Params.Egress.fConfProtect =
            DeviceSA_p->Params.Egress.fConfProtect;
        SA_p->Params.Egress.fProtectFrames =
            DeviceSA_p->Params.Egress.fProtectFrames;
        SA_p->Params.Egress.fUseES = DeviceSA_p->Params.Egress.fUseES;
        SA_p->Params.Egress.fUseSCB = DeviceSA_p->Params.Egress.fUseSCB;
        SA_p->Params.Egress.fIncludeSCI =
            DeviceSA_p->Params.Egress.fIncludeSCI;
        SA_p->Params.Egress.ConfidentialityOffset =
            DeviceSA_p->Params.Egress.ConfidentialityOffset;
        SA_p->Params.Egress.fAllowDataPkts =
            DeviceSA_p->Params.Egress.fAllowDataPkts;
        SA_p->Params.Egress.PreSecTagAuthStart =
            DeviceSA_p->Params.Egress.PreSecTagAuthStart;
        SA_p->Params.Egress.PreSecTagAuthLength =
            DeviceSA_p->Params.Egress.PreSecTagAuthLength;
    }
    else if (DeviceSA_p->ActionType == EIP164_SECY_SA_ACTION_INGRESS)
    {
        SA_p->ActionType = SECY_SA_ACTION_INGRESS;

        SA_p->Params.Ingress.fRetainSecTAG =
            DeviceSA_p->Params.Ingress.fRetainSecTAG;
        SA_p->Params.Ingress.fRetainICV =
            DeviceSA_p->Params.Ingress.fRetainICV;
        SA_p->Params.Ingress.fReplayProtect =
            DeviceSA_p->Params.Ingress.fReplayProtect;
        SA_p->Params.Ingress.ValidateFramesTagged =
            DeviceSA_p->Params.Ingress.ValidateFramesTagged;
        SA_p->Params.Ingress.ConfidentialityOffset =
            DeviceSA_p->Params.Ingress.ConfidentialityOffset;
        SA_p->Params.Ingress.fAllowTagged =
            DeviceSA_p->Params.Ingress.fAllowTagged;
        SA_p->Params.Ingress.fAllowUntagged =
            DeviceSA_p->Params.Ingress.fAllowUntagged;
        SA_p->Params.Ingress.fValidateUntagged =
            DeviceSA_p->Params.Ingress.fValidateUntagged;
        SA_p->Params.Ingress.PreSecTagAuthStart =
            DeviceSA_p->Params.Ingress.PreSecTagAuthStart;
        SA_p->Params.Ingress.PreSecTagAuthLength =
            DeviceSA_p->Params.Ingress.PreSecTagAuthLength;
    }
    else if (DeviceSA_p->ActionType == EIP164_SECY_SA_ACTION_BYPASS)
    {
        SA_p->ActionType = SECY_SA_ACTION_BYPASS;
    }
    else if (DeviceSA_p->ActionType == EIP164_SECY_SA_ACTION_DROP)
    {
        SA_p->ActionType = SECY_SA_ACTION_DROP;
    }
    else
    {
        SA_p->ActionType = SECY_SA_ACTION_CRYPT_AUTH;

        SA_p->Params.CryptAuth.fZeroLengthMessage =
            DeviceSA_p->Params.CryptAuth.fZeroLengthMessage;
        SA_p->Params.CryptAuth.ConfidentialityOffset =
            DeviceSA_p->Params.CryptAuth.ConfidentialityOffset;
        SA_p->Params.CryptAuth.IVMode =
            DeviceSA_p->Params.CryptAuth.IVMode;
        SA_p->Params.CryptAuth.fICVAppend =
            DeviceSA_p->Params.CryptAuth.fICVAppend;
        SA_p->Params.CryptAuth.fICVVerify =
            DeviceSA_p->Params.CryptAuth.fICVVerify;
        SA_p->Params.CryptAuth.fConfProtect =
            DeviceSA_p->Params.CryptAuth.fConfProtect;
    }

    /* Drop type */
    if (DeviceSA_p->DropType == EIP164_SECY_SA_DROP_CRC_ERROR)
    {
        SA_p->DropType = SECY_SA_DROP_CRC_ERROR;
    }
    else if (DeviceSA_p->DropType == EIP164_SECY_SA_DROP_PKT_ERROR)
    {
        SA_p->DropType = SECY_SA_DROP_PKT_ERROR;
    }
    else
    {
        SA_p->DropType = SECY_SA_DROP_INTERNAL;
    }
}


/*----------------------------------------------------------------------------
 * SecY_Device_Role_Get
 */
SecY_Status_t
SecY_Device_Role_Get(
        const unsigned int DeviceId,
        SecY_Role_t * const Role_p)
{
    LOG_INFO("\n %s \n", __func__);

#ifdef ADAPTER_EIP164_STRICT_ARGS
    if (DeviceId >= ADAPTER_EIP164_MAX_NOF_DEVICES)
        return SECY_ERROR_BAD_PARAMETER;

    if (Role_p == NULL)
        return SECY_ERROR_BAD_PARAMETER;
#endif

    SecYLib_Device_Lock(DeviceId);

    if (!PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device_StaticFields)[DeviceId].fInitialized)
    {
        SecYLib_Device_Unlock(DeviceId);
        return SECY_ERROR_INTERNAL;
    }
    *Role_p = PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->Role;

    SecYLib_Device_Unlock(DeviceId);

    LOG_INFO("\n %s done \n", __func__);

    return SECY_STATUS_OK;
}


/*----------------------------------------------------------------------------
 * SecY_vPort_Params_Read
 */
SecY_Status_t
SecY_vPort_Params_Read(
        const unsigned int DeviceId,
        const unsigned int vPortId,
        SecY_SA_t * const SA_p)
{
    EIP164_SecY_SA_t DeviceSA;
    EIP164_Error_t Rc;

    LOG_INFO("\n %s \n", __func__);

    ZEROINIT(DeviceSA);
#ifdef ADAPTER_EIP164_STRICT_ARGS
    if (DeviceId >= ADAPTER_EIP164_MAX_NOF_DEVICES)
        return SECY_ERROR_BAD_PARAMETER;

    if (SA_p == NULL)
        return SECY_ERROR_BAD_PARAMETER;
#endif

    SecYLib_Device_Lock(DeviceId);

    if (!PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device_StaticFields)[DeviceId].fInitialized)
    {
        SecYLib_Device_Unlock(DeviceId);
        return SECY_ERROR_INTERNAL;
    }

    cpssOsMemSet (SA_p, 0, sizeof(SecY_SA_t));
    Rc = EIP164_SecY_SAMFlow_Read(&PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->IOArea,
                                  vPortId,
                                  &DeviceSA
                                 );

    if (Rc != EIP164_NO_ERROR)
    {
        LOG_CRIT("%s: Failed to Read SA flow parameters from device  %d\n",
                 __func__,
                 DeviceId);
        SecYLib_Device_Unlock(DeviceId);
        return SECY_DLIB_ERROR(Rc);
    }

    SecYLib_DeviceSA_Flow_Copy(&DeviceSA, SA_p);

    SecYLib_Device_Unlock(DeviceId);

    LOG_INFO("\n %s done \n", __func__);

    return SECY_STATUS_OK;
}


/*----------------------------------------------------------------------------
 * SecY_SA_Params_Read
 */
SecY_Status_t
SecY_SA_Params_Read(
        const unsigned int DeviceId,
        const SecY_SAHandle_t SAHandle,
        SecY_SA_t * const SA_p,
        uint8_t * const SCI_p)
{
    unsigned int SAIndex = 0;
    EIP164_SecY_SA_t DeviceSA;
    EIP164_Error_t Rc;
    SecY_Status_t SecY_Rc = SECY_STATUS_OK;
    unsigned int vPort = 0;
    unsigned int SCIndex = 0;

    LOG_INFO("\n %s \n", __func__);
    ZEROINIT(DeviceSA);

#ifdef ADAPTER_EIP164_STRICT_ARGS
    if (DeviceId >= ADAPTER_EIP164_MAX_NOF_DEVICES)
        return SECY_ERROR_BAD_PARAMETER;

    if (SA_p == NULL)
        return SECY_ERROR_BAD_PARAMETER;

    if (SecYLib_SAHandle_IsSame(&SAHandle, &SecY_SAHandle_NULL))
        return SECY_ERROR_BAD_PARAMETER;
#endif

    /* Get the SA index in the device for this SA handle */
    if (!SecYLib_SAHandleToIndex(SAHandle, &SAIndex, &SCIndex, &vPort))
    {
        LOG_CRIT("%s: Invalid SA handle for EIP-164 device for device %d "
                 "(%s)\n",
                 __func__, DeviceId, ADAPTER_EIP164_DEVICE_NAME(DeviceId));
        return SECY_ERROR_INTERNAL;
    }

    SecYLib_Device_Lock(DeviceId);

    if (!PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device_StaticFields)[DeviceId].fInitialized)
    {
        SecYLib_Device_Unlock(DeviceId);
        return SECY_ERROR_INTERNAL;
    }

    cpssOsMemSet (SA_p, 0, sizeof(SecY_SA_t));
    Rc = EIP164_SecY_SAMFlow_Read(&PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->IOArea,
                                  vPort,
                                  &DeviceSA
                                  );

    if (Rc != EIP164_NO_ERROR)
    {
        LOG_CRIT("%s: Failed to Read SA flow parameters from device  %d\n",
                 __func__,
                 DeviceId);
        SecY_Rc =  SECY_DLIB_ERROR(Rc);
        goto error_exit;
    }

    SecYLib_DeviceSA_Flow_Copy(&DeviceSA, SA_p);

    switch(SA_p->ActionType)
    {
    case SECY_SA_ACTION_EGRESS:
#ifdef ADAPTER_EIP164_MODE_EGRESS
    {
        unsigned int ActiveSAIndex;
        bool fSAInUse;
        SA_p->SA_WordCount = 21;
        Rc = EIP164_SecY_SA_Installation_Verify(&PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->IOArea,
                                                vPort,
                                                &ActiveSAIndex,
                                                &fSAInUse);
        if (Rc != EIP164_NO_ERROR)
        {
            LOG_CRIT("%s: Failed to get active egress SA  from device  %d\n",
                 __func__,
                 DeviceId);
            SecY_Rc =  SECY_DLIB_ERROR(Rc);
            goto error_exit;
        }

        if (fSAInUse && ActiveSAIndex == SAIndex)
        {
            SA_p->Params.Egress.fSAInUse = true;
        }
    }
#else
    SecY_Rc =  SECY_ERROR_INTERNAL;
    goto error_exit;
#endif
    break;

    case  SECY_SA_ACTION_INGRESS:
#ifdef ADAPTER_EIP164_MODE_INGRESS
    {
        unsigned int ActiveSAIndex[4];
        bool fSAInUse[4];
        Rc = EIP164_SecY_SA_Active_I_Get(&PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->IOArea,
                                         SCIndex,
                                         ActiveSAIndex,
                                         fSAInUse);
        if (Rc != EIP164_NO_ERROR)
        {
            LOG_CRIT("%s: Failed to get activeingress SA  from device  %d\n",
                 __func__,
                 DeviceId);
            SecY_Rc =  SECY_DLIB_ERROR(Rc);
            goto error_exit;
        }
        if (fSAInUse[0] && ActiveSAIndex[0] == SAIndex)
        {
            SA_p->Params.Ingress.fSAInUse = true;
            SA_p->Params.Ingress.AN = 0;
        }
        else if (fSAInUse[1] && ActiveSAIndex[1] == SAIndex)
        {
            SA_p->Params.Ingress.fSAInUse = true;
            SA_p->Params.Ingress.AN = 1;
        }
        else if (fSAInUse[2] && ActiveSAIndex[2] == SAIndex)
        {
            SA_p->Params.Ingress.fSAInUse = true;
            SA_p->Params.Ingress.AN = 2;
        }
        else if (fSAInUse[3] && ActiveSAIndex[3] == SAIndex)
        {
            SA_p->Params.Ingress.fSAInUse = true;
            SA_p->Params.Ingress.AN = 3;
        }
        if (SCI_p)
        {
            SA_p->Params.Ingress.SCI_p = SCI_p;
            cpssOsMemCpy(SCI_p, PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->SCDscr_p[SCIndex].SCI, 8);
        }
        SA_p->SA_WordCount = 19;
    }
#else
    IDENTIFIER_NOT_USED(SCI_p);
    SecY_Rc =  SECY_ERROR_INTERNAL;
    goto error_exit;
#endif
    break;

    default:
        SA_p->SA_WordCount = 19;
    }

error_exit:
    SecYLib_Device_Unlock(DeviceId);

    LOG_INFO("\n %s done \n", __func__);

    return SecY_Rc;
}


/*----------------------------------------------------------------------------
 * SecY_SA_vPortIndex_Get
 */
SecY_Status_t
SecY_SA_vPortIndex_Get(
        const unsigned int DeviceId,
        const SecY_SAHandle_t SAHandle,
        unsigned int * const vPort_p)
{
    unsigned int SAIndex;
    IDENTIFIER_NOT_USED(DeviceId);

    if (SecYLib_SAHandle_IsSame(&SAHandle, &SecY_SAHandle_NULL))
    {
        return SECY_ERROR_BAD_PARAMETER;
    }

    if (vPort_p == NULL)
    {
        return SECY_ERROR_BAD_PARAMETER;
    }

    if (SecYLib_SAHandleToIndex(SAHandle, &SAIndex, NULL, vPort_p) == false)
    {
        return SECY_ERROR_INTERNAL;
    }

    return SECY_STATUS_OK;
}


/*----------------------------------------------------------------------------
 * SecY_SA_Next_Get
 */
SecY_Status_t
SecY_SA_Next_Get(
        const unsigned int DeviceId,
        const SecY_SAHandle_t CurrentSAHandle,
        SecY_SAHandle_t * const NextSAHandle_p)
{
    unsigned int SAIndex;
    SecYLib_Device_Lock(DeviceId);

    if (!PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device_StaticFields)[DeviceId].fInitialized)
    {
        SecYLib_Device_Unlock(DeviceId);
        return SECY_ERROR_INTERNAL;
    }

    if (SecYLib_SAHandle_IsSame(&CurrentSAHandle, &SecY_SAHandle_NULL))
    {
        /* Select the first SA index. */
        SAIndex = 0;
    }
    else
    {
        if (SecYLib_SAHandleToIndex(CurrentSAHandle, &SAIndex, NULL, NULL) ==
            false)
        {
            SecYLib_Device_Unlock(DeviceId);
            return SECY_ERROR_INTERNAL;
        }
        SAIndex++;       /* Select the first index after the current one. */
    }

    for ( ; SAIndex < PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->SACount; SAIndex++)
    {
        if (PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->SADscr_p[SAIndex].Magic ==
            ADAPTER_EIP164_SA_DSCR_MAGIC)
        {
            SecYLib_Device_Unlock(DeviceId);
            return SecY_SAHandle_Get(DeviceId, SAIndex, NextSAHandle_p);
        }
    }

    *NextSAHandle_p = SecY_SAHandle_NULL;
    SecYLib_Device_Unlock(DeviceId);
    return SECY_STATUS_OK;
}


#ifdef ADAPTER_EIP164_MODE_EGRESS
/*----------------------------------------------------------------------------
 * SecY_SA_Chained_Get
 */
SecY_Status_t
SecY_SA_Chained_Get(
        const unsigned int DeviceId,
        const SecY_SAHandle_t CurrentSAHandle,
        SecY_SAHandle_t * const NextSAHandle_p)
{
    unsigned int SAIndex;
    unsigned int NewSAIndex;
    bool fSAInUse;
    SecY_Status_t SecY_Rc;

    if (SecYLib_SAHandle_IsSame(&CurrentSAHandle, &SecY_SAHandle_NULL))
    {
        return SECY_ERROR_BAD_PARAMETER;
    }

    if (NextSAHandle_p == NULL)
    {
        return SECY_ERROR_BAD_PARAMETER;
    }

    SecYLib_Device_Lock(DeviceId);

    if (!PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device_StaticFields)[DeviceId].fInitialized)
    {
        SecYLib_Device_Unlock(DeviceId);
        return SECY_ERROR_INTERNAL;
    }

    if (SecYLib_SAHandleToIndex(CurrentSAHandle, &SAIndex, NULL, NULL) == false)
    {
        SecYLib_Device_Unlock(DeviceId);
        return SECY_ERROR_INTERNAL;
    }

    SecY_Rc = SecYLib_SA_Chained_Get(DeviceId,
                                     SAIndex,
                                     &NewSAIndex,
                                     &fSAInUse);
    if (SecY_Rc != SECY_STATUS_OK)
    {
        SecYLib_Device_Unlock(DeviceId);
        return SecY_Rc;
    }

    SecYLib_Device_Unlock(DeviceId);
    if (fSAInUse)
        return SecY_SAHandle_Get(DeviceId, NewSAIndex, NextSAHandle_p);
    else
        *NextSAHandle_p = SecY_SAHandle_NULL;

    return SECY_STATUS_OK;
}
#endif


#ifdef ADAPTER_EIP164_MODE_INGRESS
/*----------------------------------------------------------------------------
 * SecY_SCI_Next_Get
 */
SecY_Status_t
SecY_SCI_Next_Get(
        const unsigned int DeviceId,
        const unsigned int vPort,
        const void ** TmpHandle_p,
        uint8_t * const NextSCI_p,
        uint32_t * const SCIndex_p)
{
    SecY_SC_Descriptor_t *SCDscr_p;
    List_Status_t List_Rc;
    void *SCList;
    const List_Element_t *cur;

    SecYLib_Device_Lock(DeviceId);

    if (!PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device_StaticFields)[DeviceId].fInitialized)
    {
        SecYLib_Device_Unlock(DeviceId);
        return SECY_ERROR_INTERNAL;
    }

    SCList = PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->vPortDscr_p[vPort].SCList;
    if (*TmpHandle_p == NULL)
    {
        /* Get first record from list. */
        List_Rc = List_GetHead(0, SCList, &cur);

        if (List_Rc != LIST_STATUS_OK)
        {
            LOG_CRIT("%s: Failed, no SCI list for device %d\n",
                     __func__, DeviceId);
            SecYLib_Device_Unlock(DeviceId);
            return SECY_ERROR_INTERNAL;
        }
    }
    else
    {
        cur = *TmpHandle_p;
        cur = List_GetNextElement(cur);
    }

    *TmpHandle_p = cur;
    if (cur != NULL)
    {
        /* Found next descriptor, return the SCI. */
        SCDscr_p = cur->DataObject_p;
        cpssOsMemCpy(NextSCI_p, SCDscr_p->SCI, 8);
        if (SCIndex_p)
            *SCIndex_p = SCDscr_p->SCIndex;
    }
    SecYLib_Device_Unlock(DeviceId);
    return SECY_STATUS_OK;
}
#endif


/*-----------------------------------------------------------------------------
 * SecY_SA_WindowSize_Get
 */
SecY_Status_t
SecY_SA_WindowSize_Get(
        const unsigned int DeviceId,
        const SecY_SAHandle_t SAHandle,
        uint32_t * const  WindowSize_p)
{
    SecY_Status_t SecY_Rc;
    SABuilder_Status_t SAB_Rc;
    unsigned int Offset;
    unsigned int SAIndex;
    uint32_t Transform_0;

#ifdef ADAPTER_EIP164_STRICT_ARGS
    if (DeviceId >= ADAPTER_EIP164_MAX_NOF_DEVICES)
        return SECY_ERROR_BAD_PARAMETER;

    if (SecY_SAHandle_IsSame(&SAHandle, &SecY_SAHandle_NULL))
        return SECY_ERROR_BAD_PARAMETER;

    if (WindowSize_p == NULL)
        return SECY_ERROR_BAD_PARAMETER;
#endif

    SecYLib_Device_Lock(DeviceId);

    if (!PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device_StaticFields)[DeviceId].fInitialized)
    {
        SecYLib_Device_Unlock(DeviceId);
        return SECY_ERROR_INTERNAL;
    }

    /* Get the SA index in the device for this SA handle */
    if (!SecYLib_SAHandleToIndex(SAHandle, &SAIndex, NULL, NULL))
    {
        LOG_CRIT("%s: Invalid SA handle for EIP-160 device for device %d "
                 "(%s)\n",
                 __func__, DeviceId, ADAPTER_EIP164_DEVICE_NAME(DeviceId));
        SecYLib_Device_Unlock(DeviceId);
        return SECY_ERROR_INTERNAL;
    }


    SecY_Rc = SecYLib_SA_Read(DeviceId, SAIndex, 0, 1, &Transform_0);
    if (SecY_Rc != SECY_STATUS_OK)
    {
        SecYLib_Device_Unlock(DeviceId);
        return SecY_Rc;
    }

    SAB_Rc = SABuilder_WindowSizeOffset_Get(Transform_0, &Offset);
    if (SAB_Rc != SAB_STATUS_OK)
    {
        SecYLib_Device_Unlock(DeviceId);
        return SECY_ERROR_INTERNAL;
    }

    SecY_Rc = SecYLib_SA_Read(DeviceId, SAIndex, Offset, 1, WindowSize_p);
    if (SecY_Rc != SECY_STATUS_OK)
    {
        SecYLib_Device_Unlock(DeviceId);
        return SecY_Rc;
    }

    SecYLib_Device_Unlock(DeviceId);
    return SECY_STATUS_OK;
}


/*----------------------------------------------------------------------------
 * SecY_SA_NextPN_Get
 */
SecY_Status_t
SecY_SA_NextPN_Get(
        const unsigned int DeviceId,
        const SecY_SAHandle_t SAHandle,
        uint32_t * const NextPN_Lo_p,
        uint32_t * const NextPN_Hi_p,
        bool * fExtPN_p)
{
    SecY_Status_t SecY_Rc;
    SABuilder_Status_t SAB_Rc;
    unsigned int Offset;
    uint32_t  SeqNum[2];
    unsigned int SAIndex;
    uint32_t Transform_0;

#ifdef ADAPTER_EIP164_STRICT_ARGS
    if (DeviceId >= ADAPTER_EIP164_MAX_NOF_DEVICES)
        return SECY_ERROR_BAD_PARAMETER;

    if (SecY_SAHandle_IsSame(&SAHandle, &SecY_SAHandle_NULL))
        return SECY_ERROR_BAD_PARAMETER;

    if (fExtPN_p == NULL || NextPN_Lo_p == NULL)
        return SECY_ERROR_BAD_PARAMETER;

#endif

    SecYLib_Device_Lock(DeviceId);

    if (!PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device_StaticFields)[DeviceId].fInitialized)
    {
        SecYLib_Device_Unlock(DeviceId);
        return SECY_ERROR_INTERNAL;
    }

    /* Get the SA index in the device for this SA handle */
    if (!SecYLib_SAHandleToIndex(SAHandle, &SAIndex, NULL, NULL))
    {
        LOG_CRIT("%s: Invalid SA handle for EIP-160 device for device %d "
                 "(%s)\n",
                 __func__, DeviceId, ADAPTER_EIP164_DEVICE_NAME(DeviceId));
        SecYLib_Device_Unlock(DeviceId);
        return SECY_ERROR_INTERNAL;
    }


    SecY_Rc = SecYLib_SA_Read(DeviceId, SAIndex, 0, 1, &Transform_0);
    if (SecY_Rc != SECY_STATUS_OK)
    {
        SecYLib_Device_Unlock(DeviceId);
        return SECY_ERROR_INTERNAL;
    }

    SAB_Rc = SABuilder_SeqNumOffset_Get(Transform_0, &Offset, fExtPN_p);
    if (SAB_Rc != SAB_STATUS_OK)
    {
        SecYLib_Device_Unlock(DeviceId);
        return SECY_ERROR_INTERNAL;
    }

    SecY_Rc = SecYLib_SA_Read(DeviceId, SAIndex, Offset, *fExtPN_p?2:1, SeqNum);
    if (SecY_Rc != SECY_STATUS_OK)
    {
        SecYLib_Device_Unlock(DeviceId);
        return SECY_ERROR_INTERNAL;
    }

    *NextPN_Lo_p = SeqNum[0];

    if (NextPN_Hi_p != NULL && *fExtPN_p)
    {
        *NextPN_Hi_p = SeqNum[1];
    }

    SecYLib_Device_Unlock(DeviceId);
    return SECY_STATUS_OK;
}


/*----------------------------------------------------------------------------
 * SecY_Diag_Device_Dump
 */
SecY_Status_t
SecY_Diag_Device_Dump(
        const unsigned int DeviceId)
{
    SecY_Status_t SecY_Rc;

    SecY_Role_t Role;
    unsigned int ChannelCount, vPortCount, SACount, SCCount;
    Log_FormattedMessage("Secy_Diag_Device of device %d\n",DeviceId);
    /* Secy_Device_Limits */
    {
        SecY_Rc = SecY_Device_Role_Get(DeviceId, &Role);
        if (SecY_Rc != SECY_STATUS_OK)
        {
            LOG_CRIT("SecY_Device_Role_Get returned error %d\n",SecY_Rc);
            return SecY_Rc;
        }

        SecY_Rc = SecY_Device_Limits(DeviceId,
                                     &ChannelCount,
                                     &vPortCount,
                                     &SACount,
                                     &SCCount);
        if (SecY_Rc != SECY_STATUS_OK)
        {
            LOG_CRIT("SecY_Device_Limits returned error %d\n",SecY_Rc);
            return SecY_Rc;
        }
        Log_FormattedMessage("Secy_Diag_Device Role=%s "
                             "Channels=%d vPorts=%d SAs=%d SCs=%d\n",
                             Role==SECY_ROLE_INGRESS?"INGRESS":
                             (Role==SECY_ROLE_EGRESS?"EGRESS":"INGRESS-EGRESS"),
                             ChannelCount, vPortCount, SACount, SCCount);

    }

    /* SecY_Device_Config_Get */
    {
        SecY_Device_Params_t DeviceParams;
        SecY_EOPConf_t EOPParams;
        SecY_ECCConf_t ECCParams;
        SecY_Statistics_Control_t StatsParams;
        unsigned int MaxSecYChannels = 0;
        uint16_t i = 0;

        ZEROINIT(DeviceParams);
        ZEROINIT(StatsParams);
        DeviceParams.EOPConf_p = &EOPParams;
        DeviceParams.ECCConf_p = &ECCParams;
        DeviceParams.StatControl_p = &StatsParams;

        SecY_Rc = SecY_Device_Config_Get(DeviceId,
                                         &DeviceParams);
        if (SecY_Rc != SECY_STATUS_OK)
        {
            LOG_CRIT("SecY_Device_Config_Get returned error %d\n",SecY_Rc);
            return SecY_Rc;
        }

        SecY_Rc = SecY_Device_Limits(DeviceId,
                                     &MaxSecYChannels,
                                     NULL,
                                     NULL,
                                     NULL);
        if (SecY_Rc != SECY_STATUS_OK)
        {
            LOG_CRIT("SecY_Device_Limits returned error %d\n", SecY_Rc);
            return SecY_Rc;
        }

        Log_FormattedMessage(
            "Statistics Control: fAutoStatCntrsReset=%d CountIncDisCtrl=0x%x\n",
            StatsParams.fAutoStatCntrsReset,
            StatsParams.CountIncDisCtrl);
        Log_FormattedMessage("SeqNrThreshold=%u SeqNrThreshold64 lo=%u hi=%u\n",
                             StatsParams.SeqNrThreshold,
                             StatsParams.SeqNrThreshold64.Lo,
                             StatsParams.SeqNrThreshold64.Hi);
        Log_FormattedMessage("SACountFrameThr    lo=%10u hi=%10u\n",
                             StatsParams.SACountFrameThr.Lo,
                             StatsParams.SACountFrameThr.Hi);
        Log_FormattedMessage("SecYCountFrameThr  lo=%10u hi=%10u\n",
                             StatsParams.SecYCountFrameThr.Lo,
                             StatsParams.SecYCountFrameThr.Hi);
        Log_FormattedMessage("IFCCountFrameThr   lo=%10u hi=%10u\n",
                             StatsParams.IFCCountFrameThr.Lo,
                             StatsParams.IFCCountFrameThr.Hi);
        Log_FormattedMessage("IFC1CountFrameThr  lo=%10u hi=%10u\n",
                             StatsParams.IFC1CountFrameThr.Lo,
                             StatsParams.IFC1CountFrameThr.Hi);
        Log_FormattedMessage("RxCAMCountFrameThr lo=%10u hi=%10u\n",
                             StatsParams.RxCAMCountFrameThr.Lo,
                             StatsParams.RxCAMCountFrameThr.Hi);
        Log_FormattedMessage("SACountOctetThr    lo=%10u hi=%10u\n",
                             StatsParams.SACountOctetThr.Lo,
                             StatsParams.SACountOctetThr.Hi);
        Log_FormattedMessage("IFCCountOctetThr   lo=%10u hi=%10u\n",
                             StatsParams.IFCCountOctetThr.Lo,
                             StatsParams.IFCCountOctetThr.Hi);
        Log_FormattedMessage("IFC1CountOctetThr  lo=%10u hi=%10u\n",
                             StatsParams.IFC1CountOctetThr.Lo,
                             StatsParams.IFC1CountOctetThr.Hi);

        Log_FormattedMessage("EOP config: EOPTimeoutVal = %u\n",
                             EOPParams.EOPTimeoutVal);

        for (i = 0; i < ((MaxSecYChannels + 31) / 32); i++)
            Log_FormattedMessage("EOP config: EOPTimeoutCtrl = 0x%x\n",
                                  EOPParams.EOPTimeoutCtrl.ch_bitmask[i]);

        Log_FormattedMessage(
            "ECC config: ECCCorrectable=%d ECCUncorrectable=%d\n",
            ECCParams.ECCCorrectableThr,
            ECCParams.ECCUncorrectableThr);
    }

    /* SecY_CryptAuth_BypassLen_Get */
    {
        unsigned int BypassLen;
        SecY_Rc = SecY_CryptAuth_BypassLen_Get(DeviceId, &BypassLen);
        if (SecY_Rc != SECY_STATUS_OK)
        {
            LOG_CRIT(
                "SecY_CryptAuth_BypassLen_Get returned error %d\n",SecY_Rc);
            return SecY_Rc;
        }
        Log_FormattedMessage("CryptAuthBypassLen=%u\n",BypassLen);
    }

    /* SecY_Device_Status_Get */
    {
        SecY_ECCStatus_t ECCStatus;
        SecY_PktProcessDebug_t PktProcessDebug;
        SecY_DeviceStatus_t DeviceStatus;
        unsigned int i;
        ZEROINIT(DeviceStatus);
        ZEROINIT(PktProcessDebug);
        DeviceStatus.ECCStatus_p = &ECCStatus;
        DeviceStatus.PktProcessDebug_p = &PktProcessDebug;
        SecY_Rc = SecY_Device_Status_Get(
            DeviceId,
            &DeviceStatus);
        if (SecY_Rc != SECY_STATUS_OK)
        {
            LOG_CRIT("SecY_Device_Status_Get returned error %d\n",SecY_Rc);
            return SecY_Rc;
        }
        for (i = 0; i < SECY_ECC_NOF_STATUS_COUNTERS; i++)
        {
            Log_FormattedMessage(
                "ECC counter %u: correctable=%u thr=%d , "
                "uncorrectable=%u thr=%d\n",
                i,
                ECCStatus.Counters[i].CorrectableCount,
                ECCStatus.Counters[i].fCorrectableThr,
                ECCStatus.Counters[i].UncorrectableCount,
                                ECCStatus.Counters[i].fUncorrectableThr);
        }
        Log_FormattedMessage(
          "PktProcess ParsedDALo=0x%08x ParsedDAHi=0x%08x\n"
          "           ParsedSALo=0x%08x ParsedSAHi=0x%08x\n"
          "           ParsedSegTAGLo=0x%08x ParsedSecTAGHi=0x%08x\n"
          "           ParsedSCILo=0x%08x ParsedSCIHi=0x%08x SecTAGDeb=0x%08x\n"
          "           RxCAMSCILo=0x%08x RxCAMSCIHi=0x%08x ParserInDeb=0x%08x\n",
          PktProcessDebug.ParsedDALo,
          PktProcessDebug.ParsedDAHi,
          PktProcessDebug.ParsedSALo,
          PktProcessDebug.ParsedSAHi,
          PktProcessDebug.ParsedSecTAGLo,
          PktProcessDebug.ParsedSecTAGHi,
          PktProcessDebug.ParsedSCILo,
          PktProcessDebug.ParsedSCIHi,
          PktProcessDebug.SecTAGDebug,
          PktProcessDebug.RxCAMSCILo,
          PktProcessDebug.RxCAMSCIHi,
          PktProcessDebug.ParserInDebug);
    }

    /* SecY_Channel_PacketsInflight_Get */
    {
        uint32_t Mask;
        SecY_Rc = SecY_Channel_PacketsInflight_Get(DeviceId, 0, NULL, &Mask);
        if (SecY_Rc != SECY_STATUS_OK)
        {
            LOG_CRIT("SecY_Channel_PacketsInflight_Get returned error %d\n",
                     SecY_Rc);
            return SecY_Rc;
        }
        Log_FormattedMessage("PacketsInflight Mask=%u\n",Mask);
    }

    /* Statistics summary */
    {
        unsigned int *SummaryIndexes_p;
        unsigned int i, SummaryCount;
        unsigned int AllocSize=sizeof(unsigned int)*MAX(vPortCount,SACount);

        SummaryIndexes_p = Adapter_Alloc(AllocSize);
        if (SummaryIndexes_p == NULL)
        {
            return SECY_ERROR_INTERNAL;
        }


        SecY_Rc = SecY_SA_PnThrSummary_CheckAndClear(
            DeviceId,
            &SummaryIndexes_p,
            &SummaryCount);
        if (SecY_Rc != SECY_STATUS_OK)
        {
            LOG_CRIT("SecY_SA_PnThrSummary_CheckAndClear returned error %d\n",
                     SecY_Rc);
            Adapter_Free(SummaryIndexes_p);
            return SecY_Rc;
        }
        if (SummaryCount == 0)
        {
            Log_FormattedMessage("No SAs crossed Pn Threshold\n");
        }
        else
        {
            Log_FormattedMessage("The following SAs crossed Pn threshold:\n");
            for(i=0; i<SummaryCount; i++)
            {
                Log_FormattedMessage("%d\n",SummaryIndexes_p[i]);
            }
        }


        SecY_Rc = SecY_SA_ExpiredSummary_CheckAndClear(
            DeviceId,
            &SummaryIndexes_p,
            &SummaryCount);
        if (SecY_Rc != SECY_STATUS_OK)
        {
            LOG_CRIT("SecY_SA_ExpiredSummary_CheckAndClear returned error %d\n",
                     SecY_Rc);
            Adapter_Free(SummaryIndexes_p);
            return SecY_Rc;
        }
        if (SummaryCount == 0)
        {
             Log_FormattedMessage("No SAs were expired\n");
        }
        else
        {
            Log_FormattedMessage("The following SAs were expired:\n");
             for(i=0; i<SummaryCount; i++)
             {
                Log_FormattedMessage("%d\n",SummaryIndexes_p[i]);
            }
        }


        SecY_Rc = SecY_Device_CountSummary_PSecY_CheckAndClear(
            DeviceId,
            &SummaryIndexes_p,
            &SummaryCount);
        if (SecY_Rc != SECY_STATUS_OK)
        {
            LOG_CRIT("SecY_Device_CountSummary_PSecY_CheckAndClear"
                     " returned error %d\n", SecY_Rc);
            Adapter_Free(SummaryIndexes_p);
            return SecY_Rc;
        }
        if (SummaryCount == 0)
        {
            Log_FormattedMessage("No Per-SecY statistics crossed threshold\n");
        }
        else
        {
            Log_FormattedMessage(
                "The following Per-SecY statistics crossed threshold:\n");
            for(i=0; i<SummaryCount; i++)
            {
                Log_FormattedMessage("%d\n",SummaryIndexes_p[i]);
            }
        }

        SecY_Rc = SecY_Device_CountSummary_PIfc_CheckAndClear(
            DeviceId,
            &SummaryIndexes_p,
            &SummaryCount);
        if (SecY_Rc != SECY_STATUS_OK)
        {
            LOG_CRIT("SecY_Device_CountSummary_PIfcCheckAndClear"
                     " returned error %d\n", SecY_Rc);
            Adapter_Free(SummaryIndexes_p);
            return SecY_Rc;
        }
        if (SummaryCount == 0)
        {
            Log_FormattedMessage("No Per-Ifc statistics crossed threshold\n");
        }
        else
        {
            Log_FormattedMessage(
                "The following Per-Ifc statistics crossed threshold:\n");
            for(i=0; i<SummaryCount; i++)
            {
                Log_FormattedMessage("%d\n",SummaryIndexes_p[i]);
            }
        }

        SecY_Rc = SecY_Device_CountSummary_PIfc1_CheckAndClear(
            DeviceId,
            &SummaryIndexes_p,
            &SummaryCount);
        if (SecY_Rc != SECY_STATUS_OK)
        {
            LOG_CRIT("SecY_Device_CountSummary_PIfc1_CheckAndClear"
                     " returned error %d\n",SecY_Rc);
            Adapter_Free(SummaryIndexes_p);
            return SecY_Rc;
        }
        if (SummaryCount == 0)
        {
            Log_FormattedMessage("No Per-Ifc1 statistics crossed threshold\n");
        }
        else
        {
            Log_FormattedMessage(
                "The following Per-Ifc1 statistics crossed threshold:\n");
            for(i=0; i<SummaryCount; i++)
            {
                Log_FormattedMessage("%d\n",SummaryIndexes_p[i]);
            }
        }


        SecY_Rc = SecY_Device_CountSummary_PSA_CheckAndClear(
            DeviceId,
            &SummaryIndexes_p,
            &SummaryCount);
        if (SecY_Rc != SECY_STATUS_OK)
        {
            LOG_CRIT("SecY_Device_CountSummary_PSA_CheckAndClear"
                     " returned error %d\n", SecY_Rc);
            Adapter_Free(SummaryIndexes_p);
            return SecY_Rc;
        }
        if (SummaryCount == 0)
        {
            Log_FormattedMessage("No Per-SA statistics crossed threshold\n");
        }
        else
        {
            Log_FormattedMessage(
                "The following Per-SA statistics crossed threshold:\n");
            for(i=0; i<SummaryCount; i++)
            {
                Log_FormattedMessage("%d\n",SummaryIndexes_p[i]);
            }
        }


#ifdef ADAPTER_EIP164_MODE_INGRESS
        if (Role == SECY_ROLE_INGRESS)
        {
            SecY_Rc = SecY_Device_CountSummary_PRxCAM_CheckAndClear(
                DeviceId,
                &SummaryIndexes_p,
                &SummaryCount);
            if (SecY_Rc != SECY_STATUS_OK)
            {
                LOG_CRIT("SecY_Device_CountSummary_PRxCAM_CheckAndClear"
                         " returned error %d\n", SecY_Rc);
                Adapter_Free(SummaryIndexes_p);
                return SecY_Rc;
            }
            if (SummaryCount == 0)
            {
                Log_FormattedMessage(
                    "No Per-RxCAM statistics crossed threshold\n");
            }
            else
            {
                Log_FormattedMessage(
                    "The following Per-RxCAM statistics crossed threshold:\n");
                for(i=0; i<SummaryCount; i++)
                {
                    Log_FormattedMessage("%d\n",SummaryIndexes_p[i]);
                }
            }
        }
#endif
        Adapter_Free(SummaryIndexes_p);
    }

    return SECY_STATUS_OK;
}

/*----------------------------------------------------------------------------
 * SecY_Diag_Channel_Dump
 */
SecY_Status_t
SecY_Diag_Channel_Dump(
        const unsigned int DeviceId,
        const unsigned int ChannelId,
        const bool fAllChannels)
{
    SecY_Status_t SecY_Rc;
    if (fAllChannels)
    {
        unsigned int ChannelCount;
        unsigned int i;
        SecY_Rc = SecY_Device_Limits(DeviceId,
                                     &ChannelCount,
                                     NULL,
                                     NULL,
                                     NULL);
        if (SecY_Rc != SECY_STATUS_OK)
        {
            LOG_CRIT("SecY_Device_Limits returned error %d\n",SecY_Rc);
            return SecY_Rc;
        }
        for (i=0; i<ChannelCount; i++)
        {
            SecY_Rc = SecY_Diag_Channel_Dump(DeviceId, i, false);
            if (SecY_Rc != SECY_STATUS_OK)
            {
                LOG_CRIT("SecY_Diag_Channel_Dump returned error %d"
                         " for channel %d\n", SecY_Rc, i);
                return SecY_Rc;
            }
        }
    }
    else
    {
        SecY_Channel_t ChannelParams;
        bool fInflight;
        ZEROINIT(ChannelParams);
        Log_FormattedMessage("SecY_Diag_Channel device=%d channel=%d\n",
                             DeviceId, ChannelId);
        SecY_Rc = SecY_Channel_Config_Get(DeviceId,
                                          ChannelId,
                                          &ChannelParams);
        if (SecY_Rc != SECY_STATUS_OK)
        {
            LOG_CRIT("SecY_Channel_Config_Get returned error %d\n",SecY_Rc);
            return SecY_Rc;
        }
        Log_FormattedMessage(
            "fLowLatencyBypass=%d fLatencyEnable=%d latency=%d\n",
            ChannelParams.fLowLatencyBypass,
            ChannelParams.fLatencyEnable,
            ChannelParams.Latency);

        Log_FormattedMessage(
            "BurstLimit=%d fPktNumThrMode=%d Egress EtherType=0x%04x\n",
            ChannelParams.BurstLimit,
            ChannelParams.fPktNumThrMode,
            ChannelParams.EtherType);
        Log_FormattedMessage(
            "StatCtrl SeqNrThreshold=%u Extended SeqNrThreshold=(lo=%u hi=%u)\n",
            ChannelParams.StatCtrl.SeqNrThreshold,
            ChannelParams.StatCtrl.SeqNrThreshold64.Lo,
            ChannelParams.StatCtrl.SeqNrThreshold64.Hi);
        Log_FormattedMessage(
            "RuleSecTAG fCompEType=%d fCheckV=%d fCheckKay=%d\n"
            "  fCheckCE=%d fCheckSC=%d fCheckSL=%d fCkeckPN=%d\n"
            "  EtherType=0x%04x\n",
            ChannelParams.RuleSecTAG.fCompEType,
            ChannelParams.RuleSecTAG.fCheckV,
            ChannelParams.RuleSecTAG.fCheckKay,
            ChannelParams.RuleSecTAG.fCheckCE,
            ChannelParams.RuleSecTAG.fCheckSC,
            ChannelParams.RuleSecTAG.fCheckSL,
            ChannelParams.RuleSecTAG.fCheckPN,
            ChannelParams.RuleSecTAG.EtherType);
        SecY_Rc = SecY_Channel_PacketsInflight_Get(DeviceId,
                                                   ChannelId,
                                                   &fInflight,
                                                   NULL);
        if (SecY_Rc != SECY_STATUS_OK)
        {
            LOG_CRIT("SecY_Channel_PacketInflight_Get"
                     " returned error %d\n", SecY_Rc);
            return SecY_Rc;
        }
        Log_FormattedMessage("Packets in flight for this channel: %d\n",
                             fInflight);
    }
    return SECY_STATUS_OK;
}


/*----------------------------------------------------------------------------
 * SecY_Diag_vPort_Dump
 */
SecY_Status_t
SecY_Diag_vPort_Dump(
        const unsigned int DeviceId,
        const unsigned int vPortId,
        const bool fAllvPorts,
        const bool fIncludeSA)
{
    SecY_Status_t SecY_Rc;

    if (fAllvPorts)
    {
        unsigned int vPortCount;
        unsigned int i;
        SecY_Rc = SecY_Device_Limits(DeviceId,
                                     NULL,
                                     &vPortCount,
                                     NULL,
                                     NULL);
        if (SecY_Rc != SECY_STATUS_OK)
        {
            LOG_CRIT("SecY_Device_Limits returned error %d\n",SecY_Rc);
            return SecY_Rc;
        }
        for (i=0; i<vPortCount; i++)
        {
            SecY_Rc = SecY_Diag_vPort_Dump(DeviceId, i, false, fIncludeSA);
            if (SecY_Rc != SECY_STATUS_OK)
            {
                LOG_CRIT("SecY_Diag_vPort_Dump"
                         " returned error %d for channel %d\n", SecY_Rc, i);
                return SecY_Rc;
            }
        }
    }
    else
    {
        SecY_Role_t Role;
        SecY_SA_t SAParams;
        SecY_Rc = SecY_Device_Role_Get(DeviceId, &Role);
        if (SecY_Rc != SECY_STATUS_OK)
        {
            LOG_CRIT("SecY_Device_Role_Get returned error %d\n",SecY_Rc);
            return SecY_Rc;
        }
        Log_FormattedMessage("SecY_Diag_vPort #%d Role=%s\n",vPortId,
                             Role==SECY_ROLE_EGRESS?"EGRESS":"INGRESS");

        SecY_Rc = SecY_vPort_Params_Read(DeviceId, vPortId, &SAParams);
        if (SecY_Rc != SECY_STATUS_OK)
        {
            LOG_CRIT("SecY_SA_Params_Read returned error %d\n",SecY_Rc);
            return SecY_Rc;
        }

        Log_FormattedMessage(
            "Drop type=%s Port type=%s\n",
            SAParams.DropType == SECY_SA_DROP_CRC_ERROR ? "CRC_ERR" :
            (SAParams.DropType == SECY_SA_DROP_PKT_ERROR? "PKT_ERR" :
             (SAParams.DropType == SECY_SA_DROP_INTERNAL?"INTERNAL":"NONE")),
            SAParams.DestPort == SECY_PORT_COMMON ?"COMMON":
            (SAParams.DestPort == SECY_PORT_RESERVED ?"RESERVED":
             (SAParams.DestPort == SECY_PORT_CONTROLLED ?"CONTROLLED":
              "UNCONTROLLED")));

        switch (SAParams.ActionType)
        {
        case SECY_SA_ACTION_EGRESS:
            Log_FormattedMessage("Action is EGRESS\n");
            Log_FormattedMessage(
                "ConfidentialityOffset=%d fProtectFrames=%d\n"
                "fIncludeSCI=%d fUseES=%d fUseSCB=%d fConfProtect=%d"
                " fAllowDataPkts=%d\n"
                "PreSecTagAuthStart=%d PreSecTagAuthLength=%d\n",
                SAParams.Params.Egress.ConfidentialityOffset,
                SAParams.Params.Egress.fProtectFrames,
                SAParams.Params.Egress.fIncludeSCI,
                SAParams.Params.Egress.fUseES,
                SAParams.Params.Egress.fUseSCB,
                SAParams.Params.Egress.fConfProtect,
                SAParams.Params.Egress.fAllowDataPkts,
                SAParams.Params.Egress.PreSecTagAuthStart,
                SAParams.Params.Egress.PreSecTagAuthLength);
            break;
        case SECY_SA_ACTION_INGRESS:
            Log_FormattedMessage("Action is INGRESS\n");
            Log_FormattedMessage(
                "ConfidentialityOffset=%d fReplayProtect=%d"
                " ValidateFramesTagged=%s\n"
                "fAllowTagged=%d fAllowUntagged=%d fValidateUntagged=%d\n"
                "PreSecTagAuthStart=%d PreSecTagAuthLength=%d\n",
                SAParams.Params.Ingress.ConfidentialityOffset,
                SAParams.Params.Ingress.fReplayProtect,
                SAParams.Params.Ingress.ValidateFramesTagged ==
                SECY_FRAME_VALIDATE_DISABLE ?"DISABLE":
                (SAParams.Params.Ingress.ValidateFramesTagged ==
                 SECY_FRAME_VALIDATE_CHECK?"CHECK":"STRICT"),
                SAParams.Params.Ingress.fAllowTagged,
                SAParams.Params.Ingress.fAllowUntagged,
                SAParams.Params.Ingress.fValidateUntagged,
                SAParams.Params.Ingress.PreSecTagAuthStart,
                SAParams.Params.Ingress.PreSecTagAuthLength);
            break;
        case SECY_SA_ACTION_CRYPT_AUTH:
            Log_FormattedMessage("Action is CRYPT_AUTH\n");
            Log_FormattedMessage(
                "fZeroLengthMessage=%d ConfidentialityOffset=%d\n"
                "IVMode=%d fICVAppend=%d fICVVerify=%d fConfProtect=%d\n",
                SAParams.Params.CryptAuth.fZeroLengthMessage,
                SAParams.Params.CryptAuth.ConfidentialityOffset,
                SAParams.Params.CryptAuth.IVMode,
                SAParams.Params.CryptAuth.fICVAppend,
                SAParams.Params.CryptAuth.fICVVerify,
                SAParams.Params.CryptAuth.fConfProtect);
            break;
        case SECY_SA_ACTION_DROP:
            Log_FormattedMessage("Action is DROP\n");
            break;
        case SECY_SA_ACTION_BYPASS:
            Log_FormattedMessage("Action is BYPASS\n");
            break;
        default:
            break;
        }
#ifdef ADAPTER_EIP164_MODE_EGRESS
        if (Role==SECY_ROLE_EGRESS)
        {
            SecY_Ifc_Stat_E_t Ifc_Stat;
            SecY_SC_Rule_MTUCheck_t MTUCheck;
            SecY_SAHandle_t SAHandle1, SAHandle2;

            SecY_Rc = SecY_Ifc_Statistics_E_Get(DeviceId,
                                                vPortId,
                                                &Ifc_Stat,
                                                false);

            if (SecY_Rc != SECY_STATUS_OK)
            {
                LOG_CRIT("SecY_Ifc_Statistics_E_Get returned error %d\n",
                         SecY_Rc);
                return SecY_Rc;
            }

            Log_FormattedMessage(
                "IFC stats OutPktsUnicastUncontrolled   lo=%10u hi=%10u\n"
                "          OutPktsMulticastUncontrolled lo=%10u hi=%10u\n"
                "          OutPktsBroadcastUncontrolled lo=%10u hi=%10u\n"
                "          OutPktsUnicastControlled     lo=%10u hi=%10u\n"
                "          OutPktsMulticastControlled   lo=%10u hi=%10u\n"
                "          OutPktsBroadcastControlled   lo=%10u hi=%10u\n"
                "          OutOctetsUncontrolled        lo=%10u hi=%10u\n"
                "          OutOctetsControlled          lo=%10u hi=%10u\n"
                "          OutOctetsCommon              lo=%10u hi=%10u\n",
                Ifc_Stat.OutPktsUnicastUncontrolled.Lo,
                Ifc_Stat.OutPktsUnicastUncontrolled.Hi,
                Ifc_Stat.OutPktsMulticastUncontrolled.Lo,
                Ifc_Stat.OutPktsMulticastUncontrolled.Hi,
                Ifc_Stat.OutPktsBroadcastUncontrolled.Lo,
                Ifc_Stat.OutPktsBroadcastUncontrolled.Hi,
                Ifc_Stat.OutPktsUnicastControlled.Lo,
                Ifc_Stat.OutPktsUnicastControlled.Hi,
                Ifc_Stat.OutPktsMulticastControlled.Lo,
                Ifc_Stat.OutPktsMulticastControlled.Hi,
                Ifc_Stat.OutPktsBroadcastControlled.Lo,
                Ifc_Stat.OutPktsBroadcastControlled.Hi,
                Ifc_Stat.OutOctetsUncontrolled.Lo,
                Ifc_Stat.OutOctetsUncontrolled.Hi,
                Ifc_Stat.OutOctetsControlled.Lo,
                Ifc_Stat.OutOctetsControlled.Hi,
                Ifc_Stat.OutOctetsCommon.Lo,
                Ifc_Stat.OutOctetsCommon.Hi);

            switch (SAParams.ActionType)
            {
            case SECY_SA_ACTION_BYPASS:
            case SECY_SA_ACTION_DROP:
            case SECY_SA_ACTION_INGRESS:
            case SECY_SA_ACTION_EGRESS:
            case SECY_SA_ACTION_CRYPT_AUTH:
            default:
            {
                SecY_SecY_Stat_E_t SecY_Stat;

                SecY_Rc = SecY_SecY_Statistics_E_Get(DeviceId,
                                                     vPortId,
                                                     &SecY_Stat,
                                                     false);
                if (SecY_Rc != SECY_STATUS_OK)
                {
                    LOG_CRIT("SecY_SecY_Statistics_E_Get returned error %d\n",
                             SecY_Rc);
                    return SecY_Rc;
                }

                Log_FormattedMessage(
                    "SecY Stats OutPktsTransformErr         lo=%10u hi=%10u\n"
                    "           OutPktsControl              lo=%10u hi=%10u\n"
                    "           OutPktsUntagged             lo=%10u hi=%10u\n",
                    SecY_Stat.OutPktsTransformError.Lo,
                    SecY_Stat.OutPktsTransformError.Hi,
                    SecY_Stat.OutPktsControl.Lo,
                    SecY_Stat.OutPktsControl.Hi,
                    SecY_Stat.OutPktsUntagged.Lo,
                    SecY_Stat.OutPktsUntagged.Hi);
            }
            break;

            };

            SecY_Rc = SecY_Rules_MTUCheck_Get(DeviceId,
                                              vPortId,
                                              &MTUCheck);
            if (SecY_Rc != SECY_STATUS_OK)
            {
                LOG_CRIT("SecY_SecY_Rule_MTUCheck_Get returned error %d\n",
                         SecY_Rc);
                return SecY_Rc;
            }

            Log_FormattedMessage(
                "MTUCheck PacketMaxByteCount=%d fOverSizeDrop=%d\n",
                MTUCheck.PacketMaxByteCount,
                MTUCheck.fOverSizeDrop);

            SecY_Rc = SecY_SA_Active_E_Get(DeviceId,
                                           vPortId,
                                           &SAHandle1);
            if (SecY_Rc != SECY_STATUS_OK ||
                SecY_SAHandle_IsSame(&SAHandle1, &SecY_SAHandle_NULL))
            {
                Log_FormattedMessage("No SA active\n");
            }
            else
            {
                unsigned int SAIndex;
                SAHandle2 = SecY_SAHandle_NULL;
                while (!SecY_SAHandle_IsSame(&SAHandle1, &SecY_SAHandle_NULL))
                {
                    if (!SecY_SAHandle_IsSame(&SAHandle2, &SecY_SAHandle_NULL))
                    {
                        Log_FormattedMessage("Found chained SA\n");
                    }
                    if (fIncludeSA)
                    {
                        SecY_Rc = SecY_Diag_SA_Dump(DeviceId, SAHandle1, false);
                        if (SecY_Rc != SECY_STATUS_OK)
                        {
                            LOG_CRIT("SecY_Diag_SA_Dump  returned error %d\n",
                                     SecY_Rc);
                            return SecY_Rc;
                        }
                    }
                    else
                    {
                        SecY_Rc = SecY_SAIndex_Get(SAHandle1, &SAIndex, NULL);
                        if (SecY_Rc != SECY_STATUS_OK)
                        {
                            LOG_CRIT("SecY_SAIndex_Get returned error %d\n",
                                     SecY_Rc);
                            return SecY_Rc;
                        }
                        Log_FormattedMessage("SA index %d\n", SAIndex);
                    }
                    SecY_Rc = SecY_SA_Chained_Get(DeviceId,
                                                  SAHandle1,
                                                  &SAHandle2);
                    if (SecY_Rc != SECY_STATUS_OK)
                    {
                        LOG_CRIT("SecY_SA_Chained_Get returned error %d\n",
                                 SecY_Rc);
                        return SecY_Rc;
                    }
                    SAHandle1 = SAHandle2;
                }
            }
        }
#endif
#ifdef ADAPTER_EIP164_MODE_INGRESS
        if (Role==SECY_ROLE_INGRESS)
        {
            SecY_Ifc_Stat_I_t Ifc_Stat;
            uint8_t SCI[8];
            const void * SCI_Handle;
            unsigned int SCIndex = SC_INDEX_INVALID;
            SecY_SAHandle_t SAHandle[4];
            unsigned int i, j, SCCount;

            SecY_Rc = SecY_Ifc_Statistics_I_Get(DeviceId,
                                                vPortId,
                                                &Ifc_Stat,
                                                false);
            if (SecY_Rc != SECY_STATUS_OK)
            {
                LOG_CRIT("SecY_Ifc_Statistics_I_Get returned error %d\n",
                         SecY_Rc);
                return SecY_Rc;
            }
            Log_FormattedMessage(
                "IFC stats InPktsUnicastUncontrolled   lo=%10u hi=%10u\n"
                "          InPktsMulticastUncontrolled lo=%10u hi=%10u\n"
                "          InPktsBroadcastUncontrolled lo=%10u hi=%10u\n"
                "          InPktsUnicastControlled     lo=%10u hi=%10u\n"
                "          InPktsMulticastControlled   lo=%10u hi=%10u\n"
                "          InPktsBroadcastControlled   lo=%10u hi=%10u\n"
                "          InOctetsUncontrolled        lo=%10u hi=%10u\n"
                "          InOctetsControlled          lo=%10u hi=%10u\n",
                Ifc_Stat.InPktsUnicastUncontrolled.Lo,
                Ifc_Stat.InPktsUnicastUncontrolled.Hi,
                Ifc_Stat.InPktsMulticastUncontrolled.Lo,
                Ifc_Stat.InPktsMulticastUncontrolled.Hi,
                Ifc_Stat.InPktsBroadcastUncontrolled.Lo,
                Ifc_Stat.InPktsBroadcastUncontrolled.Hi,
                Ifc_Stat.InPktsUnicastControlled.Lo,
                Ifc_Stat.InPktsUnicastControlled.Hi,
                Ifc_Stat.InPktsMulticastControlled.Lo,
                Ifc_Stat.InPktsMulticastControlled.Hi,
                Ifc_Stat.InPktsBroadcastControlled.Lo,
                Ifc_Stat.InPktsBroadcastControlled.Hi,
                Ifc_Stat.InOctetsUncontrolled.Lo,
                Ifc_Stat.InOctetsUncontrolled.Hi,
                Ifc_Stat.InOctetsControlled.Lo,
                Ifc_Stat.InOctetsControlled.Hi);

            switch (SAParams.ActionType)
            {
                case SECY_SA_ACTION_INGRESS:
                case SECY_SA_ACTION_BYPASS:
                case SECY_SA_ACTION_DROP:
                case SECY_SA_ACTION_CRYPT_AUTH:
                default:
                {
                    SecY_SecY_Stat_I_t SecY_Stat;

                    SecY_Rc = SecY_SecY_Statistics_I_Get(DeviceId,
                                                        vPortId,
                                                        &SecY_Stat,
                                                        false);

                    if (SecY_Rc != SECY_STATUS_OK)
                    {
                        LOG_CRIT("SecY_SecY_Statistics_I_Get returned error %d\n",
                                SecY_Rc);
                        return SecY_Rc;
                    }

                    Log_FormattedMessage(
                        "SecY Stats InPktsTransformErr         lo=%10u hi=%10u\n"
                        "           InPktsControl              lo=%10u hi=%10u\n"
                        "           InPktsUntagged             lo=%10u hi=%10u\n"
                        "           InPktsNoTag                lo=%10u hi=%10u\n"
                        "           InPktsBadTag               lo=%10u hi=%10u\n"
                        "           InPktsNoSCI                lo=%10u hi=%10u\n"
                        "           InPktsUnknownSCI           lo=%10u hi=%10u\n"
                        "           InPktsTaggedCtrl           lo=%10u hi=%10u\n",
                        SecY_Stat.InPktsTransformError.Lo,
                        SecY_Stat.InPktsTransformError.Hi,
                        SecY_Stat.InPktsControl.Lo,
                        SecY_Stat.InPktsControl.Hi,
                        SecY_Stat.InPktsUntagged.Lo,
                        SecY_Stat.InPktsUntagged.Hi,
                        SecY_Stat.InPktsNoTag.Lo,
                        SecY_Stat.InPktsNoTag.Hi,
                        SecY_Stat.InPktsBadTag.Lo,
                        SecY_Stat.InPktsBadTag.Hi,
                        SecY_Stat.InPktsNoSCI.Lo,
                        SecY_Stat.InPktsNoSCI.Hi,
                        SecY_Stat.InPktsUnknownSCI.Lo,
                        SecY_Stat.InPktsUnknownSCI.Hi,
                        SecY_Stat.InPktsTaggedCtrl.Lo,
                        SecY_Stat.InPktsTaggedCtrl.Hi);
                }
                break;
            };

            SCI_Handle = NULL;

            SecY_Rc = SecY_Device_Limits(DeviceId,
                                         NULL,
                                         NULL,
                                         NULL,
                                         &SCCount);
            if (SecY_Rc != SECY_STATUS_OK)
            {
                LOG_CRIT("SecY_Device_Limits returned error %d\n", SecY_Rc);
                return SecY_Rc;
            }

            for (i=0; i<SCCount; i++)
            {
                unsigned int OldSAIndex;
                SecY_RxCAM_Stat_t RxCAM_Stat;
                SecY_Rc = SecY_SCI_Next_Get(DeviceId,
                                            vPortId,
                                            &SCI_Handle,
                                            SCI,
                                            &SCIndex);
                if (SecY_Rc != SECY_STATUS_OK)
                {
                    LOG_CRIT("SecY_SCI_Next_Get returned error %d\n", SecY_Rc);
                    return SecY_Rc;
                }
                if (SCI_Handle == NULL)
                {
                    if (SCIndex == SC_INDEX_INVALID)
                    {
                        Log_FormattedMessage("No SC assigned\n");
                    }
                    break;
                }
                else
                {
                    Log_FormattedMessage("Found SCI=%d\n",SCIndex);
                    Log_HexDump("SCI", 0, SCI, 8);
                    SecY_Rc = SecY_RxCAM_Statistics_Get(DeviceId,
                                                        SCIndex,
                                                        &RxCAM_Stat,
                                                        false);
                    if (SecY_Rc != SECY_STATUS_OK)
                    {
                        LOG_CRIT("SecY_SCI_Next_Get returned error %d\n",
                                 SecY_Rc);
                        return SecY_Rc;
                    }
                    Log_FormattedMessage(
                     "RxCAM Stats CAMHit                    lo=%10u hi=%10u\n",
                     RxCAM_Stat.CAMHit.Lo,
                     RxCAM_Stat.CAMHit.Hi);

                    SecY_Rc = SecY_SA_Active_I_Get(DeviceId,
                                                   vPortId,
                                                   SCI,
                                                   &SAHandle[0]);
                    if (SecY_Rc != SECY_STATUS_OK)
                    {
                        LOG_CRIT("SecY_SCI_Next_Get returned error %d\n",
                                 SecY_Rc);
                        return SecY_Rc;
                    }
                    OldSAIndex = SA_INDEX_INVALID;
                    for (j=0; j<4; j++)
                    {
                        if(SecY_SAHandle_IsSame(&SAHandle[j],
                                                &SecY_SAHandle_NULL))
                        {
                            Log_FormattedMessage("AN=%d no active SA\n", j);
                        }
                        else
                        {
                            unsigned int SAIndex;
                            SecY_Rc = SecY_SAIndex_Get(SAHandle[j],
                                                       &SAIndex,
                                                       &SCIndex);
                            if (SecY_Rc != SECY_STATUS_OK)
                            {
                                LOG_CRIT("SecY_SAIndex_Get returned error %d\n",
                                         SecY_Rc);
                                return SecY_Rc;
                            }
                            Log_FormattedMessage(
                                "AN=%d SA index %d SC index=%d\n",
                                j,
                                SAIndex,
                                SCIndex);
                            if (fIncludeSA && SAIndex != OldSAIndex)
                            {
                                SecY_Rc = SecY_Diag_SA_Dump(DeviceId,
                                                            SAHandle[j],
                                                            false);
                                if (SecY_Rc != SECY_STATUS_OK)
                                {
                                    LOG_CRIT(
                                        "SecY_Diag_SA_Dump returned error %d\n",
                                        SecY_Rc);
                                    return SecY_Rc;
                                }
                            }
                            OldSAIndex = SAIndex;
                        }
                    }
                }
            }
        }
#endif
    }


    return SECY_STATUS_OK;
}


/*----------------------------------------------------------------------------
 * SecY_Diag_SA_Dump
 */
SecY_Status_t
SecY_Diag_SA_Dump(
        const unsigned int DeviceId,
        const SecY_SAHandle_t SAHandle,
        const bool fAllSAs)
{
    SecY_Status_t SecY_Rc;

    if (fAllSAs)
    {
        unsigned int i, SACount;
        SecY_SAHandle_t SAHandle1, SAHandle2;

        SecY_Rc = SecY_Device_Limits(DeviceId,
                                     NULL,
                                     NULL,
                                     &SACount,
                                     NULL);
        if (SecY_Rc != SECY_STATUS_OK)
        {
            LOG_CRIT("SecY_Device_Limits returned error %d\n",SecY_Rc);
            return SecY_Rc;
        }

        SAHandle1 = SecY_SAHandle_NULL;
        for (i=0; i<SACount; i++)
        {
            SecY_Rc = SecY_SA_Next_Get(DeviceId, SAHandle1, &SAHandle2);
            if (SecY_Rc != SECY_STATUS_OK)
            {
                LOG_CRIT("SecY_SA_Next_Get returned error %d\n",SecY_Rc);
                return SecY_Rc;
            }
            if (SecYLib_SAHandle_IsSame(&SAHandle2, &SecY_SAHandle_NULL))
            {
                break;
            }
            else
            {
                SecY_Rc = SecY_Diag_SA_Dump(DeviceId, SAHandle2, false);
                if (SecY_Rc != SECY_STATUS_OK)
                {
                    LOG_CRIT("SecY_Diag_SA_Dump returned error %d\n",SecY_Rc);
                    return SecY_Rc;
                }

                SAHandle1 = SAHandle2;
            }
        }
    }
    else
    {
        SecY_Role_t Role;
        unsigned int SAIndex, SCIndex;
        uint32_t SA_Words[24];

        SecY_Rc = SecY_Device_Role_Get(DeviceId, &Role);

        if (SecY_Rc != SECY_STATUS_OK)
        {
            LOG_CRIT("SecY_Device_Role_Get returned error %d\n",SecY_Rc);
            return SecY_Rc;
        }

        SecY_Rc = SecY_SAIndex_Get(SAHandle, &SAIndex, &SCIndex);
        if (SecY_Rc != SECY_STATUS_OK)
        {
            LOG_CRIT("SecY_SAIndex_Get returned error %d\n",SecY_Rc);
            return SecY_Rc;
        }
        Log_FormattedMessage("SecY_Diag_SA Role=%s SA index %d SC index %d\n",
                             Role==SECY_ROLE_INGRESS?"INGRESS":
                             (Role==SECY_ROLE_EGRESS?"EGRESS":"INGREE-EGRESS"),
                             SAIndex,SCIndex);

        SecY_Rc = SecY_SA_Read(DeviceId,
                               SAHandle,
                               0,
                               24,
                               SA_Words);
        if (SecY_Rc != SECY_STATUS_OK)
        {
            LOG_CRIT("SecY_SA_Read returned error %d\n",SecY_Rc);
            return SecY_Rc;
        }

        Log_FormattedMessage("--Transform record-- \n");
        Log_HexDump32("SA", 0,
                      SA_Words,
                      24);

#ifdef ADAPTER_EIP164_MODE_EGRESS
        if (Role == SECY_ROLE_EGRESS)
        {
            SecY_SA_Stat_E_t SA_Stat;
            SecY_Rc = SecY_SA_Statistics_E_Get(DeviceId,
                                               SAHandle,
                                               &SA_Stat,
                                               false);
            if (SecY_Rc != SECY_STATUS_OK)
            {
                LOG_CRIT("SecY_SA_Statistics_E_Get returned error %d\n",
                         SecY_Rc);
                return SecY_Rc;
            }
            Log_FormattedMessage(
                "SA Stats OutPktsEncryptedProtected     lo=%10u hi=%10u\n"
                "         OutPktsTooLong                lo=%10u hi=%10u\n"
                "         OutPktsSANotInUse             lo=%10u hi=%10u\n"
                "         OutOctetsEncryptedProtected   lo=%10u hi=%10u\n",
                SA_Stat.OutPktsEncryptedProtected.Lo,
                SA_Stat.OutPktsEncryptedProtected.Hi,
                SA_Stat.OutPktsTooLong.Lo,
                SA_Stat.OutPktsTooLong.Hi,
                SA_Stat.OutPktsSANotInUse.Lo,
                SA_Stat.OutPktsSANotInUse.Hi,
                SA_Stat.OutOctetsEncryptedProtected.Lo,
                SA_Stat.OutOctetsEncryptedProtected.Hi);
        }
#endif

#ifdef ADAPTER_EIP164_MODE_INGRESS
        if (Role == SECY_ROLE_INGRESS)
        {
            unsigned int SAIndex;
            unsigned int vPort;
            unsigned int SCIndex;

            /* Get the SA index in the device for this SA handle */
            if (!SecYLib_SAHandleToIndex(SAHandle, &SAIndex, &SCIndex, &vPort))
            {
                LOG_CRIT("%s: Invalid SA handle for EIP-164 device for device %d "
                         "(%s)\n",
                         __func__, DeviceId, ADAPTER_EIP164_DEVICE_NAME(DeviceId));

                return SECY_ERROR_INTERNAL;
            }

            {
                SecY_SA_Stat_I_t SA_Stat;
                SecY_Rc = SecY_SA_Statistics_I_Get(DeviceId,
                                                   SAHandle,
                                                   &SA_Stat,
                                                   false);
                if (SecY_Rc != SECY_STATUS_OK)
                {
                    LOG_CRIT("SecY_SA_Statistics_I_Get returned error %d\n",
                             SecY_Rc);
                    return SecY_Rc;
                }

                Log_FormattedMessage(
                    "SA Stats InPktsUnchecked              lo=%10u hi=%10u\n"
                    "         InPktsDelayed                lo=%10u hi=%10u\n"
                    "         InPktsLate                   lo=%10u hi=%10u\n"
                    "         InPktsOK                     lo=%10u hi=%10u\n"
                    "         InPktsInvalid                lo=%10u hi=%10u\n"
                    "         InPktsNotValid               lo=%10u hi=%10u\n"
                    "         InPktsNotUsingSA             lo=%10u hi=%10u\n"
                    "         InPktsUnusedSA               lo=%10u hi=%10u\n"
                    "         InOctetsDecrypted            lo=%10u hi=%10u\n"
                    "         InOctetsValidated            lo=%10u hi=%10u\n",
                    SA_Stat.InPktsUnchecked.Lo,
                    SA_Stat.InPktsUnchecked.Hi,
                    SA_Stat.InPktsDelayed.Lo,
                    SA_Stat.InPktsDelayed.Hi,
                    SA_Stat.InPktsLate.Lo,
                    SA_Stat.InPktsLate.Hi,
                    SA_Stat.InPktsOK.Lo,
                    SA_Stat.InPktsOK.Hi,
                    SA_Stat.InPktsInvalid.Lo,
                    SA_Stat.InPktsInvalid.Hi,
                    SA_Stat.InPktsNotValid.Lo,
                    SA_Stat.InPktsNotValid.Hi,
                    SA_Stat.InPktsNotUsingSA.Lo,
                    SA_Stat.InPktsNotUsingSA.Hi,
                    SA_Stat.InPktsUnusedSA.Lo,
                    SA_Stat.InPktsUnusedSA.Hi,
                    SA_Stat.InOctetsDecrypted.Lo,
                    SA_Stat.InOctetsDecrypted.Hi,
                    SA_Stat.InOctetsValidated.Lo,
                    SA_Stat.InOctetsValidated.Hi);
            }
        }
#endif
    }

    return SECY_STATUS_OK;
}


/* end of file adapter_secy_ext.c */
