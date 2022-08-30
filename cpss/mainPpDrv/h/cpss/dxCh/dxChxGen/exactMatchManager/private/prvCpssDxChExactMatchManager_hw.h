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
* @file prvCpssDxChExactMatchManager_hw.h
*
* @brief Exact Match manager support - managing HW accessing.
*       configure specific/list/all devices registered with the manager.
*
* @version   1
********************************************************************************
*/
#ifndef __prvCpssDxChExactMatchManager_hw_h
#define __prvCpssDxChExactMatchManager_hw_h

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/dxCh/dxChxGen/exactMatchManager/cpssDxChExactMatchManager.h>

/*
* @internal prvCpssDxChExactMatchManagerHwLookupConfigSet function
* @endinternal
*
* @brief   This function set all lookup configuration.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum          - device number.
* @param[in] portGroupBmp    - port group bitmap
* @param[in] lookupPtr       - (pointer to) the Exact Match Manager lookup config.
*
* @retval GT_OK              - on success
* @retval other              - unexpected error
*
* @note
*   NONE
*
*/
GT_STATUS   prvCpssDxChExactMatchManagerHwLookupConfigSet
(
    IN GT_U8                                        devNum,
    IN GT_PORT_GROUPS_BMP                           portGroupBmp,
    IN CPSS_DXCH_EXACT_MATCH_MANAGER_LOOKUP_STC     *lookupPtr
);

/*
* @internal prvCpssDxChExactMatchManagerHwLookupConfigDisbale function
* @endinternal
*
* @brief   This function disable all lookup configuration from all registered devices in the manager.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] lookupInfoPtr   - (pointer to) the Exact Match Manager lookup config.
* @param[in] devNum          - device number.
* @param[in] portGroupBmp    - port group bitmap
*
* @retval GT_OK              - on success
* @retval other              - unexpected error
*
* @note
*   NONE
*
*/
GT_STATUS   prvCpssDxChExactMatchManagerHwLookupConfigDisbale
(
    IN CPSS_DXCH_EXACT_MATCH_MANAGER_LOOKUP_STC     *lookupInfoPtr,
    IN GT_U8                                        devNum,
    IN GT_PORT_GROUPS_BMP                           portGroupBmp
);

/*
* @internal prvCpssDxChExactMatchManagerHwExpandedTableConfigSet function
* @endinternal
*
* @brief   This function set expanded table configuration.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                     - device number.
* @param[in] portGroupsBmp              - port group bitmap
* @param[in] exactMatchExpanderArray    - the Exact Match Manager expanded config array.
*
* @retval GT_OK              - on success
* @retval other              - unexpected error
*
* @note
*   NONE
*
*/
GT_STATUS   prvCpssDxChExactMatchManagerHwExpandedTableConfigSet
(
    IN GT_U8                                                    devNum,
    IN GT_PORT_GROUPS_BMP                                       portGroupsBmp,
    IN PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_EXPANDED_ACTION_STC    exactMatchExpanderArray[]
);
/*
* @internal prvCpssDxChExactMatchManagerHwExpandedActionUpdate function
* @endinternal
*
* @brief   The function set the Expanded Action table HW for all devices defined on the manager.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] exactMatchManagerPtr  - (pointer to) the Exact Match Manager.
* @param[in] expandedActionIndex   - Exact Match Expander table index
*                                    (APPLICABLE RANGES:0..15)
* @param[in] paramsPtr             - the expanded action attributes.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong exactMatchManagerId and paramsPtr data
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_INITIALIZED       - if the Exact Match Manager is not initialized
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note
*         None
*/
GT_STATUS prvCpssDxChExactMatchManagerHwExpandedActionUpdate
(
    IN  PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_INSTANCE_INFO_STC          *exactMatchManagerPtr,
    IN  GT_U32                                                          expandedActionIndex,
    IN  CPSS_DXCH_EXACT_MATCH_MANAGER_EXPANDED_ACTION_SET_PARAMS_STC    *paramsPtr
);
/*
* @internal prvCpssDxChExactMatchManagerHwFlushByHwIndex function
* @endinternal
*
* @brief   This function Flush HW of entry hwIndex from all registered devices in the manager.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] exactMatchManagerPtr   - (pointer to) the Exact Match Manager.
* @param[in] hwIndex                - HW index.
*
* @retval GT_OK                    - on success
* @retval other                    - unexpected error
*
* @note
*   NONE
*
*/
GT_STATUS   prvCpssDxChExactMatchManagerHwFlushByHwIndex
(
    IN PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_INSTANCE_INFO_STC   *exactMatchManagerPtr,
    IN GT_U32                                                   hwIndex
);

