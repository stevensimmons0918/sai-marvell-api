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
* @file snetLion.h
*
* @brief This is a external API definition for snet Lion module of SKernel.
*
* @version   21
********************************************************************************
*/
#ifndef __snetLionh
#define __snetLionh


#include <asicSimulation/SKernel/smain/smain.h>
#include <common/Utils/FrameInfo/sframeInfoAddr.h>
#include <asicSimulation/SKernel/suserframes/snetCheetahL2.h>
#include <asicSimulation/SKernel/suserframes/snetCheetah3TTermination.h>
#include <asicSimulation/SKernel/suserframes/snetXCatPcl.h>
#include <asicSimulation/SKernel/suserframes/snetCheetahEgress.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define SNET_LION_CRC_HEADER_HASH_BYTES_CNS                     70

/* Relevant for sip 6,  for non vlan EGF sht tables with port bit map entry.
   This determines the offset of the next line in the entry for ports modes 256, 512, 1024.
*/
#define SHT_NON_VLAN_TABLE_SIP6_CHUNK_SIZE 256
/* Relevant for sip 6,  for vlan EGF sht tables with port bit map entry.
   This determines the offset of the next line in the entry for ports modes 256, 512, 1024
*/
#define SHT_VLAN_TABLE_SIP6_CHUNK_SIZE 1024
/* Relevant for sip 6,  for EGF tables with port bit map entry.
   This is the line width. Each line support 128 ports.
*/
#define SHT_TABLE_SIP6_LINE_WIDTH 128


/* Convert port number to TOD group to use */
/* {0-15,16-31,32-47,48-51,52-55,56-59,60-63,64-67,68-71}->{0,1,2,3,4,5,6,7,8} */
#define SNET_LION3_PORT_NUM_TO_TOD_GROUP_CONVERT_MAC(dev,portNum)           \
    ((dev->supportSingleTai == 0) ?                                         \
        (((portNum) < 48) ? ((portNum)/0x10) : ((portNum - 48)/4)) :        \
        0)

/**
* @enum SNET_LION_CRC_HASH_INPUT_KEY_FIELDS_ID_ENT
*/
typedef enum{
    SNET_LION_CRC_HASH_L4_TARGET_PORT_E,
    SNET_LION_CRC_HASH_L4_SOURCE_PORT_E,
    SNET_LION_CRC_HASH_IPV6_FLOW_E,
    SNET_LION_CRC_HASH_RESERVED_55_52_E,
    SNET_LION_CRC_HASH_IP_DIP_3_E,
    SNET_LION_CRC_HASH_IP_DIP_2_E,
    SNET_LION_CRC_HASH_IP_DIP_1_E,
    SNET_LION_CRC_HASH_IP_DIP_0_E,
    SNET_LION_CRC_HASH_IP_SIP_3_E,
    SNET_LION_CRC_HASH_IP_SIP_2_E,
    SNET_LION_CRC_HASH_IP_SIP_1_E,
    SNET_LION_CRC_HASH_IP_SIP_0_E,
    SNET_LION_CRC_HASH_MAC_DA_E,
    SNET_LION_CRC_HASH_MAC_SA_E,
    SNET_LION_CRC_HASH_MPLS_LABEL0_E,
    SNET_LION_CRC_HASH_RESERVED_431_428_E,
    SNET_LION_CRC_HASH_MPLS_LABEL1_E,
    SNET_LION_CRC_HASH_RESERVED_455_452_E,
    SNET_LION_CRC_HASH_MPLS_LABEL2_E,
    SNET_LION_CRC_HASH_RESERVED_479_476_E,
    SNET_LION_CRC_HASH_LOCAL_SOURCE_PORT_E,
    SNET_LION_CRC_HASH_UDB_14_TO_22_E,
    /*Lion3 only*/
    SNET_LION3_CRC_HASH_UDB_0_TO_11_E,
    SNET_LION3_CRC_HASH_UDB_23_TO_34_E,
    SNET_LION3_CRC_HASH_EVID_E,
    SNET_LION3_CRC_HASH_ORIG_SRC_EPORT_OR_TRNK_E,
    SNET_LION3_CRC_HASH_LOCAL_DEV_SRC_EPORT_E,

    SNET_LION_CRC_HASH_LAST_E,


}SNET_LION_CRC_HASH_INPUT_KEY_FIELDS_ID_ENT;

/**
 *  Typedef struct : SNET_LION_PCL_ACTION_STC
 *
 *  Description :
 *              The policy engine maintains an 1024 entries table,
 *              corresponding to the 1024 rules that may be defined in the
 *              TCAM lookup structure. The line index of the matching rule
 *              is used to index the policy action table and extract the
 *              action to perform.
 *
*/
typedef struct {
    SNET_XCAT_PCL_ACTION_STC baseAction;
    GT_BIT modifyMacDa;
    GT_BIT modifyMacSa;
}SNET_LION_PCL_ACTION_STC;

/**
* @enum SNET_LION_PTP_TOD_EVENT_ENT
*/
typedef enum{

    SNET_LION_PTP_TOD_UPDATE_E,

    SNET_LION_PTP_TOD_INCREMENT_E,

    SNET_LION_PTP_TOD_CAPTURE_E,

    SNET_LION_PTP_TOD_GENERATE_E

} SNET_LION_PTP_TOD_EVENT_ENT;

/**
* @enum SNET_LION3_PTP_TOD_FUNC_ENT
*/
typedef enum{

    SNET_LION3_PTP_TOD_UPDATE_E,

    SNET_LION3_PTP_TOD_FREQ_UPDATE_E,

    SNET_LION3_PTP_TOD_INCREMENT_E,

    SNET_LION3_PTP_TOD_DECREMENT_E,

    SNET_LION3_PTP_TOD_CAPTURE_E,

    SNET_LION3_PTP_TOD_GRACEFUL_INC_E,

    SNET_LION3_PTP_TOD_GRACEFUL_DEC_E,

    SNET_LION3_PTP_TOD_NOP_E

} SNET_LION3_PTP_TOD_FUNC_ENT;

/**
* @enum SNET_LION_PTP_GTS_INTERRUPT_EVENTS_ENT
*/
typedef enum{

    SNET_LION_PTP_GTS_INTERRUPT_GLOBAL_FIFO_FULL_E,

    SNET_LION_PTP_GTS_INTERRUPT_VALID_ENTRY_E

} SNET_LION_PTP_GTS_INTERRUPT_EVENTS_ENT;

/**
* @internal snetLionEqInterPortGroupRingFrwrd function
* @endinternal
*
* @brief   Perform Inter-Port Group Ring Forwarding
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] descrPtr                 - pointer to the frame's descriptor.
*                                      OUTPUT:
*
* @note A packet is subject to inter-port group ring forwarding if ALL the following conditions are TRUE:
*       - The Multi-port group Lookup feature is globally enabled
*       - The Bridge FDB DA lookup did not find a match
*       - If the packet is ingressed on a local port group ring port, the ingress port
*       is configured as a multi-port group ring port.
*       OR
*       - If the packet is ingressed on a local port group network port, the ingress port is
*       enabled for multi-port group lookup.
*
*/
GT_VOID snetLionEqInterPortGroupRingFrwrd
(
    IN    SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN    SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr
);

/**
* @internal snetLionL2iPortGroupMaskUnknownDaEnable function
* @endinternal
*
* @brief   Apply/Deny VLAN Unknown/Unregistered commands to unknown/unregistered packets.
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] descrPtr                 - pointer to the frame's descriptor.
*                                      OUTPUT:
*                                      RETURN:
*                                      GT_TRUE - apply VLAN Unknown/Unregistered commands to unknown/unregistered packets
*                                      GT_FALSE - deny VLAN Unknown/Unregistered commands to unknown/unregistered packets
*
* @note The Bridge engine in each port group supports a set of per-VLAN Unknown/Unregistered commands
*       that can override the default flooding behavior. However, these commands
*       are only applied to unknown/unregistered packets if the bridge engine
*       resides in the last port group of the respective ring.
*
*/
GT_BOOL snetLionL2iPortGroupMaskUnknownDaEnable
(
    IN    SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN    SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr
);

/**
* @internal snetLionIngressSourcePortGroupIdGet function
* @endinternal
*
* @brief   Assign descriptor source port group ID - port group from which
*         the packet ingress the device
* @param[in] devObjPtr                - pointer to device object
* @param[in,out] descrPtr                 - pointer to the frame's descriptor
*                                      OUTPUT:
*
* @note Source port group ID may be either the local port group Id (when ingress port is not ring port),
*       or a remote port group Id (when ingress port is ring port and value parsed from the DSA)
*       The function should be called straight after DSA tag parsing.
*
*/
GT_VOID snetLionIngressSourcePortGroupIdGet
(
    IN    SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr
);

