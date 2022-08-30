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
* @file gtAppIpFix.c
*
* @brief This file includes utilites for Host part of IpFix Demo.
*
* @version   1
********************************************************************************
*/
/***************************************/
/*  Init test Flow Manager IPFIX Demo  */
/***************************************/


#include <cpss/dxCh/dxChxGen/flowManager/prvCpssDxChFlowManagerDb.h>
#include <cpss/dxCh/dxChxGen/networkIf/cpssDxChNetIf.h>
#include <cpss/dxCh/dxChxGen/flowManager/cpssDxChFlowManager.h>
#include <cpss/dxCh/dxChxGen/pcl/cpssDxChPcl.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/pcl/cpssDxChPcl.h>
#include <cpss/dxCh/dxChxGen/ipfix/cpssDxChIpfix.h>
#include <cpss/dxCh/dxChxGen/networkIf/cpssDxChNetIf.h>
#include <cpss/dxCh/dxChxGen/virtualTcam/cpssDxChVirtualTcam.h>
#include <utfExtHelpers.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>



/* Max number of PPs in system  */
#define APP_DEMO_IPFIX_MAX_PP_DEVICES_CNS       128

/* max ports in device in Aldrin2  */
#define APP_DEMO_IPFIX_MAX_PORT_NUM             128

/* Number of key table entries */
#define APP_DEMO_IPFIX_KEY_TABLE_INDEX_CNS      16

/* Number of packet types */
#define APP_DEMO_IPFIX_MAX_PKT_TYPE_NUM         5

/* UDB key constant to state that the parameter with this value needs to be ignored */
#define APP_DEMO_IPFIX_NOT_USED_UDB             0xFF

/* Key table index constant to state that the parameter with this value needs to be ignored */
#define APP_DEMO_IPFIX_NOT_USED_KEY_TABLE_INDEX 0x10

/* Rule Index in vTcam for the flow-based default rule for IPv4-TCP flow */
#define APP_DEMO_IPFIX_IPV4_TCP_DEFAULT_RULE_ID (APP_DEMO_IPFIX_MAX_PORT_NUM + 0)

/* Rule Index in vTcam for the flow-based default rule for IPv4-UDP flow */
#define APP_DEMO_IPFIX_IPV4_UDP_DEFAULT_RULE_ID  (APP_DEMO_IPFIX_MAX_PORT_NUM + 1)

/* Rule Index in vTcam for the flow-based default rule for OTHER flow */
#define APP_DEMO_IPFIX_OTHER_DEFAULT_RULE_ID  (APP_DEMO_IPFIX_MAX_PORT_NUM + 2)

/* Rule Index in vTcam for the flow-based default rule for IPv6-TCP flow */
#define APP_DEMO_IPFIX_IPV6_TCP_DEFAULT_RULE_ID  (APP_DEMO_IPFIX_MAX_PORT_NUM + 0)

/* Rule Index in vTcam for the flow-based default rule for IPv6-UDP flow */
#define APP_DEMO_IPFIX_IPV6_UDP_DEFAULT_RULE_ID  (APP_DEMO_IPFIX_MAX_PORT_NUM + 1)

/* variables used as parammeters of appDemoIpfixInit function */
static GT_U32 appDemoIpfixPclId = 0x55;

/* Not valid flow manager id */
#define APP_DEMO_IPFIX_FLOW_MANAGER_ID_NOT_VALID 0xFFFFFFFF

/* Maximum flow manager id */
#define APP_DEMO_IPFIX_FLOW_MANAGER_MAX_ID_CNS 128

/* Tail Drop Profile for CPU Port */
#define APP_DEMO_IPFIX_FLOW_MANAGER_CPU_PORT_PROFILE_CNS CPSS_PORT_TX_SCHEDULER_PROFILE_5_E

/* SP arbitration group for CPU Port Message Packet Queue */
#define APP_DEMO_IPFIX_FLOW_MANAGER_CPU_PORT_MSG_PACKET_QUEUE_ARB_GROUP CPSS_PORT_TX_SP_ARB_GROUP_E

/* flow manager Identifier */
static GT_U32 appDemoIpfixFlowMngId = APP_DEMO_IPFIX_FLOW_MANAGER_ID_NOT_VALID;

static CPSS_PORTS_BMP_STC appDemoIpfixPortBitmap =
{{
    0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
#if CPSS_MAX_PORTS_NUM_CNS > 128
    0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
#if CPSS_MAX_PORTS_NUM_CNS > 256
    0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
    0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF
#endif
#endif
}};
static CPSS_PCL_LOOKUP_NUMBER_ENT appDemoIpfixLookupNum = CPSS_PCL_LOOKUP_NUMBER_0_E;
static CPSS_DXCH_PCL_RULE_FORMAT_TYPE_ENT appDemoIpfixKeyTypesNonIpKey =
    CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_30_E;
static CPSS_DXCH_PCL_RULE_FORMAT_TYPE_ENT appDemoIpfixKeyTypesIpv4Key  =
    CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_30_E;
static CPSS_DXCH_PCL_RULE_FORMAT_TYPE_ENT appDemoIpfixKeyTypesIpv6Key  =
    CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_60_E;
static GT_U32 appDemoIpfixFwAclVTcam = 0;
static GT_U32 appDemoIpfixShortAclVTcam = 1;
static GT_U32 appDemoIpfixLongAclVTcam = 2;
static GT_U32 appDemoIpfixFwRxQueue = 6; /* Rx Queue num for fw */
static GT_U32 appDemoIpfixFwTxQueue = 6; /* Tx Queue num for fw */
static GT_U32 appDemoIpfixMsgRxQueue = 7; /* Rx Queue num for msg sent from fw */
static GT_BOOL debug_print_enable = GT_FALSE; /* Flag to print detailed packet information */
static GT_U32 appDemoIpfixEnableBitmap = 0x00;
static GT_U32 appDemoIpfixKeyTableIndex = 0;
static GT_U32 appDemoIpfixUdbIndex = 0;
static GT_U32 appDemoIpfixOffset = 0;
static GT_U32 appDemoIpfixMask = 0;
static GT_U32 appDemoIpfixPortNum = 0;

static GT_U32 appDemoIpfixShortKeySize = CPSS_DXCH_FLOW_MANAGER_KEY_SIZE_30B_E;
static CPSS_PACKET_CMD_ENT appDemoIpfixDefaultRulePktCmd = CPSS_PACKET_CMD_MIRROR_TO_CPU_E;
static CPSS_NET_RX_CPU_CODE_ENT appDemoIpfixDefaultRuleCpuCode =
    (CPSS_NET_FIRST_USER_DEFINED_E + 2);
static CPSS_NET_RX_CPU_CODE_ENT appDemoIpfixMsgCpuCode=
     (CPSS_NET_FIRST_USER_DEFINED_E + 3);

static CPSS_DXCH_POLICER_STAGE_TYPE_ENT appDemoIpfixPolicerStage = CPSS_DXCH_POLICER_STAGE_INGRESS_0_E;
static CPSS_PCL_DIRECTION_ENT appDemoIpfixPclDirection = CPSS_PCL_DIRECTION_INGRESS_E;
static CPSS_DXCH_ETHERTYPE_TABLE_ENT appDemoIpfixTableType = CPSS_DXCH_ETHERTYPE_TABLE_INGRESS_E;
static CPSS_ETHER_MODE_ENT appDemoIpfixEthMode1 = CPSS_VLAN_ETHERTYPE0_E;
static CPSS_ETHER_MODE_ENT appDemoIpfixEthMode2 = CPSS_VLAN_ETHERTYPE1_E;
static GT_U32 appDemoIpfixTotalFlowsCount = 0;
static GT_U32 appDemoIpfixFwTcamNumOfShortRules = 2048;
static GT_U32 appDemoIpfixIpv4TcpKeyTableIndex = 0;
static GT_U32 appDemoIpfixIpv4UdpKeyTableIndex = 1;
static GT_U32 appDemoIpfixIpv6TcpKeyTableIndex = 2;
static GT_U32 appDemoIpfixIpv6UdpKeyTableIndex = 3;
static GT_U32 appDemoIpfixOtherKeyTableIndex = 4;
static GT_U32 appDemoIpfixPort = 0;

static CPSS_PORT_TX_SCHEDULER_PROFILE_SET_ENT  appDemoIpfixProfileGet;
static CPSS_PORT_TX_Q_ARB_GROUP_ENT appDemoIpfixArbGroupGet;

typedef enum{
    /** @brief IPv4-TCP
     */
    APP_DEMO_IPFIX_PACKET_TYPE_IPV4_TCP_E,

    /** @brief IPv4-UDP
     */
    APP_DEMO_IPFIX_PACKET_TYPE_IPV4_UDP_E,

    /** @brief IPv6-TCP
     */
    APP_DEMO_IPFIX_PACKET_TYPE_IPV6_TCP_E,

    /** @brief IPv6-UDP
     */
    APP_DEMO_IPFIX_PACKET_TYPE_IPV6_UDP_E,

    /** @brief OTHER
     */
    APP_DEMO_IPFIX_PACKET_TYPE_OTHER_E

}APP_DEMO_IPFIX_PACKET_TYPE_ENT;

typedef struct{

    /** @brief TCP/UDP Src Port
    */
    GT_U8 protoSrcPortIndex[2];

    /** @brief TCP/UDP Dst Port
    */
    GT_U8 protoDstPortIndex[2];

}PRV_APP_DEMO_IPFIX_PROTO_DETAILS;

typedef struct{

    /** @brief PCL-ID
     */
    GT_U8 pclIdIndex[2];

    /** @brief VLAN-ID
     */
    GT_U8 vlanIdIndex[2];

    /** @brief SourcePort
     */
    GT_U8 srcPortIndex;

    /** @brief Applicable Flow Sub-template
     */
    GT_U8 appFlowSubTempIndex;

    /** @brief SIPv4
     */
    GT_U8 sipv4Index[4];

    /** @brief DIPv4
     */
    GT_U8 dipv4Index[4];

    /** @brief packet type to identify tcp/udp
     */
    APP_DEMO_IPFIX_PACKET_TYPE_ENT pktType;

    /** @brief union of tcp and udp details
     */
    PRV_APP_DEMO_IPFIX_PROTO_DETAILS protocol;

    /** @brief not valid udbs
     */
    GT_U8 notValidIndex[30];

} PRV_APP_DEMO_IPFIX_IPV4_KEY_FIELDS_INDEX_STC;

typedef struct{

    /** @brief PCL-ID
     */
    GT_U8 pclIdIndex[2];

    /** @brief VLAN-ID
     */
    GT_U8 vlanIdIndex[2];

    /** @brief SourcePort
     */
    GT_U8 srcPortIndex;

    /** @brief Applicable Flow Sub-template
     */
    GT_U8 appFlowSubTempIndex;

    /** @brief SIPv4
     */
    GT_U8 sipv6Index[16];

    /** @brief DIPv4
     */
    GT_U8 dipv6Index[16];

    /** @brief packet type to identify tcp/udp
     */
    APP_DEMO_IPFIX_PACKET_TYPE_ENT pktType;

    /** @brief union of tcp and udp details
     */
    PRV_APP_DEMO_IPFIX_PROTO_DETAILS protocol;

    /** @brief not valid udbs
     */
    GT_U8 notValidIndex[60];

} PRV_APP_DEMO_IPFIX_IPV6_KEY_FIELDS_INDEX_STC;

typedef struct{

    /** @brief PCL-ID
     */
    GT_U8 pclIdIndex[2];

    /** @brief VLAN-ID
     */
    GT_U8 vlanIdIndex[2];

    /** @brief SourcePort
     */
    GT_U8 srcPortIndex;

    /** @brief Applicable Flow Sub-template
     */
    GT_U8 appFlowSubTempIndex;

    /** @brief SMAC
     */
    GT_U8 smacIndex[6];

    /** @brief DMAC
     */
    GT_U8 dmacIndex[6];

    /** @brief not valid udbs
     */
    GT_U8 notValidIndex[30];

} PRV_APP_DEMO_IPFIX_OTHER_KEY_FIELDS_INDEX_STC;

typedef struct {

    GT_U32 keyTableIndex; /* Max value is APP_DEMO_IPFIX_KEY_TABLE_INDEX_CNS */

    PRV_APP_DEMO_IPFIX_IPV4_KEY_FIELDS_INDEX_STC *ipv4Key;

    PRV_APP_DEMO_IPFIX_IPV6_KEY_FIELDS_INDEX_STC *ipv6Key;

    PRV_APP_DEMO_IPFIX_OTHER_KEY_FIELDS_INDEX_STC *otherTypeKey;

} PRV_APP_DEMO_IPFIX_KEY_TABLE_CFG_STC;

/**
* @struct PRV_APP_DEMO_IPFIX_KEY_INDEX_MAPPING_STC
 *
 * @brief Flow Manager packet type to key index mapping structure.
*/
typedef struct{

    /** @brief Key index to packet type mapping.
     */
    GT_U32          keyIndexArr[APP_DEMO_IPFIX_MAX_PKT_TYPE_NUM];

} PRV_APP_DEMO_IPFIX_KEY_INDEX_MAPPING_STC;

typedef struct{
    /** @brief Key table index based udb configuration for flows classification.
    */
    PRV_APP_DEMO_IPFIX_KEY_TABLE_CFG_STC keyTableUdbCfg[APP_DEMO_IPFIX_KEY_TABLE_INDEX_CNS];

    /** @brief Packet type to key index mapping.
     */
    PRV_APP_DEMO_IPFIX_KEY_INDEX_MAPPING_STC keyIndexMapCfg[APP_DEMO_IPFIX_MAX_PP_DEVICES_CNS][APP_DEMO_IPFIX_MAX_PORT_NUM];

} APP_DEMO_IPFIX_UDB_CFG_STC;

static APP_DEMO_IPFIX_UDB_CFG_STC *appDemoIpfixUdbCfg = NULL;
static GT_U32 appDemoIpfixTotalUdbConfigCount = 0;

/* Local DB to store flowIDs  */
typedef struct APP_DEMO_IPFIX_NODE_STC{
    /** @brief Flow details.
     */
    GT_U32 flowId;

    /** @brief Next pointer.
     */
    struct APP_DEMO_IPFIX_NODE_STC *next;

}APP_DEMO_IPFIX_NODE_STC;

static APP_DEMO_IPFIX_NODE_STC *appDemoIpfixHeadToFlowsSLL=NULL;
static APP_DEMO_IPFIX_NODE_STC *appDemoIpfixTailToFlowsSLL=NULL;
static GT_BOOL appDemoEnableDb = GT_FALSE;
static GT_BOOL appDemoIpfixEnableQinQ = GT_FALSE;

GT_VOID appDemoIpfixEnableDbSet
(
    IN GT_BOOL  enableDb
)
{
    appDemoEnableDb = enableDb;
}

GT_STATUS appDemoIpfixConfigParamSet
(
   IN GT_U32                          flowMngId,
   IN APP_DEMO_IPFIX_CONFIG_PARAM_ENT paramId,
   IN GT_U32                          paramValue
)
{
    GT_STATUS rc = GT_OK;

    GT_UNUSED_PARAM(flowMngId);

    switch(paramId)
    {
        case APP_DEMO_IPFIX_CONFIG_PARAM_KEY_TABLE_INDEX_E:
          appDemoIpfixKeyTableIndex = paramValue;
          break;
        case APP_DEMO_IPFIX_CONFIG_PARAM_UDB_INDEX_E:
          appDemoIpfixUdbIndex = paramValue;
          break;
        case APP_DEMO_IPFIX_CONFIG_PARAM_OFFSET_E:
          appDemoIpfixOffset = paramValue;
          break;
        case APP_DEMO_IPFIX_CONFIG_PARAM_MASK_E:
          appDemoIpfixMask = paramValue;
          break;
        case APP_DEMO_IPFIX_CONFIG_PARAM_PORT_NUM_E:
          appDemoIpfixPortNum = paramValue;
          break;
        case APP_DEMO_IPFIX_CONFIG_PARAM_ENABLE_BITMAP_E:
          appDemoIpfixEnableBitmap = paramValue;
          break;
        case APP_DEMO_IPFIX_CONFIG_PARAM_IPV4_TCP_KEY_TABLE_INDEX_E:
          appDemoIpfixIpv4TcpKeyTableIndex = paramValue;
          break;
        case APP_DEMO_IPFIX_CONFIG_PARAM_IPV4_UDP_KEY_TABLE_INDEX_E:
          appDemoIpfixIpv4UdpKeyTableIndex = paramValue;
          break;
        case APP_DEMO_IPFIX_CONFIG_PARAM_OTHER_KEY_TABLE_INDEX_E:
          appDemoIpfixOtherKeyTableIndex = paramValue;
          break;
        default:
          CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
          break;
    }
    return rc;
}

GT_STATUS appDemoIpfixConfigParamGet
(
   IN  GT_U32                          flowMngId,
   IN  APP_DEMO_IPFIX_CONFIG_PARAM_ENT paramId,
   OUT GT_U32                          *paramValuePtr
)
{
    GT_STATUS rc = GT_OK;

    GT_UNUSED_PARAM(flowMngId);

    switch(paramId)
    {
        case APP_DEMO_IPFIX_CONFIG_PARAM_KEY_TABLE_INDEX_E:
          *paramValuePtr = appDemoIpfixKeyTableIndex;
          break;
        case APP_DEMO_IPFIX_CONFIG_PARAM_UDB_INDEX_E:
          *paramValuePtr = appDemoIpfixUdbIndex;
          break;
        case APP_DEMO_IPFIX_CONFIG_PARAM_OFFSET_E:
          *paramValuePtr = appDemoIpfixOffset;
          break;
        case APP_DEMO_IPFIX_CONFIG_PARAM_MASK_E:
          *paramValuePtr = appDemoIpfixMask;
          break;
        case APP_DEMO_IPFIX_CONFIG_PARAM_PORT_NUM_E:
          *paramValuePtr = appDemoIpfixPortNum;
          break;
        case APP_DEMO_IPFIX_CONFIG_PARAM_ENABLE_BITMAP_E:
          *paramValuePtr = appDemoIpfixEnableBitmap;
          break;
        case APP_DEMO_IPFIX_CONFIG_PARAM_IPV4_TCP_KEY_TABLE_INDEX_E:
          *paramValuePtr = appDemoIpfixIpv4TcpKeyTableIndex;
          break;
        case APP_DEMO_IPFIX_CONFIG_PARAM_IPV4_UDP_KEY_TABLE_INDEX_E:
          *paramValuePtr = appDemoIpfixIpv4UdpKeyTableIndex;
          break;
        case APP_DEMO_IPFIX_CONFIG_PARAM_OTHER_KEY_TABLE_INDEX_E:
          *paramValuePtr = appDemoIpfixOtherKeyTableIndex;
          break;
        default:
          CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
          break;
    }
    return rc;
}

GT_STATUS flowManagerFlowFieldConfigSetIpv4DipOnly
(
    IN  GT_U32                                 flowMngId,
    IN  GT_U8                                  devNum
)
{
    GT_STATUS                    rc = GT_OK;
    GT_U8                        offset = 0; /* UDB offset from anchor */
    GT_U32                       udbIndex = 0; /* UDB index */
    GT_U32                       index = 0; /* UDB index */
    GT_U32                       mask = 0;
    GT_U32                       keyTableIndex = 0;
    CPSS_DXCH_PCL_UDB_SELECT_STC udbSelectStruct;

    cpssOsMemSet(&udbSelectStruct, 0, sizeof(udbSelectStruct));

    if(debug_print_enable == GT_TRUE)
    {
        cpssOsPrintf("[%s:%d] flowManagerFlowFieldConfigSetIpv4DipOnly entered\n",
                      __FUNCNAME__, __LINE__);
    }

    /* Before setting flow classification for IPV4 TCP packet, it is necessary
     * to validate whether the flow id or not else there will be unnecesary
     * calls to set not useful pcl udb keys.
     */
    if (appDemoIpfixFlowMngId == APP_DEMO_IPFIX_FLOW_MANAGER_ID_NOT_VALID ||
        appDemoIpfixFlowMngId != flowMngId)
    {
        rc = GT_FAIL;
        if(debug_print_enable == GT_TRUE)
        {
            cpssOsPrintf("[%s:%d] Flow manager ID doesn't exist rc[%d]\n",
                __FUNCNAME__, __LINE__, rc);
        }
        return rc;
    }

    /* UDB[40] offset for Applicable Flow Sub-template */
    udbIndex = 40;
    offset = 91;

    if(debug_print_enable == GT_TRUE)
    {
        cpssOsPrintf("[%s:%d] Configuring Applicable Flow Sub-template,"
                     "udbIndex[%d] Offset[%d]\n",__FUNCNAME__, __LINE__, udbIndex, offset);
    }

    rc = cpssDxChPclUserDefinedByteSet(devNum,
                                       CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_30_E,
                                       CPSS_DXCH_PCL_PACKET_TYPE_IPV4_TCP_E,
                                       appDemoIpfixPclDirection,
                                       udbIndex,
                                       CPSS_DXCH_PCL_OFFSET_METADATA_E,
                                       offset /* Applicable Flow Sub-template */);
    if (GT_OK != rc)
    {
        cpssOsPrintf("[%s:%d] cpssDxChPclUserDefinedByteSet FAILED, rc [%d] devNum [%d]\n",
                      __FUNCNAME__, __LINE__, rc, devNum);
        return rc;
    }

    /* UDB[6-9] offset for IPv4 DIP */
    for(udbIndex = 6, offset = 18 ; udbIndex < 10; udbIndex++,offset++)
    {
        if(debug_print_enable == GT_TRUE)
        {
            cpssOsPrintf("[%s:%d] Configuring DIPv4, udbIndex[%d], offset[%d] \n",
                          __FUNCNAME__, __LINE__, udbIndex, offset);
        }

        rc = cpssDxChPclUserDefinedByteSet(devNum,
                                           CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_30_E,
                                           CPSS_DXCH_PCL_PACKET_TYPE_IPV4_TCP_E,
                                           appDemoIpfixPclDirection,
                                           udbIndex,
                                           CPSS_DXCH_PCL_OFFSET_L3_MINUS_2_E,
                                           offset);
        if (GT_OK != rc)
        {
            cpssOsPrintf("[%s:%d] cpssDxChPclUserDefinedByteSet FAILED, rc [%d] devNum [%d]\n",
                         __FUNCNAME__, __LINE__, rc, devNum);
            return rc;
        }
    }

    /* it's a must to enable UDB's */
    udbSelectStruct.ingrUdbReplaceArr[0]= GT_TRUE; /* PCL-ID */
    udbSelectStruct.ingrUdbReplaceArr[1]= GT_TRUE; /* PCL-ID */
    udbSelectStruct.udbSelectArr[5] = 40; /* Applicable Flow Sub-template */
    udbSelectStruct.udbSelectArr[10] = 6; /* DIPv4 */
    udbSelectStruct.udbSelectArr[11] = 7; /* DIPv4 */
    udbSelectStruct.udbSelectArr[12] = 8; /* DIPv4 */
    udbSelectStruct.udbSelectArr[13] = 9; /* DIPv4 */

    rc = cpssDxChPclUserDefinedBytesSelectSet(devNum,
                                              CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_30_E,
                                              CPSS_DXCH_PCL_PACKET_TYPE_IPV4_TCP_E,
                                              appDemoIpfixLookupNum,
                                              &udbSelectStruct);
    if (GT_OK != rc)
    {
        cpssOsPrintf("[%s:%d] cpssDxChPclUserDefinedBytesSelectSet FAILED, rc [%d] devNum [%d]\n",
                     __FUNCNAME__, __LINE__, rc, devNum);
        return rc;
    }

    if(appDemoIpfixUdbCfg == NULL)
    {
        appDemoIpfixUdbCfg = (APP_DEMO_IPFIX_UDB_CFG_STC *) cpssOsMalloc(sizeof(APP_DEMO_IPFIX_UDB_CFG_STC));
        if(NULL == appDemoIpfixUdbCfg)
        {
            rc = GT_BAD_STATE;
            cpssOsPrintf("[%s:%d] appDemoIpfixUdbCfg memory allocation FAILED rc[%d]\n",
                    __FUNCNAME__, __LINE__, rc);
            return rc;
        }

        for(keyTableIndex = 0; keyTableIndex < PRV_CPSS_DXCH_KEY_TABLE_INDEX_CNS; keyTableIndex++)
        {
            appDemoIpfixUdbCfg->keyTableUdbCfg[keyTableIndex].ipv4Key =  NULL;
            appDemoIpfixUdbCfg->keyTableUdbCfg[keyTableIndex].ipv6Key =  NULL;
            appDemoIpfixUdbCfg->keyTableUdbCfg[keyTableIndex].otherTypeKey =  NULL;
        }
    }

    if(appDemoIpfixUdbCfg->keyTableUdbCfg[appDemoIpfixIpv4TcpKeyTableIndex].ipv4Key == NULL)
    {
        appDemoIpfixUdbCfg->keyTableUdbCfg[appDemoIpfixIpv4TcpKeyTableIndex].ipv4Key =
            (PRV_APP_DEMO_IPFIX_IPV4_KEY_FIELDS_INDEX_STC *) cpssOsMalloc(sizeof(PRV_APP_DEMO_IPFIX_IPV4_KEY_FIELDS_INDEX_STC));
        if(NULL == appDemoIpfixUdbCfg->keyTableUdbCfg[appDemoIpfixIpv4TcpKeyTableIndex].ipv4Key)
        {
            rc = GT_BAD_STATE;
            cpssOsPrintf("[%s:%d] ipv4Key memory allocation FAILED rc[%d]\n",
                    __FUNCNAME__, __LINE__, rc);
            return rc;
        }
    }
    cpssOsMemSet(appDemoIpfixUdbCfg->keyTableUdbCfg[appDemoIpfixIpv4TcpKeyTableIndex].ipv4Key,
            APP_DEMO_IPFIX_NOT_USED_UDB, sizeof(PRV_APP_DEMO_IPFIX_IPV4_KEY_FIELDS_INDEX_STC));

    offset = appDemoIpfixPclId;
    mask = 0xFF;
    for(udbIndex = 0, index = 0; udbIndex < 2; udbIndex++, index++)
    {
        if(debug_print_enable == GT_TRUE)
        {
            cpssOsPrintf("[%s:%d] Configuring PCL-ID FlowFieldKey, udbIndex[%d] offset[%d]\n",
                    __FUNCNAME__, __LINE__, udbIndex, offset);
        }

        /* UDB[0-1] offset for PCL-ID */
        rc = cpssDxChFlowManagerKeyEntryUserDefinedByteSet(flowMngId,
                                                           appDemoIpfixIpv4TcpKeyTableIndex,
                                                           appDemoIpfixShortKeySize,
                                                           udbIndex,
                                                           CPSS_DXCH_FLOW_MANAGER_ANCHOR_FIXED_E,/* offset type */
                                                           offset,
                                                           mask);
        if (GT_OK != rc)
        {
            cpssOsPrintf("[%s:%d] cpssDxChFlowManagerKeyEntryUserDefinedByteSet FAILED, rc [%d] devNum [%d]\n",
                    __FUNCNAME__, __LINE__, rc, devNum);
            return rc;
        }

        appDemoIpfixUdbCfg->keyTableUdbCfg[appDemoIpfixIpv4TcpKeyTableIndex].ipv4Key->pclIdIndex[index] = udbIndex;

        offset = 0x0;
        mask = 0x3;
    }

    mask = 0xF;
    /* UDB[2-3] offset for FlowEntry VLAN-ID */
    for(udbIndex = 2, offset = 14, index = 0 ; udbIndex < 4; udbIndex++, offset++, index++)
    {

        if(debug_print_enable == GT_TRUE)
        {
            cpssOsPrintf("[%s:%d] Configuring VLAN-ID FlowFieldKey as NOT_USED, udbIndex[%d] offset[%d]\n",
                         __FUNCNAME__, __LINE__, udbIndex, offset);
        }

        rc = cpssDxChFlowManagerKeyEntryUserDefinedByteSet(flowMngId,
                                                           appDemoIpfixIpv4TcpKeyTableIndex,
                                                           appDemoIpfixShortKeySize,
                                                           udbIndex,
                                                           CPSS_DXCH_FLOW_MANAGER_ANCHOR_NOT_USED_E,/* offset type */
                                                           offset,
                                                           mask);
        if (GT_OK != rc)
        {
            cpssOsPrintf("[%s:%d] cpssDxChFlowManagerKeyEntryUserDefinedByteSet FAILED, rc [%d] devNum [%d]\n",
                         __FUNCNAME__, __LINE__, rc, devNum);
            return rc;
        }
        appDemoIpfixUdbCfg->keyTableUdbCfg[appDemoIpfixIpv4TcpKeyTableIndex].ipv4Key->vlanIdIndex[index] = APP_DEMO_IPFIX_NOT_USED_UDB;
        mask = 0xFF;
    }

    udbIndex = 4;
    offset = 0;
    mask = 0xFF;

    if(debug_print_enable == GT_TRUE)
    {
        cpssOsPrintf("[%s:%d] Configuring SourcePort FlowFieldKey as NOT_USED, udbIndex[%d] offset[%d]\n",
                     __FUNCNAME__, __LINE__, udbIndex, offset);
    }

    /* UDB[4] offset for FlowEntry SourcePort */
    rc = cpssDxChFlowManagerKeyEntryUserDefinedByteSet(flowMngId,
                                                       appDemoIpfixIpv4TcpKeyTableIndex,
                                                       appDemoIpfixShortKeySize,
                                                       udbIndex,
                                                       CPSS_DXCH_FLOW_MANAGER_ANCHOR_NOT_USED_E,/* offset type */
                                                       offset,
                                                       mask);
    if (GT_OK != rc)
    {
        cpssOsPrintf("[%s:%d] cpssDxChFlowManagerKeyEntryUserDefinedByteSet FAILED, rc [%d] devNum [%d]\n",
                     __FUNCNAME__, __LINE__, rc, devNum);
        return rc;
    }
    appDemoIpfixUdbCfg->keyTableUdbCfg[appDemoIpfixIpv4TcpKeyTableIndex].ipv4Key->srcPortIndex = APP_DEMO_IPFIX_NOT_USED_UDB;

    udbIndex = 5;
    offset = 0x1;

    if(debug_print_enable == GT_TRUE)
    {
        cpssOsPrintf("[%s:%d] Configuring Applicable Flow Sub-template FlowFieldKey,"
                     "udbIndex[%d] offset[%d]\n", __FUNCNAME__, __LINE__,
                     udbIndex, offset);
    }

    /* UDB[5] offset for FlowEntry Applicable Flow Sub-template */
    rc = cpssDxChFlowManagerKeyEntryUserDefinedByteSet(flowMngId,
                                                       appDemoIpfixIpv4TcpKeyTableIndex,
                                                       appDemoIpfixShortKeySize,
                                                       udbIndex,
                                                       CPSS_DXCH_FLOW_MANAGER_ANCHOR_FIXED_E,/* offset type */
                                                       offset,
                                                       mask);
    if (GT_OK != rc)
    {
        cpssOsPrintf("[%s:%d] cpssDxChFlowManagerKeyEntryUserDefinedByteSet FAILED, rc [%d] devNum [%d]\n",
                     __FUNCNAME__, __LINE__, rc, devNum);
        return rc;
    }
    appDemoIpfixUdbCfg->keyTableUdbCfg[appDemoIpfixIpv4TcpKeyTableIndex].ipv4Key->appFlowSubTempIndex = udbIndex;

    /* UDB[6-9] offset for FlowEntry SIPv4 */
    for(udbIndex = 6, offset = 14 , index = 0; udbIndex < 10; udbIndex++, offset++, index++)
    {
        if(debug_print_enable == GT_TRUE)
        {
            cpssOsPrintf("[%s:%d] Configuring SIPv4 FlowFieldKey as NOT_USED, udbIndex[%d] offset[%d]\n",
                         __FUNCNAME__, __LINE__, udbIndex, offset);
        }

        rc = cpssDxChFlowManagerKeyEntryUserDefinedByteSet(flowMngId,
                                                           appDemoIpfixIpv4TcpKeyTableIndex,
                                                           appDemoIpfixShortKeySize,
                                                           udbIndex,
                                                           CPSS_DXCH_FLOW_MANAGER_ANCHOR_NOT_USED_E,/* offset type */
                                                           offset,
                                                           mask);
        if (GT_OK != rc)
        {
            cpssOsPrintf("[%s:%d] cpssDxChFlowManagerKeyEntryUserDefinedByteSet FAILED, rc [%d] devNum [%d]\n",
                         __FUNCNAME__, __LINE__, rc, devNum);
            return rc;
        }
        appDemoIpfixUdbCfg->keyTableUdbCfg[appDemoIpfixIpv4TcpKeyTableIndex].ipv4Key->sipv4Index[index] = APP_DEMO_IPFIX_NOT_USED_UDB;
    }

    /* UDB[10-13] offset for FlowEntry DIPv4 */
    for(udbIndex = 10, offset = 18 , index = 0; udbIndex < 14; udbIndex++, offset++, index++)
    {
        if(debug_print_enable == GT_TRUE)
        {
            cpssOsPrintf("[%s:%d] Configuring DIPv4 FlowFieldKey, udbIndex[%d] offset[%d]\n",
                         __FUNCNAME__, __LINE__, udbIndex, offset);
        }

        rc = cpssDxChFlowManagerKeyEntryUserDefinedByteSet(flowMngId,
                                                           appDemoIpfixIpv4TcpKeyTableIndex,
                                                           appDemoIpfixShortKeySize,
                                                           udbIndex,
                                                           CPSS_DXCH_FLOW_MANAGER_ANCHOR_L3_E,/* offset type */
                                                           offset,
                                                           mask);
        if (GT_OK != rc)
        {
            cpssOsPrintf("[%s:%d] cpssDxChFlowManagerKeyEntryUserDefinedByteSet FAILED, rc [%d] devNum [%d]\n",
                         __FUNCNAME__, __LINE__, rc, devNum);
            return rc;
        }
        appDemoIpfixUdbCfg->keyTableUdbCfg[appDemoIpfixIpv4TcpKeyTableIndex].ipv4Key->dipv4Index[index] = udbIndex;
    }

    appDemoIpfixUdbCfg->keyTableUdbCfg[appDemoIpfixIpv4TcpKeyTableIndex].ipv4Key->pktType = APP_DEMO_IPFIX_PACKET_TYPE_IPV4_TCP_E;

    /* UDB[14-15] offset for FlowEntry TCP Src Port */
    for(udbIndex = 14, offset = 0 , index = 0; udbIndex < 16; udbIndex++, offset++, index++)
    {
        if(debug_print_enable == GT_TRUE)
        {
            cpssOsPrintf("[%s:%d] Configuring TCP Src Port FlowFieldKey as NOT_USED,"
                         "udbIndex[%d] offset[%d]\n", __FUNCNAME__, __LINE__, udbIndex, offset);
        }

        rc = cpssDxChFlowManagerKeyEntryUserDefinedByteSet(flowMngId,
                                                           appDemoIpfixIpv4TcpKeyTableIndex,
                                                           appDemoIpfixShortKeySize,
                                                           udbIndex,
                                                           CPSS_DXCH_FLOW_MANAGER_ANCHOR_NOT_USED_E,/* offset type */
                                                           offset,
                                                           mask);
        if (GT_OK != rc)
        {
            cpssOsPrintf("[%s:%d] cpssDxChFlowManagerKeyEntryUserDefinedByteSet FAILED, rc [%d] devNum [%d]\n",
                         __FUNCNAME__, __LINE__, rc, devNum);
        }
        appDemoIpfixUdbCfg->keyTableUdbCfg[appDemoIpfixIpv4TcpKeyTableIndex].ipv4Key->protocol.protoSrcPortIndex[index] = APP_DEMO_IPFIX_NOT_USED_UDB;
    }

    /* UDB[16-17] offset for FlowEntry TCP Dst Port */
    for(udbIndex = 16, offset = 0 , index = 0; udbIndex < 18; udbIndex++, offset++, index++)
    {
        if(debug_print_enable == GT_TRUE)
        {
            cpssOsPrintf("[%s:%d] Configuring TCP Dst Port FlowFieldKey as NOT_USED,"
                         "udbIndex[%d] offset[%d]\n", __FUNCNAME__, __LINE__, udbIndex, offset);
        }

        rc = cpssDxChFlowManagerKeyEntryUserDefinedByteSet(flowMngId,
                                                           appDemoIpfixIpv4TcpKeyTableIndex,
                                                           appDemoIpfixShortKeySize,
                                                           udbIndex,
                                                           CPSS_DXCH_FLOW_MANAGER_ANCHOR_NOT_USED_E,/* offset type */
                                                           offset,
                                                           mask);
        if (GT_OK != rc)
        {
            cpssOsPrintf("[%s:%d] cpssDxChFlowManagerKeyEntryUserDefinedByteSet FAILED, rc [%d] devNum [%d]\n",
                         __FUNCNAME__, __LINE__, rc, devNum);
        }
        appDemoIpfixUdbCfg->keyTableUdbCfg[appDemoIpfixIpv4TcpKeyTableIndex].ipv4Key->protocol.protoDstPortIndex[index] = APP_DEMO_IPFIX_NOT_USED_UDB;
    }

    /* UDB[18-29] offset for NOT_VALID FlowEntry */
    for(udbIndex = 18, offset = 0 , index = 0; udbIndex < 30; udbIndex++, offset++, index++)
    {
        if(debug_print_enable == GT_TRUE)
        {
            cpssOsPrintf("[%s:%d] Configuring 18-29 FlowFieldKey as NOT_VALID,"
                         "udbIndex[%d] offset[%d]\n", __FUNCNAME__, __LINE__, udbIndex, offset);
        }
        rc = cpssDxChFlowManagerKeyEntryUserDefinedByteSet(flowMngId,
                                                           appDemoIpfixIpv4TcpKeyTableIndex,
                                                           appDemoIpfixShortKeySize,
                                                           udbIndex,
                                                           CPSS_DXCH_FLOW_MANAGER_ANCHOR_NOT_VALID_E,/* offset type */
                                                           offset,
                                                           mask);
        if (GT_OK != rc)
        {
            cpssOsPrintf("[%s:%d] cpssDxChFlowManagerKeyEntryUserDefinedByteSet FAILED, rc [%d] devNum [%d]\n",
                         __FUNCNAME__, __LINE__, rc, devNum);
        }
        appDemoIpfixUdbCfg->keyTableUdbCfg[appDemoIpfixIpv4TcpKeyTableIndex].ipv4Key->notValidIndex[index] = udbIndex;
    }

    /* Incrementing the udb configured count */
    appDemoIpfixTotalUdbConfigCount++;

    if(debug_print_enable == GT_TRUE)
    {
        cpssOsPrintf("[%s:%d] flowManagerFlowFieldConfigSetIpv4DipOnly successfully exiting\n",__FUNCNAME__, __LINE__);
    }

    return GT_OK;
}

