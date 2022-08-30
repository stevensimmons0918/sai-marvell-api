// xpSaiNextHopGroup.c

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
********************************************************************************/

#include "xpSai.h"
#include "xpSaiNextHopGroup.h"
#include "xpsNhGrp.h"
#include "xpSaiValidationArrays.h"
#include "xpSaiStub.h"
extern bool xpSaiNextHopDbExists(sai_object_id_t nh_oid);
XP_SAI_LOG_REGISTER_API(SAI_API_NEXT_HOP_GROUP);

/**
 * Nexthop group attributes
 */
typedef struct _xpSaiNextHopGroupAttributesT
{
    sai_attribute_value_t type;
} xpSaiNextHopGroupAttributesT;

/**
 * Nexthop group member attributes
 */
typedef struct _xpSaiNextHopGroupMemberAttributesT
{
    sai_attribute_value_t nextHopGroupId; /* Parent group id */
    sai_attribute_value_t nextHopId;
    sai_attribute_value_t weight;
} xpSaiNextHopGroupMemberAttributesT;

static sai_next_hop_group_api_t* _xpSaiNextHopGroupApi;

/**
 * NH group member information
 */
typedef struct _xpSaiNextHopGroupMemberInfo_t
{
    sai_object_id_t nhGrpMemberId;
    sai_object_id_t nhGrpId; /* Parent group id */
    sai_object_id_t nhId;   /* Associated nexthop id */
} xpSaiNextHopGroupMemberInfo_t;

typedef struct _xpSaiNextHopGroupInfo_t
{
    sai_object_id_t nhGrpId; /* Parent group id */
    uint32_t routeRefCount;   /* Route reference count corresponding to NhGrpId */
    uint32_t aclRefCount;   /* ACL (PBR) reference count corresponding to NhGrpId */
} xpSaiNextHopGroupInfo_t;

/*
 * Macros for creating/retrieving NH group member id from NH group id and NH id
 */
#define XP_SAI_NH_MEMBER_ID_GROUPID_SHIFT   (24)
#define XP_SAI_NH_MEMBER_ID_GROUPID_MASK    (0xFFFFFF)
#define XP_SAI_NH_MEMBER_ID_NHID_SHIFT      (0)
#define XP_SAI_NH_MEMBER_ID_NHID_MASK       (0xFFFFFF)

/**
 * Declarations for static functions
 */
static sai_status_t xpSaiNextHopGroupGetMemberInfo(sai_object_id_t nhMemberId,
                                                   xpSaiNextHopGroupMemberInfo_t *info);
static inline sai_status_t xpSaiNextHopGroupDestroyXpsNextHopGroup(
    uint32_t xpsNhId);
static inline sai_status_t xpSaiNextHopGroupAddXpsNextHop(
    sai_object_id_t nhGrpId, sai_object_id_t nhId, uint32_t weight);
static inline sai_status_t xpSaiNextHopGroupRemoveXpsNextHop(
    sai_object_id_t nhGrpId, sai_object_id_t nhId);
static inline sai_status_t xpSaiNextHopGroupSetXpsNextHopWeight(
    sai_object_id_t nhGrpId, sai_object_id_t nhId, uint32_t weight);
static inline sai_status_t xpSaiNextHopGroupGetXpsNextHopWeight(
    sai_object_id_t nhGrpId, sai_object_id_t nhId, uint32_t *weight);
static uint8_t xpSaiNextHopGroupContainsNextHop(sai_object_id_t nhGrpId,
                                                sai_object_id_t nhId);
static void xpSaiNextHopGroupMemberCreateId(sai_object_id_t nhGrpId,
                                            sai_object_id_t nhId, sai_object_id_t *nhMemberId);
static sai_status_t xpSaiNextHopGroupGetMemberInfo(sai_object_id_t nhMemberId,
                                                   xpSaiNextHopGroupMemberInfo_t *info);
static inline sai_status_t xpSaiNextHopGroupGetSaiId(uint32_t xpsNhGrpId,
                                                     sai_object_id_t *saiNhGrpId);
static inline sai_status_t xpSaiNextHopGroupGetXpsId(sai_object_id_t saiNhGrpId,
                                                     uint32_t *xpsNhGrpId);
static sai_status_t xpSaiGetNextHopGroupAttrNextHopMemberList(uint32_t nhGrpId,
                                                              sai_attribute_value_t* pValue);
static sai_status_t xpSaiNextHopGroupMemberSetAttributeWeight(
    sai_object_id_t nhGrpId, sai_object_id_t nhId, uint32_t weight);
static sai_status_t xpSaiNextHopGroupMemberGetAttributeWeight(
    sai_object_id_t nhGrpId, sai_object_id_t nhId, uint32_t *weight);
static sai_status_t xpSaiGetNextHopGroupAttrNextHopCount(uint32_t nhGrpId,
                                                         sai_attribute_value_t* pValue);
static sai_status_t xpSaiGetNextHopGroupAttrType(uint32_t nhGrpId,
                                                 sai_attribute_value_t* pValue);
static sai_status_t xpSaiCreateNextHopGroup(sai_object_id_t *next_hop_group_id,
                                            sai_object_id_t switch_id, uint32_t attr_count,
                                            const sai_attribute_t *attr_list);
static sai_status_t xpSaiRemoveNextHopGroup(sai_object_id_t next_hop_group_id);
static sai_status_t xpSaiSetNextHopGroupAttribute(sai_object_id_t
                                                  next_hop_group_id, const sai_attribute_t *attr);
static sai_status_t xpSaiGetNextHopGroupAttribute(sai_object_id_t saiNhGrpId,
                                                  sai_attribute_t *attr, uint32_t attr_index);
static sai_status_t xpSaiGetNextHopGroupAttributes(sai_object_id_t
                                                   next_hop_group_id, uint32_t attr_count, sai_attribute_t *attr_list);
static sai_status_t xpSaiCreateNextHopGroupMember(sai_object_id_t*
                                                  next_hop_group_member_id, sai_object_id_t switch_id, uint32_t attr_count,
                                                  const sai_attribute_t *attr_list);
static sai_status_t xpSaiRemoveNextHopGroupMember(sai_object_id_t
                                                  next_hop_group_member_id);
static sai_status_t xpSaiSetNextHopGroupMemberAttribute(
    sai_object_id_t next_hop_group_member_id, const sai_attribute_t *attr);
static sai_status_t xpSaiGetNextHopGroupMemberAttributes(
    sai_object_id_t next_hop_group_member_id, uint32_t attr_count,
    sai_attribute_t *attr_list);

static sai_status_t xpSaiRemoveNextHopGroupMembers(uint32_t object_count,
                                                   const sai_object_id_t *object_id, sai_bulk_op_error_mode_t mode,
                                                   sai_status_t *object_statuses);

static sai_status_t xpSaiCreateNextHopGroupMembers(sai_object_id_t switch_id,
                                                   uint32_t object_count, const uint32_t *attr_count,
                                                   const sai_attribute_t **attr_list,
                                                   sai_bulk_op_error_mode_t mode, sai_object_id_t *object_id,
                                                   sai_status_t *object_statuses);
/* DB handle for the Nexthop group information database */
static xpsDbHandle_t gSaiNextHopGroupInfoDbHndl = XPS_STATE_INVALID_DB_HANDLE;

/* Key compare routine for Nexthop group member information database */
static int32_t xpSaiNextHopGroupInfoKeyCompare(void* key1, void* key2)
{
    xpSaiNextHopGroupInfo_t *info1 = (xpSaiNextHopGroupInfo_t *)key1;
    xpSaiNextHopGroupInfo_t *info2 = (xpSaiNextHopGroupInfo_t *)key2;

    if (info1->nhGrpId < info2->nhGrpId)
    {
        return -1;
    }
    else if (info1->nhGrpId > info2->nhGrpId)
    {
        return 1;
    }
    else /* Both are equal */
    {
        return 0;
    }
}

// DB search API
static sai_status_t xpSaiNextHopGroupGetInfo(sai_object_id_t nhGrpId,
                                             xpSaiNextHopGroupInfo_t **info)
{
    XP_STATUS xpStatus = XP_NO_ERR;
    xpSaiNextHopGroupInfo_t keyDbEntry;

    memset(&keyDbEntry, 0, sizeof(keyDbEntry));
    keyDbEntry.nhGrpId = nhGrpId;
    xpStatus = xpsStateSearchData(XP_SCOPE_DEFAULT, gSaiNextHopGroupInfoDbHndl,
                                  (xpsDbKey_t)&keyDbEntry, (void **)info);
    if (xpStatus != XP_NO_ERR)
    {
        return xpsStatus2SaiStatus(xpStatus);
    }

    return SAI_STATUS_SUCCESS;
}

// DB insert API
static sai_status_t xpSaiNextHopGroupInfoInsert(sai_object_id_t nhGrpId)
{
    XP_STATUS xpStatus = XP_NO_ERR;

    xpSaiNextHopGroupInfo_t *dbEntry = NULL;

    /* Allocate memotry from heap */
    xpStatus = xpsStateHeapMalloc(sizeof(xpSaiNextHopGroupInfo_t),
                                  (void **)&dbEntry);
    if (xpStatus != XP_NO_ERR || dbEntry == NULL)
    {
        return xpsStatus2SaiStatus(xpStatus);
    }

    /* Copy the input data */
    memset(dbEntry, 0, sizeof(xpSaiNextHopGroupInfo_t));

    dbEntry->nhGrpId = nhGrpId;
    /* Insert the entry to DB */
    xpStatus = xpsStateInsertData(XP_SCOPE_DEFAULT, gSaiNextHopGroupInfoDbHndl,
                                  (void *)dbEntry);
    if (xpStatus != XP_NO_ERR)
    {
        xpsStateHeapFree((void *)dbEntry);
        return xpsStatus2SaiStatus(xpStatus);
    }

    return SAI_STATUS_SUCCESS;
}

// DB delete API
static sai_status_t xpSaiNextHopGroupInfoDelete(sai_object_id_t nhGrpId)
{
    XP_STATUS xpStatus = XP_NO_ERR;
    xpSaiNextHopGroupInfo_t keyDbEntry;
    xpSaiNextHopGroupInfo_t *dbEntry = NULL;

    memset(&keyDbEntry, 0, sizeof(keyDbEntry));

    /* Delete the data from DB */
    keyDbEntry.nhGrpId = nhGrpId;
    xpStatus = xpsStateDeleteData(XP_SCOPE_DEFAULT, gSaiNextHopGroupInfoDbHndl,
                                  (xpsDbKey_t)&keyDbEntry, (void **)&dbEntry);
    if (xpStatus != XP_NO_ERR)
    {
        return xpsStatus2SaiStatus(xpStatus);
    }

    if (dbEntry == NULL)
    {
        XP_SAI_LOG_ERR("Could not find next hop group in DB\n");
        return SAI_STATUS_INVALID_OBJECT_ID;
    }

    /* Free the heap memory */
    xpsStateHeapFree((void *)dbEntry);

    return SAI_STATUS_SUCCESS;
}

/********************************************************************
 *                      XPS Wrapper functions                       *
 *******************************************************************/

static inline sai_status_t xpSaiNextHopGroupDestroyXpsNextHopGroup(
    uint32_t xpsNhGrpId)
{
    xpsDevice_t xpsDevId = xpSaiGetDevId();
    XP_STATUS xpStatus;

    xpStatus = xpsL3DestroyRouteNextHopGroup(xpsDevId, xpsNhGrpId);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsL3DestroyRouteNextHopGroup() failed with error code: %d\n",
                       xpStatus);
        return xpsStatus2SaiStatus(xpStatus);
    }

    return SAI_STATUS_SUCCESS;
}

/**
 * Wrapper to add a nexthop to nexthop group in XPS layer.
 */
static inline sai_status_t xpSaiNextHopGroupAddXpsNextHop(
    sai_object_id_t nhGrpId, sai_object_id_t nhId, uint32_t weight)
{
    XP_STATUS xpStatus;
    sai_status_t saiStatus;
    uint32_t xpsNhGrpId;
    uint32_t xpsNhId;
    xpsDevice_t xpsDevId = xpSaiGetDevId();

    /* Obtain the XPS ids for NH group and NH */
    saiStatus = xpSaiNextHopGroupGetXpsId(nhGrpId, &xpsNhGrpId);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        return saiStatus;
    }
    if (!xpSaiNextHopDbExists(nhId))
    {
        XP_SAI_LOG_ERR("xpSaiNextHopGroupAddXpsNextHop NHid %" PRIx64
                       " doesn't exist ");
        return SAI_STATUS_ITEM_NOT_FOUND;
    }
    saiStatus = xpSaiConvertNextHopOid(nhId, &xpsNhId);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        return saiStatus;
    }

    xpStatus = xpsL3AddNextHopRouteNextHopGroup(xpsDevId, xpsNhGrpId, xpsNhId,
                                                weight);
    if (xpStatus != XP_NO_ERR)
    {
        return xpsStatus2SaiStatus(xpStatus);
    }

    return SAI_STATUS_SUCCESS;
}

/**
 * Wrapper to remove a nexthop to nexthop group in XPS layer.
 */
static inline sai_status_t xpSaiNextHopGroupRemoveXpsNextHop(
    sai_object_id_t nhGrpId, sai_object_id_t nhId)
{
    XP_STATUS xpStatus;
    sai_status_t saiStatus;
    uint32_t xpsNhGrpId;
    uint32_t xpsNhId;
    xpsDevice_t xpsDevId = xpSaiGetDevId();

    /* Obtain the XPS ids for NH group and NH */
    saiStatus = xpSaiNextHopGroupGetXpsId(nhGrpId, &xpsNhGrpId);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        return saiStatus;
    }

    saiStatus = xpSaiConvertNextHopOid(nhId, &xpsNhId);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        return saiStatus;
    }

    /* Program in XPS */
    xpStatus = xpsL3RemoveNextHopRouteNextHopGroup(xpsDevId, xpsNhGrpId, xpsNhId);
    if (xpStatus != XP_NO_ERR)
    {
        return xpsStatus2SaiStatus(xpStatus);
    }

    return SAI_STATUS_SUCCESS;
}

/**
 * Wrapper to set weight to a nexthop in a nexthop group in XPS layer
 */
static inline sai_status_t xpSaiNextHopGroupSetXpsNextHopWeight(
    sai_object_id_t nhGrpId, sai_object_id_t nhId, uint32_t weight)
{
    uint32_t xpsNhGrpId = 0;
    uint32_t xpsNhId = 0;
    sai_status_t saiStatus = SAI_STATUS_SUCCESS;
    XP_STATUS xpStatus = XP_NO_ERR;
    xpsDevice_t xpsDevId = xpSaiGetDevId();

    /* Obtain the XPS ids for NH group and NH */
    saiStatus = xpSaiNextHopGroupGetXpsId(nhGrpId, &xpsNhGrpId);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiNextHopGroupGetXpsId failed. RC %d\n", saiStatus);
        return saiStatus;
    }

    saiStatus = xpSaiConvertNextHopOid(nhId, &xpsNhId);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiConvertNextHopOid failed. RC %d\n", saiStatus);
        return saiStatus;
    }

    /* Set the weight to XPS */
    xpStatus = xpsL3SetNextHopWeightRouteNextHopGroup(xpsDevId, xpsNhGrpId, xpsNhId,
                                                      weight);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to set weight (%d) for nh id (%d) in nh group (%d) in XPS\n",
                       weight, xpsNhId, xpsNhGrpId);
        return xpsStatus2SaiStatus(xpStatus);
    }

    return SAI_STATUS_SUCCESS;
}

/**
 * Wrapper to get weight for a nexthop in a nexthop group in XPS layer.
 */
static inline sai_status_t xpSaiNextHopGroupGetXpsNextHopWeight(
    sai_object_id_t nhGrpId, sai_object_id_t nhId, uint32_t *weight)
{
    uint32_t xpsNhGrpId;
    uint32_t xpsNhId;
    sai_status_t saiStatus;
    XP_STATUS xpStatus;
    xpsDevice_t xpsDevId = xpSaiGetDevId();

    /* Obtain the XPS ids for NH group and NH */
    saiStatus = xpSaiNextHopGroupGetXpsId(nhGrpId, &xpsNhGrpId);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        return saiStatus;
    }

    saiStatus = xpSaiConvertNextHopOid(nhId, &xpsNhId);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        return saiStatus;
    }

    /* Set the weight to XPS */
    xpStatus = xpsL3GetNextHopWeightRouteNextHopGroup(xpsDevId, xpsNhGrpId, xpsNhId,
                                                      weight);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to get weight for nh id (%d) in nh group (%d) in XPS\n",
                       xpsNhId, xpsNhGrpId);
        return xpsStatus2SaiStatus(xpStatus);
    }

    return SAI_STATUS_SUCCESS;
}

/**
 * Checks if the nexthop group nhGrpId contains nexthop nhId.
 * Returns boolean status.
 */
static uint8_t xpSaiNextHopGroupContainsNextHop(sai_object_id_t nhGrpId,
                                                sai_object_id_t nhId)
{
    uint32_t xpsNhGrpId;
    uint32_t xpsNhId;
    sai_status_t saiStatus;
    xpsDevice_t xpsDevId = xpSaiGetDevId();

    /* Obtain the XPS ids for NH group and NH */
    saiStatus = xpSaiNextHopGroupGetXpsId(nhGrpId, &xpsNhGrpId);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiNextHopGroupGetXpsId failed. RC %d\n", saiStatus);
        return false;
    }

    saiStatus = xpSaiConvertNextHopOid(nhId, &xpsNhId);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiConvertNextHopOid failed. RC %d\n", saiStatus);
        return false;
    }

    return xpsL3ExistsNextHopRouteNextHopGroup(xpsDevId, xpsNhGrpId, xpsNhId);
}

/********************************************************************
 *              Id/Object Id/XPS Id related functions               *
 *******************************************************************/
/**
 * Creates a SAI object id for a NH group member associated with give NH group id and NH id
 */
static void xpSaiNextHopGroupMemberCreateId(sai_object_id_t nhGrpId,
                                            sai_object_id_t nhId, sai_object_id_t *nhMemberId)
{
    sai_uint64_t tmp = 0;
    tmp = (sai_uint64_t) SAI_OBJECT_TYPE_NEXT_HOP_GROUP_MEMBER <<
          XDK_SAI_OBJID_VALUE_BITS;
    tmp = tmp | ((nhGrpId & XP_SAI_NH_MEMBER_ID_GROUPID_MASK) <<
                 XP_SAI_NH_MEMBER_ID_GROUPID_SHIFT) | (nhId & XP_SAI_NH_MEMBER_ID_NHID_MASK);
    *nhMemberId = tmp;
}

static sai_status_t xpSaiNextHopGroupGetMemberInfo(sai_object_id_t nhMemberId,
                                                   xpSaiNextHopGroupMemberInfo_t *info)
{
    sai_status_t saiStatus = SAI_STATUS_SUCCESS;
    uint32_t xpsNhId = nhMemberId & XP_SAI_NH_MEMBER_ID_NHID_MASK;
    uint32_t xpsNhGrpId = (nhMemberId >> XP_SAI_NH_MEMBER_ID_GROUPID_SHIFT) &
                          XP_SAI_NH_MEMBER_ID_NHID_MASK;;
    if ((saiStatus = xpSaiNextHopGroupGetSaiId(xpsNhGrpId,
                                               &info->nhGrpId))!= SAI_STATUS_SUCCESS)
    {
        return saiStatus;
    }
    xpSaiConvertNexthopXpsId(xpsNhId, &info->nhId);
    info->nhGrpMemberId = nhMemberId;

    return SAI_STATUS_SUCCESS;
}

static inline sai_status_t xpSaiNextHopGroupGetSaiId(uint32_t xpsNhGrpId,
                                                     sai_object_id_t *saiNhGrpId)
{
    xpsDevice_t     devId       = xpSaiGetDevId();

    if (xpSaiObjIdCreate(SAI_OBJECT_TYPE_NEXT_HOP_GROUP, devId,
                         (sai_uint64_t) xpsNhGrpId, saiNhGrpId) != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("SAI object creation failed for xps NH grp id %d\n", xpsNhGrpId);
        return SAI_STATUS_FAILURE;
    }

    return SAI_STATUS_SUCCESS;
}

static inline sai_status_t xpSaiNextHopGroupGetXpsId(sai_object_id_t saiNhGrpId,
                                                     uint32_t *xpsNhGrpId)
{
    if (!XDK_SAI_OBJID_TYPE_CHECK(saiNhGrpId, SAI_OBJECT_TYPE_NEXT_HOP_GROUP))
    {
        XP_SAI_LOG_ERR("Wrong object type received for SAI NH group id(%u)",
                       xpSaiObjIdTypeGet(saiNhGrpId));
        return SAI_STATUS_INVALID_OBJECT_TYPE;
    }

    if (xpsNhGrpId == NULL)
    {
        XP_SAI_LOG_ERR("Null pointer provided as an argument\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    *xpsNhGrpId = (uint32_t)xpSaiObjIdValueGet(saiNhGrpId);

    return SAI_STATUS_SUCCESS;
}

/********************************************************************
 *              Second level Set/get attribute functions            *
 *******************************************************************/
static sai_status_t xpSaiGetNextHopGroupAttrNextHopMemberList(uint32_t nhGrpId,
                                                              sai_attribute_value_t* pValue)
{
    XP_STATUS       retVal        = XP_NO_ERR;
    uint32_t        count         = 0;
    uint32_t*       pNhId         = NULL;
    uint32_t        nhId          = 0;
    uint32_t        i             = 0;
    sai_object_id_t nhMemberObjId;

    retVal = xpsL3GetCountRouteNextHopGroup(nhGrpId, &count);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsL3GetCountRouteNextHopGroup() failed with error code: %d\n",
                       retVal);
        return xpsStatus2SaiStatus(retVal);
    }

    if (count == 0)
    {
        pValue->objlist.count = count;
        XP_SAI_LOG_INFO("NextHopGroup count is zero!\n");
        return SAI_STATUS_SUCCESS;
    }

    if (pValue->objlist.count < count)
    {
        pValue->objlist.count = count;
        return SAI_STATUS_BUFFER_OVERFLOW;
    }

    pValue->objlist.count = count;

    while ((retVal = xpsL3GetNextNextHopRouteNextHopGroup(nhGrpId, pNhId,
                                                          &nhId)) == XP_NO_ERR)
    {
        /* Get the NH member object id corresponds to the NH and NH group */
        xpSaiNextHopGroupMemberCreateId(nhGrpId, nhId, &nhMemberObjId);

        pValue->objlist.list[i] = nhMemberObjId;
        pNhId = &nhId;
        i++;
    }

    if (count != i)
    {
        /* We shouldnt get into this state at all. The count should be same as the number of NHs */
        XP_SAI_LOG_ERR("NH count %d is not equal to number of NHs %d\n", count, i);
        assert(0);
        return SAI_STATUS_FAILURE;
    }

    return SAI_STATUS_SUCCESS;
}

static sai_status_t xpSaiNextHopGroupMemberSetAttributeWeight(
    sai_object_id_t nhGrpId, sai_object_id_t nhId, uint32_t weight)
{
    sai_status_t saiStatus;

    saiStatus = xpSaiNextHopGroupSetXpsNextHopWeight(nhGrpId, nhId, weight);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        return saiStatus;
    }

    return SAI_STATUS_SUCCESS;
}

static sai_status_t xpSaiNextHopGroupMemberGetAttributeWeight(
    sai_object_id_t nhGrpId, sai_object_id_t nhId, uint32_t *weight)
{
    sai_status_t saiStatus;

    saiStatus = xpSaiNextHopGroupGetXpsNextHopWeight(nhGrpId, nhId, weight);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        return saiStatus;
    }

    return SAI_STATUS_SUCCESS;
}

/********************************************************************
 *                      Attribute set/get functions                 *
 *******************************************************************/
/**
 * SAI attributes related functions
 */
void xpSaiSetDefaultNextHopGroupAttributeVals(xpSaiNextHopGroupAttributesT*
                                              attributes)
{
    attributes->type.s32 = SAI_NEXT_HOP_GROUP_TYPE_ECMP;
}

//Func: xpSaiUpdateNextHopGroupAttributeVals

void xpSaiUpdateNextHopGroupAttributeVals(const uint32_t attr_count,
                                          const sai_attribute_t* attr_list, xpSaiNextHopGroupAttributesT* attributes)
{
    XP_SAI_LOG_DBG("Calling xpSaiUpdateNextHopGroupAttributeVals\n");

    for (uint32_t count = 0; count < attr_count; count++)
    {
        switch (attr_list[count].id)
        {
            case SAI_NEXT_HOP_GROUP_ATTR_TYPE:
                {
                    attributes->type = attr_list[count].value;
                    break;
                }
            default:
                {
                    XP_SAI_LOG_ERR("Failed to set %d\n", attr_list[count].id);
                }
        }
    }
}

void xpSaiSetDefaultNextHopGroupMemberAttributeVals(
    xpSaiNextHopGroupMemberAttributesT* attributes)
{
    attributes->weight.u32 = 1;
    attributes->nextHopId.oid = 0;
    attributes->nextHopGroupId.oid = 0;
}

//Func: xpSaiUpdateNextHopGroupMemberAttributeVals

void xpSaiUpdateNextHopGroupMemberAttributeVals(const uint32_t attr_count,
                                                const sai_attribute_t* attr_list,
                                                xpSaiNextHopGroupMemberAttributesT* attributes)
{
    XP_SAI_LOG_DBG("Calling xpSaiUpdateNextHopGroupMemberAttributeVals\n");

    for (uint32_t count = 0; count < attr_count; count++)
    {
        switch (attr_list[count].id)
        {
            case SAI_NEXT_HOP_GROUP_MEMBER_ATTR_NEXT_HOP_GROUP_ID:
                {
                    attributes->nextHopGroupId = attr_list[count].value;
                    break;
                }
            case SAI_NEXT_HOP_GROUP_MEMBER_ATTR_NEXT_HOP_ID:
                {
                    attributes->nextHopId = attr_list[count].value;
                    break;
                }
            case SAI_NEXT_HOP_GROUP_MEMBER_ATTR_WEIGHT:
                {
                    attributes->weight = attr_list[count].value;
                    break;
                }
            default:
                {
                    XP_SAI_LOG_ERR("Not Supported Attr %d\n", attr_list[count].id);
                }
        }
    }
}

static sai_status_t xpSaiGetNextHopGroupAttrNextHopCount(uint32_t nhGrpId,
                                                         sai_attribute_value_t* pValue)
{
    XP_STATUS retVal = XP_NO_ERR;

    retVal = xpsL3GetCountRouteNextHopGroup(nhGrpId, &pValue->u32);

    return xpsStatus2SaiStatus(retVal);
}

static sai_status_t xpSaiGetNextHopGroupAttrType(uint32_t nhGrpId,
                                                 sai_attribute_value_t* pValue)
{
    pValue->s32 = SAI_NEXT_HOP_GROUP_TYPE_ECMP;

    return SAI_STATUS_SUCCESS;
}

sai_status_t xpSaiIsAclNextHopGroupInUse(uint32_t nhGrpId)
{
    sai_status_t saiStatus;
    sai_object_id_t next_hop_group_id;
    xpSaiNextHopGroupInfo_t *dbEntry;

    saiStatus = xpSaiNextHopGroupGetSaiId(nhGrpId, &next_hop_group_id);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        return saiStatus;
    }

    saiStatus = xpSaiNextHopGroupGetInfo(next_hop_group_id,  &dbEntry);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        return saiStatus;
    }

    if (dbEntry == NULL)
    {
        XP_SAI_LOG_ERR("Could not find nhGrpId in DB\n");
        return SAI_STATUS_INVALID_OBJECT_ID;
    }

    if (dbEntry->aclRefCount > 0)
    {
        XP_SAI_LOG_ERR("nhGrpId : %d is part of ACL table data, cannot remove Next Hop Group\n",
                       nhGrpId);
        return SAI_STATUS_OBJECT_IN_USE;
    }

    return SAI_STATUS_SUCCESS;
}
static sai_status_t xpSaiIsNextHopGroupInUse(uint32_t nhGrpId)
{
    sai_status_t saiStatus;
    sai_object_id_t next_hop_group_id;
    xpSaiNextHopGroupInfo_t *dbEntry;

    saiStatus = xpSaiNextHopGroupGetSaiId(nhGrpId, &next_hop_group_id);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        return saiStatus;
    }

    saiStatus = xpSaiNextHopGroupGetInfo(next_hop_group_id,  &dbEntry);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        return saiStatus;
    }

    if (dbEntry == NULL)
    {
        XP_SAI_LOG_ERR("Could not find nhGrpId in DB\n");
        return SAI_STATUS_INVALID_OBJECT_ID;
    }

    if ((dbEntry->routeRefCount > 0) || (dbEntry->aclRefCount > 0))
    {
        XP_SAI_LOG_ERR("nhGrpId : %d is part of route table data, cannot remove Next Hop Group\n",
                       nhGrpId);
        return SAI_STATUS_OBJECT_IN_USE;
    }

    return SAI_STATUS_SUCCESS;
}

/********************************************************************
 *                Top level functions(registered with SAI)          *
 *******************************************************************/
