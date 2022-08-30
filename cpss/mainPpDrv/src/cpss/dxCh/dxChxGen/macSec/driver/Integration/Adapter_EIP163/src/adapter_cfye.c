/* adapter_cfye.c
 *
 * CfyE API and Adapter internal CfyE API implementation
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

/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

/* Internal adapter API */
#include <Integration/Adapter_EIP163/incl/adapter_cfye_support.h>   /* Typedefs */

/* Logging API */
#include <Kit/Log/incl/log.h>                    /* LOG_* */

/* Driver Framework Device API */
#include <Kit/DriverFramework/incl/device_types.h>
#include <Kit/DriverFramework/incl/device_mgmt.h>

/* Driver Framework Run-Time C Library Abstraction API */
#include <Kit/DriverFramework/incl/clib.h>      /* ZEROINIT() */

/* Adapter Alloc API */
#include <Integration/Adapter_EIP164/incl/adapter_alloc.h>      /* Adapter_Alloc()/_Free() */

/* Adapter Sleep API */
#include <Integration/Adapter_EIP164/incl/adapter_sleep.h>      /* Adapter_SleepMS() */

#ifdef ADAPTER_EIP163_INTERRUPTS_ENABLE
/* Adapter Interrupts API */
#include <Integration/Adapter_EIP164/incl/adapter_interrupts.h>
#endif

/* WarmBoot internal API */
#ifdef ADAPTER_EIP163_WARMBOOT_ENABLE
#include <Integration/Adapter_EIP163/incl/adapter_cfye_warmboot.h>
#endif

/*----------------------------------------------------------------------------
 * Definitions and macros
 */

#define EIP163_ALL_CLOCKS ( EIP163_DEVICE_ICE_CLOCK | \
                            EIP163_DEVICE_TCAM_CLOCK | \
                            EIP163_DEVICE_ICE_DATA_CLOCK | \
                            EIP163_DEVICE_STATS_CLOCK)

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
 * CfyELib_DeviceRole_IsValid
 */
static inline bool
CfyELib_DeviceRole_IsValid(
        const CfyE_Role_t Role)
{
    IDENTIFIER_NOT_USED(Role);
    return true;
}


/*----------------------------------------------------------------------------
 * CfyELib_DataObjectList_Init
 *
 * Initialize a data object free list
 */
static bool
CfyELib_DataObjectList_Init(
        const unsigned int DeviceId,
        void ** const ObjectFreeList_pp,
        const bool fRuleList,
        void ** const ObjectDscr_pp,
        const unsigned int ObjectCount)
{
    unsigned int i;
    List_Status_t List_Rc;
    void * FL_p;
    void * ObjectDscr_p;
    size_t ObjectDscrByteCount;
    const char *ObjectName_p;
    List_Element_t * Elmt_p;
    char *p;

    if (fRuleList)
    {
        ObjectName_p = "Rule";
        ObjectDscrByteCount = sizeof(CfyE_Rule_Descriptor_Internal_t);
    }
    else
    {
        ObjectName_p = "vPort";
        ObjectDscrByteCount = sizeof(CfyE_vPort_Descriptor_Internal_t);
    }
    IDENTIFIER_NOT_USED_LOG_OFF(ObjectName_p);
    IDENTIFIER_NOT_USED_LOG_OFF(DeviceId);

    FL_p = Adapter_Alloc(List_GetInstanceByteCount());
    if (!FL_p)
    {
        LOG_CRIT("%s: Failed to allocate %s free list "
                 "for EIP-163 device %d\n",
                 __func__,
                 ObjectName_p,
                 DeviceId);
        return false;
    }

    List_Rc = List_Init(0, FL_p);
    if (List_Rc != LIST_STATUS_OK)
    {
        LOG_CRIT("%s: Failed to initialize %s free list "
                 "for EIP-163 device %d\n",
                 __func__,
                 ObjectName_p,
                 DeviceId);
        Adapter_Free(FL_p);
        return false; /* error */
    }

    /* Allocate data object descriptors */
    ObjectDscr_p = Adapter_Alloc(ObjectCount * ObjectDscrByteCount);
    if (ObjectDscr_p == NULL)
    {
        LOG_CRIT("%s: Failed to allocate %s descriptors "
                 "for EIP-163 device for device %d\n",
                 __func__,
                 ObjectName_p,
                 DeviceId);
        List_Uninit(0, FL_p);
        Adapter_Free(FL_p);
        return false; /* error */
    }

    p = (char*)ObjectDscr_p;
    /* Initialize data objects */
    for (i = 0; i < ObjectCount; i++)
    {
        if (fRuleList)
        {
            ((CfyE_Rule_Descriptor_Internal_t*)(void*)p)->Magic = 0;

            Elmt_p = &((CfyE_Rule_Descriptor_Internal_t*)(void*)p)->u.free;
        }
        else
        {
            ((CfyE_vPort_Descriptor_Internal_t*)(void*)p)->Magic = 0;
            Elmt_p = &((CfyE_vPort_Descriptor_Internal_t*)(void*)p)->u.free;
        }

        /* Link the object descriptor to the element */
        Elmt_p->DataObject_p = p;

        /* Add the element to the Rule free list */
        List_Rc = List_AddToHead(0, FL_p, Elmt_p);
        if (List_Rc != LIST_STATUS_OK)
        {
            LOG_CRIT("%s: Failed to add element to %s free list "
                     "for EIP-163 device %d\n",
                     __func__,
                     ObjectName_p,
                     DeviceId);
            Adapter_Free(ObjectDscr_p);
            List_Uninit(0, FL_p);
            Adapter_Free(FL_p);
            return false; /* error */
        }

        p += ObjectDscrByteCount;
    } /* for */

    *ObjectFreeList_pp  = FL_p;
    *ObjectDscr_pp      = ObjectDscr_p;

    return true;
}


/*----------------------------------------------------------------------------
 * CfyE_Device_Init
 */