/* Function to be used to enable all key fields for Ipv4-TCP pkt type */
GT_STATUS appDemoIpfixIpv4TcpFlowFieldConfig
(
    IN  GT_U32                                 flowMngId,
    IN  GT_BOOL                                enable
)
{
    CPSS_DXCH_PCL_UDB_SELECT_STC   udbSelectStruct;

    GT_U8      offset = 0; /* UDB offset from anchor */
    GT_U32     udbIndex = 0;
    GT_U32     index = 0; /* UDB index */
    GT_U32     mask = 0;
    GT_U32     keyTableIndex = 0;
    GT_U32     udbIndexQinq = 18;
    GT_STATUS  rc = GT_OK;
    GT_U32     appDemoIpfixDevNum = prvWrAppFirstDevNum(0);

    cpssOsMemSet(&udbSelectStruct, 0, sizeof(udbSelectStruct));

    if(debug_print_enable == GT_TRUE)
    {
        cpssOsPrintf("[%s:%d] appDemoIpfixIpv4TcpFlowFieldConfig entered\n",__FUNCNAME__, __LINE__);
    }

    /* Before setting flow classification for IPV4 TCP packet, it is necessary
     * to validate whether the flow id or not else there will be unnecesary
     * calls to set not useful pcl udb keys.
     */
    if (appDemoIpfixFlowMngId == APP_DEMO_IPFIX_FLOW_MANAGER_ID_NOT_VALID ||
        appDemoIpfixFlowMngId != flowMngId)
    {
        rc = GT_FAIL;
        if(debug_print_enable == GT_TRUE)
        {
            cpssOsPrintf("[%s:%d] Flow manager ID doesn't exist rc[%d]\n",
                __FUNCNAME__, __LINE__, rc);
        }
        return rc;
    }

    if(enable == GT_TRUE)
    {
        /* UDB[40] offset for Applicable Flow Sub-template */
        udbIndex = 40;
        offset = 91;

        if(debug_print_enable == GT_TRUE)
        {
            cpssOsPrintf("[%s:%d] Configuring Applicable Flow Sub-template,"
                         "udbIndex[%d] Offset[%d]\n",__FUNCNAME__, __LINE__, udbIndex, offset);
        }

        rc = cpssDxChPclUserDefinedByteSet(appDemoIpfixDevNum,
                                           CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_30_E,
                                           CPSS_DXCH_PCL_PACKET_TYPE_IPV4_TCP_E,
                                           appDemoIpfixPclDirection,
                                           udbIndex,
                                           CPSS_DXCH_PCL_OFFSET_METADATA_E,
                                           offset /* Applicable Flow Sub-template */);
        if (GT_OK != rc)
        {
            cpssOsPrintf("[%s:%d] cpssDxChPclUserDefinedByteSet FAILED, rc [%d] devNum [%d]\n",
                         __FUNCNAME__, __LINE__, rc, appDemoIpfixDevNum);
            return rc;
        }

        /* UDB[0-1] offset for VLAN-ID */
        for(udbIndex = 0, offset = 14 ; udbIndex < 2; udbIndex++,offset++)
        {
            if(debug_print_enable == GT_TRUE)
            {
                cpssOsPrintf("[%s:%d] Configuring VLAN-ID, udbIndex[%d] Offset[%d]\n",
                             __FUNCNAME__, __LINE__, udbIndex, offset);
            }

            rc = cpssDxChPclUserDefinedByteSet(appDemoIpfixDevNum,
                                               CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_30_E,
                                               CPSS_DXCH_PCL_PACKET_TYPE_IPV4_TCP_E,
                                               appDemoIpfixPclDirection,
                                               udbIndex,/* vlan-id */
                                               CPSS_DXCH_PCL_OFFSET_L2_E,
                                               offset);
            if (GT_OK != rc)
            {
                cpssOsPrintf("[%s:%d] cpssDxChPclUserDefinedByteSet FAILED, rc [%d] devNum [%d]\n",
                             __FUNCNAME__, __LINE__, rc, appDemoIpfixDevNum);
                return rc;
            }
        }

        /* UDB[2-5] offset for SIPv4 */
        for(udbIndex = 2, offset = 14 ; udbIndex < 6; udbIndex++,offset++)
        {
            if(debug_print_enable == GT_TRUE)
            {
                cpssOsPrintf("[%s:%d] Configuring SIPv4, udbIndex[%d] Offset[%d]\n",
                             __FUNCNAME__, __LINE__, udbIndex, offset);
            }

            rc = cpssDxChPclUserDefinedByteSet(appDemoIpfixDevNum,
                                               CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_30_E,
                                               CPSS_DXCH_PCL_PACKET_TYPE_IPV4_TCP_E,
                                               appDemoIpfixPclDirection,
                                               udbIndex,
                                               CPSS_DXCH_PCL_OFFSET_L3_MINUS_2_E,
                                               offset);
            if (GT_OK != rc)
            {
                cpssOsPrintf("[%s:%d] cpssDxChPclUserDefinedByteSet FAILED, rc [%d] devNum [%d]\n",
                             __FUNCNAME__, __LINE__, rc, appDemoIpfixDevNum);
                return rc;
            }
        }

        /* UDB[6-9] offset for DIPv4 */
        for(udbIndex = 6, offset = 18 ; udbIndex < 10; udbIndex++,offset++)
        {
            if(debug_print_enable == GT_TRUE)
            {
                cpssOsPrintf("[%s:%d] Configuring DIPv4, udbIndex[%d] Offset[%d]\n",
                             __FUNCNAME__, __LINE__, udbIndex, offset);
            }

            rc = cpssDxChPclUserDefinedByteSet(appDemoIpfixDevNum,
                                               CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_30_E,
                                               CPSS_DXCH_PCL_PACKET_TYPE_IPV4_TCP_E,
                                               appDemoIpfixPclDirection,
                                               udbIndex,
                                               CPSS_DXCH_PCL_OFFSET_L3_MINUS_2_E,
                                               offset);
            if (GT_OK != rc)
            {
                cpssOsPrintf("[%s:%d] cpssDxChPclUserDefinedByteSet FAILED, rc [%d] devNum [%d]\n",
                             __FUNCNAME__, __LINE__, rc, appDemoIpfixDevNum);
                return rc;
            }
        }

        /* UDB[10-11] offset for TCP Src Port */
        for(udbIndex = 10, offset = 0 ; udbIndex < 12; udbIndex++,offset++)
        {
            if(debug_print_enable == GT_TRUE)
            {
                cpssOsPrintf("[%s:%d] Configuring TCP Src Port, udbIndex[%d] Offset[%d]\n",
                             __FUNCNAME__, __LINE__, udbIndex, offset);
            }
            rc = cpssDxChPclUserDefinedByteSet(appDemoIpfixDevNum,
                                               CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_30_E,
                                               CPSS_DXCH_PCL_PACKET_TYPE_IPV4_TCP_E,
                                               appDemoIpfixPclDirection,
                                               udbIndex,
                                               CPSS_DXCH_PCL_OFFSET_L4_E,
                                               offset);
            if (GT_OK != rc)
            {
                cpssOsPrintf("[%s:%d] cpssDxChPclUserDefinedByteSet FAILED, rc [%d] devNum [%d]\n",
                             __FUNCNAME__, __LINE__, rc, appDemoIpfixDevNum);
                return rc;
            }
        }

        /* UDB[12-13] offset for TCP Dst Port */
        for(udbIndex = 12, offset = 2 ; udbIndex < 14; udbIndex++,offset++)
        {
            if(debug_print_enable == GT_TRUE)
            {
                cpssOsPrintf("[%s:%d] Configuring TCP Dst Port, udbIndex[%d] Offset[%d]\n",
                             __FUNCNAME__, __LINE__, udbIndex, offset);
            }
            rc = cpssDxChPclUserDefinedByteSet(appDemoIpfixDevNum,
                                               CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_30_E,
                                               CPSS_DXCH_PCL_PACKET_TYPE_IPV4_TCP_E,
                                               appDemoIpfixPclDirection,
                                               udbIndex,
                                               CPSS_DXCH_PCL_OFFSET_L4_E,
                                               offset);
            if (GT_OK != rc)
            {
                cpssOsPrintf("[%s:%d] cpssDxChPclUserDefinedByteSet FAILED, rc [%d] devNum [%d]\n",
                             __FUNCNAME__, __LINE__, rc, appDemoIpfixDevNum);
                return rc;
            }
        }

        if(appDemoIpfixEnableQinQ == GT_TRUE)
        {
            /* UDB[14-15] offset for Inner VLAN-ID */
            for(udbIndex = 14, offset = 18; udbIndex < 16; udbIndex++,offset++)
            {
                if(debug_print_enable == GT_TRUE)
                {
                    cpssOsPrintf("[%s:%d] Configuring VLAN-ID, udbIndex[%d] Offset[%d]\n",
                        __FUNCNAME__, __LINE__, udbIndex, offset);
                }

                rc = cpssDxChPclUserDefinedByteSet(appDemoIpfixDevNum,
                    CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_30_E,
                    CPSS_DXCH_PCL_PACKET_TYPE_IPV4_TCP_E,
                    appDemoIpfixPclDirection,
                    udbIndex,/* vlan-id */
                    CPSS_DXCH_PCL_OFFSET_L2_E,
                    offset);
                if (GT_OK != rc)
                {
                    cpssOsPrintf("[%s:%d] cpssDxChPclUserDefinedByteSet FAILED, rc [%d] devNum [%d]\n",
                        __FUNCNAME__, __LINE__, rc, appDemoIpfixDevNum);
                    return rc;
                }
            }
        }

        /* it's a must to enable UDB's */
        udbSelectStruct.ingrUdbReplaceArr[0] = GT_TRUE; /* PCL-ID */
        udbSelectStruct.ingrUdbReplaceArr[1] = GT_TRUE; /* PCL-ID */
        udbSelectStruct.udbSelectArr[2]      = 0; /* VLAN-ID */
        udbSelectStruct.udbSelectArr[3]      = 1; /* VLAN-ID */
        udbSelectStruct.ingrUdbReplaceArr[4] = GT_TRUE; /* Source ePort */
        udbSelectStruct.udbSelectArr[5]      = 40; /* Applicable Flow Sub-template */
        udbSelectStruct.udbSelectArr[6]      = 2; /* SIPv4 */
        udbSelectStruct.udbSelectArr[7]      = 3; /* SIPv4 */
        udbSelectStruct.udbSelectArr[8]      = 4; /* SIPv4 */
        udbSelectStruct.udbSelectArr[9]      = 5; /* SIPv4 */
        udbSelectStruct.udbSelectArr[10]     = 6; /* DIPv4 */
        udbSelectStruct.udbSelectArr[11]     = 7; /* DIPv4 */
        udbSelectStruct.udbSelectArr[12]     = 8; /* DIPv4 */
        udbSelectStruct.udbSelectArr[13]     = 9; /* DIPv4 */
        udbSelectStruct.udbSelectArr[14]     = 10; /* TCP Src Port */
        udbSelectStruct.udbSelectArr[15]     = 11; /* TCP Src Port */
        udbSelectStruct.udbSelectArr[16]     = 12; /* TCP Dst Port */
        udbSelectStruct.udbSelectArr[17]     = 13; /* TCP Dst Port */
        if(appDemoIpfixEnableQinQ == GT_TRUE)
        {
            udbSelectStruct.udbSelectArr[18]     = 14; /* Inner VLAN-ID */
            udbSelectStruct.udbSelectArr[19]     = 15; /* Inner VLAN-ID */
        }

        rc = cpssDxChPclUserDefinedBytesSelectSet(appDemoIpfixDevNum,
                                                  CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_30_E,
                                                  CPSS_DXCH_PCL_PACKET_TYPE_IPV4_TCP_E,
                                                  appDemoIpfixLookupNum,
                                                  &udbSelectStruct);
        if (GT_OK != rc)
        {
            cpssOsPrintf("[%s:%d] cpssDxChPclUserDefinedBytesSelectSet FAILED, rc [%d] devNum [%d]\n",
                         __FUNCNAME__, __LINE__, rc, appDemoIpfixDevNum);
            return rc;
        }

        if(appDemoIpfixUdbCfg == NULL)
        {
            appDemoIpfixUdbCfg = (APP_DEMO_IPFIX_UDB_CFG_STC *) cpssOsMalloc(sizeof(APP_DEMO_IPFIX_UDB_CFG_STC));
            if(NULL == appDemoIpfixUdbCfg)
            {
                rc = GT_BAD_STATE;
                cpssOsPrintf("[%s:%d] appDemoIpfixUdbCfg memory allocation FAILED rc[%d]\n",
                        __FUNCNAME__, __LINE__, rc);
                return rc;
            }

            for(keyTableIndex = 0; keyTableIndex < APP_DEMO_IPFIX_KEY_TABLE_INDEX_CNS; keyTableIndex++)
            {
                appDemoIpfixUdbCfg->keyTableUdbCfg[keyTableIndex].ipv4Key =  NULL;
                appDemoIpfixUdbCfg->keyTableUdbCfg[keyTableIndex].ipv6Key =  NULL;
                appDemoIpfixUdbCfg->keyTableUdbCfg[keyTableIndex].otherTypeKey =  NULL;
            }
        }

        if(appDemoIpfixUdbCfg->keyTableUdbCfg[appDemoIpfixIpv4TcpKeyTableIndex].ipv4Key == NULL)
        {
            appDemoIpfixUdbCfg->keyTableUdbCfg[appDemoIpfixIpv4TcpKeyTableIndex].ipv4Key =
                (PRV_APP_DEMO_IPFIX_IPV4_KEY_FIELDS_INDEX_STC *)
                cpssOsMalloc(sizeof(PRV_APP_DEMO_IPFIX_IPV4_KEY_FIELDS_INDEX_STC));
            if(NULL == appDemoIpfixUdbCfg->keyTableUdbCfg[appDemoIpfixIpv4TcpKeyTableIndex].ipv4Key)
            {
                rc = GT_BAD_STATE;
                cpssOsPrintf("[%s:%d] ipv4Key memory allocation FAILED rc[%d]\n",
                        __FUNCNAME__, __LINE__, rc);
                return rc;
            }

        }
        cpssOsMemSet(appDemoIpfixUdbCfg->keyTableUdbCfg[appDemoIpfixIpv4TcpKeyTableIndex].ipv4Key,
                APP_DEMO_IPFIX_NOT_USED_UDB, sizeof(PRV_APP_DEMO_IPFIX_IPV4_KEY_FIELDS_INDEX_STC));

        offset = appDemoIpfixPclId;
        mask = 0xFF;
        for(udbIndex = 0, index = 0; udbIndex < 2; udbIndex++, index++)
        {
            if(debug_print_enable == GT_TRUE)
            {
                cpssOsPrintf("[%s:%d] Configuring PCL-ID FlowFieldKey, udbIndex[%d] offset[%d]\n",
                        __FUNCNAME__, __LINE__, udbIndex, offset);
            }

            /* UDB[0-1] offset for PCL-ID */
            rc = cpssDxChFlowManagerKeyEntryUserDefinedByteSet(flowMngId,
                                                               appDemoIpfixIpv4TcpKeyTableIndex,
                                                               appDemoIpfixShortKeySize,
                                                               udbIndex,
                                                               CPSS_DXCH_FLOW_MANAGER_ANCHOR_FIXED_E,/* offset type */
                                                               offset,
                                                               mask);
            if (GT_OK != rc)
            {
                cpssOsPrintf("[%s:%d] cpssDxChFlowManagerKeyEntryUserDefinedByteSet FAILED, rc [%d] devNum [%d]\n",
                        __FUNCNAME__, __LINE__, rc, appDemoIpfixDevNum);
                return rc;
            }
            appDemoIpfixUdbCfg->keyTableUdbCfg[appDemoIpfixIpv4TcpKeyTableIndex].ipv4Key->pclIdIndex[index] = udbIndex;
            offset = 0x0;
            mask = 0x3;
        }

        mask = 0xF;
        /* UDB[2-3] offset for FlowEntry VLAN-ID */
        for(udbIndex = 2, offset = 0 , index = 0; udbIndex < 4; udbIndex++, offset++, index++)
        {
            if(debug_print_enable == GT_TRUE)
            {
                cpssOsPrintf("[%s:%d] Configuring VLAN-ID FlowFieldKey, udbIndex[%d] offset[%d]\n",
                             __FUNCNAME__, __LINE__, udbIndex, offset);
            }

            rc = cpssDxChFlowManagerKeyEntryUserDefinedByteSet(flowMngId,
                                                               appDemoIpfixIpv4TcpKeyTableIndex,
                                                               appDemoIpfixShortKeySize,
                                                               udbIndex,
                                                               CPSS_DXCH_FLOW_MANAGER_ANCHOR_VID_E,/* offset type */
                                                               offset,
                                                               mask);
            if (GT_OK != rc)
            {
                cpssOsPrintf("[%s:%d] cpssDxChFlowManagerKeyEntryUserDefinedByteSet FAILED, rc [%d] devNum [%d]\n",
                             __FUNCNAME__, __LINE__, rc, appDemoIpfixDevNum);
                return rc;
            }
            appDemoIpfixUdbCfg->keyTableUdbCfg[appDemoIpfixIpv4TcpKeyTableIndex].ipv4Key->vlanIdIndex[index] = udbIndex;
            mask = 0xFF;
        }

        udbIndex = 4;
        offset = 0;
        mask = 0xFF;

        if(debug_print_enable == GT_TRUE)
        {
            cpssOsPrintf("[%s:%d] Configuring SourcePort FlowFieldKey, udbIndex[%d] offset[%d]\n",
                         __FUNCNAME__, __LINE__, udbIndex, offset);
        }

        /* UDB[4] offset for FlowEntry SourcePort */
        rc = cpssDxChFlowManagerKeyEntryUserDefinedByteSet(flowMngId,
                                                           appDemoIpfixIpv4TcpKeyTableIndex,
                                                           appDemoIpfixShortKeySize,
                                                           udbIndex,
                                                           CPSS_DXCH_FLOW_MANAGER_ANCHOR_SRC_PORT_E,/* offset type */
                                                           offset,
                                                           mask);
        if (GT_OK != rc)
        {
            cpssOsPrintf("[%s:%d] cpssDxChFlowManagerKeyEntryUserDefinedByteSet FAILED, rc [%d] devNum [%d]\n",
                         __FUNCNAME__, __LINE__, rc, appDemoIpfixDevNum);
            return rc;
        }
        appDemoIpfixUdbCfg->keyTableUdbCfg[appDemoIpfixIpv4TcpKeyTableIndex].ipv4Key->srcPortIndex = udbIndex;

        udbIndex = 5;
        offset = 0x1;

        if(debug_print_enable == GT_TRUE)
        {
            cpssOsPrintf("[%s:%d] Configuring Applicable Flow Sub-template FlowFieldKey,"
                         "udbIndex[%d] offset[%d]\n",__FUNCNAME__, __LINE__,
                         udbIndex, offset);
        }

        /* UDB[5] offset for FlowEntry Applicable Flow Sub-template */
        rc = cpssDxChFlowManagerKeyEntryUserDefinedByteSet(flowMngId,
                                                           appDemoIpfixIpv4TcpKeyTableIndex,
                                                           appDemoIpfixShortKeySize,
                                                           udbIndex,
                                                           CPSS_DXCH_FLOW_MANAGER_ANCHOR_FIXED_E,/* offset type */
                                                           offset,
                                                           mask);
        if (GT_OK != rc)
        {
            cpssOsPrintf("[%s:%d] cpssDxChFlowManagerKeyEntryUserDefinedByteSet FAILED, rc [%d] devNum [%d]\n",
                         __FUNCNAME__, __LINE__, rc, appDemoIpfixDevNum);
            return rc;
        }
        appDemoIpfixUdbCfg->keyTableUdbCfg[appDemoIpfixIpv4TcpKeyTableIndex].ipv4Key->appFlowSubTempIndex = udbIndex;

        /* UDB[6-9] offset for FlowEntry SIPv4 */
        for(udbIndex = 6, offset = 14 , index = 0; udbIndex < 10; udbIndex++, offset++, index++)
        {
            if(debug_print_enable == GT_TRUE)
            {
                cpssOsPrintf("[%s:%d] Configuring SIPv4 FlowFieldKey, udbIndex[%d] offset[%d]\n",
                             __FUNCNAME__, __LINE__, udbIndex, offset);
            }

            rc = cpssDxChFlowManagerKeyEntryUserDefinedByteSet(flowMngId,
                                                               appDemoIpfixIpv4TcpKeyTableIndex,
                                                               appDemoIpfixShortKeySize,
                                                               udbIndex,
                                                               CPSS_DXCH_FLOW_MANAGER_ANCHOR_L3_E,/* offset type */
                                                               offset,
                                                               mask);
            if (GT_OK != rc)
            {
                cpssOsPrintf("[%s:%d] cpssDxChFlowManagerKeyEntryUserDefinedByteSet FAILED, rc [%d] devNum [%d]\n",
                             __FUNCNAME__, __LINE__, rc, appDemoIpfixDevNum);
                return rc;
            }
            appDemoIpfixUdbCfg->keyTableUdbCfg[appDemoIpfixIpv4TcpKeyTableIndex].ipv4Key->sipv4Index[index] = udbIndex;
        }

        /* UDB[10-13] offset for FlowEntry DIPv4 */
        for(udbIndex = 10, offset = 18 ,index = 0; udbIndex < 14; udbIndex++, offset++, index++)
        {
            if(debug_print_enable == GT_TRUE)
            {
                cpssOsPrintf("[%s:%d] Configuring DIPv4 FlowFieldKey, udbIndex[%d] offset[%d]\n",
                             __FUNCNAME__, __LINE__, udbIndex, offset);
            }

            rc = cpssDxChFlowManagerKeyEntryUserDefinedByteSet(flowMngId,
                                                               appDemoIpfixIpv4TcpKeyTableIndex,
                                                               appDemoIpfixShortKeySize,
                                                               udbIndex,
                                                               CPSS_DXCH_FLOW_MANAGER_ANCHOR_L3_E,/* offset type */
                                                               offset,
                                                               mask);
            if (GT_OK != rc)
            {
                cpssOsPrintf("[%s:%d] cpssDxChFlowManagerKeyEntryUserDefinedByteSet FAILED, rc [%d] devNum [%d]\n",
                             __FUNCNAME__, __LINE__, rc, appDemoIpfixDevNum);
                return rc;
            }
            appDemoIpfixUdbCfg->keyTableUdbCfg[appDemoIpfixIpv4TcpKeyTableIndex].ipv4Key->dipv4Index[index] = udbIndex;
        }


        appDemoIpfixUdbCfg->keyTableUdbCfg[appDemoIpfixIpv4TcpKeyTableIndex].ipv4Key->pktType = APP_DEMO_IPFIX_PACKET_TYPE_IPV4_TCP_E;

        /* UDB[14-15] offset for FlowEntry TCP Src Port */
        for(udbIndex = 14, offset = 0 , index = 0; udbIndex < 16; udbIndex++, offset++, index++)
        {
            if(debug_print_enable == GT_TRUE)
            {
                cpssOsPrintf("[%s:%d] Configuring TCP Src Port FlowFieldKey,"
                             "udbIndex[%d] offset[%d]\n", __FUNCNAME__, __LINE__, udbIndex, offset);
            }

            rc = cpssDxChFlowManagerKeyEntryUserDefinedByteSet(flowMngId,
                                                               appDemoIpfixIpv4TcpKeyTableIndex,
                                                               appDemoIpfixShortKeySize,
                                                               udbIndex,
                                                               CPSS_DXCH_FLOW_MANAGER_ANCHOR_L4_E,/* offset type */
                                                               offset,
                                                               mask);
            if (GT_OK != rc)
            {
                cpssOsPrintf("[%s:%d] cpssDxChFlowManagerKeyEntryUserDefinedByteSet FAILED, rc [%d] devNum [%d]\n",
                             __FUNCNAME__, __LINE__, rc, appDemoIpfixDevNum);
            }
            appDemoIpfixUdbCfg->keyTableUdbCfg[appDemoIpfixIpv4TcpKeyTableIndex].ipv4Key->protocol.protoSrcPortIndex[index] = udbIndex;
        }

        /* UDB[16-17] offset for FlowEntry TCP Dst Port */
        for(udbIndex = 16, offset = 2 , index = 0; udbIndex < 18; udbIndex++, offset++, index++)
        {
            if(debug_print_enable ==  GT_TRUE)
            {
                cpssOsPrintf("[%s:%d] Configuring TCP Dst Port FlowFieldKey,"
                             "udbIndex[%d] offset[%d]\n", __FUNCNAME__, __LINE__, udbIndex, offset);
            }

            rc = cpssDxChFlowManagerKeyEntryUserDefinedByteSet(flowMngId,
                                                               appDemoIpfixIpv4TcpKeyTableIndex,
                                                               appDemoIpfixShortKeySize,
                                                               udbIndex,
                                                               CPSS_DXCH_FLOW_MANAGER_ANCHOR_L4_E,/* offset type */
                                                               offset,
                                                               mask);
            if (GT_OK != rc)
            {
                cpssOsPrintf("[%s:%d] cpssDxChFlowManagerKeyEntryUserDefinedByteSet FAILED, rc [%d] devNum [%d]\n",
                             __FUNCNAME__, __LINE__, rc, appDemoIpfixDevNum);
            }
            appDemoIpfixUdbCfg->keyTableUdbCfg[appDemoIpfixIpv4TcpKeyTableIndex].ipv4Key->protocol.protoDstPortIndex[index] = udbIndex;
        }

        if(appDemoIpfixEnableQinQ == GT_TRUE)
        {
            mask = 0xF;
            /* UDB[18-19] offset for FlowEntry Inner VLAN-ID */
            for(udbIndex = 18, offset = 18, index = 18; udbIndex < 20; udbIndex++, offset++, index++)
            {
              if(debug_print_enable == GT_TRUE)
              {
                cpssOsPrintf("[%s:%d] Configuring VLAN-ID FlowFieldKey, udbIndex[%d] offset[%d]\n",
                    __FUNCNAME__, __LINE__, udbIndex, offset);
              }

              rc = cpssDxChFlowManagerKeyEntryUserDefinedByteSet(flowMngId,
                  appDemoIpfixIpv4TcpKeyTableIndex,
                  appDemoIpfixShortKeySize,
                  udbIndex,
                  CPSS_DXCH_FLOW_MANAGER_ANCHOR_L2_E,/* offset type */
                  offset,
                  mask);
              if (GT_OK != rc)
              {
                cpssOsPrintf("[%s:%d] cpssDxChFlowManagerKeyEntryUserDefinedByteSet FAILED, rc [%d] devNum [%d]\n",
                    __FUNCNAME__, __LINE__, rc, appDemoIpfixDevNum);
                return rc;
              }
              mask = 0xFF;
            }
            udbIndexQinq = 20;
        }

        /* UDB[18-29 or 20-29 for qinq] offset for NOT_VALID FlowEntry */
        for(udbIndex = udbIndexQinq, offset = 0 , index = 0; udbIndex < 30; udbIndex++, offset++, index++)
        {
            if(debug_print_enable == GT_TRUE)
            {
                cpssOsPrintf("[%s:%d] Configuring 18-29 FlowFieldKey as NOT_VALID,"
                             "udbIndex[%d] offset[%d]\n", __FUNCNAME__, __LINE__, udbIndex, offset);
            }
            rc = cpssDxChFlowManagerKeyEntryUserDefinedByteSet(flowMngId,
                                                               appDemoIpfixIpv4TcpKeyTableIndex,
                                                               appDemoIpfixShortKeySize,
                                                               udbIndex,
                                                               CPSS_DXCH_FLOW_MANAGER_ANCHOR_NOT_VALID_E,/* offset type */
                                                               offset,
                                                               mask);
            if (GT_OK != rc)
            {
                cpssOsPrintf("[%s:%d] cpssDxChFlowManagerKeyEntryUserDefinedByteSet FAILED, rc [%d] devNum [%d]\n",
                             __FUNCNAME__, __LINE__, rc, appDemoIpfixDevNum);
            }
            appDemoIpfixUdbCfg->keyTableUdbCfg[appDemoIpfixIpv4TcpKeyTableIndex].ipv4Key->notValidIndex[index] = udbIndex;
        }

        /* Incrementing the udb configured count */
        appDemoIpfixTotalUdbConfigCount++;

    }
    else {

        if(appDemoIpfixUdbCfg == NULL)
        {
            if (appDemoIpfixTotalUdbConfigCount == 0)
            {
                rc = GT_OK;
                if(debug_print_enable)
                {
                    cpssOsPrintf("[%s:%d] No UDBs configured\n", __FUNCNAME__, __LINE__);
                }
            }
            else
            {
                rc = GT_BAD_STATE;
                cpssOsPrintf("[%s:%d] Retrieving appDemoIpfixUdbCfg  FAILED rc[%d]\n",
                        __FUNCNAME__, __LINE__, rc);
            }
            return rc;
        }

        for(keyTableIndex = 0; keyTableIndex < APP_DEMO_IPFIX_KEY_TABLE_INDEX_CNS; keyTableIndex++)
        {
            if(appDemoIpfixUdbCfg->keyTableUdbCfg[keyTableIndex].ipv4Key != NULL)
            {
                cpssOsFree(appDemoIpfixUdbCfg->keyTableUdbCfg[keyTableIndex].ipv4Key);
                appDemoIpfixUdbCfg->keyTableUdbCfg[keyTableIndex].ipv4Key = NULL;

                /* Decrementing the udb configured count */
                appDemoIpfixTotalUdbConfigCount--;
            }

        }

        /* Clearing UDB config */
        rc = cpssDxChPclUserDefinedBytesSelectSet(appDemoIpfixDevNum,
                                                  CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_30_E,
                                                  CPSS_DXCH_PCL_PACKET_TYPE_IPV4_TCP_E,
                                                  appDemoIpfixLookupNum,
                                                  &udbSelectStruct);
        if (GT_OK != rc)
        {
            cpssOsPrintf("[%s:%d] cpssDxChPclUserDefinedBytesSelectSet FAILED, rc [%d] devNum [%d]\n",
                         __FUNCNAME__, __LINE__, rc, appDemoIpfixDevNum);
            return rc;
        }
    }

    return GT_OK;
}

/* Function to be used to enable all key fields for Ipv4-UDP pkt type */
GT_STATUS appDemoIpfixIpv4UdpFlowFieldConfig
(
    IN  GT_U32                                 flowMngId,
    IN  GT_BOOL                                enable
)
{
    CPSS_DXCH_PCL_UDB_SELECT_STC   udbSelectStruct;

    GT_U8                          offset = 0; /* UDB offset from anchor */
    GT_U8                          devNum = 0;
    GT_U8                          portNum = 0;
    GT_U8                          pktType = 0;
    GT_U32                         udbIndex = 0;
    GT_U32                         keyTableIndex = 0;
    GT_U32                         index = 0; /* UDB index */
    GT_U32                         mask = 0;
    GT_U32                         udbIndexQinq = 18;
    GT_STATUS                      rc = GT_OK;
    GT_U32                         appDemoIpfixDevNum = prvWrAppFirstDevNum(0);

    cpssOsMemSet(&udbSelectStruct, 0, sizeof(udbSelectStruct));

    if(debug_print_enable == GT_TRUE)
    {
        cpssOsPrintf("[%s:%d] appDemoIpfixIpv4UdpFlowFieldConfig entered\n",__FUNCNAME__, __LINE__);
    }

    /* Before setting flow classification for IPV4 UDP packet, it is necessary
     * to validate whether the flow id exists or not else there will be
     * unnecesary calls to set not useful pcl udb keys.
     */
    if (appDemoIpfixFlowMngId == APP_DEMO_IPFIX_FLOW_MANAGER_ID_NOT_VALID ||
        appDemoIpfixFlowMngId != flowMngId)
    {
        rc = GT_FAIL;
        if(debug_print_enable == GT_TRUE)
        {
            cpssOsPrintf("[%s:%d] Flow manager ID doesn't exist rc[%d]\n",
                __FUNCNAME__, __LINE__, rc);
        }
        return rc;
    }

    if(enable == GT_TRUE)
    {
        /* UDB[40] offset for Applicable Flow Sub-template */
        udbIndex = 40;
        offset = 91;

        if(debug_print_enable == GT_TRUE)
        {
            cpssOsPrintf("[%s:%d] Configuring Applicable Flow Sub-template,"
                         "udbIndex[%d] Offset[%d]\n",__FUNCNAME__, __LINE__, udbIndex, offset);
        }

        rc = cpssDxChPclUserDefinedByteSet(appDemoIpfixDevNum,
                                           CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_30_E,
                                           CPSS_DXCH_PCL_PACKET_TYPE_IPV4_UDP_E,
                                           appDemoIpfixPclDirection,
                                           udbIndex,
                                           CPSS_DXCH_PCL_OFFSET_METADATA_E,
                                           offset /* Applicable Flow Sub-template */);
        if (GT_OK != rc)
        {
            cpssOsPrintf("[%s:%d] cpssDxChPclUserDefinedByteSet FAILED, rc [%d] devNum [%d]\n",
                         __FUNCNAME__, __LINE__, rc, appDemoIpfixDevNum);
            return rc;
        }

        /* UDB[0-1] offset for VLAN-ID */
        for(udbIndex = 0, offset = 14 ; udbIndex < 2; udbIndex++,offset++)
        {
            if(debug_print_enable == GT_TRUE)
            {
                cpssOsPrintf("[%s:%d] Configuring VLAN-ID, udbIndex[%d] Offset[%d]\n",
                             __FUNCNAME__, __LINE__, udbIndex, offset);
            }

            rc = cpssDxChPclUserDefinedByteSet(appDemoIpfixDevNum,
                                               CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_30_E,
                                               CPSS_DXCH_PCL_PACKET_TYPE_IPV4_UDP_E,
                                               appDemoIpfixPclDirection,
                                               udbIndex,/* vlan-id */
                                               CPSS_DXCH_PCL_OFFSET_L2_E,
                                               offset);
            if (GT_OK != rc)
            {
                cpssOsPrintf("[%s:%d] cpssDxChPclUserDefinedByteSet FAILED, rc [%d] devNum [%d]\n",
                             __FUNCNAME__, __LINE__, rc, appDemoIpfixDevNum);
                return rc;
            }
        }

        /* UDB[2-5] offset for SIPv4 */
        for(udbIndex = 2, offset = 14 ; udbIndex < 6; udbIndex++,offset++)
        {
            if(debug_print_enable == GT_TRUE)
            {
                cpssOsPrintf("[%s:%d] Configuring SIPv4, udbIndex[%d] Offset[%d]\n",
                             __FUNCNAME__, __LINE__, udbIndex, offset);
            }

            rc = cpssDxChPclUserDefinedByteSet(appDemoIpfixDevNum,
                                               CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_30_E,
                                               CPSS_DXCH_PCL_PACKET_TYPE_IPV4_UDP_E,
                                               appDemoIpfixPclDirection,
                                               udbIndex,
                                               CPSS_DXCH_PCL_OFFSET_L3_MINUS_2_E,
                                               offset);
            if (GT_OK != rc)
            {
                cpssOsPrintf("[%s:%d] cpssDxChPclUserDefinedByteSet FAILED, rc [%d] devNum [%d]\n",
                              __FUNCNAME__, __LINE__, rc, appDemoIpfixDevNum);
                return rc;
            }
        }

        /* UDB[6-9] offset for DIPv4 */
        for(udbIndex = 6, offset = 18 ; udbIndex < 10; udbIndex++,offset++)
        {
            if(debug_print_enable == GT_TRUE)
            {
                cpssOsPrintf("[%s:%d] Configuring DIPv4, udbIndex[%d] Offset[%d]\n",
                             __FUNCNAME__, __LINE__, udbIndex, offset);
            }

            rc = cpssDxChPclUserDefinedByteSet(appDemoIpfixDevNum,
                                               CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_30_E,
                                               CPSS_DXCH_PCL_PACKET_TYPE_IPV4_UDP_E,
                                               appDemoIpfixPclDirection,
                                               udbIndex,
                                               CPSS_DXCH_PCL_OFFSET_L3_MINUS_2_E,
                                               offset);
            if (GT_OK != rc)
            {
                cpssOsPrintf("[%s:%d] cpssDxChPclUserDefinedByteSet FAILED, rc [%d] devNum [%d]\n",
                             __FUNCNAME__, __LINE__, rc, appDemoIpfixDevNum);
                return rc;
            }
        }

        /* UDB[10-11] offset for UDP Src Port */
        for(udbIndex = 10, offset = 0 ; udbIndex < 12; udbIndex++,offset++)
        {
            if(debug_print_enable == GT_TRUE)
            {
                cpssOsPrintf("[%s:%d] Configuring TCP Src Port, udbIndex[%d] Offset[%d]\n",
                             __FUNCNAME__, __LINE__, udbIndex, offset);
            }
            rc = cpssDxChPclUserDefinedByteSet(appDemoIpfixDevNum,
                                               CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_30_E,
                                               CPSS_DXCH_PCL_PACKET_TYPE_IPV4_UDP_E,
                                               appDemoIpfixPclDirection,
                                               udbIndex,
                                               CPSS_DXCH_PCL_OFFSET_L4_E,
                                               offset);
            if (GT_OK != rc)
            {
                cpssOsPrintf("[%s:%d] cpssDxChPclUserDefinedByteSet FAILED, rc [%d] devNum [%d]\n",
                             __FUNCNAME__, __LINE__, rc, appDemoIpfixDevNum);
                return rc;
            }
        }

        /* UDB[12-13] offset for UDP Dst Port */
        for(udbIndex = 12, offset = 2 ; udbIndex < 14; udbIndex++,offset++)
        {
            if(debug_print_enable == GT_TRUE)
            {
                cpssOsPrintf("[%s:%d] Configuring TCP Dst Port, udbIndex[%d] Offset[%d]\n",
                             __FUNCNAME__, __LINE__, udbIndex, offset);
            }
            rc = cpssDxChPclUserDefinedByteSet(appDemoIpfixDevNum,
                                               CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_30_E,
                                               CPSS_DXCH_PCL_PACKET_TYPE_IPV4_UDP_E,
                                               appDemoIpfixPclDirection,
                                               udbIndex,
                                               CPSS_DXCH_PCL_OFFSET_L4_E,
                                               offset);
            if (GT_OK != rc)
            {
                cpssOsPrintf("[%s:%d] cpssDxChPclUserDefinedByteSet FAILED, rc [%d] devNum [%d]\n",
                             __FUNCNAME__, __LINE__, rc, appDemoIpfixDevNum);
                return rc;
            }
        }

        if(appDemoIpfixEnableQinQ == GT_TRUE)
        {
            /* UDB[14-15] offset for Inner VLAN-ID */
            for(udbIndex = 14, offset = 18; udbIndex < 15; udbIndex++,offset++)
            {
                if(debug_print_enable == GT_TRUE)
                {
                    cpssOsPrintf("[%s:%d] Configuring Inner VLAN-ID, udbIndex[%d] Offset[%d]\n",
                        __FUNCNAME__, __LINE__, udbIndex, offset);
                }

                rc = cpssDxChPclUserDefinedByteSet(appDemoIpfixDevNum,
                    CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_30_E,
                    CPSS_DXCH_PCL_PACKET_TYPE_IPV4_UDP_E,
                    appDemoIpfixPclDirection,
                    udbIndex,/* vlan-id */
                    CPSS_DXCH_PCL_OFFSET_L2_E,
                    offset);
                if (GT_OK != rc)
                {
                    cpssOsPrintf("[%s:%d] cpssDxChPclUserDefinedByteSet FAILED, rc [%d] devNum [%d]\n",
                        __FUNCNAME__, __LINE__, rc, appDemoIpfixDevNum);
                    return rc;
                }
            }
        }

        /* it's a must to enable UDB's */
        udbSelectStruct.ingrUdbReplaceArr[0] = GT_TRUE; /* PCL-ID */
        udbSelectStruct.ingrUdbReplaceArr[1] = GT_TRUE; /* PCL-ID */
        udbSelectStruct.udbSelectArr[2]      = 0; /* VLAN-ID */
        udbSelectStruct.udbSelectArr[3]      = 1; /* VLAN-ID */
        udbSelectStruct.ingrUdbReplaceArr[4] = GT_TRUE; /* Source ePort */
        udbSelectStruct.udbSelectArr[5]      = 40; /* Applicable Flow Sub-template */
        udbSelectStruct.udbSelectArr[6]      = 2; /* SIPv4 */
        udbSelectStruct.udbSelectArr[7]      = 3; /* SIPv4 */
        udbSelectStruct.udbSelectArr[8]      = 4; /* SIPv4 */
        udbSelectStruct.udbSelectArr[9]      = 5; /* SIPv4 */
        udbSelectStruct.udbSelectArr[10]     = 6; /* DIPv4 */
        udbSelectStruct.udbSelectArr[11]     = 7; /* DIPv4 */
        udbSelectStruct.udbSelectArr[12]     = 8; /* DIPv4 */
        udbSelectStruct.udbSelectArr[13]     = 9; /* DIPv4 */
        udbSelectStruct.udbSelectArr[14]     = 10; /* UDP Src Port */
        udbSelectStruct.udbSelectArr[15]     = 11; /* UDP Src Port */
        udbSelectStruct.udbSelectArr[16]     = 12; /* UDP Dst Port */
        udbSelectStruct.udbSelectArr[17]     = 13; /* UDP Dst Port */
        if(appDemoIpfixEnableQinQ == GT_TRUE)
        {
            udbSelectStruct.udbSelectArr[18]     = 14; /* Inner VLAN-ID */
            udbSelectStruct.udbSelectArr[19]     = 15; /* Inner VLAN-ID */
        }

        rc = cpssDxChPclUserDefinedBytesSelectSet(appDemoIpfixDevNum,
                                                  CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_30_E,
                                                  CPSS_DXCH_PCL_PACKET_TYPE_IPV4_UDP_E,
                                                  appDemoIpfixLookupNum,
                                                  &udbSelectStruct);
        if (GT_OK != rc)
        {
            cpssOsPrintf("[%s:%d] cpssDxChPclUserDefinedBytesSelectSet FAILED, rc [%d] devNum [%d]\n",
                         __FUNCNAME__, __LINE__, rc, appDemoIpfixDevNum);
            return rc;
        }

        if(appDemoIpfixUdbCfg == NULL)
        {
            appDemoIpfixUdbCfg = (APP_DEMO_IPFIX_UDB_CFG_STC *) cpssOsMalloc(sizeof(APP_DEMO_IPFIX_UDB_CFG_STC));
            if(NULL == appDemoIpfixUdbCfg)
            {
                rc = GT_BAD_STATE;
                cpssOsPrintf("[%s:%d] appDemoIpfixUdbCfg memory allocation FAILED rc[%d]\n",
                             __FUNCNAME__, __LINE__, rc);
                return rc;
            }

            for(keyTableIndex = 0; keyTableIndex < PRV_CPSS_DXCH_KEY_TABLE_INDEX_CNS; keyTableIndex++)
            {
                appDemoIpfixUdbCfg->keyTableUdbCfg[keyTableIndex].ipv4Key   =  NULL;
                appDemoIpfixUdbCfg->keyTableUdbCfg[keyTableIndex].ipv6Key   =  NULL;
                appDemoIpfixUdbCfg->keyTableUdbCfg[keyTableIndex].otherTypeKey =  NULL;
            }

            for(devNum = 0; devNum < APP_DEMO_IPFIX_MAX_PP_DEVICES_CNS; devNum++)
            {
                for(portNum = 0; portNum < APP_DEMO_IPFIX_MAX_PORT_NUM; portNum++)
                {
                    for(pktType = 0; pktType < APP_DEMO_IPFIX_MAX_PKT_TYPE_NUM; pktType++)
                    {
                        appDemoIpfixUdbCfg->keyIndexMapCfg[devNum][portNum].keyIndexArr[pktType] = APP_DEMO_IPFIX_NOT_USED_KEY_TABLE_INDEX;
                    }
                }
            }
        }

        if(appDemoIpfixUdbCfg->keyTableUdbCfg[appDemoIpfixIpv4UdpKeyTableIndex].ipv4Key == NULL)
        {
            appDemoIpfixUdbCfg->keyTableUdbCfg[appDemoIpfixIpv4UdpKeyTableIndex].ipv4Key =
                (PRV_APP_DEMO_IPFIX_IPV4_KEY_FIELDS_INDEX_STC *) cpssOsMalloc(sizeof(PRV_APP_DEMO_IPFIX_IPV4_KEY_FIELDS_INDEX_STC));
            if(NULL == appDemoIpfixUdbCfg->keyTableUdbCfg[appDemoIpfixIpv4UdpKeyTableIndex].ipv4Key)
            {
                rc = GT_BAD_STATE;
                cpssOsPrintf("[%s:%d] ipv4Key memory allocation FAILED rc[%d]\n",
                             __FUNCNAME__, __LINE__, rc);
                return rc;
            }
        }
        cpssOsMemSet(appDemoIpfixUdbCfg->keyTableUdbCfg[appDemoIpfixIpv4UdpKeyTableIndex].ipv4Key,
                APP_DEMO_IPFIX_NOT_USED_UDB, sizeof(PRV_APP_DEMO_IPFIX_IPV4_KEY_FIELDS_INDEX_STC));

        offset = appDemoIpfixPclId;
        mask = 0xFF;
        for(udbIndex = 0, index = 0; udbIndex < 2; udbIndex++, index++)
        {
            if(debug_print_enable == GT_TRUE)
            {
                cpssOsPrintf("[%s:%d] Configuring PCL-ID FlowFieldKey, udbIndex[%d] offset[%d]\n",
                             __FUNCNAME__, __LINE__, udbIndex, offset);
            }

            /* UDB[0-1] offset for PCL-ID */
            rc = cpssDxChFlowManagerKeyEntryUserDefinedByteSet(flowMngId,
                                                               appDemoIpfixIpv4UdpKeyTableIndex,
                                                               appDemoIpfixShortKeySize,
                                                               udbIndex,
                                                               CPSS_DXCH_FLOW_MANAGER_ANCHOR_FIXED_E,/* offset type */
                                                               offset,
                                                               mask);
            if (GT_OK != rc)
            {
                cpssOsPrintf("[%s:%d] cpssDxChFlowManagerKeyEntryUserDefinedByteSet FAILED, rc [%d] devNum [%d]\n",
                             __FUNCNAME__, __LINE__, rc, devNum);
                return rc;
            }

            appDemoIpfixUdbCfg->keyTableUdbCfg[appDemoIpfixIpv4UdpKeyTableIndex].ipv4Key->pclIdIndex[index] = udbIndex;

            offset = 0x0;
            mask = 0x3;
        }

        mask = 0xF;
        /* UDB[2-3] offset for FlowEntry VLAN-ID */
        for(udbIndex = 2, offset = 0, index = 0 ; udbIndex < 4; udbIndex++, offset++, index++)
        {
            if(debug_print_enable == GT_TRUE)
            {
                cpssOsPrintf("[%s:%d] Configuring VLAN-ID FlowFieldKey, udbIndex[%d] offset[%d]\n",
                             __FUNCNAME__, __LINE__, udbIndex, offset);
            }

            rc = cpssDxChFlowManagerKeyEntryUserDefinedByteSet(flowMngId,
                                                               appDemoIpfixIpv4UdpKeyTableIndex,
                                                               appDemoIpfixShortKeySize,
                                                               udbIndex,
                                                               CPSS_DXCH_FLOW_MANAGER_ANCHOR_VID_E,/* offset type */
                                                               offset,
                                                               mask);
            if (GT_OK != rc)
            {
                cpssOsPrintf("[%s:%d] cpssDxChFlowManagerKeyEntryUserDefinedByteSet FAILED, rc [%d] devNum [%d]\n",
                        __FUNCNAME__, __LINE__, rc, appDemoIpfixDevNum);
                return rc;
            }
            appDemoIpfixUdbCfg->keyTableUdbCfg[appDemoIpfixIpv4UdpKeyTableIndex].ipv4Key->vlanIdIndex[index] = udbIndex;
            mask = 0xFF;
        }

        udbIndex = 4;
        offset = 0;
        mask = 0xFF;

        if(debug_print_enable == GT_TRUE)
        {
            cpssOsPrintf("[%s:%d] Configuring SourcePort FlowFieldKey, udbIndex[%d] offset[%d]\n",
                         __FUNCNAME__, __LINE__, udbIndex, offset);
        }

        /* UDB[4] offset for FlowEntry SourcePort */
        rc = cpssDxChFlowManagerKeyEntryUserDefinedByteSet(flowMngId,
                                                           appDemoIpfixIpv4UdpKeyTableIndex,
                                                           appDemoIpfixShortKeySize,
                                                           udbIndex,
                                                           CPSS_DXCH_FLOW_MANAGER_ANCHOR_SRC_PORT_E,/* offset type */
                                                           offset,
                                                           mask);
        if (GT_OK != rc)
        {
            cpssOsPrintf("[%s:%d] cpssDxChFlowManagerKeyEntryUserDefinedByteSet FAILED, rc [%d] devNum [%d]\n",
                         __FUNCNAME__, __LINE__, rc, appDemoIpfixDevNum);
            return rc;
        }
        appDemoIpfixUdbCfg->keyTableUdbCfg[appDemoIpfixIpv4UdpKeyTableIndex].ipv4Key->srcPortIndex = udbIndex;

        udbIndex = 5;
        offset = 0x2;

        if(debug_print_enable == GT_TRUE)
        {
            cpssOsPrintf("[%s:%d] Configuring Applicable Flow Sub-template FlowFieldKey,"
                         "udbIndex[%d] offset[%d]\n",__FUNCNAME__, __LINE__,
                         udbIndex, offset);
        }

        /* UDB[5] offset for FlowEntry Applicable Flow Sub-template */
        rc = cpssDxChFlowManagerKeyEntryUserDefinedByteSet(flowMngId,
                                                           appDemoIpfixIpv4UdpKeyTableIndex,
                                                           appDemoIpfixShortKeySize,
                                                           udbIndex,
                                                           CPSS_DXCH_FLOW_MANAGER_ANCHOR_FIXED_E,/* offset type */
                                                           offset,
                                                           mask);
        if (GT_OK != rc)
        {
            cpssOsPrintf("[%s:%d] cpssDxChFlowManagerKeyEntryUserDefinedByteSet FAILED, rc [%d] devNum [%d]\n",
                         __FUNCNAME__, __LINE__, rc, appDemoIpfixDevNum);
            return rc;
        }
        appDemoIpfixUdbCfg->keyTableUdbCfg[appDemoIpfixIpv4UdpKeyTableIndex].ipv4Key->appFlowSubTempIndex = udbIndex;

        /* UDB[6-9] offset for FlowEntry SIPv4 */
        for(udbIndex = 6, offset = 14 , index = 0; udbIndex < 10; udbIndex++, offset++, index++)
        {
            if(debug_print_enable == GT_TRUE)
            {
                cpssOsPrintf("[%s:%d] Configuring SIPv4 FlowFieldKey, udbIndex[%d] offset[%d]\n",
                             __FUNCNAME__, __LINE__, udbIndex, offset);
            }

            rc = cpssDxChFlowManagerKeyEntryUserDefinedByteSet(flowMngId,
                                                               appDemoIpfixIpv4UdpKeyTableIndex,
                                                               appDemoIpfixShortKeySize,
                                                               udbIndex,
                                                               CPSS_DXCH_FLOW_MANAGER_ANCHOR_L3_E,/* offset type */
                                                               offset,
                                                               mask);
            if (GT_OK != rc)
            {
                cpssOsPrintf("[%s:%d] cpssDxChFlowManagerKeyEntryUserDefinedByteSet FAILED, rc [%d] devNum [%d]\n",
                             __FUNCNAME__, __LINE__, rc, appDemoIpfixDevNum);
                return rc;
            }
            appDemoIpfixUdbCfg->keyTableUdbCfg[appDemoIpfixIpv4UdpKeyTableIndex].ipv4Key->sipv4Index[index] = udbIndex;
        }

        /* UDB[10-13] offset for FlowEntry DIPv4 */
        for(udbIndex = 10, offset = 18 ,index = 0; udbIndex < 14; udbIndex++, offset++, index++)
        {
            if(debug_print_enable == GT_TRUE)
            {
                cpssOsPrintf("[%s:%d] Configuring DIPv4 FlowFieldKey, udbIndex[%d] offset[%d]\n",
                               __FUNCNAME__, __LINE__, udbIndex, offset);
            }

            rc = cpssDxChFlowManagerKeyEntryUserDefinedByteSet(flowMngId,
                                                               appDemoIpfixIpv4UdpKeyTableIndex,
                                                               appDemoIpfixShortKeySize,
                                                               udbIndex,
                                                               CPSS_DXCH_FLOW_MANAGER_ANCHOR_L3_E,/* offset type */
                                                               offset,
                                                               mask);
            if (GT_OK != rc)
            {
                cpssOsPrintf("[%s:%d] cpssDxChFlowManagerKeyEntryUserDefinedByteSet FAILED, rc [%d] devNum [%d]\n",
                             __FUNCNAME__, __LINE__, rc, appDemoIpfixDevNum);
                return rc;
            }
            appDemoIpfixUdbCfg->keyTableUdbCfg[appDemoIpfixIpv4UdpKeyTableIndex].ipv4Key->dipv4Index[index] = udbIndex;
        }

        appDemoIpfixUdbCfg->keyTableUdbCfg[appDemoIpfixIpv4UdpKeyTableIndex].ipv4Key->pktType = APP_DEMO_IPFIX_PACKET_TYPE_IPV4_UDP_E;

        /* UDB[14-15] offset for FlowEntry UDP Src Port */
        for(udbIndex = 14, offset = 0 , index = 0; udbIndex < 16; udbIndex++, offset++, index++)
        {
            if(debug_print_enable == GT_TRUE)
            {
                cpssOsPrintf("[%s:%d] Configuring UDP Src Port FlowFieldKey,"
                             "udbIndex[%d] offset[%d]\n", __FUNCNAME__, __LINE__, udbIndex, offset);
            }

            rc = cpssDxChFlowManagerKeyEntryUserDefinedByteSet(flowMngId,
                                                               appDemoIpfixIpv4UdpKeyTableIndex,
                                                               appDemoIpfixShortKeySize,
                                                               udbIndex,
                                                               CPSS_DXCH_FLOW_MANAGER_ANCHOR_L4_E,/* offset type */
                                                               offset,
                                                               mask);
            if (GT_OK != rc)
            {
                cpssOsPrintf("[%s:%d] cpssDxChFlowManagerKeyEntryUserDefinedByteSet FAILED, rc [%d] devNum [%d]\n",
                             __FUNCNAME__, __LINE__, rc, appDemoIpfixDevNum);
            }
            appDemoIpfixUdbCfg->keyTableUdbCfg[appDemoIpfixIpv4UdpKeyTableIndex].ipv4Key->protocol.protoSrcPortIndex[index] = udbIndex;
        }

        /* UDB[16-17] offset for FlowEntry UDP Dst Port */
        for(udbIndex = 16, offset = 2 , index = 0; udbIndex < 18; udbIndex++, offset++, index++)
        {
            if(debug_print_enable ==  GT_TRUE)
            {
                cpssOsPrintf("[%s:%d] Configuring UDP Dst Port FlowFieldKey,"
                             "udbIndex[%d] offset[%d]\n", __FUNCNAME__, __LINE__, udbIndex, offset);
            }

            rc = cpssDxChFlowManagerKeyEntryUserDefinedByteSet(flowMngId,
                                                               appDemoIpfixIpv4UdpKeyTableIndex,
                                                               appDemoIpfixShortKeySize,
                                                               udbIndex,
                                                               CPSS_DXCH_FLOW_MANAGER_ANCHOR_L4_E,/* offset type */
                                                               offset,
                                                               mask);
            if (GT_OK != rc)
            {
                cpssOsPrintf("[%s:%d] cpssDxChFlowManagerKeyEntryUserDefinedByteSet FAILED, rc [%d] devNum [%d]\n",
                             __FUNCNAME__, __LINE__, rc, appDemoIpfixDevNum);
            }
            appDemoIpfixUdbCfg->keyTableUdbCfg[appDemoIpfixIpv4UdpKeyTableIndex].ipv4Key->protocol.protoDstPortIndex[index] = udbIndex;
        }

        if(appDemoIpfixEnableQinQ == GT_TRUE)
        {
            mask = 0xF;
            /* UDB[18-19] offset for FlowEntry Inner VLAN-ID */
            for(udbIndex = 18, offset = 18, index = 18; udbIndex < 20; udbIndex++, offset++, index++)
            {
                if(debug_print_enable == GT_TRUE)
                {
                    cpssOsPrintf("[%s:%d] Configuring Inner VLAN-ID FlowFieldKey, udbIndex[%d] offset[%d]\n",
                        __FUNCNAME__, __LINE__, udbIndex, offset);
                }

                rc = cpssDxChFlowManagerKeyEntryUserDefinedByteSet(flowMngId,
                    appDemoIpfixIpv4UdpKeyTableIndex,
                    appDemoIpfixShortKeySize,
                    udbIndex,
                    CPSS_DXCH_FLOW_MANAGER_ANCHOR_L2_E,/* offset type */
                    offset,
                    mask);
                if (GT_OK != rc)
                {
                    cpssOsPrintf("[%s:%d] cpssDxChFlowManagerKeyEntryUserDefinedByteSet FAILED, rc [%d] devNum [%d]\n",
                        __FUNCNAME__, __LINE__, rc, appDemoIpfixDevNum);
                    return rc;
                }
                mask = 0xFF;
            }
            udbIndexQinq = 20;
        }

        /* UDB[18-29 or 20-29 for qinq] offset for NOT_VALID FlowEntry */
        for(udbIndex = udbIndexQinq, offset = 0 , index = 0; udbIndex < 30; udbIndex++, offset++, index++)
        {
            if(debug_print_enable == GT_TRUE)
            {
                cpssOsPrintf("[%s:%d] Configuring 18-29 FlowFieldKey as NOT_VALID,"
                             "udbIndex[%d] offset[%d]\n", __FUNCNAME__, __LINE__, udbIndex, offset);
            }
            rc = cpssDxChFlowManagerKeyEntryUserDefinedByteSet(flowMngId,
                                                               appDemoIpfixIpv4UdpKeyTableIndex,
                                                               appDemoIpfixShortKeySize,
                                                               udbIndex,
                                                               CPSS_DXCH_FLOW_MANAGER_ANCHOR_NOT_VALID_E,/* offset type */
                                                               offset,
                                                               mask);
            if (GT_OK != rc)
            {
                cpssOsPrintf("[%s:%d] cpssDxChFlowManagerKeyEntryUserDefinedByteSet FAILED, rc [%d] devNum [%d]\n",
                             __FUNCNAME__, __LINE__, rc, appDemoIpfixDevNum);
            }
            appDemoIpfixUdbCfg->keyTableUdbCfg[appDemoIpfixIpv4UdpKeyTableIndex].ipv4Key->notValidIndex[index] = udbIndex;
        }

        /* Incrementing the udb configured count */
        appDemoIpfixTotalUdbConfigCount++;

    }
    else {

        if(appDemoIpfixUdbCfg == NULL)
        {
            if (appDemoIpfixTotalUdbConfigCount == 0)
            {
                rc = GT_OK;
                if(debug_print_enable)
                {
                    cpssOsPrintf("[%s:%d] No UDBs configured\n", __FUNCNAME__, __LINE__);
                }
            }
            else
            {
                rc = GT_BAD_STATE;
                cpssOsPrintf("[%s:%d] Retrieving appDemoIpfixUdbCfg  FAILED rc[%d]\n",
                             __FUNCNAME__, __LINE__, rc);
            }
            return rc;
        }

        for(keyTableIndex = 0; keyTableIndex < APP_DEMO_IPFIX_KEY_TABLE_INDEX_CNS; keyTableIndex++)
        {
            if(appDemoIpfixUdbCfg->keyTableUdbCfg[keyTableIndex].ipv4Key != NULL)
            {
                cpssOsFree(appDemoIpfixUdbCfg->keyTableUdbCfg[keyTableIndex].ipv4Key);
                appDemoIpfixUdbCfg->keyTableUdbCfg[keyTableIndex].ipv4Key = NULL;

                /* Decrementing the udb configured count */
                appDemoIpfixTotalUdbConfigCount--;
            }
        }

        /* Clearing UDB config */
        rc = cpssDxChPclUserDefinedBytesSelectSet(appDemoIpfixDevNum,
                                                  CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_30_E,
                                                  CPSS_DXCH_PCL_PACKET_TYPE_IPV4_UDP_E,
                                                  appDemoIpfixLookupNum,
                                                  &udbSelectStruct);
        if (GT_OK != rc)
        {
            cpssOsPrintf("[%s:%d] cpssDxChPclUserDefinedBytesSelectSet FAILED, rc [%d] devNum [%d]\n",
                         __FUNCNAME__, __LINE__, rc, appDemoIpfixDevNum);
            return rc;
        }
    }

    return GT_OK;
}

/* Function to be used to enable all key fields for OTHER pkt type */
GT_STATUS appDemoIpfixOtherFlowFieldConfig
(
    IN  GT_U32                                 flowMngId,
    IN  GT_BOOL                                enable
)
{
    CPSS_DXCH_PCL_UDB_SELECT_STC   udbSelectStruct;

    GT_U8                          offset = 0; /* UDB offset from anchor */
    GT_U8                          devNum = 0;
    GT_U8                          portNum = 0;
    GT_U8                          pktType = 0;
    GT_U32                         udbIndex = 0;
    GT_U32                         index = 0; /* UDB index */
    GT_U32                         mask = 0;
    GT_U32                         keyTableIndex = 0;
    GT_U32                         udbIndexQinq = 18;
    GT_STATUS                      rc = GT_OK;
    GT_U32                         appDemoIpfixDevNum = prvWrAppFirstDevNum(0);

    cpssOsMemSet(&udbSelectStruct, 0, sizeof(udbSelectStruct));

    if(debug_print_enable == GT_TRUE)
    {
        cpssOsPrintf("[%s:%d] appDemoIpfixOtherFlowFieldConfig entered\n",__FUNCNAME__, __LINE__);
    }

    /* Before setting flow classification for OTHER packet, it is necessary
     * to validate whether the flow id exists or not else there will be
     * unnecesary calls to set not useful pcl udb keys.
     */
    if (appDemoIpfixFlowMngId == APP_DEMO_IPFIX_FLOW_MANAGER_ID_NOT_VALID ||
        appDemoIpfixFlowMngId != flowMngId)
    {
        rc = GT_FAIL;
        if(debug_print_enable == GT_TRUE)
        {
            cpssOsPrintf("[%s:%d] Flow manager ID doesn't exist rc[%d]\n",
                __FUNCNAME__, __LINE__, rc);
        }
        return rc;
    }

    if(enable == GT_TRUE)
    {
        /* UDB[40] offset for Applicable Flow Sub-template */
        udbIndex = 40;
        offset = 91;

        if(debug_print_enable == GT_TRUE)
        {
            cpssOsPrintf("[%s:%d] Configuring Applicable Flow Sub-template,"
                         "udbIndex[%d] Offset[%d]\n",__FUNCNAME__, __LINE__, udbIndex, offset);
        }

        rc = cpssDxChPclUserDefinedByteSet(appDemoIpfixDevNum,
                                           CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_30_E,
                                           CPSS_DXCH_PCL_PACKET_TYPE_ETHERNET_OTHER_E,
                                           appDemoIpfixPclDirection,
                                           udbIndex,
                                           CPSS_DXCH_PCL_OFFSET_METADATA_E,
                                           offset /* Applicable Flow Sub-template */);
        if (GT_OK != rc)
        {
            cpssOsPrintf("[%s:%d] cpssDxChPclUserDefinedByteSet FAILED, rc [%d] devNum [%d]\n",
                         __FUNCNAME__, __LINE__, rc, appDemoIpfixDevNum);
            return rc;
        }

        /* UDB[0-1] offset for VLAN-ID */
        for(udbIndex = 0, offset = 14 ; udbIndex < 2; udbIndex++,offset++)
        {
            if(debug_print_enable == GT_TRUE)
            {
                cpssOsPrintf("[%s:%d] Configuring VLAN-ID, udbIndex[%d] Offset[%d]\n",
                             __FUNCNAME__, __LINE__, udbIndex, offset);
            }

            rc = cpssDxChPclUserDefinedByteSet(appDemoIpfixDevNum,
                                               CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_30_E,
                                               CPSS_DXCH_PCL_PACKET_TYPE_ETHERNET_OTHER_E,
                                               appDemoIpfixPclDirection,
                                               udbIndex,/* vlan-id */
                                               CPSS_DXCH_PCL_OFFSET_L2_E,
                                               offset);
            if (GT_OK != rc)
            {
                cpssOsPrintf("[%s:%d] cpssDxChPclUserDefinedByteSet FAILED, rc [%d] devNum [%d]\n",
                             __FUNCNAME__, __LINE__, rc, appDemoIpfixDevNum);
                return rc;
            }
        }

        /* UDB[2-7] offset for SMAC */
        for(udbIndex = 2, offset = 6 ; udbIndex < 8; udbIndex++,offset++)
        {
            if(debug_print_enable == GT_TRUE)
            {
                cpssOsPrintf("[%s:%d] Configuring SMAC, udbIndex[%d] Offset[%d]\n",
                             __FUNCNAME__, __LINE__, udbIndex, offset);
            }

            rc = cpssDxChPclUserDefinedByteSet(appDemoIpfixDevNum,
                                               CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_30_E,
                                               CPSS_DXCH_PCL_PACKET_TYPE_ETHERNET_OTHER_E,
                                               appDemoIpfixPclDirection,
                                               udbIndex,
                                               CPSS_DXCH_PCL_OFFSET_L2_E,
                                               offset);
            if (GT_OK != rc)
            {
                cpssOsPrintf("[%s:%d] cpssDxChPclUserDefinedByteSet FAILED, rc [%d] devNum [%d]\n",
                              __FUNCNAME__, __LINE__, rc, appDemoIpfixDevNum);
                return rc;
            }
        }

        /* UDB[8-13] offset for DMAC */
        for(udbIndex = 8, offset = 0 ; udbIndex < 14; udbIndex++,offset++)
        {
            if(debug_print_enable == GT_TRUE)
            {
                cpssOsPrintf("[%s:%d] Configuring DMAC, udbIndex[%d] Offset[%d]\n",
                             __FUNCNAME__, __LINE__, udbIndex, offset);
            }

            rc = cpssDxChPclUserDefinedByteSet(appDemoIpfixDevNum,
                                               CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_30_E,
                                               CPSS_DXCH_PCL_PACKET_TYPE_ETHERNET_OTHER_E,
                                               appDemoIpfixPclDirection,
                                               udbIndex,
                                               CPSS_DXCH_PCL_OFFSET_L2_E,
                                               offset);
            if (GT_OK != rc)
            {
                cpssOsPrintf("[%s:%d] cpssDxChPclUserDefinedByteSet FAILED, rc [%d] devNum [%d]\n",
                             __FUNCNAME__, __LINE__, rc, appDemoIpfixDevNum);
                return rc;
            }
        }

        if(appDemoIpfixEnableQinQ == GT_TRUE)
        {
            /* UDB[14-15] offset for VLAN-ID */
            for(udbIndex = 14, offset = 18 ; udbIndex < 16; udbIndex++,offset++)
            {
                if(debug_print_enable == GT_TRUE)
                {
                    cpssOsPrintf("[%s:%d] Configuring VLAN-ID, udbIndex[%d] Offset[%d]\n",
                        __FUNCNAME__, __LINE__, udbIndex, offset);
                }

                rc = cpssDxChPclUserDefinedByteSet(appDemoIpfixDevNum,
                    CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_30_E,
                    CPSS_DXCH_PCL_PACKET_TYPE_ETHERNET_OTHER_E,
                    appDemoIpfixPclDirection,
                    udbIndex,/* vlan-id */
                    CPSS_DXCH_PCL_OFFSET_L2_E,
                    offset);
                if (GT_OK != rc)
                {
                    cpssOsPrintf("[%s:%d] cpssDxChPclUserDefinedByteSet FAILED, rc [%d] devNum [%d]\n",
                        __FUNCNAME__, __LINE__, rc, appDemoIpfixDevNum);
                    return rc;
                }
            }
        }

        /* it's a must to enable UDB's */
        udbSelectStruct.ingrUdbReplaceArr[0] = GT_TRUE; /* PCL-ID */
        udbSelectStruct.ingrUdbReplaceArr[1] = GT_TRUE; /* PCL-ID */
        udbSelectStruct.udbSelectArr[2]      = 0; /* VLAN-ID */
        udbSelectStruct.udbSelectArr[3]      = 1; /* VLAN-ID */
        udbSelectStruct.ingrUdbReplaceArr[4] = GT_TRUE; /* Source ePort */
        udbSelectStruct.udbSelectArr[5]      = 40; /* Applicable Flow Sub-template */
        udbSelectStruct.udbSelectArr[6]      = 2; /* SMAC */
        udbSelectStruct.udbSelectArr[7]      = 3; /* SMAC */
        udbSelectStruct.udbSelectArr[8]      = 4; /* SMAC */
        udbSelectStruct.udbSelectArr[9]      = 5; /* SMAC */
        udbSelectStruct.udbSelectArr[10]     = 6; /* SMAC */
        udbSelectStruct.udbSelectArr[11]     = 7; /* SMAC */
        udbSelectStruct.udbSelectArr[12]     = 8; /* DMAC */
        udbSelectStruct.udbSelectArr[13]     = 9; /* DMAC */
        udbSelectStruct.udbSelectArr[14]     = 10; /* DMAC */
        udbSelectStruct.udbSelectArr[15]     = 11; /* DMAC */
        udbSelectStruct.udbSelectArr[16]     = 12; /* DMAC */
        udbSelectStruct.udbSelectArr[17]     = 13; /* DMAC */

        if(appDemoIpfixEnableQinQ == GT_TRUE)
        {
            udbSelectStruct.udbSelectArr[18]     = 14; /* Inner VLAN-ID */
            udbSelectStruct.udbSelectArr[19]     = 15; /* Inner VLAN-ID */
        }

        rc = cpssDxChPclUserDefinedBytesSelectSet(appDemoIpfixDevNum,
                                                  CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_30_E,
                                                  CPSS_DXCH_PCL_PACKET_TYPE_ETHERNET_OTHER_E,
                                                  appDemoIpfixLookupNum,
                                                  &udbSelectStruct);
        if (GT_OK != rc)
        {
            cpssOsPrintf("[%s:%d] cpssDxChPclUserDefinedBytesSelectSet FAILED, rc [%d] devNum [%d]\n",
                         __FUNCNAME__, __LINE__, rc, appDemoIpfixDevNum);
            return rc;
        }

        if(appDemoIpfixUdbCfg == NULL)
        {
            appDemoIpfixUdbCfg = (APP_DEMO_IPFIX_UDB_CFG_STC *) cpssOsMalloc(sizeof(APP_DEMO_IPFIX_UDB_CFG_STC));
            if(NULL == appDemoIpfixUdbCfg)
            {
                rc = GT_BAD_STATE;
                cpssOsPrintf("[%s:%d] appDemoIpfixUdbCfg memory allocation FAILED rc[%d]\n",
                             __FUNCNAME__, __LINE__, rc);
                return rc;
            }

            for(keyTableIndex = 0; keyTableIndex < PRV_CPSS_DXCH_KEY_TABLE_INDEX_CNS; keyTableIndex++)
            {
                appDemoIpfixUdbCfg->keyTableUdbCfg[keyTableIndex].ipv4Key   =  NULL;
                appDemoIpfixUdbCfg->keyTableUdbCfg[keyTableIndex].ipv6Key   =  NULL;
                appDemoIpfixUdbCfg->keyTableUdbCfg[keyTableIndex].otherTypeKey =  NULL;
            }

            for(devNum = 0; devNum < APP_DEMO_IPFIX_MAX_PP_DEVICES_CNS; devNum++)
            {
                for(portNum = 0; portNum < APP_DEMO_IPFIX_MAX_PORT_NUM; portNum++)
                {
                    for(pktType = 0; pktType < APP_DEMO_IPFIX_MAX_PKT_TYPE_NUM; pktType++)
                    {
                        appDemoIpfixUdbCfg->keyIndexMapCfg[devNum][portNum].keyIndexArr[pktType]
                                                = APP_DEMO_IPFIX_NOT_USED_KEY_TABLE_INDEX;
                    }
                }
            }
        }

        if(appDemoIpfixUdbCfg->keyTableUdbCfg[appDemoIpfixOtherKeyTableIndex].otherTypeKey == NULL)
        {
            appDemoIpfixUdbCfg->keyTableUdbCfg[appDemoIpfixOtherKeyTableIndex].otherTypeKey =
                (PRV_APP_DEMO_IPFIX_OTHER_KEY_FIELDS_INDEX_STC *)
                cpssOsMalloc(sizeof(PRV_APP_DEMO_IPFIX_OTHER_KEY_FIELDS_INDEX_STC));
            if(NULL == appDemoIpfixUdbCfg->keyTableUdbCfg[appDemoIpfixOtherKeyTableIndex].otherTypeKey)
            {
                rc = GT_BAD_STATE;
                cpssOsPrintf("[%s:%d] otherTypeKey memory allocation FAILED rc[%d]\n",
                             __FUNCNAME__, __LINE__, rc);
                return rc;
            }
        }
        cpssOsMemSet(appDemoIpfixUdbCfg->keyTableUdbCfg[appDemoIpfixOtherKeyTableIndex].otherTypeKey,
                APP_DEMO_IPFIX_NOT_USED_UDB, sizeof(PRV_APP_DEMO_IPFIX_OTHER_KEY_FIELDS_INDEX_STC));

        offset = appDemoIpfixPclId;
        mask = 0xFF;
        for(udbIndex = 0, index = 0; udbIndex < 2; udbIndex++, index++)
        {
            if(debug_print_enable == GT_TRUE)
            {
                cpssOsPrintf("[%s:%d] Configuring PCL-ID FlowFieldKey, udbIndex[%d] offset[%d]\n",
                             __FUNCNAME__, __LINE__, udbIndex, offset);
            }

            /* UDB[0-1] offset for PCL-ID */
            rc = cpssDxChFlowManagerKeyEntryUserDefinedByteSet(flowMngId,
                                                               appDemoIpfixOtherKeyTableIndex,
                                                               appDemoIpfixShortKeySize,
                                                               udbIndex,
                                                               CPSS_DXCH_FLOW_MANAGER_ANCHOR_FIXED_E,/* offset type */
                                                               offset,
                                                               mask);
            if (GT_OK != rc)
            {
                cpssOsPrintf("[%s:%d] cpssDxChFlowManagerKeyEntryUserDefinedByteSet FAILED, rc [%d] devNum [%d]\n",
                             __FUNCNAME__, __LINE__, rc, devNum);
                return rc;
            }

            appDemoIpfixUdbCfg->keyTableUdbCfg[appDemoIpfixOtherKeyTableIndex].otherTypeKey->pclIdIndex[index] = udbIndex;

            offset = 0x0;
            mask = 0x3;
        }

        mask = 0xF;
        /* UDB[2-3] offset for FlowEntry VLAN-ID */
        for(udbIndex = 2, offset = 0, index = 0 ; udbIndex < 4; udbIndex++, offset++, index++)
        {
            if(debug_print_enable == GT_TRUE)
            {
                cpssOsPrintf("[%s:%d] Configuring VLAN-ID FlowFieldKey, udbIndex[%d] offset[%d]\n",
                             __FUNCNAME__, __LINE__, udbIndex, offset);
            }

            rc = cpssDxChFlowManagerKeyEntryUserDefinedByteSet(flowMngId,
                                                               appDemoIpfixOtherKeyTableIndex,
                                                               appDemoIpfixShortKeySize,
                                                               udbIndex,
                                                               CPSS_DXCH_FLOW_MANAGER_ANCHOR_VID_E,/* offset type */
                                                               offset,
                                                               mask);
            if (GT_OK != rc)
            {
                cpssOsPrintf("[%s:%d] cpssDxChFlowManagerKeyEntryUserDefinedByteSet FAILED, rc [%d] devNum [%d]\n",
                        __FUNCNAME__, __LINE__, rc, appDemoIpfixDevNum);
                return rc;
            }
            appDemoIpfixUdbCfg->keyTableUdbCfg[appDemoIpfixOtherKeyTableIndex].otherTypeKey->vlanIdIndex[index] = udbIndex;
            mask = 0xFF;
        }

        udbIndex = 4;
        offset = 0;
        mask = 0xFF;

        if(debug_print_enable == GT_TRUE)
        {
            cpssOsPrintf("[%s:%d] Configuring SourcePort FlowFieldKey, udbIndex[%d] offset[%d]\n",
                         __FUNCNAME__, __LINE__, udbIndex, offset);
        }

        /* UDB[4] offset for FlowEntry SourcePort */
        rc = cpssDxChFlowManagerKeyEntryUserDefinedByteSet(flowMngId,
                                                           appDemoIpfixOtherKeyTableIndex,
                                                           appDemoIpfixShortKeySize,
                                                           udbIndex,
                                                           CPSS_DXCH_FLOW_MANAGER_ANCHOR_SRC_PORT_E,/* offset type */
                                                           offset,
                                                           mask);
        if (GT_OK != rc)
        {
            cpssOsPrintf("[%s:%d] cpssDxChFlowManagerKeyEntryUserDefinedByteSet FAILED, rc [%d] devNum [%d]\n",
                         __FUNCNAME__, __LINE__, rc, appDemoIpfixDevNum);
            return rc;
        }
        appDemoIpfixUdbCfg->keyTableUdbCfg[appDemoIpfixOtherKeyTableIndex].otherTypeKey->srcPortIndex = udbIndex;

        udbIndex = 5;
        offset = 0x20;

        if(debug_print_enable == GT_TRUE)
        {
            cpssOsPrintf("[%s:%d] Configuring Applicable Flow Sub-template FlowFieldKey,"
                         "udbIndex[%d] offset[%d]\n",__FUNCNAME__, __LINE__,
                         udbIndex, offset);
        }

        /* UDB[5] offset for FlowEntry Applicable Flow Sub-template */
        rc = cpssDxChFlowManagerKeyEntryUserDefinedByteSet(flowMngId,
                                                           appDemoIpfixOtherKeyTableIndex,
                                                           appDemoIpfixShortKeySize,
                                                           udbIndex,
                                                           CPSS_DXCH_FLOW_MANAGER_ANCHOR_FIXED_E,/* offset type */
                                                           offset,
                                                           mask);
        if (GT_OK != rc)
        {
            cpssOsPrintf("[%s:%d] cpssDxChFlowManagerKeyEntryUserDefinedByteSet FAILED, rc [%d] devNum [%d]\n",
                         __FUNCNAME__, __LINE__, rc, appDemoIpfixDevNum);
            return rc;
        }
        appDemoIpfixUdbCfg->keyTableUdbCfg[appDemoIpfixOtherKeyTableIndex].otherTypeKey->appFlowSubTempIndex = udbIndex;

        /* UDB[6-11] offset for FlowEntry SMAC */
        for(udbIndex = 6, offset = 6 , index = 0; udbIndex < 12; udbIndex++, offset++, index++)
        {
            if(debug_print_enable == GT_TRUE)
            {
                cpssOsPrintf("[%s:%d] Configuring SMAC FlowFieldKey, udbIndex[%d] offset[%d]\n",
                             __FUNCNAME__, __LINE__, udbIndex, offset);
            }

            rc = cpssDxChFlowManagerKeyEntryUserDefinedByteSet(flowMngId,
                                                               appDemoIpfixOtherKeyTableIndex,
                                                               appDemoIpfixShortKeySize,
                                                               udbIndex,
                                                               CPSS_DXCH_FLOW_MANAGER_ANCHOR_L2_E,/* offset type */
                                                               offset,
                                                               mask);
            if (GT_OK != rc)
            {
                cpssOsPrintf("[%s:%d] cpssDxChFlowManagerKeyEntryUserDefinedByteSet FAILED, rc [%d] devNum [%d]\n",
                             __FUNCNAME__, __LINE__, rc, appDemoIpfixDevNum);
                return rc;
            }
            appDemoIpfixUdbCfg->keyTableUdbCfg[appDemoIpfixOtherKeyTableIndex].otherTypeKey->smacIndex[index] = udbIndex;
        }

        /* UDB[12-17] offset for FlowEntry DIPv4 */
        for(udbIndex = 12, offset = 0 ,index = 0; udbIndex < 18; udbIndex++, offset++, index++)
        {
            if(debug_print_enable == GT_TRUE)
            {
                cpssOsPrintf("[%s:%d] Configuring DMAC FlowFieldKey, udbIndex[%d] offset[%d]\n",
                               __FUNCNAME__, __LINE__, udbIndex, offset);
            }

            rc = cpssDxChFlowManagerKeyEntryUserDefinedByteSet(flowMngId,
                                                               appDemoIpfixOtherKeyTableIndex,
                                                               appDemoIpfixShortKeySize,
                                                               udbIndex,
                                                               CPSS_DXCH_FLOW_MANAGER_ANCHOR_L2_E,/* offset type */
                                                               offset,
                                                               mask);
            if (GT_OK != rc)
            {
                cpssOsPrintf("[%s:%d] cpssDxChFlowManagerKeyEntryUserDefinedByteSet FAILED, rc [%d] devNum [%d]\n",
                             __FUNCNAME__, __LINE__, rc, appDemoIpfixDevNum);
                return rc;
            }
            appDemoIpfixUdbCfg->keyTableUdbCfg[appDemoIpfixOtherKeyTableIndex].otherTypeKey->dmacIndex[index] = udbIndex;
        }

        if(appDemoIpfixEnableQinQ == GT_TRUE)
        {
            mask = 0xF;
            /* UDB[18-19] offset for FlowEntry Inner VLAN-ID */
            for(udbIndex = 18, offset = 18, index = 18; udbIndex < 20; udbIndex++, offset++, index++)
            {
                if(debug_print_enable == GT_TRUE)
                {
                    cpssOsPrintf("[%s:%d] Configuring Inner VLAN-ID FlowFieldKey, udbIndex[%d] offset[%d]\n",
                        __FUNCNAME__, __LINE__, udbIndex, offset);
                }

                rc = cpssDxChFlowManagerKeyEntryUserDefinedByteSet(flowMngId,
                    appDemoIpfixOtherKeyTableIndex,
                    appDemoIpfixShortKeySize,
                    udbIndex,
                    CPSS_DXCH_FLOW_MANAGER_ANCHOR_L2_E,/* offset type */
                    offset,
                    mask);
                if (GT_OK != rc)
                {
                    cpssOsPrintf("[%s:%d] cpssDxChFlowManagerKeyEntryUserDefinedByteSet FAILED, rc [%d] devNum [%d]\n",
                        __FUNCNAME__, __LINE__, rc, appDemoIpfixDevNum);
                    return rc;
                }
                mask = 0xFF;
            }
            udbIndexQinq = 20;
        }

        /* UDB[18-29 or 20-29 for qinq] offset for NOT_VALID FlowEntry */
        for(udbIndex = udbIndexQinq, offset = 0 , index = 0; udbIndex < 30; udbIndex++, offset++, index++)
        {
            if(debug_print_enable == GT_TRUE)
            {
                cpssOsPrintf("[%s:%d] Configuring 18-29 FlowFieldKey as NOT_VALID,"
                             "udbIndex[%d] offset[%d]\n", __FUNCNAME__, __LINE__, udbIndex, offset);
            }
            rc = cpssDxChFlowManagerKeyEntryUserDefinedByteSet(flowMngId,
                                                               appDemoIpfixOtherKeyTableIndex,
                                                               appDemoIpfixShortKeySize,
                                                               udbIndex,
                                                               CPSS_DXCH_FLOW_MANAGER_ANCHOR_NOT_VALID_E,/* offset type */
                                                               offset,
                                                               mask);
            if (GT_OK != rc)
            {
                cpssOsPrintf("[%s:%d] cpssDxChFlowManagerKeyEntryUserDefinedByteSet FAILED, rc [%d] devNum [%d]\n",
                             __FUNCNAME__, __LINE__, rc, appDemoIpfixDevNum);
            }
            appDemoIpfixUdbCfg->keyTableUdbCfg[appDemoIpfixOtherKeyTableIndex].otherTypeKey->notValidIndex[index] = udbIndex;
        }

        /* Incrementing the udb configured count */
        appDemoIpfixTotalUdbConfigCount++;

    }
    else {

        if(appDemoIpfixUdbCfg == NULL)
        {
            if (appDemoIpfixTotalUdbConfigCount == 0)
            {
                rc = GT_OK;
                if(debug_print_enable)
                {
                    cpssOsPrintf("[%s:%d] No UDBs configured\n", __FUNCNAME__, __LINE__);
                }
            }
            else
            {
                rc = GT_BAD_STATE;
                cpssOsPrintf("[%s:%d] Retrieving appDemoIpfixUdbCfg  FAILED rc[%d]\n",
                             __FUNCNAME__, __LINE__, rc);
            }
            return rc;
        }

        for(keyTableIndex = 0; keyTableIndex < APP_DEMO_IPFIX_KEY_TABLE_INDEX_CNS; keyTableIndex++)
        {
            if(appDemoIpfixUdbCfg->keyTableUdbCfg[keyTableIndex].otherTypeKey != NULL)
            {
                cpssOsFree(appDemoIpfixUdbCfg->keyTableUdbCfg[keyTableIndex].otherTypeKey);
                appDemoIpfixUdbCfg->keyTableUdbCfg[keyTableIndex].otherTypeKey = NULL;

                /* Decrementing the udb configured count */
                appDemoIpfixTotalUdbConfigCount--;
            }
        }

        /* Clearing UDB config */
        rc = cpssDxChPclUserDefinedBytesSelectSet(appDemoIpfixDevNum,
                                                  CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_30_E,
                                                  CPSS_DXCH_PCL_PACKET_TYPE_ETHERNET_OTHER_E,
                                                  appDemoIpfixLookupNum,
                                                  &udbSelectStruct);
        if (GT_OK != rc)
        {
            cpssOsPrintf("[%s:%d] cpssDxChPclUserDefinedBytesSelectSet FAILED, rc [%d] devNum [%d]\n",
                         __FUNCNAME__, __LINE__, rc, appDemoIpfixDevNum);
            return rc;
        }
    }

    return rc;
}

GT_STATUS appDemoIpfixPortEnableSet
(
    IN  GT_U32                          flowMngId,
    IN  GT_U8                           devNum,
    IN  GT_PORT_NUM                     portNum,
    IN  GT_BOOL                         enable
)
{
    GT_STATUS                      rc = GT_OK;
    GT_U32                         pktType = 0;
    GT_U32                         keyTableIndex = 0;
    GT_U32                         i = 0;
    GT_U32                         keyIndexArr[CPSS_DXCH_FLOW_MANAGER_PACKET_TYPE_NUM_E];
    GT_U32                         entryIndex;
    GT_U32                         etherType;
    GT_U32                         profileIndex;
    GT_U32                         tpidBmp;
    GT_U32                         ethMode;
    CPSS_DXCH_VIRTUAL_TCAM_RULE_ID vTcamRuleId;

    appDemoIpfixPort = portNum;
    /* Before enabling flow classification for given port, vlidating the flow id to
     * avoid calling not useful cpss calls.
     */
    if (appDemoIpfixFlowMngId == APP_DEMO_IPFIX_FLOW_MANAGER_ID_NOT_VALID ||
        appDemoIpfixFlowMngId != flowMngId)
    {
        rc = GT_FAIL;
        if(debug_print_enable == GT_TRUE)
        {
            cpssOsPrintf("[%s:%d] Flow manager ID doesn't exist rc[%d]\n",
                __FUNCNAME__, __LINE__, rc);
        }
        return rc;
    }

    cpssOsBzero((GT_CHAR*) &vTcamRuleId, sizeof(vTcamRuleId));

    for(keyTableIndex = 0; keyTableIndex < CPSS_DXCH_FLOW_MANAGER_PACKET_TYPE_NUM_E; keyTableIndex++)
    {
        keyIndexArr[keyTableIndex] = APP_DEMO_IPFIX_NOT_USED_KEY_TABLE_INDEX;
    }
    if(debug_print_enable == GT_TRUE)
    {
        cpssOsPrintf("[%s:%d] appDemoIpfixPortEnableSet entered\n",__FUNCNAME__, __LINE__);
    }

    if(appDemoIpfixUdbCfg == NULL)
    {
        if(appDemoIpfixTotalUdbConfigCount == 0 )
        {
            rc = GT_BAD_PARAM;
            cpssOsPrintf("[%s:%d] No UDBs configured\n", __FUNCNAME__, __LINE__);
            return rc;
        }
        rc = GT_BAD_STATE;
        cpssOsPrintf("[%s:%d] Failed to retrieve UDB DB\n", __FUNCNAME__, __LINE__);
        return rc;
    }

    for(keyTableIndex = 0; keyTableIndex < APP_DEMO_IPFIX_KEY_TABLE_INDEX_CNS; keyTableIndex++)
    {
        if(appDemoIpfixUdbCfg->keyTableUdbCfg[keyTableIndex].ipv4Key != NULL)
        {
            if(appDemoIpfixUdbCfg->keyTableUdbCfg[keyTableIndex].ipv4Key->pktType == APP_DEMO_IPFIX_PACKET_TYPE_IPV4_TCP_E)
            {
                keyIndexArr[CPSS_DXCH_FLOW_MANAGER_PACKET_TYPE_IPV4_TCP_E] = appDemoIpfixIpv4TcpKeyTableIndex;
                appDemoIpfixEnableBitmap = (enable == GT_TRUE)? (appDemoIpfixEnableBitmap | 0x1) : (appDemoIpfixEnableBitmap & 0x1E);
            }
            else if(appDemoIpfixUdbCfg->keyTableUdbCfg[keyTableIndex].ipv4Key->pktType == APP_DEMO_IPFIX_PACKET_TYPE_IPV4_UDP_E)
            {
                keyIndexArr[CPSS_DXCH_FLOW_MANAGER_PACKET_TYPE_IPV4_UDP_E] = appDemoIpfixIpv4UdpKeyTableIndex;
                appDemoIpfixEnableBitmap = (enable == GT_TRUE)? (appDemoIpfixEnableBitmap | 0x2) : (appDemoIpfixEnableBitmap & 0x1D);
            }
            else
            {
                rc = GT_BAD_PARAM;
                cpssOsPrintf("[%s:%d] Protocol[%d] is not a valid configuration\n", __FUNCNAME__, __LINE__,
                             appDemoIpfixUdbCfg->keyTableUdbCfg[keyTableIndex].ipv4Key->pktType);
                return rc;
            }
        }

        if(appDemoIpfixUdbCfg->keyTableUdbCfg[keyTableIndex].ipv6Key != NULL)
        {
            if(appDemoIpfixUdbCfg->keyTableUdbCfg[keyTableIndex].ipv6Key->pktType == APP_DEMO_IPFIX_PACKET_TYPE_IPV6_TCP_E)
            {
                keyIndexArr[CPSS_DXCH_FLOW_MANAGER_PACKET_TYPE_IPV6_TCP_E] = appDemoIpfixIpv6TcpKeyTableIndex;
                appDemoIpfixEnableBitmap = (enable == GT_TRUE)? (appDemoIpfixEnableBitmap | 0x4) : (appDemoIpfixEnableBitmap & 0x1B);
            }
            else if(appDemoIpfixUdbCfg->keyTableUdbCfg[keyTableIndex].ipv6Key->pktType == APP_DEMO_IPFIX_PACKET_TYPE_IPV6_UDP_E)
            {
                keyIndexArr[CPSS_DXCH_FLOW_MANAGER_PACKET_TYPE_IPV6_UDP_E] = appDemoIpfixIpv6UdpKeyTableIndex;
                appDemoIpfixEnableBitmap = (enable == GT_TRUE)? (appDemoIpfixEnableBitmap | 0x8) : (appDemoIpfixEnableBitmap & 0x17);
            }
            else
            {
                rc = GT_BAD_PARAM;
                cpssOsPrintf("[%s:%d] Protocol[%d] is not a valid configuration\n", __FUNCNAME__, __LINE__,
                             appDemoIpfixUdbCfg->keyTableUdbCfg[keyTableIndex].ipv6Key->pktType);
                return rc;
            }
        }

        if(appDemoIpfixUdbCfg->keyTableUdbCfg[keyTableIndex].otherTypeKey != NULL)
        {
            keyIndexArr[CPSS_DXCH_FLOW_MANAGER_PACKET_TYPE_OTHER_E]    = appDemoIpfixOtherKeyTableIndex;
            appDemoIpfixEnableBitmap = (enable == GT_TRUE)? (appDemoIpfixEnableBitmap | 0x10) : (appDemoIpfixEnableBitmap & 0xF);
        }
    }

    if(debug_print_enable)
    {
        for(i = 0; i < 5; i++)
        {
            cpssOsPrintf("[%s:%d] keyIndexArr[%d]=%d\n", __FUNCNAME__, __LINE__, i, keyIndexArr[i]);
        }
        cpssOsPrintf("[%s:%d] appDemoIpfixEnableBitmap=%d, enable[%d]\n",
                __FUNCNAME__, __LINE__, appDemoIpfixEnableBitmap, enable);
    }

    if(appDemoIpfixEnableBitmap == 0)
    {
        for( pktType = 0; pktType < APP_DEMO_IPFIX_MAX_PKT_TYPE_NUM; pktType++)
        {
            appDemoIpfixUdbCfg->keyIndexMapCfg[devNum][portNum].keyIndexArr[pktType] = APP_DEMO_IPFIX_NOT_USED_KEY_TABLE_INDEX;
        }
    }

    /* Store the key index to packet type mapping configuration in local flow manager db */
    if(appDemoIpfixEnableBitmap & 0x1)
    {
        appDemoIpfixUdbCfg->keyIndexMapCfg[devNum][portNum].keyIndexArr[0] = keyIndexArr[CPSS_DXCH_FLOW_MANAGER_PACKET_TYPE_IPV4_TCP_E];
        if(debug_print_enable)
        {
            cpssOsPrintf("[%s:%d] appDemoIpfixUdbCfg->keyIndexMapCfg[%d][%d].keyIndexArr[%d]=%d\n", __FUNCNAME__, __LINE__,
                         devNum, portNum, CPSS_DXCH_FLOW_MANAGER_PACKET_TYPE_IPV4_TCP_E,
                         appDemoIpfixUdbCfg->keyIndexMapCfg[devNum][portNum].keyIndexArr[0]);
        }
    }
    if(appDemoIpfixEnableBitmap & 0x2)
    {
        appDemoIpfixUdbCfg->keyIndexMapCfg[devNum][portNum].keyIndexArr[1] = keyIndexArr[CPSS_DXCH_FLOW_MANAGER_PACKET_TYPE_IPV4_UDP_E];
        if(debug_print_enable)
        {
            cpssOsPrintf("[%s:%d] appDemoIpfixUdbCfg->keyIndexMapCfg[%d][%d].keyIndexArr[%d]=%d\n", __FUNCNAME__, __LINE__,
                         devNum, portNum, CPSS_DXCH_FLOW_MANAGER_PACKET_TYPE_IPV4_UDP_E,
                         appDemoIpfixUdbCfg->keyIndexMapCfg[devNum][portNum].keyIndexArr[1]);
        }
    }
    if(appDemoIpfixEnableBitmap & 0x4)
    {
        appDemoIpfixUdbCfg->keyIndexMapCfg[devNum][portNum].keyIndexArr[2] = keyIndexArr[CPSS_DXCH_FLOW_MANAGER_PACKET_TYPE_IPV6_TCP_E];
        if(debug_print_enable)
        {
            cpssOsPrintf("[%s:%d] appDemoIpfixUdbCfg->keyIndexMapCfg[%d][%d].keyIndexArr[%d]=%d\n", __FUNCNAME__, __LINE__,
                         devNum, portNum, CPSS_DXCH_FLOW_MANAGER_PACKET_TYPE_IPV6_TCP_E,
                         appDemoIpfixUdbCfg->keyIndexMapCfg[devNum][portNum].keyIndexArr[2]);
        }
    }
    if(appDemoIpfixEnableBitmap & 0x8)
    {
        appDemoIpfixUdbCfg->keyIndexMapCfg[devNum][portNum].keyIndexArr[3] = keyIndexArr[CPSS_DXCH_FLOW_MANAGER_PACKET_TYPE_IPV6_UDP_E];
        if(debug_print_enable)
        {
            cpssOsPrintf("[%s:%d] appDemoIpfixUdbCfg->keyIndexMapCfg[%d][%d].keyIndexArr[%d]=%d\n", __FUNCNAME__, __LINE__,
                         devNum, portNum, CPSS_DXCH_FLOW_MANAGER_PACKET_TYPE_IPV6_UDP_E,
                         appDemoIpfixUdbCfg->keyIndexMapCfg[devNum][portNum].keyIndexArr[3]);
        }
    }
    if(appDemoIpfixEnableBitmap & 0x10)
    {
        appDemoIpfixUdbCfg->keyIndexMapCfg[devNum][portNum].keyIndexArr[4] = keyIndexArr[CPSS_DXCH_FLOW_MANAGER_PACKET_TYPE_OTHER_E];
        if(debug_print_enable)
        {
            cpssOsPrintf("[%s:%d] appDemoIpfixUdbCfg->keyIndexMapCfg[%d][%d].keyIndexArr[%d]=%d\n", __FUNCNAME__, __LINE__,
                         devNum, portNum, CPSS_DXCH_FLOW_MANAGER_PACKET_TYPE_OTHER_E,
                         appDemoIpfixUdbCfg->keyIndexMapCfg[devNum][portNum].keyIndexArr[4]);
        }
    }

    /* Mapping the configured keys to a source port */
    rc = cpssDxChFlowManagerSrcPortPktTypeToKeyIndexMappingSet(devNum,
                                                               portNum,
                        appDemoIpfixUdbCfg->keyIndexMapCfg[devNum][portNum].keyIndexArr,
                                                               appDemoIpfixEnableBitmap);
    if (GT_OK != rc)
    {
        cpssOsPrintf("[%s:%d] cpssDxChFlowManagerSrcPortPktTypeToKeyIndexMappingSet FAILED, rc [%d] devNum [%d]\n",
                     __FUNCNAME__, __LINE__, rc, devNum);
        return rc;
    }

    vTcamRuleId = portNum;

    /* Validate/Invalidate the default rule for portNum in vTcam(1) */
    rc = cpssDxChVirtualTcamRuleValidStatusSet(devNum, appDemoIpfixShortAclVTcam,
                                               vTcamRuleId, enable);
    if (GT_OK != rc)
    {
        cpssOsPrintf("[%s:%d] cpssDxChVirtualTcamRuleValidStatusSet FAILED :"
                     "devNum[%d],vTcamId [%d] vTcamRuleId[%d], rc[%d]\n",
                     __FUNCNAME__, __LINE__, devNum, appDemoIpfixShortAclVTcam,
                     vTcamRuleId, rc);
        return rc;
    }

    if(debug_print_enable)
    {
        cpssOsPrintf("[%s:%d] appDemoIpfixPortEnableSet devNum[%d] portNum[%d]"
                     " vTcam[%d] enable[%d] successfully exiting\n",
                     __FUNCNAME__, __LINE__, devNum, portNum, appDemoIpfixShortAclVTcam, enable);
    }

    /* Validate/Invalidate the default rule for portNum in vTcam(2) */
    rc = cpssDxChVirtualTcamRuleValidStatusSet(devNum, appDemoIpfixLongAclVTcam,
                                               vTcamRuleId, enable);
    if (GT_OK != rc)
    {
        cpssOsPrintf("[%s:%d] cpssDxChVirtualTcamRuleValidStatusSet FAILED :"
                     "devNum[%d],vTcamId [%d] vTcamRuleId[%d], rc[%d]\n",
                     __FUNCNAME__, __LINE__, devNum, appDemoIpfixLongAclVTcam,
                     vTcamRuleId, rc);
        return rc;
    }

    if(debug_print_enable)
    {
        cpssOsPrintf("[%s:%d] appDemoIpfixPortEnableSet devNum[%d] portNum[%d]"
                     " vTcam[%d] enable[%d] successfully exiting\n",
                     __FUNCNAME__, __LINE__, devNum, portNum, appDemoIpfixLongAclVTcam, enable);
    }

    /* Enable Vlan Configuration for QinQ */
    if(appDemoIpfixEnableQinQ == GT_TRUE)
    {
        etherType = 0x88a8;
        for(entryIndex = 0; entryIndex < 1; entryIndex++)
        {
            rc = cpssDxChBrgVlanTpidEntrySet(devNum, appDemoIpfixTableType, entryIndex, etherType);
            if (GT_OK != rc)
            {
                cpssOsPrintf("[%s:%d] cpssDxChBrgVlanTpidEntrySet FAILED :"
                    "devNum[%d],table type [%d], entryIndex[%d], etherType [%d], rc[%d]\n",
                    __FUNCNAME__, __LINE__, devNum, appDemoIpfixTableType,
                    entryIndex, etherType, rc);
                return rc;
            }
        }
        tpidBmp = 0x1;
        ethMode = appDemoIpfixEthMode1;
        for(profileIndex = 0; profileIndex < 2; profileIndex++)
        {
            rc = cpssDxChBrgVlanIngressTpidProfileSet(devNum, profileIndex, ethMode, tpidBmp);
            if (GT_OK != rc)
            {
                cpssOsPrintf("[%s:%d] cpssDxChBrgVlanIngressTpidProfileSet FAILED :"
                    "devNum[%d],profileIndex[%d], ethMode[%d], tpidBmp[%d], rc[%d]\n",
                    __FUNCNAME__, __LINE__, devNum, profileIndex,
                    ethMode, tpidBmp, rc);
                return rc;
            }

            rc = cpssDxChBrgVlanPortIngressTpidProfileSet(devNum, portNum, ethMode, GT_FALSE, profileIndex);
            if (GT_OK != rc)
            {
                cpssOsPrintf("[%s:%d] cpssDxChBrgVlanPortIngressTpidProfileSet FAILED :"
                    "devNum[%d],profileIndex[%d], ethMode[%d] rc[%d]\n",
                    __FUNCNAME__, __LINE__, devNum, profileIndex,
                    ethMode, rc);
                return rc;
            }

            ethMode = appDemoIpfixEthMode2;
            tpidBmp = 0x2;
        }
    }

    return GT_OK;
}

GT_STATUS appDemoIpfixPacketTypeEnableSet
(
    IN  GT_U32                                 flowMngId,
    IN  CPSS_DXCH_FLOW_MANAGER_PACKET_TYPE_ENT pktType,
    IN  GT_BOOL                                enable
)
{
    CPSS_DXCH_VIRTUAL_TCAM_RULE_ID vTcamRuleId;
    GT_U32                         keyIndexArr[CPSS_DXCH_FLOW_MANAGER_PACKET_TYPE_NUM_E];

    GT_STATUS                      rc = GT_OK;
    GT_PORT_NUM                    portNum = 0;
    GT_U32                         ruleId=0;
    GT_U32                         vTcamId=0;
    GT_U32                         keyTableIndex = 0;
    GT_U32                         appDemoIpfixDevNum = prvWrAppFirstDevNum(0);

    /* Before enabling flow classification for given packet type, vlidating the flow id to
     * avoid calling not useful cpss calls.
     */
    if (appDemoIpfixFlowMngId == APP_DEMO_IPFIX_FLOW_MANAGER_ID_NOT_VALID ||
        appDemoIpfixFlowMngId != flowMngId)
    {
        rc = GT_FAIL;
        if(debug_print_enable == GT_TRUE)
        {
            cpssOsPrintf("[%s:%d] Flow manager ID doesn't exist rc[%d]\n",
                __FUNCNAME__, __LINE__, rc);
        }
        return rc;
    }

    cpssOsMemSet((GT_CHAR*)&keyIndexArr, APP_DEMO_IPFIX_NOT_USED_KEY_TABLE_INDEX, sizeof(keyIndexArr));
    cpssOsBzero((GT_CHAR*) &vTcamRuleId, sizeof(vTcamRuleId));

    if(debug_print_enable == GT_TRUE)
    {
        cpssOsPrintf("[%s:%d] appDemoIpfixPacketTypeEnableSet entered\n", __FUNCNAME__, __LINE__);
    }

    if(appDemoIpfixUdbCfg == NULL)
    {
        if(appDemoIpfixTotalUdbConfigCount == 0 )
        {
            rc = GT_BAD_PARAM;
            cpssOsPrintf("[%s:%d] No UDBs configured\n", __FUNCNAME__, __LINE__);
            return rc;
        }
        rc = GT_BAD_STATE;
        cpssOsPrintf("[%s:%d] Failed to retrieve UDB DB\n", __FUNCNAME__, __LINE__);
        return rc;
    }

    switch(pktType)
    {
        case CPSS_DXCH_FLOW_MANAGER_PACKET_TYPE_IPV4_TCP_E:
            keyTableIndex = appDemoIpfixIpv4TcpKeyTableIndex;
            if(appDemoIpfixUdbCfg->keyTableUdbCfg[keyTableIndex].ipv4Key != NULL)
            {
                if(appDemoIpfixUdbCfg->keyTableUdbCfg[keyTableIndex].ipv4Key->pktType == APP_DEMO_IPFIX_PACKET_TYPE_IPV4_TCP_E)
                {
                    keyIndexArr[CPSS_DXCH_FLOW_MANAGER_PACKET_TYPE_IPV4_TCP_E] = appDemoIpfixIpv4TcpKeyTableIndex;
                    appDemoIpfixEnableBitmap =
                        (enable == GT_TRUE)? (appDemoIpfixEnableBitmap | 0x1) : (appDemoIpfixEnableBitmap & 0x1E);
                }
                else
                {
                    rc = GT_BAD_PARAM;
                    cpssOsPrintf("[%s:%d] Protocol[%d] is not a valid configuration\n", __FUNCNAME__, __LINE__,
                            appDemoIpfixUdbCfg->keyTableUdbCfg[keyTableIndex].ipv4Key->pktType);
                    return rc;
                }
                ruleId = 0;
                vTcamId = appDemoIpfixShortAclVTcam;
            }
            break;
        case CPSS_DXCH_FLOW_MANAGER_PACKET_TYPE_IPV4_UDP_E:
            keyTableIndex = appDemoIpfixIpv4UdpKeyTableIndex;
            if(appDemoIpfixUdbCfg->keyTableUdbCfg[keyTableIndex].ipv4Key != NULL)
            {
                if(appDemoIpfixUdbCfg->keyTableUdbCfg[keyTableIndex].ipv4Key->pktType == APP_DEMO_IPFIX_PACKET_TYPE_IPV4_UDP_E)
                {
                    keyIndexArr[CPSS_DXCH_FLOW_MANAGER_PACKET_TYPE_IPV4_UDP_E] = appDemoIpfixIpv4UdpKeyTableIndex;
                    appDemoIpfixEnableBitmap =
                        (enable == GT_TRUE)? (appDemoIpfixEnableBitmap | 0x2) : (appDemoIpfixEnableBitmap & 0x1D);
                }
                else
                {
                    rc = GT_BAD_PARAM;
                    cpssOsPrintf("[%s:%d] Protocol[%d] is not a valid configuration\n", __FUNCNAME__, __LINE__,
                            appDemoIpfixUdbCfg->keyTableUdbCfg[keyTableIndex].ipv4Key->pktType);
                    return rc;
                }
                ruleId = 1;
                vTcamId = appDemoIpfixShortAclVTcam;
            }
            break;
        case CPSS_DXCH_FLOW_MANAGER_PACKET_TYPE_IPV6_TCP_E:
            keyTableIndex = appDemoIpfixIpv6TcpKeyTableIndex;
            if(appDemoIpfixUdbCfg->keyTableUdbCfg[keyTableIndex].ipv6Key != NULL)
            {
                if(appDemoIpfixUdbCfg->keyTableUdbCfg[keyTableIndex].ipv6Key->pktType == APP_DEMO_IPFIX_PACKET_TYPE_IPV6_TCP_E)
                {
                    keyIndexArr[CPSS_DXCH_FLOW_MANAGER_PACKET_TYPE_IPV6_TCP_E] = appDemoIpfixIpv6TcpKeyTableIndex;
                    appDemoIpfixEnableBitmap =
                        (enable == GT_TRUE)? (appDemoIpfixEnableBitmap | 0x4) : (appDemoIpfixEnableBitmap & 0x1B);
                }
                else
                {
                    rc = GT_BAD_PARAM;
                    cpssOsPrintf("[%s:%d] Protocol[%d] is not a valid configuration\n", __FUNCNAME__, __LINE__,
                            appDemoIpfixUdbCfg->keyTableUdbCfg[keyTableIndex].ipv6Key->pktType);
                    return rc;
                }
                ruleId = 0;
                vTcamId = appDemoIpfixLongAclVTcam;
            }
            break;
        case CPSS_DXCH_FLOW_MANAGER_PACKET_TYPE_IPV6_UDP_E:
            keyTableIndex = appDemoIpfixIpv6UdpKeyTableIndex;
            if(appDemoIpfixUdbCfg->keyTableUdbCfg[keyTableIndex].ipv6Key != NULL)
            {
                if(appDemoIpfixUdbCfg->keyTableUdbCfg[keyTableIndex].ipv6Key->pktType == APP_DEMO_IPFIX_PACKET_TYPE_IPV6_UDP_E)
                {
                    keyIndexArr[CPSS_DXCH_FLOW_MANAGER_PACKET_TYPE_IPV6_UDP_E] = appDemoIpfixIpv6UdpKeyTableIndex;
                    appDemoIpfixEnableBitmap =
                        (enable == GT_TRUE)? (appDemoIpfixEnableBitmap | 0x8) : (appDemoIpfixEnableBitmap & 0x17);
                }
                else
                {
                    rc = GT_BAD_PARAM;
                    cpssOsPrintf("[%s:%d] Protocol[%d] is not a valid configuration\n", __FUNCNAME__, __LINE__,
                            appDemoIpfixUdbCfg->keyTableUdbCfg[keyTableIndex].ipv6Key->pktType);
                    return rc;
                }
                ruleId = 1;
                vTcamId = appDemoIpfixLongAclVTcam;
            }
            break;
        case CPSS_DXCH_FLOW_MANAGER_PACKET_TYPE_OTHER_E:
            keyTableIndex = appDemoIpfixOtherKeyTableIndex;
            if(appDemoIpfixUdbCfg->keyTableUdbCfg[keyTableIndex].otherTypeKey != NULL)
            {
                keyIndexArr[CPSS_DXCH_FLOW_MANAGER_PACKET_TYPE_OTHER_E]    = appDemoIpfixOtherKeyTableIndex;
                appDemoIpfixEnableBitmap =
                    (enable == GT_TRUE)? (appDemoIpfixEnableBitmap | 0x10) : (appDemoIpfixEnableBitmap & 0xF);
                ruleId = 2;
                vTcamId = appDemoIpfixShortAclVTcam;
            }
            break;
        default:
            rc = GT_NOT_SUPPORTED;
            cpssOsPrintf("[%s:%d] Invalid packet type\n", __FUNCNAME__, __LINE__);
            return rc;
    }

    vTcamRuleId = APP_DEMO_IPFIX_MAX_PORT_NUM + ruleId;

    if(debug_print_enable)
    {
        cpssOsPrintf("[%s:%d] flowMngId[%d] packetType[%d] enable[%d] keyIndexArr[%d]=%d appDemoIpfixEnableBitmap[%x]\n",
                __FUNCNAME__, __LINE__, flowMngId, pktType, enable, pktType, keyIndexArr[pktType], appDemoIpfixEnableBitmap);
    }

    for(portNum = 0; portNum < APP_DEMO_IPFIX_MAX_PORT_NUM; portNum++)
    {
        switch(pktType)
        {
            case CPSS_DXCH_FLOW_MANAGER_PACKET_TYPE_IPV4_TCP_E:
                appDemoIpfixUdbCfg->keyIndexMapCfg[appDemoIpfixDevNum][portNum].keyIndexArr[0]
                    = keyIndexArr[CPSS_DXCH_FLOW_MANAGER_PACKET_TYPE_IPV4_TCP_E];
                break;
            case CPSS_DXCH_FLOW_MANAGER_PACKET_TYPE_IPV4_UDP_E:
                appDemoIpfixUdbCfg->keyIndexMapCfg[appDemoIpfixDevNum][portNum].keyIndexArr[1]
                    = keyIndexArr[CPSS_DXCH_FLOW_MANAGER_PACKET_TYPE_IPV4_UDP_E];
                break;
            case CPSS_DXCH_FLOW_MANAGER_PACKET_TYPE_IPV6_TCP_E:
                appDemoIpfixUdbCfg->keyIndexMapCfg[appDemoIpfixDevNum][portNum].keyIndexArr[2]
                    = keyIndexArr[CPSS_DXCH_FLOW_MANAGER_PACKET_TYPE_IPV6_TCP_E];
                break;
            case CPSS_DXCH_FLOW_MANAGER_PACKET_TYPE_IPV6_UDP_E:
                appDemoIpfixUdbCfg->keyIndexMapCfg[appDemoIpfixDevNum][portNum].keyIndexArr[3]
                    = keyIndexArr[CPSS_DXCH_FLOW_MANAGER_PACKET_TYPE_IPV6_UDP_E];
                break;
            case CPSS_DXCH_FLOW_MANAGER_PACKET_TYPE_OTHER_E:
                appDemoIpfixUdbCfg->keyIndexMapCfg[appDemoIpfixDevNum][portNum].keyIndexArr[4]
                    = keyIndexArr[CPSS_DXCH_FLOW_MANAGER_PACKET_TYPE_OTHER_E];
                break;
            default:
                rc = GT_NOT_SUPPORTED;
                cpssOsPrintf("[%s:%d] Invalid packet type\n", __FUNCNAME__, __LINE__);
                return rc;
        }
        /* Mapping the configured keys to a source port */
        rc = cpssDxChFlowManagerSrcPortPktTypeToKeyIndexMappingSet(appDemoIpfixDevNum,
                                                                   portNum,
                appDemoIpfixUdbCfg->keyIndexMapCfg[appDemoIpfixDevNum][portNum].keyIndexArr,
                                                                   appDemoIpfixEnableBitmap);
        if (GT_OK != rc)
        {
            cpssOsPrintf("[%s:%d] cpssDxChFlowManagerSrcPortPktTypeToKeyIndexMappingSet FAILED, rc [%d] devNum [%d]\n",
                    __FUNCNAME__, __LINE__, rc, appDemoIpfixDevNum);
            return rc;
        }

        if(debug_print_enable)
        {
            cpssOsPrintf("[%s:%d] cpssDxChFlowManagerSrcPortPktTypeToKeyIndexMappingSet devNum[%d] portNum[%d]"
                    " keyIndexArr[%d]=%d appDemoIpfixEnableBitmap[%d] status set successfully\n",__FUNCNAME__, __LINE__,
                    appDemoIpfixDevNum, portNum, pktType,
                    appDemoIpfixUdbCfg->keyIndexMapCfg[appDemoIpfixDevNum][portNum].keyIndexArr[pktType],
                    appDemoIpfixEnableBitmap);
        }
    }

    /* Validate/Invalidate the default rule for the portNum */
    rc = cpssDxChVirtualTcamRuleValidStatusSet(appDemoIpfixDevNum, vTcamId,
            vTcamRuleId, enable);
    if (GT_OK != rc)
    {
        cpssOsPrintf("[%s:%d] cpssDxChVirtualTcamRuleValidStatusSet FAILED :"
                "devNum[%d],vTcamId [%d] vTcamRuleId[%d], rc[%d]\n",
                __FUNCNAME__, __LINE__, appDemoIpfixDevNum, vTcamId,
                vTcamRuleId, rc);
        return rc;
    }

    if(debug_print_enable == GT_TRUE)
    {
        cpssOsPrintf("[%s:%d] cpssDxChVirtualTcamRuleValidStatusSet devNum[%d] pktType[%d]"
                " vTcamId[%d] vTcamRuleId[%d] enable[%d] status set successfully\n",__FUNCNAME__, __LINE__,
                appDemoIpfixDevNum, pktType, vTcamId, vTcamRuleId,enable);
    }

    if(debug_print_enable == GT_TRUE)
    {
        cpssOsPrintf("[%s:%d] appDemoIpfixPacketTypeEnableSet successfully exiting\n",__FUNCNAME__, __LINE__);
    }

    return GT_OK;
}

static GT_STATUS prvFlowManagerInstallDefaultRules
(
    IN  GT_U8                           devNum,
    IN  GT_BOOL                         enable
)
{
    GT_U32                                     vTcamId = 0;
    GT_STATUS                                  rc = GT_OK;
    GT_PORT_NUM                                portNum = 0;
    CPSS_DXCH_PCL_ACTION_STC                   action;
    CPSS_DXCH_PCL_RULE_FORMAT_UNT              mask;
    CPSS_DXCH_PCL_RULE_FORMAT_UNT              pattern;
    CPSS_DXCH_VIRTUAL_TCAM_RULE_ID             vTcamRuleId;
    CPSS_DXCH_VIRTUAL_TCAM_USAGE_STC           vTcamUsage;
    CPSS_DXCH_VIRTUAL_TCAM_RULE_DATA_STC       ruleData;
    CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_STC       tcamRuleType;
    CPSS_DXCH_FLOW_MANAGER_PACKET_TYPE_ENT     pktType;
    CPSS_DXCH_VIRTUAL_TCAM_RULE_ATTRIBUTES_STC ruleAttributes;

    cpssOsBzero((GT_CHAR*) &action, sizeof(action));
    cpssOsBzero((GT_CHAR*) &mask, sizeof(mask));
    cpssOsBzero((GT_CHAR*) &pattern, sizeof(pattern));
    cpssOsBzero((GT_CHAR*) &vTcamRuleId, sizeof(vTcamRuleId));
    cpssOsBzero((GT_CHAR*) &vTcamUsage, sizeof(vTcamUsage));
    cpssOsBzero((GT_CHAR*) &ruleData, sizeof(ruleData));
    cpssOsBzero((GT_CHAR*) &tcamRuleType, sizeof(tcamRuleType));
    cpssOsBzero((GT_CHAR*) &pktType, sizeof(pktType));
    cpssOsBzero((GT_CHAR*) &ruleAttributes, sizeof(ruleAttributes));

    if(debug_print_enable == GT_TRUE)
    {
        cpssOsPrintf("[%s:%d] prvFlowManagerInstallDefaultRules entered\n",__FUNCNAME__, __LINE__);
    }

    if(enable == GT_TRUE)
    {
        /* Installing port-based default rules */
        for(vTcamRuleId=0; vTcamRuleId < APP_DEMO_IPFIX_MAX_PORT_NUM; vTcamRuleId++)
        {
            portNum = vTcamRuleId;

            pattern.ruleIngrUdbOnly.replacedFld.srcPort = portNum;
            mask.ruleIngrUdbOnly.replacedFld.srcPort = 0x3F;

            action.pktCmd                    = appDemoIpfixDefaultRulePktCmd;
            action.mirror.cpuCode            = appDemoIpfixDefaultRuleCpuCode;
            ruleData.valid                   = GT_TRUE;
            ruleData.rule.pcl.actionPtr      = &action;
            ruleData.rule.pcl.maskPtr        = &mask;
            ruleData.rule.pcl.patternPtr     = &pattern;
            tcamRuleType.ruleType            = CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_PCL_E;
            tcamRuleType.rule.pcl.ruleFormat = appDemoIpfixKeyTypesIpv4Key;

            rc = cpssDxChVirtualTcamUsageGet(devNum,
                             appDemoIpfixShortAclVTcam, &vTcamUsage);
            if (GT_OK != rc)
            {
                cpssOsPrintf("[%s:%d] cpssDxChVirtualTcamUsageGet FAILED :"
                             "devNum[%d], vTcamId[%d], rc[%d]\n",__FUNCNAME__, __LINE__,
                             devNum, appDemoIpfixShortAclVTcam, rc);
                return rc;
            }

            if (vTcamUsage.rulesFree == 0)
            {
                 cpssOsPrintf("[%s:%d] No Free rules left in TCAM FAILED :"
                              "devNum[%d], vTcamId[%d], rc[%d]\n",__FUNCNAME__, __LINE__,
                              devNum, appDemoIpfixShortAclVTcam, rc);
                 return rc;
            }

            rc = cpssDxChVirtualTcamRuleWrite(devNum, appDemoIpfixShortAclVTcam,
                                              vTcamRuleId, &ruleAttributes,
                                              &tcamRuleType, &ruleData);
            if (GT_OK != rc)
            {
                cpssOsPrintf("[%s:%d] cpssDxChVirtualTcamRuleWrite FAILED :"
                             "devNum[%d],vTcamId [%d] vTcamRuleId[%d], rc[%d]\n",
                             __FUNCNAME__, __LINE__, devNum, appDemoIpfixShortAclVTcam,
                             vTcamRuleId, rc);
                return rc;
            }

            if( debug_print_enable)
            {
                cpssOsPrintf("[%s:%d] cpssDxChVirtualTcamRuleWrite SUCCESS :"
                             "devNum[%d],vTcamId [%d], "
                             "vTcamRuleId[%d], portNum[%d] rc[%d]\n",__FUNCNAME__, __LINE__,
                             devNum, appDemoIpfixShortAclVTcam,
                             vTcamRuleId, portNum, rc);
            }

            tcamRuleType.rule.pcl.ruleFormat = appDemoIpfixKeyTypesIpv6Key;

            cpssDxChVirtualTcamUsageGet(devNum, appDemoIpfixLongAclVTcam, &vTcamUsage);
            if (GT_OK != rc)
            {
                cpssOsPrintf("[%s:%d] cpssDxChVirtualTcamUsageGet FAILED :"
                             "devNum[%d], vTcamId[%d], rc[%d]\n",__FUNCNAME__, __LINE__,
                             devNum, appDemoIpfixLongAclVTcam, rc);
                return rc;
            }

            if (vTcamUsage.rulesFree == 0)
            {
                cpssOsPrintf("[%s:%d] No Free rules left in TCAM FAILED :"
                             "devNum[%d], vTcamId[%d], rc[%d]\n",__FUNCNAME__, __LINE__,
                             devNum, appDemoIpfixLongAclVTcam, rc);
                return rc;
            }

            rc = cpssDxChVirtualTcamRuleWrite(devNum, appDemoIpfixLongAclVTcam,
                                              vTcamRuleId, &ruleAttributes,
                                              &tcamRuleType, &ruleData);
            if (GT_OK != rc)
            {
                cpssOsPrintf("[%s:%d] cpssDxChVirtualTcamRuleWrite FAILED :"
                             "devNum[%d],vTcamId [%d] vTcamRuleId[%d], rc[%d]\n",
                             __FUNCNAME__, __LINE__, devNum, appDemoIpfixLongAclVTcam,
                             vTcamRuleId, rc);
                return rc;
            }

            if( debug_print_enable)
            {
                cpssOsPrintf("[%s:%d] cpssDxChVirtualTcamRuleWrite SUCCESS :"
                             "devNum[%d],vTcamId [%d], "
                             "vTcamRuleId[%d], portNum[%d] rc[%d]\n",
                             __FUNCNAME__, __LINE__, devNum, appDemoIpfixLongAclVTcam,
                             vTcamRuleId, portNum, rc);
            }
        }

        /* Installing flow-based default rules */
        for(pktType = CPSS_DXCH_FLOW_MANAGER_PACKET_TYPE_IPV4_TCP_E;
                pktType < CPSS_DXCH_FLOW_MANAGER_PACKET_TYPE_NUM_E; pktType++)
        {
            switch(pktType)
            {
                case CPSS_DXCH_FLOW_MANAGER_PACKET_TYPE_IPV4_TCP_E:
                    vTcamRuleId = APP_DEMO_IPFIX_IPV4_TCP_DEFAULT_RULE_ID;
                    vTcamId = appDemoIpfixShortAclVTcam;
                    /* Enabling pattern as IPv4-TCP(1) for "Applicable Flow Sub-template" key field(udb[5]) */
                    pattern.ruleIngrUdbOnly.udb[5] = 1; /* IPv4-TCP */
                    mask.ruleIngrUdbOnly.udb[5] = 0xFF;
                    tcamRuleType.rule.pcl.ruleFormat = appDemoIpfixKeyTypesIpv4Key;
                    break;
                case CPSS_DXCH_FLOW_MANAGER_PACKET_TYPE_IPV4_UDP_E:
                    vTcamRuleId = APP_DEMO_IPFIX_IPV4_UDP_DEFAULT_RULE_ID;
                    vTcamId = appDemoIpfixShortAclVTcam;
                    /* Enabling pattern as IPv4-UDP(2) for "Applicable Flow Sub-template" key field(udb[5]) */
                    pattern.ruleIngrUdbOnly.udb[5] = 2; /* IPv4-UDP */
                    mask.ruleIngrUdbOnly.udb[5] = 0xFF;
                    tcamRuleType.rule.pcl.ruleFormat = appDemoIpfixKeyTypesIpv4Key;
                    break;
                case CPSS_DXCH_FLOW_MANAGER_PACKET_TYPE_OTHER_E:
                    vTcamRuleId = APP_DEMO_IPFIX_OTHER_DEFAULT_RULE_ID;
                    vTcamId = appDemoIpfixShortAclVTcam;
                    /* Enabling pattern as OTHER(32) for "Applicable Flow Sub-template" key field(udb[5]) */
                    pattern.ruleIngrUdbOnly.udb[5] = 32; /*ETHERNET_OTHER */
                    mask.ruleIngrUdbOnly.udb[5] = 0xFF;
                    tcamRuleType.rule.pcl.ruleFormat = appDemoIpfixKeyTypesIpv4Key;
                    break;
                case CPSS_DXCH_FLOW_MANAGER_PACKET_TYPE_IPV6_TCP_E:
                    vTcamRuleId = APP_DEMO_IPFIX_IPV6_TCP_DEFAULT_RULE_ID;
                    vTcamId = appDemoIpfixLongAclVTcam;
                    /* Enabling pattern as IPv6-TCP(64) for "Applicable Flow Sub-template" key field(udb[5]) */
                    pattern.ruleIngrUdbOnly.udb[5] = 64; /* IPv6 */
                    mask.ruleIngrUdbOnly.udb[5] = 0xFF;
                    tcamRuleType.rule.pcl.ruleFormat = appDemoIpfixKeyTypesIpv6Key;
                    break;
                case CPSS_DXCH_FLOW_MANAGER_PACKET_TYPE_IPV6_UDP_E:
                    vTcamRuleId = APP_DEMO_IPFIX_IPV6_UDP_DEFAULT_RULE_ID;
                    vTcamId = appDemoIpfixLongAclVTcam;
                    /* Enabling pattern as IPv6-UDP(64) for "Applicable Flow Sub-template" key field(udb[5]) */
                    pattern.ruleIngrUdbOnly.udb[5] = 64; /* IPv6 */
                    mask.ruleIngrUdbOnly.udb[5] = 0xFF;
                    tcamRuleType.rule.pcl.ruleFormat = appDemoIpfixKeyTypesIpv6Key;
                    break;
                default:
                    rc = GT_NOT_SUPPORTED;
                    cpssOsPrintf("[%s:%d] Invalid packet type\n", __FUNCNAME__, __LINE__);
                    return rc;
            }

            action.pktCmd = appDemoIpfixDefaultRulePktCmd;
            action.mirror.cpuCode = appDemoIpfixDefaultRuleCpuCode;

            rc = cpssDxChVirtualTcamUsageGet(devNum,
                    vTcamId, &vTcamUsage);
            if (GT_OK != rc)
            {
                cpssOsPrintf("[%s:%d] cpssDxChVirtualTcamUsageGet FAILED :"
                        "devNum[%d], vTcamId[%d], rc[%d]\n",
                        __FUNCNAME__, __LINE__, devNum, vTcamId, rc);
                return rc;
            }

            if(debug_print_enable)
            {
                cpssOsPrintf("[%s:%d] vTcamUsage.rulesFree=%d in vTcamId[%d]\n",
                        __FUNCNAME__, __LINE__, vTcamUsage.rulesFree, vTcamId);
            }

            if (vTcamUsage.rulesFree == 0)
            {
                cpssOsPrintf("[%s:%d] No Free rules left in TCAM FAILED :"
                        "devNum[%d], vTcamId[%d], rc[%d]\n",
                        __FUNCNAME__, __LINE__, devNum, vTcamId, rc);
                return rc;
            }

            tcamRuleType.ruleType = CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_PCL_E;

            ruleData.valid                   = GT_TRUE;
            ruleData.rule.pcl.actionPtr      = &action;
            ruleData.rule.pcl.maskPtr        = &mask;
            ruleData.rule.pcl.patternPtr     = &pattern;

            rc = cpssDxChVirtualTcamRuleWrite(devNum, vTcamId,
                    vTcamRuleId, &ruleAttributes,
                    &tcamRuleType, &ruleData);
            if (GT_OK != rc)
            {
                cpssOsPrintf("[%s:%d] cpssDxChVirtualTcamRuleWrite FAILED :"
                        "devNum[%d],vTcamId [%d] vTcamRuleId[%d], rc[%d]\n",
                        __FUNCNAME__, __LINE__, devNum, vTcamId,
                        vTcamRuleId, rc);
                return rc;
            }

            if( debug_print_enable)
            {
                cpssOsPrintf("[%s:%d] cpssDxChVirtualTcamRuleWrite SUCCESS :"
                        "devNum[%d],vTcamId [%d], "
                        "ruleIndex[%d], pktType[%d] rc[%d]\n",
                        __FUNCNAME__, __LINE__, devNum, vTcamRuleId,
                        vTcamRuleId, pktType, rc);
            }
        }
    }
    else
    {
        /* Uninstalling port-based default rules */
        for(vTcamRuleId=0; vTcamRuleId < APP_DEMO_IPFIX_MAX_PORT_NUM; vTcamRuleId++)
        {
            portNum = vTcamRuleId;
            rc = cpssDxChVirtualTcamRuleDelete(devNum, appDemoIpfixShortAclVTcam,
                                               vTcamRuleId);
            if (GT_OK != rc)
            {
                cpssOsPrintf("[%s:%d] cpssDxChVirtualTcamRuleDelete FAILED :"
                             "devNum[%d],vTcamId [%d] vTcamRuleId[%d], rc[%d]\n",
                             __FUNCNAME__, __LINE__, devNum, appDemoIpfixShortAclVTcam,
                             vTcamRuleId, rc);
                return rc;
            }

            if( debug_print_enable)
            {
                cpssOsPrintf("[%s:%d] cpssDxChVirtualTcamRuleDelete SUCCESS :"
                             "devNum[%d],vTcamId [%d], "
                             "vTcamRuleId[%d], portNum[%d] rc[%d]\n",
                             __FUNCNAME__, __LINE__, devNum, appDemoIpfixShortAclVTcam,
                             vTcamRuleId, portNum, rc);
            }

            rc = cpssDxChVirtualTcamRuleDelete(devNum, appDemoIpfixLongAclVTcam,
                                               vTcamRuleId);
            if (GT_OK != rc)
            {
                cpssOsPrintf("[%s:%d] cpssDxChVirtualTcamRuleDelete FAILED :"
                             "devNum[%d],vTcamId [%d] vTcamRuleId[%d], rc[%d]\n",
                             __FUNCNAME__, __LINE__, devNum, appDemoIpfixLongAclVTcam,
                             vTcamRuleId, rc);
                return rc;
            }

            if( debug_print_enable)
            {
                cpssOsPrintf("[%s:%d] cpssDxChVirtualTcamRuleDelete SUCCESS :"
                             "devNum[%d],vTcamId [%d], "
                             "vTcamRuleId[%d], portNum[%d] rc[%d]\n",
                             __FUNCNAME__, __LINE__, devNum, appDemoIpfixLongAclVTcam,
                             vTcamRuleId, portNum, rc);
            }
        }

        /* Installing flow-based default rules */
        for(pktType = CPSS_DXCH_FLOW_MANAGER_PACKET_TYPE_IPV4_TCP_E;
             pktType < CPSS_DXCH_FLOW_MANAGER_PACKET_TYPE_NUM_E; pktType++)
        {
            switch(pktType)
            {
                case CPSS_DXCH_FLOW_MANAGER_PACKET_TYPE_IPV4_TCP_E:
                    vTcamRuleId = APP_DEMO_IPFIX_IPV4_TCP_DEFAULT_RULE_ID;
                    vTcamId = appDemoIpfixShortAclVTcam;
                    break;
                case CPSS_DXCH_FLOW_MANAGER_PACKET_TYPE_IPV4_UDP_E:
                    vTcamRuleId = APP_DEMO_IPFIX_IPV4_UDP_DEFAULT_RULE_ID;
                    vTcamId = appDemoIpfixShortAclVTcam;
                    break;
                case CPSS_DXCH_FLOW_MANAGER_PACKET_TYPE_OTHER_E:
                    vTcamRuleId = APP_DEMO_IPFIX_OTHER_DEFAULT_RULE_ID;
                    vTcamId = appDemoIpfixShortAclVTcam;
                    break;
                case CPSS_DXCH_FLOW_MANAGER_PACKET_TYPE_IPV6_TCP_E:
                    vTcamRuleId = APP_DEMO_IPFIX_IPV6_TCP_DEFAULT_RULE_ID;
                    vTcamId = appDemoIpfixLongAclVTcam;
                    break;
                case CPSS_DXCH_FLOW_MANAGER_PACKET_TYPE_IPV6_UDP_E:
                    vTcamRuleId = APP_DEMO_IPFIX_IPV6_UDP_DEFAULT_RULE_ID;
                    vTcamId = appDemoIpfixLongAclVTcam;
                    break;
                default:
                    rc = GT_NOT_SUPPORTED;
                    cpssOsPrintf("[%s:%d] Invalid packet type\n", __FUNCNAME__, __LINE__ );
                    return rc;
            }

            rc = cpssDxChVirtualTcamRuleDelete(devNum, vTcamId,
                    vTcamRuleId);
            if (GT_OK != rc)
            {
                cpssOsPrintf("[%s:%d] cpssDxChVirtualTcamRuleDelete FAILED :"
                        " devNum[%d],vTcamId [%d] vTcamRuleId[%d],"
                        " pktType[%d] rc[%d]\n",
                        __FUNCNAME__, __LINE__, devNum, vTcamId,
                        vTcamRuleId, pktType, rc);
                return rc;
            }

            if( debug_print_enable)
            {
                cpssOsPrintf("[%s:%d] cpssDxChVirtualTcamRuleDelete SUCCESS :"
                        "devNum[%d],vTcamId [%d], "
                        "vTcamRuleId[%d], pktType[%d] rc[%d]\n",
                        __FUNCNAME__, __LINE__, devNum, vTcamId,
                        vTcamRuleId, pktType, rc);
            }
        }
    }

    if(debug_print_enable == GT_TRUE)
    {
        cpssOsPrintf("[%s:%d] prvFlowManagerInstallDefaultRules successfully exiting\n",
                     __FUNCNAME__, __LINE__);
    }

    return GT_OK;
}

static GT_STATUS prvFlowManagerBindIpfixCountersToFwRules
(
 IN  GT_U8                           devNum,
 IN  GT_BOOL                         enable
 )
{
    GT_U32                                     numPolicerEntries = 0;
    GT_STATUS                                  rc = GT_OK;
    CPSS_DXCH_PCL_ACTION_STC                   action;
    CPSS_DXCH_PCL_RULE_FORMAT_UNT              mask;
    CPSS_DXCH_PCL_RULE_FORMAT_UNT              pattern;
    CPSS_DXCH_VIRTUAL_TCAM_RULE_ID             vTcamRuleId;
    CPSS_DXCH_VIRTUAL_TCAM_RULE_DATA_STC       ruleData;
    CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_STC       tcamRuleType;
    CPSS_DXCH_POLICER_MEMORY_CTRL_MODE_ENT     mode;
    CPSS_DXCH_VIRTUAL_TCAM_RULE_ATTRIBUTES_STC ruleAttributes;

    cpssOsBzero((GT_CHAR*) &mode, sizeof(mode));
    cpssOsBzero((GT_CHAR*) &action, sizeof(action));
    cpssOsBzero((GT_CHAR*) &mask, sizeof(mask));
    cpssOsBzero((GT_CHAR*) &pattern, sizeof(pattern));
    cpssOsBzero((GT_CHAR*) &vTcamRuleId, sizeof(vTcamRuleId));
    cpssOsBzero((GT_CHAR*) &ruleData, sizeof(ruleData));
    cpssOsBzero((GT_CHAR*) &tcamRuleType, sizeof(tcamRuleType));
    cpssOsBzero((GT_CHAR*) &ruleAttributes, sizeof(ruleAttributes));

    if(debug_print_enable == GT_TRUE)
    {
        cpssOsPrintf("[%s:%d] prvFlowManagerBindIpfixCountersToFwRules entered\n",__FUNCNAME__, __LINE__);
    }

    rc = cpssDxChPolicerMemorySizeModeGet(devNum, &mode, &numPolicerEntries, NULL, NULL);
    if(rc != GT_OK)
    {
        cpssOsPrintf("[%s:%d] cpssDxChPolicerMemorySizeModeGet FAILED, rc [%d] devNum [%d]\n",
                     __FUNCNAME__, __LINE__, rc, devNum);
        return rc;
    }

    if( debug_print_enable )
    {
        cpssOsPrintf("[%s:%d] TotalPolicerEntries[%d] mode[%d]\n",
                     __FUNCNAME__, __LINE__, numPolicerEntries, mode);
    }

    if(enable == GT_TRUE)
    {
        for(vTcamRuleId=0; vTcamRuleId < appDemoIpfixFwTcamNumOfShortRules; vTcamRuleId++)
        {
            if(vTcamRuleId < numPolicerEntries)
            {
                action.policer.policerEnable     = CPSS_DXCH_PCL_POLICER_ENABLE_COUNTER_ONLY_E;
                action.policer.policerId         = vTcamRuleId;

                ruleData.valid                   = GT_FALSE;
                ruleData.rule.pcl.actionPtr      = &action;
                ruleData.rule.pcl.maskPtr        = &mask;
                ruleData.rule.pcl.patternPtr     = &pattern;

                tcamRuleType.ruleType            = CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_PCL_E;
                tcamRuleType.rule.pcl.ruleFormat = appDemoIpfixKeyTypesIpv4Key;

                rc = cpssDxChVirtualTcamRuleWrite(devNum, appDemoIpfixFwAclVTcam,
                                                  vTcamRuleId, &ruleAttributes,
                                                  &tcamRuleType, &ruleData);
                if (GT_OK != rc)
                {
                    cpssOsPrintf("[%s:%d] cpssDxChVirtualTcamRuleWrite FAILED :"
                                 "devNum[%d],vTcamId [%d] vTcamRuleId[%d], rc[%d]\n",
                                 __FUNCNAME__, __LINE__, devNum, appDemoIpfixFwAclVTcam,
                                 vTcamRuleId, rc);
                    return rc;
                }

                if( debug_print_enable)
                {
                    cpssOsPrintf("[%s:%d] cpssDxChVirtualTcamRuleWrite SUCCESS :"
                                 "devNum[%d],vTcamId [%d], "
                                 "vTcamRuleId[%d], rc[%d]\n",__FUNCNAME__, __LINE__,
                                 devNum, appDemoIpfixFwAclVTcam,
                                 vTcamRuleId, rc);
                }
            }
            else
            {
                rc = GT_BAD_PARAM;
                cpssOsPrintf("[%s:%d] Firmware TCAM rules exceed the ipfix counters rc[%d]\n",
                             __FUNCNAME__, __LINE__, rc);
                return rc;
            }
        }
    }
    else
    {
        for(vTcamRuleId=0; vTcamRuleId < appDemoIpfixFwTcamNumOfShortRules; vTcamRuleId++)
        {
            rc = cpssDxChVirtualTcamRuleDelete(devNum, appDemoIpfixFwAclVTcam,
                                               vTcamRuleId);
            if (GT_OK != rc)
            {
                cpssOsPrintf("[%s:%d] cpssDxChVirtualTcamRuleDelete FAILED :"
                             " devNum[%d],vTcamId [%d] vTcamRuleId[%d],"
                             " rc[%d]\n",
                             __FUNCNAME__, __LINE__, devNum, appDemoIpfixFwAclVTcam,
                             vTcamRuleId, rc);
                return rc;
            }

            if( debug_print_enable)
            {
                cpssOsPrintf("[%s:%d] cpssDxChVirtualTcamRuleDelete SUCCESS :"
                             "devNum[%d],vTcamId [%d], "
                             "vTcamRuleId[%d], rc[%d]\n",
                             __FUNCNAME__, __LINE__, devNum, appDemoIpfixFwAclVTcam,
                             vTcamRuleId, rc);
            }
        }
    }

    if(debug_print_enable == GT_TRUE)
    {
        cpssOsPrintf("[%s:%d] prvFlowManagerBindIpfixCountersToFwRules successfully exiting\n",
                __FUNCNAME__, __LINE__);
    }

    return GT_OK;
}

static GT_STATUS prvFlowManagerVirtualTcamInit
(
    IN  GT_U8                           devNum,
    IN  GT_BOOL                         enable
)
{
    GT_STATUS                       rc = GT_OK;
    CPSS_DXCH_VIRTUAL_TCAM_INFO_STC vTcamInfo;

    cpssOsBzero((GT_CHAR*) &vTcamInfo, sizeof(vTcamInfo));

    if(debug_print_enable == GT_TRUE)
    {
        cpssOsPrintf("[%s:%d] prvFlowManagerVirtualTcamInit entered\n",__FUNCNAME__, __LINE__);
    }

    if(enable == GT_TRUE )
    {
        if((rc = cpssDxChVirtualTcamManagerCreate(devNum, NULL)) != GT_OK)
        {
            cpssOsPrintf("[%s:%d] cpssDxChVirtualTcamManagerCreate FAILED, rc [%d] devNum[%d]\n",
                         __FUNCNAME__, __LINE__, rc, devNum);
            return rc;
        }

        if((rc = cpssDxChVirtualTcamManagerDevListAdd(devNum, &devNum, 1)) != GT_OK)
        {
            cpssOsPrintf("[%s:%d] cpssDxChVirtualTcamManagerDevListAdd FAILED, rc [%d] devNum[%d]\n",
                         __FUNCNAME__, __LINE__, rc, devNum);
            return rc;
        }

        vTcamInfo.clientGroup        = 0;
        vTcamInfo.hitNumber          = 0;
        vTcamInfo.autoResize         = GT_FALSE;
        vTcamInfo.ruleAdditionMethod = CPSS_DXCH_VIRTUAL_TCAM_RULE_ADDITION_METHOD_LOGICAL_INDEX_E;

        /* Create vTCAM(0) - firmware (30B)*/
        vTcamInfo.ruleSize             = CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_STD_E;
        vTcamInfo.guaranteedNumOfRules = appDemoIpfixFwTcamNumOfShortRules;
        vTcamInfo.tcamSpaceUnmovable   = GT_TRUE;

        if(( rc = cpssDxChVirtualTcamCreate( devNum, appDemoIpfixFwAclVTcam, &vTcamInfo)) != GT_OK)
        {
            cpssOsPrintf("[%s:%d] cpssDxChVirtualTcamCreate FAILED, rc [%d] devNum[%d] vTcamId[%d]\n",
                          __FUNCNAME__, __LINE__, rc, devNum, appDemoIpfixFwAclVTcam);
            return rc;
        }

        /* Create vTCAM(1) - short default rules (30B)*/
        vTcamInfo.ruleSize             = CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_STD_E;
        vTcamInfo.autoResize           = GT_FALSE;
        vTcamInfo.guaranteedNumOfRules = APP_DEMO_IPFIX_MAX_PORT_NUM+3;
        /* 3 are default rules for IPV4 TCP, IPV4 UDP, Other */

        if((rc = cpssDxChVirtualTcamCreate(devNum, appDemoIpfixShortAclVTcam, &vTcamInfo)) != GT_OK)
        {
            cpssOsPrintf("[%s:%d] cpssDxChVirtualTcamCreate FAILED, rc [%d] devNum[%d] vTcamId[%d]\n",
                         __FUNCNAME__, __LINE__, rc, devNum, appDemoIpfixShortAclVTcam);
            return rc;
        }

        /* Create vTCAM(2) - long default rules (60B)*/
        vTcamInfo.ruleSize             = CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_EXT_E;
        vTcamInfo.autoResize           = GT_FALSE;
        vTcamInfo.guaranteedNumOfRules = APP_DEMO_IPFIX_MAX_PORT_NUM+2;
        /* 2 are default rules for IPV6 TCP, IPV6 UDP */

        if((rc = cpssDxChVirtualTcamCreate(devNum, appDemoIpfixLongAclVTcam, &vTcamInfo)) != GT_OK)
        {
            cpssOsPrintf("[%s:%d] cpssDxChVirtualTcamCreate FAILED, rc [%d] devNum[%d] vTcamId[%d]\n",
                         __FUNCNAME__, __LINE__, rc, devNum, appDemoIpfixLongAclVTcam);
            return rc;
        }
    }
    else
    {
        if(( rc = cpssDxChVirtualTcamRemove( devNum, appDemoIpfixFwAclVTcam)) != GT_OK)
        {
            cpssOsPrintf("[%s:%d] cpssDxChVirtualTcamRemove FAILED, rc [%d] devNum[%d] vTcamId[%d]\n",
                         __FUNCNAME__, __LINE__, rc, devNum, appDemoIpfixFwAclVTcam);
            return rc;
        }

        if(( rc = cpssDxChVirtualTcamRemove( devNum, appDemoIpfixShortAclVTcam)) != GT_OK)
        {
            cpssOsPrintf("[%s:%d] cpssDxChVirtualTcamRemove FAILED, rc [%d] devNum[%d] vTcamId[%d]\n",
                         __FUNCNAME__, __LINE__, rc, devNum, appDemoIpfixShortAclVTcam);
            return rc;
        }

        if(( rc = cpssDxChVirtualTcamRemove( devNum, appDemoIpfixLongAclVTcam)) != GT_OK)
        {
            cpssOsPrintf("[%s:%d] cpssDxChVirtualTcamRemove FAILED, rc [%d] devNum[%d] vTcamId[%d]\n",
                         __FUNCNAME__, __LINE__, rc, devNum, appDemoIpfixLongAclVTcam);
            return rc;
        }

        if((rc = cpssDxChVirtualTcamManagerDevListRemove(devNum, &devNum, 1)) != GT_OK)
        {
            cpssOsPrintf("[%s:%d] cpssDxChVirtualTcamManagerDevListRemove FAILED, rc [%d] devNum[%d]\n",
                         __FUNCNAME__, __LINE__, rc, devNum);
            return rc;
        }

        if((rc = cpssDxChVirtualTcamManagerDelete(devNum)) != GT_OK)
        {
            cpssOsPrintf("[%s:%d] cpssDxChVirtualTcamManagerDelete FAILED, rc [%d] devNum[%d]\n",
                         __FUNCNAME__, __LINE__, rc, devNum);
            return rc;
        }

    }

    if(debug_print_enable == GT_TRUE)
    {
        cpssOsPrintf("[%s:%d] prvFlowManagerVirtualTcamInit successfully exiting\n",__FUNCNAME__, __LINE__);
    }

    return GT_OK;
}

/* Pcl Initialization */
static GT_STATUS prvFlowManagerPclInit
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          pclLookupNum,
    IN  GT_BOOL                         enable
)
{
    GT_U32                          subLookupNum = 0;
    GT_U32                          portTxqNum = 0;
    GT_STATUS                       rc = GT_OK;
    GT_PORT_NUM                     portNum = 0;
    CPSS_INTERFACE_INFO_STC         interfaceInfo;
    CPSS_DXCH_PCL_LOOKUP_CFG_STC    lookupCfg;

    cpssOsBzero((GT_CHAR*) &interfaceInfo, sizeof(interfaceInfo));
    cpssOsBzero((GT_CHAR*) &lookupCfg, sizeof(lookupCfg));

    if(debug_print_enable == GT_TRUE)
    {
        cpssOsPrintf("[%s:%d] prvFlowManagerPclInit entered\n",__FUNCNAME__, __LINE__);
    }

    if( enable == GT_TRUE )
    {
        if(( rc = cpssDxChPclInit(devNum)) != GT_OK)
        {
            cpssOsPrintf("[%s:%d] cpssDxChPclInit FAILED, rc [%d] devNum[%d]\n",
                         __FUNCNAME__, __LINE__, rc, devNum);
            return rc;
        }

        /* Enable Ingress Pcl */
        if(( rc = cpssDxChPclIngressPolicyEnable(
                        devNum,GT_TRUE)) != GT_OK)
        {
            cpssOsPrintf("[%s:%d] cpssDxChPclIngressPolicyEnable FAILED, rc [%d] devNum[%d]\n",
                         __FUNCNAME__, __LINE__, rc, devNum);
            return rc;
        }

        subLookupNum = 0;

        for (portNum = 0; (portNum < APP_DEMO_IPFIX_MAX_PORT_NUM); portNum++)
        {
            if (CPSS_PORTS_BMP_IS_PORT_SET_MAC(
                        (&appDemoIpfixPortBitmap), portNum) == 0) continue;

            if(( rc = prvCpssDxChPortPhysicalPortMapCheckAndConvert(devNum,
                            portNum, PRV_CPSS_DXCH_PORT_TYPE_TXQ_E, &portTxqNum)) != GT_OK)
            {
                continue;
            }

            if(( rc = cpssDxChPclPortIngressPolicyEnable(
                            devNum, portNum, GT_TRUE)) != GT_OK)
            {
                cpssOsPrintf("[%s:%d] cpssDxChPclPortIngressPolicyEnable FAILED, rc [%d] devNum[%d],"
                             "portNum[%d]\n", __FUNCNAME__, __LINE__, rc, devNum, portNum);
                return rc;
            }

            if(( rc = cpssDxChPclPortLookupCfgTabAccessModeSet(
                            devNum, portNum, appDemoIpfixPclDirection,
                            pclLookupNum, subLookupNum,
                            CPSS_DXCH_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_BY_PORT_E)) != GT_OK )
            {
                cpssOsPrintf("[%s:%d] cpssDxChPclPortLookupCfgTabAccessModeSet FAILED, rc [%d] devNum[%d],"
                             "portNum[%d] pclLookupNum[%d] subLookupNum[%d] mode[%d]\n",
                             __FUNCNAME__, __LINE__, rc, devNum, portNum, pclLookupNum, subLookupNum,
                             CPSS_DXCH_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_BY_PORT_E);
                return rc;
            }

            interfaceInfo.type               = CPSS_INTERFACE_PORT_E;
            interfaceInfo.devPort.hwDevNum   = devNum;
            interfaceInfo.devPort.portNum    = portNum;

            lookupCfg.enableLookup           = GT_TRUE;
            lookupCfg.pclId                  = appDemoIpfixPclId;
            lookupCfg.groupKeyTypes.nonIpKey = appDemoIpfixKeyTypesNonIpKey;
            lookupCfg.groupKeyTypes.ipv4Key  = appDemoIpfixKeyTypesIpv4Key;
            lookupCfg.groupKeyTypes.ipv6Key  = appDemoIpfixKeyTypesIpv6Key;
            lookupCfg.udbKeyBitmapEnable     = GT_FALSE;
            lookupCfg.tcamSegmentMode        = CPSS_DXCH_PCL_TCAM_SEGMENT_MODE_1_TCAM_E;

            if(( rc = cpssDxChPclCfgTblSet(devNum,
                            &interfaceInfo, appDemoIpfixPclDirection,
                            appDemoIpfixLookupNum, &lookupCfg)) != GT_OK)
            {
                cpssOsPrintf("[%s:%d] cpssDxChPclCfgTblSet FAILED, rc [%d] devNum [%d] port [%d]\n",
                             __FUNCNAME__, __LINE__, rc, devNum, portNum);
                return rc;
            }
        }
    }
    else
    {
        for (portNum = 0; (portNum < APP_DEMO_IPFIX_MAX_PORT_NUM); portNum++)
        {
            if(( rc = cpssDxChPclPortIngressPolicyEnable(
                            devNum, portNum, enable)) != GT_OK)
            {
                cpssOsPrintf("[%s:%d] cpssDxChPclPortIngressPolicyEnable FAILED, rc [%d] devNum[%d],"
                             "portNum[%d] enable[%d]\n", __FUNCNAME__, __LINE__, rc, devNum, portNum, enable);
                return rc;
            }
        }

        /* Disable Ingress Pcl */
        if(( rc = cpssDxChPclIngressPolicyEnable(
                        devNum,enable)) != GT_OK)
        {
            cpssOsPrintf("[%s:%d] cpssDxChPclIngressPolicyEnable FAILED, rc [%d] devNum[%d] enable[%d]\n",
                         __FUNCNAME__, __LINE__, rc, devNum, enable);
            return rc;
        }

    }

    if(debug_print_enable == GT_TRUE)
    {
        cpssOsPrintf("[%s:%d] prvFlowManagerPclInit successfully exiting\n", __FUNCNAME__, __LINE__ );
    }

    return GT_OK;
}

/* Policer Initialization */
GT_STATUS prvFlowManagerPolicerInit
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          policerStage,
    IN  GT_BOOL                         enable
)
{
    GT_STATUS           rc = GT_OK;

    if(debug_print_enable == GT_TRUE)
    {
        cpssOsPrintf("[%s:%d] prvFlowManagerPolicerInit entered\n",__FUNCNAME__, __LINE__);
    }

    if(enable == GT_TRUE)
    {
        if(( rc = cpssDxChPolicerStageMeterModeSet( devNum, policerStage,
                        CPSS_DXCH_POLICER_STAGE_METER_MODE_FLOW_E)) != GT_OK )
        {
            cpssOsPrintf("[%s:%d] cpssDxChPolicerStageMeterModeSet FAILED, rc [%d] devNum [%d]\n",
                         __FUNCNAME__, __LINE__, rc, devNum);
            return rc;
        }

        if(( rc = cpssDxChPolicerCountingModeSet( devNum, policerStage,
                        CPSS_DXCH_POLICER_COUNTING_BILLING_IPFIX_E)) != GT_OK )
        {
            cpssOsPrintf("[%s:%d] cpssDxChPolicerCountingModeSet FAILED, rc [%d] devNum [%d]\n",
                         __FUNCNAME__, __LINE__, rc, devNum);
            return rc;
        }

        if(( rc = cpssDxCh3PolicerPacketSizeModeSet( devNum, policerStage,
                        CPSS_POLICER_PACKET_SIZE_L2_INCLUDE_E)) != GT_OK )
        {
            cpssOsPrintf("[%s:%d] cpssDxCh3PolicerPacketSizeModeSet FAILED, rc [%d] devNum [%d]\n",
                         __FUNCNAME__, __LINE__, rc, devNum);
            return rc;
        }
    }
    else
    {

        if(( rc = cpssDxChPolicerCountingModeSet( devNum, policerStage,
                        CPSS_DXCH_POLICER_COUNTING_DISABLE_E)) != GT_OK )
        {
            cpssOsPrintf("[%s:%d] cpssDxChPolicerCountingModeSet FAILED, rc [%d] devNum [%d]\n",
                         __FUNCNAME__, __LINE__, rc, devNum);
            return rc;
        }
    }

    if(debug_print_enable == GT_TRUE)
    {
        cpssOsPrintf("[%s:%d] prvFlowManagerPolicerInit successfully exiting\n",__FUNCNAME__, __LINE__ );
    }

    return GT_OK;
}

/* Ipfix Wrap-around config Initialization */
static GT_STATUS prvFlowManagerIpfixWrapAroundConfig
(
    IN  GT_U8                           devNum,
    IN  GT_BOOL                         enable
)
{
    GT_U32                                  numPolicerEntries = 0;
    GT_U32                                  ipfixEntryIndex = 0;
    GT_STATUS                               rc = GT_OK;
    CPSS_DXCH_IPFIX_ENTRY_STC               ipfixCntr;
    CPSS_DXCH_IPFIX_WRAPAROUND_CFG_STC      ipfixWaCfg;
    CPSS_DXCH_POLICER_MEMORY_CTRL_MODE_ENT  mode;

    cpssOsBzero((GT_CHAR*) &mode, sizeof(mode));
    cpssOsBzero((GT_CHAR*) &ipfixWaCfg, sizeof(ipfixWaCfg));
    cpssOsBzero((GT_CHAR*) &ipfixCntr, sizeof(ipfixCntr));

    if(debug_print_enable == GT_TRUE)
    {
        cpssOsPrintf("[%s:%d] prvFlowManagerIpfixWrapAroundConfig entered\n", __FUNCNAME__, __LINE__);
    }

    rc = cpssDxChPolicerMemorySizeModeGet(devNum, &mode, &numPolicerEntries, NULL, NULL);
    if(rc != GT_OK)
    {
        cpssOsPrintf("[%s:%d] cpssDxChPolicerMemorySizeModeGet FAILED, rc [%d] devNum [%d]\n",
                     __FUNCNAME__, __LINE__, rc, devNum);
        return rc;
    }

    if( debug_print_enable )
    {
        cpssOsPrintf("[%s:%d] Total policer entries in stage0=%d mode[%d]\n",__FUNCNAME__, __LINE__, numPolicerEntries, mode);
    }

    if( enable == GT_TRUE )
    {
        if((rc = cpssDxChIpfixDropCountModeSet(devNum,
                                               appDemoIpfixPolicerStage,
                                               CPSS_DXCH_IPFIX_DROP_COUNT_MODE_ALL_E))!= GT_OK)
        {
            cpssOsPrintf("[%s:%d] cpssDxChIpfixDropCountModeSet FAILED, rc [%d] devNum [%d]\n",
                         __FUNCNAME__, __LINE__, rc, devNum);
            return rc;
        }

        ipfixWaCfg.action = CPSS_DXCH_IPFIX_WRAPAROUND_ACTION_CLEAR_E;
        ipfixWaCfg.byteThreshold.l[0] = 0xFFFFFFFF;
        ipfixWaCfg.byteThreshold.l[1] = 0;
        ipfixWaCfg.packetThreshold = BIT_30 - 1;
        ipfixWaCfg.dropThreshold = BIT_30 - 1;
        if((rc = cpssDxChIpfixWraparoundConfSet(devNum,
                                                appDemoIpfixPolicerStage,
                                                &ipfixWaCfg)) != GT_OK )
        {
            cpssOsPrintf("[%s:%d] cpssDxChIpfixWraparoundConfSet FAILED, rc [%d] devNum [%d]\n",
                         __FUNCNAME__, __LINE__, rc, devNum);
            return rc;
        }

        /* WBC flush to ensure the mode is set to billing, even in scaled environment */
        rc = cpssDxChPolicerCountingWriteBackCacheFlush(devNum, appDemoIpfixPolicerStage);
        if (rc != GT_OK)
        {
            cpssOsPrintf("[%s:%d] cpssDxChPolicerCountingWriteBackCacheFlush FAILED: devNum[%d]\n",
                         __FUNCNAME__, __LINE__, devNum);
            return rc;
        }

        for(ipfixEntryIndex = 0; ipfixEntryIndex < numPolicerEntries; ipfixEntryIndex++)
        {
            /* Configuring sampling parameters to ipfix entry index */
            cpssOsMemSet(&ipfixCntr, 0, sizeof(ipfixCntr));
            rc = cpssDxChIpfixEntrySet(devNum,
                                       appDemoIpfixPolicerStage,
                                       ipfixEntryIndex,
                                       &ipfixCntr);
            if (GT_OK != rc)
            {
                cpssOsPrintf("[%s:%d] cpssDxChIpfixEntrySet FAILED :"
                             "devNum[%d], policerStage[%d],"
                             "ipfixEntryIndex[%d], rc[%d]\n",__FUNCNAME__, __LINE__,
                             devNum, appDemoIpfixPolicerStage, ipfixEntryIndex, rc);
                return rc;
            }
        }
    }
    else
    {

        if((rc = cpssDxChIpfixDropCountModeSet(devNum,
                                               appDemoIpfixPolicerStage,
                                               CPSS_DXCH_IPFIX_DROP_COUNT_MODE_METER_ONLY_E))
                                               != GT_OK)
        {
            cpssOsPrintf("[%s:%d] cpssDxChIpfixDropCountModeSet FAILED, rc [%d] devNum [%d]\n",
                         __FUNCNAME__, __LINE__, rc, devNum);
            return rc;
        }

        ipfixWaCfg.action = CPSS_DXCH_IPFIX_WRAPAROUND_ACTION_NONE_E;
        ipfixWaCfg.byteThreshold.l[0] = 0x100000;
        ipfixWaCfg.byteThreshold.l[1] = 0xF;
        ipfixWaCfg.packetThreshold = 0x1000;
        ipfixWaCfg.dropThreshold = 0x3FFFFFFF;
        if((rc = cpssDxChIpfixWraparoundConfSet(devNum,
                                                appDemoIpfixPolicerStage,
                                                &ipfixWaCfg)) != GT_OK )
        {
            cpssOsPrintf("[%s:%d] cpssDxChIpfixWraparoundConfSet FAILED, rc [%d] devNum [%d]\n",
                         __FUNCNAME__, __LINE__, rc, devNum);
            return rc;
        }


        /* WBC flush to ensure the mode is set to billing, even in scaled environment */
        rc = cpssDxChPolicerCountingWriteBackCacheFlush(devNum, appDemoIpfixPolicerStage);
        if (rc != GT_OK)
        {
            cpssOsPrintf("[%s:%d] cpssDxChPolicerCountingWriteBackCacheFlush FAILED: devNum[%d]\n",
                         __FUNCNAME__, __LINE__, devNum);
            return rc;
        }
    }

    if(debug_print_enable == GT_TRUE)
    {
        cpssOsPrintf("[%s:%d] prvFlowManagerIpfixWrapAroundConfig successfully exiting\n",
                     __FUNCNAME__, __LINE__ );
    }

    return GT_OK;
}

/* Callback listening to msg sent from fw */
static GT_STATUS prvFlowManagerDbUpdateByMsgPacketParse
(
    IN  GT_U8                           devNum,
    IN  GT_U8                           queueIdx,
    IN  GT_U32                          numOfBuff,
    IN  GT_U8                           *packetBuffsArrPtr[],
    IN  GT_U32                          buffLenArr[],
    IN  void                            *rxParamsPtr
)
{
    GT_STATUS                   rc = GT_OK;
    CPSS_DXCH_NET_RX_PARAMS_STC tempRxParams;
    PRV_CPSS_DXCH_FLOW_MANAGER_STC *appDemoIpfixFlowMngDbPtr = NULL;

    cpssOsBzero((GT_CHAR*) &tempRxParams, sizeof(tempRxParams));

    if (debug_print_enable == GT_TRUE)
    {
        GT_U32 iter;
        GT_U32 offset = 0;
        GT_U32 jj;
        GT_U8* bufferPtr = &packetBuffsArrPtr[0][0];
        GT_U32 length = buffLenArr[0];
        GT_U32 totalLen = 0;

        cpssOsPrintf("[%s:%d] prvFlowManagerDbUpdateByMsgPacketParse entered\n",__FUNCNAME__, __LINE__);

        /* calc totalLen */
        for(jj = 0; jj < numOfBuff; jj++)
        {
            totalLen += buffLenArr[jj];
        }

        cpssOsPrintf("Total Len of rx pkt[%d]\n Packet received:\n",totalLen);

        /* Printing the packet received */
        for(jj = 0; jj < numOfBuff; jj++)
        {
            bufferPtr = &packetBuffsArrPtr[jj][0];
            length = buffLenArr[jj];

            for(iter = 0; iter < length; iter++)
            {
                if((offset & 0x0F) == 0)
                {
                    cpssOsPrintf("0x%4.4x :", offset);
                }
                cpssOsPrintf(" %2.2x", bufferPtr[iter]);

                if((offset & 0x0F) == 0x0F)
                {
                    cpssOsPrintf("\n");
                }

                offset++;
            }/*iter*/
        }
        cpssOsPrintf("\n End of pkt \n");
    }

    tempRxParams = *(CPSS_DXCH_NET_RX_PARAMS_STC *)rxParamsPtr;

    if (debug_print_enable == GT_TRUE)
    {
        cpssOsPrintf("[%s:%d] Dsa Tag Info: cpu code=%d, queue_num=%d, devNum=%d, portNum=%d\n",
                      __FUNCNAME__, __LINE__, tempRxParams.dsaParam.dsaInfo.toCpu.cpuCode,
                      queueIdx,
                      tempRxParams.dsaParam.dsaInfo.toCpu.hwDevNum,
                      tempRxParams.dsaParam.dsaInfo.toCpu.interface.portNum);
    }


    if (tempRxParams.dsaParam.dsaInfo.toCpu.cpuCode == appDemoIpfixMsgCpuCode)
    {
        appDemoIpfixFlowMngDbPtr = prvCpssDxChFlowManagerDbFlowMngGet(appDemoIpfixFlowMngId);
        if (appDemoIpfixFlowMngDbPtr == NULL)
        {
            rc = GT_FAIL;
            if(debug_print_enable == GT_TRUE)
            {
                cpssOsPrintf("[%s:%d] Flow manager DB doesn't exist rc[%d]\n",
                             __FUNCNAME__, __LINE__, rc);
            }
            return rc;
        }

        rc = cpssDxChFlowManagerMessageParseAndCpssDbUpdate(devNum,
                                                            numOfBuff, packetBuffsArrPtr,
                                                            buffLenArr);
        if (GT_OK != rc)
        {
            cpssOsPrintf("[%s:%d] cpssDxChFlowManagerMessageParseAndCpssDbUpdate FAILED, rc [%d] devNum [%d]\n",
                         __FUNCNAME__, __LINE__, rc, devNum);
            return rc;
        }
    }

    if(debug_print_enable == GT_TRUE)
    {
        cpssOsPrintf("[%s:%d] prvFlowManagerDbUpdateByMsgPacketParse successfully exiting\n",__FUNCNAME__, __LINE__);
    }

    return GT_OK;
}

GT_STATUS appDemoIpfixUdbConfigPortDump
(
    IN  GT_U32                              flowMngId,
    IN  GT_U8                               devNum,
    IN  GT_PORT_NUM                         portNum
)
{
    GT_STATUS rc = GT_OK;
    GT_U32 i=0;
    GT_U32 keyTableIndex = 0;

    GT_UNUSED_PARAM(flowMngId);

    if(debug_print_enable == GT_TRUE)
    {
        cpssOsPrintf("[%s:%d] appDemoIpfixUdbConfigPortDump entered\n",__FUNCNAME__, __LINE__);
    }

    if(appDemoIpfixUdbCfg == NULL)
    {
        if(appDemoIpfixTotalUdbConfigCount == 0)
        {
            rc = GT_OK;
            cpssOsPrintf(" No UDBs configured\n");
            return rc;
        }
        rc = GT_BAD_STATE;
        cpssOsPrintf("[%s:%d] Failed to retrieve UDB database rc[%d]\n", __FUNCNAME__, __LINE__, rc);
        return rc;
    }

    for(keyTableIndex = 0; keyTableIndex < APP_DEMO_IPFIX_KEY_TABLE_INDEX_CNS; keyTableIndex++)
    {
        if(appDemoIpfixUdbCfg->keyTableUdbCfg[keyTableIndex].ipv4Key != NULL)
        {
            if((( appDemoIpfixUdbCfg->keyIndexMapCfg[devNum][portNum].keyIndexArr[CPSS_DXCH_FLOW_MANAGER_PACKET_TYPE_IPV4_TCP_E] == keyTableIndex )
              || appDemoIpfixUdbCfg->keyIndexMapCfg[devNum][portNum].keyIndexArr[CPSS_DXCH_FLOW_MANAGER_PACKET_TYPE_IPV4_UDP_E] == keyTableIndex ))
            {

                cpssOsPrintf("-------------- keyTableIndex = %d -----------------\n",keyTableIndex);

                for(i=0;i<2;i++)
                {
                    cpssOsPrintf("pclIdIndex[%d] = %d \n", i, appDemoIpfixUdbCfg->keyTableUdbCfg[keyTableIndex].ipv4Key->pclIdIndex[i]);
                }

                for(i=0;i<2;i++)
                {
                    cpssOsPrintf("vlanIdIndex[%d] = %d \n", i, appDemoIpfixUdbCfg->keyTableUdbCfg[keyTableIndex].ipv4Key->vlanIdIndex[i]);
                }

                cpssOsPrintf("srcPortIndex = %d \n", appDemoIpfixUdbCfg->keyTableUdbCfg[keyTableIndex].ipv4Key->srcPortIndex);

                cpssOsPrintf("appFlowSubTempIndex = %d \n", appDemoIpfixUdbCfg->keyTableUdbCfg[keyTableIndex].ipv4Key->appFlowSubTempIndex);

                for(i=0;i<4;i++)
                {
                    cpssOsPrintf("sipv4Index[%d] = %d \n", i, appDemoIpfixUdbCfg->keyTableUdbCfg[keyTableIndex].ipv4Key->sipv4Index[i]);
                }

                for(i=0;i<4;i++)
                {
                    cpssOsPrintf("dipv4Index[%d] = %d \n", i, appDemoIpfixUdbCfg->keyTableUdbCfg[keyTableIndex].ipv4Key->dipv4Index[i]);
                }

                if(appDemoIpfixUdbCfg->keyTableUdbCfg[keyTableIndex].ipv4Key->pktType == APP_DEMO_IPFIX_PACKET_TYPE_IPV4_TCP_E)
                {
                    for(i=0;i<2;i++)
                    {
                        cpssOsPrintf("tcpSrcPortIndex[%d] = %d \n", i,
                                     appDemoIpfixUdbCfg->keyTableUdbCfg[keyTableIndex].ipv4Key->protocol.protoSrcPortIndex[i]);
                    }

                    for(i=0;i<2;i++)
                    {
                        cpssOsPrintf("tcpDstPortIndex[%d] = %d \n", i,
                                     appDemoIpfixUdbCfg->keyTableUdbCfg[keyTableIndex].ipv4Key->protocol.protoDstPortIndex[i]);
                    }
                }
                else
                {
                    for(i=0;i<2;i++)
                    {
                        cpssOsPrintf("udpSrcPortIndex[%d] = %d \n", i,
                                     appDemoIpfixUdbCfg->keyTableUdbCfg[keyTableIndex].ipv4Key->protocol.protoSrcPortIndex[i]);
                    }

                    for(i=0;i<2;i++)
                    {
                        cpssOsPrintf("udpDstPortIndex[%d] = %d \n", i,
                                     appDemoIpfixUdbCfg->keyTableUdbCfg[keyTableIndex].ipv4Key->protocol.protoDstPortIndex[i]);
                    }
                }

                i = 0;
                while( appDemoIpfixUdbCfg->keyTableUdbCfg[keyTableIndex].ipv4Key->notValidIndex[i] != APP_DEMO_IPFIX_NOT_USED_UDB)
                {
                    cpssOsPrintf("notValidIndex[%d] = %d \n", i,appDemoIpfixUdbCfg->keyTableUdbCfg[keyTableIndex].ipv4Key->notValidIndex[i]);
                    i++;
                }

                cpssOsPrintf("-------------------------------------------------------\n");
            }
        }
        else if(appDemoIpfixUdbCfg->keyTableUdbCfg[keyTableIndex].otherTypeKey != NULL)
        {
            if((( appDemoIpfixUdbCfg->keyIndexMapCfg[devNum][portNum].keyIndexArr[CPSS_DXCH_FLOW_MANAGER_PACKET_TYPE_OTHER_E]
                                         == keyTableIndex )
              || appDemoIpfixUdbCfg->keyIndexMapCfg[devNum][portNum].keyIndexArr[CPSS_DXCH_FLOW_MANAGER_PACKET_TYPE_OTHER_E]
                                         == keyTableIndex ))
            {

                cpssOsPrintf("-------------- keyTableIndex = %d -----------------\n",keyTableIndex);

                for(i=0;i<2;i++)
                {
                    cpssOsPrintf("pclIdIndex[%d] = %d \n", i, appDemoIpfixUdbCfg->keyTableUdbCfg[keyTableIndex].otherTypeKey->pclIdIndex[i]);
                }

                for(i=0;i<2;i++)
                {
                    cpssOsPrintf("vlanIdIndex[%d] = %d \n", i, appDemoIpfixUdbCfg->keyTableUdbCfg[keyTableIndex].otherTypeKey->vlanIdIndex[i]);
                }

                cpssOsPrintf("srcPortIndex = %d \n", appDemoIpfixUdbCfg->keyTableUdbCfg[keyTableIndex].otherTypeKey->srcPortIndex);

                cpssOsPrintf("appFlowSubTempIndex = %d \n", appDemoIpfixUdbCfg->keyTableUdbCfg[keyTableIndex].otherTypeKey->appFlowSubTempIndex);

                for(i=0;i<6;i++)
                {
                    cpssOsPrintf("smacIndex[%d] = %d \n", i, appDemoIpfixUdbCfg->keyTableUdbCfg[keyTableIndex].otherTypeKey->smacIndex[i]);
                }

                for(i=0;i<6;i++)
                {
                    cpssOsPrintf("dmacIndex[%d] = %d \n", i, appDemoIpfixUdbCfg->keyTableUdbCfg[keyTableIndex].otherTypeKey->dmacIndex[i]);
                }

                i = 0;
                while( appDemoIpfixUdbCfg->keyTableUdbCfg[keyTableIndex].otherTypeKey->notValidIndex[i] != APP_DEMO_IPFIX_NOT_USED_UDB)
                {
                    cpssOsPrintf("notValidIndex[%d] = %d \n", i,appDemoIpfixUdbCfg->keyTableUdbCfg[keyTableIndex].otherTypeKey->notValidIndex[i]);
                    i++;
                }

                cpssOsPrintf("-------------------------------------------------------\n");
            }
        }
        else
        {
            continue;
        }
    }

    if(debug_print_enable == GT_TRUE)
    {
        cpssOsPrintf("[%s:%d] appDemoIpfixUdbConfigPortDump successfully exiting\n", __FUNCNAME__, __LINE__);
    }

    return rc;
}

static GT_STATUS prvFlowManagerIpv4FlowInfoDump
(
    IN  GT_U32                              flowMngId,
    IN  GT_U32                              keyTableIndex,
    IN  GT_U32                              flowId,
    IN  GT_U8                               *keyData
)
{
    GT_STATUS rc = GT_OK;
    GT_U32 i=0;
    GT_U32 startOfKeyData = 0;
    GT_U32 temp = 0;
    GT_U32 vlanIndex, srcPortIndex, appTempIndex, sipIndex, dipIndex, protoSrcPortIndex, protoDstPortIndex;
    GT_U16 pclIdVal = 0;
    GT_U16 vlanIdVal = 0;
    GT_U16 protoSrcPortVal =0;
    GT_U16 protoDstPortVal = 0;
    GT_BOOL keyConfigFound = GT_FALSE;

    GT_UNUSED_PARAM(flowMngId);

    if(keyData == NULL)
    {
        rc = GT_BAD_STATE;
        cpssOsPrintf("[%s:%d] keyData corrupted\n",__FUNCNAME__, __LINE__);
        return rc;
    }

    if((APP_DEMO_IPFIX_PACKET_TYPE_IPV4_TCP_E != appDemoIpfixUdbCfg->keyTableUdbCfg[keyTableIndex].ipv4Key->pktType) &&
      ((APP_DEMO_IPFIX_PACKET_TYPE_IPV4_UDP_E != appDemoIpfixUdbCfg->keyTableUdbCfg[keyTableIndex].ipv4Key->pktType)))
    {
        rc = GT_BAD_PARAM;
        cpssOsPrintf("[%s:%d] IPv4 TCP/UDP not configured for keyTableIndex[%d] flowId[%d]\n", __FUNCNAME__, __LINE__,
                     keyTableIndex, flowId);
        return rc;
    }

    if(appDemoIpfixUdbCfg->keyTableUdbCfg[keyTableIndex].ipv4Key->pktType == APP_DEMO_IPFIX_PACKET_TYPE_IPV4_TCP_E)
    {
        cpssOsPrintf("----------------- IPv4-TCP Flow -----------------\n");
    }
    else
    {
        cpssOsPrintf("----------------- IPv4-UDP Flow -----------------\n");
    }

    cpssOsPrintf(" Flow Id         : %d\n", flowId);
    cpssOsPrintf(" Key Table Index : %d\n", keyTableIndex);

    if(debug_print_enable)
    {
        for(i = 0; i< 2; i++)
        {
            cpssOsPrintf(" PCL ID UDB Index[%d] = %d\n", i,
                          appDemoIpfixUdbCfg->keyTableUdbCfg[keyTableIndex].ipv4Key->pclIdIndex[i]);
        }
    }

    temp = 0;
    for(i = 0; i < 2; i++)
    {
        if( appDemoIpfixUdbCfg->keyTableUdbCfg[keyTableIndex].ipv4Key->pclIdIndex[i] == APP_DEMO_IPFIX_NOT_USED_UDB )
        {
            if(debug_print_enable)
            {
                cpssOsPrintf(" PCL ID[%d]          : NA\n", i);
            }
        }
        else
        {
            startOfKeyData = i+1;
            if(debug_print_enable)
            {
                cpssOsPrintf(" PCL ID[%d]         : %d\n", i,
                             appDemoIpfixUdbCfg->keyTableUdbCfg[keyTableIndex].ipv4Key->pclIdIndex[i]);
            }
            temp = appDemoIpfixUdbCfg->keyTableUdbCfg[keyTableIndex].ipv4Key->pclIdIndex[i];
            pclIdVal = (i == 0) ? ((temp | 0xFF00) << 8) : (temp | pclIdVal);
            keyConfigFound = GT_TRUE;
        }
    }
    if(keyConfigFound == GT_TRUE)
    {
        cpssOsPrintf(" PCL ID          : %d\n", pclIdVal);
        keyConfigFound = GT_FALSE;
    }

    if(debug_print_enable)
    {
        for(i = 0; i< 2; i++)
        {
            cpssOsPrintf(" VLAN ID UDB Index[%d] = %d\n", i,
                          appDemoIpfixUdbCfg->keyTableUdbCfg[keyTableIndex].ipv4Key->vlanIdIndex[i]);
        }
    }

    temp = 0;
    for(i = 0; i < 2; i++)
    {
        if( appDemoIpfixUdbCfg->keyTableUdbCfg[keyTableIndex].ipv4Key->vlanIdIndex[i] == APP_DEMO_IPFIX_NOT_USED_UDB)
        {
            if(debug_print_enable)
            {
                cpssOsPrintf(" VLAN ID[%d]         : NA\n", i);
            }
        }
        else
        {
            vlanIndex = appDemoIpfixUdbCfg->keyTableUdbCfg[keyTableIndex].ipv4Key->vlanIdIndex[i] - startOfKeyData;
            if(debug_print_enable)
            {
                cpssOsPrintf(" VLAN ID[%d]         : %d\n", i, (*(keyData+vlanIndex)));
            }
            temp = *(keyData+vlanIndex);
            vlanIdVal = (i == 0) ? ((temp | 0x0F00) << 8): (*(keyData+vlanIndex) | vlanIdVal);
            keyConfigFound = GT_TRUE;
        }
    }
    if(keyConfigFound == GT_TRUE)
    {
        cpssOsPrintf(" VLAN ID         : %d\n", vlanIdVal);
        keyConfigFound = GT_FALSE;
    }

    if(debug_print_enable)
    {
        cpssOsPrintf(" Source Port UDB Index[%d] = %d\n", i,
                appDemoIpfixUdbCfg->keyTableUdbCfg[keyTableIndex].ipv4Key->srcPortIndex);
    }

    if( appDemoIpfixUdbCfg->keyTableUdbCfg[keyTableIndex].ipv4Key->srcPortIndex == APP_DEMO_IPFIX_NOT_USED_UDB)
    {
        if(debug_print_enable)
        {
            cpssOsPrintf(" Source Port         : NA\n");
        }
    }
    else
    {
        srcPortIndex = appDemoIpfixUdbCfg->keyTableUdbCfg[keyTableIndex].ipv4Key->srcPortIndex - startOfKeyData;
        cpssOsPrintf(" Source Port     : %d\n", *(keyData+srcPortIndex));
    }

    if(debug_print_enable)
    {
        cpssOsPrintf(" Applicable Flow Sub-template UDB Index[%d] = %d\n", i,
                appDemoIpfixUdbCfg->keyTableUdbCfg[keyTableIndex].ipv4Key->appFlowSubTempIndex);
    }

    if( appDemoIpfixUdbCfg->keyTableUdbCfg[keyTableIndex].ipv4Key->appFlowSubTempIndex == APP_DEMO_IPFIX_NOT_USED_UDB)
    {
        if(debug_print_enable)
        {
            cpssOsPrintf(" Applicable Flow Sub-template : NA\n");
        }
    }
    else
    {
        appTempIndex = appDemoIpfixUdbCfg->keyTableUdbCfg[keyTableIndex].ipv4Key->appFlowSubTempIndex - startOfKeyData;
        cpssOsPrintf(" Applicable Flow Sub-template : %d \n", *(keyData+appTempIndex));
    }

    if(debug_print_enable)
    {
        for(i = 0; i< 4; i++)
        {
            cpssOsPrintf(" SIPv4 UDB Index[%d] = %d\n", i,
                          appDemoIpfixUdbCfg->keyTableUdbCfg[keyTableIndex].ipv4Key->sipv4Index[i]);
        }
    }

    cpssOsPrintf(" SIPv4           : ");
    for(i = 0; i < 4; i++)
    {
        if( appDemoIpfixUdbCfg->keyTableUdbCfg[keyTableIndex].ipv4Key->sipv4Index[i] == APP_DEMO_IPFIX_NOT_USED_UDB)
        {
            if(debug_print_enable)
            {
                cpssOsPrintf("NA");
                cpssOsPrintf("%s",(i < 3)? ":":"\n");
            }
        }
        else
        {
            sipIndex = appDemoIpfixUdbCfg->keyTableUdbCfg[keyTableIndex].ipv4Key->sipv4Index[i] - startOfKeyData;
            cpssOsPrintf("%02d", *(keyData+sipIndex));
            cpssOsPrintf("%s",(i < 3)? ":":"\n");
        }
    }

    if(debug_print_enable)
    {
        for(i = 0; i< 4; i++)
        {
            cpssOsPrintf(" DIPv4 UDB Index[%d] = %d\n", i,
                          appDemoIpfixUdbCfg->keyTableUdbCfg[keyTableIndex].ipv4Key->dipv4Index[i]);
        }
    }

    cpssOsPrintf(" DIPv4           : ");
    for(i = 0; i < 4; i++)
    {
        if( appDemoIpfixUdbCfg->keyTableUdbCfg[keyTableIndex].ipv4Key->dipv4Index[i] == APP_DEMO_IPFIX_NOT_USED_UDB)
        {
            if(debug_print_enable)
            {
                cpssOsPrintf("NA");
                cpssOsPrintf("%s",(i < 3)? ":":"\n");
            }
        }
        else
        {
            dipIndex = appDemoIpfixUdbCfg->keyTableUdbCfg[keyTableIndex].ipv4Key->dipv4Index[i] - startOfKeyData;
            cpssOsPrintf("%02d", *(keyData+dipIndex));
            cpssOsPrintf("%s",(i < 3)? ":":"\n");
        }
    }

    temp = 0;
    if( appDemoIpfixUdbCfg->keyTableUdbCfg[keyTableIndex].ipv4Key->pktType == APP_DEMO_IPFIX_PACKET_TYPE_IPV4_TCP_E )
    {
        if(debug_print_enable)
        {
            for(i = 0; i< 2; i++)
            {
                cpssOsPrintf(" TCP Src Port UDB Index[%d] = %d\n", i,
                        appDemoIpfixUdbCfg->keyTableUdbCfg[keyTableIndex].ipv4Key->protocol.protoSrcPortIndex[i]);
            }
        }

        for(i = 0; i < 2; i++)
        {
            if( appDemoIpfixUdbCfg->keyTableUdbCfg[keyTableIndex].ipv4Key->protocol.protoSrcPortIndex[i] == APP_DEMO_IPFIX_NOT_USED_UDB)
            {
                if(debug_print_enable)
                {
                    cpssOsPrintf(" TCP Src Port[%d]          : NA\n",i);
                }
            }
            else
            {
                protoSrcPortIndex = appDemoIpfixUdbCfg->keyTableUdbCfg[keyTableIndex].ipv4Key->protocol.protoSrcPortIndex[i] - startOfKeyData;
                if(debug_print_enable)
                {
                    if( appDemoIpfixUdbCfg->keyTableUdbCfg[keyTableIndex].ipv4Key->pktType == APP_DEMO_IPFIX_PACKET_TYPE_IPV4_TCP_E )
                    {
                        cpssOsPrintf(" TCP Src Port[%d]         : %d\n", i, (*(keyData+protoSrcPortIndex)));
                    }
                }
                temp = *(keyData+protoSrcPortIndex);
                protoSrcPortVal = (i == 0) ? ((temp | 0xFF00) << 8) : (*(keyData+protoSrcPortIndex) | protoSrcPortVal);
                keyConfigFound = GT_TRUE;
            }
        }
        if(keyConfigFound == GT_TRUE)
        {
            cpssOsPrintf(" TCP Source Port : %d\n", protoSrcPortVal);
            keyConfigFound = GT_FALSE;
        }

        if(debug_print_enable)
        {
            for(i = 0; i< 2; i++)
            {
                cpssOsPrintf(" TCP Dst Port UDB Index[%d] = %d\n", i,
                        appDemoIpfixUdbCfg->keyTableUdbCfg[keyTableIndex].ipv4Key->protocol.protoDstPortIndex[i]);
            }
        }

        temp = 0;
        for(i = 0; i < 2; i++)
        {
            if( appDemoIpfixUdbCfg->keyTableUdbCfg[keyTableIndex].ipv4Key->protocol.protoDstPortIndex[i] == APP_DEMO_IPFIX_NOT_USED_UDB)
            {
                if(debug_print_enable)
                {
                    cpssOsPrintf(" TCP Dst Port[%d]          : NA\n",i);
                }
            }
            else
            {
                protoDstPortIndex = appDemoIpfixUdbCfg->keyTableUdbCfg[keyTableIndex].ipv4Key->protocol.protoDstPortIndex[i] - startOfKeyData;
                if(debug_print_enable)
                {
                    cpssOsPrintf(" TCP Dst Port[%d]         : %d\n", i, (*(keyData+protoDstPortIndex)));
                }
                temp = *(keyData+protoDstPortIndex);
                protoDstPortVal = (i == 0) ? ((temp | 0xFF00) << 8) : (*(keyData+protoDstPortIndex) | protoDstPortVal);
                keyConfigFound = GT_TRUE;
            }
        }
        if(keyConfigFound == GT_TRUE)
        {
            cpssOsPrintf(" TCP Destination Port  : %d\n", protoDstPortVal);
            keyConfigFound = GT_FALSE;
        }
        cpssOsPrintf("---------------- End of IPv4-TCP flow ---------------------------\n");
    }
    else
    {
        if(debug_print_enable)
        {
            for(i = 0; i< 2; i++)
            {
                cpssOsPrintf(" UDP Src Port UDB Index[%d] = %d\n", i,
                        appDemoIpfixUdbCfg->keyTableUdbCfg[keyTableIndex].ipv4Key->protocol.protoSrcPortIndex[i]);
            }
        }

        for(i = 0; i < 2; i++)
        {
            if( appDemoIpfixUdbCfg->keyTableUdbCfg[keyTableIndex].ipv4Key->protocol.protoSrcPortIndex[i] == APP_DEMO_IPFIX_NOT_USED_UDB)
            {
                if(debug_print_enable)
                {
                    cpssOsPrintf(" UDP Src Port[%d]          : NA\n",i);
                }
            }
            else
            {
                protoSrcPortIndex = appDemoIpfixUdbCfg->keyTableUdbCfg[keyTableIndex].ipv4Key->protocol.protoSrcPortIndex[i] - startOfKeyData;
                if(debug_print_enable)
                {
                    if( appDemoIpfixUdbCfg->keyTableUdbCfg[keyTableIndex].ipv4Key->pktType == APP_DEMO_IPFIX_PACKET_TYPE_IPV4_UDP_E )
                    {
                        cpssOsPrintf(" UDP Src Port[%d]         : %d\n", i, (*(keyData+protoSrcPortIndex)));
                    }
                }
                temp = *(keyData+protoSrcPortIndex);
                protoSrcPortVal = (i == 0) ? ((temp | 0xFF00) << 8) : (*(keyData+protoSrcPortIndex) | protoSrcPortVal);
                keyConfigFound = GT_TRUE;
            }
        }
        if(keyConfigFound == GT_TRUE)
        {
            cpssOsPrintf(" UDP Source Port : %d\n", protoSrcPortVal);
            keyConfigFound = GT_FALSE;
        }

        if(debug_print_enable)
        {
            for(i = 0; i< 2; i++)
            {
                cpssOsPrintf(" UDP Dst Port UDB Index[%d] = %d\n", i,
                        appDemoIpfixUdbCfg->keyTableUdbCfg[keyTableIndex].ipv4Key->protocol.protoDstPortIndex[i]);
            }
        }

        temp = 0;
        for(i = 0; i < 2; i++)
        {
            if( appDemoIpfixUdbCfg->keyTableUdbCfg[keyTableIndex].ipv4Key->protocol.protoDstPortIndex[i] == APP_DEMO_IPFIX_NOT_USED_UDB)
            {
                if(debug_print_enable)
                {
                    cpssOsPrintf(" UDP Dst Port[%d]          : NA\n",i);
                }
            }
            else
            {
                protoDstPortIndex = appDemoIpfixUdbCfg->keyTableUdbCfg[keyTableIndex].ipv4Key->protocol.protoDstPortIndex[i] - startOfKeyData;
                if(debug_print_enable)
                {
                    cpssOsPrintf(" UDP Dst Port[%d]         : %d\n", i, (*(keyData+protoDstPortIndex)));
                }
                temp = *(keyData+protoDstPortIndex);
                protoDstPortVal = (i == 0) ? ((temp | 0xFF00) << 8) : (*(keyData+protoDstPortIndex) | protoDstPortVal);
                keyConfigFound = GT_TRUE;
            }
        }
        if(keyConfigFound == GT_TRUE)
        {
            cpssOsPrintf(" UDP Destination Port  : %d\n", protoDstPortVal);
            keyConfigFound = GT_FALSE;
        }
        cpssOsPrintf("---------------- End of IPv4-UDP flow ---------------------------\n");
    }

    return rc;
}

static GT_STATUS prvFlowManagerIpv6FlowInfoDump
(
    IN  GT_U32                              flowMngId,
    IN  GT_U32                              keyTableIndex,
    IN  GT_U32                              flowId,
    IN  GT_U8                               *keyData
)
{
    GT_UNUSED_PARAM(flowMngId);
    GT_UNUSED_PARAM(keyTableIndex);
    GT_UNUSED_PARAM(flowId);
    GT_UNUSED_PARAM(keyData);

    return GT_OK;
}

static GT_STATUS prvFlowManagerOtherFlowInfoDump
(
    IN  GT_U32                              flowMngId,
    IN  GT_U32                              keyTableIndex,
    IN  GT_U32                              flowId,
    IN  GT_U8                               *keyData
)
{
    GT_STATUS rc = GT_OK;
    GT_U32 i=0;
    GT_U32 startOfKeyData = 0;
    GT_U32 temp = 0;
    GT_U32 vlanIndex, srcPortIndex, appTempIndex, smacIndex, dmacIndex;
    GT_U16 pclIdVal = 0;
    GT_U16 vlanIdVal = 0;
    GT_BOOL keyConfigFound = GT_FALSE;

    GT_UNUSED_PARAM(flowMngId);

    if(keyData == NULL)
    {
        rc = GT_BAD_STATE;
        cpssOsPrintf("[%s:%d] keyData corrupted\n",__FUNCNAME__, __LINE__);
        return rc;
    }

    cpssOsPrintf("----------------- OTHER Flow -----------------\n");

    cpssOsPrintf(" Flow Id         : %d\n", flowId);
    cpssOsPrintf(" Key Table Index : %d\n", keyTableIndex);

    if(debug_print_enable)
    {
        for(i = 0; i< 2; i++)
        {
            cpssOsPrintf(" PCL ID UDB Index[%d] = %d\n", i,
                          appDemoIpfixUdbCfg->keyTableUdbCfg[keyTableIndex].otherTypeKey->pclIdIndex[i]);
        }
    }

    temp = 0;
    for(i = 0; i < 2; i++)
    {
        if( appDemoIpfixUdbCfg->keyTableUdbCfg[keyTableIndex].otherTypeKey->pclIdIndex[i] == APP_DEMO_IPFIX_NOT_USED_UDB )
        {
            if(debug_print_enable)
            {
                cpssOsPrintf(" PCL ID[%d]          : NA\n", i);
            }
        }
        else
        {
            startOfKeyData = i+1;
            if(debug_print_enable)
            {
                cpssOsPrintf(" PCL ID[%d]         : %d\n", i,
                             appDemoIpfixUdbCfg->keyTableUdbCfg[keyTableIndex].otherTypeKey->pclIdIndex[i]);
            }
            temp = appDemoIpfixUdbCfg->keyTableUdbCfg[keyTableIndex].otherTypeKey->pclIdIndex[i];
            pclIdVal = (i == 0) ? ((temp | 0xFF00) << 8) : (temp | pclIdVal);
            keyConfigFound = GT_TRUE;
        }
    }
    if(keyConfigFound == GT_TRUE)
    {
        cpssOsPrintf(" PCL ID          : %d\n", pclIdVal);
        keyConfigFound = GT_FALSE;
    }

    if(debug_print_enable)
    {
        for(i = 0; i< 2; i++)
        {
            cpssOsPrintf(" VLAN ID UDB Index[%d] = %d\n", i,
                          appDemoIpfixUdbCfg->keyTableUdbCfg[keyTableIndex].otherTypeKey->vlanIdIndex[i]);
        }
    }

    temp = 0;
    for(i = 0; i < 2; i++)
    {
        if( appDemoIpfixUdbCfg->keyTableUdbCfg[keyTableIndex].otherTypeKey->vlanIdIndex[i] == APP_DEMO_IPFIX_NOT_USED_UDB)
        {
            if(debug_print_enable)
            {
                cpssOsPrintf(" VLAN ID[%d]         : NA\n", i);
            }
        }
        else
        {
            vlanIndex = appDemoIpfixUdbCfg->keyTableUdbCfg[keyTableIndex].otherTypeKey->vlanIdIndex[i] - startOfKeyData;
            if(debug_print_enable)
            {
                cpssOsPrintf(" VLAN ID[%d]         : %d\n", i, (*(keyData+vlanIndex)));
            }
            temp = *(keyData+vlanIndex);
            vlanIdVal = (i == 0) ? ((temp | 0x0F00) << 8): (*(keyData+vlanIndex) | vlanIdVal);
            keyConfigFound = GT_TRUE;
        }
    }
    if(keyConfigFound == GT_TRUE)
    {
        cpssOsPrintf(" VLAN ID         : %d\n", vlanIdVal);
        keyConfigFound = GT_FALSE;
    }

    if(debug_print_enable)
    {
        cpssOsPrintf(" Source Port UDB Index[%d] = %d\n", i,
                appDemoIpfixUdbCfg->keyTableUdbCfg[keyTableIndex].otherTypeKey->srcPortIndex);
    }

    if( appDemoIpfixUdbCfg->keyTableUdbCfg[keyTableIndex].otherTypeKey->srcPortIndex == APP_DEMO_IPFIX_NOT_USED_UDB)
    {
        if(debug_print_enable)
        {
            cpssOsPrintf(" Source Port         : NA\n");
        }
    }
    else
    {
        srcPortIndex = appDemoIpfixUdbCfg->keyTableUdbCfg[keyTableIndex].otherTypeKey->srcPortIndex - startOfKeyData;
        cpssOsPrintf(" Source Port     : %d\n", *(keyData+srcPortIndex));
    }

    if(debug_print_enable)
    {
        cpssOsPrintf(" Applicable Flow Sub-template UDB Index[%d] = %d\n", i,
                appDemoIpfixUdbCfg->keyTableUdbCfg[keyTableIndex].otherTypeKey->appFlowSubTempIndex);
    }

    if( appDemoIpfixUdbCfg->keyTableUdbCfg[keyTableIndex].otherTypeKey->appFlowSubTempIndex == APP_DEMO_IPFIX_NOT_USED_UDB)
    {
        if(debug_print_enable)
        {
            cpssOsPrintf(" Applicable Flow Sub-template : NA\n");
        }
    }
    else
    {
        appTempIndex = appDemoIpfixUdbCfg->keyTableUdbCfg[keyTableIndex].otherTypeKey->appFlowSubTempIndex - startOfKeyData;
        cpssOsPrintf(" Applicable Flow Sub-template : %d \n", *(keyData+appTempIndex));
    }

    if(debug_print_enable)
    {
        for(i = 0; i< 6; i++)
        {
            cpssOsPrintf(" SMAC UDB Index[%d] = %d\n", i,
                          appDemoIpfixUdbCfg->keyTableUdbCfg[keyTableIndex].otherTypeKey->smacIndex[i]);
        }
    }

    cpssOsPrintf(" SMAC            : ");
    for(i = 0; i < 6; i++)
    {
        if( appDemoIpfixUdbCfg->keyTableUdbCfg[keyTableIndex].otherTypeKey->smacIndex[i] == APP_DEMO_IPFIX_NOT_USED_UDB)
        {
            if(debug_print_enable)
            {
                cpssOsPrintf("NA");
                cpssOsPrintf("%s",(i < 5)? ":":"\n");
            }
        }
        else
        {
            smacIndex = appDemoIpfixUdbCfg->keyTableUdbCfg[keyTableIndex].otherTypeKey->smacIndex[i] - startOfKeyData;
            cpssOsPrintf("%02x", *(keyData+smacIndex));
            cpssOsPrintf("%s",(i < 5)? ":":"\n");
        }
    }

    if(debug_print_enable)
    {
        for(i = 0; i< 6; i++)
        {
            cpssOsPrintf(" DMAC UDB Index[%d] = %d\n", i,
                          appDemoIpfixUdbCfg->keyTableUdbCfg[keyTableIndex].otherTypeKey->dmacIndex[i]);
        }
    }

    cpssOsPrintf(" DMAC            : ");
    for(i = 0; i < 6; i++)
    {
        if( appDemoIpfixUdbCfg->keyTableUdbCfg[keyTableIndex].otherTypeKey->dmacIndex[i] == APP_DEMO_IPFIX_NOT_USED_UDB)
        {
            if(debug_print_enable)
            {
                cpssOsPrintf("NA");
                cpssOsPrintf("%s",(i < 5)? ":":"\n");
            }
        }
        else
        {
            dmacIndex = appDemoIpfixUdbCfg->keyTableUdbCfg[keyTableIndex].otherTypeKey->dmacIndex[i] - startOfKeyData;
            cpssOsPrintf("%02x", *(keyData+dmacIndex));
            cpssOsPrintf("%s",(i < 5)? ":":"\n");
        }
    }

    cpssOsPrintf("---------------- End of OTHER flow ---------------------------\n");

    return rc;
}

GT_STATUS appDemoIpfixFlowInfoDump
(
    IN  GT_U32                              flowMngId,
    IN  GT_U32                              startOfFlowId,
    IN  GT_U32                              numOfFlows
)
{
    CPSS_DXCH_FLOW_MANAGER_FLOW_STC       flowData;
    CPSS_DXCH_FLOW_MANAGER_FLOW_ENTRY_STC flowEntry;

    GT_STATUS                   rc = GT_OK;
    GT_U32                      i = 0;
    GT_U32                      keyTableIndex = 0;
    GT_U32                      count = 0;
    GT_BOOL                     isIpv4TcpKeyIdFound = GT_FALSE;
    GT_BOOL                     isIpv4UdpKeyIdFound = GT_FALSE;
    GT_BOOL                     isIpv6TcpKeyIdFound = GT_FALSE;
    GT_BOOL                     isIpv6UdpKeyIdFound = GT_FALSE;
    GT_BOOL                     isOtherKeyIdFound = GT_FALSE;
    GT_BOOL                     startingFlowFound = GT_FALSE;
    GT_PORT_NUM                 portNum = 0;
    APP_DEMO_IPFIX_NODE_STC    *temp1 = appDemoIpfixHeadToFlowsSLL;
    GT_U32                      appDemoIpfixDevNum = prvWrAppFirstDevNum(0);

    cpssOsBzero((GT_CHAR*)&flowData, sizeof(flowData));
    cpssOsBzero((GT_CHAR*)&flowEntry, sizeof(flowEntry));

    if(debug_print_enable == GT_TRUE)
    {
        cpssOsPrintf("[%s:%d] appDemoIpfixFlowInfoDump entered\n",__FUNCNAME__, __LINE__);
    }

    if((appDemoIpfixHeadToFlowsSLL == NULL) || (appDemoIpfixHeadToFlowsSLL == NULL))
    {
        rc = GT_OK;
        cpssOsPrintf("No flows to print.\n");
        return rc;
    }

    if(appDemoIpfixTotalFlowsCount < numOfFlows)
    {
        rc = GT_BAD_PARAM;
        cpssOsPrintf("Invalid input parameter numOfFlows[%d].\n"
                     "Expected param should be less than or equal to %d (totalflowsCount) rc[%d]\n",
                      numOfFlows, appDemoIpfixTotalFlowsCount, rc);
        return rc;
    }

    if(debug_print_enable)
    {
        for(portNum = 0; portNum < APP_DEMO_IPFIX_MAX_PORT_NUM; portNum++)
        {
            for(i = 0; i < 5; i++)
            {
                cpssOsPrintf("[%s:%d] devNum[%d] portNum[%d] keyIndexArr[%d] = %d\n",__FUNCNAME__, __LINE__, appDemoIpfixDevNum,
                        portNum,i,appDemoIpfixUdbCfg->keyIndexMapCfg[appDemoIpfixDevNum][portNum].keyIndexArr[i]);
            }
        }
    }

    while(temp1 != NULL)
    {
        if(temp1->flowId == startOfFlowId)
        {
            startingFlowFound = GT_TRUE;
            break;
        }
        temp1 = temp1->next;
    }

    if((temp1 == NULL) || (startingFlowFound == GT_FALSE))
    {
        rc = GT_BAD_PARAM;
        cpssOsPrintf("Invalid input parameter flowId[%d].\n"
                     "FlowId[%d] not found\n rc[%d]",startOfFlowId, startOfFlowId, rc);
        return rc;
    }

    isIpv4TcpKeyIdFound = GT_FALSE;
    isIpv4UdpKeyIdFound = GT_FALSE;
    isIpv6TcpKeyIdFound = GT_FALSE;
    isIpv6UdpKeyIdFound = GT_FALSE;
    isOtherKeyIdFound   = GT_FALSE;

    cpssOsPrintf("============= Flows details with startOfFlowId[%d] numOfFlows[%d] ===========\n", startOfFlowId, numOfFlows);

    while((temp1 != NULL) && (count != numOfFlows))
    {
        rc = cpssDxChFlowManagerFlowDataGet(flowMngId, temp1->flowId, &flowEntry, &flowData);
        if(rc != GT_OK)
        {
            cpssOsPrintf("[%s:%d] cpssDxChFlowManagerFlowDataGet FAILED: flowMngId[%d] flowId[%d]\n",
                    __FUNCNAME__, __LINE__, flowMngId, temp1->flowId);
            return rc;
        }

        if(debug_print_enable)
        {
            cpssOsPrintf("[%s:%d] FlowId[%d] keyId[%d]\n", __FUNCNAME__, __LINE__, temp1->flowId, flowData.keyId);
        }

        for(portNum = 0; portNum < APP_DEMO_IPFIX_MAX_PORT_NUM; portNum++)
        {
            if(flowData.keyId == appDemoIpfixUdbCfg->keyIndexMapCfg[appDemoIpfixDevNum][portNum].keyIndexArr[0])
            {
                isIpv4TcpKeyIdFound = GT_TRUE;
                keyTableIndex = appDemoIpfixUdbCfg->keyIndexMapCfg[appDemoIpfixDevNum][portNum].keyIndexArr[0];
                rc = prvFlowManagerIpv4FlowInfoDump(flowMngId, keyTableIndex, temp1->flowId, flowData.keyData);
                if(rc != GT_OK)
                {
                    cpssOsPrintf("[%s:%d] prvFlowManagerIpv4TcpFlowInfoDump FAILED: flowMngId[%d]"
                                 "keyTableIndex[%d] flowId[%d]\n",
                                 __FUNCNAME__, __LINE__, flowMngId, keyTableIndex, temp1->flowId);
                    return rc;
                }
                count++;
                break;
            }
            if( flowData.keyId == appDemoIpfixUdbCfg->keyIndexMapCfg[appDemoIpfixDevNum][portNum].keyIndexArr[1])
            {
                isIpv4UdpKeyIdFound = GT_TRUE;
                keyTableIndex = appDemoIpfixUdbCfg->keyIndexMapCfg[appDemoIpfixDevNum][portNum].keyIndexArr[1];
                rc = prvFlowManagerIpv4FlowInfoDump(flowMngId, keyTableIndex, temp1->flowId, flowData.keyData);
                if(rc != GT_OK)
                {
                    cpssOsPrintf("[%s:%d] prvFlowManagerIpv4FlowInfoDump FAILED: flowMngId[%d]"
                                 "keyTableIndex[%d] flowId[%d]\n",
                                 __FUNCNAME__, __LINE__, flowMngId, keyTableIndex, temp1->flowId);
                    return rc;
                }
                count++;
                break;
            }
            if( flowData.keyId == appDemoIpfixUdbCfg->keyIndexMapCfg[appDemoIpfixDevNum][portNum].keyIndexArr[2])
            {
                isIpv6TcpKeyIdFound = GT_TRUE;
                keyTableIndex = appDemoIpfixUdbCfg->keyIndexMapCfg[appDemoIpfixDevNum][portNum].keyIndexArr[2];
                rc = prvFlowManagerIpv6FlowInfoDump(flowMngId, keyTableIndex, temp1->flowId, flowData.keyData);
                if(rc != GT_OK)
                {
                    cpssOsPrintf("[%s:%d] prvFlowManagerIpv6TcpFlowInfoDump FAILED: flowMngId[%d]"
                                 "keyTableIndex[%d] flowId[%d]\n",
                                 __FUNCNAME__, __LINE__, flowMngId, keyTableIndex, temp1->flowId);
                    return rc;
                }
                count++;
                break;
            }
            if( flowData.keyId == appDemoIpfixUdbCfg->keyIndexMapCfg[appDemoIpfixDevNum][portNum].keyIndexArr[3])
            {
                isIpv6UdpKeyIdFound = GT_TRUE;
                keyTableIndex = appDemoIpfixUdbCfg->keyIndexMapCfg[appDemoIpfixDevNum][portNum].keyIndexArr[3];
                rc = prvFlowManagerIpv6FlowInfoDump(flowMngId, keyTableIndex, temp1->flowId, flowData.keyData);
                if(rc != GT_OK)
                {
                    cpssOsPrintf("[%s:%d] prvFlowManagerIpv6UdpFlowInfoDump FAILED: flowMngId[%d]"
                                 "keyTableIndex[%d] flowId[%d]\n",
                                 __FUNCNAME__, __LINE__, flowMngId, keyTableIndex, temp1->flowId);
                    return rc;
                }
                count++;
                break;
            }
            if( flowData.keyId == appDemoIpfixUdbCfg->keyIndexMapCfg[appDemoIpfixDevNum][portNum].keyIndexArr[4])
            {
                isOtherKeyIdFound = GT_TRUE;
                keyTableIndex = appDemoIpfixUdbCfg->keyIndexMapCfg[appDemoIpfixDevNum][portNum].keyIndexArr[4];
                rc = prvFlowManagerOtherFlowInfoDump(flowMngId, keyTableIndex, temp1->flowId, flowData.keyData);
                if(rc != GT_OK)
                {
                    cpssOsPrintf("[%s:%d] prvFlowManagerOtherFlowInfoDump FAILED: flowMngId[%d]"
                                 "keyTableIndex[%d] flowId[%d]\n",
                                 __FUNCNAME__, __LINE__, flowMngId, keyTableIndex, temp1->flowId);
                    return rc;
                }
                count++;
                break;
            }
        }

        temp1 = temp1->next;

    }

    if(debug_print_enable)
    {
        if(isIpv4TcpKeyIdFound == GT_FALSE)
        {
            cpssOsPrintf("[%s:%d] Ipv4-TCP KeyId not configured for any ports\n",__FUNCNAME__, __LINE__);
        }
        if(isIpv4UdpKeyIdFound == GT_FALSE)
        {
            cpssOsPrintf("[%s:%d] Ipv4-UDP KeyId not configured for any ports\n",__FUNCNAME__, __LINE__);
        }
        if(isIpv6TcpKeyIdFound == GT_FALSE)
        {
            cpssOsPrintf("[%s:%d] Ipv6-TCP KeyId not configured for any ports\n",__FUNCNAME__, __LINE__);
        }
        if(isIpv6UdpKeyIdFound == GT_FALSE)
        {
            cpssOsPrintf("[%s:%d] Ipv6-UDP KeyId not configured for any ports\n",__FUNCNAME__, __LINE__);
        }
        if(isOtherKeyIdFound == GT_FALSE)
        {
            cpssOsPrintf("[%s:%d] OTHER KeyId not configured for any ports\n",__FUNCNAME__, __LINE__);
        }
    }

    if( count != numOfFlows )
    {
        rc = GT_BAD_PARAM;
        cpssOsPrintf("Invalid numOfFlows[%d] as DB contains only Flows[%d] after startOfFlowId[%d] rc[%d]\n",
                     numOfFlows, count, startOfFlowId, rc);
        return rc;
    }

    cpssOsPrintf("============ End of dump ================\n");

    if(debug_print_enable == GT_TRUE)
    {
        cpssOsPrintf("[%s:%d] appDemoIpfixFlowInfoDump successfully exiting\n", __FUNCNAME__, __LINE__);
    }

    return rc;
}

static GT_STATUS prvFlowManagerLocalDbUpdate
(
    IN  GT_U32                              flowMngId,
    IN  CPSS_DXCH_FLOW_MANAGER_FLOW_STC     *newFlow
)
{
    GT_STATUS rc = GT_OK;
    APP_DEMO_IPFIX_NODE_STC *newFlowNode = NULL;

    if(debug_print_enable == GT_TRUE)
    {
        cpssOsPrintf("[%s:%d] prvFlowManagerLocalDbUpdate entered\n",__FUNCNAME__, __LINE__);
    }

    if(appDemoIpfixHeadToFlowsSLL == NULL){
        if((appDemoIpfixTotalFlowsCount != 0)|| ( appDemoIpfixTailToFlowsSLL != NULL )){
            cpssOsPrintf("[%s:%d] FAILED to retrieve the local DB, rc [%d] flowMngId [%d]\n",
                         __FUNCNAME__, __LINE__, rc, flowMngId );
            return GT_BAD_STATE;
        }

        if(debug_print_enable == GT_TRUE)
        {
            cpssOsPrintf("[%s:%d] First flow received\n",__FUNCNAME__, __LINE__);
        }

        newFlowNode = (APP_DEMO_IPFIX_NODE_STC*)cpssOsMalloc(sizeof(APP_DEMO_IPFIX_NODE_STC));
        if(NULL == newFlowNode)
        {
            cpssOsPrintf("[%s:%d] FAILED to allocate memory, rc [%d] flowMngId [%d]\n",
                         __FUNCNAME__, __LINE__, rc, flowMngId );
            return GT_BAD_STATE;
        }
        newFlowNode->next = NULL;
        newFlowNode->flowId = newFlow->flowId;

        appDemoIpfixTailToFlowsSLL = newFlowNode;
        appDemoIpfixHeadToFlowsSLL = newFlowNode;

        appDemoIpfixTotalFlowsCount++;
    }
    else
    { /* Not the first flow */

        if((appDemoIpfixTotalFlowsCount == 0) || (appDemoIpfixTailToFlowsSLL == NULL))
        {
            cpssOsPrintf("[%s:%d] FAILED to retrieve the local DB, rc [%d] flowMngId [%d]\n",
                         __FUNCNAME__, __LINE__, rc, flowMngId );
            return GT_BAD_STATE;
        }

        newFlowNode = (APP_DEMO_IPFIX_NODE_STC*)cpssOsMalloc(sizeof(APP_DEMO_IPFIX_NODE_STC));
        if(NULL == newFlowNode)
        {
            cpssOsPrintf("[%s:%d] FAILED to allocate memory, rc [%d] flowMngId [%d]\n",
                         __FUNCNAME__, __LINE__, rc, flowMngId );
            return GT_BAD_STATE;
        }
        newFlowNode->next = NULL;
        newFlowNode->flowId = newFlow->flowId;

        appDemoIpfixTailToFlowsSLL->next = newFlowNode;
        appDemoIpfixTailToFlowsSLL = newFlowNode;

        appDemoIpfixTotalFlowsCount++;
    }

    if(debug_print_enable == GT_TRUE)
    {
        cpssOsPrintf("[%s:%d] prvFlowManagerLocalDbUpdate successfully exiting\n",
                     __FUNCNAME__, __LINE__);
    }

    return rc;
}

static GT_STATUS prvFlowManagerLocalDbRemove
(
    IN  GT_U32                              flowMngId,
    IN  GT_U32                              removedFlowId
)
{
    GT_STATUS rc = GT_OK;
    APP_DEMO_IPFIX_NODE_STC *temp = NULL;
    APP_DEMO_IPFIX_NODE_STC *temp1 = NULL;

    if(debug_print_enable == GT_TRUE)
    {
        cpssOsPrintf("[%s:%d] prvFlowManagerLocalDbRemove entered\n",__FUNCNAME__, __LINE__);
    }

    if((appDemoIpfixTotalFlowsCount == 0) || (appDemoIpfixTailToFlowsSLL == NULL))
    {
        cpssOsPrintf("[%s:%d] FAILED to retrieve the local DB, rc [%d] flowMngId [%d]\n",
            __FUNCNAME__, __LINE__, rc, flowMngId );
        return GT_BAD_STATE;
    }

    temp = appDemoIpfixHeadToFlowsSLL;
    temp1 = appDemoIpfixHeadToFlowsSLL;

    if(temp->flowId == removedFlowId)
    {
        appDemoIpfixHeadToFlowsSLL = temp->next;
        temp->next = NULL;
        appDemoIpfixTotalFlowsCount--;
        cpssOsFree(temp);
    }
    else
    {
        while(temp != NULL)
        {
            if(temp->next->flowId == removedFlowId && temp->next->next != NULL)
            {
                temp1 = temp->next;
                temp = temp->next->next;
                temp1->next = NULL;
                appDemoIpfixTotalFlowsCount--;
                cpssOsFree(temp1);
            }
            else if(temp->next->flowId == removedFlowId && temp->next->next == NULL)
            {
                temp1 = temp->next;
                appDemoIpfixTailToFlowsSLL = temp;
                temp1->next = NULL;
                appDemoIpfixTotalFlowsCount--;
                cpssOsFree(temp1);
            }
            temp = temp->next;
        }
    }

    if(debug_print_enable == GT_TRUE)
    {
        cpssOsPrintf("[%s:%d] prvFlowManagerLocalDbRemove successfully exiting\n",
                     __FUNCNAME__, __LINE__);
    }

    return rc;
}

static GT_STATUS prvFlowManagerGetNewFlowInfo
(
    IN  GT_U8                               devNum,
    IN  GT_U32                              flowMngId,
    OUT CPSS_DXCH_FLOW_MANAGER_FLOW_STC     *newFlow
)
{
    GT_STATUS rc = GT_OK;

    GT_UNUSED_PARAM(devNum);

    if(debug_print_enable == GT_TRUE)
    {
        cpssOsPrintf("[%s:%d] prvFlowManagerGetNewFlowInfo entered\n",__FUNCNAME__, __LINE__);
    }

    if(debug_print_enable == GT_TRUE)
    {
        cpssOsPrintf("[%s:%d] prvFlowManagerGetNewFlowInfo received new flow with flow id %d keyId %d keySize %d\n",
                __FUNCNAME__, __LINE__, newFlow->flowId, newFlow->keyId, newFlow->keySize);
        cpssOsPrintf("[%s:%d] Key Data received at application.",__FUNCNAME__, __LINE__);
    }

    if(appDemoEnableDb)
    {
        rc = prvFlowManagerLocalDbUpdate(flowMngId, newFlow);
        if (GT_OK != rc)
        {
            cpssOsPrintf("[%s:%d] prvFlowManagerLocalDbUpdate FAILED, rc [%d] flowMngId [%d]\n",
                __FUNCNAME__, __LINE__, rc, flowMngId );
            return rc;
        }
    }

    if(debug_print_enable == GT_TRUE)
    {
        cpssOsPrintf("[%s:%d] prvFlowManagerGetNewFlowInfo successfully exiting\n",__FUNCNAME__, __LINE__);
    }

    return rc;
}

static GT_STATUS prvFlowManagerRemoveFlow
(
    IN  GT_U32                              flowMngId,
    OUT GT_U32                              removedFlowIndex
)
{
    GT_STATUS rc = GT_OK;

    if(debug_print_enable == GT_TRUE)
    {
        cpssOsPrintf("[%s:%d] prvFlowManagerRemoveFlow entered\n",__FUNCNAME__, __LINE__);
        cpssOsPrintf("[%s:%d] prvFlowManagerRemoveFlow received flow id %d\n",
                __FUNCNAME__, __LINE__, removedFlowIndex);
    }

    if(appDemoEnableDb)
    {
        rc = prvFlowManagerLocalDbRemove(flowMngId, removedFlowIndex);
        if (GT_OK != rc)
        {
            cpssOsPrintf("[%s:%d] prvFlowManagerLocalDbUpdate FAILED, rc [%d] flowMngId [%d]\n",
                __FUNCNAME__, __LINE__, rc, flowMngId );
            return rc;
        }
    }

    if(debug_print_enable == GT_TRUE)
    {
        cpssOsPrintf("[%s:%d] prvFlowManagerRemoveFlow successfully exiting\n",__FUNCNAME__, __LINE__);
    }

    return rc;
}

/* Enabling/disabling flow manager configurations */
GT_STATUS prvFlowManagerConfig
(
    IN GT_U32                              flowMngId,
    IN GT_U8                               devNum,
    IN GT_BOOL                             enable
)
{
    GT_STATUS                              rc = GT_OK;
    CPSS_DXCH_FLOW_MANAGER_STC             fmStc;
    CPSS_DXCH_NET_CPU_CODE_TABLE_ENTRY_STC cpuCodeEntry;

    cpssOsBzero((GT_CHAR*)&fmStc, sizeof(fmStc));
    cpssOsBzero((GT_CHAR*)&cpuCodeEntry, sizeof(cpuCodeEntry));

    if(debug_print_enable == GT_TRUE)
    {
        cpssOsPrintf("[%s:%d] prvFlowManagerConfig entered\n",__FUNCNAME__, __LINE__);
    }

    if(enable == GT_TRUE)
    {
        /* Sample Flow manager configurations */
        fmStc.flowDbSize                     = 65536;
        fmStc.cpssFwCfg.rxSdmaQ              = appDemoIpfixFwRxQueue;
        fmStc.cpssFwCfg.txSdmaQ              = appDemoIpfixFwTxQueue;
        fmStc.cpssFwCfg.messagePacketCpuCode = appDemoIpfixMsgCpuCode;
        fmStc.cpssFwCfg.cpuCodeStart         = CPSS_NET_FIRST_USER_DEFINED_E+4;
        fmStc.cpssFwCfg.tcamStartIndex       = 0;
        fmStc.cpssFwCfg.ipfixStartIndex      = 0;
        if(appDemoIpfixEnableQinQ == GT_FALSE)
        {
            fmStc.cpssFwCfg.reducedRuleSize      = 16;
        }
        else
        {
            fmStc.cpssFwCfg.reducedRuleSize      = 18;
        }
        fmStc.cpssFwCfg.tcamNumOfShortRules  = appDemoIpfixFwTcamNumOfShortRules;
        fmStc.cpssFwCfg.tcamShortRuleSize    = appDemoIpfixShortKeySize;
        fmStc.cpssFwCfg.flowDBSize           = 4096;
        fmStc.cpssFwCfg.flowShortDBSize      = 4096;
        fmStc.appCbFlowNotificaion           = prvFlowManagerGetNewFlowInfo;
        fmStc.appCbOldFlowRemovalNotificaion = prvFlowManagerRemoveFlow;

        rc = cpssDxChFlowManagerCreate(flowMngId,&fmStc);
        if (GT_OK != rc)
        {
            cpssOsPrintf("[%s:%d] cpssDxChFlowManagerCreate FAILED, rc [%d] flowMngId [%d]\n",
                         __FUNCNAME__, __LINE__, rc, flowMngId );
            return rc;
        }

        rc = cpssDxChFlowManagerEnableSet(flowMngId,GT_TRUE);
        if (GT_OK != rc)
        {
            cpssOsPrintf("[%s:%d] cpssDxChFlowManagerEnableSet FAILED, rc [%d] flowMngId [%d]\n",
                         __FUNCNAME__, __LINE__, rc, flowMngId );
            return rc;
        }

        rc = cpssDxChFlowManagerDevListAdd(flowMngId, &devNum, 1);
        if (GT_OK != rc)
        {
            cpssOsPrintf("[%s:%d] cpssDxChFlowManagerDevListAdd FAILED, rc [%d] flowMngId [%d]\n",
                         __FUNCNAME__, __LINE__, rc, flowMngId );
            return rc;
        }

        /* Trancated packets to CPU */
        rc = cpssDxChNetIfCpuCodeTableGet(devNum,
                                          appDemoIpfixDefaultRuleCpuCode,
                                          &cpuCodeEntry);
        if (GT_OK != rc)
        {
            cpssOsPrintf("[%s:%d] cpssDxChNetIfCpuCodeTableGet FAILED, rc [%d] devNum [%d]\n",
                         __FUNCNAME__, __LINE__, rc, devNum);
            return rc;
        }

        cpuCodeEntry.truncate = GT_TRUE;
        cpuCodeEntry.tc = appDemoIpfixFwTxQueue;

        rc = cpssDxChNetIfCpuCodeTableSet(devNum,
                                          appDemoIpfixDefaultRuleCpuCode,
                                          &cpuCodeEntry);
        if (GT_OK != rc)
        {
            cpssOsPrintf("[%s:%d] cpssDxChNetIfCpuCodeTableSet FAILED, rc [%d] devNum [%d]\n",
                         __FUNCNAME__, __LINE__, rc, devNum);
            return rc;
        }

        /* Assigning queue num for cpucode received on msg from Fw */
        rc = cpssDxChNetIfCpuCodeTableGet(devNum,
                                          appDemoIpfixMsgCpuCode,
                                          &cpuCodeEntry);
        if (GT_OK != rc)
        {
            cpssOsPrintf("[%s:%d] cpssDxChNetIfCpuCodeTableGet FAILED, rc [%d] devNum [%d]\n",
                         __FUNCNAME__, __LINE__, rc, devNum);
            return rc;
        }

        cpuCodeEntry.tc = appDemoIpfixMsgRxQueue;

        rc = cpssDxChNetIfCpuCodeTableSet(devNum,
                                          appDemoIpfixMsgCpuCode,
                                          &cpuCodeEntry);
        if (GT_OK != rc)
        {
            cpssOsPrintf("[%s:%d] cpssDxChNetIfCpuCodeTableSet FAILED, rc [%d] devNum [%d]\n",
                         __FUNCNAME__, __LINE__, rc, devNum);
            return rc;
        }

        /* Bind CPU Port To Scheduler Profile and associate Message packet Queue with SP arbitration
         * group. Message packets are more important than packets for new flow learning and hence
         * assigning message packet queue to SP group for highest priority treatment.
         */
        rc = cpssDxChPortTxBindPortToSchedulerProfileGet(devNum, CPSS_CPU_PORT_NUM_CNS, &appDemoIpfixProfileGet);
        if (GT_OK != rc)
        {
            cpssOsPrintf("[%s:%d] cpssDxChPortTxBindPortToSchedulerProfileGet FAILED, rc [%d] devNum [%d] portNum %d\n",
                         __FUNCNAME__, __LINE__, rc, devNum, CPSS_CPU_PORT_NUM_CNS);
            return rc;
        }
        rc = cpssDxChPortTxBindPortToSchedulerProfileSet(devNum, CPSS_CPU_PORT_NUM_CNS, APP_DEMO_IPFIX_FLOW_MANAGER_CPU_PORT_PROFILE_CNS);
        if (GT_OK != rc)
        {
            cpssOsPrintf("[%s:%d] cpssDxChPortTxBindPortToSchedulerProfileSet FAILED, rc [%d] devNum [%d] portNum %d profile %d\n",
                         __FUNCNAME__, __LINE__, rc, devNum, CPSS_CPU_PORT_NUM_CNS, APP_DEMO_IPFIX_FLOW_MANAGER_CPU_PORT_PROFILE_CNS);
            return rc;
        }

        rc = cpssDxChPortTxQArbGroupGet(devNum, appDemoIpfixMsgRxQueue, appDemoIpfixProfileGet, &appDemoIpfixArbGroupGet);
        if (GT_OK != rc)
        {
            cpssOsPrintf("[%s:%d] cpssDxChPortTxQArbGroupGet FAILED, rc [%d] devNum [%d] queue %d  profile %d\n",
                         __FUNCNAME__, __LINE__, rc, devNum, appDemoIpfixMsgRxQueue, appDemoIpfixProfileGet);
            return rc;
        }
        rc = cpssDxChPortTxQArbGroupSet(devNum, appDemoIpfixMsgRxQueue, APP_DEMO_IPFIX_FLOW_MANAGER_CPU_PORT_MSG_PACKET_QUEUE_ARB_GROUP,
                                        APP_DEMO_IPFIX_FLOW_MANAGER_CPU_PORT_PROFILE_CNS);
        if (GT_OK != rc)
        {
            cpssOsPrintf("[%s:%d] cpssDxChPortTxQArbGroupSet FAILED, rc [%d] devNum [%d] queue %d  arb group %d profile %d\n",
                         __FUNCNAME__, __LINE__, rc, devNum, appDemoIpfixMsgRxQueue, APP_DEMO_IPFIX_FLOW_MANAGER_CPU_PORT_MSG_PACKET_QUEUE_ARB_GROUP,
                         APP_DEMO_IPFIX_FLOW_MANAGER_CPU_PORT_PROFILE_CNS);
            return rc;
        }

        /* Disabling the below events generated by trapped packets to FW as Queue#6 is meant for
         * message packet and is not meant for application usage.
         */
        rc = cpssEventDeviceMaskSet(devNum, CPSS_PP_RX_BUFFER_QUEUE6_E, CPSS_EVENT_MASK_E);
        if (GT_OK != rc)
        {
            cpssOsPrintf("[%s:%d] cpssEventDeviceMaskSet FAILED, rc [%d] devNum [%d] uniEv %d\n",
                         __FUNCNAME__, __LINE__, rc, devNum, CPSS_PP_RX_BUFFER_QUEUE6_E);
            return rc;
        }

        rc = cpssEventDeviceMaskSet(devNum, CPSS_PP_RX_ERR_QUEUE6_E, CPSS_EVENT_MASK_E);
        if (GT_OK != rc)
        {
            cpssOsPrintf("[%s:%d] cpssEventDeviceMaskSet FAILED, rc [%d] devNum [%d] uniEv %d\n",
                         __FUNCNAME__, __LINE__, rc, devNum, CPSS_PP_RX_ERR_QUEUE6_E);
            return rc;
        }
     }
     else
     {

        rc = cpssDxChFlowManagerDevListRemove(flowMngId, &devNum, 1);
        if (GT_OK != rc)
        {
            cpssOsPrintf("[%s:%d] cpssDxChFlowManagerDevListAdd FAILED, rc [%d] flowMngId [%d]\n",
                         __FUNCNAME__, __LINE__, rc, flowMngId );
            return rc;
        }

        rc = cpssDxChFlowManagerEnableSet(flowMngId, enable);
        if (GT_OK != rc)
        {
            cpssOsPrintf("[%s:%d] cpssDxChFlowManagerEnableSet FAILED, rc [%d] flowMngId [%d] enable[%d]\n",
                         __FUNCNAME__, __LINE__, rc, flowMngId, enable );
            return rc;
        }

        rc = cpssDxChFlowManagerDelete(flowMngId);
        if (GT_OK != rc)
        {
            cpssOsPrintf("[%s:%d] cpssDxChFlowManagerDelete FAILED, rc [%d] flowMngId [%d]\n",
                         __FUNCNAME__, __LINE__, rc, flowMngId );
            return rc;
        }

        /* Trancated packets to CPU */
        rc = cpssDxChNetIfCpuCodeTableGet(devNum,
                                          appDemoIpfixDefaultRuleCpuCode,
                                          &cpuCodeEntry);
        if (GT_OK != rc)
        {
            cpssOsPrintf("[%s:%d] cpssDxChNetIfCpuCodeTableGet FAILED, rc [%d] devNum [%d]\n",
                         __FUNCNAME__, __LINE__, rc, devNum);
            return rc;
        }

        cpuCodeEntry.truncate = GT_FALSE;
        cpuCodeEntry.tc = 0;

        rc = cpssDxChNetIfCpuCodeTableSet(devNum,
                                          appDemoIpfixDefaultRuleCpuCode,
                                          &cpuCodeEntry);
        if (GT_OK != rc)
        {
            cpssOsPrintf("[%s:%d] cpssDxChNetIfCpuCodeTableSet FAILED, rc [%d] devNum [%d]\n",
                         __FUNCNAME__, __LINE__, rc, devNum);
            return rc;
        }

        /* Assigning queue num for cpucode received on msg from Fw */
        rc = cpssDxChNetIfCpuCodeTableGet(devNum,
                                          appDemoIpfixMsgCpuCode,
                                          &cpuCodeEntry);
        if (GT_OK != rc)
        {
            cpssOsPrintf("[%s:%d] cpssDxChNetIfCpuCodeTableGet FAILED, rc [%d] devNum [%d]\n",
                         __FUNCNAME__, __LINE__, rc, devNum);
            return rc;
        }

        cpuCodeEntry.tc = 0;

        rc = cpssDxChNetIfCpuCodeTableSet(devNum,
                                          appDemoIpfixMsgCpuCode,
                                          &cpuCodeEntry);
        if (GT_OK != rc)
        {
            cpssOsPrintf("[%s:%d] cpssDxChNetIfCpuCodeTableSet FAILED, rc [%d] devNum [%d]\n",
                         __FUNCNAME__, __LINE__, rc, devNum);
            return rc;
        }
    }

    if(debug_print_enable == GT_TRUE)
    {
        cpssOsPrintf("[%s:%d] prvFlowManagerConfig successfully exiting\n",__FUNCNAME__, __LINE__);
    }
    return rc;

}

GT_STATUS appDemoIpfixDebugDumpEnable
(
    IN GT_BOOL  enableDump
)
{
    GT_STATUS rc = GT_OK;

    debug_print_enable = enableDump;

    if(debug_print_enable == GT_TRUE)
    {
        cpssOsPrintf("[%s:%d] appDemoIpfixDebugDumpEnable success enableDump[%d]\n",
                     __FUNCNAME__, __LINE__, enableDump);
    }

    return rc;
}

GT_STATUS appDemoIpfixInit
(
    IN GT_U32                              flowMngId
)
{
    RX_PACKET_RECEIVE_CB_FUN rxPktReceiveCbFun;

    GT_STATUS                rc = GT_OK;
    GT_BOOL                  enable = GT_TRUE;
    GT_U32                   appDemoIpfixDevNum = prvWrAppFirstDevNum(0);

    if(debug_print_enable == GT_TRUE)
    {
        cpssOsPrintf("[%s:%d] appDemoIpfixInit entered\n",__FUNCNAME__, __LINE__);
    }

    /* Validating the flow manager id upfront to ensure the other module level init does not
     * happen for not allowed values of flow manager id thus saving the unnecessary init operations.
     */
    if (flowMngId > APP_DEMO_IPFIX_FLOW_MANAGER_MAX_ID_CNS)
    {
        rc = GT_FAIL;
        if(debug_print_enable == GT_TRUE)
        {
            cpssOsPrintf("[%s:%d] Flow manager ID not in allowed range rc[%d]\n",
                __FUNCNAME__, __LINE__, rc);
        }
        return rc;
    }

    /* TCAM Init */
    rc = prvFlowManagerVirtualTcamInit(appDemoIpfixDevNum, enable);
    if (GT_OK != rc)
    {
        cpssOsPrintf("[%s:%d] prvFlowManagerVirtualTcamInit FAILED, rc [%d] devNum[%d] enable[%d]\n",
                     __FUNCNAME__, __LINE__, rc, appDemoIpfixDevNum, enable);
        return rc;
    }

    /* PCL Init */
    rc = prvFlowManagerPclInit(appDemoIpfixDevNum,
                               appDemoIpfixLookupNum, enable);
    if (GT_OK != rc)
    {
        cpssOsPrintf("[%s:%d] prvFlowManagerPclInit FAILED, rc [%d] devNum[%d] enable[%d]\n",
                     __FUNCNAME__, __LINE__, rc, appDemoIpfixDevNum, enable);
        return rc;
    }

    /* Policer Init */
    rc = prvFlowManagerPolicerInit(appDemoIpfixDevNum,
                                   appDemoIpfixPolicerStage, enable);
    if (GT_OK != rc)
    {
        cpssOsPrintf("[%s:%d] prvFlowManagerPolicerInit FAILED, rc [%d] devNum[%d] enable[%d]\n",
                     __FUNCNAME__, __LINE__, rc, appDemoIpfixDevNum, enable);
        return rc;
    }

    /* Ipfix Wrap-around configuration */
    rc = prvFlowManagerIpfixWrapAroundConfig(appDemoIpfixDevNum,
                                             enable);
    if (GT_OK != rc)
    {
        cpssOsPrintf("[%s:%d] prvFlowManagerIpfixWrapAroundConfig FAILED, rc [%d] devNum[%d] enable[%d]\n",
                     __FUNCNAME__, __LINE__, rc, appDemoIpfixDevNum, enable);
        return rc;
    }

    /* Install default rules on all ports and for all packet types */
    rc = prvFlowManagerInstallDefaultRules(appDemoIpfixDevNum, enable);
    if (GT_OK != rc)
    {
        cpssOsPrintf("[%s:%d] prvFlowManagerInstallDefaultRules FAILED, rc [%d] devNum[%d] enable[%d]\n",
                     __FUNCNAME__, __LINE__, rc, appDemoIpfixDevNum, enable);
        return rc;
    }

    /* Install ipfix bound Fw rules on all ports */
    rc = prvFlowManagerBindIpfixCountersToFwRules(appDemoIpfixDevNum, enable);
    if (GT_OK != rc)
    {
        cpssOsPrintf("[%s:%d] prvFlowManagerBindIpfixCountersToFwRules FAILED,"
                     " rc [%d] devNum[%d] enable[%d]\n",
                     __FUNCNAME__, __LINE__, rc, appDemoIpfixDevNum, enable);
        return rc;
    }

    /* Initializing and configuring firmware */
    rc = prvFlowManagerConfig(flowMngId, appDemoIpfixDevNum, enable);
    if (GT_OK != rc)
    {
        /* On failure of flow manager config necessary clean up is needed */
        appDemoIpfixDelete(flowMngId);
        cpssOsPrintf("[%s:%d] prvFlowManagerConfig FAILED, rc [%d] flowMngId[%d] devNum[%d] enable[%d]\n",
                     __FUNCNAME__, __LINE__, rc, flowMngId, appDemoIpfixDevNum, enable);
        return rc;
    }

    appDemoIpfixFlowMngId = flowMngId;

    /* Additional error in current SDMA code\package for CM3 - NA messages to CPU reach to unknown destination causing multi interrupts and memory corruption*/
    rc = cpssDxChBrgVlanNaToCpuEnable(appDemoIpfixDevNum,
                                      1, GT_FALSE);
    if (GT_OK != rc)
    {
        cpssOsPrintf("[%s:%d] cpssDxChBrgVlanNaToCpuEnable FAILED, rc [%d] devNum [%d]\n",
                     __FUNCNAME__, __LINE__, rc, appDemoIpfixDevNum);
        return rc;
    }

    /* hooking the listen function to Rx_event */
    rxPktReceiveCbFun = prvFlowManagerDbUpdateByMsgPacketParse;
    rc = prvWrAppDxChNetRxPacketCbRegister(rxPktReceiveCbFun);
    if (GT_OK != rc)
    {
        cpssOsPrintf("[%s:%d] prvWrAppDxChNetRxPacketCbRegister FAILED, rc [%d] devNum [%d]\n",
                     __FUNCNAME__, __LINE__, rc, appDemoIpfixDevNum);
        return rc;
    }

    if(debug_print_enable == GT_TRUE)
    {
        cpssOsPrintf("[%s:%d] appDemoIpfixInit successfully exiting\n",__FUNCNAME__, __LINE__);
    }

    return GT_OK;

}

static GT_STATUS prvFlowManagerLocalDbDelete
(
    IN GT_U32 flowMngId
)
{
    GT_STATUS               rc = GT_OK;
    APP_DEMO_IPFIX_NODE_STC *tempNode = NULL;
    GT_U32                  keyTableIndex = 0;

    if(debug_print_enable == GT_TRUE)
    {
        cpssOsPrintf("[%s:%d] prvFlowManagerLocalDbDelete entered\n",__FUNCNAME__, __LINE__);
    }

    if(appDemoIpfixUdbCfg == NULL)
    {
        if(appDemoIpfixTotalUdbConfigCount == 0)
        {
            rc = GT_OK;
            if(debug_print_enable)
            {
                cpssOsPrintf("[%s:%d] No UDBs configured rc[%d]\n", __FUNCNAME__, __LINE__, rc);
            }
            return rc;
        }
        /* something wrong with DB */
        return GT_FAIL;
    }

    for(keyTableIndex = 0; keyTableIndex < APP_DEMO_IPFIX_KEY_TABLE_INDEX_CNS; keyTableIndex++)
    {
        if(appDemoIpfixUdbCfg->keyTableUdbCfg[keyTableIndex].ipv4Key != NULL)
        {
            cpssOsFree(appDemoIpfixUdbCfg->keyTableUdbCfg[keyTableIndex].ipv4Key);
            appDemoIpfixTotalUdbConfigCount--;
            appDemoIpfixUdbCfg->keyTableUdbCfg[keyTableIndex].ipv4Key = NULL;
            if(debug_print_enable)
            {
                cpssOsPrintf("appDemoIpfixUdbCfg->keyTableUdbCfg[%d].ipv4Key=0x%0x\n",keyTableIndex,
                        appDemoIpfixUdbCfg->keyTableUdbCfg[keyTableIndex].ipv4Key);
            }
        }
        if(appDemoIpfixUdbCfg->keyTableUdbCfg[keyTableIndex].ipv6Key != NULL)
        {
            cpssOsFree(appDemoIpfixUdbCfg->keyTableUdbCfg[keyTableIndex].ipv6Key);
            appDemoIpfixTotalUdbConfigCount--;
            appDemoIpfixUdbCfg->keyTableUdbCfg[keyTableIndex].ipv6Key = NULL;
            if(debug_print_enable)
            {
                cpssOsPrintf("appDemoIpfixUdbCfg->keyTableUdbCfg[%d].ipv6Key=0x%0x\n",keyTableIndex,
                        appDemoIpfixUdbCfg->keyTableUdbCfg[keyTableIndex].ipv6Key);
            }
        }
        if(appDemoIpfixUdbCfg->keyTableUdbCfg[keyTableIndex].otherTypeKey != NULL)
        {
            cpssOsFree(appDemoIpfixUdbCfg->keyTableUdbCfg[keyTableIndex].otherTypeKey);
            appDemoIpfixTotalUdbConfigCount--;
            appDemoIpfixUdbCfg->keyTableUdbCfg[keyTableIndex].otherTypeKey = NULL;
            if(debug_print_enable)
            {
                cpssOsPrintf("appDemoIpfixUdbCfg->keyTableUdbCfg[%d].otherTypeKey=0x%0x\n",keyTableIndex,
                        appDemoIpfixUdbCfg->keyTableUdbCfg[keyTableIndex].otherTypeKey);
            }
        }
    }

    cpssOsFree(appDemoIpfixUdbCfg);
    appDemoIpfixTotalUdbConfigCount = 0;
    appDemoIpfixUdbCfg = NULL;

    if(appDemoIpfixHeadToFlowsSLL == NULL)
    {
        if((appDemoIpfixTotalFlowsCount != 0)|| ( appDemoIpfixTailToFlowsSLL != NULL ))
        {
            cpssOsPrintf("[%s:%d] FAILED to retrieve the local DB, rc [%d] flowMngId [%d]\n",
                         __FUNCNAME__, __LINE__, rc, flowMngId );
            return GT_BAD_STATE;
        }
        appDemoIpfixTailToFlowsSLL = NULL;
        if(debug_print_enable)
        {
            cpssOsPrintf("No flows in the DB\n");
        }
        return GT_OK;
    }

    while(appDemoIpfixHeadToFlowsSLL != NULL)
    {
        tempNode = appDemoIpfixHeadToFlowsSLL;
        appDemoIpfixHeadToFlowsSLL = appDemoIpfixHeadToFlowsSLL->next;
        cpssOsFree(tempNode);
        tempNode = NULL;
        appDemoIpfixTotalFlowsCount--;
    }
    appDemoIpfixTailToFlowsSLL = NULL;
    appDemoIpfixTotalFlowsCount = 0;

    if(debug_print_enable)
    {
        cpssOsPrintf("[%s:%d] Flow Id Database cleared appDemoIpfixTotalFlowsCount[%d]\n",
                __FUNCNAME__, __LINE__, appDemoIpfixTotalFlowsCount);
    }

    if(debug_print_enable == GT_TRUE)
    {
        cpssOsPrintf("[%s:%d] prvFlowManagerLocalDbDelete successfully exiting\n",__FUNCNAME__, __LINE__);
    }

    return rc;

}

GT_STATUS appDemoIpfixDelete
(
    IN  GT_U32                                 flowMngId
)
{
    GT_U32     entryIndex;
    GT_U32     profileIndex;
    GT_U32     index;
    GT_U32     ethMode;
    GT_U32     tpidBmp;

    GT_STATUS  rc = GT_OK;
    GT_U32     etherType = 0x8100;
    GT_BOOL    enable = GT_FALSE;
    GT_U32     appDemoIpfixDevNum = prvWrAppFirstDevNum(0);

    if(debug_print_enable == GT_TRUE)
    {
        cpssOsPrintf("[%s:%d] appDemoIpfixDelete entered\n",__FUNCNAME__, __LINE__);
    }

    /* Before deleting the flow manager related configurations, it is necessary
     * to validate whether the flow id exists or not else we may end up deleting
     * the other module init configurations and later the required flow manager
     * delete would not happen as desired in graceful manner.
     */
    if (appDemoIpfixFlowMngId == APP_DEMO_IPFIX_FLOW_MANAGER_ID_NOT_VALID ||
        appDemoIpfixFlowMngId != flowMngId)
    {
        rc = GT_FAIL;
        if(debug_print_enable == GT_TRUE)
        {
            cpssOsPrintf("[%s:%d] Flow manager ID doesn't exist rc[%d]\n",
                __FUNCNAME__, __LINE__, rc);
        }
        return rc;
    }

    /* JIRA: CPSS-9527
     * The ipfix delete should only be allowed after all the message packets
     * have been drained those inside PP received on Rx SDMA queue #6 before
     * FW stooped sending further message packets .Disabling the RX SDMA queue
     * #6 queue to ensure no further message packets are received on CPU.
     */
    rc = cpssDxChNetIfSdmaRxQueueEnable(appDemoIpfixDevNum, appDemoIpfixFwRxQueue, GT_FALSE);
    if (GT_OK != rc)
    {
        cpssOsPrintf("[%s:%d] cpssDxChNetIfSdmaRxQueueEnable FAILED, rc [%d] devNum[%d]\n",
                     __FUNCNAME__, __LINE__, rc, appDemoIpfixDevNum);
        return rc;
    }

    /* Deleting default rules on all ports */
    rc = prvFlowManagerInstallDefaultRules(appDemoIpfixDevNum, enable);
    if (GT_OK != rc)
    {
        cpssOsPrintf("[%s:%d] prvFlowManagerInstallDefaultRules FAILED, rc [%d] devNum[%d]\n",
                     __FUNCNAME__, __LINE__, rc, appDemoIpfixDevNum);
        return rc;
    }

    /* De-Configuring flow field config for IPv4-TCP pkt type */
    rc = appDemoIpfixIpv4TcpFlowFieldConfig(flowMngId, enable);
    if (GT_OK != rc)
    {
        cpssOsPrintf("[%s:%d] appDemoIpfixIpv4TcpFlowFieldConfig FAILED,"
                     "flowMngId [%d] rc[%d]\n",__FUNCNAME__, __LINE__,
                     flowMngId, rc);
        return rc;
    }

    /* Removing flow field config for IPv4-UDP pkt type */
    rc = appDemoIpfixIpv4UdpFlowFieldConfig(flowMngId, enable);
    if (GT_OK != rc)
    {
        cpssOsPrintf("[%s:%d] appDemoIpfixIpv4UdpFlowFieldConfig FAILED,"
                     "flowMngId [%d] rc[%d]\n",__FUNCNAME__, __LINE__,
                     flowMngId, rc);
        return rc;
    }

    /* Removing flow field config for OTHER pkt type */
    rc = appDemoIpfixOtherFlowFieldConfig(flowMngId, enable);
    if (GT_OK != rc)
    {
        cpssOsPrintf("[%s:%d] appDemoIpfixOtherFlowFieldConfig FAILED,"
                     "flowMngId [%d] rc[%d]\n",__FUNCNAME__, __LINE__,
                     flowMngId, rc);
        return rc;
    }

    /* De-Configuring flow field config for IPv4-TCP pkt type */
    rc = prvFlowManagerBindIpfixCountersToFwRules(appDemoIpfixDevNum, enable);
    if (GT_OK != rc)
    {
        cpssOsPrintf("[%s:%d] prvFlowManagerBindIpfixCountersToFwRules FAILED,"
                     " rc [%d] devNum[%d] enable[%d]\n",
                     __FUNCNAME__, __LINE__, rc, appDemoIpfixDevNum, enable);
        return rc;
    }

    /* Clearing the flow manager configurations */
    rc = prvFlowManagerConfig(flowMngId,
                              appDemoIpfixDevNum,
                              enable);
    if (GT_OK != rc)
    {
        cpssOsPrintf("[%s:%d] prvFlowManagerConfig FAILED, rc [%d] flowMngId [%d]\n",
                     __FUNCNAME__, __LINE__, rc, flowMngId );
        return rc;
    }

    appDemoIpfixFlowMngId = APP_DEMO_IPFIX_FLOW_MANAGER_ID_NOT_VALID;

    /* Ipfix Wrap-around configuration */
    rc = prvFlowManagerIpfixWrapAroundConfig(appDemoIpfixDevNum,
                                             enable);
    if (GT_OK != rc)
    {
        cpssOsPrintf("[%s:%d] prvFlowManagerIpfixWrapAroundConfig FAILED, rc [%d] devNum[%d]\n",
                     __FUNCNAME__, __LINE__, rc, appDemoIpfixDevNum);
        return rc;
    }

    /* Policer De-Init */
    rc = prvFlowManagerPolicerInit(appDemoIpfixDevNum,
                                   appDemoIpfixPolicerStage, enable);
    if (GT_OK != rc)
    {
        cpssOsPrintf("[%s:%d] prvFlowManagerPolicerInit FAILED, rc [%d] devNum[%d] enable[%d]\n",
                     __FUNCNAME__, __LINE__, rc, appDemoIpfixDevNum, enable);
        return rc;
    }

    /* PCL De-Init */
    rc = prvFlowManagerPclInit(appDemoIpfixDevNum,
                               appDemoIpfixLookupNum, enable);
    if (GT_OK != rc)
    {
        cpssOsPrintf("[%s:%d] prvFlowManagerPclInit FAILED, rc [%d] devNum[%d]\n",
                     __FUNCNAME__, __LINE__, rc, appDemoIpfixDevNum);
        return rc;
    }

    /* TCAM De-Init */
    rc = prvFlowManagerVirtualTcamInit(appDemoIpfixDevNum, enable);
    if (GT_OK != rc)
    {
        cpssOsPrintf("[%s:%d] prvFlowManagerVirtualTcamInit FAILED, rc [%d] devNum[%d]\n",
                     __FUNCNAME__, __LINE__, rc, appDemoIpfixDevNum);
        return rc;
    }

    /* Disable Vlan Configuration for QinQ */
    if(appDemoIpfixEnableQinQ == GT_TRUE)
    {
        etherType = 0x8100;
        for(entryIndex = 0; entryIndex < 1; entryIndex++)
        {
            rc = cpssDxChBrgVlanTpidEntrySet(appDemoIpfixDevNum, appDemoIpfixTableType, entryIndex, etherType);
            if (GT_OK != rc)
            {
                cpssOsPrintf("[%s:%d] cpssDxChBrgVlanTpidEntrySet FAILED :"
                    "devNum[%d],table type [%d], entryIndex[%d], etherType [%d], rc[%d]\n",
                    __FUNCNAME__, __LINE__, appDemoIpfixDevNum, appDemoIpfixTableType,
                    entryIndex, etherType, rc);
                return rc;
            }
        }
        tpidBmp = 0xFF;
        ethMode = appDemoIpfixEthMode1;
        for(profileIndex = 0; profileIndex < 2; profileIndex++)
        {
            rc = cpssDxChBrgVlanIngressTpidProfileSet(appDemoIpfixDevNum, profileIndex, ethMode, tpidBmp);
            if (GT_OK != rc)
            {
                cpssOsPrintf("[%s:%d] cpssDxChBrgVlanIngressTpidProfileSet FAILED :"
                    "devNum[%d],profileIndex[%d], ethMode[%d], tpidBmp[%d], rc[%d]\n",
                    __FUNCNAME__, __LINE__, appDemoIpfixDevNum, profileIndex,
                    ethMode, tpidBmp, rc);
                return rc;
            }
            index = 0;
            rc = cpssDxChBrgVlanPortIngressTpidProfileSet(appDemoIpfixDevNum, appDemoIpfixPort, ethMode, GT_FALSE, index);
            if (GT_OK != rc)
            {
                cpssOsPrintf("[%s:%d] cpssDxChBrgVlanPortIngressTpidProfileSet FAILED :"
                    "devNum[%d],profileIndex[%d], ethMode[%d] rc[%d]\n",
                    __FUNCNAME__, __LINE__, appDemoIpfixDevNum, index,
                    ethMode, rc);
                return rc;
            }
            ethMode = appDemoIpfixEthMode2;
        }
    }

    /* Deleting the UDB key DB along with flow ids DB */
    rc = prvFlowManagerLocalDbDelete(flowMngId);
    if (GT_OK != rc)
    {
        cpssOsPrintf("[%s:%d] prvFlowManagerLocalDbDelete FAILED, rc [%d] flowMngId[%d]\n",
                     __FUNCNAME__, __LINE__, rc, flowMngId);
        return rc;
    }

    rc = cpssDxChNetIfSdmaRxQueueEnable(appDemoIpfixDevNum, appDemoIpfixFwRxQueue, GT_TRUE);
    if (GT_OK != rc)
    {
        cpssOsPrintf("[%s:%d] cpssDxChNetIfSdmaRxQueueEnable FAILED, rc [%d] devNum[%d]\n",
                     __FUNCNAME__, __LINE__, rc, appDemoIpfixDevNum);
        return rc;
    }

    /* Restore the CPU Port Scheduler Profile binding and arbitration group
     * association with message packet queue.
     */
    rc = cpssDxChPortTxBindPortToSchedulerProfileSet(appDemoIpfixDevNum, CPSS_CPU_PORT_NUM_CNS, appDemoIpfixProfileGet);
    if (GT_OK != rc)
    {
      cpssOsPrintf("[%s:%d] cpssDxChPortTxBindPortToSchedulerProfileSet FAILED, rc [%d] devNum [%d] portNum %d profile %d\n",
          __FUNCNAME__, __LINE__, rc, appDemoIpfixDevNum, CPSS_CPU_PORT_NUM_CNS, appDemoIpfixProfileGet);
      return rc;
    }

    rc = cpssDxChPortTxQArbGroupSet(appDemoIpfixDevNum, appDemoIpfixMsgRxQueue, appDemoIpfixArbGroupGet, APP_DEMO_IPFIX_FLOW_MANAGER_CPU_PORT_PROFILE_CNS);
    if (GT_OK != rc)
    {
      cpssOsPrintf("[%s:%d] cpssDxChPortTxQArbGroupSet FAILED, rc [%d] devNum [%d] queue %d  arb group %d profile %d\n",
          __FUNCNAME__, __LINE__, rc, appDemoIpfixDevNum, appDemoIpfixMsgRxQueue, appDemoIpfixArbGroupGet,
          APP_DEMO_IPFIX_FLOW_MANAGER_CPU_PORT_PROFILE_CNS);
      return rc;
    }

    /* Enabling the below events generated by trapped packets to FW as Queue#6 is meant for
     * message packet and is not meant for application usage.
     */
    rc = cpssEventDeviceMaskSet(appDemoIpfixDevNum, CPSS_PP_RX_BUFFER_QUEUE6_E, CPSS_EVENT_UNMASK_E);
    if (GT_OK != rc)
    {
      cpssOsPrintf("[%s:%d] cpssEventDeviceMaskSet FAILED, rc [%d] devNum [%d] uniEv %d\n",
          __FUNCNAME__, __LINE__, rc, appDemoIpfixDevNum, CPSS_PP_RX_BUFFER_QUEUE6_E);
      return rc;
    }

    rc = cpssEventDeviceMaskSet(appDemoIpfixDevNum, CPSS_PP_RX_ERR_QUEUE6_E, CPSS_EVENT_UNMASK_E);
    if (GT_OK != rc)
    {
      cpssOsPrintf("[%s:%d] cpssEventDeviceMaskSet FAILED, rc [%d] devNum [%d] uniEv %d\n",
          __FUNCNAME__, __LINE__, rc, appDemoIpfixDevNum, CPSS_PP_RX_ERR_QUEUE6_E);
      return rc;
    }

    /* restore NA to CPU */
    rc = cpssDxChBrgVlanNaToCpuEnable(appDemoIpfixDevNum, 1, GT_TRUE);
    if (GT_OK != rc)
    {
        cpssOsPrintf("[%s:%d] cpssDxChBrgVlanNaToCpuEnable FAILED, rc [%d] devNum [%d]\n",
                     __FUNCNAME__, __LINE__, rc, appDemoIpfixDevNum);
        return rc;
    }

    if(debug_print_enable == GT_TRUE)
    {
        cpssOsPrintf("[%s:%d] appDemoIpfixDelete successfully exiting\n",__FUNCNAME__, __LINE__);
    }

    return GT_OK;
}

GT_STATUS appDemoIpfixAutoInitSystem
(
    GT_VOID
)
{
#ifndef CPSS_APP_PLATFORM_REFERENCE
    GT_STATUS   rc = GT_OK;

    rc = prvWrAppDbEntryAdd("useIpfixFlowManager", 1);
    if (rc != GT_OK)
    {
        return rc;
    }

    return cpssInitSystem(29,1,0);
#else
    return GT_OK;
#endif
}

GT_STATUS appDemoIpfixQinQEnableSet
(
    IN GT_BOOL  enableQinQ
)
{
    appDemoIpfixEnableQinQ = enableQinQ;
    return GT_OK;
}
