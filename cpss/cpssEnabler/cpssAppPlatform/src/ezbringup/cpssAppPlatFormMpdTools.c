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
  * @file cpssAppPlatFormMpdTools.c
  *
  * @brief  hold ASK specific functions (tools) for the board PHY config which is read from
  *  the 'ez_bringup' xml
  *
  *   NOTE: this file compiled only when 'EZ_BRINGUP' (and DX code) is enabled.
  *   meaning that 'stub' (if needed) are implemented elsewhere.
  *
  * @version   1
  ********************************************************************************
  */

#include <ezbringup/cpssAppPlatformEzBringupTools.h>
#include <ezbringup/cpssAppPlatformMpdTool.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfoEnhanced.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>


#include <cpss/common/smi/cpssGenSmi.h>

#if (defined ASIC_SIMULATION_ENV_FORBIDDEN && defined ASIC_SIMULATION)
    /* 'Forbid' the code to run as ASIC_SIMULATION ... we need 'like' HW compilation ! */
    #undef ASIC_SIMULATION
#endif

#ifdef ASIC_SIMULATION
    #include <asicSimulation/SEmbedded/simFS.h>
#endif /*ASIC_SIMULATION*/

#define END_OF_TABLE 0xFFFFFFFF

#include <pdl/init/pdlInit.h>
#include <pdl/packet_processor/pdlPacketProcessor.h>
#include <pdl/serdes/pdlSerdes.h>
#include <iDbgPdl/init/iDbgPdlInit.h>
#include <cpssAppPlatformPciConfig.h>
#ifdef MIN
#undef MIN
#endif
#include <mpdPrv.h>


/**
* @internal prvCpssAppPlatformEzbMpdPortInit function
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
GT_STATUS prvCpssAppPlatformEzbMpdPortInit
(
    IN GT_SW_DEV_NUM            devNum,
    IN GT_U32                   macPort,
    OUT GT_BOOL                 *isInitDonePtr
)
{
    GT_STATUS                   rc;
    GT_U32                      DbIndex;

    CPSS_NULL_PTR_CHECK_MAC(isInitDonePtr);
    *isInitDonePtr = GT_FALSE;

    DbIndex = devNum;
    rc = prvEzbMpdPortInit(macPort, DbIndex, isInitDonePtr);

    return rc;
}

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
)
{
    if(GT_OK != prvEzbMpdInitHw())
    {
        return GT_FAIL;
    }

    return GT_OK;
}




