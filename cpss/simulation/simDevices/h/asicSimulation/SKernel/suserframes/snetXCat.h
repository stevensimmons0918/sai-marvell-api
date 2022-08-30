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
* @file snetXCat.h
*
* @brief This is a external API definition for snet xCat module of SKernel.
*
* @version   17
********************************************************************************
*/
#ifndef __snetXCath
#define __snetXCath


#include <asicSimulation/SKernel/smain/smain.h>
#include <common/Utils/FrameInfo/sframeInfoAddr.h>
#include <asicSimulation/SKernel/suserframes/snetCheetahL2.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*  Struct : SNET_XCAT_LOGICAL_PORT_MAPPING_STC

    Description:
            Logical Port Mapping Entry used for mapping a packets logical
            target to an actual egress interface.

    egressIf        - Actual egress interface.
    tunnelStart     - Indicates that the packet should be sent on a tunnel.
    tunnelPtr       - Tunnel pointer in case that the packet should be tunneled.
    tunnelPassengerType
                    - Type of passenger packet being to be encapsulated.

new fields for devObjPtr->supportLogicalMapTableInfo.tableFormatVersion = 1
    assignVid0Command :
        0 = Do not override VID0 assignment
        1 = Override VID0 assignment only if packet arrives without Tag0
        2 = Always override VID0 assignment
        3 = Reserved
    egressVlanFilteringEnable :
        If set, apply Egress VLAN filtering according to LP Egress VLAN member table configuration

    assignedEgressTagStateOnlyIfUnassigned:
        Needed to AC->AC traffic.  If packet arrived from PW, then it is already assigned

    egressTagState :
        same enum as VLAN table

    vid0:
        Relevant only if <Assign VID0 Enable> is set. The new VID0 assignment

*/
typedef struct {
    SNET_DST_INTERFACE_STC egressIf;
    GT_U32          tunnelStart;
    GT_U32          tunnelPtr;
    GT_U32          tunnelPassengerType;

    /* new fields for devObjPtr->supportLogicalMapTableInfo.tableFormatVersion = 1 */
    GT_U32  assignVid0Command;        /*1 bit*/
    GT_U32  egressVlanFilteringEnable;/*1 bit*/
    GT_U32  assignedEgressTagStateOnlyIfUnassigned;/*1 bit*/
    GT_U32  egressTagState;/*3 bits*/
    GT_U32  vid0;/*12 bits*/
    GT_U32  egressPassangerTagTpidIndex;/*2 bits*/

}SNET_XCAT_LOGICAL_PORT_MAPPING_STC;

/**
* @internal snetXCatIngressVlanTagClassify function
* @endinternal
*
* @brief   Ingress Tag0 VLAN and Tag1 VLAN classification
*/
GT_VOID snetXCatIngressVlanTagClassify
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN GT_U32 ethTypeOffset,
    OUT GT_U32 * inVlanEtherType0Ptr,
    OUT GT_U32 * inVlanEtherType1Ptr,
    IN SNET_CHT_FRAME_PARSE_MODE_ENT   parseMode
);

/**
* @internal snetXCatHaEgressTagBuild function
* @endinternal
*
* @brief   HA - Build Tag0 VLAN and Tag1 VLAN according to Tag state
*/
GT_STATUS snetXCatHaEgressTagBuild
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    INOUT HA_INTERNAL_INFO_STC  *haInfoPtr,
    IN GT_U32  vlanTag0EtherType,
    IN GT_U32  vlanTag1EtherType,
    IN DSA_TAG_TYPE_E egrMarvellTag,
    IN GT_U32  destVlanTagged,
    OUT GT_U8 * tagDataPtr,
    INOUT GT_U32 * tagDataLengthPtr,
    OUT GT_U32  *tag0OffsetInTagPtr,
    OUT GT_U32  *tag1OffsetInTagPtr
);

/**
* @internal snetXCatHaEgressTagEtherType function
* @endinternal
*
* @brief   HA - Egress Tag0 VLAN and Tag1 VLAN ethertype assignment
*/
GT_STATUS snetXCatHaEgressTagEtherType
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN GT_U32 egressPort,
    INOUT HA_INTERNAL_INFO_STC  *haInfoPtr,
    OUT GT_U32 * egrTag0EtherTypePtr,
    OUT GT_U32 * egrTag1EtherTypePtr,
    OUT GT_U32 * tunnelStartEtherTypePtr
);

/**
* @internal snetXCatHaEgressTagEtherTypeByTpid function
* @endinternal
*
* @brief   HA - Egress get EtherType according to TPID
*/
GT_VOID snetXCatHaEgressTagEtherTypeByTpid
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN GT_U32   tpId,
    INOUT GT_U32 * etherTypePtr,
    INOUT GT_U32 *tagSizePtr,
    IN GT_BOOL  isForTag1
);


/**
* @internal snetXCatTxQPortIsolationFilters function
* @endinternal
*
* @brief   Port isolation filtering
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] descrPtr                 - pointer to the frame's descriptor.
* @param[in,out] destPorts[]              - number of egress port.
*/
GT_VOID snetXCatTxQPortIsolationFilters
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    INOUT GT_U32 destPorts[]
);


/**
* @internal snetXCatEqSniffFromRemoteDevice function
* @endinternal
*
* @brief   Forwarding TO_ANALYZER frames to the Rx/Tx Sniffer.
*/
GT_VOID snetXCatEqSniffFromRemoteDevice
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN GT_BOOL rxSniff
);

/**
* @internal snetXCatFdbSrcIdAssign function
* @endinternal
*
* @brief   Source-ID Assignment
*/
GT_U32 snetXCatFdbSrcIdAssign
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN SNET_CHEETAH_L2I_SA_LOOKUP_INFO * saLookupInfoPtr,
    IN SNET_CHEETAH_L2I_DA_LOOKUP_INFO * daLookupInfoPtr
);

/**
* @internal snetXCatLogicalTargetMapping function
* @endinternal
*
* @brief   The device supports a generic mechanism that maps a packets
*         logical target to an actual egress interface
*         The logical target can me mapped to any of the following new targets:
*         - Single-target (Device, Port)
*         - Single-Target (Device, Port) + Tunnel-Start Pointer
*         - Trunk-ID
*         - Multi-target (VIDX)
* @param[in] devObjPtr                - pointer to device object.
* @param[in] descrPtr                 - pointer to the frame's descriptor.
*/
GT_VOID snetXCatLogicalTargetMapping
(
    IN SKERNEL_DEVICE_OBJECT            * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC  * descrPtr
);

/**
* @internal snetXcatIngressMirrorAnalyzerIndexSelect function
* @endinternal
*
* @brief   The device supports multiple analyzers. If a packet is mirrored by
*         both the port-based ingress mirroring mechanism, and one of the other
*         ingress mirroring mechanisms, the selected analyzer
*         is the one with the higher index in the analyzer table
* @param[in] devObjPtr                - pointer to device object.
* @param[in] descrPtr                 - pointer to the frame's descriptor.
* @param[in] newAnalyzerIndex         - new analyzer index
*/
GT_VOID snetXcatIngressMirrorAnalyzerIndexSelect
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr ,
    IN GT_U32                   newAnalyzerIndex
);

/**
* @internal snetXcatEgressMirrorAnalyzerIndexSelect function
* @endinternal
*
* @brief   The device supports multiple analyzers. support Egress selection
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] descrPtr                 - pointer to the frame's descriptor.
* @param[in] newAnalyzerIndex         - new analyzer index
*/
GT_VOID snetXcatEgressMirrorAnalyzerIndexSelect
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr ,
    IN GT_U32                   newAnalyzerIndex
);

/**
* @internal snetXCatVlanTagMatchWithoutTag0Tag1Classification function
* @endinternal
*
* @brief   Check Ingress Global TPID table for packet ethernet type matching ,
*         without changing descrPtr->tpidIndex[tagIndex] !
* @param[in] devObjPtr                - pointer to device object
* @param[in] descrPtr                 - pointer to the frame's descriptor.
* @param[in] ethTypeOffset            - offset of packet ethernet type
* @param[in] portTpIdBmp              - port TPID bitmap
* @param[in] tagIndex                 - tag index :
*                                      0 - tag 0
*                                      1 - tag 1
*                                      2.. - extra tags
* @param[in,out] tagSizePtr               -  size of tag extended, NULL means not used (sip5 only)
*                                      RETURN:
*/
GT_BOOL snetXCatVlanTagMatchWithoutTag0Tag1Classification
(
    IN    SKERNEL_DEVICE_OBJECT           *devObjPtr,
    IN    SKERNEL_FRAME_CHEETAH_DESCR_STC *descrPtr ,
    IN    GT_U32                           ethTypeOffset,
    IN    GT_U32                           portTpIdBmp,
    IN    GT_U32                           tagIndex,
    INOUT GT_U32                           *tagSizePtr
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif  /* __snetXCath */