/**
* @internal snetLionHaEgressMarvellTagSourcePortGroupId function
* @endinternal
*
* @brief   HA - Overrides DSA tag <routed> and <egress filtered registered> bits with
*         Source port group ID.
* @param[in] devObjPtr                - pointer to device object
* @param[in,out] descrPtr                 - pointer to the frame's descriptor
* @param[in] egressPort               - cascade port to which traffic forwarded
* @param[in,out] mrvlTagPtr               - pointer to marvell tag
*                                      OUTPUT:
* @param[in,out] mrvlTagPtr               - pointer to marvell tag
*
* @note Source port group ID may be either the local port group Id (when ingress port is not ring port),
*       or a remote port group Id (when ingress port is ring port and value parsed from the DSA)
*       It is passed in the DSA tag <routed> and <egress filtered registered> bits.
*       The function should be called straight after Marvell Tag creation.
*
*/
GT_VOID snetLionHaEgressMarvellTagSourcePortGroupId
(
    IN  SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN  GT_U32  egressPort,
    INOUT GT_U8 * mrvlTagPtr
);

/**
* @internal snetLionL2iGetIngressVlanInfo function
* @endinternal
*
* @brief   Get Ingress VLAN Info
*/
GT_VOID snetLionL2iGetIngressVlanInfo
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    OUT SNET_CHEETAH_L2I_VLAN_INFO * vlanInfoPtr
);

/**
* @internal snetLionTxQGetEgressVidxInfo function
* @endinternal
*
* @brief   Get egress VLAN and STP information
*
* @param[out] destPorts[]              - destination ports.
* @param[out] destVlanTagged[]         - destination vlan tagged.
* @param[out] stpVector[]              - stp ports vector.
*/
GT_VOID snetLionTxQGetEgressVidxInfo
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN SKERNEL_EGR_PACKET_TYPE_ENT packetType,
    OUT GT_U32 destPorts[],
    OUT GT_U8 destVlanTagged[],
    OUT SKERNEL_STP_ENT stpVector[]
);

/**
* @internal snetLionTxQGetTrunkDesignatedPorts function
* @endinternal
*
* @brief   Trunk designated ports bitmap
*
* @param[out] designatedPortsBmpPtr    - pointer to designated port bitmap
*/
GT_VOID snetLionTxQGetTrunkDesignatedPorts
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN TRUNK_HASH_MODE_ENT    trunkHashMode,
    OUT SKERNEL_PORTS_BMP_STC * designatedPortsBmpPtr
);

/**
* @internal snetLionTxQGetDeviceMapTableAddress function
* @endinternal
*
* @brief   Get table entry address from Device Map table
*         according to source/target device and source/target port
* @param[in] devObjPtr                - pointer to device object.
* @param[in] descrPtr                 - pointer to the frame's descriptor.
* @param[in] trgDev                   - target device
* @param[in] trgPort                  - target port
* @param[in] srcDev                   - source device
* @param[in] srcPort                  - source port
*                                       Device Map Entry address
*
* @retval value                    - SMAIN_NOT_VALID_CNS --> meaning : indication that the device
*                                       map table will not be accessed
*/
GT_U32 snetLionTxQGetDeviceMapTableAddress
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN GT_U32 trgDev,
    IN GT_U32 trgPort,
    IN GT_U32 srcDev,
    IN GT_U32 srcPort
);

/**
* @internal snetLionCrcBasedTrunkHash function
* @endinternal
*
* @brief   CRC Based Hash Index Generation Procedure
*/
GT_VOID snetLionCrcBasedTrunkHash
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr
);

/**
* @internal snetLionCutThroughTrigger function
* @endinternal
*
* @brief   Cut-Through Mode Triggering
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in,out] descrPtr                 - pointer to frame data buffer Id.
*                                      OUTPUT:
* @param[in,out] descrPtr                 - pointer to frame data buffer Id.
*                                      RETURN:
*
* @note Cut-through mode can be enabled per source port and priority.
*       The priority in this context is the user priority field in the VLAN tag:
*       - A packet is identified as VLAN tagged for cut-through purposes
*       if its Ethertype (TPID) is equal to one of two configurable VLAN Ethertypes.
*       - For cascade ports, the user priority is taken from the DSA tag.
*
*/
GT_VOID snetLionCutThroughTrigger
(
    IN SKERNEL_DEVICE_OBJECT                * devObjPtr,
    INOUT SKERNEL_FRAME_CHEETAH_DESCR_STC   * descrPtr
);

/**
* @internal snetLionHaKeepVlan1Check function
* @endinternal
*
* @brief   Ha - check for forcing 'keeping VLAN1' in the packet even if need to
*         egress without tag1 (when ingress with tag1)
* @param[in] devObjPtr                - pointer to device object.
* @param[in,out] descrPtr                 - pointer to frame data buffer Id.
* @param[in] egressPort               - egress port
*                                      OUTPUT:
*                                      none
*                                      RETURN:
*
* @note Enable keeping VLAN1 in the packet, for packets received with VLAN1 and
*       even-though the tag-state of this (egress-port, VLAN0) is configured in
*       the VLAN-table to (untagged) or (VLAN0).
*
*/
GT_VOID snetLionHaKeepVlan1Check
(
    IN SKERNEL_DEVICE_OBJECT                * devObjPtr,
    INOUT SKERNEL_FRAME_CHEETAH_DESCR_STC   * descrPtr,
    IN GT_U32   egressPort
);

/**
* @internal snetLionTxqDeviceMapTableAccessCheck function
* @endinternal
*
* @brief   Check if need to access the Device map table.
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] descrPtr                 - pointer to the frame's descriptor.
* @param[in] srcPort                  - the source port
*                                      Global for port group shared device.
*                                      Local(the same value as localDevSrcPort)
*                                      for non-port group device.
* @param[in] trgDev                   - the target device
* @param[in] trgPort                  - the target port
*
* @retval GT_TRUE                  - device map need to be accessed
* @retval GT_FALSE                 - device map NOT need to be accessed
*
* @note from the FS:
*       The device map table can also be accessed when the target device is
*       the local device.
*       This capability is useful in some modular systems, where the device
*       is used in the line card (see Figure 164).
*       In some cases it is desirable for incoming traffic from the network
*       ports to be forwarded through the fabric when the target device is
*       the local device. This is done by performing a device map lookup for
*       the incoming upstream traffic.
*       Device map lookup for the local device is based on a per-source-port
*       and per-destination-port configuration. The device map table is thus
*       accessed if one of the following conditions holds:
*       1. The target device is not the local device.
*       2. The target device is the local device and both of the following
*       conditions hold:
*       A. The local source port is enabled for device map lookup for
*       local device according to the per-source-port configuration.
*       B. The target1 port is enabled for device map lookup for local
*       device according to the per-destination-port configuration.
*
*/
GT_BOOL snetLionTxqDeviceMapTableAccessCheck
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN GT_U32 srcPort,
    IN GT_U32 trgDev,
    IN GT_U32 trgPort
);

/**
* @internal snetLionResourceHistogramCount function
* @endinternal
*
* @brief   Gathering information about the resource utilization of the Transmit
*         Queues
* @param[in] devObjPtr                - pointer to device object.
* @param[in,out] descrPtr                 - pointer to frame data buffer Id.
*                                      OUTPUT:
*                                      RETURN:
*
* @note For every successful packet enqueue, if the Global Descriptors Counter
*       exceeds the Threshold(n), the Histogram Counter(n) is incremented by 1.
*
*/
GT_VOID snetLionResourceHistogramCount
(
    IN SKERNEL_DEVICE_OBJECT                * devObjPtr,
    INOUT SKERNEL_FRAME_CHEETAH_DESCR_STC   * descrPtr
);

/**
* @internal snetLionPtpIngressTimestampProcess function
* @endinternal
*
* @brief   Ingress PTP Timestamp Processing
*/
GT_VOID snetLionPtpIngressTimestampProcess
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr
);

/**
* @internal snetLionHaPtpEgressTimestampProcess function
* @endinternal
*
* @brief   Ha - Egress PTP Timestamp Processing
*/
GT_VOID snetLionHaPtpEgressTimestampProcess
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN GT_U32 egressPort
);

/**
* @internal snetLionPtpCommandResolution function
* @endinternal
*
* @brief   PTP Trapping/Mirroring to the CPU
*/
GT_VOID snetLionPtpCommandResolution
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr
);

