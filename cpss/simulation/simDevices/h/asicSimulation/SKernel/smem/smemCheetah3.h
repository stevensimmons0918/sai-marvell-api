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
* @file smemCheetah3.h
*
* @brief Data definitions for Cheetah3 memories.
*
* @version   34
********************************************************************************
*/
#ifndef __smemCht3h
#define __smemCht3h

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <asicSimulation/SKernel/smem/smemCheetah2.h>

/* Central counters blocks number */
#define     CNC_CNT_BLOCKS(dev)  ((dev)->cncBlocksNum)

typedef enum{
    SMEM_CHT_PLR_MEMORY_TYPE_METERING_E,
    SMEM_CHT_PLR_MEMORY_TYPE_COUNTING_E,
    SMEM_CHT_PLR_MEMORY_TYPE_METERING_CONFIG_E,/*new in sip 5.15 */
    SMEM_CHT_PLR_MEMORY_TYPE_CONF_LEVEL_SIGN_E, /*new in sip 5.15 */
    SMEM_CHT_PLR_MEMORY_TYPE_OTHER_E
}SMEM_CHT_PLR_MEMORY_TYPE_ENT;

/***********************************************************************
*    0- Global Registers, SDMA registers, Master XSMI registers and TWSI
*       registers.
*    1 - 2  reserved
*    3- Transmit Queue registers
*    4- Ethernet Bridge registers
*    5- IP Routing Registers
*    6- Buffer Management register
*    7- reserved
*    8- Ports group0 configuration registers (port0 through port5),
*       LEDs interface0 configuration registers and Master SMI
*       interface0 registers
*    9- Ports group1 configuration registers (port6 through port11) and
*       LEDs interface0.
*    10-Ports group2 configuration registers (port12 through port17),
*       LEDs interface1 configuration registers and Master SMI interface1
*       registers
*    11-Ports group3 configuration registers (port18through port23) and
*       LEDs interface1
*    12-MAC Table Memory
*    13-Internal Buffers memory Bank0 address space
*    14-Internal Buffers memory Bank1 address space
*    15-Buffers memory block configuration registers
*    20-VLAN Table configuration registers and VLAN Table address space.
*    21-Tri-Speed Ports MAC, CPU Port MAC, and 1.25 Gbps SERDES
*           Configuration Registers Map Table.
*    22-Pre-Egress registers
*    23-PCL registers and TCAM memory space
*    24-Policer registers and meters memory space
*    25-MLL Engine registers table map
*    26-63  Reserved.
*
***********************************************************************/
/**
* @struct CHT3_GLOBAL_MEM
 *
 * @brief Global Registers, TWSI, CPU port, SDMA and PCI internal registers.
*/
typedef struct{

    /** @brief : Global registers number.
     *  globReg    : Global registers array with size of globRegNum.
     *  globRegNum = 0xfff / 4 + 1
     *  sdmaReg - Registers with address mask 0x1FFFF000 pattern 0x00002000
     */
    GT_U32 globRegNum;

    SMEM_REGISTER * globReg;

    /** @brief : SDMA register number.
     *  sdmaReg    : SDMA register array with size of sdmaRegNum.
     *  twsiRegNum  : TWSI configuration register number.
     *  twsiReg    : TWSI registers array with the size of twsiRegNum.
     *  pexReg    : PEX registers array with the size of pexRegNum.
     *  epclConfigTblReg: Egress PCL configuration table with size of epclConfigTblRegNum
     *  macSaTblReg  : Port/VLAN MAC SA Table with size macSaTblRegNum
     *  Comments:
     *  For all registers from this struct Bits 23:28 == 0
     *  globReg  - Registers with address mask 0x1FFFF000 pattern 0x00000000
     *  sdmaRegNum = 150
     *  twsiIntRegs - Registers with address mask 0x1FFFFF00 pattern 0x00400000
     *  twsiIntRegs = 0x1C
     *  pexReg   - Registers with address mask 0x1FFFFF00 pattern 0x00001000
     */
    GT_U32 sdmaRegNum;

    SMEM_REGISTER * sdmaReg;

    GT_U32 twsiIntRegsNum;

    SMEM_REGISTER * twsiIntRegs;

    /** @brief = 0xfff / 4 + 1
     *  epclConfigTblReg - Registers with address mask 0xFFFFF000 pattern 0x00018000
     */
    GT_U32 pexRegNum;

    SMEM_REGISTER * pexReg;

    /** @brief = 4160
     *  macSaTblReg - MAC SRC address table memory pointer
     */
    GT_U32 epclConfigTblRegNum;

    SMEM_REGISTER * epclConfigTblReg;

    GT_U32 macSaTblRegNum;

    SMEM_REGISTER * macSaTblReg;

} CHT3_GLOBAL_MEM;

