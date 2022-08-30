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
* @file cpssDxChBrgFdbManager.h
*
* @brief FDB manager support - API definitions
*
* @version   1
********************************************************************************
*/
#ifndef __cpssDxChBrgFdbManager_h
#define __cpssDxChBrgFdbManager_h

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* include the FDB manager types */
#include <cpss/dxCh/dxChxGen/bridgeFdbManager/cpssDxChBrgFdbManagerTypes.h>

/**
* @internal cpssDxChBrgFdbManagerCreate function
* @endinternal
*
* @brief  The function creates the FDB Manager and its databases according to
*        input capacity structure. Internally all FDB global parameters / modes
*        initialized to their defaults (No HW access - just SW defaults that will
*        be later applied to registered devices).
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] fdbManagerId          - the FDB Manager id to associate with the newly created manager.
*                                    (APPLICABLE RANGES : 0..31)
* @param[in] capacityPtr           - (pointer to) parameters of the HW structure and capacity of its managed PPs FDBs
*                                    like number of indexes, the amount of hashes etc.
*                                    As FDB Manager is created and entries may be added before PP registered
*                                    we must know in advance the relevant capacity.
*                                    In Falcon for example we must know the shared memory allocation mode.
* @param[in] entryAttrPtr          - (pointer to) parameters that affects how HW/SW entries are
*                                    structured and specific fields behavior including:
*                                    MUX'd fields in MAC / IPMC entries format, SA / DA drop commands mode &
*                                    IP NH packet command.
* @param[in] learningPtr           - (pointer to) parameters that affects which sort of entries to learn etc.
*                                    (Learn MAC of Routed packets and MAC no-space updates).
* @param[in] lookupPtr             - (pointer to) parameters that affect lookup of entries: MAC VLAN Lookup mode
*                                    (none, single or double tags), IVL single-tag MSB mode.
*                                    And two new Falcon setting: IPv4 UC and IPv6 UC masks
*                                    (allow using FDB for prefixes lookup and not just exact match)
* @param[in] agingPtr              - (pointer to) parameters that affect refresh and aging
*                                      (Refresh Destination UC, Refresh Destination MC &Refresh IP UC)
*
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - wrong parameter
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_OUT_OF_CPU_MEM        - on fail to do Cpu memory allocation.
* @retval GT_ALREADY_EXIST         - if the FDB Manager id already exists.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgFdbManagerCreate
(
    IN GT_U32                                           fdbManagerId,
    IN CPSS_DXCH_BRG_FDB_MANAGER_CAPACITY_STC           *capacityPtr,
    IN CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_ATTRIBUTES_STC   *entryAttrPtr,
    IN CPSS_DXCH_BRG_FDB_MANAGER_LEARNING_STC           *learningPtr,
    IN CPSS_DXCH_BRG_FDB_MANAGER_LOOKUP_STC             *lookupPtr,
    IN CPSS_DXCH_BRG_FDB_MANAGER_AGING_STC              *agingPtr
);

/**
* @internal cpssDxChBrgFdbManagerDelete function
* @endinternal
*
* @brief  The function de-allocates specific FDB Manager Instance.
*         including all setting, entries, Data bases from all attached devices -
*         and return to initial state.
*         NOTE: the FDB manager will remove all HW entries from all the registered devices.
*
* @param[in] fdbManagerId            - the FDB Manager id.
*                                    (APPLICABLE RANGES : 0..31)
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - wrong parameter
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_INITIALIZED       - if the FDB Manager id is not initialized.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgFdbManagerDelete
(
    IN GT_U32 fdbManagerId
);

/**
* @internal cpssDxChBrgFdbManagerDevListAdd function
* @endinternal
*
* @brief   This function adding (registering) device(s) to specific FDB Manager Instance.
*          It can be in initialization time or in run-time in the context of Hot-Insertion or PP reset.
*          Note: the assumption is that in all use cases (Init / Reset / Hot-Insertion)
*          PP FDB is flushed and don't have any old MAC or other entries.
*          The only exception is the HA use-case (which will be handled by dedicated procedure).
*          Note: every time new device is added application may be required to call
*                in addition to this API, the per-ports APIs:
*                cpssDxChBrgFdbManagerPortLearningSet(...) and
*                cpssDxChBrgFdbManagerPortRoutingSet(...) with proper values
*                for each new device' port
*
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] fdbManagerId          - the FDB Manager id.
*                                    (APPLICABLE RANGES : 0..31)
* @param[in] devListArr[]          - the array of device ids to add to the FDB Manager.
* @param[in] numOfDevs             - the number of device ids in the array.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters , duplicated devices.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_ALREADY_EXIST         - if device already exist in DB
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_INITIALIZED       - if the FDB Manager id is not initialized.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device in devListArr
*
* @note
*   NONE
*
*/
GT_STATUS cpssDxChBrgFdbManagerDevListAdd
(
    IN GT_U32 fdbManagerId,
    IN GT_U8  devListArr[], /*arrSizeVarName=numOfDevs*/
    IN GT_U32 numOfDevs
);

/**
* @internal cpssDxChBrgFdbManagerDevListRemove function
* @endinternal
*
* @brief   This function removing (unregistering) device(s) from specific FDB Manager Instance
*          (in Hot Removal and reset etc..).
*          NOTE: the FDB manager will stop accessing to those devices (and will
*          not access to it during current function)
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] fdbManagerId          - the FDB Manager id.
*                                    (APPLICABLE RANGES : 0..31)
* @param[in] devListArr[]          - the array of device ids to remove from the FDB Manager.
* @param[in] numOfDevs             - the number of device ids in the array.
*
* @retval GT_OK                    - on success
* @retval GT_NO_SUCH               - if one of the devices not in DB.
* @retval GT_BAD_PARAM             - on wrong parameters , duplicated devices.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_NOT_INITIALIZED       - if the FDB Manager id is not initialized.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note
*   NONE
*
*/
GT_STATUS cpssDxChBrgFdbManagerDevListRemove
(
    IN GT_U32 fdbManagerId,
    IN GT_U8  devListArr[], /*arrSizeVarName=numOfDevs*/
    IN GT_U32 numOfDevs
);

/**
* @internal cpssDxChBrgFdbManagerEntryAdd function
* @endinternal
*
* @brief   This function adding entry to CPSS FDB Manager's database and HW.
*          All types of entries can be added using this API.
*          NOTE: the FDB manager will set the info to all the registered devices.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] fdbManagerId          - the FDB Manager id.
*                                    (APPLICABLE RANGES : 0..31)
* @param[in] entryPtr              - (pointer to) FDB entry format to be added.
* @param[in] paramsPtr             - (pointer to) extra info related to 'add entry' operation.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_ALREADY_EXIST         - the entry already exists.
* @retval GT_FULL                  - the table is FULL.
* @retval GT_LEARN_LIMIT_PORT_ERROR         - if learn limit on port reached.
* @retval GT_LEARN_LIMIT_TRUNK_ERROR        - if learn limit on trunk reached.
* @retval GT_LEARN_LIMIT_GLOBAL_EPORT_ERROR - if learn limit on global eport reached.
* @retval GT_LEARN_LIMIT_FID_ERROR          - if learn limit on fid reached.
* @retval GT_LEARN_LIMIT_GLOBAL_ERROR       - if learn limit globally reached.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_INITIALIZED       - if the FDB Manager id is not initialized.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note
*   NONE
*
*/
GT_STATUS cpssDxChBrgFdbManagerEntryAdd
(
    IN GT_U32                                           fdbManagerId,
    IN CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_STC              *entryPtr,
    IN CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_ADD_PARAMS_STC   *paramsPtr
);

/**
* @internal cpssDxChBrgFdbManagerEntryTempDelete function
* @endinternal
*
* @brief   This function deletes temporary entry from the HW.
*          NOTE: the FDB manager will remove if:
*           - The entry is temp entry
*           - Only present in HW (FDB Manager same index should be free)
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] fdbManagerId          - the FDB Manager id.
*                                    (APPLICABLE RANGES : 0..31)
* @param[in] entryPtr              - (pointer to) FDB entry format to be deleted.
*                                     NOTE: only the 'fdbEntryKey' part is used by the API.
* @param[in] tempEntryOffset       - Temporary entry offset in Hardware for the SP entry..
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_NOT_FOUND             - the entry was not found.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_INITIALIZED       - if the FDB Manager id is not initialized.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note
*   NONE
*
*/
GT_STATUS cpssDxChBrgFdbManagerEntryTempDelete
(
    IN GT_U32                                           fdbManagerId,
    IN CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_STC              *entryPtr,
    IN GT_U32                                           tempEntryOffset
);

/**
* @internal cpssDxChBrgFdbManagerEntryDelete function
* @endinternal
*
* @brief   This function delete entry from the CPSS FDB Manager's database and HW.
*          All types of entries can be deleted using this API.
*          NOTE: the FDB manager will remove the info from all the registered devices.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] fdbManagerId          - the FDB Manager id.
*                                    (APPLICABLE RANGES : 0..31)
* @param[in] entryPtr              - (pointer to) FDB entry format to be deleted.
*                                     NOTE: only the 'fdbEntryKey' part is used by the API.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_NOT_FOUND             - the entry was not found.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_INITIALIZED       - if the FDB Manager id is not initialized.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note
*   NONE
*
*/
GT_STATUS cpssDxChBrgFdbManagerEntryDelete
(
    IN GT_U32                                           fdbManagerId,
    IN CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_STC              *entryPtr
);

/**
* @internal cpssDxChBrgFdbManagerEntryGet function
* @endinternal
*
* @brief   The function return entry info from the manager by 'key'
*          NOTE: no HW accessing.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] fdbManagerId          - the FDB Manager id.
*                                    (APPLICABLE RANGES : 0..31)
* @param[in] entryPtr              - (pointer to) FDB entry format with the 'fdbEntryKey'.
*                                     NOTE: only the 'fdbEntryKey' part is used by the API.(as [in] parameter)
* @param[out] entryPtr             - (pointer to) FDB entry format with full info.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_NOT_FOUND             - if entry not found for that 'key'.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_NOT_INITIALIZED       - if the FDB Manager id is not initialized.
*
* @note
*   NONE
*
*/
GT_STATUS cpssDxChBrgFdbManagerEntryGet
(
    IN GT_U32                                           fdbManagerId,
    INOUT CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_STC           *entryPtr
);

/**
* @internal cpssDxChBrgFdbManagerEntryGetNext function
* @endinternal
*
* @brief   The function return first entry or next entry from previous one.
*          NOTE: 1. the 'list' is not sorted by key or hwIndex.
*                2. no HW accessing.
*           API for fetching next entry data either first one or next relative
*           to last entry retrieved.
*           FDB Manager hold internal pointer to the last entry retrieved and
*           it use that to locate next one (see FDB Manager Data Structure section for details).
*           Note that if that entry is deleted, the Delete API is moving this
*           pointer to previous entry (which can become NULL if that was the
*           first entry).
*           Once scan completed and API fetched all entries in the list it
*           returns GT_NO_MORE, Application can still call this API to see if
*           any additional entries has been added. It will keep getting
*           GT_NO_MORE until no additional entries or until restarted the scan
*           from beginning.
*           To start over from head of the list Application should pass True on
*           getFirst (its valid to restart the scan anytime - Regardless the
*           position of the scan pointer).
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] fdbManagerId          - the FDB Manager id.
*                                    (APPLICABLE RANGES : 0..31)
* @param[out] entryPtr             - (pointer to) FDB entry format with full info.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_NO_MORE               - if no more entries to retrieve
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_NOT_INITIALIZED       - if the FDB Manager id is not initialized.
*
* @note
*   NONE
*
*/
GT_STATUS cpssDxChBrgFdbManagerEntryGetNext
(
    IN GT_U32                                           fdbManagerId,
    IN GT_BOOL                                          getFirst,
    OUT CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_STC             *entryPtr
);

/**
* @internal cpssDxChBrgFdbManagerEntryUpdate function
* @endinternal
*
* @brief   The function updates existing entry in CPSS FDB Manager's database & HW.
*          All types of entries can be updated using this API.
*          Notes:
*          - Application can use this API to update entire entry data or just the source interface,
*          which is the recommended method when updating entry as a result of Entry Moved update event.
*          In this case when HW informs SW that dynamic MAC has changed, not all entry fields are being passed,
*          so to avoid overwriting previous values with defaults, application suggests to choose to overwrite only the source interface.
*          - API checks and not allows overwriting existing static entry with dynamic entry (with the same key) while the opposite is allowed.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] fdbManagerId          - the FDB Manager id.
*                                    (APPLICABLE RANGES : 0..31)
* @param[in] entryPtr              - (pointer to) FDB entry format with full info.
* @param[in] paramsPtr             - (pointer to) Update FDB entry parameters
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_INITIALIZED       - if the FDB Manager is not initialized
* @retval GT_NOT_ALLOWED           - if trying to overwrite static entry with dynamic one
* @retval GT_LEARN_LIMIT_PORT_ERROR         - if learn limit on port reached.
* @retval GT_LEARN_LIMIT_TRUNK_ERROR        - if learn limit on trunk reached.
* @retval GT_LEARN_LIMIT_GLOBAL_EPORT_ERROR - if learn limit on global eport reached.
* @retval GT_LEARN_LIMIT_FID_ERROR          - if learn limit on fid reached.
* @retval GT_LEARN_LIMIT_GLOBAL_ERROR       - if learn limit globally reached.
*
* @note
*   NONE
*
*/
GT_STATUS cpssDxChBrgFdbManagerEntryUpdate
(
    IN GT_U32                                           fdbManagerId,
    IN CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_STC              *entryPtr,
    IN CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_UPDATE_PARAMS_STC *paramsPtr
);

/**
* @internal cpssDxChBrgFdbManagerConfigGet function
* @endinternal
*
* @brief API for fetching current FDB Manager setting as they were defined in FDB Manager Create.
*
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] fdbManagerId          - the FDB Manager id.
*                                    (APPLICABLE RANGES : 0..31)
* @param[out] capacityPtr          - (pointer to) parameters of the HW structure and capacity of its managed PPs FDBs:
*                                    like number of indexes, the amount of hashes etc.
*                                    As FDB Manager is created and entries may be added before PP registered,
*                                    we must know in advance the relevant capacity.
*                                    In Falcon, for example, we must know the shared memory allocation mode.
* @param[out] entryAttrPtr         - (pointer to) parameters that affects how HW/SW entries are
*                                    structured and specific fields behavior including:
*                                    MUX'd fields in MAC / IPMC entries format, SA / DA drop commands mode and
*                                    IP NH packet command.
* @param[out] learningPtr          - (pointer to) parameters that affects which sort of entries to learn etc.
*                                    (Learn MAC of Routed packets and MAC no-space updates).
* @param[out] lookupPtr            - (pointer to) parameters that affect lookup of entries: MAC VLAN Lookup mode
*                                    (none, single or double tags), IVL single-tag MSB mode.
*                                    And two new Falcon setting: IPv4 UC and IPv6 UC masks
*                                    (allow using FDB for prefixes lookup and not just exact match)
* @param[out] agingPtr             - (pointer to) parameters that affect refresh and aging
*                                      (Refresh Destination UC, Refresh Destination MC &Refresh IP UC)
*
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
* @retval GT_BAD_PTR               - on NULL pointer.
*/
GT_STATUS cpssDxChBrgFdbManagerConfigGet
(
    IN  GT_U32                                          fdbManagerId,
    OUT CPSS_DXCH_BRG_FDB_MANAGER_CAPACITY_STC          *capacityPtr,
    OUT CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_ATTRIBUTES_STC  *entryAttrPtr,
    OUT CPSS_DXCH_BRG_FDB_MANAGER_LEARNING_STC          *learningPtr,
    OUT CPSS_DXCH_BRG_FDB_MANAGER_LOOKUP_STC            *lookupPtr,
    OUT CPSS_DXCH_BRG_FDB_MANAGER_AGING_STC             *agingPtr
);

/**
* @internal cpssDxChBrgFdbManagerDevListGet function
* @endinternal
*
* @brief API for fetching current FDB Manager registered devices (added by 'add device' API).
*
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] fdbManagerId          - the FDB Manager id.
*                                    (APPLICABLE RANGES : 0..31)
* @param[inout] numOfDevicesPtr    - (pointer to) the number of devices in the array deviceListArray as 'input parameter'
*                                    and actual number of devices as 'output parameter'.
* @param[out] deviceListArray[]    - array of registered devices (no more than list length).
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
* @retval GT_BAD_PTR               - on NULL pointer.
*/
GT_STATUS cpssDxChBrgFdbManagerDevListGet
(
    IN GT_U32           fdbManagerId,
    INOUT GT_U32        *numOfDevicesPtr,
    OUT GT_U8           deviceListArray[] /*arrSizeVarName=*numOfDevicesPtr*/
);

/**
* @internal cpssDxChBrgFdbManagerCountersGet function
* @endinternal
*
* @brief API for fetching FDB Manager counters.
*
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] fdbManagerId          - the FDB Manager id.
*                                    (APPLICABLE RANGES : 0..31)
* @param[out] countersPtr          - (pointer to) FDB Manager Counters.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
* @retval GT_BAD_PTR               - on NULL pointer.
*/
GT_STATUS cpssDxChBrgFdbManagerCountersGet
(
    IN GT_U32                                   fdbManagerId,
    OUT CPSS_DXCH_BRG_FDB_MANAGER_COUNTERS_STC *countersPtr
);

