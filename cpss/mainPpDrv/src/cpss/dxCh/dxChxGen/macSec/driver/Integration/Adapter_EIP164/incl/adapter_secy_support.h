/* adapter_secy_support.h
 *
 * Adapter Internal SecY API
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

#ifndef ADAPTER_SECY_SUPPORT_H_
#define ADAPTER_SECY_SUPPORT_H_


/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

/* SecY API */
#include <Integration/Adapter_EIP164/incl/api_secy.h>

/* SecY API, types */
#include <Integration/Adapter_EIP164/incl/api_secy_types.h>

/* EIP-164 Driver Library API, SecY types */
#include <Kit/EIP164/incl/eip164_types.h>

/* CLib API, memcmp */
#include <Kit/DriverFramework/incl/clib.h>      /* ZEROINIT() */

/* Adapter configuration. */
#include <Integration/Adapter_EIP164/incl/c_adapter_eip164.h>

/* List API */
#include <Kit/List/incl/list.h>

/* Adapter Lock API */
#include <Integration/Adapter_EIP164/incl/adapter_lock.h>       /* Adapter_Lock* */


/*----------------------------------------------------------------------------
 * Definitions and macros
 */

/* vPort, SC to SA mapping type */
typedef enum
{
    /* SA has been detached from its SC/vPort, either by direct */
    /* replacement or by chaining; or no SA record is used (bypass or drop). */
    SECY_SA_MAP_DETACHED,
    /* Egress mapping, SC mapped to single SA */
    SECY_SA_MAP_EGRESS,
    /* Egress mapping, SC mapped to single SA, for CRYPT_AUTH */
    SECY_SA_MAP_EGRESS_CRYPT_AUTH,
    /* Ingress mapping, RxCAM used, SC mapped to up to four SAs */
    SECY_SA_MAP_INGRESS,
    /* Ingress Crypt Auth, RxCAM usd, same SA is mapped four times to SC */
    SECY_SA_MAP_INGRESS_CRYPT_AUTH,
} SecY_SA_MapType_t;

/* SA descriptor */
typedef struct
{
    uint32_t    Magic;
    union {
        struct {
            unsigned int SAIndex;

            unsigned int SCIndex;

            unsigned int SAMFlowCtrlIndex;

            unsigned int AN;

            SecY_SA_MapType_t MapType;
        } InUse;

        List_Element_t free;
    } u;
} SecY_SA_Descriptor_t;


/* SC descriptor */
typedef struct
{
    SecY_SAHandle_t SAHandle[4];

    unsigned int SCIndex;

    uint8_t SCI[8];

    unsigned int vPort;

    SecY_SA_MapType_t MapType;

    uint32_t Magic;

    List_Element_t OnList; /* Either free list of per-vPort list. */
} SecY_SC_Descriptor_t;

/* vPort Descriptor */
typedef struct
{
    unsigned int SACount; /* Number of active SA records. */

    uint32_t MTU; /* MTU value for egress SA records associated with this vPort */
    void *SCList; /* List of inbound SC's associated with this vPort. */
} SecY_vPort_Descriptor_t;

/* SecY device data structure */
typedef struct
{
    void * SAFL_p; /* SA Free List instance pointer */

    SecY_SA_Descriptor_t * SADscr_p;

    void * SCFL_p; /* Secure Channel Free List instance pointer */

    SecY_SC_Descriptor_t *SCDscr_p;

    SecY_vPort_Descriptor_t *vPortDscr_p;

    unsigned char *vPortListHeads_p;

    unsigned int SACount;

    unsigned int SCCount;

    unsigned int ChannelCount;

    unsigned int vPortCount;

    SecY_Role_t Role;

#ifdef ADAPTER_EIP164_WARMBOOT_ENABLE
    unsigned int WarmBoot_AreaId;

    /* Bitmap for SAs */
    uint8_t *WarmBoot_SA_Bitmap;

#endif

    EIP164_IOArea_t IOArea;
} SecY_Device_t;

/* These fields have to stay around, even if the device is not */
/* initialized, so we can lock it and verify that it is initialized at */
/* all times. */
typedef struct
{
    const char * DeviceName_p;

    bool fInitialized;

    Adapter_Lock_Struct_t Lock;

    /* Device lock flags */
    unsigned long Flags;
} SecY_Device_StaticFields_t;

/* SecY notification */
typedef struct
{
    SecY_NotifyFunction_t NotifyCB_p;
    unsigned int EventMask;
    bool fGlobalAIC;
    unsigned int DeviceId;
} SecY_Notify_Internal_t;

#define ADAPTER_EIP164_DEVICE(name) {name, false, ADAPTER_LOCK_INITIALIZER, 0}

#define ADAPTER_EIP164_DEVICE_NAME(DeviceId) PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device_StaticFields)[DeviceId].DeviceName_p

#define ADAPTER_EIP164_SA_DSCR_MAGIC        0xFEEDBEEF

/* Convert error returned from driver library function to SECY return code.
   Codes defined by the EIP164 are converted to SECY_ERROR_INTERNAL while other
   codes are assumed to come from the driver framework API and are returned
   unchanged. Never call this macro with a zero argument (zero==success).
*/
#define SECY_DLIB_ERROR(n) ((unsigned)(n)>EIP164_HW_CONFIGURATION_MISMATCH?(n):SECY_ERROR_INTERNAL)



#ifdef ADAPTER_EIP164_INTERRUPTS_ENABLE
extern SecY_Notify_Internal_t * SecY_Notify[];
#endif


/*----------------------------------------------------------------------------
 * SecYLib_InterruptHandler
 *
 * This function is the interrupt handler for the SecY device interrupt
 * sources that indicate the occurrence of one or several of the requested events.
 * There may be several interrupt sources.
 *
 * This function is used to invoke the SecY notification callback.
 *
 * nIRQ (input)
 *      Interrupt number.
 *
 * Flags (input)
 *      Bit mask indicating the active interrupts.
 */
void
SecYLib_InterruptHandler(
        const int nIRQ,
        const unsigned int Flags);


/*----------------------------------------------------------------------------
 * SecYLib_ICDeviceID_Get
 *
 * Return the index of the interrupt controller device for the
 * specified channel.
 *
 * DeviceId (input)
 *      ID of the selected device.
 *
 * ChannelId_p (input)
 *      Pointer to the channel number for a channel interrupt controller.
 *      NULL for the global interrupt controller.
 *
 * Return: index of interrupt controller device.
 */
unsigned int
SecYLib_ICDeviceID_Get(
        const unsigned int DeviceId,
        const unsigned int * const ChannelId_p);


/*----------------------------------------------------------------------------
 * SecYLib_Device_Lock
 *
 * Lock a SecY device, ensuring exclusive access to it.
 *
 * DeviceId (input)
 *      ID of the selected device.
 */
void
SecYLib_Device_Lock(
        const unsigned int DeviceId);


/*----------------------------------------------------------------------------
 * SecYLib_Device_Unlock
 *
 * Unlock a SecY device.
 *
 * DeviceId (input)
 *      ID of the selected device.
 */
void
SecYLib_Device_Unlock(
        const unsigned int DeviceId);


/*----------------------------------------------------------------------------
 * SecYLib_Device_Sync
 *
 * Ensure that all packets submitted to the device at the time of the call
 * are processed.
 *
 * DeviceId (input)
 *      ID of the selected device.
 */
SecY_Status_t
SecYLib_Device_Sync(
        const unsigned int DeviceId);


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
        const SecY_SAHandle_t * const Handle2_p);


/*----------------------------------------------------------------------------
 * SecYLib_SAHandleToIndex
 *
 * Return SA index (and optionally the SC index and SAMFlowCtrlIndex) associated
 * with an SA Handle.
 *
 * SAHandle (input)
 *     SA Handle
 *
 * SAIndex_p (output)
 *     SA Index.
 *
 * SCIndex_p (output)
 *     SC Index, may be NULL if SC index is not needed.
 *
 * SAMFlowCtrlIndex_p (output)
 *     SAMFlowCtrlIndex  (is vPort index), may be NULL if this output is not
 *     needed.
 *
 * Return: true if SA Handle is valid, false otherwise.
 */
bool
SecYLib_SAHandleToIndex(
        const SecY_SAHandle_t SAHandle,
        unsigned int * const SAIndex_p,
        unsigned int * const SCIndex_p,
        unsigned int * const SAMFlowCtrlIndex_p);


/*----------------------------------------------------------------------------
 * SecYLib_SAIndexToHandle
 *
 * Get the SA handle in the device for this SA index
 *
 * DeviceId (input)
 *      ID of the selected device.
 *
 * SAIndex (input)
 *      SA Index.
 *
 * SAHandle_p (output)
 *      SA Handle.
 *
 * Return: true if SA index is valid for the device, false otherwise.
 */
bool
SecYLib_SAIndexToHandle(
        const unsigned int DeviceId,
        const unsigned int SAIndex,
        SecY_SAHandle_t * const SAHandle_p);


/*----------------------------------------------------------------------------
 * SecYLib_SA_Chained_Get
 *
 * Find any chained SA for an outbound SA.
 *
 * DeviceId (input)
 *      ID of the selected device.
 *
 * SAIndex (input)
 *      SA index for which to find a chained SA.
 *
 * NewSAIndex (output)
 *      SA index of the chained SA (if any).
 *
 * fSaInUse (output)
 *      true if a chained SA was found, false otherwise.
 *
 */
SecY_Status_t
SecYLib_SA_Chained_Get(
        const unsigned int DeviceId,
        const unsigned int SAIndex,
        unsigned int * const NewSAIndex,
        bool * const fSAInUse);


/*----------------------------------------------------------------------------
 * SecYLib_SA_Read
 *
 * Read (port of) an SA record.
 *
 * DeviceId (input)
 *      ID of the selected device.
 *
 * SAIndex (input)
 *      SA index of the record to read.
 *
 * WordOffset (input)
 *      Start offset within the record to read from.
 *
 * WordCount (input)
 *      Number of (32-bit) words to read.
 *
 * Transform_p (output)
 *      Pointer to area where contents of SA record must be stored.
 */
SecY_Status_t
SecYLib_SA_Read(
        const unsigned int DeviceId,
        const unsigned int SAIndex,
        const unsigned int WordOffset,
        const unsigned int WordCount,
        uint32_t * Transform_p);


/*----------------------------------------------------------------------------
 * SecYLib_vPort_Statistics_Clear
 *
 * Clear the SecY and IFC statistics counters belonging to a given vPort.
 *
 * DeviceId (input)
 *      Device identifier of the SecY device to be used.
 *
 * vPort (input)
 *      vPort number for which to clear the statistics/
 *
 * Return value:
 *     SECY_STATUS_OK : success
 *     SECY_ERROR_BAD_PARAMETER : incorrect input parameter
 *     SECY_INTERNAL_ERROR : failure
 */
SecY_Status_t
SecYLib_vPort_Statistics_Clear(
        unsigned int DeviceId,
        unsigned int vPort);


#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>
#include <cpss/dxCh/dxChxGen/macSec/private/prvCpssDxChMacSecGlobalDb.h>

/*global variables macros*/
#define PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_SET(_var,_value)\
    ((PRV_CPSS_DXCH_MACSEC_GLOBAL_DB_STC *)PRV_SHARED_GLOBAL_VAR_GET(mainPpDrvMod.macSecDriverDir.dbPtr))->adapterSecySupportSrc._var = _value

#define PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(_var)\
    (((PRV_CPSS_DXCH_MACSEC_GLOBAL_DB_STC *)PRV_SHARED_GLOBAL_VAR_GET(mainPpDrvMod.macSecDriverDir.dbPtr))->adapterSecySupportSrc._var)


#endif /* ADAPTER_SECY_SUPPORT_H_ */


/* end of file adapter_secy_support.h */
