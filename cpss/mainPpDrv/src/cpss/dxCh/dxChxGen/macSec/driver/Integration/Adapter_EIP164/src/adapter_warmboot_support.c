/* adapter_warmboot_support.c
 *
 * Top-level implementation of the WarmBoot API (Warmboot_Register) and
 * driver-internal API.
 */

/*****************************************************************************
* Copyright (c) 2017 INSIDE Secure B.V. All Rights Reserved.
*
* This confidential and proprietary software may be used only as authorized
* by a licensing agreement from INSIDE Secure.
*
* The entire notice above must be reproduced on all authorized copies that
* may only be made to the extent permitted by a licensing agreement from
* INSIDE Secure.
*
* For more information or support, please go to our online support system at
* https://customersupport.insidesecure.com.
* In case you do not have an account for this system, please send an e-mail
* to ESSEmbeddedHW-Support@insidesecure.com.
*****************************************************************************/

/*----------------------------------------------------------------------------
 * This module implements (provides) the following interface(s):
 */

/* WarmBoot API */
#include <Integration/Adapter_EIP164/incl/api_warmboot.h>

/* WarmBoot internal API */
#include <Integration/Adapter_EIP164/incl/adapter_warmboot_support.h>

/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

/* Default configuration */
#include <Integration/Adapter_EIP164/incl/c_adapter_warmboot.h>

/* Driver Framework Basic Definitions API */
#include <Kit/DriverFramework/incl/basic_defs.h>

#ifdef ADAPTER_WARMBOOT_ENABLE
/*----------------------------------------------------------------------------
 * Definitions and macros
 */
typedef struct
{
    WarmBoot_Alloc_Callback_t Alloc_CB;
    WarmBoot_Free_Callback_t Free_CB;
    WarmBoot_Write_Callback_t Write_CB;
    WarmBoot_Read_Callback_t Read_CB;
} WarmBoot_Callbacks_t;


/*----------------------------------------------------------------------------
 * Local variables
 */
static WarmBoot_Callbacks_t WarmBoot_Callbacks;


/*----------------------------------------------------------------------------
 * WarmBootLib_Alloc
 */
bool
WarmBootLib_Alloc(
        const unsigned int StorageByteCount,
        unsigned int * const AreaId_p)
{
    WarmBoot_Status_t rc;
    if(WarmBoot_Callbacks.Alloc_CB == NULL)
        return false;
    rc = WarmBoot_Callbacks.Alloc_CB(StorageByteCount, AreaId_p);
    return rc == WARMBOOT_STATUS_OK;
}


/*----------------------------------------------------------------------------
 * WarmBootLib_Free
 */
bool
WarmBootLib_Free(
        const unsigned int AreaId)
{
    WarmBoot_Status_t rc;
    if(WarmBoot_Callbacks.Free_CB == NULL)
        return true;
    rc = WarmBoot_Callbacks.Free_CB(AreaId);
    return rc == WARMBOOT_STATUS_OK;
}


/*----------------------------------------------------------------------------
 * WarmBootLib_Write
 */
bool
WarmBootLib_Write(
        const unsigned int AreaId,
        const unsigned char * const restrict  Data_p,
        const unsigned int ByteOffset,
        const unsigned int ByteCount)
{
    WarmBoot_Status_t rc;
    if(WarmBoot_Callbacks.Write_CB == NULL)
        return false;
    rc = WarmBoot_Callbacks.Write_CB(AreaId,Data_p,ByteOffset,ByteCount);
    return rc == WARMBOOT_STATUS_OK;
}


/*----------------------------------------------------------------------------
 * WarmBootLib_Read
 */
bool
WarmBootLib_Read(
        const unsigned int AreaId,
        unsigned char * const restrict Data_p,
        const unsigned int ByteOffset,
        const unsigned int ByteCount)
{
    WarmBoot_Status_t rc;
    if(WarmBoot_Callbacks.Read_CB == NULL)
        return false;
    rc = WarmBoot_Callbacks.Read_CB(AreaId,Data_p,ByteOffset,ByteCount);
    return rc == WARMBOOT_STATUS_OK;
}


/*----------------------------------------------------------------------------
 * WarmBoot_Register
 */
WarmBoot_Status_t
WarmBoot_Register(
        WarmBoot_Alloc_Callback_t Alloc_CB,
        WarmBoot_Free_Callback_t Free_CB,
        WarmBoot_Write_Callback_t Write_CB,
        WarmBoot_Read_Callback_t Read_CB)
{
    if (Alloc_CB == NULL ||
        Write_CB == NULL || Read_CB == NULL)
    {
        return WARMBOOT_ERROR_BAD_PARAMETER;
    }

    WarmBoot_Callbacks.Alloc_CB = Alloc_CB;
    WarmBoot_Callbacks.Free_CB = Free_CB;
    WarmBoot_Callbacks.Write_CB = Write_CB;
    WarmBoot_Callbacks.Read_CB = Read_CB;
    return WARMBOOT_STATUS_OK;
}

#endif /* ADAPTER_WARMBOOT_ENABLE */

/* end of file adapter_warmboot_support.c */