CfyE_Status_t
CfyE_Device_Init(
        const unsigned int DeviceId,
        const CfyE_Role_t Role,
        const CfyE_Init_t * const Init_p)
{
    Device_Handle_t Device;
    EIP163_Error_t Rc;
    EIP163_Settings_t Settings;
    EIP163_Capabilities_t Cp;

    LOG_INFO("\n %s \n", __func__);

    ADAPTER_EIP163_CHECK_INT_ATMOST(DeviceId, ADAPTER_EIP163_MAX_NOF_DEVICES - 1);
    ADAPTER_EIP163_CHECK_POINTER(Init_p);

    if (!CfyELib_DeviceRole_IsValid(Role))
        return CFYE_ERROR_BAD_PARAMETER;

    CfyELib_Device_Lock(DeviceId);

    /* Check if the device has been already initialized */
    if (CfyELib_Initialized_Check(DeviceId, __func__, false))
    {
        CfyELib_Device_Unlock(DeviceId);
        return CFYE_ERROR_INTERNAL;
    }

    ADAPTER_EIP163_DEVICE_NAME(DeviceId) =
                    Device_GetName(ADAPTER_EIP163_DEV_ID(DeviceId));

    /* Find the EIP-163 device */
    Device = Device_Find(ADAPTER_EIP163_DEVICE_NAME(DeviceId));
    if (Device == NULL)
    {
        LOG_CRIT("%s: Failed to locate EIP-163 device %d (%s)\n",
                 __func__,
                 DeviceId,
                 ADAPTER_EIP163_DEVICE_NAME(DeviceId));
        CfyELib_Device_Unlock(DeviceId);
        return CFYE_ERROR_INTERNAL;
    }

    PRV_SHARED_MACSEC_CFYE_GLOBAL_VAR_SET(CfyE_Devices[DeviceId],Adapter_Alloc(sizeof(CfyE_Device_Internal_t)));
    if (PRV_SHARED_MACSEC_CFYE_GLOBAL_VAR_GET(CfyE_Devices)[DeviceId] == NULL)
    {
        LOG_CRIT("%s: Failed to allocate descriptor for %d (%s)\n",
                 __func__,
                 DeviceId,
                 ADAPTER_EIP163_DEVICE_NAME(DeviceId));
        CfyELib_Device_Unlock(DeviceId);
        return CFYE_ERROR_INTERNAL;
    }

    ZEROINIT(Settings);

    if (Role == CFYE_ROLE_EGRESS)
        Settings.Mode = EIP163_MODE_EGRESS;
    else
        Settings.Mode = EIP163_MODE_INGRESS;

    Settings.CountFrameThrLo             = Init_p->CountFrameThrLo;
    Settings.CountFrameThrHi             = Init_p->CountFrameThrHi;
    Settings.ChanCountFrameThrLo         = Init_p->ChanCountFrameThrLo;
    Settings.ChanCountFrameThrHi         = Init_p->ChanCountFrameThrHi;
    Settings.CountIncDisCtrl             = Init_p->CountIncDisCtrl;
    Settings.EOPTimeoutVal               = Init_p->EOPTimeoutVal;
    cpssOsMemCpy(&Settings.EOPTimeoutCtrl, &Init_p->EOPTimeoutCtrl, sizeof(CfyE_Ch_Mask_t));

    Settings.ECCCorrectableThr           = Init_p->ECCCorrectableThr;
    Settings.ECCUncorrectableThr         = Init_p->ECCUncorrectableThr;

    LOG_CRIT("%s device statistics counter increment enable settings:\n"
             "\tTCAM counters: %s\n"
             "\tChannel counters: %s\n",
             ADAPTER_EIP163_DEVICE_NAME(DeviceId),
             (Init_p->CountIncDisCtrl & CFYE_TCAM_COUNT_INC_DIS) ?
                     "Disabled" : "Enabled",
             (Init_p->CountIncDisCtrl & CFYE_CHAN_COUNT_INC_DIS) ?
                     "Disabled" : "Enabled");

    Settings.fAutoStatCntrsReset         = ADAPTER_EIP163_CLEAR_ON_READ;
    Settings.fLowLatencyBypass           = Init_p->fLowLatencyBypass;
    Settings.TCAMInit.InitData_p         = Init_p->InputTCAM_p;
    Settings.TCAMInit.WordOffset         = Init_p->WordOffset;
    Settings.TCAMInit.InitData_WordCount = Init_p->InputTCAM_WordCount;

    LOG_INFO("\n\t EIP163_Device_Init \n");

    /* Request the device initialization is done */
    Rc = EIP163_Device_Init(&PRV_SHARED_MACSEC_CFYE_GLOBAL_VAR_GET(CfyE_Devices)[DeviceId]->IOArea,
                            Device,
                            &Settings);
    if (Rc != EIP163_NO_ERROR && Rc != EIP163_BUSY_RETRY_LATER)
    {
        LOG_CRIT("%s: Failed to init EIP-163 device %d (%s), "
                 "mode %s, error %d\n",
                 __func__,
                 DeviceId,
                 ADAPTER_EIP163_DEVICE_NAME(DeviceId),
                 (Settings.Mode == EIP163_MODE_EGRESS) ? "egress" : "ingress",
                 Rc);
        Adapter_Free(PRV_SHARED_MACSEC_CFYE_GLOBAL_VAR_GET(CfyE_Devices)[DeviceId]);
        PRV_SHARED_MACSEC_CFYE_GLOBAL_VAR_SET(CfyE_Devices[DeviceId],NULL);
        CfyELib_Device_Unlock(DeviceId);
        return CFYE_DLIB_ERROR(Rc);
    }

    /* Wait till the CfyE device initialization is done */
    if (Rc == EIP163_BUSY_RETRY_LATER)
    {
        unsigned int LoopCounter = ADAPTER_EIP163_MAX_NOF_INIT_RETRY_COUNT;

        do
        {
            LOG_INFO("\n\t EIP163_Device_Is_Done \n");

            Rc = EIP163_Device_Is_Done(&PRV_SHARED_MACSEC_CFYE_GLOBAL_VAR_GET(CfyE_Devices)[DeviceId]->IOArea);
            if (Rc != EIP163_BUSY_RETRY_LATER)
                break;

            Adapter_SleepMS(ADAPTER_EIP163_INIT_RETRY_TIMEOUT_MS);
        } while (--LoopCounter);

        if (Rc != EIP163_NO_ERROR)
        {
            LOG_CRIT("%s: Failed to complete EIP-163 device %d initialization\n",
                     __func__,
                     DeviceId);
            CfyELib_Device_Unlock(DeviceId);
            Adapter_Free(PRV_SHARED_MACSEC_CFYE_GLOBAL_VAR_GET(CfyE_Devices)[DeviceId]);
            PRV_SHARED_MACSEC_CFYE_GLOBAL_VAR_SET(CfyE_Devices[DeviceId],NULL);
            return CFYE_DLIB_ERROR(Rc);
        }
    }

    /* Retrieve device capabilities */
    ZEROINIT(Cp);

    LOG_INFO("\n\t EIP163_HWRevision_Get \n");

    Rc = EIP163_HWRevision_Get(Device, &Cp);
    if (Rc != EIP163_NO_ERROR)
    {
        LOG_CRIT("%s: Failed to get EIP-163 device %d capabilities, error %d\n",
                 __func__,
                 DeviceId,
                 Rc);
        Adapter_Free(PRV_SHARED_MACSEC_CFYE_GLOBAL_VAR_GET(CfyE_Devices)[DeviceId]);
        PRV_SHARED_MACSEC_CFYE_GLOBAL_VAR_SET(CfyE_Devices[DeviceId],NULL);
        CfyELib_Device_Unlock(DeviceId);
        return CFYE_DLIB_ERROR(Rc);
    }

    /* Show those capabilities not propagated to higher layer. */
    LOG_CRIT("%s device version major/minor/patch: %d/%d/%d\n",
              ADAPTER_EIP163_DEVICE_NAME(DeviceId),
              Cp.EIP163_Version.MajHWRevision,
              Cp.EIP163_Version.MinHWRevision,
              Cp.EIP163_Version.HWPatchLevel);

    LOG_CRIT("%s device capabilities\n", ADAPTER_EIP163_DEVICE_NAME(DeviceId));

    LOG_CRIT("\tEgress only:               %s\n"
             "\tIngress only:              %s\n"
             "\tExternal TCAM:             %s\n"
             "\tNof channels:              %d\n"
             "\tNof rules:                 %d\n"
             "\tNof vPorts:                %d\n\n",
             YesNo(Cp.EIP163_Options.fEgressOnly),
             YesNo(Cp.EIP163_Options.fIngressOnly),
             YesNo(Cp.EIP163_Options.fExternalTCAM),
             Cp.EIP163_Options.Channel_Count,
             Cp.EIP163_Options.Rules_Count,
             Cp.EIP163_Options.vPort_Count);

    LOG_CRIT("%s device (EIP-217) version major/minor/patch: %d/%d/%d\n",
              ADAPTER_EIP163_DEVICE_NAME(DeviceId),
              Cp.TCAM_Version.MajHWRevision,
              Cp.TCAM_Version.MinHWRevision,
              Cp.TCAM_Version.HWPatchLevel);

    LOG_CRIT("%s device (EIP-217) capabilities\n",
             ADAPTER_EIP163_DEVICE_NAME(DeviceId));

    LOG_CRIT("\tNof TCAM hit counters:            %d\n"
             "\tTCAM hit counter size (bits):     %d\n"
             "\tNof TCAM packet counters:         %d\n",
             Cp.TCAM_Options.TCAMHitCounters_Count,
             Cp.TCAM_Options.TCAMHitCountersWidth_BitCount,
             Cp.TCAM_Options.TCAMHitPktCounters_Count);

#ifndef ADAPTER_EIP163_DEV_INIT_DISABLE
    /* These clock control functions can be used to fine-grained */
    /* clock control, for now only OFF/ON sequence is done */

    LOG_INFO("\n\t EIP163_Device_Clocks_Off \n");

    /* Clock control, disable all EIP-163 clock signals */
    Rc = EIP163_Device_Clocks_Off(&PRV_SHARED_MACSEC_CFYE_GLOBAL_VAR_GET(CfyE_Devices)[DeviceId]->IOArea,
                             EIP163_ALL_CLOCKS);
    if (Rc != EIP163_NO_ERROR)
    {
        Adapter_Free(PRV_SHARED_MACSEC_CFYE_GLOBAL_VAR_GET(CfyE_Devices)[DeviceId]);
        PRV_SHARED_MACSEC_CFYE_GLOBAL_VAR_SET(CfyE_Devices[DeviceId],NULL);
        CfyELib_Device_Unlock(DeviceId);
        return CFYE_DLIB_ERROR(Rc);
    }
    LOG_INFO("\n\t EIP163_Device_Clocks_Off \n");

    /* Clock control, restore all EIP-163 clock signals, */
    /* e.g. go back to the dynamic clock control */
    Rc = EIP163_Device_Clocks_Off(&PRV_SHARED_MACSEC_CFYE_GLOBAL_VAR_GET(CfyE_Devices)[DeviceId]->IOArea, 0);
    if (Rc != EIP163_NO_ERROR)
    {
        Adapter_Free(PRV_SHARED_MACSEC_CFYE_GLOBAL_VAR_GET(CfyE_Devices)[DeviceId]);
        PRV_SHARED_MACSEC_CFYE_GLOBAL_VAR_SET(CfyE_Devices[DeviceId],NULL);
        CfyELib_Device_Unlock(DeviceId);
        return CFYE_DLIB_ERROR(Rc);
    }

    LOG_INFO("\n\t EIP163_Device_Clocks_On \n");

    /* Clock control, enable all EIP-163 clock signals */
    Rc = EIP163_Device_Clocks_On(&PRV_SHARED_MACSEC_CFYE_GLOBAL_VAR_GET(CfyE_Devices)[DeviceId]->IOArea,
                            EIP163_ALL_CLOCKS,
                            1);
    if (Rc != EIP163_NO_ERROR)
    {
        Adapter_Free(PRV_SHARED_MACSEC_CFYE_GLOBAL_VAR_GET(CfyE_Devices)[DeviceId]);
        PRV_SHARED_MACSEC_CFYE_GLOBAL_VAR_SET(CfyE_Devices[DeviceId],NULL);
        CfyELib_Device_Unlock(DeviceId);
        return CFYE_DLIB_ERROR(Rc);
    }

    LOG_INFO("\n\t EIP163_Device_Clocks_On \n");

    /* Clock control, restore all EIP-163 clock signals, */
    /* e.g. go back to the dynamic clock control, */
    /* for this ADAPTER_EIP163_CLOCK_CONTROL must be set to 0 */
    Rc = EIP163_Device_Clocks_On(&PRV_SHARED_MACSEC_CFYE_GLOBAL_VAR_GET(CfyE_Devices)[DeviceId]->IOArea,
                            EIP163_ALL_CLOCKS,
                            ADAPTER_EIP163_CLOCK_CONTROL);
    if (Rc != EIP163_NO_ERROR)
    {
        Adapter_Free(PRV_SHARED_MACSEC_CFYE_GLOBAL_VAR_GET(CfyE_Devices)[DeviceId]);
        PRV_SHARED_MACSEC_CFYE_GLOBAL_VAR_SET(CfyE_Devices[DeviceId],NULL);
        CfyELib_Device_Unlock(DeviceId);
        return CFYE_DLIB_ERROR(Rc);
    }

#endif /* !ADAPTER_EIP163_DEV_INIT_DISABLE */

    if (Cp.EIP163_Options.Rules_Count == 0)
    {
        LOG_CRIT("%s: Failed, no rules are supported for EIP-163 device %d\n",
                 __func__,
                 DeviceId);
        Adapter_Free(PRV_SHARED_MACSEC_CFYE_GLOBAL_VAR_GET(CfyE_Devices)[DeviceId]);
        PRV_SHARED_MACSEC_CFYE_GLOBAL_VAR_SET(CfyE_Devices[DeviceId],NULL);
        CfyELib_Device_Unlock(DeviceId);
        return CFYE_ERROR_INTERNAL;
    }

    PRV_SHARED_MACSEC_CFYE_GLOBAL_VAR_SET(CfyE_Devices[DeviceId]->RulesCount,Cp.EIP163_Options.Rules_Count);
    PRV_SHARED_MACSEC_CFYE_GLOBAL_VAR_SET(CfyE_Devices[DeviceId]->vPortCount,Cp.EIP163_Options.vPort_Count);
    PRV_SHARED_MACSEC_CFYE_GLOBAL_VAR_SET(CfyE_Devices[DeviceId]->ChannelsCount,Cp.EIP163_Options.Channel_Count);
    PRV_SHARED_MACSEC_CFYE_GLOBAL_VAR_SET(CfyE_Devices[DeviceId]->Role,Role);

    PRV_SHARED_MACSEC_CFYE_GLOBAL_VAR_SET(CfyE_Devices[DeviceId]->vPortDscr_p,NULL);
    PRV_SHARED_MACSEC_CFYE_GLOBAL_VAR_SET(CfyE_Devices[DeviceId]->vPortFreeList_p,NULL);
    PRV_SHARED_MACSEC_CFYE_GLOBAL_VAR_SET(CfyE_Devices[DeviceId]->RuleDscr_p,NULL);
    PRV_SHARED_MACSEC_CFYE_GLOBAL_VAR_SET(CfyE_Devices[DeviceId]->RuleFreeList_p,NULL);
    PRV_SHARED_MACSEC_CFYE_GLOBAL_VAR_SET(CfyE_Devices[DeviceId]->fExternalTCAM,Cp.EIP163_Options.fExternalTCAM);

    /* Restrict any counts to the values specified by Init_p */
    if (Init_p->MaxChannelCount > 0 &&
        Init_p->MaxChannelCount < PRV_SHARED_MACSEC_CFYE_GLOBAL_VAR_GET(CfyE_Devices)[DeviceId]->ChannelsCount)
    {
        LOG_CRIT("%s: restricting ChannelsCount to %u\n",
                 __func__,Init_p->MaxChannelCount);
        PRV_SHARED_MACSEC_CFYE_GLOBAL_VAR_SET(CfyE_Devices[DeviceId]->ChannelsCount,Init_p->MaxChannelCount);
    }
    if (Init_p->MaxvPortCount > 0 &&
        Init_p->MaxvPortCount < PRV_SHARED_MACSEC_CFYE_GLOBAL_VAR_GET(CfyE_Devices)[DeviceId]->vPortCount)
    {
        LOG_CRIT("%s: restricting vPortCount to %u\n",
                 __func__,Init_p->MaxvPortCount);
        PRV_SHARED_MACSEC_CFYE_GLOBAL_VAR_SET(CfyE_Devices[DeviceId]->vPortCount,Init_p->MaxvPortCount);
    }
    if (Init_p->MaxRuleCount > 0 &&
        Init_p->MaxRuleCount < PRV_SHARED_MACSEC_CFYE_GLOBAL_VAR_GET(CfyE_Devices)[DeviceId]->RulesCount)
    {
        LOG_CRIT("%s: restricting RulesCount to %u\n",
                 __func__,Init_p->MaxRuleCount);
        PRV_SHARED_MACSEC_CFYE_GLOBAL_VAR_SET(CfyE_Devices[DeviceId]->RulesCount,Init_p->MaxRuleCount);
    }

#ifdef ADAPTER_EIP163_WARMBOOT_ENABLE
    if (!CfyELib_WarmBoot_Device_init(DeviceId))
    {
        LOG_CRIT("%s: WarmBoot initialization failed\n",__func__);
        Adapter_Free(PRV_SHARED_MACSEC_CFYE_GLOBAL_VAR_GET(CfyE_Devices)[DeviceId]);
        PRV_SHARED_MACSEC_CFYE_GLOBAL_VAR_SET(CfyE_Devices[DeviceId],NULL);
        CfyELib_Device_Unlock(DeviceId);
        return CFYE_ERROR_INTERNAL;
    }
#endif

#ifdef ADAPTER_EIP163_INTERRUPTS_ENABLE
    {
        unsigned int i;
        int rc;
        LOG_INFO("%s: Initialize Global AIC\n",__func__);
        /* Initialize Global AIC */
        rc = Adapter_Interrupts_Init(CfyELib_ICDeviceID_Get(DeviceId, NULL));
        if (rc) goto irq_failed;
        rc = Adapter_Interrupt_SetHandler(CfyELib_ICDeviceID_Get(DeviceId, NULL),
                                 CfyELib_InterruptHandler);
        if (rc) goto irq_failed;
        /* Initialize Channel AIC's */
        for (i = 0; i < PRV_SHARED_MACSEC_CFYE_GLOBAL_VAR_GET(CfyE_Devices)[DeviceId]->ChannelsCount; i++)
        {
            LOG_INFO("%s: Initialize AIC for channel %d\n", __func__, i);
            rc = Adapter_Interrupts_Init(CfyELib_ICDeviceID_Get(DeviceId, &i));
            if (rc) goto irq_failed;
            rc = Adapter_Interrupt_SetHandler(CfyELib_ICDeviceID_Get(DeviceId, &i),
                                         CfyELib_InterruptHandler);
            if (rc) goto irq_failed;
        }
    irq_failed:
        if (rc)
        {
            Adapter_Interrupts_UnInit(CfyELib_ICDeviceID_Get(DeviceId, NULL));
            for (i = 0; i < PRV_SHARED_MACSEC_CFYE_GLOBAL_VAR_GET(CfyE_Devices)[DeviceId]->ChannelsCount; i++)
            {
                Adapter_Interrupts_UnInit(CfyELib_ICDeviceID_Get(DeviceId, &i));
            }
            Adapter_Free(PRV_SHARED_MACSEC_CFYE_GLOBAL_VAR_GET(CfyE_Devices)[DeviceId]);
            PRV_SHARED_MACSEC_CFYE_GLOBAL_VAR_SET(CfyE_Devices[DeviceId],NULL);
            CfyELib_Device_Unlock(DeviceId);
            return CFYE_DLIB_ERROR(rc);
        }
    }
#endif /* ADAPTER_EIP163_INTERRUPTS_ENABLE */

    /* Initialize the vPort free list for this device */
    if (!CfyELib_DataObjectList_Init(
                                DeviceId,
                                &PRV_SHARED_MACSEC_CFYE_GLOBAL_VAR_GET(CfyE_Devices)[DeviceId]->vPortFreeList_p,
                                false,
                                (void**)&PRV_SHARED_MACSEC_CFYE_GLOBAL_VAR_GET(CfyE_Devices)[DeviceId]->vPortDscr_p,
                                PRV_SHARED_MACSEC_CFYE_GLOBAL_VAR_GET(CfyE_Devices)[DeviceId]->vPortCount))
    {
        LOG_CRIT("%s: Failed to create vPort free list for EIP-163 device %d\n",
                 __func__,
                 DeviceId);
#ifdef ADAPTER_EIP163_WARMBOOT_ENABLE
        CfyELib_WarmBoot_Device_Uninit(DeviceId);
#endif
        Adapter_Free(PRV_SHARED_MACSEC_CFYE_GLOBAL_VAR_GET(CfyE_Devices)[DeviceId]);
        PRV_SHARED_MACSEC_CFYE_GLOBAL_VAR_SET(CfyE_Devices[DeviceId],NULL);
        CfyELib_Device_Unlock(DeviceId);
        return CFYE_ERROR_INTERNAL;
    }

    /* Initialize the Rule free list for this device */
    if (!CfyELib_DataObjectList_Init(
                                DeviceId,
                                &PRV_SHARED_MACSEC_CFYE_GLOBAL_VAR_GET(CfyE_Devices)[DeviceId]->RuleFreeList_p,
                                true,
                                (void**)&PRV_SHARED_MACSEC_CFYE_GLOBAL_VAR_GET(CfyE_Devices)[DeviceId]->RuleDscr_p,
                                PRV_SHARED_MACSEC_CFYE_GLOBAL_VAR_GET(CfyE_Devices)[DeviceId]->RulesCount))
    {
        LOG_CRIT("%s: Failed to create Rule free list for EIP-163 device %d\n",
                 __func__,
                 DeviceId);
        CfyELib_vPortFreeList_Uninit(DeviceId);
#ifdef ADAPTER_EIP163_WARMBOOT_ENABLE
        CfyELib_WarmBoot_Device_Uninit(DeviceId);
#endif
        Adapter_Free(PRV_SHARED_MACSEC_CFYE_GLOBAL_VAR_GET(CfyE_Devices)[DeviceId]);
        PRV_SHARED_MACSEC_CFYE_GLOBAL_VAR_SET(CfyE_Devices[DeviceId],NULL);
        CfyELib_Device_Unlock(DeviceId);
        return CFYE_ERROR_INTERNAL;
    }

    PRV_SHARED_MACSEC_CFYE_GLOBAL_VAR_SET(CfyE_Device_StaticFields[DeviceId].fInitialized,true);

    LOG_INFO("%s: device %d successfully initialized\n", __func__, DeviceId);

    CfyELib_Device_Unlock(DeviceId);

    LOG_INFO("\n %s done \n", __func__);

    return CFYE_STATUS_OK;
}


/*----------------------------------------------------------------------------
 * CfyE_Device_Uninit
 */
CfyE_Status_t
CfyE_Device_Uninit(
        const unsigned int DeviceId)
{
    unsigned int Count;

#ifndef ADAPTER_EIP163_DEV_UNINIT_DISABLE
    EIP163_Error_t Rc;
#endif

    LOG_INFO("\n %s \n", __func__);

    ADAPTER_EIP163_CHECK_INT_ATMOST(DeviceId, ADAPTER_EIP163_MAX_NOF_DEVICES - 1);

    CfyELib_Device_Lock(DeviceId);

    if (!CfyELib_Initialized_Check(DeviceId, __func__, true))
    {
        CfyELib_Device_Unlock(DeviceId);
        return CFYE_ERROR_INTERNAL;
    }

#ifdef ADAPTER_EIP163_INTERRUPTS_ENABLE
    LOG_INFO("%s: Uninitialize Global AIC\n", __func__);
    Adapter_Interrupts_UnInit(CfyELib_ICDeviceID_Get(DeviceId, NULL));

    Count = ADAPTER_EIP163_NOTIFY_ID(CfyELib_ICDeviceID_Get(DeviceId, NULL));
    Adapter_Free(CfyE_Notify[Count]);
    CfyE_Notify[Count] = NULL;

    for (Count = 0; Count < PRV_SHARED_MACSEC_CFYE_GLOBAL_VAR_GET(CfyE_Devices)[DeviceId]->ChannelsCount; Count++)
    {
        unsigned int NotifyId;

        LOG_INFO("%s: Uninitialize AIC for channel %d\n", __func__, Count);
        Adapter_Interrupts_UnInit(CfyELib_ICDeviceID_Get(DeviceId, &Count));

        NotifyId = ADAPTER_EIP163_NOTIFY_ID(
                            CfyELib_ICDeviceID_Get(DeviceId, &Count));
        Adapter_Free(CfyE_Notify[NotifyId]);
        CfyE_Notify[NotifyId] = NULL;
    }

#endif /* ADAPTER_EIP163_INTERRUPTS_ENABLE */

    List_GetListElementCount(0,
                             PRV_SHARED_MACSEC_CFYE_GLOBAL_VAR_GET(CfyE_Devices)[DeviceId]->vPortFreeList_p,
                             &Count);
    if (Count != PRV_SHARED_MACSEC_CFYE_GLOBAL_VAR_GET(CfyE_Devices)[DeviceId]->vPortCount)
    {
        LOG_CRIT("%s: EIP-163 device %d (%s) has %d added vPorts\n",
                 __func__,
                 DeviceId,
                 ADAPTER_EIP163_DEVICE_NAME(DeviceId),
                 PRV_SHARED_MACSEC_CFYE_GLOBAL_VAR_GET(CfyE_Devices)[DeviceId]->vPortCount - Count);
        CfyELib_Device_Unlock(DeviceId);
        return CFYE_ERROR_INTERNAL;
    }

    /* Uninitialize Rule free list */
    CfyELib_RuleFreeList_Uninit(DeviceId);

    /* Uninitialize vPort free list */
    CfyELib_vPortFreeList_Uninit(DeviceId);

#ifndef ADAPTER_EIP163_DEV_UNINIT_DISABLE
    LOG_INFO("\n\t EIP163_Device_Uninit \n");

    Rc = EIP163_Device_Uninit(&PRV_SHARED_MACSEC_CFYE_GLOBAL_VAR_GET(CfyE_Devices)[DeviceId]->IOArea);
    if (Rc != EIP163_NO_ERROR && Rc != EIP163_BUSY_RETRY_LATER)
    {
        LOG_CRIT("%s: Failed to uninit EIP-163 device %d (%s), error %d\n",
                 __func__,
                 DeviceId,
                 ADAPTER_EIP163_DEVICE_NAME(DeviceId),
                 Rc);
        PRV_SHARED_MACSEC_CFYE_GLOBAL_VAR_SET(CfyE_Device_StaticFields[DeviceId].fInitialized,false);

#ifdef ADAPTER_EIP163_WARMBOOT_ENABLE
        CfyELib_WarmBoot_Device_Uninit(DeviceId);
#endif
        Adapter_Free(PRV_SHARED_MACSEC_CFYE_GLOBAL_VAR_GET(CfyE_Devices)[DeviceId]);
        PRV_SHARED_MACSEC_CFYE_GLOBAL_VAR_SET(CfyE_Devices[DeviceId],NULL);
        CfyELib_Device_Unlock(DeviceId);
        return CFYE_DLIB_ERROR(Rc);
    }

#if ADAPTER_EIP163_MAX_NOF_UNINIT_RETRY_COUNT > 0
    /* Wait till the CfyE device uninitialization is done */
    if (Rc == EIP163_BUSY_RETRY_LATER)
    {
        unsigned int LoopCounter = ADAPTER_EIP163_MAX_NOF_UNINIT_RETRY_COUNT;

        do
        {
            LOG_INFO("\n\t EIP163_Device_Is_Done \n");

            Rc = EIP163_Device_Is_Done(&PRV_SHARED_MACSEC_CFYE_GLOBAL_VAR_GET(CfyE_Devices)[DeviceId]->IOArea);
            if (Rc != EIP163_BUSY_RETRY_LATER)
            {
                break;
            }

            Adapter_SleepMS(ADAPTER_EIP163_UNINIT_RETRY_TIMEOUT_MS);
        } while (--LoopCounter);

        if (Rc != EIP163_NO_ERROR)
        {
            LOG_CRIT(
              "%s: Failed to complete EIP-163 device %d uninitialization\n",
              __func__,
              DeviceId);
            PRV_SHARED_MACSEC_CFYE_GLOBAL_VAR_SET(CfyE_Device_StaticFields[DeviceId].fInitialized,false);
#ifdef ADAPTER_EIP163_WARMBOOT_ENABLE
            CfyELib_WarmBoot_Device_Uninit(DeviceId);
#endif
            Adapter_Free(PRV_SHARED_MACSEC_CFYE_GLOBAL_VAR_GET(CfyE_Devices)[DeviceId]);
            PRV_SHARED_MACSEC_CFYE_GLOBAL_VAR_SET(CfyE_Devices[DeviceId],NULL);
            CfyELib_Device_Unlock(DeviceId);
            return CFYE_DLIB_ERROR(Rc);
        }
    }
#endif /* ADAPTER_EIP163_MAX_NOF_UNINIT_RETRY_COUNT > 0 */
#endif /* !ADAPTER_EIP163_DEV_UNINIT_DISABLE */

    PRV_SHARED_MACSEC_CFYE_GLOBAL_VAR_SET(CfyE_Device_StaticFields[DeviceId].fInitialized,false);


    LOG_INFO("%s: device %d successfully uninitialized\n",
             __func__,
             DeviceId);

#ifdef ADAPTER_EIP163_WARMBOOT_ENABLE
    CfyELib_WarmBoot_Device_Uninit(DeviceId);
#endif

    Adapter_Free(PRV_SHARED_MACSEC_CFYE_GLOBAL_VAR_GET(CfyE_Devices)[DeviceId]);
    PRV_SHARED_MACSEC_CFYE_GLOBAL_VAR_SET(CfyE_Devices[DeviceId],NULL);

    CfyELib_Device_Unlock(DeviceId);

    LOG_INFO("\n %s done \n", __func__);

    return CFYE_STATUS_OK;
}


