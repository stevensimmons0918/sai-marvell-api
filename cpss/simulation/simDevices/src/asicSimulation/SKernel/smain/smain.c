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
* @file smain.c
*
* @brief The module is SKERNEL API and SMain functions
*
* @version   170
********************************************************************************
*/
#include <os/simTypesBind.h>
#include <asicSimulation/SCIB/scib.h>
#include <asicSimulation/SKernel/smain/smain.h>
#include <asicSimulation/SKernel/skernel.h>
#include <asicSimulation/SKernel/smem/smem.h>
#include <asicSimulation/SKernel/sfdb/sfdb.h>
#include <asicSimulation/SKernel/suserframes/snet.h>
#include <asicSimulation/SKernel/suserframes/snetCheetahL2.h>
#include <asicSimulation/SKernel/suserframes/snetLion2TrafficGenerator.h>
#include <asicSimulation/SKernel/suserframes/snetLion2Oam.h>
#include <asicSimulation/SKernel/twistCommon/sregTwist.h>
#include <asicSimulation/SKernel/smem/smemGM.h>
#include <asicSimulation/SKernel/smem/smemCheetah.h>
#include <common/SBUF/sbuf.h>
#include <common/SQue/squeue.h>
#include <gmSimulation/GM/TrafficAPI.h>
#include <gmSimulation/GM/GMApi.h>
#include <asicSimulation/SInit/sinit.h>
#include <asicSimulation/SDistributed/sdistributed.h>
#include <common/SHOST/GEN/INTR/EXP/INTR.H>
#include <asicSimulation/SKernel/sfdb/sfdbCheetah.h>
#include <asicSimulation/SKernel/cheetahCommon/sregCheetah.h>
#include <asicSimulation/SLog/simLog.h>
#include <asicSimulation/SLog/simLogInfoTypePacket.h>
#include <asicSimulation/SKernel/smain/sRemoteTm.h>
#include <asicSimulation/SKernel/suserframes/snetCheetahEgress.h>
#include <asicSimulation/wmApi.h>
#include <asicSimulation/SEmbedded/simFS.h>

/**************************** defines *****************************************/
#define SET_TASK_TYPE_MAC(newTaskType)                                  \
    {                                                                   \
        taskType = newTaskType;                                         \
        /* use cookiePtr as NULL , to not override first initialization ! */ \
        SIM_OS_MAC(simOsTaskOwnTaskPurposeSet)(taskType, NULL);         \
    }

#define FILE_MAX_LINE_LENGTH_CNS (4*SIM_OS_CNF_FILE_MAX_LINE_LENGTH_CNS)

/* minimum size of buffer for sprintf */
#define SMAIN_MIN_SIZE_OF_BUFFER_CNS 100

/* number of buffers in the pool */
#define SMAIN_BUFFERS_NUMBER_CNS                    (512 + 128)

/* end of file */
#define SMAIN_FILE_RECORDS_EOF                  0xffffffff

/* header length for NIC outPut */
#define SMAIN_NIC_TX_HEADER_LENGTH                  14

/* Aging task polling time in milliseconds */
#define SMAIN_AGING_POLLING_TIME_CNS                 5000

/* slan type */
#define SAGNTP_SLANUP_CNS            (0x400 + 3)
#define SAGNTP_SLANDN_CNS            (0x400 + 4)

#define SMAIN_NO_GM_DEVICE                  0xFF

#define SMAIN_PCI_FLAG_BUFF_SIZE            0x10

#define DEBUG_SECTION_CNS   "debug"
/* indication of number of packets that are currently in the system */
static GT_U32   skernelNumOfPacketsInTheSystem = 0;

/* number of devices in the simulation */
extern GT_U32       smainDevicesNumber;/* definition move to sinit.c */

/*are we under shutdown */
static GT_BOOL  skernelUnderShutDown    = GT_FALSE;

/* the last deviceId that was used for cores --> will be initialized to smainDevicesNumber
   in function skernelInit */
static GT_U32       lastDeviceIdUsedForCores = 0;

/* mutex to be used by all the devices to protect the LOG info full of processing of the packet
   in multi-device/multi-cores system */
GT_MUTEX LOG_fullPacketWalkThroughProtectMutex;

/* Init status of GM Traffic Manager */
GT_U32       smainGmTrafficInitDone = 0;

/* User debug info */
SKERNEL_USER_DEBUG_INFO_STC skernelUserDebugInfo;

/* allow the Puma3 to do 'external' mac loopback as processing the loop in the
   context of the sending task (not sending it to the 'buffers pool') */
static GT_U32   supportMacLoopbackInContextOfSender = 1;
/* the minimal number of buffers that should be kept , and not allowed for 'loopback port'*/
#define LOOPBACK_MIN_FREE_BUFFERS_CNS   100

extern GT_U32 simulationSoftResetDone;

/* the active device that is in the Firmware */
/* used by the FW for 'LOG info' */
SKERNEL_DEVICE_OBJECT * current_fw_devObjPtr = NULL;
GT_U32                  current_fw_ThreadId  = 0;


/**
* @struct SMAIN_DEV_NAME_STC
 *
 * @brief Describe device specific information.
*/
typedef struct{

    GT_U32 gmDevType;

    char * devName;

    /** : Device type. */
    SKERNEL_DEVICE_TYPE devType;

    /** : Number of MAC ports. */
    GT_U32 portNum;

    /** @brief > for SLANS
     *  Comments:
     */
    GT_U32 cpuPortNum;

} SMAIN_DEV_NAME_STC;

/*************************** Local Variables **********************************/

/* macro PORTS_BMP_IS_PORT_SET_MAC
    to check if port is set the bmp of ports (is corresponding bit is 1)

  portsBmpPtr - of type CPSS_PORTS_BMP_STC*
                pointer to the ports bmp
  portNum - the port num to set in the ports bmp

  return 0 -- port not set in bmp
  return 1 -- port set in bmp
*/
#define PORTS_BMP_IS_PORT_SET_MAC(portsBmpPtr,portNum)   \
    (((portsBmpPtr)->ports[(portNum)>>5] & (1 << ((portNum)& 0x1f)))? 1 : 0)

#define BMP_CONTINUES_PORTS_MAC(x)  (((x)!=32) ? ((1<<(x)) - 1) : 0xFFFFFFFF)

/* bmp of ports 32..63 -- physical ports (not virtual) */
#define BMP_PORTS_32_TO_63_MAC(portsBmpPtr, numPorts) \
    (portsBmpPtr)->ports[0] = 0xFFFFFFFF;    \
    (portsBmpPtr)->ports[1] = BMP_CONTINUES_PORTS_MAC(numPorts % 32);    \
    (portsBmpPtr)->ports[2] = 0;    \
    (portsBmpPtr)->ports[3] = 0

/* bmp of ports 64..95 -- physical ports (not virtual) */
#define BMP_PORTS_64_TO_95_MAC(portsBmpPtr, numPorts) \
    (portsBmpPtr)->ports[0] = 0xFFFFFFFF;    \
    (portsBmpPtr)->ports[1] = 0xFFFFFFFF;    \
    (portsBmpPtr)->ports[2] = BMP_CONTINUES_PORTS_MAC(numPorts % 32);    \
    (portsBmpPtr)->ports[3] = 0

/* bmp of less than 32 ports -- physical ports (not virtual) */
#define BMP_PORTS_LESS_32_MAC(portsBmpPtr,numPorts)    \
    (portsBmpPtr)->ports[0] = BMP_CONTINUES_PORTS_MAC(numPorts);  \
    (portsBmpPtr)->ports[1] = 0; \
    (portsBmpPtr)->ports[2] = 0; \
    (portsBmpPtr)->ports[3] = 0

/* macro to set range of ports in bmp --
   NOTE :  it must be (startPort + numPorts) <= 31
*/
#define PORT_RANGE_MAC(startPort,numPorts)\
    (BMP_CONTINUES_PORTS_MAC(numPorts) << startPort)

/**
* @struct PORTS_BMP_STC
 *
 * @brief Defines the bmp of ports (up to 63 ports)
*/
typedef struct{

    GT_U32 ports[(SKERNEL_DEV_MAX_SUPPORTED_PORTS_CNS + 31)/32];

} PORTS_BMP_STC;

static const PORTS_BMP_STC portsBmp24_25_27 = {{(PORT_RANGE_MAC(24,2) |
                                                      PORT_RANGE_MAC(27,1)) ,0}};

static const PORTS_BMP_STC portsBmp0to23_27 = {{(PORT_RANGE_MAC(0,24) |
                                                      PORT_RANGE_MAC(27,1)) ,0}};

static const PORTS_BMP_STC portsBmpCh3_8_Xg = {{(PORT_RANGE_MAC(24,4) |
                                                      PORT_RANGE_MAC(12,1) |
                                                      PORT_RANGE_MAC(10,1) |
                                                      PORT_RANGE_MAC(4 ,1) |
                                                      PORT_RANGE_MAC(0 ,1)) ,0}};

static const PORTS_BMP_STC portsBmpCh3Xg =    {{(PORT_RANGE_MAC(24,4) |
                                                      PORT_RANGE_MAC(22,1) |
                                                      PORT_RANGE_MAC(16,1) |
                                                      PORT_RANGE_MAC(12,1) |
                                                      PORT_RANGE_MAC(10,1) |
                                                      PORT_RANGE_MAC(4 ,1) |
                                                      PORT_RANGE_MAC(0 ,1)) ,0}};

static const PORTS_BMP_STC portsBmp0to3_24_25 = {{(PORT_RANGE_MAC(0,4) |
                                                        PORT_RANGE_MAC(24,2)) ,0}};

static const PORTS_BMP_STC portsBmp0to7_24_25 = {{(PORT_RANGE_MAC(0,8) |
                                                        PORT_RANGE_MAC(24,2)) ,0}};

static const PORTS_BMP_STC portsBmp0to15_24_25 = {{(PORT_RANGE_MAC(0,16) |
                                                         PORT_RANGE_MAC(24,2)) ,0}};

static const PORTS_BMP_STC portsBmp0to15_24to27 = {{(PORT_RANGE_MAC(0,16) |
                                                          PORT_RANGE_MAC(24,4)) ,0}};

static const PORTS_BMP_STC portsBmp56to59and62and64to71 = {{
    0/*0..31*/ ,
    (PORT_RANGE_MAC((56%32),4) /*56..59*/ | PORT_RANGE_MAC((62%32),1)) ,
     PORT_RANGE_MAC((64%32),8),/*64..71*/
    0
}};

/********************/
/* multi mode ports */
/********************/
static const PORTS_BMP_STC portsBmpMultiState_24to27 = {{PORT_RANGE_MAC(24,4) ,0}};
static const PORTS_BMP_STC portsBmpMultiState_24to29 = {{PORT_RANGE_MAC(24,6) ,0}};
static const PORTS_BMP_STC portsBmpMultiState_0to11 = {{PORT_RANGE_MAC(0,12) ,0}};

static const PORTS_BMP_STC portsBmpMultiState_0to11_32to47 = {{PORT_RANGE_MAC(0,12) ,PORT_RANGE_MAC(0,12)}};
static const PORTS_BMP_STC portsBmpMultiState_48to71 = {{0, 0xFFFF0000, 0x000000FF, 0}};
static const PORTS_BMP_STC portsBmpMultiState_0to73  = {{0xFFFFFFFF, 0xFFFFFFFF, 0x000003FF, 0}};
static const PORTS_BMP_STC portsBmpMultiState_0to72  = {{0xFFFFFFFF, 0xFFFFFFFF, 0x000001FF, 0}};
static const PORTS_BMP_STC portsBmpMultiState_0to31  = {{0xFFFFFFFF, 0, 0, 0}};

static const PORTS_BMP_STC portsBmpMultiState_0to15  = {{0x0000FFFF, 0, 0, 0}};

/* cheetah with 10 ports */
#define  CH_10_DEVICES_MAC       \
SKERNEL_98DX107                           ,\
SKERNEL_98DX106                           ,\
SKERNEL_DXCH_B0

/* cheetah with 27 ports */
#define  CH_27_DEVICES_MAC       \
SKERNEL_98DX270                           ,\
SKERNEL_98DX273                           ,\
SKERNEL_98DX803                           ,\
SKERNEL_98DX249                           ,\
SKERNEL_98DX269                           ,\
SKERNEL_98DX169                           ,\
SKERNEL_DXCH

/* cheetah with 26 ports */
#define  CH_26_DEVICES_MAC       \
SKERNEL_98DX260                           ,\
SKERNEL_98DX263                           ,\
SKERNEL_98DX262                           ,\
SKERNEL_98DX268

/* cheetah with 24 ports */
#define  CH_24_DEVICES_MAC       \
SKERNEL_98DX250                           ,\
SKERNEL_98DX253                           ,\
SKERNEL_98DX243                           ,\
SKERNEL_98DX248

/* cheetah with 25 ports */
#define  CH_25_DEVICES_MAC       \
SKERNEL_98DX133

/* cheetah with 16 ports */
#define  CH_16_DEVICES_MAC       \
SKERNEL_98DX163                           ,\
SKERNEL_98DX166                           ,\
SKERNEL_98DX167

/* cheetah-2 with 28 ports */
#define  CH2_28_DEVICES_MAC       \
SKERNEL_98DX285                           ,\
SKERNEL_98DX804                           ,\
SKERNEL_DXCH2

/* cheetah-2 with 27 ports */
#define  CH2_27_DEVICES_MAC       \
SKERNEL_98DX275

/* cheetah-2 with 26 ports */
#define  CH2_26_DEVICES_MAC       \
SKERNEL_98DX265                         ,\
SKERNEL_98DX145

/* cheetah-2 with 24 ports */
#define  CH2_24_DEVICES_MAC       \
SKERNEL_98DX255

/* cheetah-2 with 12 ports */
#define  CH2_12_DEVICES_MAC       \
SKERNEL_98DX125

/* cheetah-3 with 24 ports ports */
#define  CH3_24_DEVICES_MAC       \
SKERNEL_98DX5124

/* cheetah-3 with 24 ports plus 2 XG ports */
#define  CH3_26_DEVICES_MAC       \
SKERNEL_98DX5126                       ,\
SKERNEL_98DX5127

/* cheetah-3 with 24 ports plus 4 XG ports */
#define  CH3_28_DEVICES_MAC       \
SKERNEL_98DX5128                       ,\
SKERNEL_98DX5128_1                     ,\
SKERNEL_98DX287                        ,\
SKERNEL_98DX5129                       ,\
SKERNEL_DXCH3                          ,\
SKERNEL_98DX287   /*ch3p*/             ,\
SKERNEL_DXCH3P

/* cheetah-3 with 4 1/2.5 Gigabit Ethernet ports + 2XG */
#define  CH3_6_DEVICES_MAC       \
SKERNEL_98DX5151

/* cheetah-3 with 8 1/2.5 Gigabit Ethernet ports + 2XG */
#define  CH3_10_DEVICES_MAC       \
SKERNEL_98DX5152

/* cheetah-3 with 16 1/2.5 Gigabit Ethernet ports + 2XG */
#define  CH3_18_DEVICES_MAC       \
SKERNEL_98DX5154

/* cheetah-3 with 16 1/2.5 Gigabit Ethernet ports + 2XG + 2XGS */
#define  CH3_20_DEVICES_MAC       \
SKERNEL_98DX5155

/* cheetah-3 with 24 1/2.5 Gigabit Ethernet ports + 2XG*/
#define  CH3_24_2_DEVICES_MAC       \
SKERNEL_98DX5156

/* cheetah-3 with 24 1/2.5 Gigabit Ethernet ports + 2XG + 2XGS*/
#define  CH3_24_4_DEVICES_MAC       \
SKERNEL_98DX5157

/* cheetah-3 with 8 XG ports: 8 XG or 6XG + 2 Stack */
#define  CH3_8_XG_DEVICES_MAC       \
SKERNEL_98DX8108                       ,\
SKERNEL_98DX8109

/* cheetah-3 with 10 XG ports: 6 XG + 4 Stack */
#define  CH3_XG_DEVICES_MAC       \
SKERNEL_98DX8110                       ,\
SKERNEL_98DX8110_1                     ,\
SKERNEL_DXCH3_XG

/* Salsa with 24 ports */
#define  SALSA_24_DEVICES_MAC       \
SKERNEL_98DX240                       ,\
SKERNEL_98DX241

/* Salsa with 16 ports */
#define  SALSA_16_DEVICES_MAC       \
SKERNEL_98DX160                       ,\
SKERNEL_98DX161

/* Salsa with 12 ports */
#define  SALSA_12_DEVICES_MAC       \
SKERNEL_98DX120                       ,\
SKERNEL_98DX121

/* Twist-C with 10 ports */
#define  TWISTC_10_DEVICES_MAC     \
SKERNEL_98MX620B

/* Twist-C with 52 ports */
#define  TWISTC_52_DEVICES_MAC     \
SKERNEL_98MX610B                          ,\
SKERNEL_98MX610BS                         ,\
SKERNEL_98EX110BS                         ,\
SKERNEL_98EX111BS                         ,\
SKERNEL_98EX112BS                         ,\
SKERNEL_98EX110B                          ,\
SKERNEL_98EX111B                          ,\
SKERNEL_98EX112B

/* Twist-C with 12 ports */
#define  TWISTC_12_DEVICES_MAC     \
SKERNEL_98EX120B                          ,\
SKERNEL_98EX120B_                         ,\
SKERNEL_98EX121B                          ,\
SKERNEL_98EX122B                          ,\
SKERNEL_98EX128B                          ,\
SKERNEL_98EX129B

/* Twist-D with 52 ports */
#define  TWISTD_52_DEVICES_MAC     \
SKERNEL_98EX100D                          ,\
SKERNEL_98EX110D                          ,\
SKERNEL_98EX115D                          ,\
SKERNEL_98EX110DS                         ,\
SKERNEL_98EX115DS

/* Twist-D with 12 ports */
#define  TWISTD_12_DEVICES_MAC     \
SKERNEL_98EX120D                          ,\
SKERNEL_98EX125D

/* Twist-D with 1XG ports */
#define  TWISTD_1_DEVICES_MAC      \
SKERNEL_98EX130D                          ,\
SKERNEL_98EX135D


/* Samba with 52 ports */
#define  SAMBA_52_DEVICES_MAC      \
SKERNEL_98MX615D                           ,\
SKERNEL_98MX618

/* Samba with 12 ports */
#define  SAMBA_12_DEVICES_MAC      \
SKERNEL_98MX625D                           ,\
SKERNEL_98MX625V0                         ,\
SKERNEL_98MX628

/* Samba with 1XG ports */
#define  SAMBA_1_DEVICES_MAC       \
SKERNEL_98MX635D                           ,\
SKERNEL_98MX638


/* Tiger with 52 ports */
#define  TIGER_52_DEVICES_MAC      \
SKERNEL_98EX116                           ,\
SKERNEL_98EX106                           ,\
SKERNEL_98EX108                           ,\
SKERNEL_98EX116DI

/* Tiger with 12 ports */
#define  TIGER_12_DEVICES_MAC      \
SKERNEL_98EX126                           ,\
SKERNEL_98EX126V0                         ,\
SKERNEL_98EX126DI

/* Tiger with 1XG ports */
#define  TIGER_1_DEVICES_MAC       \
SKERNEL_98EX136                           ,\
SKERNEL_98EX136DI

/* Puma2 devices , 24 ports */
#define PUMA_24_DEVICES_MAC      \
SKERNEL_98EX2106                         ,\
SKERNEL_98EX2206                         ,\
SKERNEL_98MX2306                         ,\
SKERNEL_98EX8261                         ,\
SKERNEL_98EX8301                         ,\
SKERNEL_98EX8303                         ,\
SKERNEL_98EX8501                         ,\
SKERNEL_98EX240                          ,\
SKERNEL_98EX240_1                        ,\
SKERNEL_98MX840
/* Puma2 devices : 2 port XG devices */
#define PUMA_26_DEVICES_MAC      \
SKERNEL_98EX2110                         ,\
SKERNEL_98EX2210                         ,\
SKERNEL_98MX2310                         ,\
SKERNEL_98EX8262                         ,\
SKERNEL_98EX8302                         ,\
SKERNEL_98EX8305                         ,\
SKERNEL_98EX8502                         ,\
SKERNEL_98EX260                          ,\
SKERNEL_98MX860

#define XCAT_8FE_2STACK_PORTS_DEVICES \
SKERNEL_98DX1101,                     \
SKERNEL_98DX2101

#define XCAT_16FE_2STACK_PORTS_DEVICES \
SKERNEL_98DX1111

#define XCAT_16FE_4STACK_PORTS_DEVICES \
SKERNEL_98DX2112

#define XCAT_24FE_4STACK_PORTS_DEVICES  \
SKERNEL_98DX1122,                       \
SKERNEL_98DX1123,                       \
SKERNEL_98DX2122,                       \
SKERNEL_98DX2123,                       \
SKERNEL_98DX2151

#define XCAT_8FE_DEVICES \
XCAT_8FE_2STACK_PORTS_DEVICES

#define XCAT_16FE_DEVICES \
XCAT_16FE_2STACK_PORTS_DEVICES, \
XCAT_16FE_4STACK_PORTS_DEVICES

#define XCAT_24FE_DEVICES \
XCAT_24FE_4STACK_PORTS_DEVICES


#define XCAT_FE_DEVICES \
XCAT_8FE_DEVICES,  \
XCAT_16FE_DEVICES, \
XCAT_24FE_DEVICES

#define XCAT_8GE_2STACK_PORTS_DEVICES \
    SKERNEL_98DX3001       ,\
    SKERNEL_98DX3101       ,\
    SKERNEL_98DX4101

#define XCAT_8GE_4STACK_PORTS_DEVICES   \
    SKERNEL_98DX4102


#define XCAT_6GE_4STACK_PORTS_DEVICES   \
    SKERNEL_98DX4103


#define XCAT_16GE_PORTS_DEVICES   \
    SKERNEL_98DX3010,             \
    SKERNEL_98DX3110

#define XCAT_16GE_2STACK_PORTS_DEVICES   \
    SKERNEL_98DX3011

#define XCAT_16GE_4STACK_PORTS_DEVICES   \
    SKERNEL_98DX3111,                    \
    SKERNEL_98DX4110

#define XCAT_24GE_PORTS_DEVICES   \
    SKERNEL_98DX3120

#define XCAT_24GE_2STACK_PORTS_DEVICES   \
    SKERNEL_98DX4121

#define XCAT_24GE_4STACK_PORTS_DEVICES   \
    SKERNEL_98DX3121,                    \
    SKERNEL_98DX3122,                    \
    SKERNEL_98DX3123,                    \
    SKERNEL_98DX3124,                    \
    SKERNEL_98DX3125,                    \
    SKERNEL_98DX4120,                    \
    SKERNEL_98DX4122,                    \
    SKERNEL_98DX4123

#define XCAT_6GE_DEVICES \
XCAT_6GE_4STACK_PORTS_DEVICES,

#define XCAT_8GE_DEVICES \
XCAT_8GE_2STACK_PORTS_DEVICES, \
XCAT_8GE_4STACK_PORTS_DEVICES

#define XCAT_16GE_DEVICES \
XCAT_16GE_PORTS_DEVICES,       \
XCAT_16GE_2STACK_PORTS_DEVICES,\
XCAT_16GE_4STACK_PORTS_DEVICES

#define XCAT_24GE_DEVICES \
XCAT_24GE_PORTS_DEVICES       ,\
XCAT_24GE_2STACK_PORTS_DEVICES,\
XCAT_24GE_4STACK_PORTS_DEVICES

#define XCAT_GE_DEVICES \
XCAT_6GE_DEVICES, \
XCAT_8GE_DEVICES, \
XCAT_16GE_DEVICES,\
XCAT_24GE_DEVICES

#define XCAT_DEVICES \
XCAT_FE_DEVICES,     \
XCAT_GE_DEVICES      \

/* XCAT 24FE/24GE+4XG */
#define  XCAT_24_4_DEVICES_MAC    \
XCAT_24GE_4STACK_PORTS_DEVICES,   \
XCAT_24FE_4STACK_PORTS_DEVICES,   \
SKERNEL_XCAT_24_AND_4

#define XCAT3_24_6_DEVICES_MAC            \
    SKERNEL_XCAT3_24_AND_6

#define AC5_24_6_DEVICES_MAC            \
    SKERNEL_AC5_24_AND_6


#define XCAT2_24_4_DEVICES_MAC            \
    SKERNEL_XCAT2_24_AND_4

/* Lion's 'port group' - 12 XG/GE ports */
#define LION_PORT_GROUP_12_DEVICES_MAC    \
SKERNEL_LION_PORT_GROUP_12

/* Lion - 48 XG/GE ports */
#define LION_48_DEVICES_MAC    \
SKERNEL_LION_48

/* Lion2's 'port group' - 12 XG/GE ports */
#define LION2_PORT_GROUP_12_DEVICES_MAC    \
SKERNEL_LION2_PORT_GROUP_12

/* Lion2 - 96 XG/GE ports */
#define LION2_96_DEVICES_MAC    \
SKERNEL_LION2_96

/* Lion3's 'port group' - 12 XG/GE ports */
#define LION3_PORT_GROUP_12_DEVICES_MAC    \
SKERNEL_LION3_PORT_GROUP_12

/* Lion3 - 96 XG/GE ports */
#define LION3_96_DEVICES_MAC    \
SKERNEL_LION3_96


/* Puma3's network core + fabric core - 32 XG/GE ports */
#define PUMA3_NETWORK_FABRIC_DEVICES_MAC    \
SKERNEL_PUMA3_NETWORK_FABRIC

/* Lion2 - 96 XG/GE ports */
#define PUMA3_64_DEVICES_MAC    \
SKERNEL_PUMA3_64

/* Bobcat2 : number of 'MAC' ports */
#define BOBCAT2_NUM_OF_MAC_PORTS_CNS    72

/* Bobcat2 */
#define BOBCAT2_DEVICES_MAC    \
SKERNEL_BOBCAT2

/* bobk-caelum */
#define BOBK_CAELUM_DEVICES_MAC    \
SKERNEL_BOBK_CAELUM

/* bobk-cetus */
#define BOBK_CETUS_DEVICES_MAC    \
SKERNEL_BOBK_CETUS

/* bobk-alldrin */
#define BOBK_ALDRIN_DEVICES_MAC    \
SKERNEL_BOBK_ALDRIN

/* AC3X */
#define BOBK_AC3X_DEVICES_MAC    \
SKERNEL_AC3X

/* Bobcat3 */
#define BOBCAT3_DEVICES_MAC    \
SKERNEL_BOBCAT3

/* Aldrin2 */
#define ALDRIN2_DEVICES_MAC  \
SKERNEL_ALDRIN2

/* Falcon */
#define FALCON_SINGLE_TILE_DEVICES_MAC    \
SKERNEL_FALCON_SINGLE_TILE

#define FALCON_2T_4T_PACKAGE_DEVICES_MAC            \
        SKERNEL_98CX8525,                \
        SKERNEL_98CX8514,                \
        SKERNEL_98EX5614,                \
        SKERNEL_98CX8535


#define FALCON_2_TILES_DEVICES_MAC          \
        SKERNEL_FALCON_2_TILES,             \
        FALCON_2T_4T_PACKAGE_DEVICES_MAC

#define FALCON_4_TILES_DEVICES_MAC    \
SKERNEL_FALCON_4_TILES

/* Hawk */
#define HAWK_DEVICES_MAC  \
SKERNEL_HAWK

/* Harrier */
#define HARRIER_DEVICES_MAC  \
SKERNEL_HARRIER

/* Ironman-L */
#define IRONMAN_L_DEVICES_MAC  \
SKERNEL_IRONMAN_L

/* Ironman-S */
#define IRONMAN_S_DEVICES_MAC  \
SKERNEL_IRONMAN_S

/* phoenix */
#define PHOENIX_DEVICES_MAC  \
SKERNEL_PHOENIX

/* PIPE */
#define PIPE_DEVICES_MAC    \
SKERNEL_PIPE

/* Bobcat3 : number of 'MAC' ports (37*2= 74)
    there are 2 GOPs with 37 ports each units

    global numbers are :
    0..35   - pipe 0 : Cider ports : 0..35
    36..71  - pipe 1 : Cider ports : 0..35
    72      - pipe 0 : Cider port 36
    73      - pipe 1 : Cider port 36

*/
#define BOBCAT3_NUM_OF_MAC_PORTS_CNS    74


/* Aldrin2 GOP :
(CPSS) global |  GOP MG  |  local GOP    |      | DP local
  port #      | chain    | (in MG chain) | DP#  | port#
  ==========================================================
     0..11    |    0     |    0..11      |  0   |  0..11
    12..23    |    0     |   12..23      |  1   |  0..11
    24..47    |    1     |    0..23      |  2   |  0..23
    48..71    |    1     |   24..47      |  3   |  0..23
        72    |    1     |       63      |  0   |  24
*/
#define ALDRIN2_NUM_OF_MAC_PORTS_CNS 73

/* PIPE device with 16 MACs , with 17 SERDESes */
#define PIPE_NUM_OF_MAC_PORTS_CNS       16

/* Falcon 3.2Tera : number of 'MAC' ports (32*2= 64 + 2)
    there are 4 GOPs with 17 ports each units

    global numbers are :
    0..31   - pipe 0 : local ports : 0..35
    32..63  - pipe 1 : local ports : 0..35
    64      - pipe 1 : local port 36
    65      - pipe 2 : local port 36

*/
#define FALCON_3_2_TERA_NUM_OF_MAC_PORTS_CNS    (64 + 4)
/* Falcon 6.4Tera : number of 'MAC' ports (32*4= 128 + 2)
    there are 8 GOPs with 17 ports each units

    global numbers are :
    0..31   - pipe 0 : local ports : 0..35
    32..63  - pipe 1 : local ports : 0..35
    64..95  - pipe 2 : local ports : 0..35
    96..127 - pipe 3 : local ports : 0..35
    128     - pipe 1 : local port 36
    129     - pipe 2 : local port 36

*/
#define FALCON_6_4_TERA_NUM_OF_MAC_PORTS_CNS    (128+8)
/* Falcon 12.8Tera : number of 'MAC' ports (32*8= 256 + 2)
    there are 16 GOPs with 17 ports each units

    global numbers are :
    0..31   - pipe 0 : local ports : 0..35
    32..63  - pipe 1 : local ports : 0..35
    64..95  - pipe 2 : local ports : 0..35
    96..127 - pipe 3 : local ports : 0..35
    128..159- pipe 4 : local ports : 0..35
    160..191- pipe 5 : local ports : 0..35
    192..223- pipe 6 : local ports : 0..35
    224..255- pipe 7 : local ports : 0..35
    256     - pipe 1 : local port 36
    257     - pipe 2 : local port 36

*/
#define FALCON_12_8_TERA_NUM_OF_MAC_PORTS_CNS   (256+16)


/*tiger*/
const static SKERNEL_DEVICE_TYPE tgXGlegalDevTypes[] =
{   TIGER_1_DEVICES_MAC , END_OF_TABLE};
const static  SKERNEL_DEVICE_TYPE tg12legalDevTypes[] =
{   TIGER_12_DEVICES_MAC , END_OF_TABLE};
const static  SKERNEL_DEVICE_TYPE tg52legalDevTypes[] =
{   TIGER_52_DEVICES_MAC , END_OF_TABLE};
/*samba*/
const static  SKERNEL_DEVICE_TYPE sambaXGlegalDevTypes[] =
{   SAMBA_1_DEVICES_MAC  , END_OF_TABLE};
const static  SKERNEL_DEVICE_TYPE samba12legalDevTypes[] =
{   SAMBA_12_DEVICES_MAC , END_OF_TABLE};
const static  SKERNEL_DEVICE_TYPE samba52legalDevTypes[] =
{   SAMBA_52_DEVICES_MAC , END_OF_TABLE};
/*twistd*/
const static  SKERNEL_DEVICE_TYPE twistdXGlegalDevTypes[] =
{   TWISTD_1_DEVICES_MAC , END_OF_TABLE};
const static  SKERNEL_DEVICE_TYPE twistd12legalDevTypes[] =
{   TWISTD_12_DEVICES_MAC, END_OF_TABLE };
const static  SKERNEL_DEVICE_TYPE twistd52legalDevTypes[] =
{   TWISTD_52_DEVICES_MAC, END_OF_TABLE };
/*twistc*/
const static  SKERNEL_DEVICE_TYPE twistc10legalDevTypes[] =
{   TWISTC_10_DEVICES_MAC, END_OF_TABLE };
const static  SKERNEL_DEVICE_TYPE twistc12legalDevTypes[] =
{   TWISTC_12_DEVICES_MAC, END_OF_TABLE };
const static  SKERNEL_DEVICE_TYPE twistc52legalDevTypes[] =
{   TWISTC_52_DEVICES_MAC, END_OF_TABLE };

/* salsa */
const static  SKERNEL_DEVICE_TYPE salsa24legalDevTypes[] =
{   SALSA_24_DEVICES_MAC , END_OF_TABLE};
const static  SKERNEL_DEVICE_TYPE salsa16legalDevTypes[] =
{   SALSA_16_DEVICES_MAC , END_OF_TABLE};
const static  SKERNEL_DEVICE_TYPE salsa12legalDevTypes[] =
{   SALSA_12_DEVICES_MAC , END_OF_TABLE};
/* cheetah */
const static  SKERNEL_DEVICE_TYPE ch_27legalDevTypes[] =
{   CH_27_DEVICES_MAC , END_OF_TABLE};
const static  SKERNEL_DEVICE_TYPE ch_26legalDevTypes[] =
{   CH_26_DEVICES_MAC , END_OF_TABLE};
const static  SKERNEL_DEVICE_TYPE ch_24legalDevTypes[] =
{   CH_24_DEVICES_MAC , END_OF_TABLE};
const static  SKERNEL_DEVICE_TYPE ch_25legalDevTypes[] =
{   CH_25_DEVICES_MAC , END_OF_TABLE};
const static  SKERNEL_DEVICE_TYPE ch_16legalDevTypes[] =
{   CH_16_DEVICES_MAC , END_OF_TABLE};
const static  SKERNEL_DEVICE_TYPE ch_10legalDevTypes[] =
{   CH_10_DEVICES_MAC , END_OF_TABLE};
/* cheetah-2 */
const static  SKERNEL_DEVICE_TYPE ch2_28legalDevTypes[] =
{   CH2_28_DEVICES_MAC, END_OF_TABLE };
const static  SKERNEL_DEVICE_TYPE ch2_27legalDevTypes[] =
{   CH2_27_DEVICES_MAC, END_OF_TABLE };
const static  SKERNEL_DEVICE_TYPE ch2_26legalDevTypes[] =
{   CH2_26_DEVICES_MAC, END_OF_TABLE };
const static  SKERNEL_DEVICE_TYPE ch2_24legalDevTypes[] =
{   CH2_24_DEVICES_MAC, END_OF_TABLE };
/* cheetah-3 */
const static  SKERNEL_DEVICE_TYPE ch3_28legalDevTypes[] =
{   CH3_28_DEVICES_MAC, END_OF_TABLE };
const static  SKERNEL_DEVICE_TYPE ch3_26legalDevTypes[] =
{   CH3_26_DEVICES_MAC, END_OF_TABLE };
const static  SKERNEL_DEVICE_TYPE ch3_24legalDevTypes[] =
{   CH3_24_DEVICES_MAC, END_OF_TABLE };
const static  SKERNEL_DEVICE_TYPE ch3_6legalDevTypes[] =
{   CH3_6_DEVICES_MAC, END_OF_TABLE };
const static  SKERNEL_DEVICE_TYPE ch3_10legalDevTypes[] =
{   CH3_10_DEVICES_MAC, END_OF_TABLE };
const static  SKERNEL_DEVICE_TYPE ch3_18legalDevTypes[] =
{   CH3_18_DEVICES_MAC, END_OF_TABLE };
const static  SKERNEL_DEVICE_TYPE ch3_20legalDevTypes[] =
{   CH3_20_DEVICES_MAC, END_OF_TABLE };
const static  SKERNEL_DEVICE_TYPE ch3_24_2legalDevTypes[] =
{   CH3_24_2_DEVICES_MAC, END_OF_TABLE };
const static  SKERNEL_DEVICE_TYPE ch3_24_4legalDevTypes[] =
{   CH3_24_4_DEVICES_MAC, END_OF_TABLE };
const static  SKERNEL_DEVICE_TYPE ch3_8_XGlegalDevTypes[] =
{   CH3_8_XG_DEVICES_MAC, END_OF_TABLE };
const static  SKERNEL_DEVICE_TYPE ch3_XGlegalDevTypes[] =
{   CH3_XG_DEVICES_MAC, END_OF_TABLE };
/* Puma */
const static  SKERNEL_DEVICE_TYPE puma_26legalDevTypes[] =
{   PUMA_26_DEVICES_MAC, END_OF_TABLE };
const static  SKERNEL_DEVICE_TYPE puma_24legalDevTypes[] =
{   PUMA_24_DEVICES_MAC, END_OF_TABLE };

/* xCat */
const static  SKERNEL_DEVICE_TYPE xcat_24_4legalDevTypes[] =
{   XCAT_24_4_DEVICES_MAC, END_OF_TABLE };
const static  SKERNEL_DEVICE_TYPE xcat2_24_4legalDevTypes[] =
{   XCAT2_24_4_DEVICES_MAC, END_OF_TABLE };

/* xCat3 */
const static  SKERNEL_DEVICE_TYPE xcat3_24_6legalDevTypes[] =
{   XCAT3_24_6_DEVICES_MAC, END_OF_TABLE };

/* Ac5 */
const static  SKERNEL_DEVICE_TYPE ac5_24_6legalDevTypes[] =
{   AC5_24_6_DEVICES_MAC, END_OF_TABLE };


/* Lion */
const static  SKERNEL_DEVICE_TYPE lionPortGroup_12_legalDevTypes[] =
{   LION_PORT_GROUP_12_DEVICES_MAC, END_OF_TABLE };
const static  SKERNEL_DEVICE_TYPE lion_48_legalDevTypes[] =
{   LION_48_DEVICES_MAC, END_OF_TABLE };

/* Lion 2 */
const static  SKERNEL_DEVICE_TYPE lion2PortGroup_12_legalDevTypes[] =
{   LION2_PORT_GROUP_12_DEVICES_MAC, END_OF_TABLE };
const static  SKERNEL_DEVICE_TYPE lion2_96_legalDevTypes[] =
{   LION2_96_DEVICES_MAC, END_OF_TABLE };

/* Lion 3 */
const static  SKERNEL_DEVICE_TYPE lion3PortGroup_12_legalDevTypes[] =
{   LION3_PORT_GROUP_12_DEVICES_MAC, END_OF_TABLE };
const static  SKERNEL_DEVICE_TYPE lion3_96_legalDevTypes[] =
{   LION3_96_DEVICES_MAC, END_OF_TABLE };

const static  SKERNEL_DEVICE_TYPE bobcat2_legalDevTypes[] =
{   BOBCAT2_DEVICES_MAC, END_OF_TABLE };

const static  SKERNEL_DEVICE_TYPE bobk_caelum_legalDevTypes[] =
{   BOBK_CAELUM_DEVICES_MAC, END_OF_TABLE };

const static  SKERNEL_DEVICE_TYPE bobk_cetus_legalDevTypes[] =
{   BOBK_CETUS_DEVICES_MAC, END_OF_TABLE };

const static  SKERNEL_DEVICE_TYPE bobk_aldrin_legalDevTypes[] =
{   BOBK_ALDRIN_DEVICES_MAC, END_OF_TABLE };

const static  SKERNEL_DEVICE_TYPE ac3x_legalDevTypes[] =
{   BOBK_AC3X_DEVICES_MAC, END_OF_TABLE };

const static  SKERNEL_DEVICE_TYPE bobcat3_legalDevTypes[] =
{   BOBCAT3_DEVICES_MAC, END_OF_TABLE };

const static  SKERNEL_DEVICE_TYPE aldrin2_legalDevTypes[] =
{   ALDRIN2_DEVICES_MAC, END_OF_TABLE };

const static  SKERNEL_DEVICE_TYPE falconSingleTile_legalDevTypes[] =
{   FALCON_SINGLE_TILE_DEVICES_MAC, END_OF_TABLE };
const static  SKERNEL_DEVICE_TYPE falcon2Tiles_legalDevTypes[] =
{   FALCON_2_TILES_DEVICES_MAC, END_OF_TABLE };
const static  SKERNEL_DEVICE_TYPE falcon4Tiles_legalDevTypes[] =
{   FALCON_4_TILES_DEVICES_MAC, END_OF_TABLE };

/* Hawk devices */
const static  SKERNEL_DEVICE_TYPE hawk_legalDevTypes[] =
{   HAWK_DEVICES_MAC, END_OF_TABLE };
/* Phoenix devices */
const static  SKERNEL_DEVICE_TYPE phoenix_legalDevTypes[] =
{   PHOENIX_DEVICES_MAC, END_OF_TABLE };
/* Harrier devices */
const static  SKERNEL_DEVICE_TYPE harrier_legalDevTypes[] =
{   HARRIER_DEVICES_MAC, END_OF_TABLE };

/* Ironman-L devices */
const static  SKERNEL_DEVICE_TYPE ironman_l_legalDevTypes[] =
{   IRONMAN_L_DEVICES_MAC, END_OF_TABLE };

/* Ironman-S devices */
const static  SKERNEL_DEVICE_TYPE ironman_s_legalDevTypes[] =
{   IRONMAN_S_DEVICES_MAC, END_OF_TABLE };


/*PIPE devices*/
const static  SKERNEL_DEVICE_TYPE pipe_legalDevTypes[] =
{   PIPE_DEVICES_MAC, END_OF_TABLE };


/* other */
const static  SKERNEL_DEVICE_TYPE phy_shell_legalDevTypes[] =
{   SKERNEL_PHY_SHELL, END_OF_TABLE };

const static  SKERNEL_DEVICE_TYPE phy_core_legalDevTypes[] =
{   SKERNEL_PHY_CORE_1540M_1548M, END_OF_TABLE };

const static  SKERNEL_DEVICE_TYPE macsec_legalDevTypes[] =
{   SKERNEL_MACSEC, END_OF_TABLE };

const static  SKERNEL_DEVICE_TYPE empty_legalDevTypes[] =
{   SKERNEL_EMPTY, END_OF_TABLE };

/* puma3 */
const static  SKERNEL_DEVICE_TYPE puma3NetworkFabric_legalDevTypes[] =
{   PUMA3_NETWORK_FABRIC_DEVICES_MAC, END_OF_TABLE };
const static  SKERNEL_DEVICE_TYPE puma3_64_legalDevTypes[] =
{   PUMA3_64_DEVICES_MAC, END_OF_TABLE };



/* default state for devices that have 24G ports in 0..23 and XG ports in 23..27

NOTE: it also support devices with 24G (0..23) + 2XG (24,25)
and also support devices with 2XG (24,25)
and also devices with only 24XG
and also devices with only 12 GE ports (0..11)
...
NOTE: supports also devices with non exists ports in the middle ,
    since we always 'mask' this array with defaultPortsBmpPtr
*/
const static  SKERNEL_PORT_STATE_ENT gig24Xg4Ports[28] =
{
    SKERNEL_PORT_STATE_GE_E,     /* 0 */
    SKERNEL_PORT_STATE_GE_E,     /* 1 */
    SKERNEL_PORT_STATE_GE_E,     /* 2 */
    SKERNEL_PORT_STATE_GE_E,     /* 3 */

    SKERNEL_PORT_STATE_GE_E,     /* 4 */
    SKERNEL_PORT_STATE_GE_E,     /* 5 */
    SKERNEL_PORT_STATE_GE_E,     /* 6 */
    SKERNEL_PORT_STATE_GE_E,     /* 7 */

    SKERNEL_PORT_STATE_GE_E,     /* 8 */
    SKERNEL_PORT_STATE_GE_E,     /* 9 */
    SKERNEL_PORT_STATE_GE_E,     /* 10 */
    SKERNEL_PORT_STATE_GE_E,     /* 11 */

    SKERNEL_PORT_STATE_GE_E,     /* 12 */
    SKERNEL_PORT_STATE_GE_E,     /* 13 */
    SKERNEL_PORT_STATE_GE_E,     /* 14 */
    SKERNEL_PORT_STATE_GE_E,     /* 15 */

    SKERNEL_PORT_STATE_GE_E,     /* 16 */
    SKERNEL_PORT_STATE_GE_E,     /* 17 */
    SKERNEL_PORT_STATE_GE_E,     /* 18 */
    SKERNEL_PORT_STATE_GE_E,     /* 19 */

    SKERNEL_PORT_STATE_GE_E,     /* 20 */
    SKERNEL_PORT_STATE_GE_E,     /* 21 */
    SKERNEL_PORT_STATE_GE_E,     /* 22 */
    SKERNEL_PORT_STATE_GE_E,     /* 23 */

    SKERNEL_PORT_STATE_XG_E,     /* 24 */
    SKERNEL_PORT_STATE_XG_E,     /* 25 */
    SKERNEL_PORT_STATE_XG_E,     /* 26 */
    SKERNEL_PORT_STATE_XG_E      /* 27 */
};

/* xCat3 - 24G ports, 6XG ports and XG mac for CPU port (#31) */
const static  SKERNEL_PORT_STATE_ENT xcat3gig24Xg6PortsAndCpuPort[32] =
{
    SKERNEL_PORT_STATE_GE_E,     /* 0 */
    SKERNEL_PORT_STATE_GE_E,     /* 1 */
    SKERNEL_PORT_STATE_GE_E,     /* 2 */
    SKERNEL_PORT_STATE_GE_E,     /* 3 */

    SKERNEL_PORT_STATE_GE_E,     /* 4 */
    SKERNEL_PORT_STATE_GE_E,     /* 5 */
    SKERNEL_PORT_STATE_GE_E,     /* 6 */
    SKERNEL_PORT_STATE_GE_E,     /* 7 */

    SKERNEL_PORT_STATE_GE_E,     /* 8 */
    SKERNEL_PORT_STATE_GE_E,     /* 9 */
    SKERNEL_PORT_STATE_GE_E,     /* 10 */
    SKERNEL_PORT_STATE_GE_E,     /* 11 */

    SKERNEL_PORT_STATE_GE_E,     /* 12 */
    SKERNEL_PORT_STATE_GE_E,     /* 13 */
    SKERNEL_PORT_STATE_GE_E,     /* 14 */
    SKERNEL_PORT_STATE_GE_E,     /* 15 */

    SKERNEL_PORT_STATE_GE_E,     /* 16 */
    SKERNEL_PORT_STATE_GE_E,     /* 17 */
    SKERNEL_PORT_STATE_GE_E,     /* 18 */
    SKERNEL_PORT_STATE_GE_E,     /* 19 */

    SKERNEL_PORT_STATE_GE_E,     /* 20 */
    SKERNEL_PORT_STATE_GE_E,     /* 21 */
    SKERNEL_PORT_STATE_GE_E,     /* 22 */
    SKERNEL_PORT_STATE_GE_E,     /* 23 */

    SKERNEL_PORT_STATE_XG_E,     /* 24 */
    SKERNEL_PORT_STATE_XG_E,     /* 25 */
    SKERNEL_PORT_STATE_XG_E,     /* 26 */
    SKERNEL_PORT_STATE_XG_E,     /* 27 */
    SKERNEL_PORT_STATE_XG_E,     /* 28 */
    SKERNEL_PORT_STATE_XG_E,     /* 29 */

    SKERNEL_PORT_STATE_NOT_EXISTS_E, /* 30 */

    SKERNEL_PORT_STATE_XG_E      /* 31 */
};

/* default for 72 ports : 48 GE + 24 XG devices */
const static  SKERNEL_PORT_STATE_ENT ge48xg24Ports[BOBCAT2_NUM_OF_MAC_PORTS_CNS] =
{
    SKERNEL_PORT_STATE_GE_STACK_A1_E,/* use 'stack GE' to indicate MIB counters logic */       /* 0 */
    SKERNEL_PORT_STATE_GE_STACK_A1_E,       /* 1 */
    SKERNEL_PORT_STATE_GE_STACK_A1_E,       /* 2 */
    SKERNEL_PORT_STATE_GE_STACK_A1_E,       /* 3 */
    SKERNEL_PORT_STATE_GE_STACK_A1_E,       /* 4 */
    SKERNEL_PORT_STATE_GE_STACK_A1_E,       /* 5 */
    SKERNEL_PORT_STATE_GE_STACK_A1_E,       /* 6 */
    SKERNEL_PORT_STATE_GE_STACK_A1_E,       /* 7 */
    SKERNEL_PORT_STATE_GE_STACK_A1_E,       /* 8 */
    SKERNEL_PORT_STATE_GE_STACK_A1_E,       /* 9 */

    SKERNEL_PORT_STATE_GE_STACK_A1_E,       /* 10 */
    SKERNEL_PORT_STATE_GE_STACK_A1_E,       /* 11 */
    SKERNEL_PORT_STATE_GE_STACK_A1_E,       /* 12 */
    SKERNEL_PORT_STATE_GE_STACK_A1_E,       /* 13 */
    SKERNEL_PORT_STATE_GE_STACK_A1_E,       /* 14 */
    SKERNEL_PORT_STATE_GE_STACK_A1_E,       /* 15 */
    SKERNEL_PORT_STATE_GE_STACK_A1_E,       /* 16 */
    SKERNEL_PORT_STATE_GE_STACK_A1_E,       /* 17 */
    SKERNEL_PORT_STATE_GE_STACK_A1_E,       /* 18 */
    SKERNEL_PORT_STATE_GE_STACK_A1_E,       /* 19 */

    SKERNEL_PORT_STATE_GE_STACK_A1_E,       /* 20 */
    SKERNEL_PORT_STATE_GE_STACK_A1_E,       /* 21 */
    SKERNEL_PORT_STATE_GE_STACK_A1_E,       /* 22 */
    SKERNEL_PORT_STATE_GE_STACK_A1_E,       /* 23 */
    SKERNEL_PORT_STATE_GE_STACK_A1_E,       /* 24 */
    SKERNEL_PORT_STATE_GE_STACK_A1_E,       /* 25 */
    SKERNEL_PORT_STATE_GE_STACK_A1_E,       /* 26 */
    SKERNEL_PORT_STATE_GE_STACK_A1_E,       /* 27 */

    SKERNEL_PORT_STATE_GE_STACK_A1_E,       /* 28 */
    SKERNEL_PORT_STATE_GE_STACK_A1_E,       /* 29 */
    SKERNEL_PORT_STATE_GE_STACK_A1_E,       /* 30 */
    SKERNEL_PORT_STATE_GE_STACK_A1_E,       /* 31 */
    SKERNEL_PORT_STATE_GE_STACK_A1_E,       /* 32 */
    SKERNEL_PORT_STATE_GE_STACK_A1_E,       /* 33 */
    SKERNEL_PORT_STATE_GE_STACK_A1_E,       /* 34 */
    SKERNEL_PORT_STATE_GE_STACK_A1_E,       /* 35 */

    SKERNEL_PORT_STATE_GE_STACK_A1_E,       /* 36 */
    SKERNEL_PORT_STATE_GE_STACK_A1_E,       /* 37 */
    SKERNEL_PORT_STATE_GE_STACK_A1_E,       /* 38 */
    SKERNEL_PORT_STATE_GE_STACK_A1_E,       /* 39 */
    SKERNEL_PORT_STATE_GE_STACK_A1_E,       /* 40 */
    SKERNEL_PORT_STATE_GE_STACK_A1_E,       /* 41 */
    SKERNEL_PORT_STATE_GE_STACK_A1_E,       /* 42 */
    SKERNEL_PORT_STATE_GE_STACK_A1_E,       /* 43 */
    SKERNEL_PORT_STATE_GE_STACK_A1_E,       /* 44 */
    SKERNEL_PORT_STATE_GE_STACK_A1_E,       /* 45 */
    SKERNEL_PORT_STATE_GE_STACK_A1_E,       /* 46 */
    SKERNEL_PORT_STATE_GE_STACK_A1_E,       /* 47 */



    SKERNEL_PORT_STATE_XG_E,       /* 48 */
    SKERNEL_PORT_STATE_XG_E,       /* 49 */

    SKERNEL_PORT_STATE_XG_E,       /* 50 */
    SKERNEL_PORT_STATE_XG_E,       /* 51 */
    SKERNEL_PORT_STATE_XG_E,       /* 52 */
    SKERNEL_PORT_STATE_XG_E,       /* 53 */
    SKERNEL_PORT_STATE_XG_E,       /* 54 */
    SKERNEL_PORT_STATE_XG_E,       /* 55 */
    SKERNEL_PORT_STATE_XG_E,       /* 56 */
    SKERNEL_PORT_STATE_XG_E,       /* 57 */
    SKERNEL_PORT_STATE_XG_E,       /* 58 */
    SKERNEL_PORT_STATE_XG_E,       /* 59 */

    SKERNEL_PORT_STATE_XG_E,       /* 60 */
    SKERNEL_PORT_STATE_XG_E,       /* 61 */
    SKERNEL_PORT_STATE_XG_E,       /* 62 */
    SKERNEL_PORT_STATE_XG_E,       /* 63 */
    SKERNEL_PORT_STATE_XG_E,       /* 64 */
    SKERNEL_PORT_STATE_XG_E,       /* 65 */
    SKERNEL_PORT_STATE_XG_E,       /* 66 */
    SKERNEL_PORT_STATE_XG_E,       /* 67 */
    SKERNEL_PORT_STATE_XG_E,       /* 68 */
    SKERNEL_PORT_STATE_XG_E,       /* 69 */

    SKERNEL_PORT_STATE_XG_E,       /* 70 */
    SKERNEL_PORT_STATE_XG_E        /* 71 */

};

/* default for 72 ports : 48 GE + 12 XG devices (12 port not exists) :
    0..47 - GE
    48..55 - not exists
    56..59 - XG
    60..63 - not exists
    64..71 - XG

*/
const static  SKERNEL_PORT_STATE_ENT bobk_ge48xg12Ports[BOBCAT2_NUM_OF_MAC_PORTS_CNS] =
{
    SKERNEL_PORT_STATE_GE_STACK_A1_E,/* use 'stack GE' to indicate MIB counters logic */       /* 0 */
    SKERNEL_PORT_STATE_GE_STACK_A1_E,       /* 1 */
    SKERNEL_PORT_STATE_GE_STACK_A1_E,       /* 2 */
    SKERNEL_PORT_STATE_GE_STACK_A1_E,       /* 3 */
    SKERNEL_PORT_STATE_GE_STACK_A1_E,       /* 4 */
    SKERNEL_PORT_STATE_GE_STACK_A1_E,       /* 5 */
    SKERNEL_PORT_STATE_GE_STACK_A1_E,       /* 6 */
    SKERNEL_PORT_STATE_GE_STACK_A1_E,       /* 7 */
    SKERNEL_PORT_STATE_GE_STACK_A1_E,       /* 8 */
    SKERNEL_PORT_STATE_GE_STACK_A1_E,       /* 9 */

    SKERNEL_PORT_STATE_GE_STACK_A1_E,       /* 10 */
    SKERNEL_PORT_STATE_GE_STACK_A1_E,       /* 11 */
    SKERNEL_PORT_STATE_GE_STACK_A1_E,       /* 12 */
    SKERNEL_PORT_STATE_GE_STACK_A1_E,       /* 13 */
    SKERNEL_PORT_STATE_GE_STACK_A1_E,       /* 14 */
    SKERNEL_PORT_STATE_GE_STACK_A1_E,       /* 15 */
    SKERNEL_PORT_STATE_GE_STACK_A1_E,       /* 16 */
    SKERNEL_PORT_STATE_GE_STACK_A1_E,       /* 17 */
    SKERNEL_PORT_STATE_GE_STACK_A1_E,       /* 18 */
    SKERNEL_PORT_STATE_GE_STACK_A1_E,       /* 19 */

    SKERNEL_PORT_STATE_GE_STACK_A1_E,       /* 20 */
    SKERNEL_PORT_STATE_GE_STACK_A1_E,       /* 21 */
    SKERNEL_PORT_STATE_GE_STACK_A1_E,       /* 22 */
    SKERNEL_PORT_STATE_GE_STACK_A1_E,       /* 23 */
    SKERNEL_PORT_STATE_GE_STACK_A1_E,       /* 24 */
    SKERNEL_PORT_STATE_GE_STACK_A1_E,       /* 25 */
    SKERNEL_PORT_STATE_GE_STACK_A1_E,       /* 26 */
    SKERNEL_PORT_STATE_GE_STACK_A1_E,       /* 27 */

    SKERNEL_PORT_STATE_GE_STACK_A1_E,       /* 28 */
    SKERNEL_PORT_STATE_GE_STACK_A1_E,       /* 29 */
    SKERNEL_PORT_STATE_GE_STACK_A1_E,       /* 30 */
    SKERNEL_PORT_STATE_GE_STACK_A1_E,       /* 31 */
    SKERNEL_PORT_STATE_GE_STACK_A1_E,       /* 32 */
    SKERNEL_PORT_STATE_GE_STACK_A1_E,       /* 33 */
    SKERNEL_PORT_STATE_GE_STACK_A1_E,       /* 34 */
    SKERNEL_PORT_STATE_GE_STACK_A1_E,       /* 35 */

    SKERNEL_PORT_STATE_GE_STACK_A1_E,       /* 36 */
    SKERNEL_PORT_STATE_GE_STACK_A1_E,       /* 37 */
    SKERNEL_PORT_STATE_GE_STACK_A1_E,       /* 38 */
    SKERNEL_PORT_STATE_GE_STACK_A1_E,       /* 39 */
    SKERNEL_PORT_STATE_GE_STACK_A1_E,       /* 40 */
    SKERNEL_PORT_STATE_GE_STACK_A1_E,       /* 41 */
    SKERNEL_PORT_STATE_GE_STACK_A1_E,       /* 42 */
    SKERNEL_PORT_STATE_GE_STACK_A1_E,       /* 43 */
    SKERNEL_PORT_STATE_GE_STACK_A1_E,       /* 44 */
    SKERNEL_PORT_STATE_GE_STACK_A1_E,       /* 45 */
    SKERNEL_PORT_STATE_GE_STACK_A1_E,       /* 46 */
    SKERNEL_PORT_STATE_GE_STACK_A1_E,       /* 47 */



    SKERNEL_PORT_STATE_NOT_EXISTS_E,       /* 48 */
    SKERNEL_PORT_STATE_NOT_EXISTS_E,       /* 49 */

    SKERNEL_PORT_STATE_NOT_EXISTS_E,       /* 50 */
    SKERNEL_PORT_STATE_NOT_EXISTS_E,       /* 51 */
    SKERNEL_PORT_STATE_NOT_EXISTS_E,       /* 52 */
    SKERNEL_PORT_STATE_NOT_EXISTS_E,       /* 53 */
    SKERNEL_PORT_STATE_NOT_EXISTS_E,       /* 54 */
    SKERNEL_PORT_STATE_NOT_EXISTS_E,       /* 55 */
    SKERNEL_PORT_STATE_XG_E,       /* 56 */
    SKERNEL_PORT_STATE_XG_E,       /* 57 */
    SKERNEL_PORT_STATE_XG_E,       /* 58 */
    SKERNEL_PORT_STATE_XG_E,       /* 59 */

    SKERNEL_PORT_STATE_NOT_EXISTS_E,       /* 60 */
    SKERNEL_PORT_STATE_NOT_EXISTS_E,       /* 61 */
    SKERNEL_PORT_STATE_GE_STACK_A1_E,      /* 62 -- GIG mac only (unlike BC2)*/
    SKERNEL_PORT_STATE_NOT_EXISTS_E,       /* 63 */
    SKERNEL_PORT_STATE_XG_E,       /* 64 */
    SKERNEL_PORT_STATE_XG_E,       /* 65 */
    SKERNEL_PORT_STATE_XG_E,       /* 66 */
    SKERNEL_PORT_STATE_XG_E,       /* 67 */
    SKERNEL_PORT_STATE_XG_E,       /* 68 */
    SKERNEL_PORT_STATE_XG_E,       /* 69 */

    SKERNEL_PORT_STATE_XG_E,       /* 70 */
    SKERNEL_PORT_STATE_XG_E        /* 71 */

};

/* BC3 : 74 ports : 18CG(100G) /72XG(10G) ports + 2 XG */
const static  SKERNEL_PORT_STATE_ENT total74macs_cg36xg74Ports[BOBCAT3_NUM_OF_MAC_PORTS_CNS] =
{
/*0..35 - (pipe 0 : 0..35) */
    SKERNEL_PORT_STATE_CG_100G_E, SKERNEL_PORT_STATE_XG_E, SKERNEL_PORT_STATE_XG_E, SKERNEL_PORT_STATE_XG_E,
    SKERNEL_PORT_STATE_CG_100G_E, SKERNEL_PORT_STATE_XG_E, SKERNEL_PORT_STATE_XG_E, SKERNEL_PORT_STATE_XG_E,
    SKERNEL_PORT_STATE_CG_100G_E, SKERNEL_PORT_STATE_XG_E, SKERNEL_PORT_STATE_XG_E, SKERNEL_PORT_STATE_XG_E,
    SKERNEL_PORT_STATE_CG_100G_E, SKERNEL_PORT_STATE_XG_E, SKERNEL_PORT_STATE_XG_E, SKERNEL_PORT_STATE_XG_E,
    SKERNEL_PORT_STATE_CG_100G_E, SKERNEL_PORT_STATE_XG_E, SKERNEL_PORT_STATE_XG_E, SKERNEL_PORT_STATE_XG_E,
    SKERNEL_PORT_STATE_CG_100G_E, SKERNEL_PORT_STATE_XG_E, SKERNEL_PORT_STATE_XG_E, SKERNEL_PORT_STATE_XG_E,
    SKERNEL_PORT_STATE_CG_100G_E, SKERNEL_PORT_STATE_XG_E, SKERNEL_PORT_STATE_XG_E, SKERNEL_PORT_STATE_XG_E,
    SKERNEL_PORT_STATE_CG_100G_E, SKERNEL_PORT_STATE_XG_E, SKERNEL_PORT_STATE_XG_E, SKERNEL_PORT_STATE_XG_E,
    SKERNEL_PORT_STATE_CG_100G_E, SKERNEL_PORT_STATE_XG_E, SKERNEL_PORT_STATE_XG_E, SKERNEL_PORT_STATE_XG_E,
/*36..71 - (pipe 1 : 0..35)*/
    SKERNEL_PORT_STATE_CG_100G_E, SKERNEL_PORT_STATE_XG_E, SKERNEL_PORT_STATE_XG_E, SKERNEL_PORT_STATE_XG_E,
    SKERNEL_PORT_STATE_CG_100G_E, SKERNEL_PORT_STATE_XG_E, SKERNEL_PORT_STATE_XG_E, SKERNEL_PORT_STATE_XG_E,
    SKERNEL_PORT_STATE_CG_100G_E, SKERNEL_PORT_STATE_XG_E, SKERNEL_PORT_STATE_XG_E, SKERNEL_PORT_STATE_XG_E,
    SKERNEL_PORT_STATE_CG_100G_E, SKERNEL_PORT_STATE_XG_E, SKERNEL_PORT_STATE_XG_E, SKERNEL_PORT_STATE_XG_E,
    SKERNEL_PORT_STATE_CG_100G_E, SKERNEL_PORT_STATE_XG_E, SKERNEL_PORT_STATE_XG_E, SKERNEL_PORT_STATE_XG_E,
    SKERNEL_PORT_STATE_CG_100G_E, SKERNEL_PORT_STATE_XG_E, SKERNEL_PORT_STATE_XG_E, SKERNEL_PORT_STATE_XG_E,
    SKERNEL_PORT_STATE_CG_100G_E, SKERNEL_PORT_STATE_XG_E, SKERNEL_PORT_STATE_XG_E, SKERNEL_PORT_STATE_XG_E,
    SKERNEL_PORT_STATE_CG_100G_E, SKERNEL_PORT_STATE_XG_E, SKERNEL_PORT_STATE_XG_E, SKERNEL_PORT_STATE_XG_E,
    SKERNEL_PORT_STATE_CG_100G_E, SKERNEL_PORT_STATE_XG_E, SKERNEL_PORT_STATE_XG_E, SKERNEL_PORT_STATE_XG_E,
/*72 - (pipe 0 : 36)*/
    SKERNEL_PORT_STATE_XG_E,
/*73 - (pipe 1 : 36)*/
    SKERNEL_PORT_STATE_XG_E

};


const static  SKERNEL_PORT_STATE_ENT total73macs_cg6xg73Ports[ALDRIN2_NUM_OF_MAC_PORTS_CNS] =
{
/* GOP MG CHAIN 0 : ports : 0..23 , global ports 0..23 */
    SKERNEL_PORT_STATE_CG_100G_E, SKERNEL_PORT_STATE_XG_E, SKERNEL_PORT_STATE_XG_E, SKERNEL_PORT_STATE_XG_E,
    SKERNEL_PORT_STATE_CG_100G_E, SKERNEL_PORT_STATE_XG_E, SKERNEL_PORT_STATE_XG_E, SKERNEL_PORT_STATE_XG_E,
    SKERNEL_PORT_STATE_CG_100G_E, SKERNEL_PORT_STATE_XG_E, SKERNEL_PORT_STATE_XG_E, SKERNEL_PORT_STATE_XG_E,
    SKERNEL_PORT_STATE_CG_100G_E, SKERNEL_PORT_STATE_XG_E, SKERNEL_PORT_STATE_XG_E, SKERNEL_PORT_STATE_XG_E,
    SKERNEL_PORT_STATE_CG_100G_E, SKERNEL_PORT_STATE_XG_E, SKERNEL_PORT_STATE_XG_E, SKERNEL_PORT_STATE_XG_E,
    SKERNEL_PORT_STATE_CG_100G_E, SKERNEL_PORT_STATE_XG_E, SKERNEL_PORT_STATE_XG_E, SKERNEL_PORT_STATE_XG_E,
/* GOP MG CHAIN 1 : ports : 0..47 , global ports 24..71 */
    SKERNEL_PORT_STATE_XG_E, SKERNEL_PORT_STATE_XG_E, SKERNEL_PORT_STATE_XG_E, SKERNEL_PORT_STATE_XG_E,
    SKERNEL_PORT_STATE_XG_E, SKERNEL_PORT_STATE_XG_E, SKERNEL_PORT_STATE_XG_E, SKERNEL_PORT_STATE_XG_E,
    SKERNEL_PORT_STATE_XG_E, SKERNEL_PORT_STATE_XG_E, SKERNEL_PORT_STATE_XG_E, SKERNEL_PORT_STATE_XG_E,
    SKERNEL_PORT_STATE_XG_E, SKERNEL_PORT_STATE_XG_E, SKERNEL_PORT_STATE_XG_E, SKERNEL_PORT_STATE_XG_E,
    SKERNEL_PORT_STATE_XG_E, SKERNEL_PORT_STATE_XG_E, SKERNEL_PORT_STATE_XG_E, SKERNEL_PORT_STATE_XG_E,
    SKERNEL_PORT_STATE_XG_E, SKERNEL_PORT_STATE_XG_E, SKERNEL_PORT_STATE_XG_E, SKERNEL_PORT_STATE_XG_E,
    SKERNEL_PORT_STATE_XG_E, SKERNEL_PORT_STATE_XG_E, SKERNEL_PORT_STATE_XG_E, SKERNEL_PORT_STATE_XG_E,
    SKERNEL_PORT_STATE_XG_E, SKERNEL_PORT_STATE_XG_E, SKERNEL_PORT_STATE_XG_E, SKERNEL_PORT_STATE_XG_E,
    SKERNEL_PORT_STATE_XG_E, SKERNEL_PORT_STATE_XG_E, SKERNEL_PORT_STATE_XG_E, SKERNEL_PORT_STATE_XG_E,
    SKERNEL_PORT_STATE_XG_E, SKERNEL_PORT_STATE_XG_E, SKERNEL_PORT_STATE_XG_E, SKERNEL_PORT_STATE_XG_E,
    SKERNEL_PORT_STATE_XG_E, SKERNEL_PORT_STATE_XG_E, SKERNEL_PORT_STATE_XG_E, SKERNEL_PORT_STATE_XG_E,
    SKERNEL_PORT_STATE_XG_E, SKERNEL_PORT_STATE_XG_E, SKERNEL_PORT_STATE_XG_E, SKERNEL_PORT_STATE_XG_E,
/* GOP MG CHAIN 1 : port : 48 , global port 72 */
    SKERNEL_PORT_STATE_XG_E
};


/* 16 ports : port 12 is 100G */
const static  SKERNEL_PORT_STATE_ENT pipe_Ports[PIPE_NUM_OF_MAC_PORTS_CNS] =
{
    /*0..11  */
    SKERNEL_PORT_STATE_XG_E, SKERNEL_PORT_STATE_XG_E, SKERNEL_PORT_STATE_XG_E, SKERNEL_PORT_STATE_XG_E,
    SKERNEL_PORT_STATE_XG_E, SKERNEL_PORT_STATE_XG_E, SKERNEL_PORT_STATE_XG_E, SKERNEL_PORT_STATE_XG_E,
    SKERNEL_PORT_STATE_XG_E, SKERNEL_PORT_STATE_XG_E, SKERNEL_PORT_STATE_XG_E, SKERNEL_PORT_STATE_XG_E,
    /*12..15*/
    SKERNEL_PORT_STATE_CG_100G_E, SKERNEL_PORT_STATE_XG_E, SKERNEL_PORT_STATE_XG_E, SKERNEL_PORT_STATE_XG_E

};


/* 32 MAC ports per pipe in Falcon (not include extra 'CPU ports')*/
#define FALCON_32_PER_PIPE_MAC_PORTS \
    SKERNEL_PORT_STATE_MTI_400_E, SKERNEL_PORT_STATE_MTI_50_E , SKERNEL_PORT_STATE_MTI_100_E, SKERNEL_PORT_STATE_MTI_50_E ,      \
    SKERNEL_PORT_STATE_MTI_200_E, SKERNEL_PORT_STATE_MTI_50_E , SKERNEL_PORT_STATE_MTI_100_E, SKERNEL_PORT_STATE_MTI_50_E ,      \
    SKERNEL_PORT_STATE_MTI_400_E, SKERNEL_PORT_STATE_MTI_50_E , SKERNEL_PORT_STATE_MTI_100_E, SKERNEL_PORT_STATE_MTI_50_E ,      \
    SKERNEL_PORT_STATE_MTI_200_E, SKERNEL_PORT_STATE_MTI_50_E , SKERNEL_PORT_STATE_MTI_100_E, SKERNEL_PORT_STATE_MTI_50_E ,      \
    SKERNEL_PORT_STATE_MTI_400_E, SKERNEL_PORT_STATE_MTI_50_E , SKERNEL_PORT_STATE_MTI_100_E, SKERNEL_PORT_STATE_MTI_50_E ,      \
    SKERNEL_PORT_STATE_MTI_200_E, SKERNEL_PORT_STATE_MTI_50_E , SKERNEL_PORT_STATE_MTI_100_E, SKERNEL_PORT_STATE_MTI_50_E ,      \
    SKERNEL_PORT_STATE_MTI_400_E, SKERNEL_PORT_STATE_MTI_50_E , SKERNEL_PORT_STATE_MTI_100_E, SKERNEL_PORT_STATE_MTI_50_E ,      \
    SKERNEL_PORT_STATE_MTI_200_E, SKERNEL_PORT_STATE_MTI_50_E , SKERNEL_PORT_STATE_MTI_100_E, SKERNEL_PORT_STATE_MTI_50_E

/* Falcon 3.2Tera : number of 'MAC' ports (32*2= 64 + 2)
    there are 4 GOPs with 17 ports each units

    global numbers are :
    0..31   - pipe 0 : local ports : 0..31
    32..63  - pipe 1 : local ports : 0..31
    64      - DP[2] local port 9
    65      - DP[4] local port 9

*/
/* Falcon : 65 ports : 16CG(100G) /64XG(25G) ports + 1 XG */
const static  SKERNEL_PORT_STATE_ENT falcon_3_2_Tera_64_25G_and_1_12G_Ports[FALCON_3_2_TERA_NUM_OF_MAC_PORTS_CNS] =
{
/*pipe 0 */
    FALCON_32_PER_PIPE_MAC_PORTS,
/*pipe 1 */
    FALCON_32_PER_PIPE_MAC_PORTS,
/* 4 CPU ports for the 4 Ravens */
    SKERNEL_PORT_STATE_MTI_CPU_E, /* Raven 0 */
    SKERNEL_PORT_STATE_MTI_CPU_E, /* Raven 1 */
    SKERNEL_PORT_STATE_MTI_CPU_E, /* Raven 2 */
    SKERNEL_PORT_STATE_MTI_CPU_E  /* Raven 3 */
};

/* Falcon 6.4Tera : number of 'MAC' ports (32*4= 128 + 2)
    there are 8 GOPs with 17 ports each units

    global numbers are :
    0..31   - pipe 0 : local ports : 0..31
    32..63  - pipe 1 : local ports : 0..31
    64..95  - pipe 2 : local ports : 0..31
    96..127 - pipe 3 : local ports : 0..31
    128     - DP[ 6] local port 9
    129     - DP[ 8] local port 9

    138     - DP[ 0] local port 9
    139     - DP[14] local port 9
*/
/* Falcon : 127 ports : 32CG(100G) /128XG(25G) ports + 2 XG */
const static  SKERNEL_PORT_STATE_ENT falcon_6_4_Tera_128_25G_and_2_12G_Ports[FALCON_6_4_TERA_NUM_OF_MAC_PORTS_CNS] =
{
/*pipe 0 */
    FALCON_32_PER_PIPE_MAC_PORTS,
/*pipe 1 */
    FALCON_32_PER_PIPE_MAC_PORTS,
/*pipe 2 */
    FALCON_32_PER_PIPE_MAC_PORTS,
/*pipe 3 */
    FALCON_32_PER_PIPE_MAC_PORTS,
/* 8 CPU ports for the 8 Ravens */
    SKERNEL_PORT_STATE_MTI_CPU_E, /* Raven 0 */
    SKERNEL_PORT_STATE_MTI_CPU_E, /* Raven 1 */
    SKERNEL_PORT_STATE_MTI_CPU_E, /* Raven 2 */
    SKERNEL_PORT_STATE_MTI_CPU_E, /* Raven 3 */
    SKERNEL_PORT_STATE_MTI_CPU_E, /* Raven 4 */
    SKERNEL_PORT_STATE_MTI_CPU_E, /* Raven 5 */
    SKERNEL_PORT_STATE_MTI_CPU_E, /* Raven 6 */
    SKERNEL_PORT_STATE_MTI_CPU_E  /* Raven 7 */

};

/* Falcon : 256 ports : 64CG(100G) /256XG(25G) ports + 2 XG */
const static  SKERNEL_PORT_STATE_ENT falcon_12_8_Tera_256_25G_and_2_12G_Ports[FALCON_12_8_TERA_NUM_OF_MAC_PORTS_CNS] =
{
/*pipe 0 */
    FALCON_32_PER_PIPE_MAC_PORTS,
/*pipe 1 */
    FALCON_32_PER_PIPE_MAC_PORTS,
/*pipe 2 */
    FALCON_32_PER_PIPE_MAC_PORTS,
/*pipe 3 */
    FALCON_32_PER_PIPE_MAC_PORTS,
/*pipe 4 */
    FALCON_32_PER_PIPE_MAC_PORTS,
/*pipe 5 */
    FALCON_32_PER_PIPE_MAC_PORTS,
/*pipe 6 */
    FALCON_32_PER_PIPE_MAC_PORTS,
/*pipe 7 */
    FALCON_32_PER_PIPE_MAC_PORTS,
/* 16 CPU ports for the 16 Ravens */
    SKERNEL_PORT_STATE_MTI_CPU_E, /* Raven 0 */
    SKERNEL_PORT_STATE_MTI_CPU_E, /* Raven 1 */
    SKERNEL_PORT_STATE_MTI_CPU_E, /* Raven 2 */
    SKERNEL_PORT_STATE_MTI_CPU_E, /* Raven 3 */
    SKERNEL_PORT_STATE_MTI_CPU_E, /* Raven 4 */
    SKERNEL_PORT_STATE_MTI_CPU_E, /* Raven 5 */
    SKERNEL_PORT_STATE_MTI_CPU_E, /* Raven 6 */
    SKERNEL_PORT_STATE_MTI_CPU_E, /* Raven 7 */
    SKERNEL_PORT_STATE_MTI_CPU_E, /* Raven 8 */
    SKERNEL_PORT_STATE_MTI_CPU_E, /* Raven 9 */
    SKERNEL_PORT_STATE_MTI_CPU_E, /* Raven 10 */
    SKERNEL_PORT_STATE_MTI_CPU_E, /* Raven 11 */
    SKERNEL_PORT_STATE_MTI_CPU_E, /* Raven 12 */
    SKERNEL_PORT_STATE_MTI_CPU_E, /* Raven 13 */
    SKERNEL_PORT_STATE_MTI_CPU_E, /* Raven 14 */
    SKERNEL_PORT_STATE_MTI_CPU_E  /* Raven 15 */

};

#define  HAWK_NUM_OF_MAC_PORTS_CNS (26+26+26+26+2)
const static  SKERNEL_PORT_STATE_ENT hawk_Ports[HAWK_NUM_OF_MAC_PORTS_CNS] =
{
    /* DP0 - local 0..25 = global 0..25 */
    /*0 , 1    */SKERNEL_PORT_STATE_MTI_400_E, SKERNEL_PORT_STATE_MTI_50_E ,
    /*2 , 3..5 */SKERNEL_PORT_STATE_MTI_100_E, SKERNEL_PORT_STATE_MTI_USX_E, SKERNEL_PORT_STATE_MTI_USX_E, SKERNEL_PORT_STATE_MTI_USX_E,
    /*6 , 7..9 */SKERNEL_PORT_STATE_MTI_50_E , SKERNEL_PORT_STATE_MTI_USX_E, SKERNEL_PORT_STATE_MTI_USX_E, SKERNEL_PORT_STATE_MTI_USX_E,
    /*10,11..13*/SKERNEL_PORT_STATE_MTI_200_E, SKERNEL_PORT_STATE_MTI_USX_E, SKERNEL_PORT_STATE_MTI_USX_E, SKERNEL_PORT_STATE_MTI_USX_E,
    /*14,15..17*/SKERNEL_PORT_STATE_MTI_50_E , SKERNEL_PORT_STATE_MTI_USX_E, SKERNEL_PORT_STATE_MTI_USX_E, SKERNEL_PORT_STATE_MTI_USX_E,
    /*18,19..21*/SKERNEL_PORT_STATE_MTI_100_E, SKERNEL_PORT_STATE_MTI_USX_E, SKERNEL_PORT_STATE_MTI_USX_E, SKERNEL_PORT_STATE_MTI_USX_E,
    /*22,23..25*/SKERNEL_PORT_STATE_MTI_50_E , SKERNEL_PORT_STATE_MTI_USX_E, SKERNEL_PORT_STATE_MTI_USX_E, SKERNEL_PORT_STATE_MTI_USX_E,
    /* DP0 - local 0..25 = global 26..51 */
    /*0 , 1    */SKERNEL_PORT_STATE_MTI_400_E, SKERNEL_PORT_STATE_MTI_50_E ,
    /*2 , 3..5 */SKERNEL_PORT_STATE_MTI_100_E, SKERNEL_PORT_STATE_MTI_USX_E, SKERNEL_PORT_STATE_MTI_USX_E, SKERNEL_PORT_STATE_MTI_USX_E,
    /*6 , 7..9 */SKERNEL_PORT_STATE_MTI_50_E , SKERNEL_PORT_STATE_MTI_USX_E, SKERNEL_PORT_STATE_MTI_USX_E, SKERNEL_PORT_STATE_MTI_USX_E,
    /*10,11..13*/SKERNEL_PORT_STATE_MTI_200_E, SKERNEL_PORT_STATE_MTI_USX_E, SKERNEL_PORT_STATE_MTI_USX_E, SKERNEL_PORT_STATE_MTI_USX_E,
    /*14,15..17*/SKERNEL_PORT_STATE_MTI_50_E , SKERNEL_PORT_STATE_MTI_USX_E, SKERNEL_PORT_STATE_MTI_USX_E, SKERNEL_PORT_STATE_MTI_USX_E,
    /*18,19..21*/SKERNEL_PORT_STATE_MTI_100_E, SKERNEL_PORT_STATE_MTI_USX_E, SKERNEL_PORT_STATE_MTI_USX_E, SKERNEL_PORT_STATE_MTI_USX_E,
    /*22,23..25*/SKERNEL_PORT_STATE_MTI_50_E , SKERNEL_PORT_STATE_MTI_USX_E, SKERNEL_PORT_STATE_MTI_USX_E, SKERNEL_PORT_STATE_MTI_USX_E,
    /* DP2 - local 0..25 = global 52..77 */
    /*0 , 1    */SKERNEL_PORT_STATE_MTI_400_E, SKERNEL_PORT_STATE_MTI_50_E ,
    /*2 , 3..5 */SKERNEL_PORT_STATE_MTI_100_E, SKERNEL_PORT_STATE_NOT_EXISTS_E, SKERNEL_PORT_STATE_NOT_EXISTS_E, SKERNEL_PORT_STATE_NOT_EXISTS_E,
    /*6 , 7..9 */SKERNEL_PORT_STATE_MTI_50_E , SKERNEL_PORT_STATE_NOT_EXISTS_E, SKERNEL_PORT_STATE_NOT_EXISTS_E, SKERNEL_PORT_STATE_NOT_EXISTS_E,
    /*10,11..13*/SKERNEL_PORT_STATE_MTI_200_E, SKERNEL_PORT_STATE_NOT_EXISTS_E, SKERNEL_PORT_STATE_NOT_EXISTS_E, SKERNEL_PORT_STATE_NOT_EXISTS_E,
    /*14,15..17*/SKERNEL_PORT_STATE_MTI_50_E , SKERNEL_PORT_STATE_NOT_EXISTS_E, SKERNEL_PORT_STATE_NOT_EXISTS_E, SKERNEL_PORT_STATE_NOT_EXISTS_E,
    /*18,19..21*/SKERNEL_PORT_STATE_MTI_100_E, SKERNEL_PORT_STATE_NOT_EXISTS_E, SKERNEL_PORT_STATE_NOT_EXISTS_E, SKERNEL_PORT_STATE_NOT_EXISTS_E,
    /*22,23..25*/SKERNEL_PORT_STATE_MTI_50_E , SKERNEL_PORT_STATE_NOT_EXISTS_E, SKERNEL_PORT_STATE_NOT_EXISTS_E, SKERNEL_PORT_STATE_NOT_EXISTS_E,
    /* DP3 - local 0..25 = global 78..103 */
    /*0 , 1    */SKERNEL_PORT_STATE_MTI_400_E, SKERNEL_PORT_STATE_MTI_50_E ,
    /*2 , 3..5 */SKERNEL_PORT_STATE_MTI_100_E, SKERNEL_PORT_STATE_NOT_EXISTS_E, SKERNEL_PORT_STATE_NOT_EXISTS_E, SKERNEL_PORT_STATE_NOT_EXISTS_E,
    /*6 , 7..9 */SKERNEL_PORT_STATE_MTI_50_E , SKERNEL_PORT_STATE_NOT_EXISTS_E, SKERNEL_PORT_STATE_NOT_EXISTS_E, SKERNEL_PORT_STATE_NOT_EXISTS_E,
    /*10,11..13*/SKERNEL_PORT_STATE_MTI_200_E, SKERNEL_PORT_STATE_NOT_EXISTS_E, SKERNEL_PORT_STATE_NOT_EXISTS_E, SKERNEL_PORT_STATE_NOT_EXISTS_E,
    /*14,15..17*/SKERNEL_PORT_STATE_MTI_50_E , SKERNEL_PORT_STATE_NOT_EXISTS_E, SKERNEL_PORT_STATE_NOT_EXISTS_E, SKERNEL_PORT_STATE_NOT_EXISTS_E,
    /*18,19..21*/SKERNEL_PORT_STATE_MTI_100_E, SKERNEL_PORT_STATE_NOT_EXISTS_E, SKERNEL_PORT_STATE_NOT_EXISTS_E, SKERNEL_PORT_STATE_NOT_EXISTS_E,
    /*22,23..25*/SKERNEL_PORT_STATE_MTI_50_E , SKERNEL_PORT_STATE_NOT_EXISTS_E, SKERNEL_PORT_STATE_NOT_EXISTS_E, SKERNEL_PORT_STATE_NOT_EXISTS_E,
/* NOTE : the device hold actually single GOP port !!! that is connected to DP[0] in PIPE2 and connected to DP[1] in Aldrin3/Cygnus3 */
    /* DP[0] - local 26 = global 104 */
    SKERNEL_PORT_STATE_NOT_EXISTS_E,/* 'CPU port' */
    /* DP[1] - local 26 = global 105 */
    SKERNEL_PORT_STATE_MTI_CPU_E /* 'CPU port' */
};

#define  PHOENIX_NUM_OF_MAC_PORTS_CNS (54)
const static  SKERNEL_PORT_STATE_ENT phoenix_Ports[PHOENIX_NUM_OF_MAC_PORTS_CNS] =
{
    /* single DP0 - 0..53 */
/* 0..7 */SKERNEL_PORT_STATE_MTI_USX_E,           SKERNEL_PORT_STATE_MTI_USX_E, SKERNEL_PORT_STATE_MTI_USX_E, SKERNEL_PORT_STATE_MTI_USX_E,
          SKERNEL_PORT_STATE_MTI_USX_E,           SKERNEL_PORT_STATE_MTI_USX_E, SKERNEL_PORT_STATE_MTI_USX_E, SKERNEL_PORT_STATE_MTI_USX_E,
/* 8..15*/SKERNEL_PORT_STATE_MTI_USX_E,           SKERNEL_PORT_STATE_MTI_USX_E, SKERNEL_PORT_STATE_MTI_USX_E, SKERNEL_PORT_STATE_MTI_USX_E,
          SKERNEL_PORT_STATE_MTI_USX_E,           SKERNEL_PORT_STATE_MTI_USX_E, SKERNEL_PORT_STATE_MTI_USX_E, SKERNEL_PORT_STATE_MTI_USX_E,
/*16..23*/SKERNEL_PORT_STATE_MTI_USX_E,           SKERNEL_PORT_STATE_MTI_USX_E, SKERNEL_PORT_STATE_MTI_USX_E, SKERNEL_PORT_STATE_MTI_USX_E,
          SKERNEL_PORT_STATE_MTI_USX_E,           SKERNEL_PORT_STATE_MTI_USX_E, SKERNEL_PORT_STATE_MTI_USX_E, SKERNEL_PORT_STATE_MTI_USX_E,
/*24..31*/SKERNEL_PORT_STATE_MTI_USX_E,           SKERNEL_PORT_STATE_MTI_USX_E, SKERNEL_PORT_STATE_MTI_USX_E, SKERNEL_PORT_STATE_MTI_USX_E,
          SKERNEL_PORT_STATE_MTI_USX_E,           SKERNEL_PORT_STATE_MTI_USX_E, SKERNEL_PORT_STATE_MTI_USX_E, SKERNEL_PORT_STATE_MTI_USX_E,
/*32..39*/SKERNEL_PORT_STATE_MTI_USX_E,           SKERNEL_PORT_STATE_MTI_USX_E, SKERNEL_PORT_STATE_MTI_USX_E, SKERNEL_PORT_STATE_MTI_USX_E,
          SKERNEL_PORT_STATE_MTI_USX_E,           SKERNEL_PORT_STATE_MTI_USX_E, SKERNEL_PORT_STATE_MTI_USX_E, SKERNEL_PORT_STATE_MTI_USX_E,
/*40..47*/SKERNEL_PORT_STATE_MTI_USX_E,           SKERNEL_PORT_STATE_MTI_USX_E, SKERNEL_PORT_STATE_MTI_USX_E, SKERNEL_PORT_STATE_MTI_USX_E,
          SKERNEL_PORT_STATE_MTI_USX_E,           SKERNEL_PORT_STATE_MTI_USX_E, SKERNEL_PORT_STATE_MTI_USX_E, SKERNEL_PORT_STATE_MTI_USX_E,

/*48*/    SKERNEL_PORT_STATE_MTI_CPU_E,
/*49*/    SKERNEL_PORT_STATE_MTI_CPU_E,
/*50*/    SKERNEL_PORT_STATE_MTI_50_E,
/*51*/    SKERNEL_PORT_STATE_MTI_50_E,
/*52*/    SKERNEL_PORT_STATE_MTI_50_E,
/*53*/    SKERNEL_PORT_STATE_MTI_50_E
};

#define MAC_NA  SKERNEL_PORT_STATE_NOT_EXISTS_E
#define  HARRIER_NUM_OF_MAC_PORTS_CNS (16+16+8)
const static  SKERNEL_PORT_STATE_ENT harrier_Ports[HARRIER_NUM_OF_MAC_PORTS_CNS] =
{
    /* MACs :  0, 2, 4, 6, 8,10,12,14 (the odd MACs are the 'preemption' of the even ones)
       DP0 - ports 0..15 */
    SKERNEL_PORT_STATE_MTI_400_E,MAC_NA, SKERNEL_PORT_STATE_MTI_50_E ,MAC_NA,
    SKERNEL_PORT_STATE_MTI_100_E,MAC_NA, SKERNEL_PORT_STATE_MTI_50_E ,MAC_NA,
    SKERNEL_PORT_STATE_MTI_200_E,MAC_NA, SKERNEL_PORT_STATE_MTI_50_E ,MAC_NA,
    SKERNEL_PORT_STATE_MTI_100_E,MAC_NA, SKERNEL_PORT_STATE_MTI_50_E ,MAC_NA,
    /* MACs : 16,18,20,22,24,26,28,30 (the odd MACs are the 'preemption' of the even ones)
       DP1 - ports 16..31 */
    SKERNEL_PORT_STATE_MTI_400_E,MAC_NA, SKERNEL_PORT_STATE_MTI_50_E ,MAC_NA,
    SKERNEL_PORT_STATE_MTI_100_E,MAC_NA, SKERNEL_PORT_STATE_MTI_50_E ,MAC_NA,
    SKERNEL_PORT_STATE_MTI_200_E,MAC_NA, SKERNEL_PORT_STATE_MTI_50_E ,MAC_NA,
    SKERNEL_PORT_STATE_MTI_100_E,MAC_NA, SKERNEL_PORT_STATE_MTI_50_E ,MAC_NA,
    /* MACs : 32,34,36,38 (the odd MACs are the 'preemption' of the even ones)
       DP2 - ports 32..39 */
    SKERNEL_PORT_STATE_MTI_200_E,MAC_NA, SKERNEL_PORT_STATE_MTI_50_E ,MAC_NA,
    SKERNEL_PORT_STATE_MTI_100_E,MAC_NA, SKERNEL_PORT_STATE_MTI_50_E ,MAC_NA,

};

#define  IRONMAN_S_NUM_OF_MAC_PORTS_CNS     9
const static  SKERNEL_PORT_STATE_ENT ironman_s_Ports[IRONMAN_S_NUM_OF_MAC_PORTS_CNS] =
{
    /* mac 0..3  */
    SKERNEL_PORT_STATE_MTI_USX_E , SKERNEL_PORT_STATE_MTI_USX_E , SKERNEL_PORT_STATE_MTI_USX_E , SKERNEL_PORT_STATE_MTI_USX_E ,
    /* mac 4..7  */
    SKERNEL_PORT_STATE_MTI_USX_E , SKERNEL_PORT_STATE_MTI_USX_E , SKERNEL_PORT_STATE_MTI_USX_E , SKERNEL_PORT_STATE_MTI_USX_E ,
    /* mac 8     */
    SKERNEL_PORT_STATE_MTI_USX_E ,
};

#define  IRONMAN_L_NUM_OF_MAC_PORTS_CNS     55
const static  SKERNEL_PORT_STATE_ENT ironman_l_Ports[IRONMAN_L_NUM_OF_MAC_PORTS_CNS] =
{
/*  0.. 7 */SKERNEL_PORT_STATE_MTI_USX_E , SKERNEL_PORT_STATE_MTI_USX_E , SKERNEL_PORT_STATE_MTI_USX_E , SKERNEL_PORT_STATE_MTI_USX_E ,
            SKERNEL_PORT_STATE_MTI_USX_E , SKERNEL_PORT_STATE_MTI_USX_E , SKERNEL_PORT_STATE_MTI_USX_E , SKERNEL_PORT_STATE_MTI_USX_E ,
/*  8..15 */SKERNEL_PORT_STATE_MTI_USX_E , SKERNEL_PORT_STATE_MTI_USX_E , SKERNEL_PORT_STATE_MTI_USX_E , SKERNEL_PORT_STATE_MTI_USX_E ,
            SKERNEL_PORT_STATE_MTI_USX_E , SKERNEL_PORT_STATE_MTI_USX_E , SKERNEL_PORT_STATE_MTI_USX_E , SKERNEL_PORT_STATE_MTI_USX_E ,
/* 16..23 */SKERNEL_PORT_STATE_MTI_USX_E , SKERNEL_PORT_STATE_MTI_USX_E , SKERNEL_PORT_STATE_MTI_USX_E , SKERNEL_PORT_STATE_MTI_USX_E ,
            SKERNEL_PORT_STATE_MTI_USX_E , SKERNEL_PORT_STATE_MTI_USX_E , SKERNEL_PORT_STATE_MTI_USX_E , SKERNEL_PORT_STATE_MTI_USX_E ,
/* 24..31 */SKERNEL_PORT_STATE_MTI_USX_E , SKERNEL_PORT_STATE_MTI_USX_E , SKERNEL_PORT_STATE_MTI_USX_E , SKERNEL_PORT_STATE_MTI_USX_E ,
            SKERNEL_PORT_STATE_MTI_USX_E , SKERNEL_PORT_STATE_MTI_USX_E , SKERNEL_PORT_STATE_MTI_USX_E , SKERNEL_PORT_STATE_MTI_USX_E ,
/* 32..39 */SKERNEL_PORT_STATE_MTI_USX_E , SKERNEL_PORT_STATE_MTI_USX_E , SKERNEL_PORT_STATE_MTI_USX_E , SKERNEL_PORT_STATE_MTI_USX_E ,
            SKERNEL_PORT_STATE_MTI_USX_E , SKERNEL_PORT_STATE_MTI_USX_E , SKERNEL_PORT_STATE_MTI_USX_E , SKERNEL_PORT_STATE_MTI_USX_E ,
/* 40..47 */SKERNEL_PORT_STATE_MTI_USX_E , SKERNEL_PORT_STATE_MTI_USX_E , SKERNEL_PORT_STATE_MTI_USX_E , SKERNEL_PORT_STATE_MTI_USX_E ,
            SKERNEL_PORT_STATE_MTI_USX_E , SKERNEL_PORT_STATE_MTI_USX_E , SKERNEL_PORT_STATE_MTI_USX_E , SKERNEL_PORT_STATE_MTI_USX_E ,

/* 48 */    SKERNEL_PORT_STATE_MTI_USX_E ,
/* 49 */    SKERNEL_PORT_STATE_MTI_USX_E ,
/* 50 */    SKERNEL_PORT_STATE_MTI_USX_E ,
/* 51 */    SKERNEL_PORT_STATE_MTI_USX_E ,
/* 52 */    SKERNEL_PORT_STATE_MTI_USX_E ,
/* 53 */    SKERNEL_PORT_STATE_MTI_USX_E ,
/* 54 */    SKERNEL_PORT_STATE_MTI_USX_E ,

};

/* default for 48FE + 4GE devices */
const static  SKERNEL_PORT_STATE_ENT fast48Gig4Ports[52] =
{
    SKERNEL_PORT_STATE_FE_E,       /* 0 */
    SKERNEL_PORT_STATE_FE_E,       /* 1 */
    SKERNEL_PORT_STATE_FE_E,       /* 2 */
    SKERNEL_PORT_STATE_FE_E,       /* 3 */

    SKERNEL_PORT_STATE_FE_E,       /* 4 */
    SKERNEL_PORT_STATE_FE_E,       /* 5 */
    SKERNEL_PORT_STATE_FE_E,       /* 6 */
    SKERNEL_PORT_STATE_FE_E,       /* 7 */

    SKERNEL_PORT_STATE_FE_E,       /* 8 */
    SKERNEL_PORT_STATE_FE_E,       /* 9 */
    SKERNEL_PORT_STATE_FE_E,       /* 10 */
    SKERNEL_PORT_STATE_FE_E,       /* 11 */

    SKERNEL_PORT_STATE_FE_E,       /* 12 */
    SKERNEL_PORT_STATE_FE_E,       /* 13 */
    SKERNEL_PORT_STATE_FE_E,       /* 14 */
    SKERNEL_PORT_STATE_FE_E,       /* 15 */

    SKERNEL_PORT_STATE_FE_E,       /* 16 */
    SKERNEL_PORT_STATE_FE_E,       /* 17 */
    SKERNEL_PORT_STATE_FE_E,       /* 18 */
    SKERNEL_PORT_STATE_FE_E,       /* 19 */

    SKERNEL_PORT_STATE_FE_E,       /* 20 */
    SKERNEL_PORT_STATE_FE_E,       /* 21 */
    SKERNEL_PORT_STATE_FE_E,       /* 22 */
    SKERNEL_PORT_STATE_FE_E,       /* 23 */

    SKERNEL_PORT_STATE_FE_E,       /* 24 */
    SKERNEL_PORT_STATE_FE_E,       /* 25 */
    SKERNEL_PORT_STATE_FE_E,       /* 26 */
    SKERNEL_PORT_STATE_FE_E,       /* 27 */

    SKERNEL_PORT_STATE_FE_E,       /* 28 */
    SKERNEL_PORT_STATE_FE_E,       /* 29 */
    SKERNEL_PORT_STATE_FE_E,       /* 30 */
    SKERNEL_PORT_STATE_FE_E,       /* 31 */

    SKERNEL_PORT_STATE_FE_E,       /* 32 */
    SKERNEL_PORT_STATE_FE_E,       /* 33 */
    SKERNEL_PORT_STATE_FE_E,       /* 34 */
    SKERNEL_PORT_STATE_FE_E,       /* 35 */

    SKERNEL_PORT_STATE_FE_E,       /* 36 */
    SKERNEL_PORT_STATE_FE_E,       /* 37 */
    SKERNEL_PORT_STATE_FE_E,       /* 38 */
    SKERNEL_PORT_STATE_FE_E,       /* 39 */

    SKERNEL_PORT_STATE_FE_E,       /* 40 */
    SKERNEL_PORT_STATE_FE_E,       /* 41 */
    SKERNEL_PORT_STATE_FE_E,       /* 42 */
    SKERNEL_PORT_STATE_FE_E,       /* 43 */

    SKERNEL_PORT_STATE_FE_E,       /* 44 */
    SKERNEL_PORT_STATE_FE_E,       /* 45 */
    SKERNEL_PORT_STATE_FE_E,       /* 46 */
    SKERNEL_PORT_STATE_FE_E,       /* 47 */

    SKERNEL_PORT_STATE_GE_E,       /* 48 */
    SKERNEL_PORT_STATE_GE_E,       /* 49 */
    SKERNEL_PORT_STATE_GE_E,       /* 50 */
    SKERNEL_PORT_STATE_GE_E        /* 51 */
};


/* default for 28XG devices */
const static  SKERNEL_PORT_STATE_ENT xg28Ports[28] =
{
    SKERNEL_PORT_STATE_XG_E,       /* 0 */
    SKERNEL_PORT_STATE_XG_E,       /* 1 */
    SKERNEL_PORT_STATE_XG_E,       /* 2 */
    SKERNEL_PORT_STATE_XG_E,       /* 3 */

    SKERNEL_PORT_STATE_XG_E,       /* 4 */
    SKERNEL_PORT_STATE_XG_E,       /* 5 */
    SKERNEL_PORT_STATE_XG_E,       /* 6 */
    SKERNEL_PORT_STATE_XG_E,       /* 7 */

    SKERNEL_PORT_STATE_XG_E,       /* 8 */
    SKERNEL_PORT_STATE_XG_E,       /* 9 */
    SKERNEL_PORT_STATE_XG_E,       /* 10 */
    SKERNEL_PORT_STATE_XG_E,       /* 11 */

    SKERNEL_PORT_STATE_XG_E,       /* 12 */
    SKERNEL_PORT_STATE_XG_E,       /* 13 */
    SKERNEL_PORT_STATE_XG_E,       /* 14 */
    SKERNEL_PORT_STATE_XG_E,       /* 15 */

    SKERNEL_PORT_STATE_XG_E,       /* 16 */
    SKERNEL_PORT_STATE_XG_E,       /* 17 */
    SKERNEL_PORT_STATE_XG_E,       /* 18 */
    SKERNEL_PORT_STATE_XG_E,       /* 19 */

    SKERNEL_PORT_STATE_XG_E,       /* 20 */
    SKERNEL_PORT_STATE_XG_E,       /* 21 */
    SKERNEL_PORT_STATE_XG_E,       /* 22 */
    SKERNEL_PORT_STATE_XG_E,       /* 23 */

    SKERNEL_PORT_STATE_XG_E,       /* 24 */
    SKERNEL_PORT_STATE_XG_E,       /* 25 */
    SKERNEL_PORT_STATE_XG_E,       /* 26 */
    SKERNEL_PORT_STATE_XG_E        /* 27 */

};

/* default for puma3Ports : core 0 : 12 NW ports, 12 fabric */
/* default for puma3Ports : core 1 : are 16+ the port numbers of core 0 : for 12 NW ports, 12 fabric*/
const static  SKERNEL_PORT_STATE_ENT puma3Ports[48] =
{
    SKERNEL_PORT_STATE_XG_E,       /* 0 */
    SKERNEL_PORT_STATE_XG_E,       /* 1 */
    SKERNEL_PORT_STATE_XG_E,       /* 2 */
    SKERNEL_PORT_STATE_XG_E,       /* 3 */

    SKERNEL_PORT_STATE_XG_E,       /* 4 */
    SKERNEL_PORT_STATE_XG_E,       /* 5 */
    SKERNEL_PORT_STATE_XG_E,       /* 6 */
    SKERNEL_PORT_STATE_XG_E,       /* 7 */

    SKERNEL_PORT_STATE_XG_E,       /* 8 */
    SKERNEL_PORT_STATE_XG_E,       /* 9 */
    SKERNEL_PORT_STATE_XG_E,       /* 10 */
    SKERNEL_PORT_STATE_XG_E,       /* 11 */

    SKERNEL_PORT_STATE_NOT_EXISTS_E,       /* 12 */
    SKERNEL_PORT_STATE_NOT_EXISTS_E,       /* 13 */
    SKERNEL_PORT_STATE_NOT_EXISTS_E,       /* 14 */
    SKERNEL_PORT_STATE_NOT_EXISTS_E,       /* 15 */

    SKERNEL_PORT_STATE_NOT_EXISTS_E,       /* 16 */
    SKERNEL_PORT_STATE_NOT_EXISTS_E,       /* 17 */
    SKERNEL_PORT_STATE_NOT_EXISTS_E,       /* 18 */
    SKERNEL_PORT_STATE_NOT_EXISTS_E,       /* 19 */

    SKERNEL_PORT_STATE_NOT_EXISTS_E,       /* 20 */
    SKERNEL_PORT_STATE_NOT_EXISTS_E,       /* 21 */
    SKERNEL_PORT_STATE_NOT_EXISTS_E,       /* 22 */
    SKERNEL_PORT_STATE_NOT_EXISTS_E,       /* 23 */

    SKERNEL_PORT_STATE_NOT_EXISTS_E,       /* 24 */
    SKERNEL_PORT_STATE_NOT_EXISTS_E,       /* 25 */
    SKERNEL_PORT_STATE_NOT_EXISTS_E,       /* 26 */
    SKERNEL_PORT_STATE_NOT_EXISTS_E,       /* 27 */

    SKERNEL_PORT_STATE_NOT_EXISTS_E,       /* 28 */
    SKERNEL_PORT_STATE_NOT_EXISTS_E,       /* 29 */
    SKERNEL_PORT_STATE_NOT_EXISTS_E,       /* 30 */
    SKERNEL_PORT_STATE_NOT_EXISTS_E,       /* 31 */

    SKERNEL_PORT_STATE_XG_E,       /* 32 */
    SKERNEL_PORT_STATE_XG_E,       /* 33 */
    SKERNEL_PORT_STATE_XG_E,       /* 34 */
    SKERNEL_PORT_STATE_XG_E,       /* 35 */

    SKERNEL_PORT_STATE_XG_E,       /* 36 */
    SKERNEL_PORT_STATE_XG_E,       /* 37 */
    SKERNEL_PORT_STATE_XG_E,       /* 38 */
    SKERNEL_PORT_STATE_XG_E,       /* 39 */

    SKERNEL_PORT_STATE_XG_E,       /* 40 */
    SKERNEL_PORT_STATE_XG_E,       /* 41 */
    SKERNEL_PORT_STATE_XG_E,       /* 42 */
    SKERNEL_PORT_STATE_XG_E,       /* 43 */

    SKERNEL_PORT_STATE_NOT_EXISTS_E,       /* 44 */
    SKERNEL_PORT_STATE_NOT_EXISTS_E,       /* 45 */
    SKERNEL_PORT_STATE_NOT_EXISTS_E,       /* 46 */
    SKERNEL_PORT_STATE_NOT_EXISTS_E        /* 47 */

};

/* default for 24FE + 4GE devices */
const static  SKERNEL_PORT_STATE_ENT fast24Gig4Ports[28] =
{
    SKERNEL_PORT_STATE_FE_E,     /* 0 */
    SKERNEL_PORT_STATE_FE_E,     /* 1 */
    SKERNEL_PORT_STATE_FE_E,     /* 2 */
    SKERNEL_PORT_STATE_FE_E,     /* 3 */

    SKERNEL_PORT_STATE_FE_E,     /* 4 */
    SKERNEL_PORT_STATE_FE_E,     /* 5 */
    SKERNEL_PORT_STATE_FE_E,     /* 6 */
    SKERNEL_PORT_STATE_FE_E,     /* 7 */

    SKERNEL_PORT_STATE_FE_E,     /* 8 */
    SKERNEL_PORT_STATE_FE_E,     /* 9 */
    SKERNEL_PORT_STATE_FE_E,     /* 10 */
    SKERNEL_PORT_STATE_FE_E,     /* 11 */

    SKERNEL_PORT_STATE_FE_E,     /* 12 */
    SKERNEL_PORT_STATE_FE_E,     /* 13 */
    SKERNEL_PORT_STATE_FE_E,     /* 14 */
    SKERNEL_PORT_STATE_FE_E,     /* 15 */

    SKERNEL_PORT_STATE_FE_E,     /* 16 */
    SKERNEL_PORT_STATE_FE_E,     /* 17 */
    SKERNEL_PORT_STATE_FE_E,     /* 18 */
    SKERNEL_PORT_STATE_FE_E,     /* 19 */

    SKERNEL_PORT_STATE_FE_E,     /* 20 */
    SKERNEL_PORT_STATE_FE_E,     /* 21 */
    SKERNEL_PORT_STATE_FE_E,     /* 22 */
    SKERNEL_PORT_STATE_FE_E,     /* 23 */

    SKERNEL_PORT_STATE_GE_STACK_A1_E,     /* 24 */
    SKERNEL_PORT_STATE_GE_STACK_A1_E,     /* 25 */
    SKERNEL_PORT_STATE_GE_STACK_A1_E,     /* 26 */
    SKERNEL_PORT_STATE_GE_STACK_A1_E      /* 27 */
};

const static  SKERNEL_PORT_STATE_ENT bobkAldrin_xg32Ports[BOBCAT2_NUM_OF_MAC_PORTS_CNS] =
{
    SKERNEL_PORT_STATE_XG_E,       /* 0, DP0/DMA56 */
    SKERNEL_PORT_STATE_XG_E,       /* 1, DP0/DMA57 */
    SKERNEL_PORT_STATE_XG_E,       /* 2, DP0/DMA58 */
    SKERNEL_PORT_STATE_XG_E,       /* 3, DP0/DMA59 */
    SKERNEL_PORT_STATE_XG_E,       /* 4, DP0/DMA64 */
    SKERNEL_PORT_STATE_XG_E,       /* 5, DP0/DMA65 */
    SKERNEL_PORT_STATE_XG_E,       /* 6, DP0/DMA66 */
    SKERNEL_PORT_STATE_XG_E,       /* 7, DP0/DMA67 */
    SKERNEL_PORT_STATE_XG_E,       /* 8, DP0/DMA68 */
    SKERNEL_PORT_STATE_XG_E,       /* 9, DP0/DMA69 */
    SKERNEL_PORT_STATE_XG_E,       /* 10, DP0/DMA70 */
    SKERNEL_PORT_STATE_XG_E,       /* 11, DP0/DMA71*/
    SKERNEL_PORT_STATE_XG_E,       /* 12, DP1/DMA56 */
    SKERNEL_PORT_STATE_XG_E,       /* 13, DP1/DMA57 */
    SKERNEL_PORT_STATE_XG_E,       /* 14, DP1/DMA58 */
    SKERNEL_PORT_STATE_XG_E,       /* 15, DP1/DMA59 */
    SKERNEL_PORT_STATE_XG_E,       /* 16, DP1/DMA64 */
    SKERNEL_PORT_STATE_XG_E,       /* 17, DP1/DMA65 */
    SKERNEL_PORT_STATE_XG_E,       /* 18, DP1/DMA66 */
    SKERNEL_PORT_STATE_XG_E,       /* 19, DP1/DMA67 */
    SKERNEL_PORT_STATE_XG_E,       /* 20, DP1/DMA68 */
    SKERNEL_PORT_STATE_XG_E,       /* 21, DP1/DMA69 */
    SKERNEL_PORT_STATE_XG_E,       /* 22, DP1/DMA70 */
    SKERNEL_PORT_STATE_XG_E,       /* 23, DP1/DMA71 */
    SKERNEL_PORT_STATE_XG_E,       /* 24, DP2/DMA56 */
    SKERNEL_PORT_STATE_XG_E,       /* 25, DP2/DMA57 */
    SKERNEL_PORT_STATE_XG_E,       /* 26, DP2/DMA58 */
    SKERNEL_PORT_STATE_XG_E,       /* 27, DP2/DMA59 */
    SKERNEL_PORT_STATE_XG_E,       /* 28, DP2/DMA64 */
    SKERNEL_PORT_STATE_XG_E,       /* 29, DP2/DMA65 */
    SKERNEL_PORT_STATE_XG_E,       /* 30, DP2/DMA66 */
    SKERNEL_PORT_STATE_XG_E,       /* 31, DP2/DMA67 */
    SKERNEL_PORT_STATE_GE_STACK_A1_E,/* use 'stack GE' to indicate MIB counters logic */ /* 32, DP2/DMA62 */
    SKERNEL_PORT_STATE_NOT_EXISTS_E,       /*  68 */
    SKERNEL_PORT_STATE_NOT_EXISTS_E,       /*  69 */
    SKERNEL_PORT_STATE_NOT_EXISTS_E,       /*  70 */
    SKERNEL_PORT_STATE_NOT_EXISTS_E,       /*  71 */
};

/* simSupportedTypes :
    purpose :
        DB to hold the device types that the sim support

fields :
    devFamily - device family (cheetah/salsa/tiger/../puma)
    numOfPorts - number of ports in the device
    devTypeArray - array of devices for the device family that has same number
                   of ports
    defaultPortsBmpPtr - pointer to special ports bitmap.
                         if this is NULL , that means that the default bitmap
                         defined by the number of ports that considered to be
                         sequential for 0 to the numOfPorts
    multiModePortsBmpPtr - pointer to ports bitmap that support multi state mode.
                         if this is NULL , no port support 'Multi mode'
    existingPortsStatePtr - pointer to array of port's state that state the
                            'default' state of port.
                            NEVER NULL pointer
                          --> NOTE: the relevant ports are only the ports that
                                    set in defaultPortsBmpPtr  !!!

*/
typedef struct {
    SKERNEL_DEVICE_FAMILY_TYPE  devFamily;
    GT_U32                      numOfPorts;
    const SKERNEL_DEVICE_TYPE   *devTypeArray;
    const PORTS_BMP_STC    *defaultPortsBmpPtr;
    const PORTS_BMP_STC    *multiModePortsBmpPtr;
    const SKERNEL_PORT_STATE_ENT *existingPortsStatePtr;
}SIM_SUPPORTED_TYPES_STC;

const static SIM_SUPPORTED_TYPES_STC simSupportedTypes[] =
{
    {SKERNEL_TWIST_C_FAMILY   ,52 ,twistc52legalDevTypes ,NULL,NULL,fast48Gig4Ports},
    {SKERNEL_TWIST_C_FAMILY   ,12 ,twistc12legalDevTypes ,NULL,NULL,gig24Xg4Ports},
    {SKERNEL_TWIST_C_FAMILY   ,10 ,twistc10legalDevTypes ,NULL,NULL,gig24Xg4Ports},

    {SKERNEL_TWIST_D_FAMILY   ,52 , twistd52legalDevTypes,NULL,NULL,fast48Gig4Ports},
    {SKERNEL_TWIST_D_FAMILY   ,12 , twistd12legalDevTypes,NULL,NULL,gig24Xg4Ports},
    {SKERNEL_TWIST_D_FAMILY   , 1 , twistdXGlegalDevTypes,NULL,NULL,xg28Ports},

    {SKERNEL_SAMBA_FAMILY   ,52 ,samba52legalDevTypes, NULL, NULL,fast48Gig4Ports},
    {SKERNEL_SAMBA_FAMILY   ,12 ,samba12legalDevTypes, NULL, NULL,gig24Xg4Ports},
    {SKERNEL_SAMBA_FAMILY   , 1 ,sambaXGlegalDevTypes, NULL, NULL,xg28Ports},

    {SKERNEL_TIGER_FAMILY   ,52 ,tg52legalDevTypes, NULL, NULL,fast48Gig4Ports},
    {SKERNEL_TIGER_FAMILY   ,12 ,tg12legalDevTypes, NULL, NULL,gig24Xg4Ports},
    {SKERNEL_TIGER_FAMILY   , 1 ,tgXGlegalDevTypes, NULL, NULL,xg28Ports},

    {SKERNEL_SALSA_FAMILY   ,12 ,salsa12legalDevTypes, NULL, NULL,gig24Xg4Ports},
    {SKERNEL_SALSA_FAMILY   ,16 ,salsa16legalDevTypes, NULL, NULL,gig24Xg4Ports},
    {SKERNEL_SALSA_FAMILY   ,24 ,salsa24legalDevTypes, NULL, NULL,gig24Xg4Ports},

    {SKERNEL_CHEETAH_1_FAMILY   ,27 ,ch_27legalDevTypes, NULL, NULL,gig24Xg4Ports},
    {SKERNEL_CHEETAH_1_FAMILY   ,26 ,ch_26legalDevTypes, NULL, NULL,gig24Xg4Ports},
    {SKERNEL_CHEETAH_1_FAMILY   ,24 ,ch_24legalDevTypes, NULL, NULL,gig24Xg4Ports},
    {SKERNEL_CHEETAH_1_FAMILY   ,25 ,ch_25legalDevTypes, NULL, NULL,gig24Xg4Ports},
    {SKERNEL_CHEETAH_1_FAMILY   ,16 ,ch_16legalDevTypes, NULL, NULL,gig24Xg4Ports},
    {SKERNEL_CHEETAH_1_FAMILY   ,10 ,ch_10legalDevTypes, NULL, NULL,gig24Xg4Ports},

    {SKERNEL_CHEETAH_2_FAMILY   ,28 ,ch2_28legalDevTypes, NULL, NULL,gig24Xg4Ports},
    {SKERNEL_CHEETAH_2_FAMILY   ,27 ,ch2_27legalDevTypes, NULL, NULL,gig24Xg4Ports},
    {SKERNEL_CHEETAH_2_FAMILY   ,26 ,ch2_26legalDevTypes, NULL, NULL,gig24Xg4Ports},
    {SKERNEL_CHEETAH_2_FAMILY   ,24 ,ch2_24legalDevTypes, NULL, NULL,gig24Xg4Ports},

    {SKERNEL_CHEETAH_3_FAMILY   ,28 ,ch3_28legalDevTypes, NULL, NULL,gig24Xg4Ports},
    {SKERNEL_CHEETAH_3_FAMILY   ,26 ,ch3_26legalDevTypes, NULL, NULL,gig24Xg4Ports},
    {SKERNEL_CHEETAH_3_FAMILY   ,24 ,ch3_24legalDevTypes, NULL, NULL,gig24Xg4Ports},
    {SKERNEL_CHEETAH_3_FAMILY   ,26 ,ch3_6legalDevTypes,  &portsBmp0to3_24_25,NULL,gig24Xg4Ports},
    {SKERNEL_CHEETAH_3_FAMILY   ,26 ,ch3_10legalDevTypes, &portsBmp0to7_24_25,NULL,gig24Xg4Ports},
    {SKERNEL_CHEETAH_3_FAMILY   ,26 ,ch3_18legalDevTypes, &portsBmp0to15_24_25,NULL,gig24Xg4Ports},
    {SKERNEL_CHEETAH_3_FAMILY   ,28 ,ch3_20legalDevTypes, &portsBmp0to15_24to27,NULL,gig24Xg4Ports},
    {SKERNEL_CHEETAH_3_FAMILY   ,26 ,ch3_24_2legalDevTypes, NULL,NULL,gig24Xg4Ports},
    {SKERNEL_CHEETAH_3_FAMILY   ,28 ,ch3_24_4legalDevTypes, NULL,NULL,gig24Xg4Ports},
    {SKERNEL_CHEETAH_3_FAMILY   ,28 ,ch3_8_XGlegalDevTypes, &portsBmpCh3_8_Xg,NULL,xg28Ports},
    {SKERNEL_CHEETAH_3_FAMILY   ,28 ,ch3_XGlegalDevTypes, &portsBmpCh3Xg,NULL,xg28Ports},

    {SKERNEL_XCAT_FAMILY        ,24+4 ,xcat_24_4legalDevTypes, NULL,&portsBmpMultiState_24to27,gig24Xg4Ports},
    {SKERNEL_XCAT3_FAMILY       ,24+6+2 ,xcat3_24_6legalDevTypes, NULL,&portsBmpMultiState_24to29,xcat3gig24Xg6PortsAndCpuPort},
    {SKERNEL_AC5_FAMILY         ,24+6+2 ,ac5_24_6legalDevTypes  , NULL,&portsBmpMultiState_24to29,xcat3gig24Xg6PortsAndCpuPort},

    {SKERNEL_XCAT2_FAMILY       ,24+4 ,xcat2_24_4legalDevTypes, NULL,NULL,fast24Gig4Ports},

    /* support for the Lion's port groups -- 12 ports of XG (support also GE mode)*/
    {SKERNEL_LION_PORT_GROUP_FAMILY   ,12   ,lionPortGroup_12_legalDevTypes, NULL,&portsBmpMultiState_0to11,xg28Ports},
    /* support for the Lion (shell)*/
    {SKERNEL_LION_PORT_GROUP_SHARED_FAMILY   ,0    ,lion_48_legalDevTypes, NULL,NULL,NULL},

    {SKERNEL_PUMA_FAMILY   ,28 ,puma_26legalDevTypes, &portsBmp24_25_27,NULL,xg28Ports},
    {SKERNEL_PUMA_FAMILY   ,28 ,puma_24legalDevTypes, &portsBmp0to23_27,NULL,gig24Xg4Ports},


    /* support for the generic PHY shell */
    {SKERNEL_PHY_SHELL_FAMILY   ,0   ,phy_shell_legalDevTypes, NULL,NULL,NULL},
    /* support for the PHY core */
    {SKERNEL_PHY_CORE_FAMILY   ,2 /* single channel --> 2 ports*/   ,phy_core_legalDevTypes, NULL,NULL,gig24Xg4Ports},

    /* support macsec */
    {SKERNEL_MACSEC_FAMILY   ,8 /* 4 channels --> 8 ports*/   ,macsec_legalDevTypes, NULL,NULL,gig24Xg4Ports},

    {SKERNEL_EMPTY_FAMILY , 0 , empty_legalDevTypes , NULL,NULL,NULL},

    /* support for the Lion2's port groups -- 12 ports of XG (support also GE mode)*/
    {SKERNEL_LION2_PORT_GROUP_FAMILY   ,12   ,lion2PortGroup_12_legalDevTypes, NULL,&portsBmpMultiState_0to11,xg28Ports},
    /* support for the Lion2 (shell)*/
    {SKERNEL_LION2_PORT_GROUP_SHARED_FAMILY   ,0    ,lion2_96_legalDevTypes, NULL,NULL,NULL},

    /* support for the Lion3's port groups -- 12 ports of XG (support also GE mode)*/
    {SKERNEL_LION3_PORT_GROUP_FAMILY,12,lion3PortGroup_12_legalDevTypes, NULL,&portsBmpMultiState_0to11,xg28Ports},
    /* support for the Lion3 (shell)*/
    {SKERNEL_LION3_PORT_GROUP_SHARED_FAMILY,0,lion3_96_legalDevTypes, NULL,NULL,NULL},


    /* support for the Puma3's network+fabric -- 48 ports of XG (support also GE mode)*/
    {SKERNEL_PUMA3_NETWORK_FABRIC_FAMILY   ,48   ,puma3NetworkFabric_legalDevTypes, &portsBmpMultiState_0to11_32to47,&portsBmpMultiState_0to11_32to47,puma3Ports},
    /* support for the Lion2 (shell)*/
    {SKERNEL_PUMA3_SHARED_FAMILY   ,0    ,puma3_64_legalDevTypes, NULL,NULL,NULL},

    /* support for the BobCat2 */
    {SKERNEL_BOBCAT2_FAMILY, BOBCAT2_NUM_OF_MAC_PORTS_CNS, bobcat2_legalDevTypes, NULL, &portsBmpMultiState_48to71, ge48xg24Ports},

    /* support for the bobk-caelum */
    {SKERNEL_BOBK_CAELUM_FAMILY, BOBCAT2_NUM_OF_MAC_PORTS_CNS, bobk_caelum_legalDevTypes, NULL, &portsBmpMultiState_48to71, bobk_ge48xg12Ports},

    /* support for the bobk-cetus */
    {SKERNEL_BOBK_CETUS_FAMILY, BOBCAT2_NUM_OF_MAC_PORTS_CNS, bobk_cetus_legalDevTypes, &portsBmp56to59and62and64to71, &portsBmpMultiState_48to71, bobk_ge48xg12Ports},

    /* support for the bobk-aldrin */
    {SKERNEL_BOBK_ALDRIN_FAMILY, BOBCAT2_NUM_OF_MAC_PORTS_CNS, bobk_aldrin_legalDevTypes, NULL, &portsBmpMultiState_0to31, bobkAldrin_xg32Ports},

    /* support for the AC3X */
    {SKERNEL_AC3X_FAMILY, BOBCAT2_NUM_OF_MAC_PORTS_CNS, ac3x_legalDevTypes, NULL, &portsBmpMultiState_0to31, bobkAldrin_xg32Ports},

    /* support for the BobCat3 */
    {SKERNEL_BOBCAT3_FAMILY, BOBCAT3_NUM_OF_MAC_PORTS_CNS, bobcat3_legalDevTypes, NULL, &portsBmpMultiState_0to73, total74macs_cg36xg74Ports},

    /* support for the Aldrin2 */
    {SKERNEL_ALDRIN2_FAMILY, ALDRIN2_NUM_OF_MAC_PORTS_CNS, aldrin2_legalDevTypes, NULL, &portsBmpMultiState_0to72, total73macs_cg6xg73Ports},

    /* support for the PIPE devices */
    {SKERNEL_PIPE_FAMILY, PIPE_NUM_OF_MAC_PORTS_CNS, pipe_legalDevTypes, NULL, &portsBmpMultiState_0to15, pipe_Ports},

    /* support for the single tile Falcon */
    {SKERNEL_FALCON_FAMILY, FALCON_3_2_TERA_NUM_OF_MAC_PORTS_CNS, falconSingleTile_legalDevTypes, NULL, NULL, falcon_3_2_Tera_64_25G_and_1_12G_Ports},
    /* support for the 2 tiles Falcon */
    {SKERNEL_FALCON_FAMILY, FALCON_6_4_TERA_NUM_OF_MAC_PORTS_CNS, falcon2Tiles_legalDevTypes, NULL, NULL, falcon_6_4_Tera_128_25G_and_2_12G_Ports},
    /* support for the 4 tiles Falcon */
    {SKERNEL_FALCON_FAMILY, FALCON_12_8_TERA_NUM_OF_MAC_PORTS_CNS, falcon4Tiles_legalDevTypes, NULL, NULL, falcon_12_8_Tera_256_25G_and_2_12G_Ports},

    /* support for the Hawk */
    {SKERNEL_HAWK_FAMILY, HAWK_NUM_OF_MAC_PORTS_CNS, hawk_legalDevTypes, NULL, NULL, hawk_Ports},

    /* support for the Phoenix */
    {SKERNEL_PHOENIX_FAMILY, PHOENIX_NUM_OF_MAC_PORTS_CNS, phoenix_legalDevTypes, NULL, NULL, phoenix_Ports},

    /* support for the Harrier */
    {SKERNEL_HARRIER_FAMILY, HARRIER_NUM_OF_MAC_PORTS_CNS, harrier_legalDevTypes, NULL, NULL, harrier_Ports},

    /* support for the Ironman-L */
    {SKERNEL_IRONMAN_FAMILY, IRONMAN_L_NUM_OF_MAC_PORTS_CNS, ironman_l_legalDevTypes, NULL, NULL, ironman_l_Ports},

    /* support for the Ironman-S */
    {SKERNEL_IRONMAN_FAMILY, IRONMAN_S_NUM_OF_MAC_PORTS_CNS, ironman_s_legalDevTypes, NULL, NULL, ironman_s_Ports},

    /* End of list      */
    {END_OF_TABLE, 0, NULL, NULL}
};

/*const*/ static SIM_SUPPORTED_TYPES_STC onEmulator_simSupportedTypes[] =
{
    /* End of list      */
    {END_OF_TABLE, 0, NULL, NULL}
};


/* bmp of ports with no ports in it */
#define EMPTY_PORTS_BMP_CNS {{0,0}}

/* simSpecialDevicesBmp -
 * Purpose : DB to hold the devices with special ports BMP
 *
 * NOTE : devices that his port are sequential for 0 to the numOfPorts
 *        not need to be in this Array !!!
 *
 * fields :
 *  devType - device type that has special ports bmp (that is different from
 *            those of other devices of his family with the same numOfPort)
 *  existingPorts - the special ports bmp of the device
 *  multiModePortsBmp - ports bitmap that support multi state mode.
 *
*/
const static struct {
    SKERNEL_DEVICE_TYPE    devType;
    PORTS_BMP_STC          existingPorts;
    PORTS_BMP_STC          multiModePortsBmp;

}simSpecialDevicesBmp[] =
{
    /* 12 Giga ports (0.11) and XG 24,25 */
    {SKERNEL_98DX145 , {{PORT_RANGE_MAC(0,12) | PORT_RANGE_MAC(24,2) ,0}} , EMPTY_PORTS_BMP_CNS},
    /* 12 Giga ports (0.11) and XG 24 */
    {SKERNEL_98DX133 , {{PORT_RANGE_MAC(0,12) | PORT_RANGE_MAC(24,1) ,0}} , EMPTY_PORTS_BMP_CNS},
    /* No Giga ports and 3XG 24..26 */
    {SKERNEL_98DX803 , {{PORT_RANGE_MAC(24,3) ,0}} , EMPTY_PORTS_BMP_CNS},
    /* No Giga ports and 4XG 24..27 */
    {SKERNEL_98DX804 , {{PORT_RANGE_MAC(24,4) ,0}} , EMPTY_PORTS_BMP_CNS},

    /* End of list      */
    {END_OF_TABLE   ,{{0,0}} , EMPTY_PORTS_BMP_CNS}
};



#define NO_CPU_GMII_PORT_CNS 0xFFFFFFFF
#define SALSA_CPU_PORT_CNS  31
#define PRESTERA_CPU_PORT_CNS 63
#define XCAT3_CPU_PORT_CNS  31

/* device names database */
static SMAIN_DEV_NAME_STC    smainDevNameDb[] =
{
    /* salsa */
    {SMAIN_NO_GM_DEVICE, "98dx160", SKERNEL_98DX160,   16,SALSA_CPU_PORT_CNS},
    {SMAIN_NO_GM_DEVICE, "98dx161", SKERNEL_98DX161,   16,SALSA_CPU_PORT_CNS},
    {SMAIN_NO_GM_DEVICE, "98dx240", SKERNEL_98DX240,   24,SALSA_CPU_PORT_CNS},
    {SMAIN_NO_GM_DEVICE, "98dx241", SKERNEL_98DX241,   24,SALSA_CPU_PORT_CNS},
    /* salsa 2 */
    {SMAIN_NO_GM_DEVICE, "98dx1602", SKERNEL_98DX1602, 16,SALSA_CPU_PORT_CNS},
    {SMAIN_NO_GM_DEVICE, "98dx1612", SKERNEL_98DX1612, 16,SALSA_CPU_PORT_CNS},
    {SMAIN_NO_GM_DEVICE, "98dx2402", SKERNEL_98DX2402, 24,SALSA_CPU_PORT_CNS},
    {SMAIN_NO_GM_DEVICE, "98dx2412", SKERNEL_98DX2412, 24,SALSA_CPU_PORT_CNS},
    /* nic */
    {SMAIN_NO_GM_DEVICE, "nic",      SKERNEL_NIC,       1,NO_CPU_GMII_PORT_CNS},
    /* twist-d*/
    {SMAIN_NO_GM_DEVICE, "98ex100", SKERNEL_98EX100D,  28,NO_CPU_GMII_PORT_CNS},
    {SMAIN_NO_GM_DEVICE, "98ex110", SKERNEL_98EX110D,  52,NO_CPU_GMII_PORT_CNS},
    {SMAIN_NO_GM_DEVICE, "98ex115", SKERNEL_98EX115D,  52,NO_CPU_GMII_PORT_CNS},
    {SMAIN_NO_GM_DEVICE, "98ex120", SKERNEL_98EX120D,  12,NO_CPU_GMII_PORT_CNS},
    {SMAIN_NO_GM_DEVICE, "98ex125", SKERNEL_98EX125D,  52,NO_CPU_GMII_PORT_CNS},
    {SMAIN_NO_GM_DEVICE, "98ex130", SKERNEL_98EX130D,  1,NO_CPU_GMII_PORT_CNS},
    {SMAIN_NO_GM_DEVICE, "98ex135", SKERNEL_98EX135D,  1,NO_CPU_GMII_PORT_CNS},
    /* samba devices */
    {SMAIN_NO_GM_DEVICE, "98mx615", SKERNEL_98MX615D,  52,NO_CPU_GMII_PORT_CNS},
    {SMAIN_NO_GM_DEVICE, "98mx625", SKERNEL_98MX625D,  12,NO_CPU_GMII_PORT_CNS},
    {SMAIN_NO_GM_DEVICE, "98mx635", SKERNEL_98MX635D,  1,NO_CPU_GMII_PORT_CNS},
    /* TwistC devices */
    {SMAIN_NO_GM_DEVICE, "98mx610b", SKERNEL_98MX610B, 52,NO_CPU_GMII_PORT_CNS},
    {SMAIN_NO_GM_DEVICE, "98mx620b", SKERNEL_98MX620B, 10,NO_CPU_GMII_PORT_CNS},
    {SMAIN_NO_GM_DEVICE, "98ex110b", SKERNEL_98EX110B, 52,NO_CPU_GMII_PORT_CNS},
    {SMAIN_NO_GM_DEVICE, "98ex120b", SKERNEL_98EX120B, 12,NO_CPU_GMII_PORT_CNS},
    /* Sapphire */
    {SMAIN_NO_GM_DEVICE, "88e6183",  SKERNEL_SAPPHIRE, 10,NO_CPU_GMII_PORT_CNS},
    /* Ruby */
    {SMAIN_NO_GM_DEVICE, "88e6093",  SKERNEL_RUBY, 11,NO_CPU_GMII_PORT_CNS},
    /* Opal */
    {SMAIN_NO_GM_DEVICE, "88e6095", SKERNEL_OPAL, 11,NO_CPU_GMII_PORT_CNS},
    /* Opal Plus*/
    {SMAIN_NO_GM_DEVICE, "88e6097", SKERNEL_OPAL_PLUS, 11,NO_CPU_GMII_PORT_CNS},
    /*peridot*/
    {SMAIN_NO_GM_DEVICE, "peridot", SKERNEL_PERIDOT, 11,NO_CPU_GMII_PORT_CNS},
    /* smart PHY */
    {SMAIN_NO_GM_DEVICE, "88e1690", SKERNEL_PERIDOT, 11,NO_CPU_GMII_PORT_CNS},
    /* Jade */
    {SMAIN_NO_GM_DEVICE, "88e6185", SKERNEL_JADE, 10,NO_CPU_GMII_PORT_CNS},
    /* Tiger - Golden Model */
    {TIGER_GOLDEN_MODEL_MII , "98ex116gm", SKERNEL_98EX116, 52,NO_CPU_GMII_PORT_CNS},
    {TIGER_GOLDEN_MODEL_XII , "98ex126gm", SKERNEL_98EX126, 12,NO_CPU_GMII_PORT_CNS},
    /* Tiger */
    {SMAIN_NO_GM_DEVICE, "98ex116", SKERNEL_98EX116, 52,NO_CPU_GMII_PORT_CNS},
    {SMAIN_NO_GM_DEVICE, "98ex126", SKERNEL_98EX126, 12,NO_CPU_GMII_PORT_CNS},
    {SMAIN_NO_GM_DEVICE, "98ex136", SKERNEL_98EX136, 1,NO_CPU_GMII_PORT_CNS},
    /* Fabric Adapter */
    {SMAIN_NO_GM_DEVICE, "98fx900" , SKERNEL_98FX900 , 5,NO_CPU_GMII_PORT_CNS},
    {SMAIN_NO_GM_DEVICE, "98fx910" , SKERNEL_98FX910 , 5,NO_CPU_GMII_PORT_CNS},
    {SMAIN_NO_GM_DEVICE, "98fx915" , SKERNEL_98FX915 , 5,NO_CPU_GMII_PORT_CNS},
    {SMAIN_NO_GM_DEVICE, "98fx920" , SKERNEL_98FX920 , 5,NO_CPU_GMII_PORT_CNS} ,
    {SMAIN_NO_GM_DEVICE, "98fx930" , SKERNEL_98FX930 , 9,NO_CPU_GMII_PORT_CNS} ,
    {SMAIN_NO_GM_DEVICE, "98fx950" , SKERNEL_98FX950 , 9,NO_CPU_GMII_PORT_CNS} ,

    /* Xbar Adapter */
    {SMAIN_NO_GM_DEVICE, "98fx9110" , SKERNEL_98FX9110 , 12,NO_CPU_GMII_PORT_CNS},
    {SMAIN_NO_GM_DEVICE, "98fx9210" , SKERNEL_98FX9210 , 9,NO_CPU_GMII_PORT_CNS},
    {SMAIN_NO_GM_DEVICE, "98fx9310" , SKERNEL_98FX9310 , 64,NO_CPU_GMII_PORT_CNS},
    /* Cheetah */
    {SMAIN_NO_GM_DEVICE, "98dx270", SKERNEL_98DX270, 27,PRESTERA_CPU_PORT_CNS},
    {SMAIN_NO_GM_DEVICE, "98dx260", SKERNEL_98DX260, 26,PRESTERA_CPU_PORT_CNS},
    {SMAIN_NO_GM_DEVICE, "98dx250", SKERNEL_98DX250, 24,PRESTERA_CPU_PORT_CNS},
    {SMAIN_NO_GM_DEVICE, "98dx249", SKERNEL_98DX249, 27,PRESTERA_CPU_PORT_CNS},
    {SMAIN_NO_GM_DEVICE, "98dx269", SKERNEL_98DX269, 27,PRESTERA_CPU_PORT_CNS},
    /* Cheetah Plus */
    {SMAIN_NO_GM_DEVICE, "98dx163", SKERNEL_98DX163, 16,PRESTERA_CPU_PORT_CNS},
    {SMAIN_NO_GM_DEVICE, "98dx243", SKERNEL_98DX243, 24,PRESTERA_CPU_PORT_CNS},
    {SMAIN_NO_GM_DEVICE, "98dx253", SKERNEL_98DX253, 24,PRESTERA_CPU_PORT_CNS},
    {SMAIN_NO_GM_DEVICE, "98dx263", SKERNEL_98DX263, 26,PRESTERA_CPU_PORT_CNS},
    {SMAIN_NO_GM_DEVICE, "98dx273", SKERNEL_98DX273, 27,PRESTERA_CPU_PORT_CNS},
    {SMAIN_NO_GM_DEVICE, "98dx803", SKERNEL_98DX803, 27,PRESTERA_CPU_PORT_CNS},
    {SMAIN_NO_GM_DEVICE, "98dx133", SKERNEL_98DX133, 24,PRESTERA_CPU_PORT_CNS},
    /* Cheetah++ */
    {SMAIN_NO_GM_DEVICE, "98dx107" , SKERNEL_98DX107, 24,PRESTERA_CPU_PORT_CNS},
    /* Generic DXCH */
    {SMAIN_NO_GM_DEVICE, "dxch"  , SKERNEL_DXCH     , 27,PRESTERA_CPU_PORT_CNS},/* generic DXCH*/
    {SMAIN_NO_GM_DEVICE, "dxch_b0", SKERNEL_DXCH_B0 , 10,PRESTERA_CPU_PORT_CNS},/* generic DXCH_B0 */
    /* Generic DXCH2 */
    {SMAIN_NO_GM_DEVICE, "dxch2" , SKERNEL_DXCH2    , 28,PRESTERA_CPU_PORT_CNS},/* generic DXCH2*/
    /* Generic DXCH3 */
    {SMAIN_NO_GM_DEVICE, "dxch3" , SKERNEL_DXCH3    , 28,PRESTERA_CPU_PORT_CNS},/* generic DXCH3*/
    /* Generic DXCH3_XG */
    {SMAIN_NO_GM_DEVICE, "dxch3xg" , SKERNEL_DXCH3_XG    , 28,PRESTERA_CPU_PORT_CNS},/* generic DXCH3_XG*/
    /* Generic DXCH3P (cheetah 3+) */
    {SMAIN_NO_GM_DEVICE, "dxch3p" , SKERNEL_DXCH3P  , 28,PRESTERA_CPU_PORT_CNS},/* generic DXCH3 Plus*/
    /* Generic embedded CPU (cheetah 3+) */
    {SMAIN_NO_GM_DEVICE, "embedded_cpu" , SKERNEL_EMBEDDED_CPU , 1,NO_CPU_GMII_PORT_CNS},/* generic Embedded CPU */
    /* Generic xCat */
    {SMAIN_NO_GM_DEVICE, "xcat_24_and_4" , SKERNEL_XCAT_24_AND_4   , 28, PRESTERA_CPU_PORT_CNS},/* xCat 24+4(DX) */
    /* Generic xCat3 */
    {SMAIN_NO_GM_DEVICE, "xcat3_24_and_6" , SKERNEL_XCAT3_24_AND_6   , 30, XCAT3_CPU_PORT_CNS},/* xCat3 24+6(DX) */
    /* Generic AC5 */
    {SMAIN_NO_GM_DEVICE, "ac5_24_and_6" , SKERNEL_AC5_24_AND_6   , 30, XCAT3_CPU_PORT_CNS},/* ac5 24+6(DX) */
    /* Generic xCat3x */
    {SMAIN_NO_GM_DEVICE, "xcat3x" , SKERNEL_AC3X , BOBCAT2_NUM_OF_MAC_PORTS_CNS, NO_CPU_GMII_PORT_CNS},/* xCat3x  (not GM) - DX */

    /* Generic xCat2 */
    {SMAIN_NO_GM_DEVICE, "xcat2_24_and_4" , SKERNEL_XCAT2_24_AND_4   , 28, PRESTERA_CPU_PORT_CNS},/* xCat2 24+4(DX) */
    /* Generic XCat2 - GM */
    {GOLDEN_MODEL, "xcat2_24_and_4gm" , SKERNEL_XCAT2_24_AND_4   , 28, PRESTERA_CPU_PORT_CNS},/* xCat2 24+4(DX) */
    /* Generic Lion's port group */
    {SMAIN_NO_GM_DEVICE, "lion_port_group_12" , SKERNEL_LION_PORT_GROUP_12    , 12,PRESTERA_CPU_PORT_CNS},/* Lion's port group 12 ports(DX) */
    /* Generic Lion device */
    {SMAIN_NO_GM_DEVICE, "lion_48" , SKERNEL_LION_48 , 0/*no own ports--> use port group*/,NO_CPU_GMII_PORT_CNS},/* Lion (DX) */
    /* Generic Lion's port group - GM */
    {GOLDEN_MODEL, "lion_port_group_12gm" , SKERNEL_LION_PORT_GROUP_12    , 12,PRESTERA_CPU_PORT_CNS},/* Lion's port group 12 ports(DX) */

    /* Generic Lion2's port group */
    {SMAIN_NO_GM_DEVICE, "lion2_port_group_12" , SKERNEL_LION2_PORT_GROUP_12    , 12,PRESTERA_CPU_PORT_CNS},/* Lion2's port group 12 ports(DX) */
    /* Generic Lion2 device */
    {SMAIN_NO_GM_DEVICE, "lion2_96" , SKERNEL_LION2_96 , 0/*no own ports--> use port group*/,NO_CPU_GMII_PORT_CNS},/* Lion2 (DX) */
    /* Generic Lion2's port group - GM  */
    {GOLDEN_MODEL, "lion2_port_group_12gm" , SKERNEL_LION2_PORT_GROUP_12    , 12,PRESTERA_CPU_PORT_CNS},/* Lion2's port group 12 ports(DX) */

    /* Generic Lion3's port group */
    {SMAIN_NO_GM_DEVICE, "lion3_port_group_12" , SKERNEL_LION3_PORT_GROUP_12    , 12,PRESTERA_CPU_PORT_CNS},/* Lion3's port group 12 ports(DX) */
    /* Generic Lion3 device */
    {SMAIN_NO_GM_DEVICE, "lion3_96" , SKERNEL_LION3_96 , 0/*no own ports--> use port group*/,NO_CPU_GMII_PORT_CNS},/* Lion3 (DX) */
    /* Generic Lion3's port group - GM  */
    {GOLDEN_MODEL, "lion3_port_group_12gm" , SKERNEL_LION3_PORT_GROUP_12    , 12,PRESTERA_CPU_PORT_CNS},/* Lion3's port group 12 ports(DX) */

    /* Generic Bobcat2 - GM */
    {GOLDEN_MODEL   , "bobcat2_gm" , SKERNEL_BOBCAT2, BOBCAT2_NUM_OF_MAC_PORTS_CNS, NO_CPU_GMII_PORT_CNS},/* Bobcat2 GM - DX */
    /* Generic Bobcat2 (non GM device)*/
    {SMAIN_NO_GM_DEVICE, "bobcat2" , SKERNEL_BOBCAT2, BOBCAT2_NUM_OF_MAC_PORTS_CNS, NO_CPU_GMII_PORT_CNS},/* Bobcat2 (not GM) - DX */

    /* Generic Bobcat2 (non GM device)*/
    {SMAIN_NO_GM_DEVICE, "bobk-caelum" , SKERNEL_BOBK_CAELUM, BOBCAT2_NUM_OF_MAC_PORTS_CNS, NO_CPU_GMII_PORT_CNS},/* Bobk-Caelum (not GM) - DX */
    {SMAIN_NO_GM_DEVICE, "bobk-cetus"  , SKERNEL_BOBK_CETUS , BOBCAT2_NUM_OF_MAC_PORTS_CNS, NO_CPU_GMII_PORT_CNS},/* Bobk-Cetus  (not GM) - DX */
    {SMAIN_NO_GM_DEVICE, "bobk-aldrin" , SKERNEL_BOBK_ALDRIN , BOBCAT2_NUM_OF_MAC_PORTS_CNS, NO_CPU_GMII_PORT_CNS},/* Bobk-Aldrin  (not GM) - DX */

    {GOLDEN_MODEL   , "bobk-caelum_gm" , SKERNEL_BOBK_CAELUM, BOBCAT2_NUM_OF_MAC_PORTS_CNS, NO_CPU_GMII_PORT_CNS},/* Bobk-Caelum GM - DX */
    {GOLDEN_MODEL   , "bobk-cetus_gm"  , SKERNEL_BOBK_CETUS , BOBCAT2_NUM_OF_MAC_PORTS_CNS, NO_CPU_GMII_PORT_CNS},/* Bobk-Cetus  GM - DX */

    /* Generic Bobcat3 (non GM device)*/
    {SMAIN_NO_GM_DEVICE, "bobcat3" , SKERNEL_BOBCAT3, BOBCAT3_NUM_OF_MAC_PORTS_CNS, NO_CPU_GMII_PORT_CNS},/* Bobcat3 (not GM) - DX */
    /* Generic Bobcat3 - GM */
    {GOLDEN_MODEL   , "bobcat3_gm" , SKERNEL_BOBCAT3, BOBCAT3_NUM_OF_MAC_PORTS_CNS, NO_CPU_GMII_PORT_CNS},/* Bobcat3 GM - DX */

    /* Generic Aldrin2 (non GM device)*/
    {SMAIN_NO_GM_DEVICE, "aldrin2" , SKERNEL_ALDRIN2, ALDRIN2_NUM_OF_MAC_PORTS_CNS, NO_CPU_GMII_PORT_CNS},/* aldrin2 (not GM) - DX */
    /* Generic Aldrin2 - GM */
    {GOLDEN_MODEL   , "aldrin2_gm" , SKERNEL_ALDRIN2, ALDRIN2_NUM_OF_MAC_PORTS_CNS, NO_CPU_GMII_PORT_CNS},/* aldrin2 GM - DX */

    /* Cheetah2 */
    {SMAIN_NO_GM_DEVICE, "98dx255" , SKERNEL_98DX255, 24,PRESTERA_CPU_PORT_CNS},
    {SMAIN_NO_GM_DEVICE, "98dx265" , SKERNEL_98DX265, 26,PRESTERA_CPU_PORT_CNS},
    {SMAIN_NO_GM_DEVICE, "98dx275" , SKERNEL_98DX275, 27,PRESTERA_CPU_PORT_CNS},
    {SMAIN_NO_GM_DEVICE, "98dx285" , SKERNEL_98DX285, 28,PRESTERA_CPU_PORT_CNS},
    {SMAIN_NO_GM_DEVICE, "98dx804" , SKERNEL_98DX804, 28,PRESTERA_CPU_PORT_CNS},
    /* Cheetah3 */
    {SMAIN_NO_GM_DEVICE, "98dx286" , SKERNEL_98DX5128,  28,PRESTERA_CPU_PORT_CNS},
    {SMAIN_NO_GM_DEVICE, "98dx806" , SKERNEL_98DX806,   28,PRESTERA_CPU_PORT_CNS},
    {SMAIN_NO_GM_DEVICE, "98dx5128", SKERNEL_98DX5128,  28,PRESTERA_CPU_PORT_CNS},
    {SMAIN_NO_GM_DEVICE, "98dx5128_1", SKERNEL_98DX5128_1,28,PRESTERA_CPU_PORT_CNS},
    {SMAIN_NO_GM_DEVICE, "98dx5124", SKERNEL_98DX5124,  28,PRESTERA_CPU_PORT_CNS},
    {SMAIN_NO_GM_DEVICE, "98dx5126", SKERNEL_98DX5126,  28,PRESTERA_CPU_PORT_CNS},
    {SMAIN_NO_GM_DEVICE, "98dx5127", SKERNEL_98DX5127,  28,PRESTERA_CPU_PORT_CNS},
    {SMAIN_NO_GM_DEVICE, "98dx5129", SKERNEL_98DX5129,  28,PRESTERA_CPU_PORT_CNS},
    {SMAIN_NO_GM_DEVICE, "98dx5151", SKERNEL_98DX5151,  28,PRESTERA_CPU_PORT_CNS},
    {SMAIN_NO_GM_DEVICE, "98dx5152", SKERNEL_98DX5152,  28,PRESTERA_CPU_PORT_CNS},
    {SMAIN_NO_GM_DEVICE, "98dx5154" , SKERNEL_98DX5154, 28,PRESTERA_CPU_PORT_CNS},
    {SMAIN_NO_GM_DEVICE, "98dx5155" , SKERNEL_98DX5155, 28,PRESTERA_CPU_PORT_CNS},
    {SMAIN_NO_GM_DEVICE, "98dx5156", SKERNEL_98DX5156,  28,PRESTERA_CPU_PORT_CNS},
    {SMAIN_NO_GM_DEVICE, "98dx5157" , SKERNEL_98DX5157, 28,PRESTERA_CPU_PORT_CNS},
    {SMAIN_NO_GM_DEVICE, "98dx8108" , SKERNEL_98DX8108, 28,PRESTERA_CPU_PORT_CNS},
    {SMAIN_NO_GM_DEVICE, "98dx8109", SKERNEL_98DX8109,  28,PRESTERA_CPU_PORT_CNS},
    {SMAIN_NO_GM_DEVICE, "98dx8110", SKERNEL_98DX8110,  28,PRESTERA_CPU_PORT_CNS},
    {SMAIN_NO_GM_DEVICE, "98dx8110_1", SKERNEL_98DX8110_1,28,PRESTERA_CPU_PORT_CNS},

    /* Puma */
    {SMAIN_NO_GM_DEVICE, "98ex240"   , SKERNEL_98EX240, 28,NO_CPU_GMII_PORT_CNS},
    {GOLDEN_MODEL      , "98ex240gm" , SKERNEL_98EX240, 28,NO_CPU_GMII_PORT_CNS},
    {GOLDEN_MODEL      , "98ex260gm" , SKERNEL_98EX241, 28,NO_CPU_GMII_PORT_CNS},

    /* Generic Puma3 device */
    {SMAIN_NO_GM_DEVICE, "puma3_64" , SKERNEL_PUMA3_64 , 0/*no own ports--> use port group*/,NO_CPU_GMII_PORT_CNS},
    {GOLDEN_MODEL      , "puma3_nw_fa_gm" , SKERNEL_PUMA3_NETWORK_FABRIC, 48,NO_CPU_GMII_PORT_CNS},

    /* Generic PHY multi channels */
    {SMAIN_NO_GM_DEVICE, "phy" , SKERNEL_PHY_SHELL , 0/*no own ports--> use port group*/,NO_CPU_GMII_PORT_CNS},
    /* PHY core 1540M (single channel) */
    {SMAIN_NO_GM_DEVICE, "phy_core_1540m_1548m" , SKERNEL_PHY_CORE_1540M_1548M , 2 /*single channel - 2 ports*/,NO_CPU_GMII_PORT_CNS},
    /* MACSEC (MAC-security) - 'linkCrypt' */
    {SMAIN_NO_GM_DEVICE, "macsec" , SKERNEL_MACSEC , 8 /* 4 channels - 8 ports*/,NO_CPU_GMII_PORT_CNS},

    /* non-exists device -- for 'skip' devices  */
    {SMAIN_NO_GM_DEVICE, "empty" , SKERNEL_EMPTY , 0,NO_CPU_GMII_PORT_CNS},

    /* Generic PIPE (non GM device)*/
    {SMAIN_NO_GM_DEVICE, "pipe" , SKERNEL_PIPE, PIPE_NUM_OF_MAC_PORTS_CNS, NO_CPU_GMII_PORT_CNS},/* PIPE (not GM) - PX */

    /* Generic Falcon (non GM device)*/
    {SMAIN_NO_GM_DEVICE, "falcon_single_tile" , SKERNEL_FALCON_SINGLE_TILE, FALCON_3_2_TERA_NUM_OF_MAC_PORTS_CNS, NO_CPU_GMII_PORT_CNS},/* Falcon (not GM) - DX */
    {SMAIN_NO_GM_DEVICE, "falcon_2_tiles"     , SKERNEL_FALCON_2_TILES    , FALCON_6_4_TERA_NUM_OF_MAC_PORTS_CNS, NO_CPU_GMII_PORT_CNS},/* Falcon (not GM) - DX */
    {SMAIN_NO_GM_DEVICE, "falcon_4_tiles"     , SKERNEL_FALCON_4_TILES    , FALCON_12_8_TERA_NUM_OF_MAC_PORTS_CNS, NO_CPU_GMII_PORT_CNS},/* Falcon (not GM) - DX */
    /* Generic Falcon - GM */
    {GOLDEN_MODEL   , "falcon_single_tile_gm" , SKERNEL_FALCON_SINGLE_TILE, FALCON_3_2_TERA_NUM_OF_MAC_PORTS_CNS, NO_CPU_GMII_PORT_CNS},/* Falcon GM - DX */

    /* generic Hawk (non GM device) - DX device */
    {SMAIN_NO_GM_DEVICE, "hawk"    , SKERNEL_HAWK, HAWK_NUM_OF_MAC_PORTS_CNS, NO_CPU_GMII_PORT_CNS},
    /* generic Hawk - GM            - DX device */
    {GOLDEN_MODEL      , "hawk_gm" , SKERNEL_HAWK, HAWK_NUM_OF_MAC_PORTS_CNS, NO_CPU_GMII_PORT_CNS},

    /* generic phoenix (non GM device) - DX device */
    {SMAIN_NO_GM_DEVICE, "phoenix"    , SKERNEL_PHOENIX, PHOENIX_NUM_OF_MAC_PORTS_CNS, NO_CPU_GMII_PORT_CNS},
    /* generic phoenix - GM            - DX device */
    {GOLDEN_MODEL      , "phoenix_gm" , SKERNEL_PHOENIX, PHOENIX_NUM_OF_MAC_PORTS_CNS, NO_CPU_GMII_PORT_CNS},

    /* generic Harrier (non GM device) - DX device */
    {SMAIN_NO_GM_DEVICE, "harrier"    , SKERNEL_HARRIER, HARRIER_NUM_OF_MAC_PORTS_CNS, NO_CPU_GMII_PORT_CNS},
    /* generic Harrier - GM            - DX device */
    {GOLDEN_MODEL      , "harrier_gm" , SKERNEL_HARRIER, HARRIER_NUM_OF_MAC_PORTS_CNS, NO_CPU_GMII_PORT_CNS},
    /* generic Harrier (non GM device) - DX device */
    {SMAIN_NO_GM_DEVICE, "aldrin3m"    , SKERNEL_HARRIER, HARRIER_NUM_OF_MAC_PORTS_CNS, NO_CPU_GMII_PORT_CNS},

    /* generic ironman-l (non GM device) - DX device */
    {SMAIN_NO_GM_DEVICE, "ironman-l"    , SKERNEL_IRONMAN_L, IRONMAN_L_NUM_OF_MAC_PORTS_CNS, NO_CPU_GMII_PORT_CNS},
    /* generic ironman-s (non GM device) - DX device */
    {SMAIN_NO_GM_DEVICE, "ironman-s"    , SKERNEL_IRONMAN_S, IRONMAN_S_NUM_OF_MAC_PORTS_CNS, NO_CPU_GMII_PORT_CNS},

    /* generic ironman-l (GM device) - DX device */
    {GOLDEN_MODEL, "ironman_l_gm"    , SKERNEL_IRONMAN_L, IRONMAN_L_NUM_OF_MAC_PORTS_CNS, NO_CPU_GMII_PORT_CNS},


    /* Communication Module */
    {SMAIN_NO_GM_DEVICE  , "communication_card", SKERNEL_COM_MODULE,0,NO_CPU_GMII_PORT_CNS},
};

#define SMAIN_DEV_NAME_DB_SIZE (sizeof(smainDevNameDb)/sizeof(smainDevNameDb[1]))

#define INI_FILE_SLAN_FOR_NON_EXIST_PORT_CHECK_MAC(_nonExistPort,_port,_deviceId)    \
            if((_nonExistPort) == GT_TRUE)                          \
            {                                                       \
                if(deviceObjPtr->deviceFamily == SKERNEL_PUMA3_NETWORK_FABRIC_FAMILY) \
                {                                                   \
                    /* no warning -- valid case for puma3*/         \
                }                                                   \
                else                                                \
                {                                                   \
                    simWarningPrintf(" smainDevice2SlanBind : non-exist port [%d] in device [%d] , still you try to bind it to SLAN ?! \n",\
                             (_port) , (_deviceId));                \
                }                                                   \
                continue;                                           \
            }

/* device objects database */
SKERNEL_DEVICE_OBJECT * smainDeviceObjects[SMAIN_MAX_NUM_OF_DEVICES_CNS];

/* struct to hold info about the several buses in the system*/
typedef struct{
    GT_U32      id;   /* the bus id */
    GT_U32      membersBmp[(SMAIN_MAX_NUM_OF_DEVICES_CNS+31)/32];/* bmp of devices that members in the bus */
}BUS_INFO_STC;

#define MAX_BUS_NUM_CNS 32
static BUS_INFO_STC busInfoArr[MAX_BUS_NUM_CNS];
static GT_U32       numberOfBuses = 0;/* current number of buses */

/* slan down semaphore */
static GT_SEM slanDownSemaphore;

/* NIC Rx callback */
static SKERNEL_NIC_RX_CB_FUN smainNicRxCb;

/* NIC Rx callback */
static SKERNEL_DEVICE_OBJECT * smainNicDeviceObjPtr;

/* enable or disable Tx to Visual. Default enable */
static GT_U32 smainVisualDisabled;

static void smainNicRxHandler
(
    IN GT_U8_PTR   segmentPtr,
    IN GT_U32      segmentLen
);

static GT_STATUS deviceTypeInfoGet
(
    INOUT  SKERNEL_DEVICE_OBJECT * devObjPtr
);

static GT_STATUS smainDeviceRevisionGet
(
    IN SKERNEL_DEVICE_OBJECT    * deviceObjPtr,
    IN GT_U32                   devId
);
/* string name of the device families */
static char* familyNamesArr[] =
{
    "SKERNEL_NOT_INITIALIZED_FAMILY",
    "SKERNEL_SALSA_FAMILY",
    "SKERNEL_NIC_FAMILY",
    "SKERNEL_COM_MODULE_FAMILY",
    "SKERNEL_TWIST_D_FAMILY",
    "SKERNEL_TWIST_C_FAMILY",
    "SKERNEL_SAMBA_FAMILY",
    "SKERNEL_SOHO_FAMILY",
    "SKERNEL_CHEETAH_1_FAMILY",
    "SKERNEL_CHEETAH_2_FAMILY",
    "SKERNEL_CHEETAH_3_FAMILY",
    "SKERNEL_LION_PORT_GROUP_SHARED_FAMILY",
    "SKERNEL_LION_PORT_GROUP_FAMILY",
    "SKERNEL_XCAT_FAMILY",
    "SKERNEL_XCAT2_FAMILY",
    "SKERNEL_TIGER_FAMILY",
    "SKERNEL_FA_FOX_FAMILY",
    "SKERNEL_FAP_DUNE_FAMILY",
    "SKERNEL_XBAR_CAPOEIRA_FAMILY",
    "SKERNEL_FE_DUNE_FAMILY",
    "SKERNEL_PUMA_FAMILY",
    "SKERNEL_EMBEDDED_CPU_FAMILY",

    "SKERNEL_PHY_SHELL_FAMILY",
    "SKERNEL_PHY_CORE_FAMILY",

    "SKERNEL_MACSEC_FAMILY",

    "SKERNEL_LION2_PORT_GROUP_SHARED_FAMILY",
    "SKERNEL_LION2_PORT_GROUP_FAMILY",

    "SKERNEL_LION3_PORT_GROUP_SHARED_FAMILY",
    "SKERNEL_LION3_PORT_GROUP_FAMILY",

    "SKERNEL_PUMA3_SHARED_FAMILY",
    "SKERNEL_PUMA3_NETWORK_FABRIC_FAMILY",

    "SKERNEL_BOBCAT2_FAMILY",
    "SKERNEL_BOBK_CAELUM_FAMILY",
    "SKERNEL_BOBK_CETUS_FAMILY",
    "SKERNEL_BOBK_ALDRIN_FAMILY",
    "SKERNEL_AC3X_FAMILY",

    "SKERNEL_BOBCAT3_FAMILY",

    "SKERNEL_ALDRIN2_FAMILY",

    "SKERNEL_FALCON_FAMILY",

    "SKERNEL_HAWK_FAMILY",

    "SKERNEL_PHOENIX_FAMILY",

    "SKERNEL_XCAT3_FAMILY",
    "SKERNEL_AC5_FAMILY"  ,

    "SKERNEL_HARRIER_FAMILY",

    "SKERNEL_IRONMAN_FAMILY",

    "SKERNEL_PIPE_FAMILY",

    "SKERNEL_EMPTY_FAMILY",

    " *** "
};
/*
debug tool to be able to emulte remote TM for basic testing of
simulation that works with remote TM (udp sockets and asynchronous
messages)*/
static GT_U32   emulateRemoteTm = 1;

static SKERNEL_DEVICE_OBJECT *   skernelDeviceSoftReset
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr
);
static SKERNEL_DEVICE_OBJECT *   skernelDeviceSoftResetPart2
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr
);

/* indication that the application asked to remove all outer connections */
/* this indication needed after soft reset to know if to allow SLANs and other connections to restore or not */
static GT_U32 simDisconnectAllOuterPorts_mode = 0;

static GT_U32 skernelIsLionShellOnlyDev(IN SKERNEL_DEVICE_OBJECT * devObjPtr);
static GT_U32 skernelIsPumaShellOnlyDev(IN SKERNEL_DEVICE_OBJECT * devObjPtr);
/**
* @internal calcSlanBufSize function
* @endinternal
*
* @brief   Calculate length of buffer needed for given frame
*
* @param[in] origLen                  - original length of arrived frame
* @param[in] deviceObj                - the device object
* @param[in] use_prependNumBytes      - indication to use deviceObj->prependNumBytes in calculations
*           the flag needed for fix : IPBUSW-8538 : WM-ASIM: White Model adding 2 unexpected additional bytes
*
* @retval GT_U32                   - length of buffer
*
* @note Main purpose of this function is to calculate length of buffer for
*       undersized [ < 60+4CRC bytes] frames - it can be frame sent as
*       undersized and it could be frame which CRC was cut off by NIC
*       Actually this function should be engaged every time when frame comes
*       from outside before call to sbufDataSet
*
*/
static GT_U32 calcSlanBufSize
(
    IN GT_U32 origLen,
    IN SKERNEL_DEVICE_OBJECT * deviceObj,
    IN GT_U32   use_prependNumBytes
)
{
    GT_U32 actualLenNeeded;/* actual length needed for the frame that got from
                            * slan with the length of origLen
                            */

    /* check if need to prepend bytes for 64 bytes
     * (because egress will not send less than 64)
     * so we need to allocate the space for it
     */
    actualLenNeeded = (origLen < SGT_MIN_FRAME_LEN) ? SGT_MIN_FRAME_LEN : origLen;

    if(use_prependNumBytes)
    {
        actualLenNeeded += deviceObj->prependNumBytes;
    }

    /* add 4 FCS (frame check sum or CRC) bytes to frame from SLAN */
    if (deviceObj->crcPortsBytesAdd == 1)
    {
        actualLenNeeded += 4;
    }

    return actualLenNeeded;
}

/************** private functions ********************************************/
static void smainInterruptsMaskChanged(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32  causeRegAddr,
    IN GT_U32  maskRegAddr,
    IN GT_U32  intRegBit,
    IN GT_U32  currentCauseRegVal,
    IN GT_U32  lastMaskRegVal,
    IN GT_U32  newMaskRegVal
);

extern void snetCheetahInterruptsMaskChanged(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32  causeRegAddr,
    IN GT_U32  maskRegAddr,
    IN GT_U32  intRegBit,
    IN GT_U32  currentCauseRegVal,
    IN GT_U32  lastMaskRegVal,
    IN GT_U32  newMaskRegVal
);

extern void snetTigerInterruptsMaskChanged(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32  causeRegAddr,
    IN GT_U32  maskRegAddr,
    IN GT_U32  intRegBit,
    IN GT_U32  currentCauseRegVal,
    IN GT_U32  lastMaskRegVal,
    IN GT_U32  newMaskRegVal
);

extern void snetSohoInterruptsMaskChanged(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32  causeRegAddr,
    IN GT_U32  maskRegAddr,
    IN GT_U32  intRegBit,
    IN GT_U32  currentCauseRegVal,
    IN GT_U32  lastMaskRegVal,
    IN GT_U32  newMaskRegVal
);

#ifdef _WIN32
extern void SHOSTG_psos_reg_asic_task(void);
#endif /*_WIN32*/

#ifdef _VISUALC
/*
MSVC++ 10.0  _MSC_VER = 1600
MSVC++ 9.0   _MSC_VER = 1500
MSVC++ 8.0   _MSC_VER = 1400
MSVC++ 7.1   _MSC_VER = 1310
MSVC++ 7.0   _MSC_VER = 1300
MSVC++ 6.0   _MSC_VER = 1200
MSVC++ 5.0   _MSC_VER = 1100
*/

#if _MSC_VER >= 1600 /* from VC 10 */
    #define  strlwr _strlwr
#endif

#endif /*_VISUALC*/

extern char * strlwr(char*);

static void smainReceivedPacketDoneFromTm
(
    IN GT_U32   simDeviceId,
    IN GT_VOID* cookiePtr,
    IN GT_U32   tmFinalPort
);

/**
* @internal skernelNumOfPacketsInTheSystemSet function
* @endinternal
*
* @brief   increment/decrement the number of packet in the system
*
* @param[in] increment                - GT_TRUE  -  the number of packet in the system
*                                      GT_FALSE - decrement the number of packet in the system
*/
GT_VOID skernelNumOfPacketsInTheSystemSet(
    IN GT_BOOL     increment)
{
    SKERNEL_DEVICE_OBJECT * devObjPtr = NULL;
    SCIB_SEM_TAKE;

    if(increment == GT_TRUE)
    {
        skernelNumOfPacketsInTheSystem++;
    }
    else
    {
        if(skernelNumOfPacketsInTheSystem == 0)
        {
            skernelFatalError("skernelNumOfPacketsInTheSystemSet: can't decrement from 0 \n");
        }
        else
        {
            skernelNumOfPacketsInTheSystem--;
        }
    }

    /* save to the LOG the number of packets in the system */
    __LOG_PARAM_NO_LOCATION_META_DATA((skernelNumOfPacketsInTheSystem));

    SCIB_SEM_SIGNAL;
}

/**
* @internal skernelNumOfPacketsInTheSystemGet function
* @endinternal
*
* @brief   Get the number of packet in the system
*/
GT_U32 skernelNumOfPacketsInTheSystemGet(GT_VOID)
{
    return skernelNumOfPacketsInTheSystem;
}


/**
* @internal smainGmRegSet function
* @endinternal
*
* @brief   GM 'wrapper' to write register with 'Address Completion'.
*         assuming that the device currently work in mode of 'old address completion'
*         (with register 0)
* @param[in] deviceObjPtr             - pointer to device object.
* @param[in] address                  -  of register.
* @param[in] data                     - new register's value.
*/
static void smainGmRegSet
(
    IN SKERNEL_DEVICE_OBJECT * deviceObjPtr,
    IN GT_U32                  address,
    IN GT_U32                  data
)
{
    GT_U32  value;
    GT_U32  windowIndex = 3;
    GT_U32  addressCompletionReg = 0;

    /* set the unit into the proper window */
    value = ((address >> 24) & 0xFF) << (8*windowIndex);
    /* write to address completion : set the unit of the register */
    scibWriteMemory(deviceObjPtr->deviceId, addressCompletionReg, 1, &value );

    /* offset in the unit + the window index value */
    value = (windowIndex << 24) | (address & 0xFFFFFF);
    /* write the data to the offset of the address in the unit */
    scibWriteMemory(deviceObjPtr->deviceId, value, 1, &data );

    value = 0;
    /* reset the address completion */
    scibWriteMemory(deviceObjPtr->deviceId, addressCompletionReg, 1, &value );

}
/**
* @internal smainMemDefaultsLoad function
* @endinternal
*
* @brief   Load default values for memory from file
*
* @param[in] deviceObjPtr             - pointer to the device object.
* @param[in] fileNamePtr              - default values file name.
*/
void smainMemDefaultsLoad
(
    IN SKERNEL_DEVICE_OBJECT    * deviceObjPtr,
    IN GT_CHAR                  * fileNamePtr

)
{
    FILE    * regFilePtr;  /* file object */
    static char  buffer[FILE_MAX_LINE_LENGTH_CNS]; /* string buffer */
    GT_U32 unitAddr = 0;   /* unit address for register, 0 means not valid*/
    GT_U32 baseAddr;       /* base address for register */
    GT_U32 value;     /* default value */
    GT_32  number;    /* number of registers */
    GT_U32 addrDelta; /* delta for several registers */
    GT_U32 scanRes;   /* number scanned elements */
    GT_32    i;         /* iterator */
    GT_U32 address;   /* address of particular register */
    char *  remarkStr;
    GT_CHAR pciAddr[SMAIN_PCI_FLAG_BUFF_SIZE];
    GT_U32  currentLineIndex=0;/*current line number in the 'registers file'*/

    if(SMEM_SKIP_LOCAL_READ_WRITE)
    {
        /* don't write to memory , memory is not valid for it */
        return;
    }

    if (!fileNamePtr)
       return;

    /* open file */
    regFilePtr = fopen(fileNamePtr,"rt");
    if (regFilePtr == NULL)
    {
        skernelFatalError("smainMemDefaultsLoad: registers file not found %s\n", fileNamePtr);
    }

    while (fgets(buffer, FILE_MAX_LINE_LENGTH_CNS, regFilePtr))
    {
        currentLineIndex++;/* 1 based number */
        /* convert to lower case */
        strlwr(buffer);

        remarkStr = strstr(buffer,";");
        if (remarkStr == buffer)
        {
            continue;
        }
        /* UNIT_BASE_ADDR string detection */
        if(!sscanf(buffer, "unit_base_addr %x\n", &unitAddr))
        {
            if(strstr(buffer, "unit_base_addr not_valid"))
            {
                unitAddr = 0;
                continue;
            }
        }
        else
        {
            continue;
        }

        /* record found, parse it */
        baseAddr = 0;
        value = 0;
        number = 0;
        addrDelta = 0; /* if scanRes < 4 => addrDelta is undefined */
        memset(pciAddr, 0, SMAIN_PCI_FLAG_BUFF_SIZE);
        scanRes = sscanf(buffer, "%x %x %d %x %10s", &baseAddr,
                        &value, &number, &addrDelta, pciAddr);

        if (baseAddr == SMAIN_FILE_RECORDS_EOF)
        {
            break;
        }
        if (((baseAddr & 0xFF000000) != 0) && (unitAddr != 0))
        {
            skernelFatalError("When UNIT_BASE_ADDR is valid then the address of register must be 'zero based' %s (%d)\n", fileNamePtr, currentLineIndex);
         }
        baseAddr += unitAddr;
        if ((scanRes < 2) || (scanRes == 3) ||(scanRes > 5))
        {
            /* check end of file */
            if (scanRes == 1)
                break;

            simWarningPrintf("smainMemDefaultsLoad: registers file's bad format in line [%d]\n",currentLineIndex);
            break;
        }

        if(scanRes == 2)
        {
            number = 1;
        }
        /* set registers */
        for(i = 0; i < number; i++)
        {
            address = i * addrDelta + baseAddr;
            if ((strcmp(pciAddr,"dfx") == 0) || (strcmp(pciAddr,"DFX") == 0))
            {
                /* NOTE: the next function not triggering the 'active memory'
                         mechanism , because the purpose is to start with those
                         registers values , and not to activate actions...
                */
                smemDfxRegSet(deviceObjPtr,address,value);
            }
            else
            if ((strcmp(pciAddr,"pci") == 0) || (strcmp(pciAddr,"PCI") == 0))
            {
                /* NOTE: the next function not triggering the 'active memory'
                         mechanism , because the purpose is to start with those
                         registers values , and not to activate actions...
                */
                smemPciRegSet(deviceObjPtr,address,value);
            }
            else
            {
                /* NOTE: the next function not triggering the 'active memory'
                         mechanism , because the purpose is to start with those
                         registers values , and not to activate actions...
                */

                if((i == 0) &&
                    GT_FALSE == smemIsDeviceMemoryOwner(deviceObjPtr,address))
                {
                    /* this chunk not relevant to this device */
                    /* optimize INIT time of 'shared memory' */
                    break;
                }

                if(deviceObjPtr->gmDeviceType == GOLDEN_MODEL)
                {
                    /* called via 'wrapper' to break the write to support 'Address Completion'
                       to properly redirect the call to GM device */
                    smainGmRegSet(deviceObjPtr, address, value);
                }
                else
                {
                    smemRegSet(deviceObjPtr,address,value);
                    /* call need to be done inside the skernel without the SCIB */
                    /*scibWriteMemory(deviceObjPtr->deviceId, address, 1, &value);*/
                }
            }
        }
    }

    fclose(regFilePtr);
}


/**
* @internal smainMemConfigLoad function
* @endinternal
*
* @brief   Load memory configuration from file
*
* @param[in] deviceObjPtr             - pointer to the device object.
* @param[in] fileNamePtr              - default values file name.
* @param[in] isSmemUsed               - whether to use smem func instead of scib
*/
void smainMemConfigLoad
(
    IN SKERNEL_DEVICE_OBJECT    *deviceObjPtr,
    IN GT_CHAR                  *fileNamePtr,
    IN GT_BOOL                   isSmemUsed
)
{
    FILE    * regFilePtr;  /* file object */
    static char  buffer[FILE_MAX_LINE_LENGTH_CNS]; /* string buffer */
    GT_U32 baseAddr;       /* base address for register */
    GT_U32 value;     /* default value */
    GT_32  number;    /* number of registers */
    GT_U32 addrDelta; /* delta for several registers */
    GT_U32 scanRes;   /* number scanned elements */
    GT_32    i;         /* iterator */
    GT_U32 address;   /* address of particular register */
    char *  remarkStr;
    GT_CHAR pciAddr[SMAIN_PCI_FLAG_BUFF_SIZE];
    GT_U32  currentLineIndex=0;/*current line number in the 'registers file'*/

    if(SMEM_SKIP_LOCAL_READ_WRITE)
    {
        /* don't write to memory , memory is not valid for it */
        return;
    }

    if (!fileNamePtr)
       return;

    /* open file */
    regFilePtr = fopen(fileNamePtr,"rt");
    if (regFilePtr == NULL)
    {
        skernelFatalError("smainMemConfigLoad: registers file not found %s\n", fileNamePtr);
    }

    while (fgets(buffer, FILE_MAX_LINE_LENGTH_CNS, regFilePtr))
    {
        currentLineIndex++;/* 1 based number */
        /* convert to lower case */
        strlwr(buffer);

        remarkStr = strstr(buffer,";");
        if (remarkStr == buffer)
        {
            continue;
        }

        /* record found, parse it */
        baseAddr = 0;
        value = 0;
        number = 0;
        addrDelta = 0; /* if scanRes < 4 => addrDelta is undefined */
        memset(pciAddr, 0, SMAIN_PCI_FLAG_BUFF_SIZE);
        scanRes = sscanf(buffer, "%x %x %d %x %10s", &baseAddr,
                        &value, &number, &addrDelta, pciAddr);

        if (baseAddr == SMAIN_FILE_RECORDS_EOF)
        {
            break;
        }

        if ((scanRes < 2) || (scanRes == 3) ||(scanRes > 5))
        {
            /* check end of file */
            if (scanRes == 1)
                break;

            simWarningPrintf("smainMemConfigLoad: registers file's bad format in line [%d]\n",currentLineIndex);
            break;
        }

        if(scanRes == 2)
        {
            number = 1;
        }
        /* set registers */
        for(i = 0; i < number; i++)
        {
            address = i * addrDelta + baseAddr;
            if(GT_TRUE == isSmemUsed)
            {
                smemMemSet(deviceObjPtr, address, &value, 1);
            }
            else
            {
                if ((strcmp(pciAddr,"dfx") == 0) || (strcmp(pciAddr,"DFX") == 0))
                {
                    /* NOTE: the next function is triggering the 'active memory'
                             mechanism , because the purpose is to configurate memory
                             exactly in accordance with config file content.
                    */
                    scibMemoryClientRegWrite(deviceObjPtr->deviceId,
                        SCIB_MEM_ACCESS_DFX_E,address,1,&value);
                }
                else
                if ((strcmp(pciAddr,"pci") == 0) || (strcmp(pciAddr,"PCI") == 0))
                {
                    /* NOTE: the next function is triggering the 'active memory'
                             mechanism , because the purpose is to configurate memory
                             exactly in accordance with config file content.
                    */
                    scibPciRegWrite(deviceObjPtr->deviceId,address,1,&value);
                }
                else
                {
                    /* NOTE: the next function is triggering the 'active memory'
                             mechanism , because the purpose is to configurate memory
                             exactly in accordance with config file content.
                    */
                    scibWriteMemory(deviceObjPtr->deviceId, address, 1,&value);
                }
            }
        }
    }
    fclose(regFilePtr);
}

/**
* @internal devLegacyRevInfoSet function
* @endinternal
*
* @brief   Set up legacy revision info.
*
* @param[in,out] devObjPtr                - pointer to the device object.
* @param[in,out] devObjPtr                - pointer to the device object.
*                                       None.
*/
static GT_VOID devLegacyRevInfoSet
(
    INOUT  SKERNEL_DEVICE_OBJECT * devObjPtr
)
{
    devObjPtr->legacyRevInfo.isCh1 = 0;
    devObjPtr->legacyRevInfo.isCh2AndAbove = 0;
    devObjPtr->legacyRevInfo.isCh3AndAbove = 0;
    devObjPtr->legacyRevInfo.isXcatAndAbove = 0;
    devObjPtr->legacyRevInfo.isLionAndAbove = 0;
    devObjPtr->legacyRevInfo.isXcat2AndAbove = 0;
    devObjPtr->legacyRevInfo.isLion2AndAbove = 0;

    switch(devObjPtr->deviceType)
    {
    case SKERNEL_98DX270  :
    case SKERNEL_98DX260  :
    case SKERNEL_98DX250  :
    case SKERNEL_98DX163  :
    case SKERNEL_98DX243  :
    case SKERNEL_98DX253  :
    case SKERNEL_98DX263  :
    case SKERNEL_98DX273  :
    case SKERNEL_98DX107  :
    case SKERNEL_98DX107B0:
    case SKERNEL_98DX133  :
    case SKERNEL_98DX803  :
    /*      case SKERNEL_98DX247  : --> same as SKERNEL_98DX253 */
    case SKERNEL_98DX249  :
    case SKERNEL_98DX269  :
    case SKERNEL_DXCH     :  /* generic DXCH  */
    case SKERNEL_DXCH_B0  :  /* generic DXCH_B0  8K FDB*/
        devObjPtr->legacyRevInfo.isCh1 = 1;
        break;
    case SKERNEL_98DX255:
    case SKERNEL_98DX265:
    case SKERNEL_98DX275:
    case SKERNEL_98DX285:
    case SKERNEL_98DX804:
    case SKERNEL_98DX125:
    case SKERNEL_98DX145:
    case SKERNEL_DXCH2:
        devObjPtr->legacyRevInfo.isCh1 = 1;
        devObjPtr->legacyRevInfo.isCh2AndAbove = 1;
        break;
    case SKERNEL_98DX806    :
    case SKERNEL_98DX5128   :
    case SKERNEL_98DX5128_1 :
    case SKERNEL_98DX5124   :
    case SKERNEL_98DX5126   :
    case SKERNEL_98DX5127   :
    case SKERNEL_98DX5129   :
    case SKERNEL_98DX5151   :
    case SKERNEL_98DX5152   :
    case SKERNEL_98DX5154   :
    case SKERNEL_98DX5155   :
    case SKERNEL_98DX5156   :
    case SKERNEL_98DX5157   :
    case SKERNEL_DXCH3      :
    case SKERNEL_98DX8110   :
    case SKERNEL_98DX8108   :
    case SKERNEL_98DX8109   :
    case SKERNEL_98DX8110_1 :
    case SKERNEL_DXCH3_XG   :
        devObjPtr->legacyRevInfo.isCh1 = 1;
        devObjPtr->legacyRevInfo.isCh2AndAbove = 1;
        devObjPtr->legacyRevInfo.isCh3AndAbove = 1;
        break;
    case SKERNEL_XCAT2_24_AND_4:
        devObjPtr->legacyRevInfo.isXcatAndAbove = 1;
        devObjPtr->legacyRevInfo.isXcat2AndAbove = 1;
        devObjPtr->legacyRevInfo.isLionAndAbove = 1;
        devObjPtr->legacyRevInfo.isCh1 = 1;
        devObjPtr->legacyRevInfo.isCh2AndAbove = 1;
        devObjPtr->legacyRevInfo.isCh3AndAbove = 1;
        break;
    case SKERNEL_XCAT3_24_AND_6:
    case SKERNEL_AC5_24_AND_6:
    case SKERNEL_98DX1101:
    case SKERNEL_98DX1111:
    case SKERNEL_98DX1122:
    case SKERNEL_98DX1123:
    case SKERNEL_98DX1142:

    case SKERNEL_98DX2101:
    case SKERNEL_98DX2112:
    case SKERNEL_98DX2122:
    case SKERNEL_98DX2123:
    case SKERNEL_98DX2142:
    case SKERNEL_98DX2151:
    case SKERNEL_98DX2161:

    case SKERNEL_98DX3001:
    case SKERNEL_98DX3010:
    case SKERNEL_98DX3011:
    case SKERNEL_98DX3020:
    case SKERNEL_98DX3021:
    case SKERNEL_98DX3022:

    case SKERNEL_98DX3101:
    case SKERNEL_98DX3110:
    case SKERNEL_98DX3111:
    case SKERNEL_98DX3120:
    case SKERNEL_98DX3121:
    case SKERNEL_98DX3122:
    case SKERNEL_98DX3123:
    case SKERNEL_98DX3124:
    case SKERNEL_98DX3125:
    case SKERNEL_98DX3141:
    case SKERNEL_98DX3142:

    case SKERNEL_98DX4101:
    case SKERNEL_98DX4102:
    case SKERNEL_98DX4103:
    case SKERNEL_98DX4110:
    case SKERNEL_98DX4120:
    case SKERNEL_98DX4121:
    case SKERNEL_98DX4122:
    case SKERNEL_98DX4123:
    case SKERNEL_98DX4140:
    case SKERNEL_98DX4141:
    case SKERNEL_98DX4142:
    case SKERNEL_XCAT_24_AND_4:
        devObjPtr->legacyRevInfo.isXcatAndAbove = 1;
        devObjPtr->legacyRevInfo.isCh1 = 1;
        devObjPtr->legacyRevInfo.isCh2AndAbove = 1;
        devObjPtr->legacyRevInfo.isCh3AndAbove = 1;
        break;
    case SKERNEL_BOBCAT2:
    case SKERNEL_BOBK_CAELUM:
    case SKERNEL_BOBK_CETUS:
    case SKERNEL_BOBK_ALDRIN:
    case SKERNEL_AC3X:
    case SKERNEL_BOBCAT3:
    case SKERNEL_ALDRIN2:
    case SMEM_CASE_FALCON_DEV_MAC:
    case SKERNEL_HAWK:
    case SKERNEL_PHOENIX:
    case SKERNEL_HARRIER:
    case SKERNEL_IRONMAN_S:
    case SKERNEL_IRONMAN_L:
    case SKERNEL_PIPE:/* let the PIPE device feel like sip5.20 device*/
    case SKERNEL_LION3_PORT_GROUP_12:
        devObjPtr->legacyRevInfo.isLionAndAbove = 1;
        devObjPtr->legacyRevInfo.isLion2AndAbove = 1;
        devObjPtr->legacyRevInfo.isXcat2AndAbove = 1;
        devObjPtr->legacyRevInfo.isXcatAndAbove = 1;
        devObjPtr->legacyRevInfo.isCh1 = 1;
        devObjPtr->legacyRevInfo.isCh2AndAbove = 1;
        devObjPtr->legacyRevInfo.isCh3AndAbove = 1;
        break;
    case SKERNEL_LION2_PORT_GROUP_12:
        devObjPtr->legacyRevInfo.isLionAndAbove = 1;
        devObjPtr->legacyRevInfo.isLion2AndAbove = 1;
        devObjPtr->legacyRevInfo.isXcat2AndAbove = 1;
        devObjPtr->legacyRevInfo.isXcatAndAbove = 1;
        devObjPtr->legacyRevInfo.isCh1 = 1;
        devObjPtr->legacyRevInfo.isCh2AndAbove = 1;
        devObjPtr->legacyRevInfo.isCh3AndAbove = 1;
        break;
    case SKERNEL_LION_PORT_GROUP_12:
        devObjPtr->legacyRevInfo.isLionAndAbove = 1;
        devObjPtr->legacyRevInfo.isXcatAndAbove = 1;
        devObjPtr->legacyRevInfo.isCh1 = 1;
        devObjPtr->legacyRevInfo.isCh2AndAbove = 1;
                devObjPtr->legacyRevInfo.isCh3AndAbove = 1;
    default:
        break;
    }

}

/**
* @internal smainDevInfoGet function
* @endinternal
*
* @brief   Get device info from string
*
* @param[in] devTypeName              - name of device type.
*/
static void smainDevInfoGet
(
    IN    char           * devTypeName,
    INOUT   SKERNEL_DEVICE_OBJECT * deviceObjPtr
)
{
    GT_U32 ii,jj;
    GT_STATUS   rc;

    /* go through the database */
    for (ii=0; ii < SMAIN_DEV_NAME_DB_SIZE; ii++)
    {
        if (strcmp(smainDevNameDb[ii].devName, devTypeName) == 0)
        {
            deviceObjPtr->portsNumber = smainDevNameDb[ii].portNum;
            deviceObjPtr->gmDeviceType = smainDevNameDb[ii].gmDevType ;

            deviceObjPtr->deviceType = smainDevNameDb[ii].devType;
            devLegacyRevInfoSet(deviceObjPtr);
            rc = deviceTypeInfoGet(deviceObjPtr);
            if(rc == GT_NOT_FOUND)
            {
                /* state all ports as exists */
                for(jj =0 ; jj < deviceObjPtr->portsNumber;jj++)
                {
                    deviceObjPtr->portsArr[jj].state = SKERNEL_PORT_STATE_GE_E;
                }

                for(/*continue*/ ; jj < SKERNEL_DEV_MAX_SUPPORTED_PORTS_CNS; jj++)
                {
                    deviceObjPtr->portsArr[jj].state = SKERNEL_PORT_STATE_NOT_EXISTS_E;
                }
            }

            if(smainDevNameDb[ii].cpuPortNum != NO_CPU_GMII_PORT_CNS)
            {
                if(deviceObjPtr->portsNumber < smainDevNameDb[ii].cpuPortNum)
                {
                    /* add another optional Slan for the CPU port */
                    deviceObjPtr->numSlans = smainDevNameDb[ii].cpuPortNum + 1;
                }
                else
                {
                    deviceObjPtr->numSlans = deviceObjPtr->portsNumber;
                }

                /* set CPU port */
                if(!IS_CHT_VALID_PORT(deviceObjPtr,(smainDevNameDb[ii].cpuPortNum)))
                {
                    deviceObjPtr->portsArr[smainDevNameDb[ii].cpuPortNum].state =
                        SKERNEL_PORT_STATE_GE_E;
                }

                deviceObjPtr->portsArr[smainDevNameDb[ii].cpuPortNum].supportMultiState =
                    GT_FALSE;
            }
            else
            {
                /* the device not support the CPU port with MII */
                deviceObjPtr->numSlans = deviceObjPtr->portsNumber;
            }

            return ;
        }
    }

    /* device not found - stop processing */
    skernelFatalError(" smainDevInfoGet: cannot find device %s", devTypeName);

    return ;
}


/**
* @internal skernelPortLinkStatusChange function
* @endinternal
*
* @brief   Simulates Link Up/Down event.
*
* @param[in] deviceNumber             - device number
* @param[in] portNumber               - port number
* @param[in] newStatus                - change link to Up or to Down
*
* @retval GT_OK                    - success, GT_FAIL otherwise
*/
GT_STATUS skernelPortLinkStatusChange
(
    IN  GT_U32   deviceNumber,
    IN  GT_U32   portNumber,
    IN  GT_BOOL  newStatus
)
{
    DEVICE_ID_CHECK_MAC(deviceNumber);

    snetLinkStateNotify(smemTestDeviceIdToDevPtrConvert(deviceNumber),
        portNumber, newStatus) ;

    return GT_OK ;
}

/**
* @internal smainDeviceBackUpMemory function
* @endinternal
*
* @brief   Definition of backup/restore memory function
*
* @param[in] deviceNumber             - device ID.
* @param[in] readWrite                - backup/restore memory data
*
* @retval GT_OK                    - success read/write
* @retval GT_FAIL                  - wrong device ID
*/
GT_STATUS smainDeviceBackUpMemory
(
    IN  GT_U8    deviceNumber,
    IN  GT_BOOL  readWrite
)
{
    SKERNEL_DEVICE_OBJECT * devObjPtr;

    DEVICE_ID_CHECK_MAC(deviceNumber);

    devObjPtr = smemTestDeviceIdToDevPtrConvert(deviceNumber);

    if (devObjPtr->devMemBackUpPtr)
    {
        devObjPtr->devMemBackUpPtr(devObjPtr, readWrite);
    }
    else
    {
        if(devObjPtr->shellDevice == GT_TRUE)
        {
            GT_U32                 dev;           /* core iterator */
            SKERNEL_DEVICE_OBJECT *currDevObjPtr; /* current device object pointer */
            for(dev = 0 ; dev < devObjPtr->numOfCoreDevs ; dev++)
            {
                currDevObjPtr = devObjPtr->coreDevInfoPtr[dev].devObjPtr;
                if(currDevObjPtr != NULL)
                {
                    /* process core */
                    currDevObjPtr->devMemBackUpPtr(currDevObjPtr, readWrite);
                }
            }
        }
    }

    return GT_OK ;
}

/**
* @internal smainDeviceBackUpMemoryTest function
* @endinternal
*
* @brief   Test backup memory function
*
* @param[in] deviceNumber             - device ID.
*
* @retval GT_OK                    - success
* @retval GT_FAIL                  - wrong device ID
*/
GT_STATUS smainDeviceBackUpMemoryTest
(
    IN  GT_U8    deviceNumber
)
{
    SKERNEL_DEVICE_OBJECT * devObjPtr;

    DEVICE_ID_CHECK_MAC(deviceNumber);
    devObjPtr = smemTestDeviceIdToDevPtrConvert(deviceNumber);


    /* for multi-core : write different value per core to the same register
       (address = 0x10) , to check such device */
    if(devObjPtr->shellDevice == GT_TRUE)
    {
        GT_U32                 temp  = 0x0AFAFAF0;
        GT_U32                 temp1 = 0;
        GT_U32                 dev;           /* core iterator */
        SKERNEL_DEVICE_OBJECT *currDevObjPtr; /* current device object pointer */
        for(dev = 0 ; dev < devObjPtr->numOfCoreDevs ; dev++)
        {
            currDevObjPtr = devObjPtr->coreDevInfoPtr[dev].devObjPtr;
            if(currDevObjPtr != NULL)
            {
                temp1 = temp + dev;
                smemMemSet(currDevObjPtr, 0x10, &temp1, 1);
            }
        }
    }

    /* do backup for the device */
    smainDeviceBackUpMemory(deviceNumber, 1);

    return GT_OK ;
}

/**
* @internal smainDeviceBackUpMemoryTestOneValue function
* @endinternal
*
* @brief   Test backup/restore memory function, one value
*
* @param[in] deviceNumber             - device ID.
*
* @retval GT_OK                    - success
* @retval GT_FAIL                  - wrong device ID
*/
GT_STATUS smainDeviceBackUpMemoryTestOneValue
(
    IN  GT_U8    deviceNumber
)
{
    SKERNEL_DEVICE_OBJECT * devObjPtr;

    GT_U32  var  = 0x00028100;
    GT_U32  *varGetPtr;
    GT_U32  address = 0x10;

    DEVICE_ID_CHECK_MAC(deviceNumber);
    devObjPtr = smemTestDeviceIdToDevPtrConvert(deviceNumber);


    varGetPtr = smemMemGet(devObjPtr, address);
    printf("TEST original value was: 0x%X \n", *varGetPtr);

#if 0
    /* do backup for the device */
    smainDeviceBackUpMemory(deviceNumber, 1);

#endif
    /* do restore for the device */
    smainDeviceBackUpMemory(deviceNumber, 0);

    /*set test var */
    /*smemMemSet(devObjPtr, address, &var, 1);*/

    varGetPtr = smemMemGet(devObjPtr, address);
    printf("TEST got value: 0x%X \n", *varGetPtr);

    /*read test var */
    if(*varGetPtr != var)
    {
        printf("TEST FAILED: got 0x%X instead of 0x%X\n", *varGetPtr, var);
        return GT_FAIL ;
    }

    return GT_OK ;
}


/**
* @internal smainDevice2SlanConnect function
* @endinternal
*
* @brief   Bind ports of devices with SLANs
*
* @param[in] deviceObjPtr             - pointer to the device object for task.
*                                      port_no      - number of port
* @param[in] linkState                - 1 for up , 0 for down.
*/
static void smainDevice2SlanConnect(
        IN SKERNEL_DEVICE_OBJECT * deviceObjPtr,
        IN GT_U32   portNum,
        IN GT_U32   linkState
)
{
    SMAIN_PORT_SLAN_INFO *slanInfoPrt;          /* slan port entry pointer */
    char                 updnprefix[10]={0};


    /* allow LOG to see link change interrupts */
    SIM_OS_MAC(simOsTaskOwnTaskPurposeSet)(SIM_OS_TASK_PURPOSE_TYPE_PP_PIPE_PROCESSING_DAEMON_E, NULL);
    simLogPacketFrameUnitSet(SIM_LOG_FRAME_UNIT___ALLOW_ALL_UNITS___E);/* wild card for LOG without the unit filter */
    simLogPacketFrameCommandSet(SIM_LOG_FRAME_COMMAND_TYPE_GENERAL_E);

    slanInfoPrt=&(deviceObjPtr->portSlanInfo[portNum]);

    if (slanInfoPrt->slanName[0x0] != 0x0)
    {
        memcpy(updnprefix,slanInfoPrt->slanName,2);
        if (memcmp(updnprefix,"ls",2) == 0)
        {
            SIM_OS_MAC(simOsChangeLinkStatus)(slanInfoPrt->slanIdRx , linkState);
        }
        else
        {
            skernelPortLinkStatusChange(slanInfoPrt->deviceObj->deviceId,
                                       slanInfoPrt->portNumber,
                                       linkState == 1 ? GT_TRUE : GT_FALSE);
        }
    }
    else /* support port with no SLAN */
    {
        skernelPortLinkStatusChange(deviceObjPtr->deviceId,
                                   portNum,
                                   linkState == 1 ? GT_TRUE : GT_FALSE);
    }
}

/**
* @internal resendMessage function
* @endinternal
*
* @brief   re-send message to the Skernel task
*
* @param[in] deviceObjPtr             - pointer to the device
* @param[in] bufferId                 - the buffer that hold original message.
*
* @note NOTE: the caller must bypass the 'sbufFree' for this buffer on current
*       treatment (not on the 'resend')
*
*/
static void resendMessage(
    IN SKERNEL_DEVICE_OBJECT * deviceObjPtr,
    IN SBUF_BUF_ID             bufferId
)
{
    /* re-send the buffer to the queue */
    squeBufPut(deviceObjPtr->queueId,SIM_CAST_BUFF(bufferId));
}

/**
* @internal smainSkernelTask function
* @endinternal
*
* @brief   Skernel task
*
* @param[in] deviceObjPtr             - pointer to the device object for task.
*/
static void smainSkernelTask(
        IN SKERNEL_DEVICE_OBJECT * deviceObjPtr
)
{
    SBUF_BUF_ID             bufferId;    /* buffer id */
    SBUF_POOL_ID            bufferPool;  /* buffer pool */
    GT_U8                   *dataPtr;    /* pointer to the start of buffer's data */
    GT_U32                  dataSize;    /* size of buffer's data */
    GT_U32                  portNum;     /*port id for the link change message*/
    GT_U32                  linkPortState;/* link state(1-up,0-down) for the
                                             link change message */
    GENERIC_MSG_FUNC genMsgFunc;/*generic msg function*/
    GT_U32           genMsgTypeSize = sizeof(GENERIC_MSG_FUNC);
    SKERNEL_DEVICE_OBJECT * newDeviceObjPtr;/* new device obj needed for soft reset */
    GT_BIT                  softResetMsg = 0;/*indication of soft reset message*/
    GT_BIT                  allowBuffFree;/* indication that the buffer can be free */
    GT_U32                  tileId; /* tileId for the operation */
    SIM_OS_TASK_PURPOSE_TYPE_ENT taskType;
    static char origDirName[1024];

    /* notify that task starts to work */
    SIM_OS_MAC(simOsSemSignal)(deviceObjPtr->smemInitPhaseSemaphore);

#ifdef _WIN32
    /* call SHOST to register the application task in the asic task table*/
    SHOSTG_psos_reg_asic_task();
#endif /*_WIN32*/

    /* set task type - only after SHOSTG_psos_reg_asic_task */
    taskType = SIM_OS_TASK_PURPOSE_TYPE_PP_PIPE_GENERAL_PURPOSE_E;
    SIM_OS_MAC(simOsTaskOwnTaskPurposeSet)(taskType,
        &deviceObjPtr->task_skernelCookieInfo.generic);

    while(1)
    {

        SET_TASK_TYPE_MAC(SIM_OS_TASK_PURPOSE_TYPE_PP_PIPE_GENERAL_PURPOSE_E);

        /* get buffer */
        bufferId = SIM_CAST_BUFF(squeBufGet(deviceObjPtr->queueId));

        allowBuffFree = 1;

        /* get buffer pool of the device object */
        bufferPool = deviceObjPtr->bufPool;

        /* One of the cores initiated soft reset, but not current core */
        if(deviceObjPtr->softResetNewDevicePtr)
        {
            /* Replace device during Soft Reset */
            if(bufferId->freeState == SBUF_BUFFER_STATE_ALLOCATOR_CAN_FREE_E)
            {
                sbufFree(bufferPool, bufferId);
            }

            /* Suspend buffer and queue again and flush them*/
            sbufPoolSuspend(bufferPool);
            squeSuspend(deviceObjPtr->queueId);
            squeFlush(deviceObjPtr->queueId);
            sbufPoolFlush(bufferPool);

            /* skernelSleep will change 'old' device to 'new' and wait for
            softResetOldDevicePtr == NULL  */
            deviceObjPtr = skernelSleep(deviceObjPtr, 1);

            /* No need to free alreadry freed buffer */
            allowBuffFree = 0;
        }

        /* set the pipeId for the device (got it from the sender of the message)
            default is '0' if was not set explicitly on the message */
        smemSetCurrentPipeId(deviceObjPtr,bufferId->pipeId);
        if(bufferId->mgUnitId)
        {
            /* NOTE: by default for 'unaware' messages the smemSetCurrentPipeId(...)
               already set default MG of the tile */
            /* set EXPLICIT mgUnitId for the device (got it EXPLICITly
               from the sender of the message) */

            smemSetCurrentMgUnitIndex(deviceObjPtr,bufferId->mgUnitId);
        }

        tileId = deviceObjPtr->numOfPipesPerTile ?
            bufferId->pipeId / deviceObjPtr->numOfPipesPerTile :
            0;

        /* we allocated the buffer , and we will free it unless told otherwise */
        bufferId->freeState = SBUF_BUFFER_STATE_ALLOCATOR_CAN_FREE_E;

        if(bufferId->dataType != SMAIN_MSG_TYPE_SOFT_RESET_E)
        {
            if(deviceObjPtr->needToDoSoftReset)
            {
                /* we started 'soft reset' and we need to ignore such messages
                  until device finish re-initialization */

                goto doneWithBuffer_lbl;
            }
        }

        /* process a buffer */
        if (bufferId->srcType == SMAIN_SRC_TYPE_SLAN_E ||
            bufferId->srcType == SMAIN_SRC_TYPE_LOOPBACK_PORT_E ||
            bufferId->srcType == SMAIN_SRC_TYPE_INTERNAL_CONNECTION_E ||
            bufferId->srcType == SMAIN_SRC_TYPE_OUTER_PORT_E)
        {
            /* process data from SLAN */
            if (bufferId->dataType == SMAIN_MSG_TYPE_FRAME_E)
            {
                SET_TASK_TYPE_MAC(SIM_OS_TASK_PURPOSE_TYPE_PP_PIPE_PROCESSING_DAEMON_E);

                /* the overSize need to be used for the actual byte count that the packet hold */
                /* and used as indication to drop the packet after MRU check and mib counting  */
                if(bufferId->overSize)
                {
                    bufferId->actualDataSize = bufferId->overSize;
                }

                /* process frame */
                snetFrameProcess(deviceObjPtr, bufferId, bufferId->srcData);

                skernelNumOfPacketsInTheSystemSet(GT_FALSE);
            }
        }
        else if (bufferId->srcType == SMAIN_SRC_TYPE_CPU_E)
        {
            switch(bufferId->dataType)
            {
                case SMAIN_MSG_TYPE_FDB_UPDATE_E:

                    /* buffer contents FDB update message */
                    sbufDataGet(bufferId,&dataPtr,&dataSize);

                    /* process message */
                    sfdbMsgProcess(deviceObjPtr, dataPtr);

                    break;
                case SMAIN_CPU_FDB_ACT_TRG_E:
                    SET_TASK_TYPE_MAC(SIM_OS_TASK_PURPOSE_TYPE_PP_AGING_DAEMON_E);

                    /* buffer contents FDB update message */
                    sbufDataGet(bufferId,&dataPtr,&dataSize);

                    if(bufferId->userInfo.extValue)
                    {
                        /* indication to reset the index */
                        deviceObjPtr->fdbAgingDaemonInfo[tileId].indexInFdb = 0;

                        bufferId->userInfo.extValue = 0;
                    }

                    /* process message */
                    sfdbMacTableTriggerAction(deviceObjPtr, dataPtr);

                    break;
                case SMAIN_CPU_TX_SDMA_QUEUE_E:
                    SET_TASK_TYPE_MAC(SIM_OS_TASK_PURPOSE_TYPE_PP_PIPE_PROCESSING_DAEMON_E);

                    /* process SDMA queue message */
                    snetFromCpuDmaProcess(deviceObjPtr, bufferId);

                    break;
                case SMAIN_CPU_FDB_AUTO_AGING_E:
                    SET_TASK_TYPE_MAC(SIM_OS_TASK_PURPOSE_TYPE_PP_AGING_DAEMON_E);
                    /* buffer contents FDB aging message */
                    sbufDataGet(bufferId,&dataPtr,&dataSize);

                    /* process message */
                    sfdbMacTableAutomaticAging(deviceObjPtr, dataPtr);

                    break;
                case SMAIN_LINK_CHG_MSG_E:
                    /* buffer content link changed message */
                    sbufDataGet(bufferId,&dataPtr,&dataSize);
                    portNum       = ((GT_U32*)dataPtr)[0];/* full word */
                    linkPortState = ((GT_U32*)dataPtr)[1];/* full word */
                    smainDevice2SlanConnect(deviceObjPtr,
                                            portNum ,
                                            linkPortState);
                    break;
                case SMAIN_INTERRUPTS_MASK_REG_E:
                    /* buffer contents interrupt mask register changed message */
                    sbufDataGet(bufferId,&dataPtr,&dataSize);
                    smainInterruptsMaskChanged(deviceObjPtr,
                       ((GT_U32*)dataPtr)[0],
                       ((GT_U32*)dataPtr)[1],
                       ((GT_U32*)dataPtr)[2],
                       ((GT_U32*)dataPtr)[3],
                       ((GT_U32*)dataPtr)[4],
                       ((GT_U32*)dataPtr)[5]);
                    break;
                case SMAIN_MSG_TYPE_FDB_UPLOAD_E:
                    SET_TASK_TYPE_MAC(SIM_OS_TASK_PURPOSE_TYPE_PP_AGING_DAEMON_E);

                    /* buffer contents FDB upload message */
                    sbufDataGet(bufferId,&dataPtr,&dataSize);

                    if(bufferId->userInfo.extValue)
                    {
                        /* indication to reset the index */
                        deviceObjPtr->fdbAgingDaemonInfo[tileId].indexInFdb = 0;

                        bufferId->userInfo.extValue = 0;
                    }

                    /* implementation of the upload action */
                    sfdbMacTableUploadAction(deviceObjPtr,dataPtr);

                    break;
                case SMAIN_MSG_TYPE_CNC_FAST_DUMP_E:
                    SET_TASK_TYPE_MAC(SIM_OS_TASK_PURPOSE_TYPE_PP_AGING_DAEMON_E);

                    /* buffer contents pointer to CNC Fast Dump Trigger Register */
                    sbufDataGet(bufferId,&dataPtr,&dataSize);

                    /* implementation of the upload action */
                    snetCncFastDumpUploadAction(deviceObjPtr, (GT_U32 *)dataPtr);

                    break;

                case SMAIN_MSG_TYPE_SOFT_RESET_E:
                    /* buffer contents pointer to Global Control Register */
                    sbufDataGet(bufferId, &dataPtr, &dataSize);

                    /* allow using __LOG .. to debug the 'soft reset' feature */
                    SET_TASK_TYPE_MAC(SIM_OS_TASK_PURPOSE_TYPE_PP_PIPE_SOFT_RESET_E);

                    if (GT_FALSE == sinit_global_usePexLogic ||
                        GET_CURRENT_DIR(origDirName, sizeof(origDirName)) == NULL)
                    {
                        origDirName[0] = 0;
                    }

                    if(simFSiniFileDirectory[0] && origDirName[0])
                    {
                        simForcePrintf("set working directory for 'Embedded FS' to:[%s]\n",simFSiniFileDirectory);
                        if(CHDIR(simFSiniFileDirectory) != 0)
                        {
                            simForcePrintf("failed to set directory for 'Embedded FS' to:[%s]\n",simFSiniFileDirectory);
                        }
                    }
                    /* implementation of device soft reset */
                    newDeviceObjPtr = skernelDeviceSoftReset(deviceObjPtr);
                    if(newDeviceObjPtr != NULL)
                    {
                        /* support replacement of the old device with new device */
                        deviceObjPtr = newDeviceObjPtr;
                    }

                    softResetMsg = 1;

                    if(simFSiniFileDirectory[0] && origDirName[0])
                    {
                        /* restore directory */
                        simForcePrintf("restore working directory to:[%s]\n",origDirName);
                        if(CHDIR(origDirName) != 0)
                        {
                            simForcePrintf("failed to restore working directory to:[%s]\n",origDirName);
                        }
                    }

                    break;
                case SMAIN_MSG_TYPE_GENERIC_FUNCTION_E:
                    /* buffer contents pointer to generic function */
                    sbufDataGet(bufferId,&dataPtr,&dataSize);
                    memcpy(&genMsgFunc,dataPtr,genMsgTypeSize);
                    dataPtr  += genMsgTypeSize;/* skip the function name */
                    dataSize -= genMsgTypeSize;
                    /* call the generic message callback function */
                    (*genMsgFunc)(deviceObjPtr,dataPtr,dataSize);
                    break;

                default:
                    break;
            }
        }

        if(deviceObjPtr->needResendMessage)
        {
            GT_U32  numBuffs;
            deviceObjPtr->needResendMessage = 0;
            resendMessage(deviceObjPtr,bufferId);
            numBuffs = sbufAllocatedBuffersNumGet(bufferPool);
            if(1 >= numBuffs)
            {
                /* let the skernel time without storming it self */
                deviceObjPtr =
                    skernelSleep(deviceObjPtr,100);
            }

            /* do not allow to free the buffer */
            allowBuffFree = 0;
        }

        doneWithBuffer_lbl:

        if(allowBuffFree &&
           bufferId->freeState == SBUF_BUFFER_STATE_ALLOCATOR_CAN_FREE_E)
        {
            /* free buffer */
            sbufFree(bufferPool, bufferId);
        }

        if(softResetMsg)
        {
            /* done only after the buffer that was used to hold the first message was released */
            skernelDeviceSoftResetPart2(deviceObjPtr);
            softResetMsg = 0;
            simulationSoftResetDone = 1;
        }

        if(deviceObjPtr->supportTaskMultiUnits)/*AC5*/
        {
            /* reset the task ext params */
            smemRestCurrentTaskExtParamValues(deviceObjPtr);
        }

    }
}

/* info for creating task */
typedef struct{
    SKERNEL_DEVICE_OBJECT * devObjPtr;
    GT_U32                  tileId;
}TASK_INFO_STC;

/**
* @internal smainSagingTask function
* @endinternal
*
* @brief   SAging task
*/
static void smainSagingTask(
    IN TASK_INFO_STC   *taskCookiePtr
)
{
    SIM_OS_TASK_PURPOSE_TYPE_ENT taskType;
    SKERNEL_DEVICE_OBJECT * deviceObjPtr = taskCookiePtr->devObjPtr;
    GT_U32  tileId = taskCookiePtr->tileId;

    /* notify that task starts to work */
    SIM_OS_MAC(simOsSemSignal)(deviceObjPtr->smemInitPhaseSemaphore);

#ifdef _WIN32
    /* call SHOST to register the application task in the asic task table*/
    SHOSTG_psos_reg_asic_task();
#endif /*_WIN32*/

    /* set task type - only after SHOSTG_psos_reg_asic_task */
    taskType = SIM_OS_TASK_PURPOSE_TYPE_PP_AGING_DAEMON_E;
    SIM_OS_MAC(simOsTaskOwnTaskPurposeSet)(taskType,
        &deviceObjPtr->task_sagingCookieInfo[tileId].generic);

    smemSetCurrentPipeId(deviceObjPtr,tileId * deviceObjPtr->numOfPipesPerTile);
    while(1)
    {
        sfdbMacTableAging(deviceObjPtr);
        /*Sleep(SMAIN_AGING_POLLING_TIME_CNS);*/

        deviceObjPtr = skernelSleep(deviceObjPtr,SMAIN_AGING_POLLING_TIME_CNS);

        /* set task type - only after SHOSTG_psos_reg_asic_task */
        SIM_OS_MAC(simOsTaskOwnTaskPurposeSet)(
            SIM_OS_TASK_PURPOSE_TYPE_PP_AGING_DAEMON_E,
            &deviceObjPtr->task_sagingCookieInfo[tileId].generic);
    }
}

/**
* @internal smainOamKeepAliveTask function
* @endinternal
*
* @brief   OAM keepalive task
*
* @param[in] deviceObjPtr             - pointer to the device object for task.
*/
static void smainOamKeepAliveTask(
    IN TASK_INFO_STC   *taskCookiePtr
)
{
    SIM_OS_TASK_PURPOSE_TYPE_ENT taskType;
    SKERNEL_DEVICE_OBJECT * deviceObjPtr = taskCookiePtr->devObjPtr;

    /* notify that task starts to work */
    SIM_OS_MAC(simOsSemSignal)(deviceObjPtr->smemInitPhaseSemaphore);

#ifdef _WIN32
    /* call SHOST to register the application task in the asic task table*/
    SHOSTG_psos_reg_asic_task();
#endif /*_WIN32*/

    /* set task type - only after SHOSTG_psos_reg_asic_task */
    taskType = SIM_OS_TASK_PURPOSE_TYPE_PP_PIPE_OAM_KEEP_ALIVE_DAEMON_E;
    SIM_OS_MAC(simOsTaskOwnTaskPurposeSet)(taskType,
        &deviceObjPtr->task_oamKeepAliveAgingCookieInfo.generic);

    snetLion2OamKeepAliveAging(deviceObjPtr);
}

/**
* @internal smainPacketGeneratorTask function
* @endinternal
*
* @brief   Packet generator main task
*
* @param[in] tgDataPtr                - pointer to the traffic generator data.
*/
GT_VOID smainPacketGeneratorTask
(
    IN SKERNEL_TRAFFIC_GEN_STC * tgDataPtr
)
{
    /* notify that task starts to work */
    SIM_OS_MAC(simOsSemSignal)(tgDataPtr->deviceObjPtr->smemInitPhaseSemaphore);

#ifdef _WIN32
    /* call SHOST to register the application task in the asic task table*/
    SHOSTG_psos_reg_asic_task();
#endif /*_WIN32*/

    snetLion2TgPacketGenerator(tgDataPtr);
}

/*
    note packets under SBUF_DATA_SIZE_CNS are in sbufAlloc
    slan packets that need larger size , will use MAX_BUFFER_SIZE for temp place
    only till copied to sbufAlloc of size SBUF_DATA_SIZE_CNS
*/
#define MAX_BUFFER_SIZE (0x10000)/*64K*/

static char overSizeSingleBufferForDrops[MAX_BUFFER_SIZE];
/*******************************************************************************
*   smainSlanPacketRcv
*
* DESCRIPTION:
*       receive packet  (called from slan scope)
*
* INPUTS:
*       msg_code   - message code from SLAN.
*       res        - reason for call.
*       sender_tid - task id of sender.
*       len        - message length.
*
* OUTPUTS:
*       usr_info_PTR - SLAN user info ptr.
*       buff_PTR - message.
* RETURNS:
*
* COMMENTS:
*
*******************************************************************************/
static char *smainSlanPacketRcv(
    IN  GT_U32       msg_code,
    IN  GT_U32       res,
    IN  GT_U32       sender_tid,
    IN  GT_U32       len,
    INOUT  void   *        usr_info_PTR,
    INOUT  char   *        buff_PTR
)
{
    SMAIN_PORT_SLAN_INFO *  slanInfoPrt; /* slan info entry pointer */
    SBUF_BUF_ID             bufferId;    /* buffer */
    GT_U8                *  data_PTR;  /* pointer to the data in the buffer */
    GT_U32                  data_size;  /* data size */
    GT_U32                  buffLen;
    char *                  returnPtr = NULL;
    GT_U32                  overSize;

    if(res == SIM_OS_SLAN_NORMAL_MSG_RSN_CNS && msg_code == 0)
    {
        /* no need to take semaphore */
        /* for some reason on win32 we get here on dummy cases */
        return NULL;
    }

    /* get slan info from user info */
    slanInfoPrt = (SMAIN_PORT_SLAN_INFO *)usr_info_PTR;

    /* wait for semaphore of 'delete slan' */
    /* since the SLAN context must not do 'SCIB lock' when slanInfoPrt->slanName[0] == 0
       because other thread is taking the SCIB and wait for this thread to end !!! */
    SIM_OS_MAC(simOsSemWait)(slanDownSemaphore,SIM_OS_WAIT_FOREVER);

    if(slanInfoPrt->slanName[0] == 0)
    {
#if 0        /* calling SCIB lock not allowed in this case ... so we need to avoid from
           calling OS functions like printf that can cause WIN32 to deadlock on
           SCIB taken by other thread */
        static GT_U32   dropsNum = 0;

        dropsNum++;
        /* the port was unbind from the SLAN */
        printf(" smainSlanPacketRcv:  port was unbound from the SLAN , do not treat this packet -- drop#%d \n",dropsNum);
#endif/*0*/

        returnPtr = NULL;
        goto exitCleanly_lbl;
    }

    if ( msg_code == SAGNTP_SLANUP_CNS )
    { /*LinkUp ....*/
        skernelPortLinkStatusChange(slanInfoPrt->deviceObj->deviceId,
                                    slanInfoPrt->portNumber,
                                    GT_TRUE);
        returnPtr = NULL;

        goto exitCleanly_lbl;
    }
    else if ( msg_code == SAGNTP_SLANDN_CNS )
    { /*LinkDown ....*/
        skernelPortLinkStatusChange(slanInfoPrt->deviceObj->deviceId,
                                    slanInfoPrt->portNumber,
                                    GT_FALSE);
        returnPtr = NULL;
        goto exitCleanly_lbl;
    }

    switch ( res )
    {
        /*! Sagent first call - get buffer and user info */
        case SIM_OS_SLAN_GET_BUFF_RSN_CNS :

            /* calc buff size needed for given frame length */
            buffLen = calcSlanBufSize(len, slanInfoPrt->deviceObj,1/*use_prependNumBytes */);

            if((buffLen - slanInfoPrt->deviceObj->prependNumBytes) >= MAX_BUFFER_SIZE)
            {
                simWarningPrintf(" smainSlanPacketRcv: packet too long [%ld] bytes >= MAX_BUFFER_SIZE[%ld]\n",
                    buffLen,MAX_BUFFER_SIZE);
                returnPtr = NULL;
                goto exitCleanly_lbl;
            }
            if(buffLen/* len + perpend */ >= SBUF_DATA_SIZE_CNS)
            {
                /* will return overSizeSingleBufferForDrops to the SLAN */
                overSize = buffLen - slanInfoPrt->deviceObj->prependNumBytes;

                buffLen = SBUF_DATA_SIZE_CNS;
            }
            else
            {
                overSize = 0;
            }

            /* get buffer */
            bufferId = sbufAlloc(slanInfoPrt->deviceObj->bufPool, buffLen);

            if ( bufferId == NULL )
            {
                simWarningPrintf(" smainSlanPacketRcv: no buffers for receive\n");
                returnPtr = NULL;

                goto exitCleanly_lbl;
            }

            /* get actual data pointer */
            sbufDataGet(bufferId, &data_PTR, &data_size);

            if(slanInfoPrt->deviceObj->prependNumBytes)
            {
                /* the buffer already has two additional bytes for future
                   prepend two bytes while send to CPU. The calcSlanBufSize
                   take these two bytes into account.
                   But these two bytes must not be used now. The data pointer
                   and data size need to be corrected in order to SLAN not use
                   them. */
                data_PTR += slanInfoPrt->deviceObj->prependNumBytes;
                data_size -= slanInfoPrt->deviceObj->prependNumBytes;

                /* store new data pointer and data size  */
                sbufDataSet(bufferId, data_PTR, data_size);
            }

            /* the overSize need to be used for the actual byte count that the packet hold */
            /* and used as indication to drop the packet after MRU check and mib counting  */
            bufferId->overSize = overSize;

            /* store buffer id in the slan info */
            slanInfoPrt->buffId = bufferId;

            if(overSize)
            {
                returnPtr = &overSizeSingleBufferForDrops[0];
            }
            else
            {
                /* notify the SLAN to start copy the frame after the prepend bytes */
                returnPtr = (char *)data_PTR;
            }

            break;

        /* Sagent second call -
            set buffer data params and put in queue for process*/
        case SIM_OS_SLAN_GIVE_BUFF_SUCCS_RSN_CNS :

            /* calc buff size needed for given frame length */
            buffLen = calcSlanBufSize(len, slanInfoPrt->deviceObj,1/*use_prependNumBytes */);

            if(buffLen/* len + perpend */ >= SBUF_DATA_SIZE_CNS)
            {
                GT_U32  maxCopy;

                /* the info is in overSizeSingleBufferForDrops ! and not in sbufAlloc(...) */
                buffLen = SBUF_DATA_SIZE_CNS;
                maxCopy = MAX(512,(buffLen-slanInfoPrt->deviceObj->prependNumBytes));

                /* NOTE: buff_PTR is &overSizeSingleBufferForDrops[0] */

                /* lets copy only 512 bytes of it to sbufAlloc(...) */
                /* as this packet must be dropped                   */
                memcpy(&slanInfoPrt->buffId->data[slanInfoPrt->deviceObj->prependNumBytes],
                    buff_PTR,maxCopy);

                /* 'fix' the pointer from : overSizeSingleBufferForDrops[...]
                    to be in slanInfoPrt->buffId->data[...] so function sbufDataSet(...)
                    will not fatal error */
                buff_PTR = (char*)&slanInfoPrt->buffId->data[slanInfoPrt->deviceObj->prependNumBytes];
            }

            /* buffer length has two bytes for prepend feature.
               actual data length should be reduced to compensate
               these two bytes */
            data_size = buffLen - slanInfoPrt->deviceObj->prependNumBytes;

            /* get the buffer and put it in the queue */
            sbufDataSet(slanInfoPrt->buffId, (GT_U8*)buff_PTR, data_size);

            skernelNumOfPacketsInTheSystemSet(GT_TRUE);

            /* set source type of buffer */
            slanInfoPrt->buffId->srcType = SMAIN_SRC_TYPE_SLAN_E;

            /* set source port of buffer */
            slanInfoPrt->buffId->srcData = slanInfoPrt->portNumber;

            /* set message type of buffer */
            slanInfoPrt->buffId->dataType = SMAIN_MSG_TYPE_FRAME_E;

            /* put buffer to queue */
            squeBufPut(slanInfoPrt->deviceObj->queueId,SIM_CAST_BUFF(slanInfoPrt->buffId));

            returnPtr = NULL;

            break;

        case SIM_OS_SLAN_GIVE_BUFF_ERR_RSN_CNS:

            /* error reason..  */
            simWarningPrintf(" smainSlanPacketRcv: Frame Discarded "\
                   " in device %d port %d , for slan =[%s] \n",
                    slanInfoPrt->deviceObj->deviceId,
                    slanInfoPrt->portNumber,
                    slanInfoPrt->slanName);

            /* free buffer */
            sbufFree(slanInfoPrt->deviceObj->bufPool, slanInfoPrt->buffId);

            returnPtr = NULL;

            break;
    }

exitCleanly_lbl:

    SIM_OS_MAC(simOsSemSignal)(slanDownSemaphore);

    return returnPtr;
}

/*******************************************************************************
*   smainSlanGMPacketRcv
*
* DESCRIPTION:
*       receive packet  (called from slan task scope)
*
* INPUTS:
*       msg_code   - message code from SLAN.
*       res        - reason for call.
*       sender_tid - task id of sender.
*       len        - message length.
*
* OUTPUTS:
*       usr_info_PTR - SLAN user info ptr.
*       buff_PTR     - message.
*
* RETURNS:
*
* COMMENTS:
*
*******************************************************************************/
static char *smainSlanGMPacketRcv(
    IN  GT_U32       msg_code,
    IN  GT_U32       res,
    IN  GT_U32       sender_tid,
    IN  GT_U32       len,
    INOUT  void   *        usr_info_PTR,
    INOUT  char   *        buff_PTR
)
{
    SMAIN_PORT_SLAN_INFO *  slanInfoPrt; /* slan info entry pointer */
    SBUF_BUF_ID             bufferId;    /* buffer */
    GT_U8                *  data_PTR;  /* pointer to the data in the buffer */
    GT_U32                  data_size;  /* data size */
    GT_U32                  buffLen;

    /* get slan info from user info */
    slanInfoPrt = (SMAIN_PORT_SLAN_INFO *)usr_info_PTR;

    switch ( res )
    {
        case SIM_OS_SLAN_NORMAL_MSG_RSN_CNS :
            return NULL;
        /*! Sagent first call - get buffer and user info */
        case SIM_OS_SLAN_GET_BUFF_RSN_CNS :

            /* calc buff size needed for given frame length */
            buffLen = calcSlanBufSize(len, slanInfoPrt->deviceObj,1/*use_prependNumBytes */);

            /* get buffer */
            bufferId = sbufAlloc(slanInfoPrt->deviceObj->bufPool, buffLen);

            if ( bufferId == NULL )
            {
                simWarningPrintf(" smainSlanGMPacketRcv: no buffers for receive\n");
                return NULL;
            }

            /* get actual data pointer */
            sbufDataGet(bufferId, &data_PTR, &data_size);

            if(slanInfoPrt->deviceObj->prependNumBytes)
            {
                /* the buffer already has two additional bytes for future
                   prepend two bytes while send to CPU. The calcSlanBufSize
                   take these two bytes into account.
                   But these two bytes must not be used now. The data pointer
                   and data size need to be corrected in order to SLAN not use
                   them. */
                data_PTR += slanInfoPrt->deviceObj->prependNumBytes;
                data_size -= slanInfoPrt->deviceObj->prependNumBytes;

                /* store new data pointer and data size  */
                sbufDataSet(bufferId, data_PTR, data_size);
            }

            /* store buffer id in the slan info */
            slanInfoPrt->buffId = bufferId;

            /* notify the SLAN to start copy the frame after the prepend bytes */
            return (char *)data_PTR;

        /* Sagent second call - set buffer data params and put in queue for process*/
        case SIM_OS_SLAN_GIVE_BUFF_SUCCS_RSN_CNS :

            /* calc buff size needed for given frame length */
            buffLen = calcSlanBufSize(len, slanInfoPrt->deviceObj,1/*use_prependNumBytes */);

            /* buffer length has two bytes for prepend feature.
               actual data length should be reduced to compensate
               these two bytes */
            data_size = buffLen - slanInfoPrt->deviceObj->prependNumBytes;

            /* get the buffer and put it in the queue */
            sbufDataSet(slanInfoPrt->buffId, (GT_U8*)buff_PTR, data_size);

            if(smainGmTrafficInitDone == 1)
            {
                if (slanInfoPrt->deviceObj->gmDeviceType != GOLDEN_MODEL)
                {
                    ppSendPacket(SMEM_GM_GET_GM_DEVICE_ID(slanInfoPrt->deviceObj),
                                 slanInfoPrt->deviceObj->portGroupId,
                            slanInfoPrt->portNumber,
                            buff_PTR, len);
                }
            }
            sbufFree(slanInfoPrt->deviceObj->bufPool, slanInfoPrt->buffId);
            return NULL;

        case SIM_OS_SLAN_GIVE_BUFF_ERR_RSN_CNS:

            /* error reason..  */
            simWarningPrintf(" smainSlanGMPacketRcv: Frame Discarded "\
                   " in device %d port %d , for slan =[%s] \n",
                    slanInfoPrt->deviceObj->deviceId,
                    slanInfoPrt->portNumber,
                    slanInfoPrt->slanName);

            /* free buffer */
            sbufFree(slanInfoPrt->deviceObj->bufPool, slanInfoPrt->buffId);
            return NULL;

    }

    return NULL;
}

/**
* @internal smainTrafficIngressPacket function
* @endinternal
*
* @brief   WM traffic function to ingress packet into a port in a device for processing.
*
* @param[in] deviceId                - the WM device ID. (as appears in the INI file)
* @param[in] portNum                 - the WM device ingress port number (MAC number).
* @param[in] numOfBytes              - the munber of bytes in the packet.
* @param[in] packetPtr               - (pointer to) array of bytes of the packet
*
*   NOTE:
*       0. if the port hold SLAN , this function should NOT be called (but the WM will process it !)
*       1. the caller is responsible to free the packet memory.
*       2. the caller to this function may free the memory of the packet as soon
*       as the function ends.
*       3. the function responsible to 'copy' the packet for it's own usage , to it's own memory.
*/
WM_STATUS smainTrafficIngressPacket(
    IN GT_U32   deviceId,
    IN GT_U32   portNum,
    IN GT_U32   numOfBytes,
    IN char*    packetPtr /* pointer to start of packet (network order) according to 'numOfBytes' */
)
{
    SKERNEL_DEVICE_OBJECT        *devObjPtr;
    SBUF_BUF_ID                 bufferId;
    GT_U32                      data_size,buffLen;
    GT_U8                       *data_PTR;
    GT_U32                      overSize;

    devObjPtr = smemTestDeviceIdToDevPtrConvert(deviceId);

    if(GT_FALSE == devObjPtr->portsArr[portNum].usedByOtherConnection)
    {
        __LOG_NO_LOCATION_META_DATA__WITH_SCIB_LOCK(("WARNING : packet from Ingress port [%d] DISCARDED as the port not bound (from ASIM Env)\n",
            portNum));
        /* do not let the packet to ingress as we consider the port as not-connected
           to this interface */
        return WM_ERROR;
    }

    /* calc buff size needed for given frame length */
    /* fix : IPBUSW-8538 : WM-ASIM: White Model adding 2 unexpected additional bytes*/
    buffLen = calcSlanBufSize(numOfBytes, devObjPtr,0/*use_prependNumBytes = 0*/);

    if(buffLen >= SBUF_DATA_SIZE_CNS)
    {
        /* fix for JIRA : IPBUSW-8942 :
        ASIM-CN10KAS: Transmitting 16k packet size to switch ports results in ASIM crash (In Dataplane ODP) */

        overSize = buffLen;

        buffLen = SBUF_DATA_SIZE_CNS;
    }
    else
    {
        overSize = 0;
    }

    /* get buffer */
    bufferId = sbufAlloc(devObjPtr->bufPool, buffLen);
    if ( bufferId == NULL )
    {
        /* fix for JIRA : IPBUSW-9019 :
            ASIM-CN10KAS: Packet drops seen with switch on high mpps workload like ODP

            the fix is to 'silent' drop the packets when no buffer.
            only the 'WM LOG file' will see those drops. and not sent as printf to the asim-terminal.
        */
        __LOG_NO_LOCATION_META_DATA((" smainTrafficIgressPacket: no buffers for receive packet from port[%d]\n",
            portNum));
        return WM_ERROR;
    }

    /* get actual data pointer */
    sbufDataGet(bufferId, &data_PTR, &data_size);
    /* MUST not skip pre-pend bytes */
    /* data_PTR += devObjPtr->prependNumBytes; */
    /* copy the packet */
    memcpy(data_PTR,packetPtr,numOfBytes);

    skernelNumOfPacketsInTheSystemSet(GT_TRUE);

    /* set source type of buffer */
    bufferId->srcType = SMAIN_SRC_TYPE_OUTER_PORT_E;

    /* set source port of buffer */
    bufferId->srcData = portNum;

    /* set message type of buffer */
    bufferId->dataType = SMAIN_MSG_TYPE_FRAME_E;

    /* the overSize need to be used for the actual byte count that the packet hold */
    /* and used as indication to drop the packet after MRU check and mib counting  */
    bufferId->overSize = overSize;

    /* put buffer to queue */
    squeBufPut(devObjPtr->queueId,SIM_CAST_BUFF(bufferId));

    return WM_OK;
}

GT_STATUS test_smainTrafficIngressPacket(
    IN GT_U32   deviceId,
    IN GT_U32   portNum,
    IN GT_U32   numOfBytes
)
{
    SKERNEL_DEVICE_OBJECT        *devObjPtr;
    GT_BOOL orig_usedByOtherConnection;

    devObjPtr = smemTestDeviceIdToDevPtrConvert(deviceId);

    orig_usedByOtherConnection = devObjPtr->portsArr[portNum].usedByOtherConnection;

    /* use dummy buffer overSizeSingleBufferForDrops */
    overSizeSingleBufferForDrops[0]  = 0;
    overSizeSingleBufferForDrops[5]  = portNum+1;/*'+1' to not have macDA all ZEROs */
    overSizeSingleBufferForDrops[6]  = 0;
    overSizeSingleBufferForDrops[7]  = portNum+1;/*'+1' to not have macSA all ZEROs */
    overSizeSingleBufferForDrops[8]  = portNum;
    overSizeSingleBufferForDrops[9]  = portNum;
    overSizeSingleBufferForDrops[10] = portNum;
    overSizeSingleBufferForDrops[11] = portNum;

    devObjPtr->portsArr[portNum].usedByOtherConnection = GT_TRUE;

    smainTrafficIngressPacket(deviceId,portNum,numOfBytes,overSizeSingleBufferForDrops);

    devObjPtr->portsArr[portNum].usedByOtherConnection = orig_usedByOtherConnection;

    return GT_OK;
}


/**
* @internal internalSlanBind function
* @endinternal
*
* @brief   do Bind to slan.
*
* @param[in] slanNamePtr              - (pointer to) slan name
* @param[in] deviceObjPtr             (pointer to) the device object
* @param[in] portNumber               - port number
* @param[in] bindRx                   - bind to Rx direction ? GT_TRUE - yes , GT_FALSE - no
* @param[in] bindTx                   - bind to Tx direction ? GT_TRUE - yes , GT_FALSE - no
* @param[in] explicitFuncPtr          - explicit SLAN receive function , in NULL ignored
*                                       None
*/
static void internalSlanBind (
    IN char                         *slanNamePtr,
    IN SKERNEL_DEVICE_OBJECT        *deviceObjPtr,
    IN GT_U32                       portNumber,
    IN GT_BOOL                      bindRx,
    IN GT_BOOL                      bindTx,
    IN SMAIN_PORT_SLAN_INFO         *slanInfoPrt,
    IN SIM_OS_SLAN_RCV_FUN          explicitFuncPtr
)
{
    SIM_OS_SLAN_RCV_FUN     funcPtr;
    SIM_OS_SLAN_ID          newSlanId;

    slanInfoPrt->portNumber = portNumber;
    strncpy(slanInfoPrt->slanName,slanNamePtr,SMAIN_SLAN_NAME_SIZE_CNS);

    if(explicitFuncPtr)
    {
        funcPtr = explicitFuncPtr;
    }
    else
    if (deviceObjPtr->gmDeviceType != SMAIN_NO_GM_DEVICE &&
       (deviceObjPtr->gmDeviceType != GOLDEN_MODEL))
    {
        funcPtr = smainSlanGMPacketRcv;
    }
    else
    {
        funcPtr = smainSlanPacketRcv;
    }

    newSlanId = SIM_OS_MAC(simOsSlanBind)( slanInfoPrt->slanName, deviceObjPtr->deviceName,
                            slanInfoPrt,funcPtr);

    if(bindRx == GT_TRUE)
    {
        slanInfoPrt->slanIdRx = newSlanId;
    }

    if(bindTx == GT_TRUE)
    {
        slanInfoPrt->slanIdTx = newSlanId;
    }

    if (newSlanId == NULL)
    {
        /*skernelFatalError("internalSlanBind: SLAN %s bind failed \n",slanInfoPrt->slanName );*/
        simWarningPrintf("internalSlanBind: SLAN %s bind failed \n",slanInfoPrt->slanName );

        /* allow the non existence of the 'SLAN connector' in the LINUX */
        return;
    }

    if(bindRx == GT_TRUE)
    {
        GT_U32  portForLink = snetChtExtendedPortMacGet(deviceObjPtr,portNumber,GT_TRUE);
        /* rx is link indication */
        deviceObjPtr->portsArr[portForLink].linkStateWhenNoForce = SKERNEL_PORT_NATIVE_LINK_UP_E;
    }

    /* send LOG indications */
    simLogSlanBind(slanInfoPrt->slanName,deviceObjPtr,portNumber,bindRx,bindTx);

    if(deviceObjPtr->devSlanBindPostFuncPtr)
    {
        /* allow the device to extra after the bind */
        deviceObjPtr->devSlanBindPostFuncPtr(deviceObjPtr,portNumber,bindRx,bindTx);
    }
}

/**
* @internal internalSlanUnbind function
* @endinternal
*
* @brief   do Unbind from slan : RX,TX.
*
* @param[in] slanInfoPrt              - (pointer) slan info
*                                       indication that unbind done
*/
static GT_BOOL internalSlanUnbind (
    IN SMAIN_PORT_SLAN_INFO         *slanInfoPrt
)
{
    GT_BOOL unbindDone , unbindRxDone = GT_FALSE , unbindTxDone = GT_FALSE;
    /* unbind */
    if (slanInfoPrt->slanIdRx == slanInfoPrt->slanIdTx)
    {
        /* slans are same - unbind only one of them */
        if (slanInfoPrt->slanIdRx != NULL)
        {
            SIM_OS_MAC(simOsSlanUnbind)(slanInfoPrt->slanIdRx);
            slanInfoPrt->slanIdRx = NULL;
            slanInfoPrt->slanIdTx = NULL;
            unbindRxDone = GT_TRUE;
            unbindTxDone = GT_TRUE;
        }
    }
    else
    {
        /* unbind rx slan */
        if (slanInfoPrt->slanIdRx != NULL)
        {
            SIM_OS_MAC(simOsSlanUnbind)(slanInfoPrt->slanIdRx);
            slanInfoPrt->slanIdRx = NULL;
            unbindRxDone = GT_TRUE;
        }

        /* unbind tx slan */
        if (slanInfoPrt->slanIdTx != NULL)
        {
            SIM_OS_MAC(simOsSlanUnbind)(slanInfoPrt->slanIdTx);
            slanInfoPrt->slanIdTx = NULL;
            unbindTxDone = GT_TRUE;
        }
    }

    if(unbindRxDone == GT_TRUE || unbindTxDone == GT_TRUE)
    {
        unbindDone = GT_TRUE;
        /* send LOG indications */
        simLogSlanBind(NULL,slanInfoPrt->deviceObj,slanInfoPrt->portNumber,unbindRxDone,unbindTxDone);
    }
    else
    {
        unbindDone = GT_FALSE;
    }


    return unbindDone;
}


/**
* @internal smainDevice2SlanBind function
* @endinternal
*
* @brief   Bind ports of devices with SLANs
*
* @param[in] deviceObjPtr             - pointer to the device object for task.
* @param[in] deviceId                 - device Id with which we search the INI file
* @param[in] startPortInIniFile       - the offset from the ports of the device to the
*                                      ports we search the INI file
*
* @note Bind ports for FA also.
*
*/
static void smainDevice2SlanBind(
        IN SKERNEL_DEVICE_OBJECT * deviceObjPtr,
        IN GT_U32                  deviceId,
        IN GT_U32                  startPortInIniFile
)
{
    GT_U32               port; /* port number */
    GT_U32               buffLen;
    char                 keyString[SMAIN_MIN_SIZE_OF_BUFFER_CNS]; /* key string */
    char                 slanName[FILE_MAX_LINE_LENGTH_CNS]; /* slan name */
    SMAIN_PORT_SLAN_INFO *slanInfoPrt;          /* slan port entry pointer */
    GT_BOOL              rxSlan, txSlan; /* SLAN type for port */
    char                 updnprefix[10];
    GT_BOOL              nonExistPort;/* indication that port not exists */

    strcpy(keyString,"dev");
    buffLen=3;

    for (port = 0; port < deviceObjPtr->numSlans; port++)
    {
        slanInfoPrt=&(deviceObjPtr->portSlanInfo[port]);
        slanInfoPrt->deviceObj = deviceObjPtr;
        slanInfoPrt->portNumber = port;

        if(port >= deviceObjPtr->portsNumber)
        {
            while(port < deviceObjPtr->numSlans &&
                  deviceObjPtr->portsArr[port].state == SKERNEL_PORT_STATE_NOT_EXISTS_E)
            {
                port++;
            }

            if(port == deviceObjPtr->numSlans)
            {
                break;
            }

            slanInfoPrt=&(deviceObjPtr->portSlanInfo[port]);
            slanInfoPrt->deviceObj = deviceObjPtr;
            slanInfoPrt->portNumber = port;

        }

        if(!IS_CHT_VALID_PORT(deviceObjPtr,port))
        {
            /* skip no exists ports in the range 0..deviceObjPtr->numSlans */
            nonExistPort = GT_TRUE;
        }
        else
        {
            nonExistPort = GT_FALSE;
        }

        if(SMEM_SKIP_LOCAL_READ_WRITE)
        {
            /* no valid use of those SLANS */
            slanInfoPrt->buffId = NULL;
            slanInfoPrt->slanIdRx = NULL;
            slanInfoPrt->slanIdTx = NULL;
            slanInfoPrt->slanName[0] = 0;
            continue;
        }

        rxSlan = txSlan = GT_FALSE;

        /* build the port name (with the device name) for Rx */
        sprintf(keyString+buffLen,"%d_port%drx",deviceId,port+startPortInIniFile);

        /* get SLAN name for port */
        if(SIM_OS_MAC(simOsGetCnfValue)("ports_map", keyString,
                        SMAIN_SLAN_NAME_SIZE_CNS, slanName))
        {
            INI_FILE_SLAN_FOR_NON_EXIST_PORT_CHECK_MAC(nonExistPort,port+startPortInIniFile,deviceId);

            rxSlan = GT_TRUE;

            /* bind port - RX direction with SLAN */
            internalSlanBind(slanName,deviceObjPtr,port,GT_TRUE,GT_FALSE,slanInfoPrt,NULL);

            /* build the port name (with the device name) for Tx */
            sprintf(keyString+buffLen,"%d_port%dtx",deviceId,port+startPortInIniFile);

            /* get SLAN name for port */
            if(SIM_OS_MAC(simOsGetCnfValue)("ports_map", keyString,
                            SMAIN_SLAN_NAME_SIZE_CNS, slanName))
            {
                INI_FILE_SLAN_FOR_NON_EXIST_PORT_CHECK_MAC(nonExistPort,port+startPortInIniFile,deviceId);

                txSlan = GT_TRUE;

                /* bind port - TX direction with SLAN */
                internalSlanBind(slanName,deviceObjPtr,port,GT_FALSE,GT_TRUE,slanInfoPrt,NULL);
            }
        }
        else
        {
            /* build the port name (with the device name) for Tx */
            sprintf(keyString+buffLen,"%d_port%dtx",deviceId,port+startPortInIniFile);

            /* get SLAN name for port */
            if(SIM_OS_MAC(simOsGetCnfValue)("ports_map", keyString,
                            SMAIN_SLAN_NAME_SIZE_CNS, slanName))
            {
                INI_FILE_SLAN_FOR_NON_EXIST_PORT_CHECK_MAC(nonExistPort,port+startPortInIniFile,deviceId);

                txSlan = GT_TRUE;

                /* bind port - TX direction with SLAN */
                internalSlanBind(slanName,deviceObjPtr,port,GT_FALSE,GT_TRUE,slanInfoPrt,NULL);
            }
        }

        if (txSlan == GT_FALSE && rxSlan == GT_FALSE)
        {
            /* build the port name (with the device name)*/
            sprintf(keyString+buffLen,"%d_port%d",deviceId,port+startPortInIniFile);

            /* get SLAN name for port */
            if(!SIM_OS_MAC(simOsGetCnfValue)("ports_map", keyString,
                            SMAIN_SLAN_NAME_SIZE_CNS, slanName))
            {
                /*set port as 'down'*/
                snetLinkStateNotify(deviceObjPtr, port, 0);
                continue;
            }

            INI_FILE_SLAN_FOR_NON_EXIST_PORT_CHECK_MAC(nonExistPort,port+startPortInIniFile,deviceId);

            /* bind port - RX,TX direction with SLAN */
            internalSlanBind(slanName,deviceObjPtr,port,GT_TRUE,GT_TRUE,slanInfoPrt,NULL);

            /* notify link UP Only if prefix is not "sl"                  */
            /* The "ls" prefix should be the same as SAGENT prefix has    */
            memcpy(updnprefix,slanName,2);
            if (memcmp(updnprefix,"ls",2) != 0)
            {
                snetLinkStateNotify(deviceObjPtr, port, 1);
            }
        }
    }
}

/*******************************************************************************
*   smainNicSlanPacketRcv
*
* DESCRIPTION:
*       receive packet  (called from slan scope)
*
* INPUTS:
*       msg_code - message code from SLAN.
*       res          - reason for call.
*       sender_tid - task id of sender.
*       len - message length.
*
* OUTPUTS:
*       usr_info_PTR - SLAN user info ptr.
*       buff_PTR - message.
* RETURNS:
*
* COMMENTS:
*
*
*******************************************************************************/
static char *smainNicSlanPacketRcv(
    IN  GT_U32       msg_code,
    IN  GT_U32       res,
    IN  GT_U32       sender_tid,
    IN  GT_U32       len,
    INOUT  void   *        usr_info_PTR,
    INOUT  char   *        buff_PTR
)
{
    SMAIN_PORT_SLAN_INFO *  slanInfoPrt; /* slan info entry pointer */
    GT_U8 *   segmentList;               /* Segment list */
    GT_U32    segmentLen;                /* Segment length */

    /* get slan info from user info */
    slanInfoPrt = (SMAIN_PORT_SLAN_INFO *)usr_info_PTR;

    /*if ( msg_code == SIM_OS_SLAN_NORMAL_MSG_RSN_CNS )
        return NULL;*/

    switch ( res )
    {
        case SIM_OS_SLAN_NORMAL_MSG_RSN_CNS :
            return NULL;

        /*! Sagent first call - get buffer and user info */
        case SIM_OS_SLAN_GET_BUFF_RSN_CNS :

            /* check data length */
            if (len > SBUF_DATA_SIZE_CNS)
                skernelFatalError("smainNicSlanPacketRcv: length %d too long",
                                 len);

            return (char *)slanInfoPrt->deviceObj->egressBuffer;

            /* Sagent second call - call callback */
        case SIM_OS_SLAN_GIVE_BUFF_SUCCS_RSN_CNS :

            /* call a callback function, add 4 bytes for CRC */
            segmentList = slanInfoPrt->deviceObj->egressBuffer;
            if (slanInfoPrt->deviceObj->crcPortsBytesAdd == 0)
            {
                segmentLen = len + 4;
            }
            else
            {
                segmentLen = len;
            }
            smainNicRxHandler(segmentList, segmentLen);

            return NULL;

        case SIM_OS_SLAN_GIVE_BUFF_ERR_RSN_CNS:

            /* error reason..  */
            simWarningPrintf(" smainNicSlanPacketRcv: Frame Discarded "\
                   " in device %d port %d , for slan =[%s] \n",
                    slanInfoPrt->deviceObj->deviceId,
                    slanInfoPrt->portNumber,
                    slanInfoPrt->slanName);

            return NULL;

    }

    return NULL;
}

/**
* @internal smainNicInit function
* @endinternal
*
* @brief   Init simulation of NIC
*
* @param[in] deviceObjPtr             - pointer to the device object for task.
*/
static void smainNicInit(
    IN SKERNEL_DEVICE_OBJECT * deviceObjPtr
)
{
    GT_U32               port; /* port number */
    char        keyString[SMAIN_MIN_SIZE_OF_BUFFER_CNS]; /* key string */
    char        slanName[FILE_MAX_LINE_LENGTH_CNS]; /* slan name */
    SMAIN_PORT_SLAN_INFO * slanInfoPrt;          /* slan port entry pointer */

    /* bind SLAN */
    for (port = 0; port < deviceObjPtr->numSlans; port++)
    {
        slanInfoPrt = &(deviceObjPtr->portSlanInfo[port]);
        slanInfoPrt->deviceObj = deviceObjPtr;
        slanInfoPrt->portNumber = port;

        if(SMEM_SKIP_LOCAL_READ_WRITE)
        {
            /* no valid use of those SLANS */
            slanInfoPrt->buffId = NULL;
            slanInfoPrt->slanIdRx = NULL;
            slanInfoPrt->slanIdTx = NULL;
            slanInfoPrt->slanName[0] = 0;
            continue;
        }

        /* build the port name (with the device name)*/
        sprintf(keyString,"dev%d_port%d",deviceObjPtr->deviceId,port);

        /* get SLAN name for port */
        if(!SIM_OS_MAC(simOsGetCnfValue)("ports_map", keyString,
                        SMAIN_SLAN_NAME_SIZE_CNS, slanName))
        {
            continue;
        }

        /* bind port - RX,TX direction with SLAN */
        internalSlanBind(slanName,deviceObjPtr,port,GT_TRUE,GT_TRUE,slanInfoPrt,
            smainNicSlanPacketRcv);
    }
}


/**
* @internal skernelInitGmDevParse function
* @endinternal
*
* @brief   Init for SKernel GM device
*
* @param[in] deviceObjPtr             - allocated pointer for the device
* @param[in] devInfoSection           - the device info section in the INI file
* @param[in] devId                    - the deviceId in the INI file info
*/
static void skernelInitGmDevParse
(
    IN SKERNEL_DEVICE_OBJECT * deviceObjPtr,
    IN char*                   devInfoSection,
    IN GT_U32                  devId
)
{
    GT_TASK_PRIORITY_ENT    taskPriority;
    GT_TASK_HANDLE          taskHandl;          /* task handle */

    /* init memory module */
    smemGmInit(deviceObjPtr);
    /* reg file(s) upload */
    smainMemDefaultsLoadAll(deviceObjPtr, devId, devInfoSection);
    /* memory init part 2 */
    smemGmInit2(deviceObjPtr);

    /* network init */
    snetProcessInit(deviceObjPtr);
    taskPriority = GT_TASK_PRIORITY_ABOVE_NORMAL;

    /* Create SKernel task */
    deviceObjPtr->uplink.partnerDeviceID = deviceObjPtr->deviceId;
    taskHandl = SIM_OS_MAC(simOsTaskCreate)(
                    taskPriority,
                    (unsigned (__TASKCONV *)(void*))smainSkernelTask,
                    (void *) deviceObjPtr);
    if (taskHandl == NULL)
    {
        skernelFatalError(" skernelInit: cannot create main task for"\
                           " device %u",devId);
    }

    /* wait for semaphore */
    SIM_OS_MAC(simOsSemWait)(deviceObjPtr->smemInitPhaseSemaphore,SIM_OS_WAIT_FOREVER);
    deviceObjPtr->numThreadsOnMe++;
}

/*******************************************************************************
*   skernelInitDevParsePart1
*
* DESCRIPTION:
*       Init for SKernel device (part1)
*
* INPUTS:
*       deviceObjPtr - allocated pointer for the device
*       devInfoSection - the device info section in the INI file
*       devId - the deviceId in the INI file info
* OUTPUTS:
*       None.
*
* RETURNS:
*
* COMMENTS:
*
*
*******************************************************************************/
static void skernelInitDevParsePart1
(
    IN SKERNEL_DEVICE_OBJECT * deviceObjPtr,
    IN char*                   devInfoSection,
    IN GT_U32                  devId
)
{
    char                    param_str[FILE_MAX_LINE_LENGTH_CNS];
                            /* string for parameter */
    char                    keyStr[SMAIN_MIN_SIZE_OF_BUFFER_CNS]; /* key string*/
    GT_TASK_PRIORITY_ENT    taskPriority;
    GT_TASK_HANDLE          taskHandl;        /* task handle */
    GT_BOOL                 useSkernelTask = GT_FALSE;/*do we need skernel task*/
    GT_BOOL                 useSagingTask = GT_FALSE;/*do we need saging task*/
    GT_U32                  txQue;
    GT_U32  tileId;
    GT_U32  numOfTiles;

    if(deviceObjPtr->shellDevice == GT_TRUE ||
       SKERNEL_DEVICE_FAMILY_EMBEDDED_CPU(deviceObjPtr->deviceType) ||
       SKERNEL_FABRIC_ADAPTER_DEVICE_FAMILY(deviceObjPtr->deviceType)
    )
    {
        useSkernelTask = GT_FALSE;
        useSagingTask = GT_FALSE;
    }
    else if((deviceObjPtr->gmDeviceType != SMAIN_NO_GM_DEVICE) || /* GM devices */
            skernelIsPhyOnlyDev(deviceObjPtr) ||
            skernelIsMacSecOnlyDev(deviceObjPtr))
    {
        useSkernelTask = GT_TRUE;
        useSagingTask = GT_FALSE;
    }
    else
    if(SMEM_IS_PIPE_FAMILY_GET(deviceObjPtr))
    {
        useSkernelTask = GT_TRUE;
        /* the device hold no AGING daemon */
        useSagingTask = GT_FALSE;
    }
    else
    {
        /* generic switches */
        useSkernelTask = GT_TRUE;
        useSagingTask = GT_TRUE;

        if(skernelIsLion3PortGroupOnlyDev(deviceObjPtr) &&
           deviceObjPtr->portGroupId != LION3_UNIT_FDB_TABLE_SINGLE_INSTANCE_PORT_GROUP_CNS)
        {
            /* SIP5 devices hold single FDB instance , so single aging daemon needed */
            useSagingTask = GT_FALSE;
        }
    }

    /*
       NOTE:
       1. the simulation set device by default with proper value , without
       the need to add this line into the INI file unless for next special cases :
       2. this setting must be done prior to calling smemInit(...)

       allow a device to state that it is not support the PCI configuration
       memory even though the device have PCI configuration memory !

       like DX246/DX107 that used with the SMI interface although it is like DX250
       that uses the PCI interface
    */
    sprintf(keyStr, "dev%u_not_support_pci_config_memory", devId);
    if (SIM_OS_MAC(simOsGetCnfValue)(devInfoSection, keyStr,
                   FILE_MAX_LINE_LENGTH_CNS, param_str))
    {
        sscanf(param_str, "%u", &deviceObjPtr->notSupportPciConfigMemory);
    }

    /* get number of port groups */
    sprintf(keyStr, "dev%u_port_groups_num", devId);
    if (SIM_OS_MAC(simOsGetCnfValue)(devInfoSection, keyStr,
                   FILE_MAX_LINE_LENGTH_CNS, param_str))
    {
        sscanf(param_str, "%u", &deviceObjPtr->numOfCoreDevs);
    }

    /* get number of 'cores' == 'port groups' */
    sprintf(keyStr, "dev%u_cores_num", devId);
    if (SIM_OS_MAC(simOsGetCnfValue)(devInfoSection, keyStr,
                   FILE_MAX_LINE_LENGTH_CNS, param_str))
    {
        sscanf(param_str, "%u", &deviceObjPtr->numOfCoreDevs);
        deviceObjPtr->useCoreTerminology = GT_TRUE;
    }

    if(deviceObjPtr->numOfCoreDevs)
    {
        /* allocate the port group devices */
        deviceObjPtr->coreDevInfoPtr =
            smemDeviceObjMemoryAlloc(deviceObjPtr,deviceObjPtr->numOfCoreDevs, sizeof(SKERNEL_CORE_DEVICE_INFO_STC));
        if (deviceObjPtr->coreDevInfoPtr == NULL)
        {
            skernelFatalError(" skernelInitDevParsePart1: cannot allocate port group devices in device %u", devId);
        }
    }

    simForcePrintf("Start to allocate device memories \n");
    /* memory init */
    smemInit(deviceObjPtr);

    /* network init */
    snetProcessInit(deviceObjPtr);

    simForcePrintf("Start load default values for device memories \n");
    smainMemDefaultsLoadAll(deviceObjPtr, devId, devInfoSection);

    /* memory init part 2 */
    smemInit2(deviceObjPtr);

    simForcePrintf("Done with device memories \n");

    if (!SKERNEL_FABRIC_ADAPTER_DEVICE_FAMILY(deviceObjPtr->deviceType))
    {
        deviceObjPtr->uplink.partnerDeviceID = deviceObjPtr->deviceId;
    }

    taskPriority = GT_TASK_PRIORITY_ABOVE_NORMAL;

    numOfTiles = deviceObjPtr->numOfTiles ? deviceObjPtr->numOfTiles : 1;

    if(deviceObjPtr->softResetOldDevicePtr)
    {
        /* do not create any more tasks , but use existing threads */
        GT_U32  numOfMgUnits = deviceObjPtr->numOfMgUnits ? deviceObjPtr->numOfMgUnits : 1;
        GT_U32  mgUnit;
        for(mgUnit = 0; mgUnit < numOfMgUnits; mgUnit++)
        {
            for(txQue = 0; txQue < 8 ; txQue++)
            {
                SIM_TAKE_PARAM_FROM_OLD_DEVICE_MAC(deviceObjPtr,sdmaTransmitInfo[mgUnit].sdmaTransmitData[txQue].taskHandle);
                if(deviceObjPtr->sdmaTransmitInfo[mgUnit].sdmaTransmitData[txQue].taskHandle)
                {
                    deviceObjPtr->sdmaTransmitInfo[mgUnit].sdmaTransmitData[txQue].txQueue = txQue;
                    deviceObjPtr->sdmaTransmitInfo[mgUnit].sdmaTransmitData[txQue].devObjPtr = deviceObjPtr;
                    deviceObjPtr->numThreadsOnMe++;
                }
            }
        }

        if(useSkernelTask == GT_TRUE)
        {
            deviceObjPtr->numThreadsOnMe++;
        }

        if(useSagingTask == GT_TRUE)
        {
            for(tileId = 0; tileId < numOfTiles ; tileId++)
            {
                deviceObjPtr->numThreadsOnMe++;
            }
        }

        if(deviceObjPtr->oamSupport.keepAliveSupport)
        {
            deviceObjPtr->numThreadsOnMe++;
        }

    }
    else
    {
        TASK_INFO_STC   taskCookie;

        if(useSkernelTask == GT_TRUE)
        {
           taskHandl = SIM_OS_MAC(simOsTaskCreate)(
                            taskPriority,
                            (unsigned (__TASKCONV *)(void*))smainSkernelTask,
                            (void *) deviceObjPtr);
            if (taskHandl == NULL)
            {
                skernelFatalError(" skernelInit: cannot create main task for"\
                                   " device %u",devId);
            }
            /* wait for semaphore */
            SIM_OS_MAC(simOsSemWait)(deviceObjPtr->smemInitPhaseSemaphore,SIM_OS_WAIT_FOREVER);
            deviceObjPtr->numThreadsOnMe++;
        }

        if(useSagingTask == GT_TRUE)
        {
            for(tileId = 0; tileId < numOfTiles ; tileId++)
            {
                taskCookie.devObjPtr = deviceObjPtr;
                taskCookie.tileId = tileId;

                taskHandl = SIM_OS_MAC(simOsTaskCreate)(taskPriority,
                                  (unsigned (__TASKCONV *)(void*))smainSagingTask,
                                  (void *) &taskCookie);
                if (taskHandl == NULL)
                {
                    skernelFatalError(" skernelInit: cannot create aging "\
                                      " task for device [%d] tileId[%d]", devId , tileId);
                }
                /* wait for semaphore */
                SIM_OS_MAC(simOsSemWait)(deviceObjPtr->smemInitPhaseSemaphore,SIM_OS_WAIT_FOREVER);
                deviceObjPtr->numThreadsOnMe++;
            }
        }


        if(SMEM_CHT_IS_SIP6_GET(deviceObjPtr) &&
           SMAIN_NOT_VALID_CNS == UNIT_BASE_ADDR_GET_ALLOW_NON_EXIST_UNIT_MAC(deviceObjPtr,UNIT_IOAM))
        {
            /* keep alive is implemented by the IOAM unit */
            deviceObjPtr->oamSupport.keepAliveSupport = 0;
        }

        if(deviceObjPtr->oamSupport.keepAliveSupport)
        {
            taskCookie.devObjPtr = deviceObjPtr;
            taskCookie.tileId = 0;/*unused parameter (ignored)*/

            taskHandl = SIM_OS_MAC(simOsTaskCreate)(taskPriority,
                              (unsigned (__TASKCONV *)(void*))smainOamKeepAliveTask,
                              (void *) &taskCookie);
            if (taskHandl == NULL)
            {
                skernelFatalError(" skernelInit: cannot create keep alive aging "\
                                  " task for device [%d] \n", devId);
            }
            /* wait for semaphore */
            SIM_OS_MAC(simOsSemWait)(deviceObjPtr->smemInitPhaseSemaphore, SIM_OS_WAIT_FOREVER);
            deviceObjPtr->numThreadsOnMe++;
        }

        if(SMEM_CHT_IS_SIP6_30_GET(deviceObjPtr) &&
           deviceObjPtr->limitedResources.preqSrfNum)
        {
           taskHandl = SIM_OS_MAC(simOsTaskCreate)(
                            taskPriority,
                            (unsigned (__TASKCONV *)(void*))snet6_30PreqSrfRestartDaemonTask,
                            (void *) deviceObjPtr);
            if (taskHandl == NULL)
            {
                skernelFatalError(" skernelInit: cannot create Preq Srf Restart Daemon task for"\
                                   " device %u",devId);
            }
            /* wait for semaphore */
            SIM_OS_MAC(simOsSemWait)(deviceObjPtr->smemInitPhaseSemaphore,SIM_OS_WAIT_FOREVER);
            deviceObjPtr->numThreadsOnMe++;
        }
    }
}

/**
* @internal smainBusDevFind function
* @endinternal
*
* @brief   find device on the bus.
*
* @param[in] busId                    - the device that we look the device on
* @param[in] targetHwDeviceId         - the target HW device ID of the device that we look for.
*
* @param[out] targetDeviceIdPtr        - (pointer to) the target device number.
*                                      with this number we can access the SCIB API to access
*                                      this device.
*
* @retval GT_TRUE                  - device found
* @retval GT_FALSE                 - device not found
*/
GT_BOOL smainBusDevFind
(
    IN GT_U32   busId,
    IN GT_U32   targetHwDeviceId,
    OUT GT_U32  *targetDeviceIdPtr
)
{
    SKERNEL_DEVICE_OBJECT * deviceObjPtr;/* device object */
    SKERNEL_DEVICE_OBJECT * coreDeviceObjPtr;/* core device object */
    GT_U32  ii,jj,kk;

    for(ii = 0 ; ii < numberOfBuses; ii++)
    {
        if(busInfoArr[ii].id == busId)
        {
            /* the bus found */
            break;
        }
    }

    if(ii == numberOfBuses)
    {
        /* bus not found */
        return GT_FALSE;
    }

    for(jj = 0 ; jj < SMAIN_MAX_NUM_OF_DEVICES_CNS ; jj++)
    {
        deviceObjPtr = smainDeviceObjects[jj];

        if(0 == (busInfoArr[ii].membersBmp[jj/32] & (1 << jj%32)))
        {
            continue;
        }

        if(deviceObjPtr->shellDevice == GT_TRUE)
        {
            /* the shell device register once , so all it's cores not need to */

            /* so we need to find the relevant core */
            for(kk = 0 ; kk < deviceObjPtr->numOfCoreDevs ; kk++ )
            {
                coreDeviceObjPtr = deviceObjPtr->coreDevInfoPtr[kk].devObjPtr;

                if(coreDeviceObjPtr->deviceHwId == targetHwDeviceId)
                {
                    *targetDeviceIdPtr = coreDeviceObjPtr->deviceId;
                    return GT_TRUE;
                }
            }

            /* no core of this device match the targetHwDeviceId */

            /* the shell device don't have 'HW device id' */
            continue;
        }

        if(deviceObjPtr->deviceHwId != targetHwDeviceId)
        {
            continue;
        }

        *targetDeviceIdPtr = deviceObjPtr->deviceId;
        return GT_TRUE;
    }

    return GT_FALSE;
}

/**
* @internal smainBusSet function
* @endinternal
*
* @brief   register the device on the bus
*
* @param[in] deviceObjPtr             - allocated pointer for the device
*/
static void smainBusSet
(
    IN SKERNEL_DEVICE_OBJECT * deviceObjPtr
)
{
    GT_U32  ii;

    for(ii = 0 ; ii < numberOfBuses; ii++)
    {
        if(busInfoArr[ii].id == deviceObjPtr->busId)
        {
            /* the bus found */
            break;
        }
    }

    if(ii == numberOfBuses)
    {
        /* bus not found */
        numberOfBuses++;

        if(numberOfBuses >= MAX_BUS_NUM_CNS)
        {
            skernelFatalError(" skernelInit: not supporting [%d] buses",
                                                    numberOfBuses+1);
        }
    }

    /* register the device on this bus */
    busInfoArr[ii].id = deviceObjPtr->busId;
    /* update the devices on this bus */
    busInfoArr[ii].membersBmp[deviceObjPtr->deviceId / 32] |= 1 << (deviceObjPtr->deviceId % 32);
}

/**
* @internal interruptMppTriggered function
* @endinternal
*
* @brief   The MPP (multi-purpose pin) of my device was triggered/cleared by another
*         device.
* @param[in] devObjPtr                - pointer to device object.
* @param[in] dataPtr                  - pointer to 'MPP' info.
* @param[in] dataLength               - length of the data
*/
static GT_VOID interruptMppTriggered
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U8                 * dataPtr,
    IN GT_U32                  dataLength
)
{
    GT_U32  mppId;
    GT_U32  interruptValue;

    SIM_OS_MAC(simOsTaskOwnTaskPurposeSet)(
        SIM_OS_TASK_PURPOSE_TYPE_PP_PIPE_PROCESSING_DAEMON_E,NULL);

    /* parse the data from the buffer of the message */
    /* get parameter 1 */
    memcpy(&mppId,dataPtr,sizeof(mppId));
    dataPtr+=sizeof(mppId);

    /* get parameter 2 */
    memcpy(&interruptValue,dataPtr,sizeof(interruptValue));
    dataPtr+=sizeof(interruptValue);

    if(devObjPtr->devMemInterruptMppFuncPtr)
    {
        devObjPtr->devMemInterruptMppFuncPtr(devObjPtr,mppId,interruptValue);
    }
}

/**
* @internal smainForScibBindInterruptMppTrigger function
* @endinternal
*
* @brief   Interrupt of 'my' device need to trigger the MPP (multi-purpose pin) of
*         another device
* @param[in] deviceId                 - ID of device, which is equal to PSS Core API device ID.
*
* @note because the fa and the pp has the same interrupt link i chosen to use
*       only the interrupt line of pp .
*
*/
static void smainForScibBindInterruptMppTrigger
(
    IN  GT_U32        deviceId
)
{
    DECLARE_FUNC_NAME(smainForScibBindInterruptMppTrigger);

    SKERNEL_DEVICE_OBJECT * devObjPtr;
    SKERNEL_DEVICE_OBJECT * other_devObjPtr;
    SBUF_BUF_ID             bufferId;    /* buffer id */
    GT_U32                  messageSize;   /*message size*/
    GT_U8  * dataPtr;         /* pointer to the data in the buffer */
    GT_U32 dataSize;          /* data size */
    GENERIC_MSG_FUNC genFunc = interruptMppTriggered;/* generic function */
    GT_U32  mppId;
    GT_U32  interruptValue = 1;

    devObjPtr = smemTestDeviceIdToDevPtrConvert(deviceId);

    if(devObjPtr->interruptLineToOtherDevice.isInterruptLineToOtherDevice == 0)
    {
        __LOG(("device [%d] need to trigger MPP (multi-purpose pin) but without 'bind' info \n"));
        return;
    }

    mppId = devObjPtr->interruptLineToOtherDevice.otherDev_mppId;

    messageSize = sizeof(GENERIC_MSG_FUNC) + sizeof(mppId) + sizeof(interruptValue);

    other_devObjPtr = smemTestDeviceIdToDevPtrConvert(
        devObjPtr->interruptLineToOtherDevice.otherDev_deviceId);

    /* allocate buffer from the 'destination' device pool */
    /* get the buffer and put it in the queue */
    bufferId = sbufAlloc(other_devObjPtr->bufPool, messageSize);
    if (bufferId == NULL)
    {
        simWarningPrintf(" smainForScibBindInterruptMppTrigger : no buffers for process \n");
        return ;
    }

    /* Get actual data pointer */
    sbufDataGet(bufferId, (GT_U8 **)&dataPtr, &dataSize);

    /* put the name of the function into the message */
    memcpy(dataPtr,&genFunc,sizeof(GENERIC_MSG_FUNC));
    dataPtr+=sizeof(GENERIC_MSG_FUNC);

    /* save parameter 1 */
    memcpy(dataPtr,&mppId,sizeof(mppId));
    dataPtr+=sizeof(mppId);

    /* save parameter 1 */
    memcpy(dataPtr,&interruptValue,sizeof(interruptValue));
    dataPtr+=sizeof(interruptValue);

    /* set source type of buffer                    */
    bufferId->srcType = SMAIN_SRC_TYPE_CPU_E;

    /* set message type of buffer                   */
    bufferId->dataType = SMAIN_MSG_TYPE_GENERIC_FUNCTION_E;

    /* put buffer to queue                          */
    squeBufPut(other_devObjPtr->queueId, SIM_CAST_BUFF(bufferId));



}




/**
* @internal skernelInitDevParse function
* @endinternal
*
* @brief   Init for SKernel device
*
* @param[in] deviceObjPtr             - allocated pointer for the device
* @param[in] devInfoSection           - the device info section in the INI file
* @param[in] devId                    - the deviceId in the INI file info
*/
static void skernelInitDevParse
(
    IN SKERNEL_DEVICE_OBJECT * deviceObjPtr,
    IN char*                   devInfoSection,
    IN GT_U32                  devId
)
{
    char                    param_str[FILE_MAX_LINE_LENGTH_CNS];
                            /* string for parameter */
    char                    keyStr[SMAIN_MIN_SIZE_OF_BUFFER_CNS]; /* key string*/
    GT_U32                  intLine;         /* interrupt line number */
    char                    currDevStr[SMAIN_MIN_SIZE_OF_BUFFER_CNS]={0};
    char                    currRightSideDevStr[FILE_MAX_LINE_LENGTH_CNS]={0};
    GT_U32                  devHwId;      /* device Hw Id */
    GT_U32                  tmpRightSizeDev;
    GT_U32                  portGroupDevId=devId;
    GT_U32                  coreClock;
    GT_U32                  coreId;
    GT_U32                  ii;
    GT_U32                  tmpValue;

    /* create semaphore for 'wait' for threads to start process */
    deviceObjPtr->smemInitPhaseSemaphore = SIM_OS_MAC(simOsSemCreate)(0,1);
    if (deviceObjPtr->smemInitPhaseSemaphore == (GT_SEM)0)
    {
        skernelFatalError(" skernelInit: cannot create semaphore");
        return;
    }

    deviceObjPtr->tmpPeerDeviceId = SMAIN_NOT_VALID_CNS;

    /* default in simulation : no address completion */
    /* only the GM is using it (see smemGmInit(...))*/
    deviceObjPtr->addressCompletionType = SKERNEL_ADDRESS_COMPLETION_TYPE_NONE_E;

    deviceObjPtr->interfaceBmp = SCIB_BUS_INTERFACE_PEX;/* default */

    /* get device type */
    sprintf(keyStr, "device_type%u", devId);
    if (!SIM_OS_MAC(simOsGetCnfValue)(devInfoSection, keyStr,
                   FILE_MAX_LINE_LENGTH_CNS, param_str))
    {
        /* device's type not found */
        skernelFatalError(" skernelInit: type for device %u not defined",
                                                devId);
    }

    /* convert to lower case */
    strlwr(param_str);

    strncpy(deviceObjPtr->deviceName, param_str,
                            SKERNEL_DEVICE_NAME_MAX_SIZE_CNS - 1/*save room for '/0'*/);
    deviceObjPtr->deviceName[SKERNEL_DEVICE_NAME_MAX_SIZE_CNS - 1] = 0;


    if(deviceObjPtr->portGroupSharedDevObjPtr)
    {
        if(deviceObjPtr->portGroupSharedDevObjPtr->softResetOldDevicePtr)
        {
            /* In case of soft reset get old device IDs for cores */
            coreId = 0;
            for(ii = 0; ii < deviceObjPtr->portGroupSharedDevObjPtr->
                numOfCoreDevs; ii++)
            {
                if(deviceObjPtr->portGroupSharedDevObjPtr->coreDevInfoPtr[ii].
                    devObjPtr == deviceObjPtr)
                {
                    coreId = ii;
                    break;
                }
            }
            portGroupDevId = deviceObjPtr->portGroupSharedDevObjPtr->
                softResetOldDevicePtr->coreDevInfoPtr[coreId].devObjPtr->
                deviceId;
        }
        else
        {
            portGroupDevId = lastDeviceIdUsedForCores++;
        }

        if(portGroupDevId >= SMAIN_MAX_NUM_OF_DEVICES_CNS)
        {
            skernelFatalError(" skernelInit: shared device out of range %u",devId);
        }

        smainDeviceObjects[portGroupDevId] = deviceObjPtr;
    }
    else
    {
        smainDeviceObjects[devId] = deviceObjPtr;
    }

    /* convert device type name to integer and get ports number  */
    smainDevInfoGet(param_str,deviceObjPtr);

    /* get device hwId */
    sprintf(keyStr, "dev%u_hw_id", devId);
    if(SIM_OS_MAC(simOsGetCnfValue)(devInfoSection, keyStr,
                   FILE_MAX_LINE_LENGTH_CNS, param_str))
    {
        sscanf(param_str, "%u", &devHwId);
    }
    else
    {
        devHwId = 0;
    }

    if(skernelIsPhyOnlyDev(deviceObjPtr))
    {
        if(devHwId > 31)
        {
            skernelFatalError(" skernelInit: PHY dev[%u] hold HW id > 31 [%u]",devId,devHwId);
        }
    }

    deviceObjPtr->deviceHwId = devHwId;

    if(deviceObjPtr->portGroupSharedDevObjPtr)
    {
        deviceObjPtr->deviceId = portGroupDevId;
    }
    else
    {
        deviceObjPtr->deviceId = devId;
    }


    /* get 'usePexLogic' indication */
    sprintf(keyStr, "dev%u_usePexLogic", devId);
    if(SIM_OS_MAC(simOsGetCnfValue)(devInfoSection, keyStr,
                   FILE_MAX_LINE_LENGTH_CNS, param_str))
    {
        sscanf(param_str, "%u", &tmpValue);

        /* SCIB layer uses this info to make sure the upper application (CPSS/extDrv)
            are using only appropriate SCIB APIs to access the device in this mode */
        deviceObjPtr->deviceForceBar0Bar2 = tmpValue ? GT_TRUE : GT_FALSE;
    }
    else
    {
        /* global indication that not requires INI file indication per device */
        deviceObjPtr->deviceForceBar0Bar2 = sinit_global_usePexLogic;
    }

    if(deviceObjPtr->deviceFamily == SKERNEL_EMPTY_FAMILY)
    {
        /* skip this device */
        goto exitCleanly_lbl;
    }

    switch(deviceObjPtr->deviceFamily)
    {
        case SKERNEL_CHEETAH_1_FAMILY:
        case SKERNEL_CHEETAH_2_FAMILY:
        case SKERNEL_CHEETAH_3_FAMILY:
        case SKERNEL_XCAT_FAMILY:
        case SKERNEL_XCAT3_FAMILY:
        case SKERNEL_AC5_FAMILY:
        case SKERNEL_XCAT2_FAMILY:
        case SKERNEL_LION_PORT_GROUP_FAMILY:
        case SKERNEL_LION2_PORT_GROUP_FAMILY:
        case SKERNEL_LION3_PORT_GROUP_FAMILY:
        case SKERNEL_BOBCAT2_FAMILY:
        case SKERNEL_BOBK_CAELUM_FAMILY:
        case SKERNEL_BOBK_CETUS_FAMILY:
        case SKERNEL_BOBK_ALDRIN_FAMILY:
        case SKERNEL_AC3X_FAMILY:
        case SKERNEL_BOBCAT3_FAMILY:
        case SKERNEL_ALDRIN2_FAMILY:
        case SKERNEL_FALCON_FAMILY:
        case SKERNEL_HAWK_FAMILY:
        case SKERNEL_PHOENIX_FAMILY:
        case SKERNEL_HARRIER_FAMILY:
        case SKERNEL_IRONMAN_FAMILY:
        case SKERNEL_PIPE_FAMILY:
            deviceObjPtr->isWmDxDevice = 1;
            break;
        default:
            deviceObjPtr->isWmDxDevice = 0;
            break;
    }

    /* Get device revision ID */
    smainDeviceRevisionGet(deviceObjPtr, devId);

    /* allocate and reset SLAN table for device */
    if(deviceObjPtr->numSlans)
    {
        deviceObjPtr->portSlanInfo = smemDeviceObjMemoryAlloc(deviceObjPtr,1, sizeof (SMAIN_PORT_SLAN_INFO)
                                                 * deviceObjPtr->numSlans);
    }

    /* Add FCS 4 bytes receive/send packets SLAN */
    sprintf(keyStr, "dev%u_to_slan_fcs_bytes_add", devId);
    if(SIM_OS_MAC(simOsGetCnfValue)(devInfoSection, keyStr,
                   FILE_MAX_LINE_LENGTH_CNS, param_str))
    {
        sscanf(param_str, "%u", &deviceObjPtr->crcPortsBytesAdd);
    }
    else
    {
        /* Default value is ADD for a backward compatible */
        deviceObjPtr->crcPortsBytesAdd = 0;
    }

    if (deviceObjPtr->deviceType == SKERNEL_NIC)
    {
        /* store NIC device pointer */
        smainNicDeviceObjPtr = deviceObjPtr;

        /* init NIC simulation */
        smainNicInit(deviceObjPtr);
        goto exitCleanly_lbl;
    }

    if(skernelIsLionShellOnlyDev(deviceObjPtr) ||
       skernelIsPhyShellOnlyDev(deviceObjPtr) ||
       skernelIsPumaShellOnlyDev(deviceObjPtr))
    {
        deviceObjPtr->shellDevice = GT_TRUE;
    }

    if(deviceObjPtr->shellDevice == GT_FALSE)
    {
        if(deviceObjPtr->softResetOldDevicePtr)
        {
            /* the bufPool,queueId already taken (copied) from the old device */
        }
        else
        {
            /* create buffers pool */
            deviceObjPtr->bufPool = sbufPoolCreate(SMAIN_BUFFERS_NUMBER_CNS);

            /* create SQueue */
            deviceObjPtr->queueId = squeCreate();
        }
    }

    /* Init uplink partner info */
    sprintf(currDevStr,"dev%d",devId);
    if (SIM_OS_MAC(simOsGetCnfValue)("uplink",
                         currDevStr,
                         FILE_MAX_LINE_LENGTH_CNS,
                         currRightSideDevStr))
    {
        /* build dev str */
        sscanf(currRightSideDevStr,"dev%d",&tmpRightSizeDev);
        deviceObjPtr->uplink.partnerDeviceID = tmpRightSizeDev;
    }

    /* Init and runs Golden Model simulation */
    if ((deviceObjPtr->gmDeviceType != SMAIN_NO_GM_DEVICE) &&
        (deviceObjPtr->gmDeviceType != GOLDEN_MODEL))
    {
        /* support removed */
    }
    else
    if (deviceObjPtr->gmDeviceType == GOLDEN_MODEL)
    {
        /* specific GM initializations */
        skernelInitGmDevParse(deviceObjPtr,devInfoSection,devId);
    }
    else
    {
        /* specific device (non-GM) initializations */
        skernelInitDevParsePart1(deviceObjPtr,devInfoSection,devId);
    }

    if(sasicgSimulationRole == SASICG_SIMULATION_ROLE_NON_DISTRIBUTED_E &&
       (0 == simDisconnectAllOuterPorts_mode))
    {
        SIM_OS_MAC(simOsSlanInit)();

        if(deviceObjPtr->portGroupSharedDevObjPtr)
        {
            smainDevice2SlanBind(deviceObjPtr,
                                 devId,
                                 deviceObjPtr->portGroupSharedDevObjPtr->coreDevInfoPtr[deviceObjPtr->portGroupId].startPortNum);
        }
        else
        {
            /* bind with SLANs                 */
            smainDevice2SlanBind(deviceObjPtr,devId,0);
        }

        SIM_OS_MAC(simOsSlanStart)();
    }

    /* the interrupt line of this device may be connected to one of the MPPs
       (multi-purpose pins) of another device */
    sprintf(keyStr, "dev%u", devId);
    if(SIM_OS_MAC(simOsGetCnfValue)("interrupts_connections", keyStr,FILE_MAX_LINE_LENGTH_CNS, param_str))
    {
        sscanf(param_str, "dev%u_mpp_%u",
            &deviceObjPtr->interruptLineToOtherDevice.otherDev_deviceId,
            &deviceObjPtr->interruptLineToOtherDevice.otherDev_mppId);

        deviceObjPtr->interruptLineToOtherDevice.isInterruptLineToOtherDevice = 1;

        /* bind the SCIB to call the other device when my device need to trigger
           interrupt . see function scibSetInterrupt(...) */
        scibBindInterruptMppTrigger(devId,smainForScibBindInterruptMppTrigger);
    }

    deviceObjPtr->interruptLine = SCIB_INTERRUPT_LINE_NOT_USED_CNS;
    if(deviceObjPtr->interruptLineToOtherDevice.isInterruptLineToOtherDevice == 0)
    {
        /* bind with interrupts */
        sprintf(keyStr, "dev%u_int_line", devId);
        if(SIM_OS_MAC(simOsGetCnfValue)(devInfoSection, keyStr,
                      FILE_MAX_LINE_LENGTH_CNS, param_str))
        {
            sscanf(param_str, "%u", &intLine);
            if((sasicgSimulationRole == SASICG_SIMULATION_ROLE_NON_DISTRIBUTED_E)||
                (sasicgSimulationRole == SASICG_SIMULATION_ROLE_DISTRIBUTED_ASIC_SIDE_E))
            {
                scibSetIntLine(deviceObjPtr->deviceId,intLine);
            }
            deviceObjPtr->interruptLine = intLine;
        }
    }
    else
    {
        /* we not care about interrupt line 'number' of the device because it is
           connected via other device */
    }

    /* check if pex used */
    sprintf(keyStr, "dev%u_use_pex_to_cpu", devId);
    if(SIM_OS_MAC(simOsGetCnfValue)(devInfoSection, keyStr,
                  FILE_MAX_LINE_LENGTH_CNS, param_str))
    {
        sscanf(param_str, "%u", &tmpValue);
        if(tmpValue == 0)
        {
            /* not used */
            deviceObjPtr->interfaceBmp &= ~SCIB_BUS_INTERFACE_PEX;
        }
        else
        {
            deviceObjPtr->interfaceBmp |= SCIB_BUS_INTERFACE_PEX;
        }
    }

    /* check if smi used */
    sprintf(keyStr, "dev%u_use_smi_to_cpu", devId);
    if(SIM_OS_MAC(simOsGetCnfValue)(devInfoSection, keyStr,
                  FILE_MAX_LINE_LENGTH_CNS, param_str))
    {
        sscanf(param_str, "%u", &tmpValue);
        if(tmpValue == 0)
        {
            /* not used */
            deviceObjPtr->interfaceBmp &= ~SCIB_BUS_INTERFACE_SMI;
        }
        else
        {
            deviceObjPtr->interfaceBmp &= ~SCIB_BUS_INTERFACE_SMI_INDIRECT;
            deviceObjPtr->interfaceBmp |= SCIB_BUS_INTERFACE_SMI;
        }
    }

    /* check if indirect SMI used */
    sprintf(keyStr, "dev%u_use_smi_to_cpu_indirect", devId);
    if(SIM_OS_MAC(simOsGetCnfValue)(devInfoSection, keyStr,
                  FILE_MAX_LINE_LENGTH_CNS, param_str))
    {
        sscanf(param_str, "%u", &tmpValue);
        if(tmpValue == 0)
        {
            /* not used */
            deviceObjPtr->interfaceBmp &= ~SCIB_BUS_INTERFACE_SMI_INDIRECT;
        }
        else
        {
            deviceObjPtr->interfaceBmp &= ~SCIB_BUS_INTERFACE_SMI;
            deviceObjPtr->interfaceBmp |= SCIB_BUS_INTERFACE_SMI_INDIRECT;
        }
    }


    if(deviceObjPtr->shellDevice == GT_FALSE)
    {
        sprintf(keyStr, "dev%u_core_clock", devId);
        if(SIM_OS_MAC(simOsGetCnfValue)(devInfoSection, keyStr,
                      FILE_MAX_LINE_LENGTH_CNS, param_str))
        {
            sscanf(param_str, "%u", &coreClock);/* value in MHz */

            if(deviceObjPtr->isWmDxDevice)
            {
                smemCheetahUpdateCoreClockRegister(
                    deviceObjPtr,coreClock,SMAIN_NOT_VALID_CNS);
            }
        }
    }



    /* Add FCS 4 bytes for send to CPU packets */
    sprintf(keyStr, "dev%u_to_cpu_fcs_bytes_add", devId);
    if(SIM_OS_MAC(simOsGetCnfValue)(devInfoSection, keyStr,
                   FILE_MAX_LINE_LENGTH_CNS, param_str))
    {
        sscanf(param_str, "%u", &deviceObjPtr->crcBytesAdd);
    }
    else
    {
        /* Default value is ADD for a backward compatible */
        deviceObjPtr->crcBytesAdd = 1;

        if(deviceObjPtr->portGroupSharedDevObjPtr)
        {
            /* we allow the port group device to use the info from the 'portGroupSharedDevObjPtr'
               device */
            if(SIM_OS_MAC(simOsGetCnfValue)(INI_FILE_SYSTEM_SECTION_CNS, keyStr,
                                            FILE_MAX_LINE_LENGTH_CNS, param_str))
            {
                sscanf(param_str, "%u", &deviceObjPtr->crcBytesAdd);
            }
        }
    }

    /* Enable FCS calculation for ethernet packets */
    sprintf(keyStr, "dev%u_calc_fcs_enable", devId);
    if(SIM_OS_MAC(simOsGetCnfValue)(devInfoSection, keyStr,
                   FILE_MAX_LINE_LENGTH_CNS, param_str))
    {
        sscanf(param_str, "%u", &deviceObjPtr->calcFcsEnable);
    }
    else
    {
        /* Default value is disable FCS calculation  */
        deviceObjPtr->calcFcsEnable = 0;
    }

    /* the PHY need to know the 'smi_bus_id' that the HW_id is limited to this context */
    sprintf(keyStr, "dev%u_bus_id", devId);
    if(SIM_OS_MAC(simOsGetCnfValue)(devInfoSection, keyStr,FILE_MAX_LINE_LENGTH_CNS, param_str))
    {
        sscanf(param_str, "%u", &deviceObjPtr->busId);
    }
    else
    {
        deviceObjPtr->busId = SMAIN_NOT_VALID_CNS;
    }

    /* set bus info */
    smainBusSet(deviceObjPtr);

exitCleanly_lbl:
    SIM_OS_MAC(simOsSemDelete)(deviceObjPtr->smemInitPhaseSemaphore);
    deviceObjPtr->smemInitPhaseSemaphore = (GT_SEM)0;
}

/**
* @internal skernelInitDevUplinkParse function
* @endinternal
*
* @brief   Init for SKernel devices with UPlinks
*
* @param[in] deviceObjPtr             - allocated pointer for the device
* @param[in] devId                    - the deviceId in the INI file info
*/
static void skernelInitDevUplinkParse
(
    IN SKERNEL_DEVICE_OBJECT * deviceObjPtr,
    IN GT_U32                  devId
)
{
    char                    currDevStr[SMAIN_MIN_SIZE_OF_BUFFER_CNS]={0};
    char                    currRightSideDevStr[FILE_MAX_LINE_LENGTH_CNS]={0};

    deviceObjPtr->uplink.type = SKERNEL_UPLINK_TYPE_NONE_E;

    if (deviceObjPtr->gmDeviceType != SMAIN_NO_GM_DEVICE)
    {
        return;
    }

    sprintf(currDevStr,"dev%d",devId);
    if (!SIM_OS_MAC(simOsGetCnfValue)("uplink",
                        currDevStr,
                        FILE_MAX_LINE_LENGTH_CNS,
                        currRightSideDevStr))
    {
        return;
    }
    skernelFatalError(" smainDevInfoGet: illegal \
                                   configuration for devId %d ", devId);
}

/* info about the internal connections in the INI file.
   from devices that already parsed it */
typedef struct{
    GT_U32  devId;
    GT_U32  portNum;
    struct
    {
        GT_U32  devId;
        GT_U32  portNum;
    }info;
}INTERNAL_CONNECTION_INFO_STC;
/* max index in array : internalConnectionArr[] */
static GT_U32  internalConnectionMaxIndex = 0;
#define MAX_INTERNAL_CONNECTION_CNS 50
/* array of internal connections from the INI file.
   from devices that already parsed it  */
static INTERNAL_CONNECTION_INFO_STC internalConnectionArr[MAX_INTERNAL_CONNECTION_CNS] =
    {{0,0}};

static GT_BOOL isInInternalConnectionDb(
    IN GT_U32    devId ,
    IN GT_U32    portNum,
    OUT GT_U32   *connectedDevIdPtr,
    OUT GT_U32   *connectedPortNumPtr
)
{
    GT_U32  ii;
    for(ii = 0; ii < internalConnectionMaxIndex; ii++)
    {
        if(internalConnectionArr[ii].devId == devId &&
           internalConnectionArr[ii].portNum == portNum)
        {
            *connectedDevIdPtr   = internalConnectionArr[ii].info.devId;
            *connectedPortNumPtr = internalConnectionArr[ii].info.portNum;

            return GT_TRUE;
        }

        if(internalConnectionArr[ii].info.devId == devId &&
           internalConnectionArr[ii].info.portNum == portNum)
        {
            *connectedDevIdPtr   = internalConnectionArr[ii].devId;
            *connectedPortNumPtr = internalConnectionArr[ii].portNum;

            return GT_TRUE;
        }
    }

    *connectedDevIdPtr   = 0;
    *connectedPortNumPtr = 0;
    return GT_FALSE;
}

static void addInternalConnectionDb(
    IN GT_U32  devId,
    IN GT_U32  portIterator,
    IN GT_U32  connectedDevId,
    IN GT_U32  connectedPortNum
)
{
    GT_U32  ii = internalConnectionMaxIndex;

    if(ii >= MAX_INTERNAL_CONNECTION_CNS)
    {
        /* the DB if full , do not add to it */
        return;
    }

    internalConnectionArr[ii].devId        = devId;
    internalConnectionArr[ii].portNum      = portIterator;
    internalConnectionArr[ii].info.devId   = connectedDevId;
    internalConnectionArr[ii].info.portNum = connectedPortNum;

    internalConnectionMaxIndex++;
}

/**
* @internal skernelInitInternalConnectionParse function
* @endinternal
*
* @brief   Init for SKernel for 'internal connection' that not need slans
*
* @param[in] deviceObjPtr             - allocated pointer for the device
* @param[in] devId                    - the deviceId in the INI file info
*/
static void skernelInitInternalConnectionParse
(
    IN SKERNEL_DEVICE_OBJECT * deviceObjPtr,
    IN GT_U32                  devId
)
{
    char                    currDevStr[SMAIN_MIN_SIZE_OF_BUFFER_CNS]={0}; /*string to build device+port for lookup in INI file */
    char                    currRightSideInfoStr[FILE_MAX_LINE_LENGTH_CNS]={0};/*string to read from INI file*/
    GT_U32                  tmpRightSizeDev; /* right side device id */
    GT_U32                  tmpRightSizePort;/* right side port id */
    SKERNEL_DEVICE_OBJECT   *destDeviceObjPtr;/* (pointer to)right side device object */
    SMAIN_PORT_SLAN_INFO    *slanInfoPtr; /* slan port entry pointer */
    GT_U32                  portIterator; /* port iterator */
    GT_U32                  ii;/* iterator */
    GT_U8                   leftSideCorePort;/* left side port number -- in context of the core */
    GT_U8                   rightSideCorePort;/* right side port number -- in context of the core */
    GT_U32                  portsNumber; /* number of ports to loop over */
    SKERNEL_DEVICE_OBJECT   *leftSideDeviceCoreObjPtr;/*(pointer to) left side device core object*/
    SKERNEL_DEVICE_OBJECT   *rightSideDeviceCoreObjPtr;/*(pointer to) right side device core object*/
    GT_BOOL                 isInInternalDb;

    if(deviceObjPtr->shellDevice == GT_TRUE)
    {
        leftSideDeviceCoreObjPtr = deviceObjPtr->coreDevInfoPtr[deviceObjPtr->numOfCoreDevs - 1].devObjPtr;
        if(leftSideDeviceCoreObjPtr == NULL)
        {
            /* this can happen when the core is 'not valid' */
            return;
        }
        portsNumber = deviceObjPtr->coreDevInfoPtr[deviceObjPtr->numOfCoreDevs - 1].startPortNum +
                      leftSideDeviceCoreObjPtr->portsNumber;
        if(portsNumber > SKERNEL_DEV_MAX_SUPPORTED_PORTS_CNS)
        {
            portsNumber = SKERNEL_DEV_MAX_SUPPORTED_PORTS_CNS;
        }
    }
    else
    {
        portsNumber = deviceObjPtr->portsNumber;
    }

    /* initialize ' internal connections' */
    for(portIterator = 0 ; portIterator < (portsNumber + 1) ; portIterator ++)
    {
        if(portIterator == portsNumber)
        {
            /* last iteration */
            if(portIterator > PRESTERA_CPU_PORT_CNS)
            {
                /* CPU port already in the valid range of ports */
                break;
            }

            portIterator = PRESTERA_CPU_PORT_CNS;
        }

        tmpRightSizeDev = 0;
        tmpRightSizePort = 0;

        /* check the internal DB first */
        isInInternalDb = isInInternalConnectionDb(devId,portIterator,&tmpRightSizeDev,&tmpRightSizePort);
        if(isInInternalDb == GT_FALSE)
        {
            sprintf(currDevStr,"dev%d_port%d",devId,portIterator);
            if (!SIM_OS_MAC(simOsGetCnfValue)("internal_connections",
                                currDevStr,
                                FILE_MAX_LINE_LENGTH_CNS,
                                currRightSideInfoStr))
            {
                continue;
            }
        }

        if(deviceObjPtr->shellDevice == GT_TRUE)
        {
            /* find to which core the port belongs */
            for(ii = 0 ; ii < (deviceObjPtr->numOfCoreDevs - 1) ;ii++)
            {
                if(portIterator < deviceObjPtr->coreDevInfoPtr[ii + 1].startPortNum)
                {
                    break;
                }
            }

            leftSideDeviceCoreObjPtr = deviceObjPtr->coreDevInfoPtr[ii].devObjPtr;
            if(leftSideDeviceCoreObjPtr == NULL)
            {
                /* this can happen when the core is 'not valid' */
                continue;
            }
            leftSideCorePort = portIterator - deviceObjPtr->coreDevInfoPtr[ii].startPortNum;
        }
        else
        {
            leftSideDeviceCoreObjPtr = deviceObjPtr;
            leftSideCorePort = portIterator;
        }

        if(portIterator == PRESTERA_CPU_PORT_CNS)
        {
            /*for CPU port we not check 'state'*/
        }
        else
        if(leftSideCorePort >= leftSideDeviceCoreObjPtr->portsNumber ||
           leftSideDeviceCoreObjPtr->portsArr[leftSideCorePort].state == SKERNEL_PORT_STATE_NOT_EXISTS_E)
        {
            skernelFatalError("skernelInit: cant bind non exists port to internal connection in dev[%d],port[%d] \n",devId, portIterator);
        }

        slanInfoPtr=&(leftSideDeviceCoreObjPtr->portSlanInfo[leftSideCorePort]);
        if(slanInfoPtr->slanName[0] != 0)
        {
            skernelFatalError("skernelInit: cant bind to SLAN and to internal connection in dev[%d],port[%d] \n",devId, portIterator);
        }

        if(isInInternalDb == GT_FALSE)
        {
            /* build dev , port str */
            sscanf(currRightSideInfoStr,"dev%d_port%d",&tmpRightSizeDev,&tmpRightSizePort);

            /* add it to the internal DB */
            addInternalConnectionDb(devId,portIterator,tmpRightSizeDev,tmpRightSizePort);
        }


        destDeviceObjPtr = smainDeviceObjects[tmpRightSizeDev];

        if(tmpRightSizeDev >= smainDevicesNumber)
        {
            skernelFatalError("skernelInit: cant bind to non exists device[%d],port[%d] \n",tmpRightSizeDev, tmpRightSizePort);
        }

        if(destDeviceObjPtr == NULL)
        {
            /* the device is not 'UP' yet ... allow it to try the proper line too. */
            /* needed for xCat3x after soft reset of Aldrin */
            continue;
        }

        if(destDeviceObjPtr->shellDevice == GT_TRUE)
        {
            /* find to which core the port belongs */
            for(ii = 0 ; ii < (destDeviceObjPtr->numOfCoreDevs - 1) ;ii++)
            {
                if(tmpRightSizePort < destDeviceObjPtr->coreDevInfoPtr[ii + 1].startPortNum)
                {
                    break;
                }
            }

            rightSideDeviceCoreObjPtr = destDeviceObjPtr->coreDevInfoPtr[ii].devObjPtr;
            rightSideCorePort = tmpRightSizePort - destDeviceObjPtr->coreDevInfoPtr[ii].startPortNum;
        }
        else
        {
            rightSideDeviceCoreObjPtr = destDeviceObjPtr;
            rightSideCorePort = tmpRightSizePort;
        }



        if(rightSideCorePort == PRESTERA_CPU_PORT_CNS)
        {
            /*for CPU port we not check 'state'*/
        }
        else
        if(rightSideCorePort >= rightSideDeviceCoreObjPtr->portsNumber ||
           rightSideDeviceCoreObjPtr->portsArr[rightSideCorePort].state == SKERNEL_PORT_STATE_NOT_EXISTS_E )
        {
            skernelFatalError("skernelInit: cant bind non exists port to internal connection in dev[%d],port[%d] \n",tmpRightSizeDev, tmpRightSizePort);
        }

        leftSideDeviceCoreObjPtr->portsArr[leftSideCorePort].linkStateWhenNoForce = SKERNEL_PORT_NATIVE_LINK_UP_E;
        /* notify that the port is up - on left side device */
        snetLinkStateNotify(leftSideDeviceCoreObjPtr, leftSideCorePort, 1);

        leftSideDeviceCoreObjPtr->portsArr[leftSideCorePort].peerInfo.usePeerInfo = GT_TRUE;
        leftSideDeviceCoreObjPtr->portsArr[leftSideCorePort].peerInfo.peerDeviceId = rightSideDeviceCoreObjPtr->deviceId;
        leftSideDeviceCoreObjPtr->portsArr[leftSideCorePort].peerInfo.peerPortNum =  rightSideCorePort;
        leftSideDeviceCoreObjPtr->portsArr[leftSideCorePort].isOnBoardInternalPort = GT_TRUE;


        rightSideDeviceCoreObjPtr->portsArr[rightSideCorePort].linkStateWhenNoForce = SKERNEL_PORT_NATIVE_LINK_UP_E;
        /* notify that the port is up - on right side device */
        snetLinkStateNotify(rightSideDeviceCoreObjPtr, rightSideCorePort, 1);

        rightSideDeviceCoreObjPtr->portsArr[rightSideCorePort].peerInfo.usePeerInfo = GT_TRUE;
        rightSideDeviceCoreObjPtr->portsArr[rightSideCorePort].peerInfo.peerDeviceId = leftSideDeviceCoreObjPtr->deviceId;
        rightSideDeviceCoreObjPtr->portsArr[rightSideCorePort].peerInfo.peerPortNum =  leftSideCorePort;
        rightSideDeviceCoreObjPtr->portsArr[rightSideCorePort].isOnBoardInternalPort = GT_TRUE;
    }

    return;
}

/**
* @internal skernelInitInternalConnectionParse function
* @endinternal
*
* @brief  Init secondary application for SKernel for 'Distributed simulation'
*         send to the application side the minimal info needed about the devices
*
* @param[in] clientIndex             - application client index
*/
void skernelInitDistributedSecondaryClient
(
    IN GT_U32   clientIndex
)
{
    SKERNEL_DEVICE_OBJECT * deviceObjPtr;  /* pointer to the device object*/
    SIM_DISTRIBUTED_INIT_DEVICE_STC *devicesArray;
    GT_U32 numDevs = sinitOwnBoardSection.numDevices ? sinitOwnBoardSection.numDevices :smainDevicesNumber;
    GT_U32 ii;
    GT_U32 devId; /* device id loop */

    if(clientIndex>0)
    {
        devicesArray = malloc(numDevs * sizeof(SIM_DISTRIBUTED_INIT_DEVICE_STC));

        for (ii=0,devId = 0; devId < smainDevicesNumber; devId++)
        {
            if (!smainDeviceObjects[devId])
               continue;

            deviceObjPtr = smainDeviceObjects[devId]   ;

            devicesArray[ii].deviceId    = deviceObjPtr->deviceId;
            devicesArray[ii].deviceHwId  = deviceObjPtr->deviceHwId;
            devicesArray[ii].interruptLine = deviceObjPtr->interruptLine;
            devicesArray[ii].addressCompletionStatus = ADDRESS_COMPLETION_STATUS_GET_MAC(deviceObjPtr);
            devicesArray[ii].nicDevice = IS_NIC_DEVICE_MAC(deviceObjPtr);

            switch(deviceObjPtr->deviceFamily)
            {
                case SKERNEL_FAP_DUNE_FAMILY:
                case SKERNEL_FA_FOX_FAMILY:
                    devicesArray[ii].isPp = 0;
                    break;
                default:
                    devicesArray[ii].isPp = 1;
                    break;
            }

            ii++;
        }
        /* since next command may wait for the other side to connect ,
           we notify that most issues done , and terminal can be connected.
        */
        simulationInitReady = 1;

        /* sent initialization to the remote SCIB ! */
        simDistributedRemoteInit(numDevs,devicesArray,clientIndex);

        free(devicesArray);
        devicesArray = NULL;
    }
}


/**
* @internal skernelInitDistributedDevParse function
* @endinternal
*
* @brief   Init for SKernel for 'Distributed simulation'
*         send to the application side the minimal info needed about the devices
*/
static void skernelInitDistributedDevParse
(
    void
)
{
    SKERNEL_DEVICE_OBJECT * deviceObjPtr;  /* pointer to the device object*/
    SIM_DISTRIBUTED_INIT_DEVICE_STC *devicesArray;
    GT_U32 numDevs = sinitOwnBoardSection.numDevices ? sinitOwnBoardSection.numDevices :smainDevicesNumber;
    GT_U32 ii;
    GT_U32 devId; /* device id loop */

    devicesArray = malloc(numDevs * sizeof(SIM_DISTRIBUTED_INIT_DEVICE_STC));

    for (ii=0,devId = 0; devId < smainDevicesNumber; devId++)
    {
        if (!smainDeviceObjects[devId])
           continue;

        deviceObjPtr = smainDeviceObjects[devId]   ;

        devicesArray[ii].deviceId    = deviceObjPtr->deviceId;
        devicesArray[ii].deviceHwId  = deviceObjPtr->deviceHwId;
        devicesArray[ii].interruptLine = deviceObjPtr->interruptLine;
        devicesArray[ii].addressCompletionStatus = ADDRESS_COMPLETION_STATUS_GET_MAC(deviceObjPtr);
        devicesArray[ii].nicDevice = IS_NIC_DEVICE_MAC(deviceObjPtr);

        switch(deviceObjPtr->deviceFamily)
        {
            case SKERNEL_FAP_DUNE_FAMILY:
            case SKERNEL_FA_FOX_FAMILY:
                devicesArray[ii].isPp = 0;
                break;
            default:
                devicesArray[ii].isPp = 1;
                break;
        }

        ii++;
    }
    /* since next command may wait for the other side to connect ,
       we notify that most issues done , and terminal can be connected.
    */
    simulationInitReady = 1;

    /* sent initialization to the remote SCIB ! */
    simDistributedRemoteInit(numDevs,devicesArray,0);

    free(devicesArray);
    devicesArray = NULL;

    /* the slan notification involve generation of interrupt */
    /* all messages to Server must come after
       simDistributedRemoteInit(...) */
    for (devId = 0; devId < smainDevicesNumber; devId++)
    {
        if (!smainDeviceObjects[devId])
           continue;

        deviceObjPtr = smainDeviceObjects[devId]   ;

        SIM_OS_MAC(simOsSlanInit)();

        if(deviceObjPtr->portGroupSharedDevObjPtr)
        {
            smainDevice2SlanBind(deviceObjPtr,
                                 devId,
                                 deviceObjPtr->portGroupSharedDevObjPtr->coreDevInfoPtr[deviceObjPtr->portGroupId].startPortNum);
        }
        else
        {
            /* bind with SLANs                 */
            smainDevice2SlanBind(deviceObjPtr,devId,0);
        }

        SIM_OS_MAC(simOsSlanStart)();
    }
}

/**
* @internal skernelInitDevicePart1 function
* @endinternal
*
* @brief   Init for SKernel device
*
* @param[in] deviceObjPtr             - allocated pointer for the device
* @param[in] devId                    - the deviceId in the INI file info
*/
static void skernelInitDevicePart1
(
    IN SKERNEL_DEVICE_OBJECT * deviceObjPtr,
    IN GT_U32                  devId
)
{
    SKERNEL_DEVICE_OBJECT * portGroupDeviceObjPtr;  /* pointer to the device object*/
    char                    devInfoSection[SMAIN_MIN_SIZE_OF_BUFFER_CNS];/*the device info section in the INI file*/
    char                    param_str[FILE_MAX_LINE_LENGTH_CNS]; /* string for parameter */
    char                    keyStr[SMAIN_MIN_SIZE_OF_BUFFER_CNS]; /* key string*/
    char                   *deviceName;     /* Display device name */
    GT_U32                  jj;

    sprintf(devInfoSection, "%s",INI_FILE_SYSTEM_SECTION_CNS);

    /* call to init the device */
    skernelInitDevParse(deviceObjPtr,devInfoSection,devId);

    if(deviceObjPtr->deviceFamily == SKERNEL_EMPTY_FAMILY)
    {
        /* disconnect from the array of devices */
        smainDeviceObjects[deviceObjPtr->deviceId] = NULL;

        /* free the allocated memory */
        smemDeviceObjMemoryPtrFree(NULL,deviceObjPtr);

        /* the memory is free ! */
        deviceObjPtr = NULL;

        simGeneralPrintf("device[%u] skipped \n",devId);

        /* skip this device */
        return;
    }

    if (SKERNEL_DEVICE_FAMILY_SOHO_PERIDOT(deviceObjPtr->deviceType))
    {
        deviceName = deviceObjPtr->deviceName;
    }
    else
    if(0 == strcmp(deviceObjPtr->deviceName,"aldrin3m"))
    {
        deviceName =  "SKERNEL_ALDRIN3M_FAMILY ";
    }
    else
    {
        deviceName = familyNamesArr[deviceObjPtr->deviceFamily];
    }
    simForcePrintf("device[%u] ready : %s \n",devId,deviceName);
    simForcePrintf("allocated [%u] bytes \n",deviceObjPtr->totalNumberOfBytesAllocated);


    if(deviceObjPtr->numOfCoreDevs == 0 ||
       deviceObjPtr->coreDevInfoPtr[0].devObjPtr)
    {
        return;
    }

    simForcePrintf(" ** Multi-core device , start init it's cores \n");

    /* create the egress protecting mutex */
    if(deviceObjPtr->softResetOldDevicePtr)
    {
        SIM_TAKE_PARAM_FROM_OLD_DEVICE_MAC(deviceObjPtr,protectEgressMutex);
    }
    else
    {
        deviceObjPtr->protectEgressMutex = SIM_OS_MAC(simOsMutexCreate)();
    }

    for(jj = 0 ; jj < deviceObjPtr->numOfCoreDevs; jj++)
    {
        if(deviceObjPtr->useCoreTerminology == GT_TRUE)
        {
            sprintf(devInfoSection, "dev%u_core%u",devId,jj);

            /* get valid port group value */
            sprintf(keyStr,"%s" ,"valid_core");
        }
        else
        {
            sprintf(devInfoSection, "dev%u_port_group%u",devId,jj);

            /* get valid port group value */
            sprintf(keyStr,"%s" ,"valid_port_group");
        }

        if (SIM_OS_MAC(simOsGetCnfValue)(devInfoSection, keyStr,
                       FILE_MAX_LINE_LENGTH_CNS, param_str))
        {
            sscanf(param_str, "%u", &deviceObjPtr->coreDevInfoPtr[jj].validPortGroup);

            if(deviceObjPtr->coreDevInfoPtr[jj].validPortGroup == 0)
            {
                /*we not allocate resources for the 'Non-valid' port group */
                continue;
            }
        }
        else
        {
            /* by default the port group is valid */
            deviceObjPtr->coreDevInfoPtr[jj].validPortGroup = 1;
        }


        portGroupDeviceObjPtr = smemDeviceObjMemoryAlloc(NULL,1, sizeof(SKERNEL_DEVICE_OBJECT));

        /* bind the device object to 'father' device object pointer */
        portGroupDeviceObjPtr->portGroupSharedDevObjPtr = deviceObjPtr;
        /* bind the 'port group device' to the array of device info array */
        deviceObjPtr->coreDevInfoPtr[jj].devObjPtr = portGroupDeviceObjPtr;

        if(deviceObjPtr->softResetOldDevicePtr)
        {
            /* Set softResetOldDevicePtr device for softresetting
            corresponding portgroups */
            portGroupDeviceObjPtr->softResetOldDevicePtr =
                deviceObjPtr->softResetOldDevicePtr->coreDevInfoPtr[jj].
                devObjPtr;

            /* Set softResetNewDevicePtr device for 'old' device */
            deviceObjPtr->softResetOldDevicePtr->coreDevInfoPtr[jj].
                devObjPtr->softResetNewDevicePtr = portGroupDeviceObjPtr;

            /* Reuse buffer pool */
            portGroupDeviceObjPtr->bufPool = deviceObjPtr->softResetOldDevicePtr
                ->coreDevInfoPtr[jj].devObjPtr->bufPool;

            /* Reuse message queue */
            portGroupDeviceObjPtr->queueId = deviceObjPtr->softResetOldDevicePtr
                ->coreDevInfoPtr[jj].devObjPtr->queueId;

            /* Reuse DDR3 buffer pools */
            portGroupDeviceObjPtr->tmInfo.bufPool_DDR3_TM = deviceObjPtr->
                softResetOldDevicePtr->coreDevInfoPtr[jj].devObjPtr->tmInfo.
                bufPool_DDR3_TM;

            /* Reuse skernel task cookie */
            portGroupDeviceObjPtr->task_skernelCookieInfo.generic =
                deviceObjPtr->softResetOldDevicePtr->coreDevInfoPtr[jj].
                devObjPtr->task_skernelCookieInfo.generic;
        }

        /* set the portGroupId for the 'port group device' */
        portGroupDeviceObjPtr->portGroupId = jj;

        skernelInitDevParse(portGroupDeviceObjPtr, devInfoSection,devId);
        if(deviceObjPtr->useCoreTerminology == GT_TRUE)
        {
            simForcePrintf("device[%u] core[%d] devId[%d] ready : %s \n",
                devId,jj,portGroupDeviceObjPtr->deviceId,
                familyNamesArr[portGroupDeviceObjPtr->deviceFamily]);
        }
        else
        {
            simForcePrintf("device[%u] port group[%d] devId[%d] ready : %s \n",
                devId,jj,portGroupDeviceObjPtr->deviceId,
                familyNamesArr[portGroupDeviceObjPtr->deviceFamily]);
        }
        simForcePrintf("allocated [%u] bytes \n",portGroupDeviceObjPtr->totalNumberOfBytesAllocated);
    }

    SIM_OS_MAC(simOsSemDelete)(deviceObjPtr->smemInitPhaseSemaphore);
    deviceObjPtr->smemInitPhaseSemaphore = (GT_SEM)0;
}

/**
* @internal skernelInitDevicePart2 function
* @endinternal
*
* @brief   Init for SKernel devices with UPlinks
*
* @param[in] deviceObjPtr             - allocated pointer for the device
* @param[in] devId                    - the deviceId in the INI file info
*/
static void skernelInitDevicePart2
(
    IN SKERNEL_DEVICE_OBJECT * deviceObjPtr,
    IN GT_U32                  devId
)
{
    /* initialize UPLINK information */
    skernelInitDevUplinkParse(deviceObjPtr,devId);

    /* initialize internal_connection information */
    skernelInitInternalConnectionParse(deviceObjPtr,devId);
}

/**
* @internal skernelForceLinkDownOrLinkUpOnAllPorts function
* @endinternal
*
* @brief   force link UP/DOWN on all ports
*
* @param[in] devObjPtr                - pointer to device object.
*                                      forceLinkUp -  GT_TRUE  - force link UP   on all ports
*                                      GT_FALSE - force link DOWN on all ports
*/
void skernelForceLinkDownOrLinkUpOnAllPorts
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_BOOL                 forceLinkUpOrLinkDown
)
{
    GT_U32  port;           /* port number */
    SMAIN_PORT_SLAN_INFO *  slanInfoPtr; /* SLAN info entry pointer */
    GT_U32                  value = (forceLinkUpOrLinkDown == GT_TRUE) ? 1 : 0;

    if(devObjPtr->devPortLinkUpdateFuncPtr == NULL)
    {
        return;
    }

    /* Force link of all active SLANs down */
    for (port = 0; port < devObjPtr->numSlans; port++)
    {
        /* get SLAN info for port */
        slanInfoPtr = &(devObjPtr->portSlanInfo[port]);
        if (slanInfoPtr->slanName[0x0] != 0x0)
        {
            /* calling snetChtLinkStateNotify */
            devObjPtr->devPortLinkUpdateFuncPtr(devObjPtr, port, value);
        }
    }
}
/**
* @internal skernelDeviceAllSlansUnbind function
* @endinternal
*
* @brief   unbind all SLANs of a device
*
* @param[in] devObjPtr                - pointer to device object.
*/
static void skernelDeviceAllSlansUnbind
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr
)
{
    GT_U32                port;
    SMAIN_PORT_SLAN_INFO *slanInfoPrt;          /* slan port entry pointer */

    /* make sure that all slans will wait for changing the name 'to indicate' unbind */
    SIM_OS_MAC(simOsSemWait)(slanDownSemaphore,SIM_OS_WAIT_FOREVER);

    /* under traffic we get 'exception' in the 'slan code' unless 'detach' before unbind */
    for (port = 0; port < devObjPtr->numSlans; port++)
    {
        /* get slan info */
        slanInfoPrt = &(devObjPtr->portSlanInfo[port]);
        slanInfoPrt->slanName[0] = 0;/* before the unbind , make sure no one try to send packet to this port */
    }

    SIM_OS_MAC(simOsSemSignal)(slanDownSemaphore);

    /* allow all other tasks that send traffic to not be inside the SLAN that going to be unbound */
    SIM_OS_MAC(simOsSleep)(1);

    for (port = 0; port < devObjPtr->numSlans; port++)
    {
        /* get slan info */
        slanInfoPrt = &(devObjPtr->portSlanInfo[port]);

        /* unbind the port */
        internalSlanUnbind(slanInfoPrt);
    }
}

/* print the PCIe capabilities and the enhanced capabilities */
static void smainDevicePcieCapabilitiesPrint(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr
)
{
    GT_U32 value;
    GT_U32 nextAddr,prevAddr;
    GT_U32 capability;

    scibPciRegRead(devObjPtr->deviceId,0x4,1,&value);

    simulationPrintf("Print PCIe config space capabilities  : (start at address 0x40) \n");

    if(0 == (value & 0x00100000))
    {
        simulationPrintf("The capabilities are not allowed (in PCIe address 0x4 , bit 20 is zero) \n");
        return;
    }

    scibPciRegRead(devObjPtr->deviceId,0x34,1,&value);

    nextAddr = (value & 0xFF);  /* probably address 0x40 */

    if(0 == nextAddr)
    {
        simulationPrintf("The capabilities are not allowed (in PCIe address 0x34 , bits 0..7 are zero) \n");
        return;
    }
    else
    if(nextAddr != 0x40)
    {
        simulationPrintf("WARNING : nextAddr [0x%2.2x] read from address 0x34 (bits 0..7) expected nextAddr 0x40 \n",nextAddr);
        return;
    }

    do
    {
        prevAddr = nextAddr;

        scibPciRegRead(devObjPtr->deviceId,nextAddr,1,&value);

        nextAddr   = (value >> 8) & 0xFF;
        capability = value & 0xFF;

        if(nextAddr == 0)
        {
            simulationPrintf("No more capabilities (ended in address [0x%x])\n",prevAddr);
            break;
        }

        /*
            power manager (0x1)
            MSI (0x5)
            PCI extended (0x10)
        */
        switch(capability)
        {
            case 0x1:
                simulationPrintf("Capability : Power manager (address[0x%x] value 0x1) \n",
                prevAddr);
                break;
            case 0x5:
                simulationPrintf("Capability : MSI (address[0x%x] value 0x5) \n",
                prevAddr);
                break;
            case 0x10:
                simulationPrintf("Capability : PCI extended (address[0x%x] value 0x10) \n",
                prevAddr);
                break;
            default:
                simulationPrintf("Capability : not listed in WM (address[0x%x] value [0x%x]) \n",
                    capability,prevAddr);
                break;
        }

        if(prevAddr == nextAddr)
        {
            /* break the 'endless loop' */
            simulationPrintf("WARNING : Capability : breaking endless loop of same address \n",
                capability,prevAddr);
            break;
        }

    }while(nextAddr);

    simulationPrintf("Print PCIe config space enhanced capabilities : (start at address 0x100) \n");

    nextAddr = 0x100;

    do
    {
        prevAddr = nextAddr;

        scibPciRegRead(devObjPtr->deviceId,nextAddr,1,&value);

        nextAddr   = (value >> 20) & 0xFFF;
        capability = value & 0xFFF;

        if(nextAddr == 0)
        {
            simulationPrintf("No more enhanced capabilities (ended in address [0x%x])\n",prevAddr);
            break;
        }

        /*
            AER (0x1)
            secondary PCIe (0x19)
            PL16G (0x26)
            PCIe margining (0x27)
            L1SUB (0x1E)
            vendor specific (0xB)
        */
        switch(capability)
        {
            case 0x1:
                simulationPrintf("Enhanced capability : AER (address[0x%x] value 0x1) \n",
                prevAddr);
                break;
            case 0xB:
                simulationPrintf("Enhanced capability : vendor specific (address[0x%x] value 0xB) \n",
                prevAddr);
                break;
            case 0x19:
                simulationPrintf("Enhanced capability : secondary PCIe (address[0x%x] value 0x19) \n",
                prevAddr);
                break;
            case 0x1E:
                simulationPrintf("Enhanced capability : L1SUB (address[0x%x] value 0x1E) \n",
                prevAddr);
                break;
            case 0x26:
                simulationPrintf("Enhanced capability : PL16G (address[0x%x] value 0x26) \n",
                prevAddr);
                break;
            case 0x27:
                simulationPrintf("Enhanced capability : PCIe margining (address[0x%x] value 0x27) \n",
                prevAddr);
                break;
            default:
                simulationPrintf("Enhanced capability : not listed in WM (address[0x%x] value [0x%x]) \n",
                prevAddr,capability);
                break;
        }

        if(prevAddr == nextAddr)
        {
            /* break the 'endless loop' */
            simulationPrintf("WARNING : Enhanced Capability : breaking endless loop of same address \n",
                capability,prevAddr);
            break;
        }
    }while(nextAddr);

    simulationPrintf("Print of capabilities ended \n");
}


#define WM_DEVICE_READY_FOR_ACCESS_BY_CPU_REG_ADDR  0x38
/* add indication to the PCI config space at reserved register that the reset was done
   see in cpssSimSoftResetDoneWait that wait for it */
static void smainDeviceReadyForAccessFromCpu
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr
)
{
    GT_U32 value;
    if(!devObjPtr->isWmDxDevice || !devObjPtr->isPciCompatible)
    {
        return;
    }

    scibPciRegRead(devObjPtr->deviceId,WM_DEVICE_READY_FOR_ACCESS_BY_CPU_REG_ADDR,1,&value);
    value |= 0x80000000;
    scibPciRegWrite(devObjPtr->deviceId,WM_DEVICE_READY_FOR_ACCESS_BY_CPU_REG_ADDR,1,&value);

    if(SMEM_CHT_IS_SIP6_GET(devObjPtr))
    {
        /* print the PCIe capabilities and the enhanced capabilities */
        smainDevicePcieCapabilitiesPrint(devObjPtr);
    }
}


/*******************************************************************************
*   pha_fw_softReset
*
* DESCRIPTION:
*
*       as part of the 'soft reset' :
*       release the PHA 'bind' to the memory of 'old' device , so can be
*       reallocated by the new device
*
* INPUTS:
*       oldDeviceObjPtr - pointer to the old device
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       None.
* COMMENTS:
*       1. called in context of skernel task
*       2. must be called only for 'PHA' supporting device
*
*******************************************************************************/
static void pha_fw_softReset
(
    IN SKERNEL_DEVICE_OBJECT * oldDeviceObjPtr
)
{
    GT_U32  ii;
    MEMORY_RANGE *currentMemEntryPtr;
    currentMemEntryPtr = &oldDeviceObjPtr->PHA_FW_support.pha_memoryMap[0];

    for(ii = 0 ; currentMemEntryPtr->numOfBytes != 0 ;ii++,currentMemEntryPtr++)
    {
        /* detach old memory so the 'new device' can have new valid allocation during skernelInitDevicePart1(...) */
        currentMemEntryPtr->actualMemoryPtr = NULL;
    }
}

static WM_STATUS internal_skernelPortConnectionInfo
(
    IN SKERNEL_DEVICE_OBJECT        *devObjPtr,
    IN GT_U32                       portNum,
    IN WM_PORT_CONNECTION_INFO_STC  *infoPtr,
    IN GT_U32                       allowPrint
);
static void skernelPortLinkSet
(
    IN SKERNEL_DEVICE_OBJECT        *devObjPtr,
    IN GT_U32                       portNum,
    IN GT_U32                       link,
    IN GT_BOOL                      allowSlan,
    IN GT_U32                       allowPrint
);
/*DB of pointers to save the device ports info before soft reset to be able to reconstruct after soft reset */
static SKERNEL_PORT_INFO_STC   *beforeResetPortsArr[SMAIN_MAX_NUM_OF_DEVICES_CNS];
/**
* @internal skernelDeviceSoftResetGeneric_otherPortsSaveAndReset function
* @endinternal
*
* @brief   function to :
*       1. save 'other ports' (non-slan connections).
*       2. disconnect those ports
* @param[in] oldDeviceObjPtr   - the device object
*
*/
void skernelDeviceSoftResetGeneric_otherPortsSaveAndReset(
    IN SKERNEL_DEVICE_OBJECT * oldDeviceObjPtr
)
{
    GT_U32  portNum;
    WM_PORT_CONNECTION_INFO_STC  info;

    memset(&info,0,sizeof(info));
    info.connectionType = WM_PORT_CONNECTION_TYPE_NONE_E;
    info.allowSlan = GT_FALSE;

    beforeResetPortsArr[oldDeviceObjPtr->deviceId] = NULL;

    for(portNum = 0 ; portNum < oldDeviceObjPtr->portsNumber ; portNum++)
    {
        if(oldDeviceObjPtr->portsArr[portNum].state == SKERNEL_PORT_STATE_NOT_EXISTS_E ||
           oldDeviceObjPtr->portsArr[portNum].usedByOtherConnection == 0)
        {
            /* not a port that hold other than slan connection            */
            /* so not need to restore it's info after the soft/hard reset */
            continue;
        }

        if(!beforeResetPortsArr[oldDeviceObjPtr->deviceId]) /* allocated only if needed */
        {
            beforeResetPortsArr[oldDeviceObjPtr->deviceId] = calloc(oldDeviceObjPtr->portsNumber,sizeof(SKERNEL_PORT_INFO_STC));
            if(!beforeResetPortsArr[oldDeviceObjPtr->deviceId])
            {
                skernelFatalError("failed to calloc memory for 'beforeResetPortsArr' \n");
                return;
            }
        }

        /*save the info*/
        beforeResetPortsArr[oldDeviceObjPtr->deviceId][portNum]=oldDeviceObjPtr->portsArr[portNum];

        /* disconnect connections */
        internal_skernelPortConnectionInfo(oldDeviceObjPtr,portNum,&info,0/* NOT allowPrint */);
    }

    return;
}
/**
* @internal skernelDeviceSoftResetGeneric_otherPortsRestore function
* @endinternal
*
* @brief   function to :
*       1. restore 'other ports' (non-slan connections). as were before the soft reset.
*
* @param[in] newDeviceObjPtr   - the device object
*
*/
void skernelDeviceSoftResetGenericPart2_otherPortsRestore(
    IN SKERNEL_DEVICE_OBJECT * newDeviceObjPtr
)
{
    GT_U32  portNum;

    for(portNum = 0 ; portNum < newDeviceObjPtr->portsNumber ; portNum++)
    {
        if(beforeResetPortsArr[newDeviceObjPtr->deviceId][portNum].state == SKERNEL_PORT_STATE_NOT_EXISTS_E ||
           beforeResetPortsArr[newDeviceObjPtr->deviceId][portNum].usedByOtherConnection == 0)
        {
            /* not a port that hold other than slan connection            */
            /* so not need to restore it's info after the soft/hard reset */
            continue;
        }

        skernelPortLinkSet(newDeviceObjPtr,portNum,1/*link-Up*/,
            beforeResetPortsArr[newDeviceObjPtr->deviceId][portNum].forbidSlan ? GT_FALSE : GT_TRUE,/*allowSlan*/
            0/*allowPrint*/);

        /*  '...usedByOtherConnection = 1' already set in skernelPortLinkSet(...)
            newDeviceObjPtr->portsArr[portNum].usedByOtherConnection = 1;
        */
        newDeviceObjPtr->portsArr[portNum].peerInfo = beforeResetPortsArr[newDeviceObjPtr->deviceId][portNum].peerInfo;
    }

    /*we are done with this calloc ... so free it */
    free(beforeResetPortsArr[newDeviceObjPtr->deviceId]);
    beforeResetPortsArr[newDeviceObjPtr->deviceId] = NULL;
}

static void skernalDeviceCopyMemoryBetweenUnits
(
    IN SMEM_UNIT_CHUNKS_STC    *origUnitChunkPtr,
    OUT SMEM_UNIT_CHUNKS_STC   *dupUnitChunkPtr
)
{
    GT_U32  ii;
    SMEM_CHUNK_STC  *dupChunkPtr,*origChunkPtr;

    dupChunkPtr  = &dupUnitChunkPtr->chunksArray[0];
    origChunkPtr = &origUnitChunkPtr->chunksArray[0];

    for(ii = 0 ; ii < dupUnitChunkPtr->numOfChunks ; ii++,dupChunkPtr++,origChunkPtr++)
    {
        memcpy(dupChunkPtr->memPtr, origChunkPtr->memPtr , dupChunkPtr->memSize * sizeof(SMEM_REGISTER));
    }

    return;
}

/*******************************************************************************
*   skernelDeviceSoftResetGeneric
*
* DESCRIPTION:
*       do generic soft reset.
*       Function do soft reset on the following order:
*           - force link of all active SLANs down
*           - disable device interrupts
*           - unbind SCIB interface
*           - create new device to replace old one
*             the new device will keep the threads of the old one + queues + pools
*             the create of new device will reopen the SLANs , interrupt line , SCIB bind...
*           - delete old device object (with all it's memories)
*
* INPUTS:
*       oldDeviceObjPtr - pointer to the old device
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       pointer to the new allocated device.
* COMMENTS:
*       called in context of skernel task
*
*       see function smemLion3ActiveWriteDfxServerResetControlReg that triggers
*       soft reset
*******************************************************************************/
SKERNEL_DEVICE_OBJECT * skernelDeviceSoftResetGeneric
(
    IN SKERNEL_DEVICE_OBJECT * oldDeviceObjPtr
)
{
    DECLARE_FUNC_NAME(skernelDeviceSoftResetGeneric);

    SKERNEL_DEVICE_OBJECT *devObjPtr = oldDeviceObjPtr;/*needed ONLY for __LOG macro */

    SKERNEL_DEVICE_OBJECT * newDeviceObjPtr;
    SKERNEL_DEVICE_OBJECT * retDeviceObjPtr;
    SKERNEL_DEVICE_OBJECT * oldMainDeviceObjPtr;
    SKERNEL_DEVICE_OBJECT * iterDeviceObjPtr;
    SBUF_BUF_ID             bufferId;
    GT_U32                  devId;
    GT_BOOL                 multiCore;
    GT_U32                  ii;
    GT_U32                  waitTasks;
    GT_U32                  regValue;
    GT_U32                  softResetExcludePci = 0;
    GT_U32                  barsArr[4]={0,0,0,0};
    GT_U32                  *memPtr;
    SCIB_BIND_EXT_INFO_STC  bindExtInfo;
    GT_U32                  bar0_size = 0;
    GT_U32                  bar2_size = 0;
    GT_U32                  unitIndex;
    SMEM_CHT_GENERIC_DEV_MEM_INFO *oldDevMemInfoPtr;
    SMEM_CHT_GENERIC_DEV_MEM_INFO *newDevMemInfoPtr;

    multiCore = (oldDeviceObjPtr->portGroupSharedDevObjPtr) ? GT_TRUE: GT_FALSE;
    oldMainDeviceObjPtr = multiCore ? oldDeviceObjPtr->portGroupSharedDevObjPtr:
        oldDeviceObjPtr;
    devId = multiCore ? oldMainDeviceObjPtr->deviceId :
        oldDeviceObjPtr->deviceId;

    __LOG(("--'soft reset' -- part 1-- started \n"));
    simulationPrintf("--'soft reset' -- part 1-- started \n");

    oldDevMemInfoPtr = oldMainDeviceObjPtr->deviceMemory;

    if(SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.DFXServerUnitsBC2SpecificRegs.
        PCIeSkipInitializationMatrix != SMAIN_NOT_VALID_CNS)
    {
        smemDfxRegFldGet(devObjPtr,
            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.DFXServerUnitsBC2SpecificRegs.
                PCIeSkipInitializationMatrix,
            8,1,&regValue); /* value 0 - do reset , value 1 - don't reset */

        if(regValue)
        {
            __LOG(("'soft reset' without resetting the PCIe (restore will be done at the end) \n"));
            simulationPrintf("'soft reset' without resetting the PCIe (restore will be done at the end) \n");

            /* save the BARs in the PCIe config space */
            memPtr  = smemDevFindInUnitChunk(oldMainDeviceObjPtr, SKERNEL_MEMORY_READ_BAR0_E,
                0x10, 1,(GT_UINTPTR)(void*)(&oldDevMemInfoPtr->BAR0_UnitMem));
            barsArr[0] = memPtr[0] & ~0xF;
            barsArr[1] = memPtr[1];
            if(((memPtr[0] >> 2) & 0x3) != 0x1)/*NOT 64 bits support*/
            {
                /*NOT 64 bits support*/
                barsArr[1] = memPtr[1]  & ~0xF;
            }
            barsArr[2] = memPtr[2] & ~0xF;
            barsArr[3] = memPtr[3];
            if(((memPtr[2] >> 2) & 0x3) != 0x1)
            {
                /*NOT 64 bits support*/
                barsArr[3] = memPtr[3]  & ~0xF;
            }
            simulationPrintf("saved BAR0 [0x%8.8x] BAR1[0x%8.8x] \n",barsArr[0],barsArr[1]);
            simulationPrintf("saved BAR2 [0x%8.8x] BAR3[0x%8.8x] \n",barsArr[2],barsArr[3]);

            softResetExcludePci = 1;

            bar0_size = oldMainDeviceObjPtr->bar0_size;
            bar2_size = oldMainDeviceObjPtr->bar2_size;

            /*reset content of WM_DEVICE_READY_FOR_ACCESS_BY_CPU_REG_ADDR where
              we give indication that the WM device is ready for access by the CPU */
            memPtr  = smemDevFindInUnitChunk(oldMainDeviceObjPtr, SKERNEL_MEMORY_READ_BAR0_E,
                WM_DEVICE_READY_FOR_ACCESS_BY_CPU_REG_ADDR, 1,(GT_UINTPTR)(void*)(&oldDevMemInfoPtr->BAR0_UnitMem));
            *memPtr = 0;
        }
        else
        {
            __LOG(("'soft reset' with resetting the PCIe \n"));
            simulationPrintf("'soft reset' with resetting the PCIe \n");

            softResetExcludePci = 0;
        }
    }

    __LOG(("Force link DOWN of all active SLANs \n"));

    /* unbind all SLANs */
    skernelDeviceAllSlansUnbind(oldMainDeviceObjPtr);
    if(multiCore)
    {
        for(ii = 0; ii < oldMainDeviceObjPtr->numOfCoreDevs; ii++)
        {
            skernelDeviceAllSlansUnbind
                (oldMainDeviceObjPtr->coreDevInfoPtr[ii].devObjPtr);
        }
    }

    __LOG(("Force link DOWN of all active 'other ports' (non-SLANs) \n"));
    skernelDeviceSoftResetGeneric_otherPortsSaveAndReset(oldMainDeviceObjPtr);

#ifdef _WIN32 /* cause Linux to suspend 'current thread' !!! --> 'dead lock' */
    __LOG(("disconnect the old device from the interrupt line because the new device \n"
       "will bind to it (we not want the SHOST to fatal error) \n"));
    /* disconnect the old device from the interrupt line because the new device
       will bind to it (we not want the SHOST to fatal error) */
    if(oldMainDeviceObjPtr->interruptLine != SCIB_INTERRUPT_LINE_NOT_USED_CNS)
    {
        SHOSTG_interrupt_disable_one(WA_SHOST_NUMBERING(oldMainDeviceObjPtr->interruptLine));
    }
#endif /*_WIN32*/


    /* state that the device is in the middle of 'soft reset'
        do init sequence ..  this will
        1. bind interrupt with CSIB
        2. set init done
        3. smemChtInitStateSet will state that the reset done
    */

    if(oldDeviceObjPtr->PHA_FW_support.pha_memoryMap)
    {
        __LOG(("disconnect the old device PHA memory \n"));
        pha_fw_softReset(oldDeviceObjPtr);
    }

    __LOG(("allocate new device object \n"));
    /* allocate new device object */
    newDeviceObjPtr = smemDeviceObjMemoryAlloc(NULL,1, sizeof(SKERNEL_DEVICE_OBJECT));

    /* state that the new device is in process of replacing the old device */
    __LOG(("state that the new device is in process of replacing the old device \n"));
    newDeviceObjPtr->softResetOldDevicePtr = oldMainDeviceObjPtr;

    /* after the new device exists and point to the old device need to set the
       old device to point to the new device */
    oldMainDeviceObjPtr->softResetNewDevicePtr = newDeviceObjPtr;

    /* the 'new' device is replacing an old device , but it must be with it's
       bufPool,queueId before 'bind' it to the 'global array of devices'
       MUST be done before calling skernelInitDevicePart1(...)
    */
    __LOG(("Bind bufPool,queueId of 'old device' to 'new device' \n"));
    newDeviceObjPtr->bufPool = oldMainDeviceObjPtr->bufPool;
    newDeviceObjPtr->queueId = oldMainDeviceObjPtr->queueId;

    /* reuse also the TM queue */
    newDeviceObjPtr->tmInfo.bufPool_DDR3_TM = oldMainDeviceObjPtr->tmInfo.bufPool_DDR3_TM;

    newDeviceObjPtr->task_skernelCookieInfo.generic =
        oldMainDeviceObjPtr->task_skernelCookieInfo.generic;

    /* protect static variables/arrays used inside those init functions.
       from init of 2 devices during soft reset of the 2 devices in parallel */
    SCIB_SEM_TAKE;
    /* do initialization for the new device .. part1 , part2 */
    __LOG(("do initialization for the new device .. part1 \n"));
    skernelInitDevicePart1(newDeviceObjPtr , devId);
    __LOG(("do initialization for the new device .. part2 \n"));
    skernelInitDevicePart2(newDeviceObjPtr , devId);
    SCIB_SEM_SIGNAL;

    newDevMemInfoPtr = newDeviceObjPtr->deviceMemory;

    if(softResetExcludePci == 1)
    {
        /* The PCIe should have not gone reset.
           we need to restore it with it's values */
        /*set bars 0,2,3,4,5,6 */

        __LOG(("PCIe config space : restore BAR addresses (since 'PCIe' should have not done reset) \n"));

        if(SMAIN_NOT_VALID_CNS != UNIT_BASE_ADDR_GET_ALLOW_NON_EXIST_UNIT_MAC(newDeviceObjPtr,UNIT_CNM_PEX_MAC))
        {
            unitIndex = UNIT_INDEX_FROM_ENUM_GET_MAC(newDeviceObjPtr,UNIT_CNM_PEX_MAC);

            /* copy the content of the registers between the units */
            skernalDeviceCopyMemoryBetweenUnits(
                &oldDevMemInfoPtr->unitMemArr[unitIndex],
                &newDevMemInfoPtr->unitMemArr[unitIndex]);
        }
        /* BAR0 is 'start' of PCIe-MAC ... need to be copied separately */
        {
            skernalDeviceCopyMemoryBetweenUnits(
                &oldDevMemInfoPtr->BAR0_UnitMem,
                &newDevMemInfoPtr->BAR0_UnitMem);

            /* check the restored BARs in the PCIe config space */
            memPtr  = smemDevFindInUnitChunk(newDeviceObjPtr, SKERNEL_MEMORY_READ_BAR0_E,
                0x10, 1,(GT_UINTPTR)(void*)&newDevMemInfoPtr->BAR0_UnitMem);
            simulationPrintf("restored BAR0 [0x%8.8x] BAR1[0x%8.8x] \n",memPtr[0],memPtr[1]);
            simulationPrintf("restored BAR2 [0x%8.8x] BAR3[0x%8.8x] \n",memPtr[2],memPtr[3]);

        }

        memset(&bindExtInfo,0,sizeof(bindExtInfo));

        newDeviceObjPtr->bar0_size   = bar0_size;
        newDeviceObjPtr->bar2_size   = bar2_size;

        bindExtInfo.update_bar0_size = GT_TRUE;
        bindExtInfo.bar0_size        = bar0_size;

        bindExtInfo.update_bar2_size = GT_TRUE;
        bindExtInfo.bar2_size        = bar2_size;

        bindExtInfo.update_bar0_base = GT_TRUE;
        bindExtInfo.bar0_base        = barsArr[0];
        bindExtInfo.update_bar0_base_high = GT_TRUE;
        bindExtInfo.bar0_base_high   = barsArr[1];

        bindExtInfo.update_bar2_base = GT_TRUE;
        bindExtInfo.bar2_base        = barsArr[2];
        bindExtInfo.update_bar2_base_high = GT_TRUE;
        bindExtInfo.bar2_base_high   = barsArr[3];

        scibBindExt(devId,&bindExtInfo);
    }

    __LOG(("wait for all threads (other then skernel task) to replace old device with new device \n"));

    /* Send dummy messages to all cores in order to give them possibility to
     enter skernelSleep() */
    if(multiCore)
    {
        for(ii = 0; ii < oldMainDeviceObjPtr->numOfCoreDevs; ii++)
        {
            iterDeviceObjPtr = oldMainDeviceObjPtr->coreDevInfoPtr[ii].
                devObjPtr;
            /* Resume, suspended during reset bit set pool */
            sbufPoolResume(iterDeviceObjPtr->bufPool);
            bufferId = sbufAlloc(iterDeviceObjPtr->bufPool, sizeof(GT_U32));

            if ( bufferId == NULL )
            {
                skernelFatalError(" skernelDeviceSoftResetGeneric: cannot "
                    "allocate message buffer");
            }
            bufferId->srcType = SMAIN_SRC_TYPE_CPU_E;
            bufferId->dataType = SMAIN_MSG_TYPE_DUMMY;

            /* Resume, suspended during reset bit set queue */
            squeResume(iterDeviceObjPtr->queueId);
            squeBufPut(iterDeviceObjPtr->queueId, SIM_CAST_BUFF(bufferId));
        }
    }

    /* wait for all threads (other then skernel task) to replace old device
    with new device */

    if(multiCore)
    {
        do
        {
            waitTasks = oldMainDeviceObjPtr->numOfCoreDevs;
            for(ii = 0; ii < oldMainDeviceObjPtr->numOfCoreDevs; ii++)
            {
                if(oldMainDeviceObjPtr->coreDevInfoPtr[ii].devObjPtr->
                    numThreadsOnMe == 0)
                {
                    waitTasks--;
                }
            }

            /* numThreadsOnMe can not be '0' for current core runnoing current
            thread*/
            ii = oldDeviceObjPtr->portGroupId;
            if(oldMainDeviceObjPtr->coreDevInfoPtr[ii].devObjPtr->
                numThreadsOnMe == 1)
            {
                waitTasks--;
            }
            SIM_OS_MAC(simOsSleep)(50);
            simulationPrintf("+");
            __LOG(("+"));
        }
        while(waitTasks);
    }
    else
    {
        /* 1 is the skernel task (our current task) */
        while(oldMainDeviceObjPtr->numThreadsOnMe > 1)
        {
            SIM_OS_MAC(simOsSleep)(50);
            simulationPrintf("+");
            __LOG(("+"));
        }
    }
    simulationPrintf("\n");
    __LOG(("\n"));

    /* I point to new device instead of old device because I not want to set it
        NULL (that will cause other tasks to continue as usual) and I not want
        to keep pointer to device object that about to be free !!! */
    newDeviceObjPtr->softResetOldDevicePtr = newDeviceObjPtr;
    if(multiCore)
    {
        for(ii = 0; ii < oldMainDeviceObjPtr->numOfCoreDevs; ii++)
        {
            newDeviceObjPtr->coreDevInfoPtr[ii].devObjPtr->softResetOldDevicePtr
                = newDeviceObjPtr->coreDevInfoPtr[ii].devObjPtr;
        }
    }

    /* Rebind the device */
    if(multiCore)
    {
        for(ii = 0; ii < oldMainDeviceObjPtr->numOfCoreDevs; ii++)
        {
            scibReBindDevice(newDeviceObjPtr->coreDevInfoPtr[ii].
                devObjPtr->deviceId);
        }
    }
    else
    {
        scibReBindDevice(newDeviceObjPtr->deviceId);
    }

    /************************************************************************************/
    /* we are ready to delete the old device as no one should point to it at this stage */
    /************************************************************************************/
    devObjPtr = newDeviceObjPtr;/*needed ONLY for __LOG macro */

    /* Select descriptor to be retturn, it must correspond current core */
    if(multiCore)
    {
        retDeviceObjPtr = newDeviceObjPtr->coreDevInfoPtr
            [oldDeviceObjPtr->portGroupId].devObjPtr;
    }
    else
    {
        retDeviceObjPtr = newDeviceObjPtr;
    }

    /* register the thread on info on DB in the new device */
    SIM_OS_MAC(simOsTaskOwnTaskPurposeSet)(SIM_OS_TASK_PURPOSE_TYPE_PP_PIPE_SOFT_RESET_E,
        &newDeviceObjPtr->task_skernelCookieInfo.generic);
    simLogPacketFrameUnitSet(SIM_LOG_FRAME_UNIT___ALLOW_ALL_UNITS___E);/* wild card for LOG without the unit filter */
    simLogPacketFrameCommandSet(SIM_LOG_FRAME_COMMAND_TYPE_GENERAL_E);

    /* (for debug only) print amount of memory used by the old device */
    smemDeviceObjMemoryAllAllocationsSum(oldMainDeviceObjPtr);

    __LOG(("free all the memory that the device used including it's own pointer !!!! \n"));

    /* free all the memory that the device used including it's own pointer !!!! */
    if(multiCore)
    {
        for(ii = 0; ii < oldMainDeviceObjPtr->numOfCoreDevs; ii++)
        {
            smemDeviceObjMemoryAllAllocationsFree(
                oldMainDeviceObjPtr->coreDevInfoPtr[ii].devObjPtr, GT_TRUE);
        }
    }
    smemDeviceObjMemoryAllAllocationsFree(oldMainDeviceObjPtr,GT_TRUE);

    __LOG(("the device pointer is no longer in use !!! \n"));
    /* the device pointer is no longer in use !!! */
    oldDeviceObjPtr = NULL;

    __LOG(("The old king is dead long live the new king !!!! \n"));
    /****************************************************/
    /* The old king is dead long live the new king !!!! */
    /****************************************************/

    /* function skernelDeviceSoftResetGenericPart2(...)
       will handle the pool and the queue and will set
       newDeviceObjPtr->softResetOldDevicePtr = NULL */

    __LOG(("--'soft reset' -- part 1-- ended \n"));
    simulationPrintf("'soft reset' -- part 1-- ended\n");

    return retDeviceObjPtr;
}

/*******************************************************************************
*   skernelDeviceSoftResetGenericPart2
*
* DESCRIPTION:
*       part 2 of do generic soft reset.
*       resume pool and queue of the device to empty state.
*       and state newDeviceObjPtr->softResetOldDevicePtr = NULL
*
* INPUTS:
*       newDeviceObjPtr - pointer to the new device !!!
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       !!! NULL !!!
* COMMENTS:
*       called in context of skernel task
*
*       this is part 2 of skernelDeviceSoftResetGeneric
*******************************************************************************/
SKERNEL_DEVICE_OBJECT * skernelDeviceSoftResetGenericPart2
(
    IN SKERNEL_DEVICE_OBJECT * newDeviceObjPtr
)
{
    DECLARE_FUNC_NAME(skernelDeviceSoftResetGenericPart2);

    SKERNEL_DEVICE_OBJECT *devObjPtr = newDeviceObjPtr;/*needed ONLY for __LOG macro */
    SKERNEL_DEVICE_OBJECT *shellDevObjPtr;
    SKERNEL_DEVICE_OBJECT *iterDevObjPtr;
    GT_U32 ii;

    __LOG(("--'soft reset' -- part 2-- started \n"));

    if(beforeResetPortsArr[newDeviceObjPtr->deviceId])
    {
        __LOG(("restore non-slans connections \n"));
        skernelDeviceSoftResetGenericPart2_otherPortsRestore(newDeviceObjPtr);
    }

    shellDevObjPtr = (newDeviceObjPtr->portGroupSharedDevObjPtr) ?
        newDeviceObjPtr->portGroupSharedDevObjPtr : NULL;

    /*
       flush any buffers that were allocated but not free in the pool .
       like by the 'TXQ queue disable' that the descriptor is kept with it's
       buffer and not calling sbufFree(...) for it.

       NOTE: the queue may still hold 'old' messages even that the pool was suspended
       because buffers that 'allocated' before we suspended the pool can still
       be after the buffer of 'soft drop' (buffer that we handle at this moment)

       so first empty the the queue and then reset the content of the pool.
    */

    /* The same for cores of multicore devices */
    if(shellDevObjPtr)
    {
        for(ii = 0; ii < shellDevObjPtr->numOfCoreDevs; ii++)
        {
            iterDevObjPtr = shellDevObjPtr->coreDevInfoPtr[ii].devObjPtr;
            squeResume(iterDevObjPtr->queueId);
            sbufPoolResume(iterDevObjPtr->bufPool);
            iterDevObjPtr->softResetOldDevicePtr = NULL;
        }
    }
    else
    {
        __LOG(("flush the queue of the device \n"));
        squeFlush(newDeviceObjPtr->queueId);

        __LOG(("flush the pool of the device \n"));
        sbufPoolFlush(newDeviceObjPtr->bufPool);

        /* Allow queue of buffers to resume (it was suspended by the
         'old device') but the queue is delivered to the new device */
        __LOG(("resume the queue of the device \n"));
        squeResume(newDeviceObjPtr->queueId);

        /* Allow buffers pool to resume (it was suspended by the 'old device')
           but the pool is delivered to the new device */
        __LOG(("resume the pool of the device \n"));
        sbufPoolResume(newDeviceObjPtr->bufPool);

        /* (only after we handled the pool,queue) now we are ready to state
           the other tasks to continue with the new device. see function
           skernelSleep(...) that kept them without continue !*/
        __LOG(("now we are ready to state the other tasks to continue"
               " with the new device. \n"));
        newDeviceObjPtr->softResetOldDevicePtr = NULL;
    }

    __LOG(("--'soft reset' -- part 2-- ended \n"));
    simulationPrintf("'soft reset' -- part 2-- ended \n");

    /* add indication to the PCI config space at reserved register that the reset was done
       see in cpssSimSoftResetDoneWait that wait for it */
    smainDeviceReadyForAccessFromCpu(newDeviceObjPtr);


    return NULL; /* must be ignored by the caller */
}


/**
* @internal skernelInit function
* @endinternal
*
* @brief   Init SKernel library.
*/
void skernelInit
(
    void
)
{
    char                    param_str[FILE_MAX_LINE_LENGTH_CNS]; /* string for parameter */
    GT_U32                  param_val; /* parameter value */
    GT_U32                  devId; /* device id loop */
    SKERNEL_DEVICE_OBJECT * deviceObjPtr;  /* pointer to the device object*/
    GT_U32                  jj;
    GT_U32                  timeStart,timeEnd;

    /* state that this task is the 'initialization' for the simulation logger */
    SIM_OS_MAC(simOsTaskOwnTaskPurposeSet)(SIM_OS_TASK_PURPOSE_TYPE_INITIALIZATION_E,NULL);

    timeStart = SIM_OS_MAC(simOsTickGet)();

    /* bind generic callback to handle 'packet done' messages from the TM */
    sRemoteTmBindCallBack(&smainReceivedPacketDoneFromTm);

    /* create mutex for LOG to be synch between all devices/cores */
    LOG_fullPacketWalkThroughProtectMutex = SIM_OS_MAC(simOsMutexCreate)();
    if (LOG_fullPacketWalkThroughProtectMutex == (GT_MUTEX)0)
    {
        skernelFatalError(" skernelInit: cannot create mutex");
        return;
    }

    /* create semaphore for 'wait' for threads to start process */
    slanDownSemaphore = SIM_OS_MAC(simOsSemCreate)(1,1);
    if (slanDownSemaphore == (GT_SEM)0)
    {
        skernelFatalError(" skernelInit: cannot create semaphore");
        return;
    }

    /* reset user debug structure */
    memset(&skernelUserDebugInfo, 0, sizeof(skernelUserDebugInfo));

    /* Lookup policer performance level */
    if(SIM_OS_MAC(simOsGetCnfValue)(DEBUG_SECTION_CNS, "policer_conformance_level",
                       FILE_MAX_LINE_LENGTH_CNS, param_str))
    {
        sscanf(param_str, "%u", &param_val);
        skernelUserDebugInfo.policerConformanceLevel = param_val;
    }

    /* check if general printing allowed */
    if(SIM_OS_MAC(simOsGetCnfValue)(DEBUG_SECTION_CNS, "print_general_allowed",
                       FILE_MAX_LINE_LENGTH_CNS, param_str))
    {
        sscanf(param_str, "%u", &param_val);
        skernelUserDebugInfo.printGeneralAllowed = param_val ? GT_TRUE : GT_FALSE;
    }
    else
    {
        /* by default the general printing is NOT allowed */
        skernelUserDebugInfo.printGeneralAllowed = GT_FALSE;
    }

    /* check if general printing allowed */
    if(SIM_OS_MAC(simOsGetCnfValue)(DEBUG_SECTION_CNS, "print_warning_allowed",
                       FILE_MAX_LINE_LENGTH_CNS, param_str))
    {
        sscanf(param_str, "%u", &param_val);
        skernelUserDebugInfo.printWarningAllowed = param_val ? GT_TRUE : GT_FALSE;
    }
    else
    {
        /* by default the warning printing is allowed */
        skernelUserDebugInfo.printWarningAllowed = GT_TRUE;
    }

    /* clear device objects table */
    memset (smainDeviceObjects, 0, sizeof (smainDeviceObjects));

    /* reset NIC callback and object */
    smainNicRxCb = NULL;
    smainNicDeviceObjPtr = NULL;

    /* get to see if visual asic should be enabled */
    if(SIM_OS_MAC(simOsGetCnfValue)("visualizer",  "enable",
                             FILE_MAX_LINE_LENGTH_CNS, param_str))
    {
        GT_U32 tmp;
        sscanf(param_str, "%u", &tmp);
        smainVisualDisabled = tmp ? 0 : 1;
    }
    /* get disable visual asic */
    else if(SIM_OS_MAC(simOsGetCnfValue)("visualizer",  "disable",
                             FILE_MAX_LINE_LENGTH_CNS, param_str))
    {
        GT_U32 tmp;
        sscanf(param_str, "%u", &tmp);
        smainVisualDisabled = tmp ? 1 : 0;
    }
    else
    {
         /* set visualizer to be disabled by default*/
        smainVisualDisabled = 1;
    }

    lastDeviceIdUsedForCores = smainDevicesNumber;

    /* init all devices */
    for (devId = 0; devId < smainDevicesNumber; devId++)
    {
        /*skip devices that are not on this board section*/
        if(sinitOwnBoardSection.numDevices)
        {
            for(jj=0;jj<sinitOwnBoardSection.numDevices;jj++)
            {
                if(sinitOwnBoardSection.devicesIdArray[jj] == devId)
                {
                    break;
                }
            }

            if(jj == sinitOwnBoardSection.numDevices)
            {
                /* this device is not on current board section */
                continue;
            }
        }

        /* allocate device object */
        deviceObjPtr = smemDeviceObjMemoryAlloc(NULL,1, sizeof(SKERNEL_DEVICE_OBJECT));
        if (deviceObjPtr == NULL)
        {
            skernelFatalError(" skernelInit: cannot allocate device %u", devId);
        }

        /* do part 1 init */
        skernelInitDevicePart1(deviceObjPtr,devId);
    }/* portGroupId */

    /* NOTE: this code need to run only after all the 'device objects' already
            exists */
    /* initialize UPLINK information */
    /* initialize internal_connection information */
    for (devId = 0; devId < smainDevicesNumber; devId++)
    {
        if (!smainDeviceObjects[devId])
           continue;

       deviceObjPtr = smainDeviceObjects[devId]   ;

        /* do part 2 init */
        skernelInitDevicePart2(deviceObjPtr,devId);

        /* add indication to the PCI config space at reserved register that the reset was done
           see in cpssSimSoftResetDoneWait that wait for it */
        smainDeviceReadyForAccessFromCpu(deviceObjPtr);

    }/* devId loop */


    /* send to the application side the minimal info needed about the devices */
    if(sasicgSimulationRole != SASICG_SIMULATION_ROLE_NON_DISTRIBUTED_E)
    {
        skernelInitDistributedDevParse();
    }

    timeEnd = SIM_OS_MAC(simOsTickGet)();

    simulationPrintf("skernelInit : start tick[%d] , end tick[%d] --> total ticks[%d] \n",
        timeStart,timeEnd,timeEnd-timeStart);

    return;
}

/**
* @internal skernelShutDown function
* @endinternal
*
* @brief   Shut down SKernel library before reboot.
*
* @note The function unbinds ports from SLANs
*
*/
void skernelShutDown
(
    void
)
{
    GT_U32                                    device; /* device index */
    GT_U32                                      port;        /* port index */
    SMAIN_PORT_SLAN_INFO * slanInfoPrt;          /* slan port entry pointer */

    skernelUnderShutDown = GT_TRUE;

    printf("skernelShutDown : KILL WM ... \n");

    for (device = 0; device < SMAIN_MAX_NUM_OF_DEVICES_CNS; device++)
    {
        if (!smainDeviceObjects[device])
           continue;

        if (!smainDeviceObjects[device]->portSlanInfo)
        {
            /* early stage on initialization ... not allocated yet */
            continue;
        }

        {
            /* when 'killing' simulation under traffic we get 'exception' in the 'slan code' */
            printf("skernelShutDown : device[%d] detach SLANs from traffic ... \n",device);

            /* make sure that all slans will wait for changing the name 'to indicate' unbind */
            SIM_OS_MAC(simOsSemWait)(slanDownSemaphore,SIM_OS_WAIT_FOREVER);
            for (port = 0; port < smainDeviceObjects[device]->numSlans; port++)
            {
                /* get slan info */
                slanInfoPrt = &(smainDeviceObjects[device]->portSlanInfo[port]);
                slanInfoPrt->slanName[0] = 0;/* before the unbind , make sure no one try to send packet to this port */
            }
            SIM_OS_MAC(simOsSemSignal)(slanDownSemaphore);

            /* allow all other tasks that send traffic to not be inside the SLAN that going to be unbound */
            SIM_OS_MAC(simOsSleep)(1);
        }

        printf("skernelShutDown : device[%d] unbind from SLANs  ... \n",device);

        for (port = 0; port < smainDeviceObjects[device]->numSlans; port++)
        {
            /* get slan info */
            slanInfoPrt = &(smainDeviceObjects[device]->portSlanInfo[port]);

            /* unbind the port */
            internalSlanUnbind(slanInfoPrt);
        }
    }

#if (defined LINUX && defined ASIC_SIMULATION)
    SIM_OS_MAC(simOsSlanClose)();
#endif

    printf("skernelShutDown : close WM LOG ... \n");

    simLogClose();

#if defined LINUX
    printf("skernelShutDown : kill WM tasks ... \n");

    /* fix WM-ASIM JIRA : IPBUSW-8121 : ASIM-106xx switch: segmentation fault noticed on the ASIM side on quit */
    SIM_OS_MAC(simOsTaskDelete)((GT_TASK_HANDLE)(0xdeadbeef));
#endif

}

/**
* @internal smainFrame2InternalConnectionSend function
* @endinternal
*
* @brief   Send frame to a correspondent internal connection of a port.
*/
static void smainFrame2InternalConnectionSend
(
    IN SKERNEL_DEVICE_OBJECT * deviceObjPtr,
    IN GT_U32                  portNum,
    IN GT_U8    *              dataPrt,
    IN GT_U32                  dataSize
)
{
    DECLARE_FUNC_NAME(smainFrame2InternalConnectionSend);

    SKERNEL_DEVICE_OBJECT           *destDeviceObjPtr;/* pointer to object of the destination device */
    SBUF_BUF_ID                     bufferId; /* buffer id */
    SBUF_BUF_STC                    *dstBuf_PTR; /* pointer to the allocated buffer from the destination device pool*/
    SKERNEL_INTERNAL_CONNECTION_INFO_STC    *peerInfoPtr;/* info about the peer (destination) device */
    SKERNEL_DEVICE_OBJECT * devObjPtr = deviceObjPtr;/* for __LOG purpose */

    peerInfoPtr = &deviceObjPtr->portsArr[portNum].peerInfo;

    destDeviceObjPtr = smemTestDeviceIdToDevPtrConvert(peerInfoPtr->peerDeviceId);

    __LOG(("Send packet from local {device[%d] port [%d] (deviceName[%s])} to 'internal connection' to {device[%d] port [%d] (deviceName[%s])}\n",
        deviceObjPtr->deviceId,
        portNum,
        deviceObjPtr->deviceName,
        peerInfoPtr->peerDeviceId,
        peerInfoPtr->peerPortNum,
        destDeviceObjPtr->deviceName));

    if(destDeviceObjPtr->deviceType == SKERNEL_NIC)
    {
        /* call the NIC from this context */
        /* we keep using the 'unique' SLAN for the CPU because the MTS need to
           get packets from 'registered' thread. but we don't want to register
           the 'Asic simulation' tasks */

        if (destDeviceObjPtr->crcPortsBytesAdd == 0)
        {
            dataSize += 4;
        }

        smainNicRxHandler(dataPrt, dataSize);
        return;
    }

    /* allocate buffer from the 'destination' device pool */
    /* get the buffer and put it in the queue */
    bufferId = sbufAlloc(destDeviceObjPtr->bufPool, dataSize);
    if (bufferId == NULL)
    {
        simWarningPrintf(" smainFrame2InternalConnectionSend : no buffers for process \n");
        return ;
    }

    skernelNumOfPacketsInTheSystemSet(GT_TRUE);

    dstBuf_PTR = (SBUF_BUF_STC *) bufferId;
    /* use the SLAN type since this is generic packet */
    dstBuf_PTR->srcType = SMAIN_SRC_TYPE_INTERNAL_CONNECTION_E;
    /* set source port of buffer -- the port on the destination device */
    dstBuf_PTR->srcData = peerInfoPtr->peerPortNum;
    /* state 'regular' frame */
    dstBuf_PTR->dataType = SMAIN_MSG_TYPE_FRAME_E;

    if(SMEM_CHT_IS_SIP6_10_GET(deviceObjPtr))
    {
        /* allow the 'other side' to know "is packet out on 'preemptive' MAC ?"
           so ingress can automatically knows to use  preemptive MAC or Express MAC */
        dstBuf_PTR->isPreemptiveChannel = smemGetCurrentTaskExtParamValue(deviceObjPtr,TASK_EXT_PARAM_INDEX_IS_PREEMPTIVE_CHANNEL);
    }

    /* copy the frame into the buffer */
    memcpy(dstBuf_PTR->actualDataPtr,dataPrt,dataSize);

    /* set the pointers and buffer */
    sbufDataSet(bufferId, dstBuf_PTR->actualDataPtr,dataSize);

    /* put buffer on the queue of the destination device */
    squeBufPut(destDeviceObjPtr->queueId ,SIM_CAST_BUFF(bufferId));

    return ;
}

/**
* @internal smainFrame2PortSend function
* @endinternal
*
* @brief   Send frame to a correspondent SLAN of a port.
*
* @note if doLoopback is enabled , then the packet is sent to the portNum
*
*/
void smainFrame2PortSend
(
    IN SKERNEL_DEVICE_OBJECT * deviceObjPtr,
    IN GT_U32                  portNum,
    IN GT_U8    *              dataPrt,
    IN GT_U32                  dataSize  ,
    IN GT_BOOL                 doLoopback
)
{
    smainFrame2PortSendWithSrcPortGroup(deviceObjPtr,portNum,dataPrt,
            dataSize,doLoopback,
            deviceObjPtr->portGroupId);
}

WM_STATUS wmTrafficEgressPacket(
    IN GT_U32   wmDeviceId,
    IN GT_U32   portNum,
    IN GT_U32   numOfBytes,
    IN char*    packetPtr /* pointer to start of packet (network order) according to 'numOfBytes' */
);

/**
* @internal smainFrame2PortSendWithSrcPortGroup function
* @endinternal
*
* @brief   Send frame to a correspondent SLAN of a port. (from context of src port group)
*
* @note if doLoopback is enabled , then the packet is sent to the portNum
*
*/
void smainFrame2PortSendWithSrcPortGroup
(
    IN SKERNEL_DEVICE_OBJECT * deviceObjPtr,
    IN GT_U32                  portNum,
    IN GT_U8    *              dataPrt,
    IN GT_U32                  dataSize  ,
    IN GT_BOOL                 doLoopback,
    IN GT_U32                  srcPortGroup
)
{
    DECLARE_FUNC_NAME(smainFrame2PortSendWithSrcPortGroup);

    SMAIN_PORT_SLAN_INFO *  slanInfoPrt; /* slan info entry pointer */
    SBUF_BUF_ID bufferId; /* buffer id */
    GT_U8*  tmpPtr;/* tmp pointer in new buffer */
    GT_U32  tmpLength; /* tmp length in new buffer */
    SKERNEL_PORT_INFO_STC   *portInfoPtr;
    GT_U32  retryCount = 50;/* retry up to 500 milliseconds */
    SKERNEL_DEVICE_OBJECT * devObjPtr = deviceObjPtr;/*devObjPtr -- needed by __LOG(()) */

    simLogPacketFrameUnitSet(SIM_LOG_FRAME_UNIT___ALLOW_ALL_UNITS___E);/* wild card for LOG without the unit filter */

    __LOG(("Final Egress packet \n"));
    /* dump to log the packet that egress the port*/
    simLogPacketDump(devObjPtr,
        GT_FALSE ,
        portNum,
        dataPrt,
        dataSize);

    /* check parameters */
    if (portNum >= deviceObjPtr->numSlans)
    {
        skernelFatalError("smainFrame2PortSendWithSrcPortGroup: wrong port %d \n", portNum);
    }

    /* remove 4 FCS (frame check sum or CRC) bytes frame to SLAN */
    if (deviceObjPtr->crcPortsBytesAdd == 1)
    {
        dataSize -=4;

        __LOG(("Egress port [%d] : remove 4 FCS (frame check sum or CRC) bytes frame to SLAN \n",
            portNum));
    }

    /* get SLAN info for port */
    slanInfoPrt = &(deviceObjPtr->portSlanInfo[portNum]);

    /* transmit to SLAN  */
    if (slanInfoPrt->slanName[0x0] != 0x0)/* support unbind of slan under traffic */
    {
        __LOG(("Egress port [%d] : send to SLAN[%s] \n",
            portNum,slanInfoPrt->slanName));
        SIM_OS_MAC(simOsSlanTransmit)(slanInfoPrt->slanIdTx,SIM_OS_SLAN_MSG_CODE_FRAME_CNS,
                          dataSize,  (char*)dataPrt);
    }
    else if(deviceObjPtr->portsArr[portNum].peerInfo.usePeerInfo == GT_TRUE)
    {
        __LOG(("Egress port [%d] : send to internal connection \n",
            portNum));
        smainFrame2InternalConnectionSend(deviceObjPtr,portNum,dataPrt,dataSize);
    }

    if(GT_TRUE  == deviceObjPtr->portsArr[portNum].usedByOtherConnection &&
       GT_FALSE == deviceObjPtr->portsArr[portNum].peerInfo.usePeerInfo)
    {
        /* allow ASIM environment to get packet regardless to SLANs , as in this environment we allow
            the port to have both types at the same time !
            because only one of them is expected to be 'connected' to the testing environment.
        */
        wmTrafficEgressPacket(deviceObjPtr->deviceId,portNum,dataSize,(char*)dataPrt);
    }

    /*
        allow to do loopback regardless to the sending to the SLAN/other connection.
        (this is actual behavior of the device)
    */

    portInfoPtr =  &deviceObjPtr->portsArr[portNum];

    /* loopback on portnum*/
    if (doLoopback == GT_TRUE || portInfoPtr->loopbackForceMode == SKERNEL_PORT_LOOPBACK_FORCE_ENABLE_E)
    {
        __LOG(("Do port loopback : Send the packet to this port [%d] queue \n",
            portNum));

        if(supportMacLoopbackInContextOfSender != 0 &&
           deviceObjPtr->gmDeviceType != SMAIN_NO_GM_DEVICE)
        {
            GT_BIT loopedInCurrentContext = 1;
            static GT_U32  packetStackDepth = 0;
            static GT_U32  lastTryEgressPort = 0;/* the last port that packet try to egress from */
            static GT_U32  lastTryEgressCore = 0;/* the last core that packet try egress from */
            static GT_U32  resendCounter = 0;
            GT_BIT resendCounterChanged = 0;
            GT_U32  pipeId = deviceObjPtr->portGroupId;
            GT_U32  localPort = portNum;

            packetStackDepth++;
            switch (deviceObjPtr->deviceFamily)
            {
                case SKERNEL_PUMA3_NETWORK_FABRIC_FAMILY:
                    if(portNum < 0x20 /*network port*/)
                    {
                        /* do not do it on fabric ports to avoid stack overflow due to recursive
                           loops*/

                        /* implement 'internal loopback' for the GM. don't sent the packet to other task */
                        GT_U32  srcPort = portNum;
                        pipeId = (srcPort / 16) + deviceObjPtr->portGroupId;
                        localPort = (srcPort % 16);

                        /* the Puma3 device has 4 pipes that handle packets but there
                           are only 2 MG units . so we have only devObjPtr->portGroupId = 0 and 1 */
                        /* but still we need to notify the GM that packet came from
                            pipe 0 or pipe 1 or pipe 2 or pipe 3 with local port 0..11 */
                        /* multi core device */
                    }
                    else
                    {
                        loopedInCurrentContext = 0;
                    }
                    break;
                default:
                    /* No need pipeId and localPort conversion for non Puma3 devices  */
                    break;
            }

            if(loopedInCurrentContext)
            {
                if(packetStackDepth > 1)
                {
                    if(lastTryEgressCore == deviceObjPtr->portGroupId &&
                       lastTryEgressPort == portNum)
                    {
                        /* we are before sending packet to port that is loopback , that already got traffic from itself */
                        resendCounter++;
                        resendCounterChanged = 1;
                    }

/* do not FATAL error , just break the loop.
    1. it will not kill the list of running tests !
    2. the tests should notice that the MAC counted more packets than expected.
*/
                    if(resendCounter > 2)
                    {
                        /*skernelFatalError*/
                        printf("smainFrame2PortSendWithSrcPortGroup : Simulation detect that egress port[%d] in core[%d] is mac-loopback"
                                          " and send traffic to itself (it may cause 'stack overflow') "
                                          "... forcing to break the loop !!! \n",
                            localPort,pipeId);
                    }

                }

                /* save the values that we try to send to */
                lastTryEgressCore = deviceObjPtr->portGroupId;
                lastTryEgressPort = portNum;

                if(packetStackDepth == 5)
                {
                    /* too deep call stack .. the 'loop to same port' check was not recognized
                       but the GM keep sending to itself from one port to second port and back from second port to first
                    */
                    /* 'break' the loop instead of fatal error */
                    printf("smainFrame2PortSendWithSrcPortGroup :portNum[%d] Simulation detect that too deep 'call stack' due to mac-loopback (depth[%d])"
                                      " and send traffic to itself (it may cause 'stack overflow') "
                                      "... forcing to break the loop !!! \n",
                        portNum,packetStackDepth);
                }
                else
                if(resendCounter <= 2)/* 'break' the loop instead of fatal error */
                {
                    ppSendPacket(SMEM_GM_GET_GM_DEVICE_ID(deviceObjPtr), pipeId , localPort, (char*)dataPrt, dataSize);
                }

                if(resendCounterChanged)
                {

                    if(resendCounter == 0)
                    {
                        skernelFatalError("smainFrame2PortSendWithSrcPortGroup: bad management of resendCounter \n");
                    }
                    resendCounter--;
                }
            }

            if(packetStackDepth == 0)
            {
                skernelFatalError("smainFrame2PortSendWithSrcPortGroup: bad management of packetStackDepth \n");
            }
            packetStackDepth--;

            if(loopedInCurrentContext)
            {
                return;
            }
        }

        if(dataSize < SGT_MIN_FRAME_LEN)
        {
            dataSize = SGT_MIN_FRAME_LEN;
        }

        dataSize += deviceObjPtr->prependNumBytes;
retryAlloc_lbl:
        /* get the buffer and put it in the queue */
        bufferId = sbufAllocWithProtectedAmount(deviceObjPtr->bufPool,dataSize,LOOPBACK_MIN_FREE_BUFFERS_CNS);

        if (bufferId == NULL)
        {
            if(deviceObjPtr->needToDoSoftReset)
            {
                retryCount = 0;
            }

            if(retryCount-- &&
               deviceObjPtr->portGroupSharedDevObjPtr &&
               srcPortGroup != deviceObjPtr->portGroupId)
            {
                /* do retry only for multi-core device because on single core device
                  the context of this task is the only one that can free those buffers
                  so no meaning to wait for it

                  in multi core the buffer that ask for can be from different core ,
                  so the skernel task of this core may free buffers.
                  */
                SIM_OS_MAC(simOsMutexUnlock)(deviceObjPtr->portGroupSharedDevObjPtr->protectEgressMutex);
                SIM_OS_MAC(simOsMutexUnlock)(LOG_fullPacketWalkThroughProtectMutex);

                SIM_OS_MAC(simOsSleep)(10);

                SIM_OS_MAC(simOsMutexLock)(LOG_fullPacketWalkThroughProtectMutex);
                SIM_OS_MAC(simOsMutexLock)(deviceObjPtr->portGroupSharedDevObjPtr->protectEgressMutex);
                goto retryAlloc_lbl;
            }

            __LOG(("Do port loopback : ERROR : Failed to Send the packet to this port [%d] queue \n",
                portNum));

            {
                static GT_U32 loopback_dropped_counter = 0;
                /* limit the printings */
                if(loopback_dropped_counter < 50 ||
                   (0 == (loopback_dropped_counter & 0x7ff)))
                {
                    simWarningPrintf(" smainFrame2PortSendWithSrcPortGroup : Do port loopback : ERROR : Failed to Send the packet to this port [%d] queue  \n",
                        portNum);
                }

                loopback_dropped_counter++;
            }

            return ;
        }

        skernelNumOfPacketsInTheSystemSet(GT_TRUE);

        bufferId->srcType = SMAIN_SRC_TYPE_LOOPBACK_PORT_E;
        bufferId->srcData = portNum;
        bufferId->dataType = SMAIN_MSG_TYPE_FRAME_E;
        if(SMEM_CHT_IS_SIP6_10_GET(deviceObjPtr))
        {
            /* allow the 'Rx side' to know "is packet out on 'preemptive' MAC ?"
               so ingress can automatically knows to use  preemptive MAC or Express MAC */
            bufferId->isPreemptiveChannel = smemGetCurrentTaskExtParamValue(deviceObjPtr,TASK_EXT_PARAM_INDEX_IS_PREEMPTIVE_CHANNEL);
        }

        /* get actual data pointer */
        sbufDataGet(bufferId, &tmpPtr, &tmpLength);

        tmpLength -= deviceObjPtr->prependNumBytes;
        tmpPtr    += deviceObjPtr->prependNumBytes;

        /* copy data to the new buffer , in the needed offset */
        memcpy(tmpPtr, dataPrt , tmpLength);

        /* update the info about the buffer */
        sbufDataSet(bufferId, tmpPtr, tmpLength);

        /* put buffer on the queue */
        squeBufPut(deviceObjPtr->queueId ,SIM_CAST_BUFF(bufferId));
    }

}



/* function should be called only on he asic side of the distributed architecture */
static GT_STATUS smainNicRxCb_distributedAsic (
    IN GT_U8_PTR   segmentList[],
    IN GT_U32      segmentLen[],
    IN GT_U32      numOfSegments
)
{
    GT_U32                  frameLen = 0;   /* frame's length */
    GT_U8                   frameBuffer[SBUF_DATA_SIZE_CNS];
    GT_U32                  ii;

    if(numOfSegments == 1)
    {
        frameLen = segmentLen[0];
    }
    else
    {
        for (ii = 0; ii < numOfSegments; ii++)
        {
            /* check current frame length */
            if ((frameLen + segmentLen[ii]) > SBUF_DATA_SIZE_CNS)
                return GT_FAIL;

            /* copy segment to the buffer */
            memcpy(frameBuffer + frameLen,
                   segmentList[ii],
                   segmentLen[ii]);

            frameLen += segmentLen[ii];
        }
    }


    simDistributedNicRxFrame(frameLen,frameBuffer);

    return GT_OK;
}

/**
* @internal smainNicRxHandler function
* @endinternal
*
* @brief   Rx NIC callback function
*/
static void smainNicRxHandler
(
    IN GT_U8_PTR   segmentPtr,
    IN GT_U32      segmentLen
)
{
    if(smainNicRxCb==NULL)
    {
        return;
    }

    /* dump to log the packet that ingress the NIC */
    simLogPacketDump(smainNicDeviceObjPtr,
        GT_TRUE , /* ingress direction */
        0,
        segmentPtr,
        segmentLen);

    smainNicRxCb(&segmentPtr,&segmentLen,1);
}

/**
* @internal skernelNicRxBind function
* @endinternal
*
* @brief   Bind Rx callback routine with skernel.
*
* @param[in] rxCbFun                  - callback function for NIC Rx.
*/
void skernelNicRxBind
(
    SKERNEL_NIC_RX_CB_FUN rxCbFun
)
{
    if(sasicgSimulationRole == SASICG_SIMULATION_ROLE_NON_DISTRIBUTED_E)
    {
        smainNicRxCb = rxCbFun;
    }
    else
    {
        /* on the Asic side we send the frame to the other side */
        smainNicRxCb = smainNicRxCb_distributedAsic;
    }
}

/**
* @internal skernelNicRxUnBind function
* @endinternal
*
* @brief   UnBind Rx callback routine with skernel.
*
* @param[in] rxCbFun                  - callback function for unbind NIC Rx.
*/
void skernelNicRxUnBind
(
    SKERNEL_NIC_RX_CB_FUN rxCbFun
)
{
    smainNicRxCb = NULL;
}

/**
* @internal skernelNicOutput function
* @endinternal
*
* @brief   This function transmits an Ethernet packet to the NIC
*
* @param[in] segmentList[]            - segment list
* @param[in] segmentLen[]             - segment length
* @param[in] numOfSegments            - number of segments
*                                       GT_OK if successful, or
*                                       GT_FAIL otherwise.
*/
GT_STATUS skernelNicOutput
(
    IN GT_U8_PTR        segmentList[],
    IN GT_U32           segmentLen[],
    IN GT_U32           numOfSegments
)
{
    SMAIN_PORT_SLAN_INFO *  slanInfoPrt;    /* slan info entry pointer */
    GT_U32                  frameLen = 0;   /* frame's length */
    GT_U32                  i;

    for (i = 0; i < numOfSegments; i++)
    {
        /* check current frame length */
        if ((frameLen + segmentLen[i]) > SBUF_DATA_SIZE_CNS)
            return GT_FAIL;

        if (!smainNicDeviceObjPtr)
            return GT_FAIL;

        /* copy segment to the buffer */
        memcpy(smainNicDeviceObjPtr->egressBuffer1 + frameLen,
               segmentList[i],
               segmentLen[i]);

        frameLen += segmentLen[i];
    }

    /* add 4 CRC bytes if need */
    if (smainNicDeviceObjPtr->crcPortsBytesAdd != 0)
    {
        frameLen += 4;
    }

    /* dump to log the packet that egress the NIC */
    simLogPacketDump(smainNicDeviceObjPtr,
        GT_FALSE , /* egress direction */
        0,
        smainNicDeviceObjPtr->egressBuffer1,
        frameLen);

    if(sasicgSimulationRole == SASICG_SIMULATION_ROLE_DISTRIBUTED_APPLICATION_SIDE_E)
    {
        /* only in the application side of  distributed architecture we need
           to send the info to the other side (Asic side) */

        simDistributedNicTxFrame(frameLen,smainNicDeviceObjPtr->egressBuffer1);
        return GT_OK;
    }


    /* get SLAN info for port */
    slanInfoPrt = &(smainNicDeviceObjPtr->portSlanInfo[0]);

    /* for NIC MUST check 'internal connection' before check for SLAN
       because we may have both initialized !

       but in this case the slanIdTx is used only to allow the DX device to send packet to the slan
       and then the slanIdRx is the one that gets the packet.

       So for the 'from NIC to the device' we use 'peer info' .
    */
    if(smainNicDeviceObjPtr->portsArr[0].peerInfo.usePeerInfo == GT_TRUE)
    {
        smainFrame2InternalConnectionSend(smainNicDeviceObjPtr,0/*port num*/,smainNicDeviceObjPtr->egressBuffer1,frameLen);
    }
    else
    if (slanInfoPrt->slanName[0x0] != 0x0)/* support unbind of slan under traffic */
    {
        /* transmit to SLAN */
        SIM_OS_MAC(simOsSlanTransmit)(slanInfoPrt->slanIdTx, SIM_OS_SLAN_MSG_CODE_FRAME_CNS,
                          frameLen, (char*)(smainNicDeviceObjPtr->egressBuffer1));
    }
    else if(skernelUnderShutDown == GT_FALSE) /*do not generate this error when we shutdown*/
    {
        skernelFatalError(" skernelNicOutput: NIC not bound to sent packets (slan/internal connection)");
    }

    return GT_OK;
}

/**
* @internal smainIsVisualDisabled function
* @endinternal
*
* @brief   Returns the status of the visual asic.
*
* @retval 0                        - Enabled
* @retval 1                        - Disabled
*/
GT_U32 smainIsVisualDisabled
(
    void
)
{
    return smainVisualDisabled;
}

/**
* @internal skernelPolicerConformanceLevelForce function
* @endinternal
*
* @brief   force the conformance level for the packets entering the policer
*         (traffic cond)
* @param[in] dp                       -  conformance level (drop precedence) - green/yellow/red
*
* @retval GT_OK                    - success, GT_FAIL otherwise
*/
GT_STATUS skernelPolicerConformanceLevelForce(
    IN  GT_U32      dp
)
{
    if(dp == SKERNEL_CONFORM_GREEN ||
       dp == SKERNEL_CONFORM_YELLOW ||
       dp == SKERNEL_CONFORM_RED)
    {
        skernelUserDebugInfo.policerConformanceLevel = dp;

        return GT_OK;
    }
    return GT_FAIL;
}

/**
* @internal skernelPolicerInterFramGapInfoSet function
* @endinternal
*
* @brief   force the conformance level for the packets entering the policer
*         (traffic cond)
* @param[in] timeUnit                 - the type of unit for the time :
*                                      0 - SKERNEL_TIME_UNITS_NOT_VALID_E,
*                                      1 - SKERNEL_TIME_UNITS_MILI_SECONDS_E, 10e-3
*                                      2 - SKERNEL_TIME_UNITS_MICRO_SECONDS_E,10e-6
*                                      3 - SKERNEL_TIME_UNITS_NANO_SECONDS_E, 10e-9
*                                      4 - SKERNEL_TIME_UNITS_PICO_SECONDS_E  10e-12
* @param[in] timeValue                - the time in units of timeUnit
*
* @retval GT_OK                    - success, GT_FAIL otherwise
*/
GT_STATUS skernelPolicerInterFramGapInfoSet(
    GT_U32  timeUnit,
    GT_U32  timeValue
)
{
    SKERNEL_TIME_UNITS_ENT  skernelTimeUnit = timeUnit;
    SKERNEL_DEVICE_OBJECT * devObjPtr = NULL;

    SCIB_SEM_TAKE;
    __LOG_PARAM_NO_LOCATION_META_DATA(timeUnit);
    __LOG_PARAM_NO_LOCATION_META_DATA(timeValue);
    SCIB_SEM_SIGNAL;

    if(skernelTimeUnit > SKERNEL_TIME_UNITS_PICO_SECONDS_E)
    {
        return GT_FAIL;
    }

    skernelUserDebugInfo.enhancedPolicerIfgTime.timeUnit           = skernelTimeUnit;
    skernelUserDebugInfo.enhancedPolicerIfgTime.timeBetweenPackets = timeValue;

    return GT_OK;
}

/**
* @internal smainInterruptsMaskChanged function
* @endinternal
*
* @brief   handle interrupt mask registers
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] causeRegAddr             - cause register address
* @param[in] maskRegAddr              - mask  register address
* @param[in] intRegBit                - interrupt bit in the global cause register
* @param[in] currentCauseRegVal       - current cause register values
* @param[in] lastMaskRegVal           - last mask register values
* @param[in] newMaskRegVal            - new mask register values
*/
static void smainInterruptsMaskChanged(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32  causeRegAddr,
    IN GT_U32  maskRegAddr,
    IN GT_U32  intRegBit,
    IN GT_U32  currentCauseRegVal,
    IN GT_U32  lastMaskRegVal,
    IN GT_U32  newMaskRegVal
)
{
    if(SKERNEL_DEVICE_FAMILY_CHEETAH(devObjPtr))
    {
        snetCheetahInterruptsMaskChanged(devObjPtr,causeRegAddr,maskRegAddr,
                intRegBit,currentCauseRegVal,lastMaskRegVal,newMaskRegVal);
    }
    else if(SKERNEL_DEVICE_FAMILY_SOHO(devObjPtr->deviceType))
    {
        snetSohoInterruptsMaskChanged(devObjPtr,causeRegAddr,maskRegAddr,
                intRegBit,currentCauseRegVal,lastMaskRegVal,newMaskRegVal);
    }

    return;
}

/**
* @internal smainUpdateFrameFcs function
* @endinternal
*
* @brief   Update FCS value of ethernet frame
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] frameData                - pointer to the data
* @param[in] frameSize                - data size
*/
void smainUpdateFrameFcs(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U8 * frameData,
    IN GT_U32  frameSize
)
{
    DECLARE_FUNC_NAME(smainUpdateFrameFcs);

    GT_U32 crc;

    /* Calculate FCS enable */
    if (devObjPtr->calcFcsEnable)
    {
        /* calculate the CRC from the packet */
        crc = simCalcCrc32(0xFFFFFFFF, frameData, frameSize - 4);

        /* Set the calculated CRC */
        frameData[frameSize-4] = (GT_U8)(crc >> 24);
        frameData[frameSize-3] = (GT_U8)(crc >> 16);
        frameData[frameSize-2] = (GT_U8)(crc >> 8);
        frameData[frameSize-1] = (GT_U8)(crc);

        /* save info to log */
        __LOG(("Calculate new CRC: %2.2X %2.2X %2.2X %2.2X\n",
                                                     frameData[frameSize-4],
                                                     frameData[frameSize-3],
                                                     frameData[frameSize-2],
                                                     frameData[frameSize-1]));
    }
}

/**
* @internal skernelIsCheetah3OnlyXgDev function
* @endinternal
*
* @brief   check if device cheetah 3 XG (all ports are XG) -- not supports future
*         devices.
* @param[in] devObjPtr                - pointer to device object.
*
* @retval 0                        - not ch3 device
* @retval 1                        - ch3 device
*/
GT_U32 skernelIsCheetah3OnlyXgDev(IN SKERNEL_DEVICE_OBJECT * devObjPtr)
{
    switch(devObjPtr->deviceType)
    {
        case SKERNEL_98DX8110   :
        case SKERNEL_98DX8108   :
        case SKERNEL_98DX8109   :
        case SKERNEL_98DX8110_1 :
        case SKERNEL_DXCH3_XG   :
            return 1;
        default:
            break;
    }

    return 0;
}

/**
* @internal skernelIsXcat3OnlyDev function
* @endinternal
*
* @brief   Check if device should behave like xCat3 (or AC5 that is very much AC3) and not others.
*
* @param[in] devObjPtr                - pointer to device object.
*
* @retval 0                        - not xCat3 device
* @retval 1                        - xCat3 device
*/
GT_U32 skernelIsXcat3OnlyDev(IN SKERNEL_DEVICE_OBJECT * devObjPtr)
{
    switch(devObjPtr->deviceType)
    {
        case SKERNEL_XCAT3_24_AND_6:
        case SKERNEL_AC5_24_AND_6:
            return 1;

        default:
            break;
    }

    return 0;
}

/**
* @internal skernelIsPumaShellOnlyDev function
* @endinternal
*
* @brief   check if device should behave like Puma shell
*
* @param[in] devObjPtr                - pointer to device object.
*
* @retval 0                        - not Puma shell device
* @retval 1                        - Puma shell device
*/
static GT_U32 skernelIsPumaShellOnlyDev(IN SKERNEL_DEVICE_OBJECT * devObjPtr)
{
    switch(devObjPtr->deviceType)
    {
        case SKERNEL_PUMA3_64:
            return 1;

        default:
            break;
    }

    return 0;
}

/**
* @internal skernelIsPhyShellOnlyDev function
* @endinternal
*
* @brief   check if device should behave like PHY shell
*
* @param[in] devObjPtr                - pointer to device object.
*
* @retval 0                        - not PHY shell device
* @retval 1                        - PHY shell device
*/
GT_U32 skernelIsPhyShellOnlyDev(IN SKERNEL_DEVICE_OBJECT * devObjPtr)
{
    switch(devObjPtr->deviceType)
    {
        case SKERNEL_PHY_SHELL:
            return 1;

        default:
            break;
    }

    return 0;
}

/**
* @internal skernelIsPhyOnlyDev function
* @endinternal
*
* @brief   check if device should behave like PHY core
*
* @param[in] devObjPtr                - pointer to device object.
*
* @retval 0                        - not PHY core device
* @retval 1                        - PHY core device
*/
GT_U32 skernelIsPhyOnlyDev(IN SKERNEL_DEVICE_OBJECT * devObjPtr)
{
    switch(devObjPtr->deviceType)
    {
        case SKERNEL_PHY_CORE_1540M_1548M:
            return 1;

        default:
            break;
    }

    return 0;
}

/**
* @internal skernelIsMacSecOnlyDev function
* @endinternal
*
* @brief   check if device should behave like macSec
*
* @param[in] devObjPtr                - pointer to device object.
*
* @retval 0                        - not macSec device
* @retval 1                        - macSec device
*/
GT_U32 skernelIsMacSecOnlyDev(IN SKERNEL_DEVICE_OBJECT * devObjPtr)
{
    switch(devObjPtr->deviceType)
    {
        case SKERNEL_MACSEC:
            return 1;

        default:
            break;
    }

    return 0;
}


/**
* @internal skernelIsPipeDev function
* @endinternal
*
* @brief   check if device should behave like PIPE
*
* @param[in] devObjPtr                - pointer to device object.
*
* @retval 0                        - not PIPE device
* @retval 1                        - PIPE device
*/
static GT_U32 skernelIsPipeDev(IN SKERNEL_DEVICE_OBJECT * devObjPtr)
{
    switch(devObjPtr->deviceType)
    {
        case SKERNEL_PIPE:
            return 1;

        default:
            break;
    }

    return 0;
}



/**
* @internal skernelIsLionShellOnlyDev function
* @endinternal
*
* @brief   check if device should behave like Lion shell
*
* @param[in] devObjPtr                - pointer to device object.
*
* @retval 0                        - not Lion shell device
* @retval 1                        - Lion shell device
*/
static GT_U32 skernelIsLionShellOnlyDev(IN SKERNEL_DEVICE_OBJECT * devObjPtr)
{
    switch(devObjPtr->deviceType)
    {
        case SKERNEL_LION_48:
        case SKERNEL_LION2_96:
        case SKERNEL_LION3_96:
            return 1;

        default:
            break;
    }

    return 0;
}

/**
* @internal skernelIsBobcat2OnlyDev function
* @endinternal
*
* @brief   check if device should behave like Bobcat2
*
* @param[in] devObjPtr                - pointer to device object.
*
* @retval 0                        - not Lion3's port group device
* @retval 1                        - Lion3's port group device
*/
static GT_U32 skernelIsBobcat2OnlyDev(IN SKERNEL_DEVICE_OBJECT * devObjPtr)
{
    switch(devObjPtr->deviceType)
    {
        case SKERNEL_BOBCAT2:
        case SKERNEL_BOBK_CAELUM:
        case SKERNEL_BOBK_CETUS:
        case SKERNEL_BOBK_ALDRIN:
        case SKERNEL_AC3X:
        case SKERNEL_BOBCAT3:
        case SMEM_CASE_FALCON_DEV_MAC:
        case SKERNEL_HAWK:
        case SKERNEL_PHOENIX:
        case SKERNEL_HARRIER:
        case SKERNEL_IRONMAN_L:
        case SKERNEL_IRONMAN_S:
        case SKERNEL_ALDRIN2:
            return 1;
        default:
            return skernelIsPipeDev(devObjPtr);
    }
}

/**
* @internal skernelIsLion3PortGroupOnlyDev function
* @endinternal
*
* @brief   check if device should behave like Lion3's port group
*
* @param[in] devObjPtr                - pointer to device object.
*
* @retval 0                        - not Lion3's port group device
* @retval 1                        - Lion3's port group device
*/
GT_U32 skernelIsLion3PortGroupOnlyDev(IN SKERNEL_DEVICE_OBJECT * devObjPtr)
{
    switch(devObjPtr->deviceType)
    {
        case SKERNEL_LION3_PORT_GROUP_12:
            return 1;

        default:
            break;
    }

    if (skernelIsBobcat2OnlyDev(devObjPtr))
    {
        return 1;
    }

    return 0;
}

/**
* @internal deviceTypeInfoGet function
* @endinternal
*
* @brief   This function sets the device info :
*         number of ports ,
*         deviceType ,
*         and returns bmp of ports for a given device.
* @param[in,out] devObjPtr                - pointer to device object.
* @param[in,out] devObjPtr                - pointer to device object.
*                                       GT_OK on success,
*                                       GT_NOT_FOUND if the given pciDevType is illegal.
*
* @retval GT_NOT_SUPPORTED         - not properly supported device in DB
*/
static GT_STATUS deviceTypeInfoGet
(
    INOUT  SKERNEL_DEVICE_OBJECT * devObjPtr
)
{
    GT_U32   ii;/* index in simSupportedTypes */
    GT_U32   jj;/* index in simSupportedTypes[ii].devTypeArray */
    GT_U32   kk;/* index in simSpecialDevicesBmp */
    GT_BOOL found = GT_FALSE;
    const PORTS_BMP_STC *existingPortsPtr;
    const PORTS_BMP_STC *multiModePortsBmpPtr;
    PORTS_BMP_STC tmpBmp = {{0}};
    const SKERNEL_PORT_STATE_ENT *existingPortsStatePtr;
    const SIM_SUPPORTED_TYPES_STC *simSupportedTypesPtr;

    if(simulationCheck_onEmulator())
    {
        simSupportedTypesPtr = (const SIM_SUPPORTED_TYPES_STC *)onEmulator_simSupportedTypes;
        if(simulationCheck_onEmulator_isAldrinFull())
        {
            /* get the info about our device */
            ii = 0;
            while(onEmulator_simSupportedTypes[ii].devFamily != END_OF_TABLE)
            {
                if(onEmulator_simSupportedTypes[ii].devFamily == SKERNEL_BOBK_ALDRIN_FAMILY &&
                   onEmulator_simSupportedTypes[ii].defaultPortsBmpPtr)
                {
                    /* modify to limit no ports */
                    onEmulator_simSupportedTypes[ii].defaultPortsBmpPtr = NULL;

                    break;
                }
                ii++;
            }
        }
    }
    else
    {
        simSupportedTypesPtr = simSupportedTypes;
    }


    /* get the info about our device */
    ii = 0;
    while(simSupportedTypesPtr[ii].devFamily != END_OF_TABLE)
    {
        jj = 0;
        while(simSupportedTypesPtr[ii].devTypeArray[jj] != END_OF_TABLE)
        {
            if(devObjPtr->deviceType == simSupportedTypesPtr[ii].devTypeArray[jj])
            {
                found = GT_TRUE;
                break;
            }
            jj++;
        }

        if(found == GT_TRUE)
        {
            break;
        }
        ii++;
    }

    if(found == GT_FALSE)
    {
        devObjPtr->deviceFamily = SKERNEL_DEVICE_FAMILY(devObjPtr);
        if(devObjPtr->deviceFamily == SKERNEL_NOT_INITIALIZED_FAMILY)
        {
            skernelFatalError(" deviceTypeInfoGet: unknown family for device[0x%8.8x]",devObjPtr->deviceType);
        }

        return GT_NOT_FOUND;
    }

    devObjPtr->portsNumber = simSupportedTypesPtr[ii].numOfPorts;

    if(devObjPtr->portsNumber > SKERNEL_DEV_MAX_SUPPORTED_PORTS_CNS)
    {
        skernelFatalError(" deviceTypeInfoGet: not supporting more than %d ports \n",
                          SKERNEL_DEV_MAX_SUPPORTED_PORTS_CNS);
    }

    devObjPtr->deviceFamily = simSupportedTypesPtr[ii].devFamily;

    if(simSupportedTypesPtr[ii].defaultPortsBmpPtr == NULL)
    {
        /* no special default for the ports BMP , use continuous ports bmp */
        existingPortsPtr = &tmpBmp;

        if(simSupportedTypesPtr[ii].numOfPorts < 32)
        {
            BMP_PORTS_LESS_32_MAC((&tmpBmp),
                            simSupportedTypesPtr[ii].numOfPorts);
        }
        else if (simSupportedTypesPtr[ii].numOfPorts < 64)
        {
            BMP_PORTS_32_TO_63_MAC((&tmpBmp),
                            simSupportedTypesPtr[ii].numOfPorts);
        }
        else if (simSupportedTypesPtr[ii].numOfPorts < 96)
        {
            BMP_PORTS_64_TO_95_MAC((&tmpBmp),
                            simSupportedTypesPtr[ii].numOfPorts);
        }
        else /* generic case */
        {
            GT_U32  numOfWords = (simSupportedTypesPtr[ii].numOfPorts + 31)/32;
            GT_U32  numOfPortsLastWord = simSupportedTypesPtr[ii].numOfPorts % 32;

            for(jj = 0 ; jj < (numOfWords-1) ; jj++)
            {
                tmpBmp.ports[jj] = 0xFFFFFFFF;
            }

            if(0 == (numOfPortsLastWord))
            {
                numOfPortsLastWord = 32;/* will set 0xFFFFFFFF at last word */
            }

            tmpBmp.ports[numOfWords-1] =
                BMP_CONTINUES_PORTS_MAC(numOfPortsLastWord);
        }
    }
    else
    {
        /* use special default for the ports BMP */
        existingPortsPtr = simSupportedTypesPtr[ii].defaultPortsBmpPtr;
    }

    existingPortsStatePtr = simSupportedTypesPtr[ii].existingPortsStatePtr;
    multiModePortsBmpPtr  = simSupportedTypesPtr[ii].multiModePortsBmpPtr;

    /****************************************************************/
    /* add here specific devices BMP of ports that are not standard */
    /****************************************************************/
    kk = 0;
    while(simSpecialDevicesBmp[kk].devType != END_OF_TABLE)
    {
        if(simSpecialDevicesBmp[kk].devType == devObjPtr->deviceType)
        {
            existingPortsPtr = &simSpecialDevicesBmp[kk].existingPorts;
            multiModePortsBmpPtr = & simSpecialDevicesBmp[kk].multiModePortsBmp;
            break;
        }
        kk++;
    }

    for(ii = 0 ; ii < devObjPtr->portsNumber ; ii++)
    {
        if(PORTS_BMP_IS_PORT_SET_MAC(existingPortsPtr,ii))
        {
            /* get default port's state */
            devObjPtr->portsArr[ii].state = existingPortsStatePtr[ii];
            if(multiModePortsBmpPtr)
            {
                devObjPtr->portsArr[ii].supportMultiState =
                    PORTS_BMP_IS_PORT_SET_MAC(multiModePortsBmpPtr,ii) ?
                        GT_TRUE : GT_FALSE ;
            }
            else
            {
                devObjPtr->portsArr[ii].supportMultiState = GT_FALSE;
            }
        }
        else
        {
            /* port not exists */
            devObjPtr->portsArr[ii].state = SKERNEL_PORT_STATE_NOT_EXISTS_E;
            devObjPtr->portsArr[ii].supportMultiState = GT_FALSE;
        }
    }

    /* set all other ports a 'Not exists' */
    for(/* continue */; ii < SKERNEL_DEV_MAX_SUPPORTED_PORTS_CNS; ii++)
    {
        devObjPtr->portsArr[ii].state = SKERNEL_PORT_STATE_NOT_EXISTS_E;
        devObjPtr->portsArr[ii].supportMultiState = GT_FALSE;
    }

    return GT_OK;
}

/**
* @internal smainMemAddrDefaultGet function
* @endinternal
*
* @brief   Load default values for memory for specified address from text file
*
* @param[out] valuePtr                 - pointer to lookup address value
*
* @retval GT_OK                    - lookup address found
* @retval GT_FAIL                  - lookup address not found
* @retval GT_BAD_PTR               - NULL pointer
*/
GT_STATUS smainMemAddrDefaultGet
(
    IN SKERNEL_DEVICE_OBJECT    * deviceObjPtr,
    IN GT_CHAR                  * fileNamePtr,
    IN GT_U32                   address,
    OUT GT_U32                  * valuePtr
)
{
    FILE    * regFilePtr;   /* file object */
    static char  buffer[FILE_MAX_LINE_LENGTH_CNS]; /* string buffer */
    GT_U32 unitAddr = 0;    /* unit address for register, 0 means not valid*/
    GT_U32 baseAddr;        /* base address for register */
    GT_U32 value;           /* default value */
    GT_32  number;          /* number of registers */
    GT_U32 addrDelta;       /* delta for several registers */
    GT_U32 scanRes;         /* number scanned elements */
    char *  remarkStr;
    GT_CHAR pciAddr[SMAIN_PCI_FLAG_BUFF_SIZE];
    GT_U32  currentLineIndex = 0;/*current line number in the 'registers file'*/
    GT_STATUS status = GT_FAIL;

    if (!fileNamePtr || !valuePtr)
       return GT_BAD_PTR;

    /* open file */
    regFilePtr = fopen(fileNamePtr,"rt");
    if (regFilePtr == NULL)
    {
        /* avoid mess of messages */
        SIM_OS_MAC(simOsSleep)(1);
        skernelFatalError("smainMemAddrDefaultGet: registers file not found %s\n", fileNamePtr);
    }

    while (fgets(buffer, FILE_MAX_LINE_LENGTH_CNS, regFilePtr))
    {
        currentLineIndex++;/* 1 based number */
        /* convert to lower case */
        strlwr(buffer);

        remarkStr = strstr(buffer,";");
        if (remarkStr == buffer)
        {
            continue;
        }
        /* UNIT_BASE_ADDR string detection */
        if(!sscanf(buffer, "unit_base_addr %x\n", &unitAddr))
        {
            if(strstr(buffer, "unit_base_addr not_valid"))
            {
                unitAddr = 0;
                continue;
            }
        }
        else
        {
            continue;
        }
        /* record found, parse it */
        baseAddr = 0;
        value = 0;
        number = 0;
        addrDelta = 0; /* if scanRes < 4 => addrDelta is undefined */
        memset(pciAddr, 0, SMAIN_PCI_FLAG_BUFF_SIZE);
        scanRes = sscanf(buffer, "%x %x %d %x %10s", &baseAddr,
                        &value, &number, &addrDelta, pciAddr);

        if (baseAddr == SMAIN_FILE_RECORDS_EOF)
        {
            break;
        }
        if (((baseAddr & 0xFF000000) != 0) && (unitAddr != 0))
        {
            skernelFatalError("When UNIT_BASE_ADDR is valid then the address of register must be 'zero based' %s (%d)\n", fileNamePtr, currentLineIndex);
         }

        baseAddr += unitAddr;
        if(baseAddr == address)
        {
            *valuePtr = value;
            status = GT_OK;
            break;
        }

        if ((scanRes < 2) || (scanRes == 3) ||(scanRes > 5))
        {
            /* check end of file */
            if (scanRes == 1)
                break;

            simWarningPrintf("smainMemAddrDefaultGet: registers file's bad format in line [%d]\n",currentLineIndex);
            break;
        }
    }

    fclose(regFilePtr);

    return status;
}

/**
* @internal smainMemAdditionalAddrDefaultGet function
* @endinternal
*
* @brief   Load default values for memory for specified address from additional text file
*
* @param[out] valuePtr                 - pointer to lookup address value
*
* @retval GT_OK                    - lookup address found
* @retval GT_FAIL                  - lookup address not found
* @retval GT_BAD_PTR               - NULL pointer
*/
GT_STATUS smainMemAdditionalAddrDefaultGet
(
    IN SKERNEL_DEVICE_OBJECT    * deviceObjPtr,
    IN GT_U32                   devId,
    IN GT_U32                   address,
    OUT GT_U32                  * valuePtr
)
{
    GT_CHAR keyStr[SMAIN_MIN_SIZE_OF_BUFFER_CNS]; /* key string*/
    GT_CHAR paramStr[FILE_MAX_LINE_LENGTH_CNS];
    GT_STATUS retVal = GT_FAIL;
    GT_U32 regData;
    GT_U32 i;

    sprintf(keyStr, "registers%u", devId);
    if (!SIM_OS_MAC(simOsGetCnfValue)(INI_FILE_SYSTEM_SECTION_CNS, keyStr,
           FILE_MAX_LINE_LENGTH_CNS, paramStr))
    {
        if(deviceObjPtr->portGroupSharedDevObjPtr)
        {
            /* use the register file of the 'father' device object (from the system) */
            return smainMemAdditionalAddrDefaultGet(deviceObjPtr, devId, address, valuePtr);
        }

        return GT_FAIL;
    }

    if(GT_OK == smainMemAddrDefaultGet(deviceObjPtr, paramStr, address, &regData))
    {
        retVal = GT_OK;
        *valuePtr = regData;
    }

    /* Additional defaults registers value, files */
    for(i = 1; i < 100; i++)
    {
        sprintf(keyStr, "registers%u_%2.2u", devId, i);
        if (GT_FALSE == SIM_OS_MAC(simOsGetCnfValue)(INI_FILE_SYSTEM_SECTION_CNS,
                                                     keyStr,
                                                     FILE_MAX_LINE_LENGTH_CNS,
                                                     paramStr))
        {
            /* no more registers files */
            break;
        }

        if(GT_OK == smainMemAddrDefaultGet(deviceObjPtr, paramStr, address, &regData))
        {
            retVal = GT_OK;
            /* let other additional file to override the value */
            *valuePtr = regData;
        }
    }

    return retVal;
}

/**
* @internal smainDeviceRevisionGet function
* @endinternal
*
* @brief   Load revision ID values for memory from text file
*
* @param[in] deviceObjPtr             - pointer to the device object.
* @param[in] devId                    - the deviceId to use when build the name in INI file to look for
*
* @retval GT_OK                    - revision ID found
* @retval GT_FAIL                  - revision ID not found
* @retval GT_NOT_SUPPORTED         - not cheetah device
*/
static GT_STATUS smainDeviceRevisionGet
(
    IN SKERNEL_DEVICE_OBJECT    * deviceObjPtr,
    IN GT_U32                   devId
)
{
    GT_U32 regData;
    GT_STATUS retVal;
    GT_U32  revision = 0xFFFFFFFF;

    /* GM memory may not be initialized, so there is no access to register 0x4c.
       We also don't use deviceObjPtr->deviceRevisionId in GM */
    if (deviceObjPtr->gmDeviceType == GOLDEN_MODEL)
    {
        return GT_NOT_SUPPORTED;
    }

    /* Relevant only for cheetah devices */
    if(SKERNEL_DEVICE_FAMILY_CHEETAH(deviceObjPtr) == 0)
    {
        return GT_NOT_SUPPORTED;
    }

    retVal =
        smainMemAdditionalAddrDefaultGet(deviceObjPtr, devId, 0x4c, &regData);
    if(retVal == GT_OK)
    {
        revision = (regData & 0xf);
        /* let other additional file to override the value */
    }

    if(revision != 0xFFFFFFFF)
    {
        deviceObjPtr->deviceRevisionId = revision;
        return GT_OK;
    }

    return GT_FAIL;
}

/**
* @internal smainMemDefaultsLoadAll function
* @endinternal
*
* @brief   Load default values for device memory from all default registers files
*
* @param[in] deviceObjPtr             - pointer to the device object
* @param[in] devId                    - device Id
* @param[in] sectionPtr               - pointer to section name
*/
void smainMemDefaultsLoadAll
(
    IN SKERNEL_DEVICE_OBJECT    * deviceObjPtr,
    IN GT_U32                   devId,
    IN GT_CHAR                  * sectionPtr
)
{
    GT_CHAR  keyPtr[SMAIN_MIN_SIZE_OF_BUFFER_CNS];                          /* searching key string*/
    GT_CHAR  devInfoSection[SMAIN_MIN_SIZE_OF_BUFFER_CNS];                  /*the device info section in the INI file*/
    GT_CHAR  strVal[FILE_MAX_LINE_LENGTH_CNS];    /* output string value */
    GT_U32  ii;
    SMEM_CHT_DEV_COMMON_MEM_INFO  * commonDevMemInfoPtr = deviceObjPtr->deviceMemory;
    SMEM_UNIT_CHUNKS_STC    *dfxUnitChunkPtr;
    SMEM_CHT_GENERIC_DEV_MEM_INFO *devMemInfoPtr = deviceObjPtr->deviceMemory;

    if (deviceObjPtr->shellDevice)
    {
        return;
    }

    if(deviceObjPtr->registersDefaultsPtr && (deviceObjPtr->gmDeviceType == SMAIN_NO_GM_DEVICE))
    {
        /* the device hold default register values that need to be uploaded before the external 'Register file' loaded */
        smemInitRegistersWithDefaultValues(deviceObjPtr, SKERNEL_MEMORY_WRITE_E, deviceObjPtr->registersDefaultsPtr,NULL);
    }

    /* check for defaults 'per unit' */
    if(deviceObjPtr->isWmDxDevice && (deviceObjPtr->gmDeviceType == SMAIN_NO_GM_DEVICE))/* DX and PX ! but not 'soho' */
    {
        for(ii = 0 ; ii < SMEM_CHT_NUM_UNITS_MAX_CNS; ii++)
        {
            if(devMemInfoPtr->unitMemArr[ii].unitDefaultRegistersPtr)
            {
                smemInitRegistersWithDefaultValues(deviceObjPtr, SKERNEL_MEMORY_WRITE_E,
                    /* the default registers of the unit */
                    devMemInfoPtr->unitMemArr[ii].unitDefaultRegistersPtr,
                    /* the unit info */
                    &devMemInfoPtr->unitMemArr[ii]);
            }
        }

        /* default values for memory space for BAR0 */
        if(devMemInfoPtr->BAR0_UnitMem.unitDefaultRegistersPtr)
        {
            smemInitRegistersWithDefaultValues(deviceObjPtr,
                SCIB_MEMORY_WRITE_BAR0_E,
                devMemInfoPtr->BAR0_UnitMem.unitDefaultRegistersPtr,
                &devMemInfoPtr->BAR0_UnitMem);
        }
    }

    if(deviceObjPtr->registersDfxDefaultsPtr)
    {
        dfxUnitChunkPtr = &commonDevMemInfoPtr->pciExtMemArr[SMEM_UNIT_PCI_BUS_DFX_E].unitMem;

        if(dfxUnitChunkPtr->otherPortGroupDevObjPtr == 0)/* indication that this core actually 'holds' the DFX memory */
        {
            /* the device holds DFX server default register values that need to be uploaded before the external 'Register file' loaded */
            smemInitRegistersWithDefaultValues(deviceObjPtr, SKERNEL_MEMORY_WRITE_DFX_E, deviceObjPtr->registersDfxDefaultsPtr,NULL);
        }
    }

    if(deviceObjPtr->registersPexDefaultsPtr)
    {
        /* the device holds PEX default register values that need to be uploaded before the external 'Register file' loaded */
        smemInitRegistersWithDefaultValues(deviceObjPtr, SKERNEL_MEMORY_WRITE_PCI_E, deviceObjPtr->registersPexDefaultsPtr,NULL);
    }


    sprintf(devInfoSection, "%s", sectionPtr);

    if(deviceObjPtr->portGroupSharedDevObjPtr)
    {
        sprintf(devInfoSection, "%s", INI_FILE_SYSTEM_SECTION_CNS);
    }

    sprintf(keyPtr, "registers%u", devId);

    /* Get default registers value file */
    if(SIM_OS_MAC(simOsGetCnfValue)(devInfoSection, keyPtr,
                                    FILE_MAX_LINE_LENGTH_CNS, strVal))
    {
        smainMemDefaultsLoad(deviceObjPtr, strVal);

        for(ii = 1; ii < 100; ii++)
        {
            sprintf(keyPtr, "registers%u_%2.2u", devId, ii);

            /* Get additional default registers value file */
            if(GT_FALSE == SIM_OS_MAC(simOsGetCnfValue)(devInfoSection, keyPtr,
                                                        FILE_MAX_LINE_LENGTH_CNS, strVal))
            {
                /* no more registers files */
                break;
            }

            smainMemDefaultsLoad(deviceObjPtr, strVal);
        }
    }
}

/*******************************************************************************
*   skernelDeviceSoftReset
*
* DESCRIPTION:
*       Soft reset of single device object
*
* INPUTS:
*       devObjPtr           - pointer to device object.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*
* COMMENTS:
*       Function do device soft reset on the following order:
*           - disable device interrupts
*           - force link of all active SLANs down
*           - disable SAGING task
*           - reset all AISC memories by default values
*           - enable SAGING task
*           - force link of all active SLANs up
*           - enable device interrupts
*
*******************************************************************************/
static SKERNEL_DEVICE_OBJECT *   skernelDeviceSoftReset
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr
)
{
    if(devObjPtr->devSoftResetFunc)
    {
        return devObjPtr->devSoftResetFunc(devObjPtr);
    }

    return devObjPtr;
}

/*******************************************************************************
*   skernelDeviceSoftResetPart2
*
* DESCRIPTION:
*       Soft reset of single device object - part 2
*
* INPUTS:
*       devObjPtr           - pointer to device object.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*
* COMMENTS:
*
*******************************************************************************/
static SKERNEL_DEVICE_OBJECT *   skernelDeviceSoftResetPart2
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr
)
{
    if(devObjPtr->devSoftResetFuncPart2)
    {
        return devObjPtr->devSoftResetFuncPart2(devObjPtr);
    }


    return devObjPtr;
}


/**
* @internal skernelPortLoopbackForceModeSet function
* @endinternal
*
* @brief   the function set the 'loopback force mode' on a port of device.
*         this function needed for devices that not support loopback on the ports
*         and need 'external support'
* @param[in] deviceId                 - the simulation device Id .
* @param[in] portNum                  - the physical port number .
* @param[in] mode                     - the loopback force mode.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad portNum or mode
*
* @note function do fatal error on non-exists device or out of range device.
*
*/
GT_STATUS skernelPortLoopbackForceModeSet
(
    IN  GT_U32                      deviceId,
    IN  GT_U32                      portNum,
    IN SKERNEL_PORT_LOOPBACK_FORCE_MODE_ENT mode
)
{
    SKERNEL_DEVICE_OBJECT * devObjPtr;
    SKERNEL_PORT_INFO_STC   *portInfoPtr;

    devObjPtr = smemTestDeviceIdToDevPtrConvert(deviceId);
    if(portNum >= SKERNEL_DEV_MAX_SUPPORTED_PORTS_CNS)
    {
        return GT_BAD_PARAM;
    }

    switch(mode)
    {
        case SKERNEL_PORT_LOOPBACK_NOT_FORCED_E:
        case SKERNEL_PORT_LOOPBACK_FORCE_ENABLE_E:
            break;
        default:
            return GT_BAD_PARAM;
    }


    portInfoPtr =  &devObjPtr->portsArr[portNum];
    portInfoPtr->loopbackForceMode = mode;

    return GT_OK;
}

/**
* @internal skernelPortLinkStateSet function
* @endinternal
*
* @brief   the function set the 'link state' on a port of device.
*         this function needed for devices that not support 'link change' from the
*         'MAC registers' of the ports.
*         this is relevant to 'GM devices'
* @param[in] deviceId                 - the simulation device Id .
* @param[in] portNum                  - the physical port number .
* @param[in] linkState                - the link state to set for the port.
*                                      GT_TRUE  - force 'link UP'
*                                      GT_FALSE - force 'link down'
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad portNum or mode
*
* @note function do fatal error on non-exists device or out of range device.
*
*/
GT_STATUS skernelPortLinkStateSet
(
    IN  GT_U32                      deviceId,
    IN  GT_U32                      portNum,
    IN GT_BOOL                      linkState
)
{
    SKERNEL_DEVICE_OBJECT * devObjPtr;


    devObjPtr = smemTestDeviceIdToDevPtrConvert(deviceId);
    if(portNum >= SKERNEL_DEV_MAX_SUPPORTED_PORTS_CNS)
    {
        return GT_BAD_PARAM;
    }

    snetLinkStateNotify(devObjPtr,portNum,linkState);

    return GT_OK;
}
/**
* @internal prvConvertLocalDevPort function
* @endinternal
*
* @brief   Converts global devId and port to local (core) device and port
*
* @param[in,out] deviceIdPtr              - (pointer to)the simulation device Id
* @param[in,out] portNumPtr               - (pointer to)the physical port number
* @param[in,out] deviceIdPtr              - (pointer to)core device Id
* @param[in,out] portNumPtr               - (pointer to)the local port number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad deviceId or portNumPtr
* @retval GT_BAD_PTR               - on bad pointers
*/
static GT_STATUS prvConvertLocalDevPort
(
    INOUT GT_U32        *deviceIdPtr,
    INOUT GT_U32        *portNumPtr
)
{
    GT_U32          ii;
    SKERNEL_DEVICE_OBJECT *deviceObjPtr; /* device object pointer */

    /* get dev object */
    deviceObjPtr = smemTestDeviceIdToDevPtrConvert(*deviceIdPtr);

    /* find to which core the port belongs */
    for(ii = 0 ; ii < (deviceObjPtr->numOfCoreDevs - 1) ;ii++)
    {
        if((*portNumPtr) < deviceObjPtr->coreDevInfoPtr[ii + 1].startPortNum)
        {
            break;
        }
    }

    if(ii == (deviceObjPtr->numOfCoreDevs - 1))
    {
        if((*portNumPtr) == PRESTERA_CPU_PORT_CNS)
        {
            /* currently simulation implementation will bind 'cpu port' from the
               INI file into core 0 */
            ii = 0;
        }
        else
        {
            if(((*portNumPtr)- deviceObjPtr->coreDevInfoPtr[ii].startPortNum) >= deviceObjPtr->coreDevInfoPtr[ii].devObjPtr->portsNumber)
            {
                simWarningPrintf("prvConvertLocalDevPort: global port [%d] was not found in valid ranges of the cores \n", (*portNumPtr));
                /* global port was not found in valid ranges of the cores */
                return GT_BAD_PARAM;
            }
        }
    }

    /* get local devId */
    *deviceIdPtr = deviceObjPtr->coreDevInfoPtr[ii].devObjPtr->deviceId;

    /* get local port */
    *portNumPtr -= deviceObjPtr->coreDevInfoPtr[ii].startPortNum;

    return GT_OK;
}

/**
* @internal skernelUnbindDevPort2Slan function
* @endinternal
*
* @brief   The function unbinds given dev and port from slan
*
* @param[in] deviceId                 - the simulation device Id
* @param[in] portNum                  - the physical port number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad deviceId or portNum
*/
GT_STATUS skernelUnbindDevPort2Slan
(
    IN  GT_U32                      deviceId,
    IN  GT_U32                      portNum
)
{
    GT_STATUS              st;
    SKERNEL_DEVICE_OBJECT *deviceObjPtr; /* device object pointer */
    SMAIN_PORT_SLAN_INFO  *slanInfoPrt;  /* slan port entry pointer */
    GT_U32          port = portNum;
    GT_BOOL         unbindDone = GT_FALSE;
    GT_U32          portForLink;/* port number that need to be used to access :
        deviceObjPtr->portsArr[portForLink] , and used for calling snetLinkStateNotify(...) */

    DEVICE_ID_CHECK_MAC(deviceId);
    /* get device object */
    deviceObjPtr = smemTestDeviceIdToDevPtrConvert(deviceId);

    /* check for multiport group father device */
    if(deviceObjPtr->shellDevice == GT_TRUE)
    {
        /* get local dev id and port */
        st = prvConvertLocalDevPort(&deviceId, &port);
        if(GT_OK != st)
        {
            return st;
        }
        /* get device object */
        deviceObjPtr = smemTestDeviceIdToDevPtrConvert(deviceId);
    }

    if((port >= deviceObjPtr->numSlans) ||
       (deviceObjPtr->portsArr[port].state == SKERNEL_PORT_STATE_NOT_EXISTS_E))
    {
        simWarningPrintf("skernelUnbindDevPort2Slan: wrong port [%d]\n", port);
        return GT_BAD_PARAM;
    }

    /* get slan info */
    slanInfoPrt = &(deviceObjPtr->portSlanInfo[port]);

    SIM_OS_MAC(simOsSemWait)(slanDownSemaphore,SIM_OS_WAIT_FOREVER);
    slanInfoPrt->slanName[0] = 0; /* before the unbind , make sure no one try to send packet to this port */
    SIM_OS_MAC(simOsSemSignal)(slanDownSemaphore);

    portForLink = snetChtExtendedPortMacGet(deviceObjPtr,port,GT_TRUE);

    if(!deviceObjPtr->portsArr[portForLink].usedByOtherConnection)
    {
        deviceObjPtr->portsArr[portForLink].linkStateWhenNoForce = SKERNEL_PORT_NATIVE_LINK_DOWN_E;
    }

    /* allow all other tasks that send traffic to not be inside the SLAN that going to be unbound */
    SIM_OS_MAC(simOsSleep)(1);

    /* unbind the port */
    unbindDone = internalSlanUnbind(slanInfoPrt);

    if(unbindDone == GT_TRUE && !deviceObjPtr->portsArr[portForLink].usedByOtherConnection)
    {
        if(deviceObjPtr->portsArr[portForLink].isForcedLinkUp == GT_FALSE ||
           deviceObjPtr->portsArr[portForLink].isForcedLinkDown == GT_TRUE)
        {
            /*set port as 'down'*/
            snetLinkStateNotify(deviceObjPtr, portForLink, 0);
        }
    }

    return GT_OK;
}

/**
* @internal skernelBindDevPort2Slan function
* @endinternal
*
* @brief   The function binds given dev and port to slan
*
* @param[in] deviceId                 - the simulation device Id
* @param[in] portNum                  - the physical port number
* @param[in] slanNamePtr              - slan name string,
* @param[in] unbindOtherPortsOnThisSlan -
*                                      GT_TRUE  - if the given slanName is bound to any other port(s)
*                                      unbind it from this port(s) before bind it to the new port.
*                                      This will cause link change (link down) on this other port(s).
*                                      GT_FALSE - bind the port with slanName regardless
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad deviceId, portNum, slan name
* @retval GT_FAIL                  - on error
*/
GT_STATUS skernelBindDevPort2Slan
(
    IN  GT_U32                      deviceId,
    IN  GT_U32                      portNum,
    IN  GT_CHAR                    *slanNamePtr,
    IN  GT_BOOL                     unbindOtherPortsOnThisSlan
)
{
    GT_STATUS              st;
    SKERNEL_DEVICE_OBJECT *deviceObjPtr; /* device object pointer */
    SKERNEL_DEVICE_OBJECT *currDeviceObjPtr; /* current device object pointer */
    SMAIN_PORT_SLAN_INFO  *slanInfoPrt;  /* slan port entry pointer */
    GT_U32                 ii,jj;       /* iterators */
    GT_U32          port = portNum;
    GT_U32          portForLink;/* port number that need to be used to access :
        deviceObjPtr->portsArr[portForLink].linkStateWhenNoForce ,
        and used for calling snetLinkStateNotify(...) */

    DEVICE_ID_CHECK_MAC(deviceId);
    deviceObjPtr = smemTestDeviceIdToDevPtrConvert(deviceId);

    /* check for multiport group father device */
    if(deviceObjPtr->shellDevice)
    {
        /* get local dev id and port */
        st = prvConvertLocalDevPort(&deviceId, &port);
        if(GT_OK != st)
        {
            return st;
        }
        /* get device object */
        deviceObjPtr = smemTestDeviceIdToDevPtrConvert(deviceId);
    }

    /* check port */
    if((port >= deviceObjPtr->numSlans) ||
       (deviceObjPtr->portsArr[port].state == SKERNEL_PORT_STATE_NOT_EXISTS_E))
    {
        simWarningPrintf("skernelBindDevPort2Slan: wrong port [%d]\n", port);
        return GT_BAD_PARAM;
    }

    /* check slanNamePtr */
    if (NULL == slanNamePtr)
    {
        simWarningPrintf("skernelBindDevPort2Slan: wrong slan name pointer[NULL]\n");
        return GT_BAD_PARAM;
    }

    /* get slan info */
    slanInfoPrt = &(deviceObjPtr->portSlanInfo[port]);

    if(0 == strcmp(slanInfoPrt->slanName,slanNamePtr))
    {
        /* The slan name was not changed , do not trigger unbind and re-bind */
        return GT_OK;
    }

    /* unbind this port from previous bind */
    st = skernelUnbindDevPort2Slan(deviceId, port);
    if(GT_OK != st)
    {
        return st;
    }

    /* check link with the same slan name (if need) */
    if(unbindOtherPortsOnThisSlan == GT_TRUE)
    {
        /* search the same slan name in all devices */
        for (ii = 0; ii < SMAIN_MAX_NUM_OF_DEVICES_CNS; ii++)
        {
            currDeviceObjPtr = smainDeviceObjects[ii];
            if (!currDeviceObjPtr)
            {
                continue;
            }

            if(currDeviceObjPtr->shellDevice == GT_TRUE)
            {
                continue;
            }

            /* port loop */
            for (jj = 0; jj < currDeviceObjPtr->numSlans; jj++)
            {
                /* get slan info */
                slanInfoPrt = &(currDeviceObjPtr->portSlanInfo[jj]);

                if ( strcmp(slanInfoPrt->slanName, slanNamePtr) == 0 )
                {
                    /* unbind slan */
                    st = skernelUnbindDevPort2Slan(ii, jj);
                    if(GT_OK != st)
                    {
                        return st;
                    }
                }
            }
        }
    }

    /* get slan info */
    slanInfoPrt = &(deviceObjPtr->portSlanInfo[port]);

    slanInfoPrt->deviceObj  = deviceObjPtr;
    slanInfoPrt->portNumber = port;

    /* bind port - RX,TX direction with SLAN */
    internalSlanBind(slanNamePtr,deviceObjPtr,port,GT_TRUE,GT_TRUE,slanInfoPrt,NULL);

    /* check if conversion needed for the 'link change' , as the actual MAC number
        could be different than the 'orig number' .. like extended port in xCat3/AC5.
    */

    portForLink = snetChtExtendedPortMacGet(deviceObjPtr,port,GT_TRUE);

    /* notify link UP */
    snetLinkStateNotify(deviceObjPtr, portForLink, 1);

    return GT_OK;
}

/**
* @internal skernelDevPortSlanGet function
* @endinternal
*
* @brief   The function get the slanName of a port of device
*
* @param[in] deviceId                 - the simulation device Id
* @param[in] portNum                  - the physical port number
* @param[in] slanMaxLength            - the length of the buffer (for the string) that is
*                                      allocated by the caller for slanNamePtr
*                                      when slanNamePtr != NULL then slanMaxLength must be >= 8
*
* @param[out] portBoundPtr             - (pointer to)is port bound.
* @param[out] slanNamePtr              - (pointer to)slan name (string).
*                                      The caller function must cares about memory allocation.
*                                      NOTE:
*                                      1. this parameter can be NULL ... meaning that caller not
*                                      care about the slan name only need to know that port
*                                      bound / not bound
*                                      2. if slanMaxLength is less than the actual length of
*                                      the slanName then only part of the name will be returned.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad deviceId or portNum.
*                                       when slanNamePtr!=NULL but slanMaxLength < 8
* @retval GT_BAD_PTR               - on NULL pointer portBoundPtr
*/
GT_STATUS skernelDevPortSlanGet
(
    IN  GT_U32                deviceId,
    IN  GT_U32                portNum,
    IN  GT_U32                slanMaxLength,
    OUT GT_BOOL              *portBoundPtr,
    OUT GT_CHAR              *slanNamePtr
)
{
    GT_STATUS              st;
    SKERNEL_DEVICE_OBJECT *deviceObjPtr; /* device object pointer */
    SMAIN_PORT_SLAN_INFO  *slanInfoPtr;  /* slan port entry pointer */
    GT_U32          port = portNum;

    DEVICE_ID_CHECK_MAC(deviceId);
    deviceObjPtr = smemTestDeviceIdToDevPtrConvert(deviceId);

    *portBoundPtr = GT_FALSE;

    /* check for multiport group father device */
    if(deviceObjPtr->shellDevice)
    {
        /* get local dev id and port */
        st = prvConvertLocalDevPort(&deviceId, &port);
        if(GT_OK != st)
        {
            return st;
        }
        /* get device object */
        deviceObjPtr = smemTestDeviceIdToDevPtrConvert(deviceId);
    }

    if((port >= deviceObjPtr->numSlans) ||
       (deviceObjPtr->portsArr[port].state == SKERNEL_PORT_STATE_NOT_EXISTS_E))
    {
        return GT_BAD_PARAM;
    }

    /* get slan info */
    slanInfoPtr = &(deviceObjPtr->portSlanInfo[port]);

    if(slanInfoPtr && (slanInfoPtr->slanName[0] != 0))
    {
        *portBoundPtr = GT_TRUE;

        if(slanNamePtr)
        {
            if(slanMaxLength < 8)
            {
                simForcePrintf("skernelDevPortSlanGet: slanMaxLength [%d] < 8 \n", slanMaxLength);
                return GT_BAD_PARAM;
            }

            strncpy(slanNamePtr,slanInfoPtr->slanName,slanMaxLength);

            slanNamePtr[slanMaxLength-1] = 0;/* terminate the string in case the slanMaxLength was not enough */
        }
    }
    else
    {
        *portBoundPtr = GT_FALSE;
    }

    return GT_OK;
}

/**
* @internal skernelDevPortSlanPrint function
* @endinternal
*
* @brief   The function print the slanName of a port of device
*
* @param[in] deviceId                 - the simulation device Id
* @param[in] portNum                  - the physical port number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad deviceId or portNum
*/
GT_STATUS skernelDevPortSlanPrint
(
    IN  GT_U32                deviceId,
    IN  GT_U32                portNum
)
{
    GT_CHAR slanName[SMAIN_SLAN_NAME_SIZE_CNS];
    GT_STATUS st;
    GT_BOOL isPortBound;

    st = skernelDevPortSlanGet(deviceId, portNum,SMAIN_SLAN_NAME_SIZE_CNS ,&isPortBound , slanName);
    if(GT_OK != st)
    {
        return st;
    }

    if(isPortBound == GT_TRUE)
    {
        simForcePrintf("portNum[%d] with slan[%s] \n", portNum, slanName);
    }

    return GT_OK;
}

/**
* @internal skernelDevSlanPrint function
* @endinternal
*
* @brief   The function print ports slanName of the device
*
* @param[in] deviceId                 - the simulation device Id
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad deviceId
*/
GT_STATUS skernelDevSlanPrint
(
    IN  GT_U32                deviceId
)
{
    SKERNEL_DEVICE_OBJECT *deviceObjPtr; /* device object pointer */
    SKERNEL_DEVICE_OBJECT *currDeviceObjPtr; /* current device object pointer */
    GT_U32  dev,port;                 /* iterators */
    GT_U32  globalPort;

    DEVICE_ID_CHECK_MAC(deviceId);
    deviceObjPtr = smemTestDeviceIdToDevPtrConvert(deviceId);

    simForcePrintf("start device[%d] slan printings \n", deviceId);

    if(deviceObjPtr->shellDevice == GT_TRUE)
    {

        for(dev = 0 ; dev < deviceObjPtr->numOfCoreDevs ; dev++)
        {
            currDeviceObjPtr = deviceObjPtr->coreDevInfoPtr[dev].devObjPtr;
            if(currDeviceObjPtr == NULL)
            {
                continue;
            }

            globalPort =  deviceObjPtr->coreDevInfoPtr[dev].startPortNum;

            for(port = 0 ; port < currDeviceObjPtr->portsNumber ; port++ , globalPort++)
            {
                if(!IS_CHT_VALID_PORT(currDeviceObjPtr,port))
                {
                    continue;
                }

                (void)skernelDevPortSlanPrint(deviceId,globalPort);
            }
        }
    }
    else
    {
        for(port = 0 ; port < deviceObjPtr->portsNumber ; port++)
        {
            if(!IS_CHT_VALID_PORT(deviceObjPtr,port))
            {
                continue;
            }

            (void)skernelDevPortSlanPrint(deviceId,port);
        }
    }

    /* try to print the CPU port (if exists) */
    (void)skernelDevPortSlanPrint(deviceId,PRESTERA_CPU_PORT_CNS);


    simForcePrintf("end device slan printings \n");

    return GT_OK;
}

/**
* @internal skernelFatherDeviceIdFromSonDeviceIdGet function
* @endinternal
*
* @brief   The function convert 'son device id' to 'father deviceId'.
*         for device with no 'father' --> return 'son deviceId'
* @param[in] sonDeviceId              - the simulation device Id of the 'son'
*
* @param[out] fatherDeviceIdPtr        - (pointer to)the simulation device Id of the 'father'
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad sonDeviceId .
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS skernelFatherDeviceIdFromSonDeviceIdGet
(
    IN  GT_U32                sonDeviceId,
    OUT GT_U32               *fatherDeviceIdPtr
)
{
    GT_U32  ii;
    SKERNEL_DEVICE_OBJECT   *sonDeviceObjPtr;
    SKERNEL_DEVICE_OBJECT   *fatherDeviceObjPtr;

    DEVICE_ID_CHECK_MAC(sonDeviceId);
    sonDeviceObjPtr = smemTestDeviceIdToDevPtrConvert(sonDeviceId);

    /* use the father device */
    if(sonDeviceObjPtr->portGroupSharedDevObjPtr == NULL)
    {
        *fatherDeviceIdPtr = sonDeviceId;
        return GT_OK;
    }

    fatherDeviceObjPtr = sonDeviceObjPtr->portGroupSharedDevObjPtr;

    /* find the 'device Id' of the father */
    for(ii = 0 ; ii < SMAIN_MAX_NUM_OF_DEVICES_CNS; ii++)
    {
        if(smainDeviceObjects[ii] == fatherDeviceObjPtr)
        {
            break;
        }
    }

    *fatherDeviceIdPtr = ii;
    return GT_OK;
}


/**
* @internal skernelSupportMacLoopbackInContextOfSender function
* @endinternal
*
* @brief   The function set the device as 'Support Mac Loopback In Context Of Sender'
*         allow the device to do 'external' mac loopback as processing the loop in the
*         context of the sending task (not sending it to the 'buffers pool')
*         the function is for performance issues.
* @param[in] enable                   - the simulation device Id.
*
* @retval GT_OK                    - on success
*
* @note currently implemented only for : Puma3
*
*/
GT_STATUS skernelSupportMacLoopbackInContextOfSender
(
    IN  GT_BOOL                enable
)
{
    supportMacLoopbackInContextOfSender = enable;
    return GT_OK;
}

/**
* @internal skernelCreatePacketGenerator function
* @endinternal
*
* @brief   Create packet generator task
*
* @param[in] deviceObjPtr             - allocated pointer for the device
* @param[in] tgNumber                 - packet generator number
*/
GT_VOID skernelCreatePacketGenerator
(
    IN SKERNEL_DEVICE_OBJECT * deviceObjPtr,
    IN GT_U32       tgNumber
)
{
    SKERNEL_TRAFFIC_GEN_STC * trafficGenDataPtr;/* pointer to packet generator */
    GT_TASK_HANDLE          taskHandle;          /* task handle */

    if(deviceObjPtr->trafficGeneratorSupport.tgSupport)
    {
        deviceObjPtr->smemInitPhaseSemaphore = SIM_OS_MAC(simOsSemCreate)(0,1);

        trafficGenDataPtr =
            &deviceObjPtr->trafficGeneratorSupport.trafficGenData[tgNumber];
        trafficGenDataPtr->deviceObjPtr = deviceObjPtr;
        trafficGenDataPtr->trafficGenNumber = tgNumber;

        taskHandle = SIM_OS_MAC(simOsTaskCreate)(GT_TASK_PRIORITY_ABOVE_NORMAL,
                          (unsigned (__TASKCONV *)(void*))smainPacketGeneratorTask,
                          (void *) trafficGenDataPtr);
        if (taskHandle == NULL)
        {
            skernelFatalError(" skernelInit: cannot create traffic generator "\
                              " task for device %u", deviceObjPtr->deviceId);
        }
        /* wait for semaphore */
        SIM_OS_MAC(simOsSemWait)(deviceObjPtr->smemInitPhaseSemaphore, SIM_OS_WAIT_FOREVER);

        SCIB_SEM_TAKE;
        deviceObjPtr->numThreadsOnMe++;
        SCIB_SEM_SIGNAL;

        /* Set packet generator state in database */
        deviceObjPtr->trafficGeneratorSupport.trafficGenData[tgNumber].trafficGenActive = GT_TRUE;

        SIM_OS_MAC(simOsSemDelete)(deviceObjPtr->smemInitPhaseSemaphore);

        deviceObjPtr->smemInitPhaseSemaphore = (GT_SEM)0;
    }
}

/**
* @internal skernelStatusGet function
* @endinternal
*
* @brief   Get status (Idle or Busy) of all Simulation Kernel tasks.
*
* @retval 0                        - Simulation Kernel Tasks are Idle
* @retval other                    - Simulation Kernel Tasks are busy
*/
GT_U32 skernelStatusGet
(
    void
)
{
    GT_U32      device;     /* device index */
    GT_U32      devStatus = 0;  /* device status */

    SCIB_SEM_TAKE;
    for (device = 0; device < SMAIN_MAX_NUM_OF_DEVICES_CNS; device++)
    {
        /* skip not exist devices */
        if (!smainDeviceObjects[device])
        {
            continue;
        }

        /* skip devices without queues */
        if (smainDeviceObjects[device]->queueId == NULL)
        {
            continue;
        }

        /* get queue status */
        devStatus = squeStatusGet(smainDeviceObjects[device]->queueId);
        if (devStatus != 0)
        {
            /* device is busy */
            break;
        }

        if(smainDeviceObjects[device]->softResetOldDevicePtr)
        {
            /* we are under 'soft reset' :
               the new device already replaced the old device as pointer in
               smainDeviceObjects[device] ... but still not hold
               the 'queue' of the old device  */

            /* device is busy doing soft reset */
            devStatus = 1;
            break;
        }

        if(sRemoteTmUsed && smainDeviceObjects[device]->tmInfo.bufPool_DDR3_TM)
        {
            /* the device is connected to remote TM simulator */
            /* this pool may hold buffers that represents packets that still
               need to egress the device */
            devStatus = sbufAllocatedBuffersNumGet(smainDeviceObjects[device]->tmInfo.bufPool_DDR3_TM);
            if (devStatus != 0)
            {
                /* device is busy */
                break;
            }
        }
    }

    /* all devices are Idle */
    SCIB_SEM_SIGNAL;
    return devStatus;
}

/**
* @internal skernelBitwiseOperator function
* @endinternal
*
* @brief   calculate bitwise operator result:
*         result = (((value1Operator) on value1) value2Operator on value2)
* @param[in] value1                   - value 1
* @param[in] value1Operator           - operator to be done on value 1
*                                      valid are:
*                                      SKERNEL_BITWISE_OPERATOR_NONE_E
*                                      SKERNEL_BITWISE_OPERATOR_NOT_E  (~value1)
* @param[in] value2                   - value 2
* @param[in] value2Operator           - operator to be done on value 1
*                                      valid are:
*                                      SKERNEL_BITWISE_OPERATOR_NONE_E -> (value 2 ignored !)
*                                      SKERNEL_BITWISE_OPERATOR_XOR_E  -> ^ value2
*                                      SKERNEL_BITWISE_OPERATOR_AND_E  -> & value2
*                                      SKERNEL_BITWISE_OPERATOR_OR_E   -> | value2
*                                      SKERNEL_BITWISE_OPERATOR_NOT_E  -> this is 'added' to on of 'xor/or/and'
*                                      -> ^ (~vlaue2)
*                                      -> & (~vlaue2)
*                                      -> | (~vlaue2)
*                                       the operator result
*/
GT_U32  skernelBitwiseOperator
(
    IN GT_U32                       value1,
    IN SKERNEL_BITWISE_OPERATOR_ENT value1Operator,

    IN GT_U32                       value2,
    IN SKERNEL_BITWISE_OPERATOR_ENT value2Operator
)
{
    GT_U32  result;
    GT_U32  useNotOnValue2;
    GT_U32  value1new;

    switch(value1Operator)
    {
        case SKERNEL_BITWISE_OPERATOR_NONE_E :/* no operation */
            value1new = value1;
            break;
        case SKERNEL_BITWISE_OPERATOR_NOT_E :/*NOT ~*/
            value1new = ~value1;
            break;
        default:
            skernelFatalError("skernelBitwiseOperator: value1Operator can not be [%d] \n",
                value1Operator);
            return 0;
    }

    if(value2Operator == SKERNEL_BITWISE_OPERATOR_NOT_E)
    {
        skernelFatalError("skernelBitwiseOperator: value2Operator can not be [%d] \n",
            value2Operator);
        return 0;
    }

    useNotOnValue2 = (value2Operator & SKERNEL_BITWISE_OPERATOR_NOT_E) ? 1 : 0;

    switch(value2Operator & (~SKERNEL_BITWISE_OPERATOR_NOT_E))
    {
        case SKERNEL_BITWISE_OPERATOR_NONE_E :/* no operation */
            result = value1new;
            break;
        case SKERNEL_BITWISE_OPERATOR_XOR_E:
            if(useNotOnValue2)
            {
                result = value1new ^ (~value2);
            }
            else
            {
                result = value1new ^ value2;
            }
            break;
        case SKERNEL_BITWISE_OPERATOR_AND_E:
            if(useNotOnValue2)
            {
                result = value1new & (~value2);
            }
            else
            {
                result = value1new & value2;
            }
            break;
        case SKERNEL_BITWISE_OPERATOR_OR_E :
            if(useNotOnValue2)
            {
                result = value1new | (~value2);
            }
            else
            {
                result = value1new | value2;
            }
            break;
        default:
            skernelFatalError("skernelBitwiseOperator: value2Operator can not be [%d] \n",
                value2Operator);
            result = 0;
            break;
    }


    return result;
}


/**
* @internal skernelDebugFreeBuffersNumPrint function
* @endinternal
*
* @brief   debug function to print the number of free buffers.
*
* @param[in] devNum                   - device number as stated in the INI file.
*                                       None
*/
void skernelDebugFreeBuffersNumPrint
(
    IN  GT_U32    devNum
)
{
    SKERNEL_DEVICE_OBJECT* deviceObjPtr = smemTestDeviceIdToDevPtrConvert(devNum);
    SKERNEL_DEVICE_OBJECT* currDeviceObjPtr;
    GT_U32  dev;
    GT_U32  freeBuffersNum;

    if(deviceObjPtr->shellDevice == GT_TRUE)
    {
        simulationPrintf(" multi-core device [%d] \n",devNum);
        for(dev = 0 ; dev < deviceObjPtr->numOfCoreDevs ; dev++)
        {
            currDeviceObjPtr = deviceObjPtr->coreDevInfoPtr[dev].devObjPtr;
            freeBuffersNum = sbufFreeBuffersNumGet(currDeviceObjPtr->bufPool);

            simulationPrintf(" coreId[%d] with free buffers[%d] \n",dev,freeBuffersNum);
        }
    }
    else
    {
        freeBuffersNum = sbufFreeBuffersNumGet(deviceObjPtr->bufPool);

        simulationPrintf(" device[%d] with free buffers[%d] \n",devNum,freeBuffersNum);
    }



}


/*******************************************************************************
* skernelSleep
*
* DESCRIPTION:
*       Puts current task to sleep for specified number of millisecond.
*       this function needed instead of direct call to SIM_OS_MAC(simOsSleep)
*       because it allow the thread that calls this function to replace the 'old device'
*       with 'new device' as part of 'soft reset' processing.
*
* INPUTS:
*       devObjPtr      - the device that needs the sleep
*       timeInMilliSec - time to sleep in milliseconds
*
* OUTPUTS:
*       None
*
* RETURNS:
*       pointer to device object as the device object may have been replaces due
*       to 'soft reset' at this time
*
* COMMENTS:
*       None
*
*******************************************************************************/
SKERNEL_DEVICE_OBJECT  * skernelSleep
(
    IN SKERNEL_DEVICE_OBJECT  * devObjPtr,
    IN GT_U32                   timeInMilliSec
)
{
    static GT_U32   numTicksFor1000Mili = 0;

    SKERNEL_DEVICE_OBJECT  * newDevObjPtr;
    GT_U32  ii,iiMax,timeModulo,timeMinimal = 100;
    GT_U32                 timeStart,timeEnd;

    if(devObjPtr == NULL)
    {
        skernelFatalError("skernelSleep: can't have NULL device \n");
        return NULL;
    }

    if(timeInMilliSec >= 1000)
    {
        if(numTicksFor1000Mili == 0)
        {
            SCIB_SEM_TAKE;
            if(numTicksFor1000Mili == 0)/*second check inside the 'SCIB LOCK' */
            {
                timeStart = SIM_OS_MAC(simOsTickGet)();
                SIM_OS_MAC(simOsSleep)(1000);
                timeEnd = SIM_OS_MAC(simOsTickGet)();

                numTicksFor1000Mili = timeEnd - timeStart;
                timeInMilliSec -= 1000;
            }
            SCIB_SEM_SIGNAL;


            /*simulationPrintf("numTicksFor1000Mili = [%d] \n",numTicksFor1000Mili);*/

            if(numTicksFor1000Mili == 0)
            {
                skernelFatalError("skernelSleep: can't calc numTicksFor1000Mili \n");
            }

        }
    }


    timeStart = SIM_OS_MAC(simOsTickGet)();
    timeModulo = timeInMilliSec % timeMinimal;
    iiMax = timeInMilliSec / timeMinimal;
    for(ii = 0 ; ii < iiMax; ii++)
    {
        if(devObjPtr->softResetNewDevicePtr)
        {
            timeModulo = 0;/* no need extra sleep as we want to do soft reset ASAP */
            break;
        }

        /* break sleep to not be more than 100 mili every time
           to allow 'soft reset' to modify device pointer */
        SIM_OS_MAC(simOsSleep)(timeMinimal);

        timeEnd = SIM_OS_MAC(simOsTickGet)();
        if(numTicksFor1000Mili && (((ii+1) != iiMax) || timeModulo))/* not last iteration or extra sleep needed after the loop */
        {
            if(((1000* (timeEnd - timeStart)) / numTicksFor1000Mili) > timeInMilliSec)
            {
                /* already sleep enough */
                timeModulo = 0;

#if 0 /*we not want continues printings in WIN32 ... */
                /* NOTE: it seems that in WIN32 we get here when ii is about 90% of iiMax !!! */
                /* NOTE: it seems that Linux is more accurate and we get here exactly at last iteration */
                simulationPrintf("skernelSleep: already sleep[%d]ms needed[%d]ms after[%d] out of[%d]\n",
                    ((1000* (timeEnd - timeStart)) / numTicksFor1000Mili),
                    timeInMilliSec,
                    (ii+1),
                    iiMax);
#endif
                break;
            }
        }
    }

    if(timeModulo)
    {
        SIM_OS_MAC(simOsSleep)(timeModulo);
    }

    /* check if need to replace old device with new device */
    /* are we in the middle of soft reset */
    if(devObjPtr->softResetNewDevicePtr)
    {
        /* see function skernelDeviceSoftResetGeneric(...) that will bind the old
           device with new device */

        newDevObjPtr = devObjPtr->softResetNewDevicePtr;

        SCIB_SEM_TAKE;
        /* modify taskOwnTaskCookie if required */
        {
            GT_CHAR  *orig_cookie;
            GT_CHAR  *new_cookie;
            SIM_OS_TASK_COOKIE_INFO_STC* orig_cookieCastPtr;
            SIM_OS_TASK_COOKIE_INFO_STC* new_cookieCastPtr;

            orig_cookie = (GT_CHAR*)simOsTaskOwnTaskCookieGet();

            orig_cookieCastPtr = (SIM_OS_TASK_COOKIE_INFO_STC*)orig_cookie;

            if (    (orig_cookie >= (GT_CHAR*)devObjPtr) &&
                    (orig_cookie < (GT_CHAR*)(devObjPtr + 1)))
            {
                /* cookie present and points to devObjPtr */
                /* modify cookie to new value */
                new_cookie = orig_cookie - ((GT_CHAR*)devObjPtr - (GT_CHAR*)newDevObjPtr);

                new_cookieCastPtr = (SIM_OS_TASK_COOKIE_INFO_STC*)new_cookie;

                /* copy the content of the cookie from the orig device */
                *new_cookieCastPtr = *orig_cookieCastPtr;

                SIM_OS_MAC(simOsTaskOwnTaskPurposeSet)(
                    SIM_OS_TASK_PURPOSE_TYPE_PP_PIPE_GENERAL____LAST__E,/* indication to modify only the cookie and not the 'purpose'*/
                    new_cookieCastPtr);
            }
        }
        SCIB_SEM_SIGNAL;

        SCIB_SEM_TAKE;
        devObjPtr->numThreadsOnMe--;
        SCIB_SEM_SIGNAL;

        while(newDevObjPtr->softResetOldDevicePtr)
        {
            /* wait for the new device to be fully ready (and detached from info from the old device) */
            SIM_OS_MAC(simOsSleep)(50);
            simulationPrintf("-");
        }

        /* return the new device */
        return newDevObjPtr;
    }

    /* return original device */
    return devObjPtr;
}


/**
* @internal skernelSoftResetTest function
* @endinternal
*
* @brief   debug function to test the 'soft reset'
*         function valid for SIP5 and above
* @param[in] devNum                   - device number as stated in the INI file.
*                                       None
*/
void skernelSoftResetTest
(
    IN  GT_U32    devNum
)
{
    SKERNEL_DEVICE_OBJECT* oldDeviceObjPtr = smemTestDeviceIdToDevPtrConvert(devNum);
    SKERNEL_DEVICE_OBJECT* newDeviceObjPtr;
    GT_U32                 timeStart,timeEnd;
    GT_U32                 regValue;
    GT_U32                 regAddr;
    GT_U32                 regAddrArray[5];
    GT_U32                 ii,iiMax = 5;
    CHT_AUQ_MEM   * auqMemPtr , origAuq;
    CHT2_FUQ_MEM  * fuqMemPtr;
    GT_U32                  dfxServerResetControlRegisterValue;
    GT_U32                  globalInterruptMaskRegRegAddr = 0x34;
    GT_U32                  numErrors = 0;

    /* disable the device from sending interrupts to the CPU
       NOTE: we will not need to restore it because no meaning to restore it
       without FULL interrupts tree.
    */
    regValue = 0;
    scibWriteMemory(devNum,globalInterruptMaskRegRegAddr,1,&regValue);

    auqMemPtr = SMEM_CHT_MAC_AUQ_MEM_GET(oldDeviceObjPtr);
    origAuq = *auqMemPtr;
    fuqMemPtr = SMEM_CHT_MAC_FUQ_MEM_GET(oldDeviceObjPtr);

    regAddr = SMEM_LION2_DFX_SERVER_RESET_CONTROL_REG(oldDeviceObjPtr);

    scibMemoryClientRegRead(devNum,SCIB_MEM_ACCESS_DFX_E,regAddr,1,&dfxServerResetControlRegisterValue);
    /*<MG Soft Reset Trigger> --> should be 1 'NO trigger' */
    if(SMEM_U32_GET_FIELD(dfxServerResetControlRegisterValue,1,1) == 0)
    {
        skernelFatalError("skernelSoftResetTest: <MG Soft Reset Trigger> --> should be 1 ('NO trigger') \n");
    }

    /* write to registers values before the soft reset */
    ii = 0;
    /* MG register */
    regAddrArray[ii++] = (SMEM_CHT_MAC_REG_DB_GET(oldDeviceObjPtr))->MG.interruptCoalescingConfig.interruptCoalescingConfig;
    /* TTI register */
    regAddrArray[ii++] = SMEM_LION3_TTI_INGR_TPID_TAG_TYPE_REG(oldDeviceObjPtr);
    /* L2i register */
    regAddrArray[ii++] = SMEM_LION3_L2I_BRIDGE_ACCESS_MATRIX_DEFAULT_REG(oldDeviceObjPtr);
    /* EQ register */
    regAddrArray[ii++] = SMEM_CHT2_IP_PROT_CPU_CODE_VALID_CONF_REG(oldDeviceObjPtr);
    /* TXQ register */
    regAddrArray[ii++] = SMEM_LION_STAT_EGRESS_MIRROR_REG(oldDeviceObjPtr,0);

    /* set the registers with value of their addresses */
    for(ii = 0 ; ii < iiMax ; ii ++)
    {
        /* check content of the register that it changed */
        regValue = regAddrArray[ii];
        scibWriteMemory(devNum,regAddrArray[ii],1,&regValue);
    }

    /* check content of the registers before the soft reset */
    for(ii = 0 ; ii < iiMax ; ii ++)
    {
        scibReadMemory(devNum,regAddrArray[ii],1,&regValue);

        if(regValue != regAddrArray[ii])
        {
            simulationPrintf(" the register ii=[%d] in address [0x%8.8x] not changed value to [0x%8.8x] before the 'soft reset' \n",
                ii,regAddrArray[ii],regValue);
            numErrors ++;
        }
    }

    /* take the time before the reset */
    timeStart = SIM_OS_MAC(simOsTickGet)();

    /*********************/
    /* trigger the reset */
    /*********************/

    /*<MG Soft Reset Trigger> --> set to 0 to trigger it */
    dfxServerResetControlRegisterValue &= ~ (1<<1);
    scibMemoryClientRegWrite(devNum,SCIB_MEM_ACCESS_DFX_E,regAddr,1,&dfxServerResetControlRegisterValue);

    oldDeviceObjPtr = NULL;/* stop using the old device */
#ifndef _WIN32 /* it seems that linux miss something */
    SIM_OS_MAC(simOsSleep)(100);
#endif /*_WIN32*/

    /********************************/
    /* wait until the reset is done */
    /********************************/
    while(skernelStatusGet())
    {
        SIM_OS_MAC(simOsSleep)(10);
        simulationPrintf("7");
    }


    /* take the time that reset took */
    timeEnd = SIM_OS_MAC(simOsTickGet)();

    simulationPrintf("\n");
    simulationPrintf("skernelSoftResetTest : start tick[%d] , end tick[%d] --> total ticks[%d] \n",
        timeStart,timeEnd,timeEnd-timeStart);

    /* get pointer to the new device */
    newDeviceObjPtr = smemTestDeviceIdToDevPtrConvert(devNum);

    /* check content of the registers that it changed */
    for(ii = 0 ; ii < iiMax ; ii ++)
    {
        scibReadMemory(devNum,regAddrArray[ii],1,&regValue);

        if(regValue == regAddrArray[ii])
        {
            simulationPrintf(" the register ii=[%d] in address [0x%8.8x] not changed value after the 'soft reset' \n",
                ii,regAddrArray[ii]);
            numErrors ++;
        }
    }

    /* check the AUQ that done reset and the FUQ */
    auqMemPtr = SMEM_CHT_MAC_AUQ_MEM_GET(newDeviceObjPtr);
    fuqMemPtr = SMEM_CHT_MAC_FUQ_MEM_GET(newDeviceObjPtr);

    if(origAuq.auqBaseValid)    /* the device did CPSS initialization */
    {
        if(
            auqMemPtr->auqBase              ||
            auqMemPtr->auqBaseValid         ||
            auqMemPtr->auqBaseSize          ||
            auqMemPtr->auqShadow            ||
            auqMemPtr->auqShadowValid       ||
            auqMemPtr->auqShadowSize        ||
            auqMemPtr->auqOffset            ||
            auqMemPtr->baseInit
        )
        {
            simulationPrintf(" AUQ was not reset properly \n ");
            numErrors ++;
        }

        if(
            fuqMemPtr->fuqBase              ||
            fuqMemPtr->fuqBaseValid         ||
            fuqMemPtr->fuqShadow            ||
            fuqMemPtr->fuqShadowValid       ||
            fuqMemPtr->fuqOffset            ||
            fuqMemPtr->baseInit             ||
            fuqMemPtr->fuqNumMessages
        )
        {
            simulationPrintf(" FUQ was not reset properly \n ");
            numErrors ++;
        }
    }

    /* just for the test :  make sure that interrupts will not cause any issue */
    regValue = 0xFFFFFF;
    scibWriteMemory(devNum,globalInterruptMaskRegRegAddr,1,&regValue);


    if(numErrors == 0)
    {
        simulationPrintf("\n\n\n **** TEST PASS :-) **** \n\n\n");
    }
    else
    {
        simulationPrintf("\n\n\n **** TEST FAILED :-( **** \n\n\n");
    }
}


/**
* @internal skernelSoftResetTest_multiple function
* @endinternal
*
* @brief   multiple run of test : debug function to test the 'soft reset'
*         function valid for SIP5 and above
* @param[in] devNum                   - device number as stated in the INI file.
* @param[in] numOfRepetitions         - number of repetitions.
*                                       None
*/
void skernelSoftResetTest_multiple
(
    IN  GT_U32    devNum,
    IN GT_U32   numOfRepetitions
)
{
    GT_U32  ii;

    if(numOfRepetitions == 0)
    {
        numOfRepetitions = 1;
    }

    for(ii = 0 ; ii < numOfRepetitions ; ii++ )
    {
        skernelSoftResetTest(devNum);
        simulationPrintf("test[%d] ended \n",ii+1);
    }

    return;
}

/**
* @internal smainReceivedPacketDoneFromTm function
* @endinternal
*
* @brief   'callback' function that will be called when the 'packet done' message
*         is returned from the TM (for specific packetId).
* @param[in] simDeviceId              - device id.
* @param[in] cookiePtr                - the cookie that was attached to the packet
* @param[in] tmFinalPort              - the TM final port
*
* @note the function is called from 'UDP socket listener' task context.
*       so the CB function must not to 'sleep' or 'heavy' operations
*
*/
static void smainReceivedPacketDoneFromTm
(
    IN GT_U32   simDeviceId,
    IN GT_VOID* cookiePtr,
    IN GT_U32   tmFinalPort
)
{
    SKERNEL_DEVICE_OBJECT   *devObjPtr;
    SBUF_BUF_ID             bufferId;    /* buffer id */
    GT_U32                  messageSize;   /*message size*/
    GT_U8  * dataPtr;         /* pointer to the data in the buffer */
    GT_U32 dataSize;          /* data size */
    GENERIC_MSG_FUNC genFunc = snetChtEgressAfterTmUnit;/* generic function */

    /* the sender that built this cookie is function startProcessInTmUnit(...) */
    /* we need to here the same logic */
    devObjPtr = smemTestDeviceIdToDevPtrConvert(simDeviceId);

    /* send the device a message to finish processing the packet that returned
       from the TM unit */

    messageSize = sizeof(GENERIC_MSG_FUNC) + sizeof(cookiePtr) + sizeof(tmFinalPort);

    /* allocate buffer from the 'destination' device pool */
    /* get the buffer and put it in the queue */
    bufferId = sbufAlloc(devObjPtr->bufPool, messageSize);
    if (bufferId == NULL)
    {
        simWarningPrintf(" smainReceivedPacketDoneFromTm : no buffers for process \n");
        return ;
    }

    /* Get actual data pointer */
    sbufDataGet(bufferId, (GT_U8 **)&dataPtr, &dataSize);

    /* put the name of the function into the message */
    memcpy(dataPtr,&genFunc,sizeof(GENERIC_MSG_FUNC));
    dataPtr+=sizeof(GENERIC_MSG_FUNC);

    /* save parameter 1 */
    memcpy(dataPtr,&cookiePtr,sizeof(cookiePtr));
    dataPtr+=sizeof(cookiePtr);
    /* save parameter 2 */
    memcpy(dataPtr,&tmFinalPort,sizeof(tmFinalPort));
    dataPtr+=sizeof(tmFinalPort);

    /* set source type of buffer                    */
    bufferId->srcType = SMAIN_SRC_TYPE_CPU_E;

    /* set message type of buffer                   */
    bufferId->dataType = SMAIN_MSG_TYPE_GENERIC_FUNCTION_E;

    /* put buffer to queue                          */
    squeBufPut(devObjPtr->queueId, SIM_CAST_BUFF(bufferId));
}

/**
* @internal smainEmulateRemoteTm function
* @endinternal
*
* @brief   debug tool to be able to emulte remote TM for basic testing of
*         simulation that works with remote TM (udp sockets and asynchronous
*         messages).
*/
void smainEmulateRemoteTm
(
    IN GT_U32  emulate
)
{
    emulateRemoteTm = emulate;
}

/**
* @internal smainIsEmulateRemoteTm function
* @endinternal
*
* @brief   debug tool to be able to know if we emulte remote TM for basic testing of
*         simulation that works with remote TM (udp sockets and asynchronous
*         messages).
*/
GT_U32 smainIsEmulateRemoteTm
(
    void
)
{
    return emulateRemoteTm;
}


/**
* @internal smainRemoteTmConnect function
* @endinternal
*
* @brief   create the needed sockets for TM device.
*         using UDP sockets (as used by the TM device)
*         and wait for the 'remote TM' to be ready.
* @param[in] simDeviceId              - Simulation device ID.
*/
void smainRemoteTmConnect
(
    IN GT_U32   simDeviceId
)
{
    SKERNEL_DEVICE_OBJECT   *devObjPtr;
    devObjPtr = smemTestDeviceIdToDevPtrConvert(simDeviceId);

    if(devObjPtr->supportTrafficManager == 0)
    {
        skernelFatalError("sMainRemoteTmConnect: device not supports traffic manager \n");
    }

    /* call to create sockets on 'our size' (as clients) */
    sRemoteTmCreateUdpSockets(devObjPtr->deviceId);

    if(emulateRemoteTm)
    {
        /* open 'server' sockets as emulation to the remote TM , with dummy implementations */
        sRemoteTmTest_emulateDummyTmDevice();
    }
    else
    {
        /* wait for the TM server to be ready */
        sRemoteTmWaitForTmServer(simDeviceId);
    }

}

/**
* @internal smainGetDevicesString function
* @endinternal
*
* @brief   function to return the names of the devices.
*
* @param[in] devicesNamesString       - (pointer to) the place to set the names of devices
* @param[in] sizeOfString             - number of bytes in devicesNamesString.
*/
void smainGetDevicesString
(
    IN char   *devicesNamesString,
    IN GT_U32   sizeOfString
)
{
    static char fullStringName[SIM_OS_CNF_FILE_MAX_LINE_LENGTH_CNS];
    static char tmp[SIM_OS_CNF_FILE_MAX_LINE_LENGTH_CNS];
    GT_U32      device;     /* device index */
    SKERNEL_DEVICE_OBJECT * devObjPtr;
    char*      deviceNamePtr;
    char*      revisionNamePtr;
    GT_U8       deviceRevisionId;
    GT_U32      offset = 0;
    GT_U32      instance = 0;
    GT_U32      ii;
    GT_U32      numOfCoreDevs;
    GT_BOOL     gmDevice;/* is GM device*/

    fullStringName[0] = 0;

    for (device = 0; device < SMAIN_MAX_NUM_OF_DEVICES_CNS; device++)
    {
        /* skip not exist devices */
        if (!smainDeviceObjects[device])
        {
            continue;
        }

        devObjPtr = smainDeviceObjects[device];
        if(devObjPtr->shellDevice == GT_TRUE)
        {
        }
        else if(devObjPtr->portGroupSharedDevObjPtr)/* this is core in multi-core device */
        {
            /* we represent the device by it's 'father' */
            continue;
        }
        else if(devObjPtr->deviceFamily == SKERNEL_NIC_FAMILY)
        {
            /* not really device */
            continue;
        }

        instance++;
    }



    instance = 0;

    for (device = 0; device < SMAIN_MAX_NUM_OF_DEVICES_CNS; device++)
    {
        /* skip not exist devices */
        if (!smainDeviceObjects[device])
        {
            continue;
        }

        devObjPtr = smainDeviceObjects[device];
        numOfCoreDevs = devObjPtr->numOfCoreDevs;

        if(devObjPtr->shellDevice == GT_TRUE)
        {
            /* get one of the cores */
            for(ii = 0 ; ii < devObjPtr->numOfCoreDevs ; ii++)
            {
                if(devObjPtr->coreDevInfoPtr[ii].devObjPtr)
                {
                    devObjPtr = devObjPtr->coreDevInfoPtr[ii].devObjPtr;
                    break;
                }
            }
        }
        else if(devObjPtr->portGroupSharedDevObjPtr)/* this is core in multi-core device */
        {
            /* we represent the device by it's 'father' */
            continue;
        }
        else if(devObjPtr->deviceFamily == SKERNEL_NIC_FAMILY)
        {
            /* not really device */
            continue;
        }

        deviceNamePtr = NULL;
        revisionNamePtr = NULL;
        deviceRevisionId = devObjPtr->deviceRevisionId;
        gmDevice = (devObjPtr->gmDeviceType == SMAIN_NO_GM_DEVICE) ? GT_FALSE : GT_TRUE;

        switch(devObjPtr->deviceFamily)
        {
            case    SKERNEL_SALSA_FAMILY:                        deviceNamePtr =  "salsa"        ;  break;
            case    SKERNEL_NIC_FAMILY:                          deviceNamePtr =  "nic"          ;  break;
            case    SKERNEL_TWIST_D_FAMILY:                      deviceNamePtr =  "twist_d"      ;  break;
            case    SKERNEL_TWIST_C_FAMILY:                      deviceNamePtr =  "twist_c"      ;  break;
            case    SKERNEL_SAMBA_FAMILY:                        deviceNamePtr =  "samba"        ;  break;
            case    SKERNEL_SOHO_FAMILY:
                if(SKERNEL_DEVICE_FAMILY_SOHO_PERIDOT(devObjPtr->deviceType))
                {
                    deviceNamePtr =  "88e1690";
                }
                else
                {
                    deviceNamePtr =  "soho";
                }

                break;
            case    SKERNEL_CHEETAH_1_FAMILY:                    deviceNamePtr =  "cheetah_1"    ;  break;
            case    SKERNEL_CHEETAH_2_FAMILY:                    deviceNamePtr =  "cheetah_2"    ;  break;
            case    SKERNEL_CHEETAH_3_FAMILY:                    deviceNamePtr =  "cheetah_3"    ;  break;
            case    SKERNEL_LION_PORT_GROUP_FAMILY:
                    deviceNamePtr =  "lion"         ;
                    revisionNamePtr = deviceRevisionId == 0 ? "A0 -- NOT supported !!!!":
                                      deviceRevisionId == 2 ? "B0" :
                                      NULL;
                    break;

            case    SKERNEL_XCAT_FAMILY:                         deviceNamePtr =  "xcat"         ;  break;
            case    SKERNEL_XCAT2_FAMILY:                        deviceNamePtr =  "xcat2"        ;  break;
            case    SKERNEL_TIGER_FAMILY:                        deviceNamePtr =  "tiger"        ;  break;
            case    SKERNEL_FA_FOX_FAMILY:                       deviceNamePtr =  "fa_fox"       ;  break;
            case    SKERNEL_FAP_DUNE_FAMILY:                     deviceNamePtr =  "fap_dune"     ;  break;
            case    SKERNEL_XBAR_CAPOEIRA_FAMILY:                deviceNamePtr =  "xbar_capoeira";  break;
            case    SKERNEL_FE_DUNE_FAMILY:                      deviceNamePtr =  "fe_dune"      ;  break;
            case    SKERNEL_PUMA_FAMILY:                         deviceNamePtr =  "puma"         ;  break;
            case    SKERNEL_EMBEDDED_CPU_FAMILY:                 deviceNamePtr =  "embedded_cpu" ;  break;

            case    SKERNEL_PHY_SHELL_FAMILY   :
            case    SKERNEL_PHY_CORE_FAMILY:
                                                                 deviceNamePtr =  "phy_shell"    ;  break;


            case    SKERNEL_MACSEC_FAMILY:                       deviceNamePtr =  "macsec"       ;  break;

            case    SKERNEL_LION2_PORT_GROUP_FAMILY:
                deviceNamePtr =  numOfCoreDevs == 8 ? "lion2" :
                                 numOfCoreDevs == 4 ? "hooper" :
                                 "lion2(unknown)";
                if(numOfCoreDevs == 8)
                {
                    revisionNamePtr = deviceRevisionId == 0 ? "A0 -- NOT supported !!!!" :
                                      deviceRevisionId == 1 ? "B0" :
                                      deviceRevisionId == 2 ? "B1" :
                                      NULL;
                }
                else/* Hooper */
                {
                    revisionNamePtr = deviceRevisionId == 0 ? "A0" :
                                      NULL;
                }

                break;


            case    SKERNEL_LION3_PORT_GROUP_FAMILY:             deviceNamePtr =  "lion3"        ;  break;


            case    SKERNEL_PUMA3_NETWORK_FABRIC_FAMILY:         deviceNamePtr =  "puma3"        ;  break;


            case    SKERNEL_BOBCAT2_FAMILY:
                deviceNamePtr =  "Bobcat2"      ;
                revisionNamePtr = deviceRevisionId == 0 ? "A0" :
                                  deviceRevisionId == 1 ? "B0" :
                                  NULL;
                break;
            case SKERNEL_BOBK_CAELUM_FAMILY:
            case SKERNEL_BOBK_CETUS_FAMILY:
            case SKERNEL_BOBK_ALDRIN_FAMILY:
                if (devObjPtr->deviceFamily == SKERNEL_BOBK_CAELUM_FAMILY)
                {
                    deviceNamePtr =  "Caelum";
                }
                else
                {
                    if (devObjPtr->deviceFamily == SKERNEL_BOBK_CETUS_FAMILY)
                    {
                        deviceNamePtr =  "Cetus";
                    }
                    else
                    {
                        deviceNamePtr =  "Aldrin";
                    }
                }
                revisionNamePtr = deviceRevisionId == 0 ? "A0" :
                                  deviceRevisionId == 1 ? "B0" :
                                  NULL;
                break;
            case SKERNEL_AC3X_FAMILY:
                deviceNamePtr =  "AC3X";
                revisionNamePtr = deviceRevisionId == 0 ? "A0" :
                                  deviceRevisionId == 1 ? "B0" :
                                  NULL;
                break;
            case    SKERNEL_BOBCAT3_FAMILY:
                deviceNamePtr =  "Bobcat3"      ;
                revisionNamePtr = deviceRevisionId == 0 ? "A0" :
                                  deviceRevisionId == 1 ? "B0" :
                                  NULL;
                break;
            case    SKERNEL_ALDRIN2_FAMILY:
                deviceNamePtr =  "Aldrin2"      ;
                revisionNamePtr = deviceRevisionId == 0 ? "A0" :
                                  deviceRevisionId == 1 ? "B0" :
                                  NULL;
                break;

            case    SKERNEL_PIPE_FAMILY:
                deviceNamePtr =  "Pipe"      ;
                revisionNamePtr = deviceRevisionId == 0 ? "A0" :
                                  deviceRevisionId == 1 ? "A1" :
                                  NULL;
                break;
            case    SKERNEL_FALCON_FAMILY:
                deviceNamePtr =  "Falcon"      ;
                revisionNamePtr = deviceRevisionId == 0 ? "A0" :
                                  deviceRevisionId == 1 ? "B0" :
                                  NULL;
                break;
            case    SKERNEL_HAWK_FAMILY:
                deviceNamePtr =  "Hawk"      ;
                revisionNamePtr = deviceRevisionId == 0 ? "A0" :
                                  deviceRevisionId == 1 ? "B0" :
                                  NULL;
                break;
            case    SKERNEL_PHOENIX_FAMILY:
                deviceNamePtr =  "Phoenix"      ;
                revisionNamePtr = deviceRevisionId == 0 ? "A0" :
                                  deviceRevisionId == 1 ? "B0" :
                                  NULL;
                break;
            case    SKERNEL_HARRIER_FAMILY:
                if(0 == strcmp(devObjPtr->deviceName,"aldrin3m"))
                {
                    deviceNamePtr =  "Aldrin3M"     ;
                }
                else
                {
                    deviceNamePtr =  "Harrier"      ;
                }
                revisionNamePtr = deviceRevisionId == 0 ? "A0" :
                                  deviceRevisionId == 1 ? "B0" :
                                  NULL;
                break;
            case    SKERNEL_IRONMAN_FAMILY:
                deviceNamePtr =  devObjPtr->deviceType == SKERNEL_IRONMAN_S ? "Ironman-S" : "Ironman-L";
                revisionNamePtr = deviceRevisionId == 0 ? "A0" :
                                  deviceRevisionId == 1 ? "B0" :
                                  NULL;
                break;
            default:
                break;
        }

        if(deviceNamePtr == NULL)
        {
            deviceNamePtr = devObjPtr->deviceName;
        }

        if(deviceNamePtr == NULL)
        {
            continue;
        }

        if(revisionNamePtr == NULL && deviceRevisionId == 0)
        {
            revisionNamePtr = "";/*no revision indication*/
        }

        if(revisionNamePtr)
        {
            sprintf(tmp,"%s{%s %s}",
                gmDevice == GT_TRUE ? "GM" : "" ,
                deviceNamePtr,
                revisionNamePtr);
        }
        else
        {
            sprintf(tmp,"%s{%s rev %d}",
                gmDevice == GT_TRUE ? "GM" : "" ,
                deviceNamePtr,
                deviceRevisionId);
        }

        if((strlen(tmp) + offset + 2) >= SIM_OS_CNF_FILE_MAX_LINE_LENGTH_CNS)
        {
            /* can not add this name .. too long */
            break;
        }

        if(instance)
        {
            /* add "," */
            sprintf(fullStringName+offset,",%s",tmp);
        }
        else
        {
            /* add ":" */
            sprintf(fullStringName+offset,":%s",tmp);
        }

        /* re-calc the offset to end of string */
        offset = (GT_U32)strlen(fullStringName);

        instance++;


    }

    if(offset < sizeOfString)
    {
        strcpy(devicesNamesString,fullStringName);
    }
    else
    {
        strncpy(devicesNamesString,fullStringName,sizeOfString-1);

        devicesNamesString[sizeOfString-1] = 0;
    }


}

/**
* @internal skernelRegister_0x4c_Update function
* @endinternal
*
* @brief   Update the value of the <DeviceID> and <RevisionID> in register 0x4C.
*
* @param[in] devObjPtr                - the device object
* @param[in] DeviceID_field           - 16 bits value of <DeviceID>.
*                                      value SMAIN_NOT_VALID_CNS means ignored
* @param[in] RevisionID_field         - 4 bits value of <RevisionID>
*                                      value SMAIN_NOT_VALID_CNS means ignored
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS skernelRegister_0x4c_Update
(
    IN SKERNEL_DEVICE_OBJECT   *devObjPtr,
    IN GT_U32                  DeviceID_field,
    IN GT_U32                  RevisionID_field
)
{
    GT_U32  regAddr;

    if(DeviceID_field != SMAIN_NOT_VALID_CNS)
    {
        if(DeviceID_field > 0xFFFF)
        {
            simForcePrintf("<DeviceID> limited to 16 bits but got [0x%8.8x]\n",DeviceID_field);
            return GT_FAIL;
        }
    }
    if(RevisionID_field != SMAIN_NOT_VALID_CNS)
    {
        if(RevisionID_field > 0xF)
        {
            simForcePrintf("<RevisionID> limited to 4 bits but got [0x%8.8x]\n",RevisionID_field);
            return GT_FAIL;
        }
    }

    regAddr = SMEM_CHT_DEVICE_ID_REG(devObjPtr); /* 0x4C */

    if (DeviceID_field != SMAIN_NOT_VALID_CNS)
    {
        smemRegFldSet(devObjPtr,regAddr,4,16,DeviceID_field);
        simForcePrintf("<DeviceID> was set to [0x%4.4x]\n",DeviceID_field);
    }

    if (RevisionID_field != SMAIN_NOT_VALID_CNS)
    {
        smemRegFldSet(devObjPtr,regAddr,0,4,RevisionID_field);
        simForcePrintf("<RevisionID> was set to [0x%1.1x]\n",RevisionID_field);

        devObjPtr->deviceRevisionId = RevisionID_field;
    }

    /* write the info to the DFX too */
    if(SMEM_CHT_IS_DFX_SERVER(devObjPtr))
    {
        regAddr = SMEM_LION2_DFX_SERVER_JTAG_DEVICE_ID_STATUS_REG(devObjPtr);
        if (DeviceID_field != SMAIN_NOT_VALID_CNS)
        {
            smemDfxRegFldSet(devObjPtr, regAddr, 12, 16, DeviceID_field);
        }
        if (RevisionID_field != SMAIN_NOT_VALID_CNS)
        {
            smemDfxRegFldSet(devObjPtr, regAddr, 28, 4, RevisionID_field);
        }
    }

    /* write the info to the PEX/PCI too */
    if(devObjPtr->isPciCompatible)
    {
        if (DeviceID_field != SMAIN_NOT_VALID_CNS)
        {
            regAddr = 0x0;
            smemPciRegFldSet(devObjPtr,regAddr,16,16,DeviceID_field);
        }

        if (RevisionID_field != SMAIN_NOT_VALID_CNS)
        {
            regAddr = 0x8;
            smemPciRegFldSet(devObjPtr,regAddr,0,4,RevisionID_field);
        }
    }


    return GT_OK;
}

/**
* @internal skernelBusInterfaceGet function
* @endinternal
*
* @brief   get the bus interface(s) that the device is connected to.
*
* @param[in] devObjPtr                - the device object
* @param[out] interfaceBmpPtr         - (pointer to)the BMP of interfaces that
*                                        the device connected to
*                                       (combination of :SCIB_BUS_INTERFACE_PEX ,
*                                       SCIB_BUS_INTERFACE_SMI,
*                                       SCIB_BUS_INTERFACE_SMI_INDIRECT)
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS skernelBusInterfaceGet
(
    IN SKERNEL_DEVICE_OBJECT  *devObjPtr,
    OUT GT_U32                *interfaceBmpPtr
)
{
    *interfaceBmpPtr =  devObjPtr->interfaceBmp;
    return GT_OK;
}

/**
* @internal skernelPortConnectionInfo function
* @endinternal
*
* @brief   WM function to state the port connection info.
*
* @param[in] devObjPtr               - (pointer to) the device object
* @param[in] portNum                 - the WM device ingress port number (MAC number).
* @param[in] infoPtr                 - (pointer to) info about the port connection
*
*/
static void skernelPortLinkSet
(
    IN SKERNEL_DEVICE_OBJECT        *devObjPtr,
    IN GT_U32                       portNum,
    IN GT_U32                       link,
    IN GT_BOOL                      allowSlan,
    IN GT_U32                       allowPrint
)
{
    SMAIN_PORT_SLAN_INFO *slanInfoPrt;          /* slan port entry pointer */

    /* get slan info */
    slanInfoPrt = &devObjPtr->portSlanInfo[portNum];

    devObjPtr->portsArr[portNum].usedByOtherConnection = link;

    if(!allowSlan || !slanInfoPrt->slanIdRx)
    {
        if(slanInfoPrt->slanIdRx)
        {
            if(allowPrint)simGeneralPrintf("unbind the port[%d] from SLAN \n",portNum);

            SIM_OS_MAC(simOsSemWait)(slanDownSemaphore,SIM_OS_WAIT_FOREVER);
            slanInfoPrt->slanName[0] = 0; /* before the unbind , make sure no one try to send packet to this port */
            SIM_OS_MAC(simOsSemSignal)(slanDownSemaphore);
            /* allow all other tasks that send traffic to not be inside the SLAN that going to be unbound */
            SIM_OS_MAC(simOsSleep)(1);
            /* unbind the port from SLAN */
            internalSlanUnbind(slanInfoPrt);
        }

        if(allowPrint)simGeneralPrintf("set port[%d] as %s connected to outer cable \n",
            portNum,
            link?"":"not");
        /* ignore the SLAN or there is no SLAN */
        /* so the caller force the link up/down*/
        devObjPtr->portsArr[portNum].linkStateWhenNoForce =
            link ?
            SKERNEL_PORT_NATIVE_LINK_UP_E :
            SKERNEL_PORT_NATIVE_LINK_DOWN_E;
        /* notify that the port is up/down */
        snetLinkStateNotify(devObjPtr, portNum, link);
    }
    else
    {
        /* the SLAN already took care of the link indication */
    }

    return;
}

/**
* @internal internal_skernelPortConnectionInfo function
* @endinternal
*
* @brief   WM function to state the port connection info.
*
* @param[in] devObjPtr               - (pointer to) the device object
* @param[in] portNum                 - the WM device ingress port number (MAC number).
* @param[in] infoPtr                 - (pointer to) info about the port connection
*
*/
static WM_STATUS internal_skernelPortConnectionInfo
(
    IN SKERNEL_DEVICE_OBJECT        *devObjPtr,
    IN GT_U32                       portNum,
    IN WM_PORT_CONNECTION_INFO_STC  *infoPtr,
    IN GT_U32                       allowPrint
)
{
    SKERNEL_DEVICE_OBJECT        *peerDevObjPtr;
    GT_U32  peerPortNum;

    if(infoPtr == NULL)
    {
        if(allowPrint)simWarningPrintf("skernelPortConnectionInfo : ERROR infoPtr is NULL pointer \n");
        return WM_ERROR;
    }

    if(portNum >= devObjPtr->portsNumber)
    {
        if(allowPrint)simWarningPrintf("skernelPortConnectionInfo : ERROR portNum[%d] >= max[%d] \n",
            portNum, devObjPtr->portsNumber);
        return WM_ERROR;
    }

    if(devObjPtr->portsArr[portNum].state == SKERNEL_PORT_STATE_NOT_EXISTS_E)
    {
        if(allowPrint)simWarningPrintf("skernelPortConnectionInfo : ERROR portNum[%d] NOT exists (not valid MAC number) \n",
            portNum);
        return WM_ERROR;
    }

    if(infoPtr->connectionType == WM_PORT_CONNECTION_TYPE_NONE_E ||
       infoPtr->connectionType == WM_PORT_CONNECTION_TYPE_CALLBACK_E )
    {
        if(GT_TRUE == devObjPtr->portsArr[portNum].peerInfo.usePeerInfo)
        {
            peerPortNum   = devObjPtr->portsArr[portNum].peerInfo.peerPortNum;
            peerDevObjPtr = smemTestDeviceIdToDevPtrConvert(devObjPtr->portsArr[portNum].peerInfo.peerDeviceId);

            skernelPortLinkSet(peerDevObjPtr,peerPortNum,0/*link-Down*/,infoPtr->allowSlan,allowPrint);

            devObjPtr->portsArr[portNum].peerInfo.usePeerInfo = GT_FALSE;
            peerDevObjPtr->portsArr[peerPortNum].peerInfo.usePeerInfo = GT_FALSE;
        }

        if(infoPtr->connectionType == WM_PORT_CONNECTION_TYPE_CALLBACK_E)
        {
            skernelPortLinkSet(devObjPtr,portNum,1/*link-Up*/,infoPtr->allowSlan,allowPrint);
        }
        else /*WM_PORT_CONNECTION_TYPE_NONE_E*/
        {
            skernelPortLinkSet(devObjPtr,portNum,0/*link-Down*/,infoPtr->allowSlan,allowPrint);
        }
        devObjPtr->portsArr[portNum].forbidSlan = infoPtr->allowSlan ? 0 : 1;
    }
    else
    if(infoPtr->connectionType == WM_PORT_CONNECTION_TYPE_PEER_PORT_E)
    {
        peerPortNum = infoPtr->peerInfo.portNum;

        /* check the existence of the peer {dev,port} */
        peerDevObjPtr = smemTestDeviceIdToDevPtrConvert(infoPtr->peerInfo.wmDeviceId);

        if(peerDevObjPtr == devObjPtr && peerPortNum == portNum)
        {
            if(allowPrint)simWarningPrintf("skernelPortConnectionInfo : ERROR 'peer port' wmDeviceId[%d] , portNum[%d] can not be the same as the 'orig port' \n",
                infoPtr->peerInfo.wmDeviceId , portNum);
            return WM_ERROR;
        }

        if(peerPortNum >= peerDevObjPtr->portsNumber)
        {
            if(allowPrint)simWarningPrintf("skernelPortConnectionInfo : ERROR peer portNum[%d] >= max[%d] \n",
                peerPortNum, peerDevObjPtr->portsNumber);
            return WM_ERROR;
        }

        if(peerDevObjPtr->portsArr[peerPortNum].state == SKERNEL_PORT_STATE_NOT_EXISTS_E)
        {
            if(allowPrint)simWarningPrintf("skernelPortConnectionInfo : ERROR peer portNum[%d] NOT exists (not valid MAC number) \n",
                peerPortNum);
            return WM_ERROR;
        }

        skernelPortLinkSet(devObjPtr,portNum,1,infoPtr->allowSlan,allowPrint);

        devObjPtr->portsArr[portNum].peerInfo.usePeerInfo = GT_TRUE;
        devObjPtr->portsArr[portNum].peerInfo.peerDeviceId = peerDevObjPtr->deviceId;
        devObjPtr->portsArr[portNum].peerInfo.peerPortNum =  peerPortNum;
        devObjPtr->portsArr[portNum].isOnBoardInternalPort = GT_FALSE;
        devObjPtr->portsArr[portNum].forbidSlan = infoPtr->allowSlan ? 0 : 1;

        skernelPortLinkSet(peerDevObjPtr,peerPortNum,1,infoPtr->allowSlan,allowPrint);

        peerDevObjPtr->portsArr[peerPortNum].peerInfo.usePeerInfo = GT_TRUE;
        peerDevObjPtr->portsArr[peerPortNum].peerInfo.peerDeviceId = devObjPtr->deviceId;
        peerDevObjPtr->portsArr[peerPortNum].peerInfo.peerPortNum =  portNum;
        peerDevObjPtr->portsArr[peerPortNum].isOnBoardInternalPort = GT_FALSE;
        peerDevObjPtr->portsArr[peerPortNum].forbidSlan = infoPtr->allowSlan ? 0 : 1;
    }
    else
    {
        if(allowPrint)simWarningPrintf("skernelPortConnectionInfo : ERROR not supported connectionType[%d] \n",
            infoPtr->connectionType);
        return WM_ERROR;
    }


    return WM_OK;
}

/**
* @internal skernelPortConnectionInfo function
* @endinternal
*
* @brief   WM function to state the port connection info.
*
* @param[in] devObjPtr               - (pointer to) the device object
* @param[in] portNum                 - the WM device ingress port number (MAC number).
* @param[in] infoPtr                 - (pointer to) info about the port connection
*
*/
WM_STATUS skernelPortConnectionInfo
(
    IN SKERNEL_DEVICE_OBJECT        *devObjPtr,
    IN GT_U32                       portNum,
    IN WM_PORT_CONNECTION_INFO_STC  *infoPtr
)
{
    return internal_skernelPortConnectionInfo(devObjPtr,portNum,infoPtr,1/* allowPrint */);
}

#define WAIT_PORT_LINK_CHANGE(sleeptime) \
    /*free previous take */       \
    SCIB_SEM_SIGNAL;              \
    devObjPtr=skernelSleep(devObjPtr,sleeptime);    \
    /*take it again */            \
    SCIB_SEM_TAKE

/**
* @internal wm__simDisconnectAllOuterPorts function
* @endinternal
*
* @brief   WM function to disconnect all outer ports from slans/other connections
*
*/
void wm__simDisconnectAllOuterPorts(void)
{
    SKERNEL_DEVICE_OBJECT        *devObjPtr;
    GT_U32  deviceId,portNum;
    WM_PORT_CONNECTION_INFO_STC info;

    memset(&info,0,sizeof(info));
    info.connectionType = WM_PORT_CONNECTION_TYPE_NONE_E;
    info.allowSlan = GT_FALSE;

    for(deviceId =0 ;deviceId < SMAIN_MAX_NUM_OF_DEVICES_CNS ; deviceId++)
    {
        devObjPtr = smainDeviceObjects[deviceId];
        if(!devObjPtr)
        {
            continue;
        }

        for(portNum = 0; portNum < devObjPtr->portsNumber; portNum++)
        {
            if(devObjPtr->portsArr[portNum].isOnBoardInternalPort ||
               devObjPtr->portsArr[portNum].state == SKERNEL_PORT_STATE_NOT_EXISTS_E)
            {
                continue;
            }

            if(portNum < devObjPtr->numSlans &&
                devObjPtr->portSlanInfo[portNum].slanName[0])
            {
                /* disconnect SLAN */
                skernelUnbindDevPort2Slan(deviceId,portNum);

                /* allow the interrupts due to link change to be over before we unbind next port */
                WAIT_PORT_LINK_CHANGE(10);
            }
            if(devObjPtr->portsArr[portNum].usedByOtherConnection)
            {
                /* disconnect other connections */
                internal_skernelPortConnectionInfo(devObjPtr,portNum,&info,0/* NOT allowPrint */);

                /* allow the interrupts due to link change to be over before we unbind next port */
                WAIT_PORT_LINK_CHANGE(10);
            }

        }
    }

    /* once set to 1 , will not restore to 0 */
    simDisconnectAllOuterPorts_mode = 1;

    return;
}

/**
* @internal skernelDeviceResetType function
* @endinternal
*
* @brief   function to indicate the device to do reset according to needed type.
*
* @param[in] devObjPtr   - the device object
* @param[in] resetType   - the type of reset
*                          0 - HARD reset include all memories , ports .
*                          1 - SOFT reset without PCIe config space , skipping reset of :
*                              the PCIe config space (all registers return to 'default')
*                              all the registers in BAR0,BAR2 (all registers/memories return to 'default')
*                              info set by wmDeviceOnPciBusSet
*                              info set by wmDeviceIrqParamsSet
*                              info set by wmPortConnectionInfo about all the ports.
*
*/
void skernelDeviceResetType(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32           resetType
)
{
    GT_U32  dummyMem1 = 0,dummyMem2 = 0;

    SCIB_SEM_TAKE;

    if(resetType == 1)
    {
        simulationPrintf("WM API : 'soft reset'  is about to be triggered \n");

        smemDfxRegFldSet(devObjPtr,
            SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.DFXServerUnitsBC2SpecificRegs.
                PCIeSkipInitializationMatrix,
            8,1,1); /* value 0 - do reset , value 1 - don't reset */
    }
    else
    {
        simulationPrintf("WM API : 'HARD reset' is about to be triggered \n");
    }

    /* trigger soft reset */
    smemLion3ActiveWriteDfxServerResetControlReg(devObjPtr,0/*address*/,1/*memSize*/,
        &dummyMem1,0/*param*/,&dummyMem2);

    SCIB_SEM_SIGNAL;

    /* wait for the reset to end */
    while(!simulationSoftResetDone)
    {
        SIM_OS_MAC(simOsSleep)(100);
    }

    if(resetType == 1)
    {
        simulationPrintf("WM API : 'soft reset' ENDED \n");
    }
    else
    {
        simulationPrintf("WM API : 'HARD reset' ENDED \n");
    }

    return;
}


