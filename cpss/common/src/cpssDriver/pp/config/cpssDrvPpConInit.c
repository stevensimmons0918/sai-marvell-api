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
* @file cpssDrvPpConInit.c
*
* @brief Low level driver initialization of PPs, and declarations of global
* variables
*
* @version   79
********************************************************************************
*/

#if (defined PX_FAMILY)
    #define ALLOW_PX_CODE
#endif

#include <cpssDriver/pp/interrupts/generic/prvCpssDrvInterrupts.h>
#include <cpssDriver/pp/config/prvCpssDrvPpCfgInit.h>
#include <cpssDriver/pp/prvCpssDrvPpDefs.h>
#include <cpssDriver/pp/hardware/prvCpssDrvHwCntl.h>
#include <cpssDriver/pp/hardware/prvCpssDrvObj.h>
#include <cpssDriver/pp/interrupts/dxExMx/prvCpssDrvDxExMxInterrupts.h>
#include <cpssDriver/pp/hardware/prvCpssDrvHwPpPortGroupCntl.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

/*global variables macros*/
#define HWINIT_GLOVAR(_var) \
    PRV_SHARED_GLOBAL_VAR_GET(commonMod.genericHwInitDir._var)

extern GT_U16   prvCpssDrvDebugDeviceId[PRV_CPSS_MAX_PP_DEVICES_CNS];

#if (defined ASIC_SIMULATION_ENV_FORBIDDEN && !(defined ASIC_SIMULATION))
    /* 'Forbid' the code to run as ASIC_SIMULATION ... we need 'like' HW compilation ! */
    #define ASIC_SIMULATION_H_EXIST
#endif


#ifdef ASIC_SIMULATION_H_EXIST
    /* Simulation H files */
    #include <asicSimulation/SInit/sinit.h>
#endif /*ASIC_SIMULATION_H_EXIST*/

CPSS_TBD_BOOKMARK_FALCON    /* need to find proper solution for the driver */
static CPSS_PP_FAMILY_TYPE_ENT nextDevFamily = CPSS_PP_FAMILY_LAST_E;

/**
 * @struct PRV_DRV_MG_DFX_INFO_STC
 *
 *  @brief struct includes info to access MG and DFX unit
 *
 */
typedef struct {
    /** base address for MG_0 unit.
     *  It's equal 0 for devices before Falcon and depends
     *  on specific family for Falcon and above */
    GT_U32  mg0UnitBaseAddr;

    /** base address for DFX units.
     *  It's equal 0 for devices before Falcon and depends
     *  on specific family for Falcon and above */
    GT_U32  dfxUnitBaseAddr;

    /** defines if device has separate address space for each MG unit:
     *  1 - for devices before Falcon
     *  0 - for Falcon and above */
    GT_U32  allowMultiMgSdmaInit;

    /** address of PCI_e MAC register for Falcon WA */
    GT_U32  regAddr_ACK_F_ASPM_CTRL_OFF;
} PRV_DRV_MG_DFX_INFO_STC;

/* address of SDMA configuration register addresses */
#define SDMA_CFG_REG_ADDR       (mgDfxInfoPtr->mg0UnitBaseAddr + 0x2800)

/* DX address of PCI Pad Calibration Configuration register addresses */
#define DX_PCI_PAD_CALIB_REG_ADDR       0x0108

/* DX address of PCI Status and Command Register addresses */
#define DX_PCI_STATUS_CMD_REG_ADDR       0x04

/* DX address offset of the vendor id register address */
#define VENDOR_ID_REG_ADDR_OFFSET_CNS           0x50

/* DX address of the vendor id register address */
#define VENDOR_ID_REG_ADDR           (mgDfxInfoPtr->mg0UnitBaseAddr + VENDOR_ID_REG_ADDR_OFFSET_CNS)

#define DEVICE_ID_AND_REV_REG_ADDR_CNS (mgDfxInfoPtr->mg0UnitBaseAddr + PRV_CPSS_DEVICE_ID_AND_REV_REG_ADDR_CNS)

/* DFX address of the vendor id register address */
#define DFX_DEVICE_ID_AND_REV_REG_ADDR_CNS      (mgDfxInfoPtr->dfxUnitBaseAddr + 0xF8240)

#define DFX_JTAG_DEVICE_ID_AND_REV_REG_ADDR_CNS (mgDfxInfoPtr->dfxUnitBaseAddr + 0xF8244)

#ifndef GT_NA
#define GT_NA ((GT_U32)~0)
#endif

/* PORT_GROUPS_INFO_STC - info about the port groups
        NOTE: assumption that all port groups of the device are the same !
    numOfPortGroups - number of port groups
    portsBmp        - the actual ports bmp of each port group
                        for example: Lion 0xFFF (12 ports)
    maxNumPorts      - number of ports in range
                        for example: Lion 16 ports (12 out of 16 in each port group)
*/
typedef struct {
    GT_U32  numOfPortGroups;
    GT_U32  portsBmp;
    GT_U32  maxNumPorts;
} PORT_GROUPS_INFO_STC;

#ifdef GM_USED
extern GT_STATUS prvCpssDrvPpIntDefDxChBobcat2Init__GM(GT_U8 devNum);
#endif

/* flag to state that running on emulator */
static GT_U32  cpssDeviceRun_onEmulator_active = 0;
/**
* @internal cpssDeviceRunCheck_onEmulator function
* @endinternal
*
* @brief   Check if running on EMULATOR.
*         (when running on EMULATOR) This function must be called before
*         'phase 1' init of the device.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval 0 - not running on EMULATOR.
* @retval 1 - running on EMULATOR.
*/
GT_U32  cpssDeviceRunCheck_onEmulator(void)
{
    return cpssDeviceRun_onEmulator_active;
}
/**
* @internal cpssDeviceRunSet_onEmulator function
* @endinternal
*
* @brief   State that running on EMULATOR. (all devices considered to be running on
*         EMULATOR)
*         (when running on EMULATOR) This function must be called before
*         'phase 1' init of the device.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*                                       None.
*/
void  cpssDeviceRunSet_onEmulator(void)
{
    /* NOTE: must be called before 'phase 1' init */
    cpssDeviceRun_onEmulator_active = 1;
}

static GT_U32 cpssDeviceRun_onEmulator_internalCpu_active = 0;
/**
* @internal cpssDeviceRunSet_onEmulator_internalCpu function
* @endinternal
*
* @brief   State that running on EMULATOR and the SW runs in the 'internal CPU'
*          that is in the emulator.(not in the 'HOST CPU' that is over the PEX)
*          (all devices considered to be running on EMULATOR as 'internal CPU')
*
*         (when running on EMULATOR) This function must be called before
*         'phase 1' init of the device.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
*                                       None.
*/
void  cpssDeviceRunSet_onEmulator_internalCpu(void)
{
    cpssDeviceRun_onEmulator_internalCpu_active = 1;
}
/**
* @internal cpssDeviceRunCheck_onEmulator_internalCpu function
* @endinternal
*
* @brief   Check if running on EMULATOR and the SW runs in the 'internal CPU'
*          that is in the emulator.(not in the 'HOST CPU' that is over the PEX)
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
*
* @retval 0 - not running on EMULATOR in the internal CPU.
* @retval 1 - running on EMULATOR in the internal CPU.
*/
GT_U32  cpssDeviceRunCheck_onEmulator_internalCpu(void)
{
    return cpssDeviceRun_onEmulator_internalCpu_active;
}

static GT_U32 cpssDeviceRun_onEmulator_internalCpu_timeFactor = 1;
void  cpssDeviceRunSet_embeddedCpuTimeFactor(GT_U32 timeFactor)
{
    cpssDeviceRun_onEmulator_internalCpu_timeFactor = timeFactor ? timeFactor : 1;
}
GT_U32 cpssDeviceRunGet_embeddedCpuTimeFactor(void)
{
    return cpssDeviceRun_onEmulator_internalCpu_timeFactor;
}

extern GT_BOOL prvCpssDxChHwIsUnitUsed
(
    IN GT_U32                   devNum,
    IN GT_U32                   portGroupId,
    IN GT_U32                   regAddr
);
extern GT_BOOL prvCpssPxHwIsUnitUsed
(
    IN GT_U32                   devNum,
    IN GT_U32                   portGroupId,
    IN GT_U32                   regAddr
);

/* check if register address supported on emulator */
GT_BOOL prvCpssOnEmulatorSupportedAddrCheck(
    IN  GT_U8       devNum,
    IN  GT_U32      portGroupId,
    IN  GT_U32      regAddr
)
{
#ifdef CHX_FAMILY
    if(PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[devNum] && CPSS_IS_DXCH_FAMILY_MAC(PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[devNum]->devFamily))
    {
        if(GT_FALSE == prvCpssDxChHwIsUnitUsed(devNum,portGroupId,regAddr))
        {
            /* the memory/register is not in supported memory space */
            return GT_FALSE;
        }
    }
#endif /*CHX_FAMILY*/

#ifdef PX_FAMILY
    if(PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[devNum] && CPSS_IS_PX_FAMILY_MAC(PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[devNum]->devFamily))
    {
        if(GT_FALSE == prvCpssPxHwIsUnitUsed(devNum,portGroupId,regAddr))
        {
            /* the memory/register is not in supported memory space */
            return GT_FALSE;
        }
    }
#endif /*PX_FAMILY*/

    return GT_TRUE;
}

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
#define BMP_CONTINUES_PORTS_MAC(x)  ((1<<(x)) - 1)

/* macro to set range of ports in bmp
*/
#define PORT_RANGE_MAC(startPort,numPorts)\
    (GT_U32)(BMP_CONTINUES_PORTS_MAC(numPorts) << ((startPort) & 0x1f ))

/* macro to set bit of port in 'ports' bmp -
*/
#define GLOBAL_PORT_BIT_MAC(globalBit)\
    (GT_U32)(1 << ((globalBit) & 0x1f ))


static const CPSS_PORTS_BMP_STC portsBmp0to7_24_25 = {{(PORT_RANGE_MAC(0,8) |
                                                        PORT_RANGE_MAC(24,2)) ,0}};

static const CPSS_PORTS_BMP_STC portsBmp0to15_24to27 = {{(PORT_RANGE_MAC(0,16) |
                                                          PORT_RANGE_MAC(24,4)) ,0}};

static const CPSS_PORTS_BMP_STC portsBmp0to7_24to27 = {{(PORT_RANGE_MAC(0,8) |
                                                        PORT_RANGE_MAC(24,4)) ,0}};

static const CPSS_PORTS_BMP_STC portsBmp24to32 = {{0xFF000000 ,0x00000001}};

/* macro for bmp of ports : 0..47 , 56..59 , 64..71 */
#define PORTS_BMP_0_TO_47_56_TO_59_64_TO_71                 \
    {{PORT_RANGE_MAC(0,16) | PORT_RANGE_MAC(16,16) ,        \
      PORT_RANGE_MAC(32,16) | PORT_RANGE_MAC(56,4) ,        \
      PORT_RANGE_MAC(64,8) , 0 }}

/* macro for bmp of ports : 0..47 , 56..59 , 62, 64..71 */
#define PORTS_BMP_0_TO_47_56_TO_59_62_64_TO_71                              \
    {{PORT_RANGE_MAC(0,16) | PORT_RANGE_MAC(16,16) ,                        \
      PORT_RANGE_MAC(32,16) | PORT_RANGE_MAC(56,4) | PORT_RANGE_MAC(62,1),  \
      PORT_RANGE_MAC(64,8) , 0 }}

/* bmp of ports : 0..47 , 56..59 , 62, 64..71 */
static const CPSS_PORTS_BMP_STC portsBmp0to47_62_56to59_64to71 =
    PORTS_BMP_0_TO_47_56_TO_59_62_64_TO_71;


/* macro for bmp of ports : 0..23 , 56..59 , 64..71 */
#define PORTS_BMP_0_TO_23_56_TO_59_64_TO_71                 \
    {{PORT_RANGE_MAC(0,24),                                 \
      PORT_RANGE_MAC(56,4),                                 \
      PORT_RANGE_MAC(64,8) , 0 }}

/* macro for bmp of ports : 0..23 , 56..59 , 62, 64..71 */
#define PORTS_BMP_0_TO_23_56_TO_59_62_64_TO_71              \
    {{PORT_RANGE_MAC(0,24),                                 \
      PORT_RANGE_MAC(56,4) | PORT_RANGE_MAC(62,1),          \
      PORT_RANGE_MAC(64,8) , 0 }}

/* macro for bmp of ports : 0..27 , 56..59 , 62, 64..71 */
#define PORTS_BMP_0_TO_27_56_TO_59_62_64_TO_71              \
    {{PORT_RANGE_MAC(0,28),                                 \
      PORT_RANGE_MAC(56,4) | PORT_RANGE_MAC(62,1),          \
      PORT_RANGE_MAC(64,8) , 0 }}

/* macro for bmp of ports :  56..59 , 64..71 */
#define PORTS_BMP_56_TO_59_64_TO_71                         \
    {{0 ,                                                   \
      PORT_RANGE_MAC(56,4) ,                                \
      PORT_RANGE_MAC(64,8) , 0 }}

/* macro for bmp of ports :  56..59 , 62, 64..71 */
#define PORTS_BMP_56_TO_59_62_64_TO_71                      \
    {{0 ,                                                   \
      PORT_RANGE_MAC(56,4) | PORT_RANGE_MAC(62,1),          \
      PORT_RANGE_MAC(64,8) , 0 }}

/* bmp of ports :         56..59 , 62, 64..71 */
static const CPSS_PORTS_BMP_STC portsBmp56to59_62_64to71 =
    PORTS_BMP_56_TO_59_62_64_TO_71;

/* macro for bmp of ports : 0, 4, 8, 12, 16, 20, 24, 28, 32, 36, 40, 44, 56..59, 62, 64..71 */
#define PORTS_BMP_0_4_8_12_16_20_24_28_32_36_40_44_56_TO_59_62_64_TO_71                   \
    {{PORT_RANGE_MAC( 0,1) | PORT_RANGE_MAC( 4,1) | PORT_RANGE_MAC( 8,1) | PORT_RANGE_MAC( 12,1) | PORT_RANGE_MAC(16,1) | PORT_RANGE_MAC(20,1) | PORT_RANGE_MAC(24,1) | PORT_RANGE_MAC(28,1),\
      PORT_RANGE_MAC(32,1) | PORT_RANGE_MAC(36,1) | PORT_RANGE_MAC(40,1) | PORT_RANGE_MAC(44,1) | PORT_RANGE_MAC(56,4) | PORT_RANGE_MAC(62,1),   \
      PORT_RANGE_MAC(64,8) , 0 }}

/* macro for bmp of ports : 0..3, 8..11, 16..23, 28..39, 44..47, 52..59, 64..73  */
#define PORTS_BMP_0_TO_3_8_TO_11_16_TO_23_28_TO_39_44_TO_47_52_TO_59_64_TO_73 \
    {{PORT_RANGE_MAC( 0, 4) | PORT_RANGE_MAC( 8, 4) | PORT_RANGE_MAC(16, 8) | PORT_RANGE_MAC(28, 4), \
      PORT_RANGE_MAC(32, 8) | PORT_RANGE_MAC(44, 4) | PORT_RANGE_MAC(52, 8), \
      PORT_RANGE_MAC(64, 10), 0 }}

/* macro for bmp of ports : 4..11, 16..71 */
#define PORTS_BMP_4_TO_11_16_TO_72                              \
    {{PORT_RANGE_MAC(4,8) | PORT_RANGE_MAC(16,16) ,             \
      PORT_RANGE_MAC(32,16) | PORT_RANGE_MAC(48,16) ,           \
      PORT_RANGE_MAC(64,9), 0 }}

/* macro for bmp of ports : 0..35 , 47..59 */
#define PORTS_BMP_0_TO_35_47_TO_59                          \
    {{PORT_RANGE_MAC(0,16) | PORT_RANGE_MAC(16,16) ,        \
      PORT_RANGE_MAC(32,4) | PORT_RANGE_MAC(47, 13) , 0 }}

/* macro for bmp of ports : 0..23, 28..31, 36..39, 44..47, 52..55, 60..63, 68..71 */
#define PORTS_BMP_0_TO_23_28_TO_31_36_TO_39_44_TO_47_52_TO_55_60_TO_63_68_TO_71   \
    {{PORT_RANGE_MAC(0,24) | PORT_RANGE_MAC(28, 4) ,         \
      PORT_RANGE_MAC(36,4) | PORT_RANGE_MAC(44, 4) | PORT_RANGE_MAC(52,4) | PORT_RANGE_MAC(60, 4),   \
      PORT_RANGE_MAC(68, 4), 0 }}

/* macro for bmp of ports : 0..23 , 47 */
#define PORTS_BMP_0_TO_23_47                                \
    {{PORT_RANGE_MAC(0,24), PORT_RANGE_MAC(47,1), 0 }}

/* macro for bmp of ports : 0..7, 12..19, 47 */
#define PORTS_BMP_0_TO_7_12_TO_19_47                        \
    {{PORT_RANGE_MAC(0,8) | PORT_RANGE_MAC(12,8), PORT_RANGE_MAC(47,1), 0 }}

static const CPSS_PORTS_BMP_STC portsBmp4to11_16to72 =
    PORTS_BMP_4_TO_11_16_TO_72;

static const CPSS_PORTS_BMP_STC portsBmp0to35_47to59 =
    PORTS_BMP_0_TO_35_47_TO_59;

static const CPSS_PORTS_BMP_STC portsBmp48_special_map =
    PORTS_BMP_0_TO_23_28_TO_31_36_TO_39_44_TO_47_52_TO_55_60_TO_63_68_TO_71;

static const CPSS_PORTS_BMP_STC portsBmp0to23_47 =
    PORTS_BMP_0_TO_23_47;

static const CPSS_PORTS_BMP_STC portsBmp0to7_12to19_47 =
    PORTS_BMP_0_TO_7_12_TO_19_47;

static const CPSS_PORTS_BMP_STC portsBmp0to15 = {{PORT_RANGE_MAC(0,16) ,0}};
static const CPSS_PORTS_BMP_STC portsBmp4to13 = {{PORT_RANGE_MAC(4,10) ,0}};
static const CPSS_PORTS_BMP_STC portsBmp4to15 = {{PORT_RANGE_MAC(4,12) ,0}};
static const CPSS_PORTS_BMP_STC portsBmp4to7_13_15 = {{PORT_RANGE_MAC(4,4) | PORT_RANGE_MAC(13,1) | PORT_RANGE_MAC(15,1) ,0}};
#ifndef GM_USED
static const CPSS_PORTS_BMP_STC portsBmp0to15_32to47_80to127_CPU_0xE5 =
    {{0x0000FFFF, 0x0000FFFF, 0xFFFF0000,0xFFFFFFFF,
#if CPSS_MAX_PORTS_NUM_CNS > 128
    0x000000E5, 0
#endif
    }};

static const CPSS_PORTS_BMP_STC portsBmp0to15_32to47_80to95_112to127_CPU_0xA5 =
    {{0x0000FFFF, 0x0000FFFF, 0xFFFF0000, 0xFFFF0000,
#if CPSS_MAX_PORTS_NUM_CNS > 128
    0x000000A5, 0
#endif
    }};

static const CPSS_PORTS_BMP_STC portsBmp0to15_32to47_64to95_112to127_CPU_0xB5 =
    {{0x0000FFFF, 0x0000FFFF, 0xFFFFFFFF, 0xFFFF0000,
#if CPSS_MAX_PORTS_NUM_CNS > 128
    0x000000B5, 0
#endif
    }};

static const CPSS_PORTS_BMP_STC portsBmp0to47_80to95_112to143_160to175_208to223_224to255_CPU_0xE5A7 =
    {{0xFFFFFFFF, 0x0000FFFF, 0xFFFF0000, 0xFFFF0000,
#if CPSS_MAX_PORTS_NUM_CNS > 128
    0x0000FFFF, 0x0000FFFF, 0xFFFF0000, 0xFFFFFFFF,
#if CPSS_MAX_PORTS_NUM_CNS > 256
    0x0000E5A7
#endif
#endif
    }};

#endif

/*****************************************************************************
* Global
******************************************************************************/
/* pointer to the object that control ExMxDx */
static PRV_CPSS_DRV_GEN_OBJ_STC drvGenExMxDxObj;
PRV_CPSS_DRV_GEN_OBJ_STC *prvCpssDrvGenExMxDxObjPtr = &drvGenExMxDxObj;

/* pointer to the object that control PCI */
PRV_CPSS_DRV_MNG_INF_OBJ_STC *prvCpssDrvMngInfPciPtr = NULL;

/* pointer to the object that control PEX with 8 address completion regions */
PRV_CPSS_DRV_MNG_INF_OBJ_STC *prvCpssDrvMngInfPexMbusPtr = NULL;

/* pointer to the object that control PCI -- for HA standby */
PRV_CPSS_DRV_MNG_INF_OBJ_STC *prvCpssDrvMngInfPciHaStandbyPtr = NULL;

/* pointer to the object that control SMI */
PRV_CPSS_DRV_MNG_INF_OBJ_STC *prvCpssDrvMngInfSmiPtr = NULL;

/* pointer to the object that control TWSI */
PRV_CPSS_DRV_MNG_INF_OBJ_STC *prvCpssDrvMngInfTwsiPtr = NULL;

/* pointer to the object that control PEX */
/*PRV_CPSS_DRV_MNG_INF_OBJ_STC *prvCpssDrvMngInfPexPtr = NULL;*/

/*****************************************************************************
* Externals
******************************************************************************/


/*****************************************************************************
* Forward declarations
******************************************************************************/

/*****************************************************************************
* Local variables
******************************************************************************/
#define END_OF_TABLE    0xFFFFFFFF

/*****************/
/* xCat3 devices */
/*****************/
static const  CPSS_PP_DEVICE_TYPE xcat3_24_2legalDevTypes[] =
{   PRV_CPSS_XCAT3_24GE_2STACK_PORTS_DEVICES,
    END_OF_TABLE };

static const  CPSS_PP_DEVICE_TYPE xcat3_24_4legalDevTypes[] =
{   PRV_CPSS_XCAT3_24GE_4STACK_PORTS_DEVICES,
    END_OF_TABLE };

static const  CPSS_PP_DEVICE_TYPE xcat3_16_4legalDevTypes[] =
{   PRV_CPSS_XCAT3_16GE_4STACK_PORTS_DEVICES,
    END_OF_TABLE };

static const  CPSS_PP_DEVICE_TYPE xcat3_8_4legalDevTypes[] =
{   PRV_CPSS_XCAT3_8GE_4STACK_PORTS_DEVICES,
    END_OF_TABLE };

static const  CPSS_PP_DEVICE_TYPE xcat3_8_2legalDevTypes[] =
{   PRV_CPSS_XCAT3_8GE_2STACK_PORTS_DEVICES,
    END_OF_TABLE };

/*****************/
/* AC5 devices   */
/*****************/
static const  CPSS_PP_DEVICE_TYPE ac5_24_4legalDevTypes[] =
{   PRV_CPSS_AC5_24GE_4STACK_PORTS_DEVICES,
    END_OF_TABLE };

static const  CPSS_PP_DEVICE_TYPE ac5_8_4legalDevTypes[] =
{   PRV_CPSS_AC5_8GE_4STACK_PORTS_DEVICES,
    END_OF_TABLE };


/****************/
/* pipe devices */
/****************/
static const  CPSS_PP_DEVICE_TYPE pipe_12_4legalDevTypes[] =
{   CPSS_98PX1012_CNS,
    CPSS_98PX1016_CNS,
    CPSS_98PX1015_CNS,
    END_OF_TABLE };
static const  CPSS_PP_DEVICE_TYPE pipe_8_2legalDevTypes[] =
{   CPSS_98PX1008_CNS,
    END_OF_TABLE };
static const  CPSS_PP_DEVICE_TYPE pipe_8_4legalDevTypes[] =
{   CPSS_98PX1024_CNS,
    END_OF_TABLE };
static const  CPSS_PP_DEVICE_TYPE pipe_4_2legalDevTypes[] =
{   CPSS_98PX1022_CNS,
    END_OF_TABLE };


/****************/
/* lion2 devices */
/****************/

/* lion2 with port groups 0,1,2,3,4,5,6,7 */
static const  CPSS_PP_DEVICE_TYPE lion2_port_groups01234567legalDevTypes[] = {CPSS_LION2_PORT_GROUPS_01234567_CNS , END_OF_TABLE };

/* lion2-hooper with port groups 0,1,2,3 */
static const  CPSS_PP_DEVICE_TYPE lion2_hooper_port_groups0123legalDevTypes[] = {CPSS_LION2_HOOPER_PORT_GROUPS_0123_CNS , END_OF_TABLE };

static const PORT_GROUPS_INFO_STC lionPortGroups =
{
    4 /*numOfPortGroups*/ ,
    PORT_RANGE_MAC(0,12) /*portsBmp*/   ,
    16 /*maxNumPorts*/
};

static const PORT_GROUPS_INFO_STC lion2PortGroups =
{
    8 /*numOfPortGroups*/ ,
    PORT_RANGE_MAC(0,12) /*portsBmp*/   ,
    16 /*maxNumPorts*/
};

/* macro to set any of 4 port groups as active(exists) in a bitmap */
#define SET_4_PORT_GROUPS_MAC(portGroup0Exists,portGroup1Exists,portGroup2Exists,portGroup3Exists) \
    ((portGroup3Exists) << 3) | ((portGroup2Exists) << 2) |                        \
    ((portGroup1Exists) << 1) | (portGroup0Exists)

/* bmp of lion port groups , for 'known' devices */
static const GT_U32 portGroups0123bmp = SET_4_PORT_GROUPS_MAC(1,1,1,1);
static const GT_U32 portGroups01234567bmp = 0xff;

/* number of ports in the lion device according to the last active port group */
#define LION_NUM_PORTS_FROM_LAST_PORT_GROUP_MAC(lastPortGroup) (((lastPortGroup + 1) * 16)-4)

/* Bobcat2 devices */
static const  CPSS_PP_DEVICE_TYPE bobcat2_legalDevTypes[] =
{   PRV_CPSS_BOBCAT2_DEVICES,
    END_OF_TABLE};
/* Bobk caelum devices */
static const  CPSS_PP_DEVICE_TYPE bobkCaelum_legalDevTypes[] =
{   PRV_CPSS_BOBK_CAELUM_DEVICES,
    END_OF_TABLE};
/* Bobk cetus devices */
static const  CPSS_PP_DEVICE_TYPE bobkCetus_legalDevTypes[] =
{   PRV_CPSS_BOBK_CETUS_DEVICES,
    END_OF_TABLE};

/* Aldrin devices */
static const  CPSS_PP_DEVICE_TYPE aldrin_legalDevTypes[] =
{   PRV_CPSS_ALDRIN_DEVICES,
    END_OF_TABLE};

/* AC3X devices */
static const  CPSS_PP_DEVICE_TYPE ac3x_legalDevTypes[] =
{   PRV_CPSS_AC3X_DEVICES,
    END_OF_TABLE};

/* Bobcat3 devices */
static const  CPSS_PP_DEVICE_TYPE bobcat3_legalDevTypes[] =
{   PRV_CPSS_BOBCAT3_ALL_DEVICES,
    END_OF_TABLE};

/* Armstrong devices */
static const  CPSS_PP_DEVICE_TYPE armstrong_legalDevTypes[] =
{   PRV_CPSS_ARMSTRONG_ALL_DEVICES,
    END_OF_TABLE};

/* Aldrin2 devices */
static const  CPSS_PP_DEVICE_TYPE aldrin2_72_legalDevTypes[] =
{   PRV_CPSS_ALDRIN2_72_PORTS_DEVICES,
    END_OF_TABLE};

/* Aldrin2 devices */
static const  CPSS_PP_DEVICE_TYPE aldrin2_64_legalDevTypes[] =
{   PRV_CPSS_ALDRIN2_64_PORTS_DEVICES,
    END_OF_TABLE};

/* Aldrin2 devices */
static const  CPSS_PP_DEVICE_TYPE aldrin2_48_legalDevTypes[] =
{   PRV_CPSS_ALDRIN2_48_PORTS_DEVICES,
    END_OF_TABLE};

/* Aldrin2 devices */
static const  CPSS_PP_DEVICE_TYPE aldrin2_24_legalDevTypes[] =
{   PRV_CPSS_ALDRIN2_24_PORTS_DEVICES,
    END_OF_TABLE};

static const  CPSS_PP_DEVICE_TYPE aldrin2_16_legalDevTypes[] =
{   PRV_CPSS_ALDRIN2_16_PORTS_DEVICES,
    END_OF_TABLE};


#ifndef GM_USED /* Falcon devices for WM and BM (but not GM) */
/* Falcon devices */
static const  CPSS_PP_DEVICE_TYPE falcon_3_2_legalDevTypes[] =
{
    CPSS_98CX8513_CNS       ,
    END_OF_TABLE};
/* Falcon devices */
static const  CPSS_PP_DEVICE_TYPE falcon_6_4_legalDevTypes[] =
{
    CPSS_98CX8540_CNS       ,
    CPSS_98CX8540_H_CNS     ,
    CPSS_98CX8522_CNS       ,
    END_OF_TABLE};
/* Falcon devices */
static const  CPSS_PP_DEVICE_TYPE falcon_12_8_legalDevTypes[] =
{
    CPSS_98CX8580_CNS       ,
    CPSS_98CX8580_E_CNS     ,
    CPSS_98CX8580_10_CNS    ,
    CPSS_98CX8542_CNS       ,
    END_OF_TABLE};
/* Falcon devices */
static const CPSS_PP_DEVICE_TYPE falcon_160X50G_legalDevTypes[] =
{
    CPSS_98CX8550_CNS       ,
    END_OF_TABLE};
/* Falcon devices */
static const CPSS_PP_DEVICE_TYPE falcon_half_eagle_5_ravens_legalDevTypes[] =
{
    CPSS_98CX8530_CNS,
    CPSS_98CX8512_CNS,
    END_OF_TABLE};
/* Falcon devices */
static const CPSS_PP_DEVICE_TYPE falcon_80X50_legalDevTypes[] =
{
    CPSS_98CX8535_CNS,
    CPSS_98CX8535_H_CNS,
    CPSS_98CX8514_CNS,
    CPSS_98EX5614_CNS,
    END_OF_TABLE};

/* Falcon devices */
static const  CPSS_PP_DEVICE_TYPE falcon_64X50G_legalDevTypes[] =
{
    CPSS_98CX8520_CNS,
    CPSS_98CX8525_CNS,
    END_OF_TABLE};
static const  CPSS_PP_DEVICE_TYPE armstrong2_80X25G_legalDevTypes[] =
{
    CPSS_98EX5610_CNS,
    END_OF_TABLE};
/* Armstrong devices */
static const  CPSS_PP_DEVICE_TYPE armstrong2_96X25G_32X50G_legalDevTypes[] =
{
    CPSS_98EX5630_CNS,
    END_OF_TABLE};


#else /* device for the 'GM' to use */
/* Falcon devices */
static const  CPSS_PP_DEVICE_TYPE falcon_GM_legalDevTypes[] =
{   PRV_CPSS_FALCON_ALL_DEVICES,
    END_OF_TABLE};
#endif

/* AC5P devices */
static const  CPSS_PP_DEVICE_TYPE ac5p_legalDevTypes[] =
{
    CPSS_98DX4504_CNS       ,
    CPSS_98DX4504M_CNS      ,
    CPSS_98DX4510_CNS       ,
    CPSS_98DX4510_H_CNS     ,
    CPSS_98DX4510M_H_CNS    ,
    CPSS_98DX4510M_CNS      ,
    CPSS_98DX4530_CNS       ,
    CPSS_98DX4530M_CNS      ,
    CPSS_98DX4550_CNS       ,
    CPSS_98DX4550M_CNS      ,
    CPSS_98DX4550_H_CNS     ,
    CPSS_98DX4550M_H_CNS    ,
    END_OF_TABLE};

/* AC5P XL devices */
static const  CPSS_PP_DEVICE_TYPE ac5p_XL_legalDevTypes[] =
{
    CPSS_98DX9410_CNS       ,
    CPSS_98DX4570_CNS       ,
    CPSS_98DX4570M_CNS      ,
    CPSS_98DX4590_CNS       ,
    CPSS_98DX4590M_CNS      ,
    CPSS_98DX4590_H_CNS     ,
    CPSS_98DX4590M_H_CNS    ,
    END_OF_TABLE};

/* AC5P Armstron 2S devices */
static const  CPSS_PP_DEVICE_TYPE ac5p_Armstrong2S_legalDevTypes[] =
{
    CPSS_98EX5604_CNS,
    CPSS_98EX5604M_CNS,
    END_OF_TABLE};

/* AC5P Aldrin3 devices */
static const  CPSS_PP_DEVICE_TYPE ac5p_Aldrin3_legalDevTypes[] =
{
    CPSS_98DX7324_CNS,
    CPSS_98DX7324M_CNS,
    CPSS_98DX7325_CNS,
    CPSS_98DX7325M_CNS,
    END_OF_TABLE};

/* AC5P Aldrin3 QSGMII devices */
static const  CPSS_PP_DEVICE_TYPE ac5p_Aldrin3_QSGMII_legalDevTypes[] =
{
    CPSS_98DXC725_CNS,
    END_OF_TABLE};

/* AC5P Aldrin3 XL devices */
static const  CPSS_PP_DEVICE_TYPE ac5p_Aldrin3_XL_legalDevTypes[] =
{
    CPSS_98DX7332_CNS,
    CPSS_98DX7332M_CNS,
    CPSS_98DX7335_CNS,
    CPSS_98DX7335M_CNS,
    CPSS_98DX7335_H_CNS,
    CPSS_98DX7335M_H_CNS,
    END_OF_TABLE};

static const  CPSS_PP_DEVICE_TYPE phoenix_48G_6x25g_legalDevTypes[] =
{
    CPSS_98DX3510_CNS,
    CPSS_98DX3530_CNS,
    CPSS_98DX3550_CNS,
    CPSS_98DX3510M_CNS,
    CPSS_98DX3530M_CNS,
    CPSS_98DX3550M_CNS,
    CPSS_98DX3510_H_CNS,
    CPSS_98DX3510M_H_CNS,
    CPSS_98DX3530_H_CNS,
    CPSS_98DX3530M_H_CNS,
    END_OF_TABLE};

static const  CPSS_PP_DEVICE_TYPE phoenix_24G_6x25g_legalDevTypes[] =
{
    CPSS_98DX3500_CNS,
    CPSS_98DX3520_CNS,
    CPSS_98DX3540_CNS,
    CPSS_98DX3500M_CNS,
    CPSS_98DX3520M_CNS,
    CPSS_98DX3540M_CNS,
    END_OF_TABLE};

static const  CPSS_PP_DEVICE_TYPE phoenix_16G_6x25g_legalDevTypes[] =
{
    CPSS_98DX3501_CNS,
    CPSS_98DX3501M_CNS,
    END_OF_TABLE};

static const  CPSS_PP_DEVICE_TYPE phoenix_12X25G_legalDevTypes[] =
{
    CPSS_98DX7312_CNS,
    CPSS_98DX7312M_CNS,
    CPSS_98DXC712_CNS,
    END_OF_TABLE};

static const  CPSS_PP_DEVICE_TYPE phoenix_8X25G_legalDevTypes[] =
{
    CPSS_98DX7308_CNS,
    CPSS_98DX7308M_CNS,
    CPSS_98DXC708_CNS,
    END_OF_TABLE};


/* AC5P ports */
static const CPSS_PORTS_BMP_STC ac5p_portsBmp = {
    {GLOBAL_PORT_BIT_MAC(0 ) |
     GLOBAL_PORT_BIT_MAC(1 ) |
     GLOBAL_PORT_BIT_MAC(2 ) |
     GLOBAL_PORT_BIT_MAC(6 ) |
     GLOBAL_PORT_BIT_MAC(10) |
     GLOBAL_PORT_BIT_MAC(14) |
     GLOBAL_PORT_BIT_MAC(18) |
     GLOBAL_PORT_BIT_MAC(22) |

        /*DP[0] USX part*/
     GLOBAL_PORT_BIT_MAC(3) | GLOBAL_PORT_BIT_MAC(4) | GLOBAL_PORT_BIT_MAC(5) |
     GLOBAL_PORT_BIT_MAC(7) | GLOBAL_PORT_BIT_MAC(8) | GLOBAL_PORT_BIT_MAC(9) |
     GLOBAL_PORT_BIT_MAC(11)| GLOBAL_PORT_BIT_MAC(12)| GLOBAL_PORT_BIT_MAC(13)|
     GLOBAL_PORT_BIT_MAC(15)| GLOBAL_PORT_BIT_MAC(16)| GLOBAL_PORT_BIT_MAC(17)|
     GLOBAL_PORT_BIT_MAC(19)| GLOBAL_PORT_BIT_MAC(20)| GLOBAL_PORT_BIT_MAC(21)|
     GLOBAL_PORT_BIT_MAC(23)| GLOBAL_PORT_BIT_MAC(24)| GLOBAL_PORT_BIT_MAC(25)|

     /* start DP[1] */
     GLOBAL_PORT_BIT_MAC(26) |
     GLOBAL_PORT_BIT_MAC(27) |
     GLOBAL_PORT_BIT_MAC(28) |

        /*DP[1] USX part*/
     GLOBAL_PORT_BIT_MAC(29)| GLOBAL_PORT_BIT_MAC(30)| GLOBAL_PORT_BIT_MAC(31), /*ended ports 0..31 */

     GLOBAL_PORT_BIT_MAC(32) |
     GLOBAL_PORT_BIT_MAC(36) |
     GLOBAL_PORT_BIT_MAC(40) |
     GLOBAL_PORT_BIT_MAC(44) |
     GLOBAL_PORT_BIT_MAC(48) |

        /*DP[1] USX part*/
     GLOBAL_PORT_BIT_MAC(33)| GLOBAL_PORT_BIT_MAC(34)| GLOBAL_PORT_BIT_MAC(35)|
     GLOBAL_PORT_BIT_MAC(37)| GLOBAL_PORT_BIT_MAC(38)| GLOBAL_PORT_BIT_MAC(39)|
     GLOBAL_PORT_BIT_MAC(41)| GLOBAL_PORT_BIT_MAC(42)| GLOBAL_PORT_BIT_MAC(43)|
     GLOBAL_PORT_BIT_MAC(45)| GLOBAL_PORT_BIT_MAC(46)| GLOBAL_PORT_BIT_MAC(47)|
     GLOBAL_PORT_BIT_MAC(49)| GLOBAL_PORT_BIT_MAC(50)| GLOBAL_PORT_BIT_MAC(51)|

     GLOBAL_PORT_BIT_MAC(52) |
     GLOBAL_PORT_BIT_MAC(53) |
     GLOBAL_PORT_BIT_MAC(54) |
     GLOBAL_PORT_BIT_MAC(58) |
     GLOBAL_PORT_BIT_MAC(62),  /*ended ports 32..62 */

     GLOBAL_PORT_BIT_MAC(66) |
     GLOBAL_PORT_BIT_MAC(70) |
     GLOBAL_PORT_BIT_MAC(74),   /*ended ports 66..74 */

     GLOBAL_PORT_BIT_MAC(105) /*CPU*/ } /* ended port 105 */

};

/* AC5P XL ports */
static const CPSS_PORTS_BMP_STC ac5p_XL_portsBmp = {
    {GLOBAL_PORT_BIT_MAC(0 ) |
     GLOBAL_PORT_BIT_MAC(1 ) |
     GLOBAL_PORT_BIT_MAC(2 ) |
     GLOBAL_PORT_BIT_MAC(6 ) |
     GLOBAL_PORT_BIT_MAC(10) |
     GLOBAL_PORT_BIT_MAC(14) |
     GLOBAL_PORT_BIT_MAC(18) |
     GLOBAL_PORT_BIT_MAC(22) |
        /*DP[0] USX part*/
      GLOBAL_PORT_BIT_MAC(3) | GLOBAL_PORT_BIT_MAC(4) | GLOBAL_PORT_BIT_MAC(5) |
      GLOBAL_PORT_BIT_MAC(7) | GLOBAL_PORT_BIT_MAC(8) | GLOBAL_PORT_BIT_MAC(9) |
      GLOBAL_PORT_BIT_MAC(11)| GLOBAL_PORT_BIT_MAC(12)| GLOBAL_PORT_BIT_MAC(13)|
      GLOBAL_PORT_BIT_MAC(15)| GLOBAL_PORT_BIT_MAC(16)| GLOBAL_PORT_BIT_MAC(17)|
      GLOBAL_PORT_BIT_MAC(19)| GLOBAL_PORT_BIT_MAC(20)| GLOBAL_PORT_BIT_MAC(21)|
      GLOBAL_PORT_BIT_MAC(23)| GLOBAL_PORT_BIT_MAC(24)| GLOBAL_PORT_BIT_MAC(25)|

     /* start DP[1] */
     GLOBAL_PORT_BIT_MAC(26) |
     GLOBAL_PORT_BIT_MAC(27) |
     GLOBAL_PORT_BIT_MAC(28) |

        /*DP[1] USX part*/
      GLOBAL_PORT_BIT_MAC(29)| GLOBAL_PORT_BIT_MAC(30)| GLOBAL_PORT_BIT_MAC(31) , /*ended ports 0..31*/

     GLOBAL_PORT_BIT_MAC(32) |
     GLOBAL_PORT_BIT_MAC(36) |
     GLOBAL_PORT_BIT_MAC(40) |
     GLOBAL_PORT_BIT_MAC(44) |
     GLOBAL_PORT_BIT_MAC(48) |

        /*DP[1] USX part*/
      GLOBAL_PORT_BIT_MAC(33)| GLOBAL_PORT_BIT_MAC(34)| GLOBAL_PORT_BIT_MAC(35)|
      GLOBAL_PORT_BIT_MAC(37)| GLOBAL_PORT_BIT_MAC(38)| GLOBAL_PORT_BIT_MAC(39)|
      GLOBAL_PORT_BIT_MAC(41)| GLOBAL_PORT_BIT_MAC(42)| GLOBAL_PORT_BIT_MAC(43)|
      GLOBAL_PORT_BIT_MAC(45)| GLOBAL_PORT_BIT_MAC(46)| GLOBAL_PORT_BIT_MAC(47)|
      GLOBAL_PORT_BIT_MAC(49)| GLOBAL_PORT_BIT_MAC(50)| GLOBAL_PORT_BIT_MAC(51)|

     /* start DP[2] */
     GLOBAL_PORT_BIT_MAC(52) |
     GLOBAL_PORT_BIT_MAC(53) |
     GLOBAL_PORT_BIT_MAC(54) |
     GLOBAL_PORT_BIT_MAC(58) |
     GLOBAL_PORT_BIT_MAC(62)   ,/*ended ports 32..63*/
     GLOBAL_PORT_BIT_MAC(66) |
     GLOBAL_PORT_BIT_MAC(70) |
     GLOBAL_PORT_BIT_MAC(74) |
     /* start DP[3] */
     GLOBAL_PORT_BIT_MAC(78) |
     GLOBAL_PORT_BIT_MAC(79) |
     GLOBAL_PORT_BIT_MAC(80) |
     GLOBAL_PORT_BIT_MAC(84) |
     GLOBAL_PORT_BIT_MAC(88) |
     GLOBAL_PORT_BIT_MAC(92)   ,/*ended ports 64..95*/
     GLOBAL_PORT_BIT_MAC(96) |
     GLOBAL_PORT_BIT_MAC(100)|
     GLOBAL_PORT_BIT_MAC(105) /*CPU*/ }/*ended ports 96..127*/

};

/* AC5P Armstron 2S ports */
static const CPSS_PORTS_BMP_STC ac5p_Armstron2S_portsBmp = {
    {GLOBAL_PORT_BIT_MAC(0 ) |
     GLOBAL_PORT_BIT_MAC(1 ) |
     GLOBAL_PORT_BIT_MAC(2 ) |
     GLOBAL_PORT_BIT_MAC(6 ) |
     GLOBAL_PORT_BIT_MAC(10) |
     GLOBAL_PORT_BIT_MAC(14) |
     GLOBAL_PORT_BIT_MAC(18) |
     GLOBAL_PORT_BIT_MAC(22) |
    /* start DP[1] */
     GLOBAL_PORT_BIT_MAC(26) |
     GLOBAL_PORT_BIT_MAC(27) |
     GLOBAL_PORT_BIT_MAC(28) ,   /*ended ports 0, 1, 2, 6, 10, 14, 18, 22, 26, 27, 28 */
     GLOBAL_PORT_BIT_MAC(32) |
     GLOBAL_PORT_BIT_MAC(36) |
     GLOBAL_PORT_BIT_MAC(40) |
     GLOBAL_PORT_BIT_MAC(44) |
     GLOBAL_PORT_BIT_MAC(48) |

     /* start DP[2] */
     GLOBAL_PORT_BIT_MAC(52) |
     GLOBAL_PORT_BIT_MAC(53) |
     GLOBAL_PORT_BIT_MAC(54) |
     GLOBAL_PORT_BIT_MAC(58) |
     GLOBAL_PORT_BIT_MAC(62) ,  /*ended ports 32, 36, 40, 44, 48, 52, 53, 54, 58, 62 */
     GLOBAL_PORT_BIT_MAC(66) |
     GLOBAL_PORT_BIT_MAC(70) |
     GLOBAL_PORT_BIT_MAC(74) |

     /* start DP[3] */
     GLOBAL_PORT_BIT_MAC(78) |
     GLOBAL_PORT_BIT_MAC(79) |
     GLOBAL_PORT_BIT_MAC(80) |
     GLOBAL_PORT_BIT_MAC(84) |
     GLOBAL_PORT_BIT_MAC(88) |
     GLOBAL_PORT_BIT_MAC(92) ,  /*ended ports 66, 70, 74, 78, 79, 80, 84, 88, 92 */
     GLOBAL_PORT_BIT_MAC(96) |
     GLOBAL_PORT_BIT_MAC(100)|
     GLOBAL_PORT_BIT_MAC(105) /*CPU*/ /*ended ports 96, 100, 105 */
    }
};

/* AC5P Aldrin3 ports */
static const CPSS_PORTS_BMP_STC ac5p_Aldrin3_portsBmp = {
    {GLOBAL_PORT_BIT_MAC(0 ) |
     GLOBAL_PORT_BIT_MAC(1 ) |
     GLOBAL_PORT_BIT_MAC(2 ) |
     GLOBAL_PORT_BIT_MAC(6 ) |
     GLOBAL_PORT_BIT_MAC(10) |
     GLOBAL_PORT_BIT_MAC(14) |
     GLOBAL_PORT_BIT_MAC(18) |
     GLOBAL_PORT_BIT_MAC(22) |

     /* start DP[1] */
     GLOBAL_PORT_BIT_MAC(26) |
     GLOBAL_PORT_BIT_MAC(27) |
     GLOBAL_PORT_BIT_MAC(28) , /*ended ports 0, 1, 2, 6,10,14,18,22,26,27,28 */


     GLOBAL_PORT_BIT_MAC(32) |
     GLOBAL_PORT_BIT_MAC(36) |
     GLOBAL_PORT_BIT_MAC(40) |
     GLOBAL_PORT_BIT_MAC(44) |
     GLOBAL_PORT_BIT_MAC(48) |
     GLOBAL_PORT_BIT_MAC(52) |
     GLOBAL_PORT_BIT_MAC(53) |
     GLOBAL_PORT_BIT_MAC(54) |
     GLOBAL_PORT_BIT_MAC(58) |
     GLOBAL_PORT_BIT_MAC(62),  /*ended ports 32,36,40,44,48,52,53,54,58,62 */

     GLOBAL_PORT_BIT_MAC(66) |
     GLOBAL_PORT_BIT_MAC(70) |
     GLOBAL_PORT_BIT_MAC(74),   /*ended ports 66, 70, 74 */

     GLOBAL_PORT_BIT_MAC(105) /*CPU*/ } /* ended port 105 */
};

/* AC5P ports */
static const CPSS_PORTS_BMP_STC ac5p_Aldrin3_QSGMII_portsBmp = {
    /*DP[0] */
    {GLOBAL_PORT_BIT_MAC(0 ) |
     GLOBAL_PORT_BIT_MAC(1 ) |
     GLOBAL_PORT_BIT_MAC(2 ) |
     GLOBAL_PORT_BIT_MAC(6 ) |
     GLOBAL_PORT_BIT_MAC(10) |
    /* USX part*/
     GLOBAL_PORT_BIT_MAC(12)| GLOBAL_PORT_BIT_MAC(11)| GLOBAL_PORT_BIT_MAC(13) |

     GLOBAL_PORT_BIT_MAC(14) |
     /* USX part*/
     GLOBAL_PORT_BIT_MAC(16)| GLOBAL_PORT_BIT_MAC(15)| GLOBAL_PORT_BIT_MAC(17) |

     GLOBAL_PORT_BIT_MAC(18) |
     GLOBAL_PORT_BIT_MAC(22) |

    /* DP[1] */
     GLOBAL_PORT_BIT_MAC(26) |
     GLOBAL_PORT_BIT_MAC(27) |
     GLOBAL_PORT_BIT_MAC(28),   /*ended ports 0..28*/

     GLOBAL_PORT_BIT_MAC(32) |
     GLOBAL_PORT_BIT_MAC(36) |
    /* USX part*/
     GLOBAL_PORT_BIT_MAC(38)| GLOBAL_PORT_BIT_MAC(37)| GLOBAL_PORT_BIT_MAC(39) |

     GLOBAL_PORT_BIT_MAC(40) |
    /* USX part*/
     GLOBAL_PORT_BIT_MAC(42)| GLOBAL_PORT_BIT_MAC(41)| GLOBAL_PORT_BIT_MAC(43) |

     GLOBAL_PORT_BIT_MAC(44) |
     GLOBAL_PORT_BIT_MAC(48) |

     GLOBAL_PORT_BIT_MAC(52) |
     GLOBAL_PORT_BIT_MAC(53) |
     GLOBAL_PORT_BIT_MAC(54) |
     GLOBAL_PORT_BIT_MAC(58) |
     GLOBAL_PORT_BIT_MAC(62),   /*ended ports 32..62 */

     GLOBAL_PORT_BIT_MAC(66) |
     GLOBAL_PORT_BIT_MAC(70) |
     GLOBAL_PORT_BIT_MAC(74),   /*ended ports 66..74 */

     GLOBAL_PORT_BIT_MAC(105) /*CPU*/ } /* ended port 105 */

};

/* AC5P XL ports */
static const CPSS_PORTS_BMP_STC ac5p_Aldrin3_XL_portsBmp = {
    {GLOBAL_PORT_BIT_MAC(0 ) |
     GLOBAL_PORT_BIT_MAC(1 ) |
     GLOBAL_PORT_BIT_MAC(2 ) |
     GLOBAL_PORT_BIT_MAC(6 ) |
     GLOBAL_PORT_BIT_MAC(10) |
     GLOBAL_PORT_BIT_MAC(14) |
     GLOBAL_PORT_BIT_MAC(18) |
     GLOBAL_PORT_BIT_MAC(22) |

     /* start DP[1] */
     GLOBAL_PORT_BIT_MAC(26) |
     GLOBAL_PORT_BIT_MAC(27) |
     GLOBAL_PORT_BIT_MAC(28) , /*ended ports 0..28*/

     GLOBAL_PORT_BIT_MAC(32) |
     GLOBAL_PORT_BIT_MAC(36) |
     GLOBAL_PORT_BIT_MAC(40) |
     GLOBAL_PORT_BIT_MAC(44) |
     GLOBAL_PORT_BIT_MAC(48) |

     /* start DP[2] */
     GLOBAL_PORT_BIT_MAC(52) |
     GLOBAL_PORT_BIT_MAC(53) |
     GLOBAL_PORT_BIT_MAC(54) |
     GLOBAL_PORT_BIT_MAC(58) |
     GLOBAL_PORT_BIT_MAC(62) , /*ended ports 32..63*/
     GLOBAL_PORT_BIT_MAC(66) |
     GLOBAL_PORT_BIT_MAC(70) |
     GLOBAL_PORT_BIT_MAC(74) |
     /* start DP[3] */
     GLOBAL_PORT_BIT_MAC(78) |
     GLOBAL_PORT_BIT_MAC(79) |
     GLOBAL_PORT_BIT_MAC(80) |
     GLOBAL_PORT_BIT_MAC(84) |
     GLOBAL_PORT_BIT_MAC(88) |
     GLOBAL_PORT_BIT_MAC(92) , /*ended ports 64..95*/

     GLOBAL_PORT_BIT_MAC(96) |
     GLOBAL_PORT_BIT_MAC(100)|
     GLOBAL_PORT_BIT_MAC(105) /*CPU*/ }/*ended ports 96..127*/
};

/* AC5X devices */
static const  CPSS_PP_DEVICE_TYPE ac5x_legalDevTypes[] =
{
    PRV_CPSS_AC5X_ALL_DEVICES       ,

    END_OF_TABLE};

/* AC5X ports */
static const CPSS_PORTS_BMP_STC ac5x_portsBmp = {
    {
     /* 6*8 USX MACs */
     PORT_RANGE_MAC(0,16) | PORT_RANGE_MAC(16,16),
     PORT_RANGE_MAC(32,16) |
     /* 2 'CPU' MAC + 4 100G MAC*/
     PORT_RANGE_MAC(48, 6)
    }
};

static const CPSS_PORTS_BMP_STC ac5x_24G_6X25G_portsBmp = {
    {
     /* 3*8 USX MACs */
     PORT_RANGE_MAC(0,16) | PORT_RANGE_MAC(16,8),
     /* 2 'CPU' MAC + 4 100G MAC*/
     PORT_RANGE_MAC(48, 6)
    }
};

static const CPSS_PORTS_BMP_STC ac5x_16G_6X25G_portsBmp = {
    {
     /* 2*8 USX MACs */
     PORT_RANGE_MAC(0,16),
     /* 2 'CPU' MAC + 4 100G MAC*/
     PORT_RANGE_MAC(48, 6)
    }
};

static const CPSS_PORTS_BMP_STC ac5x_12X25G_portsBmp = {
    {
     GLOBAL_PORT_BIT_MAC(0)  |GLOBAL_PORT_BIT_MAC(1) |GLOBAL_PORT_BIT_MAC(2) |GLOBAL_PORT_BIT_MAC(3)|
     GLOBAL_PORT_BIT_MAC(8)  |GLOBAL_PORT_BIT_MAC(9) |GLOBAL_PORT_BIT_MAC(10)|GLOBAL_PORT_BIT_MAC(11)|
     GLOBAL_PORT_BIT_MAC(16) |GLOBAL_PORT_BIT_MAC(17)|GLOBAL_PORT_BIT_MAC(18)|GLOBAL_PORT_BIT_MAC(19)|
     GLOBAL_PORT_BIT_MAC(24) |GLOBAL_PORT_BIT_MAC(25)|GLOBAL_PORT_BIT_MAC(26)|GLOBAL_PORT_BIT_MAC(27),
     GLOBAL_PORT_BIT_MAC(32) |GLOBAL_PORT_BIT_MAC(33)|GLOBAL_PORT_BIT_MAC(34)|GLOBAL_PORT_BIT_MAC(35)|
     GLOBAL_PORT_BIT_MAC(40) |GLOBAL_PORT_BIT_MAC(41)|GLOBAL_PORT_BIT_MAC(42)|GLOBAL_PORT_BIT_MAC(43)|
     GLOBAL_PORT_BIT_MAC(48) |
     GLOBAL_PORT_BIT_MAC(49) |
     GLOBAL_PORT_BIT_MAC(50) |
     GLOBAL_PORT_BIT_MAC(51) |
     GLOBAL_PORT_BIT_MAC(52) |
     GLOBAL_PORT_BIT_MAC(53)
    }
};

static const CPSS_PORTS_BMP_STC ac5x_8X25G_portsBmp = {
    {
     GLOBAL_PORT_BIT_MAC(0)  |GLOBAL_PORT_BIT_MAC(1)|GLOBAL_PORT_BIT_MAC(2) |GLOBAL_PORT_BIT_MAC(3)|
     GLOBAL_PORT_BIT_MAC(8)  |GLOBAL_PORT_BIT_MAC(9)|GLOBAL_PORT_BIT_MAC(10)|GLOBAL_PORT_BIT_MAC(11),
     GLOBAL_PORT_BIT_MAC(48) |
     GLOBAL_PORT_BIT_MAC(49) |
     GLOBAL_PORT_BIT_MAC(50) |
     GLOBAL_PORT_BIT_MAC(51) |
     GLOBAL_PORT_BIT_MAC(52) |
     GLOBAL_PORT_BIT_MAC(53)
    }
};

/* Harrier devices */
static const  CPSS_PP_DEVICE_TYPE harrier_legalDevTypes[] =
{
    PRV_CPSS_HARRIER_ALL_DEVICES,
    END_OF_TABLE
};
/* Harrier ports of 50G */
/* NOTE : the 'odd' numbers are for the DP ports that represents 'preemption channels' of the 'even' ports */
static const CPSS_PORTS_BMP_STC harrier_portsBmp = {
    {
     /* start DP[0] */
     GLOBAL_PORT_BIT_MAC(0 ) |
     GLOBAL_PORT_BIT_MAC(2 ) |
     GLOBAL_PORT_BIT_MAC(4 ) |
     GLOBAL_PORT_BIT_MAC(6 ) |
     GLOBAL_PORT_BIT_MAC(8 ) |
     GLOBAL_PORT_BIT_MAC(10) |
     GLOBAL_PORT_BIT_MAC(12) |
     GLOBAL_PORT_BIT_MAC(14) |
     /* start DP[1] */
     GLOBAL_PORT_BIT_MAC(16) |
     GLOBAL_PORT_BIT_MAC(18) |
     GLOBAL_PORT_BIT_MAC(20) |
     GLOBAL_PORT_BIT_MAC(22) |
     GLOBAL_PORT_BIT_MAC(24) |
     GLOBAL_PORT_BIT_MAC(26) |
     GLOBAL_PORT_BIT_MAC(28) |
     GLOBAL_PORT_BIT_MAC(30)
     , /*ended ports 0..31*/
     /* start DP[2] */
     GLOBAL_PORT_BIT_MAC(32) |
     GLOBAL_PORT_BIT_MAC(34) |
     GLOBAL_PORT_BIT_MAC(36) |
     GLOBAL_PORT_BIT_MAC(38)
    }

};


/* Ironman-L devices */
static const  CPSS_PP_DEVICE_TYPE ironman_l_legalDevTypes[] =
{
    /* start with the PCIe identification , so the cpssSupportedTypes[] will have it */
    /* other wise 'pcie scan' by the appDemo will fail to recognize the device       */
    CPSS_IRONMAN_L_ALL_DEVICES_CASES_MAC       ,

    END_OF_TABLE
};

/* alias to make line shorter */
#define LION2_CNS           CPSS_PP_FAMILY_DXCH_LION2_E
#define BOBCAT2_CNS         CPSS_PP_FAMILY_DXCH_BOBCAT2_E
#define BOBCAT3_CNS         CPSS_PP_FAMILY_DXCH_BOBCAT3_E
#define ALDRIN_CNS          CPSS_PP_FAMILY_DXCH_ALDRIN_E
#define AC3X_CNS            CPSS_PP_FAMILY_DXCH_AC3X_E
#define XCAT3_CNS           CPSS_PP_FAMILY_DXCH_XCAT3_E
#define AC5_CNS             CPSS_PP_FAMILY_DXCH_AC5_E
#define ALDRIN2_CNS         CPSS_PP_FAMILY_DXCH_ALDRIN2_E
#define PIPE_CNS            CPSS_PX_FAMILY_PIPE_E
#define FALCON_CNS          CPSS_PP_FAMILY_DXCH_FALCON_E
#define AC5P_CNS            CPSS_PP_FAMILY_DXCH_AC5P_E
#define PHOENIX_CNS         CPSS_PP_FAMILY_DXCH_AC5X_E
#define HARRIER_CNS         CPSS_PP_FAMILY_DXCH_HARRIER_E
#define IRONMAN_CNS         CPSS_PP_FAMILY_DXCH_IRONMAN_E

#define NUM_OF_TILES_OFFSET     16
#define NUM_OF_TILES_AND_PORTS(numOfTiles,numOfPorts)   \
    (((numOfTiles) << NUM_OF_TILES_OFFSET) | (numOfPorts))

#define GET_NUM_OF_TILES(value)  ((value) >> NUM_OF_TILES_OFFSET)

#define GET_NUM_OF_PORTS(value)  ((value) & ((1<<NUM_OF_TILES_OFFSET)-1))


static GT_U32   debug_force_numOfPorts = 0;
void debug_force_numOfPorts_set(GT_U32 num)
{
    debug_force_numOfPorts = num;
}

/* cpssSupportedTypes :
    purpose :
        DB to hold the device types that the CPSS support

fields :
    devFamily - device family
    numOfPorts - number of ports in the device
    devTypeArray - array of devices for the device family that has same number
                   of ports
    defaultPortsBmpPtr - pointer to special ports bitmap.
                         if this is NULL , that means that the default bitmap
                         defined by the number of ports that considered to be
                         sequential for 0 to the numOfPorts
    portGroupInfoPtr - port groups info.
                    NULL --> meaning 'non multi port groups' device
                    otherwise hold info about the port groups of the device
    activePortGroupsBmpPtr - active port groups bitmap.
                    relevant only when portGroupInfoPtr != NULL
                    can't be NULL when portGroupInfoPtr != NULL
*/
typedef struct {
    CPSS_PP_FAMILY_TYPE_ENT     devFamily;
    GT_U32                      numOfPorts;
    const CPSS_PP_DEVICE_TYPE   *devTypeArray;
    const CPSS_PORTS_BMP_STC    *defaultPortsBmpPtr;
    const PORT_GROUPS_INFO_STC  *portGroupInfoPtr;
    const GT_U32                *activePortGroupsBmpPtr;
}CPSS_SUPPORTED_TYPES_STC;
const CPSS_SUPPORTED_TYPES_STC cpssSupportedTypes[] =
{
     /* Lion2 devices */                                             /* active port groups */
    {LION2_CNS ,LION_NUM_PORTS_FROM_LAST_PORT_GROUP_MAC(7) ,lion2_port_groups01234567legalDevTypes, NULL ,&lion2PortGroups,&portGroups01234567bmp},

     /* Lion2 Hooper devices */                                             /* active port groups */
    {LION2_CNS ,LION_NUM_PORTS_FROM_LAST_PORT_GROUP_MAC(3) ,lion2_hooper_port_groups0123legalDevTypes, NULL ,&lionPortGroups,&portGroups0123bmp},

    /* Bobcat2 supports 72 MACs, 74 DMAs. */
    {BOBCAT2_CNS ,72 ,bobcat2_legalDevTypes, NULL,NULL,NULL},

    /* Bobk supports range of 72 MACs, 74 DMAs. */
    /* bobk Caelum : */
    /* actual macs are in : 0..47 (GE) , 56..59 (XLG) , 64..71 (XLG) */
    /* there are also 2 TX DMA : 72(cpu),73(TM) */
    /* there is  also 1 RX DMA : 72(cpu) */
    /* NOTE: same MACs as for 'CPSS_98DX4221_CNS' */
    {BOBCAT2_CNS ,72 ,bobkCaelum_legalDevTypes, &portsBmp0to47_62_56to59_64to71,NULL,NULL},

    /* bobk Cetus : */
    /* actual macs are in : 56..59 (XLG) , 64..71 (XLG) */
    /* there are also 2 TX DMA : 72(cpu),73(TM) */
    /* there is  also 1 RX DMA : 72(cpu) */
    {BOBCAT2_CNS ,72 ,bobkCetus_legalDevTypes,  &portsBmp56to59_62_64to71      ,NULL,NULL},

    /* Aldrin : */
    /* actual macs are in : 0..31 */
    /* there are also 1 external CPU DMA (MG port) : 32*/
    {ALDRIN_CNS ,33 ,aldrin_legalDevTypes,  NULL,NULL,NULL},

    /* AC3X : */
    /* actual macs are in : 0..31 */
    /* there are also 1 external CPU DMA (MG port) : 32*/
    {AC3X_CNS ,33 ,ac3x_legalDevTypes,  NULL,NULL,NULL},

#ifndef GM_USED
    /* Bobcat3 supports 74 MACs, 78 DMAs. */
    {BOBCAT3_CNS ,74 ,bobcat3_legalDevTypes,   NULL,NULL,NULL},
    {BOBCAT3_CNS ,74 ,armstrong_legalDevTypes, NULL,NULL,NULL},
#else /*GM_USED*/
    /* Bobcat3 GM supports single DP : 12 MACs, 13 DMAs. */
    {BOBCAT3_CNS ,12 ,bobcat3_legalDevTypes,   NULL,NULL,NULL},
    {BOBCAT3_CNS ,12 ,armstrong_legalDevTypes, NULL,NULL,NULL},
#endif  /*GM_USED*/

    /* Aldrin2 supports 73 MACs, 77 DMAs. */
    {ALDRIN2_CNS    ,73 ,aldrin2_72_legalDevTypes, NULL,NULL,NULL},

     /* Aldrin2 : */
    /* actual macs are in : 4..11, 16..71 */
    /* there is also 1 external CPU DMA (MG port) : 72*/
    {ALDRIN2_CNS    ,73 ,aldrin2_64_legalDevTypes, &portsBmp4to11_16to72,NULL,NULL},

    /* Aldrin2 : */
    /* actual macs are in : 0..35, 48..59 */
    /* there is also 1 external CPU DMA (MG port) : 47*/
    {ALDRIN2_CNS    ,60 ,aldrin2_48_legalDevTypes, &portsBmp0to35_47to59,NULL,NULL},

    /* Aldrin2 : */
    /* actual macs are in : 0..23 */
    /* there is also 1 external CPU DMA (MG port) : 47*/
    {ALDRIN2_CNS    ,48 ,aldrin2_24_legalDevTypes, &portsBmp0to23_47,NULL,NULL},

    /* Aldrin2 : */
    /* actual macs are in : 0..7, 12..19 */
    /* there is also 1 external CPU DMA (MG port) : 47*/
    {ALDRIN2_CNS    ,48 ,aldrin2_16_legalDevTypes, &portsBmp0to7_12to19_47,NULL,NULL},

    /* xCat3 devices  - 24network, 2stack */
    /* xCat3 devices  - 24network, 2stack */
    {XCAT3_CNS    ,24+2 ,xcat3_24_2legalDevTypes, NULL,NULL,NULL},

    /* xCat3 devices  - 24network, 4stack */
    {XCAT3_CNS    ,24+4 ,xcat3_24_4legalDevTypes, NULL,NULL,NULL},/*ports 0..27 */

    /* xCat3 devices  - 16network, 2stack */
    {XCAT3_CNS    ,24+4 ,xcat3_16_4legalDevTypes, &portsBmp0to15_24to27,NULL,NULL}, /*ports 0..15,24..27 */

    /* xCat3 devices  - 8network, 4stack */
    {XCAT3_CNS    ,24+4 ,xcat3_8_4legalDevTypes, &portsBmp0to7_24to27,NULL,NULL}, /*ports 0..7,24..27 */

    /* xCat3 devices  - 8network, 2stack */
    {XCAT3_CNS    ,24+2 ,xcat3_8_2legalDevTypes, &portsBmp0to7_24_25,NULL,NULL}, /*ports 0..7,24..25 */

    /* AC5 devices  - 24network, 4stack */
    {AC5_CNS      ,24+4 ,ac5_24_4legalDevTypes, NULL,NULL,NULL},/*ports 0..27 */

    /* AC5 devices  - 8network, 4stack */
    {AC5_CNS      ,24+4 ,ac5_8_4legalDevTypes, &portsBmp0to7_24to27,NULL,NULL},/*ports 0..7,24..27 */

    /* PX : PIPE devices  - 12 network, 4 stack */
    {PIPE_CNS     ,12+4 ,pipe_12_4legalDevTypes, &portsBmp0to15,NULL,NULL}, /*ports 0..15 (NOT include CPU port) */

    /* PX : PIPE devices  - 8 network, 2 stack */
    {PIPE_CNS     ,14 ,pipe_8_2legalDevTypes, &portsBmp4to13,NULL,NULL}, /*ports 4..13 (NOT include CPU port) */

    /* PX : PIPE devices  - 8 network, 4 stack */
    {PIPE_CNS     ,16 ,pipe_8_4legalDevTypes, &portsBmp4to15,NULL,NULL}, /*ports 4..15 (NOT include CPU port) */

    /* PX : PIPE devices  - 4 network, 2 stack */
    {PIPE_CNS     ,16 ,pipe_4_2legalDevTypes, &portsBmp4to7_13_15,NULL,NULL}, /*ports 4..7,13,15 (NOT include CPU port) */

#ifndef GM_USED
    /* Falcon 3.2 Tera supports  64 50G MACs + 4 CPU ports (4 Ravens) + 4 SDMAs */
    {FALCON_CNS ,NUM_OF_TILES_AND_PORTS(1,64+4)  ,falcon_3_2_legalDevTypes, NULL, NULL, NULL},
    /* Falcon 6.4 Tera supports 128 50G MACs + 8 CPU ports (8 Ravens) + 8 SDMAs */
    {FALCON_CNS ,NUM_OF_TILES_AND_PORTS(2,128+8) ,falcon_6_4_legalDevTypes, NULL ,NULL, NULL},
    /* Falcon 12.8 Tera supports 256 50G MACs + 16 CPU ports (16 Ravens) + 16 SDMAs */
    {FALCON_CNS ,NUM_OF_TILES_AND_PORTS(4,256+16) ,falcon_12_8_legalDevTypes, NULL, NULL, NULL},
    /* Falcon 8 Tera supports 160 50G MACs + 16 CPU ports */
    {FALCON_CNS ,NUM_OF_TILES_AND_PORTS(4,256+16) ,falcon_160X50G_legalDevTypes,
        &portsBmp0to47_80to95_112to143_160to175_208to223_224to255_CPU_0xE5A7, NULL, NULL},
    /* Falcon 2/4 Tera supports 80 25G/50G MACs + 5 CPU ports (5 Ravens) + 8 SDMAs */
    {FALCON_CNS ,NUM_OF_TILES_AND_PORTS(2,128+8) ,falcon_half_eagle_5_ravens_legalDevTypes,
        &portsBmp0to15_32to47_80to127_CPU_0xE5, NULL, NULL},
    /* Armstrong2 80X25G supports 85 (range 0-135) MACs. */
    {FALCON_CNS ,NUM_OF_TILES_AND_PORTS(2,128+8) ,armstrong2_80X25G_legalDevTypes,
        &portsBmp0to15_32to47_80to127_CPU_0xE5, NULL, NULL},
    /* Armstrong2 96x25G+32x50G supports 128 (range 0-135) MACs. */
    {FALCON_CNS ,NUM_OF_TILES_AND_PORTS(2,128+8) ,armstrong2_96X25G_32X50G_legalDevTypes, NULL, NULL, NULL},
    /* Falcon 2/4 Tera supports 64 50G/50G MACs + 4 CPU ports (4 Ravens) + 8 SDMAs */
    {FALCON_CNS ,NUM_OF_TILES_AND_PORTS(2,128+8) ,falcon_64X50G_legalDevTypes,
        &portsBmp0to15_32to47_80to95_112to127_CPU_0xA5, NULL, NULL},
    /* Falcon 2/4 Tera supports 80 25G/50G MACs + 5 CPU ports (5 Ravens) + 8 SDMAs */
    {FALCON_CNS ,NUM_OF_TILES_AND_PORTS(2,128+8) ,falcon_80X50_legalDevTypes,
        &portsBmp0to15_32to47_64to95_112to127_CPU_0xB5, NULL, NULL},


#else /*GM_USED*/
    /* Falcon GM supports single DP : 8 MACs, 9 DMAs. */
    {FALCON_CNS ,NUM_OF_TILES_AND_PORTS(1,8) ,falcon_GM_legalDevTypes,    NULL,NULL,NULL},
#endif  /*GM_USED*/

    /* AC5P 1.6 Tera supports  105 MACs, +4 SDMAs. */
    {AC5P_CNS , 106  ,ac5p_legalDevTypes,   &ac5p_portsBmp,NULL,NULL},
    {AC5P_CNS , 106  ,ac5p_XL_legalDevTypes,   &ac5p_XL_portsBmp,NULL,NULL},
    {AC5P_CNS , 106  ,ac5p_Armstrong2S_legalDevTypes,   &ac5p_Armstron2S_portsBmp,NULL,NULL},
    {AC5P_CNS , 106  ,ac5p_Aldrin3_legalDevTypes,   &ac5p_Aldrin3_portsBmp,NULL,NULL},
    {AC5P_CNS , 106  ,ac5p_Aldrin3_QSGMII_legalDevTypes, &ac5p_Aldrin3_QSGMII_portsBmp,NULL,NULL},
    {AC5P_CNS , 106  ,ac5p_Aldrin3_XL_legalDevTypes,  &ac5p_Aldrin3_XL_portsBmp,NULL,NULL},



    /* AC5X supports 54 MACs, + 4 SDMAs. */
    {PHOENIX_CNS , 54  ,phoenix_48G_6x25g_legalDevTypes, &ac5x_portsBmp, NULL, NULL},
    {PHOENIX_CNS , 54  ,phoenix_24G_6x25g_legalDevTypes, &ac5x_24G_6X25G_portsBmp, NULL, NULL},
    {PHOENIX_CNS , 54  ,phoenix_16G_6x25g_legalDevTypes, &ac5x_16G_6X25G_portsBmp, NULL, NULL},
    {PHOENIX_CNS , 54  ,phoenix_12X25G_legalDevTypes, &ac5x_12X25G_portsBmp, NULL, NULL},
    {PHOENIX_CNS , 54  ,phoenix_8X25G_legalDevTypes, &ac5x_8X25G_portsBmp, NULL, NULL},

    /* Harrier 1000G (1.0T) supports  20 MACs (but on '40 ports range') , +2 SDMAs. */
    {HARRIER_CNS , 40  ,harrier_legalDevTypes,   &harrier_portsBmp,NULL,NULL},

    /* Ironman L. */
    {IRONMAN_CNS , 55  ,ironman_l_legalDevTypes,   NULL,NULL,NULL},

    /* End of list      */
    {(CPSS_PP_FAMILY_TYPE_ENT)END_OF_TABLE   ,0 , NULL,NULL, NULL, NULL}
};

/*const*/ CPSS_SUPPORTED_TYPES_STC onEmulator_cpssSupportedTypes[] =
{
    /* Aldrin : limited number of ports */
    /* actual macs are in : 24..31 */
    /* there is also 1 external CPU DMA (MG port) : 32*/
    /* NOTE: this line is 'modified' at run time if running in 'Aldrin FULL' mode */
    /*       modified to use 'NULL' instead of '&portsBmp24to32' */
    {ALDRIN_CNS ,33 ,aldrin_legalDevTypes,  &portsBmp24to32,NULL,NULL},
    {ALDRIN2_CNS,73 ,aldrin2_72_legalDevTypes, NULL,NULL,NULL},

    /* PX : PIPE devices  - 12 network, 4 stack */
    {PIPE_CNS    ,12+4 ,pipe_12_4legalDevTypes, &portsBmp0to15,NULL,NULL}, /*ports 0..15 (NOT include CPU port) */

    /* PX : PIPE devices  - 8 network, 2 stack */
    {PIPE_CNS    ,14 ,pipe_8_2legalDevTypes, &portsBmp4to13,NULL,NULL},

    /* PX : PIPE devices  - 8 network, 4 stack */
    {PIPE_CNS    ,16 ,pipe_8_4legalDevTypes, &portsBmp4to15,NULL,NULL},

    /* PX : PIPE devices  - 4 network, 2 stack */
    {PIPE_CNS    ,16 ,pipe_4_2legalDevTypes, &portsBmp4to7_13_15,NULL,NULL},

#ifndef GM_USED
    /* Falcon 3.2 Tera supports  65 MACs, +3 SDMAs. */
    {FALCON_CNS ,NUM_OF_TILES_AND_PORTS(1,64+2)  ,falcon_3_2_legalDevTypes,   NULL,NULL,NULL},
    /* Falcon 6.4 Tera supports 130 MACs, +6 SDMAs. */
    {FALCON_CNS ,NUM_OF_TILES_AND_PORTS(2,128+2) ,falcon_6_4_legalDevTypes,   NULL,NULL,NULL},
    /* Falcon 12.8 Tera supports 258 MACs, +6 SDMAs. */
    {FALCON_CNS ,NUM_OF_TILES_AND_PORTS(4,256+2) ,falcon_12_8_legalDevTypes,  NULL,NULL,NULL},
#endif  /*!GM_USED*/

    /* AC5P 1.6 Tera supports  105 MACs, +4 SDMAs. */
    {AC5P_CNS , 106  ,ac5p_legalDevTypes,   &ac5p_portsBmp,NULL,NULL},
    {AC5P_CNS , 106  ,ac5p_XL_legalDevTypes,   &ac5p_XL_portsBmp,NULL,NULL},
    {AC5P_CNS , 106  ,ac5p_Armstrong2S_legalDevTypes,   &ac5p_Armstron2S_portsBmp,NULL,NULL},
    {AC5P_CNS , 106  ,ac5p_Aldrin3_legalDevTypes,   &ac5p_Aldrin3_portsBmp,NULL,NULL},
    {AC5P_CNS , 106  ,ac5p_Aldrin3_QSGMII_legalDevTypes, &ac5p_Aldrin3_QSGMII_portsBmp,NULL,NULL},
    {AC5P_CNS , 106  ,ac5p_Aldrin3_XL_legalDevTypes,  &ac5p_Aldrin3_XL_portsBmp,NULL,NULL},

    /* AC5X supports  54 MACs, +2 SDMAs. */
    {PHOENIX_CNS , 54  ,ac5x_legalDevTypes,&ac5x_portsBmp,NULL,NULL},
    /* xCat3 devices  - 24network, 4stack */
    {XCAT3_CNS    ,24+4 ,xcat3_24_4legalDevTypes, NULL,NULL,NULL},/*ports 0..27 */
    /* AC5 devices  - 24network, 4stack */
    {AC5_CNS      ,24+4 ,ac5_24_4legalDevTypes, NULL,NULL,NULL},/*ports 0..27 */

    /* Harrier 1000G (1.0T) supports  20 MACs (but on '40 ports range') , +2 SDMAs. */
    {HARRIER_CNS , 40  ,harrier_legalDevTypes,   &harrier_portsBmp,NULL,NULL},

    /* Ironman L. */
    {IRONMAN_CNS , 55  ,ironman_l_legalDevTypes,   NULL,NULL,NULL},

    /* End of list      */
    {(CPSS_PP_FAMILY_TYPE_ENT)END_OF_TABLE   ,0 , NULL,NULL, NULL, NULL}
};


typedef struct{
    GT_U32  regAddr;
    GT_U32  regValue;
}DEFAULT_REG_STC;

/*
    on emulator - Aldrin (no limitations on units):
    below the registers' values in the MG for neutralizing the missing units.
    For the missing units you should receive "badad" if you try to read them
    and the write will be lost.
*/
static const DEFAULT_REG_STC onEmulator_AldrinDefaultRegArr_full[] =
{
     {0x000000E4        ,0x70FDE87F}
    ,{0x000000E8        ,0xF820E0C7}
    ,{0x000000EC        ,0xFF30FF81}
    ,{0x000000F0        ,0xFFA     }
    ,{0x00000140        ,0x00008100}
    /* must be last */
    ,{END_OF_TABLE      ,END_OF_TABLE}
};

/* support formula of addresses like : 0x13000000 + 0x1000*s: where s (0-32) represents serdes */
typedef struct{
    GT_U32  baseRegAddr;
    GT_U32  stepsInBytes;
    GT_U32  startStepIndex;
    GT_U32  numOfSteps;
    GT_U32  regValue;
}DEFAULT_REG_FORMULA_STC;

/* un-reset the SERDESes (set bits 2,3) */
#define UNRESET_SERDES_CNS  0x0000880d
#define OPEN_SERDES_LINK_CNS     0 /* 0 - not open SERDESes , 1 - open SERDESes */
/*
    on emulator - Aldrin  - support formulas
    0x13000000 + 0x1000*s: where s (0-32) represents serdes
*/
static const DEFAULT_REG_FORMULA_STC onEmulator_AldrinDefaultRegArr_full_formula[] =
{
#if OPEN_SERDES_LINK_CNS
    /* un-reset the SERDESes (set bits 2,3) */
     {0x13000004        ,0x1000/*steps between SERDESs*/ ,0/*start SERDES*/ , 33 /*number of SERDESes*/ , UNRESET_SERDES_CNS /* value*/}
    /* must be last */
    ,
#endif /*OPEN_SERDES_LINK_CNS*/
    {END_OF_TABLE      ,END_OF_TABLE , END_OF_TABLE , END_OF_TABLE , END_OF_TABLE}
};

/*
    on emulator - Aldrin2 - support formulas
    0x13000000 + 0x1000 * (a-9) + 0x80000: where a (72-72) represents CPU_SERDES
    0x13000000 + 0x1000 * (a-24) + 0x80000: where a (24-71) represents SERDES_Num
    0x13000000 + 0x1000 * a: where a (0-23) represents SERDES_Num.
*/
static const DEFAULT_REG_FORMULA_STC onEmulator_Aldrin2DefaultRegArr_formula[] =
{
#if OPEN_SERDES_LINK_CNS
    /* un-reset the SERDESes (set bits 2,3) */
     {0x13000004            ,0x1000/*steps between SERDESs*/ ,    0/*start SERDES*/ , 24    /*number of SERDESes*/ , UNRESET_SERDES_CNS /* value*/}
    ,{0x13000004 + 0x80000  ,0x1000/*steps between SERDESs*/ ,24-24/*start SERDES*/ , 72-24 /*number of SERDESes*/ , UNRESET_SERDES_CNS /* value*/}
    ,{0x13000004 + 0x80000  ,0x1000/*steps between SERDESs*/ ,72- 9/*start SERDES*/ , 1     /*number of SERDESes*/ , UNRESET_SERDES_CNS /* value*/}
    ,
#endif /*OPEN_SERDES_LINK_CNS*/

    /* must be last */
    {END_OF_TABLE      ,END_OF_TABLE , END_OF_TABLE , END_OF_TABLE , END_OF_TABLE}
};

/*
    on emulator - Pipe  - support formulas
    0x13000000 + 0x1000*a : where a (0-15) represents SERDES_Num
*/
static const DEFAULT_REG_FORMULA_STC onEmulator_PipeDefaultRegArr_formula[] =
{
#if OPEN_SERDES_LINK_CNS
    /* un-reset the SERDESes (set bits 2,3) */
     {0x13000004        ,0x1000/*steps between SERDESs*/ ,0/*start SERDES*/ , 16 /*number of SERDESes*/ , UNRESET_SERDES_CNS /* value*/}
    ,
#endif /*OPEN_SERDES_LINK_CNS*/

    /* must be last */
    {END_OF_TABLE      ,END_OF_TABLE , END_OF_TABLE , END_OF_TABLE , END_OF_TABLE}
};

/* cpssSpecialDevicesBmp -
 * Purpose : DB to hold the devices with special ports BMP
 *
 * NOTE : devices that his port are sequential for 0 to the numOfPorts
 *        not need to be in this Array !!!
 *
 * fields :
 *  devType - device type that has special ports bmp (that is different from
 *            those of other devices of his family with the same numOfPort)
 *  existingPorts - the special ports bmp of the device
 *
*/
static const struct {
    CPSS_PP_DEVICE_TYPE         devType;
    CPSS_PORTS_BMP_STC          existingPorts;
}cpssSpecialDevicesBmp[] =
{
    /* bobcat2 MACs : 0..23 , 56..59 , 64..71 */
    {CPSS_98DX4220_CNS , PORTS_BMP_0_TO_23_56_TO_59_64_TO_71},

    /* bobcat2 MACs : 0..47, 56..59 , 64..71 */
    {CPSS_98DX4221_CNS , PORTS_BMP_0_TO_47_56_TO_59_64_TO_71},

    /* bobcat2 MACs : 0..47, 60..71 */
    {CPSS_98DX4222_CNS , {{PORT_RANGE_MAC(0,16) | PORT_RANGE_MAC(16,16) , PORT_RANGE_MAC(32,16) | PORT_RANGE_MAC(60,4) , PORT_RANGE_MAC(64,8) , 0 }}},

    /* bobcat2 MACs : 52..55 , 56..59, 64..67, 68..71 */
    {CPSS_98DX8216_CNS , {{0, PORT_RANGE_MAC(52,8) , PORT_RANGE_MAC(64,8) , 0 }}},

    /* bobcat2 MACs : 0...8, 48..63, 64..71 */
    {CPSS_98DX8219_CNS , {{PORT_RANGE_MAC(0,8), PORT_RANGE_MAC(48,16),  PORT_RANGE_MAC(64,8) , 0 }}},

    /* bobcat2 MACs : 48..51 , 52..55 , 56..59, 60..63, 64..67, 68..71 */
    {CPSS_98DX8224_CNS , {{0, PORT_RANGE_MAC(48,16) , PORT_RANGE_MAC(64,8) , 0 }}},

    /* bobcat2 MACs : 32..47, 52..59 */
    {CPSS_98DX4253_CNS , {{0, PORT_RANGE_MAC(32,16) | PORT_RANGE_MAC(52,8) , 0 , 0 }}},

    /* Cetus MACs : 64..67, 68..71 */
    {CPSS_98DX8208_CNS , {{0, PORT_RANGE_MAC(62,1), PORT_RANGE_MAC(64,8) , 0 }}},

    /* Caelum MACs : 0..23 , 56..59 , 62, 64..71 */
    {CPSS_98DX4204_CNS , PORTS_BMP_0_TO_23_56_TO_59_62_64_TO_71},

    /* Caelum MACs : 0..23 , 56..59 , 62, 64..71 */
    {CPSS_98DX4210_CNS , PORTS_BMP_0_TO_23_56_TO_59_62_64_TO_71},

    /* Caelum MACs : 0..23 , 56..59 , 62, 64..71 */
    {CPSS_98DX3346_CNS,  PORTS_BMP_0_TO_23_56_TO_59_62_64_TO_71},

    /* Caelum MACs : 0..27 , 56..59 , 62, 64..71 */
    {CPSS_98DX3345_CNS,  PORTS_BMP_0_TO_27_56_TO_59_62_64_TO_71},

    /* Caelum MACs : 0..27 , 56..59 , 62, 64..71 */
    {CPSS_98DX4212_CNS , PORTS_BMP_0_TO_27_56_TO_59_62_64_TO_71},

    /* Aldrin Z0 MACs : 0, 4, 8 , 12, 16, 20, 24, 28, 32, 36, 40, 44, 56..59 , 62, 64..71 */
    {CPSS_98DX8332_Z0_CNS , PORTS_BMP_0_4_8_12_16_20_24_28_32_36_40_44_56_TO_59_62_64_TO_71},

    /* Bobcat3 - reduced MACs : 0..3, 8..11, 16..23, 28..39, 44..47, 52..59, 64..73 */
    {CPSS_98CX8405_CNS, PORTS_BMP_0_TO_3_8_TO_11_16_TO_23_28_TO_39_44_TO_47_52_TO_59_64_TO_73},

    /* Aldrin MACs: 0..7, 32 */
    {CPSS_98DX8308_CNS, {{PORT_RANGE_MAC(0,8), 0x00000001}}},

    /* Aldrin MACs: 0..11, 32 */
    {CPSS_98DX8312_CNS, {{PORT_RANGE_MAC(0,12), 0x00000001}}},

    /* Aldrin MACs: 0..15, 32 */
    {CPSS_98DX8316_CNS, {{PORT_RANGE_MAC(0,16), 0x00000001}}},

    /* Aldrin MACs: 0..23, 32 */
    {CPSS_98DX8324_CNS, {{PORT_RANGE_MAC(0,24), 0x00000001}}},

    /* AC3X MACs: 0..17, 32 */
    {CPSS_98DX3256_CNS, {{PORT_RANGE_MAC(0,18), 0x00000001}}},

    /* AC3X MACs: 0..23, 32 */
    {CPSS_98DX3257_CNS, {{PORT_RANGE_MAC(0,24), 0x00000001}}},

    /* Aldrin MACs: 0..15, 32 */
    {CPSS_98DXH831_CNS, {{PORT_RANGE_MAC(0,16), 0x00000001}}},

    /* Aldrin MACs: 0..23, 32 */
    {CPSS_98DXH832_CNS, {{PORT_RANGE_MAC(0,24), 0x00000001}}},

    /* Aldrin MACs: 0..21, 32 */
    {CPSS_98DXH834_CNS, {{PORT_RANGE_MAC(0,22), 0x00000001}}},

    /* Aldrin MACs: 0..29, 32 */
    {CPSS_98DXH835_CNS, {{PORT_RANGE_MAC(0,30), 0x00000001}}},

    /* End of list      */
    {END_OF_TABLE   ,{{0,0}} }
};

#ifdef CHX_FAMILY

static GT_STATUS prvCpssDrvLion2FirstPacketWA
(
    IN GT_U8    devNum,
    IN GT_U32   portGroupId,
    IN PRV_CPSS_DRV_PP_PHASE_1_INPUT_INFO_STC  *ppInInfoPtr
);

static GT_STATUS prvCpssDrvLion2MMPcsUnReset
(
    GT_U8                   devNum,
    GT_U32                  portGroupId
);

#endif /* CHX_FAMILY */

#ifdef ASIC_SIMULATION_H_EXIST
typedef enum{
    BIG_ENDIAN_E = 1, /* HOST is 'Big endian' */
    LITTLE_ENDIAN_E   /* HOST is 'Little endian' */
}ENDIAN_ENT;


/**
* @internal checkHostEndian function
* @endinternal
*
* @brief   This function check if the HOST is 'Big endian' or 'Little endian'
*
* @retval one of ENDIAN_ENT        - BIG_ENDIAN_E / LITTLE_ENDIAN_E
*/
static ENDIAN_ENT  checkHostEndian(void)
{
    GT_U32  wordMem = 0x11223344;
    GT_U8  *charMemPtr = (GT_U8*)(&wordMem);

    if(charMemPtr[0] == 0x11)
    {
        /* 'Big endian' - the bits 24..31 sit in byte0 */
        return BIG_ENDIAN_E;
    }

    /* 'Little endian' - the bits 24..31 sit in byte3 */
    return LITTLE_ENDIAN_E;
}
#endif /*ASIC_SIMULATION_H_EXIST*/


/**
* @internal phase1Part1Init function
* @endinternal
*
* @brief   This function is called by prvCpssDrvHwPpPhase1Init()
*         the function bind the PCI/SMI/TWSI driver functions to the driver object
*         functions for this device.
* @param[in] devNum                   - The PP's device number to be initialized.
* @param[in] ppInInfoPtr              - (pointer to)   the info needed for initialization of
*                                      the driver for this PP
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - bad management interface value
* @retval GT_NOT_IMPLEMENTED       - the needed driver was not compiled correctly
*/
static GT_STATUS phase1Part1Init
(
    IN  GT_U8                devNum,
    IN  PRV_CPSS_DRV_PP_PHASE_1_INPUT_INFO_STC  *ppInInfoPtr
)
{
    GT_STATUS rc;

    /********************************************/
    /* bind the BUS part to the ExMxDx BUS part */
    /********************************************/


    /* set rc as "not implemented" to suggest that the specific SMI/PCI/TWSI is
       not part of the image although needed in the image */
    rc = GT_NOT_IMPLEMENTED;
    /* driver object bind */
    if(ppInInfoPtr->mngInterfaceType == CPSS_CHANNEL_PCI_E)
    {
#if defined GT_PCI  /*|| defined GT_PEX*/
        rc = GT_OK;
        if(ppInInfoPtr->ppHAState == CPSS_SYS_HA_MODE_ACTIVE_E)
        {
            PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_OBJ_CONFIG[devNum]->busPtr = prvCpssDrvMngInfPciPtr;
        }
        else
        {
            PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_OBJ_CONFIG[devNum]->busPtr = prvCpssDrvMngInfPciHaStandbyPtr;
        }
#endif
    }
    /* SMI */
    else if (ppInInfoPtr->mngInterfaceType == CPSS_CHANNEL_SMI_E)
    {
#ifdef GT_SMI
        rc = GT_OK;
        PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_OBJ_CONFIG[devNum]->busPtr = prvCpssDrvMngInfSmiPtr;
#endif
    }
 /* TWSI */
    else if (ppInInfoPtr->mngInterfaceType == CPSS_CHANNEL_TWSI_E)
    {
#ifdef GT_I2C
        rc = GT_OK;
        PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_OBJ_CONFIG[devNum]->busPtr = prvCpssDrvMngInfTwsiPtr;
#endif
    }
    else if(CPSS_CHANNEL_IS_PEX_MAC(ppInInfoPtr->mngInterfaceType))
    {
#if defined GT_PCI  /*|| defined GT_PEX*/
        rc = GT_OK;
        if(ppInInfoPtr->ppHAState == CPSS_SYS_HA_MODE_ACTIVE_E)
        {
            PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_OBJ_CONFIG[devNum]->busPtr = prvCpssDrvMngInfPciPtr;
        }
        else
        {
            PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_OBJ_CONFIG[devNum]->busPtr = prvCpssDrvMngInfPciHaStandbyPtr;
        }
#endif
    }
    else if(CPSS_CHANNEL_IS_PEX_MBUS_MAC(ppInInfoPtr->mngInterfaceType))
    {
#if defined GT_PCI  /*|| defined GT_PEX*/
        rc = GT_OK;
        if(ppInInfoPtr->ppHAState == CPSS_SYS_HA_MODE_ACTIVE_E)
        {
            PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_OBJ_CONFIG[devNum]->busPtr = prvCpssDrvMngInfPexMbusPtr;
        }
        else
        {
            PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_OBJ_CONFIG[devNum]->busPtr = prvCpssDrvMngInfPciHaStandbyPtr;
        }
#endif
    }
    else
    {
        /* unknown interface type */
        rc = GT_BAD_PARAM;
    }
    return rc;
}

/**
* @internal phase1Part2Init function
* @endinternal
*
* @brief   This function is called by prvCpssDrvHwPpPhase1Init()
*         This function allocate the memory for the DB of the driver for this
*         device , and set some of it's values.
* @param[in] devNum                   - The PP's device number to be initialized.
* @param[in] ppInInfoPtr              - (pointer to)   the info needed for initialization of
*                                      the driver for this PP
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_OUT_OF_CPU_MEM        - failed to allocate CPU memory,
* @retval GT_BAD_STATE             - the driver is not in state that ready for initialization
*                                       for the device
*/
static GT_STATUS phase1Part2Init
(
    IN  GT_U8                devNum,
    IN  PRV_CPSS_DRV_PP_PHASE_1_INPUT_INFO_STC  *ppInInfoPtr
)
{
    PRV_CPSS_DRIVER_PP_CONFIG_STC *ppConfigPtr;/* Points to the Device's Driver*/
    GT_U32      portGroupId;/* the port group Id */
    GT_BOOL     activePortGroupFound = GT_FALSE; /*indication that there is active port group */

    if(PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[devNum] != NULL)
    {
        /* the device already exists ?! */
        /* we can't override it ! , the device must be empty before we
           initialize it */
        /* check that "remove device" was done properly */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }

    /* allocate the device a DB */
    ppConfigPtr = cpssOsMalloc(sizeof(PRV_CPSS_DRIVER_PP_CONFIG_STC));
    if(ppConfigPtr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
    }

    /* set default values */
    cpssOsMemSet(ppConfigPtr,0,sizeof(PRV_CPSS_DRIVER_PP_CONFIG_STC));
    /* bind the memory to the global DB */
    PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[devNum] = ppConfigPtr;

    /* set value */
    ppConfigPtr->hwDevNum = devNum;
    ppConfigPtr->drvHwIfSupportedBmp = PRV_CPSS_HW_IF_BMP_MAC(ppInInfoPtr->mngInterfaceType);
    ppConfigPtr->mngInterfaceType = ppInInfoPtr->mngInterfaceType;
    if (CPSS_CHANNEL_IS_PCI_COMPATIBLE_MAC(ppInInfoPtr->mngInterfaceType))
    {
        /* in most cases we will treat the "PCI" and "PEX" the same */
        ppConfigPtr->drvHwIfSupportedBmp |= PRV_CPSS_HW_IF_BMP_MAC(CPSS_CHANNEL_PCI_E);
    }

    PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_OBJ_CONFIG[devNum]->HAState = ppInInfoPtr->ppHAState;

    for(portGroupId = 0; portGroupId < ppInInfoPtr->numOfPortGroups; portGroupId++)
    {
        ppConfigPtr->hwInfo[portGroupId] = ppInInfoPtr->hwInfo[portGroupId];

        if(ppConfigPtr->hwInfo[portGroupId].busType != CPSS_HW_INFO_BUS_TYPE_NONE_E)
        {
            /* active port group */
            activePortGroupFound = GT_TRUE;
        }

        ppConfigPtr->hwCtrl[portGroupId].isrAddrCompletionRegionsBmp  =
            ppInInfoPtr->isrAddrCompletionRegionsBmp;
        ppConfigPtr->hwCtrl[portGroupId].appAddrCompletionRegionsBmp  =
            ppInInfoPtr->appAddrCompletionRegionsBmp;

    }

    if(activePortGroupFound == GT_FALSE)
    {
        /* at least one port group need to be active */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    return GT_OK;
}

/**
* @internal prvCpssDrvHwPpPrePhase1NextDevFamilySet function
* @endinternal
*
* @brief   This function called before 'phase 1' init of the cpss and the cpssDriver.
*         in order to give 'hint' for the cpssDriver about the family of the device
*         that will be 'next' processed by prvCpssDrvHwPpPhase1Init(...).
*
* @retval GT_OK                    - on success
*/
GT_STATUS prvCpssDrvHwPpPrePhase1NextDevFamilySet(IN CPSS_PP_FAMILY_TYPE_ENT devFamily)
{
    nextDevFamily = devFamily;
    return GT_OK;
}

/**
* @internal prvCpssDrvHwPpPrePhase1NextDevFamilyGet function
* @endinternal
*
* @brief   return the 'nextDevFamily' that was set by prvCpssDrvHwPpPrePhase1NextDevFamilySet().
*
* @retval GT_OK                    - on success
*/
CPSS_PP_FAMILY_TYPE_ENT prvCpssDrvHwPpPrePhase1NextDevFamilyGet(GT_VOID)
{
    return nextDevFamily;
}

/**
 * @struct PRV_DEV_FAMILY_PREFIX_STC
 *
 *  @brief struct holds prefix of device ID and family
 *
 */
typedef struct {
    /** prefix (MSBs of device ID) of family */
    GT_U32                  prefix;
    /** family of device */
    CPSS_PP_FAMILY_TYPE_ENT devFamily;
} PRV_DEV_FAMILY_PREFIX_STC;

/** defines device ID bits those define family.
 *  Falcon and AC5P use bits 15:10, other devices use bits 15:8.
 *  it's sufficient to use bits 15:10 for all devices for right recognition
 *  of family */
#define PRV_FAMILY_PREFIX_MASK_CNS 0xFC00

/** MACRO to calculate prefix from full device ID */
#define PRV_FAMILY_PREFIX_GET_MAC(_deviceId) ((_deviceId >> 16) & PRV_FAMILY_PREFIX_MASK_CNS)

/** size of prefixes DB */
#define PRV_FAMILY_PREFIX_DB_SIZE_CNS 7

/**
* @internal sip6DeviceFamilyGet function
* @endinternal
*
* @brief   This function detects device family for SIP_6 device managed by PCI Express
*          or internal CPU bus.
*
* @param[in] devNum                   - The PP's device number to be initialized.
* @param[in] portGroupId              - port group Id.
* @param[in] ppInInfoPtr              - (pointer to)   the info needed for initialization of
*                                      the driver for this PP
*
* @param[out] devFamilyPtr            - (pointer to) family of device
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
*/
static GT_STATUS sip6DeviceFamilyGet
(
    IN  GT_U8                devNum,
    IN  GT_U32               portGroupId,
    IN  PRV_CPSS_DRV_PP_PHASE_1_INPUT_INFO_STC  *ppInInfoPtr,
    OUT CPSS_PP_FAMILY_TYPE_ENT                 *devFamilyPtr
)
{
    GT_STATUS rc;       /* return code        */
    GT_U32    regAddr;  /* address of register */
    GT_U32    deviceId; /* value of device ID */
    GT_U32    prefix;   /* value of device ID prefix */
    GT_U32    ii;       /* iterator */
    PRV_DEV_FAMILY_PREFIX_STC prefixDb[PRV_FAMILY_PREFIX_DB_SIZE_CNS] = /* use representative device from each family */
                {{PRV_FAMILY_PREFIX_GET_MAC(CPSS_FALCON_ON_PEX_CNS), CPSS_PP_FAMILY_DXCH_FALCON_E},
                 {PRV_FAMILY_PREFIX_GET_MAC(CPSS_AC5P_ON_PEX_CNS),   CPSS_PP_FAMILY_DXCH_AC5P_E},
                 {PRV_FAMILY_PREFIX_GET_MAC(CPSS_98DXC720_CNS),      CPSS_PP_FAMILY_DXCH_HARRIER_E},
                 {PRV_FAMILY_PREFIX_GET_MAC(CPSS_98CN106xxS_CNS),    CPSS_PP_FAMILY_DXCH_HARRIER_E},
                 {PRV_FAMILY_PREFIX_GET_MAC(CPSS_98DX3500M_CNS),     CPSS_PP_FAMILY_DXCH_AC5X_E},
                 {PRV_FAMILY_PREFIX_GET_MAC(CPSS_98DX2538_CNS),      CPSS_PP_FAMILY_DXCH_AC5_E},
                 {PRV_FAMILY_PREFIX_GET_MAC(CPSS_98DXA010_CNS),      CPSS_PP_FAMILY_DXCH_IRONMAN_E}};

    if(ppInInfoPtr->hwInfo[portGroupId].busType == CPSS_HW_INFO_BUS_TYPE_MBUS_E)
    {
        /* code is for AC5, AC5X and IronMan devices managed by internal CPU.
           MG0 base address is same for all these devices. */
        regAddr = PRV_CPSS_PHOENIX_MG0_BASE_ADDRESS_CNS + VENDOR_ID_REG_ADDR_OFFSET_CNS;

        /* read VendorID */
        rc = prvCpssDrvHwPpPortGroupReadRegister(devNum, portGroupId,regAddr, &deviceId) ;
        if (rc != GT_OK)
            return rc;

        if((deviceId & 0xFFFF) != MARVELL_VENDOR_ID)
        {
            /* not expected value for the register */
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, "Expected 'MARVELL VENDOR ID' [0x11AB] in register[0x%8.8x] but got [0x%4.4x]",
                regAddr, (deviceId & 0xFFFF));
        }

        regAddr = PRV_CPSS_PHOENIX_MG0_BASE_ADDRESS_CNS + PRV_CPSS_DEVICE_ID_AND_REV_REG_ADDR_CNS;

        /* read DeviceID from MG register */
        rc = prvCpssDrvHwPpPortGroupGetRegField(devNum, portGroupId, regAddr,
                                                4,16, &deviceId);
        if (rc != GT_OK)
            return rc;

        prefix = deviceId & PRV_FAMILY_PREFIX_MASK_CNS;
    }
    else
    {
        /* read device id from PCI Express BAR0 register 0 */
        rc = prvCpssDrvHwPpReadInternalPciReg(devNum, 0, &deviceId);
        if (rc!= GT_OK)
            return rc;

        if((deviceId & 0xFFFF) != MARVELL_VENDOR_ID)
        {
            /* not expected value for the register */
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, "Expected 'MARVELL VENDOR ID' [0x11AB] in register[0x%8.8x] but got [0x%4.4x]",
                0, (deviceId & 0xFFFF));
        }

        /* get prefix from device ID bits */
        prefix = PRV_FAMILY_PREFIX_GET_MAC(deviceId);
    }

    for (ii = 0; ii < PRV_FAMILY_PREFIX_DB_SIZE_CNS; ii++)
    {
        if (prefixDb[ii].prefix == prefix)
        {
            *devFamilyPtr = prefixDb[ii].devFamily;
            return GT_OK;
        }
    }

    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "device ID 0x%X not found in prefix DB", deviceId);
}

/**
* @internal mgDfxInfoGet function
* @endinternal
*
* @brief   This function fills in MG/DFX info based on device family
*
* @param[in] deviceFamily             - family of device
* @param[out] mgDfxInfoPtr            - (pointer to) the MG and DFX units access info updated
*                                       for Falcon and above devices
*
* @retval none
*/
static GT_VOID mgDfxInfoGet
(
    IN  CPSS_PP_FAMILY_TYPE_ENT     deviceFamily,
    OUT PRV_DRV_MG_DFX_INFO_STC     *mgDfxInfoPtr
)
{
    switch(deviceFamily)
    {
        case CPSS_PP_FAMILY_DXCH_FALCON_E:
            mgDfxInfoPtr->mg0UnitBaseAddr = PRV_CPSS_FALCON_MG0_BASE_ADDRESS_CNS;
            mgDfxInfoPtr->dfxUnitBaseAddr = PRV_CPSS_FALCON_DFX_BASE_ADDRESS_CNS;
            /* /Cider/EBU/Falcon/Falcon {Current}/<Eagle> Eagle/MNG/<SW_CNM_MASTER>CNM_IP/<PCIE4_EXT> pcie4_x2_DWC_pcie_ep/units/Memory Map/PF0_PORT_LOGIC/ACK_F_ASPM_CTRL_OFF */
            mgDfxInfoPtr->regAddr_ACK_F_ASPM_CTRL_OFF = PRV_CPSS_FALCON_PCIE4_EXT_BASE_ADDRESS_CNS + 0x0006070C;
            break;
        case CPSS_PP_FAMILY_DXCH_AC5P_E:
            mgDfxInfoPtr->mg0UnitBaseAddr = PRV_CPSS_AC5P_MG0_BASE_ADDRESS_CNS;
            mgDfxInfoPtr->dfxUnitBaseAddr = PRV_CPSS_AC5P_DFX_BASE_ADDRESS_CNS;
            /* /Cider/Switching Dies/Hawk/Hawk {Current}/<CNM_IP>CNM_IP/<CNM_IP> <PCIe_MAC> pcie_gen3_x1_DWC_pcie_ep/units/Memory Map/PF0_PORT_LOGIC/ACK_F_ASPM_CTRL_OFF. */
            mgDfxInfoPtr->regAddr_ACK_F_ASPM_CTRL_OFF = PRV_CPSS_AC5P_PCIE4_EXT_BASE_ADDRESS_CNS + 0x0000070C;
            break;
        case CPSS_PP_FAMILY_DXCH_AC5X_E:
            mgDfxInfoPtr->mg0UnitBaseAddr = PRV_CPSS_PHOENIX_MG0_BASE_ADDRESS_CNS;
            mgDfxInfoPtr->dfxUnitBaseAddr = PRV_CPSS_PHOENIX_DFX_BASE_ADDRESS_CNS;
            break;
        case CPSS_PP_FAMILY_DXCH_HARRIER_E:
            mgDfxInfoPtr->mg0UnitBaseAddr = PRV_CPSS_HARRIER_MG0_BASE_ADDRESS_CNS;
            mgDfxInfoPtr->dfxUnitBaseAddr = PRV_CPSS_HARRIER_DFX_BASE_ADDRESS_CNS;
            break;
        case CPSS_PP_FAMILY_DXCH_IRONMAN_E:
            mgDfxInfoPtr->mg0UnitBaseAddr = PRV_CPSS_IRONMAN_MG0_BASE_ADDRESS_CNS;
            mgDfxInfoPtr->dfxUnitBaseAddr = PRV_CPSS_IRONMAN_DFX_BASE_ADDRESS_CNS;
            break;
        default:
            break;
    }
}

/**
* @internal driverHwCntlInitAndDeviceIdGet function
* @endinternal
*
* @brief   This function is called by prvCpssDrvHwPpPhase1Init()
*         This function call driverHwCntlInit to initialize the HW control.
*         The function get the deviceType from HW , doByteSwap
*         Note : for standby HA -- the doByteSwap must be figured when the system
*         changed to HA active
* @param[in] devNum                   - The PP's device number to be initialized.
* @param[in] portGroupId              - port group Id.
* @param[in] ppInInfoPtr              - (pointer to)   the info needed for initialization of
*                                      the driver for this PP
* @param[in] mgDfxInfoPtr             - (pointer to) the MG and DFX units access info
* @param[out] mgDfxInfoPtr            - (pointer to) the MG and DFX units access info updated
*                                       for Falcon and above devices
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
*/
static GT_STATUS driverHwCntlInitAndDeviceIdGet
(
    IN  GT_U8                devNum,
    IN  GT_U32               portGroupId,
    IN  PRV_CPSS_DRV_PP_PHASE_1_INPUT_INFO_STC  *ppInInfoPtr,
    INOUT PRV_DRV_MG_DFX_INFO_STC               *mgDfxInfoPtr
)
{
    GT_STATUS   rc;
    PRV_CPSS_DRIVER_PP_CONFIG_STC *ppConfigPtr;/* Points to the Device's Driver*/
    GT_U32  readDeviceId ,readVendorId;/*deviceId,vendor Id as read from HW */
    GT_U32  tempDevId = 0;/* the device type built from device id , vendor id */
    GT_U32  regData;      /* register data     */
    GT_U32  address;      /* address to access */
    CPSS_PP_FAMILY_TYPE_ENT deviceFamily;

    ppConfigPtr = PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[devNum];

    mgDfxInfoPtr->allowMultiMgSdmaInit = 1;
    if(ppConfigPtr->mngInterfaceType == CPSS_CHANNEL_PEX_EAGLE_E ||
       ppConfigPtr->mngInterfaceType == CPSS_CHANNEL_PEX_FALCON_Z_E )
    {
        /* not supporting the MG1,MG2,MG3 memory spaces */
        mgDfxInfoPtr->allowMultiMgSdmaInit = 0;
    }

    mgDfxInfoPtr->regAddr_ACK_F_ASPM_CTRL_OFF = GT_NA;

    if(0 == PRV_CPSS_DRV_HW_IF_PCI_COMPATIBLE_MAC(devNum))
    {
        /* The "do byte swap" relate only to the PCI bus , since on PCI we read
           4 BYTES and don't know if was read correctly

           so for non-PCI we set doByteSwap = GT_FALSE
        */

        /* Init Hw Cntl parameters. --
           must be done prior to any read/write registers ...*/
        rc = prvCpssDrvHwCntlInit(devNum,portGroupId,
                         &(ppConfigPtr->hwInfo[portGroupId]),
                         GT_FALSE);/* isDiag */
        if (rc!= GT_OK)
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);

        deviceFamily = nextDevFamily;
        mgDfxInfoGet(deviceFamily,mgDfxInfoPtr);

        /* read DeviceID */
        rc = prvCpssDrvHwPpPortGroupGetRegField(devNum, portGroupId ,DEVICE_ID_AND_REV_REG_ADDR_CNS,
                                                4, 16, &readDeviceId);
        if (rc!= GT_OK)
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);

        /* read VendorID */
        rc = prvCpssDrvHwPpPortGroupGetRegField(devNum, portGroupId ,VENDOR_ID_REG_ADDR, 0, 16, &readVendorId) ;
        if (rc!= GT_OK)
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);

        tempDevId = (readDeviceId << 16) | readVendorId ;
    }
    else  /* PCI/PEX compatible */
    {
        /* Init Hw Cntl parameters. The value of doByteSwap is unknown
         * Assume only prvCpssDrvHwPpReadInternalPciReg() will be used till
         * correct value of doByteSwap known */
        rc = prvCpssDrvHwCntlInit(devNum,portGroupId,
                         &(ppConfigPtr->hwInfo[portGroupId]),
                         GT_FALSE);/* isDiag */
        if (rc!= GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }

        if(ppConfigPtr->mngInterfaceType == CPSS_CHANNEL_PEX_EAGLE_E)
        {
            /* autodetect device family */
            rc = sip6DeviceFamilyGet(devNum,portGroupId,ppInInfoPtr,&deviceFamily);
            if (rc!= GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
            }

            /* fill in MG and DFX info */
            mgDfxInfoGet(deviceFamily,mgDfxInfoPtr);

            nextDevFamily = deviceFamily;
        }
        else if (ppConfigPtr->mngInterfaceType == CPSS_CHANNEL_PEX_FALCON_Z_E)
        {
            nextDevFamily = deviceFamily = CPSS_PP_FAMILY_DXCH_FALCON_E;
            /* fill in MG and DFX info */
            mgDfxInfoGet(deviceFamily,mgDfxInfoPtr);
        }

        if(mgDfxInfoPtr->regAddr_ACK_F_ASPM_CTRL_OFF != GT_NA)/* Falcon , Hawk (but not AC5 and not AC5X) */
        {
            /* ERRATUM : ERR-7425158 : PCIE3-156 : CPSS - 9488 : PCIe N_FTS default value is configured to Gen1 instead of Gen4 max value.
              we  have identified  a non-optimal configuration at Falcon PCIE-MAC.
              These non optimal configurations are not expected to effect link establishment but can slow down/impact
              the PCIE wake-up from one of it's low power modes (when there is no activity)

              We have reviewed it with MSI PCIE PHY team and Synopsys MAC support and they recommended that we will
              write these values at our SW initialization sequence.

              Please add the below configuration to the CPSS init as early as possible (best after you identify device is Falcon):

              Write  /Cider/EBU/Falcon/Falcon {Current}/<Eagle> Eagle/MNG/<SW_CNM_MASTER>CNM_IP/<PCIE4_EXT> pcie4_x2_DWC_pcie_ep/units/Memory Map/PF0_PORT_LOGIC/ACK_F_ASPM_CTRL_OFF
              address  0x1c16070c
              [23:16] = 0x60  ('d96),
              [15:8]  = 0xC0  ('d192)

              All other register fields should stay the same.
            */

            /*
                NOTE: the defaults in AC5,AC5X are OK ... so we not get in to this case !
                  [23:16] = 0x60  ('d96),
                  [15:8]  = 0xC0  ('d192)
            */

            address = mgDfxInfoPtr->regAddr_ACK_F_ASPM_CTRL_OFF;
            rc = prvCpssDrvHwPpPortGroupReadRegister(devNum, portGroupId,address, &regData);
            if (rc!= GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
            }

            regData &= ~(0x00FFFF00);/* clear bits 23:8 */
            regData |= 0x60C0 << 8;/* [23:16] = 0x60 ,  [15:8]  = 0xC0 */

            rc = prvCpssDrvHwPpPortGroupWriteRegister(devNum, portGroupId,address, regData);
            if (rc!= GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
            }
        }

        /* Read from the PCI channel */
        if(ppInInfoPtr->ppHAState == CPSS_SYS_HA_MODE_ACTIVE_E)
        {
            switch(ppInInfoPtr->devIdLocationType)
            {
                case PRV_CPSS_DRV_DEV_ID_LOCATION_PCI_CONFIG_CYCLE_ADDR_0x00000000_E:
                case PRV_CPSS_DRV_DEV_ID_LOCATION_PEX_CONFIGURATION_HEADER_ADDR_0X00070000_E:
                    if(ppInInfoPtr->devIdLocationType ==
                        PRV_CPSS_DRV_DEV_ID_LOCATION_PEX_CONFIGURATION_HEADER_ADDR_0X00070000_E)
                    {
                        address = 0x70000;
                    }
                    else
                    {
                        address = 0;
                    }

                    /* perform the direct PCI/PEX memory read access */
                    rc = prvCpssDrvHwPpPortGroupReadInternalPciReg(devNum, portGroupId,address, &tempDevId);
                    if (rc!= GT_OK)
                        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);

                    if((tempDevId & 0xFFFF) != MARVELL_VENDOR_ID)
                    {
                        /* not expected value for the register */
                        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);
                    }
                    break;

                case PRV_CPSS_DRV_DEV_ID_LOCATION_DEVICE_ADDR_0x0000004C_E:
                /* try to read register 0x50 to get the 0x11AB (MARVELL_VENDOR_ID) */

                    /* read VendorID */
                    rc = prvCpssDrvHwPpPortGroupReadRegister(devNum, portGroupId,VENDOR_ID_REG_ADDR, &readVendorId) ;
                    if (rc != GT_OK)
                        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);

                    if((readVendorId & 0xFFFF) != MARVELL_VENDOR_ID)
                    {
                        /* not expected value for the register */
                        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, "Expected 'MARVELL VENDOR ID' [0x11AB] in register[0x%8.8x] but got [0x%4.4x]",
                            VENDOR_ID_REG_ADDR,
                            (readVendorId & 0xFFFF));
                    }

                    /* read DeviceID from MG register */
                    rc = prvCpssDrvHwPpPortGroupGetRegField(devNum, portGroupId, DEVICE_ID_AND_REV_REG_ADDR_CNS,
                                                            4,16, &readDeviceId);
                    if (rc != GT_OK)
                        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);

                    tempDevId = (readDeviceId << 16) | readVendorId ;

                    break;
                default:
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
            }/*switch(ppInInfoPtr->devIdLocationType) */

            if(PRV_CPSS_DRV_ERRATA_GET_MAC(devNum,PRV_CPSS_DRV_ERRATA_PCI_SLAVE_UNABLE_TO_ABORT_TRANSACTION_E) == GT_TRUE)
            {
                /* Initialize the PCI Timer and Retry register with 0xFF.   */
                /* Perform a read modify write on the register.             */
                rc = prvCpssDrvHwPpPortGroupReadInternalPciReg(devNum,portGroupId,0x104,&regData);
                if (rc!= GT_OK)
                    CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);

                regData = (regData & 0xFFFF0000) | 0xFFFF;  /*set both timer values to maximum */

                rc = prvCpssDrvHwPpPortGroupWriteInternalPciReg(devNum,portGroupId,0x104,regData);
                if (rc!= GT_OK)
                    CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
            }
        }
        /* now doByteSwap has correct value */
    }/* PCI/PEX compatible */

    if(ppInInfoPtr->ppHAState == CPSS_SYS_HA_MODE_STANDBY_E)
    {
        /* get the info that the high availability holds for that device */
        tempDevId  = HWINIT_GLOVAR(sysGenGlobalInfo.cpssPpHaDevData[devNum]->deviceId);
    }

    /* device type was stored in the internal DB */
    if (prvCpssDrvDebugDeviceId[devNum] != 0x0)
    {
        tempDevId = ((prvCpssDrvDebugDeviceId[devNum] << 16) | MARVELL_VENDOR_ID);
    }

#ifdef CHX_FAMILY
#ifndef GM_USED /* BC2 GM does not support DFX registers */
    /* devFamily isn't defined yet */
    if(((tempDevId>>16)&0xFC00) == 0xFC00)
    {
        rc = prvCpssDrvHwPpResetAndInitControllerGetRegField(devNum,
                                                             DFX_DEVICE_ID_AND_REV_REG_ADDR_CNS,
                                                             0,
                                                             16,
                                                             &regData);
        if(rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }
        tempDevId |= (regData << 16);

    }
#endif /* GM_USED */

    if(((tempDevId>>16) == 0xEC00) || ((tempDevId>>16) == 0x8000))
    {
        if(1 <= portGroupId)
        {/* In current revision of Lion2/Hooper Register 0x4C is always 0xEC00/0x8000.
            Read DFX S@R register bits [24-21] to get proper device type,
            DFX Unit exists only in port group 1 */
            if(GT_OK != prvCpssDrvHwPpPortGroupGetRegField(devNum, 1,
                                                            0x18F8200, 21, 4, &regData))
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);
            }
            else
            {
                tempDevId |= (regData << 16);
            }
        }

        if((tempDevId>>20) != 0x800)
        {/* run only for Lion2 A0 */ /* Lion2 Hooper has also revision 0 - but WA not needed */
            /* RevisionID read */
            prvCpssDrvHwPpGetRegField(devNum,
                            DEVICE_ID_AND_REV_REG_ADDR_CNS, 0, 4, &regData);
            if(0 == regData)
            {
                rc = prvCpssDrvLion2FirstPacketWA(devNum, portGroupId, ppInInfoPtr);
                if(rc != GT_OK)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
                }
            }
        }
    }
#endif

    /* temporary setting that may change if the CPSS decide to address this
       device with other device type ! */
    ppConfigPtr->devType = tempDevId;

    return GT_OK;
}


/**
* @internal deviceTypeInfoGet function
* @endinternal
*
* @brief   This function sets the device info :
*         number of ports ,
*         deviceType ,
*         and return bmp of ports , for a given device.
* @param[in] devNum                   - The Pp's device number.
*
* @param[out] existingPortsPtr         - (pointer to)the BMP of existing ports
*                                       GT_OK on success,
*
* @retval GT_BAD_STATE             - the 'multi port groups' support of the device has differences
*                                       between the application and the cpssDrv DB
*                                       GT_NOT_FOUND if the given pciDevType is illegal.
* @retval GT_NOT_SUPPORTED         - not properly supported device in DB
*/
static GT_STATUS deviceTypeInfoGet
(
    IN  GT_U8       devNum,
    OUT CPSS_PORTS_BMP_STC  *existingPortsPtr
)
{
    GT_STATUS   rc;/* return code*/
    GT_U32   ii;/* index in cpssSupportedTypes */
    GT_U32   jj;/* index in cpssSupportedTypes[ii].devTypeArray */
    GT_U32   kk;/* index in cpssSpecialDevicesBmp */
    GT_BOOL found = GT_FALSE;
    PRV_CPSS_DRIVER_PP_CONFIG_STC *ppConfigPtr;/* Points to the Device's Driver*/
    GT_U32  port;/*port iterator*/
    GT_U32  portInCurrentPortGroup;/*port number in current port group*/
    CPSS_PORTS_BMP_STC portsInPortGroupBmp;/*bitmap of the local ports of the port group */
    CPSS_PORTS_BMP_STC fullPortsBmp; /* full ports bmp of this device */
    GT_BOOL            needToForceOutPorts = GT_FALSE;/* indication if ports need to be forced out of the 'existing ports' */
    CPSS_PORTS_BMP_STC forcedOutPortsBmp;/* bmp of ports to be forced out of the 'existing ports' */
    const CPSS_SUPPORTED_TYPES_STC *cpssSupportedTypesPtr;/* pointer to list of supported devices*/
    const DEFAULT_REG_STC *onEmulator_defaultRegPtr = NULL;/*pointer to default registers for 'Emulator' uses */
    const DEFAULT_REG_FORMULA_STC *onEmulator_defaultReg_formulaPtr = NULL;/*pointer to default formula registers for 'Emulator' uses */

    cpssSupportedTypesPtr = cpssSupportedTypes;

    if(cpssDeviceRunCheck_onEmulator())
    {
        cpssSupportedTypesPtr = (const CPSS_SUPPORTED_TYPES_STC *)onEmulator_cpssSupportedTypes;
        /* get the info about our device */
        ii = 0;
        while(onEmulator_cpssSupportedTypes[ii].devFamily != END_OF_TABLE)
        {
            if(onEmulator_cpssSupportedTypes[ii].devFamily == ALDRIN_CNS &&
               onEmulator_cpssSupportedTypes[ii].defaultPortsBmpPtr)
            {
                /* modify to limit no ports */
                onEmulator_cpssSupportedTypes[ii].defaultPortsBmpPtr = NULL;

                break;
            }
            ii++;
        }
    }

    ppConfigPtr = PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[devNum];

    /* get the info about our device */
    ii = 0;
    while(cpssSupportedTypesPtr[ii].devFamily != END_OF_TABLE)
    {
        jj = 0;
        while(cpssSupportedTypesPtr[ii].devTypeArray[jj] != END_OF_TABLE)
        {
            if(ppConfigPtr->devType == cpssSupportedTypesPtr[ii].devTypeArray[jj])
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
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, "devType[0x%4.4x] not exists in 'Supported devices' list",
            ppConfigPtr->devType);
    }

    if(cpssDeviceRunCheck_onEmulator())
    {
        /* on Emulator we may need to init some registers for proper behavior */
        switch(cpssSupportedTypesPtr[ii].devFamily)
        {
            case ALDRIN_CNS:
                onEmulator_defaultRegPtr = onEmulator_AldrinDefaultRegArr_full;
                onEmulator_defaultReg_formulaPtr = onEmulator_AldrinDefaultRegArr_full_formula;
                break;
            case ALDRIN2_CNS:
                onEmulator_defaultReg_formulaPtr = onEmulator_Aldrin2DefaultRegArr_formula;
                break;
            case PIPE_CNS:
                onEmulator_defaultReg_formulaPtr = onEmulator_PipeDefaultRegArr_formula;
                break;
            default:
                break;
        }
    }

    if(onEmulator_defaultRegPtr)
    {
        for(jj = 0 ; onEmulator_defaultRegPtr[jj].regAddr != END_OF_TABLE ; jj++)
        {
            rc = prvCpssDrvHwPpWriteRegister(devNum,
                onEmulator_defaultRegPtr[jj].regAddr,
                onEmulator_defaultRegPtr[jj].regValue);
            if(rc != GT_OK)
            {
                return rc;
            }
        }
    }

    if(onEmulator_defaultReg_formulaPtr)
    {
        /* set formula of addresses */
        for(jj = 0 ; onEmulator_defaultReg_formulaPtr[jj].baseRegAddr != END_OF_TABLE ; jj++)
        {
            for(kk = onEmulator_defaultReg_formulaPtr[jj].startStepIndex ;
                kk < (onEmulator_defaultReg_formulaPtr[jj].startStepIndex +
                      onEmulator_defaultReg_formulaPtr[jj].numOfSteps);
                kk++)
            {
                rc = prvCpssDrvHwPpWriteRegister(devNum,
                    (onEmulator_defaultReg_formulaPtr[jj].baseRegAddr +
                     onEmulator_defaultReg_formulaPtr[jj].stepsInBytes * kk) ,
                    onEmulator_defaultReg_formulaPtr[jj].regValue);
                if(rc != GT_OK)
                {
                    return rc;
                }
            }
        }
    }


    PRV_CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&forcedOutPortsBmp);

    /* do 'multi port groups' device info check */
    if(cpssSupportedTypesPtr[ii].portGroupInfoPtr)
    {
        /*************************************************/
        /* the recognized device is 'multi port groups' device */
        /*************************************************/

        /* check that application stated that the device is 'multi port groups' device */
        if(PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[devNum]->portGroupsInfo.isMultiPortGroupDevice == GT_FALSE)
        {
            /* application not aware to the 'multi port groups' device ?! */
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
        }

        if(PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[devNum]->portGroupsInfo.numOfPortGroups >
            cpssSupportedTypesPtr[ii].portGroupInfoPtr->numOfPortGroups)
        {
            /* application think more port groups then really exists */
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
        }

        if(cpssSupportedTypesPtr[ii].activePortGroupsBmpPtr == NULL)
        {
            /* the DB of the device not hold the active port groups bitmap */
            /* internal DB error ! */
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
        }


        if(*cpssSupportedTypesPtr[ii].activePortGroupsBmpPtr !=
           PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[devNum]->portGroupsInfo.activePortGroupsBmp)
        {
            if(PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[devNum]->portGroupsInfo.activePortGroupsBmp == 0)
            {
                /* the device must have at least one active port group */
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }

            /* application allowed to force CPSS to not access existing port group.
               (this port group may exist in the device but not connected by the PEX) */
            for(kk = 0 ; kk < cpssSupportedTypesPtr[ii].portGroupInfoPtr->numOfPortGroups; kk++)
            {
                if((0 == ((*cpssSupportedTypesPtr[ii].activePortGroupsBmpPtr) & (1 << kk))) &&
                   (     PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[devNum]->portGroupsInfo.activePortGroupsBmp & (1 << kk)))
                {
                    /* application think that this is active port group but it is not */
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
                }

                if((     ((*cpssSupportedTypesPtr[ii].activePortGroupsBmpPtr) & (1 << kk))) &&
                   (0 == (PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[devNum]->portGroupsInfo.activePortGroupsBmp & (1 << kk))))
                {
                    /* application force CPSS to not access existing port group.
                       (this port group may exist in the device but not connected by the PEX) */
                    needToForceOutPorts = GT_TRUE;

                    /* update the bmp of 'forced out ports' */
                    forcedOutPortsBmp.ports[kk >> 1] |= (0xFFFF << ((kk & 1) * 16));
                }
            }
        }
    }
    else if(PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[devNum]->portGroupsInfo.isMultiPortGroupDevice == GT_TRUE)
    {
        /* application stated that this is 'multi port groups' device */
        /* but according to DB this is 'non multi port groups' device */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }

    ppConfigPtr->numOfPorts = GET_NUM_OF_PORTS(cpssSupportedTypesPtr[ii].numOfPorts);
    ppConfigPtr->numOfTiles = GET_NUM_OF_TILES(cpssSupportedTypesPtr[ii].numOfPorts);
    ppConfigPtr->devFamily = cpssSupportedTypesPtr[ii].devFamily;

    if(debug_force_numOfPorts)
    {
        /* allow to test GM limitations on simulation :
           BC3 the GM supports only 13 ports and not full 74 ports
        */
        ppConfigPtr->numOfPorts = debug_force_numOfPorts;
    }

    if(cpssSupportedTypesPtr[ii].defaultPortsBmpPtr == NULL)
    {
        if(cpssSupportedTypesPtr[ii].portGroupInfoPtr == NULL)
        {
            /* no special default for the ports BMP , use continuous ports bmp */
            prvCpssDrvPortsFullMaskGet(ppConfigPtr->numOfPorts,existingPortsPtr);
        }
        else
        {
            /* 'multi port groups' device */
            /* by default we build it's bmp according to the port groups info */

            portInCurrentPortGroup = 0;
            PRV_CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(existingPortsPtr);
            PRV_CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portsInPortGroupBmp);

            portsInPortGroupBmp.ports[0] = cpssSupportedTypesPtr[ii].portGroupInfoPtr->portsBmp;

            for(port = 0 ; port < ppConfigPtr->numOfPorts ; port++,portInCurrentPortGroup++)
            {
                if(portInCurrentPortGroup == cpssSupportedTypesPtr[ii].portGroupInfoPtr->maxNumPorts)
                {
                    portInCurrentPortGroup = 0;
                }

                if(ppConfigPtr->hwInfo[((port >> 4) & (CPSS_MAX_PORT_GROUPS_CNS - 1))/*portGroupId*/].busType == CPSS_HW_INFO_BUS_TYPE_NONE_E)
                {
                    /* non active port group */
                    continue;
                }

                if(CPSS_PORTS_BMP_IS_PORT_SET_MAC((&portsInPortGroupBmp),portInCurrentPortGroup))
                {
                    CPSS_PORTS_BMP_PORT_SET_MAC(existingPortsPtr,port);
                }
            }
        }
    }
    else
    {
        /* use special default for the ports BMP */
        *existingPortsPtr = *cpssSupportedTypesPtr[ii].defaultPortsBmpPtr;
    }

    /* Aldrin2 - special ports map */
    if (cpssSupportedTypesPtr[ii].devFamily == ALDRIN2_CNS)
    {
        if (ppConfigPtr->devType == CPSS_98DX8410_CNS)
        {
            ppConfigPtr->numOfPorts = 72;
            *existingPortsPtr = portsBmp48_special_map;
        }
    }

    /****************************************************************/
    /* add here specific devices BMP of ports that are not standard */
    /****************************************************************/
    kk = 0;
    while(cpssSpecialDevicesBmp[kk].devType != END_OF_TABLE)
    {
        if(cpssSpecialDevicesBmp[kk].devType == ppConfigPtr->devType)
        {
            *existingPortsPtr = cpssSpecialDevicesBmp[kk].existingPorts;
            break;
        }
        kk++;
    }

    /* clear from the existing ports the ports beyond the 'Number of ports' */
    prvCpssDrvPortsFullMaskGet(ppConfigPtr->numOfPorts,&fullPortsBmp);

    CPSS_PORTS_BMP_BITWISE_AND_MAC(existingPortsPtr,existingPortsPtr,&fullPortsBmp);

    if(needToForceOutPorts == GT_TRUE)
    {
        /* remove the 'forced out ports' from the 'existing ports bmp'*/
        CPSS_PORTS_BMP_BITWISE_AND_NOT_MAC(existingPortsPtr,existingPortsPtr,&forcedOutPortsBmp);
    }

    return GT_OK;
}

/**
* @internal sdmaSwapModeAndDeviceRevisionGet function
* @endinternal
*
* @brief   This function is called by prvCpssDrvHwPpPhase1Init()
*         This function do PCI and DMA calibration .
*         and read the 'revision' of the device.
* @param[in] devNum                   - The PP's device number to be initialized.
* @param[in] ppInInfoPtr              - (pointer to)   the info needed for initialization of
*                                      the driver for this PP
* @param[in] mgDfxInfoPtr             - (pointer to) the MG and DFX units access info
* @param[out] revisionIdPtr           - (pointer to)the revision of the PP
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
*/
static GT_STATUS sdmaSwapModeAndDeviceRevisionGet
(
    IN  GT_U8       devNum,
    IN  PRV_CPSS_DRV_PP_PHASE_1_INPUT_INFO_STC  *ppInInfoPtr,
    IN  PRV_DRV_MG_DFX_INFO_STC                 *mgDfxInfoPtr,
    OUT GT_U8       *revisionIdPtr
)
{
    GT_U32  tempData = 0;/* data from the register */
    GT_U32 ii;
#ifndef GM_USED
    PRV_CPSS_DRIVER_PP_CONFIG_STC *ppConfigPtr;/* Points to the Device's Driver*/
    ppConfigPtr = PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[devNum];
#endif

    /* Set the Rx / Tx glue byte swapping configuration     */
    if (PRV_CPSS_DRV_HW_IF_PCI_COMPATIBLE_MAC(devNum))
    {
        /* SDMA LE/BE should works the same since the CPU get the data as raw
           byte */
        /* Enable byte swap, Enable word swap  */

        prvCpssDrvHwPpSetRegField(devNum, SDMA_CFG_REG_ADDR, 6, 2, 3);
        prvCpssDrvHwPpSetRegField(devNum,SDMA_CFG_REG_ADDR,23,2,0);

        if(mgDfxInfoPtr->allowMultiMgSdmaInit)
        {
            for(ii = 0; ii < (CPSS_4_SDMA_CPU_PORTS_CNS-1); ii++)
            {
                prvCpssDrvHwPpResourceSetRegField(devNum,0,CPSS_DRV_HW_RESOURCE_MG1_CORE_E +ii,SDMA_CFG_REG_ADDR,6,2,3);
                prvCpssDrvHwPpResourceSetRegField(devNum,0,CPSS_DRV_HW_RESOURCE_MG1_CORE_E +ii,SDMA_CFG_REG_ADDR,23,2,0);
            }
        }

#ifdef ASIC_SIMULATION_H_EXIST
        /*
            this section is needed for the distributed simulation.
            when the read/write DMA from Asic are done via socket.

            and because both descriptors and frames are on the DMA memory,
            we need to set the way that Device put frame on the socket.

            because the DMA parser on the CPU side will set the data from the
            socket the same way for descriptors (that are little endian format)
            and the frames .
        */
        if(sasicgSimulationRoleIsApplication == GT_TRUE)
        {
            GT_U32  swapBytes;
            /* override the setting of SDMA swap/not swap bytes */
            if(BIG_ENDIAN_E == checkHostEndian())
            {
                /* when PP read/write frame form/to DMA it should do it non-swapped
                    I.E the frame should be placed on socket 'Big endean'
                 */
                swapBytes = 0;
            }
            else
            {
                /* when PP read/write frame form/to DMA it should do it swapped
                    I.E the frame should be placed on socket 'Little endean'
                 */
                swapBytes = 1;
            }

            prvCpssDrvHwPpSetRegField(devNum,SDMA_CFG_REG_ADDR, 6,1,swapBytes);
            prvCpssDrvHwPpSetRegField(devNum,SDMA_CFG_REG_ADDR,23,1,swapBytes);

            if(mgDfxInfoPtr->allowMultiMgSdmaInit)
            {
                for(ii = 0; ii < (CPSS_4_SDMA_CPU_PORTS_CNS-1); ii++)
                {
                    prvCpssDrvHwPpResourceSetRegField(devNum,0,CPSS_DRV_HW_RESOURCE_MG1_CORE_E +ii,SDMA_CFG_REG_ADDR,6,1,swapBytes);
                    prvCpssDrvHwPpResourceSetRegField(devNum,0,CPSS_DRV_HW_RESOURCE_MG1_CORE_E +ii,SDMA_CFG_REG_ADDR,23,1,swapBytes);
                }
            }
        }
#endif /*ASIC_SIMULATION_H_EXIST*/
    }

    /* get the revision id of the PP */
    /* RevisionID read */
    prvCpssDrvHwPpGetRegField(devNum,
                    DEVICE_ID_AND_REV_REG_ADDR_CNS, 0, 4, &tempData) ;

/* The GM not supports the DFX server registers */
#ifndef GM_USED
    if(ppConfigPtr->devFamily == CPSS_PP_FAMILY_DXCH_BOBCAT2_E  ||
       ppConfigPtr->devFamily == CPSS_PP_FAMILY_DXCH_ALDRIN_E   ||
       ppConfigPtr->devFamily == CPSS_PP_FAMILY_DXCH_AC3X_E     ||
       ppConfigPtr->devFamily == CPSS_PP_FAMILY_DXCH_BOBCAT3_E  ||
       ppConfigPtr->devFamily == CPSS_PP_FAMILY_DXCH_ALDRIN2_E  ||
       CPSS_DXCH_ALL_SIP6_FAMILY_MAC(ppConfigPtr->devFamily))
    {
        if(prvCpssDrvHwPpResetAndInitControllerGetRegField(devNum,
                                                             DFX_JTAG_DEVICE_ID_AND_REV_REG_ADDR_CNS,
                                                             28,
                                                             4,
                                                             &tempData) != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);
        }
    }
#endif /*ASIC_SIMULATION*/

    if(ppInInfoPtr->ppHAState == CPSS_SYS_HA_MODE_ACTIVE_E)
    {
        *revisionIdPtr = (GT_U8)tempData;
    }
    else
    {
        *revisionIdPtr = HWINIT_GLOVAR(sysGenGlobalInfo.cpssPpHaDevData[devNum]->revision);
    }

    return GT_OK;
}
/**
* @internal driverDxExMxHwPpPhase1Init function
* @endinternal
*
* @brief   This function is called by cpss "phase 1" device family functions,
*         in order to enable PP Hw access,
*         the device number provided to this function may
*         be changed when calling prvCpssDrvPpHwPhase2Init().
* @param[in] devNum                   - The PP's device number to be initialized.
* @param[in] ppInInfoPtr              - (pointer to)   the info needed for initialization of
*                                      the driver for this PP
*
* @param[out] ppOutInfoPtr             - (pointer to)the info that driver return to caller.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_OUT_OF_CPU_MEM        - failed to allocate CPU memory,
* @retval GT_HW_ERROR              - on hardware error
*/
static GT_STATUS driverDxExMxHwPpPhase1Init
(
    IN  GT_U8                devNum,
    IN  PRV_CPSS_DRV_PP_PHASE_1_INPUT_INFO_STC  *ppInInfoPtr,
    OUT PRV_CPSS_DRV_PP_PHASE_1_OUTPUT_INFO_STC *ppOutInfoPtr
)
{
    GT_STATUS   rc;
    PRV_CPSS_DRIVER_PP_CONFIG_STC *ppConfigPtr;/* Points to the Device's Driver*/
    GT_U32      portGroupId;/* the port group Id */
    GT_U32      dummyReg;/* dummy reg address */
    PRV_DRV_MG_DFX_INFO_STC mgDfxInfo; /* info about MG and DFX units access */

    /* set default for MG/DFX info to be fine for SIP_4/SIP_5 devices */
    mgDfxInfo.mg0UnitBaseAddr = 0;
    mgDfxInfo.dfxUnitBaseAddr = 0;
    mgDfxInfo.allowMultiMgSdmaInit = 1;

    /* the function bind the PCI/SMI/TWSI driver functions to the driver object
       functions for this device.*/
    rc = phase1Part1Init(devNum,ppInInfoPtr);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* NOTE :
        since the function of phase1Part2Init(...) will allocate dynamic memory
        we need to free it and restore NULL state if error occur in this current
        function driverDxExMxHwPpPhase1Init(...) after calling
        phase1Part2Init(...)

        see the use of label exit_cleanly_lbl
    */


   /* This function allocate the memory for the DB of the driver for this
      device , and set some of it's values.*/
    rc = phase1Part2Init(devNum,ppInInfoPtr);
    if(rc != GT_OK)
    {
        goto exit_cleanly_lbl;
    }

    if (CPSS_CHANNEL_PCI_E == ppInInfoPtr->mngInterfaceType)
    {
        /* relevant only for PCI devices */
        /* all PCI devices need it */
        PRV_CPSS_DRV_ERRATA_SET_MAC(
            devNum,
            PRV_CPSS_DRV_ERRATA_PCI_SLAVE_UNABLE_TO_ABORT_TRANSACTION_E);
    }


    ppConfigPtr = PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[devNum];

    ppConfigPtr->portGroupsInfo.isMultiPortGroupDevice =
        (ppInInfoPtr->numOfPortGroups == 1)
            ? GT_FALSE : GT_TRUE;

    for(portGroupId = 0; portGroupId < ppInInfoPtr->numOfPortGroups; portGroupId++)
    {
        if(ppConfigPtr->hwInfo[portGroupId].busType == CPSS_HW_INFO_BUS_TYPE_NONE_E)
        {
            /* non active port group */
            continue;
        }

        /* This function call driverHwCntlInit to initialize the HW control.
           The function get the deviceType from HW , doByteSwap */
        rc = driverHwCntlInitAndDeviceIdGet(devNum,portGroupId,ppInInfoPtr,&mgDfxInfo);
        if(rc != GT_OK)
        {
            goto exit_cleanly_lbl;
        }
    }

    /* starting from this point the DB of : PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[devNum]->portGroupsInfo
       is ready because initialized in the prvCpssDrvHwCntlInit(..) that called
       from phase1Part3Init(..)

       so we can start use the macros for loops on port groups (supports also 'non multi port group' device):
       PRV_CPSS_DRV_START_LOOP_PORT_GROUPS_MAC
       PRV_CPSS_DRV_END_LOOP_PORT_GROUPS_MAC
    */


    /* This function sets the device info : number of ports , deviceType ,
      and return bmp of ports , for a given device. */
    rc = deviceTypeInfoGet(devNum,
                           &ppOutInfoPtr->existingPorts);
    if(rc != GT_OK)
    {
        goto exit_cleanly_lbl;
    }

    ppOutInfoPtr->devType = ppConfigPtr->devType;
    ppOutInfoPtr->numOfPorts = ppConfigPtr->numOfPorts;
    ppOutInfoPtr->numOfTiles = ppConfigPtr->numOfTiles;
    ppOutInfoPtr->devFamily = ppConfigPtr->devFamily;

    /*only summary interrupts are unmasked by default */
    /* according to the interrupt scan tree. */
    ppConfigPtr->intCtrl.needInterruptMaskDefaultValueInit = GT_TRUE;
            /* bind the ISR callback routine done before signalling app. */
    ppConfigPtr->intCtrl.bindTheIsrCbRoutineToFdbTrigEnded = GT_FALSE;

    if(ppConfigPtr->mngInterfaceType == CPSS_CHANNEL_PEX_EAGLE_E ||
       ppConfigPtr->mngInterfaceType == CPSS_CHANNEL_PEX_FALCON_Z_E )
    {
        /* not supporting the MG address completion */
        /* Falcon , AC5P , AC5X , AC5 */
    }
    else
    if (((ppOutInfoPtr->devFamily == CPSS_PP_FAMILY_DXCH_BOBCAT2_E) ||
         (ppOutInfoPtr->devFamily == CPSS_PP_FAMILY_DXCH_BOBCAT3_E) ||
         (ppConfigPtr->devFamily == CPSS_PP_FAMILY_DXCH_ALDRIN2_E) ||
         (ppOutInfoPtr->devFamily == CPSS_PP_FAMILY_DXCH_ALDRIN_E) ||
        (ppOutInfoPtr->devFamily == CPSS_PP_FAMILY_DXCH_AC3X_E) ||
         (ppOutInfoPtr->devFamily == CPSS_PP_FAMILY_DXCH_XCAT3_E)||
         CPSS_IS_PX_FAMILY_MAC(ppOutInfoPtr->devFamily))
        && !CPSS_CHANNEL_IS_PEX_MBUS_MAC(ppInInfoPtr->mngInterfaceType))
    {
        /* Set address completion for backward compatible */
        rc = prvCpssDrvHwPpSetRegField(devNum, 0x140, 16, 1, 1);
        if(rc != GT_OK)
        {
            goto exit_cleanly_lbl;
        }
    }

    /* For PIPE connected by SMI, set unique driver that overcomes limitation of accessing SMI->I2C-1 */
    if ( (ppConfigPtr->devFamily == CPSS_PX_FAMILY_PIPE_E) &&
         (PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[devNum]->mngInterfaceType == CPSS_CHANNEL_SMI_E) )
    {
        rc = prvCpssHwDriverPipeSlaveSMISetDrv(PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[0]->drivers[0]);
        if (rc!= GT_OK)
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    for(portGroupId = 0; portGroupId < ppInInfoPtr->numOfPortGroups; portGroupId++)
    {
        PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[devNum]->hwCtrl[portGroupId].doReadAfterWrite =
            PRV_CPSS_DRV_ERRATA_GET_MAC(devNum,PRV_CPSS_DRV_ERRATA_PCI_READ_AFTER_WRITE_E);
    }

    if (ppOutInfoPtr->devFamily == CPSS_PP_FAMILY_DXCH_LION2_E)
    {
#ifdef PRV_CPSS_DRV_ERRATA_REFERENCE_CNS  /* macro never defined --> for references purpose only  */
    PRV_CPSS_DXCH_LION_RM_SDMA_ACTIVATION_WA_E /*Lion RM#2701*/
#endif /*PRV_CPSS_DRV_ERRATA_REFERENCE_CNS*/
        /* Lion RM#2701: SDMA activation */
        /* the code must be before calling sdmaSwapModeAndDeviceRevisionGet(...) because attempt
           to access register 0x2800 will cause the PEX to hang */
        rc = prvCpssDrvHwPpSetRegField(devNum,0x58,20,1,1);
        if(rc != GT_OK)
        {
            goto exit_cleanly_lbl;
        }
    }

    /* to SDMA swapping issues and get the revision of the PP
       before init of interrupts , as the interrupts may need info
       about the 'revision' of the PP */
    rc = sdmaSwapModeAndDeviceRevisionGet(devNum,ppInInfoPtr,&mgDfxInfo,&ppOutInfoPtr->revision);
    if(rc != GT_OK)
    {
        goto exit_cleanly_lbl;
    }

    switch(ppOutInfoPtr->devFamily)
    {
        case CPSS_PP_FAMILY_DXCH_XCAT3_E:
            #ifdef CHX_FAMILY
                /*Interrupts initialization for the xCat3 devices.*/
                prvCpssDrvPpIntDefDxChXcat3Init(devNum);
                /*only summary interrupts are unmasked by default */
                /* according to the interrupt scan tree. */
                ppConfigPtr->intCtrl.needInterruptMaskDefaultValueInit = GT_FALSE;
                break;
            #else
                rc = GT_NOT_SUPPORTED;
                goto exit_cleanly_lbl;
            #endif
        case CPSS_PP_FAMILY_DXCH_AC5_E:
            #ifdef CHX_FAMILY
                /* Interrupts initialization for the AlleyCat5 devices */
                rc = prvCpssDrvPpIntDefDxChAlleyCat5Init(devNum);
                if(rc != GT_OK)
                {
                    goto exit_cleanly_lbl;
                }
                /*only summary interrupts are unmasked by default */
                /* according to the interrupt scan tree. */
                ppConfigPtr->intCtrl.needInterruptMaskDefaultValueInit = GT_FALSE;
                break;
            #else
                rc = GT_NOT_SUPPORTED;
                goto exit_cleanly_lbl;
            #endif
        case CPSS_PP_FAMILY_DXCH_LION2_E:
            #ifdef CHX_FAMILY
                /*Interrupts initialization for the Lion2 devices.*/
                rc = prvCpssDrvPpIntDefDxChLion2Init(devNum);
                if(rc != GT_OK)
                {
                    goto exit_cleanly_lbl;
                }
            #else
                rc = GT_NOT_SUPPORTED;
                goto exit_cleanly_lbl;
            #endif

            /*only summary interrupts are unmasked by default */
            /* according to the interrupt scan tree. */
            ppConfigPtr->intCtrl.needInterruptMaskDefaultValueInit = GT_FALSE;

            break;

        case CPSS_PP_FAMILY_DXCH_BOBCAT2_E:
            switch(ppOutInfoPtr->devType)
            {
                case CPSS_BOBK_ALL_DEVICES_CASES_MAC:
                    /* indicate the sub family */
                    ppConfigPtr->devSubFamily = CPSS_PP_SUB_FAMILY_BOBCAT2_BOBK_E;
                    break;
                default:
                    break;
            }
            #ifdef CHX_FAMILY

#ifdef GM_USED
                /* Interrupts initialization for Bobcat2 devices */
                rc = prvCpssDrvPpIntDefDxChBobcat2Init__GM(devNum); /* in Lion2 file */
#else
                if (ppConfigPtr->devSubFamily == CPSS_PP_SUB_FAMILY_BOBCAT2_BOBK_E)
                {
                    rc = prvCpssDrvPpIntDefDxChBobKInit(devNum, ppOutInfoPtr->revision);
                }
                else
                {
                    rc = prvCpssDrvPpIntDefDxChBobcat2Init(devNum, ppOutInfoPtr->revision); /* in BC2 file */
                }
#endif
                if(rc != GT_OK)
                {
                    goto exit_cleanly_lbl;
                }

                /*only summary interrupts are unmasked by default */
                /* according to the interrupt scan tree. */
                ppConfigPtr->intCtrl.needInterruptMaskDefaultValueInit = GT_FALSE;
                break;
            #else
                rc = GT_NOT_SUPPORTED;
                goto exit_cleanly_lbl;
            #endif

        case CPSS_PP_FAMILY_DXCH_BOBCAT3_E:
            #ifdef CHX_FAMILY
#ifdef GM_USED
                /* Interrupts initialization for Bobcat2 devices */
                rc = prvCpssDrvPpIntDefDxChBobcat2Init__GM(devNum); /* in Lion2 file */
#else /*!GM_USED*/
                rc = prvCpssDrvPpIntDefDxChBobcat3Init(devNum,ppOutInfoPtr->revision);
#endif /*!GM_USED*/
                if(rc != GT_OK)
                {
                    goto exit_cleanly_lbl;
                }

                /*only summary interrupts are unmasked by default */
                /* according to the interrupt scan tree. */
                ppConfigPtr->intCtrl.needInterruptMaskDefaultValueInit = GT_FALSE;
                break;
            #else
                rc = GT_NOT_SUPPORTED;
                goto exit_cleanly_lbl;
            #endif
        case CPSS_PP_FAMILY_DXCH_ALDRIN_E:
        case CPSS_PP_FAMILY_DXCH_AC3X_E:
            #ifdef CHX_FAMILY
#ifdef GM_USED
                /* Interrupts initialization for Bobcat2 devices */
                rc = prvCpssDrvPpIntDefDxChBobcat2Init__GM(devNum); /* in Lion2 file */
#else
                rc = prvCpssDrvPpIntDefDxChAldrinInit(devNum, ppOutInfoPtr->revision);
#endif
                if(rc != GT_OK)
                {
                    goto exit_cleanly_lbl;
                }

                /*only summary interrupts are unmasked by default */
                /* according to the interrupt scan tree. */
                ppConfigPtr->intCtrl.needInterruptMaskDefaultValueInit = GT_FALSE;
                break;
            #else
                rc = GT_NOT_SUPPORTED;
                goto exit_cleanly_lbl;
            #endif
        case CPSS_PP_FAMILY_DXCH_ALDRIN2_E:
            #ifdef CHX_FAMILY
#ifdef GM_USED
                /* Interrupts initialization for Bobcat2 devices */
                rc = prvCpssDrvPpIntDefDxChBobcat2Init__GM(devNum); /* in Lion2 file */
#else /*!GM_USED*/
                rc = prvCpssDrvPpIntDefDxChAldrin2Init(devNum,ppOutInfoPtr->revision);
#endif /*!GM_USED*/
                if(rc != GT_OK)
                {
                    goto exit_cleanly_lbl;
                }

                /*only summary interrupts are unmasked by default */
                /* according to the interrupt scan tree. */
                ppConfigPtr->intCtrl.needInterruptMaskDefaultValueInit = GT_FALSE;
                break;
            #else
                rc = GT_NOT_SUPPORTED;
                goto exit_cleanly_lbl;
            #endif

        case CPSS_PX_FAMILY_PIPE_E:
            #ifdef ALLOW_PX_CODE
                rc = prvCpssDrvPpIntDefPxPipeInit(devNum, ppOutInfoPtr->revision);
                if(rc != GT_OK)
                {
                    goto exit_cleanly_lbl;
                }
                break;
            #else
                rc = GT_NOT_SUPPORTED;
                goto exit_cleanly_lbl;
            #endif
        case CPSS_PP_FAMILY_DXCH_FALCON_E:
            #ifdef CHX_FAMILY
#ifdef GM_USED
                /* Interrupts initialization for Bobcat2 devices */
                rc = prvCpssDrvPpIntDefDxChBobcat2Init__GM(devNum); /* in Lion2 file */
#else /*!GM_USED*/
                if (ppConfigPtr->mngInterfaceType == CPSS_CHANNEL_PEX_FALCON_Z_E)
                {
                    rc = prvCpssDrvPpIntDefDxChFalconZ2Init(devNum,ppOutInfoPtr->revision);
                }
                else
                {
                    rc = prvCpssDrvPpIntDefDxChFalconInit(devNum,ppOutInfoPtr->revision);
                }
#endif /*!GM_USED*/
                if(rc != GT_OK)
                {
                    goto exit_cleanly_lbl;
                }

                /*only summary interrupts are unmasked by default */
                /* according to the interrupt scan tree. */
                ppConfigPtr->intCtrl.needInterruptMaskDefaultValueInit = GT_FALSE;
                break;
            #else
                rc = GT_NOT_SUPPORTED;
                goto exit_cleanly_lbl;
            #endif
        case CPSS_PP_FAMILY_DXCH_AC5P_E:
            #ifdef CHX_FAMILY
                rc = prvCpssDrvPpIntDefDxChHawkInit(devNum,ppOutInfoPtr->revision);
                if(rc != GT_OK)
                {
                    goto exit_cleanly_lbl;
                }

                /*only summary interrupts are unmasked by default */
                /* according to the interrupt scan tree. */
                ppConfigPtr->intCtrl.needInterruptMaskDefaultValueInit = GT_FALSE;
                break;
            #else
                rc = GT_NOT_SUPPORTED;
                goto exit_cleanly_lbl;
            #endif
            break;
        case CPSS_PP_FAMILY_DXCH_AC5X_E:
            #ifdef CHX_FAMILY
                rc = prvCpssDrvPpIntDefDxChPhoenixInit(devNum,ppOutInfoPtr->revision);
                if(rc != GT_OK)
                {
                    goto exit_cleanly_lbl;
                }

                /*only summary interrupts are unmasked by default */
                /* according to the interrupt scan tree. */
                ppConfigPtr->intCtrl.needInterruptMaskDefaultValueInit = GT_FALSE;
                break;
            #else
                rc = GT_NOT_SUPPORTED;
                goto exit_cleanly_lbl;
            #endif
            break;
        case CPSS_PP_FAMILY_DXCH_HARRIER_E:
            #ifdef CHX_FAMILY
                rc = prvCpssDrvPpIntDefDxChHarrierInit(devNum,ppOutInfoPtr->revision);
                if(rc != GT_OK)
                {
                    goto exit_cleanly_lbl;
                }

                /*only summary interrupts are unmasked by default */
                /* according to the interrupt scan tree. */
                ppConfigPtr->intCtrl.needInterruptMaskDefaultValueInit = GT_FALSE;
                break;
            #else
                rc = GT_NOT_SUPPORTED;
                goto exit_cleanly_lbl;
            #endif
            break;
        case CPSS_PP_FAMILY_DXCH_IRONMAN_E:
            #ifdef CHX_FAMILY
                rc = prvCpssDrvPpIntDefDxChIronmanInit(devNum,ppOutInfoPtr->revision);
                if(rc != GT_OK)
                {
                    goto exit_cleanly_lbl;
                }

                /*only summary interrupts are unmasked by default */
                /* according to the interrupt scan tree. */
                ppConfigPtr->intCtrl.needInterruptMaskDefaultValueInit = GT_FALSE;
                break;
            #else
                rc = GT_NOT_SUPPORTED;
                goto exit_cleanly_lbl;
            #endif
            break;
        default:
            rc = GT_NOT_SUPPORTED;
            goto exit_cleanly_lbl;
    }
    /* save the sub family */
    ppOutInfoPtr->devSubFamily = ppConfigPtr->devSubFamily;

    /* bind the ISR callback routine done before signalling app. */
    rc = prvCpssDrvExMxDxHwPpPexAddrGet(ppOutInfoPtr->devFamily,&dummyReg,&ppConfigPtr->intCtrl.intSumMaskRegAddr);
    if(rc != GT_OK)
    {
        goto exit_cleanly_lbl;
    }

exit_cleanly_lbl:
    if(rc != GT_OK)
    {
        if(rc != GT_BAD_STATE && /* another device is in this memory --
                                    don't free the memory */
           PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[devNum])
        {
            /* release the allocation */
            for(portGroupId = 0; portGroupId < ppInInfoPtr->numOfPortGroups; portGroupId++)
            {
                if(PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[devNum]->drivers[portGroupId] != NULL)
                   cpssHwDriverDestroy(PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[devNum]->drivers[portGroupId]);
            }
            cpssOsFree(PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[devNum]);
            PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[devNum] = NULL;
        }
    }

    return rc;
}

/**
* @internal driverDxExMxHwPpRamBurstConfigSet function
* @endinternal
*
* @brief   Sets the Ram burst information for a given device.
*
* @param[in] devNum                   - The Pp's device number.
* @param[in] ramBurstInfoPtr          - A list of Ram burst information for this device.
* @param[in] burstInfoLen             - Number of valid entries in ramBurstInfo.
*
* @retval GT_OK                    - on success,
* @retval GT_OUT_OF_CPU_MEM        - failed to allocate CPU memory,
* @retval GT_ALREADY_EXIST         - the ram already initialized
*
* @note function also bound to the "pci-standby" for HA
*
*/
static GT_STATUS driverDxExMxHwPpRamBurstConfigSet
(
    IN  GT_U8               devNum,
    IN  PRV_CPSS_DRV_RAM_BURST_INFO_STC   *ramBurstInfoPtr,
    IN  GT_U8               burstInfoLen
)
{
    GT_U32  portGroupId;

    if(CPSS_IS_DXCH_FAMILY_MAC(PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[devNum]->devFamily) ||
       CPSS_IS_PX_FAMILY_MAC(PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[devNum]->devFamily))
    {
        /* no more to do */
        return GT_OK;
    }

    PRV_CPSS_DRV_START_LOOP_PORT_GROUPS_MAC(devNum,portGroupId)
    {
        if(PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[devNum]->hwCtrl[portGroupId].ramBurstInfo != NULL ||
           PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[devNum]->hwCtrl[portGroupId].ramBurstInfoLen != 0 )
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[devNum]->hwCtrl[portGroupId].ramBurstInfo =
            cpssOsMalloc(sizeof(PRV_CPSS_DRV_RAM_BURST_INFO_STC)* burstInfoLen);
        if(PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[devNum]->hwCtrl[portGroupId].ramBurstInfo == NULL)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
        }

        cpssOsMemCpy(PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[devNum]->hwCtrl[portGroupId].ramBurstInfo,ramBurstInfoPtr,
                 sizeof(PRV_CPSS_DRV_RAM_BURST_INFO_STC) * burstInfoLen);

        PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[devNum]->hwCtrl[portGroupId].ramBurstInfoLen = burstInfoLen;
    }
    PRV_CPSS_DRV_END_LOOP_PORT_GROUPS_MAC(devNum,portGroupId)

    return GT_OK;
}

/**
* @internal driverExMxDxHwPpHaModeSet function
* @endinternal
*
* @brief   function to set CPU High Availability mode of operation.
*
* @note   APPLICABLE DEVICES:      all ExMx devices
* @param[in] devNum                   - the device number.
* @param[in] mode                     - active or standby
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - on failure.
*/
static GT_STATUS driverExMxDxHwPpHaModeSet
(
    IN  GT_U8   devNum,
    IN  CPSS_SYS_HA_MODE_ENT mode
)
{
    GT_STATUS   rc;
    PRV_CPSS_DRIVER_PP_CONFIG_STC *ppConfigPtr;/* Points to the Device's Driver*/
    GT_U32      portGroupId;/* the port group Id */

    if(0 == PRV_CPSS_DRV_HW_IF_PCI_COMPATIBLE_MAC(devNum))
    {
        /* nothing to update -- not supported option yet */
        return GT_OK;
    }

    ppConfigPtr =  PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[devNum];

    PRV_CPSS_DRV_START_LOOP_PORT_GROUPS_MAC(devNum,portGroupId)
    {
        if(mode == CPSS_SYS_HA_MODE_ACTIVE_E)
        {
            /* update the pointer to the functions object -- to use the PCI */
            PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_OBJ_CONFIG[devNum]->busPtr = prvCpssDrvMngInfPciPtr;
        }
        else
        {
            /* update the pointer to the functions object -- to emulate the PCI */
            PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_OBJ_CONFIG[devNum]->busPtr = prvCpssDrvMngInfPciHaStandbyPtr;
        }

        /* we need to re-initialize the HW control driver --
           because we changed the OBJ of the prvCpssDrvHwCntlInit */
        /* Init Hw Cntl parameters. Now we know the value of doByteSwap */
        rc = prvCpssDrvHwCntlInit(devNum,portGroupId,
                         &(ppConfigPtr->hwInfo[portGroupId]),
                         GT_FALSE);/* isDiag */
        if (rc!= GT_OK)
            return rc;
    }
    PRV_CPSS_DRV_END_LOOP_PORT_GROUPS_MAC(devNum,portGroupId)
    PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_OBJ_CONFIG[devNum]->HAState = mode;

    return GT_OK;
}

#ifdef CHX_FAMILY

/**
* @internal prvCpssDrvLion2FirstPacketWA function
* @endinternal
*
* @brief   This function engages Lion2 A0 517 (DIP) FE-3832472 "First Packet drop"
*         erratum WA, which must be before any device init activity
* @param[in] devNum                   - The PP device number to read from.
* @param[in] portGroupId              - number of core(port group)
* @param[in] ppInInfoPtr              - (pointer to)   the info needed for initialization of
*                                      the driver for this PP
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - if HW access failed
*
* @note Should be engaged only for A0 revision.
*
*/
static GT_STATUS prvCpssDrvLion2FirstPacketWA
(
    IN GT_U8    devNum,
    IN GT_U32   portGroupId,
    IN PRV_CPSS_DRV_PP_PHASE_1_INPUT_INFO_STC  *ppInInfoPtr
)
{
    GT_STATUS   rc;     /* return code */
    GT_U32      pcsNum; /* number of PCS lane */
    GT_U32      regAddr;/* register address */
#ifndef ASIC_SIMULATION
    GT_U32      timeOut;/* counter for wait after reast */
#endif
    GT_U32      regData;/* register data */

    /* Lion2 517 (DIP) FE-3832472 "First Packet drop" erratum WA */
    rc = prvCpssDrvLion2MMPcsUnReset(devNum,portGroupId);
    if(rc != GT_OK)
        return rc;
    /* stop MAC clock */
    for(pcsNum = 0; pcsNum < 16; pcsNum++)
    {
        regAddr = 0x88C0414 + pcsNum * 0x1000;
        rc = prvCpssDrvHwPpPortGroupSetRegField(devNum, portGroupId, regAddr,
                                                12, 1, 0);
        if (rc!= GT_OK)
            return rc;
    }

    /* execute device reset just when coming to last port group */
    if(portGroupId == ppInInfoPtr->numOfPortGroups-1)
    {
        /* MG Soft Reset */
        rc = prvCpssDrvHwPpPortGroupSetRegField(devNum, 1,0x18F805C,
                                                8, 1, 0);
        if (rc!= GT_OK)
            return rc;

        rc = prvCpssDrvHwPpPortGroupSetRegField(devNum, 1,0x18F8060,
                                                8, 1, 0);
        if (rc!= GT_OK)
            return rc;

        rc = prvCpssDrvHwPpPortGroupSetRegField(devNum, 1,0x18F800C,
                                                1, 1, 0);
        if (rc!= GT_OK)
            return rc;

        cpssOsTimerWkAfter(10);

        /* dummy reads to ensure right delay */
        rc = prvCpssDrvHwPpPortGroupReadRegister(devNum, 1, 0x58,
                                                 &regData);
        if (rc!= GT_OK)
            return rc;

        rc = prvCpssDrvHwPpPortGroupReadRegister(devNum, 1, 0xF80002C,
                                                 &regData);
        if (rc!= GT_OK)
            return rc;

        rc = prvCpssDrvHwPpPortGroupReadRegister(devNum, 1, 0xF800038,
                                                 &regData);
        if (rc!= GT_OK)
            return rc;

#ifndef ASIC_SIMULATION
        /* Verify SW reset finished */
        timeOut = 10;
        do
        {
            rc = prvCpssDrvHwPpPortGroupGetRegField(devNum, 1,0x18F800C,
                                            1, 1, &regData);
            if (rc!= GT_OK)
                return rc;

            timeOut--;
            cpssOsTimerWkAfter(10);
        }while((regData != 1) && (timeOut > 0));

        if((regData != 1) && (timeOut == 0))
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_TIMEOUT, LOG_ERROR_NO_MSG);
#endif
    }/* end if(portGroupId == */

    return GT_OK;
}

/**
* @internal prvCpssDrvLion2MMPcsUnReset function
* @endinternal
*
* @brief   Unreset Lion2 MMPCS
*
* @param[in] devNum                   - system device number
* @param[in] portGroupId              - port group (core) number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
*/
static GT_STATUS prvCpssDrvLion2MMPcsUnReset
(
    GT_U8                   devNum,
    GT_U32                  portGroupId
)
{
    GT_STATUS   rc;     /* return code */
    GT_U32      regAddr; /* address of register */
    GT_U32      pcsNum; /* PCS lane number */

    for(pcsNum = 0; pcsNum < 16; pcsNum++)
    {
        regAddr = 0x088c054c + pcsNum * 0x1000;
        rc = prvCpssDrvHwPpPortGroupSetRegField(devNum, portGroupId ,regAddr,
                                                0, 1, 1);
        if (rc!= GT_OK)
            return rc;
    }

    for(pcsNum = 0; pcsNum < 16; pcsNum++)
    {
        regAddr = 0x088c054c + pcsNum * 0x1000;
        rc = prvCpssDrvHwPpPortGroupSetRegField(devNum, portGroupId ,regAddr,
                                                1, 1, 1);
        if (rc!= GT_OK)
            return rc;
    }

    for(pcsNum = 0; pcsNum < 16; pcsNum++)
    {
        regAddr = 0x088c054c + pcsNum * 0x1000;
        rc = prvCpssDrvHwPpPortGroupSetRegField(devNum, portGroupId ,regAddr,
                                                2, 1, 1);
        if (rc!= GT_OK)
            return rc;
    }

    for(pcsNum = 0; pcsNum < 16; pcsNum++)
    {
        regAddr = 0x088c054c + pcsNum * 0x1000;
        rc = prvCpssDrvHwPpPortGroupSetRegField(devNum, portGroupId ,regAddr,
                                                5, 1, 1);
        if (rc!= GT_OK)
            return rc;
    }

    for(pcsNum = 0; pcsNum < 16; pcsNum++)
    {
        regAddr = 0x088c054c + pcsNum * 0x1000;
        rc = prvCpssDrvHwPpPortGroupSetRegField(devNum, portGroupId ,regAddr,
                                                6, 1, 1);
        if (rc!= GT_OK)
            return rc;
    }

    for(pcsNum = 0; pcsNum < 16; pcsNum++)
    {
        regAddr = 0x088c054c + pcsNum * 0x1000;
        rc = prvCpssDrvHwPpPortGroupSetRegField(devNum, portGroupId ,regAddr,
                                                4, 1, 1);
        if (rc!= GT_OK)
            return rc;
    }

    for(pcsNum = 0; pcsNum < 16; pcsNum++)
    {
        regAddr = 0x088c054c + pcsNum * 0x1000;
        rc = prvCpssDrvHwPpPortGroupSetRegField(devNum, portGroupId ,regAddr,
                                                3, 1, 1);
        if (rc!= GT_OK)
            return rc;
    }

    for(pcsNum = 0; pcsNum < 16; pcsNum++)
    {
        regAddr = 0x088c054c + pcsNum * 0x1000;
        rc = prvCpssDrvHwPpPortGroupSetRegField(devNum, portGroupId ,regAddr,
                                                7, 4, 0x4);
        if (rc!= GT_OK)
            return rc;
    }

    return GT_OK;
}

#endif /* CHX_FAMILY */

/**
* @internal prvCpssDrvDxExMxInitObject function
* @endinternal
*
* @brief   This function creates and initializes ExMxDx device driver object
*
* @param[in] devNum                   - The PP device number to read from.
*
* @retval GT_OK                    - on success
* @retval GT_ALREADY_EXIST         - if the driver object have been created before
*/
GT_STATUS prvCpssDrvDxExMxInitObject
(
     IN GT_U8   devNum
)
{
    GT_STATUS rc;

    /****************************************************/
    /* bind the generic part to the ExMxDx generic part */
    /****************************************************/
    PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_OBJ_CONFIG[devNum]->genPtr = prvCpssDrvGenExMxDxObjPtr;
    /* the bus part will be initialized in "phase 1" -
       when we get the parameter */
    PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_OBJ_CONFIG[devNum]->busPtr = NULL;
    PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_OBJ_CONFIG[devNum]->HAState = CPSS_SYS_HA_MODE_ACTIVE_E;

    /* driver object initialization */
    prvCpssDrvGenExMxDxObjPtr->drvHwPpCfgPhase1Init = driverDxExMxHwPpPhase1Init;
    prvCpssDrvGenExMxDxObjPtr->drvHwPpSetRamBurstConfig = driverDxExMxHwPpRamBurstConfigSet;
    prvCpssDrvGenExMxDxObjPtr->drvHwPpHaModeSet = driverExMxDxHwPpHaModeSet;
    prvCpssDrvGenExMxDxObjPtr->drvHwPpInterruptsTreeGet = prvCpssDrvExMxDxHwPpInterruptsTreeGet;
    rc = prvCpssDrvDxExMxEventsInitObject();

    if(rc != GT_OK)
        return rc;

    rc = prvCpssDrvDxExMxIntInitObject();

    return rc;
}

/**
* @internal prvCpssDrPpConInitMg0UnitBaseAddressGet function
* @endinternal
*
* @brief   This function gets  Mg0 Unit Base Address
*
* @param[in] devNum                   - devNum of device
*
* @retval none
*/
        GT_U32 prvCpssDrPpConInitMg0UnitBaseAddressGet
(
     IN GT_U8      devNum
)
{
    CPSS_PP_FAMILY_TYPE_ENT deviceFamily = PRV_CPSS_PP_MAC(devNum)->devFamily ;
    PRV_DRV_MG_DFX_INFO_STC     mgDfxInfo;
    cpssOsMemSet(&mgDfxInfo, 0, sizeof(PRV_DRV_MG_DFX_INFO_STC));
    mgDfxInfoGet(deviceFamily,&mgDfxInfo);
    return mgDfxInfo.mg0UnitBaseAddr;
}

