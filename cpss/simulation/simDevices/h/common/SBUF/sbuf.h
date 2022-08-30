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
* @file sbuf.h
*
* @brief This is a API definition for SBuf module of the Simulation.
*
* @version   14
********************************************************************************
*/
#ifndef __sbufh
#define __sbufh

#include <os/simTypes.h>
#include <common/SQue/squeue.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define SIM_CAST_BUFF(buf)   ((void*)(SBUF_BUF_ID)(buf))

/* NULL buffer ID in the case of allocation fail */
#define SBUF_BUF_ID_NULL                    NULL

/* typedefs */
typedef enum {
    SBUF_BUF_STATE_FREE_E,
    SBUF_BUF_STATE_BUZY_E
} SBUF_BUF_STATE_ENT;

/**
* @enum GT_RX_CPU_CODE
 *
 * @brief Defines the different CPU codes that indicate the reason for
 * sending a packet to the CPU.
*/
typedef enum{

    /** Can be initiated only by another CPU. */
    CONTROL                     = 1,

    /** Control BPDU packet. */
    CONTROL_BPDU                = 16,

    /** Destination MAC trapped packet. */
    CONTROL_DEST_MAC_TRAP       = 17,

    /** @brief Source MAC trapped packet.
     *  CONTROL_SRC_DST_MAC_TRAP Source & Destination MAC trapped packet.
     */
    CONTROL_SRC_MAC_TRAP        = 18,

    CONTROL_SRC_DST_MAC_TRAP    = 19,

    /** MAC Range Trapped packet. */
    CONTROL_MAC_RANGE_TRAP      = 20,

    /** Trapped to CPU due to Rx sniffer */
    RX_SNIFFER_TRAP             = 21,

    /** Intervention ARP */
    INTERVENTION_ARP            = 32,

    /** Intervention IGMP */
    INTERVENTION_IGMP           = 33,

    /** Intervention Source address. */
    INTERVENTION_SA             = 34,

    /** Intervention Destination address. */
    INTERVENTION_DA             = 35,

    /** Intervention Source and Destination addresses */
    INTERVENTION_SA_DA          = 36,

    /** @brief Intervention - Port Lock to CPU.
     *  Codes set by L3 L7 Engines:
     */
    INTERVENTION_PORT_LOCK      = 37,

    /** Trapped Due to Reserved SIP. */
    RESERVED_SIP_TRAP           = 128,

    /** Trapped Due to Internal SIP. */
    INTERNAL_SIP_TRAP           = 129,

    /** @brief Trapped Due to SIP Spoofing
     *  TCB Codes:
     */
    SIP_SPOOF_TRAP              = 130,

    /** @brief Trap by default Key Entry, after no match was
     *  found in the Flow Table.
     */
    DEF_KEY_TRAP                = 132,

    /** Trap by Ip Classification Tables */
    IP_CLASS_TRAP               = 133,

    /** @brief Trap by Classifier's Key entry after a match
     *  was found in the Flow Table.
     */
    CLASS_KEY_TRAP              = 134,

    /** @brief Packet Mirrored to CPU because of TCP Rst_FIN
     *  trapping.
     */
    TCP_RST_FIN_TRAP            = 135,

    /** @brief Packet Mirrored to CPU because of mirror bit
     *  in Key entry.
     */
    CLASS_KEY_MIRROR            = 136,

    /** Reserved DIP Trapping. */
    RESERVED_DIP_TRAP           = 144,

    /** Multicast Boundary Trapping. */
    MC_BOUNDARY_TRAP            = 145,

    /** Internal DIP. */
    INTERNAL_DIP                = 146,

    /** @brief Packet was trapped due to TTL = 0 (valid for
     *  IP header only).
     */
    IP_ZERO_TTL_TRAP            = 147,

    /** Bad IP header Checksum. */
    BAD_IP_HDR_CHECKSUM         = 148,

    /** @brief Packet did not pass RPF check, need to send
     *  prune message.
     */
    RPF_CHECK_FAILED            = 149,

    /** Packet with Options in the IP header. */
    OPTIONS_IN_IP_HDR           = 150,

    /** @brief Packet which is in the End of an IP tunnel
     *  sent for reassembly to the CPU.
     */
    END_OF_IP_TUNNEL            = 151,

    /** @brief Bad tunnel header - Bad GRE header or packet
     *  need to be fragmented with DF bit set.
     */
    BAD_TUNNEL_HDR              = 152,

    /** @brief IP header contains Errors - version!= 4,
     *  Ip header length < 20.
     */
    IP_HDR_ERROR                = 153,

    /** Trap Command was found in the Route Entry. */
    ROUTE_ENTRY_TRAP            = 154,

    /** @brief IPv4 MTU exceed.
     *  MPLS Codes:
     */
    IP_MTU_EXCEED               = 161,

    /** MPLS MTU exceed. */
    MPLS_MTU_EXCEED             = 162,

    /** @brief Classifier MTU exceed when redirecting a packet
     *  through the classifier.
     *  Ipv4 Codes:
     */
    CLASS_MTU_EXCEED            = 163,

    /** TTL in the MPLS shim header is 0. */
    MPLS_ZERO_TTL_TRAP          = 171,

    /** Trap command was found in the NHLFE. */
    NHLFE_ENTRY_TRAP            = 172,

    /** Illegal pop operation was done. */
    ILLEGAL_POP                 = 173,

    /** @brief Invalid MPLS Interface Entry was fetched.
     *  Egress Pipe Codes:
     */
    INVALID_MPLS_IF             = 174,

    /** @brief Regular packet passed to CPU, Last level
     *  treated (LLT) = Ethernet Bridged.
     */
    ETH_BRIDGED_LLT             = 248,

    /** @brief Regular packet passed to CPU, LLT = IPv4
     *  Routed.
     */
    IPV4_ROUTED_LLT             = 249,

    /** @brief Regular packet passed to CPU, LLT = Unicast
     *  MPLS.
     */
    UC_MPLS_LLT                 = 250,

    /** @brief Regular packet passed to CPU, LLT = Multicast
     *  MPLS (currently not supported).
     */
    MC_MPLS_LLT                 = 251,

    /** @brief Regular packet passed to CPU, LLT = IPv6
     *  Routed (currently not supported).
     */
    IPV6_ROUTED_LLT             = 252,

    /** Regular packet passed to CPU, LLT = L2CE. */
    L2CE_LLT                    = 253

} GT_RX_CPU_CODE;

/**
* @enum GT_PCKT_ENCAP
 *
 * @brief Defines the different transmitted packet encapsulations.
*/
typedef enum{

    /** Non - control packet. */
    GT_REGULAR_PCKT  = 0,

    /** Control packet. */
    GT_CONTROL_PCKT  = 7

} GT_PCKT_ENCAP;

/**
* @enum GT_MAC_TYPE
 *
 * @brief Defines the different Mac-Da types of a transmitted packet.
*/
typedef enum{

    /** MAC_DA[0] = 1'b0 */
    UNICAST_MAC,

    /** MAC_DA[0] = 1'b1 */
    MULTICAST_MAC,

    /** MAC_DA = 0xFFFFFFFF */
    BROADCAST_MAC

} GT_MAC_TYPE;


typedef struct {
    GT_U32               local_ingress_port;
    GT_U32               ingress_is_trunk;
    GT_U32               ingress_trunk; /* 0 based trunk id*/
    GT_BOOL              from_stack_port;
    GT_U32               actual_ingress_port_trunk;
    GT_U32               actual_ingress_device;
    GT_BOOL              is_tagged;
    GT_RX_CPU_CODE       opcode;
    GT_U16               vlan_id;
    GT_U8                user_priority;
    GT_U8                rxQueue;
    GT_U32               frameType ;/*one of SCOR_FRAME_TYPE_ENT*/
}SAPI_RX_PACKET_DESC_STC ;


/*
 * Typedef: struct SAPI_TX_PACKET_DESC_STC
 *
 * Description: Includes needed parameters for enqueuing a packet to the PP's
 *              Tx queues.
 *
 * Fields:
 *      userPrioTag - User priority attached to the tag header.
 *      packetTagged - Packet is tagged.
 *      packetEncap - The outgoing packet encapsulation.
 *      sendTagged  - need to send the packet as tagged (not used when useVidx =1).
 *      dropPrecedence - The packet's drop precedence.
 *      recalcCrc   - GT_TRUE the PP should add CRC to the transmitted packet,
 *                    GT_FALSE leave packet unchanged.
 *      vid         - Packet vid.
 *      macDaType   - Type of MAC-DA (valid only if packetEncap = ETHERNET).
 *      txQueue     - The tx Queue number to transmit the packet on.
 *      txDevice    -  the device from which to send the packet this device will
 *                     send to the tgtDev,tgtPort or to the vidx
 *      cookie      - The user's data to be returned to the tapi Tx End handling
 *                    function.
 *
 *      useVidx     - Use / Don't use the Vidx field for forwarding the packet.
 *      dest        - Packet destination:
 *                      if(useVidx == 1)
 *                          Transmit according to vidx.
 *                      else
 *                          Transmit according to tgtDev & tgtPort.
 *
 */
typedef union {
    GT_U16       vidx;
    struct{
        GT_U8    tgtDev;
        GT_U8    tgtPort; /* 0 based port number --- never trunk */
    }devPort;
}SAPI_VIDX_UNT;

typedef struct {
    GT_U8           userPrioTag;
    GT_BOOL         packetTagged;
    GT_BOOL         sendTagged;
    GT_PCKT_ENCAP   packetEncap;
    GT_U8           dropPrecedence;
    GT_BOOL         recalcCrc;
    GT_U16          vid;
    GT_MAC_TYPE     macDaType;
    GT_U8           txQueue;
    GT_U8           txDevice;

    /* Internal Control Data    */
    GT_PTR          cookie;

    GT_BOOL         useVidx;
    SAPI_VIDX_UNT   dest;

}SAPI_TX_PACKET_DESC_STC ;

typedef struct{
    GT_BOOL     uplinkPort;/* uplink port has numbers of its own  */
    GT_U8       srcPort; /* the number of port --
                            can be the number of upLink port (not regular port)*/
}SAPI_SLAN_DESC_STC;

/**
* @enum SBUF_FA_FORWARD_TYPE_ENT
 *
 * @brief FORWARDING to TARGET types.
*/
typedef enum{

    SBUF_FA_FORWARD_TYPE_NONE_E,

    SBUF_FA_FORWARD_TYPE_UC_FRAME_E,

    SBUF_FA_FORWARD_TYPE_MC_FRAME_E,

    SBUF_FA_FORWARD_TYPE_BC_FRAME_E,

    SBUF_FA_FORWARD_TYPE_EGRESS_FPORT_E,

    SBUF_FA_FORWARD_TYPE_EGRESS_CPU_E,

    SBUF_FA_FORWARD_TYPE_LAST_E

} SBUF_FA_FORWARD_TYPE_ENT;

/**
* @struct SAPI_FA_FORWARD_DESC_STC
 *
 * @brief Burst header .
*/
typedef struct {
    union {
        GT_U32 mcGroupId ; /* Mc */
        GT_U32 vlanId ; /* Bc, not supported by FA/XBAR, may by supported by PP */
        GT_U32 egressFport ;
        struct {
          GT_U8 deviceId ;
          GT_U8 port ;
          GT_U8 LBH ; /* load balance hash 0-3 */
          GT_U8 reserved;
        } ucTarget ;
    } target ; /* target type dependent format */
    SBUF_FA_FORWARD_TYPE_ENT targetType ;
    GT_U32 ingressFport ;
} SAPI_FA_FORWARD_DESC_STC ;

/* data send on the uplink from/to the PP */
typedef enum {
    SAPI_UPLINK_DATA_TYPE_FRAME_E,
    SAPI_UPLINK_DATA_TYPE_FDB_UPDATE_E
}SAPI_UPLINK_DATA_TYPE_ENT;

typedef struct{
    GT_MAC_TYPE     macDaType;
    GT_BOOL         useVidx;
    SAPI_VIDX_UNT   dest;
    GT_BOOL         doTrapToMaster;
}SAPI_UPLINK_FRAME_STC;

typedef struct{
    GT_U32      reserved;
}SAPI_UPLINK_FDB_UPDATE_STC;

typedef struct{
    SAPI_UPLINK_DATA_TYPE_ENT   type;
    union{
        SAPI_UPLINK_FRAME_STC      frame;
        SAPI_UPLINK_FDB_UPDATE_STC fdbUpdate;
    }info;
}SAPI_UPLINK_DATA_STC;

typedef enum {
    SBUF_USER_INFO_TYPE_SAPI_TX_E=1,
    SBUF_USER_INFO_TYPE_SAPI_RX_E,
    SBUF_USER_INFO_TYPE_SAPI_LINK_CHANGE_E,
    SBUF_USER_INFO_TYPE_SAPI_FDB_MSG_E,
    SBUF_USER_INFO_TYPE_SAPI_RW_REQUEST_E,
    SBUF_USER_INFO_TYPE_SAPI_FA_FORWARD_E,     /* for FA support */
    SAPI_USER_INFO_TYPE_SAPI_UPLINK_DATA_E,
    SAPI_USER_INFO_TYPE_SAPI_MAC_ACTION_ENDED_E,
    SAPI_USER_INFO_TYPE_SAPI_TX_SDMA_QUE_BMP_E
}SBUF_USER_INFO_TYPE_ENT;

typedef struct{
    SBUF_USER_INFO_TYPE_ENT type;
    union{
        SAPI_TX_PACKET_DESC_STC sapiTxPacket;/*scib to scor*/
        SAPI_RX_PACKET_DESC_STC sapiRxPacket;/*scor to scib*/
        SAPI_SLAN_DESC_STC      sapiSlanInfo;/*slan to scor*/
        SAPI_FA_FORWARD_DESC_STC sapiFaForward;/* scor<->scorFa<->scor
                                                or scor<->scor all via upLink*/
        SAPI_UPLINK_DATA_STC    sapiUplinkDataInfo;/* scor<->scorFa<->scor
                                                or scor<->scor all via upLink*/
        GT_U32      txSdmaQueueBmp;           /* transmit SDMA queue bitmap */
    }data;

    union{
        GT_U32      baseAddress;/* scor / scorFa send to scib */
        void*       devObj_PTR; /* scib send to scor/scor FA  */
    }target;
    GT_U32  extValue;  /* extra value */
}SBUF_USER_INFO_STC;

/* Maximal dataSize in the buffer, max size of ingress and egress packets */
#define SBUF_DATA_SIZE_CNS                  12000

/**
* @enum SBUF_BUFFER_FREE_STATE_ENT
 *
 * @brief values that the allocator of the buffer should consider
*/
typedef enum{

    /** @brief the allocator of the buffer can
     *  free the buffer , because the buffer not used by other context.
     */
    SBUF_BUFFER_STATE_ALLOCATOR_CAN_FREE_E = 0,

    /** @brief the allocator of the buffer can NOT
     *  free the buffer , because the buffer is used by other context.
     *  the other context will free the buffer.
     */
    SBUF_BUFFER_STATE_OTHER_WILL_FREE_E

} SBUF_BUFFER_FREE_STATE_ENT;

/*
 * Typedef: struct SBUF_BUF_STC
 *
 * Description:
 *      Describe the buffer in the simulation.
 *
 * Fields:
 *      magic           : Magic number for consistence check.
 *      nextBufPtr      : Pointer to the next buffer in the pool or queue.
 *      state           : Buffers' state.
 *      freeState       : Buffers' state, when need to free allocated buffer.
 *      dataType        : Type of buffer's data.
 *      srcType         : Type of buffer's source.
 *      srcData         : Source specific data.
 *      actualDataPtr   : Pointer to the actual basic data at data section.
 *      actualDataSize  : Size of the actual data.
 *      userInfo        : User information.
 *      pipeId          : support multiple pipeId
 *      mgUnitId        : support multiple MG units
 *      isPreemptiveChannel   : support 802.3BR preemptive/express ingress traffic
 *                        value 1 means PMAC (preemptive MAC) , value 0 means EMAC (express MAC)
 *      data            : Buffer's data memory - dynamic allocated buffer to hold info/packet .
 *                        when pool created.
 *      numberOfSubscribers : number of subscribers / references to this descriptor
 * Comments:
 */
typedef struct SBUF_BUF_STCT {
/* first fields bust be the same fields as in : SIM_BUFFER_STC */
    GT_U32                  magic;
    struct SBUF_BUF_STCT *  nextBufPtr;
    SBUF_BUF_STATE_ENT      state;
    SBUF_BUFFER_FREE_STATE_ENT freeState;
    GT_U32                  dataType;
    GT_U32                  srcType;
    GT_U32                  srcData;
    GT_U8                *  actualDataPtr;
    GT_U32                  actualDataSize;
    SBUF_USER_INFO_STC      userInfo;
    GT_U32                  pipeId;
    GT_U32                  mgUnitId;
    GT_U32                  isPreemptiveChannel;
    GT_U8                   *data;/* dynamic allocated buffer to hold info/packet*/
    GT_U32                  numberOfSubscribers;
    GT_U32                  overSize;  /* when not ZERO , meaning that the buffer not able to hold the
                                        full packet , and the packet must be dropped , but only after
                                        counted as oversize on MRU check */
} SBUF_BUF_STC;

/* buffer ID typedef */
typedef SBUF_BUF_STC  * SBUF_BUF_ID;


/* Pool ID typedef */
typedef  void * SBUF_POOL_ID;

/* API functions */

/**
* @internal sbufInit function
* @endinternal
*
* @brief   Initialize internal structures for pools and buffers management.
*
* @param[in] maxPoolsNum              - maximal number of buffer pools.
*
* @note In the case of memory lack the function aborts application
*
*/
void sbufInit
(
    IN  GT_U32              maxPoolsNum
);

/**
* @internal sbufPoolCreate function
* @endinternal
*
* @brief   Create new buffers pool.
*
* @param[in] poolSize                 - number of buffers in a pool.
*
* @retval SBUF_POOL_ID             - new pool ID
*
* @note In the case of memory lack the function aborts application.
*
*/
SBUF_POOL_ID sbufPoolCreate
(
    IN  GT_U32              poolSize
);

/**
* @internal sbufPoolCreateWithBufferSize function
* @endinternal
*
* @brief   Create new buffers pool.
*
* @param[in] poolSize                 - number of buffers in a pool.
* @param[in] bufferSize               - size in bytes of each buffer
*
* @retval SBUF_POOL_ID             - new pool ID
*
* @note In the case of memory lack the function aborts application.
*
*/
SBUF_POOL_ID sbufPoolCreateWithBufferSize
(
    IN  GT_U32              poolSize,
    IN  GT_U32              bufferSize
);


/**
* @internal sbufPoolFree function
* @endinternal
*
* @brief   Free buffers memory.
*
* @param[in] poolId                   - id of a pool.
*/
void sbufPoolFree
(
    IN  SBUF_POOL_ID    poolId
);
/**
* @internal sbufAlloc function
* @endinternal
*
* @brief   Allocate buffer.
*
* @param[in] poolId                   - id of a pool.
* @param[in] dataSize                 - size of the data.
*
* @retval SBUF_BUF_ID              - buffer id if exist.
* @retval SBUF_BUF_ID_NULL         - if no free buffers.
*/
SBUF_BUF_ID sbufAlloc
(
    IN  SBUF_POOL_ID    poolId,
    IN  GT_U32          dataSize
);

/**
* @internal sbufAllocWithProtectedAmount function
* @endinternal
*
* @brief   Allocate buffer , but only if there are enough free buffers after the alloc.
*
* @param[in] poolId                   - id of a pool.
* @param[in] dataSize                 - size for the alloc.
* @param[in] protectedAmount          - number of free buffers that must be still in the pool
*                                      (after alloc of 'this' one)
*
* @retval SBUF_BUF_ID              - buffer id if exist.
* @retval SBUF_BUF_ID_NULL         - if no free buffers.
*/
SBUF_BUF_ID sbufAllocWithProtectedAmount
(
    IN  SBUF_POOL_ID    poolId,
    IN  GT_U32          dataSize,
    IN  GT_U32          protectedAmount
);

/**
* @internal sbufFree function
* @endinternal
*
* @brief   Free buffer.
*
* @param[in] poolId                   - id of a pool.
* @param[in] bufId                    - id of buffer
*/
void sbufFree
(
    IN  SBUF_POOL_ID    poolId,
    IN  SBUF_BUF_ID     bufId
);
/**
* @internal sbufDataGet function
* @endinternal
*
* @brief   Get pointer on the first byte of data in the buffer
*         and actual size of data.
* @param[in] bufId                    - id of buffer
*/
void sbufDataGet
(
    IN  SBUF_BUF_ID     bufId,
    OUT GT_U8   **      dataPrtPtr,
    OUT GT_U32  *       dataSizePrt
);
/**
* @internal sbufDataSet function
* @endinternal
*
* @brief   Set pointer to the start of data and new data size.
*
* @param[in] bufId                    - id of buffer
* @param[in] dataPrt                  - pointer to actual data of buffer
* @param[in] dataSize                 - actual data size of a buffer
*/
void sbufDataSet
(
    IN  SBUF_BUF_ID     bufId,
    IN  GT_U8   *       dataPrt,
    IN  GT_U32          dataSize
);


/**
* @internal sbufGetBufIdByData function
* @endinternal
*
* @brief   Get buff ID of buffer , by the pointer to any place inside the buffer
*
* @param[in] poolId                   - id of a pool.
* @param[in] dataPtr                  - pointer to actual data of buffer
*
* @retval buff_id                  - the ID of the buffer that the data is in .
*                                       NULL if dataPtr is not in a buffer
*
* @note dataPtr - the pointer must point to data in the buffer
*
*/
SBUF_BUF_ID sbufGetBufIdByData(
    IN  SBUF_POOL_ID    poolId,
    IN GT_U8   *        dataPtr
);

/**
* @internal sbufFreeBuffersNumGet function
* @endinternal
*
* @brief   get the number of free buffers.
*
* @param[in] poolId                   - id of a pool.
*                                       the number of free buffers.
*/
GT_U32 sbufFreeBuffersNumGet
(
    IN  SBUF_POOL_ID    poolId
);

/**
* @internal sbufAllocatedBuffersNumGet function
* @endinternal
*
* @brief   get the number of allocated buffers.
*
* @param[in] poolId                   - id of a pool.
*                                       the number of allocated buffers.
*/
GT_U32 sbufAllocatedBuffersNumGet
(
    IN  SBUF_POOL_ID    poolId
);

/**
* @internal sbufPoolSuspend function
* @endinternal
*
* @brief   suspend a pool (for any next alloc)
*
* @param[in] poolId                   - id of a pool.
*/
void sbufPoolSuspend
(
    IN  SBUF_POOL_ID    poolId
);
/**
* @internal sbufPoolResume function
* @endinternal
*
* @brief   Resume a pool that was suspended by sbufAllocAndPoolSuspend or by sbufPoolSuspend
*
* @param[in] poolId                   - id of a pool.
*/
void sbufPoolResume
(
    IN  SBUF_POOL_ID    poolId
);

/**
* @internal sbufAllocAndPoolSuspend function
* @endinternal
*
* @brief   Allocate buffer and then 'suspend' the pool (for any next alloc).
*
* @param[in] poolId                   - id of a pool.
* @param[in] dataSize                 - size of the data.
*
* @retval SBUF_BUF_ID              - buffer id if exist.
* @retval SBUF_BUF_ID_NULL         - if no free buffers.
*/
SBUF_BUF_ID sbufAllocAndPoolSuspend
(
    IN  SBUF_POOL_ID    poolId,
    IN  GT_U32          dataSize
);

/**
* @internal sbufPoolFlush function
* @endinternal
*
* @brief   flush all buffers in a pool (state that all buffers are free).
*         NOTE: operation valid only if pool is suspended !!!
* @param[in] poolId                   - id of a pool.
*/
void sbufPoolFlush
(
    IN  SBUF_POOL_ID    poolId
);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif  /* __sbufh */



