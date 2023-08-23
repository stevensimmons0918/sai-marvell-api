// xpsFdb.c

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
********************************************************************************/

#include "xpsFdb.h"
#include "xpsFdbMgr.h"
#include "xpsInit.h"
#include "xpsInternal.h"
#include "xpsInterface.h"
#include "xpsState.h"
#include "xpsVlan.h"
#include "xpsPort.h"
#include "xpsScope.h"
#include "xpsLock.h"
#include "xpsGlobalSwitchControl.h"
#include "cpssHalFdb.h"
#include "cpssHalUtil.h"
#include "cpssHalDevice.h"
#include "cpssDxChBrgSecurityBreach.h"
#include "cpssDxChBrgGen.h"
#include "cpssDriverPpHw.h"
#include "cpssDxChCfgInit.h"


#ifdef XP_HEADERS
#include "xpCtrlMacMgr.h"
#include "xpL2LearnMgr.h"
#include "xpLogModXps.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>
static xpFdbLearnHandler  learnHandler = NULL;
static xpFdbLearnHandler  prevLearnHandler = NULL;
static xpFdbAgingHandler  ageHandler = NULL;

#define HAL_MAC_HASH_CHAIN_LEN       4
#define HAL_MAX_DEVICES              2

#define AGING_TASK_PRIO              100
#define LEARNING_TASK_PRIO           10

#define AU_SIZE                      10

#define DEFAULT_MAC_AGE_CNS          300
#define DEFAULT_MAC_AGE_FOR_SP_CNS   5
#define DEFAILT_MAC_AGING_TYPE_CNS   MAC_ADDR_AGING_SW
#define MAC_AGE_MSG_NOTIFICATION_INTERVAL_CNS   1
#define CPSS_MSG_RATE_LIMIT          2400

static GT_U32 agingTid;  /* The Aging task id  */
static GT_U32 *agingTidPtr = NULL;
static GT_U32 learningTid;  /* The Learning task id  */
static GT_U32 *learningTidPtr = NULL;

GT_U32 max_au_per_second;

uint32_t         default_mac_age = DEFAULT_MAC_AGE_CNS;
uint32_t         default_mac_age_sp = DEFAULT_MAC_AGE_FOR_SP_CNS;
typedef struct
{
    GT_UINTPTR          evHndl;
    GT_U32              hndlrIndex;
} EV_HNDLR_PARAM;
typedef enum
{
    MAC_ADDR_AGING_SW,
    MAC_ADDR_AGING_HW,
} mac_aging_type;

static bool fdbHalLearningAuMsgHandle(uint8_t asic_num);
static GT_STATUS fdbLearningEventHandler
(
    GT_U8                   devNum,
    CPSS_UNI_EV_CAUSE_ENT   uniEv,
    GT_U32                  evExtData
);

mac_aging_type default_mac_age_type = DEFAILT_MAC_AGING_TYPE_CNS;


#define FDB_TABLE_SIZE(devId)  cpssHalGetSKUmaxHashTable(devId)

/* number of words in the mac entry */
#define FDB_MAC_ENTRY_WORDS_SIZE_CNS   4

#define HAL_FDB_ENTRY_ADDRESS(idx) \
        (0x0B400000 + (16 * (idx)))

/* ===========  FUNCTION POINTER DECLARATIONS =========== */
XP_STATUS(*xpsFdbAddDevice)(xpsDevice_t devId, xpsInitType_t initType);
XP_STATUS(*xpsFdbRemoveDevice)(xpsDevice_t devId);
XP_STATUS(*xpsFdbRegisterLearnHandler)(xpsDevice_t devId,
                                       xpFdbLearnHandler fdbLearnHandler);
XP_STATUS(*xpsFdbUnregisterLearnHandler)(xpsDevice_t devId);
XP_STATUS(*xpsFdbRegisterAgingHandler)(xpsDevice_t devId,
                                       xpFdbAgingHandler ageHandler);
XP_STATUS(*xpsFdbFlushEntry)(xpsDevice_t devId, xpsFdbEntryType_e entryType);
XP_STATUS(*xpsFdbFlushEntryByIntf)(xpsDevice_t devId, xpsInterfaceId_t intfId,
                                   xpsFdbEntryType_e entryType);
XP_STATUS(*xpsFdbFlushEntryByVlan)(xpsDevice_t devId, xpsVlan_t vlanId,
                                   xpsFdbEntryType_e entryType);
XP_STATUS(*xpsFdbFlushEntryByIntfVlan)(xpsDevice_t devId,
                                       xpsInterfaceId_t intfId, xpsVlan_t vlanId, xpsFdbEntryType_e entryType);
XP_STATUS(*xpsFdbFindEntry)(xpsDevice_t devId, xpsFdbEntry_t *fdbEntry,
                            uint32_t *index);
XP_STATUS(*xpsFdbAddEntry)(xpsDevice_t devId, xpsFdbEntry_t *fdbEntry,
                           xpsHashIndexList_t *indexList);
XP_STATUS(*xpsFdbAddHwEntry)(xpsDevice_t devId, xpsFdbEntry_t *fdbEntry,
                             xpsHashIndexList_t *indexList);
XP_STATUS(*xpsFdbWriteEntry)(xpsDevice_t devId,  uint32_t index,
                             xpsFdbEntry_t *fdbEntry);
XP_STATUS(*xpsFdbRemoveEntry)(xpsDevice_t devId, xpsFdbEntry_t *fdbEntry);
XP_STATUS(*xpsFdbGetEntryByIndex)(xpsDevice_t devId, uint32_t index,
                                  xpsFdbEntry_t *fdbEntry);
XP_STATUS(*xpsFdbGetEntry)(xpsDevice_t devId, xpsFdbEntry_t *fdbEntry);
XP_STATUS(*xpsFdbSetAttribute)(xpsDevice_t devId, xpsFdbEntry_t *fdbEntry,
                               xpsFdbAttribute_e field, void *data);
XP_STATUS(*xpsFdbGetAttribute)(xpsDevice_t devId, xpsFdbEntry_t *fdbEntry,
                               xpsFdbAttribute_e field, void *data);
XP_STATUS(*xpsFdbGetEntriesCount)(xpsDevice_t devId, uint32_t *usedCount);

typedef struct
{
    uint32_t index;
    uint32_t macAge;
    bool valid;
} macData;

macData *macAddressData = NULL;

bool flag1 = false;
uint32_t temp = 13137;
uint32_t gAgingTimer =
    0;   /*default aging timer is zero, which means age disable*/

/*Holds CPSS LPM_FDB_EM profile index.*/
bool gFlushInProgess = 0;

void SetFdbFlushInProgress(bool val)
{
    gFlushInProgess = val;
}

bool GetFdbFlushInProgress()
{
    return gFlushInProgess;
}



/**
* @internal fdbLearningEventHandler function
* @endinternal
*
* @brief   This routine handles PP events.
*
* @param[in] devNum                   - the device number.
* @param[in] uniEv                    - Unified event number
* @param[in] evExtData                - Unified event additional information
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
static GT_STATUS fdbLearningEventHandler
(
    GT_U8                   devNum,
    CPSS_UNI_EV_CAUSE_ENT   uniEv,
    GT_U32                  evExtData
)
{
    GT_STATUS   rc = GT_OK;
    bool handleTrue = GT_FALSE;

    if (GetFdbFlushInProgress())
    {
        return XP_NO_ERR;
    }

    switch (uniEv)
    {
        case CPSS_PP_MAC_MESSAGE_TO_CPU_READY_E:
        case CPSS_PP_EB_AUQ_PENDING_E:
            handleTrue = fdbHalLearningAuMsgHandle(devNum);
            if (handleTrue != GT_TRUE)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Msg handle failed");
            }
            break;
        default:
            break;
    }

    return rc;
}

/* Aging Task */
int fdb_debug = 0;
static unsigned __TASKCONV AgingTaskFunc(GT_VOID *param)
{
    uint32_t mili2Sleep ;
    uint32_t validEntries = 0, thresholdCount = 150, powerNap = 20;
    GT_STATUS rc;
    xpDevice_t devId =0;
    GT_U8 cpssDevNum;
    xpsFdbEntry_t fdbEntry;
    CPSS_MAC_ENTRY_EXT_STC  entry;
    GT_HW_DEV_NUM           associatedHwDevNum = 0;
    GT_BOOL                 valid;
    GT_BOOL                 skip = GT_FALSE;
    GT_BOOL                 aged[2] = {GT_FALSE, GT_FALSE};
    uint32_t SecInNano = (1000*1000*1000);
    uint32_t MilliSecInNano = (1000*1000);
    uint32_t unitTime = 10; /*in seconds*/
    uint32_t startSeconds, stopSeconds,
             startNanoSeconds, stopNanoSeconds,
             elapsedSeconds=0, elapsedNanoSeconds; /* time of init */
    memset(&entry, 0, sizeof(entry));

    while (1)
    {
        cpssOsTimeRT(&startSeconds, &startNanoSeconds);
        /*scan whole FDB for valid entries and validate aging of entries*/
        for (GT_U32 entryIndex=0; entryIndex<FDB_TABLE_SIZE(devId); entryIndex++)
        {
            /*to run an empty loop for all 16k  ~ 60 us on Intel
             * 16k took ~ 200 ms
             * 256 entries takes ~ 3.2 ms
             * Yeilding 10 ms for every 256 entries, takes ( (16k/256) * (10 + 3)) = 832 ms to loop once all 16k entries*/
            /*ARM 385
             * ~300 us to scan entire table when empty
             * When full ~ 2 sec to scan entire table*/

            /* We have to run through all the entries exactly once in ONE UNIT TIME*/
            if (validEntries > thresholdCount)
            {
                validEntries = 0;
                cpssOsTimerWkAfter(powerNap);
            }
            /*do the below only for valid entries*/
            if (macAddressData[entryIndex].valid)
            {
                validEntries++;
                XPS_DEVICES_PER_SWITCH_ITER(devId, cpssDevNum)
                {
                    rc = cpssDxChBrgFdbMacEntryRead(cpssDevNum, entryIndex, &valid, &skip,
                                                    &aged[cpssDevNum], &associatedHwDevNum,  &entry);
                    if (rc != GT_OK)
                    {
                        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                              "cpssDxChBrgFdbMacEntryRead: ret[%d] \n", rc);
                    }
                }

                if (!valid)
                {
                    continue;
                }

                /*entry refreshed case*/
                if ((aged[associatedHwDevNum]))
                {
                    /*entry is refreshed, reset mac age to zero in DB*/
                    macAddressData[entryIndex].macAge = 0;


                    XPS_DEVICES_PER_SWITCH_ITER(devId, cpssDevNum)
                    {
                        entry.age = GT_FALSE;
                        rc = cpssDxChBrgFdbMacEntryWrite(cpssDevNum, entryIndex, skip, &entry);
                        if (rc != GT_OK)
                        {
                            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                                  "cpssDxChBrgFdbMacEntryWrite: ret[%d]\n", rc);
                        }
                    }
                }
                else  /*entry aged case*/
                {
                    if (gAgingTimer != 0)
                    {
                        /*check if entry is aged and needs to be deleted*/
                        if ((++macAddressData[entryIndex].macAge > (gAgingTimer/unitTime)))
                        {
                            if (fdb_debug)
                            {
                                cpssOsPrintf("aging out entry %d with age %d > %d",
                                             entryIndex, macAddressData[entryIndex].macAge, gAgingTimer/unitTime);
                            }
                            /*entry needs to be deleted from both the devices*/
                            cpssHalBrgFdbMacEntryInvalidate(devId, entryIndex);
                            macAddressData[entryIndex].macAge = 0;
                            macAddressData[entryIndex].valid = false;

                            cpssOsMemCpy((void*)fdbEntry.macAddr,
                                         (void*)entry.key.key.macVlan.macAddr.arEther, sizeof(macAddr_t));
                            fdbEntry.vlanId = entry.key.key.macVlan.vlanId ;
                            fdbEntry.isStatic = entry.isStatic;
                            fdbEntry.intfId = XPS_INTF_INVALID_ID;
                            if (ageHandler)
                            {
                                ageHandler(devId, fdbEntry);
                            }

                        }
                    }
                } /*else entry aged*/
            } /*end if mac valid*/
        } /*end for FDB_TABLE_SIZE*/

        cpssOsTimeRT(&stopSeconds, &stopNanoSeconds);
        if (stopNanoSeconds >= startNanoSeconds)
        {
            elapsedNanoSeconds = (stopNanoSeconds - startNanoSeconds);
            elapsedSeconds = (stopSeconds - startSeconds);
        }
        else // Handle wraparound
        {
            elapsedNanoSeconds = (SecInNano + stopNanoSeconds) - startNanoSeconds;
            elapsedSeconds = (stopSeconds - startSeconds) - 1;
        }

        /*yeild for remaining time in the unit time*/
        mili2Sleep = (((unitTime - elapsedSeconds)*1000)-((GT_U32)(
                                                              elapsedNanoSeconds / MilliSecInNano)));
        cpssOsTimerWkAfter(mili2Sleep);
        if (fdb_debug)
        {
            cpssOsPrintf("time elapsed %d.%d\n", elapsedSeconds, elapsedNanoSeconds);
            if (unitTime < elapsedSeconds)
            {
                cpssOsPrintf("error: start %d.%d stop %d.%d\n", startSeconds, startNanoSeconds,
                             stopSeconds, stopNanoSeconds);
                cpssOsPrintf("ageout %d mili2Sleep %d\n", gAgingTimer/unitTime, mili2Sleep);
            }
            //cpssOsTimerWkAfter(1000);
        }
    } // while (1)

    return (GT_TRUE);
}

static bool
fdbHalLearningAuMsgHandle(uint8_t asic_num)
{

    GT_STATUS                   rc = GT_OK;
    //GT_U32 ret;
    GT_U32                      numOfAu;
    GT_U32                      auIndex;

    CPSS_MAC_UPDATE_MSG_EXT_STC bufferArr[AU_SIZE];
    CPSS_MAC_ENTRY_EXT_STC      cpssMacEntry;
    CPSS_MAC_ENTRY_EXT_TYPE_ENT entryType;
    XPS_FDB_ENTRY_TYPE_ENT fdbEntryType;
    xpsFdbEntry_t fdbEntry, hwFdbEntry;
    xpsDevice_t devId = 0;
    GT_STATUS cpss_status = GT_OK;
    XP_STATUS xpStatus = XP_NO_ERR;
    uint32_t index, bankIndex=0;
    GT_U32 numOfAuHandled = 0;
    xpFdbEntryAction_e fdbAction = XP_FDB_ENTRY_NO_ACTION;
    while (rc != GT_NO_MORE)
    {
        /* prevent from learning task to starve other tasks */
        if (numOfAuHandled >= max_au_per_second)
        {
            return (GT_TRUE);
        }

        /* reset the full buffer  */
        cpssOsMemSet(bufferArr, 0, sizeof(bufferArr));

        numOfAu = AU_SIZE;

        /*************************************/
        /* get the AU message buffer from HW */
        /*************************************/
        rc = cpssDxChBrgFdbAuMsgBlockGet(asic_num, &numOfAu, bufferArr);
        if ((rc != GT_NO_MORE) && (rc != GT_OK))
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "cpssDxChBrgFdbAuMsgBlockGet: ret[%d] asic[%d]\n",
                  rc, asic_num);
            return (GT_FALSE);
        }

        numOfAuHandled += numOfAu;

        /*********************************/
        /* handle all of the AU messages */
        /*********************************/
        for (auIndex = 0; auIndex < numOfAu; auIndex++)
        {
            switch (bufferArr[auIndex].updType)
            {
                case CPSS_NA_E:
                    {
                        cpssMacEntry = bufferArr[auIndex].macEntry;

                        entryType = cpssMacEntry.key.entryType;
                        if (entryType != CPSS_MAC_ENTRY_EXT_TYPE_MAC_ADDR_E)
                        {
                            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "wrong mac type recived %d\n",
                                  entryType);
                            return GT_FALSE;//XP_ERR_INVALID_DATA;

                        }

                        fdbEntryType = XPS_FDB_ENTRY_TYPE_MAC_ADDR_E;
                        cpss_status = cpssHalConvertCpssToGenericMacEntry(devId, &cpssMacEntry,
                                                                          fdbEntryType, (void*)&fdbEntry);
                        if (cpss_status != GT_OK)
                        {

                            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                                  "could not convert cpss to xps mac \n");
                            return GT_FALSE;//xpsConvertCpssStatusToXPStatus(cpss_status);
                        }

                        cpss_status = cpssHalBrgFdbMacEntryIndexFind(devId, fdbEntryType,
                                                                     (void*)&fdbEntry, &index, &bankIndex);
                        if (cpss_status == GT_FULL)
                        {
                            /*couldn't find empty index because table is full*/
                            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                                  "Entry is not found FDB table is full");
                            return GT_FALSE;//xpsConvertCpssStatusToXPStatus(cpss_status);
                        }
                        else if ((cpss_status != GT_NOT_FOUND) && (cpss_status != GT_OK))
                        {
                            /*some other error other than finding empty index*/
                            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                                  "Fdb Index find error type: (%d)", xpsConvertCpssStatusToXPStatus(cpss_status));
                            return GT_FALSE;//xpsConvertCpssStatusToXPStatus(cpss_status);
                        }

                        if ((cpss_status == GT_NOT_FOUND) || (macAddressData[index].valid == false))
                        {
                            fdbAction = XP_FDB_ENTRY_ADD;
                        }
                        else if (cpss_status ==  GT_OK) /*trying to learn the already existing entry*/
                        {
                            /*check if FDB entry in HW and trying to learn is same*/
                            xpStatus = xpsFdbGetEntryByIndex(devId, index, &hwFdbEntry);

                            if (xpStatus != XP_NO_ERR)
                            {
                                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                                      "Failed to get entry at Index: (%d)",
                                      xpsConvertCpssStatusToXPStatus(cpss_status));
                                return GT_FALSE;//xpStatus;
                            }
                            if (hwFdbEntry.intfId != fdbEntry.intfId)
                            {
                                fdbAction = XP_FDB_ENTRY_MODIFY;
                            }

                        }

                        if ((fdbAction == XP_FDB_ENTRY_MODIFY) || (fdbAction == XP_FDB_ENTRY_ADD))
                        {
                            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_DEBUG,
                                  "adding entry vlan %d mac=%02x:%02x:%02x:%02x:%02x:%02x at index %d\n",
                                  fdbEntry.vlanId,
                                  fdbEntry.macAddr[0],
                                  fdbEntry.macAddr[1],
                                  fdbEntry.macAddr[2],
                                  fdbEntry.macAddr[3],
                                  fdbEntry.macAddr[4],
                                  fdbEntry.macAddr[5],
                                  index);

                            /*Create a new Fdb Mac Entry at the specified index or update existing mac*/
                            cpss_status = (XP_STATUS) cpssHalBrgFdbMacEntryWrite(devId, index, GT_FALSE,
                                                                                 fdbEntryType, (void*)&fdbEntry);

                            if (cpss_status != GT_OK)
                            {
                                return GT_FALSE;//xpsConvertCpssStatusToXPStatus(cpss_status);
                            }
                            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_DEBUG,
                                  "added fdb at index=%d ret=%d \n", index, cpss_status);

                            /*update local DB*/
                            macAddressData[index].valid = true;
                            macAddressData[index].macAge = 0;
                        }
                        /*Notify the application about MAC learning/Mac Move*/

                        if (learnHandler)
                        {
                            if (fdbEntry.vlanId >= XPS_MAX_USER_DEFINED_VLANS(devId))
                            {
                                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_DEBUG,
                                      " The application is not getting notified - vlanId %d", fdbEntry.vlanId);
                                return (GT_TRUE);
                            }

                            else
                            {
                                xpStatus = learnHandler(devId, fdbEntry, fdbAction);
                                if (xpStatus != XP_NO_ERR)
                                {
                                    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                                          "Failed to call LearnHandler: (%d)", xpStatus);
                                    return GT_FALSE;//xpStatus;
                                }
                            }
                        }
                        break;
                    }

                case CPSS_QA_E:
                case CPSS_TA_E:
                case CPSS_QR_E:
                case CPSS_AA_E:
                case CPSS_SA_E:
                case CPSS_QI_E:
                case CPSS_FU_E:
                default:
                    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                          "received wrong update message type asic[%d] updType[%d]", asic_num,
                          bufferArr[auIndex].updType);
                    continue;
            }
        }
    }

    return (GT_TRUE);
}

/*******************************************************************************
* fdbLearningEventsHndlr
*
* DESCRIPTION:
*       This routine is the event handler for Event-Request-Driven mode
*       (polling mode).
*
* INPUTS:
*       param - The process data structure.
*
* OUTPUTS:
*       None.
*
* RETURNS: GT_OK ot GT_FALSE
*
* COMMENTS:
*       None.
*
*******************************************************************************/
static unsigned __TASKCONV fdbLearningEventsHndlr(GT_VOID * param)
{
    xpDevice_t devId =0;
    GT_STATUS
    rc;                                         /* return code         */
    GT_U32
    i;                                          /* iterator            */
    GT_UINTPTR
    evHndl;                                     /* event handler       */
    GT_U32
    evBitmapArr[CPSS_UNI_EV_BITMAP_SIZE_CNS];   /* event bitmap array  */
    GT_U32
    evBitmap;                                   /* event bitmap 32 bit */
    GT_U32
    evExtData;                                  /* event extended data */
    GT_U8
    devNum;                                     /* device number       */
    CPSS_UNI_EV_CAUSE_ENT
    uniEv;                                    /* unified event cause */
    GT_U32
    evCauseIdx;                                 /* event index         */
    EV_HNDLR_PARAM
    *hndlrParamPtr;                             /* bind event array    */

    hndlrParamPtr = (EV_HNDLR_PARAM*)param;
    evHndl        = hndlrParamPtr->evHndl;
    while (1)
    {
        XPS_DEVICES_PER_SWITCH_ITER(devId, devNum)
        {
            rc = cpssEventSelect(evHndl, NULL, evBitmapArr,
                                 (GT_U32)CPSS_UNI_EV_BITMAP_SIZE_CNS);
            if (GT_OK != rc)
            {
                continue;
            }

            for (evCauseIdx = 0; evCauseIdx < CPSS_UNI_EV_BITMAP_SIZE_CNS; evCauseIdx++)
            {
                if (evBitmapArr[evCauseIdx] == 0)
                {
                    continue;
                }

                evBitmap = evBitmapArr[evCauseIdx];

                for (i = 0; evBitmap; evBitmap >>= 1, i++)
                {
                    if ((evBitmap & 1) == 0)
                    {
                        continue;
                    }
                    uniEv = (CPSS_UNI_EV_CAUSE_ENT)((evCauseIdx << 5) + i);

                    if ((rc=cpssEventRecv(evHndl, uniEv, &evExtData, &devNum)) == GT_OK)
                    {
                        rc = fdbLearningEventHandler(devNum, uniEv, evExtData);

                    }
                }
            }
        }
    }

    return (GT_TRUE);

}

XP_STATUS xpsFdbInit(void)
{
    XPS_FUNC_ENTRY_LOG();

    return xpsFdbInitScope(XP_SCOPE_DEFAULT);

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsFdbInitScope(xpsScope_t scopeId)
{
    XPS_FUNC_ENTRY_LOG();

    xpsLockCreate("XP_LOCKINDEX_XPS_FDB_LOCK", XP_LOCKINDEX_XPS_FDB_LOCK);

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsFdbDeInit(void)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsFdbDeInitScope(xpsScope_t scopeId)
{
    XPS_FUNC_ENTRY_LOG();
    GT_U8 devId = 0;
    GT_STATUS cpssStatus = GT_OK;
    GT_U8 devNum;

    CPSS_UNI_EV_CAUSE_ENT   evHndlrArr[] =
    {
        CPSS_PP_EB_AUQ_PENDING_E,
        CPSS_PP_MAC_MESSAGE_TO_CPU_READY_E
    };

    GT_U32 evHndlrArrSize = sizeof(evHndlrArr)/sizeof(evHndlrArr[0]);
    GT_U32 ii;

    XPS_DEVICES_PER_SWITCH_ITER(devId, devNum)
    {
        for (ii=0; ii<evHndlrArrSize; ii++)
        {
            cpssStatus = cpssEventDeviceMaskSet(devNum, evHndlrArr[ii], CPSS_EVENT_MASK_E);

            if (cpssStatus != GT_OK)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "cpss Event device mask set failed in fdb deinit\n");
                return xpsConvertCpssStatusToXPStatus(cpssStatus);
            }
        }
    }

    xpsLockDestroy(XP_LOCKINDEX_XPS_FDB_LOCK);

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}


/* api for configuring fdb defaults */
XP_STATUS xpsFdbDefaults(xpDevice_t devId)
{
    XPS_FUNC_ENTRY_LOG();
    GT_STATUS rc = GT_OK;
    XP_STATUS xpStatus = XP_NO_ERR;
    /*##############################TODO FDB ##############################################*/
    /* Enables FDB actions */
    rc = cpssHalSetBrgFdbActionsEnable(devId, GT_TRUE);
    if (rc != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to enble fdb actions\n");
        return xpsConvertCpssStatusToXPStatus(rc);
    }

    /* Disable sending NA messages to the CPU indicating that the device
       cannot learn a new SA*/
    rc = cpssHalSetBrgFdbNaMsgOnChainTooLong(devId, GT_FALSE);
    if (rc != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to disable NA messages sending limit to CPU\n");
        return xpsConvertCpssStatusToXPStatus(rc);
    }

    /* Disable the PP to/from sending an AA and TA address
       update messages to the CPU.*/
    rc = cpssHalSetBrgFdbAAandTAToCpu(devId, GT_FALSE);
    if (rc != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to disable AA TA update to CPU\n");
        return xpsConvertCpssStatusToXPStatus(rc);
    }

    /* Set Address Update CPU messages rate limitation */
    rc = cpssHalSetBrgFdbAuMsgRateLimit(devId, CPSS_MSG_RATE_LIMIT, GT_TRUE);
    if (rc != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to set rate limit to  CPU\n");
        return xpsConvertCpssStatusToXPStatus(rc);
    }

    /* Set for given security breach event it's drop mode */
    rc = cpssHalSetBrgSecurBreachEventDropMode(devId,
                                               CPSS_BRG_SECUR_BREACH_EVENTS_MOVED_STATIC_E, CPSS_DROP_MODE_SOFT_E);
    if (rc != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to set security breach event drop mode\n");
        return xpsConvertCpssStatusToXPStatus(rc);
    }

    rc = cpssHalSetBrgSecurBreachMovedStaticAddr(devId, GT_TRUE);
    if (rc != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to set security breach moved static address set\n");
        return xpsConvertCpssStatusToXPStatus(rc);
    }

    /* Set for given security breach event it's drop mode */
    rc = cpssHalSetBrgSecurBreachEventDropMode(devId,
                                               CPSS_BRG_SECUR_BREACH_EVENTS_INVALID_MAC_SA_E, CPSS_DROP_MODE_HARD_E);
    if (rc != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to set security breach CPSS_BRG_SECUR_BREACH_EVENTS_INVALID_MAC_SA_E set\n");
        return xpsConvertCpssStatusToXPStatus(rc);
    }

    /* Sets Mac address table Triggered action mode */
    rc = cpssHalSetBrgFdbMacTriggerMode(devId, CPSS_ACT_TRIG_E);
    if (rc != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to set Mac address table Triggered action mode\n");
        return xpsConvertCpssStatusToXPStatus(rc);
    }

    /* Sets FDB action mode without setting Action Trigger to Age without delete */
    rc = cpssHalSetBrgFdbActionMode(devId, CPSS_FDB_ACTION_AGE_WITHOUT_REMOVAL_E);
    if (rc != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to set FDB action mode without setting action trigger to age without delete\n");
        return xpsConvertCpssStatusToXPStatus(rc);
    }

    /* Drop all Ethernet packets with MAC SA that are Multicast*/
    rc = cpssHalEnableBrgGenDropInvalidSa(devId, GT_TRUE);
    if (rc != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to drop all ethernet packets with MAC SA that are Multicast\n");
        return xpsConvertCpssStatusToXPStatus(rc);
    }

    /* Disable reading FDB entries via AU messages to the CPU */
    rc = cpssHalSetBrgFdbUploadEnable(devId, GT_FALSE);
    if (rc != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to set disable reading FDB entries via AU messages to CPU\n");
        return xpsConvertCpssStatusToXPStatus(rc);
    }

    /* Sets the VLAN Lookup mode */
    rc = cpssHalSetBrgFdbMacVlanLookupMode(devId, CPSS_IVL_E);
    if (rc != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to set the VLAN Lookup Mode\n");
        return xpsConvertCpssStatusToXPStatus(rc);
    }

    /* Sets the FDB hash function mode */
    /*rama*/
    if (PRV_CPSS_SIP_6_CHECK_MAC(devId) == GT_FALSE)
    {
        rc = cpssHalSetBrgFdbHashMode(devId, CPSS_MAC_HASH_FUNC_CRC_MULTI_HASH_E);
        if (rc != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to set FDB HASH Mode Mode\n");
            return xpsConvertCpssStatusToXPStatus(rc);
        }
    }
    /* Disable dropping the Address Update messages when the queue is full */
    rc = cpssHalSetBrgFdbDropAuEnable(devId, GT_FALSE);
    if (rc != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to Disable dropping AU messages when queue is full\n");
        return xpsConvertCpssStatusToXPStatus(rc);
    }

    /* Disables destination address-based aging */
    rc = cpssHalSetBrgFdbAgeBitDaRefreshEnable(devId, GT_FALSE);
    if (rc != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Disable destination address based aging\n");
        return xpsConvertCpssStatusToXPStatus(rc);
    }

#ifndef ASIC_SIMULATION
    /* Traffic priority over CPU access to FDB table */
    rc = cpssHalSetDrvPpHwRegField(devId,
                                   CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                   0x0B00006C, 1, 1, 0);
    if (rc != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "failed to set fdb traffic priority to cpu access\n");
        return xpsConvertCpssStatusToXPStatus(rc);
    }
#endif

    /*init local db for macaddressdata*/
    macAddressData = (macData*)cpssOsMalloc(sizeof(macData)*FDB_TABLE_SIZE(devId));
    for (uint32_t j=0; j<FDB_TABLE_SIZE(devId); j++)
    {
        memset(&macAddressData[j], 0, sizeof(macData));
    }
    /*loop through all user ports and do appropriate port Settings default*/
    uint8_t maxTotalPorts = 0;
    int portNum;
    int  cpssDevNum;
    int cscdGPort;
    int maxPorts;
    uint32_t cpssPortNum;

    if ((xpStatus= xpsGlobalSwitchControlGetMaxPorts(devId,
                                                     &maxTotalPorts)) != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Can't get max physical port number. error code:%d\n", xpStatus);
    }

    XPS_GLOBAL_PORT_ITER(portNum, maxTotalPorts)
    {
        cpssDevNum = xpsGlobalIdToDevId(devId, portNum);
        cpssPortNum = xpsGlobalPortToPortnum(devId, portNum);

        /* disable learning of new source MAC addresses for packets received
           on specified port*/
        rc = cpssDxChBrgFdbPortLearnStatusSet(cpssDevNum, cpssPortNum, GT_FALSE,
                                              CPSS_LOCK_FRWRD_E);
        if (rc != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "failed to disable learning of new SMAC for packets recieved on port %d\n",
                  cpssPortNum);
            return xpsConvertCpssStatusToXPStatus(rc);
        }

        /* Enable forwarding a new mac address message to CPU */
        rc = cpssDxChBrgFdbNaToCpuPerPortSet(cpssDevNum, cpssPortNum, GT_TRUE);
        if (rc != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "failed to enable forwarding of new MAC to CPUfor packets recieved on port %d\n",
                  cpssPortNum);
            return xpsConvertCpssStatusToXPStatus(rc);
        }

    }

    /*loop through all cascade ports and do default settings*/
    XPS_B2B_CSCD_PORTS_ITER(cpssDevNum, portNum, cscdGPort, maxPorts)
    {
        /* disable learning of new source MAC addresses for packets received
           on specified port*/
        rc = cpssDxChBrgFdbPortLearnStatusSet(cpssDevNum, cscdGPort, GT_FALSE,
                                              CPSS_LOCK_FRWRD_E);
        if (rc != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "failed to disable learning of new SMAC on cascade port %d\n", cscdGPort);
            return xpsConvertCpssStatusToXPStatus(rc);
        }

        /* Disable forwarding a new mac address message to CPU */
        rc = cpssDxChBrgFdbNaToCpuPerPortSet(cpssDevNum, cscdGPort, GT_FALSE);
        if (rc != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "failed to disable forwarding  of new SMAC on cascade port %d\n", cscdGPort);
            return xpsConvertCpssStatusToXPStatus(rc);
        }
    }
    XPS_FUNC_EXIT_LOG();
    return XP_NO_ERR;
}


/*api to prevent security breach event by MAC*/
XP_STATUS xpsFdbMacSecurity(xpDevice_t devId)
{
    XPS_FUNC_ENTRY_LOG();

    GT_STATUS status = GT_OK;
    status = cpssHalSetBrgSecurBreachEventPacketCommand(devId,
                                                        CPSS_BRG_SECUR_BREACH_EVENTS_MAC_SA_IS_DA_E, CPSS_PACKET_CMD_DROP_HARD_E);
    if (xpsConvertCpssStatusToXPStatus(status) != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to set action for security breach when SMAC is equal to DMAC\n");
        return xpsConvertCpssStatusToXPStatus(status);
    }

    status = cpssHalSetBrgSecurBreachEventPacketCommand(devId,
                                                        CPSS_BRG_SECUR_BREACH_EVENTS_MAC_SA_IS_ZERO_E, CPSS_PACKET_CMD_DROP_HARD_E);
    if (xpsConvertCpssStatusToXPStatus(status) != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to set action for security breach when SMAC is zero\n");
        return xpsConvertCpssStatusToXPStatus(status);
    }

    status = cpssHalSetBrgSecurBreachEventPacketCommand(devId,
                                                        CPSS_BRG_SECUR_BREACH_EVENTS_INVALID_MAC_SA_E, CPSS_PACKET_CMD_DROP_HARD_E);
    if (xpsConvertCpssStatusToXPStatus(status) != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to set action for security breach when SMAC is invalid\n");
        return xpsConvertCpssStatusToXPStatus(status);
    }

    XPS_FUNC_EXIT_LOG();
    return XP_NO_ERR;
}


XP_STATUS xpsFdbAddDeviceDefault(xpsDevice_t devId, xpsInitType_t initType)
{
    XPS_FUNC_ENTRY_LOG();
    XP_STATUS status = XP_NO_ERR;
    GT_STATUS cpssStatus;
    void   *params=NULL;

    /*add device init parameters and related changes here*/

    status = xpsFdbDefaults(devId);
    if (status != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Fdb defaults failed\n");
        return status;
    }

    status = xpsFdbMacSecurity(devId);
    if (status != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Setting MAC security failed\n");
        return status;
    }

    /*event handle task creations for aging and learning*/
    CPSS_UNI_EV_CAUSE_ENT   evHndlrCauseArr[] =
    {
        CPSS_PP_EB_AUQ_PENDING_E,
        CPSS_PP_MAC_MESSAGE_TO_CPU_READY_E
    };

    GT_U32 evHndlrCauseArrSize = sizeof(evHndlrCauseArr)/sizeof(evHndlrCauseArr[0]);
    GT_U32 ii;
    GT_U8 devNum;
    static EV_HNDLR_PARAM taskParamArr;

    taskParamArr.hndlrIndex = 0;
    taskParamArr.evHndl = (GT_UINTPTR)(&fdbLearningEventsHndlr);
    GT_STATUS rc = GT_OK;
    /* call CPSS to bind the events under single handler */
    rc = cpssEventBind(evHndlrCauseArr,
                       evHndlrCauseArrSize,
                       &taskParamArr.evHndl);
    switch (rc)
    {
        case GT_FAIL:
        case GT_BAD_PTR:
        case GT_OUT_OF_CPU_MEM:
        case GT_FULL:
        case GT_ALREADY_EXIST:
            break;
    }

    if (GT_OK != rc)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Event binding failed");
        return xpsConvertCpssStatusToXPStatus(rc);
    }


    XPS_DEVICES_PER_SWITCH_ITER(devId, devNum)
    {
        for (ii=0; ii<evHndlrCauseArrSize; ii++)
        {
            cpssStatus = cpssEventDeviceMaskSet(devNum, evHndlrCauseArr[ii],
                                                CPSS_EVENT_UNMASK_E);

            if (cpssStatus != GT_OK)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "cpss Event device mask set failed in fdb add device\n");
                return xpsConvertCpssStatusToXPStatus(cpssStatus);
            }
        }
    }

    max_au_per_second = _8K;

    /*create Aging task*/
    if (agingTidPtr == NULL)
    {
        cpssStatus = cpssOsTaskCreate("AgingTask",
                                      AGING_TASK_PRIO,
                                      _24KB,
                                      AgingTaskFunc,
                                      params,
                                      &agingTid);
        if (cpssStatus != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "cpssOsTaskCreate failed to creating AgingTask\n");
            return xpsConvertCpssStatusToXPStatus(cpssStatus);
        }
    }

    agingTidPtr = &agingTid;

    /*create Learning task*/
    if (learningTidPtr == NULL)
    {
        /* Create Learning Task */
        cpssStatus = cpssOsTaskCreate("LearningTask",
                                      LEARNING_TASK_PRIO,
                                      _24KB,
                                      fdbLearningEventsHndlr,
                                      &taskParamArr,
                                      &learningTid);
        if (cpssStatus != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "cpssOsTaskCreate failed to creating AgingTask\n");
            return xpsConvertCpssStatusToXPStatus(cpssStatus);
        }
    }

    learningTidPtr = &learningTid;

    /*HASH mode set is taken care by L3 host init, don't want to repete this here*/
    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsFdbRemoveDeviceDefault(xpsDevice_t devId)
{
    XPS_FUNC_ENTRY_LOG();

    GT_STATUS status = GT_OK;

    /*delete Aging task*/
    if (agingTidPtr)
    {
        status = cpssOsTaskDelete(agingTid);
        cpssOsPrintf("Aging Task Deleted %d.\n", status);
    }
    agingTidPtr = NULL;
    agingTid = 0;

    /*delete Learning task*/
    if (learningTidPtr)
    {
        status = cpssOsTaskDelete(learningTid);
        cpssOsPrintf("Learning Task Deleted %d.\n", status);
    }
    learningTidPtr = NULL;
    learningTid = 0;

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsFdbTriggerAging(xpsDevice_t devId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsFdbConfigureTableAging(xpsDevice_t devId, uint32_t enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsFdbConfigureEntryAging(xpsDevice_t devId, uint32_t enable,
                                    uint32_t index)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsFdbGetTableAgingStatus(xpsDevice_t devId, uint8_t *enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsFdbGetEntryAgingStatus(xpsDevice_t devId, uint8_t *enable,
                                    uint32_t index)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsFdbSetAgingTime(xpsDevice_t devId, uint32_t agingTime)
{
    XPS_FUNC_ENTRY_LOG();

    gAgingTimer = agingTime;
    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_DEFAULT, "Set aging time to %d\n",
          agingTime);

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsFdbGetAgingTime(xpsDevice_t devId, uint32_t *agingTime)
{
    XPS_FUNC_ENTRY_LOG();

    *agingTime = gAgingTimer;

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}



/*
1. Check if present count more than or equal to max count.
1. Increment the current count.
2. If current count equal to max count, disable sa learning in vlan
**/

XP_STATUS xpsFdbAddHwEntryDefault(xpsDevice_t devId, xpsFdbEntry_t *fdbEntry,
                                  xpsHashIndexList_t *indexList)
{
    XPS_FUNC_ENTRY_LOG();

    XP_STATUS status = XP_NO_ERR;
    GT_STATUS cpss_status = GT_OK;
    uint8_t isFdbLimitReached = 0;
    xpsScope_t scopeId;
    xpsL2EncapType_e encapType;
    XPS_FDB_ENTRY_TYPE_ENT fdbEntryType = XPS_FDB_ENTRY_TYPE_MAC_ADDR_E;
    uint32_t index, bankIndex = 0;

    if (indexList == NULL)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Null pointer received");
        return XP_ERR_NULL_POINTER;
    }


    /* Get Scope Id from devId */
    if ((status = xpsScopeGetScopeId(devId, &scopeId))!= XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Getting scope id failed");
        return status;
    }
    //TODO: this is a hack to fix SONIC-191. The values are not used in the further logic
    //it is only used to ensure that FDB can be added only for a member port
    //of the fdb vlan.
    if ((status = xpsVlanGetIntfTagTypeScope(scopeId, fdbEntry->vlanId,
                                             fdbEntry->intfId, &encapType)) != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "vlan get interface tag type scope failed");
        return status;
    }

    status = xpsVlanIsFdbLimitReached(devId, fdbEntry->vlanId, &isFdbLimitReached);
    if (status)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Fdb limit query failed");
        return status;
    }

    if (isFdbLimitReached)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Resource not available");
        return XP_ERR_RESOURCE_NOT_AVAILABLE;
    }

    cpss_status = cpssHalBrgFdbMacEntryIndexFind(devId, fdbEntryType, fdbEntry,
                                                 &index, &bankIndex);
    if (cpss_status == GT_OK)
    {
        /*Entry exists already and index contains entry location so return entry exists*/
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Entry already exists in fdb table at index (%d)", index);
        return XP_ERR_KEY_EXISTS;
    }
    else if (cpss_status == GT_FULL)
    {
        /*couldn't find empty index because table is full*/
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Entry is not found FDB table is full");
        return xpsConvertCpssStatusToXPStatus(cpss_status);
    }
    else if (cpss_status != GT_NOT_FOUND)
    {
        /*some other error other than finding empty index*/
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Fdb Index find error type: (%d)", xpsConvertCpssStatusToXPStatus(cpss_status));
        return xpsConvertCpssStatusToXPStatus(cpss_status);
    }

    /*Create a new Fdb Mac Entry at the specified index */
    cpss_status = (XP_STATUS) cpssHalBrgFdbMacEntryWrite(devId, index, GT_FALSE,
                                                         fdbEntryType, fdbEntry);

    if (cpss_status != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Fdb entry write failed");
        return xpsConvertCpssStatusToXPStatus(cpss_status);
    }

    indexList->size = 1;
    indexList->index[0] = index;

    //Increase count and disable learning if required
    status = xpsVlanIncrementFdbCount(devId, fdbEntry->vlanId);
    if (status)
    {
        // Print error log
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Error: Invalid device %d\n",
              devId);
        return status;
    }

    XPS_FUNC_EXIT_LOG();
    return status;
}


XP_STATUS xpsFdbAddEntryDefault(xpsDevice_t devId, xpsFdbEntry_t *fdbEntry,
                                xpsHashIndexList_t *indexList)
{
    XPS_FUNC_ENTRY_LOG();


    //Get private-vlan Type
    XP_STATUS result;
    xpsVlan_t vlanId;
    xpsPrivateVlanType_e vlanType = VLAN_NONE;

    if (indexList == NULL)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Null pointer received");
        return XP_ERR_NULL_POINTER;
    }

    if (IS_DEVICE_VALID(devId) == 0)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Error: Invalid device %d\n",
              devId);
        return XP_ERR_INVALID_DEV_ID;
    }

    if (!fdbEntry)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Null pointer received");
        return XP_ERR_NULL_POINTER;
    }

    vlanId = fdbEntry->vlanId;

    result = xpsPVlanGetType(devId, vlanId, &vlanType);
    if (result != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Error: %s failed with error code: %d\n", __FUNCNAME__, result);
        return result;
    }

    if (vlanType == VLAN_NONE)
    {
        result = xpsFdbAddHwEntry(devId, fdbEntry, indexList);
        if (result != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Error: %s failed with error code: %d\n", __FUNCNAME__, result);
            return result;
        }
    }
    else
    {
        result = xpsPVlanAddFdb(devId, fdbEntry, vlanType, indexList);
        if (result != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Error: %s failed with error code: %d\n", __FUNCNAME__, result);
            return result;
        }
    }

    /*update local DB for dynamic Mac Aging*/
    if ((indexList->size == 1) && (fdbEntry->isStatic == 0))
    {
        macAddressData[indexList->index[0]].valid = true;
        macAddressData[indexList->index[0]].macAge = 0;
    }

    XPS_FUNC_EXIT_LOG();
    return result;
}


XP_STATUS xpsFdbWriteEntryDefault(xpsDevice_t devId,  uint32_t index,
                                  xpsFdbEntry_t *fdbEntry)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_LOCK(xpsFdbWriteEntryDefault);
    XP_STATUS status = XP_NO_ERR;
    xpsScope_t scopeId;
    XPS_FDB_ENTRY_TYPE_ENT fdbEntryType = XPS_FDB_ENTRY_TYPE_MAC_ADDR_E;
    GT_STATUS cpss_status = GT_OK;

    if (IS_DEVICE_VALID(devId) == 0)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Error: Invalid device %d\n",
              devId);
        return XP_ERR_INVALID_DEV_ID;
    }
    /* Get Scope Id from devId */
    if ((status = xpsScopeGetScopeId(devId, &scopeId))!= XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Getting scope id failed");
        return status;
    }

    if (!fdbEntry)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Null pointer received");
        return XP_ERR_NULL_POINTER;
    }


    cpss_status = cpssHalBrgFdbMacEntryWrite(devId, index, GT_FALSE, fdbEntryType,
                                             (void *) fdbEntry);

    XPS_FUNC_EXIT_LOG();

    return xpsConvertCpssStatusToXPStatus(cpss_status);
}


XP_STATUS xpsFdbRemoveEntryByIndex(xpsDevice_t devId, uint32_t index)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_LOCK(xpsFdbRemoveEntryByIndex);

    GT_STATUS               cpss_status = GT_OK;
    GT_BOOL                 valid = GT_FALSE;
    GT_BOOL                 skip  = GT_FALSE;
    GT_BOOL                 aged  = GT_FALSE;
    GT_HW_DEV_NUM           hwDev   = 0;
    xpsFdbEntry_t           fdbEntry;
    XPS_FDB_ENTRY_TYPE_ENT  fdbEntryType = XPS_FDB_ENTRY_TYPE_MAC_ADDR_E;
    XP_STATUS retVal = XP_NO_ERR;

    if (IS_DEVICE_VALID(devId) == 0)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Error: Invalid device %d\n",
              devId);
        return XP_ERR_INVALID_DEV_ID;
    }

    memset(&fdbEntry, 0x00, sizeof(xpsFdbEntry_t));
    /*Get Fdb Entry from HW*/
    cpss_status = cpssHalBrgFdbMacEntryRead(devId, index, &valid, &skip, &aged,
                                            &hwDev, fdbEntryType, (void *)&fdbEntry);

    if (cpss_status != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Fdb entry read failed");
        return xpsConvertCpssStatusToXPStatus(cpss_status);
    }

    /* Delete FDB MACEntry*/
    cpss_status = cpssHalBrgFdbMacEntryInvalidate(devId, index);

    if (cpss_status != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Fdb entry invalidate failed");
        return xpsConvertCpssStatusToXPStatus(cpss_status);
    }


    //Decrement counter and enable sa learning if needed
    retVal = xpsVlanDecrementFdbCount(devId, fdbEntry.vlanId);
    if (retVal)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Decrement in fdb failed");
        return retVal;
    }

    XPS_FUNC_EXIT_LOG();

    return retVal;
}

XP_STATUS xpsFdbRemoveEntryDefault(xpsDevice_t devId, xpsFdbEntry_t *fdbEntry)
{
    XPS_FUNC_ENTRY_LOG();

    XP_STATUS retVal = XP_NO_ERR;
    uint32_t index;

    if (IS_DEVICE_VALID(devId) == 0)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Error: Invalid device %d\n",
              devId);
        return XP_ERR_INVALID_DEV_ID;
    }

    // Find the fdb entry index
    retVal = xpsFdbFindEntry(devId, fdbEntry, &index);

    if (retVal != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Entry not found in table Error code : (%d)", retVal);
        return XP_ERR_KEY_EXISTS;
    }

    // Remove fdb entry by index

    retVal = xpsFdbRemoveEntryByIndex(devId, index);

    if (retVal)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Remove enrty by index failed");
        return retVal;
    }

    XPS_FUNC_EXIT_LOG();

    return retVal;
}


XP_STATUS xpsFdbFindEntryDefault(xpsDevice_t devId, xpsFdbEntry_t *fdbEntry,
                                 uint32_t *index)
{
    XPS_FUNC_ENTRY_LOG();
    xpsLockTake(XP_LOCKINDEX_XPS_FDB_LOCK);
    XPS_LOCK(xpsFdbFindEntry);

    XPS_FDB_ENTRY_TYPE_ENT fdbEntryType = XPS_FDB_ENTRY_TYPE_MAC_ADDR_E;
    GT_STATUS cpss_status = GT_OK;
    GT_U32 bankIndex = 0;
    if (IS_DEVICE_VALID(devId) == 0)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Error: Invalid device %d\n",
              devId);
        xpsLockRelease(XP_LOCKINDEX_XPS_FDB_LOCK);
        return XP_ERR_INVALID_DEV_ID;
    }

    if ((!fdbEntry) || (!index))
    {
        xpsLockRelease(XP_LOCKINDEX_XPS_FDB_LOCK);
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Null pointer received");
        return XP_ERR_NULL_POINTER;
    }

    cpss_status = cpssHalBrgFdbMacEntryIndexFind(devId, fdbEntryType,
                                                 (void *) fdbEntry, (uint32_t *)index, &bankIndex);

    xpsLockRelease(XP_LOCKINDEX_XPS_FDB_LOCK);
    XPS_FUNC_EXIT_LOG();

    return xpsConvertCpssStatusToXPStatus(cpss_status);
}


XP_STATUS xpsFdbGetEntryDefault(xpsDevice_t devId, xpsFdbEntry_t *fdbEntry)
{
    XPS_FUNC_ENTRY_LOG();


    XP_STATUS retVal = XP_NO_ERR;
    uint32_t index;

    if (IS_DEVICE_VALID(devId) == 0)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Error: Invalid device %d\n",
              devId);
        return XP_ERR_INVALID_DEV_ID;
    }

    if (!fdbEntry)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Null pointer received");
        return XP_ERR_NULL_POINTER;
    }

    // Obtain the fdb index

    retVal = xpsFdbFindEntry(devId, fdbEntry, &index);
    if (retVal != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_DEFAULT,
              "Entry not found in table Error code : (%d)", retVal);
        return XP_ERR_KEY_EXISTS;
    }

    // Obtain fdb entry
    /*we will retrieve only from device 0 and return as, both devices in B2B case are programmed symetrically at same index.*/
    retVal = xpsFdbGetEntryByIndex(devId, index, fdbEntry);

    if (retVal != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Entry read at index (%d) fialed with error code : (%d)", index, retVal);
        return retVal;
    }

    XPS_FUNC_EXIT_LOG();

    return retVal;
}

XP_STATUS xpsFdbGetTableDepth(xpsDevice_t devId, uint32_t *depth)
{
    XPS_FUNC_ENTRY_LOG();

    *depth = cpssHalGetSKUmaxHashTable(devId);

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsFdbGetNumOfValidEntries(xpsDevice_t devId,
                                     uint32_t *numOfValidEntries)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsFdbGetEntryByIndexDefault(xpsDevice_t devId, uint32_t index,
                                       xpsFdbEntry_t *fdbEntry)
{
    XPS_FUNC_ENTRY_LOG();


    XPS_LOCK(xpsFdbGetEntryByIndexDefault);

    GT_STATUS               cpss_status = GT_OK;
    GT_BOOL                 valid = GT_FALSE;
    GT_BOOL                 skip  = GT_FALSE;
    GT_BOOL                 aged  = GT_FALSE;
    GT_HW_DEV_NUM           hwDev   = 0;
    XPS_FDB_ENTRY_TYPE_ENT  fdbEntryType = XPS_FDB_ENTRY_TYPE_MAC_ADDR_E;

    if (IS_DEVICE_VALID(devId) == 0)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Error: Invalid device %d\n",
              devId);
        return XP_ERR_INVALID_DEV_ID;
    }

    if (!fdbEntry)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Null pointer received");
        return XP_ERR_NULL_POINTER;
    }


    cpss_status = cpssHalBrgFdbMacEntryRead(devId, index, &valid, &skip, &aged,
                                            &hwDev, fdbEntryType, (void *)fdbEntry);

    if (cpss_status != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Fdb entry read failed");
        return xpsConvertCpssStatusToXPStatus(cpss_status);
    }

    XPS_FUNC_EXIT_LOG();

    return xpsConvertCpssStatusToXPStatus(cpss_status);
}

XP_STATUS xpsFdbAddControlMacEntry(xpsDevice_t devId, uint32_t vlanId,
                                   macAddr_t macAddr, uint32_t reasonCode, xpsHashIndexList_t *indexList)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsFdbRemoveControlMacEntry(xpsDevice_t devId, uint32_t vlanId,
                                      macAddr_t macAddr)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsFdbGetControlMacEntryReasonCode(xpsDevice_t devId, uint32_t vlanId,
                                             macAddr_t macAddr, uint32_t *reasonCode)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsFdbMacLearnigCb(xpsDevice_t devId, xphRxHdr *xphHdr, void *buf,
                             uint16_t bufSize, void *userData)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsFdbUnRegisterLearnHandlerDefault(xpsDevice_t devId)
{
    XPS_FUNC_ENTRY_LOG();

    learnHandler = NULL;

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsFdbRegisterLearnHandlerDefault(xpsDevice_t devId,
                                            xpFdbLearnHandler fdbLearnHandler)
{
    XPS_FUNC_ENTRY_LOG();

    learnHandler = fdbLearnHandler;
    prevLearnHandler = learnHandler;

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

/* Using this API to enable/disbale of processing learn events */
XP_STATUS xpsFdbSetResetLearnHandlerDefault(uint32_t isSet)
{
    if (isSet)
    {
        if (learnHandler)
        {
            return XP_NO_ERR;
        }
        learnHandler = prevLearnHandler;
    }
    else
    {
        prevLearnHandler = learnHandler;
        learnHandler = NULL;
    }
    return XP_NO_ERR;
}

XP_STATUS xpsFdbRegisterLearn(xpsDevice_t devId, xpsFdbLearnHandler fdbLearnCb,
                              void *userData)
{
    XPS_FUNC_ENTRY_LOG();
    //    macLearningCb = fdbLearnCb;
    //   return xpsFdbRegisterLearnHandler(devId, xpsFdbMacLearnigCb, userData);

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsFdbGetL2EncapType(xpsDevice_t devId, uint32_t intfId,
                               uint32_t bdId, uint8_t isTagged, uint32_t vlanId, uint32_t* encapType,
                               xpVlan_t *encapVid, uint8_t *transTnl, bool *isBdBridge)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}



XP_STATUS xpsFdbParseLearnPacket(xpDevice_t devId, xphRxHdr *xphHdr, void *buf,
                                 uint16_t bufSize, xpsFdbEntry_t *fdbEntry, uint32_t *reasonCode)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsFdbRegisterDefaultAgingHandler(xpsDevice_t devId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsFdbRegisterAgingHandlerDefault(xpsDevice_t devId,
                                            xpFdbAgingHandler fdbAgingHandler)
{
    XPS_FUNC_ENTRY_LOG();

    ageHandler = fdbAgingHandler;

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsFdbUnregisterAgingHandler(xpsDevice_t devId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsFdbFlushEntryDefault(xpsDevice_t devId,
                                  xpsFdbEntryType_e entryType)
{
    XPS_FUNC_ENTRY_LOG();
    XPS_LOCK(xpsFdbFlushEntryDefault);
    GT_STATUS status = GT_OK;

    xpsFlushParams flushParams;
    memset(&flushParams, 0x00, sizeof(xpsFlushParams));
    flushParams.intfId = 0;
    flushParams.vlanId = 0;
    flushParams.entryType = entryType;
    flushParams.flushType = XP_FDB_FLUSH_ALL;

    if (IS_DEVICE_VALID(devId) == 0)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Error: Invalid device %d\n",
              devId);
        return XP_ERR_INVALID_DEV_ID;
    }

    status = cpssHalFlushFdbEntry(devId, &flushParams);
    if (status != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Fdb Hal Flush Entry failed");
        return xpsConvertCpssStatusToXPStatus(status);
    }

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsFdbFlushEntryWithShadowUpdate(xpsDevice_t devId,
                                           xpsFdbEntryType_e entryType, uint8_t updateShadow)
{
    XPS_FUNC_ENTRY_LOG();
    XPS_LOCK(xpsFdbFlushEntryWithShadowUpdate);
    GT_STATUS status = GT_OK;

    xpsFlushParams flushParams;
    memset(&flushParams, 0x00, sizeof(xpsFlushParams));
    flushParams.intfId = 0;
    flushParams.vlanId = 0;
    flushParams.entryType = entryType;
    flushParams.flushType = XP_FDB_FLUSH_ALL;

    if (IS_DEVICE_VALID(devId) == 0)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Error: Invalid device %d\n",
              devId);
        return XP_ERR_INVALID_DEV_ID;
    }

    status = cpssHalFlushFdbEntry(devId, &flushParams);
    if (status != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Fdb Hal Flush Entry failed");
        return xpsConvertCpssStatusToXPStatus(status);
    }

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsFdbFlushEntryByIntfDefault(xpsDevice_t devId,
                                        xpsInterfaceId_t intfId, xpsFdbEntryType_e entryType)
{
    XPS_FUNC_ENTRY_LOG();
    XPS_LOCK(xpsFdbFlushEntryByIntfDefault);
    GT_STATUS status = GT_OK;

    xpsFlushParams flushParams;
    memset(&flushParams, 0x00, sizeof(xpsFlushParams));
    flushParams.intfId = intfId;
    flushParams.vlanId = 0;
    flushParams.entryType = entryType;
    flushParams.flushType = XP_FDB_FLUSH_BY_INTF;

    if (IS_DEVICE_VALID(devId) == 0)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Error: Invalid device %d\n",
              devId);
        return XP_ERR_INVALID_DEV_ID;
    }

    status = cpssHalFlushFdbEntry(devId, &flushParams);
    if (status != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Fdb Hal Flush Entry failed");
        return xpsConvertCpssStatusToXPStatus(status);
    }

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsFdbFlushEntryByIntfWithShadowUpdate(xpsDevice_t devId,
                                                 xpsInterfaceId_t intfId, xpsFdbEntryType_e entryType, uint8_t updateShadow)
{
    XPS_FUNC_ENTRY_LOG();
    XPS_LOCK(xpsFdbFlushEntryByIntfWithShadowUpdate);
    GT_STATUS status = GT_OK;

    xpsFlushParams flushParams;
    memset(&flushParams, 0x00, sizeof(xpsFlushParams));
    flushParams.intfId = intfId;
    flushParams.vlanId = 0;
    flushParams.entryType = entryType;
    flushParams.flushType = XP_FDB_FLUSH_BY_INTF;

    if (IS_DEVICE_VALID(devId) == 0)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Error: Invalid device %d\n",
              devId);
        return XP_ERR_INVALID_DEV_ID;
    }

    status = cpssHalFlushFdbEntry(devId, &flushParams);
    if (status != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Fdb Hal Flush Entry failed");
        return xpsConvertCpssStatusToXPStatus(status);
    }
    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;

}

XP_STATUS xpsFdbFlushEntryByVlanDefault(xpsDevice_t devId, xpsVlan_t vlanId,
                                        xpsFdbEntryType_e entryType)
{
    XPS_FUNC_ENTRY_LOG();
    XPS_LOCK(xpsFdbFlushEntryByVlanDefault);
    GT_STATUS status = GT_OK;

    xpsFlushParams flushParams;
    memset(&flushParams, 0x00, sizeof(xpsFlushParams));
    flushParams.intfId = 0;
    flushParams.vlanId = vlanId;
    flushParams.entryType = entryType;
    flushParams.flushType = XP_FDB_FLUSH_BY_VLAN;

    if (IS_DEVICE_VALID(devId) == 0)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Error: Invalid device %d\n",
              devId);
        return XP_ERR_INVALID_DEV_ID;
    }

    status = cpssHalFlushFdbEntry(devId, &flushParams);
    if (status != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Fdb Hal Flush Entry failed");
        return xpsConvertCpssStatusToXPStatus(status);
    }

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsFdbFlushEntryByVlanWithShadowUpdate(xpsDevice_t devId,
                                                 xpsVlan_t vlanId, xpsFdbEntryType_e entryType, uint8_t updateShadow)
{
    XPS_FUNC_ENTRY_LOG();
    XPS_LOCK(xpsFdbFlushEntryByVlanWithShadowUpdate);
    GT_STATUS status = GT_OK;

    xpsFlushParams flushParams;
    memset(&flushParams, 0x00, sizeof(xpsFlushParams));
    flushParams.intfId = 0;
    flushParams.vlanId = vlanId;
    flushParams.entryType = entryType;
    flushParams.flushType = XP_FDB_FLUSH_BY_VLAN;

    if (IS_DEVICE_VALID(devId) == 0)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Error: Invalid device %d\n",
              devId);
        return XP_ERR_INVALID_DEV_ID;
    }

    status = cpssHalFlushFdbEntry(devId, &flushParams);
    if (status != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Fdb Hal Flush Entry failed");
        return xpsConvertCpssStatusToXPStatus(status);
    }

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;

}

XP_STATUS xpsFdbFlushEntryByIntfVlanDefault(xpsDevice_t devId,
                                            xpsInterfaceId_t intfId, xpsVlan_t vlanId, xpsFdbEntryType_e entryType)
{
    XPS_FUNC_ENTRY_LOG();
    XPS_LOCK(xpsFdbFlushEntryByIntfVlanDefault);
    GT_STATUS status = GT_OK;

    xpsFlushParams flushParams;
    memset(&flushParams, 0x00, sizeof(xpsFlushParams));
    flushParams.intfId = intfId;
    flushParams.vlanId = vlanId;
    flushParams.entryType = entryType;
    flushParams.flushType = XP_FDB_FLUSH_BY_INTF_VLAN;

    if (IS_DEVICE_VALID(devId) == 0)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Error: Invalid device %d\n",
              devId);
        return XP_ERR_INVALID_DEV_ID;
    }

    status = cpssHalFlushFdbEntry(devId, &flushParams);
    if (status != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Fdb Hal Flush Entry failed");
        return xpsConvertCpssStatusToXPStatus(status);
    }

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsFdbFlushEntryByIntfVlanWithShadowUpdate(xpsDevice_t devId,
                                                     xpsInterfaceId_t intfId, xpsVlan_t vlanId, xpsFdbEntryType_e entryType,
                                                     uint8_t updateShadow)
{
    XPS_FUNC_ENTRY_LOG();
    XPS_LOCK(xpsFdbFlushEntryByIntfVlanWithShadowUpdate);
    GT_STATUS status = GT_OK;

    xpsFlushParams flushParams;
    memset(&flushParams, 0x00, sizeof(xpsFlushParams));
    flushParams.intfId = intfId;
    flushParams.vlanId = vlanId;
    flushParams.entryType = entryType;
    flushParams.flushType = XP_FDB_FLUSH_BY_INTF_VLAN;

    if (IS_DEVICE_VALID(devId) == 0)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Error: Invalid device %d\n",
              devId);
        return XP_ERR_INVALID_DEV_ID;
    }

    status = cpssHalFlushFdbEntry(devId, &flushParams);
    if (status != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Fdb Hal Flush Entry failed");
        return xpsConvertCpssStatusToXPStatus(status);
    }

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsFdbGetSourceMacHit(xpsDevice_t devId, uint32_t index,
                                uint8_t *hwFlagStatus)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsFdbClearSourceMacHit(xpsDevice_t devId, uint32_t index)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsFdbSetAttributeByIndex(xpsDevice_t devId, uint32_t index,
                                    xpsFdbAttribute_e field, void *data)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_LOCK(xpsFdbSetAttributeByIndex);

    GT_STATUS               cpss_status = GT_OK;
    GT_BOOL                 valid = GT_FALSE;
    GT_BOOL                 skip  = GT_FALSE;
    GT_BOOL                 aged  = GT_FALSE;
    GT_HW_DEV_NUM           hwDev   = 0;
    xpsFdbEntry_t           fdbEntry;
    XPS_FDB_ENTRY_TYPE_ENT  fdbEntryType = XPS_FDB_ENTRY_TYPE_MAC_ADDR_E;

    if (IS_DEVICE_VALID(devId) == 0)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Error: Invalid device %d\n",
              devId);
        return XP_ERR_INVALID_DEV_ID;
    }

    if (!data)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Null pointer received");
        return XP_ERR_NULL_POINTER;
    }

    memset(&fdbEntry, 0x00, sizeof(xpsFdbEntry_t));
    /*Get Fdb Entry from HW*/
    cpss_status = cpssHalBrgFdbMacEntryRead(devId, index, &valid, &skip, &aged,
                                            &hwDev, fdbEntryType, (void *)&fdbEntry);

    if (cpss_status != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Fdb entry read failed");
        return xpsConvertCpssStatusToXPStatus(cpss_status);
    }

    /*TODO VGUNTA, check if more cases need to be added, currently handles the same cases as in XPS earlier*/
    switch (field)
    {
        case XPS_FDB_PKT_CMD:
            fdbEntry.pktCmd = *(xpPktCmd_e *)data;
            break;
        case XPS_FDB_IS_ROUTER_MAC:
            fdbEntry.isRouter = *(uint8_t *)data;
            break;
        case XPS_FDB_IS_STATIC_MAC:
            fdbEntry.isStatic = *(uint8_t *)data;
            break;
        default:
            return XP_ERR_INVALID_DATA;
    }

    /*set FDB entry*/
    cpss_status = cpssHalBrgFdbMacEntryWrite(devId, index, GT_FALSE, fdbEntryType,
                                             (void *) &fdbEntry);

    XPS_FUNC_EXIT_LOG();

    return xpsConvertCpssStatusToXPStatus(cpss_status);
}

XP_STATUS xpsFdbGetAttributeByIndex(xpsDevice_t devId, uint32_t index,
                                    xpsFdbAttribute_e field, void *data)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_LOCK(xpsFdbGetAttributeByIndex);

    GT_STATUS               cpss_status = GT_OK;
    GT_BOOL                 valid = GT_FALSE;
    GT_BOOL                 skip  = GT_FALSE;
    GT_BOOL                 aged  = GT_FALSE;
    GT_HW_DEV_NUM           hwDev   = 0;
    xpsFdbEntry_t           fdbEntry;
    XPS_FDB_ENTRY_TYPE_ENT  fdbEntryType = XPS_FDB_ENTRY_TYPE_MAC_ADDR_E;

    if (IS_DEVICE_VALID(devId) == 0)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Error: Invalid device %d\n",
              devId);
        return XP_ERR_INVALID_DEV_ID;
    }

    if (!data)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Null pointer received");
        return XP_ERR_NULL_POINTER;
    }

    memset(&fdbEntry, 0x00, sizeof(xpsFdbEntry_t));
    /*Get Fdb Entry from HW*/
    cpss_status = cpssHalBrgFdbMacEntryRead(devId, index, &valid, &skip, &aged,
                                            &hwDev, fdbEntryType, (void *)&fdbEntry);

    if (cpss_status != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Fdb entry read failed");
        return xpsConvertCpssStatusToXPStatus(cpss_status);
    }

    /*TODO VGUNTA, check if more cases need to be added, currently handles the same cases as in XPS earlier*/
    switch (field)
    {
        case XPS_FDB_PKT_CMD:
            *(xpPktCmd_e *)data = fdbEntry.pktCmd;
            break;
        case XPS_FDB_IS_ROUTER_MAC:
            *(uint8_t *)data = fdbEntry.isRouter;
            break;
        case XPS_FDB_IS_STATIC_MAC:
            *(uint8_t *)data = fdbEntry.isStatic;
            break;
        default:
            return XP_ERR_INVALID_DATA;
    }

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsFdbSetAttributeDefault(xpsDevice_t devId, xpsFdbEntry_t *fdbEntry,
                                    xpsFdbAttribute_e field, void *data)
{
    XPS_FUNC_ENTRY_LOG();


    uint32_t index;
    XP_STATUS retVal = XP_NO_ERR;

    if (IS_DEVICE_VALID(devId) == 0)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Error: Invalid device %d\n",
              devId);
        return XP_ERR_INVALID_DEV_ID;
    }

    if (!fdbEntry || !data)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Null pointer received");
        return XP_ERR_NULL_POINTER;
    }

    // Find the fdb entry index
    retVal = xpsFdbFindEntry(devId, fdbEntry, &index);
    if (retVal != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Entry not found in table Error code : (%d)", retVal);
        return XP_ERR_KEY_EXISTS;
    }

    return (xpsFdbSetAttributeByIndex(devId, index, field, data));

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsFdbGetAttributeDefault(xpsDevice_t devId, xpsFdbEntry_t *fdbEntry,
                                    xpsFdbAttribute_e field, void *data)
{
    XPS_FUNC_ENTRY_LOG();

    uint32_t index;
    XP_STATUS retVal = XP_NO_ERR;

    if (IS_DEVICE_VALID(devId) == 0)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Error: Invalid device %d\n",
              devId);
        return XP_ERR_INVALID_DEV_ID;
    }

    if (!fdbEntry || !data)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Null pointer received");
        return XP_ERR_NULL_POINTER;
    }

    // Find the fdb entry index
    retVal = xpsFdbFindEntry(devId, fdbEntry, &index);

    if (retVal != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Entry not found in table Error code : (%d)", retVal);
        return XP_ERR_KEY_EXISTS;
    }

    return (xpsFdbGetAttributeByIndex(devId, index, field, data));

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

extern void cpssHalDumpUtlFdbDump(GT_BOOL);
XP_STATUS xpsFdbDisplayTable(xpsDevice_t devId, uint32_t *numOfValidEntries,
                             uint32_t startIndex, uint32_t endIndex, char * logFile, uint32_t detailFormat,
                             uint32_t silentMode)
{
    XPS_FUNC_ENTRY_LOG();
    cpssHalDumpUtlFdbDump(GT_FALSE);
    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsFdbGetMacRate(xpsDevice_t devId,  uint64_t *rate)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsFdbSetRehashLevel(xpDevice_t devId, uint8_t numOfRehashLevels)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsFdbGetRehashLevel(xpDevice_t devId, uint8_t* numOfRehashLevels)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsFdbGetTableSize(xpsDevice_t devId, uint32_t *tsize)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}
XP_STATUS xpsFdbGetValidEntryByIndex(xpsDevice_t devId, uint32_t index,
                                     xpsFdbEntry_t *fdbEntry)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}
XP_STATUS xpsCountFdbObjects(xpsDevice_t xpsDevId, uint32_t* count)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsFdbClearBucketState(xpDevice_t devId, uint8_t tblCopyIdx)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsFdbSetMsgRateLimit(xpsDevice_t devId, uint32_t rateLimit,
                                uint8_t enable)
{
    GT_STATUS status = GT_OK;
    if (enable)
    {
        status = cpssHalSetBrgFdbAuMsgRateLimit(devId, (GT_U32)rateLimit, GT_TRUE);
        if (status != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to set rate limit to  CPU\n");
            return xpsConvertCpssStatusToXPStatus(status);
        }
    }
    else
    {
        status = cpssHalSetBrgFdbAuMsgRateLimit(devId, 0, GT_FALSE);
        if (status != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to disable rate limit to  CPU\n");
            return xpsConvertCpssStatusToXPStatus(status);
        }

    }
    return XP_NO_ERR;
}

extern int cpssHalDumpUtlFdbCount(uint32_t *usedEntries);

XP_STATUS xpsFdbGetEntriesCountDefault(xpsDevice_t devId, uint32_t *usedCount)
{
    GT_STATUS status = GT_OK;
    if ((status = cpssHalDumpUtlFdbCount(usedCount)))
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Failed to get count\n");
        return xpsConvertCpssStatusToXPStatus(status);

    }
    return XP_NO_ERR;
}


XP_STATUS xpsFdbInitApiDefault(xpsDevice_t devId)
{
    xpsFdbAddDevice            = xpsFdbAddDeviceDefault;
    xpsFdbRemoveDevice         = xpsFdbRemoveDeviceDefault;
    xpsFdbRegisterLearnHandler = xpsFdbRegisterLearnHandlerDefault;
    xpsFdbUnregisterLearnHandler = xpsFdbUnRegisterLearnHandlerDefault;
    xpsFdbRegisterAgingHandler = xpsFdbRegisterAgingHandlerDefault;
    xpsFdbFlushEntry           = xpsFdbFlushEntryDefault;
    xpsFdbFlushEntryByIntf     = xpsFdbFlushEntryByIntfDefault;
    xpsFdbFlushEntryByVlan     = xpsFdbFlushEntryByVlanDefault;
    xpsFdbFlushEntryByIntfVlan = xpsFdbFlushEntryByIntfVlanDefault;
    xpsFdbFindEntry            = xpsFdbFindEntryDefault;
    xpsFdbAddEntry             = xpsFdbAddEntryDefault;
    xpsFdbAddHwEntry           = xpsFdbAddHwEntryDefault;
    xpsFdbWriteEntry           = xpsFdbWriteEntryDefault;
    xpsFdbRemoveEntry          = xpsFdbRemoveEntryDefault;
    xpsFdbGetEntryByIndex      = xpsFdbGetEntryByIndexDefault;
    xpsFdbGetEntry             = xpsFdbGetEntryDefault;
    xpsFdbSetAttribute         = xpsFdbSetAttributeDefault;
    xpsFdbGetAttribute         = xpsFdbGetAttributeDefault;
    xpsFdbGetEntriesCount      = xpsFdbGetEntriesCountDefault;

    return XP_NO_ERR;
}


XP_STATUS xpsFdbInitApi(xpsDevice_t devId)
{

    GT_U8 cpssDevId = devId;

    if (GT_OK == cpssHalFdbManagerIsSupported(cpssDevId))
    {
        return xpsFdbInitApiFdbMgr(devId);
    }
    else
    {
        return xpsFdbInitApiDefault(devId);
    }


    return XP_NO_ERR;
}
/* Using this API to enable/disbale of processing learn events.
   Note that entry is installed in cpss but discarded in SAI.*/
XP_STATUS xpsFdbSetResetLearnHdl(uint32_t isSet)
{
    GT_U8 cpssDevId = 0;
    if (GT_OK == cpssHalFdbManagerIsSupported(cpssDevId))
    {
        return xpsFdbSetResetLearnHandlerFdbMgr(isSet);
    }
    else
    {
        return xpsFdbSetResetLearnHandlerDefault(isSet);
    }
    return XP_NO_ERR;
}

#ifdef __cplusplus
}
#endif
