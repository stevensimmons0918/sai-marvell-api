/* adapter_secy_warmboot.c
 *
 * SecY WarmBoot implementation
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

#include <Integration/Adapter_EIP164/incl/adapter_secy_warmboot.h> /* SecY WarmBoot functions. */

/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

/* Top-level Adapter configuration */
#include <Integration/Adapter_EIP164/incl/c_adapter_eip164.h>             /* ADAPTER_EIP164_DRIVER_NAME */

/* Logging API */
#include <Kit/Log/incl/log.h>                /* LOG_* */

/* Adapter internal SecY API */
#include <Integration/Adapter_EIP164/incl/adapter_secy_support.h>

/* Driver Framework Device API */
#include <Kit/DriverFramework/incl/device_types.h>
#include <Kit/DriverFramework/incl/device_mgmt.h>

/* Driver Framework C Library Abstraction API */
#include <Kit/DriverFramework/incl/clib.h>               /* cpssOsMemSet */

/* SA Builder API */
#include <Kit/SABuilder_MACsec/incl/sa_builder_macsec.h>

/* EIP-164 Driver Library SecY API */
#include <Kit/EIP164/incl/eip164_types.h>
#include <Kit/EIP164/incl/eip164_secy.h>

/* Adapter Alloc API */
#include <Integration/Adapter_EIP164/incl/adapter_alloc.h>      /* Adapter_Alloc()/_Free() */

#ifdef ADAPTER_EIP164_INTERRUPTS_ENABLE
/* Adapter Interrupts API */
#include <Integration/Adapter_EIP164/incl/adapter_interrupts.h>
#endif

/* List API */
#include <Kit/List/incl/list.h>

/* WarmBoot internal API. */
#ifdef ADAPTER_EIP164_WARMBOOT_ENABLE
#include <Integration/Adapter_EIP164/incl/adapter_warmboot_support.h>
#endif

/*----------------------------------------------------------------------------
 * Definitions and macros
 */


#ifdef ADAPTER_EIP164_WARMBOOT_ENABLE

/* Number of bytes to allocate in Warmboot storage area */
#define SECY_WARMBOOT_BASE_BYTE_COUNT 9

/* Number of bytes for the WarmBoot SA bitmap */
#define SECY_WARMBOOT_SA_BYTE_COUNT(n) (((n) + 7) / 8)


/*----------------------------------------------------------------------------
 * SecYLib_WarmBoot_MaxSize_Get
 */
bool
SecYLib_WarmBoot_MaxSize_Get(
    const unsigned int DeviceId,
    const bool fIngress,
    unsigned int *MaxByteCount_p)
{
    Device_Handle_t Device;
    EIP164_Capabilities_t Cp;
    EIP164_Error_t Rc;

    IDENTIFIER_NOT_USED(fIngress);

    ADAPTER_EIP164_DEVICE_NAME(DeviceId) =
                    Device_GetName(ADAPTER_EIP164_DEV_ID(DeviceId));

    /* Find the EIP-164 device */
    Device = Device_Find(ADAPTER_EIP164_DEVICE_NAME(DeviceId));
    if (Device == NULL)
    {
        LOG_CRIT("%s: Failed to locate EIP-164 device for device %d (%s)\n",
                 __func__, DeviceId, ADAPTER_EIP164_DEVICE_NAME(DeviceId));
        return false;
    }

    /* Retrieve device capabilities */
    Rc = EIP164_HWRevision_Get(Device, &Cp);
    if (Rc != EIP164_NO_ERROR)
    {
        LOG_CRIT("%s: Failed to get EIP-164 device capabilities for device %d,"
                 " error %d\n",
                 __func__, DeviceId, Rc);
        return false;
    }

    *MaxByteCount_p = SECY_WARMBOOT_BASE_BYTE_COUNT +
                      SECY_WARMBOOT_SA_BYTE_COUNT(Cp.EIP164_Options.SA_Count);

    return true;
}


/*----------------------------------------------------------------------------
 * SecYLib_WarmBoot_Device_Init
 */
bool
SecYLib_WarmBoot_Device_Init(
    unsigned int DeviceId)
{
    uint8_t RoleVal = (uint8_t)SecY_Device[DeviceId]->Role << 2;
    unsigned char DeviceInfo[9];

    SecY_Device[DeviceId]->WarmBoot_SA_Bitmap =
        Adapter_Alloc(SECY_WARMBOOT_SA_BYTE_COUNT(SecY_Device[DeviceId]->SACount));
    if (SecY_Device[DeviceId]->WarmBoot_SA_Bitmap == NULL)
    {
        LOG_CRIT("%s: Failed to allocate WarmBoot SA bitmap. "
                 "for device %d\n",
                 __func__, DeviceId);
        return false;
    }


    DeviceInfo[0] = ((DeviceId & MASK_10_BITS) >> 8) | RoleVal | BIT_6;
    DeviceInfo[1] = DeviceId & MASK_8_BITS;
    DeviceInfo[2] = (SecY_Device[DeviceId]->SACount) >> 8;
    DeviceInfo[3] = (SecY_Device[DeviceId]->SACount) & MASK_8_BITS;
    DeviceInfo[4] = ((SecY_Device[DeviceId]->SCCount) >> 8);
    DeviceInfo[5] = (SecY_Device[DeviceId]->SCCount) & MASK_8_BITS;
    DeviceInfo[6] = ((SecY_Device[DeviceId]->vPortCount) >> 8);
    DeviceInfo[7] = (SecY_Device[DeviceId]->vPortCount) & MASK_8_BITS;
    DeviceInfo[8] = SecY_Device[DeviceId]->ChannelCount;

    if (!WarmBootLib_Alloc(SECY_WARMBOOT_BASE_BYTE_COUNT +
                           SECY_WARMBOOT_SA_BYTE_COUNT(SecY_Device[DeviceId]->SACount)
                           , &SecY_Device[DeviceId]->WarmBoot_AreaId))
    {
        LOG_CRIT("%s: Failed to allocate WarmBoot area. "
                 "for device %d\n",
                 __func__, DeviceId);

        Adapter_Free(SecY_Device[DeviceId]->WarmBoot_SA_Bitmap);

        return false;
    }

    WarmBootLib_Write(SecY_Device[DeviceId]->WarmBoot_AreaId,
                      DeviceInfo, 0, SECY_WARMBOOT_BASE_BYTE_COUNT);
    cpssOsMemSet(SecY_Device[DeviceId]->WarmBoot_SA_Bitmap,
           0,
           SECY_WARMBOOT_SA_BYTE_COUNT(SecY_Device[DeviceId]->SACount));


    return true;
}


/*----------------------------------------------------------------------------
 * SecYLib_WarmBoot_Device_Uninit
 */
void
SecYLib_WarmBoot_Device_Uninit(
        unsigned int DeviceId)
{
    /* Invalidate in case free is a no-op. */
    unsigned char DeviceByte = 0;
    WarmBootLib_Write(SecY_Device[DeviceId]->WarmBoot_AreaId,
                      &DeviceByte,
                      0,
                      1);

    Adapter_Free(SecY_Device[DeviceId]->WarmBoot_SA_Bitmap);
    WarmBootLib_Free(SecY_Device[DeviceId]->WarmBoot_AreaId);
}


/*----------------------------------------------------------------------------
 * SecYLib_WarmBoot_SA_Set
 *
 * Mark SA as allocated for the purpose of the WarmBoot administration.
 *
 * DeviceId (input)
 *      ID of the selected device.
 *
 * SAIndex (input)
 *      SA index of the record to mark as allocated.
 */
void
SecYLib_WarmBoot_SA_Set(
        unsigned int DeviceId,
        unsigned int SAIndex)
{
    unsigned int ByteIndex = SAIndex / 8;
    unsigned int BitMask = 1 << (SAIndex % 8);

    SecY_Device[DeviceId]->WarmBoot_SA_Bitmap[ByteIndex] |= BitMask;

    WarmBootLib_Write(SecY_Device[DeviceId]->WarmBoot_AreaId,
                      SecY_Device[DeviceId]->WarmBoot_SA_Bitmap + ByteIndex,
                      SECY_WARMBOOT_BASE_BYTE_COUNT + ByteIndex,
                      1);
}


/*----------------------------------------------------------------------------
 * SecYLib_WarmBoot_SA_Clear
 *
 * Mark SA as free for the purpose of the WarmBoot administration..
 *
 * DeviceId (input)
 *      ID of the selected device.
 *
 * SAIndex (input)
 *      SA index of the record to mark as free.
 */
void
SecYLib_WarmBoot_SA_Clear(
        unsigned int DeviceId,
        unsigned int SAIndex)
{
    unsigned int ByteIndex = SAIndex / 8;
    unsigned int BitMask = 1 << (SAIndex % 8);

    SecY_Device[DeviceId]->WarmBoot_SA_Bitmap[ByteIndex] &= ~BitMask;

    WarmBootLib_Write(SecY_Device[DeviceId]->WarmBoot_AreaId,
                      SecY_Device[DeviceId]->WarmBoot_SA_Bitmap + ByteIndex,
                      SECY_WARMBOOT_BASE_BYTE_COUNT + ByteIndex,
                      1);
}


/*----------------------------------------------------------------------------
 * SecYLib_WarmBoot_SA_Get
 *
 * Determine if an SA is allocated.
 */
static inline bool
SecYLib_WarmBoot_SA_Get(
        unsigned int DeviceId,
        unsigned int SAIndex)
{
    unsigned int ByteIndex = SAIndex / 8;
    unsigned int BitMask = 1 << (SAIndex % 8);

    return (SecY_Device[DeviceId]->WarmBoot_SA_Bitmap[ByteIndex] & BitMask) != 0;
}


/*-----------------------------------------------------------------------------
 * SecYLib_WarmBoot_Shutdown
 */
int
SecYLib_WarmBoot_Shutdown(
        unsigned int DeviceId,
        unsigned int AreaId)
{
    List_Status_t List_Rc;
    IDENTIFIER_NOT_USED(AreaId);

    LOG_INFO("\n %s \n", __func__);

#ifdef ADAPTER_EIP164_STRICT_ARGS
    if (DeviceId >= ADAPTER_EIP164_MAX_NOF_DEVICES)
        return SECY_ERROR_INTERNAL;
#endif

    SecYLib_Device_Lock(DeviceId);

    if (!SecY_Device_StaticFields[DeviceId].fInitialized)
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

        /* Uninitialize Global AIC */
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

        /* Uninitialize Channel AIC's */
        for (i = 0; i < SecY_Device[DeviceId]->ChannelCount; i++)
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
    List_Rc = List_Uninit(0, SecY_Device[DeviceId]->SAFL_p);
    if (List_Rc != LIST_STATUS_OK)
    {
        LOG_CRIT("%s: Failed to uninit SA free list for "
                 "EIP-164 device for device %d\n",
                 __func__,
                 DeviceId);
        return SECY_ERROR_INTERNAL;
    }
    Adapter_Free(SecY_Device[DeviceId]->SAFL_p);
    SecY_Device[DeviceId]->SAFL_p = NULL;

    /* Uninitialize SC free list */
    List_Rc = List_Uninit(0, SecY_Device[DeviceId]->SCFL_p);
    if (List_Rc != LIST_STATUS_OK)
    {
        LOG_CRIT("%s: Failed to uninit SC free list for "
                 "EIP-164 device for device %d\n",
                 __func__,
                 DeviceId);
        return SECY_ERROR_INTERNAL;
    }
    Adapter_Free(SecY_Device[DeviceId]->SCFL_p);
    SecY_Device[DeviceId]->SCFL_p = NULL;

    /* Free SA descriptors */
    Adapter_Free(SecY_Device[DeviceId]->SADscr_p);
    SecY_Device[DeviceId]->SADscr_p = NULL;


    /* Free SC descriptors */
    Adapter_Free(SecY_Device[DeviceId]->SCDscr_p);
    SecY_Device[DeviceId]->SCDscr_p = NULL;

    Adapter_Free(SecY_Device[DeviceId]->vPortDscr_p);
    SecY_Device[DeviceId]->vPortDscr_p = NULL;

    Adapter_Free(SecY_Device[DeviceId]->vPortListHeads_p);
    SecY_Device[DeviceId]->vPortListHeads_p = NULL;

    SecY_Device_StaticFields[DeviceId].fInitialized = false;

    SecYLib_Device_Unlock(DeviceId);

    Adapter_Free(SecY_Device[DeviceId]->WarmBoot_SA_Bitmap);
    SecY_Device[DeviceId]->WarmBoot_SA_Bitmap = NULL;


    Adapter_Free(SecY_Device[DeviceId]);
    SecY_Device[DeviceId] = NULL;

    LOG_INFO("%s: device for device %d successfully uninitialized\n",
             __func__,
             DeviceId);

    return 0;
}


/*-----------------------------------------------------------------------------
 * SecYLib_WarmBoot_Restore
 */
int
SecYLib_WarmBoot_Restore(
        unsigned int DeviceId,
        unsigned int AreaId)
{
    EIP164_Error_t EIP164_Rc;
    SecY_Status_t SecY_Rc = 0;
    SecY_Role_t Role;
    EIP164_Device_Mode_t Mode;
    unsigned char DeviceInfo[9];
    unsigned int SACount,SCCount,vPortCount,ChannelCount;
    Device_Handle_t Device;
    unsigned int i;
    bool fSuccess;

    SecYLib_Device_Lock(DeviceId);

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

    SecY_Device[DeviceId] = Adapter_Alloc(sizeof(SecY_Device_t));
    if (SecY_Device[DeviceId] == NULL)
    {
        LOG_CRIT("%s: Failed to allocate data for SecY device %d (%s)\n",
                 __func__, DeviceId, ADAPTER_EIP164_DEVICE_NAME(DeviceId));
        SecYLib_Device_Unlock(DeviceId);
        return SECY_ERROR_INTERNAL;
    }

    if (!WarmBootLib_Read(AreaId, DeviceInfo, 0, 9))
    {
        Adapter_Free(SecY_Device[DeviceId]);
        SecYLib_Device_Unlock(DeviceId);
        return SECY_ERROR_INTERNAL;
    }

    SACount = (DeviceInfo[2]<<8) + DeviceInfo[3];
    SCCount = (DeviceInfo[4]<<8) + DeviceInfo[5];
    vPortCount = (DeviceInfo[6]<<8) + DeviceInfo[7];
    ChannelCount = DeviceInfo[8];

    Role = (SecY_Role_t)((DeviceInfo[0]>>2) & MASK_2_BITS);
    LOG_INFO("WarmBoot_Restore: SecY Device=%u Role=%d SA=%u SC=%u vPort=%u\n",
             DeviceId,Role,SACount,SCCount,vPortCount);

    SecY_Device[DeviceId]->WarmBoot_SA_Bitmap =
        Adapter_Alloc(SECY_WARMBOOT_SA_BYTE_COUNT(SACount));
    if (SecY_Device[DeviceId]->WarmBoot_SA_Bitmap == NULL)
    {
        Adapter_Free(SecY_Device[DeviceId]);
        SecYLib_Device_Unlock(DeviceId);
        return SECY_ERROR_INTERNAL;
    }


    SecY_Device[DeviceId]->SACount = SACount;
    SecY_Device[DeviceId]->SCCount = SCCount;
    SecY_Device[DeviceId]->vPortCount = vPortCount;
    SecY_Device[DeviceId]->ChannelCount = ChannelCount;
    SecY_Device[DeviceId]->Role = Role;
    SecY_Device[DeviceId]->WarmBoot_AreaId = AreaId;

    WarmBootLib_Read(AreaId,
                     SecY_Device[DeviceId]->WarmBoot_SA_Bitmap,
                     SECY_WARMBOOT_BASE_BYTE_COUNT,
                     SECY_WARMBOOT_SA_BYTE_COUNT(SACount));
    switch (Role)
    {
    case SECY_ROLE_EGRESS:
        Mode = EIP164_MODE_EGRESS;
        break;
    case SECY_ROLE_INGRESS:
        Mode = EIP164_MODE_INGRESS;
        break;
    case SECY_ROLE_EGRESS_INGRESS:
    default:
        /* Assume Egress-Ingress */
        Mode = EIP164_MODE_EGRESS_INGRESS;
        break;
    }

    /* Restore the EIP164 IO Area. */
    EIP164_Rc = EIP164_Device_Restore(&SecY_Device[DeviceId]->IOArea, Device,
                                      Mode);
    if (EIP164_Rc != EIP164_NO_ERROR)
    {
        LOG_CRIT("%s: Failed to restore EIP-164 device for device %d (%s), "
                 "mode %s, error %d\n",
                 __func__, DeviceId, ADAPTER_EIP164_DEVICE_NAME(DeviceId),
                 (Mode == EIP164_MODE_EGRESS) ? "egress" : "ingress",
                 EIP164_Rc);
        Adapter_Free(SecY_Device[DeviceId]->WarmBoot_SA_Bitmap);
        SecY_Device[DeviceId]->WarmBoot_SA_Bitmap = NULL;


        Adapter_Free(SecY_Device[DeviceId]);
        SecY_Device[DeviceId] = NULL;

        SecYLib_Device_Unlock(DeviceId);
        return EIP164_Rc;
    }

    /* Initialize the SA free list for this device */
    fSuccess = false;
    {
        List_Status_t List_Rc;

        /* Allocate SA free list */
        SecY_Device[DeviceId]->SAFL_p =
                Adapter_Alloc(List_GetInstanceByteCount());
        if (SecY_Device[DeviceId]->SAFL_p == NULL)
        {
            LOG_CRIT("%s: Failed to allocate SA free list "
                     "for EIP-164 device %d\n",
                     __func__,
                     DeviceId);
            Adapter_Free(SecY_Device[DeviceId]->WarmBoot_SA_Bitmap);
            SecY_Device[DeviceId]->WarmBoot_SA_Bitmap = NULL;
            Adapter_Free(SecY_Device[DeviceId]);
            SecY_Device[DeviceId] = NULL;
            SecYLib_Device_Unlock(DeviceId);
            return SECY_ERROR_INTERNAL;
        }

        List_Rc = List_Init(0, SecY_Device[DeviceId]->SAFL_p);
        if (List_Rc != LIST_STATUS_OK)
        {
            LOG_CRIT("%s: Failed to initialize SA free list for EIP-164 "
                     "device for device %d\n",
                     __func__,
                     DeviceId);
            goto lError;
        }

        /* Allocate SA descriptors for this device */
        SecY_Device[DeviceId]->SADscr_p =
                Adapter_Alloc(SecY_Device[DeviceId]->SACount *
                                           sizeof (SecY_SA_Descriptor_t));
        if (SecY_Device[DeviceId]->SADscr_p == NULL)
        {
            LOG_CRIT("%s: Failed to allocate SA descriptors for EIP-164 "
                     "device for device %d\n",
                     __func__,
                     DeviceId);
            goto lError;
        }

        for (i = 0; i < SecY_Device[DeviceId]->SACount; i++)
        {

            if (SecYLib_WarmBoot_SA_Get(DeviceId,i) == 0)
            {
                SecY_Device[DeviceId]->SADscr_p[i].Magic = 0;

                /* Add the SA descriptor as an element to the SA free list */
                SecY_Device[DeviceId]->SADscr_p[i].u.free.DataObject_p =
                    &SecY_Device[DeviceId]->SADscr_p[i];

                List_Rc = List_AddToHead(0,
                                     SecY_Device[DeviceId]->SAFL_p,
                                     &SecY_Device[DeviceId]->SADscr_p[i].u.free);
                if (List_Rc != LIST_STATUS_OK)
                {
                    LOG_CRIT("%s: Failed to create SA free list for EIP-164 "
                         "device for device %d\n",
                             __func__,
                         DeviceId);
                    goto lError;
                }
            }
            else
            {
                LOG_INFO("WarmBoot_Restore: SA %u in use\n",i);

                /* Create an in-use SA entry. */
                SecY_Device[DeviceId]->SADscr_p[i].u.InUse.SAIndex = i;
                SecY_Device[DeviceId]->SADscr_p[i].u.InUse.MapType = SECY_SA_MAP_DETACHED;
                SecY_Device[DeviceId]->SADscr_p[i].u.InUse.SAMFlowCtrlIndex = 0;
                SecY_Device[DeviceId]->SADscr_p[i].u.InUse.SCIndex = 0;
                SecY_Device[DeviceId]->SADscr_p[i].u.InUse.AN = 0;
                /* SA descriptor magic umber */
                SecY_Device[DeviceId]->SADscr_p[i].Magic = ADAPTER_EIP164_SA_DSCR_MAGIC;
            }
        } /* for (SA count) */
    } /* SA free list initialization done */

    /* Initialize the SC free list for this device */
    {
        List_Status_t List_Rc;

        /* Allocate SA free list */
        SecY_Device[DeviceId]->SCFL_p =
                Adapter_Alloc(List_GetInstanceByteCount());
        if (SecY_Device[DeviceId]->SCFL_p == NULL)
        {
            LOG_CRIT("%s: Failed to allocate SC free list "
                     "for EIP-164 device %d\n",
                     __func__,
                     DeviceId);
            goto lError;
        }

        List_Rc = List_Init(0, SecY_Device[DeviceId]->SCFL_p);
        if (List_Rc != LIST_STATUS_OK)
        {
            LOG_CRIT("%s: Failed to initialize SC free list for EIP-164 "
                     "device for device %d\n",
                     __func__,
                     DeviceId);
            goto lError;
        }

        /* Allocate SC descriptors for this device */
        SecY_Device[DeviceId]->SCDscr_p =
                Adapter_Alloc(SecY_Device[DeviceId]->SCCount *
                                           sizeof (SecY_SC_Descriptor_t));
        if (SecY_Device[DeviceId]->SCDscr_p == NULL)
        {
            LOG_CRIT("%s: Failed to allocate SC descriptors for EIP-164 "
                     "device for device %d\n",
                     __func__,
                     DeviceId);
            goto lError;
        }

        /* Allocate array of vPort descriptors. */
        SecY_Device[DeviceId]->vPortDscr_p =
            Adapter_Alloc(SecY_Device[DeviceId]->vPortCount *
                                sizeof(SecY_vPort_Descriptor_t));
        if (SecY_Device[DeviceId]->vPortDscr_p == NULL)
        {
            LOG_CRIT("%s: Failed to allocate vPort descriptor array for "
                     "EIP-164 device for device %d\n",
                     __func__,
                     DeviceId);
            goto lError;
        }

        SecY_Device[DeviceId]->vPortListHeads_p =
            Adapter_Alloc(SecY_Device[DeviceId]->vPortCount *
                                        List_GetInstanceByteCount());
        if (SecY_Device[DeviceId]->vPortListHeads_p == NULL)
        {
            LOG_CRIT("%s: Failed to allocate vPort SC list array for EIP-164 "
                     "device for device %d\n",
                     __func__,
                     DeviceId);
            goto lError;
        }

        for (i = 0; i < SecY_Device[DeviceId]->vPortCount; i++)
        {
            SecY_Device[DeviceId]->vPortDscr_p[i].SACount = 0;
            SecY_Device[DeviceId]->vPortDscr_p[i].MTU = 0;
            SecY_Device[DeviceId]->vPortDscr_p[i].SCList =
                SecY_Device[DeviceId]->vPortListHeads_p +
                                    i * List_GetInstanceByteCount();

            List_Rc = List_Init(0, SecY_Device[DeviceId]->vPortDscr_p[i].SCList);
            if (List_Rc != LIST_STATUS_OK)
            {
                LOG_CRIT("%s: Failed to initialize SC lookup list for EIP-164 "
                         "device for device %d\n",
                         __func__, DeviceId);
                goto lError;
            }
        }

        for (i = 0; i < SecY_Device[DeviceId]->SCCount; i++)
        {
            bool fSCIValid = false;
            /* Set MapType to Detached */
            SecY_Device[DeviceId]->SCDscr_p[i].MapType = SECY_SA_MAP_DETACHED;

#ifdef ADAPTER_EIP164_DBG
            /* SC descriptor magic number */
            SecY_Device[DeviceId]->SCDscr_p[i].Magic =
                                            ADAPTER_EIP164_SA_DSCR_MAGIC;
#endif /* ADAPTER_EIP164_DBG */

            SecY_Device[DeviceId]->SCDscr_p[i].SCIndex = i;

#ifdef ADAPTER_EIP164_MODE_INGRESS
            if (Role == SECY_ROLE_INGRESS)
            {
                unsigned int j;
                unsigned int SAIndex[4];
                bool fSAInUse[4];
                uint32_t SCI_Lo;
                uint32_t SCI_Hi;
                unsigned int vPort;
                bool fInCAM;
                SecY_SA_Descriptor_t * SADscr_p;
                SecY_SC_Descriptor_t * SCDscr_p = &SecY_Device[DeviceId]->SCDscr_p[i];

                SCDscr_p->SAHandle[0] = SecY_SAHandle_NULL;
                SCDscr_p->SAHandle[1] = SecY_SAHandle_NULL;
                SCDscr_p->SAHandle[2] = SecY_SAHandle_NULL;
                SCDscr_p->SAHandle[3] = SecY_SAHandle_NULL;

                {
                    unsigned int LoopCounter = ADAPTER_EIP164_MAX_NOF_SYNC_RETRY_COUNT;

                    for(;;)
                    {
                        LoopCounter--;
                        EIP164_Rc = EIP164_SecY_RxCAM_Read(
                            &SecY_Device[DeviceId]->IOArea,
                            i,
                            &SCI_Lo,
                            &SCI_Hi,
                            &vPort,
                            &fInCAM);
                        if (EIP164_Rc == EIP164_NO_ERROR)
                        {
                            break;
                        }
                        else if (EIP164_Rc != EIP164_BUSY_RETRY_LATER ||
                                 LoopCounter == 0)
                        {
                            goto lError;
                        }
                    }
                }

                SCDscr_p->vPort = vPort;

                if (fInCAM)
                {
                    EIP164_Rc = EIP164_SecY_SA_Active_I_Get(&SecY_Device[DeviceId]->IOArea,
                                                i,
                                                SAIndex,
                                                fSAInUse);
                    if (EIP164_Rc != EIP164_NO_ERROR)
                    {
                        goto lError;
                    }
                    if (fSAInUse[0] && fSAInUse[1] && fSAInUse[2] && fSAInUse[3] &&
                        SAIndex[0] == SAIndex[1] &&
                        SAIndex[0] == SAIndex[2] &&
                        SAIndex[0] == SAIndex[3] &&
                        SecYLib_WarmBoot_SA_Get(DeviceId,SAIndex[0]))
                    {
                        /* Crypt-auth */
                        LOG_INFO("WarmBoot_Restore: Found crypt-auth SA=%u for SC=%u\n",SAIndex[0],i);
                        SADscr_p = &(SecY_Device[DeviceId]->SADscr_p[SAIndex[0]]);
                        fSCIValid = true;
                        SCDscr_p->MapType = SECY_SA_MAP_INGRESS_CRYPT_AUTH;
                        SecYLib_SAIndexToHandle(DeviceId,
                                                SAIndex[0],
                                                &SCDscr_p->SAHandle[0]);
                        SADscr_p->u.InUse.MapType = SECY_SA_MAP_INGRESS_CRYPT_AUTH;
                        SADscr_p->u.InUse.SCIndex = i;
                        SADscr_p->u.InUse.SAMFlowCtrlIndex = vPort;
                        SADscr_p->u.InUse.AN = 0;
                        SecY_Device[DeviceId]->vPortDscr_p[vPort].SACount++;
                    }
                    else
                    {
                        SCDscr_p->MapType = SECY_SA_MAP_INGRESS;

                        /* Ingress */
                        for (j=0; j<4; j++)
                        {
                            if(fSAInUse[j] && SecYLib_WarmBoot_SA_Get(DeviceId, SAIndex[j]))
                            {
                                LOG_INFO("WarmBoot_Restore Found ingress SA=%u for SC=%u AN=%u\n",
                                         SAIndex[j],i,j);
                                SADscr_p = &(SecY_Device[DeviceId]->SADscr_p[SAIndex[j]]);
                                fSCIValid = true;

                                SADscr_p->u.InUse.MapType = SECY_SA_MAP_INGRESS;
                                SADscr_p->u.InUse.SCIndex = i;
                                SADscr_p->u.InUse.SAMFlowCtrlIndex = vPort;
                                SADscr_p->u.InUse.AN = j;
                                SecYLib_SAIndexToHandle(DeviceId,
                                                SAIndex[j],
                                                &SCDscr_p->SAHandle[j]);
                                SecY_Device[DeviceId]->vPortDscr_p[vPort].SACount++;

                            }
                        }
                    }
                }
                if (fSCIValid)
                {
                    /* Add SCI entry to vPort list. */
                    List_Element_t * Element_p;

                    LOG_INFO("WarmBoot_Restore: Ingress SCI=%u vPort=%u\n",
                             i,vPort);
                    SCDscr_p->SCI[0] =  SCI_Lo        & 0xff;
                    SCDscr_p->SCI[1] = (SCI_Lo >>  8) & 0xff;
                    SCDscr_p->SCI[2] = (SCI_Lo >> 16) & 0xff;
                    SCDscr_p->SCI[3] = (SCI_Lo >> 24) & 0xff;
                    SCDscr_p->SCI[4] =  SCI_Hi        & 0xff;
                    SCDscr_p->SCI[5] = (SCI_Hi >>  8) & 0xff;
                    SCDscr_p->SCI[6] = (SCI_Hi >> 16) & 0xff;
                    SCDscr_p->SCI[7] = (SCI_Hi >> 24) & 0xff;
                    /* Add the SC descriptor as an element to the SC free list */
                    SCDscr_p->OnList.DataObject_p = SCDscr_p;
                    Element_p = &SCDscr_p->OnList;
                    List_Rc = List_AddToHead(0,
                              SecY_Device[DeviceId]->vPortDscr_p[vPort].SCList,
                              Element_p);
                    if (List_Rc != LIST_STATUS_OK)
                    {
                        LOG_CRIT("%s: Failed to add SC elemet to lookup list for "
                                 "EIP-164 device for device %d, error %d\n",
                                 __func__,
                                 DeviceId,
                                 List_Rc);
                        goto lError;
                    }
                }
            }
#endif
#ifdef ADAPTER_EIP164_MODE_EGRESS
            if (Role != SECY_ROLE_INGRESS)
            {
                /* Egress role. */
                unsigned int SAIndex, NewIndex;
                bool fSAInUse;
                unsigned int vPort = i;
                SecY_SC_Descriptor_t * SCDscr_p = &SecY_Device[DeviceId]->SCDscr_p[i];

                EIP164_Rc = EIP164_SecY_SA_Installation_Verify(&SecY_Device[DeviceId]->IOArea,
                                                   vPort,
                                                   &SAIndex,
                                                   &fSAInUse);
                if (EIP164_Rc != EIP164_NO_ERROR)
                    goto lError;
                SCDscr_p->SAHandle[0] = SecY_SAHandle_NULL;
                SCDscr_p->SAHandle[1] = SecY_SAHandle_NULL;
                SCDscr_p->SAHandle[2] = SecY_SAHandle_NULL;
                SCDscr_p->SAHandle[3] = SecY_SAHandle_NULL;

                if (fSAInUse && SecYLib_WarmBoot_SA_Get(DeviceId, SAIndex))
                {
                    /* Valid egress SA found. */
                    SecY_SA_Descriptor_t * SADscr_p = &SecY_Device[DeviceId]->SADscr_p[SAIndex];
                    EIP164_SecY_SA_t SAParams;
                    LOG_INFO("WarmBoot_Restore: Found egress SA=%u vPort=%u\n",
                             SAIndex, vPort);
                    SecYLib_SAIndexToHandle(DeviceId,
                                            SAIndex,
                                            &SCDscr_p->SAHandle[0]);
                    SADscr_p->u.InUse.SCIndex = i;
                    SADscr_p->u.InUse.SAMFlowCtrlIndex = vPort;
                    SADscr_p->u.InUse.AN = 0;
                    SecY_Device[DeviceId]->vPortDscr_p[vPort].SACount++;

                    EIP164_Rc = EIP164_SecY_SAMFlow_Read(&SecY_Device[DeviceId]->IOArea,
                                       vPort,
                                       &SAParams
                                      );

                    if (EIP164_Rc != EIP164_NO_ERROR)
                        goto lError;

                    if(SAParams.ActionType == EIP164_SECY_SA_ACTION_EGRESS)
                    {
                        /* Read MTU from egress SA record and put it back in */
                        /* vPort descriptor. */
                        unsigned int Offset;
                        SADscr_p->u.InUse.MapType = SECY_SA_MAP_EGRESS;
                        SCDscr_p->MapType = SECY_SA_MAP_EGRESS;

                        SABuilder_MTUOffset_Get(0, &Offset);
                        SecY_Rc = SecYLib_SA_Read(DeviceId,
                                        SAIndex,
                                        Offset,
                                        1,
                                        &SecY_Device[DeviceId]->vPortDscr_p[vPort].MTU);
                        if(SecY_Rc != SECY_STATUS_OK)
                            goto lError;

                        /* Check for a pending chained SA Read SA */
                        /* update control word. If valid next SA index */
                        /* present, mark the current SA as DETACHED, the */
                        /* chained-to SA as EGRESS. */
                        fSAInUse = false;
                        SecY_Rc = SecYLib_SA_Chained_Get(DeviceId,
                                               SAIndex,
                                               &NewIndex,
                                               &fSAInUse);
                        if(SecY_Rc != SECY_STATUS_OK)
                            goto lError;
                        while (fSAInUse && SecYLib_WarmBoot_SA_Get(DeviceId, NewIndex))
                        {
                            SADscr_p->u.InUse.MapType = SECY_SA_MAP_DETACHED;
                            SAIndex = NewIndex;
                            SADscr_p = &SecY_Device[DeviceId]->SADscr_p[SAIndex];
                            SecYLib_SAIndexToHandle(DeviceId,
                                            SAIndex,
                                            &SCDscr_p->SAHandle[0]);
                            SADscr_p->u.InUse.MapType = SECY_SA_MAP_EGRESS;
                            SADscr_p->u.InUse.SCIndex = i;
                            SADscr_p->u.InUse.SAMFlowCtrlIndex = vPort;
                            SADscr_p->u.InUse.AN = 0;
                            SecY_Rc = SecYLib_SA_Chained_Get(DeviceId,
                                                   SAIndex,
                                                   &NewIndex,
                                                   &fSAInUse);
                            if(SecY_Rc != SECY_STATUS_OK)
                                goto lError;

                        }
                    }
                    else
                    {

                        SADscr_p->u.InUse.MapType = SECY_SA_MAP_EGRESS_CRYPT_AUTH;
                        SCDscr_p->MapType = SECY_SA_MAP_EGRESS_CRYPT_AUTH;
                    }
                }

            }
#endif
            if (!fSCIValid)
            {
                /* Add the SC descriptor as an element to the SC free list */
                SecY_Device[DeviceId]->SCDscr_p[i].OnList.DataObject_p =
                    &SecY_Device[DeviceId]->SCDscr_p[i];

                List_Rc = List_AddToHead(0,
                                     SecY_Device[DeviceId]->SCFL_p,
                                         &SecY_Device[DeviceId]->SCDscr_p[i].OnList);
                if (List_Rc != LIST_STATUS_OK)
                {
                    LOG_CRIT("%s: Failed to add descriptor to SC lookup list "
                         "for EIP-164 device for device %d\n",
                             __func__,
                             DeviceId);
                    goto lError;
                }
            }
        } /* for (vPorts count) */
    } /* SC free list initialization done */

#ifdef ADAPTER_EIP164_INTERRUPTS_ENABLE
    {
        int rc;
        /* Initialize Global AIC */
        rc = Adapter_Interrupts_Init(SecYLib_ICDeviceID_Get(DeviceId, NULL));
        if (rc)
            goto irq_failed;

        rc = Adapter_Interrupt_SetHandler(SecYLib_ICDeviceID_Get(DeviceId, NULL),
                                          SecYLib_InterruptHandler);
        if (rc)
            goto irq_failed;
        /* Initialize Packet Engine AIC. */
        rc = Adapter_Interrupts_Init(SecYLib_ICDeviceID_Get(DeviceId, NULL) + 1);
        if (rc)
            goto irq_failed;
        rc = Adapter_Interrupt_SetHandler(SecYLib_ICDeviceID_Get(DeviceId, NULL) + 1,
                                          SecYLib_InterruptHandler);
        if (rc)
            goto irq_failed;

        /* Initialize Channel AIC's */
        for (i = 0; i < SecY_Device[DeviceId]->ChannelCount; i++)
        {
            rc = Adapter_Interrupts_Init(SecYLib_ICDeviceID_Get(DeviceId, &i));

            if (rc)
                goto irq_failed;

            rc = Adapter_Interrupt_SetHandler(SecYLib_ICDeviceID_Get(DeviceId, &i),
                                              SecYLib_InterruptHandler);
            if (rc)
                goto irq_failed;
        }

irq_failed:
        if (rc)
        {
            /* Uninitialize Global AIC */
            Adapter_Interrupts_UnInit(SecYLib_ICDeviceID_Get(DeviceId, NULL));
            Adapter_Interrupts_UnInit(SecYLib_ICDeviceID_Get(DeviceId, NULL) + 1);

            /* Uninitialize Channel AIC's */
            for (i = 0; i < SecY_Device[DeviceId]->ChannelCount; i++)
            {
                Adapter_Interrupts_UnInit(SecYLib_ICDeviceID_Get(DeviceId, &i));
            }
            SecY_Rc = SECY_DLIB_ERROR(rc);
            goto lError;
        }
    }
#endif /* ADAPTER_EIP164_INTERRUPTS_ENABLE */

    fSuccess = true;

lError:
    if (!fSuccess)
    {
        /* Free vPort descriptors. */
        Adapter_Free(SecY_Device[DeviceId]->vPortDscr_p);
        SecY_Device[DeviceId]->vPortDscr_p = NULL;

        /* Free vPort List heads */
        Adapter_Free(SecY_Device[DeviceId]->vPortListHeads_p);
        SecY_Device[DeviceId]->vPortListHeads_p = NULL;

        /* Free SC descriptors */
        Adapter_Free(SecY_Device[DeviceId]->SCDscr_p);
        SecY_Device[DeviceId]->SCDscr_p = NULL;

        if (SecY_Device[DeviceId]->SCFL_p)
        {
            List_Uninit(0, SecY_Device[DeviceId]->SCFL_p);
            Adapter_Free(SecY_Device[DeviceId]->SCFL_p);
            SecY_Device[DeviceId]->SCFL_p = NULL;
        }

        /* Free SA descriptors */
        Adapter_Free(SecY_Device[DeviceId]->SADscr_p);
        SecY_Device[DeviceId]->SADscr_p = NULL;

        if (SecY_Device[DeviceId]->SAFL_p)
        {
            List_Uninit(0, SecY_Device[DeviceId]->SAFL_p);
            Adapter_Free(SecY_Device[DeviceId]->SAFL_p);
            SecY_Device[DeviceId]->SAFL_p = NULL;
        }

        Adapter_Free(SecY_Device[DeviceId]->WarmBoot_SA_Bitmap);
        SecY_Device[DeviceId]->WarmBoot_SA_Bitmap = NULL;
        Adapter_Free(SecY_Device[DeviceId]);
        SecY_Device[DeviceId] = NULL;
        SecYLib_Device_Unlock(DeviceId);

        if (EIP164_Rc != EIP164_NO_ERROR)
            return EIP164_Rc;
        else if (SecY_Rc != SECY_STATUS_OK)
            return SecY_Rc;
        else
            return SECY_ERROR_INTERNAL;
    }

    SecY_Device_StaticFields[DeviceId].fInitialized = true;

    SecYLib_Device_Unlock(DeviceId);
    LOG_INFO("%s: device for device %d successfully restored\n",
             __func__, DeviceId);

    return 0;
}
#endif


/* end of file adapter_secy_warmboot.c */
