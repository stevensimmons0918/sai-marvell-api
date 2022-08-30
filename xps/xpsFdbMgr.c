// xpsFdbMgr.c

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
********************************************************************************/

#include "xpsFdb.h"
#include "xpsFdbMgr.h"
#include "xpsCommon.h"
#include "cpssHalUtil.h"
#include "cpssHalFdb.h"
#include "cpssHalFdbMgr.h"
#include "cpssDxChBrgFdbManagerTypes.h"
#include "cpssDxChBrgFdbManager.h"
#include "cpssDriverPpHw.h"
#include "xpsInternal.h"
#include "cpssHalDevice.h"
#include "xpsLock.h"
#include "xpsScope.h"
#include "xpsVxlan.h"

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

extern int GetSharedProfileIdx();

static GT_BOOL fdbManagerId_created = GT_FALSE;
static GT_U32 fdbManagerId =
    7;/* (single) global FDB MANAGER ID (unique number between 0..31) */
static GT_U32 fdbManagerNumDevices = 0;

static xpFdbLearnHandler  learnHandler = NULL;
static xpFdbLearnHandler  prevLearnHandler = NULL;
static GT_U32 learningTid;  /* The Learning task id  */
static GT_U32 *learningTidPtr = NULL;
static int FdbMgrTaskParams[1];

/* priorities of tasks created in this file */
#define LEARNING_TASK_PRIO              100
#define AGING_TASK_PRIO                 100
#define TEST_ADD_FDB_ENTRIES_TASK_PRIO  100 /* task for TESTING purposes only */

uint32_t xpsFdbGetManagerId(void)
{
    return fdbManagerId;
}

#define NUM_ENTRIES_PER_SCAN         256
#define NUM_ENTRIES_PER_AGING_SCAN   256
#define NUM_ENTRIES_PER_SCAN_PER_FLUSH  256

/* Roundup Number */
#define ROUNDUP_DIV_MAC(_number , _divider)             \
        (((_number) + ((_divider)-1)) / (_divider))

#define START_TIME                                           \
    (void)cpssOsTimeRT(&startSeconds, &startNanoSeconds)

#define HOW_MUCH_TIME                                                               \
    (void)cpssOsTimeRT(&stopSeconds, &stopNanoSeconds);                             \
                                                                                    \
    elapsedSeconds = stopSeconds-startSeconds;                                      \
    if(stopNanoSeconds >= startNanoSeconds)                                          \
    {                                                                               \
        elapsedNanoSeconds = stopNanoSeconds-startNanoSeconds;                      \
    }                                                                               \
    else                                                                            \
    {                                                                               \
        elapsedNanoSeconds = (1000000000 - startNanoSeconds) + stopNanoSeconds;     \
        elapsedSeconds--;                                                           \
    }

#define SLEEP_GRANULARITY           250
static GT_U32 agingTid;  /* The Aging task id  */
static GT_U32 *agingTidPtr = NULL;
static xpFdbAgingHandler  ageHandler = NULL;
extern uint32_t
gAgingTimer;   /*default aging timer is zero, which means age disable*/
static CPSS_DXCH_BRG_FDB_MANAGER_AGING_SCAN_PARAMS_STC agingParams;

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

static bool fdbMgrLearningAuMsgHandle(uint8_t devId);
static GT_STATUS fdbMgrLearningEventHandler
(
    GT_U8                   devId,
    CPSS_UNI_EV_CAUSE_ENT   uniEv,
    GT_U32                  evExtData
);

static bool fdbMgrLearningAuMsgHandle(uint8_t devId)
{
    XP_STATUS                                               xpStatus = XP_NO_ERR;
    GT_STATUS                                               status = GT_OK;
    GT_STATUS                                               status1 = GT_OK;
    GT_U32                                                  auIndex;
    CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_UPDATE_EVENT_STC
    entriesLearningArray[NUM_ENTRIES_PER_SCAN];
    GT_U32                                                  entriesLearningNum;
    CPSS_DXCH_BRG_FDB_MANAGER_LEARNING_SCAN_PARAMS_STC      learningParams;
    xpFdbEntryAction_e                                      fdbAction =
        XP_FDB_ENTRY_NO_ACTION;
    xpsFdbEntry_t                                           fdbEntry;
    XPS_FDB_ENTRY_TYPE_ENT                                  fdbEntryType;
    CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_STC                     cpssMacEntry;
    CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_ENT                entryType;

    learningParams.addNewMacUcEntries = GT_TRUE;
    learningParams.addWithRehashEnable = GT_TRUE;
    learningParams.updateMovedMacUcEntries = GT_TRUE;

    while (1)
    {
        /*************************************/
        /* get the AU message buffer from HW */
        /*************************************/

        status1 = cpssDxChBrgFdbManagerLearningScan(fdbManagerId, &learningParams,
                                                    entriesLearningArray, &entriesLearningNum);
        if (status1 != GT_NO_MORE && status1 != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Getting entries from HW failed with cpss error code %d", status1);
            return (GT_FALSE);
        }

        for (auIndex = 0; auIndex < entriesLearningNum; auIndex++)
        {
            switch (entriesLearningArray[auIndex].updateType)
            {
                case CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_UPDATE_EVENT_TYPE_NEW_E:
                    fdbAction = XP_FDB_ENTRY_ADD;
                    break;
                case CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_UPDATE_EVENT_TYPE_MOVED_E:
                    fdbAction = XP_FDB_ENTRY_MODIFY;
                    break;
                case CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_UPDATE_EVENT_TYPE_NO_SPACE_E:
                case CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_UPDATE_EVENT_TYPE_AGED_OUT_E:
                case CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_UPDATE_EVENT_TYPE_AGED_DELETED_E:
                case CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_UPDATE_EVENT_TYPE_DELETED_E:
                case CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_UPDATE_EVENT_TYPE_TRANSPLANTED_E:
                    fdbAction = XP_FDB_ENTRY_NO_ACTION;
                    break;
                default:
                    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Wrong update type recieved");
                    continue;
            }

            cpssMacEntry = entriesLearningArray[auIndex].entry;
            entryType = entriesLearningArray[auIndex].entry.fdbEntryType;
            if (entryType != CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_MAC_ADDR_E)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Wrong mac type recived %d\n",
                      entryType);
                return GT_FALSE;//XP_ERR_INVALID_DATA;
            }

            fdbEntryType = XPS_FDB_ENTRY_TYPE_MAC_ADDR_E;
            status = cpssHalFdbMgrConvertCpssToGenericMacEntry(devId, &cpssMacEntry,
                                                               fdbEntryType, (void*)&fdbEntry);
            if (status != GT_OK)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "Failed to convert cpss to xps mac entry type , error code %d", status);
                return GT_FALSE;//xpsConvertCpssStatusToXPStatus(status);
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
        }

        if (status1 == GT_NO_MORE)
        {
            return (GT_TRUE);
        }
    }
    return (GT_TRUE);
}

static GT_U32   allow_TRACE_AGING_TASK = 0;
/* debug function to allow trace of the aging task */
GT_STATUS fdbMgr_allow_TRACE_AGING_TASK(IN GT_U32   allowTrace)
{
    allow_TRACE_AGING_TASK = allowTrace;
    return GT_OK;
}
#define TRACE_AGING_TASK(...)  if(allow_TRACE_AGING_TASK) cpssOsPrintf(__VA_ARGS__)

static GT_U32   allow_TRACE_AGING_TASK_level1 = 0;
/* debug function to allow trace of the aging task */
GT_STATUS fdbMgr_allow_TRACE_AGING_TASK_level1(IN GT_U32   allowTrace)
{
    allow_TRACE_AGING_TASK_level1 = allowTrace;
    return GT_OK;
}
#define LEVEL1_TRACE_AGING_TASK(...)  if(allow_TRACE_AGING_TASK_level1) cpssOsPrintf(__VA_ARGS__)

static GT_U32   allow_TRACE_AGING_TASK_level2 = 0;
/* debug function to allow trace of the aging task */
GT_STATUS fdbMgr_allow_TRACE_AGING_TASK_level2(IN GT_U32   allowTrace)
{
    allow_TRACE_AGING_TASK_level2 = allowTrace;
    return GT_OK;
}
#define LEVEL2_TRACE_AGING_TASK(...)  if(allow_TRACE_AGING_TASK_level2) cpssOsPrintf(__VA_ARGS__)


/* this is the main logic of the aging task when working with the cpss FDB manager :

   1. ask the manager to act on current aging bin
   2. get the aged out (deleted) entries
   for each one --> notify the upper layer (application)

NOTE: this function do no 'sleep'
 */
static void     fdbMgrMainAgingTaskLogic(void)
{
    static CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_UPDATE_EVENT_STC
    entriesAgedoutArray[NUM_ENTRIES_PER_AGING_SCAN];
    static CPSS_DXCH_BRG_FDB_MANAGER_COUNTERS_STC fdbCounters;
    xpsFdbEntry_t xpsFdbEntry;
    xpDevice_t devId =0;
    GT_U32 entriesAgedoutNum=0;
    GT_U32 entryIndex;
    GT_U32 usedEntries;
    static  GT_U32  lastTimeNumEntries = 0;

    if (allow_TRACE_AGING_TASK)
    {
        cpssDxChBrgFdbManagerCountersGet(fdbManagerId, &fdbCounters);

        TRACE_AGING_TASK("--before aging: usedEntries[%d] \n", fdbCounters.usedEntries);
    }

    /* let the CPSS fdb manager to ageout (delete due to aging) up to 256 entries (NUM_ENTRIES_PER_AGING_SCAN) */
    (void)cpssDxChBrgFdbManagerAgingScan(fdbManagerId, &agingParams,
                                         &entriesAgedoutArray[0],
                                         &entriesAgedoutNum);

    if (allow_TRACE_AGING_TASK_level1 || allow_TRACE_AGING_TASK)
    {
        cpssDxChBrgFdbManagerCountersGet(fdbManagerId, &fdbCounters);
        usedEntries = fdbCounters.usedEntries;
    }
    else
    {
        usedEntries = 0;
    }

    if (allow_TRACE_AGING_TASK_level1) /* special LEVEL 1 trace , to give minimal printings */
    {
        if (usedEntries == 0 && lastTimeNumEntries != 0)
        {
            /* ability to measure time , when the aging task finished to clear the FDB
               assumption is that the test stoped adding new FDB entries , and we only wait for the deletion of all. */
            LEVEL1_TRACE_AGING_TASK("FDB is EMPTY \n");
        }

        lastTimeNumEntries = usedEntries;
        if (entriesAgedoutNum)
        {
            LEVEL1_TRACE_AGING_TASK("--: deleted[%d] \n", entriesAgedoutNum);
        }
    }

    if (allow_TRACE_AGING_TASK)
    {
        TRACE_AGING_TASK("--after aging: usedEntries[%d] \n", usedEntries);

        if (entriesAgedoutNum)
        {
            TRACE_AGING_TASK("--: entriesAgedoutNum[%d] \n", entriesAgedoutNum);
        }
        else
        {
            TRACE_AGING_TASK("--: empty \n");
        }
    }

    /*scan whole FDB for valid entries and validate aging of entries*/
    for (entryIndex = 0; entryIndex < entriesAgedoutNum; entryIndex++)
    {
        if (entriesAgedoutArray[entryIndex].updateType ==
            CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_UPDATE_EVENT_TYPE_AGED_DELETED_E &&
            entriesAgedoutArray[entryIndex].entry.fdbEntryType ==
            CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_MAC_ADDR_E)
        {
            /* convert FDB manager entry to xpsFdbEntry */
            cpssHalFdbMgrConvertCpssToGenericMacEntry(devId,
                                                      &entriesAgedoutArray[entryIndex].entry,
                                                      XPS_FDB_ENTRY_TYPE_MAC_ADDR_E, &xpsFdbEntry);

            /*convertFdbMgrMacEntryTo_xpsFdbEntry_t(&entriesAgedoutArray[entryIndex].entry.format.fdbEntryMacAddrFormat,&xpsFdbEntry);*/
            LEVEL2_TRACE_AGING_TASK("Aged out : xpsFdbEntry:  mac[%2.2x:%2.2x:%2.2x:%2.2x:%2.2x:%2.2x] vlanId[%d] pktCmd[%d] isControl[%d] isRouter[%d] isStatic[%d] intfId[0x%x] serviceInstId[%d]\n",
                                    xpsFdbEntry.macAddr[0],
                                    xpsFdbEntry.macAddr[1],
                                    xpsFdbEntry.macAddr[2],
                                    xpsFdbEntry.macAddr[3],
                                    xpsFdbEntry.macAddr[4],
                                    xpsFdbEntry.macAddr[5],
                                    xpsFdbEntry.vlanId,
                                    xpsFdbEntry.pktCmd,
                                    xpsFdbEntry.isControl,
                                    xpsFdbEntry.isRouter,
                                    xpsFdbEntry.isStatic,
                                    xpsFdbEntry.intfId,
                                    xpsFdbEntry.serviceInstId);

            if (ageHandler)
            {
                LEVEL2_TRACE_AGING_TASK("calling 'ageHandler' \n");
                ageHandler(devId, xpsFdbEntry);
            }
        }
        else
        {
            LEVEL2_TRACE_AGING_TASK("--: unknown");
        }
    }

    if (entriesAgedoutNum)
    {
        LEVEL2_TRACE_AGING_TASK("\n");
    }
}


/* Aging Task */
static unsigned __TASKCONV fdbMgrAgingTaskFunc(GT_VOID *param)
{
    uint32_t startSeconds, stopSeconds,
             startNanoSeconds, stopNanoSeconds,
             elapsedSeconds, elapsedNanoSeconds; /* time of init */
    uint32_t elapsedTimeInMilli, mili2Sleep; /* time in milliseconds */
    GT_U32 totalAgeBinAllocated         = 0;
    GT_U32 agingTimePerAgeBin;/* the time in milliseconds between processing of 2 consecutive aging bins */
    GT_U32 totalTimeSleep;/* total time already passed in current aging bin */
    GT_U32  ii;
    GT_U32 compensationTime;/* time that we exceeded in previous loop(s) and need compensation for it in future loop(s) */
    GT_U32 prev_totalAgeBinAllocated;
    uint32_t                                         maxFdbEntries = 0;
    xpsDevice_t devId = *(xpsDevice_t*)(param);
    XP_DEV_TYPE_T   devType;
    cpssHalGetDeviceType(devId, &devType);

    if (IS_DEVICE_FALCON(devType))
    {
        int sharedProdId = GetSharedProfileIdx();

        if (sharedProdId == CPSS_HAL_MID_L3_MID_L2_NO_EM ||
            sharedProdId == CPSS_HAL_MID_L3_MID_L2_MIN_EM)
        {
            maxFdbEntries = _128K;
        }
        else if (sharedProdId == CPSS_HAL_MAX_L3_MIN_L2_NO_EM)
        {
            maxFdbEntries = _32K;
        }
        else if (sharedProdId == CPSS_HAL_LOW_MAX_L3_MID_LOW_L2_NO_EM)
        {
            maxFdbEntries = _64K;
        }
        else
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Invalid ProfileId Passed : %d\n", sharedProdId);
            return XP_ERR_INVALID_DATA;
        }
    }
    else if (IS_DEVICE_AC5X(devType))
    {
        maxFdbEntries = _32K;
    }


    totalAgeBinAllocated         = ROUNDUP_DIV_MAC(maxFdbEntries,
                                                   NUM_ENTRIES_PER_AGING_SCAN);
    cpssOsMemSet(&agingParams, 0, sizeof(agingParams));
    /* allow aging on MAC entries */
    agingParams.checkAgeMacUcEntries          =
        GT_TRUE;/* needed for 'port','trunk'*/
    agingParams.checkAgeMacMcEntries          =
        GT_TRUE;/* needed for 'vidx','vlan' */
    /* allow delete (due to aging) on MAC entries */
    agingParams.deleteAgeoutMacUcEportEntries = GT_TRUE;/* needed for 'port' */
    agingParams.deleteAgeoutMacUcTrunkEntries = GT_TRUE;/* needed for 'trunk' */
    agingParams.deleteAgeoutMacMcEntries      =
        GT_TRUE;/* relevant if not set static */

    compensationTime = 0;
    prev_totalAgeBinAllocated = 0;

    while (1)
    {
        while (gAgingTimer == 0)
        {
            /* wait for the aging to be activated */
            cpssOsTimerWkAfter(SLEEP_GRANULARITY);
        }

        agingTimePerAgeBin = ROUNDUP_DIV_MAC(gAgingTimer/*in seconds*/ * 1000,
                                             totalAgeBinAllocated);
        if (prev_totalAgeBinAllocated != agingTimePerAgeBin)
        {
            LEVEL2_TRACE_AGING_TASK("gAgingTimer changed and is now [%d] seconds \n",
                                    gAgingTimer);
            /* we are starting new timeout , so the old 'compensationTime' is not relevant any more */
            prev_totalAgeBinAllocated = agingTimePerAgeBin;
            compensationTime = 0;
        }
        /*
           TRACE_AGING_TASK("agingTimePerAgeBin[%d],gAgingTimer[%d],totalAgeBinAllocated[%d]",
           agingTimePerAgeBin,gAgingTimer,totalAgeBinAllocated);
         */
        START_TIME;

        /* do main logic of the aging of current aging bin */
        fdbMgrMainAgingTaskLogic();

        HOW_MUCH_TIME; /* calculate : elapsedSeconds , elapsedNanoSeconds */

        elapsedTimeInMilli = (elapsedSeconds * 1000) + elapsedNanoSeconds / 1000000;

        /* convert sleep time to milliseconds */
        if (elapsedTimeInMilli < agingTimePerAgeBin)
        {
            /* need to sleep till the end of the 'agingTimePerAgeBin' */
            mili2Sleep = agingTimePerAgeBin - elapsedTimeInMilli;

            if (compensationTime > mili2Sleep)
            {
                compensationTime -= mili2Sleep;
                mili2Sleep = 0;
            }
            else
            {
                mili2Sleep -= compensationTime;
                compensationTime = 0;
            }
        }
        else
        {
            /* have no time to sleep , just let other tasks a context switch */
            mili2Sleep = 0;

            compensationTime += (elapsedTimeInMilli - agingTimePerAgeBin);
            /* the processing of the entries cause to over time , than we expected to be */
            LEVEL1_TRACE_AGING_TASK(" overtime [%d] milli ",
                                    elapsedTimeInMilli - agingTimePerAgeBin);
        }

        if (compensationTime)
        {
            LEVEL1_TRACE_AGING_TASK(" compensationTime [%d] ", compensationTime);
        }

        totalTimeSleep = elapsedTimeInMilli;

        if (mili2Sleep > SLEEP_GRANULARITY)
        {
            /* don't let a large sleep to be used , because it will take a long
               time to recognize change of 'new aging timeoput' */
            for (ii = 0 ; ii < mili2Sleep / SLEEP_GRANULARITY ; ii++)
            {
                cpssOsTimerWkAfter(SLEEP_GRANULARITY);
                LEVEL1_TRACE_AGING_TASK(" sg ");/* sg - means Sleep Granularity */

                totalTimeSleep += SLEEP_GRANULARITY;

                /* check to see if 'agingTimePerAgeBin' was updated to lower value */
                agingTimePerAgeBin = ROUNDUP_DIV_MAC(gAgingTimer/*in seconds*/ * 1000,
                                                     totalAgeBinAllocated);

                if (totalTimeSleep > agingTimePerAgeBin)
                {
                    /* the 'agingTimePerAgeBin' was updated to lower value */
                    TRACE_AGING_TASK("the 'agingTimePerAgeBin' was updated to lower value \n");
                    mili2Sleep = 0;
                    break;
                }
            }
        }

        LEVEL1_TRACE_AGING_TASK(" s[%d] ", mili2Sleep % SLEEP_GRANULARITY);
        /* sleep the fraction of the time that left outside the loop */
        cpssOsTimerWkAfter(mili2Sleep % SLEEP_GRANULARITY);
    } /* while(1) */

    return (GT_TRUE); /* unreachable code ! */
}



/**
 * @internal fdbMgrLearningEventHandler function
 * @endinternal
 *
 * @brief   This routine handles events.
 *
 * @param[in] devNum                   - the device number.
 * @param[in] uniEv                    - Unified event number
 * @param[in] evExtData                - Unified event additional information
 *
 * @retval GT_OK                    - on success,
 * @retval GT_FAIL                  - otherwise.
 */
static GT_STATUS fdbMgrLearningEventHandler
(
    GT_U8                   devId,
    CPSS_UNI_EV_CAUSE_ENT   uniEv,
    GT_U32                  evExtData
)
{
    GT_STATUS   status = GT_OK;
    bool handleTrue = GT_FALSE;
    switch (uniEv)
    {
        case CPSS_PP_MAC_MESSAGE_TO_CPU_READY_E:
        case CPSS_PP_EB_AUQ_PENDING_E:
            handleTrue = fdbMgrLearningAuMsgHandle(devId);
            if (handleTrue != GT_TRUE)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Msg handle failed");
            }
            break;
        default:
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Event type is not matched");
            break;
    }

    return status;
}




/*******************************************************************************
 * fdbMgrLearningEventsHndlr
 * DESCRIPTION:
 *       This routine is the event handler for Event-Request-Driven mode
 *       (polling mode).
 * INPUTS:
 *       param - The process data structure.
 * OUTPUTS:
 *       None.
 * RETURNS: GT_OK or GT_FALSE
 * COMMENTS:
 *       None.
 *******************************************************************************/
static unsigned __TASKCONV fdbMgrLearningEventsHndlr(GT_VOID * param)
{
    xpDevice_t          devId =0;
    GT_U8               devNum = 0;
    GT_STATUS           status =
        GT_OK;                             /* return code         */
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
        status = cpssEventSelect(evHndl, NULL, evBitmapArr,
                                 (GT_U32)CPSS_UNI_EV_BITMAP_SIZE_CNS);
        if (status != GT_OK)
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

                if ((status=cpssEventRecv(evHndl, uniEv, &evExtData, &devNum)) == GT_OK)
                {
                    status = fdbMgrLearningEventHandler(devId, uniEv, evExtData);

                }
            }
        }
    }
    return (GT_TRUE);
}


