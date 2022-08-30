/**
********************************************************************************
* @file fdbLearning.c
*
* @brief This file includes the fdb learning process code for multi process appDemo.
*
* @version   10
********************************************************************************
*/
/********************************************************************************
* fdbLearning.c
*
* DESCRIPTION:
*       This file includes the fdb learning process code for multi process appDemo.
*
* DEPENDENCIES:
*
*
*
* COMMENTS:
*		The process includes :
* 			- event handler for FDB learning  events
*					CPSS_PP_EB_AUQ_PENDING_E,
*					CPSS_PP_MAC_MESSAGE_TO_CPU_READY_E
*
*			- event treatment includes the following steps:
*				> read AU message
*				> fill FDB entry according the AU message
*    			> add entry to FDB table
*
* FILE REVISION NUMBER:
*       $Revision: 10 $
*******************************************************************************/

#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <gtOs/gtOsSharedMalloc.h>
#include <gtOs/gtOsSharedMemory.h>
#include <gtOs/gtOsMem.h>
#include <gtOs/gtOsIo.h>
#include <gtOs/gtOsInit.h>

#include <cpss/generic/events/cpssGenEventRequests.h>
#include <cpss/generic/events/private/prvCpssGenEvReq.h>
#include <gtExtDrv/drivers/gtIntDrv.h>
#include <cpss/generic/config/private/prvCpssConfigTypes.h>
#include <cpssCommon/cpssPresteraDefs.h>
#include <extUtils/auEventHandler/auEventHandler.h>
#include <gtOs/gtOsSharedFunctionPointers.h>
#include <gtOs/gtOsSharedUtil.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>


#if (defined CHX_FAMILY)
    #define DXCH_CODE
#endif /* (defined CHX_FAMILY) */

#if defined CHX_FAMILY
#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgFdb.h>
#endif

#include <gtOs/gtOsSharedPp.h>

#include <fdbLearning.h>

static GT_UINTPTR auEventHanlerHnd = 0;


typedef GT_STATUS fdbAuMsgBlockGetFunc(IN    GT_U8 devNum,
                                       INOUT GT_U32                      *numOfAuPtr,
                                       OUT   CPSS_MAC_UPDATE_MSG_EXT_STC *auMessagesPtr);

typedef GT_STATUS fdbEntrySetFunc(IN GT_U8                   devNum,
                                  IN CPSS_MAC_ENTRY_EXT_STC *fdbEntryPtr);

typedef GT_STATUS fdbFromCpuAuMsgStatusGetFunc(IN  GT_U8    devNum,
                                               OUT GT_BOOL  *completedPtr,
                                               OUT GT_BOOL  *succeededPtr,
                                               INOUT GT_U32 *entryOffsetPtr);

static fdbAuMsgBlockGetFunc          *cpssEnFdbBrgFdbAuMsgBlockGet = NULL;
static fdbEntrySetFunc               *cpssEnFdbBrgFdbEntrySetFunc;
static fdbFromCpuAuMsgStatusGetFunc  *cpssEnFdbFromCpuAuMsgStatusGetFunc;

static  int showLogs = 0;
GT_BOOL multiProcessAppDemo = GT_TRUE;


#define AU_SIZE      10



/**
* @internal fdbLearningEventTreat function
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
static GT_STATUS fdbLearningEventTreat
(
    GT_U8                   devNum,
    GT_U32                  uniEv,
    GT_U32                  evExtData
)
{
    GT_STATUS   rc = GT_OK;

    switch (uniEv)
    {
        case CPSS_PP_MAC_MESSAGE_TO_CPU_READY_E:
        case CPSS_PP_EB_AUQ_PENDING_E:
            /* implemented if defined DXCH_CODE || defined PM_CODE */
            rc = fdbLearningAuMsgGet(devNum, evExtData);
            if (GT_OK != rc)
            {
                DBG_LOG(("fdbLearningAuMsgGet: error, devNum=%d, uniEv=%d, rc=%d\n", devNum, uniEv, rc, 4, 5, 6));
                if (showLogs == 1) {
                    osPrintf("fdbLearningAuMsgGet: error, devNum=%d, uniEv=%d, rc=%d\n", devNum, uniEv, rc);
                }
            }
            break;
        default:
            break;
    }

    return rc;
}

/**
* @internal fdbLearningAuMsgGet function
* @endinternal
*
* @brief   This routine handles Address Update messages.
*
* @param[in] devNum                   - the device number.
* @param[in] evExtData                - Unified event additional information
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
static GT_STATUS fdbLearningAuMsgGet
(
    IN GT_U8                devNum,
    GT_U32                  evExtData
)
{
    GT_STATUS   rc;

    rc = fdbLearningAuMsgHandle(devNum,
                                  evExtData);


    return rc;
}


#if defined PM_CODE
/**
* @internal prvFdbEntryConvertToExMxPmFormat function
* @endinternal
*
* @brief   The function converts FDB entry from general format to EXMXPM format.
*
* @note   APPLICABLE DEVICES:      ALL EXMXPM Devices
* @param[in] macEntryPtr              - pointer to FDB entry with general format
*
* @param[out] fdbEntryPtr              - pointer to FDB entry with EXMXPM format
*                                       None
*/
static GT_VOID prvFdbEntryConvertToExMxPmFormat
(
    IN  CPSS_MAC_ENTRY_EXT_STC      *macEntryPtr,
    OUT CPSS_EXMXPM_FDB_ENTRY_STC   *fdbEntryPtr
)
{

    switch(macEntryPtr->key.entryType)
    {
       case CPSS_MAC_ENTRY_EXT_TYPE_MAC_ADDR_E:
           fdbEntryPtr->key.entryType = CPSS_EXMXPM_FDB_ENTRY_TYPE_MAC_ADDR_E;
           break;
       case CPSS_MAC_ENTRY_EXT_TYPE_IPV4_MCAST_E:
           fdbEntryPtr->key.entryType = CPSS_EXMXPM_FDB_ENTRY_TYPE_IPV4_MCAST_E;
           break;
       case CPSS_MAC_ENTRY_EXT_TYPE_IPV6_MCAST_E:
           fdbEntryPtr->key.entryType = CPSS_EXMXPM_FDB_ENTRY_TYPE_IPV6_MCAST_E;
           break;
       default:
           break;
    }

    if((macEntryPtr->key.entryType == CPSS_EXMXPM_FDB_ENTRY_TYPE_IPV4_MCAST_E) ||
       (macEntryPtr->key.entryType == CPSS_EXMXPM_FDB_ENTRY_TYPE_IPV6_MCAST_E))
    {
        cpssOsMemCpy(fdbEntryPtr->key.key.ipMcast.sip,
                     macEntryPtr->key.key.ipMcast.sip, 4);
        cpssOsMemCpy(fdbEntryPtr->key.key.ipMcast.dip,
                     macEntryPtr->key.key.ipMcast.dip, 4);
        fdbEntryPtr->key.key.ipMcast.fId = macEntryPtr->key.key.ipMcast.vlanId;
    }
    else
    {
        cpssOsMemCpy(fdbEntryPtr->key.key.macFid.macAddr.arEther,
                     macEntryPtr->key.key.macVlan.macAddr.arEther, 6);

        fdbEntryPtr->key.key.macFid.fId = macEntryPtr->key.key.macVlan.vlanId;
    }

    osMemCpy(&(fdbEntryPtr->dstOutlif.interfaceInfo),
            &(macEntryPtr->dstInterface),
            sizeof(macEntryPtr->dstInterface));

    fdbEntryPtr->isStatic = macEntryPtr->isStatic;
    fdbEntryPtr->dstOutlif.outlifType = CPSS_EXMXPM_OUTLIF_TYPE_LL_E;

    switch(macEntryPtr->daCommand)
    {
       case CPSS_MAC_TABLE_FRWRD_E:
           fdbEntryPtr->daCommand = CPSS_PACKET_CMD_FORWARD_E;
           break;
       case CPSS_MAC_TABLE_DROP_E:
           fdbEntryPtr->daCommand = CPSS_PACKET_CMD_DROP_HARD_E;
           break;
       case CPSS_MAC_TABLE_INTERV_E:
       case CPSS_MAC_TABLE_CNTL_E:
           fdbEntryPtr->daCommand = CPSS_PACKET_CMD_TRAP_TO_CPU_E;
           break;
       case CPSS_MAC_TABLE_MIRROR_TO_CPU_E:
           fdbEntryPtr->daCommand = CPSS_PACKET_CMD_MIRROR_TO_CPU_E;
           break;
       case CPSS_MAC_TABLE_SOFT_DROP_E:
           fdbEntryPtr->daCommand = CPSS_PACKET_CMD_DROP_SOFT_E;
           break;
       default:
           break;
    }

    switch(macEntryPtr->saCommand)
    {
       case CPSS_MAC_TABLE_FRWRD_E:
           fdbEntryPtr->saCommand = CPSS_PACKET_CMD_FORWARD_E;
           break;
       case CPSS_MAC_TABLE_DROP_E:
           fdbEntryPtr->saCommand = CPSS_PACKET_CMD_DROP_HARD_E;
           break;
       case CPSS_MAC_TABLE_INTERV_E:
       case CPSS_MAC_TABLE_CNTL_E:
           fdbEntryPtr->saCommand = CPSS_PACKET_CMD_TRAP_TO_CPU_E;
           break;
       case CPSS_MAC_TABLE_MIRROR_TO_CPU_E:
           fdbEntryPtr->saCommand = CPSS_PACKET_CMD_MIRROR_TO_CPU_E;
           break;
       case CPSS_MAC_TABLE_SOFT_DROP_E:
           fdbEntryPtr->saCommand = CPSS_PACKET_CMD_DROP_SOFT_E;
           break;
       default:
           break;
    }

    fdbEntryPtr->daRoute = macEntryPtr->daRoute;
    fdbEntryPtr->mirrorToRxAnalyzerPortEn = macEntryPtr->mirrorToRxAnalyzerPortEn;
    fdbEntryPtr->sourceId = macEntryPtr->sourceID;
    fdbEntryPtr->userDefined = macEntryPtr->userDefined;
    fdbEntryPtr->daQosIndex = macEntryPtr->daQosIndex;
    fdbEntryPtr->saQosIndex = macEntryPtr->saQosIndex;
    fdbEntryPtr->daSecurityLevel = macEntryPtr->daSecurityLevel;
    fdbEntryPtr->saSecurityLevel = macEntryPtr->saSecurityLevel;
    fdbEntryPtr->appSpecificCpuCode = macEntryPtr->appSpecificCpuCode;
    fdbEntryPtr->spUnknown = GT_FALSE;

}

/**
* @internal prvExMxPmConvertToAuGenFormat function
* @endinternal
*
* @brief   The function converts EXMXPM AU message to general AU message format.
*
* @note   APPLICABLE DEVICES:      ALL EXMXPM Devices
* @param[in] auMessagesPtr            - pointer to AU message in EXMXPM format
*
* @param[out] auMessagesGenPtr         - pointer to AU message in general format
*                                       None
*/
static GT_VOID prvExMxPmConvertToAuGenFormat
(
    IN  CPSS_EXMXPM_FDB_UPDATE_MSG_ENTRY_STC  *auMessagesPtr,
    OUT CPSS_MAC_UPDATE_MSG_EXT_STC  *auMessagesGenPtr
)
{


    auMessagesGenPtr->updType         = auMessagesPtr->updType;
    auMessagesGenPtr->entryWasFound   = auMessagesPtr->entryWasFound;
    auMessagesGenPtr->macEntryIndex   = auMessagesPtr->fdbEntryIndex;

    switch(auMessagesPtr->fdbEntry.key.entryType)
    {
       case CPSS_EXMXPM_FDB_ENTRY_TYPE_MAC_ADDR_E:
           auMessagesGenPtr->macEntry.key.entryType = CPSS_MAC_ENTRY_EXT_TYPE_MAC_ADDR_E;
           break;
       case CPSS_EXMXPM_FDB_ENTRY_TYPE_IPV4_MCAST_E:
           auMessagesGenPtr->macEntry.key.entryType = CPSS_MAC_ENTRY_EXT_TYPE_IPV4_MCAST_E;
           break;
       case CPSS_EXMXPM_FDB_ENTRY_TYPE_IPV6_MCAST_E:
           auMessagesGenPtr->macEntry.key.entryType = CPSS_MAC_ENTRY_EXT_TYPE_IPV6_MCAST_E;
           break;
       default:
           break;
    }

    if((auMessagesPtr->fdbEntry.key.entryType ==
        CPSS_EXMXPM_FDB_ENTRY_TYPE_IPV4_MCAST_E) ||
       (auMessagesPtr->fdbEntry.key.entryType ==
         CPSS_EXMXPM_FDB_ENTRY_TYPE_IPV6_MCAST_E))
    {
        cpssOsMemCpy(auMessagesGenPtr->macEntry.key.key.ipMcast.sip,
                     auMessagesPtr->fdbEntry.key.key.ipMcast.sip, 4);
        cpssOsMemCpy(auMessagesGenPtr->macEntry.key.key.ipMcast.dip,
                     auMessagesPtr->fdbEntry.key.key.ipMcast.dip, 4);
        auMessagesGenPtr->macEntry.key.key.ipMcast.vlanId =
             (GT_U16)auMessagesPtr->fdbEntry.key.key.ipMcast.fId;
    }
    else
    {
        cpssOsMemCpy(auMessagesGenPtr->macEntry.key.key.macVlan.macAddr.arEther,
                     auMessagesPtr->fdbEntry.key.key.macFid.macAddr.arEther, 6);

        auMessagesGenPtr->macEntry.key.key.macVlan.vlanId =
            (GT_U16)auMessagesPtr->fdbEntry.key.key.macFid.fId;
    }



    osMemCpy(&(auMessagesGenPtr->macEntry.dstInterface),
             &(auMessagesPtr->fdbEntry.dstOutlif.interfaceInfo),
            sizeof(auMessagesPtr->fdbEntry.dstOutlif.interfaceInfo));



    auMessagesGenPtr->macEntry.isStatic = auMessagesPtr->fdbEntry.isStatic;

    switch(auMessagesPtr->fdbEntry.daCommand)
    {
       case CPSS_PACKET_CMD_FORWARD_E:
           auMessagesGenPtr->macEntry.daCommand = CPSS_MAC_TABLE_FRWRD_E;
           break;
       case CPSS_PACKET_CMD_MIRROR_TO_CPU_E:
           auMessagesGenPtr->macEntry.daCommand = CPSS_MAC_TABLE_MIRROR_TO_CPU_E;
           break;
       case CPSS_PACKET_CMD_TRAP_TO_CPU_E:
           auMessagesGenPtr->macEntry.daCommand = CPSS_MAC_TABLE_CNTL_E;
           break;
       case CPSS_PACKET_CMD_DROP_HARD_E:
           auMessagesGenPtr->macEntry.daCommand = CPSS_MAC_TABLE_DROP_E;
           break;
       case CPSS_PACKET_CMD_DROP_SOFT_E:
           auMessagesGenPtr->macEntry.daCommand = CPSS_MAC_TABLE_SOFT_DROP_E;
           break;
       default:
           break;
    }


    switch(auMessagesPtr->fdbEntry.saCommand)
    {
       case CPSS_PACKET_CMD_FORWARD_E:
           auMessagesGenPtr->macEntry.saCommand = CPSS_MAC_TABLE_FRWRD_E;
           break;
       case CPSS_PACKET_CMD_MIRROR_TO_CPU_E:
           auMessagesGenPtr->macEntry.saCommand = CPSS_MAC_TABLE_MIRROR_TO_CPU_E;
           break;
       case CPSS_PACKET_CMD_TRAP_TO_CPU_E:
           auMessagesGenPtr->macEntry.saCommand = CPSS_MAC_TABLE_CNTL_E;
           break;
       case CPSS_PACKET_CMD_DROP_HARD_E:
           auMessagesGenPtr->macEntry.saCommand = CPSS_MAC_TABLE_DROP_E;
           break;
       case CPSS_PACKET_CMD_DROP_SOFT_E:
           auMessagesGenPtr->macEntry.saCommand = CPSS_MAC_TABLE_SOFT_DROP_E;
           break;
       default:
           break;
    }


    auMessagesGenPtr->macEntry.daRoute = auMessagesPtr->fdbEntry.daRoute;
    auMessagesGenPtr->macEntry.mirrorToRxAnalyzerPortEn =
        auMessagesPtr->fdbEntry.mirrorToRxAnalyzerPortEn;
    auMessagesGenPtr->macEntry.sourceID = auMessagesPtr->fdbEntry.sourceId;
    auMessagesGenPtr->macEntry.userDefined = auMessagesPtr->fdbEntry.userDefined;
    auMessagesGenPtr->macEntry.daQosIndex = auMessagesPtr->fdbEntry.daQosIndex;
    auMessagesGenPtr->macEntry.saQosIndex = auMessagesPtr->fdbEntry.saQosIndex;
    auMessagesGenPtr->macEntry.daSecurityLevel =
        auMessagesPtr->fdbEntry.daSecurityLevel;
    auMessagesGenPtr->macEntry.saSecurityLevel =
        auMessagesPtr->fdbEntry.saSecurityLevel;
    auMessagesGenPtr->macEntry.appSpecificCpuCode =
        auMessagesPtr->fdbEntry.appSpecificCpuCode;

    auMessagesGenPtr->skip = auMessagesPtr->skip;
    auMessagesGenPtr->aging = auMessagesPtr->aging;
    auMessagesGenPtr->associatedHwDevNum = auMessagesPtr->associatedHwDevNum;
    auMessagesGenPtr->entryOffset = auMessagesPtr->entryOffset;


}

/**
* @internal prvExMxPmBrgFdbAuMsgBlockGet function
* @endinternal
*
* @brief   The function returns a block (array) of FDB Address Update (AU) messages,
*         the max number of elements defined by the caller.
*         The PP may place AU messages in common FDB Upload (FU) messages
*         queue or in separate queue only for AU messages. The function
*         cpssExMxPmHwPpPhase2Init configures queue for AU messages by the
*         fuqUseSeparate parameter. If common FU queue is used then function
*         returns all AU messages in the queue including FU ones.
*         If separate AU queue is used then function returns only AU messages.
*
* @note   APPLICABLE DEVICES:      ALL EXMXPM Devices
* @param[in] devNum                   - the device number from which AU are taken
* @param[in,out] numOfAuPtr               - (pointer to)max number of AU messages to get
* @param[in,out] numOfAuPtr               - (pointer to)actual number of AU messages that were received
*
* @param[out] auMessagesPtr            - array that holds received AU messages
*                                      pointer is allocated by the caller , with enough memory
*                                      for numOfAuPtr entries that was given as INPUT
*                                      parameter.
*
* @retval GT_OK                    - on success
* @retval GT_NO_MORE               - the action succeeded and there are no more waiting
*                                       AU messages
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS prvExMxPmBrgFdbAuMsgBlockGet
(
    IN      GT_U8                                    devNum,
    INOUT   GT_U32                                   *numOfAuPtr,
    OUT     CPSS_MAC_UPDATE_MSG_EXT_STC              *auMessagesPtr
)
{
    GT_U32 ii;
    CPSS_EXMXPM_FDB_UPDATE_MSG_ENTRY_STC  auPmMessagesArr[AU_SIZE];
    GT_STATUS rc;

    rc = cpssExMxPmBrgFdbAuMsgBlockGet(devNum, numOfAuPtr, auPmMessagesArr);
    if ((rc != GT_OK) && (rc != GT_NO_MORE))
    {
        return rc;
    }

    for (ii = 0; ii < *numOfAuPtr; ii++)
    {
       prvExMxPmConvertToAuGenFormat(&auPmMessagesArr[ii], &auMessagesPtr[ii]);

    }

    return rc;

}

/**
* @internal prvExMxPmBrgFdbEntrySet function
* @endinternal
*
* @brief   Create new or update existing entry in Hardware FDB table through
*         Address Update message.(AU message to the PP is non direct access to FDB
*         table).
*         The function use New Address message (NA) format.
*         The function checks that AU messaging is ready before using it.
*         The function does not check that AU message was processed by PP.
*
* @note   APPLICABLE DEVICES:      ALL EXMXPM Devices
* @param[in] devNum                   - device number
* @param[in] fdbEntryPtr              - pointer to FDB table entry
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - aging timeout exceeds hardware limitation.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_STATE             - the PP is not ready to get a message from CPU.
*                                       (PP still busy with previous message)
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS prvExMxPmBrgFdbEntrySet
(
    IN GT_U8                            devNum,
    IN CPSS_MAC_ENTRY_EXT_STC           *fdbEntryPtr
)
{
    CPSS_EXMXPM_FDB_ENTRY_STC   pmFdbEntry;

    prvFdbEntryConvertToExMxPmFormat(fdbEntryPtr, &pmFdbEntry);

    return cpssExMxPmBrgFdbEntrySet(devNum, &pmFdbEntry);
}

/**
* @internal prvExMxPmBrgFdbFromCpuAuMsgStatusGet function
* @endinternal
*
* @brief   Get status of FDB Address Update (AU) message processing in the PP.
*         The function checks the status of last AU message process:
*         completion and success.
*
* @note   APPLICABLE DEVICES:      ALL EXMXPM Devices
* @param[in] devNum                   - device number
*
* @param[out] completedPtr             - pointer to AU message processing completion.
*                                      GT_TRUE - AU message was processed by PP.
*                                      GT_FALSE - AU message processing is not completed yet by PP.
* @param[out] succeededPtr             - pointer to a success of AU operation.
*                                      GT_TRUE  - the AU action succeeded.
*                                      GT_FALSE - the AU action has failed.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or msgType
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS prvExMxPmBrgFdbFromCpuAuMsgStatusGet
(
    IN  GT_U8                   devNum,
    OUT GT_BOOL                 *completedPtr,
    OUT GT_BOOL                 *succeededPtr
)
{
    return cpssExMxPmBrgFdbFromCpuAuMsgStatusGet(devNum, CPSS_NA_E,
                                                 completedPtr, succeededPtr);

}

#endif


/**
* @internal fdbLearningAuMsgHandle function
* @endinternal
*
* @brief   This routine gets and handles the ChDx Address Update messages.
*
* @param[in] devNum                   - the device number.
* @param[in] evExtData                - Unified event additional information
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
static GT_STATUS fdbLearningAuMsgHandle
(
    IN GT_U8                devNum,
    GT_U32                  evExtData
)
{
    #define AU_SIZE      10

    GT_STATUS                   rc = GT_OK;
    GT_U32                      numOfAu;
    GT_U32                      auIndex;
    GT_BOOL                     completed;
    GT_BOOL                     succeeded;
    CPSS_MAC_UPDATE_MSG_EXT_STC bufferArr[AU_SIZE];
    CPSS_MAC_ENTRY_EXT_STC      cpssMacEntry;
    GT_BOOL                     isPrinted = GT_FALSE;

    GT_UNUSED_PARAM(evExtData);

#if defined DXCH_CODE
        cpssEnFdbBrgFdbAuMsgBlockGet       = cpssDxChBrgFdbAuMsgBlockGet;
        cpssEnFdbBrgFdbEntrySetFunc        = cpssDxChBrgFdbMacEntrySet;
        cpssEnFdbFromCpuAuMsgStatusGetFunc = cpssDxChBrgFdbFromCpuAuMsgStatusGet;
#endif

#if defined PM_CODE
        cpssEnFdbBrgFdbAuMsgBlockGet       = prvExMxPmBrgFdbAuMsgBlockGet;
		cpssEnFdbBrgFdbEntrySetFunc        = prvExMxPmBrgFdbEntrySet;
		cpssEnFdbFromCpuAuMsgStatusGetFunc = prvExMxPmBrgFdbFromCpuAuMsgStatusGet;
#endif

    numOfAu = AU_SIZE;

    while(rc != GT_NO_MORE)
    {
        /*************************************/
        /* get the AU message buffer from HW */
        /*************************************/

		 rc = cpssEnFdbBrgFdbAuMsgBlockGet(devNum, &numOfAu, bufferArr);
        if((rc != GT_NO_MORE) && (rc != GT_OK))
        {
            return rc;
        }

        /*********************************/
        /* handle all of the AU messages */
        /*********************************/

        for (auIndex = 0; auIndex < numOfAu; auIndex++)
        {
            switch (bufferArr[auIndex].updType)
            {
                case CPSS_NA_E:
                case CPSS_TA_E:
                {
                    osMemCpy(&cpssMacEntry, &bufferArr[auIndex].macEntry,
                                 sizeof(CPSS_MAC_ENTRY_EXT_STC));
                    cpssMacEntry.saCommand = CPSS_MAC_TABLE_FRWRD_E;


                        /* write data to cpss */

						rc = cpssEnFdbBrgFdbEntrySetFunc(devNum, &cpssMacEntry);
						if (rc == GT_OK && showLogs == 1 && isPrinted == GT_FALSE) {
                            osPrintf("cpssEventRecv: <dev=%d, mac-addr=%02x:%02x:%02x:%02x:%02x:%02x>\n",
                                     devNum,
                                     cpssMacEntry.key.key.macVlan.macAddr.arEther[0],
                                     cpssMacEntry.key.key.macVlan.macAddr.arEther[1],
                                     cpssMacEntry.key.key.macVlan.macAddr.arEther[2],
                                     cpssMacEntry.key.key.macVlan.macAddr.arEther[3],
                                     cpssMacEntry.key.key.macVlan.macAddr.arEther[4],
                                     cpssMacEntry.key.key.macVlan.macAddr.arEther[5]
                                     );
                            isPrinted = GT_TRUE;
                        }

                       if (rc == GT_BAD_PARAM || rc == GT_NOT_APPLICABLE_DEVICE)
                       {
                           /* These errors must be handled.
                            * GT_BAD_PARAM means "device doesn't exist"
                            */
                           continue;
                       }
					   if (rc != GT_OK)
					   {
						   return rc;
					   }
                        /* simulation task needs CPU time in order
                           to execute the process */
						#ifdef ASIC_SIMULATION
					   osTimerWkAfter(1);
						#endif

                       completed = GT_FALSE;
                       /* wait for the PP to process the action */
                       while(completed == GT_FALSE)
                       {
                    /* Check that the AU message processing has completed */

						   rc = cpssEnFdbFromCpuAuMsgStatusGetFunc(devNum,
                                                                    &completed,
                                                                    &succeeded,
                                                                    NULL);

                    if (rc != GT_OK)
                    {
                        return rc;
                    }
                       }

                       if (succeeded == GT_FALSE)
                       {
                           /* the action competed but without success !? */
                        return GT_FAIL;
                       }

                    break;

                }
                case CPSS_QA_E:
                case CPSS_QR_E:
                case CPSS_AA_E:
                case CPSS_SA_E:
                case CPSS_QI_E:
                case CPSS_FU_E:
                    break;
                default:
                    return GT_BAD_PARAM;
            }
            auEventHandlerDoCallbacks(auEventHanlerHnd,
                    (AU_EV_HANDLER_EV_TYPE_ENT)(bufferArr[auIndex].updType),
                    devNum, &bufferArr[auIndex]);
        }
    }

    return GT_OK;
}





/**
* @internal fdbEventRequestDrvnModeInit function
* @endinternal
*
* @brief   This routine :
*         - binds specified events
*         - unmask especified events
*         - run event handler
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS fdbEventRequestDrvnModeInit
(
    IN GT_VOID
)
{
    GT_STATUS                   rc;              /* The returned code            */
    GT_TASK                     eventHandlerTid; /* The task Tid                 */



    GT_U32 evHndlrCauseArrSize = sizeof(evHndlrCauseArr)/sizeof(evHndlrCauseArr[0]);
    static EV_HNDLR_PARAM taskParamArr;
    GT_BOOL cpssAslrSupport = cpssNonSharedGlobalVars.nonVolatileDb.generalVars.aslrSupport;
    CPSS_EXT_DRV_FUNC_BIND_STC  extDrvFuncBindInfo;
    CPSS_OS_FUNC_BIND_STC       osFuncBind;
    CPSS_TRACE_FUNC_BIND_STC    traceFuncBindInfo;

    if((GT_TRUE == cpssAslrSupport)&&(GT_FALSE == prvCpssGlobalDbExistGet()))
    {
      /*
                  Need to load shared memory and initialize callbacks.
      */

      /* Shared memory approach on ARM should use dlsym-based workaround for pointers to functions */
      rc = shrMemGetDefaultOsBindFuncsThruDynamicLoader(&osFuncBind);
      if (rc != GT_OK)
      {
        return GT_FAIL;
      }
      /* Shared memory approach on ARM should use dlsym-based workaround for pointers to functions */
      rc = shrMemGetDefaultExtDrvFuncs(&extDrvFuncBindInfo);
      if (rc != GT_OK)
      {
        return GT_FAIL;
      }

                  /* use NULL pointer to avoid trash stored by cpssExtServicesBind.
              The cpssExtServicesBind will use default "not implemented" callbacks in this case.  */
      osMemSet(&traceFuncBindInfo, 0, sizeof(traceFuncBindInfo));

      rc = cpssExtServicesBind(&extDrvFuncBindInfo, &osFuncBind, &traceFuncBindInfo);
      if(rc != GT_OK)
      {
          return rc;
      }

      rc = cpssGlobalDbInit(GT_TRUE,GT_FALSE);
      if(rc != GT_OK)
      {
          return rc;
      }
    }
    /* bind the events for all the App-Demo event handlers */
    taskParamArr.hndlrIndex = 0;

    /*in case event allready bound - unbind them */
    rc = cpssEventUnBind(evHndlrCauseArr,evHndlrCauseArrSize);
    if(rc != GT_OK)
    {
      return rc;
    }

    /* call CPSS to bind the events under single handler */
    rc = cpssEventBind(evHndlrCauseArr,
                       evHndlrCauseArrSize,
                       &taskParamArr.evHndl);
    switch(rc)
    {
        case GT_FAIL:
            printf("General failure\n");
            break;
        case GT_BAD_PTR:
            printf("One of the parameters is NULL pointer\n");
            break;
        case GT_OUT_OF_CPU_MEM:
            printf("Failed to allocate CPU memory\n");
            break;
        case GT_FULL:
            printf("When trying to set the 'tx buffer queue unify event' \
(CPSS_PP_TX_BUFFER_QUEUE_E) with other events in the same handler\n");
            break;
        case GT_ALREADY_EXIST:
            printf("One of the unified events already bound to another \
handler (not applicable to CPSS_PP_TX_BUFFER_QUEUE_E)\n");
            break;
    }

    if (GT_OK != rc)
    {
        return GT_FAIL;
    }

    /* call the CPSS to enable those interrupts in the HW of the device */
    rc = prvUniEvMaskAllSet(evHndlrCauseArr,
                            evHndlrCauseArrSize,
                            CPSS_EVENT_UNMASK_E);
    if (GT_OK != rc)
    {
        return GT_FAIL;
    }

    auEventHandlerLibInit();
    auEventHandlerInitHandler(
            AU_EV_HANDLER_DEVNUM_ALL,
            AU_EV_HANDLER_TYPE_ANY_E,
            &auEventHanlerHnd);
/* spawn all the event handler processes */
    rc = osTaskCreate("fdbEvHndl",
                      EV_HANDLER_MAX_PRIO,
                      _32KB,
                      fdbLearningEventsHndlr,
                      &taskParamArr,
                      &eventHandlerTid);
    if (rc != GT_OK)
    {
        return GT_FAIL;
    }

    return GT_OK;
}


/**
* @internal prvUniEvMaskAllSet function
* @endinternal
*
* @brief   This routine unmasks all the events according to the unified event list.
*
* @param[in] cpssUniEventArr[]        - The CPSS unified event list.
* @param[in] arrLength                - The unified event list length.
* @param[in] operation                - type of  mask/unmask to do on the events
*                                       GT_OK on success, or
*                                       GT_FAIL if failed.
*/
static GT_STATUS prvUniEvMaskAllSet
(
    IN  CPSS_UNI_EV_CAUSE_ENT       cpssUniEventArr[],
    IN  GT_U32                      arrLength,
    IN  CPSS_EVENT_MASK_SET_ENT     operation
)
{
    GT_STATUS rc = GT_OK;
    GT_U32    i;              /* Iterator                     */
    GT_U8     dev;            /* Device iterator              */
/*    GT_32     intKey;  */       /* The interrupt lock key       */

    /* lock section to disable interruption of ISR while unmasking events */
/*    intKey = 0;
    osTaskLock();
    extDrvSetIntLockUnlock(INTR_MODE_LOCK, &intKey); */

    /* unmask the interrupt */
    for (i = 0; i < arrLength; i++)
    {
        if (cpssUniEventArr[i] <= CPSS_PP_UNI_EV_MAX_E)
        {
            /* unmask the interrupt for all PPs */
            for (dev = 0; dev < _PRV_CPSS_MAX_PP_DEVICES_CNS; dev++)
            {
/*                if(appDemoPpConfigList[dev].valid == GT_FALSE)
                {
                    continue;
                } */

                rc = cpssEventDeviceMaskSet(/*appDemoPpConfigList[dev].devNum*/ dev, cpssUniEventArr[i], operation);
                if(rc != GT_OK)
                {
                    goto exit_cleanly_lbl;
                }
            }
        }
        else if (cpssUniEventArr[i] >= CPSS_XBAR_UNI_EV_MIN_E &&
                 cpssUniEventArr[i] <= CPSS_XBAR_UNI_EV_MAX_E)
        {
        }
        else if (cpssUniEventArr[i] >= CPSS_FA_UNI_EV_MIN_E &&
                 cpssUniEventArr[i] <= CPSS_FA_UNI_EV_MAX_E)
        {
        }
        else
        {
            rc = GT_FAIL;
            goto exit_cleanly_lbl;
        }
    }

exit_cleanly_lbl:
/*    extDrvSetIntLockUnlock(INTR_MODE_UNLOCK, &intKey);
    osTaskUnLock();*/

    return rc;
} /* prvUniEvMaskAllSet */

/*******************************************************************************
* fdbLearningEventsHndlr
*
* DESCRIPTION:
*       This routine is the event handler for PSS Event-Request-Driven mode
*       (polling mode).
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
static unsigned __TASKCONV fdbLearningEventsHndlr
(
    GT_VOID * param
)
{
    GT_STATUS           rc;                                         /* return code         */
    GT_U32              i;                                          /* iterator            */
    GT_UINTPTR          evHndl;                                     /* event handler       */
    GT_U32              evBitmapArr[CPSS_UNI_EV_BITMAP_SIZE_CNS];   /* event bitmap array  */
    GT_U32              evBitmap;                                   /* event bitmap 32 bit */
    GT_U32              evExtData;                                  /* event extended data */
    GT_U8               devNum;                                     /* device number       */
    GT_U32              uniEv;                                      /* unified event cause */
    GT_U32              evCauseIdx;                                 /* event index         */
    EV_HNDLR_PARAM      *hndlrParamPtr;                             /* bind event array    */

    hndlrParamPtr = (EV_HNDLR_PARAM*)param;
    evHndl        = hndlrParamPtr->evHndl;

    while (1)
    {
        rc = cpssEventSelect(evHndl, NULL, evBitmapArr, (GT_U32)CPSS_UNI_EV_BITMAP_SIZE_CNS);
        if (GT_OK != rc)
        {
            /* If seems like this result is not unusual... */
            /* DBG_LOG(("cpssEventSelect: err\n", 1, 2, 3, 4, 5, 6));*/
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
                uniEv = (evCauseIdx << 5) + i;

                if ((rc=cpssEventRecv(evHndl, uniEv, &evExtData, &devNum)) == GT_OK)
                {
                    DBG_LOG(("cpssEventRecv: %08x <dev=%d, uniEv=%d(%s), extData=0x%0x>\n",
                             (GT_U32)hndlrParamPtr->hndlrIndex, devNum, uniEv,
                             (GT_U32)uniEvName[uniEv], evExtData, 6));
					rc = fdbLearningEventTreat(devNum, uniEv, evExtData);

				}
				else
                {
                    DBG_LOG(("cpssEventRecv: error %d\n", rc, 2, 3, 4, 5, 6));
                    if (showLogs == 1) {
                        osPrintf("cpssEventRecv: error %d\n", rc);
                    }
                }
            }
        }
    }
    return 0;
}

/**
* @internal entryPoint function
* @endinternal
*
* @brief   The main of the fdbLearning process :
*         - run event Handler
*/
static void entryPoint(void)
{
    if(fdbEventRequestDrvnModeInit() == GT_FAIL)
    {
        osStopEngine();
    }
}
/**
* @internal main function
* @endinternal
*
* @brief   The main of the fdbLearning process :
*         - init of CPSS Shared Lib
*         - start mainOs engine
*/
int main(int argc, const char * argv[])
{
    GT_32 i;
    GT_BOOL aslrSupport = OS_ASLR_SUPPORT_DEFAULT;

    for(i=1;i<argc;i++)
    {
        if (strcmp(argv[i], "showLog") == 0)
        {
            showLogs = 1;
        }

       if (strcmp("-noaslr", argv[i]) == 0)
       {
            aslrSupport = GT_FALSE;
       }
    }

    if(shrMemInitSharedLibrary(aslrSupport) != GT_OK)
    {
        return -1;
    }
    shrMemPrintMapsDebugInfo();




    if (osStartEngine(argc, argv, "fdbLearning", entryPoint) != GT_OK)
        return 1;

    return 0;
}

GT_STATUS interruptEthPortRxSR
(
    IN GT_U8_PTR        segmentList[],
    IN GT_U32           segmentLen[],
    IN GT_U32           numOfSegments,
    IN GT_U32           rxQueue
)
{
    GT_UNUSED_PARAM(segmentList);
    GT_UNUSED_PARAM(segmentLen);
    GT_UNUSED_PARAM(numOfSegments);
    GT_UNUSED_PARAM(rxQueue);

    return 0;
}

GT_STATUS interruptEthPortTxEndSR
(
    IN GT_U8_PTR        segmentList[],
    IN GT_U32           numOfSegments
)
{
    GT_UNUSED_PARAM(segmentList);
    GT_UNUSED_PARAM(numOfSegments);
    return 0;
}

#if defined ASIC_SIMULATION

GT_STATUS   osShellExecute
(
    IN  char*   command
)
{
    command = command;
    return GT_NOT_IMPLEMENTED;
}

#endif