/**
* @struct CHT3_EGR_MEM
 *
 * @brief Egress, Transmit Queue and VLAN Configuration Register Map Table,
 * Port TxQ Configuration Register, Hyper.GStack Ports MIB Counters.
*/
typedef struct{

    /** @brief : Common registers number.
     *  egrGenReg    : Common registers number, index is port and group.
     *  txqInternRegNum : TxQ interanal registers number.
     *  txqInternReg  : TxQ interanal registers.
     */
    GT_U32 egrGenRegNum;

    SMEM_REGISTER * egrGenReg;

    GT_U32 txqInternalRegNum;

    SMEM_REGISTER * txqInternalReg[4];

    /** @brief = 2032
     *  portWdReg  - Registers with address mask 0x1FFF0000 pattern 0x01A80000
     */
    GT_U32 trQueueRegNum;

    SMEM_REGISTER * trQueueReg;

    /** @brief : Port Watchdog Configuration Register Number.
     *  portWdReg    : Port Watchdog Configuration Register.
     *  portWdRegNum = 64
     *  egrMibCntReg - Registers with address mask 0x1FFFFF00 pattern 0x01B40100
     *  egrMibCntReg = 120
     *  egrRateShpReg- Registers with address mask 0x1FF00000 pattern 0x01A00000
     */
    GT_U32 portWdRegNum;

    SMEM_REGISTER * portWdReg;

    /** @brief : Egress MIB counter number.
     *  egrMibCntReg  : Egress MIB counter.
     */
    GT_U32 egrMibCntRegNum;

    SMEM_REGISTER * egrMibCntReg;

    /** @brief : Hyper.GStack Ports MIB Counters number.
     *  hprMibCntReg  : Hyper.GStack Ports MIB Counters.
     *  egrRateShpRegNum: Egress rate shape register number.
     *  egrRateShpReg  : Egress rate shape register.
     *  hprMibCntRegNum = 64
     *  stackTcRemapMapReg - Registers with address mask 0x1FFF0000 pattern 0x01E80000
     *  hprMibCntRegNum = 8
     *  xsmiReg   - Registers with address mask 0x1FFF0000 pattern 0x01CC0000
     */
    GT_U32 hprMibCntRegNum;

    SMEM_REGISTER * hprMibCntReg[4];

    GT_U32 tailDropRegNum;

    SMEM_REGISTER * tailDropReg;

    /** @brief = 512(8 TC 64 ports)
     *  hprMibCntReg - Registers with address mask 0x1FF00000 pattern 0x01C00000
     */
    GT_U32 egrRateShpRegNum;

    SMEM_REGISTER * egrRateShpReg[9];

    /** @brief : Master XSMI Interface Register Map Table number.
     *  xsmiReg     : Master XSMI Interface Register Map Table
     *  Comments:
     *  For all registers from this struct Bits 23:28 == 3
     *  egrGenReg  - Registers with address mask 0x1FF00000 pattern 0x01800000
     *  genRegNum = 308
     *  txQInternReg - Registers with address mask 0x1FF00000 pattern 0x04840000,
     *  0x04880000, 0x048C0000, 0x04900000
     *  txQInternRegNum = 512(8 TC 64 ports)
     *  egrFilterReg - Registers with address mask 0x1FFFF000 pattern 0x01A40080
     *  xsmiRegNum = 2
     *  txSniffCntr = Register of tx sniffering
     */
    GT_U32 xsmiRegNum;

    SMEM_REGISTER * xsmiReg;

    GT_U32 egrStcTblRegNum;

    SMEM_REGISTER * egrStcTblReg;

    GT_U32 secondaryTargetPortMapRegNum;

    SMEM_REGISTER * secondaryTargetPortMapReg;

    GT_U32 stackTcRemapRegNum;

    SMEM_REGISTER * stackTcRemapMapReg;

} CHT3_EGR_MEM;

/**
* @struct CHT3_BRDG_MNG_MEM
 *
 * @brief Describe a device's Bridge Management registers memory object.
*/
typedef struct{

    GT_U32 bridgeGenRegsNum;

    SMEM_REGISTER * bridgeGenReg;

} CHT3_BRDG_MNG_MEM;

/**
* @struct CHT3_BUF_MNG_MEM
 *
 * @brief Describe a device's buffer management registers memory object and
 * egress memory.
*/
typedef struct{

    /** @brief : General buffer management registers number.
     *  bmReg        : General buffer management registers.
     *  bmRegNum = 196
     *  bmCntrReg   - Registers with address mask 0xffff0000 pattern 0x03010000
     *  bmLLRegNum = 4096
     */
    GT_U32 bmRegNum;

    SMEM_REGISTER * bmReg;

    /** @brief : Buffer management controlled registers number.
     *  bmCntrReg      : Buffer management controlled registers.
     *  Comments:
     *  For all registers from this struct Bits 23:28 == 6
     *  bmReg     - Registers with address mask 0xfffff000 pattern 0x03000000
     */
    GT_U32 bmCntrRegNum;

    SMEM_REGISTER * bmCntrReg;

} CHT3_BUF_MNG_MEM;

/**
* @struct CHT3_GOP_CONF_MEM
 *
 * @brief LEDs, Tri-Speed Ports MIB Counters and Master SMI
 * Configuration Registers.
*/
typedef struct{

    /** @brief : Group configuration registers numbers.
     *  portGroupReg    : Group configuration registers.
     *  portGroupRegNum = 512
     *  xgPortGroupRegNum[]
     *  - Registers with address mask 0x0000F000 pattern 0x4000
     */
    GT_U32 portGroupRegNum;

    SMEM_REGISTER * portGroupReg[4];

    /** @brief : XG ports group configuration registers numbers.
     *  xgPortGroupReg   : XG ports group configuration registers.
     *  xgPortGroupRegNum = 16
     *  macMibCountReg[]
     *  - Registers with address mask 0x000F0000 pattern 0x10000
     */
    GT_U32 xgPortGroupRegNum;

    SMEM_REGISTER * xgPortGroupReg[4];

    /** @brief : MAC MIB counters registers numbers.
     *  macMibCountReg   : MAC MIB counters registers.
     *  xgMacMibCountRegNum: XG ports MAC MIB counters registers numbers.
     *  xgMacMibCountReg  : XG ports MAC MIB counters registers.
     *  Comments:
     *  For all registers from this struct Bits 23:28 == 8,9,10,11
     *  portGroupReg[]
     *  - Registers with address mask 0x0000F000 pattern 0x4000
     *  macMibCountRegNum = 224
     *  xgMacMibCountReg[] -
     *  - Registers with address mask 0x000F0000 pattern 0x10000
     */
    GT_U32 macMibCountRegNum;

    SMEM_REGISTER * macMibCountReg[4];

    /** = 64 */
    GT_U32 xgMacMibCountRegNum;

    SMEM_REGISTER * xgMacMibCountReg[4];

} CHT3_GOP_CONF_MEM;


/**
* @struct CHT3_BANKS_MEM
 *
 * @brief Describe a buffers memory banks data register.
*/
typedef struct{

    SMEM_REGISTER bankWriteReg[2];

    /** @brief : Buffers Memory Address Space Bank number.
     *  bankMemReg     : Buffers Memory Address Space Bank.
     *  Comments:
     *  For all registers from this struct Bits 23:28 == 13
     *  bankMemRegNum = 98304
     *  buffMemReg   -  Register with address 0x06800000.
     */
    GT_U32 bankMemRegNum;

    SMEM_REGISTER * bankMemReg[2];

} CHT3_BANKS_MEM;


/**
* @struct CHT3_CENTRAL_CNT_MEM
 *
 * @brief Centralized counters.
*/
typedef struct{

    GT_U32 cncCntrsRegNum;

    SMEM_REGISTER * cncCntrsReg;

    GT_U32 cncCntrsPerBlockRegNum;

    SMEM_REGISTER * cncCntrsPerBlockReg;

    GT_U32 cncCntrsTblNum;

    SMEM_REGISTER ** cncCntrsTbl;

} CHT3_CENTRAL_CNT_MEM;

/**
* @struct CHT3_TRI_SPEED_PORTS_MEM
 *
 * @brief Describe a Tri-Speed Ports MAC, CPU Port MAC, and 1.25 Gbps SERDES
 * Configuration Registers Map Table, Hyper.GStack Ports MAC and
 * XAUI PHYs Configuration Registers Map Table.
*/
typedef struct{

    /** @brief : Ports registers number.
     *  triSpeedPortsReg    : Ports registers.
     *  Comments:
     *  For all registers from this struct Bits 23:28 == 21
     *  triSpeedPortsReg : Registers with address mask 0xFFFF0000 pattern 0x0A800000
     *  triSpeedPortsRegNum = 16383
     */
    GT_U32 triSpeedPortsRegNum;

    SMEM_REGISTER * triSpeedPortsReg;

} CHT3_TRI_SPEED_PORTS_MEM;


/**
* @struct CHT3_XG_CTRL_PORTS_MEM
 *
 * @brief Describe XG port MAC control and status registers and port interrupt
 * and mask registers
*/
typedef struct{

    /** @brief : XG ports registers number.
     *  xgPortsReg    : XG ports registers.
     *  Comments:
     *  For all registers from this struct Bits 23:28 == 17
     *  xgPortsReg : Registers with address mask 0xFFFFFF00 pattern 0x08800000
     *  xgPortsRegNum = 0xff / 4 + 1
     */
    GT_U32 xgPortsRegNum;

    SMEM_REGISTER * xgPortsReg;

} CHT3_XG_CTRL_PORTS_MEM;


