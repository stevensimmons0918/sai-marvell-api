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
* @file prvCpssDxChBrgFdbManager_hw.h
*
* @brief FDB manager support - managing HW accessing.
*       configure specific/list/all devices registered with the manager.
*
* @version   1
********************************************************************************
*/
#ifndef __prvCpssDxChBrgFdbManager_hw_h
#define __prvCpssDxChBrgFdbManager_hw_h

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/dxCh/dxChxGen/bridgeFdbManager/cpssDxChBrgFdbManager.h>

/* Indicates high AU message rate - For Stage A
 * unit - messages per second */
#define PRV_FDB_MANAGER_AU_MSG_RATE_STAGE_A         _128K

/* Indicates low AU message rate - For Stage B
 * unit - messages per second */
#define PRV_FDB_MANAGER_AU_MSG_RATE_STAGE_B         _5K

/*
* @internal prvCpssDxChFdbManagerHwFlushByHwIndex function
* @endinternal
*
* @brief   This function Flush HW of entry hwIndex from all registered devices in the manager.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] fdbManagerPtr          - (pointer to) the FDB Manager.
* @param[in] hwIndex                - HW index.
* @param[in] bankId                 - the bankId in case we need to decrement the HW counter.
* @param[in] updateHwBankCounter    - indication to decrement the HW counter.
*
* @retval GT_OK                    - on success
* @retval other                    - unexpected error
*
* @note
*   NONE
*
*/
GT_STATUS   prvCpssDxChFdbManagerHwFlushByHwIndex
(
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC   *fdbManagerPtr,
    IN GT_U32                                           hwIndex,
    IN GT_U32                                           bankId,
    IN GT_BOOL                                          updateHwBankCounter
);

/*
* @internal prvCpssDxChFdbManagerHwWriteByHwIndex function
* @endinternal
*
* @brief   This function write HW entry at hwIndex from all registered devices in the manager.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] fdbManagerPtr          - (pointer to) the FDB Manager.
* @param[in] calcInfoPtr            - using calcInfoPtr->dbEntryPtr,
*                                           calcInfoPtr->dbEntryExtPtr
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - convert of manager format to HW format failed.
*
* @note
*   NONE
*
*/
GT_STATUS   prvCpssDxChFdbManagerHwWriteByHwIndex
(
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC       *fdbManagerPtr,
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_HASH_CALC_ENTRY_STC     *calcInfoPtr
);

/*
* @internal prvCpssDxChFdbManagerHwInitNewDevice function
* @endinternal
*
* @brief   This function do HW initialization from new device.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] fdbManagerPtr          - (pointer to) the FDB Manager.
* @param[in] devNum                 - the device number (cpss devNum)
* @param[in] numOfEports            - number of eports to initialize (according
*                                     to number of eports of the device)
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note
*   NONE
*
*/
GT_STATUS   prvCpssDxChFdbManagerHwInitNewDevice(
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC *fdbManagerPtr,
    IN GT_U8                                devNum,
    IN GT_U32                               numOfEports
);

/**
* @internal prvCpssDxChFdbManagerHwWriteEntriesFromDbToHwOfNewDevices function
* @endinternal
*
* @brief   This function start to write entries from manager to HW of the new devices
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] fdbManagerPtr          - (pointer to) the FDB Manager.
* @param[in] devListArr[]          - the array of device ids to add to the FDB Manager.
* @param[in] numOfDevs             - the number of device ids in the array.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device in devListArr
*
* @note
*   NONE
*
*/
GT_STATUS   prvCpssDxChFdbManagerHwWriteEntriesFromDbToHwOfNewDevices(
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC *fdbManagerPtr,
    IN GT_U8  devListArr[], /*arrSizeVarName=numOfDevs*/
    IN GT_U32 numOfDevs
);

/*
* @internal prvCpssDxChFdbManagerHwFlushAll function
* @endinternal
*
* @brief   This function Flush all entries from the HW from all registered devices in the manager.
*          NOTE this function not update the DB itself ,the Caller responsible for it.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] fdbManagerPtr          - (pointer to) the FDB Manager.
*
* @retval GT_OK                    - on success
* @retval other                    - unexpected error
*
* @note
*   NONE
*
*/
GT_STATUS prvCpssDxChFdbManagerHwFlushAll
(
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC  *fdbManagerPtr
);

/*
* @internal prvCpssDxChFdbManagerLearningThrottlingSet function
* @endinternal
*
* @brief   This function sets the learning throttling related parameters to the HW, according to the threshold label.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - the device number (cpss devNum)
* @param[in] thresholdType         - the current threshold type
*
* @retval GT_OK                    - on success
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on other error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChFdbManagerLearningThrottlingSet
(
    IN GT_U8                                            devNum,
    IN PRV_CPSS_DXCH_BRG_FDB_MANAGER_THRESHOLD_TYPE_ENT thresholdType
);

/**
* @internal prvCpssDxChFdbManagerAacHwWriteEntry function
* @endinternal
*
* @brief   Write a whole FDB entry to the HW using AAC method.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - the device number
* @param[in] hwIndex                  - index in the table
* @param[in] wordsArr                 - (pointer to) the 4 words data.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on failure.
* @retval GT_HW_ERROR              - on hardware error
*
*/
GT_STATUS prvCpssDxChFdbManagerAacHwWriteEntry
(
    IN GT_U8                    devNum,
    IN GT_U32                   hwIndex,
    IN GT_U32                   wordsArr[]
);

/**
* @internal prvCpssDxChFdbManagerHwWriteEntry function
* @endinternal
*
* @brief   Write a whole FDB entry to the HW in all port groups.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - the device number
* @param[in] hwIndex                  - index in the table
* @param[in] wordsArr                 - (pointer to) the 4 words data.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on failure.
* @retval GT_OUT_OF_RANGE          - parameter not in valid range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note In the case of wordsArr == NULL and indirect table the function
*       just send write entry command to device.
*       And entry is taken from the data registers of a indirect table.
*
*/
GT_STATUS prvCpssDxChFdbManagerHwWriteEntry
(
    IN GT_U8                    devNum,
    IN GT_U32                   hwIndex,
    IN GT_U32                   wordsArr[]
);

/*
* @internal prvCpssDxChFdbManagerHwAgedOutVerify function
* @endinternal
*
* @brief   The function verifies the DB entry is aged-out not not & update the status in agedOut Out param.
*             Not aged-out - Reset the age-bit of input entry in all the devices as part of aging process.
*             Aged-out     - No updates are required.
*
* @param[in] fdbManagerPtr         - (pointer to) the FDB manager
* @param[in] dbEntryPtr            - (pointer to) the DB entry
* @param[out] agedOutPtr           - (pointer to) the aged-out status.
*                                    GT_TRUE  - Entry is aged-out
*                                    GT_FALSE - Entry is not aged-out
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on failure.
* @retval GT_OUT_OF_RANGE          - parameter not in valid range
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note
*   NONE
*
*/
GT_STATUS prvCpssDxChFdbManagerHwAgedOutVerify
(
    IN  PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC          *fdbManagerPtr,
    IN  PRV_CPSS_DXCH_FDB_MANAGER_DB_ENTRY_FORMAT_STC           *dbEntryPtr,
    OUT GT_BOOL                                                 *agedOutPtr
);

/*
* @internal prvCpssDxChBrgFdbManagerHwEntryMatchWithSwEntry_allDevice function
* @endinternal
*
* @brief   The function gets the HW entry from all device for dbEntryPtr and matches the content.
*          depending on the listType.
*
* @param[in] fdbManagerPtr         - (pointer to) the FDB manager
* @param[in] dbEntryPtr            - (pointer to) the DB entry
* @param[in] listType              - DB check list type need to be validated
* @param[out] resultPtr            - (pointer to) test result.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on failure.
* @retval GT_BAD_PARAM             - wrong parameter
* @retval GT_OUT_OF_RANGE          - parameter not in valid range
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note
*   NONE
*/
GT_STATUS prvCpssDxChBrgFdbManagerHwEntryMatchWithSwEntry_allDevice
(
    IN  PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC   *fdbManagerPtr,
    IN  GT_U32                                            hwIndex,
    IN  PRV_CPSS_DXCH_FDB_MANAGER_DB_ENTRY_FORMAT_STC    *dbEntryPtr,
    IN  PRV_CPSS_DXCH_FDB_MANAGER_DB_CHECK_LIST_TYPE_ENT  listType,
    OUT CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_RESULT_ENT   *resultPtr
);

