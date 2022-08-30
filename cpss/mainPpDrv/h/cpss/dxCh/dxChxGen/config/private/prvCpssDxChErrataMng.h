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
* @file prvCpssDxChErrataMng.h
*
* @brief DxCh manager for the Errata issues.
*
*
* @version   124
********************************************************************************
*/

#ifndef __prvCpssDxChErrataMngh
#define __prvCpssDxChErrataMngh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/cpssTypes.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortStat.h>
#include <cpss/generic/port/cpssPortTx.h>

/* macro for references purpose only -- must be 0 --> never 1 */
#define PRV_CPSS_DXCH_ERRATA_REFERENCE_CNS  0

/* number GT_U32 bmps needed to hold the known FEr errata bmp */
#define PRV_CPSS_DXCH_ERRATA_BMP_MAX_NUM_CNS \
                (1 + (((PRV_CPSS_DXCH_ERRATA_MAX_NUM_E)+1) >> 5))

/* get if the erratum need to be WA by CPSS
    devNum - the device id of the DxCh device
    FErNum -- id of the erratum
*/
#define PRV_CPSS_DXCH_ERRATA_GET_MAC(devNum,FErNum) \
  (((PRV_CPSS_DXCH_PP_MAC(devNum))->errata.FErArray[(FErNum)>>5] &(1<<((FErNum)&0x1f)))? \
      GT_TRUE : GT_FALSE)

/* set that the erratum need to be WA by CPSS
    devNum - the device id of the DxCh device
    FErNum -- 1 based number
*/
#define PRV_CPSS_DXCH_ERRATA_SET_MAC(devNum,FErNum) \
    (PRV_CPSS_DXCH_PP_MAC(devNum))->errata.FErArray[(FErNum)>>5] |= (1<<((FErNum)&0x1f))

/* set that the erratum NOT need to be WA by CPSS
    devNum - the device id of the DxCh device
    FErNum -- 1 based number
*/
#define PRV_CPSS_DXCH_ERRATA_CLEAR_MAC(devNum,FErNum) \
    (PRV_CPSS_DXCH_PP_MAC(devNum))->errata.FErArray[(FErNum)>>5] &= ~(1<<((FErNum)&0x1f))

/**
* @struct PRV_CPSS_DXCH_PROFILE_INFO_STC
 *
 * @brief A structure to hold all PP data needed profiles information for
 * IPM bridge copy get dropped when there is no member in the local
 * hemisphere vlan Errata (FE-7263524)
*/
typedef struct{

    /** @brief maximal number of buffers for a port
     *  portMaxDescrLimit- maximal number of descriptors for a port
     */
    GT_U32 portMaxBuffLimit;

    GT_U32 portMaxDescrLimit;

    CPSS_PORT_TX_Q_TAIL_DROP_PROF_TC_PARAMS_STC tailDropProfileParamsArray[CPSS_TC_RANGE_CNS];

} PRV_CPSS_DXCH_PROFILE_INFO_STC;

/**
* @struct PRV_CPSS_DXCH_BRG_L2_DLB_WA_LTT_USAGE_STC
 *
 * @brief LTT entry with secondary DLB region
*/
typedef struct PRV_CPSS_DXCH_BRG_L2_DLB_WA_LTT_USAGE_STC{
    /** @brief LTT index */
    GT_U32  lttIndex;

    /** @brief base index in DLB table secondary region, for the LTT entry */
    GT_U32 secondaryDlbBaseIndex;

    /** @brief pointer to the next LTT entry with secondary region assigned */
    struct PRV_CPSS_DXCH_BRG_L2_DLB_WA_LTT_USAGE_STC *nextPointer;
}PRV_CPSS_DXCH_BRG_L2_DLB_WA_LTT_USAGE_STC;

/**
* @enum PRV_CPSS_DXCH_ERRATA_ENT
 *
 * @brief enumerator for the numbers of Erratum that handles in the CPSS
 * code
*/
typedef enum{

    /** @brief Disabling the Egress Rate Shaper under traffic may hang its
     *  relevant transmit queue.
     *  To disable the Egress Rate Shaper follow the following procedure:
     *  1. Set MaxBucketSize to 0xFFF, Tokens to 0xFFF, SlowRateEn to 0.
     *  2. Set TokenBucketEn to 0.
     *  (FEr#47)
     */
    PRV_CPSS_DXCH_TOKEN_BUCKET_RATE_SHAPING_DISABLE_WA_E,

    /** @brief in some cases when performing scanning operations and the AUQ is
     *  full the learning operation is also stalled.
     *  The reason is:
     *  the learning and scanning machines share an access arbiter.
     *  In order to insure data coherency in the FDB only a one of
     *  the clients is allowed to access the FDB at any
     *  given time. An access being with a read and ends
     *  with a write and only then is the arbitration
     *  changed. In the scenario you saw, since the
     *  scanning machine is waiting for the AUQ to
     *  become available, it does not release the access
     *  arbiter. The learning machine in this case is
     *  stalled as well and is waiting for the scanning
     *  machine to release the access arbiter.
     *  (FEr#3119)
     */
    PRV_CPSS_DXCH_FER_FDB_AUQ_LEARNING_AND_SCANING_DEADLOCK_WA_E,

    /** @brief :
     *  The following packet layer 4 sanity checks are enabled by default:
     *  ARP SA Mismatch Drop Enable - Enables dropping or ARP packets
     *  with MAC SA not equal to the ARP and payload
     *  data containing a field called <source hardware address>.
     *  TCP SYN With Data Drop Enable - Enable dropping of TCP SYN packets with data.
     *  TCP Over MC Drop Enable - Enables dropping of TCP packets over Multicast/Broadcast MAC DA.
     *  Fragmented ICPMDropEn - Enables dropping of fragmented ICMP packets.
     *  TCPFlagesZeroDropEn - Enables dropping of TCP packets with all flags set to zero.
     *  FINWithURGWithPSH DropEn - Enables dropping of TCP packets with FIN, URG, and PSH flags set.
     *  SYNWithFINDropEn - Enables dropping of TCP packets with both SYN and FIN flags set.
     *  TCPSYNWithRSTDropEn - Enables dropping of TCP packets with both SYN and RST flags set.
     *  TCPUDPZeroPortDropEn - Enables dropping of TCP or UPD packets with source or
     *  destination port equal
     *  (RM#3018)
     */
    PRV_CPSS_DXCH2_RM_BRG_LAYER_4_SANITY_CHECKS_WA_E,

    /** @brief Direct Access write does not work under traffic for VLT memory
     *  (VLAN table, Multicast Groups table, Span State Groups table,
     *  VRF Table).
     *  Direct write to VLT memory tables work in the following way:
     *  1. for each write/read access the entire table entry is
     *  written/read by device from the memory and placed in a special
     *  temp register.
     *  2. When new access operation is starting, the temp register
     *  still contains data from the previous operation.
     *  3. Write operation is done word-by-word through updating words
     *  of the temp register, but the entire register is written to the
     *  table as many times as there are words in table entry.
     *  Assume table entry length is 6 words, first the word 0 of the
     *  temp register is updated with new data and the entire register
     *  (6 words) is written to the table, then word 1 is updated,
     *  and again the entire 6 words are written, now word 0 and word 1
     *  are correct, words 2-5 contain junk from previous operation,
     *  but still are written to the table. After 6 such operations
     *  the table entry content is correct, but during these operations
     *  there was a temporary inconsistency of table content.
     *  If done under the traffic, this may lead to unpredictable
     *  behavior of the system.
     *  Among several VLT tables, two have multi word entries -
     *  VLAN and STG. In order to avoid the problem described above,
     *  CPSS reads the content of updated entry, this way setting the
     *  temp register with consistent content, and then performing write
     *  to the table operation.
     */
    PRV_CPSS_DXCH3_VLT_TABLES_WRITE_DIRECT_ACCESS_WA_E,

    /** @brief Buffer Management GigE Ports Group Limits Configuration Register
     *  (Offset: 0x03000004) and
     *  Buffer Management HyperG.Stack Ports Group Limits Configuration
     *  Register (Offset: 0x03000008)
     *  default values are incorrect
     *  (RM#2003)
     */
    PRV_CPSS_DXCH3_RM_BM_GE_XG_PORT_GROUP_LIMIT_CFG_REG_WA_E,

    /** @brief LMS GigE Ports Mode Register1 (Offset: 0x05804144)
     *  default values are incorrect.
     *  (RM#2007)
     */
    PRV_CPSS_DXCH3_RM_GE_PORTS_MODE1_REG_WA_E,

    /** @brief LMS HyperG.Stack Ports Mode Register (Offset: 0x84804144)
     *  default value is incorrect.
     *  (RM#2008)
     */
    PRV_CPSS_DXCH3_RM_XG_PORTS_MODE_REG_WA_E,

    /** @brief LMS0/1 Group1LED Interface0 Control Register 1
     *  and Class6 Manipulation Register default values are incorrect.
     *  (RM#2024)
     */
    PRV_CPSS_DXCH3_RM_LMS0_1_GROUP1_LED_INTERFACE_WA_E,

    /** @brief The direct access to PreEgress Engine (EQ) tables for both
     *  reads and writes, under traffic is not functional. Pre-Egress
     *  Engine (EQ) tables include; CPU Code Table, Ingress STC Table,
     *  QoSProfile to QoS Table, Statistical Rate Limits Table, TO CPU
     *  Packet Rate Limiters, and Trunks Members Table.
     *  WA: The Pre-Egress Engine (EQ) tables address space must be
     *  accessed for read or for write by using indirect access only.
     *  (FEr#2028)
     */
    PRV_CPSS_DXCH3_DIRECT_ACCESS_TO_EQ_ADDRESS_SPACE_WA_E,

    /** @brief TxQ Parity calculation is wrong for Descriptors & FLL memories.
     *  The TxQ calculates parity on the FLL RAM & Desciptor RAM,
     *  but calculates it on a different number of bits used when
     *  writing the data to the RAMs.
     *  This causes parity error on almost every packet reaching the TxQ.
     *  WA: interrupts ParityErrorIn TxqPtrMem FllRamParity and
     *  ParityErrorIn TxqDescMem in the Transmit Queue General Interrupt
     *  Cause register must be masked
     *  (FEr#204)
     */
    PRV_CPSS_DXCH3_TXQ_PARITY_CALCULATION_E,

    /** @brief SDMA resource errors may cause PCI Express packets reception
     *  malfunction.
     *  When using the PCI Express host management interface
     *  for forwarding packets to/from the host CPU, the
     *  CPU receives packets from the device directly to
     *  a pre-allocated space in the host memory. To
     *  receive packets for a given CPU traffic class
     *  queue, the CPU prepares a linked list of
     *  descriptors located in the host memory. A
     *  resource error event occurs when it reaches the
     *  end of available descriptors in the linked list.
     *  This resource error occurs if either: The
     *  current descriptor for the Rx SDMA has a NULL
     *  next-descriptor pointer. OR The SDMA has reached
     *  a descriptor that is owned by the CPU (i.e., a
     *  cyclic descriptor list is implemented). This may
     *  happen at the start of a packet or in the middle
     *  of writing a packet to host memory (if it
     *  occupies more than a single descriptor buffer).
     *  This may also happen due to a speculative
     *  pre-fetch of a descriptor (i.e, without any
     *  current packet destined to the given queue). The
     *  device terminates packet writing to the host
     *  memory if it reaches the end of the linked list
     *  before completion of packet reception.Functional
     *  Impact SDMA resource errors may cause a PCI
     *  Express packets reception malfunction. WA:
     *  Configure the SDMA to drop packets upon resource
     *  errors, by setting to 1 the Retry AbortTC<
     *  traffic class queue> fields in the SDMA
     *  Configuration Register
     *  (FEr#2009)
     */
    PRV_CPSS_DXCH3_SDMA_WA_E,

    /** @brief When the CPU reads the POLICY TCAM or ROUTER TCAM
     *  via direct address space the ECC error exception signaled.
     *  WA is to mask the interrupt for the CPU read time and to restore
     *  (unmask) it for all other time.
     *  (FEr#2033)
     *  ECC check fails on reading part of PCL/Router TCAM row
     *  The PCL/Router TCAM is used by the PCL/Router engine for IPv4/6
     *  Unicast/Multicast lookups, and is also used by the tunnel
     *  termination/interface lookup.
     *  The PCL/Router TCAM is organized as rows, where each row contains
     *  four arrays of data; Array0, Array1, Array2, and Array3.
     *  PCL/Router TCAM on-chip table memory has built-in data integrity
     *  by Error Correction Code (ECC).
     *  However, due to this erratum, data returns on indirect read to
     *  part of a row (not all 4 arrays) or direct read containing
     *  incorrect ECC cause the ECC check to fail.
     *  WA : To indirectly read PCL/Router TCAM data part of a row
     *  (not all 4 arrays), or to read directly, perform the following:
     *  1. Mask the ECC interrupt.
     *  2. Read PCL/Router TCAM data.
     *  3. Read the Cause register and clear the interrupt.
     *  4. Unmask the ECC interrupt.
     *  (FEr#2050)
     *  -- NOTE: must fill info in
     *  PRV_CPSS_DXCH_ERRATA_STC::info_PRV_CPSS_DXCH3_CPU_READ_TCAM_ECC_INTERRUPT_WA_E
     *  regarding the interrupt register and index , that a device
     *  family that needs this WA should set according to it's specific
     *  values.
     */
    PRV_CPSS_DXCH3_CPU_READ_TCAM_ECC_INTERRUPT_WA_E,

    /** @brief An Interrupt Cause register and an Interrupt Mask register are
     *  defined for each functional block. However, due to this erratum,
     *  - A write to the Transmit Queue Flush Interrupt Mask register
     *  (offset: 0x01800114), also causes a write to the Transmit
     *  Queue Full Interrupt Mask register (offset: 0x018001EC).
     *  - A write to the Transmit Queue Full Interrupt Mask register
     *  (offset: 0x018001EC) does not function.
     *  WA: disable get of TXQ Full interrupts in the Transmit Queue
     *  Interrupt Summary Mask register - 0x01800104
     *  (FEr#1098)
     */
    PRV_CPSS_DXCH3_TXQ_FULL_INTERRUPT_NOT_FUNCTION_WA_E,

    /** @brief Policy and Router TCAM Tables indirect read is not functional.
     *  Direct access and indirect access to tables are supported by the device.
     *  However, due to this erratum, the indirect read from the
     *  Policy and Router TCAM tables address space is not supported.
     *  Functional Impact :
     *  The indirect read from the Policy and Router TCAM tables address
     *  space is not functional.
     *  WA : The Policy and Router TCAM tables address space must be
     *  accessed for read by using the direct access only.
     *  (FEr#2004)
     */
    PRV_CPSS_DXCH3_POLICY_AND_ROUTER_TCAM_TABLES_INDIRECT_READ_WA_E,

    /** @brief Wrong default in FDB Global Configuration register
     *  (RM# 30160)
     */
    PRV_CPSS_DXCH_RM_FDB_GLOBAL_CONF_REG_WA_E,

    /** @brief for CH3 :
     *  The device supports IP Longest Prefix Match based on the Policy TCAM
     *  and policy-based routing based on the Router TCAM.
     *  However, due to this erratum, the ingress policy action can assign a
     *  router Lookup Translation Table (LTT) pointer only in the range of first column.
     *  TCAM based routing is operational at full range.
     *  Functional Impact :
     *  The ingress policy action can assign a router Lookup Translation Table (LTT)
     *  pointer only in the range of first column.
     *  WA : None.
     *  (FEr#2018)
     *  for XCAT :
     *  Policy base routing LTT index can be written only to array0 of the LTT line.
     *  The Lookup Translation Table (LTT) is build from 4 entries in a line.
     *  When using the LTT for policy based routing, the index that received from the
     *  PCL/TTI used by the LTT as a line number, when the array number if always 0.
     *  Functional Impact :
     *  Thus, this feature uses only 25% of the capability of the LTT table.
     *  WA : When the application enter LTT index X in the PCL/TTI action, the actual
     *  value that will be written is divided by 4.
     */
    PRV_CPSS_DXCH3_LIMITED_NUMBER_OF_POLICY_BASED_ROUTES_WA_E,

    /** @brief Under heavy traffic stress, FDB accesses from the CPU can get stuck.
     *  Set the default value of bit[6] in the FDB Global Configuration
     *  Register (offset: 0x06000000) to 0x1 (giving strict priority to
     *  management accesses over the data-path).
     *  (RM#3014)
     */
    PRV_CPSS_DXCH3_RM_CPU_ACCESS_TO_FDB_UNDER_TRAFFIC_WA_E,

    /** @brief Wrong default value in Total Buffer Limit Configuration in TxQ.
     *  (RM#3008)
     */
    PRV_CPSS_DXCH_XCAT_RM_TOTAL_BUFFER_LIMIT_CONF_IN_TXQ_E,

    /** @brief Wrong default values in Counting Mode Configuration of IPCL and
     *  in TTI Unit Global Configuration of TTI.
     *  The "Enable Counting" should be set by default.
     *  (part of RM#3012)
     */
    PRV_CPSS_DXCH_XCAT_RM_CNC_ENABLE_COUNTING_E,

    /** @brief Wrong default value of bit#28 of Transmit Queue Extended Control
     *  register. The bit should be set by default.
     *  (RM#3009)
     */
    PRV_CPSS_DXCH_XCAT_RM_TXQ_EXT_CNTR_REG_WA_E,

    /** @brief Wrong default values in Mirror Interface Parameter Register<%i>
     *  bit #20 should be with value 1
     *  (part of RM#3012)
     */
    PRV_CPSS_DXCH_XCAT_RM_MIRROR_INTERFACE_PARAMETER_REGISTER_I_WA_E,

    /** @brief Wrong default values in Policy Engine Configuration
     *  bit #12 should be with value 1
     *  (part of RM#3012)
     */
    PRV_CPSS_DXCH_XCAT_RM_POLICY_ENGINE_CONFIGURATION_REGISTER_WA_E,

    /** @brief Wrong default values in Dequeue SWRR Weight Register 3
     *  bit #0..8 : set to value 0x4b ,
     *  because fields of XG and Gig1 weights are swapped
     *  (RM#3007)
     */
    PRV_CPSS_DXCH_XCAT_RM_TXQ_DEQUEUE_SWRR_WEIGHT_REGISTER_3_WA_E,

    /** @brief Wrong default values in IPLR0,1 > Policer Control0
     *  bit #21 : set to 1
     *  (part of RM#3012)
     */
    PRV_CPSS_DXCH_XCAT_RM_INGRESS_POLICER_CONTROL0_WA_E,

    /** @brief Wrong default values in IPLR0,1 > Policer Control1
     *  bit #0 : set to 1
     *  (part of RM#3012)
     */
    PRV_CPSS_DXCH_XCAT_RM_INGRESS_POLICER_CONTROL1_WA_E,

    /** @brief Wrong default values in IPLR0,1,EPLR > Policer Control2
     *  bit #0 : set to 1
     *  (part of RM#3012)
     */
    PRV_CPSS_DXCH_XCAT_RM_POLICER_CONTROL2_WA_E,

    /** @brief Wrong default values in Egress Policer Global Configuration
     *  bit #0 : set to 1
     *  (part of RM#3012)
     */
    PRV_CPSS_DXCH_XCAT_RM_EGRESS_POLICER_GLOBAL_CONFIGURATION_WA_E,

    /** @brief Wrong default values in BCN control
     *  bit #1 : set to 0
     *  bit #3 : set to 1
     *  (part of RM#3012)
     */
    PRV_CPSS_DXCH_XCAT_RM_BCN_CONTROL_WA_E,

    /** @brief Wrong default values in Ports BCN Awareness Table
     *  bits #0..27 : set to 0x0FFFFFFF
     *  (part of RM#3012)
     */
    PRV_CPSS_DXCH_XCAT_RM_PORTS_BCN_AWARENESS_TABLE_WA_E,

    /** @brief Wrong default values in TTI Unit Global Configuration
     *  bit #3 : set to 1
     *  bit #19 : set to 0
     *  (part of RM#3012)
     */
    PRV_CPSS_DXCH_XCAT_RM_TTI_UNIT_GLOBAL_CONFIGURATION_WA_E,

    /** @brief bit #8 in the 'per port' 'PORT VLAN QOS configuration'
     *  ('Extended DSA Bypass Bridg') : set to 1
     *  (RM#3005)
     */
    PRV_CPSS_DXCH_XCAT_RM_EXTENDED_DSA_BYPASS_BRIDGE_WA_E,

    /** @brief Wrong default values in Port Serial Parameters Configuration
     *  bit #1 : set to 0
     *  (RM#3004)
     */
    PRV_CPSS_DXCH_XCAT_RM_PORT_SERIAL_PARAMETERS_CONFIGURATION_WA_E,

    /** @brief Wrong default values in LMS0 Misc Configurations , LMS1 Misc
     *  Configurations.
     *  bit #15..16 : set to 2
     *  bit #20 : set to 0
     *  (RM#3002,RM#3003)
     */
    PRV_CPSS_DXCH_XCAT_RM_LMS0_LMS1_MISC_CONFIGURATIONS_WA_E,

    /** @brief Traffic from the CPU via SDMA mechanism can corrupt the switch
     *  internal buffer memory.
     *  Solution: Traffic sent from the CPU is padded with 8 bytes of
     *  zeros for byte counts of <byteblock>n+k (1<=k<=8) (including
     *  DSA tag), where n and k are integers and <byteblock> is 256 for
     *  xCat and 512 for Lion,
     *  (FEr#3048)
     */
    PRV_CPSS_DXCH_XCAT_TX_CPU_CORRUPT_BUFFER_WA_E,

    /** @brief when PP sending a packet to CPU from port that it is a member of
     *  trunk, the <source port> field should hold the source port (not
     *  the trunkId) , in DSA Marvell TAG header.
     *  but due to the erratum this field includes the <trunk ID> / 2.
     *  (FEr#3033)
     */
    PRV_CPSS_DXCH_XCAT_TRUNK_WRONG_SOURCE_PORT_TO_CPU_WA_E,

    /** @brief Reading of IEEE Reserved Multicast Configuration registers
     *  should be performed from:
     *  0x200X820 instead of 0x200X810 (table 2)
     *  0x200X828 instead of 0x200X818 (table 3)
     *  (FEr#3060)
     */
    PRV_CPSS_DXCH_XCAT_IEEE_RESERVED_MC_CONFG_REG_READ_WA_E,

    /** @brief Enable CPU lacks source port information on traffic
     *  received from trunk workaround:
     *  Set bit #17 in register 0x0C0002B0
     *  (RM#3017)
     */
    PRV_CPSS_DXCH_XCAT_RM_TRUNK_WRONG_SOURCE_PORT_TO_CPU_WA_E,

    /** @brief FEr#3054: Gig port PRBS is not functional.
     *  Initialization PRBS parameters of Giga MAC for ports 24-27.
     *  Bits 15:14 of registers
     *  0x0A806014, 0x0A806414, 0x0A806814, 0x0A806C14
     *  set to 0x03
     */
    PRV_CPSS_DXCH_XCAT_GIGA_PORT_PRBS_PARAM_INIT_WA_E,

    /** @brief Enable CPU checking addresses and filtering the restricted ones.
     *  lastValidAddress - last valid address
     *  (FEr#2739)
     */
    PRV_CPSS_DXCH_XCAT_RESTRICTED_ADDRESS_FILTERING_WA_E,

    /** @brief Wrong default values in Header Alteration Configuration
     *  bit #10 should be with value 0
     *  (RM#3024)
     */
    PRV_CPSS_DXCH_XCAT_RM_HEADER_ALTERATION_GLOBAL_CONFIGURATION_REGISTER_WA_E,

    /** @brief TOD counter must be updated in resolution of 20ns.
     *  The device maintains a free-running Time Of Day (TOD) counter
     *  for IP Flow Information Export (IPFIX) and Precision Time
     *  Protocol (PTP) time-stamping. The TOD counters are incremented
     *  20 ns on every rise and fall of the clock.
     *  (FEr#2773)
     */
    PRV_CPSS_DXCH_XCAT_TOD_NANO_SEC_SET_WA_E,

    /** Enable HA fix tunneling support for EthoverIPv4. */
    PRV_CPSS_DXCH_XCAT_ETH_OVER_IPV4_GRE_WA_E,

    /** @brief The passenger Ethernet packet may optionally retain its original CRC endtoend,
     *  or its CRC may be removed prior to being transported across the tunnel and
     *  regenerated prior to sending the packet to the remote customer attachment circuit.
     *  However, due to this erratum, the original passenger packet CRC is removed and not
     *  retained even if set to "Retain CRC".
     *  (FEr#2785)
     */
    PRV_CPSS_DXCH_XCAT_PASSENGER_ETH_PACKET_CRC_CANT_BE_RETAINED_WA_E,

    /** Enable Short Preamble fix */
    PRV_CPSS_DXCH_XCAT_RM_MAC_CTRL_REG3_PCS_PREAMBLE_WA_E,

    /** Enable Split Network and Stack ports configuration (DP clock select) fix */
    PRV_CPSS_DXCH_XCAT_RM_METAL_FIX_SGMII25G_DPCLK_SEL_WA_E,

    /** Enable CCFC frames sent to internal cascade port fix */
    PRV_CPSS_DXCH_XCAT_RM_BCN_CONTROL_CCFC_SRC_PORT_WA_E,

    /** @brief in GE port interface mode the disable bit does not affect the transmit path.
     *  (FEr#2794)
     */
    PRV_CPSS_DXCH_XCAT_GE_PORT_DISABLE_WA_E,

    /** @brief Ethernet packets with MAC DA 0180C2000001 are terminated in ingress.
     *  (FEr#3102)
     */
    PRV_CPSS_DXCH_XCAT_FC_FORWARD_NOT_FUNCTIONAL_WA_E,

    /** @brief in the Pad_Control_1 register(offset: 0x48), configure the following fields:
     *  ssmii_pad_znr: bits[19:17] to 0x3.
     *  ssmii_pad_zpr: bits[16:14] to 0x3.
     *  (RM#3042)
     */
    PRV_CPSS_DXCH_XCAT_RM_SSMII_PAD_ZNR_ZPR_WA_E,

    /** @brief The registers HA/Passenger TPID0_TPID1<%n> have default value 0x0 instead of 0x81008100.
     *  (RM#3043)
     */
    PRV_CPSS_DXCH_XCAT_RM_PASSENGER_TPID_REG_WA_E,

    /** @brief The default value of the <SelPortSDMA> field in the Global
     *  Control register (offset 0x58 in each pipe) is 0x0.
     *  This value selects the CPU port rather than the SDMA as the
     *  interface for packet to/from CPU. To activate the SDMA as the
     *  interface to/from CPU, the first required modification prior to
     *  any access to SDMA registers is to set this bit to 0x1.
     *  If an access to any of the SDMA registers is performed prior to
     *  setting the <SelPortSDMA> field, then the PCI Express interface
     *  will hang.
     *  WA : Per management control interface, write to the following
     *  registers: Change bit 20 of 0x58 to 0x1
     *  (RM#2701)
     */
    PRV_CPSS_DXCH_LION_RM_SDMA_ACTIVATION_WA_E,

    /** @brief Indirect access to TCAM (Router and PCL) is not supported
     *  (FEr#2744 --> include FEr#2004 as part of it)
     */
    PRV_CPSS_DXCH_LION_INDIRECT_ACCESS_TO_TCAM_IS_NOT_SUPPORTED_WA_E,

    /** @brief Device default is that all SERDES are selected as sources for
     *  Synchronous Ethernet recovery clock while by default none
     *  should be.
     *  (RM#2705)
     */
    PRV_CPSS_DXCH_LION_RM_SYNC_ETHERNET_NONE_SELECT_WA_E,

    /** @brief Fix value of TCAM after read delay
     *  The default values of the following registers are incorrect:
     *  Policy TCAM Configuration 0 Register Offset: 0x0D000208 Lion
     *  Policy TCAM Control Register Offset: 0x0D000148 Lion
     *  Policy TCAM Configuration 2 Register Offset: 0x0D000220 Lion
     *  Router TCAM Configuration 0 Register Offset: 0x0D80096C Lion
     *  Router TCAM Control Register Offset: 0x0D800960 Lion
     *  Router TCAM Configuration 2 Register Offset: 0x0D800978 Lion
     *  (RM#3013)
     */
    PRV_CPSS_DXCH_LION_RM_TCAM_CONFIG_WA_E,

    /** Token Buckets are not supported */
    PRV_CPSS_DXCH_LION_TB_NOT_SUPPORTED_WA_E,

    /** @brief In Lion the direct access to Policer Counting memory is not
     *  working. The indirect access should be used in order to update
     *  the entry and read it.
     *  (FEr#2771)
     */
    PRV_CPSS_DXCH_LION_POLICER_COUNTING_MEM_ACCESS_WA_E,

    /** @brief The multiport group architecture can be utilized to extend the
     *  Bridge FDB lookup across the device port groups.
     *  However, when the multi-port group FDB lookup is used, the device
     *  default settings of Auto-learning enabled, or Age mode set to
     *  Age-with-delete, can cause mis-synchronization of the different
     *  port groups FDB tables.
     *  (RM#2702)
     */
    PRV_CPSS_DXCH_LION_RM_MULTI_PORT_GROUP_FDB_LIMIT_WA_E,

    /** @brief By default, the <OrigSrcPortFilterEn> bit[3] in the Egress
     *  Filters Global Enables register (offset: 0x01810000) is 0x0 (disable).
     *  This value does not prevent filtering multi-destination packets
     *  from being sent back to the network port where it was initially
     *  received.
     *  When the multi-port group architecture is utilized to extend the
     *  Bridge FDB lookup across the device port groups, set the <OrigSrcPortFilterEn>
     *  to 0x1 (enable)
     *  (RM#2706)
     */
    PRV_CPSS_DXCH_LION_RM_ORIG_SRC_PORT_FILTERING_WA_E,

    /** @brief Ports 15, 31, and 47 cannot be members of the Multicast group 0xFFF.
     *  A packet forwarded to a Multicast port group is assigned an index
     *  called "VIDX". At initialization, the VIDX Multicast table is
     *  cleared to all zeros, with the exception of an entry for VIDX
     *  0xFFF, which is initialized to have all ports as members.
     *  This enables Multicast packets to be flooded to all the VLAN ports.
     *  However, global ports 15, 31, and 47 cannot be members of the
     *  Multicast group 0xFFF.
     *  Exclude ports 15, 31, and 47 from the Multicast group 0xFFF membership
     *  in the Multicast Groups table, (offset: 0x1186FFF0).
     *  (RM#2707)
     */
    PRV_CPSS_DXCH_LION_RM_MULTI_PORT_GROUP_CPU_IN_VIDX_WA_E,

    /** @brief The CnDropEn field, bit[5] in the CN Global Configuration register,
     *  (offset: 0x0A0B0000) enables dropping CN packets
     *  when any of the following clients is not ready:
     *  - Inc interface
     *  - Desc dup interface
     *  - Writes back FIFO full
     *  Set the CnDropEn field, bit[5] in the CN Global Configuration
     *  register, (offset: 0x0A0B0000) to 0x1 (enable).
     *  (FEr#2768)
     */
    PRV_CPSS_DXCH_LION_CN_GLOBAL_REG_WA_E,

    /** @brief need to Calibrate sensor on init in every power up
     *  (all below registers are in PortGroup0):
     *  1) Change Temperature sensor average num from 2 to 8 -
     *  Register 0x118F0D88 Bits[21:19] = 0x3
     *  2) Change Temperature sensor RefCalCount from 0x113 to 0xf1 -
     *  Register 0x118F0D8C Bits[17:9] = 0xF1
     *  3) Start init -
     *  Register 0x118F0D8C Bit[31] = 0x0
     *  4) Start init -
     *  Register 0x118F0D8C Bit[31] = 0x1
     */
    PRV_CPSS_DXCH_LION_TEMPERATURE_SENSOR_INITIALIZATION_WA_E,

    /** @brief XLG port frequency configuration
     *  The device XLG port frequency is driven from an internal PLL.
     *  The PLL frequency is locked according to a sampled at reset value
     *  from external pins. However, due to this erratum, the available
     *  values of the external pins are not correct and a software
     *  configuration is needed.
     *  (HWE#2706)
     */
    PRV_CPSS_DXCH_LION_RM_SD_PLL_REG_WA_E,

    /** @brief Slow rate is not functional on port TB
     *  (FEr#2780)
     */
    PRV_CPSS_DXCH_LION_SLOW_RATE_ON_PORT_TB_NOT_SUPPORTED_WA_E,

    /** @brief With default configurations, the UC are not served at WS in presence of
     *  MC, therfore give UC Strict Priority over MC.
     *  Set bit 2 in the MC FIFO Global Configurations Register (0x01801000).
     *  (RM#2710)
     */
    PRV_CPSS_DXCH_LION_RM_UC_STRICT_OVER_MC_WA_E,

    /** @brief The TxDMA isn't always ready to receive descriptor from the TxQ when
     *  instantaneous burstiness of data occurs.
     *  The workaround consists of 2 steps:
     *  1. Enabling the Deficit mode (handles higher burstiness of data on the
     *  ports).
     *  2. Set <Token Bucket Base Line> value to at least eight times the MTU.
     *  (RM#2709)
     */
    PRV_CPSS_DXCH_LION_RM_SCHEDULER_DEFICIT_MODE_WA_E,

    /** @brief WS is not achieved in "one to many" traffic at 316 Bytes and below.
     *  Set the number of MC descriptors duplications done per UC descriptor
     *  to 4:1.
     *  Clear bits 1:0 in the MC FIFO Global Configurations Register (0x01801000).
     *  (RM#2711)
     */
    PRV_CPSS_DXCH_LION_RM_TX_MC_IN_CHUNKS_WA_E,

    /** @brief Wrong total number of buffers limit in egress queues.
     *  Set bits 13:0 in the Global Buffer Limit Register (0x0A0A0310) to 0x1e70.
     *  (RM#2712)
     */
    PRV_CPSS_DXCH_LION_RM_BUFF_LIMIT_EGRESS_QUEUE_WA_E,

    /** @brief XLG port may not support wire speed in all cases.
     *  (FEr#2709)
     */
    PRV_CPSS_DXCH_LION_XLG_WS_SUPPORT_WA_E,

    /** @brief QCN frame format is partially compliant with IEEE 802.1 Qau.
     *  Set bit 0 in the HA Misc Configuration Register (0xE800710).
     *  (FEr#2703)
     */
    PRV_CPSS_DXCH_LION_QCN_FORMAT_COMPLIANT_WA_E,

    /** @brief The response to the XOFF PFC frame is inaccurate.
     *  Set bit 19 in the TTI Misc Confuguration Register (0x1000200).
     *  (RM#2788)
     */
    PRV_CPSS_DXCH_LION_INACCURATE_PFC_XOFF_WA_E,

    /** @brief CPU Mailbox to Neighbor CPU device source information is not
     *  complete - wrong source port assigment.
     *  (FEr#2776)
     */
    PRV_CPSS_DXCH_LION_INCOMPLETE_CPU_MAILBOX_INFO_WA_E,

    /** @brief The Router engine supports multiple sets of Router management counters.
     *  Any of the four counter-sets can be bound to an interface defined by the 3-tuple:
     *  (device+port or Trunk, VLAN, packet type: IPv4 or IPv6).
     *  However, due to this erratum, in some scenarios, the per (device+port)
     *  may incorrectly count packets from a wrong port
     *  (FEr#3116)
     */
    PRV_CPSS_DXCH_LION_WRONG_COUNTING_OF_IP_COUNTERS_PER_PORT_WA_E,

    /** @brief default DSA tag content
     *  for PFC packets is not correct.(Lion2:RM1752573, Design BTS#:435(DIP))
     */
    PRV_CPSS_DXCH_LION_RM_PFC_DSA_TAG_E,

    /** @brief The default value of Hierarchical Policer memory control should be 0x5:
     *  Policer 0 - 28 entries, Policer 1 - 256 (172+84) entries.
     *  (RM#3029)
     */
    PRV_CPSS_DXCH_XCAT2_RM_POLICER_MEM_CONTROL_WA_E,

    /** @brief Ports 2427 shaping rate defaults are different than that of the ports
     *  0-23.
     *  To enable the ports 24-27 to support shaping rates identical to ports
     *  0-23, set the <XgPortsTbAsGig> field, bit[31] in the Token Bucket
     *  Update Rate and MC FIFO Configuration Register (0x0180000C) to
     *  "Gig mode" (0x1).
     *  (RM#3030)
     */
    PRV_CPSS_DXCH_XCAT2_RM_INCORRECT_XG_SHAPER_TOKEN_BUCKET_WA_E,

    /** @brief Packets that bypass the Ingress Pipe perform a refresh of the age bit
     *  in the FDB.
     *  Set the value of the <Avoid_Age_Refresh> field, bit[22] in the Bridge
     *  Global Configuration2, to 0x1 (enable), so that packets that bypass the
     *  Ingress Pipe will not perform refresh of the age bit in the FDB.
     *  (RM#3031)
     */
    PRV_CPSS_DXCH_XCAT2_RM_BYPASS_PACKET_REFRESH_FDB_AGE_WA_E,

    /** @brief Wrong default in Pipe Select register and some new DFX settings.
     *  (RM#3032)
     */
    PRV_CPSS_DXCH_XCAT2_RM_PIPE_SELECT_WA_E,

    /** @brief the automatic
     *  assignment of the time windows to the FlexLink ports as a function of their
     *  operating speed is not functional and when rate limit is enabled on the
     *  FlexLink ports causes all packets to be dropped.
     *  (RM#3038)
     */
    PRV_CPSS_DXCH_XCAT2_RM_FLEX_LINK_PORTS_RATE_LIMIT_WA_E,

    /** @brief Wrong
     *  noninitial UDP fragments packets trap to CPU. Due to
     *  this erratum, the device checks non-initial UDP fragments
     *  and wrongly assumes a UDP header exists after the IPv4
     *  header. If the packet data contains the value 0x13F at
     *  the same offset as the UDP header (2-byte offset from
     *  the start of data), the device traps the packet to the
     *  CPU.(FEr#3112)
     */
    PRV_CPSS_DXCH_XCAT2_PTP_UDP_FRAGMENT_WA_E,

    /** @brief traffic using fiber transceivers directly
     *  connected to FlexLink uplink ports. (HWEr#3068)
     */
    PRV_CPSS_DXCH_XCAT2_100FX_WA_E,

    /** @brief In fast failover the secondary port must be in the same TXQ unit
     *  as the primary port.
     */
    PRV_CPSS_DXCH_LION2_FAST_FAILOVER_WA_E,

    /** Dual Device Id field should be enable by default. */
    PRV_CPSS_DXCH_LION2_RM_TTI_DUAL_DEV_ENABLE_WA_E,

    /** @brief HGL mac interrupts recieved when HGL mac is no
     *  used.
     */
    PRV_CPSS_DXCH_LION2_HGL_MAC_INT_WA_E,

    /** @brief PTP
     *  fragment packets may be incorrectly identified as
     *  invalid. PTP fragment packets are forwarded normally to
     *  their destination port. However, due to this erratum, IP
     *  fragment packets may be incorrectly identified as PTP
     *  fragment packets by the TTI block and trapped to the
     *  CPU. (FEr#3124)
     */
    PRV_CPSS_DXCH_XCAT2_PTP_FRAGMENT_PACKET_TRAP_WA_E,

    /** @brief The default value of the pfc global trigger
     *  register<pfcEn> should be set to 0x1.
     */
    PRV_CPSS_DXCH_LION_RM_ENABLE_PFC_TRIGGER_WA_E,

    /** @brief The default setting of the CPFC xoff and drop thresholds
     *  should be set to the maximum.
     *  (Ref #: RM-9082194)
     */
    PRV_CPSS_DXCH_LION2_RM_WRONG_DEFAULT_CPFC_THRESHOLDS_WA_E,

    /** @brief The default setting of the LLFC thresholds should be set
     *  to the maximum.
     */
    PRV_CPSS_DXCH_LION2_RM_WRONG_DEFAULT_PFC_THRESHOLDS_WA_E,

    /** @brief The default value of the pfc global trigger
     *  register<pfcEn> should be set to 0x1.
     */
    PRV_CPSS_DXCH_LION2_RM_LLFC_TRIGGER_GLOBAL_CONFIGURATION_WA_E,

    /** @brief The default value of the cpfc global trigger
     *  register<cpfcEn> should be set to 0x1
     *  (Ref #: RM-9987707)
     */
    PRV_CPSS_DXCH_LION2_RM_ENABLE_PFC_TRIGGER_WA_E,

    /** The device Global pause default setting is incorrect. */
    PRV_CPSS_DXCH_LION2_RM_GLOBAL_XOFF_ACTIVATION_ON_PORT_WA_E,

    /** @brief The device supports a Device Map table that maps a destination device
     *  number to a cascaded port or cascaded trunk group. The Device Map
     *  look-up for the local device is based on a per-source-port enable bit
     *  and per-destination-port enable bit configuration.
     *  However, due to this erratum,
     *  the Device Map table enable bit configuration supports only 6 bits
     *  (up to 64 ports) for the per-source-port field instead of 7 bits
     *  (up to 128 ports). This will cause the same local port number in both
     *  hemispheres to be enabled at the same time.
     *  The WA configures EGR units asymmetrically:
     *  - The Local Src Port Map Own Dev En bitmap of specific EGR unit
     *  comprises bits only source port group related to the unit.
     *  E.g. EGR0 comprise bits 0..15, EGR1 comprise bits 16..31 and so on.
     *  - Write non zero values of source and target port bitmaps only for
     *  ERG units where both target and source ports are exist.
     *  (FE-6816092)
     */
    PRV_CPSS_DXCH_LION2_LOCAL_SRC_PORT_DEV_MAP_ENABLE_WA_E,

    /** @brief Changing the share enable after some traffic passed in the TXDMA cause
     *  system to hang: stuck buffers & stuck port (TXFIFO). The
     *  failure occurs even when the change is not under traffic
     *  and when TXDMA is completely idle. The failure occurs
     *  when changing the share enable in both directions: share
     *  to not share and the other way around.
     *  (DIP 423, Lion-B0 #1327)
     */
    PRV_CPSS_DXCH_LION2_SHARE_EN_CHANGE_WA_E,

    /** @brief IPM bridge copy get dropped when there is no member in
     *  the local hemisphere vlan
     *  WA is related to Lion2 devices starting from B0 revision
     *  WA solution:
     *  The WA use not connected to MAC ports 14 and 64+14 in
     *  the TxQ.
     *  Ports 14 of each hemisphere have Link UP in the TxQ unit
     *  The WA sets in every VLAN enabled for IP MC Routing Port
     *  14 and port 64+14 as a member.
     *  Packets in such VLANs go to ports 14 and 64+14 TxQ
     *  queues as well as to other ports in VLANs.
     *  The WA uses the Tail drop to avoid packets stuck in the
     *  ports 14 and 64+14 TxQ queues.
     *  The tails drop configurations is:
     *  - Tail Drop must be enabled
     *  - Configure Port 14 and port 64+14 to TailDropProfile 8
     *  NOTE: User can use only 7 out of 8 available profiles
     *  - Set Queue limits of profile 8 to all zeros
     *  (FE-7263524)
     */
    PRV_CPSS_DXCH_LION2_IPM_BRIDGE_COPY_GET_DROPPED_WA_E,

    /** @brief The device supports duplication of traffic to ports within the assigned
     *  VLAN. The device incorporates two internal
     *  engines that are responsible for the replication to all ports.
     *  However, due to this erratum, In some scenarios, although the traffic bandwidth
     *  in a specific PortGroup does not exceed the allowed bandwidth,
     *  not all VLAN members will transmit the replication.
     *  This behavior is due to extra replication (relay port)
     *  that is performed between the two internal replication engines.
     *  The restriction applies to a packet size under 128B only.
     *  The problem relevant both to Assigned Multicast (VIDX) and VLAN flooded traffic.
     *  Workaround
     *  Add the relay port to the VLAN only if
     *  ports from both hemisphere are members in the VLAN.
     *  (FE-6041254)
     */
    PRV_CPSS_DXCH_LION2_MC_BANDWIDTH_RESTRICTION_WA_E,

    /** @brief The device supports two independent groups of SDWRR
     *  queues on a port. Within each SDWRR group, the queues
     *  are serviced according to their configured weight.
     *  After changing weights profile a trigger to
     *  "UpdateSchedVarTrigger" Configuration bit need to be
     *  done to activate the new setting immediatlly. However,
     *  due to this erratum, triggering the
     *  "UpdateSchedVarTrigger" Configuration bit during traffic
     *  might cause BadOctetsReceived and RxErrorFrameReceived
     *  events in the Rx MAC MIBs.
     *  (FE-221066)
     */
    PRV_CPSS_DXCH_LION2_SCHEDULER_PROFILE_UNDER_TRAFFIC_WA_E,

    /** @brief When connecting a 10GBaseR port to a 40GBaseR port, where the
     *  IEEE 802.3ba Backplane Auto-Negotiation Service (AP) is
     *  not used, this could result in a false Link Up on the
     *  10GBase-R side.
     *  (AN-333: Lion2 10G to 40G Port Link)
     */
    PRV_CPSS_DXCH_LION2_DISMATCH_PORTS_LINK_WA_E,

    /** @brief In Lion2 port in trispeed mode link status doesn't
     *  change in some cases when cable is disconnected/connected.
     *  The workaround fix the problem.
     *  (FE-4933007)
     */
    PRV_CPSS_DXCH_LION2_GIGE_MAC_LINK_STATUS_WA_E,

    /** @brief Lion2 device does not allow cut through traffic
     *  from slow port to faster one. The WA allows traffic from
     *  slow port with cut through enabled to be transmitted
     *  from faster port.
     *  PRV_CPSS_DXCH_LION2_WRONG_DEFAULT_MLL_DATA_INTEGRITY_PARITY_INTERRUPT_WA_E-
     *  When pariry error is detected in MLL FIFO, an interrupt
     *  is generated to indicate the error. However, due to this
     *  erratum, false interrupt is generated during init
     *  sequence. Reading cause register will fix it.
     *  (FE-8054972)
     *  PRV_CPSS_DXCH_LION2_WRONG_DEFAULT_BMA_MC_DATA_INTEGRITY_PARITY_INTERRUPT_WA_E-
     *  When pariry error is detected in BMA MC data, an
     *  interrupt is generated to indicate the error. However,
     *  due to this erratum, false interrupt is generated during
     *  init sequence. Reading cause register will fix it.
     *  (FE-8071239)
     */
    PRV_CPSS_DXCH_LION2_CUT_THROUGH_SLOW_TO_FAST_WA_E,

    PRV_CPSS_DXCH_LION2_WRONG_DEFAULT_MLL_DATA_INTEGRITY_PARITY_INTERRUPT_WA_E,

    PRV_CPSS_DXCH_LION2_WRONG_DEFAULT_BMA_MC_DATA_INTEGRITY_PARITY_INTERRUPT_WA_E,

    /** @brief TXDMA releases buffers during normal operation. However,
     *  due to this erratum, buffers are not released causing
     *  the system (device) to hang, when jumbo packets are
     *  handled. In register: Buffer Memory GigFIFOs Threshold
     *  Field: GigDescFifoThreshold should be 0x4 instead of
     *  0xf.
     *  (RM-261461)
     */
    PRV_CPSS_DXCH_LION2_RM_WRONG_GIG_DESC_FIFO_THRESHOLD_WA_E,

    /** @brief The device implements MAC MIB counters, providing the counters necessary
     *  to support MAU, IEEE 802.3, and EtherLike MIB. However,
     *  due to this erratum, corruption in the MIB counters'
     *  values may occur upon exiting the port MAC reset state.
     *  This erratum is relevant in cases where traffic already
     *  passed through a port. Workaround: Clear the MIB
     *  counters after MAC reset. (FE-9085687)
     */
    PRV_CPSS_DXCH_LION2_PORT_MAC_MIB_COUNTERS_CORRUPTION_WA_E,

    /** @brief the device supports 'orig vid' for ingress tagged packets that send to
     *  the CPU.
     *  due to this erratum, the vid , in the ingress tagged packets that send to
     *  the CPU , is vid that was modified by the ingress pipe.
     *  The WA that the CPSS do :
     *  1. set <Force new DSA TO_CPU> on the CPU port.
     *  Impact on application:
     *  The info about the 'tagging' of the packets is always 'not tagged'.
     *  (CPSS_DXCH_NET_RX_PARAMS_STC::dsaParam.dsaInfo.toCpu.isTagged == GT_FALSE)
     *  meaning that the application should (if need) do tag classification
     *  by it's own.
     */
    PRV_CPSS_DXCH_BOBCAT2_NON_ORIG_VID_ON_TRAP_TO_CPU_WA_E,

    /** @brief The buffer memory read request FIFO queue is too short compared to the
     *  default threshold settings of the TxDMA FIFO queue. However, due to this
     *  erratum, read requests to the buffer memory sent by the TxDMA may get
     *  lost.
     *  (RM-3702958)
     */
    PRV_CPSS_DXCH_BOBCAT2_RM_WRONG_TXDMA_FIFO_THRESHOLD_WA_E,

    /** RateLimitDefecitCountEn default value should be set */
    PRV_CPSS_DXCH_BOBCAT2_RM_RATE_LIMIT_DEFICIT_COUNT_EN_WA_E,

    /** @brief Wrong default value for register <MCUC SDWRR and Strict
     *  Priority Configuration>. The field <UcStrictEn> should
     *  be configured to "Strict Priority"
     */
    PRV_CPSS_DXCH_BOBCAT2_RM_STRICT_PRIORITY_FOR_UC_PACKETS_WA_E,

    /** @brief Wrong TTI metal fix default value
     *  (RM-7143731)
     */
    PRV_CPSS_DXCH_BOBCAT2_RM_TTI_BAD_DEFAULT_VALUE_FIX_WA_E,

    /** @brief Default for the couple <Global Eport Range Value> and
     *  <Global ePort Range Mask> should be "never match"
     */
    PRV_CPSS_DXCH_BOBCAT2_RM_GLOBAL_EPORT_RANGE_VALUE_AND_MASK_WA_E,

    /** Wrong defaults of bridge commands */
    PRV_CPSS_DXCH_BOBCAT2_RM_BRIDGE_COMMANDS_DEFAULTS_WA_E,

    /** @brief Wrong default value for enabling VLAN filtering for
     *  control traffic from CPU
     */
    PRV_CPSS_DXCH_BOBCAT2_RM_CONTROL_FROM_CPU_EGRESS_FILTER_WA_E,

    /** @brief When Passenger Parsing of Transit MPLS Tunnels is enabled,
     *  the device implicitly locates start of the passenger payload
     *  based on the bottom of stack label (S-bit is set).
     *  In this case, the TTI Action Entry <Tunnel Header Length> is not relevant.
     *  However, due to this erratum, the device does not automatically locate
     *  the bottom of stack label to find the start of the passenger payload.
     *  Instead, the device uses the TTI Action Entry <Tunnel Header Length>
     *  to locate the start of the passenger payload
     *  (FE-4300730)
     */
    PRV_CPSS_DXCH_BOBCAT2_MPLS_TRANSIT_TUNNEL_PARSING_WA_E,

    /** Egress MIB counters are not cleared on read. */
    PRV_CPSS_DXCH_BOBCAT2_EGRESS_MIB_COUNTERS_NOT_ROC_WA_E,

    /** @brief TxQ Port Request mask shall have default value 4 instead of 3
     *  (FE-8708889)
     */
    PRV_CPSS_DXCH_BOBCAT2_RM_TXQ_PORT_REQUEST_MASK_WA_E,

    /** @brief TTI Action can not set ModifyUP and/or ModifyDSCP
     *  if previously assigned with QoSPrecedence==HARD
     */
    PRV_CPSS_DXCH_BOBCAT2_NOT_ALLOW_TTI_ACTION_TO_SET_MODIFY_UP_DSCP_WA_E,

    /** @brief A packet size limitation can be enforced on the
     *  Interlaken port. Received packets that are longer than
     *  the specified limitation in bytes are dropped. Oversized
     *  packets received by the Interlaken port are not dropped
     *  and can cause the Interlaken port to stop responding.
     *  (FE-820601)
     */
    PRV_CPSS_DXCH_BOBCAT2_ILKN_PACKET_SIZE_LIMITATION_NOT_ENFORCED_WA_E,

    /** @brief WRED must not be enabled on shared pools in enhanced
     *  taildrop mode.
     *  When using enhanced tail drop (device default mode) and
     *  the WRTD(WRED) is enabled, the shared pool counters are
     *  not updated correctly by the enqueue and dequeue parts
     *  of the TXQ. (FE-4381191)
     */
    PRV_CPSS_DXCH_BOBCAT2_ENH_TAILDROP_MODE_WRED_CONFLICT_WA_E,

    /** @brief Ports 4871 do not support Prioritybased Flow Control
     *  (PFC). (FE-6841659)
     */
    PRV_CPSS_DXCH_BOBCAT2_PFC_NOT_SUPPORTED_PORTS_48_71_WA_E,

    /** @brief Flow Control is not supported for ports that pass
     *  through the Traffic Manager. (FE-7583456)
     */
    PRV_CPSS_DXCH_BOBCAT2_FC_802_3X_NOT_SUPPORTED_TM_MAPPED_PORTS_WA_E,

    /** @brief In Protection switching, Protection Switching Stage Enable bit[2] of the
     *  ePort Global Configuration register should be enabled by default.
     *  However, due to this erratum, Protection Switching Stage Enable bit[2]
     *  of the ePort Global Configuration register is incorrect.
     *  The WA sets bit to be 1 during initialization.
     *  (RM-7299863)
     */
    PRV_CPSS_DXCH_BOBCAT2_RM_PROT_SWITCH_STAGE_EN_DEFAULT_VALUE_WA_E,

    /** @brief When we are working with FC/PFC (FCA bypass disable) the
     *  FC TX packets are counted in FCA unit and in
     *  goodSentPcts and not in fcSent in Mib Counters
     *  (FE-5101126)
     */
    PRV_CPSS_DXCH_BOBCAT2_FC_SENT_COUNTERS_PROBLEM_WHEN_FCU_BYPASS_DISABLE_WA_E,

    /** @brief ECC protection of descriptor fields is not operational
     *  when a remote physical port is used.
     *  ECC protection of the descriptor is not functional when
     *  the Remote Physical Ports feature is used, because the
     *  local device's source port field of the descriptor is
     *  modified by the TTI. False ECC indications appear. This
     *  includes any use of the Interlaken interface.
     *  (FE-9897292 )
     */
    PRV_CPSS_DXCH_BOBCAT2_DESCRIPTOR_ECC_PROTECTION_CORRUPTION_WITH_REMOTE_PORTS_WA_E,

    /** @brief CPU NA message for deleting an entry does not work for
     *  UC route entries (FE-2293984)
     */
    PRV_CPSS_DXCH_BOBCAT2_FDB_ROUTE_UC_DELETE_BY_MESSAGE_WA_E,

    /** @brief The TCAM has parity protection. The parity daemon
     *  periodically reads TCAM entries and checks parity bit.
     *  The daemon does not synchronizes TCAM reads and parity DB
     *  reads with write to TCAM management operation. Therefore
     *  sometimes the daemon erroneously detects parity error.
     *  The WA disables daemon before write to TCAM and enables
     *  it back after write.
     *  (FE-5092995)
     */
    PRV_CPSS_DXCH_BOBCAT2_TCAM_WRITE_PARITY_ERROR_WA_E,

    /** @brief CPU rate limiter is not working at low values of Conf.
     *  When ToCPU RLWindow Resolution [10:0] <= 0x4, CPU rate limiter is not working.
     *  Problem also can occur with MG is accessing to Rate Limiter RAM,
     *  while Rate Limiter is Enabled.
     *  The problem is that design requires to receive response from RAM till clocks.
     *  Minimum allowed value is taken from RAM round-trip latency
     */
    PRV_CPSS_DXCH_BOBCAT2_EQ_TO_CPU_RATE_LIMIT_WINDOW_RESOLUTION_WA_E,

    /** @brief OAM engine uses Egress Policer (EPLR) for Loss Measurement (LM). TO_CPU
     *  packets are treated by EPLR for LM also. However, due to the erratum,
     *  TO_CPU OAM LM packets are not processed by EPLR.
     *  The WA enables processing of all TO_CPU packets by EPLR.
     *  (RM-410147)
     */
    PRV_CPSS_DXCH_BOBCAT2_EPLR_TO_CPU_OAM_WA_E,

    /** @brief B0 registers misconfiguration
     *  We need to change the default setting of AVS in BC2B.
     *  1.    Need to sample on upper metal
     *  2.    Update voltage to 1.02V
     *  /Cider/EBU/Bobcat2B/Bobcat2 (Current)/<SERVER_IP> DFX Server/Units/
     *  DFX Server Units/DFX Server Registers/AVS <%a> Disabled Control 1
     *  Address 0x000F8128[23]
     *  Defaults (check) should be '1' - set to '0'
     *  To set to 1.02:
     *  /Cider/EBU/Bobcat2B/Bobcat2 (Current)/<SERVER_IP> DFX Server/Units/
     *  DFX Server Units/DFX Server Registers/AVS <%a> Enabled Control
     *  Address: 0x000F8130
     *  Bits 11:4 & 19:12 default is 35[Ofir A.] (0x23), set to 38[Ofir A.] (0x26)
     */
    PRV_CPSS_DXCH_BOBCAT2_RM_WRONG_AVS_INIT_WA_E,

    /** @brief relate the RxDMA PIP feature (SIP_5_10) , the 'TC' for the IPv6 packets
     *  should be bits 116..123 of the packet but actually taken from
     *  bits 118..125
     */
    PRV_CPSS_DXCH_BOBCAT2_RXDMA_PIP_IPV6_WRONG_TC_BITS_WA_E,

    /** @brief The device LED controller provides port status for all ports modes.
     *  One of the indications is "link status". However, due to this erratum,
     *  the Gig MAC link indication status does not reflect the correct status
     *  of the port. Gig Mac is relevant for 1Gig and 2.5Gig ports.
     *  Workaround:
     *  When setting a port to use Gig MAC, set Field "port[n] class1 select"
     *  bits [9:5] in the "Port[n] indication select" register to 0xa.
     *  For all other ports mode set this field to 0x1.
     *  (FE-1904523)
     */
    PRV_CPSS_DXCH_BOBCAT2_LED_LINK_GIG_INDICATION_WA_E,

    /** @brief The GPP interrupts need to be masked , as they generate nonstop interrupts.
     *  The device General Purpose Pins (GPP) are connected to the switch core
     *  and can be used as an input/output pin or as an interrupt input.
     *  However, due to this erratum, the switch core GPP are not functional and
     *  should not be used.
     *  (HWE-9666408)
     */
    PRV_CPSS_DXCH_BOBCAT2_GPP_INTERRUPT_WA_E,

    /** @brief The Control and Management subsystem is connected to the switch using the
     *  SDMA mechanism. In addition, packets may be transferred between the Control
     *  and Management subsystem and the switch by establishing an internal SGMII
     *  connection using SERDES 20 and/or SERDES 21.
     *  However, due to this erratum, an internal SGMII connection between the
     *  Control and Management subsystem and the switch is not operational.
     *  The SGMII interconnection between the Control and Management subsystem and
     *  the switch is not supported.
     *  (FE-5613945)
     */
    PRV_CPSS_DXCH_BOBCAT2_SGMII_TO_CPU_NOT_SUPPORTED_WA_E,

    /** @brief The default setting of the Device Control 14 register, bits [31..29]
     *  should be set to four.
     *  (Ref #: RM-7056426)
     */
    PRV_CPSS_DXCH_BOBCAT2_RM_WRONG_DEFAULT_DEVICE_CONTROL_14_WA_E,

    /** @brief Memories hold optimal WTC/RTC.
     *  However, due to this erratum, several memories do not hold
     *  optimal WTC/RTC in the memory wrapper registers.
     *  (Ref #: RM-79308357)
     */
    PRV_CPSS_DXCH_BOBCAT2_RM_DFX_REGISTERS_MISSCONFIGURATION_WA_E,

    /** @brief Packet generator can generate packets only on specific
     *  ports.
     *  Ports [0..47] - Does not work on ports: 0, 4, 8, 12, ... 44.
     *  Ports [48..71] - Does not work for all ports"
     *  (FE-143364)
     */
    PRV_CPSS_DXCH_BOBCAT2_PACKET_GENERATOR_SPECIFIC_PORTS_WA_E,

    /** @brief The default value of Port MAC Control Register 4,
     *  bit [8] should be set to 1.
     *  (Ref #: RM-2141433)
     */
    PRV_CPSS_DXCH_BOBCAT2_RM_WRONG_DEFAULT_PORT_MAC_CONTROL_REG4_WA_E,

    /** @brief The default value of CNM Header Configuration,
     *  bit [20] should be set to 1.
     *  (Ref #: RM-5288408)
     */
    PRV_CPSS_DXCH_BOBCAT2_RM_WRONG_DEFAULT_CNM_HEADER_CONFIGURATION_WA_E,

    /** @brief The <Checksum Trailer Enable>bit[12] of the Timestamping UDP Checksum
     *  Mode register should be enabled by default.
     *  (Ref #: RM-644887)
     */
    PRV_CPSS_DXCH_BOBCAT2_RM_PTP_UDP_CHECKSUM_TRAILER_BIT_WA_E,

    /** @brief The Ingress Forwarding Restrictions Drop Code determines the drop code
     *  that will be marked for the CPU in the EQ Unit.
     *  The Ingress Forwarding Restrictions Drop Code is set to 0x63, while it
     *  should be 0x3F (decimal 63).
     *  (Ref #: RM-4480490)
     */
    PRV_CPSS_DXCH_BOBCAT2_RM_INGRESS_FORWARDING_RESTRICTIONS_DROP_CODE_WA_E,

    /** @brief The devices supports a default value of the <ECMP enable> field in the
     *  Trunk LTT table.
     *  The default value is 'disabled'.
     *  the proper values should be :
     *  <ECMP enable> = 'enabled' ,
     *  < L2 ECMP Start Index > = last index in 'L2 ECMP'
     *  Other fields = 0
     *  The CPSS does not initialize all 4K entries, but only indexes 1..max
     *  (where 'max' is according to cpssDxChTrunkInit(...) value).
     *  The last index in 'L2 ECMP' holds 'NULL port' (62) , and CPSS not allow
     *  to modify it.
     *  (Ref #: RM-8499271)
     */
    PRV_CPSS_DXCH_BOBCAT2_RM_TRUNK_LTT_ENTRY_WRONG_DEFAULT_VALUES_WA_E,

    /** @brief The Shaper Parity Error Interrupt is invoked if a memory parity error is
     *  encountered in shaping token bucket memory.
     *  There are related registers that count the number of times the interrupt
     *  was invoked and the address of the memory
     *  that triggered the interrupt the last time.
     *  However, due to this erratum, parity protection is not functional.
     *  Workaround -
     *  The interrupt should be considered a false alarm and should be ignored
     *  by the application.
     *  The interrupt should not be mapped to CPSS general events, and it should
     *  not be tied to a callback function.
     *  Values in the interrupt-related counter and memory address registers
     *  should also be ignored.
     *  (Ref #: HWE-3984090)
     */
    PRV_CPSS_DXCH_BOBCAT2_RM_SHAPER_PARITY_ERROR_INTERRUPT_WA_E,

    /** @brief FDB Scheduler SWRR Control Register bit 0, can be 0 to
     *  use strict priority or 1 to use SWRR. The default should
     *  be 1 to use SWRR .
     */
    PRV_CPSS_DXCH_BOBCAT2_RM_FDB_SCHEDULER_SWRR_CONTROL_WA_E,

    /** @brief Time Application Interface (TAI) minimal TOD drift effect is dependent on
     *  the drift exception threshold.
     *  Description :
     *  The device incorporates a TAI unit, which keeps the time of day.
     *  This unit is used for accurate timestamping and for
     *  the Precision Time Protocol (PTP). The TAI unit can receive an incoming 1
     *  Pulse Per Second (1PPS) through its input
     *  PTP_CLK_IN interface. The device can be configured to mode ClockReceptionAdv;
     *  in this mode the incoming 1PPS is used for both phase alignment and frequency alignment.
     *  In this mode, the <Minimal Drift> configuration determines
     *  the minimal time drift that causes the frequency to be adjusted when an
     *  incoming 1PPS rising edge is received.
     *  However, due to this erratum, the minimal TOD drift effect is dependent
     *  on the drift exception threshold and not only
     *  on minimal drift. The frequency is adjusted in ClockReceptionAdv mode
     *  when the drift is greater than or equal to Min(<Minimal Drift>,
     *  <Drift Exception Threshold>).
     *  WA - Configure the <Drift Exception Threshold> field bits in the
     *  Drift Threshold Configuration Low register to be greater than the
     *  <Minimal Drift> field value in the same register.
     *  (FE-8248885)
     */
    PRV_CPSS_DXCH_BOBCAT2_TAI_MINIMAL_TOD_DRIFT_CONSTRAIN_WA_E,

    /** @brief The device supports the ability to enable/disable local switching
     *  traffic back through the ingress interface for known Unicast packets.
     *  However, due to this erratum, Unicast packets local switching filtering
     *  is not functional with logical ports.
     *  (FE-31600)
     */
    PRV_CPSS_DXCH_XCAT3_UC_LOCAL_SWITCH_FILTER_FOR_VIRTUAL_PORTS_WA_E,

    /** @brief Traffic is identified in the FDB as "learned on a trunk", instead of
     *  learned from a logical port, when a source port is configured as a trunk
     *  member and traffic is assigned with a logical port.
     *  (FE-1000365)
     */
    PRV_CPSS_DXCH_XCAT3_TRUNK_ID_ASSIGN_INSTEAD_OF_VIRTUAL_PORT_WA_E,

    /** @brief Traffic on OAM loopback port is not getting highest QoS, due to this
     *  erratum, the transmitted back traffic on the OAM loopback port gets
     *  randomized TC and DP.
     *  (FE-27970)
     */
    PRV_CPSS_DXCH_XCAT3_OAM_LB_PORT_NOT_GETTING_HIGHEST_QOS_WA_E,

    /** @brief The default value of bits [611] in the Port Serial
     *  Parameter Configuration register is wrong.
     *  (RM-6302994) (RM-8785501)
     */
    PRV_CPSS_DXCH_XCAT3_GIG_MAC_HALF_DUPLEX_COLLISION_DOMAIN_RM_E,

    /** @brief DMA Write access weight is increased in port RxFIFO.
     *  This change ensures that packets are written to memory
     *  If there is congestion on memory access.
     *  The Buffer Memory Tx Fifo Threshold and MPPM Aceess Arbiter Configuration
     *  Register is set as 0x4C4E6106
     *  For Stacking ports Port Rx FIFO is set to 4 to accomodate the pressure.
     *  The Port FIFO Thresholds Configuration Registers are set as 0x7BE4.
     *  (RM-6565603)
     */
    PRV_CPSS_DXCH_XCAT3_XLG_PORT_DROP_EVENT_PROBLEM_RM_E,

    /** @brief The CPU receives packets from the device directly to preallocate space in
     *  host memory using up to eight device Rx SDMA queues according to the
     *  configured mapping.
     *  However, due to this erratum, if there is a congestion on CPU port packets
     *  may go to the different queue from those configured in the mapping.
     *  WA -Set field set_port_sdma_ to 0 in register General_Configurations_1
     *  (RM-8732277)
     */
    PRV_CPSS_DXCH_XCAT3_RM_RX_SDMA_WRONG_QUEUE_WA_E,

    /** @brief When disabling the AutoNeg on 100M or 10M interface,
     *  CRC error will be reported. It is due to a missing byte
     *  in the packets. There is a Rx Data Valid signal (Rx_DV)
     *  that is asserted as long as the data is valid. Rx_ER can
     *  be asserted while Rx_DV is not activate. It is caused
     *  either by clock cycles at the end of the packet as
     *  extention to the packt defined by IEEE 802.3 standard
     *  (Clause 36 in 802.3x standard), or Marking Low Power
     *  Idle (LPI). When Rx_ER is asserted, the actual
     *  replicated Rx_DV is de-asserted and result in truncation
     *  of the last BYTE. WA - Set field
     *  rf_pcs_rx_er_mask_disable to 0 in register
     *  Gige_MAC_IP_Unit
     */
    PRV_CPSS_DXCH_MISCONFIG_100BASEX_AN_DISABLE_WA_E,

    /** @brief Device resets all memories and sets to default values after power UP or
     *  soft reset.
     *  However, due to this erratum,memories of several egress units (EPCL, EPLR, HA, e.t.c)
     *  are not reset and have random values.
     *  The WA fixes wrong default values of EPLR related memories.
     *  (FE-5949793)
     */
    PRV_CPSS_DXCH_CAELUM_EGRESS_MEMORIES_NOT_RESET_WA_E,

    /** @brief Decrement operation in TOD dosent work.
     *  (FE-5094244)
     */
    PRV_CPSS_DXCH_CAELUM_TOD_DECREMENT_OPERATION_E,

    /** @brief Ingress Bridge VLAN Table has 4.5 K (4608) entries in BobK.
     *  However, due to this erratum, Ingress VLAN Range Configuration register
     *  is initialized to 8K-2 which is bigger than the table size.
     *  The WA fixes wrong default values of Ingress VLAN Range Configuration register.
     */
    PRV_CPSS_DXCH_CAELUM_INGRESS_VLAN_RANGE_CONFIG_WA_E,

    /** @brief Incorrect default value may lead to wrong metering at
     *  the beginning of traffic need to set bit 0 with value 1
     *  at PLR[0,1,2] registers (relative) offset :
     *  For Caelum: 0x00003560, Bobcat3: 0x00003560, 0x00003564
     *  (relative offsets from start of the unit)
     *  (RM-3764513,FE-6799875)
     */
    PRV_CPSS_DXCH_CAELUM_RM_PLR_METERING_INACCURACY_IN_START_TRAFFIC_WA_E,

    /** @brief IPLR0/1 stages use a metering/counting memory pool that must be
     *  split among the stages at device initialization. In particular, the split may
     *  allocate 0 records to one of the IPLR stages.
     *  However, due to this erratum, packets are stuck in the IPLR stage
     *  allocated with 0 records.
     *  WA: Configure the unused IPLR stage to "Bypass".
     *  (RM-1764202)
     */
    PRV_CPSS_DXCH_CAELUM_RM_PLR_UNUSED_IPLR_STAGES_MUST_BE_BYPASSED_WA_E,

    /** @brief Port where enabled dp_clk hung upon link establish.
     *  For XCAT and XCAT2 see CPSS_DXCH_IMPLEMENT_WA_2_5G_SGMII_LINK_UP_E.
     *  (HWEr#3065)
     */
    PRV_CPSS_DXCH_IMPLEMENT_WA_2_5G_SGMII_LINK_UP_E,

    /** @brief Default configurations in PORT MAC CONTROL REGISTER 4 of
     *  the bit <en_idle_check_for_link> may lead to a 10G port
     *  stay in link-down if setting the port under traffic (and
     *  only if it's partner transmits in uni-directional mode)
     *  because of lost 3 idles. the WA disables the bit and
     *  therefore the 10G port doesn't expect for 3 idle's from
     *  the partner.
     *  (FE-662976)
     */
    PRV_CPSS_DXCH_CAELUM_10G_XLG_LINK_UP_E,

    /** @brief Wrong default value in EGF register Cleared Unicast Distribution.
     *  For Aldrin it should be 0x2.
     */
    PRV_CPSS_DXCH_ALDRIN_WRONG_DEFAULT_CLEARED_UC_DIST_WA_E,

    /** @brief Wrong default value in BM register BM Global Configuration 3.
     *  For Aldrin it should be 0x100.
     */
    PRV_CPSS_DXCH_ALDRIN_WRONG_DEFAULT_BM_GLOBAL_GFG_3_WA_E,

    /** @brief Device invalidates TCAM content after power UP or soft reset.
     *  However, due to this erratum,TCAM content are not invalid.
     *  The WA invalidates TCAM content.
     */
    PRV_CPSS_DXCH_ALDRIN_TCAM_CONTENTS_NOT_INITALIZED_WA_E,

    /** @brief < TCAM Segment Mode > is obsolete , must be set to 4TCAM (0x4)
     *  it apply to IPCL0,1,2 configuration table
     *  it apply to EPCL   configuration table
     *  it apply to TTI    Keys Segment Mode
     *  The WA : the CPSS ignores this parameter on API of 'pcl configuration table'
     *  and sets proper value to HW.
     *  the CPSS not allow to modify TTI Keys Segment Mode.
     *  (RM-3865199)
     */
    PRV_CPSS_DXCH_BOBCAT3_RM_TCAM_SEGMENT_MODE_OBSOLETE_WA_E,

    /** @brief Default configuration for the Ingress pipe, ingress/egress mirror,
     *  ingress mirror to CPU, ingress/egress STC, QCN arbiters.
     *  This arbiter is split among two units EQ - arbitrating among
     *  Ingress pipe, ingress mirror, ingress mirror to CPU, ingress STC and
     *  a client aggregating egress mirror/egress STC/QCN.
     *  The arbiter's service rate is one descriptor per cycle and is work conserving
     *  (RM-232626)
     */
    PRV_CPSS_DXCH_BOBCAT3_RM_EQ_ARBITERS_WA_E,

    /** @brief Performance degradation due to none workconserving arbiters in EFT/QAG.
     *  (RM-6190588)
     */
    PRV_CPSS_DXCH_BOBCAT3_RM_PERFORMANCE_DEGRADATION_IN_EGF_QAG_WA_E,

    /** @brief Register TXQ_IP_bmx/Fill Level Debug must be configured
     *  before using BMX
     *  (RM-6623635)
     */
    PRV_CPSS_DXCH_BOBCAT3_RM_TXQ_IP_BMX_FILL_LEVEL_DEBUG_WA_E,

    /** @brief CT (Cutthrough) numbers of buffers sets a fixed packet
     *  length for packets whose length is unknown at the time
     *  of processing (CT) for the tail-drop function.
     *  (RM-9114918)
     */
    PRV_CPSS_DXCH_BOBCAT3_RM_CUT_THROUGH_NUMBERS_OF_BUFFERS_WA_E,

    /** @brief In order to access CG regfile registers without causing
     *  the PEX to get stuck, we need to unreset the CG regfile
     *  subunit at init.
     */
    PRV_CPSS_DXCH_BOBCAT3_CG_REGFILE_UNRESET_WA_E,

    /** @brief Each descriptors shared pool threshold should be
     *  configured to 128 above the real required value.
     *  (RM-3993804)
     */
    PRV_CPSS_DXCH_BOBCAT3_DESC_SHARED_POOL_TAILDROP_THRESHOLDS_WA_E,

    /** @brief Cutthrough operation may result in packet corruption if header
     *  processing is configured to start too early. CPSS must set minCT = 256B.
     *  This is set in the <Maximal Non Cut-Through Byte Count> field in the
     *  SCDMA <%n> CT Packet Identification Register.
     *  (RM-1428936)
     */
    PRV_CPSS_DXCH_BOBCAT3_RM_CUT_THROUGH_PACKET_CORRUPTION_WA_E,

    /** @brief Default value of the stg7_desc_fifo_threshold and stg5_desc_fifo_threshold should be changed to 342 entries.
     *  Threshold values are set in Router FIFOs threshold 1 register.
     *  (RM-9489137)
     */
    PRV_CPSS_DXCH_BOBCAT3_RM_ROUTER_FIFOS_THRESHOLD_1_WA_E,

    /** @brief all not used mac's LED port number shall be set to 0x3F
     *  (Errata FE-3763116 )
     */
    PRV_CPSS_DXCH_BOBCAT3_PORT_LED_POSITION_JUST_ON_USED_MAC_WA_E,

    /** @brief The device is capable of transmitting traffic on a port
     *  regardless of the link state (up or down). However, due
     *  to this erratum, link-down on a 100G port, blocks
     *  traffic transmission on that port, potentially
     *  oversubscribing the egress queue manager (TxQ). Upon
     *  link-up, the stalled traffic is transmitted, including a
     *  potential fragment.
     *  (FE-1030304)
     */
    PRV_CPSS_DXCH_BOBCAT3_LINK_DOWN_100G_PORT_BLOCKS_THE_TRANSMIT_PATH_WA_E,

    /** @brief The Policer Billing counting table expected to support indirect 'read'
     *  and 'read and reset' to support
     *  atomic operation and counters coherency.
     *  However, due to this erratum those actions are not applicable when
     *  <counting mode> is 'disabled'.
     *  The WA should do :
     *  a. Instead of indirect 'read' - do direct read.
     *  b. Instead of indirect 'read and reset' - do direct read , indirect write of ZERO content.
     *  (FEr-8039652)
     */
    PRV_CPSS_DXCH_BOBCAT3_PLR_INDIRECT_READ_COUNTERS_ON_DISABLED_MODE_WA_E,

    /** @brief The Shaper Baseline with defaults 1536 , cause inaccuracy for 100G ports.
     *  The defaults should be : 1010K
     *  (RM-???)
     */
    PRV_CPSS_DXCH_BOBCAT3_RM_TXQ_TX_SHAPER_BASELINE_WA_E,

    /** @brief Sync E does not work with 50G and 100G ports. Automatic
     *  mask enable configuration should be added to fix it.
     *  (FE-9442185)
     */
    PRV_CPSS_DXCH_BOBCAT3_100G_SYNC_ETHERNET_WA_E,

    PRV_CPSS_DXCH_BOBCAT2_FORCE_DOWN_LINK_PARTNER_WA_E,

    /** @brief The 1G and 2.5G MAC interfaces include a FIFO module on
     *  the port transmission side, which accumulates data up to a
     *  configurable threshold prior to data transmission, to
     *  ensure that a data underrun does not occur. A port data
     *  underrun can occur, because the default value of the FIFO
     *  threshold for 1G and 2.5G port modes is incorrect.
     *  (RM-9725147)
     *  1G/2.5G:
     *  Register: /<GIGE_MAC_IP> Gige MAC IP %a Pipe %t/Port Internal/Port FIFO Configuration Register 1
     *  Field: TxFifoMinTh
     *  The 2 Water-Marks need also to be configured:
     *  Register: /<GIGE_MAC_IP> Gige MAC IP %a Pipe %t/Port Internal/Port FIFO Configuration Register 0
     *  Field: TxFifoLowWM = TX_Threshold + 2
     *  TxFifoHiWM = TX_Threshold + 4
     *  BC3
     *  1G - TxFifoMinTh[1000] = 129;
     *  2.5G - TxFifoMinTh[2500] = 129;
     *  Aldrin/BobK
     *  1G - TxFifoMinTh[1000] = 65;
     *  2.5G - TxFifoMinTh[2500] = 129;
     *  BC2B0
     *  1G - TxFifoMinTh[1000] = 57;
     *  2.5G - TxFifoMinTh[2500] = 129;
     *  Aldrin2
     *  1G - TxFifoMinTh[1000] = 140;
     *  2.5G - TxFifoMinTh[2500] = 140;
     */
    PRV_CPSS_DXCH_GIGE_MAC_TX_FIFO_WA_E,

    /** @brief FE5803053 WA implemented for for Bobcat2, BobK, Aldrin, Bobcat3.
     *  The device enabled CPU read and write accesses to it's configuration tables
     *  and registers under traffic
     *  However, due to this erratum, CPU access to the below tables under high packet rate
     *  may be starved and lead to PCIe
     *  timeout and inability to access the devices registers.
     *  - Queue buffer limit dequeue
     *  - Queue descriptor limit dequeue
     *  - Queue dp0 enqueue
     *  - Queue dp12 enqueue
     *  - Maximum queue limits
     */
    PRV_CPSS_DXCH_BOBCAT2_TXQ_TAIL_DROP_TABLES_R_W_PENDS_BUS_WA_E,

    /** @brief  RM-7969777 WA implemented for for BobK and above.
     *  Frequency adjust time window should be internalized and configured to 0x1 only (default is 0x0!)
     */
    PRV_CPSS_DXCH_CAELUM_TAI_FREQUENCY_ADJUST_TIME_WINDOW_INIT_VAL_WA_E,

    /** @brief Set "Egress Policy FIFO Configuration" register address 0x160000A0 to value 48 (0x30).
     *  (RM-416370)
     */
    PRV_CPSS_DXCH_ALDRIN2_EPCL_TCAM_LOOKUP_LATENCY_WA_E,

    /** @brief Due to errata, rate limiting for known unicast and unknown
     *  unicast does not work
     *  (FER - 7152725)
     */
    PRV_CPSS_DXCH_ALDRIN2_BRIDGE_RATE_LIMIT_UC_WA_E,

    /** @brief Due to erratum, Policy-based routed (PBR) packets are
     *         treated as Bridge "Unknown DA"
     *  (FE - 1436291)
     */
    PRV_CPSS_DXCH_BOBCAT2_PBR_PACKETS_TREATED_AS_UNKNOWN_DA_WA_E,

    /** @brief JIRA CPSS-7938, RM TCAM-528
     *  The TCAM Answers Floor Sample register initial value is not correct -
     *  Should be fixed by CPSS to 0x2A.
     *  Fixed in HW Design of TCAM unit for next versions of Packet Processors.
     */
    PRV_CPSS_DXCH_ALDRIN2_TCAM_ANSWERS_FLOOR_SAMPLES_INIT_VALUE_WA_E,

    /** @brief In Retry Packet Reception mode, when a resource error occurs,
     *  the SDMA automatically retrys transmitting the packet to the CPU,
     *  after a fixed period of time.
     *  However, due to this erratum, in Retry Packet Reception mode,
     *  when the host CPU deliberately tries to disable a queue for packet reception,
     *  the queue may automatically be re-enabled as part of the Retry mechanism.
     *  This queue enabling happens regardless of whether there was a resource error
     *  or the queue was manually disabled.
     *  The WA is change Retry mode to Abort mode when disableing a queue. (FE-6025612)
     */
    PRV_CPSS_DXCH_ALDRIN2_QUEUE_VALUE_DISABLE_WA_E,

    /** @brief Pfc Xoff messages should be sent to source port according xoff threshold
     *  However, due to this erratum,
     *  In some cases, xoff threshold is corrupted, causing xoff messages to get lost
     *  As result, from system perspective, congested device is not sending xoff messages. Eventually tail-drops occurs.
     *  Meaning that lossless system become lossy
     *  The WA is to set "PfcIndFifoCrossModeThreshold" register to 10.
     *  (FE-9772880)
     */
    PRV_CPSS_DXCH_BOBCAT3_RM_PFC_IND_FIFO_CROSS_MODE_THRESHOLD_WA_E,

    /** @brief Falcon : the next tables in the L2i and EQ units must be read
     *  by the CPU before traffic tries to access those tables.
     *  Need to read single entry from all the tables from all the 'port groups'
     *  the tables are :
     *  1. L2I ingress eport learn prio Table (CPSS_DXCH_SIP5_TABLE_BRIDGE_INGRESS_EPORT_LEARN_PRIO_E)
     *  2. l2i source trunk attribute Table   (CPSS_DXCH_SIP5_TABLE_BRIDGE_SOURCE_TRUNK_ATTRIBUTE_E)
     *  3. eq ePort to LOC Mapping Table      (CPSS_DXCH_SIP5_TABLE_EQ_EPORT_TO_LOC_MAPPING_E)
     *  4. eq l2 ECMP Table                   (CPSS_DXCH_SIP5_TABLE_EQ_L2_ECMP_E)
     *
     *  (FE-6585612, ERR-36365478)
     */
    PRV_CPSS_DXCH_FALCON_L2I_AND_EQ_READ_TABLE_BEFORE_TRAFFIC_WA_E,

    /** @brief Falcon : TAI control 1 register have wrong default value.
     *
     *  (ERR-4023766)
     */
    PRV_CPSS_DXCH_FALCON_WRONG_DEFAULT_CONFIG_FOR_TAI_CTRL_1_E,

    /** @brief Falcon : SW reset is not supported for clients using XOR memory
     *  Read and write 7 different entries for each SBM before triggering SW reset.
     *  It will cause the (up to) 7 entries that in the cache to be written to the XOR memory
     *  and saved through the SW reset flow.
     *  The accesses should be done through the clients:
     *  - FDB, EM - through the indirect access registers
     *  - HA TS/ARP - direct mapped table
     *  - HA eport 1 (which is not in SBM) - direct mapped table. need 5 writes only
     *    LPM allocated memories don't need this workaround since they don't work in XOR mode
     *
     *  (ERR-8933811, ERR-9939563)
     */
    PRV_CPSS_DXCH_FALCON_SBM_XOR_CLIENTS_READ_WRITE_BEFORE_SW_RESET_WA_E,

    /** @brief Falcon : Back-up skipped TxQ tables content on SW reset
     *  Back-up and restore following skipped TxQ tables on SW reset:
     *  - SDQ port config
     *  - SDQ select list configuration
     *  - PDQ memory
     *
     *  (ERR-???)
    */
    PRV_CPSS_DXCH_FALCON_TXQ_SKIP_TABLES_BACKUP_WA_E,

    /** @brief Falcon : SIP search should be disabled on default
     *
     *  (ERR-8431241)
     */
    PRV_CPSS_DXCH_FALCON_WRONG_DEFAULT_CONFIG_SIP_LOOKUP_E,

    /** @brief Falcon : Default IPv6 TT HBH Extension Exception
     *         should be FORWARD
     *
     *  (RM-4808598)
     */
    PRV_CPSS_DXCH_FALCON_WRONG_DEFAULT_IPV6_TT_HBH_EXT_EXCEPTION_CMD_E,

    /** @brief Falcon : Default IPv6 TT non HBH Extension Exception
     *         should be FORWARD
     *
     *  (RM-2311861)
     */
    PRV_CPSS_DXCH_FALCON_WRONG_DEFAULT_IPV6_TT_NON_HBH_EXT_EXCEPTION_CMD_E,

    /** @brief Falcon : Default Check for CT Byte Count Length
     *         should be enabled
     *
     *  (RM-6183282)
     */
    PRV_CPSS_DXCH_FALCON_WRONG_DEFAULT_CT_BC_LEN_CHECK_E,

    /** @brief Falcon : Default CT Header Integrity Exception
     *         Command should be FORWARD
     *
     *  (RM-3612721)
     */
    PRV_CPSS_DXCH_FALCON_WRONG_DEFAULT_CT_HEADER_INTEGRITY_EXCEPTION_CMD_E,

    /** @brief The device supports a different Maximum Receive Unit (MRU) limit
     *  per default ePort. When the MAC removes the CRC (default mode) the packet
     *  size is 4 bytes less than the real packet size for the MRU calculations.
     *  WA:
     *  The value configured as the MRU should be 4 bytes less then the actual MRU.
     *  (FE-4084040)
     */
    PRV_CPSS_DXCH_FALCON_MRU_PER_DEFAULT_EPORT_WA_E,

    /** @brief The Bridge eVLAN Maximum Receive Unit (MRU)limits the maximum packet
     *  size (in bytes) that can be received for the given packet eVLAN-ID assignment.
     *  Erratum:
     *  The packet byte count does not take into account the 4B CRC that is removed
     *  when the packet is received.
     *  WA:
     *  The value configured as the MRU should be 4 bytes less then the actual MRU.
     *  (FE-4003902)
     */
    PRV_CPSS_DXCH_FALCON_MRU_BRIDGE_EVLAN_WA_E,

    /** @brief TTI sets the <SYNWithData> without checking if <L4Valid> == 1
     *
     *  (FE-2001166)
     */
    PRV_CPSS_DXCH_XCAT3_TCP_SYN_WITH_DATA_E,

    /** @brief Power-on state of bit MPLS Cut Through Enable is 1
     *  This bit causes Enabling Cut Through for any MPLS tunneled packet,
     *  even having no IP header inside and causing TTI to fail calculating Byte Count
     *  and Egress Filtering Cut Through termination.
     *  This value should be fixed to 0.
     *  CPSS has API cpssDxChCutThroughMplsPacketEnableSet to configure it if needed.
     *  (CPSS-10686)
     */
    PRV_CPSS_DXCH_AC5P_MPLS_CUT_THROW_ENABLE_E,

    /** @brief Bobcat2 TCAM Array Speed register <Match Cfg> value should be fixed to 2.
     *  Bits [2:1] of the register at offset: 0x05504014 in the TCAM domain
     *  should have a default value of 0x2. But the register default value is 0x0.
     *  WA: To overcome this misconfiguration, during the init stage,
     *  set the register default value to 0x2.
     *  RM-9545365
     */
    PRV_CPSS_DXCH_BOBCAT2_TCAM_ARRAY_SPEED_MATCH_CFG_INIT_E,

    /** @brief Master TAI <Manchester Half Bit Symbol Length> configuration reset value should be 0x7.
     * Since master TAIs of Phoenix runs on clock frequency=800MHz, default manchester half bit
     * symbol length='h4 is too short for slaves.
     * Field: <Manchester Half Bit Symbol Length>, Bits [15:0]
     * RM-6347316
     */
    PRV_CPSS_DXCH_AC5P_RM_MASTER_TAI_DEFAULT_MANCHESTER_HALF_BIT_SYMBOL_LENGTH_E,

    /** @brief There are features "EREP Convert DROP to TRAP or Analyzer", and EPCL rules can be triggered
     *  for DROP packets. These features require that packets dropped by ingress engines go to EREP unit.
     *  Such packets terminated by EQ unit by default.
     *  The WA is change default and allow packets to reach EREP unit.
     *  RM-4716194
     */
    PRV_CPSS_DXCH_AC5P_RM_RELOCATE_EQ_DROP_TO_EREP_WA_E,

     /** @brief Limit the number of virtual buffers
     *
     *  (RES-1727594)
     */
    PRV_CPSS_DXCH_FALCON_TAIL_DROP_ON_VIRTUAL_BUFFERS_E,

    /** @brief Default domain ID for any domain index is 0, but
     *  should be 255.
     *  RM-9545365
     */
    PRV_CPSS_DXCH_BOBCAT2_RM_DEFAULT_DOMAIN_ID_PER_DOMAIN_INDEX_E,

    /** @brief Falcon: The WA to change the default trunk mode to "CRC" and
     *         hash function selection to "CRC-32"
     *
     *  (CPSS-11141)
     */
    PRV_CPSS_DXCH_FALCON_RM_HASH_MODE_WA_E,


    /** @brief Pass to EPI(FSU) a future time different from the one
     *         embedded in the packet to avoid the use of FSU
     *         <offset> config.
     *         AC5P: WA - put '0' in FSU <offset> field.
     *         AC5X: Feature is not supported.
     *
     *  (ERR-9562790)
     */
    PRV_CPSS_DXCH_AC5P_TIMESTAMP_TO_THE_FUTURE_WA_E,

    /** @brief ToD update with "rounded" seconds disrupts 1PPS
     *         In/Out sync.
     *         WA: Before performing any ToD Update operation:
     *             1. Disable the PPS generation.
     *             2. Perform ToD Update operation as planned.
     *             3. Enable the PPS generation.
     *
     *  (FE-5713509)
     */
    PRV_CPSS_DXCH_AC5P_TOD_UPDATE_SECONDS_WA_E,

    /** @brief  Aldrin SDWRR not accurate in packet mode.
     *  WA is:
     *  1. configure Port Request Mask register (conains 4 mask values)
     *  configure one of 4 masks to value 10 (for ports with speed <= 10G). Leave other values unchanged (i.e. 5).
     *  2. configure Request Mask Selector register (per port)
     *  select mask configured to 10 for ports with speed <= 10G and for faster ports to other (left to be 5).
     *  (TXQ-2058) (CPSS-11105)
     */
    PRV_CPSS_DXCH_ERRATA_ALDRIN_SDWRR_WA_E,

    /** @brief TAI "Drift Min Threshold" default value should be 0x1
     *  instead of the 0x5 default.
     *
     *  (ERR-3871812) (TAI-855)
     */
    PRV_CPSS_DXCH_AC5P_RM_DRIFT_MIN_THRESHOLD_WA_E,

     /** @brief Falcon, AC5X, AC5P, Harrier: SW reset is not supported in 'skip' registers mode
     *
     *  (ERR-???)
     */
    PRV_CPSS_DXCH_FALCON_SKIP_REGISTERS_SW_RESET_NOT_SUPPORTED_WA_E,

    /** @brief By default, the IPFIX first N packets should be mirrored to the CPU.
     *  But default command for such packets is Forward.
     *  The WA is change packet command to be Mirror To Cpu.
     *  RM-5882135
     */
    PRV_CPSS_DXCH_AC5P_RM_IPFIX_FIRST_PACKET_CMD_WA_E,

    /** @brief Bridge Global Configuration1 <Check IP Header> has 2 modes:
     *  0 - Use DA only to determine if packet is IPM
     *  1 - Use DA and EtherType to determine if packet is IPM
     *  This should be an internal configuration and the default should be set to '1' - use DA and EtherType
     *  RM-4530719
     */
    PRV_CPSS_DXCH_AC5P_RM_BRIDGE_CHECK_IP_HEADER_WA_E,

    /** @brief PTP PLL by default doesn't get any clock and as a result CPSS can't access TAI master.
     *  WA: Need to set ptp_ref_clk_select[1] to 0 so PTP PLL will receive the global ref clock.
     *  RM-1791879
     */
    PRV_CPSS_DXCH_HARRIER_RM_PTP_PLL_SET_GLOBAL_REF_CLOCK_WA_E,

    /** @brief When trying to update the pclk_counter using the FREQ_UPDATE function,
     * it updates the pclk_counter's tod_step incorrectly.
     *  WA:
     *  1. set internal bit [10] to 1'b1 (PClk Drift Adjustment Enable).
     *  2. FREQ_UPDATE values should be limited to the following boundaries: (-1/4 < FREQ_UPDATE < 1/4).
     *  ERR-6757973 (TAI-917)
     */
    PRV_CPSS_DXCH_AC5P_RM_PTP_FREQ_UPDATE_FOR_PCLK_IS_DONE_INACCURATELY_WA_E,

    /** the MAX value to be used for array sizes */
    PRV_CPSS_DXCH_ERRATA_MAX_NUM_E

} PRV_CPSS_DXCH_ERRATA_ENT;

/**
* @struct PRV_CPSS_DXCH_ERRATA_ADDR_RANGE_STC
 *
 * @brief Describes restricted address range
*/
typedef struct{

    /** Range start offset */
    GT_U32 addrStart;

    /** Range end offset */
    GT_U32 addrEnd;

    /** @brief In some cases specific addresses are not valid and not
     *  the whole range. If address & addrMask equals addrPattern
     *  then this address is not valid.
     */
    GT_U32 addrMask;

    /** @brief Pattern to check address with addrMask, if addrMask is
     *  not 0 and there is match, then this address is not valid.
     */
    GT_U32 addrPattern;

} PRV_CPSS_DXCH_ERRATA_ADDR_RANGE_STC;


/**
* @struct PRV_CPSS_DXCH_ERRATA_UNIT_RANGES_STC
 *
 * @brief Describes all ranges of particular unit
*/
typedef struct{

    /** @brief Number of ranges
     *  rangesPtr  - Pointer to ranges
     */
    GT_U32 numOfRanges;

    const PRV_CPSS_DXCH_ERRATA_ADDR_RANGE_STC *rangesPtr;

} PRV_CPSS_DXCH_ERRATA_UNIT_RANGES_STC;


/**
* @struct PRV_CPSS_DXCH_ERRATA_STC
 *
 * @brief A structure to hold all Errata info that CPSS need to fix
 * (workaround) for the PP .
*/
typedef struct{
    GT_U32          FErArray[PRV_CPSS_DXCH_ERRATA_BMP_MAX_NUM_CNS];

    struct{
        GT_U32      pclInterruptCauseRegAddr;
        GT_U32      pclInterruptIndex;
        GT_U32      pclInterruptCauseRegAddr1; /*AC5 second PCL TCAM*/
        GT_U32      pclInterruptIndex1; /*AC5 second PCL TCAM*/

        GT_U32      routerInterruptCauseRegAddr;
        GT_U32      routerInterruptIndex;
    }info_PRV_CPSS_DXCH3_CPU_READ_TCAM_ECC_INTERRUPT_WA_E;

    struct{
        GT_BOOL     enabled;
        GT_U8       devNum;
    }info_PRV_CPSS_DXCH_XCAT_SDMA_PACKETS_FROM_CPU_CORRUPT_WA_E;

    struct{
        GT_BOOL     enabled;
    }info_PRV_CPSS_DXCH_XCAT_FDB_AU_FIFO_CORRUPT_WA_E;

    struct{
        GT_BOOL     enabled;
        GT_BOOL     filterEnabled;
    }info_PRV_CPSS_DXCH_LION_FDB_AU_FU_MESSAGES_FROM_NON_SOURCE_PORT_GROUP_WA_E;

    struct
    {
        PRV_CPSS_DXCH_ERRATA_UNIT_RANGES_STC    *unitRangesPtr[CPSS_MAX_PORT_GROUPS_CNS];
        GT_U32                                  lastValidAddress;
    }info_PRV_CPSS_DXCH_XCAT_RESTRICTED_ADDRESS_FILTERING_WA_E;

    struct{
        CPSS_PORTS_BMP_STC *portTxQueuesBmpPtr;
    }info_PRV_CPSS_DXCH_XCAT_GE_PORT_DISABLE_WA_E;

    struct{
        GT_BOOL enabled;
        CPSS_PORTS_BMP_STC *pcsLoopbackBmpPtr;
        CPSS_PORTS_BMP_STC *portEnableBmpPtr;
    }info_PRV_CPSS_DXCH_XCAT_GE_PORT_UNIDIRECT_WA_E;

    struct{
        GT_BOOL                                     enabled;
        GT_BOOL                                     tailDropUcEnable;
        PRV_CPSS_DXCH_PROFILE_INFO_STC              profileInfo[CPSS_DP_RANGE_CNS];
    }info_PRV_CPSS_DXCH_LION2_IPM_BRIDGE_COPY_GET_DROPPED_WA_E;

    struct{
        GT_BOOL     enabled;
    }info_PRV_CPSS_DXCH_LION2_MC_BANDWIDTH_RESTRICTION_WA_E;

    struct{
        CPSS_PORTS_BMP_STC *portForceLinkDownBmpPtr;
    }info_PRV_CPSS_DXCH_LION2_DISMATCH_PORT_LINK_WA_E;

    struct{
        GT_BOOL     enabled;
    }info_PRV_CPSS_DXCH_LION2_CUT_THROUGH_SLOW_TO_FAST_WA_E;

    struct{
        CPSS_PORT_EGRESS_CNTR_STC  egressCntrShadow[2];
        GT_U32                     histogramCntrShadow[4];
    }info_PRV_CPSS_DXCH_BOBCAT2_EGRESS_MIB_COUNTERS_NOT_ROC_WA_E;

    struct {
        GT_BOOL  enabled;
    }info_PRV_CPSS_DXCH_BOBCAT2_REV_A0_40G_NOT_THROUGH_TM_IS_PA_30G_WA_E;

    struct {
        GT_BOOL  enabled;
    }info_PRV_CPSS_DXCH_XCAT_ROUTER_TCAM_RM_WA_E;

    struct {
        GT_BOOL  enabled;
    }info_CPSS_DXCH_IMPLEMENT_WA_BOBCAT2_PTP_TIMESTAMP_E;

    struct{
        CPSS_PORTS_BMP_STC *portPcsRxErMaskDisableBmpPtr;
    }info_PRV_CPSS_DXCH_MISCONFIG_100BASEX_AN_DISABLE_WA_E;
    struct{
        GT_U32  saveRegDataArr[2];
    }info_PRV_CPSS_DXCH_TXQ_TAIL_DROP_TABLES_ACCESS_WA_E;
    struct {
        /** @brief bitmap of ports capatible to this errata.
         *  These ports are treated according to this errata
         *  when configured to XGL MAC. Ports capatity set by Application.
         */
        CPSS_PORTS_BMP_STC waCapablePortsBmp;
        /** @brief bitmap of ports actually treated
         *  according to this errata. This Port state set by
         *  API cofiguring port mode and speed.
         */
        CPSS_PORTS_BMP_STC xlgUniDirPortsBmp;
    } info_PRV_CPSS_DXCH_PORTS_XLG_UNIDIRECTIONAL_WA_E;

    struct{
        /** @brief Status of queue, indicating if restore required
         *  according to this errata. This flag is being managed
         *  by SDMA Rx queue enable function.
         */
        GT_BOOL restoreRequired[NUM_OF_SDMA_QUEUES];
    }info_PRV_CPSS_DXCH_ALDRIN2_QUEUE_VALUE_DISABLE_WA_E;

    struct{
        /** @brief Pointer to the List holding,
         * LTT Entries with index to the secondary DLB area
         **/
        PRV_CPSS_DXCH_BRG_L2_DLB_WA_LTT_USAGE_STC *headOfDlbWaUsageList;

        /** @brief Indicates to the secondary region base index of LTT table */
        GT_U32                                     secondaryLttBaseIndex;

        /** @brief Indicates to the primary region base index of LTT table */
        GT_U32                                     primaryLttBaseIndex;

        /** @brief Indicates DLB WA status
         * GT_TRUE  - DLB WA is enabled
         * GT_FALSE - DLB WA is disabled
         */
        GT_BOOL                                    dlbWaEnable;

        /* @brief Indicates which area of LTT/DLB table is current in use
         * 0 - Primary area in use
         * 1 - Secondary area in use
         **/
        GT_U32                                     blockInUse;
    }info_PRV_CPSS_DXCH_BRG_L2_DLB_TIMER_COUNTER_WA_E;

    /* @brief The control pipe calculates the packet byte count for cut-through packets.
              If incoming packets are not standard Ethernet, the parser may incorrectly
              calculate the packet byte count
    */
    struct {
        GT_BOOL  enabled;
    }info_PRV_CPSS_DXCH_IMPLEMENT_WA_CUT_THROUGH_NON_ETHERNET_WRONG_BYTE_COUNT_E;

}PRV_CPSS_DXCH_ERRATA_STC;


/* macro to check that the WA for "packets sent from cpu by sdma corrupted"
   requested by the application (even if PP not require)
*/
#define PRV_CPSS_DXCH_CHECK_SDMA_PACKETS_FROM_CPU_CORRUPT_WA_MAC(_devNum)  \
    ((PRV_CPSS_DXCH_PP_MAC(_devNum)->errata.                                \
        info_PRV_CPSS_DXCH_XCAT_SDMA_PACKETS_FROM_CPU_CORRUPT_WA_E.        \
            enabled == GT_TRUE) ? 1 : 0)

/* macro to check that the WA for "AU/FU messages from non-source port group"
   requested by the application (even if PP not require)
*/
#define PRV_CPSS_FDB_AUQ_FROM_NON_SOURCE_PORT_GROUP_WA_E(_devNum)  \
    ((PRV_CPSS_DXCH_PP_MAC(_devNum)->errata.                       \
        info_PRV_CPSS_DXCH_LION_FDB_AU_FU_MESSAGES_FROM_NON_SOURCE_PORT_GROUP_WA_E. \
            enabled == GT_TRUE) ? 1 : 0)

/* macro to check that the filter enable/disabled for
    "AU/FU messages from non-source port group" */
#define PRV_CPSS_FDB_AUQ_FROM_NON_SOURCE_PORT_GROUP_FILTER_ENABLED_WA_E(_devNum)  \
    ((PRV_CPSS_DXCH_PP_MAC(_devNum)->errata.                       \
        info_PRV_CPSS_DXCH_LION_FDB_AU_FU_MESSAGES_FROM_NON_SOURCE_PORT_GROUP_WA_E. \
            filterEnabled == GT_TRUE) ? 1 : 0)

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvCpssDxChErrataMngh */


