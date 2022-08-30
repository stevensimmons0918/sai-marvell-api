/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
*/
/**
********************************************************************************
* @file prvCpssDxChMacSecGlobalDb.h
*
* @brief CPSS DxCh MAC Security (or MACsec) Global DB definitions.
*
* @version   1
********************************************************************************
*/

#ifndef __prvCpssDxChMacSecGlobalDbh
#define __prvCpssDxChMacSecGlobalDbh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/cpssTypes.h>
#include <Integration/DriverFramework/src/device_internal.h>
#include <Integration/Adapter_EIP163/incl/adapter_cfye_support.h>
#include <Integration/Adapter_EIP164/incl/adapter_secy_support.h>


/**
* @struct PRV_CPSS_DXCH_MACSEC_GLOBAL_DB_ADAPTER_INIT_STC
 *
 * @brief  Structure contain global variables that are meant to be shared
 * between processes (when CPSS is linked as shared object)
 *   in mainPpDrv module ,macSec driver directory, adapter_init.c file
*/
typedef struct
{
    /** flag to indicate status of MACSec adapter units */
    GT_BOOL Adapter_IsInitialized;

} PRV_CPSS_DXCH_MACSEC_GLOBAL_DB_ADAPTER_INIT_STC;


/**
* @struct PRV_CPSS_DXCH_MACSEC_GLOBAL_DB_HWPAL_DEVICE_UMDEVXS_STC
 *
 * @brief  Structure contain global variables that are meant to be shared
 * between processes (when CPSS is linked as shared object)
 *   in mainPpDrv module ,macSec driver directory, hwpal_device_umdevxs.c file
*/
typedef struct
{
    /** Global administration data */
    Device_Global_Admin_t HWPALLib_Device_Global;

    /** Table which includes pointers to DB of all supported MACSec units */
    Device_Admin_t * HWPALLib_Devices_p [HWPAL_DEVICE_STATIC_COUNT];

} PRV_CPSS_DXCH_MACSEC_GLOBAL_DB_HWPAL_DEVICE_UMDEVXS_STC;

/**
* @struct PRV_CPSS_DXCH_MACSEC_GLOBAL_DB_ADAPTER_CFYE_STC
 *
 * @brief  Structure contain global variables that are ment to be shared
 * between processes (when CPSS is linked as shared object)
 *   in mainPpDrv module ,macSec driver directory, adapter_cfye_support.c file
*/
typedef struct
{
    /** MACSec Classifier fields that have to remain available even if the device is not initialized */
    CfyE_Device_StaticFields_t CfyE_Device_StaticFields[ADAPTER_EIP163_MAX_NOF_DEVICES];

    /** Pointers to MACSec Classifier device data structure */
    CfyE_Device_Internal_t *CfyE_Devices[ADAPTER_EIP163_MAX_NOF_DEVICES];

} PRV_CPSS_DXCH_MACSEC_GLOBAL_DB_ADAPTER_CFYE_STC;

/**
* @struct PRV_CPSS_DXCH_MACSEC_GLOBAL_DB_ADAPTER_SECY_STC
 *
 * @brief  Structure contain global variables that are ment to be shared
 * between processes (when CPSS is linked as shared object)
 *   in mainPpDrv module ,macSec driver directory, adapter_secy_support.c file
*/
typedef struct
{
    /** MACSec Transformer fields that have to remain available even if the device is not initialized */
    SecY_Device_StaticFields_t SecY_Device_StaticFields[ADAPTER_EIP164_MAX_NOF_DEVICES];

    /** Pointers to MACSec Transformer device data structure */
    SecY_Device_t *SecY_Device[ADAPTER_EIP164_MAX_NOF_DEVICES];

} PRV_CPSS_DXCH_MACSEC_GLOBAL_DB_ADAPTER_SECY_STC;

/**
* @struct PRV_CPSS_DXCH_MACSEC_GLOBAL_DB_STC
 *
 * @brief  Structure contain global variables that are meant to be shared
 * between processes (when CPSS is linked as shared object)
 *   in mainPpDrv module ,macSec driver directory
*/
typedef struct
{
    /** data of adapter_init.c file */
    PRV_CPSS_DXCH_MACSEC_GLOBAL_DB_ADAPTER_INIT_STC adapterInitSrc;

    /** data of hwpal_device_umdevxs.c file */
    PRV_CPSS_DXCH_MACSEC_GLOBAL_DB_HWPAL_DEVICE_UMDEVXS_STC hwpalDeviceUmdevxsSrc;

    /** data of adapter_cfye_support.c file */
    PRV_CPSS_DXCH_MACSEC_GLOBAL_DB_ADAPTER_CFYE_STC adapterCfyeSupportSrc;

    /** data of adapter_secy_support.c file */
    PRV_CPSS_DXCH_MACSEC_GLOBAL_DB_ADAPTER_SECY_STC adapterSecySupportSrc;

} PRV_CPSS_DXCH_MACSEC_GLOBAL_DB_STC;


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvCpssDxChMacSecGlobalDbh */



