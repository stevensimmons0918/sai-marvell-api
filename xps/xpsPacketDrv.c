// xpsPacketDrv.c

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
********************************************************************************/

#include "xpsPacketDrv.h"
#include "xpsPort.h"
#include "xpsMac.h"
#include "openXpsInterface.h"
#include "xpsCommon.h"
#include "cpssHalHostIf.h"
#include "cpssHalUtil.h"
#include "cpssHalUtilOsNetworkStack.h"
#include "cpssHalDevice.h"
#include "cpssHalPort.h"
#include "cpssHalMirror.h"
#include "xpsAllocator.h"
#include "xpsAcl.h"
#include "cpssDxChPortCtrl.h"
#include <net/if.h>
#ifdef PCAP_SUPPORT
#include "pcap.h"
#endif //PCAP_SUPPORT

extern bool gResetInProgress;


#ifdef __cplusplus
extern "C" {
#endif

#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

#include <linux/genetlink.h>
#include <linux/netlink.h>

/*Example txPacket used for xpsShell based transmit*/
uint8_t txPacket[] =
{
    /* IPv4 */
    0x11, 0x22, 0x33, 0x44, 0x55, 0x02, 0x00, 0x0a,
    0x08, 0x0e, 0x0b, 0x0c, 0x08, 0x00, 0x00, 0x81,
    0x29, 0x45, 0x00, 0x08, 0x57, 0xd6, 0x6c, 0x00,
    0x11, 0x52, 0x00, 0x20, 0xef, 0xd2, 0x69, 0xcb,
    0x37, 0xa4, 0x26, 0x4f, 0x34, 0x34, 0x49, 0xe0,
    0x58, 0x00, 0x9c, 0x05, 0x4c, 0x49, 0x1f, 0xe8,
    0x94, 0x35, 0xa1, 0xc0, 0x2e, 0x77, 0x0d, 0xe4,
    0xca, 0x12, 0xf3, 0x30, 0x0c, 0x36, 0xc1, 0xb3,
    0x9f, 0x49, 0x4b, 0xf9, 0xec, 0x18, 0x45, 0x83,
    0x8c, 0x92, 0xd2, 0x01, 0xd7, 0xbd, 0x87, 0xb1,
    0x8c, 0x8f, 0xe0, 0x05, 0x45, 0x11, 0x68, 0x3f,
    0xf3, 0x38, 0x45, 0x8a, 0xc7, 0x9d, 0xc4, 0x11,
    0xc3, 0x2b, 0x6f, 0xf9, 0x28, 0xfd, 0x44, 0x38,
    0x13, 0x42

};

#define XP_NETDEV_MAX_IDS                 (MAX_PP_DEVICES_PER_SWITCH * MAX_PORTS_PER_PP)
#define XP_NETDEV_RANGE_START             0

static xpsDbHandle_t netDevCtxDbHndl = XPS_STATE_INVALID_DB_HANDLE;
static xpsDbHandle_t netDevDefaultFdDbHndl = XPS_STATE_INVALID_DB_HANDLE;
static xpsDbHandle_t hostIfTableGenetlinkCtxDbHndl =
    XPS_STATE_INVALID_DB_HANDLE;
static uint32_t txEventId;
fd_set fds;
static uint16_t etherTypeForVlanTag = 0x8100;

#define GENLMSG_DATA(glh) ((void *)((char*)NLMSG_DATA(glh) + GENL_HDRLEN))
#define GENLMSG_PAYLOAD(glh) (NLMSG_PAYLOAD(glh, 0) - GENL_HDRLEN)
#define NLA_DATA(na) ((void *)((char*)(na) + NLA_HDRLEN))

#define PSAMPLE_READNL_RCV_BUF 10240

#define XPS_GENETLINK_PORTNUM_INVALID_ID 0xFFFF

typedef struct
{
    struct nlmsghdr nlmsghd;
    struct genlmsghdr genlmsghd;
    char buf[PSAMPLE_READNL_RCV_BUF];
} nl_msg_t;

enum
{
    PSAMPLE_ATTR_IIFINDEX,
    PSAMPLE_ATTR_OIFINDEX,
    PSAMPLE_ATTR_ORIGSIZE,
    PSAMPLE_ATTR_SAMPLE_GROUP,
    PSAMPLE_ATTR_GROUP_SEQ,
    PSAMPLE_ATTR_SAMPLE_RATE,
    PSAMPLE_ATTR_DATA,
    PSAMPLE_ATTR_GROUP_REFCOUNT,
    PSAMPLE_ATTR_TUNNEL,

    __PSAMPLE_ATTR_MAX
};

enum psample_command
{
    PSAMPLE_CMD_SAMPLE,
    PSAMPLE_CMD_GET_GROUP,
    PSAMPLE_CMD_NEW_GROUP,
    PSAMPLE_CMD_DEL_GROUP,
};

typedef struct xpsNetDevDbEntry
{
    //Key
    uint32_t xpnetId;

    //Data
    char     netDevName[XPS_NETDEV_IF_NAMESZ];
    uint8_t  macAddr[6];
    xpPktCmd_e  pktCmd;
    xpsPacketDrvHostifVlanTagType tagType;
    int32_t fd;
} xpsNetDevDbEntry;

typedef enum xpsPktDrvDefaultFd_e
{
    XPS_PKT_DRV_DEFAULT_NETDEV_FD,
} xpsPktDrvDefaultFd_e;

//Holds the system default fd value.
typedef struct xpsNetDevDefaultFdDbEntry
{
    //Key
    xpsPktDrvDefaultFd_e  defaultnetDevFd;

    //Data
    int32_t    defaultFd;
} xpsNetDevDefaultFdDbEntry;

typedef struct xpsHostIfTableGenetlinkDbEntry
{
    //Key
    CPSS_NET_RX_CPU_CODE_ENT cpuCode;

    //Data
    int32_t    fd;
    uint32_t   mcgrpId;
    uint16_t   nl_family_id;
} xpsHostIfTableGenetlinkDbEntry;


static XP_STATUS xpsPacketDriverGetHostIfTableGenetlinkDb(xpsScope_t scopeId,
                                                          CPSS_NET_RX_CPU_CODE_ENT cpuCode,
                                                          xpsHostIfTableGenetlinkDbEntry **hostIfEntry);

static XP_STATUS xpsPacketDriverRemoveHostIfTableGenetlinkDb(xpsScope_t scopeId,
                                                             CPSS_NET_RX_CPU_CODE_ENT cpuCode);

XP_STATUS xpsHostIfGenetlinkWrite(uint8_t devnum, int32_t tnFd,
                                  uint16_t nl_family_id,
                                  uint32_t mcgrpId, GT_U8 *pktBuf,
                                  GT_U32 packetLen, GT_VOID *rxParamsPtr);

static xpsPacketDrvRxHandler  rxPacketHandler = NULL;
static xpsPacketDrvRxHandler  prevRxPacketHandler = NULL;

static int32_t netDevMgrKeyComp(void* key1, void* key2)
{
    return ((((xpsNetDevDbEntry *) key1)->xpnetId) - (((xpsNetDevDbEntry *)
                                                       key2)->xpnetId));
}

static int32_t pktDrvMgrDefaultVarKeyComp(void* key1, void* key2)
{
    return ((((xpsNetDevDefaultFdDbEntry *) key1)->defaultnetDevFd) - (((
                                                                            xpsNetDevDefaultFdDbEntry *) key2)->defaultnetDevFd));
}

static int32_t hostIfTableGenetlinkKeyComp(void* key1, void* key2)
{
    return ((((xpsHostIfTableGenetlinkDbEntry *) key1)->cpuCode) - (((
                                                                         xpsHostIfTableGenetlinkDbEntry *)
                                                                     key2)->cpuCode));
}

/* State maintainence functions*/

//Static variables DB functions
static XP_STATUS xpsPacketDriverGetDefaultVariablesDb(xpsScope_t scopeId,
                                                      xpsNetDevDefaultFdDbEntry ** defaultVarDbPtr)
{
    XP_STATUS result = XP_NO_ERR;
    xpsNetDevDefaultFdDbEntry keyDefaultVarDb;

    keyDefaultVarDb.defaultnetDevFd = XPS_PKT_DRV_DEFAULT_NETDEV_FD;

    if ((result = xpsStateSearchData(scopeId, netDevDefaultFdDbHndl,
                                     (xpsDbKey_t)&keyDefaultVarDb, (void**)defaultVarDbPtr)) != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "State search data failed");
        return result;
    }

    if (!(*defaultVarDbPtr))
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Key not found");
        return XP_ERR_KEY_NOT_FOUND;
    }

    return result;
}

static XP_STATUS xpsPacketDriverInsertDefaultVariablesDb(xpsScope_t scopeId,
                                                         int32_t fd)
{

    XP_STATUS result = XP_NO_ERR;
    xpsNetDevDefaultFdDbEntry * defaultVarDb = NULL;

    if ((result = xpsStateHeapMalloc(sizeof(xpsNetDevDefaultFdDbEntry),
                                     (void**)&defaultVarDb)) != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Heap malloc failed");
        return result;
    }
    if (defaultVarDb == NULL)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Null pointer recieved");
        return XP_ERR_NULL_POINTER;
    }

    memset(defaultVarDb, 0, sizeof(xpsNetDevDefaultFdDbEntry));
    defaultVarDb->defaultnetDevFd = XPS_PKT_DRV_DEFAULT_NETDEV_FD;
    defaultVarDb->defaultFd = fd;
    // Insert the stg static variable into the database
    if ((result = xpsStateInsertData(scopeId, netDevDefaultFdDbHndl,
                                     (void*)defaultVarDb)) != XP_NO_ERR)
    {
        xpsStateHeapFree((void*)defaultVarDb);
        defaultVarDb = NULL;
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "State insert data failed");
        return result;
    }
    return result;
}

static void print_packet(unsigned char * dataptr, int len)
{
    int val[len], i;

    cpssOsPrintf("\n length[%4d] ->  ", len);
    if (len > 11)
    {
        for (i =0; i < len ; i++)
        {
            val[i] = dataptr[i];
        }
        cpssOsPrintf("%02x:%02x:%02x:%02x:%02x:%02x ", val[0], val[1], val[2], val[3],
                     val[4], val[5]);
        cpssOsPrintf("%02x:%02x:%02x:%02x:%02x:%02x ", val[6], val[7], val[8], val[9],
                     val[10], val[11]);
        for (i = 12; i < len; i=i+2)
        {
            cpssOsPrintf("%02x%02x ", val[i], val[i+1]);
        }
    }
    cpssOsPrintf("\n");
}

static XP_STATUS xpsPacketDriverGetNetDevDb(xpsScope_t scopeId,
                                            uint32_t xpnetId, xpsNetDevDbEntry **netDevEntry);

/**
* @internal xpsPacketDriverCpuRxHandlerCb function
* @endinternal
*
* @brief   Callback function for incoming Rx packet in the CPU
*
* @param[in] cookie                   -  value
* @param[in] devNum                   - Device number
* @param[in] evType                   - event type
* @param[in] queueIdx                 - The queue from which this packet was received.
* @param[in] numOfBuff                - Num of used buffs in packetBuffs.
* @param[in] packetBuffs[]            - The received packet buffers list.
* @param[in] buffLen[]                - List of buffer lengths for packetBuffs.
* @param[in] rxParamsPtr              - (pointer to)information parameters of received packets
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
*/
static GT_STATUS xpsPacketDriverCpuRxHandlerCb
(
    IN  GT_UINTPTR                cookie,
    IN  RX_EV_HANDLER_EV_TYPE_ENT evType,
    IN  GT_U8                     devNum,
    IN  GT_U8                     queueIdx,
    IN  GT_U32                    numOfBuff,
    IN  GT_U8                     *packetBuffs[],
    IN  GT_U32                    buffLen[],
    IN  GT_VOID                   *rxParamsPtr
)
{
    XP_STATUS status = XP_NO_ERR;
    static uint8_t*   buff = NULL;
    uint8_t   keepVlanBuff[buffLen[0]+4];
    uint8_t   first4BitVlanTag;
    uint8_t   last8BitsVlanTag;
    uint32_t  ii, packetLen;
    uint8_t pass = XPS_NETDEV_FALSE;
    uint8_t insertVlanTag = XPS_NETDEV_FALSE;
    int32_t fd = -1;
    int32_t ingressPortNum = 0;
    int32_t cpssPortNum = 0;
    uint32_t hwDevNum = 0;
    uint32_t xpnetId = 0xFFFF;
    xpsNetDevDbEntry* netDevEntry = NULL;
    xpsHostIfTableGenetlinkDbEntry *hostIfEntry = NULL;
    xpsPacketDrvHostifVlanTagType tagType;
    xpsVlan_t pvid = 1;
    CPSS_NET_RX_CPU_CODE_ENT cpuCode = CPSS_NET_UNDEFINED_CPU_CODE_E;
    uint32_t isGenetlinkFd =  XPS_NETDEV_FALSE;
    NSLOCK();
    if (!buff)
    {
        if ((buff = (uint8_t*)cpssOsMalloc((sizeof(uint8_t))
                                           *CPSS_DXCH_PORT_MAX_MRU_CNS)) == NULL)
        {
            cpssOsPrintf("malloac failed\n");
            return XP_NO_ERR;
        }
    }
    memset(buff, 0, CPSS_DXCH_PORT_MAX_MRU_CNS);
    /* only packets which are forwarded and To CPU will pass */
    CPSS_DXCH_NET_RX_PARAMS_STC *params =
        (CPSS_DXCH_NET_RX_PARAMS_STC*)rxParamsPtr;

    if (params->dsaParam.dsaType == CPSS_DXCH_NET_DSA_CMD_TO_CPU_E)
    {
        /*handle trapped to CPU flow*/
        pass = XPS_NETDEV_TRUE;
        CPSS_DXCH_NET_DSA_TO_CPU_STC *toCpu = &(params->dsaParam.dsaInfo.toCpu);
        hwDevNum = toCpu->hwDevNum;
        cpssPortNum = toCpu->interface.ePort;
        if (toCpu->srcIsTrunk == GT_TRUE)
        {
            cpssPortNum = toCpu->interface.portNum;
        }
        cpssHalL2GPortNum(hwDevNum, cpssPortNum, &ingressPortNum);
        cpuCode = toCpu->cpuCode;

        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_DEBUG,
              "Rx on dev %d port %d %d isTrunk %d trunkId %d cpuCode %d", hwDevNum,
              ingressPortNum,
              toCpu->interface.ePort, toCpu->srcIsTrunk, toCpu->interface.srcTrunkId,
              cpuCode);
    }
    else
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Not CPU pkt, what to do?\n");
    }
    if (params->dsaParam.dsaType == CPSS_DXCH_NET_DSA_CMD_FORWARD_E)
    {
        /*handle forwarded to CPU flow*/
        pass = XPS_NETDEV_TRUE;
    }

    if (pass != XPS_NETDEV_TRUE)
    {
        NSUNLOCK();
        return XP_NO_ERR;
    }

    /* Get genetlink info from hostIf table db entry, Now we only
     * support packets with cpucode: Mirror to Analyzer(CPU Port).
     */
    if ((cpuCode == CPSS_NET_INGRESS_MIRRORED_TO_ANLYZER_E ||
         cpuCode == CPSS_NET_EGRESS_MIRRORED_TO_ANLYZER_E) &&
        (xpsPacketDriverGetHostIfTableGenetlinkDb(XP_SCOPE_DEFAULT, cpuCode,
                                                  &hostIfEntry) == XP_NO_ERR))
    {
        fd = hostIfEntry->fd;
        tagType = XPS_PKT_DRV_HOSTIF_VLAN_TAG_STRIP;
        isGenetlinkFd =  XPS_NETDEV_TRUE;

        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_DEBUG,
              "Rx on port %d cpuCode %d fd %d", ingressPortNum, cpuCode, fd);
    }
    else
    {
        /*Port InterfaceId is one to one mapped to xpnetId */
        xpnetId = ingressPortNum;
        //Get value of fd from Db using xpNetId.
        status = xpsPacketDriverGetNetDevDb(XP_SCOPE_DEFAULT, xpnetId, &netDevEntry);
        if (status != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_WARNING,
                  "Get NetDev Fd  failed %d for port %d\n", status, ingressPortNum);
            NSUNLOCK();
            return status;
        }
        fd = netDevEntry->fd;
        tagType = netDevEntry->tagType;
    }

    if (fd == -1)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Invalid fd, failed\n");
        NSUNLOCK();
        return XP_ERR_INVALID_DATA;
    }

    /*Handle CPU packet based on TagType
     *For tagType XPS_PKT_DRV_HOSTIF_VLAN_TAG_ORIGINAL.
     *The packet will be sent to hostIf/netDev as it is.
     */
    if (tagType == XPS_PKT_DRV_HOSTIF_VLAN_TAG_STRIP)
    {
        /*Strip Vlan tag if present*/
        if ((packetBuffs[0][12] == (GT_U8)(etherTypeForVlanTag >> 8)) &&
            (packetBuffs[0][13] == (GT_U8)(etherTypeForVlanTag & 0xFF)))
        {
            cpssOsMemCpy(&packetBuffs[0][12], &packetBuffs[0][16], (buffLen[0] - 16));
            buffLen[0] = buffLen[0] - 4;
        }
    }
    else if (tagType == XPS_PKT_DRV_HOSTIF_VLAN_TAG_KEEP)
    {
        if ((packetBuffs[0][12] == (GT_U8)(etherTypeForVlanTag >> 8)) &&
            (packetBuffs[0][13] == (GT_U8)(etherTypeForVlanTag & 0xFF)))
        {
            /*Do Nothing*/
        }
        else
        {
            /*Insert vlan tag i.e the pvid of the port*/
            status = xpsVlanGetPvid(CPSS_GLOBAL_SWITCH_ID_0, ingressPortNum, &pvid);
            if (status != XP_NO_ERR)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "xpsVlanGetPvid, failed for port %d\n", ingressPortNum);
                NSUNLOCK();
                return status;
            }
            first4BitVlanTag = pvid >> 8;
            last8BitsVlanTag = pvid & 0xFF;
            cpssOsMemCpy(keepVlanBuff, packetBuffs[0], 12);
            keepVlanBuff[12] = 0x8100 >> 8;
            keepVlanBuff[13] = 0x8100 & 0xFF;
            keepVlanBuff[14] = first4BitVlanTag;
            keepVlanBuff[15] = last8BitsVlanTag;
            cpssOsMemCpy(&keepVlanBuff[16], &packetBuffs[0][12], (buffLen[0] - 12));
            insertVlanTag = XPS_NETDEV_TRUE;

        }
    }

    /* send packet to OS net dev */
    if (numOfBuff == 1)
    {
        if (insertVlanTag == XPS_NETDEV_FALSE)
        {
            /* one buffer, no copy */
            if (isGenetlinkFd == XPS_NETDEV_FALSE)
            {
                status = cpssHalHostIfNetDevWrite(fd, packetBuffs[0], buffLen[0]);
            }
            else
            {
                status = xpsHostIfGenetlinkWrite(devNum, fd, hostIfEntry->nl_family_id,
                                                 hostIfEntry->mcgrpId, packetBuffs[0], buffLen[0], rxParamsPtr);
            }
            if (rxPacketHandler)
            {
                status = rxPacketHandler(devNum, ingressPortNum, packetBuffs[0], buffLen[0]);
                if (status != XP_NO_ERR)
                {
                    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                          "Failed to call LearnHandler: (%d)", status);
                    return GT_FALSE; // status;
                }
            }
            NSUNLOCK();
            return status;
        }
        else
        {
            if (isGenetlinkFd == XPS_NETDEV_FALSE)
            {
                status = cpssHalHostIfNetDevWrite(fd, keepVlanBuff, (buffLen[0]+4));
            }
            else
            {
                status = xpsHostIfGenetlinkWrite(devNum, fd, hostIfEntry->nl_family_id,
                                                 hostIfEntry->mcgrpId, keepVlanBuff, (buffLen[0]+4), rxParamsPtr);
            }
            if (rxPacketHandler)
            {
                status = rxPacketHandler(devNum, ingressPortNum, keepVlanBuff, (buffLen[0]+4));
                if (status != XP_NO_ERR)
                {
                    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                          "Failed to call LearnHandler: (%d)", status);
                    return GT_FALSE; // status;
                }
            }
            NSUNLOCK();
            return status;

        }
    }
    /* copy packet data */
    packetLen = 0;
    for (ii = 0; ii < numOfBuff; ii++)
    {
        /*Add vlan tag to first buffer data ,rest of packetData remains same*/
        if (ii == 0 && insertVlanTag == XPS_NETDEV_TRUE)
        {
            if (packetLen + (buffLen[0] + 4) > CPSS_DXCH_PORT_MAX_MRU_CNS)
            {
                /* packet length is bigger than destination buffer size */
                cpssOsMemCpy(buff + packetLen, keepVlanBuff,
                             CPSS_DXCH_PORT_MAX_MRU_CNS - packetLen);
                packetLen = CPSS_DXCH_PORT_MAX_MRU_CNS;
                break;
            }
            cpssOsMemCpy(buff + packetLen, keepVlanBuff, (buffLen[0]+4));
            packetLen += (buffLen[0]+4);

        }
        else
        {
            if (packetLen + buffLen[ii] > CPSS_DXCH_PORT_MAX_MRU_CNS)
            {
                /* packet length is bigger than destination buffer size */
                cpssOsMemCpy(buff + packetLen, packetBuffs[ii],
                             CPSS_DXCH_PORT_MAX_MRU_CNS - packetLen);
                packetLen = CPSS_DXCH_PORT_MAX_MRU_CNS;
                break;
            }
            cpssOsMemCpy(buff + packetLen, packetBuffs[ii], buffLen[ii]);
            packetLen += buffLen[ii];
        }
    }
    if (isGenetlinkFd == XPS_NETDEV_FALSE)
    {
        status = cpssHalHostIfNetDevWrite(fd, buff, packetLen);
    }
    else
    {
        status = xpsHostIfGenetlinkWrite(devNum, fd, hostIfEntry->nl_family_id,
                                         hostIfEntry->mcgrpId, buff, packetLen, rxParamsPtr);
    }
    if (status != XP_NO_ERR)
    {
        //Handle error
    }
    if (rxPacketHandler)
    {
        status = rxPacketHandler(devNum, ingressPortNum, buff, packetLen);
        if (status != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to call LearnHandler: (%d)", status);
            return GT_FALSE; // status;
        }
    }
    NSUNLOCK();
    return status;
}

static XP_STATUS xpsPacketDriverNetDevTapListen
(
    GT_U8  devId
)
{
    XP_STATUS status = XP_NO_ERR;
    uint32_t size = 0;
    static uint8_t* data = NULL;
    if (!data)
    {
        if ((data = (uint8_t*)cpssOsMalloc((sizeof(uint8_t))
                                           *CPSS_HOSTIF_MAX_READ_DATA_SIZE)) == NULL)
        {
            cpssOsPrintf("malloac failed\n");
            return XP_NO_ERR;
        }
    }
    memset(data, 0, CPSS_HOSTIF_MAX_READ_DATA_SIZE);

    XPS_HOSTIF_OUT_PACKET_BUFFER_STC pkt = {0};
    int select_rc = -1;
    uint32_t timeout = 20; //Seconds
    struct timeval tv;
    xpsScope_t scopeId = XP_SCOPE_DEFAULT;
    xpsNetDevDbEntry *currNetDevEntry = NULL;
    xpsNetDevDbEntry *prevNetDevEntry = NULL;
    int portNum, cpssDevNum = 0;
    uint32_t prevNetDevDbKey = 0xFFFFFFFF;
    while (1)
    {
        if (gResetInProgress)
        {
            txEventId = 0;
            return XP_NO_ERR;
        }

        /*1. set the timeout value*/
        tv.tv_sec = timeout;
        tv.tv_usec = 0;
        FD_ZERO(&fds);

        /*2. Set the netdev fds to readfds for select call listening*/
        //Get the first tree node for in-order traversal with NULL key
        status =  xpsStateGetNextData(scopeId, netDevCtxDbHndl, NULL,
                                      (void **)&currNetDevEntry);
        if (status != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Get Data failed");
            return status;
        }
        while (currNetDevEntry)
        {
            //Store the node information in previous and get-next
            prevNetDevEntry = currNetDevEntry;
            prevNetDevDbKey = prevNetDevEntry->xpnetId;
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_DEBUG, "NetDev Node, key  %d\n",
                  prevNetDevDbKey);
            if (prevNetDevEntry->fd != -1)
            {
                FD_SET(prevNetDevEntry->fd, &fds);
            }
            //get-next node
            status =  xpsStateGetNextData(scopeId, netDevCtxDbHndl,
                                          (xpsDbKey_t)&prevNetDevDbKey, (void **)&currNetDevEntry);
            if (status != XP_NO_ERR)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Get next node failed");
                return status;
            }
        }
        select_rc = select(FD_SETSIZE, &fds, (fd_set*)NULL, (fd_set*)NULL, &tv);
        if (select_rc < 0)
        {
            if (errno == EINTR)
            {
                return XP_ERR_SOCKET_SELECT;
            }
            continue;
        }
        /*3.Traverse through all netdev fds and check if readFds is set , then read data*/
        currNetDevEntry = NULL;
        prevNetDevEntry = NULL;
        prevNetDevDbKey = 0xFFFFFFFF;
        //Get the first tree node for in-order traversal with NULL key
        status =  xpsStateGetNextData(scopeId, netDevCtxDbHndl, NULL,
                                      (void **)&currNetDevEntry);
        if (status != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Get Data failed");
            return status;
        }
        while (currNetDevEntry)
        {
            //Store the node information in previous and get-next
            prevNetDevEntry = currNetDevEntry;
            prevNetDevDbKey = prevNetDevEntry->xpnetId;
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_DEBUG, "NetDev Node, key  %d\n",
                  prevNetDevDbKey);

            if (prevNetDevEntry->fd == -1)
            {
                continue;
            }
            if (FD_ISSET(prevNetDevEntry->fd, &fds))
            {
                size = CPSS_HOSTIF_MAX_READ_DATA_SIZE;
                do
                {
                    status = cpssHalHostIfNetDevRead(prevNetDevEntry->fd, data, &size);
                    if (status != XP_NO_ERR)
                    {
                        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_DEBUG, "HostIf Net dev read failed");
                    }
                }
                while (errno == EINTR);

                if (size > 0)
                {
                    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_DEBUG, "Tx on Port %d pkt size %d\n",
                          prevNetDevEntry->xpnetId, size);

                    status = cpssHalG2LPortNum((int)prevNetDevEntry->xpnetId, &cpssDevNum,
                                               &portNum);
                    if (status == XP_NO_ERR)
                    {
                        /*xpnetId is one to one mapped to portNum*/
                        pkt.outPortNum = portNum;

                        pkt.pktDataSize = size;
                        pkt.pktDataPtr = data;
                        status = cpssHalHostIfCPUtoPPPacketTransmit(cpssDevNum, &pkt);
                        if (status != XP_NO_ERR)
                        {
                            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                                  "cpssHalHostIfCPUtoPPPacketTransmit failed");
                        }
                        else
                        {
                            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_DEBUG,
                                  "cpssHalHostIfCPUtoPPPacketTransmit success");
                        }
                    }
                }
            }


            //get-next node
            status =  xpsStateGetNextData(scopeId, netDevCtxDbHndl,
                                          (xpsDbKey_t)&prevNetDevDbKey, (void **)&currNetDevEntry);
            if (status != XP_NO_ERR)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Get next node failed");
                return status;
            }
        }

    }
    return status;
}


static XP_STATUS xpsPacketDriverGetNetDevDb(xpsScope_t scopeId,
                                            uint32_t xpnetId, xpsNetDevDbEntry **netDevEntry)
{
    XP_STATUS result = XP_NO_ERR;
    xpsNetDevDbEntry keyNetDevDbEntry;

    memset(&keyNetDevDbEntry, 0x0, sizeof(xpsNetDevDbEntry));
    keyNetDevDbEntry.xpnetId = xpnetId;

    if ((result = xpsStateSearchData(scopeId, netDevCtxDbHndl,
                                     (xpsDbKey_t)&keyNetDevDbEntry, \
                                     (void**)netDevEntry)) != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Search data failed, xpnetId(%d)\n", xpnetId);
        return result;
    }
    if (!(*netDevEntry))
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_WARNING, "netdev does not exist\n");
        return XP_ERR_KEY_NOT_FOUND;
    }

    return result;
}

static XP_STATUS xpsPacketDriverRemoveNetDevDb(xpsScope_t scopeId,
                                               uint32_t xpnetId)
{

    XP_STATUS result = XP_NO_ERR;
    xpsNetDevDbEntry* netDevEntry = NULL;
    xpsNetDevDbEntry keyNetDevEntry;

    memset(&keyNetDevEntry, 0x0, sizeof(xpsNetDevDbEntry));
    keyNetDevEntry.xpnetId = xpnetId;

    if ((result = xpsStateDeleteData(scopeId, netDevCtxDbHndl,
                                     (xpsDbKey_t)&keyNetDevEntry, (void**)&netDevEntry)) != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Delete xpnetId entry failed\n");
        return result;
    }

    if ((result = xpsStateHeapFree((void*)netDevEntry)) != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Heap Freeing failed\n");
        return result;
    }

    return result;
}

static XP_STATUS xpsPacketDriverInsertNetDevDb(xpsScope_t scopeId,
                                               uint32_t xpnetId, xpsNetDevDbEntry** netDevEntry)
{

    XP_STATUS result = XP_NO_ERR;

    if ((result = xpsStateHeapMalloc(sizeof(xpsNetDevDbEntry),
                                     (void**)netDevEntry)) != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Memory allocation for net dev state failed\n");
        return result;
    }
    if (*netDevEntry == NULL)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Null pointer recieved");
        return XP_ERR_NULL_POINTER;
    }

    memset(*netDevEntry, 0, sizeof(xpsNetDevDbEntry));

    (*netDevEntry)->xpnetId = xpnetId;
    (*netDevEntry)->fd = -1;
    (*netDevEntry)->tagType = XPS_PKT_DRV_HOSTIF_VLAN_TAG_STRIP;

    // Insert the netdev state into the database, xpnetId is the key
    if ((result = xpsStateInsertData(scopeId, netDevCtxDbHndl,
                                     (void*)*netDevEntry)) != XP_NO_ERR)
    {
        xpsStateHeapFree((void*)*netDevEntry);
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "State insert data failed");
        return result;
    }

    return result;
}

static XP_STATUS xpsPacketDriverInitScope(xpsScope_t scopeId)
{
    XP_STATUS status = XP_NO_ERR;

    /* initialize the Stp Id Allocator*/
    status = xpsAllocatorInitIdAllocator(scopeId, XPS_ALLOCATOR_NETDEV_ID,
                                         XP_NETDEV_MAX_IDS, XP_NETDEV_RANGE_START);
    if (status != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to initialize XPS netdev Id allocator\n");
        return status;
    }

    netDevCtxDbHndl = XPS_NETDEV_DB_HNDL;
    // Create Global NetDev Db
    if ((status = xpsStateRegisterDb(scopeId, "Netdev Db", XPS_GLOBAL,
                                     &netDevMgrKeyComp, netDevCtxDbHndl)) != XP_NO_ERR)
    {
        netDevCtxDbHndl = XPS_STATE_INVALID_DB_HANDLE;
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Global netdev db creation failed");
        return status;
    }

    netDevDefaultFdDbHndl = XPS_NETDEV_DEFAULT_DATA_DB_HNDL;
    status = xpsStateRegisterDb(scopeId, "netdev default fd", XPS_GLOBAL,
                                &pktDrvMgrDefaultVarKeyComp, netDevDefaultFdDbHndl);
    if (status != XP_NO_ERR)
    {
        netDevDefaultFdDbHndl = XPS_STATE_INVALID_DB_HANDLE;
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Registering state db failed");
        return status;
    }

    hostIfTableGenetlinkCtxDbHndl = XPS_HOST_INTF_GENETLINK_DB_HNDL;
    status = xpsStateRegisterDb(scopeId, "hostIf table genetlink", XPS_GLOBAL,
                                &hostIfTableGenetlinkKeyComp, hostIfTableGenetlinkCtxDbHndl);
    if (status != XP_NO_ERR)
    {
        hostIfTableGenetlinkCtxDbHndl = XPS_STATE_INVALID_DB_HANDLE;
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Registering state db failed");
        return status;
    }
    return status;
}

static XP_STATUS xpsPacketDriverDeInitScope(xpsScope_t scopeId)
{
    XP_STATUS status = XP_NO_ERR;

    // Purge Global Interface Db
    if ((status = xpsStateDeRegisterDb(scopeId, &netDevCtxDbHndl)) != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Global netdev db purge failed");
        return status;
    }
    status = xpsStateDeRegisterDb(scopeId, &netDevDefaultFdDbHndl);
    if (status != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "xpsStateDeRegisterDb failed");
        return status;
    }
    status = xpsStateDeRegisterDb(scopeId, &hostIfTableGenetlinkCtxDbHndl);
    if (status != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "xpsStateDeRegisterDb failed");
        return status;
    }

    return status;
}

#if 0 // UNIT_TEST 
static void xpsPacketDriverNetDevDebug()
{
    uint32_t xpnetId = 0xFFFF;
    char netdev_name[]="eth10";
    int32_t fd = -1;
    XP_STATUS status = XP_NO_ERR;

    status = xpsNetdevXpnetIdAllocate(XP_SCOPE_DEFAULT, &xpnetId);
    if (status != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "xpsNetdevXpnetIdAllocate failed %d\n", status);
    }
    status = xpsNetdevIfCreate(0, xpnetId, netdev_name);
    if (status != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "xpsNetdevIfCreate failed %d\n", status);
    }
    xpsNetDevDbEntry* netDevEntry = NULL;
    //Get value of Name from Db using xpNetId.
    status = xpsPacketDriverGetNetDevDb(XP_SCOPE_DEFAULT, xpnetId, &netDevEntry);
    if (status != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "xpsPacketDriverGetNetDevDb failed %d\n", status);
        return ;
    }
    fd = netDevEntry->fd;
    status = xpsPacketDriverInsertDefaultVariablesDb(XP_SCOPE_DEFAULT, fd);
    if (status != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "xpsPacketDriverInsertDefaultVariablesDb failed %d\n", status);
        return ;
    }
    return;
}
#endif

XP_STATUS xpsPacketDriverInit(xpsDevice_t devId)
{
    XPS_FUNC_ENTRY_LOG();
    XP_STATUS status = XP_NO_ERR;

    status =  xpsPacketDriverInitScope(XP_SCOPE_DEFAULT);
    if (status != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Packet Driver Init failed ");
        return status;
    }
    status = cpssHalHostIfRxTxInit(devId);
    if (status != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Rx Tx Driver event handling path's init failed ");
        return status;
    }
    XPS_FUNC_EXIT_LOG();
    return XP_NO_ERR;
}

XP_STATUS xpsPacketDriverDeInit(xpsDevice_t devId)
{
    XPS_FUNC_ENTRY_LOG();
    XP_STATUS status = XP_NO_ERR;

    status = xpsPacketDriverDeInitScope(XP_SCOPE_DEFAULT);
    if (status != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Rx Tx Driver event handling path's de-init failed ");
        return status;
    }
    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

static unsigned __TASKCONV xpsPacketDriverTxThread
(
    GT_VOID * param
)
{
    XP_STATUS status = XP_NO_ERR;
    uint32_t devId = 0;

    status = xpsPacketDriverNetDevTapListen(devId);
    if (status != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "xpsPacketDriverNetDevTapListen failed %d \n", status);
        return status;
    }
    return status;
}

XP_STATUS xpsPacketDriverNetDevTxThread
(
)
{
    XP_STATUS status = XP_NO_ERR;
    GT_STATUS rc     = GT_OK;
    uint32_t eventHandlerTid; /* The task Tid */
    rc = cpssOsTaskCreate("xpsNetDevTx",
                          EV_HANDLER_MAX_PRIO,
                          _32KB,
                          xpsPacketDriverTxThread,
                          NULL,
                          &eventHandlerTid);
    txEventId = eventHandlerTid;
    if (rc != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Task creation failed");
        return XP_ERR_THREAD_CREATION;
    }
    return status;
}

XP_STATUS xpsPacketDriverNetDevTxThreadRemove(uint32_t eventHandlerTid)
{
    XP_STATUS status = XP_NO_ERR;
    GT_STATUS rc     = GT_OK;
    rc = cpssOsTaskDelete(eventHandlerTid);
    if (rc != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Task deletion failed");
        return xpsConvertCpssStatusToXPStatus(rc);
    }
    return status;
}

XP_STATUS xpsPacketDriverNetdevInit(xpsDevice_t devId)
{
    XPS_FUNC_ENTRY_LOG();
    XP_STATUS ret = XP_NO_ERR;

    ret = cpssHalHostIfNetDevInit((uint8_t)devId, xpsPacketDriverCpuRxHandlerCb);
    if (ret!= XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "cpssHalHostIfNetDeviceInit failed");
    }
    FD_ZERO(&fds);
    ret = xpsPacketDriverNetDevTxThread();
    if (ret!= XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "xpsPacketDriverNetDevTxThread failed");
    }
    XPS_FUNC_EXIT_LOG();
    return ret;
}

XP_STATUS xpsPacketDriverNetdevDeInit(xpsDevice_t devId)
{
    XPS_FUNC_ENTRY_LOG();
    XP_STATUS ret = XP_NO_ERR;

    ret = cpssHalHostIfNetDevDeInit((uint8_t)devId, xpsPacketDriverCpuRxHandlerCb);
    if (ret!= XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "cpssHalHostIfNetDeviceDeInit failed");
    }
    if (txEventId)
    {
        ret = xpsPacketDriverNetDevTxThreadRemove(txEventId);
        if (ret!= XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "xpsPacketDriverNetDevTxThreadRemove failed %d", ret);
        }
    }
    return ret;
    XPS_FUNC_EXIT_LOG();
    return XP_NO_ERR;
}

XP_STATUS xpsPacketDriverGetPktRxTxStatistics(xpsDevice_t devId)
{
    XPS_FUNC_ENTRY_LOG();
    XP_STATUS ret = XP_NO_ERR;
    ret = cpssHalHostIfSdmaRxCountersDisplay(devId);
    if (ret!= XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "cpssHalHostIfSdmaRxCountersDisplay failed");
    }
    return ret;
    XPS_FUNC_EXIT_LOG();
}

XP_STATUS xpSendPacket(xpDevice_t deviceId, uint32_t egressPort,
                       uint16_t pktSize, uint8_t queueNum, uint32_t *pktCopies)
{

    XP_STATUS retVal;
    struct xpPacketInfo **pktInfo = NULL;
    uint8_t *recPktData = NULL, *randomData = NULL;
    uint32_t vlanHeader = 0x8100;
    uint32_t vlanSize = 0;
    uint32_t copiesSent = 0, pktNum = 0;
    int rnd = 0, sendSize;
    ssize_t readBytes = -1;
    uint16_t minRandomPktSize, maxRandomPktSize;
    uint16_t pktLen = pktSize;

    if (queueNum > 8)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "ERR: Invalid Queue Num\n");
        return XP_ERR_INVALID_DATA;
    }

    /* PktLen < 64 is not valid */
    if ((pktLen) && (pktLen < 64 || pktLen > 10000))
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "ERR: Invalid pkt length\n");
        return XP_ERR_INVALID_DATA;
    }

    minRandomPktSize = 64;
    maxRandomPktSize = 10000;
    if (0 == pktSize)
    {
        /* Choose a random Pkt size */
        pktLen = (random() % (maxRandomPktSize - minRandomPktSize)) + minRandomPktSize;
    }

    /* Update the pointer to point to start of the header */
    randomData = (uint8_t *)xpMalloc((sizeof(uint8_t) * pktLen) + sizeof(
                                         vlanHeader));
    if (randomData == NULL)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Null pointer recieved");
        return XP_ERR_NULL_POINTER;
    }

    /* Fill Packet with Random Data */
    rnd = open("/dev/urandom", O_RDONLY);
    if (rnd != -1)
    {
        readBytes = read(rnd, randomData, pktLen);
        if (readBytes < 0)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Error: read failed with error = %s\n", strerror(errno));
        }
        close(rnd);
    }
    else
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Error: open failed with error = %s\n", strerror(errno));
    }

    pktInfo = (struct xpPacketInfo **)xpMalloc(sizeof(struct xpPacketInfo *) *
                                               (*pktCopies));
    if (pktInfo == NULL)
    {
        xpFree(randomData);
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Null pointer recieved");
        return XP_ERR_NULL_POINTER;
    }

    for (pktNum = 0 ; pktNum < *pktCopies; pktNum++)
    {

        pktInfo[pktNum]  = (struct xpPacketInfo *) xpMalloc(sizeof(
                                                                struct xpPacketInfo));
        if (pktInfo[pktNum] == NULL)
        {
            while (pktNum--)
            {
                xpFree(pktInfo[pktNum]);
            }
            xpFree(randomData);
            xpFree(pktInfo);
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Null pointer recieved");
            return XP_ERR_NULL_POINTER;
        }
        memset(pktInfo[pktNum], 0, sizeof(struct xpPacketInfo));

        pktInfo[pktNum]->buf = xpMalloc((sizeof(uint8_t) * pktLen) + sizeof(
                                            vlanHeader));
        if (!(pktInfo[pktNum]->buf))
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Error: Could not get buffer\n");
            xpFree(pktInfo[pktNum]);
            while (pktNum--)
            {
                xpFree(pktInfo[pktNum]->buf);
                xpFree(pktInfo[pktNum]);
            }
            xpFree(randomData);
            xpFree(pktInfo);
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Memory allocation error");
            return XP_ERR_MEM_ALLOC_ERROR;
        }

        pktInfo[pktNum]->priority = queueNum;

        recPktData = (uint8_t *)pktInfo[pktNum]->buf;

        memcpy(recPktData, randomData, pktLen);

        /* Copy MAC Addrs*/
        memcpy(recPktData, txPacket, 2 *sizeof(macAddr_t));

        sendSize = sizeof(txPacket);
        if (sendSize > pktLen)
        {
            sendSize = pktLen;
        }

        /*Copy remaining Data*/
        memcpy(recPktData + 2 * sizeof(macAddr_t) + vlanSize,
               txPacket + 2 * sizeof(macAddr_t), sendSize - 2 * sizeof(macAddr_t));
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_TRACE,
              "Print txPacket to be sent for copyNo = %d\n", pktNum);
        print_packet(recPktData, pktLen);
        /* Set packet size*/
        pktInfo[pktNum]->bufSize = pktLen;
    }

    copiesSent = *pktCopies;


    /* Pass the counter indicating pending copies to send, of the packet provided */
    /* Stay here till all the packets will be sent */
    retVal = xpsPacketDriverSend(deviceId, (const struct xpPacketInfo **)pktInfo,
                                 pktCopies, egressPort, SYNC_TX);

    for (pktNum = 0; pktNum < copiesSent; pktNum++)
    {
        xpFree(pktInfo[pktNum]->buf);
        xpFree(pktInfo[pktNum]);
    }
    xpFree(pktInfo);
    xpFree(randomData);

    if (retVal != XP_NO_ERR)
    {
        return retVal;
    }
    return XP_NO_ERR;
}

XP_STATUS xpsPacketDriverSend(xpsDevice_t devId,
                              const xpPacketInfo** const pktInfo, uint32_t *numOfPkt, uint32_t outPort,
                              txMode sendMode)
{
    XPS_FUNC_ENTRY_LOG();
    XP_STATUS ret = XP_NO_ERR;
    XPS_HOSTIF_OUT_PACKET_BUFFER_STC pkt = {0};
    uint32_t      pktSent = 0;
    if (sendMode == ASYNC_TX)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              " xpsPacketDriverSend failed as \
                ASYNC mode is not supported \n");
    }
    while (pktSent < *numOfPkt)
    {
        if (!pktInfo[pktSent]->buf)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "%s:\tERR: Null pointer passed \
                    as pointer to packet\n", __func__);
            ret = XP_ERR_NULL_POINTER;
            break;
        }
        pkt.pktDataPtr = (uint8_t*)pktInfo[pktSent]->buf;
        pkt.pktDataSize = pktInfo[pktSent]->bufSize;
        pkt.outPortNum =  outPort;
        ret = cpssHalHostIfCPUtoPPPacketTransmit(devId, &pkt);
        if (ret!=XP_NO_ERR)
        {
            return ret;
        }
        pktSent += 1;
    }
    XPS_FUNC_EXIT_LOG();
    return XP_NO_ERR;
}

XP_STATUS xpsPacketDriverReceive(int printPktCnt)
{
    XPS_FUNC_ENTRY_LOG();
    XP_STATUS ret = XP_NO_ERR;

    ret = cpssHalHostIfCPURxPacketPrintEnable(printPktCnt);
    if (ret != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "cpssHalHostIfCPURxPacketPrintEnable failed \n");
    }
    return ret;
    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsPacketDriverCpuCaptureStart(xpsDevice_t devId,
                                         const char* fileName)
{
    XPS_FUNC_ENTRY_LOG();

    if (fileName != NULL)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "file handling not supported");
    }
    return cpssHalHostIfCPURxPacketDumpEnable((GT_BOOL)XPS_NETDEV_TRUE);
    XPS_FUNC_EXIT_LOG();
}

XP_STATUS xpsPacketDriverCpuCaptureStop(xpsDevice_t devId)
{
    XPS_FUNC_ENTRY_LOG();

    return cpssHalHostIfCPURxPacketDumpEnable((GT_BOOL)XPS_NETDEV_FALSE);


}

XP_STATUS xpsNetdevXpnetIdAllocate(xpsScope_t scopeId, uint32_t *xpnetId)
{
    XPS_FUNC_ENTRY_LOG();
    XP_STATUS result = XP_NO_ERR;
    result = xpsAllocatorAllocateId(scopeId, XPS_ALLOCATOR_NETDEV_ID, xpnetId);
    if (result != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Allocate net dev id failed");
        return result;
    }

    XPS_FUNC_EXIT_LOG();
    return XP_NO_ERR;
}


XP_STATUS xpsNetdevXpnetIdAllocateWith(xpsScope_t scopeId, uint32_t xpnetId)
{
    XPS_FUNC_ENTRY_LOG();
    XP_STATUS result = XP_NO_ERR;
    result = xpsAllocatorAllocateWithId(scopeId, XPS_ALLOCATOR_NETDEV_ID, xpnetId);
    if (result != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "netdevId  allocation with Id failed, xpnetId(%d)", xpnetId);
        return result;
    }
    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsNetdevXpnetIdFree(xpsScope_t scopeId, uint32_t xpnetId)
{
    XPS_FUNC_ENTRY_LOG();
    XP_STATUS result = XP_NO_ERR;
    result = xpsAllocatorReleaseId(scopeId, XPS_ALLOCATOR_NETDEV_ID, xpnetId);
    if (result != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Release of xpnetId failed");
        return result;
    }
    XPS_FUNC_EXIT_LOG();
    return XP_NO_ERR;
}

XP_STATUS xpsNetdevIfCreate(xpsDevice_t devId, uint32_t xpnetId, char *ifName)
{
    XPS_FUNC_ENTRY_LOG();
    XP_STATUS status = XP_NO_ERR;
    int32_t fd = -1;
    uint8_t macAddr[6];
    uint32_t size = 0;
    xpsNetDevDbEntry* netDevEntry = NULL;
    status = cpssHalHostIfNetDevCreate(devId, (GT_U8*)ifName, &fd);
    if (status != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "net dev create failed %s",
              ifName);
        return status;
    }
    status = cpssHalHostIfNetDevGetMacAddr(fd, macAddr);
    if (status != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Net dev get mac address failed for %s", ifName);
        return status;
    }

    status = xpsPacketDriverInsertNetDevDb(XP_SCOPE_DEFAULT, xpnetId, &netDevEntry);
    if (status != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Insert net dev in db failed");
        return status;
    }
    netDevEntry->fd = fd;
    if (strlen(ifName) > (XPS_NETDEV_IF_NAMESZ-1))
    {
        size = XPS_NETDEV_IF_NAMESZ -1;
    }
    else
    {

        size = strlen(ifName);
    }
    //strcpy throws warning error(stringop-overflow) and here we have the size of block in advnce so using the memcpy
    memcpy(netDevEntry->netDevName, ifName, size);
    netDevEntry->netDevName[size] = '\0';
    memcpy(netDevEntry->macAddr, macAddr, CPSS_HOSTIF_MAC_ADDR_SIZE);
    netDevEntry->pktCmd =
        XP_PKTCMD_FWD; //default pktCmd for pkts with dest mac as netdev mac

    FD_SET(fd, &fds);
    XPS_FUNC_EXIT_LOG();
    return XP_NO_ERR;
}

XP_STATUS xpsNetdevIfDelete(xpsDevice_t devId, uint32_t xpnetId)
{
    XPS_FUNC_ENTRY_LOG();

    XP_STATUS status = XP_NO_ERR;
    int32_t fd = -1;
    xpsNetDevDbEntry* netDevEntry = NULL;

    status = xpsPacketDriverGetNetDevDb(XP_SCOPE_DEFAULT, xpnetId, &netDevEntry);
    if (status != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Get NetDev Fd  failed %d for xpnetId %d\n", status, xpnetId);
        return status;
    }
    fd = netDevEntry->fd;
    if (fd == -1)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Invalid fd, failed\n");
        return XP_ERR_INVALID_DATA;
    }

    status = cpssHalHostIfNetDevDestroy(devId, fd);
    if (status != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Net dev destroy failed");
        return status;
    }
    FD_CLR((int)fd, &fds);

    status = xpsPacketDriverRemoveNetDevDb(XP_SCOPE_DEFAULT, xpnetId);
    if (status != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Remove net dev db failed");
        return status;
    }
    XPS_FUNC_EXIT_LOG();
    return XP_NO_ERR;
}

static XP_STATUS xpsPacketDriverGetHostIfTableGenetlinkDb(xpsScope_t scopeId,
                                                          CPSS_NET_RX_CPU_CODE_ENT cpuCode,
                                                          xpsHostIfTableGenetlinkDbEntry **hostIfEntry)
{
    XP_STATUS result = XP_NO_ERR;
    xpsHostIfTableGenetlinkDbEntry keyHostIfDbEntry;

    memset(&keyHostIfDbEntry, 0x0, sizeof(xpsHostIfTableGenetlinkDbEntry));
    keyHostIfDbEntry.cpuCode = cpuCode;

    if ((result = xpsStateSearchData(scopeId, hostIfTableGenetlinkCtxDbHndl,
                                     (xpsDbKey_t)&keyHostIfDbEntry, \
                                     (void**)hostIfEntry)) != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Search data failed, cpuCode(%d)\n", cpuCode);
        return result;
    }
    if (!(*hostIfEntry))
    {
        return XP_ERR_KEY_NOT_FOUND;
    }

    return result;
}

static XP_STATUS xpsPacketDriverRemoveHostIfTableGenetlinkDb(xpsScope_t scopeId,
                                                             CPSS_NET_RX_CPU_CODE_ENT cpuCode)
{

    XP_STATUS result = XP_NO_ERR;
    xpsHostIfTableGenetlinkDbEntry* hostIfEntry = NULL;
    xpsHostIfTableGenetlinkDbEntry  keyHostIfDbEntry;

    memset(&keyHostIfDbEntry, 0x0, sizeof(xpsHostIfTableGenetlinkDbEntry));
    keyHostIfDbEntry.cpuCode = cpuCode;

    if ((result = xpsStateDeleteData(scopeId, netDevCtxDbHndl,
                                     (xpsDbKey_t)&keyHostIfDbEntry, (void**)&hostIfEntry)) != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Delete hostIf entry failed\n");
        return result;
    }

    if ((result = xpsStateHeapFree((void*)hostIfEntry)) != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Heap Freeing failed\n");
        return result;
    }

    return result;
}

static XP_STATUS xpsPacketDriverInsertHostIfTableGenetlinkDb(xpsScope_t scopeId,
                                                             CPSS_NET_RX_CPU_CODE_ENT cpuCode, xpsHostIfTableGenetlinkDbEntry** hostIfEntry)
{

    XP_STATUS result = XP_NO_ERR;

    if ((result = xpsStateHeapMalloc(sizeof(xpsHostIfTableGenetlinkDbEntry),
                                     (void**)hostIfEntry)) != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Memory allocation for hostIf table state failed\n");
        return result;
    }
    if (*hostIfEntry == NULL)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Null pointer recieved");
        return XP_ERR_NULL_POINTER;
    }

    memset(*hostIfEntry, 0, sizeof(xpsHostIfTableGenetlinkDbEntry));
    (*hostIfEntry)->cpuCode = cpuCode;
    (*hostIfEntry)->fd = -1;

    // Insert the hostIf table state into the database, cpuCode is the key
    if ((result = xpsStateInsertData(scopeId, hostIfTableGenetlinkCtxDbHndl,
                                     (void*)*hostIfEntry)) != XP_NO_ERR)
    {
        xpsStateHeapFree((void*)*hostIfEntry);
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "State insert data failed");
        return result;
    }

    return result;
}


XP_STATUS xpsHostIfGenetlinkGetFamilyId(uint8_t *genetlinkName,
                                        uint32_t fd, uint16_t *nl_family_id,
                                        uint8_t *mcgrpName, uint32_t *gid)
{
    struct nlattr       *nl_na;
    nl_msg_t            nl_request_msg;
    nl_msg_t            nl_response_msg;
    uint32_t            msglen = 0;
    struct sockaddr_nl  nladdr;

    /* Create family */
    nl_request_msg.nlmsghd.nlmsg_type  = GENL_ID_CTRL;
    nl_request_msg.nlmsghd.nlmsg_flags = NLM_F_REQUEST | NLM_F_ACK;
    nl_request_msg.nlmsghd.nlmsg_seq   = 0;
    nl_request_msg.nlmsghd.nlmsg_pid   = getpid();
    nl_request_msg.nlmsghd.nlmsg_len   = NLMSG_LENGTH(GENL_HDRLEN);

    nl_request_msg.genlmsghd.cmd       = CTRL_CMD_GETFAMILY;
    nl_request_msg.genlmsghd.version   = 0x1;

    nl_na = (struct nlattr *)GENLMSG_DATA(&nl_request_msg);
    nl_na->nla_type = CTRL_ATTR_FAMILY_NAME;
    nl_na->nla_len  = XPS_NETDEV_IF_NAMESZ + NLA_HDRLEN;
    memcpy(NLA_DATA(nl_na), genetlinkName, XPS_NETDEV_IF_NAMESZ);

    nl_request_msg.nlmsghd.nlmsg_len   += NLMSG_ALIGN(nl_na->nla_len);

    memset((void*)&nladdr, 0, sizeof(struct sockaddr_nl));
    nladdr.nl_family = AF_NETLINK;

    msglen = sendto(fd,
                    (char *)&nl_request_msg,
                    nl_request_msg.nlmsghd.nlmsg_len,
                    0,
                    (struct sockaddr*)&nladdr,
                    sizeof(struct sockaddr_nl));

    if (msglen != nl_request_msg.nlmsghd.nlmsg_len)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Error: Get family %s failed!\n", genetlinkName);
        return XP_ERR_INVALID_DATA;
    }

    msglen = recv(fd, &nl_response_msg, sizeof(nl_response_msg), 0);
    if (msglen < 0)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Error: Get family %s failed!\n", genetlinkName);
        return XP_ERR_INVALID_DATA;
    }

    if (!NLMSG_OK((&nl_response_msg.nlmsghd), msglen))
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Error: Get family %s failed!\n", genetlinkName);
        return XP_ERR_INVALID_DATA;
    }

    if (nl_response_msg.nlmsghd.nlmsg_type == NLMSG_ERROR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Error: Get family %s failed!\n", genetlinkName);
        return XP_ERR_INVALID_DATA;
    }

    nl_na = (struct nlattr *)GENLMSG_DATA(&nl_response_msg);
    while (nl_na && nl_na->nla_len > 0)
    {
        if (nl_na->nla_type == CTRL_ATTR_FAMILY_ID)
        {
            *nl_family_id = *(__u16 *)NLA_DATA(nl_na);
        }
        if (nl_na->nla_type == CTRL_ATTR_MCAST_GROUPS)
        {
            for (int grp_offset = NLA_HDRLEN; grp_offset < nl_na->nla_len;)
            {
                struct nlattr *grp_attr = (struct nlattr *)((char*)nl_na + grp_offset);
                if (grp_attr->nla_len == 0 ||
                    (grp_attr->nla_len + grp_offset) > nl_na->nla_len)
                {
                    break;
                }
                char *grp_name=NULL;
                uint32_t grp_id=0;

                for (int gf_offset = NLA_HDRLEN; gf_offset < grp_attr->nla_len;)
                {
                    struct nlattr *gf_attr = (struct nlattr *)((char*)nl_na + grp_offset +
                                                               gf_offset);
                    if (gf_attr->nla_len == 0 ||
                        (gf_attr->nla_len + gf_offset) > grp_attr->nla_len)
                    {
                        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                              "%s %d: process netlink parse error!\n", __func__, __LINE__);
                        break;
                    }
                    char *grp_attr_datap = (char *)gf_attr + NLA_HDRLEN;
                    switch (gf_attr->nla_type)
                    {
                        case CTRL_ATTR_MCAST_GRP_NAME:
                            grp_name = grp_attr_datap;
                            break;
                        case CTRL_ATTR_MCAST_GRP_ID:
                            grp_id = *(uint32_t *)grp_attr_datap;
                            break;
                    }
                    gf_offset += NLMSG_ALIGN(gf_attr->nla_len);
                }
                if (grp_name && mcgrpName && (strcmp((char*)grp_name, (char*)mcgrpName) == 0))
                {
                    *gid = grp_id;
                    break;
                }
                grp_offset += NLMSG_ALIGN(grp_attr->nla_len);
            }
            break;
        }
        else
        {
            nl_na = (struct nlattr *)((char *)nl_na + NLA_ALIGN(nl_na->nla_len));
        }
    }

    return XP_NO_ERR;
}

typedef struct genetlinkAttr
{
    uint32_t packetLen;
    uint16_t ingressPortNum;
    uint16_t egressPortNum;
    uint32_t vlanId;
    uint32_t sampleRate;
} genetlinkAttr_t;


uint32_t xpsGenetlinkGetSampleRate(xpsDevice_t devnum, uint32_t portNum,
                                   bool isIngress, uint32_t *analyzerId)
{
    uint32_t sampleRate = 0;
    GT_STATUS status = XP_NO_ERR;

    status = cpssHalPortMirrorGet(devnum, portNum, isIngress, analyzerId);
    if (status != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "cpssHalPortMirrorGet, failed for port %d\n", portNum);
        return sampleRate;
    }

    status = cpssHalMirrorSampleRateGet(devnum, *analyzerId, (GT_BOOL)isIngress,
                                        &sampleRate);
    if (status != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "cpssHalMirrorSampleRateGet, failed for port %d\n", portNum);
        sampleRate = 0;
        return sampleRate;
    }

    return sampleRate;
}

XP_STATUS xpsGenetlinkGetIfIndex(uint16_t portIndex, uint16_t *ifindex)
{
    xpsNetDevDbEntry* netDevEntry = NULL;
    char ifName[XPS_NETDEV_IF_NAMESZ] = {0};
    uint16_t result = 0;
    XP_STATUS status = XP_NO_ERR;

    status = xpsPacketDriverGetNetDevDb(XP_SCOPE_DEFAULT, portIndex, &netDevEntry);
    if (status != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Get NetDev Fd failed %d for portIndex %d\n", status, portIndex);
        return status;
    }

    memcpy(ifName, netDevEntry->netDevName, XPS_NETDEV_IF_NAMESZ);

    result = if_nametoindex(ifName);

    *ifindex = result;

    return XP_NO_ERR;
}

XP_STATUS xpsGenetlinkWriteMsg(uint8_t devnum, int32_t fd,
                               uint16_t nl_family_id, uint32_t mcgrpId,
                               genetlinkAttr_t *genetlinkAttr, GT_U8 *pktBuf)
{
    struct nlattr       *nl_na;
    nl_msg_t            nl_request_msg;
    uint32_t            msglen = 0;
    struct sockaddr_nl  nladdr;
    uint32_t enable = 1;
    int err;
    bool isIngress = GT_TRUE;
    uint32_t analyzerId;
    uint32_t portNum = 0;

    if ((err = setsockopt(fd, SOL_NETLINK, NETLINK_PKTINFO, &enable,
                          sizeof(enable))) < 0)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Error: Enable pktinfo %d failed \n", enable);
        close(fd);
        return XP_ERR_INVALID_DATA;
    }

    /* Fill nl msg header */
    nl_request_msg.nlmsghd.nlmsg_type  = nl_family_id;
    nl_request_msg.nlmsghd.nlmsg_flags = NLM_F_REQUEST;
    nl_request_msg.nlmsghd.nlmsg_seq   = 0;
    nl_request_msg.nlmsghd.nlmsg_len   = NLMSG_LENGTH(GENL_HDRLEN);

    /* Fill genl msg header */
    nl_request_msg.genlmsghd.cmd       = PSAMPLE_CMD_SAMPLE;
    nl_request_msg.genlmsghd.version   = 0x1;


    /* Fill sample gid for skb, The default
     * gid supported by hsflow is 1. */
    uint32_t smapleGid = 1;
    nl_na = (struct nlattr *)GENLMSG_DATA(&nl_request_msg);
    nl_na->nla_type = PSAMPLE_ATTR_SAMPLE_GROUP;
    memcpy(NLA_DATA(nl_na), &smapleGid, sizeof(uint32_t));
    nl_na->nla_len  = sizeof(uint32_t) + NLA_HDRLEN;
    nl_request_msg.nlmsghd.nlmsg_len   += NLMSG_ALIGN(nl_na->nla_len);

    /* Fill portNum for skb */
    if (genetlinkAttr->ingressPortNum != XPS_GENETLINK_PORTNUM_INVALID_ID)
    {
        uint16_t iifindex = 0;
        xpsGenetlinkGetIfIndex(genetlinkAttr->ingressPortNum, &iifindex);

        nl_na = (struct nlattr *)((char *)nl_na + NLA_ALIGN(nl_na->nla_len));
        nl_na->nla_type = PSAMPLE_ATTR_IIFINDEX;
        memcpy(NLA_DATA(nl_na), &iifindex, sizeof(uint16_t));
        nl_na->nla_len  = sizeof(uint16_t) + NLA_HDRLEN;
        nl_request_msg.nlmsghd.nlmsg_len   += NLMSG_ALIGN(nl_na->nla_len);

        portNum = genetlinkAttr->ingressPortNum;
    }
    else if (genetlinkAttr->egressPortNum != XPS_GENETLINK_PORTNUM_INVALID_ID)
    {
        uint16_t oifindex = 0;
        xpsGenetlinkGetIfIndex(genetlinkAttr->egressPortNum, &oifindex);

        nl_na = (struct nlattr *)((char *)nl_na + NLA_ALIGN(nl_na->nla_len));
        nl_na->nla_type = PSAMPLE_ATTR_OIFINDEX;
        memcpy(NLA_DATA(nl_na), &oifindex, sizeof(uint16_t));
        nl_na->nla_len  = sizeof(uint16_t) + NLA_HDRLEN;
        nl_request_msg.nlmsghd.nlmsg_len   += NLMSG_ALIGN(nl_na->nla_len);

        portNum = genetlinkAttr->egressPortNum;
        isIngress = GT_FALSE;
    }

    /* Fill sample rate for skb */
    uint32_t smapleRate = xpsGenetlinkGetSampleRate((xpsDevice_t)devnum,
                                                    portNum, isIngress, &analyzerId);
    nl_na = (struct nlattr *)((char *)nl_na + NLA_ALIGN(nl_na->nla_len));
    nl_na->nla_type = PSAMPLE_ATTR_SAMPLE_RATE;
    memcpy(NLA_DATA(nl_na), &smapleRate, sizeof(uint32_t));
    nl_na->nla_len  = sizeof(uint32_t) + NLA_HDRLEN;
    nl_request_msg.nlmsghd.nlmsg_len   += NLMSG_ALIGN(nl_na->nla_len);

    /* Fill packetLen for skb */
    nl_na = (struct nlattr *)((char *)nl_na + NLA_ALIGN(nl_na->nla_len));
    nl_na->nla_type = PSAMPLE_ATTR_ORIGSIZE;
    memcpy(NLA_DATA(nl_na), &(genetlinkAttr->packetLen), sizeof(uint32_t));
    nl_na->nla_len  = sizeof(uint32_t) + NLA_HDRLEN;
    nl_request_msg.nlmsghd.nlmsg_len   += NLMSG_ALIGN(nl_na->nla_len);

    /* Fill data for skb */
    nl_na = (struct nlattr *)((char *)nl_na + NLA_ALIGN(nl_na->nla_len));
    nl_na->nla_type = PSAMPLE_ATTR_DATA;
    memcpy(NLA_DATA(nl_na), pktBuf, genetlinkAttr->packetLen);
    nl_na->nla_len  = genetlinkAttr->packetLen + NLA_HDRLEN;
    nl_request_msg.nlmsghd.nlmsg_len   += NLMSG_ALIGN(nl_na->nla_len);

    memset((void*)&nladdr, 0, sizeof(struct sockaddr_nl));
    nladdr.nl_family = AF_NETLINK;
    nladdr.nl_groups = (1 << (mcgrpId - 1)) & 0xFFFFFFFF;

    msglen = sendto(fd,
                    (char *)&nl_request_msg,
                    nl_request_msg.nlmsghd.nlmsg_len,
                    0,
                    (struct sockaddr*)&nladdr,
                    sizeof(struct sockaddr_nl));

    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_DEBUG,
          "%s %d: Send Msg (len: %d, expect %d) to kernel, nl_groups %x family %d!\n",
          __func__, __LINE__, msglen, nl_request_msg.nlmsghd.nlmsg_len,
          (1 << (mcgrpId -1)) & 0xFFFFFFFF, nl_family_id);

    if (msglen != nl_request_msg.nlmsghd.nlmsg_len)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_DEFAULT,
              "Error: Send family %d failed!\n", nl_family_id);
        return XP_ERR_INVALID_DATA;
    }

    return XP_NO_ERR;
}

XP_STATUS xpsHostIfGenetlinkWrite(uint8_t devnum, int32_t tnFd,
                                  uint16_t nl_family_id,
                                  uint32_t mcgrpId, GT_U8 *pktBuf,
                                  GT_U32 packetLen, GT_VOID *rxParamsPtr)
{
    genetlinkAttr_t genetlinkInfo;
    CPSS_NET_RX_CPU_CODE_ENT cpuCode;
    XP_STATUS result = XP_NO_ERR;
    int32_t cpssPortNum = 0;
    uint32_t hwDevNum   = 0;

    /* only packets which are To CPU will pass */
    CPSS_DXCH_NET_RX_PARAMS_STC *params =
        (CPSS_DXCH_NET_RX_PARAMS_STC*)rxParamsPtr;
    if (params->dsaParam.dsaType != CPSS_DXCH_NET_DSA_CMD_TO_CPU_E)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Not CPU pkt, what to do?\n");
        return XP_ERR_INVALID_VALUE;
    }

    /* Get dsa info*/
    CPSS_DXCH_NET_DSA_TO_CPU_STC *toCpu = &(params->dsaParam.dsaInfo.toCpu);
    cpuCode = toCpu->cpuCode;
    hwDevNum = toCpu->hwDevNum;

    memset(&genetlinkInfo, 0, sizeof(genetlinkAttr_t));
    genetlinkInfo.ingressPortNum = XPS_GENETLINK_PORTNUM_INVALID_ID;
    genetlinkInfo.egressPortNum = XPS_GENETLINK_PORTNUM_INVALID_ID;
    if (cpuCode == CPSS_NET_INGRESS_MIRRORED_TO_ANLYZER_E)
    {
        cpssPortNum = toCpu->interface.ePort;
        if (toCpu->srcIsTrunk == GT_TRUE)
        {
            cpssPortNum = toCpu->interface.portNum;
        }
        cpssHalL2GPortNum(hwDevNum, cpssPortNum, (int*)&genetlinkInfo.ingressPortNum);
    }
    else if (cpuCode == CPSS_NET_EGRESS_MIRRORED_TO_ANLYZER_E)
    {
        cpssPortNum = toCpu->interface.ePort;
        if (toCpu->srcIsTrunk == GT_TRUE)
        {
            cpssPortNum = toCpu->interface.portNum;
        }
        cpssHalL2GPortNum(hwDevNum, cpssPortNum, (int*)&genetlinkInfo.egressPortNum);
    }
    else
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Invalid cpucode %d!", cpuCode);
        return XP_ERR_INVALID_ID;
    }

    genetlinkInfo.vlanId = params->dsaParam.commonParams.vid;
    genetlinkInfo.packetLen = packetLen;

    result = xpsGenetlinkWriteMsg(devnum, tnFd, nl_family_id, mcgrpId,
                                  &genetlinkInfo, pktBuf);
    if (result != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "xpsGenetlinkWriteMsg failed");
        return result;
    }

    return XP_NO_ERR;
}

XP_STATUS xpsHostIfTableGenetlinkCreate(xpsDevice_t devId, uint32_t reasonCode,
                                        int fd, uint16_t nl_family_id, uint32_t mcgrpId)
{
    XPS_FUNC_ENTRY_LOG();
    XP_STATUS status = XP_NO_ERR;
    uint8_t count = 0;
    xpsHostIfTableGenetlinkDbEntry* hostIfEntry = NULL;
    CPSS_NET_RX_CPU_CODE_ENT cpuCode[MAX_CPU_CODE_COUNT];

    if (reasonCode >= XPS_COPP_CTRL_PKT_MAX)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Invalid reason code %d, " "supported number of reason codes %d\n", reasonCode,
              XPS_COPP_CTRL_PKT_MAX);
        return XP_ERR_VALUE_OUT_OF_RANGE;
    }

    /* Convert xps reason code to cpss cpu code */
    (void)xpsUtilConvertReasonCodeToCpssCpuCode((xpsCoppCtrlPkt_e)reasonCode,
                                                cpuCode, &count);

    for (int i =0; i < count; i++)
    {
        hostIfEntry = NULL;
        status = xpsPacketDriverGetHostIfTableGenetlinkDb(XP_SCOPE_DEFAULT, cpuCode[i],
                                                          &hostIfEntry);
        if (status == XP_ERR_KEY_NOT_FOUND)
        {
            status = xpsPacketDriverInsertHostIfTableGenetlinkDb(XP_SCOPE_DEFAULT,
                                                                 cpuCode[i], &hostIfEntry);
            if (status != XP_NO_ERR)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Insert net dev in db failed");
                return status;
            }
        }
        hostIfEntry->fd = fd;
        hostIfEntry->nl_family_id = nl_family_id;
        hostIfEntry->mcgrpId = mcgrpId;

        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_DEFAULT,
              "Insert db, cpuCode %d fd %d, familyId %d mcgrpId %d",
              cpuCode[i], hostIfEntry->fd, hostIfEntry->nl_family_id, hostIfEntry->mcgrpId);
    }

    XPS_FUNC_EXIT_LOG();
    return XP_NO_ERR;
}


XP_STATUS xpsHostIfTableGenetlinkDelete(xpsDevice_t devId, uint32_t reasonCode)
{
    XPS_FUNC_ENTRY_LOG();

    XP_STATUS status = XP_NO_ERR;
    uint8_t count = 0;
    xpsHostIfTableGenetlinkDbEntry* hostIfEntry = NULL;
    CPSS_NET_RX_CPU_CODE_ENT cpuCode[MAX_CPU_CODE_COUNT];

    if (reasonCode >= XPS_COPP_CTRL_PKT_MAX)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Invalid reason code %d, " "supported number of reason codes %d\n", reasonCode,
              XPS_COPP_CTRL_PKT_MAX);
        return XP_ERR_VALUE_OUT_OF_RANGE;
    }

    /* Convert xps reason code to cpss cpu code */
    (void)xpsUtilConvertReasonCodeToCpssCpuCode((xpsCoppCtrlPkt_e)reasonCode,
                                                cpuCode, &count);

    for (int i =0; i < count; i++)
    {
        status = xpsPacketDriverGetHostIfTableGenetlinkDb(XP_SCOPE_DEFAULT, cpuCode[i],
                                                          &hostIfEntry);
        if (status != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Get HostIf table failed %d for cpuCode %d\n", status, cpuCode[i]);
            return status;
        }

        status = xpsPacketDriverRemoveHostIfTableGenetlinkDb(XP_SCOPE_DEFAULT,
                                                             cpuCode[i]);
        if (status != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Remove hostIf table db failed");
            return status;
        }
    }

    XPS_FUNC_EXIT_LOG();
    return XP_NO_ERR;
}

#if 0
XP_STATUS xpsPacketDriverNetDevSetArpReplyPktCmdForSwitch(xpsDevice_t devId,
                                                          xpPktCmd_e  pktCmd)
{
    XPS_FUNC_ENTRY_LOG();

    XP_STATUS status = XP_NO_ERR;
    xpsScope_t scopeId = XP_SCOPE_DEFAULT;
    xpsNetDevDbEntry *currNetDevEntry = NULL;
    xpsNetDevDbEntry *prevNetDevEntry = NULL;
    uint32_t prevNetDevDbKey = 0xFFFFFFFF;
    xpsVlan_t pvid = 1;

    //Get the first tree node for in-order traversal with NULL key
    status =  xpsStateGetNextData(scopeId, netDevCtxDbHndl, NULL,
                                  (void **)&currNetDevEntry);
    if (status != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Get Data failed");
        return status;
    }
    while (currNetDevEntry)
    {
        //Store the node information in previous and get-next
        prevNetDevEntry = currNetDevEntry;
        prevNetDevDbKey = prevNetDevEntry->xpnetId;
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_DEBUG, "NetDev Node, key  %d\n",
              prevNetDevDbKey);

        if (prevNetDevEntry->pktCmd !=  pktCmd)
        {
            status = xpsVlanGetPvid(devId, prevNetDevDbKey, &pvid);
            if (status != XP_NO_ERR)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "xpsVlanGetPvid, failed for port %d\n", pvid);
                return status;
            }
            NSLOCK();
            status = cpssHalHostIfNetDevSetPktCmdFdbEntry(devId, prevNetDevEntry->fd, pvid,
                                                          prevNetDevEntry->macAddr, pktCmd);
            NSUNLOCK();
            if (status != XP_NO_ERR)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "cpssHalHostIfNetDevSetPktCmdFdbEntry, failed for port %d\n", pvid);
                return XP_NO_ERR;
                //return status;
            }
        }
        prevNetDevEntry->pktCmd = pktCmd;
        //get-next node
        status =  xpsStateGetNextData(scopeId, netDevCtxDbHndl,
                                      (xpsDbKey_t)&prevNetDevDbKey, (void **)&currNetDevEntry);
        if (status != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Get next node failed");
            return status;
        }
    }
    XPS_FUNC_EXIT_LOG();
    return XP_NO_ERR;
}
#endif

XP_STATUS xpsNetdevTrapSet(xpsDevice_t devId, uint32_t trapId, uint32_t rCode,
                           uint32_t ch, int32_t fd, bool flag)
{
    XPS_FUNC_ENTRY_LOG();
    XPS_FUNC_EXIT_LOG();
    return XP_NO_ERR;
}

XP_STATUS xpsNetdevTrapGet(xpsDevice_t devId, uint32_t trapId, int32_t* fd)
{
    XPS_FUNC_ENTRY_LOG();
    XP_STATUS result = XP_NO_ERR;
    xpsNetDevDefaultFdDbEntry *defaultVarData;
    xpsScope_t scopeId = XP_SCOPE_DEFAULT;

    result = xpsPacketDriverGetDefaultVariablesDb(scopeId, &defaultVarData);
    if (result != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, " Default fd get failed");
        return result;
    }
    *fd = defaultVarData->defaultFd;
    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsNetdevDefaultTrapSet(xpsDevice_t devId, uint32_t xpnetId)
{
    XPS_FUNC_ENTRY_LOG();
    xpsNetDevDbEntry* netDevEntry = NULL;
    XP_STATUS status = XP_NO_ERR;
    int32_t fd = -1;
    //Get value of Name from Db using xpNetId.
    status = xpsPacketDriverGetNetDevDb(XP_SCOPE_DEFAULT, xpnetId, &netDevEntry);
    if (status != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "xpsPacketDriverGetNetDevDb failed %d\n", status);
        return status;
    }
    fd = netDevEntry->fd;
    status = xpsPacketDriverInsertDefaultVariablesDb(XP_SCOPE_DEFAULT, fd);
    if (status != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "xpsPacketDriverInsertDefaultVariablesDb failed %d\n", status);
        return status;
    }
    XPS_FUNC_EXIT_LOG();
    return XP_NO_ERR;
}

XP_STATUS xpsPacketDriverNetdevVlanTagTypeSet(xpsDevice_t devId,
                                              uint32_t xpnetId, xpsPacketDrvHostifVlanTagType tagType)
{
    XPS_FUNC_ENTRY_LOG();
    xpsNetDevDbEntry* netDevEntry = NULL;
    XP_STATUS status = XP_NO_ERR;
    //Get value of Name from Db using xpNetId.
    status = xpsPacketDriverGetNetDevDb(XP_SCOPE_DEFAULT, xpnetId, &netDevEntry);
    if (status != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "xpsPacketDriverGetNetDevDb failed %d\n", status);
        return status;
    }
    switch (tagType)
    {
        case XPS_PKT_DRV_HOSTIF_VLAN_TAG_ORIGINAL:
            netDevEntry->tagType = XPS_PKT_DRV_HOSTIF_VLAN_TAG_ORIGINAL;
            break;
        case XPS_PKT_DRV_HOSTIF_VLAN_TAG_KEEP:
            netDevEntry->tagType = XPS_PKT_DRV_HOSTIF_VLAN_TAG_KEEP;
            break;
        case XPS_PKT_DRV_HOSTIF_VLAN_TAG_STRIP:
            netDevEntry->tagType = XPS_PKT_DRV_HOSTIF_VLAN_TAG_STRIP;
            break;
        default:
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "HostIfVlanTag type %d is not supported\n", tagType);
            return XP_ERR_NOT_SUPPORTED;
    }
    XPS_FUNC_EXIT_LOG();
    return XP_NO_ERR;
}

XP_STATUS xpsPacketDriverNetdevVlanTagTypeGet(xpsDevice_t devId,
                                              uint32_t xpnetId, xpsPacketDrvHostifVlanTagType* tagType)
{
    XPS_FUNC_ENTRY_LOG();
    xpsNetDevDbEntry* netDevEntry = NULL;
    XP_STATUS status = XP_NO_ERR;
    //Get value of Name from Db using xpNetId.
    status = xpsPacketDriverGetNetDevDb(XP_SCOPE_DEFAULT, xpnetId, &netDevEntry);
    if (status != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "xpsPacketDriverGetNetDevDb failed %d\n", status);
        return status;
    }
    *tagType = netDevEntry->tagType;
    XPS_FUNC_EXIT_LOG();
    return XP_NO_ERR;
}

XP_STATUS xpsNetdevLinkStatusSet(xpsDevice_t devId, uint32_t xpnetId,
                                 bool status)
{
    XPS_FUNC_ENTRY_LOG();
    char name[XPS_NETDEV_IF_NAMESZ] = "";
    XP_STATUS rc = XP_NO_ERR;
    xpsNetDevDbEntry* netDevEntry = NULL;
    //Get value of Name from Db using xpNetId.
    rc = xpsPacketDriverGetNetDevDb(XP_SCOPE_DEFAULT, xpnetId, &netDevEntry);
    if (rc != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_WARNING, "Get net dev db failed");
        return rc;
    }
    strncpy(name, netDevEntry->netDevName, XPS_NETDEV_IF_NAMESZ);
    if (status == true)
    {
        if (GT_OK != cpssHalUtilOsNetworkSetLinkState(name, true))
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "cpssHalUtilOsNetworkSetLinkState failed %s UP\n", name);
            rc = XP_ERR_INVALID_DATA;
        }
    }
    else
    {
        if (GT_OK != cpssHalUtilOsNetworkSetLinkState(name, false))
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "cpssHalUtilOsNetworkSetLinkState failed %s DOWN\n", name);
            rc = XP_ERR_INVALID_DATA;
        }
    }

    XPS_FUNC_EXIT_LOG();

    return rc;
}

XP_STATUS xpSendPacketWithDSA(xpDevice_t deviceId, uint32_t egressPort,
                              uint16_t pktSize, uint8_t queueNum, uint32_t *pktCopies,
                              xpDsaHeader *xpDsaHeader_stc, xpEthHdr_t *ethHdr)
{

    XP_STATUS retVal;
    struct xpPacketInfo **pktInfo = NULL;
    uint8_t *recPktData = NULL, *randomData = NULL;
    uint32_t vlanHeader;
    uint32_t vlanSize = 0;
    uint32_t copiesSent = 0, pktNum = 0;
    int rnd = 0, sendSize;
    ssize_t readBytes = -1;
    uint16_t minRandomPktSize, maxRandomPktSize;
    uint16_t pktLen = pktSize;

    if (queueNum > TC_PRIORITY_7)
    {
        queueNum = 0;
    }

    /* PktLen < 64 is not valid */
    if ((pktLen) && (pktLen < 64 || pktLen > 10000))
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "ERR: Invalid pkt length\n");
        return XP_ERR_INVALID_DATA;
    }

    minRandomPktSize = 64;
    maxRandomPktSize = 10000;
    if (0 == pktSize)
    {
        /* Choose a random Pkt size */
        pktLen = (random() % (maxRandomPktSize - minRandomPktSize)) + minRandomPktSize;
    }

    /* Update the pointer to point to start of the header */
    randomData = (uint8_t *)xpMalloc((sizeof(uint8_t) * pktLen) + sizeof(
                                         vlanHeader));
    if (randomData == NULL)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Null pointer recieved");
        return XP_ERR_NULL_POINTER;
    }

    /* Fill Packet with Random Data */
    rnd = open("/dev/urandom", O_RDONLY);
    if (rnd != -1)
    {
        readBytes = read(rnd, randomData, pktLen);
        if (readBytes < 0)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Error: read failed with error = %s\n", strerror(errno));
        }
        close(rnd);
    }
    else
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Error: open failed with error = %s\n", strerror(errno));
    }

    pktInfo = (struct xpPacketInfo **)xpMalloc(sizeof(struct xpPacketInfo *) *
                                               (*pktCopies));
    if (pktInfo == NULL)
    {
        xpFree(randomData);
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Null pointer recieved");
        return XP_ERR_NULL_POINTER;
    }

    for (pktNum = 0 ; pktNum < *pktCopies; pktNum++)
    {

        pktInfo[pktNum]  = (struct xpPacketInfo *) xpMalloc(sizeof(
                                                                struct xpPacketInfo));
        if (pktInfo[pktNum] == NULL)
        {
            while (pktNum--)
            {
                xpFree(pktInfo[pktNum]);
            }
            xpFree(randomData);
            xpFree(pktInfo);
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Null pointer recieved");
            return XP_ERR_NULL_POINTER;
        }
        memset(pktInfo[pktNum], 0, sizeof(struct xpPacketInfo));

        pktInfo[pktNum]->buf = xpMalloc((sizeof(uint8_t) * pktLen) + sizeof(
                                            vlanHeader));
        if (!(pktInfo[pktNum]->buf))
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Error: Could not get buffer\n");
            xpFree(pktInfo[pktNum]);
            while (pktNum--)
            {
                xpFree(pktInfo[pktNum]->buf);
                xpFree(pktInfo[pktNum]);
            }
            xpFree(randomData);
            xpFree(pktInfo);
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Memory allocation error");
            return XP_ERR_MEM_ALLOC_ERROR;
        }

        pktInfo[pktNum]->priority = queueNum;

        recPktData = (uint8_t *)pktInfo[pktNum]->buf;

        memcpy(recPktData, randomData, pktLen);

        /* Copy MAC Addrs*/
        memcpy(recPktData, txPacket, 2 *sizeof(macAddr_t));

        /* update the SA*/
        if (ethHdr->isSASet)
        {
            memcpy(recPktData + sizeof(macAddr_t), ethHdr->macSA, sizeof(macAddr_t));
        }

        /* update the DA*/
        if (ethHdr->isDASet)
        {
            memcpy(recPktData, ethHdr->macDA, sizeof(macAddr_t));
        }

        /* update the Insert VLAN Tag*/
        if (ethHdr->isVlanIdSet)
        {
            if (ethHdr->vlanId > 0 && ethHdr->vlanId <XPS_VLANID_MAX)
            {
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
                vlanHeader = htobe16(ethHdr->vlanId);
                vlanHeader = (vlanHeader << 16) | 0x0081;
#else
                vlanHeader = 0x8100;
                vlanHeader = (vlanHeader << 16) | (ethHdr->vlanId);
#endif
                memcpy(recPktData + 2 * sizeof(macAddr_t), &vlanHeader, sizeof(vlanHeader));
                vlanSize = sizeof(vlanHeader);

            }
        }
        sendSize = sizeof(txPacket);
        if (sendSize > pktLen)
        {
            sendSize = pktLen;
        }

        /*Copy remaining Data*/
        memcpy(recPktData + 2 * sizeof(macAddr_t) + vlanSize,
               txPacket + 2 * sizeof(macAddr_t), sendSize - 2 * sizeof(macAddr_t));
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_TRACE,
              "Print txPacket to be sent for copyNo = %d\n", pktNum);
        print_packet(recPktData, pktLen);
        /* Set packet size*/
        pktInfo[pktNum]->bufSize = pktLen;
    }

    copiesSent = *pktCopies;


    /* Pass the counter indicating pending copies to send, of the packet provided */
    /* Stay here till all the packets will be sent */
    retVal = xpsPacketDriverSendWithDSA(deviceId,
                                        (const struct xpPacketInfo **)pktInfo, pktCopies, egressPort, SYNC_TX,
                                        xpDsaHeader_stc);

    for (pktNum = 0; pktNum < copiesSent; pktNum++)
    {
        xpFree(pktInfo[pktNum]->buf);
        xpFree(pktInfo[pktNum]);
    }
    xpFree(pktInfo);
    xpFree(randomData);

    if (retVal != XP_NO_ERR)
    {
        return retVal;
    }
    return XP_NO_ERR;
}

XP_STATUS xpSendPcapWithDSA(xpDevice_t deviceId, uint32_t egressPort,
                            uint8_t queueNum, const char *pcapFile, xpDsaHeader *xpDsaHeader_stc)
{
    XP_STATUS retVal = XP_ERR_OP_NOT_SUPPORTED;
#ifdef PCAP_SUPPORT
    struct xpPacketInfo *pktInfo = NULL;
    void *recPktData = NULL;
    char errbuf[PCAP_ERRBUF_SIZE];
    pcap_t *pcap = NULL;
    uint8_t *pdata = NULL;
    uint32_t numOfPkt = 0;
    struct pcap_pkthdr hdr;

    if (!pcapFile)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Error : pcapFile is null \n");
        return XP_ERR_NULL_POINTER;
    }

    pktInfo = (struct xpPacketInfo*)xpMalloc(sizeof(struct xpPacketInfo));
    if (!pktInfo)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "%s:Error: malloc for pktInfo failed\n", __FUNCNAME__);
        return XP_ERR_MEM_ALLOC_ERROR;
    }

    memset(pktInfo, 0, sizeof(struct xpPacketInfo));
    if (queueNum < TC_PRIORITY_0 || queueNum > TC_PRIORITY_7)
    {
        queueNum = 0;
    }
    pktInfo->priority = queueNum;

    pcap = pcap_open_offline(pcapFile, errbuf);
    if (pcap == NULL)
    {
        xpFree(pktInfo);
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "%s: Error: could not get pcap: %s\n", __FUNCNAME__, strerror(errno));
        return XP_ERR_FILE_OPEN;
    }

    while (NULL != (pdata = (uint8_t *)pcap_next(pcap, &hdr)))
    {

        pktInfo->buf = xpMalloc(sizeof(uint8_t) * hdr.len);
        if (!pktInfo->buf)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "%s: ERR: Could not get buffer\n", __FUNCNAME__);
            pcap_close(pcap);
            xpFree(pktInfo);
            return XP_ERR_MEM_ALLOC_ERROR;
        }

        recPktData = (char *)pktInfo->buf;

        /* Copy packet data*/
        memcpy(recPktData, pdata, hdr.len);

        numOfPkt = 1;
        /* Set packet size */
        pktInfo->bufSize = hdr.len;

        if ((retVal = xpsPacketDriverSendWithDSA(deviceId,
                                                 (const struct xpPacketInfo **)&pktInfo, &numOfPkt, egressPort, SYNC_TX,
                                                 xpDsaHeader_stc)) != XP_NO_ERR)
        {
            xpFree(pktInfo->buf);
            xpFree(pktInfo);
            pcap_close(pcap);
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "%s:Error: xpPacketDriverSend failed with error code = %d\n", __FUNCNAME__,
                  retVal);
            return retVal;
        }
        xpFree(pktInfo->buf);
    }

    xpFree(pktInfo);
    pcap_close(pcap);
#endif //PCAP_SUPPORT

    return retVal;
}



XP_STATUS xpsPacketDriverSendWithDSA(xpsDevice_t devId,
                                     const xpPacketInfo** const pktInfo, uint32_t *numOfPkt, uint32_t outPort,
                                     txMode sendMode, xpDsaHeader *xpDsaHeader_stc)
{
    XPS_FUNC_ENTRY_LOG();
    XP_STATUS ret = XP_NO_ERR;
    XPS_HOSTIF_OUT_PACKET_BUFFER_STC pkt = {0};
    uint32_t      pktSent = 0;
    if (sendMode == ASYNC_TX)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              " xpsPacketDriverSend failed as \
                ASYNC mode is not supported \n");
    }

    while (pktSent < *numOfPkt)
    {
        if (!pktInfo[pktSent]->buf)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "%s:\tERR: Null pointer passed \
                    as pointer to packet\n", __func__);
            ret = XP_ERR_NULL_POINTER;
            break;
        }
        pkt.pktDataPtr = (uint8_t*)pktInfo[pktSent]->buf;
        pkt.pktDataSize = pktInfo[pktSent]->bufSize;
        pkt.outPortNum =  outPort;
        pkt.txData.egressQueueNum = pktInfo[pktSent]->priority;
        ret = cpssHalHostIfCPUtoPPPacketTransmitWithDSA(devId, &pkt, xpDsaHeader_stc);
        if (ret!=XP_NO_ERR)
        {
            return ret;
        }
        pktSent += 1;
    }
    XPS_FUNC_EXIT_LOG();
    return XP_NO_ERR;

}



void xpsPacketDriverTrapIngressTraffic(uint32_t devId, uint32_t portNum,
                                       bool start)
{
    static uint32_t  count = 0;
    XP_STATUS retVal = XP_NO_ERR;
    static uint32_t tableId;
    xpAclTableInfo_t tableInfo;
    xpsAclkeyFieldList_t fieldList;
    uint32_t maskNum = 0xff;
    xpsPclAction_t action;
    xpsAclkeyField_t            *aclFieldList = NULL;
    uint32_t port;
    port = xpsGlobalPortToPortnum(devId, portNum);

    if (start)
    {

        aclFieldList = (xpsAclkeyField_t *)xpMalloc(sizeof(xpsAclkeyField_t));
        if (aclFieldList == NULL)
        {
            return;
        }

        memset(&fieldList, 0, sizeof(xpsAclkeyFieldList_t));
        memset(&tableInfo, 0x00, sizeof(xpAclTableInfo_t));
        memset(&action, 0, sizeof(xpsPclAction_t));
        memset(aclFieldList, 0x00, sizeof(xpsAclkeyField_t));
        tableInfo.vTcamInfo.clientGroup = 0;
        tableInfo.vTcamInfo.hitNumber = 0;
        tableInfo.vTcamInfo.ruleSize = XPS_VIRTUAL_TCAM_RULE_SIZE_STD_E;
        tableInfo.vTcamInfo.autoResize = true;
        tableInfo.vTcamInfo.guaranteedNumOfRules =
            52; //Max number of rules required here would be max number of ports
        tableInfo.vTcamInfo.ruleAdditionMethod =
            XPS_VIRTUAL_TCAM_RULE_ADDITION_METHOD_PRIORITY_E;
        tableInfo.vTcamInfo.ruleFormat = XPS_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E;

        fieldList.numFlds = 1;
        fieldList.isValid = 1;
        aclFieldList-> keyFlds = XP_PCL_SRC_PORT;
        aclFieldList-> mask = (uint8_t *)(&maskNum);
        aclFieldList-> value  = (uint8_t *)(&port);
        fieldList.fldList = aclFieldList;
        action.pktCmd = CPSS_PACKET_CMD_TRAP_TO_CPU_E;
        action.mirror.cpuCode = CPSS_NET_LAST_USER_DEFINED_E;

        if (! count)
        {
            retVal = xpsAclCreateTable(devId, tableInfo, &tableId);
            if (retVal != XP_NO_ERR)
            {
                free(aclFieldList);
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "Acl create table failed with err: %d ", retVal);
                return;

            }
        }
        retVal =  xpsAclWriteEntry(devId, tableId, portNum, portNum, &fieldList, action,
                                   XPS_PCL_NON_IP_KEY, false);
        if (retVal != XP_NO_ERR)
        {
            free(aclFieldList);
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Acl write entry failed with err: %d ", retVal);
            return;

        }

        count ++;
    }
    else
    {
        retVal = xpsAclDeleteEntry(devId, tableId, portNum, false);
        if (retVal != XP_NO_ERR)
        {
            free(aclFieldList);
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Acl delete entry failed with err: %d ", retVal);
            return;

        }
    }
    free(aclFieldList);
}

XP_STATUS xpsPacketDriverUpdateMacNetDevDb(uint32_t xpnetId)
{
    XP_STATUS status = XP_NO_ERR;
    xpsNetDevDbEntry* netDevEntry = NULL;
    int32_t fd = -1;
    uint8_t macAddr[6];

    status = xpsPacketDriverGetNetDevDb(XP_SCOPE_DEFAULT, xpnetId, &netDevEntry);
    if (XP_NO_ERR != status)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "xpsPacketDriverGetNetDevDb failed ifId %u\n", xpnetId);
        return status;
    }

    fd = netDevEntry->fd;
    if (fd == -1)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Invalid fd, failed\n");
        NSUNLOCK();
        return XP_ERR_INVALID_DATA;
    }
    status = cpssHalHostIfNetDevGetMacAddr(fd, macAddr);
    if (status != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "cpssHalHostIfNetDevGetMacAddr failed ifId %u\n", xpnetId);
        return status;
    }
    memcpy(netDevEntry->macAddr, macAddr, CPSS_HOSTIF_MAC_ADDR_SIZE);

    return status;
}

XP_STATUS xpsPacketDriverRxDeInit(uint32_t devId)
{
    XP_STATUS status = XP_NO_ERR;
    status  = cpssHalHostIfDmaRxThreadDeInit(devId);
    if (status != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Task deletion failed");
        return status;
    }
    return status;
}

XP_STATUS xpsPacketDrvUnRegisterRxHandler(xpsDevice_t devId)
{
    XPS_FUNC_ENTRY_LOG();

    rxPacketHandler = NULL;

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsPacketDrvRegisterRxHandler(xpsDevice_t devId,
                                           xpsPacketDrvRxHandler rxPacketDrvHandler)
{
    XPS_FUNC_ENTRY_LOG();

    rxPacketHandler = rxPacketDrvHandler;
    prevRxPacketHandler = rxPacketHandler;

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

#ifdef __cplusplus
}
#endif

