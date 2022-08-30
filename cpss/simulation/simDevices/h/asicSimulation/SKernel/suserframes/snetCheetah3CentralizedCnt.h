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
* @file snetCheetah3CentralizedCnt.h
*
* @brief Cheetah3 Asic Simulation
* Centralized Counter Unit.
* Header file.
*
* @version   10
********************************************************************************
*/
#ifndef __snetCheetah3CentralizedCnt
#define __snetCheetah3CentralizedCnt

#include <asicSimulation/SKernel/smain/smain.h>
#include <asicSimulation/SKernel/smem/smem.h>
#include <os/simEnvDepTypes.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define SNET_CNC_PREQ_TRIGGER_BIT_INDEX_CNC 13
#define SNET_CNC_COPY_RESERVED_FLOW_TRACK_ENABLE_BIT_CNS 1

/**
 *  Enum: INTERNAL_SPECIFIC_HW_VALUES__SNET_CHT3_CNC_CLIENT_E
 *
 *  Description:
 *      Client number enumeration.
 *
 *  Fields:
 *
 *      ch3 clients:
 *   client 0 - L2/L3 Ingress VLAN
 *   client 1 - Ingress PCL0 lookup 0 (out of 2 dual lookups)
 *   client 2 - Ingress PCL0 lookup 1 (out of 2 dual lookups)
 *   client 3 - Ingress PCL1
 *   client 4 - Ingress VLAN Pass/Drop
 *   client 5 - Egress VLAN Pass/Drop
 *   client 6 - Egress Queue Pass/Drop
 *   client 7 - Egress PCL
 *   client 8 - ARP Table access
 *   client 9 - Tunnel Start
 *   client 10 - Tunnel Termination (index of 'hit' by TTI action)
 *
 *      sip5 clients:
 *   Client 0-1 - Tunnel Termination Interface, 2 parallel lookup clients
 *   Client 2-5- Ingress PCL_0, 4 parallel lookup clients
 *   Client 6-9 - Ingress PCL_1, 4 parallel lookup clients
 *   Client 10-13- Ingress PCL_2, 4 parallel lookup clients
 *   Client 14 - L2/L3 Ingress eVLAN
 *   Client 15 - source ePort
 *   Client 16- Ingress eVLAN Pass/Drop
 *   Client 17- packet type Pass/Drop
 *   Client 18- Egress eVLAN Pass/Drop
 *   Client 19- Egress Queue Pass/Drop and QCN Pass/Drop counters
 *   Client 20- ARP Table access
 *   Client 21 - Tunnel-Start
 *   Client 22 - target ePort
 *   Client 23-26 - Egress PCL, 4 parallel lookup clients
 *   Client 27 - TM Pass/Drop unit
 *
 *  Comments:
 *      To bind a counter block to a client, enable the relevant client number
 *      for each required block.
 **/
typedef enum {
    INTERNAL_SPECIFIC_HW_VALUES__SNET_CHT3_CNC_CLIENT_L2_L3_VLAN_INGR_E = 0,
    INTERNAL_SPECIFIC_HW_VALUES__SNET_CHT3_CNC_CLIENT_PCL0_0_LOOKUP_INGR_E,
    INTERNAL_SPECIFIC_HW_VALUES__SNET_CHT3_CNC_CLIENT_PCL0_1_LOOKUP_INGR_E,
    INTERNAL_SPECIFIC_HW_VALUES__SNET_CHT3_CNC_CLIENT_PCL1_LOOKUP_INGR_E,
    INTERNAL_SPECIFIC_HW_VALUES__SNET_CHT3_CNC_CLIENT_VLAN_PASS_DROP_INGR_E,
    INTERNAL_SPECIFIC_HW_VALUES__SNET_CHT3_CNC_CLIENT_VLAN_PASS_DROP_EGR_E,
    INTERNAL_SPECIFIC_HW_VALUES__SNET_CHT3_CNC_CLIENT_TXQ_QUEUE_PASS_DROP_EGR_E,
    INTERNAL_SPECIFIC_HW_VALUES__SNET_CHT3_CNC_CLIENT_PCL_EGR_E,
    INTERNAL_SPECIFIC_HW_VALUES__SNET_CHT3_CNC_CLIENT_ARP_TBL_E,
    INTERNAL_SPECIFIC_HW_VALUES__SNET_CHT3_CNC_CLIENT_TUNNEL_START_E,
    INTERNAL_SPECIFIC_HW_VALUES__SNET_XCAT_CNC_CLIENT_TUNNEL_TERMINATION_E,

    /* -- SIP5 clients -- */
    /*TTI - actions - start*/
    INTERNAL_SPECIFIC_HW_VALUES__SNET_LION3_CNC_CLIENT_TTI_ACTION_0_E = 0,
    INTERNAL_SPECIFIC_HW_VALUES__SNET_LION3_CNC_CLIENT_TTI_ACTION_1_E,
    /*TTI - actions - end */
    /*IPCL - start*/
    INTERNAL_SPECIFIC_HW_VALUES__SNET_LION3_CNC_CLIENT_IPCL_LOOKUP_0_ACTION_0_E,
    INTERNAL_SPECIFIC_HW_VALUES__SNET_LION3_CNC_CLIENT_IPCL_LOOKUP_0_ACTION_1_E,
    INTERNAL_SPECIFIC_HW_VALUES__SNET_LION3_CNC_CLIENT_IPCL_LOOKUP_0_ACTION_2_E,
    INTERNAL_SPECIFIC_HW_VALUES__SNET_LION3_CNC_CLIENT_IPCL_LOOKUP_0_ACTION_3_E,

    INTERNAL_SPECIFIC_HW_VALUES__SNET_LION3_CNC_CLIENT_IPCL_LOOKUP_1_ACTION_0_E,
    INTERNAL_SPECIFIC_HW_VALUES__SNET_LION3_CNC_CLIENT_IPCL_LOOKUP_1_ACTION_1_E,
    INTERNAL_SPECIFIC_HW_VALUES__SNET_LION3_CNC_CLIENT_IPCL_LOOKUP_1_ACTION_2_E,
    INTERNAL_SPECIFIC_HW_VALUES__SNET_LION3_CNC_CLIENT_IPCL_LOOKUP_1_ACTION_3_E,

    INTERNAL_SPECIFIC_HW_VALUES__SNET_LION3_CNC_CLIENT_IPCL_LOOKUP_2_ACTION_0_E,
    INTERNAL_SPECIFIC_HW_VALUES__SNET_LION3_CNC_CLIENT_IPCL_LOOKUP_2_ACTION_1_E,
    INTERNAL_SPECIFIC_HW_VALUES__SNET_LION3_CNC_CLIENT_IPCL_LOOKUP_2_ACTION_2_E,
    INTERNAL_SPECIFIC_HW_VALUES__SNET_LION3_CNC_CLIENT_IPCL_LOOKUP_2_ACTION_3_E,
    /*IPCL - end */

    INTERNAL_SPECIFIC_HW_VALUES__SNET_LION3_CNC_CLIENT_INGRESS_VLAN_L2_L3_E,
    INTERNAL_SPECIFIC_HW_VALUES__SNET_LION3_CNC_CLIENT_SOURCE_EPORT_E,
    INTERNAL_SPECIFIC_HW_VALUES__SNET_LION3_CNC_CLIENT_INGRESS_VLAN_PASS_DROP_E,
    INTERNAL_SPECIFIC_HW_VALUES__SNET_LION3_CNC_CLIENT_PACKET_TYPE_PASS_DROP_E,
    INTERNAL_SPECIFIC_HW_VALUES__SNET_LION3_CNC_CLIENT_EGRESS_VLAN_PASS_DROP_E,

    INTERNAL_SPECIFIC_HW_VALUES__SNET_LION3_CNC_CLIENT_EGRESS_QUEUE_PASS_DROP_AND_QCN_PASS_DROP_E,
    INTERNAL_SPECIFIC_HW_VALUES__SNET_LION3_CNC_CLIENT_ARP_INDEX_E,
    INTERNAL_SPECIFIC_HW_VALUES__SNET_LION3_CNC_CLIENT_TUNNEL_START_INDEX_E,

    INTERNAL_SPECIFIC_HW_VALUES__SNET_LION3_CNC_CLIENT_TARGET_EPORT_E,

    /*EPCL - start*/
    INTERNAL_SPECIFIC_HW_VALUES__SNET_LION3_CNC_CLIENT_EPCL_ACTION_0_E,
    INTERNAL_SPECIFIC_HW_VALUES__SNET_LION3_CNC_CLIENT_EPCL_ACTION_1_E,
    INTERNAL_SPECIFIC_HW_VALUES__SNET_LION3_CNC_CLIENT_EPCL_ACTION_2_E,
    INTERNAL_SPECIFIC_HW_VALUES__SNET_LION3_CNC_CLIENT_EPCL_ACTION_3_E,
    /*EPCL - end */

    INTERNAL_SPECIFIC_HW_VALUES__SNET_LION3_CNC_CLIENT_TRAFFIC_MANAGER_PASS_DROP_E,

    /* sip5_20*/
    INTERNAL_SPECIFIC_HW_VALUES__SNET_LION3_CNC_CLIENT_TTI_ACTION_2_E,
    INTERNAL_SPECIFIC_HW_VALUES__SNET_LION3_CNC_CLIENT_TTI_ACTION_3_E,

    /* PIPE device */
    INTERNAL_SPECIFIC_HW_VALUES__PIPE_SNET_CNC_PIPE_DEVICE_CLIENT_PPA_TRG_PORT_PKT_TYPE_E = 0,
    INTERNAL_SPECIFIC_HW_VALUES__PIPE_SNET_CNC_PIPE_DEVICE_CLIENT_PCP_DEST_INDEX_E,
    INTERNAL_SPECIFIC_HW_VALUES__PIPE_SNET_CNC_CLIENT_EGRESS_QUEUE_PASS_DROP_AND_QCN_PASS_DROP_E,

    /* -- sip6 clients -- */
    INTERNAL_SPECIFIC_HW_VALUES__SNET_SIP6_CNC_CLIENT_TTI_ACTION_0_E = 0,
    INTERNAL_SPECIFIC_HW_VALUES__SNET_SIP6_CNC_CLIENT_TTI_ACTION_1_E,

    INTERNAL_SPECIFIC_HW_VALUES__SNET_SIP6_CNC_CLIENT_IPCL_LOOKUP_1_ACTION_0_E,/*2*/
    INTERNAL_SPECIFIC_HW_VALUES__SNET_SIP6_CNC_CLIENT_IPCL_LOOKUP_1_ACTION_1_E,/*3*/
    INTERNAL_SPECIFIC_HW_VALUES__SNET_SIP6_CNC_CLIENT_IPCL_LOOKUP_1_ACTION_2_E,/*4*/
    INTERNAL_SPECIFIC_HW_VALUES__SNET_SIP6_CNC_CLIENT_IPCL_LOOKUP_1_ACTION_3_E,/*5*/

    INTERNAL_SPECIFIC_HW_VALUES__SNET_SIP6_CNC_CLIENT_IPCL_LOOKUP_2_ACTION_0_E,/*6*/
    INTERNAL_SPECIFIC_HW_VALUES__SNET_SIP6_CNC_CLIENT_IPCL_LOOKUP_2_ACTION_1_E,/*7*/
    INTERNAL_SPECIFIC_HW_VALUES__SNET_SIP6_CNC_CLIENT_IPCL_LOOKUP_2_ACTION_2_E,/*8*/
    INTERNAL_SPECIFIC_HW_VALUES__SNET_SIP6_CNC_CLIENT_IPCL_LOOKUP_2_ACTION_3_E,/*9*/

    INTERNAL_SPECIFIC_HW_VALUES__SNET_SIP6_CNC_CLIENT_EQ_DLB_E,                                  /*10*/
    INTERNAL_SPECIFIC_HW_VALUES__SNET_SIP6_CNC_CLIENT_EGF_DLB_E,                                 /*11*/
    INTERNAL_SPECIFIC_HW_VALUES__SNET_SIP6_CNC_CLIENT_RESERVED_12_E,                             /*12*/
    INTERNAL_SPECIFIC_HW_VALUES__SNET_SIP6_CNC_CLIENT_EGF_EVLAN_E,                               /*13*/
    INTERNAL_SPECIFIC_HW_VALUES__SNET_SIP6_CNC_CLIENT_INGRESS_VLAN_L2_L3_E,                      /*14*/
    INTERNAL_SPECIFIC_HW_VALUES__SNET_SIP6_CNC_CLIENT_SOURCE_EPORT_E,                            /*15*/
    INTERNAL_SPECIFIC_HW_VALUES__SNET_SIP6_CNC_CLIENT_INGRESS_VLAN_PASS_DROP_E,                  /*16*/
    INTERNAL_SPECIFIC_HW_VALUES__SNET_SIP6_CNC_CLIENT_PACKET_TYPE_PASS_DROP_E,                   /*17*/
    INTERNAL_SPECIFIC_HW_VALUES__SNET_SIP6_CNC_CLIENT_EGRESS_VLAN_PASS_DROP_E,                   /*18*/
    INTERNAL_SPECIFIC_HW_VALUES__SNET_SIP6_CNC_CLIENT_EGRESS_QUEUE_PASS_DROP_AND_QCN_PASS_DROP_E,/*19*/
    INTERNAL_SPECIFIC_HW_VALUES__SNET_SIP6_CNC_CLIENT_ARP_INDEX_E,                               /*20*/
    INTERNAL_SPECIFIC_HW_VALUES__SNET_SIP6_CNC_CLIENT_TUNNEL_START_INDEX_E,                      /*21*/
    INTERNAL_SPECIFIC_HW_VALUES__SNET_SIP6_CNC_CLIENT_TARGET_EPORT_E,                            /*22*/
    INTERNAL_SPECIFIC_HW_VALUES__SNET_SIP6_CNC_CLIENT_EPCL_ACTION_0_E,                           /*23*/
    INTERNAL_SPECIFIC_HW_VALUES__SNET_SIP6_CNC_CLIENT_EPCL_ACTION_1_E,                           /*24*/
    INTERNAL_SPECIFIC_HW_VALUES__SNET_SIP6_CNC_CLIENT_EPCL_ACTION_2_E,                           /*25*/
    INTERNAL_SPECIFIC_HW_VALUES__SNET_SIP6_CNC_CLIENT_EPCL_ACTION_3_E,                           /*26*/
    INTERNAL_SPECIFIC_HW_VALUES__SNET_SIP6_CNC_CLIENT_EREP_PACKET_TYPE_PASS_DROP_E,              /*27*/
    INTERNAL_SPECIFIC_HW_VALUES__SNET_SIP6_CNC_CLIENT_TTI_ACTION_2_E,                            /*28*/
    INTERNAL_SPECIFIC_HW_VALUES__SNET_SIP6_CNC_CLIENT_TTI_ACTION_3_E,                            /*29*/
    INTERNAL_SPECIFIC_HW_VALUES__SNET_SIP6_CNC_CLIENT_PREQ_QUEUE_STATISTICS_E,                   /*30*/
    INTERNAL_SPECIFIC_HW_VALUES__SNET_SIP6_CNC_CLIENT_PREQ_PORT_STATISTICS_E,                    /*31*/

    INTERNAL_SPECIFIC_HW_VALUES__SNET_CHT3_CNC_CLIENT_LAST_E

} INTERNAL_SPECIFIC_HW_VALUES__SNET_CHT_CNC_CLIENT_E;

/* CH3 : All CNC clients bitmap */
#define SNET_CHT3_CNC_CLIENTS_BMP_ALL_CNS \
        SMEM_BIT_MASK(1+INTERNAL_SPECIFIC_HW_VALUES__SNET_XCAT_CNC_CLIENT_TUNNEL_TERMINATION_E)

/* Lion3 : All CNC clients bitmap */
#define SNET_LION3_CNC_CLIENTS_BMP_ALL_CNS                          \
        (SMEM_BIT_MASK(1+INTERNAL_SPECIFIC_HW_VALUES__SNET_LION3_CNC_CLIENT_TRAFFIC_MANAGER_PASS_DROP_E))

/* sip5_20 : All CNC clients bitmap */
#define SNET_SIP5_20_CNC_CLIENTS_BMP_ALL_CNS                          \
        (SMEM_BIT_MASK(1+INTERNAL_SPECIFIC_HW_VALUES__SNET_LION3_CNC_CLIENT_TTI_ACTION_3_E))


/* sip6 : All CNC clients = 32 bits */
#define SNET_SIP6_CNC_CLIENTS_BMP_ALL_CNS                          \
        0xFFFFFFFF

/* PIPE device : CNC clients bitmap */
#define SNET_PIPE_CNC_CLIENTS_BMP_ALL_CNS                          \
        (SMEM_BIT_MASK(1+INTERNAL_SPECIFIC_HW_VALUES__PIPE_SNET_CNC_CLIENT_EGRESS_QUEUE_PASS_DROP_AND_QCN_PASS_DROP_E))



typedef enum {
    /* new SIP5 clients */
    /*TTI - actions - start*/
    SNET_CNC_CLIENT_TTI_ACTION_0_E,
    SNET_CNC_CLIENT_TTI_ACTION_1_E,
    /*TTI - actions - end */
    /*IPCL - start*/
    SNET_CNC_CLIENT_IPCL_LOOKUP_0_ACTION_0_E,
    SNET_CNC_CLIENT_IPCL_LOOKUP_0_ACTION_1_E,
    SNET_CNC_CLIENT_IPCL_LOOKUP_0_ACTION_2_E,
    SNET_CNC_CLIENT_IPCL_LOOKUP_0_ACTION_3_E,

    SNET_CNC_CLIENT_IPCL_LOOKUP_1_ACTION_0_E,
    SNET_CNC_CLIENT_IPCL_LOOKUP_1_ACTION_1_E,
    SNET_CNC_CLIENT_IPCL_LOOKUP_1_ACTION_2_E,
    SNET_CNC_CLIENT_IPCL_LOOKUP_1_ACTION_3_E,

    SNET_CNC_CLIENT_IPCL_LOOKUP_2_ACTION_0_E,
    SNET_CNC_CLIENT_IPCL_LOOKUP_2_ACTION_1_E,
    SNET_CNC_CLIENT_IPCL_LOOKUP_2_ACTION_2_E,
    SNET_CNC_CLIENT_IPCL_LOOKUP_2_ACTION_3_E,
    /*IPCL - end */

    SNET_CNC_CLIENT_INGRESS_VLAN_L2_L3_E,
    SNET_CNC_CLIENT_SOURCE_EPORT_E,
    SNET_CNC_CLIENT_INGRESS_VLAN_PASS_DROP_E,
    SNET_CNC_CLIENT_PACKET_TYPE_PASS_DROP_E,
    SNET_CNC_CLIENT_EGRESS_VLAN_PASS_DROP_E,

    SNET_CNC_CLIENT_EGRESS_QUEUE_PASS_DROP_AND_QCN_PASS_DROP_E,
    SNET_CNC_CLIENT_ARP_INDEX_E,
    SNET_CNC_CLIENT_TUNNEL_START_INDEX_E,

    SNET_CNC_CLIENT_TARGET_EPORT_E,

    /*EPCL - start*/
    SNET_CNC_CLIENT_EPCL_ACTION_0_E,
    SNET_CNC_CLIENT_EPCL_ACTION_1_E,
    SNET_CNC_CLIENT_EPCL_ACTION_2_E,
    SNET_CNC_CLIENT_EPCL_ACTION_3_E,
    /*EPCL - end */

    SNET_CNC_CLIENT_TRAFFIC_MANAGER_PASS_DROP_E,

    /*TTI - actions - start*/ /* sip5_20*/
    SNET_CNC_CLIENT_TTI_ACTION_2_E,
    SNET_CNC_CLIENT_TTI_ACTION_3_E,
    /*TTI - actions - end */

    /* in the PIPE device this client from the PCP unit for the destination ports bmp table index */
    SNET_CNC_PIPE_DEVICE_CLIENT_PCP_DEST_INDEX_E,
    /* in the PIPE device this client from the PPA unit for the {TrgPort, PktType} */
    SNET_CNC_PIPE_DEVICE_CLIENT_PPA_TRG_PORT_PKT_TYPE_E,

    /* sip6 new */
    SNET_SIP6_CNC_CLIENT_EGF_EVLAN_E,
    SNET_SIP6_CNC_CLIENT_PREQ_QUEUE_STATISTICS_E,
    SNET_SIP6_CNC_CLIENT_PREQ_PORT_STATISTICS_E,
    SNET_SIP6_CNC_CLIENT_EREP_PACKET_TYPE_PASS_DROP_E,

    /* sip6_10 new */
    SNET_SIP6_10_CNC_CLIENT_PHA_E,

    SNET_CNC_CLIENT_LAST_E

} SNET_CNC_CLIENT_E;

/**
* @internal snetCht3CncCount function
* @endinternal
*
* @brief   Trigger CNC event for specified client and set CNC counter block
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] descrPtr                 - pointer to the frame's descriptor.
* @param[in] client                   -  ID.
* @param[in] userDefined              - pointer user defined parameter
*                                       None.
*/
void snetCht3CncCount
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN SNET_CNC_CLIENT_E client,
    IN GT_UINTPTR userDefined
);

/**
* @internal snetCht3CncFastDumpFuncPtr function
* @endinternal
*
* @brief   Process upload CNC block demanded by CPU
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] cncTrigPtr               - pointer to CNC Fast Dump Trigger Register
*/
GT_VOID snetCht3CncFastDumpFuncPtr
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32 * cncTrigPtr
);

/**
* @internal snetCht3CncBlockReset function
* @endinternal
*
* @brief   Centralize counters block reset
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] block                    -  index
* @param[in] entryStart               - start entry index
* @param[in] entryNum                 - number of entry to reset
* @param[in] cncUnitIndex             - the CNC unit index (0/1) (Sip5 devices)
*/
GT_VOID snetCht3CncBlockReset
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32 block,
    IN GT_U32 entryStart,
    IN GT_U32 entryNum,
    IN GT_U32 cncUnitIndex
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif  /* __snetCheetah3CentralizedCnt */



