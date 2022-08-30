/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
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
* @file utfExtHelpers.h
*
* @brief Contains declaration of wrappers for calling all cpssEnabler functions - eg. appDemo/appRef used in mainUT
*
* @version
********************************************************************************
*/
#ifndef __utfExtraHelpersh
#define __utfExtraHelpersh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifdef CHX_FAMILY
#include<cpss/dxCh/dxChxGen/tcam/cpssDxChTcam.h>
#include<cpss/dxCh/dxChxGen/cpssHwInit/cpssDxChHwInit.h>
#include<cpss/dxCh/dxChxGen/config/cpssDxChCfgInit.h>
#include<cpss/dxCh/dxChxGen/lpm/cpssDxChLpmTypes.h>
#include<cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwInit.h>
#include<cpss/dxCh/dxChxGen/flowManager/cpssDxChFlowManager.h>

#include <ipFix/prvAppIpFix.h>
#endif

#include<cpss/generic/events/cpssGenEventUnifyTypes.h>
#include<cpss/generic/events/cpssGenEventRequests.h>
#include<cpss/extServices/trace/cpssTraceHw.h>
#include<cpss/common/port/cpssPortCtrl.h>

#include<extUtils/common/cpssEnablerUtils.h>

typedef GT_STATUS (*RX_PACKET_RECEIVE_CB_FUN)
(
    IN GT_U8                                devNum,
    IN GT_U8                                queueIdx,
    IN GT_U32                               numOfBuff,
    IN GT_U8                               *packetBuffs[],
    IN GT_U32                               buffLen[],
    IN void                                 *rxParamsPtr
);

/**
* @enum APP_DEMO_CPSS_LOG_MODE_ENT
 *
 * @brief This enum defines the different Modes of the log.
*/
typedef enum{

    /** print the logs on the screen */
    APP_DEMO_CPSS_LOG_MODE_PRINT_E,

    /** write the log into a file on the memFS */
    APP_DEMO_CPSS_LOG_MODE_MEMFS_FILE_E,

    /** write the log into a file on a local FS */
    APP_DEMO_CPSS_LOG_MODE_LOCAL_FILE_E

} APP_DEMO_CPSS_LOG_MODE_ENT;

#ifdef CPSS_APP_PLATFORM_REFERENCE
#include<appReference/cpssAppRefUtils.h>
extern GT_STATUS appRefDxChNetRxPacketCbRegister
(
    IN  RX_PACKET_RECEIVE_CB_FUN  rxPktReceiveCbFun
);
#else
extern GT_STATUS cpssInitSystem
(
    IN  GT_U32  boardIdx,
    IN  GT_U32  boardRevId,
    IN  GT_U32  reloadEeprom
);

extern GT_STATUS appDemoDbEntryAdd
(
    IN  GT_CHAR *namePtr,
    IN  GT_U32  value
);

extern GT_STATUS appDemoOsLogModeSet
(
    IN    APP_DEMO_CPSS_LOG_MODE_ENT      mode,
    IN    GT_CHAR_PTR                     name
);

#ifdef CHX_FAMILY
extern GT_STATUS appDemoDxChNetRxPacketCbRegister
(
    IN  RX_PACKET_RECEIVE_CB_FUN  rxPktReceiveCbFun
);

extern GT_STATUS appDemoDlbWaEnable
(
    GT_U8                               devNum,
    GT_BOOL                             waEnable,
    GT_U32                              newDelay
);

extern GT_STATUS appDemoPortManagerTaskSleepTimeSet(IN GT_U32 timeInMilli);

extern GT_STATUS appDemoLpmRamConfigSet
(
    IN CPSS_DXCH_LPM_RAM_CONFIG_INFO_STC    lpmRamConfigInfoArray[CPSS_DXCH_CFG_NUM_OF_DEV_TYPES_MANAGED_CNS],
    IN GT_U32                               lpmRamConfigInfoNumOfElements
);

extern GT_STATUS appDemoLpmRamConfigGet
(
    OUT CPSS_DXCH_LPM_RAM_CONFIG_INFO_STC    lpmRamConfigInfoArray[CPSS_DXCH_CFG_NUM_OF_DEV_TYPES_MANAGED_CNS],
    OUT GT_U32                               *lpmRamConfigInfoNumOfElementsPtr,
    OUT GT_BOOL                              *lpmRamConfigInfoSetFlagPtr
);

extern GT_STATUS appPlatformLpmRamConfigSet
(
    IN CPSS_DXCH_LPM_RAM_CONFIG_INFO_STC    lpmRamConfigInfoArray[CPSS_DXCH_CFG_NUM_OF_DEV_TYPES_MANAGED_CNS],
    IN GT_U32                               lpmRamConfigInfoNumOfElements
);

extern GT_STATUS appPlatformLpmRamConfigGet
(
    OUT CPSS_DXCH_LPM_RAM_CONFIG_INFO_STC    lpmRamConfigInfoArray[CPSS_DXCH_CFG_NUM_OF_DEV_TYPES_MANAGED_CNS],
    OUT GT_U32                               *lpmRamConfigInfoNumOfElementsPtr,
    OUT GT_BOOL                              *lpmRamConfigInfoSetFlagPtr
);
#endif
#endif

extern GT_CHAR * CPSS_IF_2_STR
(
    CPSS_PORT_INTERFACE_MODE_ENT ifEnm
);

extern GT_CHAR * CPSS_SPEED_2_STR
(
    CPSS_PORT_SPEED_ENT speed
);

/**
* @internal prvWrAppAllowProcessingOfAuqMessages function
* @endinternal
*
* @brief   function to allow set the flag of : allowProcessingOfAuqMessages
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
GT_STATUS   prvWrAppAllowProcessingOfAuqMessages
(
    GT_BOOL     enable
);

/**
* @internal prvWrAppDbEntryAdd function
* @endinternal
*
* @param[in] namePtr                  - points to parameter name
* @param[in] value                    - parameter value
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - if name is too long
* @retval GT_BAD_PTR               - if NULL pointer
* @retval GT_NO_RESOURCE           - if Database is full
*/
GT_STATUS prvWrAppDbEntryAdd
(
    IN  GT_CHAR *namePtr,
    IN  GT_U32  value
);

/**
* @internal prvWrAppDbEntryGet function
* @endinternal
*
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
GT_STATUS prvWrAppDbEntryGet
(
    IN  GT_CHAR *namePtr,
    OUT GT_U32  *valuePtr
);


/**
* @internal prvWrAppDebugDeviceIdReset function
* @endinternal
*
*
* @retval GT_OK                    - on success
*/
GT_STATUS prvWrAppDebugDeviceIdReset
(
    IN GT_VOID
);


/**
* @internal prvWrAppDebugDeviceIdSet function
* @endinternal
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_OUT_OF_RANGE          - on devIndex > 127
*/
GT_STATUS prvWrAppDebugDeviceIdSet
(
    IN GT_U8    devNum,
    IN GT_U32   deviceId
);


/**
* @internal prvWrAppDevIdxGet function
* @endinternal
*
*
* @param[in] devNum                   - SW device number
*
* @param[out] devIdxPtr                - (pointer to) device index in array "appDemoPpConfigList".
*
* @retval GT_OK                    - on success, match found for devNum.
* @retval GT_FAIL                  - no match found for devNum.
*/
GT_STATUS prvWrAppDevIdxGet
(
    IN  GT_U8   devNum,
    OUT GT_U32  *devIdxPtr
);

/**
* @internal prvWrAppEventsToTestsHandlerBind function
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
GT_STATUS prvWrAppEventsToTestsHandlerBind
(
    IN  CPSS_UNI_EV_CAUSE_ENT       cpssUniEventArr[],
    IN  GT_U32                      arrLength,
    IN  CPSS_EVENT_MASK_SET_ENT     operation
);

/**
* @internal prvWrAppSkipEventMaskSet function
* @endinternal
*
*
* @retval GT_OK                    - on success, 
* @retval GT_BAD_PARAM             - on bad parameter
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS prvWrAppSkipEventMaskSet
(
    IN GT_U8                       devNum,
    IN CPSS_EVENT_MASK_SET_ENT     operation
);

/**
* internal prvWrAppGenEventCounterGet function
* @endinternal
*
* @brief   get the number of times that specific event happened.
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
GT_STATUS prvWrAppGenEventCounterGet
(
    IN  GT_U8                    devNum,
    IN  CPSS_UNI_EV_CAUSE_ENT    uniEvent,
    IN  GT_BOOL                  clearOnRead,
    OUT GT_U32                  *counterPtr
);

/**
* @internal prvWrAppHwAccessCounterGet function
* @endinternal
*
* @brief   The function gets number of hw accesses
*
* @param[out] hwAccessCounterPtr       - points to hw access counter value.
*
* @retval GT_OK                    - on success
*/
GT_STATUS prvWrAppHwAccessCounterGet
(
    OUT GT_U32   *hwAccessCounterPtr
);

/**
* @internal prvWrAppHwAccessFailerBind function
* @endinternal
*
* @brief   The function binds/unbinds a WriteFailerCb callback for HW write access
*         and set hw write fail counter.
* @param[in] bind                     -  GT_TRUE -  callback routines.
*                                      GT_FALSE - un bind callback routines.
* @param[in] failCounter              - hw write fail counter: set hw write sequence
*                                      number on which write operation should fail.
*
* @retval GT_OK                    - on success
*/
GT_STATUS prvWrAppHwAccessFailerBind
(
    IN GT_BOOL                  bind,
    GT_U32                      failCounter
);

/**
* @internal prvWrAppOsLogModeSet function
* @endinternal
*
* @brief   Function for setting the mode of cpss log
*
* @param[in] mode                     -  to be set.
* @param[in] name                     - the  of the file.
*
* @retval GT_OK                    - if succsess
* @retval GT_BAD_PTR               - failed to open the file
*/
GT_STATUS prvWrAppOsLogModeSet
(
    IN    APP_DEMO_CPSS_LOG_MODE_ENT      mode,
    IN    GT_CHAR_PTR                     name
);

/**
* @internal prvWrAppOsLogStop function
* @endinternal
*
* @brief   Function for stop writing the logs into the file
*/
GT_VOID prvWrAppOsLogStop();

/**
* @internal prvWrAppStaticDbEntryGet function
* @endinternal
*
* @brief   Get parameter value from AppDemo 'static' database .
*         This DB is not reset by the 'system reset' mechanism.
* @param[in] namePtr                  - points to parameter name
*
* @param[out] valuePtr                 - points to parameter value
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - if name is too long
* @retval GT_BAD_PTR               - if NULL pointer
* @retval GT_NO_SUCH               - there is no such parameter in Database
*/
GT_STATUS prvWrAppStaticDbEntryGet
(
    IN  GT_CHAR *namePtr,
    OUT GT_U32  *valuePtr
);

/**
* @internal prvWrAppSupportAaMessageSet function
* @endinternal
*
* @brief   Configure the appDemo to support/not support the AA messages to CPU
*
* @param[in] supportAaMessage         -
*                                      GT_FALSE - not support processing of AA messages
*                                      GT_TRUE - support processing of AA messages
*                                       nothing
*/
GT_VOID prvWrAppSupportAaMessageSet
(
    IN GT_BOOL supportAaMessage
);

/**
* @internal prvWrAppTraceHwAccessClearDb function
* @endinternal
*
* @brief   Clear HW access db
*
* @retval GT_OK                    - on success else if failed
*/
GT_STATUS prvWrAppTraceHwAccessClearDb
(
    GT_VOID
);

/**
* @internal prvWrAppTraceHwAccessDbIsCorrupted function
* @endinternal
*
* @brief   Check if the HW access data base has reached the limit
*
* @retval GT_OK                    - on success else if failed
*/

GT_STATUS prvWrAppTraceHwAccessDbIsCorrupted
(
    GT_BOOL * corrupted
);

/**
* @internal prvWrAppTraceHwAccessEnable function
* @endinternal
*
* @brief   Trace HW read access information.
*
* @param[in] devNum                   - PP device number
* @param[in] accessType               - access type: read or write
* @param[in] enable                   - GT_TRUE:  tracing for given access type
*                                      GT_FALSE: disable tracing for given access type
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on fail
* @retval GT_BAD_PARAM             - on bad access type
*/
GT_STATUS prvWrAppTraceHwAccessEnable
(
    GT_U8                                  devNum,
    CPSS_ENABLER_TRACE_HW_ACCESS_TYPE_ENT      accessType,
    GT_BOOL                                enable
);

/**
* @internal prvWrAppTraceHwAccessInfoCompare function
* @endinternal
*
* @brief   Compare given data and parameters with trace hw access DB info on
*         specified index.
* @param[in] dev                      - device number
* @param[in] accessType               - access type: read or write
* @param[in] index                    -  in the hw access db
* @param[in] portGroupId              - port group ID
* @param[in] isrContext               - GT_TRUE: ISR contextt
*                                      GT_FALSE: TASK context
* @param[in] addr                     - memory address space
* @param[in] addr                     - addresses to compare
* @param[in] mask                     -  to  the data from the db
* @param[in] data                     - data (read/written) to compare
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - when there is no match between db info and input
*                                       parameters.
* @retval GT_BAD_STATE             - on unexpected ISR context
*/
GT_STATUS prvWrAppTraceHwAccessInfoCompare
(
    IN GT_U8                               dev,
    CPSS_ENABLER_TRACE_HW_ACCESS_TYPE_ENT      accessType,
    IN GT_U32                              index,
    IN GT_U32                              portGroupId,
    IN GT_BOOL                             isrContext,
    IN CPSS_TRACE_HW_ACCESS_ADDR_SPACE_ENT addrSpace,
    IN GT_U32                              addr,
    IN GT_U32                              mask,
    IN GT_U32                              data
);

/**
* @internal prvWrAppTraceHwAccessOutputModeSet function
* @endinternal
*
* @brief   Set output tracing mode.
*
* @param[in] mode                     - output tracing mode: print, printSync or store.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_STATE             - on bad state
* @retval GT_BAD_PARAM             - on bad mode
*
* @note It is not allowed to change mode, while one of the HW Access DB
*       is enabled.
*
*/
GT_STATUS prvWrAppTraceHwAccessOutputModeSet
(
    IN CPSS_ENABLER_TRACE_OUTPUT_MODE_ENT   mode
);

/**
* @internal prvWrAppEventFatalErrorEnable function
* @endinternal
*
* @brief   Set fatal error handling type.
*
* @param[in] fatalErrorType           - fatal error handling type
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong fatalErrorType
*/
GT_STATUS prvWrAppEventFatalErrorEnable
(
    CPSS_ENABLER_FATAL_ERROR_TYPE fatalErrorType
);

/**
* @internal prvWrAppInitSystemGet function
* @endinternal
*
* @brief   Function sets parameters of cpss init system proccess.
*
* @param[out] boardIdxPtr              - (pointer to) The index of the board to be initialized
*                                      from the board list.
* @param[out] boardRevIdPtr            - (pointer to) Board revision Id.
* @param[out] reloadEepromPtr          - (pointer to) Whether the Eeprom should be reloaded when
*                                      corePpHwStartInit() is called.
*                                       None.
*/
void prvWrAppInitSystemGet
(
    OUT GT_U32  *boardIdxPtr,
    OUT GT_U32  *boardRevIdPtr,
    OUT GT_U32  *reloadEepromPtr
);

/**
+* @internal prvWrAppIsHirApp function
+* @endinternal
*
* @brief   check appDemo is running in HIR mode.
*
* @retval GT_TRUE       - HIR mode
* @retval GT_FALSE      - non HIR mode
*/
GT_BOOL prvWrAppIsHirApp
(
    GT_VOID
);

/**
* @internal prvWrAppTrunkPhy1690_WA_B_Get function
* @endinternal
*
* @brief  indication for Galtis wrappers call the WA 'B' APIs.
*
* @retval 1        - trunk phy1690_WA 'B'APIs
* @retval 0        - otherwise
*/
GT_U32 prvWrAppTrunkPhy1690_WA_B_Get
(
    GT_VOID
);

/**
* @internal prvWrAppForceAutoLearn function
* @endinternal
*
* @brief  returns FDB Auto learn Status.
*
*/
GT_BOOL prvWrAppForceAutoLearn
(
    GT_VOID
);