/**
* @struct CHT3_PCL_MEM
 *
 * @brief PCL registers(ingress and egress) and TCAM memory space.
*/
typedef struct{

    GT_U32 ingrPclCommonRegNum;

    SMEM_REGISTER * ingrPclCommonReg;

    GT_U32 pclTcamInternalConfigRegNum;

    SMEM_REGISTER * pclTcamInternalConfigReg;

    GT_U32 pclTcamTestConfigRegNum;

    SMEM_REGISTER * pclTcamTestConfigReg;

    GT_U32 ingrPclConfigTableRegNum;

    SMEM_REGISTER * ingrPclConfigTableReg;

    GT_U32 policyActionTableRegNum;

    SMEM_REGISTER * policyActionTableReg;

    GT_U32 policyEccRegNum;

    SMEM_REGISTER * policyEccReg[2];

    GT_U32 policyTcamRegNum;

    SMEM_REGISTER * policyTcamReg;

    GT_U32 vlanTranslationTableRegNum;

    SMEM_REGISTER * vlanTranslationTableReg;

    GT_U32 portVlanQoSConfigRegNum;

    SMEM_REGISTER * portVlanQoSConfigReg;

    GT_U32 portProtVidQoSConfigRegNum;

    SMEM_REGISTER * portProtVidQoSConfigReg;

} CHT3_PCL_MEM;


/**
* @struct CHT3_POLICER_MEM
 *
 * @brief Policer registers and meters memory space.
*/
typedef struct{

    GT_U32 policerRegNum;

    SMEM_REGISTER * policerReg;

    GT_U32 policerTimerRegNum;

    SMEM_REGISTER * policerTimerReg;

    GT_U32 policerDescrSampleRegNum;

    SMEM_REGISTER * policerDescrSampleReg;

    GT_U32 policerTblRegNum;

    SMEM_REGISTER * policerTblReg;

    GT_U32 policerCntTblRegNum;

    SMEM_REGISTER * policerCntTblReg;

    /** @brief = 1024 4
     *  policerQosRmTblReg : Policers QoS Remarking and Initial DP Table Entry<0<=n<128>
     *  with address mask 0xFFFF0000 pattern 0x0C080000
     *  policerQosRmTblRegNum = 128
     *  policerMngCntTblReg: Management Counters Table Entry <0<=n<12>
     *  with address mask 0xFFFF0000 pattern 0x0C0C0000
     *  policerQosRmTblRegNum = 122
     */
    GT_U32 policerQosRmTblRegNum;

    SMEM_REGISTER * policerQosRmTblReg;

    GT_U32 policerMngCntTblRegNum;

    SMEM_REGISTER * policerMngCntTblReg;

} CHT3_POLICER_MEM;

/**
* @struct CHT3_PCI_MEM
 *
 * @brief PCI Registers.
*/
typedef struct{

    /** @brief : PCI registers number.
     *  pciReg     : PCI registers
     *  Comments:
     *  pciRegNum = 128 4
     */
    GT_U32 pciRegNum;

    SMEM_REGISTER * pciReg[4];

} CHT3_PCI_MEM;

/**
* @struct CHT3_PHY_MEM
 *
 * @brief PHY related Registers.
*/
typedef struct{

    GT_U32 PhyIdieeeXauiRegNum;

    GT_U32 PhyIdextXauiRegNum;

    SMEM_PHY_REGISTER *  PhyId0ieeeXauiReg[3];

    SMEM_PHY_REGISTER *  PhyId0extXauiReg[3];

    SMEM_PHY_REGISTER *  PhyId1ieeeXauiReg[3];

    SMEM_PHY_REGISTER *  PhyId1extXauiReg[3];

    SMEM_PHY_REGISTER *  PhyId2ieeeXauiReg[3];

    SMEM_PHY_REGISTER *  PhyId2extXauiReg[3];

} CHT3_PHY_MEM;


/**
* @struct CHT3_MLL_MEM
 *
 * @brief MLL engine Configuration registers map table.
*/
typedef struct{

    /** @brief : MLL global and configuration Registers number
     *  mllConfReg         : MLL global and configuration Registers
     *  mllConfRegNum : Registers with address mask 0xFFFFF000 pattern 0x0C800000
     *  pclConfRegNum = 1024
     */
    GT_U32 mllConfRegNum;

    SMEM_REGISTER * mllConfReg;

    /** @brief : MLL Entry Registers number
     *  mllEntryReg        : MLL Entry Registers
     *  Comments:
     *  For all registers from this struct Bits 23:28 == 19
     *  mllEntryRegNum : Registers with address mask 0xFFFF0000 pattern 0x0C880000
     *  pclConfRegNum = 4096
     */
    GT_U32 mllEntryRegNum;

    SMEM_REGISTER * mllEntryReg;

} CHT3_MLL_MEM;


/**
* @struct CHT3_ROUTER_MEM
 *
 * @brief Router Configuration Registers and Tables.
*/
typedef struct{

    /** @brief : Global and Configuration registers number.
     *  configReg      : Global and Configuration registers.
     */
    GT_U32 configRegNum;

    SMEM_REGISTER *         configReg;

    GT_U32 ageRegNum;

    SMEM_REGISTER *         ageReg;

    /** @brief = 23
     *  tcamReg   : Registers with address mask 0xFFF00000 pattern 0x02A00000
     */
    GT_U32 tcamTestRegNum;

    SMEM_REGISTER *         tcamTestReg;

    /** @brief : TCAM Router registers number.
     *  tcamReg       : TCAM Router registers.
     *  tcamRegNum = 20477
     *  actionTblReg : Registers with address mask 0xFFFF0000 pattern 0x02900000
     */
    GT_U32 tcamRegNum;

    SMEM_REGISTER *         tcamReg;

    /** @brief : LTT and TT Action Table Entry number.
     *  actionTblReg     : LTT and TT Action Table Entry.
     *  actionTblRegNum = 4096
     *  nextHopTblReg : Registers with address mask 0xFFFF0000 pattern 0x02B00000
     *  actionTblRegNum = 16380
     *  vrfTblReg :  Registers with address mask 0xFFFF0000 pattern 0x02B00000
     */
    GT_U32 actionTblRegNum;

    SMEM_REGISTER *         actionTblReg;

    /** @brief : Unicast/Multicast Router Next Hop Entry Number.
     *  nextHopTblReg    : Unicast/Multicast Router Next Hop Entry.
     *  Comments:
     *  For all registers from this struct Bits 23:28 == 5
     *  configReg  : Registers with address mask 0xFFFFF000 pattern 0x02800000
     *  portsRegNum = 1152
     *  ageReg    : Registers with address mask 0xFFFFF000 pattern 0x02801000
     *  portsRegNum = 1000
     *  tcamTestReg : Registers with address mask 0xFFF00000 pattern 0x02802000
     */
    GT_U32 nextHopTblRegNum;

    SMEM_REGISTER *         nextHopTblReg;

    /** = 1024 */
    GT_U32 vrfTblRegNum;

    SMEM_REGISTER *         vrfTblReg;

} CHT3_ROUTER_MEM;

/**
* @struct CHT3_VLAN_TBL_MEM
 *
 * @brief VLAN Table, Multicast Groups Table, and Span State Group Table.
*/
typedef struct{

    /** @brief : VLAN configuration registers number
     *  vlanCnfReg   : VLAN configuration registers
     */
    GT_U32 vlanCnfRegNum;

    SMEM_REGISTER * vlanCnfReg;

    /** @brief : Registers number for Vlan Table
     *  vlanTblReg   : Vlan Table Registers.
     *  vlanTblRegNum = 4
     *  vlanTblReg :  VLAN<n> Entry (0<=n<4096) address mask 0xFFF00000 pattern 0x0A400000
     *  vlanTblRegNum = 4096
     *  mcstTblReg :  Multicast Group<n> Entry (0<=n<4096) address mask 0xFFFF0000
     *  pattern 0x0A200000 , mcstTblRegNum = 4096
     *  spanTblReg :  Span State Group<n> Entry (0<=n<256) address mask 0xFFFF0000
     *  pattern 0x0A100000 , spanTblRegNum = 256
     *  vrfTblReg :  VRF Id table
     *  0xa300000 + n0x4: where n (0-4095) represents Entry
     */
    GT_U32 vlanTblRegNum;

    SMEM_REGISTER * vlanTblReg;

    /** @brief : Multicast Group configuration table number.
     *  mcstTblReg   : Multicast Group configuration table.
     */
    GT_U32 mcstTblRegNum;

    SMEM_REGISTER * mcstTblReg;

    /** @brief : STP configuration table number.
     *  spanTblReg   : STP configuration table .
     *  Comments:
     *  For all registers from this struct Bits 23:28 == 20
     *  vlanCnfReg :  Registers with address mask 0xFF000000 pattern 0x0A0000000
     */
    GT_U32 spanTblRegNum;

    SMEM_REGISTER * spanTblReg;

    GT_U32 vrfTblRegNum;

    SMEM_REGISTER * vrfTblReg;

} CHT3_VLAN_TBL_MEM;

/**
* @struct CHT3_XG_MIB_CNTRS_MEM
 *
 * @brief XG Ports MIB counters registers memory
*/
typedef struct{

    /** @brief XG MIB Counters registers memory
     *  mibReg   - XG MIB Counters registers number
     *  Comments:
     */
    GT_U32 mibRegNum;

    SMEM_REGISTER * mibReg;

} CHT3_XG_MIB_CNTRS_MEM;
/**
* @struct CHT3_PLUS_EXTRA_MEM
 *
 * @brief extra memory for the cheetah3+
*/
typedef struct{

    /** @brief number of registers in extra memory
     *  extraRegPtr   - pointer to the memory
     *  Comments:
     */
    GT_U32 extraRegNum;

    SMEM_REGISTER * extraRegPtr;

} CHT3_PLUS_EXTRA_MEM;

/** xCat revision A1 and above memory
 *
 *      vlanTblUnitMem : VLAN and Multicast Group and Span State Group Tables
 *      brdgMngUnitMem : Bridge management registers
 *      policerMemIngress0UnitMem : Ingress Policer memory
 *      policerMemIngress1UnitMem : Ingress Policer memory
 *      policerMemEgressUnitMem : Egress Policer memory
 *      mllMemUnitMem : MLL memory
 *      pclMemUnitMem : PCL memory
 *      pclMemEgrUnitMem : PCL Egress memory
 *      egrMemUnitMem : Egress Memory
 *      eqUnitUnitMem : EQ memory
 *      ipvxTccUnitMem : IPVX   memory
 *      haAndEpclUnitMem : HA and EPCL memory
 *      bufMngMemUnitMem : Buffer Management memory
 *      ttiMemUnitMem : TTI
 *      dfxUnitMem  :   DFX Memory
 *
******************************************************************************/
typedef struct {
    SMEM_UNIT_CHUNKS_STC            vlanTblUnitMem;
    SMEM_UNIT_CHUNKS_STC            brdgMngUnitMem;
    SMEM_UNIT_CHUNKS_STC            policerMemIngress0UnitMem;
    SMEM_UNIT_CHUNKS_STC            policerMemIngress1UnitMem;
    SMEM_UNIT_CHUNKS_STC            policerMemEgressUnitMem;
    SMEM_UNIT_CHUNKS_STC            mllMemUnitMem;
    SMEM_UNIT_CHUNKS_STC            pclMemUnitMem;
    SMEM_UNIT_CHUNKS_STC            pclTccMemUnitMem;
    SMEM_UNIT_CHUNKS_STC            pclMemEgrUnitMem;
    SMEM_UNIT_CHUNKS_STC            egrMemUnitMem;
    SMEM_UNIT_CHUNKS_STC            eqUnitUnitMem;
    SMEM_UNIT_CHUNKS_STC            ipvxTccUnitMem;
    SMEM_UNIT_CHUNKS_STC            haAndEpclUnitMem;
    SMEM_UNIT_CHUNKS_STC            bufMngMemUnitMem;
    SMEM_UNIT_CHUNKS_STC            ttiMemUnitMem;
    SMEM_UNIT_CHUNKS_STC            dfxUnitMem;
} SMEM_XCAT_DEV_EXTRA_MEM_INFO;


/**
* @struct SMEM_CHT3_DEV_MEM_INFO
 *
 * @brief Describe a device's memory object in the simulation.
*/
typedef struct{

    SMEM_CHT_DEV_COMMON_MEM_INFO common;

    /** : Global configuration registers. */
    CHT3_GLOBAL_MEM globalMem;

    /** : Egress registers. */
    CHT3_EGR_MEM egrMem;

    /** : Bridge management registers. */
    CHT3_BRDG_MNG_MEM brdgMngMem;

    /** : Buffers management registers. */
    CHT3_BUF_MNG_MEM bufMngMem;

    /** @brief : GOP registers.
     *  macFbdMem    : MAC FDB table.
     */
    CHT3_GOP_CONF_MEM gopCnfMem;

    CHT_MAC_FDB_MEM macFdbMem;

    /** : buffers memory banks data register */
    CHT3_BANKS_MEM banksMem;

    CHT3_CENTRAL_CNT_MEM centralCntMem;

    CHT3_TRI_SPEED_PORTS_MEM triSpeedPortsMem;

    CHT3_XG_CTRL_PORTS_MEM xgCtrlPortsMem;

    /** : PCL engine registers and tables. */
    CHT3_PCL_MEM pclMem;

    /** : Policer engine registers. */
    CHT3_POLICER_MEM policerMem;

    /** @brief : Pci internal memory.
     *  phyMem     : PHY related registers.
     *  auqMem     : Address update message
     *  fuqMem     : fdb upload message
     *  ipMem      : ARP table and TT.
     */
    CHT3_PCI_MEM pciMem;

    /** : Router engine registers and tables. */
    CHT3_ROUTER_MEM routerMem;

    /** @brief : MLL engine registers.
     *  policyEngIntMem : Policy engine interrupt registers
     *  haAndEpclUnitMem: HA and EPCL unit
     */
    CHT3_MLL_MEM mllMem;

    /** @brief : VLAN tables.
     *  triSpeedPortsMem: Tri-Speed Ports MAC registers.
     *  preegressMem  : Pre-Egress registers.
     */
    CHT3_VLAN_TBL_MEM vlanTblMem;

    CHT3_XG_MIB_CNTRS_MEM xgMibCntrsMem;

    /** @brief : cheetah3+ extra memory
     *  Comments:
     */
    CHT3_PLUS_EXTRA_MEM ch3pExtraMem;

    SMEM_UNIT_CHUNKS_STC haAndEpclUnitMem;

    /** egress) unit */
    SMEM_UNIT_CHUNKS_STC eqUnitMem;

    /** : ingress pcl Tcc Unit Memory (xcat) */
    SMEM_UNIT_CHUNKS_STC ipclTccUnitMem;

    /** : ipvx Tcc Unit Memory (xcat) */
    SMEM_UNIT_CHUNKS_STC ipvxTccUnitMem;

    /** : a unit that unified the XG,GE,combo ports (xcat) */
    SMEM_UNIT_CHUNKS_STC uniphySerdesUnitMem;

    /** : xCat A1 and above */
    SMEM_XCAT_DEV_EXTRA_MEM_INFO xCatExtraMem;

} SMEM_CHT3_DEV_MEM_INFO;

/**
* @internal smemCht3Init function
* @endinternal
*
* @brief   Init memory module for a Cht device.
*
* @param[in] deviceObj                - pointer to device object.
*/
void smemCht3Init
(
    IN SKERNEL_DEVICE_OBJECT * deviceObj
);

/**
* @internal smemCht3Init2 function
* @endinternal
*
* @brief   Init memory module for a device - after the load of the default
*         registers file
* @param[in] deviceObj                - pointer to device object.
*/
void smemCht3Init2
(
    IN SKERNEL_DEVICE_OBJECT * deviceObj
);

/**
* @internal smemCht3TableInfoSet function
* @endinternal
*
* @brief   set the table info for the device --> fill devObjPtr->tablesInfo
*
* @param[in] devObjPtr                - device object PTR.
*/
void smemCht3TableInfoSet
(
    IN         SKERNEL_DEVICE_OBJECT * devObjPtr
);

/**
* @internal smemXcatA1TableInfoSet function
* @endinternal
*
* @brief   set the table info for the device --> fill devObjPtr->tablesInfo
*
* @param[in] devObjPtr                - device object PTR.
*/
void smemXcatA1TableInfoSet
(
    IN         SKERNEL_DEVICE_OBJECT * devObjPtr
);

/**
* @internal smemXCatA1UnitPex function
* @endinternal
*
* @brief   Allocate address type specific memories -- for the PEX/MBus unit
*
* @param[in] devObjPtr                - pointer to device memory object.
* @param[in,out] unitPtr                  - pointer to the unit chunk
* @param[in] pexBaseAddr              - PCI/PEX/MNus unit base address
*/
void smemXCatA1UnitPex
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SMEM_UNIT_CHUNKS_STC  * unitPtr,
    IN GT_U32 pexBaseAddr
);

/* MAC Counters address mask */
#define     SMEM_CHT3_COUNT_MSK_CNS              0xff8ff000


/* policer management counters mask */
/* 0x40 between sets , 0x10 between counters */
/* 0x00 , 0x10 , 0x20 , 0x30 */
/* 0x40 , 0x50 , 0x60 , 0x70 */
/* 0x80 , 0x90 , 0xa0 , 0xb0 */
#define POLICER_MANAGEMENT_COUNTER_MASK_CNS         0xFFFFFF0F
/* policer management counters base address */
#define POLICER_MANAGEMENT_COUNTER_ADDR_CNS         0x00000500


/* CH3 - policer management counters mask */
/* 0x20 between sets , 0x8 between counters */
/* 0x00 , 0x08 , 0x10 , 0x18 */
/* 0x20 , 0x28 , 0x30 , 0x38 */
/* 0x40 , 0x48 , 0x50 , 0x58 */
#define CH3_POLICER_MANAGEMENT_COUNTER_MASK_CNS         0xFFFFFF87



ACTIVE_WRITE_FUNC_PROTOTYPE_MAC(smemCht3ActiveWritePclAction         );
ACTIVE_WRITE_FUNC_PROTOTYPE_MAC(smemCht3ActiveWritePolicerTbl        );
ACTIVE_WRITE_FUNC_PROTOTYPE_MAC(smemCht3ActiveWriteCncFastDumpTrigger);
ACTIVE_WRITE_FUNC_PROTOTYPE_MAC(smemCht3ActiveWriteRouterAction         );
ACTIVE_WRITE_FUNC_PROTOTYPE_MAC(smemCht3ActiveWriteFDBGlobalCfgReg      );
ACTIVE_WRITE_FUNC_PROTOTYPE_MAC(smemXcatA1ActiveWriteVlanTbl            );
ACTIVE_WRITE_FUNC_PROTOTYPE_MAC(smemXcatActiveWriteMacModeSelect        );
ACTIVE_WRITE_FUNC_PROTOTYPE_MAC(smemXCatActiveWritePolicerTbl           );
ACTIVE_WRITE_FUNC_PROTOTYPE_MAC(smemXCatActiveWriteIPFixTimeStamp       );
ACTIVE_WRITE_FUNC_PROTOTYPE_MAC(smemXCatActiveWriteLogTargetMap         );
ACTIVE_WRITE_FUNC_PROTOTYPE_MAC(smemCht3ActiveWriteCncInterruptsMaskReg );
ACTIVE_WRITE_FUNC_PROTOTYPE_MAC(smemXCatActiveWritePolicyTcamConfig_0   );
ACTIVE_WRITE_FUNC_PROTOTYPE_MAC(smemXCatActiveWritePolicerMemoryControl );
ACTIVE_WRITE_FUNC_PROTOTYPE_MAC(smemXCatActiveWriteIplr0Tables                    );
ACTIVE_WRITE_FUNC_PROTOTYPE_MAC(smemXCatActiveWriteIplr1Tables                    );

ACTIVE_WRITE_FUNC_PROTOTYPE_MAC(smemXCatActiveWriteBridgeGlobalConfig2Reg);

ACTIVE_WRITE_FUNC_PROTOTYPE_MAC(smemXCatActiveWriteEgressFilterVlanMap);
ACTIVE_WRITE_FUNC_PROTOTYPE_MAC(smemXCatActiveWriteEgressFilterVlanMember);

ACTIVE_WRITE_FUNC_PROTOTYPE_MAC(smemXCatActiveWriteEqGlobalConfigReg);
ACTIVE_WRITE_FUNC_PROTOTYPE_MAC(smemXCatActiveWriteIpclGlobalConfigReg);
ACTIVE_WRITE_FUNC_PROTOTYPE_MAC(smemXCatActiveWriteTtiInternalMetalFix);
ACTIVE_WRITE_FUNC_PROTOTYPE_MAC(smemXCatActiveWriteTtiGlobalConfigReg);
ACTIVE_WRITE_FUNC_PROTOTYPE_MAC(smemXCatActiveWriteHaGlobalConfigReg);
ACTIVE_WRITE_FUNC_PROTOTYPE_MAC(smemXCatActiveWriteMllGlobalConfigReg);

/*l2 mll*/
ACTIVE_WRITE_FUNC_PROTOTYPE_MAC(smemXCatActiveWriteL2MllVidxEnable);
ACTIVE_WRITE_FUNC_PROTOTYPE_MAC(smemXCatActiveWriteL2MllPointerMap);

/*read active memory*/
ACTIVE_READ_FUNC_PROTOTYPE_MAC(smemCht3ActiveCncBlockRead            );
ACTIVE_READ_FUNC_PROTOTYPE_MAC(smemCht3ActiveCncWrapAroundStatusRead );
ACTIVE_READ_FUNC_PROTOTYPE_MAC(smemXCatActiveReadIPFixNanoTimeStamp  );
ACTIVE_READ_FUNC_PROTOTYPE_MAC(smemXCatActiveReadIPFixSecLsbTimeStamp);
ACTIVE_READ_FUNC_PROTOTYPE_MAC(smemXCatActiveReadIPFixSecMsbTimeStamp);
ACTIVE_READ_FUNC_PROTOTYPE_MAC(smemXCatActiveReadIPFixSampleLog);
ACTIVE_READ_FUNC_PROTOTYPE_MAC(smemXCatActiveReadPolicerManagementCounters);
ACTIVE_READ_FUNC_PROTOTYPE_MAC(smemXCatActiveReadIeeeMcConfReg);
ACTIVE_READ_FUNC_PROTOTYPE_MAC(smemXCatActiveReadIplr0Tables);
ACTIVE_READ_FUNC_PROTOTYPE_MAC(smemXCatActiveReadIplr1Tables);

/* PCL/Router TCAM BIST done simulation */
ACTIVE_READ_FUNC_PROTOTYPE_MAC(smemCht3ActiveReadTcamBistConfigAction);

extern SKERNEL_DEVICE_ROUTE_TCAM_INFO_STC  xcatRoutTcamInfo;

void smemLion2RegsInfoSet_GOP_SERDES
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SMEM_CHT_PP_REGS_ADDR_STC        * regAddrDbPtr,
    IN GT_U32                   s/*serdes*/,
    IN GT_U32                   minus_s/*compensation serdes*/,
    IN GT_U32                   extraOffset,
    IN GT_BIT                   isGopVer1
);

void smemLion2RegsInfoSet_GOP_gigPort
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SMEM_CHT_PP_REGS_ADDR_STC        * regAddrDbPtr,
    IN GT_U32                   p/*port*/,
    IN GT_U32                   minus_p/*compensation port*/,
    IN GT_U32                   extraOffset,
    IN GT_BIT                   isGopVer1
);

void smemLion2RegsInfoSet_GOP_XLGIP
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SMEM_CHT_PP_REGS_ADDR_STC        * regAddrDbPtr,
    IN GT_U32                   p/*port*/,
    IN GT_U32                   minus_p/*compensation port*/,
    IN GT_U32                   extraOffset
);

void smemLion2RegsInfoSet_GOP_MPCSIP
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SMEM_CHT_PP_REGS_ADDR_STC        * regAddrDbPtr,
    IN GT_U32                   p/*port*/,
    IN GT_U32                   minus_p/*compensation port*/,
    IN GT_U32                   extraOffset,
    IN GT_BIT                   isGopVer1
);

void smemLion2RegsInfoSet_GOP_PTP
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SMEM_SIP5_PP_REGS_ADDR_STC       * regAddrDbPtr,
    IN GT_U32                   p/*port*/,
    IN GT_U32                   minus_p/*compensation port*/,
    IN GT_U32                   extraOffset
);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif  /* __smemCht3h */


