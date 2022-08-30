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
* @file appDemoBoardConfig_mpdTools.c
*
* @brief  hold common function (tools) for the board config files to use to get
*   MPD functions to use. NOTE: EZ_BRINGUP also exists.
*
*   NOTE: this file compiled only when 'EZ_BRINGUP' (and DX code) and 'INCLUDE_MPD' is enabled.
*   meaning that 'stub' (if needed) are implemented elsewhere.
*
* @version   1
********************************************************************************
*/

#include <appDemo/boardConfig/appDemoBoardConfig_ezBringupTools.h>
#include <appDemo/boardConfig/appDemoBoardConfig_mpdTools.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfoEnhanced.h>
#include <cpss/common/smi/cpssGenSmi.h>
#include <cpss/dxCh/dxChxGen/phy/cpssDxChPhySmi.h>
#include <appDemo/boardConfig/appDemoBoardConfig_mpdTools.h>

#include <pdl/init/pdlInit.h>
#include <pdl/packet_processor/pdlPacketProcessor.h>
#include <pdl/sfp/pdlSfp.h>

#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>



#undef MIN
#include <mpdPrv.h>

/**
* @internal appDemoEzbMpdDevMacToIfIndexConvert function
* @endinternal
*
* @brief  convert EZ_BRINGUP {devNum,macNum} : MPD ifIndex
*
* @param[in] devNum               - the cpss SW devNum
* @param[in] macNum               - the mac number
*
* @param[out] ifIndexPtr           - (pointer to) the ifIndex of MPD for a port
*
* @retval
* @retval GT_OK                    - on success,
* @retval GT_NOT_FOUND             - the {devNum,macNum} not found to have ifIndex.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
GT_STATUS appDemoEzbMpdDevMacToIfIndexConvert
(
    IN GT_U32 devNum,
    IN GT_U32 macNum,
    OUT GT_U32 *ifIndexPtr
)
{
    CPSS_NULL_PTR_CHECK_MAC(ifIndexPtr);

    if(mpdIsIfIndexForDevPortExists(devNum,macNum,ifIndexPtr))
    {
        return GT_OK;

    }

    return GT_NOT_FOUND;
}

/**
* @internal prvAppDemoEzbMpdPortInit function
* @endinternal
*
* @brief   EZ_BRINGUP init the DB of PHY in the MPD
*           after done an all ports in all devices need to call to : prvAppDemoEzbMpdInitHw()
*
* @param[in] devNum               - the cpss SW devNum
* @param[in] macNum               - the mac number
*
* @param[out] isInitDonePtr       - (pointer to) is the port hold phy and did init
*
* @retval
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - on fail,
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
GT_STATUS prvAppDemoEzbMpdPortInit
(
    IN GT_SW_DEV_NUM            devNum,
    IN GT_U32                   macPort,
    OUT GT_BOOL                 *isInitDonePtr
)
{
    GT_STATUS                   rc;
    GT_U32                      appDemoDbIndex;

    CPSS_NULL_PTR_CHECK_MAC(isInitDonePtr);
    *isInitDonePtr = GT_FALSE;

    appDemoEzbCpssDevNumToAppDemoIndexConvert(devNum,&appDemoDbIndex);

    rc = prvEzbMpdPortInit(macPort, appDemoDbIndex, isInitDonePtr);

    return rc;
}

/**
* @internal prvAppDemoEzbMpdInitHw function
* @endinternal
*
* @brief   EZ_BRINGUP init the DB of PHY in the MPD
*           call this one only after appDemoEzbMpdPortInit(...) called  an all ports in all devices
*
* @retval
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - on fail,
*/
GT_STATUS prvAppDemoEzbMpdInitHw
(
    void
)
{
    if(GT_OK != prvEzbMpdInitHw())
    {
        return GT_FAIL;
    }

    return GT_OK;
}

