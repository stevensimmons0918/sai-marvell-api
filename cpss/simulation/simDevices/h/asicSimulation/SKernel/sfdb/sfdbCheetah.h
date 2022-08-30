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
* @file sfdbCheetah.h
*
* @brief Cheetah FDB update, Triggered actions messages simulation
*
* @version   6
********************************************************************************
*/
#ifndef __sfdbcheetahh
#define __sfdbcheetahh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <common/Utils/PresteraHash/smacHashDx.h>

typedef enum{
    SFDB_CHT_BANK_COUNTER_ACTION_NONE_E = 0,
    SFDB_CHT_BANK_COUNTER_ACTION_INCREMENT_E = 1,
    SFDB_CHT_BANK_COUNTER_ACTION_DECREMENT_E = 2
}SFDB_CHT_BANK_COUNTER_ACTION_ENT;


typedef enum{
    SFDB_CHT_BANK_COUNTER_UPDATE_CLIENT_CPU_AU_MSG_E,
    SFDB_CHT_BANK_COUNTER_UPDATE_CLIENT_CPU_EXACT_BANK_E,
    SFDB_CHT_BANK_COUNTER_UPDATE_CLIENT_PP_AGING_DAEMON_E,
    SFDB_CHT_BANK_COUNTER_UPDATE_CLIENT_PP_AUTO_LEARN_E
}SFDB_CHT_BANK_COUNTER_UPDATE_CLIENT_ENT;

/*
 * typedef: structure SFDB_LION3_FDB_SPECIAL_MUXED_FIELDS_STC
 *
 * Description:
 *       Muxed fields from the FDB that depend on :
 *       in sip5 : vid1_assignment_mode , src_id_length_in_fdb
 *       in SIP6 : fdb_mac_entry_muxing_mode , fdb_ipv_4_6_mc_entry_muxing_mode
 *
 *      field with value : SMAIN_NOT_VALID_CNS , means 'not used'
*/
typedef struct{
    GT_U32              srcId;
    GT_U32              udb;
    GT_U32              origVid1;
    GT_U32              daAccessLevel;
    GT_U32              saAccessLevel;
}SFDB_LION3_FDB_SPECIAL_MUXED_FIELDS_STC;

/**
* @internal sfdbChtMsgProcess function
* @endinternal
*
* @brief   Process FDB update message.
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] fdbMsgPtr                - pointer to device object.
*/
void sfdbChtMsgProcess
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U8                 * fdbMsgPtr
);

/**
* @internal sfdbChtMacTableTriggerAction function
* @endinternal
*
* @brief   MAC Table Trigger Action
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] tblActPtr                - pointer to table action data
*/
void sfdbChtMacTableTriggerAction
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U8                 * tblActPtr
);


/**
* @internal sfdbChtMacTableAutomaticAging function
* @endinternal
*
* @brief   MAC Table automatic aging.
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] data_PTR                 - pointer to number of records to search
*/
void sfdbChtMacTableAutomaticAging
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U8                 * data_PTR
);

/**
* @internal sfdbChtAutoAging function
* @endinternal
*
* @brief   Age out MAC table entries.
*/
void sfdbChtAutoAging
(
    IN SKERNEL_DEVICE_OBJECT * deviceObjPtr
);

/**
* @internal sfdbCht2MacTableUploadAction function
* @endinternal
*
* @brief   MAC Table fdb upload engine.
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] tblActPtr                - ?????
*/
void sfdbCht2MacTableUploadAction
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U8                 * tblActPtr
);

/**
* @internal sfdbChtBankCounterAction function
* @endinternal
*
* @brief   SIP5 : the AU message from the CPU need to be checked if to increment/decrement
*         the counters of the relevant FDB bank
* @param[in] devObjPtr                - pointer to device object.
* @param[in] fdbIndex                 - FDB index that relate to the entry that may have bank counter action
* @param[in] counterAction            - the bank counter action.
* @param[in] client                   - triggering client
*                                       None.
*/
void sfdbChtBankCounterAction
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32                   fdbIndex,
    IN SFDB_CHT_BANK_COUNTER_ACTION_ENT     counterAction,
    IN SFDB_CHT_BANK_COUNTER_UPDATE_CLIENT_ENT  client
);

/**
* @internal sfdbChtHashResultsRegistersUpdate function
* @endinternal
*
* @brief   SIP5 : update the hash result registers that relate to last AU message
*         from the CPU to the PP.
* @param[in] devObjPtr                - pointer to device object.
* @param[in] calculatedHashArr[FDB_MAX_BANKS_CNS] - multi hash results :
*                                      array of calculated hash by the different functions
*                                      NULL means --> ignore the multi hash values
* @param[in] xorResult                -   xor result.
* @param[in] crcResult                -   CRC result.
*                                       None
*/
void sfdbChtHashResultsRegistersUpdate
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32       calculatedHashArr[FDB_MAX_BANKS_CNS],
    IN GT_U32       xorResult,
    IN GT_U32       crcResult
);

/**
* @internal sfdbLion3FdbSpecialMuxedFieldsGet function
* @endinternal
*
* @brief   Get Muxed fields from the FDB that depend on :
*         1. vid1_assignment_mode
*         2. src_id_length_in_fdb
* @param[in] devObjPtr                - pointer to device object.
*                                      fdbMsgPtr   - pointer to FDB message.
* @param[in] fdbIndex                 - fdb index
*
* @param[out] specialFieldsPtr         - (pointer to) the special fields value from the entry
*/
GT_VOID sfdbLion3FdbSpecialMuxedFieldsGet
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32                * fdbEntryPtr,
    IN GT_U32                fdbIndex,
    OUT SFDB_LION3_FDB_SPECIAL_MUXED_FIELDS_STC *specialFieldsPtr
);

/**
* @internal sfdbLion3FdbSpecialMuxedFieldsSet function
* @endinternal
*
* @brief   Set Muxed fields from the FDB that depend on :
*         1. vid1_assignment_mode
*         2. src_id_length_in_fdb
* @param[in] devObjPtr                - pointer to device object.
*                                      fdbMsgPtr   - pointer to FDB message.
* @param[in] fdbIndex                 - fdb index
* @param[in] macEntryType             - entry type
* @param[in] specialFieldsPtr         - (pointer to) the special fields value from the entry
*/
GT_VOID sfdbLion3FdbSpecialMuxedFieldsSet
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32                * fdbEntryPtr,
    IN GT_U32                fdbIndex,
    IN SNET_CHEETAH_FDB_ENTRY_ENT                macEntryType,
    IN SFDB_LION3_FDB_SPECIAL_MUXED_FIELDS_STC *specialFieldsPtr
);


/**
* @internal sfdbLion3FdbAuMsgSpecialMuxedFieldsSet function
* @endinternal
*
* @brief   Set Muxed fields from the FDB Au Msg that depend on :
*         1. vid1_assignment_mode
*         2. src_id_length_in_fdb
* @param[in] devObjPtr                - pointer to device object.
* @param[in] auMsgEntryPtr            - pointer to FDB AU MSG message.
* @param[in] macEntryType             - entry type
* @param[in] specialFieldsPtr         - (pointer to) the special fields value from the entry
*/
GT_VOID sfdbLion3FdbAuMsgSpecialMuxedFieldsSet
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32                * auMsgEntryPtr,
    IN SNET_CHEETAH_FDB_ENTRY_ENT                macEntryType,
    IN SFDB_LION3_FDB_SPECIAL_MUXED_FIELDS_STC *specialFieldsPtr
);

/**
* @internal sip6AuMsgSpecialMuxedFieldsSet function
* @endinternal
*
* @brief   Sip6 : Get Muxed fields from the FDB that depend on :
*         fdb_mac_entry_muxing_mode or fdb_ipv_4_6_mc_entry_muxing_mode
* @param[in] devObjPtr                - pointer to device object.
*                                      fdbMsgPtr   - pointer to FDB message.
*                                      fdbIndex     - fdb index
*
* @param[out] specialFieldsPtr         - (pointer to) the special fields value from the entry
*/
GT_VOID sip6AuMsgSpecialMuxedFieldsSet
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    OUT GT_U32               * macUpdMsgPtr,
    IN SNET_CHEETAH_FDB_ENTRY_ENT                macEntryType,
    IN SFDB_LION3_FDB_SPECIAL_MUXED_FIELDS_STC *specialFieldsPtr
);
/**
* @internal sip6FdbSpecialMuxedFieldsSet function
* @endinternal
*
* @brief   Sip6 : Set Muxed fields to the FDB that depend on :
*         fdb_mac_entry_muxing_mode or fdb_ipv_4_6_mc_entry_muxing_mode
* @param[in] devObjPtr                - pointer to device object.
* @param[in] specialFieldsPtr         - (pointer to) the special fields value from the entry
* @param[in] fdbIndex                 - fdb index
* @param[in] fdb_entry_type           - FDB entry type (supported only SNET_CHEETAH_FDB_ENTRY_MAC_E)
* @param[in] fdbEntryPtr              - pointer to FDB entry.
*
* @param[out] fdbEntryPtr              - pointer to FDB entry.
*
* @note function implemented only for 'Auto learn' (SNET_CHEETAH_FDB_ENTRY_MAC_E)
*       because not needed more than that.
*
*/
GT_VOID sip6FdbSpecialMuxedFieldsSet
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32                * fdbEntryPtr,
    IN GT_U32                fdbIndex,
    IN SNET_CHEETAH_FDB_ENTRY_ENT                fdb_entry_type,
    OUT SFDB_LION3_FDB_SPECIAL_MUXED_FIELDS_STC *specialFieldsPtr
);



#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif  /* __sfdbcheetahh */



