/*******************************************************************************
*              (c), Copyright 2018, Marvell International Ltd.                 *
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
* @file cpssAppPlatformPpUtils.c
*
* @brief This file contains APIs for CPSS PP utility functions.
*
* @version   1
********************************************************************************
*/

#include <cpssAppPlatformSysConfig.h>
#include <cpssAppPlatformPpConfig.h>
#include <cpssAppPlatformPpUtils.h>
#include <cpssAppPlatformPortInit.h>

#include <cpssCommon/private/prvCpssSkipList.h>
#include <cpssDriver/pp/prvCpssDrvPpDefs.h>
#include <gtExtDrv/drivers/gtPciDrv.h>

#include <extUtils/common/cpssEnablerUtils.h>

#if defined CHX_FAMILY
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwInitLedCtrl.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwInit.h>
#include <cpss/dxCh/dxChxGen/private/lpm/tcam/prvCpssDxChLpmTcamCommonTypes.h>

#include <cpss/dxCh/dxChxGen/port/cpssDxChPortTxDba.h>
#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgSecurityBreach.h>
#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgGen.h>
#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgStp.h>
#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgMc.h>
#include <cpss/dxCh/dxChxGen/nst/cpssDxChNstPortIsolation.h>
#include <cpss/dxCh/dxChxGen/nst/cpssDxChNst.h>
#include <cpss/dxCh/dxChxGen/ip/cpssDxChIp.h>
#include <cpss/dxCh/dxChxGen/ipLpmEngine/private/cpssDxChPrvIpLpm.h>
#include <cpss/dxCh/dxChxGen/ipLpmEngine/cpssDxChIpLpm.h>
#include <cpss/dxCh/dxChxGen/networkIf/cpssDxChNetIf.h>
#include <cpss/dxCh/dxChxGen/trunk/cpssDxChTrunk.h>
#include <cpss/dxCh/dxChxGen/cos/cpssDxChCos.h>
#include <cpss/dxCh/dxChxGen/cscd/cpssDxChCscd.h>
#include <cpss/dxCh/dxChxGen/tti/cpssDxChTti.h>
#include <cpss/dxCh/dxChxGen/ptp/cpssDxChPtp.h>
#include <cpss/dxCh/dxChxGen/pha/cpssDxChPha.h>
#include <cpss/dxCh/dxChxGen/policer/cpssDxChPolicer.h>
#include <cpss/dxCh/dxChxGen/tcam/cpssDxChTcam.h>
#include <cpss/dxCh/dxChxGen/pcl/cpssDxChPcl.h>
#include <cpss/dxCh/dxChxGen/mirror/cpssDxChMirror.h>
#include <cpss/dxCh/dxChxGen/phy/cpssDxChPhySmi.h>

#endif/*CHX_FAMILY*/
#include <ezbringup/cpssAppPlatformEzBringupTools.h>

#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

/*global variables macros*/
#define PRV_SHARED_PORT_DIR_IP_LPM_SRC_GLOBAL_VAR_GET(_var)\
    PRV_SHARED_GLOBAL_VAR_GET(mainPpDrvMod.ipLpmDir.ipLpmSrc._var)
#define HWINIT_GLOVAR(_var) \
    PRV_SHARED_GLOBAL_VAR_GET(commonMod.genericHwInitDir._var)

/* app database */
static CPSS_ENABLER_DB_ENTRY_STC    appDb[CPSS_ENABLER_DB_MAX_SIZE_CNS];
static GT_U32                       appDbSize = 0;
static GT_U32                       initDone = 0;

/* offset used during HW device ID calculation formula */
GT_U8 appRefHwDevNumOffset = 0x10;

static GT_BOOL lpmDbInitialized = GT_FALSE;     /* keeps if LPM DB was created */

extern GT_STATUS   prvCpssDxChPhaFwVersionPrint(IN GT_U8    devNum);

/* default values for Aldrin2 Tail Drop DBA disable configuration */
#define PRV_CPSS_ALDRIN2_TAIL_DROP_DEFAULT_MAX_BUF_LIMIT_CNS      0x28
#define PRV_CPSS_ALDRIN2_TAIL_DROP_DEFAULT_MAX_DESCR_LIMIT_CNS    0x19
#define PRV_CPSS_ALDRIN2_TAIL_DROP_DEFAULT_DBA_ALPHA_CNS          CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_0_E
#define PRV_CPSS_ALDRIN2_TAIL_DROP_DEFAULT_DBA_AVAILABLE_BUFS_CNS 0x9060

/* next info not saved in CPSS , so can not be retrieved from CPSS. (lack of support)
   so we need to store the DMAs allocated from the first time
*/
static CPSS_DXCH_PP_PHASE2_INIT_INFO_STC cpssAppPlatformPpPhase2Db[CPSS_APP_PLATFORM_MAX_PP_CNS];

static GT_BOOL applicationPlatformTtiTcamUseOffset          = GT_TRUE;
static GT_BOOL applicationPlatformPclTcamUseIndexConversion = GT_TRUE;


static GT_U32 applicationPlatformTcamPclRuleBaseIndexOffset     = 0;
static GT_U32 applicationPlatformTcamIpcl0RuleBaseIndexOffset   = 0;
static GT_U32 applicationPlatformTcamIpcl1RuleBaseIndexOffset   = 0;
static GT_U32 applicationPlatformTcamIpcl2RuleBaseIndexOffset   = 0;
static GT_U32 applicationPlatformTcamEpclRuleBaseIndexOffset    = 0;
/* number of rules that can be used per PLC clients */
static GT_U32 applicationPlatformTcamIpcl0MaxNum = 0;
static GT_U32 applicationPlatformTcamIpcl1MaxNum = 0;
static GT_U32 applicationPlatformTcamIpcl2MaxNum = 0;
static GT_U32 applicationPlatformTcamEpclMaxNum  = 0;

/* base offset for TTI client in TCAM */
static GT_U32 applicationPlatformTcamTtiHit0RuleBaseIndexOffset;
static GT_U32 applicationPlatformTcamTtiHit1RuleBaseIndexOffset;
static GT_U32 applicationPlatformTcamTtiHit2RuleBaseIndexOffset;
static GT_U32 applicationPlatformTcamTtiHit3RuleBaseIndexOffset;
/* number of rules that can be used per lookup of TTI */
static GT_U32 applicationPlatformTcamTtiHit0MaxNum = 0;
static GT_U32 applicationPlatformTcamTtiHit1MaxNum = 0;
static GT_U32 applicationPlatformTcamTtiHit2MaxNum = 0;
static GT_U32 applicationPlatformTcamTtiHit3MaxNum = 0;

static GT_U32  save_applicationPlatformTcamTtiHit0RuleBaseIndexOffset;
static GT_U32  save_applicationPlatformTcamTtiHit1RuleBaseIndexOffset;
static GT_U32  save_applicationPlatformTcamTtiHit2RuleBaseIndexOffset;
static GT_U32  save_applicationPlatformTcamTtiHit3RuleBaseIndexOffset;

static GT_U32  save_applicationPlatformTcamTtiHit0MaxNum;
static GT_U32  save_applicationPlatformTcamTtiHit1MaxNum;
static GT_U32  save_applicationPlatformTcamTtiHit2MaxNum;
static GT_U32  save_applicationPlatformTcamTtiHit3MaxNum;

static GT_U32 save_applicationPlatformTcamIpcl0RuleBaseIndexOffset = 0;
static GT_U32 save_applicationPlatformTcamIpcl1RuleBaseIndexOffset = 0;
static GT_U32 save_applicationPlatformTcamIpcl2RuleBaseIndexOffset = 0;
static GT_U32 save_applicationPlatformTcamEpclRuleBaseIndexOffset  = 0;

static GT_U32 save_applicationPlatformTcamIpcl0MaxNum = 0;
static GT_U32 save_applicationPlatformTcamIpcl1MaxNum = 0;
static GT_U32 save_applicationPlatformTcamIpcl2MaxNum = 0;
static GT_U32 save_applicationPlatformTcamEpclMaxNum  = 0;

#ifdef LINUX_NOKM
extern GT_STATUS extDrvGetDmaSize(OUT GT_U32 * dmaSize);
#endif

/* max number of LPM blocks on eArch architecture */
#define APP_CPSS_MAX_NUM_OF_LPM_BLOCKS_CNS    20

GT_BOOL                            appPlatformLpmRamConfigInfoSet=GT_FALSE;
CPSS_DXCH_LPM_RAM_CONFIG_INFO_STC  appPlatformLpmRamConfigInfo[CPSS_DXCH_CFG_NUM_OF_DEV_TYPES_MANAGED_CNS];
GT_U32                             appPlatformLpmRamConfigInfoNumOfElements;

/**
* @internal appPlatformDbEntryGet function
* @endinternal
*
* @brief   Get parameter value from App DataBase.
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
GT_STATUS appPlatformDbEntryGet
(
    IN  GT_CHAR *namePtr,
    OUT GT_U32  *valuePtr
)
{
    GT_U32 i;

    /* check parameters */
    if ((namePtr == NULL) || (valuePtr == NULL))
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PTR, LOG_ERROR_NO_MSG);

    /* check name is not too long */
    if (cpssOsStrlen(namePtr) > CPSS_ENABLER_DB_NAME_MAX_LEN_CNS)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

    /* search the name in the databse */
    for (i = 0 ; i < appDbSize ; i++)
    {
        if (cpssOsStrCmp(namePtr, appDb[i].name) == 0)
        {
            *valuePtr = appDb[i].value;
            return GT_OK;
        }
    }

    /* the entry wasn't found */
    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_SUCH, LOG_ERROR_NO_MSG);
}

/**
* @internal appPlatformDbEntryAdd function
* @endinternal
*
* @brief   Set App DataBase value.This value will be considered during system
*         initialization process.
* @param[in] namePtr                  - points to parameter name
* @param[in] value                    - parameter value
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - if name is too long
* @retval GT_BAD_PTR               - if NULL pointer
* @retval GT_NO_RESOURCE           - if Database is full
*/
GT_STATUS appPlatformDbEntryAdd
(
    IN  GT_CHAR *namePtr,
    IN  GT_U32  value
)
{
    GT_U32 i;

    if(initDone == 0)
    {
        initDone = 1;
    }

    /* check parameters */
    if (namePtr == NULL)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PTR, LOG_ERROR_NO_MSG);

    /* check database is not full */
    if (appDbSize >= CPSS_ENABLER_DB_MAX_SIZE_CNS)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_RESOURCE, LOG_ERROR_NO_MSG);

    /* check name is not too long */
    if (cpssOsStrlen(namePtr) > CPSS_ENABLER_DB_NAME_MAX_LEN_CNS)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

    /* search if the name already exists, if so override the value */
    for (i = 0 ; i < appDbSize ; i++)
    {
        if (cpssOsStrCmp(namePtr, appDb[i].name) == 0)
        {
            appDb[i].value = value;
            return GT_OK;
        }
    }

    /* the entry wasn't already in database, add it */
    cpssOsStrCpy(appDb[appDbSize].name, namePtr);
    appDb[appDbSize].value = value;
    appDbSize++;

    return GT_OK;
}

/**
* @internal appPlatformDbBlocksAllocationMethodGet function
* @endinternal
*
* @brief   Get the blocks allocation method configured in the Init System
*/
GT_STATUS appPlatformDbBlocksAllocationMethodGet
(
    IN  GT_U8                                               dev,
    OUT CPSS_DXCH_LPM_RAM_BLOCKS_ALLOCATION_METHOD_ENT  *blocksAllocationMethodGet
)
{
    GT_U32 value;
    (void)dev;
    if(appPlatformDbEntryGet("lpmRamMemoryBlocksCfg.lpmRamBlocksAllocationMethod", &value) == GT_OK)
        *blocksAllocationMethodGet = (CPSS_DXCH_LPM_RAM_BLOCKS_ALLOCATION_METHOD_ENT)value;
    else
        *blocksAllocationMethodGet = CPSS_DXCH_LPM_RAM_BLOCKS_ALLOCATION_METHOD_DYNAMIC_WITHOUT_BLOCK_SHARING_E;

    return GT_OK;
}

/**
* @internal appPlatformBc2IpLpmRamDefaultConfigCalc function
* @endinternal
*
* @brief   This function calculate the default RAM LPM DB configuration for LPM management.
*
* @param[in] devNum                   - The Pp device number to get the parameters for.
* @param[in] maxNumOfPbrEntries       - number of PBR entries to deduct from the LPM memory calculations
*
* @param[out] ramDbCfgPtr              - (pointer to) ramDbCfg structure to hold the defaults calculated
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS appPlatformBc2IpLpmRamDefaultConfigCalc
(
    IN  GT_U8                                    devNum,
    IN  GT_U32                                   maxNumOfPbrEntries,
    OUT CPSS_DXCH_LPM_RAM_CONFIG_STC             *ramDbCfgPtr
)
{
    GT_U32 i=0;
    GT_STATUS rc = GT_OK;
    GT_U32 blockSizeInBytes;
    GT_U32 blockSizeInLines;
    GT_U32 lastBlockSizeInLines;
    GT_U32 lastBlockSizeInBytes;
    GT_U32 lpmRamNumOfLines;
    GT_U32 numOfPbrBlocks;
    GT_U32 maxNumOfPbrEntriesToUse;
    GT_U32 value;

    if(appPlatformDbEntryGet("maxNumOfPbrEntries", &value) == GT_OK)
        maxNumOfPbrEntriesToUse = value;
    else
        maxNumOfPbrEntriesToUse = maxNumOfPbrEntries;

    /*relevant for BC3 only*/
    if(appPlatformDbEntryGet("lpmMemMode", &value) == GT_OK)
        ramDbCfgPtr->lpmMemMode = value?CPSS_DXCH_LPM_RAM_MEM_MODE_HALF_MEM_SIZE_E:CPSS_DXCH_LPM_RAM_MEM_MODE_FULL_MEM_SIZE_E;
    else
        ramDbCfgPtr->lpmMemMode = CPSS_DXCH_LPM_RAM_MEM_MODE_FULL_MEM_SIZE_E;

    lpmRamNumOfLines = PRV_CPSS_DXCH_LPM_RAM_GET_NUM_OF_LINES_MAC(PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.tableSize.lpmRam);

    /*if we are working in half memory mode - then do all the calculations for half size,return to the real values later in the code*/
    if(ramDbCfgPtr->lpmMemMode == CPSS_DXCH_LPM_RAM_MEM_MODE_HALF_MEM_SIZE_E)
    {
        lpmRamNumOfLines/=2;
    }

    blockSizeInLines = (lpmRamNumOfLines/APP_CPSS_MAX_NUM_OF_LPM_BLOCKS_CNS);
    if (blockSizeInLines==0)
    {
        /* can not create a shadow with the current lpmRam size */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }
    blockSizeInBytes = blockSizeInLines * 4;

    if (maxNumOfPbrEntriesToUse >= lpmRamNumOfLines)
    {
        /* No memory for Ip LPM */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    if(maxNumOfPbrEntriesToUse > blockSizeInLines)
    {
        numOfPbrBlocks = (maxNumOfPbrEntriesToUse + blockSizeInLines - 1) / blockSizeInLines;
        lastBlockSizeInLines = (numOfPbrBlocks*blockSizeInLines)-maxNumOfPbrEntriesToUse;
        if (lastBlockSizeInLines==0)/* PBR will fit exactly in numOfPbrBlocks */
        {
            ramDbCfgPtr->numOfBlocks = APP_CPSS_MAX_NUM_OF_LPM_BLOCKS_CNS - numOfPbrBlocks;
            lastBlockSizeInLines = blockSizeInLines; /* all of last block for IP LPM */
        }
        else/* PBR will not fit exactly in numOfPbrBlocks and we will have in the last block LPM lines together with PBR lines*/
        {
            ramDbCfgPtr->numOfBlocks = APP_CPSS_MAX_NUM_OF_LPM_BLOCKS_CNS - numOfPbrBlocks + 1;
        }
    }
    else
    {
        if (maxNumOfPbrEntriesToUse == blockSizeInLines)
        {
            ramDbCfgPtr->numOfBlocks = APP_CPSS_MAX_NUM_OF_LPM_BLOCKS_CNS-1;
            lastBlockSizeInLines = blockSizeInLines;
        }
        else
        {
            ramDbCfgPtr->numOfBlocks = APP_CPSS_MAX_NUM_OF_LPM_BLOCKS_CNS;
            lastBlockSizeInLines = blockSizeInLines - maxNumOfPbrEntriesToUse;
        }
    }

    /* number of LPM bytes ONLY when last block is shared between LPM and PBR */
    lastBlockSizeInBytes = lastBlockSizeInLines * 4;

    /*ram configuration should contain physical block sizes,we divided block sizes for a calcultion earlier in the code,
            so now return to real values*/

    if(ramDbCfgPtr->lpmMemMode == CPSS_DXCH_LPM_RAM_MEM_MODE_HALF_MEM_SIZE_E)
    {
        blockSizeInBytes*=2;
        lastBlockSizeInBytes*=2;
    }

    for (i=0;i<ramDbCfgPtr->numOfBlocks-1;i++)
    {
        ramDbCfgPtr->blocksSizeArray[i] = blockSizeInBytes;
    }

    ramDbCfgPtr->blocksSizeArray[ramDbCfgPtr->numOfBlocks-1] =
        lastBlockSizeInBytes == 0 ?
            blockSizeInBytes :   /* last block is fully LPM (not PBR) */
            lastBlockSizeInBytes;/* last block uses 'x' for LPM , rest for PBR */

    /* reset other sections */
    i = ramDbCfgPtr->numOfBlocks;
    for (/*continue i*/;i<APP_CPSS_MAX_NUM_OF_LPM_BLOCKS_CNS;i++)
    {
        ramDbCfgPtr->blocksSizeArray[i] = 0;
    }

    rc = appPlatformDbBlocksAllocationMethodGet(devNum,&ramDbCfgPtr->blocksAllocationMethod);
    if (rc != GT_OK)
    {
        return rc;
    }
    return GT_OK;
}

/**
* @internal appPlatformLpmRamConfigSet function
* @endinternal
*
* @brief  Set given list of device type and Shared memory configuration mode.
*         Should be called before cpssInitSystem().
*
* @param[in] lpmRamConfigInfoArray          - array of pairs: devType+Shared memory configuration mode
* @param[in] lpmRamConfigInfoNumOfElements  - number of valid pairs

* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on lpmRamConfigInfoNumOfElements bigger than array size
*/
GT_STATUS appPlatformLpmRamConfigSet
(
    IN CPSS_DXCH_LPM_RAM_CONFIG_INFO_STC    lpmRamConfigInfoArray[CPSS_DXCH_CFG_NUM_OF_DEV_TYPES_MANAGED_CNS],
    IN GT_U32                               lpmRamConfigInfoNumOfElements
)
{
    GT_U32 i=0;

    CPSS_NULL_PTR_CHECK_MAC(lpmRamConfigInfoArray);

    if(lpmRamConfigInfoNumOfElements > CPSS_DXCH_CFG_NUM_OF_DEV_TYPES_MANAGED_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    for (i=0;i<lpmRamConfigInfoNumOfElements;i++)
    {
        appPlatformLpmRamConfigInfo[i].devType =  lpmRamConfigInfoArray[i].devType;
        appPlatformLpmRamConfigInfo[i].sharedMemCnfg = lpmRamConfigInfoArray[i].sharedMemCnfg;
    }

    appPlatformLpmRamConfigInfoNumOfElements = lpmRamConfigInfoNumOfElements;
    appPlatformLpmRamConfigInfoSet = GT_TRUE;
    return GT_OK;
}

/**
* @internal appPlatformLpmRamConfigGet function
* @endinternal
*
* @brief  Get given list of device type and Shared memory configuration mode.
*
* @param[out] lpmRamConfigInfoArray             - array of pairs: devType+Shared memory configuration mode
* @param[out] lpmRamConfigInfoNumOfElementsPtr  - (pointer to) number of valid pairs
* @param[out] lpmRamConfigInfoSetFlagPtr        - (pointer to) Flag specifying that the configuration was set
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS appPlatformLpmRamConfigGet
(
    OUT CPSS_DXCH_LPM_RAM_CONFIG_INFO_STC    lpmRamConfigInfoArray[CPSS_DXCH_CFG_NUM_OF_DEV_TYPES_MANAGED_CNS],
    OUT GT_U32                               *lpmRamConfigInfoNumOfElementsPtr,
    OUT GT_BOOL                              *lpmRamConfigInfoSetFlagPtr
)
{
    GT_U32 i=0;

    CPSS_NULL_PTR_CHECK_MAC(lpmRamConfigInfoArray);
    CPSS_NULL_PTR_CHECK_MAC(lpmRamConfigInfoNumOfElementsPtr);
    CPSS_NULL_PTR_CHECK_MAC(lpmRamConfigInfoSetFlagPtr);

    *lpmRamConfigInfoSetFlagPtr=appPlatformLpmRamConfigInfoSet;

    if (appPlatformLpmRamConfigInfoSet==GT_FALSE)
    {
        return GT_OK;
    }

    for (i = 0; i < appPlatformLpmRamConfigInfoNumOfElements; i++)
    {
        lpmRamConfigInfoArray[i].devType=appPlatformLpmRamConfigInfo[i].devType;
        lpmRamConfigInfoArray[i].sharedMemCnfg=appPlatformLpmRamConfigInfo[i].sharedMemCnfg;
    }

    *lpmRamConfigInfoNumOfElementsPtr = appPlatformLpmRamConfigInfoNumOfElements;

    return GT_OK;
}

/**
* @internal appPlatformFalconIpLpmRamDefaultConfigCalc function
* @endinternal
*
* @brief   This function calculate the default RAM LPM DB configuration for LPM management.
*
* @param[in] devNum                   - The Pp device number to get the parameters for.
* @param[in] sharedTableMode          - shared tables mode
* @param[in] maxNumOfPbrEntries       - number of PBR entries to deduct from the LPM memory calculations
*
* @param[out] ramDbCfgPtr              - (pointer to) ramDbCfg structure to hold the defaults calculated
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS appPlatformFalconIpLpmRamDefaultConfigCalc
(
    IN  GT_U8                                    devNum,
    IN  CPSS_DXCH_CFG_SHARED_TABLE_MODE_ENT      sharedTableMode,
    IN  GT_U32                                   maxNumOfPbrEntries,
    OUT CPSS_DXCH_LPM_RAM_CONFIG_STC             *ramDbCfgPtr
)
{
    GT_U32 value;
    GT_U32 i=0;
    CPSS_DXCH_CFG_DEV_INFO_STC   devInfo;
    GT_STATUS rc = GT_OK;

    rc = cpssDxChCfgDevInfoGet(devNum, &devInfo);
    if (rc!=GT_OK)
    {
        return rc;
    }

    if (appPlatformLpmRamConfigInfoSet==GT_TRUE)
    {
        for (i=0;i<appPlatformLpmRamConfigInfoNumOfElements;i++)
        {
            ramDbCfgPtr->lpmRamConfigInfo[i].devType = appPlatformLpmRamConfigInfo[i].devType;
            ramDbCfgPtr->lpmRamConfigInfo[i].sharedMemCnfg = appPlatformLpmRamConfigInfo[i].sharedMemCnfg;
        }
        ramDbCfgPtr->lpmRamConfigInfoNumOfElements=appPlatformLpmRamConfigInfoNumOfElements;
    }
    else
    {
        /* set single configuration of current device */
        ramDbCfgPtr->lpmRamConfigInfo[0].devType = devInfo.genDevInfo.devType;
        ramDbCfgPtr->lpmRamConfigInfo[0].sharedMemCnfg=sharedTableMode;
        ramDbCfgPtr->lpmRamConfigInfoNumOfElements=1;
    }

    if(appPlatformDbEntryGet("maxNumOfPbrEntries", &value) == GT_OK)
        ramDbCfgPtr->maxNumOfPbrEntries = value;
    else
        ramDbCfgPtr->maxNumOfPbrEntries = maxNumOfPbrEntries;

    if(appPlatformDbEntryGet("sharedTableMode", &value) == GT_OK)
    {
        for (i=0;i<ramDbCfgPtr->lpmRamConfigInfoNumOfElements;i++)
        {
            ramDbCfgPtr->lpmRamConfigInfo[i].sharedMemCnfg = value;
        }
    }

    ramDbCfgPtr->lpmMemMode = CPSS_DXCH_LPM_RAM_MEM_MODE_FULL_MEM_SIZE_E;/*the only mode for Falcon*/
        if(appPlatformDbEntryGet("lpmRamMemoryBlocksCfg.lpmRamBlocksAllocationMethod", &value) == GT_OK)
        ramDbCfgPtr->blocksAllocationMethod  = (CPSS_DXCH_LPM_RAM_BLOCKS_ALLOCATION_METHOD_ENT)value;
    else
        ramDbCfgPtr->blocksAllocationMethod  = CPSS_DXCH_LPM_RAM_BLOCKS_ALLOCATION_METHOD_DYNAMIC_WITHOUT_BLOCK_SHARING_E;

    return GT_OK;
}

/**
* @internal cpssAppPlatformPhase2Init function
* @endinternal
*
* @brief   Phase2 PP configurations
*
* @param [in] deviceNumber       - CPSS device number,
* @param [in] *ppProfileInfo     - PP profile
*
* @retval GT_OK                  - on success,
* @retval GT_FAIL                - otherwise.
*/
GT_STATUS cpssAppPlatformPhase2Init
(
    IN GT_U8                             devNum,
    IN CPSS_APP_PLATFORM_PP_PROFILE_STC *ppProfilePtr
)
{
    GT_STATUS                  rc = GT_BAD_PARAM;
    CPSS_PP_DEVICE_TYPE        devType;
    CPSS_PP_FAMILY_TYPE_ENT    devFamily;
    CPSS_DXCH_CFG_DEV_INFO_STC devInfo;
    GT_HW_DEV_NUM              hwDevNum;
    GT_U32                    *tmpPtr;
    GT_U32                     rxBufSize = RX_BUFF_SIZE_DEF;
    GT_U32                     rxBufAllign = 1;
    GT_U32                     txQue,rxQue;
    GT_U32                     descSize,ii,i,jj;
    GT_BOOL                    txGenMode = GT_FALSE;
    CPSS_DXCH_PP_PHASE2_INIT_INFO_STC cpssPpPhase2;
    CPSS_SYSTEM_RECOVERY_INFO_STC     system_recovery;
#ifdef CHX_FAMILY
    CPSS_NET_IF_CFG_STC*       prevCpssInit_netIfCfgPtr = NULL;
    CPSS_AUQ_CFG_STC*          prevCpssInit_auqCfgPtr = NULL;
    GT_BOOL*                   prevCpssInit_fuqUseSeparatePtr = NULL;
    CPSS_AUQ_CFG_STC*          prevCpssInit_fuqCfgPtr = NULL;
    GT_BOOL*                   prevCpssInit_useMultiNetIfSdmaPtr = NULL;
    CPSS_MULTI_NET_IF_CFG_STC* prevCpssInit_multiNetIfCfgPtr = NULL;
#endif

    CPSS_APP_PLATFORM_LOG_ENTRY_MAC();

    if (NULL == ppProfilePtr)
    {
        CPSS_APP_PLATFORM_LOG_ERR_MAC("Input profile is NULL.\n");
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PTR, LOG_ERROR_NO_MSG);
    }

    cpssOsMemSet(&cpssPpPhase2, 0, sizeof(cpssPpPhase2));

    rc = cpssDxChCfgDevInfoGet(devNum, &devInfo);
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChCfgDevInfoGet);

    devType = devInfo.genDevInfo.devType;
    devFamily = devInfo.genDevInfo.devFamily;

    if (ppProfilePtr->newDevNum >= 32)
    {
        cpssPpPhase2.newDevNum = devNum;
    }
    else
    {
        cpssPpPhase2.newDevNum = ppProfilePtr->newDevNum;
    }

    cpssPpPhase2.useDoubleAuq = ppProfilePtr->useDoubleAuq;
    cpssPpPhase2.useSecondaryAuq = ppProfilePtr->useSecondaryAuq;
    cpssPpPhase2.auMessageLength = ppProfilePtr->auMessageLength;
    cpssPpPhase2.fuqUseSeparate = ppProfilePtr->fuqUseSeparate;

    cpssPpPhase2.noTraffic2CPU = ppProfilePtr->noTraffic2CPU;
    cpssPpPhase2.useMultiNetIfSdma = ppProfilePtr->useMultiNetIfSdma;
    cpssPpPhase2.netifSdmaPortGroupId = ppProfilePtr->netifSdmaPortGroupId;

    rc = cpssSystemRecoveryStateGet(&system_recovery);
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssSystemRecoveryStateGet);

#ifdef CHX_FAMILY
    cpssDxChCfgDevDbInfoGet(devNum,
        &prevCpssInit_netIfCfgPtr,
        &prevCpssInit_auqCfgPtr,
        &prevCpssInit_fuqUseSeparatePtr,
        &prevCpssInit_fuqCfgPtr,
        &prevCpssInit_useMultiNetIfSdmaPtr,
        &prevCpssInit_multiNetIfCfgPtr);
#endif

    if(prevCpssInit_auqCfgPtr)
    {
        /* the HW did not do reset ... reuse the previous init parameters */
        cpssPpPhase2.auqCfg = *prevCpssInit_auqCfgPtr;
    }
    else if(ppProfilePtr->auDescNum == 0)
    {
        cpssPpPhase2.auqCfg.auDescBlock = 0;
        cpssPpPhase2.auqCfg.auDescBlockSize = 0;
    }
    else
    {
        /* Au block size calc & malloc  */
        cpssDxChHwAuDescSizeGet(devType,&descSize);
        cpssPpPhase2.auqCfg.auDescBlockSize = descSize * ppProfilePtr->auDescNum;
        cpssPpPhase2.auqCfg.auDescBlock =
            osCacheDmaMalloc(cpssPpPhase2.auqCfg.auDescBlockSize +
                            descSize);/*allocate space for one message more for alignment purposes
                                      NOTE: we not add it to the size , only to the buffer ! */
        if(cpssPpPhase2.auqCfg.auDescBlock == NULL)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
        }
        if(((GT_UINTPTR)cpssPpPhase2.auqCfg.auDescBlock) % descSize)
        {
            /* add to the size the extra value for alignment , to the actual size
               will be as needed , after the reduction in the cpss code */
            cpssPpPhase2.auqCfg.auDescBlockSize += descSize;
        }
    }

    if(prevCpssInit_fuqCfgPtr)
    {
        /* the HW did not do reset ... reuse the previous init parameters */
        cpssPpPhase2.fuqCfg = *prevCpssInit_fuqCfgPtr;
    }
    else if(ppProfilePtr->fuDescNum == 0)
    {
        cpssPpPhase2.fuqCfg.auDescBlock = 0;
        cpssPpPhase2.fuqCfg.auDescBlockSize = 0;
    }
    else
    {
        /* Fu block size calc & malloc  */
        cpssDxChHwAuDescSizeGet(devType,&descSize);
        cpssPpPhase2.fuqCfg.auDescBlockSize = descSize * ppProfilePtr->fuDescNum;
        cpssPpPhase2.fuqCfg.auDescBlock =
            osCacheDmaMalloc(cpssPpPhase2.fuqCfg.auDescBlockSize +
                            descSize);/*allocate space for one message more for alignment purposes
                                      NOTE: we not add it to the size , only to the buffer ! */
        if(cpssPpPhase2.fuqCfg.auDescBlock == NULL)
        {
            osCacheDmaFree(cpssPpPhase2.auqCfg.auDescBlock);
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
        }

        if(((GT_UINTPTR)cpssPpPhase2.fuqCfg.auDescBlock) % descSize)
        {
            /* add to the size the extra value for alignment , to the actual size
               will be as needed , after the reduction in the cpss code */
            cpssPpPhase2.fuqCfg.auDescBlockSize += descSize;
        }
    }

    if(prevCpssInit_netIfCfgPtr)
    {
        /* the HW did not do reset ... reuse the previous init parameters */
        cpssPpPhase2.netIfCfg = *prevCpssInit_netIfCfgPtr;
    }
    else
    {
        /* Tx block size calc & malloc  */
        if(ppProfilePtr->useMultiNetIfSdma == GT_FALSE)
        {
            cpssPpPhase2.netIfCfg.rxBufInfo.allocMethod = ppProfilePtr->rxAllocMethod;
            cpssOsMemCpy(cpssPpPhase2.netIfCfg.rxBufInfo.bufferPercentage,
                         ppProfilePtr->rxBufferPercentage, sizeof(ppProfilePtr->rxBufferPercentage));

            cpssDxChHwTxDescSizeGet(devType,&descSize);
            cpssPpPhase2.netIfCfg.txDescBlockSize = descSize * ppProfilePtr->txDescNum;
            if (cpssPpPhase2.netIfCfg.txDescBlockSize != 0)
            {
                cpssPpPhase2.netIfCfg.txDescBlock =
                    osCacheDmaMalloc(cpssPpPhase2.netIfCfg.txDescBlockSize);
                if(cpssPpPhase2.netIfCfg.txDescBlock == NULL)
                {
                    osCacheDmaFree(cpssPpPhase2.auqCfg.auDescBlock);
                    osCacheDmaFree(cpssPpPhase2.fuqCfg.auDescBlock);
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
                }
            }
            if((system_recovery.systemRecoveryState == CPSS_SYSTEM_RECOVERY_COMPLETION_STATE_E)||
               ( (system_recovery.systemRecoveryState == CPSS_SYSTEM_RECOVERY_INIT_STATE_E) &&
                  (system_recovery.systemRecoveryMode.continuousTx == GT_FALSE)) )
            {
                cpssOsMemSet(cpssPpPhase2.netIfCfg.txDescBlock,0,
                         cpssPpPhase2.netIfCfg.txDescBlockSize);
            }
            /* Rx block size calc & malloc  */
            cpssDxChHwRxDescSizeGet(devType,&descSize);

            cpssPpPhase2.netIfCfg.rxDescBlockSize = descSize * ppProfilePtr->rxDescNum;
            if (cpssPpPhase2.netIfCfg.rxDescBlockSize != 0)
            {
                cpssPpPhase2.netIfCfg.rxDescBlock =
                    osCacheDmaMalloc(cpssPpPhase2.netIfCfg.rxDescBlockSize);
                if(cpssPpPhase2.netIfCfg.rxDescBlock == NULL)
                {
                    osCacheDmaFree(cpssPpPhase2.auqCfg.auDescBlock);
                    osCacheDmaFree(cpssPpPhase2.fuqCfg.auDescBlock);
                    osCacheDmaFree(cpssPpPhase2.netIfCfg.txDescBlock);
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
                }
            }
            if((system_recovery.systemRecoveryState == CPSS_SYSTEM_RECOVERY_COMPLETION_STATE_E)||
               ( (system_recovery.systemRecoveryState == CPSS_SYSTEM_RECOVERY_INIT_STATE_E) &&
                  (system_recovery.systemRecoveryMode.continuousRx == GT_FALSE)) )
            {
                cpssOsMemSet(cpssPpPhase2.netIfCfg.rxDescBlock,0,
                         cpssPpPhase2.netIfCfg.rxDescBlockSize);
            }
            /* init the Rx buffer allocation method */
            /* Set the system's Rx buffer size.     */
            if((rxBufSize % rxBufAllign) != 0)
            {
                rxBufSize = (rxBufSize + (rxBufAllign - (rxBufSize % rxBufAllign)));
            }

            if (cpssPpPhase2.netIfCfg.rxBufInfo.allocMethod == CPSS_RX_BUFF_STATIC_ALLOC_E)
            {
                cpssPpPhase2.netIfCfg.rxBufInfo.rxBufSize = rxBufSize;
                cpssPpPhase2.netIfCfg.rxBufInfo.buffData.staticAlloc.rxBufBlockSize = rxBufSize * ppProfilePtr->rxDescNum;

                /* set status of RX buffers - cacheable or not */
                cpssPpPhase2.netIfCfg.rxBufInfo.buffersInCachedMem = ppProfilePtr->rxBuffersInCachedMem;

                if ((rxBufSize * ppProfilePtr->rxDescNum) != 0)
                {
                    /* If RX buffers should be cachable - allocate it from regular memory */
                    if (GT_TRUE == cpssPpPhase2.netIfCfg.rxBufInfo.buffersInCachedMem)
                    {
                        tmpPtr = osMalloc(((rxBufSize * ppProfilePtr->rxDescNum) + rxBufAllign-1));
                    }
                    else
                    {
                        tmpPtr = osCacheDmaMalloc(((rxBufSize * ppProfilePtr->rxDescNum) + rxBufAllign-1));
                    }

                    if(tmpPtr == NULL)
                    {
                        osCacheDmaFree(cpssPpPhase2.auqCfg.auDescBlock);
                        osCacheDmaFree(cpssPpPhase2.fuqCfg.auDescBlock);
                        osCacheDmaFree(cpssPpPhase2.netIfCfg.txDescBlock);
                        osCacheDmaFree(cpssPpPhase2.netIfCfg.rxDescBlock);
                        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
                    }
                }
                else
                {
                    tmpPtr = NULL;
                }

                if((((GT_UINTPTR)tmpPtr) % rxBufAllign) != 0)
                {
                    tmpPtr = (GT_U32*)(((GT_UINTPTR)tmpPtr) +
                                       (rxBufAllign - (((GT_UINTPTR)tmpPtr) % rxBufAllign)));
                }
                cpssPpPhase2.netIfCfg.rxBufInfo.buffData.staticAlloc.rxBufBlockPtr = tmpPtr;
            }
            else if (cpssPpPhase2.netIfCfg.rxBufInfo.allocMethod == CPSS_RX_BUFF_NO_ALLOC_E)
            {
                /* do not allocate rx buffers*/
            }
            else
            {
                /* dynamic RX buffer allocation currently is not supported by applicationPlatform*/
                osCacheDmaFree(cpssPpPhase2.auqCfg.auDescBlock);
                osCacheDmaFree(cpssPpPhase2.fuqCfg.auDescBlock);
                osCacheDmaFree(cpssPpPhase2.netIfCfg.txDescBlock);
                osCacheDmaFree(cpssPpPhase2.netIfCfg.rxDescBlock);
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
            }
        }
    }

    if(prevCpssInit_useMultiNetIfSdmaPtr)
    {
        /* the HW did not do reset ... reuse the previous init parameters */
        if(prevCpssInit_multiNetIfCfgPtr)
        {
            cpssPpPhase2.multiNetIfCfg     = *prevCpssInit_multiNetIfCfgPtr;
            cpssPpPhase2.useMultiNetIfSdma = *prevCpssInit_useMultiNetIfSdmaPtr;
        }
        else
        {
            cpssPpPhase2.useMultiNetIfSdma = GT_FALSE;
        }
    }
    else if(prevCpssInit_netIfCfgPtr) /* meaning CPSS restored 'netIfCfg' but not 'useMultiNetIfSdma' */
    {
        /* the HW did not do reset ... reuse the previous init parameters */
        cpssOsMemCpy(&cpssPpPhase2.multiNetIfCfg, &cpssAppPlatformPpPhase2Db[devNum].multiNetIfCfg, sizeof(cpssPpPhase2.multiNetIfCfg));
        cpssPpPhase2.useMultiNetIfSdma = cpssAppPlatformPpPhase2Db[devNum].useMultiNetIfSdma;
    }
    else if(ppProfilePtr->useMultiNetIfSdma)
    {
        CPSS_MULTI_NET_IF_TX_SDMA_QUEUE_STC  *sdmaQueuesConfigPtr = NULL;
        CPSS_MULTI_NET_IF_RX_SDMA_QUEUE_STC  *sdmaRxQueuesConfigPtr = NULL;
        /* Tx block size calc & malloc  */
        cpssDxChHwTxDescSizeGet(devType,&descSize);
        ii=0;
        for(jj = 0; jj < CPSS_MAX_SDMA_CPU_PORTS_CNS; jj++)
        {
#ifdef CHX_FAMILY
            if(CPSS_IS_DXCH_FAMILY_MAC(devFamily))
            {
                PRV_CPSS_DXCH_NETIF_SKIP_NON_USED_NETIF_NUM_MAC(devNum, jj);
            }
#endif
            if(ii > 0 && (!PRV_CPSS_SIP_5_20_CHECK_MAC(devNum)))
            {
                break;
            }

            for(txQue = 0; txQue < 8; txQue++)
            {
                txGenMode = GT_FALSE;
                sdmaQueuesConfigPtr = &cpssPpPhase2.multiNetIfCfg.txSdmaQueuesConfig[ii][txQue];

                for(i = 0; i < ppProfilePtr->txGenQueueNum; i++)
                {
                    if((ppProfilePtr->txGenQueueList[i].sdmaPortNum == ii) &&
                       (ppProfilePtr->txGenQueueList[i].queueNum == txQue))
                    {
                        txGenMode = GT_TRUE;
                        break;
                    }
                }

                if(txGenMode == GT_FALSE)
                {
                    /* Tx block size calc & malloc  */
                    sdmaQueuesConfigPtr->queueMode = CPSS_TX_SDMA_QUEUE_MODE_NORMAL_E;
                    sdmaQueuesConfigPtr->numOfTxDesc = ppProfilePtr->txDescNum ? (ppProfilePtr->txDescNum / 8 ) : 0; /*125 */
                    sdmaQueuesConfigPtr->numOfTxBuff = 0; /*not relevant in non traffic generator mode*/
                    sdmaQueuesConfigPtr->buffSize = 0; /*not relevant in non traffic generator mode*/
                    sdmaQueuesConfigPtr->memData.staticAlloc.buffAndDescMemSize = (sdmaQueuesConfigPtr->numOfTxDesc * descSize);
                    if (sdmaQueuesConfigPtr->memData.staticAlloc.buffAndDescMemSize != 0)
                    {
                        sdmaQueuesConfigPtr->memData.staticAlloc.buffAndDescMemPtr =
                            osCacheDmaMalloc(sdmaQueuesConfigPtr->memData.staticAlloc.buffAndDescMemSize);
                        if(sdmaQueuesConfigPtr->memData.staticAlloc.buffAndDescMemPtr == NULL)
                        {
                            osCacheDmaFree(cpssPpPhase2.auqCfg.auDescBlock);
                            osCacheDmaFree(cpssPpPhase2.fuqCfg.auDescBlock);
                            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
                        }
                    }

                    sdmaQueuesConfigPtr->buffAndDescAllocMethod = CPSS_TX_BUFF_STATIC_ALLOC_E;

                    if((system_recovery.systemRecoveryState == CPSS_SYSTEM_RECOVERY_COMPLETION_STATE_E)||
                       ((system_recovery.systemRecoveryState == CPSS_SYSTEM_RECOVERY_INIT_STATE_E) &&
                       (system_recovery.systemRecoveryMode.continuousTx == GT_FALSE)) )
                    {
                        cpssOsMemSet(sdmaQueuesConfigPtr->memData.staticAlloc.buffAndDescMemPtr,0,
                                     sdmaQueuesConfigPtr->memData.staticAlloc.buffAndDescMemSize);
                    }
                }
                else
                {/* Generator mode */
                    sdmaQueuesConfigPtr->queueMode = CPSS_TX_SDMA_QUEUE_MODE_PACKET_GENERATOR_E;
                    sdmaQueuesConfigPtr->buffAndDescAllocMethod = CPSS_TX_BUFF_STATIC_ALLOC_E;
                    sdmaQueuesConfigPtr->numOfTxBuff = ppProfilePtr->txGenQueueList[i].numOfTxBuff;
                    sdmaQueuesConfigPtr->numOfTxDesc = sdmaQueuesConfigPtr->numOfTxBuff;
                    sdmaQueuesConfigPtr->buffSize = ppProfilePtr->txGenQueueList[i].txBuffSize;

                    sdmaQueuesConfigPtr->memData.staticAlloc.buffAndDescMemSize =
                        (sdmaQueuesConfigPtr->numOfTxDesc + 1) * (descSize + sdmaQueuesConfigPtr->buffSize);
                    sdmaQueuesConfigPtr->memData.staticAlloc.buffAndDescMemPtr =
                        osCacheDmaMalloc(sdmaQueuesConfigPtr->memData.staticAlloc.buffAndDescMemSize);
                    if(sdmaQueuesConfigPtr->memData.staticAlloc.buffAndDescMemPtr == NULL)
                    {
                        osCacheDmaFree(cpssPpPhase2.auqCfg.auDescBlock);
                        osCacheDmaFree(cpssPpPhase2.fuqCfg.auDescBlock);
                        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
                    }
                }
            }
            ii++;
        }

        /* Rx block size calc & malloc  */
        cpssDxChHwRxDescSizeGet(devType,&descSize);
        ii=0;
        for(jj = 0; jj < CPSS_MAX_SDMA_CPU_PORTS_CNS; jj++)
        {
#ifdef CHX_FAMILY
           if(CPSS_IS_DXCH_FAMILY_MAC(devFamily))
            {
                PRV_CPSS_DXCH_NETIF_SKIP_NON_USED_NETIF_NUM_MAC(devNum, jj);
            }
#endif

            if( ii > 0 && (!PRV_CPSS_SIP_5_20_CHECK_MAC(devNum)) )
            {
                continue;
            }
            for(rxQue = 0; rxQue < 8; rxQue++)
            {
                sdmaRxQueuesConfigPtr = &cpssPpPhase2.multiNetIfCfg.rxSdmaQueuesConfig[ii][rxQue];
                sdmaRxQueuesConfigPtr->buffAllocMethod =  ppProfilePtr->rxAllocMethod;
                sdmaRxQueuesConfigPtr->buffersInCachedMem = ppProfilePtr->rxBuffersInCachedMem;
                sdmaRxQueuesConfigPtr->numOfRxDesc = ppProfilePtr->rxDescNum/8;
                sdmaRxQueuesConfigPtr->descMemSize = sdmaRxQueuesConfigPtr->numOfRxDesc * descSize;
                sdmaRxQueuesConfigPtr->descMemPtr = osCacheDmaMalloc(sdmaRxQueuesConfigPtr->descMemSize);
                if(sdmaRxQueuesConfigPtr->descMemPtr == NULL)
                {
                    osCacheDmaFree(cpssPpPhase2.auqCfg.auDescBlock);
                    osCacheDmaFree(cpssPpPhase2.fuqCfg.auDescBlock);
                    for(txQue = 0; txQue < 8; txQue++)
                    {
                        sdmaQueuesConfigPtr = &cpssPpPhase2.multiNetIfCfg.txSdmaQueuesConfig[ii][txQue];
                        if(sdmaQueuesConfigPtr->memData.staticAlloc.buffAndDescMemSize != 0)
                        {
                            osCacheDmaFree(sdmaQueuesConfigPtr->memData.staticAlloc.buffAndDescMemPtr);
                        }
                    }
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
                }
                /* init the Rx buffer allocation method */
                /* Set the system's Rx buffer size.     */
                if((rxBufSize % rxBufAllign) != 0)
                {
                    rxBufSize = (rxBufSize + (rxBufAllign - (rxBufSize % rxBufAllign)));
                }

                sdmaRxQueuesConfigPtr->buffHeaderOffset = ppProfilePtr->rxHeaderOffset; /* give the same offset to all queues*/
                sdmaRxQueuesConfigPtr->buffSize = rxBufSize;
                sdmaRxQueuesConfigPtr->numOfRxBuff = ppProfilePtr->rxDescNum/8; /* by default the number of buffers equel the number of descriptors*/
                sdmaRxQueuesConfigPtr->memData.staticAlloc.buffMemSize = (rxBufSize * sdmaRxQueuesConfigPtr->numOfRxBuff);
                if (sdmaRxQueuesConfigPtr->memData.staticAlloc.buffMemSize != 0)
                {
                    /* If RX buffers should be cachable - allocate it from regular memory */
                    if (GT_TRUE == sdmaRxQueuesConfigPtr->buffersInCachedMem)
                    {
                        tmpPtr = osMalloc((sdmaRxQueuesConfigPtr->memData.staticAlloc.buffMemSize + rxBufAllign-1));
                    }
                    else
                    {
                        tmpPtr = osCacheDmaMalloc((sdmaRxQueuesConfigPtr->memData.staticAlloc.buffMemSize + rxBufAllign-1));
                    }

                    if(tmpPtr == NULL)
                    {
                        osCacheDmaFree(cpssPpPhase2.auqCfg.auDescBlock);
                        osCacheDmaFree(cpssPpPhase2.fuqCfg.auDescBlock);
                        for(txQue = 0; txQue < 8; txQue++)
                        {
                            sdmaQueuesConfigPtr = &cpssPpPhase2.multiNetIfCfg.txSdmaQueuesConfig[ii][txQue];
                            if(sdmaQueuesConfigPtr->memData.staticAlloc.buffAndDescMemSize != 0)
                            {
                                osCacheDmaFree(sdmaQueuesConfigPtr->memData.staticAlloc.buffAndDescMemPtr);
                            }
                        }
                        osCacheDmaFree(sdmaRxQueuesConfigPtr->descMemPtr);
                        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
                    }
                }
                else
                {
                    tmpPtr = NULL;
                }

                if((((GT_UINTPTR)tmpPtr) % rxBufAllign) != 0)
                {
                    tmpPtr = (GT_U32*)(((GT_UINTPTR)tmpPtr) +
                                       (rxBufAllign - (((GT_UINTPTR)tmpPtr) % rxBufAllign)));
                }
                sdmaRxQueuesConfigPtr->memData.staticAlloc.buffMemPtr = tmpPtr;

                if((system_recovery.systemRecoveryState == CPSS_SYSTEM_RECOVERY_COMPLETION_STATE_E)||
                   ( (system_recovery.systemRecoveryState == CPSS_SYSTEM_RECOVERY_INIT_STATE_E) &&
                     (system_recovery.systemRecoveryMode.continuousRx == GT_FALSE)) )
                {
                    cpssOsMemSet(sdmaRxQueuesConfigPtr->memData.staticAlloc.buffMemPtr,0,
                         sdmaRxQueuesConfigPtr->memData.staticAlloc.buffMemSize);
                }
            }
            ii++;
        }
    }

    rc = cpssDxChHwPpPhase2Init(devNum, &cpssPpPhase2);
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChHwPpPhase2Init);

    cpssOsMemCpy(&(cpssAppPlatformPpPhase2Db[devNum]), &cpssPpPhase2, sizeof(cpssPpPhase2));

    /* In order to configure HW device ID different from SW device ID the following */
    /* logic is used: HW_device_ID = (SW_device_ID + appDemoHwDevNumOffset) modulo 32 */
    /* (this insures different HW and SW device IDs since the HW device ID is 5 */
    /* bits length). */
    if (PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum))
    {
        hwDevNum = ((devNum + appRefHwDevNumOffset) & 0x3FF);
    }
    else
    {
        hwDevNum = ((devNum + appRefHwDevNumOffset) & 0x1F);
    }

    /* set HWdevNum related values */
    rc = prvCpssDxChHwDevNumChange(devNum,hwDevNum);
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, prvCpssDxChHwDevNumChange);

    return rc;
}

/*******************************************************************************
 * CPSS modules initialization routines
 ******************************************************************************/

static GT_STATUS prvPhyLibInit
(
    IN GT_U8 devNum
)
{
    GT_STATUS rc = GT_OK;

    rc = cpssDxChPhyPortSmiInit(devNum);
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChPhyPortSmiInit);

    return rc;
}

#define CHEETAH_CPU_PORT_PROFILE                   CPSS_PORT_RX_FC_PROFILE_1_E
#define CHEETAH_NET_GE_PORT_PROFILE                CPSS_PORT_RX_FC_PROFILE_2_E
#define CHEETAH_NET_10GE_PORT_PROFILE              CPSS_PORT_RX_FC_PROFILE_3_E
#define CHEETAH_CASCADING_PORT_PROFILE             CPSS_PORT_RX_FC_PROFILE_4_E

#define CHEETAH_GE_PORT_XON_DEFAULT                14 /* 28 Xon buffs per port   */
#define CHEETAH_GE_PORT_XOFF_DEFAULT               35 /* 70 Xoff buffs per port  */
#define CHEETAH_GE_PORT_RX_BUFF_LIMIT_DEFAULT      25 /* 100 buffers per port    */

#define CHEETAH_CPU_PORT_XON_DEFAULT               14 /* 28 Xon buffs per port   */
#define CHEETAH_CPU_PORT_XOFF_DEFAULT              35 /* 70 Xoff buffs per port  */
#define CHEETAH_CPU_PORT_RX_BUFF_LIMIT_DEFAULT     25 /* 100 buffers for CPU port */

#define CHEETAH_XG_PORT_XON_DEFAULT                25 /* 50 Xon buffs per port   */
#define CHEETAH_XG_PORT_XOFF_DEFAULT               85 /* 170 Xoff buffs per port */
#define CHEETAH_XG_PORT_RX_BUFF_LIMIT_DEFAULT      56 /* 224 buffers per port    */

static GT_STATUS prvPortLibInit
(
    IN GT_U8                             devNum,
    IN CPSS_APP_PLATFORM_PP_PROFILE_STC *ppProfilePtr
)
{
    GT_STATUS rc = GT_OK;
    CPSS_DXCH_PORT_HOL_FC_ENT modeFcHol = CPSS_DXCH_PORT_FC_E;

    rc = cpssDxChPortStatInit(devNum);
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChPortStatInit);

    /* Enable HOL system mode for revision 3 in DxCh2, DxCh3, XCAT. */
    if(ppProfilePtr->flowControlDisable)
    {
        modeFcHol = CPSS_DXCH_PORT_HOL_E;
    }

    if(ppProfilePtr->modeFcHol)
    {
        rc = cpssDxChPortFcHolSysModeSet(devNum, modeFcHol);
        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChPortFcHolSysModeSet);
    }

    rc = cpssDxChPortTxInit(devNum);
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChPortTxInit);

    return rc;
}

static GT_STATUS dxChBrgFdbInit
(
    IN GT_U8 dev
)
{
    GT_STATUS                   rc = GT_OK;
    GT_HW_DEV_NUM               hwDev;    /* HW device number */
    CPSS_MAC_HASH_FUNC_MODE_ENT hashMode;
    GT_U32                      maxLookupLen;

    rc = cpssDxChBrgFdbInit(dev);
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChBrgFdbInit);

    /* sip6 not supports any more the xor/crc , supports only the multi-hash */
    hashMode = (!PRV_CPSS_SIP_6_CHECK_MAC(dev)) ?
                CPSS_MAC_HASH_FUNC_XOR_E :
                CPSS_MAC_HASH_FUNC_CRC_MULTI_HASH_E;
    maxLookupLen = 4;

    /* Set lookUp mode and lookup length. */
    rc = cpssDxChBrgFdbHashModeSet(dev, hashMode);
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChBrgFdbHashModeSet);

    /* NOTE : in sip6 calling this API in multi-hash mode is irrelevant
       as we not modify the value in the HW , and keep it 0 .

       the  API will fail if maxLookupLen != fdbHashParams.numOfBanks
    */
    if(!PRV_CPSS_SIP_6_CHECK_MAC(dev))
    {
        rc = cpssDxChBrgFdbMaxLookupLenSet(dev, maxLookupLen);
        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChBrgFdbMaxLookupLenSet);
    }

    /******************************/
    /* do specific cheetah coding */
    /******************************/

    /* the trunk entries registered according to : macEntryPtr->dstInterface.hwDevNum
       that is to support the "renumbering" feature , but the next configuration
       should not effect the behavior on other systems that not use a
       renumbering ..
    */
    /* age trunk entries on a device that registered from all devices
       since we registered the trunk entries on device macEntryPtr->dstInterface.hwDevNum
       that may differ from "own device"
       (and auto learn set it on "own device" */
    /* Set Action Active Device Mask and Action Active Device. This is needed
       in order to enable aging only on own device.  */
    /*
       BTW : the multicast entries are registered on "own device" (implicitly by the CPSS)
        (so will require renumber for systems that needs renumber)
    */

    rc = cpssDxChCfgHwDevNumGet(dev, &hwDev);
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChCfgHwDevNumGet);

    rc = cpssDxChBrgFdbActionActiveDevSet(dev, hwDev, 0x1F);
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChBrgFdbActionActiveDevSet);

    rc = cpssDxChBrgFdbAgeOutAllDevOnTrunkEnable(dev, GT_TRUE);
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChBrgFdbAgeOutAllDevOnTrunkEnable);

    rc = cpssDxChBrgFdbAgeOutAllDevOnNonTrunkEnable(dev, GT_FALSE);
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChBrgFdbAgeOutAllDevOnNonTrunkEnable);

    return rc;
}

