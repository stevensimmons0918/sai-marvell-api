// xpsPacketDrv.h

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
********************************************************************************/

/**
 * \file xpsPacketDrv.h
 * \brief This file contains API prototypes and type definitions
 *        for the XPS Packet Driver Manager
 */

#ifndef _XPSPACKETDRV_H_
#define _XPSPACKETDRV_H_

#include "xpTypes.h"
#include "xpEnums.h"
#include "xpsEnums.h"
#include "xpsInterface.h"
#include "openXpsPacketDrv.h"
#include "xpsReasonCodeTable.h"

#ifdef __cplusplus
extern "C" {
#endif


/*Default trapId will be mapped to default netDevice Fd*/
#define XPS_DEFAULT_TRAP_ID    0xFFFFFF00
#define XPS_NETDEV_IF_NAMESZ   16
#define XPS_GENETLINK_MCGRP_NAMESZ   16

#define SOL_NETLINK     270

typedef enum
{
    XPS_NETDEV_FALSE = 0,
    XPS_NETDEV_TRUE = 1
} netdevBoolType;

/*
 * \enum xpPktTraceMode
 * \brief Modes of packet tracing
 *
 */
typedef enum
{
    XPS_PKT_TRACE_FULL_MODE,
    XPS_PKT_TRACE_BRIEF_MODE,
    XPS_PKT_TRACE_MODE_MAX
} xpPktTraceMode;

typedef enum _xpsPacketDrvHostifVlanTagType
{
    /**
     * @brief Strip vlan tag
     * Strip vlan tag from the incoming packet
     * when delivering the packet to host interface.
     */
    XPS_PKT_DRV_HOSTIF_VLAN_TAG_STRIP,

    /**
     * @brief Keep vlan tag.
     * When incoming packet is untagged, add PVID tag to the packet when delivering
     * the packet to host interface.
     */
    XPS_PKT_DRV_HOSTIF_VLAN_TAG_KEEP,

    /**
     * @brief Keep the packet same as the incoming packet
     *
     * The packet delivered to host interface is the same as the original packet.
     * When the host interface is PORT and LAG, the packet delivered to host interface is the
     * same as the original packet seen by the PORT and LAG.
     * When the host interface is VLAN, the packet delivered to host interface will not have tag.
     */
    XPS_PKT_DRV_HOSTIF_VLAN_TAG_ORIGINAL,

} xpsPacketDrvHostifVlanTagType;

typedef struct xpDsaHeader
{
    bool recalcCrc;
    uint8_t dstInterfaceType;
    uint8_t vpt;
    uint8_t cfiBit;
    uint16_t vid;
    bool dropOnSouce;
    bool packetIsLooped;
    uint8_t tc;
    uint8_t dp;
    bool egrFilterEn;
    bool cascadeControl;
    bool egrFilterRegistered;
    uint32_t srcId;
    bool mailBoxToNeighborCPU;
    uint32_t tag0TpidIndex;
} xpDsaHeader;

/**
 * \brief _xpEthHdr_t
 *
 */
typedef struct _xpEthHdr_t
{
    macAddr_t macSA;        ///< Mac SA
    bool isSASet;        ///< SA is set or not
    macAddr_t macDA;        ///< Mac DA
    bool isDASet;        ///< DA is set or not
    uint16_t vlanId;        ///< Vlan Id
    bool isVlanIdSet;        ///< Vlan Id set or not
} xpEthHdr_t;

/*
 * \brief
 *      Capture the CPU packet into a file
 * \param [in] devId device Id. Valid values are 0-63
 * \param [in] fileName
 *
 * \return [XP_STATUS] On success XP_NO_ERR
*/
XP_STATUS xpsPacketDriverCpuCaptureStart(xpsDevice_t devId,
                                         const char* fileName);

/*
 * \brief
 *      Stops capturing the CPU packet
 * \param [in] devId device Id. Valid values are 0-63
 *
 * \return [XP_STATUS] On success XP_NO_ERR
*/
XP_STATUS xpsPacketDriverCpuCaptureStop(xpsDevice_t devId);

/*
 * \brief Initialise packet driver.
 * \param [in] devId
 *
 * \return [XP_STATUS] On success XP_NO_ERR
*/
XP_STATUS xpsPacketDriverInit(xpsDevice_t devId);

/*
 * \brief De-initialise packet driver. Calls the de-init function of
 *    DMA or Ether depending on the interface selected
 *
 * \return [XP_STATUS] On success XP_NO_ERR
*/
XP_STATUS xpsPacketDriverDeInit(xpsDevice_t devId);

/**
 * \brief Initialise packet driver for the Netdev mode
 *
 * Sets up packet driver for Netdev interface library.
 *
 * \return [XP_STATUS] On success XP_NO_ERR
 */
XP_STATUS xpsPacketDriverNetdevInit(xpsDevice_t devId);

/**
 * \brief De Initialise packet driver for the Netdev mode
 *
 * Clears Netdev interface library.
 *
 * \return [XP_STATUS] On success XP_NO_ERR
 */
XP_STATUS xpsPacketDriverNetdevDeInit(xpsDevice_t devId);

/**
 * \brief get CPU TX and RX packet counters
 *
 * \param [in] devId Device Id. Valid values are 0-63
 *
 * \return XP_STATUS On success XP_NO_ERR
 */
XP_STATUS xpsPacketDriverGetPktRxTxStatistics(xpsDevice_t devId);

/**
 * \brief set NetDev Vlan Tag Type
 *
 * \param [in] devId Device Id
 * \param [in] xpnetId
 * \param [in] tagType
 *
 * \return XP_STATUS On success XP_NO_ERR
 */
XP_STATUS xpsPacketDriverNetdevVlanTagTypeSet(xpsDevice_t devId,
                                              uint32_t xpnetId, xpsPacketDrvHostifVlanTagType tagType);

/**
 * \brief get NetDev Vlan Tag Type
 *
 * \param [in] devId Device Id
 * \param [in] xpnetId
 * \param [out] tagType
 *
 * \return XP_STATUS On success XP_NO_ERR
 */
XP_STATUS xpsPacketDriverNetdevVlanTagTypeGet(xpsDevice_t devId,
                                              uint32_t xpnetId, xpsPacketDrvHostifVlanTagType* tagType);

#if 0
/**
 * \brief set NetDev PktCmd for ARP reply packets
 *
 * \param [in] devId Device Id
 * \param [in] pktCmd
 *
 * \return XP_STATUS On success XP_NO_ERR
 */
XP_STATUS xpsPacketDriverNetDevSetArpReplyPktCmdForSwitch(xpsDevice_t devId,
                                                          xpPktCmd_e  pktCmd);
#endif

/**
 * \brief start net dev fd Tx thread
 *
 * \return XP_STATUS On success XP_NO_ERR
 */
XP_STATUS xpsPacketDriverNetDevTxThread();

/**
 * \brief stop TX thread
 *
 * \param [in] eventHandlerTid
 * \return XP_STATUS On success XP_NO_ERR
 */

XP_STATUS xpsPacketDriverNetDevTxThreadRemove(uint32_t eventHandlerTid);

/**
 * \brief send predefined packet from CPU
 *
 * \param [in] devId Device Id
 * \param [in] egressPort
 * \param [in] pktSize
 * \param [in] queueNum
 * \param [in] *pktCopies
 *
 * \return XP_STATUS On success XP_NO_ERR
 */
XP_STATUS xpSendPacket(xpDevice_t deviceId, uint32_t egressPort,
                       uint16_t pktSize, uint8_t queueNum, uint32_t *pktCopies);

/**
 * \brief send custom packet from CPU
 *
 * \param [in] devId Device Id
 * \param [in] egressPort
 * \param [in] pktSize
 * \param [in] queueNum
 * \param [in] *pktBuf
 *
 * \return XP_STATUS On success XP_NO_ERR
 */
XP_STATUS xpSendPacketCustom(xpDevice_t deviceId, uint32_t egressPort,
                       uint16_t pktSize, uint8_t queueNum, uint8_t *pktBuf);                      

/**
 * \brief send predefined packet from CPU
 *
 * \param [in] devId Device Id
 * \param [in] egressPort
 * \param [in] pktSize
 * \param [in] queue
 * \param [in] *pktCopies
 * \param [in] *xpDsaHeader_stc
 *
 * \return XP_STATUS On success XP_NO_ERR
 */

XP_STATUS xpSendPacketWithDSA(xpDevice_t deviceId, uint32_t egressPort,
                              uint16_t pktSize, uint8_t queueNum, uint32_t *pktCopies,
                              xpDsaHeader *xpDsaHeader_stc, xpEthHdr_t *ethHdr);

/**
 * \brief send pcap from CPU
 *
 * \param [in] devId Device Id
 * \param [in] egressPort
 * \param [in] queue
 * \param [in] *pcapFile
 * \param [in] *xpDsaHeader_stc
 *
 * \return XP_STATUS On success XP_NO_ERR
 */
XP_STATUS xpSendPcapWithDSA(xpDevice_t deviceId, uint32_t egressPort,
                            uint8_t queueNum, const char *pcapFile, xpDsaHeader *xpDsaHeader_stc);
/**
 * \brief Send packet on the current interface
 *
 * In DMA mode, this function will setup the descriptor from information provided in
 * buffer and will configure the DMA and trigger it to start transfer of packets
 * In Ethernet mode, this function will send buffer over Ethernet Interface.
 * There are two tx modes: SYNC and ASYNC. In the SYNC mode the system waits till
 * tx is completed. In the ASYNC mode the system moves ahead after the completion
 * of packet transmission. The customer OS is notified about Tx completion via
 * interrupt.Only ASYNC mode is used here
 *
 * \param [in] devId device Id. Valid values are 0-63
 * \param [in] **pktInfo Packet information
 * \param [in/out] numOfPkt PktCopied
 * \param [in] outPort
 * \param [in] sendMode
 * \param [in] *xpDsaHeader_stc
 * \return [XP_STATUS] On success XP_NO_ERR
 */

XP_STATUS xpsPacketDriverSendWithDSA(xpsDevice_t devId,
                                     const xpPacketInfo** const pktInfo, uint32_t *numOfPkt, uint32_t outPort,
                                     txMode sendMode, xpDsaHeader *xpDsaHeader_stc);


/**
 * \brief Capture packets at specific port in the ingress part of unit test framework.
 *
 * \param [in] devId Device Id
 * \param [in] egressPort
 * \param [in] [bool] start or stop the capture
 *
 */
void xpsPacketDriverTrapIngressTraffic(uint32_t devId, uint32_t portNum,
                                       bool start);

/**
 * \brief Update the mac address in the netdev Db
 *
 * \param [in] xpnetId
 *
 */
XP_STATUS xpsPacketDriverUpdateMacNetDevDb(uint32_t xpnetId);

XP_STATUS xpsPacketDriverRxDeInit(uint32_t devId);

/**
 * \brief Create hostIf table entry for genetlink.
 *
 * \param [in] devId     device Id. Valid values are 0-63
 * \param [in] cpuCode   hostIf table entry index cpuCode
 * \param [in] fd        genetlink socket fd
 * \param [in] familyId  genetlink family ID
 * \param [in] mcgrpName genetlink mcgroup name
 * \return [XP_STATUS] On success XP_NO_ERR
 */
XP_STATUS xpsHostIfTableGenetlinkCreate(xpsDevice_t devId, uint32_t reasonCode,
                                        int fd, uint16_t familyId, uint32_t mcgrpId);

/**
 * \brief Delete hostIf table entry for genetlink.
 *
 * \param [in] devId     device Id. Valid values are 0-63
 * \param [in] cpuCode   hostIf table entry index cpuCode
 * \return [XP_STATUS] On success XP_NO_ERR
 */
XP_STATUS xpsHostIfTableGenetlinkDelete(xpsDevice_t devId, uint32_t reasonCode);


/**
 * \brief Get family Id for genetlink by family name.
 *
 * \param [in] genetlinkName  genetlink family name
 * \param [in] fd             genetlink socket fd
 * \param [out] familyId      genetlink family Id
 * \param [out] mcgrpId       genetlink multicast group Id
 * \return [XP_STATUS] On success XP_NO_ERR
 */
XP_STATUS xpsHostIfGenetlinkGetFamilyId(uint8_t *genetlinkName,
                                        uint32_t fd, uint16_t *familyId,
                                        uint8_t *mcgrpName, uint32_t *mcgrpId);




/**
 * \brief This defines the type of function pointer that the user can register for MAC learning.
 */
typedef XP_STATUS (*xpsPacketDrvRxHandler)(xpsDevice_t devNum, uint32_t ingressPortNum,
                                      uint8_t *buff, uint32_t packetLen);

/**
 * \brief This methods allow the user to register a rx packet handler api
 *
 * \param [in] devId Device Id of device.
 * \param [in] fdbLearnHandler func where fdbLearnHandler is of type XP_STATUS (*xpsPacketDrvRxHandler)(xpsDevice_t, struct xphRxHdr*, void*, uint16_t)
 * \param [in] *userData User Provided Data
 *
 * \return XP_STATUS
 */

XP_STATUS xpsPacketDrvRegisterRxHandler(xpsDevice_t devId,
                                           xpsPacketDrvRxHandler rxPacketHandler);

XP_STATUS xpsPacketDrvUnRegisterRxHandler(xpsDevice_t devId);

#ifdef __cplusplus
}
#endif

#endif //_XPPACKETDRV_H_
