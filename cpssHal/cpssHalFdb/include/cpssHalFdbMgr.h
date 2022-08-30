/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
*******************************************************************************/

/**
********************************************************************************
* @file cpssHalFdbMgr.h
*
* @brief Internal header which defines API for helpers functions
* which are specific for XPS FDB Mac Table .
*
* @version   01
********************************************************************************
*/
#ifndef __cpssHalFdbMgrh
#define __cpssHalFdbMgrh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "cpssDxChBrgFdbManagerTypes.h"
#include "cpssHalFdb.h"
#include "cpssHalUtil.h"

/**
* @internal cpssHalFdbMgrConvertDxChTablePktCmdToGenric
* @endinternal
*
* @brief   Convert generic pkt command to device specific packet command
*
* @param[in] pktCmd                     - generic acket command
* @param[out] *cpssTableCmd          - (pointer to) DxCh MAC table command
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
GT_STATUS cpssHalFdbMgrConvertDxChTablePktCmdToGenric
(
    CPSS_PACKET_CMD_ENT             cpssTableCmd,
    xpPktCmd_e                      *pktCmd
);




/**
* @internal cpssHalFdbMgrConvertCpssToGenricL2MactEntry function
* @endinternal
*
* @brief   Convert device specific MAC entry into generic MAC entry
*
* @param[in] cpssMacEntryPtr          - (pointer to) cpss MAC entry parameters
*
* @param[in] fdbEntryType      - FdbEntry type such as IPv host
*       unicast entry, or layer 2 MAC Entry.
* @param[out] macEntryPtr              - (pointer to) MAC entry parameters
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/

GT_STATUS cpssHalFdbMgrConvertCpssToGenricL2MactEntry
(
    int                                                  devId,
    CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_STC       *cpssMacEntryPtr,
    XPS_FDB_ENTRY_TYPE_ENT                                fdbEntryType,
    void                                                  *macEntryPtr
);



/**
 * @internal cpssHalFdbMgrConvertCpssToGenericMacEntry function
 * @endinternal
 *
 * @brief   Convert device specific MAC entry into generic MAC entry
 *
 * @param[in] devId                    - device Id
 * @param[in] cpssMacEntryPtr          - (pointer to) cpss MAC entry parameters
 *
 * @param[in] fdbEntryType      - FdbEntry type such as IPv host
 *       unicast entry, or layer 2 MAC Entry.
 * @param[out] macEntryPtr              - (pointer to) MAC entry parameters
 *
 * @retval GT_OK                    - on success
 * @retval GT_BAD_PARAM             - on wrong parameters
 */

GT_STATUS cpssHalFdbMgrConvertCpssToGenericMacEntry
(
    int                                                  devId,
    CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_STC      *cpssMacEntryPtr,
    XPS_FDB_ENTRY_TYPE_ENT                               fdbEntryType,
    void                                                 *macEntryPtr
);



/*
* @internal cpssHalFdbMgrLearningScan function
* @endinternal
*
* @brief   The function gets the AUQ messages according to the input attributes.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
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
GT_STATUS cpssHalFdbMgrLearningScan
(
    GT_U32                                             fdbManagerId,
    CPSS_DXCH_BRG_FDB_MANAGER_LEARNING_SCAN_PARAMS_STC learningParams,
    CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_UPDATE_EVENT_STC   entriesLearningArray[],
    GT_U32                                             *entriesLearningNumPtr
);


/**
* @internal cpssHalFdbMgrConvertGenericToDxChMacEntryKey function
* @endinternal
*
* @brief   Convert generic MAC entry key into device specific MAC entry key
*
* @param[in] fdbEntryType      - FdbEntry type such as IPv host
*       unicast entry, or layer 2 MAC Entry.
* @param[in] macEntryKeyPtr           - (pointer to) MAC entry key parameters
* @param[in] macEntryKeyPtr           - (pointer to) MAC entry
*        Type
*
* @param[out] dxChMacEntryKeyPtr       - (pointer to) DxCh MAC entry key parameters
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
GT_STATUS cpssHalFdbMgrConvertGenericToDxChMacEntryKey
(
    int                                 devId,
    XPS_FDB_ENTRY_TYPE_ENT              fdbEntryType,
    void                                *macEntryKeyPtr,
    CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_STC *dxChMacEntryKeyPtr
);



/**
* @internal cpssHalFdbMgrConvertXpsFdbEntryTypeToDxChMacEntryType function
* @endinternal
*
* @brief   return the FDB Mac
* @param[in] fdbEntryType      - FdbEntry type such as IPv host
*       unicast entry, or layer 2 MAC Entry.
* @param[in] fdbEntryType           - XPS FDB Entry Type
*
* @param[out] dxChMacEntryType          - (pointer to)
*       dxChMacEntry Type
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
GT_STATUS cpssHalFdbMgrConvertXpsFdbEntryTypeToDxChMacEntryType
(
    GT_U8 fdbEntryType,
    GT_U8 *dxChMacEntryType
);



/**
* @internal cpssHalFdbMgrConvertPktCmdToDxChFdbTableDaSaCmd
* @endinternal
*
* @brief   Convert generic pkt command to device specific packet command
*
* @param[in] pktCmd                     - generic acket command
* @param[out] *cpssMacTableCmd          - (pointer to) DxCh MAC table command
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
GT_STATUS cpssHalFdbMgrConvertPktCmdToDxChFdbTableDaSaCmd
(
    xpPktCmd_e                        pktCmd,
    CPSS_PACKET_CMD_ENT               *cpssMacTableCmd
);



/**
* @internal cpssHalFdbMgrConvertL2MactEntryToDxChMacEntry function
* @endinternal
*
* @brief   Convert generic MAC entry into device specific MAC entry
*
* @param[in] devId                   - device Id
* @param[in] fdbEntryType      - FdbEntry type such as IPv host
*       unicast entry, or layer 2 MAC Entry.
* @param[in] macEntryPtr              - (pointer to) MAC entry parameters
*
* @param[out] dxChMacEntryPtr          - (pointer to) DxCh MAC entry parameters
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
GT_STATUS cpssHalFdbMgrConvertL2MactEntryToDxChMacEntry
(
    int                                      devId,
    XPS_FDB_ENTRY_TYPE_ENT                   fdbEntryType,
    void                                     *macEntryPtr,
    CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_STC      *dxChMacEntryPtr
);


/**
* @internal cpssHalFdbMgrConvertGenericToDxChMacEntry function
* @endinternal
*
* @brief   Convert generic MAC entry into device specific MAC entry
*
* @param[in] devId                   - device Id
* @param[in] fdbEntryType            - FdbEntry type such as IPv host
*       unicast entry, or layer 2 MAC Entry.
* @param[in] macEntryPtr              - (pointer to) MAC entry parameters
*
* @param[out] dxChMacEntryPtr          - (pointer to) DxCh MAC entry parameters
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
GT_STATUS cpssHalFdbMgrConvertGenericToDxChMacEntry
(
    int                                  devId,
    XPS_FDB_ENTRY_TYPE_ENT               fdbEntryType,
    void                                 *macEntryPtr,
    CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_STC  *dxChMacEntryPtr
);




/**
* @internal cpssHalFdbMgrMacEntryFind function
* @endinternal
*
* @brief   function to find an existing entry that match the key.
*
* @param devId                       - Device Id
* @param[in] fdbEntryType            - FdbEntry type such as IPv host
*       unicast entry, or layer 2 MAC Entry.
* @param[in] macEntryPtr           - (pointer to) mac entry key
*
* @retval GT_OK                    - on success
* @retval GT_NOT_FOUND             - entry not found, but bucket is not FULL
* @retval GT_FULL                  - entry not found, but bucket is FULL
* @retval GT_BAD_PARAM             - wrong devNum.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_FAIL                  - on error
*/
GT_STATUS cpssHalFdbMgrMacEntryFind
(
    int                     devId,
    GT_U32                  fdbManagerId,
    XPS_FDB_ENTRY_TYPE_ENT  fdbEntryType,
    void                    *macEntryKeyPtr
);



/**
* @internal cpssHalFdbMgrMacEntryWrite function
* @endinternal
*
* @brief   Write the new entry in Hardware MAC address table in
*          specified index
*
* @param[in] devId                    - device Id
* @param[in] fdbEntryType             - fdbEntryType
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum,saCommand,daCommand
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
GT_STATUS cpssHalFdbMgrMacEntryWrite
(
    int                           devId,
    GT_U32                        fdbManagerId,
    XPS_FDB_ENTRY_TYPE_ENT        fdbEntryType,
    void                          *macEntryPtr
);



/*
* @internal cpssHalFdbMgrMacEntryDelete function
* @endinternal
*
* @brief   This function delete entry from the CPSS FDB Manager's database and HW.
*          All types of entries can be deleted using this API.
*          NOTE: the FDB manager will remove the info from all the registered devices.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] fdbManagerId          - the FDB Manager id.
*                                    (APPLICABLE RANGES : 0..31)
* @param[in] entryPtr              - (pointer to) FDB entry format to be deleted.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_NOT_FOUND             - the entry was not found.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_INITIALIZED       - if the FDB Manager id is not initialized.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/

GT_STATUS cpssHalFdbMgrMacEntryDelete
(
    int                     devId,
    GT_U32                  fdbManagerId,
    XPS_FDB_ENTRY_TYPE_ENT  fdbEntryType,
    void                    *macEntryPtr
);


/**
* @internal cpssHalFdbMgrConvertL3HostEntryToDxChMacEntry function
* @endinternal
*
* @brief   Convert generic MAC entry into device specific MAC entry
*
* @param[in] devId                   - device Id
* @param[in] fdbEntryType      - FdbEntry type such as IPv host
*       unicast entry, or layer 2 MAC Entry.
* @param[in] macEntryPtr              - (pointer to) MAC entry parameters
*
* @param[out] dxChMacEntryPtr          - (pointer to) DxCh MAC entry parameters
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
GT_STATUS cpssHalFdbMgrConvertL3HostEntryToDxChMacEntry
(
    int                                      devId,
    XPS_FDB_ENTRY_TYPE_ENT                   fdbEntryType,
    void                                     *macEntryPtr,
    CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_STC      *dxChMacEntryPtr
);


/**
* @internal cpssHalFdbMgrConvertCpssToL3HostMacEntry function
* @endinternal
*
* @brief   Convert device specific MAC entry into generic MAC entry
*
* @param[in] devId                    - device Id
* @param[in] cpssMacEntryPtr          - (pointer to) cpss MAC entry parameters
*
* @param[in] fdbEntryType      - FdbEntry type such as IPv host
*       unicast entry, or layer 2 MAC Entry.
* @param[out] macEntryPtr              - (pointer to) MAC entry parameters
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
GT_STATUS cpssHalFdbMgrConvertCpssToL3HostMacEntry
(
    int                                  devId,
    CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_STC  *cpssMacEntryPtr,
    XPS_FDB_ENTRY_TYPE_ENT               fdbEntryType,
    void                                 *macEntryPtr
);



/**
* @internal cpssHalFdbMgrMacEntryGet function
* @endinternal
*
* @brief   function to find an existing entry and convert to xps structure.
*
* @param devId                       - Device Id
* @param[in] fdbEntryType            - FdbEntry type such as IPv host
*       unicast entry, or layer 2 MAC Entry.
* @param[in] macEntryPtr           - (pointer to) mac entry key
*
* @retval GT_OK                    - on success
* @retval GT_NOT_FOUND             - entry not found, but bucket is not FULL
* @retval GT_FULL                  - entry not found, but bucket is FULL
* @retval GT_BAD_PARAM             - wrong devNum.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_FAIL                  - on error
*/
GT_STATUS cpssHalFdbMgrMacEntryGet
(
    int                     devId,
    GT_U32                  fdbManagerId,
    XPS_FDB_ENTRY_TYPE_ENT  fdbEntryType,
    void                    *macEntryKeyPtr
);



/**
* @internal cpssHalFdbMgrMacEntryUpdate function
* @endinternal
*
* @brief   Updates the entry in Hardware MAC address table in
*          specified index
*
* @param[in] devId                    - device Id
* @param[in] fdbEntryType             - fdbEntryType
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum,saCommand,daCommand
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
GT_STATUS cpssHalFdbMgrMacEntryUpdate
(
    int                           devId,
    GT_U32                        fdbManagerId,
    XPS_FDB_ENTRY_TYPE_ENT        fdbEntryType,
    void                          *macEntryPtr
);

/**
* @internal cpssHalFdbMgrMacCountersGet function
* @endinternal
*
* @brief API for fetching FDB Manager Mac counters.
*
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] fdbManagerId          - the FDB Manager id.
*                                    (APPLICABLE RANGES : 0..31)
* @param[out] *macCount            - FDB Manager total used mac Counts.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
* @retval GT_BAD_PTR               - on NULL pointer.
*/
GT_STATUS cpssHalFdbMgrMacCountersGet
(
    int                           devId,
    GT_U32                        fdbManagerId,
    GT_U32                        *macCount
);

/**
* @internal cpssHalFdbMgrBrgSecurBreachCommandSet function
* @endinternal
*
* @brief API for setting Bridge secure breach command set.
*
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] eventType             - event type.
* @param[in] command               - packet command.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
* @retval GT_BAD_PTR               - on NULL pointer.
*/
GT_STATUS cpssHalFdbMgrBrgSecurBreachCommandSet
(
    int                                 devId,
    CPSS_BRG_SECUR_BREACH_EVENTS_ENT    eventType,
    xpsPktCmd_e                         command
);

/**
* @internal cpssHalFdbMgrBrgSecurBreachCommandGet function
* @endinternal
*
* @brief API for setting Bridge secure breach command set.
*
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] eventType             - event type.
* @param[in] command               - packet command.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
* @retval GT_BAD_PTR               - on NULL pointer.
*/
GT_STATUS cpssHalFdbMgrBrgSecurBreachCommandGet
(
    int                                 devId,
    CPSS_BRG_SECUR_BREACH_EVENTS_ENT    eventType,
    xpsPktCmd_e                         *command
);
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssHalFdbMgrh */


