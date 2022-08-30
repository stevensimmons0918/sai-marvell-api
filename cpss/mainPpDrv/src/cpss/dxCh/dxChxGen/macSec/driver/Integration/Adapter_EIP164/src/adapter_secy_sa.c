/* adapter_secy_sa.c
 *
 * SecY API for SA Handling.
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

/* Adapter internal SecY API. */
#include <Integration/Adapter_EIP164/incl/adapter_secy_support.h>  /* Adapter internal SecY API */
#ifdef ADAPTER_EIP164_WARMBOOT_ENABLE
#include <Integration/Adapter_EIP164/incl/adapter_secy_warmboot.h>
#endif

/* Logging API */
#include <Kit/Log/incl/log.h>                /* LOG_* */

/* Driver Framework C Library Abstraction API */
#include <Kit/DriverFramework/incl/clib.h>               /* ZEROINIT(),cpssOsMemCpy(),cpssOsMemCmp() */

/* SA Builder API */
#include <Kit/SABuilder_MACsec/incl/sa_builder_macsec.h>

/* EIP-164 Driver Library SecY API */
#include <Kit/EIP164/incl/eip164_types.h>
#include <Kit/EIP164/incl/eip164_secy.h>

/* Adapter Sleep API */
#include <Integration/Adapter_EIP164/incl/adapter_sleep.h>      /* Adapter_SleepMS() */

/* List API */
#include <Kit/List/incl/list.h>

/*----------------------------------------------------------------------------
 * Definitions and macros
 */


/*----------------------------------------------------------------------------
 * SecYLib_SA_Flow_Copy
 */
static void
SecYLib_SA_Flow_Copy(
        const SecY_SA_t * const SA_p,
        EIP164_SecY_SA_t * const DeviceSA_p)
{
    switch(SA_p->DestPort)
    {
    default:
    case SECY_PORT_COMMON:
        DeviceSA_p->DestPort = EIP164_PORT_COMMON;
        break;

    case SECY_PORT_CONTROLLED:
        DeviceSA_p->DestPort = EIP164_PORT_CONTROLLED;
        break;

    case SECY_PORT_UNCONTROLLED:
        DeviceSA_p->DestPort = EIP164_PORT_UNCONTROLLED;
        break;
    }

    /* Action type */
    if (SA_p->ActionType == SECY_SA_ACTION_EGRESS)
    {
        DeviceSA_p->ActionType = EIP164_SECY_SA_ACTION_EGRESS;

        DeviceSA_p->Params.Egress.fConfProtect =
            SA_p->Params.Egress.fConfProtect;
        DeviceSA_p->Params.Egress.fProtectFrames =
            SA_p->Params.Egress.fProtectFrames;
        DeviceSA_p->Params.Egress.fUseES = SA_p->Params.Egress.fUseES;
        DeviceSA_p->Params.Egress.fUseSCB = SA_p->Params.Egress.fUseSCB;
        DeviceSA_p->Params.Egress.fIncludeSCI =
            SA_p->Params.Egress.fIncludeSCI;
        DeviceSA_p->Params.Egress.ConfidentialityOffset =
            SA_p->Params.Egress.ConfidentialityOffset;
        DeviceSA_p->Params.Egress.fAllowDataPkts =
            SA_p->Params.Egress.fAllowDataPkts;
        DeviceSA_p->Params.Egress.PreSecTagAuthStart =
            SA_p->Params.Egress.PreSecTagAuthStart;
        DeviceSA_p->Params.Egress.PreSecTagAuthLength =
            SA_p->Params.Egress.PreSecTagAuthLength;
    }
    else if (SA_p->ActionType == SECY_SA_ACTION_INGRESS)
    {
        DeviceSA_p->ActionType = EIP164_SECY_SA_ACTION_INGRESS;

        DeviceSA_p->Params.Ingress.fRetainSecTAG =
            SA_p->Params.Ingress.fRetainSecTAG;
        DeviceSA_p->Params.Ingress.fRetainICV =
            SA_p->Params.Ingress.fRetainICV;
        DeviceSA_p->Params.Ingress.fReplayProtect =
            SA_p->Params.Ingress.fReplayProtect;
        DeviceSA_p->Params.Ingress.ValidateFramesTagged =
            SA_p->Params.Ingress.ValidateFramesTagged;
        DeviceSA_p->Params.Ingress.ConfidentialityOffset =
            SA_p->Params.Ingress.ConfidentialityOffset;
        DeviceSA_p->Params.Ingress.fAllowTagged =
            SA_p->Params.Ingress.fAllowTagged;
        DeviceSA_p->Params.Ingress.fAllowUntagged =
            SA_p->Params.Ingress.fAllowUntagged;
        DeviceSA_p->Params.Ingress.fValidateUntagged =
            SA_p->Params.Ingress.fValidateUntagged;
        DeviceSA_p->Params.Ingress.PreSecTagAuthStart =
            SA_p->Params.Ingress.PreSecTagAuthStart;
        DeviceSA_p->Params.Ingress.PreSecTagAuthLength =
            SA_p->Params.Ingress.PreSecTagAuthLength;
    }
    else if (SA_p->ActionType == SECY_SA_ACTION_BYPASS)
    {
        DeviceSA_p->ActionType = EIP164_SECY_SA_ACTION_BYPASS;
    }
    else if (SA_p->ActionType == SECY_SA_ACTION_DROP)
    {
        DeviceSA_p->ActionType = EIP164_SECY_SA_ACTION_DROP;
    }
    else
    {
        DeviceSA_p->ActionType = EIP164_SECY_SA_ACTION_CRYPT_AUTH;

        DeviceSA_p->Params.CryptAuth.fZeroLengthMessage =
            SA_p->Params.CryptAuth.fZeroLengthMessage;
        DeviceSA_p->Params.CryptAuth.ConfidentialityOffset =
            SA_p->Params.CryptAuth.ConfidentialityOffset;
        DeviceSA_p->Params.CryptAuth.IVMode =
            SA_p->Params.CryptAuth.IVMode;
        DeviceSA_p->Params.CryptAuth.fICVAppend =
            SA_p->Params.CryptAuth.fICVAppend;
        DeviceSA_p->Params.CryptAuth.fICVVerify =
            SA_p->Params.CryptAuth.fICVVerify;
        DeviceSA_p->Params.CryptAuth.fConfProtect =
            SA_p->Params.CryptAuth.fConfProtect;
    }

    /* Drop type */
    if (SA_p->DropType == SECY_SA_DROP_CRC_ERROR)
    {
        DeviceSA_p->DropType = EIP164_SECY_SA_DROP_CRC_ERROR;
    }
    else if (SA_p->DropType == SECY_SA_DROP_PKT_ERROR)
    {
        DeviceSA_p->DropType = EIP164_SECY_SA_DROP_PKT_ERROR;
    }
    else
    {
        DeviceSA_p->DropType = EIP164_SECY_SA_DROP_INTERNAL;
    }

    DeviceSA_p->fDropNonReserved = ADAPTER_EIP164_SEC_DROP_NON_RESERVED;
}


/*----------------------------------------------------------------------------
 * SecYLib_SCI_Find
 */
static bool
SecYLib_SCI_Find(
        const unsigned int DeviceId,
        const uint8_t * const SCI,
        const unsigned int vPort,
        unsigned int * const SCIndex_p)
{
    List_Status_t List_Rc;
    void *SCList = PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->vPortDscr_p[vPort].SCList;
    const List_Element_t *cur;

    List_Rc = List_GetHead(0, SCList, &cur);
    if (List_Rc != LIST_STATUS_OK)
    {
        LOG_CRIT("%s: Failed, no SCI list for device %d\n",
                 __func__, DeviceId);
        return false;
    }

    while (cur != NULL)
    {
        SecY_SC_Descriptor_t *SCDscr_p = cur->DataObject_p;
        if (cpssOsMemCmp(SCDscr_p->SCI, SCI, 8) == 0)
        {
            *SCIndex_p = SCDscr_p->SCIndex;;
            return true;
        }
        cur = List_GetNextElement(cur);
    }

    return false;
}


/*----------------------------------------------------------------------------
 * SecYLib_SA_Update_Control_Word_Update
 */
static SecY_Status_t
SecYLib_SA_Update_Control_Word_Update(
        const unsigned int DeviceId,
        const unsigned int SAIndex,
        const SABuilder_UpdCtrl_Params_t * const UpdateParams)
{
    EIP164_Error_t Rc;
    SecY_Status_t SecY_Rc;
    SABuilder_Status_t SAB_Rc;
    unsigned int Offset;
    uint32_t SAUpdateCtrlWord;
    uint32_t Transform_0;

    SecY_Rc = SecYLib_SA_Read(DeviceId, SAIndex, 0, 1, &Transform_0);
    if (SecY_Rc != SECY_STATUS_OK)
    {
        return SecY_Rc;
    }

    SAB_Rc = SABuilder_UpdateCtrlOffset_Get(Transform_0, &Offset);
    if (SAB_Rc != SAB_STATUS_OK)
    {
        return SECY_ERROR_INTERNAL;
    }

    if (Offset > 0)
    {
        SAB_Rc = SABuilder_UpdateCtrl_Update(UpdateParams, &SAUpdateCtrlWord);
        if (SAB_Rc != SAB_STATUS_OK)
        {
            return SECY_ERROR_INTERNAL;
        }

        Rc = EIP164_SecY_SA_Update_Control_Word_Update(&PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->IOArea,
                                                       SAIndex,
                                                       Offset,
                                                       SAUpdateCtrlWord);
        if (Rc != EIP164_NO_ERROR)
        {
            return SECY_DLIB_ERROR(Rc);
        }
    }
    return SECY_STATUS_OK;
}


/*----------------------------------------------------------------------------
 * SecYLib_SA_MTU_Update
 */
static SecY_Status_t
SecYLib_SA_MTU_Update(
        const unsigned int DeviceId,
        const unsigned int SAIndex,
        const uint32_t MTU)
{
    EIP164_Error_t Rc;
    SecY_Status_t SecY_Rc;
    SABuilder_Status_t SAB_Rc;
    unsigned int Offset;
    uint32_t Transform_0;

    SecY_Rc = SecYLib_SA_Read(DeviceId, SAIndex, 0, 1, &Transform_0);
    if (SecY_Rc != SECY_STATUS_OK)
    {
        return SecY_Rc;
    }

    SAB_Rc = SABuilder_MTUOffset_Get(Transform_0, &Offset);
    if (SAB_Rc != SAB_STATUS_OK)
    {
        return SECY_ERROR_INTERNAL;
    }

    Rc = EIP164_SecY_SA_Update_Control_Word_Update(&PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->IOArea,
                                                   SAIndex,
                                                   Offset,
                                                   MTU);
    if (Rc != EIP164_NO_ERROR)
    {
        return SECY_DLIB_ERROR(Rc);
    }
    return SECY_STATUS_OK;
}


/*----------------------------------------------------------------------------
 * SecY_SA_Add
 */
SecY_Status_t
SecY_SA_Add(
        const unsigned int DeviceId,
        const unsigned int vPort,
        SecY_SAHandle_t * const SAHandle_p,
        const SecY_SA_t * const SA_p)
{
    EIP164_Error_t Rc;
    EIP164_SecY_SA_t DeviceSA;
    unsigned int SAIndex;
    SABuilder_UpdCtrl_Params_t SAUpdateCtrWord;
    SecY_Status_t SecY_Rc;
    bool fFirstSA;
    unsigned int saCount;
    /* Initialse SA data structure */
    ZEROINIT(DeviceSA);

#ifdef ADAPTER_EIP164_STRICT_ARGS
    if (DeviceId >= ADAPTER_EIP164_MAX_NOF_DEVICES)
        return SECY_ERROR_BAD_PARAMETER;

    if (SAHandle_p == NULL)
        return SECY_ERROR_BAD_PARAMETER;

    if (SA_p == NULL)
        return SECY_ERROR_BAD_PARAMETER;
#endif

    if (SA_p->ActionType == SECY_SA_ACTION_INGRESS &&
        SA_p->Params.Ingress.AN > 3)
    {
        /* Error out early on invalid ingress AN. */
        return SECY_ERROR_BAD_PARAMETER;
    }

    SecYLib_Device_Lock(DeviceId);

    if (!PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device_StaticFields)[DeviceId].fInitialized)
    {
        SecYLib_Device_Unlock(DeviceId);
        return SECY_ERROR_INTERNAL;
    }

#ifdef ADAPTER_EIP164_STRICT_ARGS
    if (vPort >= PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->vPortCount)
    {
        SecYLib_Device_Unlock(DeviceId);
        return SECY_ERROR_BAD_PARAMETER;
    }
#endif

    SAHandle_p->p = NULL;

    fFirstSA = (PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->vPortDscr_p[vPort].SACount == 0);

    if (fFirstSA)
    {
        LOG_INFO("\n\t EIP164_SecY_SAMFlow_Clear \n");

        Rc = EIP164_SecY_SAMFlow_Clear(&PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->IOArea, vPort);
        if (Rc != EIP164_NO_ERROR)
        {
            SecYLib_Device_Unlock(DeviceId);
            return SECY_DLIB_ERROR(Rc);
        }
    }

    /* Obtain a free SA for this SecY device */
    {
        List_Element_t * Element_p;
        SecY_SA_Descriptor_t * SADscr_p;
        List_Status_t List_Rc = List_RemoveFromTail(0,
                                                    PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->SAFL_p,
                                                    &Element_p);
        if (List_Rc != LIST_STATUS_OK || Element_p->DataObject_p == NULL)
        {
            LOG_CRIT("%s: Failed to obtain a free SA for EIP-164 device for "
                     "device %d, error %d\n",
                     __func__,
                     DeviceId,
                     List_Rc);
            SecYLib_Device_Unlock(DeviceId);
            return SECY_ERROR_INTERNAL;
        }

        /* Convert SA descriptor to SA index and to SA handle */
        SADscr_p = (SecY_SA_Descriptor_t*)Element_p->DataObject_p;
        SAIndex = SADscr_p - PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->SADscr_p;
        if (SAIndex >= PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->SACount)
        {
            LOG_CRIT("%s: Illegal SA Index for"
                     "device %d, index %u\n",
                     __func__,
                     DeviceId,
                     SAIndex);
            SecYLib_Device_Unlock(DeviceId);
            return SECY_ERROR_INTERNAL;
        }

#ifdef ADAPTER_EIP164_WARMBOOT_ENABLE
        SecYLib_WarmBoot_SA_Set(DeviceId, SAIndex);
#endif

        SADscr_p->u.InUse.SAIndex = SAIndex;
        /* SA descriptor magic umber */
        SADscr_p->Magic = ADAPTER_EIP164_SA_DSCR_MAGIC;

        SADscr_p->u.InUse.SAMFlowCtrlIndex = vPort;

        switch (SA_p->ActionType)
        {
        case SECY_SA_ACTION_EGRESS:
            SADscr_p->u.InUse.MapType = SECY_SA_MAP_EGRESS;
            break;
        case SECY_SA_ACTION_INGRESS:
            SADscr_p->u.InUse.MapType = SECY_SA_MAP_INGRESS;
            break;
        case SECY_SA_ACTION_CRYPT_AUTH:
            if (PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->Role == SECY_ROLE_EGRESS)
                SADscr_p->u.InUse.MapType = SECY_SA_MAP_EGRESS_CRYPT_AUTH;
            else
                SADscr_p->u.InUse.MapType = SECY_SA_MAP_INGRESS_CRYPT_AUTH;
            break;
        default:
            SADscr_p->u.InUse.MapType = SECY_SA_MAP_DETACHED;
        }

        SADscr_p->u.InUse.AN = 0;
        SADscr_p->u.InUse.SCIndex = 0xfffffffe;
        SAHandle_p->p = SADscr_p;

        LOG_INFO("\n\t EIP164_SecY_SA_Write \n");

        if (SA_p->SA_WordCount > 0)
        {
            /* Write transform record to device. */
            Rc = EIP164_SecY_SA_Write(&PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->IOArea,
                                      SAIndex,
                                      SA_p->TransformRecord_p,
                                      SA_p->SA_WordCount);
            if (Rc != EIP164_NO_ERROR)
            {
                SecYLib_Device_Unlock(DeviceId);
                return SECY_DLIB_ERROR(Rc);
            }
        }

        if ((SADscr_p->u.InUse.MapType == SECY_SA_MAP_INGRESS) ||
            (SADscr_p->u.InUse.MapType == SECY_SA_MAP_INGRESS_CRYPT_AUTH))
        {
            bool found;
            uint8_t *SCI;
            uint8_t SCI_None[] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
            uint8_t SCI_AllZero[] = { 0, 0, 0, 0, 0, 0, 0, 0 };

            if (SADscr_p->u.InUse.MapType == SECY_SA_MAP_INGRESS)
            {
                if (SA_p->ActionType == SECY_SA_ACTION_INGRESS)
                {
                    SCI = SA_p->Params.Ingress.SCI_p;
                    SADscr_p->u.InUse.AN = SA_p->Params.Ingress.AN;
                }
                else
                {
                    SCI = SCI_AllZero;
                    SADscr_p->u.InUse.AN = 0;
                }
            }
            else
            {
                SCI = SCI_None;
                SADscr_p->u.InUse.AN = 0;
            }

            /* Try to find an existing SC entry with same SCI and vPort. */
            found = SecYLib_SCI_Find(DeviceId,
                                     SCI,
                                     vPort,
                                     &SADscr_p->u.InUse.SCIndex);
            if (!found)
            { /* Try to allocate new SC entry. */
                SecY_SC_Descriptor_t *SCDscr_p;
                List_Rc = List_RemoveFromTail(0,
                                              PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->SCFL_p,
                                              &Element_p);
                if (List_Rc != LIST_STATUS_OK || Element_p->DataObject_p == NULL)
                {
                    LOG_CRIT("%s: Failed to obtain a free SC for EIP-164 device "
                             "for device %d, error %d\n",
                             __func__,
                             DeviceId,
                             List_Rc);
                    SecYLib_Device_Unlock(DeviceId);
                    return SECY_ERROR_INTERNAL;
                }

                /* Fill in the new SC entry. */
                SCDscr_p = (SecY_SC_Descriptor_t*)Element_p->DataObject_p;
                SADscr_p->u.InUse.SCIndex = SCDscr_p->SCIndex;
                cpssOsMemCpy(SCDscr_p->SCI, SCI, 8);
                SCDscr_p->vPort = vPort;

                SCDscr_p->MapType = SADscr_p->u.InUse.MapType;
                SCDscr_p->SAHandle[0] = SecY_SAHandle_NULL;
                SCDscr_p->SAHandle[1] = SecY_SAHandle_NULL;
                SCDscr_p->SAHandle[2] = SecY_SAHandle_NULL;
                SCDscr_p->SAHandle[3] = SecY_SAHandle_NULL;

                /* Add SC descriptor to vPort specific lookup list. */
                List_Rc = List_AddToHead(0,
                                         PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->vPortDscr_p[vPort].SCList,
                                         Element_p);
                if (List_Rc != LIST_STATUS_OK)
                {
                    LOG_CRIT("%s: Failed to add SC elemet to lookup list for "
                             "EIP-164 device for device %d, error %d\n",
                             __func__,
                             DeviceId,
                             List_Rc);
                    SecYLib_Device_Unlock(DeviceId);
                    return SECY_ERROR_INTERNAL;
                }

                {
                    unsigned int LoopCounter = ADAPTER_EIP164_MAX_NOF_SYNC_RETRY_COUNT;

                    for(;;)
                    {
                        LoopCounter--;

                        LOG_INFO("\n\t EIP164_SecY_RxCAM_Add \n");

                        /* Add entry to RX CAM. */
                        Rc = EIP164_SecY_RxCAM_Add(&PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->IOArea,
                                                   SCDscr_p->SCIndex,
                                                   (SCI[0] <<  0) |
                                                   (SCI[1] <<  8) |
                                                   (SCI[2] << 16) |
                                                   (SCI[3] << 24) ,
                                                   (SCI[4] <<  0) |
                                                   (SCI[5] <<  8) |
                                                   (SCI[6] << 16) |
                                                   (SCI[7] << 24),
                                                   vPort);
                        if (Rc == EIP164_NO_ERROR)
                        {
                            break;
                        }
                        else if (Rc != EIP164_BUSY_RETRY_LATER ||
                                 LoopCounter == 0)
                        {
                            SecYLib_Device_Unlock(DeviceId);
                            return SECY_DLIB_ERROR(Rc);
                        }
                        Adapter_SleepMS(ADAPTER_EIP164_SYNC_RETRY_TIMEOUT_MS);
                    }
                }

#ifdef ADAPTER_EIP164_MODE_INGRESS
                Rc = EIP164_SecY_RxCAM_Stat_Clear(&PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->IOArea,
                                                  SCDscr_p->SCIndex);
                if (Rc != EIP164_NO_ERROR)
                {
                    SecYLib_Device_Unlock(DeviceId);
                    return SECY_DLIB_ERROR(Rc);
                }
#endif
            }

            if (!SecY_SAHandle_IsSame(&PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->SCDscr_p[SADscr_p->u.InUse.SCIndex].SAHandle[SADscr_p->u.InUse.AN],
                                      &SecY_SAHandle_NULL))
            {
                /* There was already an ingress SA for this vPOrt. */
                /* Mark it as unused and subtract one from the SA count. */
                SecY_SA_Descriptor_t * SADscr2_p;

                SADscr2_p = (SecY_SA_Descriptor_t *)PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->SCDscr_p[SADscr_p->u.InUse.SCIndex].SAHandle[SADscr_p->u.InUse.AN].p;
                SADscr2_p->u.InUse.MapType = SECY_SA_MAP_DETACHED;
                saCount = PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->vPortDscr_p[vPort].SACount;
                PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_SET(SecY_Device[DeviceId]->vPortDscr_p[vPort].SACount,--saCount);
            }
            PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_SET(SecY_Device[DeviceId]->SCDscr_p[SADscr_p->u.InUse.SCIndex].SAHandle[SADscr_p->u.InUse.AN],
                                                  *SAHandle_p);

            if (SADscr_p->u.InUse.MapType == SECY_SA_MAP_INGRESS)
            {
                /* Enable the correct SC->SA mapping */

                LOG_INFO("\n\t EIP164_SecY_SC_SA_Map_I_Update \n");

                Rc = EIP164_SecY_SC_SA_Map_I_Update(&PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->IOArea,
                                                    SADscr_p->u.InUse.SCIndex,
                                                    SADscr_p->u.InUse.AN,
                                                    SAIndex,
                                                    true);
                if (Rc != EIP164_NO_ERROR)
                {
                    SecYLib_Device_Unlock(DeviceId);
                    return SECY_DLIB_ERROR(Rc);
                }

                SAUpdateCtrWord.SCIndex = SADscr_p->u.InUse.SCIndex;
                SAUpdateCtrWord.AN = SADscr_p->u.InUse.AN;
                SAUpdateCtrWord.SAIndex = SAIndex;
                SAUpdateCtrWord.fSAIndexValid = false;
                SAUpdateCtrWord.fUpdateEnable = true;
                SAUpdateCtrWord.fUpdateTime = false;
                SAUpdateCtrWord.fExpiredIRQ = true;

                SecY_Rc = SecYLib_SA_Update_Control_Word_Update(DeviceId,
                                                                SAIndex,
                                                                &SAUpdateCtrWord);
                if (SecY_Rc != SECY_STATUS_OK)
                {
                    LOG_CRIT("%s: Failed to update SA update control word for device %d "
                             "(%s), error %d\n",
                             __func__, DeviceId, ADAPTER_EIP164_DEVICE_NAME(DeviceId), SecY_Rc);
                    SecYLib_Device_Unlock(DeviceId);
                    return SecY_Rc;
                }
            }
            else
            {
                unsigned int i;
                /* Crypt_Auth, map all four SA slots in SC entry. */

                LOG_INFO("\n\t EIP164_SecY_SC_SA_Map_I_Update \n");

                for (i=0; i<4; i++)
                {
                    LOG_INFO("\n\t EIP164_SecY_SC_SA_Map_I_Update \n");
                    Rc = EIP164_SecY_SC_SA_Map_I_Update(&PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->IOArea,
                                                        SADscr_p->u.InUse.SCIndex,
                                                        i,
                                                        SAIndex,
                                                        true);
                    if (Rc != EIP164_NO_ERROR)
                    {
                        SecYLib_Device_Unlock(DeviceId);
                        return SECY_DLIB_ERROR(Rc);
                    }
                }
            }
        }
        else
        {
            SADscr_p->u.InUse.SCIndex = vPort;

            if ((PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->SCDscr_p[SADscr_p->u.InUse.SCIndex].MapType !=
                 SECY_SA_MAP_DETACHED) &&
                !SecY_SAHandle_IsSame(&PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->SCDscr_p[SADscr_p->u.InUse.SCIndex].SAHandle[0],
                                      &SecY_SAHandle_NULL))
            {
                /* There was already an egress SA for this vPort. */
                /* Mark it as unused and subtract one from the SA count. */
                SecY_SA_Descriptor_t * SADscr2_p;

                SADscr2_p = (SecY_SA_Descriptor_t*)PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->SCDscr_p[SADscr_p->u.InUse.SCIndex].SAHandle[0].p;
                SADscr2_p->u.InUse.MapType = SECY_SA_MAP_DETACHED;
                saCount = PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->vPortDscr_p[vPort].SACount;
                PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_SET(SecY_Device[DeviceId]->vPortDscr_p[vPort].SACount,--saCount);
            }
            PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_SET(SecY_Device[DeviceId]->SCDscr_p[SADscr_p->u.InUse.SCIndex].SAHandle[0],*SAHandle_p);
            PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_SET(SecY_Device[DeviceId]->SCDscr_p[SADscr_p->u.InUse.SCIndex].MapType,SADscr_p->u.InUse.MapType);

            LOG_INFO("\n\t EIP164_SecY_SC_SA_Map_E_Update \n");

            if (PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->SCDscr_p[SADscr_p->u.InUse.SCIndex].MapType ==
                SECY_SA_MAP_EGRESS)
            {
                if (PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->vPortDscr_p[vPort].MTU != 0)
                {
                    /* Set MTU value in new SA. */
                    SecY_Rc = SecYLib_SA_MTU_Update(DeviceId,
                                          SAIndex,
                                          PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->vPortDscr_p[vPort].MTU);
                    if (SecY_Rc != SECY_STATUS_OK)
                    {
                        LOG_CRIT("%s: Failed to update MTU word for device %d "
                                 "(%s), error %d\n",
                                 __func__, DeviceId, ADAPTER_EIP164_DEVICE_NAME(DeviceId), SecY_Rc);
                        SecYLib_Device_Unlock(DeviceId);
                        return SecY_Rc;
                    }
                }

                SAUpdateCtrWord.SCIndex = SADscr_p->u.InUse.SCIndex;
                SAUpdateCtrWord.AN = 0;
                SAUpdateCtrWord.SAIndex = 0;
                SAUpdateCtrWord.fSAIndexValid = false;
                SAUpdateCtrWord.fUpdateEnable = true;
                SAUpdateCtrWord.fExpiredIRQ = true;
                SAUpdateCtrWord.fUpdateTime = false;
                SecY_Rc = SecYLib_SA_Update_Control_Word_Update(DeviceId,
                                                                SAIndex,
                                                                &SAUpdateCtrWord);
                if (SecY_Rc != SECY_STATUS_OK)
                {
                    LOG_CRIT("%s: Failed to update SA update control word for device %d "
                             "(%s), error %d\n",
                             __func__, DeviceId, ADAPTER_EIP164_DEVICE_NAME(DeviceId), SecY_Rc);
                    SecYLib_Device_Unlock(DeviceId);
                    return SecY_Rc;
                }
            }
            /* Enable the correct SC->SA mapping */
            if (PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->SCDscr_p[SADscr_p->u.InUse.SCIndex].MapType !=
                SECY_SA_MAP_DETACHED)
            {
                Rc = EIP164_SecY_SC_SA_Map_E_Update(&PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->IOArea,
                                                    SADscr_p->u.InUse.SCIndex,
                                                    SAIndex,
                                                    true,
                                                    false);
                if (Rc != EIP164_NO_ERROR)
                {
                    SecYLib_Device_Unlock(DeviceId);
                    return SECY_DLIB_ERROR(Rc);
                }
            }
        }
        if(SADscr_p->u.InUse.MapType != SECY_SA_MAP_DETACHED)
        {
            saCount = PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->vPortDscr_p[vPort].SACount;
            PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_SET(SecY_Device[DeviceId]->vPortDscr_p[vPort].SACount,++saCount);
        }
    }

    SecYLib_SA_Flow_Copy(SA_p, &DeviceSA);

    if (fFirstSA)
    {
        LOG_INFO("\n\t EIP164_SecY_SAMFlow_Write \n");

        Rc = EIP164_SecY_SAMFlow_Write(&PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->IOArea,
                                       vPort,
                                       &DeviceSA
                                       );

        if (Rc != EIP164_NO_ERROR)
        {
            LOG_CRIT("%s: Failed to add SA for EIP-164 device for device %d "
                     "(%s), error %d\n",
                     __func__, DeviceId, ADAPTER_EIP164_DEVICE_NAME(DeviceId), Rc);
            SecYLib_Device_Unlock(DeviceId);
            return SECY_DLIB_ERROR(Rc);
        }
    }

#ifdef ADAPTER_EIP164_MODE_EGRESS
    if (SA_p->ActionType == SECY_SA_ACTION_EGRESS)
    {
        LOG_INFO("\n\t EIP164_SecY_SA_Stat_E_Clear \n");

        /* Reset/read egress SA statistics counters */
        Rc = EIP164_SecY_SA_Stat_E_Clear(&PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->IOArea,
                                         SAIndex);
        if (Rc != EIP164_NO_ERROR)
        {
            LOG_CRIT("%s: Failed to reset egress SA statistics for EIP-164 "
                     "device for device %d (%s), error %d\n",
                     __func__, DeviceId, ADAPTER_EIP164_DEVICE_NAME(DeviceId), Rc);
            SecYLib_Device_Unlock(DeviceId);
            return SECY_DLIB_ERROR(Rc);
        }
    }
#endif

#ifdef ADAPTER_EIP164_MODE_INGRESS
    if (SA_p->ActionType == SECY_SA_ACTION_INGRESS)
    {
        LOG_INFO("\n\t EIP164_SecY_SA_Stat_I_Clear \n");

        /* Reset/read ingress SA statistics counters */
        Rc = EIP164_SecY_SA_Stat_I_Clear(&PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->IOArea,
                                         SAIndex);
        if (Rc != EIP164_NO_ERROR)
        {
            LOG_CRIT("%s: Failed to reset ingress SA statistics for EIP-164 "
                     "device for device %d (%s), error %d\n",
                     __func__, DeviceId, ADAPTER_EIP164_DEVICE_NAME(DeviceId), Rc);
            SecYLib_Device_Unlock(DeviceId);
            return SECY_DLIB_ERROR(Rc);
        }
    }
#endif

    SecYLib_Device_Unlock(DeviceId);

    LOG_INFO("\n %s done \n", __func__);

    return SECY_STATUS_OK;
}


/*----------------------------------------------------------------------------
 * SecY_SA_Update
 */
SecY_Status_t
SecY_SA_Update(
        const unsigned int DeviceId,
        const SecY_SAHandle_t SAHandle,
        const SecY_SA_t * const SA_p)
{
    EIP164_Error_t Rc;
    EIP164_SecY_SA_t DeviceSA;
    unsigned int SAIndex;
    unsigned int SCIndex;
    unsigned int SAMFlowCtrlIndex;

    LOG_INFO("\n %s \n", __func__);

#ifdef ADAPTER_EIP164_STRICT_ARGS
    if (DeviceId >= ADAPTER_EIP164_MAX_NOF_DEVICES)
        return SECY_ERROR_BAD_PARAMETER;

    if (SA_p == NULL)
        return SECY_ERROR_BAD_PARAMETER;

    if (SecY_SAHandle_IsSame(&SAHandle, &SecY_SAHandle_NULL))
        return SECY_ERROR_BAD_PARAMETER;
#endif

    SecYLib_Device_Lock(DeviceId);

    if (!PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device_StaticFields)[DeviceId].fInitialized)
    {
        SecYLib_Device_Unlock(DeviceId);
        return SECY_ERROR_INTERNAL;
    }

    /* Get the SA index in the device for this SA handle */
    if (!SecYLib_SAHandleToIndex(SAHandle, &SAIndex, &SCIndex, &SAMFlowCtrlIndex))
    {
        LOG_CRIT("%s: Invalid SA handle for EIP-164 device for device %d "
                 "(%s)\n",
                 __func__, DeviceId, ADAPTER_EIP164_DEVICE_NAME(DeviceId));
        SecYLib_Device_Unlock(DeviceId);
        return SECY_ERROR_INTERNAL;
    }

    /* Update fSAInUse flag in SA mapping register. */
    {
        SecY_SA_Descriptor_t * SADscr_p = (SecY_SA_Descriptor_t*)SAHandle.p;

        if (SADscr_p->u.InUse.MapType == SECY_SA_MAP_INGRESS)
        {
            LOG_INFO("\n\t EIP164_SecY_SC_SA_Map_I_Update \n");

            Rc = EIP164_SecY_SC_SA_Map_I_Update(&PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->IOArea,
                                           SCIndex,
                                           SADscr_p->u.InUse.AN,
                                           SAIndex,
                                           SA_p->Params.Ingress.fSAInUse);
            if (Rc != EIP164_NO_ERROR)
            {
                SecYLib_Device_Unlock(DeviceId);
                return SECY_DLIB_ERROR(Rc);
            }
        }
        else if (SADscr_p->u.InUse.MapType == SECY_SA_MAP_EGRESS)
        {
            LOG_INFO("\n\t EIP164_SecY_SC_SA_Map_I_Update \n");

            Rc = EIP164_SecY_SC_SA_Map_E_Update(&PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->IOArea,
                                           SCIndex,
                                           SAIndex,
                                           SA_p->Params.Egress.fSAInUse,
                                           false);
            if (Rc != EIP164_NO_ERROR)
            {
                SecYLib_Device_Unlock(DeviceId);
                return SECY_DLIB_ERROR(Rc);
            }

        }
    }

    ZEROINIT(DeviceSA);
    SecYLib_SA_Flow_Copy(SA_p, &DeviceSA);

    LOG_INFO("\n\t EIP164_SecY_SAMFlow_Write \n");

    /* Update the SA parameters in the SAMFlowCtrl registers. */
    Rc = EIP164_SecY_SAMFlow_Write(&PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->IOArea,
                                   SAMFlowCtrlIndex,
                                   &DeviceSA
                                   );

    if (Rc != EIP164_NO_ERROR)
    {
        LOG_CRIT("%s: Failed to update SA from EIP-164 device for device %d "
                 "(%s), error %d\n",
                 __func__, DeviceId, ADAPTER_EIP164_DEVICE_NAME(DeviceId), Rc);
        SecYLib_Device_Unlock(DeviceId);
        return SECY_DLIB_ERROR(Rc);
    }

    SecYLib_Device_Unlock(DeviceId);

    LOG_INFO("\n %s done \n", __func__);

    return SECY_STATUS_OK;
}


/*----------------------------------------------------------------------------
 * SecY_SA_NextPN_Update
 */
SecY_Status_t
SecY_SA_NextPN_Update(
        const unsigned int DeviceId,
        const SecY_SAHandle_t SAHandle,
        const uint32_t NextPN_Lo,
        const uint32_t NextPN_Hi,
        bool * const fNextPNWritten_p)
{
    unsigned int SAIndex;
    EIP164_Error_t Rc;
    bool fNextPNWritten = false;
    bool fBusy = false;
    bool fExtPN = true;
    unsigned int LoopCounter = ADAPTER_EIP164_MAX_NOF_SYNC_RETRY_COUNT;

    LOG_INFO("\n %s \n", __func__);

#ifdef ADAPTER_EIP164_STRICT_ARGS
    if (DeviceId >= ADAPTER_EIP164_MAX_NOF_DEVICES)
        return SECY_ERROR_BAD_PARAMETER;

    if (SecY_SAHandle_IsSame(&SAHandle, &SecY_SAHandle_NULL))
        return SECY_ERROR_BAD_PARAMETER;
#endif

    /* Get the SA index in the device for this SA handle */
    if (!SecYLib_SAHandleToIndex(SAHandle, &SAIndex, NULL, NULL))
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

    LOG_INFO("\n\t EIP164_SecY_SA_NextPN_Update \n");

    Rc = EIP164_SecY_SA_NextPN_Update(&PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->IOArea,
                                      SAIndex,
                                      NextPN_Lo,
                                      NextPN_Hi,
                                      fExtPN);
    if (Rc != EIP164_NO_ERROR)
    {
        LOG_CRIT("%s: Failed to update in EIP-164 device for device %d (%s), "
                 "error %d\n",
                 __func__,
                 DeviceId,
                 ADAPTER_EIP164_DEVICE_NAME(DeviceId),
                 Rc);
        SecYLib_Device_Unlock(DeviceId);
        return SECY_DLIB_ERROR(Rc);
    }

    do
    {
        LOG_INFO("\n\t EIP164_SecY_SA_NextPN_Status_Get \n");

        Rc = EIP164_SecY_SA_NextPN_Status_Get(&PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->IOArea,
                                              &fNextPNWritten,
                                              &fBusy);
        if (Rc != EIP164_NO_ERROR)
        {
            LOG_CRIT("%s: Failed to read status for device %d (%s), "
                     "error %d\n",
                     __func__,
                     DeviceId,
                     ADAPTER_EIP164_DEVICE_NAME(DeviceId),
                     Rc);
            SecYLib_Device_Unlock(DeviceId);
            return SECY_DLIB_ERROR(Rc);
        }

        Adapter_SleepMS(ADAPTER_EIP164_SYNC_RETRY_TIMEOUT_MS);
        if (LoopCounter)
            LoopCounter--;
        else
            break;

    } while (fBusy);

    if (fNextPNWritten_p)
        *fNextPNWritten_p = fNextPNWritten;

    SecYLib_Device_Unlock(DeviceId);

    LOG_INFO("\n %s done \n", __func__);

    return SECY_STATUS_OK;
}


/*-----------------------------------------------------------------------------
 * SecY_SA_WindowSize_Update
 */
SecY_Status_t
SecY_SA_WindowSize_Update(
        const unsigned int DeviceId,
        const SecY_SAHandle_t SAHandle,
        const uint32_t WindowSize)
{
    EIP164_Error_t Rc;
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

    Rc = EIP164_SecY_SA_Update_Control_Word_Update(&PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->IOArea,
                                                   SAIndex,
                                                   Offset,
                                                   WindowSize);
    if (Rc != EIP164_NO_ERROR)
    {
        SecYLib_Device_Unlock(DeviceId);
        return SECY_DLIB_ERROR(Rc);
    }

    SecYLib_Device_Unlock(DeviceId);
    return SECY_STATUS_OK;
}


/*----------------------------------------------------------------------------
 * SecY_SA_Read
 */
SecY_Status_t
SecY_SA_Read(
        const unsigned int DeviceId,
        const SecY_SAHandle_t SAHandle,
        const unsigned int WordOffset,
        const unsigned int WordCount,
        uint32_t * Transform_p)
{
    unsigned int SAIndex;
    SecY_Status_t SecY_Rc = SECY_STATUS_OK;

    LOG_INFO("\n %s \n", __func__);

#ifdef ADAPTER_EIP164_STRICT_ARGS
    if (DeviceId >= ADAPTER_EIP164_MAX_NOF_DEVICES)
        return SECY_ERROR_BAD_PARAMETER;

    if (Transform_p == NULL)
        return SECY_ERROR_BAD_PARAMETER;

    if (SecY_SAHandle_IsSame(&SAHandle, &SecY_SAHandle_NULL))
        return SECY_ERROR_BAD_PARAMETER;
#endif

    /* Get the SA index in the device for this SA handle */
    if (!SecYLib_SAHandleToIndex(SAHandle, &SAIndex, NULL, NULL))
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

    SecY_Rc =
        SecYLib_SA_Read(DeviceId, SAIndex, WordOffset, WordCount, Transform_p);

    SecYLib_Device_Unlock(DeviceId);

    LOG_INFO("\n %s done \n", __func__);

    return SecY_Rc;
}


/*----------------------------------------------------------------------------
 * SecY_SA_Remove
 */
SecY_Status_t
SecY_SA_Remove(
        const unsigned int DeviceId,
        const SecY_SAHandle_t SAHandle)
{
    EIP164_Error_t Rc = 0;
    SecY_SA_Descriptor_t * SADscr_p = (SecY_SA_Descriptor_t*)SAHandle.p;
    unsigned int SAIndex;
    unsigned int vPort;
    unsigned int saCount;

    LOG_INFO("\n %s \n", __func__);

#ifdef ADAPTER_EIP164_STRICT_ARGS
    if (DeviceId >= ADAPTER_EIP164_MAX_NOF_DEVICES)
        return SECY_ERROR_BAD_PARAMETER;

#endif
    if (SecY_SAHandle_IsSame(&SAHandle, &SecY_SAHandle_NULL))
        return SECY_ERROR_BAD_PARAMETER;

#ifdef ADAPTER_EIP164_DBG
    /* SA descriptor magic number check */
    if (SADscr_p->Magic != ADAPTER_EIP164_SA_DSCR_MAGIC)
        return SECY_ERROR_INTERNAL;
#endif /* ADAPTER_EIP164_DBG */

    SAIndex = SADscr_p->u.InUse.SAIndex;
    SecYLib_Device_Lock(DeviceId);

    if (!PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device_StaticFields)[DeviceId].fInitialized)
    {
        SecYLib_Device_Unlock(DeviceId);
        return SECY_ERROR_INTERNAL;
    }

    vPort = SADscr_p->u.InUse.SAMFlowCtrlIndex;
    if (SAIndex >= PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->SACount ||
        vPort >= PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->vPortCount)
    {
        SecYLib_Device_Unlock(DeviceId);
        return SECY_ERROR_INTERNAL;
    }

#ifdef ADAPTER_EIP164_WARMBOOT_ENABLE
        SecYLib_WarmBoot_SA_Clear(DeviceId, SAIndex);
#endif

    if (SADscr_p->u.InUse.MapType != SECY_SA_MAP_DETACHED)
    {
        SecY_SC_Descriptor_t *SCDscr_p;

        if (SADscr_p->u.InUse.SCIndex >= PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->SCCount)
            goto sa_remove_error;

        SCDscr_p = &PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->SCDscr_p[SADscr_p->u.InUse.SCIndex];

        /* SA has not been detached from SC by overwriting in SecY_SA_Add */
        if ((SADscr_p->u.InUse.MapType == SECY_SA_MAP_INGRESS) ||
            (SADscr_p->u.InUse.MapType == SECY_SA_MAP_INGRESS_CRYPT_AUTH))
        {
            LOG_INFO("\n\t EIP164_SecY_SC_SA_Map_I_Update \n");

            if (SADscr_p->u.InUse.MapType == SECY_SA_MAP_INGRESS)
            {
                /* Unmap the SA from the device. */
                Rc = EIP164_SecY_SC_SA_Map_I_Update(&PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->IOArea,
                                                    SADscr_p->u.InUse.SCIndex,
                                                    SADscr_p->u.InUse.AN,
                                                    SADscr_p->u.InUse.SAIndex,
                                                    false);
                if (Rc != EIP164_NO_ERROR)
                {
                    LOG_CRIT("%s: Failed to unmap ingress SA from "
                             "EIP-164 device for device %d\n",
                         __func__,
                             DeviceId);
                    goto sa_remove_error;
                }
                SCDscr_p->SAHandle[SADscr_p->u.InUse.AN] = SecY_SAHandle_NULL;
            }
            else
            {
                int i;
                /* Crypt-authenticate. */
                for (i=0; i<4; i++)
                {
                    Rc = EIP164_SecY_SC_SA_Map_I_Update(&PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->IOArea,
                                                        SADscr_p->u.InUse.SCIndex,
                                                        i,
                                                        SADscr_p->u.InUse.SAIndex,
                                                        false);
                    if (Rc != EIP164_NO_ERROR)
                    {
                        goto sa_remove_error;
                    }

                    SCDscr_p->SAHandle[i] = SecY_SAHandle_NULL;
                }
            }

            if (SecYLib_SAHandle_IsSame(&SCDscr_p->SAHandle[0], &SecY_SAHandle_NULL) &&
                SecYLib_SAHandle_IsSame(&SCDscr_p->SAHandle[1], &SecY_SAHandle_NULL) &&
                SecYLib_SAHandle_IsSame(&SCDscr_p->SAHandle[2], &SecY_SAHandle_NULL) &&
                SecYLib_SAHandle_IsSame(&SCDscr_p->SAHandle[3], &SecY_SAHandle_NULL))
            {
                /* All four AN slots are empty now, remove CAM entry. */
                List_Status_t List_Rc;
                SCDscr_p->MapType = SECY_SA_MAP_DETACHED;

                /* Remove element from vPort specific search list. */
                List_Rc = List_RemoveAnywhere(0,
                                              PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->vPortDscr_p[vPort].SCList,
                                              &PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->SCDscr_p[SADscr_p->u.InUse.SCIndex].OnList);
                if (List_Rc != LIST_STATUS_OK)
                {
                    LOG_CRIT("%s: Failed to remove SC from lookup list for "
                             "EIP-164 device for device %d\n",
                             __func__,
                             DeviceId);
                    Rc = EIP164_UNSUPPORTED_FEATURE_ERROR;
                    goto sa_remove_error;
                }

                /* Add it to the free list */
                List_Rc = List_AddToHead(0,
                                         PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->SCFL_p,
                                         &PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->SCDscr_p[SADscr_p->u.InUse.SCIndex].OnList);
                if (List_Rc != LIST_STATUS_OK)
                {
                    LOG_CRIT("%s: Failed to free an SC for EIP-164 device for "
                             "device %d\n",
                             __func__,
                             DeviceId);
                    Rc = EIP164_UNSUPPORTED_FEATURE_ERROR;
                    goto sa_remove_error;
                }

                {
                    unsigned int LoopCounter = ADAPTER_EIP164_MAX_NOF_SYNC_RETRY_COUNT;

                    for(;;)
                    {
                        LoopCounter--;

                        LOG_INFO("\n\t EIP164_SecY_RxCAM_Remove \n");

                        Rc = EIP164_SecY_RxCAM_Remove(&PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->IOArea,
                                                      SADscr_p->u.InUse.SCIndex);
                        if (Rc == EIP164_NO_ERROR)
                        {
                            break;
                        }
                        else if (Rc != EIP164_BUSY_RETRY_LATER ||
                                 LoopCounter == 0)
                        {
                            goto sa_remove_error;
                        }
                        Adapter_SleepMS(ADAPTER_EIP164_SYNC_RETRY_TIMEOUT_MS);
                    }
                }
            }
        }
        else
        {
            LOG_INFO("\n\t EIP164_SecY_SC_SA_Map_E_Update \n");

            /* Unmap the SA from the device. */
            Rc = EIP164_SecY_SC_SA_Map_E_Update(&PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->IOArea,
                                                SADscr_p->u.InUse.SCIndex,
                                                SADscr_p->u.InUse.SAIndex,
                                                false,
                                                false);
            if (Rc != EIP164_NO_ERROR)
            {
                LOG_CRIT("%s: Failed to unmap ingress SA from "
                         "EIP-164 device for device %d\n",
                         __func__,
                         DeviceId);
                goto sa_remove_error;
            }

            SCDscr_p->SAHandle[0] = SecY_SAHandle_NULL;
        }

        /* If last SA removed, clear SAMFlow entry. */
        saCount = PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->vPortDscr_p[vPort].SACount;
        PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_SET(SecY_Device[DeviceId]->vPortDscr_p[vPort].SACount,--saCount);
        if (PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->vPortDscr_p[vPort].SACount==0)
        {
            LOG_INFO("\n\t EIP164_SecY_SAMFlow_Clear \n");

            Rc = EIP164_SecY_SAMFlow_Clear(&PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->IOArea,
                                           vPort);
            if (Rc != EIP164_NO_ERROR)
            {
                LOG_CRIT("%s: Failed to remove SA from EIP-164 device for "
                         "device %d (%s), error %d\n",
                         __func__, DeviceId, ADAPTER_EIP164_DEVICE_NAME(DeviceId), Rc);
                goto sa_remove_error;
            }
        }
    }

sa_remove_error:
    /* Add SA to be removed to the SA free list */
    {
        List_Status_t List_Rc;
        /* Invalidate record. */
        PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_SET(SecY_Device[DeviceId]->SADscr_p[SAIndex].Magic,0);

        PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_SET(SecY_Device[DeviceId]->SADscr_p[SAIndex].u.free.DataObject_p,
            &PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->SADscr_p[SAIndex]);

        List_Rc = List_AddToHead(0,
                                 PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->SAFL_p,
                                 &PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->SADscr_p[SAIndex].u.free);
        if (List_Rc != LIST_STATUS_OK)
        {
            LOG_CRIT("%s: Failed to free an SA for EIP-164 device for "
                     "device %d\n",
                     __func__,
                     DeviceId);
            SecYLib_Device_Unlock(DeviceId);
            return SECY_ERROR_INTERNAL;
        }
    }

    SecYLib_Device_Unlock(DeviceId);

    LOG_INFO("\n %s done \n", __func__);

    if (Rc)
    {
        return SECY_DLIB_ERROR(Rc);
    }
    else
    {
        return SECY_STATUS_OK;;
    }
}


#ifdef ADAPTER_EIP164_MODE_EGRESS
/*----------------------------------------------------------------------------
 * SecY_SA_Chain
 */
SecY_Status_t
SecY_SA_Chain(
        const unsigned int DeviceId,
        const SecY_SAHandle_t ActiveSAHandle,
        SecY_SAHandle_t * const NewSAHandle_p,
        const SecY_SA_t * const NewSA_p)
{
    EIP164_Error_t Rc;
    SecY_Status_t SecY_Rc;
    EIP164_SecY_SA_t DeviceSA;
    unsigned int NewSAIndex;
    unsigned int ActiveSAIndex;
    unsigned int SAIndex;
    unsigned int ActiveSCIndex;
    unsigned int SAMFlowCtrlIndex;
    SABuilder_UpdCtrl_Params_t  SAUpdateCtrWord;
    bool fSAInUse;

    LOG_INFO("\n %s \n", __func__);

#ifdef ADAPTER_EIP164_STRICT_ARGS
    if (DeviceId >= ADAPTER_EIP164_MAX_NOF_DEVICES)
        return SECY_ERROR_BAD_PARAMETER;

    if (NewSAHandle_p == NULL)
        return SECY_ERROR_BAD_PARAMETER;

    if (NewSA_p == NULL)
        return SECY_ERROR_BAD_PARAMETER;

    if (SecY_SAHandle_IsSame(&ActiveSAHandle, &SecY_SAHandle_NULL))
        return SECY_ERROR_BAD_PARAMETER;
#endif

    SecYLib_Device_Lock(DeviceId);

    if (!PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device_StaticFields)[DeviceId].fInitialized)
    {
        SecYLib_Device_Unlock(DeviceId);
        return SECY_ERROR_INTERNAL;
    }

    NewSAHandle_p->p = NULL;

    /* Get the SA index in the device for the active SA handle */
    if (!SecYLib_SAHandleToIndex(ActiveSAHandle,
                                 &ActiveSAIndex,
                                 &ActiveSCIndex,
                                 &SAMFlowCtrlIndex))
    {
        LOG_CRIT("%s: Invalid SA handle for EIP-164 device for device %d "
                 "(%s)\n",
                 __func__, DeviceId, ADAPTER_EIP164_DEVICE_NAME(DeviceId));
        SecYLib_Device_Unlock(DeviceId);
        return SECY_ERROR_INTERNAL;
    }

    /* Obtain a free SA for this SecY device */
    {
        List_Element_t * Element_p;
        SecY_SA_Descriptor_t * NewSADscr_p;
        SecY_SA_Descriptor_t * SADscr_p = (SecY_SA_Descriptor_t*)ActiveSAHandle.p;
        SecY_SC_Descriptor_t *SCDscr_p = &PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->SCDscr_p[SADscr_p->u.InUse.SCIndex];
        List_Status_t List_Rc = List_RemoveFromTail(0,
                                                    PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->SAFL_p,
                                                    &Element_p);
        if (List_Rc != LIST_STATUS_OK || Element_p->DataObject_p == NULL)
        {
            LOG_CRIT("%s: Failed to obtain a free SA for EIP-164 device "
                     "for device %d, error %d\n",
                     __func__,
                     DeviceId,
                     List_Rc);
            SecYLib_Device_Unlock(DeviceId);
            return SECY_ERROR_INTERNAL;
        }

        /* Convert SA descriptor to SA index and to SA handle */
        NewSADscr_p = (SecY_SA_Descriptor_t*)Element_p->DataObject_p;

        NewSAIndex = NewSADscr_p - PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->SADscr_p;
        NewSADscr_p->u.InUse.SAIndex = NewSAIndex;
        NewSADscr_p->u.InUse.SAMFlowCtrlIndex = SAMFlowCtrlIndex;
        NewSADscr_p->u.InUse.SCIndex = ActiveSCIndex;
        NewSADscr_p->u.InUse.AN = 0;
        NewSADscr_p->u.InUse.MapType = SADscr_p->u.InUse.MapType;

#ifdef ADAPTER_EIP164_WARMBOOT_ENABLE
        SecYLib_WarmBoot_SA_Set(DeviceId, NewSAIndex);
#endif

        /* SA descriptor magic umber */
        NewSADscr_p->Magic = ADAPTER_EIP164_SA_DSCR_MAGIC;
        NewSAHandle_p->p = NewSADscr_p;
        SCDscr_p->SAHandle[0] = *NewSAHandle_p;
        SADscr_p->u.InUse.MapType = SECY_SA_MAP_DETACHED;
    }

    /* Convert SA data structure */
    ZEROINIT(DeviceSA);

    LOG_INFO("\n\t EIP164_SecY_SA_Write \n");

    Rc = EIP164_SecY_SA_Write(&PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->IOArea,
                              NewSAIndex,
                              NewSA_p->TransformRecord_p,
                              NewSA_p->SA_WordCount);
    if (Rc != EIP164_NO_ERROR)
    {
        LOG_CRIT("%s: Failed to add SA for EIP-164 device for device %d (%s), "
                 "error %d\n",
                 __func__, DeviceId, ADAPTER_EIP164_DEVICE_NAME(DeviceId), Rc);
        SecYLib_Device_Unlock(DeviceId);
        return SECY_DLIB_ERROR(Rc);
    }

    /* Clear SA statistics */
    if (NewSA_p->ActionType == SECY_SA_ACTION_EGRESS)
    {
        LOG_INFO("\n\t EIP164_SecY_SA_Stat_E_Clear \n");

        /* Reset/read egress SA statistics counters */
        Rc = EIP164_SecY_SA_Stat_E_Clear(&PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->IOArea,
                                         NewSAIndex);
        if (Rc != EIP164_NO_ERROR)
        {
            LOG_CRIT("%s: Failed to reset egress SA statistics for EIP-164 "
                     "device for device %d (%s), error %d\n",
                     __func__, DeviceId, ADAPTER_EIP164_DEVICE_NAME(DeviceId), Rc);
            SecYLib_Device_Unlock(DeviceId);
            return SECY_DLIB_ERROR(Rc);
        }
    }

    if (PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->vPortDscr_p[ActiveSCIndex].MTU != 0)
    {
        /* Set MTU value in new SA. */
        SecY_Rc = SecYLib_SA_MTU_Update(DeviceId,
                                        NewSAIndex,
                                        PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->vPortDscr_p[ActiveSCIndex].MTU);
        if (SecY_Rc != SECY_STATUS_OK)
        {
            LOG_CRIT("%s: Failed to update MTU word for device %d "
                     "(%s), error %d\n",
                     __func__, DeviceId, ADAPTER_EIP164_DEVICE_NAME(DeviceId), SecY_Rc);
            SecYLib_Device_Unlock(DeviceId);
            return SecY_Rc;
        }
    }


    /* Enable automatic expiration of new SA. */
    SAUpdateCtrWord.SCIndex = SAMFlowCtrlIndex;
    SAUpdateCtrWord.AN = 0;
    SAUpdateCtrWord.SAIndex = 0;
    SAUpdateCtrWord.fSAIndexValid = false;
    SAUpdateCtrWord.fUpdateEnable = true;
    SAUpdateCtrWord.fExpiredIRQ = true;
    SAUpdateCtrWord.fUpdateTime = false;
    SecY_Rc = SecYLib_SA_Update_Control_Word_Update(DeviceId,
                                                    NewSAIndex,
                                                    &SAUpdateCtrWord);
    if (SecY_Rc != SECY_STATUS_OK)
    {
        LOG_CRIT("%s: Failed to update SA update control word for device %d "
                 "(%s), error %d\n",
                 __func__, DeviceId, ADAPTER_EIP164_DEVICE_NAME(DeviceId), SecY_Rc);
                 SecYLib_Device_Unlock(DeviceId);
                 return SecY_Rc;
    }


    /* Chain SA */
    SAUpdateCtrWord.SCIndex = SAMFlowCtrlIndex;
    SAUpdateCtrWord.AN = 0;
    SAUpdateCtrWord.SAIndex = NewSAIndex;
    SAUpdateCtrWord.fSAIndexValid = true;
    SAUpdateCtrWord.fUpdateEnable = true;
    SAUpdateCtrWord.fExpiredIRQ = true;
    SAUpdateCtrWord.fUpdateTime = false;

    LOG_INFO("\n\t EIP164_SecY_SA_Update_Control_Word_Update \n");

    SecY_Rc = SecYLib_SA_Update_Control_Word_Update(DeviceId,
                                                    ActiveSAIndex,
                                                    &SAUpdateCtrWord);
    if (SecY_Rc != SECY_STATUS_OK)
    {
        LOG_CRIT("%s: Failed to update SA update control word for device %d "
                 "(%s), error %d\n",
                 __func__, DeviceId, ADAPTER_EIP164_DEVICE_NAME(DeviceId), SecY_Rc);
        SecYLib_Device_Unlock(DeviceId);
        return SecY_Rc;
    }

    /* Sync */
    SecY_Rc = SecYLib_Device_Sync(DeviceId);
    if (SecY_Rc != SECY_STATUS_OK)
    {
        LOG_CRIT("%s: synchronization failed for device %d, error %d\n",
                 __func__, DeviceId, SecY_Rc);
        SecYLib_Device_Unlock(DeviceId);
        return SecY_Rc;
    }

    LOG_INFO("\n\t EIP164_SecY_SA_Installation_Verify \n");

    /* Check if it is ready */
    Rc = EIP164_SecY_SA_Installation_Verify(&PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->IOArea,
                                       SAMFlowCtrlIndex,
                                       &SAIndex,
                                       &fSAInUse);

    SecYLib_Device_Unlock(DeviceId);

    if (Rc == EIP164_NO_ERROR && fSAInUse)
    {
        LOG_INFO("\n %s done \n", __func__);

        return SECY_STATUS_OK;
    }
    else
        return SECY_DLIB_ERROR(Rc);
}

/*----------------------------------------------------------------------------
 * SecY_SA_Switch
 */
SecY_Status_t
SecY_SA_Switch(
        const unsigned int DeviceId,
        const SecY_SAHandle_t ActiveSAHandle,
        const SecY_SAHandle_t NewSAHandle,
        const SecY_SA_t * const NewSA_p)
{
    EIP164_Error_t Rc;
    SecY_Status_t SecY_Rc;
    unsigned int NewSAIndex;
    unsigned int ActiveSAIndex;
    unsigned int SAIndex;
    unsigned int SAMFlowCtrlIndex;
    bool fSAInUse;

    IDENTIFIER_NOT_USED(NewSA_p);

    LOG_INFO("\n %s \n", __func__);

#ifdef ADAPTER_EIP164_STRICT_ARGS
    if (DeviceId >= ADAPTER_EIP164_MAX_NOF_DEVICES)
        return SECY_ERROR_BAD_PARAMETER;

    if (SecY_SAHandle_IsSame(&ActiveSAHandle, &SecY_SAHandle_NULL))
        return SECY_ERROR_BAD_PARAMETER;

    if (SecY_SAHandle_IsSame(&NewSAHandle, &SecY_SAHandle_NULL))
        return SECY_ERROR_BAD_PARAMETER;
#endif

    SecYLib_Device_Lock(DeviceId);

    if (!PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device_StaticFields)[DeviceId].fInitialized)
    {
        SecYLib_Device_Unlock(DeviceId);
        return SECY_ERROR_INTERNAL;
    }

    /* Get the SA index in the device for the active SA handle */
    if (!SecYLib_SAHandleToIndex(ActiveSAHandle,
                                 &ActiveSAIndex,
                                 NULL,
                                 &SAMFlowCtrlIndex))
    {
        LOG_CRIT("%s: Invalid SA handle for EIP-164 device for device %d "
                 "(%s)\n",
                 __func__, DeviceId, ADAPTER_EIP164_DEVICE_NAME(DeviceId));
        SecYLib_Device_Unlock(DeviceId);
        return SECY_ERROR_INTERNAL;
    }

    /* Get the SA index in the device for the new SA handle */
    if (!SecYLib_SAHandleToIndex(NewSAHandle,
                                 &NewSAIndex,
                                 NULL,
                                 NULL))
    {
        LOG_CRIT("%s: Invalid SA handle for EIP-164 device for device %d "
                 "(%s)\n",
                 __func__, DeviceId, ADAPTER_EIP164_DEVICE_NAME(DeviceId));
        SecYLib_Device_Unlock(DeviceId);
        return SECY_ERROR_INTERNAL;
    }

    {
        SecY_SA_Descriptor_t * SADscr_p = (SecY_SA_Descriptor_t*)ActiveSAHandle.p;
        LOG_INFO("\n\t EIP164_SecY_SC_SA_Map_E_Update \n");

        Rc = EIP164_SecY_SC_SA_Map_E_Update(&PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->IOArea,
                                            SADscr_p->u.InUse.SCIndex,
                                            NewSAIndex,
                                            true,
                                            true);
        if (Rc != EIP164_NO_ERROR)
        {
            LOG_CRIT("%s: Failed to update SA from EIP-164 device for device "
                     "%d (%s), error %d\n",
                     __func__, DeviceId, ADAPTER_EIP164_DEVICE_NAME(DeviceId), Rc);
            SecYLib_Device_Unlock(DeviceId);
            return SECY_DLIB_ERROR(Rc);
        }
    }
    /* Sync */
    SecY_Rc = SecYLib_Device_Sync(DeviceId);
    if (SecY_Rc != SECY_STATUS_OK)
    {
        LOG_CRIT("%s: synchronization failed for device %d, error %d\n",
                 __func__, DeviceId, SecY_Rc);
        SecYLib_Device_Unlock(DeviceId);
        return SecY_Rc;
    }

    LOG_INFO("\n\t EIP164_SecY_SA_Installation_Verify \n");

    /* Check if it is ready */
    Rc = EIP164_SecY_SA_Installation_Verify(&PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->IOArea,
                                            SAMFlowCtrlIndex,
                                            &SAIndex,
                                            &fSAInUse);

    SecYLib_Device_Unlock(DeviceId);

    if (Rc == EIP164_NO_ERROR && fSAInUse)
    {
        LOG_INFO("\n %s done \n", __func__);

        return SECY_STATUS_OK;
    }
    else
    {
        return SECY_DLIB_ERROR(Rc);
    }
}


/*----------------------------------------------------------------------------
 * SecY_SA_Active_E_Get
 */
SecY_Status_t
SecY_SA_Active_E_Get(
        const unsigned int DeviceId,
        const unsigned int vPort,
        SecY_SAHandle_t * const ActiveSAHandle_p)
{
    EIP164_Error_t Rc;
    unsigned int SAIndex;
    bool fSAInUse;

#ifdef ADAPTER_EIP164_STRICT_ARGS
    if (DeviceId >= ADAPTER_EIP164_MAX_NOF_DEVICES)
        return SECY_ERROR_BAD_PARAMETER;

    if (ActiveSAHandle_p == NULL)
        return SECY_ERROR_BAD_PARAMETER;
#endif

    SecYLib_Device_Lock(DeviceId);

    if (!PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device_StaticFields)[DeviceId].fInitialized)
    {
        SecYLib_Device_Unlock(DeviceId);
        return SECY_ERROR_INTERNAL;
    }

#ifdef ADAPTER_EIP164_STRICT_ARGS
    if (vPort >= PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->vPortCount)
    {
        SecYLib_Device_Unlock(DeviceId);
        return SECY_ERROR_BAD_PARAMETER;
    }
#endif

    Rc = EIP164_SecY_SA_Installation_Verify(&PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->IOArea,
                                       vPort,
                                       &SAIndex,
                                       &fSAInUse);


    if(Rc != EIP164_NO_ERROR ||
       !fSAInUse || !SecYLib_SAIndexToHandle(DeviceId,
                                             SAIndex,
                                             ActiveSAHandle_p))
    {
        SecYLib_Device_Unlock(DeviceId);
        return SECY_DLIB_ERROR(Rc);
    }
    else
    {
        SecYLib_Device_Unlock(DeviceId);
        return SECY_STATUS_OK;
    }
}
#endif /* ADAPTER_EIP164_MODE_EGRESS */


#ifdef ADAPTER_EIP164_MODE_INGRESS
/*----------------------------------------------------------------------------
 * SecY_SA_Active_I_Get
 */
SecY_Status_t
SecY_SA_Active_I_Get(
        const unsigned int DeviceId,
        const unsigned int vPort,
        const uint8_t * const SCI_p,
        SecY_SAHandle_t * const ActiveSAHandle_p)
{
    EIP164_Error_t Rc;
    unsigned int SAIndex[4];
    bool fSAInUse[4];
    unsigned int SCIndex;
    unsigned int i;

#ifdef ADAPTER_EIP164_STRICT_ARGS
    if (DeviceId >= ADAPTER_EIP164_MAX_NOF_DEVICES)
        return SECY_ERROR_BAD_PARAMETER;

    if (SCI_p == NULL)
        return SECY_ERROR_BAD_PARAMETER;

    if (ActiveSAHandle_p == NULL)
        return SECY_ERROR_BAD_PARAMETER;
#endif

    SecYLib_Device_Lock(DeviceId);

    if (!PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device_StaticFields)[DeviceId].fInitialized)
    {
        SecYLib_Device_Unlock(DeviceId);
        return SECY_ERROR_INTERNAL;
    }

#ifdef ADAPTER_EIP164_STRICT_ARGS
    if (vPort >= PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->vPortCount)
    {
        SecYLib_Device_Unlock(DeviceId);
        return SECY_ERROR_BAD_PARAMETER;
    }
#endif
    if( !SecYLib_SCI_Find(DeviceId, SCI_p, vPort, &SCIndex))
    {
        /* SCI not found for this vPort */
        SecYLib_Device_Unlock(DeviceId);
        return SECY_ERROR_BAD_PARAMETER;
    }

    Rc = EIP164_SecY_SA_Active_I_Get(&PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->IOArea,
                                SCIndex,
                                SAIndex,
                                fSAInUse);
    if (Rc != EIP164_NO_ERROR)
    {
        SecYLib_Device_Unlock(DeviceId);
        return SECY_DLIB_ERROR(Rc);
    }
    for (i=0; i<4; i++)
    {
        if( !fSAInUse[i] || !SecYLib_SAIndexToHandle(DeviceId,
                                             SAIndex[i],
                                             &ActiveSAHandle_p[i]))
        {
            ActiveSAHandle_p[i] = SecY_SAHandle_NULL;
        }
    }

    SecYLib_Device_Unlock(DeviceId);
    return SECY_STATUS_OK;
}
#endif /* ADAPTER_EIP164_MODE_INGRESS */


/*----------------------------------------------------------------------------
 * SecY_Rules_MTUCheck_Update
 */
SecY_Status_t
SecY_Rules_MTUCheck_Update(
        const unsigned int DeviceId,
        const unsigned int SCIndex,
        const SecY_SC_Rule_MTUCheck_t * const MTUCheck_Rule_p)
{
#ifdef ADAPTER_EIP164_MODE_EGRESS
    EIP164_Error_t Rc;
    SABuilder_Status_t SAB_Rc;
    SecY_Status_t SecY_Rc;
    SABuilder_MTU_Params_t MTUParams;
    bool fSAInUse;
    unsigned int SAIndex;
    LOG_INFO("\n %s \n", __func__);

#ifdef ADAPTER_EIP164_STRICT_ARGS
    if (DeviceId >= ADAPTER_EIP164_MAX_NOF_DEVICES)
        return SECY_ERROR_BAD_PARAMETER;

    if (SCIndex >= PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->SCCount)
        return SECY_ERROR_BAD_PARAMETER;

    if (MTUCheck_Rule_p == NULL)
        return SECY_ERROR_BAD_PARAMETER;
#endif

    SecYLib_Device_Lock(DeviceId);

    if (!PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device_StaticFields)[DeviceId].fInitialized)
    {
        SecYLib_Device_Unlock(DeviceId);
        return SECY_ERROR_INTERNAL;
    }

    /* Convert MTU parameters to value to be put in SA. */
    MTUParams.MTU = MTUCheck_Rule_p->PacketMaxByteCount;
    MTUParams.fEnable = (MTUCheck_Rule_p->PacketMaxByteCount > 0);
    MTUParams.fDrop = MTUCheck_Rule_p->fOverSizeDrop;
    SAB_Rc = SABuilder_MTU_Update(&MTUParams, &PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->vPortDscr_p[SCIndex].MTU);
    if (SAB_Rc != SAB_STATUS_OK)
    {
        SecYLib_Device_Unlock(DeviceId);
        return SECY_ERROR_INTERNAL;
    }

    /* Check for an existing active egress SA. */
    Rc = EIP164_SecY_SA_Installation_Verify(&PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->IOArea,
                                            SCIndex,
                                            &SAIndex,
                                            &fSAInUse);
    if (Rc != EIP164_NO_ERROR)
    {

            SecYLib_Device_Unlock(DeviceId);
            return SECY_DLIB_ERROR(Rc);
    }
    while (fSAInUse)
    {
        LOG_CRIT("Updating MTU in existing SA=%u for SC=%u\n",SAIndex,SCIndex);

        /* Update MTU field in active egress SA */
        SecY_Rc = SecYLib_SA_MTU_Update(DeviceId,
                                        SAIndex,
                                        PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->vPortDscr_p[SCIndex].MTU);
        if (SecY_Rc != SECY_STATUS_OK)
        {
            SecYLib_Device_Unlock(DeviceId);
            return SECY_DLIB_ERROR(Rc);
        }

        /* Visit any pending chained SAs to update MTU. */
        SecY_Rc = SecYLib_SA_Chained_Get(DeviceId,
                                         SAIndex,
                                         &SAIndex,
                                         &fSAInUse);
        if (SecY_Rc != SECY_STATUS_OK)
        {
            SecYLib_Device_Unlock(DeviceId);
            return SecY_Rc;
        }
    }

    SecYLib_Device_Unlock(DeviceId);

    LOG_INFO("\n %s done \n", __func__);

#else
    IDENTIFIER_NOT_USED(DeviceId);
    IDENTIFIER_NOT_USED(SCIndex);
    IDENTIFIER_NOT_USED(MTUCheck_Rule_p);
#endif
    return SECY_STATUS_OK;
}


/*----------------------------------------------------------------------------
 * SecY_Rules_MTUCheck_Get
 */
SecY_Status_t
SecY_Rules_MTUCheck_Get(
        const unsigned int DeviceId,
        const unsigned int SCIndex,
        SecY_SC_Rule_MTUCheck_t * const MTUCheck_Rule_p)
{
#ifdef ADAPTER_EIP164_MODE_EGRESS
    SABuilder_Status_t SAB_Rc;
    uint32_t MTU;
    SABuilder_MTU_Params_t MTUParams;
    LOG_INFO("\n %s \n", __func__);

#ifdef ADAPTER_EIP164_STRICT_ARGS
    if (DeviceId >= ADAPTER_EIP164_MAX_NOF_DEVICES)
        return SECY_ERROR_BAD_PARAMETER;

    if (SCIndex >= PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->SCCount)
        return SECY_ERROR_BAD_PARAMETER;

    if (MTUCheck_Rule_p == NULL)
        return SECY_ERROR_BAD_PARAMETER;
#endif

    SecYLib_Device_Lock(DeviceId);

    if (!PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device_StaticFields)[DeviceId].fInitialized)
    {
        SecYLib_Device_Unlock(DeviceId);
        return SECY_ERROR_INTERNAL;
    }

    MTU = PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->vPortDscr_p[SCIndex].MTU;

    SAB_Rc = SABuilder_MTU_Decode(MTU, &MTUParams);
    if (SAB_Rc != SAB_STATUS_OK)
    {
        SecYLib_Device_Unlock(DeviceId);
        return SECY_ERROR_INTERNAL;
    }

    MTUCheck_Rule_p->PacketMaxByteCount = MTUParams.MTU;
    MTUCheck_Rule_p->fOverSizeDrop = MTUParams.fDrop;
    SecYLib_Device_Unlock(DeviceId);

    LOG_INFO("\n %s done \n", __func__);

#else
    IDENTIFIER_NOT_USED(DeviceId);
    IDENTIFIER_NOT_USED(SCIndex);
    IDENTIFIER_NOT_USED(MTUCheck_Rule_p);
#endif
    return SECY_STATUS_OK;
}


/*----------------------------------------------------------------------------
 * SecY_SAHandle_IsSame
 */
bool
SecY_SAHandle_IsSame(
        const SecY_SAHandle_t * const Handle1_p,
        const SecY_SAHandle_t * const Handle2_p)
{
    return SecYLib_SAHandle_IsSame(Handle1_p, Handle2_p);
}


/*----------------------------------------------------------------------------
 * SecY_SAIndex_IsSame
 */
bool
SecY_SAHandle_SAIndex_IsSame(
        const SecY_SAHandle_t SAHandle,
        const unsigned int SAIndex)
{
    unsigned int SAIndexLoc;

    if (SecYLib_SAHandleToIndex(SAHandle, &SAIndexLoc, NULL, NULL) == false)
    {
        return false;
    }

    return (SAIndex == SAIndexLoc);
}


/**----------------------------------------------------------------------------
 * SecY_SAIndex_Get
 */
SecY_Status_t
SecY_SAIndex_Get(
        const SecY_SAHandle_t SAHandle,
        unsigned int * const SAIndex_p,
        unsigned int * const SCIndex_p)
{
    if (SecYLib_SAHandle_IsSame(&SAHandle, &SecY_SAHandle_NULL))
    {
        return SECY_ERROR_BAD_PARAMETER;
    }

    if (SAIndex_p == NULL)
    {
        return SECY_ERROR_BAD_PARAMETER;
    }

    if (SecYLib_SAHandleToIndex(SAHandle, SAIndex_p, SCIndex_p, NULL) == false)
    {
        return SECY_ERROR_INTERNAL;
    }

    return SECY_STATUS_OK;
}


/**----------------------------------------------------------------------------
 * SecY_SAHandle_Get
 */
SecY_Status_t
SecY_SAHandle_Get(
        const unsigned int DeviceId,
        const unsigned int SAIndex,
        SecY_SAHandle_t * const SAHandle_p)
{
    SecY_Status_t SecY_Rc = SECY_STATUS_OK;

    if (SAHandle_p == NULL)
    {
        return SECY_ERROR_BAD_PARAMETER;
    }

    SecYLib_Device_Lock(DeviceId);

    if (!PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device_StaticFields)[DeviceId].fInitialized)
    {
        SecYLib_Device_Unlock(DeviceId);
        return SECY_ERROR_INTERNAL;
    }

    if(!SecYLib_SAIndexToHandle(DeviceId,
                                SAIndex,
                                SAHandle_p))
    {
        SecYLib_Device_Unlock(DeviceId);
        SecY_Rc = SECY_ERROR_INTERNAL;
    }

    SecYLib_Device_Unlock(DeviceId);
    return SecY_Rc;
}


/* end of file adapter_secy_sa.c */