/**
* @internal prvWrAppunifiedFdbPortGroupsBmpGet function
* @endinternal
*
* @brief  returns unified Fdb Port group Bitmap.
*
* @param[in] dev       - device number
* @param[in] portGroup - portGroup number
*
*/
GT_PORT_GROUPS_BMP prvWrAppunifiedFdbPortGroupsBmpGet
(
    IN GT_U8 dev,
    IN GT_U32 portGroup
);

/**
* @internal prvWrAppUplinkPortGroupPortsBmpGet function
* @endinternal
*
* @brief  returns uplink port Group Bitmap.
*
* @param[in] dev       - device number
*
* @param[out] portsBmp - pointer to uplink PortGroup PortsBmp.
*
* @retval GT_OK        - on success
* @retval GT_FAIL      - otherwise
*/
GT_STATUS prvWrAppUplinkPortGroupPortsBmpGet
(
    IN GT_U8 dev,
    OUT CPSS_PORTS_BMP_STC **portsBmp
);

/**
* @internal prvWrAppMultiPortGroupRingPortsBmpGet function
* @endinternal
*
* @brief  Get MutliPortGroup ring Ports Bitmap.
*
* @param[in] dev       - device number
*
* @param[out] portsBmp - pointer to MutliPortGroup ringPortsBmp.
*
* @retval GT_OK        - on success
* @retval GT_FAIL      - otherwise
*/
GT_STATUS prvWrAppMultiPortGroupRingPortsBmpGet
(
    IN GT_U8 dev,
    OUT CPSS_PORTS_BMP_STC **portsBmp
);

/**
* @internal prvWrAppRelayPortGroupPortsBmpGet function
* @endinternal
*
* @brief  returns relay ports Bitmap.
*
* @param[in] dev       - device number
*
* @param[out] portsBmp - pointer to relay PortsBmp.
*
* @retval GT_OK        - on success
* @retval GT_FAIL      - otherwise
*/
GT_STATUS prvWrAppRelayPortGroupPortsBmpGet
(
    IN GT_U8 dev,
    OUT CPSS_PORTS_BMP_STC **portsBmp
);

#ifdef CHX_FAMILY
GT_STATUS prvWrAppAldrinPortGobalResourcesPrint
(
    IN GT_U8 dev
);

GT_STATUS prvWrAppBobk2PortGobalResourcesPrint
(
    IN GT_U8 dev
);

GT_STATUS prvWrAppBobk2PortListResourcesPrint
(
    IN GT_U8 dev
);

/**
* @internal prvWrAppCaelumEgressCntrReset function
* @endinternal
*
* @brief   Function resets TxQ Egress counters on Caelum devices.
*
* @param[in] devNum                   - device number
* @param[in] cntrSetNum               - counter set number to be reset
*
* @retval GT_OK                    - on success,
* @retval GT_BAD_STATE             - traffic is still run in device. Cannot execute the WA.
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS prvWrAppCaelumEgressCntrReset
(
    IN GT_U8        devNum,
    IN GT_U32       cntrSetNum
);

/**
* @internal prvWrAppDxAldrin2TailDropDbaEnableRestore function
* @endinternal
*
*
* @param[in] dev                      -   device number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS prvWrAppDxAldrin2TailDropDbaEnableRestore
(
    IN  GT_U8     dev
);


/**
* @internal prvWrAppDxAldrin2TailDropDbaEnableSet function
* @endinternal
*
*
* @param[in] dev                      -   device number
* @param[in] enable                   -   enable (GT_TRUE) or disable (GT_FALSE) Tail Drop DBA
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS prvWrAppDxAldrin2TailDropDbaEnableSet
(
    IN  GT_U8     dev,
    IN  GT_BOOL   enable
);

/**
* @internal prvWrAppDxChNewTtiTcamSupportSet function
* @endinternal
*
*
* @param[in] useTTIOffset         - whether to take the TTI offset in TCAM into consideration
*/
GT_VOID prvWrAppDxChNewTtiTcamSupportSet
(
    IN  GT_BOOL             useTTIOffset
);

/**
* @internal prvWrAppBc2IpLpmRamDefaultConfigCalc function
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
GT_STATUS prvWrAppBc2IpLpmRamDefaultConfigCalc
(
    IN  GT_U8                                    devNum,
    IN  GT_U32                                   maxNumOfPbrEntries,
    OUT CPSS_DXCH_LPM_RAM_CONFIG_STC             *ramDbCfgPtr
);

/**
* @internal prvWrAppDxChPhase1ParamsGet function
* @endinternal
*
* @brief   Gets the parameters used during phase 1 init.
*
* @param[out] ppPhase1ParamsPtr        - (pointer to) parameters used during phase 1 init.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS prvWrAppDxChPhase1ParamsGet
(
    OUT CPSS_DXCH_PP_PHASE1_INIT_INFO_STC *ppPhase1ParamsPtr
);

/**
* @internal prvWrAppDxChTcamClientSectionSet function
* @endinternal
*
* @brief   Sets TCAM Section for Client rules : Base index + Num Of Indexes
*/
GT_STATUS prvWrAppDxChTcamClientSectionSet
(
    IN     CPSS_DXCH_TCAM_CLIENT_ENT        client,
    IN     GT_U32                           hitNum,
    IN     GT_U32                           baseIndex,
    IN     GT_U32                           numOfIndexes
);

/**
* @internal prvWrAppDxChTcamEpclBaseIndexGet function
* @endinternal
*
* @brief   Gets TCAM base index for EPCL rules
*
* @param[in] devNum                   - device number
*                                       The converted rule index.
*/
GT_U32 prvWrAppDxChTcamEpclBaseIndexGet
(
    IN     GT_U8                            devNum
);

/**
* @internal prvWrAppDxChTcamEpclNumOfIndexsGet function
* @endinternal
*
* @brief   Gets TCAM number of indexes for EPCL rules
*
* @param[in] devNum                   - device number
*                                       TCAM number of indexes for EPCL rules.
*/
GT_U32 prvWrAppDxChTcamEpclNumOfIndexsGet
(
    IN     GT_U8                            devNum
);

/**
* @internal prvWrAppDxChTcamIpclBaseIndexGet function
* @endinternal
*
* @brief   Gets TCAM base index for IPCL0/1/2 rules
*
* @param[in] devNum                   - device number
* @param[in] lookupId                 - lookup 0/1/2
*                                       The converted rule index.
*/
GT_U32 prvWrAppDxChTcamIpclBaseIndexGet
(
    IN     GT_U8                            devNum,
    IN     GT_U32                           lookupId
);

/**
* @internal prvWrAppDxChTcamIpclNumOfIndexsGet function
* @endinternal
*
* @brief   Gets TCAM number of indexes for IPCL0/1/2 rules
*
* @param[in] devNum                   - device number
* @param[in] lookupId                 - lookup 0/1/2
*                                       TCAM number of indexes for IPCL0/1/2 rules.
*/
GT_U32 prvWrAppDxChTcamIpclNumOfIndexsGet
(
    IN     GT_U8                            devNum,
    IN     GT_U32                           lookupId
);

GT_U32 prvWrAppDxChTcamPclConvertedIndexGet_fromUT
(
    IN     GT_U8                                devNum,
    IN     GT_U32                               index,
    IN     CPSS_DXCH_TCAM_RULE_SIZE_ENT         ruleSize
);

/* restore TCAM 'Sections' (base+size) for TTI and IPCL/EPLC */
GT_VOID prvWrAppDxChTcamSectionsRestore(GT_VOID);

/**
* @internal prvWrAppDxChTcamTtiBaseIndexGet function
* @endinternal
*
* @brief   Gets TCAM base index for TTI rules
*
* @param[in] devNum                   - device number
* @param[in] hitNum                   - hit index, applicable range depends on device
*                                       The converted rule index.
*/
GT_U32 prvWrAppDxChTcamTtiBaseIndexGet
(
    IN     GT_U8                            devNum,
    IN     GT_U32                           hitNum
);

/**
* @internal prvWrAppDxChTcamTtiConvertedIndexGet function
* @endinternal
*
* @brief   Gets TCAM converted index for client TTI
*
* @param[in] devNum                   - device number
* @param[in] index                    - global line  in TCAM to write to.
*                                       The converted rule index.
*/
GT_U32 prvWrAppDxChTcamTtiConvertedIndexGet
(
    IN     GT_U8                            devNum,
    IN     GT_U32                           index
);

/**
* @internal prvWrAppDxChTcamTtiNumOfIndexsGet function
* @endinternal
*
* @brief   Gets TCAM number of indexes for TTI rules
*
* @param[in] devNum                   - device number
* @param[in] hitNum                   - hit index, applicable range depends on device
*                                       TCAM number of indexes for TTI rules.
*/
GT_U32 prvWrAppDxChTcamTtiNumOfIndexsGet
(
    IN     GT_U8                            devNum,
    IN     GT_U32                           hitNum
);

/**
* @internal prvWrAppDxHwDevNumChange function
* @endinternal
*
* @brief   Implements HW devNum changes.
*         1. 'ownDevNum'
*         2. E2PHY update
*
* @param[out] devNum                   -   SW devNum
* @param[out] hwDevNum                 -   HW devNum
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               -  on bad pointer
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS prvWrAppDxHwDevNumChange
(
    IN  GT_U8                       devNum,
    IN  GT_HW_DEV_NUM               hwDevNum
);

/**
* @internal prvWrAppFalconIpLpmRamDefaultConfigCalc function
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
GT_STATUS prvWrAppFalconIpLpmRamDefaultConfigCalc
(
    IN  GT_U8                                    devNum,
    IN  CPSS_DXCH_CFG_SHARED_TABLE_MODE_ENT      sharedTableMode,
    IN  GT_U32                                   maxNumOfPbrEntries,
    OUT CPSS_DXCH_LPM_RAM_CONFIG_STC             *ramDbCfgPtr
);

/* is TM init required by initSystem */
GT_BOOL prvWrAppIsTmEnabled(void);

/*------------------------------------------------------------------------------------------------------------------------
 *  /Cider/EBU/Bobcat2B/Bobcat2 {Current}/Reset and Init Controller/DFX Server Units - BC2 specific registers/Device SAR2
 *  address : 0x000F8204
 *  15-17  PLL_2_Config (Read-Only) TM clock frequency
 *    0x0 = Disabled;  Disabled; TM clock is disabled
 *    0x1 = 400MHz;    TM runs 400MHz, DDR3 runs 800MHz
 *    0x2 = 466MHz;    TM runs 466MHz, DDR3 runs 933MHz
 *    0x3 = 333MHz;    TM runs 333MHz, DDR3 runs 667MHz
 *    0x5 = Reserved0; TM runs from core clock, DDR3 runs 800MHz.; (TM design does not support DDR and TM_core from different source)
 *    0x6 = Reserved1; TM runs from core clock, DDR3 runs 933MHz.; (TM design does not support DDR and TM_core from different source)
 *    0x7 = Reserved; Reserved; PLL bypass
 *
 *    if SAR2 - PLL2  is 0 (disable)  , do we need to configure TM ????
 *    is TM supported ? probably not !!!
 *------------------------------------------------------------------------------------------------------------------------*/
GT_STATUS prvWrAppIsTmSupported
(
    IN   GT_U8 devNum,
    OUT  GT_BOOL *isTmSupported
);

/**
* @internal prvWrAppLion2FabricCleanUp function
* @endinternal
*
* @brief   The function restores Lion2 device from all Lion2Fabric configurations.
*
* @param[in] lion2FabricDevNum        - Lion2 CPSS device number
*                                       GT_OK on success, others on fail.
*
* @note NONE
*
*/
GT_STATUS prvWrAppLion2FabricCleanUp
(
    IN GT_U8              lion2FabricDevNum
);

/**
* @internal prvWrAppLion2FabricConfigureQos function
* @endinternal
*
* @brief   The function configure Lion2Fabric for TC/DP support inside Lion2 device.
*         The eDSA Tag should remain unchanged.
* @param[in] lion2FabricDevNum        - Lion2 CPSS device number
* @param[in] toCpuTc                  - TC for all TO_CPU packets
* @param[in] toCpuDp                  - DP for all TO_CPU packets
* @param[in] toAnalyzerTc             - TC for all TO_ANALYZER packets
* @param[in] toAnalyzerDp             - DP for all TO_ANALYZER packets
* @param[in] fromCpuDp                - DP for all FROM_CPU packets
*                                       GT_OK on success, others on fail.
*
* @note NONE
*
*/
GT_STATUS prvWrAppLion2FabricConfigureQos
(
    IN GT_U8              lion2FabricDevNum,
    IN GT_U32             toCpuTc,
    IN CPSS_DP_LEVEL_ENT  toCpuDp,
    IN GT_U32             toAnalyzerTc,
    IN CPSS_DP_LEVEL_ENT  toAnalyzerDp,
    IN CPSS_DP_LEVEL_ENT  fromCpuDp
);

/**
* @internal prvWrAppLion2FabricForwardAddBc2Device function
* @endinternal
*
* @brief   The function writes PCL rules for UC packets with commands FORWARD, FROM_CPU, TO_ANALYSER
*         that must be forwarded to the given target device.
* @param[in] lion2FabricDevNum        - Lion2 CPSS device number
* @param[in] hwDevId                  - HW id of BC2 target device
* @param[in] linkPortNum              - number of Lion2 port linked to BC2 target device
*                                       GT_OK on success, others on fail.
*
* @note NONE
*
*/
GT_STATUS prvWrAppLion2FabricForwardAddBc2Device
(
    IN  GT_U8           lion2FabricDevNum,
    IN  GT_U32          hwDevId,
    IN  GT_PORT_NUM     linkPortNum
);

/**
* @internal prvWrAppLion2FabricForwardAddBc2Vidx function
* @endinternal
*
* @brief   The function writes PCL rules for MC packets with the given target VIDX
*         with commands FORWARD, FROM_CPU, TO_ANALYSER
*         that must be egressed from the given set (bitmap) of Lion2 ports.
* @param[in] lion2FabricDevNum        - Lion2 CPSS device number
* @param[in] bc2Vidx                  - target VIDX (coded by BC2 device in eDSA Tag)
* @param[in] targetPortsBmpPtr        - (pointer to) bitmap of target Lion2 ports
*                                       GT_OK on success, others on fail.
*
* @note NONE
*
*/
GT_STATUS prvWrAppLion2FabricForwardAddBc2Vidx
(
    IN  GT_U8                 lion2FabricDevNum,
    IN  GT_U32                bc2Vidx,
    IN  CPSS_PORTS_BMP_STC    *targetPortsBmpPtr
);

/**
* @internal prvWrAppLion2FabricForwardAddBc2Vlan function
* @endinternal
*
* @brief   The function writes PCL rules for BC packets with the given target VID
*         with commands FORWARD, FROM_CPU
*         that must be egressed from the given set (bitmap) of Lion2 ports.
* @param[in] lion2FabricDevNum        - Lion2 CPSS device number
* @param[in] bc2VlanId                - target eVID (coded by BC2 device in eDSA Tag)
* @param[in] targetPortsBmpPtr        - (pointer to) bitmap of target Lion2 ports
*                                       GT_OK on success, others on fail.
*
* @note NONE
*
*/
GT_STATUS prvWrAppLion2FabricForwardAddBc2Vlan
(
    IN  GT_U8                 lion2FabricDevNum,
    IN  GT_U32                bc2VlanId,
    IN  CPSS_PORTS_BMP_STC    *targetPortsBmpPtr
);

/**
* @internal prvWrAppLion2FabricInit function
* @endinternal
*
* @brief   The function initializes DB and writes PCL rule for TO_CPU packets
*
* @param[in] lion2FabricDevNum        - Lion2 CPSS device number
* @param[in] cpuLinkPortNum           - number of Lion2 port linked to BC2 devices with CPU port
*                                       GT_OK on success, others on fail.
*
* @note NONE
*
*/
GT_STATUS prvWrAppLion2FabricInit
(
    IN GT_U8              lion2FabricDevNum,
    IN GT_PORT_NUM        cpuLinkPortNum
);

/**
* @internal prvWrAppLionTrunkSrcPortHashMappingSet_ForMultiCoreFdbLookup function
* @endinternal
*
* @brief   set the trunk srcPort hash mode settings.
*
* @param[in] boardRevId               - The board revision Id.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS prvWrAppLionTrunkSrcPortHashMappingSet_ForMultiCoreFdbLookup
(
    IN  GT_U8                       boardRevId
);

#ifdef INCLUDE_TM
/**
* @internal prvWrAppBc2DramOrTmInit function
* @endinternal
*
* @brief   Board specific configurations of TM and all related HW
*         or of TM related DRAM only.
* @param[in] dev                      - device number
* @param[in] flags                    - initialization flags
*                                      currently supported value 1 that means "DRAM only"
*                                      otherwise TM and all related HW initialized
*
* @param[out] dramIfBmpPtr             - pointer to bitmap of TM DRAM interfaces. may be NULL
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*
* @note Used type of board recognized at last device (as in other code)
*
*/
GT_STATUS prvWrAppBc2DramOrTmInit
(
    IN  GT_U8       dev,
    IN  GT_U32      flags,
    OUT GT_U32     *dramIfBmpPtr
);

/**
* @internal prvWrAppTmGeneral48PortsInit function
* @endinternal
*
* @brief   Implements general 48 Ports initialization for specific device.
*
* @param[in] dev                      -   device number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS prvWrAppTmGeneral48PortsInit
(
    IN  GT_U8   dev
);

/**
* @internal prvWrAppTmGeneral4PortsInit function
* @endinternal
*
* @brief   Implements general 4 Ports initialization for specific device.
*
* @param[in] dev                      -   device number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS prvWrAppTmGeneral4PortsInit
(
    IN  GT_U8   dev
);

/**
* @internal prvWrAppTmScenario20Init function
* @endinternal
*
* @brief   Implements scenario 2 initialization for specific device.
*         10 1G ports, 4 c node per port, 5 b node per c node, 10 a node per b node, 8 queues per a node
*         shaping on all levels (a,b,c,p)
*         Tail Drop on port and queue level
*         port 64 - 40G  -> C -> B -> A -> 252 Queues
*         no shaping , no drop
* @param[in] dev                      -   device number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS prvWrAppTmScenario20Init
(
    IN  GT_U8   dev
);

/**
* @internal prvWrAppTmScenario2Init function
* @endinternal
*
* @brief   Implements scenario 2 initialization for specific device.
*         10 1G ports, 4 c node per port, 5 b node per c node, 10 a node per b node, 8 queues per a node
*         shaping on all levels (a,b,c,p)
*         Tail Drop on port and queue level
* @param[in] dev                      -   device number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS prvWrAppTmScenario2Init
(
    IN  GT_U8   dev
);

/**
* @internal prvWrAppTmScenario3Init function
* @endinternal
*
* @brief   Implements scenario 3 initialization for specific device.
*         10 1G ports, 4 c node per port, 5 b node per c node, 10 a node per b node, 8 queues per a node
*         shaping on all levels (a,b,c,p), propagated priority.
*         Tail Drop on port and queue level and color aware on Q1 in each A node.
* @param[in] dev                      -   device number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS prvWrAppTmScenario3Init
(
    IN  GT_U8   dev
);

/**
* @internal prvWrAppTmScenario4Init function
* @endinternal
*
* @brief   Implements scenario 4 initialization for specific device.
*         10 1G ports, 4 c node per port, 5 b node per c node, 10 a node per b node, 8 queues per a node
*         shaping on all levels (a,b,c,p), propagated priority.
*         Tail Drop on port and queue level and color aware on Q1 in each A node and wred mode on queue0 in each A node
* @param[in] dev                      -   device number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS prvWrAppTmScenario4Init
(
    IN  GT_U8   dev
);

/**
* @internal prvWrAppTmScenario5Init function
* @endinternal
*
* @brief   Implements scenario 5 initialization for specific device.
*         10 1G ports, 4 c node per port, 5 b node per c node, 10 a node per b node, 4 queues per a node
*         shaping on all levels (a,b,c,p), propagated priority.
*         Tail Drop on port wred drop all queues
* @param[in] dev                      -   device number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS prvWrAppTmScenario5Init
(
    IN  GT_U8   dev
);

/**
* @internal prvWrAppTmScenario6Init function
* @endinternal
*
* @brief   Implements scenario 6 initialization for specific device.
*
* @param[in] dev                      -   device number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS prvWrAppTmScenario6Init
(
    IN  GT_U8   dev
);

/**
* @internal prvWrAppTmScenario7Init function
* @endinternal
*
* @brief   Implements scenario 5 initialization for specific device.
*         10 1G ports, 4 c node per port, 5 b node per c node, 10 a node per b node, 4 queues per a node
*         shaping on all levels (a,b,c,p), propagated priority.
*         Tail Drop on port wred drop all queues
* @param[in] dev                      -   device number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS prvWrAppTmScenario7Init
(
    IN  GT_U8   dev
);

/**
* @internal prvWrAppTmScenario8Init function
* @endinternal
*
* @brief   Implements scenario 8 initialization for specific device.
*         In this scenario there are shpaers for every (VLAN,TC) pair
* @param[in] dev                      -   device number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS prvWrAppTmScenario8Init
(
    IN  GT_U8   dev
);

/**
* @internal prvWrAppTmScenario9Init function
* @endinternal
*
* @brief   Scenario for demonstration of PFC. There is one C-node for one Q-node, including nodes
*         in between
* @param[in] dev                      -   device number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS prvWrAppTmScenario9Init
(
    IN  GT_U8   dev
);

/**
* @internal prvWrAppTmStressScenarioInit function
* @endinternal
*
* @brief   Implements strss test
*
* @param[in] dev                      -   device number
* @param[in] mode                     -   select tree configuration  -  implementation nodes in  .c file
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS prvWrAppTmStressScenarioInit
(
    IN  GT_U8   dev,
    IN  GT_U8   mode
);
#endif

/* convert the trunkId to the global EPort that represents it */
GT_STATUS prvWrAppTrunkEPortGet_phy1690_WA_B(
    IN GT_TRUNK_ID              trunkId,
    OUT GT_PORT_NUM              *globalEPortPtr
);

GT_STATUS prvWrAppDemoFdbManagerAuMsgHandlerMethod
(
    IN APP_DEMO_AU_MESSAGE_HANDLER_METHORD_ENT auMsgHandler
);

GT_STATUS prvWrAppDxChNetRxPacketCbRegister
(
    IN  RX_PACKET_RECEIVE_CB_FUN  rxPktReceiveCbFun
);

/**
* @internal prvWrAppFirstDevNum function
* @endinternal
*
* @brief convert local devNum to system devNum to add ability to set different device numbers for stacking
*
* @retval GT_OK                    - on success
*
*
*/
GT_STATUS prvWrAppFirstDevNum
(
    GT_U32 devNum
);

GT_VOID prvWrAppDxChNewPclTcamSupportSet
(
    IN  GT_BOOL             enableIndexConversion
);

GT_STATUS prvWrAppDemoFdbManagerControlSet
(
    IN APP_DEMO_AU_MESSAGE_HANDLER_METHORD_ENT auMsgHandler,
    IN GT_BOOL                                  autoAgingEn,
    IN GT_U32                                   autoAgingInterval
);

GT_STATUS prvWrAppDemoFdbManagerAutoAgingStatisticsGet
(
    OUT APP_DEMO_CPSS_DXCH_BRG_FDB_MANAGER_STATISTICS_STC *statistics
);

GT_STATUS prvWrAppDemoFdbManagerAutoAgingStatisticsClear();

GT_STATUS prvWrAppDemoFdbManagerGlobalIDSet
(
    IN GT_U32 fdbManagerId
);

/**
* @internal prvWrAppTraceAuqFlagSet function
* @endinternal
*
* @brief   function to allow set the flag of : traceAuq
*
* @param[in] enable                   - enable/disable the printings of indication of AUQ/FUQ messages:
*                                      "+",    CPSS_NA_E
*                                      "qa",   CPSS_QA_E should not be
*                                      "qr",   CPSS_QR_E
*                                      "-",    CPSS_AA_E
*                                      "t",    CPSS_TA_E
*                                      "sa",   CPSS_SA_E should not be
*                                      "qi",   CPSS_QI_E should not be
*                                      "f"     CPSS_FU_E
*
* @retval GT_OK                    - on success
*/
GT_STATUS prvWrAppTraceAuqFlagSet
(
    IN GT_U32   enable
);

/**
* @internal prvWrAppPrintPortEeeInterruptInfoSet function
* @endinternal
*
* @brief   function to allow set the flag of : printEeeInterruptInfo
*
* @param[in] enable                   - enable/disable the printings of ports 'EEE interrupts info'
*
* @retval GT_OK                    - on success
*/
GT_STATUS prvWrAppPrintPortEeeInterruptInfoSet
(
    IN GT_U32   enable
);

/**
 * @internal prvWrAppDlbWaEnable function
 * @endinternal
 *
 * @brief   Enable/Disable DLB WA and creates a task to call waExecute periodically.
 *
 * @param[in] devNum                - device number
 * @param[in] waEnable              - status of DLB WA
 *                                    GT_TRUE     - DLB WA is enabled
 *                                    GT_FALSE    - DLB WA is disabled
 * @param[in] newDelay              - Interval between consecutive DLB WA call in milisec.
 * @retval GT_OK                    - on success.
 * @retval GT_FAIL                  - otherwise.
 * @note none
 *
 */
GT_STATUS prvWrAppDlbWaEnable
(
    GT_U8                               devNum,
    GT_BOOL                             waEnable,
    GT_U32                              newDelay
);

/**
* @internal prvWrAppPortManagerTaskSleepTimeSet function
* @endinternal
*
* @brief   debug option to state the 'sleep' for the port manager ! between 'all devices' iterations
*
* @param[in] timeInMilli
*
* @retval GT_OK                    - on success
*/
GT_STATUS prvWrAppPortManagerTaskSleepTimeSet
(
    GT_U32 timeInMilli
);

/**
* @internal prvWrAppPrintLinkChangeFlagSet function
* @endinternal
*
* @brief   function to allow set the flag of : printLinkChangeEnabled
*
* @param[in] enable                   - enable/disable the printing of 'link change'
*
* @retval GT_OK                    - on success
*/
GT_STATUS prvWrAppPrintLinkChangeFlagSet
(
    IN GT_U32   enable
);

/**
* @internal prvWrAppPrintLinkChangeFlagGet function
* @endinternal
*
* @brief   function to get flag state of : printLinkChangeEnabled
*/
GT_BOOL prvWrAppPrintLinkChangeFlagGet
(
    IN GT_VOID
);

/**
* @internal prvWrAppEventHandlerTaskRestore function
* @endinternal
*
* @brief   This routine unbind the unified event list 
*          and remove the event handler made in eventsToTestsHandlerbind .
*
* @param[in] cpssUniEventArr[]        - The CPSS unified event list.
* @param[in] arrLength                - The unified event list length.
*/
GT_STATUS prvWrAppEventHandlerTaskRestore
(
    IN  CPSS_UNI_EV_CAUSE_ENT       cpssUniEventArr[],
    IN  GT_U32                      arrLength
);

#endif

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __utfExtraHelpersh */


