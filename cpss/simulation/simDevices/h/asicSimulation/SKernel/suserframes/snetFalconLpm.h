/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* snetFalconLpm.h
*
* DESCRIPTION:
*       This is a external API definition for SIP6 LPM
*
* DEPENDENCIES:
*       None.
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*
*******************************************************************************/
#ifndef __snetFalconLpmh
#define __snetFalconLpmh

#include <asicSimulation/SKernel/smain/smain.h>
#include <asicSimulation/SKernel/suserframes/snetCheetah2Routing.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */



typedef enum{
    LPM_NODE_TYPE_EMPTY_E       = 0,
    LPM_NODE_TYPE_LEAF_E        = 1, 
    LPM_NODE_TYPE_REGULAR_E     = 2, 
    LPM_NODE_TYPE_COMPRESSED_E  = 3  
}LPM_NODE_TYPE_ENT;

typedef enum{
    LPM_LEAF_TYPE_REGULAR_E       = 0, /* Regular       */
    LPM_LEAF_TYPE_MULTIPATH_E     = 1  /* Multipath     */
}LPM_LEAF_TYPE_ENT;

typedef enum{
    LPM_MULTIPATH_MODE_ECMP_E     = 0,
    LPM_MULTIPATH_MODE_QOS_E      = 1
}LPM_MULTIPATH_MODE_ENT;

typedef enum{
    LPM_LEAF_PRIORITY_FDB_E       = 0,
    LPM_LEAF_PRIORITY_LPM_E       = 1
}LPM_LEAF_PRIORITY_ENT;

/* SIP 6.10 PBR-LPM Priority. Valid only when LPM Leaf isTrigger = 0 */
typedef enum{
    PBR_LPM_LEAF_PRIORITY_LPM_E       = 0,
    PBR_LPM_LEAF_PRIORITY_PBR_E       = 1
} PBR_LPM_LEAF_PRIORITY_ENT;
/*
 * Typedef: struct SNET_LPM_LTT_DATA_STC
 *
 * Fields:
 *         lttIpv6MulticastGroupScopeLevel      : Ipv6 Multicast Scope checking
 *         lttUnicastRpfCheckEnable             : If enabled the Unicast RPF CHECK performs
 *         lttUnicastSrcAddrCheckMismatchEnable : If enabled the Unicast MAC SIP/SA performs
*/
typedef struct{
    GT_U8   lttIpv6MulticastGroupScopeLevel;
    GT_BOOL lttUnicastRpfCheckEnable;
    GT_BOOL lttUnicastSrcAddrCheckMismatchEnable;
}SNET_SIP6_LPM_LTT_DATA_STC;

/*
 * Typedef: struct SNET_LPM_ECMP_OR_QOS_DATA_STC
 *
 * Fields:
 *         nextHopBaseAddr : The base address of the next hops that are associated to this ECMP/QoS block
 *         numOfPaths      : Number of ECMP or QoS paths.
 *                           Require for calculating the next HOP pointer.
 *                           The real <Number Of Paths> = <Number Of Paths>+1
 *         randomEn        : Define if to do random selection of the NH
*/
typedef struct{
    GT_U16  nextHopBaseAddr;
    GT_U16  numOfPaths;
    GT_BOOL randomEn;
    LPM_MULTIPATH_MODE_ENT mode;
}SNET_SIP6_LPM_ECMP_OR_QOS_DATA_STC;

/*
 * Typedef: struct SNET_LPM_MEM_LEAF_STC
 *
 * Fields:
 *         lttData      : LTT data struct
 *         activity     : activity bit is used for aging of the prefix.
 *                        It's described in SIP5 section
 *                        8.11.2 Refreshing the prefix activity bit.
 *         ecmpOrQosData: ecmp or qos data
 *                        (relevant only if pointerType == ecmp leaf or
 *                                          pointerType == qos  leaf)
*/
typedef struct {
    SNET_SIP6_LPM_LTT_DATA_STC          lttData;
    GT_BIT                              activity;
    SNET_SIP6_LPM_ECMP_OR_QOS_DATA_STC  ecmpOrQosData;
    LPM_LEAF_TYPE_ENT                   leafType;
    GT_U32                              nextHopPointer;
    GT_BOOL                             isTrigger; /* entry type */
    GT_BOOL                             isNextNodeCopmressed;
    LPM_LEAF_PRIORITY_ENT               priority;
    GT_BOOL                             applyPbr; /* SIP 6.10: Valid only when isTrigger = 0 */
} SNET_SIP6_LPM_MEM_LEAF_STC;

/*
 * Typedef: struct SNET_LPM_MEM_ENTRY_STC
 *
 * Fields:
 *         nextPtr    : next pointer, used for all node types
 *         pointerType: lpm mem node type 
 *         leafNumber : leaf number
*/
typedef struct {
    GT_U32                      nextPtr;
    LPM_NODE_TYPE_ENT           nodeType;
    GT_U32                      leafNumber;
} SNET_SIP6_LPM_MEM_ENTRY_STC;

/*******************************************************************************
*   snetFalconLpm
*
* DESCRIPTION:
*        Falcon LPM lookup function
*
* INPUTS:
*        devObjPtr - pointer to device object.
*        descrPtr  - pointer to the frame's descriptor.
*
* OUTPUTS:
*        matchIndexPtr         - array to indicate dip sip match
*        priorityPtr           - routing priority between FDB and LPM lookup
*        isPbrPtr              - pointer to isPbr with values
*                                  GT_TRUE  - Policy Based Routing
*                                  GT_FALSE - LPM lookup
*        ipSecurChecksInfoPtr  - routing security checks information
*
* RETURNS:
*       whether sip lookup was performed
*
*******************************************************************************/
GT_U32 snetFalconLpm
(
    IN  SKERNEL_DEVICE_OBJECT               *devObjPtr,
    IN  SKERNEL_FRAME_CHEETAH_DESCR_STC     *descrPtr,
    OUT GT_U32                              *matchIndexPtr,
    OUT GT_BOOL                             *isPbrPtr,
    OUT LPM_LEAF_PRIORITY_ENT               *priorityPtr,
    OUT SNET_ROUTE_SECURITY_CHECKS_INFO_STC *ipSecurChecksInfoPtr
);

/*******************************************************************************
*   snetSip6FetchRouteEntry
*
* DESCRIPTION:
*       process sip/dip ecmp or qos leafs (if any)
*       and update matchIndex array (if nessecary)
*
* INPUTS:
*        devObjPtr     - pointer to device object.
*        descrPtr      - pointer to the frame's descriptor.
*        dipLpmDataPtr - pointer to dip leaf entry structure
*        sipLpmDataPtr - pointer to sip leaf entry structure
*        matchIndexPtr - array to indicate dip sip match
*
* OUTPUTS:
*        matchIndexPtr         - array to indicate dip sip match
*
*******************************************************************************/
GT_VOID snetSip6FetchRouteEntry
(
    IN    SKERNEL_DEVICE_OBJECT               *devObjPtr,
    IN    SKERNEL_FRAME_CHEETAH_DESCR_STC     *descrPtr,
    IN    SNET_SIP6_LPM_MEM_LEAF_STC          *dipLpmDataPtr,
    IN    SNET_SIP6_LPM_MEM_LEAF_STC          *sipLpmDataPtr,
    INOUT GT_U32                              *matchIndexPtr
);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif  /* __snetFalconLpmh */









