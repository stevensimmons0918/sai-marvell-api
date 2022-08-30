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
* @file cpssAppPlatformProfileMgr.c
*
* @brief Library to manage profiles.
*
* @version   1
********************************************************************************
*/

#include <profiles/cpssAppPlatformProfile.h>
#include <cpssAppPlatformSysConfig.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>
/**** DSs *****/
static GT_U32                         cap_profile_count = 0;
static GT_U32                         last_used_profile = 0;

static GT_CHAR*                       cap_profile_name_arr[CPSS_APP_PLATFORM_MAX_PROFILES_CNS] = {0,};
static GT_CHAR*                       cap_profile_descr_arr[CPSS_APP_PLATFORM_MAX_PROFILES_CNS] = {0,};
static CPSS_APP_PLATFORM_PROFILE_STC* cap_profile_list_arr[CPSS_APP_PLATFORM_MAX_PROFILES_CNS] = {0,};

/**** DSs for testing/Validation *****/
static GT_U32                         cap_test_profile_count = 0;
static GT_CHAR*                       cap_test_profile_name_arr[CPSS_APP_PLATFORM_MAX_PROFILES_CNS] = {0,};
static GT_CHAR*                       cap_test_profile_descr_arr[CPSS_APP_PLATFORM_MAX_PROFILES_CNS] = {0,};
static CPSS_APP_PLATFORM_PROFILE_STC* cap_test_profile_list_arr[CPSS_APP_PLATFORM_MAX_PROFILES_CNS] = {0,};

typedef struct _CAP_PROFILE_DB_STC {
    GT_CHAR*   ppProfileStr[CPSS_APP_PLATFORM_MAX_PP_CNS];
    GT_CHAR*   runtimeProfileStr;
} CAP_PROFILE_DB_STC;

CAP_PROFILE_DB_STC cap_profile_db = {{0,},0};

CAP_PROFILE_DB_STC cap_test_profile_db = {{0,},0};

/**** functions *****/

GT_STATUS
cpssAppPlatformProfileDbEntryGet
(
    IN  GT_U8                                       devNum,
    IN  CPSS_APP_PLATFORM_PROFILE_DB_ENTRY_TYPE_ENT entryType,
    OUT GT_CHAR                                   **profileStr
)
{
    if (devNum >= CPSS_APP_PLATFORM_MAX_PP_CNS)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

    if (!profileStr)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PTR, LOG_ERROR_NO_MSG);

    if (entryType & CPSS_APP_PLATFORM_PROFILE_DB_ENTRY_TYPE_PP_E)
    {
        if ( cap_profile_db.ppProfileStr[devNum] )
        {
            *profileStr =  cap_profile_db.ppProfileStr[devNum];
            return GT_OK;
        }
    }

    if (entryType & CPSS_APP_PLATFORM_PROFILE_DB_ENTRY_TYPE_RT_E)
    {
        if (cap_profile_db.runtimeProfileStr)
        {
            *profileStr = cap_profile_db.runtimeProfileStr;
            return GT_OK;
        }
    }

    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
}


GT_STATUS
cpssAppPlatformProfileDbReset
(
    GT_VOID
)
{
    GT_U32 i;

    for( i=0; i<CPSS_APP_PLATFORM_MAX_PP_CNS; i++)
    {

        cap_profile_db.ppProfileStr[i] = NULL;
    }

    for( i=0; i<CPSS_APP_PLATFORM_MAX_PP_CNS; i++)
    {

        cap_test_profile_db.ppProfileStr[i] = NULL;
    }

    cap_profile_db.runtimeProfileStr = NULL;
    cap_test_profile_db.runtimeProfileStr = NULL;

    return GT_OK;
}

GT_STATUS
cpssAppPlatformProfileDbDelete
(
    IN  GT_U8                                       devNum,
    IN  CPSS_APP_PLATFORM_PROFILE_DB_ENTRY_TYPE_ENT entryType
)
{
    if (devNum >= CPSS_APP_PLATFORM_MAX_PP_CNS)
       CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

    if (entryType & CPSS_APP_PLATFORM_PROFILE_DB_ENTRY_TYPE_PP_E)
    {
       cap_profile_db.ppProfileStr[devNum] = NULL;
       cap_test_profile_db.ppProfileStr[devNum] = NULL;
    }

    if (entryType & CPSS_APP_PLATFORM_PROFILE_DB_ENTRY_TYPE_RT_E)
    {
       cap_profile_db.runtimeProfileStr = NULL;
       cap_test_profile_db.runtimeProfileStr = NULL;
    }

    return GT_OK;
}

GT_STATUS
cpssAppPlatformProfileDbAdd
(
    IN  GT_U8                                        devNum,
    IN  CPSS_APP_PLATFORM_PROFILE_STC               *profilePtr,
    IN  CPSS_APP_PLATFORM_PROFILE_DB_ENTRY_TYPE_ENT  entryType
)
{
    GT_U32 i;

    if (devNum >= CPSS_APP_PLATFORM_MAX_PP_CNS)
       CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

    if (!profilePtr)
       CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PTR, LOG_ERROR_NO_MSG);

    for (i=0; i<cap_profile_count; i++)
    {
        if (cap_profile_list_arr[i] == profilePtr && cap_profile_name_arr[i])
        {
            if (entryType & CPSS_APP_PLATFORM_PROFILE_DB_ENTRY_TYPE_PP_E)
                cap_profile_db.ppProfileStr[devNum] = cap_profile_name_arr[i];
            else if (entryType & CPSS_APP_PLATFORM_PROFILE_DB_ENTRY_TYPE_RT_E)
                cap_profile_db.runtimeProfileStr = cap_profile_name_arr[i];
            else
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);

            return GT_OK;
        }
    }

    for (i=0; i<cap_test_profile_count; i++)
    {
        if (cap_test_profile_list_arr[i] == profilePtr && cap_test_profile_name_arr[i])
        {
            if (entryType & CPSS_APP_PLATFORM_PROFILE_DB_ENTRY_TYPE_PP_E)
                cap_test_profile_db.ppProfileStr[devNum] = cap_test_profile_name_arr[i];
            else if (entryType & CPSS_APP_PLATFORM_PROFILE_DB_ENTRY_TYPE_RT_E)
                cap_test_profile_db.runtimeProfileStr = cap_test_profile_name_arr[i];
            else
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);

            return GT_OK;
        }
    }

    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
}


GT_U32
get_cap_profile_count
(
   GT_VOID
)
{
   return  cap_profile_count;
}

GT_VOID
increment_cap_profile_count
(
   GT_VOID
)
{
   cap_profile_count++;
}

GT_VOID
set_cap_profile_name
(
   IN GT_U32 count,
   IN GT_CHAR* name
)
{
    cap_profile_name_arr[count] = name;
}

GT_VOID
set_cap_profile_descr
(
   IN GT_U32 count,
   IN GT_CHAR* descr
)
{
    cap_profile_descr_arr[count] = descr;
}

GT_VOID
set_cap_profile_list
(
   IN GT_U32                         count,
   IN CPSS_APP_PLATFORM_PROFILE_STC* name
)
{
    cap_profile_list_arr[count] = name;
}

GT_U32
get_cap_test_profile_count
(
   GT_VOID
)
{
   return  cap_test_profile_count;
}

GT_VOID
increment_cap_test_profile_count
(
   GT_VOID
)
{
   cap_test_profile_count++;
}

GT_VOID
set_cap_test_profile_name
(
   IN GT_U32 count,
   IN GT_CHAR* name
)
{
    cap_test_profile_name_arr[count] = name;
}

GT_VOID
set_cap_test_profile_descr
(
   IN GT_U32 count,
   IN GT_CHAR* descr
)
{
    cap_test_profile_descr_arr[count] = descr;
}

GT_VOID
set_cap_test_profile_list
(
   IN GT_U32                         count,
   IN CPSS_APP_PLATFORM_PROFILE_STC* name
)
{
    cap_test_profile_list_arr[count] = name;
}

GT_STATUS
cpssAppPlatformShowProfileCount
(
   GT_VOID
)
{
    CPSS_APP_PLATFORM_LOG_PRINT_MAC("\nprofile count =  %d\n", cap_profile_count);
    return GT_OK;
}

/*****************************************************************************
 * Profile Print Functions
 *****************************************************************************/

GT_VOID
prvCpssAppPlatformPrintPpProfile
(
   IN  CPSS_APP_PLATFORM_PP_PROFILE_STC  *profilePtr
)
{
    CPSS_APP_PLATFORM_LOG_PRINT_MAC("PP Params:\n");

    CPSS_APP_PLATFORM_LOG_PRINT_MAC("PP Phase-1 Params:\n");
    CPSS_APP_PLATFORM_LOG_PRINT_MAC("\tcoreClock: %d\n", profilePtr->coreClock);
    CPSS_APP_PLATFORM_LOG_PRINT_MAC("\tisrAddrCompletionRegionsBmp: %d\n", profilePtr->isrAddrCompletionRegionsBmp);
    CPSS_APP_PLATFORM_LOG_PRINT_MAC("\tappAddrCompletionRegionsBmp: %d\n", profilePtr->appAddrCompletionRegionsBmp);
    CPSS_APP_PLATFORM_LOG_PRINT_MAC("\tnumOfPortGroups: %d\n", profilePtr->numOfPortGroups);
    CPSS_APP_PLATFORM_LOG_PRINT_MAC("\tmaxNumOfPhyPortsToUse: %d\n", profilePtr->maxNumOfPhyPortsToUse);
    CPSS_APP_PLATFORM_LOG_PRINT_MAC("\ttcamParityCalcEnable: %d\n", profilePtr->tcamParityCalcEnable);
    CPSS_APP_PLATFORM_LOG_PRINT_MAC("\tcpssWaNum: %d\n", profilePtr->cpssWaNum);
    CPSS_APP_PLATFORM_LOG_PRINT_MAC("\tapEnable: %d\n", profilePtr->apEnable);
    CPSS_APP_PLATFORM_LOG_PRINT_MAC("\tnumOfDataIntegrityElements: %d\n", profilePtr->numOfDataIntegrityElements);

    CPSS_APP_PLATFORM_LOG_PRINT_MAC("PP Phase-2 Params:\n");
    CPSS_APP_PLATFORM_LOG_PRINT_MAC("\tnewDevNum: %d\n", profilePtr->newDevNum);

    CPSS_APP_PLATFORM_LOG_PRINT_MAC("AU Queue Params:\n");
    CPSS_APP_PLATFORM_LOG_PRINT_MAC("\tauDescNum: %d\n", profilePtr->auDescNum);
    CPSS_APP_PLATFORM_LOG_PRINT_MAC("\tuseDoubleAuq: %d\n", profilePtr->useDoubleAuq);
    CPSS_APP_PLATFORM_LOG_PRINT_MAC("\tuseSecondaryAuq: %d\n", profilePtr->useSecondaryAuq);
    CPSS_APP_PLATFORM_LOG_PRINT_MAC("\tfuDescNum: %d\n", profilePtr->fuDescNum);
    CPSS_APP_PLATFORM_LOG_PRINT_MAC("\tfuqUseSeparate: %d\n", profilePtr->fuqUseSeparate);
    CPSS_APP_PLATFORM_LOG_PRINT_MAC("\tnoTraffic2CPU: %d\n", profilePtr->noTraffic2CPU);

    CPSS_APP_PLATFORM_LOG_PRINT_MAC("SDMA Params:\n");
    CPSS_APP_PLATFORM_LOG_PRINT_MAC("\tnetifSdmaPortGroupId: %d\n", profilePtr->netifSdmaPortGroupId);
    CPSS_APP_PLATFORM_LOG_PRINT_MAC("\tuseMultiNetIfSdma: %d\n", profilePtr->useMultiNetIfSdma);
    CPSS_APP_PLATFORM_LOG_PRINT_MAC("\ttxDescNum: %d\n", profilePtr->txDescNum);
    CPSS_APP_PLATFORM_LOG_PRINT_MAC("\trxDescNum: %d\n", profilePtr->rxDescNum);
    CPSS_APP_PLATFORM_LOG_PRINT_MAC("\trxBuffersInCachedMem: %d\n", profilePtr->rxBuffersInCachedMem);
    CPSS_APP_PLATFORM_LOG_PRINT_MAC("\trxHeaderOffset: %d\n", profilePtr->rxHeaderOffset);
    CPSS_APP_PLATFORM_LOG_PRINT_MAC("\ttxGenQueueNum: %d\n", profilePtr->txGenQueueNum);

    CPSS_APP_PLATFORM_LOG_PRINT_MAC("MII Params:\n");
    CPSS_APP_PLATFORM_LOG_PRINT_MAC("\t:miiTxDescNum %d\n", profilePtr->miiTxDescNum);
    CPSS_APP_PLATFORM_LOG_PRINT_MAC("\t:miiTxBufBlockSize %d\n", profilePtr->miiTxBufBlockSize);
    CPSS_APP_PLATFORM_LOG_PRINT_MAC("\tmiiRxBufSize: %d\n", profilePtr->miiRxBufSize);
    CPSS_APP_PLATFORM_LOG_PRINT_MAC("\tmiiRxBufBlockSize: %d\n", profilePtr->miiRxBufBlockSize);
    CPSS_APP_PLATFORM_LOG_PRINT_MAC("\tmiiRxHeaderOffset: %d\n", profilePtr->miiRxHeaderOffset);

    CPSS_APP_PLATFORM_LOG_PRINT_MAC("Led Init Params:\n");
    CPSS_APP_PLATFORM_LOG_PRINT_MAC("\tinvertEnable: %d\n", profilePtr->invertEnable);
    CPSS_APP_PLATFORM_LOG_PRINT_MAC("\tdisableOnLinkDown: %d\n", profilePtr->disableOnLinkDown);
    CPSS_APP_PLATFORM_LOG_PRINT_MAC("\tclkInvert: %d\n", profilePtr->clkInvert);
    CPSS_APP_PLATFORM_LOG_PRINT_MAC("\tledStart: %d\n", profilePtr->ledStart);
    CPSS_APP_PLATFORM_LOG_PRINT_MAC("\tledEnd: %d\n", profilePtr->ledEnd);

    CPSS_APP_PLATFORM_LOG_PRINT_MAC("Led Class Manip Config. Params:\n");
    CPSS_APP_PLATFORM_LOG_PRINT_MAC("\tinvertionEnable: %d\n", profilePtr->invertionEnable);
    CPSS_APP_PLATFORM_LOG_PRINT_MAC("\tforceEnable: %d\n", profilePtr->forceEnable);
    CPSS_APP_PLATFORM_LOG_PRINT_MAC("\tforceData: %d\n", profilePtr->forceData);
    CPSS_APP_PLATFORM_LOG_PRINT_MAC("\tpulseStretchEnable: %d\n", profilePtr->pulseStretchEnable);

    CPSS_APP_PLATFORM_LOG_PRINT_MAC("Logical Init Params:\n");
    CPSS_APP_PLATFORM_LOG_PRINT_MAC("\tmaxNumOfPbrEntries: %d\n", profilePtr->maxNumOfPbrEntries);

    CPSS_APP_PLATFORM_LOG_PRINT_MAC("General Init Params:\n");
    CPSS_APP_PLATFORM_LOG_PRINT_MAC("\tctrlMacLearn: %d\n", profilePtr->ctrlMacLearn);
    CPSS_APP_PLATFORM_LOG_PRINT_MAC("\tflowControlDisable: %d\n", profilePtr->flowControlDisable);
    CPSS_APP_PLATFORM_LOG_PRINT_MAC("\tmodeFcHol: %d\n", profilePtr->modeFcHol);
    CPSS_APP_PLATFORM_LOG_PRINT_MAC("\tmirrorAnalyzerPortNum: %d\n", profilePtr->mirrorAnalyzerPortNum);

    CPSS_APP_PLATFORM_LOG_PRINT_MAC("\tLib Init:\n");
    CPSS_APP_PLATFORM_LOG_PRINT_MAC("\tinitBridge: %d\n", profilePtr->initBridge);
    CPSS_APP_PLATFORM_LOG_PRINT_MAC("\tinitIp: %d\n", profilePtr->initIp);
    CPSS_APP_PLATFORM_LOG_PRINT_MAC("\tinitMirror: %d\n", profilePtr->initMirror);
    CPSS_APP_PLATFORM_LOG_PRINT_MAC("\tinitNetIf: %d\n", profilePtr->initNetIf);
    CPSS_APP_PLATFORM_LOG_PRINT_MAC("\tinitPhy: %d\n", profilePtr->initPhy);
    CPSS_APP_PLATFORM_LOG_PRINT_MAC("\tinitPort: %d\n", profilePtr->initPort);
    CPSS_APP_PLATFORM_LOG_PRINT_MAC("\tinitTrunk: %d\n", profilePtr->initTrunk);
    CPSS_APP_PLATFORM_LOG_PRINT_MAC("\tinitPcl: %d\n", profilePtr->initPcl);
    CPSS_APP_PLATFORM_LOG_PRINT_MAC("\tinitTcam: %d\n", profilePtr->initTcam);
    CPSS_APP_PLATFORM_LOG_PRINT_MAC("\tinitPolicer: %d\n", profilePtr->initPolicer);
    CPSS_APP_PLATFORM_LOG_PRINT_MAC("\tinitPha: %d\n", profilePtr->initPha);

    CPSS_APP_PLATFORM_LOG_PRINT_MAC("IP Lib Params:\n");
    CPSS_APP_PLATFORM_LOG_PRINT_MAC("\tusePolicyBasedRouting: %d\n", profilePtr->usePolicyBasedRouting);
    CPSS_APP_PLATFORM_LOG_PRINT_MAC("\tmaxNumOfIpv4Prefixes: %d\n", profilePtr->maxNumOfIpv4Prefixes);
    CPSS_APP_PLATFORM_LOG_PRINT_MAC("\tmaxNumOfIpv4McEntries: %d\n", profilePtr->maxNumOfIpv4McEntries);
    CPSS_APP_PLATFORM_LOG_PRINT_MAC("\tmaxNumOfIpv6Prefixes: %d\n", profilePtr->maxNumOfIpv6Prefixes);
    CPSS_APP_PLATFORM_LOG_PRINT_MAC("\tlpmDbFirstTcamLine: %d\n", profilePtr->lpmDbFirstTcamLine);
    CPSS_APP_PLATFORM_LOG_PRINT_MAC("\tlpmDbLastTcamLine: %d\n", profilePtr->lpmDbLastTcamLine);
    CPSS_APP_PLATFORM_LOG_PRINT_MAC("\tlpmDbSupportIpv4: %d\n", profilePtr->lpmDbSupportIpv4);
    CPSS_APP_PLATFORM_LOG_PRINT_MAC("\tlpmDbSupportIpv6: %d\n", profilePtr->lpmDbSupportIpv6);
    CPSS_APP_PLATFORM_LOG_PRINT_MAC("\tlpmDbPartitionEnable: %d\n", profilePtr->lpmDbPartitionEnable);

    CPSS_APP_PLATFORM_LOG_PRINT_MAC("Trunk Lib Params:\n");
    CPSS_APP_PLATFORM_LOG_PRINT_MAC("\tnumOfTrunks: %d\n", profilePtr->numOfTrunks);

    CPSS_APP_PLATFORM_LOG_PRINT_MAC("Pha Lib Params:\n");
    CPSS_APP_PLATFORM_LOG_PRINT_MAC("\tpha_packetOrderChangeEnable: %d\n", profilePtr->pha_packetOrderChangeEnable);

    CPSS_APP_PLATFORM_LOG_PRINT_MAC("Bridge Lib Params:\n");
    CPSS_APP_PLATFORM_LOG_PRINT_MAC("\tpolicerMruSupported: %d\n", profilePtr->policerMruSupported);
}

GT_VOID
prvCpssAppPlatformPrintRunTimeProfile
(
   IN  CPSS_APP_PLATFORM_RUNTIME_PROFILE_STC  *profilePtr
)
{
  (void)profilePtr;
}

GT_STATUS
prvCpssAppPlatformPrintBoardLedProfile
(
   IN  CPSS_APP_PLATFORM_BOARD_PARAM_LED_STC      *ledPtr
)
{
    GT_U32              i           = 0;
    CPSS_LED_CONF_STC  *ledConfPtr  = NULL;
    CPSS_LED_CLASS_MANIPULATION_STC
        *ledClassPtr = NULL;
    CPSS_LED_SIP6_CONF_STC
        *sip6        = NULL;

    if (!ledPtr)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

    ledConfPtr  = &ledPtr->ledConf;
    ledClassPtr = &ledPtr->ledClass;
    sip6 = &ledPtr->ledConf.sip6LedConfig;

    CPSS_APP_PLATFORM_LOG_PRINT_MAC("Board:LED\n");
    CPSS_APP_PLATFORM_LOG_PRINT_MAC("\tCPSS Dev Num: %d\n", ledPtr->devNum);
    CPSS_APP_PLATFORM_LOG_PRINT_MAC("\tLED Itf Num: %d\n", ledPtr->ledInterfaceNum);

    CPSS_APP_PLATFORM_LOG_PRINT_MAC("\tBoard:LED:Conf\n");
    CPSS_APP_PLATFORM_LOG_PRINT_MAC("\t\tledOrganize:%d\n", ledConfPtr->ledOrganize);
    CPSS_APP_PLATFORM_LOG_PRINT_MAC("\t\tdisableOnLinkDown:%d\n", ledConfPtr->disableOnLinkDown);
    CPSS_APP_PLATFORM_LOG_PRINT_MAC("\t\tblink0DutyCycle:%d\n", ledConfPtr->blink0DutyCycle);
    CPSS_APP_PLATFORM_LOG_PRINT_MAC("\t\tblink0Duration:%d\n", ledConfPtr->blink0Duration);
    CPSS_APP_PLATFORM_LOG_PRINT_MAC("\t\tblink1DutyCycle:%d\n", ledConfPtr->blink1DutyCycle);
    CPSS_APP_PLATFORM_LOG_PRINT_MAC("\t\tblink1Duration:%d\n", ledConfPtr->blink1Duration);
    CPSS_APP_PLATFORM_LOG_PRINT_MAC("\t\tpulseStretch:%d\n", ledConfPtr->pulseStretch);
    CPSS_APP_PLATFORM_LOG_PRINT_MAC("\t\tledStart:%d\n", ledConfPtr->ledStart);
    CPSS_APP_PLATFORM_LOG_PRINT_MAC("\t\tledEnd:%d\n", ledConfPtr->ledEnd);
    CPSS_APP_PLATFORM_LOG_PRINT_MAC("\t\tclkInvert:%d\n", ledConfPtr->clkInvert);
    CPSS_APP_PLATFORM_LOG_PRINT_MAC("\t\tclass5select:%d\n", ledConfPtr->class5select);
    CPSS_APP_PLATFORM_LOG_PRINT_MAC("\t\tclass13select:%d\n", ledConfPtr->class13select);
    CPSS_APP_PLATFORM_LOG_PRINT_MAC("\t\tinvertEnable:%d\n", ledConfPtr->invertEnable);
    CPSS_APP_PLATFORM_LOG_PRINT_MAC("\t\tledClockFrequency:%d\n", ledConfPtr->ledClockFrequency);

    CPSS_APP_PLATFORM_LOG_PRINT_MAC("\t\tBoard:LED:Conf:SIP6\n");
    CPSS_APP_PLATFORM_LOG_PRINT_MAC("\t\t\tledClockFrequency:%d\n", sip6->ledClockFrequency);

    for (; i< CPSS_CHIPLETS_MAX_NUM_CNS; i++)
    {
        CPSS_APP_PLATFORM_LOG_PRINT_MAC("\t\t\tledStart[%2d] = %d\n", i, sip6->ledStart[i]);
        CPSS_APP_PLATFORM_LOG_PRINT_MAC("\t\t\tledEnd[%2d] = %d\n", i, sip6->ledEnd[i]);
    }

    CPSS_APP_PLATFORM_LOG_PRINT_MAC("\tBoard:LED:ClassManipulation\n");
    CPSS_APP_PLATFORM_LOG_PRINT_MAC("\t\tinvertEnable;:%d\n", ledClassPtr->invertEnable);
    CPSS_APP_PLATFORM_LOG_PRINT_MAC("\t\tblinkEnable;:%d\n", ledClassPtr->blinkEnable);
    CPSS_APP_PLATFORM_LOG_PRINT_MAC("\t\tblinkSelect;:%d\n", ledClassPtr->blinkSelect);
    CPSS_APP_PLATFORM_LOG_PRINT_MAC("\t\tforceEnable;:%d\n", ledClassPtr->forceEnable);
    CPSS_APP_PLATFORM_LOG_PRINT_MAC("\t\tforceData;:%d\n", ledClassPtr->forceData);
    CPSS_APP_PLATFORM_LOG_PRINT_MAC("\t\tpulseStretchEnable;:%d\n", ledClassPtr->pulseStretchEnable);
    CPSS_APP_PLATFORM_LOG_PRINT_MAC("\t\tdisableOnLinkDown;:%d\n", ledClassPtr->disableOnLinkDown);

    return GT_OK;
}

GT_STATUS
prvCpssAppPlatformPrintBoardProfile
(
   IN  CPSS_APP_PLATFORM_BOARD_PROFILE_STC  *profilePtr
)
{
    if(!profilePtr)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

    switch (profilePtr->boardInfoType)
    {
        case CPSS_APP_PLATFORM_BOARD_PARAM_GENERIC_E:
            CPSS_APP_PLATFORM_LOG_PRINT_MAC("Board:Generic\n");
            CPSS_APP_PLATFORM_LOG_PRINT_MAC("\tName: %s\n", profilePtr->boardParam.boardPtr->boardName);
            CPSS_APP_PLATFORM_LOG_PRINT_MAC("\tosCallType: %d\n", profilePtr->boardParam.boardPtr->osCallType);
            CPSS_APP_PLATFORM_LOG_PRINT_MAC("\tdrvCallType: %d\n", profilePtr->boardParam.boardPtr->extDrvCallType);
            break;
        case CPSS_APP_PLATFORM_BOARD_PARAM_PP_MAP_E:
            CPSS_APP_PLATFORM_LOG_PRINT_MAC("Board:PpMap\n");
            CPSS_APP_PLATFORM_LOG_PRINT_MAC("\tCPSS Dev Num: %d\n", profilePtr->boardParam.ppMapPtr->devNum);
            CPSS_APP_PLATFORM_LOG_PRINT_MAC("\tMapType: %d\n", profilePtr->boardParam.ppMapPtr->mapType);
            switch (profilePtr->boardParam.ppMapPtr->mapType)
            {
                case CPSS_APP_PLATFORM_PP_MAP_TYPE_FIXED_E:
                    switch (profilePtr->boardParam.ppMapPtr->mngInterfaceType)
                    {
                        case CPSS_CHANNEL_PEX_MBUS_E:
                        case CPSS_CHANNEL_PEX_E:
                        case CPSS_CHANNEL_PEX_EAGLE_E:
                        case CPSS_CHANNEL_PEX_FALCON_Z_E:
                        case CPSS_CHANNEL_PCI_E:
                        case CPSS_CHANNEL_PEX_KERNEL_E:
                            CPSS_APP_PLATFORM_LOG_PRINT_MAC("\tPCI Bus Id: %d\n", profilePtr->boardParam.ppMapPtr->mngInterfaceAddr.pciAddr.busId);
                            CPSS_APP_PLATFORM_LOG_PRINT_MAC("\tPCI Dev Id: %d\n", profilePtr->boardParam.ppMapPtr->mngInterfaceAddr.pciAddr.deviceId);
                            CPSS_APP_PLATFORM_LOG_PRINT_MAC("\tPCI Fn. Id: %d\n", profilePtr->boardParam.ppMapPtr->mngInterfaceAddr.pciAddr.functionId);
                            break;
                        case CPSS_CHANNEL_SMI_E:
                            CPSS_APP_PLATFORM_LOG_PRINT_MAC("\tSMI Bus Id: %d\n", profilePtr->boardParam.ppMapPtr->mngInterfaceAddr.smiAddr.busId);
                            CPSS_APP_PLATFORM_LOG_PRINT_MAC("\tSMI Dev Id: %d\n", profilePtr->boardParam.ppMapPtr->mngInterfaceAddr.smiAddr.deviceId);
                            break;
                        case CPSS_CHANNEL_TWSI_E:
                        case CPSS_CHANNEL_LAST_E:
                        default:
                            break;
                    }
                    break;
                case CPSS_APP_PLATFORM_PP_MAP_TYPE_AUTOSCAN_E:
                case CPSS_APP_PLATFORM_PP_MAP_TYPE_LAST_E:
                default:
                    CPSS_APP_PLATFORM_LOG_PRINT_MAC("\t!! MapType: %d not supported !!\n", profilePtr->boardParam.ppMapPtr->mapType);
                    break;
            }
            break;
        case CPSS_APP_PLATFORM_BOARD_PARAM_LED_E:
            prvCpssAppPlatformPrintBoardLedProfile(profilePtr->boardParam.ledInfoPtr);
            break;
#ifdef CPSS_APP_PLATFORM_PHASE_2
        case CPSS_APP_PLATFORM_BOARD_PARAM_CASCADE_E:
            CPSS_APP_PLATFORM_LOG_PRINT_MAC("Board:Cascade Port\n");
            CPSS_APP_PLATFORM_LOG_PRINT_MAC("\tdevNum:%d\n", profilePtr->boardParam.cascadePtr->devNum);
            CPSS_APP_PLATFORM_LOG_PRINT_MAC("\tnumberOfCscdTrunks:%d\n", profilePtr->boardParam.cascadePtr->numberOfCscdTrunks);
            CPSS_APP_PLATFORM_LOG_PRINT_MAC("\tnumberOfCscdTargetDevs:%d\n", profilePtr->boardParam.cascadePtr->numberOfCscdTargetDevs);
            CPSS_APP_PLATFORM_LOG_PRINT_MAC("\tnumberOfCscdPorts:%d\n", profilePtr->boardParam.cascadePtr->numberOfCscdPorts);

            break;
#endif
        default:
            break;
    }

    return GT_OK;
}


GT_STATUS
cpssAppPlatformPrintProfile
(
   IN  GT_CHAR                       *profileName
)
{
    GT_STATUS                     rc             = GT_OK;
    CPSS_APP_PLATFORM_PROFILE_STC *profileListPtr = NULL;

    if(!profileName)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

    rc = cpssAppPlatformProfileGet(profileName, &profileListPtr);
    if (rc != GT_OK)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

    while(profileListPtr->profileType < CPSS_APP_PLATFORM_PROFILE_TYPE_LAST_E)
    {
        switch(profileListPtr->profileType)
        {
           case CPSS_APP_PLATFORM_PROFILE_TYPE_BOARD_E:
               prvCpssAppPlatformPrintBoardProfile(profileListPtr->profileValue.boardInfoPtr);
              break;
           case CPSS_APP_PLATFORM_PROFILE_TYPE_PP_E:
               prvCpssAppPlatformPrintPpProfile(profileListPtr->profileValue.ppInfoPtr);
              break;
           case CPSS_APP_PLATFORM_PROFILE_TYPE_RUNTIME_E:
               prvCpssAppPlatformPrintRunTimeProfile(profileListPtr->profileValue.runTimeInfoPtr);
              break;
           case CPSS_APP_PLATFORM_PROFILE_TYPE_LAST_E:
              break;
           default:
              break;
        }
        profileListPtr++;
    }

    return rc;
}

/*****************************************************************************
 * END - Profile Print Functions
 *****************************************************************************/


GT_STATUS
cpssAppPlatformProfileGet
(
   IN  GT_CHAR                        *profileName,
   OUT CPSS_APP_PLATFORM_PROFILE_STC **profile
)
{
    GT_U32 i;

    if(!profileName || !profile)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    for (i=0; i < cap_profile_count; i++)
    {
        if (0 == cpssOsStrCmp(cap_profile_name_arr[i], profileName))
        {
            if (cap_profile_list_arr[i])
            {
                *profile = cap_profile_list_arr[i];
                last_used_profile =i;
                return GT_OK;
            } else {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);

            }
        }
    }
    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
}

GT_STATUS
cpssAppPlatformProfileNameGet
(
   IN   CPSS_APP_PLATFORM_PROFILE_STC  *profilePtr,
   OUT  GT_CHAR                       **profileName
)
{
    GT_U32 i;

    if(!profileName || !profilePtr)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

    for (i=0; i<cap_profile_count; i++)
    {
        if (cap_profile_list_arr[i] == profilePtr && cap_profile_name_arr[i])
        {
            *profileName = cap_profile_name_arr[i];
            return GT_OK;
        }
    }

    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
}

GT_STATUS
cpssAppPlatformListProfiles
(
   GT_VOID
)
{
    GT_U32 i;

    CPSS_APP_PLATFORM_LOG_PRINT_MAC("\n-------------------------------------------------");
    CPSS_APP_PLATFORM_LOG_PRINT_MAC("\nCPSS App Platform Main Profiles:");
    CPSS_APP_PLATFORM_LOG_PRINT_MAC("\n-------------------------------------------------");

    for (i=0; i < cap_profile_count; i++)
    {
        CPSS_APP_PLATFORM_LOG_PRINT_MAC("\n[%2d]   %40s : %s", i, cap_profile_name_arr[i],
                                                                  cap_profile_descr_arr[i]);
    }

    CPSS_APP_PLATFORM_LOG_PRINT_MAC("\n=================================================");
    CPSS_APP_PLATFORM_LOG_PRINT_MAC("\n");

    return GT_OK;
}

GT_STATUS
cpssAppPlatformListTestProfiles
(
   GT_VOID
)
{
    GT_U32 i;

    CPSS_APP_PLATFORM_LOG_PRINT_MAC("\n-------------------------------------------------");
    CPSS_APP_PLATFORM_LOG_PRINT_MAC("\nCPSS App Platform Main Profiles - Validation/Testing:");
    CPSS_APP_PLATFORM_LOG_PRINT_MAC("\n-------------------------------------------------");

    for (i=0; i < cap_test_profile_count; i++)
    {
        CPSS_APP_PLATFORM_LOG_PRINT_MAC("\n[%2d]   %40s : %s", i, cap_test_profile_name_arr[i],
                                                                  cap_test_profile_descr_arr[i]);
    }

    CPSS_APP_PLATFORM_LOG_PRINT_MAC("\n=================================================");
    CPSS_APP_PLATFORM_LOG_PRINT_MAC("\n");

    return GT_OK;
}

GT_STATUS
cpssAppPlatformPrintProfileDb
(
   GT_VOID
)
{
    GT_U32 i;

    CPSS_APP_PLATFORM_LOG_PRINT_MAC("\n----------------------------------------------");
    CPSS_APP_PLATFORM_LOG_PRINT_MAC("\nCPSS App Platform Profile DB:");
    CPSS_APP_PLATFORM_LOG_PRINT_MAC("\n----------------------------------------------");

    for (i=0; i < CPSS_APP_PLATFORM_MAX_PP_CNS; i++)
    {
        if (cap_profile_db.ppProfileStr[i])
            CPSS_APP_PLATFORM_LOG_PRINT_MAC("\n[%2d]  [PP: %s ][RT: %s ]", i,
                          cap_profile_db.ppProfileStr[i] ? cap_profile_db.ppProfileStr[i] : "NULL",
                          cap_profile_db.runtimeProfileStr ? cap_profile_db.runtimeProfileStr : "NULL"
                        );
    }

    CPSS_APP_PLATFORM_LOG_PRINT_MAC("\n==============================================");
    CPSS_APP_PLATFORM_LOG_PRINT_MAC("\n");

    return GT_OK;
}


GT_CHAR_PTR cpssAppPlatformLastUsedProfileGet
(
    GT_VOID
)
{
    return cap_profile_name_arr[last_used_profile];
}