// xpSaiAclMapper.h

/*******************************************************************************
* copyright (c) 2021 marvell. all rights reserved. the following file is       *
* subject to the limited use license agreement by and between marvell and you, *
* your employer or other entity on behalf of whom you act. in the absence of   *
* such license agreement the following file is subject to marvell’s standard   *
* limited use license agreement.                                               *
********************************************************************************/

#ifndef _xpSaiAclMapper_h_
#define _xpSaiAclMapper_h_

#include "xpSai.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * What is mapper and why do we need it?
 * Due to different ACL approaches in SAI 0.9.1 and XPS we do have to maintain
 * SAI tableId mapping to internal XPS acl_stage + IACL type.
 * Also, we do have to maintain SAI entryId mapping to XPS rule mapping.
 *
 * In some cases single SAI entryId may have multiple corresponding XPS ruleId's.
 * In particular, if SAI uses port bit map, XPS has to install as many rules,
 * as there are ports. In this case, the difference between rules will be in
 * aclId.
 *
 * Some variables explanation:
 *     tableId - ID application uses to access table
 *     entryId - Entry value that SAI returns to application level
 *     ruleId  - Value XPS uses as TCAM ID
 *     aclId   - Random value to assign port rule to port
 */
/*
 * Ipv4 or Ipv6 can't  coexist at a time. Use Max of the ipv4 or Ipv6.(IPv4 is having more fields).
 */
#define XP_SAI_ACL_TBL_MAX_IDS                 256
#define XP_SAI_ACL_TBL_RANGE_START             0
#define XP_SAI_PACL_TBL_MAX_IDS                128  ///< supportig only 7 bits for ACL ID, MSB 1 bit is reserved for Switch Acl ID.
#define XP_SAI_PACL_TBL_RANGE_START            1
#define XP_SAI_BACL_TBL_MAX_IDS                256
#define XP_SAI_BACL_TBL_RANGE_START            0
#define XP_SAI_RACL_TBL_MAX_IDS                256
#define XP_SAI_RACL_TBL_RANGE_START            0
#define XP_SAI_ACL_COUNTER_MODE_C_ENTRY_NUMB   (4 * 1024)  ///< Sai Acl Counter Mode C Entry Num (Please see: ACM-Design.doc )
#define XP_SAI_ACL_ENTRY_MAX_IDS               (12 * 1024)
#define XP_SAI_ACL_ENTRY_RANGE_START           0
#define XP_SAI_ACL_RANGE_MAX_IDS               256
#define XP_SAI_ACL_RANGE_RANGE_START           1
//#define XP_SAI_ACL_PER_TBL_COUNTER_MAX_IDS     (XP_SAI_ACL_COUNTER_MODE_C_ENTRY_NUMB * 3)
#define XP_SAI_ACL_PER_TBL_COUNTER_MAX_IDS     (1024 * 6)
#define XP_SAI_ACL_PER_TBL_COUNTER_RANGE_START 0
//ToDo as per SONiC per port an acl table group is created
//Currently making it to 100. Need to come up with a proper value.
#define XP_SAI_ACL_TBL_GRP_MAX_IDS             256
#define XP_SAI_ACL_TBL_GRP_RANGE_START         10 /*0-9 reserve it for ctrl acls*/
#define XP_SAI_ACL_TBL_GRP_MEMBER_MAX_IDS      (XP_SAI_ACL_TBL_GRP_MAX_IDS * XP_SAI_ACL_TBL_MAX_IDS)
#define XP_SAI_ACL_TBL_GRP_MEMBER_RANGE_START  0
#define XP_SAI_ACL_TBL_ID_SHIFT                24

//#define XP_SAI_ACL_PER_TBL_COUNTER_ALLOC_ID(tableId) (uint16_t(XP_SAI_ALLOC_ACL_COUNTER_START + (tableId)))

#define SAI_IACL_IPV4_FIELD_LIST_SIZE XPS_PCL_MAX_NUM_FLDS
#define SAI_EACL_IPV4_FIELD_LIST_SIZE XPS_PCL_MAX_NUM_FLDS
#define SAI_IACL_IPV6_FIELD_LIST_SIZE XPS_PCL_MAX_NUM_FLDS
#define SAI_EACL_IPV6_FIELD_LIST_SIZE XPS_PCL_MAX_NUM_FLDS
#define EACL_TABLE_NUM (3)
#define XP_SAI_ACL_DEFAULT_ENTRYID_VALUE (0xffFFffFF) // Default EntryId value, to set when Counter is created, but not assigned yet 
#define XP_SAI_ACL_DEFAULT_COUNTERID_VALUE (0xffFFffFF) // Default CounterId value, to set when Counter is created, but not assigned yet 

/**
 * Deliver EID index from Entry ID
 */
#define XP_SAI_ACL_EID_FROM_ENTRY_ID_GET(entry_id) (entry_id && 0x00FFffFF)

/* Maximum table attribute count */
#define XP_SAI_ACL_MAX_ATTR_COUNT ((SAI_ACL_ENTRY_ATTR_FIELD_END -        \
                                    SAI_ACL_ENTRY_ATTR_FIELD_START + 3) + \
                                   (SAI_ACL_ENTRY_ATTR_ACTION_END -       \
                                    SAI_ACL_ENTRY_ATTR_ACTION_START))

#define SAI_ACL_TBL_GROUP_SIZE          (128)
#define SAI_ACL_BIND_POINT_LIST_SIZE    (256)

/**
 * \brief This table provides SAI tableId mapping to
 *        XPS layer stage + iaclType
 */
typedef struct _xpSaiAclTableIdMappingT
{
    sai_uint32_t              tableId;
    bool                      isSwitchAclEn;
    sai_acl_stage_t           stage;
    xpSaiKeyType              keyType;
    sai_uint32_t              tableSize;
    sai_uint32_t              numOfEntryInst0;//Per SE Entry
    sai_uint32_t              numOfEntryInst1;//Per SE Entry
    sai_uint32_t              priority;
    sai_uint32_t              numEntries;
    sai_uint32_t
    bp_list[XP_SAI_ACL_MAX_BIND_POINTS]; // Bind point type list
    sai_uint32_t              bindPointCount;
    bool                      match[XP_SAI_ACL_TABLE_MATCH_NUMBER_MAX];
    bool                      isMirror;
    sai_uint32_t              entryId[XP_SAI_ENTRY_MAX];
} xpSaiAclTableIdMappingT;

/**
 * \brief This table provides SAI groupId mapping to
 *        ACL Table Group object state
 */
typedef struct _xpSaiAclTableGrpIdT
{
    sai_uint32_t               groupId;
    sai_acl_stage_t            stage;
    sai_int32_t                bp_list[XP_SAI_ACL_BP_MAX]; // Bind point type list
    sai_acl_table_group_type_t type;
    sai_uint32_t               bindPointCount;
    sai_object_id_t
    bindPointList[SAI_ACL_BIND_POINT_LIST_SIZE]; // Bind point list, can be switch, port, lag and vlan
    sai_uint32_t               memberCount;
    sai_object_id_t
    memberList[SAI_ACL_TBL_GROUP_SIZE];
} xpSaiAclTableGrpIdMappingT;

/**
 * \brief This table provides SAI memberId mapping to
 *        ACL Table Group Member object state
 */
typedef struct _xpSaiAclTableGrpMembIdT
{
    sai_uint32_t  memberId;
    sai_uint32_t  groupId;
    sai_uint32_t  tableId;
    sai_uint32_t  priority;
} xpSaiAclTableGrpMembIdMappingT;

/**
 * \brief This table provides SAI counterId mapping to
 *        SAI layer entryId
 */
typedef struct _xpSaiAclCounterIdMappingT
{
    sai_uint32_t counterId;
    sai_uint32_t numEntries;
    xpSaiAclCounterAttributesT attr;
    sai_uint32_t               entryId [XP_SAI_ENTRY_MAX];
} xpSaiAclCounterIdMappingT;

/**
 * \brief This table provides SAI entryId mapping to counterId
 */
typedef struct _xpSaiAclEntryIdMappingT
{
    sai_uint32_t entryId;
    xpSaiAclEntryAttributesT attrs;
} xpSaiAclEntryIdMappingT;


typedef struct _xpSaiAclRangeIdMappingT
{
    sai_uint32_t entryId;
    xpSaiAclRangeAttributesT attrs;
} xpSaiAclRangeIdMappingT;

/**
 * @brief This table provides SAI entryId mapping to counterId
 */
typedef struct _xpSaiAclRsnCodeToQueueMapInfo_t
{
    sai_uint32_t rsnCode;
    sai_uint32_t queueId;
    sai_uint16_t refCount;
} xpSaiAclRsnCodeToQueueMapInfo_t;

/**
 * \brief This Acl-table having table static values.
 */
typedef struct _xpSaiAclStaticTableMappingT
{
    sai_uint32_t              tableId;
    sai_uint32_t              keySize;
    sai_uint32_t              numOfDb;
    sai_uint32_t              maxNumEntries;
    sai_uint32_t              numOfEntryInstOrResv;
} xpSaiAclStaticTableMappingT;

/**
 * Function: xpSaiAclEntryPriNumMaxGet
 * \brief Get SAI ACL entry MAX priority
 *
 * \return sai_uint32_t
 */
sai_uint32_t xpSaiAclEntryPriNumMaxGet();

/**
 * Function: xpSaiAclMapperInit
 * \brief Initialize SAI ACL mapper
 *
 * \param [in] devId  -- (input) Device ID
 * \return sai_status_t
 */
sai_status_t xpSaiAclMapperInit(xpsDevice_t devId);

/**
 * Function: xpSaiAclMapperDeinit
 * \brief Deinitialize SAI ACL mapper
 * \param [in] devId  -- (input) Device ID
 * \return sai_status_t
 */
sai_status_t xpSaiAclMapperDeinit(xpsDevice_t devId);

/**
 * Function: xpSaiAclMapperTableIdGet
 * \brief Get tableId from table type and ACL stage
 * \param [in]sai_acl_stage_t stage  -- (input) SAI_ACL_STAGE_INGRESS or SAI_ACL_STAGE_EGRESS
 * \param [out] tableId      -- (output) returns table ID for application level
 * \param [in] switchEn  -- (input) True in case when Table need to add as switch Acl support.
 *
 * \return sai_status_t
 */
sai_status_t xpSaiAclMapperTableIdGet(sai_acl_stage_t stage,
                                      sai_uint32_t *tableId, bool switchEn);

/**
 * Function: xpSaiAclMapperCreateTableGrpId
 * \brief Create ACL table groupId
 * \param [in] devId     -- (input) switch device ID
 * \param [out] tblGrpId -- (output) returns group ID for application level
 * \param [in] stage     -- (input) SAI_ACL_STAGE_INGRESS or SAI_ACL_STAGE_EGRESS
 *
 * \return sai_status_t
 */
sai_status_t xpSaiAclMapperCreateTableGrpId(xpsDevice_t devId,
                                            sai_uint32_t *tblGrpId, xpSaiAclTableGrpAttributesT *attibutes);

/**
 * Function: xpSaiAclMapperDeleteTableGrpId
 * \brief Delete ACL table groupId
 * \param [in] devId     -- (input) switch device ID
 * \param [in] tblGrpId  -- (input) group ID from application level
 *
 * \return sai_status_t
 */
sai_status_t xpSaiAclMapperRemoveTableGrpId(xpsDevice_t devId,
                                            sai_uint32_t tblGrpId);

/**
 * Function: xpSaiAclMapperCreateTableGrpMembId
 * \brief Create ACL table group memberId
 * \param [in] devId      -- (input) switch device ID
 * \param [out] grpMembId -- (output) returns group member ID for application level
 * \param [in] grpId      -- (input) related group ID for the member
 * \param [in] tblId      -- (input) related table ID for the member
 * \param [in] prio       -- (input) priority for the member
 *
 * \return sai_status_t
 */
sai_status_t xpSaiAclMapperCreateTableGrpMembId(xpsDevice_t devId,
                                                sai_uint32_t *grpMembId, xpSaiAclTableGrpMembAttributesT *attibutes);

/**
 * Function: xpSaiAclMapperDeleteTableGrpId
 * \brief Delete ACL table group memberId
 * \param [in] devId     -- (input) switch device ID
 * \param [in] grpMembId -- (input) group member ID from application level
 *
 * \return sai_status_t
 */
sai_status_t xpSaiAclMapperRemoveTableGrpMembId(xpsDevice_t devId,
                                                sai_uint32_t grpMembId);

/**
 * Function: xpSaiAclMapperTableSizeSet
 * \brief Setting maximum entry number per table
 *
 * \param [in] keyFormat -- (input) Table entry
 * \param [in] tableSize -- (input) maximum entry number per table
 * \param [in] keySize   -- (input) key size for this table
 *
 * \return sai_status_t
 */
sai_status_t xpSaiAclMapperTableSizeSet(xpSaiAclTableIdMappingT *keyFormat,
                                        sai_uint32_t tableSize, sai_uint32_t keySize);

/**
 * Function: xpSaiAclMapperEntryCreate
 * \brief Create a new ACL entry
 *
 * @param sai_uint32_t entryId -- (output) application level entry ID
 *
 * \return sai_status_t
 */
sai_status_t xpSaiAclMapperEntryCreate(sai_uint32_t* entryId);

/**
 * Function: xpSaiAclMapperEntryAttributesSet
 * \brief Store entry attributes
 *
 * \param entry_id    -- (input) entry ID from application level
 * \param pAttributes -- (input) array of attributes
 *
 * \return sai_status_t
 */
sai_status_t xpSaiAclMapperEntryAttributesSet(sai_uint32_t entry_id,
                                              xpSaiAclEntryAttributesT* pAttributes);

/**
 * Function: xpSaiAclMapperEntryAttributesGet
 * \brief Retreive entry attributes
 *
 * \param entry_id    -- (input) entry ID from application level
 * \param pAttributes -- (output) array of attributes
 *
 * \return sai_status_t
 */
sai_status_t xpSaiAclMapperEntryAttributesGet(sai_uint32_t entry_id,
                                              xpSaiAclEntryAttributesT** pAttributes);

/**
 * Function: xpSaiAclMapperCounterIdGet
 * \brief Get tableId from table type and ACL stage
 *
 * \param [in] tableId   -- (input) table ID from application level
 * \param [in] counterId -- (output) pointer to application level counter ID
 *
 * \return sai_status_t
 */
sai_status_t xpSaiAclMapperCounterIdGet(sai_uint32_t devId,
                                        sai_uint32_t tableId, sai_uint32_t *counterId);

/**
 * Function: xpSaiAclMapperCounterIdDelete
 * \brief Delete counterId from the system
 *
 * \param [in] counterId -- (input) application level counter ID
 *
 * \return sai_status_t
 */
sai_status_t xpSaiAclMapperCounterIdDelete(sai_uint32_t devId,
                                           sai_uint32_t counterId);

/**
 * Function: xpSaiAclMapperCounterIdAttributesUpdate
 * \brief Update counter attributes
 *
 * @param counterId -- (input) application level counter ID
 * @param attributes -- (input) attributes
 *
 * \return sai_status_t
 */
sai_status_t xpSaiAclMapperCounterIdAttributesUpdate(sai_uint32_t counterId,
                                                     xpSaiAclCounterAttributesT attributes);

/**
 * Function: xpSaiAclMapperCounterIdAttributesGet
 * \brief Get counter attributes
 *
 * \param [in] counterId -- (input) application level counter ID
 * \param xpSaiAclCounterAttributesT attributes -- (output) counter attributes structure
 *
 * \return sai_status_t
 */
sai_status_t xpSaiAclMapperCounterIdAttributesGet(sai_uint32_t counterId,
                                                  xpSaiAclCounterAttributesT *attributes);

/**
 * Function: xpSaiAclMapperEntryIdFromCounterIdGet
 * \brief Get entryId corresponding to counterId
 *
 * \param [in] counterId -- (input) application level counter ID
 * \param [in] entryId  -- (output) application level entry ID
 *
 * \return sai_status_t
 */
sai_status_t xpSaiAclMapperEntryIdFromCounterIdGet(sai_uint32_t counterId,
                                                   sai_uint32_t *entryId);

/**
 * Function: xpSaiAclMapperCounterEntryIdSet
 * \brief Set entryId corresponding to counterId
 *
 * \param [in] counterId -- (input) application level counter ID
 * \param [in] entryId -- (input) application level entry ID
 *
 * \return sai_status_t
 */
sai_status_t xpSaiAclMapperCounterEntryIdSet(sai_uint32_t counterId,
                                             sai_uint32_t entryId);

/**
 * Function: xpSaiAclMapperCounterEntryIdClear
 * \brief Clears entryId for counterId that corresponds to entryId
 *
 * \param [in] entryId -- (input) application level entry ID
 *
 * \return sai_status_t
 */
sai_status_t xpSaiAclMapperCounterEntryIdClear(sai_uint32_t entryId);

/**
 * Function: xpSaiAclMapperAclRangeCreate
 * \brief Create a new ACL range
 *
 * @param sai_uint32_t entryId -- (output) application level acl range ID
 *
 * \return sai_status_t
 */
sai_status_t xpSaiAclMapperAclRangeCreate(sai_uint32_t* entryId);
/**
 * Function: xpSaiAclMapperAclRangeAttributesSet
 * \brief Store acl range attributes
 *
 * \param entry_id    -- (input) entry ID from application level
 * \param pAttributes -- (output) array of attributes
 *
 * \return sai_status_t
 */
sai_status_t xpSaiAclMapperAclRangeAttributesSet(sai_uint32_t entry_id,
                                                 xpSaiAclRangeAttributesT* pAttributes);
/**
 * Function: xpSaiAclMapperAclRangeAttributesGet
 * \brief Retreive entry attributes
 *
 * \param entry_id    -- (input) entry ID from application level
 * \param pAttributes -- (output) array of attributes
 *
 * \return sai_status_t
 */
sai_status_t xpSaiAclMapperAclRangeAttributesGet(sai_uint32_t entry_id,
                                                 xpSaiAclRangeAttributesT** pAttributes);
/**
 * Function: xpSaiAclMapperAclRangeDelete
 * \brief Remove state for acl range
 *
 * \param [in] devId -- (input) device ID
 * \param [in] entryId -- (input) entry ID from application level
 *
 * \return   sai_status_t
 */
sai_status_t xpSaiAclMapperAclRangeDelete(xpsDevice_t devId,
                                          sai_uint32_t entryId);
/**
 * Function: xpSaiAclTableAttributesGet
 * \brief Get table type and stage from table id
 *
 * \param [in]  tableId  -- (input) table ID from application level
 * \param [in] iaclType      -- (output) pointer to table attributes struct
 *
 * \return sai_status_t
 */
sai_status_t xpSaiAclTableAttributesGet(sai_uint32_t tableId,
                                        xpSaiAclTableIdMappingT **pSaiAclTableAttribute);

/**
 * Function: xpSaiAclMapperTableAttributesSet
 * \brief Store table attributes
 *
 * \param tableId     -- (input) table ID from application level
 * \param pAttributes -- (input) array of attributes
 *
 * \return sai_status_t
 */
sai_status_t xpSaiAclMapperTableAttributesSet(sai_uint32_t tableId,
                                              xpSaiAclTableAttributesT* pAttributes, sai_uint32_t keySize);

/**
 * Function: xpSaiGetAclMapperTableAttributes
 * \brief Get table attributes
 *
 * \param [in]        tableId -- (input) table ID from application level
 * \param [in]        attr_count -- (input) number of attributes
 * \param sai_attribute_t *attr_list -- (output) array of attributes
 *
 * \return sai_status_t
 */
sai_status_t xpSaiGetAclMapperTableAttributes(sai_uint32_t tableId,
                                              sai_uint32_t attr_count, sai_attribute_t *attr_list);

/**
 * Function: xpSaiBulkGetAclMapperTableAttributes
 * \brief Bulk Get ACL table attributes
 *
 * \param [in]        tableId -- (input) table ID from application level
 * \param [in/out]    *attr_count -- (input/output) pointer to the number of attributes
 * \param [in/out]    *attr_list -- (input/output) pointer to the array of attributes
 *
 * \return sai_status_t
 */
sai_status_t xpSaiBulkGetAclMapperTableAttributes(sai_uint32_t tableId,
                                                  sai_uint32_t *attr_count, sai_attribute_t *attr_list);

/**
 * Function: xpSaiAclMapperTableAttributesDelete
 * \brief Delete table attributes
 *
 * \param [in] tableId -- (input) table ID from application level
 *
 * \return sai_status_t
 */
sai_status_t xpSaiAclMapperTableAttributesDelete(sai_uint32_t tableId);

/**
 * Function: xpSaiGetAclMapperTableGrpAttributes
 * \brief Get table group attributes
 *
 * \param [in]   groupId    -- (input) table group ID from application level
 * \param [in]   attr_count -- (input) number of attributes
 * \param [out] *attr_list  -- (output) array of attributes
 *
 * \return sai_status_t
 */
sai_status_t xpSaiGetAclMapperTableGrpAttributes(sai_uint32_t groupId,
                                                 sai_uint32_t attr_count, sai_attribute_t *attr_list);

/**
 * Function: xpSaiGetAclMapperTableGrpMembAttributes
 * \brief Get table group member attributes
 *
 * \param [in]   memberId   -- (input) table group member ID from application level
 * \param [in]   attr_count -- (input) number of attributes
 * \param [out] *attr_list  -- (output) array of attributes
 *
 * \return sai_status_t
 */
sai_status_t xpSaiGetAclMapperTableGrpMembAttributes(sai_uint32_t memberId,
                                                     sai_uint32_t attr_count, sai_attribute_t *attr_list);
sai_status_t
xpSaiAclMapperAclGroupMemberAttributesGet(sai_uint32_t entry_id,
                                          xpSaiAclTableGrpMembIdMappingT** pAttributes);
/**
 * Function: xpSaiGetACLMapperTableGrpTableIdList
 * \brief Get tableId list for specified table group
 *
 * \param [in]      groupId     -- (input) table group ID
 * \param [in/out] *count       -- (input) number of items in a list
 * \param [out]    *tableIdList -- (output) array of tableId
 *
 * \return sai_status_t
 */
sai_status_t xpSaiGetACLMapperTableGrpTableIdList(sai_object_id_t groupId,
                                                  sai_uint32_t *count, sai_uint32_t *tableIdList);

/**
 * Function: xpSaiAclMapperAclIdGenerate
 * \brief Generate Acl Id based on portId and tableId
 *
 * \param [in] tableId -- (input) table ID from application level
 * \param [in] portId            -- (input) port ID
 * \param [in]             *aclId -- (output) acl ID for XPS
 *
 * \return sai_status_t
 */
sai_status_t xpSaiAclMapperAclIdGenerate(sai_uint32_t tableId,
                                         sai_uint32_t portId, sai_uint8_t *aclId);

/**
 * Function: xpSaiAclMapperRuleIdGet
 * \brief Get rule ID array and rule count from application level table ID and entry ID
 *
 * \param [in] devId -- (input) device ID
 * \param [in] tableId -- (input) table ID from application level
 * \param [in] entryId -- (input) CP application entry ID
 * \param [in] ruleId -- (output) returns pointer to XPS rule ID array
 *
 * \return sai_status_t
 */
sai_status_t xpSaiAclMapperRuleIdGet(xpsDevice_t devId, sai_uint32_t tableId,
                                     sai_uint32_t entryId, sai_uint32_t *ruleId);

/**
 * Function: xpSaiAclMapperEntryKeyDeleteWithoutCntr
 * \brief Delete entry which points to ruleId, Dont delete the counters, This is for rules without counters
 *
 * \param [in] devId -- (input) device ID
 * \param [in] entryId -- (input) entry ID from application level
 *
 * \return   sai_status_t
 */
sai_status_t xpSaiAclMapperEntryKeyDeleteWithoutCntr(xpsDevice_t devId,
                                                     sai_uint32_t entryId);

/**
 * Function: xpSaiAclMapperEntryDelete
 * \brief Delete entry which points to ruleId
 *
 * \param [in] devId -- (input) device ID
 * \param [in] entryId -- (input) entry ID from application level
 *
 * \return   sai_status_t
 */
sai_status_t xpSaiAclMapperEntryDelete(xpsDevice_t devId, sai_uint32_t entryId);

/**
 * Function: xpSaiAclMapperTableIdMappingShow
 * \brief Print TableId mapping array
 *
 * \return sai_status_t
 */
sai_status_t xpSaiAclMapperTableIdMappingShow(void);

/**
 * Function: xpSaiAclMapperEntryIdMappingShow
 * \brief Print Entry Id mapping array
 *
 * \param [in] devId -- (input) device ID
 * \param [in] tableId -- (input) table ID from application level
 *
 * \return sai_status_t
 */
sai_status_t xpSaiAclMapperEntryIdMappingShow(xpsDevice_t devId,
                                              sai_uint32_t tableId);

/**
 * Function: xpSaiAclMapperTcamEntryCreate
 * @brief Add TCAM entry into XPS DB
 *
 * @param devId     -- (input) device ID
 * @param tableId   -- (input) table ID from application level
 * @param entryId   -- (input) entry ID from application level
 * @param counterId -- (input) counter ID from application level
 *
 * @return sai_status_t
 */
sai_status_t xpSaiAclMapperTcamEntryCreate(sai_uint32_t devId,
                                           sai_uint32_t tableId, sai_uint32_t entryId, sai_uint32_t counterId);

/**
 * Function: xpSaiAclMapperTcamEntryDelete
 * @brief Remove TCAM entry FROM_CPU_XPH_SIZE XPS DB
 *
 * @param devId   -- (input) device ID
 * @param tableId   -- (input) table ID from application level
 * @param entryId -- (input) entry ID from application level
 *
 * @return sai_status_t
 */
sai_status_t xpSaiAclMapperTcamEntryDelete(sai_uint32_t devId,
                                           sai_uint32_t tableId, sai_uint32_t entryId);

/**
 * Function: xpSaiAclMapperTableIdRemove
 * @brief Remove Table ID from DB
 *
 * @param tableId   -- (input) table ID from application level
 *
 * @return sai_status_t
 */

sai_status_t xpSaiAclMapperTableIdRemove(sai_uint32_t tableId);
/**
 * Function: xpSaiAclMapperRemoveEntryInTable
 * @brief Remove entry in entry list from table DB
 *
 * @param tableId   -- (input) table ID from application level
 * @param entryId   -- (input) entry id
 *
 * @return sai_status_t
 */

sai_status_t xpSaiAclMapperRemoveEntryInTable(sai_uint32_t tableId,
                                              sai_uint32_t entryId);
/**
 * Function: xpSaiAclMapperAddEntryInTable
 * @brief add entry in entry list from table DB
 *
 * @param tableId   -- (input) table ID from application level
 * @param entryId   -- (input) entry id
 *
 * @return sai_status_t
 */
sai_status_t xpSaiAclMapperAddEntryInTable(sai_uint32_t tableId,
                                           sai_uint32_t entryId);
/**
 * Function: xpSaiDeleteAclMapperRsnCodeToQueueMap
 * @brief Add Mapping for Reason code to Queu
 *
 * @param xpsDevId   -- (input) Device Id
 * @param rsnCode    -- (input) Reason code
 * @param queueId    -- (input) Queue Id
 *
 * @return sai_status_t
 */

sai_status_t xpSaiAddAclMapperRsnCodeToQueueMap(sai_uint32_t xpsDevId,
                                                sai_uint32_t rsnCode, sai_uint32_t queueId);

/**
 * Function: xpSaiDeleteAclMapperRsnCodeToQueueMap
 * @brief Remove Mapping of Reason code and Queue
 *
 * @param xpsDevId   -- (input) Device Id
 * @param rsnCode    -- (input) Reason code
 *
 * @return sai_status_t
 */

sai_status_t xpSaiDeleteAclMapperRsnCodeToQueueMap(sai_uint32_t xpsDevId,
                                                   sai_uint32_t rsnCode);

/**
 * Function: saiAclMapperVirTableToXpsTable
 * @brief    convert virtual table Id to xps Table Id
 *
 * @param tableId    -- (input) tableId
 *
 * @return sai_uint32_t (xpsTableId)
 */
sai_uint32_t saiAclMapperVirTableToXpsTable(sai_uint32_t tableId);

sai_status_t xpSaiCountAclMapperTableObjects(sai_uint32_t *count);
sai_status_t xpSaiCountAclMapperTableGrpObjects(sai_uint32_t *count);
sai_status_t xpSaiCountAclMapperTableGrpMembObjects(sai_uint32_t *count);
sai_status_t xpSaiCountAclMapperCounterObjects(sai_uint32_t *count);
sai_status_t xpSaiCountAclMapperEntryObjects(sai_uint32_t *count);
sai_status_t xpSaiGetAclMapperTableObjectList(sai_uint32_t *object_count,
                                              sai_object_key_t *object_list);
sai_status_t xpSaiGetAclMapperCounterObjectList(sai_uint32_t *object_count,
                                                sai_object_key_t *object_list);
sai_status_t xpSaiGetAclMapperEntryObjectList(sai_uint32_t *object_count,
                                              sai_object_key_t *object_list);
sai_status_t xpSaiUnSetAclToPolicerMap(sai_object_id_t policer_id,
                                       xpsPolicerType_e client);
sai_status_t xpSaiAclStaticTableMappingGet(sai_uint32_t tableId,
                                           xpSaiAclStaticTableMappingT **pSaiAclStaticTableMapping);
sai_status_t xpSaiAclGetAcmCounterValue(xpsDevice_t devId, sai_uint32_t tableId,
                                        sai_acl_stage_t stage, sai_uint32_t seValue, sai_uint32_t ruleId,
                                        sai_uint64_t *pktCounterValue, sai_uint64_t *byteCounterValue);
sai_status_t xpSaiAclMapperInsertTableId(xpsDevice_t devId,
                                         xpSaiAclTableIdMappingT  *keyFormat);
sai_status_t xpSaiAclMapperRemoveTableId(xpsDevice_t devId, uint32_t tableId);
sai_status_t xpSaiAclMapperBindToTableGroup(xpsDevice_t devId,
                                            sai_object_id_t tableGrpObjId, sai_object_id_t ObjId);
sai_status_t xpSaiAclMapperUnBindFromTableGroup(xpsDevice_t devId,
                                                sai_object_id_t tableGrpObjId, sai_object_id_t interfaceObjId);
sai_status_t xpSaiAclMapperGetTableGroupStateData(xpsDevice_t devId,
                                                  sai_object_id_t tableGrpObjId, xpSaiAclTableGrpIdMappingT **ppTblGrpEntry);
sai_status_t  xpSaiAclValidateEntryAttr(const sai_uint32_t attr_count,
                                        const sai_attribute_t* attr_list,
                                        xpSaiAclTableIdMappingT *pSaiAclTableAttribute);
sai_status_t
xpSaicustAclMapperTableAttributesSet(sai_uint32_t tableId,
                                     xpSaiAclTableAttributesT* pAttributes, sai_uint32_t keySize);
sai_status_t xpSaicustAclMapperInit(xpsDevice_t devId);
sai_status_t xpSaicustAclMapperTableIdRemove(sai_uint32_t tableId);
sai_status_t xpSaicustAclMapperTableIdGet(sai_acl_stage_t stage,
                                          sai_uint32_t *tableId, sai_uint32_t priority);
sai_status_t xpSaicustCreateInPortsAclEntry(xpsDevice_t devId,
                                            sai_uint32_t tableId,
                                            xpSaiAclTableIdMappingT  *pSaiAclTableAttribute,
                                            xpSaiAclEntryAttributesT *pSaiAclEntryAttributes);
sai_status_t xpSaiAclUpdateMemberInfo(sai_object_id_t aclObjId,
                                      xpsPort_t xpsPortId, bool bindAcl);

#ifdef __cplusplus
}
#endif

#endif //_xpSaiAclMapper_h_