static GT_STATUS prvBridgeLibInit
(
    IN GT_U8                             devNum,
    IN CPSS_APP_PLATFORM_PP_PROFILE_STC *ppProfilePtr
)
{
    GT_STATUS rc = GT_OK;
    GT_U32  numOfPhysicalPorts;
    GT_U32  numOfEports;
    GT_U32  portNum;
    GT_U32  i;
    GT_HW_DEV_NUM hwDevNum;
    GT_U32        stpEntry[CPSS_DXCH_STG_ENTRY_SIZE_CNS];
    GT_BOOL                             isCpu;
    /* allow processing of AA messages */
    /*
    applicationPlatformSysConfig.supportAaMessage = GT_TRUE;
    */

    /* Init VLAN */
    rc = cpssDxChBrgVlanInit(devNum);
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChBrgVlanInit);

    /** STP **/
    rc = cpssDxChBrgStpInit(devNum);
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChBrgStpInit);

    rc = dxChBrgFdbInit(devNum);
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, dxChBrgFdbInit);

    rc = cpssDxChBrgMcInit(devNum);
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChBrgMcInit);

    /* set first entry in STP like default entry */
    cpssOsMemSet(stpEntry, 0, sizeof(stpEntry));
    rc = cpssDxChBrgStpEntryWrite(devNum, 0, stpEntry);
    if( GT_OK != rc)
    {
        /* the device not support STP !!! --> it's ok ,we have those ... */
        CPSS_APP_PLATFORM_LOG_ERR_MAC("cpssDxChBrgStpEntryWrite : device[%d] not supported \n",devNum);
        rc = GT_OK;
    }

    if (! PRV_CPSS_SIP_5_CHECK_MAC(devNum))
    {
        return GT_OK;
    }

    if(PRV_CPSS_SIP_5_10_CHECK_MAC(devNum))
    {
        /* TBD: FE HA-3259 fix and removed from CPSS.
           Allow to the CPU to get the original vlan tag as payload after
           the DSA tag , so the info is not changed. */
        rc = cpssDxChBrgVlanForceNewDsaToCpuEnableSet(devNum, GT_TRUE);
        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChBrgVlanForceNewDsaToCpuEnableSet);

        /* set the command of 'SA static moved' to be 'forward' because this command
           applied also on non security breach event ! */
        rc = cpssDxChBrgSecurBreachEventPacketCommandSet(devNum,
            CPSS_BRG_SECUR_BREACH_EVENTS_MOVED_STATIC_E,
            CPSS_PACKET_CMD_FORWARD_E);
        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChBrgSecurBreachEventPacketCommandSet);
    }

    rc = cpssDxChCfgHwDevNumGet(devNum, &hwDevNum);
    if (rc != GT_OK)
        return rc;

    /* Enable configuration of drop for ARP MAC SA mismatch due to check per port */
    /* Loop on the first 256 (num of physical ports , and CPU port (63)) entries
       of the table */
    numOfPhysicalPorts = PRV_CPSS_DXCH_MAX_PHY_PORT_NUMBER_MAC(devNum);

    for(portNum=0; portNum < numOfPhysicalPorts; portNum++)
    {
        /* ARP MAC SA mismatch check per port configuration enabling */
        rc = cpssDxChBrgGenPortArpMacSaMismatchDropEnable(devNum,
                                                          portNum,
                                                          GT_TRUE);
        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChBrgGenPortArpMacSaMismatchDropEnable);

        /* this code can be restored after link up/ lind down EGF WA is implemented */
        if(PRV_CPSS_SIP_5_20_CHECK_MAC(devNum) && !cpssAppPlatformIsPortMgrPort(devNum))
        {
            /* set the EGF to filter traffic to ports that are 'link down'.
               state that all ports are currently 'link down' (except for 'CPU PORT')

               see function: sip5_20_linkChange , which handles runtime 'link change' event.
            */
            rc = cpssDxChPortPhysicalPortMapIsCpuGet(devNum, portNum, &isCpu);
            CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChPortPhysicalPortMapIsCpuGet);
            rc = cpssDxChBrgEgrFltPortLinkEnableSet(devNum,
                                                    portNum,
                                                    (isCpu==GT_FALSE) ?
                                                    CPSS_DXCH_BRG_EGR_FLT_PORT_LINK_STATUS_FORCE_LINK_DOWN_E: /* FIlter non CPU port*/
                                                    CPSS_DXCH_BRG_EGR_FLT_PORT_LINK_STATUS_FORCE_LINK_UP_E  /*don't filter*/);
            CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChBrgEgrFltPortLinkEnableSet);
        }
    }

    /* Port Isolation is enabled if all three configurations are enabled:
       In the global TxQ registers, AND
       In the eVLAN egress entry, AND
       In the ePort entry

       For legacy purpose loop on all ePort and Trigger L2 & L3 Port
       Isolation filter for all ePorts */

    numOfEports = PRV_CPSS_DXCH_MAX_PORT_NUMBER_MAC(devNum);

    for(portNum=0; portNum < numOfEports; portNum++)
    {
        rc = cpssDxChNstPortIsolationModeSet(devNum,
                                             portNum,
                                             CPSS_DXCH_NST_PORT_ISOLATION_ALL_ENABLE_E);
        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChNstPortIsolationModeSet);

        /* for legacy : enable per eport <Egress eVLAN Filtering Enable>
           because Until today there was no enable bit, egress VLAN filtering is
           always performed, subject to the global <BridgedUcEgressFilterEn>. */
        rc = cpssDxChBrgEgrFltVlanPortFilteringEnableSet(devNum,
                                                          portNum,
                                                          GT_TRUE);
        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChBrgEgrFltVlanPortFilteringEnableSet);
    }

    /* Flow Control Initializations */
    for(portNum = 0; portNum < numOfPhysicalPorts; portNum++)
    {
        CPSS_APPLICATION_PORT_SKIP_CHECK(devNum, portNum);

        if(prvCpssDxChPortRemotePortCheck(devNum,portNum))
        {
            /* There is no supported way of setting MAC SA Lsb on remote ports */
            continue;
        }
        rc = cpssDxChPortMacSaLsbSet(devNum, portNum, (GT_U8)portNum);
        if (GT_OK != rc)
           CPSS_APP_PLATFORM_LOG_ERR_MAC("cpssDxChPortMacSaLsbSet rc=%d", rc);
    }

    if(ppProfilePtr->policerMruSupported)
    {
        /* PLR MRU : needed for bobk that hold default different then our tests expect */
        /* NOTE: for bobk it is not good value for packets > (10K/8) bytes */
        for(i = CPSS_DXCH_POLICER_STAGE_INGRESS_0_E;
            i <= CPSS_DXCH_POLICER_STAGE_EGRESS_E ;
            i++)
        {
            rc = cpssDxCh3PolicerMruSet(devNum,i,_10K);
            if (GT_OK != rc)
               CPSS_APP_PLATFORM_LOG_ERR_MAC("cpssDxCh3PolicerMruSet rc=%d", rc);
        }
    }

    if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        /* init Exact Match DB */
        rc = prvCpssDxChExactMatchDbInit(devNum);
        if (GT_OK != rc)
           CPSS_APP_PLATFORM_LOG_ERR_MAC("prvCpssDxChExactMatchDbInit rc=%d", rc);
    }

    return rc;
}

static GT_STATUS prvNetIfLibInit
(
    IN GT_U8                             devNum,
    IN CPSS_APP_PLATFORM_PP_PROFILE_STC *ppProfilePtr
)
{
    GT_STATUS rc = GT_OK;
    GT_U32 i;

    if (ppProfilePtr->cpuPortMode == CPSS_NET_CPU_PORT_MODE_SDMA_E)
    {
        rc = cpssDxChNetIfInit(devNum);
        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChNetIfInit);

        /* When CPSS_RX_BUFF_NO_ALLOC_E method is used application is responsible for
         * RX buffer allocation and attachment to descriptors.*/
    }
    else if (ppProfilePtr->cpuPortMode == CPSS_NET_CPU_PORT_MODE_MII_E)
    {
        CPSS_DXCH_NETIF_MII_INIT_STC miiInit;
        miiInit.numOfTxDesc = ppProfilePtr->miiTxDescNum;
        miiInit.txInternalBufBlockSize = ppProfilePtr->miiTxBufBlockSize;
        miiInit.txInternalBufBlockPtr = cpssOsCacheDmaMalloc(miiInit.txInternalBufBlockSize);
        if (miiInit.txInternalBufBlockPtr == NULL && miiInit.txInternalBufBlockSize != 0)
        {
            CPSS_APP_PLATFORM_LOG_ERR_MAC("cpssOsCacheDmaMalloc failed\r\n");
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
        }
        for (i = 0; i < CPSS_MAX_RX_QUEUE_CNS; i++)
        {
            miiInit.bufferPercentage[i] = ppProfilePtr->miiRxBufferPercentage[i];
        }
        miiInit.rxBufSize = ppProfilePtr->miiRxBufSize;
        miiInit.headerOffset = ppProfilePtr->miiRxHeaderOffset;
        miiInit.rxBufBlockSize = ppProfilePtr->miiRxBufBlockSize;
        miiInit.rxBufBlockPtr = cpssOsCacheDmaMalloc(miiInit.rxBufBlockSize);
        if (miiInit.rxBufBlockPtr == NULL && miiInit.rxBufBlockSize != 0)
        {
            CPSS_APP_PLATFORM_LOG_ERR_MAC("cpssOsCacheDmaMalloc failed\r\n");
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
        }
        if (miiInit.rxBufBlockPtr != NULL)
        {
            rc = cpssDxChNetIfMiiInit(devNum,&miiInit);
            CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChNetIfMiiInit);
        }
        else
        {
            rc = GT_OK;
        }
    }

    return rc;
}

static GT_STATUS prvMirrorLibInit
(
    IN GT_U8                             devNum,
    IN CPSS_APP_PLATFORM_PP_PROFILE_STC *ppProfilePtr
)
{
    GT_STATUS rc = GT_OK;
    GT_HW_DEV_NUM     hwDev; /* HW device number */
    CPSS_DXCH_MIRROR_ANALYZER_INTERFACE_STC   interface;

    /* The following mirroring settings are needed because several RDE tests */
    /* assume that that default HW values of analyzers is port 0 SW device ID 0. */
    if( 0 == devNum )
    {
        rc = cpssDxChCfgHwDevNumGet(0, &hwDev);
        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChCfgHwDevNumGet);
        interface.interface.type = CPSS_INTERFACE_PORT_E;
        interface.interface.devPort.hwDevNum = hwDev;
        interface.interface.devPort.portNum = ppProfilePtr->mirrorAnalyzerPortNum;

        rc = cpssDxChMirrorAnalyzerInterfaceSet(devNum, 0, &interface);
        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChMirrorAnalyzerInterfaceSet);

        rc = cpssDxChMirrorAnalyzerInterfaceSet(devNum, 1, &interface);
        if (GT_OK != rc)
           CPSS_APP_PLATFORM_LOG_ERR_MAC("cpssDxChMirrorAnalyzerInterfaceSet ret=%d\r\n", rc);
    }
    return rc;
}

static GT_STATUS prvPclLibInit
(
    IN GT_U8 devNum
)
{
    GT_STATUS rc = GT_OK;

    rc = cpssDxChPclInit(devNum);
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChPclInit);

    rc = cpssDxChPclIngressPolicyEnable(devNum, GT_TRUE);
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChPclIngressPolicyEnable);

    return rc;
}

/* indication of bc2 b0 */
static GT_U32 isBobcat2B0 = 0;

#define GROUP_0             0
#define GROUP_1             1
#define GROUP_2             2
#define GROUP_3             3
#define GROUP_4             4

#define HIT_NUM_0           0
#define HIT_NUM_1           1
#define HIT_NUM_2           2
#define HIT_NUM_3           3
#define END_OF_LIST_MAC     0xFFFFFFFF

typedef struct{
    GT_U32  floorNum;/*if END_OF_LIST_MAC --> not valid */
    GT_U32  bankIndex;
    GT_U32  hitNum;
}BANK_PARTITION_INFO_STC;

/* save TCAM 'Sections' (base+size) for TTI and IPCL/EPLC */
GT_VOID applicationPlatformDxChTcamSectionsSave(GT_VOID)
{
    save_applicationPlatformTcamTtiHit0RuleBaseIndexOffset  = applicationPlatformTcamTtiHit0RuleBaseIndexOffset;
    save_applicationPlatformTcamTtiHit1RuleBaseIndexOffset  = applicationPlatformTcamTtiHit1RuleBaseIndexOffset;
    save_applicationPlatformTcamTtiHit2RuleBaseIndexOffset  = applicationPlatformTcamTtiHit2RuleBaseIndexOffset;
    save_applicationPlatformTcamTtiHit3RuleBaseIndexOffset  = applicationPlatformTcamTtiHit3RuleBaseIndexOffset;

    save_applicationPlatformTcamTtiHit0MaxNum               = applicationPlatformTcamTtiHit0MaxNum;
    save_applicationPlatformTcamTtiHit1MaxNum               = applicationPlatformTcamTtiHit1MaxNum;
    save_applicationPlatformTcamTtiHit2MaxNum               = applicationPlatformTcamTtiHit2MaxNum;
    save_applicationPlatformTcamTtiHit3MaxNum               = applicationPlatformTcamTtiHit3MaxNum;

    save_applicationPlatformTcamIpcl0RuleBaseIndexOffset    = applicationPlatformTcamIpcl0RuleBaseIndexOffset;
    save_applicationPlatformTcamIpcl1RuleBaseIndexOffset    = applicationPlatformTcamIpcl1RuleBaseIndexOffset;
    save_applicationPlatformTcamIpcl2RuleBaseIndexOffset    = applicationPlatformTcamIpcl2RuleBaseIndexOffset;
    save_applicationPlatformTcamEpclRuleBaseIndexOffset     = applicationPlatformTcamEpclRuleBaseIndexOffset;

    save_applicationPlatformTcamIpcl0MaxNum                 = applicationPlatformTcamIpcl0MaxNum;
    save_applicationPlatformTcamIpcl1MaxNum                 = applicationPlatformTcamIpcl1MaxNum;
    save_applicationPlatformTcamIpcl2MaxNum                 = applicationPlatformTcamIpcl2MaxNum;
    save_applicationPlatformTcamEpclMaxNum                  = applicationPlatformTcamEpclMaxNum;

    return;
}

static GT_STATUS prvTcamLibInit
(
    IN  GT_U8                       devNum,
    IN CPSS_APP_PLATFORM_PP_PROFILE_STC  *ppProfilePtr
)
{
    GT_STATUS rc;
    GT_U32 value;
    CPSS_DXCH_TCAM_BLOCK_INFO_STC          floorInfoArr[CPSS_DXCH_TCAM_MAX_NUM_BLOCKS_CNS];
    GT_U32 baseFloorForTtiLookup0 = 0;
    GT_U32 baseFloorForTtiLookup1 = 0;
    GT_U32 baseFloorForTtiLookup2 = 0;
    GT_U32 baseFloorForTtiLookup3 = 0;
    GT_U32 tcamFloorsNum;
    GT_U32  ii,jj,index;
    GT_U32 tcamEntriesNum;     /* number of entries for TTI in TCAM */
    GT_U32 tcamFloorEntriesNum;/* number of entries for TTI in TCAM floor */

    static BANK_PARTITION_INFO_STC  bc2A0_ttiLookupArr[] = {
                    {6,0                                 ,HIT_NUM_0},
                    {6,CPSS_DXCH_TCAM_MAX_NUM_BANKS_CNS/2,HIT_NUM_0},

                    {7,0                                 ,HIT_NUM_0},
                    {7,CPSS_DXCH_TCAM_MAX_NUM_BANKS_CNS/2,HIT_NUM_0},

                    {8,0                                 ,HIT_NUM_0},
                    {8,CPSS_DXCH_TCAM_MAX_NUM_BANKS_CNS/2,HIT_NUM_0},

                    {9,0                                 ,HIT_NUM_0},
                    {9,CPSS_DXCH_TCAM_MAX_NUM_BANKS_CNS/2,HIT_NUM_0},

                    {10,0                                 ,HIT_NUM_1},
                    {10,CPSS_DXCH_TCAM_MAX_NUM_BANKS_CNS/2,HIT_NUM_1},

                    {11,0                                 ,HIT_NUM_1},
                    {11,CPSS_DXCH_TCAM_MAX_NUM_BANKS_CNS/2,HIT_NUM_1},

                    {END_OF_LIST_MAC,END_OF_LIST_MAC,END_OF_LIST_MAC}
                    };

    static BANK_PARTITION_INFO_STC  bc2B0_ttiLookupArr[] = {
                    {6,0                                 ,HIT_NUM_0},
                    {6,CPSS_DXCH_TCAM_MAX_NUM_BANKS_CNS/2,HIT_NUM_0},

                    {7,0                                 ,HIT_NUM_0},
                    {7,CPSS_DXCH_TCAM_MAX_NUM_BANKS_CNS/2,HIT_NUM_0},

                    {8,0                                 ,HIT_NUM_3},
                    {8,CPSS_DXCH_TCAM_MAX_NUM_BANKS_CNS/2,HIT_NUM_3},

                    {9,0                                 ,HIT_NUM_2},
                    {9,CPSS_DXCH_TCAM_MAX_NUM_BANKS_CNS/2,HIT_NUM_2},

                    {10,0                                 ,HIT_NUM_1},
                    {10,CPSS_DXCH_TCAM_MAX_NUM_BANKS_CNS/2,HIT_NUM_1},

                    {11,0                                 ,HIT_NUM_1},
                    {11,CPSS_DXCH_TCAM_MAX_NUM_BANKS_CNS/2,HIT_NUM_1},

                    {END_OF_LIST_MAC,END_OF_LIST_MAC,END_OF_LIST_MAC}
                    };

    static BANK_PARTITION_INFO_STC  bobk_ttiLookupArr[] = {
                    {3,0                                 ,HIT_NUM_0},
                    {3,CPSS_DXCH_TCAM_MAX_NUM_BANKS_CNS/2,HIT_NUM_0},

                    {4,0                                 ,HIT_NUM_2},
                    {4,CPSS_DXCH_TCAM_MAX_NUM_BANKS_CNS/2,HIT_NUM_3},

                    {5,0                                 ,HIT_NUM_1},
                    {5,CPSS_DXCH_TCAM_MAX_NUM_BANKS_CNS/2,HIT_NUM_1},

                    {END_OF_LIST_MAC,END_OF_LIST_MAC,END_OF_LIST_MAC}
                    };

    BANK_PARTITION_INFO_STC *ttiLookupInfoPtr = NULL;
    GT_BOOL ttiLookup3FromMidFloor = GT_FALSE;
    GT_BOOL ttiLookup0NonStandard = GT_FALSE;

    (void)ppProfilePtr;
    tcamFloorEntriesNum =
        CPSS_DXCH_TCAM_MAX_NUM_BANKS_CNS * CPSS_DXCH_TCAM_MAX_NUM_RULES_PER_BANKS_CNS;

    rc = cpssDxChCfgTableNumEntriesGet(
        devNum, CPSS_DXCH_CFG_TABLE_TTI_TCAM_E, &tcamEntriesNum);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    /* value must be a multiple of floor size */
    if (((tcamEntriesNum % tcamFloorEntriesNum) != 0) ||
        (tcamEntriesNum == 0) ||
        (tcamEntriesNum > (CPSS_DXCH_TCAM_MAX_NUM_FLOORS_CNS * tcamFloorEntriesNum)))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    if(PRV_CPSS_SIP_5_10_CHECK_MAC(devNum))
    {
        isBobcat2B0 = 1;

        if(PRV_CPSS_SIP_5_15_CHECK_MAC(devNum))
        {
            ttiLookupInfoPtr = bobk_ttiLookupArr;
        }
        else
        {
            if ((tcamEntriesNum / tcamFloorEntriesNum) >= 12)
            {
                ttiLookupInfoPtr = bc2B0_ttiLookupArr;
            }
            else
            {
                /* Drake with 6-floor TCAM */
                ttiLookupInfoPtr = bobk_ttiLookupArr;
            }
        }
    }
    else
    {
        ttiLookupInfoPtr = bc2A0_ttiLookupArr;
    }

    /* init TCAM - Divide the TCAM into 2 groups:
       ingress policy 0, ingress policy 1, ingress policy 2 and egress policy belong to group 0; using floors 0-5.
       client tunnel termination belong to group 1; using floor 6-11.
       applicationPlatformDbEntryAdd can change the division such that 6 will be replaced by a different value. */
    rc = cpssDxChTcamPortGroupClientGroupSet(devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,CPSS_DXCH_TCAM_IPCL_0_E,GROUP_0,GT_TRUE);
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChTcamPortGroupClientGroupSet);

    rc = cpssDxChTcamPortGroupClientGroupSet(devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,CPSS_DXCH_TCAM_IPCL_1_E,GROUP_0,GT_TRUE);
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChTcamPortGroupClientGroupSet);

    if (PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.pcl.iPcl0Bypass == GT_FALSE)
    {
        rc = cpssDxChTcamPortGroupClientGroupSet(devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,CPSS_DXCH_TCAM_IPCL_2_E,GROUP_0,GT_TRUE);
        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChTcamPortGroupClientGroupSet);
    }

    rc = cpssDxChTcamPortGroupClientGroupSet(devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,CPSS_DXCH_TCAM_EPCL_E,GROUP_0,GT_TRUE);
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChTcamPortGroupClientGroupSet);

    rc = cpssDxChTcamPortGroupClientGroupSet(devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,CPSS_DXCH_TCAM_TTI_E,GROUP_1,GT_TRUE);
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChTcamPortGroupClientGroupSet);

    tcamFloorsNum = (tcamEntriesNum / tcamFloorEntriesNum);
    if(tcamFloorsNum == 3 && PRV_CPSS_SIP_5_15_CHECK_MAC(devNum))
    {
        /* floor 0 for PCL clients
           All clients are connected to hit num 0 */
        for (ii = 0; ii < CPSS_DXCH_TCAM_MAX_NUM_BLOCKS_CNS; ii++)
        {
            floorInfoArr[ii].group  = GROUP_0;
            floorInfoArr[ii].hitNum = HIT_NUM_0;
        }

        rc = cpssDxChTcamIndexRangeHitNumAndGroupSet(devNum, 0,floorInfoArr);
        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChTcamIndexRangeHitNumAndGroupSet);

        /* floor 1: used by TTI_0 and TTI_1 */
        for (ii = 0; ii < CPSS_DXCH_TCAM_MAX_NUM_BLOCKS_CNS; ii++)
        {
            floorInfoArr[ii].group  = GROUP_1;
            floorInfoArr[ii].hitNum = (ii < 3) ? HIT_NUM_0 : HIT_NUM_1;
        }

        rc = cpssDxChTcamIndexRangeHitNumAndGroupSet(devNum, 1,floorInfoArr);
        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChTcamIndexRangeHitNumAndGroupSet);

        /* floor 2: used by TTI_2 and TTI_3 */
        for (ii = 0; ii < CPSS_DXCH_TCAM_MAX_NUM_BLOCKS_CNS; ii++)
        {
            floorInfoArr[ii].group  = GROUP_1;
            floorInfoArr[ii].hitNum = (ii < 3) ? HIT_NUM_2 : HIT_NUM_3;
        }

        rc = cpssDxChTcamIndexRangeHitNumAndGroupSet(devNum, 2,floorInfoArr);
        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChTcamIndexRangeHitNumAndGroupSet);

        applicationPlatformTcamTtiHit0RuleBaseIndexOffset = tcamFloorEntriesNum;
        applicationPlatformTcamTtiHit1RuleBaseIndexOffset = applicationPlatformTcamTtiHit0RuleBaseIndexOffset + 6;
        applicationPlatformTcamTtiHit2RuleBaseIndexOffset = tcamFloorEntriesNum * 2;
        applicationPlatformTcamTtiHit3RuleBaseIndexOffset = applicationPlatformTcamTtiHit2RuleBaseIndexOffset + 6;
        applicationPlatformTcamTtiHit0MaxNum =
        applicationPlatformTcamTtiHit1MaxNum =
        applicationPlatformTcamTtiHit2MaxNum =
        applicationPlatformTcamTtiHit3MaxNum = CPSS_DXCH_TCAM_MAX_NUM_RULES_PER_HALF_FLOOR_CNS;
    }
    else
    {
        baseFloorForTtiLookup0 = ((tcamFloorsNum + 1) / 2);
        baseFloorForTtiLookup1 = (tcamFloorsNum - 1);
        if (baseFloorForTtiLookup1 > 10)
        {
            baseFloorForTtiLookup1 = 10;
        }

        if(isBobcat2B0)
        {
            if ((tcamFloorsNum/2) < 4) /* support bobk and some bc2 flavors */
            {
                /* we can not provide floor for each lookup TTI 0,1,2,3 */
                switch (tcamFloorsNum/2)
                {
                    case 2:
                        /* need to allow support for 'half floor' and not full floor */
                        CPSS_TBD_BOOKMARK_BOBCAT2_BOBK
                        /* lookup 0,2 on floor 0 */
                        /* lookup 1,3 on floor 1 */
                        baseFloorForTtiLookup1 = baseFloorForTtiLookup0 + 1;
                        baseFloorForTtiLookup2 = baseFloorForTtiLookup0;
                        baseFloorForTtiLookup3 = baseFloorForTtiLookup1;
                        break;
                    case 1:
                        /* need to allow support for 'half floor' and not full floor */
                        /* and probably to allow only 2 lookups and not 4 ! */
                        CPSS_TBD_BOOKMARK_BOBCAT2_BOBK
                        /* lookup 0,1,2,3 on floor 0 */
                        baseFloorForTtiLookup1 = baseFloorForTtiLookup0;
                        baseFloorForTtiLookup2 = baseFloorForTtiLookup0;
                        baseFloorForTtiLookup3 = baseFloorForTtiLookup0;
                        break;
                    case 3:
                        /* need to allow support for 'half floor' and not full floor */
                        CPSS_TBD_BOOKMARK_BOBCAT2
                        /* lookup 0   on floor 0 */
                        /* lookup 2,3 on floor 1 */
                        /* lookup 1   on floor 2 */
                        baseFloorForTtiLookup1 = baseFloorForTtiLookup0 + 2;
                        baseFloorForTtiLookup2 = baseFloorForTtiLookup0 + 1;
                        baseFloorForTtiLookup3 = baseFloorForTtiLookup2;
                        ttiLookup3FromMidFloor = GT_TRUE;/* lookup 3 from mid floor */
                        break;
                    default:  /*0*/
                        /* should not happen*/
                        break;
                }
            }
            else
            {
                baseFloorForTtiLookup2 = (baseFloorForTtiLookup1 - 1);
                baseFloorForTtiLookup3 = (baseFloorForTtiLookup1 - 2);
            }

            applicationPlatformTcamTtiHit2RuleBaseIndexOffset = (baseFloorForTtiLookup2 * tcamFloorEntriesNum);
            applicationPlatformTcamTtiHit3RuleBaseIndexOffset = (baseFloorForTtiLookup3 * tcamFloorEntriesNum);
            if(ttiLookup3FromMidFloor == GT_TRUE)
            {
                applicationPlatformTcamTtiHit3RuleBaseIndexOffset += (CPSS_DXCH_TCAM_MAX_NUM_BANKS_CNS / 2);/*6*/
            }

        }

        applicationPlatformTcamTtiHit0RuleBaseIndexOffset = (baseFloorForTtiLookup0 * tcamFloorEntriesNum);
        applicationPlatformTcamTtiHit1RuleBaseIndexOffset = (baseFloorForTtiLookup1 * tcamFloorEntriesNum);

        for (ii = 0; ii < CPSS_DXCH_TCAM_MAX_NUM_BLOCKS_CNS; ii++)
        {
            floorInfoArr[ii].group  = GROUP_0;
            floorInfoArr[ii].hitNum = HIT_NUM_0;
        }

        /* PCL : All clients are connected to hit num 0 */
        for (value = 0; value < baseFloorForTtiLookup0; value++){
            rc = cpssDxChTcamIndexRangeHitNumAndGroupSet(devNum,value,floorInfoArr);
            CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChTcamIndexRangeHitNumAndGroupSet);
        }

        for (ii = 0; ii < CPSS_DXCH_TCAM_MAX_NUM_BLOCKS_CNS; ii++)
        {
            floorInfoArr[ii].group = GROUP_1;
        }

        applicationPlatformTcamTtiHit0MaxNum = 0;
        applicationPlatformTcamTtiHit1MaxNum = 0;
        applicationPlatformTcamTtiHit2MaxNum = 0;
        applicationPlatformTcamTtiHit3MaxNum = 0;

        /* TTI : All clients are connected to hit num 0..3 */
        for(ii = 0 ; ttiLookupInfoPtr[ii].floorNum != END_OF_LIST_MAC ; ii += 2)
        {
            value = ttiLookupInfoPtr[ii+0].floorNum;

            if(value >= tcamFloorsNum)
            {
                /* ignore */
                continue;
            }
            if (PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.tcam.numBanksForHitNumGranularity == 2)
            {
                floorInfoArr[0].hitNum=ttiLookupInfoPtr[ii+0].hitNum;
                floorInfoArr[1].hitNum=ttiLookupInfoPtr[ii+1].hitNum;
            }
            else
            {
                floorInfoArr[0].hitNum=ttiLookupInfoPtr[ii+0].hitNum;
                floorInfoArr[1].hitNum=ttiLookupInfoPtr[ii+0].hitNum;
                floorInfoArr[2].hitNum=ttiLookupInfoPtr[ii+0].hitNum;
                floorInfoArr[3].hitNum=ttiLookupInfoPtr[ii+1].hitNum;
                floorInfoArr[4].hitNum=ttiLookupInfoPtr[ii+1].hitNum;
                floorInfoArr[5].hitNum=ttiLookupInfoPtr[ii+1].hitNum;
            }

            /* support case that the baseFloorForTtiLookup0 was set by 'applicationPlatformDbEntryGet'
               to value > 6 */
            if(baseFloorForTtiLookup0 > ttiLookupInfoPtr[0].floorNum)
            {
                if(baseFloorForTtiLookup0 > value/*current floor*/)
                {
                    /* this floor is part of the 'PCL' */
                    continue;
                }
                else if(baseFloorForTtiLookup0 == value)
                {
                    /* use it for lookup 0 */
                    for (jj=0; jj<CPSS_DXCH_TCAM_MAX_NUM_BLOCKS_CNS; jj++) {
                        floorInfoArr[jj].hitNum = HIT_NUM_0;
                    }
                }
            }

            /* calculate the number of entries that each lookup can use */
            for (jj = 0 ; jj < 2 ; jj++)
            {
                if (PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.tcam.numBanksForHitNumGranularity == 2)
                {
                    index = jj;
                }
                else
                {
                    index = 3*jj;
                }
                if (floorInfoArr[index].hitNum == HIT_NUM_0)
                {
                    /* another half floor for lookup 0 */
                    applicationPlatformTcamTtiHit0MaxNum += tcamFloorEntriesNum / 2;
                }
                else
                if(floorInfoArr[index].hitNum == HIT_NUM_1)
                {
                    /* another half floor for lookup 1 */
                    applicationPlatformTcamTtiHit1MaxNum += tcamFloorEntriesNum / 2;
                }
                else
                if(floorInfoArr[index].hitNum == HIT_NUM_2)
                {
                    /* another half floor for lookup 2 */
                    applicationPlatformTcamTtiHit2MaxNum += tcamFloorEntriesNum / 2;
                }
                else
                if(floorInfoArr[index].hitNum == HIT_NUM_3)
                {
                    /* another half floor for lookup 3 */
                    applicationPlatformTcamTtiHit3MaxNum += tcamFloorEntriesNum / 2;
                }
            }

            rc = cpssDxChTcamIndexRangeHitNumAndGroupSet(devNum,value,floorInfoArr);
            CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChTcamIndexRangeHitNumAndGroupSet);
        }

        /* support case that the baseFloorForTtiLookup0 was set by 'applicationPlatformDbEntryGet'
           to value != 6 */
        if(ttiLookup0NonStandard == GT_TRUE)
        {
            /* dedicated floors for lookup 0 */
            for (ii = 0; ii < CPSS_DXCH_TCAM_MAX_NUM_BLOCKS_CNS; ii++)
            {
                floorInfoArr[ii].hitNum = HIT_NUM_0;
            }

            for(ii = baseFloorForTtiLookup0 ; ii < ttiLookupInfoPtr[0].floorNum ; ii ++)
            {
                value = ii;

                if(value >= tcamFloorsNum)
                {
                    /* ignore */
                    continue;
                }

                /* calculate the number of entries that each lookup can use */
                /* another floor for lookup 0 */
                applicationPlatformTcamTtiHit0MaxNum += tcamFloorEntriesNum;

                rc = cpssDxChTcamIndexRangeHitNumAndGroupSet(devNum,value,floorInfoArr);
                CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChTcamIndexRangeHitNumAndGroupSet);
            }
        }
    }

    /* IPCL/EPCL get what the TTI not use */
    applicationPlatformTcamIpcl0MaxNum = (tcamFloorsNum * tcamFloorEntriesNum) - /* full tcam size */
            (applicationPlatformTcamTtiHit0MaxNum +   /* used by TTI hit 0*/
             applicationPlatformTcamTtiHit1MaxNum +   /* used by TTI hit 1*/
             applicationPlatformTcamTtiHit2MaxNum +   /* used by TTI hit 2*/
             applicationPlatformTcamTtiHit3MaxNum);   /* used by TTI hit 3*/
    applicationPlatformTcamIpcl1MaxNum = applicationPlatformTcamIpcl0MaxNum;
    if (PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.pcl.iPcl0Bypass == GT_FALSE)
    {
        applicationPlatformTcamIpcl2MaxNum = applicationPlatformTcamIpcl0MaxNum;
    }
    applicationPlatformTcamEpclMaxNum  = applicationPlatformTcamIpcl0MaxNum;

    /* save TCAM 'Sections' (base+size) for TTI and IPCL/EPLC */
    applicationPlatformDxChTcamSectionsSave();
    return GT_OK;
}

static GT_STATUS prvPolicerLibInit
(
    IN GT_U8 devNum
)
{
    GT_STATUS rc = GT_OK;

    rc = cpssDxChPolicerInit(devNum);
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChPolicerInit);

    return rc;
}

static GT_STATUS prvIngressPolicerDisable
(
    IN GT_U8 devNum
)
{
    GT_STATUS rc = GT_OK;

    /* check if Ingress stage #1 exists */
    if (PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.featureInfo.iplrSecondStageSupported
        == GT_TRUE)
    {
        /* Disable Policer Metering on Ingress stage #1 */
        rc = cpssDxCh3PolicerMeteringEnableSet(devNum,
                                               CPSS_DXCH_POLICER_STAGE_INGRESS_1_E,
                                               GT_FALSE);
        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxCh3PolicerMeteringEnableSet);

        /* Disable Policer Counting on Ingress stage #1 */
        rc = cpssDxChPolicerCountingModeSet(devNum,
                                            CPSS_DXCH_POLICER_STAGE_INGRESS_1_E,
                                            CPSS_DXCH_POLICER_COUNTING_DISABLE_E);
        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChPolicerCountingModeSet);
   }

   return rc;
}

static GT_STATUS prvTrunkLibInit
(
    IN GT_U8                             devNum,
    IN CPSS_APP_PLATFORM_PP_PROFILE_STC *ppProfilePtr
)
{
    GT_STATUS rc = GT_OK;
    GT_U32    currMaxTrunks = 0;
    GT_U32    maxTrunksNeeded = 0;
    GT_BOOL   flexWithFixedSize = GT_FALSE;
    GT_U32    fixedNumOfMembersInTrunks = 0;
    CPSS_DXCH_TRUNK_MEMBERS_MODE_ENT membersMode = ppProfilePtr->trunkMembersMode;

    if (PRV_CPSS_SIP_5_CHECK_MAC(devNum))
    {
        /* at this stage the PRV_CPSS_DXCH_PP_MAC(dev)->fineTuning.tableSize.trunksNum
            hold the number '4K' ... which is NOT what we need !

            we need '8 members' trunks that take 1/2 of L2ECMP table.
        */
        maxTrunksNeeded = PRV_CPSS_DXCH_PP_MAC(devNum)->bridge.l2EcmpNumMembers;
        maxTrunksNeeded = ((maxTrunksNeeded / 2) / PRV_CPSS_TRUNK_8_MAX_NUM_OF_MEMBERS_CNS) - 1;
    }
    else
    {
        maxTrunksNeeded = PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.tableSize.trunksNum;
    }

    if(ppProfilePtr->numOfTrunks > maxTrunksNeeded)
    {
        currMaxTrunks = maxTrunksNeeded;
    }
    else
    {
        currMaxTrunks = ppProfilePtr->numOfTrunks;
    }

    /* for falcon port mode 512 ( 512 trunks) and port 1024 (256 trunks) support maxTrunks according to table entries number */
    if((PRV_CPSS_SIP_6_CHECK_MAC(devNum)) &&
       (currMaxTrunks > PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.tableSize.trunksNum))
    {
        currMaxTrunks =  PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.tableSize.trunksNum;
    }

    rc = cpssDxChTrunkInit(devNum, currMaxTrunks, membersMode);
    while(rc == GT_OUT_OF_RANGE && currMaxTrunks)
    {
        currMaxTrunks--;
        rc = cpssDxChTrunkInit(devNum, currMaxTrunks, membersMode);
    }

    if((rc != GT_OK) && (currMaxTrunks != ppProfilePtr->numOfTrunks))
    {
        CPSS_APP_PLATFORM_LOG_ERR_MAC("cpssDxChTrunkInit: device[%d] not support [%d] trunks\n",devNum,currMaxTrunks);
    }

    if(rc == GT_OK &&
       membersMode == CPSS_DXCH_TRUNK_MEMBERS_MODE_FLEX_E &&
       flexWithFixedSize == GT_TRUE)
    {
        /* set all our trunks to be with the same max size (the value that we want) */
        /* set the 'hybrid mode' of 'flex' and 'fixed size' */
        rc = cpssDxChTrunkFlexInfoSet(devNum,0xFFFF/*hybrid mode indication*/,
                                      0, fixedNumOfMembersInTrunks);
        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChTrunkFlexInfoSet);
    }

    return rc;
}

static GT_STATUS prvDxCh2Ch3IpLibInit
(
    IN GT_U8                             devNum,
    IN CPSS_APP_PLATFORM_PP_PROFILE_STC *ppProfilePtr
)
{
    GT_STATUS                                       rc = GT_OK;
    CPSS_DXCH_IP_TCAM_LPM_MANGER_CAPCITY_CFG_STC    cpssLpmDbCapacity;
    CPSS_DXCH_IP_TCAM_LPM_MANGER_INDEX_RANGE_STC    cpssLpmDbRange;
    CPSS_DXCH_IP_LPM_MEMORY_CONFIG_UNT              lpmMemoryConfig;
    CPSS_DXCH_IP_UC_ROUTE_ENTRY_STC                 ucRouteEntry;
    CPSS_DXCH_IP_MC_ROUTE_ENTRY_STC                 mcRouteEntry;
    CPSS_DXCH_IP_LPM_SHADOW_TYPE_ENT                shadowType;
    CPSS_IP_PROTOCOL_STACK_ENT                      protocolStack;
    CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT          defUcLttEntry;
    CPSS_DXCH_IP_LTT_ENTRY_STC                      defMcLttEntry;
    CPSS_DXCH_IP_LPM_VR_CONFIG_STC                  vrConfigInfo;
    GT_U32                                          lpmDbId = 0;
    CPSS_DXCH_CFG_DEV_INFO_STC                      devInfo;
    CPSS_PP_FAMILY_TYPE_ENT                         devFamily;
    /*CPSS_DXCH_LPM_RAM_CONFIG_STC                    lpmRamMemoryBlocksCfg;*/

    rc = cpssDxChCfgDevInfoGet(devNum, &devInfo);
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChCfgDevInfoGet);

    devFamily = devInfo.genDevInfo.devFamily;

    switch (devFamily)
    {
        case CPSS_PP_FAMILY_DXCH_ALDRIN_E:
        case CPSS_PP_FAMILY_DXCH_ALDRIN2_E:
        case CPSS_PP_FAMILY_DXCH_AC3X_E:
            shadowType = CPSS_DXCH_IP_LPM_RAM_SIP5_SHADOW_E;
            break;
        case CPSS_PP_FAMILY_DXCH_FALCON_E:
        case CPSS_PP_FAMILY_DXCH_AC5X_E:
            shadowType = CPSS_DXCH_IP_LPM_RAM_SIP6_SHADOW_E;
            break;

        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_IMPLEMENTED, LOG_ERROR_NO_MSG);
    }

    /* init default UC and MC entries */
    cpssOsMemSet(&defUcLttEntry,0,sizeof(CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT));
    cpssOsMemSet(&defMcLttEntry,0,sizeof(CPSS_DXCH_IP_LTT_ENTRY_STC));

    defUcLttEntry.ipLttEntry.ipv6MCGroupScopeLevel    = CPSS_IPV6_PREFIX_SCOPE_GLOBAL_E;
    defUcLttEntry.ipLttEntry.numOfPaths               = 0;
    defUcLttEntry.ipLttEntry.routeEntryBaseIndex      = 0;
    if ((shadowType == CPSS_DXCH_IP_LPM_RAM_SIP5_SHADOW_E)||
        (shadowType == CPSS_DXCH_IP_LPM_RAM_SIP6_SHADOW_E))
    {
        defUcLttEntry.ipLttEntry.routeType            = CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_REGULAR_E;
    }
    else
    {
        defUcLttEntry.ipLttEntry.routeType            = CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_ECMP_E;
    }

    defUcLttEntry.ipLttEntry.sipSaCheckMismatchEnable = GT_FALSE;
    defUcLttEntry.ipLttEntry.ucRPFCheckEnable         = GT_FALSE;

    defMcLttEntry.ipv6MCGroupScopeLevel    = CPSS_IPV6_PREFIX_SCOPE_GLOBAL_E;
    defMcLttEntry.numOfPaths               = 0;
    defMcLttEntry.routeEntryBaseIndex      = 1;
    if ((shadowType == CPSS_DXCH_IP_LPM_RAM_SIP5_SHADOW_E)||
        (shadowType == CPSS_DXCH_IP_LPM_RAM_SIP6_SHADOW_E))
    {
        defMcLttEntry.routeType            = CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_REGULAR_E;
    }
    else
    {
        defMcLttEntry.routeType            = CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_ECMP_E;
    }

    defMcLttEntry.sipSaCheckMismatchEnable = GT_FALSE;
    defMcLttEntry.ucRPFCheckEnable         = GT_FALSE;


    cpssOsMemSet(&vrConfigInfo,0,sizeof(CPSS_DXCH_IP_LPM_VR_CONFIG_STC));

    /* define protocolStack */
    if ((ppProfilePtr->lpmDbSupportIpv4 == GT_FALSE) && (ppProfilePtr->lpmDbSupportIpv6 == GT_FALSE))
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
     if ((ppProfilePtr->lpmDbSupportIpv4 == GT_TRUE) && (ppProfilePtr->lpmDbSupportIpv6 == GT_TRUE))
        protocolStack = CPSS_IP_PROTOCOL_IPV4V6_E;
    else
        protocolStack = (ppProfilePtr->lpmDbSupportIpv4 == GT_TRUE) ?
            CPSS_IP_PROTOCOL_IPV4_E : CPSS_IP_PROTOCOL_IPV6_E;

    /* fill vrConfigInfo structure */
    switch(protocolStack)
    {
    case CPSS_IP_PROTOCOL_IPV4_E:
        vrConfigInfo.supportIpv4Uc = GT_TRUE;
        cpssOsMemCpy(&vrConfigInfo.defIpv4UcNextHopInfo.ipLttEntry,
                     &defUcLttEntry.ipLttEntry,
                     sizeof(CPSS_DXCH_IP_LTT_ENTRY_STC));
        vrConfigInfo.supportIpv6Uc = GT_FALSE;
        vrConfigInfo.supportIpv6Mc = GT_FALSE;

        vrConfigInfo.supportIpv4Mc = GT_TRUE;
        cpssOsMemCpy(&vrConfigInfo.defIpv4McRouteLttEntry,
                     &defMcLttEntry,
                     sizeof(CPSS_DXCH_IP_LTT_ENTRY_STC));

        break;
    case CPSS_IP_PROTOCOL_IPV6_E:
        vrConfigInfo.supportIpv4Uc = GT_FALSE;
        vrConfigInfo.supportIpv6Uc = GT_TRUE;
        cpssOsMemCpy(&vrConfigInfo.defIpv6UcNextHopInfo.ipLttEntry,
                     &defUcLttEntry.ipLttEntry,
                     sizeof(CPSS_DXCH_IP_LTT_ENTRY_STC));
        vrConfigInfo.supportIpv4Mc = GT_FALSE;
        vrConfigInfo.supportIpv6Mc = GT_TRUE;
        cpssOsMemCpy(&vrConfigInfo.defIpv6McRouteLttEntry,
                     &defMcLttEntry,
                     sizeof(CPSS_DXCH_IP_LTT_ENTRY_STC));
        break;
    case CPSS_IP_PROTOCOL_IPV4V6_E:
        vrConfigInfo.supportIpv4Uc = GT_TRUE;
        cpssOsMemCpy(&vrConfigInfo.defIpv4UcNextHopInfo.ipLttEntry,
                     &defUcLttEntry.ipLttEntry,
                     sizeof(CPSS_DXCH_IP_LTT_ENTRY_STC));
        vrConfigInfo.supportIpv6Uc = GT_TRUE;
        cpssOsMemCpy(&vrConfigInfo.defIpv6UcNextHopInfo.ipLttEntry,
                     &defUcLttEntry.ipLttEntry,
                     sizeof(CPSS_DXCH_IP_LTT_ENTRY_STC));
        vrConfigInfo.supportIpv4Mc = GT_TRUE;
        vrConfigInfo.supportIpv6Mc = GT_TRUE;

        vrConfigInfo.supportIpv6Mc = GT_TRUE;
        vrConfigInfo.supportIpv4Mc = GT_TRUE;
        cpssOsMemCpy(&vrConfigInfo.defIpv4McRouteLttEntry,
                     &defMcLttEntry,
                     sizeof(CPSS_DXCH_IP_LTT_ENTRY_STC));
        cpssOsMemCpy(&vrConfigInfo.defIpv6McRouteLttEntry,
                     &defMcLttEntry,
                     sizeof(CPSS_DXCH_IP_LTT_ENTRY_STC));


        break;
    default:
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }


    if( (vrConfigInfo.supportIpv4Uc == GT_TRUE) || (vrConfigInfo.supportIpv6Uc == GT_TRUE) )
    {
        /************************************************************/
        /* in this case ipv4/ipv6 unicast have common default route */
        /* set route entry 0 as default ipv4/6 unicast              */
        /************************************************************/
        cpssOsMemSet(&ucRouteEntry,0,sizeof(ucRouteEntry));
        ucRouteEntry.type = CPSS_DXCH_IP_UC_ROUTE_ENTRY_E;
        ucRouteEntry.entry.regularEntry.cmd = CPSS_PACKET_CMD_TRAP_TO_CPU_E;
        rc = cpssDxChIpUcRouteEntriesWrite(devNum, 0, &ucRouteEntry, 1);
        if (rc != GT_OK)
        {
            if(rc == GT_OUT_OF_RANGE)
            {
                /* the device not support any IP (not router device)*/
                rc = GT_OK;

                CPSS_APP_PLATFORM_LOG_ERR_MAC("cpssDxChIpUcRouteEntriesWrite : device[%d] not supported \n",devNum);
            }

            return  rc;
        }
    }

    if( (vrConfigInfo.supportIpv4Mc == GT_TRUE) || (vrConfigInfo.supportIpv6Mc == GT_TRUE) )
    {
        /*******************************************************/
        /* in this case ipv4/ipv6 mc have common default route */
        /* set route entry 1 as default ipv4/6 multicast       */
        /*******************************************************/
        cpssOsMemSet(&mcRouteEntry,0,sizeof(mcRouteEntry));
        mcRouteEntry.cmd = CPSS_PACKET_CMD_TRAP_TO_CPU_E;
        mcRouteEntry.RPFFailCommand = CPSS_PACKET_CMD_TRAP_TO_CPU_E;
        rc = cpssDxChIpMcRouteEntriesWrite(devNum, 1, &mcRouteEntry);
        if (rc != GT_OK)
        {
            return  rc;
        }
    }

    /********************************************************************/
    /* if lpm db is already created, all that is needed to do is to add */
    /* the device to the lpm db                                         */
    /********************************************************************/
    if (lpmDbInitialized == GT_TRUE)
    {
        rc = cpssDxChIpLpmDBDevListAdd(lpmDbId, &devNum, 1);
        if(rc == GT_BAD_PARAM)
        {
            CPSS_APP_PLATFORM_LOG_ERR_MAC("cpssDxChIpLpmDBDevListAdd : device[%d] not supported \n",devNum);
            /* the device not support the router tcam */
            rc = GT_OK;
        }

        return rc;
    }

    /*****************/
    /* create LPM DB */
    /*****************/

    /* set parameters */

    cpssLpmDbCapacity.numOfIpv4Prefixes         = ppProfilePtr->maxNumOfIpv4Prefixes;
    cpssLpmDbCapacity.numOfIpv6Prefixes         = ppProfilePtr->maxNumOfIpv6Prefixes;
    cpssLpmDbCapacity.numOfIpv4McSourcePrefixes = ppProfilePtr->maxNumOfIpv4McEntries;
    cpssLpmDbRange.firstIndex                   = ppProfilePtr->lpmDbFirstTcamLine;
    cpssLpmDbRange.lastIndex                    = ppProfilePtr->lpmDbLastTcamLine;

    if ((shadowType != CPSS_DXCH_IP_LPM_RAM_SIP5_SHADOW_E)&&(shadowType != CPSS_DXCH_IP_LPM_RAM_SIP6_SHADOW_E))
    {
        lpmMemoryConfig.tcamDbCfg.indexesRangePtr = &cpssLpmDbRange;
        lpmMemoryConfig.tcamDbCfg.partitionEnable = ppProfilePtr->lpmDbPartitionEnable;
        lpmMemoryConfig.tcamDbCfg.tcamLpmManagerCapcityCfgPtr = &cpssLpmDbCapacity;
        lpmMemoryConfig.tcamDbCfg.tcamManagerHandlerPtr = NULL;
    }
    else
    {
        if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
        {
            rc = appPlatformFalconIpLpmRamDefaultConfigCalc(devNum, ppProfilePtr->sharedTableMode, ppProfilePtr->maxNumOfPbrEntries, &lpmMemoryConfig.ramDbCfg);
            CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, appPlatformBc2IpLpmRamDefaultConfigCalc);
        }
        else
        {
            rc = appPlatformBc2IpLpmRamDefaultConfigCalc(devNum, ppProfilePtr->maxNumOfPbrEntries, &lpmMemoryConfig.ramDbCfg);
            CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, appPlatformBc2IpLpmRamDefaultConfigCalc);
        }

        lpmMemoryConfig.ramDbCfg.lpmRamConfigInfo[0].sharedMemCnfg    = ppProfilePtr->sharedTableMode;
    }

    rc = cpssDxChIpLpmDBCreate(lpmDbId, shadowType,
                               protocolStack, &lpmMemoryConfig);
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChIpLpmDBCreate);


    /* mark the lpm db as created */
    lpmDbInitialized = GT_TRUE;

    /*******************************/
    /* add active device to LPM DB */
    /*******************************/
    rc = cpssDxChIpLpmDBDevListAdd(lpmDbId, &devNum, 1);
    if(rc == GT_BAD_PARAM)
    {
        /* the device not support the router tcam */
        CPSS_APP_PLATFORM_LOG_ERR_MAC("cpssDxChIpLpmDBDevListAdd : device[%d] not supported \n",devNum);
        return GT_OK;
    }
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChIpLpmDBDevListAdd);

    /*************************/
    /* create virtual router */
    /*************************/
    rc = cpssDxChIpLpmVirtualRouterAdd(lpmDbId, 0, &vrConfigInfo);
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChIpLpmVirtualRouterAdd);

    return rc;
}

static GT_STATUS prvIpLibInit
(
    IN GT_U8                             devNum,
    IN CPSS_APP_PLATFORM_PP_PROFILE_STC *ppProfilePtr
)
{
    GT_STATUS rc = GT_OK;

    /* if policy based routing, use same initialization as Ch+,
       if not use same initialization as Ch2,Ch3 */
    if (ppProfilePtr->usePolicyBasedRouting == GT_TRUE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_IMPLEMENTED, LOG_ERROR_NO_MSG);
    }
    else
    {
        rc = prvDxCh2Ch3IpLibInit(devNum,ppProfilePtr);
        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, prvDxCh2Ch3IpLibInit);
    }

    return rc;
}

static GT_VOID_PTR lpmDbSL;


extern GT_VOID_PTR prvCpssSlSearch
(
    IN GT_VOID_PTR   ctrlPtr,
    IN GT_VOID_PTR   dataPtr
);

