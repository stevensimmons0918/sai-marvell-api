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
* @file cpssAppRefAuEventHandle.c
*
* @brief this file contains the implementation of the AU event handling functions
*
* @version   0
********************************************************************************
*/

#include <cpss/common/cpssTypes.h>
#include <cpss/generic/networkIf/cpssGenNetIfMii.h>
#include <cpss/dxCh/dxChxGen/networkIf/cpssDxChNetIfMii.h>
#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgFdbHash.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChErrataMng.h>
#include <cpss/dxCh/dxChxGen/networkIf/cpssDxChNetIf.h>
#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgFdb.h>
#include <cpss/dxCh/dxChxGen/bridgeFdbManager/cpssDxChBrgFdbManager.h>
#include <cpss/dxCh/dxChxGen/multiPortGroup/cpssDxChMultiPortGroup.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>

#include <extUtils/auEventHandler/auFdbUpdateLock.h>
#include <extUtils/auEventHandler/auEventHandler.h>
#include <extUtils/cpssEnabler/appDemoFdbIpv6UcLinkedList.h>

#include <cpssAppPlatformSysConfig.h>
#include <cpssAppPlatformRunTimeConfig.h>
#include <appReference/cpssAppRefUtils.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

#include <gtOs/gtOsMem.h>
#include <gtOs/gtOsTimer.h>

#ifdef ASIC_SIMULATION
static GT_U32 appDemoMaxRetryOnSleep0 = 128;
#endif /*ASIC_SIMULATION*/

#define AU_SIZE 10

/* The number of AU messages per type*/
GT_U32 cpssAppPlatformAuMessageNum[CPSS_HR_E + 1] = {0};
/* The number of wrong type messages*/
GT_U32 wrongAuMessageNum = 0;

#ifdef CHX_FAMILY

#define PRV_APPDEMO_INVALID_FDB_INDEX_VALUE_CNS 0xFFFFFFFF
#define EV_HANDLER_MAX_PRIO 200

/* Roundup Number */
#define ROUNDUP_DIV_MAC(_number , _divider)             \
            (((_number) + ((_divider)-1)) / (_divider))

/*******************************************************************************/
/********************************* AU MESSAGES HANDLE **************************/
/*******************************************************************************/
GT_STATUS prvDxChBrgFdbFromCpuAuMsgStatusGetFunc
(
    IN GT_U8    devNum,
    IN GT_BOOL  usePortGroupsBmp,
    IN GT_U32   currPortGroupsBmp
)
{
    GT_STATUS   st = GT_OK;
    GT_BOOL completed;
    GT_BOOL succeeded;
    GT_PORT_GROUPS_BMP completedBmp;
    GT_PORT_GROUPS_BMP succeededBmp;
    GT_PORT_GROUPS_BMP completedBmpSummary;
    GT_PORT_GROUPS_BMP succeededBmpSummary;
    #ifdef ASIC_SIMULATION
    GT_U32  sleepTime = 0;
    GT_U32  maxRetry = appDemoMaxRetryOnSleep0;
    GT_U32  max_get_tries = 10000;
    #endif/*ASIC_SIMULATION*/

    if(usePortGroupsBmp == GT_FALSE)
    {
        completed = GT_FALSE;
        succeeded = GT_FALSE;

        do{
            #ifdef ASIC_SIMULATION
            if(max_get_tries-- == 0){
                succeeded = GT_FALSE;
                break;
            }
            if ((sleepTime == 0) && maxRetry)
            {
                /* do max retries with 'busy wait' but with option to other tasks
                   to also run --> reduce run time */
                maxRetry--;
            }
            else if (maxRetry == 0)
            {
                sleepTime = 1;
            }
            osTimerWkAfter(sleepTime);
            #endif /*ASIC_SIMULATION*/
            st = cpssDxChBrgFdbFromCpuAuMsgStatusGet(devNum, &completed, &succeeded,NULL);
            if(st != GT_OK)
            {
                goto exit_cleanly_lbl;
            }
        } while (completed == GT_FALSE);

        if(succeeded == GT_FALSE)
        {
            /* operation finished but not succeeded */
            st = GT_FAIL;
            goto exit_cleanly_lbl;
        }
    }
    else
    {
        completedBmpSummary = 0;
        succeededBmpSummary = 0;
        do{
            completedBmp = 0;
            succeededBmp = 0;
            #ifdef ASIC_SIMULATION
            if ((sleepTime == 0) && maxRetry)
            {
                /* do max retries with 'busy wait' but with option to other tasks
                   to also run --> reduce run time */
                maxRetry--;
            }
            else if (maxRetry == 0)
            {
                sleepTime = 1;
            }
            osTimerWkAfter(sleepTime);
            #endif /*ASIC_SIMULATION*/
            st = cpssDxChBrgFdbPortGroupFromCpuAuMsgStatusGet(devNum, currPortGroupsBmp, &completedBmp, &succeededBmp);
            if(st != GT_OK)
            {
                goto exit_cleanly_lbl;
            }

            completedBmpSummary |= completedBmp;
            succeededBmpSummary |= succeededBmp;
        }while (
            (completedBmpSummary & PRV_CPSS_PP_MAC(devNum)->portGroupsInfo.activePortGroupsBmp) !=
            (currPortGroupsBmp & PRV_CPSS_PP_MAC(devNum)->portGroupsInfo.activePortGroupsBmp));

        if(completedBmpSummary != succeededBmpSummary)
        {
            /* operation finished but not succeeded on all port groups */
            st = GT_FAIL;
            goto exit_cleanly_lbl;
        }
    }

exit_cleanly_lbl:

    return st;
}

/* 'mac set' entry for multi port groups device */
GT_STATUS prvDxChBrgFdbPortGroupMacEntrySetFunc
(
    IN GT_U8                        devNum,
    IN GT_U32                       messagePortGroupId,
    IN CPSS_MAC_ENTRY_EXT_STC       *macEntryPtr
)
{
    GT_STATUS   rc;
    GT_PORT_GROUPS_BMP  deletedPortGroupsBmp;  /* bitmap of Port Groups - to delete the entry from */
    GT_PORT_GROUPS_BMP  portGroupsBmp;  /* bitmap of Port Groups */

    (void)messagePortGroupId;

    if(PRV_CPSS_PP_MAC(devNum)->portGroupsInfo.numOfPortGroups <= 1)
    {
        rc = cpssDxChBrgFdbMacEntrySet(devNum,macEntryPtr);
        if(rc != GT_OK)
        {
            return rc;
        }

        rc =  prvDxChBrgFdbFromCpuAuMsgStatusGetFunc(devNum,GT_FALSE,0);

        if(rc == GT_FAIL)
        {
            /* indicate that the bucket is FULL */
            rc = GT_FULL;
            CPSS_APP_PLATFORM_LOG_ERR_MAC("NA Msg: Adding New MAC Entry failed\n");
        }

        return rc;
    }

    if(PRV_CPSS_DXCH_PP_MAC(devNum)->portGroupsExtraInfo.fdbMode ==
        PRV_CPSS_DXCH_MULTI_PORT_GROUP_FDB_MODE_UNIFIED_E)
    {
        portGroupsBmp = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;
    }
    else
    {
        /* do NOT care if entry is :
            1. port or trunk
            2. or relate to local/remote device
           because this is ' controlled learn' that actually emulates 'auto learn'.
           so when message come from one of the port groups we add the entry
           to all port groups that associated with this src port group (unifiedFdbPortGroupsBmp)
        */
#if 0
/* TBD */
        if(messagePortGroupId < PRV_CPSS_PP_MAC(devNum)->portGroupsInfo.numOfPortGroups)
        {
            portGroupsBmp =
                appDemoPpConfigList[devIdx].portGroupsInfo[messagePortGroupId].unifiedFdbPortGroupsBmp;
        }
        else
        {
            /* error -- should not happen */
            portGroupsBmp = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;
        }
#endif
        portGroupsBmp = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;
    }

    rc =  cpssDxChBrgFdbPortGroupMacEntrySet(devNum,portGroupsBmp,macEntryPtr);
    if(rc != GT_OK)
    {
        return rc;
    }

    if(portGroupsBmp == CPSS_PORT_GROUP_UNAWARE_MODE_CNS)
    {
        rc = prvDxChBrgFdbFromCpuAuMsgStatusGetFunc(devNum,GT_FALSE,0);
        if(rc == GT_FAIL)
        {
            /* indicate that the bucket is FULL */
            rc = GT_FULL;
            CPSS_APP_PLATFORM_LOG_ERR_MAC("NA Msg: Adding New MAC Entry failed\n");
        }

        return rc;
    }

    /* remove from the active port groups the port groups that we added the entry
       to them */
    deletedPortGroupsBmp =
        PRV_CPSS_PP_MAC(devNum)->portGroupsInfo.activePortGroupsBmp & (~portGroupsBmp);

    /* we need to delete the mac entry from other port groups that it may be in
       (station movement) */
    rc = cpssDxChBrgFdbPortGroupMacEntryDelete(devNum,deletedPortGroupsBmp,&macEntryPtr->key);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* wait for finish on deleted port groups , but not require success ,
       because mac not necessarily exists there */
    rc =  prvDxChBrgFdbFromCpuAuMsgStatusGetFunc(devNum,GT_TRUE,deletedPortGroupsBmp);
    if(rc != GT_OK)
    {
        /* this is not error !!!! */
        rc = GT_OK;
    }

    /* wait for finish and success on added port groups to see that that added
       entry succeeded */
    rc = prvDxChBrgFdbFromCpuAuMsgStatusGetFunc(devNum,GT_TRUE,portGroupsBmp);

    return rc;
}

GT_STATUS prvDxChBrgFdbMacEntryDeleteFunc
(
    IN GT_U8                        devNum,
    IN CPSS_MAC_ENTRY_EXT_KEY_STC   *macEntryKeyPtr
)
{
    GT_STATUS   rc;

    rc = cpssDxChBrgFdbMacEntryDelete(devNum,macEntryKeyPtr);
    if(rc != GT_OK)
    {
        return rc;
    }

    rc =  prvDxChBrgFdbFromCpuAuMsgStatusGetFunc(devNum,GT_FALSE,0);

    if(rc != GT_OK)
    {
        if(PRV_CPSS_PP_MAC(devNum)->portGroupsInfo.numOfPortGroups == 0 ||
           (PRV_CPSS_DXCH_PP_MAC(devNum)->portGroupsExtraInfo.fdbMode ==
            PRV_CPSS_DXCH_MULTI_PORT_GROUP_FDB_MODE_UNIFIED_E))
        {
            return rc;
        }

        /* wait for finish on deleted port groups , but not require success ,
           because mac not necessarily exists there */

        /* this is not error !!!! */
        rc = GT_OK;
    }

    return rc;
}

GT_STATUS prvDxChBrgFdbMacEntryInvalidateFunc
(
    IN  GT_U8                           devNum,
    IN  CPSS_MAC_ENTRY_EXT_TYPE_ENT     entryType,
    IN  GT_U32                          index
)
{
    GT_STATUS rc=GT_OK;
    GT_BOOL   valid;
    GT_BOOL   skip;
    GT_U32    pairIndex=0;

    /* get index pair incase of ipv6 key or data */
    if((entryType == CPSS_MAC_ENTRY_EXT_TYPE_IPV6_UC_ADDR_ENTRY_E)||
       (entryType == CPSS_MAC_ENTRY_EXT_TYPE_IPV6_UC_DATA_ENTRY_E))
    {
        /* need to get second index to delete */
        rc = prvAppDemoBrgFdbIpv6UcFindPairIndexToDelete(devNum, entryType, index, GT_FALSE, &pairIndex);
        if((rc != GT_OK)&&(rc!=GT_NOT_FOUND))
        {
            return rc;
        }
    }

    rc = cpssDxChBrgFdbMacEntryStatusGet(devNum,index,&valid,&skip);
    if(rc != GT_OK)
        return rc;

    /* If the entry is valid need to invalidate and to decrement the bank counter
       If the entry is invalid - do nothing */
    if(valid)
    {
        rc = cpssDxChBrgFdbMacEntryInvalidate(devNum,index);
        if(rc!=GT_OK)
            return rc;

        /* decrement bank counter */
        rc =  cpssDxChBrgFdbBankCounterUpdate(devNum, index%16, GT_FALSE);
        if(rc!=GT_OK)
            return rc;

    }

    if(pairIndex != PRV_APPDEMO_INVALID_FDB_INDEX_VALUE_CNS)
    {
        rc = cpssDxChBrgFdbMacEntryStatusGet(devNum,pairIndex,&valid,&skip);
        if(rc != GT_OK)
            return rc;

        /* If the entry is valid need to invalidate and to decrement the bank counter
           If the entry is invalid - do nothing */
        if(valid)
        {
            rc = cpssDxChBrgFdbMacEntryInvalidate(devNum,pairIndex);
            if(rc!=GT_OK)
                return rc;

            /* decrement bank counter */
            rc =  cpssDxChBrgFdbBankCounterUpdate(devNum, pairIndex%16, GT_FALSE);
            if(rc!=GT_OK)
                return rc;
        }
    }

    /* if we succeed in deleting some or both entries (ipv6 UC addr and data) from HW
       we need to delete the address/data index from the Link List data base */
    rc = prvAppDemoBrgFdbIpv6UcFindPairIndexToDelete(devNum, entryType, index, GT_TRUE, &pairIndex);
    if(rc != GT_OK)
    {
        return rc;
    }

    return rc;
}
#endif /*CHX_FAMILY*/

/**
* @internal prvCpssAppRefAaMsgHandleFdbUcRouteEntries function
* @endinternal
*
* @brief   This handle AA message for FDB UC Route Entries
*
* @param[in] entryType                - UC Entry type: ipv4/ipv6 Key/ipv6 data
* @param[in] macEntryIndex            - UC Entry index to invalidate
*
* @param[out] errorCounterPtr          - (pointer to) incremented on every error.
*
* @retval GT_OK                    - on success
*/
static GT_STATUS prvCpssAppRefAaMsgHandleFdbUcRouteEntries
(
    IN  CPSS_MAC_ENTRY_EXT_TYPE_ENT entryType,
    IN  GT_U32                      macEntryIndex,
    OUT GT_U32                      *errorCounterPtr
)
{
    GT_STATUS rc = GT_OK;
    GT_U8     dev;

    for (dev = 0; dev < CPSS_APP_PLATFORM_MAX_PP_CNS; dev++)
    {
        if(!PRV_CPSS_IS_DEV_EXISTS_MAC(dev))
        {
            continue;
        }

        AU_FDB_UPDATE_LOCK();
        /* delete data from cpss */

        rc = prvDxChBrgFdbMacEntryInvalidateFunc(dev, entryType, macEntryIndex);

        if (rc != GT_OK)
        {
            /* go to next message to handle */
            (*errorCounterPtr)++;
            AU_FDB_UPDATE_UNLOCK();
            continue;
         }

         AU_FDB_UPDATE_UNLOCK();
    }

    return rc;
}

static GT_BOOL supportAaMessageCap = GT_TRUE;

/**
* @internal appRefSupportAaMessageSet function
* @endinternal
*
* @brief   Configure the appRef to support/not support the AA messages to CPU
*
* @param[in] supportAaMessage         -
*                                      GT_FALSE - not support processing of AA messages
*                                      GT_TRUE - support processing of AA messages
*                                       nothing
*/
GT_VOID appRefSupportAaMessageSet
(
    IN GT_BOOL supportAaMessage
)
{
    supportAaMessageCap = supportAaMessage;
}

/**
* @internal cpssAppPlatformAuMsgHandle function
* @endinternal
*
* @brief   This handles AU message received from HW.
*
* @param[in] auEvHandler - Handler registered for AU events.
* @param[in] devNum             - the device number.
* @param[in] auMessage          - AU message
*
* @param[out] errorCounterPtr   - (pointer to) incremented on every error.
*
* @retval GT_OK                 - on success
*/
static GT_STATUS cpssAppPlatformAuMsgHandle
(
    IN  GT_UINTPTR auEvHandler,
    IN  GT_U8 devNum,
    IN  CPSS_MAC_UPDATE_MSG_EXT_STC bufferArr[],
    IN  GT_U32 numOfAu,
    OUT GT_U32 *errorCounterPtr
)
{
    GT_STATUS                   rc;
    GT_U32                      auIndex;
    CPSS_MAC_ENTRY_EXT_STC      cpssMacEntry;
    GT_U8                       dev;
    CPSS_MAC_ENTRY_EXT_TYPE_ENT entryType;

    for (auIndex = 0; auIndex < numOfAu; auIndex++)
    {
        if(bufferArr[auIndex].updType <= CPSS_FU_E )
        {
            cpssAppPlatformAuMessageNum[bufferArr[auIndex].updType]++;
        }
        else
        {
            wrongAuMessageNum++;
        }

        switch (bufferArr[auIndex].updType)
        {
            case CPSS_AA_E:
                if(supportAaMessageCap == GT_FALSE)
                {
                    break;
                }

                entryType = bufferArr[auIndex].macEntry.key.entryType;
                if (((entryType==CPSS_MAC_ENTRY_EXT_TYPE_IPV4_UC_E)||
                    (entryType==CPSS_MAC_ENTRY_EXT_TYPE_IPV6_UC_ADDR_ENTRY_E)||
                    (entryType==CPSS_MAC_ENTRY_EXT_TYPE_IPV6_UC_DATA_ENTRY_E))
#ifdef CHX_FAMILY
                    && (GT_TRUE == PRV_CPSS_DXCH_ERRATA_GET_MAC(devNum, PRV_CPSS_DXCH_BOBCAT2_FDB_ROUTE_UC_DELETE_BY_MESSAGE_WA_E))
#endif
                    )
                {
                    rc = prvCpssAppRefAaMsgHandleFdbUcRouteEntries(entryType,bufferArr[auIndex].macEntryIndex,errorCounterPtr);
                    if(rc != GT_OK)
                        return rc;

                    break;
                }
                /* fall through ... */
            case CPSS_NA_E:

                osMemCpy(&cpssMacEntry, &bufferArr[auIndex].macEntry,
                             sizeof(CPSS_MAC_ENTRY_EXT_STC));
                cpssMacEntry.saCommand = CPSS_MAC_TABLE_FRWRD_E;
                cpssMacEntry.age = GT_TRUE;
                cpssMacEntry.spUnknown = GT_FALSE;/*even if SP came from device , we need to set entry with 'SP false' */

                for (dev = 0; dev < CPSS_APP_PLATFORM_MAX_PP_CNS; dev++)
                {
                    if(!PRV_CPSS_IS_DEV_EXISTS_MAC(dev))
                    {
                        continue;
                    }

                    AU_FDB_UPDATE_LOCK();
                    if(bufferArr[auIndex].updType == CPSS_NA_E)
                    {
                        /* write data to cpss */
                        rc = prvDxChBrgFdbPortGroupMacEntrySetFunc(dev, bufferArr[auIndex].portGroupId, &cpssMacEntry);
                    }
                    else /*AA*/
                    {
                        /* delete data from cpss */
                        rc = prvDxChBrgFdbMacEntryDeleteFunc(dev, &cpssMacEntry.key);
                    }

                    if (rc != GT_OK)
                    {
                        /* go to next message to handle */
                        (*errorCounterPtr)++;
                        AU_FDB_UPDATE_UNLOCK();
                        continue;
                    }

                    AU_FDB_UPDATE_UNLOCK();
                }
                break;

            case CPSS_QA_E:
            case CPSS_QR_E:
            case CPSS_SA_E:
            case CPSS_QI_E:
            case CPSS_FU_E:
            case CPSS_TA_E:
                break;
            default:
                (*errorCounterPtr)++;
                continue;
        }

        CPSS_APP_PLATFORM_LOG_DBG_MAC("AU Message Type:%d\n", bufferArr[auIndex].updType);

        /* check if we have callback function for this type of message */
        if(auEvHandler)
        {
            auEventHandlerDoCallbacks(auEvHandler,
                    (AU_EV_HANDLER_EV_TYPE_ENT)(bufferArr[auIndex].updType),
                    devNum, &bufferArr[auIndex]);
        }
    }

    return GT_OK;
}

/**
* @internal cpssAppPlatformFdbManagerAuMsgHandle function
* @endinternal
*
* @brief   This routine gets and handles Address Update messages.
*
* @param[in] auEvHandler - Handler registered for AU events.
* @param[in] devNum      - Device number.
* @param[in] evExtData   - Unified event additional information
*
* @retval GT_OK          - on success,
* @retval GT_FAIL        - otherwise.
*/
GT_STATUS cpssAppPlatformFdbManagerAuMsgHandle
(
    GT_U32                                              fdbManagerId,
    CPSS_DXCH_BRG_FDB_MANAGER_LEARNING_SCAN_PARAMS_STC  *scanParamPtr
)
{
    GT_STATUS                   rc = GT_OK;

    while(rc != GT_NO_MORE)
    {
        rc = cpssDxChBrgFdbManagerLearningScan(fdbManagerId, scanParamPtr, NULL, NULL);
    }
    return rc;
}

/**
* @internal cpssAppPlatformGenAuMsgHandle function
* @endinternal
*
* @brief   This routine gets and handles Address Update messages.
*
* @param[in] auEvHandler - Handler registered for AU events.
* @param[in] devNum      - Device number.
* @param[in] evExtData   - Unified event additional information
*
* @retval GT_OK          - on success,
* @retval GT_FAIL        - otherwise.
*/
GT_STATUS cpssAppPlatformGenAuMsgHandle
(
    IN GT_UINTPTR auEvHandler,
    IN GT_U8      devNum,
    IN GT_U32     evExtData
)
{
    GT_STATUS                   rc = GT_OK;
    GT_STATUS                   rc1;
    GT_U32                      numOfAu;
    CPSS_MAC_UPDATE_MSG_EXT_STC bufferArr[AU_SIZE];
    GT_U32                      error = 0;/* number of errors */

    evExtData = evExtData;

    if(!CPSS_IS_DXCH_FAMILY_MAC(PRV_CPSS_PP_MAC(devNum)->devFamily))
    {
        /* the AU messages processing is not implemented yet for the device */
        return GT_OK;
    }

    /* reset the full buffer ... as CPSS not set all fields and it will cause 'memcmp' to fail. */
    cpssOsMemSet(bufferArr,0,sizeof(bufferArr));

    while(rc != GT_NO_MORE)
    {
        /*************************************/
        /* get the AU message buffer from HW */
        /*************************************/
        numOfAu = AU_SIZE;

        AU_FDB_UPDATE_LOCK();
        rc = cpssDxChBrgFdbAuMsgBlockGet(devNum, &numOfAu, bufferArr);
        AU_FDB_UPDATE_UNLOCK();

        if((rc != GT_NO_MORE) && (rc != GT_OK))
        {
            return rc;
        }

        /*********************************/
        /* handle all of the AU messages */
        /*********************************/
        rc1 = cpssAppPlatformAuMsgHandle(auEvHandler,devNum,bufferArr,numOfAu,&error);
        if(rc1 != GT_OK)
        {
            return rc1;
        }

        /* reset the entries that we got */
        cpssOsMemSet(bufferArr,0,numOfAu * sizeof(bufferArr[0]));
    }

    if(error)
    {
        return GT_FAIL;
    }

    return GT_OK;
}

/**
* @internal cpssAppPlatformAuMessageNumberDump function
* @endinternal
*
* @brief   Get number of Au messages (for debgug).
*
* @retval GT_OK                    - on success
*/
GT_STATUS cpssAppPlatformAuMessageNumberDump
(
    GT_VOID
)
{
    CPSS_APP_PLATFORM_LOG_PRINT_MAC("The number NA messages is %d\n", cpssAppPlatformAuMessageNum[CPSS_NA_E]);
    CPSS_APP_PLATFORM_LOG_PRINT_MAC("The number QA messages is %d\n", cpssAppPlatformAuMessageNum[CPSS_QA_E]);
    CPSS_APP_PLATFORM_LOG_PRINT_MAC("The number QR messages is %d\n", cpssAppPlatformAuMessageNum[CPSS_QR_E]);
    CPSS_APP_PLATFORM_LOG_PRINT_MAC("The number AA messages is %d\n", cpssAppPlatformAuMessageNum[CPSS_AA_E]);
    CPSS_APP_PLATFORM_LOG_PRINT_MAC("The number TA messages is %d\n", cpssAppPlatformAuMessageNum[CPSS_TA_E]);
    CPSS_APP_PLATFORM_LOG_PRINT_MAC("The number SA messages is %d\n", cpssAppPlatformAuMessageNum[CPSS_SA_E]);
    CPSS_APP_PLATFORM_LOG_PRINT_MAC("The number QI messages is %d\n", cpssAppPlatformAuMessageNum[CPSS_QI_E]);
    CPSS_APP_PLATFORM_LOG_PRINT_MAC("The number FU messages is %d\n", cpssAppPlatformAuMessageNum[CPSS_FU_E]);
    if(wrongAuMessageNum != 0)
    {
        CPSS_APP_PLATFORM_LOG_ERR_MAC("The number AU wrong messages is %d\n", wrongAuMessageNum);
    }

    return GT_OK;
}

static GT_BOOL allowProcessingOfAuqMsg = GT_TRUE;
extern GT_UINTPTR auEventHandle;

/**
* @internal appRefAllowProcessingOfAuqMessages function
* @endinternal
*
* @brief   function to allow set the flag of : allowProcessingOfAuqMsg
*
* @param[in] enable                   - enable/disable the processing of the AUQ messages
*
* @retval GT_OK                    - on success
*
* @note flag that state the tasks may process events that relate to AUQ messages.
*       this flag allow us to stop processing those messages , by that the AUQ may
*       be full, or check AU storm prevention, and other.
*
*/
GT_STATUS  appRefAllowProcessingOfAuqMessages
(
    GT_BOOL     enable
)
{
    if(enable == GT_TRUE && allowProcessingOfAuqMsg== GT_FALSE)
    {
        /* move from no learn to learn */
        /* start processing the messages stuck in the queue */
        cpssAppPlatformGenAuMsgHandle(auEventHandle,0,0);
    }

    allowProcessingOfAuqMsg = enable;

    return GT_OK;
}

static GT_U32                                  appDemoFdbManagerId;
static APP_DEMO_AU_MESSAGE_HANDLER_METHORD_ENT auMsgHandlerMethord = APP_DEMO_AU_MESSAGE_HANDLER_LOW_LEVEL_E;
static unsigned __TASKCONV appDemoFdbManagerAutoAging(GT_VOID * param);
static GT_BOOL                                 appDemoAutoAgingEn  = GT_FALSE;
static GT_U32                                  appDemoAutoAgingInterval;          /* Debug purpose */
static GT_U32                                  appDemoAutoAgingIterationInterval; /* Dynamically calculated */
static GT_U32                                  fdbManagerAutoAgingTaskCreated = 0;
static GT_U32                                  appDemoAutoAgingStatisticsArr[APP_DEMO_PRV_CPSS_DXCH_FDB_MANAGER_AUTO_AGING_API_STATISTICS___LAST___E];

GT_STATUS appRefFdbManagerGlobalIDSet
(
    IN GT_U32 fdbManagerId
)
{
    if(fdbManagerId>=32)
    {
        return GT_BAD_PARAM;
    }
    appDemoFdbManagerId = fdbManagerId;
    return GT_OK;
}

/*******************************************************************************
* appDemoFdbManagerAutoAging
*
* DESCRIPTION:
*       This routine handles the auto aging process.
*       calls the CPSS auto aging API in regular interval.(to meet the configured aging time)
*
* INPUTS:
*       param - The process data structure.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK   - on success,
*       GT_FAIL - otherwise.
*
* COMMENTS:
*       None.
*
*******************************************************************************/
static unsigned __TASKCONV appDemoFdbManagerAutoAging
(
    GT_VOID * param
)
{
    param = param;
    GT_STATUS                                                           rc;
    CPSS_DXCH_BRG_FDB_MANAGER_AGING_SCAN_PARAMS_STC                     ageScanParam;
    APP_DEMO_PRV_CPSS_DXCH_FDB_MANAGER_AUTO_AGING_API_STATISTICS_ENT    statisticsType = APP_DEMO_PRV_CPSS_DXCH_FDB_MANAGER_AUTO_AGING_API_STATISTICS_ERROR_FAIL_E;

    (void)param;

    /* indicate that task start running */
    fdbManagerAutoAgingTaskCreated = 1;
    osTimerWkAfter(1);

    osMemSet(&ageScanParam, 0, sizeof(ageScanParam));
    ageScanParam.checkAgeMacUcEntries           = GT_TRUE;
    ageScanParam.checkAgeMacMcEntries           = GT_TRUE;
    ageScanParam.checkAgeIpMcEntries            = GT_TRUE;
    ageScanParam.checkAgeIpUcEntries            = GT_TRUE;
    ageScanParam.deleteAgeoutMacUcEportEntries  = GT_TRUE;
    ageScanParam.deleteAgeoutMacUcTrunkEntries  = GT_TRUE;
    ageScanParam.deleteAgeoutMacMcEntries       = GT_TRUE;
    ageScanParam.deleteAgeoutIpUcEntries        = GT_TRUE;
    ageScanParam.deleteAgeoutIpMcEntries        = GT_TRUE;

    while (1)
    {
        /* appDemoAutoAgingIterationInterval - can change dynamically by appRefFdbManagerControlSet
         * appDemoFdbManagerId               - can change dynamically by appDemoFdbManagerGlobalIDSet
         */
        if(appDemoAutoAgingEn)
        {
            osTimerWkAfter(appDemoAutoAgingIterationInterval);
            rc = cpssDxChBrgFdbManagerAgingScan(appDemoFdbManagerId, &ageScanParam, NULL, NULL);
            switch(rc)
            {
                case GT_OK:
                    statisticsType = APP_DEMO_PRV_CPSS_DXCH_FDB_MANAGER_AUTO_AGING_API_STATISTICS_OK_E;
                    break;
                case GT_FAIL:
                    statisticsType = APP_DEMO_PRV_CPSS_DXCH_FDB_MANAGER_AUTO_AGING_API_STATISTICS_ERROR_FAIL_E;
                    break;
                case GT_HW_ERROR:
                    statisticsType = APP_DEMO_PRV_CPSS_DXCH_FDB_MANAGER_AUTO_AGING_API_STATISTICS_ERROR_HW_ERROR_E;
                    break;
                case GT_BAD_PARAM:
                case GT_BAD_PTR:
                case GT_NOT_APPLICABLE_DEVICE:
                    statisticsType = APP_DEMO_PRV_CPSS_DXCH_FDB_MANAGER_AUTO_AGING_API_STATISTICS_ERROR_INPUT_INVALID_E;
                    break;
                default:
                    break;
            }
            appDemoAutoAgingStatisticsArr[statisticsType]+=1;
        }
        else
        {
            /* Time to avoid cpu consumption by aging task */
            osTimerWkAfter(1000);
        }
    }
    return 0;
}

GT_STATUS appRefFdbManagerAutoAgingStatisticsClear()
{
    osMemSet(appDemoAutoAgingStatisticsArr, 0, sizeof(appDemoAutoAgingStatisticsArr));
    return GT_OK;
}

GT_STATUS appRefFdbManagerAutoAgingStatisticsGet
(
    OUT APP_DEMO_CPSS_DXCH_BRG_FDB_MANAGER_STATISTICS_STC *statistics
)
{
    APP_DEMO_PRV_CPSS_DXCH_FDB_MANAGER_AUTO_AGING_API_STATISTICS_ENT    statisticType;
    GT_U32                                                              *statisticValuePtr;

    if(statistics == NULL)
    {
        return GT_BAD_PARAM;
    }

    osMemSet(statistics, 0, sizeof(APP_DEMO_CPSS_DXCH_BRG_FDB_MANAGER_STATISTICS_STC));

    /* Auto Aging appDemo task statistics */
    statisticType = APP_DEMO_PRV_CPSS_DXCH_FDB_MANAGER_AUTO_AGING_API_STATISTICS_OK_E;
    while(statisticType < APP_DEMO_PRV_CPSS_DXCH_FDB_MANAGER_AUTO_AGING_API_STATISTICS___LAST___E)
    {
        statisticValuePtr = NULL;
        switch(statisticType)
        {
            case APP_DEMO_PRV_CPSS_DXCH_FDB_MANAGER_AUTO_AGING_API_STATISTICS_OK_E:
                statisticValuePtr = &statistics->autoAgingOk;
                break;
            case APP_DEMO_PRV_CPSS_DXCH_FDB_MANAGER_AUTO_AGING_API_STATISTICS_ERROR_INPUT_INVALID_E:
                statisticValuePtr = &statistics->autoAgingErrorInputInvalid;
                break;
            case APP_DEMO_PRV_CPSS_DXCH_FDB_MANAGER_AUTO_AGING_API_STATISTICS_ERROR_FAIL_E:
                statisticValuePtr = &statistics->autoAgingErrorFail;
                break;
            case APP_DEMO_PRV_CPSS_DXCH_FDB_MANAGER_AUTO_AGING_API_STATISTICS_ERROR_HW_ERROR_E:
                statisticValuePtr = &statistics->autoAgingErrorHwError;
                break;
            default:
                break;
        }
        if(statisticValuePtr)
        {
            *statisticValuePtr = appDemoAutoAgingStatisticsArr[statisticType];
        }
        statisticType++;
    }
    return GT_OK;
}

GT_STATUS appDemoFdbManagerAuMsgHandlerMethod
(
    IN APP_DEMO_AU_MESSAGE_HANDLER_METHORD_ENT auMsgHandler
)
{
    GT_U32   devNum;

    switch(auMsgHandler)
    {
        case APP_DEMO_AU_MESSAGE_HANDLER_LOW_LEVEL_E:
            break;
        case APP_DEMO_AU_MESSAGE_HANDLER_FDB_MANAGER_E:
        case APP_DEMO_AU_MESSAGE_HANDLER_FDB_MANAGER_WITH_CUCKOO_E:
            /* FDB Manager support AU queue only.
               Disable AU FIFO related interrupt to avoid redundant actions. */
            for(devNum = 0; devNum < CPSS_APP_PLATFORM_MAX_PP_CNS; devNum++)
            {
                if(PRV_CPSS_IS_DEV_EXISTS_MAC(devNum))
                {
                    (GT_VOID)cpssEventDeviceMaskSet(devNum,
                                                CPSS_PP_MAC_MESSAGE_TO_CPU_READY_E,
                                                CPSS_EVENT_MASK_E);
                }
            }
            break;
        case APP_DEMO_AU_MESSAGE_HANDLER_DISABLED_E:
            break;
        default:
            CPSS_APP_PLATFORM_LOG_ERR_MAC("prvAppDemoFdbManagerAuMsgHandlerMethod: - Invalid methord");
    }

    auMsgHandlerMethord = auMsgHandler;

    return GT_OK;
}

static GT_STATUS prvAppDemoFdbManagerAutoAgingEnable
(
    IN GT_U32       autoAgingInterval
)
{
    GT_TASK                                              autoAgeTid;         /* Task Id */
    GT_STATUS                                            rc;
    CPSS_DXCH_BRG_FDB_MANAGER_CAPACITY_STC               capacityInfo;
    CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_ATTRIBUTES_STC       entryAttrInfo;
    CPSS_DXCH_BRG_FDB_MANAGER_LEARNING_STC               learningInfo;
    CPSS_DXCH_BRG_FDB_MANAGER_LOOKUP_STC                 lookupInfo;
    CPSS_DXCH_BRG_FDB_MANAGER_AGING_STC                  agingInfo;
    GT_U32                                               totalAgeBinAllocated;

    /* Validity check for auto aging interval */
    if(autoAgingInterval == 0)
    {
        CPSS_APP_PLATFORM_LOG_ERR_MAC("appRefFdbManagerControlSet: autoAgingInterval invalid");
        return GT_BAD_PARAM;
    }

    /* Calculate interval between CPSS age call API */
    rc = cpssDxChBrgFdbManagerConfigGet(appDemoFdbManagerId, &capacityInfo, &entryAttrInfo, &learningInfo, &lookupInfo, &agingInfo);
    if(rc != GT_OK)
    {
        CPSS_APP_PLATFORM_LOG_ERR_MAC("appDemoFdbManagerAutoAging: cpssDxChBrgFdbManagerConfigGet failed");
        return rc;
    }

    /* appDemoAutoAgingIterationInterval in milisec
     * autoAgingInterval                 is in sec
     **/
    totalAgeBinAllocated                = ROUNDUP_DIV_MAC(capacityInfo.maxTotalEntries, capacityInfo.maxEntriesPerAgingScan);
    appDemoAutoAgingIterationInterval   = ROUNDUP_DIV_MAC(autoAgingInterval * 1000, totalAgeBinAllocated);
    appDemoAutoAgingInterval            = autoAgingInterval; /* kept in global data, Just for debug purpose */

    if(fdbManagerAutoAgingTaskCreated == 1)
    {
        return GT_OK;
    }

    rc = osTaskCreate("FdbManagerAutoAging",
            EV_HANDLER_MAX_PRIO + 1,
            _64KB,
            appDemoFdbManagerAutoAging,
            NULL,
            &autoAgeTid);
    if (rc != GT_OK)
    {
        return GT_FAIL;
    }

    while(fdbManagerAutoAgingTaskCreated == 0)
    {
        /* wait for indication that task created */
        osTimerWkAfter(1);
    }
    return GT_OK;
}

GT_STATUS appDemoFdbManagerAuMsgControlSet
(
    IN APP_DEMO_AU_MESSAGE_HANDLER_METHORD_ENT auMsgHandler,
    IN GT_BOOL                                 autoAgingEn,
    IN GT_U32                                  autoAgingInterval
)
{
    GT_STATUS   rc;

    switch(auMsgHandler)
    {
        case APP_DEMO_AU_MESSAGE_HANDLER_LOW_LEVEL_E:
            appDemoAutoAgingEn = GT_FALSE;
            break;
        case APP_DEMO_AU_MESSAGE_HANDLER_FDB_MANAGER_E:
        case APP_DEMO_AU_MESSAGE_HANDLER_FDB_MANAGER_WITH_CUCKOO_E:
            if(autoAgingEn == GT_TRUE)
            {
                rc = prvAppDemoFdbManagerAutoAgingEnable(autoAgingInterval);
                if(rc != GT_OK)
                {
                    return rc;
                }
            }
            appDemoAutoAgingEn = autoAgingEn;
            break;
        case APP_DEMO_AU_MESSAGE_HANDLER_DISABLED_E:
            appDemoAutoAgingEn = GT_FALSE;
            break;
        default:
            CPSS_APP_PLATFORM_LOG_ERR_MAC("prvAppDemoFdbManagerAuMsgHandlerMethod: - Invalid methord");
            return GT_BAD_PARAM;
    }
    return appDemoFdbManagerAuMsgHandlerMethod(auMsgHandler);
}

GT_STATUS appRefFdbManagerControlSet
(
    IN APP_DEMO_AU_MESSAGE_HANDLER_METHORD_ENT  auMsgHandler,
    IN GT_BOOL                                  autoAgingEn,
    IN GT_U32                                   autoAgingInterval
)
{
    GT_STATUS   rc;

    switch(auMsgHandler)
    {
        case APP_DEMO_AU_MESSAGE_HANDLER_LOW_LEVEL_E:
            appDemoAutoAgingEn = GT_FALSE;
            break;
        case APP_DEMO_AU_MESSAGE_HANDLER_FDB_MANAGER_E:
        case APP_DEMO_AU_MESSAGE_HANDLER_FDB_MANAGER_WITH_CUCKOO_E:
            if(autoAgingEn == GT_TRUE)
            {
                rc = prvAppDemoFdbManagerAutoAgingEnable(autoAgingInterval);
                if(rc != GT_OK)
                {
                    return rc;
                }
            }
            appDemoAutoAgingEn = autoAgingEn;
            break;
        case APP_DEMO_AU_MESSAGE_HANDLER_DISABLED_E:
            appDemoAutoAgingEn = GT_FALSE;
            break;
        default:
            CPSS_APP_PLATFORM_LOG_ERR_MAC("prvAppDemoFdbManagerAuMsgHandlerMethod: - Invalid methord");
            return GT_BAD_PARAM;
    }

    return appDemoFdbManagerAuMsgHandlerMethod(auMsgHandler);
}

GT_STATUS cpssAppRefAuEventHandle
(
    GT_U8  devNum,
    GT_U32 uniEv,
    GT_U32 evExtData
)
{
    GT_STATUS rc = GT_OK;
    CPSS_DXCH_BRG_FDB_MANAGER_LEARNING_SCAN_PARAMS_STC data;

    if (notifyEventArrivedFunc != NULL)
    {
        notifyEventArrivedFunc(devNum, uniEv, evExtData);
    }

    switch (uniEv)
    {
        case CPSS_PP_MAC_MESSAGE_TO_CPU_READY_E:
        case CPSS_PP_EB_AUQ_PENDING_E:

            if(allowProcessingOfAuqMsg == GT_TRUE)
            {
                switch(auMsgHandlerMethord)
                {
                    case APP_DEMO_AU_MESSAGE_HANDLER_LOW_LEVEL_E:
                        rc = cpssAppPlatformGenAuMsgHandle(auEventHandle, devNum, evExtData);
                        if (rc != GT_OK)
                        {
                            CPSS_APP_PLATFORM_LOG_ERR_MAC("cpssEnAuMsgGet: error, devNum=%d, uniEv=%d, rc=%d\n", devNum, uniEv, rc);
                        }
                        break;
                    case APP_DEMO_AU_MESSAGE_HANDLER_FDB_MANAGER_E:
                        data.addNewMacUcEntries         = GT_TRUE;
                        data.updateMovedMacUcEntries    = GT_TRUE;
                        data.addWithRehashEnable        = GT_FALSE;
                        rc = cpssAppPlatformFdbManagerAuMsgHandle(appDemoFdbManagerId, &data);
                        if (rc != GT_OK)
                        {
                            CPSS_APP_PLATFORM_LOG_ERR_MAC("cpssAppPlatformFdbManagerAuMsgHandle: error, devNum=%d, uniEv=%d, rc=%d\n", devNum, uniEv, rc);
                        }
                        break;
                    case APP_DEMO_AU_MESSAGE_HANDLER_FDB_MANAGER_WITH_CUCKOO_E: /* TODO - Rehash is Yet to impliment */
                        data.addNewMacUcEntries         = GT_TRUE;
                        data.updateMovedMacUcEntries    = GT_TRUE;
                        data.addWithRehashEnable        = GT_TRUE;
                        rc = cpssAppPlatformFdbManagerAuMsgHandle(appDemoFdbManagerId, &data);
                        if (rc != GT_OK)
                        {
                            CPSS_APP_PLATFORM_LOG_ERR_MAC("cpssAppPlatformFdbManagerAuMsgHandle: error, devNum=%d, uniEv=%d, rc=%d\n", devNum, uniEv, rc);
                        }
                        break;
                    case APP_DEMO_AU_MESSAGE_HANDLER_DISABLED_E:
                        break;
                    default:
                        CPSS_APP_PLATFORM_LOG_ERR_MAC("cpssEnAuMsgGet: error AU Message Handler");
                }
            }

            break;
        case CPSS_PP_EB_FUQ_PENDING_E:
            /* FUQ messages are not processed here */
            /* FUQ messages are kept in the FUQ until explicitly called */
            break;
        default:
            /* do nothing */
            break;
    }

    return rc;
}
