/*******************************************************************************
*              (c), Copyright 2018, Marvell International Ltd.                 *
* THIS CODE IS A REFERENCE CODE FOR MARVELL SWITCH PRODUCTS.  IT IS PROVIDED   *
* "AS IS" WITH NO WARRANTIES, EXPRESSED, IMPLIED OR OTHERWISE, REGARDING ITS   *
* ACCURACY, COMPLETENESS OR PERFORMANCE.                                       *
* CUSTOMERS ARE FREE TO MODIFY IT AND USE IT ONLY IN THEIR PRODUCTION          *
* SOFTWARE RELEASES WITH MARVELL SWITCH CHIPSETS.                              *
*******************************************************************************/
/**
********************************************************************************
* @file cpssApplicationBoardProfile.h
*
* @brief This file defines the board profile structure.
*
* @version   1
********************************************************************************
*/

#ifndef __CPSS_APP_PLATFORM_BOARD_PROFILE_H
#define __CPSS_APP_PLATFORM_BOARD_PROFILE_H

#ifdef CHX_FAMILY
#include <cpss/dxCh/dxChxGen/config/cpssDxChCfgInit.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortMapping.h>
#endif

#if 0
#include <gtExtDrv/drivers/gtPciDrv.h>
#endif

#define CPSS_APP_PLATFORM_MAX_BOARD_NAME_LEN_CNS   100
/*
 * @enum, CPSS_APP_PLATFORM_BOARD_PARAM_ENT
 * @brief, Enum identfies what kind of value CPSS_APP_PLATFORM_BOARD_PROFILE_STC
 *         is holding.
 */
typedef enum _CPSS_APP_PLATFORM_BOARD_PARAM_ENT {
    CPSS_APP_PLATFORM_BOARD_PARAM_GENERIC_E,
    CPSS_APP_PLATFORM_BOARD_PARAM_PP_MAP_E,
    CPSS_APP_PLATFORM_BOARD_PARAM_LED_E,
#ifdef CPSS_APP_PLATFORM_PHASE_2
    CPSS_APP_PLATFORM_BOARD_PARAM_CASCADE_E,
#endif
    CPSS_APP_PLATFORM_BOARD_PARAM_LAST_E
} CPSS_APP_PLATFORM_BOARD_PARAM_ENT;

/*
 *@enum, CPSS_APP_PLATFORM_OS_CALL_TYPE_ENT
 *@brief, OS call type used.
 */
typedef enum _CPSS_APP_PLATFORM_OS_CALL_TYPE_ENT {
    CPSS_APP_PLATFORM_OS_CALL_TYPE_STATIC_E,
    CPSS_APP_PLATFORM_OS_CALL_TYPE_SHARED_E,
    CPSS_APP_PLATFORM_OS_CALL_TYPE_EXTERNAL_E,
    CPSS_APP_PLATFORM_OS_CALL_TYPE_LAST_E
} CPSS_APP_PLATFORM_OS_CALL_TYPE_ENT;

/*
 * @enum, CPSS_APP_PLATFORM_EXT_DRV_CALL_TYPE_ENT
 * @brief, External driver call type used.
 */
typedef enum _CPSS_APP_PLATFORM_EXT_DRV_CALL_TYPE_ENT {
    CPSS_APP_PLATFORM_EXT_DRV_CALL_TYPE_STATIC_E,
    CPSS_APP_PLATFORM_EXT_DRV_CALL_TYPE_SHARED_E,
    CPSS_APP_PLATFORM_EXT_DRV_CALL_TYPE_EXTERNAL_E,
    CPSS_APP_PLATFORM_EXT_DRV_CALL_TYPE_LAST_E
} CPSS_APP_PLATFORM_EXT_DRV_CALL_TYPE_ENT;

/*
 * @struct, CPSS_APP_PLATFORM_BOARD_PARAM_GENERIC_STC
 * @brief, information about board.
 */
typedef struct _CPSS_APP_PLATFORM_BOARD_PARAM_GENERIC_STC
{

    GT_CHAR                                       boardName[CPSS_APP_PLATFORM_MAX_BOARD_NAME_LEN_CNS];
    CPSS_APP_PLATFORM_OS_CALL_TYPE_ENT            osCallType;
    CPSS_APP_PLATFORM_EXT_DRV_CALL_TYPE_ENT       extDrvCallType;

} CPSS_APP_PLATFORM_BOARD_PARAM_GENERIC_STC;

/*
 * @struct, CPSS_APP_PLATFORM_DEVICE_PCI_ADDRESS_STC
 * @brief, PCI address = bus + device + function.
 */
typedef struct _CPSS_APP_PLATFORM_DEVICE_PCI_ADDRESS_STC
{
    GT_U8       busId;
    GT_U8       deviceId;
    GT_U8       functionId;

} CPSS_APP_PLATFORM_DEVICE_PCI_ADDRESS_STC;

/*
 * @struct, CPSS_APP_PLATFORM_DEVICE_SMI_ADDRESS_STC
 * @brief, SMI address = bus + device.
 */
typedef struct _CPSS_APP_PLATFORM_DEVICE_SMI_ADDRESS_STC
{
    GT_U8       busId;
    GT_U8       deviceId;

} CPSS_APP_PLATFORM_DEVICE_SMI_ADDRESS_STC;

/*
 * @enum, CPSS_APP_PLATFORM_PP_MAP_TYPE_ENT
 * @brief, Map type - fixed/autoscan.
 */
typedef enum _CPSS_APP_PLATFORM_PP_MAP_TYPE_ENT {

   CPSS_APP_PLATFORM_PP_MAP_TYPE_FIXED_E,
   CPSS_APP_PLATFORM_PP_MAP_TYPE_AUTOSCAN_E,
   CPSS_APP_PLATFORM_PP_MAP_TYPE_LAST_E

} CPSS_APP_PLATFORM_PP_MAP_TYPE_ENT;

typedef union _CPSS_APP_PLATFORM_INF_ADDR_UNT {
   CPSS_APP_PLATFORM_DEVICE_PCI_ADDRESS_STC pciAddr;
   CPSS_APP_PLATFORM_DEVICE_SMI_ADDRESS_STC smiAddr;
} CPSS_APP_PLATFORM_INF_ADDR_UNT;

#ifdef CHX_FAMILY
/*
 * @struct CPSS_APP_PLATFORM_BOARD_PARAM_PP_MAP_STC.
 * @brief: Mapping of a PP (devNum) to Interface address
 */
typedef struct _CPSS_APP_PLATFORM_BOARD_PARAM_PP_MAP_STC {

   CPSS_APP_PLATFORM_PP_MAP_TYPE_ENT    mapType;
   CPSS_PP_INTERFACE_CHANNEL_ENT        mngInterfaceType;
   GT_U8                                devNum;
   CPSS_DXCH_PORT_MAP_STC              *portMap;
   GT_U8                                portMapSize;
   /* Valid only if mapType is FIXED */
   CPSS_APP_PLATFORM_INF_ADDR_UNT       mngInterfaceAddr;

} CPSS_APP_PLATFORM_BOARD_PARAM_PP_MAP_STC;

/*
 * @struct LED_PARAM_STC.
 * @brief: LED related settings.
 */
typedef struct _CPSS_APP_PLATFORM_BOARD_PARAM_LED_STC {

    GT_U32                          devNum; /*device ID of PP */
    CPSS_LED_CONF_STC               ledConf;
    CPSS_LED_CLASS_MANIPULATION_STC ledClass;
    GT_U32                          ledInterfaceNum;
} CPSS_APP_PLATFORM_BOARD_PARAM_LED_STC;
#else
/*dummy structures for PX*/
typedef struct _CPSS_APP_PLATFORM_BOARD_PARAM_PP_MAP_STC {

   GT_U8                                dummy;
} CPSS_APP_PLATFORM_BOARD_PARAM_PP_MAP_STC;

typedef struct _CPSS_APP_PLATFORM_BOARD_PARAM_LED_STC {

   GT_U32                          dummy;
} CPSS_APP_PLATFORM_BOARD_PARAM_LED_STC;
#endif

typedef union _CPSS_APP_PLATFORM_BOARD_PARAM_UNT {
        CPSS_APP_PLATFORM_BOARD_PARAM_GENERIC_STC  *boardPtr;
        CPSS_APP_PLATFORM_BOARD_PARAM_PP_MAP_STC   *ppMapPtr;
        CPSS_APP_PLATFORM_BOARD_PARAM_LED_STC      *ledInfoPtr;
        GT_VOID_PTR                                 lastPtr;
} CPSS_APP_PLATFORM_BOARD_PARAM_UNT;
/**
 *@struct : CPSS_APP_PLATFORM_BOARD_PROFILE_STC
 *@brief: Board profile can have information of board,
 *        ppMapping information, led and cascade port information
 **/
typedef struct _CPSS_APP_PLATFORM_BOARD_PROFILE_STC {
    CPSS_APP_PLATFORM_BOARD_PARAM_ENT boardInfoType;
    CPSS_APP_PLATFORM_BOARD_PARAM_UNT boardParam;
} CPSS_APP_PLATFORM_BOARD_PROFILE_STC;

#endif /* __CPSS_APP_PLATFORM_BOARD_PROFILE_H */
