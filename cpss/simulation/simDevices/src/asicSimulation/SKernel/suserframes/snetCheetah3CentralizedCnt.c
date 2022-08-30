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
* @file snetCheetah3CentralizedCnt.c
*
* @brief Cheetah3 Asic Simulation
* Centralized Counter Unit.
* Source Code file.
*
* @version   54
********************************************************************************
*/
#include <asicSimulation/SKernel/skernel.h>
#include <asicSimulation/SKernel/smem/smemCheetah3.h>
#include <asicSimulation/SKernel/suserframes/snet.h>
#include <asicSimulation/SKernel/cheetahCommon/sregCheetah.h>
#include <common/Utils/Math/sMath.h>
#include <asicSimulation/SKernel/suserframes/snetCheetah3CentralizedCnt.h>
#include <asicSimulation/SLog/simLog.h>
#include <asicSimulation/SLog/simLogInfoTypePacket.h>

/* CNC block counters number */
#define MAX_CNC_BLOCKS_CNS                          16

/* CNC blocks in single control pipe  : legacy : 2 , Hawk : 4 */
#define MAX_CNC_UNITS_CNS                          4

/**
 *  Enum: SMEM_CHT3_CNC_BYTE_CNT_MODE_E
 *
 *  Description:
 *      Byte Count Mode.
 *
 *  Fields:
 *
 *  SMEM_CHT3_CNC_BYTE_CNT_L2_MODE_E - The Byte Count counter counts the entire packet byte count for all packet types.
 *  SMEM_CHT3_CNC_BYTE_CNT_L3_MODE_E - Byte Count counters counts the packet L3 fields (the entire packet minus the L3 offset)
 *  SMEM_CHT3_CNC_BYTE_CNT_PACKETS_MODE_E - Byte Count counters counts packet number
 *  SMEM_CHT3_CNC_BYTE_CNT_FLEX_VALUE_E   - Flex value set by the client
 *
**/
typedef enum {
    SMEM_CHT3_CNC_BYTE_CNT_L2_MODE_E = 0,
    SMEM_CHT3_CNC_BYTE_CNT_L3_MODE_E,
    SMEM_CHT3_CNC_BYTE_CNT_PACKETS_MODE_E,
    SMEM_CHT3_CNC_BYTE_CNT_FLEX_VALUE_E
}SMEM_CHT3_CNC_BYTE_CNT_MODE_E;

/**
* @enum SMEM_CHT3_CNC_EGRESS_QUEUE_CLIENT_MODE_ENT
 *
 * @brief CNC modes of Egress Queue counting.
*/
typedef enum{

    /** count egress queue pass and taildropped packets. */
    SMEM_CHT3_CNC_EGRESS_QUEUE_CLIENT_MODE_TAIL_DROP_E,

    /** count Congestion Notification messages. */
    SMEM_CHT3_CNC_EGRESS_QUEUE_CLIENT_MODE_CN_E,

    /** @brief count egress queue pass and taildropped packets with unaware DP.
     *  (APPLICABLE DEVICES: Bobcat3; Aldrin2; Falcon)
     */
    SMEM_CHT3_CNC_EGRESS_QUEUE_CLIENT_MODE_TAIL_DROP_REDUCED_E

} SMEM_CHT3_CNC_EGRESS_QUEUE_CLIENT_MODE_ENT;

/**
* @struct SNET_CHT3_CNC_CLIENT_INFO_STC
 *
 * @brief CNC client info
*/
typedef struct{

    /** type of client (unified value) */
    SNET_CNC_CLIENT_E client;

    /** the bit index of the client */
    GT_U32 clientHwBit;

    /** client index */
    GT_U32 index;

    /** byte count mode */
    SMEM_CHT3_CNC_BYTE_CNT_MODE_E byteCntMode;

    /** user defined field */
    GT_UINTPTR userDefined;

    GT_U64 rangeBitmap[MAX_CNC_BLOCKS_CNS];

    GT_U32 cncUnitIndex;

    GT_CHAR* clientsNamePtr;

    GT_BIT  triggered;/* new SIP6 support for indication from the unit */
    GT_BIT  debug_is_expected_counter_format_CNC_COUNTER_FORMAT_PKT_44_MAX_SIZE_20_E;/* sip6 client that expects to use 'CNC_COUNTER_FORMAT_PKT_44_MAX_SIZE_20_E' */
    GT_U32  explicitByteCountValue;

    GT_BIT  isErrata_HA2CNC_swapIndexAndBc;/* is errata for 'HA that swap the index and the byte count' -->
                                         SIPCTRL-74 Ha2CNC wrapper connections are turn around */

    GT_BIT  isErrata_NotCountCrc;/* is errata for 'IPCL not count 4 bytes of CRC in L2 mode and in L3 mode !!!' -->
                                         [JIRA] [PCL-829] pcl clients count 4 bytes less that tti client in cnc counetrs in Falcon device */

    /*for sip 6.30 this client instance allow several clients on the same block in parallel
      each client in different offset within the block*/
    struct{
        GT_U32  clientInstance;         /* the current instance to check         */
        GT_U32  clientOffsetInBlock[MAX_CNC_BLOCKS_CNS]; /* the offset of the client in the block */
    }fewClientsPerBlock;

} SNET_CHT3_CNC_CLIENT_INFO_STC;

#define SNET_CHT3_CNC_CLIENT_TRIG_FUN_MAC(funcName)     \
GT_STATUS funcName                                      \
(                                                       \
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,               \
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,      \
    IN SNET_CHT3_CNC_CLIENT_INFO_STC * clientInfoPtr    \
)

static SNET_CHT3_CNC_CLIENT_TRIG_FUN_MAC(snetCht3CncClientL2L3VlanIngrTrigger        );
static SNET_CHT3_CNC_CLIENT_TRIG_FUN_MAC(snetCht3CncClientPclLookUpIngrTrigger       );
static SNET_CHT3_CNC_CLIENT_TRIG_FUN_MAC(snetCht3CncClientVlanPassDropIngrTrigger    );
static SNET_CHT3_CNC_CLIENT_TRIG_FUN_MAC(snetCht3CncClientVlanPassDropEgrTrigger     );
static SNET_CHT3_CNC_CLIENT_TRIG_FUN_MAC(snetCht3CncClientTxqQueuePassDropEgrTrigger );
static SNET_CHT3_CNC_CLIENT_TRIG_FUN_MAC(snetCht3CncClientPclEgrTrigger              );
static SNET_CHT3_CNC_CLIENT_TRIG_FUN_MAC(snetCht3CncClientArpTblTrigger              );
static SNET_CHT3_CNC_CLIENT_TRIG_FUN_MAC(snetCht3CncClientTunnelStartTrigger         );
static SNET_CHT3_CNC_CLIENT_TRIG_FUN_MAC(snetXCatCncClientTunnelTerminatedTrigger    );
static SNET_CHT3_CNC_CLIENT_TRIG_FUN_MAC(snetCht3CncClientHaTargetEPortTrigger       );
static SNET_CHT3_CNC_CLIENT_TRIG_FUN_MAC(snetCht3CncClientIpclSourceEPortTrigger     );
static SNET_CHT3_CNC_CLIENT_TRIG_FUN_MAC(snetLion3CncClientPreEgressPacketTypePassDropTrigger);
static SNET_CHT3_CNC_CLIENT_TRIG_FUN_MAC(snetPipeCncClientPcpDestIndexTrigger     );
static SNET_CHT3_CNC_CLIENT_TRIG_FUN_MAC(snetPipeCncClientPpaTrgPortPktTypeTrigger);
static SNET_CHT3_CNC_CLIENT_TRIG_FUN_MAC(snetCht3CncClientEgfPassDropTrigger);
static SNET_CHT3_CNC_CLIENT_TRIG_FUN_MAC(snetCht3CncClientErepPacketTypePassDropTrigger);
static SNET_CHT3_CNC_CLIENT_TRIG_FUN_MAC(snetSip6CncClientPreqQueueStatisticTrigger    );
static SNET_CHT3_CNC_CLIENT_TRIG_FUN_MAC(snetSip6CncClientPreqPortStatisticTrigger     );

static SNET_CHT3_CNC_CLIENT_TRIG_FUN_MAC(snetPhoenixCncClientPclLookUpIngrTrigger_mux_pcl1_action3_and_pcl2_action3);
static SNET_CHT3_CNC_CLIENT_TRIG_FUN_MAC(snetPhoenixCncClientPclLookUpIngrTrigger_mux_tti_action3_and_pcl0_action3);
static SNET_CHT3_CNC_CLIENT_TRIG_FUN_MAC(snetSip6_10_cncClientPhaTrigger);

/* the "first non-zero bit table" for values 0-15       */
/* the value for 0 is not relevant                      */
/* 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 */
/* 0, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1,  0,  2,  0,  1,  0  */
/* packed in one 32-bit value                           */
/* bits n,n+1 contain the "first non-zero bit" for n    */
#define PRV_FIRST_NON_ZERO_BIT_CNS 0x12131210

/* CNC block entry size is 2 words */
#define CNC_BLOCK_ENTRY_WORDS_CNS                   2

/* Counters update message was dropped in block<N> due to Rate Limiting FIFO Full */
#define SMEM_CHT3_CNC_BLOCK_RATE_LIMIT_FIFO(block)      0x2000 << block

/* Fast Dump of last triggered block finished */
#define SMEM_CHT3_CNC_DUMP_BLOCK_FINISHED               1 << 25

#define SMEM_CHT3_CNC_BLOCK_WRAP_AROUND(block)          1 << ((block) + 1)

#define SNET_CHT3_NO_CNC_DUMP_BUFFERS_SLEEP_TIME        1

extern GT_BIT  oldWaitDuringSkernelTaskForAuqOrFua;

/**
* @enum CNC_COUNTER_FORMAT_ENT
 *
 * @brief CNC modes of counter formats.
*/
typedef enum{

    CNC_COUNTER_FORMAT_PKT_29_BC_35_E,

    CNC_COUNTER_FORMAT_PKT_27_BC_37_E,

    CNC_COUNTER_FORMAT_PKT_37_BC_27_E,

    /** @brief sip5_20
     *  PKT_64_BC_0; PKT_64_BC_0; Partitioning of the 64 Entry bits is as follows:
     *  Packet counter: 64 bits
     *  Byte Count counter: 0 bits (No Counting)
     */
    CNC_COUNTER_FORMAT_PKT_64_BC_0_E,

    /** @brief sip5_20
     *  PKT_0_BC_64; PKT_0_BC_64; Partitioning of the 64 Entry bits is as follows:
     *  Packet counter: 0 bits (No Counting)
     *  Byte Count counter: 64 bits
     */
    CNC_COUNTER_FORMAT_PKT_0_BC_64_E,

    /** @brief sip6
     *  PKT_44_MAX_SIZE_20 : Partitioning of the 64 Entry bits is as follows:
     *  Packet counter: 44 bits
     *  max size: 20 bits
     */
    CNC_COUNTER_FORMAT_PKT_44_MAX_SIZE_20_E

} CNC_COUNTER_FORMAT_ENT;

/**
* @struct SNET_CHT3_CNC_COUNTER_STC
 *
 * @brief The counter entry contents.
*/
typedef struct{

    /** byte count */
    GT_U64 byteCount;

    /** @brief packets count
     *  Comment:
     *  See the possible counter HW formats of Lion and above devices in
     *  SNET_CHT3_CNC_COUNTER_STC_CNC_COUNTER_FORMAT_ENT description.
     *  For DxCh3 and DxChXcat devices the byte counter has 35 bits,
     *  the packets counter has 29 bits
     */
    GT_U64 packetCount;

} SNET_CHT3_CNC_COUNTER_STC;

/*******************************************************************************
*   SNET_CHT3_CNC_CLIENT_TRIG_FUN
*
* DESCRIPTION:
*       CNC client trigger function
*
* INPUTS:
*       devObjPtr       - pointer to device object.
*       descrPtr        - pointer to the frame's descriptor.
*       clientInfoPtr   - pointer to CNC client info.
*
* RETURNS:
*       GT_OK           - client is triggered
*       GT_FAIL         - fail in client triggering
*
*******************************************************************************/
typedef GT_STATUS (* SNET_CHT3_CNC_CLIENT_TRIG_FUN)
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN SNET_CHT3_CNC_CLIENT_INFO_STC * clientInfoPtr
);

static GT_STATUS snetCht3CncClientRangeBitmapGet
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SNET_CHT3_CNC_CLIENT_INFO_STC * clientInfoPtr
);

static GT_BOOL snetCht3CncSendMsg2Cpu
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32 block,
    IN GT_U32 * cncDumpPtr,
    INOUT GT_BOOL * doInterruptPtr,
    IN GT_U32   cncUnitIndex
);

static GT_STATUS snetCht3CncBlockWrapStatusSet
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32 block,
    IN GT_U32 index,
    IN GT_U32   cncUnitIndex
);

static GT_VOID snetCht3CncCounterFormatGet
(
    IN SKERNEL_DEVICE_OBJECT                * devObjPtr,
    IN  GT_U32                              block,
    OUT CNC_COUNTER_FORMAT_ENT    *formatPtr,
    IN GT_U32   cncUnitIndex
);

static GT_STATUS snetCht3CncCounterHwRead
(
    IN   CNC_COUNTER_FORMAT_ENT  format,
    IN   GT_U32                            * regPtr,
    OUT  SNET_CHT3_CNC_COUNTER_STC         * cncCounterPtr
);

static GT_STATUS snetCht3CncCounterHwWrite
(
    IN   CNC_COUNTER_FORMAT_ENT  format,
    IN   GT_U32                            * regPtr,
    IN   SNET_CHT3_CNC_COUNTER_STC         * cncCounterPtr
);

static GT_STATUS snetCht3CncCounterWrapAroundCheck
(
    IN   GT_U32 wrapEn,
    IN   CNC_COUNTER_FORMAT_ENT  format,
    IN   SNET_CHT3_CNC_COUNTER_STC         * cncCounterPtr,
    IN   GT_BIT                         do64BitsByteWrapAround
);

static GT_CHAR*     cncClientsNamesArr[SNET_CNC_CLIENT_LAST_E + 1] =
{
    "(cnc)TTI ACTION 0"                                  ,    /* SNET_CNC_CLIENT_TTI_ACTION_0_E,                                  */
    "(cnc)TTI ACTION 1"                                  ,    /* SNET_CNC_CLIENT_TTI_ACTION_1_E,                                  */
    "(cnc)IPCL_LOOKUP 0 ACTION 0"                        ,    /* SNET_CNC_CLIENT_IPCL_LOOKUP_0_ACTION_0_E,                        */
    "(cnc)IPCL_LOOKUP 0 ACTION 1"                        ,    /* SNET_CNC_CLIENT_IPCL_LOOKUP_0_ACTION_1_E,                        */
    "(cnc)IPCL_LOOKUP 0 ACTION 2"                        ,    /* SNET_CNC_CLIENT_IPCL_LOOKUP_0_ACTION_2_E,                        */
    "(cnc)IPCL_LOOKUP 0 ACTION 3"                        ,    /* SNET_CNC_CLIENT_IPCL_LOOKUP_0_ACTION_3_E,                        */
    "(cnc)IPCL_LOOKUP 1 ACTION 0"                        ,    /* SNET_CNC_CLIENT_IPCL_LOOKUP_1_ACTION_0_E,                        */
    "(cnc)IPCL_LOOKUP 1 ACTION 1"                        ,    /* SNET_CNC_CLIENT_IPCL_LOOKUP_1_ACTION_1_E,                        */
    "(cnc)IPCL_LOOKUP 1 ACTION 2"                        ,    /* SNET_CNC_CLIENT_IPCL_LOOKUP_1_ACTION_2_E,                        */
    "(cnc)IPCL_LOOKUP 1 ACTION 3"                        ,    /* SNET_CNC_CLIENT_IPCL_LOOKUP_1_ACTION_3_E,                        */
    "(cnc)IPCL_LOOKUP 2 ACTION 0"                        ,    /* SNET_CNC_CLIENT_IPCL_LOOKUP_2_ACTION_0_E,                        */
    "(cnc)IPCL_LOOKUP 2 ACTION 1"                        ,    /* SNET_CNC_CLIENT_IPCL_LOOKUP_2_ACTION_1_E,                        */
    "(cnc)IPCL_LOOKUP 2 ACTION 2"                        ,    /* SNET_CNC_CLIENT_IPCL_LOOKUP_2_ACTION_2_E,                        */
    "(cnc)IPCL_LOOKUP 2 ACTION 3"                        ,    /* SNET_CNC_CLIENT_IPCL_LOOKUP_2_ACTION_3_E,                        */
    "(cnc)INGRESS VLAN L2 L3"                            ,    /* SNET_CNC_CLIENT_INGRESS_VLAN_L2_L3_E,                            */
    "(cnc)SOURCE EPORT"                                  ,    /* SNET_CNC_CLIENT_SOURCE_EPORT_E,                                  */
    "(cnc)INGRESS VLAN_PASS_DROP"                        ,    /* SNET_CNC_CLIENT_INGRESS_VLAN_PASS_DROP_E,                        */
    "(cnc)PACKET TYPE_PASS_DROP"                         ,    /* SNET_CNC_CLIENT_PACKET_TYPE_PASS_DROP_E,                         */
    "(cnc)EGRESS VLAN_PASS_DROP"                         ,    /* SNET_CNC_CLIENT_EGRESS_VLAN_PASS_DROP_E,                         */
    "(cnc)EGRESS QUEUE PASS DROP AND QCN PASS DROP"      ,    /* SNET_CNC_CLIENT_EGRESS_QUEUE_PASS_DROP_AND_QCN_PASS_DROP_E,      */
    "(cnc)ARP INDEX"                                     ,    /* SNET_CNC_CLIENT_ARP_INDEX_E,                                     */
    "(cnc)TUNNEL START INDEX"                            ,    /* SNET_CNC_CLIENT_TUNNEL_START_INDEX_E,                            */
    "(cnc)TARGET EPORT"                                  ,    /* SNET_CNC_CLIENT_TARGET_EPORT_E,                                  */
    "(cnc)EPCL ACTION 0"                                 ,    /* SNET_CNC_CLIENT_EPCL_ACTION_0_E,                                 */
    "(cnc)EPCL ACTION 1"                                 ,    /* SNET_CNC_CLIENT_EPCL_ACTION_1_E,                                 */
    "(cnc)EPCL ACTION 2"                                 ,    /* SNET_CNC_CLIENT_EPCL_ACTION_2_E,                                 */
    "(cnc)EPCL ACTION 3"                                 ,    /* SNET_CNC_CLIENT_EPCL_ACTION_3_E,                                 */
    "(cnc)TRAFFIC MANAGER PASS DROP"                     ,    /* SNET_CNC_CLIENT_TRAFFIC_MANAGER_PASS_DROP_E,                     */
    /*sip5_20*/
    "(cnc)TTI ACTION 2"                                  ,    /* SNET_CNC_CLIENT_TTI_ACTION_2_E,                                  */
    "(cnc)TTI ACTION 3"                                  ,    /* SNET_CNC_CLIENT_TTI_ACTION_3_E,                                  */
    /* pipe device*/
    "(cnc)PCP unit "                                     ,    /* SNET_CNC_PIPE_DEVICE_CLIENT_PCP_DEST_INDEX_E                      */
    "(cnc)PPA unit "                                     ,    /* SNET_CNC_PIPE_DEVICE_CLIENT_PPA_TRG_PORT_PKT_TYPE_E               */

    NULL
};

/* convert enum of 'SNET_CNC_CLIENT_E' to names of clients in sip6 and above */
static GT_CHAR*     sip6_cncClientsNamesArr[SNET_CNC_CLIENT_LAST_E + 1] =
{
    "(cnc)(tti) TTI ACTION 0"                            ,    /* SNET_CNC_CLIENT_TTI_ACTION_0_E,                                  */
    "(cnc)(tti) TTI ACTION 1"                            ,    /* SNET_CNC_CLIENT_TTI_ACTION_1_E,                                  */
    "(cnc)(ipcl)IPCL_LOOKUP 0 ACTION 0"                  ,    /* SNET_CNC_CLIENT_IPCL_LOOKUP_0_ACTION_0_E,                        */
    "(cnc)(ipcl)IPCL_LOOKUP 0 ACTION 1"                  ,    /* SNET_CNC_CLIENT_IPCL_LOOKUP_0_ACTION_1_E,                        */
    "(cnc)(ipcl)IPCL_LOOKUP 0 ACTION 2"                  ,    /* SNET_CNC_CLIENT_IPCL_LOOKUP_0_ACTION_2_E,                        */
    "(cnc)(ipcl)IPCL_LOOKUP 0 ACTION 3"                  ,    /* SNET_CNC_CLIENT_IPCL_LOOKUP_0_ACTION_3_E,                        */
    "(cnc)(ipcl)IPCL_LOOKUP 1 ACTION 0"                  ,    /* SNET_CNC_CLIENT_IPCL_LOOKUP_1_ACTION_0_E,                        */
    "(cnc)(ipcl)IPCL_LOOKUP 1 ACTION 1"                  ,    /* SNET_CNC_CLIENT_IPCL_LOOKUP_1_ACTION_1_E,                        */
    "(cnc)(ipcl)IPCL_LOOKUP 1 ACTION 2"                  ,    /* SNET_CNC_CLIENT_IPCL_LOOKUP_1_ACTION_2_E,                        */
    "(cnc)(ipcl)IPCL_LOOKUP 1 ACTION 3"                  ,    /* SNET_CNC_CLIENT_IPCL_LOOKUP_1_ACTION_3_E,                        */
    "(cnc)(ipcl)IPCL_LOOKUP 2 ACTION 0"                  ,    /* SNET_CNC_CLIENT_IPCL_LOOKUP_2_ACTION_0_E,                        */
    "(cnc)(ipcl)IPCL_LOOKUP 2 ACTION 1"                  ,    /* SNET_CNC_CLIENT_IPCL_LOOKUP_2_ACTION_1_E,                        */
    "(cnc)(ipcl)IPCL_LOOKUP 2 ACTION 2"                  ,    /* SNET_CNC_CLIENT_IPCL_LOOKUP_2_ACTION_2_E,                        */
    "(cnc)(ipcl)IPCL_LOOKUP 2 ACTION 3"                  ,    /* SNET_CNC_CLIENT_IPCL_LOOKUP_2_ACTION_3_E,                        */
    "(cnc)(ipcl)INGRESS VLAN L2 L3"                      ,    /* SNET_CNC_CLIENT_INGRESS_VLAN_L2_L3_E,                            */
    "(cnc)(ipcl)SOURCE EPORT"                            ,    /* SNET_CNC_CLIENT_SOURCE_EPORT_E,                                  */
    "(cnc)(eq)  INGRESS VLAN_PASS_DROP"                  ,    /* SNET_CNC_CLIENT_INGRESS_VLAN_PASS_DROP_E,                        */
    "(cnc)(eq)  PACKET TYPE_PASS_DROP"                   ,    /* SNET_CNC_CLIENT_PACKET_TYPE_PASS_DROP_E,                         */
    "(cnc)(preq)EGRESS VLAN_PASS_DROP"                   ,    /* SNET_CNC_CLIENT_EGRESS_VLAN_PASS_DROP_E,                         */
    "(cnc)(preq)EGRESS QUEUE PASS DROP AND QCN PASS DROP",    /* SNET_CNC_CLIENT_EGRESS_QUEUE_PASS_DROP_AND_QCN_PASS_DROP_E,      */
    "(cnc)(ha)  ARP INDEX"                               ,    /* SNET_CNC_CLIENT_ARP_INDEX_E,                                     */
    "(cnc)(ha)  TUNNEL START INDEX"                      ,    /* SNET_CNC_CLIENT_TUNNEL_START_INDEX_E,                            */
    "(cnc)(ha)  TARGET EPORT"                            ,    /* SNET_CNC_CLIENT_TARGET_EPORT_E,                                  */
    "(cnc)(epcl)EPCL ACTION 0"                           ,    /* SNET_CNC_CLIENT_EPCL_ACTION_0_E,                                 */
    "(cnc)(epcl)EPCL ACTION 1"                           ,    /* SNET_CNC_CLIENT_EPCL_ACTION_1_E,                                 */
    "(cnc)(epcl)EPCL ACTION 2"                           ,    /* SNET_CNC_CLIENT_EPCL_ACTION_2_E,                                 */
    "(cnc)(epcl)EPCL ACTION 3"                           ,    /* SNET_CNC_CLIENT_EPCL_ACTION_3_E,                                 */
    "(NON-EXIST)TRAFFIC MANAGER PASS DROP"               ,    /* SNET_CNC_CLIENT_TRAFFIC_MANAGER_PASS_DROP_E,                     */
    "(cnc)(tti) TTI ACTION 2"                            ,    /* SNET_CNC_CLIENT_TTI_ACTION_2_E,                                  */
    "(cnc)(tti) TTI ACTION 3"                            ,    /* SNET_CNC_CLIENT_TTI_ACTION_3_E,                                  */
    "(NON-EXIST)PCP unit "                               ,    /* SNET_CNC_PIPE_DEVICE_CLIENT_PCP_DEST_INDEX_E                     */
    "(NON-EXIST)PPA unit "                               ,    /* SNET_CNC_PIPE_DEVICE_CLIENT_PPA_TRG_PORT_PKT_TYPE_E              */

    "(cnc)(egf) EGRESS FILTER EVLAN_PASS_DROP"           ,    /* SNET_SIP6_CNC_CLIENT_EGF_EVLAN_E                                 */
    "(cnc)(preq)EGRESS QUEUE STATISTICS"                 ,    /* SNET_SIP6_CNC_CLIENT_PREQ_QUEUE_STATISTICS_E                     */
    "(cnc)(preq)EGRESS PORT STATISTICS"                  ,    /* SNET_SIP6_CNC_CLIENT_PREQ_PORT_STATISTICS_E                      */
    "(cnc)(erep)EGRESS PACKET TYPE_PASS_DROP"            ,    /* SNET_SIP6_CNC_CLIENT_EREP_PACKET_TYPE_PASS_DROP_E,               */

    "(cnc)(pha)PHA unit "                                ,    /* SNET_SIP6_10_CNC_CLIENT_PHA_E                                    */


    NULL
};

/* Trigger function array holds trigger function for all clients */
static SNET_CHT3_CNC_CLIENT_TRIG_FUN gCncClientTrigFuncArr[SNET_CNC_CLIENT_LAST_E] =
{
    snetXCatCncClientTunnelTerminatedTrigger,   /* SNET_CNC_CLIENT_TTI_ACTION_0_E                                 */
    snetXCatCncClientTunnelTerminatedTrigger,   /* SNET_CNC_CLIENT_TTI_ACTION_1_E                                 */
    snetCht3CncClientPclLookUpIngrTrigger,      /* SNET_CNC_CLIENT_IPCL_LOOKUP_0_ACTION_0_E                       */
    snetCht3CncClientPclLookUpIngrTrigger,      /* SNET_CNC_CLIENT_IPCL_LOOKUP_0_ACTION_1_E                       */
    snetCht3CncClientPclLookUpIngrTrigger,      /* SNET_CNC_CLIENT_IPCL_LOOKUP_0_ACTION_2_E                       */
    snetCht3CncClientPclLookUpIngrTrigger,      /* SNET_CNC_CLIENT_IPCL_LOOKUP_0_ACTION_3_E                       */
    snetCht3CncClientPclLookUpIngrTrigger,      /* SNET_CNC_CLIENT_IPCL_LOOKUP_1_ACTION_0_E                       */
    snetCht3CncClientPclLookUpIngrTrigger,      /* SNET_CNC_CLIENT_IPCL_LOOKUP_1_ACTION_1_E                       */
    snetCht3CncClientPclLookUpIngrTrigger,      /* SNET_CNC_CLIENT_IPCL_LOOKUP_1_ACTION_2_E                       */
    snetCht3CncClientPclLookUpIngrTrigger,      /* SNET_CNC_CLIENT_IPCL_LOOKUP_1_ACTION_3_E                       */
    snetCht3CncClientPclLookUpIngrTrigger,      /* SNET_CNC_CLIENT_IPCL_LOOKUP_2_ACTION_0_E                       */
    snetCht3CncClientPclLookUpIngrTrigger,      /* SNET_CNC_CLIENT_IPCL_LOOKUP_2_ACTION_1_E                       */
    snetCht3CncClientPclLookUpIngrTrigger,      /* SNET_CNC_CLIENT_IPCL_LOOKUP_2_ACTION_2_E                       */
    snetCht3CncClientPclLookUpIngrTrigger,      /* SNET_CNC_CLIENT_IPCL_LOOKUP_2_ACTION_3_E                       */
    snetCht3CncClientL2L3VlanIngrTrigger,       /* SNET_CNC_CLIENT_INGRESS_VLAN_L2_L3_E                           */
    snetCht3CncClientIpclSourceEPortTrigger,    /* SNET_CNC_CLIENT_SOURCE_EPORT_E                                 */
    snetCht3CncClientVlanPassDropIngrTrigger,   /* SNET_CNC_CLIENT_INGRESS_VLAN_PASS_DROP_E                       */
    snetLion3CncClientPreEgressPacketTypePassDropTrigger,/* SNET_CNC_CLIENT_PACKET_TYPE_PASS_DROP_E                */
    snetCht3CncClientVlanPassDropEgrTrigger,    /* SNET_CNC_CLIENT_EGRESS_VLAN_PASS_DROP_E                        */
    snetCht3CncClientTxqQueuePassDropEgrTrigger,/* SNET_CNC_CLIENT_EGRESS_QUEUE_PASS_DROP_AND_QCN_PASS_DROP_E     */
    snetCht3CncClientArpTblTrigger,             /* SNET_CNC_CLIENT_ARP_INDEX_E                                    */
    snetCht3CncClientTunnelStartTrigger,        /* SNET_CNC_CLIENT_TUNNEL_START_INDEX_E                           */
    snetCht3CncClientHaTargetEPortTrigger,      /* SNET_CNC_CLIENT_TARGET_EPORT_E                                 */
    snetCht3CncClientPclEgrTrigger,             /* SNET_CNC_CLIENT_EPCL_ACTION_0_E                                */
    snetCht3CncClientPclEgrTrigger,             /* SNET_CNC_CLIENT_EPCL_ACTION_1_E                                */
    snetCht3CncClientPclEgrTrigger,             /* SNET_CNC_CLIENT_EPCL_ACTION_2_E                                */
    snetCht3CncClientPclEgrTrigger,             /* SNET_CNC_CLIENT_EPCL_ACTION_3_E                                */
    NULL,                                       /* SNET_CNC_CLIENT_TRAFFIC_MANAGER_PASS_DROP_E                    */
    snetXCatCncClientTunnelTerminatedTrigger,   /* SNET_CNC_CLIENT_TTI_ACTION_2_E                                 */
    snetXCatCncClientTunnelTerminatedTrigger,   /* SNET_CNC_CLIENT_TTI_ACTION_3_E                                 */

    snetPipeCncClientPcpDestIndexTrigger,       /*SNET_CNC_PIPE_DEVICE_CLIENT_PCP_DEST_INDEX_E*/
    snetPipeCncClientPpaTrgPortPktTypeTrigger,  /*SNET_CNC_PIPE_DEVICE_CLIENT_PPA_TRG_PORT_PKT_TYPE_E*/

};


/* Trigger function array for sip_6 device only, array holds trigger function for cnc client */
static SNET_CHT3_CNC_CLIENT_TRIG_FUN gCncClientTrigFuncArrSip6[SNET_CNC_CLIENT_LAST_E] =
{
    snetXCatCncClientTunnelTerminatedTrigger,   /* SNET_CNC_CLIENT_TTI_ACTION_0_E                                 */  /* 0*/
    snetXCatCncClientTunnelTerminatedTrigger,   /* SNET_CNC_CLIENT_TTI_ACTION_1_E                                 */  /* 1*/
    snetCht3CncClientPclLookUpIngrTrigger,      /* SNET_CNC_CLIENT_IPCL_LOOKUP_1_ACTION_0_E                       */  /* 2*/
    snetCht3CncClientPclLookUpIngrTrigger,      /* SNET_CNC_CLIENT_IPCL_LOOKUP_1_ACTION_1_E                       */  /* 3*/
    snetCht3CncClientPclLookUpIngrTrigger,      /* SNET_CNC_CLIENT_IPCL_LOOKUP_1_ACTION_2_E                       */  /* 4*/
    snetCht3CncClientPclLookUpIngrTrigger,      /* SNET_CNC_CLIENT_IPCL_LOOKUP_1_ACTION_3_E                       */  /* 5*/
    snetCht3CncClientPclLookUpIngrTrigger,      /* SNET_CNC_CLIENT_IPCL_LOOKUP_2_ACTION_0_E                       */  /* 6*/
    snetCht3CncClientPclLookUpIngrTrigger,      /* SNET_CNC_CLIENT_IPCL_LOOKUP_2_ACTION_1_E                       */  /* 7*/
    snetCht3CncClientPclLookUpIngrTrigger,      /* SNET_CNC_CLIENT_IPCL_LOOKUP_2_ACTION_2_E                       */  /* 8*/
    snetCht3CncClientPclLookUpIngrTrigger,      /* SNET_CNC_CLIENT_IPCL_LOOKUP_2_ACTION_3_E                       */  /* 9*/
    NULL,                                       /* Reserved */                                                        /*10*/
    NULL,                                       /* Reserved */                                                        /*11*/
    NULL,                                       /* Reserved */                                                        /*12*/
    snetCht3CncClientEgfPassDropTrigger,         /* SNET_SIP6_CNC_CLIENT_EGF_EVLAN_E                              */  /*13*/
    snetCht3CncClientL2L3VlanIngrTrigger,       /* SNET_CNC_CLIENT_INGRESS_VLAN_L2_L3_E                           */  /*14*/
    snetCht3CncClientIpclSourceEPortTrigger,    /* SNET_CNC_CLIENT_SOURCE_EPORT_E                                 */  /*15*/
    snetCht3CncClientVlanPassDropIngrTrigger,   /* SNET_CNC_CLIENT_INGRESS_VLAN_PASS_DROP_E                       */  /*16*/
    snetLion3CncClientPreEgressPacketTypePassDropTrigger,/* SNET_CNC_CLIENT_PACKET_TYPE_PASS_DROP_E               */  /*17*/
    snetCht3CncClientVlanPassDropEgrTrigger,    /* SNET_CNC_CLIENT_EGRESS_VLAN_PASS_DROP_E                        */  /*18*/
    snetCht3CncClientTxqQueuePassDropEgrTrigger,/* SNET_CNC_CLIENT_EGRESS_QUEUE_PASS_DROP_AND_QCN_PASS_DROP_E     */  /*19*/
    snetCht3CncClientArpTblTrigger,             /* SNET_CNC_CLIENT_ARP_INDEX_E                                    */  /*20*/
    snetCht3CncClientTunnelStartTrigger,        /* SNET_CNC_CLIENT_TUNNEL_START_INDEX_E                           */  /*21*/
    snetCht3CncClientHaTargetEPortTrigger,      /* SNET_CNC_CLIENT_TARGET_EPORT_E                                 */  /*22*/
    snetCht3CncClientPclEgrTrigger,             /* SNET_CNC_CLIENT_EPCL_ACTION_0_E                                */  /*23*/
    snetCht3CncClientPclEgrTrigger,             /* SNET_CNC_CLIENT_EPCL_ACTION_1_E                                */  /*24*/
    snetCht3CncClientPclEgrTrigger,             /* SNET_CNC_CLIENT_EPCL_ACTION_2_E                                */  /*25*/
    snetCht3CncClientPclEgrTrigger,             /* SNET_CNC_CLIENT_EPCL_ACTION_3_E                                */  /*26*/
    snetCht3CncClientErepPacketTypePassDropTrigger, /* SNET_SIP6_CNC_CLIENT_EREP_PACKET_TYPE_PASS_DROP_E          */  /*27*/
    snetXCatCncClientTunnelTerminatedTrigger,   /* SNET_CNC_CLIENT_TTI_ACTION_2_E                                 */  /*28*/
    snetXCatCncClientTunnelTerminatedTrigger,   /* SNET_CNC_CLIENT_TTI_ACTION_3_E                                 */  /*29*/
    snetSip6CncClientPreqQueueStatisticTrigger, /* SNET_SIP6_CNC_CLIENT_PREQ_QUEUE_STATISTICS_E                   */  /*30*/
    snetSip6CncClientPreqPortStatisticTrigger,  /* SNET_SIP6_CNC_CLIENT_PREQ_PORT_STATISTICS_E                    */  /*31*/
};

typedef struct{
    GT_U32                          clientHwBitArr[MAX_CNC_UNITS_CNS]; /* need to be more/equal to devObjPtr->cncNumOfUnits */
    SNET_CHT3_CNC_CLIENT_TRIG_FUN   cncClientTriggerFunc;
}SIP6_10_CNC_CLIENT_IN_BLOCK;
/* in sip6_10 blocks 0,1 are the same , blocks 2,3 are invalid ! */
#define SIP6_10_ENTRY_CNC_CLIENT_01(bit01,func) \
    {{bit01,bit01,GT_NA,GT_NA}/*clientHwBitArr[]*/,func/*cncClientTriggerFunc*/}
/* in sip6_10 blocks 2,3 are the same , blocks 0,1 are invalid ! */
#define SIP6_10_ENTRY_CNC_CLIENT_23(bit23, func)    \
    {{GT_NA,GT_NA,bit23,bit23}/*clientHwBitArr[]*/,func/*cncClientTriggerFunc*/}
/* in sip6_10 blocks 0,1,2,3 are the same */
#define SIP6_10_ENTRY_CNC_CLIENT_ALL(bit, func) \
    {{bit,bit,bit,bit}/*clientHwBitArr[]*/,func/*cncClientTriggerFunc*/}
/* in sip6_10 INVALID_CLIENT */
#define SIP6_10_ENTRY_CNC_INVALID_CLIENT    \
    {{GT_NA,GT_NA,GT_NA,GT_NA},NULL}

#define GT_NA SMAIN_NOT_VALID_CNS

/* Trigger function array for sip_6_10 device only for CNC blocks 0,1 array holds trigger function for cnc client */
static SIP6_10_CNC_CLIENT_IN_BLOCK gCncClientTrigFuncArrSip6_10[SNET_CNC_CLIENT_LAST_E] =
{
     SIP6_10_ENTRY_CNC_CLIENT_01 (28,snetXCatCncClientTunnelTerminatedTrigger)                 /* SNET_CNC_CLIENT_TTI_ACTION_0_E */
    ,SIP6_10_ENTRY_CNC_CLIENT_01 (29,snetXCatCncClientTunnelTerminatedTrigger)                 /* SNET_CNC_CLIENT_TTI_ACTION_1_E */
    ,SIP6_10_ENTRY_CNC_CLIENT_ALL( 0,snetCht3CncClientPclLookUpIngrTrigger)                    /* SNET_CNC_CLIENT_IPCL_LOOKUP_0_ACTION_0_E */
    ,SIP6_10_ENTRY_CNC_CLIENT_ALL( 1,snetCht3CncClientPclLookUpIngrTrigger)                    /* SNET_CNC_CLIENT_IPCL_LOOKUP_0_ACTION_1_E */
    ,SIP6_10_ENTRY_CNC_CLIENT_ALL(10,snetCht3CncClientPclLookUpIngrTrigger)                    /* SNET_CNC_CLIENT_IPCL_LOOKUP_0_ACTION_2_E */
    ,SIP6_10_ENTRY_CNC_CLIENT_ALL(11,snetCht3CncClientPclLookUpIngrTrigger)                    /* SNET_CNC_CLIENT_IPCL_LOOKUP_0_ACTION_3_E */
    ,SIP6_10_ENTRY_CNC_CLIENT_ALL( 2,snetCht3CncClientPclLookUpIngrTrigger)                    /* SNET_CNC_CLIENT_IPCL_LOOKUP_1_ACTION_0_E */
    ,SIP6_10_ENTRY_CNC_CLIENT_ALL( 3,snetCht3CncClientPclLookUpIngrTrigger)                    /* SNET_CNC_CLIENT_IPCL_LOOKUP_1_ACTION_1_E */
    ,SIP6_10_ENTRY_CNC_CLIENT_ALL( 4,snetCht3CncClientPclLookUpIngrTrigger)                    /* SNET_CNC_CLIENT_IPCL_LOOKUP_1_ACTION_2_E */
    ,SIP6_10_ENTRY_CNC_CLIENT_ALL( 5,snetCht3CncClientPclLookUpIngrTrigger)                    /* SNET_CNC_CLIENT_IPCL_LOOKUP_1_ACTION_3_E */
    ,SIP6_10_ENTRY_CNC_CLIENT_ALL( 6,snetCht3CncClientPclLookUpIngrTrigger)                    /* SNET_CNC_CLIENT_IPCL_LOOKUP_2_ACTION_0_E */
    ,SIP6_10_ENTRY_CNC_CLIENT_ALL( 7,snetCht3CncClientPclLookUpIngrTrigger)                    /* SNET_CNC_CLIENT_IPCL_LOOKUP_2_ACTION_1_E */
    ,SIP6_10_ENTRY_CNC_CLIENT_ALL( 8,snetCht3CncClientPclLookUpIngrTrigger)                    /* SNET_CNC_CLIENT_IPCL_LOOKUP_2_ACTION_2_E */
    ,SIP6_10_ENTRY_CNC_CLIENT_ALL( 9,snetCht3CncClientPclLookUpIngrTrigger)                    /* SNET_CNC_CLIENT_IPCL_LOOKUP_2_ACTION_3_E */
    ,SIP6_10_ENTRY_CNC_CLIENT_ALL(14,snetCht3CncClientL2L3VlanIngrTrigger)                     /* SNET_CNC_CLIENT_INGRESS_VLAN_L2_L3_E     */
    ,SIP6_10_ENTRY_CNC_CLIENT_ALL(15,snetCht3CncClientIpclSourceEPortTrigger)                  /* SNET_CNC_CLIENT_SOURCE_EPORT_E           */
    ,SIP6_10_ENTRY_CNC_CLIENT_ALL(16,snetCht3CncClientVlanPassDropIngrTrigger)                 /* SNET_CNC_CLIENT_INGRESS_VLAN_PASS_DROP_E */
    ,SIP6_10_ENTRY_CNC_CLIENT_ALL(17,snetLion3CncClientPreEgressPacketTypePassDropTrigger)     /* SNET_CNC_CLIENT_PACKET_TYPE_PASS_DROP_E  */
    ,SIP6_10_ENTRY_CNC_CLIENT_ALL(18,snetCht3CncClientVlanPassDropEgrTrigger)                  /* SNET_CNC_CLIENT_EGRESS_VLAN_PASS_DROP_E  */
    ,SIP6_10_ENTRY_CNC_CLIENT_ALL(19,snetCht3CncClientTxqQueuePassDropEgrTrigger)              /* SNET_CNC_CLIENT_EGRESS_QUEUE_PASS_DROP_AND_QCN_PASS_DROP_E */
    ,SIP6_10_ENTRY_CNC_CLIENT_ALL(20,snetCht3CncClientArpTblTrigger)                           /* SNET_CNC_CLIENT_ARP_INDEX_E  */
    ,SIP6_10_ENTRY_CNC_CLIENT_ALL(21,snetCht3CncClientTunnelStartTrigger)                      /* SNET_CNC_CLIENT_TUNNEL_START_INDEX_E  */
    ,SIP6_10_ENTRY_CNC_CLIENT_ALL(22,snetCht3CncClientHaTargetEPortTrigger)                    /* SNET_CNC_CLIENT_TARGET_EPORT_E  */
    ,SIP6_10_ENTRY_CNC_CLIENT_ALL(23,snetCht3CncClientPclEgrTrigger)                           /* SNET_CNC_CLIENT_EPCL_ACTION_0_E  */
    ,SIP6_10_ENTRY_CNC_CLIENT_ALL(24,snetCht3CncClientPclEgrTrigger)                           /* SNET_CNC_CLIENT_EPCL_ACTION_1_E  */
    ,SIP6_10_ENTRY_CNC_CLIENT_ALL(25,snetCht3CncClientPclEgrTrigger)                           /* SNET_CNC_CLIENT_EPCL_ACTION_2_E  */
    ,SIP6_10_ENTRY_CNC_CLIENT_ALL(26,snetCht3CncClientPclEgrTrigger)                           /* SNET_CNC_CLIENT_EPCL_ACTION_3_E  */
    ,SIP6_10_ENTRY_CNC_INVALID_CLIENT                                                          /* SNET_CNC_CLIENT_TRAFFIC_MANAGER_PASS_DROP_E */
    ,SIP6_10_ENTRY_CNC_CLIENT_23(28,snetXCatCncClientTunnelTerminatedTrigger)                  /* SNET_CNC_CLIENT_TTI_ACTION_2_E */
    ,SIP6_10_ENTRY_CNC_CLIENT_23(29,snetXCatCncClientTunnelTerminatedTrigger)                  /* SNET_CNC_CLIENT_TTI_ACTION_3_E */
    ,SIP6_10_ENTRY_CNC_INVALID_CLIENT                                                          /* SNET_CNC_PIPE_DEVICE_CLIENT_PCP_DEST_INDEX_E        */
    ,SIP6_10_ENTRY_CNC_INVALID_CLIENT                                                          /* SNET_CNC_PIPE_DEVICE_CLIENT_PPA_TRG_PORT_PKT_TYPE_E */
    ,SIP6_10_ENTRY_CNC_CLIENT_ALL(13,snetCht3CncClientEgfPassDropTrigger)                      /* SNET_SIP6_CNC_CLIENT_EGF_EVLAN_E  */
    ,SIP6_10_ENTRY_CNC_CLIENT_ALL(30,snetSip6CncClientPreqQueueStatisticTrigger)               /* SNET_SIP6_CNC_CLIENT_PREQ_QUEUE_STATISTICS_E */
    ,SIP6_10_ENTRY_CNC_CLIENT_ALL(31,snetSip6CncClientPreqPortStatisticTrigger)                /* SNET_SIP6_CNC_CLIENT_PREQ_PORT_STATISTICS_E  */
    ,SIP6_10_ENTRY_CNC_CLIENT_ALL(27,snetCht3CncClientErepPacketTypePassDropTrigger)           /* SNET_SIP6_CNC_CLIENT_EREP_PACKET_TYPE_PASS_DROP_E  */
    ,SIP6_10_ENTRY_CNC_CLIENT_ALL(12,snetSip6_10_cncClientPhaTrigger)                          /* SNET_SIP6_10_CNC_CLIENT_PHA_E                      */
};

/* Trigger function array for sip_6_15 device only for CNC blocks 0 array holds trigger function for cnc client */
static SIP6_10_CNC_CLIENT_IN_BLOCK gCncClientTrigFuncArrSip6_15[SNET_CNC_CLIENT_LAST_E] =
{
     SIP6_10_ENTRY_CNC_CLIENT_ALL( 0,snetXCatCncClientTunnelTerminatedTrigger)                 /* SNET_CNC_CLIENT_TTI_ACTION_0_E */
    ,SIP6_10_ENTRY_CNC_CLIENT_ALL( 1,snetXCatCncClientTunnelTerminatedTrigger)                 /* SNET_CNC_CLIENT_TTI_ACTION_1_E */
    ,SIP6_10_ENTRY_CNC_CLIENT_ALL( 9,snetCht3CncClientPclLookUpIngrTrigger)                    /* SNET_CNC_CLIENT_IPCL_LOOKUP_0_ACTION_0_E */
    ,SIP6_10_ENTRY_CNC_CLIENT_ALL(10,snetCht3CncClientPclLookUpIngrTrigger)                    /* SNET_CNC_CLIENT_IPCL_LOOKUP_0_ACTION_1_E */
    ,SIP6_10_ENTRY_CNC_CLIENT_ALL(11,snetCht3CncClientPclLookUpIngrTrigger)                    /* SNET_CNC_CLIENT_IPCL_LOOKUP_0_ACTION_2_E */
    ,SIP6_10_ENTRY_CNC_CLIENT_ALL(12,snetPhoenixCncClientPclLookUpIngrTrigger_mux_tti_action3_and_pcl0_action3)     /* SNET_CNC_CLIENT_IPCL_LOOKUP_0_ACTION_3_E */
    ,SIP6_10_ENTRY_CNC_CLIENT_ALL( 2,snetCht3CncClientPclLookUpIngrTrigger)                    /* SNET_CNC_CLIENT_IPCL_LOOKUP_1_ACTION_0_E */
    ,SIP6_10_ENTRY_CNC_CLIENT_ALL( 3,snetCht3CncClientPclLookUpIngrTrigger)                    /* SNET_CNC_CLIENT_IPCL_LOOKUP_1_ACTION_1_E */
    ,SIP6_10_ENTRY_CNC_CLIENT_ALL( 4,snetCht3CncClientPclLookUpIngrTrigger)                    /* SNET_CNC_CLIENT_IPCL_LOOKUP_1_ACTION_2_E */
    ,SIP6_10_ENTRY_CNC_CLIENT_ALL( 5,snetPhoenixCncClientPclLookUpIngrTrigger_mux_pcl1_action3_and_pcl2_action3)    /* SNET_CNC_CLIENT_IPCL_LOOKUP_1_ACTION_3_E */
    ,SIP6_10_ENTRY_CNC_CLIENT_ALL( 6,snetCht3CncClientPclLookUpIngrTrigger)                    /* SNET_CNC_CLIENT_IPCL_LOOKUP_2_ACTION_0_E */
    ,SIP6_10_ENTRY_CNC_CLIENT_ALL( 7,snetCht3CncClientPclLookUpIngrTrigger)                    /* SNET_CNC_CLIENT_IPCL_LOOKUP_2_ACTION_1_E */
    ,SIP6_10_ENTRY_CNC_CLIENT_ALL( 8,snetCht3CncClientPclLookUpIngrTrigger)                    /* SNET_CNC_CLIENT_IPCL_LOOKUP_2_ACTION_2_E */
    ,SIP6_10_ENTRY_CNC_CLIENT_ALL( 5,snetPhoenixCncClientPclLookUpIngrTrigger_mux_pcl1_action3_and_pcl2_action3)    /* SNET_CNC_CLIENT_IPCL_LOOKUP_2_ACTION_3_E */
    ,SIP6_10_ENTRY_CNC_CLIENT_ALL(14,snetCht3CncClientL2L3VlanIngrTrigger)                     /* SNET_CNC_CLIENT_INGRESS_VLAN_L2_L3_E     */
    ,SIP6_10_ENTRY_CNC_CLIENT_ALL(15,snetCht3CncClientIpclSourceEPortTrigger)                  /* SNET_CNC_CLIENT_SOURCE_EPORT_E           */
    ,SIP6_10_ENTRY_CNC_CLIENT_ALL(16,snetCht3CncClientVlanPassDropIngrTrigger)                 /* SNET_CNC_CLIENT_INGRESS_VLAN_PASS_DROP_E */
    ,SIP6_10_ENTRY_CNC_CLIENT_ALL(17,snetLion3CncClientPreEgressPacketTypePassDropTrigger)     /* SNET_CNC_CLIENT_PACKET_TYPE_PASS_DROP_E  */
    ,SIP6_10_ENTRY_CNC_CLIENT_ALL(18,snetCht3CncClientVlanPassDropEgrTrigger)                  /* SNET_CNC_CLIENT_EGRESS_VLAN_PASS_DROP_E  */
    ,SIP6_10_ENTRY_CNC_CLIENT_ALL(19,snetCht3CncClientTxqQueuePassDropEgrTrigger)              /* SNET_CNC_CLIENT_EGRESS_QUEUE_PASS_DROP_AND_QCN_PASS_DROP_E */
    ,SIP6_10_ENTRY_CNC_CLIENT_ALL(20,snetCht3CncClientArpTblTrigger)                           /* SNET_CNC_CLIENT_ARP_INDEX_E  */
    ,SIP6_10_ENTRY_CNC_CLIENT_ALL(21,snetCht3CncClientTunnelStartTrigger)                      /* SNET_CNC_CLIENT_TUNNEL_START_INDEX_E  */
    ,SIP6_10_ENTRY_CNC_CLIENT_ALL(22,snetCht3CncClientHaTargetEPortTrigger)                    /* SNET_CNC_CLIENT_TARGET_EPORT_E  */
    ,SIP6_10_ENTRY_CNC_CLIENT_ALL(23,snetCht3CncClientPclEgrTrigger)                           /* SNET_CNC_CLIENT_EPCL_ACTION_0_E  */
    ,SIP6_10_ENTRY_CNC_CLIENT_ALL(24,snetCht3CncClientPclEgrTrigger)                           /* SNET_CNC_CLIENT_EPCL_ACTION_1_E  */
    ,SIP6_10_ENTRY_CNC_CLIENT_ALL(25,snetCht3CncClientPclEgrTrigger)                           /* SNET_CNC_CLIENT_EPCL_ACTION_2_E  */
    ,SIP6_10_ENTRY_CNC_CLIENT_ALL(26,snetCht3CncClientPclEgrTrigger)                           /* SNET_CNC_CLIENT_EPCL_ACTION_3_E  */
    ,SIP6_10_ENTRY_CNC_INVALID_CLIENT                                                          /* SNET_CNC_CLIENT_TRAFFIC_MANAGER_PASS_DROP_E */
    ,SIP6_10_ENTRY_CNC_CLIENT_ALL(28,snetXCatCncClientTunnelTerminatedTrigger)                  /* SNET_CNC_CLIENT_TTI_ACTION_2_E */
    ,SIP6_10_ENTRY_CNC_CLIENT_ALL(12,snetPhoenixCncClientPclLookUpIngrTrigger_mux_tti_action3_and_pcl0_action3)      /* SNET_CNC_CLIENT_TTI_ACTION_3_E */
    ,SIP6_10_ENTRY_CNC_INVALID_CLIENT                                                          /* SNET_CNC_PIPE_DEVICE_CLIENT_PCP_DEST_INDEX_E        */
    ,SIP6_10_ENTRY_CNC_INVALID_CLIENT                                                          /* SNET_CNC_PIPE_DEVICE_CLIENT_PPA_TRG_PORT_PKT_TYPE_E */
    ,SIP6_10_ENTRY_CNC_CLIENT_ALL(13,snetCht3CncClientEgfPassDropTrigger)                      /* SNET_SIP6_CNC_CLIENT_EGF_EVLAN_E  */
    ,SIP6_10_ENTRY_CNC_CLIENT_ALL(30,snetSip6CncClientPreqQueueStatisticTrigger)               /* SNET_SIP6_CNC_CLIENT_PREQ_QUEUE_STATISTICS_E */
    ,SIP6_10_ENTRY_CNC_CLIENT_ALL(31,snetSip6CncClientPreqPortStatisticTrigger)                /* SNET_SIP6_CNC_CLIENT_PREQ_PORT_STATISTICS_E  */
    ,SIP6_10_ENTRY_CNC_CLIENT_ALL(27,snetCht3CncClientErepPacketTypePassDropTrigger)           /* SNET_SIP6_CNC_CLIENT_EREP_PACKET_TYPE_PASS_DROP_E  */
    ,SIP6_10_ENTRY_CNC_CLIENT_ALL(29,snetSip6_10_cncClientPhaTrigger)                          /* SNET_SIP6_10_CNC_CLIENT_PHA_E                      */
};

/* Trigger function array for sip_6_30 device only for CNC blocks 0 array holds trigger function for cnc client */
static SIP6_10_CNC_CLIENT_IN_BLOCK gCncClientTrigFuncArrSip6_30[SNET_CNC_CLIENT_LAST_E] =
{
     SIP6_10_ENTRY_CNC_CLIENT_ALL( 0,snetXCatCncClientTunnelTerminatedTrigger)                 /* SNET_CNC_CLIENT_TTI_ACTION_0_E */
    ,SIP6_10_ENTRY_CNC_CLIENT_ALL( 1,snetXCatCncClientTunnelTerminatedTrigger)                 /* SNET_CNC_CLIENT_TTI_ACTION_1_E */
    ,SIP6_10_ENTRY_CNC_INVALID_CLIENT                                                          /* SNET_CNC_CLIENT_IPCL_LOOKUP_0_ACTION_0_E */
    ,SIP6_10_ENTRY_CNC_INVALID_CLIENT                                                          /* SNET_CNC_CLIENT_IPCL_LOOKUP_0_ACTION_1_E */
    ,SIP6_10_ENTRY_CNC_INVALID_CLIENT                                                          /* SNET_CNC_CLIENT_IPCL_LOOKUP_0_ACTION_2_E */
    ,SIP6_10_ENTRY_CNC_INVALID_CLIENT                                                          /* SNET_CNC_CLIENT_IPCL_LOOKUP_0_ACTION_3_E */
    ,SIP6_10_ENTRY_CNC_CLIENT_ALL( 2,snetCht3CncClientPclLookUpIngrTrigger)                    /* SNET_CNC_CLIENT_IPCL_LOOKUP_1_ACTION_0_E */
    ,SIP6_10_ENTRY_CNC_CLIENT_ALL( 3,snetCht3CncClientPclLookUpIngrTrigger)                    /* SNET_CNC_CLIENT_IPCL_LOOKUP_1_ACTION_1_E */
    ,SIP6_10_ENTRY_CNC_INVALID_CLIENT                                                          /* SNET_CNC_CLIENT_IPCL_LOOKUP_1_ACTION_2_E */
    ,SIP6_10_ENTRY_CNC_INVALID_CLIENT                                                          /* SNET_CNC_CLIENT_IPCL_LOOKUP_1_ACTION_3_E */
    ,SIP6_10_ENTRY_CNC_CLIENT_ALL( 4,snetCht3CncClientPclLookUpIngrTrigger)                    /* SNET_CNC_CLIENT_IPCL_LOOKUP_2_ACTION_0_E */
    ,SIP6_10_ENTRY_CNC_CLIENT_ALL( 5,snetCht3CncClientPclLookUpIngrTrigger)                    /* SNET_CNC_CLIENT_IPCL_LOOKUP_2_ACTION_1_E */
    ,SIP6_10_ENTRY_CNC_INVALID_CLIENT                                                          /* SNET_CNC_CLIENT_IPCL_LOOKUP_2_ACTION_2_E */
    ,SIP6_10_ENTRY_CNC_INVALID_CLIENT                                                          /* SNET_CNC_CLIENT_IPCL_LOOKUP_2_ACTION_3_E */
    ,SIP6_10_ENTRY_CNC_CLIENT_ALL( 7,snetCht3CncClientL2L3VlanIngrTrigger)                     /* SNET_CNC_CLIENT_INGRESS_VLAN_L2_L3_E     */
    ,SIP6_10_ENTRY_CNC_CLIENT_ALL( 8,snetCht3CncClientIpclSourceEPortTrigger)                  /* SNET_CNC_CLIENT_SOURCE_EPORT_E           */
    ,SIP6_10_ENTRY_CNC_CLIENT_ALL( 9,snetCht3CncClientVlanPassDropIngrTrigger)                 /* SNET_CNC_CLIENT_INGRESS_VLAN_PASS_DROP_E */
    ,SIP6_10_ENTRY_CNC_CLIENT_ALL(10,snetLion3CncClientPreEgressPacketTypePassDropTrigger)     /* SNET_CNC_CLIENT_PACKET_TYPE_PASS_DROP_E  */
    ,SIP6_10_ENTRY_CNC_CLIENT_ALL(11,snetCht3CncClientVlanPassDropEgrTrigger)                  /* SNET_CNC_CLIENT_EGRESS_VLAN_PASS_DROP_E  */
    ,SIP6_10_ENTRY_CNC_CLIENT_ALL(12,snetCht3CncClientTxqQueuePassDropEgrTrigger)              /* SNET_CNC_CLIENT_EGRESS_QUEUE_PASS_DROP_AND_QCN_PASS_DROP_E */
    ,SIP6_10_ENTRY_CNC_CLIENT_ALL(13,snetCht3CncClientArpTblTrigger)                           /* SNET_CNC_CLIENT_ARP_INDEX_E  */
    ,SIP6_10_ENTRY_CNC_CLIENT_ALL(14,snetCht3CncClientTunnelStartTrigger)                      /* SNET_CNC_CLIENT_TUNNEL_START_INDEX_E  */
    ,SIP6_10_ENTRY_CNC_CLIENT_ALL(15,snetCht3CncClientHaTargetEPortTrigger)                    /* SNET_CNC_CLIENT_TARGET_EPORT_E  */
    ,SIP6_10_ENTRY_CNC_CLIENT_ALL(16,snetCht3CncClientPclEgrTrigger)                           /* SNET_CNC_CLIENT_EPCL_ACTION_0_E  */
    ,SIP6_10_ENTRY_CNC_CLIENT_ALL(17,snetCht3CncClientPclEgrTrigger)                           /* SNET_CNC_CLIENT_EPCL_ACTION_1_E  */
    ,SIP6_10_ENTRY_CNC_INVALID_CLIENT                                                          /* SNET_CNC_CLIENT_EPCL_ACTION_2_E  */
    ,SIP6_10_ENTRY_CNC_INVALID_CLIENT                                                          /* SNET_CNC_CLIENT_EPCL_ACTION_3_E  */
    ,SIP6_10_ENTRY_CNC_INVALID_CLIENT                                                          /* SNET_CNC_CLIENT_TRAFFIC_MANAGER_PASS_DROP_E */
    ,SIP6_10_ENTRY_CNC_INVALID_CLIENT                                                          /* SNET_CNC_CLIENT_TTI_ACTION_2_E */
    ,SIP6_10_ENTRY_CNC_INVALID_CLIENT                                                          /* SNET_CNC_CLIENT_TTI_ACTION_3_E */
    ,SIP6_10_ENTRY_CNC_INVALID_CLIENT                                                          /* SNET_CNC_PIPE_DEVICE_CLIENT_PCP_DEST_INDEX_E        */
    ,SIP6_10_ENTRY_CNC_INVALID_CLIENT                                                          /* SNET_CNC_PIPE_DEVICE_CLIENT_PPA_TRG_PORT_PKT_TYPE_E */
    ,SIP6_10_ENTRY_CNC_CLIENT_ALL( 6,snetCht3CncClientEgfPassDropTrigger)                      /* SNET_SIP6_CNC_CLIENT_EGF_EVLAN_E  */
    ,SIP6_10_ENTRY_CNC_CLIENT_ALL(30,snetSip6CncClientPreqQueueStatisticTrigger)               /* SNET_SIP6_CNC_CLIENT_PREQ_QUEUE_STATISTICS_E */
    ,SIP6_10_ENTRY_CNC_CLIENT_ALL(31,snetSip6CncClientPreqPortStatisticTrigger)                /* SNET_SIP6_CNC_CLIENT_PREQ_PORT_STATISTICS_E  */
    ,SIP6_10_ENTRY_CNC_CLIENT_ALL(18,snetCht3CncClientErepPacketTypePassDropTrigger)           /* SNET_SIP6_CNC_CLIENT_EREP_PACKET_TYPE_PASS_DROP_E  */
    ,SIP6_10_ENTRY_CNC_INVALID_CLIENT                                                          /* SNET_SIP6_10_CNC_CLIENT_PHA_E                      */
};

/**
* @internal sip6_10_convertClientToBitIndex function
* @endinternal
*
* @brief   convert enum of SNET_CNC_CLIENT_E to INTERNAL_SPECIFIC_HW_VALUES__SNET_CHT_CNC_CLIENT_E
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] clientUnified            - the unified client
*                                       the bit index in the HW
* @param[in] cncUnitIndex             - the CNC unit index (0..3) (Sip6_10 devices)
*/
static INTERNAL_SPECIFIC_HW_VALUES__SNET_CHT_CNC_CLIENT_E sip6_10_convertClientToBitIndex
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SNET_CNC_CLIENT_E    clientUnified,
    IN GT_U32               cncUnitIndex
)
{
    SIP6_10_CNC_CLIENT_IN_BLOCK *cncBlockToClientInfoPtr;

    cncBlockToClientInfoPtr = SMEM_CHT_IS_SIP6_30_GET(devObjPtr) ?  &gCncClientTrigFuncArrSip6_30[clientUnified] :
                              SMEM_CHT_IS_SIP6_15_GET(devObjPtr) ?  &gCncClientTrigFuncArrSip6_15[clientUnified] :
                              &gCncClientTrigFuncArrSip6_10[clientUnified] ;

    if(cncBlockToClientInfoPtr->cncClientTriggerFunc == NULL)
    {
        skernelFatalError("sip6_10_convertClientToBitIndex : unknown client [%d] \n" , clientUnified);
    }

    if(cncUnitIndex >= MAX_CNC_UNITS_CNS)
    {
        skernelFatalError("sip6_10_convertClientToBitIndex : unknown cncUnitIndex [%d] \n" , cncUnitIndex);
    }

    /* if return 'GT_NA' the caller will ignore this client for this block ! */
    return cncBlockToClientInfoPtr->clientHwBitArr[cncUnitIndex];
}

/**
* @internal convertClientToBitIndex function
* @endinternal
*
* @brief   convert enum of SNET_CNC_CLIENT_E to INTERNAL_SPECIFIC_HW_VALUES__SNET_CHT_CNC_CLIENT_E
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] clientUnified            - the unified client
*                                       the bit index in the HW
* @param[in] cncUnitIndex             - the CNC unit index (0..1) (Sip5    devices)
*                                       the CNC unit index (0..3) (Sip6_10 devices)
*/
static INTERNAL_SPECIFIC_HW_VALUES__SNET_CHT_CNC_CLIENT_E convertClientToBitIndex
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SNET_CNC_CLIENT_E    clientUnified,
    IN GT_U32               cncUnitIndex
)
{
    if(SMEM_CHT_IS_SIP6_10_GET(devObjPtr))
    {
        return sip6_10_convertClientToBitIndex(devObjPtr,clientUnified,cncUnitIndex);
    }

    if(SMEM_IS_PIPE_FAMILY_GET(devObjPtr))
    {
        switch(clientUnified)
        {
            case SNET_CNC_PIPE_DEVICE_CLIENT_PCP_DEST_INDEX_E:
                return INTERNAL_SPECIFIC_HW_VALUES__PIPE_SNET_CNC_PIPE_DEVICE_CLIENT_PCP_DEST_INDEX_E;
            case SNET_CNC_PIPE_DEVICE_CLIENT_PPA_TRG_PORT_PKT_TYPE_E:
                return INTERNAL_SPECIFIC_HW_VALUES__PIPE_SNET_CNC_PIPE_DEVICE_CLIENT_PPA_TRG_PORT_PKT_TYPE_E;
            case SNET_CNC_CLIENT_EGRESS_QUEUE_PASS_DROP_AND_QCN_PASS_DROP_E:
                return INTERNAL_SPECIFIC_HW_VALUES__PIPE_SNET_CNC_CLIENT_EGRESS_QUEUE_PASS_DROP_AND_QCN_PASS_DROP_E;
            default:
                break;
        }
    }
    else
    if(0 == SMEM_CHT_IS_SIP5_GET(devObjPtr))
    {
        /* ch3 .. Lion2 */
        switch(clientUnified)
        {
            case SNET_CNC_CLIENT_TTI_ACTION_0_E:
                return INTERNAL_SPECIFIC_HW_VALUES__SNET_XCAT_CNC_CLIENT_TUNNEL_TERMINATION_E;
            case SNET_CNC_CLIENT_IPCL_LOOKUP_0_ACTION_0_E:
                return INTERNAL_SPECIFIC_HW_VALUES__SNET_CHT3_CNC_CLIENT_PCL0_0_LOOKUP_INGR_E;
            case SNET_CNC_CLIENT_IPCL_LOOKUP_1_ACTION_0_E:
                return INTERNAL_SPECIFIC_HW_VALUES__SNET_CHT3_CNC_CLIENT_PCL0_1_LOOKUP_INGR_E;
            case SNET_CNC_CLIENT_IPCL_LOOKUP_2_ACTION_0_E:
                return INTERNAL_SPECIFIC_HW_VALUES__SNET_CHT3_CNC_CLIENT_PCL1_LOOKUP_INGR_E;
            case SNET_CNC_CLIENT_INGRESS_VLAN_L2_L3_E:
                return INTERNAL_SPECIFIC_HW_VALUES__SNET_CHT3_CNC_CLIENT_L2_L3_VLAN_INGR_E;
            case SNET_CNC_CLIENT_INGRESS_VLAN_PASS_DROP_E:
                return INTERNAL_SPECIFIC_HW_VALUES__SNET_CHT3_CNC_CLIENT_VLAN_PASS_DROP_INGR_E;
            case SNET_CNC_CLIENT_EGRESS_VLAN_PASS_DROP_E:
                return INTERNAL_SPECIFIC_HW_VALUES__SNET_CHT3_CNC_CLIENT_VLAN_PASS_DROP_EGR_E;
            case SNET_CNC_CLIENT_EGRESS_QUEUE_PASS_DROP_AND_QCN_PASS_DROP_E:
                return INTERNAL_SPECIFIC_HW_VALUES__SNET_CHT3_CNC_CLIENT_TXQ_QUEUE_PASS_DROP_EGR_E;
            case SNET_CNC_CLIENT_ARP_INDEX_E:
                return INTERNAL_SPECIFIC_HW_VALUES__SNET_CHT3_CNC_CLIENT_ARP_TBL_E;
            case SNET_CNC_CLIENT_TUNNEL_START_INDEX_E:
                return INTERNAL_SPECIFIC_HW_VALUES__SNET_CHT3_CNC_CLIENT_TUNNEL_START_E;
            case SNET_CNC_CLIENT_EPCL_ACTION_0_E:
                return INTERNAL_SPECIFIC_HW_VALUES__SNET_CHT3_CNC_CLIENT_PCL_EGR_E;

            default:
                break;
        }
    }
    else
    if(SMEM_CHT_IS_SIP6_GET(devObjPtr))
    {
        switch(clientUnified)
        {
            case SNET_CNC_CLIENT_TTI_ACTION_0_E:
                return INTERNAL_SPECIFIC_HW_VALUES__SNET_SIP6_CNC_CLIENT_TTI_ACTION_0_E;
            case SNET_CNC_CLIENT_TTI_ACTION_1_E:
                return INTERNAL_SPECIFIC_HW_VALUES__SNET_SIP6_CNC_CLIENT_TTI_ACTION_1_E;
            case SNET_CNC_CLIENT_IPCL_LOOKUP_1_ACTION_0_E:
                return INTERNAL_SPECIFIC_HW_VALUES__SNET_SIP6_CNC_CLIENT_IPCL_LOOKUP_1_ACTION_0_E;
            case SNET_CNC_CLIENT_IPCL_LOOKUP_1_ACTION_1_E:
                return INTERNAL_SPECIFIC_HW_VALUES__SNET_SIP6_CNC_CLIENT_IPCL_LOOKUP_1_ACTION_1_E;
            case SNET_CNC_CLIENT_IPCL_LOOKUP_1_ACTION_2_E:
                return INTERNAL_SPECIFIC_HW_VALUES__SNET_SIP6_CNC_CLIENT_IPCL_LOOKUP_1_ACTION_2_E;
            case SNET_CNC_CLIENT_IPCL_LOOKUP_1_ACTION_3_E:
                return INTERNAL_SPECIFIC_HW_VALUES__SNET_SIP6_CNC_CLIENT_IPCL_LOOKUP_1_ACTION_3_E;
            case SNET_CNC_CLIENT_IPCL_LOOKUP_2_ACTION_0_E:
                return INTERNAL_SPECIFIC_HW_VALUES__SNET_SIP6_CNC_CLIENT_IPCL_LOOKUP_2_ACTION_0_E;
            case SNET_CNC_CLIENT_IPCL_LOOKUP_2_ACTION_1_E:
                return INTERNAL_SPECIFIC_HW_VALUES__SNET_SIP6_CNC_CLIENT_IPCL_LOOKUP_2_ACTION_1_E;
            case SNET_CNC_CLIENT_IPCL_LOOKUP_2_ACTION_2_E:
                return INTERNAL_SPECIFIC_HW_VALUES__SNET_SIP6_CNC_CLIENT_IPCL_LOOKUP_2_ACTION_2_E;
            case SNET_CNC_CLIENT_IPCL_LOOKUP_2_ACTION_3_E:
                return INTERNAL_SPECIFIC_HW_VALUES__SNET_SIP6_CNC_CLIENT_IPCL_LOOKUP_2_ACTION_3_E;
            case SNET_SIP6_CNC_CLIENT_EGF_EVLAN_E:
                return INTERNAL_SPECIFIC_HW_VALUES__SNET_SIP6_CNC_CLIENT_EGF_EVLAN_E;
            case SNET_SIP6_CNC_CLIENT_EREP_PACKET_TYPE_PASS_DROP_E:
                return INTERNAL_SPECIFIC_HW_VALUES__SNET_SIP6_CNC_CLIENT_EREP_PACKET_TYPE_PASS_DROP_E;
            case SNET_CNC_CLIENT_INGRESS_VLAN_L2_L3_E:
                return INTERNAL_SPECIFIC_HW_VALUES__SNET_SIP6_CNC_CLIENT_INGRESS_VLAN_L2_L3_E;
            case SNET_CNC_CLIENT_SOURCE_EPORT_E:
                return INTERNAL_SPECIFIC_HW_VALUES__SNET_SIP6_CNC_CLIENT_SOURCE_EPORT_E;
            case SNET_CNC_CLIENT_INGRESS_VLAN_PASS_DROP_E:
                return INTERNAL_SPECIFIC_HW_VALUES__SNET_SIP6_CNC_CLIENT_INGRESS_VLAN_PASS_DROP_E;
            case SNET_CNC_CLIENT_PACKET_TYPE_PASS_DROP_E:
                return INTERNAL_SPECIFIC_HW_VALUES__SNET_SIP6_CNC_CLIENT_PACKET_TYPE_PASS_DROP_E;
            case SNET_CNC_CLIENT_EGRESS_VLAN_PASS_DROP_E:
                return INTERNAL_SPECIFIC_HW_VALUES__SNET_SIP6_CNC_CLIENT_EGRESS_VLAN_PASS_DROP_E;
            case SNET_CNC_CLIENT_EGRESS_QUEUE_PASS_DROP_AND_QCN_PASS_DROP_E:
                return INTERNAL_SPECIFIC_HW_VALUES__SNET_SIP6_CNC_CLIENT_EGRESS_QUEUE_PASS_DROP_AND_QCN_PASS_DROP_E;
            case SNET_CNC_CLIENT_ARP_INDEX_E:
                return INTERNAL_SPECIFIC_HW_VALUES__SNET_SIP6_CNC_CLIENT_ARP_INDEX_E;
            case SNET_CNC_CLIENT_TUNNEL_START_INDEX_E:
                return INTERNAL_SPECIFIC_HW_VALUES__SNET_SIP6_CNC_CLIENT_TUNNEL_START_INDEX_E;
            case SNET_CNC_CLIENT_TARGET_EPORT_E:
                return INTERNAL_SPECIFIC_HW_VALUES__SNET_SIP6_CNC_CLIENT_TARGET_EPORT_E;
            case SNET_CNC_CLIENT_EPCL_ACTION_0_E:
                return INTERNAL_SPECIFIC_HW_VALUES__SNET_SIP6_CNC_CLIENT_EPCL_ACTION_0_E;
            case SNET_CNC_CLIENT_EPCL_ACTION_1_E:
                return INTERNAL_SPECIFIC_HW_VALUES__SNET_SIP6_CNC_CLIENT_EPCL_ACTION_1_E;
            case SNET_CNC_CLIENT_EPCL_ACTION_2_E:
                return INTERNAL_SPECIFIC_HW_VALUES__SNET_SIP6_CNC_CLIENT_EPCL_ACTION_2_E;
            case SNET_CNC_CLIENT_EPCL_ACTION_3_E:
                return INTERNAL_SPECIFIC_HW_VALUES__SNET_SIP6_CNC_CLIENT_EPCL_ACTION_3_E;
            case SNET_CNC_CLIENT_TTI_ACTION_2_E:
                return INTERNAL_SPECIFIC_HW_VALUES__SNET_SIP6_CNC_CLIENT_TTI_ACTION_2_E;
            case SNET_CNC_CLIENT_TTI_ACTION_3_E:
                return INTERNAL_SPECIFIC_HW_VALUES__SNET_SIP6_CNC_CLIENT_TTI_ACTION_3_E;
            case SNET_SIP6_CNC_CLIENT_PREQ_QUEUE_STATISTICS_E:
                return INTERNAL_SPECIFIC_HW_VALUES__SNET_SIP6_CNC_CLIENT_PREQ_QUEUE_STATISTICS_E;
            case SNET_SIP6_CNC_CLIENT_PREQ_PORT_STATISTICS_E:
                return INTERNAL_SPECIFIC_HW_VALUES__SNET_SIP6_CNC_CLIENT_PREQ_PORT_STATISTICS_E;
            default:
                break;
        }
    }
    else
    {
        /* Lion3,bobcat2*/
        switch(clientUnified)
        {
            case SNET_CNC_CLIENT_TTI_ACTION_0_E:
                return INTERNAL_SPECIFIC_HW_VALUES__SNET_LION3_CNC_CLIENT_TTI_ACTION_0_E;
            case SNET_CNC_CLIENT_TTI_ACTION_1_E:
                return INTERNAL_SPECIFIC_HW_VALUES__SNET_LION3_CNC_CLIENT_TTI_ACTION_1_E;
            case SNET_CNC_CLIENT_IPCL_LOOKUP_0_ACTION_0_E:
                return INTERNAL_SPECIFIC_HW_VALUES__SNET_LION3_CNC_CLIENT_IPCL_LOOKUP_0_ACTION_0_E;
            case SNET_CNC_CLIENT_IPCL_LOOKUP_0_ACTION_1_E:
                return INTERNAL_SPECIFIC_HW_VALUES__SNET_LION3_CNC_CLIENT_IPCL_LOOKUP_0_ACTION_1_E;
            case SNET_CNC_CLIENT_IPCL_LOOKUP_0_ACTION_2_E:
                return INTERNAL_SPECIFIC_HW_VALUES__SNET_LION3_CNC_CLIENT_IPCL_LOOKUP_0_ACTION_2_E;
            case SNET_CNC_CLIENT_IPCL_LOOKUP_0_ACTION_3_E:
                return INTERNAL_SPECIFIC_HW_VALUES__SNET_LION3_CNC_CLIENT_IPCL_LOOKUP_0_ACTION_3_E;
            case SNET_CNC_CLIENT_IPCL_LOOKUP_1_ACTION_0_E:
                return INTERNAL_SPECIFIC_HW_VALUES__SNET_LION3_CNC_CLIENT_IPCL_LOOKUP_1_ACTION_0_E;
            case SNET_CNC_CLIENT_IPCL_LOOKUP_1_ACTION_1_E:
                return INTERNAL_SPECIFIC_HW_VALUES__SNET_LION3_CNC_CLIENT_IPCL_LOOKUP_1_ACTION_1_E;
            case SNET_CNC_CLIENT_IPCL_LOOKUP_1_ACTION_2_E:
                return INTERNAL_SPECIFIC_HW_VALUES__SNET_LION3_CNC_CLIENT_IPCL_LOOKUP_1_ACTION_2_E;
            case SNET_CNC_CLIENT_IPCL_LOOKUP_1_ACTION_3_E:
                return INTERNAL_SPECIFIC_HW_VALUES__SNET_LION3_CNC_CLIENT_IPCL_LOOKUP_1_ACTION_3_E;
            case SNET_CNC_CLIENT_IPCL_LOOKUP_2_ACTION_0_E:
                return INTERNAL_SPECIFIC_HW_VALUES__SNET_LION3_CNC_CLIENT_IPCL_LOOKUP_2_ACTION_0_E;
            case SNET_CNC_CLIENT_IPCL_LOOKUP_2_ACTION_1_E:
                return INTERNAL_SPECIFIC_HW_VALUES__SNET_LION3_CNC_CLIENT_IPCL_LOOKUP_2_ACTION_1_E;
            case SNET_CNC_CLIENT_IPCL_LOOKUP_2_ACTION_2_E:
                return INTERNAL_SPECIFIC_HW_VALUES__SNET_LION3_CNC_CLIENT_IPCL_LOOKUP_2_ACTION_2_E;
            case SNET_CNC_CLIENT_IPCL_LOOKUP_2_ACTION_3_E:
                return INTERNAL_SPECIFIC_HW_VALUES__SNET_LION3_CNC_CLIENT_IPCL_LOOKUP_2_ACTION_3_E;
            case SNET_CNC_CLIENT_INGRESS_VLAN_L2_L3_E:
                return INTERNAL_SPECIFIC_HW_VALUES__SNET_LION3_CNC_CLIENT_INGRESS_VLAN_L2_L3_E;
            case SNET_CNC_CLIENT_SOURCE_EPORT_E:
                return INTERNAL_SPECIFIC_HW_VALUES__SNET_LION3_CNC_CLIENT_SOURCE_EPORT_E;
            case SNET_CNC_CLIENT_INGRESS_VLAN_PASS_DROP_E:
                return INTERNAL_SPECIFIC_HW_VALUES__SNET_LION3_CNC_CLIENT_INGRESS_VLAN_PASS_DROP_E;
            case SNET_CNC_CLIENT_PACKET_TYPE_PASS_DROP_E:
                return INTERNAL_SPECIFIC_HW_VALUES__SNET_LION3_CNC_CLIENT_PACKET_TYPE_PASS_DROP_E;
            case SNET_CNC_CLIENT_EGRESS_VLAN_PASS_DROP_E:
                return INTERNAL_SPECIFIC_HW_VALUES__SNET_LION3_CNC_CLIENT_EGRESS_VLAN_PASS_DROP_E;
            case SNET_CNC_CLIENT_EGRESS_QUEUE_PASS_DROP_AND_QCN_PASS_DROP_E:
                return INTERNAL_SPECIFIC_HW_VALUES__SNET_LION3_CNC_CLIENT_EGRESS_QUEUE_PASS_DROP_AND_QCN_PASS_DROP_E;
            case SNET_CNC_CLIENT_ARP_INDEX_E:
                return INTERNAL_SPECIFIC_HW_VALUES__SNET_LION3_CNC_CLIENT_ARP_INDEX_E;
            case SNET_CNC_CLIENT_TUNNEL_START_INDEX_E:
                return INTERNAL_SPECIFIC_HW_VALUES__SNET_LION3_CNC_CLIENT_TUNNEL_START_INDEX_E;
            case SNET_CNC_CLIENT_TARGET_EPORT_E:
                return INTERNAL_SPECIFIC_HW_VALUES__SNET_LION3_CNC_CLIENT_TARGET_EPORT_E;
            case SNET_CNC_CLIENT_EPCL_ACTION_0_E:
                return INTERNAL_SPECIFIC_HW_VALUES__SNET_LION3_CNC_CLIENT_EPCL_ACTION_0_E;
            case SNET_CNC_CLIENT_EPCL_ACTION_1_E:
                return INTERNAL_SPECIFIC_HW_VALUES__SNET_LION3_CNC_CLIENT_EPCL_ACTION_1_E;
            case SNET_CNC_CLIENT_EPCL_ACTION_2_E:
                return INTERNAL_SPECIFIC_HW_VALUES__SNET_LION3_CNC_CLIENT_EPCL_ACTION_2_E;
            case SNET_CNC_CLIENT_EPCL_ACTION_3_E:
                return INTERNAL_SPECIFIC_HW_VALUES__SNET_LION3_CNC_CLIENT_EPCL_ACTION_3_E;
            case SNET_CNC_CLIENT_TRAFFIC_MANAGER_PASS_DROP_E:
                return INTERNAL_SPECIFIC_HW_VALUES__SNET_LION3_CNC_CLIENT_TRAFFIC_MANAGER_PASS_DROP_E;
            case SNET_CNC_CLIENT_TTI_ACTION_2_E:
                if(!SMEM_CHT_IS_SIP5_15_GET(devObjPtr)) break;
                if(!SMEM_CHT_IS_SIP5_20_GET(devObjPtr))
                {
                    /* SIP5.15 */
                    return INTERNAL_SPECIFIC_HW_VALUES__SNET_LION3_CNC_CLIENT_TTI_ACTION_0_E;
                }
                return INTERNAL_SPECIFIC_HW_VALUES__SNET_LION3_CNC_CLIENT_TTI_ACTION_2_E;
            case SNET_CNC_CLIENT_TTI_ACTION_3_E:
                if(!SMEM_CHT_IS_SIP5_15_GET(devObjPtr)) break;
                if(!SMEM_CHT_IS_SIP5_20_GET(devObjPtr))
                {
                    /* SIP5.15 */
                    return INTERNAL_SPECIFIC_HW_VALUES__SNET_LION3_CNC_CLIENT_TTI_ACTION_1_E;
                }
                return INTERNAL_SPECIFIC_HW_VALUES__SNET_LION3_CNC_CLIENT_TTI_ACTION_3_E;
            default:
                break;
        }
    }

    skernelFatalError("convertClientToBitIndex : unknown client [%d] \n" , clientUnified);
    return 0;
}



/**
* @internal snetCht3CncIncrement function
* @endinternal
*
* @brief   Increment CNC counter - packets and bytes
*
* @param[in] regPtr                   - pointer to CNC counter register data.
* @param[in] block                    -  index
* @param[in] cncFormat                - CNC modes of counter formats
* @param[in] cncCounterAddPtr         - pointer to increment CNC counter value
* @param[in] wrapEn                   - wraparound counter enable/disable
* @param[in] clientInfoPtr            - pointer to CNC client info.
*
* @retval GT_OK                    - CNC packet counter successfully incremented
* @retval GT_FULL                  - CNC packet counter is full
*/
static GT_STATUS snetCht3CncIncrement
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    GT_U32 * regPtr,
    GT_U32 block,
    CNC_COUNTER_FORMAT_ENT cncFormat,
    SNET_CHT3_CNC_COUNTER_STC  * cncCounterAddPtr,
    GT_U32 wrapEn,
    IN SNET_CHT3_CNC_CLIENT_INFO_STC * clientInfoPtr
)
{
    SNET_CHT3_CNC_COUNTER_STC  cncCounter;
    GT_STATUS status = GT_OK;
    GT_BIT      do64BitsByteWrapAround = 0;
    GT_BIT      incrementPacketCount = 0;

    /* Read the 64-bit HW counter to SW format */
    snetCht3CncCounterHwRead(cncFormat, regPtr, &cncCounter);

    __LOG_NO_LOCATION_META_DATA(("CNC : increment counter :before update : packetCount.l[0] value [0x%8.8x] \n",cncCounter.packetCount.l[0]));
    __LOG_NO_LOCATION_META_DATA(("CNC : increment counter :before update : packetCount.l[1] value [0x%8.8x]\n" ,cncCounter.packetCount.l[1]));
    __LOG_NO_LOCATION_META_DATA(("CNC : increment counter :before update : byteCount.l[0] value [0x%8.8x] \n",cncCounter.byteCount.l[0]));
    __LOG_NO_LOCATION_META_DATA(("CNC : increment counter :before update : byteCount.l[1] value [0x%8.8x]\n" ,cncCounter.byteCount.l[1]));

    /* test if wraparound is disabled and maximum value already reached */
    status = snetCht3CncCounterWrapAroundCheck(wrapEn, cncFormat, &cncCounter,0);
    /* Maximum value reached */
    if(status != GT_OK)
    {
        /* Wraparound disabled */
        if(wrapEn == 0)
        {
            __LOG_NO_LOCATION_META_DATA(("CNC : increment counter :Wraparound disabled \n"));
            return status;
        }
    }

    if(cncFormat == CNC_COUNTER_FORMAT_PKT_44_MAX_SIZE_20_E)
    {
        if(!clientInfoPtr->debug_is_expected_counter_format_CNC_COUNTER_FORMAT_PKT_44_MAX_SIZE_20_E)
        {
            __LOG_NO_LOCATION_META_DATA(("CNC : WARNING : the client is NOT expected to be in counting mode PKT_44_MAX_SIZE_20 ! (counting values may be unexpected !) \n"));
        }

        if(clientInfoPtr->triggered)
        {
            /* need to update the 'packet count' section (increment by one per packet) */
            incrementPacketCount = 1;
            __LOG_NO_LOCATION_META_DATA(("CNC : (counting mode PKT_44_MAX_SIZE_20) 'triggered' : allow to update the 'packet count' \n"));
        }
        else
        {
            __LOG_NO_LOCATION_META_DATA(("CNC : (counting mode PKT_44_MAX_SIZE_20) 'not triggered' : NOT allow to update the 'packet count' \n"));
        }
    }
    else
    {
        if(clientInfoPtr->debug_is_expected_counter_format_CNC_COUNTER_FORMAT_PKT_44_MAX_SIZE_20_E)
        {
            __LOG_NO_LOCATION_META_DATA(("CNC : WARNING : the client is expected to be in counting mode PKT_44_MAX_SIZE_20 , but is not . (counting values may be unexpected !) \n"));
        }

        incrementPacketCount = 1;
    }

    if(incrementPacketCount)
    {
        /* Increment CNC packet counter */
        cncCounter.packetCount =
            prvSimMathAdd64(cncCounter.packetCount, cncCounterAddPtr->packetCount);
    }

    if((cncFormat == CNC_COUNTER_FORMAT_PKT_0_BC_64_E) &&
        cncCounter.byteCount.l[1] == 0xFFFFFFFF &&
        (0xFFFFFFFF - cncCounter.byteCount.l[0]) < cncCounterAddPtr->byteCount.l[0])
    {
        /* even 64 bits 'bytes' counter will wrap around */
        do64BitsByteWrapAround = 1;
    }

    if(cncFormat == CNC_COUNTER_FORMAT_PKT_44_MAX_SIZE_20_E)
    {
        /* check if need to update the 'max value' ... regardless to 'clientInfoPtr->triggered' !!! */
        if(cncCounter.byteCount.l[0]/* old value*/ < cncCounterAddPtr->byteCount.l[0] /* new value*/)
        {
            __LOG_NO_LOCATION_META_DATA(("CNC : (counting mode PKT_44_MAX_SIZE_20) new 'max value' was set [0x%5.5x]\n",
                cncCounterAddPtr->byteCount));
            cncCounter.byteCount = cncCounterAddPtr->byteCount;/* update the 'max value' */
        }
    }
    else
    {
        /* Increment CNC byte counter */
        cncCounter.byteCount =
            prvSimMathAdd64(cncCounter.byteCount, cncCounterAddPtr->byteCount);
    }

    /* Test if wraparound is disabled and packet or maximum CNC byte count value have reached */
    status = snetCht3CncCounterWrapAroundCheck(wrapEn, cncFormat, &cncCounter,do64BitsByteWrapAround);

    __LOG_NO_LOCATION_META_DATA(("CNC : increment counter :after update : packetCount.l[0] value [0x%8.8x]\n",cncCounter.packetCount.l[0]));
    __LOG_NO_LOCATION_META_DATA(("CNC : increment counter :after update : packetCount.l[1] value [0x%8.8x]\n",cncCounter.packetCount.l[1]));
    __LOG_NO_LOCATION_META_DATA(("CNC : increment counter :after update : byteCount.l[0] value [0x%8.8x]\n" ,cncCounter.byteCount.l[0]));
    __LOG_NO_LOCATION_META_DATA(("CNC : increment counter :after update : byteCount.l[1] value [0x%8.8x]\n" ,cncCounter.byteCount.l[1]));

    /* Write the 64-bit HW counter to HW format */
    snetCht3CncCounterHwWrite(cncFormat, regPtr, &cncCounter);

    return status;
}

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
* @param[in] cncUnitIndex             the CNC unit index (0/1) (Sip5 devices)
*/
GT_VOID snetCht3CncBlockReset
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32 block,
    IN GT_U32 entryStart,
    IN GT_U32 entryNum,
    IN GT_U32 cncUnitIndex
)
{
    GT_U32 regAddr;                 /* register address */
    GT_U32 * regPtr;                /* register data pointer */
    GT_U32 clearByRead;             /* enable setting of a configurable value
                                       to the Counters after read by the CPU */
    GT_U32 entry;                   /* CNC block entry index */
    GT_U32 totalEntries;

    totalEntries = entryStart + entryNum;

    if ( entryNum < 1 ||
         totalEntries > devObjPtr->cncBlockMaxRangeIndex)
    {
        skernelFatalError("Wrong block entry start index or number of entries\n");
        return;
    }

    smemRegFldGet(devObjPtr, SMEM_CHT3_CNC_GLB_CONF_REG(devObjPtr,cncUnitIndex), 1, 1, &clearByRead);
    if (clearByRead == 0)
    {
        return;
    }

    /* pointer to CNC default value */
    regPtr = smemMemGet(devObjPtr, SMEM_CHT3_CNC_ROC_WORD0_REG(devObjPtr,cncUnitIndex));

    /* Get start address of entry in the block */
    regAddr = SMEM_CHT3_CNC_BLOCK_COUNTER_TBL_MEM(devObjPtr,block, entryStart , cncUnitIndex);

    for (entry = entryStart; entry < totalEntries; entry++, regAddr += (CNC_BLOCK_ENTRY_WORDS_CNS*4))
    {
        /* copy from the 'Default value' into the entry */
        smemMemSet(devObjPtr, regAddr, regPtr, CNC_BLOCK_ENTRY_WORDS_CNS);
    }
}


/**
* @internal cncByteCountGet function
* @endinternal
*
* @brief   CNC byte count get
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] descrPtr                 - pointer to the frame's descriptor.
* @param[in] clientInfoPtr            - pointer to CNC client info.
*                                       value to be incremented (bytes of L2/L2/packets)
*/
static GT_STATUS cncByteCountGet
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN SNET_CHT3_CNC_CLIENT_INFO_STC * clientInfoPtr
)
{
    DECLARE_FUNC_NAME(cncByteCountGet);

    GT_U32  cncFinalSizeToCount;/* final size to count */
    GT_U32  byteCount,l2HeaderSize,l3ByteCount,egressByteCount;
    GT_U32  dsaBytes;/* number of dsa bytes*/
    GT_BOOL forceIngressByteCountLogic;/* indication to use 'ingress logic' even for egress pipe units */
    GT_BOOL unitIsHa;/* is the client is from HA unit */

    __LOG(("CNC : Start calculate the byte count \n"));

    if(clientInfoPtr->byteCntMode == SMEM_CHT3_CNC_BYTE_CNT_FLEX_VALUE_E)
    {
        __LOG(("'byte count' was already set explicitly by the unit to [0x%x] \n" ,
            clientInfoPtr->explicitByteCountValue));
        return clientInfoPtr->explicitByteCountValue;
    }

    if(0 == SMEM_CHT_IS_SIP5_GET(devObjPtr))
    {
        switch(clientInfoPtr->client)
        {
            case SNET_CNC_CLIENT_EPCL_ACTION_0_E:
                /* the EPCL unit never knew the ingress byte count */
                forceIngressByteCountLogic = GT_FALSE;
                break;
            default:
                forceIngressByteCountLogic = GT_TRUE;
                break;
        }
    }
    else
    {
        forceIngressByteCountLogic = GT_FALSE;
    }

    switch(clientInfoPtr->client)
    {
        case SNET_CNC_CLIENT_ARP_INDEX_E :
        case SNET_CNC_CLIENT_TUNNEL_START_INDEX_E :
        case SNET_CNC_CLIENT_TARGET_EPORT_E :
            unitIsHa = GT_TRUE;
            break;
        default:
            unitIsHa = GT_FALSE;
            break;
    }

    if(descrPtr->egressByteCount)
    {
        __LOG_PARAM(descrPtr->haToEpclInfo.paddingZeroForLess64BytesLength);
        __LOG_PARAM(descrPtr->egressByteCount);

        __LOG(("CNC : for Egress Pipe after packet length and format is known \n"));

        if(descrPtr->haToEpclInfo.paddingZeroForLess64BytesLength)
        {
            __LOG(("CNC : use byte count without MAC padding \n"));
        }

        egressByteCount = descrPtr->egressByteCount -
            descrPtr->haToEpclInfo.paddingZeroForLess64BytesLength;

    }
    else
    {
        egressByteCount = 0;

        __LOG(("CNC : for Ingress pipe or Egress Pipe before egress packet length and format is known \n"));
    }

    __LOG_PARAM(forceIngressByteCountLogic);
    __LOG_PARAM(descrPtr->byteCount);
    __LOG_PARAM(descrPtr->tunnelTerminated);
    __LOG_PARAM(descrPtr->tunnelStart);

    if(egressByteCount == 0 ||
       forceIngressByteCountLogic == GT_TRUE)
    {
        if(descrPtr->tunnelTerminated == 0 ||
           clientInfoPtr->client == SNET_CNC_CLIENT_TUNNEL_START_INDEX_E)
        {
            byteCount = descrPtr->byteCount;

            if(clientInfoPtr->client == SNET_CNC_CLIENT_TUNNEL_START_INDEX_E)
            {
                __LOG(("CNC : Tunnel start packet use ingress L2 byte count [%d] (using descrPtr->byteCount) \n",
                    byteCount));
            }
            else
            {
                __LOG(("CNC : non TT (non tunnel terminated) packet use L2 byte count [%d]  \n",
                    byteCount));
            }


            if((clientInfoPtr->client >= SNET_CNC_CLIENT_IPCL_LOOKUP_0_ACTION_0_E ||
                clientInfoPtr->client <= SNET_CNC_CLIENT_IPCL_LOOKUP_2_ACTION_3_E) &&
                descrPtr->marvellTagged &&
                clientInfoPtr->byteCntMode == SMEM_CHT3_CNC_BYTE_CNT_L2_MODE_E)
            {
                dsaBytes = 4 * (descrPtr->marvellTaggedExtended + 1);

                if(0 == SMEM_CHT_IS_SIP5_GET(devObjPtr))
                {
                    __LOG(("CNC : IPCL : L2 counting mode ignore DSA [%d] bytes  \n",
                        dsaBytes));
                    /* PCL : L2 counting check whether the packet is DSA tagged or not. */
                    byteCount -= dsaBytes;
                }
                else  /* according to ebook this is diff from sip 4.0 */
                {
                    __LOG(("CNC : IPCL : L2 counting mode NOT ignore DSA [%d] bytes \n",
                        dsaBytes));
                }
            }
        }
        else
        {
            byteCount = descrPtr->origByteCount;

            __LOG(("CNC : TT (tunnel terminated) packet use L2 'orig' byte count [%d]  \n",
                byteCount));
        }

        l2HeaderSize = descrPtr->l2HeaderSize;

        if(SMEM_CHT_IS_SIP5_GET(devObjPtr) && descrPtr->tunnelTerminated &&
            descrPtr->innerPacketType == SKERNEL_INNER_PACKET_TYPE_ETHERNET_WITH_CRC)
        {
            __LOG(("CNC : for diff between L3 and L2 ignore 4 CRC bytes of TT with Ethernet passenger (beside CRC of packet) \n"));
            l2HeaderSize += 4;/* this is the CRC of the passenger */
        }

        l3ByteCount = byteCount - l2HeaderSize;

        if(clientInfoPtr->isErrata_NotCountCrc &&
           descrPtr->tunnelTerminated == 0)
        {
            /* JIRA : PCL-829 : pcl clients count 4 bytes less that tti client in cnc counetrs in Falcon device */
            __LOG(("Erratum : byteCount for 'L3 and L2 counting' is 4 bytes less (removing 4 bytes 'CRC' by mistake) \n"));
            byteCount   -= 4;
            l3ByteCount -= 4;
        }
    }
    else
    {
        /* egress pipe (HA,EPCL) ... we need to use the egress byte count */
        byteCount       = egressByteCount;

        /* calc the L3 mode */

        if(descrPtr->tunnelTerminated || descrPtr->tunnelStart == 0)
        {
            l2HeaderSize = descrPtr->haToEpclInfo.l3StartOffsetPtr -
                           descrPtr->haToEpclInfo.macDaSaPtr;

            if(descrPtr->tunnelTerminated)
            {
                __LOG(("CNC : use L3 of the passenger \n"));
                if(descrPtr->innerPacketType == SKERNEL_INNER_PACKET_TYPE_ETHERNET_WITH_CRC)
                {
                    __LOG(("CNC : L3 of passenger ignore 4 CRC bytes of Ethernet passenger (beside CRC of packet) \n"));
                    l2HeaderSize += 4;/* this is the CRC of the passenger */
                }
            }
            else /* non TS and non TT */
            {
                __LOG(("CNC : use L3 of the packet \n"));
            }

        }
        else /* descrPtr->tunnelStart == 1 && descrPtr->tunnelTerminated == 0 */
        {
            l2HeaderSize = descrPtr->haToEpclInfo.l3StartOffsetPtr - /* the L3 of passenger */
                           descrPtr->haToEpclInfo.tunnelStartL2StartOffsetPtr;/* the start of egress packet */
            __LOG(("CNC : use L3 of the passenger of the TS (tunnel start) \n"));
            /*for TS that is not with ethernet passenger this is NULL */
            /*for TS that is     with ethernet passenger this is the mac da of the passenger */
            /*for non-TS that this is the mac da of the packet */
            if(descrPtr->haToEpclInfo.macDaSaPtr)
            {
                /* the TS with Ethernet passenger ... need to ignore 4 bytes of CRC of passenger */
                __LOG(("CNC : L3 of TS with Ethernet passenger is without CRC of passenger(beside CRC of packet) \n"));
                l2HeaderSize += 4;
            }
        }

        l3ByteCount  = byteCount - l2HeaderSize;

        __LOG(("CNC : egress packet use L2 byte count [%d] (using descrPtr->egressByteCount) \n",
            byteCount));
        if(unitIsHa == GT_TRUE &&
           devObjPtr->errata.cncHaIgnoreCrc4BytesInL2ByteCountMode &&
           descrPtr->haToEpclInfo.paddingZeroForLess64BytesLength == 0)/* if missed padding the CRC is ignored anyway */
        {
            /* BC is 4 bytes less than in egress packet. I assume that CRC is not added to the BC */
            byteCount -= 4;
            __LOG(("CNC : NOTE : ERROR : HA unit ignores 4 bytes of CRC \n"));
        }
    }

    /* For L3 byte count mode the 4 L2 CRC bytes are also subtract */
    l3ByteCount -= 4;

    __LOG_PARAM(byteCount);
    __LOG_PARAM(l3ByteCount);

    /* Calculate packet size according to client byte count mode */
    switch (clientInfoPtr->byteCntMode)
    {
        case SMEM_CHT3_CNC_BYTE_CNT_L2_MODE_E:
            cncFinalSizeToCount = byteCount;
            __LOG(("CNC : byte count L2 size=[%d] \n",
                cncFinalSizeToCount));
            break;
        case SMEM_CHT3_CNC_BYTE_CNT_L3_MODE_E:
            cncFinalSizeToCount = l3ByteCount;
            __LOG(("CNC : byte count L3 size=[%d] \n",
                cncFinalSizeToCount));
            break;
        case SMEM_CHT3_CNC_BYTE_CNT_PACKETS_MODE_E:
            cncFinalSizeToCount = 1;
            __LOG(("Packet count mode \n "));
            break;
        default:
            skernelFatalError("cncByteCountGet: wrong byte count mode %d\n", clientInfoPtr->byteCntMode);
            return GT_FAIL; /* should never happened*/
    }

    __LOG(("CNC : Ended calculate the byte count [%d] \n",
        cncFinalSizeToCount));

    return cncFinalSizeToCount;

}

/**
* @internal snetCht3CncBlockSet function
* @endinternal
*
* @brief   Centralize counters block set
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] descrPtr                 - pointer to the frame's descriptor.
* @param[in] clientInfoPtr            - pointer to CNC client info.
*
* @retval GT_OK                    - counter was successfully set
* @retval GT_NOT_SUPPORTED         - CNC feature not supported
* @retval GT_EMPTY                 - all ranges for client are zero
*/
static GT_STATUS snetCht3CncBlockSet
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN SNET_CHT3_CNC_CLIENT_INFO_STC * clientInfoPtr
)
{
    DECLARE_FUNC_NAME(snetCht3CncBlockSet);

    GT_U32 regAddr;                 /* register address */
    GT_U32 * regPtr;                /* register data pointer */
    GT_U32 clientIndex;             /* CNC client index inside block */
    GT_U32 block;                   /* CNC block index */
    GT_U32 numOfPackets;            /* number of packets to count */
    SNET_CHT3_CNC_COUNTER_STC cncCounter;
                                    /* CNC counter value */
    CNC_COUNTER_FORMAT_ENT cncFormat;
                                    /* CNC counter format */
    GT_U32 size;                    /* frame header size */
                                    /* CNC range bitmap binded to the block */
    GT_STATUS status;
    GT_U32 wrapArroundEn;           /* enable/disable counter wraparound */
    GT_U32 i;                       /* loop index */
    GT_U32 rangeIndex;              /* counter index range */
    GT_U32 cncUnitIndex = clientInfoPtr->cncUnitIndex;/*the CNC unit index (0/1) (Sip5 devices)*/
    SNET_CNC_CLIENT_E client = clientInfoPtr->client;
    GT_CHAR*    clientNamePtr = clientInfoPtr->clientsNamePtr;

    __LOG(("Cnc client name : [%s] \n",
        clientNamePtr));

    /* Get client range bitmaps for all CNC blocks */
    status = snetCht3CncClientRangeBitmapGet(devObjPtr, clientInfoPtr);
    if (status != GT_OK)
    {
        __LOG(("the client not enabled for counting \n"));
        return status;
    }

    if(clientInfoPtr->isErrata_HA2CNC_swapIndexAndBc)
    {
        GT_U32      part1,part2;
        GT_U32      busValue;

        /* NOTE: the busValue was expected to be:
            part1 = clientInfoPtr->index & 0xFFFF
            part2 = 'byte count'

            busValue =  (part2 << 16) | (part1 & 0xFFFF);

            but due to the Erratum (the swap of values):
            busValue =  (part1 << 14) | (part2 & 0x3FFF);
        */

        part1                = clientInfoPtr->index & 0xFFFF;/*16bits*/
        part2                = cncByteCountGet(devObjPtr,descrPtr,clientInfoPtr) & 0x3FFF;

        busValue = (part1 << 14) | part2;

        /* now the 'CNC' unit take : 'index' as 16LSBits (expected to be part1 ... but is mix from part 1,2 ... after SWAP!)*/
        clientInfoPtr->index = busValue & 0xFFFF;
        /* now the 'CNC' unit take : 'byte count' as next 14Bits (expected to be part2 ... value of (part 1)>>2)*/
        size     = (busValue >> 16) & 0x3FFF;

        numOfPackets         = size ? 1 : 0;/* no count packets if no bytes to count */

        __LOG(("ERRATA : swapping the BC(%d) with the index(%d) \n",
            size , clientInfoPtr->index));

        /* Set counter increment to one packet */
        cncCounter.packetCount.l[0] = numOfPackets;
        cncCounter.packetCount.l[1] = 0;
    }
    else
    {
        if(0 == SMEM_CHT_IS_SIP5_GET(devObjPtr))
        {
            clientInfoPtr->index = clientInfoPtr->index & 0x3FFF;/*14bits*/
        }
        else
        {
            /* SIP5 supports 16 bits of counters' index */
            clientInfoPtr->index = clientInfoPtr->index & 0xFFFF;/*16bits*/
        }

        __LOG(("selected index[%d] \n",
            clientInfoPtr->index));

        /* Set counter increment to one packet */
        cncCounter.packetCount.l[0] = 1;
        cncCounter.packetCount.l[1] = 0;

        size = cncByteCountGet(devObjPtr,descrPtr,clientInfoPtr);

        numOfPackets = 1;
    }

    __LOG_PARAM(clientInfoPtr->index);
    __LOG_PARAM(size);


    /* Enable Wraparound in the Packet and Byte Count Counters */
    smemRegFldGet(devObjPtr, SMEM_CHT3_CNC_GLB_CONF_REG(devObjPtr,cncUnitIndex), 0, 1, &wrapArroundEn);
    if(wrapArroundEn)
    {
        __LOG(("Enable Wraparound in the Packet and Byte Count Counters \n"));
    }

    /* Get range index for specific client */
    rangeIndex = clientInfoPtr->index / devObjPtr->cncBlockMaxRangeIndex;
    __LOG(("range index is [%d] \n",
        rangeIndex));

    __LOG(("Set client byte count mode [%s] \n",
            ((clientInfoPtr->byteCntMode == SMEM_CHT3_CNC_BYTE_CNT_L2_MODE_E) ?
                    "SMEM_CHT3_CNC_BYTE_CNT_L2_MODE_E" :
                    "SMEM_CHT3_CNC_BYTE_CNT_L3_MODE_E")
        ));

    /* for egress vlan client packet number is multiplied */
    if( client == SNET_CNC_CLIENT_EGRESS_VLAN_PASS_DROP_E )
    {
        if( clientInfoPtr->userDefined != 0 )
        {
            numOfPackets = clientInfoPtr->userDefined;
            __LOG(("for egress vlan client packet number is multiplied [%d] \n",
                numOfPackets));
        }
    }

    for (block = 0; block < (GT_U32)CNC_CNT_BLOCKS(devObjPtr); block++)
    {
        /* Range bounded to specific CNC block */
        if (0 == snetFieldValueGet(clientInfoPtr->rangeBitmap[block].l, rangeIndex, 1))
        {
            continue;
        }

        if(SMEM_CHT_IS_SIP6_30_GET(devObjPtr))
        {
            __LOG(("Client instance[%d] in block[%d] with offset[0x%3.3x] \n",
                clientInfoPtr->fewClientsPerBlock.clientInstance,
                block,
                clientInfoPtr->fewClientsPerBlock.clientOffsetInBlock[block]));
        }

        /* Get client index inside CNC block */
        clientIndex = (clientInfoPtr->index + clientInfoPtr->fewClientsPerBlock.clientOffsetInBlock[block]) %
                    devObjPtr->cncBlockMaxRangeIndex;
        __LOG(("CNC Block[%d] , index [%d] in the block \n",
            block , clientIndex));

        /* Get memory address inside CNC block */
        regAddr = SMEM_CHT3_CNC_BLOCK_COUNTER_TBL_MEM(devObjPtr,block, clientIndex,cncUnitIndex);

        /* Read block data pointer */
        regPtr = smemMemGet(devObjPtr, regAddr);

        CNV_U32_TO_U64(size, cncCounter.byteCount);

        /* Get format of CNC counter */
        snetCht3CncCounterFormatGet(devObjPtr, block, &cncFormat,cncUnitIndex);

        for( i = 0 ; i < numOfPackets ; i++ )
        {
            /* Increment counters field */
            __LOG(("Increment counters field"));
            if (snetCht3CncIncrement(devObjPtr,regPtr, block, cncFormat, &cncCounter, wrapArroundEn,clientInfoPtr) == GT_FULL)
            {
                snetCht3CncBlockWrapStatusSet(devObjPtr, block, clientIndex,cncUnitIndex);
                /* if wraparound is disabled no need to count further packets */
                /* since no update to the counter will be done.               */
                if( wrapArroundEn == 0 )
                {
                    break;
                }
            }
        }

        /* Write block data pointer */
        smemMemSet(devObjPtr, regAddr, regPtr, CNC_BLOCK_ENTRY_WORDS_CNS);
    }

    return GT_OK;
}

/**
* @internal snetCht3CncTrigger function
* @endinternal
*
* @brief   Trigger CNC event for specified client
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] descrPtr                 - pointer to the frame's descriptor.
* @param[in,out] clientInfoPtr            - pointer to CNC client info.
* @param[in,out] clientInfoPtr            - pointer to CNC client info.
*
* @retval GT_OK                    - CNC event triggered
* @retval GT_NOT_SUPPORTED         - CNC feature not supported
* @retval GT_FAIL                  - CNC event is not triggered
* @retval GT_BAD_PARAM             - wrong CNC client index
* @retval GT_NO_SUCH               -  the CNC client is valid but not bound to current CNC block
*/
static GT_STATUS snetCht3CncTrigger
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    INOUT SNET_CHT3_CNC_CLIENT_INFO_STC * clientInfoPtr
)
{
    DECLARE_FUNC_NAME(snetCht3CncTrigger);

    SNET_CHT3_CNC_CLIENT_TRIG_FUN   cncClientTriggerFunc;

    /* save the bit index */
    clientInfoPtr->clientHwBit = convertClientToBitIndex(devObjPtr,clientInfoPtr->client,clientInfoPtr->cncUnitIndex);

    if(clientInfoPtr->clientHwBit == GT_NA)
    {
        /* invalid index ... meaning that the client not relevant to this block */
        /* the caller should skip this CNC block */
        return GT_NO_SUCH;
    }
    else
    if(clientInfoPtr->clientHwBit >= 32)
    {
        skernelFatalError("snetCht3CncTrigger: Error calculating bit index \n");
    }
    else
    if (SMEM_U32_GET_FIELD(devObjPtr->cncClientSupportBitmap, clientInfoPtr->clientHwBit, 1) == 0)
    {
        skernelFatalError("snetCht3CncTrigger: CNC client is not supported\n");
    }

    __LOG(("Call CNC trigger function for client [%s] \n", clientInfoPtr->clientsNamePtr));

    if(SMEM_CHT_IS_SIP6_30_GET(devObjPtr))
    {
        cncClientTriggerFunc = gCncClientTrigFuncArrSip6_30[clientInfoPtr->client].cncClientTriggerFunc;
    }
    else
    if(SMEM_CHT_IS_SIP6_15_GET(devObjPtr))
    {
        cncClientTriggerFunc = gCncClientTrigFuncArrSip6_15[clientInfoPtr->client].cncClientTriggerFunc;
    }
    else
    if(SMEM_CHT_IS_SIP6_10_GET(devObjPtr))
    {
        cncClientTriggerFunc = gCncClientTrigFuncArrSip6_10[clientInfoPtr->client].cncClientTriggerFunc;
    }
    else
    if(SMEM_CHT_IS_SIP6_GET(devObjPtr))
    {
        cncClientTriggerFunc = gCncClientTrigFuncArrSip6[clientInfoPtr->clientHwBit];
    }
    else
    {
        cncClientTriggerFunc = gCncClientTrigFuncArr[clientInfoPtr->client];
    }

    if(cncClientTriggerFunc == NULL)
    {
        skernelFatalError("snetCht3CncTrigger: CNC client is not BOUND to function \n");
        return GT_NOT_SUPPORTED;
    }

    return cncClientTriggerFunc(devObjPtr, descrPtr, clientInfoPtr);

}

/**
* @internal getFirstBitInValue function
* @endinternal
*
* @brief   get first bit in value
*
* @param[in] value                    - the  to get the first bit
*                                       the 'bit index' of the firts bit that is 'set' in 'value'
*
* @note based on prvCpssPpConfigBitmapFirstActiveBitGet
*
*/
static GT_U32   getFirstBitInValue(IN GT_U32    value)
{
    GT_U32 bmp;       /* bitmap             */
    GT_U32 bmp4;      /* bitmap 4 LSBs      */
    GT_U32 ii;         /* loop index         */
    GT_U32  firstBit;

    bmp = value;

    /* search the first non-zero bit in bitmap */
    /* loop on 8 4-bit in 32-bit bitmap        */
    for (ii = 0; (ii < 8); ii++, bmp >>= 4)
    {
        bmp4 = bmp & 0x0F;

        if (bmp4 == 0)
        {
            /* non-zero bit not found */
            /* search in next 4 bits  */
            continue;
        }

        /* non-zero bit found                                            */
        /* the expression below is the fast version of                   */
        /* (i * 4) + ((PRV_FIRST_NON_ZERO_BIT_CNS >> (bmp4 * 2)) & 0x03) */
        firstBit =
            (ii << 2) + ((PRV_FIRST_NON_ZERO_BIT_CNS >> (bmp4 << 1)) & 0x03);
        return firstBit;
    }

    /* occurs only if bmp == 0 */
    return SMAIN_NOT_VALID_CNS;

}

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
)
{
    DECLARE_FUNC_NAME(snetCht3CncFastDumpFuncPtr);

    GT_U32 regAddr;                 /* Register address */
    GT_U32 entry, * entryPtr;       /* CNC entry index and pointer */
    GT_U32 block;                   /* Trigger block index */
    GT_U32 intrNo;                  /* Interrupt cause bit */
    GT_BOOL doInterrupt;            /* No interrupt while message not sent */
    GT_U32 numOfCncInFu;            /* number of CNC counters in single FU message */
    GT_U32 cncUnitIndex;/* the CNC unit index (0/1) (Sip5 devices)*/
    GT_U32  pipeId,localPipeId;/* the PipeId */
    GT_U32  globalMgUnit;/* the serving MG unit for the upload */
    GT_U32 clearByRead;             /* enable setting of a configurable value
                                       to the Counters after read by the CPU */
    GT_U32  *clearByReadValuePtr;/* pointer to 2 words memory of clear by read */
    GT_U32  ii;

    block = getFirstBitInValue(cncTrigPtr[0]);

    if(block == SMAIN_NOT_VALID_CNS ||
       (0 == SMEM_U32_GET_FIELD(devObjPtr->cncClientSupportBitmap, block, 1)))
    {
        /* not triggered any active client */
        return;
    }

    cncUnitIndex = cncTrigPtr[1];

    smemRegFldGet(devObjPtr, SMEM_CHT3_CNC_GLB_CONF_REG(devObjPtr,cncUnitIndex), 1, 1, &clearByRead);
    clearByReadValuePtr = smemMemGet(devObjPtr, SMEM_CHT3_CNC_ROC_WORD0_REG(devObjPtr,cncUnitIndex));

    /* Get entryPtr according to entry index */
    regAddr = SMEM_CHT3_CNC_BLOCK_COUNTER_TBL_MEM(devObjPtr,block, 0,cncUnitIndex);
    /* Get start block data pointer */
    entryPtr = smemMemGet(devObjPtr, regAddr);

    /* Enable first time interrupt */
    doInterrupt = GT_TRUE;

    /* send each time 2 counters regardless to the alignment (as this is the minimal alignment) */
    numOfCncInFu = 2;

    if(devObjPtr->numOfPipesPerTile)
    {
        pipeId = smemGetCurrentPipeId(devObjPtr);
        localPipeId = smemConvertGlobalPipeIdToTileAndLocalPipeIdInTile(devObjPtr,pipeId,NULL);
        /* need to set the MG unit that expected to treat to FUQ messages */
        smemSetCurrentMgUnitIndex_withRelativeMgToTile(devObjPtr,
            localPipeId);
    }
    else
    if(devObjPtr->numOfMgUnits >= 2 && devObjPtr->cncUnitInfo[cncUnitIndex].mgUnit)
    {
        smemSetCurrentMgUnitIndex(devObjPtr,devObjPtr->cncUnitInfo[cncUnitIndex].mgUnit);
    }

    globalMgUnit = smemGetCurrentMgUnitIndex(devObjPtr);


    entry = devObjPtr->cncUploadInfo[globalMgUnit].indexInCnc;
    entryPtr += (CNC_BLOCK_ENTRY_WORDS_CNS * numOfCncInFu) * entry;


    if(entry == 0)
    {
        /* DMA message size is 4 words. Iterate memory space with step of 4 words */
        __LOG(("MG[%d] Send CNC messages to FUQ - start \n",
            globalMgUnit));
    }
    else
    {
        /* DMA message size is 4 words. Iterate memory space with step of 4 words */
        __LOG(("MG[%d] Send CNC messages to FUQ - continue from entry [%d] \n",
            entry,globalMgUnit));
    }


    for (/**/; entry < devObjPtr->cncBlockMaxRangeIndex / numOfCncInFu; entry++, entryPtr+= CNC_BLOCK_ENTRY_WORDS_CNS * numOfCncInFu)
    {
        /* Send FU message to CPU */
        while(GT_FALSE == snetCht3CncSendMsg2Cpu(devObjPtr, block, entryPtr,
                                                 &doInterrupt,cncUnitIndex))
        {
            /* Wait for SW to free buffers */
            if(oldWaitDuringSkernelTaskForAuqOrFua)
            {
                SIM_OS_MAC(simOsSleep)(SNET_CHT3_NO_CNC_DUMP_BUFFERS_SLEEP_TIME);
            }
            else
            {
                /* the action is broken and will be continued in new message */
                /* save the index */
                devObjPtr->needResendMessage = 1;
                devObjPtr->cncUploadInfo[globalMgUnit].indexInCnc = entry;

                __LOG(("MG[%d] Send CNC messages to FUQ - was broken at entry [%d] (will be continued...) \n",
                    entry,globalMgUnit));
                return;
            }
        }

        if(clearByRead)
        {
            for(ii = 0; ii < numOfCncInFu; ii++)
            {
                /* Set configurable value to the Counters after read by the CPU */
                entryPtr[(ii*2) + 0] = clearByReadValuePtr[0];
                entryPtr[(ii*2) + 1] = clearByReadValuePtr[1];
            }
        }

        /* Enable interrupt when FU message has sent successfully */
        doInterrupt = GT_TRUE;
    }

    devObjPtr->cncUploadInfo[globalMgUnit].indexInCnc = 0;

    __LOG(("MG[%d] Send CNC messages to FUQ - ended \n",
        globalMgUnit));

    /* Clear upload message trigger bit when the action is completed  */
    regAddr = SMEM_CHT3_CNC_FAST_DUMP_TRIG_REG(devObjPtr,cncUnitIndex);
    smemRegFldSet(devObjPtr, regAddr, block, 1, 0);


    intrNo = SMEM_CHT3_CNC_DUMP_BLOCK_FINISHED;
    snetChetahDoInterrupt(devObjPtr,
                        SMEM_CHT3_CNC_INTR_CAUSE_REG(devObjPtr,cncUnitIndex),
                        SMEM_CHT3_CNC_INTR_MASK_REG(devObjPtr,cncUnitIndex),
                        intrNo,
                        SMEM_CHT_CNC_ENGINE_INT(devObjPtr,cncUnitIndex));
}

/**
* @internal snetCht3CncSendMsg2Cpu function
* @endinternal
*
* @brief   Send FDB Upload message to CPU
*/
static GT_BOOL snetCht3CncSendMsg2Cpu
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32 block,
    IN GT_U32 * cncDumpPtr,
    INOUT GT_BOOL * doInterruptPtr,
    IN GT_U32   cncUnitIndex
)
{
    GT_U32 regAddr;                 /* Register address */
    GT_U32 fldValue;                /* Register's field value */
    GT_BOOL status;                 /* Function return value */
    GT_U32 intrNo;                  /* Interrupt cause bit */

    status = GT_TRUE;

    regAddr = SMEM_CHT_FU_QUE_BASE_ADDR_REG(devObjPtr);
    smemRegFldGet(devObjPtr, regAddr, 31, 1, &fldValue);

    /* Enable forwarding address update message to a separate queue */
    if (fldValue)
    {
        /* The device initiates a PCI Master transaction for each FU message it
           forwards to the CPU */
        status = snetChtL2iPciFuMsg(devObjPtr, cncDumpPtr,(2 * CNC_BLOCK_ENTRY_WORDS_CNS)/*always 2 CNC counters*/);

        /* if the device started 'soft reset' we not care if NA not reached the CPU
           and we must not 'wait until success' so declare success !!! */
        if(devObjPtr->needToDoSoftReset && status == GT_FALSE)
        {
            status = GT_TRUE;
        }

        if (status == GT_FALSE && (*doInterruptPtr) ==  GT_TRUE)
        {
            intrNo = SMEM_CHT3_CNC_BLOCK_RATE_LIMIT_FIFO(block);
            snetChetahDoInterrupt(devObjPtr,
                                SMEM_CHT3_CNC_INTR_CAUSE_REG(devObjPtr,cncUnitIndex),
                                SMEM_CHT3_CNC_INTR_MASK_REG(devObjPtr,cncUnitIndex),
                                intrNo,
                                SMEM_CHT_CNC_ENGINE_INT(devObjPtr,cncUnitIndex));

            *doInterruptPtr = GT_FALSE;
        }
    }

    return status;
}

/**
* @internal snetCht3CncClientRangeBitmapGet function
* @endinternal
*
* @brief   CNC range bitmap for client from CNC block configuration register
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] clientInfoPtr            - pointer to CNC client info.
*
* @retval GT_OK                    - client range was successfully get
* @retval GT_EMPTY                 - all ranges for client are zero
*/
static GT_STATUS snetCht3CncClientRangeBitmapGet
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SNET_CHT3_CNC_CLIENT_INFO_STC * clientInfoPtr
)
{
    DECLARE_FUNC_NAME(snetCht3CncClientRangeBitmapGet);

    GT_U32 block, *regPtr;
    GT_BOOL rangeEmpty = GT_TRUE;
    GT_U32 wordBit;
    GT_U32 cncUnitIndex = clientInfoPtr->cncUnitIndex;/*the CNC unit index (0/1) (Sip5 devices)*/
    GT_U32  numBlocks = CNC_CNT_BLOCKS(devObjPtr);
    GT_U32  clientHwBit = clientInfoPtr->clientHwBit;
    GT_U32  clientInstance = clientInfoPtr->fewClientsPerBlock.clientInstance;

    __LOG(("check which block enabled for counting (out of [%d] blocks) \n" ,
        numBlocks));

    for (block = 0; block < numBlocks; block++)
    {
        regPtr = smemMemGet(devObjPtr,
                   SMEM_CHT_ANY_CNC_BLOCK_CNF_REG(devObjPtr, block, clientHwBit,cncUnitIndex,clientInstance));

        if(SMEM_CHT_IS_SIP5_GET(devObjPtr))
        {
            if(SMEM_CHT_IS_SIP6_30_GET(devObjPtr))
            {
                /*Block <<%n>> Client Enable <<%m>>*/
                if(0 == SMEM_U32_GET_FIELD(regPtr[0], 0, 1))
                {
                    continue;
                }
                /*Block <<%n>> Client Sel <<%m>>*/
                if(clientHwBit != SMEM_U32_GET_FIELD(regPtr[0], 4, 5))
                {
                    continue;
                }

                clientInfoPtr->fewClientsPerBlock.clientOffsetInBlock[block] =
                    SMEM_U32_GET_FIELD(regPtr[0], 16,10);

            }
            else
            {
                if(0 == SMEM_U32_GET_FIELD(regPtr[0], clientHwBit, 1))
                {
                    continue;
                }
            }

            /* 64 ranges of 1K counters.
               The ranges mapped to the block for all clients  */
            smemRegGet(devObjPtr,
                SMEM_LION3_CNC_BLOCK_COUNT_EN_0_REG(devObjPtr,block,cncUnitIndex,clientInstance),
                &clientInfoPtr->rangeBitmap[block].l[0]);
            smemRegGet(devObjPtr,
                SMEM_LION3_CNC_BLOCK_COUNT_EN_1_REG(devObjPtr,block,cncUnitIndex,clientInstance),
                &clientInfoPtr->rangeBitmap[block].l[1]);
        }
        else
        /* Lion B0 and above */
        if(SKERNEL_IS_LION_REVISON_B0_DEV(devObjPtr))
        {
            /* Lion and above - 64 ranges of 512 counters.
               The ranges mapped to the block for all clients  */
            if(0 == SMEM_U32_GET_FIELD(regPtr[0], clientHwBit, 1))
            {
                continue;
            }
            clientInfoPtr->rangeBitmap[block].l[0] = regPtr[1];
            clientInfoPtr->rangeBitmap[block].l[1] = regPtr[2];
        }
        else
        {
            wordBit = (clientHwBit % 2) * 13;
            /* DxCh3, DxChXCat - 8 ranges of 2048 counters */
            if(0 == SMEM_U32_GET_FIELD(regPtr[0], wordBit, 1))
            {
                continue;
            }
            clientInfoPtr->rangeBitmap[block].l[0] =
                SMEM_U32_GET_FIELD(regPtr[0], wordBit + 1, 12);
        }

        if(clientInfoPtr->rangeBitmap[block].l[0] ||
           clientInfoPtr->rangeBitmap[block].l[1])
        {
            __LOG(("Block [%d] is enabled with ranges bmp [0x%8.8x] [0x%8.8x]\n",
                block,
                clientInfoPtr->rangeBitmap[block].l[0],
                clientInfoPtr->rangeBitmap[block].l[1]));

            rangeEmpty = GT_FALSE;
        }
        else
        {
            __LOG(("Block [%d] is enabled but no range valid (so ignored) \n",
                block));
        }
    }

    return (rangeEmpty == GT_TRUE) ? GT_EMPTY : GT_OK;
}

/**
* @internal snetCht3CncClientL2L3VlanIngrTrigger function
* @endinternal
*
* @brief   CNC L2 and L3 VLAN ingress client
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] descrPtr                 - pointer to the frame's descriptor.
* @param[in] clientInfoPtr            - pointer to CNC client info.
*
* @retval GT_OK                    - client is triggered
* @retval GT_FAIL                  - fail in client triggering
*/
static GT_STATUS snetCht3CncClientL2L3VlanIngrTrigger
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN SNET_CHT3_CNC_CLIENT_INFO_STC * clientInfoPtr
)
{
    DECLARE_FUNC_NAME(snetCht3CncClientL2L3VlanIngrTrigger);

    GT_U32 fldValue;
    smemRegFldGet(devObjPtr,
                  SMEM_CHT3_CNC_VLAN_EN_REG(devObjPtr, descrPtr->localDevSrcPort),
                  descrPtr->localDevSrcPort % 32, 1, &fldValue);
    if (fldValue == 0)
    {
        __LOG(("[%s] Counting Disabled 'per port' : ingress physical port disabled for vlan/eport ingress counting \n",
            clientInfoPtr->clientsNamePtr));

        /* ingress physical port not enabled for vlan/eport ingress counting */
        return GT_FAIL;
    }

    clientInfoPtr->index = descrPtr->eVid;
    smemRegFldGet(devObjPtr, SMEM_CHT3_CNC_COUNT_MODE_REG(devObjPtr), 3, 1,
                  &fldValue);

    /* Set byte count mode for L2 and L3 VLAN ingress client */
    clientInfoPtr->byteCntMode = fldValue;

    clientInfoPtr->isErrata_NotCountCrc = devObjPtr->errata.PCL2CNC_ByteCountWithOut4BytesCrc;

    return GT_OK;
}

/**
* @internal snetCht3CncClientPclLookUpIngrTrigger function
* @endinternal
*
* @brief   CNC PCL lookup ingress client
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] descrPtr                 - pointer to the frame's descriptor.
* @param[in] clientInfoPtr            - pointer to CNC client info.
*
* @retval GT_OK                    - client is triggered
* @retval GT_FAIL                  - fail in client triggering
*/
static GT_STATUS snetCht3CncClientPclLookUpIngrTrigger
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN SNET_CHT3_CNC_CLIENT_INFO_STC * clientInfoPtr
)
{
    GT_U32 cntModeBit;
    GT_U32 fldValue;

    clientInfoPtr->index = clientInfoPtr->userDefined;

    /* Count mode bit:  0, 1 or 2 */

    switch(clientInfoPtr->client)
    {
        case SNET_CNC_CLIENT_IPCL_LOOKUP_0_ACTION_0_E:
        case SNET_CNC_CLIENT_IPCL_LOOKUP_0_ACTION_1_E:
        case SNET_CNC_CLIENT_IPCL_LOOKUP_0_ACTION_2_E:
        case SNET_CNC_CLIENT_IPCL_LOOKUP_0_ACTION_3_E:
            /* action 0 */
            cntModeBit = 0;
            break;
        case SNET_CNC_CLIENT_IPCL_LOOKUP_1_ACTION_0_E:
        case SNET_CNC_CLIENT_IPCL_LOOKUP_1_ACTION_1_E:
        case SNET_CNC_CLIENT_IPCL_LOOKUP_1_ACTION_2_E:
        case SNET_CNC_CLIENT_IPCL_LOOKUP_1_ACTION_3_E:
            /* action 1 */
            cntModeBit = 1;
            break;
        default:
            /* action 2 */
            cntModeBit = 2;
            break;
    }

    smemRegFldGet(devObjPtr, SMEM_CHT3_CNC_COUNT_MODE_REG(devObjPtr),
                  cntModeBit, 1, &fldValue);

    /* Set byte count mode for PCL lookup ingress client */
    clientInfoPtr->byteCntMode = fldValue;

    clientInfoPtr->isErrata_NotCountCrc = devObjPtr->errata.PCL2CNC_ByteCountWithOut4BytesCrc;


    return GT_OK;
}

/**
* @internal snetCht3CncClientEgfPassDropTrigger function
* @endinternal
*
* @brief   CNC Egress eVLAN Egress-Filter pass/drop client
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] descrPtr                 - pointer to the frame's descriptor.
* @param[in] clientInfoPtr            - pointer to CNC client info.
*
* @retval GT_OK                    - client is triggered
* @retval GT_FAIL                  - fail in client triggering
*/
static GT_STATUS snetCht3CncClientEgfPassDropTrigger
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN SNET_CHT3_CNC_CLIENT_INFO_STC * clientInfoPtr
)
{
    DECLARE_FUNC_NAME(snetCht3CncClientEgfPassDropTrigger);

    GT_U32 dropBit = 0, *regPtr;
    GT_U32 cncVlanSelection;/* CNC VLAN Selection */
    GT_U32 vlanId;  /* the vlanId for the CNC indexing */
    GT_U32 cncVlanEnable;

    regPtr = smemMemGet(devObjPtr, SMEM_SIP6_EGF_EVLAN_PASS_DROP_REG(devObjPtr));

    cncVlanEnable = SMEM_U32_GET_FIELD(*regPtr, 3, 1);
    __LOG_PARAM(cncVlanEnable);

    if(!cncVlanEnable)
        return GT_FAIL;

    /*CNC VLAN Selection*/
    cncVlanSelection = SMEM_U32_GET_FIELD(*regPtr, 0, 2);

    __LOG_PARAM(cncVlanSelection);

    if(cncVlanSelection == 2) /*not same value as in EQ !!!*/
    {
        vlanId = descrPtr->vid0Or1AfterTti;/* OrigVID; The original incoming VID is used as the CNC index */
        vlanId &= 0xFFF;/*12 bits*/

        __LOG(("(egress)VLAN Selection mode : OrigVID(12bits); Use the original incoming VID [0x%x] \n",
            vlanId));
    }
    else if(cncVlanSelection == 0)/*not same value as in EQ !!!*/
    {
        vlanId = descrPtr->eVid;
        vlanId &= 0x3FFF;/*14 bits*/

        __LOG(("(egress)VLAN Selection mode : eVLAN((14bits)); Use the eVLAN [0x%x] \n",
            vlanId));
    }
    else if(cncVlanSelection == 1)/*not same value as in EQ !!!*/
    {
        vlanId = descrPtr->vid1;
        vlanId &= 0xFFF;/*12 bits*/

        __LOG(("(egress)VLAN Selection mode : Tag1(12bits); Use the Tag1 VID [0x%x] \n",
            vlanId));
    }
    else
    {
        vlanId = 0;/* not supported option*/
        __LOG(("(egress)VLAN Selection mode : unknown; Use the VID = 0 \n"));
    }

    clientInfoPtr->index = dropBit  | (vlanId << 1);

    clientInfoPtr->byteCntMode = SMEM_U32_GET_FIELD(*regPtr, 2, 1);

    return GT_OK;
}

/**
* @internal snetCht3CncClientVlanPassDropIngrTrigger function
* @endinternal
*
* @brief   CNC VLAN pass/drop ingress client
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] descrPtr                 - pointer to the frame's descriptor.
* @param[in] clientInfoPtr            - pointer to CNC client info.
*
* @retval GT_OK                    - client is triggered
* @retval GT_FAIL                  - fail in client triggering
*/
static GT_STATUS snetCht3CncClientVlanPassDropIngrTrigger
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN SNET_CHT3_CNC_CLIENT_INFO_STC * clientInfoPtr
)
{
    DECLARE_FUNC_NAME(snetCht3CncClientVlanPassDropIngrTrigger);

    GT_U32 enable, dropBit, *regPtr;
    GT_U32 cncVlanSelection;/* CNC VLAN Selection */
    GT_U32 vlanId;  /* the vlanId for the CNC indexing */
    GT_U32 vidNumBits;/* number of bits from the vlanId to use */
    GT_U32 cncFromCpuEnOffset; /* offset of the cnc_from_cpu_en bit */
    GT_U32 byteCountModeOffset; /* offset of EQ Clients Byte Count mode */

    if (descrPtr->packetCmd == SKERNEL_EXT_PKT_CMD_TRAP_TO_CPU_E)
    {
        __LOG(("[%s]Counting Disabled : TO_CPU packets should NOT be counted \n",
            clientInfoPtr->clientsNamePtr));
        return GT_FAIL;
    }

    /* TO_CPU packets should NOT be counted (fix CQ Bugs00124345) */
    if (descrPtr->useVidx == 0 && descrPtr->trgEPort == SNET_CHT_CPU_PORT_CNS)
    {
        __LOG(("[%s]Counting Disabled : forward packets to port 63 (CPU) should NOT be counted \n",
            clientInfoPtr->clientsNamePtr));
        return GT_FAIL;
    }

    if(SMEM_CHT_IS_SIP5_GET(devObjPtr))
    {
        cncFromCpuEnOffset = 14;
        byteCountModeOffset = 13;
    }
    else
    {
        cncFromCpuEnOffset = 15;
        byteCountModeOffset = 14;
    }

    regPtr = smemMemGet(devObjPtr, SMEM_CHT_PRE_EGR_GLB_CONF_REG(devObjPtr));
    if (descrPtr->marvellTagged &&
        descrPtr->packetCmd == SKERNEL_EXT_PKT_CMD_FROM_CPU_E)
    {
        enable = SMEM_U32_GET_FIELD(*regPtr, cncFromCpuEnOffset, 1);
        if (enable == 0)
        {
            __LOG(("[%s]Counting globally Disabled : 'FROM_CPU' Marvell tagged \n",
                clientInfoPtr->clientsNamePtr));
            return GT_FAIL;
        }
    }

    dropBit = clientInfoPtr->userDefined;
    __LOG_PARAM(dropBit);

    if(SMEM_CHT_IS_SIP5_GET(devObjPtr))
    {
        /*CNC VLAN Selection*/
        cncVlanSelection = SMEM_U32_GET_FIELD(*regPtr, 17, 2);

        if(cncVlanSelection == 0)
        {
            vlanId = descrPtr->vid0Or1AfterTti;/* OrigVID; The original incoming VID is used as the CNC index */
            vlanId &= 0xFFF;/*12 bits*/

            __LOG(("VLAN Selection mode : OrigVID(12bits); Use the original incoming VID [0x%x] \n",
                vlanId));
        }
        else if(cncVlanSelection == 1)
        {
            vlanId = descrPtr->eVid;
            vlanId &= 0x3FFF;/*14 bits*/

            __LOG(("(ingress)VLAN Selection mode : eVLAN((14bits)); Use the eVLAN [0x%x] \n",
                vlanId));

        }
        else if(cncVlanSelection == 2)
        {
            vlanId = descrPtr->vid1;
            vlanId &= 0xFFF;/*12 bits*/

            __LOG(("(ingress)VLAN Selection mode : Tag1(12bits); Use the Tag1 VID [0x%x] \n",
                vlanId));
        }
        else
        {
            vlanId = 0;/* not supported option*/
            __LOG(("(ingress)VLAN Selection mode : unknown; Use the VID = 0 \n"));
        }

        vidNumBits = 14;
    }
    else
    {
        vlanId = descrPtr->ingressPipeVid;/* The index passed to the bound
                counter blocks is the packet VLAN assignment after the ingress
                Policy engine */
        vidNumBits = 12;

        __LOG(("The VLAN used is the VLAN assignment after the ingress Policy : the CNC index [%d] \n",
            vlanId));
    }

    if(SMEM_CHT_IS_SIP6_30_GET(devObjPtr))
    {
        __LOG(("NOTE: SIP6.30 : The drop bit is 0 and not 14 like in legacy sip5 devices \n"));
        clientInfoPtr->index = dropBit /*bit 0*/  | (SMEM_U32_GET_FIELD(vlanId,0,vidNumBits) << 1);
    }
    else
    {
        clientInfoPtr->index = (dropBit << vidNumBits) | SMEM_U32_GET_FIELD(vlanId,0,vidNumBits);
    }

    clientInfoPtr->byteCntMode = SMEM_U32_GET_FIELD(*regPtr, byteCountModeOffset, 1);

    return GT_OK;
}

/**
* @internal snetCht3CncClientVlanPassDropEgrTrigger function
* @endinternal
*
* @brief   CNC VLAN pass/drop egress client
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] descrPtr                 - pointer to the frame's descriptor.
* @param[in] clientInfoPtr            - pointer to CNC client info.
*
* @retval GT_OK                    - client is triggered
* @retval GT_FAIL                  - fail in client triggering
*/
static GT_STATUS snetCht3CncClientVlanPassDropEgrTrigger
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN SNET_CHT3_CNC_CLIENT_INFO_STC * clientInfoPtr
)
{
    DECLARE_FUNC_NAME(snetCht3CncClientVlanPassDropEgrTrigger);

    GT_U32 cncEgressVlanClientCountMode, outPorts, dropBit = 0, *regPtr;
    GT_U32 cncVlanSelection;/* CNC VLAN Selection */
    GT_U32 vlanId;  /* the vlanId for the CNC indexing */
    GT_U32 vidNumBits;/* number of bits from the vlanId to use */

    outPorts = clientInfoPtr->userDefined;
    if (SMEM_CHT_IS_SIP6_GET(devObjPtr))
    {
        regPtr = smemMemGet(devObjPtr, SMEM_SIP6_PREQ_GLB_CONF_REG(devObjPtr));
    }
    else
    {
        regPtr = smemMemGet(devObjPtr, SMEM_CHT3_CNC_MODES_REG(devObjPtr));
    }
    if (outPorts == 0)
    {
        __LOG(("No Replication will egress (all replications filtered) \n "));
        if (SMEM_CHT_IS_SIP6_GET(devObjPtr) == GT_FALSE)
        {
            /* <CNC Egress VLAN Client Count Mode>
                Selects the type of packets counted by the CNC Egress VLAN Client
               according to the packet's VLAN Egress filtering status.
               0 = CountEgressFilterAndTailDrop
               1 = CountEgressFilterOnly
               2 = CountTailDropOnly
            */

            cncEgressVlanClientCountMode = SMEM_U32_GET_FIELD(*regPtr, 4, 2);

            __LOG_PARAM(cncEgressVlanClientCountMode);

            if (cncEgressVlanClientCountMode == 2 || cncEgressVlanClientCountMode == 3)
            {
                __LOG(("[%s]Counting Disabled : Simulation not Supports 'Tail Drop' so nothing to count \n",
                    clientInfoPtr->clientsNamePtr));

            return GT_FAIL;
            }
        }

        dropBit = 1;
    }

    __LOG_PARAM(dropBit);

    if(devObjPtr->supportEArch && devObjPtr->unitEArchEnable.eq)
    {
        if (SMEM_CHT_IS_SIP6_GET(devObjPtr))
        {
            /*CNC VLAN Selection*/
            cncVlanSelection = SMEM_U32_GET_FIELD(*regPtr, 19, 2);
        }
        else if (SMEM_CHT_IS_SIP5_20_GET(devObjPtr) == 0)
        {
            /*CNC VLAN Selection*/
            cncVlanSelection = SMEM_U32_GET_FIELD(*regPtr, 10, 2);
        }
        else
        {
            /*CNC VLAN Selection*/
            cncVlanSelection = SMEM_U32_GET_FIELD(*regPtr, 11, 2);
        }

        if(cncVlanSelection == 2) /*not same value as in EQ !!!*/
        {
            vlanId = descrPtr->vid0Or1AfterTti;/* OrigVID; The original incoming VID is used as the CNC index */
            vlanId &= 0xFFF;/*12 bits*/

            __LOG(("(egress)VLAN Selection mode : OrigVID(12bits); Use the original incoming VID [0x%x] \n",
                vlanId));
        }
        else if(cncVlanSelection == 0)/*not same value as in EQ !!!*/
        {
            vlanId = descrPtr->eVid;
            vlanId &= 0x3FFF;/*14 bits*/

            __LOG(("(egress)VLAN Selection mode : eVLAN((14bits)); Use the eVLAN [0x%x] \n",
                vlanId));
        }
        else if(cncVlanSelection == 1)/*not same value as in EQ !!!*/
        {
            vlanId = descrPtr->vid1;
            vlanId &= 0xFFF;/*12 bits*/

            __LOG(("(egress)VLAN Selection mode : Tag1(12bits); Use the Tag1 VID [0x%x] \n",
                vlanId));
        }
        else
        {
            vlanId = 0;/* not supported option*/
            __LOG(("(egress)VLAN Selection mode : unknown; Use the VID = 0 \n"));
        }

        vidNumBits = 14;
    }
    else
    {
        vlanId = descrPtr->eVid;
        vidNumBits = 12;
    }

    if (SMEM_CHT_IS_SIP6_GET(devObjPtr))
    {
        clientInfoPtr->index = dropBit | (vlanId << 1);
        clientInfoPtr->byteCntMode = SMEM_U32_GET_FIELD(*regPtr, 18, 1);
    }
    else
    {
        clientInfoPtr->index = (dropBit << vidNumBits) | SMEM_U32_GET_FIELD(vlanId,0,vidNumBits);
        clientInfoPtr->byteCntMode = SMEM_U32_GET_FIELD(*regPtr, 2, 1);
    }

    return GT_OK;
}

/**
* @internal snetCht3CncClientTxqQueuePassDropEgrTrigger function
* @endinternal
*
* @brief   CNC EQ pass/drop egress client
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] descrPtr                 - pointer to the frame's descriptor.
* @param[in] clientInfoPtr            - pointer to CNC client info.
*
* @retval GT_OK                    - client is triggered
* @retval GT_FAIL                  - fail in client triggering
*/
static GT_STATUS snetCht3CncClientTxqQueuePassDropEgrTrigger
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN SNET_CHT3_CNC_CLIENT_INFO_STC * clientInfoPtr
)
{
    DECLARE_FUNC_NAME(snetCht3CncClientTxqQueuePassDropEgrTrigger);

    GT_U32 egrPort;
    GT_U32 fldValue;                        /* register's field value */
    GT_U32 globalPort;
    GT_U32 cncModeRegAddr;
    GT_U32 tailDrop = 0;/*no tail drop in simulation*/
    GT_U32 isCnMessage;
    SMEM_CHT3_CNC_EGRESS_QUEUE_CLIENT_MODE_ENT cncEqClientMode;
    GT_U32  dp,QueueID;

    /* Set client byte count mode for EQ pass/drop egress client */
    if (SMEM_CHT_IS_SIP6_GET(devObjPtr))
    {
        cncModeRegAddr = SMEM_SIP6_PREQ_GLB_CONF_REG(devObjPtr);
        smemRegFldGet(devObjPtr, cncModeRegAddr, 18, 1, &fldValue);
        clientInfoPtr->byteCntMode = fldValue;

        /* Selects the counting mode of the CNC Egress Queue Client TailDropMode/CNMode */
        smemRegFldGet(devObjPtr, cncModeRegAddr, 16, 2, &fldValue);

        QueueID = clientInfoPtr->userDefined;
        dp = descrPtr->dp;/*0..2*/
        __LOG_PARAM(QueueID);
        __LOG_PARAM(tailDrop);

        switch (fldValue)
        {
            case 0: /*SMEM_CHT3_CNC_EGRESS_QUEUE_CLIENT_MODE_TAIL_DROP_E*/
                if(tailDrop)
                {
                    /* will not happen in simulation since there are no tail drop */
                    /* still ... this is the logic */
                    dp = 3;
                }
                __LOG_PARAM(dp);
                /*{Queue ID, Queue DP[1:0]}*/
                clientInfoPtr->index =  QueueID << 2 | dp;
                __LOG(("MODE_TAIL_DROP : CNC index : {Queue ID, Queue DP[1:0]} : [0x%x] \n",
                    clientInfoPtr->index));
                break;
            case 1:/*SMEM_CHT3_CNC_EGRESS_QUEUE_CLIENT_MODE_CN_E*/
                __LOG(("used CN Counting Mode (NOTE: forced to use packets count mode) \n"));
                __LOG(("Simulation currently not differ between CN and NON-CN messages .. so treat all as 'non CN' \n"));
                isCnMessage = 0;
                __LOG_PARAM(isCnMessage);
                /* the CN counts packets */
                /*{Queue ID, Pass/Drop, CN/Non-CN}*/
                clientInfoPtr->index =  QueueID << 2 | (tailDrop << 1) | isCnMessage;

                clientInfoPtr->byteCntMode = SMEM_CHT3_CNC_BYTE_CNT_PACKETS_MODE_E;
                __LOG(("MODE_CN : CNC index : {Queue ID, Pass/Drop, CN/Non-CN} : [0x%x] \n",
                    clientInfoPtr->index));
                break;
            case 2:/*SMEM_CHT3_CNC_EGRESS_QUEUE_CLIENT_MODE_TAIL_DROP_REDUCED_E*/
            default:
                /*{Queue ID, Pass/Drop}*/
                clientInfoPtr->index =  QueueID << 1 | tailDrop;
                __LOG(("MODE_TAIL_DROP_REDUCED : CNC index : {Queue ID, Pass/Drop} : [0x%x] \n",
                    clientInfoPtr->index));
                break;
        }

        return GT_OK;
    }

    egrPort = clientInfoPtr->userDefined;
    cncModeRegAddr = SMEM_CHT3_CNC_MODES_REG(devObjPtr);

    smemRegFldGet(devObjPtr, cncModeRegAddr, 0, 1, &fldValue);
    clientInfoPtr->byteCntMode = fldValue;

    /* Lion B0 and above */
    if (devObjPtr->txqRevision)
    {
        if(0 == SMEM_CHT_IS_SIP5_GET(devObjPtr))
        {
            /* Lion B0 uses Global port */
            globalPort = SMEM_CHT_GLOBAL_PORT_FROM_LOCAL_PORT_MAC(devObjPtr, egrPort);
            __LOG(("convert local port [%d] to  Global port [%d]",
                egrPort,globalPort));

            egrPort = globalPort;

            egrPort &= 0x3F;/*6 bits*/
        }
        else
        {
            /*egrPort was already set*/
        }

        if (SMEM_CHT_IS_SIP5_20_GET(devObjPtr) == 0)
        {
            /* Selects the counting mode of the CNC Egress Queue Client TailDropMode/CNMode */
            smemRegFldGet(devObjPtr, cncModeRegAddr, 8, 1, &fldValue);

            switch (fldValue)
            {
                case 0:
                    cncEqClientMode = SMEM_CHT3_CNC_EGRESS_QUEUE_CLIENT_MODE_TAIL_DROP_E;
                    break;
                default:
                    cncEqClientMode = SMEM_CHT3_CNC_EGRESS_QUEUE_CLIENT_MODE_CN_E;
                    break;
            }
        }
        else
        {
            /* Selects the counting mode of the CNC Egress Queue Client TailDropMode/CNMode */
            smemRegFldGet(devObjPtr, cncModeRegAddr, 8, 2, &fldValue);

            switch (fldValue)
            {
                case 0:
                    cncEqClientMode = SMEM_CHT3_CNC_EGRESS_QUEUE_CLIENT_MODE_TAIL_DROP_REDUCED_E;
                    break;
                case 1:
                    cncEqClientMode = SMEM_CHT3_CNC_EGRESS_QUEUE_CLIENT_MODE_TAIL_DROP_E;
                    break;
                default:
                    cncEqClientMode = SMEM_CHT3_CNC_EGRESS_QUEUE_CLIENT_MODE_CN_E;
                    break;
            }
        }

        switch (cncEqClientMode)
        {
            case SMEM_CHT3_CNC_EGRESS_QUEUE_CLIENT_MODE_TAIL_DROP_E:
                __LOG(("used Tail-Drop Counting Mode \n"));
                if(0 == SMEM_CHT_IS_SIP5_GET(devObjPtr))
                {
                    /* index bit 10 is 0 at simulation since there are no tail drop */
                    clientInfoPtr->index = (descrPtr->dp >> 1) << 11 | (tailDrop << 10) | egrPort << 4 |
                        descrPtr->tc << 1 | (descrPtr->dp & 1);
                }
                else
                if(!SMEM_CHT_IS_SIP5_20_GET(devObjPtr))
                {
                    /* index bit 13 is 0 at simulation since there are no tail drop */
                    clientInfoPtr->index = (tailDrop << 13) | egrPort << 5 |
                        descrPtr->tc << 2 | (descrPtr->dp);
                }
                else
                {
                    dp = descrPtr->dp;/*0..2*/

                    if(tailDrop)
                    {
                        /* will not happen in simulation since there are no tail drop */
                        /* still ... this is the logic */
                        dp = 3;
                    }

                    /* fix according to : TXQ-1430 : New mode for queue pass/drop CNC counting */
                    /*
                        Fix the legacy queue pass/drop (TD) address to CNC to =
                        {TxQ-Port[9:0], TC[2:0], DP[1:0]}
                        , where DP[1:0] represent: 0-2: pass with DP=0-2, respectively; 3: drop with any DP.
                        This fix is specific to BC3 because the CNC unit per pipe supports 32K
                        and we don't want the TxQ to consume all counters.
                    */
                    clientInfoPtr->index =  egrPort << 5 | descrPtr->tc << 2 | dp;
                }
                break;
            case SMEM_CHT3_CNC_EGRESS_QUEUE_CLIENT_MODE_CN_E:
                __LOG(("used CN Counting Mode (NOTE: forced to use packets count mode) \n"));

                __LOG(("Simulation currently not differ between CN and NON-CN messages .. so treat all as 'non CN' \n"));
                isCnMessage = 0;
                __LOG_PARAM(isCnMessage);

                if(0 == SMEM_CHT_IS_SIP5_GET(devObjPtr))
                {
                    /* index bit 10 is 0 at simulation since there are no tail drop, bit 0 is 0 - None-CN Message Counter */
                    clientInfoPtr->index = (tailDrop << 10) | (egrPort << 4) | (descrPtr->tc << 1) | isCnMessage;
                }
                else
                if(!SMEM_CHT_IS_SIP5_20_GET(devObjPtr))
                {
                    /* index bit 12 is 0 at simulation since there are no tail drop, bit 0 is 0 - None-CN Message Counter */
                    clientInfoPtr->index = (tailDrop << 12) | (egrPort << 4) | (descrPtr->tc << 1) | isCnMessage ;
                }
                else
                {
                    /* fix according to : TXQ-1430 : New mode for queue pass/drop CNC counting */
                    /*
                        Fix the legacy CN address to
                        {TxQ-Port[9:0], TC[2:0], Pass/Drop [1bit], CN/non-CN[1bit]}
                    */
                    __LOG(("NOTE: index format in sip5_20 VERY different than previous devices"));
                    clientInfoPtr->index =  (egrPort << 5) | (descrPtr->tc << 2) | (tailDrop << 1) | isCnMessage;
                }

                /* the CN counts packets */
                clientInfoPtr->byteCntMode = SMEM_CHT3_CNC_BYTE_CNT_PACKETS_MODE_E;
                break;
            case SMEM_CHT3_CNC_EGRESS_QUEUE_CLIENT_MODE_TAIL_DROP_REDUCED_E:
                __LOG(("used Tail-Drop Reduced Counting Mode \n"));
                /* index bit 0 is 0 since there is no tail drop in simulation */
                clientInfoPtr->index =  egrPort << 4 | descrPtr->tc << 1 | tailDrop;
                break;
            default:
                __LOG(("Unsupported Counting Mode \n"));
                break;
        }
    }
    else
    {
        /* In this case CPU port is port 28 (XCat, AlleyCat3 and PONCat3 functional specifications, 16.4.5 )*/
        if(SNET_CHT_CPU_PORT_CNS == egrPort)
        {
            egrPort = 28;
        }

        /*
        Text from FS BTS xCAT (xCAT) #322
        All the CNC indexes are defined in the functional to be 14 bits, so only bits 13:11 are "0".
        ------------------------------------------------------------------
        In (section 21.5.5.2), figure 88, Egress counters index format dp occupies 1 bit.
        The format should be as below (DP is extended to 2 bits)
        bits 1:0 = dp
        bits 4:2 = tc
        bits 9:5 = port
        bit 10 = drop/pass
        bits 15:11 = "0" */
        /* index bit 10 is 0 at simulation since there are no tail drop */
        clientInfoPtr->index = (egrPort << 5) | (descrPtr->tc << 2) | (descrPtr->dp);
    }

    return GT_OK;
}

/**
* @internal snetCht3CncClientPclEgrTrigger function
* @endinternal
*
* @brief   CNC PCL egress client
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] descrPtr                 - pointer to the frame's descriptor.
* @param[in] clientInfoPtr            - pointer to CNC client info.
*
* @retval GT_OK                    - client is triggered
* @retval GT_FAIL                  - fail in client triggering
*/
static GT_STATUS snetCht3CncClientPclEgrTrigger
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN SNET_CHT3_CNC_CLIENT_INFO_STC * clientInfoPtr
)
{
    GT_U32 *regPtr;

    regPtr = smemMemGet(devObjPtr, SMEM_CHT3_EPCL_GLOBAL_REG(devObjPtr));

    clientInfoPtr->index = clientInfoPtr->userDefined;

    if(devObjPtr->supportEArch)
    {
        clientInfoPtr->byteCntMode = SMEM_U32_GET_FIELD(*regPtr, 10, 1);
    }
    else
    {
        clientInfoPtr->byteCntMode = SMEM_U32_GET_FIELD(*regPtr, 25, 1);
    }


    return GT_OK;
}

/**
* @internal snetCht3CncClientArpTblTrigger function
* @endinternal
*
* @brief   CNC ARP client
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] descrPtr                 - pointer to the frame's descriptor.
* @param[in] clientInfoPtr            - pointer to CNC client info.
*
* @retval GT_OK                    - client is triggered
* @retval GT_FAIL                  - fail in client triggering
*/
static GT_STATUS snetCht3CncClientArpTblTrigger
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN SNET_CHT3_CNC_CLIENT_INFO_STC * clientInfoPtr
)
{
    DECLARE_FUNC_NAME(snetCht3CncClientArpTblTrigger);

    GT_U32 fldValue;

    if(devObjPtr->errata.HA2CNC_swapIndexAndBc)/*SIPCTRL-74 Ha2CNC wrapper connections are turn around*/
    {
        clientInfoPtr->isErrata_HA2CNC_swapIndexAndBc = 1;
    }

    if(descrPtr->isNat)
    {
        clientInfoPtr->index = descrPtr->arpPtr;

        __LOG(("The NAT accessed to the ARP/TS table at index [%d] (from  descrPtr->arpPtr) \n" ,
            clientInfoPtr->index));

        /* The CNC clients ARP and NAT are muxed between them
            (since for a given packet the user can access for ARP entry or for NAT entry).
           This offset is added to the NAT when sending the pointer to the CNC. */
        smemRegFldGet(devObjPtr, SMEM_BOBCAT2_HA_NAT_CONFIGURATIONS_REG(devObjPtr), 0, 16,
                      &fldValue);
        __LOG(("This offset is added to the NAT when sending the pointer to the CNC [%d] \n", fldValue));
        clientInfoPtr->index += fldValue;
    }
    else
    {
        clientInfoPtr->index = descrPtr->arpPtr;
    }

    __LOG(("client index[%d] \n", clientInfoPtr->index));

    smemRegFldGet(devObjPtr, SMEM_CHT3_ROUTE_HA_GLB_CNF_REG(devObjPtr), 0, 1,
                  &fldValue);

    __LOG(("Set byte count mode for ARP client [%d] \n", fldValue));
    clientInfoPtr->byteCntMode = fldValue;

    return GT_OK;
}

/**
* @internal snetCht3CncClientHaTargetEPortTrigger function
* @endinternal
*
* @brief   CNC HA target EPort client
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] descrPtr                 - pointer to the frame's descriptor.
* @param[in] clientInfoPtr            - pointer to CNC client info.
*
* @retval GT_OK                    - client is triggered
* @retval GT_FAIL                  - fail in client triggering
*/
static GT_STATUS snetCht3CncClientHaTargetEPortTrigger
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN SNET_CHT3_CNC_CLIENT_INFO_STC * clientInfoPtr
)
{
    DECLARE_FUNC_NAME(snetCht3CncClientHaTargetEPortTrigger);

    GT_U32  regValue;

    if(devObjPtr->errata.HA2CNC_swapIndexAndBc)/*SIPCTRL-74 Ha2CNC wrapper connections are turn around*/
    {
        clientInfoPtr->isErrata_HA2CNC_swapIndexAndBc = 1;
    }

    clientInfoPtr->index = descrPtr->trgEPort;

    if(descrPtr->useVidx == 1 ||
       descrPtr->outGoingMtagCmd == SKERNEL_MTAG_CMD_TO_TRG_SNIFFER_E ||
       descrPtr->outGoingMtagCmd == SKERNEL_MTAG_CMD_TO_CPU_E)
    {
        /* no counting */
        __LOG(("[%s]Counting Disabled : due to VIDX / TO_TRG_SNIFFER / FROM_CPU \n",
            clientInfoPtr->clientsNamePtr));
        return GT_FAIL;
    }

    if((0 == SKERNEL_IS_MATCH_DEVICES_MAC(descrPtr->trgDev, descrPtr->ownDev,
                                                  devObjPtr->dualDeviceIdEnable.ha))
            &&
            descrPtr->eArchExtInfo.assignTrgEPortAttributesLocally == 0)
    {
        /* no counting */
        __LOG(("[%s]Counting Disabled : due to 'not own dev' && 'not locally assigned' \n",
            clientInfoPtr->clientsNamePtr));
        return GT_FAIL;
    }

    /*Use Target ePort as CNC Index*/
    clientInfoPtr->index = descrPtr->trgEPort;

    smemRegGet(devObjPtr, SMEM_CHT3_ROUTE_HA_GLB_CNF_REG(devObjPtr),&regValue);
    clientInfoPtr->byteCntMode = SMEM_U32_GET_FIELD(regValue,2,1);

    return GT_OK;
}

/**
* @internal snetCht3CncClientTunnelStartTrigger function
* @endinternal
*
* @brief   CNC tunnel start client
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] descrPtr                 - pointer to the frame's descriptor.
* @param[in] clientInfoPtr            - pointer to CNC client info.
*
* @retval GT_OK                    - client is triggered
* @retval GT_FAIL                  - fail in client triggering
*/
static GT_STATUS snetCht3CncClientTunnelStartTrigger
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN SNET_CHT3_CNC_CLIENT_INFO_STC * clientInfoPtr
)
{
    GT_U32  regValue;

    if(devObjPtr->errata.HA2CNC_swapIndexAndBc)/*SIPCTRL-74 Ha2CNC wrapper connections are turn around*/
    {
        clientInfoPtr->isErrata_HA2CNC_swapIndexAndBc = 1;
    }

    smemRegGet(devObjPtr, SMEM_CHT3_ROUTE_HA_GLB_CNF_REG(devObjPtr),&regValue);
    clientInfoPtr->index = descrPtr->tunnelPtr;
    clientInfoPtr->byteCntMode = SMEM_U32_GET_FIELD(regValue,1,1);

    return GT_OK;
}

/**
* @internal snetXCatCncClientTunnelTerminatedTrigger function
* @endinternal
*
* @brief   CNC tunnel termination client
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] descrPtr                 - pointer to the frame's descriptor.
* @param[in] clientInfoPtr            - pointer to CNC client info.
*
* @retval GT_OK                    - client is triggered
* @retval GT_FAIL                  - fail in client triggering
*/
static GT_STATUS snetXCatCncClientTunnelTerminatedTrigger
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN SNET_CHT3_CNC_CLIENT_INFO_STC * clientInfoPtr
)
{
    DECLARE_FUNC_NAME(snetXCatCncClientTunnelTerminatedTrigger);

    GT_U32 * regPtr;

    regPtr = smemMemGet(devObjPtr, SMEM_XCAT_A1_TTI_UNIT_GLB_CONF_REG(devObjPtr));

    /* Enables counting by the Ingress Counter engine */
    if (SMEM_U32_GET_FIELD(*regPtr, 3, 1) == 0)
    {
        __LOG(("[%s]Counting Globally Disabled \n",
            clientInfoPtr->clientsNamePtr));
        return GT_FAIL;
    }

    clientInfoPtr->index = clientInfoPtr->userDefined;

    /* Byte count mode - determines whether a packet size is determined by its L2 or L3 size */
    clientInfoPtr->byteCntMode = SMEM_U32_GET_FIELD(*regPtr, 4, 1);

    return GT_OK;
}

/**
* @internal snetCht3CncClientIpclSourceEPortTrigger function
* @endinternal
*
* @brief   CNC IPCL source EPort client
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] descrPtr                 - pointer to the frame's descriptor.
* @param[in] clientInfoPtr            - pointer to CNC client info.
*
* @retval GT_OK                    - client is triggered
* @retval GT_FAIL                  - fail in client triggering
*/
static GT_STATUS snetCht3CncClientIpclSourceEPortTrigger
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN SNET_CHT3_CNC_CLIENT_INFO_STC * clientInfoPtr
)
{
    DECLARE_FUNC_NAME(snetCht3CncClientIpclSourceEPortTrigger);

    GT_U32 fldValue;                /* Register's field value */


    /* The IPCL Source ePort counter is only triggered if SrcDev=OwnDev */
    if(descrPtr->srcDevIsOwn == 0)
    {
        __LOG(("[%s]Counting Disabled : source not from 'own device' \n",
            clientInfoPtr->clientsNamePtr));
        return GT_FAIL;
    }

    smemRegFldGet(devObjPtr,
                  SMEM_CHT3_CNC_VLAN_EN_REG(devObjPtr, descrPtr->localDevSrcPort),
                  descrPtr->localDevSrcPort % 32, 1, &fldValue);
    if (fldValue == 0)
    {
        /* per-port enable for VLAN counting is also applied also to the ePort counting */
        /* SAME registers as the 'snetCht3CncClientL2L3VlanIngrTrigger' */
        /* ingress physical port not enabled for vlan/eport ingress counting */
        __LOG(("[%s] Counting Disabled 'per port' : ingress physical port disabled for vlan/eport ingress counting \n",
            clientInfoPtr->clientsNamePtr));
        return GT_FAIL;
    }

    /* <Ingress ePort Count Mode> */
    smemRegFldGet(devObjPtr, SMEM_CHT3_CNC_COUNT_MODE_REG(devObjPtr), 5, 1,
                  &fldValue);

    /* Set byte count mode for L2 and L3 VLAN ingress client */
    clientInfoPtr->byteCntMode = fldValue;
    /*Use source ePort as CNC Index*/
    clientInfoPtr->index = clientInfoPtr->userDefined;

    clientInfoPtr->isErrata_NotCountCrc = devObjPtr->errata.PCL2CNC_ByteCountWithOut4BytesCrc;

    return GT_OK;
}


/**
* @internal snetLion3CncClientPreEgressPacketTypePassDropTrigger function
* @endinternal
*
* @brief   Sip5 only function.
*         This client allows counting packets based on their type;
*         separated counters are held for dropped or passed packets.
*         Counting is performed in the Pre-egress engine after all replications
*         and dropping mechanisms
* @param[in] devObjPtr                - pointer to device object.
* @param[in] descrPtr                 - pointer to the frame's descriptor.
* @param[in] clientInfoPtr            - pointer to CNC client info.
*
* @retval GT_OK                    - client is triggered
* @retval GT_FAIL                  - fail in client triggering
*/
static GT_STATUS snetLion3CncClientPreEgressPacketTypePassDropTrigger
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN SNET_CHT3_CNC_CLIENT_INFO_STC * clientInfoPtr
)
{
    DECLARE_FUNC_NAME(snetLion3CncClientPreEgressPacketTypePassDropTrigger);

    GT_U32 * regPtr;     /* register data pointer */
    GT_BIT  cpuCodeValid;/*is the CPU code valid*/
    GT_U32  baseIndex;
    GT_BIT  dropBit;/*is the packet dropped */
    GT_U32  outGoingMtagCmd = descrPtr->outGoingMtagCmd;
    GT_U32  cncDropClientMode;
    GT_U32  passDropCncMode;
    GT_U32  passDropCncPaddingMode;
    GT_U32  baseIndexNumBits;
    GT_U32  packetSource = 0;
    GT_U32  firstBit,lastBit;/*first bit and last bit to be used from the hash value*/
    GT_BIT  flowTrackEnable;

    if(SMEM_CHT_IS_SIP6_GET(devObjPtr))
    {
        /* check new in SIP_6 mode - Hash Client */
        regPtr = smemMemGet(devObjPtr, SMEM_SIP6_EQ_PRE_EGR_GLB_CONF_2_REG(devObjPtr));
        cncDropClientMode = SMEM_U32_GET_FIELD(*regPtr, 0, 1);

        if(SMEM_CHT_IS_SIP6_10_GET(devObjPtr))
        { /* SIP_6_10 */
            /* Get value from dedicated field */
            flowTrackEnable = descrPtr->flowTrackEn;
        }
        else
        { /* SIP_6 */
            /* Get value from bit#1 in copyreserved field */
            flowTrackEnable = descrPtr->copyReserved & (1 << SNET_CNC_COPY_RESERVED_FLOW_TRACK_ENABLE_BIT_CNS);
        }

        __LOG_PARAM(cncDropClientMode);
        if((cncDropClientMode == 1) && flowTrackEnable )
        {
            __LOG(("Use Packet Hash [%d] in index to CNC Packet Type Pass/Drop \n",
                 descrPtr->pktHash));

            /* Flow Tracking. Use packet hash as CNC key */
            regPtr = smemMemGet(devObjPtr, SMEM_SIP6_EQ_COARSE_GRAIN_DETECTOR_HASH_BIT_SELECT_CONFIG_REG(devObjPtr));
            firstBit = SMEM_U32_GET_FIELD(*regPtr, 0, 5);
            lastBit  = SMEM_U32_GET_FIELD(*regPtr, 5, 5);

            __LOG_PARAM(firstBit);
            __LOG_PARAM(lastBit);

            if(firstBit && ((lastBit + 1)  < firstBit))
            {
                __LOG(("ERROR : lastBit[%d] less than firstBit[%d] \n",
                    lastBit ,
                    firstBit));

                lastBit = firstBit - 1;
            }

            /* Hash Client index has 16 bits */
            clientInfoPtr->index =  (SMEM_U32_GET_FIELD(descrPtr->pktHash,firstBit,((lastBit+1) -firstBit)) & 0xFFFF);

            /* Set byte count mode for EQ clients */
            regPtr = smemMemGet(devObjPtr, SMEM_CHT_PRE_EGR_GLB_CONF_REG(devObjPtr));
            clientInfoPtr->byteCntMode = SMEM_U32_GET_FIELD(*regPtr, 13, 1);

            return GT_OK;
        }
    }

    dropBit = clientInfoPtr->userDefined;
    __LOG_PARAM(dropBit);

    cpuCodeValid = /* CPU code valid for drops (in sip5) and for TO_CPU */
        (dropBit || outGoingMtagCmd == SKERNEL_MTAG_CMD_TO_CPU_E)
          ? 1 : 0;

    __LOG_PARAM(cpuCodeValid);

    regPtr = smemMemGet(devObjPtr, SMEM_CHT_PRE_EGR_GLB_CONF_REG(devObjPtr));

    if ((dropBit == 1) || (descrPtr->outGoingMtagCmd == SKERNEL_MTAG_CMD_TO_CPU_E))
    {
        /* Defines the operation mode for the CNC Client: Packet Type Drop/Pass Counter
            0 = Code: Use packet CPU/DROP_Code in index to CNC Packet Type Pass/Drop
            1 = Src Port: Use physical source port in index to CNC Packet Type Pass/Drop
        */
        if(SMEM_CHT_IS_SIP6_30_GET(devObjPtr))
        {
            smemRegFldGet(devObjPtr,
                SMEM_SIP6_EQ_PRE_EGR_GLB_CONF_2_REG(devObjPtr),13,2,&passDropCncMode);
        }
        else
        {
            passDropCncMode = SMEM_U32_GET_FIELD(*regPtr, 16, 1);
        }

        __LOG_PARAM(passDropCncMode);

        if(/*(cpuCodeValid == 0) ||*/ (passDropCncMode == 1))
        {
            /*Use physical source port in index to CNC Packet Type Pass/Drop*/
            baseIndex = descrPtr->localDevSrcPort;

            __LOG(("Use physical source port [%d] in index to CNC Packet Type Pass/Drop \n",
                baseIndex));

            passDropCncMode = 1;/* allow next code to refer to the index mode */

            baseIndexNumBits =
                SMEM_CHT_IS_SIP6_GET(devObjPtr)    ? 10:
                SMEM_CHT_IS_SIP5_20_GET(devObjPtr) ?  9: 8;
        }
        else
        {
            baseIndex = cpuCodeValid ? descrPtr->cpuCode : 0;
            __LOG(("Use packet CPU/DROP_Code [%d] in index to CNC Packet Type Pass/Drop \n",
                baseIndex));

            baseIndexNumBits = 8;
        }
   }
   else
   {
       /*Use physical source port in index to CNC Packet Type Pass/Drop*/
       baseIndex = descrPtr->localDevSrcPort;

       __LOG(("Use physical source port [%d] in index to CNC Packet Type Pass/Drop \n",
            baseIndex));

       passDropCncMode = 1;/* allow next code to refer to the index mode */

       baseIndexNumBits =
            SMEM_CHT_IS_SIP6_GET(devObjPtr)    ? 10:
            SMEM_CHT_IS_SIP5_20_GET(devObjPtr) ?  9: 8;
   }

    if(SMEM_CHT_IS_SIP5_10_GET(devObjPtr))
    {
        /* new field */
        /*
            ingress --> 0
            egress  --> 1
            sniffer --> 2
            mirror  --> 3
            STC     --> 4
        */

        /*<Pass Drop CNC Padding Mode>*/
        passDropCncPaddingMode = SMEM_U32_GET_FIELD(*regPtr, 11, 1);

        if(passDropCncPaddingMode)
        {
            packetSource = descrPtr->txqToEq ? 1 :
                           descrPtr->analyzerIndex ? 3 :
                           descrPtr->incomingMtagCmd == SKERNEL_MTAG_CMD_TO_TRG_SNIFFER_E ? 2 :
                           0;

            SIM_TBD_BOOKMARK/* not supported yet ,
                    see logic in snetChtEqCncCount(...) that not calls for 'egress'
                    and also without any EQ replications */
            __LOG_PARAM(packetSource);
        }
    }

    baseIndex &= SMEM_BIT_MASK(baseIndexNumBits);

    __LOG_PARAM(baseIndex);

    clientInfoPtr->index =  (packetSource       << (baseIndexNumBits+4)) |
                            (outGoingMtagCmd    << (baseIndexNumBits+1)) |
                            (dropBit            << (baseIndexNumBits+0)) |
                            baseIndex;

    /* Update CNC index in case operation mode of Drop/Pass Counter is of "Stream Gate mode" */
    if(SMEM_CHT_IS_SIP6_30_GET(devObjPtr))
    {
        /* Get the operation mode for the CNC Client: Packet Type Drop/Pass Counter */
        smemRegFldGet(devObjPtr,SMEM_SIP6_EQ_PRE_EGR_GLB_CONF_2_REG(devObjPtr),13,2,&passDropCncMode);

        /* Check if the operation mode is of "Stream Gate Mode". In Stream Gate Mode,
           the CNC client is only applied if packet is not an ingress/egress mirrored copy */
        if ((passDropCncMode == 2) && (descrPtr->analyzerIndex == 0))
        {
            /* Stream Gate Mode CNC Index Format: | Zero padding | Gate State  |  Gate ID   |
                                                  | index[15:11] | index[10:9] | index[8:0] |  */
            clientInfoPtr->index = ((descrPtr->gateState & 0x3) << 9) | (descrPtr->gateId & 0x1FF);
        }
    }

    /* Set byte count mode for EQ clients */
    clientInfoPtr->byteCntMode = SMEM_U32_GET_FIELD(*regPtr, 13, 1);

    return GT_OK;
}


/**
* @internal snetCht3CncCount function
* @endinternal
*
* @brief   Trigger CNC event for specified client and set CNC counter block
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] descrPtr                 - pointer to the frame's descriptor.
* @param[in] client                   -  ID.
* @param[in] userDefined              - user defined parameter
*
* @retval GT_OK                    - CNC event triggered
* @retval GT_NOT_SUPPORTED         - CNC feature not supported
* @retval GT_FAIL                  - CNC event is not triggered
* @retval GT_BAD_PARAM             - wrong CNC client index
*/
void snetCht3CncCount
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN SNET_CNC_CLIENT_E client,
    IN GT_UINTPTR userDefined
)
{
    DECLARE_FUNC_NAME(snetCht3CncCount);

    SNET_CHT3_CNC_CLIENT_INFO_STC clientInfo;   /* client info structure */
    GT_STATUS rc;                               /* return code */
    GT_CHAR*    clientNamePtr;
    GT_U32 cncUnitIndex;
    GT_U32 maxCncUnits;
    GT_U32  clientInstance;
    GT_U32  cncClientInstances = devObjPtr->cncClientInstances ? devObjPtr->cncClientInstances : 1;

    memset(&clientInfo, 0, sizeof(clientInfo));

    if (SKERNEL_IS_CHEETAH3_DEV(devObjPtr) == 0)
    {
        return;
    }

    simLogPacketFrameUnitSet(SIM_LOG_FRAME_UNIT_CNC_E);

    maxCncUnits  = devObjPtr->cncNumOfUnits ? devObjPtr->cncNumOfUnits : 1;

    /* Initialize input parameters of structure */
    clientInfo.client = client;
    clientInfo.userDefined = userDefined;

    if(client >= SNET_CNC_CLIENT_LAST_E)
    {
        clientInfo.clientsNamePtr = "unknown";
    }
    else
    if(SMEM_CHT_IS_SIP6_GET(devObjPtr))
    {
        clientInfo.clientsNamePtr = sip6_cncClientsNamesArr[client];
    }
    else
    {
        clientInfo.clientsNamePtr = cncClientsNamesArr[client];
    }

    clientNamePtr = clientInfo.clientsNamePtr;

    for(cncUnitIndex = 0 ; cncUnitIndex < maxCncUnits ; cncUnitIndex++)
    {
        if(SMEM_CHT_IS_SIP5_15_GET(devObjPtr) &&
            !SMEM_CHT_IS_SIP5_20_GET(devObjPtr))
        {
            /* SIP5.15: Ignore half of blocks range on TTI lookups */
            switch (client)
            {
                case SNET_CNC_CLIENT_TTI_ACTION_0_E:
                case SNET_CNC_CLIENT_TTI_ACTION_1_E:
                    if (cncUnitIndex > 0)
                    {
                        __LOG(("SIP5.15: Skip CNC unit[%d] for TTI lookups 0,1 - CNC client [%s] \n",
                                    cncUnitIndex,clientNamePtr));
                        continue;
                    }
                    break;
                case SNET_CNC_CLIENT_TTI_ACTION_2_E:
                case SNET_CNC_CLIENT_TTI_ACTION_3_E:
                    if (cncUnitIndex == 0)
                    {
                        __LOG(("SIP5.15: Skip CNC unit[%d] for TTI lookups 2,3 - CNC client [%s] \n",
                                    cncUnitIndex,clientNamePtr));
                        continue;
                    }
                    break;
                default:
                    /* Do nothing */
                    break;
            }
        }

        clientInfo.cncUnitIndex = cncUnitIndex;
        __LOG(("Check CNC client triggering [%s] in CNC unit[%d] \n",
            clientNamePtr,cncUnitIndex));
        rc = snetCht3CncTrigger(devObjPtr, descrPtr, &clientInfo);
        if (rc == GT_OK)
        {
            /* Set CNC block for specified client */
            __LOG(("the Client [%s] is enabled in CNC unit[%d] \n",
                clientNamePtr,cncUnitIndex));

            for(clientInstance = 0 ;
                clientInstance < cncClientInstances;
                clientInstance++)
            {
                clientInfo.fewClientsPerBlock.clientInstance = clientInstance;
                snetCht3CncBlockSet(devObjPtr, descrPtr, &clientInfo);
            }
        }
        else
        if(rc == GT_NO_SUCH)/* new in SIP6_10 */
        {
            __LOG(("NOTE: The Client [%s] is not wired to CNC unit[%d] \n",
                clientNamePtr,cncUnitIndex));
        }
        else
        {
            __LOG(("the Client [%s] is disabled in CNC unit[%d] \n",
                clientNamePtr,cncUnitIndex));
        }
    }

    simLogPacketFrameUnitSet(SIM_LOG_FRAME_UNIT___RESTORE_PREVIOUS_UNIT___E);


    return ;
}

/**
* @internal snetCht3CncBlockWrapStatusSet function
* @endinternal
*
* @brief   Set CNC Block Wraparound Status Register
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] block                    - wraparound counter block.
* @param[in] index                    - wraparound counter index.
* @param[in] cncUnitIndex             the CNC unit index (0/1) (Sip5 devices , legacy device only 1 unit)
*
* @retval GT_OK                    - CNC wraparound status table was successfully written
* @retval GT_FAIL                  - CNC wraparound status table write fail
*/
static GT_STATUS snetCht3CncBlockWrapStatusSet
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32 block,
    IN GT_U32 index,
    IN GT_U32   cncUnitIndex
)
{
    GT_U32 regAddr;                 /* Resister address */
    GT_U32 * regPtr;                /* register data pointer */
    GT_U32 fldValue;                /* Register's field value */
    GT_U32 entry;                   /* Number of counters in wrap-around status table */
    GT_U32 intrNo;                  /* Interrupt cause bit */
    GT_STATUS status = GT_FAIL;

    /* CNC Block <block> Wraparound Status Register 0 */
    regAddr = SMEM_CHT3_CNC_BLOCK_WRAP_AROUND_STATUS_REG(devObjPtr, block, 0,cncUnitIndex);
    regPtr = smemMemGet(devObjPtr, regAddr);

    /* if the index already exists in the Wraparound table do nothing */
    for (entry = 0; entry < 4; entry++)
    {
        /* Block wraparound valid bit 0 */
        fldValue = SMEM_U32_GET_FIELD(regPtr[entry], 0, 1);
        /* Entry is in use */
        if (fldValue == 1)
        {
            if( SMEM_U32_GET_FIELD(regPtr[entry], 1, 12) ==  index)
            {
                return GT_OK;
            }
        }

        /* Block wraparound valid bit 16 */
        fldValue = SMEM_U32_GET_FIELD(regPtr[entry], 16, 1);
        /* Entry is in use */
        if (fldValue == 1)
        {
            if( SMEM_U32_GET_FIELD(regPtr[entry], 17, 12) ==  index)
            {
                return GT_OK;
            }
        }
    }

    for (entry = 0; entry < 4; entry++)
    {
        /* Block wraparound valid bit 0 */
        fldValue = SMEM_U32_GET_FIELD(regPtr[entry], 0, 1);
        /* Entry is not in use */
        if (fldValue == 0)
        {
            /* Set block wraparound index */
            SMEM_U32_SET_FIELD(regPtr[entry], 1, 12, index);
            SMEM_U32_SET_FIELD(regPtr[entry],0,1,1);
            status = GT_OK;
            break;
        }

        /* Block wraparound valid bit 16 */
        fldValue = SMEM_U32_GET_FIELD(regPtr[entry], 16, 1);
        /* Entry is not in use */
        if (fldValue == 0)
        {
            /* Set block wraparound index */
            SMEM_U32_SET_FIELD(regPtr[entry], 17, 12, index);
            SMEM_U32_SET_FIELD(regPtr[entry],16,1,1);
            status = GT_OK;
            break;
        }
    }

    intrNo = SMEM_CHT3_CNC_BLOCK_WRAP_AROUND(block);
    snetChetahDoInterrupt(devObjPtr,
                          SMEM_CHT3_CNC_INTR_CAUSE_REG(devObjPtr,cncUnitIndex),
                          SMEM_CHT3_CNC_INTR_MASK_REG(devObjPtr,cncUnitIndex),
                          intrNo,
                          SMEM_CHT_CNC_ENGINE_INT(devObjPtr,cncUnitIndex));

    return status;
}
/**
* @internal snetCht3CncCounterFormatGet function
* @endinternal
*
* @brief   The function gets format of CNC counter
*
* @note   APPLICABLE DEVICES:      Lion and above.
*
* @param[out] formatPtr                - (pointer to) CNC counter format
*/
static GT_VOID snetCht3CncCounterFormatGet
(
    IN SKERNEL_DEVICE_OBJECT                * devObjPtr,
    IN  GT_U32                              block,
    OUT CNC_COUNTER_FORMAT_ENT              *formatPtr,
    IN GT_U32   cncUnitIndex
)
{
    GT_U32  regAddr;                /* register address         */
    GT_U32  * regPtr;               /* register's data pointer  */
    GT_U32  fldValue;               /* register's field value   */
    GT_U32  startBit;
    GT_U32  numOfBits = 2;

    *formatPtr = CNC_COUNTER_FORMAT_PKT_29_BC_35_E;
    if(SKERNEL_IS_LION_REVISON_B0_DEV(devObjPtr) == 0)
    {
        /* CNC counter format is not supported */
        return;
    }

    if(SMEM_CHT_IS_SIP5_20_GET(devObjPtr))
    {
        numOfBits = 3;
        /*Blocks Counter Entry Mode Register%r*/
        regAddr = SMEM_SIP5_20_CNC_BLOCK_ENTRY_MODE_REG( devObjPtr, block, cncUnitIndex);
        regPtr = smemMemGet(devObjPtr, regAddr);
        startBit = 3* (block % 8);

    }
    else
    {
        /*CNC Block %n Configuration Register0*/
        regAddr = SMEM_LION_CNC_BLOCK_CNF0_REG( devObjPtr, block, cncUnitIndex,0/*BWC*/);
        regPtr = smemMemGet(devObjPtr, regAddr);

        if(0 == SMEM_CHT_IS_SIP5_GET(devObjPtr))
        {
            startBit = 12;
        }
        else
        {
            startBit = 30;
        }
    }

    /*<Block%nCounterEntryMode>*/
    fldValue = SMEM_U32_GET_FIELD(regPtr[0], startBit, numOfBits);
    switch (fldValue)
    {
        default:
            break;
        case 1:
            *formatPtr = CNC_COUNTER_FORMAT_PKT_27_BC_37_E;
            break;
        case 2:
            *formatPtr = CNC_COUNTER_FORMAT_PKT_37_BC_27_E;
            break;
        case 3:
            if(SMEM_CHT_IS_SIP5_20_GET(devObjPtr))
            {
                *formatPtr = CNC_COUNTER_FORMAT_PKT_64_BC_0_E;
            }
            break;
        case 4:
            if(SMEM_CHT_IS_SIP5_20_GET(devObjPtr))
            {
                *formatPtr = CNC_COUNTER_FORMAT_PKT_0_BC_64_E;
            }
            break;
        case 5:
            if(SMEM_CHT_IS_SIP6_GET(devObjPtr))
            {
                *formatPtr = CNC_COUNTER_FORMAT_PKT_44_MAX_SIZE_20_E;
            }
            break;
    }

}

/**
* @internal snetCht3CncCounterHwRead function
* @endinternal
*
* @brief   The function read the 64-bit HW counter to SW format.
*
* @note   APPLICABLE DEVICES:      DxCh3 and above.
* @param[in] format                   - CNC counter HW format,
*                                      relevant only for Lion and above
* @param[in] regPtr                   - (pointer to) CNC Counter register
*
* @param[out] cncCounterPtr            - (pointer to) CNC Counter in SW format
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
static GT_STATUS snetCht3CncCounterHwRead
(
    IN   CNC_COUNTER_FORMAT_ENT         format,
    IN   GT_U32                         * regPtr,
    OUT  SNET_CHT3_CNC_COUNTER_STC      * cncCounterPtr
)
{
    switch (format)
    {
        case CNC_COUNTER_FORMAT_PKT_29_BC_35_E:
            /* Packets counter: 29 bits, Byte Count counter: 35 bits */
            cncCounterPtr->packetCount.l[0] =
                (regPtr[0] & 0x1FFFFFFF);
            cncCounterPtr->packetCount.l[1] = 0;
            cncCounterPtr->byteCount.l[0] = regPtr[1];
            cncCounterPtr->byteCount.l[1] =
                ((regPtr[0] >> 29) & 0x07);
            break;

        case CNC_COUNTER_FORMAT_PKT_27_BC_37_E:
            /* Packets counter: 27 bits, Byte Count counter: 37 bits */
            cncCounterPtr->packetCount.l[0] =
                (regPtr[0] & 0x07FFFFFF);
            cncCounterPtr->packetCount.l[1] = 0;
            cncCounterPtr->byteCount.l[0] = regPtr[1];
            cncCounterPtr->byteCount.l[1] =
                ((regPtr[0] >> 27) & 0x1F);
            break;

        case CNC_COUNTER_FORMAT_PKT_37_BC_27_E:
            /* Packets counter: 37 bits, Byte Count counter: 27 bits */
            cncCounterPtr->packetCount.l[0] = regPtr[0];
            cncCounterPtr->packetCount.l[1] =
                ((regPtr[1] >> 27) & 0x1F);
            cncCounterPtr->byteCount.l[0] =
                (regPtr[1] & 0x07FFFFFF);
            cncCounterPtr->byteCount.l[1] = 0;
            break;
        case CNC_COUNTER_FORMAT_PKT_64_BC_0_E:
            /* Packets counter: 64 bits, Byte Count counter: 0 bits */
            cncCounterPtr->packetCount.l[0] = regPtr[0];
            cncCounterPtr->packetCount.l[1] = regPtr[1];
            cncCounterPtr->byteCount.l[0] = 0;
            cncCounterPtr->byteCount.l[1] = 0;
            break;
        case CNC_COUNTER_FORMAT_PKT_0_BC_64_E:
            /* Packets counter: 64 bits, Byte Count counter: 0 bits */
            cncCounterPtr->packetCount.l[0] = 0;
            cncCounterPtr->packetCount.l[1] = 0;
            cncCounterPtr->byteCount.l[0] = regPtr[0];
            cncCounterPtr->byteCount.l[1] = regPtr[1];
            break;
        case CNC_COUNTER_FORMAT_PKT_44_MAX_SIZE_20_E:
            /* Packets counter: 44 bits, Byte Count counter: 20 bits */
            cncCounterPtr->packetCount.l[0] = regPtr[0];/*32 bits*/
            cncCounterPtr->packetCount.l[1] = regPtr[1] >> 20;/*12 bits*/
            cncCounterPtr->byteCount.l[0]   = regPtr[1] & 0x000FFFFF;/*20 bits*/
            cncCounterPtr->byteCount.l[1]   = 0;
            break;
        default: return GT_BAD_PARAM;
    }

    return GT_OK;
}

/**
* @internal snetCht3CncCounterHwWrite function
* @endinternal
*
* @brief   The function writes the SW counter to HW in 64-bit HW format.
*
* @note   APPLICABLE DEVICES:      DxCh3 and above.
* @param[in] format                   - CNC counter HW format,
*                                      relevant only for Lion and above
* @param[in] cncCounterPtr            - (pointer to) CNC Counter in SW format
*
* @param[out] regPtr                   - (pointer to) CNC Counter register
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
static GT_STATUS snetCht3CncCounterHwWrite
(
    IN   CNC_COUNTER_FORMAT_ENT         format,
    IN   GT_U32                         * regPtr,
    IN   SNET_CHT3_CNC_COUNTER_STC      * cncCounterPtr
)
{
    switch (format)
    {
        case CNC_COUNTER_FORMAT_PKT_29_BC_35_E:
            /* Packets counter: 29 bits, Byte Count counter: 35 bits */
            regPtr[0] =
                (cncCounterPtr->packetCount.l[0] & 0x1FFFFFFF)
                | (cncCounterPtr->byteCount.l[1] << 29);
            regPtr[1] = cncCounterPtr->byteCount.l[0];
            break;

        case CNC_COUNTER_FORMAT_PKT_27_BC_37_E:
            /* Packets counter: 27 bits, Byte Count counter: 37 bits */
            regPtr[0] =
                (cncCounterPtr->packetCount.l[0] & 0x07FFFFFF)
                | (cncCounterPtr->byteCount.l[1] << 27);
            regPtr[1] = cncCounterPtr->byteCount.l[0];
            break;

        case CNC_COUNTER_FORMAT_PKT_37_BC_27_E:
            /* Packets counter: 37 bits, Byte Count counter: 27 bits */
            regPtr[0] = cncCounterPtr->packetCount.l[0];
            regPtr[1] =
                (cncCounterPtr->byteCount.l[0] & 0x07FFFFFF)
                | (cncCounterPtr->packetCount.l[1] << 27);
            break;

        case CNC_COUNTER_FORMAT_PKT_64_BC_0_E:
            /* Packets counter: 64 bits, Byte Count counter: 0 bits */
            regPtr[0] = cncCounterPtr->packetCount.l[0];
            regPtr[1] = cncCounterPtr->packetCount.l[1];
            break;

        case CNC_COUNTER_FORMAT_PKT_0_BC_64_E:
            /* Packets counter: 0 bits, Byte Count counter: 64 bits */
            regPtr[0] = cncCounterPtr->byteCount.l[0];
            regPtr[1] = cncCounterPtr->byteCount.l[1];
            break;
        case CNC_COUNTER_FORMAT_PKT_44_MAX_SIZE_20_E:
            /* Packets counter: 44 bits, Byte Count counter: 20 bits */
            regPtr[0] = cncCounterPtr->packetCount.l[0];
            regPtr[1] =
                (cncCounterPtr->byteCount.l[0] & 0x000FFFFF)
                | (cncCounterPtr->packetCount.l[1] << 20);
            break;


        default: return GT_BAD_PARAM;
    }

    return GT_OK;
}

/**
* @internal snetCht3CncCounterWrapAroundCheck function
* @endinternal
*
* @brief   The function checks the 64-bit HW counter wraparound.
*
* @note   APPLICABLE DEVICES:      DxCh3 and above.
* @param[in] wrapEn                   - wraparound enabled/disabled
* @param[in] format                   - CNC counter HW format,
*                                      relevant only for Lion and above
* @param[in] cncCounterPtr            - (pointer to) CNC Counter in SW format
*
* @retval GT_OK                    - on success
* @retval GT_FULL                  - counter(s) value has reached it's maximal value
* @retval GT_BAD_PARAM             - on wrong parameters
*/
static GT_STATUS snetCht3CncCounterWrapAroundCheck
(
    IN   GT_U32 wrapEn,
    IN   CNC_COUNTER_FORMAT_ENT         format,
    IN   SNET_CHT3_CNC_COUNTER_STC      * cncCounterPtr,
    IN   GT_BIT                         do64BitsByteWrapAround
)
{
    GT_BOOL wrapAround = GT_FALSE;

    switch (format)
    {
        case CNC_COUNTER_FORMAT_PKT_29_BC_35_E:
            /* Packets counter: 29 bits, Byte Count counter: 35 bits */
            if(cncCounterPtr->packetCount.l[0] >= 0x1FFFFFFF)
            {
                /* Wraparound disabled */
                if(wrapEn == 0)
                {
                    cncCounterPtr->packetCount.l[0] = 0x1FFFFFFF;
                }
                wrapAround = GT_TRUE;
            }
            if(cncCounterPtr->byteCount.l[1] > 0x7 ||
              (cncCounterPtr->byteCount.l[0] == 0xFFFFFFFF &&
               cncCounterPtr->byteCount.l[1] == 0x7))
            {
                /* Wraparound disabled */
                if(wrapEn == 0)
                {
                    cncCounterPtr->byteCount.l[0] = 0xFFFFFFFF;
                    cncCounterPtr->byteCount.l[1] = 0x7;
                }
                wrapAround = GT_TRUE;
            }
            break;

        case CNC_COUNTER_FORMAT_PKT_27_BC_37_E:
            /* Packets counter: 27 bits, Byte Count counter: 37 bits */
            if(cncCounterPtr->packetCount.l[0] >= 0x07FFFFFF)
            {
                /* Wraparound disabled */
                if(wrapEn == 0)
                {
                    cncCounterPtr->packetCount.l[0] = 0x07FFFFFF;
                }
                wrapAround = GT_TRUE;
            }
            if(cncCounterPtr->byteCount.l[1] > 0x1F ||
               (cncCounterPtr->byteCount.l[0] == 0xFFFFFFFF &&
                cncCounterPtr->byteCount.l[1] == 0x1F))
            {
                /* Wraparound disabled */
                if(wrapEn == 0)
                {
                    cncCounterPtr->byteCount.l[0] = 0xFFFFFFFF;
                    cncCounterPtr->byteCount.l[1] = 0x1F;
                }
                wrapAround = GT_TRUE;
            }
            break;

        case CNC_COUNTER_FORMAT_PKT_37_BC_27_E:
            /* Packets counter: 37 bits, Byte Count counter: 27 bits */
            if(cncCounterPtr->packetCount.l[1] > 0x1F ||
              (cncCounterPtr->packetCount.l[0] == 0xFFFFFFFF &&
               cncCounterPtr->packetCount.l[1] == 0x1F))
            {
                /* Wraparound disabled */
                if(wrapEn == 0)
                {
                    cncCounterPtr->packetCount.l[0] = 0xFFFFFFFF;
                    cncCounterPtr->packetCount.l[1] = 0x1F;
                }
                wrapAround = GT_TRUE;
            }

            if(cncCounterPtr->byteCount.l[0] >= 0x07FFFFFF)
            {
                /* Wraparound disabled */
                if(wrapEn == 0)
                {
                    cncCounterPtr->byteCount.l[0] = 0x07FFFFFF;
                }
                wrapAround = GT_TRUE;
            }
            break;

        case CNC_COUNTER_FORMAT_PKT_64_BC_0_E:
            /* Packets counter: 64 bits, Byte Count counter: 0 bits */
            if(cncCounterPtr->packetCount.l[0] == 0xFFFFFFFF &&
               cncCounterPtr->packetCount.l[1] == 0xFFFFFFFF)
            {
                wrapAround = GT_TRUE;
            }
            break;

        case CNC_COUNTER_FORMAT_PKT_0_BC_64_E:
            /* Packets counter: 0 bits, Byte Count counter: 64 bits */
            if(do64BitsByteWrapAround)
            {
                if(wrapEn == 0)
                {
                    cncCounterPtr->byteCount.l[0] = 0xFFFFFFFF;
                    cncCounterPtr->byteCount.l[1] = 0xFFFFFFFF;
                }
                wrapAround = GT_TRUE;
            }
            break;

        case CNC_COUNTER_FORMAT_PKT_44_MAX_SIZE_20_E:
            /* Packets counter: 44 bits, Byte Count counter: 20 bits */
            if(cncCounterPtr->packetCount.l[1] > 0xFFF ||
              (cncCounterPtr->packetCount.l[0] == 0xFFFFFFFF &&
               cncCounterPtr->packetCount.l[1] == 0xFFF))
            {
                /* Wraparound disabled */
                if(wrapEn == 0)
                {
                    cncCounterPtr->packetCount.l[0] = 0xFFFFFFFF;
                    cncCounterPtr->packetCount.l[1] = 0xFFF;
                }
                wrapAround = GT_TRUE;
            }

            if(cncCounterPtr->byteCount.l[0] >= 0x000FFFFF)
            {
                /* Wraparound disabled */
                if(wrapEn == 0)
                {
                    cncCounterPtr->byteCount.l[0] = 0x000FFFFF;
                }
                wrapAround = GT_TRUE;
            }
            break;

        default:
            return GT_BAD_PARAM;
    }

    return ((wrapAround == GT_TRUE) ? GT_FULL : GT_OK);
}



/**
* @internal snetPipeCncClientPcpDestIndexTrigger function
* @endinternal
*
* @brief   in the PIPE device this client from the PCP unit for the destination ports bmp table index
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] descrPtr                 - pointer to the frame's descriptor.
* @param[in] clientInfoPtr            - pointer to CNC client info.
*
* @retval GT_OK                    - client is triggered
* @retval GT_FAIL                  - fail in client triggering
*/
static GT_STATUS snetPipeCncClientPcpDestIndexTrigger
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN SNET_CHT3_CNC_CLIENT_INFO_STC * clientInfoPtr
)
{

    clientInfoPtr->index = clientInfoPtr->userDefined;

    /* Byte count mode - determines whether a packet size is determined by its L2 or L3 size */
    clientInfoPtr->byteCntMode = SMEM_CHT3_CNC_BYTE_CNT_L2_MODE_E;

    return GT_OK;
}

/**
* @internal snetPipeCncClientPpaTrgPortPktTypeTrigger function
* @endinternal
*
* @brief   in the PIPE device this client from the PPA unit for the (TrgPort, PktType)
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] descrPtr                 - pointer to the frame's descriptor.
* @param[in] clientInfoPtr            - pointer to CNC client info.
*
* @retval GT_OK                    - client is triggered
* @retval GT_FAIL                  - fail in client triggering
*/
static GT_STATUS snetPipeCncClientPpaTrgPortPktTypeTrigger
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN SNET_CHT3_CNC_CLIENT_INFO_STC * clientInfoPtr
)
{

    clientInfoPtr->index = clientInfoPtr->userDefined;

    /* Byte count mode - determines whether a packet size is determined by its L2 or L3 size */
    clientInfoPtr->byteCntMode = SMEM_CHT3_CNC_BYTE_CNT_L2_MODE_E;

    return GT_OK;
}

/**
* @internal snetCht3CncClientErepPacketTypePassDropTrigger function
* @endinternal
*
* @brief  Sip6 only function.
*         This client allows counting packets based on their type;
*         separated counters are held for dropped or passed packets.
*         Counting is performed in the Erep engine after all replications
*         and dropping mechanisms
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] descrPtr                 - pointer to the frame's descriptor.
* @param[in] clientInfoPtr            - pointer to CNC client info.
*
* @retval GT_OK                    - client is triggered
* @retval GT_FAIL                  - fail in client triggering
*/
static GT_STATUS snetCht3CncClientErepPacketTypePassDropTrigger
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN SNET_CHT3_CNC_CLIENT_INFO_STC * clientInfoPtr
)
{
    DECLARE_FUNC_NAME(snetCht3CncClientErepPacketTypePassDropTrigger);

    GT_U32 * regPtr;     /* register data pointer */
    GT_BIT  cpuCodeValid;/*is the CPU code valid*/
    GT_U32  cpuCode;
    GT_U32  baseIndex;
    GT_BIT  dropBit;/*is the packet dropped */
    GT_U32  egrPacketCmd; /* egress processing packet command */

    dropBit = (descrPtr->packetCmd == SKERNEL_EXT_PKT_CMD_HARD_DROP_E ||
               descrPtr->packetCmd == SKERNEL_EXT_PKT_CMD_SOFT_DROP_E) ? 1 : 0;

    __LOG_PARAM(dropBit);

    cpuCodeValid = /* CPU code valid for drops (in sip5) and for TO_CPU */
      (dropBit || (descrPtr->packetCmd == SKERNEL_EXT_PKT_CMD_MIRROR_TO_CPU_E ||
                     descrPtr->packetCmd == SKERNEL_EXT_PKT_CMD_TRAP_TO_CPU_E))
       ? 1 : 0;

    __LOG_PARAM(cpuCodeValid);

    regPtr = smemMemGet(devObjPtr, SMEM_SIP6_EREP_GLB_CFG_REG(devObjPtr));

    cpuCode = ((dropBit == 1) || (cpuCodeValid == 1)) ? descrPtr->cpuCode : 0;

    baseIndex = 0xff & cpuCode;

    /* convert packet type to Egress Packet Command:
        0 - Forward
        1 - Mirror
        2 - Trap
        3 - Hard Drop
        4 - Soft Drop */
    egrPacketCmd = descrPtr->packetCmd;

    /* FROM_CPU and TO_TRG_SNIFFER became Forward in egress processing */
    if((descrPtr->packetCmd == SKERNEL_EXT_PKT_CMD_FROM_CPU_E) ||
       (descrPtr->packetCmd == SKERNEL_EXT_PKT_CMD_TO_TRG_SNIFFER_E))
    {
        egrPacketCmd = SKERNEL_EXT_PKT_CMD_FORWARD_E;
    }

    clientInfoPtr->index = (egrPacketCmd << 8) |
                           baseIndex;

    clientInfoPtr->index = clientInfoPtr->index & 0x7ff;

     __LOG_PARAM(clientInfoPtr->index);

    clientInfoPtr->byteCntMode = SMEM_U32_GET_FIELD(*regPtr, 1, 1);

    return GT_OK;
}

/**
* @internal snetSip6CncClientPreqQueueStatisticTrigger function
* @endinternal
*
* @brief   CNC preq : common to port/queue statistics
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] descrPtr                 - pointer to the frame's descriptor.
* @param[in] clientInfoPtr            - pointer to CNC client info.
*
* @retval GT_OK                    - client is triggered
* @retval GT_FAIL                  - fail in client triggering
*/
static GT_STATUS snetSip6CncClientPreqCommonStatisticTrigger
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN SNET_CHT3_CNC_CLIENT_INFO_STC * clientInfoPtr
)
{
    DECLARE_FUNC_NAME(snetSip6CncClientPreqCommonStatisticTrigger);

    clientInfoPtr->triggered = SMEM_U32_GET_FIELD(clientInfoPtr->userDefined,SNET_CNC_PREQ_TRIGGER_BIT_INDEX_CNC,1);
    clientInfoPtr->index     = SMEM_U32_GET_FIELD(clientInfoPtr->userDefined,0,SNET_CNC_PREQ_TRIGGER_BIT_INDEX_CNC);
    clientInfoPtr->explicitByteCountValue = SMEM_U32_GET_FIELD(clientInfoPtr->userDefined,SNET_CNC_PREQ_TRIGGER_BIT_INDEX_CNC+1,31);

    /* Byte count mode - determines whether a packet size is determined by its L2 or L3 size */
    clientInfoPtr->byteCntMode = SMEM_CHT3_CNC_BYTE_CNT_FLEX_VALUE_E;

     __LOG_PARAM(clientInfoPtr->triggered);
     __LOG_PARAM(clientInfoPtr->index);
     __LOG_PARAM(clientInfoPtr->explicitByteCountValue);

    clientInfoPtr->debug_is_expected_counter_format_CNC_COUNTER_FORMAT_PKT_44_MAX_SIZE_20_E = 1;

    return GT_OK;
}

/**
* @internal snetSip6CncClientPreqQueueStatisticTrigger function
* @endinternal
*
* @brief   CNC preq : queue statistics
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] descrPtr                 - pointer to the frame's descriptor.
* @param[in] clientInfoPtr            - pointer to CNC client info.
*
* @retval GT_OK                    - client is triggered
* @retval GT_FAIL                  - fail in client triggering
*/
static GT_STATUS snetSip6CncClientPreqQueueStatisticTrigger
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN SNET_CHT3_CNC_CLIENT_INFO_STC * clientInfoPtr
)
{
    return snetSip6CncClientPreqCommonStatisticTrigger(devObjPtr,descrPtr,clientInfoPtr);
}

/**
* @internal snetSip6CncClientPreqPortStatisticTrigger function
* @endinternal
*
* @brief   CNC preq : port statistics
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] descrPtr                 - pointer to the frame's descriptor.
* @param[in] clientInfoPtr            - pointer to CNC client info.
*
* @retval GT_OK                    - client is triggered
* @retval GT_FAIL                  - fail in client triggering
*/
static GT_STATUS snetSip6CncClientPreqPortStatisticTrigger
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN SNET_CHT3_CNC_CLIENT_INFO_STC * clientInfoPtr
)
{
    return snetSip6CncClientPreqCommonStatisticTrigger(devObjPtr,descrPtr,clientInfoPtr);
}


/**
* @internal snetPhoenixCncClientPclLookUpIngrTrigger_mux_pcl1_action3_and_pcl2_action3 function
* @endinternal
*
* @brief   Phoenix CNC mux  pcl1 action3 and pcl2 action3
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] descrPtr                 - pointer to the frame's descriptor.
* @param[in] clientInfoPtr            - pointer to CNC client info.
*
* @retval GT_OK                    - client is triggered
* @retval GT_FAIL                  - fail in client triggering
*/
static GT_STATUS snetPhoenixCncClientPclLookUpIngrTrigger_mux_pcl1_action3_and_pcl2_action3
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN SNET_CHT3_CNC_CLIENT_INFO_STC * clientInfoPtr
)
{
    GT_U32 regAddr; /* register address */
    GT_U32 fldValue;/* field value      */
    GT_U32 fldOffset; /* mux register field offset */

    DECLARE_FUNC_NAME(snetPhoenixCncClientPclLookUpIngrTrigger_mux_pcl1_action3_and_pcl2_action3);

    if(devObjPtr->deviceFamily == SKERNEL_HARRIER_FAMILY)
    {
        regAddr = SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.DFXServerUnitsBC2SpecificRegs.deviceCtrl29;
        fldOffset = 31;
    }
    else
    {
        regAddr = SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.DFXServerUnitsBC2SpecificRegs.deviceCtrl20;
        fldOffset = 4;
    }

    /* check if the DFX server configured to get requests from this client */
    smemDfxRegFldGet(devObjPtr, regAddr, fldOffset, 1, &fldValue);

    if(fldValue == 0 && clientInfoPtr->client == SNET_CNC_CLIENT_IPCL_LOOKUP_1_ACTION_3_E)
    {
        /* PCL1 action3*/
        __LOG(("PCL1 action3 is bound to CNC (muxed with PCL2 action3) \n"));
    }
    else
    if(fldValue == 1 && clientInfoPtr->client == SNET_CNC_CLIENT_IPCL_LOOKUP_1_ACTION_3_E)
    {
        /* mismatch ! not counting */
        __LOG(("WARNING : Phoenix limitation : PCL1 action3 not bound to CNC (muxed with PCL2 action3) (DFX configuration)) \n"));
        return GT_FAIL;
    }
    else
    if(fldValue == 1 && clientInfoPtr->client == SNET_CNC_CLIENT_IPCL_LOOKUP_2_ACTION_3_E)
    {
        /* PCL2 action3*/
        __LOG(("PCL2 action3 is bound to CNC (muxed with PCL1 action3) \n"));
    }
    else
    if(fldValue == 0 && clientInfoPtr->client == SNET_CNC_CLIENT_IPCL_LOOKUP_2_ACTION_3_E)
    {
        /* mismatch ! not counting */
        __LOG(("WARNING : Phoenix limitation : PCL2 action3 not bound to CNC (muxed with PCL1 action3) (DFX configuration) \n"));
        return GT_FAIL;
    }
    else /* wrong client bound to this function */
    {
        skernelFatalError("snetPhoenixCncClientPclLookUpIngrTrigger_mux_pcl1_action3_and_pcl2_action3 : wrong client bound to this function \n");
    }

    return snetCht3CncClientPclLookUpIngrTrigger(devObjPtr,descrPtr,clientInfoPtr);
}


/**
* @internal snetPhoenixCncClientPclLookUpIngrTrigger_mux_tti_action3_and_pcl0_action3 function
* @endinternal
*
* @brief   Phoenix CNC mux  tti action3 and pcl0 action3
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] descrPtr                 - pointer to the frame's descriptor.
* @param[in] clientInfoPtr            - pointer to CNC client info.
*
* @retval GT_OK                    - client is triggered
* @retval GT_FAIL                  - fail in client triggering
*/
static GT_STATUS snetPhoenixCncClientPclLookUpIngrTrigger_mux_tti_action3_and_pcl0_action3
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN SNET_CHT3_CNC_CLIENT_INFO_STC * clientInfoPtr
)
{
    GT_U32 regAddr; /* register address */
    GT_U32 fldValue;/* field value      */
    GT_U32 fldOffset; /* mux register field offset */

    DECLARE_FUNC_NAME(snetPhoenixCncClientPclLookUpIngrTrigger_mux_tti_action3_and_pcl0_action3);

    if(devObjPtr->deviceFamily == SKERNEL_HARRIER_FAMILY)
    {
        regAddr = SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.DFXServerUnitsBC2SpecificRegs.deviceCtrl29;
        fldOffset = 30;
    }
    else
    {
        regAddr = SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.DFXServerUnitsBC2SpecificRegs.deviceCtrl20;
        fldOffset = 5;
    }

    /* check if the DFX server configured to get requests from this client */
    smemDfxRegFldGet(devObjPtr, regAddr, fldOffset, 1, &fldValue);

    if(fldValue == 0 && clientInfoPtr->client == SNET_CNC_CLIENT_TTI_ACTION_3_E)
    {
        /* PCL1 action3*/
        __LOG(("TTI action3 is bound to CNC (muxed with PCL0 action3) \n"));
    }
    else
    if(fldValue == 1 && clientInfoPtr->client == SNET_CNC_CLIENT_TTI_ACTION_3_E)
    {
        /* mismatch ! not counting */
        __LOG(("WARNING : Phoenix limitation : TTI action3 not bound to CNC (muxed with PCL0 action3) (DFX configuration)) \n"));
        return GT_FAIL;
    }
    else
    if(fldValue == 1 && clientInfoPtr->client == SNET_CNC_CLIENT_IPCL_LOOKUP_0_ACTION_3_E)
    {
        /* PCL2 action3*/
        __LOG(("PCL0 action3 is bound to CNC (muxed with PCL1 TTI) \n"));
    }
    else
    if(fldValue == 0 && clientInfoPtr->client == SNET_CNC_CLIENT_IPCL_LOOKUP_0_ACTION_3_E)
    {
        /* mismatch ! not counting */
        __LOG(("WARNING : Phoenix limitation : PCL0 action3 not bound to CNC (muxed with TTI action3) (DFX configuration) \n"));
        return GT_FAIL;
    }
    else /* wrong client bound to this function */
    {
        skernelFatalError("snetPhoenixCncClientPclLookUpIngrTrigger_mux_tti_action3_and_pcl0_action3 : wrong client bound to this function \n");
    }

    return snetCht3CncClientPclLookUpIngrTrigger(devObjPtr,descrPtr,clientInfoPtr);
}

/**
* @internal snetSip6_10_cncClientPhaTrigger function
* @endinternal
*
* @brief   CNC PHA client to trigger counting of programmable events per flow
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] descrPtr                 - pointer to the frame's descriptor.
* @param[in] clientInfoPtr            - pointer to CNC client info.
*
* @retval GT_OK                    - client is triggered
* @retval GT_FAIL                  - fail in client triggering
*/
static GT_STATUS snetSip6_10_cncClientPhaTrigger
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN SNET_CHT3_CNC_CLIENT_INFO_STC * clientInfoPtr
)
{
    GT_U32 *regPtr;

    DECLARE_FUNC_NAME(snetSip6_10_cncClientPhaTrigger);

    /* Get the PHA CNC client enable status */
    regPtr = smemMemGet(devObjPtr, SMEM_SIP6_PHA_CTRL_REG(devObjPtr));

    /* Get counting enable status */
    clientInfoPtr->triggered = SMEM_U32_GET_FIELD(*regPtr, 1, 1);

    /* Check whether the counting is enabled */
    if((clientInfoPtr->triggered == 0) && (descrPtr->pha.pha_cncClientTrigger == 0))
    {
        __LOG(("[%s]Counting Globally Disabled \n",
            clientInfoPtr->clientsNamePtr));
        return GT_FAIL;
    }

    clientInfoPtr->byteCntMode = SMEM_U32_GET_FIELD(*regPtr, 3, 1);
    clientInfoPtr->index = descrPtr->flowId;

    if(descrPtr->pha.pha_threadId)
    {
        if(descrPtr->pha.pha_cncClientTrigger && !clientInfoPtr->triggered)
        {
            __LOG(("PPA changed PHA CNC client trigger to [%d] \n", descrPtr->pha.pha_cncClientTrigger));
            clientInfoPtr->triggered = descrPtr->pha.pha_cncClientTrigger;
        }

        if(descrPtr->pha.pha_cncClientIndexOverride)
        {
            __LOG(("PPA override PHA CNC client index to [%d] \n", descrPtr->pha.pha_cncClientIndexOverride));
            clientInfoPtr->index = descrPtr->pha.pha_cncClientIndexOverride ? (descrPtr->pha.pha_metadata[0] & 0xFFFF) : descrPtr->flowId;
        }
    }

    __LOG((" PHA CNC trigger status: %d \n", clientInfoPtr->triggered));
    __LOG((" PHA CNC Client byteCount mode: %d \n", clientInfoPtr->byteCntMode));
    __LOG((" PHA CNC counter index: %d \n", clientInfoPtr->index));

    return GT_OK;
}