/**
* @internal snetLionPtpTodCounterApply function
* @endinternal
*
* @brief   TOD Counter Functions
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] direction                - ingress or egress direction
* @param[in] todEvent                 - TOD counter event type
*                                      RETURN:
*
* @note The TOD counter supports four types of time-driven and time-setting
*       functions — Update, Increment, Capture, and Generate.
*       These functions use a shadow register, which has exactly the same format
*       as the TOD counter
*
*/
GT_VOID snetLionPtpTodCounterApply
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SMAIN_DIRECTION_ENT direction,
    IN SNET_LION_PTP_TOD_EVENT_ENT todEvent
);

/**
* @internal snetLionPtpTodCounterSecondsRead function
* @endinternal
*
* @brief   Read TOD counter seconds
*/
GT_VOID snetLionPtpTodCounterSecondsRead
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SMAIN_DIRECTION_ENT direction,
    OUT GT_U64 * seconds64
);

/**
* @internal snetLionPtpTodCounterNanosecondsRead function
* @endinternal
*
* @brief   Read TOD counter nanoseconds
*/
GT_VOID snetLionPtpTodCounterNanosecondsRead
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SMAIN_DIRECTION_ENT direction,
    OUT GT_U32 * nanoseconds
);

/**
* @internal snetLion3PtpTodCounterApply function
* @endinternal
*
* @brief   TOD Counter Functions
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] todEvent                 - TOD counter event type
* @param[in] taiGroup                 - The TAI selected group - one of 9
* @param[in] taiInst                  - within the selected group - TAI0 or TAI1
*                                      RETURN:
*
* @note The functions are based on shadow\capture registers which have the
*       same format as the TOD counter
*
*/
GT_VOID snetLion3PtpTodCounterApply
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SNET_LION3_PTP_TOD_FUNC_ENT todEvent,
    IN GT_U32 taiGroup,
    IN GT_U32 taiInst
);

/**
* @internal snetLionPclUdbKeyValueGet function
* @endinternal
*
* @brief   Get user defined value by user defined key.
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] descrPtr                 - pointer to frame data buffer Id.
* @param[in] direction                - ingress or egress direction
*                                      udbType             - UDB type
* @param[in] udbIdx                   - UDB index in UDB configuration entry.
*
* @param[out] byteValuePtr             - pointer to UDB value.
*                                      RETURN:
*                                      GT_OK - OK
*                                      GT_FAIL - Not valid byte
*                                      GT_BAD_SIZE - In case policy key field
*                                                   cannot be extracted from the packet
*                                                   due to lack of header depth
*                                      COMMENTS:
*/
GT_STATUS snetLionPclUdbKeyValueGet
(
    IN SKERNEL_DEVICE_OBJECT                        * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC              * descrPtr,
    IN SMAIN_DIRECTION_ENT                            direction,
    IN GT_U32                                         udbIdx,
    OUT GT_U8                                       * byteValuePtr
);

/**
* @internal snetLion3PtpTodGetTimeCounter function
* @endinternal
*
* @brief   Get the TOD content
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] taiGroup                 - The TAI selected group - one of 9
* @param[in] taiInst                  - within the selected group - TAI0 or TAI1
*                                      OUTPUT:
* @param[in] timeCounterPtr           - (pointer to) the TOD contetnt
*                                      RETURN:
*/
GT_VOID snetLion3PtpTodGetTimeCounter
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32 taiGroup,
    IN GT_U32 taiInst,
    OUT SNET_TOD_TIMER_STC *timeCounterPtr
);

/**
* @internal snetlion3TimestampQueueRemoveEntry function
* @endinternal
*
* @brief   Remove entry from timestamp queue (due to CPU read)
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] direction                - ingress or egress.
* @param[in] queueNum                 - 0 or 1.
*                                      queueEntryPtr - pointer to queue entry data.
*                                      OUTPUT:
*                                      None
*/
GT_VOID snetlion3TimestampQueueRemoveEntry
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SMAIN_DIRECTION_ENT   direction,
    IN GT_U32 queueNum
);

/**
* @internal snetlion3TimestampPortEgressQueueRemoveEntry function
* @endinternal
*
* @brief   Remove entry from timestamp port egress queue (due to CPU read)
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] macPort                  - Egress MAC port.
* @param[in] queueNum                 - 0 or 1.
*                                      OUTPUT:
*                                      None
*/
GT_VOID snetlion3TimestampPortEgressQueueRemoveEntry
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32 macPort,
    IN GT_U32 queueNum
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif  /* __snetLionh */



