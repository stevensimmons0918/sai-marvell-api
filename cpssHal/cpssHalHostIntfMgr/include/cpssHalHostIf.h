/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
*******************************************************************************/

/**
********************************************************************************
* @file cpssHalHostIf.h
*
* @brief Include cpssHalHostIf for PCIe SDMA based control packet handling
*
*
* @version   1
********************************************************************************
*/

#ifndef _cpssHalHostIf_h_
#define _cpssHalHostIf_h_

#ifdef __cplusplus
extern "C" {
#endif

#include "xpTypes.h"
#include "xpsEnums.h"
//#include <extUtils/osNetworkStack/osNetworkStack.h>
#include <extUtils/rxEventHandler/rxEventHandler.h>
#include "cpss/generic/cpssTypes.h"
#include "cpss/generic/events/cpssGenEventUnifyTypes.h"
#include "cpss/generic/events/cpssGenEventRequests.h"
#include "cpss/dxCh/dxChxGen/networkIf/cpssDxChNetIf.h"
#include "cpss/dxCh/dxChxGen/networkIf/cpssDxChNetIfTypes.h"
#include "cpss/dxCh/dxChxGen/cpssHwInit/cpssDxChHwInit.h"
#include "extUtils/rxEventHandler/rxEventHandler.h"
#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwTables.h>
#include <gtOs/gtOsMem.h>
#include "cpssHalUtilOsNetworkStack.h"
#include "stdio.h"
#include "cpssHalDevice.h"
#include "xpsPacketDrv.h"
#include "cpssDxChPortCtrl.h"

#define NUMBER_OF_TX_BUFFER 1
#define NUMBER_OF_RX_BUFFER 6
#define MAX_TX_SDMA_BUFFER_SIZE (10240+64) //For dma memory allocation
#define DMA_BUFFER_OFFSET   32
#define MIN_DMA_BUFF_PKT_LEN 60
#define PKTDUMP_MAX_BYTES       64
#define PKTDUMP_BYTES_PER_LINE  16
#define MAC_FCS_LENGTH_CNS      4
#define EV_HANDLER_MAX_PRIO 200
#define CPSS_HOSTIF_DEFAULT_TIMEOUT_VAL 0
#define CPSS_HOSTIF_IMAGINARY_SOURCE_DEV 10

/* Traffic from CPU have Strict Priority over regular traffic */
#define FROM_CPU_QUEUE_NUM 7
#define CPSS_HOSTIF_MAX_READ_DATA_SIZE CPSS_DXCH_PORT_MAX_MRU_CNS

#define XPS_DESIGNATED_DEV_NUM_INDEX  1
#define XPS_NUM_OF_ASICS_IN_B2B 2
#define CPSS_HOSTIF_MAC_ADDR_SIZE 6

typedef struct
{
    GT_UINTPTR          evHndl;
    GT_U32              hndlrIndex;
} EV_HNDLR_PARAM;

typedef struct
{
    uint8_t  egressQueueNum;             /* egress port queue to transmit */
    uint8_t  tagVpt;                      /* tag priority field */
    uint32_t tagVlan;                    /* tag vlan id */
} CPSS_HAL_HOSTIF_TX_DATA_T;

/*In Packet buffer */
/* PP to CPU*/
typedef struct
{
    CPSS_NET_RX_CPU_CODE_ENT  reason;
    uint32_t                  ingressPortNum;
    uint32_t                  hostPktDataSize;
    uint8_t                   *hostPktDataPtr;
    uint16_t                  vlanId;

} XPS_HOSTIF_IN_PACKET_BUFFER_STC;

/*Out Packet buffer */
/*CPU to PP*/
typedef struct
{
    uint32_t                  outPortNum;
    CPSS_HAL_HOSTIF_TX_DATA_T txData;
    uint32_t                  pktDataSize;
    uint8_t                   *pktDataPtr;
} XPS_HOSTIF_OUT_PACKET_BUFFER_STC;

/*Traffic class*/
typedef enum
{
    TC_PRIORITY_0 = 0,
    TC_PRIORITY_1,
    TC_PRIORITY_2,
    TC_PRIORITY_3,
    TC_PRIORITY_4,
    TC_PRIORITY_5,
    TC_PRIORITY_6,
    TC_PRIORITY_7
} XPS_HOSTIF_TRAFFIC_CLASS;

/**
 * @enum RX_EV_HANDLER_EV_TYPE_ENT
 *
 * @brief Event type
*/
typedef enum
{

    /** CPSS_PP_RX_BUFFER_QUEUE_E events */
    XPS_HOSTIF_HANDLER_TYPE_RX_E,

    /** CPSS_PP_RX_ERR_QUEUE_E events */
    XPS_HOSTIF_HANDLER_TYPE_RX_ERR_E,

    /** any type of events */
    XPS_HOSTIF_HANDLER_TYPE_ANY_E

} XPS_HOSTIF_RX_EV_HANDLER_TYPE_ENT;

/**
 * \brief Initialize CPU Rx Tx .
 *
 * \param [in] devId
 *
 * \return XP_STATUS
*/
XP_STATUS cpssHalHostIfRxTxInit
(
    xpsDevice_t devId
);

/**
 * \brief Initialize the CPU Rx cause event binding mechanism
 *
 * \param [in] isPollingEnabled
 *
 * \return XP_STATUS
*/
XP_STATUS cpssHalHostIfEventRequest
(
    bool      isPollingEnabled,
    GT_U8     deviceNumber
);

/**
 * \brief Clear all CPU Rx cause events
 *
 *
 * \return XP_STATUS
*/
XP_STATUS cpssHalHostIfRxEventClear
(
);

/**
 * \brief This routine is the creates thread for CPU packets Rx
 * \      events . This uses  Event-Request-Driven mode(polling mode).
 *
 * \param [in] timeout
 *
 * \return XP_STATUS
 */
XP_STATUS cpssHalHostIfDmaRxThread
(
    uint32_t timeout
);

/**
 * \brief  get the SDMA Rx counters
 *
 * \param [in] devNum
 *
 * \return XP_STATUS
*/
XP_STATUS cpssHalHostIfRxPacketShow
(
    uint8_t                         devNum,
    uint8_t                         queueIdx,
    uint32_t                        numOfBuff,
    uint8_t                         **packetBuffs,
    uint32_t                        *buffLenArr,
    CPSS_DXCH_NET_RX_PARAMS_STC     *rxParamsPtr
);

/**
 * \brief API for CPU to PP packet forward (DSA info as FWD) packet
 *
 * \param [in] devId
 * \param [in] *packetPointer
 *
 * \return XP_STATUS
*/
XP_STATUS cpssHalHostIfCPUtoPPPacketForward
(
    uint32_t devId,
    XPS_HOSTIF_OUT_PACKET_BUFFER_STC *pktPtr
);

/**
 * \brief API for CPU to PP packet transmision (DSA info as FROM_CPU)
 * \      with syncronized mode.
 *
 * \param [in] devId
 * \param [in] *packetPointer
 *
 * \return XP_STATUS
*/
XP_STATUS cpssHalHostIfCPUtoPPPacketTransmit
(
    uint32_t devId,
    XPS_HOSTIF_OUT_PACKET_BUFFER_STC *packetPointer
);

/**
 * \brief Recieves the Rx event and sends it out for
 * \      further processing.This method is a task for selecting
 * \      and recieving events once it occurs.
 *
 * \param [in] timeout
 * \param [out] pktPtr
 *
 * \return XP_STATUS
 */
XP_STATUS cpssHalHostIfPPtoCPUPacketReceive
(
    uint32_t timeout,
    XPS_HOSTIF_IN_PACKET_BUFFER_STC *packetPointer
);


/**
 * \brief display SDMA Rx Counters
 *
 * \param [in] deviceNumber
 *
 * \return XP_STATUS
 */
XP_STATUS cpssHalHostIfSdmaRxCountersDisplay
(
    uint8_t deviceNumber
);

/**
 * \brief allows to 'see' the packets that are received at
 *        CPU get (with the DSA)
 *
 * \param [in] enable
 *
 * \return XP_STATUS
 */
XP_STATUS cpssHalHostIfCPURxPacketDumpEnable
(
    GT_BOOL enable
);

/**
 * \brief Init net dev library
 *
 * \param [in] devId
 * \param [in] rxHandlerCb
 *
 * \return XP_STATUS
 */
XP_STATUS cpssHalHostIfNetDevInit
(
    GT_U8 devId,
    RX_EV_PACKET_RECEIVE_CB_FUN    rxHandlerCb
);

/**
 * \brief DeInit net dev library
 *
 * \param [in] devId
 * \param [in] rxHandlerCb
 *
 * \return XP_STATUS
 */
XP_STATUS cpssHalHostIfNetDevDeInit
(
    GT_U8 devId,
    RX_EV_PACKET_RECEIVE_CB_FUN    rxHandlerCb
);

/**
 * \brief create/open netdevice
 *
 * \param [in] devId
 * \param [in] netDevName
 * \param [out] fd
 *
 * \return XP_STATUS
 */
XP_STATUS cpssHalHostIfNetDevCreate
(
    GT_U8         devId,
    GT_U8         *netDevName,
    GT_32        *fd
);

/**
 * \brief destroy/close netdevice
 *
 * \param [in] devId
 * \param [in] fd
 *
 * \return XP_STATUS
 */
XP_STATUS cpssHalHostIfNetDevDestroy
(
    GT_U8          devId,
    GT_32         fd
);

/**
 * \brief write to Net Device
 *
 * \param [in] tnFd
 * \param [in] pktBuf
 * \param [in] pktLen
 *
 * \return XP_STATUS
 */
XP_STATUS cpssHalHostIfNetDevWrite
(
    int32_t tnFd,
    GT_U8   *pktBuf,
    GT_U32  pktLen
);

/**
 * \brief Read from Net device
 *        CPU get (with the DSA)
 *
 * \param [in] tnFd
 * \param [in] *pktBuf
 * \param [inout] *pktLen
 *
 * \return XP_STATUS
 */
XP_STATUS cpssHalHostIfNetDevRead
(
    int32_t tnFd,
    GT_U8   *pktBuf,
    INOUT GT_U32  *pktLen
);

/**
 * \brief configure Net device
 *
 * \param [in] config
 * \param [in] tnName
 *
 * \return XP_STATUS
 */
XP_STATUS cpssHalHostIfNetDevConfig
(
    const char *config,
    char* tnName
);

/**
 * \brief Api to get netdev mac addr
 *
 * \param [in] tnFd
 * \param [in] etherPtr
 *
 * \return XP_STATUS
 */
XP_STATUS cpssHalHostIfNetDevGetMacAddr(
    int32_t tnFd,
    uint8_t macAddr_t[]
);

/**
 * \brief Api to set fdb entry for netdev mac addr
 *
 * \param [in] devId
 * \param [in] fd
 * \param [in] vlanId
 * \param [in] macAddr_t[]
 * \param [in] cmd
 *
 * \return XP_STATUS
 */
XP_STATUS cpssHalHostIfSetPktCmdFdbEntry
(
    uint32_t devId,
    uint16_t vlanId,
    uint8_t macAddr_t[],
    xpPktCmd_e cmd
);

/**
 * \brief debug function to allow print (dump) of the Rx Packet
 *        including DSA tag. (before DSA tag is removed)
 *
 * \param [in] enableDumpRxPacket
 *
 * \return XP_STATUS
 */
extern void dxChNetIfRxPacketParse_DebugDumpEnable
(
    GT_BOOL  enableDumpRxPacket
);

/**
 * \brief API for CPU to PP packet transmision (DSA info as FROM_CPU)
 * \      with syncronized mode.
 *
 * \param [in] devId
 * \param [in] *packetPointer
 * \param [in] *xpDsaHeader_stc
 *
 * \return XP_STATUS
 */

XP_STATUS cpssHalHostIfCPUtoPPPacketTransmitWithDSA
(
    uint32_t devId,
    XPS_HOSTIF_OUT_PACKET_BUFFER_STC *pktPtr,
    xpDsaHeader *xpDsaHeader_stc
);


/**
 * \brief configure Net Dev Tap listen
 * \      send packet read from tap to PP
 *
 * \param [in] devId
 *
 * \return XP_STATUS
 */
XP_STATUS cpssHalHostIfNetDevTapListen
(
    GT_U8  devId
);

/**
 * \brief enable CPU Rx Packet Printing
 *
 * \param [in] printEnable
 */

XP_STATUS cpssHalHostIfCPURxPacketPrintEnable
(
    IN int printPktCnt
);

XP_STATUS cpssHalHostIfDmaRxThreadDeInit(uint32_t devId);


#ifdef __cplusplus
}
#endif

#endif
