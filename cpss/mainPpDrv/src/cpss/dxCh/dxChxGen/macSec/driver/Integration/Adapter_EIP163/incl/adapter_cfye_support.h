/* adapter_cfye_support.h
 *
 * Internal CfyE API
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

#ifndef ADAPTER_CFYE_SUPPORT_H_
#define ADAPTER_CFYE_SUPPORT_H_

/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

/* CfyE API */
#include <Integration/Adapter_EIP163/incl/api_cfye.h>

/* EIP-163 Driver Library API */
#include <Kit/EIP163/incl/eip163.h>

/* Adapter configuration. */
#include <Integration/Adapter_EIP163/incl/c_adapter_eip163.h>

/* Basic defs API, uint32_t, uint8_t */
#include <Kit/DriverFramework/incl/basic_defs.h>

/* List API */
#include <Kit/List/incl/list.h>

/* Adapter Lock API */
#include <Integration/Adapter_EIP164/incl/adapter_lock.h>       /* Adapter_Lock* */


/*----------------------------------------------------------------------------
 * Definitions and macros
 */

/* Magic number used as a vPort descriptor marker for debugging purposes */
#define ADAPTER_EIP163_VPORT_DSCR_MAGIC          0xFEEDBEE1

/* Magic number used as a rule descriptor marker for debugging purposes */
#define ADAPTER_EIP163_RULE_DSCR_MAGIC           0xFEEDBEE2

/* vPort descriptor */
typedef struct
{
    uint32_t Magic;
    union {
        struct {
            unsigned int vPortPolicyId;

            /** Mode of operation */
            CfyE_Channel_Mode_t ChannelMode;

            /* Number of rules associated with this vPortPolicyId */
            unsigned int BoundRulesCount;
        } InUse;
        List_Element_t free;
    } u;
} CfyE_vPort_Descriptor_Internal_t;

/* Rule descriptor */
typedef struct
{
    uint32_t Magic;
    union {
        struct {
            unsigned int RuleId;

            /* Pointer to the matching vPort for this rule */
            CfyE_vPort_Descriptor_Internal_t * vPortDscr_p;
        } InUse;
        List_Element_t free;
    } u;
} CfyE_Rule_Descriptor_Internal_t;

/* CfyE device data structure */
typedef struct
{
    CfyE_Role_t Role;

    unsigned int ChannelsCount; /* Max number of supported channels */
    unsigned int RulesCount;    /* Max number of supported rules */
    unsigned int vPortCount;    /* Max number of supported vPorts */

    /* vPort free list instance */
    void * vPortFreeList_p;

    /* Rule free list instance */
    void * RuleFreeList_p;

    /* Pointer to an array of vPort descriptors */
    CfyE_vPort_Descriptor_Internal_t * vPortDscr_p;

    /* Pointer to an array of Rule descriptors */
    CfyE_Rule_Descriptor_Internal_t * RuleDscr_p;

#ifdef ADAPTER_EIP163_WARMBOOT_ENABLE
    /* WarmBoot Area. */
    unsigned int WarmBoot_AreaId;

    /* Bitmap for allocated vPorts */
    unsigned char *WarmBoot_vPort_Bitmap;

    /* Bitmap for allocated Rules */
    unsigned char *WarmBoot_Rule_Bitmap;
#endif

    /* IO Area for EIP163 device */
    EIP163_IOArea_t IOArea;

    bool fExternalTCAM;
} CfyE_Device_Internal_t;

/* Static fields that have to remain available even if the device is */
/* not initialized. */
typedef struct
{
    const char * DeviceName_p;

    bool fInitialized;

    Adapter_Lock_Struct_t Lock;

    /* Device lock flags */
    unsigned long Flags;

} CfyE_Device_StaticFields_t;

/* CfyE notification */
typedef struct
{
    CfyE_NotifyFunction_t NotifyCB_p;
    unsigned int EventMask;
    bool fGlobalAIC;
} CfyE_Notify_Internal_t;

#define ADAPTER_EIP163_DEVICE(name) {name, false, ADAPTER_LOCK_INITIALIZER, 0}

#ifdef ADAPTER_EIP163_STRICT_ARGS
#define ADAPTER_EIP163_CHECK_POINTER(_p) \
    if (NULL == (_p)) \
        return CFYE_ERROR_BAD_PARAMETER;
#define ADAPTER_EIP163_CHECK_INT_INRANGE(_i, _min, _max) \
    if ((_i) < (_min) || (_i) > (_max)) \
        return CFYE_ERROR_BAD_PARAMETER;
#define ADAPTER_EIP163_CHECK_INT_ATLEAST(_i, _min) \
    if ((_i) < (_min)) \
        return CFYE_ERROR_BAD_PARAMETER;
#define ADAPTER_EIP163_CHECK_INT_ATMOST(_i, _max) \
    if ((_i) > (_max)) \
        return CFYE_ERROR_BAD_PARAMETER;
#define ADAPTER_EIP163_CHECK_INT_EQUAL(_i, _val) \
    if ((_i) != (_val)) \
        return CFYE_ERROR_BAD_PARAMETER;
#else /* ADAPTER_EIP163_STRICT_ARGS undefined */
#define ADAPTER_EIP163_CHECK_POINTER(_p)
#define ADAPTER_EIP163_CHECK_INT_INRANGE(_i, _min, _max)
#define ADAPTER_EIP163_CHECK_INT_ATLEAST(_i, _min)
#define ADAPTER_EIP163_CHECK_INT_ATMOST(_i, _max)
#define ADAPTER_EIP163_CHECK_INT_EQUAL(_i, _val)
#endif /* end of ADAPTER_EIP163_STRICT_ARGS */

#define ADAPTER_EIP163_DEVICE_NAME(DeviceId) PRV_SHARED_MACSEC_CFYE_GLOBAL_VAR_GET(CfyE_Device_StaticFields)[DeviceId].DeviceName_p


extern CfyE_Device_Internal_t * CfyE_Devices[];

#ifdef ADAPTER_EIP163_INTERRUPTS_ENABLE
extern CfyE_Notify_Internal_t * CfyE_Notify[];
#endif

/* Convert error returned from driver library function to CFYE return code.
   Codes defined by the EIP163 are converted to CFYE_ERROR_INTERNAL while other
   codes are assumed to come from the driver framework API and are returned
   unchanged. Never call this macro with a zero argument (zero==success).
*/
#define CFYE_DLIB_ERROR(n) ((unsigned)(n)>EIP163_HW_CONFIGURATION_MISMATCH?(n):CFYE_ERROR_INTERNAL)


/*----------------------------------------------------------------------------
 * CfyELib_InterruptHandler
 *
 * This function is the interrupt handler for the CfyE device
 * interrupt sources that indicate the occurrence of one or several of
 * the requested events.  There may be several interrupt sources.
 *
 * This function is used to invoke the CfyE notification callback.
 *
 * nIRQ (input)
 *      Interrupt number.
 *
 * Flags (input)
 *      Bit mask indicating the active interrupts.
 */
void
CfyELib_InterruptHandler(
        const int nIRQ,
        const unsigned int Flags);


/*----------------------------------------------------------------------------
 * CfyELib_ICDeviceID_Get
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
CfyELib_ICDeviceID_Get(
        const unsigned int DeviceId,
        const unsigned int * const ChannelId_p);


/*----------------------------------------------------------------------------
 * CfyELib_Initialized_Check
 *
 * Check whether the device is or is not initialized.
 *
 * DeviceId (input)
 *      ID of the selected device.
 *
 * FuncName (input)
 *      Name of the function to report an error.
 *
 * fExpectedReturn (input)
 *      true if the device is expected to be initialized, false otherwise.
 *
 * Return: true if the device is or is not initialized according to the
 *         expectation, false otherwise.
 */
bool
CfyELib_Initialized_Check(
        const unsigned int DeviceId,
        const char * FuncName,
        const bool fExpectedReturn);

/*----------------------------------------------------------------------------
 * CfyELib_vPortFreeList_Uninit
 *
 * Release any memory resources used for the vPort records.
 *
 * DeviceId (input)
 *      ID of the selected device.
 */
void
CfyELib_vPortFreeList_Uninit(
        const unsigned int DeviceId);


/*----------------------------------------------------------------------------
 * CfyELib_RuleFreeList_Uninit
 *
 * Release any memory resources used for the Rule records.
 *
 * DeviceId (input)
 *      ID of the selected device.
 */
void
CfyELib_RuleFreeList_Uninit(
        const unsigned int DeviceId);


/*----------------------------------------------------------------------------
 * CfyELib_Device_Lock
 *
 * Lock a CfyE device, ensuring exclusive access to it.
 *
 * DeviceId (input)
 *      ID of the selected device.
 */
void
CfyELib_Device_Lock(
        const unsigned int DeviceId);


/*----------------------------------------------------------------------------
 * CfyELib_Device_Unlock
 *
 * Unlock a CfyE device.
 *
 * DeviceId (input)
 *      ID of the selected device.
 */
void
CfyELib_Device_Unlock(
        const unsigned int DeviceId);


/*----------------------------------------------------------------------------
 * CfyELib_Device_Sync
 *
 * Ensure that all packets submitted to the device at the time of the call
 * are processed.
 *
 * DeviceId (input)
 *      ID of the selected device.
 */
CfyE_Status_t
CfyELib_Device_Sync(
        const unsigned int DeviceId);


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
bool
CfyELib_vPortHandle_IsSame(
        const CfyE_vPortHandle_t Handle1,
        const CfyE_vPortHandle_t Handle2);


/*----------------------------------------------------------------------------
 * CfyELib_vPortHandle_IsValid
 *
 * Check if the given vPort is valid.
 *
 * vPortHandle (input)
 *     vPort handle of the vPort to check.
 *
 * Return: true if vPort is valid, false otherwise.
 */
bool
CfyELib_vPortHandle_IsValid(
        const CfyE_vPortHandle_t vPortHandle);

/*----------------------------------------------------------------------------
 * CfyELib_vPortId_Get
 *
 * Get the vPort index in the device for this vPort handle
 *
 * vPortHandle (input)
 *     vPort Handle
 *
 * Return vPort index.
 */
unsigned int
CfyELib_vPortId_Get(
        const CfyE_vPortHandle_t vPortHandle);


/*----------------------------------------------------------------------------
 * CfyELib_vPortIndexToHandle
 *
 * Get the vPort handle in the device for this vPort index
 *
 * DeviceId (input)
 *      ID of the selected device.
 *
 * vPortIndex (input)
 *      vPort index.
 *
 * vPortHandle_p (output)
 *      Location where the handle will be stored.
 *
 * Return true if the vPort is valid, false otherwise.
 */
bool
CfyELib_vPortIndexToHandle(
        const unsigned int DeviceId,
        const unsigned int vPortIndex,
        CfyE_vPortHandle_t * const vPortHandle_p);


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
bool
CfyELib_RuleHandle_IsSame(
        const CfyE_RuleHandle_t Handle1,
        const CfyE_RuleHandle_t Handle2);

/*----------------------------------------------------------------------------
 * CfyELib_RuleHandle_IsValid
 *
 * Check if the given rule is valid.
 *
 * RuleHandle (input)
 *     Rule handle of the rule to check.
 *
 * Return: true if rule is valid, false otherwise.
 */
bool
CfyELib_RuleHandle_IsValid(
        const CfyE_RuleHandle_t RuleHandle);


/*----------------------------------------------------------------------------
 * CfyELib_RuleId_Get
 *
 * Get the rule index in the device for this rule handle
 *
 * RuleHandle (input)
 *     Rule Handle
 *
 * Return Rule index.
 */
unsigned int
CfyELib_RuleId_Get(
        const CfyE_RuleHandle_t RuleHandle);


/*----------------------------------------------------------------------------
 * CfyELib_RuleIndexToHandle
 *
 * Get the Rule handle in the device for this Rule index
 *
 * DeviceId (input)
 *      ID of the selected device.
 *
 * RuleIndex (input)
 *      Rule index.
 *
 * RuleHandle_p (output)
 *      Location where the handle will be stored.
 *
 * Return true if the Rule is valid, false otherwise.
 */
bool
CfyELib_RuleIndexToHandle(
        const unsigned int DeviceId,
        const unsigned int RuleIndex,
        CfyE_RuleHandle_t * const RuleHandle_p);



#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>
#include <cpss/dxCh/dxChxGen/macSec/private/prvCpssDxChMacSecGlobalDb.h>

/*global variables macros*/
#define PRV_SHARED_MACSEC_CFYE_GLOBAL_VAR_SET(_var,_value)\
    ((PRV_CPSS_DXCH_MACSEC_GLOBAL_DB_STC *)PRV_SHARED_GLOBAL_VAR_GET(mainPpDrvMod.macSecDriverDir.dbPtr))->adapterCfyeSupportSrc._var = _value

#define PRV_SHARED_MACSEC_CFYE_GLOBAL_VAR_GET(_var)\
    (((PRV_CPSS_DXCH_MACSEC_GLOBAL_DB_STC *)PRV_SHARED_GLOBAL_VAR_GET(mainPpDrvMod.macSecDriverDir.dbPtr))->adapterCfyeSupportSrc._var)


#endif /* ADAPTER_CFYE_SUPPORT_H_ */

/* end of file adapter_cfye_support.h */