/*-----------------------------------------------------------------------------
 * CfyE_Device_Limits_Get(
 */
CfyE_Status_t
CfyE_Device_Limits_Get(
        const unsigned int DeviceId,
        CfyE_Device_Limits_t* const device_limits_p)
{
    Device_Handle_t Device;
    EIP163_Error_t rc;
    EIP163_Capabilities_t cp;
    LOG_INFO("\n %s \n", __func__);

    ADAPTER_EIP163_CHECK_INT_ATMOST(DeviceId, ADAPTER_EIP163_MAX_NOF_DEVICES - 1);
    ADAPTER_EIP163_CHECK_POINTER(device_limits_p);

    CfyELib_Device_Lock(DeviceId);

    if (!CfyELib_Initialized_Check(DeviceId, __func__, true))
    {
        CfyELib_Device_Unlock(DeviceId);
        return CFYE_ERROR_INTERNAL;
    }

    /* Find the EIP-163 device */
    Device = Device_Find(ADAPTER_EIP163_DEVICE_NAME(DeviceId));
    if (Device == NULL)
    {
        LOG_CRIT("%s: Failed to locate EIP-163 device %d (%s)\n",
                 __func__,
                 DeviceId,
                 ADAPTER_EIP163_DEVICE_NAME(DeviceId));
        CfyELib_Device_Unlock(DeviceId);
        return CFYE_ERROR_INTERNAL;
    }

    rc = EIP163_HWRevision_Get(Device, &cp);
    if (rc != EIP163_NO_ERROR)
    {
        LOG_CRIT("%s: Failed to get EIP-163 device %d capabilities, error %d\n",
                 __func__,
                 DeviceId,
                 rc);
        CfyELib_Device_Unlock(DeviceId);
        return CFYE_DLIB_ERROR(rc);
    }

    device_limits_p->major_version = cp.EIP163_Version.MajHWRevision;
    device_limits_p->minor_version = cp.EIP163_Version.MinHWRevision;
    device_limits_p->patch_level = cp.EIP163_Version.HWPatchLevel;
    device_limits_p->channel_count = PRV_SHARED_MACSEC_CFYE_GLOBAL_VAR_GET(CfyE_Devices)[DeviceId]->ChannelsCount;
    device_limits_p->vport_count = PRV_SHARED_MACSEC_CFYE_GLOBAL_VAR_GET(CfyE_Devices)[DeviceId]->vPortCount;
    device_limits_p->rule_count = PRV_SHARED_MACSEC_CFYE_GLOBAL_VAR_GET(CfyE_Devices)[DeviceId]->RulesCount;

    LOG_INFO("\n %s done\n", __func__);
    CfyELib_Device_Unlock(DeviceId);

    return CFYE_STATUS_OK;
}


/*----------------------------------------------------------------------------
 * CfyE_Device_Limits
 */
CfyE_Status_t
CfyE_Device_Limits(
        const unsigned int DeviceId,
        unsigned int * const MaxChannelCount_p,
        unsigned int * const MaxvPortCount_p,
        unsigned int * const MaxRuleCount_p)
{
    LOG_INFO("\n %s \n", __func__);

    ADAPTER_EIP163_CHECK_INT_ATMOST(DeviceId, ADAPTER_EIP163_MAX_NOF_DEVICES - 1);

    CfyELib_Device_Lock(DeviceId);

    if (!CfyELib_Initialized_Check(DeviceId, __func__, true))
    {
        CfyELib_Device_Unlock(DeviceId);
        return CFYE_ERROR_INTERNAL;
    }

    if (MaxChannelCount_p != NULL)
        *MaxChannelCount_p = PRV_SHARED_MACSEC_CFYE_GLOBAL_VAR_GET(CfyE_Devices)[DeviceId]->ChannelsCount;

    if (MaxvPortCount_p != NULL)
        *MaxvPortCount_p = PRV_SHARED_MACSEC_CFYE_GLOBAL_VAR_GET(CfyE_Devices)[DeviceId]->vPortCount;

    if (MaxRuleCount_p != NULL)
        *MaxRuleCount_p = PRV_SHARED_MACSEC_CFYE_GLOBAL_VAR_GET(CfyE_Devices)[DeviceId]->RulesCount;

    LOG_INFO("\n %s done\n", __func__);
    CfyELib_Device_Unlock(DeviceId);

    return CFYE_STATUS_OK;
}