static sai_status_t xpSaiCreateNextHopGroup(sai_object_id_t *next_hop_group_id,
                                            sai_object_id_t switch_id, uint32_t attr_count,
                                            const sai_attribute_t *attr_list)
{
    sai_status_t    saiStatus  = SAI_STATUS_SUCCESS;
    XP_STATUS       xpSatus    = XP_NO_ERR;
    uint32_t        xpsNhGrpId = 0;
    xpsDevice_t     xpsDevId   = xpSaiGetDevId();
    xpSaiTableEntryCountDbEntry *entryCountCtxPtr = NULL;

    xpSaiNextHopGroupAttributesT attributes;

    memset(&attributes, 0, sizeof(attributes));

    if (next_hop_group_id == NULL)
    {
        XP_SAI_LOG_ERR("Null pointer provided as an argument\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    saiStatus = xpSaiAttrCheck(attr_count, attr_list,
                               NEXT_HOP_GROUP_VALIDATION_ARRAY_SIZE, next_hop_group_attribs,
                               SAI_COMMON_API_CREATE);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Attribute check failed with error %d\n", saiStatus);
        return saiStatus;
    }

    /* Init the values */
    xpSaiSetDefaultNextHopGroupAttributeVals(&attributes);
    xpSaiUpdateNextHopGroupAttributeVals(attr_count, attr_list, &attributes);

    /*There only one type of NH group. i.e ECMP type*/
    if (attributes.type.s32 != SAI_NEXT_HOP_GROUP_TYPE_ECMP)
    {
        XP_SAI_LOG_ERR("Unsupported next hop group type: %d\n", attributes.type.s32);
        return SAI_STATUS_INVALID_ATTR_VALUE_0;
    }

    xpSatus = xpsL3CreateRouteNextHopGroupScope(xpsDevId, &xpsNhGrpId);
    if (xpSatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsL3CreateRouteNextHopGroup() failed with error code: %d\n",
                       xpSatus);
        return xpsStatus2SaiStatus(xpSatus);
    }

    saiStatus = xpSaiNextHopGroupGetSaiId(xpsNhGrpId, next_hop_group_id);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiNextHopGroupGetSaiId() failed, saiStatus %d\n", saiStatus);
        return saiStatus;
    }

    saiStatus = xpSaiNextHopGroupInfoInsert(*next_hop_group_id);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiNextHopGroupInfoInsert() failed, saiStatus %d\n",
                       saiStatus);
        return saiStatus;
    }

    saiStatus = xpSaiGetTableEntryCountCtxDb(xpsDevId, &entryCountCtxPtr);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to get Table Entry Count DB, saiStatus %d\n", saiStatus);
        return saiStatus;
    }
    entryCountCtxPtr->nextHopGroupEntries++;

    return SAI_STATUS_SUCCESS;
}

static sai_status_t xpSaiRemoveNextHopGroup(sai_object_id_t next_hop_group_id)
{
    uint32_t xpsNhGrpId;
    sai_status_t saiStatus;
    xpsDevice_t  xpsDevId = xpSaiGetDevId();
    xpSaiTableEntryCountDbEntry *entryCountCtxPtr = NULL;

    saiStatus = xpSaiNextHopGroupGetXpsId(next_hop_group_id, &xpsNhGrpId);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiNextHopGroupGetXpsId() failed, saiStatus %d\n", saiStatus);
        return saiStatus;
    }

    saiStatus = xpSaiIsNextHopGroupInUse(xpsNhGrpId);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Next hop Group already in use, nhGrpId: %d, retVal %d \n",
                       xpsNhGrpId, saiStatus);
        return saiStatus;
    }

    saiStatus = xpSaiNextHopGroupDestroyXpsNextHopGroup(xpsNhGrpId);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiNextHopGroupDestroyXpsNextHopGroup() failed, saiStatus %d\n",
                       saiStatus);
        return saiStatus;
    }

    saiStatus = xpSaiNextHopGroupInfoDelete(next_hop_group_id);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiNextHopGroupInfoDelete() failed, saiStatus %d\n",
                       saiStatus);
        return saiStatus;
    }

    saiStatus = xpSaiGetTableEntryCountCtxDb(xpsDevId, &entryCountCtxPtr);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to get Table Entry Count DB, saiStatus %d\n", saiStatus);
        return saiStatus;
    }
    entryCountCtxPtr->nextHopGroupEntries--;

    return SAI_STATUS_SUCCESS;
}

sai_status_t xpSaiIncrementNextHopGroupRouteRefCount(uint32_t nhGrpId,
                                                     bool isRouteNH)
{
    sai_status_t saiStatus;
    sai_object_id_t next_hop_group_id;
    xpSaiNextHopGroupInfo_t *dbEntry;

    saiStatus = xpSaiNextHopGroupGetSaiId(nhGrpId, &next_hop_group_id);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        return saiStatus;
    }

    saiStatus = xpSaiNextHopGroupGetInfo(next_hop_group_id, &dbEntry);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        return saiStatus;
    }

    if (dbEntry == NULL)
    {
        XP_SAI_LOG_ERR("Could not find nhGrpId in DB\n");
        return SAI_STATUS_INVALID_OBJECT_ID;
    }

    if (isRouteNH)
    {
        ++dbEntry->routeRefCount;
    }
    else
    {
        ++dbEntry->aclRefCount;
    }
    return SAI_STATUS_SUCCESS;
}

sai_status_t xpSaiDecrementNextHopGroupRouteRefCount(uint32_t nhGrpId,
                                                     bool isRouteNH)
{
    sai_status_t saiStatus;
    sai_object_id_t next_hop_group_id;
    xpSaiNextHopGroupInfo_t *dbEntry;

    saiStatus = xpSaiNextHopGroupGetSaiId(nhGrpId, &next_hop_group_id);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        return saiStatus;
    }

    saiStatus = xpSaiNextHopGroupGetInfo(next_hop_group_id, &dbEntry);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        return saiStatus;
    }

    if (dbEntry == NULL)
    {
        XP_SAI_LOG_ERR("Could not find nhGrpId in DB\n");
        return SAI_STATUS_INVALID_OBJECT_ID;
    }

    if (isRouteNH)
    {
        --dbEntry->routeRefCount;
    }
    else
    {
        --dbEntry->aclRefCount;
    }

    return SAI_STATUS_SUCCESS;
}

static sai_status_t xpSaiSetNextHopGroupAttribute(sai_object_id_t
                                                  next_hop_group_id, const sai_attribute_t *attr)
{
    /*
     * No attribute to set. This should not have been called. Return error.
     */
    return SAI_STATUS_INVALID_ATTRIBUTE_0;
}

static sai_status_t xpSaiGetNextHopGroupAttribute(sai_object_id_t saiNhGrpId,
                                                  sai_attribute_t *attr, uint32_t attr_index)
{
    sai_status_t saiStatus  = SAI_STATUS_SUCCESS;
    uint32_t     xpsNhGrpId = 0;

    saiStatus = xpSaiNextHopGroupGetXpsId(saiNhGrpId, &xpsNhGrpId);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiNextHopGroupGetXpsId() failed with saiStatus: %d\n",
                       saiStatus);
        return saiStatus;
    }

    switch (attr->id)
    {
        case SAI_NEXT_HOP_GROUP_ATTR_NEXT_HOP_COUNT:
            {
                saiStatus = xpSaiGetNextHopGroupAttrNextHopCount(xpsNhGrpId, &attr->value);
                if (saiStatus != SAI_STATUS_SUCCESS)
                {
                    XP_SAI_LOG_ERR("Failed to get (SAI_NEXT_HOP_GROUP_ATTR_NEXT_HOP_COUNT)\n");
                    return saiStatus;
                }
                break;
            }
        case SAI_NEXT_HOP_GROUP_ATTR_TYPE:
            {
                saiStatus = xpSaiGetNextHopGroupAttrType(xpsNhGrpId, &attr->value);
                if (saiStatus != SAI_STATUS_SUCCESS)
                {
                    XP_SAI_LOG_ERR("Failed to get (SAI_NEXT_HOP_GROUP_ATTR_TYPE)\n");
                    return saiStatus;
                }
                break;
            }
        case SAI_NEXT_HOP_GROUP_ATTR_NEXT_HOP_MEMBER_LIST:
            {
                saiStatus = xpSaiGetNextHopGroupAttrNextHopMemberList(xpsNhGrpId, &attr->value);
                if (saiStatus != SAI_STATUS_SUCCESS)
                {
                    XP_SAI_LOG_ERR("Failed to get (SAI_NEXT_HOP_GROUP_ATTR_NEXT_HOP_MEMBER_LIST)\n");
                    return saiStatus;
                }
                break;
            }
        default:
            {
                XP_SAI_LOG_ERR("Failed to get %d\n", attr->id);
                return SAI_STATUS_INVALID_PARAMETER;
            }
    }

    return SAI_STATUS_SUCCESS;
}

static sai_status_t xpSaiGetNextHopGroupAttributes(sai_object_id_t
                                                   next_hop_group_id, uint32_t attr_count, sai_attribute_t *attr_list)
{
    uint32_t count;
    sai_status_t saiStatus;

    saiStatus = xpSaiAttrCheck(attr_count, attr_list,
                               NEXT_HOP_GROUP_VALIDATION_ARRAY_SIZE, next_hop_group_attribs,
                               SAI_COMMON_API_GET);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Attribute check failed with error %d\n", saiStatus);
        return saiStatus;
    }

    for (count = 0; count < attr_count; count++)
    {
        saiStatus = xpSaiGetNextHopGroupAttribute(next_hop_group_id, &attr_list[count],
                                                  count);
        if (saiStatus != SAI_STATUS_SUCCESS)
        {
            return saiStatus;
        }
    }

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiBulkGetNextHopGroupAttributes

sai_status_t xpSaiBulkGetNextHopGroupAttributes(sai_object_id_t id,
                                                uint32_t *attr_count, sai_attribute_t *attr_list)
{
    sai_status_t     saiRetVal  = SAI_STATUS_SUCCESS;
    uint32_t         idx        = 0;
    uint32_t         maxcount   = 0;

    XP_SAI_LOG_DBG("Calling %s \n", __FUNCNAME__);

    /* Check incoming parameters */
    if (!XDK_SAI_OBJID_TYPE_CHECK(id, SAI_OBJECT_TYPE_NEXT_HOP_GROUP))
    {
        XP_SAI_LOG_ERR("Wrong object type received(%u).", xpSaiObjIdTypeGet(id));
        return SAI_STATUS_INVALID_PARAMETER;
    }

    if ((attr_count == NULL) || (attr_list == NULL))
    {
        XP_SAI_LOG_ERR("Invalid parameter have been passed!\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    saiRetVal = xpSaiMaxCountNextHopGroupAttribute(&maxcount);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Could not get max attribute count!\n");
        return SAI_STATUS_FAILURE;
    }

    if (*attr_count < maxcount)
    {
        *attr_count = maxcount;
        return SAI_STATUS_BUFFER_OVERFLOW;
    }

    for (uint32_t count = 0; count < maxcount; count++)
    {
        attr_list[idx].id = SAI_NEXT_HOP_GROUP_ATTR_START + count;
        saiRetVal = xpSaiGetNextHopGroupAttribute(id, &attr_list[idx], count);

        if (saiRetVal == SAI_STATUS_SUCCESS)
        {
            idx++;
        }
    }
    *attr_count = idx;

    return SAI_STATUS_SUCCESS;
}

static sai_status_t xpSaiCreateNextHopGroupMembers(sai_object_id_t switch_id,
                                                   uint32_t object_count, const uint32_t *attr_count,
                                                   const sai_attribute_t **attr_list,
                                                   sai_bulk_op_error_mode_t mode, sai_object_id_t *object_id,
                                                   sai_status_t *object_statuses)
{
    sai_status_t status = SAI_STATUS_SUCCESS;
    uint32_t count = 0;
    uint32_t idx   = 0;

    if ((attr_count == NULL) || (attr_count == NULL) ||
        (object_id == NULL) || (object_statuses == NULL))
    {
        XP_SAI_LOG_ERR("Null pointer passed as a parameter!\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    if (!object_count)
    {
        XP_SAI_LOG_ERR("Zero object_count passed!\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    if (mode != SAI_BULK_OP_ERROR_MODE_STOP_ON_ERROR &&
        mode != SAI_BULK_OP_ERROR_MODE_IGNORE_ERROR)
    {
        XP_SAI_LOG_ERR("Invalid Bulk error mode\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    for (count = 0; count < object_count; count++)
    {
        object_statuses[count] = xpSaiCreateNextHopGroupMember(&(object_id[count]),
                                                               switch_id,
                                                               attr_count[count], attr_list[count]);
        if (object_statuses[count] != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("NH member Add Faild for index %d\n", count);
            status = SAI_STATUS_FAILURE;

            if (mode == SAI_BULK_OP_ERROR_MODE_STOP_ON_ERROR)
            {
                for (idx = (count + 1); idx < object_count; idx++)
                {
                    object_statuses[idx] = SAI_STATUS_NOT_EXECUTED;
                }
                break;
            }
        }
    }

    return status;
}

static sai_status_t xpSaiCreateNextHopGroupMember(sai_object_id_t*
                                                  next_hop_group_member_id,
                                                  sai_object_id_t switch_id, uint32_t attr_count,
                                                  const sai_attribute_t *attr_list)
{
    sai_status_t                       saiStatus;
    xpSaiNextHopGroupMemberAttributesT attributes;
    sai_object_id_t nhId = 0, nhGroupId = 0;
    uint32_t weight = 0;
    xpsDevice_t  xpsDevId = xpSaiGetDevId();
    xpSaiTableEntryCountDbEntry *entryCountCtxPtr = NULL;

#if 0
    xpSaiNextHopGroupMemberInfo_t memberInfo;
    memset(&memberInfo, 0, sizeof(memberInfo));
#endif
    memset(&attributes, 0, sizeof(attributes));
    saiStatus = xpSaiAttrCheck(attr_count, attr_list,
                               NEXT_HOP_GROUP_MEMBER_VALIDATION_ARRAY_SIZE, next_hop_group_member_attribs,
                               SAI_COMMON_API_CREATE);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Attribute check failed with error %d\n", saiStatus);
        return saiStatus;
    }

    /* Initialize the attribute values */
    xpSaiSetDefaultNextHopGroupMemberAttributeVals(&attributes);
    xpSaiUpdateNextHopGroupMemberAttributeVals(attr_count, attr_list, &attributes);

    nhId = attributes.nextHopId.oid;
    nhGroupId = attributes.nextHopGroupId.oid;
    weight = attributes.weight.u32;

    /* Check incoming parameters */
    if (!XDK_SAI_OBJID_TYPE_CHECK(nhId, SAI_OBJECT_TYPE_NEXT_HOP))
    {
        XP_SAI_LOG_ERR("Wrong object type received(%u).", xpSaiObjIdTypeGet(nhId));
        return SAI_STATUS_INVALID_ATTR_VALUE_0;
    }

    /*
     * XXX: Though it is theoretically possible to create a NH group member with an
     * NH id which is already present in another NH group member which present
     * in the group, such configuration does make NO sense. Only difference
     * between old NH group member and this newly added NH group member could be the weight.
     * So, weight attribute should be set instead of creating another member with new weight.
     *
     * Not supporting such configuration to avert the complexity owing to no possible use-case.
     * SONIC-1059: Seen a scenario where SONIC is sending redundant member add for the same NHId.
     * Adding a Work around temporarily to avoid orchagent crash.
     */
    if (xpSaiNextHopGroupContainsNextHop(nhGroupId, nhId) == true)
    {
        XP_SAI_LOG_ERR("Nexthop group (%" PRId64 ") already has NH id (%" PRId64
                       "). WA now !!!\n", nhGroupId, nhId);
        xpSaiNextHopGroupMemberCreateId(nhGroupId, nhId, next_hop_group_member_id);
        return SAI_STATUS_SUCCESS;
    }

    /* Program to XPS */
    saiStatus = xpSaiNextHopGroupAddXpsNextHop(nhGroupId, nhId, weight);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiNextHopGroupAddXpsNextHop() failed with saiStatus: %d\n",
                       saiStatus);
        return saiStatus;
    }

    /* Create an ID for this NH group member */
    xpSaiNextHopGroupMemberCreateId(nhGroupId, nhId, next_hop_group_member_id);

    saiStatus = xpSaiGetTableEntryCountCtxDb(xpsDevId, &entryCountCtxPtr);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to get Table Entry Count DB, saiStatus %d\n", saiStatus);
        return saiStatus;
    }
    entryCountCtxPtr->nextHopGroupMemberEntries++;

    return SAI_STATUS_SUCCESS;
}

static sai_status_t xpSaiRemoveNextHopGroupMembers(uint32_t object_count,
                                                   const sai_object_id_t *object_id, sai_bulk_op_error_mode_t mode,
                                                   sai_status_t *object_statuses)
{
    sai_status_t status = SAI_STATUS_SUCCESS;
    uint32_t count = 0;
    uint32_t idx   = 0;

    if ((object_id == NULL) || (object_statuses == NULL))
    {
        XP_SAI_LOG_ERR("Null pointer passed as a parameter!\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    if (!object_count)
    {
        XP_SAI_LOG_ERR("Zero object_count passed!\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    if (mode != SAI_BULK_OP_ERROR_MODE_STOP_ON_ERROR &&
        mode != SAI_BULK_OP_ERROR_MODE_IGNORE_ERROR)
    {
        XP_SAI_LOG_ERR("Invalid Bulk error mode\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    for (count = 0; count < object_count; count++)
    {
        object_statuses[count] = xpSaiRemoveNextHopGroupMember(object_id[count]);
        if (object_statuses[count] != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("NH member Del failed for index %d\n", count);
            status = SAI_STATUS_FAILURE;

            if (mode == SAI_BULK_OP_ERROR_MODE_STOP_ON_ERROR)
            {
                for (idx = (count + 1); idx < object_count; idx++)
                {
                    object_statuses[idx] = SAI_STATUS_NOT_EXECUTED;
                }
                break;
            }
        }
    }

    return status;
}

static sai_status_t xpSaiRemoveNextHopGroupMember(sai_object_id_t
                                                  next_hop_group_member_id)
{
    sai_status_t saiStatus;
    sai_object_id_t nhGroupId;
    sai_object_id_t nhId;
    xpSaiNextHopGroupMemberInfo_t memberInfo;
    xpsDevice_t  xpsDevId = xpSaiGetDevId();
    xpSaiTableEntryCountDbEntry *entryCountCtxPtr = NULL;


    /* Check incoming parameters */
    if (!XDK_SAI_OBJID_TYPE_CHECK(next_hop_group_member_id,
                                  SAI_OBJECT_TYPE_NEXT_HOP_GROUP_MEMBER))
    {
        XP_SAI_LOG_ERR("Wrong object type received(%u).",
                       xpSaiObjIdTypeGet(next_hop_group_member_id));
        return SAI_STATUS_INVALID_OBJECT_TYPE;
    }

    memset(&memberInfo, 0, sizeof(memberInfo));

    saiStatus = xpSaiNextHopGroupGetMemberInfo(next_hop_group_member_id,
                                               &memberInfo);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to get NH group member info for NH group member id (%"
                       PRId64 ")\n", next_hop_group_member_id);
        return saiStatus;
    }
    nhGroupId = memberInfo.nhGrpId;
    nhId = memberInfo.nhId;

    /* Remove it from XPS */
    saiStatus = xpSaiNextHopGroupRemoveXpsNextHop(nhGroupId, nhId);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to remove NH id (%" PRId64 ") from NH group (%" PRId64
                       ")\n", nhId, nhGroupId);
        return saiStatus;
    }

    saiStatus = xpSaiGetTableEntryCountCtxDb(xpsDevId, &entryCountCtxPtr);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to get Table Entry Count DB, saiStatus %d\n", saiStatus);
        return saiStatus;
    }
    entryCountCtxPtr->nextHopGroupMemberEntries--;

    return SAI_STATUS_SUCCESS;
}

static sai_status_t xpSaiSetNextHopGroupMemberAttribute(
    sai_object_id_t next_hop_group_member_id, const sai_attribute_t *attr)
{
    sai_status_t saiStatus;
    xpSaiNextHopGroupMemberInfo_t memberInfo;

    memset(&memberInfo, 0, sizeof(memberInfo));

    saiStatus = xpSaiAttrCheck(1, attr,
                               NEXT_HOP_GROUP_MEMBER_VALIDATION_ARRAY_SIZE, next_hop_group_member_attribs,
                               SAI_COMMON_API_SET);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Attribute check failed with error %d\n", saiStatus);
        return saiStatus;
    }

    saiStatus = xpSaiNextHopGroupGetMemberInfo(next_hop_group_member_id,
                                               &memberInfo);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to get NH member info for member id (%" PRId64 ")\n",
                       next_hop_group_member_id);
        return saiStatus;
    }

    switch (attr->id)
    {
        case SAI_NEXT_HOP_GROUP_MEMBER_ATTR_WEIGHT:
            {
                saiStatus = xpSaiNextHopGroupMemberSetAttributeWeight(memberInfo.nhGrpId,
                                                                      memberInfo.nhId, attr->value.u32);
                if (saiStatus != SAI_STATUS_SUCCESS)
                {
                    return saiStatus;
                }
                break;
            }
        default:
            {
                XP_SAI_LOG_ERR("Invalid attribute (%d) set\n", attr->id);
                return SAI_STATUS_INVALID_ATTRIBUTE_0;
            }
    }

    return SAI_STATUS_SUCCESS;
}

static sai_status_t xpSaiGetNextHopGroupMemberAttributes(
    sai_object_id_t next_hop_group_member_id, uint32_t attr_count,
    sai_attribute_t *attr_list)
{
    sai_status_t saiStatus;
    uint32_t count;
    xpSaiNextHopGroupMemberInfo_t memberInfo;
    uint32_t weight;

    saiStatus = xpSaiAttrCheck(attr_count, attr_list,
                               NEXT_HOP_GROUP_MEMBER_VALIDATION_ARRAY_SIZE, next_hop_group_member_attribs,
                               SAI_COMMON_API_GET);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Attribute check failed with error %d\n", saiStatus);
        return saiStatus;
    }

    memset(&memberInfo, 0, sizeof(memberInfo));

    /* Get the MH group member info */
    saiStatus = xpSaiNextHopGroupGetMemberInfo(next_hop_group_member_id,
                                               &memberInfo);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to get nexthop group member info for member id (%" PRId64
                       ")\n", next_hop_group_member_id);
        return saiStatus;
    }

    for (count = 0; count < attr_count; count++)
    {
        switch (attr_list[count].id)
        {
            case SAI_NEXT_HOP_GROUP_MEMBER_ATTR_NEXT_HOP_GROUP_ID:
                {
                    attr_list[count].value.oid = memberInfo.nhGrpId;
                    break;
                }
            case SAI_NEXT_HOP_GROUP_MEMBER_ATTR_NEXT_HOP_ID:
                {
                    attr_list[count].value.oid = memberInfo.nhId;
                    break;
                }
            case SAI_NEXT_HOP_GROUP_MEMBER_ATTR_WEIGHT:
                {
                    saiStatus = xpSaiNextHopGroupMemberGetAttributeWeight(memberInfo.nhGrpId,
                                                                          memberInfo.nhId, &weight);
                    if (saiStatus != SAI_STATUS_SUCCESS)
                    {
                        XP_SAI_LOG_ERR("Failed to get NH group member (%" PRId64 ") weight for NH id (%"
                                       PRId64 ") in NH group id (%" PRId64 ")\n", next_hop_group_member_id,
                                       memberInfo.nhGrpId, memberInfo.nhId);
                        /* Continue for next attributes */
                    }
                    attr_list[count].value.u32 = weight;
                    break;
                }
            default:
                {
                    return SAI_STATUS_INVALID_ATTRIBUTE_0;
                }
        }
    }

    return SAI_STATUS_SUCCESS;
}

/********************************************************************
 *                      Init/Deinit functions                       *
 *******************************************************************/
XP_STATUS xpSaiNextHopGroupApiInit(uint64_t flag,
                                   const sai_service_method_table_t* adapHostServiceMethodTable)
{
    XP_STATUS retVal = XP_NO_ERR;
    sai_status_t saiRetVal = SAI_STATUS_SUCCESS;

    XP_SAI_LOG_DBG("Calling xpSaiNextHopGroupApiInit\n");

    _xpSaiNextHopGroupApi = (sai_next_hop_group_api_t *) xpMalloc(sizeof(
                                                                      sai_next_hop_group_api_t));
    if (NULL == _xpSaiNextHopGroupApi)
    {
        XP_SAI_LOG_ERR("Error: allocation failed for _xpSaiNextHopGroupApi\n");
        return XP_ERR_MEM_ALLOC_ERROR;
    }

    _xpSaiNextHopGroupApi->create_next_hop_group = xpSaiCreateNextHopGroup;
    _xpSaiNextHopGroupApi->remove_next_hop_group = xpSaiRemoveNextHopGroup;
    _xpSaiNextHopGroupApi->set_next_hop_group_attribute =
        xpSaiSetNextHopGroupAttribute;
    _xpSaiNextHopGroupApi->get_next_hop_group_attribute =
        xpSaiGetNextHopGroupAttributes;
    _xpSaiNextHopGroupApi->create_next_hop_group_member =
        xpSaiCreateNextHopGroupMember;
    _xpSaiNextHopGroupApi->remove_next_hop_group_member =
        xpSaiRemoveNextHopGroupMember;
    _xpSaiNextHopGroupApi->set_next_hop_group_member_attribute =
        xpSaiSetNextHopGroupMemberAttribute;
    _xpSaiNextHopGroupApi->get_next_hop_group_member_attribute =
        xpSaiGetNextHopGroupMemberAttributes;

    _xpSaiNextHopGroupApi->create_next_hop_group_members =
        xpSaiCreateNextHopGroupMembers;
    _xpSaiNextHopGroupApi->remove_next_hop_group_members =
        xpSaiRemoveNextHopGroupMembers;

    saiRetVal = xpSaiApiRegister(SAI_API_NEXT_HOP_GROUP,
                                 (void*)_xpSaiNextHopGroupApi);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Error: Failed to register next hop group API\n");
        return XP_ERR_ARRAY_OUT_OF_BOUNDS;
    }

    return retVal;
}

XP_STATUS xpSaiNextHopGroupApiDeinit()
{
    XP_STATUS retVal = XP_NO_ERR;

    XP_SAI_LOG_DBG("Calling xpSaiNextHopGroupApiDeinit\n");

    xpFree(_xpSaiNextHopGroupApi);
    _xpSaiNextHopGroupApi = NULL;

    return  retVal;
}

XP_STATUS xpSaiNextHopGroupInit(xpsDevice_t xpSaiDevId)
{
    XP_STATUS xpStatus;

    gSaiNextHopGroupInfoDbHndl = XPSAI_NH_GRP_DB_HNDL;

    xpStatus = xpsStateRegisterDb(XP_SCOPE_DEFAULT, "XP SAI NH GRP DB", XPS_GLOBAL,
                                  xpSaiNextHopGroupInfoKeyCompare, gSaiNextHopGroupInfoDbHndl);
    if (xpStatus != XP_NO_ERR)
    {
        gSaiNextHopGroupInfoDbHndl = XPS_STATE_INVALID_DB_HANDLE;
        XP_SAI_LOG_ERR("Failed to register DB for Nexthop group\n");
        return xpStatus;
    }

    return XP_NO_ERR;
}

XP_STATUS xpSaiNextHopGroupDeInit(xpsDevice_t xpSaiDevId)
{
    XP_STATUS xpStatus;

    xpStatus = xpsStateDeRegisterDb(XP_SCOPE_DEFAULT, &gSaiNextHopGroupInfoDbHndl);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to deregister DB for Nexthop group\n");
        return xpStatus;
    }

    gSaiNextHopGroupInfoDbHndl = XPS_STATE_INVALID_DB_HANDLE;

    return XP_NO_ERR;
}

sai_status_t xpSaiMaxCountNextHopGroupAttribute(uint32_t *count)
{
    if (!count)
    {
        XP_SAI_LOG_ERR("Invalid parameter have been passed!\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }
    *count = SAI_NEXT_HOP_GROUP_ATTR_END;

    return SAI_STATUS_SUCCESS;
}

sai_status_t xpSaiCountNextHopGroupObjects(uint32_t *count)
{
    XP_STATUS  retVal  = XP_NO_ERR;

    if (!count)
    {
        XP_SAI_LOG_ERR("Invalid parameter have been passed!\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    retVal = xpsCountNextHopGroupObjects(count);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to get Count. return Value : %d\n", retVal);
    }
    return xpsStatus2SaiStatus(retVal);
}

sai_status_t xpSaiGetNextHopGroupObjectList(uint32_t *object_count,
                                            sai_object_key_t *object_list)
{
    XP_STATUS       retVal      = XP_NO_ERR;
    sai_status_t    saiRetVal   = SAI_STATUS_SUCCESS;
    uint32_t        objCount    = 0;
    uint32_t        *nhgrpId;
    xpsDevice_t     devId       = xpSaiGetDevId();


    XP_SAI_LOG_DBG("Calling %s \n", __FUNCNAME__);

    saiRetVal = xpSaiCountNextHopGroupObjects(&objCount);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to get object count!\n");
        return saiRetVal;
    }

    if (objCount > *object_count)
    {
        *object_count = objCount;
        XP_SAI_LOG_ERR("Buffer overflow occured\n");
        return SAI_STATUS_BUFFER_OVERFLOW;
    }

    if (object_list == NULL)
    {
        *object_count = objCount;
        XP_SAI_LOG_ERR("Invalid parameter have been passed!\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    nhgrpId = (uint32_t *)xpMalloc(sizeof(uint32_t) * (*object_count));
    if (nhgrpId == NULL)
    {
        XP_SAI_LOG_ERR("Could not allocate memory for nhgrpId array!\n");
        return SAI_STATUS_NO_MEMORY;
    }

    retVal = xpsGetNhGrpIdList(XP_SCOPE_DEFAULT, nhgrpId);
    if (retVal != XP_NO_ERR)
    {
        xpFree(nhgrpId);
        return xpsStatus2SaiStatus(retVal);
    }

    *object_count = objCount;

    for (uint32_t i = 0; i < *object_count; i++)
    {
        saiRetVal = xpSaiObjIdCreate(SAI_OBJECT_TYPE_NEXT_HOP_GROUP, devId,
                                     (sai_uint64_t)nhgrpId[i], &object_list[i].key.object_id);
        if (saiRetVal != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("SAI objectId could not be created, error %d\n", saiRetVal);
            xpFree(nhgrpId);
            return saiRetVal;
        }
    }

    xpFree(nhgrpId);
    return SAI_STATUS_SUCCESS;
}
