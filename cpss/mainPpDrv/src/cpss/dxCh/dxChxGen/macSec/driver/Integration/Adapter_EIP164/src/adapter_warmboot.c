/* adapter_warmboot.c
 *
 * Top-level implementation of the WarmBoot API,
 * Warmboot_MaxSize_Get, WarmBoot_Shutdown and WarmBoot_Restore functions.
 */

/*****************************************************************************
* Copyright (c) 2016-2019 INSIDE Secure B.V. All Rights Reserved.
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


/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

/* Default configuration */
#include <Integration/Adapter_EIP164/incl/c_adapter_warmboot.h>

/* Driver Framework Basic Definitions API */
#include <Kit/DriverFramework/incl/basic_defs.h>

/* WarmBoot internal API */
#include <Integration/Adapter_EIP164/incl/adapter_warmboot_support.h>
#include <Integration/Adapter_EIP164/incl/adapter_init_warmboot.h>
#include <Integration/Adapter_EIP164/incl/adapter_secy_warmboot.h>
#ifdef ADAPTER_WARMBOOT_CFYE_ENABLE
#include <Integration/Adapter_EIP163/incl/adapter_cfye_warmboot.h>
#endif

#ifdef ADAPTER_WARMBOOT_ENABLE


/* Convert error returned from driver library function to WARMBOOT
   return code.  Not-driver error codes (1..3) are converted to
   WARMBOOT_INTERNAL_ERROR while other codes are assumed to come from
   the driver framework API and are returned unchanged. Never call
   this macro with a zero argument (zero==success).
*/
#define WARMBOOT_DLIB_ERROR(n) ((unsigned)(n)>3?(n):WARMBOOT_INTERNAL_ERROR)



/*----------------------------------------------------------------------------
 * WarmBoot_MaxSize_Get()
 */
WarmBoot_Status_t
WarmBoot_MaxSize_Get(
        const WarmBoot_DeviceType_t Type,
        unsigned int * const restrict MaxByteCount_p)
{
    bool fSuccess;
    if (MaxByteCount_p == NULL)
    {
        return WARMBOOT_ERROR_BAD_PARAMETER;
    }

    switch(Type)
    {
    case WARMBOOT_DEVICE_TYPE_SECY_E:
        fSuccess = SecYLib_WarmBoot_MaxSize_Get(0, false, MaxByteCount_p);
        break;
    case WARMBOOT_DEVICE_TYPE_SECY_I:
        fSuccess = SecYLib_WarmBoot_MaxSize_Get(0, true, MaxByteCount_p);
        break;
#ifdef ADAPTER_WARMBOOT_CFYE_ENABLE
    case WARMBOOT_DEVICE_TYPE_CFYE_E:
        fSuccess = CfyELib_WarmBoot_MaxSize_Get(0, false, MaxByteCount_p);
        break;
    case WARMBOOT_DEVICE_TYPE_CFYE_I:
        fSuccess = CfyELib_WarmBoot_MaxSize_Get(0, true, MaxByteCount_p);
        break;
#endif
    case WARMBOOT_DEVICE_TYPE_DF:
        fSuccess = Adapter_Init_WarmBoot_MaxSize_Get(MaxByteCount_p);
        break;
    default:
        return WARMBOOT_ERROR_BAD_PARAMETER;
    }

    if (fSuccess)
    {
        return WARMBOOT_STATUS_OK;
    }
    else
    {
        return WARMBOOT_INTERNAL_ERROR;
    }
}


/*----------------------------------------------------------------------------
 * WarmBoot_Shutdown()
 */
WarmBoot_Status_t
WarmBoot_Shutdown(
        const unsigned int AreaId)
{
    uint8_t DeviceInfo[2];
    uint16_t DeviceId;
    uint8_t FlagByte;
    int rc;

    /* Obtain the type byte */
    if (!WarmBootLib_Read(AreaId, DeviceInfo, 0, 2))
        return WARMBOOT_INTERNAL_ERROR;
    FlagByte = DeviceInfo[0];
    DeviceId = ((DeviceInfo[0] & MASK_2_BITS) << 8) | DeviceInfo[1];

    if ((FlagByte & (BIT_7|BIT_6)) == BIT_6) /* SecY device. */
    {
        rc = SecYLib_WarmBoot_Shutdown(DeviceId, AreaId);
        if (rc)
        {
            return WARMBOOT_DLIB_ERROR(rc);
        }
    }

#ifdef ADAPTER_WARMBOOT_CFYE_ENABLE
    if ((FlagByte & (BIT_7|BIT_6)) == BIT_7) /* CfyE device. */
    {
        rc = CfyELib_WarmBoot_Shutdown(DeviceId, AreaId);
        if (rc)
        {
            return WARMBOOT_DLIB_ERROR(rc);
        }
    }
#endif

    if ((FlagByte & (BIT_7|BIT_6)) == (BIT_7|BIT_6)) /* DF area. */
    {
        rc = Adapter_Init_WarmBoot_Shutdown(AreaId);
        if (rc)
        {
            return WARMBOOT_DLIB_ERROR(rc);
        }
    }

    return WARMBOOT_STATUS_OK;
}


/*----------------------------------------------------------------------------
 * WarmBoot_Restore
 */
WarmBoot_Status_t
WarmBoot_Restore(
        const unsigned int AreaId)
{
    uint8_t DeviceInfo[2];
    uint16_t DeviceId;
    uint8_t FlagByte;
    int rc;

    /* Obtain the type byte */
    if (!WarmBootLib_Read(AreaId, DeviceInfo, 0, 2))
        return WARMBOOT_INTERNAL_ERROR;
    FlagByte = DeviceInfo[0];
    DeviceId = ((DeviceInfo[0] & MASK_2_BITS) << 8) | DeviceInfo[1];

    if ((FlagByte & (BIT_7|BIT_6)) == BIT_6) /* SecY device. */
    {
        rc = SecYLib_WarmBoot_Restore(DeviceId, AreaId);
        if (rc)
        {
            return WARMBOOT_DLIB_ERROR(rc);
        }
    }

#ifdef ADAPTER_WARMBOOT_CFYE_ENABLE
    if ((FlagByte & (BIT_7|BIT_6)) == BIT_7) /* CfyE device. */
    {
        rc = CfyELib_WarmBoot_Restore(DeviceId, AreaId);
        if (rc)
        {
            return WARMBOOT_DLIB_ERROR(rc);
        }
    }
#endif

    if ((FlagByte & (BIT_7|BIT_6)) == (BIT_7|BIT_6)) /* DF area. */
    {
        rc = Adapter_Init_WarmBoot_Restore(AreaId);
        if (rc)
        {
            return WARMBOOT_DLIB_ERROR(rc);
        }
    }

    return WARMBOOT_STATUS_OK;
}

#endif /* ADAPTER_WARMBOOT_ENABLE */

/* end of file adapter_warmboot.c */
