/* adapter_cfye_warmboot.c
 *
 * CfyE WarmBoot implementation
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

#include <Integration/Adapter_EIP163/incl/adapter_cfye_warmboot.h>
/* Internal adapter API */
#include <Integration/Adapter_EIP163/incl/adapter_cfye_support.h>

/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

/* Top-level Adapter configuration */
#include <Integration/Adapter_EIP163/incl/c_adapter_eip163.h>       /* ADAPTER_EIP163_DRIVER_NAME */

/* Logging API */
#include <Kit/Log/incl/log.h>               /* LOG_* */

/* Driver Framework Device API */
#include <Kit/DriverFramework/incl/device_types.h>
#include <Kit/DriverFramework/incl/device_mgmt.h>

/* Driver Framework Run-Time C Library Abstraction API */
#include <Kit/DriverFramework/incl/clib.h>  /* cpssOsMemSet */ 

/* EIP-163 Driver Library API */
#include <Kit/EIP163/incl/eip163.h>

/* Adapter Alloc API */
#include <Integration/Adapter_EIP164/incl/adapter_lock.h>   /* Adapter_Alloc()/_Free() */

#ifdef ADAPTER_EIP163_INTERRUPTS_ENABLE
/* Adapter Interrupts API */
#include <Integration/Adapter_EIP164/incl/adapter_interrupts.h>
#endif

/* WarmBoot internal API. */
#ifdef ADAPTER_EIP163_WARMBOOT_ENABLE
#include <Integration/Adapter_EIP164/incl/adapter_warmboot_support.h>
#endif

/*----------------------------------------------------------------------------
 * Definitions and macros
 */

#ifdef ADAPTER_EIP163_WARMBOOT_ENABLE

/* Number of bytes to allocate in Warmboot storage area */
#define CFYE_WARMBOOT_BASE_BYTE_COUNT 7

/* Number of bytes for the WarmBoot vPort bitmap */
#define CFYE_WARMBOOT_VPORT_BYTE_COUNT(n)   (((n)+7)/8)

/* Number of bytes for the WarmBoot Rule bitmap */
#define CFYE_WARMBOOT_RULE_BYTE_COUNT(n)    (((n)+7)/8)


/*----------------------------------------------------------------------------
 * CfyELib_WarmBoot_vPort_Set
 *
 * Mark vPort as allocated.
 */
void
CfyELib_WarmBoot_vPort_Set(
        unsigned int DeviceId,
        unsigned int vPortIndex)
{
    unsigned int ByteIndex = vPortIndex / 8;
    unsigned int BitMask = 1<<(vPortIndex % 8);

    CfyE_Devices[DeviceId]->WarmBoot_vPort_Bitmap[ByteIndex] |= BitMask;

    WarmBootLib_Write(CfyE_Devices[DeviceId]->WarmBoot_AreaId,
                      CfyE_Devices[DeviceId]->WarmBoot_vPort_Bitmap + ByteIndex,
                      CFYE_WARMBOOT_BASE_BYTE_COUNT + ByteIndex,
                      1);
}


/*----------------------------------------------------------------------------
 * CfyELib_WarmBoot_vPort_Clear
 *
 * Mark vPort as free.
 */
void
CfyELib_WarmBoot_vPort_Clear(
        unsigned int DeviceId,
        unsigned int vPortIndex)
{
    unsigned int ByteIndex = vPortIndex / 8;
    unsigned int BitMask = 1<<(vPortIndex % 8);

    CfyE_Devices[DeviceId]->WarmBoot_vPort_Bitmap[ByteIndex] &= ~BitMask;

    WarmBootLib_Write(CfyE_Devices[DeviceId]->WarmBoot_AreaId,
                      CfyE_Devices[DeviceId]->WarmBoot_vPort_Bitmap + ByteIndex,
                      CFYE_WARMBOOT_BASE_BYTE_COUNT + ByteIndex,
                      1);
}


/*----------------------------------------------------------------------------
 * CfyELib_WarmBoot_Rule_Set
 *
 * Mark Rule as allocated.
 */
void
CfyELib_WarmBoot_Rule_Set(
        unsigned int DeviceId,
        unsigned int RuleIndex)
{
    unsigned int ByteIndex = RuleIndex / 8;
    unsigned int BitMask = 1<<(RuleIndex % 8);

    CfyE_Devices[DeviceId]->WarmBoot_Rule_Bitmap[ByteIndex] |= BitMask;

    WarmBootLib_Write(CfyE_Devices[DeviceId]->WarmBoot_AreaId,
                      CfyE_Devices[DeviceId]->WarmBoot_Rule_Bitmap + ByteIndex,
                      CFYE_WARMBOOT_BASE_BYTE_COUNT +
                      CFYE_WARMBOOT_VPORT_BYTE_COUNT(CfyE_Devices[DeviceId]->vPortCount) +
                      ByteIndex,
                      1);
}


/*----------------------------------------------------------------------------
 * CfyELib_WarmBoot_Rule_Clear
 *
 * Mark Rule as free.
 */
void
CfyELib_WarmBoot_Rule_Clear(
        unsigned int DeviceId,
        unsigned int RuleIndex)
{
    unsigned int ByteIndex = RuleIndex / 8;
    unsigned int BitMask = 1<<(RuleIndex % 8);

    CfyE_Devices[DeviceId]->WarmBoot_Rule_Bitmap[ByteIndex] &= ~BitMask;

    WarmBootLib_Write(CfyE_Devices[DeviceId]->WarmBoot_AreaId,
                      CfyE_Devices[DeviceId]->WarmBoot_Rule_Bitmap + ByteIndex,
                      CFYE_WARMBOOT_BASE_BYTE_COUNT +
                      CFYE_WARMBOOT_VPORT_BYTE_COUNT(CfyE_Devices[DeviceId]->vPortCount) +
                      ByteIndex,
                      1);
}


/*----------------------------------------------------------------------------
 * CfyELib_WarmBoot_vPort_Get
 *
 * Check if a vPort is allocated
 */
static inline bool
CfyELib_WarmBoot_vPort_Get(
        unsigned int DeviceId,
        unsigned int vPortIndex)
{
    unsigned int ByteIndex = vPortIndex / 8;
    unsigned int BitMask = 1<<(vPortIndex % 8);

    return (CfyE_Devices[DeviceId]->WarmBoot_vPort_Bitmap[ByteIndex] & BitMask) != 0;

}


/*----------------------------------------------------------------------------
 * CfyELib_WarmBoot_Rule_Get
 *
 * Check if a Rule is allocated
 */
static inline bool
CfyELib_WarmBoot_Rule_Get(
        unsigned int DeviceId,
        unsigned int RuleIndex)
{
    unsigned int ByteIndex = RuleIndex / 8;
    unsigned int BitMask = 1<<(RuleIndex % 8);

    return (CfyE_Devices[DeviceId]->WarmBoot_Rule_Bitmap[ByteIndex] & BitMask) != 0;

}


/*----------------------------------------------------------------------------
 * CfyELib_WarmBoot_MaxSize_Get
 */
bool
CfyELib_WarmBoot_MaxSize_Get(
        const unsigned int DeviceId,
        const bool fIngress,
        unsigned int *MaxByteCount_p)
{
    Device_Handle_t Device;
    EIP163_Capabilities_t Cp;
    EIP163_Error_t Rc;

    IDENTIFIER_NOT_USED(fIngress);

    ADAPTER_EIP163_DEVICE_NAME(DeviceId) =
                    Device_GetName(ADAPTER_EIP163_DEV_ID(DeviceId));

    /* Find the EIP-163 device */
    Device = Device_Find(ADAPTER_EIP163_DEVICE_NAME(DeviceId));
    if (Device == NULL)
    {
        LOG_CRIT("%s: Failed to locate EIP-163 device for device %d (%s)\n",
                 __func__, DeviceId, ADAPTER_EIP163_DEVICE_NAME(DeviceId));
        return false;
    }

    /* Retrieve device capabilities */
    Rc = EIP163_HWRevision_Get(Device, &Cp);
    if (Rc != EIP163_NO_ERROR)
    {
        LOG_CRIT("%s: Failed to get EIP-163 device capabilities for device %d,"
                 " error %d\n",
                 __func__, DeviceId, Rc);
        return false;
    }

    *MaxByteCount_p = CFYE_WARMBOOT_BASE_BYTE_COUNT +
        CFYE_WARMBOOT_VPORT_BYTE_COUNT(Cp.EIP163_Options.vPort_Count) +
        CFYE_WARMBOOT_RULE_BYTE_COUNT(Cp.EIP163_Options.Rules_Count);

    return true;
}


/*-----------------------------------------------------------------------------
 * CfyELib_WarmBoot_Device_Init
 */
bool
CfyELib_WarmBoot_Device_init(
        unsigned int DeviceId)
{
    unsigned char RoleVal = (uint8_t)CfyE_Devices[DeviceId]->Role << 2;
    unsigned char DeviceInfo[7];

    CfyE_Devices[DeviceId]->WarmBoot_vPort_Bitmap =
        Adapter_Alloc(CFYE_WARMBOOT_VPORT_BYTE_COUNT(CfyE_Devices[DeviceId]->vPortCount));

    if (CfyE_Devices[DeviceId]->WarmBoot_vPort_Bitmap == NULL)
    {
        LOG_CRIT("%s: Failed to allocate WarmBoot vPort bitmap. "
                 "for device %d\n",
                 __func__, DeviceId);
        return false;
    }

    CfyE_Devices[DeviceId]->WarmBoot_Rule_Bitmap =
        Adapter_Alloc(CFYE_WARMBOOT_RULE_BYTE_COUNT(CfyE_Devices[DeviceId]->RulesCount));
    if (CfyE_Devices[DeviceId]->WarmBoot_Rule_Bitmap == NULL)
    {
        LOG_CRIT("%s: Failed to allocate WarmBoot Rule bitmap. "
                 "for device %d\n",
                 __func__, DeviceId);
        Adapter_Free(CfyE_Devices[DeviceId]->WarmBoot_vPort_Bitmap);
        return false;
    }
    DeviceInfo[0] = ((DeviceId & MASK_10_BITS)>>8) | RoleVal| BIT_7;
    DeviceInfo[1] = DeviceId & MASK_8_BITS;
    DeviceInfo[2] = (CfyE_Devices[DeviceId]->RulesCount)>>8;
    DeviceInfo[3] = (CfyE_Devices[DeviceId]->RulesCount) & MASK_8_BITS;
    DeviceInfo[4] = (CfyE_Devices[DeviceId]->vPortCount)>>8;
    DeviceInfo[5] = (CfyE_Devices[DeviceId]->vPortCount) & MASK_8_BITS;
    DeviceInfo[6] = CfyE_Devices[DeviceId]->ChannelsCount & MASK_7_BITS;
    if (CfyE_Devices[DeviceId]->fExternalTCAM)
    {
        DeviceInfo[6] |= BIT_7;
    }
    if ( !WarmBootLib_Alloc(CFYE_WARMBOOT_BASE_BYTE_COUNT +
                            CFYE_WARMBOOT_VPORT_BYTE_COUNT(CfyE_Devices[DeviceId]->vPortCount) +
                            CFYE_WARMBOOT_RULE_BYTE_COUNT(CfyE_Devices[DeviceId]->RulesCount)
                            , &CfyE_Devices[DeviceId]->WarmBoot_AreaId))
    {
        LOG_CRIT("%s: Failed to allocate WarmBoot area. "
                 "for device %d\n",
                     __func__, DeviceId);
        Adapter_Free(CfyE_Devices[DeviceId]->WarmBoot_vPort_Bitmap);
        Adapter_Free(CfyE_Devices[DeviceId]->WarmBoot_Rule_Bitmap);
        return false;
    }

    WarmBootLib_Write(CfyE_Devices[DeviceId]->WarmBoot_AreaId,
                      DeviceInfo, 0, CFYE_WARMBOOT_BASE_BYTE_COUNT);
    cpssOsMemSet(CfyE_Devices[DeviceId]->WarmBoot_vPort_Bitmap, 0,
           CFYE_WARMBOOT_VPORT_BYTE_COUNT(CfyE_Devices[DeviceId]->vPortCount));
    cpssOsMemSet(CfyE_Devices[DeviceId]->WarmBoot_Rule_Bitmap, 0,
           CFYE_WARMBOOT_RULE_BYTE_COUNT(CfyE_Devices[DeviceId]->RulesCount));

    return true;
}


/*-----------------------------------------------------------------------------
 * CfyELib_WarmBoot_Device_Uninit
 */
void
CfyELib_WarmBoot_Device_Uninit(
        unsigned int DeviceId)
{
    unsigned char DeviceByte=0;
    /* Invalidate in case free is a no-op. */
    WarmBootLib_Write(CfyE_Devices[DeviceId]->WarmBoot_AreaId,
                      &DeviceByte,
                      0,
                      1);

    Adapter_Free(CfyE_Devices[DeviceId]->WarmBoot_vPort_Bitmap);
    Adapter_Free(CfyE_Devices[DeviceId]->WarmBoot_Rule_Bitmap);
    WarmBootLib_Free(CfyE_Devices[DeviceId]->WarmBoot_AreaId);
}


/*-----------------------------------------------------------------------------
 * CfyE_WarmBoot_Shutdown
 */
int
CfyELib_WarmBoot_Shutdown(
        unsigned int DeviceId,
        unsigned int AreaId)
{
#ifdef ADAPTER_EIP163_INTERRUPTS_ENABLE
    unsigned int Count;
#endif

    IDENTIFIER_NOT_USED(AreaId);
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

    for (Count = 0; Count < CfyE_Devices[DeviceId]->ChannelsCount; Count++)
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

    /* Uninitialize Rule free list */
    CfyELib_RuleFreeList_Uninit(DeviceId);

    /* Uninitialize vPort free list */
    CfyELib_vPortFreeList_Uninit(DeviceId);


    CfyE_Device_StaticFields[DeviceId].fInitialized = false;

    Adapter_Free(CfyE_Devices[DeviceId]->WarmBoot_vPort_Bitmap);
    Adapter_Free(CfyE_Devices[DeviceId]->WarmBoot_Rule_Bitmap);

    Adapter_Free(CfyE_Devices[DeviceId]);
    CfyE_Devices[DeviceId] = NULL;

    LOG_INFO("%s: device %d successfully uninitialized\n",
             __func__,
             DeviceId);

    CfyELib_Device_Unlock(DeviceId);

    LOG_INFO("\n %s done \n", __func__);

    return 0;
}


/*-----------------------------------------------------------------------------
 * CfyE_WarmBoot_Restore
 */
int
CfyELib_WarmBoot_Restore(
        unsigned int DeviceId,
        unsigned int AreaId)
{
    bool fSuccess;
    EIP163_Error_t EIP163_Rc;
    CfyE_Role_t Role;
    EIP163_Device_Mode_t Mode;
    unsigned char DeviceInfo[7];
    unsigned int vPortCount, RulesCount, ChannelCount;
    Device_Handle_t Device;
    unsigned int i;

    ADAPTER_EIP163_CHECK_INT_ATMOST(DeviceId, ADAPTER_EIP163_MAX_NOF_DEVICES - 1);
    CfyELib_Device_Lock(DeviceId);

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
        return CFYE_ERROR_INTERNAL;
    }

    CfyE_Devices[DeviceId] = Adapter_Alloc(sizeof(CfyE_Device_Internal_t));
    if (CfyE_Devices[DeviceId] == NULL)
    {
        LOG_CRIT("%s: Failed to allocate descriptor for %d (%s)\n",
                 __func__,
                 DeviceId,
                 ADAPTER_EIP163_DEVICE_NAME(DeviceId));
        return CFYE_ERROR_INTERNAL;
    }

    if ( !WarmBootLib_Read(AreaId, DeviceInfo, 0, CFYE_WARMBOOT_BASE_BYTE_COUNT))
    {
        Adapter_Free(CfyE_Devices[DeviceId]);
        CfyELib_Device_Unlock(DeviceId);
        return CFYE_ERROR_INTERNAL;
    }
    RulesCount = (DeviceInfo[2]<<8) + DeviceInfo[3];
    ChannelCount = DeviceInfo[6] & MASK_7_BITS;
    vPortCount = (DeviceInfo[4]<<8) + DeviceInfo[5];
    Role = (CfyE_Role_t)((DeviceInfo[0]>>2)& MASK_2_BITS);
    LOG_INFO("WarmBoot_Restore: CfyE Device=%u Role=%d Rule=%u vPort=%u\n",DeviceId,Role,RulesCount,vPortCount);

    CfyE_Devices[DeviceId]->WarmBoot_vPort_Bitmap =
        Adapter_Alloc(CFYE_WARMBOOT_VPORT_BYTE_COUNT(vPortCount));
    if (CfyE_Devices[DeviceId]->WarmBoot_vPort_Bitmap == NULL)
    {
        Adapter_Free(CfyE_Devices[DeviceId]);
        CfyELib_Device_Unlock(DeviceId);
        return CFYE_ERROR_INTERNAL;
    }

    CfyE_Devices[DeviceId]->WarmBoot_Rule_Bitmap =
        Adapter_Alloc(CFYE_WARMBOOT_RULE_BYTE_COUNT(RulesCount));
    if (CfyE_Devices[DeviceId]->WarmBoot_Rule_Bitmap == NULL)
    {
        Adapter_Free(CfyE_Devices[DeviceId]->WarmBoot_vPort_Bitmap);
        Adapter_Free(CfyE_Devices[DeviceId]);
        CfyELib_Device_Unlock(DeviceId);
        return CFYE_ERROR_INTERNAL;
    }

    CfyE_Devices[DeviceId]->RulesCount = RulesCount;
    CfyE_Devices[DeviceId]->vPortCount = vPortCount;
    CfyE_Devices[DeviceId]->ChannelsCount = ChannelCount;
    CfyE_Devices[DeviceId]->Role = Role;
    CfyE_Devices[DeviceId]->WarmBoot_AreaId = AreaId;
    CfyE_Devices[DeviceId]->fExternalTCAM = (DeviceInfo[6] & BIT_7) != 0;

    WarmBootLib_Read(AreaId,
                     CfyE_Devices[DeviceId]->WarmBoot_vPort_Bitmap,
                     CFYE_WARMBOOT_BASE_BYTE_COUNT,
                     CFYE_WARMBOOT_VPORT_BYTE_COUNT(vPortCount));

    WarmBootLib_Read(AreaId,
                     CfyE_Devices[DeviceId]->WarmBoot_Rule_Bitmap,
                     CFYE_WARMBOOT_BASE_BYTE_COUNT +
                     CFYE_WARMBOOT_VPORT_BYTE_COUNT(vPortCount),
                     CFYE_WARMBOOT_RULE_BYTE_COUNT(RulesCount));
    if (Role == CFYE_ROLE_EGRESS)
        Mode = EIP163_MODE_EGRESS;
    else
        Mode = EIP163_MODE_INGRESS;

    /* Restore the EIP164 IO Area. */
    EIP163_Rc = EIP163_Device_Restore(&CfyE_Devices[DeviceId]->IOArea, Device,
                                      Mode);
    if (EIP163_Rc != EIP163_NO_ERROR)
    {
        LOG_CRIT("%s: Failed to restore EIP-163 device for device %d (%s), "
                 "mode %s, error %d\n",
                 __func__, DeviceId, ADAPTER_EIP163_DEVICE_NAME(DeviceId),
                 (Mode == EIP163_MODE_EGRESS) ? "egress" : "ingress",
                 EIP163_Rc);
        Adapter_Free(CfyE_Devices[DeviceId]->WarmBoot_vPort_Bitmap);
        Adapter_Free(CfyE_Devices[DeviceId]->WarmBoot_Rule_Bitmap);
        Adapter_Free(CfyE_Devices[DeviceId]);
        CfyE_Devices[DeviceId] = NULL;

        CfyELib_Device_Unlock(DeviceId);
        return EIP163_Rc;
    }

    /* Initialize the vPort free list for this device */
    fSuccess = false;
    {
        List_Status_t List_Rc;

        /* Allocate vPort free list */
        CfyE_Devices[DeviceId]->vPortFreeList_p =
                Adapter_Alloc(List_GetInstanceByteCount());
        if (CfyE_Devices[DeviceId]->vPortFreeList_p == NULL)
        {
            LOG_CRIT("%s: Failed to allocate vPort free list "
                     "for EIP-163 device %d\n",
                     __func__,
                     DeviceId);
            Adapter_Free(CfyE_Devices[DeviceId]->WarmBoot_vPort_Bitmap);
            Adapter_Free(CfyE_Devices[DeviceId]->WarmBoot_Rule_Bitmap);
            Adapter_Free(CfyE_Devices[DeviceId]);
            CfyE_Devices[DeviceId] = NULL;
            CfyELib_Device_Unlock(DeviceId);
            return CFYE_ERROR_INTERNAL;
        }

        List_Rc = List_Init(0, CfyE_Devices[DeviceId]->vPortFreeList_p);
        if (List_Rc != LIST_STATUS_OK)
        {
            LOG_CRIT("%s: Failed to initialize vPort free list for EIP-163 "
                     "device for device %d\n",
                     __func__,
                     DeviceId);
            goto lError;
        }

        /* Allocate vPort descriptors for this device */
        CfyE_Devices[DeviceId]->vPortDscr_p =
                Adapter_Alloc(CfyE_Devices[DeviceId]->vPortCount *
                                           sizeof (CfyE_vPort_Descriptor_Internal_t));
        if (CfyE_Devices[DeviceId]->vPortDscr_p == NULL)
        {
            LOG_CRIT("%s: Failed to allocate vPort descriptors for EIP-163 "
                     "device for device %d\n",
                     __func__,
                     DeviceId);
            goto lError;
        }

        for (i = 0; i < CfyE_Devices[DeviceId]->vPortCount; i++)
        {
            if (CfyELib_WarmBoot_vPort_Get(DeviceId, i) == 0)
            {
                CfyE_Devices[DeviceId]->vPortDscr_p[i].Magic = 0;

                /* Add the vPort descriptor as an element to the vPort free list */
                CfyE_Devices[DeviceId]->vPortDscr_p[i].u.free.DataObject_p =
                    &CfyE_Devices[DeviceId]->vPortDscr_p[i];

                List_Rc = List_AddToHead(0,
                                     CfyE_Devices[DeviceId]->vPortFreeList_p,
                                     &CfyE_Devices[DeviceId]->vPortDscr_p[i].u.free);
                if (List_Rc != LIST_STATUS_OK)
                {
                    LOG_CRIT("%s: Failed to create vPort free list for EIP-163 "
                         "device for device %d\n",
                             __func__,
                         DeviceId);
                    goto lError;
                }
            }
            else
            {
                /* vPort is allocated. */
                LOG_INFO("WarmBoot_Restore: vPort %u in use\n", i);
                CfyE_Devices[DeviceId]->vPortDscr_p[i].u.InUse.vPortPolicyId = i;
                CfyE_Devices[DeviceId]->vPortDscr_p[i].u.InUse.BoundRulesCount = 0;


                CfyE_Devices[DeviceId]->vPortDscr_p[i].Magic =
                    ADAPTER_EIP163_VPORT_DSCR_MAGIC;
            }
        }

        /* Allocate Rule free list */
        CfyE_Devices[DeviceId]->RuleFreeList_p =
                Adapter_Alloc(List_GetInstanceByteCount());
        if (CfyE_Devices[DeviceId]->RuleFreeList_p == NULL)
        {
            LOG_CRIT("%s: Failed to allocate Rule free list "
                     "for EIP-163 device %d\n",
                     __func__,
                     DeviceId);
            goto lError;
        }

        List_Rc = List_Init(0, CfyE_Devices[DeviceId]->RuleFreeList_p);
        if (List_Rc != LIST_STATUS_OK)
        {
            LOG_CRIT("%s: Failed to initialize Rule free list for EIP-163 "
                     "device for device %d\n",
                     __func__,
                     DeviceId);
            goto lError;
        }

        /* Allocate Rule descriptors for this device */
        CfyE_Devices[DeviceId]->RuleDscr_p =
                Adapter_Alloc(CfyE_Devices[DeviceId]->RulesCount *
                                           sizeof (CfyE_Rule_Descriptor_Internal_t));
        if (CfyE_Devices[DeviceId]->RuleDscr_p == NULL)
        {
            LOG_CRIT("%s: Failed to allocate Rule descriptors for EIP-163 "
                     "device for device %d\n",
                     __func__,
                     DeviceId);
            goto lError;
        }

        /* Initialize all rule descriptors, build free list. */
        for (i = 0; i < CfyE_Devices[DeviceId]->RulesCount; i++)
        {
            EIP163_Rule_Policy_t RulePolicy;
            unsigned int vPort;
            bool fRuleEnabled;

            EIP163_Rc = EIP163_Rule_Policy_Read(&CfyE_Devices[DeviceId]->IOArea,
                                                i,
                                                &RulePolicy,
                                                &fRuleEnabled);
            if (EIP163_Rc != EIP163_NO_ERROR)
            {
                LOG_CRIT("%s: Failed to read EIP-163 rule %d rc=$d"
                         "device for device %d\n",
                         __func__,
                         i,
                         EIP163_Rc);
                goto lError;
            }

            if (CfyELib_WarmBoot_Rule_Get(DeviceId, i) == 0)
            {
                CfyE_Devices[DeviceId]->RuleDscr_p[i].Magic = 0;

                /* Add the Rule descriptor as an element to the Rule free list */
                CfyE_Devices[DeviceId]->RuleDscr_p[i].u.free.DataObject_p =
                    &CfyE_Devices[DeviceId]->RuleDscr_p[i];

                List_Rc = List_AddToHead(0,
                                     CfyE_Devices[DeviceId]->RuleFreeList_p,
                                     &CfyE_Devices[DeviceId]->RuleDscr_p[i].u.free);
                if (List_Rc != LIST_STATUS_OK)
                {
                    LOG_CRIT("%s: Failed to create Rule free list for EIP-163 "
                         "device for device %d\n",
                             __func__,
                         DeviceId);
                    goto lError;
                }
            }
            else
            {
                /* Active rule found. */
                vPort = RulePolicy.vPortId;
                LOG_INFO("WarmBoot_Restore: Rule %u for vPort %u\n",i,vPort);
                CfyE_Devices[DeviceId]->RuleDscr_p[i].u.InUse.RuleId = i;
                CfyE_Devices[DeviceId]->RuleDscr_p[i].u.InUse.vPortDscr_p =
                    &CfyE_Devices[DeviceId]->vPortDscr_p[vPort];
                CfyE_Devices[DeviceId]->RuleDscr_p[i].Magic =
                    ADAPTER_EIP163_RULE_DSCR_MAGIC;

                CfyE_Devices[DeviceId]->vPortDscr_p[vPort].u.InUse.BoundRulesCount++;
            }
        }

    } /* List initialization done */


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
        for (i = 0; i < CfyE_Devices[DeviceId]->ChannelsCount; i++)
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
            for (i = 0; i < CfyE_Devices[DeviceId]->ChannelsCount; i++)
            {
                Adapter_Interrupts_UnInit(CfyELib_ICDeviceID_Get(DeviceId, &i));
            }
            EIP163_Rc = rc;
            goto lError;
        }
    }
#endif /* ADAPTER_EIP163_INTERRUPTS_ENABLE */


    fSuccess = true;
lError:
    if (!fSuccess)
    {
        /* Free vPort descriptors. */
        Adapter_Free(CfyE_Devices[DeviceId]->vPortDscr_p);
        CfyE_Devices[DeviceId]->vPortDscr_p = NULL;

        /* Free vPort free list */
        if (CfyE_Devices[DeviceId]->vPortFreeList_p)
        {
            List_Uninit(0, CfyE_Devices[DeviceId]->vPortFreeList_p);
            Adapter_Free(CfyE_Devices[DeviceId]->vPortFreeList_p);
            CfyE_Devices[DeviceId]->vPortFreeList_p = NULL;
        }

        /* Free Rule descriptors */
        Adapter_Free(CfyE_Devices[DeviceId]->RuleDscr_p);
        CfyE_Devices[DeviceId]->RuleDscr_p = NULL;

        /* Free Rule free list */
        if (CfyE_Devices[DeviceId]->RuleFreeList_p)
        {
            List_Uninit(0, CfyE_Devices[DeviceId]->RuleFreeList_p);
            Adapter_Free(CfyE_Devices[DeviceId]->RuleFreeList_p);
            CfyE_Devices[DeviceId]->RuleFreeList_p = NULL;
        }

        Adapter_Free(CfyE_Devices[DeviceId]->WarmBoot_vPort_Bitmap);
        Adapter_Free(CfyE_Devices[DeviceId]->WarmBoot_Rule_Bitmap);
        Adapter_Free(CfyE_Devices[DeviceId]);
        CfyE_Devices[DeviceId] = NULL;
        CfyELib_Device_Unlock(DeviceId);
        if (EIP163_Rc)
            return EIP163_Rc;
        else
            return CFYE_ERROR_INTERNAL;
    }

    CfyE_Device_StaticFields[DeviceId].fInitialized = true;

    LOG_INFO("%s: device %d successfully initialized\n", __func__, DeviceId);

    CfyELib_Device_Unlock(DeviceId);

    LOG_INFO("\n %s done \n", __func__);


    return 0;
}
#endif


/* end of file adapter_cfye_warmboot.c */
