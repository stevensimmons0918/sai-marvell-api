// xpSaiPort.h

/*******************************************************************************
* copyright (c) 2021 marvell. all rights reserved. the following file is       *
* subject to the limited use license agreement by and between marvell and you, *
* your employer or other entity on behalf of whom you act. in the absence of   *
* such license agreement the following file is subject to marvell’s standard   *
* limited use license agreement.                                               *
********************************************************************************/

#ifndef _xpSaiPort_h_
#define _xpSaiPort_h_

#include "xpSai.h"
#include "xpSaiBuffer.h"
#include "xpSaiPortCfgManager.h"
#include "xpSaiLag.h"

#ifdef __cplusplus
extern "C" {
#endif

#define XPSAI_DIRECTED_MIRROR_SESSION_NUM 1

#define XPSAI_PORT_DEFAULT_MTU      1514
#define XPSAI_PORT_MAX_QUEUES       32  /* falcon supports upto 16 queues per port + 16 queues for SNMP testcase WA */

#define XPS_MAX_QUEUES_PER_PORT     8

#define XPS_MAX_QUEUE_BIT_MAP       XPSAI_PORT_MAX_QUEUES/SIZEOF_BYTE

typedef void (*saiLinkAddEventHandler_t)(xpsDevice_t devId, uint32_t portNum);
struct xpSaiPortLagInfo_t;

typedef struct xpSaiPortQosInfo_t
{
    sai_object_id_t portId;
    sai_object_id_t schedObjId;
    sai_object_id_t policerId;
    sai_object_id_t
    floodScPolicerObjId;   // flood (unknown unicast or unknown multicast) storm control policer
    sai_object_id_t bcastScPolicerObjId;   // broadcast storm control policer
    sai_object_id_t mcastScPolicerObjId;   // multicast storm control policer
    sai_object_id_t dot1pToTcObjId;
    sai_object_id_t dot1pToDpObjId;
    sai_object_id_t dscpToTcObjId;
    sai_object_id_t dscpToDpObjId;
    sai_object_id_t tcToQueueObjId;
    sai_object_id_t tcDpToDot1pObjId;
    sai_object_id_t tcDpToDscpObjId;
    sai_object_id_t tcToPgObjId;
    sai_object_id_t pfcPrioToPgObjId;
    sai_object_id_t pfcPriToQueueObjId;
    sai_object_id_t ingBufProfileObjId;
    sai_object_id_t egrBufProfileObjId;
    sai_uint32_t
    numOfQueues;                                            // Number of queues created on a particular port
    sai_uint8_t
    qListBitMap[XPS_MAX_QUEUE_BIT_MAP];                     // List of queues created on a particular port
    sai_uint32_t
    numOfSchedGrps;                                         // Number of scheduler groups created on a particular port
    sai_uint32_t
    schedGrpList[XPSAI_SG_MAX_GROUPS_PER_PORT];             // List of scheduler groups created on a particular port
    sai_uint32_t
    numOfIngPrioGrps;                                       // Max Number of Buffer Ingress Priority Groups
    sai_object_id_t
    ingPrioGrps[XPSAI_INGRESS_PRIORITY_GROUP_INDEX_MAX];    // List of Buffer Ingress Priority Groups
} xpSaiPortQosInfo_t;

typedef enum
{
    pfcVectorUpdateTypeRx,
    pfcVectorUpdateTypeTx,
    pfcVectorUpdateTypeCombined
} pfcVectorUpdateType;


typedef struct _xpSaiPortDbEntryT
{
    xpsInterfaceId_t keyIntfId;
    sai_uint8_t pfc;
    bool combinedMode;
    sai_uint8_t pfcTx;
    sai_uint8_t pfcRx;
    bool fullDuplexMode;
    sai_port_media_type_t mediaType;
    sai_object_id_t ingressAclId;
    uint64_t egressAclId;
    sai_object_id_t ingressMirrorSessionOid[XPSAI_DIRECTED_MIRROR_SESSION_NUM];
    sai_object_id_t egressMirrorSessionOid[XPSAI_DIRECTED_MIRROR_SESSION_NUM];
    sai_object_id_t ingressSampleSessionOid;
    sai_object_id_t egressSampleSessionOid;
    bool txEnable;
    sai_uint16_t pvidUserSetting;
    xpsPortList_t blockedPortList;
} xpSaiPortDbEntryT;

typedef struct _xpSaiPortStatisticDbEntryT
{
    // key
    sai_int32_t portNum;

    // metadata, for CNC counter v4/v6
    uint32_t counterId_Ingress_v4;
    uint32_t counterId_Ingress_v6;
    uint32_t counterId_Egress_v4;
    uint32_t counterId_Egress_v6;

    // port statistics data
    sai_uint64_t rxOctets;
    sai_uint64_t rxUnicast;
    sai_uint64_t rxNonUnicast;
    sai_uint64_t rxErrOrDiscard;
    sai_uint64_t rxBroadcast;
    sai_uint64_t rxMulticast;
    sai_uint64_t rxUnknownProtocol;
    sai_uint64_t txOctets;
    sai_uint64_t txUnicast;
    sai_uint64_t txNonUnicast;
    sai_uint64_t txErrOrDiscard;
    sai_uint64_t txBroadcast;
    sai_uint64_t txMulticast;
    sai_uint64_t fragments;
    sai_uint64_t rxOversize;
    sai_uint64_t TxOversize;
    sai_uint64_t txNoError;
    sai_uint64_t rxNoError;
    sai_uint64_t crcAlignErr;
    sai_uint64_t statOversize;
    sai_uint64_t statUnderSize;
    sai_uint64_t statJabbers;
    sai_uint64_t rx64Octets;
    sai_uint64_t rx65To127Octets;
    sai_uint64_t rx128To255Octets;
    sai_uint64_t rx256To511Octets;
    sai_uint64_t rx512To1023Octets;
    sai_uint64_t rx1024To1518Octets;
    sai_uint64_t rx1519To2047Octets;
    sai_uint64_t rx2048To4095Octets;
    sai_uint64_t rx4096To9216Octets;
    sai_uint64_t rx9217To16383Octets;
    sai_uint64_t tx64Octets;
    sai_uint64_t tx65To127Octets;
    sai_uint64_t tx128To255Octets;
    sai_uint64_t tx256To511Octets;
    sai_uint64_t tx512To1023Octets;
    sai_uint64_t tx1024To1518Octets;
    sai_uint64_t tx1519To2047Octets;
    sai_uint64_t tx2048To4095Octets;
    sai_uint64_t tx4096To9216Octets;
    sai_uint64_t tx9217To16383Octets;
    sai_uint64_t rxPause;
    sai_uint64_t txPause;
    sai_uint64_t rxPfc0;
    sai_uint64_t txPfc0;
    sai_uint64_t rxPfc1;
    sai_uint64_t txPfc1;
    sai_uint64_t rxPfc2;
    sai_uint64_t txPfc2;
    sai_uint64_t rxPfc3;
    sai_uint64_t txPfc3;
    sai_uint64_t rxPfc4;
    sai_uint64_t txPfc4;
    sai_uint64_t rxPfc5;
    sai_uint64_t txPfc5;
    sai_uint64_t rxPfc6;
    sai_uint64_t txPfc6;
    sai_uint64_t rxPfc7;
    sai_uint64_t txPfc7;
    sai_uint64_t wredDropPkts;
    sai_uint64_t wredDropBytes;
    sai_uint64_t txV4Pkts;
    sai_uint64_t txV4Octets;
    sai_uint64_t rxV4Pkts;
    sai_uint64_t rxV4Octets;
    sai_uint64_t txV6Pkts;
    sai_uint64_t txV6Octets;
    sai_uint64_t rxV6Pkts;
    sai_uint64_t rxV6Octets;
} xpSaiPortStatisticDbEntryT;

XP_STATUS xpSaiPortApiInit(uint64_t flag,
                           const sai_service_method_table_t* adapHostServiceMethodTable);
XP_STATUS xpSaiPortApiDeinit();
XP_STATUS xpSaiPortInit(xpsDevice_t xpSaiDevId);
XP_STATUS xpSaiPortDeInit(xpsDevice_t xpSaiDevId);
XP_STATUS xpSaiPortAddQueueToPortQList(sai_object_id_t portObjId,
                                       sai_uint8_t queueNum);
XP_STATUS xpSaiPortRemoveQueueFromPortQList(sai_object_id_t portObjId,
                                            sai_uint8_t queueNum);
sai_status_t xpSaiConvertPortOid(sai_object_id_t port_id,
                                 xpsInterfaceId_t* pXpsIntf);
sai_status_t xpSaiPortSetSchedulerInfo(sai_object_id_t portId,
                                       sai_object_id_t schedulerId);
sai_status_t xpSaiPortLagInfoGet(xpsInterfaceId_t intfId,
                                 xpSaiPortLagInfo_t **ppSaiPortLagInfoEntry);
sai_status_t xpSaiMaxCountPortAttribute(uint32_t *count);
sai_status_t xpSaiCountPortObjects(uint32_t *count);
sai_status_t xpSaiGetPortObjectList(uint32_t *object_count,
                                    sai_object_key_t *object_list);
sai_status_t xpSaiBulkGetPortAttributes(sai_object_id_t id,
                                        uint32_t *attr_count, sai_attribute_t *attr_list);
sai_status_t xpSaiPortDbInsert(xpsInterfaceId_t xpsIntf);
sai_status_t xpSaiPortDbInfoGet(xpsInterfaceId_t xpsIntf,
                                xpSaiPortDbEntryT **ppPortEntry);
sai_status_t xpSaiSetPortAttribute(sai_object_id_t port_id,
                                   const sai_attribute_t *attr);
sai_status_t xpSaiCreatePort(sai_object_id_t *port_id,
                             sai_object_id_t switch_id, uint32_t attr_count,
                             const sai_attribute_t *attr_list);
sai_status_t xpSaiRemovePort(sai_object_id_t port_id);
sai_status_t xpSaiSetPortAttrIngressSampleSessionOid(xpsInterfaceId_t xpsIntf,
                                                     sai_object_id_t sampleSessionOid);
sai_status_t xpSaiGetPortAttrIngressSampleSessionOid(xpsInterfaceId_t xpsIntf,
                                                     sai_object_id_t *sampleSessionOid);
sai_status_t xpSaiSetPortAttrEgressSampleSessionOid(xpsInterfaceId_t xpsIntf,
                                                    sai_object_id_t sampleSessionOid);
sai_status_t xpSaiGetPortAttrEgressSampleSessionOid(xpsInterfaceId_t xpsIntf,
                                                    sai_object_id_t *sampleSessionOid);
sai_status_t xpSaiGetPortType(xpsDevice_t xpsDevId, xpsInterfaceId_t xpsIntf,
                              sai_attribute_value_t *value);
sai_status_t xpSaiPortStatisticsStoreDB(xpsDevice_t devId, uint32_t portNum);
XP_STATUS xpSaiGetPortStatisticInfo(sai_int32_t portNum,
                                    xpSaiPortStatisticDbEntryT **portStatistics);

sai_status_t xpSaiSetPortAttrDefaultVlan(sai_object_id_t portId,
                                         sai_attribute_value_t value);
sai_status_t xpSaiGetPortAttrDefaultVlan(sai_object_id_t portId,
                                         sai_attribute_value_t *value);
sai_status_t xpSaiSetPortAttrDefaultVlanPriority(sai_object_id_t portId,
                                                 sai_attribute_value_t value);
sai_status_t xpSaiGetPortAttrDefaultVlanPriority(sai_object_id_t portId,
                                                 sai_attribute_value_t* value);
sai_status_t xpSaiSetPortAttrDropUntagged(sai_object_id_t portId,
                                          sai_attribute_value_t value);
sai_status_t xpSaiGetPortAttrDropUntagged(sai_object_id_t portId,
                                          sai_attribute_value_t *value);
sai_status_t xpSaiSetPortAttrDropTagged(sai_object_id_t portId,
                                        sai_attribute_value_t value);
sai_status_t xpSaiGetPortAttrDropTagged(sai_object_id_t portId,
                                        sai_attribute_value_t *value);
sai_status_t xpSaiSetPortAttrFdbLearningMode(sai_object_id_t port_id,
                                             sai_attribute_value_t value);
sai_status_t xpSaiGetPortAttrFdbLearningMode(sai_object_id_t port_id,
                                             sai_attribute_value_t* value, uint32_t attr_index);
sai_status_t xpSaiSetPortLagConfig(sai_object_id_t portId,
                                   xpSaiPortLagInfo_t lagConfig);
sai_status_t xpSaiGetPortLagConfig(sai_object_id_t portId,
                                   xpSaiPortLagInfo_t *lagConfig);
sai_status_t xpSaiStoreCurrentPortConfigDB(sai_object_id_t portId,
                                           xpSaiPortLagInfo_t lagConfig);
sai_status_t xpSaiRestorePortConfigDB(sai_object_id_t portId);

/* Utilities */
/* Get xpsInterface Id from SAI port obj id */
sai_status_t xpSaiPortGetXpsIntfId(sai_object_id_t port,
                                   xpsInterfaceId_t *intfId);
XP_STATUS xpSaiGetLanesByPort(xpsDevice_t devId, uint32_t portNum,
                              sai_u32_list_t *laneList);

//Func: xpSaiInsertPortQosDbEntry
XP_STATUS xpSaiInsertPortQosDbEntry(sai_object_id_t port_id);
//Func: xpSaiGetPortQosInfo
XP_STATUS xpSaiGetPortQosInfo(sai_object_id_t port_id,
                              xpSaiPortQosInfo_t **ppPortQosInfo);
sai_status_t xpSaiGetPortAttrQosNumberOfQueues(sai_object_id_t port_id,
                                               sai_attribute_value_t* value);
sai_status_t xpSaiGetPortAttrQosQueueList(sai_object_id_t port_id,
                                          sai_attribute_value_t* value);
XP_STATUS xpSaiPortBindSchedulerGroupToPort(sai_object_id_t portObjId,
                                            sai_uint32_t schedGrpKey);
XP_STATUS xpSaiPortUnbindSchedulerGroupFromPort(sai_object_id_t portObjId,
                                                sai_uint32_t schedGrpKey);
sai_status_t xpSaiPortBindIngressPriorityGrpToPort(sai_object_id_t portObjId,
                                                   sai_object_id_t priorityGroupId);
sai_status_t xpSaiPortUnbindIngressPriorityGrpFromPort(sai_object_id_t
                                                       portObjId, sai_object_id_t priorityGroupId);
sai_status_t xpSaiPortDbBlockedPortListGet(xpsInterfaceId_t xpsIntf,
                                           xpsPortList_t *blockedPortList);
sai_status_t xpSaiPortDbBlockedPortListSet(xpsInterfaceId_t xpsIntf,
                                           xpsPortList_t blockedPortList);
xpSaiPortMgrInfo_t *xpSaiPortInfoGet(uint32_t portNum);

sai_status_t xpSaiSetPortAttrQosDefaultTc(sai_object_id_t port_id,
                                          sai_attribute_value_t value);
sai_status_t xpSaiSetPortAttrQosDot1pToTcMap(sai_object_id_t portObjId,
                                             sai_attribute_value_t value);
sai_status_t xpSaiSetPortAttrQosDot1pToColorMap(sai_object_id_t portObjId,
                                                sai_attribute_value_t value);
sai_status_t xpSaiSetPortAttrQosDscpToTcMap(sai_object_id_t portObjId,
                                            sai_attribute_value_t value);
sai_status_t xpSaiSetPortAttrQosDscpToColorMap(sai_object_id_t portObjId,
                                               sai_attribute_value_t value);
sai_status_t xpSaiSetPortAttrQosTcToQueueMap(sai_object_id_t portObjId,
                                             sai_attribute_value_t value);
sai_status_t xpSaiSetPortAttrQosTcAndColorToDot1pMap(sai_object_id_t portObjId,
                                                     sai_attribute_value_t value);
sai_status_t xpSaiSetPortAttrQosTcAndColorToDscpMap(sai_object_id_t portObjId,
                                                    sai_attribute_value_t value);
XP_STATUS xpSaiPortGetNumPfcPrioritiesSupported(xpsDevice_t devId,
                                                uint32_t *totalSupportedPrio);
sai_status_t xpSaiSetPortAttrQosIngressBufferProfileList(
    sai_object_id_t portObjId, sai_attribute_value_t value);
sai_status_t xpSaiSetPortAttrQosEgressBufferProfileList(
    sai_object_id_t portObjId, sai_attribute_value_t value, bool forceSet);

sai_status_t xpSaiPortApplyBufferProfileOnPort(sai_object_id_t portObjId,
                                               sai_object_id_t profileObjId, bool ingressList);

XP_STATUS xpSaiInsertPortStatisticDbEntry(sai_int32_t port_id);


void xpSaiPortCnpRateLimitParamSet(xpsDevice_t devId, uint32_t portNum,
                                   bool enabled);

#ifdef __cplusplus
}
#endif

#endif //_xpSaiPort_h_