/*----------------------------------------------------------------------------
 * CfyE_Device_Update
 */
CfyE_Status_t
CfyE_Device_Update(
        const unsigned int DeviceId,
        unsigned int ChannelId,
        const CfyE_Device_t * const Device_p)
{
    EIP163_Error_t EIP163_Rc;
    unsigned int ErrorType; /* Index in ErrorString */
    const char * ErrorString[] = {"bypass",
                                  "control packet",
                                  "header parser",
                                  "statistics",
                                  "statistics sync"};

    LOG_INFO("\n %s \n", __func__);

    ADAPTER_EIP163_CHECK_INT_ATMOST(DeviceId, ADAPTER_EIP163_MAX_NOF_DEVICES - 1);
    ADAPTER_EIP163_CHECK_POINTER(Device_p);

    CfyELib_Device_Lock(DeviceId);

    if (!CfyELib_Initialized_Check(DeviceId, __func__, true))
    {
        CfyELib_Device_Unlock(DeviceId);
        return CFYE_ERROR_INTERNAL;
    }

    if (Device_p->Control_p)
    {
        EIP163_Device_Control_t DeviceControl;
        EIP163_Device_Exceptions_t DeviceEx;
        CfyE_Device_Control_t * DevCtrl_p = Device_p->Control_p;

        ZEROINIT(DeviceControl);

        DeviceControl.fLowLatencyBypass = DevCtrl_p->fLowLatencyBypass;

        if (DevCtrl_p->Exceptions_p)
        {
            ZEROINIT(DeviceEx);

            DeviceEx.DropAction         = DevCtrl_p->Exceptions_p->DropAction;
            DeviceEx.fForceDrop         = DevCtrl_p->Exceptions_p->fForceDrop;
            DeviceEx.fDefaultVPortValid = DevCtrl_p->Exceptions_p->fDefaultVPortValid;
            DeviceEx.DefaultVPort       = DevCtrl_p->Exceptions_p->DefaultVPort;

            DeviceControl.Exceptions_p = &DeviceEx;
        }

        LOG_INFO("\n\t EIP163_Device_Update \n");

        /* Update the classification device vPort control settings */
        EIP163_Rc = EIP163_Device_Update(&PRV_SHARED_MACSEC_CFYE_GLOBAL_VAR_GET(CfyE_Devices)[DeviceId]->IOArea,
                                         ChannelId,
                                         &DeviceControl);
        if (EIP163_Rc != EIP163_NO_ERROR)
        {
            ErrorType = 0; /* index in ErrorString */
            goto error_exit;
        }
    }

    if (Device_p->CP_p)
    {
        unsigned int i;
        EIP163_ControlPacket_t DeviceCP;
        CfyE_ControlPacket_t * CtrlPkt_p = Device_p->CP_p;

        ZEROINIT(DeviceCP);

        DeviceCP.CPMatchModeMask    = CtrlPkt_p->CPMatchModeMask;
        DeviceCP.CPMatchEnableMask  = CtrlPkt_p->CPMatchEnableMask;
        DeviceCP.SCPMatchModeMask   = CtrlPkt_p->SCPMatchModeMask;
        DeviceCP.SCPMatchEnableMask = CtrlPkt_p->SCPMatchEnableMask;

        DeviceCP.MAC_DA_44Bit_Const_p = CtrlPkt_p->MAC_DA_44Bit_Const_p;
        DeviceCP.MAC_DA_48Bit_Const_p = CtrlPkt_p->MAC_DA_48Bit_Const_p;

        DeviceCP.MAC_DA_Range.MAC_DA_Start_p =
                                CtrlPkt_p->MAC_DA_Range.MAC_DA_Start_p;
        DeviceCP.MAC_DA_Range.MAC_DA_End_p =
                                CtrlPkt_p->MAC_DA_Range.MAC_DA_End_p;

        for (i = 0; i < EIP163_MAC_DA_ET_MATCH_RULES_COUNT; i++)
        {
            DeviceCP.MAC_DA_ET_Rules[i].MAC_DA_p =
                            CtrlPkt_p->MAC_DA_ET_Rules[i].MAC_DA_p;
            DeviceCP.MAC_DA_ET_Rules[i].EtherType =
                            CtrlPkt_p->MAC_DA_ET_Rules[i].EtherType;
        }

        for (i = 0; i < EIP163_MAC_DA_ET_RANGE_MATCH_RULES_COUNT; i++)
        {
            DeviceCP.MAC_DA_ET_Range[i].Range.MAC_DA_Start_p =
                    CtrlPkt_p->MAC_DA_ET_Range[i].Range.MAC_DA_Start_p;
            DeviceCP.MAC_DA_ET_Range[i].Range.MAC_DA_End_p =
                    CtrlPkt_p->MAC_DA_ET_Range[i].Range.MAC_DA_End_p;
            DeviceCP.MAC_DA_ET_Range[i].EtherType =
                    CtrlPkt_p->MAC_DA_ET_Range[i].EtherType;
        }

        LOG_INFO("\n\t EIP163_ControlPacket_Update \n");

        /* Update the classification device control packet detection settings */
        EIP163_Rc = EIP163_ControlPacket_Update(&PRV_SHARED_MACSEC_CFYE_GLOBAL_VAR_GET(CfyE_Devices)[DeviceId]->IOArea,
                                                ChannelId,
                                                &DeviceCP);
        if (EIP163_Rc != EIP163_NO_ERROR)
        {
            ErrorType = 1; /* index in ErrorString */
            goto error_exit;
        }
    }

    if (Device_p->HeaderParser_p)
    {
        EIP163_SecTAG_Parser_t  SecTag;
        EIP163_VLAN_Parser_t    VLAN_Parser;
        EIP163_HeaderParser_t HdrParserControl;
        CfyE_HeaderParser_t * HdrPrs_p = Device_p->HeaderParser_p;

        ZEROINIT(HdrParserControl);


        /* MACsec SecTAG parser control settings */
        if (HdrPrs_p->SecTAG_Parser_p)
        {
            CfyE_SecTAG_Parser_t * STag_p = HdrPrs_p->SecTAG_Parser_p;

            ZEROINIT(SecTag);

            SecTag.fCompType        = STag_p->fCompType;
            SecTag.fCheckVersion    = STag_p->fCheckVersion;
            SecTag.fCheckKay        = STag_p->fCheckKay;
            SecTag.fLookupUseSCI    = STag_p->fLookupUseSCI;
            SecTag.MACsecTagValue   = STag_p->MACsecTagValue;

            HdrParserControl.SecTAG_Parser_p = &SecTag;
        }


        /* VLAN parser control settings */
        if (HdrPrs_p->VLAN_Parser_p)
        {
            unsigned int i;
            CfyE_VLAN_Parser_t * VLAN_p = HdrPrs_p->VLAN_Parser_p;

            ZEROINIT(VLAN_Parser);

            VLAN_Parser.DefaultUp      = VLAN_p->DefaultUp;
            VLAN_Parser.QTag           = VLAN_p->QTag;
            VLAN_Parser.STag1          = VLAN_p->STag1;
            VLAN_Parser.STag2          = VLAN_p->STag2;
            VLAN_Parser.STag3          = VLAN_p->STag3;
            VLAN_Parser.CP.fParseQinQ  = VLAN_p->CP.fParseQinQ;
            VLAN_Parser.CP.fParseQTag  = VLAN_p->CP.fParseQTag;
            VLAN_Parser.CP.fParseStag1 = VLAN_p->CP.fParseStag1;
            VLAN_Parser.CP.fParseStag2 = VLAN_p->CP.fParseStag2;
            VLAN_Parser.CP.fParseStag3 = VLAN_p->CP.fParseStag3;
            VLAN_Parser.fQTagUpEnable  = VLAN_p->fQTagUpEnable;
            VLAN_Parser.fSTagUpEnable  = VLAN_p->fSTagUpEnable;

            for (i = 0; i < CFYE_VLAN_UP_MAX_COUNT; i++)
            {
                VLAN_Parser.UpTable1[i] = VLAN_p->UpTable1[i];
                VLAN_Parser.UpTable2[i] = VLAN_p->UpTable2[i];
            }

            HdrParserControl.VLAN_Parser_p = &VLAN_Parser;
        }

        LOG_INFO("\n\t EIP163_HeaderParser_Update \n");

        /* Update the classification device header parser control settings */
        EIP163_Rc = EIP163_HeaderParser_Update(&PRV_SHARED_MACSEC_CFYE_GLOBAL_VAR_GET(CfyE_Devices)[DeviceId]->IOArea,
                                               ChannelId,
                                               &HdrParserControl);
        if (EIP163_Rc != EIP163_NO_ERROR)
        {
            ErrorType = 2; /* index in ErrorString */
            goto error_exit;
        }
    }

    if (Device_p->StatControl_p)
    {
        EIP163_Statistics_Control_t StatControl;
        CfyE_Statistics_Control_t * StatCtrl_p = Device_p->StatControl_p;

        ZEROINIT(StatControl);
        StatControl.fAutoStatCntrsReset   = StatCtrl_p->fAutoStatCntrsReset;
        StatControl.CountFrameThr.low     = StatCtrl_p->CountFrameThr.Lo;
        StatControl.CountFrameThr.hi      = StatCtrl_p->CountFrameThr.Hi;
        StatControl.ChanCountFrameThr.low = StatCtrl_p->ChanCountFrameThr.Lo;
        StatControl.ChanCountFrameThr.hi  = StatCtrl_p->ChanCountFrameThr.Hi;
        StatControl.CountIncDisCtrl       = StatCtrl_p->CountIncDisCtrl;

        LOG_INFO("\n\t EIP163_StatisticsControl_Update \n");

        LOG_CRIT("%s device statistics counter increment enable settings:\n"
                 "\tTCAM counters: %s\n"
                 "\tChannel counters: %s\n",
                 ADAPTER_EIP163_DEVICE_NAME(DeviceId),
                 (StatCtrl_p->CountIncDisCtrl & CFYE_TCAM_COUNT_INC_DIS) ?
                         "Disabled" : "Enabled",
                 (StatCtrl_p->CountIncDisCtrl & CFYE_CHAN_COUNT_INC_DIS) ?
                         "Disabled" : "Enabled");

        /* Update the classification device statistics control settings */
        EIP163_Rc = EIP163_StatisticsControl_Update(&PRV_SHARED_MACSEC_CFYE_GLOBAL_VAR_GET(CfyE_Devices)[DeviceId]->IOArea,
                                                    &StatControl);
        if ((EIP163_Rc != EIP163_NO_ERROR) && (EIP163_Rc != EIP163_BUSY_RETRY_LATER))
        {
            ErrorType = 3; /* index in ErrorString */
            goto error_exit;
        }

        if (EIP163_Rc == EIP163_BUSY_RETRY_LATER)
        {
            unsigned int LoopCounter = ADAPTER_EIP163_MAX_NOF_INIT_RETRY_COUNT;

            do
            {
                LOG_INFO("\n\t EIP163_Device_Is_Done \n");

                EIP163_Rc = EIP163_Device_Is_Done(&PRV_SHARED_MACSEC_CFYE_GLOBAL_VAR_GET(CfyE_Devices)[DeviceId]->IOArea);
                if (EIP163_Rc != EIP163_BUSY_RETRY_LATER)
                {
                    break;
                }

                Adapter_SleepMS(ADAPTER_EIP163_INIT_RETRY_TIMEOUT_MS);
            } while (--LoopCounter);

            if (EIP163_Rc != EIP163_NO_ERROR)
            {
                ErrorType = 4; /* index in ErrorString */
                goto error_exit;
            }
        }
    }

    if (Device_p->EOPConf_p)
    {
        LOG_INFO("\n\t EIP163_Device_EOP_Configure \n");

        EIP163_Rc = EIP163_Device_EOP_Configure(
                &PRV_SHARED_MACSEC_CFYE_GLOBAL_VAR_GET(CfyE_Devices)[DeviceId]->IOArea,
                Device_p->EOPConf_p->EOPTimeoutVal,
                (const EIP163_Ch_Mask_t * const) &Device_p->EOPConf_p->EOPTimeoutCtrl);

        if (EIP163_Rc != EIP163_NO_ERROR)
        {
            LOG_CRIT("%s: Failed to configure EOP EIP-163 device for device %d "
                     "(%s), error %d\n",
                     __func__,
                     DeviceId,
                     ADAPTER_EIP163_DEVICE_NAME(DeviceId),
                     EIP163_Rc);
            ErrorType = 4;
            goto error_exit;
        }
    }

    if (Device_p->ECCConf_p)
    {
        LOG_INFO("\n\t EIP163_Device_ECC_Configure \n");

        EIP163_Rc = EIP163_Device_ECC_Configure(&PRV_SHARED_MACSEC_CFYE_GLOBAL_VAR_GET(CfyE_Devices)[DeviceId]->IOArea,
                                         Device_p->ECCConf_p->ECCCorrectableThr,
                                         Device_p->ECCConf_p->ECCUncorrectableThr);
        if (EIP163_Rc != EIP163_NO_ERROR)
        {
            LOG_CRIT("%s: Failed to configure ECC EIP-163 device for device %d "
                     "(%s), error %d\n",
                     __func__,
                     DeviceId,
                     ADAPTER_EIP163_DEVICE_NAME(DeviceId),
                     EIP163_Rc);
            ErrorType = 4;
            goto error_exit;
        }
    }

    CfyELib_Device_Unlock(DeviceId);

    LOG_INFO("\n %s done \n", __func__);

    IDENTIFIER_NOT_USED_LOG_OFF(ErrorType);
    IDENTIFIER_NOT_USED_LOG_OFF(ErrorString);
    return CFYE_STATUS_OK;

error_exit:
    LOG_CRIT("%s: Failed to update EIP-163 device %d (%s) for %s, "
             "error %d\n",
             __func__,
             DeviceId,
             ADAPTER_EIP163_DEVICE_NAME(DeviceId),
             ErrorString[ErrorType],
             EIP163_Rc);

    CfyELib_Device_Unlock(DeviceId);

    return CFYE_DLIB_ERROR(EIP163_Rc);
}


/*-----------------------------------------------------------------------------
 * CfyE_Device_Config_Get
 */
CfyE_Status_t
CfyE_Device_Config_Get(
        const unsigned int DeviceId,
        unsigned int ChannelId,
        CfyE_Device_t * const Device_p)
{
    EIP163_Error_t Rc;
    CfyE_Device_Control_t * DevCtrl_p;
    CfyE_ControlPacket_t * CtrlPkt_p;
    CfyE_HeaderParser_t * HdrPrs_p;
    CfyE_Statistics_Control_t *StatControl_p;
    CfyE_EOPConf_t * EOPConf_p;
    CfyE_ECCConf_t * ECCConf_p;

    CfyELib_Device_Lock(DeviceId);

    if (!CfyELib_Initialized_Check(DeviceId, __func__, true))
    {
        CfyELib_Device_Unlock(DeviceId);
        return CFYE_ERROR_INTERNAL;
    }

    DevCtrl_p = Device_p->Control_p;
    if (DevCtrl_p)
    {
        EIP163_Device_Control_t DeviceControl;
        EIP163_Device_Exceptions_t DeviceEx;
        ZEROINIT(DeviceControl);
        ZEROINIT(DeviceEx);

        if (DevCtrl_p->Exceptions_p)
        {
            DeviceControl.Exceptions_p = &DeviceEx;
        }


        LOG_INFO("\n\t EIP163_Device_Control_Read \n");

        /* Read the classification device vPort control settings */
        Rc = EIP163_Device_Control_Read(&PRV_SHARED_MACSEC_CFYE_GLOBAL_VAR_GET(CfyE_Devices)[DeviceId]->IOArea,
                                         ChannelId,
                                         &DeviceControl);
        if (Rc != EIP163_NO_ERROR)
        {
            LOG_CRIT("%s: Failed to read device control EIP-163 device for device %d "
                     "(%s), error %d\n",
                     __func__,
                     DeviceId,
                     ADAPTER_EIP163_DEVICE_NAME(DeviceId),
                     Rc);
            CfyELib_Device_Unlock(DeviceId);
            return CFYE_DLIB_ERROR(Rc);
        }

        DevCtrl_p->fLowLatencyBypass = DeviceControl.fLowLatencyBypass;

        if (DevCtrl_p->Exceptions_p)
        {
            DevCtrl_p->Exceptions_p->DropAction = DeviceEx.DropAction;
            DevCtrl_p->Exceptions_p->fForceDrop = DeviceEx.fForceDrop;
            DevCtrl_p->Exceptions_p->fDefaultVPortValid = DeviceEx.fDefaultVPortValid;
            DevCtrl_p->Exceptions_p->DefaultVPort = DeviceEx.DefaultVPort;
        }
    }

    CtrlPkt_p = Device_p->CP_p;
    if (CtrlPkt_p)
    {
        unsigned int i;
        EIP163_ControlPacket_t DeviceCP;

        LOG_INFO("\n\t EIP163_ControlPacket_Read \n");
        ZEROINIT(DeviceCP);

        DeviceCP.MAC_DA_44Bit_Const_p = CtrlPkt_p->MAC_DA_44Bit_Const_p ;
        DeviceCP.MAC_DA_48Bit_Const_p = CtrlPkt_p->MAC_DA_48Bit_Const_p;

        DeviceCP.MAC_DA_Range.MAC_DA_Start_p =
            CtrlPkt_p->MAC_DA_Range.MAC_DA_Start_p;
        DeviceCP.MAC_DA_Range.MAC_DA_End_p =
            CtrlPkt_p->MAC_DA_Range.MAC_DA_End_p;

        for (i = 0; i < EIP163_MAC_DA_ET_MATCH_RULES_COUNT; i++)
        {
            DeviceCP.MAC_DA_ET_Rules[i].MAC_DA_p =
                CtrlPkt_p->MAC_DA_ET_Rules[i].MAC_DA_p;
        }

        for (i = 0; i < EIP163_MAC_DA_ET_RANGE_MATCH_RULES_COUNT; i++)
        {
            DeviceCP.MAC_DA_ET_Range[i].Range.MAC_DA_Start_p =
                CtrlPkt_p->MAC_DA_ET_Range[i].Range.MAC_DA_Start_p;
            DeviceCP.MAC_DA_ET_Range[i].Range.MAC_DA_End_p =
                CtrlPkt_p->MAC_DA_ET_Range[i].Range.MAC_DA_End_p;
        }

        /* Read the classification device control packet detection settings */
        Rc = EIP163_ControlPacket_Read(&PRV_SHARED_MACSEC_CFYE_GLOBAL_VAR_GET(CfyE_Devices)[DeviceId]->IOArea,
                                                ChannelId,
                                                &DeviceCP);
        if (Rc != EIP163_NO_ERROR)
        {
            LOG_CRIT("%s: Failed to read control packet EIP-163 device for device %d "
                     "(%s), error %d\n",
                     __func__,
                     DeviceId,
                     ADAPTER_EIP163_DEVICE_NAME(DeviceId),
                     Rc);
            CfyELib_Device_Unlock(DeviceId);
            return CFYE_DLIB_ERROR(Rc);
        }
        CtrlPkt_p->CPMatchModeMask = DeviceCP.CPMatchModeMask;
        CtrlPkt_p->CPMatchEnableMask = DeviceCP.CPMatchEnableMask;
        CtrlPkt_p->SCPMatchModeMask = DeviceCP.SCPMatchModeMask;
        CtrlPkt_p->SCPMatchEnableMask = DeviceCP.SCPMatchEnableMask;

        for (i = 0; i < EIP163_MAC_DA_ET_MATCH_RULES_COUNT; i++)
        {
            CtrlPkt_p->MAC_DA_ET_Rules[i].EtherType =
                DeviceCP.MAC_DA_ET_Rules[i].EtherType;
        }

        for (i = 0; i < EIP163_MAC_DA_ET_RANGE_MATCH_RULES_COUNT; i++)
        {
            CtrlPkt_p->MAC_DA_ET_Range[i].EtherType =
                DeviceCP.MAC_DA_ET_Range[i].EtherType;
        }
    }

    HdrPrs_p = Device_p->HeaderParser_p;
    if (HdrPrs_p)
    {
        EIP163_SecTAG_Parser_t  SecTAG_Parser;

        EIP163_VLAN_Parser_t    VLAN_Parser;
        EIP163_HeaderParser_t HdrParserControl;

        ZEROINIT(HdrParserControl);
        ZEROINIT(SecTAG_Parser);
        ZEROINIT(VLAN_Parser);

        /* MACsec SecTAG parser control settings */
        if (HdrPrs_p->SecTAG_Parser_p)
        {
            HdrParserControl.SecTAG_Parser_p = &SecTAG_Parser;
        }
        /* VLAN parser control settings */
        if (HdrPrs_p->VLAN_Parser_p)
        {
            HdrParserControl.VLAN_Parser_p = &VLAN_Parser;
        }

        /* Read the classification device header parser control settings */
        Rc = EIP163_HeaderParser_Read(&PRV_SHARED_MACSEC_CFYE_GLOBAL_VAR_GET(CfyE_Devices)[DeviceId]->IOArea,
                                      ChannelId,
                                      &HdrParserControl);
        if (Rc != EIP163_NO_ERROR)
        {
            LOG_CRIT("%s: Failed to read header parser EIP-163 device for device %d "
                     "(%s), error %d\n",
                     __func__,
                     DeviceId,
                     ADAPTER_EIP163_DEVICE_NAME(DeviceId),
                     Rc);
            CfyELib_Device_Unlock(DeviceId);
            return CFYE_DLIB_ERROR(Rc);
        }


        /* MACsec SecTAG parser control settings */
        if (HdrPrs_p->SecTAG_Parser_p)
        {
            CfyE_SecTAG_Parser_t * STag_p = HdrPrs_p->SecTAG_Parser_p;

            STag_p->fCompType = SecTAG_Parser.fCompType;
            STag_p->fCheckVersion = SecTAG_Parser.fCheckVersion;
            STag_p->fCheckKay = SecTAG_Parser.fCheckKay;
            STag_p->fLookupUseSCI = SecTAG_Parser.fLookupUseSCI;
            STag_p->MACsecTagValue = SecTAG_Parser.MACsecTagValue;
        }
        /* VLAN parser control settings */
        if (HdrPrs_p->VLAN_Parser_p)
        {
            unsigned int i;
            CfyE_VLAN_Parser_t * VLAN_p = HdrPrs_p->VLAN_Parser_p;

            VLAN_p->DefaultUp = VLAN_Parser.DefaultUp;
            VLAN_p->QTag = VLAN_Parser.QTag;
            VLAN_p->STag1 = VLAN_Parser.STag1;
            VLAN_p->STag2 = VLAN_Parser.STag2;
            VLAN_p->STag3 = VLAN_Parser.STag3;
            VLAN_p->CP.fParseQinQ = VLAN_Parser.CP.fParseQinQ;
            VLAN_p->CP.fParseQTag = VLAN_Parser.CP.fParseQTag;
            VLAN_p->CP.fParseStag1 = VLAN_Parser.CP.fParseStag1;
            VLAN_p->CP.fParseStag2 = VLAN_Parser.CP.fParseStag2;
            VLAN_p->CP.fParseStag3 = VLAN_Parser.CP.fParseStag3;
            VLAN_p->fQTagUpEnable = VLAN_Parser.fQTagUpEnable;
            VLAN_p->fSTagUpEnable = VLAN_Parser.fSTagUpEnable;

            for (i = 0; i < CFYE_VLAN_UP_MAX_COUNT; i++)
            {
                VLAN_p->UpTable1[i] = VLAN_Parser.UpTable1[i];
                VLAN_p->UpTable2[i] = VLAN_Parser.UpTable2[i];
            }
        }
    }

    StatControl_p = Device_p->StatControl_p;
    if (StatControl_p)
    {
        EIP163_Statistics_Control_t StatControl;
        ZEROINIT(StatControl);
        LOG_INFO("\n\t EIP163_StatisticsControl_Read \n");

        Rc = EIP163_StatisticsControl_Read(&PRV_SHARED_MACSEC_CFYE_GLOBAL_VAR_GET(CfyE_Devices)[DeviceId]->IOArea,
                                           &StatControl);
        if (Rc != EIP163_NO_ERROR)
        {
            LOG_CRIT("%s: Failed to read statistics control EIP-163 device for device %d "
                     "(%s), error %d\n",
                     __func__,
                     DeviceId,
                     ADAPTER_EIP163_DEVICE_NAME(DeviceId),
                     Rc);
            CfyELib_Device_Unlock(DeviceId);
            return CFYE_DLIB_ERROR(Rc);
        }
        StatControl_p->fAutoStatCntrsReset = StatControl.fAutoStatCntrsReset;
        StatControl_p->CountFrameThr.Hi = StatControl.CountFrameThr.hi;
        StatControl_p->CountFrameThr.Lo = StatControl.CountFrameThr.low;
        StatControl_p->ChanCountFrameThr.Hi = StatControl.ChanCountFrameThr.hi;
        StatControl_p->ChanCountFrameThr.Lo = StatControl.ChanCountFrameThr.low;
        StatControl_p->CountIncDisCtrl = StatControl.CountIncDisCtrl;
    }

    EOPConf_p = Device_p->EOPConf_p;
    if (EOPConf_p)
    {
        LOG_INFO("\n\t EIP163_Device_EOP_Configuration_Get \n");

        Rc = EIP163_Device_EOP_Configuration_Get(&PRV_SHARED_MACSEC_CFYE_GLOBAL_VAR_GET(CfyE_Devices)[DeviceId]->IOArea,
                                                 &EOPConf_p->EOPTimeoutVal,
                                                 (EIP163_Ch_Mask_t * const) &EOPConf_p->EOPTimeoutCtrl);
        if (Rc != EIP163_NO_ERROR)
        {
            LOG_CRIT("%s: Failed to read EOP conf EIP-163 device for device %d "
                     "(%s), error %d\n",
                     __func__,
                     DeviceId,
                     ADAPTER_EIP163_DEVICE_NAME(DeviceId),
                     Rc);
            CfyELib_Device_Unlock(DeviceId);
            return CFYE_DLIB_ERROR(Rc);
        }
    }

    ECCConf_p = Device_p->ECCConf_p;
    if (ECCConf_p)
    {
        LOG_INFO("\n\t EIP163_Device_ECC_Configuration_Get \n");

        Rc = EIP163_Device_ECC_Configuration_Get(&PRV_SHARED_MACSEC_CFYE_GLOBAL_VAR_GET(CfyE_Devices)[DeviceId]->IOArea,
                                                &ECCConf_p->ECCCorrectableThr,
                                                &ECCConf_p->ECCUncorrectableThr);
        if (Rc != EIP163_NO_ERROR)
        {
            LOG_CRIT("%s: Failed to read ECC conf EIP-163 device for device %d "
                     "(%s), error %d\n",
                     __func__,
                     DeviceId,
                     ADAPTER_EIP163_DEVICE_NAME(DeviceId),
                     Rc);
            CfyELib_Device_Unlock(DeviceId);
            return CFYE_DLIB_ERROR(Rc);
        }
    }

    CfyELib_Device_Unlock(DeviceId);

    return CFYE_STATUS_OK;
}


/*-----------------------------------------------------------------------------
 * CfyE_Device_Status_Get
 */
CfyE_Status_t
CfyE_Device_Status_Get(
        const unsigned int DeviceId,
        CfyE_DeviceStatus_t * const DeviceStatus_p)
{
    EIP163_Error_t Rc;
    EIP163_Device_PktProcessDebug_t PktProcessDebug;
    EIP163_Device_ECCStatus_t ECCStatus[EIP163_ECC_NOF_STATUS_COUNTERS];
    unsigned int i;

    CfyELib_Device_Lock(DeviceId);

    if (!CfyELib_Initialized_Check(DeviceId, __func__, true))
    {
        CfyELib_Device_Unlock(DeviceId);
        return CFYE_ERROR_INTERNAL;
    }

    if (DeviceStatus_p->ECCStatus_p)
    {
        CfyE_ECCStatus_t *ECCStatus_p = DeviceStatus_p->ECCStatus_p;
        LOG_INFO("\n\t EIP163_Device_ECCStatus_Get \n");

        Rc = EIP163_Device_ECCStatus_Get(&PRV_SHARED_MACSEC_CFYE_GLOBAL_VAR_GET(CfyE_Devices)[DeviceId]->IOArea,
                                         ECCStatus);
        if (Rc != EIP163_NO_ERROR)
        {
            LOG_CRIT("%s: Failed to read ECC status EIP-163 device for device %d "
                     "(%s), error %d\n",
                     __func__,
                     DeviceId,
                     ADAPTER_EIP163_DEVICE_NAME(DeviceId),
                     Rc);
            CfyELib_Device_Unlock(DeviceId);
            return CFYE_DLIB_ERROR(Rc);
        }

        for (i=0; i<EIP163_ECC_NOF_STATUS_COUNTERS; i++)
        {
            ECCStatus_p->Counters[i].CorrectableCount = ECCStatus[i].CorrectableCount;
            ECCStatus_p->Counters[i].UncorrectableCount = ECCStatus[i].UncorrectableCount;
            ECCStatus_p->Counters[i].fCorrectableThr = ECCStatus[i].fCorrectableThr;
            ECCStatus_p->Counters[i].fUncorrectableThr = ECCStatus[i].fUncorrectableThr;
        }
    }

    if (DeviceStatus_p->PktProcessDebug_p)
    {
        CfyE_PktProcessDebug_t *PktProcessDebug_p = DeviceStatus_p->PktProcessDebug_p;
        LOG_INFO("\n\t EIP163_Device_PktProcessDebug_Get \n");

        Rc = EIP163_Device_PktProcessDebug_Get(&PRV_SHARED_MACSEC_CFYE_GLOBAL_VAR_GET(CfyE_Devices)[DeviceId]->IOArea,
                                          &PktProcessDebug);
        if (Rc != EIP163_NO_ERROR)
        {
            LOG_CRIT("%s: Failed to read packet process debug EIP-163 device for device %d "
                     "(%s), error %d\n",
                     __func__,
                     DeviceId,
                     ADAPTER_EIP163_DEVICE_NAME(DeviceId),
                     Rc);
            CfyELib_Device_Unlock(DeviceId);
            return CFYE_DLIB_ERROR(Rc);
        }
        PktProcessDebug_p->CPMatchDebug = PktProcessDebug.CPMatchDebug;
        PktProcessDebug_p->TCAMDebug = PktProcessDebug.TCAMDebug;
        PktProcessDebug_p->SecTAGDebug = PktProcessDebug.SecTAGDebug;
        PktProcessDebug_p->SAMPPDebug1 = PktProcessDebug.SAMPPDebug1;
        PktProcessDebug_p->SAMPPDebug2 = PktProcessDebug.SAMPPDebug2;
        PktProcessDebug_p->SAMPPDebug3 = PktProcessDebug.SAMPPDebug3;
        PktProcessDebug_p->ParsedDALo = PktProcessDebug.ParsedDALo;
        PktProcessDebug_p->ParsedDAHi = PktProcessDebug.ParsedDAHi;
        PktProcessDebug_p->ParsedSALo = PktProcessDebug.ParsedSALo;
        PktProcessDebug_p->ParsedSAHi = PktProcessDebug.ParsedSAHi;
        PktProcessDebug_p->ParsedSecTAGLo = PktProcessDebug.ParsedSecTAGLo;
        PktProcessDebug_p->ParsedSecTAGHi = PktProcessDebug.ParsedSecTAGHi;
        PktProcessDebug_p->DebugFlowLookup = PktProcessDebug.DebugFlowLookup;
    }

    if (DeviceStatus_p->fExternalTCAM_p)
    {
        *DeviceStatus_p->fExternalTCAM_p =
            PRV_SHARED_MACSEC_CFYE_GLOBAL_VAR_GET(CfyE_Devices)[DeviceId]->fExternalTCAM;
    }

    CfyELib_Device_Unlock(DeviceId);

    return CFYE_STATUS_OK;
}


