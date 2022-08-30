/* adapter_cfye_support.c
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

#include <Integration/Adapter_EIP163/incl/api_cfye.h>               /* CfyE API */
#include <Integration/Adapter_EIP163/incl/adapter_cfye_support.h>

/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

/* Logging API */
#include <Kit/Log/incl/log.h>                    /* LOG_* */


/* Adapter Alloc API */
#include <Integration/Adapter_EIP164/incl/adapter_alloc.h>      /* Adapter_Alloc()/_Free() */

/* Adapter Sleep API */
#include <Integration/Adapter_EIP164/incl/adapter_sleep.h>      /* Adapter_SleepMS() */

/* Adapter Lock API */
#include <Integration/Adapter_EIP164/incl/adapter_lock.h>       /* Adapter_Lock*, ADAPTER_LOCK_* */

#ifdef ADAPTER_EIP163_INTERRUPTS_ENABLE
/* Adapter Interrupts API */
#include <Integration/Adapter_EIP164/incl/adapter_interrupts.h>
#endif


/*----------------------------------------------------------------------------
 * Definitions and macros
 */

/* Adapter EIP-163 Global AIC events resulting in interrupts */
#define ADAPTER_EIP163_EVENT_STAT_TCAM_THR            BIT_0
#define ADAPTER_EIP163_EVENT_STAT_CHAN_THR            BIT_1
#define ADAPTER_EIP163_EVENT_ECC_ERR                  BIT_8

/* Adapter EIP-163 Channel AIC events resulting in interrupts */
#define ADAPTER_EIP163_EVENT_CHAN_ERR_SOP_WO_EOP      BIT_0
#define ADAPTER_EIP163_EVENT_CHAN_ERR_EOP_WO_SOP      BIT_1
#define ADAPTER_EIP163_EVENT_CHAN_ERR_XFER_WO_SOP     BIT_2
#define ADAPTER_EIP163_EVENT_CHAN_ERR_SLOT_SOP        BIT_3
#define ADAPTER_EIP163_EVENT_CHAN_ERR_SLOT_CHID       BIT_4
#define ADAPTER_EIP163_EVENT_CHAN_ERR_NOT_B2B         BIT_5
#define ADAPTER_EIP163_EVENT_CHAN_CTRL_PACKET         BIT_6
#define ADAPTER_EIP163_EVENT_CHAN_DATA_PACKET         BIT_7
#define ADAPTER_EIP163_EVENT_CHAN_DROP_PACKET         BIT_8
#define ADAPTER_EIP163_EVENT_CHAN_TCAM_HIT_MULT       BIT_9
#define ADAPTER_EIP163_EVENT_CHAN_TCAM_MISS           BIT_10
#define ADAPTER_EIP163_EVENT_CHAN_ERR_SLOT_2PKTS      BIT_11

#if (EIP163_MAC_DA_ET_MATCH_RULES_COUNT != CFYE_MAC_DA_ET_MATCH_RULES_COUNT) || \
    (EIP163_MAC_DA_ET_RANGE_MATCH_RULES_COUNT != CFYE_MAC_DA_ET_RANGE_MATCH_RULES_COUNT) || \
    (EIP163_VLAN_UP_MAX_COUNT != CFYE_VLAN_UP_MAX_COUNT) || \
    (EIP163_RULE_NON_CTRL_WORD_COUNT != CFYE_RULE_NON_CTRL_WORD_COUNT)
#error "CfyE API and EIP-163 Driver Library API mismatch"
#endif

#define ADAPTER_NOF_IC_PER_EIP163_DEVICE \
    (ADAPTER_EIP163_MAX_IC_COUNT/ADAPTER_EIP163_MAX_NOF_DEVICES)


#define CFYE_TEST_EQUAL(a,b) \
    extern int CfyE_##a##must_be_equal[1-2*((unsigned int)(a)!=(unsigned int)(b))]

/* As we are assigning between enum type values defined at the
 * Adapter level and enum type values defined at the Driver Library
 * level, enum values of the corresponding types must be
 * equal. Perform compile-time checks here.
 */
CFYE_TEST_EQUAL(CFYE_ROLE_EGRESS, EIP163_ROLE_EGRESS);
CFYE_TEST_EQUAL(CFYE_ROLE_INGRESS, EIP163_ROLE_INGRESS);

CFYE_TEST_EQUAL(CFYE_DROP_CRC_ERROR, EIP163_DROP_CRC_ERROR);
CFYE_TEST_EQUAL(CFYE_DROP_PKT_ERROR, EIP163_DROP_PKT_ERROR);
CFYE_TEST_EQUAL(CFYE_DROP_INTERNAL, EIP163_DROP_INTERNAL);
CFYE_TEST_EQUAL(CFYE_DO_NOT_DROP, EIP163_DO_NOT_DROP);

CFYE_TEST_EQUAL(CFYE_RULE_PKT_TYPE_OTHER, EIP163_RULE_PKT_TYPE_OTHER);
CFYE_TEST_EQUAL(CFYE_RULE_PKT_TYPE_MPLS, EIP163_RULE_PKT_TYPE_MPLS);
CFYE_TEST_EQUAL(CFYE_RULE_PKT_TYPE_PBB, EIP163_RULE_PKT_TYPE_PBB);
CFYE_TEST_EQUAL(CFYE_RULE_PKT_TYPE_MACSEC, EIP163_RULE_PKT_TYPE_MACSEC);

CFYE_TEST_EQUAL(CFYE_ECC_NOF_STATUS_COUNTERS, EIP163_ECC_NOF_STATUS_COUNTERS);


/*----------------------------------------------------------------------------
 * Local variables
 */

#ifdef ADAPTER_EIP163_INTERRUPTS_ENABLE
CfyE_Notify_Internal_t * CfyE_Notify[ADAPTER_EIP163_MAX_IC_COUNT];
#endif








/*----------------------------------------------------------------------------
 * CfyELib_vPortHandle_IsSame
 *
 * Check if two vPort handles are equal.
 *
 * Handle1 (input)
 *     First handle.
 *
 * Handle2 (input)
 *     Second handle.
 *
 * Return: true if both handles are equal, false otherwise.
 */
bool CfyELib_vPortHandle_IsSame(
        const CfyE_vPortHandle_t Handle1,
        const CfyE_vPortHandle_t Handle2)
{
    return (Handle1 == Handle2);
}

/*----------------------------------------------------------------------------
 * CfyELib_RuleHandle_IsSame
 *
 * Check if two Rule handles are equal.
 *
 * Handle1 (input)
 *     First handle.
 *
 * Handle2 (input)
 *     Second handle.
 *
 * Return: true if both handles are equal, false otherwise.
 */
bool CfyELib_RuleHandle_IsSame(
        const CfyE_RuleHandle_t Handle1,
        const CfyE_RuleHandle_t Handle2)
{
    return (Handle1 == Handle2);
}

/*----------------------------------------------------------------------------
 * CfyELib_Initialized_Check
 */
bool
CfyELib_Initialized_Check(
        const unsigned int DeviceId,
        const char * FuncName,
        const bool fExpectedReturn)
{
    if (PRV_SHARED_MACSEC_CFYE_GLOBAL_VAR_GET(CfyE_Device_StaticFields)[DeviceId].fInitialized)
    {
        IDENTIFIER_NOT_USED_LOG_OFF(FuncName);

        if (!fExpectedReturn)
        {
            LOG_CRIT("%s: failed, EIP-163 device %d already initialized\n",
                     FuncName,
                     DeviceId);
        }
        return true;
    }
    else
    {
        if (fExpectedReturn)
        {
            LOG_CRIT("%s: failed, EIP-163 device %d not initialized yet\n",
                     FuncName,
                     DeviceId);
        }

        return false;
    }
}




/*----------------------------------------------------------------------------
 * CfyELib_Device_Sync
 */
CfyE_Status_t
CfyELib_Device_Sync(
        const unsigned int DeviceId)
{
    EIP163_Error_t Rc;

    LOG_INFO("\n\t EIP163_Device_Sync \n");

    /* Sync with the device */
    Rc = EIP163_Device_Sync(&PRV_SHARED_MACSEC_CFYE_GLOBAL_VAR_GET(CfyE_Devices)[DeviceId]->IOArea);
    if (Rc == EIP163_BUSY_RETRY_LATER)
    {
        /* Wait till the device synchronization is done */
        unsigned int LoopCounter = ADAPTER_EIP163_MAX_NOF_SYNC_RETRY_COUNT;

        while (LoopCounter)
        {
            LOG_INFO("\n\t EIP163_Device_Is_Sync_Done \n");

            Rc = EIP163_Device_Is_Sync_Done(&PRV_SHARED_MACSEC_CFYE_GLOBAL_VAR_GET(CfyE_Devices)[DeviceId]->IOArea);
            if (Rc != EIP163_BUSY_RETRY_LATER)
            {
                break;
            }

            Adapter_SleepMS(ADAPTER_EIP163_SYNC_RETRY_TIMEOUT_MS);
            LoopCounter--;
        }
    }
    if (Rc == EIP163_NO_ERROR)
    {
        return CFYE_STATUS_OK;
    }
    else
    {
        return CFYE_DLIB_ERROR(Rc);
    }
}


/*----------------------------------------------------------------------------
 * CfyELib_vPortFreeList_Uninit
 */
void
CfyELib_vPortFreeList_Uninit(
        const unsigned int DeviceId)
{
    /* Free vPort descriptors */
    Adapter_Free(PRV_SHARED_MACSEC_CFYE_GLOBAL_VAR_GET(CfyE_Devices)[DeviceId]->vPortDscr_p);
    PRV_SHARED_MACSEC_CFYE_GLOBAL_VAR_SET(CfyE_Devices[DeviceId]->vPortDscr_p,NULL);

    List_Uninit(0, PRV_SHARED_MACSEC_CFYE_GLOBAL_VAR_GET(CfyE_Devices)[DeviceId]->vPortFreeList_p);

    /* Free vPort free list */
    Adapter_Free(PRV_SHARED_MACSEC_CFYE_GLOBAL_VAR_GET(CfyE_Devices)[DeviceId]->vPortFreeList_p);
    PRV_SHARED_MACSEC_CFYE_GLOBAL_VAR_SET(CfyE_Devices[DeviceId]->vPortFreeList_p,NULL);
}


/*----------------------------------------------------------------------------
 * CfyELib_RuleFreeList_Uninit
 */
void
CfyELib_RuleFreeList_Uninit(
        const unsigned int DeviceId)
{
    /* Free Rule descriptors */
    Adapter_Free(PRV_SHARED_MACSEC_CFYE_GLOBAL_VAR_GET(CfyE_Devices)[DeviceId]->RuleDscr_p);
    PRV_SHARED_MACSEC_CFYE_GLOBAL_VAR_SET(CfyE_Devices[DeviceId]->RuleDscr_p,NULL);

    List_Uninit(0, PRV_SHARED_MACSEC_CFYE_GLOBAL_VAR_GET(CfyE_Devices)[DeviceId]->RuleFreeList_p);

    /* Free Rule free list */
    Adapter_Free(PRV_SHARED_MACSEC_CFYE_GLOBAL_VAR_GET(CfyE_Devices)[DeviceId]->RuleFreeList_p);
    PRV_SHARED_MACSEC_CFYE_GLOBAL_VAR_SET(CfyE_Devices[DeviceId]->RuleFreeList_p,NULL);
}


/*----------------------------------------------------------------------------
 * CfyELib_Device_Lock
 */
void
CfyELib_Device_Lock(
        const unsigned int DeviceId)
{
    Adapter_Lock_Acquire(&PRV_SHARED_MACSEC_CFYE_GLOBAL_VAR_GET(CfyE_Device_StaticFields)[DeviceId].Lock,
                         &PRV_SHARED_MACSEC_CFYE_GLOBAL_VAR_GET(CfyE_Device_StaticFields)[DeviceId].Flags);
}


/*----------------------------------------------------------------------------
 * CfyELib_Device_Unlock
 */
void
CfyELib_Device_Unlock(
        const unsigned int DeviceId)
{
    Adapter_Lock_Release(&PRV_SHARED_MACSEC_CFYE_GLOBAL_VAR_GET(CfyE_Device_StaticFields)[DeviceId].Lock,
                         &PRV_SHARED_MACSEC_CFYE_GLOBAL_VAR_GET(CfyE_Device_StaticFields)[DeviceId].Flags);
}


#ifdef ADAPTER_EIP163_INTERRUPTS_ENABLE
/*----------------------------------------------------------------------------
 * CfyELib_ICDeviceID_Get
 */
unsigned int
CfyELib_ICDeviceID_Get(
        const unsigned int DeviceId,
        const unsigned int * const ChannelId_p)
{
    bool fGlobal, fPE = false;
    unsigned int ChID = 0;

    if (ChannelId_p)
    {
        fGlobal = false;
        ChID = *ChannelId_p;
    }
    else
        fGlobal = true;

    return ADAPTER_EIP163_ICDEV_ID(ADAPTER_EIP163_DEV_ID(DeviceId),
                                   fGlobal,
                                   fPE,
                                   ChID);
}


/*----------------------------------------------------------------------------
 * CfyELib_CfyEEvents_Get
 */
static inline unsigned int
CfyELib_CfyEEvents_Get(
        const unsigned int AdapterEvents,
        const bool fGlobalAIC)
{
    unsigned int CfyEEvents = 0;

    if (fGlobalAIC)
    {
        if (AdapterEvents & ADAPTER_EIP163_EVENT_STAT_TCAM_THR)
            CfyEEvents |= CFYE_EVENT_STAT_TCAM_THR;

        if (AdapterEvents & ADAPTER_EIP163_EVENT_STAT_CHAN_THR)
            CfyEEvents |= CFYE_EVENT_STAT_CHAN_THR;

        if (AdapterEvents & ADAPTER_EIP163_EVENT_ECC_ERR)
            CfyEEvents |= CFYE_EVENT_ECC_ERR;
    }
    else
    {
        if (AdapterEvents & ADAPTER_EIP163_EVENT_CHAN_ERR_SOP_WO_EOP)
            CfyEEvents |= CFYE_EVENT_CHAN_ERR_SOP_WO_EOP;

        if (AdapterEvents & ADAPTER_EIP163_EVENT_CHAN_ERR_EOP_WO_SOP)
            CfyEEvents |= CFYE_EVENT_CHAN_ERR_EOP_WO_SOP;

        if (AdapterEvents & ADAPTER_EIP163_EVENT_CHAN_ERR_XFER_WO_SOP)
            CfyEEvents |= CFYE_EVENT_CHAN_ERR_XFER_WO_SOP;

        if (AdapterEvents & ADAPTER_EIP163_EVENT_CHAN_ERR_SLOT_SOP)
            CfyEEvents |= CFYE_EVENT_CHAN_ERR_SLOT_SOP;

        if (AdapterEvents & ADAPTER_EIP163_EVENT_CHAN_ERR_SLOT_CHID)
            CfyEEvents |= CFYE_EVENT_CHAN_ERR_SLOT_CHID;

        if (AdapterEvents & ADAPTER_EIP163_EVENT_CHAN_ERR_NOT_B2B)
            CfyEEvents |= CFYE_EVENT_CHAN_ERR_NOT_B2B;

        if (AdapterEvents & ADAPTER_EIP163_EVENT_CHAN_CTRL_PACKET)
            CfyEEvents |= CFYE_EVENT_CHAN_CTRL_PACKET;

        if (AdapterEvents & ADAPTER_EIP163_EVENT_CHAN_DATA_PACKET)
            CfyEEvents |= CFYE_EVENT_CHAN_DATA_PACKET;

        if (AdapterEvents & ADAPTER_EIP163_EVENT_CHAN_DROP_PACKET)
            CfyEEvents |= CFYE_EVENT_CHAN_DROP_PACKET;

        if (AdapterEvents & ADAPTER_EIP163_EVENT_CHAN_TCAM_HIT_MULT)
            CfyEEvents |= CFYE_EVENT_CHAN_TCAM_HIT_MULT;

        if (AdapterEvents & ADAPTER_EIP163_EVENT_CHAN_TCAM_MISS)
            CfyEEvents |= CFYE_EVENT_CHAN_TCAM_MISS;

        if (AdapterEvents & ADAPTER_EIP163_EVENT_CHAN_ERR_SLOT_2PKTS)
            CfyEEvents |= CFYE_EVENT_CHAN_ERR_SLOT_2PKTS;
    }

    return CfyEEvents;
}


/*----------------------------------------------------------------------------
 * CfyELib_AdapterEvents_Get
 */
static inline unsigned int
CfyELib_AdapterEvents_Get(
        const unsigned int CfyEEvents,
        const bool fGlobalAIC)
{
    unsigned int AdapterEvents = 0;

    if (fGlobalAIC)
    {
        if (CfyEEvents & CFYE_EVENT_STAT_TCAM_THR)
            AdapterEvents |= ADAPTER_EIP163_EVENT_STAT_TCAM_THR;

        if (CfyEEvents & CFYE_EVENT_STAT_CHAN_THR)
            AdapterEvents |= ADAPTER_EIP163_EVENT_STAT_CHAN_THR;

        if (CfyEEvents & CFYE_EVENT_ECC_ERR)
            AdapterEvents |= ADAPTER_EIP163_EVENT_ECC_ERR;
    }
    else
    {
        if (CfyEEvents & CFYE_EVENT_CHAN_ERR_SOP_WO_EOP)
            AdapterEvents |= ADAPTER_EIP163_EVENT_CHAN_ERR_SOP_WO_EOP;

        if (CfyEEvents & CFYE_EVENT_CHAN_ERR_EOP_WO_SOP)
            AdapterEvents |= ADAPTER_EIP163_EVENT_CHAN_ERR_EOP_WO_SOP;

        if (CfyEEvents & CFYE_EVENT_CHAN_ERR_XFER_WO_SOP)
            AdapterEvents |= ADAPTER_EIP163_EVENT_CHAN_ERR_XFER_WO_SOP;

        if (CfyEEvents & CFYE_EVENT_CHAN_ERR_SLOT_SOP)
            AdapterEvents |= ADAPTER_EIP163_EVENT_CHAN_ERR_SLOT_SOP;

        if (CfyEEvents & CFYE_EVENT_CHAN_ERR_SLOT_CHID)
            AdapterEvents |= ADAPTER_EIP163_EVENT_CHAN_ERR_SLOT_CHID;

        if (CfyEEvents & CFYE_EVENT_CHAN_ERR_NOT_B2B)
            AdapterEvents |= ADAPTER_EIP163_EVENT_CHAN_ERR_NOT_B2B;

        if (CfyEEvents & CFYE_EVENT_CHAN_CTRL_PACKET)
            AdapterEvents |= ADAPTER_EIP163_EVENT_CHAN_CTRL_PACKET;

        if (CfyEEvents & CFYE_EVENT_CHAN_DATA_PACKET)
            AdapterEvents |= ADAPTER_EIP163_EVENT_CHAN_DATA_PACKET;

        if (CfyEEvents & CFYE_EVENT_CHAN_DROP_PACKET)
            AdapterEvents |= ADAPTER_EIP163_EVENT_CHAN_DROP_PACKET;

        if (CfyEEvents & CFYE_EVENT_CHAN_TCAM_HIT_MULT)
            AdapterEvents |= ADAPTER_EIP163_EVENT_CHAN_TCAM_HIT_MULT;

        if (CfyEEvents & CFYE_EVENT_CHAN_TCAM_MISS)
            AdapterEvents |= ADAPTER_EIP163_EVENT_CHAN_TCAM_MISS;

        if (CfyEEvents & CFYE_EVENT_CHAN_ERR_SLOT_2PKTS)
            AdapterEvents |= ADAPTER_EIP163_EVENT_CHAN_ERR_SLOT_2PKTS;
    }

    return AdapterEvents;
}


/*----------------------------------------------------------------------------
 * CfyELib_InterruptHandler
 *
 * This function is the interrupt handler for the CfyE device interrupt
 * sources that indicate the occurrence of one or several of the requested events.
 * There may be several interrupt sources.
 *
 * This function is used to invoke the CfyE notification callback.
 */
void
CfyELib_InterruptHandler(
        const int nIRQ,
        const unsigned int Flags)
{
    unsigned int ICDeviceId = nIRQ;
    unsigned int NotifyId = ADAPTER_EIP163_NOTIFY_ID(ICDeviceId);

    LOG_INFO("%s: Flags = 0x%x\n", __func__, Flags);

    if (NotifyId >= ADAPTER_EIP163_MAX_IC_COUNT)
    {
        LOG_CRIT("%s: IC device handler id %d invalid, max id %d\n",
                 __func__,
                 NotifyId,
                 ADAPTER_EIP163_MAX_IC_COUNT - 1);
        return;
    }

    if (CfyE_Notify[NotifyId] == NULL)
    {
        LOG_CRIT("%s: IC device handler id %d invalid, not requested?\n",
                 __func__,
                 NotifyId);
        return;
    }

    /* Interrupt is expected to be disabled by now */
    if (CfyE_Notify[NotifyId]->NotifyCB_p != NULL)
    {
        CfyE_NotifyFunction_t CBFunc_p;
        unsigned int CfyEEvents;

        /* Keep the callback on stack to allow registration */
        /* of another notify request from callback */
        CBFunc_p = CfyE_Notify[NotifyId]->NotifyCB_p;

        /* Convert to CfyE events from adapter events */
        CfyEEvents = CfyELib_CfyEEvents_Get(Flags,
                                            CfyE_Notify[NotifyId]->fGlobalAIC);

        CfyE_Notify[NotifyId]->NotifyCB_p = NULL;
        CfyE_Notify[NotifyId]->EventMask  = 0;
        CfyE_Notify[NotifyId]->fGlobalAIC = false;

        LOG_INFO("%s: Invoking CfyE notification callback %p for IC device %d\n",
                 __func__,
                 CBFunc_p,
                 ICDeviceId);

        CBFunc_p(CfyEEvents);
    }

    LOG_INFO("\n %s done \n", __func__);
}
#endif


/*----------------------------------------------------------------------------
 * CfyELib_vPortHandle_IsValid
 */
bool
CfyELib_vPortHandle_IsValid(
        const CfyE_vPortHandle_t vPortHandle)
{
#ifdef ADAPTER_EIP163_STRICT_ARGS
    CfyE_vPort_Descriptor_Internal_t * vPortDscr_p =
                                (CfyE_vPort_Descriptor_Internal_t*)vPortHandle;

    if (CfyELib_vPortHandle_IsSame(vPortHandle, CfyE_vPortHandle_NULL))
        return false;

#ifdef ADAPTER_EIP163_DBG
    if (vPortDscr_p->Magic == ADAPTER_EIP163_VPORT_DSCR_MAGIC)
        return true;
    else
        return false;
#endif /* ADAPTER_EIP163_DBG */

#else

    IDENTIFIER_NOT_USED(vPortHandle);
    return true;

#endif /* ADAPTER_EIP163_STRICT_ARGS */
}


/*----------------------------------------------------------------------------
 * CfyELib_vPortId_Get
 *
 * Get the vPort index in the device for this vPort handle
 */
unsigned int
CfyELib_vPortId_Get(
        const CfyE_vPortHandle_t vPortHandle)
{
    CfyE_vPort_Descriptor_Internal_t * vPortDscr_p =
                                (CfyE_vPort_Descriptor_Internal_t*)vPortHandle;

    return vPortDscr_p->u.InUse.vPortPolicyId;
}


/*----------------------------------------------------------------------------
 * CfyELib_vPortIndexToHandle
 *
 * Get the vPort handle in the device for this vPort index
 */
bool
CfyELib_vPortIndexToHandle(
        const unsigned int DeviceId,
        const unsigned int vPortIndex,
        CfyE_vPortHandle_t * const vPortHandle_p)
{
    /* We can not return the original handle, but make a new one by finding */
    /* the corresponding pointer and filling it in. Thus creating a */
    /* new handle that has the same contents as before */
    CfyE_vPort_Descriptor_Internal_t * vPortDscr_p =
                            &(PRV_SHARED_MACSEC_CFYE_GLOBAL_VAR_GET(CfyE_Devices)[DeviceId]->vPortDscr_p[vPortIndex]);

    *vPortHandle_p = vPortDscr_p;

#ifdef ADAPTER_EIP163_DBG
    /* vPort descriptor magic number check */
    if (vPortDscr_p->Magic != ADAPTER_EIP163_VPORT_DSCR_MAGIC)
    {
        return false;
    }
#endif /* ADAPTER_EIP163_DBG */

    return true;
}


/*----------------------------------------------------------------------------
 * CfyELib_RuleHandle_IsValid
 */
bool
CfyELib_RuleHandle_IsValid(
        const CfyE_RuleHandle_t RuleHandle)
{
#ifdef ADAPTER_EIP163_STRICT_ARGS

    CfyE_Rule_Descriptor_Internal_t * RuleDscr_p =
                                (CfyE_Rule_Descriptor_Internal_t*)RuleHandle;

    if (CfyELib_RuleHandle_IsSame(RuleHandle, CfyE_RuleHandle_NULL))
        return false;

#ifdef ADAPTER_EIP163_DBG
    if (RuleDscr_p->Magic == ADAPTER_EIP163_RULE_DSCR_MAGIC)
        return true;
    else
        return false;
#endif /* ADAPTER_EIP163_DBG */

#else

    IDENTIFIER_NOT_USED(RuleHandle);
    return true;

#endif /* ADAPTER_EIP163_STRICT_ARGS */
}


/*----------------------------------------------------------------------------
 * CfyELib_RuleId_Get
 *
 * Get the rule index in the device for this rule handle
 */
unsigned int
CfyELib_RuleId_Get(
        const CfyE_RuleHandle_t RuleHandle)
{
    CfyE_Rule_Descriptor_Internal_t * RuleDscr_p =
                                (CfyE_Rule_Descriptor_Internal_t*)RuleHandle;

    return RuleDscr_p->u.InUse.RuleId;
}


/*----------------------------------------------------------------------------
 * CfyELib_RuleIndexToHandle
 *
 * Get the Rule handle in the device for this Rule index
 */
bool
CfyELib_RuleIndexToHandle(
        const unsigned int DeviceId,
        const unsigned int RuleIndex,
        CfyE_RuleHandle_t * const RuleHandle_p)
{
    /* We can not return the original handle, but make a new one by finding */
    /* the corresponding pointer and filling it in. Thus creating a */
    /* new handle that has the same contents as before */
    CfyE_Rule_Descriptor_Internal_t * RuleDscr_p =
                            &(PRV_SHARED_MACSEC_CFYE_GLOBAL_VAR_GET(CfyE_Devices)[DeviceId]->RuleDscr_p[RuleIndex]);

    *RuleHandle_p = RuleDscr_p;

#ifdef ADAPTER_EIP163_DBG
    /* Rule descriptor magic number check */
    if (RuleDscr_p->Magic != ADAPTER_EIP163_RULE_DSCR_MAGIC)
    {
        return false;
    }
#endif /* ADAPTER_EIP163_DBG */

    return true;
}


#ifdef ADAPTER_EIP163_INTERRUPTS_ENABLE
/*----------------------------------------------------------------------------
 * CfyE_Notify_Request
 */
CfyE_Status_t
CfyE_Notify_Request(
        const unsigned int DeviceId,
        const CfyE_Notify_t * const Notify_p)
{
    unsigned int AdapterEvents, ICDeviceId, NotifyId;
    int rc;
    const unsigned int * ChannelId_p;

    LOG_INFO("\n %s \n", __func__);

    ADAPTER_EIP163_CHECK_INT_ATMOST(DeviceId, ADAPTER_EIP163_MAX_NOF_DEVICES - 1);
    ADAPTER_EIP163_CHECK_POINTER(Notify_p);
    ADAPTER_EIP163_CHECK_INT_ATMOST(Notify_p->ChannelId,
                                    PRV_SHARED_MACSEC_CFYE_GLOBAL_VAR_GET(CfyE_Devices)[DeviceId]->ChannelsCount - 1);
    ADAPTER_EIP163_CHECK_INT_ATLEAST(Notify_p->EventMask, 1);

    /* Get AIC device Id */
    ChannelId_p = Notify_p->fGlobal ? NULL : &Notify_p->ChannelId;
    ICDeviceId = CfyELib_ICDeviceID_Get(DeviceId, ChannelId_p);

    NotifyId = ADAPTER_EIP163_NOTIFY_ID(ICDeviceId);

    ADAPTER_EIP163_CHECK_INT_ATMOST(NotifyId,
                                    ADAPTER_EIP163_MAX_IC_COUNT - 1);

    CfyELib_Device_Lock(DeviceId);

    if (!CfyELib_Initialized_Check(DeviceId, __func__, true))
    {
        CfyELib_Device_Unlock(DeviceId);
        return CFYE_ERROR_INTERNAL;
    }

    /* install it */
    {
        /* Allocate notification if not done yet */
        if (CfyE_Notify[NotifyId] == NULL)
        {
            CfyE_Notify[NotifyId] =
                        Adapter_Alloc(sizeof(CfyE_Notify_Internal_t));
            if (CfyE_Notify[NotifyId] == NULL)
            {
                CfyELib_Device_Unlock(DeviceId);
                LOG_CRIT("%s: failed, out of memory", __func__);
                return CFYE_ERROR_INTERNAL;
            }
        }

        CfyE_Notify[NotifyId]->EventMask  = Notify_p->EventMask;
        CfyE_Notify[NotifyId]->NotifyCB_p = Notify_p->CBFunc_p;
        CfyE_Notify[NotifyId]->fGlobalAIC = Notify_p->fGlobal;
    }

    AdapterEvents = CfyELib_AdapterEvents_Get(Notify_p->EventMask,
                                              Notify_p->fGlobal);

    /* Note: The results may have become available before the call to */
    /*       CfyE_Notify_Request() and the associated interrupts may already */
    /*       be pending. In this case the interrupt will occur immediately. */
    rc = Adapter_Interrupt_Enable(ICDeviceId, AdapterEvents);

    if (rc)
        return CFYE_DLIB_ERROR(rc);


    CfyELib_Device_Unlock(DeviceId);

    LOG_INFO("\n %s done \n", __func__);

    return CFYE_STATUS_OK;
}
#endif /* ADAPTER_EIP163_INTERRUPTS_ENABLE */


/* end of file adapter_cfye_support.c */
