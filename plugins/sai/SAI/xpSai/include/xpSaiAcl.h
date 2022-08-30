// xpSaiAcl.h

/*******************************************************************************
* copyright (c) 2021 marvell. all rights reserved. the following file is       *
* subject to the limited use license agreement by and between marvell and you, *
* your employer or other entity on behalf of whom you act. in the absence of   *
* such license agreement the following file is subject to marvell’s standard   *
* limited use license agreement.                                               *
********************************************************************************/

#ifndef _xpSaiAcl_h_
#define _xpSaiAcl_h_

#include "xpSai.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \file xpSaiAcl.h
 * \brief This file contains API prototypes, defines and type definitions
 *        for the ACL
 *
 */

/**
 * \brief The ACL table size is dependent on the key size selected when the
 *        table is created. Each DB is 192K, so for a key size of 64b it
 *        will give us a table of 3K entries.
 *
 */
#define XP_SAI_ACL_TCAM_DB_SIZE (192*1024)
#define XP_SAI_ACL_MAX_BIND_POINTS         6
#define SAI_SWITCH_BIND_POINT_PCL_ID (0)
#define XP_SAI_ACL_TABLE_SIZE_DEFAULT (0)
/**
 * \brief If we are allocated all the db's to PACL then numDb's should be set
 *        to 0x6. This is a limitation since SAI doesn't have the concept of
 *        num db's as part of the input to the API.
 *        Today we only create PACL table, SAI doesn't create table for any
 *        other lookup type in the 0.9.2 version.
 *
 */
#define XP_SAI_ACL_NUM_DB (0x1) // Number of databases to create. Value was verified manually 

#define XP_SAI_EACL_NUM_DB   XP_SAI_ACL_NUM_DB
#define XP_SAI_ENTRY_NUM_MAX             (512 * XP_SAI_ACL_NUM_DB)
#define XP_SAI_ENTRY_PRI_NUM_MAX_PARAM   "aclEntryMaxPrio"
#define XP_SAI_DEFAULT_ENTRY_PRI_NUM_MAX XP_SAI_ENTRY_NUM_MAX
#define XP_SAI_TABLE_PRI_NUM_MAX  128
#define XP_SAI_ENTRY_MAX     (1024*6)
#define XP_SAI_ACL_BP_MAX    1
#define XP_SAI_MAX_BIND_TYPE 5
#define XP_SAI_RANGE_SUB_INTERVALS_MAX 1024
#define XP_SAI_DB_SIZE       199680 // value in bits
#define XP_SAI_IACL0_NUM_DB  XP_SAI_ACL_NUM_DB
#define XP_SAI_IACL1_NUM_DB  XP_SAI_ACL_NUM_DB
#define XP_SAI_IACL2_NUM_DB  XP_SAI_ACL_NUM_DB
#define XP_SAI_IACL3_NUM_DB  XP_SAI_ACL_NUM_DB
#define XP_SAI_IACL4_NUM_DB  XP_SAI_ACL_NUM_DB
#define XP_SAI_IACL5_NUM_DB  XP_SAI_ACL_NUM_DB
#define XP_SAI_ENTRY_PRI_NUM_MAX (0xFFFFFFFF)
//#define XP_SAI_TABLE_PRI_NUM_MAX (0xFF)
#define XP_SAI_ENTRY_PRI_NUM_MIN (0)
#define XP_SAI_TABLE_PRI_NUM_MIN (0)
#define XP_SAI_CUST_PRIORITY_MAX      (0xFFFFFFFF)
#define XP_SAI_PRIORITY_SHIFT_NUM 24
#define XP_SAI_INVALID_RULE_ID        (0xFFFFFFFF)
#define XPSAI_ACL_PBR_MAX_ECMP_SIZE 16
#define XP_SAI_ACL_PORT_IN_TBL_CPY1_MAX 36
#define XP_SAI_ACL_TABLE_COPY_MAX 2
#define XP_SAI_ACL_ASY_TABLE_START(iaclType)    ((iaclType*4) + XP_IACL0_0)

#define XP_SAI_ACL_PORT_NUMBER_MAX 256

#define XP_SAI_ACL_TABLE_MATCH_NUMBER_MAX ((SAI_ACL_TABLE_ATTR_FIELD_END - SAI_ACL_TABLE_ATTR_FIELD_START) + 1)

#define XP_SAI_ACL_MATCH_NUMBER_MAX ((SAI_ACL_ENTRY_ATTR_FIELD_END - SAI_ACL_ENTRY_ATTR_FIELD_START) + 1)

#define XP_SAI_ACL_ACTION_NUMBER_MAX ((SAI_ACL_ENTRY_ATTR_ACTION_END - SAI_ACL_ENTRY_ATTR_ACTION_START) + 1)

#define XP_SAI_GET_MATCH_ID(KEY_VALUE) (KEY_VALUE - SAI_ACL_ENTRY_ATTR_FIELD_START)
#define XP_SAI_GET_ACTION_ID(ACTION_VALUE) (ACTION_VALUE - SAI_ACL_ENTRY_ATTR_ACTION_START)
typedef enum _xpSaiKeyType
{

    XP_SAI_KEY_NON_IP,
    XP_SAI_KEY_IPV4,
    XP_SAI_KEY_IPV6,
    XP_SAI_KEY_IPV4_AND_IPV6,

} xpSaiKeyType;

/**
 * \brief SAI ACL table attributes
 *
 */
typedef struct _xpSaiAclTableAttributesT
{
    sai_acl_stage_t
    stage;                     ///< SAI_ACL_TABLE_ATTR_STAGE
    bool                       stage_valid;
    xpSaiKeyType               keyType;
    sai_acl_bind_point_type_t
    aclBindPoint;               //<currently supporting one Bind point only ,
    //will update code for list.
    bool
    isBindPointSet;             //<Bind point is mandatory attribute, so if that attribute is
    //pass we will set this as TRUE.
    sai_uint32_t
    bp_list[XP_SAI_ACL_MAX_BIND_POINTS]; // Bind point type list
    sai_uint32_t               bindPointCount;
    sai_uint32_t               priority;                   // Table Priority
    sai_uint32_t
    tableSize;                 ///< SAI_ACL_TABLE_ATTR_SIZE
    bool                       match[XP_SAI_ACL_TABLE_MATCH_NUMBER_MAX];
    bool                       isMirror;
} xpSaiAclTableAttributesT;

/**
 * \brief SAI ACL table group attributes
 *
 */
typedef struct _xpSaiAclTableGrpAttributesT
{
    sai_acl_stage_t            stage;
    sai_uint32_t               bp_list[XP_SAI_ACL_BP_MAX];
    sai_acl_table_group_type_t type;
} xpSaiAclTableGrpAttributesT;

/**
 * \brief SAI ACL table group member attributes
 *
 */
typedef struct _xpSaiAclTableGrpMembAttributesT
{
    sai_object_id_t  groupId;
    sai_object_id_t  tableId;
    sai_uint32_t     priority;
} xpSaiAclTableGrpMembAttributesT;

/**
 * \brief SAI ACL counter attributes
 *
 */
typedef struct _xpSaiAclCounterAttributesT
{
    sai_uint32_t    tableId;               ///< SAI_ACL_COUNTER_ATTR_TABLE_ID
    sai_object_id_t tableOid;              ///< SAI_ACL_COUNTER_ATTR_TABLE_ID
    bool            table_valid;
    bool
    enablePacketCount;     ///< SAI_ACL_COUNTER_ATTR_ENABLE_PACKET_COUNT
    bool
    enableByteCount;       ///< SAI_ACL_COUNTER_ATTR_ENABLE_BYTE_COUNT
    sai_uint64_t    packets;               ///< SAI_ACL_COUNTER_ATTR_PACKETS
    sai_uint64_t    bytes;                 ///< SAI_ACL_COUNTER_ATTR_BYTES
} xpSaiAclCounterAttributesT;

/**
 * \brief SAI ACL rule info
 *
 */
typedef struct _xpSaiAclEntryRuleT
{
    sai_uint32_t id;
    sai_uint32_t evif;
    bool         valid;
} xpSaiAclEntryRuleT;

typedef struct _xpSaiAclEntryFieldData
{
    bool isSetByUser;
    sai_acl_field_data_t entryValue;
} xpSaiAclEntryFieldData;

typedef struct _xpSaiAclEntryActionData
{
    bool isSetByUser;
    sai_acl_action_data_t actionValue;
} xpSaiAclEntryActionData;


/**
 * \brief SAI ACL entry attributes
 *
 */
typedef struct _xpSaiAclEntryAttributesT
{
    sai_object_id_t          tableOid;          ///< SAI_ACL_ENTRY_ATTR_TABLE_ID
    sai_uint32_t             tableId;           ///< SAI_ACL_ENTRY_ATTR_TABLE_ID
    sai_object_list_t        aclRangeObjList;
    sai_uint32_t             priority;          ///< SAI_ACL_ENTRY_ATTR_PRIORITY
    bool                     adminState;        ///< SAI_ACL_ENTRY_ATTR_ADMIN_STATE
    sai_acl_ip_type_t
    entryType;         ///< SAI_ACL_ENTRY_ATTR_FIELD_IP_PROTOCOL
    bool                     isEntryTypeValid;
    sai_uint32_t
    isSwInstDone;      ///< use to check whether entry is configured for switch.
    sai_uint32_t             in_ports_count;
    sai_uint32_t             in_ports[XP_SAI_ACL_PORT_NUMBER_MAX];
    sai_uint32_t             out_ports_count;
    sai_uint32_t             out_ports[XP_SAI_ACL_PORT_NUMBER_MAX];
    xpSaiAclEntryFieldData   match[XP_SAI_ACL_MATCH_NUMBER_MAX];
    xpSaiAclEntryActionData  action[XP_SAI_ACL_ACTION_NUMBER_MAX];
    xpSaiAclEntryRuleT       ruleMap[XP_SAI_ACL_PORT_NUMBER_MAX];
    sai_uint32_t             rule_count;
    bool                     isAsyEntry;
    bool                     isRangeBase;
    sai_uint32_t             entryIdCount;
    sai_uint32_t             entryIds[XP_SAI_RANGE_SUB_INTERVALS_MAX];
    sai_uint8_t              entryAsyTable;
    bool                     isPolicerEnable;
    sai_object_id_t          policerId;
    bool                     isMirrorEnable;
    sai_object_id_t          mirrorId;
    bool                     isSampleEnable;
    sai_object_id_t          sampleId;
} xpSaiAclEntryAttributesT;

/**
 * \brief SAI ACL Range entry attributes
 *
 */

typedef struct _xpSaiAclRangeAttributesT
{
    sai_acl_range_type_t     rangeType;
    sai_uint32_t             xpsComparatorIndex;
    sai_uint32_t             from;
    sai_uint32_t             to;
    bool                     isRangeSet;
} xpSaiAclRangeAttributesT;

/**
 * brief State structure maintained by ACL
 *
 * This state contains the SAI Policer to index information
 */
typedef struct xpSaiAclPolicerInfo_t
{
    sai_object_id_t  policerId;
    xpsPolicerType_e    client;
    xpAcm_t          acmIndex;
    sai_uint16_t     refCount;
} xpSaiAclPolicerInfo_t;


/**
 * \brief API that initializes the ACL
 *
 * API will register for the state databases that are
 * needed by the ACL
 *
 * \return XP_STATUS
 */
XP_STATUS xpSaiAclApiInit(sai_uint64_t flag,
                          const sai_service_method_table_t* adapHostServiceMethodTable);
/**
 * \brief API to De-Init the ACL
 *
 * This API will Deregister all state databases for ACL
 *
 * \return XP_STATUS
 */
XP_STATUS xpSaiAclApiDeinit();

/**
 * \brief API to register the ACL
 *
 * This API will register ACL
 *
 * \return XP_STATUS
 */
sai_status_t xpSaiAclInit(xpsDevice_t xpsDevId);

/**
 * \brief API to deregister the ACL
 *
 * This API will deregister ACL
 *
 * \return XP_STATUS
 */

sai_status_t xpSaiAclDeInit(xpsDevice_t xpsDevId);

/**
 * \brief API to get name of the table attribute
 *
 * \param [out] saiAttr Attribute identifier
 *
 * \return Attribute name
 */
const char* xpSaiTableAttrNameGet(sai_acl_table_attr_t saiAttr);
sai_status_t xpSaiBulkGetAclTableAttributes(sai_object_id_t id,
                                            sai_uint32_t *attr_count, sai_attribute_t *attr_list);
sai_status_t xpSaiBulkGetAclEntryAttributes(sai_object_id_t id,
                                            sai_uint32_t *attr_count, sai_attribute_t *attr_list);
sai_status_t xpSaiBulkGetAclCounterAttributes(sai_object_id_t id,
                                              sai_uint32_t *attr_count, sai_attribute_t *attr_list);

sai_status_t xpSaiMaxCountAclTableAttribute(sai_uint32_t *count);
sai_status_t xpSaiMaxCountAclCounterAttribute(sai_uint32_t *count);
sai_status_t xpSaiMaxCountAclEntryAttribute(sai_uint32_t *count);
sai_status_t xpSaiMaxCountAclRangeAttribute(sai_uint32_t *count);

sai_status_t xpSaiCountAclTableObjects(sai_uint32_t *count);
sai_status_t xpSaiCountAclCounterObjects(sai_uint32_t *count);
sai_status_t xpSaiCountAclEntryObjects(sai_uint32_t *count);

sai_status_t xpSaiGetAclTableObjectList(sai_uint32_t *object_count,
                                        sai_object_key_t *object_list);
sai_status_t xpSaiGetAclCounterObjectList(sai_uint32_t *object_count,
                                          sai_object_key_t *object_list);
sai_status_t xpSaiGetAclEntryObjectList(sai_uint32_t *object_count,
                                        sai_object_key_t *object_list);

/**
 * \brief API to Get tableId list for specified table group
 *
 * \param [in]      groupId     -- (input) table group ID
 * \param [in/out] *count       -- (input) number of items in a list
 * \param [out]    *tableIdList -- (output) array of tableId
 *
 * \return sai_status_t
 */
sai_status_t xpSaiGetACLTableGrpTableIdList(sai_object_id_t groupId,
                                            sai_uint32_t *count, sai_uint32_t *tableIdList);

/** \brief Validate whether entry attribute supports list or not
 *
 * \param [in] sai_acl_entry_attr_t attr
 *
 * \return bool if true indicates the attribute will have value with list
 */
bool xpSaiValidateEntryListAttributes(sai_acl_entry_attr_t attr);

/** \brief Validate whether table attribute supports object list or not
 *
 * \param [in] sai_acl_table_attr_t attr
 *
 * \return bool if true indicates the attribute will have value with list
 */
bool xpSaiValidateTableListAttributes(sai_acl_table_attr_t attr);

bool xpSaiAclEntryActionAttributeEnabledCheck(xpSaiAclEntryAttributesT*
                                              pAttributes, sai_acl_entry_attr_t attr);
bool xpSaiAclEntryMatchAttributeIsSetByUser(xpSaiAclEntryAttributesT*
                                            pAttributes, sai_acl_entry_attr_t attr);
bool xpSaiAclEntryActionAttributeIsSetByUser(xpSaiAclEntryAttributesT*
                                             pAttributes, sai_acl_entry_attr_t attr);
sai_status_t xpSaiAclEntryActionAttributeGet(xpSaiAclEntryAttributesT*
                                             pAttributes, sai_acl_entry_attr_t attr, sai_acl_action_data_t* pAction);

sai_status_t  xpSaiAclEntryMatchAttributeGet(xpSaiAclEntryAttributesT*
                                             pAttributes, sai_acl_entry_attr_t attr,
                                             sai_acl_field_data_t* match);
bool xpSaiAclEntryMatchAttributeEnabledCheck(xpSaiAclEntryAttributesT*
                                             pAttributes, sai_acl_entry_attr_t attr);

sai_status_t xpSaiAclEntryActionAttributeSet(xpSaiAclEntryAttributesT*
                                             pAttributes, sai_acl_entry_attr_t attr, sai_attribute_value_t value);
sai_status_t xpSaiAclEntryMatchAttributeSet(xpSaiAclEntryAttributesT*
                                            pAttributes, sai_acl_entry_attr_t attr, sai_attribute_value_t value);
sai_status_t xpSaiConvertAclCounterOid(sai_object_id_t acl_counter_id,
                                       uint32_t* pCounterId);
void xpSaiSetDefaultAclEntryAttributeVals(xpSaiAclEntryAttributesT* attributes);
sai_status_t xpSaiAclEntryAttributesUpdate(const sai_uint32_t attr_count,
                                           const sai_attribute_t* attr_list, xpSaiAclEntryAttributesT* pAttributes);
sai_status_t xpSaiAclEntryCounterApply(sai_uint32_t devId, sai_uint32_t entryId,
                                       xpSaiAclEntryAttributesT* pAttributes);
sai_status_t xpSaiConvertAclEntryOid(sai_object_id_t acl_entry_id,
                                     sai_uint32_t* pEntryId);
sai_status_t xpSaiAclEntryAttributeUpdate(const sai_attribute_t* pAttr,
                                          xpSaiAclEntryAttributesT* pAttributes);
const char* xpSaiEntryAttrNameGet(sai_acl_entry_attr_t saiAttr);
sai_status_t xpSaiAclEntryMatchAttributeConvert(sai_acl_field_data_t match,
                                                sai_acl_entry_attr_t attr, xpsAclkeyFieldList_t *pFieldData,
                                                xpsAclKeyFlds_t  fld, sai_uint32_t *keyIndex,
                                                xpSaiAclEntryAttributesT* pAttributes) ;
sai_status_t xpSaiAclEntryUnMatchAttributeConvert(sai_acl_entry_attr_t attr,
                                                  xpsAclkeyFieldList_t *pFieldData, xpsAclKeyFlds_t  fld, sai_uint32_t *keyIndex);
sai_status_t xpSaiConvertAclTableOid(sai_object_id_t acl_table_id,
                                     sai_uint32_t* pTableId);
sai_status_t xpSaiUpdateAclTableAttributeVals(const sai_uint32_t attr_count,
                                              const sai_attribute_t* attr_list, xpSaiAclTableAttributesT* attributes);
void xpSaiSetDefaultAclTableAttributeVals(xpSaiAclTableAttributesT* attributes);
void xpSaiKeySizeByteConvert(sai_uint32_t bitKeySize,
                             sai_uint32_t *pByteKeySize);
bool xpSaiAclTableAttributeEnabledCheck(xpSaiAclTableAttributesT* pAttributes,
                                        sai_acl_table_attr_t attr);
sai_status_t xpSaiGetAclPolicerInfo(xpsPolicerType_e client,
                                    sai_object_id_t policer_id, xpAcm_t *acmIndex);
sai_status_t xpSaiSetAclToPolicerMap(sai_object_id_t policer_id,
                                     xpsPolicerType_e client);
sai_status_t xpSaiRemoveAclEntry(sai_object_id_t acl_entry_id);
sai_status_t xpSaiDeleteAclEntry(xpsDevice_t devId, sai_uint32_t tableId,
                                 xpSaiAclEntryRuleT *entryRule);
sai_status_t xpSaiAclMapperAddEntryInCounter(sai_uint32_t CounterId,
                                             sai_uint32_t entryId);
sai_status_t xpSaiAclMapperRemoveEntryInCounter(sai_uint32_t CounterId,
                                                sai_uint32_t entryId);
sai_status_t xpSaiCreateAclRange(sai_object_id_t *acl_entry_id,
                                 sai_object_id_t switchId, sai_uint32_t attr_count, const sai_attribute_t *attr);
sai_status_t xpSaiSetAclRangeAttribute(sai_object_id_t acl_range_id,
                                       const sai_attribute_t *attr);
sai_status_t xpSaiGetAclRangeAttributes(sai_object_id_t acl_range_id,
                                        sai_uint32_t attr_count, sai_attribute_t *attr_list);
sai_status_t xpSaiAclRangeAttributesUpdate(const sai_uint32_t attr_count,
                                           const sai_attribute_t* attr_list, xpSaiAclRangeAttributesT* pAttributes);
sai_status_t xpSaiAclRangeAttributeUpdate(const sai_attribute_t* pAttr,
                                          xpSaiAclRangeAttributesT* pAttributes);
sai_status_t xpSaiAclAddDuplicateAclEntry(sai_uint32_t tableId,
                                          sai_uint32_t entryId);
sai_status_t xpSaiAclRemoveDuplicateAclEntry(sai_uint32_t tableId,
                                             sai_uint32_t entryId);
#ifdef __cplusplus
}
#endif

#endif //_xpSaiAcl_h_
