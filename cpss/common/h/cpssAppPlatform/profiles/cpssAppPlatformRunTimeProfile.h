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
* @file cpssAppPlatformRunTimeProfile.h
*
* @brief This file contains RunTime profile definitions.
*
* @version   1
********************************************************************************
*/

#ifndef __CPSS_APP_PLATFORM_RUN_TIME_PROFILE_H
#define __CPSS_APP_PLATFORM_RUN_TIME_PROFILE_H

#include <cpssAppPlatformExtPhyConfig.h>

#define CPSS_APP_PLATFORM_PORT_LIST_PORTS_MAX           64

/* Event Callback function type */
typedef GT_STATUS (*CPSS_APP_PLATFORM_USER_EVENT_CB_FUNC)
(
    GT_U8  devNum,
    GT_U32 uniEv,
    GT_U32 evExtData
);

typedef GT_STATUS (*CPSS_APP_PLATFORM_PORT_MANAGER_FUNC)
(
    GT_VOID
);

/**
* @enum CPSS_APP_PLATFORM_RUNTIME_PARAM_TYPE_ENT
*
* @brief types of runtime profile values.
*
**/
typedef enum _CPSS_APP_PLATFORM_RUNTIME_PARAM_TYPE_ENT
{
    CPSS_APP_PLATFORM_RUNTIME_PARAM_EVENT_HANDLE_E,
    CPSS_APP_PLATFORM_RUNTIME_PARAM_PORT_MANAGER_HANDLE_E,
    CPSS_APP_PLATFORM_RUNTIME_PARAM_TRAFFIC_ENABLE_E,
    CPSS_APP_PLATFORM_RUNTIME_PARAM_LAST_E = 0xFF
} CPSS_APP_PLATFORM_RUNTIME_PARAM_TYPE_ENT;

/**
* @struct CPSS_APP_PLATFORM_EVENT_HANDLE_STC
*
* @brief Event Handle structure definition.
*
**/
typedef struct _CPSS_APP_PLATFORM_EVENT_HANDLE_STC
{
    /* Pointer to list of CPSS user events */
    CPSS_UNI_EV_CAUSE_ENT               *eventListPtr;
    /* Number of events in eventList */
    GT_U8                                numOfEvents;
    /* Event Callback function Ptr*/
    CPSS_APP_PLATFORM_USER_EVENT_CB_FUNC callbackFuncPtr;
    /* Event Handler Task priority */
    GT_U32                               taskPriority;

} CPSS_APP_PLATFORM_EVENT_HANDLE_STC;

/**
* @struct CPSS_APP_PLATFORM_PORT_MANAGER_HANDLE_STC
*
* @brief Port manager handler.
*
**/
typedef struct _CPSS_APP_PLATFORM_PORT_MANAGER_HANDLE_STC
{
    /* Port Manager Task priority */
    GT_U32                              taskPriority;
    /* Port Manager callback function */
    CPSS_APP_PLATFORM_PORT_MANAGER_FUNC portManagerFunc;

} CPSS_APP_PLATFORM_PORT_MANAGER_HANDLE_STC;

/**
* @struct _CPSS_APP_PLATFORM_PORT_CONFIG_STC
*
* @brief Port config parameters:
*         interface mode and speed for entryType
*         entryType:
*            interval : 0 startPort
*                       1 stopPort
*                       2 step
*                       3 APP_INV_PORT_CNS
*            list     : 0...x  ports
*            APP_INV_PORT_CNS
*
**/
#define APP_INV_PORT_CNS ((GT_U32)(~0))

typedef enum _CPSS_APP_PLATFORM_PORT_LIST_TYPE_ENT
{
     CPSS_APP_PLATFORM_PORT_LIST_TYPE_EMPTY_E = 0
    ,CPSS_APP_PLATFORM_PORT_LIST_TYPE_INTERVAL_E
    ,CPSS_APP_PLATFORM_PORT_LIST_TYPE_LIST_E
} CPSS_APP_PLATFORM_PORT_LIST_TYPE_ENT;

typedef struct _CPSS_APP_PLATFORM_PORT_CONFIG_STC
{
    CPSS_APP_PLATFORM_PORT_LIST_TYPE_ENT entryType;
    GT_PHYSICAL_PORT_NUM                 portList[CPSS_APP_PLATFORM_PORT_LIST_PORTS_MAX];  /* depending on list type */
    CPSS_PORT_INTERFACE_MODE_ENT         ifMode;
    CPSS_PORT_SPEED_ENT                  speed;
    CPSS_PORT_FEC_MODE_ENT               fecMode;
} CPSS_APP_PLATFORM_PORT_CONFIG_STC;

/**
* @struct CPSS_APP_PLATFORM_TRAFFIC_ENABLE_STC
*
* @brief Enable Traffic and port config handle
*
**/
typedef struct _CPSS_APP_PLATFORM_TRAFFIC_ENABLE_STC
{
    /* Device number of PP */
    GT_U32                               devNum;

    /* Pointer to Port configuration List */
    CPSS_APP_PLATFORM_PORT_CONFIG_STC   *portTypeListPtr;

    /* External Phy Config details */
    CPSS_APP_PLATFORM_EXT_PHY_PORT_MAP  *phyMapListPtr;
    GT_U32                               phyMapListPtrSize;

} CPSS_APP_PLATFORM_TRAFFIC_ENABLE_STC;

/**
* @union CPSS_APP_PLATFORM_RUNTIME_PARAM_VALUE_UNT
*
* @brief This union holds the value depending on
*        CPSS_APP_PLATFORM_RUNTIME_PARAM_TYPE_ENT
**/
typedef union _CPSS_APP_PLATFORM_RUNTIME_PARAM_VALUE_UNT
{
    /** Event handle structure */
    CPSS_APP_PLATFORM_EVENT_HANDLE_STC        *eventHandlePtr;
    /** Port Manager Handle structure */
    CPSS_APP_PLATFORM_PORT_MANAGER_HANDLE_STC *portManagerHandlePtr;
    /**Traffic enable information **/
    CPSS_APP_PLATFORM_TRAFFIC_ENABLE_STC      *trafficEnablePtr;
    /** Used as a last data in an array of RUNTIME PROFILE **/
    GT_VOID_PTR                                lastPtr;
} CPSS_APP_PLATFORM_RUNTIME_PARAM_VALUE_UNT;

/*
* @struct CPSS_APP_PLATFORM__RUNTIME_PROFILE_STC
*
* @brief Key value pair which identfies APP RUN TIME PROFILE
**/
typedef struct _CPSS_APP_PLATFORM_RUNTIME_PROFILE_STC
{
    CPSS_APP_PLATFORM_RUNTIME_PARAM_TYPE_ENT  runtimeInfoType;
    CPSS_APP_PLATFORM_RUNTIME_PARAM_VALUE_UNT runtimeInfoValue;
} CPSS_APP_PLATFORM_RUNTIME_PROFILE_STC;

#endif /* __CPSS_APP_PLATFORM_RUN_TIME_PROFILE_H */
