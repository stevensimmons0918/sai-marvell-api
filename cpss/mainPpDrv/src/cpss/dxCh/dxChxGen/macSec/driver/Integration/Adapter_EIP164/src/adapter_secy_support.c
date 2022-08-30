/* adapter_secy_support.c
 *
 * SecY API and Adapter internal SecY API implementation
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

#include <Integration/Adapter_EIP164/incl/api_secy.h>              /* SecY API */
#include <Integration/Adapter_EIP164/incl/adapter_secy_support.h>  /* Adapter internal SecY API */

/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

/* Top-level Adapter configuration */
#include <Integration/Adapter_EIP164/incl/c_adapter_eip164.h>

/* Basic defs API, uint32_t, uint8_t */
#include <Kit/DriverFramework/incl/basic_defs.h>

/* Logging API */
#include <Kit/Log/incl/log.h>                /* LOG_* */

/* EIP-164 Driver Library SecY API */
#include <Kit/EIP164/incl/eip164_types.h>
#include <Kit/EIP164/incl/eip164_secy.h>

/* Adapter Alloc API */
#include <Integration/Adapter_EIP164/incl/adapter_alloc.h>      /* Adapter_Alloc()/_Free() */

/* Adapter Sleep API */
#include <Integration/Adapter_EIP164/incl/adapter_sleep.h>      /* Adapter_SleepMS() */

/* Adapter Lock API */
#include <Integration/Adapter_EIP164/incl/adapter_lock.h>       /* Adapter_Lock* */

#ifdef ADAPTER_EIP164_INTERRUPTS_ENABLE
/* Adapter Interrupts API */
#include <Integration/Adapter_EIP164/incl/adapter_interrupts.h>
#endif

/* SA Builder. */
#include <Kit/SABuilder_MACsec/incl/sa_builder_macsec.h>


/*----------------------------------------------------------------------------
 * Definitions and macros
 */

/* Adapter EIP-164 Global AIC events resulting in interrupts */
#define ADAPTER_EIP164_EVENT_DROP_CLASS         BIT_0
#define ADAPTER_EIP164_EVENT_DROP_PP            BIT_1
#define ADAPTER_EIP164_EVENT_DROP_MTU           BIT_2
#define ADAPTER_EIP164_EVENT_ENG_IRQ            BIT_3

#define ADAPTER_EIP164_EVENT_STAT_SA_THR        BIT_4
#define ADAPTER_EIP164_EVENT_STAT_SECY_THR      BIT_5
#define ADAPTER_EIP164_EVENT_STAT_IFC_THR       BIT_6
#define ADAPTER_EIP164_EVENT_STAT_IFC1_THR      BIT_7
#define ADAPTER_EIP164_EVENT_STAT_RXCAM_THR     BIT_8

#define ADAPTER_EIP164_EVENT_SA_PN_THR          BIT_10
#define ADAPTER_EIP164_EVENT_SA_EXPIRED         BIT_11
#define ADAPTER_EIP164_EVENT_ECC_ERR            BIT_12

/* Adapter EIP-164 Channel AIC events resulting in interrupts */
#define ADAPTER_EIP164_EVENT_CHAN_ERR_SOP_WO_EOP  BIT_0
#define ADAPTER_EIP164_EVENT_CHAN_ERR_EOP_WO_SOP  BIT_1
#define ADAPTER_EIP164_EVENT_CHAN_ERR_XFER_WO_SOP BIT_2
#define ADAPTER_EIP164_EVENT_CHAN_ERR_SLOT_SOP    BIT_3
#define ADAPTER_EIP164_EVENT_CHAN_ERR_SLOT_CHID   BIT_4
#define ADAPTER_EIP164_EVENT_CHAN_ERR_NOT_B2B     BIT_5
#define ADAPTER_EIP164_EVENT_CHAN_RXCAM_HIT_MULT  BIT_6
#define ADAPTER_EIP164_EVENT_CHAN_RXCAM_MISS      BIT_7
#define ADAPTER_EIP164_EVENT_PKT_DATA_OVERRUN     BIT_8
#define ADAPTER_EIP164_EVENT_PKT_CFY_OVERRUN      BIT_9

#define SECY_TEST_EQUAL(a,b) \
    extern int SecY_##a##must_be_equal[1-2*((unsigned int)(a)!=(unsigned int)(b))]

/* As we are assigning between enum type values defined at the
 * Adapter level and enum type values defined at the Driver Library
 * level, enum values of the corresponding types must be
 * equal. Perform compile-time checks here.
 */
SECY_TEST_EQUAL(SECY_ROLE_EGRESS,EIP164_MODE_EGRESS);
SECY_TEST_EQUAL(SECY_ROLE_INGRESS,EIP164_MODE_INGRESS);
SECY_TEST_EQUAL(SECY_ROLE_EGRESS_INGRESS,EIP164_MODE_EGRESS_INGRESS);

SECY_TEST_EQUAL(SECY_PORT_COMMON,EIP164_PORT_COMMON);
SECY_TEST_EQUAL(SECY_PORT_RESERVED,EIP164_PORT_RESERVED);
SECY_TEST_EQUAL(SECY_PORT_CONTROLLED,EIP164_PORT_CONTROLLED);
SECY_TEST_EQUAL(SECY_PORT_UNCONTROLLED,EIP164_PORT_UNCONTROLLED);

SECY_TEST_EQUAL(SECY_SA_DROP_CRC_ERROR,EIP164_SECY_SA_DROP_CRC_ERROR);
SECY_TEST_EQUAL(SECY_SA_DROP_PKT_ERROR,EIP164_SECY_SA_DROP_PKT_ERROR);
SECY_TEST_EQUAL(SECY_SA_DROP_INTERNAL,EIP164_SECY_SA_DROP_INTERNAL);
SECY_TEST_EQUAL(SECY_SA_DROP_NONE,EIP164_SECY_SA_DO_NOT_DROP);

SECY_TEST_EQUAL(SECY_FRAME_VALIDATE_DISABLE,EIP164_VALIDATE_FRAME_DISABLED);
SECY_TEST_EQUAL(SECY_FRAME_VALIDATE_CHECK,EIP164_VALIDATE_FRAME_CHECK);
SECY_TEST_EQUAL(SECY_FRAME_VALIDATE_STRICT,EIP164_VALIDATE_FRAME_STRICT);

/* We also assign directly between interrupt sources at the EIP164
   packet engine level and SecY events, so check we have equal values. */

SECY_TEST_EQUAL(SECY_EVENT_PE_LENGTH_ERR0, EIP164_SECY_INT_LENGTH_ERR0 << 16);
SECY_TEST_EQUAL(SECY_EVENT_PE_LENGTH_ERR1, EIP164_SECY_INT_LENGTH_ERR1 << 16);
SECY_TEST_EQUAL(SECY_EVENT_PE_LENGTH_ERR2, EIP164_SECY_INT_LENGTH_ERR2 << 16);
SECY_TEST_EQUAL(SECY_EVENT_PE_CTR_ERR, EIP164_SECY_INT_CTR_ERR << 16);
SECY_TEST_EQUAL(SECY_EVENT_PE_TOKEN_ERR, EIP164_SECY_INT_TOKEN_ERR << 16);
SECY_TEST_EQUAL(SECY_EVENT_PE_CTX_ERR, EIP164_SECY_INT_CTX_ERR << 16);
SECY_TEST_EQUAL(SECY_EVENT_PE_ECC_ERR, EIP164_SECY_INT_ECC_ERR << 16);
SECY_TEST_EQUAL(SECY_EVENT_PE_IB_ZERO_PKTNUM, EIP164_SECY_INT_IB_ZERO_PKTNUM << 16);
SECY_TEST_EQUAL(SECY_EVENT_PE_OB_SEQNR_THR, EIP164_SECY_INT_OB_SEQNR_THRSH << 16);
SECY_TEST_EQUAL(SECY_EVENT_PE_IB_AUTH_FAIL, EIP164_SECY_INT_IB_AUTH_FAIL << 16);
SECY_TEST_EQUAL(SECY_EVENT_PE_OB_SEQNR_RO, EIP164_SECY_INT_OB_SEQNR_RLOVR << 16);
SECY_TEST_EQUAL(SECY_EVENT_PE_IB_SL, EIP164_SECY_INT_IB_SL << 16);
SECY_TEST_EQUAL(SECY_EVENT_PE_OB_MTU_CHECK, EIP164_SECY_INT_OB_MTU_CHECK << 16);
SECY_TEST_EQUAL(SECY_EVENT_PE_OB_EXPANSION_ERR, EIP164_SECY_INT_OB_EXPANSION_ERR << 16);


SECY_TEST_EQUAL(EIP164_ECC_NOF_STATUS_COUNTERS, SECY_ECC_NOF_STATUS_COUNTERS);


#define ADAPTER_NOF_IC_PER_EIP164_DEVICE \
    (ADAPTER_EIP164_MAX_IC_COUNT/ADAPTER_EIP164_MAX_NOF_DEVICES)


/*----------------------------------------------------------------------------
 * Local variables
 */

#ifdef ADAPTER_EIP164_INTERRUPTS_ENABLE
SecY_Notify_Internal_t * SecY_Notify[ADAPTER_EIP164_MAX_IC_COUNT];
#endif




/*----------------------------------------------------------------------------
 * SecY_SAHandle_NULL
 */
const SecY_SAHandle_t SecY_SAHandle_NULL = { NULL };



/*----------------------------------------------------------------------------
 * SecYLib_SAHandle_IsSame
 *
 * Check if two SA handles are equal.
 *
 * Handle1_p (input)
 *     Pointer to first handle.
 *
 * Handle2_p (input)
 *     Pointer to second handle.
 *
 * Return: true if both handles are equal, false otherwise.
 */
bool
SecYLib_SAHandle_IsSame(
        const SecY_SAHandle_t * const Handle1_p,
        const SecY_SAHandle_t * const Handle2_p)
{
    if (cpssOsMemCmp(Handle1_p, Handle2_p, sizeof(SecY_SAHandle_t)) != 0)
    {
        return false;
    }

    return true;
}

/*----------------------------------------------------------------------------
 * SecYLib_Device_Sync
 */
SecY_Status_t
SecYLib_Device_Sync(
        const unsigned int DeviceId)
{
    EIP164_Error_t Rc;

    LOG_INFO("\n\t EIP164_Device_Sync \n");

    /* Sync with the device */
    Rc = EIP164_Device_Sync(&PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->IOArea);

    if (Rc == EIP164_BUSY_RETRY_LATER)
    {
        /* Wait till the device synchronization is done */
        unsigned int LoopCounter = ADAPTER_EIP164_MAX_NOF_SYNC_RETRY_COUNT;

        while (LoopCounter)
        {
            LOG_INFO("\n\t EIP164_Device_Is_Sync_Done \n");

            Rc = EIP164_Device_Is_Sync_Done(&PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->IOArea);

            if (Rc != EIP164_BUSY_RETRY_LATER)
            {
                break;
            }

            Adapter_SleepMS(ADAPTER_EIP164_SYNC_RETRY_TIMEOUT_MS);
            LoopCounter--;
        }
    }

    if (Rc != EIP164_NO_ERROR)
    {
        return SECY_DLIB_ERROR(Rc);
    }
    else
    {
        return SECY_STATUS_OK;
    }
}


/*----------------------------------------------------------------------------
 * SecYLib_AdapterEvents_Get
 */
static inline unsigned int
SecYLib_AdapterEvents_Get(
        const unsigned int SecYEvents,
        const bool fGlobalAIC)
{
    unsigned int AdapterEvents = 0;

    if (fGlobalAIC)
    {
        if (SecYEvents & SECY_EVENT_DROP_CLASS)
            AdapterEvents |= ADAPTER_EIP164_EVENT_DROP_CLASS;

        if (SecYEvents & SECY_EVENT_DROP_PP)
            AdapterEvents |= ADAPTER_EIP164_EVENT_DROP_PP;

        if (SecYEvents & SECY_EVENT_DROP_MTU)
            AdapterEvents |= ADAPTER_EIP164_EVENT_DROP_MTU;

        if (SecYEvents & SECY_EVENT_ENG_IRQ)
            AdapterEvents |= ADAPTER_EIP164_EVENT_ENG_IRQ;

        if (SecYEvents & SECY_EVENT_STAT_SA_THR)
            AdapterEvents |= ADAPTER_EIP164_EVENT_STAT_SA_THR;

        if (SecYEvents & SECY_EVENT_STAT_SECY_THR)
            AdapterEvents |= ADAPTER_EIP164_EVENT_STAT_SECY_THR;

        if (SecYEvents & SECY_EVENT_STAT_IFC_THR)
            AdapterEvents |= ADAPTER_EIP164_EVENT_STAT_IFC_THR;

        if (SecYEvents & SECY_EVENT_STAT_IFC1_THR)
            AdapterEvents |= ADAPTER_EIP164_EVENT_STAT_IFC1_THR;

        if (SecYEvents & SECY_EVENT_STAT_RXCAM_THR)
            AdapterEvents |= ADAPTER_EIP164_EVENT_STAT_RXCAM_THR;

        if (SecYEvents & SECY_EVENT_SA_PN_THR)
            AdapterEvents |= ADAPTER_EIP164_EVENT_SA_PN_THR;

        if (SecYEvents & SECY_EVENT_SA_EXPIRED)
            AdapterEvents |= ADAPTER_EIP164_EVENT_SA_EXPIRED;

        if (SecYEvents & SECY_EVENT_ECC_ERR)
            AdapterEvents |= ADAPTER_EIP164_EVENT_ECC_ERR;
    }
    else
    {
        if (SecYEvents & SECY_EVENT_CHAN_ERR_SOP_WO_EOP)
            AdapterEvents |= ADAPTER_EIP164_EVENT_CHAN_ERR_SOP_WO_EOP;

        if (SecYEvents & SECY_EVENT_CHAN_ERR_EOP_WO_SOP)
            AdapterEvents |= ADAPTER_EIP164_EVENT_CHAN_ERR_EOP_WO_SOP;

        if (SecYEvents & SECY_EVENT_CHAN_ERR_XFER_WO_SOP)
            AdapterEvents |= ADAPTER_EIP164_EVENT_CHAN_ERR_XFER_WO_SOP;

        if (SecYEvents & SECY_EVENT_CHAN_ERR_SLOT_SOP)
            AdapterEvents |= ADAPTER_EIP164_EVENT_CHAN_ERR_SLOT_SOP;

        if (SecYEvents & SECY_EVENT_CHAN_ERR_SLOT_CHID)
            AdapterEvents |= ADAPTER_EIP164_EVENT_CHAN_ERR_SLOT_CHID;

        if (SecYEvents & SECY_EVENT_CHAN_ERR_NOT_B2B)
            AdapterEvents |= ADAPTER_EIP164_EVENT_CHAN_ERR_NOT_B2B;

        if (SecYEvents & SECY_EVENT_CHAN_RXCAM_HIT_MULT)
            AdapterEvents |= ADAPTER_EIP164_EVENT_CHAN_RXCAM_HIT_MULT;

        if (SecYEvents & SECY_EVENT_CHAN_RXCAM_MISS)
            AdapterEvents |= ADAPTER_EIP164_EVENT_CHAN_RXCAM_MISS;

        if (SecYEvents & SECY_EVENT_CHAN_PKT_DATA_OVERRUN)
            AdapterEvents |= ADAPTER_EIP164_EVENT_PKT_DATA_OVERRUN;

        if (SecYEvents & SECY_EVENT_CHAN_PKT_CFY_OVERRUN)
            AdapterEvents |= ADAPTER_EIP164_EVENT_PKT_CFY_OVERRUN;
    }

    return AdapterEvents;
}


/*----------------------------------------------------------------------------
 * SecYLib_SecYEvents_Get
 */
static inline unsigned int
SecYLib_SecYEvents_Get(
        const unsigned int AdapterEvents,
        const bool fGlobalAIC)
{
    unsigned int SecYEvents = 0;

    if (fGlobalAIC)
    {
        if (AdapterEvents & ADAPTER_EIP164_EVENT_DROP_CLASS)
            SecYEvents |= SECY_EVENT_DROP_CLASS;

        if (AdapterEvents & ADAPTER_EIP164_EVENT_DROP_PP)
            SecYEvents |= SECY_EVENT_DROP_PP;

        if (AdapterEvents & ADAPTER_EIP164_EVENT_DROP_MTU)
            SecYEvents |= SECY_EVENT_DROP_MTU;

        if (AdapterEvents & ADAPTER_EIP164_EVENT_ENG_IRQ)
            SecYEvents |= SECY_EVENT_ENG_IRQ;

        if (AdapterEvents & ADAPTER_EIP164_EVENT_STAT_SA_THR)
            SecYEvents |= SECY_EVENT_STAT_SA_THR;

        if (AdapterEvents & ADAPTER_EIP164_EVENT_STAT_SECY_THR)
            SecYEvents |= SECY_EVENT_STAT_SECY_THR;

        if (AdapterEvents & ADAPTER_EIP164_EVENT_STAT_IFC_THR)
            SecYEvents |= SECY_EVENT_STAT_IFC_THR;

        if (AdapterEvents & ADAPTER_EIP164_EVENT_STAT_IFC1_THR)
            SecYEvents |= SECY_EVENT_STAT_IFC1_THR;

        if (AdapterEvents & ADAPTER_EIP164_EVENT_STAT_RXCAM_THR)
            SecYEvents |= SECY_EVENT_STAT_RXCAM_THR;

        if (AdapterEvents & ADAPTER_EIP164_EVENT_SA_PN_THR)
            SecYEvents |= SECY_EVENT_SA_PN_THR;

        if (AdapterEvents & ADAPTER_EIP164_EVENT_SA_EXPIRED)
            SecYEvents |= SECY_EVENT_SA_EXPIRED;

        if (AdapterEvents & ADAPTER_EIP164_EVENT_ECC_ERR)
            SecYEvents |= SECY_EVENT_ECC_ERR;
    }
    else
    {
        if (AdapterEvents & ADAPTER_EIP164_EVENT_CHAN_ERR_SOP_WO_EOP)
            SecYEvents |= SECY_EVENT_CHAN_ERR_SOP_WO_EOP;

        if (AdapterEvents & ADAPTER_EIP164_EVENT_CHAN_ERR_EOP_WO_SOP)
            SecYEvents |= SECY_EVENT_CHAN_ERR_EOP_WO_SOP;

        if (AdapterEvents & ADAPTER_EIP164_EVENT_CHAN_ERR_XFER_WO_SOP)
            SecYEvents |= SECY_EVENT_CHAN_ERR_XFER_WO_SOP;

        if (AdapterEvents & ADAPTER_EIP164_EVENT_CHAN_ERR_SLOT_SOP)
            SecYEvents |= SECY_EVENT_CHAN_ERR_SLOT_SOP;

        if (AdapterEvents & ADAPTER_EIP164_EVENT_CHAN_ERR_SLOT_CHID)
            SecYEvents |= SECY_EVENT_CHAN_ERR_SLOT_CHID;

        if (AdapterEvents & ADAPTER_EIP164_EVENT_CHAN_ERR_NOT_B2B)
            SecYEvents |= SECY_EVENT_CHAN_ERR_NOT_B2B;

        if (AdapterEvents & ADAPTER_EIP164_EVENT_CHAN_RXCAM_HIT_MULT)
            SecYEvents |= SECY_EVENT_CHAN_RXCAM_HIT_MULT;

        if (AdapterEvents & ADAPTER_EIP164_EVENT_CHAN_RXCAM_MISS)
            SecYEvents |= SECY_EVENT_CHAN_RXCAM_MISS;

        if (AdapterEvents & ADAPTER_EIP164_EVENT_PKT_DATA_OVERRUN)
            SecYEvents |= SECY_EVENT_CHAN_PKT_DATA_OVERRUN;

        if (AdapterEvents & ADAPTER_EIP164_EVENT_PKT_CFY_OVERRUN)
            SecYEvents |= SECY_EVENT_CHAN_PKT_CFY_OVERRUN;
    }

    return SecYEvents;
}


/*----------------------------------------------------------------------------
 * SecYLib_EIP66Sources_Get
 */
static inline unsigned int
SecYLib_EIP66Sources_Get(
        const unsigned int SecYEvents)
{
    return SecYEvents >> 16 ;
}


/*----------------------------------------------------------------------------
 * SecYLib_EIP66Events_Get
 */
static inline unsigned int
SecYLib_EIP66Events_Get(
        const unsigned int EIP66Sources)
{
    return EIP66Sources << 16;
}


#ifdef ADAPTER_EIP164_INTERRUPTS_ENABLE
/*----------------------------------------------------------------------------
 * SecYLib_ICDeviceID_Get
 */
unsigned int
SecYLib_ICDeviceID_Get(
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

    return ADAPTER_EIP164_ICDEV_ID(ADAPTER_EIP164_DEV_ID(DeviceId),
                                   fGlobal,
                                   fPE,
                                   ChID);

}


/*----------------------------------------------------------------------------
 * SecYLib_InterruptHandler
 */
void
SecYLib_InterruptHandler(
        const int nIRQ,
        const unsigned int Flags)
{
    unsigned int ICDeviceId = nIRQ;
    unsigned int NotifyId = ADAPTER_EIP164_NOTIFY_ID(ICDeviceId);

    LOG_INFO("%s: Flags = 0x%x\n", __func__, Flags);

    if (NotifyId >= ADAPTER_EIP164_MAX_IC_COUNT)
    {
        LOG_CRIT("%s: IC device handler id %d invalid, max id %d\n",
                 __func__,
                 NotifyId,
                 ADAPTER_EIP164_MAX_IC_COUNT - 1);
        return;
    }

    if (SecY_Notify[NotifyId] == NULL)
    {
        LOG_CRIT("%s: IC device handler id %d invalid, not requested?\n",
                 __func__,
                 NotifyId);
        return;
    }

    /* Interrupt is expected to be disabled by now */
    if (SecY_Notify[NotifyId]->NotifyCB_p != NULL)
    {
        SecY_NotifyFunction_t CBFunc_p;
        unsigned int SecYEvents;
        int rc;

        /* Keep the callback on stack to allow registration */
        /* of another notify request from callback */
        CBFunc_p = SecY_Notify[NotifyId]->NotifyCB_p;

        /* Convert to SecY events from adapter events */
        SecYEvents = SecYLib_SecYEvents_Get(Flags,
                                            SecY_Notify[NotifyId]->fGlobalAIC);

        if (SecY_Notify[NotifyId]->fGlobalAIC &&
            (ADAPTER_EIP164_EVENT_ENG_IRQ & Flags))
        {
            unsigned int PE_ICDeviceId = ADAPTER_EIP164_ICDEV_ID(
                                            ADAPTER_EIP164_DEV_ID(
                                               SecY_Notify[NotifyId]->DeviceId),
                                            false,
                                            true,
                                            0);
            unsigned int EIP66_Sources;
            rc = Adapter_Interrupt_EnabledStatus_Get(PE_ICDeviceId, &EIP66_Sources);
            if (rc)
            {
                LOG_CRIT("%s: Adapter_Interrupt_EnabledStatus_Get returns %d\n",
                         __func__,rc);
            }

            SecYEvents |= SecYLib_EIP66Events_Get(EIP66_Sources);
            rc = Adapter_Interrupt_Clear(PE_ICDeviceId, EIP66_Sources);
            if (rc)
            {
                LOG_CRIT("%s: Adapter_Interrupt_Clear returns %d\n",
                         __func__,rc);
            }
        }

        SecY_Notify[NotifyId]->NotifyCB_p = NULL;
        SecY_Notify[NotifyId]->EventMask  = 0;
        SecY_Notify[NotifyId]->fGlobalAIC = false;

        LOG_INFO("%s: Invoking SecY notification callback for IC device %d\n",
                 __func__,
                 ICDeviceId);

        CBFunc_p(SecYEvents);
    }

    LOG_INFO("\n %s done \n", __func__);
}
#endif /* ADAPTER_EIP164_INTERRUPTS_ENABLE */


/*----------------------------------------------------------------------------
 * SecYLib_Device_Lock
 */
void
SecYLib_Device_Lock(
        const unsigned int DeviceId)
{
    Adapter_Lock_Acquire(&PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device_StaticFields)[DeviceId].Lock,
                         &PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device_StaticFields)[DeviceId].Flags);
}


/*----------------------------------------------------------------------------
 * SecYLib_Device_Unlock
 */
void
SecYLib_Device_Unlock(
        const unsigned int DeviceId)
{
    Adapter_Lock_Release(&PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device_StaticFields)[DeviceId].Lock,
                         &PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device_StaticFields)[DeviceId].Flags);
}


/*----------------------------------------------------------------------------
 * SecYLib_SAHandleToIndex
 */
bool
SecYLib_SAHandleToIndex(
        const SecY_SAHandle_t SAHandle,
        unsigned int * const SAIndex_p,
        unsigned int * const SCIndex_p,
        unsigned int * const SAMFlowCtrlIndex_p)
{
    SecY_SA_Descriptor_t * SADscr_p = (SecY_SA_Descriptor_t*)SAHandle.p;

    if (SADscr_p == NULL)
        return false;

    *SAIndex_p = SADscr_p->u.InUse.SAIndex;

    if (SAMFlowCtrlIndex_p)
    {
        *SAMFlowCtrlIndex_p = SADscr_p->u.InUse.SAMFlowCtrlIndex;
    }

    if (SCIndex_p)
    {
        *SCIndex_p = SADscr_p->u.InUse.SCIndex;
    }

#ifdef ADAPTER_EIP164_DBG
    /* SA descriptor magic number check */
    if (SADscr_p->Magic != ADAPTER_EIP164_SA_DSCR_MAGIC)
    {
        return false;
    }
#endif /* ADAPTER_EIP164_DBG */

    return true;
}


/*----------------------------------------------------------------------------
 * SecYLib_SAIndexToHandle
 *
 * Get the SA handle in the device for this SA index
 */
bool
SecYLib_SAIndexToHandle(
        const unsigned int DeviceId,
        const unsigned int SAIndex,
        SecY_SAHandle_t * const SAHandle_p)
{
    /* We can not return the original handle, but make a new one by finding */
    /* the corresponding pointer and filling it in. Thus creating a */
    /* new handle that has the same contents as before */
    SecY_SA_Descriptor_t * SADscr_p =
                                &(PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->SADscr_p[SAIndex]);

    SAHandle_p->p = SADscr_p;

#ifdef ADAPTER_EIP164_DBG
    /* SA descriptor magic number check */
    if (SADscr_p->Magic != ADAPTER_EIP164_SA_DSCR_MAGIC)
    {
        return false;
    }
#endif /* ADAPTER_EIP164_DBG */

    return true;
}


/*----------------------------------------------------------------------------
 * SecYLib_SA_Read
 */
SecY_Status_t
SecYLib_SA_Read(
        const unsigned int DeviceId,
        const unsigned int SAIndex,
        const unsigned int WordOffset,
        const unsigned int WordCount,
        uint32_t * Transform_p)
{
    EIP164_Error_t Rc;

    LOG_INFO("\n\t EIP164_SecY_SA_Read \n");

    /* Read this SA from the device */
    Rc = EIP164_SecY_SA_Read(&PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->IOArea,
                             SAIndex,
                             WordOffset,
                             WordCount,
                             Transform_p);
    if (Rc != EIP164_NO_ERROR)
    {
        LOG_CRIT("%s: Failed to read SA from EIP-164 device for device %d "
                 "(%s), error %d\n",
                 __func__,
                 DeviceId,
                 ADAPTER_EIP164_DEVICE_NAME(DeviceId),
                 Rc);
        return SECY_DLIB_ERROR(Rc);
    }

    return SECY_STATUS_OK;
}


#ifdef ADAPTER_EIP164_MODE_EGRESS
/*----------------------------------------------------------------------------
 * SecYLib_SA_Chained_Get
 */
SecY_Status_t
SecYLib_SA_Chained_Get(
        const unsigned int DeviceId,
        const unsigned int SAIndex,
        unsigned int * const NewSAIndex,
        bool * const fSAInUse)
{
    SecY_Status_t SecY_Rc;
    SABuilder_Status_t SAB_Rc;
    unsigned int Offset;
    uint32_t UpdControlWord;

    SAB_Rc = SABuilder_UpdateCtrlOffset_Get(0, &Offset);
    if (SAB_Rc != SAB_STATUS_OK)
    {
        return SECY_ERROR_INTERNAL;
    }

    SecY_Rc = SecYLib_SA_Read(DeviceId, SAIndex, Offset, 1, &UpdControlWord);
    if (SecY_Rc != SECY_STATUS_OK)
    {
        return SecY_Rc;
    }

    *NewSAIndex = UpdControlWord & MASK_13_BITS;
    *fSAInUse = ( (UpdControlWord & (BIT_31|BIT_15)) == (BIT_31|BIT_15));

    return SECY_STATUS_OK;
}
#endif


/*----------------------------------------------------------------------------
 * SecYLib_vPort_Statistics_Clear
 */
SecY_Status_t
SecYLib_vPort_Statistics_Clear(
        unsigned int DeviceId,
        unsigned int vPort)
{
    EIP164_Error_t Rc = 0;
    SecY_Role_t Role;
#ifdef ADAPTER_EIP164_STRICT_ARGS
    if (DeviceId >= ADAPTER_EIP164_MAX_NOF_DEVICES)
        return SECY_ERROR_BAD_PARAMETER;
#endif
    SecYLib_Device_Lock(DeviceId);

    if (!PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device_StaticFields)[DeviceId].fInitialized)
    {
        SecYLib_Device_Unlock(DeviceId);
        return SECY_ERROR_INTERNAL;
    }

    Role = PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->Role;
#ifdef ADAPTER_EIP164_MODE_EGRESS
    if (Role == SECY_ROLE_EGRESS || Role == SECY_ROLE_EGRESS_INGRESS)
    {
        Rc = EIP164_SecY_Stat_E_Clear(&PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->IOArea, vPort);
        if (Rc != EIP164_NO_ERROR)
        {
            LOG_CRIT("%s: Failed to clear EIP164 SecY statistics for device %d "
                     "(%s), error %d\n",
                     __func__,
                     DeviceId,
                     ADAPTER_EIP164_DEVICE_NAME(DeviceId),
                     Rc);
            goto lError;
        }
        Rc = EIP164_SecY_Ifc_Stat_E_Clear(&PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->IOArea,
                                          vPort);
        if (Rc != EIP164_NO_ERROR)
        {
            LOG_CRIT("%s: Failed to clear EIP164 Ifc statistics for device %d "
                     "(%s), error %d\n",
                     __func__,
                     DeviceId,
                     ADAPTER_EIP164_DEVICE_NAME(DeviceId),
                     Rc);
            goto lError;
        }
    }
#endif

#ifdef ADAPTER_EIP164_MODE_INGRESS
    if (Role == SECY_ROLE_INGRESS || Role == SECY_ROLE_EGRESS_INGRESS)
    {
        Rc = EIP164_SecY_Stat_I_Clear(&PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->IOArea, vPort);
        if (Rc != EIP164_NO_ERROR)
        {
            LOG_CRIT("%s: Failed to clear EIP164 SecY statistics for device %d "
                     "(%s), error %d\n",
                     __func__,
                     DeviceId,
                     ADAPTER_EIP164_DEVICE_NAME(DeviceId),
                     Rc);
            goto lError;
        }
        Rc = EIP164_SecY_Ifc_Stat_I_Clear(&PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->IOArea,
                                          vPort);
        if (Rc != EIP164_NO_ERROR)
        {
            LOG_CRIT("%s: Failed to clear EIP164 Ifc statistics for device %d "
                     "(%s), error %d\n",
                     __func__,
                     DeviceId,
                     ADAPTER_EIP164_DEVICE_NAME(DeviceId),
                     Rc);
            goto lError;
        }
    }
#endif

lError:
    SecYLib_Device_Unlock(DeviceId);
    if (Rc != EIP164_NO_ERROR)
    {
        return SECY_DLIB_ERROR(Rc);
    }
    else
    {
        return SECY_STATUS_OK;
    }
}


/*----------------------------------------------------------------------------
 * External SecY API implementation
 */

#ifdef ADAPTER_EIP164_INTERRUPTS_ENABLE
/*----------------------------------------------------------------------------
 * SecY_Notify_Request
 */
SecY_Status_t
SecY_Notify_Request(
        const unsigned int DeviceId,
        const SecY_Notify_t * const Notify_p)
{
    unsigned int AdapterEvents, ICDeviceId, NotifyId;
    const unsigned int * ChannelId_p;
    int rc;

    LOG_INFO("\n %s \n", __func__);

#ifdef ADAPTER_EIP164_STRICT_ARGS
    if (DeviceId >= ADAPTER_EIP164_MAX_NOF_DEVICES)
        return SECY_ERROR_BAD_PARAMETER;

    if (Notify_p == NULL)
        return SECY_ERROR_BAD_PARAMETER;

    if (Notify_p->ChannelId >= PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[DeviceId]->ChannelCount)
        return SECY_ERROR_BAD_PARAMETER;
#endif

    if (Notify_p->EventMask == 0)
    {
        LOG_WARN("\n Warning: %s invoked with EventMask==0.\n",__func__);
    }

    /* Get AIC device Id */
    ChannelId_p = Notify_p->fGlobal ? NULL : &Notify_p->ChannelId;
    ICDeviceId = SecYLib_ICDeviceID_Get(DeviceId, ChannelId_p);
    NotifyId = ADAPTER_EIP164_NOTIFY_ID(ICDeviceId);

#ifdef ADAPTER_EIP164_STRICT_ARGS
    if (NotifyId >= ADAPTER_EIP164_MAX_IC_COUNT)
        return SECY_ERROR_BAD_PARAMETER;
#endif

    SecYLib_Device_Lock(DeviceId);

    if (!PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device_StaticFields)[DeviceId].fInitialized)
    {
        SecYLib_Device_Unlock(DeviceId);
        return SECY_ERROR_INTERNAL;
    }

    /* Allocate notification if not done yet */
    if (SecY_Notify[NotifyId] == NULL)
    {
        SecY_Notify[NotifyId] =
                    Adapter_Alloc(sizeof(SecY_Notify_Internal_t));
        if (SecY_Notify[NotifyId] == NULL)
        {
            SecYLib_Device_Unlock(DeviceId);
            LOG_CRIT("%s: failed, out of memory", __func__);
            return SECY_ERROR_INTERNAL;
        }
    }

    /* install it */
    SecY_Notify[NotifyId]->EventMask  = Notify_p->EventMask;
    SecY_Notify[NotifyId]->NotifyCB_p = Notify_p->CBFunc_p;
    SecY_Notify[NotifyId]->fGlobalAIC = Notify_p->fGlobal;
    SecY_Notify[NotifyId]->DeviceId   = DeviceId;

    AdapterEvents = SecYLib_AdapterEvents_Get(Notify_p->EventMask,
                                              Notify_p->fGlobal);

    /* Clear and Enable interrupts again if requested */
    if (Notify_p->fGlobal && (AdapterEvents & ADAPTER_EIP164_EVENT_ENG_IRQ))
    {
        /* Access PE AIC */
        unsigned int Mask;

        Mask = SecYLib_EIP66Sources_Get(Notify_p->EventMask);

        rc = Adapter_Interrupt_Enable(ADAPTER_EIP164_ICDEV_ID(
                                        ADAPTER_EIP164_DEV_ID(DeviceId),
                                        false, /* Global AIC */
                                        true,  /* PE AIC */
                                        0),
                                 Mask);
        if (rc) return SECY_DLIB_ERROR(rc);
    }

    /* Note: The results may have become available before the call to */
    /*       SecY_Notify_Request() and the associated interrupts may already */
    /*       be pending. In this case the interrupt will occur immediately. */
    rc = Adapter_Interrupt_Enable(ICDeviceId, AdapterEvents);
    if (rc) return SECY_DLIB_ERROR(rc);

    SecYLib_Device_Unlock(DeviceId);

    LOG_INFO("\n %s done \n", __func__);

    return SECY_STATUS_OK;
}
#endif /* ADAPTER_EIP164_INTERRUPTS_ENABLE */


/* end of file adapter_secy_support.c */