/*-----------------------------------------------------------------------------
 * CfyE_Device_InsertSOP
 */
CfyE_Status_t
CfyE_Device_InsertSOP(
        const unsigned int DeviceId,
        const CfyE_Ch_Mask_t * const ChannelMask_p)
{
    EIP163_Error_t Rc;

    LOG_INFO("\n %s \n", __func__);

#ifdef ADAPTER_EIP163_STRICT_ARGS
    if (DeviceId >= ADAPTER_EIP163_MAX_NOF_DEVICES)
        return CFYE_ERROR_BAD_PARAMETER;
#endif

    ADAPTER_EIP163_CHECK_POINTER(ChannelMask_p);

    CfyELib_Device_Lock(DeviceId);

    if (!PRV_SHARED_MACSEC_CFYE_GLOBAL_VAR_GET(CfyE_Device_StaticFields)[DeviceId].fInitialized)
    {
        CfyELib_Device_Unlock(DeviceId);
        return CFYE_ERROR_INTERNAL;
    }

    LOG_INFO("\n\t EIP163_Device_InsertSOP\n");

    Rc = EIP163_Device_InsertSOP(&PRV_SHARED_MACSEC_CFYE_GLOBAL_VAR_GET(CfyE_Devices)[DeviceId]->IOArea,
                                 (const EIP163_Ch_Mask_t * const) ChannelMask_p);
    if (Rc != EIP163_NO_ERROR)
    {
        LOG_CRIT("%s: Failed to insert SOP "
                 "for EIP-163 device for device %d\n",
                 __func__, DeviceId);
        CfyELib_Device_Unlock(DeviceId);
        return CFYE_DLIB_ERROR(Rc);
    }

    CfyELib_Device_Unlock(DeviceId);

    LOG_INFO("\n %s done \n", __func__);

    return CFYE_STATUS_OK;
}


/*-----------------------------------------------------------------------------
 * CfyE_Device_InsertEOP
 */
CfyE_Status_t
CfyE_Device_InsertEOP(
        const unsigned int DeviceId,
        const CfyE_Ch_Mask_t * const ChannelMask_p)
{
    EIP163_Error_t Rc;

    LOG_INFO("\n %s \n", __func__);

#ifdef ADAPTER_EIP163_STRICT_ARGS
    if (DeviceId >= ADAPTER_EIP163_MAX_NOF_DEVICES)
        return CFYE_ERROR_BAD_PARAMETER;
#endif

    ADAPTER_EIP163_CHECK_POINTER(ChannelMask_p);
    CfyELib_Device_Lock(DeviceId);

    if (!PRV_SHARED_MACSEC_CFYE_GLOBAL_VAR_GET(CfyE_Device_StaticFields)[DeviceId].fInitialized)
    {
        CfyELib_Device_Unlock(DeviceId);
        return CFYE_ERROR_INTERNAL;
    }

    LOG_INFO("\n\t EIP163_Device_InsertEOP\n");

    Rc = EIP163_Device_InsertEOP(&PRV_SHARED_MACSEC_CFYE_GLOBAL_VAR_GET(CfyE_Devices)[DeviceId]->IOArea,
                                 (const EIP163_Ch_Mask_t * const) ChannelMask_p);
    if (Rc != EIP163_NO_ERROR)
    {
        LOG_CRIT("%s: Failed to insert EOP "
                 "for EIP-163 device for device %d\n",
                 __func__, DeviceId);
        CfyELib_Device_Unlock(DeviceId);
        return CFYE_DLIB_ERROR(Rc);
    }

    CfyELib_Device_Unlock(DeviceId);

    LOG_INFO("\n %s done \n", __func__);

    return CFYE_STATUS_OK;
}


/*----------------------------------------------------------------------------
 * CfyE_Channel_Bypass_Get
 */
CfyE_Status_t
CfyE_Channel_Bypass_Get(
        const unsigned int DeviceId,
        const unsigned int ChannelId,
        bool * const fBypass_p)
{
    EIP163_Error_t Rc;
    EIP163_Device_Control_t DeviceControl;

    LOG_INFO("\n %s \n", __func__);

    ZEROINIT(DeviceControl);

#ifdef ADAPTER_EIP163_STRICT_ARGS
    if (DeviceId >= ADAPTER_EIP163_MAX_NOF_DEVICES)
        return CFYE_ERROR_BAD_PARAMETER;
#endif

    CfyELib_Device_Lock(DeviceId);

    if (!PRV_SHARED_MACSEC_CFYE_GLOBAL_VAR_GET(CfyE_Device_StaticFields)[DeviceId].fInitialized)
    {
        CfyELib_Device_Unlock(DeviceId);
        return CFYE_ERROR_INTERNAL;
    }

    LOG_INFO("\n\t EIP163_Device_InsertEOP\n");

    Rc = EIP163_Device_Control_Read(&PRV_SHARED_MACSEC_CFYE_GLOBAL_VAR_GET(CfyE_Devices)[DeviceId]->IOArea,
                                    ChannelId,
                                    &DeviceControl);
    if (Rc != EIP163_NO_ERROR)
    {
        LOG_CRIT("%s: Failed to read Device Control "
                 "for EIP-163 device for device %d\n",
                 __func__, DeviceId);
        CfyELib_Device_Unlock(DeviceId);
        return CFYE_DLIB_ERROR(Rc);
    }

    *fBypass_p = DeviceControl.fLowLatencyBypass;

    CfyELib_Device_Unlock(DeviceId);

    LOG_INFO("\n %s done \n", __func__);

    return CFYE_STATUS_OK;
}


/*----------------------------------------------------------------------------
 * CfyE_Channel_Bypass_Set
 */
CfyE_Status_t
CfyE_Channel_Bypass_Set(
        const unsigned int DeviceId,
        const unsigned int ChannelId,
        const bool fBypass)
{
    EIP163_Error_t Rc;
    EIP163_Device_Control_t DeviceControl;

    LOG_INFO("\n %s \n", __func__);

    ZEROINIT(DeviceControl);

#ifdef ADAPTER_EIP163_STRICT_ARGS
    if (DeviceId >= ADAPTER_EIP163_MAX_NOF_DEVICES)
        return CFYE_ERROR_BAD_PARAMETER;
#endif

    CfyELib_Device_Lock(DeviceId);

    if (!PRV_SHARED_MACSEC_CFYE_GLOBAL_VAR_GET(CfyE_Device_StaticFields)[DeviceId].fInitialized)
    {
        CfyELib_Device_Unlock(DeviceId);
        return CFYE_ERROR_INTERNAL;
    }
    DeviceControl.fLowLatencyBypass = fBypass;

    LOG_INFO("\n\t EIP163_Device_InsertEOP\n");

    Rc = EIP163_Device_Update(&PRV_SHARED_MACSEC_CFYE_GLOBAL_VAR_GET(CfyE_Devices)[DeviceId]->IOArea,
                              ChannelId,
                              &DeviceControl);
    if (Rc != EIP163_NO_ERROR)
    {
        LOG_CRIT("%s: Failed to read Device Control "
                 "for EIP-163 device for device %d\n",
                 __func__, DeviceId);
        CfyELib_Device_Unlock(DeviceId);
        return CFYE_DLIB_ERROR(Rc);
    }


    CfyELib_Device_Unlock(DeviceId);

    LOG_INFO("\n %s done \n", __func__);

    return CFYE_STATUS_OK;
}


/* end of file adapter_cfye.c */