XP_STATUS xpsFdbAddDeviceFdbMgr(xpsDevice_t devId, xpsInitType_t initType)
{
    XPS_FUNC_ENTRY_LOG();

    /*TODO -- AddDefaults api*/
    XP_STATUS                                        result = XP_NO_ERR;
    GT_U8                                            devNum;/* cpssDevNum */
    GT_STATUS                                        status = GT_OK;
    CPSS_DXCH_BRG_FDB_MANAGER_CAPACITY_STC           capacityInfo;
    CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_ATTRIBUTES_STC   entryAttrInfo;
    CPSS_DXCH_BRG_FDB_MANAGER_LEARNING_STC           learningInfo;
    CPSS_DXCH_BRG_FDB_MANAGER_LOOKUP_STC             lookupInfo;
    CPSS_DXCH_BRG_FDB_MANAGER_AGING_STC              agingInfo;
    uint32_t                                         maxFdbEntries = 0;
    uint32_t                                         numHashes = 0;
    XP_DEV_TYPE_T   devType;
    cpssHalGetDeviceType(devId, &devType);


    memset(&entryAttrInfo, 0,
           sizeof(CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_ATTRIBUTES_STC));
    memset(&capacityInfo, 0, sizeof(CPSS_DXCH_BRG_FDB_MANAGER_CAPACITY_STC));
    memset(&learningInfo, 0, sizeof(CPSS_DXCH_BRG_FDB_MANAGER_LEARNING_STC));
    memset(&lookupInfo, 0, sizeof(CPSS_DXCH_BRG_FDB_MANAGER_LOOKUP_STC));
    memset(&agingInfo, 0, sizeof(CPSS_DXCH_BRG_FDB_MANAGER_AGING_STC));

    if (IS_DEVICE_FALCON(devType))
    {
        switch (GetSharedProfileIdx())
        {
            case CPSS_HAL_MID_L3_MID_L2_NO_EM:
            case CPSS_HAL_MID_L3_MID_L2_MIN_EM:
                {
                    maxFdbEntries = _128K;
                    numHashes     = 16;
                }
                break;
            case CPSS_HAL_MAX_L3_MIN_L2_NO_EM:
                {
                    maxFdbEntries = _32K;
                    numHashes     = 4;
                }
                break;
            case CPSS_HAL_LOW_MAX_L3_MID_LOW_L2_NO_EM:
                {
                    maxFdbEntries = _64K;
                    numHashes     = 8;
                }
                break;
            default:
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "Invalid ProfileId Passed : %d\n", GetSharedProfileIdx());
                return XP_ERR_INVALID_DATA;
        }
    }
    else if (IS_DEVICE_AC5X(devType))
    {
        maxFdbEntries = _32K;
        numHashes     = 16;
    }
    capacityInfo.hwCapacity.numOfHwIndexes   = maxFdbEntries;
    capacityInfo.hwCapacity.numOfHashes      = numHashes;
    capacityInfo.maxEntriesPerLearningScan   = NUM_ENTRIES_PER_SCAN;
    capacityInfo.maxEntriesPerAgingScan      = NUM_ENTRIES_PER_AGING_SCAN;
    capacityInfo.maxTotalEntries             = maxFdbEntries;
    capacityInfo.maxEntriesPerDeleteScan     = NUM_ENTRIES_PER_SCAN_PER_FLUSH;
    capacityInfo.maxEntriesPerTransplantScan = NUM_ENTRIES_PER_SCAN;

    entryAttrInfo.macEntryMuxingMode         =
        CPSS_DXCH_BRG_FDB_MANAGER_MAC_ENTRY_MUXING_MODE_SRC_ID_E;
    entryAttrInfo.ipmcEntryMuxingMode        =
        CPSS_DXCH_BRG_FDB_MANAGER_IPMC_ENTRY_MUXING_MODE_SRC_ID_E;
    entryAttrInfo.saDropCommand              = CPSS_PACKET_CMD_DROP_SOFT_E;
    entryAttrInfo.daDropCommand              = CPSS_PACKET_CMD_DROP_HARD_E;
    entryAttrInfo.ipNhPacketcommand          = CPSS_PACKET_CMD_ROUTE_E;
    entryAttrInfo.shadowType                 =
        CPSS_DXCH_BRG_FDB_MANAGER_SHADOW_TYPE_SIP6_E;

    learningInfo.macNoSpaceUpdatesEnable     = GT_FALSE;
    learningInfo.macRoutedLearningEnable     = GT_FALSE;
    learningInfo.macVlanLookupMode           =
        CPSS_DXCH_BRG_FDB_MANAGER_MAC_VLAN_LOOKUP_MODE_MAC_AND_FID_E;

    lookupInfo.crcHashUpperBitsMode          =
        CPSS_DXCH_BRG_FDB_MANAGER_CRC_HASH_UPPER_BITS_MODE_ALL_ZERO_E;
    lookupInfo.ipv4PrefixLength              = 32;
    lookupInfo.ipv6PrefixLength              = 128;

    agingInfo.destinationUcRefreshEnable     = GT_FALSE;
    agingInfo.destinationMcRefreshEnable     = GT_FALSE;
    agingInfo.ipUcRefreshEnable              = GT_FALSE;

    result = xpsFdbMacSecurity(devId);
    if (result != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Setting MAC security failed\n");
        return result;
    }


    if (fdbManagerId_created == GT_FALSE)
    {
        status = cpssDxChBrgFdbManagerCreate(fdbManagerId, &capacityInfo,
                                             &entryAttrInfo, &learningInfo, &lookupInfo, &agingInfo);
        if (status != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Creation of FDB Manager failed %d ", status);
            return xpsConvertCpssStatusToXPStatus(status);
        }
        fdbManagerId_created = GT_TRUE;
    }

    XPS_DEVICES_PER_SWITCH_ITER(devId, devNum)
    {
        status = cpssDxChBrgFdbManagerDevListAdd(fdbManagerId, &devNum, 1);
        if (status != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Adding new device to FDB Manager failed");
            return xpsConvertCpssStatusToXPStatus(status);
        }

        fdbManagerNumDevices++;
    }

    /*event handle task creations for aging and learning */
    CPSS_UNI_EV_CAUSE_ENT   evHndlrCauseArr[] =
    {
        CPSS_PP_EB_AUQ_PENDING_E
        /* FDB Manager support AU queue only.
           Disable AU FIFO related interrupt to avoid redundant actions.
        CPSS_PP_MAC_MESSAGE_TO_CPU_READY_E */
    };

    GT_U32 evHndlrCauseArrSize = sizeof(evHndlrCauseArr)/sizeof(evHndlrCauseArr[0]);
    GT_U32 ii;
    static EV_HNDLR_PARAM taskParamArr;
    FdbMgrTaskParams[0] = devId;
    void   *agingParams=&FdbMgrTaskParams;

    taskParamArr.hndlrIndex = 0;
    taskParamArr.evHndl = (GT_UINTPTR)(&fdbMgrLearningEventsHndlr);

    /* call CPSS to bind the events under single handler */
    status = cpssEventBind(evHndlrCauseArr,
                           evHndlrCauseArrSize,
                           &taskParamArr.evHndl);
    switch (status)
    {
        case GT_FAIL:
        case GT_BAD_PTR:
        case GT_OUT_OF_CPU_MEM:
        case GT_FULL:
        case GT_ALREADY_EXIST:
            break;
    }

    if (GT_OK != status)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "cpss Event bind failed");
        return xpsConvertCpssStatusToXPStatus(status);
    }


    for (ii=0; ii<evHndlrCauseArrSize; ii++)
    {
        status = cpssEventDeviceMaskSet(devId, evHndlrCauseArr[ii],
                                        CPSS_EVENT_UNMASK_E);

        if (status != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "cpss Event device mask set failed in fdb add device");
            return xpsConvertCpssStatusToXPStatus(status);
        }
    }

    /*create Learning task*/
    if (learningTidPtr == NULL)
    {
        /* Create Learning Task */
        status = cpssOsTaskCreate("LearningTask",
                                  LEARNING_TASK_PRIO,
                                  _64KB,
                                  fdbMgrLearningEventsHndlr,
                                  &taskParamArr,
                                  &learningTid);
        if (status != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "cpssOsTaskCreate failed to creating Learning Task\n");
            return xpsConvertCpssStatusToXPStatus(status);
        }
    }

    learningTidPtr = &learningTid;

    /*create Aging task*/
    if (agingTidPtr == NULL)
    {
        status = cpssOsTaskCreate("AgingTask",
                                  AGING_TASK_PRIO,
                                  _64KB,
                                  fdbMgrAgingTaskFunc,
                                  agingParams,
                                  &agingTid);
        if (status != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "cpssOsTaskCreate failed to creating AgingTask\n");
            return xpsConvertCpssStatusToXPStatus(status);
        }
    }

    agingTidPtr = &agingTid;


    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsFdbRemoveDeviceFdbMgr(xpsDevice_t devId)
{
    XPS_FUNC_ENTRY_LOG();

    GT_STATUS    status = GT_OK;
    GT_U8        devNum;/* cpssDevNum */

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

    XPS_DEVICES_PER_SWITCH_ITER(devId, devNum)
    {
        status = cpssDxChBrgFdbManagerDevListRemove(fdbManagerId, &devNum, 1);
        if (status != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Removing old device from FDB Manager failed");
            return xpsConvertCpssStatusToXPStatus(status);
        }

        fdbManagerNumDevices--;
    }

    if (fdbManagerNumDevices == 0) /* all the device removed */
    {
        status = cpssDxChBrgFdbManagerDelete(fdbManagerId);
        if (status != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Deletion of FDB Manager failed");
            return xpsConvertCpssStatusToXPStatus(status);
        }

        fdbManagerId_created = GT_FALSE;
    }

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsFdbUnRegisterLearnHandlerFdbMgr(xpsDevice_t devId)
{
    XPS_FUNC_ENTRY_LOG();

    learnHandler = NULL;

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsFdbRegisterLearnHandlerFdbMgr(xpsDevice_t devId,
                                           xpFdbLearnHandler fdbLearnHandler)
{
    XPS_FUNC_ENTRY_LOG();

    learnHandler = fdbLearnHandler;
    prevLearnHandler = learnHandler;

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

/* Using this API to enable/disbale of processing learn events */
XP_STATUS xpsFdbSetResetLearnHandlerFdbMgr(uint32_t isSet)
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

static GT_U32   allow_TRACE_FDB_FLUSH = 0;
/* debug function to allow trace of the test of 'add fdb entries' task */
GT_STATUS fdbMgr_allow_TRACE_FDB_FLUSH(IN GT_U32   allowTrace)
{
    allow_TRACE_FDB_FLUSH = allowTrace;
    return GT_OK;
}
#define TRACE_FDB_FLUSH(...)  if(allow_TRACE_FDB_FLUSH) cpssOsPrintf(__VA_ARGS__)

/**
 * \brief Flushes all the fdb entries of a specific type
 *
 * \param [in] devId Device Id of device.
 * \param [in] flushParams (pointer to) for FDB flush options.
 *
 * \return XP_STATUS
 */
static GT_STATUS xpsFdbMgrFlushCommon(
    int devId,
    xpsFlushParams* flushParamsPtr
)
{
    XP_STATUS   xp_rc;
    GT_STATUS   rc;
    GT_BOOL     scanStart;
    CPSS_DXCH_BRG_FDB_MANAGER_DELETE_SCAN_PARAMS_STC fdbManagerFlushParams;
    GT_BOOL     useVlan = GT_FALSE;
    GT_BOOL     useInterface = GT_FALSE;
    GT_U8       devNum;
    xpsInterfaceType_e xpsInterfaceType;
    GT_U32      entriesDeletedNum;/* number of deleted entries */
    GT_U32      tmp_entriesDeletedNum;/* temp number of deleted entries */
    static CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_UPDATE_EVENT_STC
    entriesDeletedArray[NUM_ENTRIES_PER_SCAN_PER_FLUSH];
    CPSS_DXCH_BRG_FDB_MANAGER_COUNTERS_STC fdbCounters;
    GT_U32      ii, iiMax;
    xpsVlan_t   hwEVlanId = 0;

    cpssOsMemSet(&fdbManagerFlushParams, 0, sizeof(fdbManagerFlushParams));
    fdbManagerFlushParams.deleteMacUcEntries = GT_TRUE;
    fdbManagerFlushParams.deleteMacMcEntries = GT_TRUE;

    xp_rc = xpsInterfaceGetType(flushParamsPtr->intfId, &xpsInterfaceType);
    if (xp_rc != XP_NO_ERR)
    {
        TRACE_FDB_FLUSH("Error : fdb flush : xpsInterfaceGetType failed on intfId[%d] \n",
                        flushParamsPtr->intfId);
        return GT_FAIL;
    }
    if (flushParamsPtr->entryType == XP_FDB_ENTRY_TYPE_STATIC)
    {
        TRACE_FDB_FLUSH("Error : fdb flush not support delete static but not delete dynamic \n");
        /* the FDB manager not supports delete of static without dynamic entries */

        /*return unsupported error type*/
        return GT_BAD_VALUE;
    }

    switch (flushParamsPtr->flushType)
    {
        default:
        case XP_FDB_FLUSH_ALL:
            TRACE_FDB_FLUSH("fdb flush All \n");
            break;
        case XP_FDB_FLUSH_BY_INTF:
            TRACE_FDB_FLUSH("fdb flush by interface \n");
            useInterface = GT_TRUE;
            break;
        case XP_FDB_FLUSH_BY_VLAN:
            TRACE_FDB_FLUSH("fdb flush by vlan \n");
            useVlan      = GT_TRUE;
            break;
        case XP_FDB_FLUSH_BY_INTF_VLAN:
            TRACE_FDB_FLUSH("Flush by interface and vlan \n");
            useVlan      = GT_TRUE;
            useInterface = GT_TRUE;
            break;
    }

    if (useVlan == GT_TRUE)
    {
        fdbManagerFlushParams.fid       = flushParamsPtr->vlanId;
        if (xpsInterfaceType == XPS_TUNNEL_VXLAN)
        {
            xp_rc = xpsVxlanGetTunnelHwEVlanId(devId, flushParamsPtr->vlanId,
                                               &hwEVlanId);
            if (xp_rc != XP_NO_ERR)
            {
                TRACE_FDB_FLUSH("Error : fdb flush : xpsVxlanGetTunnelHwEVlanId failed \n");
                return GT_FAIL;
            }
            fdbManagerFlushParams.fid = hwEVlanId;
        }
        fdbManagerFlushParams.fidMask   = 0xFFFFFFFF;
        TRACE_FDB_FLUSH("fdb flush in fid [%d] \n", fdbManagerFlushParams.fid);
    }

    if (useInterface == GT_TRUE)
    {
        fdbManagerFlushParams.ePortTrunkNum = flushParamsPtr->intfId;

        if (xpsInterfaceType == XPS_PORT)
        {
            devNum = xpsGlobalIdToDevId(devId, flushParamsPtr->intfId);
            rc = cpssDxChCfgHwDevNumGet(devNum, &fdbManagerFlushParams.hwDevNum);
            if (rc != GT_OK)
            {
                TRACE_FDB_FLUSH("Error : fdb flush : cpssDxChCfgHwDevNumGet failed on devNum[%d] \n",
                                devNum);
                return rc;
            }
            fdbManagerFlushParams.hwDevNumMask = 0xFFFFFFFF;

            fdbManagerFlushParams.ePortTrunkNum = xpsGlobalPortToPortnum(devId,
                                                                         flushParamsPtr->intfId);
            fdbManagerFlushParams.isTrunk   = GT_FALSE;
            TRACE_FDB_FLUSH("fdb flush [hwDev %d, portNum %d] \n",
                            fdbManagerFlushParams.hwDevNum, fdbManagerFlushParams.ePortTrunkNum);
        }
        else if (xpsInterfaceType == XPS_LAG)
        {
            fdbManagerFlushParams.ePortTrunkNum = xpsUtilXpstoCpssInterfaceConvert(
                                                      flushParamsPtr->intfId, XPS_LAG);
            fdbManagerFlushParams.isTrunk   = GT_TRUE;
            TRACE_FDB_FLUSH("fdb flush [trunkId %d] \n",
                            fdbManagerFlushParams.ePortTrunkNum);
        }
        else if (xpsInterfaceType == XPS_TUNNEL_VXLAN)
        {
            rc = cpssDxChCfgHwDevNumGet(devId, &fdbManagerFlushParams.hwDevNum);
            if (rc != GT_OK)
            {
                TRACE_FDB_FLUSH("Error : fdb flush : cpssDxChCfgHwDevNumGet failed on devNum[%d] \n",
                                devId);
                return rc;
            }
            xp_rc = xpsVxlanGetTunnelEPort(devId, flushParamsPtr->intfId,
                                           &fdbManagerFlushParams.ePortTrunkNum);
            if (xp_rc != XP_NO_ERR)
            {
                TRACE_FDB_FLUSH("Error : fdb flush : xpsVxlanGetTunnelEPort failed \n");
                return GT_FAIL;
            }
            fdbManagerFlushParams.hwDevNumMask = 0xFFFFFFFF;
            fdbManagerFlushParams.isTrunk   = GT_FALSE;
            TRACE_FDB_FLUSH("fdb flush [tunnel ePort %d] \n",
                            fdbManagerFlushParams.ePortTrunkNum);
        }
        else
        {
            TRACE_FDB_FLUSH("Error : fdb flush : unknown 'xps interface type' [%d] \n",
                            xpsInterfaceType);
            /*return unsupported error type*/
            return GT_BAD_VALUE;
        }

        fdbManagerFlushParams.ePortTrunkNumMask = 0xFFFFFFFF;
        fdbManagerFlushParams.isTrunkMask       = GT_TRUE;
    }


    rc = cpssDxChBrgFdbManagerCountersGet(fdbManagerId, &fdbCounters);
    if (rc != GT_OK)
    {
        TRACE_FDB_FLUSH("Error : fdb flush : cpssDxChBrgFdbManagerCountersGet failed \n");
        return rc;
    }

    TRACE_FDB_FLUSH("\n fdb flush : before operation [%d] valid entries in fdb \n",
                    fdbCounters.usedEntries);
    /* the number of iterations that need to call cpssDxChBrgFdbManagerDeleteScan()
       in order to go over all the entries in the FDB that 'currently' known

    NOTE: this logic will also prevent potential endless loop if waiting for
    'GT_NO_MORE' as the learning task may add new entries all the time.
     */
    iiMax = ROUNDUP_DIV_MAC(fdbCounters.usedEntries,
                            NUM_ENTRIES_PER_SCAN_PER_FLUSH);

    entriesDeletedNum = 0;
    scanStart = GT_TRUE;/* indication to start from the 'start' */
    for (ii = 0 ; ii < iiMax; ii++)
    {
        rc = cpssDxChBrgFdbManagerDeleteScan(fdbManagerId, scanStart,
                                             &fdbManagerFlushParams,
                                             &entriesDeletedArray[0], /*not need to know about the deleted entries - but due to CPP bug need to supply this array to get tmp_entriesDeletedNum */
                                             &tmp_entriesDeletedNum/*not need to know about the deleted entries - needed for debug purposes and due to cpss bug that not return GT_NO_MORE*/);
        if (rc != GT_OK && rc != GT_NO_MORE)
        {
            TRACE_FDB_FLUSH("Error : fdb flush : cpssDxChBrgFdbManagerDeleteScan failed !!! \n");
            return rc;
        }

        entriesDeletedNum += tmp_entriesDeletedNum;
        scanStart = GT_FALSE;/* indication to continue from the last point */

        if (0x1f == (ii & 0x1f))
        {
            /* every 32*256 = 8K entries ... give 'keep alive' indication*/
            TRACE_FDB_FLUSH(".");
        }
    }

    TRACE_FDB_FLUSH("\n fdb flush : deleted total of [%d] \n", entriesDeletedNum);

    return GT_OK;
}


/**
 * \brief Flushes all the fdb entries of a specific type
 *
 * \param [in] devId Device Id of device.
 * \param [in] entryType Type of FDB entry that must be flushed.
 *
 * \return XP_STATUS
 */
/*static*/ XP_STATUS xpsFdbMgrFlushEntry(xpsDevice_t devId,
                                         xpsFdbEntryType_e entryType)
{
    XPS_FUNC_ENTRY_LOG();
    XPS_LOCK(xpsFdbMgrFlushEntry);
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

    status = xpsFdbMgrFlushCommon(devId, &flushParams);
    if (status != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Fdb Hal Flush table failed");
        return xpsConvertCpssStatusToXPStatus(status);
    }

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}


/**
 * \brief Flushes all the fdb entries for a particular interface, given the interface Id
 *
 * \param [in] devId Device Id of device.
 * \param [in] intfId Interface id of the FDB entry that must be flushed.
 *
 * \return XP_STATUS
 */
/*static*/ XP_STATUS xpsFdbMgrFlushEntryByIntf(xpsDevice_t devId,
                                               xpsInterfaceId_t intfId, xpsFdbEntryType_e entryType)
{
    XPS_FUNC_ENTRY_LOG();
    XPS_LOCK(xpsFdbMgrFlushEntryByIntf);
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

    status = xpsFdbMgrFlushCommon(devId, &flushParams);
    if (status != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Fdb manager Hal Flush 'by interface' failed");
        return xpsConvertCpssStatusToXPStatus(status);
    }

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

/**
 * \brief Flushes all the fdb entries for a particular vlan
 *
 * \param [in] devId Device Id of device.
 * \param [in] vlanId Vlan id of the FDB entry that must be flushed.
 *
 * \return XP_STATUS
 */
/*static*/ XP_STATUS xpsFdbMgrFlushEntryByVlan(xpsDevice_t devId,
                                               xpsVlan_t vlanId, xpsFdbEntryType_e entryType)
{
    XPS_FUNC_ENTRY_LOG();
    XPS_LOCK(xpsFdbMgrFlushEntryByVlan);
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

    status = xpsFdbMgrFlushCommon(devId, &flushParams);
    if (status != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Fdb manager Hal Flush 'by vlan' failed with error code %d", status);
        return xpsConvertCpssStatusToXPStatus(status);
    }

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

/**
 * \brief Flushes all the fdb entries for a particular vlan and interface pair
 *
 * \param [in] devId Device Id of device.
 * \param [in] intfId Interface id of the FDB entry that must be flushed.
 * \param [in] vlanId Vlan id of the FDB entry that must be flushed.
 *
 * \return XP_STATUS
 */
/*static*/ XP_STATUS xpsFdbMgrFlushEntryByIntfVlan(xpsDevice_t devId,
                                                   xpsInterfaceId_t intfId, xpsVlan_t vlanId, xpsFdbEntryType_e entryType)
{
    XPS_FUNC_ENTRY_LOG();
    XPS_LOCK(xpsFdbMgrFlushEntryByIntfVlan);
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

    status = xpsFdbMgrFlushCommon(devId, &flushParams);
    if (status != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Fdb manager Hal Flush 'by interface,vlan' failed");
        return xpsConvertCpssStatusToXPStatus(status);
    }

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

static XP_STATUS xpsFdbMgrRegisterAgingHandler(xpsDevice_t devId,
                                               xpFdbAgingHandler fdbAgingHandler)
{
    XPS_FUNC_ENTRY_LOG();

    ageHandler = fdbAgingHandler;

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

/**
 * \brief Generic Init API for initializing the function pointers for 'fdb manager'
 *
 * \returns XP_STATUS indicating success or failure
 */
XP_STATUS xpsFdbInitApiFdbMgr(xpsDevice_t devId)
{
    xpsFdbAddDevice              = xpsFdbAddDeviceFdbMgr;
    xpsFdbRemoveDevice           = xpsFdbRemoveDeviceFdbMgr;
    xpsFdbRegisterLearnHandler   = xpsFdbRegisterLearnHandlerFdbMgr;
    xpsFdbUnregisterLearnHandler = xpsFdbUnRegisterLearnHandlerFdbMgr;
    xpsFdbRegisterAgingHandler   = xpsFdbMgrRegisterAgingHandler;
    xpsFdbFlushEntry             = xpsFdbMgrFlushEntry;
    xpsFdbFlushEntryByIntf       = xpsFdbMgrFlushEntryByIntf;
    xpsFdbFlushEntryByVlan       = xpsFdbMgrFlushEntryByVlan;
    xpsFdbFlushEntryByIntfVlan   = xpsFdbMgrFlushEntryByIntfVlan;
    xpsFdbFindEntry              = xpsFdbFindEntryFdbMgr;
    xpsFdbAddEntry               = xpsFdbAddEntryFdbMgr;
    xpsFdbAddHwEntry             = xpsFdbAddHwEntryFdbMgr;
    xpsFdbWriteEntry             = xpsFdbWriteEntryFdbMgr;
    xpsFdbRemoveEntry            = xpsFdbRemoveEntryFdbMgr;
    xpsFdbGetEntryByIndex        = xpsFdbGetEntryByIndexFdbMgr;
    xpsFdbGetEntry               = xpsFdbGetEntryFdbMgr;
    xpsFdbSetAttribute           = xpsFdbSetAttributeFdbMgr;
    xpsFdbGetAttribute           = xpsFdbGetAttributeFdbMgr;
    xpsFdbGetEntriesCount        = xpsFdbGetEntriesCountFdbMgr;

    return XP_NO_ERR;
}


static GT_U32   allow_TRACE_TEST_ADD_TO_FDB = 0;
/* debug function to allow trace of the test of 'add fdb entries' task */
GT_STATUS fdbMgr_allow_TRACE_TEST_ADD_TO_FDB(IN GT_U32   allowTrace)
{
    allow_TRACE_TEST_ADD_TO_FDB = allowTrace;
    return GT_OK;
}
#define TRACE_TEST_ADD_TO_FDB(...)  if(allow_TRACE_TEST_ADD_TO_FDB) cpssOsPrintf(__VA_ARGS__)

static GT_U32   allow_TRACE_TEST_ADD_TO_FDB_level1 = 0;
/* debug function to allow trace of the test of 'add fdb entries' task */
GT_STATUS fdbMgr_allow_TRACE_TEST_ADD_TO_FDB_level1(IN GT_U32   allowTrace)
{
    allow_TRACE_TEST_ADD_TO_FDB_level1 = allowTrace;
    return GT_OK;
}
#define LEVEL1_TRACE_TEST_ADD_TO_FDB(...)  if(allow_TRACE_TEST_ADD_TO_FDB_level1) cpssOsPrintf(__VA_ARGS__)


static GT_U32   pause_testTaskForAddingMacEntriesToFdb = 0;
GT_STATUS fdbMgr_pause_testTaskForAddingMacEntriesToFdb(IN GT_U32   pause)
{
    pause_testTaskForAddingMacEntriesToFdb = pause;
    return GT_OK;
}

static CPSS_TASK tid_testTaskForAddingMacEntriesToFdb = 0;
/* main function for the TESTING of adding FDB entries , so we can see the aging task delete entries */
static unsigned __TASKCONV testTaskForAddingMacEntriesToFdb(
    GT_VOID * param/* not used*/)
{
    GT_STATUS   rc;
    static CPSS_DXCH_BRG_FDB_MANAGER_COUNTERS_STC fdbCounters;
    CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_ADD_PARAMS_STC fdbManagerEntryAddParam;
    CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_STC  fdbEntry;
    CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_MAC_ADDR_FORMAT_STC  *fdbEntryMacAddrFormatPtr;
    GT_U32  rand30Bits;
    GT_U32  iterator = 0;
    GT_U32  numEntriesOK = 0;

    cpssOsMemSet(&fdbManagerEntryAddParam, 0, sizeof(fdbManagerEntryAddParam));
    fdbManagerEntryAddParam.tempEntryExist =
        GT_FALSE;/* adding entry , that not triggered from the AUQ of 'new address'/'moved address' */
    fdbManagerEntryAddParam.tempEntryOffset = 0;/* not relevant*/
    fdbManagerEntryAddParam.rehashEnable = GT_TRUE;

    cpssOsMemSet(&fdbEntry, 0, sizeof(fdbEntry));
    fdbEntry.fdbEntryType = CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_MAC_ADDR_E;
    fdbEntryMacAddrFormatPtr = &fdbEntry.format.fdbEntryMacAddrFormat;
    /* this bit is actually 'is entry refreshed' , and for '2 cycle' aging. we need this field to be with value GT_TRUE */
    fdbEntryMacAddrFormatPtr->age = GT_TRUE;

    while (1)
    {
        while (pause_testTaskForAddingMacEntriesToFdb)
        {
            cpssOsTimerWkAfter(500);
        }

        rand30Bits = cpssOsRand() | cpssOsRand() << 15;
        fdbEntryMacAddrFormatPtr->macAddr.arEther[0] = ((rand30Bits >> 24) & 0xFF);
        fdbEntryMacAddrFormatPtr->macAddr.arEther[1] = ((rand30Bits >> 16) & 0xFF);
        fdbEntryMacAddrFormatPtr->macAddr.arEther[2] = ((rand30Bits >>  8) & 0xFF);
        fdbEntryMacAddrFormatPtr->macAddr.arEther[3] = ((rand30Bits >>  0) & 0xFF);

        rand30Bits = cpssOsRand() | cpssOsRand() << 15;
        fdbEntryMacAddrFormatPtr->macAddr.arEther[4] = ((rand30Bits >>  8) & 0xFF);
        fdbEntryMacAddrFormatPtr->macAddr.arEther[5] = ((rand30Bits >>  0) & 0xFF);

        fdbEntryMacAddrFormatPtr->fid = iterator & 0x1FFF;
        /* Set the flag to support Application Specific CPU CODE Assignment */
        fdbEntry.format.fdbEntryMacAddrFormat.appSpecificCpuCode = GT_TRUE;

        rc = cpssDxChBrgFdbManagerEntryAdd(fdbManagerId, &fdbEntry,
                                           &fdbManagerEntryAddParam);
        if (rc == GT_OK)
        {
            TRACE_TEST_ADD_TO_FDB("++: fid[%d]", fdbEntryMacAddrFormatPtr->fid);
            if ((numEntriesOK & 0xff) == 0xff)
            {
                TRACE_TEST_ADD_TO_FDB("\n");
                if (allow_TRACE_TEST_ADD_TO_FDB_level1)
                {
                    static GT_U32 lastTime = 0;
                    cpssDxChBrgFdbManagerCountersGet(fdbManagerId, &fdbCounters);

                    if (lastTime != fdbCounters.usedEntries)
                    {
                        LEVEL1_TRACE_TEST_ADD_TO_FDB("++: usedEntries[%d]", fdbCounters.usedEntries);
                        lastTime = fdbCounters.usedEntries;
                    }
                }

                /* every 256 entries ... get some sleep */
                cpssOsTimerWkAfter(5);
            }
            numEntriesOK++;
        }
        iterator++;
    }

    return 0;/* should never get here */
}

/* create task for TESTING purposes only */
XP_STATUS xpsFdbMgrTest_createTaskForAddingMacEntriesToFdb(void)
{
    GT_STATUS   status;

    if (tid_testTaskForAddingMacEntriesToFdb == 0)
    {
        status = cpssOsTaskCreate("testAddFdbEntries",
                                  TEST_ADD_FDB_ENTRIES_TASK_PRIO,
                                  _24KB,
                                  testTaskForAddingMacEntriesToFdb,
                                  NULL,
                                  &tid_testTaskForAddingMacEntriesToFdb);
        if (status != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "cpssOsTaskCreate failed to creating AgingTask\n");
            return xpsConvertCpssStatusToXPStatus(status);
        }
    }

    return XP_NO_ERR;
}


XP_STATUS xpsFdbFindEntryFdbMgr(xpsDevice_t devId, xpsFdbEntry_t *fdbEntry,
                                uint32_t *index)
{
    XPS_FUNC_ENTRY_LOG();

    xpsLockTake(XP_LOCKINDEX_XPS_FDB_LOCK);

    XPS_FDB_ENTRY_TYPE_ENT fdbEntryType = XPS_FDB_ENTRY_TYPE_MAC_ADDR_E;
    GT_STATUS status = GT_OK;

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
        return XP_ERR_NULL_POINTER;
    }

    status = cpssHalFdbMgrMacEntryFind(devId, fdbManagerId, fdbEntryType,
                                       (void *) fdbEntry);
    if (status != GT_OK)
    {
        if (status == GT_NOT_FOUND)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_DEBUG, "Entry not found");
            xpsLockRelease(XP_LOCKINDEX_XPS_FDB_LOCK);
            return xpsConvertCpssStatusToXPStatus(status);
        }

        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              " Entry find failed with cpss error code %d", status);
        xpsLockRelease(XP_LOCKINDEX_XPS_FDB_LOCK);
        return xpsConvertCpssStatusToXPStatus(status);
    }

    xpsLockRelease(XP_LOCKINDEX_XPS_FDB_LOCK);

    XPS_FUNC_EXIT_LOG();

    return xpsConvertCpssStatusToXPStatus(status);;
}


XP_STATUS xpsFdbAddHwEntryFdbMgr(xpsDevice_t devId, xpsFdbEntry_t *fdbEntry,
                                 xpsHashIndexList_t *indexList)
{
    XPS_FUNC_ENTRY_LOG();

    XP_STATUS result = XP_NO_ERR;
    GT_STATUS status = GT_OK;
    xpsScope_t scopeId;
    xpsL2EncapType_e encapType;
    XPS_FDB_ENTRY_TYPE_ENT fdbEntryType = XPS_FDB_ENTRY_TYPE_MAC_ADDR_E;

    if (indexList == NULL)
    {
        return XP_ERR_NULL_POINTER;
    }

    /* Get Scope Id from devId */
    if ((result = xpsScopeGetScopeId(devId, &scopeId))!= XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Get Scope failed with xps error code %d", result);
        return result;
    }

    // TODO --- is this required?
    //TODO: this is a hack to fix SONIC-191. The values are not used in the further logic
    //it is only used to ensure that FDB can be added only for a member port
    //of the fdb vlan.
    if ((result = xpsVlanGetIntfTagTypeScope(scopeId, fdbEntry->vlanId,
                                             fdbEntry->intfId, &encapType)) != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Get tag type scope failed with xps error code %d", result);
        return result;
    }

#if 0
    status = cpssHalFdbMgrMacEntryFind(devId, fdbManagerId, fdbEntryType,
                                       (void *) fdbEntry);
    if (status == GT_OK)
    {
        /*Entry exists already so return entry exists*/
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Entry already exists in fdb table at index ");
        return XP_ERR_KEY_EXISTS;
    }

    else if (status == GT_FULL)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "FDB table is full and entry not found");
        return xpsConvertCpssStatusToXPStatus(status);
    }

    else if (status != GT_NOT_FOUND)
    {
        /*some other error other than finding entry*/
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Fdb find failed with cpss error code: (%d)", status);
        return xpsConvertCpssStatusToXPStatus(status);
    }
#endif

    /*Create a new Fdb Mac Entry  */
    status = cpssHalFdbMgrMacEntryWrite(devId, fdbManagerId, fdbEntryType,
                                        (void *) fdbEntry);
    if (status != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "FDB entry write failed with cpss error code %d", status);
        return xpsConvertCpssStatusToXPStatus(status);
    }

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}


XP_STATUS xpsFdbAddEntryFdbMgr(xpsDevice_t devId, xpsFdbEntry_t *fdbEntry,
                               xpsHashIndexList_t *indexList)
{
    XPS_FUNC_ENTRY_LOG();

    //Get private-vlan Type
    XP_STATUS result = XP_NO_ERR;
    xpsVlan_t vlanId;
    xpsPrivateVlanType_e vlanType = VLAN_NONE;

    if (indexList == NULL || !fdbEntry)
    {
        return XP_ERR_NULL_POINTER;
    }

    if (IS_DEVICE_VALID(devId) == 0)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Error: Invalid device %d\n",
              devId);
        return XP_ERR_INVALID_DEV_ID;
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
        result = xpsFdbAddHwEntryFdbMgr(devId, fdbEntry, indexList);
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

    XPS_FUNC_EXIT_LOG();
    return result;
}


XP_STATUS xpsFdbWriteEntryFdbMgr(xpsDevice_t devId,  uint32_t index,
                                 xpsFdbEntry_t *fdbEntry)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_LOCK(xpsFdbWriteEntryFdbMgr);
    XPS_FDB_ENTRY_TYPE_ENT fdbEntryType = XPS_FDB_ENTRY_TYPE_MAC_ADDR_E;
    GT_STATUS status = GT_OK;

    if (IS_DEVICE_VALID(devId) == 0)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Error: Invalid device %d\n",
              devId);
        return XP_ERR_INVALID_DEV_ID;
    }

    if (!fdbEntry)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Null pointer recieved");
        return XP_ERR_NULL_POINTER;
    }

    status = cpssHalFdbMgrMacEntryWrite(devId, fdbManagerId, fdbEntryType,
                                        (void *) fdbEntry);
    if (status != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "FDB entry write failed with cpss error code %d", status);
        return xpsConvertCpssStatusToXPStatus(status);
    }

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;

}


XP_STATUS xpsFdbRemoveEntryFdbMgr(xpsDevice_t devId, xpsFdbEntry_t *fdbEntry)
{
    XPS_FUNC_ENTRY_LOG();

    XP_STATUS result = XP_NO_ERR;
    GT_STATUS status = GT_OK;
    XPS_FDB_ENTRY_TYPE_ENT fdbEntryType = XPS_FDB_ENTRY_TYPE_MAC_ADDR_E;
    uint32_t index;

    if (IS_DEVICE_VALID(devId) == 0)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Error: Invalid device %d\n",
              devId);
        return XP_ERR_INVALID_DEV_ID;
    }

    result = xpsFdbFindEntry(devId, fdbEntry, &index);
    if (result != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Entry find failed code (%d)\n", result);
        return XP_ERR_KEY_EXISTS;
    }

    status = cpssHalFdbMgrMacEntryDelete(devId, fdbManagerId, fdbEntryType,
                                         (void *)fdbEntry);
    if (status != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Entry deletion failed with cpss error code %d\n", status);
        return xpsConvertCpssStatusToXPStatus(status);
    }

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

/* This Api is deprecated in the usage of Fdb manager*/
/* This api is maintained here only to support the object oriented approach*/

XP_STATUS xpsFdbGetEntryByIndexFdbMgr(xpsDevice_t devId, uint32_t index,
                                      xpsFdbEntry_t *fdbEntry)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}


XP_STATUS xpsFdbGetEntryFdbMgr(xpsDevice_t devId, xpsFdbEntry_t *fdbEntry)
{
    XPS_FUNC_ENTRY_LOG();

    GT_STATUS status = GT_OK;
    XP_STATUS result = XP_NO_ERR;
    uint32_t index;
    XPS_FDB_ENTRY_TYPE_ENT fdbEntryType = XPS_FDB_ENTRY_TYPE_MAC_ADDR_E;

    if (IS_DEVICE_VALID(devId) == 0)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Error: Invalid device %d\n",
              devId);
        return XP_ERR_INVALID_DEV_ID;
    }

    if (!fdbEntry)
    {
        return XP_ERR_NULL_POINTER;
    }

    result = xpsFdbFindEntryFdbMgr(devId, fdbEntry, &index);
    if (result != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_DEFAULT,
              "Entry not found in table Error code : (%d)", result);
        return XP_ERR_KEY_EXISTS;
    }

    status = cpssHalFdbMgrMacEntryGet(devId, fdbManagerId, fdbEntryType,
                                      (void *) fdbEntry);
    if (status != GT_OK)
    {
        if (status == GT_NOT_FOUND)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_DEBUG, "Entry not found");
            return xpsConvertCpssStatusToXPStatus(status);
        }

        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              " Entry find failed with cpss error code %d", status);
        return xpsConvertCpssStatusToXPStatus(status);
    }

    XPS_FUNC_EXIT_LOG();

    return result;
}


XP_STATUS xpsFdbSetAttributeFdbMgr(xpsDevice_t devId, xpsFdbEntry_t *fdbEntry,
                                   xpsFdbAttribute_e field, void *data)
{
    XPS_FUNC_ENTRY_LOG();

    GT_STATUS status = GT_OK;
    XPS_FDB_ENTRY_TYPE_ENT fdbEntryType = XPS_FDB_ENTRY_TYPE_MAC_ADDR_E;

    if (IS_DEVICE_VALID(devId) == 0)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Error: Invalid device %d\n",
              devId);
        return XP_ERR_INVALID_DEV_ID;
    }

    if (!fdbEntry || !data)
    {
        return XP_ERR_NULL_POINTER;
    }

    status = cpssHalFdbMgrMacEntryGet(devId, fdbManagerId, fdbEntryType,
                                      (void *)fdbEntry);
    if (status != GT_OK)
    {
        if (status == GT_NOT_FOUND)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_DEBUG, "Entry not found");
            return xpsConvertCpssStatusToXPStatus(status);
        }

        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              " Entry find failed with cpss error code %d", status);
        return xpsConvertCpssStatusToXPStatus(status);
    }

    switch (field)
    {
        case XPS_FDB_PKT_CMD:
            fdbEntry->pktCmd = *(xpPktCmd_e *)data;
            break;
        case XPS_FDB_IS_ROUTER_MAC:
            fdbEntry->isRouter = *(uint8_t *)data;
            break;
        case XPS_FDB_IS_STATIC_MAC:
            fdbEntry->isStatic = *(uint8_t *)data;
            break;
        default:
            return XP_ERR_INVALID_DATA;
    }

    status = cpssHalFdbMgrMacEntryUpdate(devId, fdbManagerId, fdbEntryType,
                                         (void *)fdbEntry);
    if (status != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "FDB entry update failed with cpss error code %d", status);
        return xpsConvertCpssStatusToXPStatus(status);
    }

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}



XP_STATUS xpsFdbGetAttributeFdbMgr(xpsDevice_t devId, xpsFdbEntry_t *fdbEntry,
                                   xpsFdbAttribute_e field, void *data)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FDB_ENTRY_TYPE_ENT fdbEntryType = XPS_FDB_ENTRY_TYPE_MAC_ADDR_E;
    GT_STATUS status = GT_OK;

    if (IS_DEVICE_VALID(devId) == 0)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Error: Invalid device %d\n",
              devId);
        return XP_ERR_INVALID_DEV_ID;
    }

    if (!fdbEntry || !data)
    {
        return XP_ERR_NULL_POINTER;
    }

    status = cpssHalFdbMgrMacEntryGet(devId, fdbManagerId, fdbEntryType,
                                      (void *)fdbEntry);
    if (status != GT_OK)
    {
        if (status == GT_NOT_FOUND)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_DEBUG, "Entry not found");
            return xpsConvertCpssStatusToXPStatus(status);
        }

        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              " Entry find failed with cpss error code %d", status);
        return xpsConvertCpssStatusToXPStatus(status);
    }

    switch (field)
    {
        case XPS_FDB_PKT_CMD:
            *(xpPktCmd_e *)data = fdbEntry->pktCmd;
            break;
        case XPS_FDB_IS_ROUTER_MAC:
            *(uint8_t *)data = fdbEntry->isRouter;
            break;
        case XPS_FDB_IS_STATIC_MAC:
            *(uint8_t *)data = fdbEntry->isStatic;
            break;
        default:
            return XP_ERR_INVALID_DATA;
    }

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsFdbGetEntriesCountFdbMgr(xpsDevice_t devId, uint32_t *usedCount)
{
    GT_STATUS status = GT_OK;

    if ((status = cpssHalFdbMgrMacCountersGet(devId, fdbManagerId, usedCount)))
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Failed to get count\n");
        return xpsConvertCpssStatusToXPStatus(status);

    }
    return XP_NO_ERR;
}


XP_STATUS xpsFdbGetEntryByKeyFdbMgr(xpsDevice_t devId, xpsFdbEntry_t *fdbEntry)
{
    XP_STATUS status = XP_NO_ERR;

    status = xpsFdbGetEntryFdbMgr(devId, fdbEntry);
    if (status != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Finding fdb entry failed");
        return status;
    }

    return XP_NO_ERR;

}

XP_STATUS xpsFdbFdbMgrBrgSecurBreachCommandSet(xpsDevice_t devId,
                                               xpsPktCmd_e command)
{
    XPS_FUNC_ENTRY_LOG();

    GT_STATUS status = GT_OK;

    if (IS_DEVICE_VALID(devId) == 0)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Error: Invalid device %d\n",
              devId);
        return XP_ERR_INVALID_DEV_ID;
    }

    status = cpssHalFdbMgrBrgSecurBreachCommandSet(devId,
                                                   CPSS_BRG_SECUR_BREACH_EVENTS_MOVED_STATIC_E, command);
    if (status != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Secur Breach set failed with cpss error code %d\n", status);
        return xpsConvertCpssStatusToXPStatus(status);
    }

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}


XP_STATUS xpsFdbFdbMgrBrgSecurBreachCommandGet(xpsDevice_t devId,
                                               xpsPktCmd_e *command)
{
    XPS_FUNC_ENTRY_LOG();

    GT_STATUS status = GT_OK;

    if (IS_DEVICE_VALID(devId) == 0)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Error: Invalid device %d\n",
              devId);
        return XP_ERR_INVALID_DEV_ID;
    }

    status = cpssHalFdbMgrBrgSecurBreachCommandGet(devId,
                                                   CPSS_BRG_SECUR_BREACH_EVENTS_MOVED_STATIC_E, command);
    if (status != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Secur Breach get failed with cpss error code %d\n", status);
        return xpsConvertCpssStatusToXPStatus(status);
    }

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

#if 0

/*======================================================================================*/
/*                                      STUB CODE                                       */
/*======================================================================================*/

#define DEFAULT_MAC_AGE_CNS          300
#define DEFAULT_MAC_AGE_FOR_SP_CNS   5
#define DEFAILT_MAC_AGING_TYPE_CNS   MAC_ADDR_AGING_SW
#define MAC_AGE_MSG_NOTIFICATION_INTERVAL_CNS   1
#define CPSS_MSG_RATE_LIMIT          1200

uint32_t default_mac_age = DEFAULT_MAC_AGE_CNS;
uint32_t default_mac_age_sp = DEFAULT_MAC_AGE_FOR_SP_CNS;



#define HAL_MAC_HASH_CHAIN_LEN       4
#define HAL_MAX_DEVICES              2

mac_aging_type default_mac_age_type = DEFAILT_MAC_AGING_TYPE_CNS;

typedef struct
{
    uint32_t index;
    uint32_t macAge;
    bool valid;
} macData;

macData *macAddressData = NULL;

bool flag1 = false;
uint32_t temp = 13137;


XP_STATUS xpsFdbMgrInit(void)
{
    XPS_FUNC_ENTRY_LOG();

    return xpsFdbMgrInitScope(XP_SCOPE_DEFAULT);

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsFdbMgrInitScope(xpsScope_t scopeId)
{
    XPS_FUNC_ENTRY_LOG();

    xpsLockCreate("XP_LOCKINDEX_XPS_FDB_LOCK", XP_LOCKINDEX_XPS_FDB_LOCK);

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsFdbMgrDeInit(void)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsFdbMgrDeInitScope(xpsScope_t scopeId)
{
    XPS_FUNC_ENTRY_LOG();
    xpsLockDestroy(XP_LOCKINDEX_XPS_FDB_LOCK);

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}


/* api for configuring fdb defaults */
XP_STATUS xpsFdbMgrDefaults(xpDevice_t devId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();
    return XP_NO_ERR;
}


/*api to prevent security breach event by MAC*/
XP_STATUS xpsFdbMgrMacSecurity(xpDevice_t devId)
{
    XPS_FUNC_ENTRY_LOG();
    XPS_FUNC_EXIT_LOG();
    return XP_NO_ERR;
}

XP_STATUS xpsFdbMgrSetAgingTime(xpsDevice_t devId, uint32_t agingTime)
{
    XPS_FUNC_ENTRY_LOG();

    gAgingTimer = agingTime;

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsFdbMgrGetAgingTime(xpsDevice_t devId, uint32_t *agingTime)
{
    XPS_FUNC_ENTRY_LOG();

    *agingTime = gAgingTimer;

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsFdbMgrSetMsgRateLimit(xpsDevice_t devId, uint32_t rateLimit,
                                   uint8_t enable)
{
    return XP_NO_ERR;
}

#endif

#ifdef __cplusplus
}
#endif
