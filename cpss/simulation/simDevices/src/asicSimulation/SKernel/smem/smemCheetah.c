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
* @file smemCheetah.c
*
* @brief This is API implementation for Cheetah memories.
*
* @version   186
********************************************************************************
*/
#include <os/simTypes.h>
#include <asicSimulation/SKernel/smem/smemCheetah.h>
#include <asicSimulation/SKernel/smem/smemCheetah2.h>
#include <asicSimulation/SKernel/smem/smemCheetah3.h>
#include <asicSimulation/SKernel/smem/smemLion.h>
#include <asicSimulation/SKernel/smem/smemXCat2.h>
#include <asicSimulation/SKernel/cheetahCommon/sregCheetah.h>
#include <asicSimulation/SKernel/sfdb/sfdbCheetah.h>
#include <asicSimulation/SKernel/skernel.h>
#include <asicSimulation/SKernel/smain/smain.h>
#include <asicSimulation/SKernel/suserframes/snetCheetahEgress.h>
#include <asicSimulation/SKernel/suserframes/snetCheetahL2.h>
#include <asicSimulation/SKernel/suserframes/snet.h>
#include <asicSimulation/SKernel/suserframes/snetCheetahTxQ.h>
#include <asicSimulation/SLog/simLog.h>
#include <asicSimulation/SCIB/scib.h>

/*#define TRACE_CHECKED_UNITS*/
extern GT_U32   debugModeByPassFatalErrorOnMemNotExists;

#if 0
static void gopMtiPrint
(
    IN SKERNEL_DEVICE_OBJECT *devObjPtr,
    IN GT_U32 portNum
    )
{
    GT_U32 channelIdx;

    channelIdx = (portNum / 8);

    simWarningPrintf("portStatus[%2d]            [0x%8.8x]\n", portNum, SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI[portNum].MTI_EXT_PORT.portStatus);
    simWarningPrintf("globalResetControl[%2d]    [0x%8.8x]\n", channelIdx, SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI_EXTERNAL[channelIdx].MTI_GLOBAL.globalResetControl);
    simWarningPrintf("MTI_STATS.control[%2d]     [0x%8.8x]\n", channelIdx, SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI_EXTERNAL[channelIdx].MTI_STATS.control);
    simWarningPrintf("commandConfig400[%2d]      [0x%8.8x]\n", portNum, SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_high_speed[portNum].MTI400_MAC.commandConfig);
    simWarningPrintf("commandConfig64[%2d]       [0x%8.8x]\n", portNum, SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI[portNum].MTI_WRAPPER.MTI64_MAC.commandConfig);
    simWarningPrintf("PCS_10254050.control1[%2d] [0x%8.8x]\n", portNum, SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI[portNum].MTI_WRAPPER.PCS_10254050.control1);
    simWarningPrintf("PCS_10TO100.control1[%2d]  [0x%8.8x]\n", portNum, SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI[portNum].MTI_WRAPPER.PCS_10TO100.control1);
    simWarningPrintf("\n\n");

}


void gopMtiPortPrintData
(
    IN GT_U32 portNum
    )
{
    SKERNEL_DEVICE_OBJECT *devObjPtr;
    GT_U32 regData;
    GT_U32 channelIdx;


    devObjPtr = smainDeviceObjects[0];
    channelIdx = (portNum / 8);

    smemRegGet(devObjPtr, SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI[portNum].MTI_EXT_PORT.portStatus, &regData);
    simWarningPrintf("portStatus[%2d]            [0x%8.8x] = [0x%8.8x]\n", portNum, SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI[portNum].MTI_EXT_PORT.portStatus, regData);

    smemRegGet(devObjPtr, SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI_EXTERNAL[channelIdx].MTI_STATS.control, &regData);
    simWarningPrintf("MTI_STATS.control[%2d]     [0x%8.8x] = [0x%8.8x]\n", channelIdx, SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI_EXTERNAL[channelIdx].MTI_STATS.control, regData);

    smemRegGet(devObjPtr, SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI[portNum].MTI_WRAPPER.MTI64_MAC.commandConfig, &regData);
    simWarningPrintf("commandConfig64[%2d]       [0x%8.8x] = [0x%8.8x]\n", portNum, SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI[portNum].MTI_WRAPPER.MTI64_MAC.commandConfig, regData);

    if (portNum & 0x1)
    {
        smemRegGet(devObjPtr, SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI[portNum].MTI_WRAPPER.PCS_10254050.control1, &regData);
        simWarningPrintf("PCS_10254050.control1[%2d] [0x%8.8x] = [0x%8.8x]\n", portNum, SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI[portNum].MTI_WRAPPER.PCS_10254050.control1, regData);
    } else
    {
        smemRegGet(devObjPtr, SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI[portNum].MTI_WRAPPER.PCS_10TO100.control1, &regData);
        simWarningPrintf("PCS_10TO100.control1[%2d]  [0x%8.8x] = [0x%8.8x]\n", portNum, SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI[portNum].MTI_WRAPPER.PCS_10TO100.control1, regData);
    }

    simWarningPrintf("\n\n");
}
#endif

#define TILE_OFFSET          0x20000000

#define LOOP_N_M_REG(regArr, baseAddr , numN ,stepN, numM , stepM)\
    {                                               \
        GT_U32 n,m;                                 \
        for (n = 0; n < (numN); n++)                \
        {                                           \
            for (m = 0; m < (numM); m++)            \
            {                                       \
                regArr[n][m] = baseAddr + (n * (stepN)) + (m * (stepM)); \
            }                                       \
        }                                           \
    }

#define LOOP_N_REG(regArr, baseAddr , numN , stepN) \
    {                                               \
        GT_U32 n;                                   \
        for (n = 0; n < (numN); n++)                \
        {                                           \
            regArr[n] = baseAddr + (n * (stepN));   \
        }                                           \
    }


#define TABLE_NAME_MAC(name)    (name) ? (name) : "unknown name"

/* NOTE : the SFDB_MAC_TBL_ACT_WORDS need to cover the mac table action
    registers starting at address 0x06000004 .
   we need data from registers :
   0x06000004 , 0x06000008 , 0x06000020

   that is why we need continue 8 words from the memory
   so  dataPtr[0] is 0x06000004
       dataPtr[1] is 0x06000008
       dataPtr[7] is 0x06000020
*/
#define SFDB_MAC_TBL_ACT_WORDS  8
#define SFDB_MAC_TBL_ACT_BYTES  (SFDB_MAC_TBL_ACT_WORDS * sizeof(GT_U32))

/* Flag is set when simulation SoftReset is done */
extern GT_U32 simulationSoftResetDone;

static DECLARE_UNIT_MEM_FUNCTION_MAC(smemChtGlobalReg               );
static DECLARE_UNIT_MEM_FUNCTION_MAC(smemChtTransQueReg             );
static DECLARE_UNIT_MEM_FUNCTION_MAC(smemChtEtherBrdgReg            );
static DECLARE_UNIT_MEM_FUNCTION_MAC(smemChtBufMngReg               );
static DECLARE_UNIT_MEM_FUNCTION_MAC(smemChtPortGroupConfReg        );
static DECLARE_UNIT_MEM_FUNCTION_MAC(smemChtMacTableReg             );
static DECLARE_UNIT_MEM_FUNCTION_MAC(smemChtInterBufBankReg         );
static DECLARE_UNIT_MEM_FUNCTION_MAC(smemChtBufMemoryConfReg        );
static DECLARE_UNIT_MEM_FUNCTION_MAC(smemChtVlanTableReg            );
static DECLARE_UNIT_MEM_FUNCTION_MAC(smemChtTriSpeedReg             );
static DECLARE_UNIT_MEM_FUNCTION_MAC(smemChtPreEgressReg            );
static DECLARE_UNIT_MEM_FUNCTION_MAC(smemChtPclReg                  );
static DECLARE_UNIT_MEM_FUNCTION_MAC(smemChtPolicerReg              );
static DECLARE_UNIT_MEM_FUNCTION_MAC(smemChtPclTbl                  );
static DECLARE_UNIT_MEM_FUNCTION_MAC(smemChtMacTbl                  );
static DECLARE_UNIT_MEM_FUNCTION_MAC(smemChtPolicerTbl              );
static DECLARE_UNIT_MEM_FUNCTION_MAC(smemChtTrunkTbl                );
static DECLARE_UNIT_MEM_FUNCTION_MAC(smemChtVlanTbl                 );
static DECLARE_UNIT_MEM_FUNCTION_MAC(smemChtArpTbl                  );
static DECLARE_UNIT_MEM_FUNCTION_MAC(smemChtPciReg                  );

static ACTIVE_WRITE_FUNC_PROTOTYPE_MAC(smemChtActiveWritePclAction      );
static ACTIVE_WRITE_FUNC_PROTOTYPE_MAC(smemChtActiveWriteVlanTbl        );


static void updateGopOldDbWithPipes(
    IN  SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN  GT_U32 gopBaseAddress
);

static GT_BOOL updateDevObjCoreClockInfo
(
    IN SKERNEL_DEVICE_OBJECT * deviceObjPtr
);

static GT_BOOL smemCheetahDuplicateUnitAllocationCheck
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SMEM_UNIT_DUPLICATION_INFO_STC* currentPtr
);


#define SET_UNIT_AND_SIZE_MAC(unitName)  (void*)&unitName, ((sizeof(unitName) / sizeof(GT_U32)))

/* number of registers to skip when we encounter start of unit */
static GT_U32  skipUnitStartNumReg = sizeof(SMEM_CHT_PP_REGS_UNIT_START_INFO_STC) / sizeof(GT_U32);

/* add offset address to all the registers */
static void offsetAddrSet
(
    IN void*    unitPtr,
    IN GT_U32   numOfRegisters ,
    IN GT_U32   addrOffset
)
{
    GT_U32  ii;
    GT_U32  *regAddPtr;

    regAddPtr = unitPtr;
    for(ii = 0 ; ii < numOfRegisters ; ii++,regAddPtr++)
    {
        if((*regAddPtr) == SMEM_CHT_PP_REGS_UNIT_START_INFO_MAGIC_NUMBER_CNS)
        {
            ii          += skipUnitStartNumReg;
            regAddPtr   += skipUnitStartNumReg;
        }

        if((*regAddPtr) == SMAIN_NOT_VALID_CNS)
        {
            continue;
        }
        /* address assumed to be 0 based (not unit based)*/
        (*regAddPtr) = addrOffset + (*regAddPtr);
    }
}


static void smemChtResetAuq
(
    IN  SKERNEL_DEVICE_OBJECT * devObjPtr
);

static void smemCht2ResetFuq
(
    IN  SKERNEL_DEVICE_OBJECT * devObjPtr
);

static void smemChtInitFuncArray(
    INOUT SMEM_CHT_DEV_MEM_INFO  * devMemInfoPtr
);

static void smemChtAllocSpecMemory(
    INOUT  SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SMEM_CHT_DEV_MEM_INFO  * devMemInfoPtr
);

static void smemChtReadWriteVlanQos (
    IN         SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN         GT_U32  port,
    IN         GT_U32  rdWr
);

static void smemChtReadWriteVidQosProt (
    IN         SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN         GT_U32  port,
    IN         GT_U32  rdWr
);

static void smemChtReadWritePolicyTcam (
    IN         SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN         GT_U32  line,
    IN         GT_U32  mode,
    IN         GT_U32  type,
    IN         GT_U32  rdWr,
    IN         GT_U32  valid
);
static void smemChtReadPolicyTcamCtrl (
    IN         SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN         GT_U32  line,
    IN         GT_U32  type,
    IN         GT_U32  mode
);
static void smemChtReadPolicyTcamData (
    IN         SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN         GT_U32  line,
    IN         GT_U32  type,
    IN         GT_U32  mode
);

static void smemChtWritePolicyTcamCtrl (
    IN         SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN         GT_U32  line,
    IN         GT_U32  type,
    IN         GT_U32  mode ,
    IN         GT_U32  valid
);
static void smemChtWritePolicyTcamData (
    IN         SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN         GT_U32  line,
    IN         GT_U32  type,
    IN         GT_U32  mode
);
static void smemChtReadWritePclAction (
    IN         SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN         GT_U32  line,
    IN         GT_U32  rdWr
);

static void smemChtReadWriteArpTable (
    IN         SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN         GT_U32  line,
    IN         GT_U32  rdWr
);

static void smemChtReadWriteMacSaTable (
    IN         SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN         GT_U32  line,
    IN         GT_U32  rdWr
);

static void smemChtTablesInfoCheck
(
    IN         SKERNEL_DEVICE_OBJECT * devObjPtr
);

static void smemChtRegDbCheck
(
    IN         SKERNEL_DEVICE_OBJECT * devObjPtr
);

#define SMI_TRANSACTION_SIZE       50

/* Private definition */
#define     GLB_REGS_NUM                        (76)
#define     GLB_SDMA_REGS_NUM                   (1023)
#define     EGR_GLB_REGS_NUM                    (0xffff / 4)
#define     TXQ_INTERNAL_REGS_NUM               (512)
#define     EGR_TRUNK_FILTER_REGS_NUM           (2032)
#define     EGR_PORT_WD_REGS_NUM                (64)
#define     EGR_MIB_CNT_REGS_NUM                (120)
#define     EGR_HYP_MIB_REGS_NUM                (60 + 28)
#define     EGR_XSMI_REGS_NUM                   (2)
#define     EGR_STC_TBL_REGS_NUM                (82)
#define     EGR_TALE_DROP_REGS_NUM              (3132)
#define     EGR_RATE_SHAPES_REGS_NUM            (512)
#define     BRDG_ETH_BRDG_REGS_NUM              (262655)
#define     BUF_MNG_REGS_NUM                    (196)
#define     GOP_CONF_REGS_NUM                   (512)
#define     GOP_MAC_MIB_REGS_NUM                (892)
#define     GOP_LED_REGS_NUM                    (16)
#define     MAC_FDB_REGS_NUM                    (124)
#define     MAC_TBL_REGS_NUM                    (16384 * 4)
#define     BANK_MEM_REGS_NUM                   (98304)
#define     BANK_WRITE_REGS_NUM                 (1)
#define     BUF_MEM_REGS_NUM                    (204)
#define     VTL_VLAN_CONF_REGS_NUM              (4)
#define     VTL_VLAN_TBL_REGS_NUM               (4096 * 3)
#define     VTL_MCST_TBL_REGS_NUM               (4096)
#define     VTL_STP_TBL_REGS_NUM                (256 * 2)
#define     TRI_SPEED_REGS_NUM                  (16383)
#define     PRE_EGR_PORTS_REGS_NUM              (22)
#define     PRE_EGR_GEN_REGS_NUM                (22)
#define     PRE_EGR_TRUNK_TBL_REGS_NUM          (128)
#define     PRE_QOS_TBL_REGS_NUM                (72 + 10)
#define     PRE_CPU_CODE_TBL_REGS_NUM           (256)
#define     PRE_STAT_RATE_TBL_REGS_NUM          (32)
#define     PRE_PORTS_INGR_STC_REGS_NUM         (27 * 3)
#define     PCL_CONF_REGS_NUM                   (4220)
#define     PCL_TCAM_REGS_NUM                   (16384)
#define     PCL_INTERN_REGS_NUM                 (22)
#define     PCL_PORT_VLAN_QOS_REGS_NUM          (64 * 2)
#define     PCL_PROT_BASED_VLAN_QOS_REGS_NUM    (64)
#define     PCL_ACTION_TBL_REGS_NUM             (1024 * 3)
#define     PCL_ID_CONF_TBL_REGS_NUM            (1152)
#define     POLICER_REGS_NUM                    (27)
#define     POLICER_TBL_REGS_NUM                (256 * 4)
#define     POLICER_QOS_REM_TBL_REGS_NUM        (72)
#define     POLICER_COUNT_TBL_REGS_NUM          (16 * 2)
#define     PCI_REGS_NUM                        (280)
#define     PHY_XAUI_DEV_NUM                    (3)
#define     PHY_IEEE_XAUI_REGS_NUM              (0xffff)
#define     PHY_EXT_XAUI_REGS_NUM               (0xffff)

#define     ARP_TABLE_REGS_NUM                  (1024 * 2)
#define     MAC_SA_TABLE_REGS_NUM               (4096)

/* Active memory table */
static SMEM_ACTIVE_MEM_ENTRY_STC smemChtActiveTable[] =
{
    /* Global interrupt cause register */
    {0x00000030, SMEM_FULL_MASK_CNS, NULL, 0 , smemChtActiveWriteIntrCauseReg, 0},

    /* Port<n> Interrupt Cause Register  */
    {0x0A800020, SMEM_CHT_GOPINT_MSK_CNS,
        smemChtActiveReadIntrCauseReg, 18, smemChtActiveWriteIntrCauseReg, 0},
    /* Tri-Speed Port<n> Interrupt Mask Register */
    {0x0A800024, SMEM_CHT_GOPINT_MSK_CNS,
        NULL, 0, smemChtActiveWritePortInterruptsMaskReg, 0},

    /* SDMA Receive SDMA Interrupt Cause Register (RxSDMAInt) */
    {0x0000280C, SMEM_FULL_MASK_CNS,
        smemChtActiveReadIntrCauseReg, 22, smemChtActiveWriteIntrCauseReg, 0},
    /* Receive SDMA Interrupt Mask Register */
    {0x00002814, SMEM_FULL_MASK_CNS,
        NULL, 0, smemChtActiveWriteRcvSdmaInterruptsMaskReg, 0},

    /* Bridge Interrupt Cause Register */
    {0x02040130, SMEM_FULL_MASK_CNS,
        smemChtActiveReadIntrCauseReg, 14, smemChtActiveWriteIntrCauseReg, 0},
    /* Bridge Interrupt Mask Register */
    {0x02040134, SMEM_FULL_MASK_CNS,
        NULL, 0, smemChtActiveWriteBridgeInterruptsMaskReg, 0},

    /* Security Breach Status Register2 */
    {0x020401A8, SMEM_FULL_MASK_CNS, smemChtActiveReadBreachStatusReg, 0, NULL, 0},

    /* Port MIB Counters Capture */
    {0x04004020, SMEM_FULL_MASK_CNS, NULL, 0, smemChtActiveWriteMibCntCtrl, 0},
    {0x04804020, SMEM_FULL_MASK_CNS, NULL, 0, smemChtActiveWriteMibCntCtrl, 1},
    {0x05004020, SMEM_FULL_MASK_CNS, NULL, 0, smemChtActiveWriteMibCntCtrl, 2},
    {0x05804020, SMEM_FULL_MASK_CNS, NULL, 0, smemChtActiveWriteMibCntCtrl, 3},

    /* SMI0 Management Register : 0x04004054 , 0x05004054 */
    {0x04004054, 0xFEFFFFFF, NULL, 0 , smemChtActiveWriteSmi,0},

    /* HyperG.Stack Ports MIB Counters and XSMII Configuration Register */
    {0x01800180, SMEM_FULL_MASK_CNS, NULL, 0, smemChtActiveWriteHgsMibCntCtrl, 0},

    /* The MAC MIB Counters */
    {0x04010000, SMEM_CHT_COUNT_MSK_CNS, smemChtActiveReadCntrs, 0, NULL, 0},
    {0x04810000, SMEM_CHT_COUNT_MSK_CNS, smemChtActiveReadCntrs, 1, NULL, 0},
    {0x05010000, SMEM_CHT_COUNT_MSK_CNS, smemChtActiveReadCntrs, 2, NULL, 0},
    {0x05810000, SMEM_CHT_COUNT_MSK_CNS, smemChtActiveReadCntrs, 3, NULL, 0},

    /* The MAC MIB Counters of XG ports 24..26 */
    {0x01C00000, SMEM_CHT_COUNT_MSK_CNS, smemChtActiveReadCntrs, 24, NULL, 0},
    {0x01C40000, SMEM_CHT_COUNT_MSK_CNS, smemChtActiveReadCntrs, 25, NULL, 0},
    {0x01C80000, SMEM_CHT_COUNT_MSK_CNS, smemChtActiveReadCntrs, 26, NULL, 0},

    /* Host Incoming Packets Count */
    {0x020400BC, SMEM_FULL_MASK_CNS, smemChtActiveReadCntrs, 0, NULL,0},
    /* Host Outgoing Packets Count */
    {0x020400C0, SMEM_FULL_MASK_CNS, smemChtActiveReadCntrs, 0, NULL,0},
    /* Host Outgoing Multicast Packet Count */
    {0x020400CC, SMEM_FULL_MASK_CNS, smemChtActiveReadCntrs, 0, NULL,0},
    /* Host Outgoing Broadcast Packet Count */
    {0x020400D0, SMEM_FULL_MASK_CNS, smemChtActiveReadCntrs, 0, NULL,0},
    /* Matrix Source/Destination Packet Count */
    {0x020400D4, SMEM_FULL_MASK_CNS, smemChtActiveReadCntrs, 0, NULL,0},

    /* Set Incoming Packet Count */
    {0x020400E0, 0xFFFFFFF0, smemChtActiveReadCntrs, 0, NULL,0}, /* Set #0 counters */
    {0x020400F4, SMEM_FULL_MASK_CNS, smemChtActiveReadCntrs, 0, NULL,0}, /* Set #1 Incoming counters */
    {0x020400F8, SMEM_FULL_MASK_CNS, smemChtActiveReadCntrs, 0, NULL,0}, /* Set #1 VLAN ingress filtered counters */
    {0x020400FC, SMEM_FULL_MASK_CNS, smemChtActiveReadCntrs, 0, NULL,0}, /* Set #1 Security filtered counters */
    {0x02040100, SMEM_FULL_MASK_CNS, smemChtActiveReadCntrs, 0, NULL,0}, /* Set #1 Bridge filtered counters */

    /* The Egress packet count registers */
    {0x01B40140, 0xFFFFFFF0, smemChtActiveReadCntrs, 0, NULL,0},
    {0x01B40150, 0xFFFFFFF0, smemChtActiveReadCntrs, 0, NULL,0},
    {0x01B40160, 0xFFFFFFF0, smemChtActiveReadCntrs, 0, NULL,0},
    {0x01B40170, 0xFFFFFFF0, smemChtActiveReadCntrs, 0, NULL,0},

    /* Message to CPU register  */
    {0x06000034, SMEM_FULL_MASK_CNS, smemChtActiveReadMsg2Cpu, 0 , NULL,0},

    /* The SDMA packet count registers */
    {0x00002820, 0xFFFFFFF0, smemChtActiveReadCntrs, 0, NULL,0},
    {0x00002830, 0xFFFFFFF0, smemChtActiveReadCntrs, 0, NULL,0},

    /* The SDMA byte count registers */
    {0x00002840, 0xFFFFFFF0, smemChtActiveReadCntrs, 0, NULL,0},
    {0x00002850, 0xFFFFFFF0, smemChtActiveReadCntrs, 0, NULL,0},

    /* The SDMA resource error registers */
    {0x00002860, SMEM_CHT_COUNT_MSK_CNS, smemChtActiveReadCntrs, 0, NULL,0},
    {0x00002864, SMEM_CHT_COUNT_MSK_CNS, smemChtActiveReadCntrs, 0, NULL,0},

    /* Mac Table Access Control Register */
    {0x06000064, SMEM_FULL_MASK_CNS, NULL, 0 , smemChtActiveWriteFdbMsg,0},

    /* Address Update Queue Base Address */
    {0x000000C0, SMEM_FULL_MASK_CNS, NULL, 0 , smemChtActiveAuqBaseWrite, 0},

    /* XSMI Management Register */
    {0x01CC0000, SMEM_FULL_MASK_CNS, NULL, 0 , smemChtActiveWriteXSmii, 0},

    /* Ports VLAN, QoS and Protocol Access Control Register */
    {0x0B800328, SMEM_FULL_MASK_CNS, NULL, 0 , smemChtActiveWriteVlanQos, 0},

    /* Action Table and Policy TCAM Access Control Register */
    {0x0B800130, SMEM_FULL_MASK_CNS, NULL, 0 , smemChtActiveWritePclAction, 0},

    /* PCL-ID Table Access Control Register */
    {0x0B800204, SMEM_FULL_MASK_CNS, NULL, 0 , smemChtActiveWritePclId, 0},

    /* Policers Table Access Control Register */
    {0x0C000014, SMEM_FULL_MASK_CNS, NULL, 0 , smemChtActiveWritePolicerTbl, 0},

    /* Policers QoS Remarking and Initial DP Table Access Control Register */
    {0x0C000024, SMEM_FULL_MASK_CNS, NULL, 0 , smemChtActiveWritePolicerQos, 0},

    /* Policer Counters Table Access Control Register */
    {0x0C000038, SMEM_FULL_MASK_CNS, NULL, 0 , smemChtActiveWritePolicerCnt, 0},

    /* Trunk table Access Control Register */
    {0x0B000028, SMEM_FULL_MASK_CNS, NULL, 0 , smemChtActiveWriteTrunkTbl, 0},

    /* QoSProfile to QoS Access Control Register */
    {0x0B00002C, SMEM_FULL_MASK_CNS, NULL, 0 , smemChtActiveWriteQosAccess, 0},

    /* CPU Code Access Control Register */
    {0x0B000030, SMEM_FULL_MASK_CNS, NULL, 0 , smemChtActiveWriteCpuCodeAccess, 0},

    /* Statistical Rate Limits Table Access Control Register */
    {0x0B000034, SMEM_FULL_MASK_CNS, NULL, 0 , smemChtActiveWriteStatRateLimTbl, 0},

    /* Ingress STC Table Access Control Register */
    {0x0B000038, SMEM_FULL_MASK_CNS, NULL, 0 , smemChtActiveWriteIngrStcTbl, 0},

    /* VLT Tables Access Control Register */
    {0x0A00000C, SMEM_FULL_MASK_CNS, NULL, 0 , smemChtActiveWriteVlanTbl, 0},

    /* SDMA */
    {0x00002680, SMEM_FULL_MASK_CNS, NULL, 0 , smemChtActiveWriteSdmaCommand,0},

    /* Message From CPU Management */
    {0x06000050, SMEM_FULL_MASK_CNS, NULL, 0 , smemChtActiveWriteNewFdbMsgFromCpu, 0},

    /* MAC Table Action0 Register */
    {0x06000004, SMEM_FULL_MASK_CNS, NULL, 0 , smemChtActiveWriteFdbActionTrigger, 0},

    /* Transmit SDMA Queue Command Register */
    {0x00002868, SMEM_FULL_MASK_CNS, NULL, 0 , smemChtActiveWriteStartFromCpuDma, 0},

    /* Port<n> Auto-Negotiation Configuration Register */
    {0x0A80000C, 0xFFFF20FF, NULL, 0 , smemChtActiveWriteForceLinkDown, 0},

    {0x0A801800, 0xFFFFF8FF, NULL, 0 , smemChtActiveWriteForceLinkDownXg, 0},
    /* ARP MAC DA table */
    {0x07800208, SMEM_FULL_MASK_CNS, NULL, 0 , smemChtActiveWriteArpTable, 0},

    /* HGS Port<n> Interrupt Cause Register  */
    {0x0A801814, SMEM_FULL_MASK_CNS,
        smemChtActiveReadIntrCauseReg, 24, smemChtActiveWriteIntrCauseReg, 0},
    /* HGS Port<n> Interrupt Mask Register */
    {0x0A801818, SMEM_FULL_MASK_CNS,
        NULL, 0, smemChtActiveWriteHgsInterruptsMaskReg, 0},

    {0x0A801914, SMEM_FULL_MASK_CNS,
        smemChtActiveReadIntrCauseReg, 25, smemChtActiveWriteIntrCauseReg, 0},
    /* HGS Port<n> Interrupt Mask Register */
    {0x0A801918, SMEM_FULL_MASK_CNS,
        NULL, 0, smemChtActiveWriteHgsInterruptsMaskReg, 0},

    {0x0A801A14, SMEM_FULL_MASK_CNS,
        smemChtActiveReadIntrCauseReg, 26, smemChtActiveWriteIntrCauseReg, 0},
    /* HGS Port<n> Interrupt Mask Register */
    {0x0A801A18, SMEM_FULL_MASK_CNS,
        NULL, 0, smemChtActiveWriteHgsInterruptsMaskReg, 0},

    /* Transmit SDMA Interrupt Cause Register */
    {0x00002810, SMEM_FULL_MASK_CNS,
        smemChtActiveReadIntrCauseReg, 21 , smemChtActiveWriteIntrCauseReg, 0},
    /* Transmit SDMA Interrupt Mask Register */
    {0x00002818, SMEM_FULL_MASK_CNS,
        NULL, 0, smemChtActiveWriteTransSdmaInterruptsMaskReg, 0},

    /* MAC Table Interrupt Cause Register */
    {0x06000018, SMEM_FULL_MASK_CNS,
        smemChtActiveReadIntrCauseReg, 17, smemChtActiveWriteIntrCauseReg, 0},
    /* MAC Table Interrupt Mask Register */
    {0x0600001C, SMEM_FULL_MASK_CNS,
        NULL, 0, smemChtActiveWriteMacInterruptsMaskReg, 0},

    /* Global control register */
    {0x00000058, SMEM_FULL_MASK_CNS, NULL, 0 , smemChtActiveWriteGlobalReg, 0},

    /* SDMA configuration register */
    {0x00002800, SMEM_FULL_MASK_CNS, NULL, 0 , smemChtActiveWriteSdmaConfigReg, 0},

    /* Transmit Queue Control Register */
    {0x01800000, SMEM_FULL_MASK_CNS, NULL, 0 , smemChtActiveWriteTxQuCtrlReg, 0},

    /* Transmit Queue Config Register */
    {0x01800080, 0XFFFF01FF, NULL, 0 , smemChtActiveWriteTxQConfigReg, 0},

    /* Port<%n> Rate Limit Counter */
    {0x02000400, 0XFFFE0FFF,
        smemChtActiveReadRateLimitCntReg, 0 , smemChtActiveWriteToReadOnlyReg, 0},

    /* CPU Port MIB counters registers */
    {0x00000060, 0xFFFFFFE0, smemChtActiveReadCntrs, 0, NULL,0},

    /* must be last anyway */
    {END_OF_TABLE, SMEM_FULL_MASK_CNS, NULL,0,NULL,0}
};

#define SMEM_ACTIVE_MEM_TABLE_SIZE \
    (sizeof(smemChtActiveTable)/sizeof(smemChtActiveTable[0]))

/* Active memory table */
static SMEM_ACTIVE_MEM_ENTRY_STC smemChtPciActiveTable[] =
{
    /* PCI Interrupt Cause Register */
    {0x00000114, SMEM_FULL_MASK_CNS, NULL, 0 , smemChtActiveWriteIntrCauseReg, 0},
    /* PCI Interrupt Mask Register -- all actions here --> do on 0x00000034 */
    {0x00000118, SMEM_FULL_MASK_CNS, smemChtActiveReadRedirect, 0x00000034 , smemChtActiveWriteRedirect, 0x00000034},

    /* must be last anyway */
    {END_OF_TABLE, SMEM_FULL_MASK_CNS, NULL,0,NULL,0}
};

#define SMEM_ACTIVE_PCI_MEM_TABLE_SIZE \
    (sizeof(smemChtPciActiveTable)/sizeof(smemChtPciActiveTable[0]))

/**
* @internal smemChtInit function
* @endinternal
*
* @brief   Init memory module for a Cht device.
*
* @param[in] devObjPtr                - pointer to device object.
*/
void smemChtInit
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr
)
{
    SMEM_CHT_DEV_MEM_INFO  * devMemInfoPtr;

    /* alloc SMEM_CHT_DEV_MEM_INFO */
    devMemInfoPtr = (SMEM_CHT_DEV_MEM_INFO *)smemDeviceObjMemoryAlloc(devObjPtr,1, sizeof(SMEM_CHT_DEV_MEM_INFO));
    if (devMemInfoPtr == 0)
    {
            skernelFatalError("smemChtInit: allocation error\n");
    }

    devObjPtr->deviceMemory = devMemInfoPtr;

    if (SKERNEL_IS_DEVICE_CHEETAH_B0(devObjPtr))
    {
        /*DX106 (AKA DX107-B0)*/
        devObjPtr->notSupportPciConfigMemory = 1;/* the device not support PCI/PEX configuration memory space */
        devObjPtr->fdbNumEntries =  SMEM_MAC_TABLE_SIZE_8KB;
    }
    else
    {
        devObjPtr->fdbNumEntries = SMEM_MAC_TABLE_SIZE_16KB;
    }

    devObjPtr->fdbMaxNumEntries = devObjPtr->fdbNumEntries;
    devObjPtr->supportEqEgressMonitoringNumPorts = 32;

    devObjPtr->supportOnlyUcRouter = 1;

    /* init specific functions array */
    smemChtInitFuncArray(devMemInfoPtr);

    /* allocate address type specific memories */
    smemChtAllocSpecMemory(devObjPtr, devMemInfoPtr);

    /*override default of 0 */
    devObjPtr->globalInterruptCauseRegWriteBitmap_pci = 0x1fe;/* bits 1..8 */


    devObjPtr->activeMemPtr = smemChtActiveTable;


    smemChtTableInfoSet(devObjPtr);
}

/**
* @internal smemChtInitPciRegistres function
* @endinternal
*
* @brief   Init PCI registers : devId,revision for a device - to the 0x4c register
*
* @param[in] devObjPtr                - pointer to device object.
*/
void smemChtInitPciRegistres
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr
)
{
    GT_U32  data;
    GT_U32  pciDeviceAndVendorIdRegisterVal;/*data to write to Device and Vendor ID Register */
    GT_U32  pciClassCodeAndRevisionIdRegister;/*data to write to Class Code and Revision ID Register*/
    GT_U32  devRev;/*device revision */

    /* Get the 'devId' and 'revision'  */
    scibReadMemory(devObjPtr->deviceId, SMEM_CHT_DEVICE_ID_REG(devObjPtr), 1, &data);
    devRev = data & 0xF;/*4 LSBits*/

    if((GT_U32)devObjPtr->deviceType > SMEM_CHT_MARVELL_VENDOR_ID_CNS)
    {
        /* write the 'devId' to 0x4c also */
        /* Device ID Register */
        pciDeviceAndVendorIdRegisterVal = (GT_U32)(devObjPtr->deviceType >> 16);
        SMEM_U32_SET_FIELD(data, 4, 16, pciDeviceAndVendorIdRegisterVal);

        scibWriteMemory(devObjPtr->deviceId, SMEM_CHT_DEVICE_ID_REG(devObjPtr), 1, &data);
    }
    else /* support 'wildcard' devId */
    {
        /* Read the 'devId' from 0x4c  */
        pciDeviceAndVendorIdRegisterVal = SMEM_U32_GET_FIELD(data, 4, 16);

        if(pciDeviceAndVendorIdRegisterVal == 0)
        {
            /* support backward compatible --> the registers file not specified the
               devId  --> so set it according to the deviceId */

            switch(devObjPtr->deviceType)
            {
                case SKERNEL_DXCH:
                    pciDeviceAndVendorIdRegisterVal = (GT_U32)SKERNEL_98DX275;
                    break;
                case SKERNEL_DXCH_B0:
                    /* the DX106 has no 'PCI' registers but still code is here to be unified
                       code */
                    pciDeviceAndVendorIdRegisterVal = (GT_U32)SKERNEL_98DX106;
                    break;
                case SKERNEL_DXCH2:
                    pciDeviceAndVendorIdRegisterVal = (GT_U32)SKERNEL_98DX285;
                    break;
                case SKERNEL_DXCH3:
                    pciDeviceAndVendorIdRegisterVal = (GT_U32)SKERNEL_98DX5128;
                    break;
                case SKERNEL_DXCH3P:
                    pciDeviceAndVendorIdRegisterVal = (GT_U32)SKERNEL_98DX287;
                    break;
                case SKERNEL_DXCH3_XG:
                case SKERNEL_XCAT_24_AND_4:
                default:
                    /* not supported 'Backward compatible' because those are 'new devices' */
                    skernelFatalError("smemChtInitPciRegistres: register[0x%8.8x] was not set \n",
                        SMEM_CHT_DEVICE_ID_REG(devObjPtr));
                    break;
            }

            /* write the 'devId' to 0x4c also */
            /* Device ID Register */
            SMEM_U32_SET_FIELD(data, 4, 16, pciDeviceAndVendorIdRegisterVal >> 16);

            scibWriteMemory(devObjPtr->deviceId, SMEM_CHT_DEVICE_ID_REG(devObjPtr), 1, &data);
        }
    }

    if(SMEM_CHT_IS_SIP6_GET(devObjPtr))
    {
        /* write the info to the DFX too */
        smemDfxRegFldSet(devObjPtr,
            SMEM_FALCON_DFX_SERVER_DEVICE_ID_STATUS_REG(devObjPtr),
                0, 16, pciDeviceAndVendorIdRegisterVal);
        smemDfxRegFldSet(devObjPtr,
            SMEM_FALCON_DFX_SERVER_DEVICE_ID_STATUS_REG(devObjPtr),
                16, 4, devRev);

        simForcePrintf("The deviceId [0x%4.4x] rev[0x%x] in MG and DFX \n",
            pciDeviceAndVendorIdRegisterVal,devRev);

        if(!SMEM_CHT_IS_SIP6_10_GET(devObjPtr))
        {
            /* PEX MAC capable of receiving only the 10..15 bits */
            pciDeviceAndVendorIdRegisterVal &= 0xFC00;/* set the devId WITHOUT 5 bits 'SER' and WITHOUT the 'bonding' bit 9 */
        }

        simForcePrintf("The deviceId [0x%4.4x] in the PCI config space \n",
            pciDeviceAndVendorIdRegisterVal);
    }


    pciDeviceAndVendorIdRegisterVal <<= 16;/* set the devId in 16 MSB */
    pciDeviceAndVendorIdRegisterVal |= SMEM_CHT_MARVELL_VENDOR_ID_CNS;/* set the vendorId in 16 LSB */
    scibPciRegWrite(devObjPtr->deviceId, 0x0, 1, &pciDeviceAndVendorIdRegisterVal);

    /* set the revision */
    pciClassCodeAndRevisionIdRegister = SMEM_U32_GET_FIELD(data,0,4);
    /* set BaseClass */
    pciClassCodeAndRevisionIdRegister |= 0x2 << 24;
    scibPciRegWrite(devObjPtr->deviceId, 0x8, 1, &pciClassCodeAndRevisionIdRegister);

    /* set the Device Memory Mapped Internal Base Address Register */
    scibPciRegWrite(devObjPtr->deviceId, 0x18/* BAR2*/, 1, &devObjPtr->deviceHwId);

    if(SMEM_CHT_IS_SIP5_GET(devObjPtr))
    {
        /* write the info to the DFX too */
        smemDfxRegFldSet(devObjPtr,
            SMEM_LION2_DFX_SERVER_JTAG_DEVICE_ID_STATUS_REG(devObjPtr),
                28, 4, devRev);
    }

}

/**
* @internal smemSip5AdditionalDefaultRegInit function
* @endinternal
*
* @brief   additional Init memory module for a Sip5 device -
*         after the load of the default registers file
* @param[in] devObjPtr                - pointer to device object.
*/
static void smemSip5AdditionalDefaultRegInit
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr
)
{
    GT_U32  value;  /*register value*/
    GT_U32  port;   /* port iterator */
    GT_U32  maxPhysicalPort,maxPort;
    GT_U32  *memPtr;
    GT_U32  physicalPort;
    GT_U32  ownDev;
    GT_U32  vlan,maxVlan;/* vlan iterator */
    GT_U32  index,ii;/*iterator*/
    GT_U32  regAddr;/* register address */
    GT_U32  *regPtr;/* register value pointer */
    GT_U32  startBit;/* start bit */
    GT_U32  bridgeIngressMaxPhysicalPorts =  (SMEM_CHT_IS_SIP6_GET(devObjPtr) ? 128 : 256);
    smemRegFldGet(devObjPtr, SMEM_CHT_GLB_CTRL_REG(devObjPtr),
        4, devObjPtr->flexFieldNumBitsSupport.hwDevNum,
        &value);
    /* ownDevNum[4:0] */
    ownDev = value;

    maxPhysicalPort = devObjPtr->limitedResources.phyPort;

    for(physicalPort = 0 ; physicalPort < maxPhysicalPort ; physicalPort++)
    {
        /* need to initialize the TTI Ingress Physical Port Table for the first 256 entries */
        memPtr = smemMemGet(devObjPtr,SMEM_LION2_TTI_PHYSICAL_PORT_ATTRIBUTE_TBL_MEM(devObjPtr, physicalPort));
        /* check if not initialized by the reg file (check only word 0) */
        if(0 == snetFieldValueGet(memPtr , 0 ,32))
        {
            /*set <Port default Source ePort number> */
            snetFieldFromEntry_GT_U32_Set(devObjPtr,
                memPtr,
                devObjPtr->tableFormatInfo[SKERNEL_TABLE_FORMAT_TTI_PHYSICAL_PORT_ATTRIBUTE_E].formatNamePtr,
                physicalPort,/* the physical port */
                devObjPtr->tableFormatInfo[SKERNEL_TABLE_FORMAT_TTI_PHYSICAL_PORT_ATTRIBUTE_E].fieldsInfoPtr,
                devObjPtr->tableFormatInfo[SKERNEL_TABLE_FORMAT_TTI_PHYSICAL_PORT_ATTRIBUTE_E].fieldsNamePtr,
                SMEM_LION3_TTI_PHYSICAL_PORT_TABLE_FIELDS_PORT_DEFAULT_SOURCE_EPORT_NUMBER,
                physicalPort);
        }

        /* need to initialize the HA Egress Physical Port Table */
        memPtr = smemMemGet(devObjPtr,SMEM_LION2_HA_PHYSICAL_PORT_1_ATTRIBUTES_TBL_MEM(devObjPtr, physicalPort));

        /* init the HA physical port 1 table */
        snetFieldFromEntry_GT_U32_Set(devObjPtr,
            memPtr,
            devObjPtr->tableFormatInfo[SKERNEL_TABLE_FORMAT_HA_PHYSICAL_PORT_1_E].formatNamePtr,
            physicalPort,/* the physical port */
            devObjPtr->tableFormatInfo[SKERNEL_TABLE_FORMAT_HA_PHYSICAL_PORT_1_E].fieldsInfoPtr,
            devObjPtr->tableFormatInfo[SKERNEL_TABLE_FORMAT_HA_PHYSICAL_PORT_1_E].fieldsNamePtr,
            SMEM_LION3_HA_PHYSICAL_PORT_TABLE_1_FIELDS_ROUTED_MAC_SA_MOD_EN,
            1);

        if(SMEM_CHT_IS_SIP6_GET(devObjPtr))
        {
            snetFieldFromEntry_GT_U32_Set(devObjPtr,
                memPtr,
                devObjPtr->tableFormatInfo[SKERNEL_TABLE_FORMAT_HA_PHYSICAL_PORT_1_E].formatNamePtr,
                physicalPort,/* the physical port */
                devObjPtr->tableFormatInfo[SKERNEL_TABLE_FORMAT_HA_PHYSICAL_PORT_1_E].fieldsInfoPtr,
                devObjPtr->tableFormatInfo[SKERNEL_TABLE_FORMAT_HA_PHYSICAL_PORT_1_E].fieldsNamePtr,
                SMEM_LION3_HA_PHYSICAL_PORT_TABLE_1_FIELDS_ROUTER_MAC_SA_ASSIGNMENT_MODE,
                1);
        }


        snetFieldFromEntry_GT_U32_Set(devObjPtr,
            memPtr,
            devObjPtr->tableFormatInfo[SKERNEL_TABLE_FORMAT_HA_PHYSICAL_PORT_1_E].formatNamePtr,
            physicalPort,/* the physical port */
            devObjPtr->tableFormatInfo[SKERNEL_TABLE_FORMAT_HA_PHYSICAL_PORT_1_E].fieldsInfoPtr,
            devObjPtr->tableFormatInfo[SKERNEL_TABLE_FORMAT_HA_PHYSICAL_PORT_1_E].fieldsNamePtr,
            SMEM_LION3_HA_PHYSICAL_PORT_TABLE_1_FIELDS_PORT_LIST_BIT_VECTOR_OFFSET,
            0x1f);

        if(SMEM_CHT_IS_SIP6_30_GET(devObjPtr))
        {
            snetFieldFromEntry_GT_U32_Set(devObjPtr,
                memPtr,
                devObjPtr->tableFormatInfo[SKERNEL_TABLE_FORMAT_HA_PHYSICAL_PORT_1_E].formatNamePtr,
                physicalPort,/* the physical port */
                devObjPtr->tableFormatInfo[SKERNEL_TABLE_FORMAT_HA_PHYSICAL_PORT_1_E].fieldsInfoPtr,
                devObjPtr->tableFormatInfo[SKERNEL_TABLE_FORMAT_HA_PHYSICAL_PORT_1_E].fieldsNamePtr,
                SMEM_SIP6_30_HA_PHYSICAL_PORT_TABLE_1_FIELDS_PRP_BASE_LSDU_OFFSET_E,
                0xe);
        }

        memPtr = smemMemGet(devObjPtr,SMEM_LION2_HA_PHYSICAL_PORT_2_ATTRIBUTES_TBL_MEM(devObjPtr, physicalPort));
        /* init the HA physical port 2 table */
        snetFieldFromEntry_GT_U32_Set(devObjPtr,
            memPtr,
            devObjPtr->tableFormatInfo[SKERNEL_TABLE_FORMAT_HA_PHYSICAL_PORT_2_E].formatNamePtr,
            physicalPort,/* the physical port */
            devObjPtr->tableFormatInfo[SKERNEL_TABLE_FORMAT_HA_PHYSICAL_PORT_2_E].fieldsInfoPtr,
            devObjPtr->tableFormatInfo[SKERNEL_TABLE_FORMAT_HA_PHYSICAL_PORT_2_E].fieldsNamePtr,
            SMEM_LION3_HA_PHYSICAL_PORT_TABLE_2_FIELDS_SER_CHECK_ENABLE,
            1);

        snetFieldFromEntry_GT_U32_Set(devObjPtr,
            memPtr,
            devObjPtr->tableFormatInfo[SKERNEL_TABLE_FORMAT_HA_PHYSICAL_PORT_2_E].formatNamePtr,
            physicalPort,/* the physical port */
            devObjPtr->tableFormatInfo[SKERNEL_TABLE_FORMAT_HA_PHYSICAL_PORT_2_E].fieldsInfoPtr,
            devObjPtr->tableFormatInfo[SKERNEL_TABLE_FORMAT_HA_PHYSICAL_PORT_2_E].fieldsNamePtr,
            SMEM_LION3_HA_PHYSICAL_PORT_TABLE_2_FIELDS_INVALID_CRC_MODE,
            0x1);

        if(SMEM_CHT_IS_SIP6_30_GET(devObjPtr))
        {
            snetFieldFromEntry_GT_U32_Set(devObjPtr,
                memPtr,
                devObjPtr->tableFormatInfo[SKERNEL_TABLE_FORMAT_HA_PHYSICAL_PORT_2_E].formatNamePtr,
                physicalPort,/* the physical port */
                devObjPtr->tableFormatInfo[SKERNEL_TABLE_FORMAT_HA_PHYSICAL_PORT_2_E].fieldsInfoPtr,
                devObjPtr->tableFormatInfo[SKERNEL_TABLE_FORMAT_HA_PHYSICAL_PORT_2_E].fieldsNamePtr,
                SMEM_SIP6_30_HA_PHYSICAL_PORT_TABLE_2_FIELDS_HSR_BASE_LSDU_OFFSET_E,
                0xe);
        }

        if(devObjPtr->supportTrafficManager)
        {
            /* 4 ports in entry */
            if(0 == (physicalPort % 4))
            {
                memPtr = smemMemGet(devObjPtr,
                    SMEM_BOBCAT2_TM_EGRESS_GLUE_TARGET_INTERFACE_TBL_MEM(devObjPtr, physicalPort));

                value = (((physicalPort + 0) % 128) <<  0) |
                        (((physicalPort + 1) % 128) <<  8) |
                        (((physicalPort + 2) % 128) << 16) |
                        (((physicalPort + 3) % 128) << 24) ;

                *memPtr = value;
            }
        }

        if(physicalPort < bridgeIngressMaxPhysicalPorts)/* limited to 128 (FALCON) or 2 56(BC3) even there are 1024 ports (Falcon) 512 ports (BC3) */
        {
            /*,{&STRING_FOR_UNIT_NAME(UNIT_L2I),            0x00200000,         0x1fffff80,    256,    0x4      }*/
            memPtr = smemMemGet(devObjPtr,SMEM_LION2_BRIDGE_INGRESS_PHYSICAL_PORT_TBL_MEM(devObjPtr, physicalPort));
            *memPtr = 0x1fffff80;
        }

    }

    {
        GT_U32  numStg;

        if(SMEM_CHT_IS_SIP6_15_GET(devObjPtr))
        {
            numStg = devObjPtr->limitedResources.stgId;
        }
        else
        {
            numStg = 1 << devObjPtr->flexFieldNumBitsSupport.stgId;
        }

        /*,{&STRING_FOR_UNIT_NAME(UNIT_EGF_SHT_4),            0x00040000,         0xffffffff,  32768,    0x4      }*/
        for(index = 0 ; index < numStg; index++)
        {
            memPtr = smemMemGet(devObjPtr,SMEM_LION_EGR_STP_TBL_MEM_REG(devObjPtr, index));
            for(port = 0 ; port < devObjPtr->tablesInfo.egressStp.paramInfo[0].step *8 ; port += 32)
            {
                memPtr[port>>5] = 0xffffffff;
            }
        }
    }


    maxPort = devObjPtr->defaultEPortNumEntries;

    /* loop on default eports */
    for(port = 0 ; port < maxPort ; port++)
    {
        /* need to initialize the pre-TTI ingress EPort Table */
        memPtr = smemMemGet(devObjPtr,SMEM_LION2_TTI_PRE_TTI_LOOKUP_INGRESS_EPORT_TABLE_TBL_MEM(devObjPtr, port));
        /* check if not initialized by the reg file (check only word 0) */
        if(0 == snetFieldValueGet(memPtr , 0 ,32))
        {
    #if 0
            /*set <Default Tag1 VLAN-ID> */
            snetFieldFromEntry_GT_U32_Set(devObjPtr,
                memPtr,
                STR(lion3TtiDefaultEPortTableFieldsFormat),
                port,/* the default ePort */
                lion3TtiDefaultEPortTableFieldsFormat,lion3TtiDefaultEPortFieldsTableNames,
                SMEM_LION3_TTI_DEFAULT_E_PORT_TABLE_FIELDS_DEF_TAG1_VLAN_ID,
                1);
    #endif /*0*/
            /*set <Port default eVLAN> */
            snetFieldFromEntry_GT_U32_Set(devObjPtr,
                memPtr,
                devObjPtr->tableFormatInfo[SKERNEL_TABLE_FORMAT_TTI_DEFAULT_EPORT_E].formatNamePtr,
                port,/* the default ePort */
                devObjPtr->tableFormatInfo[SKERNEL_TABLE_FORMAT_TTI_DEFAULT_EPORT_E].fieldsInfoPtr,
                devObjPtr->tableFormatInfo[SKERNEL_TABLE_FORMAT_TTI_DEFAULT_EPORT_E].fieldsNamePtr,
                SMEM_LION3_TTI_DEFAULT_E_PORT_TABLE_FIELDS_P_EVLAN,
                1);

            /*set <DSA Bypass Bridge> */
            snetFieldFromEntry_GT_U32_Set(devObjPtr,
                memPtr,
                devObjPtr->tableFormatInfo[SKERNEL_TABLE_FORMAT_TTI_DEFAULT_EPORT_E].formatNamePtr,
                port,/* the default ePort */
                devObjPtr->tableFormatInfo[SKERNEL_TABLE_FORMAT_TTI_DEFAULT_EPORT_E].fieldsInfoPtr,
                devObjPtr->tableFormatInfo[SKERNEL_TABLE_FORMAT_TTI_DEFAULT_EPORT_E].fieldsNamePtr,
                SMEM_LION3_TTI_DEFAULT_E_PORT_TABLE_FIELDS_EXTENDED_DSA_BYPASS_BRIDGE,
                1);

            /*set <TRILL Ingress outer VID0> */
            snetFieldFromEntry_GT_U32_Set(devObjPtr,
                memPtr,
                devObjPtr->tableFormatInfo[SKERNEL_TABLE_FORMAT_TTI_DEFAULT_EPORT_E].formatNamePtr,
                port,/* the default ePort */
                devObjPtr->tableFormatInfo[SKERNEL_TABLE_FORMAT_TTI_DEFAULT_EPORT_E].fieldsInfoPtr,
                devObjPtr->tableFormatInfo[SKERNEL_TABLE_FORMAT_TTI_DEFAULT_EPORT_E].fieldsNamePtr,
                SMEM_LION3_TTI_DEFAULT_E_PORT_TABLE_FIELDS_TRILL_INGRESS_OUTER_VID0,
                1);

            if(SMEM_CHT_IS_SIP6_30_GET(devObjPtr))
            {
                /* set <LSDU constant> : Constant for LSDU size calculation */
                snetFieldFromEntry_GT_U32_Set(devObjPtr,
                    memPtr,
                    devObjPtr->tableFormatInfo[SKERNEL_TABLE_FORMAT_TTI_DEFAULT_EPORT_E].formatNamePtr,
                    port,/* the default ePort */
                    devObjPtr->tableFormatInfo[SKERNEL_TABLE_FORMAT_TTI_DEFAULT_EPORT_E].fieldsInfoPtr,
                    devObjPtr->tableFormatInfo[SKERNEL_TABLE_FORMAT_TTI_DEFAULT_EPORT_E].fieldsNamePtr,
                    SMEM_SIP6_30_TTI_DEFAULT_EPORT_TABLE_FIELDS_LSDU_CONSTANT_E,
                    0xf);
            }
        }
    }

    maxPort = devObjPtr->limitedResources.ePort;
    /* loop on eports */
    for(port = 0 ; port < maxPort ; port++)
    {
        /* init the TTI eport table */
        {
            memPtr = smemMemGet(devObjPtr,
                SMEM_LION2_TTI_POST_TTI_LOOKUP_INGRESS_EPORT_TABLE_TBL_MEM(devObjPtr, port));

            snetFieldFromEntry_GT_U32_Set(devObjPtr,
                memPtr,
                devObjPtr->tableFormatInfo[SKERNEL_TABLE_FORMAT_TTI_EPORT_ATTRIBUTES_E].formatNamePtr,
                port,
                devObjPtr->tableFormatInfo[SKERNEL_TABLE_FORMAT_TTI_EPORT_ATTRIBUTES_E].fieldsInfoPtr,
                devObjPtr->tableFormatInfo[SKERNEL_TABLE_FORMAT_TTI_EPORT_ATTRIBUTES_E].fieldsNamePtr,
                SMEM_LION3_TTI_EPORT_ATTRIBUTES_TABLE_FIELDS_PHY_SRC_MC_FILTERING_EN,
                1);/* 'Enable filter '*/
        }


        /* initialize the E2PHY table */
        {
            memPtr = smemMemGet(devObjPtr,SMEM_LION2_EQ_E2PHY_TBL_MEM(devObjPtr, port));

            snetFieldFromEntry_GT_U32_Set(devObjPtr,
                memPtr,
                devObjPtr->tableFormatInfo[SKERNEL_TABLE_FORMAT_E2PHY_E].formatNamePtr,
                port,
                devObjPtr->tableFormatInfo[SKERNEL_TABLE_FORMAT_E2PHY_E].fieldsInfoPtr,
                devObjPtr->tableFormatInfo[SKERNEL_TABLE_FORMAT_E2PHY_E].fieldsNamePtr,
                SMEM_LION3_EQ_E2PHY_TABLE_FIELDS_TARGET_DEVICE,
                ownDev);/*Own Device*/

            snetFieldFromEntry_GT_U32_Set(devObjPtr,
                memPtr,
                devObjPtr->tableFormatInfo[SKERNEL_TABLE_FORMAT_E2PHY_E].formatNamePtr,
                port,
                devObjPtr->tableFormatInfo[SKERNEL_TABLE_FORMAT_E2PHY_E].fieldsInfoPtr,
                devObjPtr->tableFormatInfo[SKERNEL_TABLE_FORMAT_E2PHY_E].fieldsNamePtr,
                SMEM_LION3_EQ_E2PHY_TABLE_FIELDS_TARGET_PHYSICAL_PORT,
                (port & (SMEM_CHT_IS_SIP6_GET(devObjPtr)    ? 0x3FF/*10bits*/ :
                         SMEM_CHT_IS_SIP5_20_GET(devObjPtr) ? 0x1FF/*9bits*/  :
                         0xFF/*8bits*/)));
        }


        {/* initialize the L2I EPort Table */
            memPtr = smemMemGet(devObjPtr,SMEM_LION2_BRIDGE_INGRESS_EPORT_ATTRIBUTE_TBL_MEM(devObjPtr, port));
            snetFieldFromEntry_GT_U32_Set(devObjPtr,
                        memPtr,
                        devObjPtr->tableFormatInfo[SKERNEL_TABLE_FORMAT_BRIDGE_INGRESS_EPORT_E].formatNamePtr,
                        port,
                        devObjPtr->tableFormatInfo[SKERNEL_TABLE_FORMAT_BRIDGE_INGRESS_EPORT_E].fieldsInfoPtr,
                        devObjPtr->tableFormatInfo[SKERNEL_TABLE_FORMAT_BRIDGE_INGRESS_EPORT_E].fieldsNamePtr,
                        SMEM_LION3_L2I_EPORT_TABLE_FIELDS_EN_LEARN_ON_TRAP_IEEE_RSRV_MC,
                        1);
            snetFieldFromEntry_GT_U32_Set(devObjPtr,
                        memPtr,
                        devObjPtr->tableFormatInfo[SKERNEL_TABLE_FORMAT_BRIDGE_INGRESS_EPORT_E].formatNamePtr,
                        port,
                        devObjPtr->tableFormatInfo[SKERNEL_TABLE_FORMAT_BRIDGE_INGRESS_EPORT_E].fieldsInfoPtr,
                        devObjPtr->tableFormatInfo[SKERNEL_TABLE_FORMAT_BRIDGE_INGRESS_EPORT_E].fieldsNamePtr,
                        SMEM_LION3_L2I_EPORT_TABLE_FIELDS_ARP_MAC_SA_MIS_DROP_EN,
                        1);
        }

        /* need to initialize the HA egress EPort Att Table 2 */
        memPtr = smemMemGet(devObjPtr,SMEM_LION2_HA_EGRESS_EPORT_ATTRIBUTE_2_TBL_MEM(devObjPtr, port));
        snetFieldFromEntry_GT_U32_Set(devObjPtr,
            memPtr,
            devObjPtr->tableFormatInfo[SKERNEL_TABLE_FORMAT_HA_EGRESS_EPORT_2_E].formatNamePtr,
            port,/* the eport */
            devObjPtr->tableFormatInfo[SKERNEL_TABLE_FORMAT_HA_EGRESS_EPORT_2_E].fieldsInfoPtr,
            devObjPtr->tableFormatInfo[SKERNEL_TABLE_FORMAT_HA_EGRESS_EPORT_2_E].fieldsNamePtr,
            SMEM_LION3_HA_EPORT_TABLE_2_MIRROR_TO_ANALYZER_KEEP_TAGS,
            1);

        /* need to initialize the EGF_SHT egress EPort Table */
        memPtr = smemMemGet(devObjPtr,SMEM_LION2_EGF_SHT_EGRESS_EPORT_TBL_MEM(devObjPtr, port));
        /* check if not initialized by the reg file (check only word 0) */
        if(0 == snetFieldValueGet(memPtr , 0 ,32))
        {
            /*Egress eVLAN Filtering Enable*/
            snetFieldValueSet(memPtr,4,1,1);
            if(SMEM_CHT_IS_SIP5_15_GET(devObjPtr))
            {
                /* STP membership enable */
                snetFieldValueSet(memPtr,7,1,1);
            }
        }

        /* need to initialize the EQ Ingress ePort Table */
        /* there are 4 eports in each entry ... so init 4 eports every time */
        if(0 ==(port % 4))
        {
            memPtr = smemMemGet(devObjPtr,SMEM_LION2_EQ_INGRESS_EPORT_TBL_MEM(devObjPtr, port));
            if(SMEM_CHT_IS_SIP5_10_GET(devObjPtr))
            {
                snetFieldValueSet(memPtr,0,32,0x1E3C78F);
            }
            else
            {
                snetFieldValueSet(memPtr,0,32,0x0F0F0F0F);
            }
        }


        /*,{&STRING_FOR_UNIT_NAME(UNIT_EQ_1),            0x00100000,         0x0000f800,   8192,    0x4      }*/
        memPtr = smemMemGet(devObjPtr,SMEM_LION2_EQ_L2_ECMP_TBL_MEM(devObjPtr, port));
        *memPtr = 0x0000f800;

    }

    {/* ingress default vlan entry (vlan 1) */
        vlan = 1;
        memPtr = smemMemGet(devObjPtr, SMEM_CHT_VLAN_TBL_MEM(devObjPtr, vlan));

        snetFieldFromEntry_GT_U32_Set(devObjPtr,
                    memPtr,
                    devObjPtr->tableFormatInfo[SKERNEL_TABLE_FORMAT_BRIDGE_INGRESS_EVLAN_E].formatNamePtr,
                    vlan,
                    devObjPtr->tableFormatInfo[SKERNEL_TABLE_FORMAT_BRIDGE_INGRESS_EVLAN_E].fieldsInfoPtr,
                    devObjPtr->tableFormatInfo[SKERNEL_TABLE_FORMAT_BRIDGE_INGRESS_EVLAN_E].fieldsNamePtr,
                    SMEM_LION3_L2I_INGRESS_VLAN_TABLE_FIELDS_VALID,
                    1);

        snetFieldFromEntry_GT_U32_Set(devObjPtr,
                    memPtr,
                    devObjPtr->tableFormatInfo[SKERNEL_TABLE_FORMAT_BRIDGE_INGRESS_EVLAN_E].formatNamePtr,
                    vlan,
                    devObjPtr->tableFormatInfo[SKERNEL_TABLE_FORMAT_BRIDGE_INGRESS_EVLAN_E].fieldsInfoPtr,
                    devObjPtr->tableFormatInfo[SKERNEL_TABLE_FORMAT_BRIDGE_INGRESS_EVLAN_E].fieldsNamePtr,
                    SMEM_LION3_L2I_INGRESS_VLAN_TABLE_FIELDS_NEW_SRC_ADDR_IS_NOT_SECURITY_BREACH,
                    1);

        /* set all ports as members */
        regAddr = SMEM_LION2_BRIDGE_INGRESS_PORT_MEMBERSHIP_TBL_MEM(devObjPtr, vlan);
        startBit = 0;
        startBit = startBit;
        regPtr = smemMemGet(devObjPtr, regAddr);
        regPtr = regPtr;

        /* All devices until sip 6 had default vlan 1 with all ports members.
           For BC3 the default for vlan 1 was 256 ports ( and not 512 ports )
           NOTE: the CIDER does not shows the default vlan 1 configurations !!
           In sip6 vlan 1 does not have default configurations because the various ports modes.
           */
        if(!SMEM_CHT_IS_SIP6_GET(devObjPtr))
        {
            if(SMEM_CHT_IS_SIP5_25_GET(devObjPtr))
            {
                maxPort = devObjPtr->limitedResources.phyPort;
            }
            else
            {
                maxPort = MIN(256,devObjPtr->limitedResources.phyPort);
            }

            for(port = 0; port < maxPort ; port += 32)
            {
                /* set next batch of 32 ports (total of 256 ports) */
                snetFieldValueSet(regPtr,
                            startBit + port,/*from this port*/
                            32,  /*set 32 ports */
                            0xFFFFFFFF);
            }
        }

    }/*vlan 1*/

    maxVlan  = devObjPtr->limitedResources.eVid;
    /* ingress vlan table */
    for(vlan = 0 ; vlan < maxVlan ; vlan++)
    {
        memPtr = smemMemGet(devObjPtr, SMEM_CHT_VLAN_TBL_MEM(devObjPtr, vlan));
        snetFieldFromEntry_GT_U32_Set(devObjPtr,
                    memPtr,
                    devObjPtr->tableFormatInfo[SKERNEL_TABLE_FORMAT_BRIDGE_INGRESS_EVLAN_E].formatNamePtr,
                    vlan,
                    devObjPtr->tableFormatInfo[SKERNEL_TABLE_FORMAT_BRIDGE_INGRESS_EVLAN_E].fieldsInfoPtr,
                    devObjPtr->tableFormatInfo[SKERNEL_TABLE_FORMAT_BRIDGE_INGRESS_EVLAN_E].fieldsNamePtr,
                    SMEM_LION3_L2I_INGRESS_VLAN_TABLE_FIELDS_FLOOD_EVIDX,
                    0xFFF);

        snetFieldFromEntry_GT_U32_Set(devObjPtr,
                    memPtr,
                    devObjPtr->tableFormatInfo[SKERNEL_TABLE_FORMAT_BRIDGE_INGRESS_EVLAN_E].formatNamePtr,
                    vlan,
                    devObjPtr->tableFormatInfo[SKERNEL_TABLE_FORMAT_BRIDGE_INGRESS_EVLAN_E].fieldsInfoPtr,
                    devObjPtr->tableFormatInfo[SKERNEL_TABLE_FORMAT_BRIDGE_INGRESS_EVLAN_E].fieldsNamePtr,
                    SMEM_LION3_L2I_INGRESS_VLAN_TABLE_FIELDS_NA_MSG_TO_CPU_EN,
                    1);
    }

    /* next code added to reduce the time of initialization .
        When those tables init were in REG file , it took about 56 seconds
                to init the 8 port groups of Lion3
        From this function code --> 7 seconds (instead of 56)
    */
    if(GT_TRUE ==
        smemIsDeviceMemoryOwner(devObjPtr,SMEM_XCAT_L2_PORT_ISOLATION_REG(devObjPtr, 0)))
    {
        GT_U32  portIsolationNum =
                SMEM_CHT_IS_SIP6_30_GET(devObjPtr) ? (4*_1K + 64) :
                SMEM_CHT_IS_SIP6_10_GET(devObjPtr) ?
                (4*_1K + 128) :
                (2*_1K + 128) ;
        GT_U32  numOfWords = devObjPtr->tablesInfo.l3PortIsolation.paramInfo[0].step / 4;
        GT_U32  *l2baseAddrPtr = smemMemGet(devObjPtr, SMEM_XCAT_L2_PORT_ISOLATION_REG(devObjPtr, 0));
        GT_U32  *l3baseAddrPtr = smemMemGet(devObjPtr, SMEM_XCAT_L3_PORT_ISOLATION_REG(devObjPtr, 0));

        /* sanity check for last valid index */
        (void)smemMemGet(devObjPtr,SMEM_XCAT_L2_PORT_ISOLATION_REG(devObjPtr, portIsolationNum-1));
        (void)smemMemGet(devObjPtr,SMEM_XCAT_L3_PORT_ISOLATION_REG(devObjPtr, portIsolationNum-1));

        /* those are EGF_SHT tables that are 'shared' between every 2 port groups */

        /* init all words in L2 Port Isolation table */
        for(index = 0; index < (numOfWords * portIsolationNum) ;index++,l2baseAddrPtr++)
        {
            *l2baseAddrPtr = 0xFFFFFFFF;
        }
        /* init all words in L3 Port Isolation table */
        for(index = 0; index < (numOfWords * portIsolationNum) ;index++,l3baseAddrPtr++)
        {
            *l3baseAddrPtr = 0xFFFFFFFF;
        }
    }



    if(0 == devObjPtr->ipvxEcmpIndirectMaxNumEntries)
    {
    }
    else
    if(!SMEM_CHT_IS_SIP6_GET(devObjPtr))
    {
        /* sip 5.25 : access IPvX ECMP indirect table

                BWC compatibility to the Legacy ECMP/QoS mode :
               The BWC compatibility will be achieved by setting the correct default
               value to the new indirection table.
               Default value should be as follow:
               Entry 0: Default value of 0
               Entry 1: Default value of 1
               ..
               Entry n: Default value of n
            */
        for (index=0;index < devObjPtr->ipvxEcmpIndirectMaxNumEntries;index++)
        {
            /* access IPvX ECMP indirect table : default 1:1 ... */
            smemRegSet(devObjPtr,
                SMEM_SIP5_25_IPVX_ECMP_POINTER_ENTRY_TBL_MEM(devObjPtr, index),
                index);
        }
    }
    else
    {

        /* sip 6 : 4 pointers in line */
        for (index=0;index < devObjPtr->ipvxEcmpIndirectMaxNumEntries;index++)
        {
            memPtr = smemMemGet(devObjPtr,
                SMEM_SIP5_25_IPVX_ECMP_POINTER_ENTRY_TBL_MEM(devObjPtr, index));
            /* 4 pointers in line */
            for( ii = 0 ; ii < 4 ; ii++)
            {
                /* access IPvX ECMP indirect table : default 1:1 ... */
                snetFieldValueSet(&memPtr[0],ii*15, 15 , index*4 + ii);
            }
        }
    }

    if(devObjPtr->limitedResources.smuIrfNum)/* SMU unit */
    {
        for(index = 0 ; index < devObjPtr->limitedResources.smuIrfNum ; index++)
        {
            memPtr = smemMemGet(devObjPtr,
                SMEM_SIP6_30_SMU_SNG_IRF_TBL_MEM(devObjPtr,index));

            SNET_TABLE_ENTRY_FIELD_SET(devObjPtr,memPtr,index,
                        SMEM_SIP6_30_SMU_IRF_SNG_TABLE_FIELDS_IRF_RECOVERY_SEQUENCE_NUM_E,
                        0xFFFF,
                        SKERNEL_TABLE_FORMAT_SMU_IRF_SNG_E);
        }
    }

    if(devObjPtr->limitedResources.preqSrfNum)/* PREQ unit */
    {
        for(index = 0 ; index < devObjPtr->limitedResources.preqSrfNum ; index++)
        {
            memPtr = smemMemGet(devObjPtr,
                SMEM_SIP6_30_PREQ_SRF_CONFIG_TBL_MEM(devObjPtr,index));

            SNET_TABLE_ENTRY_FIELD_SET(devObjPtr,memPtr,index,
                        SMEM_SIP6_30_PREQ_SRF_CONFIG_TABLE_FIELDS_RECOV_SEQ_NUM_E,
                        0xFFFF,
                        SKERNEL_TABLE_FORMAT_PREQ_SRF_CONFIG_E);

        }
    }


}


/**
* @internal smemChtInit2 function
* @endinternal
*
* @brief   Init memory module for a Cht device - after the load of the default
*         registers file
* @param[in] devObjPtr                - pointer to device object.
*/
void smemChtInit2
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr
)
{
    GT_U32  value;  /*register value*/
    GT_U32  port;   /* port iterator */
    GT_U32  tmpIndex;/*tmp index*/
    GT_U32  currAddrOffset;/* current address offset from start of TCAM (in BYTEs) */
    GT_U32  numOfPipes = devObjPtr->numOfPipes ? devObjPtr->numOfPipes : 1;
    GT_U32  regAddr;
    GT_U32  ii;
    GT_BIT  isDxContolPipe = SMEM_IS_PIPE_FAMILY_GET(devObjPtr) ? 0 : 1;

    if(SMEM_CHT_IS_SIP5_GET(devObjPtr))
    {
        /* init tables format */
        snetIngressTablesFormatInit(devObjPtr);

        devObjPtr->isPciCompatible = 1;
    }
    else
    if(SKERNEL_IS_XCAT3_BASED_DEV(devObjPtr))
    {
        devObjPtr->isPciCompatible = 1;
    }
    else
    {
        /* get the value that was given in the register file for the bit of PCIEn */
        smemRegFldGet(devObjPtr,SMEM_CHT_GLB_CTRL_REG(devObjPtr), 20, 1, &devObjPtr->isPciCompatible);
    }

    /*get RxByteSwap value */
    smemRegFldGet(devObjPtr,SMEM_CHT_SDMA_CONFIG_REG(devObjPtr), 6, 1, &devObjPtr->rxByteSwap);

    /*get TxByteSwap value */
    smemRegFldGet(devObjPtr,SMEM_CHT_SDMA_CONFIG_REG(devObjPtr), 23, 1, &devObjPtr->txByteSwap);

    /* set the vendor Id into the register */
    smemRegFldSet(devObjPtr,SMEM_CHT_VENDOR_ID_REG(devObjPtr),0,16,
                    SMEM_CHT_MARVELL_VENDOR_ID_CNS);

    /* set the PCI registers according to info also got from the 'registers' file */
    smemChtInitPciRegistres(devObjPtr);

    if(devObjPtr->numOfPipes)
    {
        if(SMAIN_NOT_VALID_CNS != UNIT_BASE_ADDR_GET_ALLOW_NON_EXIST_UNIT_MAC(devObjPtr,UNIT_GOP))
        {
            /* update the old reg DB about PIPE offset (Bobcat3) */
            updateGopOldDbWithPipes(devObjPtr, UNIT_BASE_ADDR_GET_MAC(devObjPtr, UNIT_GOP));
        }

        if(SMAIN_NOT_VALID_CNS != UNIT_BASE_ADDR_GET_ALLOW_NON_EXIST_UNIT_MAC(devObjPtr,UNIT_BASE_RAVEN_0))
        {
            /* update the old reg DB about PIPE offset (Raven) */
            updateGopOldDbWithPipes(devObjPtr, UNIT_BASE_ADDR_GET_MAC(devObjPtr, UNIT_BASE_RAVEN_0));
        }
    }

    if(!isDxContolPipe)
    {
        /* skip smemCht3Init2(...) */
    }
    else
    if(SKERNEL_IS_CHEETAH3_DEV(devObjPtr))
    {
        smemCht3Init2(devObjPtr) ;
    }

    /* check the tables info initialization */
    smemChtTablesInfoCheck(devObjPtr);

    simForcePrintf("Start checking the Registers DB initialization (may take few seconds !) ... \n");
    /* check the reg DB initialization */
    smemChtRegDbCheck(devObjPtr);
    simForcePrintf("Done checking DB \n");

    if(isDxContolPipe && !SMEM_CHT_IS_SIP6_GET(devObjPtr) )
    {
        smemRegGet(devObjPtr, SMEM_CHT_MCST_TBL_MEM(devObjPtr,0xFFF),&value);

        if(value == 0)  /* was not set in the 'Registers file' */
        {
            /* set the ports into the VIDX 0xFFF */
            if(devObjPtr->portsNumber < 31)
            {
                /* note : bit 0 is 'reserved' (for CPU port) */
                smemRegFldSet(devObjPtr, SMEM_CHT_MCST_TBL_MEM(devObjPtr,0xFFF), 1,31,
                    SMEM_BIT_MASK(devObjPtr->portsNumber));
            }
            else /* for future device */
            {
                smemRegFldSet(devObjPtr, SMEM_CHT_MCST_TBL_MEM(devObjPtr,0xFFF), 1,31,
                    0x7FFFFFFF);
                smemRegSet(devObjPtr, SMEM_CHT_MCST_TBL_MEM(devObjPtr,0xFFF) + 4,
                    SMEM_BIT_MASK((devObjPtr->portsNumber - 31)));
            }
        }
    }

    for (port = 0; port < SNET_CHEETAH_MAX_PORT_NUM(devObjPtr); port++)
    {
        if(!IS_CHT_VALID_PORT(devObjPtr,port))
        {
            continue;
        }

        if(isDxContolPipe)
        {
            /* Ports current time window */
            snetChtL2iPortCurrentTimeWindowGet(devObjPtr, port/*mac*/,port/*physical*/,
                                               &devObjPtr->portsArr[port].portCurrentTimeWindow);
        }

        if(devObjPtr->portsArr[port].state == SKERNEL_PORT_STATE_CG_100G_E)
        {
            /* check if port can stay at this mode or need to be 10G/1G port */
            /* calling function smemChtActiveWrite100GMacControl0() */
            regAddr = SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->GOP.ports_100G_CG_MAC[port].CG_CONVERTERS.CGMAControl0;
            smemRegUpdateAfterRegFile(devObjPtr,regAddr,1);
        }

    }

    if(SMEM_CHT_IS_SIP5_GET(devObjPtr) && isDxContolPipe)
    {
        for(ii = 0; ii < numOfPipes ; ii++)
        {
            /* allow both pipes of bobcat3 to get the same configuration */
            smemSetCurrentPipeId(devObjPtr,ii);
            if(numOfPipes > 1)
            {
                simForcePrintf("start additional load of default values for memories in pipe[%d] \n",
                    ii);
            }
            else
            {
                simForcePrintf("start additional load of default values for memories \n");
            }
            smemSip5AdditionalDefaultRegInit(devObjPtr);
        }

        smemSetCurrentPipeId(devObjPtr,0);/*restore*/

        simForcePrintf("ended additional load of default values for memories \n");
    }/*sip5*/

    if(SKERNEL_IS_CHEETAH3_DEV(devObjPtr) && (!SMEM_CHT_IS_SIP5_GET(devObjPtr)))
    {
        /* in function snetCht3L3iTcamLookUp(...) to optimize the lookup in the
           tcam we use 'pointer offsets' and not access the memory manager on
           every tcam index change

           but still we want to catch bugs in TCAM allocated memory.
        */

        /******************************/
        /* so check index = 1/4 table */
        /******************************/
        tmpIndex = devObjPtr->routeTcamInfo.numEntriesIpv4/4;
        currAddrOffset =  tmpIndex * devObjPtr->routeTcamInfo.entryWidth * 4;
        /*  Get pointer to Tcam data entry */
        smemMemGet(devObjPtr,SSMEM_CHT3_TCAM_X_DATA_TBL_MEM(devObjPtr) + currAddrOffset);

        /******************************/
        /* so check index = 2/4 table */
        /******************************/
        tmpIndex = devObjPtr->routeTcamInfo.numEntriesIpv4/2;
        currAddrOffset =  tmpIndex * devObjPtr->routeTcamInfo.entryWidth * 4;
        /*  Get pointer to Tcam data entry */
        smemMemGet(devObjPtr,SSMEM_CHT3_TCAM_X_DATA_TBL_MEM(devObjPtr) + currAddrOffset);

        /******************************/
        /* so check index = 3/4 table */
        /******************************/
        tmpIndex = (devObjPtr->routeTcamInfo.numEntriesIpv4 * 3)/4;
        currAddrOffset =  tmpIndex * devObjPtr->routeTcamInfo.entryWidth * 4;
        /*  Get pointer to Tcam data entry */
        smemMemGet(devObjPtr,SSMEM_CHT3_TCAM_X_DATA_TBL_MEM(devObjPtr) + currAddrOffset);

        /******************************/
        /* so check index = 4/4 table */
        /******************************/
        tmpIndex = devObjPtr->routeTcamInfo.numEntriesIpv4;
        if (SKERNEL_IS_XCAT_DEV(devObjPtr))
        {
            tmpIndex -= 4;
        }
        else
        {
            tmpIndex -= 1;
        }

        currAddrOffset =  tmpIndex * devObjPtr->routeTcamInfo.entryWidth * 4;
        /*  Get pointer to Tcam data entry */
        smemMemGet(devObjPtr,SSMEM_CHT3_TCAM_X_DATA_TBL_MEM(devObjPtr) + currAddrOffset);
    }

    if(devObjPtr->supportMaskAuFuMessageToCpuOnNonLocal_with3BitsCoreId)
    {
        /* make sure that the bit is set by default */
        smemRegFldSet(devObjPtr,SMEM_LION2_FDB_METAL_FIX_REG(devObjPtr),16,1,1);
    }

    return;
}

/*******************************************************************************
*   smemChtFindMem
*
* DESCRIPTION:
*       Return pointer to the register's or tables's memory.
*
* INPUTS:
*       devObjPtr   - pointer to device object.
*       address     - address of memory(register or table).
*
* OUTPUTS:
*     activeMemPtrPtr - pointer to the active memory entry or NULL if not exist.
*
* RETURNS:
*        pointer to the memory location
*        NULL - if memory not exist
*
* COMMENTS:
*
*
*******************************************************************************/
static void * smemChtFindMem
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SCIB_MEMORY_ACCESS_TYPE accessType,
    IN GT_U32                  address,
    IN GT_U32                  memSize,
    OUT SMEM_ACTIVE_MEM_ENTRY_STC ** activeMemPtrPtr
)
{
    void                * memPtr;
    SMEM_CHT_DEV_MEM_INFO  * devMemInfoPtr;
    GT_32                 index;
    GT_U32                param;

    if (devObjPtr == 0)
    {
        skernelFatalError("smemChtFindMem: illegal pointer \n");
    }
    memPtr = 0;
    devMemInfoPtr = (SMEM_CHT_DEV_MEM_INFO  *)devObjPtr->deviceMemory;

    /* Find PCI registers memory  */
    if (SMEM_ACCESS_PCI_FULL_MAC(accessType))
    {
        memPtr = smemChtPciReg(devObjPtr, accessType, address, memSize, 0);

        /* find PCI active memory entry */
        if (activeMemPtrPtr != NULL)
        {
            for (index = 0; index < (SMEM_ACTIVE_PCI_MEM_TABLE_SIZE - 1);
                  index++)
            {
                /* check address */
                if ((address & smemChtPciActiveTable[index].mask)
                      == smemChtPciActiveTable[index].address)
                {
                    *activeMemPtrPtr = &smemChtPciActiveTable[index];
                    break;
                }
            }
        }
        return memPtr;
    }

    index = (address & REG_SPEC_FUNC_INDEX) >>
                 SMEM_CHT_UNIT_INDEX_FIRST_BIT_CNS;

    if (((address >> SMEM_PHY_UNIT_INDEX_CNS) & 0x3) == 0x3)
    {
      memPtr  = smemChtPhyReg(devObjPtr, address, memSize);
    }
    else
    {
      param   = devMemInfoPtr->common.specFunTbl[index].specParam;
      memPtr  = devMemInfoPtr->common.specFunTbl[index].specFun(devObjPtr,
                                                                accessType,
                                                                address,
                                                                memSize,
                                                                param);
    }

    /* find active memory entry */
    if (activeMemPtrPtr != NULL)
    {
        *activeMemPtrPtr = NULL;
        for (index = 0; index < (SMEM_ACTIVE_MEM_TABLE_SIZE - 1); index++)
        {
            /* check address */
            if ((address & smemChtActiveTable[index].mask)
                 == smemChtActiveTable[index].address)
            {
                *activeMemPtrPtr = &smemChtActiveTable[index];
                break;
            }
        }
    }

    return memPtr;
}

/*******************************************************************************
*   smemChtGlobalReg
*
* DESCRIPTION:
*       Global Registers, TWSI, CPU port, SDMA and PCI internal registers
*
* INPUTS:
*       devObjPtr   - pointer to device object.
*       address     - address of memory(register or table).
*       memSize     - size of the requested memory
*       param       - extra parameter might be used in
*
* OUTPUTS:
*       None.
*
* RETURNS:
*        pointer to the memory location
*        NULL - if memory not exist
*
* COMMENTS:
*
*
*******************************************************************************/
static GT_U32 *  smemChtGlobalReg(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SCIB_MEMORY_ACCESS_TYPE accessType,
    IN GT_U32 address,
    IN GT_U32 memSize,
    IN GT_UINTPTR param
)
{
    SMEM_CHT_DEV_MEM_INFO  * devMemInfoPtr;
    GT_U32              * regValPtr;
    GT_U32                index;

    regValPtr = 0;
    devMemInfoPtr = (SMEM_CHT_DEV_MEM_INFO  *)devObjPtr->deviceMemory;

    /* Global registers */
    if ((address & 0x1FFFFF00) == 0x0)
    {
        index = (address & 0xFF) / 0x4;
        CHECK_MEM_BOUNDS(devMemInfoPtr->globalMem.globReg,
                         devMemInfoPtr->globalMem.globRegNum,
                         index, memSize);
        regValPtr = &devMemInfoPtr->globalMem.globReg[index];
    }
    else
     /* SDMA registers */
    if ((address & 0x1FFFF000) == 0x00002000) {
        index = (address & 0xFFF) / 0x4;
        CHECK_MEM_BOUNDS(devMemInfoPtr->globalMem.sdmaReg,
                         devMemInfoPtr->globalMem.sdmaRegNum,
                         index, memSize);
        regValPtr = &devMemInfoPtr->globalMem.sdmaReg[index];
    }

    return regValPtr;
}
/*******************************************************************************
*   smemChtTransQueReg
*
* DESCRIPTION:
*       Egress, Transmit Queue and VLAN Configuration Register Map Table,
*       Port TxQ Configuration Register, Hyper.GStack Ports MIB Counters
*
* INPUTS:
*       devObjPtr   - pointer to device object.
*       address     - address of memory(register or table).
*       memSize     - size of the requested memory
*       param       - extra parameter might be used in
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       pointer to the memory location
*       NULL - if memory not exist
*
* COMMENTS:
*
*******************************************************************************/
static GT_U32 *  smemChtTransQueReg(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SCIB_MEMORY_ACCESS_TYPE accessType,
    IN GT_U32 address,
    IN GT_U32 memSize,
    IN GT_UINTPTR param
)
{
    SMEM_CHT_DEV_MEM_INFO  * devMemInfoPtr;
    GT_U32              * regValPtr;
    GT_U32                index;
    GT_U32                group;

    regValPtr = 0;
    devMemInfoPtr = (SMEM_CHT_DEV_MEM_INFO  *)devObjPtr->deviceMemory;

    /* Common registers */
    if ((address & 0x1FFF0000) == 0x01800000)
    {
        index = (address & 0xFFFF) / 0x4;
        CHECK_MEM_BOUNDS(devMemInfoPtr->egrMem.genReg,
                         devMemInfoPtr->egrMem.genRegNum,
                         index, memSize);
        regValPtr = &devMemInfoPtr->egrMem.genReg[index];
    }
    else
    /* Trunks Filtering and Multicast Distribution Configuration Registers,
       Device Map Table, Source-ID Egress Filtering Registers*/
    if ((address & 0x1FFFF000) == 0x01A40000)
    {
        index = (address & 0xFFF) / 0x4;
        CHECK_MEM_BOUNDS(devMemInfoPtr->egrMem.trnFilterReg,
                         devMemInfoPtr->egrMem.trnFilterRegNum,
                         index, memSize);
        regValPtr = &devMemInfoPtr->egrMem.trnFilterReg[index];
    }
    else
    /* Port Watchdog Configuration Registers */
    if ((address & 0x1FFF0000) == 0x01A80000)
    {
        index = (address & 0xFFF) / 0x200;
        CHECK_MEM_BOUNDS(devMemInfoPtr->egrMem.portWdReg,
                         devMemInfoPtr->egrMem.portWdRegNum,
                         index, memSize);
        regValPtr = &devMemInfoPtr->egrMem.portWdReg[index];
    }
    else
    /* Egress MIB Counters */
    if ((address & 0x1FFFFF00) == 0x01B40100)
    {
        index = (address & 0xFF) / 0x4;
        CHECK_MEM_BOUNDS(devMemInfoPtr->egrMem.egrMibCntReg,
                         devMemInfoPtr->egrMem.egrMibCntRegNum,
                         index, memSize);
        regValPtr = &devMemInfoPtr->egrMem.egrMibCntReg[index];
    }
    else
    /* Hyper.GStack Ports MIB Counters */
    if ((address & 0x1FFF0000) >= 0x01C00000 &&
        (address & 0x1FFF0000) <= 0x01C80000)
    {
        group = ((address >> SMEM_CHT_GROUP_INDEX_FIRST_BIT_CNS) & 0xF) / 0x4;
        index = (address & 0xFF) / 0x4;
        CHECK_MEM_BOUNDS(devMemInfoPtr->egrMem.hprMibCntReg[group],
                         devMemInfoPtr->egrMem.hprMibCntRegNum,
                         index, memSize);
        regValPtr = &devMemInfoPtr->egrMem.hprMibCntReg[group][index];
    }
    else
    /* Master XSMI Interface Configuration Registers */
    if ((address & 0x1FFF0000) == 0x01CC0000)
    {
        index = (address & 0xF) / 0x8;
        CHECK_MEM_BOUNDS(devMemInfoPtr->egrMem.xsmiReg,
                         devMemInfoPtr->egrMem.xsmiRegNum,
                         index, memSize);
        regValPtr = &devMemInfoPtr->egrMem.xsmiReg[index];
    }
    else
    /* Egress STC Table */
    if ((address & 0x1FFF0000) == 0x01D40000)
    {
        group = (address & 0xF) / 0x4;
        index = (address & 0xFFF) / 0x200;
        CHECK_MEM_BOUNDS(devMemInfoPtr->egrMem.stcEgrReg[group],
                         devMemInfoPtr->egrMem.stcEgrRegNum,
                         index, memSize);
        regValPtr = &devMemInfoPtr->egrMem.stcEgrReg[group][index];
    }
    else
    /* Tail Drop Profile Configuration Registers */
    if ((address & 0x1FFFF000) == 0x01940000)
    {
        index = (address & 0xFFF) / 0x4;
        CHECK_MEM_BOUNDS(devMemInfoPtr->egrMem.tailDropReg,
                         devMemInfoPtr->egrMem.tailDropRegNum,
                         index, memSize);
        regValPtr = &devMemInfoPtr->egrMem.tailDropReg[index];
    }
    else
    /* Port<n> Token Bucket Configuration Registers */
    if ((address & 0x1FFF000F) == 0x01AC0008)
    {
        index = (address & 0xFFF) / 0x200;
        CHECK_MEM_BOUNDS(devMemInfoPtr->egrMem.egrRateShpReg[0],
                         devMemInfoPtr->egrMem.egrRateShpRegNum,
                         index, memSize);
        regValPtr = &devMemInfoPtr->egrMem.egrRateShpReg[0][index];
    }
    else
    /* Port<n> TC<t> Token Bucket Configuration Registers */
    if (address >= 0x01AC0000 && address <= 0x01AFFE00)
    {
        group = (address & 0xFFF) / 0x200;
        index = (address & 0x3FFFF) / 0x8000;
        CHECK_MEM_BOUNDS(devMemInfoPtr->egrMem.egrRateShpReg[group],
                         devMemInfoPtr->egrMem.egrRateShpRegNum,
                         index, memSize);
        regValPtr = &devMemInfoPtr->egrMem.egrRateShpReg[group][index];
    }
    /* Txq Internal Registers */
    if ((address & 0x01FF0000) >= 0x01840000 &&
        (address & 0x01FF0000) <= 0x01930000)
    {
        group = ((address & 0x1C0000) >> 18) - 1;
        index = (address & 0x3FFFF) / 0x200;
        CHECK_MEM_BOUNDS(devMemInfoPtr->egrMem.txqInternalReg[group],
                         devMemInfoPtr->egrMem.txqInternalRegNum,
                         index, memSize);
        regValPtr = &devMemInfoPtr->egrMem.txqInternalReg[group][index];
    }

    return regValPtr;
}

/*******************************************************************************
*   smemChtEtherBrdgReg
*
* DESCRIPTION:
*       Bridge Configuration Registers
*
* INPUTS:
*       devObjPtr   - pointer to device object.
*       address     - address of memory(register or table).
*       memSize     - size of the requested memory
*       param       - extra parameter might be used in
*
* OUTPUTS:
*       None.
*
* RETURNS:
*        pointer to the memory location
*        NULL - if memory not exist
*
* COMMENTS:
*
*
*******************************************************************************/
static GT_U32 *  smemChtEtherBrdgReg(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SCIB_MEMORY_ACCESS_TYPE accessType,
    IN GT_U32 address,
    IN GT_U32 memSize,
    IN GT_UINTPTR param
)
{
    SMEM_CHT_DEV_MEM_INFO  * devMemInfoPtr;
    GT_U32              * regValPtr;
    GT_U32                index;

    regValPtr = 0;
    devMemInfoPtr = (SMEM_CHT_DEV_MEM_INFO  *)devObjPtr->deviceMemory;
    if ((address & 0xFF000000) == 0x02000000)
    {
        index = (address & 0x000FFFFF) / 4;
        CHECK_MEM_BOUNDS(devMemInfoPtr->brdgMngMem.genReg,
                         devMemInfoPtr->brdgMngMem.genRegsNum,
                         index, memSize);
        regValPtr = &devMemInfoPtr->brdgMngMem.genReg[index];
    }

    return regValPtr;
}

/*******************************************************************************
*   smemChtBufMngReg
*
* DESCRIPTION:
*       Describe a device's buffer management registers memory object
*
* INPUTS:
*       devObjPtr   - pointer to device object.
*       address     - address of memory(register or table).
*       memSize     - size of the requested memory
*       param       - extra parameter might be used in
*
* OUTPUTS:
*       None.
*
* RETURNS:
*        pointer to the memory location
*        NULL - if memory not exist
*
* COMMENTS:
*
*
*******************************************************************************/
static GT_U32 *  smemChtBufMemoryConfReg(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SCIB_MEMORY_ACCESS_TYPE accessType,
    IN GT_U32 address,
    IN GT_U32 memSize,
    IN GT_UINTPTR param
)
{
    SMEM_CHT_DEV_MEM_INFO  * devMemInfoPtr;
    GT_U32              * regValPtr;
    GT_U32                index;

    regValPtr = 0;
    devMemInfoPtr = (SMEM_CHT_DEV_MEM_INFO  *)devObjPtr->deviceMemory;

    /* Buffers Memory Registers */
    if ((address & 0xFFF00000) == 0x07800000)
    {
        index = (address & 0xFFF) / 0x4;
        CHECK_MEM_BOUNDS(devMemInfoPtr->bufMem.bufMemReg,
                         devMemInfoPtr->bufMem.bufMemRegNum,
                         index, memSize);
        regValPtr = &devMemInfoPtr->bufMem.bufMemReg[index];
    }

    return regValPtr;
}

/*******************************************************************************
*   smemChtPortGroupConfReg
*
* DESCRIPTION:
*       Describe a device's Port registers memory object
*
* INPUTS:
*       devObjPtr   - pointer to device object.
*       address     - address of memory(register or table).
*       memSize     - size of the requested memory
*       param       - extra parameter might be used in
*
* OUTPUTS:
*       None.
*
* RETURNS:
*        pointer to the memory location
*        NULL - if memory not exist
*
* COMMENTS:
*
*
*******************************************************************************/
static GT_U32 *  smemChtPortGroupConfReg(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SCIB_MEMORY_ACCESS_TYPE accessType,
    IN GT_U32 address,
    IN GT_U32 memSize,
    IN GT_UINTPTR param
)
{
    SMEM_CHT_DEV_MEM_INFO  * devMemInfoPtr;
    GT_U32              * regValPtr;
    GT_U32                index;
    GT_U32                groupNum;
    GT_U32                gopMaskResult = 0;
    GT_U32                macMibMaskResult = 0;
    GT_U32                ledMaskResult = 0;

    regValPtr = 0;
    devMemInfoPtr = (SMEM_CHT_DEV_MEM_INFO  *)devObjPtr->deviceMemory;

    groupNum = param;

    if (groupNum == 0)
    {
        gopMaskResult =     0x04004000;
        macMibMaskResult =  0x04010000;
        ledMaskResult =     0x04005100;
    }
    else
    if (groupNum == 1)
    {
        gopMaskResult =     0x04804000;
        macMibMaskResult =  0x04810000;
        ledMaskResult =     0x04805100;
    }
    else
    if (groupNum == 2)
    {
        gopMaskResult =     0x05004000;
        macMibMaskResult =  0x05010000;
        ledMaskResult =     0x05005100;
    }
    else
    if (groupNum == 3)
    {
        gopMaskResult =     0x05804000;
        macMibMaskResult =  0x05810000;
        ledMaskResult =     0x05805100;
    }

    /*  Group configuration registers */
    if ((address & 0xFFFFF000) == gopMaskResult)
    {
        index = (address & 0xFFF) / 0x4;
        CHECK_MEM_BOUNDS(devMemInfoPtr->gopCnfMem.gopReg[groupNum],
                         devMemInfoPtr->gopCnfMem.gopRegNum,
                         index, memSize);
        regValPtr = &devMemInfoPtr->gopCnfMem.gopReg[groupNum][index];
    }
    else
    /* MAC MIB Counters registers */
    if ((address & 0xFFFF0000) == macMibMaskResult)
    {
        index = (address & 0xFFF) / 0x4;
        CHECK_MEM_BOUNDS(devMemInfoPtr->gopCnfMem.macMibCountReg[groupNum],
                         devMemInfoPtr->gopCnfMem.macMibCountRegNum,
                         index, memSize);
        regValPtr = &devMemInfoPtr->gopCnfMem.macMibCountReg[groupNum][index];
    }
    else
    /* LED Interfaces Configuration Registers */
    if ((address & 0xFFFFFF00) == ledMaskResult)
    {
        index = (address & 0xFF) / 0x4;
        CHECK_MEM_BOUNDS(devMemInfoPtr->gopCnfMem.ledReg[groupNum],
                         devMemInfoPtr->gopCnfMem.ledRegNum,
                         index, memSize);
        regValPtr = &devMemInfoPtr->gopCnfMem.ledReg[groupNum][index];
    }

    return regValPtr;
}

/*******************************************************************************
*   smemChtMacTableReg
*
* DESCRIPTION:
*       Describe a device's Bridge registers and FDB
*
* INPUTS:
*       devObjPtr   - pointer to device object.
*       address     - address of memory(register or table).
*       memSize     - size of the requested memory
*       param       - extra parameter might be used in
*
* OUTPUTS:
*       None.
*
* RETURNS:
*        pointer to the memory location
*        NULL - if memory not exist
*
* COMMENTS:
*
*
*******************************************************************************/
static GT_U32 *  smemChtMacTableReg(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SCIB_MEMORY_ACCESS_TYPE accessType,
    IN GT_U32 address,
    IN GT_U32 memSize,
    IN GT_UINTPTR param
)
{
    SMEM_CHT_DEV_MEM_INFO  * devMemInfoPtr;
    GT_U32              * regValPtr;
    GT_U32                index;

    regValPtr = 0;
    devMemInfoPtr = (SMEM_CHT_DEV_MEM_INFO  *)devObjPtr->deviceMemory;
    /* FDB Global Configuration Registers */
    if ((address & 0xFF00000) == 0x06000000)
    {
        index = (address & 0xFF) / 0x4;
        CHECK_MEM_BOUNDS(devMemInfoPtr->macFdbMem.fdbReg,
                         devMemInfoPtr->macFdbMem.fdbRegNum,
                         index, memSize);
        regValPtr = &devMemInfoPtr->macFdbMem.fdbReg[index];
    }

    return regValPtr;
}

/*******************************************************************************
*   smemChtInterBufBankReg
*
* DESCRIPTION:
*
*
* INPUTS:
*       devObjPtr   - pointer to device object.
*       address     - address of memory(register or table).
*       memSize     - size of the requested memory
*       param       - extra parameter might be used in
*
* OUTPUTS:
*       None.
*
* RETURNS:
*        pointer to the memory location
*        NULL - if memory not exist
*
* COMMENTS:
*
*
*******************************************************************************/
static GT_U32 *  smemChtInterBufBankReg(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SCIB_MEMORY_ACCESS_TYPE accessType,
    IN GT_U32 address,
    IN GT_U32 memSize,
    IN GT_UINTPTR param
)
{
    SMEM_CHT_DEV_MEM_INFO  * devMemInfoPtr;
    GT_U32              * regValPtr;
    GT_U32                index;
    GT_U32                groupNum;

    regValPtr = 0;
    devMemInfoPtr = (SMEM_CHT_DEV_MEM_INFO  *)devObjPtr->deviceMemory;

    groupNum = param;

    if (address == 0x06800000)
    {
        regValPtr = &devMemInfoPtr->banksMem.bankWriteReg[0];
    }
    else if (address < 0x06980000)
    {
        groupNum = 0;
        index = (address & 0xFFFFF) / 0x4;
        CHECK_MEM_BOUNDS(devMemInfoPtr->banksMem.bankMemReg[groupNum],
                         devMemInfoPtr->banksMem.bankMemRegNum,
                         index, memSize);
        regValPtr = &devMemInfoPtr->banksMem.bankMemReg[groupNum][index];
    }
    else
    /* Buffers Memory Bank1 Write Data Register number */
    {
        groupNum = 1;
        index = (((address  - 0x80000) & 0xFFFFF)) / 0x4;
        CHECK_MEM_BOUNDS(devMemInfoPtr->banksMem.bankMemReg[groupNum],
                         devMemInfoPtr->banksMem.bankMemRegNum,
                         index, memSize);
        regValPtr = &devMemInfoPtr->banksMem.bankMemReg[groupNum][index];
    }

    return regValPtr;
}

/*******************************************************************************
*   smemChtBufMemoryConfReg
*
* DESCRIPTION:
*       Registers, index is group number
*
* INPUTS:
*       devObjPtr   - pointer to device object.
*       address     - address of memory(register or table).
*       memSize     - size of the requested memory
*       param       - extra parameter might be used in
*
* OUTPUTS:
*       None.
*
* RETURNS:
*        pointer to the memory location
*        NULL - if memory not exist
*
* COMMENTS:
*
*
*******************************************************************************/
static GT_U32 *  smemChtBufMngReg(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SCIB_MEMORY_ACCESS_TYPE accessType,
    IN GT_U32 address,
    IN GT_U32 memSize,
    IN GT_UINTPTR param
)
{
    SMEM_CHT_DEV_MEM_INFO  * devMemInfoPtr;
    GT_U32              * regValPtr;
    GT_U32              index;

    regValPtr = 0;
    devMemInfoPtr = (SMEM_CHT_DEV_MEM_INFO  *)devObjPtr->deviceMemory;

    if ((address & 0xFFFFF000) == 0x03000000)
    {
        index = (address & 0xFFF) / 0x4;
        CHECK_MEM_BOUNDS(devMemInfoPtr->bufMngMem.bmReg,
                         devMemInfoPtr->bufMngMem.bmRegNum,
                         index, memSize);
        regValPtr = &devMemInfoPtr->bufMngMem.bmReg[index];
    }

    return regValPtr;
}

/*******************************************************************************
*   smemChtVlanTableReg
*
* DESCRIPTION:
*       Describe a device's Bridge registers and FDB
*
* INPUTS:
*       devObjPtr   - pointer to device object.
*       address     - address of memory(register or table).
*       memSize     - size of the requested memory
*       param       - extra parameter might be used in
*
* OUTPUTS:
*       None.
*
* RETURNS:
*        pointer to the memory location
*        NULL - if memory not exist
*
* COMMENTS:
*
*
*******************************************************************************/
static GT_U32 *  smemChtVlanTableReg(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SCIB_MEMORY_ACCESS_TYPE accessType,
    IN GT_U32 address,
    IN GT_U32 memSize,
    IN GT_UINTPTR param
)
{
    SMEM_CHT_DEV_MEM_INFO  * devMemInfoPtr;
    GT_U32              * regValPtr;
    GT_U32                index;

    regValPtr = 0;
    devMemInfoPtr = (SMEM_CHT_DEV_MEM_INFO  *)devObjPtr->deviceMemory;

    if ((address & 0xFF000000) == 0x0A000000)
    {
        index = (address & 0xFF) / 0x4;
        CHECK_MEM_BOUNDS(devMemInfoPtr->vlanTblMem.vlanCnfReg,
                         devMemInfoPtr->vlanTblMem.vlanCnfRegNum,
                         index, memSize);
        regValPtr = &devMemInfoPtr->vlanTblMem.vlanCnfReg[index];
    }

    return regValPtr;
}

/*******************************************************************************
*   smemChtFatalError
*
* DESCRIPTION:
*       function for non-bound memories
*
* INPUTS:
*       devObjPtr   - pointer to device object.
*       address     - address of memory(register or table).
*       memSize     - size of the requested memory
*       param       - extra parameter might be used in
*
* OUTPUTS:
*       None.
*
* RETURNS:
*        pointer to the memory location
*        NULL - if memory not exist
*
* COMMENTS:
*
*
*******************************************************************************/
GT_U32 *  smemChtFatalError(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SCIB_MEMORY_ACCESS_TYPE accessType,
    IN GT_U32 address,
    IN GT_U32 memSize,
    IN GT_UINTPTR param
)
{
    if(debugModeByPassFatalErrorOnMemNotExists)
    {
        debugModeByPassFatalErrorOnMemNotExists--;/* limit the number of times */

        simWarningPrintf("smemChtFatalError: memory unit not exists for address[0x%8.8x]\n",address);
    }
    else
    {
        if(GT_FALSE == skernelUserDebugInfo.disableFatalError)
        {
            skernelFatalError( "smemChtFatalError: memory unit not exists for address[0x%8.8x]\n",address);
        }
    }

    return NULL;
}

/**
* @internal smemConvertChunkIndexToPointerAsParam function
* @endinternal
*
* @brief   convert the index of chunk memory to a pointer to the memory chunk,
*         and then convert it to GT_U32 to be used as parameter for generic chunk
*         accessing
*/
GT_UINTPTR  smemConvertChunkIndexToPointerAsParam
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32                  index /*not the unitId*/
)
{
    SMEM_CHT_GENERIC_DEV_MEM_INFO  * devMemInfoPtr = devObjPtr->deviceMemory;

    return (GT_UINTPTR)(void*)&devMemInfoPtr->unitMemArr[index];
}

/**
* @internal smemChtInitFuncArray function
* @endinternal
*
* @brief   Init specific functions array.
*
* @param[in,out] devMemInfoPtr            - pointer to device memory object.
*/
static void smemChtInitFuncArray(
    INOUT SMEM_CHT_DEV_MEM_INFO  * devMemInfoPtr
)
{
    GT_U32              i;

    for (i = 0; i < 64; i++) {
        devMemInfoPtr->common.specFunTbl[i].specFun    = smemChtFatalError;
    }
    devMemInfoPtr->common.specFunTbl[0].specFun        = smemChtGlobalReg;

    devMemInfoPtr->common.specFunTbl[3].specFun        = smemChtTransQueReg;

    devMemInfoPtr->common.specFunTbl[4].specFun        = smemChtEtherBrdgReg;

    devMemInfoPtr->common.specFunTbl[6].specFun        = smemChtBufMngReg;

    devMemInfoPtr->common.specFunTbl[8].specFun        = smemChtPortGroupConfReg;
    devMemInfoPtr->common.specFunTbl[8].specParam      = 0;

    devMemInfoPtr->common.specFunTbl[9].specFun        = smemChtPortGroupConfReg;
    devMemInfoPtr->common.specFunTbl[9].specParam      = 1;

    devMemInfoPtr->common.specFunTbl[10].specFun       = smemChtPortGroupConfReg;
    devMemInfoPtr->common.specFunTbl[10].specParam     = 2;

    devMemInfoPtr->common.specFunTbl[11].specFun       = smemChtPortGroupConfReg;
    devMemInfoPtr->common.specFunTbl[11].specParam     = 3;

    devMemInfoPtr->common.specFunTbl[12].specFun       = smemChtMacTableReg;

    devMemInfoPtr->common.specFunTbl[13].specFun       = smemChtInterBufBankReg;
    devMemInfoPtr->common.specFunTbl[13].specParam     = 0;

    devMemInfoPtr->common.specFunTbl[14].specFun       = smemChtInterBufBankReg;
    devMemInfoPtr->common.specFunTbl[14].specParam     = 1;

    devMemInfoPtr->common.specFunTbl[15].specFun       = smemChtBufMemoryConfReg;

    devMemInfoPtr->common.specFunTbl[20].specFun       = smemChtVlanTableReg;

    devMemInfoPtr->common.specFunTbl[21].specFun       = smemChtTriSpeedReg;

    devMemInfoPtr->common.specFunTbl[22].specFun       = smemChtPreEgressReg;

    devMemInfoPtr->common.specFunTbl[23].specFun       = smemChtPclReg;

    devMemInfoPtr->common.specFunTbl[24].specFun       = smemChtPolicerReg;

    /* Tables active memory access functions for visualizer */
    devMemInfoPtr->common.specFunTbl[26].specFun       = smemChtPclTbl;
    devMemInfoPtr->common.specFunTbl[27].specFun       = smemChtMacTbl;
    devMemInfoPtr->common.specFunTbl[28].specFun       = smemChtPolicerTbl;
    devMemInfoPtr->common.specFunTbl[29].specFun       = smemChtTrunkTbl;
    devMemInfoPtr->common.specFunTbl[30].specFun       = smemChtVlanTbl;
    devMemInfoPtr->common.specFunTbl[31].specFun       = smemChtArpTbl;

}

/**
* @internal smemChtInitInternalPhyMem function
* @endinternal
*
* @brief   Allocate and init the PHY memory
*/
static void smemChtInitInternalPhyMem
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SMEM_CHT_DEV_COMMON_MEM_INFO  * commonDevMemInfoPtr
)
{
    GT_U32  i;

    /* PHY related Registers ieeeXauiReg[0..3]->0x000 - 0x020 */
    /* extXauiReg[0..3]->0x000 - 0x02D */
    commonDevMemInfoPtr->phyMem.PhyIdieeeXauiRegNum = PHY_IEEE_XAUI_REGS_NUM;
    commonDevMemInfoPtr->phyMem.PhyIdextXauiRegNum = PHY_EXT_XAUI_REGS_NUM;
    for (i = 0; i < PHY_XAUI_DEV_NUM; i++)
    {
        commonDevMemInfoPtr->phyMem.PhyId0ieeeXauiReg[i] =
            smemDevMemoryCalloc(devObjPtr, PHY_IEEE_XAUI_REGS_NUM, sizeof(SMEM_PHY_REGISTER));
        if (commonDevMemInfoPtr->phyMem.PhyId0ieeeXauiReg[i] == 0)
        {
            skernelFatalError("smemChtAllocSpecMemory: allocation error\n");
        }
        commonDevMemInfoPtr->phyMem.PhyId0extXauiReg[i] =
            smemDevMemoryCalloc(devObjPtr, PHY_EXT_XAUI_REGS_NUM, sizeof(SMEM_PHY_REGISTER));
        if (commonDevMemInfoPtr->phyMem.PhyId0extXauiReg[i] == 0)
        {
            skernelFatalError("smemChtAllocSpecMemory: allocation error\n");
        }

        commonDevMemInfoPtr->phyMem.PhyId1ieeeXauiReg[i] =
            smemDevMemoryCalloc(devObjPtr, PHY_IEEE_XAUI_REGS_NUM, sizeof(SMEM_PHY_REGISTER));
        if (commonDevMemInfoPtr->phyMem.PhyId1ieeeXauiReg[i] == 0)
        {
            skernelFatalError("smemChtAllocSpecMemory: allocation error\n");
        }
        commonDevMemInfoPtr->phyMem.PhyId1extXauiReg[i] =
            smemDevMemoryCalloc(devObjPtr, PHY_EXT_XAUI_REGS_NUM, sizeof(SMEM_PHY_REGISTER));
        if (commonDevMemInfoPtr->phyMem.PhyId1extXauiReg[i] == 0)
        {
            skernelFatalError("smemChtAllocSpecMemory: allocation error\n");
        }

        commonDevMemInfoPtr->phyMem.PhyId2ieeeXauiReg[i] =
            smemDevMemoryCalloc(devObjPtr, PHY_IEEE_XAUI_REGS_NUM, sizeof(SMEM_PHY_REGISTER));
        if (commonDevMemInfoPtr->phyMem.PhyId2ieeeXauiReg[i] == 0)
        {
            skernelFatalError("smemChtAllocSpecMemory: allocation error\n");
        }
        commonDevMemInfoPtr->phyMem.PhyId2extXauiReg[i] =
            smemDevMemoryCalloc(devObjPtr, PHY_EXT_XAUI_REGS_NUM, sizeof(SMEM_PHY_REGISTER));
        if (commonDevMemInfoPtr->phyMem.PhyId2extXauiReg[i] == 0)
        {
            skernelFatalError("smemChtAllocSpecMemory: allocation error\n");
        }
    }
}

/**
* @internal smemChtInitInternalSimMemory function
* @endinternal
*
* @brief   Allocate and init the internal simulation memory
*/
void smemChtInitInternalSimMemory
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SMEM_CHT_DEV_COMMON_MEM_INFO  * commonDevMemInfoPtr
)
{
    GT_U32  *internalMemPtr;
    GT_U32  numOfTiles;
    GT_U32  ii;

    /* allocate and init the internal memory */
    commonDevMemInfoPtr->internalSimMem.internalRegNum = CHT_INTERNAL_SIMULATION_USE_MEM_TOTAL_E;
    commonDevMemInfoPtr->internalSimMem.internalRegPtr =
        smemDevMemoryCalloc(devObjPtr, CHT_INTERNAL_SIMULATION_USE_MEM_TOTAL_E,sizeof(SMEM_REGISTER));
    if (commonDevMemInfoPtr->internalSimMem.internalRegPtr == 0)
    {
        skernelFatalError("smemChtInitInternalSimMemory: allocation error\n");
    }

    /* init the specific bit 0 of this memory */
    internalMemPtr = CHT_INTERNAL_MEM_PTR(devObjPtr,
        CHT_INTERNAL_SIMULATION_USE_MEM_SECURITY_BREACH_STATUS_E);

    *internalMemPtr = 1;

    /* init the AUQ FIFO */
    numOfTiles = (devObjPtr->numOfTiles == 0) ? 1 : devObjPtr->numOfTiles;
    for(ii = 0 ; ii < numOfTiles ; ii++)
    {
        memset(commonDevMemInfoPtr->auqFifoMem[ii].macUpdFifoRegs, 0xFF,
               MAC_UPD_FIFO_REGS_NUM * sizeof(SMEM_REGISTER));
    }

    memset(commonDevMemInfoPtr->gtsIngressFifoMem.gtsFifoRegs, 0xFF,
           GTS_FIFO_REGS_NUM * sizeof(SMEM_REGISTER));

    memset(commonDevMemInfoPtr->gtsEgressFifoMem.gtsFifoRegs, 0xFF,
           GTS_FIFO_REGS_NUM * sizeof(SMEM_REGISTER));

    /* init the PHY memory */
    smemChtInitInternalPhyMem(devObjPtr,commonDevMemInfoPtr);

    /* clear AU structure */
    smemChtResetAuq(devObjPtr);

    if(! SKERNEL_DEVICE_FAMILY_CHEETAH_1_ONLY(devObjPtr))
    {
        /* clear FU structure */
        smemCht2ResetFuq(devObjPtr);
    }

    devObjPtr->isPciCompatible = 0;/*just for the initialization*/
    devObjPtr->globalInterruptCauseRegWriteBitmap_pci = 0;
    devObjPtr->globalInterruptCauseRegWriteBitmap_nonPci = 0;

    /* init the byte swapping */
    devObjPtr->txByteSwap = 0;
    devObjPtr->rxByteSwap = 0;

    /* set defaults that may be override by the device */
    SET_IF_ZERO_MAC(devObjPtr->flexFieldNumBitsSupport.phyPort , 6); /*64 values*/
    SET_IF_ZERO_MAC(devObjPtr->flexFieldNumBitsSupport.trunkId , 7); /*128 values*/
    SET_IF_ZERO_MAC(devObjPtr->flexFieldNumBitsSupport.ePort , 0);  /* only for eArch device */
    SET_IF_ZERO_MAC(devObjPtr->flexFieldNumBitsSupport.eVid , 0);   /* only for eArch device */
    SET_IF_ZERO_MAC(devObjPtr->flexFieldNumBitsSupport.eVidx , 0);  /* only for eArch device */
    SET_IF_ZERO_MAC(devObjPtr->flexFieldNumBitsSupport.hwDevNum , 5);/*32 values*/
    SET_IF_ZERO_MAC(devObjPtr->flexFieldNumBitsSupport.sstId , 5);/*32 values*/

    SET_IF_ZERO_MAC(devObjPtr->limitedResources.eVid , 0);   /* only for eArch device */
    SET_IF_ZERO_MAC(devObjPtr->limitedResources.ePort , 0);  /* only for eArch device */
    SET_IF_ZERO_MAC(devObjPtr->limitedResources.phyPort , 64);  /* only for eArch device */

    if(devObjPtr->numWordsAlignmentInAuMsg == 0)
    {
        /*set default only if was not already set */
        devObjPtr->numWordsAlignmentInAuMsg = 4;
    }

    if(devObjPtr->numOfWordsInAuMsg == 0)
    {
        /*set default only if was not already set */
        devObjPtr->numOfWordsInAuMsg = 4;
    }

    devObjPtr->devMemBackUpPtr = smemCheetahBackUpMemory;

    if (devObjPtr->devMemSpecificDeviceUnitAlloc)
    {
        devObjPtr->devMemSpecificDeviceUnitAlloc(devObjPtr);
    }

    return;
}

/**
* @internal smemCheetahBackUpMemory function
* @endinternal
*
* @brief   Definition of backup/restore memory function
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] readWrite                - GT_TRUE  - backup device memory data
*                                      GT_FALSE - restore device memory data
*/
GT_VOID smemCheetahBackUpMemory
(
    IN SKERNEL_DEVICE_OBJECT *devObjPtr,
    IN GT_BOOL                readWrite
)
{
    GT_U32 *memPtr;                     /* Memory pointer */
    GT_U32  address;                    /* Memory space address */
    GT_CHAR buffer[SIM_OS_CNF_FILE_MAX_LINE_LENGTH_CNS] = {0}; /* Read/Write buffer */
    GT_CHAR keyStr[100] = {0};                 /* Key string */
    GT_CHAR *fileNamePtr;               /* Dump memory file name */
    FILE    *memObjFilePtr;             /* Dump memory file descriptor */

    /* Read dump memory file name from INI file */
    sprintf(keyStr, "dev%u_mem_dump", devObjPtr->deviceId);
    if (SIM_OS_MAC(simOsGetCnfValue)("system", keyStr,
            SIM_OS_CNF_FILE_MAX_LINE_LENGTH_CNS, buffer))
    {
        fileNamePtr = buffer;
    }
    else if (NULL == devObjPtr->portGroupSharedDevObjPtr)
    {
        /* devObjPtr->portGroupSharedDevObjPtr not initialized */
        fileNamePtr = keyStr;
    }
    else
    {
        sprintf(keyStr, "device%u_asic%u_mem_dump.txt", devObjPtr->portGroupSharedDevObjPtr->deviceId, devObjPtr->deviceId);
        fileNamePtr = keyStr;
    }

    if (readWrite == GT_FALSE)
    {
        /* Restore data from dump memory file */
        smainMemConfigLoad(devObjPtr, fileNamePtr, GT_TRUE/*use smem*/);
        return;
    }

    skernelUserDebugInfo.disableFatalError = GT_TRUE;

    /* Open file for write */
    memObjFilePtr = fopen(fileNamePtr, "w");

    /* disable buffering: useful for debug with "tail -f" */
    setbuf(memObjFilePtr, NULL);

    /* Iterate all address space and check address memory type */
    for (address = 0; address < (GT_U32)0xfffffff0; address += 0x4)
    {
        /* check device type and call device search memory function*/
        memPtr = smemMemGet(devObjPtr, address);

        /* Does memory exist and set */
        if (memPtr && memPtr[0] && memPtr[0] != 0xcdcdcdcd)
        {
            /* dump memory value */
            sprintf(buffer, "%08x %08x\n", address, memPtr[0]);
            fprintf(memObjFilePtr, buffer);
        }
    }

    fclose(memObjFilePtr);

    skernelUserDebugInfo.disableFatalError = GT_FALSE;
}

/**
* @internal smemChtAllocSpecMemory function
* @endinternal
*
* @brief   Allocate address type specific memories.
*
* @param[in,out] devMemInfoPtr            - pointer to device memory object.
*/
static void smemChtAllocSpecMemory(
    INOUT  SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SMEM_CHT_DEV_MEM_INFO  * devMemInfoPtr
)
{
    GT_U8 i;

    /* init the internal memory */
    smemChtInitInternalSimMemory(devObjPtr,&devMemInfoPtr->common);

    /* Global register, TWSI, CPU port, SDMA and PCI internal registers */
    devMemInfoPtr->globalMem.globRegNum = GLB_REGS_NUM;
    devMemInfoPtr->globalMem.globReg =
        smemDevMemoryCalloc(devObjPtr, GLB_REGS_NUM, sizeof(SMEM_REGISTER));
    if (devMemInfoPtr->globalMem.globReg == 0)
    {
        skernelFatalError("smemChtAllocSpecMemory: allocation error\n");
    }

    devMemInfoPtr->globalMem.sdmaRegNum = GLB_SDMA_REGS_NUM;
    devMemInfoPtr->globalMem.sdmaReg =
        smemDevMemoryCalloc(devObjPtr, GLB_SDMA_REGS_NUM, sizeof(SMEM_REGISTER));
    if (devMemInfoPtr->globalMem.sdmaReg == 0)
    {
        skernelFatalError("smemChtAllocSpecMemory: allocation error\n");
    }

    /* Egress, Transmit Queue and VLAN Configuration Register Map Table,
       Port TxQ Configuration Register, Hyper.GStack Ports MIB Counters */

    devMemInfoPtr->egrMem.genRegNum = EGR_GLB_REGS_NUM;
    devMemInfoPtr->egrMem.genReg =
        smemDevMemoryCalloc(devObjPtr, EGR_GLB_REGS_NUM, sizeof(SMEM_REGISTER));
    if (devMemInfoPtr->egrMem.genReg == 0)
    {
        skernelFatalError("smemChtAllocSpecMemory: allocation error\n");
    }

    devMemInfoPtr->egrMem.txqInternalRegNum = TXQ_INTERNAL_REGS_NUM;
    for (i = 0; i < 4; i++)
    {
        devMemInfoPtr->egrMem.txqInternalReg[i] =
            smemDevMemoryCalloc(devObjPtr, TXQ_INTERNAL_REGS_NUM, sizeof(SMEM_REGISTER));
        if (devMemInfoPtr->egrMem.txqInternalReg[i] == 0)
        {
            skernelFatalError("smemCht2AllocSpecMemory: allocation error\n");
        }
    }

    devMemInfoPtr->egrMem.trnFilterRegNum = EGR_TRUNK_FILTER_REGS_NUM;
    devMemInfoPtr->egrMem.trnFilterReg =
        smemDevMemoryCalloc(devObjPtr, EGR_TRUNK_FILTER_REGS_NUM, sizeof(SMEM_REGISTER));
    if (devMemInfoPtr->egrMem.trnFilterReg == 0)
    {
        skernelFatalError("smemChtAllocSpecMemory: allocation error\n");
    }

    devMemInfoPtr->egrMem.portWdRegNum = EGR_PORT_WD_REGS_NUM;
    devMemInfoPtr->egrMem.portWdReg =
        smemDevMemoryCalloc(devObjPtr, EGR_PORT_WD_REGS_NUM, sizeof(SMEM_REGISTER));
    if (devMemInfoPtr->egrMem.portWdReg == 0)
    {
        skernelFatalError("smemChtAllocSpecMemory: allocation error\n");
    }

    devMemInfoPtr->egrMem.egrMibCntRegNum = EGR_MIB_CNT_REGS_NUM;
    devMemInfoPtr->egrMem.egrMibCntReg =
        smemDevMemoryCalloc(devObjPtr, EGR_MIB_CNT_REGS_NUM, sizeof(SMEM_REGISTER));
    if (devMemInfoPtr->egrMem.egrMibCntReg == 0)
    {
        skernelFatalError("smemChtAllocSpecMemory: allocation error\n");
    }

    devMemInfoPtr->egrMem.hprMibCntRegNum = EGR_HYP_MIB_REGS_NUM;
    for (i = 0; i < 3; i++)
    {
        devMemInfoPtr->egrMem.hprMibCntReg[i] =
            smemDevMemoryCalloc(devObjPtr, EGR_HYP_MIB_REGS_NUM, sizeof(SMEM_REGISTER));
        if (devMemInfoPtr->egrMem.hprMibCntReg[i] == 0)
        {
            skernelFatalError("smemChtAllocSpecMemory: allocation error\n");
        }
    }
    devMemInfoPtr->egrMem.xsmiRegNum = EGR_XSMI_REGS_NUM;
    devMemInfoPtr->egrMem.xsmiReg =
        smemDevMemoryCalloc(devObjPtr, EGR_XSMI_REGS_NUM, sizeof(SMEM_REGISTER));
    if (devMemInfoPtr->egrMem.xsmiReg == 0)
    {
        skernelFatalError("smemChtAllocSpecMemory: allocation error\n");
    }

    devMemInfoPtr->egrMem.stcEgrRegNum = EGR_STC_TBL_REGS_NUM;
    for (i = 0; i < 3; i++)
    {
        devMemInfoPtr->egrMem.stcEgrReg[i] =
            smemDevMemoryCalloc(devObjPtr, EGR_STC_TBL_REGS_NUM, sizeof(SMEM_REGISTER));
        if (devMemInfoPtr->egrMem.stcEgrReg[i] == 0)
        {
            skernelFatalError("smemChtAllocSpecMemory: allocation error\n");
        }
    }

    devMemInfoPtr->egrMem.tailDropRegNum = EGR_TALE_DROP_REGS_NUM;
    devMemInfoPtr->egrMem.tailDropReg =
        smemDevMemoryCalloc(devObjPtr, EGR_TALE_DROP_REGS_NUM, sizeof(SMEM_REGISTER));
    if (devMemInfoPtr->egrMem.tailDropReg == 0)
    {
        skernelFatalError("smemChtAllocSpecMemory: allocation error\n");
    }

    devMemInfoPtr->egrMem.egrRateShpRegNum = EGR_RATE_SHAPES_REGS_NUM;
    for (i = 0; i < 9; i++)
    {
        devMemInfoPtr->egrMem.egrRateShpReg[i] =
            smemDevMemoryCalloc(devObjPtr, EGR_RATE_SHAPES_REGS_NUM, sizeof(SMEM_REGISTER));
        if (devMemInfoPtr->egrMem.egrRateShpReg[i] == 0)
        {
            skernelFatalError("smemChtAllocSpecMemory: allocation error\n");
        }
    }

    /* Bridge Management registers memory object */

    devMemInfoPtr->brdgMngMem.genRegsNum = BRDG_ETH_BRDG_REGS_NUM;
    devMemInfoPtr->brdgMngMem.genReg =
        smemDevMemoryCalloc(devObjPtr, BRDG_ETH_BRDG_REGS_NUM, sizeof(SMEM_REGISTER));
    if (devMemInfoPtr->brdgMngMem.genReg == 0)
    {
        skernelFatalError("smemChtAllocSpecMemory: allocation error\n");
    }

    /* Buffer management registers memory object */

    devMemInfoPtr->bufMngMem.bmRegNum = BUF_MNG_REGS_NUM;
    devMemInfoPtr->bufMngMem.bmReg =
        smemDevMemoryCalloc(devObjPtr, BUF_MNG_REGS_NUM, sizeof(SMEM_REGISTER));
    if (devMemInfoPtr->bufMngMem.bmRegNum == 0)
    {
        skernelFatalError("smemChtAllocSpecMemory: allocation error\n");
    }

    /* Ports groups configuration registers (port0 through port5),
       LEDs interfaces configuration registers and Master SMI
       interfaces registers                                         */

    devMemInfoPtr->gopCnfMem.gopRegNum = GOP_CONF_REGS_NUM;
    devMemInfoPtr->gopCnfMem.macMibCountRegNum = GOP_MAC_MIB_REGS_NUM;
    devMemInfoPtr->gopCnfMem.ledRegNum = GOP_LED_REGS_NUM;
    for (i = 0; i < 4; i++)
    {
        devMemInfoPtr->gopCnfMem.gopReg[i] =
            smemDevMemoryCalloc(devObjPtr, devMemInfoPtr->gopCnfMem.gopRegNum, sizeof(SMEM_REGISTER));
        if (devMemInfoPtr->gopCnfMem.gopReg[i] == 0)
        {
            skernelFatalError("smemChtAllocSpecMemory: allocation error\n");
        }

        devMemInfoPtr->gopCnfMem.macMibCountReg[i] =
            smemDevMemoryCalloc(devObjPtr, devMemInfoPtr->gopCnfMem.macMibCountRegNum,
                                                    sizeof(SMEM_REGISTER));
        if (devMemInfoPtr->gopCnfMem.macMibCountReg[i] == 0)
        {
            skernelFatalError("smemChtAllocSpecMemory: allocation error\n");
        }

        devMemInfoPtr->gopCnfMem.ledReg[i] =
            smemDevMemoryCalloc(devObjPtr, devMemInfoPtr->gopCnfMem.ledRegNum, sizeof(SMEM_REGISTER));
        if (devMemInfoPtr->gopCnfMem.ledReg[i] == 0)
        {
            skernelFatalError("smemChtAllocSpecMemory: allocation error\n");
        }
    }

    /* Bridge registers and FDB */

    devMemInfoPtr->macFdbMem.fdbRegNum = MAC_FDB_REGS_NUM;
    devMemInfoPtr->macFdbMem.fdbReg =
        smemDevMemoryCalloc(devObjPtr, MAC_FDB_REGS_NUM, sizeof(SMEM_REGISTER));
    if (devMemInfoPtr->macFdbMem.fdbReg == 0)
    {
        skernelFatalError("smemChtAllocSpecMemory: allocation error\n");
    }

    devMemInfoPtr->macFdbMem.macTblRegNum = MAC_TBL_REGS_NUM;
    devMemInfoPtr->macFdbMem.macTblReg =
        smemDevMemoryCalloc(devObjPtr, MAC_TBL_REGS_NUM, sizeof(SMEM_REGISTER));
    if (devMemInfoPtr->macFdbMem.macTblReg == 0)
    {
        skernelFatalError("smemChtAllocSpecMemory: allocation error\n");
    }

    /* Buffers memory banks data register */

    devMemInfoPtr->banksMem.bankWriteRegNum = BANK_WRITE_REGS_NUM;
    devMemInfoPtr->banksMem.bankWriteReg =
        smemDevMemoryCalloc(devObjPtr, BANK_WRITE_REGS_NUM, sizeof(SMEM_REGISTER));
    if (devMemInfoPtr->banksMem.bankWriteReg == 0)
    {
        skernelFatalError("smemChtAllocSpecMemory: allocation error\n");
    }

    devMemInfoPtr->banksMem.bankMemRegNum = BANK_MEM_REGS_NUM;
    for (i = 0; i < 2; i++)
    {
        devMemInfoPtr->banksMem.bankMemReg[i] =
            smemDevMemoryCalloc(devObjPtr, BANK_MEM_REGS_NUM, sizeof(SMEM_REGISTER));
        if (devMemInfoPtr->banksMem.bankMemReg[i] == 0)
        {
            skernelFatalError("smemChtAllocSpecMemory: allocation error\n");
        }
    }

    /* Buffers Memory, Ingress MAC Errors Indications and
       Egress Header Alteration Register Map Table registers            */

    devMemInfoPtr->bufMem.bufMemRegNum = BUF_MEM_REGS_NUM;
    devMemInfoPtr->bufMem.bufMemReg =
        smemDevMemoryCalloc(devObjPtr, BUF_MEM_REGS_NUM, sizeof(SMEM_REGISTER));
    if (devMemInfoPtr->bufMem.bufMemReg == 0)
    {
        skernelFatalError("smemChtAllocSpecMemory: allocation error\n");
    }

    /* VLAN Table, Multicast Groups Table, and Span State Group Table */

    devMemInfoPtr->vlanTblMem.vlanCnfRegNum = VTL_VLAN_CONF_REGS_NUM;
    devMemInfoPtr->vlanTblMem.vlanCnfReg =
        smemDevMemoryCalloc(devObjPtr, VTL_VLAN_CONF_REGS_NUM, sizeof(SMEM_REGISTER));
    if (devMemInfoPtr->vlanTblMem.vlanCnfReg == 0)
    {
        skernelFatalError("smemChtAllocSpecMemory: allocation error\n");
    }

    devMemInfoPtr->vlanTblMem.vlanTblRegNum = VTL_VLAN_TBL_REGS_NUM;
    devMemInfoPtr->vlanTblMem.vlanTblReg =
        smemDevMemoryCalloc(devObjPtr, VTL_VLAN_TBL_REGS_NUM, sizeof(SMEM_REGISTER));
    if (devMemInfoPtr->vlanTblMem.vlanTblReg == 0)
    {
        skernelFatalError("smemChtAllocSpecMemory: allocation error\n");
    }

    devMemInfoPtr->vlanTblMem.mcstTblRegNum = VTL_MCST_TBL_REGS_NUM;
    devMemInfoPtr->vlanTblMem.mcstTblReg =
        smemDevMemoryCalloc(devObjPtr, VTL_MCST_TBL_REGS_NUM, sizeof(SMEM_REGISTER));
    if (devMemInfoPtr->vlanTblMem.mcstTblReg == 0)
    {
        skernelFatalError("smemChtAllocSpecMemory: allocation error\n");
    }

    devMemInfoPtr->vlanTblMem.spanTblRegNum = VTL_STP_TBL_REGS_NUM;
    devMemInfoPtr->vlanTblMem.spanTblReg =
        smemDevMemoryCalloc(devObjPtr, VTL_STP_TBL_REGS_NUM/*5000*/, sizeof(SMEM_REGISTER));
    if (devMemInfoPtr->vlanTblMem.spanTblReg == 0)
    {
        skernelFatalError("smemChtAllocSpecMemory: allocation error\n");
    }

    /* Tri-Speed Ports MAC */

    devMemInfoPtr->triSpeedPortsMem.triSpeedPortsRegNum = TRI_SPEED_REGS_NUM;
    devMemInfoPtr->triSpeedPortsMem.triSpeedPortsReg =
        smemDevMemoryCalloc(devObjPtr, TRI_SPEED_REGS_NUM, sizeof(SMEM_REGISTER));
    if (devMemInfoPtr->triSpeedPortsMem.triSpeedPortsReg == 0)
    {
        skernelFatalError("smemChtAllocSpecMemory: allocation error\n");
    }

    /* Pre-Egress Engine Registers Map Table, Trunk Table,
       QoSProfile to QoS Table, CPU Code Table, Data Access Statistical Rate
       Limits Table Data Access, Ingress STC Table Registers                */

    devMemInfoPtr->preegressMem.portsRegNum = PRE_EGR_PORTS_REGS_NUM;
    devMemInfoPtr->preegressMem.portsReg =
        smemDevMemoryCalloc(devObjPtr, PRE_EGR_PORTS_REGS_NUM, sizeof(SMEM_REGISTER));
    if (devMemInfoPtr->preegressMem.portsReg == 0)
    {
        skernelFatalError("smemChtAllocSpecMemory: allocation error\n");
    }

    devMemInfoPtr->preegressMem.genRegNum = PRE_EGR_GEN_REGS_NUM;
    devMemInfoPtr->preegressMem.genReg =
        smemDevMemoryCalloc(devObjPtr, PRE_EGR_GEN_REGS_NUM, sizeof(SMEM_REGISTER));
    if (devMemInfoPtr->preegressMem.genReg == 0)
    {
        skernelFatalError("smemChtAllocSpecMemory: allocation error\n");
    }

    devMemInfoPtr->preegressMem.trunkTblRegNum = PRE_EGR_TRUNK_TBL_REGS_NUM;
    for (i = 0; i < 8; i++)
    {
        devMemInfoPtr->preegressMem.trunkTblReg[i] =
            smemDevMemoryCalloc(devObjPtr, PRE_EGR_TRUNK_TBL_REGS_NUM, sizeof(SMEM_REGISTER));
        if (devMemInfoPtr->preegressMem.trunkTblReg[i] == 0)
        {
            skernelFatalError("smemChtAllocSpecMemory: allocation error\n");
        }
    }

    devMemInfoPtr->preegressMem.qosTblRegNum = PRE_QOS_TBL_REGS_NUM;
    devMemInfoPtr->preegressMem.qosTblReg =
        smemDevMemoryCalloc(devObjPtr, PRE_QOS_TBL_REGS_NUM, sizeof(SMEM_REGISTER));
    if (devMemInfoPtr->preegressMem.qosTblReg == 0)
    {
        skernelFatalError("smemChtAllocSpecMemory: allocation error\n");
    }

    devMemInfoPtr->preegressMem.cpuCodeTblRegNum = PRE_CPU_CODE_TBL_REGS_NUM;
    devMemInfoPtr->preegressMem.cpuCodeTblReg =
        smemDevMemoryCalloc(devObjPtr, PRE_CPU_CODE_TBL_REGS_NUM, sizeof(SMEM_REGISTER));
    if (devMemInfoPtr->preegressMem.cpuCodeTblReg == 0)
    {
        skernelFatalError("smemChtAllocSpecMemory: allocation error\n");
    }

    devMemInfoPtr->preegressMem.statRateTblRegNum = PRE_STAT_RATE_TBL_REGS_NUM;
    devMemInfoPtr->preegressMem.statRateTblReg =
        smemDevMemoryCalloc(devObjPtr, PRE_STAT_RATE_TBL_REGS_NUM/*300*/, sizeof(SMEM_REGISTER));
    if (devMemInfoPtr->preegressMem.statRateTblReg == 0)
    {
        skernelFatalError("smemChtAllocSpecMemory: allocation error\n");
    }

    devMemInfoPtr->preegressMem.portInStcTblRegNum = PRE_PORTS_INGR_STC_REGS_NUM;
    devMemInfoPtr->preegressMem.portInStcTblReg =
        smemDevMemoryCalloc(devObjPtr, PRE_PORTS_INGR_STC_REGS_NUM, sizeof(SMEM_REGISTER));
    if (devMemInfoPtr->preegressMem.portInStcTblReg == 0)
    {
        skernelFatalError("smemChtAllocSpecMemory: allocation error\n");
    }

    /* PCL registers and TCAM memory space */

    devMemInfoPtr->pclMem.pclConfRegNum = PCL_CONF_REGS_NUM;
    devMemInfoPtr->pclMem.pclConfReg =
        smemDevMemoryCalloc(devObjPtr, PCL_CONF_REGS_NUM, sizeof(SMEM_REGISTER));
    if (devMemInfoPtr->pclMem.pclConfReg == 0)
    {
        skernelFatalError("smemChtAllocSpecMemory: allocation error\n");
    }

    devMemInfoPtr->pclMem.tcamRegNum = PCL_TCAM_REGS_NUM;
    devMemInfoPtr->pclMem.tcamReg =
        smemDevMemoryCalloc(devObjPtr, PCL_TCAM_REGS_NUM, sizeof(SMEM_REGISTER));
    if (devMemInfoPtr->pclMem.tcamReg == 0)
    {
        skernelFatalError("smemChtAllocSpecMemory: allocation error\n");
    }

    devMemInfoPtr->pclMem.pclIntRegNum = PCL_INTERN_REGS_NUM;
    devMemInfoPtr->pclMem.pclIntReg =
        smemDevMemoryCalloc(devObjPtr, PCL_INTERN_REGS_NUM, sizeof(SMEM_REGISTER));
    if (devMemInfoPtr->pclMem.pclIntReg == 0)
    {
        skernelFatalError("smemChtAllocSpecMemory: allocation error\n");
    }

    devMemInfoPtr->pclMem.portVlanQosTblRegNum = PCL_PORT_VLAN_QOS_REGS_NUM;
    devMemInfoPtr->pclMem.portVlanQosTblReg =
        smemDevMemoryCalloc(devObjPtr, PCL_PORT_VLAN_QOS_REGS_NUM, sizeof(SMEM_REGISTER));
    if (devMemInfoPtr->pclMem.portVlanQosTblReg == 0)
    {
        skernelFatalError("smemChtAllocSpecMemory: allocation error\n");
    }

    devMemInfoPtr->pclMem.protBasedVlanQosTblRegNum =
                                            PCL_PROT_BASED_VLAN_QOS_REGS_NUM;
    for (i = 0; i < 8; i++)
    {
        devMemInfoPtr->pclMem.protBasedVlanQosTblReg[i] =
            smemDevMemoryCalloc(devObjPtr, PCL_PROT_BASED_VLAN_QOS_REGS_NUM, sizeof(SMEM_REGISTER));
        if (devMemInfoPtr->pclMem.protBasedVlanQosTblReg[i] == 0)
        {
            skernelFatalError("smemChtAllocSpecMemory: allocation error\n");
        }
    }

    devMemInfoPtr->pclMem.pceActionsTblRegNum = PCL_ACTION_TBL_REGS_NUM;
    devMemInfoPtr->pclMem.pceActionsTblReg =
        smemDevMemoryCalloc(devObjPtr, PCL_ACTION_TBL_REGS_NUM, sizeof(SMEM_REGISTER));
    if (devMemInfoPtr->pclMem.pceActionsTblReg == 0)
    {
        skernelFatalError("smemChtAllocSpecMemory: allocation error\n");
    }

    devMemInfoPtr->pclMem.pclIdTblRegNum = PCL_ID_CONF_TBL_REGS_NUM;
    devMemInfoPtr->pclMem.pclIdTblReg =
        smemDevMemoryCalloc(devObjPtr, PCL_ID_CONF_TBL_REGS_NUM, sizeof(SMEM_REGISTER));
    if (devMemInfoPtr->pclMem.pclIdTblReg == 0)
    {
        skernelFatalError("smemChtAllocSpecMemory: allocation error\n");
    }

    /* Policer registers and meters memory space */
    devMemInfoPtr->policerMem.policerRegNum = POLICER_REGS_NUM;
    devMemInfoPtr->policerMem.policerReg =
        smemDevMemoryCalloc(devObjPtr, POLICER_REGS_NUM, sizeof(SMEM_REGISTER));
    if (devMemInfoPtr->policerMem.policerReg == 0)
    {
        skernelFatalError("smemChtAllocSpecMemory: allocation error\n");
    }

    devMemInfoPtr->policerMem.policerTblRegNum = POLICER_TBL_REGS_NUM;
    devMemInfoPtr->policerMem.policerTblReg =
        smemDevMemoryCalloc(devObjPtr, POLICER_TBL_REGS_NUM, sizeof(SMEM_REGISTER));
    if (devMemInfoPtr->policerMem.policerTblReg == 0)
    {
        skernelFatalError("smemChtAllocSpecMemory: allocation error\n");
    }

    devMemInfoPtr->policerMem.policerQosRmTblRegNum =
                                                POLICER_QOS_REM_TBL_REGS_NUM;
    devMemInfoPtr->policerMem.policerQosRmTblReg =
        smemDevMemoryCalloc(devObjPtr, POLICER_QOS_REM_TBL_REGS_NUM, sizeof(SMEM_REGISTER));
    if (devMemInfoPtr->policerMem.policerQosRmTblReg == 0)
    {
        skernelFatalError("smemChtAllocSpecMemory: allocation error\n");
    }

    devMemInfoPtr->policerMem.policerCntTblRegNum = POLICER_COUNT_TBL_REGS_NUM;
    devMemInfoPtr->policerMem.policerCntTblReg =
        smemDevMemoryCalloc(devObjPtr, POLICER_COUNT_TBL_REGS_NUM, sizeof(SMEM_REGISTER));
    if (devMemInfoPtr->policerMem.policerCntTblReg == 0)
    {
        skernelFatalError("smemChtAllocSpecMemory: allocation error\n");
    }

    /* PCI Registers */
    devMemInfoPtr->pciMem.pciRegNum = PCI_REGS_NUM;
    devMemInfoPtr->pciMem.pciReg =
        smemDevMemoryCalloc(devObjPtr, PCI_REGS_NUM, sizeof(SMEM_REGISTER));
    if (devMemInfoPtr->pciMem.pciReg == 0)
    {
        skernelFatalError("smemChtAllocSpecMemory: allocation error\n");
    }


    /* ARP table memory */
    devMemInfoPtr->ipMem.arpTblRegNum = ARP_TABLE_REGS_NUM;
    devMemInfoPtr->ipMem.arpTblReg =
        smemDevMemoryCalloc(devObjPtr, ARP_TABLE_REGS_NUM, sizeof(SMEM_REGISTER));
    if (devMemInfoPtr->ipMem.arpTblReg == 0)
    {
        skernelFatalError("smemChtAllocSpecMemory: allocation error\n");
    }

    /*  VLAN/Port MAC SA Table */
    devMemInfoPtr->ipMem.macSaTblRegNum = MAC_SA_TABLE_REGS_NUM;
    devMemInfoPtr->ipMem.macSaTblReg =
        smemDevMemoryCalloc(devObjPtr, MAC_SA_TABLE_REGS_NUM, sizeof(SMEM_REGISTER));
    if (devMemInfoPtr->ipMem.macSaTblReg == 0)
    {
        skernelFatalError("smemChtAllocSpecMemory: allocation error\n");
    }

    memset(devMemInfoPtr->common.auqFifoMem[0].macUpdFifoRegs, 0xFF,
        MAC_UPD_FIFO_REGS_NUM * sizeof(SMEM_REGISTER));


    devObjPtr->devFindMemFunPtr = (void *)smemChtFindMem;
}

/**
* @internal smemChtActiveWriteFdbMsg function
* @endinternal
*
* @brief   Write to the Message from CPU Register3 - activate update FDB message.
*
* @param[in] devObjPtr                - device object PTR.
* @param[in] address                  - Address for ASIC memory.
* @param[in] memPtr                   - Pointer to the register's memory in the simulation.
* @param[in] param                    - Registers' specific parameter.
* @param[in] inMemPtr                 - Pointer to the memory to get register's content.
*/
void smemChtActiveWriteFdbMsg (
    IN         SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN         GT_U32   address,
    IN         GT_U32   memSize,
    IN         GT_U32 * memPtr,
    IN         GT_UINTPTR   param,
    IN         GT_U32 * inMemPtr
)
{
    GT_U32            * regPtr;         /* register's entry pointer */
    GT_U32              line;           /* line number */
    GT_U32              rdWr;           /* Read or write operation */
    GT_U32              regAddr;        /* table memory address */

    rdWr = (*inMemPtr >> 1) & 0x1;
    line = (*inMemPtr >> 2) & (devObjPtr->fdbNumEntries-1);

    regAddr = SMEM_CHT_MAC_TBL_MEM(devObjPtr,line);

    if (rdWr)
    {
        /* Mac Table Access Data */
        regPtr = smemMemGet(devObjPtr, SMEM_CHT_MAC_TBL_ACC_DATA0_REG(devObjPtr));
        smemMemSet(devObjPtr, regAddr, regPtr, 4);
    }
    else
    {
        regPtr = smemMemGet(devObjPtr, regAddr);
        /* Mac Table Access Data */
        smemMemSet(devObjPtr, SMEM_CHT_MAC_TBL_ACC_DATA0_REG(devObjPtr), regPtr, 4);
    }
}


/**
* @internal smemChtCheckAndSendLinkChange function
* @endinternal
*
* @brief   check if port changed link , and send message (to smain task) if needed
*         IMPORTENT : the function doing the the operation of:
*         memPtr = inMemPtr ;
* @param[in] devObjPtr                - device object PTR.
* @param[in] port                     - the  to check/update the link status and generate interrupt.
*                                      bypassCheckLinkChange - indication to bypass the check of 'old state'/'new state'
* @param[in] memPtr                   - Pointer to the register's memory in the simulation.
* @param[in] inMemPtr                 - Pointer to the memory to get register's content.
*/
void smemChtCheckAndSendLinkChange (
    IN         SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN         GT_U32   port,
    IN         GT_U32 * memPtr,
    IN         GT_U32 * inMemPtr
)
{
    SBUF_BUF_ID bufferId;       /* buffer */
    GT_U32  newLinkValue;       /* new link value */
    GT_U8  * dataPtr;         /* pointer to the data in the buffer */
    GT_U32 dataSize;          /* data size */
    GT_U32  linkStateWhenNoForce; /* state of port when no forcing of link UP , or down*/
    GT_U32   oldLinkState;
    GT_U32   bypassCheckLinkChange = 0;
    GT_U32   forceLinkChangeInterrupt = 0;
    GT_U32   forceLinkDown,old_forceLinkDown;
    GT_U32   forceLinkUp,old_forceLinkUp;
    GT_U32  old_PortMACReset,PortMACReset;

    /* get values before the 'write to the register' */

    /* when the 'old PortMACReset' was 'reset' and the current is 'not reset' */
    /* we send link change interrupt ... bypass the check of 'old state'/'new state'  !!! */
    old_PortMACReset = snetChtPortMacFieldGet(devObjPtr, port,
        SNET_CHT_PORT_MAC_FIELDS_PortMACReset_E);

    old_forceLinkDown = snetChtPortMacFieldGet(devObjPtr, port,
        SNET_CHT_PORT_MAC_FIELDS_forceLinkDown_E);
    old_forceLinkUp = snetChtPortMacFieldGet(devObjPtr, port,
        SNET_CHT_PORT_MAC_FIELDS_forceLinkPass_E);

    /* update the values in the register due to 'write to the register' */
    *memPtr = *inMemPtr ;

    forceLinkDown = snetChtPortMacFieldGet(devObjPtr, port,
        SNET_CHT_PORT_MAC_FIELDS_forceLinkDown_E);
    forceLinkUp = snetChtPortMacFieldGet(devObjPtr, port,
        SNET_CHT_PORT_MAC_FIELDS_forceLinkPass_E);
    PortMACReset = snetChtPortMacFieldGet(devObjPtr, port,
        SNET_CHT_PORT_MAC_FIELDS_PortMACReset_E);

    if((old_forceLinkUp   != forceLinkUp  )||
       (old_forceLinkDown != forceLinkDown))
    {
        forceLinkChangeInterrupt = 1;
    }

    /* not GIG  (not tested on HW)
       not CG   (not work on HW)
       only XLG (checked on HW and works)
    */
    if(IS_CHT_HYPER_GIGA_PORT(devObjPtr, port) &&
       !CHT_IS_CG_OR_MTI_MAC_USED_MAC(devObjPtr,port))
    {
        if(old_PortMACReset == 1 && PortMACReset == 0)
        {
            bypassCheckLinkChange = 1;
        }
    }

    devObjPtr->portsArr[port].isForcedLinkUp   = forceLinkUp;
    devObjPtr->portsArr[port].isForcedLinkDown = forceLinkDown;


    oldLinkState = snetChtPortMacFieldGet(devObjPtr, port,
        SNET_CHT_PORT_MAC_FIELDS_LinkState_E);

    if(forceLinkChangeInterrupt)
    {
        newLinkValue = forceLinkUp   ? 1 :
                       forceLinkDown ? 0 :
                       (devObjPtr->portsArr[port].linkStateWhenNoForce ==
                                    SKERNEL_PORT_NATIVE_LINK_UP_E) ? 1 :
                       0;
    }
    else
    if(bypassCheckLinkChange)
    {
        /* bypass the check of 'old state'/'new state' */
        newLinkValue = oldLinkState;
    }
    else
    {
        /* NOTE: force link pass holds priority vs force link down !!!
            this is the HW behavior (tested on XG port)
        */

        if(forceLinkUp)
        {
            /* force link UP */
            /* check if there was a change in the link state */
            if(oldLinkState)
            {
                /* port already "up" */
                return;
            }
        }
        else
        if(forceLinkDown)
        {
            /* check if there was a change in the link state */
            if (!oldLinkState)
            {
                /* port already "down" */
                return;
            }
        }
        else
        {
            /* no forcing of link UP , or down */
            linkStateWhenNoForce = (devObjPtr->portsArr[port].linkStateWhenNoForce ==
                                    SKERNEL_PORT_NATIVE_LINK_UP_E) ? 1 : 0;

            if(linkStateWhenNoForce == oldLinkState)
            {
                /* port already in the needed state */
                return;
            }
        }

        /* the link status must change */
        newLinkValue = !oldLinkState;
    }

    /* Get buffer      */
    bufferId = sbufAlloc(devObjPtr->bufPool, LINK_FORCE_MSG_SIZE);
    if (bufferId == NULL)
    {
        simWarningPrintf(" checkAndSendLinkChange : no buffers for link change \n");
        return;
    }

    /* Get actual data pointer */
    sbufDataGet(bufferId, (GT_U8 **)&dataPtr, &dataSize);
    /* copy port number and link status to buffer  */
    memcpy(dataPtr, &port , sizeof(GT_U32) );
    dataPtr += sizeof(GT_U32);
    memcpy(dataPtr, &newLinkValue , sizeof(GT_U32) );

    /* set source type of buffer                    */
    bufferId->srcType = SMAIN_SRC_TYPE_CPU_E;

    /* set message type of buffer                   */
    bufferId->dataType = SMAIN_LINK_CHG_MSG_E;

    /* put buffer to queue                          */
    squeBufPut(devObjPtr->queueId, SIM_CAST_BUFF(bufferId));

}

/**
* @internal smemChtActiveWriteForceLinkDown function
* @endinternal
*
* @brief   Write Message to the main task - Link change on port.
*
* @param[in] devObjPtr                - device object PTR.
* @param[in] address                  - Address for ASIC memory.
* @param[in] memSize                  - size of the requested memory
* @param[in] memPtr                   - Pointer to the register's memory in the simulation.
* @param[in] param                    - Registers' specific parameter.
* @param[in] inMemPtr                 - Pointer to the memory to get register's content.
*
* @note The function invoked when the linkdown bit (0) on
*       port<n> Auto-Negotiation has been changed.
*
*/
void smemChtActiveWriteForceLinkDown (
    IN         SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN         GT_U32   address,
    IN         GT_U32   memSize,
    IN         GT_U32 * memPtr,
    IN         GT_UINTPTR   param,
    IN         GT_U32 * inMemPtr
)
{
    GT_U32 port_no;             /* Port number */
    GT_U32 autoNeg, newSpeed;

    port_no = CHT_FROM_ADDRESS_GET_PORT_CNS(devObjPtr, address);

    if(IS_CHT_HYPER_GIGA_PORT(devObjPtr, port_no))
    {
        *memPtr = *inMemPtr ;

        /* this function was bind in xcat also for memory space of the XG ports
            so if the port activate as XG port , we not need to do 'Active memory'
            on this register

            unless the port is 'gig' or 'Stack gig' port
        */

        return;
    }

    /* update Gig mac speed status if needed */
    autoNeg = SMEM_U32_GET_FIELD(inMemPtr[0], 7, 1);
    if(autoNeg == 0)
    {
        /* status register bits are swapped  bit 1 is GMII and bit 2 is MII,
         * while in auto-neg. config register bit 5 is MII and bit 6 is GMII */
        newSpeed = (SMEM_U32_GET_FIELD(inMemPtr[0], 5, 1) << 1) /* miiSpeed */
                        | SMEM_U32_GET_FIELD(inMemPtr[0], 6, 1) /* gmiiSpeed */;
        /* Update speed in status register */
        smemRegFldSet(devObjPtr, SMEM_CHT_PORT_STATUS0_REG(devObjPtr, port_no), 1, 2, newSpeed);
    }

    /*    *memPtr = *inMemPtr ; is done inside : smemChtCheckAndSendLinkChange */
    smemChtCheckAndSendLinkChange(devObjPtr,port_no,memPtr,inMemPtr);
}


/**
* @internal smemChtActiveWriteMacGigControl2 function
* @endinternal
*
* @brief   Write Message to the main task - check for mac reset change
*
* @param[in] devObjPtr                - device object PTR.
* @param[in] address                  - Address for ASIC memory.
* @param[in] memSize                  - size of the requested memory
* @param[in] memPtr                   - Pointer to the register's memory in the simulation.
* @param[in] param                    - Registers' specific parameter.
* @param[in] inMemPtr                 - Pointer to the memory to get register's content.
*
* @note refer to <GIGE_MAC_IP> Gige MAC IP %a/Tri-Speed Port MAC Configuration/Port MAC Control Register2
*       the function check change in bit 6 <PortMACReset> (in Gig mode)
*
*/
void smemChtActiveWriteMacGigControl2 (
    IN         SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN         GT_U32   address,
    IN         GT_U32   memSize,
    IN         GT_U32 * memPtr,
    IN         GT_UINTPTR   param,
    IN         GT_U32 * inMemPtr
)
{
    GT_U32 port_no;             /* Port number */

    port_no = CHT_FROM_ADDRESS_GET_PORT_CNS(devObjPtr, address);

    if(IS_CHT_HYPER_GIGA_PORT(devObjPtr, port_no))
    {
        *memPtr = *inMemPtr ;

        /* this function was bind in xcat also for memory space of the XG ports
            so if the port activate as XG port , we not need to do 'Active memory'
            on this register

            unless the port is 'gig' or 'Stack gig' port
        */

        return;
    }
    /*    *memPtr = *inMemPtr ; is done inside : smemChtCheckAndSendLinkChange */
    smemChtCheckAndSendLinkChange(devObjPtr,port_no,memPtr,inMemPtr);
}

/**
* @internal smemChtActiveWriteForceLinkDownXg function
* @endinternal
*
* @brief   Write Message to the main task - Link change on port.
*
* @param[in] devObjPtr                - device object PTR.
* @param[in] address                  - Address for ASIC memory.
* @param[in] memSize                  - size of the requested memory
* @param[in] memPtr                   - Pointer to the register's memory in the simulation.
* @param[in] param                    - Registers' specific parameter.
* @param[in] inMemPtr                 - Pointer to the memory to get register's content.
*
* @note The function invoked when the linkdown bit (0) on
*       port<n> Auto-Negotiation has been changed.
*
*/
void smemChtActiveWriteForceLinkDownXg (
    IN         SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN         GT_U32   address,
    IN         GT_U32   memSize,
    IN         GT_U32 * memPtr,
    IN         GT_UINTPTR   param,
    IN         GT_U32 * inMemPtr
)
{
    GT_U32 port_no;             /* Port number */

    port_no = CHT_FROM_ADDRESS_GET_PORT_CNS(devObjPtr, address);

    if(!IS_CHT_HYPER_GIGA_PORT(devObjPtr, port_no))
    {
        *memPtr = *inMemPtr ;
        /* this function was bind in xcat also for memory space of the 'Stack gig' ports
            so if the port activate as 'Stack Gig' port , we not need to do 'active memory'
            on this register

            unless the port is 'XG' port
        */

        return;
    }
    else
    if(CHT_IS_CG_OR_MTI_MAC_USED_MAC(devObjPtr,port_no))
    {
        *memPtr = *inMemPtr ;

        /* the CG MAC need to be triggered */
        return;
    }

    /*    *memPtr = *inMemPtr ; is done inside : smemChtCheckAndSendLinkChange */
    smemChtCheckAndSendLinkChange(devObjPtr,port_no,memPtr,inMemPtr);
}

/**
* @internal smemChtActiveReadCntrs function
* @endinternal
*
* @brief   Read counters.
*
* @param[in] devObjPtr                - device object PTR.
* @param[in] address                  - Address for ASIC memory.
* @param[in] memPtr                   - Pointer to the register's memory in the simulation.
* @param[in] param                    - Registers' specific parameter -
*                                      global interrupt bit number.
*
* @param[out] outMemPtr                - Pointer to the memory to copy register's content.
*
* @note Output counter value and reset it.
*
*/
 void smemChtActiveReadCntrs (
    IN         SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN         GT_U32   address,
    IN         GT_U32   memSize,
    IN         GT_U32 * memPtr,
    IN         GT_UINTPTR   param,
    OUT        GT_U32 * outMemPtr
)
{
    GT_U32      regAddr;                    /* register address */
    GT_U32      * regDataPtr;               /* pointer to register's data */
    GT_U32      fldValue;                   /* register's field value */
    GT_U32      bitIndex;                   /* bit index */
    GT_U32      resetCounter = 1;           /* do we reset the counter */
    GT_U32      port;                       /* port number */
    GT_U32      isLmsUnit = 0;/* is address part of LMS units */
    GT_U32      isGopUnit = 0;/* is address part of GOP units */
    GT_U32      unitMask =  0xFF800000;/*9 MSbits*/
    GT_U32      lmsBaseAddr;
    GT_U32      lmsSteps;

    /* output actual value of counter */
    * outMemPtr = * memPtr;

    lmsSteps = SMEM_CHT_UNIT_BASE_ADDR_ALIGN_FROM_NUM_OF_BITS(devObjPtr);

    if(SMEM_CHT_IS_SIP5_GET(devObjPtr))
    {
        if(SMAIN_NOT_VALID_CNS != UNIT_BASE_ADDR_GET_ALLOW_NON_EXIST_UNIT_MAC(devObjPtr,UNIT_GOP))
        {
            isGopUnit = ((address & unitMask) == UNIT_BASE_ADDR_GET_MAC(devObjPtr,UNIT_GOP));
        }
        lmsBaseAddr = UNIT_BASE_ADDR_GET_ALLOW_NON_EXIST_UNIT_MAC(devObjPtr,UNIT_LMS);
        if(lmsBaseAddr != SMAIN_NOT_VALID_CNS)
        {
            isLmsUnit = ((address & unitMask) == lmsBaseAddr) ||
                        ((address & unitMask) == (lmsBaseAddr + (lmsSteps*1))) ||
                        ((address & unitMask) == (lmsBaseAddr + (lmsSteps*2))) ||
                        ((address & unitMask) == (lmsBaseAddr + (lmsSteps*3))) ;
        }

    }
    else
    {
        isGopUnit = ((address & unitMask) == devObjPtr->tablesInfo.xgPortMibCounters.commonInfo.baseAddress);
        if( devObjPtr->memUnitBaseAddrInfo.lms[0] )
        {
            lmsBaseAddr = devObjPtr->memUnitBaseAddrInfo.lms[0];
        }
        else
        {
            lmsBaseAddr = 0x04000000;
        }
        isLmsUnit = ((address & unitMask) == lmsBaseAddr) ||
                    ((address & unitMask) == (lmsBaseAddr + (lmsSteps*1))) ||
                    ((address & unitMask) == (lmsBaseAddr + (lmsSteps*2))) ||
                    ((address & unitMask) == (lmsBaseAddr + (lmsSteps*3))) ;
    }


    /* The MAC MIB Counters for Ch/Ch2/Ch3/XCat */
    if(isLmsUnit &&((address & 0x00010000) == 0x00010000))
    {
        /*
            support for MAC MIB Counters (Ports 023)
            0x04010000
            0x04810000
            0x05010000
            0x05810000
        */

        /* MIB Counters Control Register */
        regAddr = 0x04004020 + (0x00800000 * param);
        regDataPtr = smemMemGet(devObjPtr,regAddr);
        /* <DontClearOnRead> defines if counters are cleared when read */
        fldValue = SMEM_U32_GET_FIELD(regDataPtr[0], 4, 1);
        if(fldValue == 1)
        {
            /* don't Reset counter */
            resetCounter = 0;
        }
    }
    else if((0 == SKERNEL_IS_CHEETAH3_DEV(devObjPtr)) &&
            ((address & 0x01E0F000) == 0x01C00000))
    {
        /* ch1,2 ports 24..26
            0x01C00000
            0x01C40000
            0x01C80000
           ch2 port 27
            0x01DC0000
        */

        /* HyperG.Stack Ports MIB Counters and XSMII Configuration Register, Cht2/Cht  */
        regAddr = SMEM_CHT_HYPG_MIB_COUNT_XSMII_CONF_REG(devObjPtr);
        regDataPtr = smemMemGet(devObjPtr,regAddr);
        bitIndex = (param < 27) ? (param - 24) : 19;
        /* <DontClearOnRead> defines if counters are cleared when read */
        fldValue = SMEM_U32_GET_FIELD(regDataPtr[0], bitIndex, 1);
        if(fldValue == 1)
        {
            /* don't Reset counter */
            resetCounter = 0;
        }
    }
    else if(SKERNEL_IS_CHEETAH3_DEV(devObjPtr) &&
            isGopUnit)
    {
        SKERNEL_MIB_COUNTER_INFO_STC mibInfo;

        smemMibCounterInfoByAddrGet(devObjPtr,address,&mibInfo);

        /* XG ports in ch3 and above  */
        /* get port using formula of: 'start address' = (0x09000000 + (port * 0x20000)) */
        port = mibInfo.portNum;

        resetCounter = snetChtPortMacFieldGet(devObjPtr,port,SNET_CHT_PORT_MAC_FIELDS_Port_Clear_After_Read_E);
    }

    if(resetCounter)
    {
        * memPtr = 0;
    }
}

/**
* @internal smemChtActiveReadCntrs64Bit function
* @endinternal
*
* @brief   Read 64 bits counter - the active 'ROC' happens on both registers (high,low)
*         only when the read the 'low' register.
*         NOTE: function assume that the 64 bits counter is one 2 consecutive addresses.
* @param[in] devObjPtr                - device object PTR.
* @param[in] address                  - Address for ASIC memory.
* @param[in] memPtr                   - Pointer to the register's memory in the simulation.
* @param[in] param                    - Registers' specific parameter -
*                                      value 0 - means that the 'low register' ended with (Address % 8) == 0
*
* @param[out] outMemPtr                - Pointer to the memory to copy register's content.
*/
void smemChtActiveReadCntrs64Bit (
    IN         SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN         GT_U32   address,
    IN         GT_U32   memSize,
    IN         GT_U32 * memPtr,
    IN         GT_UINTPTR   param,
    OUT        GT_U32 * outMemPtr
)
{
    GT_BIT isLowCounter;

    /* output actual value of counter */
    * outMemPtr = * memPtr;

    if(param == 0)
    {
        isLowCounter = ((address & 0x4) == 0) ? 1 : 0;
    }
    else
    {
        isLowCounter = ((address & 0x4) != 0) ? 1 : 0;
    }

    if(isLowCounter)
    {
        /* reset the 'high' and 'low' registers */
        memPtr[0] = memPtr[1] = 0;
    }
    else
    {
        /* do nothing */
    }
}

/**
* @internal smemChtActiveReadConst function
* @endinternal
*
* @brief   Read const value.
*
* @param[in] devObjPtr                - device object PTR.
* @param[in] address                  - Address for ASIC memory.
* @param[in] memPtr                   - Pointer to the register's memory in the simulation.
* @param[in] param                    - Registers' specific parameter -
*                                      global interrupt bit number.
*
* @param[out] outMemPtr                - Pointer to the memory to copy register's content.
*
* @note Output counter value and reset it.
*
*/
 void smemChtActiveReadConst (
    IN         SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN         GT_U32   address,
    IN         GT_U32   memSize,
    IN         GT_U32 * memPtr,
    IN         GT_UINTPTR   param,
    OUT        GT_U32 * outMemPtr
)
{
    /* output const value */
    * outMemPtr = (GT_U32)param;
}


/**
* @internal smemChtActiveReadRedirect function
* @endinternal
*
* @brief   redirect the read of register to do the read of another register
*
* @param[in] devObjPtr                - device object PTR.
* @param[in] address                  - Address for ASIC memory.
* @param[in] memPtr                   - Pointer to the register's memory in the simulation.
* @param[in] param                    - Registers' specific parameter
*
* @param[out] outMemPtr                - Pointer to the memory to copy register's content.
*/
 void smemChtActiveReadRedirect (
    IN         SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN         GT_U32   address,
    IN         GT_U32   memSize,
    IN         GT_U32 * memPtr,
    IN         GT_UINTPTR   param,
    OUT        GT_U32 * outMemPtr
)
{
    /* NOTE: memSize may hold MSBits value of 'hiden' value at this point.
        so need to remove it because we access 'PP' address */
    /*redirect the read from address to param */
    smemReadWriteMem(SKERNEL_MEMORY_READ_E,devObjPtr,param,memSize & 0xFFFF,outMemPtr);
}

/**
* @internal smemChtActiveWriteRedirect function
* @endinternal
*
* @brief   redirect the write of register to do the read of another register
*
* @param[in] devObjPtr                - device object PTR.
* @param[in] address                  - Address for ASIC memory.
* @param[in] memSize                  - size of the requested memory
* @param[in] memPtr                   - Pointer to the register's memory in the simulation.
* @param[in] param                    - Registers' specific parameter.
* @param[in] inMemPtr                 - Pointer to the memory to get register's content.
*/
void smemChtActiveWriteRedirect (
    IN         SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN         GT_U32   address,
    IN         GT_U32   memSize,
    IN         GT_U32 * memPtr,
    IN         GT_UINTPTR   param,
    IN         GT_U32 * inMemPtr
)
{
    /* NOTE: memSize may hold MSBits value of 'hiden' value at this point.
        so need to remove it because we access 'PP' address */
    /*redirect the read from address to param */
    smemReadWriteMem(SKERNEL_MEMORY_WRITE_E,devObjPtr,param,memSize & 0xFFFF,inMemPtr);
}
/*******************************************************************************
*   smemChtPciReg
*
* DESCRIPTION:
*       PCI memory access
*
* INPUTS:
*       devObjPtr   - pointer to device object.
*       address     - address of memory(register or table).
*       memSize     - size of the requested memory
*       param       - extra parameter might be used in
*
* OUTPUTS:
*       None.
*
* RETURNS:
*        pointer to the memory location
*        NULL - if memory not exist
*
* COMMENTS:
*
*
*******************************************************************************/
static GT_U32 *  smemChtPciReg(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SCIB_MEMORY_ACCESS_TYPE accessType,
    IN GT_U32 address,
    IN GT_U32 memSize,
    IN GT_UINTPTR param
)
{
    SMEM_CHT_DEV_MEM_INFO  * devMemInfoPtr;
    GT_U32              * regValPtr;
    GT_U32              index;

    regValPtr = 0;
    devMemInfoPtr = (SMEM_CHT_DEV_MEM_INFO *)devObjPtr->deviceMemory;

    if ((address & 0xFFFFFFFF) == 0x00000114)
    {
        /* Reference to global interrupt cause address */
        regValPtr = smemChtGlobalReg(devObjPtr, accessType,
                                     SMEM_CHT_GLB_INT_CAUSE_REG(devObjPtr), memSize, param);
    }
    else
    {
        index = (address & 0xFFF) / 0x4;
        CHECK_MEM_BOUNDS(devMemInfoPtr->pciMem.pciReg,
                         devMemInfoPtr->pciMem.pciRegNum,
                         index, memSize);
        regValPtr = &devMemInfoPtr->pciMem.pciReg[index];
    }

    return regValPtr;
}

/*******************************************************************************
*   smemChtPclReg
*
* DESCRIPTION:
*       PCL registers and TCAM memory space access
*
* INPUTS:
*       devObjPtr   - pointer to device object.
*       address     - address of memory(register or table).
*       memSize     - size of the requested memory
*       param       - extra parameter might be used in
*
* OUTPUTS:
*       None.
*
* RETURNS:
*        pointer to the memory location
*        NULL - if memory not exist
*
* COMMENTS:
*
*
*******************************************************************************/
static GT_U32 *  smemChtPclReg(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SCIB_MEMORY_ACCESS_TYPE accessType,
    IN GT_U32 address,
    IN GT_U32 memSize,
    IN GT_UINTPTR param
)
{
    SMEM_CHT_DEV_MEM_INFO  * devMemInfoPtr;
    GT_U32                * regValPtr;
    GT_U32                  index;

    regValPtr = 0;
    devMemInfoPtr = (SMEM_CHT_DEV_MEM_INFO  *)devObjPtr->deviceMemory;

    /* PCL Registers */
    if ((address & 0xFFFF0000) == 0x0B800000)
    {
        index = (address & 0xFFFF) / 0x4;
        CHECK_MEM_BOUNDS(devMemInfoPtr->pclMem.pclConfReg,
                         devMemInfoPtr->pclMem.pclConfRegNum,
                         index, memSize);
        regValPtr = &devMemInfoPtr->pclMem.pclConfReg[index];
    }
    else
    /* TCAM Registers */
    if ((address & 0xFFFF0000) == 0x0B810000)
    {
        index = (address & 0xFFFF) / 0x4;
        CHECK_MEM_BOUNDS(devMemInfoPtr->pclMem.tcamReg,
                         devMemInfoPtr->pclMem.tcamRegNum,
                         index, memSize);
        regValPtr = &devMemInfoPtr->pclMem.tcamReg[index];
    }
    else
    /* PCL internal registers */
    if ((address & 0xFFFFFF00) == 0x0B820000)
    {
        index = (address & 0xFF) / 0x4;
        CHECK_MEM_BOUNDS(devMemInfoPtr->pclMem.pclIntReg,
                         devMemInfoPtr->pclMem.pclIntRegNum,
                         index, memSize);
        regValPtr = &devMemInfoPtr->pclMem.pclIntReg[index];
    }

    return regValPtr;
}

/*******************************************************************************
*   smemChtPolicerReg
*
* DESCRIPTION:
*       Policer registers and meters memory space access
*
* INPUTS:
*       devObjPtr   - pointer to device object.
*       address     - address of memory(register or table).
*       memSize     - size of the requested memory
*       param       - extra parameter might be used in
*
* OUTPUTS:
*       None.
*
* RETURNS:
*        pointer to the memory location
*        NULL - if memory not exist
*
* COMMENTS:
*
*
*******************************************************************************/
static GT_U32 *  smemChtPolicerReg(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SCIB_MEMORY_ACCESS_TYPE accessType,
    IN GT_U32 address,
    IN GT_U32 memSize,
    IN GT_UINTPTR param
)
{
    SMEM_CHT_DEV_MEM_INFO  * devMemInfoPtr;
    GT_U32              * regValPtr;
    GT_32               index;

    regValPtr = 0;
    devMemInfoPtr = (SMEM_CHT_DEV_MEM_INFO  *)devObjPtr->deviceMemory;

    /* Policer Registers */
    if ((address & 0xFFFFFF00) == 0x0C000000)
    {
        index = (address & 0xFF) / 0x4;
        CHECK_MEM_BOUNDS(devMemInfoPtr->policerMem.policerReg,
                         devMemInfoPtr->policerMem.policerRegNum,
                         index, memSize);
        regValPtr = &devMemInfoPtr->policerMem.policerReg[index];
    }

    return regValPtr;
}

/*******************************************************************************
*   smemChtPhyReg
*
* DESCRIPTION:
*       PHY related Registers access
*
* INPUTS:
*       devObjPtr   - pointer to device object.
*       address     - address of memory(register or table).
*       memSize     - size of the requested memory
*       param       - extra parameter might be used in
*
* OUTPUTS:
*       None.
*
* RETURNS:
*        pointer to the memory location
*        NULL - if memory not exist
*
* COMMENTS:
*
*
*******************************************************************************/
GT_U32 *  smemChtPhyReg(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32 address,
    IN GT_U32 memSize
)
{
    SMEM_CHT_DEV_MEM_INFO  * devMemInfoPtr;
    SMEM_PHY_REGISTER     * regValPtr;
    GT_32                   index;
    GT_32                   device;
    GT_32                   phyid;

    regValPtr = 0;
    devMemInfoPtr = (SMEM_CHT_DEV_MEM_INFO  *)devObjPtr->deviceMemory;

    phyid = ((address >> SMEM_CHT_GROUP_INDEX_FIRST_BIT_CNS) & 0x1F) % 3;
    device = ((address >> SMEM_CHT_GROUP_INDEX_SECOND_BIT_CNS) & 0x1F) % 3;
    index = (address & 0xFF);

    if (phyid == 0)
    {
      if ((address & 0xF000) == 0x8000)
      {
          CHECK_MEM_BOUNDS(devMemInfoPtr->common.phyMem.PhyId0extXauiReg[device],
                           devMemInfoPtr->common.phyMem.PhyIdextXauiRegNum,
                           index, memSize);
          regValPtr = &devMemInfoPtr->common.phyMem.PhyId0extXauiReg[device][index];
      }
      else
      {
          CHECK_MEM_BOUNDS(devMemInfoPtr->common.phyMem.PhyId0ieeeXauiReg[device],
                           devMemInfoPtr->common.phyMem.PhyIdieeeXauiRegNum,
                           index, memSize);
          regValPtr = &devMemInfoPtr->common.phyMem.PhyId0ieeeXauiReg[device][index];
      }
    }
    else
    if (phyid == 1)
    {
      if ((address & 0xF000) == 0x8000)
      {
          CHECK_MEM_BOUNDS(devMemInfoPtr->common.phyMem.PhyId1extXauiReg[device],
                           devMemInfoPtr->common.phyMem.PhyIdextXauiRegNum,
                           index, memSize);
          regValPtr = &devMemInfoPtr->common.phyMem.PhyId1extXauiReg[device][index];
      }
      else
      {
          CHECK_MEM_BOUNDS(devMemInfoPtr->common.phyMem.PhyId1ieeeXauiReg[device],
                           devMemInfoPtr->common.phyMem.PhyIdieeeXauiRegNum,
                           index, memSize);
          regValPtr = &devMemInfoPtr->common.phyMem.PhyId1ieeeXauiReg[device][index];
      }
    }
    else
    {
      if ((address & 0xF000) == 0x8000)
      {
          CHECK_MEM_BOUNDS(devMemInfoPtr->common.phyMem.PhyId2extXauiReg[device],
                           devMemInfoPtr->common.phyMem.PhyIdextXauiRegNum,
                           index, memSize);
          regValPtr = &devMemInfoPtr->common.phyMem.PhyId2extXauiReg[device][index];
      }
      else
      {
          CHECK_MEM_BOUNDS(devMemInfoPtr->common.phyMem.PhyId2ieeeXauiReg[device],
                           devMemInfoPtr->common.phyMem.PhyIdieeeXauiRegNum,
                           index, memSize);
          regValPtr = &devMemInfoPtr->common.phyMem.PhyId2ieeeXauiReg[device][index];
      }
    }

    return (GT_U32 *)regValPtr;
}

/*******************************************************************************
*   smemChtTriSpeedReg
*
* DESCRIPTION:
*      Describe a Tri-Speed Ports MAC, CPU Port MAC, and 1.25 Gbps SERDES
*      Configuration Registers Map Table,  Hyper.GStack Ports MAC and
*      XAUI PHYs Configuration Registers Map Table memory access
*
* INPUTS:
*       devObjPtr   - pointer to device object.
*       address     - address of memory(register or table).
*       memSize     - size of the requested memory
*       param       - extra parameter might be used in
*
* OUTPUTS:
*       None.
*
* RETURNS:
*        pointer to the memory location
*        NULL - if memory not exist
*
* COMMENTS:
*
*
*******************************************************************************/
static GT_U32 *  smemChtTriSpeedReg(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SCIB_MEMORY_ACCESS_TYPE accessType,
    IN GT_U32 address,
    IN GT_U32 memSize,
    IN GT_UINTPTR param
)
{
    SMEM_CHT_DEV_MEM_INFO  * devMemInfoPtr;
    GT_U32              * regValPtr;
    GT_32               index;

    regValPtr = 0;
    devMemInfoPtr = (SMEM_CHT_DEV_MEM_INFO  *)devObjPtr->deviceMemory;

    /* Tri-Speed Ports */
    if ((address & 0xFFFF0000) == 0x0A800000)
    {
        index = (address & 0x3FFF) / 0x4;
        CHECK_MEM_BOUNDS(devMemInfoPtr->triSpeedPortsMem.triSpeedPortsReg,
                         devMemInfoPtr->triSpeedPortsMem.triSpeedPortsRegNum,
                         index, memSize);
        regValPtr = &devMemInfoPtr->triSpeedPortsMem.triSpeedPortsReg[index];
    }

    return regValPtr;
}

/*******************************************************************************
*   smemChtPreEgressReg
*
* DESCRIPTION:
*      Pre-Egress Engine Registers Map Table, Trunk Table,
*      QoSProfile to QoS Table, CPU Code Table,
*      Data Access Statistical Rate Limits Table Data Access,
*      Ingress STC Table Registers memory access
*
* INPUTS:
*       devObjPtr   - pointer to device object.
*       address     - address of memory(register or table).
*       memSize     - size of the requested memory
*       param       - extra parameter might be used in
*
* OUTPUTS:
*       None.
*
* RETURNS:
*        pointer to the memory location
*        NULL - if memory not exist
*
* COMMENTS:
*
*
*******************************************************************************/
static GT_U32 *  smemChtPreEgressReg(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SCIB_MEMORY_ACCESS_TYPE accessType,
    IN GT_U32 address,
    IN GT_U32 memSize,
    IN GT_UINTPTR param
)
{
    SMEM_CHT_DEV_MEM_INFO  * devMemInfoPtr;
    GT_U32                * regValPtr;
    GT_U32                  index;

    regValPtr = 0;
    devMemInfoPtr = (SMEM_CHT_DEV_MEM_INFO  *)devObjPtr->deviceMemory;

    /* Pre-Egress Engine Registers */
    if ((address & 0xFFFFFF00) == 0x0B000000)
    {
        index = (address & 0xFF) / 0x4;
        CHECK_MEM_BOUNDS(devMemInfoPtr->preegressMem.portsReg,
                         devMemInfoPtr->preegressMem.portsRegNum,
                         index, memSize);
        regValPtr = &devMemInfoPtr->preegressMem.portsReg[index];
    }
    else
    /* Ports registers */
    if ((address & 0xFFFFF000) >= 0x0B001000)
    {
        index = (address & 0xFFFF) / 0x1000;
        index += (address & 0xF) / 0x4;
        CHECK_MEM_BOUNDS(devMemInfoPtr->preegressMem.genReg,
                         devMemInfoPtr->preegressMem.genRegNum,
                         index, memSize);
        regValPtr = &devMemInfoPtr->preegressMem.genReg[index];
    }


    return regValPtr;
}

/**
* @internal smemChtActiveWriteXSmii function
* @endinternal
*
* @brief   Handler for write to the 10G SMII control register - phy configuration.
*
* @param[in] devObjPtr                - device object PTR.
* @param[in] address                  - Address for ASIC memory.
* @param[in] memPtr                   - Pointer to the register's memory in the simulation.
* @param[in] param                    - Registers' specific parameter.
* @param[in] inMemPtr                 - Pointer to the memory to get register's content.
*/
void smemChtActiveWriteXSmii (
    IN         SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN         GT_U32   address,
    IN         GT_U32   memSize,
    IN         GT_U32 * memPtr,
    IN         GT_UINTPTR   param,
    IN         GT_U32 * inMemPtr
)
{
    GT_U32                regAddr;   /* address of the phy register */
    GT_U32                fieldVal;  /* register's field value */
    GT_U32                phyDevice; /* device id of phy */
    GT_U32                phyId;     /* id of phy */
    GT_U32                operCode;  /* SMII Operation code */
    GT_U16                regData;   /* Register data */
    GT_U32              * regPtr;    /* register's entry pointer */

    /* Update the register value */
    *memPtr = *inMemPtr;

    phyDevice = ((*inMemPtr) >> 21) & 0x1f;     /* find device id */
    phyId     = ((*inMemPtr) >> 16) & 0x1f;     /* find phy id    */
    /* read address to be read or write (0x01CC0008)   */
    smemRegFldGet(devObjPtr, SMEM_CHT_XSMI_ADDR_REG(devObjPtr), 0, 15, &fieldVal);
    /* Create 32-bit format address: device + 16 bits of the register */
    regAddr = 0xC0000000 | (phyDevice << 24) | (phyId << 16) | fieldVal;

    /* find the data to be written */
    regData = (GT_U16)((*inMemPtr) & 0xff);
    operCode = ((*inMemPtr) >> 26) & 0x7;

    /* clear ReadValid bit */
    SMEM_U32_SET_FIELD((*memPtr),29,1,0);
    /* set busy bit */
    SMEM_U32_SET_FIELD((*memPtr),30,1,1);

    switch (operCode)
    {
        case 0x5 :  /* Address then write     */
        case 0x1 :  /* writing a phy register */
            /* Write only */
            regPtr = (SMEM_REGISTER *)smemChtPhyReg(devObjPtr, regAddr, 1);
            *regPtr = regData;
            break;

        case 0x2: /* A Post-Read-Inc-Addr transaction */
            fieldVal += 1;
            smemRegFldSet(devObjPtr, SMEM_CHT_XSMI_ADDR_REG(devObjPtr), 0, 15, fieldVal);
            break;

        case 0x3 : /* reading a phy register */
        case 0x7 : /* read only              */
        case 0x6 : /* address then increment address read */
            /* find register in simulation   */
            regPtr = (SMEM_REGISTER *)smemChtPhyReg(devObjPtr, regAddr, 1);
            /* Read only , bits[15:0] are the only relevant */
            *memPtr &= 0xffff0000;
            *memPtr |= *regPtr;

            /* read data ready , read-valid */
            *memPtr |= (1 << 29);
            if (operCode == 0x6)
            {
                fieldVal += 1;
                smemRegFldSet(devObjPtr, SMEM_CHT_XSMI_ADDR_REG(devObjPtr), 0, 15, fieldVal);
            }
            break;

      default: skernelFatalError("smemChtActiveWriteXSmii: wrong opCode %d",
                                    operCode);
    }

    /* clear busy bit */
    SMEM_U32_SET_FIELD((*memPtr),30,1,0);
}

/**
* @internal smemChtActiveWriteVlanQos function
* @endinternal
*
* @brief   Handler for write to Ports VLAN and QoS configuration table and
*         Port Protocol Based VLANs and Protocol Based QoS table .
* @param[in] devObjPtr                - device object PTR.
* @param[in] address                  - Address for ASIC memory.
* @param[in] memPtr                   - Pointer to the register's memory in the simulation.
* @param[in] param                    - Registers' specific parameter.
* @param[in] inMemPtr                 - Pointer to the memory to get register's content.
*/
void smemChtActiveWriteVlanQos (
    IN         SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN         GT_U32   address,
    IN         GT_U32   memSize,
    IN         GT_U32 * memPtr,
    IN         GT_UINTPTR   param,
    IN         GT_U32 * inMemPtr
)
{
    GT_U32                port;      /* port number */
    GT_U32                actionType;/* Action performed on table */
    GT_U32                rdWr;      /* Read or write operation */

    rdWr = (*inMemPtr >> 1) & 0x1;
    actionType = (*inMemPtr >> 10) & 0x3;
    port = (*inMemPtr >> 2) & 0x3F;

    switch(actionType)
    {
    case 0:
        smemChtReadWriteVlanQos(devObjPtr, port, rdWr);
        smemChtReadWriteVidQosProt(devObjPtr, port, rdWr);
    break;
    case 1:
        smemChtReadWriteVlanQos(devObjPtr, port, rdWr);
    break;
    case 2:
        smemChtReadWriteVidQosProt(devObjPtr, port, rdWr);
    break;
    default:
    break;
    }

    /* Cleared by the device when the read or write action is completed. */
    *memPtr = *inMemPtr & (~1);
}

/**
* @internal smemChtReadWriteVlanQos function
* @endinternal
*
* @brief   Handler for write to Ports VLAN and QoS configuration table
*
* @param[in] devObjPtr                - device object PTR.
* @param[in] port                     - device  number whose tables are to be read/write.
* @param[in] rdWr                     - Read/Write flag
*/
static void smemChtReadWriteVlanQos (
    IN         SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN         GT_U32  port,
    IN         GT_U32  rdWr
)
{
    GT_U32                * regPtr;         /* register's entry pointer */
    GT_U32                  regAddr;        /* memory table address */

    /* Make 32 bit Vasic compatible address */
    regAddr = SMEM_CHT_PORT_VLAN_QOS_CONFIG_TBL_MEM(devObjPtr, port);
    if (rdWr)
    {
        regPtr = smemMemGet(devObjPtr, SMEM_CHT_PORTS_VLAN_QOS_WORD0_REG(devObjPtr));
        smemMemSet(devObjPtr, regAddr, regPtr, 2);
    }
    else
    {
        regPtr = smemMemGet(devObjPtr, regAddr);
        smemMemSet(devObjPtr, SMEM_CHT_PORTS_VLAN_QOS_WORD0_REG(devObjPtr), regPtr, 2);
    }
}

/**
* @internal smemChtReadWriteVidQosProt function
* @endinternal
*
* @brief   Handler for write Port Protocol Based VLANs and Protocol Based QoS table
*
* @param[in] devObjPtr                - device object PTR
* @param[in] port                     -  number
* @param[in] rdWr                     - read/write action
*/
static void smemChtReadWriteVidQosProt
(
    IN         SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN         GT_U32  port,
    IN         GT_U32  rdWr
)
{
    GT_U32                * regPtr;         /* register's entry pointer */
    GT_U32                  i;              /* memory access index  */
    GT_U32                  regAddr;        /* register's entry address */

    /* 0 <= i == protocol < 8*/
    for (i = 0; i < 8; i++, regPtr++)
    {
        /* Make 32 bit Vasic compatible address */
        regAddr = SMEM_CHT_PROT_VLAN_QOS_TBL_MEM(devObjPtr, i, port);
        if (rdWr)
        {
            regPtr = smemMemGet(devObjPtr, SMEM_CHT_PORTS_VLAN_QOS_PROT_REG(i));
            smemMemSet(devObjPtr, regAddr, regPtr, 1);
        }
        else
        {
            regPtr = smemMemGet(devObjPtr, regAddr);
            smemMemSet(devObjPtr, SMEM_CHT_PORTS_VLAN_QOS_PROT_REG(i), regPtr, 1);
        }
    }
}
/**
* @internal smemChtActiveWritePclAction function
* @endinternal
*
* @brief   The action table and policy TCAM write access
*
* @param[in] devObjPtr                - device object PTR.
* @param[in] address                  - Address for ASIC memory.
* @param[in] memPtr                   - Pointer to the register's memory in the simulation.
* @param[in] memSize                  - Size of memory to be written
* @param[in] param                    - Registers' specific parameter.
* @param[in] inMemPtr                 - Pointer to the memory to get register's content.
*/
static void smemChtActiveWritePclAction (
    IN         SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN         GT_U32   address,
    IN         GT_U32   memSize,
    IN         GT_U32 * memPtr,
    IN         GT_UINTPTR   param,
    IN         GT_U32 * inMemPtr
)
{
    GT_U32     line;            /* line number in TCAM and policy action table*/
    GT_U32     rdWr;            /* Read or write operation */
    GT_U32     rdWrTarget;      /* Read or Write Target Data Structure */
    GT_U32     mode;            /* Mode for the TCAM key short/long */
    GT_U32     type;            /* TCAM data type */
    GT_U32     valid;           /* The Valid bit of this TCAM Line */

    /* Read Action Table and Policy TCAM Access Control Register */
    mode = (*inMemPtr >> 17) & 0x1;
    rdWrTarget = (*inMemPtr >> SMEM_CHT_GROUP_INDEX_FIRST_BIT_CNS) & 0x1;
    type = (*inMemPtr >> 15) & 0x1; /* 0 = mask , 1 - data*/
    line = (*inMemPtr >> 2) & 0x1FFF;
    rdWr = (*inMemPtr >> 1) & 0x1;
    valid  = (*inMemPtr >> 0x12) & 0x1;

    /* stored as is to be retrieved in called functions */
    *memPtr = *inMemPtr ;

    if (rdWrTarget == 0)
    {
        smemChtReadWritePolicyTcam(devObjPtr, line, mode, type, rdWr , valid);
    }
    else
    {
        smemChtReadWritePclAction(devObjPtr, line, rdWr);
    }

    /* Cleared by the device when the read or write action is completed. */
    *memPtr = *inMemPtr & (~1);
}

/**
* @internal smemChtActiveWritePclId function
* @endinternal
*
* @brief   The action table and policy TCAM write access
*
* @param[in] devObjPtr                - device object PTR.
* @param[in] address                  - Address for ASIC memory.
* @param[in] memPtr                   - Pointer to the register's memory in the simulation.
* @param[in] memSize                  - Size of memory to be written
* @param[in] param                    - Registers' specific parameter.
* @param[in] inMemPtr                 - Pointer to the memory to get register's content.
*/
void smemChtActiveWritePclId (
    IN         SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN         GT_U32   address,
    IN         GT_U32   memSize,
    IN         GT_U32 * memPtr,
    IN         GT_UINTPTR   param,
    IN         GT_U32 * inMemPtr
)
{
    GT_U32                  rdWr;           /* Read or write operation */
    GT_U32                  line;           /* line number */
    GT_U32                * regPtr;         /* register's entry pointer */
    GT_U32                  regAddr;        /* register's address */

    rdWr = (*inMemPtr >> 1) & 0x1;
    line = (*inMemPtr >> 2) & 0x1FFF;

    regAddr = SMEM_CHT_PCL_ID_TBL_MEM(devObjPtr, line);

    if (rdWr)
    {
        /* PCL-ID Table Access Data Register */
        regPtr = smemMemGet(devObjPtr, SMEM_CHT_PCL_ID_TBL_ACC_DATA_REG(devObjPtr));
        smemMemSet(devObjPtr, regAddr, regPtr, 1);
    }
    else
    {
        regPtr = smemMemGet(devObjPtr, regAddr);
        /* PCL-ID Table Access Data Register */
        smemMemSet(devObjPtr, SMEM_CHT_PCL_ID_TBL_ACC_DATA_REG(devObjPtr), regPtr, 1);
    }

    /* Cleared by the device when the read or write action is completed. */
    *memPtr = *inMemPtr & (~1);
}

/**
* @internal smemChtActiveWritePolicerQos function
* @endinternal
*
* @brief   Policers QoS Remarking and Initial DP Table
*
* @param[in] devObjPtr                - device object PTR.
* @param[in] address                  - Address for ASIC memory.
* @param[in] memPtr                   - Pointer to the register's memory in the simulation.
* @param[in] memSize                  - Size of memory to be written
* @param[in] param                    - Registers' specific parameter.
* @param[in] inMemPtr                 - Pointer to the memory to get register's content.
*/
void smemChtActiveWritePolicerQos (
    IN         SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN         GT_U32   address,
    IN         GT_U32   memSize,
    IN         GT_U32 * memPtr,
    IN         GT_UINTPTR   param,
    IN         GT_U32 * inMemPtr
)
{
    GT_U32                  rdWr;               /* Read or write operation */
    GT_U32                  line;               /* line number */
    GT_U32                * regPtr;             /* register's entry pointer */
    GT_U32                  regAddr;            /* register's address */

    rdWr = (*inMemPtr >> 1) & 0x1;
    line = (*inMemPtr >> 2) & 0x7F;

    regAddr = SMEM_CHT_POLICER_QOS_TBL_MEM(devObjPtr, 0, line);

    if (rdWr)
    {
        /* Policers QoS Remarking and Initial DP Table Data Register */
        regPtr = smemMemGet(devObjPtr, SMEM_CHT_POLICER_QOS_REM_DATA_REG(dev));
        smemMemSet(devObjPtr, regAddr, regPtr, 1);
    }
    else
    {
        regPtr = smemMemGet(devObjPtr, regAddr);
        /* Policers QoS Remarking and Initial DP Table Data Register */
        smemMemSet(devObjPtr, SMEM_CHT_POLICER_QOS_REM_DATA_REG(dev), regPtr, 1);
    }

    /* Cleared by the device when the read or write action is completed. */
    *memPtr = *inMemPtr & (~1);
}
/**
* @internal smemChtActiveWritePolicerCnt function
* @endinternal
*
* @brief   Policer Counters Table Access Control Register
*
* @param[in] devObjPtr                - device object PTR.
* @param[in] address                  - Address for ASIC memory.
* @param[in] memPtr                   - Pointer to the register's memory in the simulation.
* @param[in] memSize                  - Size of memory to be written
* @param[in] param                    - Registers' specific parameter.
* @param[in] inMemPtr                 - Pointer to the memory to get register's content.
*/
void smemChtActiveWritePolicerCnt (
    IN         SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN         GT_U32   address,
    IN         GT_U32   memSize,
    IN         GT_U32 * memPtr,
    IN         GT_UINTPTR   param,
    IN         GT_U32 * inMemPtr
)
{
    GT_U32                  rdWr;           /* Read or write operation */
    GT_U32                  line;           /* line number */
    GT_U32                * regPtr;         /* register's entry pointer */
    GT_U32                  regAddr;        /* register's address */

    rdWr = (*inMemPtr >> 1) & 0x1;
    line = (*inMemPtr >> 2) & 0xF;

    regAddr = SMEM_CHT_POLICER_CNT_TBL_MEM(devObjPtr, 0, line);

    if (rdWr)
    {
        /* Policer Counters Table Data0 Access Register */
        regPtr = smemMemGet(devObjPtr, SMEM_CHT_POLICER_CNT_TBL_DATA0_REG(devObjPtr));
        smemMemSet(devObjPtr, regAddr, regPtr, 2);
    }
    else
    {
        regPtr = smemMemGet(devObjPtr, regAddr);
        /* Policer Counters Table Data0 Access Register */
        smemMemSet(devObjPtr, SMEM_CHT_POLICER_CNT_TBL_DATA0_REG(devObjPtr), regPtr, 2);
    }

    /* Cleared by the device when the read or write action is completed. */
    *memPtr = *inMemPtr & (~1);
}

/**
* @internal smemChtActiveWritePolicerTbl function
* @endinternal
*
* @brief   The Policers table write access
*
* @param[in] devObjPtr                - device object PTR.
* @param[in] address                  - Address for ASIC memory.
* @param[in] memPtr                   - Pointer to the register's memory in the simulation.
* @param[in] memSize                  - Size of memory to be written
* @param[in] param                    - Registers' specific parameter.
* @param[in] inMemPtr                 - Pointer to the memory to get register's content.
*/
void smemChtActiveWritePolicerTbl (
    IN         SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN         GT_U32   address,
    IN         GT_U32   memSize,
    IN         GT_U32 * memPtr,
    IN         GT_UINTPTR   param,
    IN         GT_U32 * inMemPtr
)
{
    GT_U32                  rdWr;           /* Read or write operation */
    GT_U32                  line;           /* line number */
    GT_U32                * regPtr;         /* register's entry pointer */
    GT_U32                  regAddr;        /* register's address */

    rdWr = (*inMemPtr >> 1) & 0x1;
    line = (*inMemPtr >> 2) & 0xFFF;

    regAddr = SMEM_CHT_POLICER_TBL_MEM(devObjPtr, 0, line);

    if (rdWr)
    {
        /* Policer Entry Word0 Access Register */
        regPtr = smemMemGet(devObjPtr, SMEM_CHT_POLICER_ENT_WORD_0_REG(dev));
        smemMemSet(devObjPtr, regAddr, regPtr, 2);
    }
    else
    {
        regPtr = smemMemGet(devObjPtr, regAddr);
        /* Policer Entry Word0 Access Register */
        smemMemSet(devObjPtr, SMEM_CHT_POLICER_ENT_WORD_0_REG(dev), regPtr, 2);
    }

    /* Cleared by the device when the read or write action is completed. */
    *memPtr = *inMemPtr & (~1);
}
/**
* @internal smemChtActiveWriteVlanTbl function
* @endinternal
*
* @brief   VLT Tables Access Control Register
*
* @param[in] devObjPtr                - device object PTR.
* @param[in] address                  - Address for ASIC memory.
* @param[in] memPtr                   - Pointer to the register's memory in the simulation.
* @param[in] memSize                  - Size of memory to be written
* @param[in] param                    - Registers' specific parameter.
* @param[in] inMemPtr                 - Pointer to the memory to get register's content.
*/
static void smemChtActiveWriteVlanTbl (
    IN         SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN         GT_U32   address,
    IN         GT_U32   memSize,
    IN         GT_U32 * memPtr,
    IN         GT_UINTPTR   param,
    IN         GT_U32 * inMemPtr
)
{
    GT_U32                  rdWr;           /* Read or write operation */
    GT_U32                  entry;          /* table entry number */
    GT_U32                  trgTbl;         /* target table */
    GT_U32                * regPtr;         /* register's entry pointer */
    GT_U32                  regAddr;        /* register's address */
    GT_U32                  hwWords[3];     /* swapped entry */

    rdWr = (*inMemPtr >> 12) & 0x1;
    entry = *inMemPtr & 0xFFF;
    trgTbl = (*inMemPtr >> 13)& 0x3;

    /* VLAN table */
    if (trgTbl == 0)
    {
        regAddr = SMEM_CHT_VLAN_TBL_MEM(devObjPtr, entry);
        if (rdWr)
        {
            /* VLT Tables Access Data Register2 */
            regPtr = smemMemGet(devObjPtr, SMEM_CHT_VLAN_TBL_WORD2_REG(devObjPtr));
            /* Swap VLAN entry 3 words */
            hwWords[0] = regPtr[2];
            hwWords[1] = regPtr[1];
            hwWords[2] = regPtr[0];
            smemMemSet(devObjPtr, regAddr, hwWords, 3);
        }
        else
        {
            regPtr = smemMemGet(devObjPtr, regAddr);
            /* Swap VLAN entry 3 words */
            hwWords[0] = regPtr[2];
            hwWords[1] = regPtr[1];
            hwWords[2] = regPtr[0];
            /* VLT Tables Access Data Register0 */
            smemMemSet(devObjPtr, SMEM_CHT_VLAN_TBL_WORD2_REG(devObjPtr), hwWords, 3);
        }
    }
    else
    /* Multicast Groups table */
    if (trgTbl == 1)
    {
        regAddr = SMEM_CHT_MCST_TBL_MEM(devObjPtr, entry);
        if (rdWr)
        {
            /* VLT Tables Access Data Register0 */
            regPtr = smemMemGet(devObjPtr, SMEM_CHT_VLAN_TBL_WORD0_REG(devObjPtr));
            smemMemSet(devObjPtr, regAddr, regPtr, 1);
        }
        else
        {
            regPtr = smemMemGet(devObjPtr, regAddr);
            /* VLT Tables Access Data Register0 */
            smemMemSet(devObjPtr, SMEM_CHT_VLAN_TBL_WORD0_REG(devObjPtr), regPtr, 1);
        }
    }
    else
    /* Span Stage Groups table */
    if (trgTbl == 2)
    {
        regAddr = SMEM_CHT_STP_TBL_MEM(devObjPtr, entry);
        if (rdWr)
        {
            /* VLT Tables Access Data Register0 */
            regPtr = smemMemGet(devObjPtr, SMEM_CHT_VLAN_TBL_WORD1_REG(devObjPtr));
            /* Swap STP entry words */
            hwWords[0] = regPtr[1];
            hwWords[1] = regPtr[0];
            smemMemSet(devObjPtr, regAddr, hwWords, 2);
        }
        else
        {
            regPtr = smemMemGet(devObjPtr, regAddr);
            /* Swap STP entry words */
            hwWords[0] = regPtr[1];
            hwWords[1] = regPtr[0];
            /* VLT Tables Access Data Register0 */
            smemMemSet(devObjPtr, SMEM_CHT_VLAN_TBL_WORD1_REG(devObjPtr), hwWords, 2);
        }
    }


    /* Cleared by the device when the read or write action is completed. */
    *memPtr = *inMemPtr & (~(1 << 15));
}

/**
* @internal smemChtActiveWriteIngrStcTbl function
* @endinternal
*
* @brief   Ingress STC Table Access Control Register
*
* @param[in] devObjPtr                - device object PTR.
* @param[in] address                  - Address for ASIC memory.
* @param[in] memPtr                   - Pointer to the register's memory in the simulation.
* @param[in] memSize                  - Size of memory to be written
* @param[in] param                    - Registers' specific parameter.
* @param[in] inMemPtr                 - Pointer to the memory to get register's content.
*
* @note NOTE: not applicable to sip6 devices
*
*/
GT_VOID smemChtActiveWriteIngrStcTbl
(
    IN         SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN         GT_U32   address,
    IN         GT_U32   memSize,
    IN         GT_U32 * memPtr,
    IN         GT_UINTPTR   param,
    IN         GT_U32 * inMemPtr
)
{
    GT_U32                  rdWr;            /* Read or write operation */
    GT_U32                  line;            /* line number */
    GT_U32                * regPtr;          /* register's entry pointer */
    GT_U32                  regAddr;         /* register's address */

    rdWr = SMEM_U32_GET_FIELD((*inMemPtr), 1, 1);
    line = SMEM_U32_GET_FIELD((*inMemPtr), 2, 5);

    regAddr = SMEM_CHT_INGR_STC_TBL_MEM(devObjPtr, line);

    if (rdWr)
    {
        /* Ingress STC Table Word0 Access Register -- Write */
        /* get pointer to the indirect data registers */
        regPtr = smemMemGet(devObjPtr, SMEM_CHT_INGR_STC_TBL_WORD0_REG(devObjPtr));
        /* NOTE : word 1 - is "read only" so CPU can't modify it */
        if(SMEM_U32_GET_FIELD((*inMemPtr), 7, 1))
        {
            smemMemSet(devObjPtr, regAddr+0 , &regPtr[0], 1);/* word 0 */
        }

        if(SMEM_U32_GET_FIELD((*inMemPtr), 8, 1))
        {
            smemMemSet(devObjPtr, regAddr+8 , &regPtr[2], 1);/* word 2 */
        }

        /* NOTE: the 'triggering' of field <IngressSTCNewLimValRdy> is done only
           when traffic pass through the port !!! */
    }
    else
    {
        regPtr = smemMemGet(devObjPtr, regAddr);
        /* Ingress STC Table Word0 Access Register -- Read */
        smemMemSet(devObjPtr, SMEM_CHT_INGR_STC_TBL_WORD0_REG(devObjPtr), regPtr, 3);
    }

    /* Cleared by the device when the read or write action is completed. */
    *memPtr = *inMemPtr & (~1);
}
/**
* @internal smemChtActiveWriteStatRateLimTbl function
* @endinternal
*
* @brief   Statistical Rate Limits Table Access Control Register
*
* @param[in] devObjPtr                - device object PTR.
* @param[in] address                  - Address for ASIC memory.
* @param[in] memPtr                   - Pointer to the register's memory in the simulation.
* @param[in] memSize                  - Size of memory to be written
* @param[in] param                    - Registers' specific parameter.
* @param[in] inMemPtr                 - Pointer to the memory to get register's content.
*/
void smemChtActiveWriteStatRateLimTbl (
    IN         SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN         GT_U32   address,
    IN         GT_U32   memSize,
    IN         GT_U32 * memPtr,
    IN         GT_UINTPTR   param,
    IN         GT_U32 * inMemPtr
)
{
    GT_U32                  rdWr;           /* Read or write operation */
    GT_U32                  line;           /* line number */
    GT_U32                * regPtr;         /* register's entry pointer */
    GT_U32                  regAddr;        /* register's address */

    rdWr = (*inMemPtr >> 1) & 0x1;
    line = (*inMemPtr >> 2) & 0x1F;

    regAddr = SMEM_CHT_STAT_RATE_TBL_MEM(devObjPtr, line);

    if (rdWr)
    {
        /* Statistical Rate Limits Table Data Access Register */
        regPtr = smemMemGet(devObjPtr, SMEM_CHT_STAT_RATE_LIMIT_DATA_REG(devObjPtr));
        smemMemSet(devObjPtr, regAddr, regPtr, 1);
    }
    else
    {
        regPtr = smemMemGet(devObjPtr, regAddr);
        /* Statistical Rate Limits Table Data Access Register */
        smemMemSet(devObjPtr, SMEM_CHT_STAT_RATE_LIMIT_DATA_REG(devObjPtr), regPtr, 1);
    }

    /* Cleared by the device when the read or write action is completed. */
    *memPtr = *inMemPtr & (~1);
}

/**
* @internal smemChtActiveWriteCpuCodeAccess function
* @endinternal
*
* @brief   QoSProfile to QoS Access Control Register
*
* @param[in] devObjPtr                - device object PTR.
* @param[in] address                  - Address for ASIC memory.
* @param[in] memPtr                   - Pointer to the register's memory in the simulation.
* @param[in] memSize                  - Size of memory to be written
* @param[in] param                    - Registers' specific parameter.
* @param[in] inMemPtr                 - Pointer to the memory to get register's content.
*/
void smemChtActiveWriteCpuCodeAccess (
    IN         SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN         GT_U32   address,
    IN         GT_U32   memSize,
    IN         GT_U32 * memPtr,
    IN         GT_UINTPTR   param,
    IN         GT_U32 * inMemPtr
)
{
    GT_U32                  rdWr;           /* Read or write operation */
    GT_U32                  line;           /* line number */
    GT_U32                * regPtr;         /* register's entry pointer */
    GT_U32                  regAddr;        /* register's address */

    rdWr = (*inMemPtr >> 1) & 0x1;
    line = (*inMemPtr >> 2) & 0xFF;

    regAddr = SMEM_CHT_CPU_CODE_TBL_MEM(devObjPtr, line);

    if (rdWr)
    {
        /* CPU Code Table Data Access Register */
        regPtr = smemMemGet(devObjPtr, SMEM_CHT_CPU_CODE_DATA_REG(devObjPtr));
        smemMemSet(devObjPtr, regAddr, regPtr, 1);
    }
    else
    {
        regPtr = smemMemGet(devObjPtr, regAddr);
        /* CPU Code Table Data Access Register */
        smemMemSet(devObjPtr, SMEM_CHT_CPU_CODE_DATA_REG(devObjPtr), regPtr, 1);
    }

    /* Cleared by the device when the read or write action is completed. */
    *memPtr = *inMemPtr & (~1);
}

/**
* @internal smemChtActiveWriteQosAccess function
* @endinternal
*
* @brief   QoSProfile to QoS Access Control Register
*
* @param[in] devObjPtr                - device object PTR.
* @param[in] address                  - Address for ASIC memory.
* @param[in] memPtr                   - Pointer to the register's memory in the simulation.
* @param[in] memSize                  - Size of memory to be written
* @param[in] param                    - Registers' specific parameter.
* @param[in] inMemPtr                 - Pointer to the memory to get register's content.
*/
void smemChtActiveWriteQosAccess (
    IN         SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN         GT_U32   address,
    IN         GT_U32   memSize,
    IN         GT_U32 * memPtr,
    IN         GT_UINTPTR   param,
    IN         GT_U32 * inMemPtr
)
{
    GT_U32                  rdWr;           /* Read or write operation */
    GT_U32                  line;           /* line number */
    GT_U32                * regPtr;         /* register's entry pointer */
    GT_U32                  regAddr;        /* register's address */

    rdWr = (*inMemPtr >> 1) & 0x1;
    line = (*inMemPtr >> 2) & 0x7F;

    regAddr = SMEM_CHT_QOS_TBL_MEM(devObjPtr, line);

    if (rdWr)
    {
        /* QoSProfile to QoS table Data Access Register */
        regPtr = smemMemGet(devObjPtr, SMEM_CHT_QOS_PROFILE_DATA_REG(devObjPtr));
        smemMemSet(devObjPtr, regAddr, regPtr, 1);
    }
    else
    {
        regPtr = smemMemGet(devObjPtr, regAddr);
        /* QoSProfile to QoS table Data Access Register */
        smemMemSet(devObjPtr, SMEM_CHT_QOS_PROFILE_DATA_REG(devObjPtr), regPtr, 1);
    }

    /* Cleared by the device when the read or write action is completed. */
    *memPtr = *inMemPtr & (~1);
}
/**
* @internal smemChtActiveWriteTrunkTbl function
* @endinternal
*
* @brief   Trunk table Access Control Register
*
* @param[in] devObjPtr                - device object PTR.
* @param[in] address                  - Address for ASIC memory.
* @param[in] memPtr                   - Pointer to the register's memory in the simulation.
* @param[in] memSize                  - Size of memory to be written
* @param[in] param                    - Registers' specific parameter.
* @param[in] inMemPtr                 - Pointer to the memory to get register's content.
*/
void smemChtActiveWriteTrunkTbl (
    IN         SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN         GT_U32   address,
    IN         GT_U32   memSize,
    IN         GT_U32 * memPtr,
    IN         GT_UINTPTR   param,
    IN         GT_U32 * inMemPtr
)
{
    GT_U32                  rdWr;           /* Read or write operation */
    GT_U32                  line;           /* line number */
    GT_U32                  member;         /* trunk member */
    GT_U32                * regPtr;         /* register's entry pointer */
    GT_U32                  trunkRegAddr;   /* trunk table register address */
    GT_U32                  fieldVal;
    GT_U32                * trunkTableMemPtr;/* (pointer to) memory in the
                                                trunk table */
    rdWr = (*inMemPtr >> 1) & 0x1;
    fieldVal = (*inMemPtr >> 2) & SMEM_BIT_MASK(3/*member index*/ + devObjPtr->flexFieldNumBitsSupport.trunkId);
    member = (fieldVal & 0x7);/* member index*/
    line = (fieldVal >> 3);   /* trunkId */

    trunkRegAddr = SMEM_CHT_TRUNK_TBL_MEM(devObjPtr, member, line);

    if (rdWr)
    {
        /* write action -- from data register to trunk table */

        /* pointer to the memory of the Trunk table Data Access Register */
        regPtr = smemMemGet(devObjPtr, SMEM_CHT_TRUNK_TBL_DATA_REG(devObjPtr));

        /* copy from Trunk table Data Access Register to trunk table */
        smemMemSet(devObjPtr, trunkRegAddr, regPtr, 1);
    }
    else
    {
        /* read action -- from trunk table to data register */

        /* pointer to the memory of the Trunk table */
        trunkTableMemPtr = smemMemGet(devObjPtr, trunkRegAddr);

        /* copy from trunk table to Trunk table Data Access Register */
        smemMemSet(devObjPtr, SMEM_CHT_TRUNK_TBL_DATA_REG(devObjPtr), trunkTableMemPtr, 1);
    }

    /* Cleared by the device when the read or write action is completed. */
    *memPtr = *inMemPtr & (~1);
}

/**
* @internal smemChtReadWritePolicyTcam function
* @endinternal
*
* @brief   Handler for read/write policy TCAM memory
*
* @param[in] devObjPtr                - device object PTR.
* @param[in] line                     - table  number to be read or write
* @param[in] mode                     - short/long key mode
* @param[in] rdWr                     - read/write action
*/
static void smemChtReadWritePolicyTcam
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32  line,
    IN GT_U32  mode,
    IN GT_U32  type,
    IN GT_U32  rdWr,
    IN GT_U32  valid
)
{
    if (rdWr)
    {
        smemChtWritePolicyTcamData(devObjPtr, line, type, mode);
        smemChtWritePolicyTcamCtrl(devObjPtr, line, type, mode , valid);
    }
    else
    {
        smemChtReadPolicyTcamData(devObjPtr, line, type, mode);
        smemChtReadPolicyTcamCtrl(devObjPtr, line, type, mode);
    }
}

/**
* @internal smemChtWritePolicyTcamData function
* @endinternal
*
* @brief   Handler for read/write policy TCAM memory
*
* @param[in] devObjPtr                - device object PTR.
* @param[in] line                     - table  number to be read or write
* @param[in] type                     - TCAM data or mask
* @param[in] mode                     - short/long key mode
*/
static void smemChtWritePolicyTcamData (
    IN         SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN         GT_U32  line,
    IN         GT_U32  type,
    IN         GT_U32  mode
)
{
    GT_U32                * regPtr;         /* register's entry pointer */
    GT_U32                  regAddr0;       /* register's address group 0-2 */
    GT_U32                  regAddr3;       /* register's address group 3-5 */

    /* TCAM data or mask */
    if (type == 1)
    {
        regAddr0 = SMEM_CHT_PCL_TCAM_ENTRY_WORD0_REG(line);
        regAddr3 = SMEM_CHT_PCL_TCAM_ENTRY_WORD3_REG(line);
    }
    else
    {
        regAddr0 = SMEM_CHT_PCL_TCAM_ENTRY_MASK0_REG(line);
        regAddr3 = SMEM_CHT_PCL_TCAM_ENTRY_MASK3_REG(line);
    }

    /* A long key */
    if (mode)
    {
        /* Read TCAM data register word 0 */
        regPtr = smemMemGet(devObjPtr, SMEM_CHT_PCL_ACTION_TCAM_DATA0_REG(devObjPtr));
    }
    else
    {
        /* Read TCAM data register word 6*/
        regPtr = smemMemGet(devObjPtr, SMEM_CHT_PCL_ACTION_TCAM_DATA6_REG(devObjPtr));
    }

    /* Write words 0-2 */
    smemMemSet(devObjPtr, regAddr0, regPtr, 3);
    regPtr += 3;
    /* Write words 3-5 */
    smemMemSet(devObjPtr, regAddr3, regPtr, 3);

    /* A long key */
    if (mode)
    {
        if (type == 1)
        {
            regAddr0 = SMEM_CHT_PCL_TCAM_ENTRY_WORD0_REG(line + 512);
            regAddr3 = SMEM_CHT_PCL_TCAM_ENTRY_WORD3_REG(line + 512);
        }
        else
        {
            regAddr0 = SMEM_CHT_PCL_TCAM_ENTRY_MASK0_REG(line + 512);
            regAddr3 = SMEM_CHT_PCL_TCAM_ENTRY_MASK3_REG(line + 512);
        }

        regPtr += 3;
        /* Write words 6-8 */
        smemMemSet(devObjPtr, regAddr0, regPtr, 3);
        regPtr += 3;
        /* Write words 9-11 */
        smemMemSet(devObjPtr, regAddr3, regPtr, 3);
    }
}

/**
* @internal smemChtActiveWriteSdmaCommand function
* @endinternal
*
* @brief   Enable/Disable SDMA Rx queue.
*
* @param[in] devObjPtr                - device object PTR.
* @param[in] address                  - Address for ASIC memory.
* @param[in] memPtr                   - Pointer to the register's memory in the simulation.
* @param[in] memSize                  - Size of memory to be written
* @param[in] param                    - Registers' specific parameter.
* @param[in] inMemPtr                 - Pointer to the memory to get register's content.
*/
void smemChtActiveWriteSdmaCommand (
    IN         SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN         GT_U32   address,
    IN         GT_U32   memSize,
    IN         GT_U32 * memPtr,
    IN         GT_UINTPTR   param,
    IN         GT_U32 * inMemPtr
)
{
    GT_U32        ii;
    GT_U32        bitFlag;

    for (ii = 0 ; ii < 7 ; ++ii)
    {
        bitFlag = 1 << ii;
        /* Writing '1' to an ENQ bit resets the matching DISQ bit           *
         Writing '1' to an ENQ of an SDMA that is already in enable state *
         * has no meaning. Writing '0' to an ENQ bit has no effect          */
        if ( (*inMemPtr & bitFlag) && ((*memPtr & bitFlag)==0) )
        {
            *inMemPtr &= ~ (1 << (ii + 8) );
        }
    }

    for (ii = 0 ; ii < 7 ; ++ii)
    {
        bitFlag = 1 << (ii + 8) ;
        /* Writing '1' to a bit disables the corresponding queue
         Writing '0' to a DISQ has no meaning                             */
        if (*inMemPtr & (bitFlag))
        {
            *inMemPtr &= ~ ( 1 << ii );
        }
    }

    *memPtr = *inMemPtr;
}



/**
* @internal smemChtReadPolicyTcamData function
* @endinternal
*
* @brief   Handler for read/write policy TCAM memory
*
* @param[in] devObjPtr                - device object PTR.
* @param[in] line                     - table  number to be read or write
* @param[in] type                     - TCAM data or mask
* @param[in] mode                     - short/long key mode
*/
static void smemChtReadPolicyTcamData (
    IN         SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN         GT_U32  line,
    IN         GT_U32  type,
    IN         GT_U32  mode
)
{
    GT_U32                * regPtr0;        /* register's entry pointer */
    GT_U32                * regPtr3;        /* register's entry pointer */
    GT_U32                  regAddr0;       /* register's address group 0-2 */
    GT_U32                  regAddr3;       /* register's address group 3-5 */

    /* TCAM data or mask */
    if (type == 1)
    {
        regAddr0 = SMEM_CHT_PCL_TCAM_ENTRY_WORD0_REG(line);
        regAddr3 = SMEM_CHT_PCL_TCAM_ENTRY_WORD3_REG(line);
    }
    else
    {
        regAddr0 = SMEM_CHT_PCL_TCAM_ENTRY_MASK0_REG(line);
        regAddr3 = SMEM_CHT_PCL_TCAM_ENTRY_MASK3_REG(line);
    }

    /* Read words 0-2 */
    regPtr0 = smemMemGet(devObjPtr, regAddr0);
    /* Read words 3-5 */
    regPtr3 = smemMemGet(devObjPtr, regAddr3);

    /* A long key */
    if (mode)
    {
        /* Write TCAM data register word 0 */
        smemMemSet(devObjPtr, SMEM_CHT_PCL_ACTION_TCAM_DATA0_REG(devObjPtr), regPtr0, 3);
        /* Write TCAM data register word 3 */
        smemMemSet(devObjPtr, SMEM_CHT_PCL_ACTION_TCAM_DATA3_REG(devObjPtr), regPtr3, 3);
    }
    else
    {
        /* Write TCAM data register word 6*/
        smemMemSet(devObjPtr, SMEM_CHT_PCL_ACTION_TCAM_DATA6_REG(devObjPtr), regPtr0, 3);
        /* Write TCAM data register word 9 */
        smemMemSet(devObjPtr, SMEM_CHT_PCL_ACTION_TCAM_DATA9_REG(devObjPtr), regPtr3, 3);
    }


    /* A long key */
    if (mode)
    {
        if (type == 1)
        {
            regAddr0 = SMEM_CHT_PCL_TCAM_ENTRY_WORD0_REG(line + 512);
            regAddr3 = SMEM_CHT_PCL_TCAM_ENTRY_WORD3_REG(line + 512);
        }
        else
        {
            regAddr0 = SMEM_CHT_PCL_TCAM_ENTRY_MASK0_REG(line + 512);
            regAddr3 = SMEM_CHT_PCL_TCAM_ENTRY_MASK3_REG(line + 512);
        }

        /* Read words 6-8 */
        regPtr0 = smemMemGet(devObjPtr, regAddr0);
        /* Read words 9-11 */
        regPtr3 = smemMemGet(devObjPtr, regAddr3);

        /* Write TCAM data register word 0 */
        smemMemSet(devObjPtr, SMEM_CHT_PCL_ACTION_TCAM_DATA6_REG(devObjPtr), regPtr0, 3);
        /* Write TCAM data register word 3 */
        smemMemSet(devObjPtr, SMEM_CHT_PCL_ACTION_TCAM_DATA9_REG(devObjPtr), regPtr3, 3);

    }
}

/**
* @internal smemChtWritePolicyTcamCtrl function
* @endinternal
*
* @brief   Handler for read/write policy TCAM memory
*
* @param[in] devObjPtr                - device object PTR.
* @param[in] line                     - table  number to be read or write
* @param[in] mode                     - short/long key mode
* @param[in] type                     - TCAM data or mask
*/
static void smemChtWritePolicyTcamCtrl (
    IN         SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN         GT_U32  line,
    IN         GT_U32  type,
    IN         GT_U32  mode ,
    IN         GT_U32  valid
)
{
    GT_U32                * regReadPtr;     /* read register's entry pointer */
    GT_U32                * regWritePtr0;   /* write register's entry pointer */
    GT_U32                * regWritePtr1;   /* write register's entry pointer */
    GT_U32                  regAddr;        /* register's address */
    GT_U32                  validEntry;          /* is pattern of  valid PCE       */
    GT_U32                  expanded;       /* 48 byte 0r 24 byte key         */
    GT_U32                  data;           /* data to write to control regs  */

    /* Read Action Table and Policy TCAM Access Control Register */
    regReadPtr = smemMemGet(devObjPtr, SMEM_CHT_PCL_ACTION_TCAM_CTRL_REG(devObjPtr));

    /* for mask(type =0) valid bit is always "1" */
    validEntry = (type == 0) ? 1 : valid ;

    expanded = (((*regReadPtr) >> 17) & 1);

    /* to expose in all read-only control registers */
    data = (validEntry << 1) | (expanded << 3);

    /* TCAM data type */
    if (type == 0)
    {
        regAddr = SMEM_CHT_PCL_TCAM_MASK_CTRL0_REG(line);
        regWritePtr0 = smemMemGet(devObjPtr, regAddr);
        regAddr = SMEM_CHT_PCL_TCAM_MASK_CTRL1_REG(line);
        regWritePtr1 = smemMemGet(devObjPtr, regAddr);
    }
    else
    {
        regAddr = SMEM_CHT_PCL_TCAM_WORD_CTRL0_REG(line);
        regWritePtr0 = smemMemGet(devObjPtr, regAddr);
        regAddr = SMEM_CHT_PCL_TCAM_WORD_CTRL1_REG(line);
        regWritePtr1 = smemMemGet(devObjPtr, regAddr);
    }

    *regWritePtr0 = data;
    *regWritePtr1 = data;

    /* A long key */
    if (mode)
    {
        if (type == 0)
        {
            regAddr = SMEM_CHT_PCL_TCAM_MASK_CTRL0_REG(line + 512);
            regWritePtr0 = smemMemGet(devObjPtr, regAddr);
            regAddr = SMEM_CHT_PCL_TCAM_MASK_CTRL1_REG(line + 512);
            regWritePtr1 = smemMemGet(devObjPtr, regAddr);
        }
        else
        {
            /* type == 1 - pattern */
            regAddr = SMEM_CHT_PCL_TCAM_WORD_CTRL0_REG(line + 512);
            regWritePtr0 = smemMemGet(devObjPtr, regAddr);
            regAddr = SMEM_CHT_PCL_TCAM_WORD_CTRL1_REG(line + 512);
            regWritePtr1 = smemMemGet(devObjPtr, regAddr);
        }

        *regWritePtr0 = data;
        *regWritePtr1 = data;
    }
}


/**
* @internal smemChtReadPolicyTcamCtrl function
* @endinternal
*
* @brief   Handler for read/write policy TCAM memory
*
* @param[in] devObjPtr                - device object PTR.
* @param[in] line                     - table  number to be read or write
* @param[in] mode                     - short/long key mode
* @param[in] type                     - TCAM data or mask
*/
static void smemChtReadPolicyTcamCtrl (
    IN         SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN         GT_U32  line,
    IN         GT_U32  type,
    IN         GT_U32  mode
)
{
    GT_U32                * regReadPtr;     /* read register's entry pointer */
    GT_U32                * regWritePtr;    /* write register's entry pointer*/
    GT_U32                  regAddr;        /* register's address */

    /* Read Action Table and Policy TCAM Access Control Register */
    regWritePtr = smemMemGet(devObjPtr, SMEM_CHT_PCL_ACTION_TCAM_CTRL_REG(devObjPtr));

    /* TCAM data type */
    if (type == 0)
    {
        regAddr = SMEM_CHT_PCL_TCAM_MASK_CTRL0_REG(line);
        regReadPtr = smemMemGet(devObjPtr, regAddr);
    }
    else
    {
        regAddr = SMEM_CHT_PCL_TCAM_WORD_CTRL0_REG(line);
        regReadPtr = smemMemGet(devObjPtr, regAddr);
    }

    *regWritePtr = *regReadPtr;
    /* A long key */
    if (mode)
    {
        /* TCAM data type */
        if (type == 0)
        {
            regAddr = SMEM_CHT_PCL_TCAM_MASK_CTRL1_REG(line + 512);
            regReadPtr = smemMemGet(devObjPtr, regAddr);
        }
        else
        {
            regAddr = SMEM_CHT_PCL_TCAM_WORD_CTRL1_REG(line + 512);
            regReadPtr = smemMemGet(devObjPtr, regAddr);
        }

        *regWritePtr = *regReadPtr;
    }
}


/**
* @internal smemChtReadWritePclAction function
* @endinternal
*
* @brief   Handler for write Port Protocol Based VLANs and Protocol Based QoS table
*
* @param[in] devObjPtr                - device object PTR.
*                                      address     - Address for ASIC memory.
*                                      memPtr      - Pointer to the register's memory in the simulation.
*                                      param       - Registers' specific parameter.
*                                      inMemPtr    - Pointer to the memory to get register's content.
*/
static void smemChtReadWritePclAction (
    IN         SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN         GT_U32  line,
    IN         GT_U32  rdWr
)
{
    GT_U32                * regPtr;         /* register's entry pointer */
    GT_U32                  regAddr;        /* register's address */

    /* PCL action table address */
    regAddr = SMEM_CHT_PCL_ACTION_TBL_MEM(devObjPtr, line);

    if (rdWr)
    {
        /* Read TCAM data register word 9 */
        regPtr = smemMemGet(devObjPtr, SMEM_CHT_PCL_ACTION_TCAM_DATA9_REG(devObjPtr));
        smemMemSet(devObjPtr, regAddr, regPtr, 3);
    }
    else
    {
        /* Read TCAM data register word 9 */
        regPtr = smemMemGet(devObjPtr, regAddr);
        smemMemSet(devObjPtr, SMEM_CHT_PCL_ACTION_TCAM_DATA9_REG(devObjPtr), regPtr, 3);
    }
}

/**
* @internal smemChtActiveWriteArpTable function
* @endinternal
*
* @brief   Handler for write ARP MAC Destination Address and VLAN/Port MAC SA Table
*
* @param[in] devObjPtr                - device object PTR.
* @param[in] address                  - Address for ASIC memory.
* @param[in] memPtr                   - Pointer to the register's memory in the simulation.
* @param[in] param                    - Registers' specific parameter.
* @param[in] inMemPtr                 - Pointer to the memory to get register's content.
*/
void smemChtActiveWriteArpTable (
    IN         SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN         GT_U32   address,
    IN         GT_U32   memSize,
    IN         GT_U32 * memPtr,
    IN         GT_UINTPTR   param,
    IN         GT_U32 * inMemPtr
)
{
    GT_U32     line;            /* line number in ARP table*/
    GT_U32     rdWrTrig;        /* Read/Write command trigger */
    GT_U32     rdWr;            /* Read or write operation */
    GT_U32     tblSel;          /* Memory access target ARP or MAC SA table */


    rdWrTrig = (*inMemPtr) & 0x1;
    if (rdWrTrig == 0)
    {
        /* If Read/Write trigger 0, store register's value and exit */
        *memPtr = *inMemPtr;
        return;
    }

    /* Read/Write command is preformed */
    rdWr = (*inMemPtr >> 1) & 0x1;
    tblSel = (*inMemPtr >> 2) & 0x1;
    line = (*inMemPtr >> 3) & 0xFFF;

    /* stored as is to be retrieved in called functions */
    *memPtr = *inMemPtr ;

    if (tblSel == 0)
    {
        smemChtReadWriteArpTable(devObjPtr, line, rdWr);
    }
    else
    {
        smemChtReadWriteMacSaTable(devObjPtr, line, rdWr);
    }

    /* Cleared by the device when the read or write action is completed. */
    *memPtr = *inMemPtr & (~1);
}

/**
* @internal smemChtReadWriteArpTable function
* @endinternal
*
* @brief   Handler for read/write ARP MAC table
*
* @param[in] devObjPtr                - device object PTR.
* @param[in] line                     - table  number to be read or write
* @param[in] rdWr                     - The action when there is a trigger
*/
static void smemChtReadWriteArpTable (
    IN         SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN         GT_U32  line,
    IN         GT_U32  rdWr
)
{
    GT_U32                 * regPtr;         /* register's entry pointer */
    GT_U32                   regAddr;        /* register's address */

    /* ARP table address */
    regAddr = SMEM_CHT_ARP_TBL_MEM(devObjPtr, line);

    if (rdWr)
    {
        /* Write Router Header Alteration Tables Access Data0 */
        regPtr = smemMemGet(devObjPtr, SMEM_CHT_ROUT_HA_DATA0_REG(devObjPtr));
        smemMemSet(devObjPtr, regAddr, regPtr, 2);
    }
    else
    {
        /* Read Router Header Alteration Tables Access Data0 */
        regPtr = smemMemGet(devObjPtr, regAddr);
        smemMemSet(devObjPtr, SMEM_CHT_ROUT_HA_DATA0_REG(devObjPtr), regPtr, 2);
    }
}

/**
* @internal smemChtReadWriteMacSaTable function
* @endinternal
*
* @brief   Handler for read/write VLAN/Port MAC SA Table
*
* @param[in] devObjPtr                - device object PTR.
* @param[in] line                     - table  number to be read or write
* @param[in] rdWr                     - The action when there is a trigger
*/
static void smemChtReadWriteMacSaTable (
    IN         SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN         GT_U32  line,
    IN         GT_U32  rdWr
)
{
    GT_U32                 * regPtr;         /* register's entry pointer */
    GT_U32                   regAddr;        /* register's address */

    /* ARP table address */
    regAddr = SMEM_CHT_MAC_SA_TBL_MEM(devObjPtr, line);

    if (rdWr)
    {
        /* Write Router Header Alteration Tables Access Data0 */
        regPtr = smemMemGet(devObjPtr, SMEM_CHT_ROUT_HA_DATA0_REG(devObjPtr));
        smemMemSet(devObjPtr, regAddr, regPtr, 1);
    }
    else
    {
        /* Read Router Header Alteration Tables Access Data0 */
        regPtr = smemMemGet(devObjPtr, regAddr);
        smemMemSet(devObjPtr, SMEM_CHT_ROUT_HA_DATA0_REG(devObjPtr), regPtr, 1);
    }
}


/*******************************************************************************
*   smemChtPclTbl,   smemChtMacTbl,  smemChtPolicerTbl,
*   smemChtTrunkTbl, smemChtVlanTbl
*
* DESCRIPTION:
*   Visualizer specific table memory access functions
*
* INPUTS:
*       devObjPtr   - pointer to device object.
*       address     - address of memory(register or table).
*       memSize     - size of the requested memory
*       param       - extra parameter might be used in
*
* OUTPUTS:
*       None.
*
* RETURNS:
*        pointer to the memory location
*        NULL - if memory not exist
*
* COMMENTS:
*
*******************************************************************************/
static GT_U32 *  smemChtPclTbl(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SCIB_MEMORY_ACCESS_TYPE accessType,
    IN GT_U32 address,
    IN GT_U32 memSize,
    IN GT_UINTPTR param
)
{
    SMEM_CHT_DEV_MEM_INFO  * devMemInfoPtr;
    GT_U32              * regValPtr;
    GT_U32                index;
    GT_U32                tbl;
    GT_U32                member;

    regValPtr = 0;
    devMemInfoPtr = (SMEM_CHT_DEV_MEM_INFO  *)devObjPtr->deviceMemory;

    tbl = (address >> SMEM_CHT_SUB_UNIT_INDEX_FIRST_BIT_CNS) & 0x7;
    member = ((address >> SMEM_CHT_GROUP_INDEX_FIRST_BIT_CNS) & 0xf);
    index = (address & 0x1fff) / 0x4;
    /* Port<n> VLAN and QoS Configuration Entry */
    if (tbl == SMEM_CHT_VLAN_QOS_SUB_UNIT_INDEX_CNS)
    {
        CHECK_MEM_BOUNDS(devMemInfoPtr->pclMem.portVlanQosTblReg,
                         devMemInfoPtr->pclMem.portVlanQosTblRegNum,
                         index, memSize);
        regValPtr = &devMemInfoPtr->pclMem.portVlanQosTblReg[index];
    }
    else
    /* Port<n> Protocol<m> VID and QoS Configuration */
    if (tbl == SMEM_CHT_PROT_VID_QOS_SUB_UNIT_INDEX_CNS)
    {
        CHECK_MEM_BOUNDS(devMemInfoPtr->pclMem.protBasedVlanQosTblReg[member],
                         devMemInfoPtr->pclMem.protBasedVlanQosTblRegNum,
                         index, memSize);
        regValPtr = &devMemInfoPtr->pclMem.protBasedVlanQosTblReg[member][index];
    }
    else
    /* Policy Action Entry<n> (0<=n<1024) */
    if (tbl == SMEM_CHT_PCL_ACTION_SUB_UNIT_INDEX_CNS)
    {
        /* the action table size is 1024 entries * 4 words */
        index = (address & 0x3fff) / 0x4;
        CHECK_MEM_BOUNDS(devMemInfoPtr->pclMem.pceActionsTblReg,
                         devMemInfoPtr->pclMem.pceActionsTblRegNum,
                         index, memSize);
        regValPtr = &devMemInfoPtr->pclMem.pceActionsTblReg[index];
    }
    else
    /* PCL-ID Table entry<n> (0<=n<1152) */
    if (tbl == SMEM_CHT_PCL_ID_SUB_UNIT_INDEX_CNS)
    {
        CHECK_MEM_BOUNDS(devMemInfoPtr->pclMem.pclIdTblReg,
                         devMemInfoPtr->pclMem.pclIdTblRegNum,
                         index, memSize);
        regValPtr = &devMemInfoPtr->pclMem.pclIdTblReg[index];
    }

    return regValPtr;
}
/*******************************************************************************
* smemChtMacTbl
*******************************************************************************/
static GT_U32 *  smemChtMacTbl(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SCIB_MEMORY_ACCESS_TYPE accessType,
    IN GT_U32 address,
    IN GT_U32 memSize,
    IN GT_UINTPTR param
)
{
    SMEM_CHT_DEV_MEM_INFO  * devMemInfoPtr;
    GT_U32              * regValPtr;
    GT_U32                index;

    regValPtr = 0;
    devMemInfoPtr = (SMEM_CHT_DEV_MEM_INFO  *)devObjPtr->deviceMemory;
    index = (address & 0xfffff) / 0x4;

    /* MAC Table Entry */
    CHECK_MEM_BOUNDS(devMemInfoPtr->macFdbMem.macTblReg,
                     devMemInfoPtr->macFdbMem.macTblRegNum,
                     index, memSize);
    regValPtr = &devMemInfoPtr->macFdbMem.macTblReg[index];

    return regValPtr;
}
/*******************************************************************************
* smemChtPolicerTbl
*******************************************************************************/
static GT_U32 *  smemChtPolicerTbl(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SCIB_MEMORY_ACCESS_TYPE accessType,
    IN GT_U32 address,
    IN GT_U32 memSize,
    IN GT_UINTPTR param
)
{
    SMEM_CHT_DEV_MEM_INFO* devMemInfoPtr;
    GT_U32              * regValPtr;
    GT_U32                index;
    GT_U32                tbl;


    regValPtr = 0;
    devMemInfoPtr = (SMEM_CHT_DEV_MEM_INFO  *)devObjPtr->deviceMemory;

    tbl = (address >> SMEM_CHT_SUB_UNIT_INDEX_FIRST_BIT_CNS) & 0x7;
    index = (address & 0xffff) / 0x4;

    /* Policers Table Entry<n> (0<=n<256) */
    if (tbl == SMEM_CHT_POLICER_TBL_SUB_UNIT_INDEX_CNS)
    {
        CHECK_MEM_BOUNDS(devMemInfoPtr->policerMem.policerTblReg,
                         devMemInfoPtr->policerMem.policerTblRegNum,
                         index, memSize);
        regValPtr = &devMemInfoPtr->policerMem.policerTblReg[index];
    }
    else
    /* Policers QoS Remarking and Initial DP Table Entry<n> (0<=n<72) */
    if (tbl == SMEM_CHT_POLICER_QOS_REM_UNIT_INDEX_CNS)
    {
        CHECK_MEM_BOUNDS(devMemInfoPtr->policerMem.policerQosRmTblReg,
                         devMemInfoPtr->policerMem.policerQosRmTblRegNum,
                         index, memSize);
        regValPtr = &devMemInfoPtr->policerMem.policerQosRmTblReg[index];
    }
    else
    /* Policers Counters Table Entry<n> (0<=n<16) */
    if (tbl == SMEM_CHT_POLICER_COUNTER_UNIT_INDEX_CNS)
    {
        index = (address & 0xffff) / 0x10;
        /* Policers Table Entry */
        CHECK_MEM_BOUNDS(devMemInfoPtr->policerMem.policerCntTblReg,
                         devMemInfoPtr->policerMem.policerCntTblRegNum,
                         index, memSize);
        regValPtr = &devMemInfoPtr->policerMem.policerCntTblReg[index];
    }

    return regValPtr;
}
/*******************************************************************************
* smemChtTrunkTbl
*******************************************************************************/
static GT_U32 *  smemChtTrunkTbl(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SCIB_MEMORY_ACCESS_TYPE accessType,
    IN GT_U32 address,
    IN GT_U32 memSize,
    IN GT_UINTPTR param
)
{
    SMEM_CHT_DEV_MEM_INFO  * devMemInfoPtr;
    GT_U32              * regValPtr;
    GT_U32                index;
    GT_U32                tbl;
    GT_U32                member;

    regValPtr = 0;
    devMemInfoPtr = (SMEM_CHT_DEV_MEM_INFO  *)devObjPtr->deviceMemory;

    tbl = (address >> SMEM_CHT_SUB_UNIT_INDEX_FIRST_BIT_CNS) & 0x7;
    member = ((address >> SMEM_CHT_GROUP_INDEX_FIRST_BIT_CNS) & 0xf);
    index = (address & 0xfff) / 0x4;
    /* Trunk table Trunk<n> Member<i> Entry (1<=n<128, 0<=i<8)*/
    if (tbl == SMEM_CHT_TRUNK_TBL_MEMBER_SUB_UNIT_INDEX_CNS)
    {
        if(member > 7)
        {
            skernelFatalError("smemChtTrunkTbl: "\
                "member [%d]out of range\n",member);
        }
        CHECK_MEM_BOUNDS(devMemInfoPtr->preegressMem.trunkTblReg[member],
                         devMemInfoPtr->preegressMem.trunkTblRegNum,
                         index, memSize);
        regValPtr = &devMemInfoPtr->preegressMem.trunkTblReg[member][index];
    }
    else
    /* QoSProfile to QoS table Entry<n> (0<=n<72) */
    if (tbl == SMEM_CHT_QOS_PROFILE_UNIT_INDEX_CNS)
    {
        CHECK_MEM_BOUNDS(devMemInfoPtr->preegressMem.qosTblReg,
                         devMemInfoPtr->preegressMem.qosTblRegNum,
                         index, memSize);
        regValPtr = &devMemInfoPtr->preegressMem.qosTblReg[index];
    }
    else
    /* CPU Code Table Entry<n> (0<=n<256) */
    if (tbl == SMEM_CHT_CPU_CODE_UNIT_INDEX_CNS)
    {
        CHECK_MEM_BOUNDS(devMemInfoPtr->preegressMem.cpuCodeTblReg,
                         devMemInfoPtr->preegressMem.cpuCodeTblRegNum,
                         index, memSize);
        regValPtr = &devMemInfoPtr->preegressMem.cpuCodeTblReg[index];
    }
    else
    /* Statistical Rate Limits Table Entry<n> (0<=n<32) */
    if (tbl == SMEM_CHT_STAT_RATE_LIM_UNIT_INDEX_CNS)
    {
        CHECK_MEM_BOUNDS(devMemInfoPtr->preegressMem.statRateTblReg,
                         devMemInfoPtr->preegressMem.statRateTblRegNum,
                         index, memSize);
        regValPtr = &devMemInfoPtr->preegressMem.statRateTblReg[index];
    }
    else
    /* Port<n> Ingress STC Table Entry (0<=n<27) */
    if (tbl == SMEM_CHT_PORT_INGRESS_STC_UNIT_INDEX_CNS)
    {
        CHECK_MEM_BOUNDS(devMemInfoPtr->preegressMem.portInStcTblReg,
                         devMemInfoPtr->preegressMem.portInStcTblRegNum,
                         index, memSize);
        regValPtr = &devMemInfoPtr->preegressMem.portInStcTblReg[index];
    }

    return regValPtr;
}
/*******************************************************************************
* smemChtVlanTbl
*******************************************************************************/
static GT_U32 *  smemChtVlanTbl(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SCIB_MEMORY_ACCESS_TYPE accessType,
    IN GT_U32 address,
    IN GT_U32 memSize,
    IN GT_UINTPTR param
)
{
    SMEM_CHT_DEV_MEM_INFO  * devMemInfoPtr;
    GT_U32              * regValPtr;
    GT_U32                index;
    GT_U32                tbl;

    regValPtr = 0;
    devMemInfoPtr = (SMEM_CHT_DEV_MEM_INFO  *)devObjPtr->deviceMemory;

    tbl = (address >> SMEM_CHT_SUB_UNIT_INDEX_FIRST_BIT_CNS) & 0x7;
    index = (address & 0xffff) / 0x4;
    /* VLAN<n> Entry (0<=n<4096) */
    if (tbl == SMEM_CHT_VLAN_TBL_SUB_UNIT_INDEX_CNS)
    {
        CHECK_MEM_BOUNDS(devMemInfoPtr->vlanTblMem.vlanTblReg,
                         devMemInfoPtr->vlanTblMem.vlanTblRegNum,
                         index, memSize);
        regValPtr = &devMemInfoPtr->vlanTblMem.vlanTblReg[index];
    }
    else
    /* Multicast Group<n> Entry (0<=n<4096) */
    if (tbl == SMEM_CHT_MCST_TBL_SUB_UNIT_INDEX_CNS)
    {
        CHECK_MEM_BOUNDS(devMemInfoPtr->vlanTblMem.mcstTblReg,
                         devMemInfoPtr->vlanTblMem.mcstTblRegNum,
                         index, memSize);
        regValPtr = &devMemInfoPtr->vlanTblMem.mcstTblReg[index];
    }
    else
    /* Span State Group<n> Entry (0<=n<256) */
    if (tbl == SMEM_CHT_STP_TBL_SUB_UNIT_INDEX_CNS)
    {
        CHECK_MEM_BOUNDS(devMemInfoPtr->vlanTblMem.spanTblReg,
                         devMemInfoPtr->vlanTblMem.spanTblRegNum,
                         index, memSize);
        regValPtr = &devMemInfoPtr->vlanTblMem.spanTblReg[index];
    }

    return regValPtr;
}

/*******************************************************************************
/ ARP table and MAC SA table
*******************************************************************************/
static GT_U32 *  smemChtArpTbl(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SCIB_MEMORY_ACCESS_TYPE accessType,
    IN GT_U32 address,
    IN GT_U32 memSize,
    IN GT_UINTPTR param
)
{
    SMEM_CHT_DEV_MEM_INFO  * devMemInfoPtr;
    GT_U32              * regValPtr;
    GT_U32                index;
    GT_U32                tbl;

    regValPtr = 0;
    devMemInfoPtr = (SMEM_CHT_DEV_MEM_INFO  *)devObjPtr->deviceMemory;

    tbl = (address >> SMEM_CHT_SUB_UNIT_INDEX_FIRST_BIT_CNS) & 0x1;

    if (tbl == SMEM_CHT_ARP_TBL_SUB_UNIT_INDEX_CNS)
    {
        /* ARP Entry (0<=n<1024) */
        index = (address & 0x1fff) / 0x4;
        CHECK_MEM_BOUNDS(devMemInfoPtr->ipMem.arpTblReg,
                         devMemInfoPtr->ipMem.arpTblRegNum,
                         index, memSize);
        regValPtr = &devMemInfoPtr->ipMem.arpTblReg[index];
    }
    else
    if (tbl == SMEM_CHT_MAC_SA_TBL_SUB_UNIT_INDEX_CNS)
    {
        /* MAC SRC Entry (0<=n<4096) */
        index = (address & 0x1ffff) / 0x4;
        CHECK_MEM_BOUNDS(devMemInfoPtr->ipMem.macSaTblReg,
                         devMemInfoPtr->ipMem.macSaTblRegNum,
                         index, memSize);
        regValPtr = &devMemInfoPtr->ipMem.macSaTblReg[index];
    }

    return regValPtr;
}

/**
* @internal smemChtActiveReadBreachStatusReg function
* @endinternal
*
* @brief   Definition of the Active register clear function.
*
* @param[in] devObjPtr                - device object PTR.
* @param[in] address                  - Address for ASIC memory.
* @param[in] memPtr                   - Pointer to the register's memory in the simulation.
* @param[in] param                    - Registers' specific parameter(global interrupt bit number)
*
* @param[out] outMemPtr                - Pointer to the memory to copy register's content.
*/
void smemChtActiveReadBreachStatusReg
(
    IN         SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN         GT_U32   address,
    IN         GT_U32   memSize,
    IN         GT_U32 * memPtr,
    IN         GT_UINTPTR   param,
    OUT        GT_U32 * outMemPtr
)
{
    GT_U32  *internalMemPtr;                /* pointer to internal memory */

    /* copy registers content to the output memory */
    *outMemPtr = *memPtr;

    internalMemPtr = CHT_INTERNAL_MEM_PTR(devObjPtr,
        CHT_INTERNAL_SIMULATION_USE_MEM_SECURITY_BREACH_STATUS_E);

    /* state that the CPU read the registers and device is allowed to update them again */
    SMEM_U32_SET_FIELD(internalMemPtr[0],0,1,1);
}

/**
* @internal smemChtActiveReadMsg2Cpu function
* @endinternal
*
* @brief   Read Message to CPU Register - simulate read from FDB FIFO.
*
* @param[out] outMemPtr                - Pointer to the memory to copy register's content.
*/
void smemChtActiveReadMsg2Cpu
(
    IN         SKERNEL_DEVICE_OBJECT * deviceObjPtr,
    IN         GT_U32   address,
    IN         GT_U32   memSize,
    IN         GT_U32 * memPtr,
    IN         GT_UINTPTR   param,
    OUT        GT_U32 * outMemPtr
)
{
    GT_U32                 * fifoBufferPrt; /* pointer to MAC msg FIFO buffer */
    SMEM_CHT_DEV_COMMON_MEM_INFO  * memInfoPtr; /* pointer to Salsa memory */
    GT_U32                 i;
    GT_U32                  numOfWordsToLoop;

    /* get pointer to the start of FIFO for MAC update messages */
    memInfoPtr =  (SMEM_CHT_DEV_COMMON_MEM_INFO *)(deviceObjPtr->deviceMemory);
    fifoBufferPrt = memInfoPtr->auqFifoMem[smemChtInstanceGet(deviceObjPtr,SMEM_CHT_MEMORY_DEVICE_ON_CHIP_FIFO_AUQ_E)].macUpdFifoRegs;

    /* output value */
    *outMemPtr = *fifoBufferPrt;

    if (*fifoBufferPrt == SMEM_CHEETAH_INVALID_MAC_MSG_CNS)
    {
        /* buffer is empty, there is no new message */
        return;
    }

    numOfWordsToLoop = deviceObjPtr->numOfWordsInAuMsg * MAC_ON_CHIP_FIFO_NUM_ENTRIES_CNS;

    /* shift other words up */
    for (i = 1; i < numOfWordsToLoop; i++)
    {
        fifoBufferPrt[i - 1] = fifoBufferPrt[i];
    }

    /* invalidate last word */
    fifoBufferPrt[numOfWordsToLoop - 1] = SMEM_CHEETAH_INVALID_MAC_MSG_CNS;
}

/**
* @internal smemChtActiveAuqBaseWrite function
* @endinternal
*
* @brief   Address Update Queue Base Address active write.
*
* @param[in] devObjPtr                - device object PTR.
* @param[in] address                  - Address for ASIC memory.
* @param[in] memPtr                   - Pointer to the register's memory in the simulation.
* @param[in] param                    - Registers' specific parameter.
* @param[in] inMemPtr                 - Pointer to the memory to get register's content.
*/
void smemChtActiveAuqBaseWrite
(
    IN         SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN         GT_U32   address,
    IN         GT_U32   memSize,
    IN         GT_U32 * memPtr,
    IN         GT_UINTPTR   param,
    IN         GT_U32 * inMemPtr
)
{
    CHT_AUQ_MEM  * auqMemPtr;
    GT_U32         auqSize;

    *memPtr = *inMemPtr;

    auqMemPtr = SMEM_CHT_MAC_AUQ_MEM_GET(devObjPtr);

    if (SKERNEL_IS_CHEETAH3_DEV(devObjPtr))
    {
        /* AUQSize */
        smemRegFldGet(devObjPtr, SMEM_CHT_AUQ_CTRL_REG(devObjPtr), 0, 30, &auqSize);
    }
    else
    {
        /* AUQSize */
        smemRegFldGet(devObjPtr, SMEM_CHT_AUQ_CTRL_REG(devObjPtr), 0, 31, &auqSize);
    }

    /* First time */
    if(auqMemPtr->baseInit ==  GT_FALSE)
    {
       auqMemPtr->auqBase = *inMemPtr;
       auqMemPtr->baseInit = GT_TRUE;
       auqMemPtr->auqBaseSize = auqSize;
       auqMemPtr->auqBaseValid = GT_TRUE;
    }
    else
    {
        if (auqMemPtr->auqBaseValid == GT_TRUE)
        {
            auqMemPtr->auqShadow = *inMemPtr;
            auqMemPtr->auqShadowSize = auqSize;
            auqMemPtr->auqShadowValid = GT_TRUE;
        }
        else
        {
            auqMemPtr->auqBase = *inMemPtr;
            auqMemPtr->auqOffset = 0;
            auqMemPtr->auqBaseSize = auqSize;
            auqMemPtr->auqBaseValid = GT_TRUE;

            /* indicate that AUQ is NOT full */
            if (SKERNEL_IS_CHEETAH3_DEV(devObjPtr))
            {
                /* <GeneralAUQFull> */
                smemRegFldSet(devObjPtr, SMEM_CHT_AUQ_CTRL_REG(devObjPtr), 30, 1, 0);
            }
        }
    }
}

/**
* @internal smemChtResetAuq function
* @endinternal
*
* @brief   Reset AUQ structure.
*
* @param[in] devObjPtr                - pointer to device object.
*
* @param[out] devObjPtr                - pointer to device object.
*/
static void smemChtResetAuq
(
    IN  SKERNEL_DEVICE_OBJECT * devObjPtr
)
{
    CHT_AUQ_MEM  * auqMemPtr;
    SMEM_CHT_DEV_COMMON_MEM_INFO  * commonDevMemInfoPtr =
        (SMEM_CHT_DEV_COMMON_MEM_INFO *)devObjPtr->deviceMemory;

    auqMemPtr = SMEM_CHT_MAC_AUQ_MEM_GET(devObjPtr);

    memset(auqMemPtr,0,sizeof(commonDevMemInfoPtr->auqMem));
}

/**
* @internal smemCht2ResetFuq function
* @endinternal
*
* @brief   Reset FU message Queue structure. -- not relevant to ch1 devices
*
* @param[in] devObjPtr                - pointer to device object.
*
* @param[out] devObjPtr                - pointer to device object.
*/
static void smemCht2ResetFuq
(
    IN  SKERNEL_DEVICE_OBJECT * devObjPtr
)
{
    CHT2_FUQ_MEM  * fuqMemPtr;
    SMEM_CHT_DEV_COMMON_MEM_INFO  * commonDevMemInfoPtr =
        (SMEM_CHT_DEV_COMMON_MEM_INFO *)devObjPtr->deviceMemory;

    fuqMemPtr = SMEM_CHT_MAC_FUQ_MEM_GET(devObjPtr);

    memset(fuqMemPtr,0,sizeof(commonDevMemInfoPtr->fuqMem));
}

/**
* @internal smemChtActiveWriteNewFdbMsgFromCpu function
* @endinternal
*
* @brief   Trigger for start of From CPU FDB message processing, called from
*         Application/PSS task context by write to Message From CPU Management register
* @param[in] devObjPtr                - device object PTR.
* @param[in] address                  - Address for ASIC memory.
* @param[in] memPtr                   - Pointer to the register's memory in the simulation.
* @param[in] memSize                  - Size of memory to be written
* @param[in] param                    - Registers' specific parameter.
* @param[in] inMemPtr                 - Pointer to the memory to get register's content.
*/
void smemChtActiveWriteNewFdbMsgFromCpu
(
    IN         SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN         GT_U32   address,
    IN         GT_U32   memSize,
    IN         GT_U32 * memPtr,
    IN         GT_UINTPTR   param,
    IN         GT_U32 * inMemPtr
)
{
    SBUF_BUF_ID bufferId;               /* buffer */
    GT_U32 fieldVal;                    /* register's field value */
    GT_U32 * regPtr;                    /* register's entry pointer */
    GT_U8  * dataPtr;                   /* pointer to the data in the buffer */
    GT_U32 dataSize;                    /* data size */

    /* New Message Trigger */
    fieldVal = SMEM_U32_GET_FIELD(inMemPtr[0], 0, 1);
    if (fieldVal == 0)
    {
        return;
    }

    /* Get buffer */
    bufferId = sbufAlloc(devObjPtr->bufPool, 4*devObjPtr->numOfWordsInAuMsg);

    if (bufferId == NULL)
    {
        simWarningPrintf(" smemChtActiveWriteNewFdbMsgFromCpu: "\
                "no buffers to update MAC table\n");
        return;
    }

    /* Get actual data pointer */
    sbufDataGet(bufferId, &dataPtr, &dataSize);

    /* Message from CPU Register0 Entry */
    regPtr = smemMemGet(devObjPtr, SMEM_CHT_MSG_FROM_CPU_0_REG(devObjPtr));

    /* copy fdb update message to buffer */
    memcpy(dataPtr, regPtr, 4*devObjPtr->numOfWordsInAuMsg);

    /* set source type of buffer */
    bufferId->srcType = SMAIN_SRC_TYPE_CPU_E;

    /* set message type of buffer */
    bufferId->dataType = SMAIN_MSG_TYPE_FDB_UPDATE_E;

    *memPtr = 0x1;

    if(devObjPtr->numOfPipes > 2)
    {
        /* needed in Falcon to identify the 'TileId' */
        bufferId->pipeId = smemGetCurrentPipeId(devObjPtr);
    }

    /* put buffer to queue */
    squeBufPut(devObjPtr->queueId, SIM_CAST_BUFF(bufferId));
}

/**
* @internal smemChtActiveWriteFdbActionTrigger function
* @endinternal
*
* @brief   Trigger for start of FDB Action processing, called from Application/PSS
*         task context by write to MAC Table Action0 Register
* @param[in] devObjPtr                - device object PTR.
* @param[in] address                  - Address for ASIC memory.
* @param[in] memPtr                   - Pointer to the register's memory in the simulation.
* @param[in] memSize                  - Size of memory to be written
* @param[in] param                    - Registers' specific parameter.
* @param[in] inMemPtr                 - Pointer to the memory to get register's content.
*/
void smemChtActiveWriteFdbActionTrigger
(
    IN         SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN         GT_U32   address,
    IN         GT_U32   memSize,
    IN         GT_U32 * memPtr,
    IN         GT_UINTPTR   param,
    IN         GT_U32 * inMemPtr
)
{
    SBUF_BUF_ID bufferId;               /* buffer */
    GT_U32 fieldVal;                    /* register's field value */
    GT_U8  * dataPtr;                   /* pointer to the data in the buffer */
    GT_U32 dataSize;                    /* data size */
    GT_U32 FDBUploadEnable;             /*FDB Upload Enable*/
    GT_U32 ActionMode;
    GT_BIT  previousTriggerBit;         /* value of trigger bit before current write */

    /* get value of trigger bit before current write */
    previousTriggerBit = SMEM_U32_GET_FIELD(memPtr[0], 1, 1);

    /* update content of register */
    *memPtr = *inMemPtr;

    if(previousTriggerBit)
    {
        /* the action was enabled before we started the operation ...
            we not need to send another 'trigger action' to the skernel task */
        return;
    }


    /* Aging Trigger */
    fieldVal = SMEM_U32_GET_FIELD(memPtr[0], 1, 1);
    if (fieldVal == 0)
    {
        return;
    }

    /* Action Mode */
    ActionMode  = SMEM_U32_GET_FIELD(memPtr[0], 3, 2);

    /* Get buffer */
    bufferId = sbufAlloc(devObjPtr->bufPool, SFDB_MAC_TBL_ACT_BYTES);

    if (bufferId == NULL)
    {
        simWarningPrintf(" smemCht2ActiveWriteFdbActionTrigger: "\
                "no buffers for trigger aging\n");
        return;
    }
    /* Get actual data pointer */
    sbufDataGet(bufferId, &dataPtr, &dataSize);

    /* copy MAC table action 8 words to buffer */
    memcpy(dataPtr, memPtr, SFDB_MAC_TBL_ACT_BYTES);

    /* set source type of buffer */
    bufferId->srcType = SMAIN_SRC_TYPE_CPU_E;

    if(SMEM_CHT_IS_SIP5_GET(devObjPtr))
    {
        smemRegFldGet(devObjPtr, address, 18,1, &FDBUploadEnable);
    }
    else
    if(! SKERNEL_DEVICE_FAMILY_CHEETAH_1_ONLY(devObjPtr))
    {
        smemRegFldGet(devObjPtr, SMEM_CHT_MAC_TBL_ACTION2_REG(devObjPtr), 30,1, &FDBUploadEnable);
    }
    else
    {
        /* the device not supports FDB upload */
        FDBUploadEnable = 0;
    }

    /* check FDB upload action */
    if ((ActionMode == 1) /* trigger aging no remove */ &&
        FDBUploadEnable)
    {
        /* set message type of buffer */
        bufferId->dataType = SMAIN_MSG_TYPE_FDB_UPLOAD_E;
    }
    else
    {
        bufferId->dataType = SMAIN_CPU_FDB_ACT_TRG_E;
    }

    /* indication to set
        devObjPtr->fdbAgingDaemonInfo.indexInFdb = 0 when stating to process
        the buffer for the first time */
    bufferId->userInfo.extValue = 1;
    if(devObjPtr->numOfPipes > 2)
    {
        /* needed in Falcon to identify the 'TileId' */
        bufferId->pipeId = smemGetCurrentPipeId(devObjPtr);
    }

    /* put buffer to queue */
    squeBufPut(devObjPtr->queueId, SIM_CAST_BUFF(bufferId));
}

/**
* @internal smemChtActiveWriteStartFromCpuDma function
* @endinternal
*
* @brief   Trigger for Start From CPU Frame SDMA, called from Application/PSS task
*         context by write to Transmit Queue Command Register (TQCR)
* @param[in] devObjPtr                - device object PTR.
* @param[in] address                  - Address for ASIC memory.
* @param[in] memPtr                   - Pointer to the register's memory in the simulation.
* @param[in] memSize                  - Size of memory to be written
* @param[in] param                    - Registers' specific parameter.
* @param[in] inMemPtr                 - Pointer to the memory to get register's content.
*/
void smemChtActiveWriteStartFromCpuDma
(
    IN         SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN         GT_U32   address,
    IN         GT_U32   memSize,
    IN         GT_U32 * memPtr,
    IN         GT_UINTPTR   param,
    IN         GT_U32 * inMemPtr
)
{
    SBUF_BUF_ID bufferId;               /* buffer */
    GT_U32 txSdmaQueueBmpDis;            /* bmp of disabled Q */
    GT_U32 txSdmaQueueBmpEn;            /* bmp of enabled Q */
    GT_U8  *dataPtr;                   /* pointer to the data in the buffer */
    GT_U32 dataSize;                    /* data size */
    GT_U32  origRegValue;/* original value in register before writing to it */
    GT_U32 txQue;                       /* TxQ number */
    GT_U32 packetGenerationEnable;               /* bypass owner bit is enabled */
    GT_U32 * txSdmaPktGenCfgRegPtr;     /* Pointer to register entry value */
    GT_U32  mgUnit = smemGetCurrentMgUnitIndex(devObjPtr);

    origRegValue = *memPtr;

    /*Writing 0 to the DISQ bit has no effect.*/
    /*Writing 0 to the ENQ bit has no effect.*/
    *memPtr |= inMemPtr[0];

    /* Disable Queue [7:0] */
    txSdmaQueueBmpDis = SMEM_U32_GET_FIELD(inMemPtr[0], 8, 8);

    /* Writing 1 to the DISQ bit resets the matching ENQ bit. */
    *memPtr &= ~txSdmaQueueBmpDis;

    /* Enable Queue [7:0] */
    txSdmaQueueBmpEn = SMEM_U32_GET_FIELD(memPtr[0], 0, 8);

    if(devObjPtr->oamSupport.keepAliveSupport)
    {
        /* Loop all queues and update data base */
        for(txQue = 0; txQue < 8; txQue++)
        {
            /* Tx SDMA Packet Generator Config Queue */
            txSdmaPktGenCfgRegPtr =
                &devObjPtr->sdmaTransmitInfo[mgUnit].sdmaTransmitData[txQue].txQueuePacketGenCfgReg;
            packetGenerationEnable = SMEM_U32_GET_FIELD(*txSdmaPktGenCfgRegPtr, 0, 1);

            if(packetGenerationEnable)
            {
                devObjPtr->sdmaTransmitInfo[mgUnit].sdmaTransmitData[txQue].txQueueEn =
                    ((txSdmaQueueBmpEn >> txQue) & 0x1) ? GT_TRUE : GT_FALSE;
            }
            else
            {
                devObjPtr->sdmaTransmitInfo[mgUnit].sdmaTransmitData[txQue].txQueueEn =
                    GT_FALSE;
            }
        }
    }

    /* Writing 1 to the ENQ bit of a TxSDMA that is already in enabled state
       has no effect. */
    /* so remove the bits already on */
    txSdmaQueueBmpEn &= ~origRegValue;

    /* Writing 1 to the ENQ bit resets the matching DISQ bit. */
    *memPtr &= ~(txSdmaQueueBmpEn << 8);

    if (txSdmaQueueBmpEn == 0)
    {
        /* no action to do (beside updating the register) */
        return;
    }

    /* Get buffer -- allocate size for max supported frame size */
    bufferId = sbufAlloc(devObjPtr->bufPool, SBUF_DATA_SIZE_CNS);/*2000*/

    if (bufferId == NULL)
    {
        simWarningPrintf(" smemChtActiveWriteStartFromCpuDma: "\
                "no buffers to start TX SDMA\n");

        /* clear the new bits , so next time we will try again to send message */
        *memPtr &= (~txSdmaQueueBmpEn);

        return;
    }
    /* Get actual data pointer */
    sbufDataGet(bufferId, &dataPtr, &dataSize);

    /* Copy TxENQ[7:0] bitmap to user info structure*/
    bufferId->userInfo.data.txSdmaQueueBmp = txSdmaQueueBmpEn;

    /* Set up user info data type */
    bufferId->userInfo.type = SAPI_USER_INFO_TYPE_SAPI_TX_SDMA_QUE_BMP_E;

    /* set source type of buffer */
    bufferId->srcType = SMAIN_SRC_TYPE_CPU_E;

    /* set message type of buffer */
    bufferId->dataType = SMAIN_CPU_TX_SDMA_QUEUE_E;

    /* needed in Falcon to identify the 'mgUnit' */
    bufferId->mgUnitId = smemGetCurrentMgUnitIndex(devObjPtr);

    /* put buffer to queue */
    squeBufPut(devObjPtr->queueId, SIM_CAST_BUFF(bufferId));
}

/**
* @internal smemChtActiveWriteMacInterruptsMaskReg function
* @endinternal
*
* @brief   the application changed the value of the interrupts mask register.
*         check if there is waiting interrupt for that.
* @param[in] devObjPtr                - device object PTR.
* @param[in] address                  - Address for ASIC memory.
* @param[in] memPtr                   - Pointer to the register's memory in the simulation.
* @param[in] memSize                  - Size of memory to be written
* @param[in] param                    - Registers' specific parameter.
* @param[in] inMemPtr                 - Pointer to the memory to get register's content.
*/
void smemChtActiveWriteMacInterruptsMaskReg (
    IN         SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN         GT_U32   address,
    IN         GT_U32   memSize,
    IN         GT_U32 * memPtr,
    IN         GT_UINTPTR   param,
    IN         GT_U32 * inMemPtr
)
{
    GT_U32 dataArray[3];                     /* Data array to put into buffer */

    /* Address of MAC table interrupt cause register */
    dataArray[0] = SMEM_CHT_MAC_TBL_INTR_CAUSE_REG(devObjPtr);
    /* Address of MAC table interrupt mask register */
    dataArray[1] = SMEM_CHT_MAC_TBL_INTR_CAUSE_MASK_REG(devObjPtr);
    /* FDB Interrupt Cause register summary bit */
    dataArray[2] = SMEM_CHT_FDB_SUM_INT(devObjPtr);

    /* Call for common interrupt mask active write function */
    smemChtActiveWriteInterruptsMaskReg(devObjPtr, address, memSize, memPtr,
                                        (GT_UINTPTR)dataArray, inMemPtr);
}

/**
* @internal smemChtActiveWriteGlobalReg function
* @endinternal
*
* @brief   the application changed the value of the global register.
*
* @param[in] devObjPtr                - device object PTR.
* @param[in] address                  - Address for ASIC memory.
* @param[in] memPtr                   - Pointer to the register's memory in the simulation.
* @param[in] memSize                  - Size of memory to be written
* @param[in] param                    - Registers' specific parameter.
* @param[in] inMemPtr                 - Pointer to the memory to get register's content.
*/
void smemChtActiveWriteGlobalReg (
    IN         SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN         GT_U32   address,
    IN         GT_U32   memSize,
    IN         GT_U32 * memPtr,
    IN         GT_UINTPTR   param,
    IN         GT_U32 * inMemPtr
)
{
    SMEM_ACTIVE_MEM_ENTRY_STC *activeMemPtr;
    SBUF_BUF_ID bufferId;               /* Buffer ID */
    GT_U8  * dataPtr;                   /* Pointer to the data in the buffer */
    GT_U32 dataSize;                    /* Data size */
    GT_BIT  supportSoftReset; /*indication that we support soft reset from this register*/

    /* save the new value */
    *memPtr = *inMemPtr;

    supportSoftReset = SMEM_CHT_IS_SIP5_GET(devObjPtr) ? 0 :/* sip5 --> DFX server */
                       SKERNEL_IS_XCAT2_DEV(devObjPtr) ? 0 :/* xcat2,lion2--> DFX server */
                       1;

    /* Device Soft Reset */
    if(supportSoftReset && snetFieldValueGet(memPtr, 16, 1))
    {
        simulationSoftResetDone = 0;

        /* Get buffer */
        bufferId = sbufAlloc(devObjPtr->bufPool, sizeof(GT_U32));

        if (bufferId == NULL)
        {
            simWarningPrintf("smemChtActiveWriteGlobalReg: no buffers to trigger soft reset \n");
            return;
        }

        /* Set initStage to  InitializeDuringReset */
        snetFieldValueSet(memPtr, 17, 2, 0);

        /* Cheetah 3 and above(PEX devices) support the option to unbind/bind SCIB interface */
        if(SKERNEL_IS_CHEETAH3_DEV(devObjPtr))
        {
            /* Reset the PEX on Soft Reset */
            if(snetFieldValueGet(memPtr, 24, 1) == 0)
            {
                /* Unbind SCIB interface */
                scibUnBindDevice(devObjPtr->deviceId);
            }
        }

        /* Get actual data pointer */
        sbufDataGet(bufferId, &dataPtr, &dataSize);

        /* Copy Global Control Register to buffer */
        memcpy(dataPtr, memPtr, sizeof(GT_U32));

        /* Set source type of buffer */
        bufferId->srcType = SMAIN_SRC_TYPE_CPU_E;

        /* Set message type of buffer */
        bufferId->dataType = SMAIN_MSG_TYPE_SOFT_RESET_E;

        /* Put buffer to queue */
        squeBufPut(devObjPtr->queueId, SIM_CAST_BUFF(bufferId));

        return;
    }

    if(!SMEM_CHT_IS_SIP5_GET(devObjPtr) &&
       !SKERNEL_IS_XCAT3_BASED_DEV(devObjPtr))
    {
        /* check the pciEn bit */
        if((1<<20) & (*memPtr))
        {
            devObjPtr->isPciCompatible = 1;
        }
        else
        {
            devObjPtr->isPciCompatible = 0;
        }
    }

    if(SKERNEL_IS_XCAT_DEV(devObjPtr))
        return; /* xcat uses both pci and smi(for mii) */

    if(devObjPtr->globalInterruptCauseRegWriteBitmap_pci !=
       devObjPtr->globalInterruptCauseRegWriteBitmap_nonPci)
    {
        activeMemPtr = devObjPtr->activeMemPtr;

        while(activeMemPtr->address != END_OF_TABLE)
        {
            if(activeMemPtr->address == SMEM_CHT_GLB_INT_CAUSE_REG(devObjPtr))
            {
                /* update the mask of bits that can be set */
                activeMemPtr->writeFunParam =
                    devObjPtr->isPciCompatible ?
                        devObjPtr->globalInterruptCauseRegWriteBitmap_pci :
                        devObjPtr->globalInterruptCauseRegWriteBitmap_nonPci;
                break;
            }

            activeMemPtr++;
        }
    }

}


/**
* @internal smemChtActiveWritePortInterruptsMaskReg function
* @endinternal
*
* @brief   The application changed the value of the interrupts mask register.
*/
void smemChtActiveWritePortInterruptsMaskReg (
    IN         SKERNEL_DEVICE_OBJECT * devPtr,
    IN         GT_U32   address,
    IN         GT_U32   memSize,
    IN         GT_U32 * memPtr,
    IN         GT_UINTPTR   param,
    IN         GT_U32 * inMemPtr
)
{
    GT_U32 dataArray[3];                     /* Data array to put into buffer */
    GT_U32 port;

    port = CHT_FROM_ADDRESS_GET_PORT_CNS(devPtr,address);

    /* Address of Port<n> interrupt cause register */
    dataArray[0] = SMEM_CHT_PORT_INT_CAUSE_REG(devPtr, port);
    /* Address of Port<n> table interrupt mask register */
    dataArray[1] = SMEM_CHT_PORT_INT_CAUSE_MASK_REG(devPtr, port);
    /* Ports Summary Interrupt register SUM bit. */
    dataArray[2] = SMEM_CHT_PORT_CAUSE_INT_SUM_BMP(devPtr,port);

    /* Call for common interrupt mask active write function */
    smemChtActiveWriteInterruptsMaskReg(devPtr, address, memSize, memPtr,
                                        (GT_UINTPTR)dataArray, inMemPtr);
}

/**
* @internal smemChtActiveWriteRcvSdmaInterruptsMaskReg function
* @endinternal
*
* @brief   The application changed the value of the interrupts mask register.
*/
void smemChtActiveWriteRcvSdmaInterruptsMaskReg (
    IN         SKERNEL_DEVICE_OBJECT * devPtr,
    IN         GT_U32   address,
    IN         GT_U32   memSize,
    IN         GT_U32 * memPtr,
    IN         GT_UINTPTR   param,
    IN         GT_U32 * inMemPtr
)
{
    GT_U32 dataArray[3];                     /* Data array to put into buffer */

    /* Address of SDMA Interrupt Registers */
    dataArray[0] = SMEM_CHT_SDMA_INT_CAUSE_REG(devPtr);
    /* Address of SDMA Interrupt Mask Register */
    dataArray[1] = SMEM_CHT_SDMA_INT_MASK_REG(devPtr);
    /* Receive SDMA Interrupt Cause Register Summery Bit */
    dataArray[2] = SMEM_CHT_RX_SDMA_SUM_INT(devPtr);

    /* Call for common interrupt mask active write function */
    smemChtActiveWriteInterruptsMaskReg(devPtr, address, memSize, memPtr,
                                        (GT_UINTPTR)dataArray, inMemPtr);
}

/**
* @internal smemChtActiveWriteBridgeInterruptsMaskReg function
* @endinternal
*
* @brief   The application changed the value of the interrupts mask register.
*/
void smemChtActiveWriteBridgeInterruptsMaskReg (
    IN         SKERNEL_DEVICE_OBJECT * devPtr,
    IN         GT_U32   address,
    IN         GT_U32   memSize,
    IN         GT_U32 * memPtr,
    IN         GT_UINTPTR   param,
    IN         GT_U32 * inMemPtr
)
{
    GT_U32 dataArray[3];                     /* Data array to put into buffer */

    /* Address of bridge interrupt cause register */
    dataArray[0] = SMEM_CHT_BRIDGE_INT_CAUSE_REG(devPtr);
    /* Address of bridge interrupt mask register */
    dataArray[1] = SMEM_CHT_BRIDGE_INT_MASK_CAUSE_REG(devPtr);
    /* L2 Bridge Interrupt Cause register SUM bit. */
    dataArray[2] = SMEM_CHT_L2I_SUM_INT(devPtr);

    /* Call for common interrupt mask active write function */
    smemChtActiveWriteInterruptsMaskReg(devPtr, address, memSize, memPtr,
                                        (GT_UINTPTR)dataArray, inMemPtr);
}

/**
* @internal smemChtActiveWriteHgsInterruptsMaskReg function
* @endinternal
*
* @brief   The application changed the value of the interrupts mask register.
*/
void smemChtActiveWriteHgsInterruptsMaskReg (
    IN         SKERNEL_DEVICE_OBJECT * devPtr,
    IN         GT_U32   address,
    IN         GT_U32   memSize,
    IN         GT_U32 * memPtr,
    IN         GT_UINTPTR   param,
    IN         GT_U32 * inMemPtr
)
{
    GT_U32 dataArray[3];                     /* Data array to put into buffer */
    GT_U32 port;

    port = (address >> 8) & 0x3f;

    /* Address of HGS interrupt cause register */
    dataArray[0] = SMEM_CHT_PORT_INT_CAUSE_REG(devPtr, port);
    /* Address of HGS interrupt mask register */
    dataArray[1] = SMEM_CHT_PORT_INT_CAUSE_MASK_REG(devPtr, port);
    /* Ports Summary Interrupt register SUM bit */
    dataArray[2] = (1 << port);

    /* Call for common interrupt mask active write function */
    smemChtActiveWriteInterruptsMaskReg(devPtr, address, memSize, memPtr,
                                        (GT_UINTPTR)dataArray, inMemPtr);
}
/**
* @internal smemChtActiveWriteTransSdmaInterruptsMaskReg function
* @endinternal
*
* @brief   The application changed the value of the interrupts mask register.
*/
void smemChtActiveWriteTransSdmaInterruptsMaskReg (
    IN         SKERNEL_DEVICE_OBJECT * devPtr,
    IN         GT_U32   address,
    IN         GT_U32   memSize,
    IN         GT_U32 * memPtr,
    IN         GT_UINTPTR   param,
    IN         GT_U32 * inMemPtr
)
{
    GT_U32 dataArray[3];                     /* Data array to put into buffer */

    /* Address of transmit SDMA interrupt cause register */
    dataArray[0] = SMEM_CHT_TX_SDMA_INT_CAUSE_REG(devPtr);
    /* Address of transmit SDMA interrupt mask register */
    dataArray[1] = SMEM_CHT_TX_SDMA_INT_MASK_REG(devPtr);
    /* Receive SDMA Interrupt Cause Register Summery Bit */
    dataArray[2] = SMEM_CHT_TX_SDMA_SUM_INT(devPtr);

    /* Call for common interrupt mask active write function */
    smemChtActiveWriteInterruptsMaskReg(devPtr, address, memSize, memPtr,
                                        (GT_UINTPTR)dataArray, inMemPtr);
}

/**
* @internal smemChtActiveWriteMiscInterruptsMaskReg function
* @endinternal
*
* @brief   the application changed the value of the interrupts mask register.
*         check if there is waiting interrupt for that.
* @param[in] devObjPtr                - device object PTR.
* @param[in] address                  - Address for ASIC memory.
* @param[in] memPtr                   - Pointer to the register's memory in the simulation.
* @param[in] memSize                  - Size of memory to be written
* @param[in] param                    - Registers' specific parameter.
* @param[in] inMemPtr                 - Pointer to the memory to get register's content.
*/
void smemChtActiveWriteMiscInterruptsMaskReg (
    IN         SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN         GT_U32   address,
    IN         GT_U32   memSize,
    IN         GT_U32 * memPtr,
    IN         GT_UINTPTR   param,
    IN         GT_U32 * inMemPtr
)
{
    GT_U32 dataArray[3];                     /* Data array to put into buffer */

    /* Address of miscellaneous interrupt cause register */
    dataArray[0] = (param == 0) ? SMEM_CHT_MISC_INTR_CAUSE_REG(devObjPtr) : SMEM_CHT_MISC_INTR_TREE1_CAUSE_REG(devObjPtr);
    /* Address of miscellaneous  interrupt mask register */
    dataArray[1] = (param == 0) ? SMEM_CHT_MISC_INTR_MASK_REG(devObjPtr) : SMEM_CHT_MISC_INTR_TREE1_MASK_REG(devObjPtr);
    /* Summary bit in global interrupt cause register */
    dataArray[2] = SMEM_CHT_MISCELLANEOUS_SUM_INT(devObjPtr);

    /* Call for common interrupt mask active write function */
    smemChtActiveWriteInterruptsMaskReg(devObjPtr, address, memSize, memPtr,
                                        (GT_UINTPTR)dataArray, inMemPtr);
}


/**
* @internal smemChtActiveWriteMibCntCtrl function
* @endinternal
*
* @brief   Write MIB Counters Control Register for Tri-Speed ports.
*
* @param[in] devObjPtr                - device object PTR.
* @param[in] address                  - Address for ASIC memory.
* @param[in] memSize                  - memory size to be read.
* @param[in] memPtr                   - Pointer to the register's memory in the simulation.
* @param[in] group                    -  of triggered capture port.
* @param[in] inMemPtr                 - Pointer to the memory to set register's content.
*/
void smemChtActiveWriteMibCntCtrl
(
    IN         SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN         GT_U32   address,
    IN         GT_U32   memSize,
    IN         GT_U32 * memPtr,
    IN         GT_UINTPTR group,
    IN         GT_U32 * inMemPtr
)
{
    GT_U32 fieldVal;                /* register field value */
    GT_U32 port;                    /* captured port# modulo 6 */
    GT_U32 regAddr;                 /* register address */
    GT_U32 * regCopyFromPtr;        /* pointer to registers content to copy from */
    GT_U32 * regCopyToPtr;          /* pointer to registers content to copy to */

    /* Update the register value */
    *memPtr = *inMemPtr;

    /* Capture trigger */
    fieldVal = SMEM_U32_GET_FIELD(inMemPtr[0], 0, 1);
    if (fieldVal == 0)
    {
        return;
    }

    /* Capture port */
    fieldVal = SMEM_U32_GET_FIELD(inMemPtr[0], 1, 3);

    /* Convert capture port to real port */
    port = (group * 6) + fieldVal;

    /* MAC MIB Counters Port */
    regAddr = SMEM_CHT_PORT_MAC_COUNT_REG(devObjPtr,port);
    /* Find memory pointer of MAC MIB Counter for port */
    regCopyFromPtr = smemMemGet(devObjPtr, regAddr);

    /* MAC MIB Counters Capture Port  */
    regAddr = SMEM_CHT_CAPTURE_PORT_REG(devObjPtr,port);
    /* Find memory pointer of MAC MIB Counters Capture for port */
    regCopyToPtr = smemMemGet(devObjPtr, regAddr);

    /* Copy captured port MAC MIB counters to the Capture Area (32 counters) */
    memcpy(regCopyToPtr, regCopyFromPtr, (32 * 4));
    /* <DontClearOnRead> defines if counters are cleared when read */
    fieldVal = SMEM_U32_GET_FIELD(inMemPtr[0], 4, 1);
    if(fieldVal == 0)
    {
        /* Counters are cleared */
        memset(regCopyFromPtr, 0, (32 * 4));
    }

    /* Set back <CaptureTrigger> to 0 once the capture action is done */
    SMEM_U32_SET_FIELD(memPtr[0], 0, 1, 0);
}

/**
* @internal smemChtActiveWriteHgsMibCntCtrl function
* @endinternal
*
* @brief   Write MIB Counters Control Register for HyperG.Stack Ports.
*
* @param[in] devObjPtr                - device object PTR.
* @param[in] address                  - Address for ASIC memory.
* @param[in] memSize                  - memory size to be read.
* @param[in] memPtr                   - Pointer to the register's memory in the simulation.
* @param[in] hgsPort                  - triggered capture port.
* @param[in] inMemPtr                 - Pointer to the memory to set register's content.
*/
void smemChtActiveWriteHgsMibCntCtrl
(
    IN         SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN         GT_U32   address,
    IN         GT_U32   memSize,
    IN         GT_U32 * memPtr,
    IN         GT_UINTPTR hgsPort,
    IN         GT_U32 * inMemPtr
)
{
    GT_U32 clearOnCaptureBit[4];    /* the bit that relate to clear/not the counters after the capture */
    GT_U32 actionDoneBit[4];        /* the bits that relate to start/done of action */
    GT_U32 ports[4];                /* triggered capture ports */
    GT_U32 port;                    /* current triggered capture port */
    GT_U32 i;
    GT_U32 regAddr;                 /* register address */
    GT_U32 fldValue;                /* registers field value */
    GT_U32 * regCopyFromPtr;        /* pointer to registers content to copy from */
    GT_U32 * regCopyToPtr;          /* pointer to registers content to copy to */
    GT_U32 numTriggers;             /* number of triggers */

    /* Update the register value */
    *memPtr = *inMemPtr;

    numTriggers = 0;

    if(SKERNEL_IS_CHEETAH3_DEV(devObjPtr))
    {
        /* from ch3 this is 'per port' indication of capture */

        /* get the port number from the address each port with formula of : 0x400*port + offset */
        port = CHT_FROM_ADDRESS_GET_PORT_CNS(devObjPtr,address);
        if(devObjPtr->supportXgMacMibCountersControllOnAllPorts == 0 &&
           (! IS_CHT_HYPER_GIGA_PORT(devObjPtr,port)))
        {
            /* the memory activated but it is not used as 'Active memory'
               because the address is used as register for the 'Stacking Giga'
               ports , and not as address for the XG ports */
            return;
        }

        /* HGS ports capture trigger Cht3,xcat,lion.. */
        ports[numTriggers] = port;
        actionDoneBit[numTriggers] = 0;
        clearOnCaptureBit[numTriggers] = 1;

        numTriggers++;
    }
    else
    {
        /************************************/
        /* HGS ports capture trigger Cht1/2 */
        /************************************/

        /* port 24 support */
        ports[numTriggers] = 24;
        actionDoneBit[numTriggers] = 3;
        clearOnCaptureBit[numTriggers] = 0;
        numTriggers++;

        /* port 25 support */
        ports[numTriggers] = 25;
        actionDoneBit[numTriggers] = 4;
        clearOnCaptureBit[numTriggers] = 1;
        numTriggers++;

        /* port 26 support */
        ports[numTriggers] = 26;
        actionDoneBit[numTriggers] = 5;
        clearOnCaptureBit[numTriggers] = 2;
        numTriggers++;

        if (SKERNEL_IS_CHEETAH2_ONLY_DEV(devObjPtr))
        {
            /* port 27 support */
            ports[numTriggers] = 27;
            actionDoneBit[numTriggers] = 20;
            clearOnCaptureBit[numTriggers] = 19;
            numTriggers++;
        }
    }

    /* Check port capture trigger */
    for (i = 0; i < numTriggers; i++)
    {
        if (0 == SMEM_U32_GET_FIELD(memPtr[0], actionDoneBit[i], 1))
        {
            /* action not required for this port */
            continue;
        }

        port = ports[i];

        /* Hyper.GStack Ports Capture MAC MIB Counters  Cht/Cht2/Cht3 */
        regAddr =
            SMEM_CHT_HGS_PORT_CAPTURE_MAC_MIB_COUNT_REG(devObjPtr, port);
        /* Find memory pointer of MAC MIB Counter for Hyper.GStack port */
        regCopyToPtr =
            smemMemGet(devObjPtr, regAddr);

        /* Hyper.GStack Ports MAC MIB Counters, Cht/Cht2/Cht3 */
        regAddr =
            SMEM_CHT_HGS_PORT_MAC_COUNT_REG(devObjPtr, port);
        /* Find memory pointer of MAC MIB Counters Capture for Hyper.GStack port */
        regCopyFromPtr =
            smemMemGet(devObjPtr, regAddr);

        /* Copy captured port MAC MIB counters to the Capture Area (32 counters) */
        memcpy(regCopyToPtr, regCopyFromPtr, (32 * 4));
        /* <DontClearOnRead> defines if counters are cleared when read */
        fldValue = SMEM_U32_GET_FIELD(memPtr[0], clearOnCaptureBit[i], 1);
        if(fldValue == 0)
        {
            /* Counters are cleared */
            memset(regCopyFromPtr, 0, (32 * 4));
        }

        /* Set back <CaptureTrigger> to 0 once the capture action is done */
        SMEM_U32_SET_FIELD(memPtr[0], actionDoneBit[i], 1, 0);
    }

}


/**
* @internal smemChtActiveWriteSdmaConfigReg function
* @endinternal
*
* @brief   the application changed the value of the SDMA control register.
*
* @param[in] devObjPtr                - device object PTR.
* @param[in] address                  - Address for ASIC memory.
* @param[in] memPtr                   - Pointer to the register's memory in the simulation.
* @param[in] memSize                  - Size of memory to be written
* @param[in] param                    - Registers' specific parameter.
* @param[in] inMemPtr                 - Pointer to the memory to get register's content.
*/
void smemChtActiveWriteSdmaConfigReg (
    IN         SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN         GT_U32   address,
    IN         GT_U32   memSize,
    IN         GT_U32 * memPtr,
    IN         GT_UINTPTR   param,
    IN         GT_U32 * inMemPtr
)
{
    /*get RxByteSwap value */
    devObjPtr->rxByteSwap = SMEM_U32_GET_FIELD((*inMemPtr), 6,1);

    /*get TxByteSwap value */
    devObjPtr->txByteSwap = SMEM_U32_GET_FIELD((*inMemPtr),23,1);

    (*memPtr) = (*inMemPtr);
}

/**
* @internal smemChtActiveWriteTxQuCtrlReg function
* @endinternal
*
* @brief   Set the <UpdateSchedVarTrigger> field in the Transmit Queue Control
*         Register which loads the new profile configuration.
* @param[in] devObjPtr                - device object PTR.
* @param[in] address                  - Address for ASIC memory.
* @param[in] memPtr                   - Pointer to the register's memory in the simulation.
* @param[in] param                    - Registers' specific parameter -
*                                      global interrupt bit number.
* @param[in] inMemPtr                 - Pointer to the memory to get register's content.
*/
void smemChtActiveWriteTxQuCtrlReg
(
    IN         SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN         GT_U32   address,
    IN         GT_U32   memSize,
    IN         GT_U32 * memPtr,
    IN         GT_UINTPTR   param,
    IN         GT_U32 * inMemPtr
)
{
    GT_U32 reg_value;

    /* data to be written */
    reg_value = *inMemPtr;

    /* Clear bit[4] in Transmit Queue Control Register in order to flag that updates were performed */
    reg_value &= ~0x10;

    /* write to simulation memory */
    *memPtr = reg_value;

    return;
}

/**
* @internal smemChtActiveWriteTxQConfigReg function
* @endinternal
*
* @brief   Process Transmit Queue Control Register which loads the new
*         tc configuration (tc enable/disable).
* @param[in] devObjPtr                - device object PTR.
* @param[in] address                  - Address for ASIC memory.
* @param[in] memPtr                   - Pointer to the register's memory in the simulation.
* @param[in] param                    - Registers' specific parameter -
* @param[in] inMemPtr                 - Pointer to the memory to get register's content.
*/
GT_VOID smemChtActiveWriteTxQConfigReg
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32   address,
    IN GT_U32   memSize,
    IN GT_U32 * memPtr,
    IN GT_UINTPTR   param,
    IN GT_U32 * inMemPtr
)
{
    GT_U32  port = 0;

    /* get port num */
    port = (address & 0xFF00) / 0x200;

    smemTxqSendDequeueMessages(devObjPtr, *inMemPtr, port, NUM_OF_TRAFFIC_CLASSES);

    /* Update the register value */
    *memPtr = *inMemPtr;

    return;
}

/**
* @internal smemChtSmiTransaction function
* @endinternal
*
* @brief   Process smi transaction
*/
static GT_VOID smemChtSmiTransaction
(
    IN SKERNEL_DEVICE_OBJECT * deviceObjPtr,
    IN GT_U8                 * dataPtr,
    IN GT_U32                  dataLength
)
{
    GT_U32  data; /* on 'write transaction' : data to write to the 'PHY'
                     on 'read transaction' : data read from the 'PHY' */
    GT_U32  phyRegAddr;/* register address in the PHY */
    GT_BIT  doReadOperation;/* do read or write operation : 0 - write , 1 - read */
    GT_U32  scibDeviceId;/* the scib device id for the PHY */
    GT_U32  address;/* SMI control register address */
    GT_U32  regValue;/* SMI control register value */

    /* parse the data from the buffer of the message */
    memcpy(&address,dataPtr,sizeof(GT_U32));
    dataPtr+=sizeof(GT_U32);
    memcpy(&regValue,dataPtr,sizeof(GT_U32));
    dataPtr+=sizeof(GT_U32);
    memcpy(&scibDeviceId,dataPtr,sizeof(GT_U32));
    dataPtr+=sizeof(GT_U32);

    phyRegAddr = SMEM_U32_GET_FIELD(regValue,21,5);
    doReadOperation = SMEM_U32_GET_FIELD(regValue,26,1);

    if(doReadOperation)
    {
        /* read operation */
        scibReadMemory(scibDeviceId,phyRegAddr,1,&data);
        /*set the data into the register*/
        SMEM_U32_SET_FIELD(regValue,0,16,data);
        /* set ReadValid bit */
        SMEM_U32_SET_FIELD(regValue,27,1,1);
    }
    else
    {
        /* write operation */
        data = SMEM_U32_GET_FIELD(regValue,0,16);
        scibWriteMemory(scibDeviceId,phyRegAddr,1,&data);
    }

    /* clear busy bit */
    SMEM_U32_SET_FIELD(regValue,28,1,0);

    /* update the actual register memory */
    smemRegSet(deviceObjPtr,address,regValue);

}

/**
* @internal smemChtActiveWriteSmi function
* @endinternal
*
* @brief   Write to the SMI0/1 Management Register - start SMI transaction.
*/
void smemChtActiveWriteSmi (
    IN         SKERNEL_DEVICE_OBJECT * deviceObjPtr,
    IN         GT_U32   address,
    IN         GT_U32   memSize,
    IN         GT_U32 * memPtr,
    IN         GT_UINTPTR   param,
    IN         GT_U32 * inMemPtr
)
{
    SBUF_BUF_ID bufferId;       /* buffer */
    GT_U8  * dataPtr;         /* pointer to the data in the buffer */
    GT_U32 dataSize;          /* data size */
    GT_U32  phyAddr;/* phy address */
    GT_BIT  doReadOperation;/* do read or write operation : 0 - write , 1 - read */
    GT_U32  smiScope = 0;/* smi scope : SMI-0 or SMI-1 */
    GT_U32  scibDeviceId;/* the scib device id for the PHY */
    GT_U32   busId;/* bus ID on which the PHY exists*/
    GENERIC_MSG_FUNC genFunc = smemChtSmiTransaction;/* generic function */

    *memPtr = *inMemPtr;
    /* clear ReadValid bit (Read only bit) */
    SMEM_U32_SET_FIELD((*memPtr),27,1,0);
    /* set busy bit (Read only bit) */
    SMEM_U32_SET_FIELD((*memPtr),28,1,1);

    if(SMEM_CHT_IS_SIP5_10_GET(deviceObjPtr))
    {
        /* SMI became separate unit starting from SIP 5_10 */
        GT_CHAR* unitName = smemUnitNameByAddressGet(deviceObjPtr,address);
        if(unitName == NULL)
        {
            skernelFatalError("smemChtActiveWriteSmi : can't find unit SMI control register");
        }
        smiScope = 0;
        if(0 == strcmp(unitName,STR(UNIT_GOP_SMI_0)))
        {
            smiScope = 0;
        }
        else
        if(0 == strcmp(unitName,STR(UNIT_GOP_SMI_1)))
        {
            smiScope = 1;
        }
        else
        if(0 == strcmp(unitName,STR(UNIT_GOP_SMI_2)))
        {
            smiScope = 2;
        }
        else
        if(0 == strcmp(unitName,STR(UNIT_GOP_SMI_3)))
        {
            smiScope = 3;
        }
    }
    else
    if(SKERNEL_IS_XCAT3_BASED_DEV(deviceObjPtr) ||
        SMEM_CHT_IS_SIP5_GET(deviceObjPtr))
    {
        if((address & 0x02000000) == 0x02000000)
        {
            smiScope = 1;
        }
        else
        {
            smiScope = 0;
        }
    }
    else
    {
        /* send message to smain task */
        switch(address & 0xFFFF0000)
        {
            case 0x04000000:
                /* smi-0 */
                smiScope = 0;
                break;
            case 0x05000000:
                /* smi-1 */
                smiScope = 1;
                break;
            default:
                skernelFatalError("smemChtActiveWriteSmi : not supported SMI control register");
        }
    }

    phyAddr    = SMEM_U32_GET_FIELD((*inMemPtr),16,5);
    doReadOperation = SMEM_U32_GET_FIELD((*inMemPtr),26,1);

    busId = (deviceObjPtr->deviceId * SMAIN_BUS_ID_MULTIPLIER_CNS) + smiScope;

    if(GT_FALSE == smainBusDevFind(busId,phyAddr,&scibDeviceId))
    {
        /* do not waste time on transaction to other task */

        /* device not found */
        if(doReadOperation)
        {
            /*read operation -- put 0xFFFF value as 'read data' */
            SMEM_U32_SET_FIELD((*memPtr),0,16,0xFFFF);

            /* set ReadValid bit */
            SMEM_U32_SET_FIELD((*memPtr),27,1,1);
        }
        else
        {
            /* do nothing */
        }

        /* clear busy bit */
        SMEM_U32_SET_FIELD((*memPtr),28,1,0);

        return;
    }
    /* Get buffer      */
    bufferId = sbufAlloc(deviceObjPtr->bufPool, SMI_TRANSACTION_SIZE);
    if (bufferId == NULL)
    {
        simWarningPrintf(" smemChtActiveWriteSmi : no buffers for SMI transaction \n");
        return;
    }

    /* Get actual data pointer */
    sbufDataGet(bufferId, (GT_U8 **)&dataPtr, &dataSize);

    /* put the name of the function into the message */
    memcpy(dataPtr,&genFunc,sizeof(GENERIC_MSG_FUNC));
    dataPtr+=sizeof(GENERIC_MSG_FUNC);

    /* save register address */
    memcpy(dataPtr,&address,sizeof(GT_U32));
    dataPtr+=sizeof(GT_U32);

    /* save register content */
    memcpy(dataPtr,memPtr,sizeof(GT_U32));
    dataPtr+=sizeof(GT_U32);

    /* save SCIB device id of the PHY */
    memcpy(dataPtr,&scibDeviceId,sizeof(GT_U32));
    dataPtr+=sizeof(GT_U32);

    /* set source type of buffer                    */
    bufferId->srcType = SMAIN_SRC_TYPE_CPU_E;

    /* set message type of buffer                   */
    bufferId->dataType = SMAIN_MSG_TYPE_GENERIC_FUNCTION_E;

    /* put buffer to queue                          */
    squeBufPut(deviceObjPtr->queueId, SIM_CAST_BUFF(bufferId));
}

/**
* @internal smemChtTableInfoSet function
* @endinternal
*
* @brief   set the table info for the device --> fill devObjPtr->tablesInfo
*
* @param[in] devObjPtr                - device object PTR.
*/
void smemChtTableInfoSet
(
    IN         SKERNEL_DEVICE_OBJECT * devObjPtr
)
{
    smemChtTableNamesSet(devObjPtr);

    if(SMEM_CHT_IS_SIP6_GET(devObjPtr))
    {
        return;/* no more */
    }

    devObjPtr->globalInterruptCauseRegister.pexInt     = SMAIN_NOT_VALID_CNS;
    devObjPtr->globalInterruptCauseRegister.pexErr     = SMAIN_NOT_VALID_CNS;
    devObjPtr->globalInterruptCauseRegister.cnc[0]     = SMAIN_NOT_VALID_CNS;
    devObjPtr->globalInterruptCauseRegister.cnc[1]     = SMAIN_NOT_VALID_CNS;
    devObjPtr->globalInterruptCauseRegister.txqDq      = SMAIN_NOT_VALID_CNS;
    devObjPtr->globalInterruptCauseRegister.txqSht     = SMAIN_NOT_VALID_CNS;
    devObjPtr->globalInterruptCauseRegister.tti        = SMAIN_NOT_VALID_CNS;
    devObjPtr->globalInterruptCauseRegister.tccLower   = SMAIN_NOT_VALID_CNS;
    devObjPtr->globalInterruptCauseRegister.tccUpper   = SMAIN_NOT_VALID_CNS;
    devObjPtr->globalInterruptCauseRegister.bcn        = SMAIN_NOT_VALID_CNS;
    devObjPtr->globalInterruptCauseRegister.ha         = SMAIN_NOT_VALID_CNS;
    devObjPtr->globalInterruptCauseRegister.misc       = 11;
    devObjPtr->globalInterruptCauseRegister.mem        = 12;
    devObjPtr->globalInterruptCauseRegister.txq        = 13;
    devObjPtr->globalInterruptCauseRegister.txqll      = SMAIN_NOT_VALID_CNS;
    devObjPtr->globalInterruptCauseRegister.txqQueue   = SMAIN_NOT_VALID_CNS;
    devObjPtr->globalInterruptCauseRegister.l2i        = 14;
    devObjPtr->globalInterruptCauseRegister.bm0        = 15;
    devObjPtr->globalInterruptCauseRegister.bm1        = 16;
    devObjPtr->globalInterruptCauseRegister.txqEgr     = SMAIN_NOT_VALID_CNS;
    devObjPtr->globalInterruptCauseRegister.xlgPort    = SMAIN_NOT_VALID_CNS;
    devObjPtr->globalInterruptCauseRegister.fdb        = 17;
    devObjPtr->globalInterruptCauseRegister.ports      = 18;
    devObjPtr->globalInterruptCauseRegister.cpuPort    = 19;
    devObjPtr->globalInterruptCauseRegister.global1    = SMAIN_NOT_VALID_CNS;
    devObjPtr->globalInterruptCauseRegister.txSdma     = 21;
    devObjPtr->globalInterruptCauseRegister.rxSdma     = 22;
    devObjPtr->globalInterruptCauseRegister.pcl        = 23;
    devObjPtr->globalInterruptCauseRegister.port24     = 24;
    devObjPtr->globalInterruptCauseRegister.port25     = 25;
    devObjPtr->globalInterruptCauseRegister.port26     = 26;
    devObjPtr->globalInterruptCauseRegister.port27     = SMAIN_NOT_VALID_CNS;
    devObjPtr->globalInterruptCauseRegister.iplr0      = SMAIN_NOT_VALID_CNS;
    devObjPtr->globalInterruptCauseRegister.iplr1      = SMAIN_NOT_VALID_CNS;
    devObjPtr->globalInterruptCauseRegister.rxDma      = SMAIN_NOT_VALID_CNS;
    devObjPtr->globalInterruptCauseRegister.txDma      = SMAIN_NOT_VALID_CNS;
    devObjPtr->globalInterruptCauseRegister.eq         = 28;
    devObjPtr->globalInterruptCauseRegister.bma        = SMAIN_NOT_VALID_CNS;
    devObjPtr->globalInterruptCauseRegister.eplr       = SMAIN_NOT_VALID_CNS;


    devObjPtr->tablesInfo.stp.commonInfo.baseAddress = 0x0F200000;
    devObjPtr->tablesInfo.stp.paramInfo[0].step = 0x8;


    devObjPtr->tablesInfo.mcast.commonInfo.baseAddress = 0x0F100000;
    devObjPtr->tablesInfo.mcast.paramInfo[0].step = 0x4;

    devObjPtr->tablesInfo.vlan.commonInfo.baseAddress = 0x0F000000;
    devObjPtr->tablesInfo.vlan.paramInfo[0].step = 0xc;

    devObjPtr->tablesInfo.ingrStc.commonInfo.baseAddress = 0x0EC00000;
    devObjPtr->tablesInfo.ingrStc.paramInfo[0].step = 0xc;

    devObjPtr->tablesInfo.egressStc.commonInfo.baseAddress = 0x01D40000;
    devObjPtr->tablesInfo.egressStc.paramInfo[0].step = 0x200;/*port*/

    devObjPtr->tablesInfo.statisticalRateLimit.commonInfo.baseAddress = 0x0EB00000;
    devObjPtr->tablesInfo.statisticalRateLimit.paramInfo[0].step = 0x4;

    devObjPtr->tablesInfo.cpuCode.commonInfo.baseAddress = 0x0EA00000;
    devObjPtr->tablesInfo.cpuCode.paramInfo[0].step = 0x4;

    devObjPtr->tablesInfo.qosProfile.commonInfo.baseAddress = 0x0E900000;
    devObjPtr->tablesInfo.qosProfile.paramInfo[0].step = 0x4;

    devObjPtr->tablesInfo.fdb.commonInfo.baseAddress = 0x0D800000;
    devObjPtr->tablesInfo.fdb.paramInfo[0].step = 0x10;

    devObjPtr->tablesInfo.portVlanQosConfig.commonInfo.baseAddress = 0x0D000000;
    devObjPtr->tablesInfo.portVlanQosConfig.paramInfo[0].step = 0x8;

    devObjPtr->tablesInfo.pclConfig.commonInfo.baseAddress = 0x0D300000;
    devObjPtr->tablesInfo.pclConfig.paramInfo[0].step = 0x4; /*entry*/

    devObjPtr->tablesInfo.pclActionTcamData.commonInfo.baseAddress = 0x0B800100;
    devObjPtr->tablesInfo.pclActionTcamData.paramInfo[0].step = 0x4;

    devObjPtr->tablesInfo.policer.commonInfo.baseAddress = 0x0E000000;
    devObjPtr->tablesInfo.policer.paramInfo[0].step = 0x10;
    devObjPtr->tablesInfo.policer.paramInfo[1].step = 0;/* not supported*/

    devObjPtr->tablesInfo.policerReMarking.commonInfo.baseAddress = 0x0E100000;
    devObjPtr->tablesInfo.policerReMarking.paramInfo[0].step = 0x4;

    devObjPtr->tablesInfo.policerCounters.commonInfo.baseAddress = 0x0E200000;
    devObjPtr->tablesInfo.policerCounters.paramInfo[0].step = 0x10;
    devObjPtr->tablesInfo.policerCounters.paramInfo[1].step = 0;/* not supported*/

    devObjPtr->tablesInfo.arp.commonInfo.baseAddress = 0x0F800000;
    devObjPtr->tablesInfo.arp.paramInfo[0].step = 0x8;

    devObjPtr->tablesInfo.vlanPortMacSa.commonInfo.baseAddress = 0x0F900000;
    devObjPtr->tablesInfo.vlanPortMacSa.paramInfo[0].step = 0x4;


    devObjPtr->tablesInfo.pclAction.commonInfo.baseAddress = 0x0D200000;
    devObjPtr->tablesInfo.pclAction.paramInfo[0].step = 0xc;

    devObjPtr->tablesInfo.trunkNumOfMembers.commonInfo.baseAddress = 0x02000804;
    devObjPtr->tablesInfo.trunkNumOfMembers.paramInfo[0].step = 0x1000;
    devObjPtr->tablesInfo.trunkNumOfMembers.paramInfo[0].divider = 8;

    devObjPtr->tablesInfo.nonTrunkMembers.commonInfo.baseAddress = 0x01A40000;
    devObjPtr->tablesInfo.nonTrunkMembers.paramInfo[0].step = 0x10;

    devObjPtr->tablesInfo.designatedPorts.commonInfo.baseAddress = 0x01A40008;
    devObjPtr->tablesInfo.designatedPorts.paramInfo[0].step = 0x10;

    devObjPtr->tablesInfo.dscpToDscpMap.commonInfo.baseAddress = 0x0B800400;
    devObjPtr->tablesInfo.dscpToDscpMap.paramInfo[0].step = 0x4;
    devObjPtr->tablesInfo.dscpToDscpMap.paramInfo[0].divider = 4;

    devObjPtr->tablesInfo.dscpToQoSProfile.commonInfo.baseAddress = 0x0B800450;
    devObjPtr->tablesInfo.dscpToQoSProfile.paramInfo[0].step = 0x4;
    devObjPtr->tablesInfo.dscpToQoSProfile.paramInfo[0].divider = 4;

    devObjPtr->tablesInfo.upToQoSProfile.commonInfo.baseAddress = 0x0B8004A0;
    devObjPtr->tablesInfo.upToQoSProfile.paramInfo[0].step = 0x4;
    devObjPtr->tablesInfo.upToQoSProfile.paramInfo[0].divider = 4;

    devObjPtr->tablesInfo.expToQoSProfile.commonInfo.baseAddress = 0x0B8004A8;
    devObjPtr->tablesInfo.expToQoSProfile.paramInfo[0].step = 0x4;
    devObjPtr->tablesInfo.expToQoSProfile.paramInfo[0].divider = 4;

    devObjPtr->tablesInfo.trunkMembers.commonInfo.baseAddress = 0x0E800000;
    devObjPtr->tablesInfo.trunkMembers.paramInfo[0].step       = 1 << 16;  /*member*/
    devObjPtr->tablesInfo.trunkMembers.paramInfo[1].step       = 0x4;      /*trunkId*/

    devObjPtr->tablesInfo.deviceMapTable.commonInfo.baseAddress = 0x01A40004;
    devObjPtr->tablesInfo.deviceMapTable.paramInfo[0].step       = 0x10;   /*device*/
    devObjPtr->tablesInfo.deviceMapTable.paramInfo[1].step       = 0;      /*not supported*/

    devObjPtr->tablesInfo.ieeeRsrvMcConfTable.commonInfo.baseAddress = 0x02000800;
    devObjPtr->tablesInfo.ieeeRsrvMcConfTable.paramInfo[0].step = 0x1000;
    devObjPtr->tablesInfo.ieeeRsrvMcConfTable.paramInfo[0].divider = 16;

    devObjPtr->tablesInfo.sst.commonInfo.baseAddress = 0x01A4000C;
    devObjPtr->tablesInfo.sst.paramInfo[0].step = 0x10;


    devObjPtr->tablesInfo.portProtocolVidQoSConf.commonInfo.baseAddress = 0x0D100000;
    devObjPtr->tablesInfo.portProtocolVidQoSConf.paramInfo[0].step = 0x8;
    devObjPtr->tablesInfo.portProtocolVidQoSConf.paramInfo[1].step = 0x10000;
}

/**
* @internal isRegisterIgnored function
* @endinternal
*
* @brief   check if register for default values is ignored
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] unitBaseAddr             - the base address of unit
* @param[in] relativeAddressToCheck   - relative (to start of unit) address to check
*                                      linkListRegistersDefaultsPtr - (pointer to) link list of arrays of registers to ignore.
*
* @retval 1                        - the 'relativeAddressToCheck' is   in the list of 'ignored' addresses
* @retval 0                        - the 'relativeAddressToCheck' is NOT in the list of 'ignored' addresses
*/
static GT_U32 isRegisterIgnored
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32                   unitBaseAddr,
    IN GT_U32                   relativeAddressToCheck,
    IN SMEM_LINK_LIST_REGISTER_DEFAULT_VALUE_STC  *registersDefaultsPtr_ignored
)
{
    GT_U32  ii,jj[3];
    GT_U32  currentAddress;
    GT_U32  startAddress;
    GT_U32  numOfRepetitions[3];
    GT_U32  stepSizeBeteenRepetitions[3];
    SMEM_REGISTER_DEFAULT_VALUE_STC *registersDefaultsArr;
    GT_U32  currentUnitBaseAddr;

    while(registersDefaultsPtr_ignored != NULL)
    {
        registersDefaultsArr = registersDefaultsPtr_ignored->currDefaultValuesArrPtr;

        for(ii = 0 ; registersDefaultsArr[ii].unitNameStrPtr != NULL ; ii++)
        {
            currentUnitBaseAddr = smemUnitBaseAddrByNameGet(devObjPtr,*(registersDefaultsArr[ii].unitNameStrPtr),1);

            if (currentUnitBaseAddr != unitBaseAddr)
            {
                /* no match in the unit base address*/
                continue;
            }

            startAddress = registersDefaultsArr[ii].address;

            if(startAddress == SMAIN_NOT_VALID_CNS) /* indication for range according to step */
            {
                GT_U32  startIndex = registersDefaultsArr[ii].numOfRepetitions;
                GT_U32  endIndex   = registersDefaultsArr[ii].stepSizeBeteenRepetitions;
                GT_U32  rangeSize  = registersDefaultsArr[ii].numOfRepetitions_2;
                GT_U32  step       = registersDefaultsArr[ii].stepSizeBeteenRepetitions_2;
                GT_U32  startAddrRange,endAddrRange;

                startAddress       = registersDefaultsArr[ii].value;

                for (jj[0] = startIndex ; jj[0] <= endIndex ; jj[0]+= step)
                {
                    startAddrRange = startAddress + ( jj[0]   *rangeSize) ;
                    endAddrRange   = startAddress + ((jj[0]+1)*rangeSize) - 4;

                    if(relativeAddressToCheck >= startAddrRange &&
                       relativeAddressToCheck <= endAddrRange)
                    {
                        /* the address match the range */
                        return 1;
                    }
                }
            }
            else
            {
                stepSizeBeteenRepetitions[0] = registersDefaultsArr[ii].stepSizeBeteenRepetitions;
                numOfRepetitions[0] = registersDefaultsArr[ii].numOfRepetitions;

                stepSizeBeteenRepetitions[1] = registersDefaultsArr[ii].stepSizeBeteenRepetitions_2;
                numOfRepetitions[1] = registersDefaultsArr[ii].numOfRepetitions_2;
                if(0 == numOfRepetitions[1])
                {
                    numOfRepetitions[1] = 1;
                    stepSizeBeteenRepetitions[1] = 4;
                }

                stepSizeBeteenRepetitions[2] = registersDefaultsArr[ii].stepSizeBeteenRepetitions_3;
                numOfRepetitions[2] = registersDefaultsArr[ii].numOfRepetitions_3;
                if(0 == numOfRepetitions[2])
                {
                    numOfRepetitions[2] = 1;
                    stepSizeBeteenRepetitions[2] = 4;
                }

                for(jj[0] = 0 ; jj[0] < numOfRepetitions[0]; jj[0]++)
                {
                    for(jj[1] = 0 ; jj[1] < numOfRepetitions[1]; jj[1]++)
                    {
                        for(jj[2] = 0 ; jj[2] < numOfRepetitions[2]; jj[2]++)
                        {
                            currentAddress = startAddress +
                                stepSizeBeteenRepetitions[0] * jj[0] +
                                stepSizeBeteenRepetitions[1] * jj[1] +
                                stepSizeBeteenRepetitions[2] * jj[2] ;

                            if(relativeAddressToCheck == currentAddress)
                            {
                                return 1;
                            }
                        }
                    }
                }
            }
        }

        /* jump to next element in the link list */
        registersDefaultsPtr_ignored = registersDefaultsPtr_ignored->nextPtr;
    }

    return 0;
}
/**
* @internal isUnitDuplicated function
* @endinternal
*
* @brief   check if unit for default values is duplicated (due to duplicated units).
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] unitBaseAddr             - the base address of unit
*
* @param[out] numOfOtherUnitsPtr       - (pointer to) number of other units.
*                                      0 - means not other units
*                                      otherUnitsArr[] - array of other units
*
* @retval GT_FALSE                 - no other units
* @retval GT_TRUE                  - there are other units
*/
static GT_BOOL isUnitDuplicated
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32                   unitBaseAddr,
    OUT GT_U32                  *numOfOtherUnitsPtr,
    OUT GT_U32                  otherUnitsBaseAddr[]
)
{
    GT_U32  ii,jj;
    GT_U32  currentUnitBaseAddr;
    SMEM_UNIT_DUPLICATION_INFO_STC* currentPtr = &devObjPtr->registersDefaultsPtr_unitsDuplications[0];
    GT_CHAR* name_a,*name_b;
    GT_U32  numOfUnits;

    GT_U32 numOtherUnits;
    GT_BOOL isDuplicated;
    GT_U32  totalUnits;

    *numOfOtherUnitsPtr = 0;
    if(currentPtr == NULL)
    {
        /* the device not supports duplications */
        return GT_FALSE;
    }

    totalUnits = 0;

    while(currentPtr->unitNameStr != NULL)
    {
        name_a = currentPtr->unitNameStr;
        numOfUnits = currentPtr->numOfUnits;

        currentPtr++;/* jump to start of the duplicated units*/

        if (smemCheetahDuplicateUnitAllocationCheck(devObjPtr,currentPtr) == GT_FALSE)
        {
            /* unit memory is not duplicated */

            /* jump over all it's duplicated units */
            currentPtr += numOfUnits;
            continue;
        }

        /* check only first unit name/address ... as it represents the duplicated units */
        currentUnitBaseAddr = smemUnitBaseAddrByNameGet(devObjPtr,name_a,1);
        if (currentUnitBaseAddr != unitBaseAddr)
        {
            /* no match in the unit base address*/

            /* jump over all it's duplicated units */
            currentPtr += numOfUnits;

            continue;
        }

        /* we got match , need to return the duplicated units */

        for(ii = 0 ; ii < numOfUnits ; ii++ , currentPtr++)
        {
            name_b = currentPtr->unitNameStr;
            currentUnitBaseAddr = smemUnitBaseAddrByNameGet(devObjPtr,name_b,1);
            otherUnitsBaseAddr[ii+totalUnits] = currentUnitBaseAddr;
        }

        if(ii == 0)
        {
            skernelFatalError("isUnitDuplicated: no numOfUnits stated for [%s] \n" ,
                name_a);
        }

        totalUnits += ii;

        /* continue ... maybe this unit hold another instance in the list ! */

        /* check if my unit hold another instance in the list :
           like RxDma_0 also for Pipe 1 (not only for RxDma_1,RxDma_2).
        */
    }

    (*numOfOtherUnitsPtr) = totalUnits;

    if(0 == (*numOfOtherUnitsPtr))
    {
        return GT_FALSE;
    }

    /* check if one of my duplicated units , is also subject to duplication !
       like RxDma_0 that may be also instance ,2 AND also each of them is
       duplicated to Pipe 1.
    */

    ii = totalUnits;
    for(jj = 0 ; jj < ii ; jj++)
    {
        unitBaseAddr = otherUnitsBaseAddr[jj];

        /* add the extra units at the end of previous units */
        isDuplicated =
            isUnitDuplicated(devObjPtr,unitBaseAddr,&numOtherUnits,
                &otherUnitsBaseAddr[totalUnits]);
        if(isDuplicated == GT_TRUE)
        {
            totalUnits += numOtherUnits;
        }
    }

    (*numOfOtherUnitsPtr) = totalUnits;

    return GT_TRUE;
}

/**
* @internal smemChtRegDbCheck function
* @endinternal
*
* @brief   check the reg DB info for the device --> from SMEM_CHT_MAC_REG_DB_GET(devObjPtr)
*
* @param[in] devObjPtr                - device object PTR.
*/
static void smemChtRegDbCheck
(
    IN         SKERNEL_DEVICE_OBJECT * devObjPtr
)
{
    GT_U32        ii , jj;
    GT_U32       *regsAddrPtr32;
    GT_U32        regsAddrPtr32Size;
    GT_U32       data;/* data read from the register */
    GT_U32      dbIndex;/* index to DB of SMEM_CHT_PP_REGS_ADDR_STC or to SMEM_SIP5_PP_REGS_ADDR_STC */

    if(devObjPtr->supportRegistersDb == 0)
    {
        return;
    }

    if(devObjPtr->portGroupId != 0)
    {
        /* check only on the first port group , to limit the checking */
        return;
    }

    for(dbIndex = 0 ; dbIndex < 3 ; dbIndex ++)
    {
        if(dbIndex == 0)
        {
            /* view the structure of SMEM_CHT_PP_REGS_ADDR_STC as array of GT_U32 fields */
            regsAddrPtr32       = (void*)SMEM_CHT_MAC_REG_DB_GET(devObjPtr);
            regsAddrPtr32Size   = sizeof(SMEM_CHT_PP_REGS_ADDR_STC)/sizeof(GT_U32);
        }
        else
        if(dbIndex == 1)
        {
            regsAddrPtr32       = (void*)SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr);
            regsAddrPtr32Size   = sizeof(SMEM_SIP5_PP_REGS_ADDR_STC)/sizeof(GT_U32);
        }
        else /*2*/
        {
            if(! SMEM_IS_PIPE_FAMILY_GET(devObjPtr))
            {
                continue;
            }

            regsAddrPtr32       = (void*)SMEM_PIPE_MAC_REG_DB_GET(devObjPtr);
            regsAddrPtr32Size   = sizeof(SMEM_PIPE_PP_REGS_ADDR_STC)/sizeof(GT_U32);
        }


        for( ii = 0; ii < regsAddrPtr32Size; ii++ )
        {
            if(regsAddrPtr32[ii] == SMEM_CHT_PP_REGS_UNIT_START_INFO_MAGIC_NUMBER_CNS)
            {
#ifdef TRACE_CHECKED_UNITS
                {
                    SMEM_CHT_PP_REGS_UNIT_START_INFO_STC      *lastUnitStartPtr;

                    lastUnitStartPtr = (SMEM_CHT_PP_REGS_UNIT_START_INFO_STC*)&(regsAddrPtr32[ii]);
                    osPrintf(
                        "Checking RegAddrDB of Unit %s, base address 0x%08X index 0x%04X\n",
                        lastUnitStartPtr->unitNameStr, lastUnitStartPtr->unitNameStr, ii);
                }
#endif /*TRACE_CHECKED_UNITS*/
                ii += skipUnitStartNumReg;
            }

            if(regsAddrPtr32[ii] == SMAIN_NOT_VALID_CNS)
            {
                /* skip not defined registers */
                continue;
            }

            if(GT_FALSE ==
                smemIsDeviceMemoryOwner(devObjPtr,regsAddrPtr32[ii]))
            {
                /* the address not belongs to this device */
                continue;
            }

            /*check duplications*/
            for(jj = 0 ; jj < ii ; jj++)
            {
                if(regsAddrPtr32[jj] == SMEM_CHT_PP_REGS_UNIT_START_INFO_MAGIC_NUMBER_CNS)
                {
                    jj += skipUnitStartNumReg;

                    if(jj >= ii)
                    {
                        break;
                    }
                }

                if(regsAddrPtr32[ii] == regsAddrPtr32[jj])
                {
                    simWarningPrintf("smemChtRegDbCheck : Warning: address [0x%8.8x] is duplicated in 'reg DB' \n",
                        regsAddrPtr32[ii]);
                    break;/*once is enough*/
                }
            }


            if(devObjPtr->registersDefaultsPtr_ignored)
            {
                if(isRegisterIgnored(devObjPtr,
                        regsAddrPtr32[ii] & 0xff800000,/* base unit */
                        regsAddrPtr32[ii] & 0x007fffff,/* relative address */
                        devObjPtr->registersDefaultsPtr_ignored))
                {
                    /* ignore the register */
                    continue;
                }
            }

            /* read the register --> check that memory exists */
            smemRegGet(devObjPtr,
                       regsAddrPtr32[ii],
                       &data);
        }/*ii*/
    }/*dbIndex*/

}

/**
* @internal smemChtRegDbPreInit_part1 function
* @endinternal
*
* @brief   set reg DB info for the device --> from SMEM_CHT_MAC_REG_DB_GET(devObjPtr)
*         all value to SMAIN_NOT_VALID_CNS
* @param[in] devObjPtr                - device object PTR.
*/
void smemChtRegDbPreInit_part1
(
    IN         SKERNEL_DEVICE_OBJECT * devObjPtr
)
{
    GT_U32 i;
    GT_U32 numOfRegisters;
    GT_U32 * curRegAddrPtr;
    SMEM_CHT_PP_REGS_ADDR_STC        * regAddrDbPtr;
    GT_U32  skipUnitStartNumReg;/* number of registers to skip when we encounter start of unit */
    SMEM_SIP5_PP_REGS_ADDR_STC      *regAddrDbSip5Ptr;
    SMEM_PIPE_PP_REGS_ADDR_STC      *regAddrDbPipePtr;
    SMEM_DFX_SERVER_PP_REGS_ADDR_STC    *regAddrExternalDfxPtr;

    skipUnitStartNumReg = sizeof(SMEM_CHT_PP_REGS_UNIT_START_INFO_STC) / sizeof(GT_U32);

    regAddrDbPtr = SMEM_CHT_MAC_REG_DB_GET(devObjPtr);

    numOfRegisters = sizeof(SMEM_CHT_PP_REGS_ADDR_STC) / sizeof(GT_U32);

    curRegAddrPtr = (GT_U32 *)regAddrDbPtr;

    /* Init structure by SMAIN_NOT_VALID_CNS values */
    for(i = 0;  i < numOfRegisters; i++)
    {
        if(curRegAddrPtr[i] == SMEM_CHT_PP_REGS_UNIT_START_INFO_MAGIC_NUMBER_CNS)
        {
            i += skipUnitStartNumReg;
        }

        curRegAddrPtr[i] = SMAIN_NOT_VALID_CNS;
    }
    regAddrDbSip5Ptr = SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr);

    numOfRegisters = sizeof(SMEM_SIP5_PP_REGS_ADDR_STC) / sizeof(GT_U32);

    curRegAddrPtr = (GT_U32 *)regAddrDbSip5Ptr;

    /* Init structure by SMAIN_NOT_VALID_CNS values */
    for(i = 0;  i < numOfRegisters; i++)
    {
        if(curRegAddrPtr[i] == SMEM_CHT_PP_REGS_UNIT_START_INFO_MAGIC_NUMBER_CNS)
        {
            i += skipUnitStartNumReg;
        }

        curRegAddrPtr[i] = SMAIN_NOT_VALID_CNS;
    }
    if(SMEM_IS_PIPE_FAMILY_GET(devObjPtr))
    {
        regAddrDbPipePtr = SMEM_PIPE_MAC_REG_DB_GET(devObjPtr);

        numOfRegisters = sizeof(SMEM_PIPE_PP_REGS_ADDR_STC) / sizeof(GT_U32);

        curRegAddrPtr = (GT_U32 *)regAddrDbPipePtr;

        /* Init structure by SMAIN_NOT_VALID_CNS values */
        for(i = 0;  i < numOfRegisters; i++)
        {
            if(curRegAddrPtr[i] == SMEM_CHT_PP_REGS_UNIT_START_INFO_MAGIC_NUMBER_CNS)
            {
                i += skipUnitStartNumReg;
            }

            curRegAddrPtr[i] = SMAIN_NOT_VALID_CNS;
        }
    }

    regAddrExternalDfxPtr = SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr);

    numOfRegisters = sizeof(*regAddrExternalDfxPtr) / sizeof(GT_U32);

    curRegAddrPtr = (GT_U32 *)regAddrExternalDfxPtr;

    /* Init structure by SMAIN_NOT_VALID_CNS values */
    for(i = 0;  i < numOfRegisters; i++)
    {
        curRegAddrPtr[i] = SMAIN_NOT_VALID_CNS;
    }
}

/**
* @internal smemChtRegDbPreInit_part2 function
* @endinternal
*
* @brief   set reg DB 'base addr' info for the device --> SMAIN_NOT_VALID_CNS
*
* @param[in] devObjPtr                - device object PTR.
*/
void smemChtRegDbPreInit_part2
(
    IN         SKERNEL_DEVICE_OBJECT * devObjPtr
)
{
    SMEM_CHT_PP_REGS_UNIT_START_INFO_SET_MAC(devObjPtr,MG                 ,SMAIN_NOT_VALID_CNS);
    SMEM_CHT_PP_REGS_UNIT_START_INFO_SET_MAC(devObjPtr,egrAndTxqVer0      ,SMAIN_NOT_VALID_CNS);
    SMEM_CHT_PP_REGS_UNIT_START_INFO_SET_MAC(devObjPtr,egrAndTxqVer1.TXQ.TXQ_Queue      ,SMAIN_NOT_VALID_CNS);
    SMEM_CHT_PP_REGS_UNIT_START_INFO_SET_MAC(devObjPtr,egrAndTxqVer1.TXQ.TXQ_DIST      ,SMAIN_NOT_VALID_CNS);
    SMEM_CHT_PP_REGS_UNIT_START_INFO_SET_MAC(devObjPtr,egrAndTxqVer1.TXQ.TXQ_SHT      ,SMAIN_NOT_VALID_CNS);
    SMEM_CHT_PP_REGS_UNIT_START_INFO_SET_MAC(devObjPtr,egrAndTxqVer1.TXQ.TXQ_DQ      ,SMAIN_NOT_VALID_CNS);
    SMEM_CHT_PP_REGS_UNIT_START_INFO_SET_MAC(devObjPtr,egrAndTxqVer1.TXQ.TXQ_LL      ,SMAIN_NOT_VALID_CNS);
    SMEM_CHT_PP_REGS_UNIT_START_INFO_SET_MAC(devObjPtr,egrAndTxqVer1.TXQ.TXQ_EGR[0]      ,SMAIN_NOT_VALID_CNS);
    SMEM_CHT_PP_REGS_UNIT_START_INFO_SET_MAC(devObjPtr,egrAndTxqVer1.TXQ.TXQ_EGR[1]      ,SMAIN_NOT_VALID_CNS);
    SMEM_CHT_PP_REGS_UNIT_START_INFO_SET_MAC(devObjPtr,L2I                ,SMAIN_NOT_VALID_CNS);
    SMEM_CHT_PP_REGS_UNIT_START_INFO_SET_MAC(devObjPtr,ucRouter           ,SMAIN_NOT_VALID_CNS);
    SMEM_CHT_PP_REGS_UNIT_START_INFO_SET_MAC(devObjPtr,IPvX               ,SMAIN_NOT_VALID_CNS);
    SMEM_CHT_PP_REGS_UNIT_START_INFO_SET_MAC(devObjPtr,BM                 ,SMAIN_NOT_VALID_CNS);
    SMEM_CHT_PP_REGS_UNIT_START_INFO_SET_MAC(devObjPtr,LMS                ,SMAIN_NOT_VALID_CNS);
    SMEM_CHT_PP_REGS_UNIT_START_INFO_SET_MAC(devObjPtr,LMS1               ,SMAIN_NOT_VALID_CNS);
    SMEM_CHT_PP_REGS_UNIT_START_INFO_SET_MAC(devObjPtr,FDB                ,SMAIN_NOT_VALID_CNS);
    SMEM_CHT_PP_REGS_UNIT_START_INFO_SET_MAC(devObjPtr,MPPM0              ,SMAIN_NOT_VALID_CNS);
    SMEM_CHT_PP_REGS_UNIT_START_INFO_SET_MAC(devObjPtr,MPPM1              ,SMAIN_NOT_VALID_CNS);
    SMEM_CHT_PP_REGS_UNIT_START_INFO_SET_MAC(devObjPtr,CTU0               ,SMAIN_NOT_VALID_CNS);
    SMEM_CHT_PP_REGS_UNIT_START_INFO_SET_MAC(devObjPtr,CTU1               ,SMAIN_NOT_VALID_CNS);
    SMEM_CHT_PP_REGS_UNIT_START_INFO_SET_MAC(devObjPtr,EPCL               ,SMAIN_NOT_VALID_CNS);
    SMEM_CHT_PP_REGS_UNIT_START_INFO_SET_MAC(devObjPtr,HA                 ,SMAIN_NOT_VALID_CNS);
    SMEM_CHT_PP_REGS_UNIT_START_INFO_SET_MAC(devObjPtr,RXDMA              ,SMAIN_NOT_VALID_CNS);
    SMEM_CHT_PP_REGS_UNIT_START_INFO_SET_MAC(devObjPtr,TXDMA              ,SMAIN_NOT_VALID_CNS);
    SMEM_CHT_PP_REGS_UNIT_START_INFO_SET_MAC(devObjPtr,MEM                ,SMAIN_NOT_VALID_CNS);
    SMEM_CHT_PP_REGS_UNIT_START_INFO_SET_MAC(devObjPtr,centralizedCntrs[0]   ,SMAIN_NOT_VALID_CNS);
    SMEM_CHT_PP_REGS_UNIT_START_INFO_SET_MAC(devObjPtr,centralizedCntrs[1]   ,SMAIN_NOT_VALID_CNS);
    SMEM_CHT_PP_REGS_UNIT_START_INFO_SET_MAC(devObjPtr,uniphySERDES       ,SMAIN_NOT_VALID_CNS);
    SMEM_CHT_PP_REGS_UNIT_START_INFO_SET_MAC(devObjPtr,networkPorts       ,SMAIN_NOT_VALID_CNS);
    SMEM_CHT_PP_REGS_UNIT_START_INFO_SET_MAC(devObjPtr,EQ                 ,SMAIN_NOT_VALID_CNS);
    SMEM_CHT_PP_REGS_UNIT_START_INFO_SET_MAC(devObjPtr,IPCL               ,SMAIN_NOT_VALID_CNS);
    SMEM_CHT_PP_REGS_UNIT_START_INFO_SET_MAC(devObjPtr,PLR[0]             ,SMAIN_NOT_VALID_CNS);
    SMEM_CHT_PP_REGS_UNIT_START_INFO_SET_MAC(devObjPtr,PLR[1]             ,SMAIN_NOT_VALID_CNS);
    SMEM_CHT_PP_REGS_UNIT_START_INFO_SET_MAC(devObjPtr,MLL                ,SMAIN_NOT_VALID_CNS);
    SMEM_CHT_PP_REGS_UNIT_START_INFO_SET_MAC(devObjPtr,TCCLowerIPCL[0]       ,SMAIN_NOT_VALID_CNS);
    SMEM_CHT_PP_REGS_UNIT_START_INFO_SET_MAC(devObjPtr,TCCLowerIPCL[1]       ,SMAIN_NOT_VALID_CNS);
    SMEM_CHT_PP_REGS_UNIT_START_INFO_SET_MAC(devObjPtr,TCCUpperIPvX       ,SMAIN_NOT_VALID_CNS);
    SMEM_CHT_PP_REGS_UNIT_START_INFO_SET_MAC(devObjPtr,TTI                ,SMAIN_NOT_VALID_CNS);
    if( devObjPtr->devMemUnitNameAndIndexPtr == NULL ||/* support xcat3/lion2*/
        SMAIN_NOT_VALID_CNS != UNIT_BASE_ADDR_GET_ALLOW_NON_EXIST_UNIT_MAC(devObjPtr,UNIT_GOP))
    {
        SMEM_CHT_PP_REGS_UNIT_START_INFO_SET_MAC(devObjPtr,GOP.ports          ,SMAIN_NOT_VALID_CNS);
        SMEM_CHT_PP_REGS_UNIT_START_INFO_SET_MAC(devObjPtr,GOP.packGenConfig  ,SMAIN_NOT_VALID_CNS);
    }
    if( devObjPtr->devMemUnitNameAndIndexPtr == NULL ||/* support xcat3/lion2*/
        SMAIN_NOT_VALID_CNS != UNIT_BASE_ADDR_GET_ALLOW_NON_EXIST_UNIT_MAC(devObjPtr,UNIT_SERDES))
    {
        SMEM_CHT_PP_REGS_UNIT_START_INFO_SET_MAC(devObjPtr,GOP.SERDESes       ,SMAIN_NOT_VALID_CNS);
    }
    SMEM_CHT_PP_REGS_UNIT_START_INFO_SET_MAC(devObjPtr,PLR[2]             ,SMAIN_NOT_VALID_CNS);
    SMEM_CHT_PP_REGS_UNIT_START_INFO_SET_MAC(devObjPtr,temp               ,SMAIN_NOT_VALID_CNS);

    /* start the SIP5 units */
    SMEM_CHT_SIP5_PP_REGS_UNIT_START_INFO_SET_MAC(devObjPtr, TTI , SMAIN_NOT_VALID_CNS);
    SMEM_CHT_SIP5_PP_REGS_UNIT_START_INFO_SET_MAC(devObjPtr, rxDMA[0] , SMAIN_NOT_VALID_CNS);
    SMEM_CHT_SIP5_PP_REGS_UNIT_START_INFO_SET_MAC(devObjPtr, rxDMA[1] , SMAIN_NOT_VALID_CNS);
    SMEM_CHT_SIP5_PP_REGS_UNIT_START_INFO_SET_MAC(devObjPtr, GTS[0] , SMAIN_NOT_VALID_CNS);
    SMEM_CHT_SIP5_PP_REGS_UNIT_START_INFO_SET_MAC(devObjPtr, GTS[1] , SMAIN_NOT_VALID_CNS);
    SMEM_CHT_SIP5_PP_REGS_UNIT_START_INFO_SET_MAC(devObjPtr, FDB , SMAIN_NOT_VALID_CNS);
    SMEM_CHT_SIP5_PP_REGS_UNIT_START_INFO_SET_MAC(devObjPtr, L2I , SMAIN_NOT_VALID_CNS);
    SMEM_CHT_SIP5_PP_REGS_UNIT_START_INFO_SET_MAC(devObjPtr, EQ , SMAIN_NOT_VALID_CNS);
    SMEM_CHT_SIP5_PP_REGS_UNIT_START_INFO_SET_MAC(devObjPtr, LPM , SMAIN_NOT_VALID_CNS);
    SMEM_CHT_SIP5_PP_REGS_UNIT_START_INFO_SET_MAC(devObjPtr, TCAM , SMAIN_NOT_VALID_CNS);
    SMEM_CHT_SIP5_PP_REGS_UNIT_START_INFO_SET_MAC(devObjPtr, EGF_eft , SMAIN_NOT_VALID_CNS);
    SMEM_CHT_SIP5_PP_REGS_UNIT_START_INFO_SET_MAC(devObjPtr, EGF_sht , SMAIN_NOT_VALID_CNS);
    SMEM_CHT_SIP5_PP_REGS_UNIT_START_INFO_SET_MAC(devObjPtr, EGF_qag , SMAIN_NOT_VALID_CNS);
    SMEM_CHT_SIP5_PP_REGS_UNIT_START_INFO_SET_MAC(devObjPtr, HA , SMAIN_NOT_VALID_CNS);
    SMEM_CHT_SIP5_PP_REGS_UNIT_START_INFO_SET_MAC(devObjPtr, MLL , SMAIN_NOT_VALID_CNS);
    SMEM_CHT_SIP5_PP_REGS_UNIT_START_INFO_SET_MAC(devObjPtr, PCL , SMAIN_NOT_VALID_CNS);
    SMEM_CHT_SIP5_PP_REGS_UNIT_START_INFO_SET_MAC(devObjPtr, EPCL , SMAIN_NOT_VALID_CNS);
    SMEM_CHT_SIP5_PP_REGS_UNIT_START_INFO_SET_MAC(devObjPtr, OAMUnit[0] , SMAIN_NOT_VALID_CNS);
    SMEM_CHT_SIP5_PP_REGS_UNIT_START_INFO_SET_MAC(devObjPtr, OAMUnit[1] , SMAIN_NOT_VALID_CNS);
    SMEM_CHT_SIP5_PP_REGS_UNIT_START_INFO_SET_MAC(devObjPtr, ERMRK , SMAIN_NOT_VALID_CNS);
    SMEM_CHT_SIP5_PP_REGS_UNIT_START_INFO_SET_MAC(devObjPtr, GOP ,     SMAIN_NOT_VALID_CNS);
    SMEM_CHT_SIP5_PP_REGS_UNIT_START_INFO_SET_MAC(devObjPtr, GOP_PTP ,     SMAIN_NOT_VALID_CNS);
    SMEM_CHT_SIP5_PP_REGS_UNIT_START_INFO_SET_MAC(devObjPtr, TAI ,     SMAIN_NOT_VALID_CNS);
    SMEM_CHT_SIP5_PP_REGS_UNIT_START_INFO_SET_MAC(devObjPtr, IPvX, SMAIN_NOT_VALID_CNS);


    if(SMEM_IS_PIPE_FAMILY_GET(devObjPtr))
    {
        SMEM_PIPE_PP_REGS_UNIT_START_INFO_SET_MAC(devObjPtr, PCP, SMAIN_NOT_VALID_CNS);
        SMEM_PIPE_PP_REGS_UNIT_START_INFO_SET_MAC(devObjPtr, MCFC, SMAIN_NOT_VALID_CNS);
        SMEM_PIPE_PP_REGS_UNIT_START_INFO_SET_MAC(devObjPtr, PHA, SMAIN_NOT_VALID_CNS);
    }

}


/**
* @internal smemChtTablesInfoCheck function
* @endinternal
*
* @brief   check the table info for the device --> from devObjPtr->tablesInfo
*
* @param[in] devObjPtr                - device object PTR.
*/
static void smemChtTablesInfoCheck
(
    IN         SKERNEL_DEVICE_OBJECT * devObjPtr
)
{
    SKERNEL_TABLE_INFO_STC  *tablesPtr;/* pointer to tables with single parameter */
    GT_U32  numTables;/* number of tables with single parameter */
    SKERNEL_TABLE_2_PARAMETERS_INFO_STC  *tables2ParamsPtr; /* pointer to tables with 2 parameters */
    GT_U32  numTables2Params;/* number of tables with 2 parameters */
    SKERNEL_TABLE_3_PARAMETERS_INFO_STC  *tables3ParamsPtr; /* pointer to tables with 3 parameters */
    GT_U32  numTables3Params;/* number of tables with 3 parameters */
    SKERNEL_TABLE_4_PARAMETERS_INFO_STC  *tables4ParamsPtr; /* pointer to tables with 4 parameters */
    GT_U32  numTables4Params;/* number of tables with 4 parameters */
    GT_U32  ii;
    GT_U32  error = 0;
    GT_U32  localError;

    smemGenericTablesSectionsGet(devObjPtr,
                            &tablesPtr,&numTables,
                            &tables2ParamsPtr,&numTables2Params,
                            &tables3ParamsPtr,&numTables3Params,
                            &tables4ParamsPtr,&numTables4Params);

    /* loop on tables with single parameter */
    for(ii = 0 ;ii < numTables ;ii++,tablesPtr++)
    {
        localError = 0;

        if(tablesPtr->commonInfo.baseAddress)
        {
            if(tablesPtr->commonInfo.nameString == NULL)
            {
                localError = 1;
            }
            if(tablesPtr->paramInfo[0].step == 0)
            {
                localError = 1;
            }
            else if(tablesPtr->paramInfo[0].step & 0x3)
            {
                localError = 1;
            }
        }

        error |=  localError;

        if(localError)
        {
            simWarningPrintf("--ERROR: %s \t 0x%8.8x     0x%4.4x      %d \n"
                    ,TABLE_NAME_MAC(tablesPtr->commonInfo.nameString)
                    ,tablesPtr->commonInfo.baseAddress
                    ,tablesPtr->paramInfo[0].step
                    ,tablesPtr->paramInfo[0].divider);
        }
    }

    for(ii = 0 ;ii < numTables2Params ;ii++,tables2ParamsPtr++)
    {
        localError = 0;

        if(tables2ParamsPtr->commonInfo.baseAddress)
        {
            if(tables2ParamsPtr->commonInfo.nameString == NULL)
            {
                localError = 1;
            }

            if(tables2ParamsPtr->paramInfo[0].step == 0)
            {
                localError = 1;
            }
            else if(tables2ParamsPtr->paramInfo[0].step & 0x3)
            {
                localError = 1;
            }

            if(tables2ParamsPtr->paramInfo[1].step == SMAIN_NOT_VALID_CNS)
            {
                /* set with this value on purpose : to check fatal error on
                   runtime only if tring to access the 'second paramInfo[0].step' */
                /* no error */
            }
            else
        /* allow paramInfo[0].step 2 to be 0 , because we support devices with less dimensions
           if(tables2ParamsPtr->paramInfo[1].step == 0)
            {
                localError = 1;
            }
            else */ if(tables2ParamsPtr->paramInfo[1].step & 0x3)
            {
                localError = 1;
            }
        }

        error |=  localError;

        if(localError)
        {
            simWarningPrintf("--ERROR: %s \t 0x%8.8x     0x%4.4x    %d    0x%4.4x    %d \n"
                    ,TABLE_NAME_MAC(tables2ParamsPtr->commonInfo.nameString)
                    ,tables2ParamsPtr->commonInfo.baseAddress
                    ,tables2ParamsPtr->paramInfo[0].step
                    ,tables2ParamsPtr->paramInfo[0].divider
                    ,tables2ParamsPtr->paramInfo[1].step
                    ,tables2ParamsPtr->paramInfo[1].divider);
        }
    }

    for(ii = 0 ;ii < numTables3Params ;ii++,tables3ParamsPtr++)
    {
        localError = 0;

        if(tables3ParamsPtr->commonInfo.baseAddress)
        {
            if(tables3ParamsPtr->commonInfo.nameString == NULL)
            {
                localError = 1;
            }

            if(tables3ParamsPtr->paramInfo[0].step == 0)
            {
                localError = 1;
            }
            else if(tables3ParamsPtr->paramInfo[0].step & 0x3)
            {
                localError = 1;
            }

            if(tables3ParamsPtr->paramInfo[1].step == SMAIN_NOT_VALID_CNS ||
               tables3ParamsPtr->paramInfo[2].step == SMAIN_NOT_VALID_CNS )
            {
                /* set with this value on purpose : to check fatal error on
                   runtime only if tring to access the 'second paramInfo[0].step' */
                /* no error */
            }
            else
        /* allow paramInfo[0].step 2 to be 0 , because we support devices with less dimensions
           if(tables3ParamsPtr->paramInfo[1].step == 0)
            {
                localError = 1;
            }
            else */ if((tables3ParamsPtr->paramInfo[1].step & 0x3) ||
                       (tables3ParamsPtr->paramInfo[2].step & 0x3) )
            {
                localError = 1;
            }
        }

        error |=  localError;

        if(localError)
        {
            simWarningPrintf("--ERROR: %s \t 0x%8.8x     "
                             "0x%4.4x    %d   %d  "
                             "0x%4.4x    %d   %d  "
                             "0x%4.4x    %d   %d"
                             "\n"
                    ,TABLE_NAME_MAC(tables3ParamsPtr->commonInfo.nameString)
                    ,tables3ParamsPtr->commonInfo.baseAddress
                    ,tables3ParamsPtr->paramInfo[0].step
                    ,tables3ParamsPtr->paramInfo[0].divider
                    ,tables3ParamsPtr->paramInfo[0].modulo
                    ,tables3ParamsPtr->paramInfo[1].step
                    ,tables3ParamsPtr->paramInfo[1].divider
                    ,tables3ParamsPtr->paramInfo[1].modulo
                    ,tables3ParamsPtr->paramInfo[2].step
                    ,tables3ParamsPtr->paramInfo[2].divider
                    ,tables3ParamsPtr->paramInfo[2].modulo
                    );
        }
    }

    for(ii = 0 ;ii < numTables4Params ;ii++,tables4ParamsPtr++)
    {
        localError = 0;

        if(tables4ParamsPtr->commonInfo.baseAddress)
        {
            if(tables4ParamsPtr->commonInfo.nameString == NULL)
            {
                localError = 1;
            }

            if(tables4ParamsPtr->paramInfo[0].step == 0)
            {
                localError = 1;
            }
            else if(tables4ParamsPtr->paramInfo[0].step & 0x3)
            {
                localError = 1;
            }

            if(tables4ParamsPtr->paramInfo[1].step == SMAIN_NOT_VALID_CNS ||
               tables4ParamsPtr->paramInfo[2].step == SMAIN_NOT_VALID_CNS )
            {
                /* set with this value on purpose : to check fatal error on
                   runtime only if tring to access the 'second paramInfo[0].step' */
                /* no error */
            }
            else
        /* allow paramInfo[0].step 2 to be 0 , because we support devices with less dimensions
           if(tables4ParamsPtr->paramInfo[1].step == 0)
            {
                localError = 1;
            }
            else */ if((tables4ParamsPtr->paramInfo[1].step & 0x3) ||
                       (tables4ParamsPtr->paramInfo[2].step & 0x3) )
            {
                localError = 1;
            }
        }

        error |=  localError;

        if(localError)
        {
            simWarningPrintf("--ERROR: %s \t 0x%8.8x     "
                             "0x%4.4x    %d   %d  "
                             "0x%4.4x    %d   %d  "
                             "0x%4.4x    %d   %d  "
                             "0x%4.4x    %d   %d"
                             "\n"
                    ,TABLE_NAME_MAC(tables4ParamsPtr->commonInfo.nameString)
                    ,tables4ParamsPtr->commonInfo.baseAddress
                    ,tables4ParamsPtr->paramInfo[0].step
                    ,tables4ParamsPtr->paramInfo[0].divider
                    ,tables4ParamsPtr->paramInfo[0].modulo
                    ,tables4ParamsPtr->paramInfo[1].step
                    ,tables4ParamsPtr->paramInfo[1].divider
                    ,tables4ParamsPtr->paramInfo[1].modulo
                    ,tables4ParamsPtr->paramInfo[2].step
                    ,tables4ParamsPtr->paramInfo[2].divider
                    ,tables4ParamsPtr->paramInfo[2].modulo
                    ,tables4ParamsPtr->paramInfo[3].step
                    ,tables4ParamsPtr->paramInfo[3].divider
                    ,tables4ParamsPtr->paramInfo[3].modulo
                    );
        }
    }

    if(error)
    {
        skernelFatalError("smemChtTablesInfoCheck: error in tables info above \n");
    }

    /* check that All the addresses of the base address are valid !!
       try to access them by 'read' operation */
    genericTablesTestValidity(devObjPtr->deviceId);
}

/**
* @internal smemChtTableNamesSet function
* @endinternal
*
* @brief   set the names in the table info for the device --> fill devObjPtr->tablesInfo.xxx.commonInfo.nameString
*
* @param[in] devObjPtr                - device object PTR.
*/
void smemChtTableNamesSet
(
    IN         SKERNEL_DEVICE_OBJECT * devObjPtr
)
{
    /* table --> reference to type , one of:
        SKERNEL_TABLE_INFO_STC or
        SKERNEL_TABLE_2_PARAMETERS_INFO_STC
    */
    #define TABLES_NAME_TO_STRING_CONVERT_MAC(table)    \
        devObjPtr->tablesInfo.table.commonInfo.nameString = #table

    /* tables with single parameter */
    TABLES_NAME_TO_STRING_CONVERT_MAC(stp);
    TABLES_NAME_TO_STRING_CONVERT_MAC(mcast);
    TABLES_NAME_TO_STRING_CONVERT_MAC(vlan);
    TABLES_NAME_TO_STRING_CONVERT_MAC(ingrStc);
    TABLES_NAME_TO_STRING_CONVERT_MAC(egressStc);
    TABLES_NAME_TO_STRING_CONVERT_MAC(statisticalRateLimit);
    TABLES_NAME_TO_STRING_CONVERT_MAC(cpuCode);
    TABLES_NAME_TO_STRING_CONVERT_MAC(qosProfile);
    TABLES_NAME_TO_STRING_CONVERT_MAC(fdb);
    TABLES_NAME_TO_STRING_CONVERT_MAC(portVlanQosConfig);
    TABLES_NAME_TO_STRING_CONVERT_MAC(pclActionTcamData);
    TABLES_NAME_TO_STRING_CONVERT_MAC(arp);
    TABLES_NAME_TO_STRING_CONVERT_MAC(vlanPortMacSa);
    TABLES_NAME_TO_STRING_CONVERT_MAC(pclAction);
    TABLES_NAME_TO_STRING_CONVERT_MAC(ipProtCpuCode);
    TABLES_NAME_TO_STRING_CONVERT_MAC(tunnelStart);
    TABLES_NAME_TO_STRING_CONVERT_MAC(tunnelStartGenericIpProfile);
    TABLES_NAME_TO_STRING_CONVERT_MAC(vrfId);
    TABLES_NAME_TO_STRING_CONVERT_MAC(ingressVlanTranslation);
    TABLES_NAME_TO_STRING_CONVERT_MAC(egressVlanTranslation);
    TABLES_NAME_TO_STRING_CONVERT_MAC(macToMe);
    TABLES_NAME_TO_STRING_CONVERT_MAC(mll);
    TABLES_NAME_TO_STRING_CONVERT_MAC(mllOutInterfaceConfig);
    TABLES_NAME_TO_STRING_CONVERT_MAC(mllOutInterfaceCounter);
    TABLES_NAME_TO_STRING_CONVERT_MAC(trunkNumOfMembers);
    TABLES_NAME_TO_STRING_CONVERT_MAC(nonTrunkMembers);
    TABLES_NAME_TO_STRING_CONVERT_MAC(designatedPorts);
    TABLES_NAME_TO_STRING_CONVERT_MAC(dscpToDscpMap);
    TABLES_NAME_TO_STRING_CONVERT_MAC(dscpToQoSProfile);
    TABLES_NAME_TO_STRING_CONVERT_MAC(expToQoSProfile);
    TABLES_NAME_TO_STRING_CONVERT_MAC(cfiUpQoSProfileSelect);
    TABLES_NAME_TO_STRING_CONVERT_MAC(ieeeTblSelect);
    TABLES_NAME_TO_STRING_CONVERT_MAC(ieeeRsrvMcCpuIndex);
    TABLES_NAME_TO_STRING_CONVERT_MAC(routeNextHopAgeBits);
    TABLES_NAME_TO_STRING_CONVERT_MAC(xgPortMibCounters);
    TABLES_NAME_TO_STRING_CONVERT_MAC(xgPortMibCounters_1);

    TABLES_NAME_TO_STRING_CONVERT_MAC(egressPolicerMeters);
    TABLES_NAME_TO_STRING_CONVERT_MAC(egressPolicerCounters);
    TABLES_NAME_TO_STRING_CONVERT_MAC(egressPolicerConfig);
    TABLES_NAME_TO_STRING_CONVERT_MAC(policerTblAccessData);
    TABLES_NAME_TO_STRING_CONVERT_MAC(policerMeterPointer);
    TABLES_NAME_TO_STRING_CONVERT_MAC(policerIpfixWaAlert);
    TABLES_NAME_TO_STRING_CONVERT_MAC(policerIpfixAgingAlert);
    TABLES_NAME_TO_STRING_CONVERT_MAC(policerTimer);
    TABLES_NAME_TO_STRING_CONVERT_MAC(policerReMarking);
    TABLES_NAME_TO_STRING_CONVERT_MAC(policerManagementCounters);
    TABLES_NAME_TO_STRING_CONVERT_MAC(policerHierarchicalQos);
    TABLES_NAME_TO_STRING_CONVERT_MAC(policerQosAttributes);

    TABLES_NAME_TO_STRING_CONVERT_MAC(egressVlan);
    TABLES_NAME_TO_STRING_CONVERT_MAC(egressStp);
    TABLES_NAME_TO_STRING_CONVERT_MAC(l2PortIsolation);
    TABLES_NAME_TO_STRING_CONVERT_MAC(l3PortIsolation);
    TABLES_NAME_TO_STRING_CONVERT_MAC(sst);
    TABLES_NAME_TO_STRING_CONVERT_MAC(secondTargetPort);

    TABLES_NAME_TO_STRING_CONVERT_MAC(ipclUserDefinedBytesConf);
    TABLES_NAME_TO_STRING_CONVERT_MAC(ttiUserDefinedBytesConf);
    TABLES_NAME_TO_STRING_CONVERT_MAC(pearsonHash);
    TABLES_NAME_TO_STRING_CONVERT_MAC(crcHashMask);
    TABLES_NAME_TO_STRING_CONVERT_MAC(crcHashMode);
    TABLES_NAME_TO_STRING_CONVERT_MAC(tcamBistArea);
    TABLES_NAME_TO_STRING_CONVERT_MAC(tcamArrayCompareEn);
    TABLES_NAME_TO_STRING_CONVERT_MAC(haUp0PortKeepVlan1);
    TABLES_NAME_TO_STRING_CONVERT_MAC(ipcl0SourcePortConfig);
    TABLES_NAME_TO_STRING_CONVERT_MAC(ipcl1SourcePortConfig);
    TABLES_NAME_TO_STRING_CONVERT_MAC(ipcl2SourcePortConfig);

    TABLES_NAME_TO_STRING_CONVERT_MAC(ttiPhysicalPortAttribute);
    TABLES_NAME_TO_STRING_CONVERT_MAC(ttiPhysicalPort2Attribute);
    TABLES_NAME_TO_STRING_CONVERT_MAC(ttiQcnToPauseTimerMap);
    TABLES_NAME_TO_STRING_CONVERT_MAC(ttiPreTtiLookupIngressEPort);
    TABLES_NAME_TO_STRING_CONVERT_MAC(ttiPostTtiLookupIngressEPort);
    TABLES_NAME_TO_STRING_CONVERT_MAC(ttiTrillAdjacencyTcam);
    TABLES_NAME_TO_STRING_CONVERT_MAC(ttiTrillRbid);
    TABLES_NAME_TO_STRING_CONVERT_MAC(ttiPort2QueueTranslation);
    TABLES_NAME_TO_STRING_CONVERT_MAC(bridgeIngressEPort);
    TABLES_NAME_TO_STRING_CONVERT_MAC(bridgeIngressEPortLearnPrio);
    TABLES_NAME_TO_STRING_CONVERT_MAC(bridgeIngressTrunk);
    TABLES_NAME_TO_STRING_CONVERT_MAC(bridgePhysicalPortEntry);
    TABLES_NAME_TO_STRING_CONVERT_MAC(bridgePhysicalPortRateLimitCountersEntry);
    TABLES_NAME_TO_STRING_CONVERT_MAC(bridgeIngressPortMembership);
    TABLES_NAME_TO_STRING_CONVERT_MAC(ingressSpanStateGroupIndex);

    TABLES_NAME_TO_STRING_CONVERT_MAC(egressAndTxqIngressEcid);

    /* IPvX  tables*/
    TABLES_NAME_TO_STRING_CONVERT_MAC(ipvxIngressEPort);
    TABLES_NAME_TO_STRING_CONVERT_MAC(ipvxIngressEVlan);
    TABLES_NAME_TO_STRING_CONVERT_MAC(ipvxQoSProfileOffsets);
    TABLES_NAME_TO_STRING_CONVERT_MAC(ipvxAccessMatrix);
    TABLES_NAME_TO_STRING_CONVERT_MAC(ipvxNextHop);
    TABLES_NAME_TO_STRING_CONVERT_MAC(ipvxEcmpPointer);
    TABLES_NAME_TO_STRING_CONVERT_MAC(ipvxEcmp);
    TABLES_NAME_TO_STRING_CONVERT_MAC(l2MllLtt);
    TABLES_NAME_TO_STRING_CONVERT_MAC(eqIngressEPort);
    TABLES_NAME_TO_STRING_CONVERT_MAC(eqL2EcmpLtt);
    TABLES_NAME_TO_STRING_CONVERT_MAC(pathUtilization0);
    TABLES_NAME_TO_STRING_CONVERT_MAC(pathUtilization1);
    TABLES_NAME_TO_STRING_CONVERT_MAC(pathUtilization2);
    TABLES_NAME_TO_STRING_CONVERT_MAC(pathUtilization3);
    TABLES_NAME_TO_STRING_CONVERT_MAC(sourcePortHash);
    TABLES_NAME_TO_STRING_CONVERT_MAC(eqTrunkLtt);
    TABLES_NAME_TO_STRING_CONVERT_MAC(eqL2Ecmp);
    TABLES_NAME_TO_STRING_CONVERT_MAC(eqE2Phy);
    TABLES_NAME_TO_STRING_CONVERT_MAC(txqEgressEPort);
    TABLES_NAME_TO_STRING_CONVERT_MAC(txqDistributorDeviceMapTable);
    TABLES_NAME_TO_STRING_CONVERT_MAC(haEgressEPortAttr1);
    TABLES_NAME_TO_STRING_CONVERT_MAC(haEgressEPortAttr2);
    TABLES_NAME_TO_STRING_CONVERT_MAC(haEgressPhyPort1);
    TABLES_NAME_TO_STRING_CONVERT_MAC(haEgressPhyPort2);
    TABLES_NAME_TO_STRING_CONVERT_MAC(haGlobalMacSa);
    TABLES_NAME_TO_STRING_CONVERT_MAC(haQosProfileToExp);
    TABLES_NAME_TO_STRING_CONVERT_MAC(haEpclUserDefinedBytesConfig);
    TABLES_NAME_TO_STRING_CONVERT_MAC(policerEPortEVlanTrigger);
    TABLES_NAME_TO_STRING_CONVERT_MAC(policerIpfix1StNPackets);
    TABLES_NAME_TO_STRING_CONVERT_MAC(l2MllVirtualPortToMllMapping);
    TABLES_NAME_TO_STRING_CONVERT_MAC(l2MllVidxToMllMapping);
    TABLES_NAME_TO_STRING_CONVERT_MAC(eqLogicalPortEgressVlanMember);
    TABLES_NAME_TO_STRING_CONVERT_MAC(eqVlanMapping);
    TABLES_NAME_TO_STRING_CONVERT_MAC(eqPhysicalPortIngressMirrorIndexTable);
    TABLES_NAME_TO_STRING_CONVERT_MAC(egfQagTargetPortMapper);
    TABLES_NAME_TO_STRING_CONVERT_MAC(egfQagEVlanDescriptorAssignmentAttributes);
    TABLES_NAME_TO_STRING_CONVERT_MAC(egfShtVidMapper);
    TABLES_NAME_TO_STRING_CONVERT_MAC(egfShtEportEVlanFilter);
    TABLES_NAME_TO_STRING_CONVERT_MAC(egfShtEgressEPort);
    TABLES_NAME_TO_STRING_CONVERT_MAC(egfQagEgressEPort);
    TABLES_NAME_TO_STRING_CONVERT_MAC(egfShtEVlanAttribute);
    TABLES_NAME_TO_STRING_CONVERT_MAC(egfShtEVlanSpanning);
    TABLES_NAME_TO_STRING_CONVERT_MAC(nonTrunkMembers2);
    TABLES_NAME_TO_STRING_CONVERT_MAC(egfQagPortTargetAttribute);
    TABLES_NAME_TO_STRING_CONVERT_MAC(egfQagPortSourceAttribute);
    TABLES_NAME_TO_STRING_CONVERT_MAC(egfQagTcDpMapper);
    TABLES_NAME_TO_STRING_CONVERT_MAC(egfQagCpuCodeToLbMapper);
    TABLES_NAME_TO_STRING_CONVERT_MAC(egfQagFwdFromCpuToLbMapper);
    TABLES_NAME_TO_STRING_CONVERT_MAC(egfQagVlanQOffsetMappingTable);

    /* OAM Tables */
    TABLES_NAME_TO_STRING_CONVERT_MAC(oamTable);
    TABLES_NAME_TO_STRING_CONVERT_MAC(oamAgingTable);
    TABLES_NAME_TO_STRING_CONVERT_MAC(oamMegExceptionTable);
    TABLES_NAME_TO_STRING_CONVERT_MAC(oamSrcInterfaceExceptionTable);
    TABLES_NAME_TO_STRING_CONVERT_MAC(oamInvalidKeepAliveHashTable);
    TABLES_NAME_TO_STRING_CONVERT_MAC(oamExcessKeepAliveTable);
    TABLES_NAME_TO_STRING_CONVERT_MAC(oamRdiStatusChangeExceptionTable);
    TABLES_NAME_TO_STRING_CONVERT_MAC(oamExceptionSummaryTable);
    TABLES_NAME_TO_STRING_CONVERT_MAC(oamOpCodePacketCommandTable);
    TABLES_NAME_TO_STRING_CONVERT_MAC(oamLmOffsetTable);
    TABLES_NAME_TO_STRING_CONVERT_MAC(oamTimeStampOffsetTable);
    TABLES_NAME_TO_STRING_CONVERT_MAC(oamTxPeriodExceptionTable);
    TABLES_NAME_TO_STRING_CONVERT_MAC(oamProtectionLocStatusTable);
    TABLES_NAME_TO_STRING_CONVERT_MAC(oamTxProtectionLocStatusTable);

    TABLES_NAME_TO_STRING_CONVERT_MAC(ePortToLocMappingTable);
    TABLES_NAME_TO_STRING_CONVERT_MAC(txProtectionSwitchingTable);

    TABLES_NAME_TO_STRING_CONVERT_MAC(ptpPacketCommandTable);
    TABLES_NAME_TO_STRING_CONVERT_MAC(ptpTargetPortTable);
    TABLES_NAME_TO_STRING_CONVERT_MAC(ptpSourcePortTable);

    TABLES_NAME_TO_STRING_CONVERT_MAC(ERMRKQosMapTable);

    /* LPM Tables */
    TABLES_NAME_TO_STRING_CONVERT_MAC(lpmIpv4VrfId);
    TABLES_NAME_TO_STRING_CONVERT_MAC(lpmIpv6VrfId);
    TABLES_NAME_TO_STRING_CONVERT_MAC(lpmFcoeVrfId);
    TABLES_NAME_TO_STRING_CONVERT_MAC(lpmEcmp);
    TABLES_NAME_TO_STRING_CONVERT_MAC(lpmAgingMemory);

    TABLES_NAME_TO_STRING_CONVERT_MAC(tcamMemory);
    TABLES_NAME_TO_STRING_CONVERT_MAC(globalActionTable);

    TABLES_NAME_TO_STRING_CONVERT_MAC(epclConfigTable);
    TABLES_NAME_TO_STRING_CONVERT_MAC(ipcl0UdbSelect);
    TABLES_NAME_TO_STRING_CONVERT_MAC(ipcl1UdbSelect);
    TABLES_NAME_TO_STRING_CONVERT_MAC(ipcl2UdbSelect);
    TABLES_NAME_TO_STRING_CONVERT_MAC(ipcl0UdbReplacement);
    TABLES_NAME_TO_STRING_CONVERT_MAC(ipcl1UdbReplacement);
    TABLES_NAME_TO_STRING_CONVERT_MAC(ipcl2UdbReplacement);
    TABLES_NAME_TO_STRING_CONVERT_MAC(epclUdbReplacement);
    TABLES_NAME_TO_STRING_CONVERT_MAC(epclUdbSelect);
    TABLES_NAME_TO_STRING_CONVERT_MAC(epclExactMatchProfileIdMapping);
    TABLES_NAME_TO_STRING_CONVERT_MAC(epclPortLatencyMonitoring);
    TABLES_NAME_TO_STRING_CONVERT_MAC(epclSourcePhysicalPortMapping);
    TABLES_NAME_TO_STRING_CONVERT_MAC(epclTargetPhysicalPortMapping);

    TABLES_NAME_TO_STRING_CONVERT_MAC(Shared_Queue_Maximum_Queue_Limits);
    TABLES_NAME_TO_STRING_CONVERT_MAC(Queue_Limits_DP0_Enqueue);
    TABLES_NAME_TO_STRING_CONVERT_MAC(Queue_Buffer_Limits_Dequeue);
    TABLES_NAME_TO_STRING_CONVERT_MAC(Queue_Descriptor_Limits_Dequeue);
    TABLES_NAME_TO_STRING_CONVERT_MAC(Queue_Limits_DP12_Enqueue);
    TABLES_NAME_TO_STRING_CONVERT_MAC(FC_Mode_Profile_TC_XOff_Thresholds);
    TABLES_NAME_TO_STRING_CONVERT_MAC(CN_Sample_Intervals);
    TABLES_NAME_TO_STRING_CONVERT_MAC(Scheduler_State_Variable);

    TABLES_NAME_TO_STRING_CONVERT_MAC(preqQueuePortMapping);
    TABLES_NAME_TO_STRING_CONVERT_MAC(preqProfiles);
    TABLES_NAME_TO_STRING_CONVERT_MAC(preqQueueConfiguration);
    TABLES_NAME_TO_STRING_CONVERT_MAC(preqPortProfile);
    TABLES_NAME_TO_STRING_CONVERT_MAC(preqTargetPhyPort);
    TABLES_NAME_TO_STRING_CONVERT_MAC(preqSrfMapping);
    TABLES_NAME_TO_STRING_CONVERT_MAC(preqSrfConfig);
    TABLES_NAME_TO_STRING_CONVERT_MAC(preqDaemons);
    TABLES_NAME_TO_STRING_CONVERT_MAC(preqHistoryBuffer);
    TABLES_NAME_TO_STRING_CONVERT_MAC(preqSrfCounters);
    TABLES_NAME_TO_STRING_CONVERT_MAC(preqZeroBitVector0);
    TABLES_NAME_TO_STRING_CONVERT_MAC(preqZeroBitVector1);

    TABLES_NAME_TO_STRING_CONVERT_MAC(efuseFeaturesDisableBypass);
    TABLES_NAME_TO_STRING_CONVERT_MAC(efuseDevIdBypass);
    TABLES_NAME_TO_STRING_CONVERT_MAC(idEfuseSlave);
    TABLES_NAME_TO_STRING_CONVERT_MAC(hdEfuseSlave);
    TABLES_NAME_TO_STRING_CONVERT_MAC(debugUnit);

    TABLES_NAME_TO_STRING_CONVERT_MAC(tmDropQueueProfileId);
    TABLES_NAME_TO_STRING_CONVERT_MAC(tmDropDropMasking);

    TABLES_NAME_TO_STRING_CONVERT_MAC(tmEgressGlueTargetInterface);

    TABLES_NAME_TO_STRING_CONVERT_MAC(bmaPortMapping);

    TABLES_NAME_TO_STRING_CONVERT_MAC(PHA_PPAThreadsConf1);
    TABLES_NAME_TO_STRING_CONVERT_MAC(PHA_PPAThreadsConf2);
    TABLES_NAME_TO_STRING_CONVERT_MAC(PHA_targetPortData);
    TABLES_NAME_TO_STRING_CONVERT_MAC(PHA_sourcePortData);

    /* PPU tables */
    TABLES_NAME_TO_STRING_CONVERT_MAC(ppuActionTable0);
    TABLES_NAME_TO_STRING_CONVERT_MAC(ppuActionTable1);
    TABLES_NAME_TO_STRING_CONVERT_MAC(ppuActionTable2);
    TABLES_NAME_TO_STRING_CONVERT_MAC(ppuDauProfileTable);

    /* SMU tables */
    TABLES_NAME_TO_STRING_CONVERT_MAC(smuSngIrf);
    TABLES_NAME_TO_STRING_CONVERT_MAC(smuIrfCounters);

    /* LMU tables */
    TABLES_NAME_TO_STRING_CONVERT_MAC(lmuStatTable);
    TABLES_NAME_TO_STRING_CONVERT_MAC(lmuCfgTable);

    /*PIPE device : */
    TABLES_NAME_TO_STRING_CONVERT_MAC(pipe_PCP_dstPortMapTable);
    TABLES_NAME_TO_STRING_CONVERT_MAC(pipe_PCP_portFilterTable);

    TABLES_NAME_TO_STRING_CONVERT_MAC(pipe_PHA_haTable);
    TABLES_NAME_TO_STRING_CONVERT_MAC(pipe_PHA_targetPortData);
    TABLES_NAME_TO_STRING_CONVERT_MAC(pipe_PHA_srcPortData);

    /* sip6_10 tables */
    TABLES_NAME_TO_STRING_CONVERT_MAC(exactMatchAutoLearnedEntryIndexTable);
    TABLES_NAME_TO_STRING_CONVERT_MAC(queueGroupLatencyProfileConfigTable);
    TABLES_NAME_TO_STRING_CONVERT_MAC(ttiEmProfileId1Mapping);
    TABLES_NAME_TO_STRING_CONVERT_MAC(ttiEmProfileId2Mapping);
    TABLES_NAME_TO_STRING_CONVERT_MAC(ttiPacketTypeTcamProfileIdMapping);
    TABLES_NAME_TO_STRING_CONVERT_MAC(ttiPortAndPacketTypeTcamProfileIdMapping);

    /* tables with 2 parameters */
    TABLES_NAME_TO_STRING_CONVERT_MAC(pclTcam);
    TABLES_NAME_TO_STRING_CONVERT_MAC(pclTcamMask);
    TABLES_NAME_TO_STRING_CONVERT_MAC(tcpUdpDstPortRangeCpuCode);
    TABLES_NAME_TO_STRING_CONVERT_MAC(routerTcam);
    TABLES_NAME_TO_STRING_CONVERT_MAC(ttiAction);
    TABLES_NAME_TO_STRING_CONVERT_MAC(trunkMembers);
    TABLES_NAME_TO_STRING_CONVERT_MAC(deviceMapTable);
    TABLES_NAME_TO_STRING_CONVERT_MAC(policer);
    TABLES_NAME_TO_STRING_CONVERT_MAC(policerCounters);
    TABLES_NAME_TO_STRING_CONVERT_MAC(policerConfig);
    TABLES_NAME_TO_STRING_CONVERT_MAC(policer1Config);
    TABLES_NAME_TO_STRING_CONVERT_MAC(policerConformanceLevelSign);
    TABLES_NAME_TO_STRING_CONVERT_MAC(ieeeRsrvMcConfTable);
    TABLES_NAME_TO_STRING_CONVERT_MAC(logicalTrgMappingTable);
    TABLES_NAME_TO_STRING_CONVERT_MAC(pclConfig);
    TABLES_NAME_TO_STRING_CONVERT_MAC(portProtocolVidQoSConf);
    TABLES_NAME_TO_STRING_CONVERT_MAC(cncMemory);
    TABLES_NAME_TO_STRING_CONVERT_MAC(haPtpDomain);
    TABLES_NAME_TO_STRING_CONVERT_MAC(lpmMemory);
    TABLES_NAME_TO_STRING_CONVERT_MAC(txqPdxQueueGroupMap);
    TABLES_NAME_TO_STRING_CONVERT_MAC(ttiVrfidEvlanMapping);
    TABLES_NAME_TO_STRING_CONVERT_MAC(tcamProfileSubkeySizeAndMux);
    TABLES_NAME_TO_STRING_CONVERT_MAC(tcamSubkeyMux2byteUnits);

    /* tables with 3 parameters */
    TABLES_NAME_TO_STRING_CONVERT_MAC(upToQoSProfile);
    TABLES_NAME_TO_STRING_CONVERT_MAC(ptpLocalActionTable);

    /* tables with 4 parameters */

}


/**
* @internal smemChtTableInfoDump function
* @endinternal
*
* @brief   print the tables info : address,paramInfo[0].steps..
*         info of devObjPtr->tablesInfo
*         print info about the 'reg DB' units
* @param[in] deviceId                 - the simulation device number (according to INI file)
*/
void smemChtTableInfoDump
(
    IN  GT_U32                      deviceId
)
{
/* following macro allows to define filter table mechanism (in one place only)
   it is convinient for table filtering.
   For example to check tables within IPvX baseAddress range only

       if ((ptr->commonInfo.baseAddress == 0) ||\
            (ptr->commonInfo.baseAddress < 0x02800000) || \
            (ptr->commonInfo.baseAddress > 0x02FFFFFC))\
           { continue; } */

#define NOT_SUPPORTED_TABLE_CHECK_MAC(ptr) \
       if(ptr->commonInfo.baseAddress == 0) { continue; }


    SKERNEL_DEVICE_OBJECT * devObjPtr;
    SKERNEL_TABLE_INFO_STC  *tablesPtr;/* pointer to tables with single parameter */
    GT_U32  numTables;/* number of tables with single parameter */
    SKERNEL_TABLE_2_PARAMETERS_INFO_STC  *tables2ParamsPtr; /* pointer to tables with 2 parameters */
    GT_U32  numTables2Params;/* number of tables with 2 parameters */
    SKERNEL_TABLE_3_PARAMETERS_INFO_STC  *tables3ParamsPtr; /* pointer to tables with 3 parameters */
    GT_U32  numTables3Params;/* number of tables with 3 parameters */
    SKERNEL_TABLE_4_PARAMETERS_INFO_STC  *tables4ParamsPtr; /* pointer to tables with 4 parameters */
    GT_U32  numTables4Params;/* number of tables with 4 parameters */
    GT_U32  ii;

    devObjPtr = smemTestDeviceIdToDevPtrConvert(deviceId);

    if(devObjPtr->shellDevice == GT_TRUE)
    {
        /* print info of core 0 */
        devObjPtr = devObjPtr->coreDevInfoPtr[0].devObjPtr;
    }

    /* view the struct  of SMEM_CHT_PP_REGS_ADDR_STC as array of GT_U32 fields */

    if(devObjPtr->supportRegistersDb)
    {
        GT_U32       *regsAddrPtr32;
        GT_U32        regsAddrPtr32Size;
        GT_U32  skipUnitStartNumReg;/* number of registers to skip when we encounter start of unit */
        SMEM_CHT_PP_REGS_UNIT_START_INFO_STC    *startUnitInofPtr;/*pointer to start of current unit*/
        GT_U32       actualRegistersNum = 0;/*number of actual registers that are used in the 'reg DB'*/

        regsAddrPtr32       = (void*)SMEM_CHT_MAC_REG_DB_GET(devObjPtr);
        regsAddrPtr32Size   = sizeof(SMEM_CHT_PP_REGS_ADDR_STC)/sizeof(GT_U32);

        skipUnitStartNumReg = sizeof(SMEM_CHT_PP_REGS_UNIT_START_INFO_STC) / sizeof(GT_U32);

        for( ii = 0; ii < regsAddrPtr32Size; ii++ )
        {
            if(regsAddrPtr32[ii] == SMEM_CHT_PP_REGS_UNIT_START_INFO_MAGIC_NUMBER_CNS)
            {
                startUnitInofPtr = (void*)(&regsAddrPtr32[ii]);
                if(startUnitInofPtr->unitBaseAddress != SMAIN_NOT_VALID_CNS)
                {
                    simForcePrintf(" unit[%s] baseAddr[0x%8.8x] \n",
                        startUnitInofPtr->unitNameStr,startUnitInofPtr->unitBaseAddress);
                }

                ii += skipUnitStartNumReg;
            }

            if(regsAddrPtr32[ii] == SMAIN_NOT_VALID_CNS)
            {
                /* skip not defined registers */
                continue;
            }


            actualRegistersNum++;
        }

        simForcePrintf("actual number of valid registers in DB are [%d] , out of[%d] \n",
            actualRegistersNum,regsAddrPtr32Size);
    }

    smemGenericTablesSectionsGet(devObjPtr,
                            &tablesPtr,&numTables,
                            &tables2ParamsPtr,&numTables2Params,
                            &tables3ParamsPtr,&numTables3Params,
                            &tables4ParamsPtr,&numTables4Params
                            );

    simForcePrintf(" tables with single parameter \n");
    simForcePrintf(" \n");

    simForcePrintf("nameString  , baseAddress , step0 , divider0 \n");
    simForcePrintf("========================================== \n");

    /* loop on tables with single parameter */
    for(ii = 0 ;ii < numTables ;ii++,tablesPtr++)
    {
        NOT_SUPPORTED_TABLE_CHECK_MAC(tablesPtr);

        simForcePrintf("%20.22s \t 0x%8.8x     0x%4.4x      %d \n"
                ,TABLE_NAME_MAC(tablesPtr->commonInfo.nameString)
                ,tablesPtr->commonInfo.baseAddress
                ,tablesPtr->paramInfo[0].step
                ,tablesPtr->paramInfo[0].divider);
    }

    simForcePrintf(" \n");
    simForcePrintf(" \n");

    simForcePrintf(" tables with 2 parameters \n");
    simForcePrintf(" \n");

    simForcePrintf("nameString  , baseAddress , step1 , divider1 , step2 , divider2 \n");
    simForcePrintf("=============================================================== \n");

    for(ii = 0 ;ii < numTables2Params ;ii++,tables2ParamsPtr++)
    {
        NOT_SUPPORTED_TABLE_CHECK_MAC(tables2ParamsPtr);

        simForcePrintf("%20.22s \t 0x%8.8x |  0x%4.4x    %d  | 0x%4.4x    %d \n"
                ,TABLE_NAME_MAC(tables2ParamsPtr->commonInfo.nameString)
                ,tables2ParamsPtr->commonInfo.baseAddress
                ,tables2ParamsPtr->paramInfo[0].step
                ,tables2ParamsPtr->paramInfo[0].divider
                ,tables2ParamsPtr->paramInfo[1].step
                ,tables2ParamsPtr->paramInfo[1].divider);
    }

    simForcePrintf(" \n");
    simForcePrintf(" \n");

    simForcePrintf(" tables with 3 parameters \n");
    simForcePrintf(" \n");

    simForcePrintf("nameString  , baseAddr , step1 , divider1 , step2 , divider2 ,step3 , divider3 \n");
    simForcePrintf("============================================================================== \n");

    for(ii = 0 ;ii < numTables3Params ;ii++,tables3ParamsPtr++)
    {
        NOT_SUPPORTED_TABLE_CHECK_MAC(tables3ParamsPtr);

        simForcePrintf("%20.22s \t 0x%8.8x | "
                        "0x%x %d  |  "
                        "0x%x %d  |  "
                        "0x%x %d"
                        "\n"
                ,TABLE_NAME_MAC(tables3ParamsPtr->commonInfo.nameString)
                ,tables3ParamsPtr->commonInfo.baseAddress
                ,tables3ParamsPtr->paramInfo[0].step
                ,tables3ParamsPtr->paramInfo[0].divider
                ,tables3ParamsPtr->paramInfo[1].step
                ,tables3ParamsPtr->paramInfo[1].divider
                ,tables3ParamsPtr->paramInfo[2].step
                ,tables3ParamsPtr->paramInfo[2].divider
                );
    }

    simForcePrintf(" \n");
    simForcePrintf(" \n");

    simForcePrintf(" tables with 4 parameters \n");
    simForcePrintf(" \n");

    simForcePrintf("nameString  , baseAddr , step1 , div1 , step2 , div2 ,step3 , div3, step4 , div4 \n");
    simForcePrintf("================================================================================ \n");

    for(ii = 0 ;ii < numTables4Params ;ii++,tables4ParamsPtr++)
    {
        NOT_SUPPORTED_TABLE_CHECK_MAC(tables4ParamsPtr);

        simForcePrintf("%20.22s \t 0x%8.8x | "
                        "0x%x %d | "
                        "0x%x %d | "
                        "0x%x %d | "
                        "0x%x %d"
                        "\n"
                ,TABLE_NAME_MAC(tables4ParamsPtr->commonInfo.nameString)
                ,tables4ParamsPtr->commonInfo.baseAddress
                ,tables4ParamsPtr->paramInfo[0].step
                ,tables4ParamsPtr->paramInfo[0].divider
                ,tables4ParamsPtr->paramInfo[1].step
                ,tables4ParamsPtr->paramInfo[1].divider
                ,tables4ParamsPtr->paramInfo[2].step
                ,tables4ParamsPtr->paramInfo[2].divider
                ,tables4ParamsPtr->paramInfo[3].step
                ,tables4ParamsPtr->paramInfo[3].divider
                );
    }

#undef NOT_SUPPORTED_TABLE_CHECK_MAC
}

/**
* @internal smemChtMemCallocReset function
* @endinternal
*
* @brief   Reset memory allocated by function smemDevMemoryCalloc by zeroes.
*
* @param[in] devObjPtr                - pointer to device object
*/
static void smemChtMemCallocReset
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr
)
{
    GT_U32 i;
    SMEM_CALLOC_STC * callocMemPtr;
    SMEM_CHT_DEV_COMMON_MEM_INFO  * devMemInfoPtr =
        (SMEM_CHT_DEV_COMMON_MEM_INFO *)devObjPtr->deviceMemory;

    for(i = 0; i < SMEM_MAX_CALLOC_MEM_SIZE; i++)
    {
        callocMemPtr = &devMemInfoPtr->callocMemArray[i];
        if(callocMemPtr->regPtr)
        {
            memset(callocMemPtr->regPtr, 0,
                   callocMemPtr->regNum * sizeof(SMEM_REGISTER));
        }
    }
}

/**
* @internal smemChtMemReset function
* @endinternal
*
* @brief   Reset memory module with default values for a Cht device.
*
* @param[in] devObjPtr                - pointer to device object.
*/
void smemChtMemReset
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr
)
{
    switch(devObjPtr->deviceFamily)
    {
        case SKERNEL_CHEETAH_1_FAMILY:
        case SKERNEL_CHEETAH_2_FAMILY:
        case SKERNEL_CHEETAH_3_FAMILY:
        case SKERNEL_XCAT_FAMILY:
            smemChtMemCallocReset(devObjPtr);
            smemUnitChunksReset(devObjPtr);
            break;
        case SKERNEL_LION_PORT_GROUP_SHARED_FAMILY:
        case SKERNEL_LION_PORT_GROUP_FAMILY:
        case SKERNEL_XCAT2_FAMILY:
        case SKERNEL_LION2_PORT_GROUP_SHARED_FAMILY:
        case SKERNEL_LION2_PORT_GROUP_FAMILY:
        case SKERNEL_LION3_PORT_GROUP_SHARED_FAMILY:
        case SKERNEL_LION3_PORT_GROUP_FAMILY:
        case SKERNEL_XCAT3_FAMILY:
        case SKERNEL_AC5_FAMILY:
            smemUnitChunksReset(devObjPtr);
            break;
        default:
            skernelFatalError("smemChtMemReset:  device doesn't support SW reset\n");
    }

    /* Set the internal simulation memory defaults */
    smemChtSetInternalSimMemory(devObjPtr);
    /* Load default values for memory from all default registers files */
    smainMemDefaultsLoadAll(devObjPtr, 0, INI_FILE_SYSTEM_SECTION_CNS);
    /* Init memory module for a device - after the load of the default registers file */
    smemInit2(devObjPtr);
}

/**
* @internal smemChtSetInternalSimMemory function
* @endinternal
*
* @brief   Set the internal simulation memory to it's default
*/
void smemChtSetInternalSimMemory
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr
)
{
    GT_U32  *internalMemPtr;
    GT_U32  numOfTiles;
    GT_U32  ii;

    SMEM_CHT_DEV_COMMON_MEM_INFO  * commonDevMemInfoPtr =
        (SMEM_CHT_DEV_COMMON_MEM_INFO *)devObjPtr->deviceMemory;

    /* init the specific bit 0 of this memory */
    internalMemPtr = CHT_INTERNAL_MEM_PTR(devObjPtr,
        CHT_INTERNAL_SIMULATION_USE_MEM_SECURITY_BREACH_STATUS_E);

    *internalMemPtr = 1;

    /* init the AUQ FIFO */
    numOfTiles = (devObjPtr->numOfTiles == 0) ? 1 : devObjPtr->numOfTiles;
    for(ii = 0 ; ii < numOfTiles ; ii++)
    {
        memset(commonDevMemInfoPtr->auqFifoMem[ii].macUpdFifoRegs, 0xFF,
               MAC_UPD_FIFO_REGS_NUM * sizeof(SMEM_REGISTER));
    }

    memset(commonDevMemInfoPtr->gtsIngressFifoMem.gtsFifoRegs, 0xFF,
           GTS_FIFO_REGS_NUM * sizeof(SMEM_REGISTER));

    memset(commonDevMemInfoPtr->gtsEgressFifoMem.gtsFifoRegs, 0xFF,
           GTS_FIFO_REGS_NUM * sizeof(SMEM_REGISTER));

    /* clear AU structure */
    smemChtResetAuq(devObjPtr);

    if(!SKERNEL_DEVICE_FAMILY_CHEETAH_1_ONLY(devObjPtr))
    {
        /* clear FU structure */
        smemCht2ResetFuq(devObjPtr);
    }

    devObjPtr->isPciCompatible = 0;/*just for the initialization*/
    devObjPtr->globalInterruptCauseRegWriteBitmap_pci = 0;
    devObjPtr->globalInterruptCauseRegWriteBitmap_nonPci = 0;

    /* init the byte swapping */
    devObjPtr->txByteSwap = 0;
    devObjPtr->rxByteSwap = 0;

    return;
}

/**
* @internal smemChtPolicerTableIndexCheck function
* @endinternal
*
* @brief   Perform Ingress policer table index out-of-range check.
*         Used only when Policer Memory Control supported
*         Do a fatal error if index is out-of-range.
* @param[in] devObjPtr                - pointer to device object.
* @param[in] cycle                    - 0 - iplr0, 1 - iplr1, 2 - egress policer stage
* @param[in] entryIndex               - index of ingress policer metering or counters table entry
*
* @retval 0                        - index is out-of-range
* @retval 1                        - index is ok (or Policer Memory Control not supported)
*
* @note Without Policer Memory Control - index is checked by another mechanize
*
*/
GT_U8 smemChtPolicerTableIndexCheck
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32 cycle,
    IN GT_U32 entryIndex
)
{
    GT_U32        tableSize;

    /* check index only if Policer Memory Control supported */
    if (devObjPtr->policerSupport.supportPolicerMemoryControl == 0)
    {
        return 1; /* index is ok */
    }
    /* cycle 0 - ingress policer0 stage
        1 - ingress policer1 stage
        2 - egress policer */
    if (cycle > 1)
    {
        return 1; /* index is Ok */
    }

    tableSize = (cycle ? devObjPtr->policerSupport.iplr1TableSize :
                    devObjPtr->policerSupport.iplr0TableSize );

    if (entryIndex >= tableSize)
    {
        skernelFatalError("smemChtPolicerTableIndexCheck: Ingress policer table index is out of range.\n"
                          " Check <Policer Memory Control> configuration. \n"
                          "cycle = %d, index = %d \n", cycle, entryIndex);
    }

    return 1;        /* index is ok */
}


/**
* @internal smemChtPolicerMeteringConfigTableEntryGet function
* @endinternal
*
* @brief  return metering configuration table entry. Used only
*         when Policer Memory Control supported
* @param[in] devObjPtr                - pointer to device object.
* @param[in] cycle                    - 0 - iplr0, 1 - iplr1, 2 - egress policer stage
* @param[in] entryIndex               - index of ingress policer metering or counters table entry
*
* @retval 0                        - index is out-of-range
* @retval 1                        - index is ok (or Policer Memory Control not supported)
*
* @note Without Policer Memory Control - index is checked by another mechanize
*
*/
GT_U32 smemChtPolicerMeteringConfigTableEntryGet
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32 cycle,
    IN GT_U32 entryIndex
)
{
    GT_U32 regAddr;

    /* in sip 6 each PLR unit has it's own instance of the metering configuration table*/
    if(!SMEM_CHT_IS_SIP6_GET(devObjPtr))
    {
        if (((cycle) < 2) || SMEM_CHT_IS_SIP5_20_GET(devObjPtr))
        {
            if (smemChtPolicerTableIndexCheck(devObjPtr, cycle, entryIndex))
            {
                regAddr = SMEM_TABLE_ENTRY_2_PARAMS_INDEX_GET_MAC(devObjPtr, policerConfig, entryIndex, cycle);
            }
            else
            {
                regAddr =  SMAIN_NOT_VALID_CNS;
            }
        }
        else
        {
            regAddr = SMEM_TABLE_ENTRY_INDEX_GET_MAC(devObjPtr, egressPolicerConfig, entryIndex);
        }
    }
    else /*Sip 6*/
    {
        switch (cycle)
        {
            case 0:
                /*IPLR0 */
                regAddr = SMEM_TABLE_ENTRY_2_PARAMS_INDEX_GET_MAC(devObjPtr, policerConfig, entryIndex,cycle);
                break;

            case 1:
                /*IPLR1 */
                regAddr = SMEM_TABLE_ENTRY_INDEX_GET_MAC(devObjPtr, policer1Config, entryIndex);
                break;

            case 2:
                /*egress*/
                regAddr = SMEM_TABLE_ENTRY_INDEX_GET_MAC(devObjPtr, egressPolicerConfig, entryIndex);
                break;

            default:
                regAddr = SMAIN_NOT_VALID_CNS;
        }
    }
    return regAddr;
}



/**
* @internal smemChtActiveReadRateLimitCntReg function
* @endinternal
*
* @brief   Support read of ingress port rate limit counter.
*         Reset counter for current port if current port's window was changed.
* @param[in] devObjPtr                - device object PTR.
* @param[in] address                  - Address for ASIC memory.
* @param[in] memPtr                   - Pointer to the register's memory in the simulation.
*                                      param       - Registers' specific parameter
*
* @param[out] outMemPtr                - Pointer to the memory to copy register's content.
*/
void smemChtActiveReadRateLimitCntReg
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32   address,
    IN GT_U32   memSize,
    IN GT_U32 * memPtr,
    IN GT_UINTPTR sumBit,
    OUT GT_U32 * outMemPtr
)
{
    GT_U32 port;
    GT_U32 currentTimeWindow;                   /* current time window */

    /* Get current port */
    port = address / 0x1000 & 0x3f;

    /* Get port current time window */
    snetChtL2iPortCurrentTimeWindowGet(devObjPtr, port,port, &currentTimeWindow);
    /* Current time window exceeds time window for the port */
    if (currentTimeWindow > devObjPtr->portsArr[port].portCurrentTimeWindow)
    {
        /* Set internal port counter */
        SMEM_U32_SET_FIELD(memPtr[0], 0, 22, 0);
        devObjPtr->portsArr[port].portCurrentTimeWindow = currentTimeWindow;
    }
    /* copy registers content to the output memory */
    *outMemPtr = *memPtr;
}

/**
* @internal smemChtActiveWriteToReadOnlyReg function
* @endinternal
*
* @brief   Protect register from writing since this field is RO.
*
* @param[in] devObjPtr                - device object PTR.
* @param[in] address                  - Address for ASIC memory.
* @param[in] memPtr                   - Pointer to the register's memory in the simulation.
* @param[in] memSize                  - Size of memory to be written
*                                      param      - Registers' specific parameter.
*                                      inMemPtr   - Pointer to the memory to get register's content.
*/
void smemChtActiveWriteToReadOnlyReg
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32   address,
    IN GT_U32   memSize,
    IN GT_U32 * memPtr,
    IN GT_UINTPTR sumBit,
    OUT GT_U32 * outMemPtr
)
{
    return;
}

#define MAX_CLOCKS  16
#define  NOT_VALID_8_CNS      0,0,0,0,0,0,0,0
#define  NOT_VALID_9_CNS      0,NOT_VALID_8_CNS
#define  NOT_VALID_13_CNS     0,0,0,0,NOT_VALID_9_CNS


typedef struct {
    SKERNEL_DEVICE_FAMILY_TYPE deviceFamily;
    GT_U32                      extraIndex;

    SCIB_MEM_ACCESS_CLIENT     registerMemorySpace;
    GT_U32                     regAddr;
    GT_U32                     startBit;
    GT_U32                     numBits;

    GT_U32                     baseCoreClock;
    GT_U32                      coreClkArr[MAX_CLOCKS];/* in the array value 0 is ignored --> use default values*/
}SUPPORTED_CORE_CLOCKS_INFO_STC;

/* DB of supported core clocks */
static SUPPORTED_CORE_CLOCKS_INFO_STC supportedCoreClocksDb[]=
{
    /*SKERNEL_CHEETAH_1_FAMILY*/

    {SKERNEL_CHEETAH_1_FAMILY,  0,  SCIB_MEM_ACCESS_CORE_E ,        /*x*/
        0x000004 ,16,4, 200   , {141,150,166,175,187,200,220, 0,195,197,198,206,200,222,225,250} },

    {SKERNEL_CHEETAH_1_FAMILY,  1,  SCIB_MEM_ACCESS_CORE_E ,        /*x*/
        0x000004 ,16,4, 200 ,   {141,150,166,175,187,200,220, 0,195,191,198,206,200,222,225,250} },

    {SKERNEL_CHEETAH_1_FAMILY,  2,  SCIB_MEM_ACCESS_CORE_E ,
        0x000004 ,16,4, 200 , {144,141,156,NOT_VALID_13_CNS} },

    /*SKERNEL_CHEETAH_2_FAMILY*/

    {SKERNEL_CHEETAH_2_FAMILY,  0,  SCIB_MEM_ACCESS_CORE_E ,
        0x000004 ,16,4, 220   , {141,154,167,180,187,194,207, 0,221,210,214,217,200,224,228,233} },

    /*SKERNEL_CHEETAH_3_FAMILY*/

    {SKERNEL_CHEETAH_3_FAMILY,  0,  SCIB_MEM_ACCESS_CORE_E ,
        0x00000028 ,22,4, 270 , {271,275,267,250,221,204,200,196,187,179,175,321,317,308,300} },

    /*SKERNEL_XCAT_FAMILY*/

    {SKERNEL_XCAT_FAMILY,  0,  SCIB_MEM_ACCESS_CORE_E ,
        0x0000002C,2,3 , 222 , {250,222,200,182,167,154,143,288,NOT_VALID_8_CNS} },

    {SKERNEL_XCAT_FAMILY,  1,  SCIB_MEM_ACCESS_CORE_E ,
        0x0000002C,2,3 , 222 , {250,222,200,182,167,154,143,288,NOT_VALID_8_CNS} },

    /*SKERNEL_XCAT2_FAMILY*/
    {SKERNEL_XCAT2_FAMILY,  0,  SCIB_MEM_ACCESS_CORE_E ,
        0x0000002C,2,3 , 222 , {250,222,200,182,167,154,143,133,NOT_VALID_8_CNS} },


    /*SKERNEL_XCAT3_FAMILY*/
    {SKERNEL_XCAT3_FAMILY,  0,  SCIB_MEM_ACCESS_DFX_E ,
        0x000F8204,21,3 , 360 , {290,250,220,167,200,133,360,NOT_VALID_9_CNS} },

    /*SKERNEL_LION_PORT_GROUP_FAMILY*/
    {SKERNEL_LION_PORT_GROUP_FAMILY,  0 , SCIB_MEM_ACCESS_CORE_E ,
        0x00000028,2, 2 , 360 , {360,320,300,NOT_VALID_13_CNS} },


    /*SKERNEL_LION2_PORT_GROUP_FAMILY*/
    {SKERNEL_LION2_PORT_GROUP_FAMILY,  0 , SCIB_MEM_ACCESS_CORE_E ,
        0x018F8200,0, 3 , 480 , {480,360,270,450,312,240,210,NOT_VALID_9_CNS} },

    /*SKERNEL_BOBCAT2_FAMILY*/
    {SKERNEL_BOBCAT2_FAMILY,  0,  SCIB_MEM_ACCESS_DFX_E ,
        0x000F8204,21,3 , 480 , {362,221,250,400,500,521,450,NOT_VALID_9_CNS} },

    /*SKERNEL_BOBCAT2_FAMILY - B0*/
    {SKERNEL_BOBCAT2_FAMILY,  1,  SCIB_MEM_ACCESS_DFX_E ,
        0x000F8204,21,3 , 480 , {362,221,250,400,175,521,450,NOT_VALID_9_CNS} },

    /*SKERNEL_BOBK_CAELUM_FAMILY*/
    {SKERNEL_BOBK_CAELUM_FAMILY,  0,  SCIB_MEM_ACCESS_DFX_E ,
        0x000F8204,21,3 , 480 , {365,220,250,200,167,133,225,NOT_VALID_9_CNS} },

    /*SKERNEL_BOBCAT3_FAMILY*/
    {SKERNEL_BOBCAT3_FAMILY,  0,  SCIB_MEM_ACCESS_DFX_E ,
        0x000F8204,20,4 , 480 , {525, 487, 525, 583, 600, 625, 556, 250, 572, 577, 585, 400, 535, 545, 593, 0} },

    /*SKERNEL_BOBK_ALDRIN_FAMILY*/
    {SKERNEL_BOBK_ALDRIN_FAMILY,  0,  SCIB_MEM_ACCESS_DFX_E ,
        0x000F8204,21,3 , 480 , {365,220,250,200,480,510,540,NOT_VALID_9_CNS} },

    /*SKERNEL_AC3X_FAMILY*/
    {SKERNEL_AC3X_FAMILY,  0,  SCIB_MEM_ACCESS_DFX_E ,
        0x000F8204,21,3 , 480 , {365,220,250,200,480,510,540,NOT_VALID_9_CNS} },

    /*SKERNEL_PIPE_FAMILY */
    {SKERNEL_PIPE_FAMILY,  0,  SCIB_MEM_ACCESS_DFX_E ,
        0x000F8204,21,3 , 480 , {500,450,288,350,531,550,575,NOT_VALID_9_CNS} },

    /*SKERNEL_ALDRIN2_FAMILY */
    {SKERNEL_ALDRIN2_FAMILY,  0,  SCIB_MEM_ACCESS_DFX_E ,
        0x000F8204,21,3 , 600 , {450,0,600,525,0,0,0,NOT_VALID_9_CNS} },

    /*SKERNEL_FALCON_FAMILY*/
    {SKERNEL_FALCON_FAMILY,  0,  SCIB_MEM_ACCESS_DFX_E ,
        SMAIN_NOT_VALID_CNS,18,3 , 700 , {700, 732, 661, 818, 618, 553, 420, 78} },

    /*SKERNEL_HAWK_FAMILY*/
    {SKERNEL_HAWK_FAMILY,  0,  SCIB_MEM_ACCESS_DFX_E ,
        SMAIN_NOT_VALID_CNS,18,3 , 812 , {812, 844, 781, 875, 750, 650, 487, 25} },

    /*SKERNEL_PHOENIX_FAMILY*/
    {SKERNEL_PHOENIX_FAMILY,  0,  SCIB_MEM_ACCESS_DFX_E ,
        SMAIN_NOT_VALID_CNS,18,3 , 510 , {510, 535, 485,140, 147, 133, 0, NOT_VALID_9_CNS }  },

    /*SKERNEL_AC5_FAMILY*/
    {SKERNEL_AC5_FAMILY,  0 , SCIB_MEM_ACCESS_DFX_E ,
        SMAIN_NOT_VALID_CNS,22, 2, 395 , {395,290,167,NOT_VALID_13_CNS} },

    /*SKERNEL_HARRIER_FAMILY*/ /*currently values from Hawk */
    {SKERNEL_HARRIER_FAMILY,  0,  SCIB_MEM_ACCESS_DFX_E ,
        SMAIN_NOT_VALID_CNS,18,3 , 812 , {812, 844, 781, 875, 750, 650, 487, 25} },

    /*SKERNEL_IRONMAN_FAMILY*/ /*currently values from Hawk */
    {SKERNEL_IRONMAN_FAMILY,  0,  SCIB_MEM_ACCESS_DFX_E ,
        SMAIN_NOT_VALID_CNS,18,3 , 600 , {600, 844, 781, 875, 750, 650, 487, 25} },

    /* last entry */
    { SKERNEL_LAST_FAMILY , 0 , 0 ,
        0,0,0          ,0   ,  {0}}
};


/*******************************************************************************
* hwPpCoreClockGetDbEntry
*
* DESCRIPTION:
*       get the DB entry of core clocks for the device , or NULL for not supporting device.
*
* INPUTS:
*       deviceObjPtr         - the device pointer
*
* OUTPUTS:
*
*
* RETURNS:
*       pointer to the DB entry.
*
* COMMENTS:
*
*
*******************************************************************************/
static SUPPORTED_CORE_CLOCKS_INFO_STC* hwPpCoreClockGetDbEntry
(
    IN SKERNEL_DEVICE_OBJECT * deviceObjPtr
)
{
    GT_U32  ii;
    GT_U32  extraIndex;
    SKERNEL_DEVICE_FAMILY_TYPE deviceFamily;

    extraIndex = 0;
    deviceFamily = deviceObjPtr->deviceFamily;

    switch(deviceObjPtr->deviceFamily)
    {
        case SKERNEL_CHEETAH_1_FAMILY:
            if (deviceObjPtr->deviceType == SKERNEL_98DX269 ||
                deviceObjPtr->deviceType == SKERNEL_98DX269 )
            {
                extraIndex = 1;
            }
            else if (deviceObjPtr->deviceType == SKERNEL_98DX106)
            {
                extraIndex = 2;
            }

            break;
        case SKERNEL_XCAT_FAMILY:
            if(deviceObjPtr->deviceRevisionId != 0)
            {
                extraIndex   = 0;
            }
            else
            {
                extraIndex   = 1;
            }
            break;
        case SKERNEL_LION2_PORT_GROUP_FAMILY:
            if(GT_FALSE ==
                smemIsDeviceMemoryOwner(deviceObjPtr,0x018F8200))
            {
                /*we will get info from the 'father' that is set by core '1' */
                return NULL;
            }
            break;
        case SKERNEL_BOBCAT2_FAMILY:
            if(deviceObjPtr->deviceRevisionId == 0)
            {
                extraIndex   = 0;
            }
            else
            {
                extraIndex   = 1;
            }
            break;

        case SKERNEL_BOBK_CETUS_FAMILY:
            deviceFamily = SKERNEL_BOBK_CAELUM_FAMILY;
            break;

        default:
            break;
    }

    for(ii = 0 ; supportedCoreClocksDb[ii].deviceFamily != SKERNEL_LAST_FAMILY ; ii++)
    {
        if(deviceFamily != supportedCoreClocksDb[ii].deviceFamily)
        {
            continue;
        }

        if(extraIndex != supportedCoreClocksDb[ii].extraIndex)
        {
            continue;
        }

        /* we found our entry */
        return &supportedCoreClocksDb[ii];
    }

    return NULL;
}

/**
* @internal setCoreClockToRegister function
* @endinternal
*
* @brief   Write to the register the new core clock + UPADET the DB of the 'device object'
*
* @param[in] deviceObjPtr             - the device pointer
* @param[in] dbEntryPtr               - the DB entry for the coreClock of the device
* @param[in] value                    - the  to write to the register
*/
static void setCoreClockToRegister
(
    IN SKERNEL_DEVICE_OBJECT * deviceObjPtr,
    IN SUPPORTED_CORE_CLOCKS_INFO_STC  *dbEntryPtr,
    IN GT_U32               value
)
{
    GT_U32  regAddr = dbEntryPtr->regAddr;

    if(value >= (GT_U32)(1<<dbEntryPtr->numBits))
    {
        skernelFatalError("setCoreClockToRegister: out of range [%d] \n",value);
        return ;
    }

    if(dbEntryPtr->registerMemorySpace == SCIB_MEM_ACCESS_DFX_E)
    {
        /* DFX memory space */
        if (regAddr == SMAIN_NOT_VALID_CNS)
        {
            /* Default register address is taken from DB */
            regAddr = SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(deviceObjPtr)->resetAndInitCtrller.DFXServerUnitsBC2SpecificRegs.deviceSAR2;
        }
        smemDfxRegFldSet(deviceObjPtr, regAddr,
            dbEntryPtr->startBit, dbEntryPtr->numBits, value);
    }
    else
    {
        /* Device memory space */
        smemRegFldSet(deviceObjPtr, regAddr,
            dbEntryPtr->startBit, dbEntryPtr->numBits, value);
    }

    simForcePrintf("Core clock value set: ");
    /*
    *       read from the register the core clock and update the update :
    *            deviceObjPtr->coreClk
    *            deviceObjPtr->baseCoreClock
    *            deviceObjPtr->fdbBaseCoreClock
    *            deviceObjPtr->fdbAgingFactor
    */
    updateDevObjCoreClockInfo(deviceObjPtr);

    return;
}

/**
* @internal getCoreClockFromRegister function
* @endinternal
*
* @brief   read from the register the core clock
*
* @param[in] deviceObjPtr             - the device pointer
* @param[in] dbEntryPtr               - the DB entry for the coreClock of the device
*                                       the core clock. (value in MHz)
*
* @retval 0 means                  --> use 'default'
*/
static GT_U32 getCoreClockFromRegister
(
    IN SKERNEL_DEVICE_OBJECT * deviceObjPtr,
    IN SUPPORTED_CORE_CLOCKS_INFO_STC  *dbEntryPtr
)
{
    GT_U32  data;
    GT_U32  regAddr = dbEntryPtr->regAddr;

    if(dbEntryPtr->registerMemorySpace == SCIB_MEM_ACCESS_DFX_E)
    {
        /* DFX memory space */
        if (regAddr == SMAIN_NOT_VALID_CNS)
        {
            /* Default register address is taken from DB */
            regAddr = SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(deviceObjPtr)->resetAndInitCtrller.DFXServerUnitsBC2SpecificRegs.deviceSAR2;
        }
        smemDfxRegFldGet(deviceObjPtr, regAddr,
            dbEntryPtr->startBit, dbEntryPtr->numBits, &data);
    }
    else
    {
        /* Device memory space */
        smemRegFldGet(deviceObjPtr, regAddr,
            dbEntryPtr->startBit, dbEntryPtr->numBits, &data);
    }

    if(data >= MAX_CLOCKS)
    {
        skernelFatalError("getCoreClockFromRegister: out of range [%d] \n",data);
        return 0;
    }

    return dbEntryPtr->coreClkArr[data];
}


/**
* @internal updateDevObjCoreClockInfo function
* @endinternal
*
* @brief   read from the register the core clock and update the update :
*         deviceObjPtr->coreClk
*         deviceObjPtr->baseCoreClock
*         deviceObjPtr->fdbBaseCoreClock
*         deviceObjPtr->fdbAgingFactor
*
* @retval GT_TRUE                  - the device   hold coreClock info
* @retval GT_FALSE                 - the device NOT hold coreClock info
*/
static GT_BOOL updateDevObjCoreClockInfo
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr
)
{
    GT_U32  coreClk,baseCoreClock;
    GT_U32  fdbAgingFactor = 10;
    SUPPORTED_CORE_CLOCKS_INFO_STC  *dbEntryPtr;

    /* check that the DB hold info about our device */
    dbEntryPtr = hwPpCoreClockGetDbEntry(devObjPtr);
    if(dbEntryPtr == NULL)
    {
        return GT_FALSE;
    }

    coreClk = getCoreClockFromRegister(devObjPtr,dbEntryPtr);
    if(coreClk == 0)
    {
        coreClk = 222;/*????*/
    }

    baseCoreClock = dbEntryPtr->baseCoreClock;


    if(devObjPtr->portGroupSharedDevObjPtr)
    {
        /* the multi core device need to use single info of the FATHER because
            it may not be per core */

        devObjPtr = devObjPtr->portGroupSharedDevObjPtr;
    }

    devObjPtr->baseCoreClock = baseCoreClock;
    devObjPtr->fdbBaseCoreClock = devObjPtr->baseCoreClock;

    switch(devObjPtr->deviceFamily)
    {
        case SKERNEL_CHEETAH_3_FAMILY:
            fdbAgingFactor  = 16;
            break;
        case SKERNEL_XCAT_FAMILY:
        case SKERNEL_XCAT3_FAMILY:
        case SKERNEL_AC5_FAMILY:
            devObjPtr->fdbBaseCoreClock = 235;
            break;
        case SKERNEL_XCAT2_FAMILY:
            devObjPtr->fdbBaseCoreClock = 167;
            break;
        default:
            break;
    }

    devObjPtr->fdbAgingFactor = fdbAgingFactor;
    devObjPtr->coreClk       = coreClk;

    __LOG_PARAM_NO_LOCATION_META_DATA(devObjPtr->baseCoreClock);
    __LOG_PARAM_NO_LOCATION_META_DATA(devObjPtr->fdbBaseCoreClock);
    __LOG_PARAM_NO_LOCATION_META_DATA(devObjPtr->fdbAgingFactor);

    __LOG_PARAM_NO_LOCATION_META_DATA(devObjPtr->coreClk);

    simForcePrintf("device [%d] coreClk [%d] MHz \n",
        devObjPtr->deviceId,
        devObjPtr->coreClk);

    return GT_TRUE;
}


/**
* @internal hwPpAutoDetectCoreClock function
* @endinternal
*
* @brief   Retrieves Core Clock value
*
* @param[in] deviceObjPtr             - the device pointer
*
* @retval GT_OK                    - on success,
* @retval GT_BAD_STATE             - can't map HW value to core clock value.
* @retval GT_FAIL                  - otherwise.
*
* @note Function is using hard coded registers address since registers DB is not
*       yet initialized.
*
*/
static void hwPpAutoDetectCoreClock
(
    IN SKERNEL_DEVICE_OBJECT * deviceObjPtr
)
{
    simForcePrintf("Auto-detected core clock value: ");
    /*
    *       read from the register the core clock and update the update :
    *            deviceObjPtr->coreClk
    *            deviceObjPtr->baseCoreClock
    *            deviceObjPtr->fdbBaseCoreClock
    *            deviceObjPtr->fdbAgingFactor
    */
    updateDevObjCoreClockInfo(deviceObjPtr);
}

/**
* @internal smemCheetahUpdateCoreClockRegister function
* @endinternal
*
* @brief   Update the value of the Core Clock in the register.
*         there are 2 options:
*         1. give 'coreClockInMHz' which will be translated to proper 'hw field value'
*         and then written to the HW.
*         2. give 'hwFieldValue' which written directly to the HW.
* @param[in] deviceObjPtr             - the device pointer
* @param[in] coreClockInMHz           - core clock in MHz.
*                                      value SMAIN_NOT_VALID_CNS means ignored
* @param[in] hwFieldValue             -  value to set to the core clock field in the HW.
*                                      NOTE: used only when coreClockInMHz == SMAIN_NOT_VALID_CNS.
*
* @retval GT_OK                    - on success,
* @retval GT_BAD_STATE             - can't map HW value to core clock value.
* @retval GT_FAIL                  - otherwise.
*/
void smemCheetahUpdateCoreClockRegister
(
    IN SKERNEL_DEVICE_OBJECT * deviceObjPtr,
    IN GT_U32                  coreClockInMHz,
    IN GT_U32                  hwFieldValue
)
{
    SUPPORTED_CORE_CLOCKS_INFO_STC  *dbEntryPtr;
    GT_U32  ii;

    /* check that the DB hold info about our device */
    dbEntryPtr = hwPpCoreClockGetDbEntry(deviceObjPtr);
    if(dbEntryPtr == NULL)
    {
        simForcePrintf("updateCoreClockRegister: not supported device : deviceId[%d] (core clock not updated) \n",
            deviceObjPtr->deviceId);
        return;
    }

    if(coreClockInMHz == SMAIN_NOT_VALID_CNS)
    {
        setCoreClockToRegister(deviceObjPtr,dbEntryPtr,hwFieldValue);
        return;
    }

    /* need to calculate the proper value that need to write to the HW based on the 'MHz' value */
    if(coreClockInMHz != 0)
    {
        for(ii = 0 ; ii < MAX_CLOCKS ; ii++)
        {
            if(dbEntryPtr->coreClkArr[ii] == coreClockInMHz)
            {
                setCoreClockToRegister(deviceObjPtr,dbEntryPtr,ii);
                return;
            }
        }
    }

    skernelFatalError("smemCheetahUpdateCoreClockRegister: the core clock [%d] MHz was not found in the DB \n"
        "Only next values are supported : "
        "[%d]"
        "[%d]"
        "[%d]"
        "[%d]"
        "[%d]"
        "[%d]"
        "[%d]"
        "[%d]"
        "[%d]"
        "[%d]"
        "[%d]"
        "[%d]"
        "[%d]"
        "[%d]"
        "[%d]"
        "[%d]"
        ,
        coreClockInMHz,
        dbEntryPtr->coreClkArr[0],
        dbEntryPtr->coreClkArr[1],
        dbEntryPtr->coreClkArr[2],
        dbEntryPtr->coreClkArr[3],
        dbEntryPtr->coreClkArr[4],
        dbEntryPtr->coreClkArr[5],
        dbEntryPtr->coreClkArr[6],
        dbEntryPtr->coreClkArr[7],
        dbEntryPtr->coreClkArr[8],
        dbEntryPtr->coreClkArr[9],
        dbEntryPtr->coreClkArr[10],
        dbEntryPtr->coreClkArr[11],
        dbEntryPtr->coreClkArr[12],
        dbEntryPtr->coreClkArr[13],
        dbEntryPtr->coreClkArr[14],
        dbEntryPtr->coreClkArr[15]
        );
}

/**
* @internal smemChtInitStateSet function
* @endinternal
*
* @brief   State init status to global control registers
*/
void smemChtInitStateSet
(
    IN SKERNEL_DEVICE_OBJECT * deviceObjPtr
)
{
    GT_U32  address;

    if(deviceObjPtr->shellDevice == GT_TRUE)
    {
        return;
    }

    /* get the core clock values */
    hwPpAutoDetectCoreClock(deviceObjPtr);

    if(SMEM_CHT_IS_SIP5_GET(deviceObjPtr) || /* also PIPE */
       SKERNEL_IS_XCAT3_BASED_DEV(deviceObjPtr))
    {
        goto dfx_sip5_lbl;
    }

    switch(deviceObjPtr->deviceFamily)
    {
        case SKERNEL_CHEETAH_1_FAMILY:
        case SKERNEL_CHEETAH_2_FAMILY:
        case SKERNEL_CHEETAH_3_FAMILY:
        case SKERNEL_XCAT_FAMILY:
        case SKERNEL_XCAT2_FAMILY:
            /* Set the global control register */
            smemRegFldSet(deviceObjPtr,SMEM_CHT_GLB_CTRL_REG(deviceObjPtr), 17, 2, 0x3);
            break;
        case SKERNEL_LION_PORT_GROUP_FAMILY:
            /* Set the global control register */
            smemRegFldSet(deviceObjPtr,SMEM_CHT_GLB_CTRL_REG(deviceObjPtr), 17, 2, 0x3);
            /* Set the extended global control register only for lion B0 and lion2 portGroup 0 */
            if(deviceObjPtr->portGroupId == 0)
            {
                smemRegFldSet(deviceObjPtr,SMEM_LION_EXT_GLB_CTRL_REG(deviceObjPtr), 13, 2, 0x3);
            }
            break;
        case SKERNEL_LION2_PORT_GROUP_FAMILY:
            dfx_sip5_lbl:
            address = SMEM_LION2_DFX_SERVER_INIT_DONE_STATUS_REG(deviceObjPtr);
            if(SMEM_CHT_IS_DFX_SERVER(deviceObjPtr)) /*sip5,xcat3,pipe,sip6*/
            {
                /* Initialization is fully functional */
                smemDfxRegFldSet(deviceObjPtr, address, 4, 2, 3);

                address = SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(deviceObjPtr)->resetAndInitCtrller.
                    DFXServerUnitsBC2SpecificRegs.initializationStatusDone;
                if (SMAIN_NOT_VALID_CNS != address)
                {
                    smemDfxRegFldSet(deviceObjPtr, address, 0, 1, 0);
                }

                /* set 1 to <MG Soft Reset Trigger> this indicates that soft reset in not processed (or finished processed) */
                smemDfxRegFldSet(deviceObjPtr, SMEM_LION2_DFX_SERVER_RESET_CONTROL_REG(deviceObjPtr), 1, 1, 1);
            }
            else
            if(GT_FALSE ==
                smemIsDeviceMemoryOwner(deviceObjPtr,address))
            {
                /* the address not belongs to this device */
            }
            else
            {
                /* Set internal init done bit[1] and all init done bit[0] */
                smemRegFldSet(deviceObjPtr, address, 0, 2, 0);
            }
            break;
        default:
            break;
    }

    return;
}

/**
* @internal smemChtFdbTableNumEntriesPrint function
* @endinternal
*
* @brief   print number of valid and not skipped entries in the FDB
*
* @param[in] deviceId                 - the simulation device number (according to INI file)
*/
void smemChtFdbTableNumEntriesPrint
(
    IN  GT_U32                      deviceId
)
{
    SKERNEL_DEVICE_OBJECT * devObjPtr;
    GT_U32  numOfFdbEntries , ii ,value1, value2, skip,totalValid;
    GT_U32  *memPtr;

    devObjPtr = smemTestDeviceIdToDevPtrConvert(deviceId);

    if(devObjPtr->shellDevice == GT_TRUE)
    {
        /* print info of core 0 */
        devObjPtr = devObjPtr->coreDevInfoPtr[0].devObjPtr;
    }

    numOfFdbEntries = devObjPtr->fdbMaxNumEntries;
    totalValid = 0;

    if(SMEM_CHT_IS_SIP6_GET(devObjPtr))
    {
        for(ii = 0 ; ii < numOfFdbEntries; ii++ )
        {
            /* Get FDB entry */
            memPtr = SMEM_SIP6_HIDDEN_FDB_PTR(devObjPtr, ii);

            /* valid*/
            value1 =
                SMEM_SIP6_FDB_FDB_ENTRY_FIELD_GET(devObjPtr,memPtr,
                    ii,
                    SMEM_SIP6_FDB_FDB_TABLE_FIELDS_VALID);

            if(devObjPtr->multiHashEnable)
            {
                /* skip is ignored in multi hash mode ! */
                skip = 0;
            }
            else
            {
                /*skip*/
                skip =
                    SMEM_SIP6_FDB_FDB_ENTRY_FIELD_GET(devObjPtr,memPtr,
                        ii,
                        SMEM_SIP6_FDB_FDB_TABLE_FIELDS_SKIP);
            }
            /*SPUnknown*/
            value2 =
                SMEM_SIP6_FDB_FDB_ENTRY_FIELD_GET(devObjPtr,memPtr,
                    ii,
                    SMEM_SIP6_FDB_FDB_TABLE_FIELDS_SP_UNKNOWN);

            if(value1 == 1 &&  /* valid    */
               skip == 0 &&    /* not skip */
               value2 == 0)    /* not SP   */
            {
                totalValid++;
            }
        }
    }
    else
    if(SMEM_CHT_IS_SIP5_GET(devObjPtr))
    {
        for(ii = 0 ; ii < numOfFdbEntries; ii++ )
        {
            memPtr = smemMemGet( devObjPtr , SMEM_CHT_MAC_TBL_MEM(devObjPtr,ii));

            /* valid*/
            value1 =
                SMEM_LION3_FDB_FDB_ENTRY_FIELD_GET(devObjPtr,memPtr,
                    ii,
                    SMEM_LION3_FDB_FDB_TABLE_FIELDS_VALID);

            if(devObjPtr->multiHashEnable)
            {
                /* skip is ignored in multi hash mode ! */
                skip = 0;
            }
            else
            {
                /*skip*/
                skip =
                    SMEM_LION3_FDB_FDB_ENTRY_FIELD_GET(devObjPtr,memPtr,
                        ii,
                        SMEM_LION3_FDB_FDB_TABLE_FIELDS_SKIP);
            }
            /*SPUnknown*/
            value2 =
                SMEM_LION3_FDB_FDB_ENTRY_FIELD_GET(devObjPtr,memPtr,
                    ii,
                    SMEM_LION3_FDB_FDB_TABLE_FIELDS_SP_UNKNOWN);

            if(value1 == 1 &&  /* valid    */
               skip == 0 &&    /* not skip */
               value2 == 0)    /* not SP   */
            {
                totalValid++;
            }
        }
    }
    else
    {
        for(ii = 0 ; ii < numOfFdbEntries; ii++ )
        {
            memPtr = smemMemGet( devObjPtr , SMEM_CHT_MAC_TBL_MEM(devObjPtr,ii));

            value1 = snetFieldValueGet(memPtr,0,2); /* valid = 1 , skip = 0*/
            value2 = snetFieldValueGet(memPtr,98,1); /* SPUnknown = 0*/
            if(value1 == 1 && value2 == 0)
            {
                totalValid++;
            }
        }
    }

    simForcePrintf(" FDB hold [%d] valid,non-skip,non SP entries out of [%d] FDB size , max HW is [%d] \n",
        totalValid,devObjPtr->fdbNumEntries,devObjPtr->fdbMaxNumEntries);

}

/**
* @internal smemChtActiveReadRocRegister function
* @endinternal
*
* @brief   Read register and then clear it's data
*
* @param[in] devObjPtr                - device object PTR.
* @param[in] memPtr                   - pointer to the register's memory in the simulation.
*
* @param[out] outMemPtr                - pointer to the memory to copy register's content.
*
* @note Generic function is used in active memory read functions for ROC registers.
*
*/
void smemChtActiveReadRocRegister
(
    IN         SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN         GT_U32 * memPtr,
    OUT        GT_U32 * outMemPtr
)
{
    /* Copy registers content to the output memory */
    *outMemPtr = *memPtr;
    /* Clear register after read */
    *memPtr = 0;
}

/* get the instance in the device that hold the needed memory */
GT_U32 smemChtInstanceGet(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SMEM_CHT_MEMORY_DEVICE_ENT memoryType
)
{
    DECLARE_FUNC_NAME(smemChtInstanceGet);

    GT_U32 pipeId,tileId,mgUnitIndex;
    if(! SMEM_CHT_IS_SIP6_GET(devObjPtr))
    {
        return 0;
    }

    switch(memoryType)
    {
        case SMEM_CHT_MEMORY_DEVICE_AUQ_E:
        case SMEM_CHT_MEMORY_DEVICE_ON_CHIP_FIFO_AUQ_E:
            if(devObjPtr->numOfTiles &&
               devObjPtr->numOfMgUnits)
            {
                /* AUQ,FIFO per tile */
                mgUnitIndex = smemGetCurrentMgUnitIndex(devObjPtr);

                tileId = mgUnitIndex / (devObjPtr->numOfMgUnits / devObjPtr->numOfTiles);

                __LOG(("AUQ/FIFO (per tile) served by MG unit [%d] for tile[%d] \n",
                    mgUnitIndex,
                    tileId));
            }
            else
            {
                tileId = 0;/*single AUQ,FIFO */
            }
            return tileId;
        case SMEM_CHT_MEMORY_DEVICE_FUQ_E:
            if(devObjPtr->numOfMgUnits && devObjPtr->numOfPipes)
            {
                GT_U32  numOfTiles = devObjPtr->numOfTiles ?
                                     devObjPtr->numOfTiles :
                                     1;/* support single tile with 2 MG unit for 2 pipes */
                /* FUQ per pipe */
                mgUnitIndex = smemGetCurrentMgUnitIndex(devObjPtr);
                tileId = mgUnitIndex / (devObjPtr->numOfMgUnits / numOfTiles);

                pipeId = (tileId * devObjPtr->numOfPipesPerTile) +
                          mgUnitIndex % devObjPtr->numOfPipesPerTile;

                __LOG(("FUQ (per pipe) served by MG unit [%d] for pipe[%d] (in tile [%d]) \n",
                    mgUnitIndex,
                    pipeId,
                    tileId));
            }
            else
            if(devObjPtr->numOfMgUnits)/* support HAwk with FUQ for cnc0,1 in MG[0] and for CNC2,3 in MG[1] */
            {
                /* support multiple MGs , each may hold FUQ for CNC uploads */
                mgUnitIndex = smemGetCurrentMgUnitIndex(devObjPtr);
                __LOG(("FUQ (per MG) served by MG unit [%d] \n",
                    mgUnitIndex));

                if(mgUnitIndex >= SIM_MAX_PIPES_CNS)
                {
                    skernelFatalError("must enlarge fuqMem array to at least [%d] ,and not only SIM_MAX_PIPES_CNS[%d]\n",
                        mgUnitIndex+1,
                        SIM_MAX_PIPES_CNS);
                    return 0;
                }

                return mgUnitIndex;
            }
            else
            {
                pipeId =  0;/* single FUQ */
            }

            /* FUQ per pipe */
            return pipeId;
        default:
            skernelFatalError("smemChtInstanceGet: unknown type [%d] \n",memoryType);
            return 0;
    }
}


/**
* @internal smemUnitBaseAddrByNameGet function
* @endinternal
*
* @brief   Get the base address of a unit by it's name (STRING).
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] unitNameStr              - string of the name of the unit.
* @param[in] allowNonExistUnit        - indication to allow not exists unit ..
*                                      when != 0
*                                      if unit not exists return SMAIN_NOT_VALID_CNS
*                                      when == 0
*                                      if unit not exists --> fatal error
*                                       base address of unit .
*                                       returns SMAIN_NOT_VALID_CNS for not valid name for the device.
*/
GT_U32 smemUnitBaseAddrByNameGet
(
    IN SKERNEL_DEVICE_OBJECT_T_PTR devObjPtr,
    IN GT_CHAR*                    unitNameStr,
    IN GT_BIT                      allowNonExistUnit
)
{
    GT_U32      ii;
    GT_U32      addressIndex;
    GT_U32      baseAddr;

    if(devObjPtr->devMemUnitNameAndIndexPtr == NULL ||
       unitNameStr == NULL)
    {
        skernelFatalError("smemUnitBaseAddrByNameGet: NULL pointer \n");
        return SMAIN_NOT_VALID_CNS;
    }

    for(ii = 0 ;
        devObjPtr->devMemUnitNameAndIndexPtr[ii].unitNameStr != NULL ;
        ii++)
    {
        if(0 == strcmp(devObjPtr->devMemUnitNameAndIndexPtr[ii].unitNameStr,unitNameStr))
        {
            /* match found */
            addressIndex =  devObjPtr->devMemUnitNameAndIndexPtr[ii].unitNameIndex;

            baseAddr = smemUnitBaseAddrByIndexGet(devObjPtr,addressIndex);
            if(baseAddr == SMAIN_NOT_VALID_CNS && (allowNonExistUnit == 0))
            {
                skernelFatalError("smemUnitBaseAddrByNameGet: unit named[%s] declared as 'not valid' for the device \n",unitNameStr);
            }

            return baseAddr;
        }
    }

    if(allowNonExistUnit == 0)
    {
        skernelFatalError("smemUnitBaseAddrByNameGet: unit named[%s] not found \n",unitNameStr);
    }

    return SMAIN_NOT_VALID_CNS;
}

/**
* @internal smemUnitBaseAddrByIndexGet function
* @endinternal
*
* @brief   Get the base address of a unit by it's index in
*         devObjPtr->genericUsedUnitsAddressesArray[] .
* @param[in] devObjPtr                - pointer to device object.
* @param[in] addressIndex             - index in devObjPtr->genericUsedUnitsAddressesArray[]
*                                       base address of unit .
*                                       returns SMAIN_NOT_VALID_CNS for not valid name for the device.
*/
GT_U32 smemUnitBaseAddrByIndexGet
(
    IN SKERNEL_DEVICE_OBJECT_T_PTR devObjPtr,
    IN GT_U32                      addressIndex
)
{
    if(devObjPtr->genericUsedUnitsAddressesArray == NULL ||
        addressIndex >= devObjPtr->genericNumUnitsAddresses)
    {
        skernelFatalError("smemUnitBaseAddrByIndexGet: NULL pointer or access violation index \n");
        return SMAIN_NOT_VALID_CNS;
    }

    return devObjPtr->genericUsedUnitsAddressesArray[addressIndex].unitBaseAddr;
}

/**
* @internal smemUnitIndexByNameGet function
* @endinternal
*
* @brief   Get the unit index (index in devObjPtr->genericUsedUnitsAddressesArray[])
*         of a unit by it's name.
* @param[in] devObjPtr                - pointer to device object.
* @param[in] unitNameStr              - string of the name of the unit.
*
* @retval index in devObjPtr       ->genericUsedUnitsAddressesArray[] .
*/
GT_U32 smemUnitIndexByNameGet
(
    IN SKERNEL_DEVICE_OBJECT_T_PTR devObjPtr,
    IN GT_CHAR*                    unitNameStr
)
{
    GT_U32  ii;

    if(devObjPtr->support_memoryRanges == 0)
    {
        /* implementation of (UNIT_BASE_ADDR_GET_MAC(devObjPtr,unitName) >> SMEM_CHT_UNIT_INDEX_FIRST_BIT_CNS) */
        return smemUnitBaseAddrByNameGet(devObjPtr,unitNameStr,0) >> SMEM_CHT_UNIT_INDEX_FIRST_BIT_CNS;
    }

    /* need to search for the index in devObjPtr->devMemUnitNameAndIndexPtr[]
       that is also index in devObjPtr->genericUsedUnitsAddressesArray[] */
    for(ii = 0 ; devObjPtr->devMemUnitNameAndIndexPtr[ii].unitNameStr; ii++)
    {
        if(0 == strcmp(devObjPtr->devMemUnitNameAndIndexPtr[ii].unitNameStr,unitNameStr))
        {
            return ii;/* this is also index to devObjPtr->genericUsedUnitsAddressesArray[] */
        }
    }

    skernelFatalError("smemUnitIndexByNameGet: unit [%s] was not found \n" ,
        unitNameStr);

    return 0;
}

/**
* @internal address_cellCompare function
* @endinternal
*
* @brief   Function compare two units base address of formula array
*
* @param[in] cell1Ptr                 - pointer to tested value
* @param[in] cell2Ptr                 - pointer to device formula cell array
*                                       The return value of this function represents whether cell1Ptr is considered
*                                       less than, equal, or grater than cell2Ptr by returning, respectively,
*                                       a negative value, zero or a positive value.
*
* @note function's prototype must be defined return int to avoid warnings and be
*       consistent with qsort() function prototype
*
*/
static int  address_cellCompare
(
    const GT_VOID * cell1Ptr,
    const GT_VOID * cell2Ptr
)
{
    SMEM_UNIT_BASE_AND_SIZE_STC * cellTestedPtr = (SMEM_UNIT_BASE_AND_SIZE_STC *)cell1Ptr;
    SMEM_UNIT_BASE_AND_SIZE_STC * cellInDbPtr   = (SMEM_UNIT_BASE_AND_SIZE_STC *)cell2Ptr;

    if (cellTestedPtr->unitBaseAddr < cellInDbPtr->unitBaseAddr)
    {
        return -1;
    }

    if (cellTestedPtr->unitBaseAddr < (cellInDbPtr->unitBaseAddr + cellInDbPtr->unitSizeInBytes))
    {
        /* belongs to this range */
        return 0;
    }

    return 1;
}

/**
* @internal smemUnitIndexByAddrGet function
* @endinternal
*
* @brief   Get the unit index (index in devObjPtr->genericUsedUnitsAddressesArray[])
*         of a unit by address of register/table in it.
* @param[in] devObjPtr                - pointer to device object.
* @param[in] address                  -  to get unit index for.
*
* @retval index in devObjPtr       ->genericUsedUnitsAddressesArray[] .
*/
GT_U32 smemUnitIndexByAddrGet
(
    IN SKERNEL_DEVICE_OBJECT_T_PTR devObjPtr,
    IN GT_U32                      address
)
{
    GT_U32  index;
    SMEM_UNIT_BASE_AND_SIZE_STC findAddr;
    void *foundAddrPtr;

    if(devObjPtr->support_memoryRanges == 0)
    {
        return address >> SMEM_CHT_UNIT_INDEX_FIRST_BIT_CNS;
    }

    /* need to search for the index in devObjPtr->genericUsedUnitsAddressesArray[] */
    findAddr.unitBaseAddr    = address;
    findAddr.unitSizeInBytes = 0;/* not used in compare */
    /********************************************************/
    /* assume that the genericNumUnitsAddresses is sorted ! */
    /********************************************************/
    foundAddrPtr = bsearch(&findAddr,
        devObjPtr->genericUsedUnitsAddressesArray,
        devObjPtr->genericNumUnitsAddresses,
        sizeof(SMEM_UNIT_BASE_AND_SIZE_STC),
        address_cellCompare);

    if(foundAddrPtr)
    {
        /* deducting of pointers results in index */
       index = ((SMEM_UNIT_BASE_AND_SIZE_STC*)foundAddrPtr - devObjPtr->genericUsedUnitsAddressesArray);

        if(address >= devObjPtr->genericUsedUnitsAddressesArray[index].unitBaseAddr &&
           address < (devObjPtr->genericUsedUnitsAddressesArray[index].unitBaseAddr +
                      devObjPtr->genericUsedUnitsAddressesArray[index].unitSizeInBytes))
        {
            return index;
        }
    }

    if(debugModeByPassFatalErrorOnMemNotExists)
    {
        /* the caller need to handle it . */
        /* should happen only from smemGenericFindMem(...) */
        debugModeByPassFatalErrorOnMemNotExists--;/* limit the number of times */
        return SMAIN_NOT_VALID_CNS;
    }

    skernelFatalError("smemUnitIndexByAddrGet: address [0x%8.8x] was not found (to belong to any existing unit in the memory), foundAddrPtr %d \n" ,
        address,(foundAddrPtr == NULL? 0: 1));
    return 0;

}


/*******************************************************************************
*   smemUnitNameByAddressGet
*
* DESCRIPTION:
*       Get the name of the unit by the address .
*
* INPUTS:
*       devObjPtr   - pointer to device object.
*       address     - address to get unit name for.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       name of unit or NULL
*
* COMMENTS:
*
*******************************************************************************/
GT_CHAR* smemUnitNameByAddressGet
(
    IN SKERNEL_DEVICE_OBJECT_T_PTR devObjPtr,
    IN GT_U32                      address
)
{
    GT_U32      addressIndex;
    GT_U32      ii;
    GT_U32      currentIndex,unitNameIndex;

    if(devObjPtr->devMemUnitNameAndIndexPtr == NULL)
    {
        return NULL;
    }

    addressIndex = UNIT_INDEX_FROM_ADDR_GET_MAC(devObjPtr,address);

    for(ii = 0 ; devObjPtr->devMemUnitNameAndIndexPtr[ii].unitNameStr; ii++)
    {
        unitNameIndex  = devObjPtr->devMemUnitNameAndIndexPtr[ii].unitNameIndex;
        currentIndex = UNIT_INDEX_FROM_ADDR_GET_MAC(devObjPtr,devObjPtr->genericUsedUnitsAddressesArray[unitNameIndex].unitBaseAddr);

        if(currentIndex == addressIndex)
        {
            return devObjPtr->devMemUnitNameAndIndexPtr[ii].unitNameStr;
        }
    }

    return NULL;
}

/**
* @internal smemMibCounterInfoByAddrGet function
* @endinternal
*
* @brief   get info about mib counter according to associated address .
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] address                  - the  in the MIB
*
* @param[out] infoPtr                  - pointer to the needed info.
*/
void smemMibCounterInfoByAddrGet
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32                   address,
    OUT SKERNEL_MIB_COUNTER_INFO_STC   *infoPtr
)
{
    GT_U32  port;
    GT_U32  offsetInUnit;
    GT_U32  offsetFromStartTable;
    GT_U32  stepPerPort;
    GT_U32  portOffset;

    if(devObjPtr->devMemMibPortByAddrGetPtr)
    {
        /* hold special/complex logic */
        infoPtr->portNum = devObjPtr->devMemMibPortByAddrGetPtr(devObjPtr,address);
        return;
    }

    if(devObjPtr->offsetToXgCounters_1)
    {
        offsetInUnit = address & 0x00FFFFFF;

        if(offsetInUnit >= devObjPtr->offsetToXgCounters_1)
        {
            offsetFromStartTable = offsetInUnit - devObjPtr->offsetToXgCounters_1;
            stepPerPort = devObjPtr->xgCountersStepPerPort_1;
            portOffset = devObjPtr->startPortNumInXgCounters_1;
        }
        else
        {
            offsetFromStartTable = offsetInUnit;
            stepPerPort = devObjPtr->xgCountersStepPerPort;
            portOffset = 0;
        }

        port = (offsetFromStartTable / stepPerPort) & 0xFF;

        port += portOffset;
    }
    else
    {
        port =  CHT3_FROM_XG_PORT_COUNTER_ADDRESS_GET_PORT_CNS(devObjPtr,address);
    }

    if(devObjPtr->numOfPipes)
    {
        smemConvertCurrentPipeIdAndLocalPortToGlobal(devObjPtr,port,GT_FALSE,&port);
    }

    infoPtr->portNum = port;

    return;
}

/**
* @internal smemGopPortByAddrGet function
* @endinternal
*
* @brief   get port id according to associated address .
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] address                  - the  in the  space of the port
*
* @note the port id
*
*/
GT_U32 smemGopPortByAddrGet
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32                   address
)
{
    GT_U32  offset;
    GT_U32  mask = 0x3f;/* don't use SMEM_BIT_MASK((dev)->flexFieldNumBitsSupport.phyPort) because port is local to the core*/
    GT_U32  addrMask = SMEM_CHT_IS_SIP5_GET(devObjPtr) ? 0x00F3FFFF : 0x007FFFFF;
    GT_U32  addressToCheck = address & addrMask;/* take relative address */
    GT_U32  portCompensation = 0;
    GT_U32  gopPortNum;

    if(devObjPtr->devMemGopPortByAddrGetPtr)
    {
        /* hold dedicated/complex logic */
        return devObjPtr->devMemGopPortByAddrGetPtr(devObjPtr,address);
    }

    if(devObjPtr->portMacOffset && devObjPtr->portMacMask)
    {
        offset = devObjPtr->portMacOffset;
        mask   = devObjPtr->portMacMask;

        if(devObjPtr->portMacSecondBase && (addressToCheck >= devObjPtr->portMacSecondBase))
        {
            /* the addressToCheck is within the 'second base' of ports */

            /* so adjust port number and addressToCheck accordingly */
            addressToCheck  -= devObjPtr->portMacSecondBase;
            portCompensation = devObjPtr->portMacSecondBaseFirstPort;
        }
    }
    else
    if(devObjPtr->supportRegistersDb)
    {
        offset = (SMEM_CHT_MAC_REG_DB_GET(devObjPtr))->GOP.ports.gigPort[1].portMACCtrlReg[0] -
                 (SMEM_CHT_MAC_REG_DB_GET(devObjPtr))->GOP.ports.gigPort[0].portMACCtrlReg[0];
    }
    else
    if(SKERNEL_IS_CHEETAH3_DEV(devObjPtr))
    {
        offset = 0x400;
    }
    else
    {
        offset = 0x100;
    }

    gopPortNum = ((addressToCheck / offset) + portCompensation) & mask;

    if(devObjPtr->numOfPipes)
    {
        smemConvertCurrentPipeIdAndLocalPortToGlobal(devObjPtr,gopPortNum,GT_FALSE,&gopPortNum);
    }

    return gopPortNum;
}


/**
* @internal smemMibCounterAddrByPortGet function
* @endinternal
*
* @brief   get start address of MIB counter memory.
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] portNum                  - the port number for the MIB
*                                       start address of MIB counter memory
*/
GT_U32 smemMibCounterAddrByPortGet
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32                   portNum
)
{
    GT_U32  portOffset;
    GT_U32  index;
    GT_BIT  found;

    if(devObjPtr->portsArr[portNum].mibBaseAddr)
    {
        return devObjPtr->portsArr[portNum].mibBaseAddr;
    }

    /* we need the portNum as 'local port' in the Pipe */
    if(devObjPtr->gop_specialPortMappingArr)
    {
        found = 0;

        /* check for 'special' port numbers */
        for (index = 0 ; devObjPtr->gop_specialPortMappingArr[index].globalPortNumber != SMAIN_NOT_VALID_CNS ; index++)
        {
            if(devObjPtr->gop_specialPortMappingArr[index].globalPortNumber == portNum)
            {
                portNum = devObjPtr->gop_specialPortMappingArr[index].localPortNumber;
                found = 1;
                break;
            }
        }

        if (found == 0 && devObjPtr->numOfPipes)
        {
            portNum   %= devObjPtr->numOfPortsPerPipe;
        }

    }

    if(devObjPtr->offsetToXgCounters_1)
    {
        if(portNum >= devObjPtr->startPortNumInXgCounters_1)
        {
            portOffset = portNum - devObjPtr->startPortNumInXgCounters_1;
            /* access table_1 */
            return SMEM_CHT3_HGS_PORT_MAC_MIB_COUNT_TABLE1_REG(devObjPtr, portOffset);
        }
        else
        {
            /* access table_0 */
            return SMEM_CHT3_HGS_PORT_MAC_MIB_COUNT_TABLE0_REG(devObjPtr, portNum);
        }
    }

    return SMEM_CHT3_HGS_PORT_MAC_MIB_COUNT_TABLE0_REG(devObjPtr, portNum);
}

#define SMAIN_NO_GM_DEVICE                  0xFF
/**
* @internal smemInitRegistersWithDefaultValues function
* @endinternal
*
* @brief   Init registers with default values.
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] accessType               - memory access type
* @param[in] linkListRegistersDefaultsPtr - (pointer to) link list of arrays of registers with default values.
* @param[in] specificUnitPtr          - (pointer to) specific unit info in case of 'per unit'.
*                                      can be NULL (not 'per port')
*/
void smemInitRegistersWithDefaultValues
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SCIB_MEMORY_ACCESS_TYPE accessType,
    IN SMEM_LINK_LIST_REGISTER_DEFAULT_VALUE_STC  *linkListRegistersDefaultsPtr,
    IN SMEM_UNIT_CHUNKS_STC*   specificUnitPtr
)
{
    GT_U32  ii,jj[3];
    GT_U32  currentAddress;
    GT_U32  startAddress;
    GT_U32  value;
    GT_U32  numOfRepetitions[3];
    GT_U32  stepSizeBeteenRepetitions[3];
    GT_U32  unitBaseAddr;
    GT_U32  previous_unitBaseAddr = SMAIN_NOT_VALID_CNS;
    SMEM_CHT_GENERIC_DEV_MEM_INFO *devMemInfoPtr = devObjPtr->deviceMemory;
    SMEM_REGISTER_DEFAULT_VALUE_STC *registersDefaultsArr;
    SMEM_UNIT_CHUNKS_STC    *currUnitChunkPtr;
    GT_U32  unitIndex;
    GT_U32  repCount; /* repetition counter - number of current register instance */

    if(devObjPtr->gmDeviceType != SMAIN_NO_GM_DEVICE)
    {
        return;
    }

    while(linkListRegistersDefaultsPtr != NULL)
    {
        registersDefaultsArr = linkListRegistersDefaultsPtr->currDefaultValuesArrPtr;

        for(ii = 0 ; registersDefaultsArr[ii].unitNameStrPtr != NULL ; ii++)
        {
            if(IS_BAR0_OPERATION_MAC(accessType))
            {
                unitBaseAddr = 0;
            }
            else
            if(IS_PCI_OPERATION_MAC(accessType))
            {
                unitBaseAddr = devMemInfoPtr->common.pciUnitBaseAddr;
            }
            else
            if((IS_DFX_OPERATION_MAC(accessType) == 0) ||
                (!SMEM_CHT_IS_DFX_ON_UNIQUE_MEMORY_SPCAE(devObjPtr)))
            {
                if(specificUnitPtr == NULL)
                {
                    unitBaseAddr = smemUnitBaseAddrByNameGet(devObjPtr,*(registersDefaultsArr[ii].unitNameStrPtr),1);

                    if(unitBaseAddr != SMAIN_NOT_VALID_CNS)
                    {
                        /* check if the unit hold explicit default register values */
                        unitIndex = UNIT_INDEX_FROM_ADDR_GET_MAC(devObjPtr,unitBaseAddr);
                        currUnitChunkPtr = &devMemInfoPtr->unitMemArr[unitIndex];
                        if(currUnitChunkPtr->unitDefaultRegistersPtr)
                        {
                            /* skip this entry .. it is set via
                                currUnitChunkPtr->unitDefaultRegistersPtr */
                            unitBaseAddr = SMAIN_NOT_VALID_CNS;
                        }
                    }
                }
                else
                {
                    specificUnitPtr = specificUnitPtr->hugeUnitSupportPtr ?
                                      specificUnitPtr->hugeUnitSupportPtr :
                                      specificUnitPtr;

                    if(devObjPtr->support_memoryRanges == 0)
                    {
                        unitBaseAddr = specificUnitPtr->chunkIndex << SMEM_CHT_UNIT_INDEX_FIRST_BIT_CNS;
                    }
                    else
                    {
                        unitBaseAddr = smemUnitBaseAddrByIndexGet(devObjPtr,specificUnitPtr->chunkIndex);
                    }
                }
            }
            else
            {
                unitBaseAddr = devMemInfoPtr->common.dfxMemBaseAddr;
            }

            if(unitBaseAddr == SMAIN_NOT_VALID_CNS)
            {
                continue;
            }

            startAddress = unitBaseAddr + registersDefaultsArr[ii].address;

            value = registersDefaultsArr[ii].value;
            stepSizeBeteenRepetitions[0] = registersDefaultsArr[ii].stepSizeBeteenRepetitions;
            numOfRepetitions[0] = registersDefaultsArr[ii].numOfRepetitions;
            if(0 == numOfRepetitions[0])
            {
                numOfRepetitions[0] = 1;
                stepSizeBeteenRepetitions[0] = 4;
            }

            stepSizeBeteenRepetitions[1] = registersDefaultsArr[ii].stepSizeBeteenRepetitions_2;
            numOfRepetitions[1] = registersDefaultsArr[ii].numOfRepetitions_2;
            if(0 == numOfRepetitions[1])
            {
                numOfRepetitions[1] = 1;
                stepSizeBeteenRepetitions[1] = 4;
            }

            stepSizeBeteenRepetitions[2] = registersDefaultsArr[ii].stepSizeBeteenRepetitions_3;
            numOfRepetitions[2] = registersDefaultsArr[ii].numOfRepetitions_3;
            if(0 == numOfRepetitions[2])
            {
                numOfRepetitions[2] = 1;
                stepSizeBeteenRepetitions[2] = 4;
            }

            for(jj[0] = 0 ; jj[0] < numOfRepetitions[0]; jj[0]++)
            {
                if((0 == IS_PCI_OPERATION_MAC(accessType)) &&
                   ((0 == IS_DFX_OPERATION_MAC(accessType)) ||
                    (!SMEM_CHT_IS_DFX_ON_UNIQUE_MEMORY_SPCAE(devObjPtr))))
                {
                    if((jj[0] == 0) &&
                        GT_FALSE == smemIsDeviceMemoryOwner(devObjPtr,startAddress))
                    {
                        /* this chunk not relevant to this device */
                        /* optimize INIT time of 'shared memory' */
                        break;
                    }
                }

                for(jj[1] = 0 ; jj[1] < numOfRepetitions[1]; jj[1]++)
                {
                    for(jj[2] = 0 ; jj[2] < numOfRepetitions[2]; jj[2]++)
                    {
                        currentAddress = startAddress +
                            stepSizeBeteenRepetitions[0] * jj[0] +
                            stepSizeBeteenRepetitions[1] * jj[1] +
                            stepSizeBeteenRepetitions[2] * jj[2] ;

                        if (registersDefaultsArr[ii].valueFuncPtr)
                        {
                            repCount = jj[2] + (jj[1] * numOfRepetitions[2]) + (jj[0] *  numOfRepetitions[2] *  numOfRepetitions[1]);
                            value = (*registersDefaultsArr[ii].valueFuncPtr)(&(registersDefaultsArr[ii]), repCount);
                        }

                        if(IS_BAR0_OPERATION_MAC(accessType))
                        {
                            smemBar0RegSet(devObjPtr, currentAddress, value);
                        }
                        else
                        if(IS_PCI_OPERATION_MAC(accessType))
                        {
                            smemPciRegSet(devObjPtr, currentAddress, value);
                        }
                        else
                        if(IS_DFX_OPERATION_MAC(accessType) &&
                            SMEM_CHT_IS_DFX_ON_UNIQUE_MEMORY_SPCAE(devObjPtr))
                        {
                            smemDfxRegSet(devObjPtr, currentAddress, value);
                        }
                        else
                        {
                            GT_U32  relativeAddr = currentAddress-unitBaseAddr;
                            static GT_U32   otherUnitsBaseAddr[50];
                            static GT_U32  numOtherUnits = 0;
                            GT_U32  iter;
                            GT_U32  tempBaseAddr;

                            /* check ignore before isUnitDuplicated() , to
                               reduce the need to duplicate the units into 'ignored' list */
                            if(devObjPtr->registersDefaultsPtr_ignored)
                            {
                                if(isRegisterIgnored(devObjPtr,
                                        unitBaseAddr,
                                        relativeAddr,
                                        devObjPtr->registersDefaultsPtr_ignored))
                                {
                                    /* ignore the register */
                                    continue;
                                }
                            }

                            /* avoid calculation of unit duplication if the unit not changed */
                            if(previous_unitBaseAddr != unitBaseAddr)
                            {
                                (void)isUnitDuplicated(devObjPtr,unitBaseAddr,&numOtherUnits,otherUnitsBaseAddr);

                                /* set 'my unit' as first , and swap first to be last one */
                                tempBaseAddr = otherUnitsBaseAddr[0];
                                otherUnitsBaseAddr[0] = unitBaseAddr;
                                if(numOtherUnits)
                                {
                                    otherUnitsBaseAddr[numOtherUnits] = tempBaseAddr;
                                }
                                numOtherUnits++;

                                /* update the 'prevoius' unit base address */
                                previous_unitBaseAddr = unitBaseAddr;
                            }

                            for(iter = 0 ; iter < numOtherUnits; iter++)
                            {
                                tempBaseAddr = otherUnitsBaseAddr[iter];

                                smemRegSet(devObjPtr, (tempBaseAddr + relativeAddr) ,value);
                            }

                        }
                    }
                }
            }
        }

        /* jump to next element in the link list */
        linkListRegistersDefaultsPtr = linkListRegistersDefaultsPtr->nextPtr;
    }

    return;
}




/**
* @internal smemCheetahDuplicateSingleUnit function
* @endinternal
*
* @brief   do duplications of single unit from orig unit.
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] origUnitChunkPtr         - pointer to the original unit.
* @param[in] dupUnitBaseAddr          - base address of the duplicated unit
*
* @param[out] dupUnitChunkPtr          - pointer to the duplicated unit.
*                                       None.
*/
static void smemCheetahDuplicateSingleUnit
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SMEM_UNIT_CHUNKS_STC    *origUnitChunkPtr,
    OUT SMEM_UNIT_CHUNKS_STC   *dupUnitChunkPtr,
    IN GT_U32                   dupUnitBaseAddr
)
{
    GT_U32  ii;
    GT_U32  numOfUnits;
    SMEM_CHUNK_STC  *dupChunkPtr,*origChunkPtr;

    /* copy all 'flat' content */
    *dupUnitChunkPtr = *origUnitChunkPtr;
    /* handle dynamic allocations */

    dupUnitChunkPtr->chunksArray =
        smemDeviceObjMemoryAlloc(devObjPtr,dupUnitChunkPtr->numOfChunks,sizeof(SMEM_CHUNK_STC));

    dupChunkPtr  = &dupUnitChunkPtr->chunksArray[0];
    origChunkPtr = &origUnitChunkPtr->chunksArray[0];

    for(ii = 0 ; ii < dupUnitChunkPtr->numOfChunks ; ii++,dupChunkPtr++,origChunkPtr++)
    {
        /* copy all 'flat' content */
        *dupChunkPtr = *origChunkPtr;

        if(dupChunkPtr->tableOffsetInBytes != SMAIN_NOT_VALID_CNS)
        {
            /* pipe 1 should not 'steal' the table indication from the 'original' unit */
            /* also regular unit should not 'steal' the table indication from the 'original' unit*/
            dupChunkPtr->tableOffsetInBytes = 0;
        }

        /* handle dynamic allocations */
        dupChunkPtr->memPtr =
            smemDeviceObjMemoryAlloc(devObjPtr,dupChunkPtr->memSize , sizeof(SMEM_REGISTER));

    }

    dupUnitChunkPtr->hugeUnitSupportPtr = dupUnitChunkPtr;/* point to myself */

    numOfUnits = dupUnitChunkPtr->numOfUnits;
    if(dupUnitChunkPtr->chunkType == SMEM_UNIT_CHUNK_TYPE_8_MSB_E)
    {
        numOfUnits *= 2;
    }

    /* duplicate the numOfUnits*/
    for(ii = 1 ; ii < numOfUnits; ii++)
    {
        dupUnitChunkPtr[ii].hugeUnitSupportPtr = dupUnitChunkPtr;
        dupUnitChunkPtr[ii].numOfUnits = 1;
        dupUnitChunkPtr[ii].chunkType = SMEM_UNIT_CHUNK_TYPE_9_MSB_E;
    }

    return;
}

/**
* @internal smemCheetahDuplicateUnitAllocationCheck function
* @endinternal
*
* @brief   Check duplications of the units.
*
* @param[in] devObjPtr                - pointer to device object.
*                                       None.
*/
static GT_BOOL smemCheetahDuplicateUnitAllocationCheck
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SMEM_UNIT_DUPLICATION_INFO_STC* currentPtr
)
{
    GT_U32  baseAddr;
    GT_CHAR * unitName;
    GT_U32  pipeOffset;
    GT_U32  pipeId = SMAIN_NOT_VALID_CNS;

    /* Get duplicated unit name and base address */
    unitName = currentPtr->unitNameStr;
    baseAddr = smemUnitBaseAddrByNameGet(devObjPtr, unitName, 0);

    if(devObjPtr->numOfPipes)
    {
        /* validate pipe offset for duplicated/sub-init */
        pipeOffset = SMEM_UNIT_PIPE_OFFSET_GET_MAC(devObjPtr, baseAddr , &pipeId);
    }
    else
    {
        pipeOffset = 0;
        pipeId     = 0;
    }

    if(devObjPtr->support_memoryRanges)
    {
        if(pipeId == SMAIN_NOT_VALID_CNS)
        {
            /* indication that the unit NOT exists , and should be skipped ! */
            return GT_FALSE;
        }
    }
    else
    {
        if (pipeOffset && (pipeOffset >> (SMEM_CHT_UNIT_INDEX_FIRST_BIT_CNS + 1)) == 0)
        {
            /* The 'sub-unit' is in the address space of the 'parent unit', and will be allocated in scope of the 'parent unit' */
            return GT_FALSE;
        }
    }

    return GT_TRUE;
}

/**
* @internal smemCheetahDuplicateUnits function
* @endinternal
*
* @brief   do duplications of the units. according to unitsDuplicationsPtr.
*
* @param[in] devObjPtr                - pointer to device object.
*                                       None.
*/
void smemCheetahDuplicateUnits
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr
)
{
    GT_U32  ii;
    GT_U32  currentUnitBaseAddr;
    SMEM_UNIT_DUPLICATION_INFO_STC* currentPtr = &devObjPtr->unitsDuplicationsPtr[0];
    GT_CHAR* name_a,*name_b;
    GT_U32  numOfUnits;
    SMEM_CHT_GENERIC_DEV_MEM_INFO *devMemInfoPtr = devObjPtr->deviceMemory;
    SMEM_UNIT_CHUNKS_STC    *origUnitChunkPtr;
    SMEM_UNIT_CHUNKS_STC    *currUnitChunkPtr;
    GT_U32  unitIndex;

    while(currentPtr->unitNameStr != NULL)
    {
        name_a = currentPtr->unitNameStr;
        numOfUnits = currentPtr->numOfUnits;

        currentPtr++;/* jump to start of the duplicated units*/

        if (smemCheetahDuplicateUnitAllocationCheck(devObjPtr, currentPtr) == GT_FALSE)
        {
            currentPtr += numOfUnits;
            /* don't duplicate unit memory */
            continue;
        }

        currentUnitBaseAddr = smemUnitBaseAddrByNameGet(devObjPtr,name_a,0);

        /* get pointer to the original unit */
        unitIndex = UNIT_INDEX_FROM_ADDR_GET_MAC(devObjPtr,currentUnitBaseAddr);
        origUnitChunkPtr = &devMemInfoPtr->unitMemArr[unitIndex];
        if(origUnitChunkPtr->numOfChunks == 0)
        {
            skernelFatalError("smemCheetahDuplicateUnits: empty unit need duplication ?! [%s] \n" ,
                name_a);
        }

        /* we got orig unit info , need to duplicate it's allocations */
        for(ii = 0 ; ii < numOfUnits ; ii++ , currentPtr++)
        {
            name_b = currentPtr->unitNameStr;
            currentUnitBaseAddr = smemUnitBaseAddrByNameGet(devObjPtr,name_b,0);

            /* get pointer to the duplicated unit */
            unitIndex = UNIT_INDEX_FROM_ADDR_GET_MAC(devObjPtr,currentUnitBaseAddr);

            currUnitChunkPtr = &devMemInfoPtr->unitMemArr[unitIndex];
            if(currUnitChunkPtr->numOfChunks != 0)
            {
                skernelFatalError("smemCheetahDuplicateUnits: unit that need duplication [%s] (from[%s]) already allocated ? \n" ,
                    name_b,name_a);
            }

            /* duplicate the unit from the original one */
            smemCheetahDuplicateSingleUnit(devObjPtr,origUnitChunkPtr,currUnitChunkPtr,currentUnitBaseAddr);

            /* set the index of the new unit */
            currUnitChunkPtr->chunkIndex = unitIndex;
        }
    }
}

/**
* @internal smemCheetahDuplicateSingleUnitActiveMemory function
* @endinternal
*
* @brief   do duplications of the active memories of single unit from orig unit.
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] origUnitChunkPtr         - pointer to the original unit.
* @param[in] origUnitBaseAddr         - base address of the original unit
*                                      currentUnitBaseAddr  - base address of the duplicated unit
*
* @param[out] dupUnitChunkPtr          - pointer to the duplicated unit.
*                                       None.
*/
static void smemCheetahDuplicateSingleUnitActiveMemory
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SMEM_UNIT_CHUNKS_STC    *origUnitChunkPtr,
    OUT SMEM_UNIT_CHUNKS_STC   *dupUnitChunkPtr,
    IN GT_U32                   origUnitBaseAddr,
    IN GT_U32                   dupUnitBaseAddr
)
{
    GT_U32  ii;
    GT_BIT  isOrigAlreadyWithBaseAddr;

    /* calculate the size of array in the original unit */
    for (ii = 0; origUnitChunkPtr->unitActiveMemPtr[ii].address != END_OF_TABLE; ii++)
    {
        /* just increment ii */
    }

    if(origUnitChunkPtr->unitActiveMemPtr[ii].mask == SMEM_FULL_MASK_CNS)
    {   /* same logic in initBaseAddressOfUnitActiveMemory(...) */
        isOrigAlreadyWithBaseAddr = 0;
    }
    else
    {
        isOrigAlreadyWithBaseAddr = 1;
    }

    ii++;/* entry of END_OF_TABLE */

    dupUnitChunkPtr->unitActiveMemPtr =
        smemDeviceObjMemoryAlloc(devObjPtr,ii,sizeof(SMEM_ACTIVE_MEM_ENTRY_STC));

    /* copy the memory to new allocated */
    /* calculate the size of array in the original unit */
    for (ii = 0; origUnitChunkPtr->unitActiveMemPtr[ii].address != END_OF_TABLE; ii++)
    {
        dupUnitChunkPtr->unitActiveMemPtr[ii] = origUnitChunkPtr->unitActiveMemPtr[ii];

        if(isOrigAlreadyWithBaseAddr)
        {
            if(dupUnitChunkPtr->unitActiveMemPtr[ii].address < origUnitBaseAddr)
            {
                skernelFatalError("smemCheetahDuplicateSingleUnitActiveMemory: base address [0x%8.8x] of unit is more than active memory address[0x%8.8x]\n",
                    origUnitBaseAddr,
                    dupUnitChunkPtr->unitActiveMemPtr[ii].address);
            }


            /* remove base address of orig unit , and add base address of dup unit */
            dupUnitChunkPtr->unitActiveMemPtr[ii].address -= origUnitBaseAddr;
            dupUnitChunkPtr->unitActiveMemPtr[ii].address += dupUnitBaseAddr;
        }
    }

    dupUnitChunkPtr->unitActiveMemPtr[ii] = origUnitChunkPtr->unitActiveMemPtr[ii];/* entry of END_OF_TABLE */
    if(isOrigAlreadyWithBaseAddr)
    {
        /* remove base address of orig unit , and add base address of dup unit */
        dupUnitChunkPtr->unitActiveMemPtr[ii].mask -= origUnitBaseAddr;
        dupUnitChunkPtr->unitActiveMemPtr[ii].mask += dupUnitBaseAddr;
    }


    return;
}
/**
* @internal smemCheetahDuplicateUnitsActiveMemory function
* @endinternal
*
* @brief   do duplications of the active memories of units.
*
* @param[in] devObjPtr                - pointer to device object.
*                                       None.
*/
void smemCheetahDuplicateUnitsActiveMemory
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr
)
{
    GT_U32  ii;
    GT_U32  currentUnitBaseAddr,origUnitBaseAddr;
    SMEM_UNIT_DUPLICATION_INFO_STC* currentPtr = &devObjPtr->unitsDuplicationsPtr[0];
    GT_CHAR* name_a,*name_b;
    GT_U32  numOfUnits;
    SMEM_CHT_GENERIC_DEV_MEM_INFO *devMemInfoPtr = devObjPtr->deviceMemory;
    SMEM_UNIT_CHUNKS_STC    *origUnitChunkPtr;
    SMEM_UNIT_CHUNKS_STC    *currUnitChunkPtr;
    GT_U32  unitIndex;

    while(currentPtr->unitNameStr != NULL)
    {
        name_a = currentPtr->unitNameStr;
        numOfUnits = currentPtr->numOfUnits;

        currentPtr++;/* jump to start of the duplicated units*/

        if (smemCheetahDuplicateUnitAllocationCheck(devObjPtr, currentPtr) == GT_FALSE)
        {
            currentPtr += numOfUnits;/* jump to start of the duplicated units*/
            /* don't duplicate unit memory */
            continue;
        }

        currentUnitBaseAddr = smemUnitBaseAddrByNameGet(devObjPtr,name_a,0);
        origUnitBaseAddr = currentUnitBaseAddr;

        /* get pointer to the original unit */
        unitIndex = UNIT_INDEX_FROM_ADDR_GET_MAC(devObjPtr,currentUnitBaseAddr);
        origUnitChunkPtr = &devMemInfoPtr->unitMemArr[unitIndex];

        /* we got orig unit info , need to duplicate it's allocations */
        for(ii = 0 ; ii < numOfUnits ; ii++ , currentPtr++)
        {
            name_b = currentPtr->unitNameStr;
            currentUnitBaseAddr = smemUnitBaseAddrByNameGet(devObjPtr,name_b,0);

            unitIndex = UNIT_INDEX_FROM_ADDR_GET_MAC(devObjPtr,currentUnitBaseAddr);

            /* get pointer to the original unit */
            currUnitChunkPtr = &devMemInfoPtr->unitMemArr[unitIndex];
            /* duplicate the unit from the original one */
            smemCheetahDuplicateSingleUnitActiveMemory(devObjPtr,origUnitChunkPtr,currUnitChunkPtr,origUnitBaseAddr,currentUnitBaseAddr);
        }
    }
}


/**
* @internal gopCgInit function
* @endinternal
*
* @brief   init the DB - GOP CG unit
*         INPUTS:
*         devObjPtr  - pointer to device object.
*         portNum - port number
* @param[in] devObjPtr                - pointer to device object.
*                                      portNum  - port number
*                                      INPUTS:
*/
static void gopCgInit
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN  GT_U32 portIndex
)
{
    GT_U32 ii;
    GT_U32 portNum;
    GT_U32 portFormula;

    portNum = portIndex;

    if((devObjPtr->numOfPipes >= 2) && (portIndex >= devObjPtr->numOfPortsPerPipe))
    {
        /* set only ports of pipe 0 */
        /* the rest of the alignment will be in updateGopOldDbWithPipes(...) */
        portIndex = portIndex % devObjPtr->numOfPortsPerPipe;
    }

    portFormula = 0x1000 * portIndex;

    /*start of unit CG */
    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->GOP.ports_100G_CG_MAC[portNum].CG_CONVERTERS.CGMAControl0 = 0x340000 + portFormula;
    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->GOP.ports_100G_CG_MAC[portNum].CG_CONVERTERS.CGMAConvertersStatus = 0x340020 + portFormula;
    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->GOP.ports_100G_CG_MAC[portNum].CG_CONVERTERS.CGMAConvertersResets = 0x340010 + portFormula;
    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->GOP.ports_100G_CG_MAC[portNum].CG_CONVERTERS.CGMAConvertersIpStatus = 0x340028 + portFormula;
    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->GOP.ports_100G_CG_MAC[portNum].CG_CONVERTERS.CGMAConvertersFcControl0 = 0x340060 + portFormula;

    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->GOP.ports_100G_CG_MAC[portNum].CG_CONVERTERS.CGMAConvertersCG_Interrupt_cause = 0x340040 + portFormula;
    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->GOP.ports_100G_CG_MAC[portNum].CG_CONVERTERS.CGMAConvertersCG_Interrupt_mask = 0x340044 + portFormula;
    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->GOP.ports_100G_CG_MAC[portNum].CG_CONVERTERS.CGMAConvertersExternal_Units_Interrupts_cause = 0x340048 + portFormula;
    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->GOP.ports_100G_CG_MAC[portNum].CG_CONVERTERS.CGMAConvertersExternal_Units_Interrupts_mask = 0x34004c + portFormula;


    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->GOP.ports_100G_CG_MAC[portNum].CGPORTMACCommandConfig = 0x340408 + portFormula;
    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->GOP.ports_100G_CG_MAC[portNum].CGPORTMACaddress0 = 0x34040c + portFormula;
    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->GOP.ports_100G_CG_MAC[portNum].CGPORTMACaddress1 = 0x340410 + portFormula;
    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->GOP.ports_100G_CG_MAC[portNum].CGPORTMACFrmLength = 0x340414 + portFormula;
    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->GOP.ports_100G_CG_MAC[portNum].CGPORTMACRxCrcOpt = 0x340448 + portFormula;
    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->GOP.ports_100G_CG_MAC[portNum].CGPORTMACStatus = 0x340440 + portFormula;
    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->GOP.ports_100G_CG_MAC[portNum].CGPORTMACStatNConfig = 0x340478 + portFormula;
    /* CG MIBS */
    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->GOP.ports_100G_CG_MAC[portNum].CG_MIBs.ifOutOctets = 0x3404D0 + portFormula;
    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->GOP.ports_100G_CG_MAC[portNum].CG_MIBs.ifOutMulticastPkts = 0x340510 + portFormula;
    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->GOP.ports_100G_CG_MAC[portNum].CG_MIBs.ifOutErrors = 0x3404F8 + portFormula;
    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->GOP.ports_100G_CG_MAC[portNum].CG_MIBs.ifOutUcastPkts = 0x340508 + portFormula;
    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->GOP.ports_100G_CG_MAC[portNum].CG_MIBs.ifOutBroadcastPkts = 0x340518 + portFormula;
    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->GOP.ports_100G_CG_MAC[portNum].CG_MIBs.ifInOctets = 0x3404D8 + portFormula;
    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->GOP.ports_100G_CG_MAC[portNum].CG_MIBs.ifInErrors = 0x340590 + portFormula;
    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->GOP.ports_100G_CG_MAC[portNum].CG_MIBs.ifInBroadcastPkts = 0x3404F0 + portFormula;
    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->GOP.ports_100G_CG_MAC[portNum].CG_MIBs.ifInMulticastPkts = 0x3404E8 + portFormula;
    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->GOP.ports_100G_CG_MAC[portNum].CG_MIBs.ifInUcastPkts = 0x3404E0 + portFormula;
    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->GOP.ports_100G_CG_MAC[portNum].CG_MIBs.aFramesTransmittedOK = 0x340480 + portFormula;
    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->GOP.ports_100G_CG_MAC[portNum].CG_MIBs.aFramesReceivedOK = 0x340488 + portFormula;
    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->GOP.ports_100G_CG_MAC[portNum].CG_MIBs.aFrameCheckSequenceErrors = 0x340490 + portFormula;
    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->GOP.ports_100G_CG_MAC[portNum].CG_MIBs.aMACControlFramesTransmitted = 0x340618 + portFormula;
    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->GOP.ports_100G_CG_MAC[portNum].CG_MIBs.aMACControlFramesReceived = 0x340620 + portFormula;
    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->GOP.ports_100G_CG_MAC[portNum].CG_MIBs.aPAUSEMACCtrlFramesTransmitted = 0x3404A0 + portFormula;
    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->GOP.ports_100G_CG_MAC[portNum].CG_MIBs.aPAUSEMACCtrlFramesReceived = 0x3404A8 + portFormula;
    for(ii = 0 ; ii < 8 ; ii++)
    {
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->GOP.ports_100G_CG_MAC[portNum].CG_MIBs.aCBFCPAUSEFramesTransmitted[ii] = 0x340598 + 0x8*ii + portFormula;
    }
    for(ii = 0 ; ii < 8 ; ii++)
    {
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->GOP.ports_100G_CG_MAC[portNum].CG_MIBs.aCBFCPAUSEFramesReceived[ii] = 0x3405D8 + 0x8*ii + portFormula;
    }
    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->GOP.ports_100G_CG_MAC[portNum].CG_MIBs.etherStatsOctets = 0x340528 + portFormula;
    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->GOP.ports_100G_CG_MAC[portNum].CG_MIBs.etherStatsUndersizePkts = 0x340538 + portFormula;
    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->GOP.ports_100G_CG_MAC[portNum].CG_MIBs.etherStatsOversizePkts = 0x340578 + portFormula;
    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->GOP.ports_100G_CG_MAC[portNum].CG_MIBs.etherStatsJabbers = 0x340580 + portFormula;
    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->GOP.ports_100G_CG_MAC[portNum].CG_MIBs.etherStatsDropEvents = 0x340520 + portFormula;
    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->GOP.ports_100G_CG_MAC[portNum].CG_MIBs.etherStatsFragments = 0x340588 + portFormula;
    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->GOP.ports_100G_CG_MAC[portNum].CG_MIBs.etherStatsPkts = 0x340530 + portFormula;
    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->GOP.ports_100G_CG_MAC[portNum].CG_MIBs.etherStatsPkts64Octets = 0x340540 + portFormula;
    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->GOP.ports_100G_CG_MAC[portNum].CG_MIBs.etherStatsPkts65to127Octets = 0x340548 + portFormula;
    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->GOP.ports_100G_CG_MAC[portNum].CG_MIBs.etherStatsPkts128to255Octets = 0x340550 + portFormula;
    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->GOP.ports_100G_CG_MAC[portNum].CG_MIBs.etherStatsPkts256to511Octets = 0x340558 + portFormula;
    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->GOP.ports_100G_CG_MAC[portNum].CG_MIBs.etherStatsPkts512to1023Octets = 0x340560 + portFormula;
    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->GOP.ports_100G_CG_MAC[portNum].CG_MIBs.etherStatsPkts1024to1518Octets = 0x340568 + portFormula;
    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->GOP.ports_100G_CG_MAC[portNum].CG_MIBs.etherStatsPkts1519toMaxOctets = 0x340570 + portFormula;
    /* CG PCS*/
    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->GOP.ports_100G_CG_PCS[portNum].CGPORTPCSControl1  = 0x300000 + portFormula;

}

/**
* @internal raven_gopMtiInit_ptp function
* @endinternal
*
* @brief   initialize the register DB - PTP unit (TSU) in Raven
*
* @param[in] devObjPtr - pointer to device object.
*            portNum   - port number
*
*/
static void raven_gopMtiInit_ptp
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32 portNum,
    IN GT_U32 portIndex,
    IN GT_U32 channelIndex,
    IN GT_U32 dieIndex,
    IN GT_U32 isCpuPort
)
{
    GT_U32  aa = channelIndex;
    GT_U32  bb = portIndex;
    GT_U32  portFormula, index;

    if(isCpuPort)
    {
        portFormula = 0x00515000;
    }
    else
    {
        /*0x00405000 + 0x80000*a + 0x4000*b: where a (0-1) represents mpf num, where b (0-9) represents mpfs num*/
        portFormula = 0x00405000 + 0x80000*aa + 0x4000*bb;
    }

    portFormula += (0x01000000 * dieIndex);

    {
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->GOP_PTP.PTP[portNum].PTPInterruptCause = 0x00000000 + portFormula;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->GOP_PTP.PTP[portNum].PTPInterruptMask = 0x00000004 + portFormula;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->GOP_PTP.PTP[portNum].PTPGeneralCtrl = 0x00000008 + portFormula;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->GOP_PTP.PTP[portNum].PTPTXTimestampQueue0Reg0 = 0x00000024 + portFormula;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->GOP_PTP.PTP[portNum].PTPTXTimestampQueue0Reg1 = 0x00000028 + portFormula;

        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->GOP_PTP.PTP[portNum].PTPTXTimestampQueue1Reg0 = 0x0000002C + portFormula;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->GOP_PTP.PTP[portNum].PTPTXTimestampQueue1Reg1 = 0x00000030 + portFormula;

        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->GOP_PTP.PTP[portNum].totalPTPPktsCntr = 0x00000220 + portFormula;

        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->GOP_PTP.PTP[portNum].NTPPTPOffsetLow = 0x0000000c + portFormula;

        for(index=0; index<4; index++)
        {
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->GOP_PTP.PTP[portNum].timestampFrameCntrControl[index] = 0x00000090 + 0x4 * index + portFormula;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->GOP_PTP.PTP[portNum].timestampFrameCntr[index] = 0x00000080 + 0x4 * index + portFormula;
        }
    }
}

/**
* @internal raven_gopMtiInit_cpuPort function
* @endinternal
*
* @brief   initialize the register DB - for CPU port of MTI unit  in Raven
*
* @param[in] devObjPtr - pointer to device object.
*            portNum   - port number
*
*/
static void raven_gopMtiInit_cpuPort
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32 portNum,
    IN GT_U32 dieIndex,
    IN GT_U32 globalRavenNum
)
{
    GT_U32  portFormula;

    portFormula = 0x0051C000 + (0x01000000 * dieIndex);

    /* <GOP TAP 1>GOP TAP 1/<MTIP IP CPU WRAPPER>MTIP IP CPU WRAPPER/<MTIP CPU EXT> MTIP CPU EXT/MTIP CPU EXT Units */
    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI_cpu_port[globalRavenNum].MTI_EXT_PORT.portStatus          = 0x00000004  + portFormula; /* 0x0051C004 */
    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI_cpu_port[globalRavenNum].MTI_EXT_PORT.portInterruptCause  = 0x00000008 + portFormula;  /* 0x0051C008 */
    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI_cpu_port[globalRavenNum].MTI_EXT_PORT.portInterruptMask   = 0x0000000C + portFormula;  /* 0x0051C00C */

    /*start of unit MTI_EXT - per channel/die */
    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI_cpu_port[globalRavenNum].MTI_GLOBAL.control               = 0x00000000 + portFormula;  /* 0x0051C000 */
    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI_cpu_port[globalRavenNum].MTI_GLOBAL.globalResetControl    = 0x00000018 + portFormula;  /* 0x0051C018 */

#if 0 /* initialized in smemChtGopMtiInitMacMibCounters */
    portFormula = 0x0051a000 + (0x01000000 * dieIndex);

    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI_cpu_port[globalRavenNum].cpu_MTI_STATS.config   = 0x00000008 + portFormula; /* 0x0051a008 */
    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI_cpu_port[globalRavenNum].cpu_MTI_STATS.control  = 0x0000000C + portFormula; /* 0x0051a00c */
    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI_cpu_port[globalRavenNum].cpu_MTI_STATS.datHiCdc = 0x0000001C + portFormula; /* 0x0051a01c */
    for (ii = 0; ii < 56; ii++)
    {
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI_cpu_port[globalRavenNum].cpu_MTI_STATS.counterCapture[ii] = 0x00000020 + 0x4 * ii + portFormula; /* 0x0051a020 */
    }
#endif
    /*Raven : <GOP TAP 1>GOP TAP 1/<GOP TAP 1> <GOP SERVER>GOP SERVER/<GOP SERVER> <MPFS> MPFS/MPFS Units*/
    portFormula = 0x00516000 + (0x01000000 * dieIndex);
    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI_cpu_port[globalRavenNum].MPFS_CPU.SAUControl = 0x00000100 + portFormula;
    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI_cpu_port[globalRavenNum].MPFS_CPU.SAUStatus  = 0x00000104 + portFormula;


    portFormula = 0x00518000 + (0x01000000 * dieIndex);
    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI_cpu_port[globalRavenNum].MTI_cpuMac.commandConfig = 0x00000008 + portFormula; /* 0x00518008 */
    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI_cpu_port[globalRavenNum].MTI_cpuMac.frmLength     = 0x00000014 + portFormula; /* 0x00518014 */
    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI_cpu_port[globalRavenNum].MTI_cpuMac.macAddr0      = 0x0000000C + portFormula; /* 0x0051800C */
    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI_cpu_port[globalRavenNum].MTI_cpuMac.macAddr1      = 0x00000010 + portFormula; /* 0x00518010 */
    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI_cpu_port[globalRavenNum].MTI_cpuMac.status        = 0x00000040 + portFormula; /* 0x00518040 */

    portFormula = 0x00519000 + (0x01000000 * dieIndex);
    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI_cpu_port[globalRavenNum].MTI_cpuPcs.control1      = 0x00000000 + portFormula; /* 0x00519000 */
    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI_cpu_port[globalRavenNum].MTI_cpuPcs.status1       = 0x00000004 + portFormula; /* 0x00519004 */


}

/**
* @internal smemChtGopMtiInitMacMibCounters function
* @endinternal
*
* @brief   initialize the register DB - MTI unit - MAC mib counters of the port
*
* @param[in] devObjPtr - pointer to device object.
*            portNum   - global port number
*            portIndex - local port in the channel
*            channelIndex - the channel id (in Hawk this is unit index)
*            dieIndex  - the die index (relevant to Raven only)
*            isCpuPort - is this the special 'cpu port'
*/
void smemChtGopMtiInitMacMibCounters
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32 portNum,
    IN GT_U32 portIndex,
    IN GT_U32 channelIndex,
    IN GT_U32 dieIndex,
    IN GT_U32 globalRavenNum,
    IN GT_U32 isCpuPort
)
{
    GT_U32  rx_portFormula;
    GT_U32  tx_portFormula;
    GT_U32  tx_portFormula_additional = 0;
    GT_U32  portBaseAddr;
    GT_U32  portFormula;
    GT_U32  ii;
    GT_U32  MTI_EXTERNAL_representativePort;
    SMEM_CHT_PORT_MTI_STATS_STC *portStatsPtr;
    SMEM_CHT_PORT_MTI_STATS_COMMON_CONFIG_STC *portStatsCommonPtr;
    GT_U32  portSupportDualMacMode = 0;

    if(devObjPtr->portsArr[portNum].portSupportPreemption == GT_TRUE)
    {
        /* the EMAC hold counters at the 'even' indexes and the PMAC hold
           counters in the 'odd' indexes */
        /* the 2 indexes of the port are consecutive for the sake of formula */
        portIndex             *= 2;
        portSupportDualMacMode = 1;
        tx_portFormula_additional = 0x560;/*additional gap due to 16 RX ports ! instead of 8*/
    }

    if(devObjPtr->numOfRavens == 0) /* Hawk */
    {
        GT_U32  unitIndex = channelIndex;
        ENHANCED_PORT_INFO_STC portInfo;
        SMEM_UNIT_TYPE_ENT unitType;
        SKERNEL_PORT_STATE_ENT state;

        state = devObjPtr->portsArr[portNum].state;
        unitType = SMEM_MTI_UNIT_TYPE_GET(devObjPtr,portNum);

        if(state != SKERNEL_PORT_STATE_MTI_CPU_E)
        {
            portBaseAddr = devObjPtr->memUnitBaseAddrInfo.macMib[unitIndex];
        }
        else
        {
            isCpuPort = 1;
            portBaseAddr = devObjPtr->memUnitBaseAddrInfo.cpuMacMib[unitIndex];
        }

        rx_portFormula = (43 * portIndex * 0x4);
        tx_portFormula = (34 * portIndex * 0x4);

        devObjPtr->devMemPortInfoGetPtr(devObjPtr,unitType  ,portNum,&portInfo);
        MTI_EXTERNAL_representativePort = portInfo.sip6_MTI_EXTERNAL_representativePortIndex;
    }
    else
    {
        if(isCpuPort)
        {
            rx_portFormula = 0;
            tx_portFormula = 0;
            portBaseAddr   = 0x0051a000;
            portBaseAddr  += (0x01000000 * dieIndex);
            MTI_EXTERNAL_representativePort =  globalRavenNum;
        }
        else
        {
            rx_portFormula = (43 * portIndex * 0x4);
            tx_portFormula = (34 * portIndex * 0x4);

            portBaseAddr   = 0x00450000;
            portBaseAddr  += (0x01000000 * dieIndex) + (0x80000 * channelIndex);
            MTI_EXTERNAL_representativePort =  portNum/8;
        }
    }

    rx_portFormula += portBaseAddr;
    tx_portFormula += portBaseAddr;

    portStatsPtr       =  &SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI[portNum].MTI_STATS;
    portStatsCommonPtr =  isCpuPort ? &SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI_cpu_port[MTI_EXTERNAL_representativePort].cpu_MTI_STATS:
                                      &SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI_EXTERNAL[MTI_EXTERNAL_representativePort].MTI_STATS;

    if(portStatsCommonPtr->config == 0xffffffff/*not initialized yet*/)/* per 8 ports */
    {
        portStatsCommonPtr->config   = 0x00000008 + portBaseAddr;
        portStatsCommonPtr->control  = 0x0000000C + portBaseAddr;
        portStatsCommonPtr->datHiCdc = 0x0000001C + portBaseAddr;
        for (ii = 0; ii < 56; ii++)
        {
            portStatsCommonPtr->counterCapture[ii] = 0x00000020 + 0x4 * ii + portBaseAddr;
        }
    }

doPreemptionInit_lbl:

    /* Statistic RX counters */
    portFormula = rx_portFormula;
    if(portStatsPtr->RX_etherStatsOctets != 0xffffffff/*already initialized */)
    {
        /* currently the Hawk gets here for USX and for MAC 400G , on the same port */
        return;
    }
    portStatsPtr->RX_etherStatsOctets                          = 0x00000100 + portFormula; /* 0x00450100 */
    portStatsPtr->RX_OctetsReceivedOK                          = 0x00000104 + portFormula; /* 0x00450104 */
    portStatsPtr->RX_aAlignmentErrors                          = 0x00000108 + portFormula; /* 0x00450108 */
    portStatsPtr->RX_aPauseMacCtrlFramesReceived               = 0x0000010c + portFormula; /* 0x0045010c */
    portStatsPtr->RX_aFrameTooLong                             = 0x00000110 + portFormula; /* 0x00450110 */
    portStatsPtr->RX_aInRangeLengthError                       = 0x00000114 + portFormula; /* 0x00450114 */
    portStatsPtr->RX_aFramesReceivedOK                         = 0x00000118 + portFormula; /* 0x00450118 */
    portStatsPtr->RX_aFrameCheckSequenceErrors                 = 0x0000011C + portFormula; /* 0x0045011C */
    portStatsPtr->RX_VLANReceivedOK                            = 0x00000120 + portFormula; /* 0x00450120 */
    portStatsPtr->RX_ifInErrors                                = 0x00000124 + portFormula; /* 0x00450124 */
    portStatsPtr->RX_ifInUcastPkts                             = 0x00000128 + portFormula; /* 0x00450128 */
    portStatsPtr->RX_ifInMulticastPkts                         = 0x0000012C + portFormula; /* 0x0045012C */
    portStatsPtr->RX_ifInBroadcastPkts                         = 0x00000130 + portFormula; /* 0x00450130 */
    portStatsPtr->RX_etherStatsDropEvents                      = 0x00000134 + portFormula; /* 0x00450134 */
    portStatsPtr->RX_etherStatsPkts                            = 0x00000138 + portFormula; /* 0x00450138 */
    portStatsPtr->RX_etherStatsUndersizePkts                   = 0x0000013C + portFormula; /* 0x0045013C */
    portStatsPtr->RX_etherStatsPkts64Octets                    = 0x00000140 + portFormula; /* 0x00450140 */
    portStatsPtr->RX_etherStatsPkts65to127Octets               = 0x00000144 + portFormula; /* 0x00450144 */
    portStatsPtr->RX_etherStatsPkts128to255Octets              = 0x00000148 + portFormula; /* 0x00450148 */
    portStatsPtr->RX_etherStatsPkts256to511Octets              = 0x0000014C + portFormula; /* 0x0045014C */
    portStatsPtr->RX_etherStatsPkts512to1023Octets             = 0x00000150 + portFormula; /* 0x00450150 */
    portStatsPtr->RX_etherStatsPkts1024to1518Octets            = 0x00000154 + portFormula; /* 0x00450154 */
    portStatsPtr->RX_etherStatsPkts1519toMaxOctets             = 0x00000158 + portFormula; /* 0x00450158 */
    portStatsPtr->RX_etherStatsOversizePkts                    = 0x0000015C + portFormula; /* 0x0045015C */
    portStatsPtr->RX_etherStatsJabbers                         = 0x00000160 + portFormula; /* 0x00450160 */
    portStatsPtr->RX_etherStatsFragments                       = 0x00000164 + portFormula; /* 0x00450164 */
    for (ii = 0; ii < 15; ii++)
    {
        portStatsPtr->RX_aCBFCPAUSEFramesReceived[ii]          = 0x00000168 + 0x4 * ii + portFormula; /* 0x00450168 */
    }
    portStatsPtr->RX_aMACControlFramesReceived                 = 0x000001A8 + portFormula; /* 0x004501A8 */


    /* Statistic TX counters */
    portFormula = tx_portFormula + tx_portFormula_additional;
    portStatsPtr->TX_etherStatsOctets                          = 0x00000660 + portFormula; /* 0x00450660 */
    portStatsPtr->TX_OctetsTransmittedOK                       = 0x00000664 + portFormula; /* 0x00450664 */
    portStatsPtr->TX_aPauseMacCtrlFramesTransmitted            = 0x00000668 + portFormula; /* 0x00450668 */
    portStatsPtr->TX_aFramesTransmittedOK                      = 0x0000066C + portFormula; /* 0x0045066C */
    portStatsPtr->TX_VLANTransmittedOK                         = 0x00000670 + portFormula; /* 0x00450670 */
    portStatsPtr->TX_ifOutErrors                               = 0x00000674 + portFormula; /* 0x00450674 */
    portStatsPtr->TX_ifOutUcastPkts                            = 0x00000678 + portFormula; /* 0x00450678 */
    portStatsPtr->TX_ifOutMulticastPkts                        = 0x0000067C + portFormula; /* 0x0045067C */
    portStatsPtr->TX_ifOutBroadcastPkts                        = 0x00000680 + portFormula; /* 0x00450680 */
    portStatsPtr->TX_etherStatsPkts64Octets                    = 0x00000684 + portFormula; /* 0x00450684 */
    portStatsPtr->TX_etherStatsPkts65to127Octets               = 0x00000688 + portFormula; /* 0x00450688 */
    portStatsPtr->TX_etherStatsPkts128to255Octets              = 0x0000068C + portFormula; /* 0x0045068C */
    portStatsPtr->TX_etherStatsPkts256to511Octets              = 0x00000690 + portFormula; /* 0x00450690 */
    portStatsPtr->TX_etherStatsPkts512to1023Octets             = 0x00000694 + portFormula; /* 0x00450694 */
    portStatsPtr->TX_etherStatsPkts1024to1518Octets            = 0x00000698 + portFormula; /* 0x00450698 */
    portStatsPtr->TX_etherStatsPkts1519toMaxOctets             = 0x0000069C + portFormula; /* 0x0045069C */
    for (ii = 0; ii < 15; ii++)
    {
        portStatsPtr->TX_aCBFCPAUSEFramesTransmitted[ii]       = 0x000006A0 + 0x4 * ii + portFormula; /*0x004506A0*/
    }

    portStatsPtr->TX_aMACControlFramesTransmitted              = 0x000006E0 + portFormula; /* 0x004506E0 */
    portStatsPtr->TX_etherStatsPkts                            = 0x000006E4 + portFormula; /* 0x004506E4 */

    if(portSupportDualMacMode == 1)
    {
        portSupportDualMacMode = 2;

        portIndex ++;/* now run with start with the odd index for the 'PMAC' */

        rx_portFormula = portBaseAddr + (43 * portIndex * 0x4);
        tx_portFormula = portBaseAddr + (34 * portIndex * 0x4);

        portStatsPtr       =  &SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI[portNum].MTI_PREEMPTION_STATS;

        goto doPreemptionInit_lbl;
    }

}

/**
* @internal raven_gopMtiInit function
* @endinternal
*
* @brief   initialize the register DB - for port of MTI unit  in Raven
*
* @param[in] devObjPtr - pointer to device object.
*            portNum   - port number
*
*/
static void raven_gopMtiInit
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32 portNum
)
{
    GT_U32 portIndex;
    GT_U32 channelIndex;
    GT_U32 dieIndex,globalRavenNum;
    GT_U32 portFormula;
    GT_U32 isPort_200_400 = 0;
    GT_U32 isCpuPort;

    /* Convert physical port to Raven port, channel and die indexes */
    smemRavenGlobalPortConvertGet(devObjPtr,portNum,&globalRavenNum,&dieIndex,NULL,&portIndex,&channelIndex,&isCpuPort);

    /* do PTP registers */
    raven_gopMtiInit_ptp(devObjPtr,portNum ,portIndex , channelIndex , dieIndex , isCpuPort);
    /* do mib counters */
    smemChtGopMtiInitMacMibCounters(devObjPtr,portNum ,portIndex , channelIndex , dieIndex , globalRavenNum , isCpuPort);

    if(isCpuPort)
    {
        raven_gopMtiInit_cpuPort(devObjPtr,portNum,dieIndex,globalRavenNum);
        return;
    }

    if ((portIndex % 4) == 0)
    {
        isPort_200_400 = 1;
    }
#define CHANNEL_FORMULA         ((0x80000 * channelIndex) + (0x01000000 * dieIndex))
#define PORT_FORMULA(offset)    ((offset) + CHANNEL_FORMULA)

    /* Set formula per 50/100 ports*/
    portFormula = PORT_FORMULA((0x18 * portIndex));
    /* start of unit MTI_EXT - per channel/port*/
    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI[portNum].MTI_EXT_PORT.portStatus = 0x00478088  + portFormula;
    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI[portNum].MTI_EXT_PORT.portInterruptCause = 0x00478094 + portFormula;
    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI[portNum].MTI_EXT_PORT.portInterruptMask = 0x00478098 + portFormula;

    /*start of unit MTI_EXT - per channel/die */
    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI_EXTERNAL[portNum/8].MTI_GLOBAL.globalChannelControl = 0x00478008 + CHANNEL_FORMULA;
    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI_EXTERNAL[portNum/8].MTI_GLOBAL.globalResetControl = 0x00478014 + CHANNEL_FORMULA;
    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI_EXTERNAL[portNum/8].MTI_GLOBAL.globalInterruptSummaryCause = 0x0047804C + CHANNEL_FORMULA;
    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI_EXTERNAL[portNum/8].MTI_GLOBAL.globalInterruptSummaryMask = 0x00478050 + CHANNEL_FORMULA;

    /* FEC */
    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI_EXTERNAL[portNum/8].MTI_GLOBAL.globalFecCeInterruptCause = 0x0047803C + CHANNEL_FORMULA;
    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI_EXTERNAL[portNum/8].MTI_GLOBAL.globalFecCeInterruptMask = 0x00478040 + CHANNEL_FORMULA;
    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI_EXTERNAL[portNum/8].MTI_GLOBAL.globalFecNceInterruptCause = 0x00478044 + CHANNEL_FORMULA;
    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI_EXTERNAL[portNum/8].MTI_GLOBAL.globalFecNceInterruptMask = 0x00478048 + CHANNEL_FORMULA;

    /* info of PCS that is per representavive port */
    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI_EXTERNAL[portNum/8].PCS_common.LPCS_common.gmode = 0x004743E0 + CHANNEL_FORMULA;


    /* Set formula per 50/100 ports */
    portFormula = PORT_FORMULA((0x1000 * portIndex));
    /* start of unit MTI_WRAPPER - per channel/port: 0, 4 */
    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI[portNum].MTI_WRAPPER.MTI64_MAC.commandConfig = 0x00444008 + portFormula;
    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI[portNum].MTI_WRAPPER.MTI64_MAC.frmLength = 0x00444014 + portFormula;
    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI[portNum].MTI_WRAPPER.MTI64_MAC.macAddr0 = 0x0044400C + portFormula;
    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI[portNum].MTI_WRAPPER.MTI64_MAC.macAddr1 = 0x00444010 + portFormula;
    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI[portNum].MTI_WRAPPER.MTI64_MAC.status = 0x00444040 + portFormula;

    if (portIndex % 2)
    {
        /* Set formula per odd ports */
        portFormula = PORT_FORMULA((0x1000 * (portIndex - 1)));
        /* start of unit MTI_WRAPPER - per channel/port: 1, 3, .., 31 */
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI[portNum].MTI_WRAPPER.PCS_10254050.control1 = 0x00465000 + portFormula;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI[portNum].MTI_WRAPPER.PCS_10254050.status1 = 0x00465004 + portFormula;
    }
    else
    {
        /* Set formula per even ports */
        portFormula = PORT_FORMULA((0x1000 * portIndex));
        /* start of unit MTI_WRAPPER - per channel/port: 0, 2, .. , 30 */
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI[portNum].MTI_WRAPPER.PCS_10TO100.control1 = 0x00464000 + portFormula;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI[portNum].MTI_WRAPPER.PCS_10TO100.status1 = 0x00464004 + portFormula;
    }

    portFormula = PORT_FORMULA((0x80 * portIndex));
    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI[portNum].MTI_WRAPPER.PCS_LPCS.control1 = 0x00474000 + portFormula;
    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI[portNum].MTI_WRAPPER.PCS_LPCS.status1  = 0x00474004 + portFormula;

    portFormula = PORT_FORMULA((0x4000 * portIndex));
    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI[portNum].MPFS.SAUControl = 0x00406100 + portFormula;
    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI[portNum].MPFS.SAUStatus  = 0x00406104 + portFormula;

    if(isPort_200_400)
    {
        /* note : HWS_MPF_CHANNEL_OFFSET and logic of 8,9 taken from HWS */
        #define HWS_MPF_CHANNEL_OFFSET  0x4000
        #define SET_HWS_MPF_HIGH_SPEED_REG(dev,regName,portNum,portIndex)    \
            SMEM_CHT_MAC_REG_DB_SIP5_GET(dev)->sip6_high_speed[portNum].MPFS.regName = \
                (SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI[portNum].MPFS.regName + \
                    ((portIndex == 0) ? 8*HWS_MPF_CHANNEL_OFFSET : \
                                5/*not 9!*/ *HWS_MPF_CHANNEL_OFFSET)/*port 4 already hold offset 4*HWS_MPF_CHANNEL_OFFSET from port 0*/)

        SET_HWS_MPF_HIGH_SPEED_REG(devObjPtr,SAUControl,portNum,portIndex);
        SET_HWS_MPF_HIGH_SPEED_REG(devObjPtr,SAUStatus ,portNum,portIndex);

        portFormula = PORT_FORMULA((0x1000 * (portIndex / 4)));
        /*port 0 : 0x00460000 + j*0x80000: where j (0-1) represents 8channels instance number*/
        /*port 4 : 0x00461000 + j*0x80000: where j (0-1) represents 8channels instance number*/
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_high_speed[portNum].PCS_400G_200G.control1 = 0x00460000 + portFormula;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_high_speed[portNum].PCS_400G_200G.status1  = 0x00460004 + portFormula;

        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_high_speed[portNum].MTI400_MAC.commandConfig = 0x00440008 + portFormula;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_high_speed[portNum].MTI400_MAC.frmLength = 0x00440014 + portFormula;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_high_speed[portNum].MTI400_MAC.macAddr0 = 0x0044000C + portFormula;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_high_speed[portNum].MTI400_MAC.macAddr1 = 0x00440010 + portFormula;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_high_speed[portNum].MTI400_MAC.status = 0x00440040 + portFormula;

        /* Set formula per 200/400 ports*/
        portFormula = PORT_FORMULA((0x14 * (portIndex / 4)));
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI[portNum].MTI_EXT_PORT.segPortStatus = 0x00478060 + portFormula;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI[portNum].MTI_EXT_PORT.segPortInterruptCause = 0x00478068 + portFormula;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI[portNum].MTI_EXT_PORT.segPortInterruptMask = 0x0047806C + portFormula;
    }

/*    gopMtiPrint(devObjPtr, portNum); */
}

/* copy DP unit registers from DP[0] to DP[trgDp] */
#define DUPLICATE_DP_UNIT_ADDR_FROM_DP0_MAC(dpUnit,trgDp)       \
    memcpy(&dpUnit[trgDp].startUnitInfo + 1,                    \
           &dpUnit[0].startUnitInfo + 1,                        \
           sizeof(dpUnit[0]) - sizeof(SMEM_CHT_PP_REGS_UNIT_START_INFO_STC))

/* copy pipe unit registers from pipe[0] to pipe[trgDp] */
#define DUPLICATE_PIPE_UNIT_ADDR_FROM_PIPE0_MAC(pipeUnit,trgPipe) \
    memcpy(&pipeUnit[trgPipe].startUnitInfo + 1,                  \
           &pipeUnit[0].startUnitInfo + 1,                        \
           sizeof(pipeUnit[0]) - sizeof(SMEM_CHT_PP_REGS_UNIT_START_INFO_STC))


/**
* @internal smemSip5RegsInfoSet function
* @endinternal
*
* @brief   Init memory module for Sip5 DB devices.
*
* @param[in] devObjPtr                - pointer to device object.
*/
void smemSip5RegsInfoSet
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr
)
{
    GT_U32  plrUnit;/*PLR unit id*/
    GT_U32  isSip5_15 = SMEM_CHT_IS_SIP5_15_GET(devObjPtr);
    GT_U32  isSip5_20 = SMEM_CHT_IS_SIP5_20_GET(devObjPtr);
    GT_U32  isSip5_25 = SMEM_CHT_IS_SIP5_25_GET(devObjPtr);
    GT_U32  isSip6    = SMEM_CHT_IS_SIP6_GET(devObjPtr);
    GT_U32  isSip6_10 = SMEM_CHT_IS_SIP6_10_GET(devObjPtr);
    GT_U32  isSip6_30 = SMEM_CHT_IS_SIP6_30_GET(devObjPtr);
    GT_U32  unitIndex;/*unit index*/
    GT_BIT  isDxControlPipe = SMEM_IS_PIPE_FAMILY_GET(devObjPtr) ? 0 : 1;
    GT_U32  isPipeDevice = SMEM_IS_PIPE_FAMILY_GET(devObjPtr) ? 1 : 0;
    GT_U32  txqNumPorts        = devObjPtr->txqNumPorts ? devObjPtr->txqNumPorts : 72;
    GT_U32  shtNumPorts        = SMEM_CHT_IS_SIP5_25_GET(devObjPtr) ? 128 :
                                 SMEM_CHT_IS_SIP5_20_GET(devObjPtr) ? 512 :
                                 256;

    if(isDxControlPipe)
    {/*start of unit TTI */     /* manually added : TTI_Internal */

        {/*start of unit TTI_Internal */
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TTI.TTI_Internal.MetalFixRegister  = 0x000013F8;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TTI.TTI_Internal.TTIInternalConfig = 0x000013FC;
        }/*end of unit TTI_Internal */

        {/*start of unit VLANAssignment */
            {/*300+i*4*/
                GT_U32    i;
                for(i = 0 ; i <= 3 ; i++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TTI.VLANAssignment.ingrTPIDConfig_[i] =
                        0x300+i*4;
                }/* end of loop i */
            }/*300+i*4*/
            {/*310+(0x8*t)+(0x4*p)*/
                GT_U32    t,p;
                for(t = 0 ; t <= 1 ; t++) {
                    for(p = 0 ; p <= 1 ; p++) {
                        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TTI.VLANAssignment.ingrTagProfileTPIDSelect[t][p] =
                            0x310+(0x8*t)+(0x4*p);
                    }/* end of loop p */
                }/* end of loop t */
            }/*310+(0x8*t)+(0x4*p)*/
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TTI.VLANAssignment.ingrTPIDTagType = 0x00000320;
            if(isSip6_10)
            {
                {/*350+(0x4*t)*/
                    GT_U32    t;
                    for(t = 0 ; t <= 7 ; t++) {
                        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TTI.VLANAssignment.ingrTPIDEnhClasification[t] =
                            0x350+(0x4*t);
                    }/* end of loop t */
                }/*350+(0x4*t)*/
                {/*370+(0x4*t)*/
                    GT_U32    t;
                    for(t = 0 ; t <= 7 ; t++) {
                        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TTI.VLANAssignment.ingrTPIDEnhClasificationExt[t] =
                            0x370+(0x4*t);
                    }/* end of loop t */
                }/*370+(0x4*t)*/
                {/*390+(0x4*t)*/
                    GT_U32    t;
                    for(t = 0 ; t <= 7 ; t++) {
                        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TTI.VLANAssignment.ingrTPIDEnhClasificationExt2[t] =
                            0x390+(0x4*t);
                    }/* end of loop t */
                }/*390+(0x4*t)*/
                {/*3B0+(0x4*t)*/
                    GT_U32    t;
                    for(t = 0 ; t <= 7 ; t++) {
                        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TTI.VLANAssignment.ingrTPIDEnhClasificationExt3[t] =
                            0x3B0+(0x4*t);
                    }/* end of loop t */
                }/*3B0+(0x4*t)*/
            }
        }/*end of unit VLANAssignment */


        {/*start of unit userDefinedBytes */
            if (isSip6_10)
            {
                {/*0x2C00+n*0x4*/
                    GT_U32    n;
                    for(n = 0 ; n <= 63 ; n++) {
                        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TTI.userDefinedBytes.IPCLTCPPortRangeComparatorConfig[n] =
                            0x2C00+n*0x4;
                    }/* end of loop n */
                }/*0x2C00+n*0x4*/
                {/*0x2E00+n*0x4*/
                    GT_U32    n;
                    for(n = 0 ; n <= 63 ; n++) {
                        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TTI.userDefinedBytes.IPCLUDPPortRangeComparatorConfig[n] =
                            0x2E00+n*0x4;
                    }/* end of loop n */
                }/*0x2E00+n*0x4*/
            }
            else
            {
                {/*b0+n*0x4*/
                    GT_U32    n;
                    for(n = 0 ; n <= 7 ; n++) {
                        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TTI.userDefinedBytes.IPCLTCPPortRangeComparatorConfig[n] =
                            0xb0+n*0x4;
                    }/* end of loop n */
                }/*b0+n*0x4*/
                {/*d0+n*0x4*/
                    GT_U32    n;
                    for(n = 0 ; n <= 7 ; n++) {
                        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TTI.userDefinedBytes.IPCLUDPPortRangeComparatorConfig[n] =
                            0xd0+n*0x4;
                    }/* end of loop n */
                }/*d0+n*0x4*/
            }

        }/*end of unit userDefinedBytes */


        {/*start of unit TTIUnitGlobalConfigs */
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TTI.TTIUnitGlobalConfigs.TTIUnitGlobalConfig = 0x00000000;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TTI.TTIUnitGlobalConfigs.TTIUnitGlobalConfigForCCPorts = 0x00000064;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TTI.TTIUnitGlobalConfigs.TTIUnitGlobalConfigExt = 0x00000140;

            if (SMEM_CHT_IS_SIP6_GET(devObjPtr))
            {
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TTI.TTIUnitGlobalConfigs.TTIUnitGlobalConfigExt2 = 0x000001E8;
            }
            if (SMEM_CHT_IS_SIP6_30_GET(devObjPtr))
            {
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TTI.TTIUnitGlobalConfigs.TTIUnitGlobalConfigExt3 = 0x000001EC;
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TTI.TTIUnitGlobalConfigs.hsrPrpGlobalConfig0 = 0x00000220;
            }

        }/*end of unit TTIUnitGlobalConfigs */


        {/*start of unit TTIEngine */
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TTI.TTIEngine.TTIEngineInterruptCause = 0x00000004;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TTI.TTIEngine.TTIEngineInterruptMask = 0x00000008;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TTI.TTIEngine.TTIEngineConfig = 0x0000000c;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TTI.TTIEngine.TTIPCLIDConfig0 = 0x00000010;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TTI.TTIEngine.TTIPCLIDConfig1 = 0x00000014;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TTI.TTIEngine.TTILookupIPExceptionCommands = 0x00000068;
            {/*120+0x4*p*/
                GT_U32    p;
                for(p = 0 ; p <= 7 ; p++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TTI.TTIEngine.TTIPktTypeKeys[p] =
                        0x120+0x4*p;
                }/* end of loop p */
            }/*120+0x4*p*/
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TTI.TTIEngine.TTILookupIPExceptionCodes0 = 0x00000110;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TTI.TTIEngine.TTIKeySegmentMode0 = 0x00000114;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TTI.TTIEngine.TTIKeyType = 0x00000118;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TTI.TTIEngine.TTILookupMPLSExceptionCommands = 0x0000011c;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TTI.TTIEngine.TTILookupIPExceptionCodes1 = 0x00000144;
            if (isSip6)
            {/*250+0x4*p*/
                GT_U32    p;
                for(p = 0 ; p <= 9 ; p++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TTI.TTIEngine.TTIPacketTypeExactMatchProfileId[p] = 0x250+0x4*p;
                }/* end of loop p */
            }/*0x250 + 0x4*p: where p (0-9) represents packet types pairs*/
            if (isSip6_10)
            {
                {/*2700+0x4*p*/
                    GT_U32    p;
                    for(p = 0 ; p <= 127 ; p++) {
                        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TTI.TTIEngine.TTIExactMatchProfileId1Mapping[p] =
                            0x2700+0x4*p;
                    }/* end of loop p */
                }/*2700+0x4*p: where p  (0-127) represents remote port */
                {/*2900+0x4*p*/
                    GT_U32    p;
                    for(p = 0 ; p <= 127 ; p++) {
                        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TTI.TTIEngine.TTIExactMatchProfileId2Mapping[p] =
                            0x2900+0x4*p;
                    }/* end of loop p */
                }/*2900+0x4*p: where p  (0-127) represents remote port */
            }

        }/*end of unit TTIEngine */


        {/*start of unit trunkHashSimpleHash */
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TTI.trunkHashSimpleHash.trunkHashConfigReg0 = 0x00000084;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TTI.trunkHashSimpleHash.trunkHashConfigReg1 = 0x00000088;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TTI.trunkHashSimpleHash.trunkHashConfigReg2 = 0x0000008c;

        }/*end of unit trunkHashSimpleHash */


        {/*start of unit TRILLConfigs */
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TTI.TRILLConfigs.TRILLGlobalConfigs0 = 0x00000200;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TTI.TRILLConfigs.TRILLGlobalConfigs1 = 0x00000204;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TTI.TRILLConfigs.TRILLExceptionCommands0 = 0x00000208;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TTI.TRILLConfigs.TRILLExceptionCommands1 = 0x0000020c;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TTI.TRILLConfigs.TRILLGlobalConfigs2 = 0x00000210;

        }/*end of unit TRILLConfigs */

        if (!SMEM_CHT_IS_SIP6_30_GET(devObjPtr))
        {/*start of unit TRILLAdjacency */
            {/*6008+e*0xc*/
                GT_U32    e;
                for(e = 0 ; e <= 127 ; e++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TTI.TRILLAdjacency.adjacencyEntryReg2[e] =
                        0x6008+e*0xc;
                }/* end of loop e */
            }/*6008+e*0xc*/
            {/*6004+0xc*e*/
                GT_U32    e;
                for(e = 0 ; e <= 127 ; e++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TTI.TRILLAdjacency.adjacencyEntryReg1[e] =
                        0x6004+0xc*e;
                }/* end of loop e */
            }/*6004+0xc*e*/
            {/*6000+e*0xc*/
                GT_U32    e;
                for(e = 0 ; e <= 127 ; e++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TTI.TRILLAdjacency.adjacencyEntryReg0[e] =
                        0x6000+e*0xc;
                }/* end of loop e */
            }/*6000+e*0xc*/

        }/*end of unit TRILLAdjacency */


        {/*start of unit PTP */
            {/*3040+d*0x10+i*0x4*/
                GT_U32    i,d;
                for(i = 0 ; i <= 3 ; i++) {
                    for(d = 0 ; d <= 3 ; d++) {
                        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TTI.PTP._1588V1DomainDomainNumber[i][d] =
                            0x3040+d*0x10+i*0x4;
                    }/* end of loop d */
                }/* end of loop i */
            }/*3040+d*0x10+i*0x4*/
            {/*309c+0x4*d*/
                GT_U32    d;
                for(d = 0 ; d <= 4 ; d++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TTI.PTP.PTPExceptionCheckingModeDomain[d] =
                        0x309c+0x4*d;
                }/* end of loop d */
            }/*309c+0x4*d*/
            if(! SMEM_CHT_IS_SIP5_20_GET(devObjPtr))
            {/*3010+0x4*n*/
                GT_U32    n;
                for(n = 0 ; n <= 7 ; n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TTI.PTP.piggyBackTSConfigs[n] =
                        0x3010+0x4*n;
                }/* end of loop n */
            }/*3010+0x4*n*/
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TTI.PTP.PTPConfigs = 0x00003030;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TTI.PTP.PTPOverUDPDestinationPorts = 0x00003004;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TTI.PTP.PTPExceptionsAndCPUCodeConfig = 0x0000300c;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TTI.PTP.PTPEtherTypes = 0x00003000;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TTI.PTP.PTPExceptionsCntr = 0x00003034;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TTI.PTP._1588V2DomainDomainNumber = 0x00003090;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TTI.PTP.timestampEtherTypes = 0x00003094;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TTI.PTP.timestampConfigs = 0x00003098;

        }/*end of unit PTP */


        {/*start of unit pseudoWire */
            {/*1504+0x4*n*/
                GT_U32    n;
                for(n = 0 ; n <= 14 ; n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TTI.pseudoWire.MPLSOAMChannelTypeProfile[n] =
                        0x1504+0x4*n;
                }/* end of loop n */
            }/*1504+0x4*n*/
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TTI.pseudoWire.PWE3Config = 0x00001500;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TTI.pseudoWire.PWTagModeExceptionConfig = 0x00001540;

        }/*end of unit pseudoWire */


        {/*start of unit protocolMatch */
            {/*90+n*0x4*/
                GT_U32    n;
                for(n = 0 ; n <= 5 ; n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TTI.protocolMatch.protocolsConfig[n] =
                        0x90+n*0x4;
                }/* end of loop n */
            }/*90+n*0x4*/
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TTI.protocolMatch.protocolsEncapsulationConfig0 = 0x000000a8;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TTI.protocolMatch.protocolsEncapsulationConfig1 = 0x000000ac;

        }/*end of unit protocolMatch */

        if(! SMEM_CHT_IS_SIP5_20_GET(devObjPtr))
        {/*start of unit portMap */
            {/*10b0+4*n*/
                GT_U32    n;
                for(n = 0 ; n <= 31 ; n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TTI.portMap.portGroupMap[n] =
                        0x10b0+4*n;
                }/* end of loop n */
            }/*10b0+4*n*/
            {/*1000+n*4*/
                GT_U32    n;
                for(n = 0 ; n <= 42 ; n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TTI.portMap.portListMap[n] =
                        0x1000+n*4;
                }/* end of loop n */
            }/*1000+n*4*/

        }/*end of unit portMap */


         {/*start of unit phyPortAndEPortAttributes */
            {/*110004+p*4*/
                GT_U32    p;
                GT_U32    numPorts = isSip6_10 ? 74  :
                                     isSip6    ? 287 :
                                     255;
                if(devObjPtr->TTI_myPhysicalPortAttributes_numEntries)
                {
                    numPorts = devObjPtr->TTI_myPhysicalPortAttributes_numEntries - 1;
                }
                for(p = 0 ; p <= numPorts ; p++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TTI.phyPortAndEPortAttributes.myPhysicalPortAttributes[p] = 0x110004+p*4;
                }/* end of loop p */
            }/*110004+p*4*/
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TTI.phyPortAndEPortAttributes.remotePhysicalPortAssignmentGlobalConfig = 0x00110000;
        }/*end of unit phyPortAndEPortAttributes */
         if (isSip6)
         {
             SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TTI.phyPortAndEPortAttributes.remotePhysicalPortAssignmentGlobalConfigExt = 0x00110500;
         }


        {/*start of unit MRUException */
            {/*170+n*4*/
                GT_U32    n;
                for(n = 0 ; n <= 3 ; n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TTI.MRUException.MRUSize[n] = 0x170+n*4;
                }/* end of loop n */
            }/*170+n*4*/
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TTI.MRUException.MRUCommandAndCode = 0x00000180;

        }/*end of unit MRUException */



        {/*start of unit MAC2ME */

            if(!SMEM_CHT_IS_SIP5_20_GET(devObjPtr))
            {/*start of unit MAC2METCAMRegs */
                {/*2600+n*4*/
                    GT_U32    n;
                    for(n = 0 ; n <= 2 ; n++) {
                        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TTI.MAC2ME.MAC2METCAMRegs.MGLookupToInternalTCAMData[n] =
                            0x2600+n*4;
                    }/* end of loop n */
                }/*2600+n*4*/
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TTI.MAC2ME.MAC2METCAMRegs.MGLookupToInternalTCAMCtrl = 0x0000260c;
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TTI.MAC2ME.MAC2METCAMRegs.MGLookupToInternalTCAMResult = 0x00002610;

            }/*end of unit MAC2METCAMRegs */


            {/*160c+n*0x20*/
                GT_U32    n;
                for(n = 0 ; n <= 127 ; n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TTI.MAC2ME.MAC2MEHighMask[n] =
                        0x160c+n*0x20;
                }/* end of loop n */
            }/*160c+n*0x20*/
            {/*1600+n*0x20*/
                GT_U32    n;
                for(n = 0 ; n <= 127 ; n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TTI.MAC2ME.MAC2MELow[n] =
                        0x1600+n*0x20;
                }/* end of loop n */
            }/*1600+n*0x20*/
            {/*1604+n*0x20*/
                GT_U32    n;
                for(n = 0 ; n <= 127 ; n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TTI.MAC2ME.MAC2MEHigh[n] =
                        0x1604+n*0x20;
                }/* end of loop n */
            }/*1604+n*0x20*/
            {/*1608+n*0x20*/
                GT_U32    n;
                for(n = 0 ; n <= 127 ; n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TTI.MAC2ME.MAC2MELowMask[n] =
                        0x1608+n*0x20;
                }/* end of loop n */
            }/*1608+n*0x20*/
            {/*1610+0x20*n*/
                GT_U32    n;
                for(n = 0 ; n <= 127 ; n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TTI.MAC2ME.MAC2MESrcInterface[n] =
                        0x1610+0x20*n;
                }/* end of loop n */
            }/*1610+0x20*n*/
            {/*1614+0x20*n*/
                GT_U32    n;
                for(n = 0 ; n <= 127 ; n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TTI.MAC2ME.MAC2MESrcInterfaceSel[n] =
                        0x1614+0x20*n;
                }/* end of loop n */
            }/*1614+0x20*n*/

        }/*end of unit MAC2ME */


        {/*start of unit IPv6SolicitedNode */
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TTI.IPv6SolicitedNode.IPv6SolicitedNodeMcAddrConfigReg0 = 0x00000040;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TTI.IPv6SolicitedNode.IPv6SolicitedNodeMcAddrConfigReg1 = 0x00000044;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TTI.IPv6SolicitedNode.IPv6SolicitedNodeMcAddrConfigReg2 = 0x00000048;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TTI.IPv6SolicitedNode.IPv6SolicitedNodeMcAddrConfigReg3 = 0x0000004c;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TTI.IPv6SolicitedNode.IPv6SolicitedNodeMcAddrMaskReg0 = 0x00000050;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TTI.IPv6SolicitedNode.IPv6SolicitedNodeMcAddrMaskReg1 = 0x00000054;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TTI.IPv6SolicitedNode.IPv6SolicitedNodeMcAddrMaskReg2 = 0x00000058;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TTI.IPv6SolicitedNode.IPv6SolicitedNodeMcAddrMaskReg3 = 0x0000005c;

        }/*end of unit IPv6SolicitedNode */


        {/*start of unit FCOE */
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TTI.FCOE.FCoEGlobalConfig = 0x00000070;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TTI.FCOE.FCoEExceptionConfigs = 0x00000074;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TTI.FCOE.FCoEExceptionCntrs0 = 0x00000078;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TTI.FCOE.FCoEExceptionCntrs1 = 0x0000007c;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TTI.FCOE.FCoEExceptionCntrs2 = 0x00000080;

        }/*end of unit FCOE */


        {/*start of unit duplicationConfigs */
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TTI.duplicationConfigs.duplicationConfigs0 = 0x00000150;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TTI.duplicationConfigs.duplicationConfigs1 = 0x00000154;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TTI.duplicationConfigs.MPLSMCDALow = 0x00000158;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TTI.duplicationConfigs.MPLSMCDAHigh = 0x0000015c;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TTI.duplicationConfigs.MPLSMCDALowMask = 0x00000160;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TTI.duplicationConfigs.MPLSMCDAHighMask = 0x00000164;

        }/*end of unit duplicationConfigs */


        {/*1a4+n*4*/
            GT_U32    n;
            for(n = 0 ; n <= 2 ; n++) {
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TTI.TTIKRuleMatchCntr[n] =
                    0x1a4+n*4;
            }/* end of loop n */
        }/*1a4+n*4*/
        {/*1b4+0x4*n*/
            GT_U32    n;
            for(n = 0 ; n <= 2 ; n++) {
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TTI.TTIKRuleMatchCntr[n+3] =
                    0x1b4+n*4;
            }/* end of loop n */
        }/*1b4+0x4*n*/

        {/*100+4*i*/
            GT_U32    i;
            for(i = 0 ; i <= 3 ; i++) {
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TTI.TTIUserDefinedEtherTypes_[i] =
                    0x100+4*i;
            }/* end of loop i */
        }/*100+4*i*/
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TTI.TTIIPv6GREEtherType = 0x00000034;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TTI.TTIIPv4GREEthertype = 0x00000018;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TTI.specialEtherTypes = 0x00000028;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TTI.sourceIDAssignment0 = 0x0000014c;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TTI.receivedFlowCtrlPktsCntr = 0x000001d0;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TTI.preRouteExceptionMasks = 0x0000002c;
        {/*f0+4*i*/
            GT_U32    i;
            for(i = 0 ; i <= 3 ; i++) {
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TTI.PCLUserDefinedEtherTypes_[i] =
                    0xf0+4*i;
            }/* end of loop i */
        }/*f0+4*i*/
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TTI.MPLSEtherTypes = 0x00000030;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TTI.IPv6ExtensionValue = 0x00000038;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TTI.IPMinimumOffset = 0x00000020;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TTI.IPLengthChecks = 0x000001cc;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TTI.ECCTwoErrorsCntr = 0x000001dc;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TTI.ECCOneErrorCntr = 0x000001d8;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TTI.DSAConfig = 0x00000060;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TTI.droppedFlowCtrlPktsCntr = 0x000001d4;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TTI.CNAndFCConfig = 0x00000024;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TTI.CFMEtherType = 0x000001c8;
        if (SMEM_CHT_IS_SIP6_GET(devObjPtr))
        {
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TTI.cutThroughTtiConfig = 0x0000006C;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TTI.cutThroughBcExtactionFailCntr = 0x00000184;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TTI.
                cutThroughHeaderIntegrityExceptionConfig1 = 0x00000188;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TTI.
                cutThroughHeaderIntegrityExceptionConfig2 = 0x0000018C;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TTI.ECNDroppedConfiguration = 0x0000016C;
        }

        if(SMEM_CHT_IS_SIP6_10_GET(devObjPtr))
        {
            {/* 0x00001000 + 0x4*n */
                GT_U32    n;
                for(n = 0 ; n < 16 ; n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TTI.ppuProfile[n] = 0x00001000 + 0x4*n;
                }
            }/* 0x00001000 + 0x4*n */

            {/* 0x00001050 + 0x4*n */
                GT_U32    n;
                for(n = 0 ; n < 8 ; n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TTI.tunnelHeaderLengthProfile[n] = 0x00001050 + 0x4*n;
                }
            }/* 0x00001050 + 0x4*n */
        }

        if(SMEM_CHT_IS_SIP6_30_GET(devObjPtr))
        {/*start of unit IP2ME */
            {/*4000+n*0x18*/
                GT_U32    n;
                for(n = 0 ; n <= 7 ; n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TTI.IP2ME.IP2MEDip0[n] = 0x4000 + n*0x18;
                }/* end of loop n */
            }/*4000+n*0x18*/
            {/*4004+n*0x18*/
                GT_U32    n;
                for(n = 0 ; n <= 7 ; n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TTI.IP2ME.IP2MEDip1[n] = 0x4004 + n*0x18;
                }/* end of loop n */
            }/*4004+n*0x18*/
            {/*4008+n*0x18*/
                GT_U32    n;
                for(n = 0 ; n <= 7 ; n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TTI.IP2ME.IP2MEDip2[n] = 0x4008 + n*0x18;
                }/* end of loop n */
            }/*4008+n*0x18*/
            {/*400c+n*0x18*/
                GT_U32    n;
                for(n = 0 ; n <= 7 ; n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TTI.IP2ME.IP2MEDip3[n] = 0x400c + n*0x18;
                }/* end of loop n */
            }/*400c+n*0x18*/
            {/*4010+n*0x18*/
                GT_U32    n;
                for(n = 0 ; n <= 7 ; n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TTI.IP2ME.IP2MEConf0[n] = 0x4010 + n*0x18;
                }/* end of loop n */
            }/*4010+n*0x18*/
            {/*4014+n*0x18*/
                GT_U32    n;
                for(n = 0 ; n <= 7 ; n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TTI.IP2ME.IP2MEConf1[n] = 0x4014 + n*0x18;
                }/* end of loop n */
            }/*4014+n*0x18*/
        }/*end of unit IP2ME */

    }/*end of unit TTI */

    if(!SMEM_CHT_IS_SIP6_GET(devObjPtr))
    {/*start of unit rxDMA */
        unitIndex = 0;

        if(!SMEM_CHT_IS_SIP5_10_GET(devObjPtr))
        {/*start of unit singleChannelDMAConfigs */
            {/*304+n*0x4*/
                GT_U32    n;
                for(n = 0 ; n <= 72 ; n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->rxDMA[unitIndex].singleChannelDMAConfigs.SCDMAConfig0[n] =
                        0x304+n*0x4;
                }/* end of loop n */
            }/*304+n*0x4*/
            {/*450+n*0x4*/
                GT_U32    n;
                for(n = 0 ; n <= 72 ; n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->rxDMA[unitIndex].singleChannelDMAConfigs.SCDMAConfig1[n] =
                        0x450+n*0x4;
                }/* end of loop n */
            }/*450+n*0x4*/
            {/*600+n*0x4*/
                GT_U32    n;
                for(n = 0 ; n <= 72 ; n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->rxDMA[unitIndex].singleChannelDMAConfigs.SCDMACTPktIndentification[n] =
                        0x600+n*0x4;
                }/* end of loop n */
            }/*600+n*0x4*/

        }/*end of unit singleChannelDMAConfigs */


        {/*start of unit globalRxDMAConfigs */
            {/*start of unit rejectConfigs */
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->rxDMA[unitIndex].globalRxDMAConfigs.rejectConfigs.rejectPktCommand = 0x000000a0;
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->rxDMA[unitIndex].globalRxDMAConfigs.rejectConfigs.rejectCPUCode = 0x000000a4;

            }/*end of unit rejectConfigs */


            {/*start of unit globalConfigCutThrough */
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->rxDMA[unitIndex].globalRxDMAConfigs.globalConfigCutThrough.CTEtherTypeIdentification = 0x00000084;
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->rxDMA[unitIndex].globalRxDMAConfigs.globalConfigCutThrough.CTUPEnable = 0x00000088;

            }/*end of unit globalConfigCutThrough */


            {/*start of unit buffersLimitProfiles */
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->rxDMA[unitIndex].globalRxDMAConfigs.buffersLimitProfiles.portsBuffersLimitProfile0XONXOFFConfig = 0x00000000;
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->rxDMA[unitIndex].globalRxDMAConfigs.buffersLimitProfiles.portsBuffersLimitProfile0RxlimitConfig = 0x00000004;
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->rxDMA[unitIndex].globalRxDMAConfigs.buffersLimitProfiles.portsBuffersLimitProfile1XONXOFFConfig = 0x00000008;
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->rxDMA[unitIndex].globalRxDMAConfigs.buffersLimitProfiles.portsBuffersLimitProfile1RxlimitConfig = 0x0000000c;
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->rxDMA[unitIndex].globalRxDMAConfigs.buffersLimitProfiles.portsBuffersLimitProfile7XONXOFFConfig = 0x00000038;
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->rxDMA[unitIndex].globalRxDMAConfigs.buffersLimitProfiles.portsBuffersLimitProfile2RxlimitConfig = 0x00000014;
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->rxDMA[unitIndex].globalRxDMAConfigs.buffersLimitProfiles.portsBuffersLimitProfile3XONXOFFConfig = 0x00000018;
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->rxDMA[unitIndex].globalRxDMAConfigs.buffersLimitProfiles.portsBuffersLimitProfile3RxlimitConfig = 0x0000001c;
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->rxDMA[unitIndex].globalRxDMAConfigs.buffersLimitProfiles.portsBuffersLimitProfile4XONXOFFConfig = 0x00000020;
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->rxDMA[unitIndex].globalRxDMAConfigs.buffersLimitProfiles.portsBuffersLimitProfile4RxlimitConfig = 0x00000024;
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->rxDMA[unitIndex].globalRxDMAConfigs.buffersLimitProfiles.portsBuffersLimitProfile5XONXOFFConfig = 0x00000028;
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->rxDMA[unitIndex].globalRxDMAConfigs.buffersLimitProfiles.portsBuffersLimitProfile5RxlimitConfig = 0x0000002c;
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->rxDMA[unitIndex].globalRxDMAConfigs.buffersLimitProfiles.portsBuffersLimitProfile6XONXOFFConfig = 0x00000030;
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->rxDMA[unitIndex].globalRxDMAConfigs.buffersLimitProfiles.portsBuffersLimitProfile6RxlimitConfig = 0x00000034;
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->rxDMA[unitIndex].globalRxDMAConfigs.buffersLimitProfiles.portsBuffersLimitProfile2XONXOFFConfig = 0x00000010;
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->rxDMA[unitIndex].globalRxDMAConfigs.buffersLimitProfiles.portsBuffersLimitProfile7RxlimitConfig = 0x0000003c;
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->rxDMA[unitIndex].globalRxDMAConfigs.buffersLimitProfiles.RFCntrOverrunLmitConfig = 0x00000040;

            }/*end of unit buffersLimitProfiles */


        }/*end of unit globalRxDMAConfigs */


        {/*start of unit SIP_COMMON_MODULES */
            {/*start of unit pizzaArbiter */
                {/*2508+n*0x4*/
                    GT_U32    n;
                    for(n = 0 ; n <= 84 ; n++) {
                        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->rxDMA[unitIndex].SIP_COMMON_MODULES.pizzaArbiter.pizzaArbiterConfigReg[n] =
                            0x2508+n*0x4;
                    }/* end of loop n */
                }/*2508+n*0x4*/
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->rxDMA[unitIndex].SIP_COMMON_MODULES.pizzaArbiter.pizzaArbiterCtrlReg = 0x00002500;
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->rxDMA[unitIndex].SIP_COMMON_MODULES.pizzaArbiter.pizzaArbiterStatusReg = 0x00002504;

            }/*end of unit pizzaArbiter */


        }/*end of unit SIP_COMMON_MODULES */


        if(!SMEM_CHT_IS_SIP5_10_GET(devObjPtr))
        {/*start of unit interrupts */
            {/*d0c+n*0x4*/
                GT_U32    n;
                for(n = 0 ; n <= 72 ; n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->rxDMA[unitIndex].interrupts.rxDMASCDMAInterruptMask[n] =
                        0xd0c+n*0x4;
                }/* end of loop n */
            }/*d0c+n*0x4*/
            {/*908+n*0x4*/
                GT_U32    n;
                for(n = 0 ; n <= 72 ; n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->rxDMA[unitIndex].interrupts.rxDMASCDMAInterruptCause[n] =
                        0x908+n*0x4;
                }/* end of loop n */
            }/*908+n*0x4*/
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->rxDMA[unitIndex].interrupts.rxDMAInterrupt0Cause = 0x00000900;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->rxDMA[unitIndex].interrupts.rxDMAInterrupt0Mask = 0x00000904;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->rxDMA[unitIndex].interrupts.rxDMAInterruptSummary0Cause = 0x00001028;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->rxDMA[unitIndex].interrupts.rxDMAInterruptSummary0Mask = 0x0000102c;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->rxDMA[unitIndex].interrupts.rxDMAInterruptSummary1Cause = 0x00001030;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->rxDMA[unitIndex].interrupts.rxDMAInterruptSummary1Mask = 0x00001034;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->rxDMA[unitIndex].interrupts.rxDMAInterruptSummary2Cause = 0x00001038;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->rxDMA[unitIndex].interrupts.rxDMAInterruptSummary2Mask = 0x0000103c;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->rxDMA[unitIndex].interrupts.rxDMAInterruptSummary3Cause = 0x00001040;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->rxDMA[unitIndex].interrupts.rxDMAInterruptSummary3Mask = 0x00001044;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->rxDMA[unitIndex].interrupts.rxDMAInterruptSummaryCause = 0x00001048;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->rxDMA[unitIndex].interrupts.rxDMAInterruptSummaryMask = 0x0000104c;

        }/*end of unit interrupts */

    }/*end of unit rxDMA */

    if(!isPipeDevice && isSip5_20 && !isSip6)
    {/* start of unit IA */
        {
            GT_U32  unitIndex;
            for(unitIndex = 0; unitIndex < devObjPtr->multiDataPath.maxIa ; unitIndex++)
            {
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->ingressAggregator[unitIndex].PIPPrioThresholds0          = 0x00000000;
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->ingressAggregator[unitIndex].PIPPrioThresholds1          = 0x00000004;
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->ingressAggregator[unitIndex].PIPEnable                   = 0x00000008;
            }
        }
    }/* end of unit IA */

    /* not needed for bc2 */
    if(devObjPtr->portGroupSharedDevObjPtr)
    {/*start of unit GTS */
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->GTS[0].GTS.globalConfigs = 0x00000000;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->GTS[0].GTS.enableTimestamping = 0x00000004;
        {/*8+4*i*/
            GT_U32    i;
            for(i = 0 ; i <= 1 ; i++) {
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->GTS[0].GTS.timestampingPortEnable[i] =
                    0x8+4*i;
            }/* end of loop i */
        }/*8+4*i*/
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->GTS[0].GTS.TODCntrNanoseconds = 0x00000010;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->GTS[0].GTS.TODCntrSeconds[0] = 0x00000014;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->GTS[0].GTS.TODCntrSeconds[1] = 0x00000018;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->GTS[0].GTS.TODCntrShadowNanoseconds = 0x0000001c;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->GTS[0].GTS.TODCntrShadowSeconds[0] = 0x00000020;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->GTS[0].GTS.TODCntrShadowSeconds[1] = 0x00000024;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->GTS[0].GTS.GTSInterruptMask = 0x0000002c;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->GTS[0].GTS.GTSInterruptCause = 0x00000028;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->GTS[0].GTS.globalFIFOCurrentEntry[1] = 0x00000034;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->GTS[0].GTS.globalFIFOCurrentEntry[0] = 0x00000030;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->GTS[0].GTS.overrunStatus = 0x00000038;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->GTS[0].GTS.underrunStatus = 0x0000003c;

        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->GTS[1] = /* added manually */
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->GTS[0];

        {
            GT_U32  gtsIndex;
            for(gtsIndex = 0 ; gtsIndex <=1 ;gtsIndex++)
            {
                offsetAddrSet(SET_UNIT_AND_SIZE_MAC(SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->GTS[gtsIndex])/*unit name*/,
                    devObjPtr->memUnitBaseAddrInfo.gts[gtsIndex]);         /*offset*/
            }
        }

    }/*end of unit GTS */

    if(isDxControlPipe)
    {/*start of unit L2I */
        {/*start of unit VLANRangesConfig */
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->L2I.VLANRangesConfig.ingrVLANRangeConfig = 0x00000130;

        }/*end of unit VLANRangesConfig */


        {/*start of unit VLANMRUProfilesConfig */
            {/*140+n*0x4*/
                GT_U32    n;
                for(n = 0 ; n <= 3 ; n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->L2I.VLANMRUProfilesConfig.VLANMRUProfileConfig[n] =
                        0x140+n*0x4;
                }/* end of loop n */
            }/*140+n*0x4*/

        }/*end of unit VLANMRUProfilesConfig */


        {/*start of unit UDPBcMirrorTrapUDPRelayConfig */
            {/*1100+n*0x4*/
                GT_U32    n;
                for(n = 0 ; n <= 11 ; n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->L2I.UDPBcMirrorTrapUDPRelayConfig.UDPBcDestinationPortConfigTable[n] =
                        0x1100+n*0x4;
                }/* end of loop n */
            }/*1100+n*0x4*/

        }/*end of unit UDPBcMirrorTrapUDPRelayConfig */


        {/*start of unit securityBreachStatus */
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->L2I.securityBreachStatus.securityBreachStatus0 = 0x00001400;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->L2I.securityBreachStatus.securityBreachStatus1 = 0x00001404;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->L2I.securityBreachStatus.securityBreachStatus2 = 0x00001408;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->L2I.securityBreachStatus.securityBreachStatus3 = 0x0000140c;

        }/*end of unit securityBreachStatus */


        {/*start of unit MACBasedQoSTable */
            {/*1000+n*0x4*/
                GT_U32    n;
                for(n = 0 ; n <= 6 ; n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->L2I.MACBasedQoSTable.MACQoSTableEntry[n] =
                        0x1000+n*0x4;
                }/* end of loop n */
            }/*1000+n*0x4*/

        }/*end of unit MACBasedQoSTable */


        {/*start of unit layer2BridgeMIBCntrs */
            {/*153c+n*0x20*/
                GT_U32    n;
                for(n = 0 ; n <= 1 ; n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->L2I.layer2BridgeMIBCntrs.setVLANIngrFilteredPktCount[n] =
                        0x153c+n*0x20;
                }/* end of loop n */
            }/*153c+n*0x20*/
            {/*1530+0x20*n*/
                GT_U32    n;
                for(n = 0 ; n <= 1 ; n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->L2I.layer2BridgeMIBCntrs.cntrsSetConfig0[n] =
                        0x1530+0x20*n;
                }/* end of loop n */
            }/*1530+0x20*n*/
            {/*1534+0x20*n*/
                GT_U32    n;
                for(n = 0 ; n <= 1 ; n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->L2I.layer2BridgeMIBCntrs.cntrsSetConfig1[n] =
                        0x1534+0x20*n;
                }/* end of loop n */
            }/*1534+0x20*n*/
            {/*1538+n*0x20*/
                GT_U32    n;
                for(n = 0 ; n <= 1 ; n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->L2I.layer2BridgeMIBCntrs.setIncomingPktCount[n] =
                        0x1538+n*0x20;
                }/* end of loop n */
            }/*1538+n*0x20*/
            {/*1540+n*0x20*/
                GT_U32    n;
                for(n = 0 ; n <= 1 ; n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->L2I.layer2BridgeMIBCntrs.setSecurityFilteredPktCount[n] =
                        0x1540+n*0x20;
                }/* end of loop n */
            }/*1540+n*0x20*/
            {/*1544+n*0x20*/
                GT_U32    n;
                for(n = 0 ; n <= 1 ; n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->L2I.layer2BridgeMIBCntrs.setBridgeFilteredPktCount[n] =
                        0x1544+n*0x20;
                }/* end of loop n */
            }/*1544+n*0x20*/
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->L2I.layer2BridgeMIBCntrs.MACAddrCount0 = 0x00001500;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->L2I.layer2BridgeMIBCntrs.MACAddrCount1 = 0x00001504;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->L2I.layer2BridgeMIBCntrs.MACAddrCount2 = 0x00001508;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->L2I.layer2BridgeMIBCntrs.hostOutgoingMcPktCount = 0x00001520;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->L2I.layer2BridgeMIBCntrs.hostOutgoingPktsCount = 0x00001514;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->L2I.layer2BridgeMIBCntrs.hostIncomingPktsCount = 0x00001510;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->L2I.layer2BridgeMIBCntrs.hostOutgoingBcPktCount = 0x00001524;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->L2I.layer2BridgeMIBCntrs.matrixSourceDestinationPktCount = 0x00001528;

        }/*end of unit layer2BridgeMIBCntrs */


        {/*start of unit IPv6McBridgingBytesSelection */
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->L2I.IPv6McBridgingBytesSelection.IPv6MCBridgingBytesSelectionConfig = 0x00000120;

        }/*end of unit IPv6McBridgingBytesSelection */


        {/*start of unit ingrPortsRateLimitConfig */
            {/*12a0+n*0x4*/
                GT_U32    n;
                for(n = 0 ; n <= 7 ; n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->L2I.ingrPortsRateLimitConfig.portSpeedOverrideEnable[n] =
                        0x12a0+n*0x4;
                }/* end of loop n */
            }/*12a0+n*0x4*/
            {/*1240+n*0x4*/
                GT_U32    n;
                for(n = 0 ; n <= 7 ; n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->L2I.ingrPortsRateLimitConfig.MIISpeedGranularity[n] =
                        0x1240+n*0x4;
                }/* end of loop n */
            }/*1240+n*0x4*/
            {/*1260+n*0x4*/
                GT_U32    n;
                for(n = 0 ; n <= 7 ; n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->L2I.ingrPortsRateLimitConfig.GMIISpeedGranularity[n] =
                        0x1260+n*0x4;
                }/* end of loop n */
            }/*1260+n*0x4*/
            {/*1280+n*0x4*/
                GT_U32    n;
                for(n = 0 ; n <= 7 ; n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->L2I.ingrPortsRateLimitConfig.XGSpeedGranularity[n] =
                        0x1280+n*0x4;
                }/* end of loop n */
            }/*1280+n*0x4*/
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->L2I.ingrPortsRateLimitConfig.ingrRateLimitConfig0 = 0x00001200;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->L2I.ingrPortsRateLimitConfig.ingrRateLimitConfig3 = 0x0000120c;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->L2I.ingrPortsRateLimitConfig.ingrRateLimitConfig2 = 0x00001208;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->L2I.ingrPortsRateLimitConfig.ingrRateLimitConfig1 = 0x00001204;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->L2I.ingrPortsRateLimitConfig.ingrRateLimitDropCntr310 = 0x00001220;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->L2I.ingrPortsRateLimitConfig.ingrRateLimitDropCntr3932 = 0x00001224;

        }/*end of unit ingrPortsRateLimitConfig */


        if(SMEM_CHT_IS_SIP6_GET(devObjPtr))
        {
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->L2I.globalEportConifguration.globalEportMinValue  = 0x00000160;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->L2I.globalEportConifguration.globalEportMaxValue  = 0x00000164;
        }
        else
        {/*start of unit globalEportConifguration */
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->L2I.globalEportConifguration.globalEPortValue = 0x00000160;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->L2I.globalEportConifguration.globalEPortMask = 0x00000164;

        }/*end of unit globalEportConifguration */

        {/*start of unit eVLANMirrToAnalyzerEnable */
            {/*1600+n*0x4*/
                GT_U32    n;

                for(n = 0 ; n < devObjPtr->limitedResources.phyPort/32 ; n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->L2I.eVLANMirrToAnalyzerEnable.eVLAN_Mirr_to_Analyzer_Enable[n] =
                        0x1600+n*0x4;
                }/* end of loop n */
            }/*1600+n*0x4*/


        }/*end of unit eVLANMirrToAnalyzerEnable */

        {/*start of unit ctrlTrafficToCPUConfig */
            {/*c00+n*0x8 +t*0x4*/
                GT_U32    n,t;
                for(n = 0 ; n <= 7 ; n++) {
                    for(t = 0 ; t <= 1 ; t++) {
                        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->L2I.ctrlTrafficToCPUConfig.IPv6McLinkLocalCPUCodeIndex[n][t] =
                            0xc00+n*0x8 +t*0x4;
                    }/* end of loop t */
                }/* end of loop n */
            }/*c00+n*0x8 +t*0x4*/
            {/*d00+n*0x4*/
                GT_U32    n;
                for(n = 0 ; n <= 7 ; n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->L2I.ctrlTrafficToCPUConfig.IPv6McLinkLocalReged[n] =
                        0xd00+n*0x4;
                }/* end of loop n */
            }/*d00+n*0x4*/
            {/*b00+n*0x4*/
                GT_U32    n;
                for(n = 0 ; n <= 7 ; n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->L2I.ctrlTrafficToCPUConfig.IPv6McLinkLocalConfig[n] =
                        0xb00+n*0x4;
                }/* end of loop n */
            }/*b00+n*0x4*/
            {/*e04+n*0x4*/
                GT_U32    n;
                for(n = 0 ; n <= 1 ; n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->L2I.ctrlTrafficToCPUConfig.IPv6ICMPMsgTypeConfig[n] =
                        0xe04+n*0x4;
                }/* end of loop n */
            }/*e04+n*0x4*/
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->L2I.ctrlTrafficToCPUConfig.IPv6ICMPCommand = 0x00000e00;
            {/*a00+n*0x4*/
                GT_U32    n;
                for(n = 0 ; n <= 7 ; n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->L2I.ctrlTrafficToCPUConfig.IPv4McLinkLocalReged[n] =
                        0xa00+n*0x4;
                }/* end of loop n */
            }/*a00+n*0x4*/
            {/*900+n*0x8+ t*0x4*/
                GT_U32    n,t;
                for(n = 0 ; n <= 7 ; n++) {
                    for(t = 0 ; t <= 1 ; t++) {
                        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->L2I.ctrlTrafficToCPUConfig.IPv4McLinkLocalCPUCodeIndex[n][t] =
                            0x900+n*0x8+ t*0x4;
                    }/* end of loop t */
                }/* end of loop n */
            }/*900+n*0x8+ t*0x4*/
            {/*800+n*0x4*/
                GT_U32    n;
                for(n = 0 ; n <= 7 ; n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->L2I.ctrlTrafficToCPUConfig.IPv4McLinkLocalConfig[n] =
                        0x800+n*0x4;
                }/* end of loop n */
            }/*800+n*0x4*/
            {/*700+n*0x4*/
                GT_U32    n;
                for(n = 0 ; n <= 7 ; n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->L2I.ctrlTrafficToCPUConfig.IEEEReservedMcReged[n] =
                        0x700+n*0x4;
                }/* end of loop n */
            }/*700+n*0x4*/
            {/*600+n*0x4*/
                GT_U32    n;
                for(n = 0 ; n <= 15 ; n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->L2I.ctrlTrafficToCPUConfig.IEEEReservedMcCPUIndex[n] =
                        0x600+n*0x4;
                }/* end of loop n */
            }/*600+n*0x4*/
            {/*580+n*0x4*/
                GT_U32    n;
                for(n = 0 ; n <= 15 ; n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->L2I.ctrlTrafficToCPUConfig.IEEEReservedMcConfig7[n] =
                        0x580+n*0x4;
                }/* end of loop n */
            }/*580+n*0x4*/
            {/*500+n*0x4*/
                GT_U32    n;
                for(n = 0 ; n <= 15 ; n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->L2I.ctrlTrafficToCPUConfig.IEEEReservedMcConfig6[n] =
                        0x500+n*0x4;
                }/* end of loop n */
            }/*500+n*0x4*/
            {/*480+n*0x4*/
                GT_U32    n;
                for(n = 0 ; n <= 15 ; n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->L2I.ctrlTrafficToCPUConfig.IEEEReservedMcConfig5[n] =
                        0x480+n*0x4;
                }/* end of loop n */
            }/*480+n*0x4*/
            {/*400+n*0x4*/
                GT_U32    n;
                for(n = 0 ; n <= 15 ; n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->L2I.ctrlTrafficToCPUConfig.IEEEReservedMcConfig4[n] =
                        0x400+n*0x4;
                }/* end of loop n */
            }/*400+n*0x4*/
            {/*380+n*0x4*/
                GT_U32    n;
                for(n = 0 ; n <= 15 ; n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->L2I.ctrlTrafficToCPUConfig.IEEEReservedMcConfig3[n] =
                        0x380+n*0x4;
                }/* end of loop n */
            }/*380+n*0x4*/
            {/*300+n*0x4*/
                GT_U32    n;
                for(n = 0 ; n <= 15 ; n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->L2I.ctrlTrafficToCPUConfig.IEEEReservedMcConfig2[n] =
                        0x300+n*0x4;
                }/* end of loop n */
            }/*300+n*0x4*/
            {/*280+n*0x4*/
                GT_U32    n;
                for(n = 0 ; n <= 15 ; n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->L2I.ctrlTrafficToCPUConfig.IEEEReservedMcConfig1[n] =
                        0x280+n*0x4;
                }/* end of loop n */
            }/*280+n*0x4*/
            {/*200+n*0x4*/
                GT_U32    n;
                for(n = 0 ; n <= 15 ; n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->L2I.ctrlTrafficToCPUConfig.IEEEReservedMcConfig0[n] =
                        0x200+n*0x4;
                }/* end of loop n */
            }/*200+n*0x4*/

        }/*end of unit ctrlTrafficToCPUConfig */


        {/*start of unit bridgeEngineInterrupts */
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->L2I.bridgeEngineInterrupts.bridgeInterruptCause = 0x00002100;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->L2I.bridgeEngineInterrupts.bridgeInterruptMask = 0x00002104;

        }/*end of unit bridgeEngineInterrupts */


        {/*start of unit bridgeEngineConfig */
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->L2I.bridgeEngineConfig.bridgeGlobalConfig0 = 0x00000000;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->L2I.bridgeEngineConfig.bridgeGlobalConfig1 = 0x00000004;
            if(SMEM_CHT_IS_SIP6_10_GET(devObjPtr))
            {
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->L2I.bridgeEngineConfig.bridgeGlobalConfig3 = 0x00000008;
            }
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->L2I.bridgeEngineConfig.bridgeCommandConfig0 = 0x00000020;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->L2I.bridgeEngineConfig.bridgeCommandConfig1 = 0x00000024;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->L2I.bridgeEngineConfig.bridgeCommandConfig2 = 0x00000028;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->L2I.bridgeEngineConfig.bridgeCommandConfig3 = 0x0000002c;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->L2I.bridgeEngineConfig.bridgeSecurityBreachDropCntrCfg0 = 0x00000040;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->L2I.bridgeEngineConfig.bridgeSecurityBreachDropCntrCfg1 = 0x00000044;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->L2I.bridgeEngineConfig.bridgecpuPortConfig = 0x00000048;

        }/*end of unit bridgeEngineConfig */


        {/*start of unit bridgeDropCntrAndSecurityBreachDropCntrs */
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->L2I.bridgeDropCntrAndSecurityBreachDropCntrs.globalSecurityBreachFilterCntr = 0x00001300;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->L2I.bridgeDropCntrAndSecurityBreachDropCntrs.portVLANSecurityBreachDropCntr = 0x00001304;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->L2I.bridgeDropCntrAndSecurityBreachDropCntrs.bridgeFilterCntr = 0x00001308;

        }/*end of unit bridgeDropCntrAndSecurityBreachDropCntrs */


        {/*start of unit bridgeAccessMatrix */
            {/*180+n*0x4*/
                GT_U32    n;
                for(n = 0 ; n <= 7 ; n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->L2I.bridgeAccessMatrix.bridgeAccessMatrixLine[n] =
                        0x180+n*0x4;
                }/* end of loop n */
            }/*180+n*0x4*/
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->L2I.bridgeAccessMatrix.bridgeAccessMatrixDefault = 0x00000170;

        }/*end of unit bridgeAccessMatrix */

    }/*end of unit L2I */

    if(isDxControlPipe)
    {/*start of unit FDB */
        {/*start of unit FDBCore */
            {/*start of unit sourceAddrNotLearntCntr */
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->FDB.FDBCore.sourceAddrNotLearntCntr.learnedEntryDiscardsCount = 0x00000060;

            }/*end of unit sourceAddrNotLearntCntr */


            {/*start of unit FDB */
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->FDB.FDBCore.FDB.MTCPipeDispatcher = 0x00000400;

            }/*end of unit FDB */

            {/*start of unit FDBSERProtection */
                {/*420+4*n*/
                    GT_U32    n;
                    for(n = 0 ; n <= 4 ; n++) {
                        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->FDB.FDBCore.FDBSERProtection.FDBFaultAddr[n] =
                            0x420+4*n;
                    }/* end of loop n */
                }/*420+4*n*/
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->FDB.FDBCore.FDBSERProtection.FDBSerConfig = 0x00000410;
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->FDB.FDBCore.FDBSERProtection.FDBSerErrData = 0x00000414;
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->FDB.FDBCore.FDBSERProtection.FDBSerErrAddr = 0x00000418;
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->FDB.FDBCore.FDBSERProtection.FDBSerErrCntr = 0x0000041c;

            }/*end of unit FDBSERProtection */


            {/*start of unit FDBSchedulerConfig */
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->FDB.FDBCore.FDBSchedulerConfig.FDBSchedulerSWRRCtrl = 0x00000380;
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->FDB.FDBCore.FDBSchedulerConfig.FDBSchedulerSWRRConfig = 0x00000384;

            }/*end of unit FDBSchedulerConfig */

            {/*start of unit FDBInterrupt */
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->FDB.FDBCore.FDBInterrupt.FDBInterruptCauseReg = 0x000001b0;
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->FDB.FDBCore.FDBInterrupt.FDBInterruptMaskReg = 0x000001b4;

            }/*end of unit FDBInterrupt */


            {/*start of unit FDBInternal */
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->FDB.FDBCore.FDBInternal.metalFix = 0x00000170;
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->FDB.FDBCore.FDBInternal.FDBIdleState = 0x00000174;

            }/*end of unit FDBInternal */


            {/*start of unit FDBIndirectAccess */
                {/*134+n*4*/
                    GT_U32    n;
                    for(n = 0 ; n <= 4 ; n++) {
                        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->FDB.FDBCore.FDBIndirectAccess.FDBIndirectAccessData[n] =
                            0x134+n*4;
                    }/* end of loop n */
                }/*134+n*4*/
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->FDB.FDBCore.FDBIndirectAccess.FDBIndirectAccessCtrl = 0x00000130;
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->FDB.FDBCore.FDBIndirectAccess.FDBIndirectAccessAccess = 0x00000150;

            }/*end of unit FDBIndirectAccess */


            {/*start of unit FDBHashResults */
                {/*300+0x4*n*/
                    GT_U32    n;
                    for(n = 0 ; n <= 7 ; n++) {
                        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->FDB.FDBCore.FDBHashResults.FDBMultiHashCRCResultReg[n] =
                            0x300+0x4*n;
                    }/* end of loop n */
                }/*300+0x4*n*/
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->FDB.FDBCore.FDBHashResults.FDBNonMulti_hash_crc_result = 0x00000320;
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->FDB.FDBCore.FDBHashResults.FDBXorHash = 0x00000324;

            }/*end of unit FDBHashResults */


            {/*start of unit FDBGlobalConfig */
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->FDB.FDBCore.FDBGlobalConfig.FDBGlobalConfig = 0x00000000;
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->FDB.FDBCore.FDBGlobalConfig.FDBGlobalConfig1 = 0x00000004;
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->FDB.FDBCore.FDBGlobalConfig.FDBCRCHashInitConfig = 0x0000000c;
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->FDB.FDBCore.FDBGlobalConfig.bridgeAccessLevelConfig = 0x00000010;
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->FDB.FDBCore.FDBGlobalConfig.FDBAgingWindowSize = 0x00000014;

                if(isSip6)
                {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->FDB.FDBCore.FDBGlobalConfig.FDBGlobalConfig2 = 0x00000008;
                }

            }/*end of unit FDBGlobalConfig */


            {/*start of unit FDBCntrsUpdate */
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->FDB.FDBCore.FDBCntrsUpdate.FDBCntrsUpdateCtrl = 0x00000340;

            }/*end of unit FDBCntrsUpdate */


            {/*start of unit FDBBankCntrs */
                {/*200+n * 0x4*/
                    GT_U32    n;
                    for(n = 0 ; n <= 15 ; n++) {
                        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->FDB.FDBCore.FDBBankCntrs.FDBBankCntr[n] =
                            0x200+n * 0x4;
                    }/* end of loop n */
                }/*200+n * 0x4*/
                {/*240+n * 0x4*/                 /*manually replaced 16 registers into array*/
                    GT_U32    n;
                    for(n = 0 ; n <= 15 ; n++) {
                        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->FDB.FDBCore.FDBBankCntrs.FDBBankRank[n] =
                            0x240+n * 0x4;
                    }/* end of loop n */
                }/*240+n * 0x4*/
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->FDB.FDBCore.FDBBankCntrs.FDBBankCntrsCtrl = 0x00000280;

            }/*end of unit FDBBankCntrs */


            {/*start of unit FDBAddrUpdateMsgs */
                {/*start of unit AUMsgToCPU */
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->FDB.FDBCore.FDBAddrUpdateMsgs.AUMsgToCPU.AUMsgToCPU = 0x00000090;
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->FDB.FDBCore.FDBAddrUpdateMsgs.AUMsgToCPU.AUFIFOToCPUConfig = 0x00000094;

                }/*end of unit AUMsgToCPU */


                {/*start of unit AUMsgFromCPU */
                    {/*d0+n*4*/
                        GT_U32    n;
                        for(n = 0 ; n <= 5 ; n++) {
                            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->FDB.FDBCore.FDBAddrUpdateMsgs.AUMsgFromCPU.AUMsgFromCPUWord[n] =
                                0xd0+n*4;
                        }/* end of loop n */
                    }/*d0+n*4*/
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->FDB.FDBCore.FDBAddrUpdateMsgs.AUMsgFromCPU.AUMsgFromCPUCtrl = 0x000000c0;

                }/*end of unit AUMsgFromCPU */


            }/*end of unit FDBAddrUpdateMsgs */


            {/*start of unit FDBAction */
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->FDB.FDBCore.FDBAction.FDBActionGeneral = 0x00000020;
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->FDB.FDBCore.FDBAction.FDBAction0 = 0x00000024;
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->FDB.FDBCore.FDBAction.FDBAction1 = 0x00000028;
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->FDB.FDBCore.FDBAction.FDBAction2 = 0x0000002c;
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->FDB.FDBCore.FDBAction.FDBAction3 = 0x00000030;
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->FDB.FDBCore.FDBAction.FDBAction4 = 0x00000034;
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->FDB.FDBCore.FDBAction.FDBActionOldParameters = 0x00000038;
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->FDB.FDBCore.FDBAction.FDBActionNewParameters = 0x0000003c;

            }/*end of unit FDBAction */


            if(SMEM_CHT_IS_SIP6_30_GET(devObjPtr))
            {
                {/*hsrPrp*/
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->FDB.FDBCore.hsrPrp.hsrPrpGlobalConfig       = 0x00000500;
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->FDB.FDBCore.hsrPrp.ddeExpirationTimeConfig  = 0x00000504;
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->FDB.FDBCore.hsrPrp.hsrPrpTimer              = 0x00000508;
                    {
                        GT_U32    n;
                        for(n = 0 ; n <= 9 ; n++) {
                            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->FDB.FDBCore.hsrPrp.hsrPrpCountersFirst [n] = 0x00000510 + 0xc*n;
                            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->FDB.FDBCore.hsrPrp.hsrPrpCountersSecond[n] = 0x00000514 + 0xc*n;
                            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->FDB.FDBCore.hsrPrp.hsrPrpCountersThird [n] = 0x00000518 + 0xc*n;
                        }
                    }
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->FDB.FDBCore.hsrPrp.hsrPrpOverridenNonExpiredCounter = 0x00000590;
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->FDB.FDBCore.hsrPrp.hsrPrpAging     = 0x00000594;
                }/*hsrPrp;*/
            }
        }/*end of unit FDBCore */


    }/*end of unit FDB */


    if(isDxControlPipe)
    {/*start of unit EQ */
        {/*start of unit toCpuPktRateLimiters */
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EQ.toCpuPktRateLimiters.CPUCodeRateLimiterDropCntr = 0x00000068;
            {/*80000+n*0x4*/
                GT_U32    n;
                for(n = 0 ; n <= 254 ; n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EQ.toCpuPktRateLimiters.toCpuRateLimiterPktCntr[n] =
                        0x80000+n*0x4;
                }/* end of loop n */
            }/*80000+n*0x4*/

        }/*end of unit toCpuPktRateLimiters */


        {/*start of unit SCTRateLimitersInterrupt */
            {/*10020+(n-1)*0x10*/
                GT_U32    n;
                GT_U32    nMax = SMEM_CHT_IS_SIP6_GET(devObjPtr) ? 16 :
                                 SMEM_CHT_IS_SIP5_20_GET(devObjPtr) ? 32 : 16;
                for(n = 1 ; n <= nMax ; n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EQ.SCTRateLimitersInterrupt.CPUCodeRateLimitersInterruptCause[n-1] =
                        0x10020+(n-1)*0x10;
                }/* end of loop n */
            }/*10020+(n-1)*0x10*/
            {/*10024+(n-1)*0x10*/
                GT_U32    n;
                GT_U32    nMax = SMEM_CHT_IS_SIP6_GET(devObjPtr) ? 16 :
                                 SMEM_CHT_IS_SIP5_20_GET(devObjPtr) ? 32 : 16;
                for(n = 1 ; n <= nMax ; n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EQ.SCTRateLimitersInterrupt.CPUCodeRateLimitersInterruptMask[n-1] =
                        0x10024+(n-1)*0x10;
                }/* end of loop n */
            }/*10024+(n-1)*0x10*/
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EQ.SCTRateLimitersInterrupt.CPUCodeRateLimitersInterruptSummary = 0x00010000;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EQ.SCTRateLimitersInterrupt.CPUCodeRateLimitersInterruptSummaryMask = 0x00010004;

        }/*end of unit SCTRateLimitersInterrupt */


        {/*start of unit preEgrInterrupt */
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EQ.preEgrInterrupt.preEgrInterruptSummary = 0x00000058;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EQ.preEgrInterrupt.preEgrInterruptSummaryMask = 0x0000005c;

        }/*end of unit preEgrInterrupt */


        {/*start of unit preEgrEngineGlobalConfig */
            {/*a100+r*0x4*/
                GT_U32    r;
                GT_U32    rMax =
                    SMEM_CHT_IS_SIP6_10_GET(devObjPtr) ?  3 :
                    SMEM_CHT_IS_SIP6_GET(devObjPtr)    ? 31 :
                    SMEM_CHT_IS_SIP5_20_GET(devObjPtr) ? 15 : 7;
                for(r = 0 ; r <= rMax ; r++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EQ.preEgrEngineGlobalConfig.duplicationOfPktsToCPUConfig[r] =
                        0xa100+r*0x4;
                }/* end of loop r */
            }/*a100+r*0x4*/
            {/*a028+r*0x4*/
                GT_U32    r;
                for(r = 0 ; r <= 3 ; r++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EQ.preEgrEngineGlobalConfig.MTUTableEntry[r] =
                        0xa028+r*0x4;
                }/* end of loop r */
            }/*a028+r*0x4*/
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EQ.preEgrEngineGlobalConfig.preEgrEngineGlobalConfig = 0x0000a000;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EQ.preEgrEngineGlobalConfig.MTUGlobalConfig = 0x0000a020;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EQ.preEgrEngineGlobalConfig.lossyDropConfig = 0x0000a008;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EQ.preEgrEngineGlobalConfig.cscdTrunkHashBitSelectionConfig = 0x0000a010;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EQ.preEgrEngineGlobalConfig.targetEPortMTUExceededCntr = 0x0000a024;

            if(SMEM_CHT_IS_SIP6_GET(devObjPtr))
            {
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EQ.preEgrEngineGlobalConfig.incomingWRRArbiterWeights = 0x0000af00;
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EQ.preEgrEngineGlobalConfig.incomingWRRArbiterPriority = 0x0000af08;
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EQ.preEgrEngineGlobalConfig.preEgrEngineGlobalConfig2 = 0x0000a004;
            }
            if(SMEM_CHT_IS_SIP6_10_GET(devObjPtr))
            {
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EQ.preEgrEngineGlobalConfig.scalableSgtSupport = 0x0000AF34;
            }
        }/*end of unit preEgrEngineGlobalConfig */


        {/*start of unit mirrToAnalyzerPortConfigs */
            {/*b200+i*4*/
                GT_U32    i;
                for(i = 0 ; i <= 6 ; i++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EQ.mirrToAnalyzerPortConfigs.mirrorInterfaceParameterReg[i] =
                        0xb200+i*4;
                }/* end of loop i */
            }/*b200+i*4*/
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EQ.mirrToAnalyzerPortConfigs.analyzerPortGlobalConfig = 0x0000b000;

            if(SMEM_CHT_IS_SIP6_GET(devObjPtr))
            {
                {/*b04c+i*4*/
                    GT_U32    i;
                    for(i = 0 ; i <= 6 ; i++) {
                        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EQ.mirrToAnalyzerPortConfigs.ingrStatisticMirrToAnalyzerPortRatioConfig[i] =
                            0xb04c+i*4;
                    }/* end of loop i */
                }/*b04c+i*4*/
                {/*b00c+i*4*/
                    GT_U32    i;
                    for(i = 0 ; i <= 6 ; i++) {
                        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EQ.mirrToAnalyzerPortConfigs.ingrStatisticMirrToAnalyzerPortEnableConfig[i] =
                            0xb00c+i*4;
                    }/* end of loop i */
                }/*b00c+i*4*/
            }
            else
            {
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EQ.mirrToAnalyzerPortConfigs.ingrStatisticMirrToAnalyzerPortConfig = 0x0000b004;
            }
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EQ.mirrToAnalyzerPortConfigs.ingrAndEgrMonitoringToAnalyzerQoSConfig = 0x0000b008;

        }/*end of unit mirrToAnalyzerPortConfigs */


        {/*start of unit L2ECMP */
            if(SMEM_CHT_IS_SIP6_GET(devObjPtr))
            {
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EQ.L2ECMP.ePortECMPEPortMinimum = 0x0000502C;
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EQ.L2ECMP.ePortECMPEPortMaximum = 0x00005030;
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EQ.L2ECMP.ePortECMPHashBitSelectionConfig = 0x00005028;

                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EQ.L2ECMP.dlbLTTIndexBaseEport  = 0x0000501C;
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EQ.L2ECMP.dlbEportMinValue = 0x00005034;
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EQ.L2ECMP.dlbEportMaxValue = 0x00005038;
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EQ.L2ECMP.globalEportDLBConfig = 0x0000503C;
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EQ.L2ECMP.ePortDLBCurrentTime = 0x00005040;
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EQ.L2ECMP.dlbMinimumFlowletIpg = 0x00005044;
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EQ.L2ECMP.coarseGrainDetectorHashBitSelectionConfig = 0x0000d014;
            }
            else
            {
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EQ.L2ECMP.ePortECMPEPortValue = 0x00005010;
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EQ.L2ECMP.ePortECMPEPortMask = 0x00005014;
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EQ.L2ECMP.ePortECMPHashBitSelectionConfig = 0x00005030;
            }

            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EQ.L2ECMP.ePortECMPLTTIndexBaseEPort = 0x00005018;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EQ.L2ECMP.ePortECMPLFSRConfig = 0x00005020;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EQ.L2ECMP.ePortECMPLFSRSeed = 0x00005024;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EQ.L2ECMP.trunkLFSRConfig = 0x0000d000;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EQ.L2ECMP.trunkLFSRSeed = 0x0000d004;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EQ.L2ECMP.trunkHashBitSelectionConfig = 0x0000d010;

            if(SMEM_CHT_IS_SIP6_10_GET(devObjPtr))/* added manually */
            {
                GT_U32    n;
                for(n = 0 ; n <= 15 ; n ++)
                {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EQ.L2ECMP.L2ECMP_Hash_Bit_Selection[n] = 0x00005060 + n*4;
                }

                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EQ.L2ECMP.L2ECMP_config = 0x00005014;
            }


        }/*end of unit L2ECMP */


        if(!SMEM_CHT_IS_SIP6_GET(devObjPtr))
        {/*start of unit ingrSTCInterrupt */
            {/*620+(n-1)*16*/
                GT_U32    n;
                GT_U32    nMax = SMEM_CHT_IS_SIP5_20_GET(devObjPtr) ? 32 : 16;
                for(n = 1 ; n <= nMax ; n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EQ.ingrSTCInterrupt.ingrSTCInterruptCause[n-1] =
                        0x620+(n-1)*16;
                }/* end of loop n */
            }/*620+(n-1)*16*/
            {/*624+(n-1)*16*/
                GT_U32    n;
                GT_U32    nMax = SMEM_CHT_IS_SIP5_20_GET(devObjPtr) ? 32 : 16;
                for(n = 1 ; n <= nMax ; n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EQ.ingrSTCInterrupt.ingrSTCInterruptMask[n-1] =
                        0x624+(n-1)*16;
                }/* end of loop n */
            }/*624+(n-1)*16*/
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EQ.ingrSTCInterrupt.ingrSTCInterruptSummary = 0x00000600;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EQ.ingrSTCInterrupt.ingrSTCInterruptSummaryMask = 0x00000604;

            if(SMEM_CHT_IS_SIP5_20_GET(devObjPtr))
            {
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EQ.ingrSTCInterrupt.ingrSTCInterruptSummary1 = 0x00000608;
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EQ.ingrSTCInterrupt.ingrSTCInterruptSummary1Mask = 0x0000060c;
            }
        }/*end of unit ingrSTCInterrupt */


        if(!SMEM_CHT_IS_SIP6_GET(devObjPtr))
        {/*start of unit ingrSTCConfig */
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EQ.ingrSTCConfig.ingrSTCConfig = 0x0000001c;

        }/*end of unit ingrSTCConfig */


        {/*start of unit ingrForwardingRestrictions */
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EQ.ingrForwardingRestrictions.ingrForwardingRestrictionsDroppedPktsCntr = 0x0002000c;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EQ.ingrForwardingRestrictions.ingrForwardingRestrictionsDropCode = 0x00020010;

        }/*end of unit ingrForwardingRestrictions */


        {/*start of unit ingrDropCntr */
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EQ.ingrDropCntr.ingrDropCntrConfig = 0x0000003c;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EQ.ingrDropCntr.ingrDropCntr = 0x00000040;

        }/*end of unit ingrDropCntr */


        {/*start of unit ingrDropCodesCntr */
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EQ.ingrDropCodesCntr.dropCodesCntrConfig = 0x0000006c;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EQ.ingrDropCodesCntr.dropCodesCntr = 0x00000070;

        }/*end of unit ingrDropCodesCntr */


        if(SMEM_CHT_IS_SIP6_GET(devObjPtr))
        {/*start of unit ingrDropCodeLatching */
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EQ.ingrDropCodeLatching.ingrDropCodeLatching = 0x000000b0;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EQ.ingrDropCodeLatching.lastDropCode = 0x000000b4;

        }/*end of unit ingrDropCodeLatching */

        {/*start of unit ePort */
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EQ.ePort.ePortGlobalConfig = 0x00005000;
            if(!SMEM_CHT_IS_SIP6_GET(devObjPtr))
            {
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EQ.ePort.protectionLOCWriteMask = 0x00005050;
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EQ.ePort.protectionSwitchingRXExceptionConfig = 0x00005054;
            }
            else
            {
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EQ.ePort.protectionLOCWriteMask = 0x00005054;
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EQ.ePort.protectionSwitchingRXExceptionConfig = 0x00005058;
            }

        }/*end of unit ePort */


        {/*start of unit criticalECCCntrs */
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EQ.criticalECCCntrs.criticalECCConfig = 0x00000074;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EQ.criticalECCCntrs.BMClearCriticalECCErrorCntr = 0x00000078;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EQ.criticalECCCntrs.BMClearCriticalECCErrorParams = 0x0000007c;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EQ.criticalECCCntrs.BMIncCriticalECCErrorCntr = 0x00000088;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EQ.criticalECCCntrs.BMIncCriticalECCErrorParams = 0x0000008c;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EQ.criticalECCCntrs.BMCriticalECCInterruptCause = 0x000000a0;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EQ.criticalECCCntrs.BMCriticalECCInterruptMask = 0x000000a4;

        }/*end of unit criticalECCCntrs */


        {/*start of unit CPUTargetDeviceConfig */
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EQ.CPUTargetDeviceConfig.CPUTargetDeviceConfigReg0 = 0x00000110;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EQ.CPUTargetDeviceConfig.CPUTargetDeviceConfigReg1 = 0x00000114;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EQ.CPUTargetDeviceConfig.CPUTargetDeviceConfigReg2 = 0x00000118;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EQ.CPUTargetDeviceConfig.CPUTargetDeviceConfigReg3 = 0x0000011c;

        }/*end of unit CPUTargetDeviceConfig */


        {/*start of unit applicationSpecificCPUCodes */
            {/*7c00+n*0x4*/
                GT_U32    n;
                for(n = 0 ; n <= 3 ; n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EQ.applicationSpecificCPUCodes.IPProtocolCPUCodeEntry[n] =
                        0x7c00+n*0x4;
                }/* end of loop n */
            }/*7c00+n*0x4*/
            {/*7000+n*0x4*/
                GT_U32    n;
                for(n = 0 ; n <= 15 ; n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EQ.applicationSpecificCPUCodes.TCPUDPDestPortRangeCPUCodeEntryWord0[n] =
                        0x7000+n*0x4;
                }/* end of loop n */
            }/*7000+n*0x4*/
            {/*7800+n*0x4*/
                GT_U32    n;
                for(n = 0 ; n <= 15 ; n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EQ.applicationSpecificCPUCodes.TCPUDPDestPortRangeCPUCodeEntryWord1[n] =
                        0x7800+n*0x4;
                }/* end of loop n */
            }/*7800+n*0x4*/
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EQ.applicationSpecificCPUCodes.IPProtocolCPUCodeValidConfig = 0x00007c10;

        }/*end of unit applicationSpecificCPUCodes */

    }/*end of unit EQ */


    if(isDxControlPipe)
    {/*start of unit LPM */
        if(!SMEM_CHT_IS_SIP6_GET(devObjPtr))
        {
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->LPM.globalConfig       = 0x00D00000;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->LPM.directAccessMode   = 0x00D00010;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->LPM.exceptionStatus    = 0x00D00020;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->LPM.LPMInterruptsCause = 0x00D00100;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->LPM.LPMInterruptsMask  = 0x00D00110;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->LPM.LPMGeneralIntCause = 0x00D00120;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->LPM.LPMGeneralIntMask  = 0x00D00130;
        }
        else
        {
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->LPM.globalConfig       = 0x00F00000;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->LPM.directAccessMode   = 0x00F00010;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->LPM.exceptionStatus    = 0x00F00020;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->LPM.LPMGeneralIntCause = 0x00F00120;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->LPM.LPMGeneralIntMask  = 0x00F00130;

        }

    }/*end of unit LPM */


    if(isDxControlPipe)
    {/*start of unit TCAM */
        {/*start of unit tcamInterrupts */
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TCAM.tcamInterrupts.TCAMInterruptsSummaryMask = 0x00501000;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TCAM.tcamInterrupts.TCAMInterruptsSummaryCause = 0x00501004;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TCAM.tcamInterrupts.TCAMInterruptMask = 0x00501008;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TCAM.tcamInterrupts.TCAMInterruptCause = 0x0050100c;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TCAM.tcamInterrupts.tcamParityErrorAddr = 0x00501010;

        }/*end of unit tcamInterrupts */


        {/*start of unit tcamArrayConf */
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TCAM.tcamArrayConf.tcamActiveFloors = 0x00504010;

        }/*end of unit tcamArrayConf */


        {/*start of unit parityDaemonRegs */
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TCAM.parityDaemonRegs.parityDaemonCtrl = 0x00503000;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TCAM.parityDaemonRegs.parityDaemonWriteAdressStart = 0x00503004;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TCAM.parityDaemonRegs.parityDaemonWriteAdressEnd = 0x00503008;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TCAM.parityDaemonRegs.parityDaemonWriteAdressJump = 0x0050300c;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TCAM.parityDaemonRegs.parityDaemonReadAdressStart = 0x00503010;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TCAM.parityDaemonRegs.parityDaemonReadAdressEnd = 0x00503014;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TCAM.parityDaemonRegs.parityDaemonReadAdressJump = 0x00503018;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TCAM.parityDaemonRegs.parityDaemonDefaultWriteDataX0 = 0x0050301c;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TCAM.parityDaemonRegs.parityDaemonDefaultWriteDataX1 = 0x00503020;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TCAM.parityDaemonRegs.parityDaemonDefaultWriteDataY0 = 0x00503028;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TCAM.parityDaemonRegs.parityDaemonDefaultWriteDataX2 = 0x00503024;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TCAM.parityDaemonRegs.parityDaemonDefaultWriteDataY1 = 0x0050302c;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TCAM.parityDaemonRegs.parityDaemonDefaultWriteDataY2 = 0x00503030;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TCAM.parityDaemonRegs.parityDaemonStatus = 0x0050303c;

        }/*end of unit parityDaemonRegs */


        {/*start of unit mgAccess */
            GT_U32  maxGroups = isSip6_10 ? 0 : isSip6 ? 3 : 4;

            {/*5021a8+n*0x4*/
                GT_U32    n;
                for(n = 0 ; n <= maxGroups ; n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TCAM.mgAccess.mgKeySizeAndTypeGroup[n] =
                        0x5021a8+n*0x4;
                }/* end of loop n */
            }/*5021a8+n*0x4*/
            {/*5021bc+i*0x4+n*0x14*/
                GT_U32    n,i;
                GT_U32  maxGroupId = devObjPtr->numofTcamClients;
                for(n = 0 ; n < maxGroupId ; n++) {
                    for(i = 0 ; i <= 3 ; i++) {
                        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TCAM.mgAccess.mgHitGroupHitNum[n][i] =
                            0x5021bc+i*0x4+n*0x14;
                    }/* end of loop i */
                }/* end of loop n */
            }/*5021bc+i*0x4+n*0x14*/
            {/*502004+i*0x4+n*0x54*/
                GT_U32    n,i;
                for(n = 0 ; n <= maxGroups ; n++) {
                    for(i = 0 ; i <= (GT_U32)(isSip6_10 ? 19 : 20) ; i++) {
                        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TCAM.mgAccess.mgKeyGroupChunk[n][i] =
                            0x502004+i*0x4+n*0x54;
                    }/* end of loop i */
                }/* end of loop n */
            }/*502004+i*0x4+n*0x54*/
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TCAM.mgAccess.mgCompRequest = 0x00502000;
            if(!isSip6_10)
            {
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TCAM.mgAccess.mgKeyTypeUpdateReq = 0x00502300;
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TCAM.mgAccess.keyTypeData = 0x00502304;
            }

        }/*end of unit mgAccess */


        {/*500000+n*0x4*/
            GT_U32    n;
            for(n = 0 ; n <= 4 ; n++) {
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TCAM.groupClientEnable[n] =
                    0x500000+n*0x4;
            }/* end of loop n */
        }/*500000+n*0x4*/
        {/*500100+n*0x8*/
            GT_U32    n;
            for(n = 0 ; n < devObjPtr->tcamNumOfFloors ; n++) {
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TCAM.tcamHitNumAndGroupSelFloor[n] =
                    0x500100+n*0x8;
            }/* end of loop n */
        }/*500100+n*0x8*/

        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TCAM.tcamGlobalConfig = 0x00507008;

        if (isSip6)
        {/*start of unit exactMatchProfilesTable */
            if (isSip6_10)
            {
                {/*507080+n*0x4*/
                    GT_U32    n;
                    for(n = 0 ; n <= 0 ; n++) {
                        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TCAM.exactMatchProfilesTable.exactMatchPortMapping[n] =
                            0x507080+n*0x4;
                    }/* end of loop n */
                }/*507080+n*0x4*/
            }
            else
            {
                {/*507080+n*0x4*/
                    GT_U32    n;
                    for(n = 0 ; n <= 1 ; n++) {
                        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TCAM.exactMatchProfilesTable.exactMatchPortMapping[n] =
                            0x507080+n*0x4;
                    }/* end of loop n */
                }/*507080+n*0x4*/
            }
            {/*507088+32*4*(n-1)*/
                GT_U32    n;
                for(n = 1 ; n <= 15 ; n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TCAM.exactMatchProfilesTable.exactMatchProfileTableControl[n] =
                        0x507088+32*4*(n-1);
                }/* end of loop n */
            }/*507088+32*4*(n-1)*/
            {/*50708C+4*i+32*4*(n-1)*/
                GT_U32    n,i;
                for(n = 1 ; n <= 15 ; n++) {
                    for(i = 0 ; i <= 11 ; i++) {
                        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TCAM.exactMatchProfilesTable.exactMatchProfileTableMaskData[n][i] =
                            0x50708C+4*i+32*4*(n-1);
                    }/* end of loop i */
                }/* end of loop n */
            }/*50708C+4*i+32*4*(n-1)*/
            {/*5070C0+4*i+32*4*(n-1)*/
                GT_U32    n,i;
                for(n = 1 ; n <= 15 ; n++) {
                    for(i = 0 ; i <= 7 ; i++) {
                        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TCAM.exactMatchProfilesTable.exactMatchProfileTableDefaultActionData[n][i] =
                            0x5070C0+4*i+32*4*(n-1);
                    }/* end of loop i */
                }/* end of loop n */
            }/*5070C0+4*i+32*4*(n-1)*/

        }/*end of unit exactMatchProfilesTable */

        if (isSip6_10)
        {/*508000+4*i+32*4*n*/
            GT_U32    n;
            GT_U32    i;
            for(n = 0 ; n <= 15 ; n++) {
                for(i = 0 ; i <= 31 ; i++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TCAM.exactMatchActionAssignment[n][i] =
                        0x508000+4*i+32*4*n;
                }/* end of loop i */
            }/* end of loop n */
        }/*508000+4*i+32*4*n*/
        else
        {
            if (isSip6)
            {/*508000+4*i+32*4*n*/
                GT_U32    n;
                GT_U32    i;
                for(n = 0 ; n <= 15 ; n++) {
                    for(i = 0 ; i <= 30 ; i++) {
                        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TCAM.exactMatchActionAssignment[n][i] =
                            0x508000+4*i+32*4*n;
                    }/* end of loop i */
                }/* end of loop n */
            }/*508000+4*i+32*4*n*/
        }

    }/*end of unit TCAM */


    if(isDxControlPipe && ! SMEM_CHT_IS_SIP6_GET(devObjPtr))
    {/*start of unit EGF_eft */
        {/*start of unit MCFIFO */
            {/*start of unit MCFIFOConfigs */
                if(SMEM_CHT_IS_SIP5_20_GET(devObjPtr))
                {/*2200+4*n*/
                    GT_U32    n;
                    for(n = 0 ; n <= 31 ; n++) {
                        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EGF_eft.MCFIFO.MCFIFOConfigs.portToMcFIFOAttributionReg[n] =
                            0x2200+4*n;
                    }/* end of loop n */
                }/*2200+4*n*/
                else
                {/*21f0+4*n*/
                    GT_U32    n;
                    for(n = 0 ; n <= 15 ; n++) {
                        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EGF_eft.MCFIFO.MCFIFOConfigs.portToMcFIFOAttributionReg[ n] =
                            0x21f0+4*n;
                    }/* end of loop n */
                }/*21f0+4*n*/
                if(SMEM_CHT_IS_SIP5_20_GET(devObjPtr))
                {/*2280+4*n*/
                    GT_U32    n;
                    for(n = 0 ; n <= 15 ; n++) {
                        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EGF_eft.MCFIFO.MCFIFOConfigs.portToHemisphereMapReg[n] =
                            0x2280+4*n;
                    }/* end of loop n */
                }/*2280+4*n*/
                else
                {/*2240+4*n*/
                    GT_U32    n;
                    for(n = 0 ; n <= 7 ; n++) {
                        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EGF_eft.MCFIFO.MCFIFOConfigs.portToHemisphereMapReg[n] =
                            0x2240+4*n;
                    }/* end of loop n */
                }/*2240+4*n*/
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EGF_eft.MCFIFO.MCFIFOConfigs.MCFIFOGlobalConfigs = 0x00002100;
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EGF_eft.MCFIFO.MCFIFOConfigs.MCFIFOArbiterWeights0 = 0x00002108;
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EGF_eft.MCFIFO.MCFIFOConfigs.lossyDropAndPfcUcThreshold = 0x00002300;

            }/*end of unit MCFIFOConfigs */


        }/*end of unit MCFIFO */


        {/*start of unit global */
            {/*1150+4*n*/
                GT_U32    n;
                GT_U32    nMax;

                nMax = SMEM_CHT_IS_SIP5_20_GET(devObjPtr) ? 128 : 64;
                for(n = 0 ; n < nMax ; n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EGF_eft.global.cpuPortMapOfReg[n] =
                        0x1150+4*n;
                }/* end of loop n */
            }/*1150+4*n*/
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EGF_eft.global.cpuPortDist = 0x00001000;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EGF_eft.global.portIsolationLookup0 = 0x00001020;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EGF_eft.global.portIsolationLookup1 = 0x00001030;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EGF_eft.global.EFTInterruptsCause = 0x000010a0;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EGF_eft.global.EFTInterruptsMask = 0x000010b0;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EGF_eft.global.ECCConf = 0x00001130;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EGF_eft.global.ECCStatus = 0x00001140;

        }/*end of unit global */


        {/*start of unit egrFilterConfigs */
            {/*100f0+0x4*n*/
                GT_U32    n;
                GT_U32    nMax = SMEM_CHT_IS_SIP5_20_GET(devObjPtr) ? 15 : 7;
                GT_U32    offset = SMEM_CHT_IS_SIP5_20_GET(devObjPtr) ? 0x10180 : 0x100f0;
                for(n = 0 ; n <= nMax ; n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EGF_eft.egrFilterConfigs.enFCTriggerByCNFrameOnPort[n] =
                        offset+0x4*n;
                }/* end of loop n */
            }/*100f0+0x4*n*/
            {/*100d0+0x4*n*/
                GT_U32    n;
                GT_U32    nMax = SMEM_CHT_IS_SIP5_20_GET(devObjPtr) ? 15 : 7;
                GT_U32    offset = SMEM_CHT_IS_SIP5_20_GET(devObjPtr) ? 0x10140 : 0x100d0;
                for(n = 0 ; n <= nMax ; n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EGF_eft.egrFilterConfigs.enCNFrameTxOnPort[n] =
                        offset+0x4*n;
                }/* end of loop n */
            }/*100d0+0x4*n*/
            {/*10050+0x4*n*/
                GT_U32    n;
                GT_U32    nMax = SMEM_CHT_IS_SIP5_20_GET(devObjPtr) ? 15 : 7;
                GT_U32    offset = SMEM_CHT_IS_SIP5_20_GET(devObjPtr) ? 0x100c0 : 0x10050;
                for(n = 0 ; n <= nMax ; n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EGF_eft.egrFilterConfigs.unregedBCFilterEn[n] =
                        offset+0x4*n;
                }/* end of loop n */
            }/*10050+0x4*n*/
            {/*10010+0x4*n*/
                GT_U32    n;
                GT_U32    nMax = SMEM_CHT_IS_SIP5_20_GET(devObjPtr) ? 15 : 7;
                GT_U32    offset = SMEM_CHT_IS_SIP5_20_GET(devObjPtr) ? 0x10040 : 0x10010;
                for(n = 0 ; n <= nMax ; n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EGF_eft.egrFilterConfigs.unknownUCFilterEn[n] =
                        offset+0x4*n;
                }/* end of loop n */
            }/*10010+0x4*n*/
            {/*10030+0x4*n*/
                GT_U32    n;
                GT_U32    nMax = SMEM_CHT_IS_SIP5_20_GET(devObjPtr) ? 15 : 7;
                GT_U32    offset = SMEM_CHT_IS_SIP5_20_GET(devObjPtr) ? 0x10080 : 0x10030;
                for(n = 0 ; n <= nMax ; n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EGF_eft.egrFilterConfigs.unregedMCFilterEn[n] =
                        offset+0x4*n;
                }/* end of loop n */
            }/*10030+0x4*n*/
            {/*100b0+0x4*n*/
                GT_U32    n;
                GT_U32    nMax = SMEM_CHT_IS_SIP5_20_GET(devObjPtr) ? 15 : 7;
                GT_U32    offset = SMEM_CHT_IS_SIP5_20_GET(devObjPtr) ? 0x10100 : 0x100b0;
                for(n = 0 ; n <= nMax ; n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EGF_eft.egrFilterConfigs.OAMLoopbackFilterEn[n] =
                        offset+0x4*n;
                }/* end of loop n */
            }/*100b0+0x4*n*/
            if(!SMEM_CHT_IS_SIP5_20_GET(devObjPtr))
            {/*10120+4*n*/
                GT_U32    n;
                GT_U32    nMax =  63;
                GT_U32    offset = 0x10120;
                for(n = 0 ; n <= nMax ; n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EGF_eft.egrFilterConfigs.virtual2PhysicalPortRemap[n] =
                        offset+4*n;
                }/* end of loop n */
            }/*10120+4*n*/
            if(!SMEM_CHT_IS_SIP5_20_GET(devObjPtr))
            {/*10220+4*n*/
                GT_U32    n;
                GT_U32    nMax = 15;
                GT_U32    offset = 0x10220;
                for(n = 0 ; n <= nMax ; n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EGF_eft.egrFilterConfigs.physicalPortLinkStatusMask[n] =
                        offset+4*n;
                }/* end of loop n */
            }/*10220+4*n*/
            else
            {/*101c0+4*n*/
                GT_U32    n;
                GT_U32    nMax = 15;
                GT_U32    offset = 0x101c0;
                for(n = 0 ; n <= nMax ; n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EGF_eft.egrFilterConfigs.physicalPortLinkStatusMask[n] =
                        offset+4*n;
                }/* end of loop n */
            }/*101c0+4*n*/
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EGF_eft.egrFilterConfigs.egrFiltersGlobal = 0x00010000;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EGF_eft.egrFilterConfigs.egrFiltersEnable = 0x00010004;

        }/*end of unit egrFilterConfigs */


        {/*start of unit deviceMapConfigs */
            {/*3000+4*p*/
                GT_U32    p;
                for(p = 0 ; p <= 7 ; p++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EGF_eft.deviceMapConfigs.devMapTableAddrConstructProfile[p] =
                        0x3000+4*p;
                }/* end of loop p */
            }/*3000+4*p*/
            {/*3020+0x4*n*/
                GT_U32    n;
                GT_U32    nMax = SMEM_CHT_IS_SIP5_20_GET(devObjPtr) ? 15 : 7;
                for(n = 0 ; n <= nMax ; n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EGF_eft.deviceMapConfigs.localSrcPortMapOwnDevEn[n] =
                        0x3020+0x4*n;
                }/* end of loop n */
            }/*3020+0x4*n*/
            {/*3040+0x4*n*/
                GT_U32    n;
                GT_U32    nMax = SMEM_CHT_IS_SIP5_20_GET(devObjPtr) ? 15 : 7;
                GT_U32    offset = SMEM_CHT_IS_SIP5_20_GET(devObjPtr) ? 0x3060 : 0x3040;
                for(n = 0 ; n <= nMax ; n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EGF_eft.deviceMapConfigs.localTrgPortMapOwnDevEn[n] =
                        offset+0x4*n;
                }/* end of loop n */
            }/*3040+0x4*n*/
            {/*3060+4*n*/
                GT_U32    n;
                GT_U32    nMax = SMEM_CHT_IS_SIP5_20_GET(devObjPtr) ? 63 : 255;
                GT_U32    offset = SMEM_CHT_IS_SIP5_20_GET(devObjPtr) ? 0x3100 : 0x3060;
                for(n = 0 ; n <= nMax ; n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EGF_eft.deviceMapConfigs.portAddrConstructMode[n] =
                        offset+4*n;
                }/* end of loop n */
            }/*3060+4*n*/

        }/*end of unit deviceMapConfigs */

        {/*start of unit control */

            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EGF_eft.counters.eftDescEcc1ErrorCntr = 0x00001120;

        }/*end of unit control */

    }/*end of unit EGF_eft */


    if(isDxControlPipe && ! SMEM_CHT_IS_SIP6_GET(devObjPtr))
    {/*start of unit EGF_qag */
        {/*start of unit uniMultiArb */
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EGF_qag.uniMultiArb.uniMultiCtrl = 0x00f00100;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EGF_qag.uniMultiArb.uniWeights = 0x00f00104;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EGF_qag.uniMultiArb.multiWeights = 0x00f00108;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EGF_qag.uniMultiArb.multiShaperCtrl = 0x00f0010c;

        }/*end of unit uniMultiArb */


        {/*start of unit global */
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EGF_qag.global.QAGInterruptsCause = 0x00f00010;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EGF_qag.global.QAGInterruptsMask = 0x00f00020;

        }/*end of unit global */

        {/*start of unit distributor */
            if(! SMEM_CHT_IS_SIP5_20_GET(devObjPtr))
            {/*e00a04+4*n*/
                GT_U32    n;
                for(n = 0 ; n <= 7 ; n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EGF_qag.distributor.useVlanTag1ForTagStateReg[n] =
                        0xe00a04+4*n;
                }/* end of loop n */
            }/*e00a04+4*n*/
            {/*e00180+0x4*n*/
                GT_U32    n;
                for(n = 0 ; n <= 7 ; n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EGF_qag.distributor.toAnalyzerLoopback[n] =
                        0xe00180+0x4*n;
                }/* end of loop n */
            }/*e00180+0x4*n*/
            if(! SMEM_CHT_IS_SIP5_20_GET(devObjPtr))
            {/*e001e0+4*n*/
                GT_U32    n;
                for(n = 0 ; n <= 31 ; n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EGF_qag.distributor.TCRemap[n] =
                        0xe001e0+4*n;
                }/* end of loop n */
            }/*e001e0+4*n*/
            if(! SMEM_CHT_IS_SIP5_20_GET(devObjPtr))
            {/*e00050+0x4*n*/
                GT_U32    n;
                for(n = 0 ; n <= 7 ; n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EGF_qag.distributor.stackRemapEn[n] =
                        0xe00050+0x4*n;
                }/* end of loop n */
            }/*e00050+0x4*n*/
            if(! SMEM_CHT_IS_SIP5_20_GET(devObjPtr))
            {/*e00130+0x4*n*/
                GT_U32    n;
                for(n = 0 ; n <= 15 ; n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EGF_qag.distributor.perTrgPhyPortLbProfile[n] =
                        0xe00130+0x4*n;
                }/* end of loop n */
            }/*e00130+0x4*n*/
            if(! SMEM_CHT_IS_SIP5_20_GET(devObjPtr))
            {/*e000f0+0x4*n*/
                GT_U32    n;
                for(n = 0 ; n <= 15 ; n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EGF_qag.distributor.perSrcPhyPortLbProfile[n] =
                        0xe000f0+0x4*n;
                }/* end of loop n */
            }/*e000f0+0x4*n*/
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EGF_qag.distributor.loopbackForwardingGlobalConfig = 0x00e00a00;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EGF_qag.distributor.loopbackEnableConfig[0] = 0x00e00170;
            if(! SMEM_CHT_IS_SIP5_20_GET(devObjPtr))
            {/*e001c0+0x4*n*/
                GT_U32    n;
                for(n = 0 ; n <= 7 ; n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EGF_qag.distributor.evlanMirrEnable[n] =
                        0xe001c0+0x4*n;
                }/* end of loop n */
            }/*e001c0+0x4*n*/
            if(! SMEM_CHT_IS_SIP5_20_GET(devObjPtr))
            {/*e005fc+4*n*/
                GT_U32    n;
                for(n = 0 ; n <= 15 ; n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EGF_qag.distributor.DPRemap[n] =
                        0xe005fc+4*n;
                }/* end of loop n */
            }/*e005fc+4*n*/
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EGF_qag.distributor.distributorGeneralConfigs = 0x00e00000;

        }/*end of unit distributor */


    }/*end of unit EGF_qag */



    if(isDxControlPipe && ! SMEM_CHT_IS_SIP6_GET(devObjPtr))
    {/*start of unit EGF_sht */
        {/*start of unit global */
            {/*60200d0+0x4*n*/
                GT_U32    n;
                GT_U32    nMax = (shtNumPorts-1)/32;
                GT_U32    offset = SMEM_CHT_IS_SIP5_20_GET(devObjPtr) ? 0x6020180 : 0x60200d0;
                for(n = 0 ; n <= nMax ; n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EGF_sht.global.eportStpStateMode[n] =
                        offset+0x4*n;
                }/* end of loop n */
            }/*60200d0+0x4*n*/
            {/*60200b0+0x4*n*/
                GT_U32    n;
                GT_U32    nMax = (shtNumPorts-1)/32;
                GT_U32    offset = SMEM_CHT_IS_SIP5_20_GET(devObjPtr) ? 0x6020140 : 0x60200b0;
                for(n = 0 ; n <= nMax ; n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EGF_sht.global.eportEvlanFilteringEnable[n] =
                        offset+0x4*n;
                }/* end of loop n */
            }/*60200b0+0x4*n*/
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EGF_sht.global.SHTInterruptsCause = 0x06020010;
            {/*6020030+0x4*n*/
                GT_U32    n;
                GT_U32    nMax = (shtNumPorts-1)/32;
                GT_U32    offset = SMEM_CHT_IS_SIP5_20_GET(devObjPtr) ? 0x6020040 : 0x6020030;
                for(n = 0 ; n <= nMax ; n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EGF_sht.global.UCSrcIDFilterEn[n] =
                        offset+0x4*n;
                }/* end of loop n */
            }/*6020030+0x4*n*/
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EGF_sht.global.SHTEgrFiltersEnable = 0x06020008;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EGF_sht.global.MESHIDConfigs = 0x06020004;
            {/*6020270+0x4*n*/
                GT_U32    n;
                GT_U32    nMax = (shtNumPorts-1)/32;
                GT_U32    offset = SMEM_CHT_IS_SIP5_20_GET(devObjPtr) ? 0x60208c0 : 0x6020270;
                for(n = 0 ; n <= nMax ; n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EGF_sht.global.MCLocalEn[n] =
                        offset+0x4*n;
                }/* end of loop n */
            }/*6020270+0x4*n*/
            {/*6020240+0x4*n*/
                GT_U32    n;
                GT_U32    nMax = (shtNumPorts-1)/32;
                GT_U32    offset = SMEM_CHT_IS_SIP5_20_GET(devObjPtr) ? 0x6020880 : 0x6020240;
                for(n = 0 ; n <= nMax ; n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EGF_sht.global.ignorePhySrcMcFilterEn[n] =
                        offset+0x4*n;
                }/* end of loop n */
            }/*6020240+0x4*n*/
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EGF_sht.global.eportVlanEgrFiltering =
                                SMEM_CHT_IS_SIP5_20_GET(devObjPtr) ? 0x0602000c : 0x06020310;
            {/*60200f0+0x4*n*/
                GT_U32    n;
                GT_U32    nMax = (shtNumPorts-1)/32;
                GT_U32    offset = SMEM_CHT_IS_SIP5_20_GET(devObjPtr) ? 0x60201c0 : 0x60200f0;
                for(n = 0 ; n <= nMax ; n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EGF_sht.global.eportStpState[n] =
                        offset+0x4*n;
                }/* end of loop n */
            }/*60200f0+0x4*n*/
            {/*6020190+0x4*n*/
                GT_U32    n;
                GT_U32    nMax = (shtNumPorts-1)/32;
                GT_U32    offset = SMEM_CHT_IS_SIP5_20_GET(devObjPtr) ? 0x60202c0 : 0x6020190;
                for(n = 0 ; n <= nMax ; n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EGF_sht.global.eportOddOnlySrcIdFilterEnable[n] =
                        offset+0x4*n;
                }/* end of loop n */
            }/*6020190+0x4*n*/
            {/*6020090+0x4*n*/
                GT_U32    n;
                GT_U32    nMax = (shtNumPorts-1)/32;
                GT_U32    offset = SMEM_CHT_IS_SIP5_20_GET(devObjPtr) ? 0x6020100 : 0x6020090;
                for(n = 0 ; n <= nMax ; n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EGF_sht.global.eportRoutedForwardRestricted[n] =
                        offset+0x4*n;
                }/* end of loop n */
            }/*6020090+0x4*n*/
            {/*6020130+0x4*n*/
                GT_U32    n;
                GT_U32    nMax = (2*shtNumPorts-1)/32;
                GT_U32    offset = SMEM_CHT_IS_SIP5_20_GET(devObjPtr) ? 0x6020200 : 0x6020130;
                for(n = 0 ; n <= nMax ; n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EGF_sht.global.eportPortIsolationMode[n] =
                        offset+0x4*n;
                }/* end of loop n */
            }/*6020130+0x4*n*/
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EGF_sht.global.SHTGlobalConfigs = 0x06020000;
            {/*6020320+n * 0x4*/
                GT_U32    n;
                GT_U32    nMax = (4*shtNumPorts-1)/32;
                GT_U32    offset = SMEM_CHT_IS_SIP5_20_GET(devObjPtr) ? 0x6020300 : 0x6020320;
                for(n = 0 ; n <= nMax ; n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EGF_sht.global.ePortMeshID[n] =
                        offset+n * 0x4;
                }/* end of loop n */
            }/*6020320+n * 0x4*/
            {/*6020170+0x4*n*/
                GT_U32    n;
                GT_U32    nMax = (shtNumPorts-1)/32;
                GT_U32    offset = SMEM_CHT_IS_SIP5_20_GET(devObjPtr) ? 0x6020280 : 0x6020170;
                for(n = 0 ; n <= nMax ; n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EGF_sht.global.eportIPMCRoutedFilterEn[n] =
                        offset+0x4*n;
                }/* end of loop n */
            }/*6020170+0x4*n*/
            {/*6020050+0x4*n*/
                GT_U32    n;
                GT_U32    nMax = (shtNumPorts-1)/32;
                GT_U32    offset = SMEM_CHT_IS_SIP5_20_GET(devObjPtr) ? 0x6020080 : 0x6020050;
                for(n = 0 ; n <= nMax ; n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EGF_sht.global.eportFromCPUForwardRestricted[n] =
                        offset+0x4*n;
                }/* end of loop n */
            }/*6020050+0x4*n*/
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EGF_sht.global.SHTInterruptsMask = 0x06020020;
            {/*6020070+0x4*n*/
                GT_U32    n;
                GT_U32    nMax = (shtNumPorts-1)/32;
                GT_U32    offset = SMEM_CHT_IS_SIP5_20_GET(devObjPtr) ? 0x60200c0 : 0x6020070;
                for(n = 0 ; n <= nMax ; n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EGF_sht.global.eportBridgedForwardRestricted[n] =
                        offset+0x4*n;
                }/* end of loop n */
            }/*6020070+0x4*n*/

        }/*end of unit global */


    }/*end of unit EGF_sht */


    if(isDxControlPipe)
    {/*start of unit HA */
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->HA.invalidPTPHeaderCntr = 0x00000014;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->HA.TRILLEtherType = 0x00000558;
        {/*500+n*4*/
            GT_U32    n;
            for(n = 0 ; n <= 7 ; n++) {
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->HA.TPIDConfigReg[n] =
                    0x500+n*4;
                if(SMEM_CHT_IS_SIP6_30_GET(devObjPtr))
                {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->HA.egrTPIDEnhClasificationExt1[n] =
                        0x520+0x4*n;
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->HA.egrTPIDEnhClasificationExt2[n] =
                        0x590+0x4*n;
                }
            }/* end of loop n */
        }/*500+n*4*/
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->HA.routerMACSABaseReg1 = 0x00000144;
        {/*20+(w-1)*4*/
            GT_U32    w;
            for(w = 1 ; w <= 15 ; w++) {
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->HA.PWCtrlWord[w-1] =
                    0x20+(w-1)*4;
            }/* end of loop w */
        }/*20+(w-1)*4*/
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->HA.PWETreeEnable = 0x00000060;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->HA.invalidPTPOutgoingPiggybackCntr = 0x00000018;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->HA.PTPTimestampTagEthertype = 0x0000000c;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->HA.PTPAndTimestampingExceptionConfig = 0x00000010;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->HA.MPLSEtherType = 0x00000550;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->HA.MPLSChannelTypeProfileReg7 = 0x0000016c;
        if(SMEM_CHT_IS_SIP6_GET(devObjPtr))
        {
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->HA.PTPExceptionCPUCodeConfig = 0x000000A0;
        }

        {/*150+4*r*/
            GT_U32    r;
            for(r = 0 ; r <= 6 ; r++) {
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->HA.MPLSChannelTypeProfileReg[r] =
                    0x150+4*r;
            }/* end of loop r */
        }/*150+4*r*/
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->HA.IPLengthOffset = 0x00000560;
        if (isSip6_10)
        {
            {/*900+4*n*/
                GT_U32    n;
                for(n = 0 ; n <= 63 ; n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->HA.egrUDPPortRangeComparatorConfig[n] =
                        0x900+4*n;
                }/* end of loop n */
            }/*900+4*n*/
        }
        else
        {
            {/*120+4*n*/
                GT_U32    n;
                for(n = 0 ; n <= 7 ; n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->HA.egrUDPPortRangeComparatorConfig[n] =
                        0x120+4*n;
                }/* end of loop n */
            }/*120+4*n*/
        }
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->HA.invalidPTPIncomingPiggybackCntr = 0x0000001c;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->HA.egrPolicyDIPSolicitationData1 = 0x00000084;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->HA.ingrAnalyzerVLANTagConfig = 0x00000408;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->HA.IEtherType = 0x00000554;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->HA.haGlobalConfig1 = 0x00000004;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->HA.haGlobalConfig = 0x00000000;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->HA.HAInterruptMask = 0x00000304;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->HA.HAInterruptCause = 0x00000300;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->HA.ethernetOverGREProtocolType = 0x0000055c;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->HA.routerMACSABaseReg0 = 0x00000140;
        if (isSip6_10)
        {
            {/*0x800+4*n*/
                GT_U32    n;
                for(n = 0 ; n <= 63 ; n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->HA.egrTCPPortRangeComparatorConfig[n] =
                        0x800+4*n;
                }/* end of loop n */
            }/*0x800+4*n*/
        }
        else
        {
            {/*100+4*n*/
                GT_U32    n;
                for(n = 0 ; n <= 7 ; n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->HA.egrTCPPortRangeComparatorConfig[n] =
                        0x100+4*n;
                }/* end of loop n */
            }/*100+4*n*/
        }
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->HA.egrPolicyDIPSolicitationMask3 = 0x0000009c;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->HA.egrPolicyDIPSolicitationMask2 = 0x00000098;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->HA.egrPolicyDIPSolicitationMask1 = 0x00000094;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->HA.egrPolicyDIPSolicitationMask0 = 0x00000090;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->HA.egrPolicyDIPSolicitationData3 = 0x0000008c;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->HA.egrPolicyDIPSolicitationData2 = 0x00000088;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->HA.tunnelStartFragmentIDConfig = 0x00000410;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->HA.egrPolicyDIPSolicitationData0 = 0x00000080;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->HA.egrAnalyzerVLANTagConfig = 0x0000040c;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->HA.dataECCStatus = 0x00000064;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->HA.CPIDReg1 = 0x00000434;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->HA.CPIDReg0 = 0x00000430;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->HA.congestionNotificationConfig = 0x00000420;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->HA.CNMHeaderConfig = 0x00000424;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->HA.BPEConfigReg2 = 0x0000006c;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->HA.BPEConfigReg1 = 0x00000068;

        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->HA.haMetalFix = 0x00000710;
        if(SMEM_CHT_IS_SIP5_25_GET(devObjPtr))
        {
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->HA.haGlobalConfig2 = 0x00000714;
        }

    }/*end of unit HA */


    if(isDxControlPipe)
    {/*start of unit MLL */
        {/*start of unit multiTargetRateShape */
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->MLL.multiTargetRateShape.multiTargetRateShapeConfig = 0x00000210;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->MLL.multiTargetRateShape.multiTargetRateShapeConfig2 = 0x00000218;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->MLL.multiTargetRateShape.multiTargetMTUReg = 0x0000021c;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->MLL.multiTargetRateShape.tokenBucketBaseLine = 0x00000220;

        }/*end of unit multiTargetRateShape */


        {/*start of unit mcLinkedListMLLTables */
            {/*100+n*0x4*/
                GT_U32    n;
                for(n = 0 ; n <= 63 ; n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->MLL.mcLinkedListMLLTables.qoSProfileToMultiTargetTCQueuesReg[n] =
                        0x100+n*0x4;
                }/* end of loop n */
            }/*100+n*0x4*/
            {/*400+n*0x4*/
                GT_U32    n;
                for(n = 0 ; n <= 1 ; n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->MLL.mcLinkedListMLLTables.extQoSModeMCQueueSelectionReg[n] =
                        0x400+n*0x4;
                }/* end of loop n */
            }/*400+n*0x4*/

        }/*end of unit mcLinkedListMLLTables */


        {/*start of unit multiTargetVsUcSDWRRAndStrictPriorityScheduler */
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->MLL.multiTargetVsUcSDWRRAndStrictPriorityScheduler.mcUcSDWRRAndStrictPriorityConfig = 0x00000214;

        }/*end of unit multiTargetVsUcSDWRRAndStrictPriorityScheduler */


        {/*start of unit multiTargetTCQueuesAndArbitrationConfig */
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->MLL.multiTargetTCQueuesAndArbitrationConfig.multitargetTCQueuesGlobalConfig = 0x00000004;
            if(isSip6)
            {
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->MLL.multiTargetTCQueuesAndArbitrationConfig.multitargetTCQueuesPriorityConfig = 0x00000208;
            }
            else
            {
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->MLL.multiTargetTCQueuesAndArbitrationConfig.multitargetTCQueuesStrictPriorityEnableConfig = 0x00000200;
            }
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->MLL.multiTargetTCQueuesAndArbitrationConfig.multitargetTCQueuesWeightConfig = 0x00000204;

        }/*end of unit multiTargetTCQueuesAndArbitrationConfig */


        {/*start of unit multiTargetEPortMap */
            if(isSip6)
            {
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->MLL.multiTargetEPortMap.multiTargetGlobalEPortMinValue = 0x00000310;
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->MLL.multiTargetEPortMap.multiTargetGlobalEPortMaxValue = 0x00000314;
            }
            else
            {
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->MLL.multiTargetEPortMap.multiTargetEPortValue = 0x00000300;
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->MLL.multiTargetEPortMap.multiTargetEPortMask = 0x00000304;
            }
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->MLL.multiTargetEPortMap.multiTargetEPortBase = 0x00000308;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->MLL.multiTargetEPortMap.ePortToEVIDXBase = 0x0000030c;

        }/*end of unit multiTargetEPortMap */


        {/*start of unit MLLOutInterfaceCntrs */
            {/*c84+n*0x100*/
                GT_U32    n;
                for(n = 0 ; n <= 1 ; n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->MLL.MLLOutInterfaceCntrs.L2MLLOutInterfaceCntrConfig1[n] =
                        0xc84+n*0x100;
                }/* end of loop n */
            }/*c84+n*0x100*/
            {/*c80+n*0x100*/
                GT_U32    n;
                for(n = 0 ; n <= 1 ; n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->MLL.MLLOutInterfaceCntrs.L2MLLOutInterfaceCntrConfig[n] =
                        0xc80+n*0x100;
                }/* end of loop n */
            }/*c80+n*0x100*/
            {/*e20+(n-1)*0x4*/
                GT_U32    n;
                for(n = 1 ; n <= 3 ; n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->MLL.MLLOutInterfaceCntrs.L2MLLValidProcessedEntriesCntr[n-1] =
                        0xe20+(n-1)*0x4;
                }/* end of loop n */
            }/*e20+(n-1)*0x4*/
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->MLL.MLLOutInterfaceCntrs.L2MLLTTLExceptionCntr = 0x00000e08;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->MLL.MLLOutInterfaceCntrs.L2MLLSkippedEntriesCntr = 0x00000e04;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->MLL.MLLOutInterfaceCntrs.L2MLLSilentDropCntr = 0x00000e00;
            {/*c00+n*0x100*/
                GT_U32    n;
                for(n = 0 ; n <= 1 ; n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->MLL.MLLOutInterfaceCntrs.L2MLLOutMcPktsCntr[n] =
                        0xc00+n*0x100;
                }/* end of loop n */
            }/*c00+n*0x100*/
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->MLL.MLLOutInterfaceCntrs.MLLSilentDropCntr = 0x00000800;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->MLL.MLLOutInterfaceCntrs.MLLMCFIFODropCntr = 0x00000804;
            if(isSip6)
            {
                {/*808+n*0x4*/
                    GT_U32    n;
                    for(n = 0 ; n <= 3 ; n++) {
                        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->MLL.MLLOutInterfaceCntrs.MLLMCQueueDropCnter[n] =
                            0x808+n*0x4;
                    }/* end of loop n */
                }/*808+n*0x4*/
            }
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->MLL.MLLOutInterfaceCntrs.IPMLLSkippedEntriesCntr = 0x00000b04;
            {/*900+n*0x100*/
                GT_U32    n;
                for(n = 0 ; n <= 1 ; n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->MLL.MLLOutInterfaceCntrs.IPMLLOutMcPktsCntr[n] =
                        0x900+n*0x100;
                }/* end of loop n */
            }/*900+n*0x100*/
            {/*984+n*0x100*/
                GT_U32    n;
                for(n = 0 ; n <= 1 ; n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->MLL.MLLOutInterfaceCntrs.IPMLLOutInterfaceCntrConfig1[n] =
                        0x984+n*0x100;
                }/* end of loop n */
            }/*984+n*0x100*/
            {/*980+n*0x100*/
                GT_U32    n;
                for(n = 0 ; n <= 1 ; n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->MLL.MLLOutInterfaceCntrs.IPMLLOutInterfaceCntrConfig[n] =
                        0x980+n*0x100;
                }/* end of loop n */
            }/*980+n*0x100*/

        }/*end of unit MLLOutInterfaceCntrs */


        {/*start of unit MLLGlobalCtrl */
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->MLL.MLLGlobalCtrl.MLLGlobalCtrl = 0x00000000;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->MLL.MLLGlobalCtrl.MLLLookupTriggerConfig = 0x0000000c;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->MLL.MLLGlobalCtrl.globalTTLExceptionConfig = 0x00000010;

            if(isSip6)
            {
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->MLL.MLLGlobalCtrl.L2mllGlobalEPortMinValue = 0x00000068;
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->MLL.MLLGlobalCtrl.L2mllGlobalEPortMaxValue = 0x0000006c;
            }
            else
            {
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->MLL.MLLGlobalCtrl.globalEPortRangeConfig = 0x00000014;
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->MLL.MLLGlobalCtrl.globalEPortRangeMask = 0x00000018;
            }

            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->MLL.MLLGlobalCtrl.IPMLLTableResourceSharingConfig = 0x00000020;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->MLL.MLLGlobalCtrl.L2MLLTableResourceSharingConfig = 0x00000024;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->MLL.MLLGlobalCtrl.MLLInterruptCauseReg = 0x00000030;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->MLL.MLLGlobalCtrl.MLLInterruptMaskReg = 0x00000034;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->MLL.MLLGlobalCtrl.MLLReplicatedTrafficCtrl = 0x00000040;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->MLL.MLLGlobalCtrl.sourceBasedL2MLLFiltering = 0x00000054;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->MLL.MLLGlobalCtrl.MLLEccStatus = 0x00000058;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->MLL.MLLGlobalCtrl.MLLMCSourceID = 0x00000060;

        }/*end of unit MLLGlobalCtrl */


    }/*end of unit MLL */



    if(isDxControlPipe)
    {/*start of unit PCL */
        {/*start of unit OAMConfigs */
            {/*740+4*n*/
                GT_U32    n;
                for(n = 1 ; n <= 15 ; n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PCL.OAMConfigs.channelTypeOAMOpcode[n-1] =
                        0x740+4*n;
                }/* end of loop n */
            }/*740+4*n*/
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PCL.OAMConfigs.OAMRDIConfigs = 0x00000730;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PCL.OAMConfigs.OAMMEGLevelModeConfigs = 0x00000734;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PCL.OAMConfigs.OAMRFlagConfigs = 0x00000738;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PCL.OAMConfigs.RBitAssignmentEnable = 0x0000073c;

        }/*end of unit OAMConfigs */


        {/*start of unit HASH */
            {/*5c0+4*i*/
                GT_U32    i;
                for(i = 0 ; i <= 15 ; i++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PCL.HASH.pearsonHashTable[i] =
                        0x5c0+4*i;
                }/* end of loop i */
            }/*5c0+4*i*/
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PCL.HASH.CRCHashConfig = 0x00000020;
            {/*804+0x4*n*/
                GT_U32    n;
                for(n = 0 ; n <= 19 ; n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PCL.HASH.CRC32Salt[n] =
                        0x804+0x4*n;
                }/* end of loop n */
            }/*804+0x4*n*/
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PCL.HASH.CRC32Seed = 0x00000800;
            if(isSip6_10)
            {
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PCL.HASH.CRC32SeedHash1 = 0x00000860;
                {/*864+0x4*n*/
                    GT_U32    n;
                    for(n = 0 ; n <= 19 ; n++) {
                        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PCL.HASH.CRC32SaltHash1[n] =
                                    0x864+0x4*n;
                    }/* end of loop n */
                }/*864+0x4*n*/
            }
        }/*end of unit HASH */


        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PCL.ingrPolicyGlobalConfig = 0x00000000;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PCL.PCLUnitInterruptCause = 0x00000004;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PCL.PCLUnitInterruptMask = 0x00000008;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PCL.policyEngineConfig = 0x0000000c;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PCL.policyEngineUserDefinedBytesConfig = 0x00000014;
        {/*28+0x4*n*/
            GT_U32    n;
            GT_U32    nMin = devObjPtr->isIpcl0NotValid ? 1 : 0;
            for(n = nMin ; n <= 2 ; n++) {
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PCL.IPCLReservedBitMask[n] =
                    0x28+0x4*n;
            }/* end of loop n */
        }/*28+0x4*n*/
        {/*34+4*n*/
            GT_U32    n;
            GT_U32    nMin = devObjPtr->isIpcl0NotValid ? 1 : 0;
            for(n = nMin ; n <= 2 ; n++) {
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PCL.lookupSourceIDMask[n] =
                    0x34+4*n;
            }/* end of loop n */
        }/*34+4*n*/
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PCL.policyEngineUserDefinedBytesExtConfig = 0x00000024;
        {/*74+0x4*n*/
            GT_U32    n;
            for(n = 0 ; n <= 1 ; n++) {
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PCL.TCAMKeyTypeMap[n] =
                    0x74+0x4*n;
            }/* end of loop n */
        }/*74+0x4*n*/
        {/*80+4*i*/
            GT_U32    i;
            GT_U32    nMax = isSip6 ? 31 : SMEM_CHT_IS_SIP5_20_GET(devObjPtr) ? 15 : 7;
            for(i = 0 ; i <= nMax ; i++) {
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PCL.L2L3IngrVLANCountingEnable[i] =
                    0x80+4*i;
            }/* end of loop i */
        }/*80+4*i*/
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PCL.countingModeConfig = 0x00000070;
        if (isSip6)
        {
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PCL.CRCHashModes = 0x00000100;
            if(isSip6_10)
            {
                GT_U32    i;
                for(i = 0 ; i <= 255 ; i++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PCL.exactMatchProfileIdMapEntry[i] =
                        0x900+4*i;
                }
            }
            else
            {/*200+4*i*/
                GT_U32    i;
                for(i = 0 ; i <= 127 ; i++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PCL.exactMatchProfileIdMapEntry[i] =
                        0x200+4*i;
                }
            }/* end of loop i */
        }/*200+4*i*/

        if(isSip6_30)
        {
            {/*0x110+4*n*/
                GT_U32    n;
                for(n = 0 ; n <= 3 ; n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PCL.maxSduSizeProfile[n] = 0x110+4*n;
                }/* end of loop n */
            }/*0x110+4*n*/

            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PCL.maxSduException = 0x00000130;

            {/*0x150+4*n*/
                GT_U32    n;
                GT_U32    nMin = devObjPtr->isIpcl0NotValid ? 1 : 0;
                for(n = nMin ; n <= 2 ; n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PCL.flowIdActionAssignmentModeConfig[n] =
                        0x150+4*n;
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PCL.policyGenericActionConfig[n] =
                        0x140+4*n;
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PCL.policyGlobalConfig[n] =
                        0x034+4*n;
                }/* end of loop n */
            }/*0x150+4*n*/

            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PCL.globalConfig_ext_1 = 0x00000060;
        }

    }/*end of unit PCL */


    if(isDxControlPipe)
    {/*start of unit EPCL */
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EPCL.egrPolicyGlobalConfig = 0x00000000;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EPCL.EPCLOAMPktDetection = 0x00000004;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EPCL.EPCLOAMMEGLevelMode = 0x00000008;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EPCL.EPCLOAMRDI = 0x0000000c;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EPCL.EPCLInterruptCause = 0x00000010;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EPCL.EPCLInterruptMask = 0x00000014;
        {/*30+4*s*/
            GT_U32    s;
            for(s = 0 ; s <= 4 ; s++) {
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EPCL.EPCLChannelTypeToOAMOpcodeMap[s] =
                    0x30+4*s;
            }/* end of loop s */
        }/*30+4*s*/
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EPCL.EPCLReservedBitMask = 0x00000020;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EPCL.TCAMKeyTypeMap = 0x00000074;

        if(isSip6)
        {

            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EPCL.PHAMetadataMask1 = 0x00000100;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EPCL.PHAMetadataMask2 = 0x00000104;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EPCL.PHAMetadataMask3 = 0x00000108;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EPCL.PHAMetadataMask4 = 0x0000010C;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EPCL.PHAThreadNumMask = 0x00000110;
            {/*120+4*i*/
                GT_U32    i;
                for(i = 0 ; i <= 7 ; i++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EPCL.latencyMonitoringSamplingConfig[i] =
                        0x120+4*i;
                }/* end of loop i */
            }/*120+4*i*/
            {/*140+4*i*/
                GT_U32    i;
                for(i = 0 ; i <= 7 ; i++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EPCL.latencyMonitoringSamplingCounter[i] =
                        0x140+4*i;
                }/* end of loop i */
            }/*140+4*i*/
            {/*300+4*i*/
                GT_U32    i;
                for(i = 0 ; i <= 63 ; i++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EPCL.latencyMonitoringProfile2SamplingProfile[i] =
                        0x300 + 4*i;
                }/* end of loop i */
            }/*300+4*i*/
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EPCL.latencyMonitoringControl = 0x00000164;
        }

        if(isSip6_30)
        {
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EPCL.egressPolicyGlobalConfig_ext_1 = 0x00000024;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EPCL.egressPolicyFlowIdActionAssignmentModeConfig = 0x00000028;

            {/*0x170+4*n*/
                GT_U32    n;
                for(n = 0 ; n <= 3 ; n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EPCL.egressMaxSduSizeProfile[n] = 0x170+4*n;
                }/* end of loop n */
            }/*0x170+4*n*/

            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EPCL.egressMaxSduException = 0x00000180;
        }

    }/*end of unit EPCL */

    if(isDxControlPipe)
    {/*start of unit ERMRK */
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->ERMRK.ERMRKGlobalConfig = 0x00000000;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->ERMRK.ERMRKInterruptCause = 0x00000004;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->ERMRK.ERMRKInterruptMask = 0x00000008;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->ERMRK.ERMRKPTPConfig = 0x00000010;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->ERMRK.timestampingUDPChecksumMode = 0x00000014;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->ERMRK.timestampQueueMsgType = 0x00000018;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->ERMRK.timestampQueueEntryID = 0x0000001c;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->ERMRK.PTPNTPOffset = 0x0000010c;
        {/*40+s*4*/
            GT_U32    s;
            for(s = 0 ; s <= 11 ; s++) {
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->ERMRK.ERMRKUP2UPMapTable[s] =
                    0x40+s*4;
            }/* end of loop s */
        }/*40+s*4*/
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->ERMRK.ERMRKPTPInvalidTimestampCntr = 0x00000020;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->ERMRK.ERMRKDP2CFITable = 0x00000038;
        {/*70+s*4*/
            GT_U32    s;
            for(s = 0 ; s <= 11 ; s++) {
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->ERMRK.ERMRKEXP2EXPMapTable[s] =
                    0x70+s*4;
            }/* end of loop s */
        }/*70+s*4*/
        {/*110+4*q*/
            GT_U32    q;
            for(q = 0 ; q <= 3 ; q++) {
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->ERMRK.timestampQueueEntryWord0[q] =
                    0x110+4*q;
            }/* end of loop q */
        }/*110+4*q*/
        {/*120+4*q*/
            GT_U32    q;
            for(q = 0 ; q <= 3 ; q++) {
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->ERMRK.timestampQueueEntryWord1[q] =
                    0x120+4*q;
            }/* end of loop q */
        }/*120+4*q*/
        {/*130+4*q*/
            GT_U32    q;
            for(q = 0 ; q <= 1 ; q++) {
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->ERMRK.timestampQueueEntryWord2[q] =
                    0x130+4*q;
            }/* end of loop q */
        }/*130+4*q*/
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->ERMRK.timestampQueueEntryIDClear = 0x000000e4;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->ERMRK.ECCErrorInjection = 0x00000024;

        if(isSip6_30)
        {
            {/*140+4*q*/
                GT_U32    q;
                for(q = 0 ; q <= 3 ; q++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->ERMRK.cumulativeScaleRateTaiReg0[q] = 0x140+4*q;
                }/* end of loop q */
            }/*140+4*q*/
            {/*150+4*q*/
                GT_U32    q;
                for(q = 0 ; q <= 3 ; q++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->ERMRK.cumulativeScaleRateTaiReg1[q] = 0x150+4*q;
                }/* end of loop q */
            }/*150+4*q*/
        }

    }/*end of unit ERMRK */


    if(isDxControlPipe)
    {
        GT_U32  index;/* manually added */

        for(index = 0 ; index < 2 ; index++) /* manually added */
        {/*start of unit OAMUnit */

            if(index == 0 && (SMAIN_NOT_VALID_CNS == UNIT_BASE_ADDR_GET_ALLOW_NON_EXIST_UNIT_MAC(devObjPtr,UNIT_IOAM)))
            {
                /*IOAM unit not exists*/
                continue;
            }
            if(index == 1 && (SMAIN_NOT_VALID_CNS == UNIT_BASE_ADDR_GET_ALLOW_NON_EXIST_UNIT_MAC(devObjPtr,UNIT_EOAM)))
            {
                /*EOAM unit not exists*/
                continue;
            }

            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->OAMUnit[index].OAMGlobalCtrl = 0x00000000;
            {/*e0+4*n*/
                GT_U32    n;
                for(n = 0 ; n <= 1 ; n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->OAMUnit[index].agingEntryGroupStatus[n] = isSip5_15 ? 0x900+4*n :
                        0xe0+4*n;
                }/* end of loop n */
            }/*e0+4*n*/
            {/*2c0+4*n*/
                GT_U32    n;
                for(n = 0 ; n <= 1 ; n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->OAMUnit[index].summaryExceptionGroupStatus[n] = isSip5_15 ? 0x800+4*n :
                        0x2c0+4*n;
                }/* end of loop n */
            }/*2c0+4*n*/
            {/*2a0+n*4*/
                GT_U32    n;
                for(n = 0 ; n <= 1 ; n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->OAMUnit[index].txPeriodExceptionGroupStatus[n] = isSip5_15 ? 0x700+4*n :
                        0x2a0+n*4;
                }/* end of loop n */
            }/*2a0+n*4*/
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->OAMUnit[index].sourceInterfaceMismatchCntr = isSip5_15 ? 0x00000a04 : 0x00000304;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->OAMUnit[index].OAMGlobalCtrl1 = 0x00000004;
            {/*280+4*n*/
                GT_U32    n;
                for(n = 0 ; n <= 1 ; n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->OAMUnit[index].RDIStatusChangeExceptionGroupStatus[n] = isSip5_15 ? 0x600+4*n :
                        0x280+4*n;
                }/* end of loop n */
            }/*280+4*n*/
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->OAMUnit[index].RDIStatusChangeCntr = isSip5_15 ? 0x00000a14 : 0x00000314;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->OAMUnit[index].OAMUnitInterruptMask = 0x000000f4;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->OAMUnit[index].OAMUnitInterruptCause = 0x000000f0;
            {/*d4+n*4*/
                GT_U32    n;
                GT_U32    nMax = isSip5_15 ? 3 : 2;
                for(n = 0 ; n <= nMax ; n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->OAMUnit[index].OAMTableAccessData[n] =
                        0xd4+n*4;
                }/* end of loop n */
            }/*d4+n*4*/
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->OAMUnit[index].OAMTableAccessCtrl = 0x000000d0;
            {/*30+4*n*/
                GT_U32    n;
                for(n = 0 ; n <= 3 ; n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->OAMUnit[index].OAMLossMeasurementOpcodes[n] =
                        0x30+4*n;
                }/* end of loop n */
            }/*30+4*n*/
            {/*28+4*n*/
                GT_U32    n;
                for(n = 0 ; n <= 1 ; n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->OAMUnit[index].OAMLMCountedOpcodes[n] =
                        0x28+4*n;
                }/* end of loop n */
            }/*28+4*n*/
            {/*70+4*n*/
                GT_U32    n;
                for(n = 0 ; n <= 3 ; n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->OAMUnit[index].OAMKeepAliveOpcodes[n] =
                        0x70+4*n;
                }/* end of loop n */
            }/*70+4*n*/
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->OAMUnit[index].OAMInvalidKeepaliveExceptionConfigs = 0x00000020;
            {/*94+8*n*/
                GT_U32    n;
                for(n = 0 ; n <= 7 ; n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->OAMUnit[index].agingPeriodEntryHigh[n] =
                        0x94+8*n;
                }/* end of loop n */
            }/*94+8*n*/
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->OAMUnit[index].dualEndedLM = 0x00000084;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->OAMUnit[index].OAMExceptionConfigs2 = 0x0000001c;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->OAMUnit[index].OAMExceptionConfigs1 = 0x00000018;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->OAMUnit[index].OAMExceptionConfigs = 0x00000014;
            {/*50+4*n*/
                GT_U32    n;
                for(n = 0 ; n <= 3 ; n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->OAMUnit[index].OAMDelayMeasurementOpcodes[n] =
                        0x50+4*n;
                }/* end of loop n */
            }/*50+4*n*/
            {/*240+4*n*/
                GT_U32    n;
                for(n = 0 ; n <= 1 ; n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->OAMUnit[index].MEGLevelGroupStatus[n] = isSip5_15 ? 0x400+4*n :
                        0x240+4*n;
                }/* end of loop n */
            }/*240+4*n*/
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->OAMUnit[index].MEGLevelExceptionCntr = isSip5_15 ? 0x00000a00 : 0x00000300;
            {/*260+4*n*/
                GT_U32    n;
                for(n = 0 ; n <= 1 ; n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->OAMUnit[index].sourceInterfaceGroupStatus[n] = isSip5_15 ? 0x500+4*n :
                        0x260+4*n;
                }/* end of loop n */
            }/*260+4*n*/
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->OAMUnit[index].invalidKeepaliveHashCntr = isSip5_15 ? 0x00000a08 : 0x00000308;
            {/*220+4*n*/
                GT_U32    n;
                for(n = 0 ; n <= 1 ; n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->OAMUnit[index].invalidKeepaliveGroupStatus[n] = isSip5_15 ? 0x300+4*n :
                        0x220+4*n;
                }/* end of loop n */
            }/*220+4*n*/
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->OAMUnit[index].hashBitsSelection = 0x00000024;
            {/*200+4*n*/
                GT_U32    n;
                for(n = 0 ; n <= 1 ; n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->OAMUnit[index].excessKeepaliveGroupStatus[n] =
                        0x200+4*n;
                }/* end of loop n */
            }/*200+4*n*/
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->OAMUnit[index].excessKeepaliveCntr = isSip5_15 ? 0x00000a0c : 0x0000030c;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->OAMUnit[index].keepaliveAgingCntr = isSip5_15 ? 0x00000a10 : 0x00000310;
            {/*90+8*n*/
                GT_U32    n;
                for(n = 0 ; n <= 7 ; n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->OAMUnit[index].agingPeriodEntryLow[n] =
                        0x90+8*n;
                }/* end of loop n */
            }/*90+8*n*/
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->OAMUnit[index].txPeriodExceptionCntr = isSip5_15 ? 0x00000a18 : 0x00000318;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->OAMUnit[index].summaryExceptionCntr = isSip5_15 ? 0x00000a1c : 0x0000031c;

        }/*end of unit OAMUnit */
    }


    if(isDxControlPipe)
    {/*start of unit IPvX */
        {/*start of unit routerManagementCntrsSets */
            {/*90c+0x100*n*/
                GT_U32    n;
                for(n = 0 ; n <= 3 ; n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->IPvX.routerManagementCntrsSets.routerManagementInUcNonRoutedExceptionPktsCntrSet[n] =
                        0x90c+0x100*n;
                }/* end of loop n */
            }/*90c+0x100*n*/
            {/*91c+0x100*n*/
                GT_U32    n;
                for(n = 0 ; n <= 3 ; n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->IPvX.routerManagementCntrsSets.routerManagementInMcTrappedMirroredPktCntrSet[n] =
                        0x91c+0x100*n;
                }/* end of loop n */
            }/*91c+0x100*n*/
            {/*900+0x100*n*/
                GT_U32    n;
                for(n = 0 ; n <= 3 ; n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->IPvX.routerManagementCntrsSets.routerManagementInUcPktsCntrSet[n] =
                        0x900+0x100*n;
                }/* end of loop n */
            }/*900+0x100*n*/
            {/*904+0x100*n*/
                GT_U32    n;
                for(n = 0 ; n <= 3 ; n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->IPvX.routerManagementCntrsSets.routerManagementInMcPktsCntrSet[n] =
                        0x904+0x100*n;
                }/* end of loop n */
            }/*904+0x100*n*/
            {/*908+0x100*n*/
                GT_U32    n;
                for(n = 0 ; n <= 3 ; n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->IPvX.routerManagementCntrsSets.routerManagementInUcNonRoutedNonException[n] =
                        0x908+0x100*n;
                }/* end of loop n */
            }/*908+0x100*n*/
            {/*910+0x100*n*/
                GT_U32    n;
                for(n = 0 ; n <= 3 ; n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->IPvX.routerManagementCntrsSets.routerManagementInMcNonRoutedNonExceptionPktsCntrSet[n] =
                        0x910+0x100*n;
                }/* end of loop n */
            }/*910+0x100*n*/
            {/*914+0x100*n*/
                GT_U32    n;
                for(n = 0 ; n <= 3 ; n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->IPvX.routerManagementCntrsSets.routerManagementInMcNonRoutedExceptionPktsCntrSet[n] =
                        0x914+0x100*n;
                }/* end of loop n */
            }/*914+0x100*n*/
            {/*918+0x100*n*/
                GT_U32    n;
                for(n = 0 ; n <= 3 ; n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->IPvX.routerManagementCntrsSets.routerManagementInUcTrappedMirroredPktCntrSet[n] =
                        0x918+0x100*n;
                }/* end of loop n */
            }/*918+0x100*n*/
            {/*920+0x100*n*/
                GT_U32    n;
                for(n = 0 ; n <= 3 ; n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->IPvX.routerManagementCntrsSets.routerManagementInMcRPFFailCntrSet[n] =
                        0x920+0x100*n;
                }/* end of loop n */
            }/*920+0x100*n*/
            {/*924+0x100*n*/
                GT_U32    n;
                for(n = 0 ; n <= 3 ; n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->IPvX.routerManagementCntrsSets.routerManagementOutUcPktCntrSet[n] =
                        0x924+0x100*n;
                }/* end of loop n */
            }/*924+0x100*n*/
            {/*980+0x100*n*/
                GT_U32    n;
                for(n = 0 ; n <= 3 ; n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->IPvX.routerManagementCntrsSets.routerManagementCntrsSetConfig0[n] =
                        0x980+0x100*n;
                }/* end of loop n */
            }/*980+0x100*n*/
            {/*984+0x100*n*/
                GT_U32    n;
                for(n = 0 ; n <= 3 ; n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->IPvX.routerManagementCntrsSets.routerManagementCntrsSetConfig1[n] =
                        0x984+0x100*n;
                }/* end of loop n */
            }/*984+0x100*n*/
            if(SMEM_CHT_IS_SIP6_GET(devObjPtr))
            {/*988+0x100*n*/
                GT_U32    n;
                for(n = 0 ; n <= 3 ; n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->IPvX.routerManagementCntrsSets.routerManagementCntrsSetConfig2[n] =
                        0x988+0x100*n;
                }/* end of loop n */
            }/*988+0x100*n*/
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->IPvX.routerManagementCntrsSets.routerManagementIncomingPktCntr = 0x00000944;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->IPvX.routerManagementCntrsSets.routerManagementOutgoingPktCntr = 0x00000948;
            if(!SMEM_CHT_IS_SIP5_20_GET(devObjPtr))
            {
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->IPvX.routerManagementCntrsSets.routerManagementInSIPFilterCntr = 0x00000958;
            }

        }/*end of unit routerManagementCntrsSets */


        {/*start of unit routerGlobalCtrl */
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->IPvX.routerGlobalCtrl.routerGlobalCtrl0 = 0x00000000;
            {/*4+n*0x4*/
                GT_U32    n;
                for(n = 0 ; n <= 3 ; n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->IPvX.routerGlobalCtrl.routerMTUConfigReg[n] =
                        0x4+n*0x4;
                }/* end of loop n */
            }/*4+n*0x4*/
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->IPvX.routerGlobalCtrl.routerGlobalCtrl1 = 0x00000020;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->IPvX.routerGlobalCtrl.routerGlobalCtrl2 = 0x00000024;
            if (SMEM_CHT_IS_SIP5_25_GET(devObjPtr))
            {
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->IPvX.routerGlobalCtrl.routerGlobalCtrl3 = 0x00000028;
            }
        }/*end of unit routerGlobalCtrl */


        {/*start of unit routerDropCntr */
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->IPvX.routerDropCntr.routerDropCntr = 0x00000950;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->IPvX.routerDropCntr.routerDropCntrConfig = 0x00000954;

        }/*end of unit routerDropCntr */


        {/*start of unit routerBridgedPktsExceptionCntr */
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->IPvX.routerBridgedPktsExceptionCntr.routerBridgedPktExceptionsCntr = 0x00000940;

        }/*end of unit routerBridgedPktsExceptionCntr */


        {/*start of unit IPv6Scope */

            {/*278+n*4*/
                GT_U32    n;
                for(n = 0 ; n <= 3 ; n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->IPvX.IPv6Scope.IPv6UcScopeTableReg[n] =
                        0x278+n*4;
                }/* end of loop n */
            }/*278+n*4*/
            {/*288+n*4*/
                GT_U32    n;
                for(n = 0 ; n <= 3 ; n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->IPvX.IPv6Scope.IPv6McScopeTableReg[n] =
                        0x288+n*4;
                }/* end of loop n */
            }/*288+n*4*/
            {/*250+n*4*/
                GT_U32    n;
                for(n = 0 ; n <= 3 ; n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->IPvX.IPv6Scope.IPv6UcScopePrefix[n] =
                        0x250+n*4;
                }/* end of loop n */
            }/*250+n*4*/
            {/*260+n*4*/
                GT_U32    n;
                for(n = 0 ; n <= 3 ; n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->IPvX.IPv6Scope.IPv6UcScopeLevel[n] =
                        0x260+n*4;
                }/* end of loop n */
            }/*260+n*4*/

        }/*end of unit IPv6Scope */


        {/*start of unit IPv6GlobalCtrl */
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->IPvX.IPv6GlobalCtrl.IPv6CtrlReg0 = 0x00000200;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->IPvX.IPv6GlobalCtrl.IPv6CtrlReg1 = 0x00000204;

        }/*end of unit IPv6GlobalCtrl */


        {/*start of unit IPv4GlobalCtrl */
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->IPvX.IPv4GlobalCtrl.IPv4CtrlReg0 = 0x00000100;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->IPvX.IPv4GlobalCtrl.IPv4CtrlReg1 = 0x00000104;

        }/*end of unit IPv4GlobalCtrl */


        {/*start of unit internalRouter */
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->IPvX.internalRouter.routerInterruptCause = 0x00000970;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->IPvX.internalRouter.routerInterruptMask = 0x00000974;

        }/*end of unit internalRouter */


        {/*start of unit FCoEGlobalCtrl */
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->IPvX.FCoEGlobalCtrl.routerFCoEGlobalConfig = 0x00000380;

        }/*end of unit FCoEGlobalCtrl */


        {/*start of unit ECMPRoutingConfig */
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->IPvX.ECMPRoutingConfig.ECMPConfig = 0x00000360;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->IPvX.ECMPRoutingConfig.ECMPSeed = 0x00000364;

        }/*end of unit ECMPRoutingConfig */


        {/*start of unit CPUCodes */
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->IPvX.CPUCodes.CPUCodes0 = 0x00000e00;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->IPvX.CPUCodes.CPUCodes1 = 0x00000e04;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->IPvX.CPUCodes.CPUCodes2 = 0x00000e08;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->IPvX.CPUCodes.CPUCodes3 = 0x00000e0c;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->IPvX.CPUCodes.CPUCodes4 = 0x00000e10;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->IPvX.CPUCodes.CPUCodes5 = 0x00000e14;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->IPvX.CPUCodes.CPUCodes6 = 0x00000e18;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->IPvX.CPUCodes.CPUCodes7 = 0x00000e1c;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->IPvX.CPUCodes.CPUCodes8 = 0x00000e20;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->IPvX.CPUCodes.CPUCodes9 = 0x00000e24;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->IPvX.CPUCodes.CPUCodes10 = 0x00000e28;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->IPvX.CPUCodes.CPUCodes11 = 0x00000e2c;

        }/*end of unit CPUCodes */

        if (SMEM_CHT_IS_SIP6_GET(devObjPtr))
        {/* start of unit  FdbIpLookup*/
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->IPvX.FdbIpLookup.FdbIpv4RouteLookupMask = 0x00000f00;
            { /*0xf04+n*4*/
                GT_U32    n;
                for(n = 0 ; n <= 3 ; n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->IPvX.FdbIpLookup.FdbIpv6RouteLookupMask[n] =
                        0xf04+n*4;
                }/* end of loop n */
            }
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->IPvX.FdbRouteLookup.FdbHostLookup = 0x00000f20;
        }

    }/*end of unit IPvX */

    {/*start of unit GOP */

        {/*start of unit TAI[36][5] */
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TAI.TAI[0][0].TAIInterruptCause = 0x10180a00;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TAI.TAI[0][0].TAIInterruptMask = 0x10180a04;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TAI.TAI[0][0].TAICtrlReg0 = 0x10180a08;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TAI.TAI[0][0].TAICtrlReg1 = 0x10180a0c;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TAI.TAI[0][0].timeCntrFunctionConfig0 = 0x10180a10;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TAI.TAI[0][0].timeCntrFunctionConfig1 = 0x10180a14;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TAI.TAI[0][0].timeCntrFunctionConfig2 = 0x10180a18;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TAI.TAI[0][0].TODStepNanoConfigHigh = 0x10180a1c;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TAI.TAI[0][0].TODStepNanoConfigLow = 0x10180a20;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TAI.TAI[0][0].TODStepFracConfigHigh = 0x10180a24;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TAI.TAI[0][0].TODStepFracConfigLow = 0x10180a28;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TAI.TAI[0][0].timeAdjustmentPropagationDelayConfigHigh = 0x10180a2c;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TAI.TAI[0][0].timeAdjustmentPropagationDelayConfigLow = 0x10180a30;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TAI.TAI[0][0].triggerGenerationTODSecHigh = 0x10180a34;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TAI.TAI[0][0].triggerGenerationTODSecMed = 0x10180a38;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TAI.TAI[0][0].triggerGenerationTODSecLow = 0x10180a3c;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TAI.TAI[0][0].triggerGenerationTODNanoHigh = 0x10180a40;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TAI.TAI[0][0].triggerGenerationTODNanoLow = 0x10180a44;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TAI.TAI[0][0].triggerGenerationTODFracHigh = 0x10180a48;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TAI.TAI[0][0].triggerGenerationTODFracLow = 0x10180a4c;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TAI.TAI[0][0].timeLoadValueSecHigh = 0x10180a50;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TAI.TAI[0][0].timeLoadValueSecMed = 0x10180a54;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TAI.TAI[0][0].timeLoadValueSecLow = 0x10180a58;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TAI.TAI[0][0].timeLoadValueNanoHigh = 0x10180a5c;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TAI.TAI[0][0].timeLoadValueNanoLow = 0x10180a60;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TAI.TAI[0][0].timeLoadValueFracHigh = 0x10180a64;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TAI.TAI[0][0].timeLoadValueFracLow = 0x10180a68;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TAI.TAI[0][0].timeCaptureValue0SecHigh = 0x10180a6c;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TAI.TAI[0][0].timeCaptureValue0SecMed = 0x10180a70;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TAI.TAI[0][0].timeCaptureValue0SecLow = 0x10180a74;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TAI.TAI[0][0].timeCaptureValue0NanoHigh = 0x10180a78;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TAI.TAI[0][0].timeCaptureValue0NanoLow = 0x10180a7c;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TAI.TAI[0][0].timeCaptureValue0FracHigh = 0x10180a80;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TAI.TAI[0][0].timeCaptureValue0FracLow = 0x10180a84;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TAI.TAI[0][0].timeCaptureValue1SecHigh = 0x10180a88;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TAI.TAI[0][0].timeCaptureValue1SecMed = 0x10180a8c;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TAI.TAI[0][0].timeCaptureValue1SecLow = 0x10180a90;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TAI.TAI[0][0].timeCaptureValue1NanoHigh = 0x10180a94;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TAI.TAI[0][0].timeCaptureValue1NanoLow = 0x10180a98;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TAI.TAI[0][0].timeCaptureValue1FracHigh = 0x10180a9c;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TAI.TAI[0][0].timeCaptureValue1FracLow = 0x10180aa0;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TAI.TAI[0][0].timeCaptureStatus = 0x10180aa4;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TAI.TAI[0][0].timeUpdateCntr = 0x10180aa8;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TAI.TAI[0][0].PClkClockCycleConfigLow = 0x10180ad8;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TAI.TAI[0][0].PClkClockCycleConfigHigh = 0x10180ad4;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TAI.TAI[0][0].incomingClockInCountingConfigLow = 0x10180af8;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TAI.TAI[0][0].incomingClockInCountingEnable = 0x10180af4;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TAI.TAI[0][0].generateFunctionMaskSecMed = 0x10180ab0;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TAI.TAI[0][0].generateFunctionMaskSecLow = 0x10180ab4;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TAI.TAI[0][0].generateFunctionMaskSecHigh = 0x10180aac;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TAI.TAI[0][0].generateFunctionMaskNanoLow = 0x10180abc;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TAI.TAI[0][0].generateFunctionMaskNanoHigh = 0x10180ab8;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TAI.TAI[0][0].generateFunctionMaskFracLow = 0x10180ac4;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TAI.TAI[0][0].generateFunctionMaskFracHigh = 0x10180ac0;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TAI.TAI[0][0].externalClockPropagationDelayConfigLow = 0x10180af0;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TAI.TAI[0][0].externalClockPropagationDelayConfigHigh = 0x10180aec;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TAI.TAI[0][0].driftThresholdConfigLow = 0x10180ae0;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TAI.TAI[0][0].driftThresholdConfigHigh = 0x10180adc;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TAI.TAI[0][0].driftAdjustmentConfigLow = 0x10180acc;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TAI.TAI[0][0].driftAdjustmentConfigHigh = 0x10180ac8;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TAI.TAI[0][0].clockCycleConfigLow = 0x10180ae8;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TAI.TAI[0][0].clockCycleConfigHigh = 0x10180ae4;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TAI.TAI[0][0].captureTriggerCntr = 0x10180ad0;

            if(SMAIN_NOT_VALID_CNS != UNIT_BASE_ADDR_GET_ALLOW_NON_EXIST_UNIT_MAC(devObjPtr,UNIT_TAI))
            {
                GT_U32  removeBaseAddr = 0x10180a00;
                GT_U32  unitBaseAddr = UNIT_BASE_ADDR_FROM_STRING_GET_MAC(devObjPtr,STR(UNIT_TAI));

                if(unitBaseAddr != 0x10000000)
                {
                    GT_U32        unitArrayIndex;
                    GT_U32        *unitArray_0_ptr,*currU32Ptr;
                    unitArray_0_ptr = (void*)&(SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TAI.TAI[0][0]);
                    currU32Ptr = unitArray_0_ptr;
                    /* loop on all elements in the unit , and set/update the address of register/table */
                    for(unitArrayIndex = 0 ;
                        unitArrayIndex < NUM_ELEMENTS_IN_UNIT_MAC(SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TAI.TAI[0][0],GT_U32) ;
                        unitArrayIndex++ , currU32Ptr++){
                        if(unitArray_0_ptr[unitArrayIndex] == 0xFFFFFFFF)
                        {
                            continue;
                        }
                        (*currU32Ptr) = unitArray_0_ptr[unitArrayIndex] - removeBaseAddr;
                    } /*loop on unitArrayIndex */
                }/* end of loop o */
                if((devObjPtr->supportSingleTai) && isSip6)
                {
                    GT_U32        unitArrayIndex;
                    GT_U32        *unitArray_0_ptr,*currU32Ptr;
                    GT_U32  unitBaseAddr1 = UNIT_BASE_ADDR_FROM_STRING_GET_MAC(devObjPtr,STR(UNIT_TAI_1));
                    currU32Ptr = (void*)&SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TAI.TAI[0][1] ;
                    unitArray_0_ptr = (void*)&(SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TAI.TAI[0][0]);
                    /* loop on all elements in the unit , and set/update the address of register/table */
                    for(unitArrayIndex = 0 ;
                        unitArrayIndex < NUM_ELEMENTS_IN_UNIT_MAC(SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TAI.TAI[0][0],GT_U32) ;
                        unitArrayIndex++ , currU32Ptr++){
                        if(unitArray_0_ptr[unitArrayIndex] == 0xFFFFFFFF)
                        {
                            continue;
                        }
                        (*currU32Ptr) = unitArray_0_ptr[unitArrayIndex] + (unitBaseAddr1-unitBaseAddr);
                    } /*loop on unitArrayIndex */
                }/* MASTER TAI_1 */
                if((devObjPtr->supportSingleTai) && isSip6_30)
                {
                    /* define additional MASTER TAI 2,3,4 */
                    GT_U32        unitArrayIndex;
                    GT_U32        *unitArray_0_ptr,*currU32Ptr;
                    GT_U32  unitBaseAddr1 = UNIT_BASE_ADDR_FROM_STRING_GET_MAC(devObjPtr,STR(UNIT_TAI_2));
                    currU32Ptr = (void*)&SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TAI.TAI[0][2] ;
                    unitArray_0_ptr = (void*)&(SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TAI.TAI[0][0]);
                    /* loop on all elements in the unit , and set/update the address of register/table */
                    for(unitArrayIndex = 0 ;
                        unitArrayIndex < NUM_ELEMENTS_IN_UNIT_MAC(SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TAI.TAI[0][0],GT_U32) ;
                        unitArrayIndex++ , currU32Ptr++){
                        if(unitArray_0_ptr[unitArrayIndex] == 0xFFFFFFFF)
                        {
                            continue;
                        }
                        (*currU32Ptr) = unitArray_0_ptr[unitArrayIndex] + (unitBaseAddr1-unitBaseAddr);
                    } /*loop on unitArrayIndex */

                    unitBaseAddr1 = UNIT_BASE_ADDR_FROM_STRING_GET_MAC(devObjPtr,STR(UNIT_TAI_3));
                    currU32Ptr = (void*)&SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TAI.TAI[0][3] ;
                    /* loop on all elements in the unit , and set/update the address of register/table */
                    for(unitArrayIndex = 0 ;
                        unitArrayIndex < NUM_ELEMENTS_IN_UNIT_MAC(SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TAI.TAI[0][0],GT_U32) ;
                        unitArrayIndex++ , currU32Ptr++){
                        if(unitArray_0_ptr[unitArrayIndex] == 0xFFFFFFFF)
                        {
                            continue;
                        }
                        (*currU32Ptr) = unitArray_0_ptr[unitArrayIndex] + (unitBaseAddr1-unitBaseAddr);
                    } /*loop on unitArrayIndex */

                    unitBaseAddr1 = UNIT_BASE_ADDR_FROM_STRING_GET_MAC(devObjPtr,STR(UNIT_TAI_4));
                    currU32Ptr = (void*)&SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TAI.TAI[0][4] ;
                    /* loop on all elements in the unit , and set/update the address of register/table */
                    for(unitArrayIndex = 0 ;
                        unitArrayIndex < NUM_ELEMENTS_IN_UNIT_MAC(SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TAI.TAI[0][0],GT_U32) ;
                        unitArrayIndex++ , currU32Ptr++){
                        if(unitArray_0_ptr[unitArrayIndex] == 0xFFFFFFFF)
                        {
                            continue;
                        }
                        (*currU32Ptr) = unitArray_0_ptr[unitArrayIndex] + (unitBaseAddr1-unitBaseAddr);
                    } /*loop on unitArrayIndex */

                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TAI.TAI[0][4].phySignature        = 0x00000500;
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TAI.TAI[0][4].phyTimestampSecLow  = 0x00000504;
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TAI.TAI[0][4].phyTimestampSecHigh = 0x00000508;
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TAI.TAI[0][4].phyTimestampNanoSec = 0x0000050c;

                }/* MASTER TAI_2,3,4 */
            }

            if(!devObjPtr->supportSingleTai)
            {
                /* spesific code for Bobcat2B */
                /* 0x10180A00 + 0x1000*(z+o*16): where z (0-1) represents TAI index, where o (0-2) represents Port index
                   0x10180A00 + 0x1000*(z+o*4+48): where z (0-1) represents TAI index, where o (0-1) represents Port index
                   0x10180A00 + 0x200000 + 0x1000*(z+o*4): where z (0-1) represents TAI index, where o (0-3) represents Port index */

                /*10180a00+0x1000*(z+o*16) and 10180a00+0x1000*(z+o*4+48) and 0x10180a00 + 0x200000 + 0x1000*(z+o*4) */
                GT_U32    z,o;
                for(z = 0 ; z <= 1 ; z++) {
                    for(o = 0 ; o <= 2 ; o++) {
                        GT_U32        unitArrayIndex;
                        GT_U32        *unitArray_0_ptr,*currU32Ptr;
                        currU32Ptr = (void*)&SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TAI.TAI[o][z] ;
                        unitArray_0_ptr = (void*)&(SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TAI.TAI[0][0]);
                        /* loop on all elements in the unit , and set/update the address of register/table */
                        for(unitArrayIndex = 0 ;
                            unitArrayIndex < NUM_ELEMENTS_IN_UNIT_MAC(SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TAI.TAI[0][0],GT_U32) ;
                            unitArrayIndex++ , currU32Ptr++){
                            if(unitArray_0_ptr[unitArrayIndex] == 0xFFFFFFFF)
                            {
                                continue;
                            }
                            (*currU32Ptr) = unitArray_0_ptr[unitArrayIndex] + 0* 0x10180a00+0x1000*(z+o*16);
                        } /*loop on unitArrayIndex */
                    }/* end of loop o */
                }/* end of loop z */
                for(z = 0 ; z <= 1 ; z++) {
                    for(o = 0 ; o <= 1 ; o++) {
                        GT_U32        unitArrayIndex;
                        GT_U32        *unitArray_0_ptr,*currU32Ptr;
                        currU32Ptr = (void*)&SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TAI.TAI[o + 3][z] ;
                        unitArray_0_ptr = (void*)&(SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TAI.TAI[0][0]);
                        /* loop on all elements in the unit , and set/update the address of register/table */
                        for(unitArrayIndex = 0 ;
                            unitArrayIndex < NUM_ELEMENTS_IN_UNIT_MAC(SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TAI.TAI[0][0],GT_U32) ;
                            unitArrayIndex++ , currU32Ptr++){
                            if(unitArray_0_ptr[unitArrayIndex] == 0xFFFFFFFF)
                            {
                                continue;
                            }
                            (*currU32Ptr) = unitArray_0_ptr[unitArrayIndex] + 0* 0x10180a00+0x1000*(z+o*4+48);
                        } /*loop on unitArrayIndex */
                    }/* end of loop o */
                }/* end of loop z */
                for(z = 0 ; z <= 1 ; z++) {
                    for(o = 0 ; o <= 3 ; o++) {
                        GT_U32        unitArrayIndex;
                        GT_U32        *unitArray_0_ptr,*currU32Ptr;
                        currU32Ptr = (void*)&SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TAI.TAI[o + 5][z] ;
                        unitArray_0_ptr = (void*)&(SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TAI.TAI[0][0]);
                        /* loop on all elements in the unit , and set/update the address of register/table */
                        for(unitArrayIndex = 0 ;
                            unitArrayIndex < NUM_ELEMENTS_IN_UNIT_MAC(SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TAI.TAI[0][0],GT_U32) ;
                            unitArrayIndex++ , currU32Ptr++){
                            if(unitArray_0_ptr[unitArrayIndex] == 0xFFFFFFFF)
                            {
                                continue;
                            }
                            (*currU32Ptr) = unitArray_0_ptr[unitArrayIndex] + 0* 0x10180a00+0x200000+0x1000*(z+o*4);
                        } /*loop on unitArrayIndex */
                    }/* end of loop o */
                }/* end of loop z */
            }/*10180a00+0x1000*(z+o*16) and 10180a00+0x1000*(z+o*4+48) and 0x10180a00 + 0x200000 + 0x1000*(z+o*4) */
        }/*end of unit TAI[4][3] */

        if(devObjPtr->devMemGopRegDbInitFuncPtr)
        {
            /* the CB function already called and set the GOP PTP addresses */
        }
        else
        if(SMAIN_NOT_VALID_CNS != UNIT_BASE_ADDR_GET_ALLOW_NON_EXIST_UNIT_MAC(devObjPtr,UNIT_GOP))
        {
            {/*start of unit PTP[72] */
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->GOP_PTP.PTP[0].PTPInterruptCause = 0x10180800;
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->GOP_PTP.PTP[0].PTPInterruptMask = 0x10180804;
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->GOP_PTP.PTP[0].PTPGeneralCtrl = 0x10180808;
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->GOP_PTP.PTP[0].PTPTXTimestampQueue0Reg0 = 0x1018080c;
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->GOP_PTP.PTP[0].PTPTXTimestampQueue0Reg1 = 0x10180810;
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->GOP_PTP.PTP[0].PTPTXTimestampQueue0Reg2 = 0x10180814;
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->GOP_PTP.PTP[0].PTPTXTimestampQueue1Reg0 = 0x10180818;
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->GOP_PTP.PTP[0].PTPTXTimestampQueue1Reg1 = 0x1018081c;
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->GOP_PTP.PTP[0].PTPTXTimestampQueue1Reg2 = 0x10180820;
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->GOP_PTP.PTP[0].totalPTPPktsCntr = 0x10180824;
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->GOP_PTP.PTP[0].PTPv1PktCntr = 0x10180828;
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->GOP_PTP.PTP[0].PTPv2PktCntr = 0x1018082c;
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->GOP_PTP.PTP[0].Y1731PktCntr = 0x10180830;
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->GOP_PTP.PTP[0].NTPTsPktCntr = 0x10180834;
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->GOP_PTP.PTP[0].NTPReceivePktCntr = 0x10180838;
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->GOP_PTP.PTP[0].NTPTransmitPktCntr = 0x1018083c;
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->GOP_PTP.PTP[0].WAMPPktCntr = 0x10180840;
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->GOP_PTP.PTP[0].addCorrectedTimeActionPktCntr = 0x10180858;
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->GOP_PTP.PTP[0].NTPPTPOffsetHigh = 0x10180870;
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->GOP_PTP.PTP[0].noneActionPktCntr = 0x10180844;
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->GOP_PTP.PTP[0].forwardActionPktCntr = 0x10180848;
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->GOP_PTP.PTP[0].dropActionPktCntr = 0x1018084c;
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->GOP_PTP.PTP[0].captureIngrTimeActionPktCntr = 0x1018086c;
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->GOP_PTP.PTP[0].captureAddTimeActionPktCntr = 0x1018085c;
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->GOP_PTP.PTP[0].captureAddIngrTimeActionPktCntr = 0x10180868;
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->GOP_PTP.PTP[0].captureAddCorrectedTimeActionPktCntr = 0x10180860;
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->GOP_PTP.PTP[0].captureActionPktCntr = 0x10180850;
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->GOP_PTP.PTP[0].addTimeActionPktCntr = 0x10180854;
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->GOP_PTP.PTP[0].addIngrTimeActionPktCntr = 0x10180864;
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->GOP_PTP.PTP[0].NTPPTPOffsetLow = 0x10180874;

                if(SMEM_IS_PIPE_FAMILY_GET(devObjPtr))
                {
                    GT_U32    t;
                    GT_U32  numPortsInPipe = 16;
                    for(t = 0 ; t < numPortsInPipe; t++)
                    {
                        smemLion2RegsInfoSet_GOP_PTP(devObjPtr, SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr), t,0,0);
                    }/* end of loop t */
                }
                else
                if(SMEM_CHT_IS_SIP5_20_GET(devObjPtr))
                {
                    GT_U32    t;
                    GT_U32  numPortsInPipe = smemCheetahNumGopPortsInPipe(devObjPtr);
                    for(t = 0 ; t < numPortsInPipe; t++)
                    {
                        smemLion2RegsInfoSet_GOP_PTP(devObjPtr, SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr), t,0,0);
                    }/* end of loop t */
                }
                else if(SMEM_CHT_IS_SIP5_16_GET(devObjPtr))
                {
                    GT_U32    t;
                    for(t = 0 ; t <= 32 ; t++) {
                        GT_U32        unitArrayIndex;
                        GT_U32        *unitArray_0_ptr,*currU32Ptr;
                        currU32Ptr = (void*)&SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->GOP_PTP.PTP[t] ;
                        unitArray_0_ptr = (void*)&(SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->GOP_PTP.PTP[0]);
                        /* loop on all elements in the unit , and set/update the address of register/table */
                        for(unitArrayIndex = 0 ;
                            unitArrayIndex < NUM_ELEMENTS_IN_UNIT_MAC(SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->GOP_PTP.PTP[0],GT_U32) ;
                            unitArrayIndex++ , currU32Ptr++){
                            if(unitArray_0_ptr[unitArrayIndex] != SMAIN_NOT_VALID_CNS)
                            {
                                (*currU32Ptr) = unitArray_0_ptr[unitArrayIndex] + 0* 0x10180800+0x1000*t;
                            }
                        } /*loop on unitArrayIndex */
                    }/* end of loop t */
                }
                else
                {/*10180800+0x1000*t and 10180800+0x200000 + 0x1000*t*/
                    GT_U32    t;
                    for(t = 0 ; t <= 55 ; t++) {
                        GT_U32        unitArrayIndex;
                        GT_U32        *unitArray_0_ptr,*currU32Ptr;
                        currU32Ptr = (void*)&SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->GOP_PTP.PTP[t] ;
                        unitArray_0_ptr = (void*)&(SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->GOP_PTP.PTP[0]);
                        /* loop on all elements in the unit , and set/update the address of register/table */
                        for(unitArrayIndex = 0 ;
                            unitArrayIndex < NUM_ELEMENTS_IN_UNIT_MAC(SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->GOP_PTP.PTP[0],GT_U32) ;
                            unitArrayIndex++ , currU32Ptr++){
                            if(unitArray_0_ptr[unitArrayIndex] != SMAIN_NOT_VALID_CNS)
                            {
                                (*currU32Ptr) = unitArray_0_ptr[unitArrayIndex] + 0* 0x10180800+0x1000*t;
                            }
                        } /*loop on unitArrayIndex */
                    }/* end of loop t */
                    for(t = 0 ; t <= 15 ; t++) {
                        GT_U32        unitArrayIndex;
                        GT_U32        *unitArray_0_ptr,*currU32Ptr;
                        currU32Ptr = (void*)&SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->GOP_PTP.PTP[(t - 0) + 56] ;
                        unitArray_0_ptr = (void*)&(SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->GOP_PTP.PTP[0]);
                        /* loop on all elements in the unit , and set/update the address of register/table */
                        for(unitArrayIndex = 0 ;
                            unitArrayIndex < NUM_ELEMENTS_IN_UNIT_MAC(SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->GOP_PTP.PTP[0],GT_U32) ;
                            unitArrayIndex++ , currU32Ptr++){
                            if(unitArray_0_ptr[unitArrayIndex] != SMAIN_NOT_VALID_CNS)
                            {
                                (*currU32Ptr) = unitArray_0_ptr[unitArrayIndex] + 0* 0x10180800+0x200000 + 0x1000*t;
                            }
                        } /*loop on unitArrayIndex */
                    }/* end of loop t */
                }/*10180800+0x1000*t and 10180800+0x200000 + 0x1000*t*/
            }/*end of unit PTP[72] */

            {/* manually added (since parsed from bc2 and not from sip5) unify addresses to be 0 based (remove the base address from them) */
                GT_U32    regIndex;
                GT_U32        *currU32Ptr;
                SMEM_CHT_PP_REGS_UNIT_START_INFO_STC *unitStartInfoPtr = &SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->GOP_PTP.startUnitInfo;
                GT_U32 numOfElem = NUM_ELEMENTS_IN_UNIT_MAC(SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->GOP_PTP,GT_U32) -
                                   (sizeof(SMEM_CHT_PP_REGS_UNIT_START_INFO_STC) / sizeof(GT_U32));
                GT_U32   baseAddress = 0x10000000;
                currU32Ptr = (void*)(unitStartInfoPtr+1); /*jump after the 'start unit info'*/
                for(regIndex = 0 ; regIndex < numOfElem ; regIndex++,currU32Ptr++)
                {
                    if((*currU32Ptr) < baseAddress)
                    {
                        /* the addresses not hold '0x10000000' as base */
                        break;
                    }

                    if (*currU32Ptr != SMAIN_NOT_VALID_CNS)
                    {
                        (*currU32Ptr) -= baseAddress;
                    }
                }/* end of loop regIndex */
            }/* manually added (since parsed from bc2 and not from sip5) unify addresses to be 0 based (remove the base address from them) */
        }

        if(SMAIN_NOT_VALID_CNS != UNIT_BASE_ADDR_GET_ALLOW_NON_EXIST_UNIT_MAC(devObjPtr,UNIT_LMS))
        {/*start of unit LMS_0 */
            GT_U32 instance;

            for(instance = 0; instance < 3; instance++)
            {
                {/*start of unit LMS0 */
                    {/*start of unit LMS0Group0 */
                        {/*start of unit SMIConfig */
                            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->LMS[instance].LMS0.LMS0Group0.SMIConfig.PHYAddrReg0forPorts0Through5 = 0x00004030;
                            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->LMS[instance].LMS0.LMS0Group0.SMIConfig.PHYAutoNegConfigReg0 = 0x00004034;
                            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->LMS[instance].LMS0.LMS0Group0.SMIConfig.SMI0Management = 0x00004054;
                            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->LMS[instance].LMS0.LMS0Group0.SMIConfig.LMS0MiscConfigs = 0x00004200;

                        }/*end of unit SMIConfig */


                        {/*start of unit portsMIBCntrsInterrupts */
                            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->LMS[instance].LMS0.LMS0Group0.portsMIBCntrsInterrupts.portsGOP0MIBsInterruptCause = 0x00004000;
                            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->LMS[instance].LMS0.LMS0Group0.portsMIBCntrsInterrupts.portsGOP0MIBsInterruptMask = 0x00004004;

                        }/*end of unit portsMIBCntrsInterrupts */


                        {/*start of unit portsMACSourceAddr */
                            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->LMS[instance].LMS0.LMS0Group0.portsMACSourceAddr.sourceAddrMiddle = 0x00004024;
                            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->LMS[instance].LMS0.LMS0Group0.portsMACSourceAddr.sourceAddrHigh = 0x00004028;

                        }/*end of unit portsMACSourceAddr */


                        {/*start of unit portsMACMIBCntrs */
                            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->LMS[instance].LMS0.LMS0Group0.portsMACMIBCntrs.MIBCntrsCtrlReg0forPorts0Through5 = 0x00004020;

                        }/*end of unit portsMACMIBCntrs */


                        {/*start of unit portsAndMIBCntrsInterruptSummary */
                            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->LMS[instance].LMS0.LMS0Group0.portsAndMIBCntrsInterruptSummary.MIBCntrsInterruptSummary = 0x00004010;

                        }/*end of unit portsAndMIBCntrsInterruptSummary */


                        {/*start of unit periodicFCRegs */
                            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->LMS[instance].LMS0.LMS0Group0.periodicFCRegs.gigPortsPeriodicFCCntrMaxValueReg = 0x00004140;

                        }/*end of unit periodicFCRegs */


                        {/*start of unit LEDInterfaceConfig */
                            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->LMS[instance].LMS0.LMS0Group0.LEDInterfaceConfig.LEDInterface0CtrlReg0forPorts0Through11AndStackPort = 0x00004100;
                            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->LMS[instance].LMS0.LMS0Group0.LEDInterfaceConfig.LEDInterface0Group01ConfigRegforPorts0Through11 = 0x00004104;
                            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->LMS[instance].LMS0.LMS0Group0.LEDInterfaceConfig.LEDInterface0Class01ManipulationRegforPorts0Through11 = 0x00004108;
                            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->LMS[instance].LMS0.LMS0Group0.LEDInterfaceConfig.LEDInterface0Class4ManipulationRegforPorts0Through11 = 0x0000410c;
                            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->LMS[instance].LMS0.LMS0Group0.LEDInterfaceConfig.stackLEDInterface0Class04ManipulationRegforStackPorts = 0x00005100;
                            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->LMS[instance].LMS0.LMS0Group0.LEDInterfaceConfig.stackLEDInterface0Class59ManipulationRegforStackPorts = 0x00005104;
                            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->LMS[instance].LMS0.LMS0Group0.LEDInterfaceConfig.LEDInterface0FlexlinkPortsDebugSelectRegforStackPort = 0x00005110;
                            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->LMS[instance].LMS0.LMS0Group0.LEDInterfaceConfig.LEDInterface0FlexlinkPortsDebugSelectReg1forStackPort = 0x00005114;

                        }/*end of unit LEDInterfaceConfig */


                    }/*end of unit LMS0Group0 */


                    {/*start of unit LMS0Group1 */
                        {/*start of unit SMIConfig */
                            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->LMS[instance].LMS0.LMS0Group1.SMIConfig.flowCtrlAdvertiseForFiberMediaSelectedConfigReg0forPorts0Through11 = 0x01004024;
                            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->LMS[instance].LMS0.LMS0Group1.SMIConfig.PHYAddrReg1forPorts6Through11 = 0x01004030;
                            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->LMS[instance].LMS0.LMS0Group1.SMIConfig.PHYAutoNegConfigReg1 = 0x01004034;

                        }/*end of unit SMIConfig */


                        {/*start of unit portsMIBCntrsInterrupts */
                            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->LMS[instance].LMS0.LMS0Group1.portsMIBCntrsInterrupts.triSpeedPortsGOP1MIBsInterruptCause = 0x01004000;
                            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->LMS[instance].LMS0.LMS0Group1.portsMIBCntrsInterrupts.triSpeedPortsGOP1MIBsInterruptMask = 0x01004004;

                        }/*end of unit portsMIBCntrsInterrupts */


                        {/*start of unit portsMACMIBCntrs */
                            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->LMS[instance].LMS0.LMS0Group1.portsMACMIBCntrs.MIBCntrsCtrlReg0forPorts6Through11 = 0x01004020;

                        }/*end of unit portsMACMIBCntrs */


                        {/*start of unit portsAndMIBCntrsInterruptSummary */
                            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->LMS[instance].LMS0.LMS0Group1.portsAndMIBCntrsInterruptSummary.portsMIBCntrsInterruptSummaryMask = 0x01005110;
                            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->LMS[instance].LMS0.LMS0Group1.portsAndMIBCntrsInterruptSummary.portsInterruptSummaryMask = 0x01005114;

                        }/*end of unit portsAndMIBCntrsInterruptSummary */


                        {/*start of unit periodicFCRegs */
                            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->LMS[instance].LMS0.LMS0Group1.periodicFCRegs.twoAndHalfGigPortsPeriodicFCCntrMaxValueReg = 0x01004140;

                        }/*end of unit periodicFCRegs */


                        {/*start of unit LEDInterfaceConfig */
                            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->LMS[instance].LMS0.LMS0Group1.LEDInterfaceConfig.LEDInterface0CtrlReg1AndClass6ManipulationRegforPorts0Through11 = 0x01004100;
                            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->LMS[instance].LMS0.LMS0Group1.LEDInterfaceConfig.LEDInterface0Group23ConfigRegforPorts0Through11 = 0x01004104;
                            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->LMS[instance].LMS0.LMS0Group1.LEDInterfaceConfig.LEDInterface0Class23ManipulationRegforPorts0Through11 = 0x01004108;
                            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->LMS[instance].LMS0.LMS0Group1.LEDInterfaceConfig.LEDInterface0Class5ManipulationRegforPorts0Through11 = 0x0100410c;
                            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->LMS[instance].LMS0.LMS0Group1.LEDInterfaceConfig.stackLEDInterface0Class1011ManipulationRegforStackPorts = 0x01005100;
                            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->LMS[instance].LMS0.LMS0Group1.LEDInterfaceConfig.stackLEDInterface0Group01ConfigRegforStackPort = 0x01005104;

                        }/*end of unit LEDInterfaceConfig */


                        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->LMS[instance].LMS0.LMS0Group1.stackPortsModeReg = 0x01004144;

                    }/*end of unit LMS0Group1 */


                }/*end of unit LMS0 */


                {/*start of unit LMS1 */
                    {/*start of unit LMS1Group0 */
                        {/*start of unit SMIConfig */
                            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->LMS[instance].LMS1.LMS1Group0.SMIConfig.PHYAddrReg2forPorts12Through17 = 0x02004030;
                            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->LMS[instance].LMS1.LMS1Group0.SMIConfig.PHYAutoNegConfigReg2 = 0x02004034;
                            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->LMS[instance].LMS1.LMS1Group0.SMIConfig.SMI1Management = 0x02004054;
                            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->LMS[instance].LMS1.LMS1Group0.SMIConfig.LMS1MiscConfigs = 0x02004200;

                        }/*end of unit SMIConfig */


                        {/*start of unit portsMIBCntrsInterrupts */
                            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->LMS[instance].LMS1.LMS1Group0.portsMIBCntrsInterrupts.portsGOP2MIBsInterruptCause = 0x02004000;
                            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->LMS[instance].LMS1.LMS1Group0.portsMIBCntrsInterrupts.portsGOP2MIBsInterruptMask = 0x02004004;

                        }/*end of unit portsMIBCntrsInterrupts */


                        {/*start of unit portsMACMIBCntrs */
                            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->LMS[instance].LMS1.LMS1Group0.portsMACMIBCntrs.MIBCntrsCtrlReg0forPorts12Through17 = 0x02004020;

                        }/*end of unit portsMACMIBCntrs */


                        {/*start of unit portsInterruptSummary */
                            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->LMS[instance].LMS1.LMS1Group0.portsInterruptSummary.portsInterruptSummary = 0x02004010;

                        }/*end of unit portsInterruptSummary */


                        {/*start of unit LEDInterfaceConfig */
                            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->LMS[instance].LMS1.LMS1Group0.LEDInterfaceConfig.LEDInterface1CtrlReg0forPorts12Through23AndStackPort = 0x02004100;
                            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->LMS[instance].LMS1.LMS1Group0.LEDInterfaceConfig.LEDInterface1Group01ConfigRegforPorts12Through23 = 0x02004104;
                            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->LMS[instance].LMS1.LMS1Group0.LEDInterfaceConfig.LEDInterface1Class01ManipulationRegforPorts12Through23 = 0x02004108;
                            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->LMS[instance].LMS1.LMS1Group0.LEDInterfaceConfig.LEDInterface1Class4ManipulationRegforPorts12Through23 = 0x0200410c;
                            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->LMS[instance].LMS1.LMS1Group0.LEDInterfaceConfig.stackLEDInterface1Class04ManipulationRegforStackPorts = 0x02005100;
                            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->LMS[instance].LMS1.LMS1Group0.LEDInterfaceConfig.stackLEDInterface1Class59ManipulationRegforStackPorts = 0x02005104;
                            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->LMS[instance].LMS1.LMS1Group0.LEDInterfaceConfig.LEDInterface1FlexlinkPortsDebugSelectRegforStackPort = 0x02005110;
                            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->LMS[instance].LMS1.LMS1Group0.LEDInterfaceConfig.LEDInterface1FlexlinkPortsDebugSelectReg1forStackPort = 0x02005114;

                        }/*end of unit LEDInterfaceConfig */


                    }/*end of unit LMS1Group0 */


                    {/*start of unit LMS1Group1 */
                        {/*start of unit SMIConfig */
                            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->LMS[instance].LMS1.LMS1Group1.SMIConfig.flowCtrlAdvertiseForFiberMediaSelectedConfigReg1forPorts12Through23 = 0x03004024;
                            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->LMS[instance].LMS1.LMS1Group1.SMIConfig.PHYAddrReg3forPorts18Through23 = 0x03004030;
                            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->LMS[instance].LMS1.LMS1Group1.SMIConfig.PHYAutoNegConfigReg3 = 0x03004034;

                        }/*end of unit SMIConfig */


                        {/*start of unit portsMIBCntrsInterrupts */
                            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->LMS[instance].LMS1.LMS1Group1.portsMIBCntrsInterrupts.portsGOP3AndStackMIBsInterruptCause = 0x03004000;
                            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->LMS[instance].LMS1.LMS1Group1.portsMIBCntrsInterrupts.portsGOP3AndStackMIBsInterruptMask = 0x03004004;

                        }/*end of unit portsMIBCntrsInterrupts */


                        {/*start of unit portsMACMIBCntrs */
                            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->LMS[instance].LMS1.LMS1Group1.portsMACMIBCntrs.MIBCntrsCtrlReg0forPorts18Through23 = 0x03004020;

                        }/*end of unit portsMACMIBCntrs */


                        {/*start of unit periodicFCRegs */
                            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->LMS[instance].LMS1.LMS1Group1.periodicFCRegs.stackPorts1GPeriodicFCCntrMaxValueReg = 0x03004140;

                        }/*end of unit periodicFCRegs */


                        {/*start of unit LEDInterfaceConfig */
                            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->LMS[instance].LMS1.LMS1Group1.LEDInterfaceConfig.LEDInterface1CtrlReg1AndClass6ManipulationforPorts12Through23 = 0x03004100;
                            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->LMS[instance].LMS1.LMS1Group1.LEDInterfaceConfig.LEDInterface1Group23ConfigRegforPorts12Through23 = 0x03004104;
                            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->LMS[instance].LMS1.LMS1Group1.LEDInterfaceConfig.LEDInterface1Class23ManipulationRegforPorts12Through23 = 0x03004108;
                            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->LMS[instance].LMS1.LMS1Group1.LEDInterfaceConfig.LEDInterface1Class5ManipulationRegforPorts12Through23 = 0x0300410c;
                            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->LMS[instance].LMS1.LMS1Group1.LEDInterfaceConfig.stackLEDInterface1Class1011ManipulationRegforStackPort = 0x03005100;
                            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->LMS[instance].LMS1.LMS1Group1.LEDInterfaceConfig.stackLEDInterface1Group01ConfigRegforStackPort = 0x03005104;

                        }/*end of unit LEDInterfaceConfig */


                    }/*end of unit LMS1Group1 */

                }/*end of unit LMS1 */
            }

        }/*end of unit LMS_0 */

    }/*end of unit GOP */

    /* Support the LMU of Falcon : inside the Raven unit */
    /* for Hawk the LMU registers are initialized inside the unit itself */
    if(SMAIN_NOT_VALID_CNS != UNIT_BASE_ADDR_GET_ALLOW_NON_EXIST_UNIT_MAC(devObjPtr,UNIT_BASE_RAVEN_0))
    {/*start of unit LMU */
        GT_U32    n, k, j;
        GT_U32    lmuNum;
        GT_U32    offset;
        /* init Ravens (0,1) of Pipe 0 only , because the memory manager will add base address to other Ravens */
        {
            for(j = 0; j < 2; j++)/* Num of ravens per pipe */
            {
                for(n = 0 ; n < 2 ; n++)/*  number of LMU units per Raven */
                {
                    lmuNum = j * LMU_NUMBER_PER_RAVEN + n;
                    offset = 0x00430000 + 0x00080000 * n;
                    offset += (j == 0) ? UNIT_BASE_ADDR_GET_MAC(devObjPtr, UNIT_BASE_RAVEN_0):
                                         UNIT_BASE_ADDR_GET_MAC(devObjPtr, UNIT_BASE_RAVEN_1);
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->LMU[lmuNum].averageCoefficient           = offset + 0x00008020;
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->LMU[lmuNum].profileStatisticsReadData[0] = offset + 0x00008030;
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->LMU[lmuNum].profileStatisticsReadData[1] = offset + 0x00008034;
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->LMU[lmuNum].profileStatisticsReadData[2] = offset + 0x00008038;
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->LMU[lmuNum].profileStatisticsReadData[3] = offset + 0x0000803C;
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->LMU[lmuNum].profileStatisticsReadData[4] = offset + 0x00008040;
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->LMU[lmuNum].profileStatisticsReadData[5] = offset + 0x00008044;
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->LMU[lmuNum].summary_cause                = offset + 0x00008230;
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->LMU[lmuNum].summary_mask                 = offset + 0x00008234;
                    for(k = 0; k < 17; k++)
                    {
                        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->LMU[lmuNum].latencyOverThreshold_cause[k] = offset + 0x00008100 + 0x4 * k;
                        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->LMU[lmuNum].latencyOverThreshold_mask[k]  = offset + 0x00008180 + 0x4 * k;
                    }
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->LMU[lmuNum].channelEnable = offset + 0x00008000;
                }
            }
        }
    }/*end of unit LMU */

    if(isDxControlPipe && !SMEM_CHT_IS_SIP6_GET(devObjPtr))
    {
        for(plrUnit = 0 ; plrUnit < SMEM_SIP5_PP_PLR_UNIT__LAST__E; plrUnit++)
        {/*start of unit IPLR */
            {/*c0+n*4*/
                GT_U32    n;
                for(n = 0 ; n <= 63 ; n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PLR[plrUnit].policerInitialDP[n] =
                        0xc0+n*4;
                }/* end of loop n */
            }/*c0+n*4*/
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PLR[plrUnit].policerCtrl0 = 0x00000000;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PLR[plrUnit].policerCtrl1 = 0x00000004;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PLR[plrUnit].globalBillingCntrIndexingModeConfig0 = 0x00000008;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PLR[plrUnit].policerScanCtrl = 0x000000b8;
            {/*228+n*4*/
                GT_U32    n;
                GT_U32    nMax = SMEM_CHT_IS_SIP5_20_GET(devObjPtr) ? 15 : 7;
                for(n = 0 ; n <= nMax ; n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PLR[plrUnit].policerPortMeteringReg[n] =
                        0x228+n*4;
                }/* end of loop n */
            }/*228+n*4*/
            {/*250+x*0x4*/
                GT_U32    x;
                GT_U32    nMax = SMEM_CHT_IS_SIP5_20_GET(devObjPtr) ? 31 : 15;
                GT_U32    offset = SMEM_CHT_IS_SIP5_20_GET(devObjPtr) ? 0x270 : 0x250;
                for(x = 0 ; x <= nMax ; x++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PLR[plrUnit].policerPerPortRemarkTableID[x] =
                        offset+x*0x4;
                }/* end of loop x */
            }/*250+x*0x4*/
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PLR[plrUnit].policerOAM = 0x00000224;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PLR[plrUnit].policerMRU = 0x00000010;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PLR[plrUnit].IPFIXSecondsMSbTimerStampUpload = 0x00000020;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PLR[plrUnit].policerCountingBaseAddr = 0x00000024;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PLR[plrUnit].policerMeteringBaseAddr = 0x00000028;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PLR[plrUnit].policerIPFIXPktCountWAThreshold = 0x00000034;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PLR[plrUnit].policerIPFIXDroppedPktCountWAThreshold = 0x00000030;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PLR[plrUnit].policerIPFIXByteCountWAThresholdMSB = 0x0000003c;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PLR[plrUnit].policerIPFIXByteCountWAThresholdLSB = 0x00000038;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PLR[plrUnit].policerInterruptMask = 0x00000204;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PLR[plrUnit].policerInterruptCause = 0x00000200;
            {/*74+n*4*/
                GT_U32    n;
                for(n = 0 ; n <= 7 ; n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PLR[plrUnit].policerTableAccessData[n] =
                        0x74+n*4;
                }/* end of loop n */
            }/*74+n*4*/
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PLR[plrUnit].IPFIXSampleEntriesLog1 = 0x0000004c;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PLR[plrUnit].policerError = 0x00000050;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PLR[plrUnit].policerErrorCntr = 0x00000054;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PLR[plrUnit].meteringAndCountingRAMSize0 = 0x00000060;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PLR[plrUnit].meteringAndCountingRAMSize1 = 0x00000064;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PLR[plrUnit].meteringAndCountingRAMSize2 = 0x00000068;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PLR[plrUnit].policerTableAccessCtrl = 0x00000070;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PLR[plrUnit].policerMeteringRefreshScanRateLimit = 0x000000b0;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PLR[plrUnit].policerMeteringRefreshScanAddrRange = 0x000000b4;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PLR[plrUnit].IPFIXSecondsLSbTimerStampUpload = 0x0000001c;
            if(!SMEM_CHT_IS_SIP5_20_GET(devObjPtr))/* THE REGISTERS NOT USED IN SIP 5.20*/
            {/*208+n*4*/
                GT_U32    n;
                for(n = 0 ; n <= 1 ; n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PLR[plrUnit].policerShadow[n] =
                        0x208+n*4;
                }/* end of loop n */
            }/*208+n*4*/
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PLR[plrUnit].IPFIXSampleEntriesLog0 = 0x00000048;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PLR[plrUnit].IPFIXNanoTimerStampUpload = 0x00000018;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PLR[plrUnit].IPFIXCtrl = 0x00000014;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PLR[plrUnit].hierarchicalPolicerCtrl = 0x0000002c;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PLR[plrUnit].globalBillingCntrIndexingModeConfig1 = 0x0000000c;
            if(SMEM_CHT_IS_SIP5_20_GET(devObjPtr))
            {
                GT_U32    p;
                for(p = 0 ; p < 512 ; p ++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PLR[plrUnit].portAndPktTypeTranslationTable[p] =
                        0x1800+p*0x4;
                }/* end of loop p */
            }
            else
            {/*1800+p*2<perPort>*/
                GT_U32    p;
                for(p = 0 ; p <= 255 ; p += 2) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PLR[plrUnit].portAndPktTypeTranslationTable[p / 2] =
                        0x1800+(p/2)*0x4;
                }/* end of loop p */
            }/*1800+p*2*/

            if(SMEM_CHT_IS_SIP5_15_GET(devObjPtr))
            {
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PLR[plrUnit].statisticalMeteringConfig0 = 0x00002004;
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PLR[plrUnit].statisticalMeteringConfig1 = 0x00002008;
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PLR[plrUnit].
                    meteringAddressingModeConfiguration0 = 0x00000290;
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PLR[plrUnit].
                    meteringAddressingModeConfiguration1 = 0x00000294;
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PLR[plrUnit].
                    mef10_3_bucket0_max_rate_tab = 0x00002100; /* 128 regs*/
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PLR[plrUnit].
                    mef10_3_bucket1_max_rate_tab = 0x00002300; /* 128 regs */
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PLR[plrUnit].
                    qosProfileToPriority_tab = 0x00002500; /* 128 regs each (8 * 4bits) */
            }
            if(SMEM_CHT_IS_SIP5_20_GET(devObjPtr))
            {
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PLR[plrUnit].
                    meteringAddressingModeConfiguration0 = 0x00000040;
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PLR[plrUnit].
                    meteringAddressingModeConfiguration1 = 0x00000044;
                {/*0002040+4*t*/
                    GT_U32    t;
                    for(t = 0 ; t <= 6 ; t++) {
                        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PLR[plrUnit].globalConfig.countingConfigTableThreshold[t] =
                            0x0002040+4*t;
                    }/* end of loop t */
                }/*0002040+4*t*/
                {/*0002020+4*n*/
                    GT_U32    n;
                    for(n = 0 ; n <= 7 ; n++) {
                        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PLR[plrUnit].globalConfig.countingConfigTableSegment[n] =
                            0x0002020+4*n;
                    }/* end of loop n */
                }/*0002020+4*n*/
            }

            /* PLR */
            if(SMEM_CHT_IS_SIP5_10_GET(devObjPtr))
            {
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PLR[plrUnit].policerBadAddressLatch = 0x00002000;
            }
        }/*end of unit IPLR */
    }

    {/*start of unit CNC */
        {/*start of unit perBlockRegs */
            {/*start of unit wraparound */
                {/*00001498+n*0x10+m*0x4*/
                    GT_U32    n,m;
                    for(n = 0 ; n <= (devObjPtr->cncBlocksNum-1) ; n++) {
                        for(m = 0 ; m <= 3 ; m++) {
                            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->CNC[0].perBlockRegs.wraparound.CNCBlockWraparoundStatusReg[n][m] =
                                0x00001498+n*0x10+m*0x4;
                        }/* end of loop m */
                    }/* end of loop n */
                }/*00001498+n*0x10+m*0x4*/

            }/*end of unit wraparound */


            {
                GT_U32  m,m_formula,m_max ;
                GT_U32  n_formula;
                if(SMEM_CHT_IS_SIP6_30_GET(devObjPtr))
                {
                    /* 0x0001C98 + 0x30*n+m*0x8+j*0x4 : where j (0-1) represents group of ranges, where n (0-7) represents block, where m (0-2) represents Client */
                    m_max     = 2;
                    m_formula = 0x08;
                    n_formula = 0x30;
                }
                else
                {
                    /*00001c98+0x10*n+j*0x4*/
                    m_max     = 0;
                    m_formula = 0x00;
                    n_formula = 0x10;
                }

                for(m = 0; m <= m_max; m++)
                {/*00001c98+0x10*n+j*0x4*/
                    GT_U32    j,n;

                    for(j = 0; j <= 1; j++)
                    {
                        for(n = 0; n <= (devObjPtr->cncBlocksNum-1); n++)
                        {
                            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->CNC[0].perBlockRegs.
                                rangeCount.CNCBlockRangeCountEnable[j][n][m] =
                                0x00001c98 + (n_formula * n) + (j * 0x4) + m*m_formula;
                        }/* end of loop n */
                    }/* end of loop j */

                }/*00001c98+0x10*n+j*0x4*/
            }


            if(SMEM_CHT_IS_SIP6_30_GET(devObjPtr))
            {
                GT_U32    n;
                GT_U32    m,m_max=2;

                for(m = 0; m <= m_max; m++)
                {
                    for(n = 0; n <= (devObjPtr->cncBlocksNum-1); n++)
                    {
                        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->CNC[0].perBlockRegs.
                            clientEnable.CNCBlockConfigReg0[n][m] =
                            0x00001198 + n*0x20+m*0x4;
                    }/* end of loop n */
                }
            }
            else
            {/*start of unit clientEnable */
                {/*00001298+n*0x4*/
                    GT_U32    n;
                    for(n = 0 ; n <= (devObjPtr->cncBlocksNum-1) ; n++) {
                        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->CNC[0].perBlockRegs.clientEnable.CNCBlockConfigReg0[n][0] =
                            0x00001298+n*0x4;
                    }/* end of loop n */
                }/*00001298+n*0x4*/

            }/*end of unit clientEnable */

            /* unit entryMode */
            if(isSip5_20)
            {/*00001398+n*0x4*/
                GT_U32    n;

                for(n = 0; n < ((devObjPtr->cncBlocksNum+7)/8); n++) /* 8 blocks in register */
                {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->CNC[0].perBlockRegs.entryMode.CNCBlocksCounterEntryModeRegister[n] =
                        0x00001398 + (n * 0x4);
                }/* end of loop n */

            }/*00001398+n*0x4*/

        }/*end of unit perBlockRegs */


        {/*start of unit globalRegs */
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->CNC[0].globalRegs.CNCGlobalConfigReg = 0x00000000;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->CNC[0].globalRegs.CNCFastDumpTriggerReg = 0x00000030;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->CNC[0].globalRegs.CNCClearByReadValueRegWord0 = 0x00000040;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->CNC[0].globalRegs.CNCClearByReadValueRegWord1 = 0x00000044;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->CNC[0].globalRegs.CNCInterruptSummaryCauseReg = 0x00000100;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->CNC[0].globalRegs.CNCInterruptSummaryMaskReg = 0x00000104;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->CNC[0].globalRegs.wraparoundFunctionalInterruptCauseReg = 0x00000190;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->CNC[0].globalRegs.wraparoundFunctionalInterruptMaskReg = 0x000001a4;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->CNC[0].globalRegs.rateLimitFunctionalInterruptCauseReg = 0x000001b8;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->CNC[0].globalRegs.rateLimitFunctionalInterruptMaskReg = 0x000001cc;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->CNC[0].globalRegs.miscFunctionalInterruptCauseReg = 0x000001e0;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->CNC[0].globalRegs.miscFunctionalInterruptMaskReg = 0x000001e4;

        }/*end of unit globalRegs */

        {/*manually added*/
            GT_U32 ii;
            SMEM_CHT_PP_REGS_UNIT_START_INFO_STC    startUnitInfo;

            for(ii = 1 ; ii < devObjPtr->cncNumOfUnits; ii++)
            {
                /* save value*/
                startUnitInfo = SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->CNC[ii].startUnitInfo;
                /* copy info value*/
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->CNC[ii] = SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->CNC[0];
                /*restore value*/
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->CNC[ii].startUnitInfo = startUnitInfo;
            }
        }

    }/*end of unit CNC[2] */

    /*****************************/
    /* SIP6 not hold TXQ as SIP5 */
    /*****************************/
    if(!SMEM_CHT_IS_SIP6_GET(devObjPtr))
    {/*start of unit TXQ */


        {/*start of unit queue */
            {/*start of unit tailDrop */
                {/*start of unit tailDropLimits */
                    {/*000a0a20+t*0x4*/
                        GT_U32    t;
                        for(t = 0 ; t <= 7 ; t++) {
                            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.queue.tailDrop.tailDropLimits.poolBufferLimits[t] =
                                0x000a0a20+t*0x4;
                        }/* end of loop t */
                    }/*000a0a20+t*0x4*/
                    {/*000a0a00+t*0x4*/
                        GT_U32    t;
                        for(t = 0 ; t <= 7 ; t++) {
                            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.queue.tailDrop.tailDropLimits.poolDescLimits[t] =
                                0x000a0a00+t*0x4;
                        }/* end of loop t */
                    }/*000a0a00+t*0x4*/
                    {/*000a0820+0x4*n*/
                        GT_U32    n;
                        for(n = 0 ; n <= 15 ; n++) {
                            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.queue.tailDrop.tailDropLimits.profilePortDescLimits[n] =
                                0x000a0820+0x4*n;
                        }/* end of loop n */
                    }/*000a0820+0x4*n*/
                    {/*000a0900+0x4*n*/
                        GT_U32    n;
                        for(n = 0 ; n <= 15 ; n++) {
                            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.queue.tailDrop.tailDropLimits.profilePortBufferLimits[n] =
                                0x000a0900+0x4*n;
                        }/* end of loop n */
                    }/*000a0900+0x4*n*/
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.queue.tailDrop.tailDropLimits.globalDescsLimit = 0x000a0800;
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.queue.tailDrop.tailDropLimits.globalBufferLimit = 0x000a0810;

                }/*end of unit tailDropLimits */


                {/*start of unit tailDropCntrs */
                    {/*000a4130+n*0x4*/
                        GT_U32    n;
                        for(n = 0 ; n <= 71 && n < txqNumPorts; n++) {
                            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.queue.tailDrop.tailDropCntrs.portBuffersCntr[n] =
                                0x000a4130+n*0x4;
                        }/* end of loop n */
                        if(SMEM_CHT_IS_SIP5_20_GET(devObjPtr) == GT_TRUE && n == 72)
                        {
                            for(; n <= 575 ; n++) {
                                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.queue.tailDrop.tailDropCntrs.portBuffersCntr[n] =
                                    0x000b2800+n*0x4;
                            }/* end of loop n */
                        }
                    }/*000a4130+n*0x4*/
                    {/*000a4010+n*0x4*/
                        GT_U32    n;
                        for(n = 0 ; n <= 71 && n < txqNumPorts; n++) {
                            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.queue.tailDrop.tailDropCntrs.portDescCntr[n] =
                                0x000a4010+n*0x4;
                        }/* end of loop n */
                        if(SMEM_CHT_IS_SIP5_20_GET(devObjPtr) == GT_TRUE && n == 72)
                        {
                            for(; n <= 575 ; n++) {
                                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.queue.tailDrop.tailDropCntrs.portDescCntr[n] =
                                    0x000b2000+n*0x4;
                            }/* end of loop n */
                        }
                    }/*000a4010+n*0x4*/
                    {/*000a4250+t*0x4*/
                        GT_U32    t;
                        for(t = 0 ; t <= 7 ; t++) {
                            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.queue.tailDrop.tailDropCntrs.priorityDescCntr[t] =
                                0x000a4250+t*0x4;
                        }/* end of loop t */
                    }/*000a4250+t*0x4*/
                    {/*000a4290+t*0x4*/
                        GT_U32    t;
                        for(t = 0 ; t <= 7 ; t++) {
                            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.queue.tailDrop.tailDropCntrs.priorityBuffersCntr[t] =
                                0x000a4290+t*0x4;
                        }/* end of loop t */
                    }/*000a4290+t*0x4*/
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.queue.tailDrop.tailDropCntrs.totalDescCntr = 0x000a4000;
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.queue.tailDrop.tailDropCntrs.totalBuffersCntr = 0x000a4004;

                }/*end of unit tailDropCntrs */


                {/*start of unit tailDropConfig */
                    {/*000a0200+n * 0x4*/
                        GT_U32    n;
                        for(n = 0 ; n <= 71 && n < txqNumPorts; n++) {
                            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.queue.tailDrop.tailDropConfig.portTailDropCNProfile[n] =
                                0x000a0200+n * 0x4;
                        }/* end of loop n */
                        if(SMEM_CHT_IS_SIP5_20_GET(devObjPtr) == GT_TRUE && n == 72)
                        {
                            for(; n <= 575 ; n++) {
                                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.queue.tailDrop.tailDropConfig.portTailDropCNProfile[n] =
                                    0x000b1800+n*0x4;
                            }/* end of loop n */
                        }
                    }/*000a0200+n * 0x4*/
                    {/*000a0050+n * 0x4*/
                        GT_U32    n;
                        for(n = 0 ; n <= 71 && n < txqNumPorts; n++) {
                            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.queue.tailDrop.tailDropConfig.portEnqueueEnable[n] =
                                0x000a0050+n * 0x4;
                        }/* end of loop n */
                        if(SMEM_CHT_IS_SIP5_20_GET(devObjPtr) == GT_TRUE && n == 72)
                        {
                            for(; n <= 575 ; n++) {
                                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.queue.tailDrop.tailDropConfig.portEnqueueEnable[n] =
                                    0x000b1000+n*0x4;
                            }/* end of loop n */
                        }
                    }/*000a0050+n * 0x4*/
                    {/*000a0004+p*0x4*/
                        GT_U32    p;
                        for(p = 0 ; p <= 15 ; p++) {
                            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.queue.tailDrop.tailDropConfig.profileTailDropConfig[p] =
                                0x000a0004+p*0x4;
                        }/* end of loop p */
                    }/*000a0004+p*0x4*/
                    {/*000a03d0+0x4*p*/
                        GT_U32    p;
                        for(p = 0 ; p <= 15 ; p++) {
                            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.queue.tailDrop.tailDropConfig.profilePriorityQueueToPoolAssociation[p] =
                                0x000a03d0+0x4*p;
                        }/* end of loop p */
                    }/*000a03d0+0x4*p*/
                    {/*000a03a0+t*0x4*/
                        GT_U32    t;
                        for(t = 0 ; t <= 7 ; t++) {
                            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.queue.tailDrop.tailDropConfig.profilePriorityQueueEnablePoolUsage[t] =
                                0x000a03a0+t*0x4;
                        }/* end of loop t */
                    }/*000a03a0+t*0x4*/
                    {/*000a05c0+0x4*p*/
                        GT_U32    p;
                        for(p = 0 ; p <= 15 ; p++) {
                            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.queue.tailDrop.tailDropConfig.profilePoolWRTDEn[p] =
                                0x000a05c0+0x4*p;
                        }/* end of loop p */
                    }/*000a05c0+0x4*p*/
                    {/*000a0580+0x4*p*/
                        GT_U32    p;
                        for(p = 0 ; p <= 15 ; p++) {
                            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.queue.tailDrop.tailDropConfig.profileQueueWRTDEn[p] =
                                0x000a0580+0x4*p;
                        }/* end of loop p */
                    }/*000a0580+0x4*p*/
                    {/*000a0540+0x4*p*/
                        GT_U32    p;
                        for(p = 0 ; p <= 15 ; p++) {
                            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.queue.tailDrop.tailDropConfig.profilePortWRTDEn[p] =
                                0x000a0540+0x4*p;
                        }/* end of loop p */
                    }/*000a0540+0x4*p*/
                    {/*000a0500+0x4*p*/
                        GT_U32    p;
                        for(p = 0 ; p <= 15 ; p++) {
                            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.queue.tailDrop.tailDropConfig.profileMaxQueueWRTDEn[p] =
                                0x000a0500+0x4*p;
                        }/* end of loop p */
                    }/*000a0500+0x4*p*/
                    {/*000a0600+0x4*p*/
                        GT_U32    p;
                        for(p = 0 ; p <= 15 ; p++) {
                            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.queue.tailDrop.tailDropConfig.profileECNMarkEn[p] =
                                0x000a0600+0x4*p;
                        }/* end of loop p */
                    }/*000a0600+0x4*p*/
                    {/*000a0360+0x4*n*/
                        GT_U32    n;
                        for(n = 0 ; n <= 15 ; n++) {
                            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.queue.tailDrop.tailDropConfig.profileCNMTriggeringEnable[n] =
                                0x000a0360+0x4*n;
                        }/* end of loop n */
                    }/*000a0360+0x4*n*/
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.queue.tailDrop.tailDropConfig.globalTailDropConfig = 0x000a0000;
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.queue.tailDrop.tailDropConfig.pktLengthForTailDropEnqueue = 0x000a0044;
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.queue.tailDrop.tailDropConfig.maxQueueWRTDMasks = 0x000a0440;
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.queue.tailDrop.tailDropConfig.portWRTDMasks = 0x000a0444;
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.queue.tailDrop.tailDropConfig.queueWRTDMasks = 0x000a0448;
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.queue.tailDrop.tailDropConfig.poolWRTDMasks = 0x000a044c;

                }/*end of unit tailDropConfig */


                {/*start of unit resourceHistogram */
                    {/*start of unit resourceHistogramLimits */
                        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.queue.tailDrop.resourceHistogram.resourceHistogramLimits.resourceHistogramLimitReg1 = 0x000a9500;
                        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.queue.tailDrop.resourceHistogram.resourceHistogramLimits.resourceHistogramLimitReg2 = 0x000a9504;

                    }/*end of unit resourceHistogramLimits */


                    {/*start of unit resourceHistogramCntrs */
                        {/*000a9510+n*0x4*/
                            GT_U32    n;
                            for(n = 0 ; n <= 3 ; n++) {
                                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.queue.tailDrop.resourceHistogram.resourceHistogramCntrs.resourceHistogramCntr[n] =
                                    0x000a9510+n*0x4;
                            }/* end of loop n */
                        }/*000a9510+n*0x4*/

                    }/*end of unit resourceHistogramCntrs */


                }/*end of unit resourceHistogram */


                {/*start of unit mcFilterLimits */
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.queue.tailDrop.mcFilterLimits.mirroredPktsToAnalyzerPortDescsLimit = 0x000a9000;
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.queue.tailDrop.mcFilterLimits.ingrMirroredPktsToAnalyzerPortBuffersLimit = 0x000a9004;
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.queue.tailDrop.mcFilterLimits.egrMirroredPktsToAnalyzerPortBuffersLimit = 0x000a9010;
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.queue.tailDrop.mcFilterLimits.mcDescsLimit = 0x000a9020;
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.queue.tailDrop.mcFilterLimits.mcBuffersLimit = 0x000a9030;

                }/*end of unit mcFilterLimits */


                {/*start of unit muliticastFilterCntrs */
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.queue.tailDrop.muliticastFilterCntrs.snifferDescsCntr = 0x000a9200;
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.queue.tailDrop.muliticastFilterCntrs.ingrSnifferBuffersCntr = 0x000a9204;
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.queue.tailDrop.muliticastFilterCntrs.egrSnifferBuffersCntr = 0x000a9208;
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.queue.tailDrop.muliticastFilterCntrs.mcDescsCntr = 0x000a920c;
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.queue.tailDrop.muliticastFilterCntrs.mcBuffersCntr = 0x000a9210;

                }/*end of unit muliticastFilterCntrs */


                {/*start of unit FCUAndQueueStatisticsLimits */
                    {/*000aa1b0+n*0x4*/
                        GT_U32    n;
                        for(n = 0 ; n <= 7 ; n++) {
                            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.queue.tailDrop.FCUAndQueueStatisticsLimits.XONLimit[n] =
                                0x000aa1b0+n*0x4;
                        }/* end of loop n */
                    }/*000aa1b0+n*0x4*/
                    {/*000aa110+n*0x4*/
                        GT_U32    n;
                        for(n = 0 ; n <= 7 ; n++) {
                            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.queue.tailDrop.FCUAndQueueStatisticsLimits.XOFFLimit[n] =
                                0x000aa110+n*0x4;
                        }/* end of loop n */
                    }/*000aa110+n*0x4*/
                    {/*000aa0d0+n*0x4*/
                        GT_U32    n;
                        for(n = 0 ; n <= 15 ; n++) {
                            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.queue.tailDrop.FCUAndQueueStatisticsLimits.queueXONLimitProfile[n] =
                                0x000aa0d0+n*0x4;
                        }/* end of loop n */
                    }/*000aa0d0+n*0x4*/
                    {/*000aa090+n*0x4*/
                        GT_U32    n;
                        for(n = 0 ; n <= 15 ; n++) {
                            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.queue.tailDrop.FCUAndQueueStatisticsLimits.queueXOFFLimitProfile[n] =
                                0x000aa090+n*0x4;
                        }/* end of loop n */
                    }/*000aa090+n*0x4*/
                    {/*000aa050+n*0x4*/
                        GT_U32    n;
                        for(n = 0 ; n <= 15 ; n++) {
                            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.queue.tailDrop.FCUAndQueueStatisticsLimits.portXONLimitProfile[n] =
                                0x000aa050+n*0x4;
                        }/* end of loop n */
                    }/*000aa050+n*0x4*/
                    {/*000aa004+n*0x4*/
                        GT_U32    n;
                        for(n = 0 ; n <= 15 ; n++) {
                            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.queue.tailDrop.FCUAndQueueStatisticsLimits.portXOFFLimitProfile[n] =
                                0x000aa004+n*0x4;
                        }/* end of loop n */
                    }/*000aa004+n*0x4*/
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.queue.tailDrop.FCUAndQueueStatisticsLimits.globalXONLimit = 0x000aa230;
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.queue.tailDrop.FCUAndQueueStatisticsLimits.globalXOFFLimit = 0x000aa210;
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.queue.tailDrop.FCUAndQueueStatisticsLimits.FCUMode = 0x000aa000;

                }/*end of unit FCUAndQueueStatisticsLimits */


            }/*end of unit tailDrop */


            {/*start of unit global */
                {/*start of unit interrupt */
                    if(!SMEM_CHT_IS_SIP5_20_GET(devObjPtr))
                    {/*start of unit queueStatistics */
                        {/*000908c0+0x4*i*/
                            GT_U32    i;
                            for(i = 0 ; i <= 17 ; i++) {
                                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.queue.global.interrupt.queueStatistics.lowCrossingStatusReg[i] =
                                    0x000908c0+0x4*i;
                            }/* end of loop i */
                        }/*000908c0+0x4*i*/
                        {/*00090680+p*0x4*/
                            GT_U32    p;
                            for(p = 0 ; p <= 71 && p < txqNumPorts; p++) {
                                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.queue.global.interrupt.queueStatistics.highCrossingInterruptMaskPort[p] =
                                    0x00090680+p*0x4;
                            }/* end of loop p */
                        }/*00090680+p*0x4*/
                        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.queue.global.interrupt.queueStatistics.lowCrossingInterruptSummaryMaskPorts61To31 = 0x0009088c;
                        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.queue.global.interrupt.queueStatistics.lowCrossingInterruptSummaryMaskPorts30To0 = 0x00090884;
                        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.queue.global.interrupt.queueStatistics.lowCrossingInterruptSummaryCausePorts71To62 = 0x00090890;
                        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.queue.global.interrupt.queueStatistics.lowCrossingInterruptSummaryCausePorts61To31 = 0x00090888;
                        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.queue.global.interrupt.queueStatistics.lowCrossingInterruptSummaryCausePorts30To0 = 0x00090880;
                        {/*00090280+p*0x4*/
                            GT_U32    p;
                            for(p = 0 ; p <= 71 && p < txqNumPorts; p++) {
                                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.queue.global.interrupt.queueStatistics.lowCrossingInterruptMaskPort[p] =
                                    0x00090280+p*0x4;
                            }/* end of loop p */
                        }/*00090280+p*0x4*/
                        {/*00090080+0x4*p*/
                            GT_U32    p;
                            for(p = 0 ; p <= 71 && p < txqNumPorts; p++) {
                                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.queue.global.interrupt.queueStatistics.lowCrossingInterruptCausePort[p] =
                                    0x00090080+0x4*p;
                            }/* end of loop p */
                        }/*00090080+0x4*p*/
                        {/*000909a0+0x4*i*/
                            GT_U32    i;
                            for(i = 0 ; i <= 17 ; i++) {
                                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.queue.global.interrupt.queueStatistics.highCrossingStatusReg[i] =
                                    0x000909a0+0x4*i;
                            }/* end of loop i */
                        }/*000909a0+0x4*i*/
                        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.queue.global.interrupt.queueStatistics.highCrossingInterruptSummaryMaskPorts71To62 = 0x000908ac;
                        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.queue.global.interrupt.queueStatistics.lowCrossingInterruptSummaryMaskPorts71To62 = 0x00090894;
                        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.queue.global.interrupt.queueStatistics.highCrossingInterruptSummaryCausePorts30To0 = 0x00090898;
                        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.queue.global.interrupt.queueStatistics.highCrossingInterruptSummaryMaskPorts30To0 = 0x0009089c;
                        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.queue.global.interrupt.queueStatistics.highCrossingInterruptSummaryCausePorts61To31 = 0x000908a0;
                        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.queue.global.interrupt.queueStatistics.highCrossingInterruptSummaryMaskPorts61To31 = 0x000908a4;
                        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.queue.global.interrupt.queueStatistics.highCrossingInterruptSummaryCausePorts71To62 = 0x000908a8;
                        {/*00090480+0x4*p*/
                            GT_U32    p;
                            for(p = 0 ; p <= 71 && p < txqNumPorts; p++) {
                                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.queue.global.interrupt.queueStatistics.highCrossingInterruptCausePort[p] =
                                    0x00090480+0x4*p;
                            }/* end of loop p */
                        }/*00090480+0x4*p*/

                    }/*end of unit queueStatistics */


                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.queue.global.interrupt.transmitQueueInterruptSummaryCause = 0x00090000;
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.queue.global.interrupt.transmitQueueInterruptSummaryMask = 0x00090004;

                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.queue.global.interrupt.generalInterrupt1Cause = 0x00090008;
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.queue.global.interrupt.generalInterrupt1Mask = 0x0009000c;

                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.queue.global.interrupt.portDescFullInterruptCause0 = 0x00090040;
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.queue.global.interrupt.portDescFullInterruptMask0 = 0x00090044;
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.queue.global.interrupt.portBufferFullInterruptCause0 = 0x00090060;
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.queue.global.interrupt.portBufferFullInterruptMask0 = 0x00090064;

                    if(txqNumPorts > 32)
                    {
                        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.queue.global.interrupt.portDescFullInterruptCause1 = 0x00090048;
                        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.queue.global.interrupt.portDescFullInterruptMask1 = 0x0009004c;
                        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.queue.global.interrupt.portBufferFullInterruptCause1 = 0x00090068;
                        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.queue.global.interrupt.portBufferFullInterruptMask1 = 0x0009006c;
                    }

                    if(txqNumPorts > 64)
                    {
                        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.queue.global.interrupt.portDescFullInterruptCause2 = 0x00090050;
                        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.queue.global.interrupt.portDescFullInterruptMask2 = 0x00090054;
                        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.queue.global.interrupt.portBufferFullInterruptCause2 = 0x00090070;
                        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.queue.global.interrupt.portBufferFullInterruptMask2 = 0x00090074;
                    }
                }/*end of unit interrupt */


                {/*start of unit ECCCntrs */
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.queue.global.ECCCntrs.TDClrECCErrorCntr = 0x00090a40;

                }/*end of unit ECCCntrs */


                {/*start of unit dropCntrs */
                    {/*00090a00+p*0x4*/
                        GT_U32    p;
                        for(p = 0 ; p <= 3 ; p++) {
                            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.queue.global.dropCntrs.clearPktsDroppedCntrPipe[p] =
                                0x00090a00+p*0x4;
                        }/* end of loop p */
                    }/*00090a00+p*0x4*/

                }/*end of unit dropCntrs */

            }/*end of unit global */


            {/*start of unit peripheralAccess */
                {/*start of unit peripheralAccessMisc */
                    {/*00093004+0x4*n*/
                        GT_U32    n;
                        for(n = 0 ; n <= 3 ; n++) {
                            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.queue.peripheralAccess.peripheralAccessMisc.portGroupDescToEQCntr[n] =
                                0x00093004+0x4*n;
                        }/* end of loop n */
                    }/*00093004+0x4*n*/
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.queue.peripheralAccess.peripheralAccessMisc.peripheralAccessMisc = 0x00093000;
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.queue.peripheralAccess.peripheralAccessMisc.QCNIncArbiterCtrl = 0x00093020;

                }/*end of unit peripheralAccessMisc */


                {/*start of unit egrMIBCntrs */
                    {/*00093208+n*0x4*/
                        GT_U32    n;
                        for(n = 0 ; n <= 1 ; n++) {
                            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.queue.peripheralAccess.egrMIBCntrs.txQMIBCntrsPortSetConfig[n] =
                                0x00093208+n*0x4;
                        }/* end of loop n */
                    }/*00093208+n*0x4*/
                    {/*00093200+0x4*n*/
                        GT_U32    n;
                        for(n = 0 ; n <= 1 ; n++) {
                            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.queue.peripheralAccess.egrMIBCntrs.txQMIBCntrsSetConfig[n] =
                                0x00093200+0x4*n;
                        }/* end of loop n */
                    }/*00093200+0x4*n*/
                    {/*00093250+0x4*n*/
                        GT_U32    n;
                        for(n = 0 ; n <= 1 ; n++) {
                            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.queue.peripheralAccess.egrMIBCntrs.setTailDroppedPktCntr[n] =
                                0x00093250+0x4*n;
                        }/* end of loop n */
                    }/*00093250+0x4*n*/
                    {/*00093210+0x4*n*/
                        GT_U32    n;
                        for(n = 0 ; n <= 1 ; n++) {
                            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.queue.peripheralAccess.egrMIBCntrs.setOutgoingUcPktCntr[n] =
                                0x00093210+0x4*n;
                        }/* end of loop n */
                    }/*00093210+0x4*n*/
                    {/*00093220+0x4*n*/
                        GT_U32    n;
                        for(n = 0 ; n <= 1 ; n++) {
                            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.queue.peripheralAccess.egrMIBCntrs.setOutgoingMcPktCntr[n] =
                                0x00093220+0x4*n;
                        }/* end of loop n */
                    }/*00093220+0x4*n*/
                    {/*00093230+0x4*n*/
                        GT_U32    n;
                        for(n = 0 ; n <= 1 ; n++) {
                            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.queue.peripheralAccess.egrMIBCntrs.setOutgoingBcPktCntr[n] =
                                0x00093230+0x4*n;
                        }/* end of loop n */
                    }/*00093230+0x4*n*/
                    {/*00093290+0x4*n*/
                        GT_U32    n;
                        for(n = 0 ; n <= 1 ; n++) {
                            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.queue.peripheralAccess.egrMIBCntrs.setMcFIFO7_4DroppedPktsCntr[n] =
                                0x00093290+0x4*n;
                        }/* end of loop n */
                    }/*00093290+0x4*n*/
                    {/*00093280+0x4*n*/
                        GT_U32    n;
                        for(n = 0 ; n <= 1 ; n++) {
                            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.queue.peripheralAccess.egrMIBCntrs.setMcFIFO3_0DroppedPktsCntr[n] =
                                0x00093280+0x4*n;
                        }/* end of loop n */
                    }/*00093280+0x4*n*/
                    {/*00093260+0x4*n*/
                        GT_U32    n;
                        for(n = 0 ; n <= 1 ; n++) {
                            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.queue.peripheralAccess.egrMIBCntrs.setCtrlPktCntr[n] =
                                0x00093260+0x4*n;
                        }/* end of loop n */
                    }/*00093260+0x4*n*/
                    {/*00093240+0x4*n*/
                        GT_U32    n;
                        for(n = 0 ; n <= 1 ; n++) {
                            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.queue.peripheralAccess.egrMIBCntrs.setBridgeEgrFilteredPktCntr[n] =
                                0x00093240+0x4*n;
                        }/* end of loop n */
                    }/*00093240+0x4*n*/

                }/*end of unit egrMIBCntrs */


                {/*start of unit CNCModes */
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.queue.peripheralAccess.CNCModes.CNCModes = 0x000936a0;

                }/*end of unit CNCModes */


            }/*end of unit peripheralAccess */


        }/*end of unit queue */



        if(SMAIN_NOT_VALID_CNS != UNIT_BASE_ADDR_GET_ALLOW_NON_EXIST_UNIT_MAC(devObjPtr,UNIT_TXQ_LL))
        {/*start of unit ll */
            {/*start of unit global */
                {/*start of unit portsLinkEnableStatus */
                    GT_U32    p;
                    GT_U32    nMax = isPipeDevice ? 0 : isSip5_25 ? 3 : SMEM_CHT_IS_SIP5_20_GET(devObjPtr) ? 17 : 2;
                    GT_U32    offset = SMEM_CHT_IS_SIP5_20_GET(devObjPtr) ? 0x00112100 : 0x000a0600;
                    for(p = 0 ; p <= nMax ; p++) {
                        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.ll.global.portsLinkEnableStatus.portsLinkEnableStatus[p] =
                            offset + 0x4*p;
                    }
                }/*end of unit portsLinkEnableStatus */


                {/*start of unit interrupts */
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.ll.global.interrupts.LLInterruptCause =
                        SMEM_CHT_IS_SIP5_20_GET(devObjPtr) ? 0x00112008 : 0x000a0508;
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.ll.global.interrupts.LLInterruptMask =
                        SMEM_CHT_IS_SIP5_20_GET(devObjPtr) ? 0x0011200c : 0x000a050c;

                }/*end of unit interrupts */

                if (!SMEM_CHT_IS_SIP5_20_GET(devObjPtr)) /* BC3 does not have such registers */
                {/*start of unit globalLLConfig */
                    {/*000a0300+0x4*p*/
                        GT_U32    p;
                        GT_U32    offset = 0x000a0300;
                        for(p = 0 ; p <= 15 ; p++) {
                            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.ll.global.globalLLConfig.profilePriorityQueueToPoolAssociation[p] =
                                offset+0x4*p;
                        }/* end of loop p */
                    }/*000a0300+0x4*p*/
                    {/*000a0040+n * 0x4*/
                        GT_U32    n;
                        GT_U32    nMax = 71;
                        GT_U32    offset = SMEM_CHT_IS_SIP5_10_GET(devObjPtr) ? 0x000a0044 : 0x000a0040;
                        for(n = 0 ; n <= nMax ; n++) {
                            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.ll.global.globalLLConfig.portProfile[n] =
                                offset+n * 0x4;
                        }/* end of loop n */
                    }/*000a0040+n * 0x4*/
                    {/*000a0004+n*0x4*/
                        GT_U32    n;
                        GT_U32    nMax = 7;
                        GT_U32    offset = 0x000a0004;

                        for(n = 0 ; n <= nMax ; n++) {
                            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.ll.global.globalLLConfig.highSpeedPort[n] =
                                offset+n*0x4;
                        }/* end of loop n */
                    }/*000a0004+n*0x4*/

                }/*end of unit globalLLConfig */


            }/*end of unit global */


        }/*end of unit ll */


        if(SMAIN_NOT_VALID_CNS != UNIT_BASE_ADDR_GET_ALLOW_NON_EXIST_UNIT_MAC(devObjPtr,UNIT_CPFC))
        {/*start of unit pfc */
            {/*start of unit PFCInterrupts */
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.pfc.PFCInterrupts.PFCPortGroupGlobalOverUnderflowInterruptMask = 0x00001350;
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.pfc.PFCInterrupts.PFCInterruptSummaryCause = 0x00001300;
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.pfc.PFCInterrupts.PFCInterruptSummaryMask = 0x00001304;
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.pfc.PFCInterrupts.PFCParityInterruptCause = 0x00001310;
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.pfc.PFCInterrupts.PFCCntrsOverUnderFlowInterruptMask = 0x00001324;
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.pfc.PFCInterrupts.PFCPortGroupGlobalOverUnderflowInterruptCause = 0x00001330;
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.pfc.PFCInterrupts.PFCParityInterruptMask = 0x00001314;
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.pfc.PFCInterrupts.PFCFIFOsOverrunsInterruptCause = 0x000013b0;
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.pfc.PFCInterrupts.PFCFIFOsOverrunsInterruptMask = 0x000013b4;
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.pfc.PFCInterrupts.PFCMsgsSetsOverrunsInterruptCause = 0x000013b8;
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.pfc.PFCInterrupts.PFCMsgsSetsOverrunsInterruptMask = 0x000013bc;
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.pfc.PFCInterrupts.PFCCntrsOverUnderFlowInterruptCause = 0x00001320;

            }/*end of unit PFCInterrupts */


            {/*00000170+0x4*n*/
                GT_U32    n;
                if(0 == SMEM_CHT_IS_SIP5_20_GET(devObjPtr))
                {
                    for(n = 0 ; n <= 63 ; n++) {
                        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.pfc.LLFCTargetPortRemap[n] =
                            0x00000170+0x4*n;
                    }/* end of loop n */
                }
                else
                {
                    for(n = 0 ; n < ((txqNumPorts+3)/4) ; n++) { /*4 ports in register*/
                        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.pfc.LLFCTargetPortRemap[n] =
                            0x000002F0+0x4*n;
                    }/* end of loop n */
                }
            }/*00000170+0x4*n*/
            {/*00000010+0x4*n*/
                GT_U32    n;
                if(0 == SMEM_CHT_IS_SIP5_20_GET(devObjPtr))
                {
                    for(n = 0 ; n <= 7 ; n++) {
                        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.pfc.portFCMode[n] =
                            0x00000010+0x4*n;
                    }/* end of loop n */
                }
                else
                {
                    for(n = 0 ; n < ((txqNumPorts+31)/32) ; n++) {
                        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.pfc.portFCMode[n] =
                            0x000006F0+0x4*n;
                    }/* end of loop n */
                }
            }/*00000010+0x4*n*/
            {/*00000900+p*4*/
                GT_U32    p;
                if(0 == SMEM_CHT_IS_SIP5_20_GET(devObjPtr))
                {
                    for(p = 0 ; p <= 255 ; p++) {
                        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.pfc.PFCSourcePortToPFCIndexMap[p] =
                            0x00000900+p*4;
                    }/* end of loop p */
                }
                else
                {
                    for(p = 0 ; p <= (GT_U32)(isPipeDevice ? 31 : 511) ; p++) {
                        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.pfc.PFCSourcePortToPFCIndexMap[p] =
                            0x00001C00+p*4;
                    }/* end of loop p */
                }
            }/*00000900+p*4*/
            {/*000000f0+0x4*r*/
                GT_U32    r;
                if(0 == SMEM_CHT_IS_SIP5_20_GET(devObjPtr))
                {
                    for(r = 0 ; r <= 31 ; r++) {
                        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.pfc.PFCPortProfile[r] =
                            0x000000f0+0x4*r;
                    }/* end of loop r */
                }
                else
                {
                    for(r = 0 ; r <= (GT_U32)(isPipeDevice ? 3 : 63) ; r++) {
                        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.pfc.PFCPortProfile[r] =
                            0x000000f0+0x4*r;
                    }/* end of loop r */
                }

            }/*000000f0+0x4*r*/
            {/*00000050+0x4*p*/
                GT_U32    p;
                for(p = 0 ; p <= 0 ; p++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.pfc.PFCPortGroupFlowCtrlIndicationFIFOOverrunEntry0[p] =
                        0x00000050+0x4*p;
                }/* end of loop p */
            }/*00000050+0x4*p*/
            {/*00000070+0x4*p*/
                GT_U32    p;
                for(p = 0 ; p <= 0 ; p++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.pfc.PFCPortGroupFlowCtrlIndicationFIFOOverrunEntry1[p] =
                        0x00000070+0x4*p;
                }/* end of loop p */
            }/*00000070+0x4*p*/
            {/*00000090+0x4*p*/
                GT_U32    p;
                for(p = 0 ; p <= 0 ; p++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.pfc.PFCPortGroupFlowCtrlIndicationFIFOOverrunCntr[p] =
                        0x00000090+0x4*p;
                }/* end of loop p */
            }/*00000090+0x4*p*/
            if(0 == SMEM_CHT_IS_SIP5_10_GET(devObjPtr))
            {/*000000b0+0x4*p*/
                GT_U32    p;
                for(p = 0 ; p <= 0 ; p++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.pfc.PFCPortGroupEqFIFOOverrunCntr[p] =
                        0x000000b0+0x4*p;
                }/* end of loop p */
            }/*000000b0+0x4*p*/
            {/*00000030+0x4*p*/
                GT_U32    p;
                for(p = 0 ; p <= 0 ; p++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.pfc.PFCPortGroupCntrsStatus[p] =
                        0x00000030+0x4*p;
                }/* end of loop p */
            }/*00000030+0x4*p*/
            {/*000000d0+0x4*p*/
                GT_U32    p;
                for(p = 0 ; p <= 0 ; p++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.pfc.PFCPortGroupCntrsParityErrorsCntr[p] =
                        0x000000d0+0x4*p;
                }/* end of loop p */
            }/*000000d0+0x4*p*/
            {/*00014200+0x20*p + 0x4*t*/
                GT_U32    t,p;
                for(t = 0 ; t <= 7 ; t++) {
                    for(p = 0 ; p <= 0 ; p++) {
                        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.pfc.PFCGroupOfPortsTCXonThresholds[t][p] =
                            0x00014200+0x20*p + 0x4*t;
                    }/* end of loop p */
                }/* end of loop t */
            }/*00014200+0x20*p + 0x4*t*/
            {/*00014000+0x20*p + 0x4*t*/
                GT_U32    t,p;
                for(t = 0 ; t <= 7 ; t++) {
                    for(p = 0 ; p <= 0 ; p++) {
                        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.pfc.PFCGroupOfPortsTCXoffThresholds[t][p] =
                            0x00014000+0x20*p + 0x4*t;
                    }/* end of loop p */
                }/* end of loop t */
            }/*00014000+0x20*p + 0x4*t*/
            {/*00014400+0x20*p + 0x4*t*/
                GT_U32    t,p;
                for(t = 0 ; t <= 7 ; t++) {
                    for(p = 0 ; p <= 0 ; p++) {
                        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.pfc.PFCGroupOfPortsTCDropThresholds[t][p] =
                            0x00014400+0x20*p + 0x4*t;
                    }/* end of loop p */
                }/* end of loop t */
            }/*00014400+0x20*p + 0x4*t*/
            {/*00000d00+0x20*p + 0x4*t*/
                GT_U32    p,t;
                for(p = 0 ; p <= 0 ; p++) {
                    for(t = 0 ; t <= 7 ; t++) {
                        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.pfc.PFCGroupOfPortsTCCntr[p][t] =
                            0x00000d00+0x20*p + 0x4*t;
                    }/* end of loop t */
                }/* end of loop p */
            }/*00000d00+0x20*p + 0x4*t*/
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.pfc.PFCTriggerGlobalConfig = 0x00000000;

        }/*end of unit pfc */


        if(SMAIN_NOT_VALID_CNS != UNIT_BASE_ADDR_GET_ALLOW_NON_EXIST_UNIT_MAC(devObjPtr,UNIT_TXQ_QCN))
        {/*start of unit qcn */
            {/*00000b40+0x4*n*/
                GT_U32    n;
                GT_U32    offset = SMEM_CHT_IS_SIP5_20_GET(devObjPtr) ? 0x00000d00 : 0x00000b40;
                GT_U32    nMax = SMEM_CHT_IS_SIP5_20_GET(devObjPtr) ? 71 : 8;
                for(n = 0 ; n <= nMax ; n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.qcn.CCFCSpeedIndex[n] =
                        offset+0x4*n;
                }/* end of loop n */
            }/*00000b40+0x4*n*/
            {/*00000090+0x4*n*/
                GT_U32    n;
                for(n = 0 ; n <= 7 ; n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.qcn.CNSampleTbl[n] =
                        0x00000090+0x4*n;
                }/* end of loop n */
            }/*00000090+0x4*n*/
            {/*00000300+0x20*p + 0x4*t*/
                GT_U32    p,t;
                for(p = 0 ; p <= 7 ; p++) {
                    for(t = 0 ; t <= 7 ; t++) {
                        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.qcn.profilePriorityQueueCNThreshold[p][t] =
                            0x00000300+0x20*p + 0x4*t;
                    }/* end of loop t */
                }/* end of loop p */
            }/*00000300+0x20*p + 0x4*t*/
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.qcn.feedbackMIN = 0x00000050;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.qcn.feedbackMAX = 0x00000060;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.qcn.CNGlobalConfig = 0x00000000;
            {/*00000b20+0x4*n*/
                GT_U32    n;
                if(0 == SMEM_CHT_IS_SIP5_20_GET(devObjPtr))
                {
                    for(n = 0 ; n <= 7 ; n++) {
                        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.qcn.enFCTriggerByCNFrameOnPort[n] =
                            0x00000b20+0x4*n;
                    }/* end of loop n */
                }
                else
                {
                    for(n = 0 ; n <= 15 ; n++) {
                        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.qcn.enFCTriggerByCNFrameOnPort[n] =
                            0x00000c00+0x4*n;
                    }/* end of loop n */
                }
            }/*00000b20+0x4*n*/
            {/*00000b00+0x4*n*/
                GT_U32    n;
                if(0 == SMEM_CHT_IS_SIP5_20_GET(devObjPtr))
                {
                    for(n = 0 ; n <= 7 ; n++) {
                        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.qcn.enCNFrameTxOnPort[n] =
                            0x00000b00+0x4*n;
                    }/* end of loop n */
                }
                else
                {
                    for(n = 0 ; n <= 15 ; n++) {
                        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.qcn.enCNFrameTxOnPort[n] =
                            0x00000b00+0x4*n;
                    }/* end of loop n */
                }
            }/*00000b00+0x4*n*/
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.qcn.feedbackCalcConfigs = 0x00000040;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.qcn.txqECCConf = 0x00000130;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.qcn.descECCSingleErrorCntr = 0x00000120;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.qcn.descECCDoubleErrorCntr = 0x00000124;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.qcn.CNTargetAssignment = 0x00000004;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.qcn.CNDropCntr = 0x000000f0;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.qcn.QCNInterruptCause = 0x00000100;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.qcn.QCNInterruptMask = 0x00000110;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.qcn.CNDescAttributes = 0x00000008;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.qcn.CNBufferFIFOParityErrorsCntr = 0x000000e0;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.qcn.CNBufferFIFOOverrunsCntr = 0x000000d0;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.qcn.ECCStatus = 0x00000140;

        }/*end of unit qcn */


        if(SMAIN_NOT_VALID_CNS != UNIT_BASE_ADDR_GET_ALLOW_NON_EXIST_UNIT_MAC(devObjPtr,UNIT_TXQ_DQ))
        {/*start of unit dq */
            GT_U32  numTxqDqPortsPerUnit_firstBlock = (SIP_5_DQ_NUM_PORTS_CNS - 1);
            GT_U32  numTxqDqPortsPerUnit_secondBlock = SMEM_CHT_IS_SIP5_20_GET(devObjPtr) ?
                            (SIP_5_20_DQ_NUM_PORTS_CNS - SIP_5_DQ_NUM_PORTS_CNS) :
                            0;
            GT_U32  ii;

            if(devObjPtr->multiDataPath.txqDqNumPortsPerDp)
            {
                if((numTxqDqPortsPerUnit_firstBlock+1) > devObjPtr->multiDataPath.txqDqNumPortsPerDp)
                {
                    numTxqDqPortsPerUnit_firstBlock = devObjPtr->multiDataPath.txqDqNumPortsPerDp - 1;
                    numTxqDqPortsPerUnit_secondBlock = 0;
                }
            }

            {/*start of unit statisticalAndCPUTrigEgrMirrToAnalyzerPort */
                {/*0000d040+i*0x4*/
                    GT_U32    i;
                    if(0 == SMEM_CHT_IS_SIP5_20_GET(devObjPtr))
                    {
                        for(i = 0 ; i <= 31 ; i++) {
                            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.dq[0].statisticalAndCPUTrigEgrMirrToAnalyzerPort.egrAnalyzerEnableIndex[i] =
                                0x0000d040+i*0x4;
                        }/* end of loop i */
                    }
                    else
                    {
                        GT_U32  numRegs = isPipeDevice ? 3 : (shtNumPorts /8);

                        for(i = 0 ; i < numRegs ; i++) {
                            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.dq[0].statisticalAndCPUTrigEgrMirrToAnalyzerPort.egrAnalyzerEnableIndex[i] =
                                0x0000d040+i*0x4;
                        }/* end of loop i */
                    }
                }/*0000d040+i*0x4*/
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.dq[0].statisticalAndCPUTrigEgrMirrToAnalyzerPort.STCStatisticalTxSniffConfig = 0x0000d000;
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.dq[0].statisticalAndCPUTrigEgrMirrToAnalyzerPort.egrCTMTrigger = 0x0000d004;
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.dq[0].statisticalAndCPUTrigEgrMirrToAnalyzerPort.CTMAnalyzerIndex = 0x0000d010;
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.dq[0].statisticalAndCPUTrigEgrMirrToAnalyzerPort.STCAnalyzerIndex = 0x0000d014;

            }/*end of unit statisticalAndCPUTrigEgrMirrToAnalyzerPort */


            {/*start of unit shaper */
                {/*00004100+n*0x4*/
                    GT_U32    n;
                    for(n = 0 ; n <= numTxqDqPortsPerUnit_firstBlock ; n++) {
                        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.dq[0].shaper.portTokenBucketMode[n] =
                            0x00004100+n*0x4;
                    }/* end of loop n */
                    {
                        GT_U32  baseAddr = 0x00010000;
                        /* continue with n */
                        for(ii = 0 ; ii < numTxqDqPortsPerUnit_secondBlock ; ii++,n++)
                        {
                            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.dq[0].shaper.portTokenBucketMode[n] =
                                baseAddr+n*0x4;
                        }
                    }
                }/*00004100+n*0x4*/
                {/*00004300+n * 0x4*/
                    GT_U32    n;
                    for(n = 0 ; n <= numTxqDqPortsPerUnit_firstBlock ; n++) {
                        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.dq[0].shaper.portDequeueEnable[n] =
                            0x00004300+n * 0x4;
                    }/* end of loop n */
                    {
                        GT_U32  baseAddr = 0x00010100;
                        /* continue with n */
                        for(ii = 0 ; ii < numTxqDqPortsPerUnit_secondBlock ; ii++,n++)
                        {
                            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.dq[0].shaper.portDequeueEnable[n] =
                                baseAddr+n*0x4;
                        }
                    }
                }/*00004300+n * 0x4*/
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.dq[0].shaper.tokenBucketUpdateRate = 0x00004000;
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.dq[0].shaper.tokenBucketBaseLine = 0x00004004;
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.dq[0].shaper.CPUTokenBucketMTU = 0x00004008;
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.dq[0].shaper.portsTokenBucketMTU = 0x0000400c;

            }/*end of unit shaper */


            {/*start of unit scheduler */
                {/*start of unit schedulerConfig */
                    {/*00001040+n * 0x4*/
                        GT_U32    n;
                        for(n = 0 ; n <= numTxqDqPortsPerUnit_firstBlock ; n++) {
                            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.dq[0].scheduler.schedulerConfig.conditionalMaskForPort[n] =
                                0x00001040+n * 0x4;
                        }/* end of loop n */
                        {
                            GT_U32  baseAddr = 0x0000F000;
                            /* continue with n */
                            for(ii = 0 ; ii < numTxqDqPortsPerUnit_secondBlock ; ii++,n++)
                            {
                                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.dq[0].scheduler.schedulerConfig.conditionalMaskForPort[n] =
                                    baseAddr+n*0x4;
                            }
                        }
                    }/*00001040+n * 0x4*/
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.dq[0].scheduler.schedulerConfig.schedulerConfig = 0x00001000;
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.dq[0].scheduler.schedulerConfig.pktLengthForTailDropDequeue = 0x00001008;
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.dq[0].scheduler.schedulerConfig.highSpeedPorts[0] = 0x0000100c;

                }/*end of unit schedulerConfig */


                {/*start of unit priorityArbiterWeights */
                    {/*000012c0+p*0x4*/
                        GT_U32    p;
                        for(p = 0 ; p <= 15 ; p++) {
                            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.dq[0].scheduler.priorityArbiterWeights.profileSDWRRGroup[p] =
                                0x000012c0+p*0x4;
                        }/* end of loop p */
                    }/*000012c0+p*0x4*/
                    {/*00001240+p*0x4*/
                        GT_U32    p;
                        for(p = 0 ; p <= 15 ; p++) {
                            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.dq[0].scheduler.priorityArbiterWeights.profileSDWRRWeightsConfigReg0[p] =
                                0x00001240+p*0x4;
                        }/* end of loop p */
                    }/*00001240+p*0x4*/
                    {/*00001280+p*0x4*/
                        GT_U32    p;
                        for(p = 0 ; p <= 15 ; p++) {
                            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.dq[0].scheduler.priorityArbiterWeights.profileSDWRRWeightsConfigReg1[p] =
                                0x00001280+p*0x4;
                        }/* end of loop p */
                    }/*00001280+p*0x4*/
                    {/*00001300+p*0x4*/
                        GT_U32    p;
                        for(p = 0 ; p <= 15 ; p++) {
                            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.dq[0].scheduler.priorityArbiterWeights.profileSDWRREnable[p] =
                                0x00001300+p*0x4;
                        }/* end of loop p */
                    }/*00001300+p*0x4*/
                    {/*00001380+n * 0x4*/
                        GT_U32    n;
                        for(n = 0 ; n <= numTxqDqPortsPerUnit_firstBlock ; n++) {
                            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.dq[0].scheduler.priorityArbiterWeights.portSchedulerProfile[n] =
                                0x00001380+n * 0x4;
                        }/* end of loop n */
                        {
                            GT_U32  baseAddr = 0x0000F100;
                            /* continue with n */
                            for(ii = 0 ; ii < numTxqDqPortsPerUnit_secondBlock ; ii++,n++)
                            {
                                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.dq[0].scheduler.priorityArbiterWeights.portSchedulerProfile[n] =
                                    baseAddr+n*0x4;
                            }
                        }
                    }/*00001380+n * 0x4*/

                }/*end of unit priorityArbiterWeights */


                {/*start of unit portShaper */
                    {/*0000190c+0x4*n*/
                        GT_U32    n;
                        for(n = 0 ; n <= numTxqDqPortsPerUnit_firstBlock ; n++) {
                            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.dq[0].scheduler.portShaper.portRequestMaskSelector[n] =
                                0x0000190c+0x4*n;
                        }/* end of loop n */
                        {
                            GT_U32  baseAddr = 0x0000F300;
                            /* continue with n */
                            for(ii = 0 ; ii < numTxqDqPortsPerUnit_secondBlock ; ii++,n++)
                            {
                                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.dq[0].scheduler.portShaper.portRequestMaskSelector[n] =
                                    baseAddr+n*0x4;
                            }
                        }
                    }/*0000190c+0x4*n*/
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.dq[0].scheduler.portShaper.portRequestMask = 0x00001904;

                }/*end of unit portShaper */


                {/*start of unit portArbiterConfig */
                    {/*00001510+n*0x4*/
                        GT_U32    n;
                        for(n = 0 ; n <= numTxqDqPortsPerUnit_firstBlock ; n++) {
                            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.dq[0].scheduler.portArbiterConfig.portsArbiterPortWorkConservingEnable[n] =
                                0x00001510+n*0x4;
                        }/* end of loop n */
                        {
                            GT_U32  baseAddr = 0x0000F200;
                            /* continue with n */
                            for(ii = 0 ; ii < numTxqDqPortsPerUnit_secondBlock ; ii++,n++)
                            {
                                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.dq[0].scheduler.portArbiterConfig.portsArbiterPortWorkConservingEnable[n] =
                                    baseAddr+n*0x4;
                            }
                        }
                    }/*00001510+n*0x4*/
                    {/*00001650+n*4*/
                        GT_U32    n;
                        for(n = 0 ; n <= 84 ; n++) {
                            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.dq[0].scheduler.portArbiterConfig.portsArbiterMap[n] =
                                0x00001650+n*4;
                        }/* end of loop n */
                    }/*00001650+n*4*/
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.dq[0].scheduler.portArbiterConfig.portsArbiterConfig = 0x00001500;
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.dq[0].scheduler.portArbiterConfig.portsArbiterStatus = 0x00001800;

                }/*end of unit portArbiterConfig */


            }/*end of unit scheduler */


            {/*start of unit global */
                {/*start of unit memoryParityError */
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.dq[0].global.memoryParityError.tokenBucketPriorityParityErrorCntr = 0x00000800;
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.dq[0].global.memoryParityError.parityErrorBadAddr = 0x00000808;

                }/*end of unit memoryParityError */


                {/*start of unit interrupt */
                    GT_U32  pMax;
                    /* An interrupt register stores 31 'state' bits (+ 1 summary bit) */
                    pMax = devObjPtr->multiDataPath.txqDqNumPortsPerDp ?
                           ((devObjPtr->multiDataPath.txqDqNumPortsPerDp + 30) / 31) - 1 :
                           3;


                    {/*00000610+p*0x4*/
                        GT_U32    p;
                        for(p = 0 ; p <= pMax ; p++) {
                            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.dq[0].global.interrupt.flushDoneInterruptCause[p] =
                                0x00000610+p*0x4;
                        }/* end of loop p */
                    }/*00000610+p*0x4*/
                    {/*00000620+p*0x4*/
                        GT_U32    p;
                        for(p = 0 ; p <= pMax ; p++) {
                            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.dq[0].global.interrupt.flushDoneInterruptMask[p] =
                                0x00000620+p*0x4;
                        }/* end of loop p */
                    }/*00000620+p*0x4*/
                    {/*00000630+p*0x4*/
                        GT_U32    p;
                        for(p = 0 ; p <= pMax ; p++) {
                            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.dq[0].global.interrupt.egrSTCInterruptCause[p] =
                                0x00000630+p*0x4;
                        }/* end of loop p */
                    }/*00000630+p*0x4*/
                    {/*00000640+p*0x4*/
                        GT_U32    p;
                        for(p = 0 ; p <= pMax ; p++) {
                            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.dq[0].global.interrupt.egrSTCInterruptMask[p] =
                                0x00000640+p*0x4;
                        }/* end of loop p */
                    }/*00000640+p*0x4*/
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.dq[0].global.interrupt.txQDQInterruptSummaryCause = 0x00000600;
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.dq[0].global.interrupt.txQDQInterruptSummaryMask = 0x00000604;
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.dq[0].global.interrupt.txQDQMemoryErrorInterruptCause = 0x00000650;
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.dq[0].global.interrupt.txQDQMemoryErrorInterruptMask = 0x00000660;
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.dq[0].global.interrupt.txQDQGeneralInterruptCause = 0x00000670;
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.dq[0].global.interrupt.txQDQGeneralInterruptMask = 0x00000680;

                }/*end of unit interrupt */


                {/*start of unit globalDQConfig */
                    {/*000002c0+n*0x4*/
                        GT_U32    n;
                        for(n = 0 ; n <= numTxqDqPortsPerUnit_firstBlock ; n++) {
                            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.dq[0].global.globalDQConfig.portToDMAMapTable[n] =
                                0x000002c0+n*0x4;
                        }/* end of loop n */
                        {
                            GT_U32  baseAddr = 0x0000E100;
                            /* continue with n */
                            for(ii = 0 ; ii < numTxqDqPortsPerUnit_secondBlock ; ii++,n++)
                            {
                                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.dq[0].global.globalDQConfig.portToDMAMapTable[n] =
                                    baseAddr+n*0x4;
                            }
                        }
                    }/*000002c0+n*0x4*/
                    {/*0000020c+p*0x4*/
                        GT_U32    p;
                        for(p = 0 ; p <= 15 ; p++) {
                            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.dq[0].global.globalDQConfig.profileByteCountModification[p] =
                                0x0000020c+p*0x4;
                        }/* end of loop p */
                    }/*0000020c+p*0x4*/
                    {/*00000004+n*0x4*/
                        GT_U32    n;
                        for(n = 0 ; n <= numTxqDqPortsPerUnit_firstBlock ; n++) {
                            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.dq[0].global.globalDQConfig.portBCConstantValue[n] =
                                0x00000004+n*0x4;
                        }/* end of loop n */
                        {
                            GT_U32  baseAddr = 0x0000E000;
                            /* continue with n */
                            for(ii = 0 ; ii < numTxqDqPortsPerUnit_secondBlock ; ii++,n++)
                            {
                                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.dq[0].global.globalDQConfig.portBCConstantValue[n] =
                                    baseAddr+n*0x4;
                            }
                        }
                    }/*00000004+n*0x4*/
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.dq[0].global.globalDQConfig.globalDequeueConfig = 0x00000000;
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.dq[0].global.globalDQConfig.BCForCutThrough = 0x00000208;

                }/*end of unit globalDQConfig */


                {/*start of unit flushTrig */
                    {/*00000400+n * 0x4*/
                        GT_U32    n;
                        for(n = 0 ; n <= numTxqDqPortsPerUnit_firstBlock ; n++) {
                            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.dq[0].global.flushTrig.portTxQFlushTrigger[n] =
                                0x00000400+n * 0x4;
                        }/* end of loop n */
                        {
                            GT_U32  baseAddr = 0x0000E400;
                            /* continue with n */
                            for(ii = 0 ; ii < numTxqDqPortsPerUnit_secondBlock ; ii++,n++)
                            {
                                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.dq[0].global.flushTrig.portTxQFlushTrigger[n] =
                                    baseAddr+n*0x4;
                            }
                        }
                    }/*00000400+n * 0x4*/

                }/*end of unit flushTrig */


                {/*start of unit ECCCntrs */
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.dq[0].global.ECCCntrs.DQIncECCErrorCntr = 0x00000920;
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.dq[0].global.ECCCntrs.DQClrECCErrorCntr = 0x00000924;

                }/*end of unit ECCCntrs */


                {/*start of unit dropCntrs */
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.dq[0].global.dropCntrs.egrMirroredDroppedCntr = 0x00000900;
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.dq[0].global.dropCntrs.STCDroppedCntr = 0x00000904;

                }/*end of unit dropCntrs */


            }/*end of unit global */


            {/*start of unit flowCtrlResponse */
                {/*0000c100+n*0x4*/
                    GT_U32    n;
                    for(n = 0 ; n <= 15 ; n++) {
                        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.dq[0].flowCtrlResponse.schedulerProfilePFCTimerToPriorityQueueMapEnable[n] =
                            0x0000c100+n*0x4;
                    }/* end of loop n */
                }/*0000c100+n*0x4*/
                {/*0000c180+0x40*t+0x4*p*/
                    GT_U32    p,t;
                    for(p = 0 ; p <= 15 ; p++) {
                        for(t = 0 ; t <= 7 ; t++) {
                            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.dq[0].flowCtrlResponse.schedulerProfilePriorityQueuePFCFactor[p][t] =
                                0x0000c180+0x40*t+0x4*p;
                        }/* end of loop t */
                    }/* end of loop p */
                }/*0000c180+0x40*t+0x4*p*/
                {/*0000c008+n*0x4*/
                    GT_U32    n;
                    for(n = 0 ; n <= 15 ; n++) {
                        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.dq[0].flowCtrlResponse.schedulerProfileLLFCXOFFValue[n] =
                            0x0000c008+n*0x4;
                    }/* end of loop n */
                }/*0000c008+n*0x4*/
                {/*0000c500+p*4*/
                    GT_U32    p;
                    GT_U32    nMax = isPipeDevice ? 19 : SMEM_CHT_IS_SIP5_20_GET(devObjPtr) ? 127 : 255;
                    GT_U32    offset = SMEM_CHT_IS_SIP5_20_GET(devObjPtr) ? 0x00011000 : 0x0000c500;
                    /* NOTE: in SIP5_20 implemented as table (memory) in the device */
                    for(p = 0 ; p <= nMax ; p++) {
                        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.dq[0].flowCtrlResponse.portToTxQPortMap[p] =
                            offset + p*4;
                    }/* end of loop p */
                }/*0000c500+p*4*/
                {/*0000c080+n*0x4*/
                    GT_U32    n;
                    for(n = 0 ; n <= 7 ; n++) {
                        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.dq[0].flowCtrlResponse.PFCTimerToPriorityQueueMap[n] =
                            0x0000c080+n*0x4;
                    }/* end of loop n */
                }/*0000c080+n*0x4*/
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.dq[0].flowCtrlResponse.flowCtrlResponseConfig = 0x0000c000;

            }/*end of unit flowCtrlResponse */


        }/*end of unit dq */


    }/*end of unit TXQ (sip5 but not sip6)*/


    if(SMEM_CHT_IS_SIP5_10_GET(devObjPtr))
    {
        if(isDxControlPipe)
        {/* start of unit TTI */
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TTI.oamLabelGlobalConfiguration0 = 0x0000001c;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TTI.oamLabelGlobalConfiguration1 = 0x0000003c;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TTI.TTIEngine.TTILookupIPExceptionCodes1 = 0x00000114;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TTI.TTIEngine.TTIKeySegmentMode0 = 0x00000144;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TTI.TTIEngine.TTIKeySegmentMode1 = 0x00000148;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TTI.sourceIDAssignment0 = 0x0000014c;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TTI.sourceIDAssignment1 = 0x00000150;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TTI.duplicationConfigs.duplicationConfigs0 = 0x00000154;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TTI.duplicationConfigs.duplicationConfigs1 = 0x00000158;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TTI.duplicationConfigs.MPLSMCDALow = 0x0000015c;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TTI.duplicationConfigs.MPLSMCDAHigh = 0x00000160;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TTI.duplicationConfigs.MPLSMCDALowMask = 0x00000164;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TTI.duplicationConfigs.MPLSMCDAHighMask = 0x00000168;

            {/*190+n*4*/
                GT_U32    n;
                for(n = 0 ; n <= 11 ; n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TTI.TTIKRuleMatchCntr[n] =
                        0x190+n*4;
                }/* end of loop n */
            }/*190+n*4*/

            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TTI.CFMEtherType =
                SMAIN_NOT_VALID_CNS;
        }

        if(isDxControlPipe)
        {/* start of unit PCL */
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PCL.pclBadAddressLatchReg = 0x00000010;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PCL.CFMEtherType = 0x00000018;
        }

        if(isDxControlPipe)
        {/* start of unit L2i */
            {/*12a0+n*0x4*/
                GT_U32    n;
                for(n = 0 ; n <= 7 ; n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->L2I.ingrPortsRateLimitConfig.CGSpeedGranularity[n] =
                        0x12a0+n*0x4;
                }/* end of loop n */
            }/*12a0+n*0x4*/
            {/*12c0+n*0x4*/
                GT_U32    n;
                for(n = 0 ; n <= 7 ; n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->L2I.ingrPortsRateLimitConfig.portSpeedOverrideEnable[n] =
                        0x12c0+n*0x4;
                }/* end of loop n */
            }/*12c0+n*0x4*/

            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->L2I.l2iBadAddressLatchReg = 0x00002120;

            {/*800+n*0x4*/
                GT_U32    n;
                for(n = 0 ; n <= 31 ; n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->L2I.ctrlTrafficToCPUConfig.IPv4McLinkLocalConfig[n] =
                        0x800+n*0x4;
                }/* end of loop n */
            }/*800+n*0x4*/
            {/*b00+n*0x4*/
                GT_U32    n;
                for(n = 0 ; n <= 31 ; n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->L2I.ctrlTrafficToCPUConfig.IPv6McLinkLocalConfig[n] =
                        0xb00+n*0x4;
                }/* end of loop n */
            }/*b00+n*0x4*/

            if(SMEM_CHT_IS_SIP6_30_GET(devObjPtr))
            {
                {/*hsrPrp*/
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->L2I.hsrPrp.hsrPrpGlobalConfig0  = 0x00002200;
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->L2I.hsrPrp.hsrPrpGlobalConfig1  = 0x00002204;
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->L2I.hsrPrp.hsrPrpGlobalConfig2  = 0x00002208;
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->L2I.hsrPrp.hsrPrpGlobalConfig3  = 0x0000220C;
                    {
                        GT_U32    n;
                        for(n = 0 ; n <= 9 ; n++) {
                            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->L2I.hsrPrp.hsrPrpSourcePortConfig0[n] = 0x00002210 + n * 0x8;
                            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->L2I.hsrPrp.hsrPrpSourcePortConfig1[n] = 0x00002214 + n * 0x8;
                        }
                    }
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->L2I.hsrPrp.hsrPrpPntCounter          = 0x000022A0;
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->L2I.hsrPrp.hsrPrpPntCounterThreshold = 0x000022A4;

                    {
                        GT_U32    n;
                        for(n = 0 ; n <= 3 ; n++) {
                            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->L2I.hsrPrp.hsrPrpPntDiscardPacketsLreACounter[n] = 0x000022C0 + n*0x4;
                            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->L2I.hsrPrp.hsrPrpPntDiscardPacketsLreBCounter[n] = 0x000022D0 + n*0x4;
                            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->L2I.hsrPrp.hsrPrpLreInstanceTable[n]             = 0x00002300 + n*0x4;
                        }
                    }

                    {
                        GT_U32    n;
                        for(n = 0 ; n <= 127 ; n++) {
                            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->L2I.hsrPrp.hsrPrpPntEntryPart0 [n] = 0x00002400 + n * 0xC;
                            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->L2I.hsrPrp.hsrPrpPntEntryPart1 [n] = 0x00002404 + n * 0xC;
                            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->L2I.hsrPrp.hsrPrpPntEntryPart2 [n] = 0x00002408 + n * 0xC;
                            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->L2I.hsrPrp.hsrPrpPntReady      [n] = 0x00002A00 + n * 0x4;
                            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->L2I.hsrPrp.hsrPrpPntValid      [n] = 0x00002C00 + n * 0x4;
                        }
                    }
                }/*hsrPrp;*/
            }
        }

        if(isDxControlPipe && !SMEM_CHT_IS_SIP6_GET(devObjPtr))
        {/* start of unit EGF */
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EGF_eft.MCFIFO.MCFIFOConfigs.MCFIFOArbiterWeights0 = 0x00002108;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EGF_eft.MCFIFO.MCFIFOConfigs.lossyDropAndPfcUcThreshold =
                SMAIN_NOT_VALID_CNS;

            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EGF_eft.counters.descEccCriticalDropCounter = 0x00004000;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EGF_eft.counters.descEccNonCriticalDropCounter = 0x00004004;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EGF_eft.counters.eftDescEcc1ErrorCntr = 0x00004008;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EGF_eft.miscellaneous.badAddressLatch = 0x00005000;
            {/*e00170+n*0x4*/
                GT_U32    n;
                for(n = 0 ; n <= 3 ; n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EGF_qag.distributor.loopbackEnableConfig[n] =
                        0xe00170+n*0x4;
                }/* end of loop n */
            }/*e00170+n*0x4*/
            {/*60203a0+0x4*n*/
                GT_U32    n;
                GT_U32    nMax = (shtNumPorts-1)/32;
                GT_U32    offset = SMEM_CHT_IS_SIP5_20_GET(devObjPtr) ? 0x6020800 : 0x60203a0;
                for(n = 0; n <= nMax ; n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EGF_sht.global.eportDropOnEportVid1Mismatch[n] =
                        offset+0x4*n;
                }/* end of loop n */
            }/*60203a0+0x4*n*/
            {/*6020400+0x4*n*/
                GT_U32    n;
                GT_U32    nMax = (16*shtNumPorts-1)/32;
                for(n = 0; n <= nMax ; n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EGF_sht.global.eportAssociatedVid1[n] =
                        0x6020400+0x4*n;
                }/* end of loop n */
            }/*6020400+0x4*n*/

            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EGF_sht.miscellaneous.badAddressLatch =
                SMEM_CHT_IS_SIP5_20_GET(devObjPtr) ? 0x6020900 : 0x6020600;
        }

        if(isDxControlPipe)
        {/* start of unit HA */
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->HA.haBadAddrLatchReg = 0x00000308;
            if(!SMEM_CHT_IS_SIP6_30_GET(devObjPtr))
            {
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->HA.NATConfig = 0x00000070;
            }
            else
            {
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->HA.NATConfig = 0x00000570;
            }
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->HA.NATExceptionDropCounter = 0x00000314;
        }

        if(isDxControlPipe)
        {/* start of unit EPCL */
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EPCL.EPCLBadAddrLatchReg = 0x00000018;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EPCL.oamRBbitConfigReg = 0x0000001c;
        }

        if(isDxControlPipe)
        {/* start of ERMRK */
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->ERMRK.ERMRKBadAddrLatch = 0x00000028;
        }

        if(SMAIN_NOT_VALID_CNS != UNIT_BASE_ADDR_GET_ALLOW_NON_EXIST_UNIT_MAC(devObjPtr,UNIT_TXQ_DQ))
        {/* start of unit TXQ  */
            { /*0000100c+n*0x4*/
                GT_U32    n;
                for(n = 0; n <= 1; n++)
                {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.dq[0].scheduler.schedulerConfig.highSpeedPorts[n] =
                        0x0000100c+n*0x4;
                }
            } /*0000100c+n*0x4*/

            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.dq[0].scheduler.portShaper.fastPortShaperThreshold =
                0x00001908;

            { /*b00+n*0x4*/
                GT_U32    n;
                GT_U32    nMax = SMEM_CHT_IS_SIP5_20_GET(devObjPtr) ? 12 : 73;
                for(n = 0; n <= nMax; n++)
                {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.dq[0].global.creditCounters.txDmaPortCreditCntr[n] =
                        0xb00+n*0x4;
                }
            } /*b00+n*0x4*/

            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.dq[0].global.interrupt.txQDQBadAddressLatch =
                0x00000690;
        }

        if(SMAIN_NOT_VALID_CNS != UNIT_BASE_ADDR_GET_ALLOW_NON_EXIST_UNIT_MAC(devObjPtr,UNIT_TXQ_LL))
        {
            if (!SMEM_CHT_IS_SIP5_20_GET(devObjPtr)) /* BC3 does not have these registers */
            {/*a1140+n * 0x4*/
                GT_U32    n;
                GT_U32    nMax = 15;
                GT_U32    offset1 = 0xa1100;
                GT_U32    offset2 = 0xa1140;
                for(n = 0 ; n <= nMax ; n++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.ll.global.LLECC.llECCControlAndStatusBank[n] =
                        offset1+n*0x4;
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.ll.global.LLECC.llECCErrorCounterBank[n] =
                        offset2+n*0x4;
                }/* end of loop n */
            }/*a1140+n * 0x4*/

            if(!SMEM_CHT_IS_SIP5_20_GET(devObjPtr))
            {
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.ll.global.interrupts.LLECCInterruptCause =
                    SMEM_CHT_IS_SIP5_20_GET(devObjPtr) ? 0x00112010 : 0x000A0510;
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.ll.global.interrupts.LLECCInterruptMask =
                    SMEM_CHT_IS_SIP5_20_GET(devObjPtr) ? 0x00112014 : 0x000A0514;
            }

            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.ll.global.interrupts.txqLLBadAddressLatch =
                SMEM_CHT_IS_SIP5_20_GET(devObjPtr) ? 0x00112028 : 0x000A0528;
        }

        if(SMAIN_NOT_VALID_CNS != UNIT_BASE_ADDR_GET_ALLOW_NON_EXIST_UNIT_MAC(devObjPtr,UNIT_CPFC))
        {
            {/*000000a0+0x4*p*/
                GT_U32    p;
                for(p = 0 ; p <= 0 ; p++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.pfc.PFCPortGroupFlowCtrlIndicationFIFOOverrunCntr[p] =
                        0x000000a0+0x4*p;
                }/* end of loop p */
            }/*000000a0+0x4*p*/
            {/*00000060+0x4*p*/
                GT_U32    p;
                for(p = 0 ; p <= 0 ; p++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.pfc.PFCPortGroupFlowCtrlIndicationFIFOOverrunEntry1[p] =
                        0x00000060+0x4*p;
                }/* end of loop p */
            }/*00000060+0x4*p*/

            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.pfc.PFCInterrupts.PFCPortGroupGlobalOverUnderflowInterruptCause = 0x00001370;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.pfc.PFCInterrupts.PFCPortGroupGlobalOverUnderflowInterruptMask = 0x00001390;
        }

        if(SMAIN_NOT_VALID_CNS != UNIT_BASE_ADDR_GET_ALLOW_NON_EXIST_UNIT_MAC(devObjPtr,UNIT_TXQ_QUEUE))
        {
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.queue.global.interrupt.txqQueueBadAddressLatch = 0x000909f0;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.queue.global.CPFCInterface.cpuPortCPFCCountConfiguration = 0x00090a90;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.queue.global.CPFCInterface.cpuPortsNumber = 0x00090a94;
        }

        if(!SMEM_CHT_IS_SIP6_GET(devObjPtr))
        {/*RXDMA*/
            GT_U32  nMax = isPipeDevice ? 16 : isSip5_25 ? 24 : 72;

            unitIndex = 0;

            {/*start of unit SIP_COMMON_MODULES */
                {/*start of unit pizzaArbiter */
                    {/*2a08+n*0x4*/
                        GT_U32    n;
                        for(n = 0 ; n <= 84 ; n++) {
                            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->rxDMA[unitIndex].SIP_COMMON_MODULES.pizzaArbiter.pizzaArbiterConfigReg[n] =
                                0x2a08+n*0x4;
                        }/* end of loop n */
                    }/*2a08+n*0x4*/
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->rxDMA[unitIndex].SIP_COMMON_MODULES.pizzaArbiter.pizzaArbiterCtrlReg = 0x00002a00;
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->rxDMA[unitIndex].SIP_COMMON_MODULES.pizzaArbiter.pizzaArbiterStatusReg = 0x00002a04;

                }/*end of unit pizzaArbiter */


            }/*end of unit SIP_COMMON_MODULES */

            {/*start of unit singleChannelDMAConfigs */
                {/*804+n*0x4*/
                    GT_U32    n;
                    for(n = 0 ; n <= nMax ; n++) {
                        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->rxDMA[unitIndex].singleChannelDMAConfigs.SCDMAConfig0[n] =
                            0x804+n*0x4;
                    }/* end of loop n */
                }/*804+n*0x4*/
                {/*950+n*0x4*/
                    GT_U32    n;
                    for(n = 0 ; n <= nMax ; n++) {
                        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->rxDMA[unitIndex].singleChannelDMAConfigs.SCDMAConfig1[n] =
                            0x950+n*0x4;
                    }/* end of loop n */
                }/*950+n*0x4*/
                {/*b00+n*0x4*/
                    GT_U32    n;
                    for(n = 0 ; n <= nMax ; n++) {
                        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->rxDMA[unitIndex].singleChannelDMAConfigs.SCDMACTPktIndentification[n] =
                            0xb00+n*0x4;
                    }/* end of loop n */
                }/*b00+n*0x4*/

            }/*end of unit singleChannelDMAConfigs */
            {/*start of unit interrupts */
                {/*120c+n*0x4*/
                    GT_U32    n;
                    for(n = 0 ; n <= nMax ; n++) {
                        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->rxDMA[unitIndex].interrupts.rxDMASCDMAInterruptMask[n] =
                            0x120c+n*0x4;
                    }/* end of loop n */
                }/*120c+n*0x4*/
                {/*e08+n*0x4*/
                    GT_U32    n;
                    for(n = 0 ; n <= nMax ; n++) {
                        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->rxDMA[unitIndex].interrupts.rxDMASCDMAInterruptCause[n] =
                            0xe08+n*0x4;
                    }/* end of loop n */
                }/*e08+n*0x4*/
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->rxDMA[unitIndex].interrupts.rxDMAInterrupt0Cause = 0x00000e00;
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->rxDMA[unitIndex].interrupts.rxDMAInterrupt0Mask = 0x00000e04;
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->rxDMA[unitIndex].interrupts.rxDMAInterruptSummary0Cause = 0x00001528;
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->rxDMA[unitIndex].interrupts.rxDMAInterruptSummary0Mask = 0x0000152c;
                if((nMax+1) >= (32-2))
                {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->rxDMA[unitIndex].interrupts.rxDMAInterruptSummary1Cause = 0x00001530;
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->rxDMA[unitIndex].interrupts.rxDMAInterruptSummary1Mask = 0x00001534;
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->rxDMA[unitIndex].interrupts.rxDMAInterruptSummary2Cause = 0x00001538;
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->rxDMA[unitIndex].interrupts.rxDMAInterruptSummary2Mask = 0x0000153c;
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->rxDMA[unitIndex].interrupts.rxDMAInterruptSummary3Cause = 0x00001540;
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->rxDMA[unitIndex].interrupts.rxDMAInterruptSummary3Mask = 0x00001544;
                }
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->rxDMA[unitIndex].interrupts.rxDMAInterruptSummaryCause = 0x00001548;
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->rxDMA[unitIndex].interrupts.rxDMAInterruptSummaryMask = 0x0000154c;
            }/*end of unit interrupts */

            {/*start of unit preIngrPrioritizationConfStatus */
                {/*000000c4+n * 0x4*/
                    GT_U32    n;
                    for(n = 0 ; n <= 3 ; n++) {
                        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->rxDMA[unitIndex].globalRxDMAConfigs.preIngrPrioritizationConfStatus.VLANUPPriorityArrayProfile[n] =
                            0x000000c4+n * 0x4;
                    }/* end of loop n */
                }/*000000c4+n * 0x4*/
                {/*000003a8+n*0x4*/
                    GT_U32    n;
                    for(n = 0 ; n <= 1 ; n++) {
                        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->rxDMA[unitIndex].globalRxDMAConfigs.preIngrPrioritizationConfStatus.MPLSEtherTypeConf[n] =
                            0x000003a8+n*0x4;
                    }/* end of loop n */
                }/*000003a8+n*0x4*/
                {/*00000450+n*0x4*/
                    GT_U32    n;
                    for(n = 0 ; n <= 3 ; n++) {
                        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->rxDMA[unitIndex].globalRxDMAConfigs.preIngrPrioritizationConfStatus.VIDConf[n] =
                            0x00000450+n*0x4;
                    }/* end of loop n */
                }/*00000450+n*0x4*/
                {/*000003c8+n*0x4*/
                    GT_U32    n;
                    for(n = 0 ; n <= 3 ; n++) {
                        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->rxDMA[unitIndex].globalRxDMAConfigs.preIngrPrioritizationConfStatus.genericEtherType[n] =
                            0x000003c8+n*0x4;
                    }/* end of loop n */
                }/*000003c8+n*0x4*/
                {/*000000e4+m*0x4 + n*0x20*/
                    GT_U32    n,m;
                    for(n = 0 ; n <= 3 ; n++) {
                        for(m = 0 ; m <= 7 ; m++) {
                            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->rxDMA[unitIndex].globalRxDMAConfigs.preIngrPrioritizationConfStatus.qoSDSAPriorityArrayProfileSegment[n][m] =
                                0x000000e4+m*0x4 + n*0x20;
                        }/* end of loop m */
                    }/* end of loop n */
                }/*000000e4+m*0x4 + n*0x20*/

                if(!isPipeDevice && !isSip5_20)
                {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->rxDMA[unitIndex].globalRxDMAConfigs.preIngrPrioritizationConfStatus.preingressPrioritizationEnable = 0x000000b4;
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->rxDMA[unitIndex].globalRxDMAConfigs.preIngrPrioritizationConfStatus.PIPPrioThresholds = 0x000000b0;
                }
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->rxDMA[unitIndex].globalRxDMAConfigs.preIngrPrioritizationConfStatus.PIPPriorityDropGlobalCntrsClear = 0x00000740;
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->rxDMA[unitIndex].globalRxDMAConfigs.preIngrPrioritizationConfStatus.PIPMedPriorityDropGlobalCntrMsb = 0x00000724;
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->rxDMA[unitIndex].globalRxDMAConfigs.preIngrPrioritizationConfStatus.PIPMedPriorityDropGlobalCntrLsb = 0x00000720;
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->rxDMA[unitIndex].globalRxDMAConfigs.preIngrPrioritizationConfStatus.PIPLowPriorityDropGlobalCntrMsb = 0x00000714;
                if(SMEM_CHT_IS_SIP5_20_GET(devObjPtr))
                {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->rxDMA[unitIndex].globalRxDMAConfigs.preIngrPrioritizationConfStatus.PIPVeryHighPriorityDropGlobalCntrLsb = 0x00000760;
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->rxDMA[unitIndex].globalRxDMAConfigs.preIngrPrioritizationConfStatus.PIPVeryHighPriorityDropGlobalCntrMsb = 0x00000764;
                }
                {/*00000398+n*0x4*/
                    GT_U32    n;
                    for(n = 0 ; n <= 3 ; n++) {
                        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->rxDMA[unitIndex].globalRxDMAConfigs.preIngrPrioritizationConfStatus.VLANEtherTypeConf[n] =
                            0x00000398+n*0x4;
                    }/* end of loop n */
                }/*00000398+n*0x4*/
                {/*00000408+n*0x4*/
                    GT_U32    n;
                    for(n = 0 ; n <= 3 ; n++) {
                        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->rxDMA[unitIndex].globalRxDMAConfigs.preIngrPrioritizationConfStatus.MACDA2MsbBytesConf[n] =
                            0x00000408+n*0x4;
                    }/* end of loop n */
                }/*00000408+n*0x4*/
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->rxDMA[unitIndex].globalRxDMAConfigs.preIngrPrioritizationConfStatus.PIPHighPriorityDropGlobalCntrLsb = 0x00000730;
                {/*00000174+n * 0x4*/
                    GT_U32    n;
                    for(n = 0 ; n <= 3 ; n++) {
                        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->rxDMA[unitIndex].globalRxDMAConfigs.preIngrPrioritizationConfStatus.MPLSEXPPriorityArrayProfile[n] =
                            0x00000174+n * 0x4;
                    }/* end of loop n */
                }/*00000174+n * 0x4*/
                {/*00000630+n * 0x4*/
                    GT_U32    n;
                    for(n = 0 ; n <= 3 ; n++) {
                        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->rxDMA[unitIndex].globalRxDMAConfigs.preIngrPrioritizationConfStatus.UPDSATagArrayProfile[n] =
                            0x00000630+n * 0x4;
                    }/* end of loop n */
                }/*00000630+n * 0x4*/
                {/*000002f0+n * 0x4*/
                    GT_U32    n;
                    for(n = 0 ; n <= 3 ; n++) {
                        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->rxDMA[unitIndex].globalRxDMAConfigs.preIngrPrioritizationConfStatus.MACDAPriorityArrayProfile[n] =
                            0x000002f0+n * 0x4;
                    }/* end of loop n */
                }/*000002f0+n * 0x4*/
                {/*00000428+n *0x4*/
                    GT_U32    n;
                    for(n = 0 ; n <= 3 ; n++) {
                        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->rxDMA[unitIndex].globalRxDMAConfigs.preIngrPrioritizationConfStatus.MACDAMask4LsbConf[n] =
                            0x00000428+n *0x4;
                    }/* end of loop n */
                }/*00000428+n *0x4*/
                {/*0000043c+n*0x4*/
                    GT_U32    n;
                    for(n = 0 ; n <= 3 ; n++) {
                        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->rxDMA[unitIndex].globalRxDMAConfigs.preIngrPrioritizationConfStatus.MACDAMask2MsbConf[n] =
                            0x0000043c+n*0x4;
                    }/* end of loop n */
                }/*0000043c+n*0x4*/
                {/*000003e8+n*0x4*/
                    GT_U32    n;
                    for(n = 0 ; n <= 3 ; n++) {
                        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->rxDMA[unitIndex].globalRxDMAConfigs.preIngrPrioritizationConfStatus.MACDA4LsbBytesConf[n] =
                            0x000003e8+n*0x4;
                    }/* end of loop n */
                }/*000003e8+n*0x4*/
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->rxDMA[unitIndex].globalRxDMAConfigs.preIngrPrioritizationConfStatus.PIPLowPriorityDropGlobalCntrLSb = 0x00000710;
                {/*000004d0+m*0x4 + n*0x40*/
                    GT_U32    n,m;
                    for(n = 0 ; n <= 3 ; n++) {
                        for(m = 0 ; m <= 15 ; m++) {
                            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->rxDMA[unitIndex].globalRxDMAConfigs.preIngrPrioritizationConfStatus.IPv6TCPriorityArrayProfileSegment[n][m] =
                                0x000004d0+m*0x4 + n*0x40;
                        }/* end of loop m */
                    }/* end of loop n */
                }/*000004d0+m*0x4 + n*0x40*/
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->rxDMA[unitIndex].globalRxDMAConfigs.preIngrPrioritizationConfStatus.IPv6EtherTypeConf = 0x00000374;
                {/*00000198+m*0x4 + n*0x40*/
                    GT_U32    n,m;
                    for(n = 0 ; n <= 3 ; n++) {
                        for(m = 0 ; m <= 15 ; m++) {
                            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->rxDMA[unitIndex].globalRxDMAConfigs.preIngrPrioritizationConfStatus.IPv4ToSPriorityArrayProfileSegment[n][m] =
                                0x00000198+m*0x4 + n*0x40;
                        }/* end of loop m */
                    }/* end of loop n */
                }/*00000198+m*0x4 + n*0x40*/
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->rxDMA[unitIndex].globalRxDMAConfigs.preIngrPrioritizationConfStatus.IPv4EtherTypeConf = 0x00000350;
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->rxDMA[unitIndex].globalRxDMAConfigs.preIngrPrioritizationConfStatus.PIPHighPriorityDropGlobalCntrMsb = 0x00000734;

                if(SMEM_CHT_IS_SIP5_15_GET(devObjPtr))
                {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->rxDMA[unitIndex].globalRxDMAConfigs.preIngrPrioritizationConfStatus.PIPDropGlobalCounterTrigger = 0x00000738;
                }

                {/*00000320+n*0x4*/
                    GT_U32    n;
                    for(n = 0 ; n <= 3 ; n++) {
                        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->rxDMA[unitIndex].globalRxDMAConfigs.preIngrPrioritizationConfStatus.etherTypePriorityArrayProfile[n] =
                            0x00000320+n*0x4;
                    }/* end of loop n */
                }/*00000320+n*0x4*/

            }/*end of unit preIngrPrioritizationConfStatus */

            {
                /* manually added : rxDMA.singleChannelDMAPip */
                GT_U32  nMax = isPipeDevice ? 17 : isSip5_25 ? 25 : 73;

                LOOP_N_REG(SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->rxDMA[unitIndex].singleChannelDMAPip.SCDMA_n_PIP_Config,
                    0x00003A00 ,  nMax/*numN*/, 0x4/*stepN*/);

                LOOP_N_REG(SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->rxDMA[unitIndex].singleChannelDMAPip.SCDMA_n_Drop_Pkt_Counter,
                    0x00003500 , nMax /*numN*/, 0x4/*stepN*/);
            }
        }
    }

    /* next are 'TM glue units' that exists in devices that hold TM unit */
    if(devObjPtr->supportTrafficManager)
    {
        {/*TMDROP*/
            {/*0000000+k*0x04*/
                GT_U32    k;
                for (k = 0 ; k <= 7 ; k++)
                {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TMDROP.portsInterface[k] =
                        0x0000000 + (k * 0x04);
                }/* end of loop k */
            }/*0000000+k*0x04*/
            {/*0000020+k*0x04*/
                GT_U32    k;
                for (k = 0 ; k <= 1 ; k++)
                {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TMDROP.TMDROPTMTCMap8To87[k] =
                        0x0000020 + (k * 0x04);
                }/* end of loop k */
            }/*0000020+k*0x04*/
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TMDROP.globalConfigs = 0x00000028;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TMDROP.TMDROPInterruptCause = 0x00000030;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TMDROP.TMDROPInterruptMask = 0x00000034;
        }/*TMDROP*/

        {/*TMQMAP*/
            {/*0000004+k*0x04*/
                GT_U32    k;
                for(k = 0; (k <= 5); k++) {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TMQMAP.TC2TCMap8To16[k] =
                        0x0000004 + (k * 0x04);
                }/* end of loop k */
            }/*0000004+k*0x04*/

            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TMQMAP.TMQMAPGlobalConfig   = 0x00000000;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TMQMAP.TMQMAPInterruptCause = 0x00000020;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TMQMAP.TMQMAPInterruptMask  = 0x00000024;
        }/*TMQMAP*/

        {/*TM_INGR_GLUE*/
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TM_INGR_GLUE.globalConfig             = 0x00000000;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TM_INGR_GLUE.debug                    = 0x00000004;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TM_INGR_GLUE.TMIngrGlueInterruptCause = 0x00000008;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TM_INGR_GLUE.TMIngrGlueInterruptMask  = 0x0000000C;
        }/*TM_INGR_GLUE*/


        {/*TM_EGR_GLUE*/
            {/*00000600+4*c*/
                GT_U32    c;
                for  (c = 0; (c <= 99); c++)
                {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TM_EGR_GLUE.statisticalAgingTimeCntrMsb[c] =
                        0x00000600 + (4 * c);
                }/* end of loop c */
            }/*00000600+4*c*/
            {/*00000400+c*4*/
                GT_U32    c;
                for  (c = 0; (c <= 99); c++)
                {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TM_EGR_GLUE.statisticalAgingTimeCntrLsb[c] =
                        0x00000400 + (4 * c);
                }/* end of loop c */
            }/*00000400+c*4*/
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TM_EGR_GLUE.TMEgrGlueInterruptMask = 0x00000028;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TM_EGR_GLUE.TMEgrGlueInterruptCause = 0x00000024;
            {/*00001400+4*c*/
                GT_U32    c;
                for  (c = 0; (c <= 99); c++)
                {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TM_EGR_GLUE.statisticalMinAgeTime[c] =
                        0x00001400 + (4 * c);
                }/* end of loop c */
            }/*00001400+4*c*/
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TM_EGR_GLUE.statisticalMinAgeTimeShadow = 0x0000200c;
            {/*00001200+4*c*/
                GT_U32    c;
                for  (c = 0; (c <= 99); c++)
                {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TM_EGR_GLUE.statisticalMaxAgeTime[c] =
                        0x00001200 + (4 * c);
                }/* end of loop c */
            }/*00001200+4*c*/
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TM_EGR_GLUE.statisticalMaxAgeTimeShadow = 0x00002008;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TM_EGR_GLUE.statisticalAgingTimeCntrsWraparoundInterruptMask3 = 0x0000011c;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TM_EGR_GLUE.statisticalAgingTimeCntrsWraparoundInterruptMask2 = 0x00000118;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TM_EGR_GLUE.statisticalAgingTimeCntrsWraparoundInterruptMask1 = 0x00000114;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TM_EGR_GLUE.statisticalAgingTimeCntrsWraparoundInterruptMask0 = 0x00000110;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TM_EGR_GLUE.statisticalAgingTimeCntrsWraparoundInterruptCause3 = 0x0000010c;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TM_EGR_GLUE.statisticalAgingTimeCntrsWraparoundInterruptCause2 = 0x00000108;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TM_EGR_GLUE.statisticalAgingTimeCntrsWraparoundInterruptCause1 = 0x00000104;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TM_EGR_GLUE.statisticalAgingTimeCntrsWraparoundInterruptCause0 = 0x00000100;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TM_EGR_GLUE.statisticalAgingTimeCntrsMsbShadow = 0x00002004;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TM_EGR_GLUE.statisticalAgingTimeCntrsLsbShadow = 0x00002000;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TM_EGR_GLUE.TMEgrGlueSummaryCause = 0x0000002c;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TM_EGR_GLUE.TMEgrGlueSummaryMask = 0x00000030;
            {/*00000200+c*4*/
                GT_U32    c;
                for  (c = 0; (c <= 99); c++)
                {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TM_EGR_GLUE.statisticalAgingQueueID[c] =
                        0x00000200 + (4 * c);
                }/* end of loop c */
            }/*00000200+c*4*/
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TM_EGR_GLUE.statisticalAgingPktCntrsWraparoundInterruptMask3 = 0x00000054;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TM_EGR_GLUE.statisticalAgingPktCntrsWraparoundInterruptMask2 = 0x00000050;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TM_EGR_GLUE.statisticalAgingPktCntrsWraparoundInterruptMask1 = 0x0000004c;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TM_EGR_GLUE.statisticalAgingPktCntrsWraparoundInterruptMask0 = 0x00000048;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TM_EGR_GLUE.statisticalAgingPktCntrsWraparoundInterruptCause3 = 0x00000044;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TM_EGR_GLUE.statisticalAgingPktCntrsWraparoundInterruptCause2 = 0x00000040;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TM_EGR_GLUE.statisticalAgingPktCntrsWraparoundInterruptCause1 = 0x0000003c;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TM_EGR_GLUE.statisticalAgingPktCntrsWraparoundInterruptCause0 = 0x00000038;
            {/*00000800+c*4*/
                GT_U32    c;
                for  (c = 0; (c <= 99); c++)
                {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TM_EGR_GLUE.statisticalAgingPktCntr[c] =
                        0x00000800 + (4 * c);
                }/* end of loop c */
            }/*00000800+c*4*/
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TM_EGR_GLUE.statisticalAgingOctetCntrsWraparoundInterruptMask3 = 0x0000013c;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TM_EGR_GLUE.statisticalAgingOctetCntrsWraparoundInterruptMask2 = 0x00000138;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TM_EGR_GLUE.statisticalAgingOctetCntrsWraparoundInterruptMask1 = 0x00000134;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TM_EGR_GLUE.statisticalAgingOctetCntrsWraparoundInterruptMask0 = 0x00000130;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TM_EGR_GLUE.statisticalAgingOctetCntrsWraparoundInterruptCause3 = 0x0000012c;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TM_EGR_GLUE.statisticalAgingOctetCntrsWraparoundInterruptCause2 = 0x00000128;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TM_EGR_GLUE.statisticalAgingOctetCntrsWraparoundInterruptCause1 = 0x00000124;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TM_EGR_GLUE.statisticalAgingOctetCntrsWraparoundInterruptCause0 = 0x00000120;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TM_EGR_GLUE.statisticalAgingOctetCntrsMsbShadow = 0x00002014;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TM_EGR_GLUE.statisticalAgingOctetCntrsLsbShadow = 0x00002010;
            {/*00001000+c*4*/
                GT_U32    c;
                for  (c = 0; (c <= 99); c++)
                {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TM_EGR_GLUE.statisticalAgingOctetCntrMsb[c] =
                        0x00001000 + (4 * c);
                }/* end of loop c */
            }/*00001000+c*4*/
            {/*00000c00+c*4*/
                GT_U32    c;
                for  (c = 0; (c <= 99); c++)
                {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TM_EGR_GLUE.statisticalAgingOctetCntrLsb[c] =
                        0x00000c00 + (4 * c);
                }/* end of loop c */
            }/*00000c00+c*4*/
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TM_EGR_GLUE.statisticalAgingAgedOutPktCntrsWraparoundInterruptMask3 = 0x0000015c;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TM_EGR_GLUE.statisticalAgingAgedOutPktCntrsWraparoundInterruptMask2 = 0x00000158;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TM_EGR_GLUE.statisticalAgingAgedOutPktCntrsWraparoundInterruptMask1 = 0x00000154;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TM_EGR_GLUE.statisticalAgingAgedOutPktCntrsWraparoundInterruptMask0 = 0x00000150;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TM_EGR_GLUE.statisticalAgingAgedOutPktCntrsWraparoundInterruptCause3 = 0x0000014c;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TM_EGR_GLUE.statisticalAgingAgedOutPktCntrsWraparoundInterruptCause2 = 0x00000148;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TM_EGR_GLUE.statisticalAgingAgedOutPktCntrsWraparoundInterruptCause1 = 0x00000144;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TM_EGR_GLUE.statisticalAgingAgedOutPktCntrsWraparoundInterruptCause0 = 0x00000140;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TM_EGR_GLUE.statisticalAgingAgedOutPktCntrsShadow = 0x00002018;
            {/*00001600+c*4*/
                GT_U32    c;
                for  (c = 0; (c <= 99); c++)
                {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TM_EGR_GLUE.statisticalAgingAgedOutPktCntr[c] =
                        0x00001600 + (4 * c);
                }/* end of loop c */
            }/*00001600+c*4*/
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TM_EGR_GLUE.queueTimerConfig = 0x00000064;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TM_EGR_GLUE.queueTimer = 0x00000060;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TM_EGR_GLUE.portInterface2 = 0x00000078;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TM_EGR_GLUE.portInterface1 = 0x00000074;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TM_EGR_GLUE.portInterface0 = 0x00000070;
            {/*00001a00+c*4*/
                GT_U32    c;
                for  (c = 0; (c <= 191); c++)
                {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TM_EGR_GLUE.perPortAgedOutPktCntr[c] =
                        0x00001a00 + (4 * c);
                }/* end of loop c */
            }/*00001a00+c*4*/
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TM_EGR_GLUE.perPortAgedOutPktCntrsWraparoundInterruptMask6 = 0x00002058;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TM_EGR_GLUE.perPortAgedOutPktCntrsWraparoundInterruptMask5 = 0x00002054;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TM_EGR_GLUE.perPortAgedOutPktCntrsWraparoundInterruptMask4 = 0x00002050;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TM_EGR_GLUE.perPortAgedOutPktCntrsWraparoundInterruptMask3 = 0x0000204c;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TM_EGR_GLUE.perPortAgedOutPktCntrsWraparoundInterruptMask2 = 0x00002048;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TM_EGR_GLUE.perPortAgedOutPktCntrsWraparoundInterruptMask1 = 0x00002044;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TM_EGR_GLUE.perPortAgedOutPktCntrsWraparoundInterruptMask0 = 0x00002040;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TM_EGR_GLUE.perPortAgedOutPktCntrsWraparoundInterruptCause6 = 0x00002038;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TM_EGR_GLUE.perPortAgedOutPktCntrsWraparoundInterruptCause5 = 0x00002034;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TM_EGR_GLUE.perPortAgedOutPktCntrsWraparoundInterruptCause4 = 0x00002030;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TM_EGR_GLUE.perPortAgedOutPktCntrsWraparoundInterruptCause3 = 0x0000202c;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TM_EGR_GLUE.perPortAgedOutPktCntrsWraparoundInterruptCause2 = 0x00002028;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TM_EGR_GLUE.perPortAgedOutPktCntrsWraparoundInterruptCause1 = 0x00002024;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TM_EGR_GLUE.perPortAgedOutPktCntrsWraparoundInterruptCause0 = 0x00002020;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TM_EGR_GLUE.globalConfig = 0x00000000;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TM_EGR_GLUE.ECCStatus = 0x0000000c;
            {/*00002100+4*c*/
                GT_U32    c;
                for  (c = 0; (c <= 191); c++)
                {
                    SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TM_EGR_GLUE.dropPortAllOutgoingPkts[c] =
                        0x00002100 + (4 * c);
                }/* end of loop c */
            }/*00002100+4*c*/
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TM_EGR_GLUE.dropAgedPktCntrConfig = 0x00000004;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TM_EGR_GLUE.dropAgedPktCntr = 0x00000008;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TM_EGR_GLUE.debug = 0x00000010;
        }/*TM_EGR_GLUE*/
    }

    if(SMAIN_NOT_VALID_CNS != UNIT_BASE_ADDR_GET_ALLOW_NON_EXIST_UNIT_MAC(devObjPtr,UNIT_BM))
    {/*start of BM unit*/
        {/*start of unit BMInterrupts*/
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->BM.BMInterrupts.generalInterruptCause1 = 0x00000300;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->BM.BMInterrupts.generalInterruptMask1 = 0x00000304;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->BM.BMInterrupts.summaryInterruptCause = 0x00000310;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->BM.BMInterrupts.summaryInterruptMask = 0x00000314;
        }
    }/*end of BM unit*/

    if(!SMEM_CHT_IS_SIP6_GET(devObjPtr))
    {/*start of TXDMA unit*/
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXDMA[0].interrupts.txDMAInterruptCause = 0x00002038;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXDMA[0].interrupts.txDMAInterruptMask = 0x0000203C;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXDMA[0].interrupts.txDMAInterruptSummary = 0x00002040;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXDMA[0].interrupts.txDMAInterruptSummaryMask = 0x00002044;

        if(devObjPtr->supportCutThroughFastToSlow)
        {
            {/*start of TxDMAGlobalConfigurations*/
                {/*0x00000020 + i*0x4: where i (0-127) represents entry*/
                    GT_U32    i;
                    GT_U32  iiMax = isPipeDevice ? 7 : 127;

                    for  (i = 0; (i <= iiMax); i++)
                    {
                        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXDMA[0].TxDMAGlobalConfigurations.localDevSrcPortToDmaNumberMap[i] =
                            0x00000020 + (4 * i);
                    }/* end of loop c */
                }/*0x00000020+4*i*/

            }/*end of TxDMAGlobalConfigurations*/
        }

    }/*end of TXDMA unit*/

    if(!SMEM_CHT_IS_SIP6_GET(devObjPtr))
    {/*start of TXFIFO unit*/
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXFIFO[0].interrupts.txFIFOInterruptCause = 0x00000400;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXFIFO[0].interrupts.txFIFOInterruptMask = 0x00000404;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXFIFO[0].interrupts.txFIFOInterruptSummary = 0x00000440;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXFIFO[0].interrupts.txFIFOInterruptSummaryMask = 0x00000444;
    }/*end of TXFIFO unit*/

    if(SMAIN_NOT_VALID_CNS != UNIT_BASE_ADDR_GET_ALLOW_NON_EXIST_UNIT_MAC(devObjPtr,UNIT_MPPM))
    {/*start of MPPM unit*/
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->MPPM.ECC.interrupts.EccErrorInterruptCause = 0x00002000;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->MPPM.ECC.interrupts.EccErrorInterruptMask = 0x00002004;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->MPPM.interrupts.MPPMInterruptsSummary = 0x000021B0;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->MPPM.interrupts.MPPMInterruptsSummaryMask = 0x000021B4;

        if (SMEM_CHT_IS_SIP5_15_GET(devObjPtr))
        {
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->MPPM.interrupts.MPPMGeneralInterruptCause = 0x000021C0;
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->MPPM.interrupts.MPPMGeneralInterruptMask  = 0x000021C4;
        }

    }/*end of MPPM unit*/

    if(devObjPtr->supportTrafficManager)
    {/*start of ETH unit*/
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->ETH.TXFIFO.interrupts.EthTxFIFOInterruptCause = 0x00000400;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->ETH.TXFIFO.interrupts.EthTxFIFOInterruptMask = 0x00000404;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->ETH.TXFIFO.interrupts.EthTxFIFOInterruptsSummary = 0x00000440;
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->ETH.TXFIFO.interrupts.EthTxFIFOInterruptsSummaryMask = 0x00000444;
    }/*end of ETH unit*/


    if(devObjPtr->multiDataPath.supportMultiDataPath)
    {
        GT_U32  ii,jj,index;
        GT_U32  numOfPipes = devObjPtr->numOfPipes ? devObjPtr->numOfPipes : 1;

        index = 0;
        for(jj = 0 ; jj < numOfPipes; jj++)
        {
            if(SMEM_CHT_IS_SIP6_GET(devObjPtr) && jj >= 1)
            {
                if(SMAIN_NOT_VALID_CNS != UNIT_BASE_ADDR_GET_ALLOW_NON_EXIST_UNIT_MAC(devObjPtr,UNIT_IA))
                {
                    DUPLICATE_PIPE_UNIT_ADDR_FROM_PIPE0_MAC(SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->ingressAggregator , jj);
                }
            }

            /* copy same offsets from rxDMA 0 to rxDMA x */
            for(ii = 0 ; ii < devObjPtr->multiDataPath.maxDp ; ii++,index++)
            {
                if(index == 0)
                {
                    continue;
                }

                /* NOTE: next copy of units is skipping the 'startUnitInfo' that
                   was already set by previous logic , and should not be run over

                   this is why using '&kkk.startUnitInfo + 1' and
                    '- sizeof(SMEM_CHT_PP_REGS_UNIT_START_INFO_STC)'
                */

                DUPLICATE_DP_UNIT_ADDR_FROM_DP0_MAC(SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->rxDMA , index);

                if(SMEM_CHT_IS_SIP6_GET(devObjPtr))
                {
                    /* sip6_rxDMA - sip6 hold extra  rxDMA registers */
                    DUPLICATE_DP_UNIT_ADDR_FROM_DP0_MAC(SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_rxDMA   , index);

                    DUPLICATE_DP_UNIT_ADDR_FROM_DP0_MAC(SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->SIP6_TXQ_SDQ , index);

                    DUPLICATE_DP_UNIT_ADDR_FROM_DP0_MAC(SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->SIP6_TXQ_QFC , index);

                    DUPLICATE_DP_UNIT_ADDR_FROM_DP0_MAC(SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_txDMA   , index);

                    DUPLICATE_DP_UNIT_ADDR_FROM_DP0_MAC(SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_txFIFO  , index);

                    if(devObjPtr->numOfRavens)
                    {
                        DUPLICATE_DP_UNIT_ADDR_FROM_DP0_MAC(SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->eagle_d2d    , index);
                        /* one raven include 2 dp*/
                        if ((index%2) == 0)
                        {
                            DUPLICATE_DP_UNIT_ADDR_FROM_DP0_MAC(SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->raven_d2d_pair, (index / 2));
                            DUPLICATE_DP_UNIT_ADDR_FROM_DP0_MAC(SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->MG_RAVEN      , (index / 2));
                        }
                    }
                }
                else
                {
                    DUPLICATE_DP_UNIT_ADDR_FROM_DP0_MAC(SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXFIFO , index);
                    DUPLICATE_DP_UNIT_ADDR_FROM_DP0_MAC(SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXDMA  , index);
                }

                if(ii < devObjPtr->multiDataPath.numTxqDq)
                {
                    DUPLICATE_DP_UNIT_ADDR_FROM_DP0_MAC(SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.dq  , index);
                }
            }
        }
    }
    else
    if(devObjPtr->multiDataPath.numTxqDq > 1)
    {
        GT_U32  index;
        for(index = 1 ; index < devObjPtr->multiDataPath.numTxqDq ; index++)
        {
            memcpy(&SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.dq[index],
                   &SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.dq[0],
                   sizeof(SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TXQ.dq[0]));
        }
    }

    {
        GT_U32  ii;

        for(ii = 0 ; ii < devObjPtr->portsNumber ; ii++)
        {
            if(devObjPtr->portsArr[ii].state < SKERNEL_PORT_STATE_CG_100G_E)
            {
                continue;
            }

            if(devObjPtr->numOfPipes)
            {
                smemConvertGlobalPortToCurrentPipeId(devObjPtr,ii,NULL);
            }

            if (devObjPtr->portsArr[ii].state == SKERNEL_PORT_STATE_CG_100G_E)
            {
                /* relevant only to 100G ports */
                gopCgInit(devObjPtr,ii);
            }


            /*
                NOTE: sip6 that not hold Ravens will initial the MTI MACs by itself
            */
            if(devObjPtr->numOfRavens)/* actually Falcon only */ /*(Hawk using : regAddr_ptp , regAddr_MacMib ...)*/
            {
                /* Raven MTI registers */
                raven_gopMtiInit(devObjPtr,ii);
            }
        }
        if(devObjPtr->numOfPipes)
        {
            /* restore */
            smemConvertGlobalPortToCurrentPipeId(devObjPtr,0,NULL);
        }
    }
}

/**
* @internal smemDfxServerRegsInfoSet function
* @endinternal
*
* @brief   Init memory module for DFX server DB devices.
*
* @param[in] devObjPtr                - pointer to device object.
*/
void smemDfxServerRegsInfoSet
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr
)
{
    GT_U32  isSip6    = SMEM_CHT_IS_SIP6_GET(devObjPtr);

    {/*start of unit resetAndInitCtrller */
        if (isSip6 == GT_FALSE)
        {/*start of unit XSB_XBAR */
            {/*start of unit DRAMCtrllerRegsDDR3 */
                {/*start of unit dunitRegs */
                    {/*start of unit SDRAMScratchPad */
                        {/*start of unit D_Domain_Regs */
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR3.dunitRegs.SDRAMScratchPad.D_Domain_Regs.scratchPad0 = 0x00001504;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR3.dunitRegs.SDRAMScratchPad.D_Domain_Regs.scratchPad1 = 0x0000150c;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR3.dunitRegs.SDRAMScratchPad.D_Domain_Regs.scratchPad2 = 0x00001514;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR3.dunitRegs.SDRAMScratchPad.D_Domain_Regs.scratchPad3 = 0x0000151c;

                        }/*end of unit D_Domain_Regs */


                    }/*end of unit SDRAMScratchPad */


                    {/*start of unit SDRAMDataProtectionAndErrorReport */
                        {/*start of unit R_Domain_Regs */
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR3.dunitRegs.SDRAMDataProtectionAndErrorReport.R_Domain_Regs.SDRAMErrorCtrl = 0x00001454;

                        }/*end of unit R_Domain_Regs */


                        {/*start of unit D_Domain_Regs */
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR3.dunitRegs.SDRAMDataProtectionAndErrorReport.D_Domain_Regs.DDRCtrllerErrorInterruptCause = 0x000014d0;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR3.dunitRegs.SDRAMDataProtectionAndErrorReport.D_Domain_Regs.DDRCtrllerErrorInterruptMask = 0x000014d4;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR3.dunitRegs.SDRAMDataProtectionAndErrorReport.D_Domain_Regs.DDRCtrllerMsgInterruptCause = 0x000014d8;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR3.dunitRegs.SDRAMDataProtectionAndErrorReport.D_Domain_Regs.DDRCtrllerMsgInterruptMask = 0x000014dc;

                        }/*end of unit D_Domain_Regs */


                    }/*end of unit SDRAMDataProtectionAndErrorReport */


                    {/*start of unit SDRAMCtrlRegs */
                        {/*start of unit T_Domain_Regs */
                            {/*start of unit newTrainingUnit */
                                {/*00010b8+n*0x4*/
                                    GT_U32    n;
                                    for(n = 0 ; n <= 1 ; n++) {
                                        SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR3.dunitRegs.SDRAMCtrlRegs.T_Domain_Regs.newTrainingUnit.currentObjectValueReg[n] =
                                            0x00010b8+n*0x4;
                                    }/* end of loop n */
                                }/*00010b8+n*0x4*/
                                {/*00010c0+n*0x4*/
                                    GT_U32    n;
                                    for(n = 0 ; n <= 1 ; n++) {
                                        SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR3.dunitRegs.SDRAMCtrlRegs.T_Domain_Regs.newTrainingUnit.calibratedObjectPRFAReg[n] =
                                            0x00010c0+n*0x4;
                                    }/* end of loop n */
                                }/*00010c0+n*0x4*/
                                {/*0001830+0x90+(n-40)*0x4+m*0x100 and 0001830+0x4d0*(n-79)+m*0*/
                                    GT_U32    n,m;
                                    for(n = 40 ; n <= 43 ; n++) {
                                        for(m = 0 ; m <= 4 ; m++) {
                                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR3.dunitRegs.SDRAMCtrlRegs.T_Domain_Regs.newTrainingUnit.resultCtrlReg[n - 40][m] =
                                                0x0001830+0x90+(n-40)*0x4+m*0x100;
                                        }/* end of loop m */
                                    }/* end of loop n */
                                    for(n = 80 ; n <= 80 ; n++) {
                                        for(m = 0 ; m <= 0 ; m++) {
                                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR3.dunitRegs.SDRAMCtrlRegs.T_Domain_Regs.newTrainingUnit.resultCtrlReg[(n - 79) + 3][m] =
                                                0x0001830+0x4d0*(n-79)/*+m*0*/;
                                        }/* end of loop m */
                                    }/* end of loop n */
                                }/*0001830+0x90+(n-40)*0x4+m*0x100 and 0001830+0x4d0*(n-79)+m*0*/
                                {/*0001130+n*0x4 and 0001130+0x1c0+(n-28)*0x4*/
                                    GT_U32    n;
                                    for(n = 0 ; n <= 3 ; n++) {
                                        SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR3.dunitRegs.SDRAMCtrlRegs.T_Domain_Regs.newTrainingUnit.PUPTestSequenceRegs[n] =
                                            0x0001130+n*0x4;
                                    }/* end of loop n */
                                    for(n = 28 ; n <= 31 ; n++) {
                                        SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR3.dunitRegs.SDRAMCtrlRegs.T_Domain_Regs.newTrainingUnit.PUPTestSequenceRegs[(n - 28) + 4] =
                                            0x0001130+0x1c0+(n-28)*0x4;
                                    }/* end of loop n */
                                }/*0001130+n*0x4 and 0001130+0x1c0+(n-28)*0x4*/
                                SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR3.dunitRegs.SDRAMCtrlRegs.T_Domain_Regs.newTrainingUnit.PUPTestCtrl = 0x000010f0;
                                {/*00010b0+n*0x4*/
                                    GT_U32    n;
                                    for(n = 0 ; n <= 1 ; n++) {
                                        SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR3.dunitRegs.SDRAMCtrlRegs.T_Domain_Regs.newTrainingUnit.objectOpCodeReg1[n] =
                                            0x00010b0+n*0x4;
                                    }/* end of loop n */
                                }/*00010b0+n*0x4*/
                                {/*0001038+n*0x4*/
                                    GT_U32    n;
                                    for(n = 0 ; n <= 1 ; n++) {
                                        SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR3.dunitRegs.SDRAMCtrlRegs.T_Domain_Regs.newTrainingUnit.objectOpCodeReg0[n] =
                                            0x0001038+n*0x4;
                                    }/* end of loop n */
                                }/*0001038+n*0x4*/
                                SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR3.dunitRegs.SDRAMCtrlRegs.T_Domain_Regs.newTrainingUnit.maxWindowSizeObjectValueReg = 0x000010f4;
                                SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR3.dunitRegs.SDRAMCtrlRegs.T_Domain_Regs.newTrainingUnit.globalCtrlAndStatusReg = 0x00001030;
                                SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR3.dunitRegs.SDRAMCtrlRegs.T_Domain_Regs.newTrainingUnit.generalPhaseOpcodeReg = 0x00001034;
                                SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR3.dunitRegs.SDRAMCtrlRegs.T_Domain_Regs.newTrainingUnit.writeLevelingDoneCntrReference = 0x000010f8;
                                SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR3.dunitRegs.SDRAMCtrlRegs.T_Domain_Regs.newTrainingUnit.writeReadTestCtrl = 0x000010fc;

                            }/*end of unit newTrainingUnit */


                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR3.dunitRegs.SDRAMCtrlRegs.T_Domain_Regs.SDRAMInterfaceMbusCtrlLow = 0x00001430;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR3.dunitRegs.SDRAMCtrlRegs.T_Domain_Regs.SDRAMInterfaceMbusCtrlHigh = 0x00001434;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR3.dunitRegs.SDRAMCtrlRegs.T_Domain_Regs.SDRAMInterfaceMbusTimeout = 0x00001438;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR3.dunitRegs.SDRAMCtrlRegs.T_Domain_Regs.DRAMCtrllerMiscellaneous = 0x000014b0;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR3.dunitRegs.SDRAMCtrlRegs.T_Domain_Regs.DRAMAddrAndCtrlDrivingStrengthLow = 0x000014c0;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR3.dunitRegs.SDRAMCtrlRegs.T_Domain_Regs.DRAMDataAndDQSDrivingStrengthLow = 0x000014c4;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR3.dunitRegs.SDRAMCtrlRegs.T_Domain_Regs.DRAMVerticalCalibrationMachineCtrl = 0x000014c8;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR3.dunitRegs.SDRAMCtrlRegs.T_Domain_Regs.DRAMMainPadsCalibrationMachineCtrl = 0x000014cc;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR3.dunitRegs.SDRAMCtrlRegs.T_Domain_Regs.SDRAMInterfaceMbusCtrl2Low = 0x000014f4;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR3.dunitRegs.SDRAMCtrlRegs.T_Domain_Regs.SDRAMInterfaceMbusCtrl2High = 0x000014f8;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR3.dunitRegs.SDRAMCtrlRegs.T_Domain_Regs.readDataSampleDelays = 0x00001538;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR3.dunitRegs.SDRAMCtrlRegs.T_Domain_Regs.readDataReadyDelays = 0x0000153c;

                        }/*end of unit T_Domain_Regs */


                        {/*start of unit D_Domain_Regs */
                            {/*start of unit newTrainingUnit */
                                SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR3.dunitRegs.SDRAMCtrlRegs.D_Domain_Regs.newTrainingUnit.write_Leveling_DQS_pattern = 0x000016dc;

                            }/*end of unit newTrainingUnit */


                            {/*0001878+n*0x10*/
                                GT_U32    n;
                                for(n = 0 ; n <= 3 ; n++) {
                                    SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR3.dunitRegs.SDRAMCtrlRegs.D_Domain_Regs.DDR3MR2CS[n] =
                                        0x0001878+n*0x10;
                                }/* end of loop n */
                            }/*0001878+n*0x10*/
                            {/*000187c+n*0x10*/
                                GT_U32    n;
                                for(n = 0 ; n <= 3 ; n++) {
                                    SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR3.dunitRegs.SDRAMCtrlRegs.D_Domain_Regs.DDR3MR3CS[n] =
                                        0x000187c+n*0x10;
                                }/* end of loop n */
                            }/*000187c+n*0x10*/
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR3.dunitRegs.SDRAMCtrlRegs.D_Domain_Regs.SDRAMTimingHigh = 0x0000140c;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR3.dunitRegs.SDRAMCtrlRegs.D_Domain_Regs.SDRAMOperation = 0x00001418;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR3.dunitRegs.SDRAMCtrlRegs.D_Domain_Regs.SDRAMODTCtrlLow = 0x00001494;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR3.dunitRegs.SDRAMCtrlRegs.D_Domain_Regs.SDRAMODTCtrlHigh = 0x00001498;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR3.dunitRegs.SDRAMCtrlRegs.D_Domain_Regs.SDRAMTimingLow = 0x00001408;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR3.dunitRegs.SDRAMCtrlRegs.D_Domain_Regs.SDRAMConfig = 0x00001400;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR3.dunitRegs.SDRAMCtrlRegs.D_Domain_Regs.SDRAMAddrCtrl = 0x00001410;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR3.dunitRegs.SDRAMCtrlRegs.D_Domain_Regs.SDRAMOpenPagesCtrl = 0x00001414;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR3.dunitRegs.SDRAMCtrlRegs.D_Domain_Regs.readBufferSelect = 0x000014a4;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR3.dunitRegs.SDRAMCtrlRegs.D_Domain_Regs.DDRCtrllerCtrlHigh = 0x00001424;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR3.dunitRegs.SDRAMCtrlRegs.D_Domain_Regs.PHYLockMask = 0x00001670;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR3.dunitRegs.SDRAMCtrlRegs.D_Domain_Regs.DDR3Timing = 0x0000142c;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR3.dunitRegs.SDRAMCtrlRegs.D_Domain_Regs.DDRODTTimingHigh = 0x0000147c;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR3.dunitRegs.SDRAMCtrlRegs.D_Domain_Regs.SDRAMInitializationCtrl = 0x00001480;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR3.dunitRegs.SDRAMCtrlRegs.D_Domain_Regs.DRAMPHYConfig = 0x000015ec;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR3.dunitRegs.SDRAMCtrlRegs.D_Domain_Regs.DFS = 0x00001528;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR3.dunitRegs.SDRAMCtrlRegs.D_Domain_Regs.DDRCtrllerODTCtrl = 0x0000149c;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR3.dunitRegs.SDRAMCtrlRegs.D_Domain_Regs.DDR3RegedDRAMTiming = 0x000016d4;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR3.dunitRegs.SDRAMCtrlRegs.D_Domain_Regs.DDRIO = 0x00001524;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR3.dunitRegs.SDRAMCtrlRegs.D_Domain_Regs.DDR3RankCtrl = 0x000015e0;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR3.dunitRegs.SDRAMCtrlRegs.D_Domain_Regs.DDR3MR0 = 0x000015d0;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR3.dunitRegs.SDRAMCtrlRegs.D_Domain_Regs.DDR3MR1 = 0x000015d4;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR3.dunitRegs.SDRAMCtrlRegs.D_Domain_Regs.DDR3MR2 = 0x000015d8;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR3.dunitRegs.SDRAMCtrlRegs.D_Domain_Regs.DDR3MR3 = 0x000015dc;
                            {/*0001874+n*0x10*/
                                GT_U32    n;
                                for(n = 0 ; n <= 3 ; n++) {
                                    SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR3.dunitRegs.SDRAMCtrlRegs.D_Domain_Regs.DDR3MR1CS[n] =
                                        0x0001874+n*0x10;
                                }/* end of loop n */
                            }/*0001874+n*0x10*/
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR3.dunitRegs.SDRAMCtrlRegs.D_Domain_Regs.ZQCConfig = 0x000015e4;
                            {/*0001870+n*0x10*/
                                GT_U32    n;
                                for(n = 0 ; n <= 3 ; n++) {
                                    SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR3.dunitRegs.SDRAMCtrlRegs.D_Domain_Regs.DDR3MR0CS[n] =
                                        0x0001870+n*0x10;
                                }/* end of loop n */
                            }/*0001870+n*0x10*/
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR3.dunitRegs.SDRAMCtrlRegs.D_Domain_Regs.ODPGCTRLCtrl = 0x00001600;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR3.dunitRegs.SDRAMCtrlRegs.D_Domain_Regs.DDRODTTimingLow = 0x00001428;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR3.dunitRegs.SDRAMCtrlRegs.D_Domain_Regs.DRAMPHYLockStatus = 0x00001674;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR3.dunitRegs.SDRAMCtrlRegs.D_Domain_Regs.PHYRegFileAccess = 0x000016a0;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR3.dunitRegs.SDRAMCtrlRegs.D_Domain_Regs.DDR3RegedDRAMCtrl = 0x000016d0;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR3.dunitRegs.SDRAMCtrlRegs.D_Domain_Regs.DDRCtrllerCtrlLow = 0x00001404;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR3.dunitRegs.SDRAMCtrlRegs.D_Domain_Regs.dualDRAMCtrllerConfig = 0x000016d8;

                        }/*end of unit D_Domain_Regs */


                    }/*end of unit SDRAMCtrlRegs */


                }/*end of unit dunitRegs */


            }/*end of unit DRAMCtrllerRegsDDR3 */


            {/*start of unit DRAMCtrllerRegsDDR1 */
                {/*start of unit dunitRegs */
                    {/*start of unit SDRAMScratchPad */
                        {/*start of unit D_Domain_Regs */
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR1.dunitRegs.SDRAMScratchPad.D_Domain_Regs.scratchPad0 = 0x00029504;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR1.dunitRegs.SDRAMScratchPad.D_Domain_Regs.scratchPad1 = 0x0002950c;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR1.dunitRegs.SDRAMScratchPad.D_Domain_Regs.scratchPad2 = 0x00029514;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR1.dunitRegs.SDRAMScratchPad.D_Domain_Regs.scratchPad3 = 0x0002951c;

                        }/*end of unit D_Domain_Regs */


                    }/*end of unit SDRAMScratchPad */


                    {/*start of unit SDRAMDataProtectionAndErrorReport */
                        {/*start of unit R_Domain_Regs */
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR1.dunitRegs.SDRAMDataProtectionAndErrorReport.R_Domain_Regs.SDRAMErrorCtrl = 0x00029454;

                        }/*end of unit R_Domain_Regs */


                        {/*start of unit D_Domain_Regs */
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR1.dunitRegs.SDRAMDataProtectionAndErrorReport.D_Domain_Regs.DDRCtrllerErrorInterruptCause = 0x000294d0;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR1.dunitRegs.SDRAMDataProtectionAndErrorReport.D_Domain_Regs.DDRCtrllerErrorInterruptMask = 0x000294d4;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR1.dunitRegs.SDRAMDataProtectionAndErrorReport.D_Domain_Regs.DDRCtrllerMsgInterruptCause = 0x000294d8;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR1.dunitRegs.SDRAMDataProtectionAndErrorReport.D_Domain_Regs.DDRCtrllerMsgInterruptMask = 0x000294dc;

                        }/*end of unit D_Domain_Regs */


                    }/*end of unit SDRAMDataProtectionAndErrorReport */


                    {/*start of unit SDRAMCtrlRegs */
                        {/*start of unit T_Domain_Regs */
                            {/*start of unit newTrainingUnit */
                                {/*00290b8+n*0x4*/
                                    GT_U32    n;
                                    for(n = 0 ; n <= 1 ; n++) {
                                        SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR1.dunitRegs.SDRAMCtrlRegs.T_Domain_Regs.newTrainingUnit.currentObjectValueReg[n] =
                                            0x00290b8+n*0x4;
                                    }/* end of loop n */
                                }/*00290b8+n*0x4*/
                                {/*00290c0+n*0x4*/
                                    GT_U32    n;
                                    for(n = 0 ; n <= 1 ; n++) {
                                        SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR1.dunitRegs.SDRAMCtrlRegs.T_Domain_Regs.newTrainingUnit.calibratedObjectPRFAReg[n] =
                                            0x00290c0+n*0x4;
                                    }/* end of loop n */
                                }/*00290c0+n*0x4*/
                                {/*0029830+0x90+(n-40)*0x4+m*0x100 and 0029830+0x4d0*(n-79)+m*0*/
                                    GT_U32    n,m;
                                    for(n = 40 ; n <= 43 ; n++) {
                                        for(m = 0 ; m <= 4 ; m++) {
                                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR1.dunitRegs.SDRAMCtrlRegs.T_Domain_Regs.newTrainingUnit.resultCtrlReg[n - 40][m] =
                                                0x0029830+0x90+(n-40)*0x4+m*0x100;
                                        }/* end of loop m */
                                    }/* end of loop n */
                                    for(n = 80 ; n <= 80 ; n++) {
                                        for(m = 0 ; m <= 0 ; m++) {
                                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR1.dunitRegs.SDRAMCtrlRegs.T_Domain_Regs.newTrainingUnit.resultCtrlReg[(n - 79) + 3][m] =
                                                0x0029830+0x4d0*(n-79)/*+m*0*/;
                                        }/* end of loop m */
                                    }/* end of loop n */
                                }/*0029830+0x90+(n-40)*0x4+m*0x100 and 0029830+0x4d0*(n-79)+m*0*/
                                {/*0029130+n*0x4 and 0029130+0x1c0+(n-28)*0x4*/
                                    GT_U32    n;
                                    for(n = 0 ; n <= 3 ; n++) {
                                        SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR1.dunitRegs.SDRAMCtrlRegs.T_Domain_Regs.newTrainingUnit.PUPTestSequenceRegs[n] =
                                            0x0029130+n*0x4;
                                    }/* end of loop n */
                                    for(n = 28 ; n <= 31 ; n++) {
                                        SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR1.dunitRegs.SDRAMCtrlRegs.T_Domain_Regs.newTrainingUnit.PUPTestSequenceRegs[(n - 28) + 4] =
                                            0x0029130+0x1c0+(n-28)*0x4;
                                    }/* end of loop n */
                                }/*0029130+n*0x4 and 0029130+0x1c0+(n-28)*0x4*/
                                SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR1.dunitRegs.SDRAMCtrlRegs.T_Domain_Regs.newTrainingUnit.PUPTestCtrl = 0x000290f0;
                                {/*00290b0+n*0x4*/
                                    GT_U32    n;
                                    for(n = 0 ; n <= 1 ; n++) {
                                        SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR1.dunitRegs.SDRAMCtrlRegs.T_Domain_Regs.newTrainingUnit.objectOpCodeReg1[n] =
                                            0x00290b0+n*0x4;
                                    }/* end of loop n */
                                }/*00290b0+n*0x4*/
                                {/*0029038+n*0x4*/
                                    GT_U32    n;
                                    for(n = 0 ; n <= 1 ; n++) {
                                        SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR1.dunitRegs.SDRAMCtrlRegs.T_Domain_Regs.newTrainingUnit.objectOpCodeReg0[n] =
                                            0x0029038+n*0x4;
                                    }/* end of loop n */
                                }/*0029038+n*0x4*/
                                SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR1.dunitRegs.SDRAMCtrlRegs.T_Domain_Regs.newTrainingUnit.maxWindowSizeObjectValueReg = 0x000290f4;
                                SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR1.dunitRegs.SDRAMCtrlRegs.T_Domain_Regs.newTrainingUnit.globalCtrlAndStatusReg = 0x00029030;
                                SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR1.dunitRegs.SDRAMCtrlRegs.T_Domain_Regs.newTrainingUnit.generalPhaseOpcodeReg = 0x00029034;
                                SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR1.dunitRegs.SDRAMCtrlRegs.T_Domain_Regs.newTrainingUnit.writeLevelingDoneCntrReference = 0x000290f8;
                                SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR1.dunitRegs.SDRAMCtrlRegs.T_Domain_Regs.newTrainingUnit.writeReadTestCtrl = 0x000290fc;

                            }/*end of unit newTrainingUnit */


                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR1.dunitRegs.SDRAMCtrlRegs.T_Domain_Regs.SDRAMInterfaceMbusCtrlLow = 0x00029430;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR1.dunitRegs.SDRAMCtrlRegs.T_Domain_Regs.SDRAMInterfaceMbusCtrlHigh = 0x00029434;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR1.dunitRegs.SDRAMCtrlRegs.T_Domain_Regs.SDRAMInterfaceMbusTimeout = 0x00029438;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR1.dunitRegs.SDRAMCtrlRegs.T_Domain_Regs.DRAMCtrllerMiscellaneous = 0x000294b0;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR1.dunitRegs.SDRAMCtrlRegs.T_Domain_Regs.DRAMAddrAndCtrlDrivingStrengthLow = 0x000294c0;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR1.dunitRegs.SDRAMCtrlRegs.T_Domain_Regs.DRAMDataAndDQSDrivingStrengthLow = 0x000294c4;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR1.dunitRegs.SDRAMCtrlRegs.T_Domain_Regs.DRAMVerticalCalibrationMachineCtrl = 0x000294c8;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR1.dunitRegs.SDRAMCtrlRegs.T_Domain_Regs.DRAMMainPadsCalibrationMachineCtrl = 0x000294cc;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR1.dunitRegs.SDRAMCtrlRegs.T_Domain_Regs.SDRAMInterfaceMbusCtrl2Low = 0x000294f4;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR1.dunitRegs.SDRAMCtrlRegs.T_Domain_Regs.SDRAMInterfaceMbusCtrl2High = 0x000294f8;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR1.dunitRegs.SDRAMCtrlRegs.T_Domain_Regs.readDataSampleDelays = 0x00029538;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR1.dunitRegs.SDRAMCtrlRegs.T_Domain_Regs.readDataReadyDelays = 0x0002953c;

                        }/*end of unit T_Domain_Regs */


                        {/*start of unit D_Domain_Regs */
                            {/*start of unit newTrainingUnit */
                                SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR1.dunitRegs.SDRAMCtrlRegs.D_Domain_Regs.newTrainingUnit.write_Leveling_DQS_pattern = 0x000296dc;

                            }/*end of unit newTrainingUnit */


                            {/*002987c+n*0x10*/
                                GT_U32    n;
                                for(n = 0 ; n <= 3 ; n++) {
                                    SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR1.dunitRegs.SDRAMCtrlRegs.D_Domain_Regs.DDR3MR3CS[n] =
                                        0x002987c+n*0x10;
                                }/* end of loop n */
                            }/*002987c+n*0x10*/
                            {/*0029870+n*0x10*/
                                GT_U32    n;
                                for(n = 0 ; n <= 3 ; n++) {
                                    SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR1.dunitRegs.SDRAMCtrlRegs.D_Domain_Regs.DDR3MR0CS[n] =
                                        0x0029870+n*0x10;
                                }/* end of loop n */
                            }/*0029870+n*0x10*/
                            {/*0029874+n*0x10*/
                                GT_U32    n;
                                for(n = 0 ; n <= 3 ; n++) {
                                    SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR1.dunitRegs.SDRAMCtrlRegs.D_Domain_Regs.DDR3MR1CS[n] =
                                        0x0029874+n*0x10;
                                }/* end of loop n */
                            }/*0029874+n*0x10*/
                            {/*0029878+n*0x10*/
                                GT_U32    n;
                                for(n = 0 ; n <= 3 ; n++) {
                                    SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR1.dunitRegs.SDRAMCtrlRegs.D_Domain_Regs.DDR3MR2CS[n] =
                                        0x0029878+n*0x10;
                                }/* end of loop n */
                            }/*0029878+n*0x10*/
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR1.dunitRegs.SDRAMCtrlRegs.D_Domain_Regs.SDRAMConfig = 0x00029400;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR1.dunitRegs.SDRAMCtrlRegs.D_Domain_Regs.DDRCtrllerCtrlLow = 0x00029404;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR1.dunitRegs.SDRAMCtrlRegs.D_Domain_Regs.SDRAMTimingLow = 0x00029408;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR1.dunitRegs.SDRAMCtrlRegs.D_Domain_Regs.SDRAMTimingHigh = 0x0002940c;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR1.dunitRegs.SDRAMCtrlRegs.D_Domain_Regs.SDRAMAddrCtrl = 0x00029410;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR1.dunitRegs.SDRAMCtrlRegs.D_Domain_Regs.SDRAMOpenPagesCtrl = 0x00029414;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR1.dunitRegs.SDRAMCtrlRegs.D_Domain_Regs.SDRAMOperation = 0x00029418;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR1.dunitRegs.SDRAMCtrlRegs.D_Domain_Regs.DDRCtrllerCtrlHigh = 0x00029424;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR1.dunitRegs.SDRAMCtrlRegs.D_Domain_Regs.PHYLockMask = 0x00029670;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR1.dunitRegs.SDRAMCtrlRegs.D_Domain_Regs.DDR3Timing = 0x0002942c;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR1.dunitRegs.SDRAMCtrlRegs.D_Domain_Regs.DDRODTTimingHigh = 0x0002947c;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR1.dunitRegs.SDRAMCtrlRegs.D_Domain_Regs.SDRAMInitializationCtrl = 0x00029480;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR1.dunitRegs.SDRAMCtrlRegs.D_Domain_Regs.SDRAMODTCtrlLow = 0x00029494;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR1.dunitRegs.SDRAMCtrlRegs.D_Domain_Regs.SDRAMODTCtrlHigh = 0x00029498;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR1.dunitRegs.SDRAMCtrlRegs.D_Domain_Regs.DDRCtrllerODTCtrl = 0x0002949c;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR1.dunitRegs.SDRAMCtrlRegs.D_Domain_Regs.readBufferSelect = 0x000294a4;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR1.dunitRegs.SDRAMCtrlRegs.D_Domain_Regs.DDRIO = 0x00029524;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR1.dunitRegs.SDRAMCtrlRegs.D_Domain_Regs.DFS = 0x00029528;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR1.dunitRegs.SDRAMCtrlRegs.D_Domain_Regs.DDR3MR0 = 0x000295d0;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR1.dunitRegs.SDRAMCtrlRegs.D_Domain_Regs.DDR3MR1 = 0x000295d4;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR1.dunitRegs.SDRAMCtrlRegs.D_Domain_Regs.DDR3MR2 = 0x000295d8;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR1.dunitRegs.SDRAMCtrlRegs.D_Domain_Regs.DDR3MR3 = 0x000295dc;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR1.dunitRegs.SDRAMCtrlRegs.D_Domain_Regs.DDR3RankCtrl = 0x000295e0;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR1.dunitRegs.SDRAMCtrlRegs.D_Domain_Regs.ZQCConfig = 0x000295e4;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR1.dunitRegs.SDRAMCtrlRegs.D_Domain_Regs.DRAMPHYConfig = 0x000295ec;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR1.dunitRegs.SDRAMCtrlRegs.D_Domain_Regs.ODPGCTRLCtrl = 0x00029600;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR1.dunitRegs.SDRAMCtrlRegs.D_Domain_Regs.DDRODTTimingLow = 0x00029428;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR1.dunitRegs.SDRAMCtrlRegs.D_Domain_Regs.DRAMPHYLockStatus = 0x00029674;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR1.dunitRegs.SDRAMCtrlRegs.D_Domain_Regs.PHYRegFileAccess = 0x000296a0;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR1.dunitRegs.SDRAMCtrlRegs.D_Domain_Regs.DDR3RegedDRAMCtrl = 0x000296d0;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR1.dunitRegs.SDRAMCtrlRegs.D_Domain_Regs.DDR3RegedDRAMTiming = 0x000296d4;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR1.dunitRegs.SDRAMCtrlRegs.D_Domain_Regs.dualDRAMCtrllerConfig = 0x000296d8;

                        }/*end of unit D_Domain_Regs */


                    }/*end of unit SDRAMCtrlRegs */


                }/*end of unit dunitRegs */


            }/*end of unit DRAMCtrllerRegsDDR1 */


            {/*start of unit DRAMCtrllerRegsDDR2 */
                {/*start of unit dunitRegs */
                    {/*start of unit SDRAMScratchPad */
                        {/*start of unit D_Domain_Regs */
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR2.dunitRegs.SDRAMScratchPad.D_Domain_Regs.scratchPad0 = 0x00049504;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR2.dunitRegs.SDRAMScratchPad.D_Domain_Regs.scratchPad1 = 0x0004950c;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR2.dunitRegs.SDRAMScratchPad.D_Domain_Regs.scratchPad2 = 0x00049514;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR2.dunitRegs.SDRAMScratchPad.D_Domain_Regs.scratchPad3 = 0x0004951c;

                        }/*end of unit D_Domain_Regs */


                    }/*end of unit SDRAMScratchPad */


                    {/*start of unit SDRAMDataProtectionAndErrorReport */
                        {/*start of unit R_Domain_Regs */
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR2.dunitRegs.SDRAMDataProtectionAndErrorReport.R_Domain_Regs.SDRAMErrorCtrl = 0x00049454;

                        }/*end of unit R_Domain_Regs */


                        {/*start of unit D_Domain_Regs */
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR2.dunitRegs.SDRAMDataProtectionAndErrorReport.D_Domain_Regs.DDRCtrllerErrorInterruptCause = 0x000494d0;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR2.dunitRegs.SDRAMDataProtectionAndErrorReport.D_Domain_Regs.DDRCtrllerErrorInterruptMask = 0x000494d4;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR2.dunitRegs.SDRAMDataProtectionAndErrorReport.D_Domain_Regs.DDRCtrllerMsgInterruptCause = 0x000494d8;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR2.dunitRegs.SDRAMDataProtectionAndErrorReport.D_Domain_Regs.DDRCtrllerMsgInterruptMask = 0x000494dc;

                        }/*end of unit D_Domain_Regs */


                    }/*end of unit SDRAMDataProtectionAndErrorReport */


                    {/*start of unit SDRAMCtrlRegs */
                        {/*start of unit T_Domain_Regs */
                            {/*start of unit newTrainingUnit */
                                {/*00490b8+n*0x4*/
                                    GT_U32    n;
                                    for(n = 0 ; n <= 1 ; n++) {
                                        SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR2.dunitRegs.SDRAMCtrlRegs.T_Domain_Regs.newTrainingUnit.currentObjectValueReg[n] =
                                            0x00490b8+n*0x4;
                                    }/* end of loop n */
                                }/*00490b8+n*0x4*/
                                {/*00490c0+n*0x4*/
                                    GT_U32    n;
                                    for(n = 0 ; n <= 1 ; n++) {
                                        SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR2.dunitRegs.SDRAMCtrlRegs.T_Domain_Regs.newTrainingUnit.calibratedObjectPRFAReg[n] =
                                            0x00490c0+n*0x4;
                                    }/* end of loop n */
                                }/*00490c0+n*0x4*/
                                {/*0049830+0x90+(n-40)*0x4+m*0x100 and 0049830+0x4d0*(n-79)+m*0*/
                                    GT_U32    n,m;
                                    for(n = 40 ; n <= 43 ; n++) {
                                        for(m = 0 ; m <= 4 ; m++) {
                                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR2.dunitRegs.SDRAMCtrlRegs.T_Domain_Regs.newTrainingUnit.resultCtrlReg[n - 40][m] =
                                                0x0049830+0x90+(n-40)*0x4+m*0x100;
                                        }/* end of loop m */
                                    }/* end of loop n */
                                    for(n = 80 ; n <= 80 ; n++) {
                                        for(m = 0 ; m <= 0 ; m++) {
                                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR2.dunitRegs.SDRAMCtrlRegs.T_Domain_Regs.newTrainingUnit.resultCtrlReg[(n - 79) + 3][m] =
                                                0x0049830+0x4d0*(n-79)/*+m*0*/;
                                        }/* end of loop m */
                                    }/* end of loop n */
                                }/*0049830+0x90+(n-40)*0x4+m*0x100 and 0049830+0x4d0*(n-79)+m*0*/
                                {/*0049130+n*0x4 and 0049130+0x1c0+(n-28)*0x4*/
                                    GT_U32    n;
                                    for(n = 0 ; n <= 3 ; n++) {
                                        SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR2.dunitRegs.SDRAMCtrlRegs.T_Domain_Regs.newTrainingUnit.PUPTestSequenceRegs[n] =
                                            0x0049130+n*0x4;
                                    }/* end of loop n */
                                    for(n = 28 ; n <= 31 ; n++) {
                                        SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR2.dunitRegs.SDRAMCtrlRegs.T_Domain_Regs.newTrainingUnit.PUPTestSequenceRegs[(n - 28) + 4] =
                                            0x0049130+0x1c0+(n-28)*0x4;
                                    }/* end of loop n */
                                }/*0049130+n*0x4 and 0049130+0x1c0+(n-28)*0x4*/
                                SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR2.dunitRegs.SDRAMCtrlRegs.T_Domain_Regs.newTrainingUnit.PUPTestCtrl = 0x000490f0;
                                {/*00490b0+n*0x4*/
                                    GT_U32    n;
                                    for(n = 0 ; n <= 1 ; n++) {
                                        SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR2.dunitRegs.SDRAMCtrlRegs.T_Domain_Regs.newTrainingUnit.objectOpCodeReg1[n] =
                                            0x00490b0+n*0x4;
                                    }/* end of loop n */
                                }/*00490b0+n*0x4*/
                                {/*0049038+n*0x4*/
                                    GT_U32    n;
                                    for(n = 0 ; n <= 1 ; n++) {
                                        SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR2.dunitRegs.SDRAMCtrlRegs.T_Domain_Regs.newTrainingUnit.objectOpCodeReg0[n] =
                                            0x0049038+n*0x4;
                                    }/* end of loop n */
                                }/*0049038+n*0x4*/
                                SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR2.dunitRegs.SDRAMCtrlRegs.T_Domain_Regs.newTrainingUnit.maxWindowSizeObjectValueReg = 0x000490f4;
                                SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR2.dunitRegs.SDRAMCtrlRegs.T_Domain_Regs.newTrainingUnit.globalCtrlAndStatusReg = 0x00049030;
                                SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR2.dunitRegs.SDRAMCtrlRegs.T_Domain_Regs.newTrainingUnit.generalPhaseOpcodeReg = 0x00049034;
                                SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR2.dunitRegs.SDRAMCtrlRegs.T_Domain_Regs.newTrainingUnit.writeLevelingDoneCntrReference = 0x000490f8;
                                SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR2.dunitRegs.SDRAMCtrlRegs.T_Domain_Regs.newTrainingUnit.writeReadTestCtrl = 0x000490fc;

                            }/*end of unit newTrainingUnit */


                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR2.dunitRegs.SDRAMCtrlRegs.T_Domain_Regs.SDRAMInterfaceMbusCtrlLow = 0x00049430;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR2.dunitRegs.SDRAMCtrlRegs.T_Domain_Regs.SDRAMInterfaceMbusCtrlHigh = 0x00049434;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR2.dunitRegs.SDRAMCtrlRegs.T_Domain_Regs.SDRAMInterfaceMbusTimeout = 0x00049438;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR2.dunitRegs.SDRAMCtrlRegs.T_Domain_Regs.DRAMCtrllerMiscellaneous = 0x000494b0;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR2.dunitRegs.SDRAMCtrlRegs.T_Domain_Regs.DRAMAddrAndCtrlDrivingStrengthLow = 0x000494c0;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR2.dunitRegs.SDRAMCtrlRegs.T_Domain_Regs.DRAMDataAndDQSDrivingStrengthLow = 0x000494c4;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR2.dunitRegs.SDRAMCtrlRegs.T_Domain_Regs.DRAMVerticalCalibrationMachineCtrl = 0x000494c8;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR2.dunitRegs.SDRAMCtrlRegs.T_Domain_Regs.DRAMMainPadsCalibrationMachineCtrl = 0x000494cc;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR2.dunitRegs.SDRAMCtrlRegs.T_Domain_Regs.SDRAMInterfaceMbusCtrl2Low = 0x000494f4;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR2.dunitRegs.SDRAMCtrlRegs.T_Domain_Regs.SDRAMInterfaceMbusCtrl2High = 0x000494f8;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR2.dunitRegs.SDRAMCtrlRegs.T_Domain_Regs.readDataSampleDelays = 0x00049538;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR2.dunitRegs.SDRAMCtrlRegs.T_Domain_Regs.readDataReadyDelays = 0x0004953c;

                        }/*end of unit T_Domain_Regs */


                        {/*start of unit D_Domain_Regs */
                            {/*start of unit newTrainingUnit */
                                SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR2.dunitRegs.SDRAMCtrlRegs.D_Domain_Regs.newTrainingUnit.write_Leveling_DQS_pattern = 0x000496dc;

                            }/*end of unit newTrainingUnit */


                            {/*004987c+n*0x10*/
                                GT_U32    n;
                                for(n = 0 ; n <= 3 ; n++) {
                                    SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR2.dunitRegs.SDRAMCtrlRegs.D_Domain_Regs.DDR3MR3CS[n] =
                                        0x004987c+n*0x10;
                                }/* end of loop n */
                            }/*004987c+n*0x10*/
                            {/*0049870+n*0x10*/
                                GT_U32    n;
                                for(n = 0 ; n <= 3 ; n++) {
                                    SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR2.dunitRegs.SDRAMCtrlRegs.D_Domain_Regs.DDR3MR0CS[n] =
                                        0x0049870+n*0x10;
                                }/* end of loop n */
                            }/*0049870+n*0x10*/
                            {/*0049874+n*0x10*/
                                GT_U32    n;
                                for(n = 0 ; n <= 3 ; n++) {
                                    SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR2.dunitRegs.SDRAMCtrlRegs.D_Domain_Regs.DDR3MR1CS[n] =
                                        0x0049874+n*0x10;
                                }/* end of loop n */
                            }/*0049874+n*0x10*/
                            {/*0049878+n*0x10*/
                                GT_U32    n;
                                for(n = 0 ; n <= 3 ; n++) {
                                    SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR2.dunitRegs.SDRAMCtrlRegs.D_Domain_Regs.DDR3MR2CS[n] =
                                        0x0049878+n*0x10;
                                }/* end of loop n */
                            }/*0049878+n*0x10*/
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR2.dunitRegs.SDRAMCtrlRegs.D_Domain_Regs.SDRAMConfig = 0x00049400;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR2.dunitRegs.SDRAMCtrlRegs.D_Domain_Regs.DDRCtrllerCtrlLow = 0x00049404;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR2.dunitRegs.SDRAMCtrlRegs.D_Domain_Regs.SDRAMTimingLow = 0x00049408;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR2.dunitRegs.SDRAMCtrlRegs.D_Domain_Regs.SDRAMTimingHigh = 0x0004940c;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR2.dunitRegs.SDRAMCtrlRegs.D_Domain_Regs.SDRAMAddrCtrl = 0x00049410;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR2.dunitRegs.SDRAMCtrlRegs.D_Domain_Regs.SDRAMOpenPagesCtrl = 0x00049414;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR2.dunitRegs.SDRAMCtrlRegs.D_Domain_Regs.SDRAMOperation = 0x00049418;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR2.dunitRegs.SDRAMCtrlRegs.D_Domain_Regs.DDRCtrllerCtrlHigh = 0x00049424;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR2.dunitRegs.SDRAMCtrlRegs.D_Domain_Regs.PHYLockMask = 0x00049670;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR2.dunitRegs.SDRAMCtrlRegs.D_Domain_Regs.DDR3Timing = 0x0004942c;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR2.dunitRegs.SDRAMCtrlRegs.D_Domain_Regs.DDRODTTimingHigh = 0x0004947c;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR2.dunitRegs.SDRAMCtrlRegs.D_Domain_Regs.SDRAMInitializationCtrl = 0x00049480;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR2.dunitRegs.SDRAMCtrlRegs.D_Domain_Regs.SDRAMODTCtrlLow = 0x00049494;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR2.dunitRegs.SDRAMCtrlRegs.D_Domain_Regs.SDRAMODTCtrlHigh = 0x00049498;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR2.dunitRegs.SDRAMCtrlRegs.D_Domain_Regs.DDRCtrllerODTCtrl = 0x0004949c;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR2.dunitRegs.SDRAMCtrlRegs.D_Domain_Regs.readBufferSelect = 0x000494a4;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR2.dunitRegs.SDRAMCtrlRegs.D_Domain_Regs.DDRIO = 0x00049524;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR2.dunitRegs.SDRAMCtrlRegs.D_Domain_Regs.DFS = 0x00049528;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR2.dunitRegs.SDRAMCtrlRegs.D_Domain_Regs.DDR3MR0 = 0x000495d0;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR2.dunitRegs.SDRAMCtrlRegs.D_Domain_Regs.DDR3MR1 = 0x000495d4;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR2.dunitRegs.SDRAMCtrlRegs.D_Domain_Regs.DDR3MR2 = 0x000495d8;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR2.dunitRegs.SDRAMCtrlRegs.D_Domain_Regs.DDR3MR3 = 0x000495dc;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR2.dunitRegs.SDRAMCtrlRegs.D_Domain_Regs.DDR3RankCtrl = 0x000495e0;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR2.dunitRegs.SDRAMCtrlRegs.D_Domain_Regs.ZQCConfig = 0x000495e4;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR2.dunitRegs.SDRAMCtrlRegs.D_Domain_Regs.DRAMPHYConfig = 0x000495ec;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR2.dunitRegs.SDRAMCtrlRegs.D_Domain_Regs.ODPGCTRLCtrl = 0x00049600;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR2.dunitRegs.SDRAMCtrlRegs.D_Domain_Regs.DDRODTTimingLow = 0x00049428;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR2.dunitRegs.SDRAMCtrlRegs.D_Domain_Regs.DRAMPHYLockStatus = 0x00049674;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR2.dunitRegs.SDRAMCtrlRegs.D_Domain_Regs.PHYRegFileAccess = 0x000496a0;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR2.dunitRegs.SDRAMCtrlRegs.D_Domain_Regs.DDR3RegedDRAMCtrl = 0x000496d0;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR2.dunitRegs.SDRAMCtrlRegs.D_Domain_Regs.DDR3RegedDRAMTiming = 0x000496d4;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR2.dunitRegs.SDRAMCtrlRegs.D_Domain_Regs.dualDRAMCtrllerConfig = 0x000496d8;

                        }/*end of unit D_Domain_Regs */


                    }/*end of unit SDRAMCtrlRegs */


                }/*end of unit dunitRegs */


            }/*end of unit DRAMCtrllerRegsDDR2 */


            {/*start of unit DRAMCtrllerRegsDDR0 */
                {/*start of unit dunitRegs */
                    {/*start of unit SDRAMDataProtectionAndErrorReport */
                        {/*start of unit R_Domain_Regs */
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR0.dunitRegs.SDRAMDataProtectionAndErrorReport.R_Domain_Regs.SDRAMErrorCtrl = 0x00071454;

                        }/*end of unit R_Domain_Regs */


                        {/*start of unit D_Domain_Regs */
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR0.dunitRegs.SDRAMDataProtectionAndErrorReport.D_Domain_Regs.DDRCtrllerErrorInterruptCause = 0x000714d0;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR0.dunitRegs.SDRAMDataProtectionAndErrorReport.D_Domain_Regs.DDRCtrllerErrorInterruptMask = 0x000714d4;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR0.dunitRegs.SDRAMDataProtectionAndErrorReport.D_Domain_Regs.DDRCtrllerMsgInterruptCause = 0x000714d8;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR0.dunitRegs.SDRAMDataProtectionAndErrorReport.D_Domain_Regs.DDRCtrllerMsgInterruptMask = 0x000714dc;

                        }/*end of unit D_Domain_Regs */


                    }/*end of unit SDRAMDataProtectionAndErrorReport */


                    {/*start of unit SDRAMCtrlRegs */
                        {/*start of unit T_Domain_Regs */
                            {/*start of unit newTrainingUnit */
                                {/*00710b8+n*0x4*/
                                    GT_U32    n;
                                    for(n = 0 ; n <= 1 ; n++) {
                                        SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR0.dunitRegs.SDRAMCtrlRegs.T_Domain_Regs.newTrainingUnit.currentObjectValueReg[n] =
                                            0x00710b8+n*0x4;
                                    }/* end of loop n */
                                }/*00710b8+n*0x4*/
                                {/*00710c0+n*0x4*/
                                    GT_U32    n;
                                    for(n = 0 ; n <= 1 ; n++) {
                                        SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR0.dunitRegs.SDRAMCtrlRegs.T_Domain_Regs.newTrainingUnit.calibratedObjectPRFAReg[n] =
                                            0x00710c0+n*0x4;
                                    }/* end of loop n */
                                }/*00710c0+n*0x4*/
                                {/*0071830+0x90+(n-40)*0x4+m*0x100 and 0071830+0x4d0*(n-79)+m*0*/
                                    GT_U32    n,m;
                                    for(n = 40 ; n <= 43 ; n++) {
                                        for(m = 0 ; m <= 4 ; m++) {
                                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR0.dunitRegs.SDRAMCtrlRegs.T_Domain_Regs.newTrainingUnit.resultCtrlReg[n - 40][m] =
                                                0x0071830+0x90+(n-40)*0x4+m*0x100;
                                        }/* end of loop m */
                                    }/* end of loop n */
                                    for(n = 80 ; n <= 80 ; n++) {
                                        for(m = 0 ; m <= 0 ; m++) {
                                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR0.dunitRegs.SDRAMCtrlRegs.T_Domain_Regs.newTrainingUnit.resultCtrlReg[(n - 79) + 3][m] =
                                                0x0071830+0x4d0*(n-79)/*+m*0*/;
                                        }/* end of loop m */
                                    }/* end of loop n */
                                }/*0071830+0x90+(n-40)*0x4+m*0x100 and 0071830+0x4d0*(n-79)+m*0*/
                                {/*0071130+n*0x4 and 0071130+0x1c0+(n-28)*0x4*/
                                    GT_U32    n;
                                    for(n = 0 ; n <= 3 ; n++) {
                                        SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR0.dunitRegs.SDRAMCtrlRegs.T_Domain_Regs.newTrainingUnit.PUPTestSequenceRegs[n] =
                                            0x0071130+n*0x4;
                                    }/* end of loop n */
                                    for(n = 28 ; n <= 31 ; n++) {
                                        SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR0.dunitRegs.SDRAMCtrlRegs.T_Domain_Regs.newTrainingUnit.PUPTestSequenceRegs[(n - 28) + 4] =
                                            0x0071130+0x1c0+(n-28)*0x4;
                                    }/* end of loop n */
                                }/*0071130+n*0x4 and 0071130+0x1c0+(n-28)*0x4*/
                                SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR0.dunitRegs.SDRAMCtrlRegs.T_Domain_Regs.newTrainingUnit.PUPTestCtrl = 0x000710f0;
                                {/*00710b0+n*0x4*/
                                    GT_U32    n;
                                    for(n = 0 ; n <= 1 ; n++) {
                                        SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR0.dunitRegs.SDRAMCtrlRegs.T_Domain_Regs.newTrainingUnit.objectOpCodeReg1[n] =
                                            0x00710b0+n*0x4;
                                    }/* end of loop n */
                                }/*00710b0+n*0x4*/
                                {/*0071038+n*0x4*/
                                    GT_U32    n;
                                    for(n = 0 ; n <= 1 ; n++) {
                                        SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR0.dunitRegs.SDRAMCtrlRegs.T_Domain_Regs.newTrainingUnit.objectOpCodeReg0[n] =
                                            0x0071038+n*0x4;
                                    }/* end of loop n */
                                }/*0071038+n*0x4*/
                                SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR0.dunitRegs.SDRAMCtrlRegs.T_Domain_Regs.newTrainingUnit.maxWindowSizeObjectValueReg = 0x000710f4;
                                SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR0.dunitRegs.SDRAMCtrlRegs.T_Domain_Regs.newTrainingUnit.globalCtrlAndStatusReg = 0x00071030;
                                SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR0.dunitRegs.SDRAMCtrlRegs.T_Domain_Regs.newTrainingUnit.generalPhaseOpcodeReg = 0x00071034;
                                SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR0.dunitRegs.SDRAMCtrlRegs.T_Domain_Regs.newTrainingUnit.writeLevelingDoneCntrReference = 0x000710f8;
                                SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR0.dunitRegs.SDRAMCtrlRegs.T_Domain_Regs.newTrainingUnit.writeReadTestCtrl = 0x000710fc;

                            }/*end of unit newTrainingUnit */


                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR0.dunitRegs.SDRAMCtrlRegs.T_Domain_Regs.SDRAMInterfaceMbusCtrlLow = 0x00071430;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR0.dunitRegs.SDRAMCtrlRegs.T_Domain_Regs.SDRAMInterfaceMbusCtrlHigh = 0x00071434;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR0.dunitRegs.SDRAMCtrlRegs.T_Domain_Regs.SDRAMInterfaceMbusTimeout = 0x00071438;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR0.dunitRegs.SDRAMCtrlRegs.T_Domain_Regs.DRAMCtrllerMiscellaneous = 0x000714b0;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR0.dunitRegs.SDRAMCtrlRegs.T_Domain_Regs.DRAMAddrAndCtrlDrivingStrengthLow = 0x000714c0;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR0.dunitRegs.SDRAMCtrlRegs.T_Domain_Regs.DRAMDataAndDQSDrivingStrengthLow = 0x000714c4;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR0.dunitRegs.SDRAMCtrlRegs.T_Domain_Regs.DRAMVerticalCalibrationMachineCtrl = 0x000714c8;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR0.dunitRegs.SDRAMCtrlRegs.T_Domain_Regs.DRAMMainPadsCalibrationMachineCtrl = 0x000714cc;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR0.dunitRegs.SDRAMCtrlRegs.T_Domain_Regs.SDRAMInterfaceMbusCtrl2Low = 0x000714f4;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR0.dunitRegs.SDRAMCtrlRegs.T_Domain_Regs.SDRAMInterfaceMbusCtrl2High = 0x000714f8;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR0.dunitRegs.SDRAMCtrlRegs.T_Domain_Regs.readDataSampleDelays = 0x00071538;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR0.dunitRegs.SDRAMCtrlRegs.T_Domain_Regs.readDataReadyDelays = 0x0007153c;

                        }/*end of unit T_Domain_Regs */


                        {/*start of unit D_Domain_Regs */
                            {/*start of unit newTrainingUnit */
                                SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR0.dunitRegs.SDRAMCtrlRegs.D_Domain_Regs.newTrainingUnit.write_Leveling_DQS_pattern = 0x000716dc;

                            }/*end of unit newTrainingUnit */


                            {/*0071878+n*0x10*/
                                GT_U32    n;
                                for(n = 0 ; n <= 3 ; n++) {
                                    SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR0.dunitRegs.SDRAMCtrlRegs.D_Domain_Regs.DDR3MR2CS[n] =
                                        0x0071878+n*0x10;
                                }/* end of loop n */
                            }/*0071878+n*0x10*/
                            {/*007187c+n*0x10*/
                                GT_U32    n;
                                for(n = 0 ; n <= 3 ; n++) {
                                    SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR0.dunitRegs.SDRAMCtrlRegs.D_Domain_Regs.DDR3MR3CS[n] =
                                        0x007187c+n*0x10;
                                }/* end of loop n */
                            }/*007187c+n*0x10*/
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR0.dunitRegs.SDRAMCtrlRegs.D_Domain_Regs.SDRAMTimingHigh = 0x0007140c;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR0.dunitRegs.SDRAMCtrlRegs.D_Domain_Regs.SDRAMOperation = 0x00071418;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR0.dunitRegs.SDRAMCtrlRegs.D_Domain_Regs.SDRAMODTCtrlLow = 0x00071494;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR0.dunitRegs.SDRAMCtrlRegs.D_Domain_Regs.SDRAMODTCtrlHigh = 0x00071498;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR0.dunitRegs.SDRAMCtrlRegs.D_Domain_Regs.SDRAMTimingLow = 0x00071408;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR0.dunitRegs.SDRAMCtrlRegs.D_Domain_Regs.SDRAMConfig = 0x00071400;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR0.dunitRegs.SDRAMCtrlRegs.D_Domain_Regs.SDRAMAddrCtrl = 0x00071410;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR0.dunitRegs.SDRAMCtrlRegs.D_Domain_Regs.SDRAMOpenPagesCtrl = 0x00071414;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR0.dunitRegs.SDRAMCtrlRegs.D_Domain_Regs.readBufferSelect = 0x000714a4;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR0.dunitRegs.SDRAMCtrlRegs.D_Domain_Regs.DDRCtrllerCtrlHigh = 0x00071424;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR0.dunitRegs.SDRAMCtrlRegs.D_Domain_Regs.PHYLockMask = 0x00071670;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR0.dunitRegs.SDRAMCtrlRegs.D_Domain_Regs.DDR3Timing = 0x0007142c;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR0.dunitRegs.SDRAMCtrlRegs.D_Domain_Regs.DDRODTTimingHigh = 0x0007147c;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR0.dunitRegs.SDRAMCtrlRegs.D_Domain_Regs.SDRAMInitializationCtrl = 0x00071480;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR0.dunitRegs.SDRAMCtrlRegs.D_Domain_Regs.DRAMPHYConfig = 0x000715ec;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR0.dunitRegs.SDRAMCtrlRegs.D_Domain_Regs.DFS = 0x00071528;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR0.dunitRegs.SDRAMCtrlRegs.D_Domain_Regs.DDRCtrllerODTCtrl = 0x0007149c;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR0.dunitRegs.SDRAMCtrlRegs.D_Domain_Regs.DDR3RegedDRAMTiming = 0x000716d4;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR0.dunitRegs.SDRAMCtrlRegs.D_Domain_Regs.DDRIO = 0x00071524;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR0.dunitRegs.SDRAMCtrlRegs.D_Domain_Regs.DDR3RankCtrl = 0x000715e0;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR0.dunitRegs.SDRAMCtrlRegs.D_Domain_Regs.DDR3MR0 = 0x000715d0;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR0.dunitRegs.SDRAMCtrlRegs.D_Domain_Regs.DDR3MR1 = 0x000715d4;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR0.dunitRegs.SDRAMCtrlRegs.D_Domain_Regs.DDR3MR2 = 0x000715d8;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR0.dunitRegs.SDRAMCtrlRegs.D_Domain_Regs.DDR3MR3 = 0x000715dc;
                            {/*0071874+n*0x10*/
                                GT_U32    n;
                                for(n = 0 ; n <= 3 ; n++) {
                                    SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR0.dunitRegs.SDRAMCtrlRegs.D_Domain_Regs.DDR3MR1CS[n] =
                                        0x0071874+n*0x10;
                                }/* end of loop n */
                            }/*0071874+n*0x10*/
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR0.dunitRegs.SDRAMCtrlRegs.D_Domain_Regs.ZQCConfig = 0x000715e4;
                            {/*0071870+n*0x10*/
                                GT_U32    n;
                                for(n = 0 ; n <= 3 ; n++) {
                                    SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR0.dunitRegs.SDRAMCtrlRegs.D_Domain_Regs.DDR3MR0CS[n] =
                                        0x0071870+n*0x10;
                                }/* end of loop n */
                            }/*0071870+n*0x10*/
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR0.dunitRegs.SDRAMCtrlRegs.D_Domain_Regs.ODPGCTRLCtrl = 0x00071600;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR0.dunitRegs.SDRAMCtrlRegs.D_Domain_Regs.DDRODTTimingLow = 0x00071428;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR0.dunitRegs.SDRAMCtrlRegs.D_Domain_Regs.DRAMPHYLockStatus = 0x00071674;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR0.dunitRegs.SDRAMCtrlRegs.D_Domain_Regs.PHYRegFileAccess = 0x000716a0;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR0.dunitRegs.SDRAMCtrlRegs.D_Domain_Regs.DDR3RegedDRAMCtrl = 0x000716d0;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR0.dunitRegs.SDRAMCtrlRegs.D_Domain_Regs.DDRCtrllerCtrlLow = 0x00071404;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR0.dunitRegs.SDRAMCtrlRegs.D_Domain_Regs.dualDRAMCtrllerConfig = 0x000716d8;

                        }/*end of unit D_Domain_Regs */


                    }/*end of unit SDRAMCtrlRegs */


                    {/*start of unit SDRAMScratchPad */
                        {/*start of unit D_Domain_Regs */
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR0.dunitRegs.SDRAMScratchPad.D_Domain_Regs.scratchPad0 = 0x00071504;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR0.dunitRegs.SDRAMScratchPad.D_Domain_Regs.scratchPad1 = 0x0007150c;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR0.dunitRegs.SDRAMScratchPad.D_Domain_Regs.scratchPad2 = 0x00071514;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR0.dunitRegs.SDRAMScratchPad.D_Domain_Regs.scratchPad3 = 0x0007151c;

                        }/*end of unit D_Domain_Regs */


                    }/*end of unit SDRAMScratchPad */


                }/*end of unit dunitRegs */


            }/*end of unit DRAMCtrllerRegsDDR0 */


            {/*start of unit DRAMCtrllerRegsDDR4 */
                {/*start of unit dunitRegs */
                    {/*start of unit SDRAMScratchPad */
                        {/*start of unit D_Domain_Regs */
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR4.dunitRegs.SDRAMScratchPad.D_Domain_Regs.scratchPad0 = 0x000B9504;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR4.dunitRegs.SDRAMScratchPad.D_Domain_Regs.scratchPad1 = 0x000B950c;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR4.dunitRegs.SDRAMScratchPad.D_Domain_Regs.scratchPad2 = 0x000B9514;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR4.dunitRegs.SDRAMScratchPad.D_Domain_Regs.scratchPad3 = 0x000B951c;

                        }/*end of unit D_Domain_Regs */


                    }/*end of unit SDRAMScratchPad */


                    {/*start of unit SDRAMCtrlRegs */
                        {/*start of unit T_Domain_Regs */
                            {/*start of unit newTrainingUnit */
                                {/*00B90b8+n*0x4*/
                                    GT_U32    n;
                                    for(n = 0 ; n <= 1 ; n++) {
                                        SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR4.dunitRegs.SDRAMCtrlRegs.T_Domain_Regs.newTrainingUnit.currentObjectValueReg[n] =
                                            0x00B90b8+n*0x4;
                                    }/* end of loop n */
                                }/*00B90b8+n*0x4*/
                                {/*00B90c0+n*0x4*/
                                    GT_U32    n;
                                    for(n = 0 ; n <= 1 ; n++) {
                                        SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR4.dunitRegs.SDRAMCtrlRegs.T_Domain_Regs.newTrainingUnit.calibratedObjectPRFAReg[n] =
                                            0x00B90c0+n*0x4;
                                    }/* end of loop n */
                                }/*00B90c0+n*0x4*/
                                {/*00B9830+0x90+(n-40)*0x4+m*0x100 and 00B9830+0x4d0*(n-79)+m*0*/
                                    GT_U32    n,m;
                                    for(n = 40 ; n <= 43 ; n++) {
                                        for(m = 0 ; m <= 4 ; m++) {
                                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR4.dunitRegs.SDRAMCtrlRegs.T_Domain_Regs.newTrainingUnit.resultCtrlReg[n - 40][m] =
                                                0x00B9830+0x90+(n-40)*0x4+m*0x100;
                                        }/* end of loop m */
                                    }/* end of loop n */
                                    for(n = 80 ; n <= 80 ; n++) {
                                        for(m = 0 ; m <= 0 ; m++) {
                                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR4.dunitRegs.SDRAMCtrlRegs.T_Domain_Regs.newTrainingUnit.resultCtrlReg[(n - 79) + 3][m] =
                                                0x00B9830+0x4d0*(n-79)/*+m*0*/;
                                        }/* end of loop m */
                                    }/* end of loop n */
                                }/*00B9830+0x90+(n-40)*0x4+m*0x100 and 00B9830+0x4d0*(n-79)+m*0*/
                                {/*00B9130+n*0x4 and 00B9130+0x1c0+(n-28)*0x4*/
                                    GT_U32    n;
                                    for(n = 0 ; n <= 3 ; n++) {
                                        SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR4.dunitRegs.SDRAMCtrlRegs.T_Domain_Regs.newTrainingUnit.PUPTestSequenceRegs[n] =
                                            0x00B9130+n*0x4;
                                    }/* end of loop n */
                                    for(n = 28 ; n <= 31 ; n++) {
                                        SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR4.dunitRegs.SDRAMCtrlRegs.T_Domain_Regs.newTrainingUnit.PUPTestSequenceRegs[(n - 28) + 4] =
                                            0x00B9130+0x1c0+(n-28)*0x4;
                                    }/* end of loop n */
                                }/*00B9130+n*0x4 and 00B9130+0x1c0+(n-28)*0x4*/
                                SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR4.dunitRegs.SDRAMCtrlRegs.T_Domain_Regs.newTrainingUnit.PUPTestCtrl = 0x000B90f0;
                                {/*00B90b0+n*0x4*/
                                    GT_U32    n;
                                    for(n = 0 ; n <= 1 ; n++) {
                                        SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR4.dunitRegs.SDRAMCtrlRegs.T_Domain_Regs.newTrainingUnit.objectOpCodeReg1[n] =
                                            0x00B90b0+n*0x4;
                                    }/* end of loop n */
                                }/*00B90b0+n*0x4*/
                                {/*00B9038+n*0x4*/
                                    GT_U32    n;
                                    for(n = 0 ; n <= 1 ; n++) {
                                        SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR4.dunitRegs.SDRAMCtrlRegs.T_Domain_Regs.newTrainingUnit.objectOpCodeReg0[n] =
                                            0x00B9038+n*0x4;
                                    }/* end of loop n */
                                }/*00B9038+n*0x4*/
                                SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR4.dunitRegs.SDRAMCtrlRegs.T_Domain_Regs.newTrainingUnit.maxWindowSizeObjectValueReg = 0x000B90f4;
                                SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR4.dunitRegs.SDRAMCtrlRegs.T_Domain_Regs.newTrainingUnit.globalCtrlAndStatusReg = 0x000B9030;
                                SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR4.dunitRegs.SDRAMCtrlRegs.T_Domain_Regs.newTrainingUnit.generalPhaseOpcodeReg = 0x000B9034;
                                SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR4.dunitRegs.SDRAMCtrlRegs.T_Domain_Regs.newTrainingUnit.writeLevelingDoneCntrReference = 0x000B90f8;
                                SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR4.dunitRegs.SDRAMCtrlRegs.T_Domain_Regs.newTrainingUnit.writeReadTestCtrl = 0x000B90fc;

                            }/*end of unit newTrainingUnit */


                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR4.dunitRegs.SDRAMCtrlRegs.T_Domain_Regs.SDRAMInterfaceMbusCtrlLow = 0x000B9430;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR4.dunitRegs.SDRAMCtrlRegs.T_Domain_Regs.SDRAMInterfaceMbusCtrlHigh = 0x000B9434;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR4.dunitRegs.SDRAMCtrlRegs.T_Domain_Regs.SDRAMInterfaceMbusTimeout = 0x000B9438;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR4.dunitRegs.SDRAMCtrlRegs.T_Domain_Regs.DRAMCtrllerMiscellaneous = 0x000B94b0;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR4.dunitRegs.SDRAMCtrlRegs.T_Domain_Regs.DRAMAddrAndCtrlDrivingStrengthLow = 0x000B94c0;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR4.dunitRegs.SDRAMCtrlRegs.T_Domain_Regs.DRAMDataAndDQSDrivingStrengthLow = 0x000B94c4;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR4.dunitRegs.SDRAMCtrlRegs.T_Domain_Regs.DRAMVerticalCalibrationMachineCtrl = 0x000B94c8;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR4.dunitRegs.SDRAMCtrlRegs.T_Domain_Regs.DRAMMainPadsCalibrationMachineCtrl = 0x000B94cc;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR4.dunitRegs.SDRAMCtrlRegs.T_Domain_Regs.SDRAMInterfaceMbusCtrl2Low = 0x000B94f4;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR4.dunitRegs.SDRAMCtrlRegs.T_Domain_Regs.SDRAMInterfaceMbusCtrl2High = 0x000B94f8;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR4.dunitRegs.SDRAMCtrlRegs.T_Domain_Regs.readDataSampleDelays = 0x000B9538;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR4.dunitRegs.SDRAMCtrlRegs.T_Domain_Regs.readDataReadyDelays = 0x000B953c;

                        }/*end of unit T_Domain_Regs */


                        {/*start of unit D_Domain_Regs */
                            {/*start of unit newTrainingUnit */
                                SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR4.dunitRegs.SDRAMCtrlRegs.D_Domain_Regs.newTrainingUnit.write_Leveling_DQS_pattern = 0x000B96dc;

                            }/*end of unit newTrainingUnit */


                            {/*00B9878+n*0x10*/
                                GT_U32    n;
                                for(n = 0 ; n <= 3 ; n++) {
                                    SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR4.dunitRegs.SDRAMCtrlRegs.D_Domain_Regs.DDR3MR2CS[n] =
                                        0x00B9878+n*0x10;
                                }/* end of loop n */
                            }/*00B9878+n*0x10*/
                            {/*00B987c+n*0x10*/
                                GT_U32    n;
                                for(n = 0 ; n <= 3 ; n++) {
                                    SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR4.dunitRegs.SDRAMCtrlRegs.D_Domain_Regs.DDR3MR3CS[n] =
                                        0x00B987c+n*0x10;
                                }/* end of loop n */
                            }/*00B987c+n*0x10*/
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR4.dunitRegs.SDRAMCtrlRegs.D_Domain_Regs.SDRAMTimingHigh = 0x000B940c;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR4.dunitRegs.SDRAMCtrlRegs.D_Domain_Regs.SDRAMOperation = 0x000B9418;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR4.dunitRegs.SDRAMCtrlRegs.D_Domain_Regs.SDRAMODTCtrlLow = 0x000B9494;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR4.dunitRegs.SDRAMCtrlRegs.D_Domain_Regs.SDRAMODTCtrlHigh = 0x000B9498;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR4.dunitRegs.SDRAMCtrlRegs.D_Domain_Regs.SDRAMTimingLow = 0x000B9408;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR4.dunitRegs.SDRAMCtrlRegs.D_Domain_Regs.SDRAMConfig = 0x000B9400;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR4.dunitRegs.SDRAMCtrlRegs.D_Domain_Regs.SDRAMAddrCtrl = 0x000B9410;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR4.dunitRegs.SDRAMCtrlRegs.D_Domain_Regs.SDRAMOpenPagesCtrl = 0x000B9414;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR4.dunitRegs.SDRAMCtrlRegs.D_Domain_Regs.readBufferSelect = 0x000B94a4;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR4.dunitRegs.SDRAMCtrlRegs.D_Domain_Regs.DDRCtrllerCtrlHigh = 0x000B9424;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR4.dunitRegs.SDRAMCtrlRegs.D_Domain_Regs.PHYLockMask = 0x000B9670;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR4.dunitRegs.SDRAMCtrlRegs.D_Domain_Regs.DDR3Timing = 0x000B942c;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR4.dunitRegs.SDRAMCtrlRegs.D_Domain_Regs.DDRODTTimingHigh = 0x000B947c;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR4.dunitRegs.SDRAMCtrlRegs.D_Domain_Regs.SDRAMInitializationCtrl = 0x000B9480;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR4.dunitRegs.SDRAMCtrlRegs.D_Domain_Regs.DRAMPHYConfig = 0x000B95ec;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR4.dunitRegs.SDRAMCtrlRegs.D_Domain_Regs.DFS = 0x000B9528;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR4.dunitRegs.SDRAMCtrlRegs.D_Domain_Regs.DDRCtrllerODTCtrl = 0x000B949c;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR4.dunitRegs.SDRAMCtrlRegs.D_Domain_Regs.DDR3RegedDRAMTiming = 0x000B96d4;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR4.dunitRegs.SDRAMCtrlRegs.D_Domain_Regs.DDRIO = 0x000B9524;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR4.dunitRegs.SDRAMCtrlRegs.D_Domain_Regs.DDR3RankCtrl = 0x000B95e0;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR4.dunitRegs.SDRAMCtrlRegs.D_Domain_Regs.DDR3MR0 = 0x000B95d0;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR4.dunitRegs.SDRAMCtrlRegs.D_Domain_Regs.DDR3MR1 = 0x000B95d4;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR4.dunitRegs.SDRAMCtrlRegs.D_Domain_Regs.DDR3MR2 = 0x000B95d8;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR4.dunitRegs.SDRAMCtrlRegs.D_Domain_Regs.DDR3MR3 = 0x000B95dc;
                            {/*00B9874+n*0x10*/
                                GT_U32    n;
                                for(n = 0 ; n <= 3 ; n++) {
                                    SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR4.dunitRegs.SDRAMCtrlRegs.D_Domain_Regs.DDR3MR1CS[n] =
                                        0x00B9874+n*0x10;
                                }/* end of loop n */
                            }/*00B9874+n*0x10*/
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR4.dunitRegs.SDRAMCtrlRegs.D_Domain_Regs.ZQCConfig = 0x000B95e4;
                            {/*00B9870+n*0x10*/
                                GT_U32    n;
                                for(n = 0 ; n <= 3 ; n++) {
                                    SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR4.dunitRegs.SDRAMCtrlRegs.D_Domain_Regs.DDR3MR0CS[n] =
                                        0x00B9870+n*0x10;
                                }/* end of loop n */
                            }/*00B9870+n*0x10*/
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR4.dunitRegs.SDRAMCtrlRegs.D_Domain_Regs.ODPGCTRLCtrl    = 0x000B9600;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR4.dunitRegs.SDRAMCtrlRegs.D_Domain_Regs.DDRODTTimingLow = 0x000B9428;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR4.dunitRegs.SDRAMCtrlRegs.D_Domain_Regs.DRAMPHYLockStatus = 0x000B9674;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR4.dunitRegs.SDRAMCtrlRegs.D_Domain_Regs.PHYRegFileAccess = 0x000B96a0;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR4.dunitRegs.SDRAMCtrlRegs.D_Domain_Regs.DDR3RegedDRAMCtrl = 0x000B96d0;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR4.dunitRegs.SDRAMCtrlRegs.D_Domain_Regs.DDRCtrllerCtrlLow = 0x000B9404;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR4.dunitRegs.SDRAMCtrlRegs.D_Domain_Regs.dualDRAMCtrllerConfig = 0x000B96d8;

                        }/*end of unit D_Domain_Regs */


                    }/*end of unit SDRAMCtrlRegs */


                    {/*start of unit SDRAMDataProtectionAndErrorReport */
                        {/*start of unit R_Domain_Regs */
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR4.dunitRegs.SDRAMDataProtectionAndErrorReport.R_Domain_Regs.SDRAMErrorCtrl = 0x000B9454;

                        }/*end of unit R_Domain_Regs */


                        {/*start of unit D_Domain_Regs */
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR4.dunitRegs.SDRAMDataProtectionAndErrorReport.D_Domain_Regs.DDRCtrllerErrorInterruptCause = 0x000B94d0;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR4.dunitRegs.SDRAMDataProtectionAndErrorReport.D_Domain_Regs.DDRCtrllerErrorInterruptMask = 0x000B94d4;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR4.dunitRegs.SDRAMDataProtectionAndErrorReport.D_Domain_Regs.DDRCtrllerMsgInterruptCause = 0x000B94d8;
                            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.XSB_XBAR.DRAMCtrllerRegsDDR4.dunitRegs.SDRAMDataProtectionAndErrorReport.D_Domain_Regs.DDRCtrllerMsgInterruptMask = 0x000B94dc;

                        }/*end of unit D_Domain_Regs */


                    }/*end of unit SDRAMDataProtectionAndErrorReport */


                }/*end of unit dunitRegs */


            }/*end of unit DRAMCtrllerRegsDDR4 */


        }/*end of unit XSB_XBAR */


        {/*start of unit DFXServerUnitsBC2SpecificRegs */
            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.DFXServerUnitsBC2SpecificRegs.deviceResetCtrl = 0x000f800c;
            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.DFXServerUnitsBC2SpecificRegs.initializationStatusDone = 0x000f8014;
            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.DFXServerUnitsBC2SpecificRegs.configSkipInitializationMatrix = 0x000f8020;
            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.DFXServerUnitsBC2SpecificRegs.RAMInitSkipInitializationMatrix = 0x000f8030;
            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.DFXServerUnitsBC2SpecificRegs.trafficManagerSkipInitializationMatrix = 0x000f8040;
            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.DFXServerUnitsBC2SpecificRegs.tableSkipInitializationMatrix = 0x000f8060;
            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.DFXServerUnitsBC2SpecificRegs.SERDESSkipInitializationMatrix = 0x000f8064;
            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.DFXServerUnitsBC2SpecificRegs.EEPROMSkipInitializationMatrix = 0x000f8068;
            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.DFXServerUnitsBC2SpecificRegs.PCIeSkipInitializationMatrix = 0x000f806c;
            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.DFXServerUnitsBC2SpecificRegs.deviceSAR1 = 0x000f8200;
            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.DFXServerUnitsBC2SpecificRegs.deviceSAR2 = 0x000f8204;
            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.DFXServerUnitsBC2SpecificRegs.deviceCtrl4 = 0x000f8260;
            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.DFXServerUnitsBC2SpecificRegs.deviceCtrl5 = 0x000f8264;
            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.DFXServerUnitsBC2SpecificRegs.deviceCtrl8 = 0x000f8270;
            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.DFXServerUnitsBC2SpecificRegs.deviceCtrl10 = 0x000f8278;
            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.DFXServerUnitsBC2SpecificRegs.deviceCtrl12 = 0x000f8280;
            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.DFXServerUnitsBC2SpecificRegs.deviceCtrl21 = 0x000f82a4;
            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.DFXServerUnitsBC2SpecificRegs.deviceCtrl20 = 0x000f82a0;
            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.DFXServerUnitsBC2SpecificRegs.deviceCtrl19 = 0x000f829c;
            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.DFXServerUnitsBC2SpecificRegs.deviceCtrl18 = 0x000f8298;
            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.DFXServerUnitsBC2SpecificRegs.deviceCtrl15 = 0x000f828c;
            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.DFXServerUnitsBC2SpecificRegs.deviceCtrl14 = 0x000f8288;
            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.DFXServerUnitsBC2SpecificRegs.deviceStatus0 = 0x000f82b0;
            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.DFXServerUnitsBC2SpecificRegs.deviceStatus1 = 0x000f82b4;

            if(SMEM_CHT_IS_SIP5_20_GET(devObjPtr))
            {
                SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.DFXServerUnitsBC2SpecificRegs.deviceCtrl16 = 0x000f8290;
                SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.DFXServerUnitsBC2SpecificRegs.deviceCtrl32 = 0x000f8d20;
            }

            if(SMEM_CHT_IS_SIP6_GET(devObjPtr))
            {
                SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.DFXServerUnitsBC2SpecificRegs.deviceCtrl29 = 0x000F8D14;
                SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.DFXServerUnitsBC2SpecificRegs.deviceCtrl25 = 0x000F8D04;
            }
            if(SMEM_CHT_IS_SIP6_10_GET(devObjPtr))
            {
                SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.DFXServerUnitsBC2SpecificRegs.deviceCtrl17 = 0x000F8294;
            }
            if(SMEM_CHT_IS_SIP6_15_GET(devObjPtr))
            {
                SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.DFXServerUnitsBC2SpecificRegs.deviceCtrl33 = 0x000F8D24;
                SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.DFXServerUnitsBC2SpecificRegs.deviceCtrl34 = 0x000F8D28;
            }
            if(SMEM_CHT_IS_SIP6_20_GET(devObjPtr))
            {
                SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.DFXServerUnitsBC2SpecificRegs.deviceCtrl3 = 0x000f825C;
                SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.DFXServerUnitsBC2SpecificRegs.deviceCtrl6 = 0x000f8268;
                SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.DFXServerUnitsBC2SpecificRegs.deviceCtrl30 = 0x000f8d18;
            }
            if(SMEM_CHT_IS_SIP6_30_GET(devObjPtr))
            {
                SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.DFXServerUnitsBC2SpecificRegs.deviceCtrl48 = 0x000f8D60;
                SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.DFXServerUnitsBC2SpecificRegs.deviceCtrl49 = 0x000f8D64;
                SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.DFXServerUnitsBC2SpecificRegs.deviceCtrl50 = 0x000f8D68;
            }
        }/*end of unit DFXServerUnitsBC2SpecificRegs */


        {/*start of unit DFXServerUnits */
            {/*start of unit DFXServerRegs */
                {/*f812c+28*a*/
                    GT_U32    a;
                    for(a = 0 ; a <= 0 ; a++) {
                        SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.DFXServerUnits.DFXServerRegs.AVSDisabledCtrl2[a] =
                            0xf812c+28*a;
                    }/* end of loop a */
                }/*f812c+28*a*/
                {/*f8128+28*a*/
                    GT_U32    a;
                    for(a = 0 ; a <= 0 ; a++) {
                        SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.DFXServerUnits.DFXServerRegs.AVSDisabledCtrl1[a] =
                            0xf8128+28*a;
                    }/* end of loop a */
                }/*f8128+28*a*/
                {/*f8450+t*4*/
                    GT_U32    t;
                    for(t = 1 ; t <= 4 ; t++) {
                        SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.DFXServerUnits.DFXServerRegs.serverXBARTargetPortConfig[t-1] =
                            0xf8450+t*4;
                    }/* end of loop t */
                }/*f8450+t*4*/
                SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.DFXServerUnits.DFXServerRegs.serverStatus = 0x000f8010;
                SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.DFXServerUnits.DFXServerRegs.serverInterruptSummaryCause = 0x000f8100;
                SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.DFXServerUnits.DFXServerRegs.serverInterruptSummaryMask = 0x000f8104;
                SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.DFXServerUnits.DFXServerRegs.serverInterruptMask = 0x000f810c;
                SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.DFXServerUnits.DFXServerRegs.serverInterruptCause = 0x000f8108;
                SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.DFXServerUnits.DFXServerRegs.serverAddrSpace = 0x000f8018;
                SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.DFXServerUnits.DFXServerRegs.pipeSelect = 0x000f8000;

                if (isSip6 == GT_FALSE)
                {
                    SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.DFXServerUnits.DFXServerRegs.temperatureSensor28nmCtrlLSB = 0x000f8070;
                    SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.DFXServerUnits.DFXServerRegs.temperatureSensor28nmCtrlMSB = 0x000f8074;
                    SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.DFXServerUnits.DFXServerRegs.temperatureSensorStatus      = 0x000f8078;
                }
                else
                {
                    SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.DFXServerUnits.DFXServerRegs.temperatureSensorStatus      = 0x000f8078;
                    SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.DFXServerUnits.DFXServerRegs.temperatureSensor28nmCtrlLSB = 0x000f807c;
                    SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.DFXServerUnits.DFXServerRegs.temperatureSensor28nmCtrlMSB = 0x000f8080;
                }

                {/*f8134+28*a*/
                    GT_U32    a;
                    for(a = 0 ; a <= 0 ; a++) {
                        SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.DFXServerUnits.DFXServerRegs.AVSStatus[a] =
                            0xf8134+28*a;
                    }/* end of loop a */
                }/*f8134+28*a*/
                {/*f8138+28*a*/
                    GT_U32    a;
                    for(a = 0 ; a <= 0 ; a++) {
                        SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.DFXServerUnits.DFXServerRegs.AVSMinStatus[a] =
                            0xf8138+28*a;
                    }/* end of loop a */
                }/*f8138+28*a*/
                {/*f813c+28*a*/
                    GT_U32    a;
                    for(a = 0 ; a <= 0 ; a++) {
                        SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.DFXServerUnits.DFXServerRegs.AVSMaxStatus[a] =
                            0xf813c+28*a;
                    }/* end of loop a */
                }/*f813c+28*a*/
                {/*f8130+28*a*/
                    GT_U32    a;
                    for(a = 0 ; a <= 0 ; a++) {
                        SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.DFXServerUnits.DFXServerRegs.AVSEnabledCtrl[a] =
                            0xf8130+28*a;
                    }/* end of loop a */
                }/*f8130+28*a*/
                SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.DFXServerUnits.DFXServerRegs.snoopBusStatus = 0x000f8220;
                SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.DFXServerUnits.DFXServerRegs.deviceIDStatus = 0x000f8240;
                SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.DFXServerUnits.DFXServerRegs.JTAGDeviceIDStatus = 0x000f8244;

            }/*end of unit DFXServerRegs */


        }/*end of unit DFXServerUnits */


        if (isSip6 == GT_FALSE)
        {/*start of unit DFX_RAM_4_8[1] */
            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.DFX_RAM_4_8[0].errorsCntr = 0x00000004;
            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.DFX_RAM_4_8[0].erroneousAddr = 0x00000008;
            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.DFX_RAM_4_8[0].ECCSyndrome = 0x0000000c;
            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.DFX_RAM_4_8[0].erroneousSegment = 0x00000010;
            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.DFX_RAM_4_8[0].memoryCtrl = 0x00000014;
            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.DFX_RAM_4_8[0].memoryInterruptMask = 0x00000018;
            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.DFX_RAM_4_8[0].memoryInterruptCause = 0x0000001c;
        }/*end of unit DFX_RAM_4_8[1] */

    }/*end of unit resetAndInitCtrller */

}

/**
* @internal smemInternalPciRegsInfoSet function
* @endinternal
*
* @brief   Init memory module for 'internal PCI' (internal PEX) DB devices.
*
* @param[in] devObjPtr                - pointer to device object.
*/
void smemInternalPciRegsInfoSet
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr
)
{
    GT_U32  n,nMax;
    SMEM_INTERNAL_PCI_REGS_ADDR_STC *regsAddr = SMEM_CHT_MAC_REG_DB_INTERNAL_PCI_GET(devObjPtr);

    switch (devObjPtr->deviceType)
    {
        case SKERNEL_BOBK_ALDRIN:
        case SKERNEL_AC3X:
            devObjPtr->maxMppIndex = 33;
            break;
        default:
            devObjPtr->maxMppIndex = 32;/* BC2,Bobk,BC3 */
            break;
    }

    nMax = ((devObjPtr->maxMppIndex + 31)/32);

    for(n = 0 ; n < nMax ; n++)
    {
        regsAddr->Runit_RFU.GPIODataInPolarity[n] = 0x0001810C + 0x40*n;
        regsAddr->Runit_RFU.GPIODataIn[n] =         0x00018110 + 0x40*n;
        regsAddr->Runit_RFU.GPIOInterruptCause[n] = 0x00018114 + 0x40*n;
        regsAddr->Runit_RFU.GPIOInterruptMask[n]  = 0x00018118 + 0x40*n;
        regsAddr->Runit_RFU.GPIOInterruptLevelMask[n]  = 0x0001811C + 0x40*n;
    }
}

/**
* @internal updateGopOldDbWithPipes_Ravens function
* @endinternal
*
* @brief   add the 'pipe offset' to registers of GOP (per port).
*
* @param[in] devObjPtr                - pointer to device object.
*            gopBaseAddress           - GOP base address (UNIT_GOP or UNIT_BASE_RAVEN_0)
*/
static void updateGopOldDbWithPipes_Ravens(
    IN  SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN  GT_U32 gopBaseAddress
)
{
    GT_U32  globalPortNum  ;
    GT_U32  tileOffset,tileId,pipeId,pipeOffset;
    /* indication of unit in pipe 1*/
    /* the offset of second pipe units */
    GT_U32  gop_pipeOffset = SMEM_UNIT_PIPE_OFFSET_GET_MAC(devObjPtr, gopBaseAddress, NULL);
    GT_U32  localPipeId;
    GT_U32  globalRavenNum;
    GT_U32  isCpuPort;

    for (globalPortNum = 0;
         globalPortNum < devObjPtr->portsNumber;
         globalPortNum++)
    {
        smemConvertGlobalPortToCurrentPipeId(devObjPtr,globalPortNum,NULL);

        pipeId = smemGetCurrentPipeId(devObjPtr);

        if(pipeId == 0)
        {
            /* skip ports in pipe 0 */
            continue;
        }

        if(devObjPtr->numOfTiles >= 2)
        {
            /* convert the global pipeId to local pipeId */
            localPipeId = smemConvertGlobalPipeIdToTileAndLocalPipeIdInTile(devObjPtr,pipeId,NULL);

            tileId = pipeId / devObjPtr->numOfPipesPerTile;
            tileOffset = tileId > 0 ? (devObjPtr->tileOffset * tileId) : 0;
            if(0 == localPipeId)
            {
                pipeOffset = 0;/* only tile offset */
            }
            else
            {
                pipeOffset = gop_pipeOffset;/* tile offset and pipe offset */
            }
        }
        else
        {
            tileOffset = 0;
            pipeOffset = gop_pipeOffset;
        }

        /* Convert physical port to Raven port, channel and die indexes */
        smemRavenGlobalPortConvertGet(devObjPtr,globalPortNum,&globalRavenNum,NULL,NULL,NULL,NULL,&isCpuPort);

        offsetAddrSet(SET_UNIT_AND_SIZE_MAC(SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->GOP_PTP.PTP[globalPortNum])/*unit name*/,
            pipeOffset+tileOffset);

        offsetAddrSet(SET_UNIT_AND_SIZE_MAC(SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI[globalPortNum])/*unit name*/,
            pipeOffset+tileOffset);

        if ((globalPortNum % 8) == 0 && !isCpuPort)
        {
            /* Set offset per ports per channel (1 for 8 channel ports */
            offsetAddrSet(SET_UNIT_AND_SIZE_MAC(SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI_EXTERNAL[globalPortNum/8])/*unit name*/,
                pipeOffset+tileOffset);

/*                        gopMtiPrint(devObjPtr, globalPortNum); */
        }

        offsetAddrSet(SET_UNIT_AND_SIZE_MAC(SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_high_speed[globalPortNum])/*unit name*/,
            pipeOffset+tileOffset);

        if(isCpuPort)
        {
            offsetAddrSet(SET_UNIT_AND_SIZE_MAC(SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI_cpu_port[globalRavenNum])/*unit name*/,
                pipeOffset+tileOffset);
        }
    }

    /* restore */
    smemSetCurrentPipeId(devObjPtr,0);
}

/**
* @internal updateGopOldDbWithPipes function
* @endinternal
*
* @brief   add the 'pipe offset' to registers of GOP (per port).
*
* @param[in] devObjPtr                - pointer to device object.
*            gopBaseAddress           - GOP base address (UNIT_GOP or UNIT_BASE_RAVEN_0)
*/
static void updateGopOldDbWithPipes(
    IN  SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN  GT_U32 gopBaseAddress
)
{
    GT_U32  maxloop        = devObjPtr->numOfPortsPerPipe + 1;
    GT_U32  globalPortNum  ;
    GT_U32  ii;
    GT_U32  tileOffset,tileId,pipeId,pipeOffset;
    /* indication of unit in pipe 1*/
    /* the offset of second pipe units */
    GT_U32  gop_pipeOffset = SMEM_UNIT_PIPE_OFFSET_GET_MAC(devObjPtr, gopBaseAddress, NULL);
    GT_U32  localPipeId;

    if(devObjPtr->numOfRavens)
    {
        updateGopOldDbWithPipes_Ravens(devObjPtr,gopBaseAddress);
        return;
    }

    for (pipeId = 1; pipeId < devObjPtr->numOfPipes; pipeId++)
    {
        smemSetCurrentPipeId(devObjPtr,pipeId /*currentPipeId*/);

        if(devObjPtr->numOfTiles >= 2)
        {
            /* convert the global pipeId to local pipeId */
            localPipeId = smemConvertGlobalPipeIdToTileAndLocalPipeIdInTile(devObjPtr,pipeId,NULL);

            tileId = pipeId / devObjPtr->numOfPipesPerTile;
            tileOffset = tileId > 0 ? (devObjPtr->tileOffset * tileId) : 0;
            if(0 == localPipeId)
            {
                pipeOffset = 0;/* only tile offset */
            }
            else
            {
                pipeOffset = gop_pipeOffset;/* tile offset and pipe offset */
            }
        }
        else
        {
            tileOffset = 0;
            pipeOffset = gop_pipeOffset;
        }

        for(ii = 0 ; ii < maxloop ; ii++)
        {
            smemConvertCurrentPipeIdAndLocalPortToGlobal(devObjPtr,ii,GT_TRUE,&globalPortNum);
            if(globalPortNum >= devObjPtr->portsNumber)
            {
                /* do not copy this port.
                   In Falcon pipes 3..max not hold this port.
                */
                continue;
            }

            if (SMAIN_NOT_VALID_CNS != UNIT_BASE_ADDR_GET_ALLOW_NON_EXIST_UNIT_MAC(devObjPtr,UNIT_GOP) &&
                gopBaseAddress == UNIT_BASE_ADDR_GET_MAC(devObjPtr, UNIT_GOP))
            {
                    offsetAddrSet(SET_UNIT_AND_SIZE_MAC(SMEM_CHT_MAC_REG_DB_GET(devObjPtr)->GOP.ports.gigPort[globalPortNum])/*unit name*/,
                        pipeOffset+tileOffset);

                    offsetAddrSet(SET_UNIT_AND_SIZE_MAC(SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->GOP_PTP.PTP[globalPortNum])/*unit name*/,
                        pipeOffset+tileOffset);

                    offsetAddrSet(SET_UNIT_AND_SIZE_MAC(SMEM_CHT_MAC_REG_DB_GET(devObjPtr)->GOP.ports.XLGIP[globalPortNum])/*unit name*/,
                        pipeOffset+tileOffset);

                    offsetAddrSet(SET_UNIT_AND_SIZE_MAC(SMEM_CHT_MAC_REG_DB_GET(devObjPtr)->GOP.ports.MPCSIP[globalPortNum])/*unit name*/,
                        pipeOffset+tileOffset);

                    offsetAddrSet(SET_UNIT_AND_SIZE_MAC(SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->GOP.ports_100G_CG_MAC[globalPortNum])/*unit name*/,
                        pipeOffset+tileOffset);

                    offsetAddrSet(SET_UNIT_AND_SIZE_MAC(SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->GOP.ports_100G_CG_PCS[globalPortNum])/*unit name*/,
                        pipeOffset+tileOffset);
            }
        }
    }

    /* restore */
    smemSetCurrentPipeId(devObjPtr,0);
}

/**
* @internal smemCheetahInternalSimulationUseMemForSip_5_15_RxdmaPipGlobalCountersIncrement function
* @endinternal
*
* @brief   RXDMA: sip 5_15 :
*         function to increment the shadow of the 'per DP unit' 3 counters of 64 bits
*/
void smemCheetahInternalSimulationUseMemForSip_5_15_RxdmaPipGlobalCountersIncrement
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32                  rxDmaUnit,
    IN GT_U32                  counterId
)
{
    GT_U32      *internalMemPtr;
    GT_U32      shadowIndex;

    internalMemPtr = CHT_INTERNAL_MEM_PTR(devObjPtr,
        CHT_INTERNAL_SIMULATION_USE_MEM_FOR_SIP_5_15_RXDMA_PIP_GLOBAL_COUNTERS_START_E);

    shadowIndex =  counterId * 2/*lsb/msb*/ + (rxDmaUnit * 3/*low/mid/high*/ * 2/*lsb/msb*/);

    __LOG_NO_LOCATION_META_DATA(("PIP : SIP_5_15 : priority [%d] shadow counter : Increment shadow of PIP drop Counter from [0x%x](high32)[0x%x](low32) \n",
        counterId,internalMemPtr[shadowIndex+1],internalMemPtr[shadowIndex]));

    /* update the LSB*/
    internalMemPtr[shadowIndex]++;
    if(internalMemPtr[shadowIndex] == 0)
    {
        /* update the MSB*/
        internalMemPtr[shadowIndex+1]++;
    }

}

/**
* @internal smemCheetahInternalSimulationUseMemForSip_5_20_RxdmaPipGlobalCountersIncrement function
* @endinternal
*
* @brief   RXDMA: sip 5_20 :
*         function to increment the shadow of the 'per DP unit' 3 counters of 64 bits
*/
void smemCheetahInternalSimulationUseMemForSip_5_20_RxdmaPipGlobalCountersIncrement
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32                  rxDmaUnit,
    IN GT_U32                  counterId
)
{
    GT_U32      *internalMemPtr;
    GT_U32      shadowIndex;

    internalMemPtr = CHT_INTERNAL_MEM_PTR(devObjPtr,
        CHT_INTERNAL_SIMULATION_USE_MEM_FOR_SIP_5_15_RXDMA_PIP_GLOBAL_COUNTERS_START_E);

    shadowIndex = counterId * 2/*lsb/msb*/ +
        (rxDmaUnit * 4 /*low/mid/high/very high*/ * 2/*lsb/msb*/);

    __LOG_NO_LOCATION_META_DATA(("PIP : SIP_5_20 : priority [%d] "
        "shadow counter : Increment shadow of PIP drop Counter "
        "from [0x%x](high32)[0x%x](low32) \n",
        counterId,internalMemPtr[shadowIndex+1],internalMemPtr[shadowIndex]));

    /* update the LSB*/
    internalMemPtr[shadowIndex]++;
    if(internalMemPtr[shadowIndex] == 0)
    {
        /* update the MSB*/
        internalMemPtr[shadowIndex+1]++;
    }
}

/**
* @internal smemCheetahInternalSimulationUseMemForSip_5_15_RxdmaPipGlobalCountersCopyToRealCounterAndResetShadow function
* @endinternal
*
* @brief   RXDMA: sip 5_15 and 5_20:
*         function to copy from the shadow of the 'per DP unit' ALL 3 (or 4) counters of 64 bits ,
*         into the actual counters memory , and reset the shadow
*/
void smemCheetahInternalSimulationUseMemForSip_5_15_RxdmaPipGlobalCountersCopyToRealCounterAndResetShadow
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32                  rxDmaUnit
)
{
    GT_U32  pipPriority;
    GT_U32  regAddr_msb,regAddr_lsb;
    GT_U32      *internalMemPtr;
    GT_U32      shadowIndex;
    GT_U32  priorityCount;

    internalMemPtr = CHT_INTERNAL_MEM_PTR(devObjPtr,
        CHT_INTERNAL_SIMULATION_USE_MEM_FOR_SIP_5_15_RXDMA_PIP_GLOBAL_COUNTERS_START_E);

    if(SMEM_CHT_IS_SIP5_20_GET(devObjPtr))
    {
        priorityCount = 4; /*low/mid/very high*/
    }
    else
    {
        priorityCount = 3; /*low/mid/high*/
    }

    shadowIndex =  (rxDmaUnit * priorityCount * 2/*lsb/msb*/);

    /* start of the counters of this rxDMA */
    internalMemPtr += shadowIndex;

    for(pipPriority = 0 ; pipPriority < priorityCount ; pipPriority++ , internalMemPtr+=2)
    {
        if(SMEM_CHT_IS_SIP5_20_GET(devObjPtr))
        {
            /* get into regAddr_msb,regAddr_lsb the addresses of 'rxdma pip global drop counters' (per pipPriority)*/
            SMEM_CHEETAH_SIP_5_20_RXDMA_PIP_DROP_GLOBAL_COUNTER_ADDR_GET(devObjPtr,rxDmaUnit,pipPriority,regAddr_msb,regAddr_lsb);

        }
        else
        {
            /* get into regAddr_msb,regAddr_lsb the addresses of 'rxdma pip global drop counters' (per pipPriority)*/
            SMEM_CHEETAH_SIP_5_10_RXDMA_PIP_DROP_GLOBAL_COUNTER_ADDR_GET(devObjPtr,rxDmaUnit,pipPriority,regAddr_msb,regAddr_lsb);
        }

        __LOG_NO_LOCATION_META_DATA(("PIP : priority [%d] COPY counter from shadow drop Counter : [0x%x](high32)[0x%x](low32) \n",
            pipPriority,internalMemPtr[1],internalMemPtr[0]));
        /* copy the shadow counters of this priority to actual registers */
        smemRegSet(devObjPtr, regAddr_lsb, internalMemPtr[0]);
        smemRegSet(devObjPtr, regAddr_msb, internalMemPtr[1]);

        __LOG_NO_LOCATION_META_DATA(("PIP : priority [%d] RESET shadow drop Counter \n",
            pipPriority));

        /* reset the shadow counters of this priority */
        internalMemPtr[0] = 0;
        internalMemPtr[1] = 0;
    }
}
/**
* @internal smemCheetahNumGopPortsInPipe function
* @endinternal
*
* @brief   Get number of (GOP) ports in pipe
*/
GT_U32 smemCheetahNumGopPortsInPipe
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr
)
{
    if(devObjPtr->numOfPipes >= 2)/* BC3,Falcon */
    {
        return devObjPtr->numOfPortsPerPipe + 1;
    }

    return devObjPtr->portsNumber;
}


GT_STATUS sim_printPtpFirstRegister
(
    IN GT_U8    deviceNumber
)
{
    SKERNEL_DEVICE_OBJECT * devObjPtr;
    GT_U32  ii,addr;

    DEVICE_ID_CHECK_MAC(deviceNumber);

    printf("PTP ports addresses \n");
    printf("\n");

    printf("port      address   \n");
    printf("====================\n");

    devObjPtr = smemTestDeviceIdToDevPtrConvert(deviceNumber);
    for(ii = 0 ; ii < devObjPtr->portsNumber ; ii++)
    {
        if(devObjPtr->portsArr[ii].state == SKERNEL_PORT_STATE_NOT_EXISTS_E)
        {
            continue;
        }
        addr = SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->GOP_PTP.PTP[ii].PTPInterruptCause;

        if (0xFFFFFFFF == addr)
        {
            continue;
        }
        printf("[%d] [0x%8.8x]\n",ii,addr);
    }

    printf("The end \n");

    return GT_OK;
}

