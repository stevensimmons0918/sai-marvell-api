/* adapter_cfye_rule.c
 *
 * CfyE Ruke API implementation
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

/* Adapter internal API */
#include <Integration/Adapter_EIP163/incl/adapter_cfye_support.h>

/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

/* Sleep API */
#include <Integration/Adapter_EIP164/incl/adapter_sleep.h>

/* Logging API */
#include <Kit/Log/incl/log.h>                    /* LOG_* */

/* Driver Framework Run-Time C Library Abstraction API */
#include <Kit/DriverFramework/incl/clib.h>       /* ZEROINIT() */


#ifdef ADAPTER_EIP163_WARMBOOT_ENABLE
#include <Integration/Adapter_EIP163/incl/adapter_cfye_warmboot.h>
#endif

/*----------------------------------------------------------------------------
 * External variables
 *
 */

const CfyE_RuleHandle_t  CfyE_RuleHandle_NULL  = NULL;


/*----------------------------------------------------------------------------
 * CfyELib_Rule_Update
 */
static CfyE_Status_t
CfyELib_Rule_Update(
        const unsigned int DeviceId,
        const unsigned int RuleId,
        const CfyE_RuleHandle_t RuleHandle,
        const CfyE_Rule_t * const Rule_p)
{
    EIP163_Error_t EIP163_Rc;
    EIP163_Rule_t DeviceRule;

    /* Fill in device-specific rule data structure for packet classification */
    ZEROINIT(DeviceRule);

    /* Copy rule key, mask and packet data */
    DeviceRule.Key.NumTags     = Rule_p->Key.NumTags;
    DeviceRule.Key.PacketType  = Rule_p->Key.PacketType;
    DeviceRule.Key.ChannelID   = Rule_p->Key.ChannelID;

    cpssOsMemCpy(DeviceRule.Data,
           Rule_p->Data,
           EIP163_RULE_NON_CTRL_WORD_COUNT * sizeof(uint32_t));

    DeviceRule.Mask.NumTags    = Rule_p->Mask.NumTags;
    DeviceRule.Mask.PacketType = Rule_p->Mask.PacketType;
    DeviceRule.Mask.ChannelID  = Rule_p->Mask.ChannelID;
    cpssOsMemCpy(DeviceRule.DataMask,
           Rule_p->DataMask,
           EIP163_RULE_NON_CTRL_WORD_COUNT * sizeof(uint32_t));

    /* Copy matching rule policy data */
    DeviceRule.Policy.Priority       = Rule_p->Policy.Priority;
    DeviceRule.Policy.vPortId        =
                            CfyELib_vPortId_Get(Rule_p->Policy.vPortHandle);
    DeviceRule.Policy.fDrop          = Rule_p->Policy.fDrop;
    DeviceRule.Policy.fControlPacket = Rule_p->Policy.fControlPacket;

    {
        unsigned int LoopCounter = ADAPTER_EIP163_MAX_NOF_SYNC_RETRY_COUNT;

        IDENTIFIER_NOT_USED_LOG_OFF(RuleHandle);

        for(;;)
        {
            LoopCounter--;

            LOG_INFO("\n\t EIP163_Rule_Update \n");
            /* Add the vPort to the Classification device */
            EIP163_Rc = EIP163_Rule_Update(&PRV_SHARED_MACSEC_CFYE_GLOBAL_VAR_GET(CfyE_Devices)[DeviceId]->IOArea,
                                           RuleId,
                                           &DeviceRule);
            if (EIP163_Rc == EIP163_NO_ERROR)
            {
                break;
            }
            else if (EIP163_Rc != EIP163_BUSY_RETRY_LATER || LoopCounter == 0)
            {
                LOG_CRIT("%s: Failed to update rule %p (id=%d) "
                         "for EIP-163 device %d (%s), error %d\n",
                         __func__,
                         RuleHandle,
                         RuleId,
                         DeviceId,
                         ADAPTER_EIP163_DEVICE_NAME(DeviceId),
                         EIP163_Rc);
                return CFYE_DLIB_ERROR(EIP163_Rc);
            }
            Adapter_SleepMS(ADAPTER_EIP163_SYNC_RETRY_TIMEOUT_MS);
        }
    }

   return CFYE_STATUS_OK;
}


/*----------------------------------------------------------------------------
 * CfyE_Rule_Add
 */
CfyE_Status_t
CfyE_Rule_Add(
        const unsigned int DeviceId,
        const CfyE_vPortHandle_t vPortHandle,
        CfyE_RuleHandle_t * const RuleHandle_p,
        const CfyE_Rule_t * const Rule_p)
{
    unsigned int RuleId;

    LOG_INFO("\n %s \n", __func__);

    ADAPTER_EIP163_CHECK_INT_ATMOST(DeviceId, ADAPTER_EIP163_MAX_NOF_DEVICES - 1);
    ADAPTER_EIP163_CHECK_POINTER(Rule_p);

    if (!CfyELib_vPortHandle_IsValid(vPortHandle))
        return CFYE_ERROR_BAD_PARAMETER;

    CfyELib_Device_Lock(DeviceId);

    if (!CfyELib_Initialized_Check(DeviceId, __func__, true))
    {
        CfyELib_Device_Unlock(DeviceId);
        return CFYE_ERROR_INTERNAL;
    }

    *RuleHandle_p = NULL;

    /* Obtain a free Rule for this classification device */
    {
        List_Element_t * RuleElmt_p;
        CfyE_Rule_Descriptor_Internal_t * RuleDscr_p;
        void * const RFL_p = PRV_SHARED_MACSEC_CFYE_GLOBAL_VAR_GET(CfyE_Devices)[DeviceId]->RuleFreeList_p;

        List_Status_t List_Rc = List_RemoveFromTail(0, RFL_p, &RuleElmt_p);
        if (List_Rc != LIST_STATUS_OK || RuleElmt_p->DataObject_p == NULL)
        {
            LOG_CRIT("%s: Failed to obtain a free Rule for "
                     "EIP-163 device %d (%s), error %d\n",
                     __func__,
                     DeviceId,
                     ADAPTER_EIP163_DEVICE_NAME(DeviceId),
                     List_Rc);
            CfyELib_Device_Unlock(DeviceId);
            return CFYE_ERROR_INTERNAL;
        }

        /* Convert Rule descriptor to Rule index and handle */
        RuleDscr_p     =
                  (CfyE_Rule_Descriptor_Internal_t*)RuleElmt_p->DataObject_p;
        RuleId = RuleDscr_p - PRV_SHARED_MACSEC_CFYE_GLOBAL_VAR_GET(CfyE_Devices)[DeviceId]->RuleDscr_p;
        RuleDscr_p->u.InUse.RuleId = RuleId;
        /* Rule descriptor magic number */
        RuleDscr_p->Magic = ADAPTER_EIP163_RULE_DSCR_MAGIC;
        RuleDscr_p->u.InUse.vPortDscr_p = (CfyE_vPort_Descriptor_Internal_t *)vPortHandle;
        RuleDscr_p->u.InUse.vPortDscr_p->u.InUse.BoundRulesCount++;
        *RuleHandle_p  = RuleDscr_p;
    }

#ifdef ADAPTER_EIP163_WARMBOOT_ENABLE
    CfyELib_WarmBoot_Rule_Set(DeviceId, RuleId);
#endif

    /* Now we have RuleId */
    {
        CfyE_Status_t rc;

        rc = CfyELib_Rule_Update(DeviceId, RuleId, *RuleHandle_p, Rule_p);

        CfyELib_Device_Unlock(DeviceId);

        LOG_INFO("\n %s done \n", __func__);

        return rc;
    }
}


/*-----------------------------------------------------------------------------
 * CfyE_Rule_Add_Index
 */
CfyE_Status_t
CfyE_Rule_Add_Index(
        const unsigned int DeviceId,
        const CfyE_vPortHandle_t vPortHandle,
        CfyE_RuleHandle_t * const RuleHandle_p,
        const CfyE_Rule_t * const Rule_p,
        const unsigned int RuleId)
{
    LOG_INFO("\n %s \n", __func__);

    ADAPTER_EIP163_CHECK_INT_ATMOST(DeviceId, ADAPTER_EIP163_MAX_NOF_DEVICES - 1);
    ADAPTER_EIP163_CHECK_POINTER(Rule_p);

    if (!CfyELib_vPortHandle_IsValid(vPortHandle))
        return CFYE_ERROR_BAD_PARAMETER;

    CfyELib_Device_Lock(DeviceId);

    if (!CfyELib_Initialized_Check(DeviceId, __func__, true))
    {
        CfyELib_Device_Unlock(DeviceId);
        return CFYE_ERROR_INTERNAL;
    }

    if (RuleId >= PRV_SHARED_MACSEC_CFYE_GLOBAL_VAR_GET(CfyE_Devices)[DeviceId]->RulesCount)
    {
        CfyELib_Device_Unlock(DeviceId);
        return CFYE_ERROR_BAD_PARAMETER;
    }

    *RuleHandle_p = NULL;

    /* Obtain a free Rule for this classification device */
    {
        List_Status_t List_Rc;
        List_Element_t * RuleElmt_p;
        CfyE_Rule_Descriptor_Internal_t * RuleDscr_p;
        void * const RFL_p = PRV_SHARED_MACSEC_CFYE_GLOBAL_VAR_GET(CfyE_Devices)[DeviceId]->RuleFreeList_p;

        RuleDscr_p = &PRV_SHARED_MACSEC_CFYE_GLOBAL_VAR_GET(CfyE_Devices)[DeviceId]->RuleDscr_p[RuleId];
        if (RuleDscr_p->Magic == ADAPTER_EIP163_RULE_DSCR_MAGIC)
        {
            /* Rule with desired index already in use. */
            LOG_CRIT("%s: Desired rule index %d already in use for "
                     "EIP-163 device %d (%s)\n",
                     __func__,
                     RuleId,
                     DeviceId,
                     ADAPTER_EIP163_DEVICE_NAME(DeviceId));
            CfyELib_Device_Unlock(DeviceId);
            return CFYE_ERROR_BAD_PARAMETER;
        }

        RuleElmt_p = &RuleDscr_p->u.free;

        List_Rc = List_RemoveAnywhere(0, RFL_p, RuleElmt_p);
        if (List_Rc != LIST_STATUS_OK || RuleElmt_p->DataObject_p == NULL)
        {
            LOG_CRIT("%s: Failed to obtain a free Rule for "
                     "EIP-163 device %d (%s), error %d\n",
                     __func__,
                     DeviceId,
                     ADAPTER_EIP163_DEVICE_NAME(DeviceId),
                     List_Rc);
            CfyELib_Device_Unlock(DeviceId);
            return CFYE_ERROR_INTERNAL;
        }

        /* Convert Rule descriptor to Rule index and handle */
        RuleDscr_p->u.InUse.RuleId = RuleId;
        /* Rule descriptor magic number */
        RuleDscr_p->Magic = ADAPTER_EIP163_RULE_DSCR_MAGIC;
        RuleDscr_p->u.InUse.vPortDscr_p = (CfyE_vPort_Descriptor_Internal_t *)vPortHandle;
        RuleDscr_p->u.InUse.vPortDscr_p->u.InUse.BoundRulesCount++;
        *RuleHandle_p  = RuleDscr_p;
    }

#ifdef ADAPTER_EIP163_WARMBOOT_ENABLE
    CfyELib_WarmBoot_Rule_Set(DeviceId, RuleId);
#endif

    /* Now we have RuleId */
    {
        CfyE_Status_t rc;

        rc = CfyELib_Rule_Update(DeviceId, RuleId, *RuleHandle_p, Rule_p);

        CfyELib_Device_Unlock(DeviceId);

        LOG_INFO("\n %s done \n", __func__);

        return rc;
    }
}


/*----------------------------------------------------------------------------
 * CfyE_Rule_Remove
 */
CfyE_Status_t
CfyE_Rule_Remove(
        const unsigned int DeviceId,
        const CfyE_RuleHandle_t RuleHandle)
{
    EIP163_Error_t EIP163_Rc;
    unsigned int RuleId;
    unsigned int boundRulesCount;

    LOG_INFO("\n %s \n", __func__);

    ADAPTER_EIP163_CHECK_INT_ATMOST(DeviceId, ADAPTER_EIP163_MAX_NOF_DEVICES - 1);

    if (!CfyELib_RuleHandle_IsValid(RuleHandle))
        return CFYE_ERROR_BAD_PARAMETER;

    CfyELib_Device_Lock(DeviceId);

    if (!CfyELib_Initialized_Check(DeviceId, __func__, true))
    {
        CfyELib_Device_Unlock(DeviceId);
        return CFYE_ERROR_INTERNAL;
    }

    RuleId = CfyELib_RuleId_Get(RuleHandle);

    if (RuleId >= PRV_SHARED_MACSEC_CFYE_GLOBAL_VAR_GET(CfyE_Devices)[DeviceId]->RulesCount)
    {
            CfyELib_Device_Unlock(DeviceId);
            return CFYE_ERROR_INTERNAL;
    }
#ifdef ADAPTER_EIP163_WARMBOOT_ENABLE
    CfyELib_WarmBoot_Rule_Clear(DeviceId, RuleId);
#endif


    {
        unsigned int LoopCounter = ADAPTER_EIP163_MAX_NOF_SYNC_RETRY_COUNT;

         for(;;)
        {
            LoopCounter--;

            LOG_INFO("\n\t EIP163_Rule_Remove \n");
            EIP163_Rc = EIP163_Rule_Remove(&PRV_SHARED_MACSEC_CFYE_GLOBAL_VAR_GET(CfyE_Devices)[DeviceId]->IOArea,
                                           RuleId);
            if (EIP163_Rc == EIP163_NO_ERROR)
            {
                break;
            }
            else if (EIP163_Rc != EIP163_BUSY_RETRY_LATER || LoopCounter == 0)
            {
                LOG_CRIT("%s: Failed to remove Rule %p (id=%d) "
                         "from EIP-163 device %d (%s), error %d\n",
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
    /* Add removed Rule to the Rule free list */
    {
        List_Status_t List_Rc;
        void * const RFL_p = PRV_SHARED_MACSEC_CFYE_GLOBAL_VAR_GET(CfyE_Devices)[DeviceId]->RuleFreeList_p;
        List_Element_t * const RuleElmt_p =
                            &PRV_SHARED_MACSEC_CFYE_GLOBAL_VAR_GET(CfyE_Devices)[DeviceId]->RuleDscr_p[RuleId].u.free;

        /* Invalidate magic number. */
        PRV_SHARED_MACSEC_CFYE_GLOBAL_VAR_SET(CfyE_Devices[DeviceId]->RuleDscr_p[RuleId].Magic,0);

        boundRulesCount = PRV_SHARED_MACSEC_CFYE_GLOBAL_VAR_GET(CfyE_Devices)[DeviceId]->RuleDscr_p[RuleId].u.InUse.vPortDscr_p->u.InUse.BoundRulesCount;
        PRV_SHARED_MACSEC_CFYE_GLOBAL_VAR_SET(CfyE_Devices[DeviceId]->RuleDscr_p[RuleId].u.InUse.vPortDscr_p->u.InUse.BoundRulesCount,(--boundRulesCount));

        RuleElmt_p->DataObject_p = &PRV_SHARED_MACSEC_CFYE_GLOBAL_VAR_GET(CfyE_Devices)[DeviceId]->RuleDscr_p[RuleId];

        List_Rc = List_AddToHead(0, RFL_p, RuleElmt_p);
        if (List_Rc != LIST_STATUS_OK)
        {
            LOG_CRIT("%s: Failed to add Rule %p (id=%d) to free list for "
                     "EIP-163 device %d (%s)\n",
                     __func__,
                     RuleHandle,
                     RuleId,
                     DeviceId,
                     ADAPTER_EIP163_DEVICE_NAME(DeviceId));
            CfyELib_Device_Unlock(DeviceId);
            return CFYE_ERROR_INTERNAL;
        }
    }

    CfyELib_Device_Unlock(DeviceId);

    LOG_INFO("\n %s done \n", __func__);

    return CFYE_STATUS_OK;
}


/*----------------------------------------------------------------------------
 * CfyE_Rule_Update
 */
CfyE_Status_t
CfyE_Rule_Update(
        const unsigned int DeviceId,
        const CfyE_RuleHandle_t RuleHandle,
        const CfyE_Rule_t * const Rule_p)
{
    unsigned int RuleId;

    LOG_INFO("\n %s \n", __func__);

    ADAPTER_EIP163_CHECK_INT_ATMOST(DeviceId, ADAPTER_EIP163_MAX_NOF_DEVICES - 1);
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
        CfyE_Status_t rc;

        rc = CfyELib_Rule_Update(DeviceId, RuleId, RuleHandle, Rule_p);

        CfyELib_Device_Unlock(DeviceId);

        LOG_INFO("\n %s done \n", __func__);

        return rc;
    }
}


/*----------------------------------------------------------------------------
 * CfyE_Rule_Enable
 */
CfyE_Status_t
CfyE_Rule_Enable(
        const unsigned int DeviceId,
        const CfyE_RuleHandle_t RuleHandle,
        const bool fSync)
{
    EIP163_Error_t EIP163_Rc;
    CfyE_Status_t Rc;
    unsigned int RuleId;

    LOG_INFO("\n %s \n", __func__);

    ADAPTER_EIP163_CHECK_INT_ATMOST(DeviceId, ADAPTER_EIP163_MAX_NOF_DEVICES - 1);

    if (!CfyELib_RuleHandle_IsValid(RuleHandle))
        return CFYE_ERROR_BAD_PARAMETER;

    CfyELib_Device_Lock(DeviceId);

    if (!CfyELib_Initialized_Check(DeviceId, __func__, true))
    {
        CfyELib_Device_Unlock(DeviceId);
        return CFYE_ERROR_INTERNAL;
    }

    /* Synchronize with the device if required */
    if (fSync)
    {
        Rc = CfyELib_Device_Sync(DeviceId);
        if (Rc != CFYE_STATUS_OK)
        {
            LOG_CRIT("%s: synchronization failed for device %d, error %d\n",
                     __func__, DeviceId, Rc);
            CfyELib_Device_Unlock(DeviceId);
            return Rc;
        }
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

    LOG_INFO("\n\t EIP163_Rule_Enable \n");

    EIP163_Rc = EIP163_Rule_Enable(&PRV_SHARED_MACSEC_CFYE_GLOBAL_VAR_GET(CfyE_Devices)[DeviceId]->IOArea, RuleId);
    if (EIP163_Rc != EIP163_NO_ERROR)
    {
        LOG_CRIT("%s: Failed to enable rule %p (id=%d) "
                 "for EIP-163 device %d (%s), error %d\n",
                 __func__,
                 RuleHandle,
                 RuleId,
                 DeviceId,
                 ADAPTER_EIP163_DEVICE_NAME(DeviceId),
                 EIP163_Rc);
    }

    CfyELib_Device_Unlock(DeviceId);

    LOG_INFO("\n %s done \n", __func__);

    if (EIP163_Rc == EIP163_NO_ERROR)
    {
        return CFYE_STATUS_OK;
    }
    else
    {
        return CFYE_DLIB_ERROR(EIP163_Rc);
    }
}


/*----------------------------------------------------------------------------
 * CfyE_Rule_Disable
 */
CfyE_Status_t
CfyE_Rule_Disable(
        const unsigned int DeviceId,
        const CfyE_RuleHandle_t RuleHandle,
        const bool fSync)
{
    EIP163_Error_t EIP163_Rc;
    CfyE_Status_t Rc;
    unsigned int RuleId;

    LOG_INFO("\n %s \n", __func__);

    ADAPTER_EIP163_CHECK_INT_ATMOST(DeviceId, ADAPTER_EIP163_MAX_NOF_DEVICES - 1);

    if (!CfyELib_RuleHandle_IsValid(RuleHandle))
        return CFYE_ERROR_BAD_PARAMETER;

    CfyELib_Device_Lock(DeviceId);

    if (!CfyELib_Initialized_Check(DeviceId, __func__, true))
    {
        CfyELib_Device_Unlock(DeviceId);
        return CFYE_ERROR_INTERNAL;
    }

    /* Synchronize with the device if required */
    if (fSync)
    {
        Rc = CfyELib_Device_Sync(DeviceId);
        if (Rc != CFYE_STATUS_OK)
        {
            LOG_CRIT("%s: synchronization failed for device %d, error %d\n",
                     __func__, DeviceId, Rc);
            CfyELib_Device_Unlock(DeviceId);
            return Rc;
        }
    }

    RuleId = CfyELib_RuleId_Get(RuleHandle);

    if (RuleId >= PRV_SHARED_MACSEC_CFYE_GLOBAL_VAR_GET(CfyE_Devices)[DeviceId]->RulesCount)
    {
            CfyELib_Device_Unlock(DeviceId);
            return CFYE_ERROR_INTERNAL;
    }

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
        unsigned int LoopCounter = ADAPTER_EIP163_MAX_NOF_SYNC_RETRY_COUNT;

        for(;;)
        {
            LoopCounter--;
            LOG_INFO("\n\t EIP163_Rule_Disable \n");

            EIP163_Rc = EIP163_Rule_Disable(&PRV_SHARED_MACSEC_CFYE_GLOBAL_VAR_GET(CfyE_Devices)[DeviceId]->IOArea, RuleId);
            if (EIP163_Rc == EIP163_NO_ERROR)
            {
                break;
            }
            else if (EIP163_Rc != EIP163_BUSY_RETRY_LATER || LoopCounter == 0)
            {
                LOG_CRIT("%s: Failed to disable rule %p (id=%d) "
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

    CfyELib_Device_Unlock(DeviceId);

    LOG_INFO("\n %s done \n", __func__);

    return CFYE_STATUS_OK;
}


/*----------------------------------------------------------------------------
 * CfyE_Rule_EnableDisable
 */
CfyE_Status_t
CfyE_Rule_EnableDisable(
        const unsigned int DeviceId,
        const CfyE_RuleHandle_t RuleHandleEnable,
        const CfyE_RuleHandle_t RuleHandleDisable,
        const bool EnableAll,
        const bool DisableAll,
        const bool fSync)
{
    EIP163_Error_t EIP163_Rc;
    CfyE_Status_t Rc;
    unsigned int RuleIdEnable = 0, RuleIdDisable = 0;
    bool fEnable = false, fDisable = false;

    LOG_INFO("\n %s \n", __func__);

    ADAPTER_EIP163_CHECK_INT_ATMOST(DeviceId, ADAPTER_EIP163_MAX_NOF_DEVICES - 1);

    CfyELib_Device_Lock(DeviceId);

    if (!CfyELib_Initialized_Check(DeviceId, __func__, true))
    {
        CfyELib_Device_Unlock(DeviceId);
        return CFYE_ERROR_INTERNAL;
    }

    /* Synchronize with the device if required */
    if (fSync)
    {
        Rc = CfyELib_Device_Sync(DeviceId);
        if (Rc != CFYE_STATUS_OK)
        {
            LOG_CRIT("%s: synchronization failed for device %d, error %d\n",
                     __func__, DeviceId, Rc);
            CfyELib_Device_Unlock(DeviceId);
            return Rc;
        }
    }

    {
        unsigned int LoopCounter = ADAPTER_EIP163_MAX_NOF_SYNC_RETRY_COUNT;

        for(;;)
        {
            LoopCounter--;

            if (EnableAll)
            {
                LOG_INFO("\n\t EIP163_Rule_EnableDisable \n");

                EIP163_Rc = EIP163_Rule_EnableDisable(&PRV_SHARED_MACSEC_CFYE_GLOBAL_VAR_GET(CfyE_Devices)[DeviceId]->IOArea,
                                                      0,
                                                      0,
                                                      fEnable,
                                                      fDisable,
                                                      true,
                                                      false);
            }
            else if (DisableAll)
            {
                LOG_INFO("\n\t EIP163_Rule_EnableDisable \n");

                EIP163_Rc = EIP163_Rule_EnableDisable(&PRV_SHARED_MACSEC_CFYE_GLOBAL_VAR_GET(CfyE_Devices)[DeviceId]->IOArea,
                                                      0,
                                                      0,
                                                      fEnable,
                                                      fDisable,
                                                      false,
                                                      true);
            }
            else
            {
                fEnable = !CfyELib_RuleHandle_IsSame(RuleHandleEnable,
                                                     CfyE_RuleHandle_NULL);
                if (fEnable)
                {
                    if (!CfyELib_RuleHandle_IsValid(RuleHandleEnable))
                    {
                        CfyELib_Device_Unlock(DeviceId);
                        return CFYE_ERROR_BAD_PARAMETER;
                    }

                    RuleIdEnable  = CfyELib_RuleId_Get(RuleHandleEnable);
                }

                if (PRV_SHARED_MACSEC_CFYE_GLOBAL_VAR_GET(CfyE_Devices)[DeviceId]->RuleDscr_p[RuleIdEnable].u.InUse.vPortDscr_p == NULL)
                {
                    LOG_CRIT("%s: Failed, missing vPort for rule %p (id=%d) "
                             "for EIP-163 device %d (%s)\n",
                             __func__,
                             RuleHandleEnable,
                             RuleIdEnable,
                             DeviceId,
                             ADAPTER_EIP163_DEVICE_NAME(DeviceId));
                    CfyELib_Device_Unlock(DeviceId);
                    return CFYE_ERROR_INTERNAL;
                }

                fDisable = !CfyELib_RuleHandle_IsSame(RuleHandleDisable,
                                                      CfyE_RuleHandle_NULL);
                if (fDisable)
                {
                    if (!CfyELib_RuleHandle_IsValid(RuleHandleDisable))
                    {
                        CfyELib_Device_Unlock(DeviceId);
                        return CFYE_ERROR_BAD_PARAMETER;
                    }

                    RuleIdDisable = CfyELib_RuleId_Get(RuleHandleDisable);
                }

                if (PRV_SHARED_MACSEC_CFYE_GLOBAL_VAR_GET(CfyE_Devices)[DeviceId]->RuleDscr_p[RuleIdDisable].u.InUse.vPortDscr_p == NULL)
                {
                    LOG_CRIT("%s: Failed, missing vPort for rule %p (id=%d) "
                             "for EIP-163 device %d (%s)\n",
                             __func__,
                             RuleHandleDisable,
                             RuleIdDisable,
                             DeviceId,
                             ADAPTER_EIP163_DEVICE_NAME(DeviceId));
                    CfyELib_Device_Unlock(DeviceId);
                    return CFYE_ERROR_INTERNAL;
                }

                LOG_INFO("\n\t EIP163_Rule_EnableDisable \n");

                EIP163_Rc = EIP163_Rule_EnableDisable(&PRV_SHARED_MACSEC_CFYE_GLOBAL_VAR_GET(CfyE_Devices)[DeviceId]->IOArea,
                                                      RuleIdEnable,
                                                      RuleIdDisable,
                                                      fEnable,
                                                      fDisable,
                                                      false,
                                                      false);
            }

            if (EIP163_Rc == EIP163_NO_ERROR)
            {
                break;
            }
            else if (EIP163_Rc != EIP163_BUSY_RETRY_LATER || LoopCounter == 0)
            {
                LOG_CRIT("%s: Failed to enable/disable rule %p/%p "
                         "(id %d/%d, enable/disable %d/%d) "
                         "for EIP-163 device %d (%s), error %d\n",
                 __func__,
                         RuleHandleEnable,
                         RuleHandleDisable,
                         RuleIdEnable,
                         RuleIdDisable,
                         fEnable,
                         fDisable,
                         DeviceId,
                         ADAPTER_EIP163_DEVICE_NAME(DeviceId),
                         EIP163_Rc);
                CfyELib_Device_Unlock(DeviceId);
                return CFYE_ERROR_INTERNAL;
            }
            Adapter_SleepMS(ADAPTER_EIP163_SYNC_RETRY_TIMEOUT_MS);
        }
    }
    CfyELib_Device_Unlock(DeviceId);

    LOG_INFO("\n %s done \n", __func__);

    return CFYE_STATUS_OK;
}


/*----------------------------------------------------------------------------
 * CfyE_RuleHandle_IsSame
 */
bool
CfyE_RuleHandle_IsSame(
        const CfyE_RuleHandle_t Handle1_p,
        const CfyE_RuleHandle_t Handle2_p)
{
    return CfyELib_RuleHandle_IsSame(Handle1_p, Handle2_p);
}


/*-----------------------------------------------------------------------------
 * CfyE_RuleIndex_Get
 */
CfyE_Status_t
CfyE_RuleIndex_Get(
        const CfyE_RuleHandle_t RuleHandle,
        unsigned int * const RuleIndex_p)
{
    if (!CfyELib_RuleHandle_IsValid(RuleHandle))
        return CFYE_ERROR_BAD_PARAMETER;

    ADAPTER_EIP163_CHECK_POINTER(RuleIndex_p);

    *RuleIndex_p = CfyELib_RuleId_Get(RuleHandle);

    return CFYE_STATUS_OK;
}


/*-----------------------------------------------------------------------------
 * CfyE_RuleHandle_Get
 */
CfyE_Status_t
CfyE_RuleHandle_Get(
        const unsigned int DeviceId,
        const unsigned int RuleIndex,
        CfyE_RuleHandle_t * const RuleHandle_p)
{
    CfyE_Status_t CfyE_Rc = CFYE_STATUS_OK;
    ADAPTER_EIP163_CHECK_POINTER(RuleHandle_p);

    CfyELib_Device_Lock(DeviceId);

    if (!CfyELib_Initialized_Check(DeviceId, __func__, true))
    {
        CfyELib_Device_Unlock(DeviceId);
        return CFYE_ERROR_INTERNAL;
    }

    if(!CfyELib_RuleIndexToHandle(DeviceId,
                                  RuleIndex,
                                  RuleHandle_p))
    {
        CfyE_Rc = CFYE_ERROR_INTERNAL;
    }

    CfyELib_Device_Unlock(DeviceId);
    return CfyE_Rc;
}


/* end of file adapter_cfye_rule.c */
