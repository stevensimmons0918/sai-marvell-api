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
* @file cpssAppUtilsEvents.c
*
* @brief Implements common code for unified events.
*
* @version   1
********************************************************************************
*/



#include <cpss/common/cpssTypes.h>
#include <cpssDriver/pp/prvCpssDrvPpDefs.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>
#include <appDemo/sysHwConfig/gtAppDemoSysConfig.h>
#include <cpssAppUtilsEvents.h>

#ifdef CPSS_APP_PLATFORM
    #include <cpssAppPlatformSysConfig.h>
    #include <appReference/cpssAppRefUtils.h>
#endif

static GT_BOOL prvAppUtilsUniEventEmulateLegacyCounters = GT_TRUE;

/* event handler counter update protection */
CPSS_OS_MUTEX appUtilsEventCounterUpdateLockMtx;

APP_UTILS_UNI_EVENT_COUNTER_STC  *appUtilsEventCounters[APP_UTILS_MAX_INSTANCES_CNS];

/* debug flag to open trace of events */
GT_U32 wrapCpssTraceEvents = 0;
const char * uniEvName[CPSS_UNI_EVENT_COUNT_E] = {UNI_EV_NAME};

/**
* @internal cpssAppUtilsEventLegacyCounterEnable function
* @endinternal
*
* @brief   Emulates old counters functionality. In this mode there will be no dynamic allocation per event/extended data, and
*          the counters will be incremented per event type.
*
* @param[in] enable   - enable/disable 'old' counting mode
*                       GT_TRUE     - emulate legacy mode
*                       GT_FALSE    - the new count mode is active
*
*/
GT_VOID cpssAppUtilsEventLegacyCounterEnable
(
    IN GT_BOOL   enable
)
{
    prvAppUtilsUniEventEmulateLegacyCounters = enable;
}

/**
* @internal cpssAppUtilsGenEventCounterIncrement function
* @endinternal
*
* @brief  Increments the event counter per extended data according to the device number and the event type. Iterates link list
*         per event type to find entry for spesific extended data and if entry found increments counter, otherwise allocates new
*         entry and adds it to link list before counter been incremented.
*
*
* @param[in] devNum                   - device number
* @param[in] uniEvCounter             - number which represent the event type
* @param[in] evExtData                - extended data of event
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - in case of bad parameters
* @retval GT_OUT_OF_CPU_MEM        - in case of running out of memory when initialize the database
*/
GT_STATUS cpssAppUtilsGenEventCounterIncrement
(
    IN  GT_U8   devNum,
    IN GT_U32   uniEvCounter,
    IN GT_U32   evExtData
)
{
    APP_UTILS_UNI_EVENT_COUNTER_STC *currentEntryPtr;

    GT_U32 eventsNum = CPSS_UNI_EVENT_COUNT_E; /* number of events */
    GT_BOOL wasNullBeforeCall = GT_FALSE;

    if(devNum >= APP_UTILS_MAX_INSTANCES_CNS)
    {
        /* check device out of range */
        return GT_BAD_PARAM;
    }

    /* Counting the event */
    if(appUtilsEventCounters[devNum] == NULL)
    {
        wasNullBeforeCall = GT_TRUE;
        appUtilsEventCounters[devNum] = (APP_UTILS_UNI_EVENT_COUNTER_STC *)osMalloc((eventsNum) * sizeof(APP_UTILS_UNI_EVENT_COUNTER_STC));

        if(appUtilsEventCounters[devNum] == NULL)
            return GT_OUT_OF_CPU_MEM;

        osMemSet(appUtilsEventCounters[devNum], 0, ((eventsNum) * sizeof(APP_UTILS_UNI_EVENT_COUNTER_STC)));
    }

    if(uniEvCounter == 0xFFFFFFFF && wasNullBeforeCall == GT_TRUE)
    {
        /* just trigger to allocate the needed memory for the device (during initialization)
           and not to wait till the first event.

           because the 'Enhanced UT' will check for memory leakage , and if first
           event will come only after the test started , then the test will fail .
            */
        return GT_OK;
    }

    if (uniEvCounter >= eventsNum)
    {
        return GT_BAD_PARAM;
    }

    osMutexLock(appUtilsEventCounterUpdateLockMtx);

    currentEntryPtr = &appUtilsEventCounters[devNum][uniEvCounter];

    if(prvAppUtilsUniEventEmulateLegacyCounters == GT_FALSE)
    {
        while(currentEntryPtr)
        {
            /* First empty entry in the link list head */
            if(currentEntryPtr->counterValue == 0)
            {
                currentEntryPtr->extData = evExtData;
                break;
            }
            /* The entry was found  */
            if (currentEntryPtr->extData == evExtData)
            {
                break;
            }

            /* Last entry in the link list */
            if(currentEntryPtr->nextEntryPtr == NULL)
            {
                /* Need to allocate new memory */
                currentEntryPtr->nextEntryPtr = (APP_UTILS_UNI_EVENT_COUNTER_STC *)osMalloc(sizeof(APP_UTILS_UNI_EVENT_COUNTER_STC));
                if(currentEntryPtr->nextEntryPtr == NULL)
                {
                    osMutexUnlock(appUtilsEventCounterUpdateLockMtx);
                    return GT_OUT_OF_CPU_MEM;
                }
                currentEntryPtr = currentEntryPtr->nextEntryPtr;
                /* Initialize new entry */
                osMemSet(currentEntryPtr, 0, sizeof(APP_UTILS_UNI_EVENT_COUNTER_STC));
                currentEntryPtr->extData = evExtData;
                break;
            }

            /* Iterate to the next entry in the link list */
            currentEntryPtr = currentEntryPtr->nextEntryPtr;
        }
    }

    if (currentEntryPtr)
    {
        /* Increment counter for event extended data */
        currentEntryPtr->counterValue++;
    }

    if (wrapCpssTraceEvents)
    {
        osPrintf("Event [%s], extData [%d], counter [%d]\n", uniEvName[uniEvCounter], evExtData, appUtilsEventCounters[devNum][uniEvCounter].counterValue);
    }

    osMutexUnlock(appUtilsEventCounterUpdateLockMtx);

    return GT_OK;
}

/**
* @internal prvAppUtilsGenEventCounterGet function
* @endinternal
*
* @brief   Gets the number of times that specific event happened.
*
* @param[in] devNum                   - device number
* @param[in] uniEvent                 - unified event
* @param[in] clearOnRead              - do we 'clear' the counter after 'read' it
*                                      GT_TRUE - set counter to 0 after get it's value
*                                      GT_FALSE - don't update the counter (only read it)
*
* @param[out] counterPtr               - (pointer to)the counter (the number of times that specific event happened)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad devNum or uniEvent.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_NOT_INITIALIZED       - the counters DB not initialized for the device.
*
* @note none
*
*/
static GT_STATUS prvAppUtilsGenEventCounterGet
(
    IN GT_U8                    devNum,
    IN CPSS_UNI_EV_CAUSE_ENT    uniEvent,
    IN GT_BOOL                  clearOnRead,
    OUT GT_U32                 *counterPtr
)
{
    APP_UTILS_UNI_EVENT_COUNTER_STC *currentEntryPtr;

    if (counterPtr != NULL)
    {
        *counterPtr = 0;
    }

    osMutexLock(appUtilsEventCounterUpdateLockMtx);

    currentEntryPtr = &appUtilsEventCounters[devNum][uniEvent];

    while(currentEntryPtr)
    {
        if (counterPtr != NULL)
        {
            *counterPtr += currentEntryPtr->counterValue;
        }

        if(clearOnRead == GT_TRUE)
        {
            currentEntryPtr->counterValue = 0;
        }
        currentEntryPtr = currentEntryPtr->nextEntryPtr;
    }

    osMutexUnlock(appUtilsEventCounterUpdateLockMtx);

    return GT_OK;
}

/**
* @internal prvEventExtendedDataCounterReadAndClear function
* @endinternal
*
* @brief  Gets counters per event type and extended data and optionally clears it after read.
*
* @param[in] devNum                 - device number
* @param[in] uniEvCounter           - event type number
* @param[in] evExtData              - extended data of event
* @param[in] clearOnRead            - do we 'clear' the counter after 'read' it
*                                       GT_TRUE     - set counter to 0 after get it's value
*                                       GT_FALSE    - don't update the counter (only read it)
* @param[out] counterPtr            - (pointer to) counter for event type and extended data
*
*/
static GT_VOID prvEventExtendedDataCounterReadAndClear
(
    IN  GT_U8                           devNum,
    IN  CPSS_UNI_EV_CAUSE_ENT           uniEvCounter,
    IN  GT_U32                          evExtData,
    IN  GT_BOOL                         clearOnRead,
    OUT GT_U32                          *counterPtr
)
{
    APP_UTILS_UNI_EVENT_COUNTER_STC *currentEntryPtr;

    if (counterPtr != NULL)
    {
        *counterPtr = 0;
    }

    osMutexLock(appUtilsEventCounterUpdateLockMtx);

    currentEntryPtr = &appUtilsEventCounters[devNum][uniEvCounter];

    while(currentEntryPtr)
    {
        if(currentEntryPtr->extData == evExtData)
        {
            if (counterPtr != NULL)
            {
                *counterPtr = currentEntryPtr->counterValue;
            }
            if(clearOnRead)
            {
                currentEntryPtr->counterValue = 0;
            }
            break;
        }

        currentEntryPtr = currentEntryPtr->nextEntryPtr;
    }

    osMutexUnlock(appUtilsEventCounterUpdateLockMtx);
}

/**
* @internal cpssAppUtilsExtendedEventCounterGet function
* @endinternal
*
* @brief  Gets counters per event type and extended data and optionally clears it after read.
*
* @param[in] devNum                 - device number
* @param[in] uniEvCounter           - event type number
* @param[in] evExtData              - extended data of event
* @param[in] clearOnRead            - do we 'clear' the counter after 'read' it
*                                       GT_TRUE     - set counter to 0 after get it's value
*                                       GT_FALSE    - don't update the counter (only read it)
* @param[out] counterPtr            - (pointer to) counter for event type and extended data
*
* @retval GT_OK                     - on success
* @retval GT_BAD_PARAM              - on wrong parameter
* @retval GT_BAD_PTR                - on NULL pointer.
*
* @note none
*
*/
GT_STATUS cpssAppUtilsExtendedEventCounterGet
(
    IN  GT_U8                           devNum,
    IN  CPSS_UNI_EV_CAUSE_ENT           uniEvCounter,
    IN  GT_U32                          evExtData,
    IN  GT_BOOL                         clearOnRead,
    OUT GT_U32                          *counterPtr
)
{
    if (uniEvCounter >= CPSS_UNI_EVENT_COUNT_E)
    {
        return GT_BAD_PARAM;
    }

    if(counterPtr == NULL && clearOnRead == GT_FALSE)
    {
        return GT_BAD_PTR;
    }

    /* Read and clear counter per event type and extended data */
    prvEventExtendedDataCounterReadAndClear(devNum, uniEvCounter, evExtData, clearOnRead, counterPtr);

    return GT_OK;
}

/**
* @internal cpssAppUtilsEventHandlerPreInit function
* @endinternal
*
* @brief   Initialize event counters for specific device.
*
* @param [in] devNum       - CPSS Device Number.
*
* @retval GT_OK            - on success,
* @retval GT_FAIL          - otherwise.
*/
GT_STATUS cpssAppUtilsEventHandlerPreInit
(
    IN GT_U8 devNum
)
{
    GT_STATUS rc = GT_OK;

    if(appUtilsEventCounterUpdateLockMtx == (CPSS_OS_MUTEX)0)
    {
        rc = cpssOsMutexCreate("appUtilsEventCounterUpdateLockMtx", &appUtilsEventCounterUpdateLockMtx);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    cpssOsMutexLock(appUtilsEventCounterUpdateLockMtx);

    /* Initialize event counters */
    if(!PRV_CPSS_IS_DEV_EXISTS_MAC(devNum))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if(appUtilsEventCounters[devNum] == NULL)
    {
        appUtilsEventCounters[devNum] =  (APP_UTILS_UNI_EVENT_COUNTER_STC *)cpssOsMalloc((CPSS_UNI_EVENT_COUNT_E) * sizeof(APP_UTILS_UNI_EVENT_COUNTER_STC));

        if(appUtilsEventCounters[devNum] == NULL)
        {
            cpssOsMutexUnlock(appUtilsEventCounterUpdateLockMtx);
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
        }

        cpssOsMemSet(appUtilsEventCounters[devNum], 0, ((CPSS_UNI_EVENT_COUNT_E) * sizeof(APP_UTILS_UNI_EVENT_COUNTER_STC)));
    }

    cpssOsMutexUnlock(appUtilsEventCounterUpdateLockMtx);

    return rc;
}

/**
* @internal cpssAppUtilsEventCounterGet function
* @endinternal
*
* @brief   Gets the number of times that specific event happened.
*
* @param[in] devNum                - device number
* @param[in] uniEvent              - unified event
* @param[in] clearOnRead           - GT_TRUE - set counter to 0 after get it's value
*                                    GT_FALSE - don't update the counter (only read it)
*
* @param[out] counterPtr           - pointer to the counter
*                                    Number of times that specific event happened.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad devNum or uniEvent.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_NOT_INITIALIZED       - the counters DB not initialized for the device.
*
*/
GT_STATUS cpssAppUtilsEventCounterGet
(
    IN GT_U8                 devNum,
    IN CPSS_UNI_EV_CAUSE_ENT uniEvent,
    IN GT_BOOL               clearOnRead,
    OUT GT_U32              *counterPtr
)
{
    if(!PRV_CPSS_IS_DEV_EXISTS_MAC(devNum) || uniEvent >= CPSS_UNI_EVENT_COUNT_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if(counterPtr == NULL && clearOnRead != GT_TRUE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PTR, LOG_ERROR_NO_MSG);
    }

    /* Counting the event */
    if(appUtilsEventCounters[devNum] == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    return prvAppUtilsGenEventCounterGet(devNum, uniEvent, clearOnRead, counterPtr);
}

/**
* @internal cpssAppUtilsUniEventsFree function
* @endinternal
*
* @brief   This routine frees memory for unified event counters
* counter.
*
* @retval  None
*/
GT_VOID cpssAppUtilsUniEventsFree
(
    IN GT_U8 devNum
)
{
    GT_U32 uniEvent;        /* number of events */
    APP_UTILS_UNI_EVENT_COUNTER_STC *currentEntryPtr, *nextEntryPtr;

    if(appUtilsEventCounters[devNum] == NULL)
    {
        /* Device not exist */
        return;
    }

    cpssOsMutexLock(appUtilsEventCounterUpdateLockMtx);

    for(uniEvent = 0; uniEvent < CPSS_UNI_EVENT_COUNT_E; uniEvent++)
    {
        currentEntryPtr = appUtilsEventCounters[devNum][uniEvent].nextEntryPtr;

        /* Free all counters for specific event */
        while(currentEntryPtr)
        {
            /* Save next pointer to free it in next iteration */
            nextEntryPtr = currentEntryPtr->nextEntryPtr;
            /* Free current entry */
            osFree(currentEntryPtr);
            /* Iterate to the next entry */
            currentEntryPtr = nextEntryPtr;
        }
        appUtilsEventCounters[devNum][uniEvent].nextEntryPtr = NULL;
    }

    osFree(appUtilsEventCounters[devNum]);
    appUtilsEventCounters[devNum] = NULL;

    cpssOsMutexUnlock(appUtilsEventCounterUpdateLockMtx);
}

/**
* @internal cpssAppUtilsGenExtendedEventCounterEntryGet function
* @endinternal
*
* @brief  Gets pointer to event counters entry from DB.
*
* @param[in] devNum                 - device number
* @param[in] uniEvCounter           - event type number
* @param[inout] eventEntryPtr       - (pointer to (pointer to)) counter entry per event type.
*                                     If NULL pointer - set the ponter to the first entry in link list
*                                     Else - move to the next entry and set pointer to the entry
*
*
*/
GT_VOID cpssAppUtilsGenExtendedEventCounterEntryGet
(
    IN  GT_U8                               devNum,
    IN  CPSS_UNI_EV_CAUSE_ENT               uniEvCounter,
    INOUT APP_UTILS_UNI_EVENT_COUNTER_STC   **eventEntryPtr
)
{
    APP_UTILS_UNI_EVENT_COUNTER_STC *currentEntryPtr = *eventEntryPtr;

    cpssOsMutexLock(appUtilsEventCounterUpdateLockMtx);

    if(currentEntryPtr == NULL)
    {
        /* Set pointer to first entry in list */
        currentEntryPtr = &appUtilsEventCounters[devNum][uniEvCounter];
    }
    else
    {
        /* Set pointer to next entry in list */
        currentEntryPtr = currentEntryPtr->nextEntryPtr;
    }

    *eventEntryPtr = currentEntryPtr;

    cpssOsMutexUnlock(appUtilsEventCounterUpdateLockMtx);
}

/**
* @internal cpssAppUtilsGenExtendedEventCounterEntryGetNext function
* @endinternal
*
* @brief  Gets events and extended data with non-zero counters from events table.
*
* @param[in] devNum                 - device number 
* @param[in] getFirst               - start from the first entry in  event table
*                                       GT_TRUE  - start reading from the begining of event table
*                                       GT_FALSE - continue reading from current entry of event table
* @param[in] uniEvCounter           - event type number 
*                                       If getFirst = True, ignore this parameter,
*                                       Else start reading from this event
* @param[in] evExtData              - extended data of event
*                                       If getFirst - True, ignore this parameter,
*                                       Else start reading from the specific event and extended data
* @param[out] uniEvCounter          - (pointer to) event type number with non-zero counters 
*                                       The first event with non-zero counters that was found in the event table
* @param[out] evExtData             - (pointer to) extended data of event with non-zero counters
*                                       The first extended data for event with non-zero counters that was found in the event table
* @param[out] counterPtr            - (pointer to) counter for event and extended data
*                                       The number of times that specific event with extended data is happened, or zero if not found.
*
* @retval GT_OK                     - on success
* @retval GT_BAD_PTR                - on NULL-pointer parameter. 
* @retval GT_NOT_FOUND              - the entry with non-zero counter not found 
*/
GT_STATUS cpssAppUtilsGenExtendedEventCounterEntryGetNext
(
    IN  GT_U8                           devNum,
    IN  GT_BOOL                         getFirst,
    IN  CPSS_UNI_EV_CAUSE_ENT           uniEvCounter,
    IN  GT_U32                          evExtData,
    OUT CPSS_UNI_EV_CAUSE_ENT           *uniEvCounterPtr,
    OUT GT_U32                          *evExtDataPtr,
    OUT GT_U32                          *counterPtr
)
{
    CPSS_UNI_EV_CAUSE_ENT uniEvCurrentCounter;
    GT_U32                extData;
    APP_UTILS_UNI_EVENT_COUNTER_STC *currentDbEntryPtr;

    if(uniEvCounterPtr == NULL || evExtDataPtr == NULL || counterPtr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PTR, LOG_ERROR_NO_MSG);
    }

    /* Initialize output parameters with values of input parameters and zeroes counter */
    *uniEvCounterPtr = uniEvCounter;
    *evExtDataPtr = evExtData;
    *counterPtr = 0;
    currentDbEntryPtr = NULL;

    if (getFirst) 
    {
        /* Starts from the begining */
        uniEvCurrentCounter = CPSS_PP_UNI_EV_MIN_E;
        extData = 0;
    }
    else
    {
        uniEvCurrentCounter = uniEvCounter;
        extData = evExtData;
    }

    osMutexLock(appUtilsEventCounterUpdateLockMtx);

    for(; uniEvCurrentCounter < CPSS_PP_UNI_EV_MAX_E; uniEvCurrentCounter++)
    {
        /* Get event's root node  */
        cpssAppUtilsGenExtendedEventCounterEntryGet(devNum, uniEvCurrentCounter, &currentDbEntryPtr);

        /* Look-up for extended data */
        while (currentDbEntryPtr)
        {
            if (currentDbEntryPtr->extData >= extData)
            {
                /* The eztended data is equal or greater than input data */
                if (currentDbEntryPtr->counterValue != 0)
                {
                    /* Non-zero counter was found */
                    *uniEvCounterPtr = uniEvCurrentCounter;
                    *evExtDataPtr = currentDbEntryPtr->extData;
                    *counterPtr = currentDbEntryPtr->counterValue;
                    return GT_OK;
                }
            }
            /* Skip to the next event's entry */
            cpssAppUtilsGenExtendedEventCounterEntryGet(devNum, uniEvCurrentCounter, &currentDbEntryPtr);
        }
        /* Skip to the next event */
        currentDbEntryPtr = NULL;
    }

    osMutexUnlock(appUtilsEventCounterUpdateLockMtx);

    /* Reached the end of the table - the entry not found */
    return GT_NOT_FOUND;
}

/**
* @internal cpssAppUtilsGenExtendedEventCountersClearAll function
* @endinternal
*
* @brief  Clears all event table counters.
*
* @param[in] devNum                 - device number
*
*/
GT_VOID cpssAppUtilsGenExtendedEventCountersClearAll
(
    IN  GT_U8                           devNum
)
{
    CPSS_UNI_EV_CAUSE_ENT           uniEvCounter;
    APP_UTILS_UNI_EVENT_COUNTER_STC *currentEntryPtr;

    osMutexLock(appUtilsEventCounterUpdateLockMtx);

    for(uniEvCounter = CPSS_PP_UNI_EV_MIN_E; uniEvCounter < CPSS_PP_UNI_EV_MAX_E; uniEvCounter++)
    {
        /* Clear counter per event type and all extended data */
        currentEntryPtr = &appUtilsEventCounters[devNum][uniEvCounter];

        while(currentEntryPtr)
        {
            currentEntryPtr->counterValue = 0;
            currentEntryPtr = currentEntryPtr->nextEntryPtr;
        }
    }

    osMutexUnlock(appUtilsEventCounterUpdateLockMtx);
}

GT_U32 wrapCpssTraceEventsEnable
(
    IN GT_U32 enable
)
{
    GT_U32 tmp = wrapCpssTraceEvents;
    wrapCpssTraceEvents = enable;
    return tmp;
}

