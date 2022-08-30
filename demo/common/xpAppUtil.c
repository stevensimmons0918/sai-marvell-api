// xpAppUtil.c

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
********************************************************************************/

#include "xpAppUtil.h"
#include "xpAppConfig.h"
#include "xpsLink.h"
#include "xpPyInc.h"
#include "xpAppSignalHandler.h"
#include "xpsScope.h"
#include "xpsOpenflow.h"
#include "xpsInit.h"
#include "xpsPacketDrv.h"
#include "xpsInt.h"
#include "xpsXpImports.h"
#include "xpsAcl.h"
#include <unistd.h>
#include "xpsGlobalSwitchControl.h"

#ifdef __MACH__
#include <libkern/OSByteOrder.h>
#define htobe32(x) OSSwapHostToBigInt32(x)
#endif

#ifndef MAC
#include <sys/socket.h>
#include <net/if_arp.h>
#include <linux/netlink.h>
#include <sys/ioctl.h>
#include <net/if.h>
#endif

#if 0
XP_STATUS xpAppTestIaclAddDevice(xpDevice_t devNum);
XP_STATUS xpAppTestIaclWrite(xpDevice_t devNum);
#endif

void *mplsTnlVifRoot[XP_MAX_DEVICES];
int xpAppAllocatedDeviceIds[XP_MAX_DEVICES];
static pthread_t recvTid;
extern uint8_t shellRun;
static pthread_t portTuneAndANThreadID;
static pthread_t fdbLearningThreadID;
static pthread_t intUpdateTid;
#ifdef MAC
char xpSemName[XP_SEM_NAME_LEN];
#endif  // MAC
xpSem_t *rxSemaphore[MAX_RX_QUEUES];
struct xpPacketInfo **rxPacket;
extern int xpsGetExitStatus();
extern void xpsSetExitStatus();
extern XP_STATUS xpAppCleanup();
#ifdef __cplusplus
extern "C" {
#endif
extern XP_STATUS xpAppAddDevice(xpDevice_t deviceId, xpInitType_t initType,
                                xpDevConfigStruct* config);
extern XP_STATUS xpHwAccessXpPrintCompleteMemToFile(xpDevice_t devId,
                                                    XP_PRINT_MEM_TYPE_T type, int zeros);
extern XP_STATUS xpAppDeInitDevice(xpsDevice_t devNum);

extern XP_STATUS xpAppSendPacketToXp(xpsDevice_t deviceId);

#ifdef __cplusplus
}
#endif


uint32_t tnlVif[XP_MAX_DEVICES][NUM_OF_TUNNELS];
uint32_t vlanToL3Intf[4096];
uint32_t l3TnlIntf[XP_MAX_DEVICES][NUM_OF_TUNNELS];
uint32_t tnlType[XP_MAX_DEVICES][NUM_OF_TUNNELS];
uint32_t tnlVirtualId[XP_MAX_DEVICES][NUM_OF_TUNNELS];
uint32_t pbbTnlVif[XP_MAX_DEVICES][NUM_OF_TUNNELS];
uint32_t pbbTnlIsid[XP_MAX_DEVICES][NUM_OF_TUNNELS];
uint32_t multicastVif[XP_MAX_DEVICES][XP_MULTICAST_TYPE_MAX][MAX_MULTICAST_ENTRIES];

uint64_t vlanToRif[XP_MAX_DOT1Q_VLAN];

extern xpTimeStamp_t xpTimeStamp[XP_MAX_FUNCTION];
/** Port Tune and Auto Negotiation thread information*/
static xpPortTuneAndANThreadInfo portTuneAndANThreadInfo[XP_MAX_DEVICES];
char scpuIpcMsg[MAX_IPC_MSG_LENGTH];
char fullFileName[MAX_PATH_NAME_LENGTH];
//void *mplsTnlVifRoot = NULL;


XP_STATUS xpGetPktDataAndPktLength(int fd, char **readData, uint32_t *pktLen)
{
    char readChar[2] = "";
    int idx = 0;
    uint8_t retVal = 0;
    int readBytes = -1;

    if (!pktLen || !(*readData))
    {
        PRINT_LOG("Error: Null Pointer recieved while Getting Pkt Data and Length\n");
        return XP_ERR_NULL_POINTER;
    }
    while (1)
    {
        readBytes = read(fd, readChar, 2);
        if (readBytes <= 0)
        {
            break;
        }
        if (!((*readData) + idx))
        {
            PRINT_LOG("Error: Packet size is larger than allocated buffer size in %s()\n",
                      __FUNCTION__);
            return XP_ERR_INVALID_INPUT;
        }
        retVal = sscanf(readChar, "%x", (unsigned int *)&(*readData)[idx]);
        if (!retVal)
        {
            PRINT_LOG("Error: Conversion from ascii to hex failed in %s()\n", __FUNCTION__);
            return XP_ERR_INVALID_INPUT;
        }
        idx++;
        readBytes = read(fd, readChar, 1);
        if (readBytes <= 0)
        {
            break;
        }
    }
    *pktLen = idx;
    return XP_NO_ERR;
}

//get allocated tnlvif from mplsIdx
void* getAllocatedMplsTnlVif(xpDevice_t devId, uint32_t mplsIdx)
{
    allocatedMplsTnlVifMap tmpCtx;

    tmpCtx.key = mplsIdx;
    void *r = NULL;
    r = tfind(&tmpCtx, &mplsTnlVifRoot[devId], compareMplsTnlVifMap);
    if (r)
    {
        //LOG HERE.
    }
    return r;
}

//compare function for mpls tunnel vif database
int compareMplsTnlVifMap(const void *l, const void *r)
{
    const allocatedMplsTnlVifMap *lm = (allocatedMplsTnlVifMap *)l;
    const allocatedMplsTnlVifMap *lr = (allocatedMplsTnlVifMap *)r;
    return lm->key - lr->key;
}

//User needs to take care devId is not invalid
void setBit(uint32_t devId, uint8_t *devBitMap)
{
    if (devId <= MAX_SYS_DEVICES)
    {
        uint32_t index = devId / SIZEOF_BYTE;
        uint32_t location = devId % SIZEOF_BYTE;
        devBitMap[index] |= (0x1 << location);
    }
}

//User needs to take care devId is not invalid
void clearBit(uint32_t devId, uint8_t *devBitMap)
{
    if (devId <= MAX_SYS_DEVICES)
    {
        uint32_t index = devId / SIZEOF_BYTE;
        uint32_t location = devId % SIZEOF_BYTE;

        devBitMap[index] &= ~(0x1 << location);
    }
}

int32_t isAllBitClear(uint32_t devId, uint8_t *devBitMap)
{
    if (devId > MAX_SYS_DEVICES)
    {
        return XP_ERR_INVALID_INPUT;
    }

    uint32_t index = 0;
    for (index = 0; index <= MAX_SYS_DEVICES/8 + 1; index++)
    {
        if (devBitMap[index] != 0)
        {
            return -1;
        }
    }

    return XP_NO_ERR;
}

int32_t isBitSet(uint32_t devId, uint8_t *devBitMap)
{
    if (devId > MAX_SYS_DEVICES)
    {
        return XP_ERR_INVALID_INPUT;
    }

    uint32_t index = devId / SIZEOF_BYTE;
    uint32_t location = devId % SIZEOF_BYTE;

    return (devBitMap[index] & (0x1 << location));
}

void xpAppReceiveQueuePacketHndlr(xpDevice_t devNum,
                                  xpRecvPacketInfo_t recvPacketInfo)
{
    XP_STATUS ret = XP_NO_ERR;
    ret = xpSemPost(rxSemaphore[recvPacketInfo.queueNum]);
    (void)ret;
    //PRINT_LOG("Interrupt received for deviceId - %d with QueueNum - %d, and notified to user with status -%d\n", devNum, recvPacketInfo.queueNum, ret);
}

void xpAppReceiveTrapPacketHndlr(xpDevice_t devNum,
                                 xpRecvPacketInfo_t recvPacketInfo)
{
    //xpsPacketDriverProcessRxTrapPacket(devNum, (void*)recvPacketInfo.buf, recvPacketInfo.bufSize);
    //PRINT_LOG("Interrupt received for deviceId - %d, and notified to user with status -%d\n", devNum, ret);
}

void xpAppTransmitPacketHndlr(xpDevice_t devNum)
{
    PRINT_LOG("\n************************************\n");
    PRINT_LOG("Packet Xmited to the CHIP !!!");
    PRINT_LOG("\n************************************\n");
}

XP_STATUS xpAppPacketDriverCallbackConfigurations(xpDevice_t deviceId)
{
    xpPacketInterface pktDrvIntf = XP_DMA;
    XP_STATUS status = XP_NO_ERR;

#if 0
    if ((status = xpsPacketDriverInterfaceGet(deviceId, &pktDrvIntf)) != XP_NO_ERR)
    {
        return status;
    }
#endif
    if (pktDrvIntf == XP_DMA)
    {
        /* Register call back for Rx/Tx */
        //status = xpsPacketDriverCompletionHndlrRegister(deviceId, xpAppTransmitPacketHndlr, xpAppReceiveQueuePacketHndlr, NULL);
        if (status != XP_NO_ERR)
        {
            PRINT_LOG("Error: Register handler, error code:%d\n", status);
            return status;
        }

        uint8_t queueCnt = 0;

        /* Initialize the semaphore with count as one (binary semaphore) to use in receive event handler */
        for (queueCnt = 0; queueCnt < xpAppConf.numOfRxQueue; queueCnt++)
        {
#ifndef MAC
            rxSemaphore[queueCnt] = (xpSem_t*) malloc(sizeof(xpSem_t));
            status = xpSemCreate(rxSemaphore[queueCnt], 1);
            if (status != XP_NO_ERR)
            {
                PRINT_LOG("Error: Unable to init semaphore for Rx eventHandler for queue - %d with error code - %d\n",
                          queueCnt, status);
                return status;
            }
#else
            sprintf(xpSemName, "rxSemaphore_%d", queueCnt);
            rxSemaphore[queueCnt] = xpSemCreate(xpSemName, 1);
            if (rxSemaphore[queueCnt] == SEM_FAILED)
            {
                PRINT_LOG("Error: Unable to init semaphore for Rx eventHandler for queue - %d\n",
                          queueCnt);
                return -1;
            }
#endif  // MAC
        }
    }
    else
    {
        /* Register call back for Rx/Tx */
        //status = xpsPacketDriverCompletionHndlrRegister(deviceId, xpAppTransmitPacketHndlr, xpAppReceiveTrapPacketHndlr, NULL);
        if (status != XP_NO_ERR)
        {
            PRINT_LOG("Error: Register handler, error code:%d\n", status);
            return status;
        }
    }

    return status;
}

int32_t recvTrapPacket(xpDevice_t devId, uint16_t NumOfPkt)
{
    XP_STATUS ret = XP_NO_ERR;
    uint16_t pkts_received = 0;
    uint16_t rxPktNum = 0;

    if (rxPacket == NULL)
    {
        return XP_ERR_NULL_POINTER;
    }

    /* Reset the buffer-size before fetching new packet */
    for (rxPktNum = 0; rxPktNum < NumOfPkt; rxPktNum++)
    {
        rxPacket[rxPktNum]->bufSize = XP_MAX_PACKET_SIZE;
    }

    pkts_received = NumOfPkt;
    //ret = xpsPacketDriverReceive(devId, rxPacket, &pkts_received);
    if ((ret != XP_NO_ERR) || (pkts_received != NumOfPkt))
    {
        return -1;
    }

    return 0;
}


int32_t recvQueueTrapPacket(xpDevice_t devId, uint16_t NumOfPkt)
{
    XP_STATUS ret = XP_NO_ERR;
    uint16_t pkts_received = 0;
    uint16_t rxPktNum = 0;
    uint8_t queueNum = 0;

    if (rxPacket == NULL)
    {
        return XP_ERR_NULL_POINTER;
    }

    /* Reset the buffer-size before fetching new packet */
    for (rxPktNum = 0; rxPktNum < NumOfPkt; rxPktNum++)
    {
        rxPacket[rxPktNum]->bufSize = XP_MAX_PACKET_SIZE;
    }

    while (queueNum < xpAppConf.numOfRxQueue)   //Scan for all the queues
    {
        //Note: As all the queues are getting scanned consequtively, currently non-blocking call is use
        //User can modify the scenarios if he is interested to have blocking call - xpSemWait over queue
        if (ret !=
            XP_MORE_PKTS_AVAILABLE)       //Semaphore should be checked at first before accessing any queue
        {
            if ((ret = xpSemTryWait(rxSemaphore[queueNum])) !=
                XP_NO_ERR)         //Returns zero if semaphore is set - indicating packets are avilable
            {
                queueNum++;
                continue;        //Packets are not available
            }
        }

        //Note: Currently packets are getting received with the fic count
        //However, in case of XP_MORE_PKTS_AVAILABLE user needs to take care explicitly as suites the purpose
        pkts_received = NumOfPkt;

        //Note: User has to modify the scenarios here as requirement
        //Currently all the queues are getting scanned in consecutive manner - user has to manage this calls as per queue priorities and all
        //ret = xpsPacketDriverReceiveForQueue(devId, queueNum, rxPacket, &pkts_received);
        if ((ret == XP_MORE_PKTS_AVAILABLE) || (ret == XP_NO_ERR))
        {
            // Process the received packet
            for (rxPktNum = 0; rxPktNum < pkts_received; rxPktNum++)
            {
                //xpsPacketDriverProcessRxTrapPacket(devId, rxPacket[rxPktNum]->buf, rxPacket[rxPktNum]->bufSize);
            }
        }
        else
        {
            break;        //Go out of the loop in case of error
        }

        //If more packets are not available in the same queue than currently code is written to check for all the queues
        //However, user has to handle XP_MORE_PKTS_AVAILABLE as per requirement
        if ((ret != XP_MORE_PKTS_AVAILABLE) || (ret == XP_ERR_PKT_NOT_AVAILABLE))
        {
            queueNum++;
        }
    }

    if ((ret != XP_NO_ERR) || (pkts_received != NumOfPkt))
    {
        return -1;
    }
    return 0;
}

XP_STATUS xpAppConfigDefault(xpDevice_t devId, void* arg)
{
    return xpAppDevDefaultConfigSet();
}

#if 0
XP_STATUS xpAppTestIaclAddDevice(xpDevice_t devNum)
{
    xpIaclTableProfile_t tblProfile;
    XP_STATUS retVal = XP_NO_ERR;
    tblProfile.numTables = 3;
    tblProfile.tableProfile[0].tblType = (xpIaclType_e)0;
    tblProfile.tableProfile[1].tblType = (xpIaclType_e)1;
    tblProfile.tableProfile[2].tblType = (xpIaclType_e)2;
    tblProfile.tableProfile[0].keySize = 390;
    tblProfile.tableProfile[1].keySize = 390;
    tblProfile.tableProfile[2].keySize = 390;
    tblProfile.tableProfile[0].numDb = 1;
    tblProfile.tableProfile[1].numDb = 1;
    tblProfile.tableProfile[2].numDb = 1;

    if ((retVal = xpsIaclCreateTable(devNum, tblProfile)) != XP_NO_ERR)
    {
        PRINT_LOG(" create Table returns %d :\n", retVal);
        return retVal;
    }

    PRINT_LOG("iACL : Table Created SucessFully\n");

    return retVal;
}



XP_STATUS xpAppPopulateData(xpDevice_t devId, void* arg)
{
    XP_STATUS ret = XP_NO_ERR;
#ifndef DISABLE_WM
    ret = xpAppAddDevice(devId, INIT_COLD, &devDefaultConfig);
#endif // DISABLE_WM
    return ret;
}

#endif

XP_STATUS xpAppShellInit(xpDevice_t devId, void* arg)
{
    XP_STATUS status = XP_NO_ERR;

    if ((XP_CONFIG.userCli & XP_CLI_CONFIG) &&
        (strcmp(XP_CONFIG.shellConfigFile, "None") != 0))
    {
        xpPyInvoke(0, XP_SHELL_APP, XP_CONFIG.shellConfigFile);
        XP_CONFIG.userCli &= ~(XP_CLI_CONFIG);
    }

    if ((!XP_CONFIG.userCli) && (XP_CONFIG.noConfig))
    {
        /* Normal process termination and the value of status
           & 0377 is returned to the parent */
        exit(3);
    }

    return status;
}

XP_STATUS xpAppHwValidDeviceIdsGet(xpDevice_t *devIds, uint32_t  *n)
{
    uint8_t index = 0;

    if (!devIds || !n)
    {
        return XP_ERR_NULL_POINTER;
    }

    if (xpAppConf.withHw == NULL)
    {
        *n = 1;
        devIds[0] = 0;
        return (XP_NO_ERR);
    }
    else
    {
        //PCI scan
        *n = 1;
        devIds[0] = 0;
        return (XP_NO_ERR);

    }
    if (*n == 0 && index == XP_MAX_DEVICES)
    {
        return XP_ERR_RESOURCE_NOT_AVAILABLE;
    }

    return XP_NO_ERR;
}

int xpAllocatePacketMemory(uint16_t NumOfPkt)
{
    uint16_t rxPktNum = 0;

    rxPacket = (struct xpPacketInfo **)malloc(sizeof(struct xpPacketInfo *) *
                                              NumOfPkt);
    if (!rxPacket)
    {
        PRINT_LOG("ERROR: memory not available to allocate buffer for Rx Packet\n");
        return -1;
    }

    for (rxPktNum = 0; rxPktNum < NumOfPkt; rxPktNum++)
    {
        rxPacket[rxPktNum]  = (struct xpPacketInfo *) malloc(sizeof(
                                                                 struct xpPacketInfo));
        if (!rxPacket[rxPktNum])
        {
            PRINT_LOG("ERROR: memory not available to allocate buffer for Rx Packets\n");
            return -1;
        }
        memset(rxPacket[rxPktNum], 0, sizeof(struct xpPacketInfo));
        if (xpGetSalType() != XP_SAL_KERN_TYPE)
        {
            rxPacket[rxPktNum]->buf = malloc(XP_MAX_PACKET_SIZE);
        }
        else
        {
            rxPacket[rxPktNum]->buf = malloc(XP_MAX_PACKET_SIZE);
        }
        if (!rxPacket[rxPktNum]->buf)
        {
            PRINT_LOG("ERROR: memory not available to allocate buffer for Rx packet buffer\n");
            /* Free all the allocated memories */
            while (rxPktNum--)
            {
                if (xpGetSalType() != XP_SAL_KERN_TYPE)
                {
                    free(rxPacket[rxPktNum]->buf);
                }
                else
                {
                    free(rxPacket[rxPktNum]->buf);
                }
                rxPacket[rxPktNum]->buf = NULL;
            }
            free(rxPacket);
            rxPacket = NULL;

            xpsSetExitStatus();
            return -1;
        }
        rxPacket[rxPktNum]->bufSize = XP_MAX_PACKET_SIZE;
    }

    return 0;
}

void xpDeAllocatePacketMemory(uint16_t NumOfPkt)
{
    uint16_t rxPktNum = 0;

    for (rxPktNum = 0; rxPktNum < NumOfPkt; rxPktNum++)
    {
        if (rxPacket)
        {
            if (rxPacket[rxPktNum])
            {
                if (rxPacket[rxPktNum]->buf)
                {
                    if (xpGetSalType() != XP_SAL_KERN_TYPE)
                    {
                        free(rxPacket[rxPktNum]->buf);
                    }
                    else
                    {
                        free(rxPacket[rxPktNum]->buf);
                    }
                    rxPacket[rxPktNum]->buf = NULL;
                }
                free(rxPacket[rxPktNum]);
                rxPacket[rxPktNum] = NULL;
            }
            free(rxPacket);
            rxPacket = NULL;
        }
    }
}

void * xpAppReceiveTrapPacket(void* arg)
{
    xpRecvThreadCfg *recvThrCfg = (xpRecvThreadCfg *)arg;
    uint32_t numPkts=0;
    int32_t ret;
    xpRxConfigMode configMode = POLL;
    xpPacketInterface pktDrvIntf = XP_DMA;
    uint32_t   validDevices = 0, i = 0;
    XP_STATUS  status = XP_NO_ERR;
    xpDevice_t devIds[XP_MAX_DEVICES] = { 0 };

    /* Wait for the valid devices */
    do
    {
        if (xpsGetExitStatus())
        {
            break;
        }
        status = xpAppGetValidDeviceIds(devIds, &validDevices);
        usleep(5);
    }
    while (status != XP_NO_ERR);

    /* Note: Configuration Mode and Communication Interface will remain same across all the devices */
    /* TODO: Remove dependancy of passing the devId from such packetDriver APIs */
    //xpsPacketDriverRxConfigModeGet(devIds[0], &configMode);
    //xpsPacketDriverInterfaceGet(devIds[0], &pktDrvIntf);

    if (xpAllocatePacketMemory(XP_NUM_OF_PKTS_TO_RECV) < 0)
    {
        return NULL;
    }

    if (configMode == INTR)
    {
        /* No explicit handler required if packet driver is running in interrupt mode and the interface is other than DMA */
        if (pktDrvIntf == XP_DMA)
        {
            while (1)
            {
                if ((status = xpAppGetValidDeviceIds(devIds, &validDevices)) != XP_NO_ERR)
                {
                    continue;
                }

                for (i = 0; i < validDevices; i++)
                {
                    /* User has to pass the number of packets needed to receive in a go as second argument */
                    ret = recvQueueTrapPacket(devIds[i], XP_NUM_OF_PKTS_TO_RECV);
                    if ((XP_NO_ERR == ret) && (recvThrCfg->numPktsToRecv))
                    {
                        numPkts++;
                        if (numPkts == recvThrCfg->numPktsToRecv)
                        {
                            PRINT_LOG("RxThread: Received %u num of packets, exiting\n", numPkts);
                            break;
                        }
                    }
                }

                usleep(30);
                if (xpsGetExitStatus())
                {
                    break;
                }
            }
        }
    }
    else
    {
        while (1)
        {

            if (xpsGetExitStatus())
            {
                break;
            }

            if ((status = xpAppGetValidDeviceIds(devIds, &validDevices)) != XP_NO_ERR)
            {
                continue;
            }

            for (i = 0; i < validDevices; i++)
            {
                /* User has to pass the number of packets needed to receive in a go as second argument */
                ret = recvTrapPacket(devIds[i], XP_NUM_OF_PKTS_TO_RECV);
                if ((XP_NO_ERR == ret) && (recvThrCfg->numPktsToRecv))
                {
                    numPkts++;
                    if (numPkts == recvThrCfg->numPktsToRecv)
                    {
                        PRINT_LOG("RxThread: Received %u num of packets, exiting\n", numPkts);
                        break;
                    }
                }
            }

            usleep(30);
        }
    }

    xpDeAllocatePacketMemory(XP_NUM_OF_PKTS_TO_RECV);

    xpsSetExitStatus();
    return (NULL);
}

XP_STATUS xpAppDevDefaultConfigSet()
{
    if (xpAppConf.withHw == NULL)
    {
        devDefaultConfig.dalType = XP_SHADOW_REMOTEWM;
    }
    devDefaultConfig.staticTablesNeeded = 1;
    devDefaultConfig.portInitAllowed = xpAppConf.portInitAllowed;
    devDefaultConfig.initialPortVifVal = xpAppConf.initialPortVifVal;
    return XP_NO_ERR;
}

XP_STATUS xpAppSchedPolicyInit(xpDevice_t devId)
{
#define XP_APP_QUEUES_NUM 64
#define XP_APP_QUEUES_WEIGHT 10

    uint32_t          queueNum = 0;
    rxQueueWeightInfo weights[XP_APP_QUEUES_NUM];

    memset(weights, 0, sizeof(weights));

    /* Set scheduling policy */
    PRINT_LOG("XP_CONFIG.schedPolicy: %s\n", XP_CONFIG.schedPolicy);

    if (0 != strcmp(XP_CONFIG.schedPolicy, "RR"))
    {
        if (0 == strcmp(XP_CONFIG.schedPolicy, "SP"))
        {
#if 0
            PRINT_LOG("Setting sched policy to SP\n");
            if ((status = xpsPacketDriverSetSchedPolicy(devId, SP, NULL, 0)) != XP_NO_ERR)
            {
                PRINT_LOG("xpsPacketDriverSetSchedPolicy failed. RC: %u\n", status);
                return XP_ERR_INIT_FAILED;
            }
#endif
        }
        else if (0 == strcmp(XP_CONFIG.schedPolicy, "WRR"))
        {
            for (queueNum = 0; queueNum < XP_APP_QUEUES_NUM; queueNum++)
            {
                weights[queueNum].queueNum = queueNum;
                weights[queueNum].weight = ((queueNum + 1) * XP_APP_QUEUES_WEIGHT);
            }

            PRINT_LOG("Setting sched policy to WRR\n");
#if 0
            if ((status = xpsPacketDriverSetSchedPolicy(devId, WRR, weights,
                                                        XP_APP_QUEUES_NUM)) != XP_NO_ERR)
            {
                PRINT_LOG("xpsPacketDriverSetSchedPolicy failed. RC: %u\n", status);
                return XP_ERR_INIT_FAILED;
            }
#endif
        }
    }

    return XP_NO_ERR;
}

XP_STATUS xpAppHostInterfaceDemo(xpDevice_t devId)
{
    XP_STATUS rc = XP_NO_ERR;
#ifndef MAC
    uint32_t intfId = 4; // Create netdev for port#0
    uint32_t knetId = intfId;
    char intfName[IFNAMSIZ] = "et0_0"; //Name of netdev
    struct ifreq ifr;
    uint8_t switchMac[6] = {0xF2, 0xF2, 0xF2, 0xF2, 0xF2, 0xF3};
    int sock;
    //uint32_t dst_vif = 4; //vif of port#0

    rc = xpsNetdevXpnetIdAllocateWith(devId, knetId);
    if (XP_NO_ERR != rc)
    {
        printf("Unable to allocate knetId for interface: %u, %s\n", intfId, intfName);
        return rc;
    }

    rc = xpsNetdevIfCreate(devId, knetId, intfName);
    if (XP_NO_ERR != rc)
    {
        printf("Unable to create interface: %u, %s\n", intfId, intfName);
        return rc;
    }

    //rc = xpsNetdevIfLinkSet(devId, knetId, intfId, true);
    if (XP_NO_ERR != rc)
    {
        printf("Unable to link interface: %u, %s\n", intfId, intfName);
    }

    /* For reason code 0 set Rx channel as netdev */
    rc = xpsNetdevTrapSet(devId, 0, 0, 2, 0, true);
    if (XP_NO_ERR != rc)
    {
        printf("Unable to set rx channel as netdev for reasoncode 0 \n");
    }

    /* For reason code 103 set Rx channel as netdev */
    rc = xpsNetdevTrapSet(devId, 1, 103, 2, 0, true);
    if (XP_NO_ERR != rc)
    {
        printf("Unable to set rx channel as netdev for reasoncode 103 \n");
    }

    /* For reason code same as intfId, set Rx channel as netdev */
    rc = xpsNetdevTrapSet(devId, 3, intfId, 2, 0, true);
    if (XP_NO_ERR != rc)
    {
        printf("Unable to set rx channel as netdev for reasoncode 104 \n");
    }

    /* For reason code 206 set Rx channel as netdev */
    rc = xpsNetdevTrapSet(devId, 2, 206, 2, 0, true);
    if (XP_NO_ERR != rc)
    {
        printf("Unable to set rx channel as netdev for reasoncode 206 \n");
    }


    //rc = xpsNetdevIfTxHeaderSet(0, knetId, dst_vif, true);
    if (XP_NO_ERR != rc)
    {
        printf("Unable to set tx header \n");
    }

    strncpy(ifr.ifr_ifrn.ifrn_name, (const char*) intfName,
            sizeof(ifr.ifr_ifrn.ifrn_name));
    sock = socket(AF_INET, SOCK_DGRAM, 0);

    if (sock == -1)
    {
        printf("fail to create socket. errno=%d, %s\n", errno, strerror(errno));
        return  XP_ERR_SOCKET_INIT;
    }

    ifr.ifr_hwaddr.sa_family = ARPHRD_ETHER;
    memcpy(ifr.ifr_hwaddr.sa_data, switchMac, 6);

    if (ioctl(sock, SIOCSIFHWADDR, &ifr) < 0)
    {
        printf("fail to set interface mac Addr for interface %s. errno=%d %s\n",
               ifr.ifr_ifrn.ifrn_name, errno, strerror(errno));
        close(sock);
        return XP_ERR_IOCTL;
    }

    close(sock);
#endif
    return rc;
}

XP_STATUS xpAppNewDevicesInit(xpInitType_t initType)
{
    XP_STATUS status = XP_NO_ERR;
    uint32_t   validDevices = 0, i = 0;
    xpDevice_t devIds[XP_MAX_DEVICES] = { 0 };

    xpAppDevDefaultConfigSet();

    if ((status = xpAppHwValidDeviceIdsGet(devIds, &validDevices)) != XP_NO_ERR)
    {
        return status;
    }

    for (i = 0; i < validDevices ; i++)
    {
        if ((status = xpAppAddDevice(devIds[i], initType,
                                     &devDefaultConfig)) != XP_NO_ERR)
        {
            PRINT_LOG("Adding device %d failed with error code: %u\n", devIds[i], status);
            return status;
        }
        xpAppSetDeviceValid(devIds[i]);
        if (xpAppConf.createdemoports)
        {
            xpAppHostInterfaceDemo(devIds[i]);
        }
    }

    if ((XP_CONFIG.userCli & XP_CLI_CONFIG) &&
        (strcmp(XP_CONFIG.shellConfigFile, "None") != 0))
    {
        xpPyInvoke(0, XP_SHELL_APP, XP_CONFIG.shellConfigFile);
        XP_CONFIG.userCli &= ~(XP_CLI_CONFIG);
    }

    if ((!XP_CONFIG.userCli) && (XP_CONFIG.noConfig))
    {
        /* Normal process termination and the value of status
           & 0377 is returned to the parent */
        exit(3);
    }

    return status;
}


static void xpAppCommonCleanup(void)
{
    XP_STATUS  status = XP_NO_ERR;
    xpDevice_t devIds[XP_MAX_DEVICES] = { 0 };
    uint32_t validDevices = 0;

    if (xpsGetExitStatus() <= 0)
    {
        xpsSetExitStatus();
    }
    while (xpsGetExitStatus() < 2)
    {
        usleep(100);
    }
    sleep(1);

    //Reset interrupt manager states to avoid interruption during exist()
    //This won't create an exception in case signal handling goes bad
    status = xpAppGetValidDeviceIds(devIds, &validDevices);
    if (status != XP_NO_ERR)
    {
        return;
    }

    xpAppCleanup();

    uint8_t queueCnt = 0;
    /* Deinitialize the semaphore used for receive event handler */
    for (queueCnt = 0; queueCnt < xpAppConf.numOfRxQueue; queueCnt++)
    {
        if (rxSemaphore[queueCnt])
        {
#ifndef MAC
            xpSemDestroy(rxSemaphore[queueCnt]);
            free(rxSemaphore[queueCnt]);
#endif
        }
    }
}

XP_STATUS xpAppMainLoop(xpInitType_t initType)
{
    XP_STATUS status = XP_NO_ERR;
    xpRecvThreadCfg recvThrdCfg;
    /* Register handler to receive events from driver
     * This method will fall via SAL and for current case (i.e. linux)
     * it will register our PID with driver to send us(this process) events
     * and also setup a signal handler
     */
    recvThrdCfg.numPktsToRecv = XP_CONFIG.numOfPktsRecv;

    registerDmaInterruptHndlr();

    //start xpAppReceiveTrapPacket in thread
    //No need to terminate externally as it will get terminated itself by following reasons
    //1. The entire process is terminated due to making a call to either the exec() or exit()
    //or
    //2. If main() finishes first
    int err = 0;
    err = xpSalThreadCreate(&recvTid, NULL, &xpAppReceiveTrapPacket, &recvThrdCfg);
    if (err)
    {
        PRINT_LOG("ERROR; return code from xpSalThreadCreate() is %d\n", err);
        return status;
    }

    /* for multiple xpShells you need to pass -U option */
    if (XP_CONFIG.userCli & XP_CLI_MULTISHELL)
    {
        xpPyCliRequests(0);
    }
    GET_TIME_STAMP(xpTimeStamp[XP_APP_NEW_DEVICE_INIT].startTime);

    if ((status = xpAppNewDevicesInit(xpAppConf.initType)) != XP_NO_ERR)
    {
        PRINT_LOG("Cannot add new devices. RC: %u\n", status);
        return status;
    }

    GET_TIME_STAMP(xpTimeStamp[XP_APP_NEW_DEVICE_INIT].endTime);

    GET_TIME_STAMP(xpTimeStamp[XP_BOOTUP_ALL].endTime);
    /*
       PRINT_LOG("Total BootUp Time : %f seconds.\n",
       (float)(xpTimeStamp[XP_BOOTUP_ALL].endTime -
       xpTimeStamp[XP_BOOTUP_ALL].startTime)/1000000);
       */
    /* Restart the xpShell if needed */
    while (1)
    {
        if (shellRun == XP_SHELL_EXIT_2)
        {
            XP_CONFIG.userCli = 0;
            shellRun = XP_SHELL_EXIT_0;
        }
        if (shellRun == XP_SHELL_EXIT_5)
        {
            XP_CONFIG.userCli = XP_CLI_USER;
            shellRun = XP_SHELL_EXIT_0;
        }
        if ((XP_CONFIG.userCli & XP_CLI_USER) && (!(XP_CONFIG.userCli & XP_CLI_CONFIG)))
        {
            xpPyInvoke(1, XP_SHELL_APP, NULL);
        }

        if (xpsGetExitStatus())
        {
            break;
        }
        if (!(XP_CONFIG.userCli & XP_CLI_USER))
        {
            break;
        }

        usleep(100);
    }

    if (xpsGetExitStatus())
    {
        xpAppCommonCleanup();
    }
    return status;
}

XP_STATUS xpAppGetExecutableFileDir(const char *executableFileName,
                                    char *executableFileDir, unsigned long int maxLen)
{
    /* Get executable name */
    char exeId[MAX_FILENAME_LEN] = {0};
    char symLink[MAX_FILENAME_LEN] = {0};
    char *tmp = NULL;
    ssize_t len =0;

    if ((executableFileDir == NULL) || (maxLen == 0))
    {
        return XP_ERR_NULL_POINTER;
    }

    /* If execute file name is empty then get current pid execute file name */
    if (executableFileName == NULL)
    {
        snprintf(exeId, MAX_FILENAME_LEN, "/proc/%d/exe", getpid());
        if ((len = readlink(exeId, executableFileDir, maxLen)) < 0)
        {
            PRINT_LOG("Error: readlink failed. exeId:%s, error:%s\n", exeId,
                      strerror(errno));
        }
        executableFileDir[len] = '\0';
    }
    else
    {
        /* Copy programName in temp var, and make sure it's null terminated */
        strncpy(executableFileDir, executableFileName, maxLen - 1);
        executableFileDir[maxLen-1] = '\0';
    }

    /* Replace symlink */
    if ((len = readlink(executableFileDir, symLink, MAX_FILENAME_LEN)) > 0)
    {
        symLink[len] = '\0';
        /* Remove binaryname from programname */
        tmp = strrchr(executableFileDir, '/');
        if (tmp)
        {
            *(tmp+1) = '\0';
        }
        /* Append sym link */
        strncat(executableFileDir, symLink, maxLen - strlen(executableFileDir) - 1);
    }

    /* Remove binaryname from programname */
    tmp = strrchr(executableFileDir, '/');
    if (tmp)
    {
        *(tmp+1) = '\0';
    }

    return XP_NO_ERR;
}

XP_STATUS xpAppSafeMemoryFree(void **pptr)
{
    if (NULL == pptr || NULL == *pptr)
    {
        return XP_ERR_NULL_POINTER;
    }

    free(*pptr);
    *pptr=NULL;
    return XP_NO_ERR;
}

void twalkFreeTreeNode(const void *nodep, const VISIT which, const int depth)
{
    if (NULL == nodep)
    {
        return;
    }

    switch (which)
    {
        case preorder:
            break;
        case postorder:
            break;
        case endorder:
            printf("twalk free 1---------------------->%p\n", (void**)&(*(void**)nodep));
            xpAppSafeMemoryFree((void**)&(*(void**)nodep));
            xpAppSafeMemoryFree((void**)&nodep);
            break;
        case leaf:
            printf("twalk free 3---------------------->%p\n", (void**)&(*(void**)nodep));
            xpAppSafeMemoryFree((void**)&(*(void**)nodep));
            xpAppSafeMemoryFree((void**)&nodep);
            break;
    }
}

XP_STATUS xpAppTDestroy(void **root)
{
    if (NULL == root || NULL == *root)
    {
        return XP_ERR_NULL_POINTER;
    }

    twalk(*root, twalkFreeTreeNode);
    *root = NULL;

    return XP_NO_ERR;
}



XP_STATUS xpAppGetValidDeviceIds(xpDevice_t* devIds, unsigned int* n)
{
    XP_STATUS status = XP_NO_ERR;
    xpDevice_t devId;

    *n = 0; /* in case there is an error/null pointer, we should return number of valid deviceIds as 0 */

    if (!devIds)
    {
        PRINT_LOG("devIds array was not allocated\n");
        return XP_ERR_NULL_POINTER;
    }
    for (devId = 0; devId < XP_MAX_DEVICES; ++devId)
    {
        if (xpAppAllocatedDeviceIds[devId] != 0)
        {
            devIds[*n] = devId;
            *n = (*n) + 1;
        }
    }

    if (!(*n))
    {
        status = XP_ERR_NOT_FOUND;
    }

    return status;
}

XP_STATUS xpAppSetDeviceValid(xpDevice_t devId)
{
    if (!xpAppAllocatedDeviceIds[devId])
    {
        xpAppAllocatedDeviceIds[devId] = 1;
        PRINT_LOG("Device ID %d is set as a valid device\n", (int) devId);
    }
    else
    {
        PRINT_LOG("DeviceId %d is already set as valid\n", (int)devId);
    }

    return XP_NO_ERR;
}

XP_STATUS xpAppPortTuneAndANThreadSleepIntervalGet(xpDevice_t deviceId,
                                                   uint32_t *value)
{
    // Check for application runs for HW.
    if (NULL == XP_CONFIG.withHw)
    {
        PRINT_LOG("This option is invalid for WM platform. Please check help.\n");
        return XP_ERR_INVALID_INPUT;
    }

    if (deviceId < XP_MAX_DEVICES)
    {
        *value = portTuneAndANThreadInfo[deviceId].sleepInterval;
        return XP_NO_ERR;
    }
    return XP_ERR_INVALID_INPUT;
}

XP_STATUS xpAppPortTuneAndANThreadSleepIntervalSet(xpDevice_t deviceId,
                                                   uint32_t value)
{
    // Check for application runs for HW.
    if (NULL == XP_CONFIG.withHw)
    {
        PRINT_LOG("This option is invalid for WM platform. Please check help.\n");
        return XP_ERR_INVALID_INPUT;
    }

    if (deviceId < XP_MAX_DEVICES)
    {
        portTuneAndANThreadInfo[deviceId].sleepInterval = value;
        return XP_NO_ERR;
    }
    return XP_ERR_INVALID_INPUT;
}

int xpAppGetExitStatus(void)
{
    return xpsGetExitStatus();
}

void xpAppSetExitStatus(void)
{
    xpsSetExitStatus();
}

void* xpAppPortTuneAndANThreadHndlr(void *args)
{
    xpDevice_t deviceId = 0;
    xpsPortList_t portsList;
    uint16_t count = 0, maxPortNum = 0;
    XP_STATUS ret = XP_NO_ERR;

    deviceId = (xpDevice_t)((size_t)args);

    if (deviceId >= XP_MAX_DEVICES)
    {
        PRINT_LOG("Error: Invalid device ID:%d\n", deviceId);
        return NULL;
    }

    PRINT_LOG("Created Port Tune and Auto Negotiation and AN thread\n");

    /* Register Port Tune and Auto Negotiation thread into logger with initially log disabled */
    xpsSdkThreadLogRegister((char*) "MgmtSdk", XP_LOG_PORT_TUNE_AND_AN_THREAD_BLOCK,
                            0);
    xpAppPortTuneAndANThreadSleepIntervalSet(deviceId,
                                             PORT_TUNE_AND_AN_THREAD_INTERVAL);

    memset(&portsList, 0, sizeof(portsList));

    /* Set port list to auto tune*/
    portsList.size = 0;
    ret = xpsGlobalSwitchControlGetMaxNumPhysicalPorts(deviceId, &maxPortNum);
    if (ret != XP_NO_ERR)
    {
        PRINT_LOG("xpsGlobalSwitchControlGetMaxNumPhysicalPorts get failed with error : %d",
                  ret);
        return NULL;
    }
    for (count = 0; count < maxPortNum; count++)
    {
        /* Setting all ports to auto tune */
        portsList.portList[count] = count;
        portsList.size++;
        if (XP_CONFIG.portTuneStatus)
        {
            xpsLinkManagerPortAutoTuneEnable(deviceId, count, 1);
        }
    }
    while (1)
    {

        if (xpsGetExitStatus())
        {
            break;
        }
        xpsLinkManagerPortANOrAutoTunePerform(deviceId, &portsList);
        if (xpsGetExitStatus())
        {
            break;
        }
        usleep(portTuneAndANThreadInfo[deviceId].sleepInterval);
    }
    return NULL;
}
XP_STATUS xpAppPortTuneAndANThreadCreate(xpDevice_t deviceId)
{
    if (XP_CONFIG.withHw != NULL && !XP_CONFIG.diag &&
        xpGetSalType() != XP_SAL_KERN_TYPE)
    {
        PRINT_LOG("Creating Port Tune and Auto Negotiation thread\n");
        if (xpSalThreadCreate(&portTuneAndANThreadID, NULL,
                              &xpAppPortTuneAndANThreadHndlr, (void*)((size_t)deviceId)))
        {
            PRINT_LOG("xpSalThreadCreate Failed...\n");
            return XP_ERR_RESOURCE_NOT_AVAILABLE;
        }
        usleep(50);
    }
    else
    {
        PRINT_LOG("Port Tune and Auto Negotiation thread is Disable\n");
    }

    return XP_NO_ERR;
}

void *xpAppIntTableUpdateThread(void* args)
{
    (void)(args);
    uint32_t validDevices = 0;
    XP_STATUS  status = XP_NO_ERR;
    xpDevice_t devIds[XP_MAX_DEVICES] = { 0 };

    /* Wait for the valid devices */
    do
    {
        status = xpAppGetValidDeviceIds(devIds, &validDevices);
        usleep(5);
    }
    while (status != XP_NO_ERR);

    while (1)
    {
        if (xpsGetExitStatus())
        {
            break;
        }
        status = xpsIntUpdateMetadata(devIds[0]);
        if (status != XP_NO_ERR)
        {
            PRINT_LOG("Error - Updating Metadata with INT Thread\n");
            break;
        }
    }
    return NULL;
}

XP_STATUS xpAppIntThreadCreate(xpDevice_t deviceId)
{
    PRINT_LOG("Creating INT table update thread...\n");
    if (xpSalThreadCreate(&intUpdateTid, NULL, &xpAppIntTableUpdateThread, NULL))
    {
        PRINT_LOG("xpSalThreadCreate Failed...\n");
        return XP_ERR_RESOURCE_NOT_AVAILABLE;
    }
    usleep(50);

    return XP_NO_ERR;
}

void* xpAppFdbLearnigThreadHndlr(void *arg)
{
    xpDevice_t deviceId = (xpDevice_t)((size_t)arg);
    while (1)
    {
        if (xpsGetExitStatus())
        {
            break;
        }
        //xpsPacketDriverProcessL2LearnPacket(deviceId);
    }

    //xpsPacketDriverL2PktLearningDeinit(deviceId);

    return ((void*)((size_t)deviceId));
}

XP_STATUS xpAppFdbL2LearningThreadCreate(xpDevice_t deviceId)
{

    if (XP_CONFIG.withHw != NULL && !XP_CONFIG.diag &&
        (xpGetSalType() != XP_SAL_KERN_TYPE))
    {
        //xpsPacketDriverL2PktLearningInit(deviceId);
        //xpsPacketDriverL2PacketLearningStatusSet(deviceId, XP_CONFIG.learnThreadStatus);
        PRINT_LOG("Creating FDB L2 Learning thread\n");
        if (xpSalThreadCreate(&fdbLearningThreadID, NULL, &xpAppFdbLearnigThreadHndlr,
                              (void*)((size_t)deviceId)))
        {
            PRINT_LOG("xpSalThreadCreate Failed...\n");
            //xpsPacketDriverL2PktLearningDeinit(deviceId);
            return XP_ERR_RESOURCE_NOT_AVAILABLE;
        }

        usleep(50);
    }
    else
    {
        PRINT_LOG("FDB L2 Learning thread is Disable\n");
    }

    return XP_NO_ERR;
}
