/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE IS A REFERENCE CODE FOR MARVELL SWITCH PRODUCTS.  IT IS PROVIDED   *
* "AS IS" WITH NO WARRANTIES, EXPRESSED, IMPLIED OR OTHERWISE, REGARDING ITS   *
* ACCURACY, COMPLETENESS OR PERFORMANCE.                                       *
* CUSTOMERS ARE FREE TO MODIFY IT AND USE IT ONLY IN THEIR PRODUCTION          *
* SOFTWARE RELEASES WITH MARVELL SWITCH CHIPSETS.                              *
*******************************************************************************/
/**
********************************************************************************
* @file appDemoDb.h
*
* @brief App demo database.
*
* @version   2
********************************************************************************
*/
#ifndef __appDemoDbh
#define __appDemoDbh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/cpssTypes.h>
#include <cpss/common/cpssHwInit/cpssHwInit.h>
#include <appDemo/sysHwConfig/appDemoStaticDb.h>
#include <extUtils/common/cpssEnablerUtils.h>

#define APP_DEMO_DB_ENTRY_ADD_PREVENT_OVERRIDE_MAC(name,value) \
{                                                              \
    GT_U32 tempReadValue;                                      \
    if (appDemoDbEntryGet(name, &tempReadValue) != GT_OK)      \
    {                                                          \
        appDemoDbEntryAdd(name,value);                         \
    }                                                          \
}

/**
* @internal appDemoDbEntryAdd function
* @endinternal
*
* @brief   Set AppDemo DataBase value.This value will be considered during system
*         initialization process.
* @param[in] namePtr                  - points to parameter name
* @param[in] value                    - parameter value
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - if name is too long
* @retval GT_BAD_PTR               - if NULL pointer
* @retval GT_NO_RESOURCE           - if Database is full
*/
GT_STATUS appDemoDbEntryAdd
(
    IN  GT_CHAR *namePtr,
    IN  GT_U32  value
);


/**
* @internal appDemoDbEntryGet function
* @endinternal
*
* @brief   Get parameter value from AppDemo DataBase.
*
* @param[in] namePtr                  - points to parameter name
*
* @param[out] valuePtr                 - points to parameter value
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - if name is too long
* @retval GT_BAD_PTR               - if NULL pointer
* @retval GT_NO_SUCH               - there is no such parameter in Database
*/
GT_STATUS appDemoDbEntryGet
(
    IN  GT_CHAR *namePtr,
    OUT GT_U32  *valuePtr
);

/**
* @internal appDemoDbDump function
* @endinternal
*
* @brief   Dumps entries set in AppDemo database to console.
*
* @retval GT_OK                    - on success
*/
GT_STATUS appDemoDbDump
(
    GT_VOID
);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __appDemoDbh */