/*
* @internal prvCpssDxChExactMatchManagerHwWriteByHwIndex function
* @endinternal
*
* @brief   This function write HW entry at hwIndex from all registered devices in the manager.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] exactMatchManagerPtr   - (pointer to) the Exact Match Manager.
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
GT_STATUS   prvCpssDxChExactMatchManagerHwWriteByHwIndex
(
    IN PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_INSTANCE_INFO_STC       *exactMatchManagerPtr,
    IN PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_HASH_CALC_ENTRY_STC     *calcInfoPtr
);

/*
* @internal prvCpssDxChExactMatchManagerHwReadByHwIndex function
* @endinternal
*
* @brief   This function read HW entry at hwIndex from all registered devices in the manager.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] exactMatchManagerPtr   - (pointer to) the Exact Match Manager.
* @param[in] calcInfoPtr            - using calcInfoPtr->dbEntryPtr
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - convert of manager format to HW format failed.
*
* @note
*   NONE
*
*/
GT_STATUS   prvCpssDxChExactMatchManagerHwReadByHwIndex
(
    IN PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_INSTANCE_INFO_STC       *exactMatchManagerPtr,
    IN PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_HASH_CALC_ENTRY_STC     *calcInfoPtr
);

/*
* @internal prvCpssDxChExactMatchManagerHwInitNewDevice function
* @endinternal
*
* @brief   This function do HW initialization from new device.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] exactMatchManagerPtr   - (pointer to) the Exact Match Manager.
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
GT_STATUS   prvCpssDxChExactMatchManagerHwInitNewDevice(
    IN PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_INSTANCE_INFO_STC   *exactMatchManagerPtr,
    IN GT_U8                                                    devNum,
    IN GT_U32                                                   numOfEports
);

GT_STATUS   prvCpssDxChExactMatchManagerHwWriteSingleEntryFromDbToHwOfNewDevices(
    IN PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_INSTANCE_INFO_STC   *exactMatchManagerPtr,
    IN PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_ENTRY_FORMAT_STC    *dbEntryPtr,
    IN CPSS_DXCH_EXACT_MATCH_MANAGER_DEV_PORT_GROUP_PAIR_STC    pairListArr[],/*arrSizeVarName=numOfPairs*/
    IN GT_U32                                                   numOfPairs
);

/*
* @internal prvCpssDxChExactMatchManagerHwWriteEntriesFromDbToHwOfNewDevices function
* @endinternal
*
* @brief  This function start to write entries from manager to HW of the new devices
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] exactMatchManagerPtr  - (pointer to) the Exact Match Manager.
* @param[in] pairListArr[]         - the array of pairs (device ids + portGroupBmp)
*                                    to add to the Exact Match Manager.
* @param[in] numOfPairs            - the number of pairs in the array.
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
GT_STATUS   prvCpssDxChExactMatchManagerHwWriteEntriesFromDbToHwOfNewDevices(
    IN PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_INSTANCE_INFO_STC   *exactMatchManagerPtr,
    IN CPSS_DXCH_EXACT_MATCH_MANAGER_DEV_PORT_GROUP_PAIR_STC    pairListArr[],/*arrSizeVarName=numOfPairs*/
    IN GT_U32                                                   numOfPairs
);

/*
* @internal prvCpssDxChExactMatchManagerHwFlushAll function
* @endinternal
*
* @brief   This function Flush all entries from the HW from all registered devices in the manager.
*          NOTE this function not update the DB itself ,the Caller responsible for it.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] exactMatchManagerPtr  - (pointer to) the Exact Match Manager.
*
* @retval GT_OK                    - on success
* @retval other                    - unexpected error
*
* @note
*   NONE
*
*/
GT_STATUS prvCpssDxChExactMatchManagerHwFlushAll
(
    IN PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_INSTANCE_INFO_STC  *exactMatchManagerPtr
);

/*
* @internal prvCpssDxChExactMatchManagerHwFlushAllSpecificDevice function
* @endinternal
*
* @brief   This function Flush all entries from the HW from a specific devNum+portGroupsBmp.
*          NOTE this function not update the DB itself ,the Caller responsible for it.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] exactMatchManagerPtr   - (pointer to) the Exact Match Manager.
* @param[in] devNum                 - device number.
* @param[in] portGroupsBmp          - port group bitmap
*
* @retval GT_OK                    - on success
* @retval other                    - unexpected error
*
* @note
*   NONE
*
*/
GT_STATUS prvCpssDxChExactMatchManagerHwFlushAllSpecificDevice
(
  IN PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_INSTANCE_INFO_STC *exactMatchManagerPtr,
  IN GT_U8                                                  devNum,
  IN GT_PORT_GROUPS_BMP                                     portGroupsBmp
);
/*
* @internal prvCpssDxChExactMatchManagerHwEntryMatchWithSwEntry_allDevice function
* @endinternal
*
* @brief   The function gets the HW entry from all device for dbEntryPtr and matches the content.
*          depending on the listType.
*
* @param[in] exactMatchManagerPtr  - (pointer to) the Exact Match manager
* @param[in] hwIndex               - HW index.
* @param[in] dbEntryPtr            - (pointer to) the DB entry
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
GT_STATUS prvCpssDxChExactMatchManagerHwEntryMatchWithSwEntry_allDevice
(
    IN PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_INSTANCE_INFO_STC   *exactMatchManagerPtr,
    IN GT_U32                                                   hwIndex,
    IN PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_ENTRY_FORMAT_STC    *dbEntryPtr,
    OUT CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_RESULT_ENT      *resultPtr
);
/*
* @internal prvCpssDxChExactMatchManagerHwLookupMatchWithSwLookup function
* @endinternal
*
* @brief  This function check HW lookup configuration match with SW lookup configuration
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum          - device number.
* @param[in] portGroupsBmp   - port group bitmap
* @param[out]dbLookupPtr     - (pointer to) the Exact Match Manager lookup config saved in DB.
* @param[out]resultPtr       - (pointer to) test result.
*
* @retval GT_OK              - on success
* @retval other              - unexpected error
*
* @note
*   NONE
*
*/
GT_STATUS   prvCpssDxChExactMatchManagerHwLookupMatchWithSwLookup
(
    IN GT_U8                                                    devNum,
    IN GT_PORT_GROUPS_BMP                                       portGroupsBmp,
    IN CPSS_DXCH_EXACT_MATCH_MANAGER_LOOKUP_STC                 *dbLookupPtr,
    OUT CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_RESULT_ENT      *resultPtr
);
/*
* @internal prvCpssDxChExactMatchManagerHwLookupConfigMatchWithSwLookupConfig_allDevice function
* @endinternal
*
* @brief   The function gets the HW configuration from all device for
*          exactMatchManagerPtr->lookupInfo and matches the content.
*
* @param[in] exactMatchManagerPtr  - (pointer to) the Exact Match manager
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
GT_STATUS prvCpssDxChExactMatchManagerHwLookupConfigMatchWithSwLookupConfig_allDevice
(
    IN PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_INSTANCE_INFO_STC   *exactMatchManagerPtr,
    OUT CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_RESULT_ENT      *resultPtr
);
/*
* @internal prvCpssDxChExactMatchManagerHwExpandedArrayMatchWithSwExpandedArray function
* @endinternal
*
* @brief  This function check HW expanded configuration match with SW expanded configuration
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum          - device number.
* @param[in] portGroupsBmp   - port group bitmap
* @param[out]dbExpandedArray - the Exact Match Manager expanded config saved in DB.
* @param[out]resultPtr       - (pointer to) test result.
*
* @retval GT_OK              - on success
* @retval other              - unexpected error
*
* @note
*   NONE
*
*/
GT_STATUS   prvCpssDxChExactMatchManagerHwExpandedArrayMatchWithSwExpandedArray
(
    IN GT_U8                                                    devNum,
    IN GT_PORT_GROUPS_BMP                                       portGroupsBmp,
    IN PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_EXPANDED_ACTION_STC    dbExpandedArray[],
    OUT CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_RESULT_ENT      *resultPtr
);
/*
* @internal prvCpssDxChExactMatchManagerHwExpandedArrayMatchWithSwExpandedArray_allDevice function
* @endinternal
*
* @brief   The function gets the HW configuration from all device for
*          exactMatchManagerPtr->exactMatchExpanderArray and matches the content.
*
* @param[in] exactMatchManagerPtr  - (pointer to) the Exact Match manager
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
GT_STATUS prvCpssDxChExactMatchManagerHwExpandedArrayMatchWithSwExpandedArray_allDevice
(
    IN PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_INSTANCE_INFO_STC   *exactMatchManagerPtr,
    OUT CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_RESULT_ENT      *resultPtr
);

/*
* @internal prvCpssDxChExactMatchManagerHwAgedOutVerify function
* @endinternal
*
* @brief   The function verifies the DB entry is aged-out not not & update the status in agedOut Out param.
*             Not aged-out - Reset the age-bit of input entry in all the devices as part of aging process.
*             Aged-out     - No updates are required.
*
* @param[in] exactMatchManagerPtr  - (pointer to) the ExactMatch manager
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
GT_STATUS prvCpssDxChExactMatchManagerHwAgedOutVerify
(
    IN  PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_INSTANCE_INFO_STC          *exactMatchManagerPtr,
    IN  PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_ENTRY_FORMAT_STC           *dbEntryPtr,
    OUT GT_BOOL                                                         *agedOutPtr
);

/*
* @internal prvCpssDxChExactMatchManagerHwDbValidityCheck function
* @endinternal
*
* @brief The function verifies the Exact Match manager DB is sync with the HW.
*
* @param[in]  exactMatchManagerId  - The Exact Match Manager id.
*                                    (APPLICABLE RANGES : 0..31)
* @param[in]  checksPtr            - (Pointer to) DB check attributes
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*
*/
GT_STATUS prvCpssDxChExactMatchManagerHwDbValidityCheck
(
    IN  GT_U32                                              exactMatchManagerId,
    IN  CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_STC         *checksPtr
);

/**
* @internal prvCpssDxChExactMatchManagerHwHaTempDbCreate function
* @endinternal
*
* @brief The function create a temporary database for in manager and read all
*       Exact Match entries from HW to be used later on when the entries are replayed
*       by the application in the HA process
*
* @param[in]  exactMatchManagerId  - The Exact Match Manager id.
*                                    (APPLICABLE RANGES : 0..31)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*
*/
GT_STATUS prvCpssDxChExactMatchManagerHwHaTempDbCreate
(
    IN  GT_U32  exactMatchManagerId
);

/**
* @internal prvCpssDxChExactMatchManagerHwHaTempDbDelete function
* @endinternal
*
* @brief The function Delete the HA temporary database.
*
* @param[in]  exactMatchManagerId  - The Exact Match Manager id.
*                                    (APPLICABLE RANGES : 0..31)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*
*/
GT_STATUS prvCpssDxChExactMatchManagerHwHaTempDbDelete
(
    IN  GT_U32  exactMatchManagerId
);

/**
* @internal prvCpssDxChExactMatchManagerHwInvalidateNonTakenIndexFromHaTempDb function
* @endinternal
*
* @brief   This function go over the HA temporary database and
*          invalidate all non taken indexes in HW
*
* @param[in]  exactMatchManagerId  - The Exact Match Manager id.
*                                    (APPLICABLE RANGES : 0..31)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*
*/
GT_STATUS   prvCpssDxChExactMatchManagerHwInvalidateNonTakenIndexFromHaTempDb
(
        IN  GT_U32  exactMatchManagerId
);

/**
* @internal prvCpssDxChExactMatchManagerAacHwWriteEntry function
* @endinternal
*
* @brief   Write a whole EM entry to the HW using AAC method.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - the device number
* @param[in] tableType                - the specific table name
* @param[in] entryIndex               - index in the table
* @param[in] entryValuePtr            - (pointer to) the data that will be written to the table
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on failure.
* @retval GT_HW_ERROR              - on hardware error
*
*/
GT_STATUS prvCpssDxChExactMatchManagerAacHwWriteEntry
(
    IN GT_U8                   devNum,
    IN CPSS_DXCH_TABLE_ENT     tableType,
    IN GT_U32                  entryIndex,
    IN GT_U32                  *entryValuePtr
);
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvCpssDxChExactMatchManager_hw_h */


