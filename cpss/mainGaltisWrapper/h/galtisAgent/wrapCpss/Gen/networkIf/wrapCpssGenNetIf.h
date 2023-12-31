/*******************************************************************************
*                Copyright 2001, Marvell International Ltd.
* This code contains confidential information of Marvell semiconductor, inc.
* no rights are granted herein under any patent, mask work right or copyright
* of Marvell or any third party.
* Marvell reserves the right at its sole discretion to request that this code
* be immediately returned to Marvell. This code is provided "as is".
* Marvell makes no warranties, express, implied or otherwise, regarding its
* accuracy, completeness or performance.
********************************************************************************
*/
/**
********************************************************************************
* @file wrapCpssGenNetIf.h
*
* @brief This file implements Receive/Transmit functionaly user exit functions
* for Galtis.
*
* @version   18
********************************************************************************
*/

#ifndef __wrapCpssGenNetIf_h__
#define __wrapCpssGenNetIf_h__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/cpssTypes.h>
#include <cpss/common/cos/cpssCosTypes.h>
#ifdef CHX_FAMILY
#include <cpss/generic/networkIf/private/prvCpssGenNetworkIfDsaTag.h>
#endif /*CHX_FAMILY*/

#define   PRV_MAX_NUM_OF_SDMA_BUFFERS_PER_CHAIN   24
/*number of buffer at the pool*/
#define   PRV_MAX_NUM_OF_BUFFERS_AT_POOL 128
#define   PRV_TX_BUFFER_SIZE 512

#define   MAX_NUM_OF_SDMA_BUFFERS_PER_CHAIN     64
/*number of buffer at the pool*/
#define   MAX_NUM_OF_BUFFERS_AT_POOL            124
#define   TX_BUFFER_SIZE                        500

/* Size of block to aligned to. Must be power of 2, 2 is minimum */
#define ALIGN_ADDR_CHUNK                        64
/* This macro used to get alighnment address from allocated */
#define ALIGN_ADR_FROM_MEM_BLOCK(adr,align)\
                   (( ((GT_U32)( ((GT_U8*)(adr)) + (ALIGN_ADDR_CHUNK) - 1)) &\
                   ((0xFFFFFFFF - (ALIGN_ADDR_CHUNK) + 1) )) + (align))

/*
 * Typedef: struct GT_PKT_DESC
 *
 * Description: Packet Description structure
 *
 *      valid           - This flag indicates that the entry is valid (or not)
 *      entryID         - Entry ID number for Delete/Get/ChangeAllign operations
 *      devNum          - device number to which to send the packet.
 *      txSyncMode      - should the packet be send in a sync mode or not
 *      packetIsTagged  - does the packet already include tag.
 *      dropPrecedence  - Drop precedence (color), range 0-2
 *      trafficClass    - Egress tx traffic class queue, range 0-7
 *      recalcCrc       - (GT_TRUE) Add Crc to the transmitted packet,
 *                        (GT_FALSE) Leave the packet as is.
 *      txQueue         - the queue that packet should be sent to CPU port. (0..7)
 *
 *      invokeTxBufferQueueEvent - invoke Tx buffer Queue event.
 *              when the SDMA copies the buffers of the packet from the CPU ,
 *              the PP may invoke the event of CPSS_PP_TX_BUFFER_QUEUE_E.
 *              this event notify the CPU that the Tx buffers of the packet can
 *              be reused by CPU for other packets. (this in not event of
 *              CPSS_PP_TX_END_E that notify that packet left the SDMA)
 *              The invokeTxBufferQueueEvent parameter control the invoking of
 *              that event for this Packet.
 *              GT_TRUE - PP will invoke the event when buffers are copied (for
 *                        this packet's buffers).
 *              GT_FALSE - PP will NOT invoke the event when buffers are copied.
 *                        (for this packet's buffers).
 *              NOTE :
 *                  when the sent is done as "Tx synchronic" this parameter
 *                  IGNORED (the behavior will be as GT_FALSE)
 *
 *      numPacketsToSend- Number of packets to send.
 *      gap             - The wait time between re transmission of this entry.
 *      waitTime        - The wait time before moving to the next entry.
 *      pcktData        - Array of pointers to packet binary data.
 *      pcktDataLen     - Array of lengths of pcktData pointers.
 *      numOfBuffers    - number of buffers used to store the packet data.
 *      numSentPackets  - Number of sent packets.
 *      dstInterface    - Packet's destination interface:
 *         type == CPSS_INTERFACE_TRUNK_E --> not supported !
 *         when type == CPSS_INTERFACE_PORT_E
 *              devPort.tgtDev - the target device that packet will be send to.
 *                               NOTE : HW device number !!
 *              devPort.tgtPort- the target port on device that packet will be
 *                             send to.
 *         when type == CPSS_INTERFACE_VIDX_E
 *              vidx     - the vidx group that packet will be sent to.
 *         when type == CPSS_INTERFACE_VID_E
 *              vlanId   - must be equal to vid !!!
 *      excludeInterface - to exclude a "source" interface from the VLAN/VIDX
 *                        flooding.
 *                      GT_FALSE - no "source" interface to exclude
 *                      GT_TRUE  - use "source" interface to exclude , see
 *                                 parameter excludedInterface.
 *      excludedInterface - the "source" interface to exclude from the
 *                           only Port or Trunk
 *      specificDeviceTypeInfo - additional specific device info, such as DSA format.
 *
 *      dsaTagType - DSA tag types
 *
 *      Forward DSA tag specific fields. Relevant for ExMxPm devices
 *
 *      srcIsTrunk  - indicates that packet was/was not received from a network port that is part of a trunk
 *      trunkId     - if srcIsTrunk == 1 - source trunk number on which the packet was received
 *      portNum     - if srcIsTrunk == 0 - together with source device, source port indicates the packet�s source port number
 *      srcDevice   - source device from which the packet was received
 *      wasRouted   - indicates that packet has/has not been L3 routed
 */
typedef struct
{
    GT_BOOL                  valid;
    GT_U32                   entryId;
    GT_U8                    devNum;
    GT_BOOL                  packetIsTagged;
    GT_U16                   vid;
    CPSS_DP_LEVEL_ENT        dropPrecedence;
    GT_U8                    trafficClass;
    GT_BOOL                  recalcCrc;
    GT_BOOL                  txSyncMode;
    GT_U8                    txQueue;
    GT_BOOL                  invokeTxBufferQueueEvent;
    GT_U32                   numPacketsToSend;
    GT_U32                   gap;
    GT_U32                   waitTime;
    GT_U8                    *pcktData[PRV_MAX_NUM_OF_SDMA_BUFFERS_PER_CHAIN];
    GT_U32                   pcktDataLen[PRV_MAX_NUM_OF_SDMA_BUFFERS_PER_CHAIN];
    GT_U32                   numOfBuffers;
    GT_U32                   numSentPackets;
    CPSS_INTERFACE_INFO_STC  dstInterface; /* vid/vidx/port */
    GT_BOOL                  dstIsTagged; /* In case dstInterface is port.*/
    GT_BOOL                  excludeInterface;
    CPSS_INTERFACE_INFO_STC  excludedInterface; /* port/trunk */
    void                    *specificDeviceTypeInfo;
#ifdef CHX_FAMILY
    PRV_CPSS_NET_DSA_CMD_ENT dsaTagCmd;
#endif
    /* Forward DSA tag specific fields */
    GT_BOOL                  srcIsTrunk;
    union
    {
        GT_TRUNK_ID          trunkId;
        GT_U8                portNum;
    }source;
    GT_U8                    srcDevice;
    GT_BOOL                  wasRouted;
} CPSS_TX_PKT_DESC;

/*
 * Typedef: struct CPSS_TX_MODE_SETITNGS_STC
 *
 * Description: Packet Tx mode structure
 *
 *      cyclesNum - The maximum number of loop cycles (-1 = forever)
 *      gap       - The time to wait between two transmittion cycles.
 *
 */

typedef struct
{
    GT_U32  cyclesNum;
    GT_U32  gap;
} CPSS_TX_MODE_SETITNGS_STC;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /*__wrapCpssGenNetIf_h__*/