/**
* @internal prvCpssAppPlatformIpLpmLibReset function
* @endinternal
*
* @brief   IP LPM module reset
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS prvCpssAppPlatformIpLpmLibReset
(
   GT_VOID
)
{
    GT_STATUS       rc;                 /* return code */
    GT_U32          lpmDbId = 0;        /* LPM DB index */
    GT_UINTPTR      slIterator;         /* Skip List iterator */
    GT_U8           devListArray[PRV_CPSS_MAX_PP_DEVICES_CNS]; /* device list */
    GT_U32          numOfDevs;          /* device number (array size) */
    PRV_CPSS_DXCH_LPM_SHADOW_STC        *lpmDbPtr,tmpLpmDb; /* pointer to and temp instance of LMP DB entry */
    PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC    *ipShadowPtr; /* pointer to shadow entry */
    PRV_CPSS_DXCH_LPM_RAM_SHADOWS_DB_STC*ipShadowsPtr; /* pointer to shadows DB */
    PRV_CPSS_DXCH_LPM_TCAM_SHADOW_STC    *tcam_ipShadowPtr;/* TCAM (xcat style): pointer to shadow entry */
    PRV_CPSS_DXCH_LPM_TCAM_VR_TBL_BLOCK_STC  *vrEntryPtr = NULL;/* TCAM (xcat style): iterator over virtual routers */
    GT_UINTPTR                          vrIterator;
    GT_U32                                 vrId;

    GT_U32          shadowIdx;          /* shadow index */
    GT_U32          i;                  /* loop iterator */
    CPSS_DXCH_IP_LPM_SHADOW_TYPE_ENT    shadowType;
    CPSS_IP_PROTOCOL_STACK_ENT          protocolStack;
    CPSS_DXCH_IP_LPM_MEMORY_CONFIG_UNT  memoryCfg;
    CPSS_DXCH_IP_TCAM_LPM_MANGER_INDEX_RANGE_STC indexesRange;
    CPSS_DXCH_IP_TCAM_LPM_MANGER_CAPCITY_CFG_STC capcityCfg;

    cpssOsMemSet(&memoryCfg,0,sizeof(memoryCfg));
    memoryCfg.tcamDbCfg.indexesRangePtr = &indexesRange;
    memoryCfg.tcamDbCfg.tcamLpmManagerCapcityCfgPtr = &capcityCfg;

    while (1)
    {
        slIterator = 0;
        rc =  prvCpssDxChIpLpmDbIdGetNext(&lpmDbId,&slIterator);
        if (rc == GT_NO_MORE)
        {
            break;
        }
        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, prvCpssDxChIpLpmDbIdGetNext);

        rc = cpssDxChIpLpmDBConfigGet(lpmDbId,&shadowType,&protocolStack,&memoryCfg);
        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChIpLpmDBConfigGet);

        lpmDbSL = PRV_SHARED_PORT_DIR_IP_LPM_SRC_GLOBAL_VAR_GET(lpmDbSL);

        /* check parameters */
        tmpLpmDb.lpmDbId = lpmDbId;
        lpmDbPtr = prvCpssSlSearch(lpmDbSL,&tmpLpmDb);
        if (lpmDbPtr == NULL)
        {
            cpssOsPrintSync("prvIpLpmLibReset: lpmDbId = %d\r\n", lpmDbId);
            /* can't find the lpm DB */
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
        }

        if((shadowType == CPSS_DXCH_IP_LPM_RAM_SIP5_SHADOW_E) || (shadowType == CPSS_DXCH_IP_LPM_RAM_SIP6_SHADOW_E) )
        {
            ipShadowsPtr = lpmDbPtr->shadow;

            for (shadowIdx = 0; shadowIdx < ipShadowsPtr->numOfShadowCfg; shadowIdx++)
            {
                ipShadowPtr = &ipShadowsPtr->shadowArray[shadowIdx];

                for(i = 0; i < ipShadowPtr->vrfTblSize; i++)
                {
                    if(ipShadowPtr->vrRootBucketArray[i].valid == GT_TRUE)
                    {
                        rc = cpssDxChIpLpmVirtualRouterDel(lpmDbId, i);
                        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChIpLpmVirtualRouterDel);
                    }
                }
            }
        }
        else
        {
            tcam_ipShadowPtr = (PRV_CPSS_DXCH_LPM_TCAM_SHADOW_STC*)(lpmDbPtr->shadow);
            vrIterator = 0;/* get first */
            vrEntryPtr = prvCpssSlGetFirst(tcam_ipShadowPtr->vrSl,NULL,&vrIterator);
            if(vrEntryPtr != NULL)
            {
                do
                {
                    vrId = vrEntryPtr->vrId;
                    /* hard coded ... tempo */
                    rc = cpssDxChIpLpmVirtualRouterDel(lpmDbId, vrId);
                    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChIpLpmVirtualRouterDel);
                }
                while ((vrEntryPtr = prvCpssSlGetNext(tcam_ipShadowPtr->vrSl,&vrIterator)) != NULL);
            }
        }

        numOfDevs = PRV_CPSS_MAX_PP_DEVICES_CNS;

        /* Get device list array from skip list for given LMP DB id */
        rc = cpssDxChIpLpmDBDevListGet(lpmDbId, &numOfDevs, devListArray);
        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChIpLpmDBDevListGet);

        /* Delete device list array for given LMP DB id */
        rc = cpssDxChIpLpmDBDevsListRemove(lpmDbId, devListArray , numOfDevs);
        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChIpLpmDBDevsListRemove);

        rc = cpssDxChIpLpmDBDelete(lpmDbId);
        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChIpLpmDBDelete);

    }

    lpmDbInitialized = GT_FALSE;

    return GT_OK;
}

/* NOTE: should be used only for sip6 device */
static GT_STATUS prvPhaLibInit
(
    IN  GT_U8                            devNum,
    IN CPSS_APP_PLATFORM_PP_PROFILE_STC *ppProfilePtr
)
{
    GT_STATUS rc = GT_OK;
    CPSS_DXCH_PHA_FW_IMAGE_ID_ENT  phaFwImageId;

    /* Set PHA firmware image ID to default */
    phaFwImageId = CPSS_DXCH_PHA_FW_IMAGE_ID_DEFAULT_E;

    rc = cpssDxChPhaInit(devNum,ppProfilePtr->pha_packetOrderChangeEnable, phaFwImageId);
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChPhaInit);

    (GT_VOID)prvCpssDxChPhaFwVersionPrint(devNum);

    return rc;
}

/**
* @internal cpssAppPlatformPpLogicalInit function
* @endinternal
*
* @brief logical initialization for a specific Pp.
*
* @param[in] devNum                - Device Number.
* @param [in] *ppProfileInfo     - PP profile
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS cpssAppPlatformPpLogicalInit
(
    IN GT_U8                              devNum,
    IN CPSS_APP_PLATFORM_PP_PROFILE_STC  *ppProfilePtr
)
{
    GT_STATUS                        rc;
    GT_U32                           value;
    CPSS_DXCH_PP_CONFIG_INIT_STC     ppLogicalInfo;     /* CPSS format - DxCh info  */

    cpssOsMemSet(&ppLogicalInfo ,0, sizeof(ppLogicalInfo));

    ppLogicalInfo.routingMode        = ppProfilePtr->routingMode;

    if(appPlatformDbEntryGet("maxNumOfPbrEntries", &value) == GT_OK)
        ppLogicalInfo.maxNumOfPbrEntries = value;
    else
        ppLogicalInfo.maxNumOfPbrEntries = ppProfilePtr->maxNumOfPbrEntries;

    if(appPlatformDbEntryGet("lpmMemMode", &value) == GT_OK)
        ppLogicalInfo.lpmMemoryMode      = value?CPSS_DXCH_LPM_RAM_MEM_MODE_HALF_MEM_SIZE_E:CPSS_DXCH_LPM_RAM_MEM_MODE_FULL_MEM_SIZE_E;
    else
        ppLogicalInfo.lpmMemoryMode      = ppProfilePtr->lpmMemoryMode;

    if (PRV_CPSS_SIP_6_CHECK_MAC(devNum))
        ppLogicalInfo.sharedTableMode = ppProfilePtr->sharedTableMode;

    /* call CPSS to do Pp logical init */
    rc = cpssDxChCfgPpLogicalInit(devNum, &ppLogicalInfo);
    CPSS_APP_PLATFORM_LOG_INFO_MAC("Logical Init done...\n");
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChCfgPpLogicalInit);

    return GT_OK;

} /* cpssAppPlatformPpLogicalInit */

/**
* @internal cpssAppPlatformPpLibInit function
* @endinternal
*
* @brief   initialize CPSS LIBs in correct order.
*
* @param[in] devNum                - Device Number.
* @param [in] *ppProfileInfo       - PP profile.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS cpssAppPlatformPpLibInit
(
    IN GT_U8                             devNum,
    IN CPSS_APP_PLATFORM_PP_PROFILE_STC *ppProfilePtr
)
{
    GT_STATUS rc = GT_OK;

    if(ppProfilePtr->initPort == GT_TRUE)
    {
        rc = prvPortLibInit(devNum, ppProfilePtr);
        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, prvPortLibInit);
    }

    if(ppProfilePtr->initPhy)
    {
        rc = prvPhyLibInit(devNum);
        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, prvPhyLibInit);
    }

    if(ppProfilePtr->initBridge)
    {
        rc = prvBridgeLibInit(devNum, ppProfilePtr);
        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, prvBridgeLibInit);
    }

    if(ppProfilePtr->initNetIf)
    {
        rc = prvNetIfLibInit(devNum, ppProfilePtr);
        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, prvNetIfLibInit);
    }

    if(ppProfilePtr->initMirror)
    {
        rc = prvMirrorLibInit(devNum, ppProfilePtr);
        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, prvMirrorLibInit);
    }

    if(ppProfilePtr->initPcl)
    {
        rc = prvPclLibInit(devNum);
        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, prvPclLibInit);
    }

    if(ppProfilePtr->initTcam)
    {
        rc = prvTcamLibInit(devNum, ppProfilePtr);
        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, prvTcamLibInit);
    }

    if(ppProfilePtr->initPolicer)
    {
        rc = prvPolicerLibInit(devNum);
        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, prvPolicerLibInit);
    }
    else
    {
        rc = prvIngressPolicerDisable(devNum);
        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, prvIngressPolicerDisable);
    }

    if(ppProfilePtr->initTrunk)
    {
        rc = prvTrunkLibInit(devNum, ppProfilePtr);
        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, prvTrunkLibInit);
    }

    if(ppProfilePtr->initIp)
    {
        rc = prvIpLibInit(devNum, ppProfilePtr);
        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, prvIpLibInit);
    }

    if(ppProfilePtr->initPha)
    {
        rc = prvPhaLibInit(devNum, ppProfilePtr);
        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, prvPhaLibInit);
    }

    CPSS_APP_PLATFORM_LOG_INFO_MAC("Lib Init done...\n");

    return rc;

} /* cpssAppPlatformPpLibInit */

/**
* @internal prvFdbActionDelete function
* @endinternal
*
* @brief   Deletes all addresses from FDB table.
*
* @param[in] dev                      - physical device number
*
* @param[out] actDevPtr                - pointer to old action device number
* @param[out] actDevMaskPtr            - pointer to old action device mask
* @param[out] triggerModePtr           - pointer to old trigger mode
* @param[out] modePtr                  - pointer to old action mode
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
*/
static GT_STATUS prvFdbActionDelete
(
    IN  GT_U8                       dev,
    OUT GT_U32                      *actDevPtr,
    OUT GT_U32                      *actDevMaskPtr,
    OUT CPSS_MAC_ACTION_MODE_ENT    *triggerModePtr,
    OUT CPSS_FDB_ACTION_MODE_ENT    *modePtr
)
{
    GT_STATUS rc;

    /* save Mac Action action mode */
    rc = cpssDxChBrgFdbActionModeGet(dev, modePtr);
    if (rc != GT_OK)
    {
        CPSS_APP_PLATFORM_LOG_ERR_MAC("cpssDxChBrgFdbActionModeGet rc= %d ret=%d", rc, GT_HW_ERROR);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);
    }

    /* save Mac Action trigger mode */
    rc = cpssDxChBrgFdbMacTriggerModeGet(dev, triggerModePtr);
    if (rc != GT_OK)
    {
        CPSS_APP_PLATFORM_LOG_ERR_MAC("cpssDxChBrgFdbMacTriggerModeGet rc= %d ret=%d", rc, GT_HW_ERROR);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);
    }

    /* save mac action device parameters */
    rc = cpssDxChBrgFdbActionActiveDevGet(dev,actDevPtr,actDevMaskPtr);
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChBrgFdbActionActiveDevGet);


    /* disable actions to be done on the entire MAC table
       before change active configuration */
    rc = cpssDxChBrgFdbActionsEnableSet(dev, GT_FALSE);
    if (rc != GT_OK)
    {
        CPSS_APP_PLATFORM_LOG_ERR_MAC("cpssDxChBrgFdbActionsEnableSet rc= %d ret=%d", rc, GT_HW_ERROR);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);
    }


    /* delete all entries regardless of device number */
    rc = cpssDxChBrgFdbActionActiveDevSet(dev,0, 0);
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChBrgFdbActionActiveDevSet);

    /* enable actions, action mode, triggered mode, trigger the action */
    rc = cpssDxChBrgFdbTrigActionStart(dev, CPSS_FDB_ACTION_DELETING_E);
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChBrgFdbTrigActionStart);

    return rc;
}

/**
* @internal appPlatformDxChFdbFlush function
* @endinternal
*
* @brief   Deletes all addresses from FDB table.
*
* @param[in] dev                      - physical device number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS appPlatformDxChFdbFlush
(
    IN GT_U8   dev
)
{
    GT_STATUS                 rc;
    GT_BOOL                   actionCompleted;      /* Action Trigger Flag         */
    GT_U32                    loopCounter;          /* Counter for busy wait loops */
    /* old value of AA TA messages to CPU enabler */
    GT_BOOL                   aaTaMessegesToCpuEnable;
    GT_U32                    actDev;               /* old action device number */
    GT_U32                    actDevMask;           /* old action device mask */
    CPSS_MAC_ACTION_MODE_ENT  triggerMode;          /* old trigger mode */
    CPSS_FDB_ACTION_MODE_ENT  mode;                 /* old action mode */
    GT_U32 numFdbWaitIterations = 0;


    /* enable delete of static entries */
    rc = cpssDxChBrgFdbStaticDelEnable(dev, CPSS_DXCH_BRG_FDB_DEL_MODE_ALL_E);
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChBrgFdbStaticDelEnable);

    /* save old value of AA TA messages to CPU enabler */
    rc = cpssDxChBrgFdbAAandTAToCpuGet(dev, &aaTaMessegesToCpuEnable);
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChBrgFdbAAandTAToCpuGet);

    if(aaTaMessegesToCpuEnable == GT_TRUE)
    {


        /* disable AA and TA messages to CPU */
        rc = cpssDxChBrgFdbAAandTAToCpuSet(dev, GT_FALSE);
        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChBrgFdbAAandTAToCpuSet);

    }

    rc = prvFdbActionDelete(dev, &actDev, &actDevMask, &triggerMode, &mode);
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, prvFdbActionDelete);


    actionCompleted = GT_FALSE;
    loopCounter = 0;

    /* busy wait for Action Trigger Status */
    while (!actionCompleted)
    {
        rc = cpssDxChBrgFdbTrigActionStatusGet(dev, &actionCompleted);
        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChBrgFdbTrigActionStatusGet);

        loopCounter ++;

        /* check if loop counter has reached 100000000 */
        if (loopCounter >= 100000000)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_TIMEOUT, LOG_ERROR_NO_MSG);
        }

        if(actionCompleted == GT_TRUE)
        {
            /* do nothing */
        }
        else if (cpssDeviceRunCheck_onEmulator())
        {
            osTimerWkAfter(500);
            CPSS_APP_PLATFORM_LOG_INFO_MAC(".");/* show progress ... */
            numFdbWaitIterations++;
        }
        else
        {
            osTimerWkAfter(1);
        }

    }

    if(numFdbWaitIterations)
    {
        CPSS_APP_PLATFORM_LOG_INFO_MAC("(trigger) FDB flush : numFdbWaitIterations[%d] equal [%d]ms wait \n",
            numFdbWaitIterations,numFdbWaitIterations*500);
    }

    if(aaTaMessegesToCpuEnable == GT_TRUE)
    {
        /* restore AA and TA messages to CPU */
        rc = cpssDxChBrgFdbAAandTAToCpuSet(dev, aaTaMessegesToCpuEnable);
        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChBrgFdbAAandTAToCpuSet);
    }


    /* restore active device parameters */
    rc = cpssDxChBrgFdbActionActiveDevSet(dev,
                                          actDev,
                                          actDevMask);

    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChBrgFdbActionActiveDevSet);

    /* disable delete of static entries */
    rc = cpssDxChBrgFdbStaticDelEnable(dev, GT_FALSE);
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChBrgFdbStaticDelEnable);


    /* restore Trigger mode - it should be last operation because
       it may start auto aging. All active config should be restored
       before start of auto aging */
    rc = cpssDxChBrgFdbMacTriggerModeSet(dev, triggerMode);
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChBrgFdbMacTriggerModeSet);

    return rc;

}

/**
* @internal appPlatformDxChFdbInit function
* @endinternal
*
* @brief   Perform Fdb initialization for a Pp.
*
* @param[in] devNum                - Device Number.
* @param [in] *ppProfileInfo     - PP profile
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS appPlatformDxChFdbInit
(
    IN GT_U8 devNum,
    IN CPSS_APP_PLATFORM_PP_PROFILE_STC *ppProfilePtr
)
{
    GT_STATUS rc = GT_OK;
    GT_U32    port;
    GT_BOOL   actFinished = GT_FALSE;
    GT_BOOL   autoLearn = GT_FALSE;
    GT_U32    fdbBmp;
    GT_U32    numPorts;
    GT_HW_DEV_NUM   hwDevNum;
    CPSS_FDB_ACTION_MODE_ENT    actionMode;

    /*********************************************************/
    /* Set FDB learning mode                                 */
    /*********************************************************/
    rc = cpssDxChBrgFdbMacVlanLookupModeSet(devNum, CPSS_IVL_E);
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChBrgFdbMacVlanLookupModeSet);

    /* restore automatic action mode*/
    rc = cpssDxChBrgFdbMacTriggerModeSet(devNum, CPSS_ACT_AUTO_E);
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChBrgFdbMacTriggerModeSet);

    /* default */
    actionMode = CPSS_FDB_ACTION_AGE_WITH_REMOVAL_E;

    switch(PRV_CPSS_DXCH_PP_MAC(devNum)->portGroupsExtraInfo.fdbMode)
    {
        case PRV_CPSS_DXCH_MULTI_PORT_GROUP_FDB_MODE_UNIFIED_E:
        case PRV_CPSS_DXCH_MULTI_PORT_GROUP_FDB_MODE_UNIFIED_LINKED_E:
            actionMode = CPSS_FDB_ACTION_AGE_WITHOUT_REMOVAL_E;
            break;
        case PRV_CPSS_DXCH_MULTI_PORT_GROUP_FDB_MODE_NON_MULTI_PORT_GROUP_DEVICE_E:
            if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_LION2_E)
            {
                actionMode = CPSS_FDB_ACTION_AGE_WITHOUT_REMOVAL_E;
            }
            break;
        default:
            break;
    }

    if (cpssDeviceRunCheck_onEmulator())
    {
        /* a waist of time to trigger operation on huge FDB table that is empty anyway */
        rc = cpssDxChBrgFdbActionModeSet(devNum, actionMode);
        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChBrgFdbActionModeSet);
    }
    else
    {
        /* restore age with/without removal */
        rc = cpssDxChBrgFdbTrigActionStart(devNum, actionMode);
        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChBrgFdbTrigActionStart);

        /* wait for action to be over */
        while(actFinished == GT_FALSE)
        {
            rc = cpssDxChBrgFdbTrigActionStatusGet(devNum,&actFinished);
            CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChBrgFdbTrigActionStatusGet);

            if(actFinished == GT_TRUE)
            {
                /* do nothing */
            }
            else
            {
                osTimerWkAfter(1);
            }
        }
    }

    /* enable the sending to CPU of AA/TA */
    rc = cpssDxChBrgFdbAAandTAToCpuSet(devNum, GT_TRUE);
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChBrgFdbAAandTAToCpuSet);

    /* Setting aging timeout to default timeout (300 seconds) is needed because
       DX106 core clock is 144MHz and not 200MHz as in other PPs. */
    rc = cpssDxChBrgFdbAgingTimeoutSet(devNum, 300);
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChBrgFdbAgingTimeoutSet);

    if(GT_FALSE == PRV_CPSS_DXCH_PP_HW_INFO_PORT_MAP_CHECK_SUPPORTED_MAC(devNum))
    {
        numPorts = cpssCapMaxPortNum[devNum]; /*Not PRV_CPSS_PP_MAC(devNum)->numOfPorts, but we need the max port number.*/
    }
    else
    {
        /* set range of physical ports although those are in eport range table */
        numPorts = MAX(256,PRV_CPSS_DXCH_MAX_PHY_PORT_NUMBER_MAC(devNum));
    }

    if(ppProfilePtr->ctrlMacLearn == GT_FALSE)
    {
        autoLearn = GT_TRUE;
    }

    for (port = 0; port < numPorts; port++)
    {
        if(GT_FALSE == PRV_CPSS_DXCH_PP_HW_INFO_PORT_MAP_CHECK_SUPPORTED_MAC(devNum))
        {
            CPSS_APPLICATION_PORT_SKIP_CHECK(devNum,port);
        }

        rc = cpssDxChBrgFdbPortLearnStatusSet(devNum, port, autoLearn, CPSS_LOCK_FRWRD_E);
        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChBrgFdbPortLearnStatusSet);

        /* note the cascade init will make sure to disable the sending from
           the cascade ports */
        rc = cpssDxChBrgFdbNaToCpuPerPortSet(devNum, port, GT_TRUE);
        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChBrgFdbNaToCpuPerPortSet);
    }

    fdbBmp = 0;
    /*Currently 1 device supported.will be updated later for more device*/
    /*
    for(i = 0; i < PRV_CPSS_MAX_PP_DEVICES_CNS; i++)
    {
        if((appDemoPpConfigList[i].valid == GT_TRUE) &&
           (PRV_CPSS_PP_MAC(appDemoPpConfigList[i].devNum)->devFamily !=
                            CPSS_PX_FAMILY_PIPE_E))
        {
            rc = cpssDxChCfgHwDevNumGet(appDemoPpConfigList[i].devNum, &hwDevNum);
            CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChCfgHwDevNumGet);
        }
    }*/
    rc = cpssDxChCfgHwDevNumGet(devNum, &hwDevNum);
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChCfgHwDevNumGet);
    fdbBmp |= (1 << hwDevNum );

    if (! PRV_CPSS_SIP_5_CHECK_MAC(devNum))
    {
        if(fdbBmp != 0)
        {
            rc = cpssDxChBrgFdbDeviceTableSet(devNum, fdbBmp);
            CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChBrgFdbDeviceTableSet);
        }
    }

    if (cpssDeviceRunCheck_onEmulator())
    {
        /* a waist of time to trigger operation on huge FDB table that is empty anyway */
    }
    else
    {
        /* Deletes all addresses from FDB table */
        rc = appPlatformDxChFdbFlush(devNum);
        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, appPlatformDxChFdbFlush);
    }

    if(ppProfilePtr->ctrlMacLearn == GT_TRUE)
    {
        /* code from afterInitBoardConfig(...) */
        /* Set 'Controlled aging' because the port groups can't share refresh info
           the AA to CPU enabled from
           appDemoDxChFdbInit(...) --> call cpssDxChBrgFdbAAandTAToCpuSet(...) */
        rc = cpssDxChBrgFdbActionModeSet(devNum, CPSS_FDB_ACTION_AGE_WITHOUT_REMOVAL_E);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    return GT_OK;
}

/**
* @internal appPlatformDxChIsLocalPort_or_RemotePort_with_MAC_PHY_OBJ function
* @endinternal
*
* @brief   the function allow to distinguish between ports that supports 'MAC'
*           related APIs on the 'local device' to those that not.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                - device number
* @param[in] portNum               - port number
*
* @retval 0         - the port is remote without 'MAC_PHY_OBJ' (not support 'MAC' APIs)
* @retval non-zero  - the port is local or remote with 'MAC_PHY_OBJ'
*/
GT_U32 appPlatformDxChIsLocalPort_or_RemotePort_with_MAC_PHY_OBJ
(
     GT_U8   devNum,
     GT_U32  portNum
)
{
    GT_STATUS             rc;
    CPSS_MACDRV_OBJ_STC * portMacObjPtr;
    GT_U32                  portMacNum;      /* MAC number */

    if(0 == PRV_CPSS_IS_DEV_EXISTS_MAC(devNum))
    {
        return 1;
    }

    if(!prvCpssDxChPortRemotePortCheck(devNum,portNum))
    {
        /* the port is local */
        return 1;
    }

    /* convert the remote port to it's LOCAL MAC number */
    rc = prvCpssDxChPortPhysicalPortMapCheckAndConvertForRemote(devNum, portNum, PRV_CPSS_DXCH_PORT_TYPE_MAC_E, &portMacNum);
    if(rc != GT_OK)
    {
        return 0;
    }

    /* Get PHY MAC object pointer */
    portMacObjPtr = PRV_CPSS_PHY_MAC_OBJ(devNum,portMacNum);

    return  portMacObjPtr ?
            1 :  /* the port is remote with 'MAC_PHY_OBJ' */
            0;   /* the port is remote without 'MAC_PHY_OBJ' (not support 'MAC' APIs) */
}

/**
* @internal appPlatformDxAldrin2TailDropDbaEnableConfigure function
* @endinternal
*
* @brief   Enables/disables Tail Drop DBA. Relevant for Aldrin2 only, does nothing for other devices.
*
* @param[in] dev                      -   device number
* @param[in] enable                   -   enable (GT_TRUE) or disable (GT_FALSE) Tail Drop DBA
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS appPlatformDxAldrin2TailDropDbaEnableConfigure
(
    IN  GT_U8     dev,
    IN  GT_BOOL   enable
)
{
    /* Aldrin2 defaults from JIRA CPSS-7422 */
    GT_STATUS                               rc;
    CPSS_PORT_TX_DROP_PROFILE_SET_ENT       profileSet;
    GT_U8                                   trafficClass;
    CPSS_PORT_TX_Q_TAIL_DROP_PROF_TC_PARAMS_STC tailDropProfileParams;
    CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_ENT    profileTdDbaAlfa;
    GT_U32                                  portMaxBuffLimit;
    GT_U32                                  portMaxDescrLimit;
    GT_U32                                  dp0MaxBuffNum;
    CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_ENT    dp0QueueAlpha;
    GT_U32                                  dp1MaxBuffNum;
    CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_ENT    dp1QueueAlpha;
    GT_U32                                  dp2MaxBuffNum;
    CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_ENT    dp2QueueAlpha;
    GT_U32                                  dp0MaxDescrNum;
    GT_U32                                  dp1MaxDescrNum;
    GT_U32                                  dp2MaxDescrNum;

    /* If not Aldrin2 do nothing */
    if (0 == PRV_CPSS_SIP_5_25_CHECK_MAC(dev)) return GT_OK;
    if (PRV_CPSS_SIP_6_CHECK_MAC(dev))         return GT_OK;

    if (enable == GT_FALSE)
    {
        /* HW defaults */
        profileTdDbaAlfa     = PRV_CPSS_ALDRIN2_TAIL_DROP_DEFAULT_DBA_ALPHA_CNS;
        portMaxBuffLimit     = PRV_CPSS_ALDRIN2_TAIL_DROP_DEFAULT_MAX_BUF_LIMIT_CNS;
        portMaxDescrLimit    = PRV_CPSS_ALDRIN2_TAIL_DROP_DEFAULT_MAX_DESCR_LIMIT_CNS;
        dp0MaxBuffNum        = PRV_CPSS_ALDRIN2_TAIL_DROP_DEFAULT_MAX_BUF_LIMIT_CNS;
        dp0QueueAlpha        = PRV_CPSS_ALDRIN2_TAIL_DROP_DEFAULT_DBA_ALPHA_CNS;
        dp1MaxBuffNum        = PRV_CPSS_ALDRIN2_TAIL_DROP_DEFAULT_MAX_BUF_LIMIT_CNS;
        dp1QueueAlpha        = PRV_CPSS_ALDRIN2_TAIL_DROP_DEFAULT_DBA_ALPHA_CNS;
        dp2MaxBuffNum        = PRV_CPSS_ALDRIN2_TAIL_DROP_DEFAULT_MAX_BUF_LIMIT_CNS;
        dp2QueueAlpha        = PRV_CPSS_ALDRIN2_TAIL_DROP_DEFAULT_DBA_ALPHA_CNS;
        dp0MaxDescrNum       = PRV_CPSS_ALDRIN2_TAIL_DROP_DEFAULT_MAX_DESCR_LIMIT_CNS;
        dp1MaxDescrNum       = PRV_CPSS_ALDRIN2_TAIL_DROP_DEFAULT_MAX_DESCR_LIMIT_CNS;
        dp2MaxDescrNum       = PRV_CPSS_ALDRIN2_TAIL_DROP_DEFAULT_MAX_DESCR_LIMIT_CNS;
    }
    else
    {
        profileTdDbaAlfa     = CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_0_E;
        portMaxBuffLimit     = 0xFFFF;
        portMaxDescrLimit    = 0xFFFF;
        dp0MaxBuffNum        = 0;
        dp0QueueAlpha        = CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_1_E;
        dp1MaxBuffNum        = 0;
        dp1QueueAlpha        = CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_1_E;
        dp2MaxBuffNum        = 0;
        dp2QueueAlpha        = CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_1_E;
        dp0MaxDescrNum       = 0xFFFF;
        dp1MaxDescrNum       = 0xFFFF;
        dp2MaxDescrNum       = 0xFFFF;
    }

    /* PFC DBA is not enabled by default */
    /* Aldrin2: /<CPFC_IP> CPFC_IP TLU/Units/CPFC_IP_SIP6.70 Units/Dynamic Buffer Allocation
       /Dynamic Buffer Allocation Disable bit0 default is 0 */

    /* QCN DBA is not enabled by default */
    /* Aldrin2: <TXQ_IP> TXQ_QCN/Units/TXQ_IP_qcn/CN Global Configuration bit7 default is 0 */

    /* TC Descriptor set to infinite */
    /* Buffer limit set to 0         */
    /* o Port thresholds set to infinity (disabled) */
    /* o Queue alpha\92s = 1 */
    /* o Available buffers = 38K   */
    for (profileSet = CPSS_PORT_TX_DROP_PROFILE_1_E;
         (profileSet <= CPSS_PORT_TX_DROP_PROFILE_16_E); profileSet++)
    {
        rc = cpssDxChPortTxTailDropProfileSet(
            dev, profileSet, profileTdDbaAlfa,
            portMaxBuffLimit, portMaxDescrLimit);
        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChPortTxTailDropProfileSet);

        for (trafficClass = 0; (trafficClass < 8); trafficClass++)
        {
            rc = cpssDxChPortTx4TcTailDropProfileGet(
                dev, profileSet, trafficClass, &tailDropProfileParams);
            CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChPortTx4TcTailDropProfileGet);

            tailDropProfileParams.dp0MaxBuffNum  = dp0MaxBuffNum;
            tailDropProfileParams.dp0QueueAlpha  = dp0QueueAlpha;
            tailDropProfileParams.dp1MaxBuffNum  = dp1MaxBuffNum;
            tailDropProfileParams.dp1QueueAlpha  = dp1QueueAlpha;
            tailDropProfileParams.dp2MaxBuffNum  = dp2MaxBuffNum;
            tailDropProfileParams.dp2QueueAlpha  = dp2QueueAlpha;
            tailDropProfileParams.dp0MaxDescrNum = dp0MaxDescrNum;
            tailDropProfileParams.dp1MaxDescrNum = dp1MaxDescrNum;
            tailDropProfileParams.dp2MaxDescrNum = dp2MaxDescrNum;
            rc = cpssDxChPortTx4TcTailDropProfileSet(
                dev, profileSet, trafficClass, &tailDropProfileParams);
            CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChPortTx4TcTailDropProfileSet);
        }
    }

    /* Tail drop DBA enabled by default */
    rc =  cpssDxChPortTxDbaEnableSet(dev, enable);
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChPortTxDbaEnableSet);

    if (PRV_CPSS_DXCH_PP_MAC(dev)->cutThroughEnable != GT_FALSE)
    {
        /* DBA tail drop algorithm use number of buffers for management.       */
        /* Size of cut-through packets for DBA defined by configuration below  */
        /* but not real one. Jumbo 10K packet use 40 buffers.                  */
        /* Therefore configure this value to be ready for worst case scenario. */
        rc = cpssDxChPortTxTailDropBufferConsumptionModeSet(
            dev, CPSS_DXCH_PORT_TX_TAIL_DROP_BUFFER_CONSUMPTION_MODE_BYTE_E, 40);
        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChPortTxTailDropBufferConsumptionModeSet);
    }
    return GT_OK;
}

/**
* @internal cpssAppPlatformPpGeneralInit function
* @endinternal
*
* @brief   General Pp configurations.
*
* @param[in] devNum                - Device Number.
* @param [in] *ppProfilePtr        - PP profile
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS cpssAppPlatformPpGeneralInit
(
    IN GT_U8                             devNum,
    IN CPSS_APP_PLATFORM_PP_PROFILE_STC *ppProfilePtr
)
{
    GT_STATUS           rc = GT_OK;
    GT_U32              maxPortNum;
    CPSS_PORTS_BMP_STC  portsMembers;
    CPSS_PORTS_BMP_STC  portsTagging;
    CPSS_DXCH_BRG_VLAN_INFO_STC  cpssVlanInfo;   /* cpss vlan info format    */
    CPSS_DXCH_BRG_VLAN_PORTS_TAG_CMD_STC portsTaggingCmd; /* ports tagging command */
    GT_U32              port;           /* current port number      */
    GT_U16              vid;            /* default VLAN id          */
    GT_U32              mruSize, cpuMruSize;
    GT_BOOL             flowControlEnable;
    CPSS_PORT_MAC_TYPE_ENT  portMacType;
    CPSS_CSCD_PORT_TYPE_ENT cscdPortType;
    CPSS_SYSTEM_RECOVERY_INFO_STC system_recovery; /* holds system recovery information */
    GT_U32 isLocalPort_or_RemotePort_with_MAC_PHY_OBJ;/*
            0         - the port is remote without 'MAC_PHY_OBJ' (not support 'MAC' APIs)
            non-zero  - the port is local or remote with 'MAC_PHY_OBJ'
                    */

    rc = cpssSystemRecoveryStateGet(&system_recovery);
    if (rc != GT_OK)
        CPSS_APP_PLATFORM_LOG_ERR_MAC("cpssSystemRecoveryStateGet rc=%d", rc);
    /*
    if(appDemoOnDistributedSimAsicSide)
    {
        return GT_OK;
    }
    */
    /***********************/
    /* FDB configuration   */
    /***********************/
    rc = appPlatformDxChFdbInit(devNum,ppProfilePtr);
    if (GT_OK != rc)
        CPSS_APP_PLATFORM_LOG_ERR_MAC("appPlatformDxChFdbInit rc=%d", rc);

    /*********************************************************/
    /* Port Configuration                                    */
    /*********************************************************/
    /* Jumbo frame support */
    mruSize = 1522; /* default */
    cpuMruSize = CPSS_DXCH_PORT_MAX_MRU_CNS; /* maximum */

    /*
    rc = appDemoDevIdxGet(dev, &devIdx);
    if ( GT_OK != rc )
    {
        return rc;
    }
     */

    for (port = 0, maxPortNum = cpssCapMaxPortNum[devNum]; port < maxPortNum; port++)
    {
        CPSS_APPLICATION_PORT_SKIP_CHECK(devNum,port);

        isLocalPort_or_RemotePort_with_MAC_PHY_OBJ =
            appPlatformDxChIsLocalPort_or_RemotePort_with_MAC_PHY_OBJ(devNum,port);

        /*  The next 2 calls are to set Port-Override-Tc-Enable */
        /* Set port trust mode */
        rc = cpssDxChCosPortQosTrustModeSet(devNum, port, CPSS_QOS_PORT_TRUST_L2_L3_E);
        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChCosPortQosTrustModeSet);

        /* set: Packet's DSCP is not remapped */
        rc = cpssDxChCosPortReMapDSCPSet(devNum, port, GT_FALSE);
        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChCosPortReMapDSCPSet);

        rc = cpssDxChPortMacTypeGet(devNum, port, &portMacType);
        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChPortMacTypeGet);

        if (system_recovery.systemRecoveryProcess == CPSS_SYSTEM_RECOVERY_PROCESS_NOT_ACTIVE_E)
        {

            if(portMacType < CPSS_PORT_MAC_TYPE_XG_E)
            {
                rc = cpssDxChPortDuplexModeSet(devNum, port, CPSS_PORT_FULL_DUPLEX_E);
                CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChPortDuplexModeSet);

                if (PRV_CPSS_PP_MAC(devNum)->devFamily != CPSS_PP_FAMILY_DXCH_LION2_E)
                {/* in devices of Lion2 family half-duplex not supported in any port mode/speed */
                    rc = cpssDxChPortDuplexAutoNegEnableSet(devNum, port, GT_TRUE);
                    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChPortDuplexAutoNegEnableSet);
                }

                rc = cpssDxChPortFlowCntrlAutoNegEnableSet(devNum, port, GT_FALSE, GT_TRUE);
                CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChPortFlowCntrlAutoNegEnableSet);

                rc = cpssDxChPortSpeedAutoNegEnableSet(devNum, port, GT_TRUE);
                CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChPortSpeedAutoNegEnableSet);
            }

            /* Disable Flow Control for revision 3 in DxCh2, DxCh3, XCAT. */
            if(ppProfilePtr->flowControlDisable)
            {
                flowControlEnable = GT_FALSE;
            }
            else
            {
                flowControlEnable = GT_TRUE;
            }

            if(isLocalPort_or_RemotePort_with_MAC_PHY_OBJ)
            {
                rc = cpssDxChPortFlowControlEnableSet(devNum, port, flowControlEnable);
                CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChPortFlowControlEnableSet);

                rc = cpssDxChPortPeriodicFcEnableSet(devNum, port, GT_FALSE);
                CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChPortPeriodicFcEnableSet);
            }
        }

        if(isLocalPort_or_RemotePort_with_MAC_PHY_OBJ)
        {
            rc = cpssDxChCscdPortTypeGet(devNum, port,CPSS_PORT_DIRECTION_BOTH_E, &cscdPortType);
            CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChCscdPortTypeGet);

            if((cscdPortType == CPSS_CSCD_PORT_NETWORK_E) &&
                (prvCpssDxChPortRemotePortCheck(devNum, port) == GT_FALSE))
            {
                rc = cpssDxChPortMruSet(devNum, port, mruSize);
                CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChPortMruSet);
            }
            else
            {
                /* the MRU for cascade or remote ports are set outside this function */
                /* in general the MRU of cascade ports should not cause drops ! */
                /* so needed to be set to 'max' value of the 'system' (+ max DSA tag bytes)  */
            }
        }

        if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_AC3X_E)
        {
            if(cscdPortType == CPSS_CSCD_PORT_NETWORK_E)
            {
                rc = cpssDxChPortCrcCheckEnableSet(devNum, port, GT_TRUE);
                CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChPortCrcCheckEnableSet);
            }
            else
            {
                /* Configure cascade port as 'pass-through" which means the CRC check is disabled by default -
                  letting the 88e1690 port configuration to decide */
                rc = cpssDxChPortCrcCheckEnableSet(devNum, port, GT_FALSE);
                CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChPortCrcCheckEnableSet);
            }
        }
        else
        {
            /* falcon in port mode 1024 use remote ports that are not bind to any device*/
            if(isLocalPort_or_RemotePort_with_MAC_PHY_OBJ)
            {
                rc = cpssDxChPortCrcCheckEnableSet(devNum, port, GT_TRUE);
                CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChPortCrcCheckEnableSet);
            }
        }

        /* call cpss api function with UP == 1 */
        rc = cpssDxChPortDefaultUPSet(devNum, port, 1);
        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChPortDefaultUPSet);
    }

    /* No dedicated MAC for CPU port in E_ARCH */
    if(PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.dedicatedCpuMac.isNotSupported == GT_FALSE)
    {
        rc = cpssDxChPortMruSet(devNum, CPSS_CPU_PORT_NUM_CNS, cpuMruSize);
        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChPortMruSet);
    }

    /****************************************************/
    /* XG ports additional board-specific configuration */
    /****************************************************/
    /* TODO in CPSS Application Platform PHASE2
    rc = prv10GPortsConfig(devNum);
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, prv10GPortsConfig);
    */



    /*********************************************************/
    /* Default VLAN configuration: VLAN 1 contains all ports */
    /*********************************************************/

    /* default VLAN id is 1 */
    vid = 1;

    /* Fill Vlan info */
    cpssOsMemSet(&cpssVlanInfo, 0, sizeof(cpssVlanInfo));
    /* default IP MC VIDX */
    cpssVlanInfo.unregIpmEVidx = 0xFFF;

    cpssVlanInfo.naMsgToCpuEn           = GT_TRUE;

    if(ppProfilePtr->ctrlMacLearn == GT_TRUE)
    {
        cpssVlanInfo.autoLearnDisable       = GT_TRUE; /* Disable auto learn on VLAN */
    }

    cpssVlanInfo.unkUcastCmd            = CPSS_PACKET_CMD_FORWARD_E;
    cpssVlanInfo.unregIpv6McastCmd      = CPSS_PACKET_CMD_FORWARD_E;
    cpssVlanInfo.unregIpv4McastCmd      = CPSS_PACKET_CMD_FORWARD_E;
    cpssVlanInfo.unregNonIpMcastCmd     = CPSS_PACKET_CMD_FORWARD_E;
    cpssVlanInfo.unregNonIpv4BcastCmd   = CPSS_PACKET_CMD_FORWARD_E;
    cpssVlanInfo.unregIpv4BcastCmd      = CPSS_PACKET_CMD_FORWARD_E;
    cpssVlanInfo.floodVidx              = 0xFFF;

    cpssVlanInfo.mirrToRxAnalyzerIndex = 0;
    cpssVlanInfo.mirrToTxAnalyzerEn = GT_FALSE;
    cpssVlanInfo.mirrToTxAnalyzerIndex = 0;
    cpssVlanInfo.fidValue = vid;
    cpssVlanInfo.unknownMacSaCmd = CPSS_PACKET_CMD_FORWARD_E;
    cpssVlanInfo.ipv4McBcMirrToAnalyzerEn = GT_FALSE;
    cpssVlanInfo.ipv4McBcMirrToAnalyzerIndex = 0;
    cpssVlanInfo.ipv6McMirrToAnalyzerEn = GT_FALSE;
    cpssVlanInfo.ipv6McMirrToAnalyzerIndex = 0;

    /* Fill ports and tagging members */
    PRV_CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portsMembers);
    PRV_CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portsTagging);
    cpssOsMemSet(&portsTaggingCmd, 0, sizeof(CPSS_DXCH_BRG_VLAN_PORTS_TAG_CMD_STC));

    /* set all ports as VLAN members */
    if (system_recovery.systemRecoveryProcess == CPSS_SYSTEM_RECOVERY_PROCESS_NOT_ACTIVE_E)
    {
        for (port = 0, maxPortNum = cpssCapMaxPortNum[devNum]; port < maxPortNum; port++)
        {
            CPSS_APPLICATION_PORT_SKIP_CHECK(devNum,port);

            /* set the port as member of vlan */
            CPSS_PORTS_BMP_PORT_SET_MAC(&portsMembers, port);

            /* Set port pvid */
            rc = cpssDxChBrgVlanPortVidSet(devNum, port, CPSS_DIRECTION_INGRESS_E,vid);
            CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChBrgVlanPortVidSet);

            portsTaggingCmd.portsCmd[port] = CPSS_DXCH_BRG_VLAN_PORT_UNTAGGED_CMD_E;
        }

        /* Write default VLAN entry */
        rc = cpssDxChBrgVlanEntryWrite(devNum, vid,
                                       &portsMembers,
                                       &portsTagging,
                                       &cpssVlanInfo,
                                       &portsTaggingCmd);

        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChBrgVlanEntryWrite);
    }
    /*********************************************************/
    /* Default TTI configuration (xCat A1 and above only):   */
    /*   - Set TTI PCL ID for IPV4 lookup to 1               */
    /*   - Set TTI PCL ID for MPLS lookup to 2               */
    /*   - Set TTI PCL ID for ETH  lookup to 3               */
    /*   - Set TTI PCL ID for MIM  lookup to 4               */
    /*********************************************************/
    if (PRV_CPSS_DXCH_XCAT_FAMILY_CHECK_MAC(devNum))
    {
        rc = cpssDxChTtiPclIdSet(devNum,CPSS_DXCH_TTI_KEY_IPV4_E,1);
        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChTtiPclIdSet);

        rc = cpssDxChTtiPclIdSet(devNum,CPSS_DXCH_TTI_KEY_MPLS_E,2);
        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChTtiPclIdSet);

        rc = cpssDxChTtiPclIdSet(devNum,CPSS_DXCH_TTI_KEY_ETH_E,3);
        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChTtiPclIdSet);

        rc = cpssDxChTtiPclIdSet(devNum,CPSS_DXCH_TTI_KEY_MIM_E,4);
        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChTtiPclIdSet);
    }

    if(PRV_CPSS_DXCH_XCAT_FAMILY_CHECK_MAC(devNum))
    {
        /* Mirror initialization -
          init Rx and Tx Global Analyzer indexes to be 0 for Ingress
          and 1 for Egress */
        /* special setting for tastBoot test */

        if(ppProfilePtr->mirrorAnalyzerPortNum == 0)
        {
            rc = cpssDxChMirrorTxGlobalAnalyzerInterfaceIndexSet(devNum,
                                                                 GT_TRUE, 1);
            CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChMirrorTxGlobalAnalyzerInterfaceIndexSet);

            rc = cpssDxChMirrorRxGlobalAnalyzerInterfaceIndexSet(devNum,
                                                                 GT_TRUE, 0);
            CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChMirrorRxGlobalAnalyzerInterfaceIndexSet);
        }

        /* eArch device configuration */
        if (PRV_CPSS_SIP_5_CHECK_MAC(devNum) &&
            !PRV_CPSS_SIP_6_CHECK_MAC(devNum))
        {
                 /* enable Rx mirroring by FDB DA lookup and set analyser index 0 */
                 rc = cpssDxChBrgFdbDaLookupAnalyzerIndexSet(devNum, GT_TRUE, 0);
                 CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChBrgFdbDaLookupAnalyzerIndexSet);

                 /* enable Rx mirroring by FDB SA lookup and set analyser index 0 */
                 rc = cpssDxChBrgFdbSaLookupAnalyzerIndexSet(devNum, GT_TRUE, 0);
                 CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChBrgFdbSaLookupAnalyzerIndexSet);
        }
    }

    /* if not Aldrin2 the function does nothing */
    rc = appPlatformDxAldrin2TailDropDbaEnableConfigure(devNum, GT_TRUE);
    if (rc != GT_OK)
    {
        return rc;
    }

    CPSS_APP_PLATFORM_LOG_INFO_MAC("General Init done...\n");
    return GT_OK;

} /* cpssAppPlatformPpGeneralInit */

/**
* @internal cpssAppPlatformPIPEngineInit function
* @endinternal
*
* @brief   ASIC specific configurations : of PIP engine
*         initialization before port configuration.
* @param[in] dev                      - devNumber
*
* @retval GT_OK                    - on success,
*/
GT_STATUS cpssAppPlatformPIPEngineInit
(
    GT_U8 dev
)
{
    GT_STATUS rc;
    GT_PHYSICAL_PORT_NUM    portNum;
    GT_U32  numOfPhysicalPorts;
    /*-------------------------------------------------------
     * CPSS-5826  Oversubscription (PIP) Engine Init Configuration
     * JUST for BC3
     *  1. Enable PIP
     *      cpssDxChPortPipGlobalEnableSet(GT_TRUE); -- default , don't configure
     *
     *
     *  2. Define thresholds :
     *      reg -0 :  0-15 - index 0   PIP Very High Priority
     *      reg -0 : 16-31 - index 1   PIP High Priority
     *      reg -1 :  0-15 - index 2   PIP Med Priority
     *      reg -1 : 16-31 - index 3   PIP Low High Priority
     *
     *
     *
     *      cpssDxChPortPipGlobalBurstFifoThresholdsSet(0,1839), very high
     *      cpssDxChPortPipGlobalBurstFifoThresholdsSet(1,1380), high
     *      cpssDxChPortPipGlobalBurstFifoThresholdsSet(2,920),  medium
     *      cpssDxChPortPipGlobalBurstFifoThresholdsSet(2,460),  low
     *
     *  3. for each port
     *             cpssDxChPortPipProfileSet(0) -- High Priority  (default)
     *             cpssDxChPortPipTrustEnableSet(FALSE)   -- Not trusted (not default)
     *     end
     *-------------------------------------------------------------------------------*/
    if (PRV_CPSS_SIP_5_20_CHECK_MAC(dev)) /* BC3 , Aldrin2 */
    {
        typedef struct
        {
            GT_U32 priority;
            GT_U32 threshold;
        }BurstFifoThreshold_STC;

        GT_U32 i;

        #define BAD_VALUE (GT_U32)(~0)

        BurstFifoThreshold_STC threshArr[] =
        {
              {         0,       1839 /* "Very High Priority"*/}
             ,{         1,       1380 /* "High Priority"     */}
             ,{         2,        920 /* "Medium Priority"   */}
             ,{         3,        460 /* "Low Priority"      */}
             ,{ BAD_VALUE,  BAD_VALUE /*  NULL               */}
        };

        /*------------------------------------------------*
         * set thresholds
         *------------------------------------------------*/
        for (i = 0; threshArr[i].priority != BAD_VALUE; i++)
        {
            rc = cpssDxChPortPipGlobalBurstFifoThresholdsSet(dev,CPSS_DATA_PATH_UNAWARE_MODE_CNS,threshArr[i].priority,threshArr[i].threshold);
            if (rc != GT_OK)
            {
                CPSS_APP_PLATFORM_LOG_ERR_MAC("cpssDxChPortPipGlobalBurstFifoThresholdsSet i=[%d], rc=%d", i, rc);
            }
        }

        if(!PRV_CPSS_SIP_6_CHECK_MAC(dev))
        {
            /*---------------------------------------------------------*
             * pass over all physical ports and set Pip Untrusted mode *
             *---------------------------------------------------------*/
           numOfPhysicalPorts =
                PRV_CPSS_DXCH_MAX_PHY_PORT_NUMBER_MAC(dev);
           for (portNum = 0; portNum < numOfPhysicalPorts; portNum++)
           {
                CPSS_APPLICATION_PORT_SKIP_CHECK(dev, portNum);

                if(prvCpssDxChPortRemotePortCheck(dev,portNum))
                {
                    continue;
                }
                rc = cpssDxChPortPipTrustEnableSet(dev,portNum,GT_FALSE);
                if (rc != GT_OK)
                {
                    CPSS_APP_PLATFORM_LOG_ERR_MAC("cpssDxChPortPipTrustEnableSet() ret=%d", rc);
                }
            }
        }
        else
        {
            /* in sip6 the per port defaults are 'untrusted'  */
            /* so no need to disable the network ports        */
        }
    }

    return GT_OK;
}

CPSS_PORT_MAC_TO_SERDES_STC  phoenix_DB_MacToSerdesMap[] =
{
    {{2, 1, 3, 0}} /* port 50,51,52,53
                           10,9, 11, 8*/
};


static CPSS_PORT_MAC_TO_SERDES_STC falcon_12_8T_DB_MacToSerdesMap[] =
{
   {{6,7,2,5,1,3,0,4}},  /* 0-7*/    /*Raven 0 */
   {{0,5,1,7,4,3,2,6}},  /* 8-15*/   /*Raven 0 */
   {{3,7,0,6,4,5,1,2}},  /*16-23*/   /*Raven 1 */
   {{1,5,3,7,0,4,2,6}},  /*24-31*/   /*Raven 1 */
   {{3,7,1,5,2,6,0,4}},  /*32-39*/   /*Raven 2 */
   {{2,4,7,5,1,3,0,6}},  /*40-47*/   /*Raven 2 */
   {{5,7,2,4,1,6,0,3}},  /*48-55*/   /*Raven 3 */
   {{0,6,3,7,2,4,1,5}},  /*56-63*/   /*Raven 3 */
   {{3,5,2,4,1,6,0,7}},  /*64-71*/   /*Raven 4 */
   {{3,5,1,7,4,0,2,6}},  /*72-79*/   /*Raven 4 */
   {{1,7,0,4,6,5,3,2}},  /*80-87*/   /*Raven 5 */
   {{1,5,3,7,0,4,2,6}},  /*88-95*/   /*Raven 5 */
   {{3,7,1,5,2,6,0,4}},  /*96-103*/  /*Raven 6 */
   {{2,4,7,5,1,3,0,6}},  /*104-111*/ /*Raven 6 */
   {{5,7,2,4,1,6,0,3}},  /*112-119*/ /*Raven 7 */
   {{0,6,3,7,2,4,1,5}},  /*120-127*/ /*Raven 7 */
   {{3,7,2,5,1,6,0,4}},  /*128-135*/ /*Raven 8 */
   {{0,5,1,7,4,3,2,6}},  /*136-143*/ /*Raven 8 */
   {{1,7,0,4,6,5,3,2}},  /*144-151*/ /*Raven 9 */
   {{1,5,3,7,0,4,2,6}},  /*152-159*/ /*Raven 9 */
   {{3,7,1,5,2,6,0,4}},  /*160-167*/ /*Raven 10*/
   {{2,4,7,5,1,3,0,6}},  /*168-175*/ /*Raven 10*/
   {{5,7,2,4,1,6,0,3}},  /*176-183*/ /*Raven 11*/
   {{0,6,3,7,2,4,1,5}},  /*184-191*/ /*Raven 11*/
   {{6,7,2,5,1,3,0,4}},  /*192-199*/ /*Raven 12*/
   {{0,5,4,7,1,3,2,6}},  /*200-207*/ /*Raven 12*/
   {{1,7,0,4,6,5,3,2}},  /*208-215*/ /*Raven 13*/
   {{1,5,3,7,0,4,2,6}},  /*216-223*/ /*Raven 13*/
   {{3,7,1,5,2,6,0,4}},  /*224-231*/ /*Raven 14*/
   {{2,7,0,5,1,3,4,6}},  /*232-239*/ /*Raven 14*/
   {{1,6,0,4,5,7,2,3}},  /*240-247*/ /*Raven 15*/
   {{0,4,1,5,2,6,3,7}}   /*248-255*/ /*Raven 15*/
};

#define ARR_PTR_AND_SIZE_MAC(myArr , arrPtr , arrLen) \
    arrPtr = &myArr[0];                               \
    arrLen = sizeof(myArr)/sizeof(myArr[0])

GT_U32                              falcon_MacToSerdesMap_arrSize;
CPSS_PORT_MAC_TO_SERDES_STC*        falcon_MacToSerdesMap = NULL;

GT_STATUS prvAppPlatformPortLaneMacToSerdesMuxSet
(
    IN  GT_U8 devNum,
    IN  CPSS_PORT_MAC_TO_SERDES_STC serdesMapArr[],
    IN  GT_U32 serdesMapArrSize
)
{
    GT_STATUS                   rc = GT_OK;
    GT_U32                      portNum, portMacNum;
    CPSS_DXCH_PORT_MAP_STC      portMap;
    CPSS_PORT_MAC_TO_SERDES_STC   macToSerdesMuxStc;
    GT_U32 maxPortNumber = cpssCapMaxPortNum[devNum];
    CPSS_PORT_MAC_TO_SERDES_STC         *phoenixMacToSerdesMapPtr;

    if(GT_TRUE == ezbIsXmlLoaded())
    {
        rc = cpssAppPlatformEzbSerdesMuxSet(devNum);
        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssAppPlatformEzbSerdesMuxSet);
    }
    else
    {
        for(portNum = 0; portNum < maxPortNumber ; portNum++)
        {
            CPSS_ENABLER_PORT_SKIP_CHECK(devNum, portNum);
            if (prvCpssDxChPortRemotePortCheck(devNum,portNum))
            {
                continue;
            }

            rc = cpssDxChPortPhysicalPortMapGet(devNum, portNum, 1, /*OUT*/&portMap);
            if(rc != GT_OK)
            {
                continue;
            }

            portMacNum = portMap.interfaceNum;

            if(serdesMapArr !=NULL)
            {
                if(serdesMapArrSize <= (portMacNum/8))
                {
                    break;
                }

                if((portMacNum % 8) != 0)
                {
                   continue;
                }

                rc = cpssDxChPortLaneMacToSerdesMuxSet(devNum, portNum,
                                                       &serdesMapArr[portMacNum / 8]);
                if (rc != GT_OK)
                {
                    CPSS_APP_PLATFORM_LOG_ERR_MAC("cpssDxChPortLaneMacToSerdesMuxSet ret=%d", rc);
                }
            }
            else
            {
                if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_AC5X_E)
                {
                    if(portMacNum != 50)
                    {
                        continue;
                    }

                    phoenixMacToSerdesMapPtr = &phoenix_DB_MacToSerdesMap[0];

                    rc = cpssDxChPortLaneMacToSerdesMuxSet(devNum, portNum,
                                                           &phoenixMacToSerdesMapPtr[0]);
                }
                else
                {
                    if(PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles == 4) /*12.8T*/
                    {
                        ARR_PTR_AND_SIZE_MAC(falcon_12_8T_DB_MacToSerdesMap, falcon_MacToSerdesMap, falcon_MacToSerdesMap_arrSize);
                    }
                    else
                    {
                        CPSS_APP_PLATFORM_LOG_INFO_MAC("serdes mux not supported\n");
                        return GT_OK;
                    }

                    if(falcon_MacToSerdesMap_arrSize <= (portMacNum/8))
                    {
                        break;
                    }
                    if((portMacNum % 8) != 0)
                    {
                       continue;
                    }

                    rc = cpssDxChPortLaneMacToSerdesMuxGet(devNum,portNum,&macToSerdesMuxStc);
                    if (rc != GT_OK)
                    {
                        CPSS_APP_PLATFORM_LOG_ERR_MAC("cpssDxChPortLaneMacToSerdesMuxGet ret=%d", rc);
                    }

                    if(cpssOsMemCmp(&macToSerdesMuxStc, &(falcon_MacToSerdesMap[portMacNum/8]), sizeof(CPSS_PORT_MAC_TO_SERDES_STC)) != 0)
                    {
                        rc = cpssDxChPortLaneMacToSerdesMuxSet(devNum,portNum,&falcon_MacToSerdesMap[portMacNum / 8]);
                        if (rc != GT_OK)
                        {
                            CPSS_APP_PLATFORM_LOG_ERR_MAC("cpssDxChPortLaneMacToSerdesMuxGet ret=%d", rc);
                       }
                    }

                }
            }
        }
    }

    return rc;
}
/**
* @internal prvAppPlatformSerdesPolarityConfigSet function
* @endinternal
*
* @brief   Per DB/RD board configure the Tx/Rx Polarity values on Serdeses
*
* @note   APPLICABLE DEVICES:      Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2.
*
* @param[in] devNum                   - device number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
static GT_STATUS prvAppPlatformSerdesPolarityConfigSet
(
    IN  GT_U8 devNum,
    IN  CPSS_APP_PLATFORM_SERDES_LANE_POLARITY_STC  polarityArr[],
    IN  GT_U32  polarityArrSize
)
{
    GT_STATUS                               rc;
    GT_U32                                  i;
    CPSS_APP_PLATFORM_SERDES_LANE_POLARITY_STC  *currentPolarityArrayPtr;

    currentPolarityArrayPtr = polarityArr;

    if(currentPolarityArrayPtr == NULL)
    {
        return GT_OK;
    }

    for (i = 0; i < polarityArrSize; i++)
    {
        rc = cpssDxChPortSerdesLanePolaritySet(devNum, 0,
                                               currentPolarityArrayPtr[i].laneNum,
                                               currentPolarityArrayPtr[i].invertTx,
                                               currentPolarityArrayPtr[i].invertRx);

        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChPortSerdesLanePolaritySet);
    }

    return GT_OK;
}

/**
* @internal cpssAppPlatformPortInterfaceInit function
* @endinternal
*
* @brief   Execute predefined ports configuration.
*
* @param[in] dev                      - device number
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS cpssAppPlatformPortInterfaceInit
(
    IN  GT_U8 dev,
    IN  CPSS_APP_PLATFORM_PP_PROFILE_STC  *ppProfilePtr
)
{
    GT_STATUS   rc;                     /* return code */

#ifndef ASIC_SIMULATION
    rc = prvAppPlatformPortLaneMacToSerdesMuxSet(dev, ppProfilePtr->serdesMapPtr, ppProfilePtr->serdesMapArrSize);
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, prvAppPlatformPortLaneMacToSerdesMuxSet);
#endif

    rc = prvAppPlatformSerdesPolarityConfigSet(dev, ppProfilePtr->polarityPtr, ppProfilePtr->polarityArrSize);
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, prvAppPlatformSerdesPolarityConfigSet);

    return GT_OK;
}

/**
* @internal cpssAppPlatformPtpConfig function
* @endinternal
*
* @brief   PTP and TAIs related configuration.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - physical device number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssAppPlatformPtpConfig
(
    IN GT_U8 devNum
)
{
    CPSS_DXCH_PTP_TAI_ID_STC        taiId;      /* TAI Units identification */
    CPSS_DXCH_PTP_TAI_TOD_STEP_STC  todStep;    /* TOD Step */
    GT_STATUS                       rc;         /* return code */
    GT_PHYSICAL_PORT_NUM            portNum;    /* port number */
    CPSS_DXCH_PTP_TSU_CONTROL_STC   control;    /* control structure */
    GT_U32                          ptpClkInKhz; /* PTP clock in KHz */
    GT_U32  numOfPhysicalPorts;

    /* Aldrin2 and Bobcat3 have 500 MHz PTP clock */
    ptpClkInKhz = 500000;

    /* Configure TAIs nanosec step values */
    taiId.taiInstance = CPSS_DXCH_PTP_TAI_INSTANCE_ALL_E;
    taiId.taiNumber = CPSS_DXCH_PTP_TAI_NUMBER_ALL_E;
    todStep.nanoSeconds = 1000000/ptpClkInKhz;
    todStep.fracNanoSeconds = (GT_U32)(0xFFFFFFFF *
                  (1000000.0/ptpClkInKhz - todStep.nanoSeconds) +
                  (1000000.0/ptpClkInKhz - todStep.nanoSeconds));

    rc = cpssDxChPtpTaiTodStepSet(devNum, &taiId, &todStep);
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChPtpTaiTodStepSet);

    numOfPhysicalPorts =
          PRV_CPSS_DXCH_MAX_PHY_PORT_NUMBER_MAC(devNum);

    for(portNum = 0; portNum < numOfPhysicalPorts; portNum++)
    {
        CPSS_APPLICATION_PORT_SKIP_CHECK(devNum, portNum);

        if(prvCpssDxChPortRemotePortCheck(devNum,portNum))
        {
            continue;
        }

        rc = cpssDxChPtpTsuControlGet(devNum, portNum, &control);
        if(rc != GT_OK)
        {
            return rc;
        }

        if(control.unitEnable == GT_FALSE)
        {
            control.unitEnable = GT_TRUE;
            /* Timestamping unit enable */
            rc = cpssDxChPtpTsuControlSet(devNum, portNum, &control);
            if(rc != GT_OK)
            {
                return rc;
            }
        }
    }

    return GT_OK;
}

/**
* @internal cpssAppPlatformAfterInitDeviceConfig function
* @endinternal
*
* @brief   Device specific configurations that should be done after device
*         initialization.
* @param[in] boardRevId               - The board revision Id.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
static GT_STATUS cpssAppPlatformAfterInitDeviceConfig
(
    IN  GT_U8   devNum,
    IN  CPSS_APP_PLATFORM_PP_PROFILE_STC  *ppProfilePtr
)
{
    GT_STATUS rc = GT_OK, rc1 = GT_OK;
    CPSS_PORTS_BMP_STC  portsMembers; /* VLAN members */
    CPSS_PORTS_BMP_STC  portsTagging; /* VLAN tagging */
    CPSS_DXCH_BRG_VLAN_INFO_STC  cpssVlanInfo;   /* cpss vlan info format    */
    CPSS_DXCH_BRG_VLAN_PORTS_TAG_CMD_STC portsTaggingCmd; /* ports tagging command */
    CPSS_PORTS_BMP_STC  portsAdditionalMembers; /* VLAN members to add */
    GT_BOOL             isValid; /* is Valid flag */
    CPSS_SYSTEM_RECOVERY_INFO_STC system_recovery; /* holds system recovery information */
    CPSS_SYSTEM_RECOVERY_INFO_STC saved_system_recovery;
    CPSS_SYSTEM_RECOVERY_INFO_STC tmp_system_recovery;
    cpssOsMemSet(&tmp_system_recovery,0,sizeof(CPSS_SYSTEM_RECOVERY_INFO_STC));
    cpssOsMemSet(&system_recovery,0,sizeof(CPSS_SYSTEM_RECOVERY_INFO_STC));
    cpssOsMemSet(&saved_system_recovery,0,sizeof(CPSS_SYSTEM_RECOVERY_INFO_STC));

    rc = cpssSystemRecoveryStateGet(&system_recovery);
    if (rc != GT_OK)
    {
        return rc;
    }

    if (CPSS_SYSTEM_RECOVERY_PROCESS_NOT_ACTIVE_E != system_recovery.systemRecoveryProcess)
    {
        tmp_system_recovery.systemRecoveryProcess = CPSS_SYSTEM_RECOVERY_PROCESS_HA_E;
        tmp_system_recovery.systemRecoveryState = CPSS_SYSTEM_RECOVERY_INIT_STATE_E;
        tmp_system_recovery.systemRecoveryMode = system_recovery.systemRecoveryMode;
        rc =  cpssSystemRecoveryStateSet(&tmp_system_recovery);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    rc = cpssAppPlatformPIPEngineInit(devNum);
    if(rc != GT_OK)
    {
        rc =  cpssSystemRecoveryStateSet(&system_recovery);
        if (rc != GT_OK)
        {
            return rc;
        }
        return rc;
    }

    /* After all CG MAC related configuration were done,
        it is needed to disable GC MAC UNIT in order to reduce power consumption.
        Only requested CG MACs will be enabled during appDemoBc2PortInterfaceInit */

    /* Bobcat3 CG MAC unit disable */
    rc = cpssSystemRecoveryStateGet(&tmp_system_recovery);
    if (rc != GT_OK)
    {
        return rc;
    }
    saved_system_recovery = tmp_system_recovery;

    tmp_system_recovery.systemRecoveryProcess = CPSS_SYSTEM_RECOVERY_PROCESS_NOT_ACTIVE_E;
    tmp_system_recovery.systemRecoveryState = CPSS_SYSTEM_RECOVERY_COMPLETION_STATE_E;
    rc =  cpssSystemRecoveryStateSet(&tmp_system_recovery);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = prvCpssDxChCgMacUnitDisable(devNum, GT_FALSE);

    rc1 =  cpssSystemRecoveryStateSet(&saved_system_recovery);
    if (rc1 != GT_OK)
    {
        return rc1;
    }
    if(GT_OK != rc)
    {
        return rc;
    }

    rc = cpssAppPlatformPortInterfaceInit(devNum, ppProfilePtr);
    if(rc != GT_OK)
    {
        rc1 =  cpssSystemRecoveryStateSet(&system_recovery);
        if (rc1 != GT_OK)
        {
            return rc1;
        }
        return rc;
    }

    if (CPSS_SYSTEM_RECOVERY_PROCESS_NOT_ACTIVE_E != system_recovery.systemRecoveryProcess)
    {
        rc =  cpssSystemRecoveryStateSet(&system_recovery);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    /* clear additional default VLAN members */
    PRV_CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portsAdditionalMembers);

    if(PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.gop_ilkn.supported == GT_TRUE)
    {
        GT_PHYSICAL_PORT_NUM    chId;

        /* add ILKN channels to default VLAN, although they are not mapped yet */
        for(chId = 128; chId < 192; chId++)
        {
            /* set the port as member of vlan */
            CPSS_PORTS_BMP_PORT_SET_MAC(&portsAdditionalMembers, chId);
        }
    }

    rc = cpssSystemRecoveryStateGet(&system_recovery);
    if (rc != GT_OK)
    {
        return rc;
    }
    if (system_recovery.systemRecoveryProcess == CPSS_SYSTEM_RECOVERY_PROCESS_NOT_ACTIVE_E)
        {
        /* What is spl with port 83? */
        /* add port 83 to default VLAN, although it could be not mapped */
        CPSS_PORTS_BMP_PORT_SET_MAC(&portsAdditionalMembers, 83);

        /* read VLAN entry */
        rc = cpssDxChBrgVlanEntryRead(devNum, 1, &portsMembers, &portsTagging,
                                        &cpssVlanInfo, &isValid, &portsTaggingCmd);
        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChBrgVlanEntryRead);

        /* add new ports as members, portsTaggingCmd is default - untagged */
        CPSS_PORTS_BMP_BITWISE_OR_MAC(&portsMembers, &portsMembers, &portsAdditionalMembers);

        /* Write default VLAN entry */
        rc = cpssDxChBrgVlanEntryWrite(devNum, 1,
                                        &portsMembers,
                                        &portsTagging,
                                        &cpssVlanInfo,
                                        &portsTaggingCmd);

        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChBrgVlanEntryWrite);
    }

    /* PTP (and TAIs) configurations */
    if (system_recovery.systemRecoveryProcess != CPSS_SYSTEM_RECOVERY_PROCESS_FAST_BOOT_E)
    {
        /* PTP (and TAIs) configurations */
        rc = cpssAppPlatformPtpConfig(devNum);
        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, prvBobcat2PtpConfig);
    }

    CPSS_APP_PLATFORM_LOG_INFO_MAC("After Device Init done...\n");

    return GT_OK;
}

/**
* @internal cpssAppPlatformAfterInitConfig function
* @endinternal
*
* @brief   After init configurations of PP device.
*
* @param[in] devNum                - Device Number.
* @param [in] *ppProfileInfo     - PP profile
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS cpssAppPlatformAfterInitConfig
(
    IN GT_U8                              devNum,
    IN CPSS_APP_PLATFORM_PP_PROFILE_STC  *ppProfilePtr
)
{
    GT_STATUS rc = GT_OK;
    GT_U32    portNum, maxPortNum;

    rc = cpssAppPlatformAfterInitDeviceConfig(devNum, ppProfilePtr);
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssAppPlatformAfterInitDeviceConfig);

    rc = cpssDxChCfgDevEnable(devNum, GT_TRUE);
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChCfgDevEnable);

    for (portNum = 0, maxPortNum = cpssCapMaxPortNum[devNum]; portNum < maxPortNum; portNum++)
    {
        CPSS_APPLICATION_PORT_SKIP_CHECK(devNum, portNum);

        rc = cpssDxChPortForceLinkDownEnableSet(devNum, portNum, GT_FALSE);
        if (rc != GT_OK)
        {
            return rc;
        }

        rc = cpssDxChNstPortEgressFrwFilterSet(devNum, portNum, CPSS_NST_EGRESS_FRW_FILTER_FROM_CPU_E, GT_FALSE);
        if (rc != GT_OK)
        {
            return rc;
        }

        rc = cpssDxChNstPortIngressFrwFilterSet(devNum, portNum, CPSS_NST_INGRESS_FRW_FILTER_TO_CPU_E, GT_FALSE);
        if (rc != GT_OK)
        {
            return rc;
        }

        rc =  cpssDxChBrgFdbNaToCpuPerPortSet(devNum,portNum,GT_TRUE);
        if (rc != GT_OK)
        {
            return rc;
        }

        rc =  cpssDxChBrgFdbNaStormPreventSet(devNum,portNum,GT_TRUE);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    if(PRV_CPSS_SIP_5_CHECK_MAC(devNum) == GT_TRUE)
    {
        GT_U32  regAddr;
        regAddr = PRV_DXCH_REG1_UNIT_FDB_MAC(devNum).FDBCore.FDBGlobalConfig.FDBGlobalConfig;
        rc = prvCpssHwPpSetRegField(devNum, regAddr, 30, 1, 0);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    return rc;
}

static GT_VOID findMinMaxPositionByLedIf(GT_U32 ledIf,
                                            CPSS_APP_PLATFORM_MAC_LEDPOSITION_STC *ledPositionTbl,
                                            GT_U32 *minPositionPtr,
                                            GT_U32 *maxPositionPtr)
{
    GT_U32 i;
    GT_U32  minPosition = (GT_U32) (-1);
    GT_U32  maxPosition = 0;

    for (i = 0 ; ledPositionTbl[i].ledIf !=  APP_PLATFORM_BAD_VALUE; i++)
    {
        if (ledPositionTbl[i].ledIf == ledIf)
        {
            if (ledPositionTbl[i].ledPosition < minPosition)
            {
                minPosition = ledPositionTbl[i].ledPosition;
            }
            if (ledPositionTbl[i].ledPosition > maxPosition)
            {
                maxPosition = ledPositionTbl[i].ledPosition;
            }
        }
    }

    *minPositionPtr = minPosition;
    *maxPositionPtr = maxPosition;
}

static GT_U32 findLedPositionByMac(GT_U32 mac, CPSS_APP_PLATFORM_MAC_LEDPOSITION_STC *ledPositionTbl)
{
    GT_U32 i;
    for (i = 0 ; ledPositionTbl[i].portMac !=  APP_PLATFORM_BAD_VALUE; i++)
    {
        if (ledPositionTbl[i].portMac == mac)
        {
            return ledPositionTbl[i].ledPosition;
        }
    }
    return APP_PLATFORM_BAD_VALUE;
}

/************Falcon LED config*************/
#ifndef GM_USED
/*
 * typedef: struct CPSS_FALCON_LED_STREAM_INDICATIONS_STC
 *
 * Description:
 *      Positions of LED bit indications in stream.
 *
 * Fields:
 *
 *      ledStart            - The first bit in the LED stream indication to be driven in current chiplet
 *                            (APPLICABLE RANGES: 0..255)
 *      ledEnd              - The last bit in the LED stream indication to be driven in the current chiplet
 *                            (APPLICABLE RANGES: 0..255)
 *      cpuPort             - CPU port is connected to the current chiplet
 */
typedef struct {
    GT_U32     ledStart;
    GT_U32     ledEnd;
    GT_BOOL    cpuPort;
} CPSS_FALCON_LED_STREAM_INDICATIONS_STC;

static const CPSS_FALCON_LED_STREAM_INDICATIONS_STC falcon_12_8_led_indications[CPSS_CHIPLETS_MAX_NUM_CNS] =
{
    {64, 65, GT_FALSE},   /* Raven  0 - 2 LED ports */
    {64, 66, GT_TRUE },   /* Raven  1 - 2 LED ports + CPU Port */
    {64, 66, GT_TRUE },   /* Raven  2 - 2 LED ports + CPU Port */
    {64, 65, GT_FALSE},   /* Raven  3 - 2 LED ports */
    {64, 65, GT_FALSE},   /* Raven  4 - 2 LED ports */
    {64, 65, GT_FALSE},   /* Raven  5 - 2 LED ports */
    {64, 65, GT_FALSE},   /* Raven  6 - 2 LED ports */
    {64, 65, GT_FALSE},   /* Raven  7 - 2 LED ports */
    {64, 65, GT_FALSE},   /* Raven  8 - 2 LED ports */
    {64, 65, GT_FALSE},   /* Raven  9 - 2 LED ports */
    {64, 65, GT_FALSE},   /* Raven 10 - 2 LED ports */
    {64, 65, GT_FALSE},   /* Raven 11 - 2 LED ports */
    {64, 65, GT_FALSE},   /* Raven 12 - 2 LED ports */
    {64, 65, GT_FALSE},   /* Raven 13 - 2 LED ports */
    {64, 65, GT_FALSE},   /* Raven 14 - 2 LED ports */
    {64, 65, GT_FALSE}    /* Raven 15 - 2 LED ports */
};

static const CPSS_FALCON_LED_STREAM_INDICATIONS_STC falcon_6_4_led_indications[CPSS_CHIPLETS_MAX_NUM_CNS] =
{
    {64, 66, GT_TRUE },   /* Raven  0 - 2 LED ports + CPU Port */
    {64, 65, GT_FALSE},   /* Raven  1 - 2 LED ports */
    {64, 65, GT_FALSE},   /* Raven  2 - 2 LED ports */
    {64, 66, GT_TRUE },   /* Raven  3 - 2 LED ports + CPU Port */
    {64, 66, GT_TRUE },   /* Raven  4 - 2 LED ports + CPU Port */
    {64, 65, GT_FALSE},   /* Raven  5 - 2 LED ports */
    {64, 65, GT_FALSE},   /* Raven  6 - 2 LED ports */
    {64, 66, GT_TRUE },   /* Raven  7 - 2 LED ports + CPU Port */
};

static const CPSS_FALCON_LED_STREAM_INDICATIONS_STC falcon_6_4_reduced_led_indications[CPSS_CHIPLETS_MAX_NUM_CNS] =
{
    {64, 66, GT_TRUE },   /* Raven  0 - 2 LED ports + CPU Port */
    { 0,  0, GT_FALSE},   /* Raven  1 - not used */
    {64, 65, GT_FALSE},   /* Raven  2 - 2 LED ports */
    {64, 65, GT_FALSE},   /* Raven  3 - 2 LED ports */
    { 0,  0, GT_FALSE},   /* Raven  4 - not used */
    {64, 66, GT_TRUE },   /* Raven  5 - 2 LED ports + CPU Port */
    { 0,  0, GT_FALSE},   /* Raven  6 - not used */
    { 0,  0, GT_FALSE},   /* Raven  7 - not used */
};

static const CPSS_FALCON_LED_STREAM_INDICATIONS_STC falcon_2T_4T_led_indications[CPSS_CHIPLETS_MAX_NUM_CNS] =
{
    {64, 79, GT_FALSE},   /* Raven  0 - 16 LED ports */
    { 0,  0, GT_FALSE},   /* Raven  1 - not used */
    {64, 79, GT_FALSE},   /* Raven  2 - 16 LED ports */
    { 0,  0, GT_FALSE},   /* Raven  3 - not used */
    {64, 79, GT_FALSE},   /* Raven  4 - 16 LED ports */
    {64, 80, GT_TRUE },   /* Raven  5 - 16 LED ports + CPU Port */
    { 0,  0, GT_FALSE},   /* Raven  6 - not used */
    {64, 80, GT_TRUE },   /* Raven  7 - 16 LED ports + CPU Port */
};

#define LED_BAD_VALUE (GT_U32)(~0)
/**
* @internal led_cpu_port_mac_check function
* @endinternal
*
* @brief   Checks if current MAC port is CPU port
*
* @param[in] devNum                - The CPSS devNum.
*
* @retval GT_TRUE                  - the mac port is CPU port
* @retval GT_FAIL                  - otherwise.
*
*/
static GT_BOOL led_cpu_port_mac_check(GT_U32 portMacNum, GT_U32 *cpuPortsArray)
{
    GT_U32 ii;
    for (ii = 0; ii < CPSS_CHIPLETS_MAX_NUM_CNS; ii++)
    {
        if (cpuPortsArray[ii] == portMacNum)
        {
            return GT_TRUE;
        }
    }
    return GT_FALSE;
}

/**
* @internal led_port_mac_raven_with_cpu
* @endinternal
*
* @brief   Checks if current MAC port and CPU port together in the same Raven
*
* @param[in] portMacNum            - current MAC to check.
* @param[in] cpuPortsArray         - CPU ports array
*
* @retval GT_TRUE                  - the mac port and CPU ports are in the same Raven
* @retval GT_FAIL                  - the mac port and CPU ports are NOT in the same Raven.
*
*/
static GT_BOOL led_port_mac_raven_with_cpu(GT_U32 portMacNum, GT_U32 *cpuPortsArray)
{
    GT_U32 raven = portMacNum / CPSS_CHIPLETS_MAX_NUM_CNS;
    /* current mac not in Raven with connected CPU port */
    if (cpuPortsArray[raven] == 0xFFFFFFFF)
    {
        return GT_FALSE;
    }
    return GT_TRUE;
}

/**
* @internal first_port_in_raven
* @endinternal
*
* @brief   Checks if current MAC port is the first 400G port in Raven
*
* @param[in] portMacNum            - current MAC to check.
*
* @retval GT_TRUE                  - the mac port is the first 400G port in Raven.
* @retval GT_FAIL                  - the mac port is NOT the first 400G port in Raven.
*
*/
static GT_BOOL first_port_in_raven(GT_U32 portMacNum)
{
    GT_STATUS rc = (((portMacNum / 8) % 2) == 0) ? GT_TRUE : GT_FALSE;

    return rc;
}

/**
* @internal led_port_position_get_mac
* @endinternal
*
* @brief   Gets LED ports position for the current MAC port
*
* @param[in] devNum                - device number
* @param[in] portMacNum            - current MAC to check
* @param[in] cpuPortsArray         - CPU ports array
*
* @retval GT_U32                   - the LED ports position for the current MAC port.
*
*/
static GT_U32 led_port_position_get_mac(GT_U8 devNum, GT_U32 portMacNum, GT_U32 *cpuPortsArray)
{
    GT_U32 firstLedPosition, secondLedPosition;


    switch(PRV_CPSS_PP_MAC(devNum)->devType)
    {
        case CPSS_FALCON_2T_4T_PACKAGE_DEVICES_CASES_MAC:
            return portMacNum % 16;
        default:
            if (portMacNum % 8 == 0)
            {
                if (led_port_mac_raven_with_cpu(portMacNum, cpuPortsArray) == GT_FALSE)
                {
                    if (PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles == 4)
                    {
                        firstLedPosition =  0;
                        secondLedPosition = 1;
                    }
                    else
                    {
                        firstLedPosition =  1;
                        secondLedPosition = 0;
                    }
                }
                else
                {
                    if (PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles == 4)
                    {
                        firstLedPosition =  0;
                        secondLedPosition = 1;
                    }
                    else
                    {
                        firstLedPosition =  2;
                        secondLedPosition = 1;
                    }
                }

                return first_port_in_raven(portMacNum) ? firstLedPosition : secondLedPosition;
            }
            else
            {
                return LED_BAD_VALUE;
            }
        }
}

/**
* @internal led_cpu_port_position_get_mac
* @endinternal
*
* @brief   Gets CPU LED ports position for the current CPU port
*
* @param[in] devNum                - device number
*
* @retval GT_U32                   - the LED ports position for the current CPU port.
*
*/
static GT_U32 led_cpu_port_position_get_mac(GT_U8 devNum)
{
    GT_U32 cpuLedPosition;

    switch(PRV_CPSS_PP_MAC(devNum)->devType)
    {
        case CPSS_FALCON_2T_4T_PACKAGE_DEVICES_CASES_MAC:
            cpuLedPosition = 16;
            break;
        default:
            if (PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles == 4)
            {
                cpuLedPosition = 2;
            }
            else
            {
                cpuLedPosition = 0;
            }
    }

    return cpuLedPosition;
}

/**
* @internal falconLedInit function
* @endinternal
*
* @brief   LED configurations
*
* @param[in] devNum                - The CPSS devNum.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*
*/
static GT_STATUS falcon_LedInit
(
    GT_U8    devNum
)
{
    GT_STATUS                       rc;
    GT_U32                          classNum;
    GT_PHYSICAL_PORT_NUM            portNum;
    CPSS_LED_CONF_STC               ledConfig;
    CPSS_LED_CLASS_MANIPULATION_STC ledClassManip;
    GT_U32                          position;
    GT_U32                          tileIndex;
    GT_U32                          ravenIndex;
    GT_U32                          ledUnit;
    CPSS_DXCH_PORT_MAP_STC          portMap;
    GT_U32                          portMacNum;
    static const CPSS_FALCON_LED_STREAM_INDICATIONS_STC * ledStreamIndication;
    GT_U32                          cpuPorts[CPSS_CHIPLETS_MAX_NUM_CNS] = {0};
    GT_U32                          cpuPortMacFirst;
    CPSS_SYSTEM_RECOVERY_INFO_STC   system_recovery; /* holds system recovery information */

    #define FIRST_PORT_IN_RAVEN(portMacNum) \
            first_port_in_raven(portMacNum)

    /* Macro returns LED position per MTI400 ports
      (two ports per Raven, one LED per port) */
    #define LED_PORT_POSITION_GET_MAC(portMacNum) \
            led_port_position_get_mac(devNum, portMacNum, cpuPorts);

    /* Macro returns true if mac port is CPU and false otherwise */
    #define LED_CPU_PORT_MAC(portMacNum)    \
            led_cpu_port_mac_check(portMacNum, cpuPorts)

    /* Macro returns LED position per CPU ports */
    #define LED_CPU_PORT_POSITION_GET_MAC(portMacNum) \
            led_cpu_port_position_get_mac(devNum)

    cpssOsMemSet(&ledConfig, 0, sizeof(CPSS_LED_CONF_STC));
    cpssOsMemSet(&cpuPorts,  0xFF, sizeof(cpuPorts));

    ledConfig.ledOrganize                       = CPSS_LED_ORDER_MODE_BY_CLASS_E;                       /* LedControl.OrganizeMode     [ 1: 1] =   1  by class       */
    ledConfig.sip6LedConfig.ledClockFrequency   = 1627;                                                 /* LedChainClockControl.ledClkOutDiv  [11: 22] = 512 (app_clock - 833333/led_clock_out - 1627)  */
    ledConfig.pulseStretch                      = CPSS_LED_PULSE_STRETCH_1_E;                           /* LedControl.Pulse-stretch-div[ 0: 29] =  100000 (ledClockFrequencyDivider - 10 * 10000  */

    ledConfig.blink0Duration          = CPSS_LED_BLINK_DURATION_1_E;    /* 64 ms */                     /* BlinkDivision0.blink0Duration  [ 0: 29]  = 0x32DCD40 (64/app_clock in mSec)*/
    ledConfig.blink0DutyCycle         = CPSS_LED_BLINK_DUTY_CYCLE_1_E;  /* 50%   */                     /* blinkGlobalControl.Blink0-Duty [ 3: 4]  = 1 25%  */
    ledConfig.blink1Duration          = CPSS_LED_BLINK_DURATION_1_E;    /* 64 ms */                     /* BlinkDivision1.blink0Duration  [ 0: 29]  = 0x32DCD40 (64/app_clock in mSec)*/
    ledConfig.blink1DutyCycle         = CPSS_LED_BLINK_DUTY_CYCLE_1_E;  /* 50%   */                     /* blinkGlobalControl.Blink1-Duty [ 8: 9]  = 1 25%  */

    ledConfig.disableOnLinkDown       = GT_FALSE;  /* don't care , see led-class manipulation */
    ledConfig.clkInvert               = GT_FALSE;  /* don't care */
    ledConfig.class5select            = CPSS_LED_CLASS_5_SELECT_FIBER_LINK_UP_E;   /* don't care */
    ledConfig.class13select           = CPSS_LED_CLASS_13_SELECT_COPPER_LINK_UP_E; /* don't care */

    rc = cpssSystemRecoveryStateGet(&system_recovery);
    if (rc != GT_OK)
    {
      return rc;
    }

    switch(PRV_CPSS_PP_MAC(devNum)->devType)
    {
        case CPSS_98CX8540_CNS:
        case CPSS_98CX8540_H_CNS:
        case CPSS_98CX8522_CNS:
            ledStreamIndication = falcon_6_4_led_indications;
            cpuPortMacFirst     = 128;
            break;
        case CPSS_98EX5610_CNS:
            ledStreamIndication = falcon_6_4_reduced_led_indications;
            cpuPortMacFirst     = 128;
            break;
        case CPSS_FALCON_2T_4T_PACKAGE_DEVICES_CASES_MAC:
            ledStreamIndication = falcon_2T_4T_led_indications;
            cpuPortMacFirst     = 128;
            break;
        default:
            ledStreamIndication = falcon_12_8_led_indications;
            cpuPortMacFirst     = 256;
    }

    for (tileIndex = 0; tileIndex < PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles; tileIndex++)
    {
        for(ravenIndex = 0; ravenIndex < FALCON_RAVENS_PER_TILE; ravenIndex++)
        {
            ledUnit = PRV_DXCH_LED_NUM_MAC(tileIndex, ravenIndex);

            /* Start of LED stream location - Class 2 */
            ledConfig.sip6LedConfig.ledStart[ledUnit] = ledStreamIndication[ledUnit].ledStart;    /* LedControl.LedStart[5:12] */
            /* End of LED stream location: LED ports 0, 1 or 0, 1 and 2*/
            ledConfig.sip6LedConfig.ledEnd[ledUnit] = ledStreamIndication[ledUnit].ledEnd;        /* LedControl.LedEnd[5:12] */

            if (ledStreamIndication[ledUnit].cpuPort == GT_TRUE)
            {
                cpuPorts[ledUnit] = cpuPortMacFirst + ledUnit;
            }
            /* The LED unit is included into LED chain */
            ledConfig.sip6LedConfig.ledChainBypass[ledUnit] = GT_FALSE;
        }
    }

    rc = cpssDxChLedStreamConfigSet(devNum, 0, &ledConfig);
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChLedStreamConfigSet);

    ledClassManip.invertEnable            = GT_FALSE; /* not relevant for Falcon */
    ledClassManip.blinkSelect             = CPSS_LED_BLINK_SELECT_0_E;                            /* blinkGlobalControl.Blink Sel            [10-17] = 0 blink-0         */
    ledClassManip.forceEnable             = GT_FALSE;                                             /* classesAndGroupConfig.Force En          [24-29] = pos = classNum = 0*/
    ledClassManip.forceData               = 0;                                                    /* classForcedData[class]                  [ 0:31] = 0                 */
    ledClassManip.pulseStretchEnable      = GT_FALSE;                                             /* classGlobalControl.stretch class        [ 6-11] = pos = classNum = 0/1*/

    for (classNum = 0 ; classNum < 6; classNum++)
    {
        if(classNum == 2)
        {
            ledClassManip.disableOnLinkDown       = GT_TRUE;                                  /* classGlobalControl.disable on link down [ 0- 5] = pos = classNum = 0*/
            ledClassManip.blinkEnable             = GT_TRUE;                                  /* blinkGlobalControl.blinkEn     [18-25] = pos = classNum val = 1 */
        }                                                                                     /* blinkGlobalControl.Blink Select[10-17] = pos = classNum val = 0 */
        else
        {
            ledClassManip.disableOnLinkDown       = GT_FALSE;
            ledClassManip.blinkEnable             = GT_FALSE;
        }
        rc = cpssDxChLedStreamClassManipulationSet(devNum, 0,
                                                   CPSS_DXCH_LED_PORT_TYPE_XG_E /* don't care , not applicable for Falcon */
                                                   ,classNum ,&ledClassManip);
        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChLedStreamClassManipulationSet);
    }
    /*--------------------------------------------------------------------------------------------------------*
     * LED. classesAndGroupConfig uses default setting , don't configure   cpssDxChLedStreamGroupConfigSet()  *
     *--------------------------------------------------------------------------------------------------------*
     * now configure port dependent LED configurations                                                        *
     *       - port led position (stream is hard wired                                                        *
     *           (One LED per 400G port, two 400G per Raven)                                                  *
     *           The port LED positions for full chip is as follows:                                          *
     *           P31,                   Raven 15    Position 1                                                *
     *           P30,                   Raven 15    Position 0                                                *
     *           P29,                   Raven 14    Position 1                                                *
     *           P28,                   Raven 14    Position 0                                                *
     *           ...,                                                                                         *
     *           HOST1,                 Raven 2     Position 2                                                *
     *           P5,                    Raven 2     Position 1                                                *
     *           P4,                    Raven 2     Position 0                                                *
     *           HOST0,                 Raven 1     Position 2                                                *
     *           P3,                    Raven 1     Position 1                                                *
     *           P2,                    Raven 1     Position 0                                                *
     *           P1,                    Raven 0     Position 1                                                *
     *           P0                     Raven 0     Position 0                                                *
     *                                                                                                        *
     *       - invert polarity                                                                                *
     *--------------------------------------------------------------------------------------------------------*/
    for(portNum = 0; portNum < 256; portNum++)
    {
        CPSS_ENABLER_PORT_SKIP_CHECK(devNum, portNum);

        if (prvCpssDxChPortRemotePortCheck(devNum,portNum))
        {
            continue;
        }

        rc = cpssDxChPortPhysicalPortMapGet(devNum, portNum, 1, /*OUT*/&portMap);
        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChPortPhysicalPortMapGet);

        portMacNum = portMap.interfaceNum;
        position = (LED_CPU_PORT_MAC(portMacNum)) ? LED_CPU_PORT_POSITION_GET_MAC(portMacNum) :  LED_PORT_POSITION_GET_MAC(portMacNum);

        if (position != LED_BAD_VALUE)
        {
            rc = cpssDxChLedStreamPortPositionSet(devNum, portNum, position);
            CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChLedStreamPortPositionSet);

            /*skip if in recovery mode HA */
            if (system_recovery.systemRecoveryProcess != CPSS_SYSTEM_RECOVERY_PROCESS_HA_E)
            {
                rc = cpssDxChLedStreamPortClassPolarityInvertEnableSet(devNum, portNum, /*classNum*/2, /*invertEnable*/1);
                CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChLedStreamPortClassPolarityInvertEnableSet);
            }
        }
    }

    return GT_OK;
}
#endif
/************Falcon LED config*************/

/**
* @internal appPlatformLedInterfacesInit function
* @endinternal
*
* @brief   LED Init configurations
*
* @param [in] deviceNumber       - CPSS device number,
* @param [in] *ppProfileInfo     - PP profile
*
* @retval GT_OK                  - on success,
* @retval GT_FAIL                - otherwise.
*/

GT_STATUS appPlatformLedInterfacesInit
(
    IN GT_U8                             devNum,
    IN CPSS_APP_PLATFORM_PP_PROFILE_STC *ppProfilePtr
)
{
    GT_STATUS                       rc = GT_OK;
    GT_U32                          ledInterfaceNum;
    GT_U32                          classNum;
    GT_PHYSICAL_PORT_NUM            portNum;
    CPSS_LED_CONF_STC               ledConfig;
    CPSS_LED_CLASS_MANIPULATION_STC ledClassManip;
    GT_U32                          position;
    CPSS_DXCH_PORT_MAP_STC          portMap;
    CPSS_APP_PLATFORM_MAC_LEDPOSITION_STC
                                   *ledPositionTbl;
    GT_U32                          numOfPhysicalPorts;
    PRV_CPSS_DXCH_PP_CONFIG_STC *pDev;

    pDev = PRV_CPSS_DXCH_PP_MAC(devNum);

    if(ppProfilePtr->ledPositionTable != NULL)
    {
        CPSS_APP_PLATFORM_LOG_INFO_MAC("LED Init\n");
        ledPositionTbl = ppProfilePtr->ledPositionTable;
    }
    else
    {
        if (pDev->genInfo.devFamily == CPSS_PP_FAMILY_DXCH_FALCON_E)
        {
#ifndef GM_USED
            rc = falcon_LedInit(CAST_SW_DEVNUM(devNum));
            CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, falcon_LedInit);
#endif
        }
        else
        {
            CPSS_APP_PLATFORM_LOG_INFO_MAC("LED Init - Skipping: LED Table Not Found\n");
        }
        return GT_OK;
    }

    ledConfig.ledOrganize             = ppProfilePtr->ledOrganize;
    ledConfig.ledClockFrequency       = ppProfilePtr->ledClockFrequency;
    ledConfig.invertEnable            = ppProfilePtr->invertEnable;
    ledConfig.pulseStretch            = ppProfilePtr->pulseStretch;
    ledConfig.blink0Duration          = ppProfilePtr->blink0Duration;
    ledConfig.blink0DutyCycle         = ppProfilePtr->blink0DutyCycle;
    ledConfig.blink1Duration          = ppProfilePtr->blink1Duration;
    ledConfig.blink1DutyCycle         = ppProfilePtr->blink1DutyCycle;
    ledConfig.disableOnLinkDown       = ppProfilePtr->disableOnLinkDown;
    ledConfig.clkInvert               = ppProfilePtr->clkInvert;
    ledConfig.class5select            = ppProfilePtr->class5select;
    ledConfig.class13select           = ppProfilePtr->class13select;

    if(pDev->genInfo.devFamily == CPSS_PP_FAMILY_DXCH_ALDRIN_E || pDev->genInfo.devFamily == CPSS_PP_FAMILY_DXCH_AC3X_E )
    {
        ledConfig.ledStart                = ppProfilePtr->ledStart;
        ledConfig.ledEnd                  = ppProfilePtr->ledEnd;
        for(ledInterfaceNum = 0; ledInterfaceNum < PRV_CPSS_DXCH_ALDRIN_LED_IF_NUM_CNS; ledInterfaceNum++)
        {
            rc = cpssDxChLedStreamConfigSet(devNum, ledInterfaceNum, &ledConfig);
            if(rc != GT_OK)
            {
                return rc;
            }
        }
        ledClassManip.invertEnable            = ppProfilePtr->invertionEnable;
        ledClassManip.blinkSelect             = ppProfilePtr->blinkSelect;
        ledClassManip.forceEnable             = ppProfilePtr->forceEnable;
        ledClassManip.forceData               = ppProfilePtr->forceData;
        ledClassManip.pulseStretchEnable      = ppProfilePtr->pulseStretchEnable;
        ledClassManip.disableOnLinkDown       = ppProfilePtr->disableOnLinkDown;

        for(ledInterfaceNum = 0; ledInterfaceNum < PRV_CPSS_DXCH_ALDRIN_LED_IF_NUM_CNS; ledInterfaceNum++)
        {
            for (classNum = 0; classNum < 6; classNum++)
            {
                if(classNum == 2)
                {
                    ledClassManip.blinkEnable             = GT_TRUE;
                }
                else
                {
                    ledClassManip.blinkEnable             = GT_FALSE;
                }
                rc = cpssDxChLedStreamClassManipulationSet(devNum
                                                            ,ledInterfaceNum
                                                            ,CPSS_DXCH_LED_PORT_TYPE_XG_E /* don't care for Aldrin */
                                                            ,classNum
                                                            ,&ledClassManip);
                if(rc != GT_OK)
                {
                    return rc;
                }
            }
        }
    }
    else
    {
        for(ledInterfaceNum = 0; ledInterfaceNum < PRV_CPSS_DXCH_BOBCAT3_LED_IF_NUM_CNS; ledInterfaceNum++)
        {
            findMinMaxPositionByLedIf(ledInterfaceNum,
                                    ledPositionTbl,
                                    &ledConfig.ledStart,
                                    &ledConfig.ledEnd);

            ledConfig.ledStart += ppProfilePtr->ledStart;
            ledConfig.ledEnd += ppProfilePtr->ledEnd;

            rc = cpssDxChLedStreamConfigSet(devNum,ledInterfaceNum, &ledConfig);
            if(rc != GT_OK)
            {
                return rc;
            }
        }
        ledClassManip.invertEnable            = ppProfilePtr->invertionEnable;
        ledClassManip.blinkSelect             = ppProfilePtr->blinkSelect;
        ledClassManip.forceEnable             = ppProfilePtr->forceEnable;
        ledClassManip.forceData               = ppProfilePtr->forceData;
        ledClassManip.pulseStretchEnable      = ppProfilePtr->pulseStretchEnable;

        for(ledInterfaceNum = 0; ledInterfaceNum < PRV_CPSS_DXCH_BOBCAT3_LED_IF_NUM_CNS; ledInterfaceNum++)
        {
            for (classNum = 0 ; classNum < 6; classNum++)
            {
                if(classNum == 2)
                {
                    ledClassManip.disableOnLinkDown       = GT_TRUE;
                    ledClassManip.blinkEnable             = GT_TRUE;
                }
                else
                {
                    ledClassManip.disableOnLinkDown       = GT_FALSE;
                    ledClassManip.blinkEnable             = GT_FALSE;
                }
                rc = cpssDxChLedStreamClassManipulationSet(devNum
                                                            ,ledInterfaceNum
                                                            ,CPSS_DXCH_LED_PORT_TYPE_XG_E
                                                            ,classNum
                                                            ,&ledClassManip);
                if(rc != GT_OK)
                {
                    return rc;
                }
            }
        }
    }

    /*--------------------------------------------------------------------------------------------------------*
     * LED. classesAndGroupConfig uses default setting , don't configure   cpssDxChLedStreamGroupConfigSet()  *
     *--------------------------------------------------------------------------------------------------------*
     * now configure port dependent LED configurations                                                        *
     *       - port led position (stream is hard wired                                                        *
     *       - invert polarity                                                                                *
     *--------------------------------------------------------------------------------------------------------*/
    numOfPhysicalPorts =
        PRV_CPSS_DXCH_MAX_PHY_PORT_NUMBER_MAC(devNum);
    for(portNum = 0; portNum < numOfPhysicalPorts; portNum++)
    {
        CPSS_APPLICATION_PORT_SKIP_CHECK(devNum, portNum);

        if (prvCpssDxChPortRemotePortCheck(devNum,portNum))
        {
            continue;
        }

        rc = cpssDxChPortPhysicalPortMapGet(devNum, portNum, 1, /*OUT*/&portMap);
        if(rc != GT_OK)
        {
            continue;
        }

        position = findLedPositionByMac(portMap.interfaceNum, ledPositionTbl);
        if (position != APP_PLATFORM_BAD_VALUE)
        {
            rc = cpssDxChLedStreamPortPositionSet(devNum, portNum, position);
            if(rc != GT_OK)
            {
                return rc;
            }
            if(pDev->genInfo.devFamily == CPSS_PP_FAMILY_DXCH_ALDRIN2_E)
            {
                rc = cpssDxChLedStreamPortClassPolarityInvertEnableSet(devNum,portNum,/*classNum*/2,/*invertEnable*/1);
                if(rc != GT_OK)
                {
                    return rc;
                }
            }
        }
    }

    return rc;
}

#define PRV_MEM_FREE_NON_NULL_AND_SET_NULL_MAC(y, rc) \
if(y)                                             \
{                                                 \
  if(GT_OK == osCacheDmaFree(y))                  \
     y = NULL;                                    \
  else                                            \
  {                                               \
     rc = GT_FAIL;                                \
  }                                               \
}

GT_STATUS prvCpssAppPlatformDeAllocateDmaMem
(
    IN GT_U8 devNum
)
{
    GT_STATUS rc = GT_OK;
    GT_U32    ii = 0;
    GT_U32    txQue = 0;
    GT_U32    rxQue = 0;

    if(!PRV_CPSS_IS_DEV_EXISTS_MAC(devNum))
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

    if(cpssAppPlatformPpPhase2Db[devNum].useMultiNetIfSdma == GT_TRUE)
    {
        for(ii = 0; ii < CPSS_MAX_SDMA_CPU_PORTS_CNS; ii++)
        {
            if( ii > 0 && (!PRV_CPSS_SIP_5_20_CHECK_MAC(devNum)) )
            {
                continue;
            }

            for(txQue = 0; txQue < CPSS_MAX_TX_QUEUE_CNS; txQue++)
            {
                PRV_MEM_FREE_NON_NULL_AND_SET_NULL_MAC(
                     cpssAppPlatformPpPhase2Db[devNum].multiNetIfCfg.txSdmaQueuesConfig[ii][txQue].memData.staticAlloc.buffAndDescMemPtr, rc)
                CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, osCacheDmaFree);
            }

            for(rxQue = 0; rxQue < CPSS_MAX_RX_QUEUE_CNS; rxQue++)
            {

                PRV_MEM_FREE_NON_NULL_AND_SET_NULL_MAC(
                     cpssAppPlatformPpPhase2Db[devNum].multiNetIfCfg.rxSdmaQueuesConfig[ii][rxQue].descMemPtr, rc)
                CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, osCacheDmaFree);

                PRV_MEM_FREE_NON_NULL_AND_SET_NULL_MAC(
                     cpssAppPlatformPpPhase2Db[devNum].multiNetIfCfg.rxSdmaQueuesConfig[ii][rxQue].memData.staticAlloc.buffMemPtr, rc)
                CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, osCacheDmaFree);

            }
        }
    }

    PRV_MEM_FREE_NON_NULL_AND_SET_NULL_MAC(cpssAppPlatformPpPhase2Db[devNum].auqCfg.auDescBlock, rc)
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, osCacheDmaFree);

    PRV_MEM_FREE_NON_NULL_AND_SET_NULL_MAC(cpssAppPlatformPpPhase2Db[devNum].fuqCfg.auDescBlock, rc)
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, osCacheDmaFree);

    cpssOsMemSet(&(cpssAppPlatformPpPhase2Db[devNum]), 0, sizeof(CPSS_DXCH_PP_PHASE2_INIT_INFO_STC));

#ifdef CHX_FAMILY
    if (GT_FALSE == HWINIT_GLOVAR(
       sysGenGlobalInfo.reInitializationDbPtr[devNum].deviceDidHwReset))
    {
        /* state to the special DB that the dma memory is deleted */
        rc = prvCpssPpConfigDevDbHwResetSet(devNum, GT_TRUE);
        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, prvCpssPpConfigDevDbHwResetSet);
    }
#endif

    return rc;
}

GT_STATUS prvCpssAppPlatformPpPhase1ConfigClear
(
    IN GT_U8 devNum
)
{
    GT_STATUS rc = GT_OK;
    (void)devNum;
#ifndef ASIC_SIMULATION
    rc = extDrvPexRemove (PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[devNum]->hwInfo[0].hwAddr.busNo,
                          PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[devNum]->hwInfo[0].hwAddr.devSel,
                          PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[devNum]->hwInfo[0].hwAddr.funcNo);
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, extDrvPexRemove);
#endif

    return rc;
}

GT_STATUS cpssAppPlatformTcamLibParamsGet
(
    OUT CPSS_APP_PLATFORM_TCAM_LIB_INFO_STC *tcamInfo
)
{
   tcamInfo->ttiTcamUseOffset                     = applicationPlatformTtiTcamUseOffset ;
   tcamInfo->pclTcamUseIndexConversion            = applicationPlatformPclTcamUseIndexConversion ;
   tcamInfo->tcamPclRuleBaseIndexOffset           = applicationPlatformTcamPclRuleBaseIndexOffset ;
   tcamInfo->tcamIpcl0RuleBaseIndexOffset         = applicationPlatformTcamIpcl0RuleBaseIndexOffset ;
   tcamInfo->tcamIpcl1RuleBaseIndexOffset         = applicationPlatformTcamIpcl1RuleBaseIndexOffset ;
   tcamInfo->tcamIpcl2RuleBaseIndexOffset         = applicationPlatformTcamIpcl2RuleBaseIndexOffset ;
   tcamInfo->tcamEpclRuleBaseIndexOffset          = applicationPlatformTcamEpclRuleBaseIndexOffset ;
   tcamInfo->tcamIpcl0MaxNum                      = applicationPlatformTcamIpcl0MaxNum ;
   tcamInfo->tcamIpcl1MaxNum                      = applicationPlatformTcamIpcl1MaxNum ;
   tcamInfo->tcamIpcl2MaxNum                      = applicationPlatformTcamIpcl2MaxNum ;
   tcamInfo->tcamEpclMaxNum                       = applicationPlatformTcamEpclMaxNum  ;
   tcamInfo->tcamTtiHit0RuleBaseIndexOffset       = applicationPlatformTcamTtiHit0RuleBaseIndexOffset;
   tcamInfo->tcamTtiHit1RuleBaseIndexOffset       = applicationPlatformTcamTtiHit1RuleBaseIndexOffset;
   tcamInfo->tcamTtiHit2RuleBaseIndexOffset       = applicationPlatformTcamTtiHit2RuleBaseIndexOffset;
   tcamInfo->tcamTtiHit3RuleBaseIndexOffset       = applicationPlatformTcamTtiHit3RuleBaseIndexOffset;
   tcamInfo->tcamTtiHit0MaxNum                    = applicationPlatformTcamTtiHit0MaxNum ;
   tcamInfo->tcamTtiHit1MaxNum                    = applicationPlatformTcamTtiHit1MaxNum ;
   tcamInfo->tcamTtiHit2MaxNum                    = applicationPlatformTcamTtiHit2MaxNum ;
   tcamInfo->tcamTtiHit3MaxNum                    = applicationPlatformTcamTtiHit3MaxNum ;
   tcamInfo->save_tcamTtiHit0RuleBaseIndexOffset  = save_applicationPlatformTcamTtiHit0RuleBaseIndexOffset;
   tcamInfo->save_tcamTtiHit1RuleBaseIndexOffset  = save_applicationPlatformTcamTtiHit1RuleBaseIndexOffset;
   tcamInfo->save_tcamTtiHit2RuleBaseIndexOffset  = save_applicationPlatformTcamTtiHit2RuleBaseIndexOffset;
   tcamInfo->save_tcamTtiHit3RuleBaseIndexOffset  = save_applicationPlatformTcamTtiHit3RuleBaseIndexOffset;
   tcamInfo->save_tcamTtiHit0MaxNum               = save_applicationPlatformTcamTtiHit0MaxNum;
   tcamInfo->save_tcamTtiHit1MaxNum               = save_applicationPlatformTcamTtiHit1MaxNum;
   tcamInfo->save_tcamTtiHit2MaxNum               = save_applicationPlatformTcamTtiHit2MaxNum;
   tcamInfo->save_tcamTtiHit3MaxNum               = save_applicationPlatformTcamTtiHit3MaxNum;
   tcamInfo->save_tcamIpcl0RuleBaseIndexOffset    = save_applicationPlatformTcamIpcl0RuleBaseIndexOffset;
   tcamInfo->save_tcamIpcl1RuleBaseIndexOffset    = save_applicationPlatformTcamIpcl1RuleBaseIndexOffset;
   tcamInfo->save_tcamIpcl2RuleBaseIndexOffset    = save_applicationPlatformTcamIpcl2RuleBaseIndexOffset;
   tcamInfo->save_tcamEpclRuleBaseIndexOffset     = save_applicationPlatformTcamEpclRuleBaseIndexOffset;
   tcamInfo->save_tcamIpcl0MaxNum                 = save_applicationPlatformTcamIpcl0MaxNum;
   tcamInfo->save_tcamIpcl1MaxNum                 = save_applicationPlatformTcamIpcl1MaxNum;
   tcamInfo->save_tcamIpcl2MaxNum                 = save_applicationPlatformTcamIpcl2MaxNum;
   tcamInfo->save_tcamEpclMaxNum                  = save_applicationPlatformTcamEpclMaxNum;
   return GT_OK;
}

