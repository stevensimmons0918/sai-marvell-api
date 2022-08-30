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
* @file  cpssAppUtilsEvents.h
*
* @brief Includes common definitions and data-structures for unified events.
*
* @version   1
********************************************************************************
*/

#ifndef __cpssAppUtilsEvents_h
#define __cpssAppUtilsEvents_h

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @struct APP_UTILS_UNI_EVENT_COUNTER_STC
 *
 * @brief Used to store the event counter's info during event counter increment.
 */
typedef struct APP_UTILS_UNI_EVENT_COUNTER_STCT
{
    /** @brief event counter value */
    GT_U32 counterValue;

    /** @brief event extended data */
    GT_U32 extData;

    /** @brief pointer to the next counter entry */
    struct APP_UTILS_UNI_EVENT_COUNTER_STCT *nextEntryPtr;

}APP_UTILS_UNI_EVENT_COUNTER_STC;

#define APP_UTILS_MAX_INSTANCES_CNS     128

extern APP_UTILS_UNI_EVENT_COUNTER_STC  *appUtilsEventCounters[APP_UTILS_MAX_INSTANCES_CNS];
extern CPSS_OS_MUTEX appUtilsEventCounterUpdateLockMtx;

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
);

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
);

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
);

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
);

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
);

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
);

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
);

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
);

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
);

GT_U32 wrapCpssTraceEventsEnable
(
    IN GT_U32 enable
);

#ifdef __cplusplus
}
#endif

#endif  /* __cpssAppUtilsEvents_h */

