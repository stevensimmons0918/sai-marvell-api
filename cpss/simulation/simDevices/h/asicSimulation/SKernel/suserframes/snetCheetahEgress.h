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
* @file snetCheetahEgress.h
*
* @brief This is a external API definition for SMem module of SKernel.
*
* @version   21
********************************************************************************
*/
#ifndef __snetCheetahEgressh
#define __snetCheetahEgressh

#include <asicSimulation/SKernel/smain/smain.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define SNET_CHT_RX_DESC_CPU_OWN     (0)

#define CFM_OPCODE_BYTE_OFFSET_CNS   (1)

/* Sets the Own bit field of the rx descriptor */
/* Returns / Sets the Own bit field of the rx descriptor.           */
#define SNET_CHT_RX_DESC_GET_OWN_BIT(rxDesc) (((rxDesc)->word1) >> 31)

/* Return the buffer size field from the second word of an Rx desc. */
/* Make sure to set the lower 3 bits to 0.                          */
#define SNET_CHT_RX_DESC_GET_BUFF_SIZE_FIELD(rxDesc)             \
            (((((rxDesc)->word2) >> 3) & 0x7FF) << 3)

#define SNET_CHT_RX_DESC_SET_OWN_BIT(rxDesc, val)                     \
               (SMEM_U32_SET_FIELD((rxDesc)->word1,31,1,val))

#define SNET_CHT_RX_DESC_SET_FIRST_BIT(rxDesc, val)                     \
            (SMEM_U32_SET_FIELD((rxDesc)->word1,27,1,val))

#define SNET_CHT_RX_DESC_SET_LAST_BIT(rxDesc, val)                     \
            (SMEM_U32_SET_FIELD((rxDesc)->word1,26,1,val))

/* Return the byte count field from the second word of an Rx desc.  */
/* Make sure to set the lower 3 bits to 0.                          */
#define SNET_CHT_RX_DESC_SET_BYTE_COUNT_FIELD(rxDesc, val)        \
            (SMEM_U32_SET_FIELD((rxDesc)->word2,16,14,val))

/* Return the EI (enable interrupt) field from the first word of an Rx desc.  */
#define SNET_CHT_RX_DESC_GET_EI_FIELD(rxDesc)        \
             (((rxDesc)->word1  & 0x20000000) >> 29)


/* This is a reserved SrcPort indicating packet was TS on prior device.
  (relevant for XCat devices) */
#define SNET_CHT_PORT_60_CNS                           60

/* Embedded CPU port -- CAPWAP DTLS */
#define SNET_CHT_EMBEDDED_CPU_PORT_CNS                 59

/* OSM special port -- CAPWAP 802.11 "protected" data channel */
/* 7. CAPWAP Data Channel 802.11 Encryption/Decryption on the AC
    In Split MAC mode, in the event that the 802.11 encryption/decryption is
    performed on the AC rather than the WTP, the 802.11 data channel payload
    must be processed by an external CPU prior to ingress processing or
    transmission.
    This functionality is supported using the OSM ingress and egress redirection
    facility.
    7.1 Ingress 802.11 Decryption
        For the ingress case, the incoming packet matches a TTI for the given
        BSSID, and the TTI Action configuration has the <Ingress OSM Redirection>
        field set and the <OSM_CPU_Code> set to user-defined CPU Code.
        The packet is then redirected according to the CPU Code table entry for
        the user-defined CPU code.
        The CPU returns the packet with the 802.11 payload unencrypted,
        specifying in the DSA tag the packets original source port,and a
        special destination port 60 indicating that this packet was OSM
        redirected.
        Note the packet is subject to normal ingress processing as if it was
        received on the original ingress port.
    7.2 Egress 802.11 Decryption
        For the egress case, the outgoing packet tunnel-start entry is
        configured with <Egress OSM Redirect> and the <OSM_CPU_Code> set to the
        user-defined CPU Code.
        The packet is then redirected according to the CPU Code table entry
        for the user-defined CPU code.
        The CPU returns the packet with the encrypted 802.11 payload, specifying
        in the FROM CPU DSA tag the packets egress port.
        The packet is then transmitted as-is on the egress port.
*/
#define SNET_CHT_OSM_PORT_CNS                 0x3C /*60*/

/* get the actual number of ports of the device */
#define SNET_CHEETAH_MAX_PORT_NUM(dev)   ((dev)->portsNumber)

/* TXQ works in 'Hemisphere' -
   So Global port number is limited to 0..63 .
   the start port of the device/core start with (in context of 'Hemisphere')*/
#define SNET_CHT_EGR_TXQ_START_PORT_MAC(dev) \
       (SMEM_CHT_IS_SIP5_GET(dev) ? 0 :/* start from 0 */ \
       (((dev)->supportTxQGlobalPorts) ? \
        ((SMEM_CHT_GLOBAL_PORT_FROM_LOCAL_PORT_MAC(dev,0)) & 0x3f) : 0))

/* max physical ports on sip5 devices */
#define SNET_SIP5_EGR_MAX_PHY_PORTS_MAC(dev)    (GT_U32)(devObjPtr->limitedResources.phyPort ? \
                                                         devObjPtr->limitedResources.phyPort : \
                                                         (1 << (dev)->flexFieldNumBitsSupport.phyPort))

/* Global port number that the device start with plus total number of ports for this device */
#define SNET_CHT_EGR_TXQ_END_PORT_MAC(dev) \
       (SMEM_CHT_IS_SIP5_GET(dev) ? SNET_SIP5_EGR_MAX_PHY_PORTS_MAC(dev) :/* end at port 127/255/512 */ \
        (SNET_CHT_EGR_TXQ_START_PORT_MAC(dev) + SNET_CHEETAH_MAX_PORT_NUM(dev)))

/**
* @struct SNET_CHT_RX_DESC
 *
 * @brief Includes the PP Rx descriptor fields, to be used for handling
 * received packets.
*/
typedef struct{

    /** First word of the Rx Descriptor. */
    GT_U32 word1;

    /** Second word of the Rx Descriptor. */
    GT_U32 word2;

    /** The physical data */
    GT_U32 buffPointer;

    /** The physical address of the next Rx descriptor. */
    GT_U32 nextDescPointer;

} SNET_CHT_RX_DESC;

typedef enum {
    SNET_CHT_READ_VLT_ACTION = 0,
    SNET_CHT_WRITE_VLT_ACTION
}SNET_CHT_RDWR_VLT_ACTION;

typedef enum {
    SNET_CHT_VLAN_VLT_TABLE = 0,
    SNET_CHT_MCGROUP_VLT_TABLE,
    SNET_CHT_STG_VLT_TABLE
}SNET_CHT_TABLE_VLT_TYPE;

typedef enum{
    SNET_CHT_PRIO_TABLE_ACCESS_MODE_2B_SRC_2B_TRG_E = 0,
    SNET_CHT_PRIO_TABLE_ACCESS_MODE_4B_TRG,
    SNET_CHT_PRIO_TABLE_ACCESS_MODE_1B_SRC_3B_TRG
} SNET_CHT_PRIO_TABLE_ACCESS_MODE;

/*  Modes of hashing into the designated trunk table that related to trunk member selection.
    NOTE: Cascade trunk is trunk that pointer by the 'Device map table' as
    destination for remote device.
    As well as multi-destination packets are forwarded according to
    global hashing mode of trunk hash generation.
*/
typedef enum{
    TRUNK_HASH_MODE_USE_PACKET_HASH_E,  /* use packet hash */
    TRUNK_HASH_MODE_USE_GLOBAL_SRC_PORT_HASH_E,/* use global src port for hash */
    TRUNK_HASH_MODE_USE_GLOBAL_DST_PORT_HASH_E, /* use global dst port for hash */
    TRUNK_HASH_MODE_USE_LOCAL_SRC_PORT_HASH_E,/* use global src port for hash , but not trunkId */

    TRUNK_HASH_MODE_USE_MULIT_DESTINATION_HASH_SETTINGS_E/* used for cascade trunk
                                                            that use the values of
                                                            the multi destination and
                                                            not per cascade trunk entry */

}TRUNK_HASH_MODE_ENT;

/**
* @internal snetChtEgress function
* @endinternal
*
* @brief   Egress processing main routine
*/
GT_VOID snetChtEgress
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr
);

/**
* @internal snetChtEgressAfterTmUnit function
* @endinternal
*
* @brief   finish processing the packet in the egress pipe after the TM unit
*         finished with the packet.
* @param[in] devObjPtr                - pointer to device object.
* @param[in] dataPtr                  - pointer to TM attached info.
* @param[in] dataLength               - length of the data
*
* @note this function called in context of smain task but hold buffer with
*       packet + descriptor that where send from the same task but 'long time ago'
*
*/
GT_VOID snetChtEgressAfterTmUnit
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U8                 * dataPtr,
    IN GT_U32                  dataLength
);


/**
* @internal snetChtHaPerPortInfoGet function
* @endinternal
*
* @brief   Header Alteration - get indication about the index of per port and use
*         of second register
*
* @param[out] isSecondRegisterPtr      - pointer to 'use the second register'
* @param[out] outputPortBitPtr         - pointer to the bit index for the egress port
*/
void snetChtHaPerPortInfoGet
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32   egressPort,
    OUT GT_BOOL     *isSecondRegisterPtr,
    OUT GT_U32     *outputPortBitPtr
);


/**
* @internal snetChtEgfEgressEPortEntryGet function
* @endinternal
*
* @brief   function of the EGF unit
*         get pointer to the egress EPort table
*         Set value into descrPtr->eArchExtInfo.egfShtEgressEPortTablePtr
*         Set value into descrPtr->eArchExtInfo.egfQagEgressEPortTablePtr
* @param[in] devObjPtr                - pointer to device object.
* @param[in] descrPtr                 - Cht frame descriptor
* @param[in] globalEgressPort         - physical egress port -- Global port number !!!
*                                      (referred as <LocalDevTrgPhyPort> in documentation ,
*                                      but it is not part of the 'descriptor' !)
* @param[in] skipLogInfo              - indication to skip LOG info
*                                      GT_TRUE - skip LOG info in the function
*                                      GT_FALSE - allow LOG info in the function
*/
void snetChtEgfEgressEPortEntryGet
(
   IN SKERNEL_DEVICE_OBJECT * devObjPtr,
   IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
   IN GT_U32    globalEgressPort,
   IN GT_BOOL   skipLogInfo
);

/**
* @internal snetChtHaArpTblEntryGet function
* @endinternal
*
* @brief   HA - Get ARP table mac address entry
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] entryIndex               - pointer to ARP memory table entry.
*                                      OUTPUT:
* @param[in] arpEntry                 - pointer to ARP entry
*                                       COMMENT:
*/
GT_VOID snetChtHaArpTblEntryGet(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32 entryIndex,
    OUT SGT_MAC_ADDR_TYP * arpEntry
);

/**
* @internal snetChtHaEgressTagDataExtGet function
* @endinternal
*
* @brief   HA - build VLAN tag info , in network order
*/
void snetChtHaEgressTagDataExtGet
(
    IN  GT_U8   vpt,
    IN  GT_U16  vid,
    IN  GT_U8   cfiDeiBit,
    IN  GT_U16  etherType,
    OUT GT_U8   tagData[] /* 4 bytes */
);

/**
* @internal snetChtHaMacFromMeBuild function
* @endinternal
*
* @brief   HA - get the SRC mac address to use as "mac from me"
*
* @param[out] macAddrPtr               - pointer to the mac address (6 bytes)
*                                      COMMENTS:
*/
void snetChtHaMacFromMeBuild
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN GT_U32   egressPort,
    IN GT_BOOL  usedForTunnelStart,
    OUT GT_U8   *macAddrPtr
);

/**
* @internal snetChtHaMain function
* @endinternal
*
* @brief   HA - main HA unit logic
*/
void snetChtHaMain
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN GT_U32   egressPort,
    IN GT_U8    destVlanTagged,
    OUT GT_U8 **frameDataPtrPtr,
    OUT GT_U32 *frameDataSize,
    OUT GT_BOOL *isPciSdma
);

/**
* @internal snetChtEgressDev_afterEnqueFromTqxDq function
* @endinternal
*
* @brief   Final Egress proceeding for descriptor that was stack in the queue of
*         TXQ_DQ port
* @param[in] devObjPtr                - pointer to device object.
* @param[in] descrPtr                 - Cht frame descriptor
* @param[in] egressPort               - egress port
*                                       COMMENTS :
*/
GT_VOID snetChtEgressDev_afterEnqueFromTqxDq
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN GT_U32   egressPort,
    IN GT_U32   destVlanTagged
);

/**
* @internal snetChtTxqDqPerPortInfoGet function
* @endinternal
*
* @brief   Txq.dq - get indication about the index of per port and use of second register
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] descrPtr                 - pointer to the frame's descriptor.
* @param[in] egressPort               - egress port (local port on multi-port group device)
*                                      CPU port is port 63
*
* @param[out] isSecondRegisterPtr      - pointer to 'use the second register'
* @param[out] outputPortBitPtr         - pointer to the bit index for the egress port
*/
void snetChtTxqDqPerPortInfoGet
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN    SKERNEL_FRAME_CHEETAH_DESCR_STC *descrPtr,
    IN GT_U32   egressPort,
    OUT GT_BOOL     *isSecondRegisterPtr,
    OUT GT_U32     *outputPortBitPtr
);

/**
* @internal snetChtTx2Port function
* @endinternal
*
* @brief   Forward frame to target port
*
* @param[in] devObjPtr                -  pointer to device object.
* @param[in,out] descrPtr                 -    pointer to the frame's descriptor.
* @param[in] egressPort               -   number of egress port.
*/
GT_BOOL snetChtTx2Port
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN GT_U32 egressPort,
    IN GT_U8 *frameDataPtr,
    IN GT_U32 FrameDataSize
);

/**
* @internal snetChtEgressGetPortsBmpFromMem function
* @endinternal
*
* @brief   Egress Pipe : build bmp of ports from the pointer to the memory (registers/table entry)
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] memPtr                   - pointer to the memory
* @param[in] fillHalfPorts            - 0 - fill all
*                                      1 - fill first half ports
*                                      2 - fill second half ports
*                                      >=64 --> the number of ports needed (from first part)
*
* @param[out] portsBmpPtr              - pointer to the ports bmp
*                                      COMMENTS :
*/
GT_VOID snetChtEgressGetPortsBmpFromMem
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32                 *memPtr,
    OUT SKERNEL_PORTS_BMP_STC *portsBmpPtr,
    IN GT_U32                  fillHalfPorts
);

/**
* @internal snetChtEgressPortsBmpOperators function
* @endinternal
*
* @brief   Egress Pipe :
*         BMP operators :
*         operator1 - operator on bmp 1
*         operator2 - operator on bmp 2
*         resultBmpPtr - the bmp for the result of the operators.
* @param[in] devObjPtr                - pointer to device object.
* @param[in] operator1                - operator on bmp1
* @param[in] bmp1Ptr                  - pointer to bmp 1
* @param[in] operator2                - operator on bmp2
* @param[in] bmp2Ptr                  - pointer to bmp 2
*
* @param[out] resultBmpPtr             - pointer to the ports bmp that is the 'result'
*                                      COMMENTS :
*/
GT_VOID snetChtEgressPortsBmpOperators
(
    IN SKERNEL_DEVICE_OBJECT        * devObjPtr,
    IN SKERNEL_BITWISE_OPERATOR_ENT operator1,
    IN SKERNEL_PORTS_BMP_STC        *bmp1Ptr,
    IN SKERNEL_BITWISE_OPERATOR_ENT operator2,
    IN SKERNEL_PORTS_BMP_STC        *bmp2Ptr,
    OUT SKERNEL_PORTS_BMP_STC       *resultBmpPtr
);

/**
* @internal snetChtEgressPortsBmpIsEmpty function
* @endinternal
*
* @brief   Egress Pipe : is BMP empty
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] bmpPtr                   - pointer to bmp to check
*
* @retval 1                        - empty
* @retval 0                        - not empty
*                                       COMMENTS :
*/
GT_BIT snetChtEgressPortsBmpIsEmpty
(
    IN SKERNEL_DEVICE_OBJECT        * devObjPtr,
    IN SKERNEL_PORTS_BMP_STC        *bmpPtr
);

/**
* @internal snetChtEgressFinalPacketAlteration function
* @endinternal
*
* @brief   do egress final packet modifications (after EPCL and EPLR)
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] descrPtr                 - pointer to the frame's descriptor.
* @param[in] egressPort               - egress port.
*/
void snetChtEgressFinalPacketAlteration
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN GT_U32  egressPort
);

/**
* @internal snetSip5_20EgfQagTcDpRemap function
* @endinternal
*
* @brief   SIP5_20 :
*         set TC and DP Remapping , for all traffic.
* @param[in] devObjPtr                - pointer to device object.
* @param[in] descrPtr                 - Cht frame descriptor
* @param[in] egressPort               - output port
*                                       COMMENTS :
*/
void snetSip5_20EgfQagTcDpRemap(

    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN GT_U32               egressPort
);
/**
* @internal snetChtEgfQagVidAssignment function
* @endinternal
*
* @brief   the TXQ per port (EPort) Assignment of Vid
*         relevant for EArch devices
*/
void snetChtEgfQagVidAssignment
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN GT_U32               egressPort
);

/**
* @internal snetChtEgfQagVlanTagModeGet function
* @endinternal
*
* @brief   SIP5 :
*         get the egress tag mode for the packet that egress the port.
* @param[in] devObjPtr                - pointer to device object.
* @param[in] descrPtr                 - Cht frame descriptor
*                                       the tag state of the egress port.
*                                       COMMENTS :
*/
GT_U32 snetChtEgfQagVlanTagModeGet
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN GT_U32               egressPort
);

/**
* @internal snetChtDma2Cpu function
* @endinternal
*
* @brief   Send frame to CPU by DMA
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] descrPtr                 - pointer to the frame's descriptor.
* @param[in] frameData                - pointer to the data
* @param[in] frameSize                - data size
*/
void snetChtDma2Cpu
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN GT_U8        *          frameData,
    IN GT_U32                  frameSize
);

/**
* @internal snetChtEgressCommandAndCpuCodeResolution function
* @endinternal
*
* @brief   1. resolve from old and current commands the new packet command
*         2. resolve from old and new commands the new packet cpu code (relevant
*         to copy that goes to CPU)
* @param[in] devObjPtr                - (pointer to) device object.
* @param[in,out] descrPtr                 - (pointer to) frame descriptor
* @param[in] prevCmd                  - previous command
* @param[in] currCmd                  - current command
* @param[in] prevCpuCode              - previous cpu code
* @param[in] currCpuCode              - current cpu code
* @param[in,out] descrPtr            - (pointer to) frame descriptor
*/

extern void snetChtEgressCommandAndCpuCodeResolution
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SKERNEL_FRAME_CHEETAH_DESCR_STC   * descrPtr,
    IN SKERNEL_EXT_PACKET_CMD_ENT prevCmd,
    IN SKERNEL_EXT_PACKET_CMD_ENT currCmd,
    IN GT_U32                     prevCpuCode,
    IN GT_U32                     currCpuCode
);

/**
* @internal snetHaSip5EgressTagAfter4BytesExtension function
* @endinternal
*
* @brief  set the egress tag extra bytes after the first 4 bytes
* @param[in] devObjPtr                - (pointer to) device object.
* @param[in] descrPtr                 - (pointer to) frame descriptor
* @param[in] haInfoPtr                - (pointer to) the HA internal info
* @param[in] isTag0                   - indication for tag0 or tag1
* @param[inout] egressTagPtr          - the (start of) buffer that hold egress tag0 or tag1
*                                       the first 4 bytes expected to hold info set by snetChtHaEgressTagDataExtGet
*/
void snetHaSip5EgressTagAfter4BytesExtension
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN HA_INTERNAL_INFO_STC *haInfoPtr,
    IN GT_BOOL      isTag0,
    INOUT GT_U8   *egressTagPtr
);

/**
* @internal snet6_30PreqSrfRestartDaemon function
* @endinternal
*
* @brief   PREQ : restart daemon for SRF (Sequence Recovery Function) for 802.1cb :
*           supported on SIP6.30 devices
*           The Restart Daemon scans the SRF Table (1K entries for Ironman L)
*           The Restart Daemon usually set to 10ms between scans.
*
* @param[in] devObjPtr                - pointer to device object.
*
*/
void snet6_30PreqSrfRestartDaemonTask
(
  IN SKERNEL_DEVICE_OBJECT           *devObjPtr
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif  /* __snetCheetahEgressh */



