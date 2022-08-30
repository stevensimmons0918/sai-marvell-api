/* adapter_secy_stats.c
 *
 * SecY API Statistics implementation.
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


/* Logging API */
#include <Kit/Log/incl/log.h>                /* LOG_* */

/* Driver Framework C Library Abstraction API */
#include <Kit/DriverFramework/incl/clib.h>               /* ZEROINIT() */

/* EIP-164 Driver Library SecY API */
#include <Kit/EIP164/incl/eip164_types.h>
#include <Kit/EIP164/incl/eip164_secy.h>

/*----------------------------------------------------------------------------
 * Definitions and macros
 */


/*----------------------------------------------------------------------------
 * SecYLib_DeviceCounter_Copy
 */
static inline void
SecYLib_DeviceCounter_Copy(
        const EIP164_UI64_t * const SrcCtr_p,
        SecY_Stat_Counter_t * const DstCtr_p)
{
    DstCtr_p->Lo = SrcCtr_p->low;
    DstCtr_p->Hi = SrcCtr_p->hi;
}


#ifdef ADAPTER_EIP164_MODE_EGRESS
/*----------------------------------------------------------------------------
 * SecY_SA_Statistics_E_Get
 */
SecY_Status_t
SecY_SA_Statistics_E_Get(
        const unsigned int DeviceId,
        const SecY_SAHandle_t SAHandle,
        SecY_SA_Stat_E_t * const Stat_p,
        const bool fSync)
{
    EIP164_Error_t Rc;
    SecY_Status_t SecY_Rc;
    unsigned int SAIndex;
    EIP164_SecY_SA_Stat_E_t DeviceStat;

    LOG_INFO("\n %s \n", __func__);

#ifdef ADAPTER_EIP164_STRICT_ARGS
    if (DeviceId >= ADAPTER_EIP164_MAX_NOF_DEVICES)
        return SECY_ERROR_BAD_PARAMETER;

    if (SecY_SAHandle_IsSame(&SAHandle, &SecY_SAHandle_NULL))
        return SECY_ERROR_BAD_PARAMETER;

    if (Stat_p == NULL)
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
        LOG_CRIT("%s: Invalid SA handle for EIP-164 device for device %d "
                 "(%s)\n",
                 __func__, DeviceId, ADAPTER_EIP164_DEVICE_NAME(DeviceId));
        SecYLib_Device_Unlock(DeviceId);
        return SECY_ERROR_INTERNAL;
    }

    /* Synchronize with the device if required */
    if (fSync)
    {
        SecY_Rc = SecYLib_Device_Sync(DeviceId);
        if (SecY_Rc != SECY_STATUS_OK)
        {
            LOG_CRIT("%s: synchronization failed for device %d, error %d\n",
                     __func__, DeviceId, SecY_Rc);
            SecYLib_Device_Unlock(DeviceId);
            return SecY_Rc;
        }
    }

    ZEROINIT(DeviceStat);

    LOG_INFO("\n\t EIP164_SecY_SA_Stat_E_Get \n");

    /* Read this egress SA statistics from the device */
    Rc = EIP164_SecY_SA_Stat_E_Get(&PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->IOArea,
                                   SAIndex,
                                   &DeviceStat,
                                   ADAPTER_EIP164_WRITE_TO_DECREMENT);
    if (Rc != EIP164_NO_ERROR)
    {
        LOG_CRIT("%s: Failed to read egress SA statistics from EIP-164 device "
                 "for device %d (%s), error %d\n",
                 __func__, DeviceId, ADAPTER_EIP164_DEVICE_NAME(DeviceId), Rc);
        SecYLib_Device_Unlock(DeviceId);
        return SECY_DLIB_ERROR(Rc);
    }

    /* Copy egress SA statistics counters */
    SecYLib_DeviceCounter_Copy(&DeviceStat.OutOctetsEncryptedProtected,
                               &Stat_p->OutOctetsEncryptedProtected);

    SecYLib_DeviceCounter_Copy(&DeviceStat.OutPktsEncryptedProtected,
                               &Stat_p->OutPktsEncryptedProtected);
    SecYLib_DeviceCounter_Copy(&DeviceStat.OutPktsTooLong,
                               &Stat_p->OutPktsTooLong);
    SecYLib_DeviceCounter_Copy(&DeviceStat.OutPktsSANotInUse,
                               &Stat_p->OutPktsSANotInUse);

    SecYLib_Device_Unlock(DeviceId);

    LOG_INFO("\n %s done \n", __func__);

    return SECY_STATUS_OK;
}
#endif /* ADAPTER_EIP164_MODE_EGRESS */


#ifdef ADAPTER_EIP164_MODE_INGRESS
/*----------------------------------------------------------------------------
 * SecY_SA_Statistics_I_Get
 */
SecY_Status_t
SecY_SA_Statistics_I_Get(
        const unsigned int DeviceId,
        const SecY_SAHandle_t SAHandle,
        SecY_SA_Stat_I_t * const Stat_p,
        const bool fSync)
{
    EIP164_Error_t Rc;
    SecY_Status_t SecY_Rc;
    unsigned int SAIndex;
    EIP164_SecY_SA_Stat_I_t DeviceStat;

    LOG_INFO("\n %s \n", __func__);

#ifdef ADAPTER_EIP164_STRICT_ARGS
    if (DeviceId >= ADAPTER_EIP164_MAX_NOF_DEVICES)
        return SECY_ERROR_BAD_PARAMETER;

    if (SecY_SAHandle_IsSame(&SAHandle, &SecY_SAHandle_NULL))
        return SECY_ERROR_BAD_PARAMETER;

    if (Stat_p == NULL)
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
        LOG_CRIT("%s: Invalid SA handle for EIP-164 device for device %d "
                 "(%s)\n",
                 __func__, DeviceId, ADAPTER_EIP164_DEVICE_NAME(DeviceId));
        SecYLib_Device_Unlock(DeviceId);
        return SECY_ERROR_INTERNAL;
    }

    if (fSync)
    {
        SecY_Rc = SecYLib_Device_Sync(DeviceId);
        if (SecY_Rc != SECY_STATUS_OK)
        {
            LOG_CRIT("%s: synchronization failed for device %d, error %d\n",
                     __func__, DeviceId, SecY_Rc);
            SecYLib_Device_Unlock(DeviceId);
            return SecY_Rc;
        }
    }

    ZEROINIT(DeviceStat);

    LOG_INFO("\n\t EIP164_SecY_SA_Stat_I_Get \n");

    /* Read this Ingress SA statistics from the device */
    Rc = EIP164_SecY_SA_Stat_I_Get(&PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->IOArea,
                                   SAIndex,
                                   &DeviceStat,
                                   ADAPTER_EIP164_WRITE_TO_DECREMENT);
    if (Rc != EIP164_NO_ERROR)
    {
        LOG_CRIT("%s: Failed to read ingress SA statistics from EIP-164 "
                 "device for device %d (%s), error %d\n",
                 __func__, DeviceId, ADAPTER_EIP164_DEVICE_NAME(DeviceId), Rc);
        SecYLib_Device_Unlock(DeviceId);
        return SECY_DLIB_ERROR(Rc);
    }

    /* Copy ingress SA statistics counters */
    SecYLib_DeviceCounter_Copy(&DeviceStat.InOctetsDecrypted,
                               &Stat_p->InOctetsDecrypted);
    SecYLib_DeviceCounter_Copy(&DeviceStat.InOctetsValidated,
                               &Stat_p->InOctetsValidated);

    SecYLib_DeviceCounter_Copy(&DeviceStat.InPktsUnchecked,
                               &Stat_p->InPktsUnchecked);
    SecYLib_DeviceCounter_Copy(&DeviceStat.InPktsDelayed,
                               &Stat_p->InPktsDelayed);
    SecYLib_DeviceCounter_Copy(&DeviceStat.InPktsLate,
                               &Stat_p->InPktsLate);
    SecYLib_DeviceCounter_Copy(&DeviceStat.InPktsOK,
                               &Stat_p->InPktsOK);
    SecYLib_DeviceCounter_Copy(&DeviceStat.InPktsInvalid,
                               &Stat_p->InPktsInvalid);
    SecYLib_DeviceCounter_Copy(&DeviceStat.InPktsNotValid,
                               &Stat_p->InPktsNotValid);
    SecYLib_DeviceCounter_Copy(&DeviceStat.InPktsNotUsingSA,
                               &Stat_p->InPktsNotUsingSA);
    SecYLib_DeviceCounter_Copy(&DeviceStat.InPktsUnusedSA,
                               &Stat_p->InPktsUnusedSA);

    SecYLib_Device_Unlock(DeviceId);

    LOG_INFO("\n %s done \n", __func__);

    return SECY_STATUS_OK;
}

#endif /* ADAPTER_EIP164_MODE_INGRESS */


/*----------------------------------------------------------------------------
 * SecY_Device_CountSummary_SecY_CheckAndClear
 */
SecY_Status_t
SecY_Device_CountSummary_PSecY_CheckAndClear(
        const unsigned int DeviceId,
        unsigned int ** const SecYIndexes_pp,
        unsigned int * const NumSecYIndexes_p)
{
    EIP164_Error_t Rc;
    uint32_t Summary;
    unsigned int i;
    unsigned int j;
    unsigned int Index;
    unsigned int MaxIndex;
    unsigned int MaxSummaryIndex;
    unsigned int NumTriggered = 0;
    unsigned int * IndexLoc_p;

    LOG_INFO("\n %s \n", __func__);

#ifdef ADAPTER_EIP164_STRICT_ARGS
    if (DeviceId >= ADAPTER_EIP164_MAX_NOF_DEVICES)
        return SECY_ERROR_BAD_PARAMETER;

    if (SecYIndexes_pp == NULL)
        return SECY_ERROR_BAD_PARAMETER;

    if (NumSecYIndexes_p == NULL)
        return SECY_ERROR_BAD_PARAMETER;
#endif

    SecYLib_Device_Lock(DeviceId);

    if (!PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device_StaticFields)[DeviceId].fInitialized)
    {
        SecYLib_Device_Unlock(DeviceId);
        return SECY_ERROR_INTERNAL;
    }

    *NumSecYIndexes_p = 0;
    IndexLoc_p = *SecYIndexes_pp;
    MaxIndex = PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->vPortCount;
    MaxSummaryIndex = (PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->vPortCount + 31) / 32;

    for (i = 1; i <= MaxSummaryIndex; i++)
    {
        LOG_INFO("\n\t EIP164_Device_CountSummary_PSecY_Read \n");

        /* Read summary register */
        Summary = 0;
        Rc = EIP164_Device_CountSummary_PSecY_Read(&PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->IOArea,
                                                  i,
                                                  &Summary);
        if (Rc != EIP164_NO_ERROR)
        {
            SecYLib_Device_Unlock(DeviceId);
            return SECY_DLIB_ERROR(Rc);
        }

        if (Summary != 0)
        {
            LOG_INFO("\n\t EIP164_Device_CountSummary_PSecY_Clear \n");

            /* Clear it after read */
            Rc = EIP164_Device_CountSummary_PSecY_Clear(&PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->IOArea,
                                                        i,
                                                        Summary);
            if (Rc != EIP164_NO_ERROR)
            {
                SecYLib_Device_Unlock(DeviceId);
                return SECY_DLIB_ERROR(Rc);
            }

            /* return SecY index that crossed the threshold */
            Index = (i - 1) * 32;
            for (j = 0; j < 32; j++)
            {
                if (Index >= MaxIndex)
                {
                    /* All vPort's are handled */
                    goto AllHandled;
                }

                if (Summary & (BIT_0 << j))
                {
                    NumTriggered++;
                    *IndexLoc_p = Index;
                    IndexLoc_p++;
                }

                Index++;
            }
        }
    }

AllHandled:
    *NumSecYIndexes_p = NumTriggered;

    SecYLib_Device_Unlock(DeviceId);

    LOG_INFO("\n %s done \n", __func__);

    return SECY_STATUS_OK;
}


#ifdef ADAPTER_EIP164_PERCOUNTER_SUMMARY_ENABLE
/*----------------------------------------------------------------------------
 * SecY_Device_CountSummary_SecY_CheckAndClear
 */
SecY_Status_t
SecY_Device_CountSummary_SecY_CheckAndClear(
        const unsigned int DeviceId,
        const unsigned int SecYIndex,
        uint32_t * const CountSummarySecY_p)
{
    EIP164_Error_t Rc;
    uint32_t Summary = 0;

    LOG_INFO("\n %s \n", __func__);

#ifdef ADAPTER_EIP164_STRICT_ARGS
    if (DeviceId >= ADAPTER_EIP164_MAX_NOF_DEVICES)
        return SECY_ERROR_BAD_PARAMETER;

    if (CountSummarySecY_p == NULL)
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


    LOG_INFO("\n\t EIP164_Device_CountSummary_SecY_Read \n");

    /* Read IFC1 count summary register */
    Rc = EIP164_Device_CountSummary_SecY_Read(&PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->IOArea,
                                              SecYIndex,
                                              &Summary);
    if (Rc != EIP164_NO_ERROR)
    {
        SecYLib_Device_Unlock(DeviceId);
        return SECY_DLIB_ERROR(Rc);
    }

    if (Summary != 0)
    {
        LOG_INFO("\n\t EIP164_Device_CountSummary_SecY_Clear \n");

        /* Clear the bits set */
        Rc = EIP164_Device_CountSummary_SecY_Clear(&PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->IOArea,
                                                   SecYIndex,
                                                   Summary);
        if (Rc != EIP164_NO_ERROR)
        {
            SecYLib_Device_Unlock(DeviceId);
            return SECY_DLIB_ERROR(Rc);
        }
    }

    *CountSummarySecY_p = Summary;

    SecYLib_Device_Unlock(DeviceId);

    LOG_INFO("\n %s done \n", __func__);

    return SECY_STATUS_OK;
}
#endif


/*----------------------------------------------------------------------------
 * SecY_Device_CountSummary_PIfc_CheckAndClear
 */
SecY_Status_t
SecY_Device_CountSummary_PIfc_CheckAndClear(
        const unsigned int DeviceId,
        unsigned int ** const IfcIndexes_pp,
        unsigned int * const NumIfcIndexes_p)
{
    EIP164_Error_t Rc;
    uint32_t Summary;
    unsigned int i;
    unsigned int j;
    unsigned int Index;
    unsigned int MaxIndex;
    unsigned int MaxSummaryIndex;
    unsigned int NumTriggered = 0;
    unsigned int * IndexLoc_p;

    LOG_INFO("\n %s \n", __func__);

#ifdef ADAPTER_EIP164_STRICT_ARGS
    if (DeviceId >= ADAPTER_EIP164_MAX_NOF_DEVICES)
        return SECY_ERROR_BAD_PARAMETER;

    if (IfcIndexes_pp == NULL)
        return SECY_ERROR_BAD_PARAMETER;

    if (NumIfcIndexes_p == NULL)
        return SECY_ERROR_BAD_PARAMETER;
#endif

    SecYLib_Device_Lock(DeviceId);

    if (!PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device_StaticFields)[DeviceId].fInitialized)
    {
        SecYLib_Device_Unlock(DeviceId);
        return SECY_ERROR_INTERNAL;
    }

    *NumIfcIndexes_p = 0;
    IndexLoc_p = *IfcIndexes_pp;
    MaxIndex = PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->vPortCount;
    MaxSummaryIndex = (PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->vPortCount + 31) / 32;

    for (i = 1; i <= MaxSummaryIndex; i++)
    {
        LOG_INFO("\n\t EIP164_Device_CountSummary_PIFC_Read \n");

        /* Read summary register */
        Summary = 0;
        Rc = EIP164_Device_CountSummary_PIFC_Read(&PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->IOArea,
                                                  i,
                                                  &Summary);
        if (Rc != EIP164_NO_ERROR)
        {
            SecYLib_Device_Unlock(DeviceId);
            return SECY_DLIB_ERROR(Rc);
        }

        if (Summary != 0)
        {
            LOG_INFO("\n\t EIP164_Device_CountSummary_PIFC_Clear \n");

            /* Clear it after read */
            Rc = EIP164_Device_CountSummary_PIFC_Clear(&PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->IOArea,
                                                       i,
                                                       Summary);
            if (Rc != EIP164_NO_ERROR)
            {
                SecYLib_Device_Unlock(DeviceId);
                return SECY_DLIB_ERROR(Rc);
            }

            /* return IFC index that crossed the threshold */
            Index = (i - 1) * 32;
            for (j = 0; j < 32; j++)
            {
                if (Index >= MaxIndex)
                {
                    /* All vPort's are handled */
                    goto AllHandled;
                }

                if (Summary & (BIT_0 << j))
                {
                    NumTriggered++;
                    *IndexLoc_p = Index;
                    IndexLoc_p++;
                }

                Index++;
            }
        }
    }

AllHandled:
    *NumIfcIndexes_p = NumTriggered;

    SecYLib_Device_Unlock(DeviceId);
    LOG_INFO("\n %s done \n", __func__);

    return SECY_STATUS_OK;
}


#ifdef ADAPTER_EIP164_PERCOUNTER_SUMMARY
/*----------------------------------------------------------------------------
 * SecY_Device_CountSummary_Ifc_CheckAndClear
 */
SecY_Status_t
SecY_Device_CountSummary_Ifc_CheckAndClear(
        const unsigned int DeviceId,
        const unsigned int IfcIndex,
        uint32_t * const CountSummaryIfc_p)
{
    EIP164_Error_t Rc;
    uint32_t Summary = 0;

    LOG_INFO("\n %s \n", __func__);

#ifdef ADAPTER_EIP164_STRICT_ARGS
    if (DeviceId >= ADAPTER_EIP164_MAX_NOF_DEVICES)
        return SECY_ERROR_BAD_PARAMETER;

    if (CountSummaryIfc_p == NULL)
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

    LOG_INFO("\n\t EIP164_Device_CountSummary_IFC_Read \n");

    /* Read IFC1 count summary register */
    Rc = EIP164_Device_CountSummary_IFC_Read(&PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->IOArea,
                                             IfcIndex,
                                             &Summary);
    if (Rc != EIP164_NO_ERROR)
    {
        SecYLib_Device_Unlock(DeviceId);
        return SECY_DLIB_ERROR(Rc);
    }

    if (Summary != 0)
    {
        LOG_INFO("\n\t EIP164_Device_CountSummary_IFC_Clear \n");

        /* Clear the bits set */
        Rc = EIP164_Device_CountSummary_IFC_Clear(&PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->IOArea,
                                                  IfcIndex,
                                                  Summary);
        if (Rc != EIP164_NO_ERROR)
        {
            SecYLib_Device_Unlock(DeviceId);
            return SECY_DLIB_ERROR(Rc);
        }
    }

    *CountSummaryIfc_p = Summary;

    SecYLib_Device_Unlock(DeviceId);

    LOG_INFO("\n %s done \n", __func__);

    return SECY_STATUS_OK;
}
#endif


/*----------------------------------------------------------------------------
 * SecY_Device_CountSummary_PIfc1_CheckAndClear
 */
SecY_Status_t
SecY_Device_CountSummary_PIfc1_CheckAndClear(
        const unsigned int DeviceId,
        unsigned int ** const IfcIndexes_pp,
        unsigned int * const NumIfcIndexes_p)
{
    EIP164_Error_t Rc;
    uint32_t Summary;
    unsigned int i;
    unsigned int j;
    unsigned int Index;
    unsigned int MaxIndex;
    unsigned int MaxSummaryIndex;
    unsigned int NumTriggered = 0;
    unsigned int * IndexLoc_p;

    LOG_INFO("\n %s \n", __func__);

#ifdef ADAPTER_EIP164_STRICT_ARGS
    if (DeviceId >= ADAPTER_EIP164_MAX_NOF_DEVICES)
        return SECY_ERROR_BAD_PARAMETER;

    if (IfcIndexes_pp == NULL)
        return SECY_ERROR_BAD_PARAMETER;

    if (NumIfcIndexes_p == NULL)
        return SECY_ERROR_BAD_PARAMETER;
#endif

    SecYLib_Device_Lock(DeviceId);

    if (!PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device_StaticFields)[DeviceId].fInitialized)
    {
        SecYLib_Device_Unlock(DeviceId);
        return SECY_ERROR_INTERNAL;
    }

    *NumIfcIndexes_p = 0;
    IndexLoc_p = *IfcIndexes_pp;
    MaxIndex = PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->vPortCount;
    MaxSummaryIndex = (PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->vPortCount + 31) / 32;

    for (i = 1; i <= MaxSummaryIndex; i++)
    {
        LOG_INFO("\n\t EIP164_Device_CountSummary_PIFC1_Read \n");

        /* Read summary register */
        Summary = 0;
        Rc = EIP164_Device_CountSummary_PIFC1_Read(&PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->IOArea,
                                                   i,
                                                   &Summary);
        if (Rc != EIP164_NO_ERROR)
        {
            SecYLib_Device_Unlock(DeviceId);
            return SECY_DLIB_ERROR(Rc);
        }

        if (Summary != 0)
        {
            LOG_INFO("\n\t EIP164_Device_CountSummary_PIFC1_Clear \n");

            /* Clear it after read */
            Rc = EIP164_Device_CountSummary_PIFC1_Clear(&PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->IOArea,
                                                        i,
                                                        Summary);
            if (Rc != EIP164_NO_ERROR)
            {
                SecYLib_Device_Unlock(DeviceId);
                return SECY_DLIB_ERROR(Rc);
            }

            /* return IFC index that crossed the threshold */
            Index = (i - 1) * 32;
            for (j = 0; j < 32; j++)
            {
                if (Index >= MaxIndex)
                {
                    /* All vPort's are handled */
                    goto AllHandled;
                }

                if (Summary & (BIT_0 << j))
                {
                    NumTriggered++;
                    *IndexLoc_p = Index;
                    IndexLoc_p++;
                }

                Index++;
            }
        }
    }

AllHandled:
    *NumIfcIndexes_p = NumTriggered;

    SecYLib_Device_Unlock(DeviceId);

    LOG_INFO("\n %s done \n", __func__);

    return SECY_STATUS_OK;
}


#ifdef ADAPTER_EIP164_PERCOUNTER_SUMMARY
/*----------------------------------------------------------------------------
 * SecY_Device_CountSummary_Ifc1_CheckAndClear
 */
SecY_Status_t
SecY_Device_CountSummary_Ifc1_CheckAndClear(
        const unsigned int DeviceId,
        const unsigned int IfcIndex,
        uint32_t * const CountSummaryIfc_p)
{
    EIP164_Error_t Rc;
    uint32_t Summary = 0;

    LOG_INFO("\n %s \n", __func__);

#ifdef ADAPTER_EIP164_STRICT_ARGS
    if (DeviceId >= ADAPTER_EIP164_MAX_NOF_DEVICES)
        return SECY_ERROR_BAD_PARAMETER;

    if (CountSummaryIfc_p == NULL)
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
    LOG_INFO("\n\t EIP164_Device_CountSummary_IFC1_Read \n");

    /* Read IFC1 count summary register */
    Rc = EIP164_Device_CountSummary_IFC1_Read(&PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->IOArea,
                                              IfcIndex,
                                              &Summary);
    if (Rc != EIP164_NO_ERROR)
    {
        SecYLib_Device_Unlock(DeviceId);
        return SECY_DLIB_ERROR(Rc);
    }

    if (Summary != 0)
    {
        LOG_INFO("\n\t EIP164_Device_CountSummary_IFC1_Clear \n");

        /* Clear the bits set */
        Rc = EIP164_Device_CountSummary_IFC1_Clear(&PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->IOArea,
                                                   IfcIndex,
                                                   Summary);
        if (Rc != EIP164_NO_ERROR)
        {
            SecYLib_Device_Unlock(DeviceId);
            return SECY_DLIB_ERROR(Rc);
        }
    }

    *CountSummaryIfc_p = Summary;

    SecYLib_Device_Unlock(DeviceId);

    LOG_INFO("\n %s done \n", __func__);

    return SECY_STATUS_OK;
}
#endif


/*----------------------------------------------------------------------------
 * SecY_Device_CountSummary_PRxCAM_CheckAndClear
 */
#ifdef ADAPTER_EIP164_MODE_INGRESS
SecY_Status_t
SecY_Device_CountSummary_PRxCAM_CheckAndClear(
        const unsigned int DeviceId,
        unsigned int ** const RxCAMIndexes_pp,
        unsigned int * const NumRxCAMIndexes_p)
{
    EIP164_Error_t Rc;
    uint32_t Summary;
    unsigned int i;
    unsigned int j;
    unsigned int Index;
    unsigned int MaxIndex;
    unsigned int MaxSummaryIndex;
    unsigned int NumTriggered = 0;
    unsigned int * IndexLoc_p;

    LOG_INFO("\n %s \n", __func__);

#ifdef ADAPTER_EIP164_STRICT_ARGS
    if (DeviceId >= ADAPTER_EIP164_MAX_NOF_DEVICES)
        return SECY_ERROR_BAD_PARAMETER;

    if (RxCAMIndexes_pp == NULL)
        return SECY_ERROR_BAD_PARAMETER;

    if (NumRxCAMIndexes_p == NULL)
        return SECY_ERROR_BAD_PARAMETER;
#endif

    SecYLib_Device_Lock(DeviceId);

    if (!PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device_StaticFields)[DeviceId].fInitialized)
    {
        SecYLib_Device_Unlock(DeviceId);
        return SECY_ERROR_INTERNAL;
    }

    *NumRxCAMIndexes_p = 0;
    IndexLoc_p = *RxCAMIndexes_pp;
    MaxIndex = PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->SCCount;
    MaxSummaryIndex = (PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->SCCount + 31) / 32;

    for (i = 1; i <= MaxSummaryIndex; i++)
    {
        LOG_INFO("\n\t EIP164_Device_CountSummary_PRxCAM_Read \n");

        /* Read summary register */
        Summary = 0;
        Rc = EIP164_Device_CountSummary_PRxCAM_Read(&PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->IOArea,
                                                    i,
                                                    &Summary);
        if (Rc != EIP164_NO_ERROR)
        {
            SecYLib_Device_Unlock(DeviceId);
            return SECY_DLIB_ERROR(Rc);
        }

        if (Summary != 0)
        {
            LOG_INFO("\n\t EIP164_Device_CountSummary_PRxCAM_Clear \n");

            /* Clear it after read */
            Rc = EIP164_Device_CountSummary_PRxCAM_Clear(&PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->IOArea,
                                                         i,
                                                         Summary);
            if (Rc != EIP164_NO_ERROR)
            {
                SecYLib_Device_Unlock(DeviceId);
                return SECY_DLIB_ERROR(Rc);
            }

            /* return IFC index that crossed the threshold */
            Index = (i - 1) * 32;
            for (j = 0; j < 32; j++)
            {
                if (Index >= MaxIndex)
                {
                    /* All SC's are handled */
                    goto AllHandled;
                }

                if (Summary & (BIT_0 << j))
                {
                    NumTriggered++;
                    *IndexLoc_p = Index;
                    IndexLoc_p++;
                }

                Index++;
            }
        }
    }

AllHandled:
    *NumRxCAMIndexes_p = NumTriggered;

    SecYLib_Device_Unlock(DeviceId);

    LOG_INFO("\n %s done \n", __func__);

    return SECY_STATUS_OK;
}
#endif


/*----------------------------------------------------------------------------
 * SecY_Device_CountSummary_PSA_CheckAndClear
 */
SecY_Status_t
SecY_Device_CountSummary_PSA_CheckAndClear(
        const unsigned int DeviceId,
        unsigned int ** const SAIndexes_pp,
        unsigned int * const NumSAIndexes_p)
{
    EIP164_Error_t Rc;
    uint32_t Summary;
    unsigned int i;
    unsigned int j;
    unsigned int Index;
    unsigned int MaxIndex;
    unsigned int MaxSummaryIndex;
    unsigned int NumTriggered = 0;
    unsigned int * IndexLoc_p;

    LOG_INFO("\n %s \n", __func__);

#ifdef ADAPTER_EIP164_STRICT_ARGS
    if (DeviceId >= ADAPTER_EIP164_MAX_NOF_DEVICES)
        return SECY_ERROR_BAD_PARAMETER;

    if (SAIndexes_pp == NULL)
        return SECY_ERROR_BAD_PARAMETER;

    if (NumSAIndexes_p == NULL)
        return SECY_ERROR_BAD_PARAMETER;
#endif

    SecYLib_Device_Lock(DeviceId);

    if (!PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device_StaticFields)[DeviceId].fInitialized)
    {
        SecYLib_Device_Unlock(DeviceId);
        return SECY_ERROR_INTERNAL;
    }

    *NumSAIndexes_p = 0;
    IndexLoc_p = *SAIndexes_pp;
    MaxIndex = PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->SACount;
    MaxSummaryIndex = (PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->SACount + 31) / 32;

    for (i = 1; i <= MaxSummaryIndex; i++)
    {
        LOG_INFO("\n\t EIP164_Device_CountSummary_PSA_Read \n");

        /* Read summary register */
        Summary = 0;
        Rc = EIP164_Device_CountSummary_PSA_Read(&PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->IOArea,
                                                 i,
                                                 &Summary);
        if (Rc != EIP164_NO_ERROR)
        {
            SecYLib_Device_Unlock(DeviceId);
            return SECY_DLIB_ERROR(Rc);
        }

        if (Summary != 0)
        {
            LOG_INFO("\n\t EIP164_Device_CountSummary_PSA_Clear \n");

            /* Clear it after read */
            Rc = EIP164_Device_CountSummary_PSA_Clear(&PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->IOArea,
                                                      i,
                                                      Summary);
            if (Rc != EIP164_NO_ERROR)
            {
                SecYLib_Device_Unlock(DeviceId);
                return SECY_DLIB_ERROR(Rc);
            }

            /* return SA handles that crossed the threshold */
            Index = (i - 1) * 32;
            for (j = 0; j < 32; j++)
            {
                if (Index >= MaxIndex)
                {
                    /* All SA's are handled */
                    goto AllHandled;
                }

                if (Summary & (BIT_0 << j))
                {
                    NumTriggered++;
                    *IndexLoc_p = Index;
                    IndexLoc_p++;
                }

                Index++;
            }
        }
    }

AllHandled:
    *NumSAIndexes_p = NumTriggered;

    SecYLib_Device_Unlock(DeviceId);

    LOG_INFO("\n %s done \n", __func__);

    return SECY_STATUS_OK;
}


#ifdef ADAPTER_EIP164_PERCOUNTER_SUMMARY
/*----------------------------------------------------------------------------
 * SecY_Device_CountSummary_SA_CheckAndClear
 */
SecY_Status_t
SecY_Device_CountSummary_SA_CheckAndClear(
        const unsigned int DeviceId,
        const unsigned int SAIndex,
        uint32_t * const CountSummarySA_p)
{
    EIP164_Error_t Rc;
    uint32_t Summary = 0;

    LOG_INFO("\n %s \n", __func__);

#ifdef ADAPTER_EIP164_STRICT_ARGS
    if (DeviceId >= ADAPTER_EIP164_MAX_NOF_DEVICES)
        return SECY_ERROR_BAD_PARAMETER;

    if (CountSummarySA_p == NULL)
        return SECY_ERROR_BAD_PARAMETER;

    if (SAIndex >= PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->SACount)
        return SECY_ERROR_BAD_PARAMETER;
#endif

    SecYLib_Device_Lock(DeviceId);

    if (!PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device_StaticFields)[DeviceId].fInitialized)
    {
        SecYLib_Device_Unlock(DeviceId);
        return SECY_ERROR_INTERNAL;
    }

    LOG_INFO("\n\t EIP164_Device_CountSummary_SA_Read \n");

    /* Read SA count summary register */
    Rc = EIP164_Device_CountSummary_SA_Read(&PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->IOArea,
                                            SAIndex,
                                            &Summary);
    if (Rc != EIP164_NO_ERROR)
    {
        SecYLib_Device_Unlock(DeviceId);
        return SECY_DLIB_ERROR(Rc);
    }

    if (Summary != 0)
    {
        LOG_INFO("\n\t EIP164_Device_CountSummary_SA_Clear \n");

        /* Clear the bits set */
        Rc = EIP164_Device_CountSummary_SA_Clear(&PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->IOArea,
                                                 SAIndex,
                                                 Summary);
        if (Rc != EIP164_NO_ERROR)
        {
            SecYLib_Device_Unlock(DeviceId);
            return SECY_DLIB_ERROR(Rc);
        }
    }

    *CountSummarySA_p = Summary;

    SecYLib_Device_Unlock(DeviceId);

    LOG_INFO("\n %s done \n", __func__);

    return SECY_STATUS_OK;
}
#endif


#ifdef ADAPTER_EIP164_MODE_EGRESS
/*----------------------------------------------------------------------------
 * SecY_SecY_Statistics_E_Get
 */
SecY_Status_t
SecY_SecY_Statistics_E_Get(
        const unsigned int DeviceId,
        unsigned int vPort,
        SecY_SecY_Stat_E_t * const Stats_p,
        const bool fSync)
{
    EIP164_Error_t Rc;
    SecY_Status_t SecY_Rc;
    EIP164_SecY_Statistics_E_t Stats;

    LOG_INFO("\n %s \n", __func__);

#ifdef ADAPTER_EIP164_STRICT_ARGS
    if (DeviceId >= ADAPTER_EIP164_MAX_NOF_DEVICES)
        return SECY_ERROR_BAD_PARAMETER;

    if (Stats_p == NULL)
        return SECY_ERROR_BAD_PARAMETER;

    if (vPort >= PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->vPortCount)
        return SECY_ERROR_BAD_PARAMETER;
#endif

    SecYLib_Device_Lock(DeviceId);

    if (!PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device_StaticFields)[DeviceId].fInitialized)
    {
        SecYLib_Device_Unlock(DeviceId);
        return SECY_ERROR_INTERNAL;
    }

    /* Synchronize with the device if required */
    if (fSync)
    {
        SecY_Rc = SecYLib_Device_Sync(DeviceId);
        if (SecY_Rc != SECY_STATUS_OK)
        {
            LOG_CRIT("%s: synchronization failed for device %d, error %d\n",
                     __func__, DeviceId, SecY_Rc);
            SecYLib_Device_Unlock(DeviceId);
            return SecY_Rc;
        }
    }

    ZEROINIT(Stats);

    LOG_INFO("\n\t EIP164_SecY_Stat_E_Get \n");

    Rc = EIP164_SecY_Stat_E_Get(&PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->IOArea, vPort, &Stats,
                                   ADAPTER_EIP164_WRITE_TO_DECREMENT);
    if (Rc != EIP164_NO_ERROR)
    {
        LOG_CRIT("%s: failed for device %d, error %d\n",
                 __func__, DeviceId, Rc);
        SecYLib_Device_Unlock(DeviceId);
        return SECY_DLIB_ERROR(Rc);
    }

    /* Copy egress SecY statistics counters */
    SecYLib_DeviceCounter_Copy(&Stats.TransformErrorPkts,
                               &Stats_p->OutPktsTransformError);

    SecYLib_DeviceCounter_Copy(&Stats.OutPktsCtrl,
                               &Stats_p->OutPktsControl);
    SecYLib_DeviceCounter_Copy(&Stats.OutPktsUntagged,
                               &Stats_p->OutPktsUntagged);

    SecYLib_Device_Unlock(DeviceId);

    LOG_INFO("\n %s done \n", __func__);

    return SECY_STATUS_OK;
}

#endif /* ADAPTER_EIP164_MODE_EGRESS */


#ifdef ADAPTER_EIP164_MODE_INGRESS
/*----------------------------------------------------------------------------
 * SecY_SecY_Statistics_I_Get
 */
SecY_Status_t
SecY_SecY_Statistics_I_Get(
        const unsigned int DeviceId,
        unsigned int vPort,
        SecY_SecY_Stat_I_t * const Stats_p,
        const bool fSync)
{
    EIP164_Error_t Rc;
    SecY_Status_t SecY_Rc;
    EIP164_SecY_Statistics_I_t Stats;

    LOG_INFO("\n %s \n", __func__);

#ifdef ADAPTER_EIP164_STRICT_ARGS
    if (DeviceId >= ADAPTER_EIP164_MAX_NOF_DEVICES)
        return SECY_ERROR_BAD_PARAMETER;

    if (Stats_p == NULL)
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
    /* Synchronize with the device if required */
    if (fSync)
    {
        SecY_Rc = SecYLib_Device_Sync(DeviceId);
        if (SecY_Rc != SECY_STATUS_OK)
        {
            LOG_CRIT("%s: synchronization failed for device %d, error %d\n",
                     __func__, DeviceId, SecY_Rc);
            SecYLib_Device_Unlock(DeviceId);
            return SecY_Rc;
        }
    }

    ZEROINIT(Stats);

    LOG_INFO("\n\t EIP164_SecY_Stat_I_Get \n");

    Rc = EIP164_SecY_Stat_I_Get(&PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->IOArea, vPort, &Stats,
                                   ADAPTER_EIP164_WRITE_TO_DECREMENT);
    if (Rc != EIP164_NO_ERROR)
    {
        LOG_CRIT("%s: failed for device %d, error %d\n",
                 __func__, DeviceId, Rc);
        SecYLib_Device_Unlock(DeviceId);
        return SECY_DLIB_ERROR(Rc);
    }

    /* Copy ingress SecY statistics counters */
    SecYLib_DeviceCounter_Copy(&Stats.TransformErrorPkts,
                               &Stats_p->InPktsTransformError);

    SecYLib_DeviceCounter_Copy(&Stats.InPktsCtrl,
                               &Stats_p->InPktsControl);
    SecYLib_DeviceCounter_Copy(&Stats.InPktsUntagged,
                               &Stats_p->InPktsUntagged);
    SecYLib_DeviceCounter_Copy(&Stats.InPktsNoTag,
                               &Stats_p->InPktsNoTag);
    SecYLib_DeviceCounter_Copy(&Stats.InPktsBadTag,
                               &Stats_p->InPktsBadTag);
    SecYLib_DeviceCounter_Copy(&Stats.InPktsNoSCI,
                               &Stats_p->InPktsNoSCI);
    SecYLib_DeviceCounter_Copy(&Stats.InPktsUnknownSCI,
                               &Stats_p->InPktsUnknownSCI);
    SecYLib_DeviceCounter_Copy(&Stats.InPktsTaggedCtrl,
                               &Stats_p->InPktsTaggedCtrl);

    SecYLib_Device_Unlock(DeviceId);

    LOG_INFO("\n %s done \n", __func__);

    return SECY_STATUS_OK;
}


#endif /* ADAPTER_EIP164_MODE_INGRESS */


#ifdef ADAPTER_EIP164_MODE_EGRESS
/*----------------------------------------------------------------------------
 * SecY_Ifc_Statistics_E_Get
 */
SecY_Status_t
SecY_Ifc_Statistics_E_Get(
        const unsigned int DeviceId,
        unsigned int vPort,
        SecY_Ifc_Stat_E_t * const Stats_p,
        const bool fSync)
{
    EIP164_Error_t Rc;
    SecY_Status_t SecY_Rc;
    EIP164_SecY_Ifc_Stat_E_t Stats;

    LOG_INFO("\n %s \n", __func__);

#ifdef ADAPTER_EIP164_STRICT_ARGS
    if (DeviceId >= ADAPTER_EIP164_MAX_NOF_DEVICES)
        return SECY_ERROR_BAD_PARAMETER;

    if (Stats_p == NULL)
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
    /* Synchronize with the device if required */
    if (fSync)
    {
        SecY_Rc = SecYLib_Device_Sync(DeviceId);
        if (SecY_Rc != SECY_STATUS_OK)
        {
            LOG_CRIT("%s: synchronization failed for device %d, error %d\n",
                     __func__, DeviceId, SecY_Rc);
            SecYLib_Device_Unlock(DeviceId);
            return SecY_Rc;
        }
    }

    ZEROINIT(Stats);

    LOG_INFO("\n\t EIP164_SecY_Ifc_Stat_E_Get \n");

    Rc = EIP164_SecY_Ifc_Stat_E_Get(&PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->IOArea,
                                    vPort,
                                    &Stats,
                                   ADAPTER_EIP164_WRITE_TO_DECREMENT);
    if (Rc != EIP164_NO_ERROR)
    {
        LOG_CRIT("%s: failed for device %d, error %d\n",
                 __func__, DeviceId, Rc);
        SecYLib_Device_Unlock(DeviceId);
        return SECY_DLIB_ERROR(Rc);
    }

    /* Copy egress IFC/IFC1 statistics counters */
    SecYLib_DeviceCounter_Copy(&Stats.OutOctetsUncontrolled,
                               &Stats_p->OutOctetsUncontrolled);
    SecYLib_DeviceCounter_Copy(&Stats.OutOctetsControlled,
                               &Stats_p->OutOctetsControlled);
    SecYLib_DeviceCounter_Copy(&Stats.OutOctetsCommon,
                               &Stats_p->OutOctetsCommon);

    SecYLib_DeviceCounter_Copy(&Stats.OutPktsUnicastUncontrolled,
                               &Stats_p->OutPktsUnicastUncontrolled);
    SecYLib_DeviceCounter_Copy(&Stats.OutPktsMulticastUncontrolled,
                               &Stats_p->OutPktsMulticastUncontrolled);
    SecYLib_DeviceCounter_Copy(&Stats.OutPktsBroadcastUncontrolled,
                               &Stats_p->OutPktsBroadcastUncontrolled);

    SecYLib_DeviceCounter_Copy(&Stats.OutPktsUnicastControlled,
                               &Stats_p->OutPktsUnicastControlled);
    SecYLib_DeviceCounter_Copy(&Stats.OutPktsMulticastControlled,
                               &Stats_p->OutPktsMulticastControlled);
    SecYLib_DeviceCounter_Copy(&Stats.OutPktsBroadcastControlled,
                               &Stats_p->OutPktsBroadcastControlled);

    SecYLib_Device_Unlock(DeviceId);

    LOG_INFO("\n %s done \n", __func__);

    return SECY_STATUS_OK;
}
#endif /* ADAPTER_EIP164_MODE_EGRESS */


#ifdef ADAPTER_EIP164_MODE_INGRESS
/*----------------------------------------------------------------------------
 * SecY_Ifc_Statistics_I_Get
 */
SecY_Status_t
SecY_Ifc_Statistics_I_Get(
        const unsigned int DeviceId,
        unsigned int vPort,
        SecY_Ifc_Stat_I_t * const Stats_p,
        const bool fSync)
{
    EIP164_Error_t Rc;
    SecY_Status_t SecY_Rc;
    EIP164_SecY_Ifc_Stat_I_t Stats;

    LOG_INFO("\n %s \n", __func__);

#ifdef ADAPTER_EIP164_STRICT_ARGS
    if (DeviceId >= ADAPTER_EIP164_MAX_NOF_DEVICES)
        return SECY_ERROR_BAD_PARAMETER;

    if (!PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device_StaticFields)[DeviceId].fInitialized)
        return SECY_ERROR_INTERNAL;

    if (Stats_p == NULL)
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
    /* Synchronize with the device if required */
    if (fSync)
    {
        SecY_Rc = SecYLib_Device_Sync(DeviceId);
        if (SecY_Rc != SECY_STATUS_OK)
        {
            LOG_CRIT("%s: synchronization failed for device %d, error %d\n",
                     __func__, DeviceId, SecY_Rc);
            SecYLib_Device_Unlock(DeviceId);
            return SecY_Rc;
        }
    }

    ZEROINIT(Stats);

    LOG_INFO("\n\t EIP164_SecY_Ifc_Stat_I_Get \n");

    Rc = EIP164_SecY_Ifc_Stat_I_Get(&PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->IOArea,
                                    vPort,
                                    &Stats,
                                   ADAPTER_EIP164_WRITE_TO_DECREMENT);
    if (Rc != EIP164_NO_ERROR)
    {
        LOG_CRIT("%s: failed for device %d, error %d\n",
                 __func__, DeviceId, Rc);
        SecYLib_Device_Unlock(DeviceId);
        return SECY_DLIB_ERROR(Rc);
    }

    /* Copy ingress IFC/IFC1 statistics counters */
    SecYLib_DeviceCounter_Copy(&Stats.InOctetsUncontrolled,
                               &Stats_p->InOctetsUncontrolled);
    SecYLib_DeviceCounter_Copy(&Stats.InOctetsControlled,
                               &Stats_p->InOctetsControlled);

    SecYLib_DeviceCounter_Copy(&Stats.InPktsUnicastUncontrolled,
                               &Stats_p->InPktsUnicastUncontrolled);
    SecYLib_DeviceCounter_Copy(&Stats.InPktsMulticastUncontrolled,
                               &Stats_p->InPktsMulticastUncontrolled);
    SecYLib_DeviceCounter_Copy(&Stats.InPktsBroadcastUncontrolled,
                               &Stats_p->InPktsBroadcastUncontrolled);

    SecYLib_DeviceCounter_Copy(&Stats.InPktsUnicastControlled,
                               &Stats_p->InPktsUnicastControlled);
    SecYLib_DeviceCounter_Copy(&Stats.InPktsMulticastControlled,
                               &Stats_p->InPktsMulticastControlled);
    SecYLib_DeviceCounter_Copy(&Stats.InPktsBroadcastControlled,
                               &Stats_p->InPktsBroadcastControlled);

    SecYLib_Device_Unlock(DeviceId);

    LOG_INFO("\n %s done \n", __func__);

    return SECY_STATUS_OK;
}
#endif /* ADAPTER_EIP164_MODE_INGRESS */


#ifdef ADAPTER_EIP164_MODE_INGRESS
/*----------------------------------------------------------------------------
 * SecY_RxCAM_Statistics_Get
 */
SecY_Status_t
SecY_RxCAM_Statistics_Get(
        const unsigned int DeviceId,
        const unsigned int SCIndex,
        SecY_RxCAM_Stat_t * const Stats_p,
        const bool fSync)
{
    EIP164_Error_t Rc;
    SecY_Status_t SecY_Rc;
    EIP164_SecY_RxCAM_Stat_t Stats;

    LOG_INFO("\n %s \n", __func__);

#ifdef ADAPTER_EIP164_STRICT_ARGS
    if (DeviceId >= ADAPTER_EIP164_MAX_NOF_DEVICES)
        return SECY_ERROR_BAD_PARAMETER;

    if (Stats_p == NULL)
        return SECY_ERROR_BAD_PARAMETER;

    if (SCIndex >= PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->SCCount)
        return SECY_ERROR_BAD_PARAMETER;
#endif

    SecYLib_Device_Lock(DeviceId);

    if (!PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device_StaticFields)[DeviceId].fInitialized)
    {
        SecYLib_Device_Unlock(DeviceId);
        return SECY_ERROR_INTERNAL;
    }

    /* Synchronize with the device if required */
    if (fSync)
    {
        SecY_Rc = SecYLib_Device_Sync(DeviceId);
        if (SecY_Rc != SECY_STATUS_OK)
        {
            LOG_CRIT("%s: synchronization failed for device %d, error %d\n",
                     __func__, DeviceId, SecY_Rc);
            SecYLib_Device_Unlock(DeviceId);
            return SecY_Rc;
        }
    }

    ZEROINIT(Stats);

    LOG_INFO("\n\t EIP164_SecY_RxCAM_Stat_Get \n");

    Rc = EIP164_SecY_RxCAM_Stat_Get(&PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->IOArea,
                                    SCIndex,
                                    &Stats,
                                    ADAPTER_EIP164_WRITE_TO_DECREMENT);
    if (Rc != EIP164_NO_ERROR)
    {
        LOG_CRIT("%s: failed for device %d, error %d\n",
                 __func__, DeviceId, Rc);
        SecYLib_Device_Unlock(DeviceId);
        return SECY_DLIB_ERROR(Rc);
    }

    /* Copy RxCAM statistics counter */
    SecYLib_DeviceCounter_Copy(&Stats.CAMHit, &Stats_p->CAMHit);

    SecYLib_Device_Unlock(DeviceId);

    LOG_INFO("\n %s done \n", __func__);

    return SECY_STATUS_OK;
}
#endif /* ADAPTER_EIP164_MODE_INGRESS */


/*----------------------------------------------------------------------------
 * SecY_SA_PnThrSummary_CheckAndClear
 */
SecY_Status_t
SecY_SA_PnThrSummary_CheckAndClear(
        const unsigned int DeviceId,
        unsigned int ** const SAIndexes_pp,
        unsigned int * const NumSAIndexes_p)
{
    EIP164_Error_t Rc;
    uint32_t Summary;
    unsigned int i;
    unsigned int j;
    unsigned int SAIndex;
    unsigned int MaxIndex;
    unsigned int MaxSummaryIndex;
    unsigned int NumTriggered = 0;
    unsigned int *IndexLoc_p;

    LOG_INFO("\n %s \n", __func__);

#ifdef ADAPTER_EIP164_STRICT_ARGS
    if (DeviceId >= ADAPTER_EIP164_MAX_NOF_DEVICES)
        return SECY_ERROR_BAD_PARAMETER;

    if (SAIndexes_pp == NULL)
        return SECY_ERROR_BAD_PARAMETER;

    if (NumSAIndexes_p == NULL)
        return SECY_ERROR_BAD_PARAMETER;
#endif

    SecYLib_Device_Lock(DeviceId);

    if (!PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device_StaticFields)[DeviceId].fInitialized)
    {
        SecYLib_Device_Unlock(DeviceId);
        return SECY_ERROR_INTERNAL;
    }

    *NumSAIndexes_p = 0;
    IndexLoc_p = *SAIndexes_pp;
    MaxIndex = PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->SACount;
    MaxSummaryIndex = (PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->SACount + 31) / 32;

    for (i = 1; i <= MaxSummaryIndex; i++)
    {
        Summary = 0;

        LOG_INFO("\n\t EIP164_Device_SAPnThrSummary_Read \n");

        /* Read summary register */
        Rc = EIP164_Device_SAPnThrSummary_Read(&PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->IOArea,
                                               i,
                                               &Summary);
        if (Rc != EIP164_NO_ERROR)
        {
            SecYLib_Device_Unlock(DeviceId);
            return SECY_DLIB_ERROR(Rc);
        }

        if (Summary != 0)
        {
            LOG_INFO("\n\t EIP164_Device_SAPnThrSummary_Clear \n");

            /* Clear it after read */
            Rc = EIP164_Device_SAPnThrSummary_Clear(&PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->IOArea,
                                                    i,
                                                    Summary);
            if (Rc != EIP164_NO_ERROR)
            {
                SecYLib_Device_Unlock(DeviceId);
                return SECY_DLIB_ERROR(Rc);
            }

            /* return SA handles that crossed the threshold */
            SAIndex = (i - 1) * 32;
            for (j = 0; j < 32; j++)
            {
                if (SAIndex >= MaxIndex)
                {
                    /* All SA's are handled */
                    goto AllHandled;
                }

                if (Summary & (BIT_0 << j))
                {
                    NumTriggered++;
                    *IndexLoc_p = SAIndex;
                    IndexLoc_p++;
                }

                SAIndex++;
            }
        }
    }

AllHandled:
    *NumSAIndexes_p = NumTriggered;

    SecYLib_Device_Unlock(DeviceId);

    LOG_INFO("\n %s done \n", __func__);

    return SECY_STATUS_OK;
}


/*----------------------------------------------------------------------------
 * SecY_SA_ExpiredSummary_CheckAndClear
 */
SecY_Status_t
SecY_SA_ExpiredSummary_CheckAndClear(
        const unsigned int DeviceId,
        unsigned int ** const SAIndexes_pp,
        unsigned int * const NumSAIndexes_p)
{
    EIP164_Error_t Rc;
    uint32_t Summary;
    unsigned int i;
    unsigned int j;
    unsigned int SAIndex;
    unsigned int MaxIndex;
    unsigned int MaxSummaryIndex;
    unsigned int NumTriggered = 0;
    unsigned int *IndexLoc_p;

    LOG_INFO("\n %s \n", __func__);

#ifdef ADAPTER_EIP164_STRICT_ARGS
    if (DeviceId >= ADAPTER_EIP164_MAX_NOF_DEVICES)
        return SECY_ERROR_BAD_PARAMETER;

    if (SAIndexes_pp == NULL)
        return SECY_ERROR_BAD_PARAMETER;

    if (NumSAIndexes_p == NULL)
        return SECY_ERROR_BAD_PARAMETER;
#endif

    SecYLib_Device_Lock(DeviceId);

    if (!PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device_StaticFields)[DeviceId].fInitialized)
    {
        SecYLib_Device_Unlock(DeviceId);
        return SECY_ERROR_INTERNAL;
    }

    *NumSAIndexes_p = 0;
    IndexLoc_p = *SAIndexes_pp;
    MaxIndex = PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->SACount;
    MaxSummaryIndex = (PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->SACount + 31) / 32;

    for (i = 1; i <= MaxSummaryIndex; i++)
    {
        Summary = 0;

        LOG_INFO("\n\t EIP164_Device_SAExpiredSummary_Read \n");

        /* Read summary register */
        Rc = EIP164_Device_SAExpiredSummary_Read(&PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->IOArea,
                                                 i,
                                                 &Summary);
        if (Rc != EIP164_NO_ERROR)
        {
            SecYLib_Device_Unlock(DeviceId);
            return SECY_DLIB_ERROR(Rc);
        }

        if (Summary != 0)
        {
            LOG_INFO("\n\t EIP164_Device_SAExpiredSummary_Clear \n");

            /* Clear it after read */
            Rc = EIP164_Device_SAExpiredSummary_Clear(&PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->IOArea,
                                                      i,
                                                      Summary);
            if (Rc != EIP164_NO_ERROR)
            {
                SecYLib_Device_Unlock(DeviceId);
                return SECY_DLIB_ERROR(Rc);
            }

            /* return SA handles that crossed the threshold */
            SAIndex = (i - 1) * 32;
            for (j = 0; j < 32; j++)
            {
                if (SAIndex >= MaxIndex)
                {
                    /* All SA's are handled */
                    goto AllHandled;
                }

                if (Summary & (BIT_0 << j))
                {
                    NumTriggered++;
                    *IndexLoc_p = SAIndex;
                    IndexLoc_p++;
                }

                SAIndex++;
            }
        }
    }

AllHandled:
    *NumSAIndexes_p = NumTriggered;

    SecYLib_Device_Unlock(DeviceId);

    LOG_INFO("\n %s done \n", __func__);

    return SECY_STATUS_OK;
}


/*----------------------------------------------------------------------------
 * SecY_vPort_Statistics_Clear
 */
SecY_Status_t
SecY_vPort_Statistics_Clear(
        unsigned int DeviceId,
        unsigned int vPort)
{
    return SecYLib_vPort_Statistics_Clear(DeviceId, vPort);
}

/* end of file adapter_secy_stats.c */
