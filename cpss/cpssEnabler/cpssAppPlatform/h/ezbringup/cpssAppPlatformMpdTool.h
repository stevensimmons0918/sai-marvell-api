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
* @file cpssAppPlatformMpdTools.h
*
* @brief  hold common function (tools) for the board config files to use to get
*         info from the 'ez_bringup' xml
*
* @version   1
********************************************************************************
*/
#ifndef __cpssAppPlatformMpdTools_H
#define __cpssAppPlatformMpdTools_H


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <mpdTools.h>

#if (defined INCLUDE_MPD) && (defined CHX_FAMILY)

/**
* @internal prvCpssAppPlatformEzbMpdPortInit function
* @endinternal
*
* @brief   EZ_BRINGUP init the DB of PHY in the MPD
*           after done an all ports in all devices need to call to : prvAppDemoEzbMpdInitHw()
*
* @param[in] devNum               - the cpss SW devNum
* @param[in] macPort              - the mac number
*
* @param[out] isInitDonePtr       - (pointer to) is the port hold phy and did init
*
* @retval
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - on fail,
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
GT_STATUS prvCpssAppPlatformEzbMpdPortInit
(
    IN GT_SW_DEV_NUM            devNum,
    IN GT_U32                   macPort,
    OUT GT_BOOL                 *isInitDonePtr
);

/**
* @internal prvCpssAppPlatformEzbMpdInitHw function
* @endinternal
*
* @brief   EZ_BRINGUP init the DB of PHY in the MPD
*           call this one only after appDemoEzbMpdPortInit(...) called  an all ports in all devices
*
* @retval
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - on fail,
*/
GT_STATUS prvCpssAppPlatformEzbMpdInitHw
(
    void
);
#else /*INCLUDE_MPD*/
/* implement MACROS STUBs for compilation */
#define prvCpssAppPlatformEzbMpdPortInit(a,b,c)            	GT_NOT_IMPLEMENTED; (*c)=0;
#define prvCpssAppPlatformEzbMpdInitHw()					GT_NOT_IMPLEMENTED
#endif
#ifdef __cplusplus
}
#endif  /* __cplusplus */
#endif
