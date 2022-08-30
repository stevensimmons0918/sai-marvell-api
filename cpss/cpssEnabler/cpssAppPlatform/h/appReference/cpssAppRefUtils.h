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
* @file cpssAppRefUtils.h
*
* @brief .
*
* @version   1
********************************************************************************
*/

#ifndef __CPSS_APP_REF_UTILS_H
#define __CPSS_APP_REF_UTILS_H


#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/diag/cpssDxChDiagDataIntegrity.h>
#include <cpss/common/diag/cpssCommonDiag.h>
#include <cpss/dxCh/dxChxGen/tcam/cpssDxChTcam.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortTxDba.h>

#include <gtOs/gtOsTask.h>
#include <gtOs/gtOsSem.h>

#include <cpssDriver/pp/prvCpssDrvPpDefs.h>
#include <cpssDriver/pp/hardware/prvCpssDrvObj.h>

#include <extUtils/common/cpssEnablerUtils.h>

#include <cpssAppPlatformSysConfig.h>
#include <cpssAppPlatformRunTimeConfig.h>
#include <cpssAppPlatformPpConfig.h>

/* commander compile-time configuration macros */
#define CMD_MAX_ARGS    256     /* maximum # of arguments       */
#define CMD_MAX_FIELDS  256     /* maximum # of fields          */

/***** Constants and Enumerations **************************************/
#define CMD_MAX_BUFFER  4096    /* maximum input buffer size    */

/*******************************************************************************
* RX_PACKET_RECEIVE_CB_FUN
*
* DESCRIPTION:
*       Function called to handle incoming Rx packet in the CPU
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* INPUTS:
*       devNum       - Device number.
*       queueIdx     - The queue from which this packet was received.
*       numOfBuffPtr - Num of used buffs in packetBuffs.
*       packetBuffs  - The received packet buffers list.
*       buffLen      - List of buffer lengths for packetBuffs.
*       rxParamsPtr  - (pointer to)information parameters of received packets
*
* RETURNS:
*       GT_OK - no error
*
* COMMENTS:
*
*******************************************************************************/
typedef GT_STATUS (*RX_PACKET_RECEIVE_CB_FUN)
(
    IN GT_U8            devNum,
    IN GT_U8            queueIdx,
    IN GT_U32           numOfBuff,
    IN GT_U8            *packetBuffs[],
    IN GT_U32           buffLen[],
    IN GT_VOID          *rxParamsPtr
);

extern GT_BOOL useDebugDeviceId;

extern GT_STATUS
cpssAppPlatformSysPpAdd
(
   IN GT_CHAR  *profileName
);

extern GT_STATUS
cpssAppPlatformSysPpRemove
(
    IN GT_U8                         devNum,
    IN GT_U8                         removalType
);

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
GT_STATUS   appRefAllowProcessingOfAuqMessages
(
    GT_BOOL     enable
);

/**
* @internal appRefDebugDeviceIdReset function
* @endinternal
*
* @brief   Reverts to HW device ID value for all devices.
*         Should be called before cpssInitSystem().
*
* @retval GT_OK                    - on success
*/
GT_STATUS appRefDebugDeviceIdReset
(
    IN GT_VOID
);

/**
* @internal appRefDxChPhase1ParamsGet function
* @endinternal
*
* @brief   Gets the parameters used during phase 1 init.
*
* @param[out] ppPhase1ParamsPtr        - (pointer to) parameters used during phase 1 init.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS appRefDxChPhase1ParamsGet
(
    OUT CPSS_DXCH_PP_PHASE1_INIT_INFO_STC *ppPhase1ParamsPtr
);

GT_STATUS appRefAldrinPortGobalResourcesPrint
(
    IN GT_U8 dev
);

/**
* @internal localUtfInitRef function
* @endinternal
*
* @brief   init the UTF
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS localUtfInitRef
(
    IN GT_U8 devNum
);

/**
* @internal appRefDxChDiagDataIntegrityEventCounterIncrement function
* @endinternal
*
* @brief   Function for increment counter per data integrity event
*
* @param[in] devNum                   - device number
* @param[in] eventPtr                 - (pointer to) data integrity event structure
*
* @retval CMD_OK                   - on success.
* @retval GT_BAD_PARAM             - on wrong values of input parameters.
* @retval GT_OUT_OF_CPU_MEM        - on out of CPU memory
*
* @note First call is init DB, eventPtr = NULL
*
*/

GT_STATUS appRefDxChDiagDataIntegrityEventCounterIncrement
(
    IN  GT_U8                                       devNum,
    IN  CPSS_DXCH_DIAG_DATA_INTEGRITY_EVENT_STC     *eventPtr
);

/**
* @internal appRefDxChDiagDataIntegrityEventTableGetFirst function
* @endinternal
*
* @brief   get first data integrity event counter entry.
*
* @retval CMD_OK                   - on success.
* @retval CMD_AGENT_ERROR          - on failure.
* @retval CMD_FIELD_UNDERFLOW      - not enough field arguments.
* @retval CMD_FIELD_OVERFLOW       - too many field arguments.
*
* @note none
*
*/
GT_STATUS appRefDxChDiagDataIntegrityEventTableGetFirst
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
);

/**
* @internal appRefDxChDiagDataIntegrityEventTableGetNext function
* @endinternal
*
* @brief   get next data integrity event counter entry.
*
* @retval CMD_OK                   - on success.
* @retval CMD_AGENT_ERROR          - on failure.
* @retval CMD_FIELD_UNDERFLOW      - not enough field arguments.
* @retval CMD_FIELD_OVERFLOW       - too many field arguments.
*
* @note none
*
*/
GT_STATUS appRefDxChDiagDataIntegrityEventTableGetNext
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
);

/**
* @internal appRefDxChDiagDataIntegrityEventTableClear function
* @endinternal
*
* @brief   clear all entries in event counter entry block.
*
* @retval CMD_OK                   - on success.
* @retval CMD_AGENT_ERROR          - on failure.
* @retval CMD_FIELD_UNDERFLOW      - not enough field arguments.
* @retval CMD_FIELD_OVERFLOW       - too many field arguments.
*
* @note none
*
*/
GT_STATUS appRefDxChDiagDataIntegrityEventTableClear
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
);

/**
* @internal cpssAppRefEventFuncInit function
* @endinternal
*
* @brief   init rx, au, data Integrity CBs used by UT.
*          create serdes training task.
*
* @param[in] devNum    - device number.
*
* @retval GT_OK        - on success,
* @retval GT_FAIL      - otherwise.
*
*/
GT_STATUS cpssAppRefEventFuncInit
(
    GT_U8  devNum
);

/**
* @internal appRefDxChNewTtiTcamSupportSet function
* @endinternal
*
* @brief   set the useAppOffset for new TTI TCAM mode.
*
* @param[in] useAppOffset         - whether to take the TTI offset in TCAM into consideration
*/
GT_VOID appRefDxChNewTtiTcamSupportSet
(
    IN  GT_BOOL             useAppOffset
);

/**
* @internal appRefDxChNewTtiTcamSupportGet function
* @endinternal
*
* @brief   get the utilNewTtiTcamSupportEnable for new TTI TCAM mode.
*
* @param[out] useAppBase           - (pointer to) whether to take the TTI offset in TCAM
*                                      into consideration
*
* @retval GT_OK                    - OK
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS appRefDxChNewTtiTcamSupportGet
(
    OUT GT_BOOL *useAppBase
);

/**
* @internal appRefDxChNewPclTcamSupportSet function
* @endinternal
*
* @brief   set the appRefPclTcamUseIndexConversion for new PCL TCAM mode.
*
* @param[in] enableIndexConversion    - whether to convert PCL TCAM index
*/
GT_VOID appRefDxChNewPclTcamSupportSet
(
    IN  GT_BOOL             enableIndexConversion
);

/**
* @internal appRefDxChNewPclTcamSupportGet function
* @endinternal
*
* @brief   get the appRefPclTcamUseIndexConversion for new PCL TCAM mode.
*
* @param[out] enableIndexConversion    - (pointer to) whether to convert PCL TCAM index
*
* @retval GT_OK                    - OK
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS appRefDxChNewPclTcamSupportGet
(
    OUT GT_BOOL *enableIndexConversion
);

/**
* @internal appRefDxChTcamTtiConvertedIndexGet function
* @endinternal
*
* @brief   Gets TCAM converted index for client TTI
*
* @param[in] devNum                   - device number
* @param[in] index                    - global line  in TCAM to write to.
*                                       The converted rule index.
*/
GT_U32 appRefDxChTcamTtiConvertedIndexGet
(
    IN     GT_U8                            devNum,
    IN     GT_U32                           index
);

/**
* @internal appRefDxChTcamClientBaseIndexGet function
* @endinternal
*
* @brief   Gets TCAM base index for Client rules
*
* @param[in] devNum                   - device number
* @param[in] hitNum                   - hit index, applicable range depends on device
*                                       The converted rule index.
*/
GT_U32 appRefDxChTcamClientBaseIndexGet
(
    IN     GT_U8                            devNum,
    IN     CPSS_DXCH_TCAM_CLIENT_ENT        client,
    IN     GT_U32                           hitNum
);

/**
* @internal appRefDxChTcamClientNumOfIndexsGet function
* @endinternal
*
* @brief   Gets TCAM number of indexes for Client rules
*
* @param[in] devNum                   - device number
* @param[in] hitNum                   - hit index, applicable range depends on device
*                                       TCAM number of indexes for TTI rules.
*/
 GT_U32 appRefDxChTcamClientNumOfIndexsGet
(
    IN     GT_U8                            devNum,
    IN     CPSS_DXCH_TCAM_CLIENT_ENT        client,
    IN     GT_U32                           hitNum
);

/**
* @internal appRefDxChTcamIpclBaseIndexGet function
* @endinternal
*
* @brief   Gets TCAM base index for IPCL0/1/2 rules
*
* @param[in] devNum                   - device number
* @param[in] lookupId                 - lookup 0/1/2
*                                       The converted rule index.
*/
GT_U32 appRefDxChTcamIpclBaseIndexGet
(
    IN     GT_U8                            devNum,
    IN     GT_U32                           lookupId
);

/**
* @internal appRefDxChTcamIpclNumOfIndexsGet function
* @endinternal
*
* @brief   Gets TCAM number of indexes for IPCL0/1/2 rules
*
* @param[in] devNum                   - device number
* @param[in] lookupId                 - lookup 0/1/2
*                                       TCAM number of indexes for IPCL0/1/2 rules.
*/
GT_U32 appRefDxChTcamIpclNumOfIndexsGet
(
    IN     GT_U8                            devNum,
    IN     GT_U32                           lookupId
);

/**
* @internal appRefDxChTcamEpclBaseIndexGet function
* @endinternal
*
* @brief   Gets TCAM base index for EPCL rules
*
* @param[in] devNum                   - device number
*                                       The converted rule index.
*/
GT_U32 appRefDxChTcamEpclBaseIndexGet
(
    IN     GT_U8                            devNum
);

/**
* @internal appRefDxChTcamEpclNumOfIndexsGet function
* @endinternal
*
* @brief   Gets TCAM number of indexes for EPCL rules
*
* @param[in] devNum                   - device number
*                                       TCAM number of indexes for EPCL rules.
*/
GT_U32 appRefDxChTcamEpclNumOfIndexsGet
(
    IN     GT_U8                            devNum
);

/**
* @internal appRefDxChTcamTtiBaseIndexGet function
* @endinternal
*
* @brief   Gets TCAM base index for TTI rules
*
* @param[in] devNum                   - device number
* @param[in] hitNum                   - hit index, applicable range depends on device
*                                       The converted rule index.
*/
GT_U32 appRefDxChTcamTtiBaseIndexGet
(
    IN     GT_U8                            devNum,
    IN     GT_U32                           hitNum
);

/**
* @internal appRefDxChTcamTtiNumOfIndexsGet function
* @endinternal
*
* @brief   Gets TCAM number of indexes for TTI rules
*
* @param[in] devNum                   - device number
* @param[in] hitNum                   - hit index, applicable range depends on device
*                                       TCAM number of indexes for TTI rules.
*/
GT_U32 appRefDxChTcamTtiNumOfIndexsGet
(
    IN     GT_U8                            devNum,
    IN     GT_U32                           hitNum
);

/* save TCAM 'Sections' (base+size) for TTI and IPCL/EPLC */
GT_VOID appRefDxChTcamSectionsSave(GT_VOID);

/* restore TCAM 'Sections' (base+size) for TTI and IPCL/EPLC */
GT_VOID appRefDxChTcamSectionsRestore(GT_VOID);

/**
* @internal appRefDxChTcamClientSectionSet function
* @endinternal
*
* @brief   Sets TCAM Section for Client rules : Base index + Num Of Indexes
*/
GT_STATUS appRefDxChTcamClientSectionSet
(
    IN     CPSS_DXCH_TCAM_CLIENT_ENT        client,
    IN     GT_U32                           hitNum,
    IN     GT_U32                           baseIndex,
    IN     GT_U32                           numOfIndexes
);

/**
* @internal pclConvertedIndexGet function
* @endinternal
*
* @brief   Gets TCAM converted index for client PCL
*
* @param[in] devNum                   - device number
* @param[in] index                    - global line  in TCAM to write to.
* @param[in] ruleSize                 - rule size in TCAM
*                                       The converted rule index.
*/
GT_U32 pclConvertedIndexGet
(
    IN     GT_U8                                devNum,
    IN     GT_U32                               index,
    IN     CPSS_DXCH_TCAM_RULE_SIZE_ENT         ruleSize,
    IN     GT_BOOL                              calledFromGaltisWrapper
);

/**
* @internal appRefDxChTcamPclConvertedIndexGet_fromUT function
* @endinternal
*
* @brief   Gets TCAM converted index for client PCL - from UT
*
* @param[in] devNum                   - device number
* @param[in] index                    - global line  in TCAM to write to.
* @param[in] ruleSize                 - rule size in TCAM
*                                       The converted rule index.
*/
GT_U32 appRefDxChTcamPclConvertedIndexGet_fromUT
(
    IN     GT_U8                                devNum,
    IN     GT_U32                               index,
    IN     CPSS_DXCH_TCAM_RULE_SIZE_ENT         ruleSize
);

GT_CHAR * CPSS_IF_2_STR
(
    CPSS_PORT_INTERFACE_MODE_ENT ifEnm
);

GT_CHAR * CPSS_SPEED_2_STR
(
    CPSS_PORT_SPEED_ENT speed
);

GT_STATUS prvAppDemoFdbManagerAuMsgHandlerMethod
(
     IN APP_DEMO_AU_MESSAGE_HANDLER_METHORD_ENT auMsgHandler
);

GT_STATUS prvAppDemoFdbManagerGlobalIDSet
(
    IN GT_U32 fdbManagerId
);

GT_STATUS appDemoFdbManagerAutoAgingStatisticsClear();

/**
* @internal appRefDxAldrin2TailDropDbaEnableSet function
* @endinternal
*
* @brief   Enables/disables Tail Drop DBA.
*          Relevant for Aldrin2 only, does nothing for other devices.
*          On first call saves the found DBA enable state for restore.
*
* @param[in] dev                      -   device number
* @param[in] enable                   -   enable (GT_TRUE) or disable (GT_FALSE) Tail Drop DBA
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS appRefDxAldrin2TailDropDbaEnableSet
(
    IN  GT_U8     dev,
    IN  GT_BOOL   enable
);

/**
* @internal appRefDxAldrin2TailDropDbaEnableRestore function
* @endinternal
*
* @brief   Restores Tail Drop DBA enable state. Relevant for Aldrin2 only, does nothing for other devices.
*
* @param[in] dev                      -   device number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS appRefDxAldrin2TailDropDbaEnableRestore
(
    IN  GT_U8     dev
);

/**
* @internal appRefEventsToTestsHandlerBind function
* @endinternal
*
* @brief   This routine creates new event handler for the unified event list.
*
* @param[in] cpssUniEventArr[]        - The CPSS unified event list.
* @param[in] arrLength                - The unified event list length.
* @param[in] operation                - type of  mask/unmask to do on the events
*                                       GT_OK on success, or
*                                       GT_FAIL if failed.
*/
GT_STATUS appRefEventsToTestsHandlerBind
(
    IN  CPSS_UNI_EV_CAUSE_ENT       cpssUniEventArr[],
    IN  GT_U32                      arrLength,
    IN  CPSS_EVENT_MASK_SET_ENT     operation
);

/**
* @internal appRefEventFatalErrorEnable function
* @endinternal
*
* @brief   Set fatal error handling type.
*
* @param[in] fatalErrorType           - fatal error handling type
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong fatalErrorType
*/
GT_STATUS appRefEventFatalErrorEnable
(
    CPSS_ENABLER_FATAL_ERROR_TYPE fatalErrorType
);

/**
* @internal appRefPrintPortEeeInterruptInfoSet function
* @endinternal
*
* @brief   function to allow set the flag of : printEeeInterruptInfo
*
* @param[in] enable                   - enable/disable the printings of ports 'EEE interrupts info'
*
* @retval GT_OK                    - on success
*/
GT_STATUS appRefPrintPortEeeInterruptInfoSet
(
    IN GT_U32   enable
);

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
);

GT_STATUS appRefFdbManagerGlobalIDSet
(
    IN GT_U32 fdbManagerId
);

GT_STATUS appRefFdbManagerControlSet
(
    IN APP_DEMO_AU_MESSAGE_HANDLER_METHORD_ENT  auMsgHandler,
    IN GT_BOOL                                  autoAgingEn,
    IN GT_U32                                   autoAgingInterval
);

GT_STATUS appRefFdbManagerAutoAgingStatisticsGet
(
    OUT APP_DEMO_CPSS_DXCH_BRG_FDB_MANAGER_STATISTICS_STC *statistics
);

GT_STATUS appRefFdbManagerAutoAgingStatisticsClear();

GT_STATUS appRefDxChPortMgrPortModeSpeedSet
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  GT_BOOL                         powerUp,
    IN  CPSS_PORT_INTERFACE_MODE_ENT    ifMode,
    IN  CPSS_PORT_SPEED_ENT             speed
);

GT_STATUS appRefPortManagerTaskSleepTimeSet(IN GT_U32 timeInMilli);

/**
* @internal appRefPrintLinkChangeFlagSet function
* @endinternal
*
* @brief   function to allow set the flag of : printLinkChangeEnabled
*
* @param[in] enable                   - enable/disable the printing of 'link change'
*
* @retval GT_OK                    - on success
*/
GT_STATUS appRefPrintLinkChangeFlagSet
(
    IN GT_U32   enable
);

/**
* @internal appRefPrintLinkChangeFlagGet function
* @endinternal
*
* @brief   function to get flag state of : printLinkChangeEnabled
*
*/
GT_BOOL appRefPrintLinkChangeFlagGet
(
    IN GT_VOID
);

/**
* @internal appRefEventsToTestsHandlerUnbind function
* @endinternal
*
* @brief   This routine unbind the unified event list and remove
*          the event handler made in appRefEventsToTestsHandlerBind .
*
* @param[in] cpssUniEventArr[]        - The CPSS unified event list.
* @param[in] arrLength                - The unified event list length.
*/
GT_STATUS appRefEventsToTestsHandlerUnbind
(
    IN  CPSS_UNI_EV_CAUSE_ENT       cpssUniEventArr[],
    IN  GT_U32                      arrLength
);

#endif /* __CPSS_APP_REF_UTILS_H*/
