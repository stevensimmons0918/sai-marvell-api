// xpSaiRpfGroup.h

/*******************************************************************************
* copyright (c) 2021 marvell. all rights reserved. the following file is       *
* subject to the limited use license agreement by and between marvell and you, *
* your employer or other entity on behalf of whom you act. in the absence of   *
* such license agreement the following file is subject to marvell’s standard   *
* limited use license agreement.                                               *
********************************************************************************/

#ifndef _xpSaiRpfGroup_h_
#define _xpSaiRpfGroup_h_

#include "xpSai.h"

#ifdef __cplusplus
extern "C" {
#endif

/** Defines default number of IP RPF Group Members */
#define XP_SAI_DEFAULT_MAX_RPF_GROUP_MEMBERS 16

/** Defines default number of L3 MC entry associated to IP RPF Group */
#define XP_SAI_DEFAULT_MAX_RPF_GROUP_MC_ENTRY_HW_INDEX 16

#define XP_SAI_RPF_GROUP_HW_IDX_IPV6 0x80000000

/**
 * @brief SAI RPF Group Member Attributes struct
 */
typedef struct _xpSaiRpfGroupMemberAttributesT
{
    sai_object_id_t groupId;
    sai_object_id_t rpfInterfaceId;

} xpSaiRpfGroupMemberAttributesT;

/**
 * @brief SAI RPF Group Member Data Base Entry struct
 */
typedef struct _xpSaiRpfGroupMemberContextDbEntry
{
    /** RPF Group Member Id OID as a key */
    sai_object_id_t groupMemberIdOid;

    /** RPF Group Id OID */
    sai_object_id_t groupIdOid;

    /** Output Id OID */
    sai_object_id_t rpfInterfaceIdOid;

} xpSaiRpfGroupMemberContextDbEntry;

/**
 * @brief SAI RPF Group Data Base Entry struct for maintaining the memberOid's
 */
typedef struct _xpSaiRpfGroupContextDbEntry
{
    /** RPF Group Id OID as a key */
    sai_object_id_t groupIdOid;

    /** group members info */
    uint32_t numItems;
    sai_object_id_t rpfGroupMemberIdOid[XP_SAI_DEFAULT_MAX_RPF_GROUP_MEMBERS];

} xpSaiRpfGroupContextDbEntry;

/**
 * @brief SAI RPF Group Data Base Entry struct for maintaining the mcEntryHwIndex
 */
typedef struct _xpSaiRpfGroupMcEntryHwIdxDbEntry
{
    /** RPF Group Id OID as a key */
    sai_object_id_t groupIdOid;

    /** group members info */
    uint32_t numItems;
    uint32_t rpfGroupMcEntryHwIdx[XP_SAI_DEFAULT_MAX_RPF_GROUP_MC_ENTRY_HW_INDEX];

} xpSaiRpfGroupMcEntryHwIdxDbEntry;

sai_status_t xpSaiGetRpfValue(sai_object_id_t rpfGroupId, uint32_t *rpfValue,
                              xpMulticastRpfCheckType_e *rpfType);
sai_status_t xpSaiRpfGroupMcEntryHwIdxAddElement(xpsDevice_t xpsDevId,
                                                 sai_object_id_t groupIdOid, uint32_t mcEntryHwIndex);
sai_status_t xpSaiRpfGroupMcEntryHwIdxDelElement(xpsDevice_t xpsDevId,
                                                 sai_object_id_t groupIdOid, uint32_t mcEntryHwIndex);
sai_status_t xpSaiRpfGroupMcEntryHwIdxGetRpfGroupOId(xpsDevice_t xpsDevId,
                                                     uint32_t mcEntryHwIndex, sai_object_id_t *rpfGroupId);
sai_status_t xpSaiRpfGroupMcEntryHwIdxReplaceHwIdx(xpsDevice_t xpsDevId,
                                                   uint32_t mcEntryHwIndex, uint32_t newMcEntryHwIndex);

/**
 * \brief Standard xpSai module initialization interface
 */
sai_status_t xpSaiRpfGroupApiInit(uint64_t flag,
                                  const sai_service_method_table_t* adapHostServiceMethodTable);
sai_status_t xpSaiRpfGroupApiDeinit();
sai_status_t xpSaiRpfGroupInit(xpsDevice_t xpSaiDevId);
sai_status_t xpSaiRpfGroupDeInit(xpsDevice_t xpSaiDevId);

#ifdef __cplusplus
}
#endif

#endif //_xpSaiRpfGroup_h_