/*
* @internal prvCpssDxChFdbManagerDebugSelfTest_SpEntry function
* @endinternal
*
* @brief   The function Injects/corrects error relates to HW SP entry.
*
* @param[in] fdbManagerPtr         - (pointer to) the FDB manager
* @param[in] errorEnable           - Status value indicates error/correct state
*                                    GT_TRUE  - Error need to injected(Create a SP entry in place of free index)
*                                    GT_FALSE - Error need to be corrected(Replace the SP entry with in valid entry)
* @param[in,out] hwIndexPtr        - (pointer to) the hwIndex
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on failure.
* @retval GT_BAD_PARAM             - wrong parameter
* @retval GT_OUT_OF_RANGE          - parameter not in valid range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note
*   NONE
*/
GT_STATUS prvCpssDxChFdbManagerDebugSelfTest_SpEntry
(
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC    *fdbManagerPtr,
    IN GT_BOOL                                            errorEnable,
    INOUT GT_U32                                         *hwIndexPtr
);

/*
* @internal prvCpssDxChBrgFdbManagerPortGroupBankCounterValueGet function
* @endinternal
*
* @brief   The function Get the value of counter of the specific FDB table bank and portGroup.
*
* @param[in] devNum                - device number
* @param[in] portGroupId           - the portGroupId. to support multi-port-groups device-port-groups device
* @param[in] bankIndex             - the index of the bank
*                                    (APPLICABLE RANGES: 0..15)
* @param[out] valuePtr             - (pointer to)the value of the counter.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong data
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS prvCpssDxChBrgFdbManagerPortGroupBankCounterValueGet
(
    IN  GT_U8           devNum,
    IN  GT_U32          portGroupId,
    IN  GT_U32          bankIndex,
    OUT GT_U32          *valuePtr
);

/*
* @internal prvCpssDxChBrgFdbManagerPortGroupBankCounterUpdate function
* @endinternal
*
* @brief  This function Update (increment/decrement) the counter of the specific bank.
*         The function will fail (GT_BAD_STATE) if the PP is busy with previous update.
*
*         the application can determine if PP is ready by calling
*         cpssDxChBrgFdbBankCounterUpdateStatusGet.
*         relevant to CPSS_MAC_HASH_FUNC_CRC_MULTI_HASH_E
*           NOTE: this function is needed to be called after calling one of the next:
*         'write by index'      - cpssDxChBrgFdbMacEntryWrite ,
*               cpssDxChBrgFdbPortGroupMacEntryWrite
*         'invalidate by index' - cpssDxChBrgFdbMacEntryInvalidate ,
*               cpssDxChBrgFdbPortGroupMacEntryInvalidate
*
*         The application logic should be:
*             if last action was 'write by index' then :
*               - if the previous entry (in the index) was valid      -- do no call this function.
*               - if the previous entry (in the index) was not valid  -- do 'increment'.
*             if last action was 'invalidate by index' then :
*               - if the previous entry (in the index) was valid      -- do 'decrement'.
*               - if the previous entry (in the index) was not valid  -- do no call this function.
*
* @param[in] devNum                - device number
* @param[in] portGroupId           - the portGroupId. to support multi-port-groups device-port-groups device
* @param[in] bankIndex             - the index of the bank
*                                    (APPLICABLE RANGES: 0..15)
* @param[in] incOrDec              - increment or decrement the counter by one.
*                                    GT_TRUE - increment the counter
*                                    GT_FALSE - decrement the counter
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_OUT_OF_RANGE          - bankIndex > 15
* @retval GT_BAD_STATE             - the PP is not ready to get a new update from CPU.
*                                       (PP still busy with previous update)
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChBrgFdbManagerPortGroupBankCounterUpdate
(
    IN  GT_U8           devNum,
    IN  GT_U32          portGroupId,
    IN  GT_U32          bankIndex,
    IN GT_BOOL          incOrDec
);

/*
* @internal prvCpssDxChFdbManagerHwCounterSyncUpWithDb function
* @endinternal
*
* @brief   The function Updates HW per bank counter, according to the current DB counter.
*
* @param[in] fdbManagerPtr         - (pointer to) the FDB Manager.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong data
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS prvCpssDxChFdbManagerHwCounterSyncUpWithDb
(
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC          *fdbManagerPtr
);

/*
* @internal prvCpssDxChFdbManagerHwDeviceConfigUpdate function
* @endinternal
*
* @brief   This function do HW updates on global parameters for existing device.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman; AC5.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] fdbManagerPtr          - (pointer to) the FDB Manager.
* @param[in] devNum                 - the device number (cpss devNum)
* @param[in] entryAttrPtr          - (pointer to) parameters to override entryAttrPtr
*                                       that was given during 'manager create' API.
*                                     NOTE: ignored if NULL
* @param[in] learningPtr           - (pointer to) parameters to override learningPtr
*                                       that was given during 'manager create' API.
*                                     NOTE: ignored if NULL
* @param[in] agingPtr              - (pointer to) parameters to override agingPtr
*                                       that was given during 'manager create' API.
*                                     NOTE: ignored if NULL
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note
*   NONE
*
*/
GT_STATUS   prvCpssDxChFdbManagerHwDeviceConfigUpdate(
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC *fdbManagerPtr,
    IN GT_U8                                devNum,
    IN CPSS_DXCH_BRG_FDB_MANAGER_CONFIG_UPDATE_ENTRY_ATTRIBUTES_STC  *entryAttrPtr,
    IN CPSS_DXCH_BRG_FDB_MANAGER_CONFIG_UPDATE_LEARNING_STC          *learningPtr,
    IN CPSS_DXCH_BRG_FDB_MANAGER_CONFIG_UPDATE_AGING_STC             *agingPtr
);

/**
* @internal prvCpssDxChFdbManagerDeviceHwFormatGet function
* @endinternal
*
* @brief   This function gets the FDB entry HW format type. (Applicable for Hybrid/SIP5 shadow type)
*
*
* @param[in] fdbManagerPtr   - (pointer to) the FDB Manager.
* @param[in] devNum          - device number
* @param[in] hwFormatType    - (pointer to) Type of device
*
* @retval GT_OK              - on success
* @retval GT_BAD_PARAM       - on wrong parameters
*
*/
GT_STATUS prvCpssDxChFdbManagerDeviceHwFormatGet
(
    IN  PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC   *fdbManagerPtr,
    IN  GT_U8                                             devNum,
    OUT PRV_CPSS_DXCH_BRG_FDB_MANAGER_HW_FORMAT_TYPE_ENT *hwFormatType
);

/**
* @internal prvCpssDxChFdbManagerHwEntryRead function
* @endinternal
*
* @brief The function reads FDB entry from HW.
*
* @param[in] devNum               - device number.
* @param[in] hwIndex              - HW index to the FDB entry.
* @param[in] fdbManagerPtr        - (pointer to) the FDB Manager.
* @param[in] hwFormatType          - FDB Entry hw format type.
* @param[out] addParamsPtr        - (pointer to) the add param for FDB Manager entry add.
* @param[out] isValidPtr          - Indicates is the entry is valid to add to SW/HW.
*
* @retval GT_OK                   - on success
* @retval GT_FAIL                 - on error
* @retval GT_BAD_PARAM            - on wrong FDB manager ID.
* @retval GT_NOT_INITIALIZED      - if the FDB Manager is not initialized.
*
*/
GT_STATUS prvCpssDxChFdbManagerHwEntryRead
(
    IN  GT_U8                                             devNum,
    IN  GT_U32                                            hwIndex,
    IN  PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC   *fdbManagerPtr,
    IN  PRV_CPSS_DXCH_BRG_FDB_MANAGER_HW_FORMAT_TYPE_ENT  hwFormatType,
    OUT CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_ADD_PARAMS_STC   *addParamsPtr,
    OUT GT_BOOL                                          *isValidPtr
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvCpssDxChBrgFdbManager_hw_h */


