/* adapter_cfye_warmboot.h
 *
 * CfyE WarmBoot Interface.
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

#ifndef INCLUDE_GUARD_ADAPTER_CFYE_WARMBOOT_H
#define INCLUDE_GUARD_ADAPTER_CFYE_WARMBOOT_H

/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

/* Driver Framework Basic Definitions API */
#include <Kit/DriverFramework/incl/basic_defs.h>  /* bool */

/*----------------------------------------------------------------------------
 * Definitions and macros
 */


/*----------------------------------------------------------------------------
 * CfyELib_WarmBoot_vPort_Set
 *
 * Mark vPort as allocated for the purpose of the WarmBoot administration.
 *
 * DeviceId (input)
 *      ID of the selected device.
 *
 * vPortIndex (input)
 *      vPort index of the record to mark as allocated.
 */
void
CfyELib_WarmBoot_vPort_Set(
        unsigned int DeviceId,
        unsigned int vPortIndex);


/*----------------------------------------------------------------------------
 * CfyELib_WarmBoot_vPort_Clear
 *
 * Mark vPort as free for the purpose of the WarmBoot administration..
 *
 * DeviceId (input)
 *      ID of the selected device.
 *
 * vPortIndex (input)
 *      vPort index of the record to mark as free.
 */
void
CfyELib_WarmBoot_vPort_Clear(
        unsigned int DeviceId,
        unsigned int vPortIndex);


/*----------------------------------------------------------------------------
 * CfyELib_WarmBoot_Rule_Set
 *
 * Mark Rule as allocated for the purpose of the WarmBoot administration.
 *
 * DeviceId (input)
 *      ID of the selected device.
 *
 * RuleIndex (input)
 *      Rule index of the record to mark as allocated.
 */
void
CfyELib_WarmBoot_Rule_Set(
        unsigned int DeviceId,
        unsigned int RuleIndex);


/*----------------------------------------------------------------------------
 * CfyELib_WarmBoot_Rule_Clear
 *
 * Mark Rule as free for the purpose of the WarmBoot administration..
 *
 * DeviceId (input)
 *      ID of the selected device.
 *
 * RuleIndex (input)
 *      Rule index of the record to mark as free.
 */
void
CfyELib_WarmBoot_Rule_Clear(
        unsigned int DeviceId,
        unsigned int RuleIndex);


/*----------------------------------------------------------------------------
 * CfyELib_WarmBoot_MaxSize_Get
 *
 * Determine the maximum size required for a WarmBoot area when the device
 * is initialized.
 *
 * DeviceId (input)
 *      ID of the selected device.
 *
 * fIngress (input)
 *      true if the device is for ingress.
 *
 * MaxByteCount_p (output)
 *      Maximum size of the WarmBoot area for that device.
 *
 * Return value:
 *     true: success.
 *     false: failure
 */
bool
CfyELib_WarmBoot_MaxSize_Get(
        const unsigned int DeviceId,
        const bool fIngress,
        unsigned int *MaxByteCount_p);


/*-----------------------------------------------------------------------------
 * CfyELib_WarmBoot_Device_Init
 *
 * Initialize the WarmBoot administration when a device is initialized.
 *
 * DeviceId (input)
 *      ID of the selected device.
 *
 * Return values:
 *     true: success
 *     false: failure
 */
bool
CfyELib_WarmBoot_Device_init(
        unsigned int DeviceId);


/*-----------------------------------------------------------------------------
 * CfyELib_WarmBoot_Device_Uninit
 *
 * Free the WarmBoot administration if a device is uninitialized.
 *
 * DeviceId (input)
 *      ID of the selected device.
 *
 */
void
CfyELib_WarmBoot_Device_Uninit(
        unsigned int DeviceId);


/*----------------------------------------------------------------------------
 * CfyELib_WarmBoot_Shutdown().
 *
 * CfyE API specific function for WarmBoot Shutdown.
 *
 * DeviceId (input)
 *     Device Id of the device to be shut down.
 *
 * AreaId (input)
 *     Id of the storage area where device info is stored.
 *
 * Return value:
 *    0: success.
 *    nonzero value: failure.
 */
int
CfyELib_WarmBoot_Shutdown(
        unsigned int DeviceId,
        unsigned int AreaId);


/*----------------------------------------------------------------------------
 * CfyELib_WarmBoot_Restore().
 *
 * CfyE API specific function for WarmBoot Restore.
 *
 * DeviceId (input)
 *     Device Id of the device to be restored.
 *
 * AreaId (input)
 *     Id of the storage area where device info is stored.
 *
 * Return value:
 *    0: success.
 *    nonzero value: failure.
 */
int
CfyELib_WarmBoot_Restore(
        unsigned int DeviceId,
        unsigned int AreaId);


#endif /* INCLUDE_GUARD_ADAPTER_CFYE_WARMBOOT_H */


/* end of file adapter_cfye_warmboot.h */
