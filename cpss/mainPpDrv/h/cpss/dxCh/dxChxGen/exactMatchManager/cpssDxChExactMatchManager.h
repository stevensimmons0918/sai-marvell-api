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
* @file cpssDxChExactMatchManager.h
*
* @brief Exact Match  manager support - API definitions
*
* @version   1
********************************************************************************
*/
#ifndef __cpssDxChExactMatchManager_h
#define __cpssDxChExactMatchManager_h

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* include the Exact Match manager types */
#include <cpss/dxCh/dxChxGen/exactMatchManager/cpssDxChExactMatchManagerTypes.h>

/**
* @internal cpssDxChExactMatchManagerCreate function
* @endinternal
*
* @brief The function creates the Exact Match Manager and its
*        databases according to input capacity structure.
*        Internally all Exact Match global parameters / modes
*        initialized to their defaults (No HW access - just SW
*        defaults that will be later applied to registered devices).
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] exactMatchManagerId   - the Exact Match Manager id to associate with
*                                    the newly created manager.
*                                    (APPLICABLE RANGES : 0..31)
* @param[in] capacityPtr           - (pointer to) parameters of the HW structure and
*                                    capacity of its managed PPs Exact Matchs like number of
*                                    indexes, the amount of hashes etc. As Exact Match
*                                    Manager is created and entries may be added before
*                                    PP registered we must know in advance the relevant
*                                    capacity. In Falcon for example we must know the
*                                    shared memory allocation mode.
* @param[in] lookupPtr             - (pointer to) parameters that affect lookup of entries:
*                                    clients and their profiles
* @param[in] entryAttrPtr          - (pointer to) parameters that affect exact match entries:
*                                    expanded configuration
* @param[in] agingPtr              - (pointer to) parameters that affect refresh and aging
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - wrong parameter
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_OUT_OF_CPU_MEM        - on fail to do Cpu memory allocation.
* @retval GT_ALREADY_EXIST         - if the Exact Match Manager id already exists.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChExactMatchManagerCreate
(
    IN GT_U32                                                exactMatchManagerId,
    IN CPSS_DXCH_EXACT_MATCH_MANAGER_CAPACITY_STC            *capacityPtr,
    IN CPSS_DXCH_EXACT_MATCH_MANAGER_LOOKUP_STC              *lookupPtr,
    IN CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_ATTRIBUTES_STC    *entryAttrPtr,
    IN CPSS_DXCH_EXACT_MATCH_MANAGER_AGING_STC               *agingPtr
);
/**
* @internal cpssDxChExactMatchManagerDelete function
* @endinternal
*
* @brief  The function de-allocates specific Exact Match
*         Manager Instance. including all setting, entries, Data
*         bases from all attached devices - and return to
*         initial state. NOTE: the Exact Match manager will
*         remove all HW entries from all the registered devices.
*
* @param[in] exactMatchManagerId   - the Exact Match Manager id.
*                                   (APPLICABLE RANGES :0..31)
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - wrong parameter
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_INITIALIZED       - if the Exact Match Manager id is not initialized.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChExactMatchManagerDelete
(
    IN GT_U32 exactMatchManagerId
);
/**
* @internal cpssDxChExactMatchManagerDevListAdd function
* @endinternal
*
* @brief   This function adding (registering) device(s) to specific Exact Match Manager Instance.
*          It can be in initialization time or in run-time in the context of Hot-Insertion or PP reset.
*          Note: the assumption is that in all use cases (Init / Reset / Hot-Insertion) PP Exact Match
*          is flushed and don't have any old other entries. The only exception is the HA use-case
*          (which will be handled by dedicated procedure).
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] exactMatchManagerId   - the Exact Match Manager id.
*                                    (APPLICABLE RANGES : 0..31)
* @param[in] pairListArr[]         - the array of pairs (device ids + portGroupBmp)
*                                    to add to the Exact Match Manager.
* @param[in] numOfPairs            - the number of pairs in the array.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters , duplicated devices.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_ALREADY_EXIST         - if device already exist in DB
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_INITIALIZED       - if the Exact Match Manager id is not initialized.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device in pairListArr
*
* @note
*   NONE
*
*/
GT_STATUS cpssDxChExactMatchManagerDevListAdd
(
    IN GT_U32                                                   exactMatchManagerId,
    IN CPSS_DXCH_EXACT_MATCH_MANAGER_DEV_PORT_GROUP_PAIR_STC    pairListArr[],/*arrSizeVarName=numOfPairs*/
    IN GT_U32                                                   numOfPairs
);
/**
* @internal cpssDxChExactMatchManagerDevListRemove function
* @endinternal
*
* @brief   This function removing (unregistering) device(s) from
*          specific Exact Match Manager Instance (in Hot Removal
*          and reset etc..). NOTE: the Exact Match manager will
*          stop accessing to those devices (and will not access
*          to it during current function)
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] exactMatchManagerId   - the Exact Match Manager id.
*                                    (APPLICABLE RANGES : 0..31)
* @param[in] pairListArr[]         - the array of pairs (device ids + portGroupBmp)
*                                    to remove from the Exact Match Manager.
* @param[in] numOfPairs            - the number of pairs in the array.
*
* @retval GT_OK                    - on success
* @retval GT_NO_SUCH               - if one of the devices not in DB.
* @retval GT_BAD_PARAM             - on wrong parameters , duplicated devices.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_NOT_INITIALIZED       - if the Exact Match Manager id is not initialized.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note
*   NONE
*
*/
GT_STATUS cpssDxChExactMatchManagerDevListRemove
(
    IN GT_U32                                                   exactMatchManagerId,
    IN CPSS_DXCH_EXACT_MATCH_MANAGER_DEV_PORT_GROUP_PAIR_STC    pairListArr[],/*arrSizeVarName=numOfPairs*/
    IN GT_U32                                                   numOfPairs
);

/**
* @internal cpssDxChExactMatchManagerEntryAdd function
* @endinternal
*
* @brief   This function adding entry to CPSS Exact Match Manager's database and HW.
*          All types of entries can be added using this API.
*          NOTE: the Exact Match manager will set the info to all the registered devices.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] exactMatchManagerId   - the Exact Match Manager id.
*                                    (APPLICABLE RANGES : 0..31)
* @param[in] entryPtr              - (pointer to) Exact Match entry format to be added.
* @param[in] paramsPtr             - (pointer to) extra info related to 'add entry' operation.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_ALREADY_EXIST         - the entry already exists.
* @retval GT_FULL                  - the table is FULL.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_INITIALIZED       - if the Exact Match Manager id is not initialized.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note
*   NONE
*
*/
GT_STATUS cpssDxChExactMatchManagerEntryAdd
(
    IN GT_U32                                               exactMatchManagerId,
    IN CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_STC              *entryPtr,
    IN CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_ADD_PARAMS_STC   *paramsPtr
);
/**
* @internal cpssDxChExactMatchManagerEntryUpdate function
* @endinternal
*
* @brief   The function updates existing entry in CPSS Exact Match Manager's database & HW.
*          All types of entries can be updated using this API.
*          The function only updates the action of the entry.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] exactMatchManagerId   - the Exact Match Manager id.
*                                    (APPLICABLE RANGES : 0..31)
* @param[in] entryPtr              - (pointer to) Exact Match entry format with full info.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_INITIALIZED       - if the Exact Match Manager is not initialized
* @retval GT_NOT_ALLOWED           - if trying to overwrite static entry with dynamic one
*
* @note
*   NONE
*
*/
GT_STATUS cpssDxChExactMatchManagerEntryUpdate
(
    IN GT_U32                                                exactMatchManagerId,
    IN CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_STC               *entryPtr
);
/**
* @internal cpssDxChExactMatchManagerEntryDelete function
* @endinternal
*
* @brief   This function delete entry from the CPSS Exact Match Manager's database and HW.
*          All types of entries can be deleted using this API.
*          NOTE: the Exact Match manager will remove the info from all the registered devices.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] exactMatchManagerId   - the Exact Match Manager id.
*                                    (APPLICABLE RANGES : 0..31)
* @param[in] entryPtr              - (pointer to) Exact Match entry format to be deleted.
*                                     NOTE: only the 'exactMatchEntryKey' part is used by the API.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_NOT_FOUND             - the entry was not found.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_INITIALIZED       - if the Exact Match Manager id is not initialized.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note
*   NONE
*
*/
GT_STATUS cpssDxChExactMatchManagerEntryDelete
(
    IN GT_U32                                           exactMatchManagerId,
    IN CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_STC          *entryPtr
);
/**
* @internal cpssDxChExactMatchManagerEntryGet function
* @endinternal
*
* @brief   The function return entry info from the manager by 'key'
*          NOTE: no HW accessing.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] exactMatchManagerId   - the Exact Match Manager id.
*                                    (APPLICABLE RANGES : 0..31)
* @param[in] entryPtr              - (pointer to) Exact Match entry format with the 'exactMatchEntryKey'.
*                                     NOTE: only the 'exactMatchEntryKey' part is used by the API.(as [in] parameter)
* @param[out] entryPtr             - (pointer to) Exact Match entry format with full info.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_NOT_FOUND             - if entry not found for that 'key'.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_NOT_INITIALIZED       - if the Exact Match Manager id is not initialized.
*
* @note
*   NONE
*
*/
GT_STATUS cpssDxChExactMatchManagerEntryGet
(
    IN      GT_U32                                        exactMatchManagerId,
    INOUT   CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_STC       *entryPtr
);
/**
* @internal cpssDxChExactMatchManagerEntryGetNext function
* @endinternal
*
* @brief   The function return first entry or next entry from previous one.
*          NOTE: 1. the 'list' is sorted per insertion order
*                2. no HW accessing.
*           API for fetching next entry data either first one or next relative
*           to last entry retrieved.
*           Once scan completed and API fetched all entries in
*           the list it returns GT_NO_MORE, Application can
*           still call this API to see if any additional entries
*           has been added. It will keep getting GT_NO_MORE
*           until no additional entries or until restarted the
*           scan from beginning. To start over from head of the
*           list Application should pass True on getFirst (its
*           valid to restart the scan anytime - Regardless the
*           position of the scan pointer).
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] exactMatchManagerId   - the Exact Match Manager id.
*                                    (APPLICABLE RANGES : 0..31)
* @param[in] getFirst              - To start over from head of the list
*                                    should pass True on getFirst
* @param[out] entryPtr             - (pointer to) Exact Match entry format with full info.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_NO_MORE               - if no more entries to retrieve
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_NOT_INITIALIZED       - if the Exact Match Manager id is not initialized.
*
* @note
*   NONE
*
*/
GT_STATUS cpssDxChExactMatchManagerEntryGetNext
(
    IN  GT_U32                                          exactMatchManagerId,
    IN  GT_BOOL                                         getFirst,
    OUT CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_STC         *entryPtr
);
/**
* @internal cpssDxChExactMatchManagerExpandedActionUpdate function
* @endinternal
*
* @brief   The function set the Expanded Action table.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] exactMatchManagerId   - the Exact Match Manager id.
*                                    (APPLICABLE RANGES : 0..31)
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
*  None
*/
GT_STATUS cpssDxChExactMatchManagerExpandedActionUpdate
(
    IN  GT_U32                                                          exactMatchManagerId,
    IN  GT_U32                                                          expandedActionIndex,
    IN  CPSS_DXCH_EXACT_MATCH_MANAGER_EXPANDED_ACTION_SET_PARAMS_STC    *paramsPtr
);
/**
* @internal cpssDxChExactMatchManagerExpandedActionGet function
* @endinternal
*
* @brief  The function return the Expanded Action table entry
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] exactMatchManagerId   - the Exact Match Manager id.
*                                    (APPLICABLE RANGES : 0..31)
* @param[in] expandedActionIndex   - Exact Match Expander table index
*                                    (APPLICABLE RANGES:0..15)
* @param[out] paramsPtr             - the expanded action attributes.
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
*  None
*/
GT_STATUS cpssDxChExactMatchManagerExpandedActionGet
(
    IN  GT_U32                                                          exactMatchManagerId,
    IN  GT_U32                                                          expandedActionIndex,
    OUT CPSS_DXCH_EXACT_MATCH_MANAGER_EXPANDED_ACTION_SET_PARAMS_STC    *paramsPtr
);
/**
* @internal cpssDxChExactMatchManagerConfigGet function
* @endinternal
*
* @brief API for fetching current Exact Match Manager setting as they
*        were defined in Exact Match Manager Create.
*
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] exactMatchManagerId   - the Exact Match Manager id.
*                                    (APPLICABLE RANGES : 0..31)
* @param[out] capacityPtr          - (pointer to) parameters of the HW structure and capacity of its managed
*                                    PPs EMs: like number of indexes, the amount of hashes etc. As Exact Match
*                                    Manager is created and entries may be added before PP registered,
*                                    we must know in advance the relevant capacity. In Falcon, for
*                                    example, we must know the shared memory allocation mode.
* @param[out] lookupPtr            - (pointer to) parameters that affect lookup of entries
* @param[in]  entryAttrPtr         - (pointer to) parameters that affect exact match entries:
*                                    expanded configuration
* @param[out] agingPtr             - (pointer to) parameters that affect refresh and aging
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_NOT_INITIALIZED       - if the Exact Match Manager is not initialized
*/
GT_STATUS cpssDxChExactMatchManagerConfigGet
(
    IN  GT_U32                                              exactMatchManagerId,
    OUT CPSS_DXCH_EXACT_MATCH_MANAGER_CAPACITY_STC          *capacityPtr,
    OUT CPSS_DXCH_EXACT_MATCH_MANAGER_LOOKUP_STC            *lookupPtr,
    OUT CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_ATTRIBUTES_STC  *entryAttrPtr,
    OUT CPSS_DXCH_EXACT_MATCH_MANAGER_AGING_STC             *agingPtr
);
/**
* @internal cpssDxChExactMatchManagerDevListGet function
* @endinternal
*
* @brief API for fetching current Exact Match Manager registered devices
*        (added by 'add device' API).
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] exactMatchManagerId   - the Exact Match Manager id.
*                                    (APPLICABLE RANGES : 0..31)
* @param[inout] numOfPairsPtr      - (pointer to) the number of devices in the array pairListArr as 'input parameter'
*                                    and actual number of pairs as 'output parameter'.
* @param[out] pairListManagedArray[]- array of registered pairs (no more than list length).

* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_NOT_INITIALIZED       - if the Exact Match Manager is not initialized
*/
GT_STATUS cpssDxChExactMatchManagerDevListGet
(
    IN    GT_U32                                                   exactMatchManagerId,
    INOUT GT_U32                                                   *numOfPairsPtr,
    OUT   CPSS_DXCH_EXACT_MATCH_MANAGER_DEV_PORT_GROUP_PAIR_STC    pairListManagedArray[]/*maxArraySize=128*/
);
/**
* @internal cpssDxChExactMatchManagerCountersGet function
* @endinternal
*
* @brief API for fetching Exact Match Manager counters.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] exactMatchManagerId    -the Exact Match Manager id.
*                                    (APPLICABLE RANGES : 0..31)
* @param[out] countersPtr           -(pointer to) Exact Match Manager Counters.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_NOT_INITIALIZED       - if the Exact Match Manager is not initialized
*/
GT_STATUS cpssDxChExactMatchManagerCountersGet
(
    IN  GT_U32                                      exactMatchManagerId,
    OUT CPSS_DXCH_EXACT_MATCH_MANAGER_COUNTERS_STC  *countersPtr
);
/**
* @internal cpssDxChExactMatchManagerStatisticsGet function
* @endinternal
*
* @brief API for fetching Exact Match Manager statistics.
*
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in]exctMatchManagerId  - the Exact Match Manager id.
*                                 (APPLICABLE RANGES : 0..31)
* @param[out]statisticsPtr      - (pointer to) Exact Match Manager statistics.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_NOT_INITIALIZED       - if the Exact Match Manager is not initialized
*/
GT_STATUS cpssDxChExactMatchManagerStatisticsGet
(
    IN GT_U32                                           exactMatchManagerId,
    OUT CPSS_DXCH_EXACT_MATCH_MANAGER_STATISTICS_STC    *statisticsPtr
);

/**
* @internal cpssDxChExactMatchManagerStatisticsClear function
* @endinternal
*
* @brief API for clearing Exact Match Manager statistics.
*
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] exactMatchManagerId   - the Exact Match Manager id.
*                                    (APPLICABLE RANGES : 0..31)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
* @retval GT_NOT_INITIALIZED       - if the Exact Match Manager is not initialized
*/
GT_STATUS cpssDxChExactMatchManagerStatisticsClear
(
    IN GT_U32 exactMatchManagerId
);
/**
* @internal cpssDxChExactMatchManagerDatabaseCheck function
* @endinternal
*
* @brief Run -time API that can be triggered to test various aspects
*        of Exact Match Manager. Tests includes things such as
*        internal DB, sync with HW, counters correctness etc.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] exactMatchManagerId   - the Exact Match Manager id.
*                                    (APPLICABLE RANGES : 0..31)
* @param[in] checksPtr             - (Pointer to) DB check attributes
* @param[out] resultArray          - (pointer to) Test result array.
* @param[out] errorNumberPtr       - (pointer to) Number of error types filled in resultArray
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
* @retval GT_NOT_INITIALIZED       - if the Exact Match Manager is not initialized
*/
GT_STATUS cpssDxChExactMatchManagerDatabaseCheck
(
    IN  GT_U32                                                  exactMatchManagerId,
    IN  CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_STC             *checksPtr,
    OUT CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_RESULT_ENT      resultArray[CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS___LAST___E],
    OUT GT_U32                                                  *errorNumberPtr
);
/**
* @internal cpssDxChExactMatchManagerDeleteScan function
* @endinternal
*
* @brief  The function scans the entire Exact Match (Till maximal per
*         scan allowed). Delete the filtered entries.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] exactMatchManagerId   - the Exact Match Manager id.
*                                    (APPLICABLE RANGES : 0..31)
* @param[in] exactMatchScanStart   - Indicates the starting point of the scan.
*                                    GT_TRUE  - Start from the beginning of the used list.
*                                    GT_FALSE - Start from the last scanned point.
* @param[in] paramsPtr             - the delete scan attributes.
* @param[out] entriesDeletedArray  - (pointer to) Update event array.
*                                    This is optional (can be NULL if application do not require it).
*                                    If used, should have memory for at least "max entries allowed per delete scan".
* @param[out] entriesDeletedNumberPtr - (pointer to) Number of Update events filled in array.
*                                    This is optional (can be NULL if application do not require it).
*
* @retval GT_OK                    - on success
* @retval GT_NO_MORE               - if no more entries to delete
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong exactMatchManagerId and paramsPtr data
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_INITIALIZED       - if the Exact Match Manager is not initialized
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note
*   NONE
*/
GT_STATUS cpssDxChExactMatchManagerDeleteScan
(
    IN  GT_U32                                                  exactMatchManagerId,
    IN  GT_BOOL                                                 exactMatchScanStart,
    IN  CPSS_DXCH_EXACT_MATCH_MANAGER_DELETE_SCAN_PARAMS_STC    *paramsPtr,
    OUT CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_UPDATE_EVENT_STC    entriesDeletedArray[],
    OUT GT_U32                                                  *entriesDeletedNumberPtr
);
/**
* @internal cpssDxChExactMatchManagerAgingScan function
* @endinternal
*
* @brief   The function scans the entire Exact Match and process age-out for the filtered entries.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] exactMatchManagerId   - the Exact Match Manager id.
*                                    (APPLICABLE RANGES : 0..31)
* @param[in] paramsPtr             - the aging scan attributes.
* @param[out] entriesAgedOutArray  - (pointer to) Update event array.
*                                    This is optional (can be NULL if application do not require it).
*                                    If used, should have memory for at least "max entries allowed per aging scan".
* @param[out] entriesAgedOutNumPtr - (pointer to) Number of Update events filled in array.
*                                    This is optional (can be NULL if application do not require it).
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
*   NONE
*/
GT_STATUS cpssDxChExactMatchManagerAgingScan
(
    IN  GT_U32                                                  exactMatchManagerId,
    IN  CPSS_DXCH_EXACT_MATCH_MANAGER_AGING_SCAN_PARAMS_STC     *paramsPtr,
    OUT CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_UPDATE_EVENT_STC    entriesAgedOutArray[],
    OUT GT_U32                                                  *entriesAgedOutNumPtr
);

/**
* @internal cpssDxChExactMatchManagerEntryRewrite function
* @endinternal
*
* @brief   The function rewrites Exact Match Manager HW entries according to SW DB.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] exactMatchManagerId   - The Exact Match Manager id.
*                                    (APPLICABLE RANGES : 0..31)
* @param[in] entriesIndexesArray   - The array of HW index.
* @param[in] entriesIndexesNum     - number of HW indexes present in index array.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_BAD_PARAM             - on wrong exactMatchManagerId data
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChExactMatchManagerEntryRewrite
(
    IN  GT_U32 exactMatchManagerId,
    IN  GT_U32 entriesIndexesArray[], /* arrSizeVarName=entriesIndexesNum */
    IN  GT_U32 entriesIndexesNum
);
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssDxChExactMatchMananer_h */