/**
* @internal cpssDxChBrgFdbManagerStatisticsGet function
* @endinternal
*
* @brief API for fetching FDB Manager statistics.
*
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] fdbManagerId          - the FDB Manager id.
*                                    (APPLICABLE RANGES : 0..31)
* @param[out] statisticsPtr        - (pointer to) FDB Manager Statistics.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
* @retval GT_BAD_PTR               - on NULL pointer.
*/
GT_STATUS cpssDxChBrgFdbManagerStatisticsGet
(
    IN GT_U32 fdbManagerId,
    OUT CPSS_DXCH_BRG_FDB_MANAGER_STATISTICS_STC *statisticsPtr
);

/**
* @internal cpssDxChBrgFdbManagerStatisticsClear function
* @endinternal
*
* @brief API for clearing FDB Manager statistics.
*
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] fdbManagerId          - the FDB Manager id.
*                                    (APPLICABLE RANGES : 0..31)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
*/
GT_STATUS cpssDxChBrgFdbManagerStatisticsClear
(
    IN GT_U32 fdbManagerId
);

/**
* @internal cpssDxChBrgFdbManagerPortLearningSet function
* @endinternal
*
* @brief   The function configures ports learning related attributes
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                - device number
* @param[in] portNum               - port number
* @param[in] learningPtr           - (pointer to)learning attributes specific for port to set
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum or portNum.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgFdbManagerPortLearningSet
(
    IN GT_U8                                        devNum,
    IN GT_PORT_NUM                                  portNum,
    IN CPSS_DXCH_BRG_FDB_MANAGER_PORT_LEARNING_STC *learningPtr
);

/**
* @internal cpssDxChBrgFdbManagerPortLearningGet function
* @endinternal
*
* @brief   The function gets ports learning related attributes
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                - device number
* @param[in] portNum               - port number
* @param[out] learningPtr          - (pointer to)learning attributes specific for port to get
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum or portNum.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgFdbManagerPortLearningGet
(
    IN  GT_U8                                        devNum,
    IN  GT_PORT_NUM                                  portNum,
    OUT CPSS_DXCH_BRG_FDB_MANAGER_PORT_LEARNING_STC *learningPtr
);

/**
* @internal cpssDxChBrgFdbManagerPortRoutingSet function
* @endinternal
*
* @brief   The function configures ports "FDB based" routing status
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                - device number
* @param[in] portNum               - port number
* @param[in] routingPtr            - (pointer to)routing status of the port to set.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum or portNum.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgFdbManagerPortRoutingSet
(
    IN GT_U8                                        devNum,
    IN GT_PORT_NUM                                  portNum,
    IN CPSS_DXCH_BRG_FDB_MANAGER_PORT_ROUTING_STC  *routingPtr
);

/**
* @internal cpssDxChBrgFdbManagerPortRoutingGet function
* @endinternal
*
* @brief   The function gets ports "FDB based" routing status
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                - device number
* @param[in] portNum               - port number
* @param[out] routingPtr           - (pointer to)routing status of the port to get.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum or portNum.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgFdbManagerPortRoutingGet
(
    IN  GT_U8                                        devNum,
    IN  GT_PORT_NUM                                  portNum,
    OUT CPSS_DXCH_BRG_FDB_MANAGER_PORT_ROUTING_STC  *routingPtr
);

/**
* @internal cpssDxChBrgFdbManagerLearningThrottlingGet function
* @endinternal
*
* @brief   The function gets the learning throttling attributes
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] fdbManagerId          - the FDB Manager id.
*                                    (APPLICABLE RANGES : 0..31)
* @param[out] dataPtr              - (pointer to) Learning throttling related attributes.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong fdbManagerId.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgFdbManagerLearningThrottlingGet
(
    IN  GT_U32                                              fdbManagerId,
    OUT CPSS_DXCH_BRG_FDB_MANAGER_LEARNING_THROTTLING_STC   *dataPtr
);

/**
* @internal cpssDxChBrgFdbManagerLearningScan function
* @endinternal
*
* @brief   The function gets the AUQ messages according to the input attributes.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] fdbManagerId          - the FDB Manager id.
*                                    (APPLICABLE RANGES : 0..31)
* @param[in] paramsPtr             - the learning scan attributes.
* @param[out] entriesLearningArray - (pointer to) Update event array.
* @param[out] entriesLearningNumPtr- (pointer to) Number of Update events filled in array.
*
* @retval GT_OK                    - on success
* @retval GT_NO_MORE               - if no more entries to retrieve
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum or portNum.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgFdbManagerLearningScan
(
    IN  GT_U32                                             fdbManagerId,
    IN  CPSS_DXCH_BRG_FDB_MANAGER_LEARNING_SCAN_PARAMS_STC *paramsPtr,
    OUT CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_UPDATE_EVENT_STC   entriesLearningArray[],
    OUT GT_U32                                             *entriesLearningNumPtr
);

/**
* @internal cpssDxChBrgFdbManagerTransplantScan function
* @endinternal
*
* @brief   The function scans the entire FDB (Till maximal per scan allowed).
*          Transplant(Change the source info) the filtered entries.
*          (Applicable entry types - MAC Unicast(static/dynamic), IPv4/6 Unicast)
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] fdbManagerId               - the FDB Manager id.
*                                         (APPLICABLE RANGES : 0..31)
* @param[in] scanStart                  - Indicates the starting point of the scan.
*                                         GT_TRUE  - Start from the beginning of the used list.
*                                         GT_FALSE - Start from the last scanned point.
* @param[in] paramsPtr                  - the transplant scan attributes.
* @param[out] entriesTransplantedArray  - (pointer to) Update event array.
*                                         This is optional (can be NULL if application do not require it).
*                                         If used, should have memory for at least "max entries allowed per transplant scan".
* @param[out] entriesTransplantedNumPtr - (pointer to) Number of Update events filled in array.
*                                         This is optional (can be NULL if application do not require it).
*
* @retval GT_OK                    - on success
* @retval GT_NO_MORE               - if no more entries to transplant
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong fdbManagerId and paramsPtr data
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note
*          Invalid Filter table for entry type
*          +-------------------------+------------+-------------+
*          |                         |FID         |Interface    |
*          +-------------------------+------------+-------------+
*          |MAC Unicast              |Valid       |Valid        |
*          |IPv4/6 Unicast           |NA          |Valid        |
*          +-------------------------+------------+-------------+
*/
GT_STATUS cpssDxChBrgFdbManagerTransplantScan
(
    IN  GT_U32                                                fdbManagerId,
    IN  GT_BOOL                                               scanStart,
    IN  CPSS_DXCH_BRG_FDB_MANAGER_TRANSPLANT_SCAN_PARAMS_STC *paramsPtr,
    OUT CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_UPDATE_EVENT_STC      entriesTransplantedArray[],
    OUT GT_U32                                               *entriesTransplantedNumPtr
);

/**
* @internal cpssDxChBrgFdbManagerDeleteScan function
* @endinternal
*
* @brief   The function scans the entire FDB (Till maximal per scan allowed).
*          Delete the filtered entries.
*          (Applicable entry types - MAC Unicast(static/dynamic), MAC Multicast, IPv4/6 Unicast, IPv4/6 Multicast)
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] fdbManagerId          - the FDB Manager id.
*                                    (APPLICABLE RANGES : 0..31)
* @param[in] scanStart             - Indicates the starting point of the scan.
*                                    GT_TRUE  - Start from the beginning of the used list.
*                                    GT_FALSE - Start from the last scanned point.
* @param[in] paramsPtr             - the delete scan attributes.
* @param[out] entriesDeletedArray  - (pointer to) Update event array.
*                                    This is optional (can be NULL if application do not require it).
*                                    If used, should have memory for at least "max entries allowed per delete scan".
* @param[out] entriesDeletedNumPtr - (pointer to) Number of Update events filled in array.
*                                    This is optional (can be NULL if application do not require it).
*
* @retval GT_OK                    - on success
* @retval GT_NO_MORE               - if no more entries to delete
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong fdbManagerId and paramsPtr data
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note
*          Invalid Filter table for entry type
*          +-------------------------+-------+---------+-------------+--------+---------+------+
*          |                         |FID    |Device   |ePort/Trunk  |isTrunk |Vid1     |User  |
*          +-------------------------+------------+------------------+--------+---------+------+
*          |MAC Unicast              |Valid  |Valid    |Valid        |Valid   |Valid    |Valid |
*          |MAC Multicast            |Valid  |NA       |NA           |NA      |Valid    |Valid |
*          |IPv4/6 Multicast         |Valid  |NA       |NA           |NA      |NA       |Valid |
*          |IPv4/6 Unicast           |NA     |Valid    |Valid        |Valid   |NA       |NA    |
*          +-------------------------+-------+---------+-------------+--------+---------+------+
*/
GT_STATUS cpssDxChBrgFdbManagerDeleteScan
(
    IN  GT_U32                                            fdbManagerId,
    IN  GT_BOOL                                           scanStart,
    IN  CPSS_DXCH_BRG_FDB_MANAGER_DELETE_SCAN_PARAMS_STC *paramsPtr,
    OUT CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_UPDATE_EVENT_STC  entriesDeletedArray[],
    OUT GT_U32                                           *entriesDeletedNumPtr
);

/**
* @internal cpssDxChBrgFdbManagerAgingScan function
* @endinternal
*
* @brief   The function scans the entire FDB and process age-out for the filtered entries.
*          (Applicable entry types - MAC Unicast(dynamic), MAC Multicast, IPv4/6 Unicast, IPv4/6 Multicast)
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] fdbManagerId          - the FDB Manager id.
*                                    (APPLICABLE RANGES : 0..31)
* @param[in] paramsPtr             - the aging scan attributes.
* @param[out] entriesAgedoutArray  - (pointer to) Update event array.
*                                    This is optional (can be NULL if application do not require it).
*                                    If used, should have memory for at least "max entries allowed per aging scan".
* @param[out] entriesAgedoutNumPtr - (pointer to) Number of Update events filled in array.
*                                    This is optional (can be NULL if application do not require it).
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong fdbManagerId and paramsPtr data
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note
*          Invalid Filter table for entry type
*          +-------------------------+-------+---------+-------------+--------+---------+------+
*          |                         |FID    |Device   |ePort/Trunk  |isTrunk |Vid1     |User  |
*          +-------------------------+------------+------------------+--------+---------+------+
*          |MAC Unicast              |Valid  |Valid    |Valid        |Valid   |Valid    |Valid |
*          |MAC Multicast            |Valid  |NA       |NA           |NA      |Valid    |Valid |
*          |IPv4/6 Multicast         |Valid  |NA       |NA           |NA      |NA       |Valid |
*          |IPv4/6 Unicast           |NA     |Valid    |Valid        |Valid   |NA       |NA    |
*          +-------------------------+-------+---------+-------------+--------+---------+------+
*/
GT_STATUS cpssDxChBrgFdbManagerAgingScan
(
    IN  GT_U32                                           fdbManagerId,
    IN  CPSS_DXCH_BRG_FDB_MANAGER_AGING_SCAN_PARAMS_STC *paramsPtr,
    OUT CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_UPDATE_EVENT_STC entriesAgedoutArray[],
    OUT GT_U32                                          *entriesAgedoutNumPtr
);

/**
* @internal cpssDxChBrgFdbManagerDatabaseCheck function
* @endinternal
*
* @brief   The function verifies the FDB manager DB/HW according to the input test types.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] fdbManagerId          - The FDB Manager id.
*                                    (APPLICABLE RANGES : 0..31)
* @param[in] checksPtr             - (Pointer to) DB check attributes
* @param[out] resultArray          - (pointer to) Test result array.
* @param[out] errorNumPtr          - (pointer to) Number of error types filled in resultArray
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong fdbManagerId and checksPtr data
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChBrgFdbManagerDatabaseCheck
(
    IN  GT_U32                                           fdbManagerId,
    IN  CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_STC          *checksPtr,
    OUT CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_RESULT_ENT   resultArray[CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS___LAST___E],
    OUT GT_U32                                           *errorNumPtr
);

/**
* @internal cpssDxChBrgFdbManagerHighAvailabilityEntriesSync function
* @endinternal
*
* @brief   The function updates FDB Manager DB according to HW states.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] fdbManagerId          - The FDB Manager id.
*                                    (APPLICABLE RANGES : 0..31)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong fdbManagerId data
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChBrgFdbManagerHighAvailabilityEntriesSync
(
    IN  GT_U32 fdbManagerId
);

/**
* @internal cpssDxChBrgFdbManagerEntryRewrite function
* @endinternal
*
* @brief   The function rewrites FDB Manager HW entries according to SW DB.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] fdbManagerId          - The FDB Manager id.
*                                    (APPLICABLE RANGES : 0..31)
* @param[in] entriesIndexesArray   - The array of HW index.
* @param[in] entriesIndexesNum     - number of HW indexes present in index array.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_BAD_PARAM             - on wrong fdbManagerId data
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChBrgFdbManagerEntryRewrite
(
    IN  GT_U32 fdbManagerId,
    IN  GT_U32 entriesIndexesArray[], /* arrSizeVarName=entriesIndexesNum */
    IN  GT_U32 entriesIndexesNum
);

/**
* @internal cpssDxChBrgFdbManagerLimitSet function
* @endinternal
*
* @brief   The function configures the maximal allowed dynamic UC MAC entries to
*           a specific limit info.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                - device number
* @param[in] limitInfoPtr          - (pointer to) limit info : global/fid/trunk/global-eport/port and the 'index'
* @param[in] limitValue            - the limit for this limit info
* @param[out] currentValuePtr      - (pointer to) the current number of dynamic UC mac entries in FDB for the specific limit info.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum or other param in limitInfoPtr.
* @retval GT_NOT_SUPPORTED         - the manager was created without support for 'learn limits'
*                                    or on global eport case : The manager for SHADOW_TYPE_SIP4 hold no 'global eport' support.
*                                    or on global eport case : The manager was created 'global eport' support.
* @retval GT_NO_SUCH               - hwDevNum not exists (no device that added by 'dev list add' hold this hwDevNum)
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgFdbManagerLimitSet
(
    IN GT_U32  fdbManagerId,
    IN CPSS_DXCH_BRG_FDB_MANAGER_DYNAMIC_MAC_UC_LIMIT_STC   *limitInfoPtr,
    IN GT_U32   limitValue,
    OUT GT_U32  *currentValuePtr
);

/**
* @internal cpssDxChBrgFdbManagerLimitGet function
* @endinternal
*
* @brief   The function get the maximal allowed dynamic UC MAC entries on
*           a specific limit info.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                - device number
* @param[in] limitInfoPtr          - (pointer to) limit info : global/fid/trunk/global-eport/port and the 'index'
* @param[out] limitValuePtr        - (pointer to) the limit for this index of limit info
* @param[out] currentValuePtr      - (pointer to) the current number of dynamic UC mac entries in FDB for the specific limit info.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum or other param in limitInfoPtr.
* @retval GT_NOT_SUPPORTED         - the manager was created without support for 'learn limits'
*                                    or on global eport case : The manager for SHADOW_TYPE_SIP4 hold no 'global eport' support.
*                                    or on global eport case : The manager was created 'global eport' support.
* @retval GT_NO_SUCH               - hwDevNum not exists (no device that added by 'dev list add' hold this hwDevNum)
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgFdbManagerLimitGet
(
    IN GT_U32  fdbManagerId,
    IN CPSS_DXCH_BRG_FDB_MANAGER_DYNAMIC_MAC_UC_LIMIT_STC   *limitInfoPtr,
    OUT GT_U32  *limitValuePtr,
    OUT GT_U32  *currentValuePtr
);

/**
* @internal cpssDxChBrgFdbManagerConfigUpdate function
* @endinternal
*
* @brief API for updating global FDB Manager settings which were set in FDB Manager Create initialization phase.
*        The API includes only small subset of the Create API global parameters –
*        these are attributes which does not require special handling at the
*        FDB Manager, nor it contradicts with entries already installed into HW.
*        These settings can be changed in runtime, without the need to delete and
*        recreate the manager (like in case that changing hashing parameters for
*        example).
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] fdbManagerId          - the FDB Manager id.
*                                    (APPLICABLE RANGES : 0..31)
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
* @retval GT_BAD_PARAM             - wrong parameter
* @retval GT_BAD_PTR               - when entryAttrPtr and learningPtr and agingPtr are NULL pointers.
*/
GT_STATUS cpssDxChBrgFdbManagerConfigUpdate
(
    IN  GT_U32                                                        fdbManagerId,
    IN CPSS_DXCH_BRG_FDB_MANAGER_CONFIG_UPDATE_ENTRY_ATTRIBUTES_STC  *entryAttrPtr,
    IN CPSS_DXCH_BRG_FDB_MANAGER_CONFIG_UPDATE_LEARNING_STC          *learningPtr,
    IN CPSS_DXCH_BRG_FDB_MANAGER_CONFIG_UPDATE_AGING_STC             *agingPtr
);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssDxChBrgFdbMananer_h */


