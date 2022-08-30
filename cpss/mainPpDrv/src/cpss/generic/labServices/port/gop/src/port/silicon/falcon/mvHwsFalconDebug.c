/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* mvHwsFalconDebug.c
*
* DESCRIPTION:
*     Falcon debug functions .
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*
*******************************************************************************/

#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

#define PR cpssOsPrintf
typedef struct
{
    GT_U32      regOffset;
    GT_CHAR*    regName;
    GT_U32      numRegs;
    GT_U32      offsetFormula;
}REG_INFO;

#define REG_INFO_MAC(_regOffset,_regName,_numRegs,_offsetFormula) {_regOffset,_regName,_numRegs,_offsetFormula}
#define LAST_REG_INFO_MAC   {0,NULL,0,0}

/*function to read register from the chiplet (Raven) using SMI */
extern GT_STATUS prvCpssDrvHwPpSmiChipletRegisterRead
(
    IN GT_U8    devNum,
    IN GT_U32   chipletId,
    IN GT_U32   regAddr,
    OUT GT_U32  *data
);

/*macro to read register from the chiplet (Raven) using SMI */
#define SMI_READ    prvCpssDrvHwPpSmiChipletRegisterRead

static REG_INFO falcon_d2d_gw_reg_info[] = {
         REG_INFO_MAC(0x00684604, " MAC RX TDM Schedule %n                 " ,160/4,0x4)
        ,REG_INFO_MAC(0x006847E4, " MAC RX FIFO Bandwidth Select %n        "  ,3,0x4)
        ,REG_INFO_MAC(0x00684804, " MAC RX Channel %n                      " ,18,0x4)
        ,REG_INFO_MAC(0x00684904, " MAC RX FIFO Segment Pointer %n         " ,34,0x4)
        ,REG_INFO_MAC(0x00684A04, " MAC RX Stat Good Packets Low %n        " ,17,0x8)
        ,REG_INFO_MAC(0x00684A08, " MAC RX Stat Good Packets High %n       " ,17,0x8)
        ,REG_INFO_MAC(0x00684C04, " MAC RX Stat Bad Packets Low %n         " ,17,0x8)
        ,REG_INFO_MAC(0x00684C08, " MAC RX Stat Bad Packets High %n        " ,17,0x8)
        ,REG_INFO_MAC(0x00684E04, " MAC RX Channel 2 %n                    " ,18,0x4)
        ,REG_INFO_MAC(0x00684F04, " MAC Error Mask                         " ,1,0x4)
        ,REG_INFO_MAC(0x00684F08, " MAC Error Cause                        " ,1,0x4)
        ,REG_INFO_MAC(0x00684F0C, " MAC Error TX First FIFO Overflow       " ,1,0x4)
        ,REG_INFO_MAC(0x00684F10, " MAC Error TX First Interface Sequencing" ,1,0x4)
        ,REG_INFO_MAC(0x00684F14, " MAC Error TX First Credit Underflow    " ,1,0x4)
        ,REG_INFO_MAC(0x00684F18, " MAC Error RX First FIFO Overflow 0     " ,1,0x4)
        ,REG_INFO_MAC(0x00684F1C, " MAC Error RX First Credit Underflow    " ,1,0x4)
        ,REG_INFO_MAC(0x00684F24, " MAC Error RX Bad Address               " ,1,0x4)
        ,REG_INFO_MAC(0x00684FFC, " MAC Debug Frame Enable                 " ,1,0x4)


        ,REG_INFO_MAC(0x00684000, "  MAC Revision ID                       " ,1,0x4)
        ,REG_INFO_MAC(0x00684004, "  MAC TX Channel %n                     " ,18,0x4)
        ,REG_INFO_MAC(0x00684104, "  MAC TX FIFO Segment Pointer %seg      " ,32,0x4)
        ,REG_INFO_MAC(0x00684204, "  MAC TX Stat Good Packets Low %n       " ,17,0x8)
        ,REG_INFO_MAC(0x00684208, "  MAC TX Stat Good Packets High %n      " ,17,0x8)
        ,REG_INFO_MAC(0x00684400, "  MAC Error TX Bad Address              " ,1,0x4)
        ,REG_INFO_MAC(0x00684404, "  MAC TX Stat Bad Packets Low %n        " ,17,0x8)
        ,REG_INFO_MAC(0x00684408, "  MAC TX Stat Bad Packets High %n       " ,17,0x8)
        ,REG_INFO_MAC(0x006845FC, "  MAC TX Error Mask                     " ,1,0x4)
        ,REG_INFO_MAC(0x00684600, "  MAC TX Error Cause                    " ,1,0x4)

/* not exists in Cider Eagle (and debug on Raven)
        ,REG_INFO_MAC(0x00685C00, "   DBG_FRM_CHECK_CFG_%n                 " ,17,0x4)
        ,REG_INFO_MAC(0x00685CFC, "   MAC Debug Check Error Bad Address    " ,1,0x4)
        ,REG_INFO_MAC(0x00685D00, "   DBG_FRM_CHECK_SEQ_%n                 " ,17,0x4)
        ,REG_INFO_MAC(0x00685DFC, "   MAC Debug Check Error Cause          " ,1,0x4)
        ,REG_INFO_MAC(0x00685E00, "   DBG_FRM_CNT_GOOD_%n                  " ,17,0x4)
        ,REG_INFO_MAC(0x00685EFC, "   MAC Debug Check Error Mask           " ,1,0x4)
        ,REG_INFO_MAC(0x00685F00, "   DBG_FRM_CNT_BAD_%n                   " ,17,0x4)

        ,REG_INFO_MAC(0x00685000, " DBG_FRM_GEN_CFG_%n                     " ,17,0x4)
        ,REG_INFO_MAC(0x006850FC, " MAC Debug Gen Error Bad Address        " ,1,0x4)
        ,REG_INFO_MAC(0x00685100, " DBG_FRM_GEN_CFG_STAT_%n                " ,17,0x8)
        ,REG_INFO_MAC(0x006851FC, " MAC Debug Gen Error Cause              " ,1,0x4)
        ,REG_INFO_MAC(0x00685200, " DBG_FRM_TBL_%n                         " ,32,0x4)
        ,REG_INFO_MAC(0x006852FC, " MAC Debug Gen Error Mask               " ,1,0x4)
        ,REG_INFO_MAC(0x00685300, " DBG_FRM_TBL_LEN_%n                     " ,32,0x4)
        ,REG_INFO_MAC(0x00685400, " DBG_FRM_TBL_HEADER_%i_%n               " ,28*32,0x4)
        ,REG_INFO_MAC(0x00685B00, " DBG_FRM_TBL_PAYLOAD_%n                 " ,32,0x4)
*/
        ,REG_INFO_MAC(0x00686038, " SERDES MAC Error Mask                  " ,1,0x4)
        ,REG_INFO_MAC(0x0068603C, " SERDES MAC Error Cause                 " ,1,0x4)
        ,REG_INFO_MAC(0x0068607C, " SERDES MAC Bad Address                 " ,1,0x4)
        ,REG_INFO_MAC(0x00686000, " SERDES MAC TX Config %n                " ,8,0x80)
        ,REG_INFO_MAC(0x00686020, " SERDES MAC TX Error Cause %n           " ,8,0x80)
        ,REG_INFO_MAC(0x00686024, " SERDES MAC TX Error Mask %n            " ,8,0x80)
        ,REG_INFO_MAC(0x00686040, " SERDES MAC RX Config %n                " ,8,0x80)
        ,REG_INFO_MAC(0x00686060, " SERDES MAC RX Error Cause %n           " ,8,0x80)
        ,REG_INFO_MAC(0x00686064, " SERDES MAC RX Error Mask %n            " ,8,0x80)

        ,REG_INFO_MAC(0x00687000, " PCS Global Control                     " ,1,0x4)
        ,REG_INFO_MAC(0x00687004, " PCS Transmit Calendar Control          " ,1,0x4)
        ,REG_INFO_MAC(0x00687008, " PCS Receive Calendar Control           " ,1,0x4)
        ,REG_INFO_MAC(0x0068700C, " PCS Transmit Rate Control              " ,1,0x4)
        ,REG_INFO_MAC(0x00687010, " PCS Transmit Statistic Low part        " ,1,0x4)
        ,REG_INFO_MAC(0x00687014, " PCS Transmit Statistic High part       " ,1,0x4)
        ,REG_INFO_MAC(0x00687018, " PCS Receive Statistic Low part         " ,1,0x4)
        ,REG_INFO_MAC(0x0068701C, " PCS Receive Statistic High part        " ,1,0x4)
        ,REG_INFO_MAC(0x00687020, " PCS Alignment Marker BER Count for Each Lane %n" ,4,0x4)
        ,REG_INFO_MAC(0x00687040, " PCS Event Cause                        " ,1,0x4)
        ,REG_INFO_MAC(0x00687044, " PCS Event Mask                         " ,1,0x4)
        ,REG_INFO_MAC(0x00687048, " PCS Transmit Internal Flowcontrol Mask   " ,1,0x4)
        ,REG_INFO_MAC(0x0068704C, " PCS Transmit Internal Flowcontrol Force  " ,1,0x4)
        ,REG_INFO_MAC(0x00687050, " PCS Transmit Internal Flowcontrol Status      " ,1,0x4)
        ,REG_INFO_MAC(0x00687054, " PCS Transmit Internal Flowcontrol Change Event" ,1,0x4)
        ,REG_INFO_MAC(0x00687058, " PCS Receive Status                     " ,1,0x4)
        ,REG_INFO_MAC(0x0068705C, " PCS Control CPU Slot Arbitration       " ,1,0x4)
        ,REG_INFO_MAC(0x00687060, " PCS Debug PFC Control                  " ,1,0x4)
        ,REG_INFO_MAC(0x00687064, " PCS Debug PFC Read Value 1             " ,1,0x4)
        ,REG_INFO_MAC(0x00687068, " PCS Debug PFC Read Value 0             " ,1,0x4)
        ,REG_INFO_MAC(0x0068706C, " PCS Debug PFC Write Value 1            " ,1,0x4)
        ,REG_INFO_MAC(0x00687070, " PCS Debug PFC Write Value 0            " ,1,0x4)
        ,REG_INFO_MAC(0x00687074, " PCS Debug error inject                 " ,1,0x4)
        ,REG_INFO_MAC(0x00687078, " PCS Metal Fix                          " ,1,0x4)
        ,REG_INFO_MAC(0x0068707C, " PCS MAC mode selector                  " ,1,0x4)
        ,REG_INFO_MAC(0x00687080, " PCS Register Access Status             " ,1,0x4)
        ,REG_INFO_MAC(0x00687084, " PCS Out Of Reset                       " ,1,0x4)
        ,REG_INFO_MAC(0x00687100, " PCS Debug Lane Control for lane %n         " ,4,0x4)
        ,REG_INFO_MAC(0x00687200, " PCS Transmit Calendar Slot Configuration %n" ,32,0x4)
        ,REG_INFO_MAC(0x00687400, " PCS Receive Calendar Slot Configuration %n " ,32,0x4)
        ,REG_INFO_MAC(0x00687600, " PCS Internal Flow Control Remapping_%ch    " ,32,0x4)

        ,REG_INFO_MAC(0x00688004, " PMA Reset Control Per Lane                     " ,1,0x4)
        ,REG_INFO_MAC(0x00688008, " PMA Operation Control                          " ,1,0x4)
        ,REG_INFO_MAC(0x00688010, " PMA Debug Boundary Scan Bypass Overrule Control" ,1,0x4)
        ,REG_INFO_MAC(0x00688014, " PMA Debug Boundary Scan Bypass Receive data    " ,1,0x4)
        ,REG_INFO_MAC(0x00688018, " PMA Debug Boundary Scan Bypass Transmit data   " ,1,0x4)
        ,REG_INFO_MAC(0x0068801C, " PMA Debug Clock Domain Crossing FIFO levels    " ,1,0x4)
        ,REG_INFO_MAC(0x00688020, " PMA Debug Alignment Control %lid               " ,4,0x4)
        ,REG_INFO_MAC(0x00688030, " PMA Sticky Status                              " ,1,0x4)
        ,REG_INFO_MAC(0x00688034, " PMA 66b Alignment Status                       " ,1,0x4)
        ,REG_INFO_MAC(0x00688038, " PMA Debug Clock Domain Crossing Sticky Status  " ,1,0x4)
        ,REG_INFO_MAC(0x0068803C, " PMA Metal Fix                                  " ,1,0x4)
        ,REG_INFO_MAC(0x00688040, " PMA FEC Corrected Error Count for lane%n       " ,4,0x4)
        ,REG_INFO_MAC(0x00688050, " PMA FEC uncorrected err count for lane %n      " ,4,0x4)
        ,REG_INFO_MAC(0x00688060, " PMA Event Cause                                " ,1,0x4)
        ,REG_INFO_MAC(0x00688064, " PMA Event Mask                                 " ,1,0x4)
        ,REG_INFO_MAC(0x00688068, " PMA Register Access Status                     " ,1,0x4)


        ,REG_INFO_MAC(0x00682000, " ID Register                                    " ,1,0x4)
        ,REG_INFO_MAC(0x00682004, " autostart                                      " ,1,0x4)
        ,REG_INFO_MAC(0x00682008, " Autostart error flags                          " ,1,0x4)
        ,REG_INFO_MAC(0x0068200C, " Autostart Calibration Mask                     " ,1,0x4)
        ,REG_INFO_MAC(0x00682010, " Autostart Calibration Status                   " ,1,0x4)
        ,REG_INFO_MAC(0x00682014, " Autostart Rx PLL mask                          " ,1,0x4)
        ,REG_INFO_MAC(0x00682018, " bgcal                                          " ,1,0x4)
        ,REG_INFO_MAC(0x0068201C, " Configuration                                  " ,1,0x4)
        ,REG_INFO_MAC(0x00682040, " Fixed current trim                             " ,1,0x4)
        ,REG_INFO_MAC(0x00682044, " Fixed bias trim                                " ,1,0x4)
        ,REG_INFO_MAC(0x00682048, " iftrimovr                                      " ,1,0x4)
        ,REG_INFO_MAC(0x0068204C, " Fixed bias trim override mode                  " ,1,0x4)
        ,REG_INFO_MAC(0x00682050, " Rx bias configuration                          " ,1,0x4)
        ,REG_INFO_MAC(0x00682054, " bgmode                                         " ,1,0x4)
        ,REG_INFO_MAC(0x00682058, " termovr                                        " ,1,0x4)
        ,REG_INFO_MAC(0x0068205C, " Calibration termination                        " ,1,0x4)
        ,REG_INFO_MAC(0x00682060, " termcalovr                                     " ,1,0x4)
        ,REG_INFO_MAC(0x00682064, " Calibration termination reference current      " ,1,0x4)
        ,REG_INFO_MAC(0x00682068, " Calibration termination common mode voltage    " ,1,0x4)
        ,REG_INFO_MAC(0x0068206C, " compinvovr                                     " ,1,0x4)
        ,REG_INFO_MAC(0x00682070, " Calibration Clock Configuration                " ,1,0x4)
        ,REG_INFO_MAC(0x00682074, " Fixed Bias Cal params                          " ,1,0x4)
        ,REG_INFO_MAC(0x00682078, " Termination cal params                         " ,1,0x4)
        ,REG_INFO_MAC(0x0068207C, " rerun                                          " ,1,0x4)
        ,REG_INFO_MAC(0x00682080, " DAC Configuration                              " ,1,0x4)
        ,REG_INFO_MAC(0x006820D0, " Rx PLL charge pump current                     " ,1,0x4)
        ,REG_INFO_MAC(0x006820D4, " Rx PLL loop filter                             " ,1,0x4)
        ,REG_INFO_MAC(0x006820D8, " Refclk detect config                           " ,1,0x4)
        ,REG_INFO_MAC(0x006820DC, " Rx PLL FLA current                             " ,1,0x4)
        ,REG_INFO_MAC(0x006820F0, " PLL reg level                                  " ,1,0x4)
        ,REG_INFO_MAC(0x006820F4, " PLL reg compensation                           " ,1,0x4)
        ,REG_INFO_MAC(0x006820F8, " PLL reg reference                              " ,1,0x4)
        ,REG_INFO_MAC(0x00682100, " Temperature Sensor                             " ,1,0x4)
        ,REG_INFO_MAC(0x00682140, " Debugclk configuration                         " ,1,0x4)
        ,REG_INFO_MAC(0x00682144, " Anabus Configuration                           " ,1,0x4)
        ,REG_INFO_MAC(0x00682148, " Anabus Selection                               " ,1,0x4)
        ,REG_INFO_MAC(0x0068214C, " Ananbus mux (data tile)                        " ,1,0x4)
        ,REG_INFO_MAC(0x00682158, " Rxcon FSM                                      " ,1,0x4)
        ,REG_INFO_MAC(0x00682180, " Power scheduler                                " ,1,0x4)
        ,REG_INFO_MAC(0x00682184, " Power scheduler delay                          " ,1,0x4)
        ,REG_INFO_MAC(0x00682340, " Spare Observes                                 " ,1,0x4)
        ,REG_INFO_MAC(0x00682344, " Spare Controls                                 " ,1,0x4)

#define RXIF_RXi(name,offsetFrom0) \
         REG_INFO_MAC(0x00682400+offsetFrom0,name " Auto calibration mask                          " ,1,0x4)      \
        ,REG_INFO_MAC(0x00682404+offsetFrom0,name " Auto calibration status                        " ,1,0x4)      \
        ,REG_INFO_MAC(0x00682408+offsetFrom0,name " Rerun auto-calibration                         " ,1,0x4)      \
        ,REG_INFO_MAC(0x0068240C+offsetFrom0,name " Analysis mode                                  " ,1,0x4)      \
        ,REG_INFO_MAC(0x00682410+offsetFrom0,name " Coarse offset calibration word count           " ,1,0x4)      \
        ,REG_INFO_MAC(0x00682414+offsetFrom0,name " Fine offset calibration word count             " ,1,0x4)      \
        ,REG_INFO_MAC(0x00682418+offsetFrom0,name " Eyecope Configuration Register                 " ,1,0x4)      \
        ,REG_INFO_MAC(0x0068241C+offsetFrom0,name " Eye scope gating time                          " ,1,0x4)      \
        ,REG_INFO_MAC(0x00682420+offsetFrom0,name " Eyescope level                                 " ,1,0x4)      \
        ,REG_INFO_MAC(0x00682424+offsetFrom0,name " Data invert                                    " ,1,0x4)      \
        ,REG_INFO_MAC(0x00682428+offsetFrom0,name " RX Data Calibration Status Flag Register       " ,1,0x4)      \
        ,REG_INFO_MAC(0x0068242C+offsetFrom0,name " Coarse offset threshold                        " ,1,0x4)      \
        ,REG_INFO_MAC(0x00682430+offsetFrom0,name " Fine offset threshold                          " ,1,0x4)      \
        ,REG_INFO_MAC(0x00682434+offsetFrom0,name " Word synchronization threshold                 " ,1,0x4)      \
        ,REG_INFO_MAC(0x00682438+offsetFrom0,name " Word synchronization cycle count               " ,1,0x4)      \
        ,REG_INFO_MAC(0x0068243C+offsetFrom0,name " DAC update delay                               " ,1,0x4)      \
        ,REG_INFO_MAC(0x00682440+offsetFrom0,name " DAC0 output value                              " ,1,0x4)      \
        ,REG_INFO_MAC(0x00682444+offsetFrom0,name " DAC1 output value                              " ,1,0x4)      \
        ,REG_INFO_MAC(0x00682448+offsetFrom0,name " DAC2 output value                              " ,1,0x4)      \
        ,REG_INFO_MAC(0x0068244C+offsetFrom0,name " DAC3 output value                              " ,1,0x4)      \
        ,REG_INFO_MAC(0x00682450+offsetFrom0,name " QMC values                                     " ,1,0x4)      \
        ,REG_INFO_MAC(0x006824A4+offsetFrom0,name " QMC values                                     " ,1,0x4)      \
        ,REG_INFO_MAC(0x006824A8+offsetFrom0,name " QMC values                                     " ,1,0x4)      \
        ,REG_INFO_MAC(0x006824AC+offsetFrom0,name " QMC select override                            " ,1,0x4)      \
        ,REG_INFO_MAC(0x006824B0+offsetFrom0,name " QMC Output Status Register                     " ,1,0x4)      \
        ,REG_INFO_MAC(0x006824B4+offsetFrom0,name " QMC Manual Trigger/Lab Control Register        " ,1,0x4)      \
        ,REG_INFO_MAC(0x006824C0+offsetFrom0,name " Sampler offset                                 " ,(0x00682534-0x006824C0+4)/4,0x4) \
        ,REG_INFO_MAC(0x00682540+offsetFrom0,name " Sampler offset override select [7:0]           " ,1,0x4)      \
        ,REG_INFO_MAC(0x00682544+offsetFrom0,name " Sampler offset override select [15:8]          " ,1,0x4)      \
        ,REG_INFO_MAC(0x00682548+offsetFrom0,name " Sampler offset override select [23:16]         " ,1,0x4)      \
        ,REG_INFO_MAC(0x0068254C+offsetFrom0,name " Sampler offset override select [29:24]         " ,1,0x4)      \
        ,REG_INFO_MAC(0x00682550+offsetFrom0,name " Sampler offset override value                  " ,1,0x4)      \
        ,REG_INFO_MAC(0x00682554+offsetFrom0,name " Sampler offset override enable                 " ,1,0x4)      \
        ,REG_INFO_MAC(0x00682558+offsetFrom0,name " DAC Select Override Register                   " ,1,0x4)      \
        ,REG_INFO_MAC(0x0068255C+offsetFrom0,name " DAC sign invert                                " ,1,0x4)      \
        ,REG_INFO_MAC(0x00682560+offsetFrom0,name " Data Output Mux Select Override Control Registe" ,1,0x4)      \
        ,REG_INFO_MAC(0x00682570+offsetFrom0,name " Error count value [7:0]                        " ,1,0x4)      \
        ,REG_INFO_MAC(0x00682574+offsetFrom0,name " Error count value [15:8]                       " ,1,0x4)      \
        ,REG_INFO_MAC(0x00682578+offsetFrom0,name " Error count value [23:16]                      " ,1,0x4)      \
        ,REG_INFO_MAC(0x00682580+offsetFrom0,name " Error count value [7:0]                        " ,1,0x4)      \
        ,REG_INFO_MAC(0x00682584+offsetFrom0,name " Error count value [15:8]                       " ,1,0x4)      \
        ,REG_INFO_MAC(0x00682588+offsetFrom0,name " Error count value [23:16]                      " ,1,0x4)      \
        ,REG_INFO_MAC(0x00682590+offsetFrom0,name " Error count value [7:0]                        " ,1,0x4)      \
        ,REG_INFO_MAC(0x00682594+offsetFrom0,name " Error count value [15:8]                       " ,1,0x4)      \
        ,REG_INFO_MAC(0x00682598+offsetFrom0,name " Error count value [23:16]                      " ,1,0x4)      \
        ,REG_INFO_MAC(0x006825A0+offsetFrom0,name " Error count value [7:0]                        " ,1,0x4)      \
        ,REG_INFO_MAC(0x006825A4+offsetFrom0,name " Error count value [15:8]                       " ,1,0x4)      \
        ,REG_INFO_MAC(0x006825A8+offsetFrom0,name " Error count value [23:16]                      " ,1,0x4)      \
        ,REG_INFO_MAC(0x006825B0+offsetFrom0,name " Error count value [7:0]                        " ,1,0x4)      \
        ,REG_INFO_MAC(0x006825B4+offsetFrom0,name " Error count value [15:8]                       " ,1,0x4)      \
        ,REG_INFO_MAC(0x006825B8+offsetFrom0,name " Error count value [23:16]                      " ,1,0x4)      \
        ,REG_INFO_MAC(0x006825C4+offsetFrom0,name " OK state                                       " ,1,0x4)      \
        ,REG_INFO_MAC(0x006825C8+offsetFrom0,name " PRBS Pattern Checker Configuration Register    " ,1,0x4)      \
        ,REG_INFO_MAC(0x006825D0+offsetFrom0,name " Open-loop PRBS select                          " ,1,0x4)      \
        ,REG_INFO_MAC(0x006825D4+offsetFrom0,name " Pattern Checker Reset and Snap Control Register" ,1,0x4)      \
        ,REG_INFO_MAC(0x006825DC+offsetFrom0,name " Error status                                   " ,1,0x4)      \
        ,REG_INFO_MAC(0x006825E0+offsetFrom0,name " UI mask [7:0]                                  " ,1,0x4)      \
        ,REG_INFO_MAC(0x006825E4+offsetFrom0,name " UI mask [15:8]                                 " ,1,0x4)      \
        ,REG_INFO_MAC(0x006825E8+offsetFrom0,name " UI mask [23:16]                                " ,1,0x4)      \
        ,REG_INFO_MAC(0x006825EC+offsetFrom0,name " UI mask [31:24]                                " ,1,0x4)      \
        ,REG_INFO_MAC(0x006825F0+offsetFrom0,name " User pattern                                   " ,(0x0068263C-0x006825F0+4)/4,0x4) \
        ,REG_INFO_MAC(0x00682640+offsetFrom0,name " User pattern mode selection                    " ,1,0x4)      \
        ,REG_INFO_MAC(0x00682644+offsetFrom0,name " XOR mode selection                             " ,1,0x4)      \
        ,REG_INFO_MAC(0x00682648+offsetFrom0,name " Duration of initial CDA run                    " ,1,0x4)      \
        ,REG_INFO_MAC(0x0068264C+offsetFrom0,name " CDA sub-channel select                         " ,1,0x4)      \
        ,REG_INFO_MAC(0x00682650+offsetFrom0,name " Kp constant                                    " ,1,0x4)      \
        ,REG_INFO_MAC(0x00682654+offsetFrom0,name " CDA Configuration Register                     " ,1,0x4)      \
        ,REG_INFO_MAC(0x00682658+offsetFrom0,name " CDA Clock Select Override Register             " ,1,0x4)      \
        ,REG_INFO_MAC(0x0068265C+offsetFrom0,name " CDA clock phase select                         " ,1,0x4)      \
        ,REG_INFO_MAC(0x00682660+offsetFrom0,name " CDA Calibration Smoothing                      " ,1,0x4)      \
        ,REG_INFO_MAC(0x00682664+offsetFrom0,name " CDA calibration cycle count                    " ,1,0x4)      \
        ,REG_INFO_MAC(0x00682680+offsetFrom0,name " PI code value                                  " ,1,0x4)      \
        ,REG_INFO_MAC(0x00682684+offsetFrom0,name " PI code override value                         " ,1,0x4)      \
        ,REG_INFO_MAC(0x0068268C+offsetFrom0,name " CDA Snapshot Register                          " ,1,0x4)      \
        ,REG_INFO_MAC(0x00682690+offsetFrom0,name " CDA data sampler snapshot [7:0]                " ,1,0x4)      \
        ,REG_INFO_MAC(0x00682694+offsetFrom0,name " CDA data sampler snapshot [15:8]               " ,1,0x4)      \
        ,REG_INFO_MAC(0x00682698+offsetFrom0,name " CDA data sampler snapshot [23:16]              " ,1,0x4)      \
        ,REG_INFO_MAC(0x0068269C+offsetFrom0,name " CDA data sampler snapshot [31:24]              " ,1,0x4)      \
        ,REG_INFO_MAC(0x006826A0+offsetFrom0,name " CDA spare sampler snapshot [7:0]               " ,1,0x4)      \
        ,REG_INFO_MAC(0x006826A4+offsetFrom0,name " CDA spare sampler snapshot [15:8]              " ,1,0x4)      \
        ,REG_INFO_MAC(0x006826A8+offsetFrom0,name " Data Jog/Slip Configuration Register           " ,1,0x4)      \
        ,REG_INFO_MAC(0x006826C0+offsetFrom0,name " Scheduler Configuration Register               " ,1,0x4)      \
        ,REG_INFO_MAC(0x006826C4+offsetFrom0,name " Scheduler exponential period                   " ,1,0x4)      \
        ,REG_INFO_MAC(0x006826C8+offsetFrom0,name " Scheduler CDA exponential period               " ,1,0x4)      \
        ,REG_INFO_MAC(0x006826CC+offsetFrom0,name " Scheduler background divider                   " ,1,0x4)      \
        ,REG_INFO_MAC(0x006826D0+offsetFrom0,name " Scheduler state                                " ,1,0x4)      \
        ,REG_INFO_MAC(0x00682700+offsetFrom0,name " CDA offset override select                     " ,1,0x4)      \
        ,REG_INFO_MAC(0x00682704+offsetFrom0,name " CDA offset                                     " ,1,0x4)      \
        ,REG_INFO_MAC(0x00682708+offsetFrom0,name " CDA offset                                     " ,1,0x4)      \
        ,REG_INFO_MAC(0x0068270C+offsetFrom0,name " CDA offset                                     " ,1,0x4)      \
        ,REG_INFO_MAC(0x00682710+offsetFrom0,name " CDA offset                                     " ,1,0x4)      \
        ,REG_INFO_MAC(0x00682714+offsetFrom0,name " CDA offset                                     " ,1,0x4)      \
        ,REG_INFO_MAC(0x00682718+offsetFrom0,name " CDA offset clip value                          " ,1,0x4)      \
        ,REG_INFO_MAC(0x0068271C+offsetFrom0,name " CDA offset calibration duration                " ,1,0x4)      \
        ,REG_INFO_MAC(0x00682720+offsetFrom0,name " CDA offset scaling                             " ,1,0x4)      \
        ,REG_INFO_MAC(0x00682724+offsetFrom0,name " Eye centering level                            " ,1,0x4)      \
        ,REG_INFO_MAC(0x00682728+offsetFrom0,name " Eye centering max PI                           " ,1,0x4)      \
        ,REG_INFO_MAC(0x0068272C+offsetFrom0,name " Eye centering duration                         " ,1,0x4)      \
        ,REG_INFO_MAC(0x00682730+offsetFrom0,name " Left side PI steps                             " ,1,0x4)      \
        ,REG_INFO_MAC(0x00682734+offsetFrom0,name " Right side PI steps                            " ,1,0x4)      \
        ,REG_INFO_MAC(0x00682738+offsetFrom0,name " Centering threshold                            " ,1,0x4)      \
                                                                                                                  \
        ,REG_INFO_MAC(0x00682740+offsetFrom0,name " CTLE control                                   " ,1,0x4)      \
        ,REG_INFO_MAC(0x00682744+offsetFrom0,name " Rx PLL Control                                 " ,1,0x4)      \
        ,REG_INFO_MAC(0x00682748+offsetFrom0,name " RX Data Configuration Register                 " ,1,0x4)      \
        ,REG_INFO_MAC(0x0068274C+offsetFrom0,name " PLL unlock status                              " ,1,0x4)      \
        ,REG_INFO_MAC(0x00682750+offsetFrom0,name " PLL lock control                               " ,1,0x4)      \
        ,REG_INFO_MAC(0x00682754+offsetFrom0,name " PLL Calibration Control Register               " ,1,0x4)      \
        ,REG_INFO_MAC(0x00682758+offsetFrom0,name " PLL Calibration Status Register                " ,1,0x4)      \
        ,REG_INFO_MAC(0x0068275C+offsetFrom0,name " PLL Regulator Control                          " ,1,0x4)      \
        ,REG_INFO_MAC(0x00682760+offsetFrom0,name " PLL Single shot trigger Control registerr      " ,1,0x4)      \
        ,REG_INFO_MAC(0x00682764+offsetFrom0,name " PLL single shot count tolerance                " ,1,0x4)      \
        ,REG_INFO_MAC(0x00682768+offsetFrom0,name " PLL single shot expected count [7:0]           " ,1,0x4)      \
        ,REG_INFO_MAC(0x0068276C+offsetFrom0,name " PLL single shot expected count [12:8]          " ,1,0x4)      \
        ,REG_INFO_MAC(0x00682770+offsetFrom0,name " PLL cal single shot result [7:0]               " ,1,0x4)      \
        ,REG_INFO_MAC(0x00682774+offsetFrom0,name " PLL cal single shot result [12:8]              " ,1,0x4)      \
        ,REG_INFO_MAC(0x00682778+offsetFrom0,name " PLL cal DAC iterations counter                 " ,1,0x4)      \
        ,REG_INFO_MAC(0x00682780+offsetFrom0,name " Rx Termination Config                          " ,1,0x4)      \
        ,REG_INFO_MAC(0x00682784+offsetFrom0,name " CTLE Equalisation                              " ,1,0x4)      \
        ,REG_INFO_MAC(0x00682788+offsetFrom0,name " PLL control voltage DAC                        " ,1,0x4)      \
        ,REG_INFO_MAC(0x0068278C+offsetFrom0,name " PLL DAC Override Control Register              " ,1,0x4)      \
        ,REG_INFO_MAC(0x00682790+offsetFrom0,name " pregupmask                                     " ,1,0x4)      \
        ,REG_INFO_MAC(0x00682794+offsetFrom0,name " plock_ovr                                      " ,1,0x4)      \
        ,REG_INFO_MAC(0x00682798+offsetFrom0,name " RX Duty Cycle Control Register                 " ,1,0x4)      \
        ,REG_INFO_MAC(0x0068279C+offsetFrom0,name " Loopback Termination trimming.                 " ,1,0x4)      \
        ,REG_INFO_MAC(0x006827A0+offsetFrom0,name " PLL cal startup delay                          " ,1,0x4)      \
        ,REG_INFO_MAC(0x006827A4+offsetFrom0,name " PLL cal DAC delay                              " ,1,0x4)      \
        ,REG_INFO_MAC(0x006827A8+offsetFrom0,name " PLL cal FLA delay                              " ,1,0x4)      \
        ,REG_INFO_MAC(0x006827AC+offsetFrom0,name " PLL calibration lock FLA delay                 " ,1,0x4)      \
        ,REG_INFO_MAC(0x006827C0+offsetFrom0,name " Autostart Duration Counter [7:0]               " ,1,0x4)      \
        ,REG_INFO_MAC(0x006827C4+offsetFrom0,name " Autostart Duration Counter [15:8]              " ,1,0x4)      \
        ,REG_INFO_MAC(0x006827C8+offsetFrom0,name " Autostart Duration Counter [23:16]             " ,1,0x4)      \
        ,REG_INFO_MAC(0x006827E0+offsetFrom0,name " Wire 0 deskew                                  " ,1,0x4)      \
        ,REG_INFO_MAC(0x006827E4+offsetFrom0,name " Wire 1 deskew                                  " ,1,0x4)      \
        ,REG_INFO_MAC(0x006827E8+offsetFrom0,name " Wire 2 deskew                                  " ,1,0x4)      \
        ,REG_INFO_MAC(0x006827EC+offsetFrom0,name " Wire 3 deskew                                  " ,1,0x4)      \
        ,REG_INFO_MAC(0x006827F0+offsetFrom0,name " Wire 4 deskew                                  " ,1,0x4)      \
        ,REG_INFO_MAC(0x006827F4+offsetFrom0,name " Wire 5 deskew                                  " ,1,0x4)      \
        ,REG_INFO_MAC(0x006827F8+offsetFrom0,name " Spare Observes                                 " ,1,0x4)      \
        ,REG_INFO_MAC(0x006827FC+offsetFrom0,name " Spare Controls                                 " ,1,0x4)

        ,RXIF_RXi("rx0" ,0x000)
        ,RXIF_RXi("rx1" ,0x400)
        ,RXIF_RXi("rx2" ,0x800)
        ,RXIF_RXi("rx3" ,0xc00)

        ,RXIF_RXi("rxbr",0x1800)

        ,REG_INFO_MAC(0x00680004, " APB Wait States                                             " ,1,0x4)
        ,REG_INFO_MAC(0x00680008, " Test Override Switches [7:0]                                " ,1,0x4)
        ,REG_INFO_MAC(0x0068000C, " Test Override Switches [9:8]                                " ,1,0x4)
        ,REG_INFO_MAC(0x00680010, " TX Clock Tile Override Control Register                     " ,1,0x4)
        ,REG_INFO_MAC(0x00680014, " TX Clock Tile Configuration Register                        " ,1,0x4)
        ,REG_INFO_MAC(0x00680018, " Clock Driver Slice Enable [7:0]                             " ,1,0x4)
        ,REG_INFO_MAC(0x0068001C, " Clock Driver Slice Enable [8:8]                             " ,1,0x4)
        ,REG_INFO_MAC(0x00680020, " PLL Lock Count 0                                            " ,1,0x4)
        ,REG_INFO_MAC(0x00680024, " PLL Lock Count 1                                            " ,1,0x4)
        ,REG_INFO_MAC(0x00680028, " Clock Common Mode Termination Enable                        " ,1,0x4)
        ,REG_INFO_MAC(0x0068002C, " Clock Equal Eye Common Mode Termination Enable              " ,1,0x4)
        ,REG_INFO_MAC(0x00680038, " Spare Observes                                              " ,1,0x4)
        ,REG_INFO_MAC(0x0068003C, " Spare Controls                                              " ,1,0x4)
        ,REG_INFO_MAC(0x00680040, " Tx PLL Control                                              " ,1,0x4)
        ,REG_INFO_MAC(0x00680044, " PLL Lock Status                                             " ,1,0x4)
        ,REG_INFO_MAC(0x00680048, " Clock Buffer I Swing Amplitude                              " ,1,0x4)
        ,REG_INFO_MAC(0x0068004C, " Clock Buffer Q Swing Amplitude                              " ,1,0x4)
        ,REG_INFO_MAC(0x00680050, " PLL Charge Pump Control                                     " ,1,0x4)
        ,REG_INFO_MAC(0x00680054, " PLL Loop Control                                            " ,1,0x4)
        ,REG_INFO_MAC(0x00680058, " PLL VCO Capacitive Load control                             " ,1,0x4)
        ,REG_INFO_MAC(0x0068005C, " PLL Regulator Control                                       " ,1,0x4)
        ,REG_INFO_MAC(0x00680060, " PLL Feedback Divider [7:0]                                  " ,1,0x4)
        ,REG_INFO_MAC(0x00680064, " PLL Feedback Divider [8:8]                                  " ,1,0x4)
        ,REG_INFO_MAC(0x00680068, " Debug Clock Control                                         " ,1,0x4)
        ,REG_INFO_MAC(0x00680070, " Correction Clock Control 1                                  " ,1,0x4)
        ,REG_INFO_MAC(0x00680074, " Correction Clock Control 2                                  " ,1,0x4)
        ,REG_INFO_MAC(0x00680078, " Correction Clock Control 3                                  " ,1,0x4)
        ,REG_INFO_MAC(0x0068007C, " corrstatus                                                  " ,1,0x4)
        ,REG_INFO_MAC(0x00680080, " Tx Clock Tile Clock Enable                                  " ,1,0x4)
        ,REG_INFO_MAC(0x00680084, " Tx Clock Tile DCD Correction                                " ,1,0x4)
        ,REG_INFO_MAC(0x00680098, " Bias Current Control                                        " ,1,0x4)
        ,REG_INFO_MAC(0x0068009C, " Tx Clock Tile Autozero Comparator Source Select             " ,1,0x4)
        ,REG_INFO_MAC(0x006800A0, " Tx Clock Tile Test Mode                                     " ,1,0x4)
        ,REG_INFO_MAC(0x006800A4, " Tx Clock Tile Phase Interpolator Bias Current Trim          " ,1,0x4)
        ,REG_INFO_MAC(0x006800A8, " Tx Clock Tile Production Test Phase Interpolator            " ,1,0x4)
        ,REG_INFO_MAC(0x006800AC, " Tx Clock Tile Amplitude Detector Control                    " ,1,0x4)
        ,REG_INFO_MAC(0x006800B4, " Invert sense of slc thresholds                              " ,1,0x4)
        ,REG_INFO_MAC(0x006800B8, " Slice Control Threshold 0                                   " ,1,0x4)
        ,REG_INFO_MAC(0x006800BC, " Slice Control Threshold 1                                   " ,1,0x4)
        ,REG_INFO_MAC(0x006800C0, " Analog Mux Select                                           " ,1,0x4)
        ,REG_INFO_MAC(0x006800C4, " Reference Clock Bias Current Trim                           " ,1,0x4)
        ,REG_INFO_MAC(0x006800C8, " Bias Control 1                                              " ,1,0x4)
        ,REG_INFO_MAC(0x006800CC, " Bias Control 2                                              " ,1,0x4)
        ,REG_INFO_MAC(0x006800D0, " Bias Control 3                                              " ,1,0x4)
        ,REG_INFO_MAC(0x006800D8, " Bias Control 4                                              " ,1,0x4)
        ,REG_INFO_MAC(0x006800DC, " Bias Control 5                                              " ,1,0x4)
        ,REG_INFO_MAC(0x00680100, " Tx Data Tile 0 Clock Enable                                 " ,1,0x4)
        ,REG_INFO_MAC(0x00680104, " Tx Data Tile 0 IQ Skew Control                              " ,1,0x4)
        ,REG_INFO_MAC(0x00680108, " Tx Data Tile 0 Ip DCD Correction                            " ,1,0x4)
        ,REG_INFO_MAC(0x00680110, " Tx Data Tile 0 Qp DCD Correction                            " ,1,0x4)
        ,REG_INFO_MAC(0x00680118, " Data Tile 0 Bias Control                                    " ,1,0x4)
        ,REG_INFO_MAC(0x0068011C, " Tx Data Tile 0 Autozero Comparator Source Select            " ,1,0x4)
        ,REG_INFO_MAC(0x00680140, " Tx Data Tile 0 Clock Enable                                 " ,1,0x4)
        ,REG_INFO_MAC(0x00680144, " Tx Data Tile 0 IQ Skew Control                              " ,1,0x4)
        ,REG_INFO_MAC(0x00680148, " Tx Data Tile 0 Ip DCD Correction                            " ,1,0x4)
        ,REG_INFO_MAC(0x00680150, " Tx Data Tile 0 Qp DCD Correction                            " ,1,0x4)
        ,REG_INFO_MAC(0x00680158, " Data Tile 0 Bias Control                                    " ,1,0x4)
        ,REG_INFO_MAC(0x0068015C, " Tx Data Tile 0 Autozero Comparator Source Select            " ,1,0x4)
        ,REG_INFO_MAC(0x00680180, " Tx Data Tile 0 Clock Enable                                 " ,1,0x4)
        ,REG_INFO_MAC(0x00680184, " Tx Data Tile 0 IQ Skew Control                              " ,1,0x4)
        ,REG_INFO_MAC(0x00680188, " Tx Data Tile 0 Ip DCD Correction                            " ,1,0x4)
        ,REG_INFO_MAC(0x00680190, " Tx Data Tile 0 Qp DCD Correction                            " ,1,0x4)
        ,REG_INFO_MAC(0x00680198, " Data Tile 0 Bias Control                                    " ,1,0x4)
        ,REG_INFO_MAC(0x0068019C, " Tx Data Tile 0 Autozero Comparator Source Select            " ,1,0x4)
        ,REG_INFO_MAC(0x006801C0, " Tx Data Tile 0 Clock Enable                                 " ,1,0x4)
        ,REG_INFO_MAC(0x006801C4, " Tx Data Tile 0 IQ Skew Control                              " ,1,0x4)
        ,REG_INFO_MAC(0x006801C8, " Tx Data Tile 0 Ip DCD Correction                            " ,1,0x4)
        ,REG_INFO_MAC(0x006801D0, " Tx Data Tile 0 Qp DCD Correction                            " ,1,0x4)
        ,REG_INFO_MAC(0x006801D8, " Data Tile 0 Bias Control                                    " ,1,0x4)
        ,REG_INFO_MAC(0x006801DC, " Tx Data Tile 0 Autozero Comparator Source Select            " ,1,0x4)
        ,REG_INFO_MAC(0x006801E4, " SST Driver Enable Obs                                       " ,1,0x4)
        ,REG_INFO_MAC(0x00680200, " Observable Clock Tile FSM state [7:0]                       " ,1,0x4)
        ,REG_INFO_MAC(0x00680204, " Observable Clock Tile FSM state [15:8]                      " ,1,0x4)
        ,REG_INFO_MAC(0x00680208, " Observable Data Tile 0 FSM state [7:0]                      " ,1,0x4)
        ,REG_INFO_MAC(0x0068020C, " Observable Data Tile 0 FSM state [15:8]                     " ,1,0x4)
        ,REG_INFO_MAC(0x00680210, " Observable Data Tile 1 FSM state [7:0]                      " ,1,0x4)
        ,REG_INFO_MAC(0x00680214, " Observable Data Tile 1 FSM state [15:8]                     " ,1,0x4)
        ,REG_INFO_MAC(0x00680218, " Observable Data Tile 2 FSM state [7:0]                      " ,1,0x4)
        ,REG_INFO_MAC(0x0068021C, " Observable Data Tile 2 FSM state [15:8]                     " ,1,0x4)
        ,REG_INFO_MAC(0x00680220, " Observable Data Tile 3 FSM state [7:0]                      " ,1,0x4)
        ,REG_INFO_MAC(0x00680224, " Observable Data Tile 3 FSM state [15:8]                     " ,1,0x4)
        ,REG_INFO_MAC(0x00680228, " Observable Clock Tile Autozero Comparator Source Select     " ,1,0x4)
        ,REG_INFO_MAC(0x00680230, " Observable Clock buffer I/Q swing amplitude                 " ,1,0x4)
        ,REG_INFO_MAC(0x00680238, " Observable Clock Tile Duty Cycle Control                    " ,1,0x4)
        ,REG_INFO_MAC(0x00680240, " Observable Data Tile 0 Autozero Comparator Source Select    " ,1,0x4)
        ,REG_INFO_MAC(0x00680244, " Observable Data Tile 0 IQ Skew Control                      " ,1,0x4)
        ,REG_INFO_MAC(0x00680248, " Observable Data Tile 0 Ip/m DCD correction                  " ,1,0x4)
        ,REG_INFO_MAC(0x0068024C, " Observable Data Tile 0 Qp/m DCD correction                  " ,1,0x4)
        ,REG_INFO_MAC(0x00680250, " Observable Data Tile 1 Autozero Comparator Source Select    " ,1,0x4)
        ,REG_INFO_MAC(0x00680254, " Observable Data Tile 1 IQ Skew Control                      " ,1,0x4)
        ,REG_INFO_MAC(0x00680258, " Observable Data Tile 1 Ip/m DCD correction                  " ,1,0x4)
        ,REG_INFO_MAC(0x0068025C, " Observable Data Tile 1 Qp/m DCD correction                  " ,1,0x4)
        ,REG_INFO_MAC(0x00680260, " Observable Data Tile 2 Autozero Comparator Source Select    " ,1,0x4)
        ,REG_INFO_MAC(0x00680264, " Observable Data Tile 2 IQ Skew Control                      " ,1,0x4)
        ,REG_INFO_MAC(0x00680268, " Observable Data Tile 2 Ip/m DCD correction                  " ,1,0x4)
        ,REG_INFO_MAC(0x0068026C, " Observable Data Tile 2 Qp/m DCD correction                  " ,1,0x4)
        ,REG_INFO_MAC(0x00680270, " Observable Data Tile 3 Autozero Comparator Source Select    " ,1,0x4)
        ,REG_INFO_MAC(0x00680274, " Observable Data Tile 3 IQ Skew Control                      " ,1,0x4)
        ,REG_INFO_MAC(0x00680278, " Observable Data Tile 3 Ip/m DCD correction                  " ,1,0x4)
        ,REG_INFO_MAC(0x0068027C, " Observable Data Tile 3 Qp/m DCD correction                  " ,1,0x4)

        ,REG_INFO_MAC(0x00680440, " Configuration                             " ,1, 0x4)
        ,REG_INFO_MAC(0x00680444, " User Test Pattern Selection               " ,1, 0x4)
        ,REG_INFO_MAC(0x00680448, " Pattern select 1                          " ,1, 0x4)
        ,REG_INFO_MAC(0x00680450, " User Test Pattern 0 [7:0]                 " ,1, 0x4)
        ,REG_INFO_MAC(0x00680454, " User Test Pattern 0 [15:8]                " ,1, 0x4)
        ,REG_INFO_MAC(0x00680458, " User Test Pattern 0 [23:16]               " ,1, 0x4)
        ,REG_INFO_MAC(0x0068045C, " User Test Pattern 0 [31:24]               " ,1, 0x4)
        ,REG_INFO_MAC(0x00680460, " User Test Pattern 1 [7:0]                 " ,1, 0x4)
        ,REG_INFO_MAC(0x00680464, " User Test Pattern 1 [15:8]                " ,1, 0x4)
        ,REG_INFO_MAC(0x00680468, " User Test Pattern 1 [23:16]               " ,1, 0x4)
        ,REG_INFO_MAC(0x0068046C, " User Test Pattern 1 [31:24]               " ,1, 0x4)
        ,REG_INFO_MAC(0x00680470, " User Test Pattern 2 [7:0]                 " ,1, 0x4)
        ,REG_INFO_MAC(0x00680474, " User Test Pattern 2 [15:8]                " ,1, 0x4)
        ,REG_INFO_MAC(0x00680478, " User Test Pattern 2 [23:16]               " ,1, 0x4)
        ,REG_INFO_MAC(0x0068047C, " User Test Pattern 2 [31:24]               " ,1, 0x4)
        ,REG_INFO_MAC(0x00680480, " User Test Pattern 3 [7:0]                 " ,1, 0x4)
        ,REG_INFO_MAC(0x00680484, " User Test Pattern 3 [15:8]                " ,1, 0x4)
        ,REG_INFO_MAC(0x00680488, " User Test Pattern 3 [23:16]               " ,1, 0x4)
        ,REG_INFO_MAC(0x0068048C, " User Test Pattern 3 [31:24]               " ,1, 0x4)
        ,REG_INFO_MAC(0x00680490, " User Test Pattern 4 [7:0]                 " ,1, 0x4)
        ,REG_INFO_MAC(0x00680494, " User Test Pattern 4 [15:8]                " ,1, 0x4)
        ,REG_INFO_MAC(0x00680498, " User Test Pattern 4 [23:16]               " ,1, 0x4)
        ,REG_INFO_MAC(0x0068049C, " User Test Pattern 4 [31:24]               " ,1, 0x4)
        ,REG_INFO_MAC(0x006804A0, " Data Driver Slice Enable [7:0]            " ,1, 0x4)
        ,REG_INFO_MAC(0x006804A4, " Data Driver Slice Enable [11:8]           " ,1, 0x4)
        ,REG_INFO_MAC(0x006804A8, " Common Mode Termination Enable            " ,1, 0x4)
        ,REG_INFO_MAC(0x006804AC, " Equal Eye Common Mode Termination Enable  " ,1, 0x4)
        ,REG_INFO_MAC(0x006804B0, " SST Driver Enable                         " ,1, 0x4)
        ,REG_INFO_MAC(0x006804D0, " SST Driver Enable Obs                     " ,1, 0x4)
        ,REG_INFO_MAC(0x006804E0, " Loopback Data Driver Slice Enable [7:0]   " ,1, 0x4)
        ,REG_INFO_MAC(0x006804E4, " Loopback Data Driver Slice Enable [11:8]  " ,1, 0x4)
        ,REG_INFO_MAC(0x006804E8, " Loopback SST Driver Enable                " ,1, 0x4)
        ,REG_INFO_MAC(0x006804F0, " odctrl                                    " ,1, 0x4)
        ,REG_INFO_MAC(0x00680500, " Spare Observes                            " ,1, 0x4)
        ,REG_INFO_MAC(0x00680504, " Spare Controls                            " ,1, 0x4)

        ,REG_INFO_MAC(0x00680840, " Configuration                             " ,1, 0x4)
        ,REG_INFO_MAC(0x00680844, " User Test Pattern Selection               " ,1, 0x4)
        ,REG_INFO_MAC(0x00680848, " Pattern select 1                          " ,1, 0x4)
        ,REG_INFO_MAC(0x00680850, " User Test Pattern 0 [7:0]                 " ,1, 0x4)
       ,REG_INFO_MAC(0x00680854, " User Test Pattern 0 [15:8]                " ,1, 0x4)
        ,REG_INFO_MAC(0x00680858, " User Test Pattern 0 [23:16]               " ,1, 0x4)
        ,REG_INFO_MAC(0x0068085C, " User Test Pattern 0 [31:24]               " ,1, 0x4)
        ,REG_INFO_MAC(0x00680860, " User Test Pattern 1 [7:0]                 " ,1, 0x4)
        ,REG_INFO_MAC(0x00680864, " User Test Pattern 1 [15:8]                " ,1, 0x4)
        ,REG_INFO_MAC(0x00680868, " User Test Pattern 1 [23:16]               " ,1, 0x4)
        ,REG_INFO_MAC(0x0068086C, " User Test Pattern 1 [31:24]               " ,1, 0x4)
        ,REG_INFO_MAC(0x00680870, " User Test Pattern 2 [7:0]                 " ,1, 0x4)
        ,REG_INFO_MAC(0x00680874, " User Test Pattern 2 [15:8]                " ,1, 0x4)
        ,REG_INFO_MAC(0x00680878, " User Test Pattern 2 [23:16]               " ,1, 0x4)
        ,REG_INFO_MAC(0x0068087C, " User Test Pattern 2 [31:24]               " ,1, 0x4)
        ,REG_INFO_MAC(0x00680880, " User Test Pattern 3 [7:0]                 " ,1, 0x4)
        ,REG_INFO_MAC(0x00680884, " User Test Pattern 3 [15:8]                " ,1, 0x4)
        ,REG_INFO_MAC(0x00680888, " User Test Pattern 3 [23:16]               " ,1, 0x4)
        ,REG_INFO_MAC(0x0068088C, " User Test Pattern 3 [31:24]               " ,1, 0x4)
        ,REG_INFO_MAC(0x00680890, " User Test Pattern 4 [7:0]                 " ,1, 0x4)
        ,REG_INFO_MAC(0x00680894, " User Test Pattern 4 [15:8]                " ,1, 0x4)
        ,REG_INFO_MAC(0x00680898, " User Test Pattern 4 [23:16]               " ,1, 0x4)
        ,REG_INFO_MAC(0x0068089C, " User Test Pattern 4 [31:24]               " ,1, 0x4)
        ,REG_INFO_MAC(0x006808A0, " Data Driver Slice Enable [7:0]            " ,1, 0x4)
        ,REG_INFO_MAC(0x006808A4, " Data Driver Slice Enable [11:8]           " ,1, 0x4)
        ,REG_INFO_MAC(0x006808A8, " Common Mode Termination Enable            " ,1, 0x4)
        ,REG_INFO_MAC(0x006808AC, " Equal Eye Common Mode Termination Enable  " ,1, 0x4)
        ,REG_INFO_MAC(0x006808B0, " SST Driver Enable                         " ,1, 0x4)
        ,REG_INFO_MAC(0x006808D0, " SST Driver Enable Obs                     " ,1, 0x4)
        ,REG_INFO_MAC(0x006808E0, " Loopback Data Driver Slice Enable [7:0]   " ,1, 0x4)
        ,REG_INFO_MAC(0x006808E4, " Loopback Data Driver Slice Enable [11:8]  " ,1, 0x4)
        ,REG_INFO_MAC(0x006808E8, " Loopback SST Driver Enable                " ,1, 0x4)
        ,REG_INFO_MAC(0x006808F0, " odctrl                                    " ,1, 0x4)
        ,REG_INFO_MAC(0x00680900, " Spare Observes                            " ,1, 0x4)
        ,REG_INFO_MAC(0x00680904, " Spare Controls                            " ,1, 0x4)

        ,REG_INFO_MAC(0x00680C40, " Configuration                            " ,1,0x4)
        ,REG_INFO_MAC(0x00680C44, " User Test Pattern Selection              " ,1,0x4)
        ,REG_INFO_MAC(0x00680C48, " Pattern select 1                         " ,1,0x4)
        ,REG_INFO_MAC(0x00680C50, " User Test Pattern 0 [7:0]                " ,1,0x4)
        ,REG_INFO_MAC(0x00680C54, " User Test Pattern 0 [15:8]               " ,1,0x4)
        ,REG_INFO_MAC(0x00680C58, " User Test Pattern 0 [23:16]              " ,1,0x4)
        ,REG_INFO_MAC(0x00680C5C, " User Test Pattern 0 [31:24]              " ,1,0x4)
        ,REG_INFO_MAC(0x00680C60, " User Test Pattern 1 [7:0]                " ,1,0x4)
        ,REG_INFO_MAC(0x00680C64, " User Test Pattern 1 [15:8]               " ,1,0x4)
        ,REG_INFO_MAC(0x00680C68, " User Test Pattern 1 [23:16]              " ,1,0x4)
        ,REG_INFO_MAC(0x00680C6C, " User Test Pattern 1 [31:24]              " ,1,0x4)
        ,REG_INFO_MAC(0x00680C70, " User Test Pattern 2 [7:0]                " ,1,0x4)
        ,REG_INFO_MAC(0x00680C74, " User Test Pattern 2 [15:8]               " ,1,0x4)
        ,REG_INFO_MAC(0x00680C78, " User Test Pattern 2 [23:16]              " ,1,0x4)
        ,REG_INFO_MAC(0x00680C7C, " User Test Pattern 2 [31:24]              " ,1,0x4)
        ,REG_INFO_MAC(0x00680C80, " User Test Pattern 3 [7:0]                " ,1,0x4)
        ,REG_INFO_MAC(0x00680C84, " User Test Pattern 3 [15:8]               " ,1,0x4)
        ,REG_INFO_MAC(0x00680C88, " User Test Pattern 3 [23:16]              " ,1,0x4)
        ,REG_INFO_MAC(0x00680C8C, " User Test Pattern 3 [31:24]              " ,1,0x4)
        ,REG_INFO_MAC(0x00680C90, " User Test Pattern 4 [7:0]                " ,1,0x4)
        ,REG_INFO_MAC(0x00680C94, " User Test Pattern 4 [15:8]               " ,1,0x4)
        ,REG_INFO_MAC(0x00680C98, " User Test Pattern 4 [23:16]              " ,1,0x4)
        ,REG_INFO_MAC(0x00680C9C, " User Test Pattern 4 [31:24]              " ,1,0x4)
        ,REG_INFO_MAC(0x00680CA0, " Data Driver Slice Enable [7:0]           " ,1,0x4)
        ,REG_INFO_MAC(0x00680CA4, " Data Driver Slice Enable [11:8]          " ,1,0x4)
        ,REG_INFO_MAC(0x00680CA8, " Common Mode Termination Enable           " ,1,0x4)
        ,REG_INFO_MAC(0x00680CAC, " Equal Eye Common Mode Termination Enable " ,1,0x4)
        ,REG_INFO_MAC(0x00680CB0, " SST Driver Enable                        " ,1,0x4)
        ,REG_INFO_MAC(0x00680CD0, " SST Driver Enable Obs                    " ,1,0x4)
        ,REG_INFO_MAC(0x00680CE0, " Loopback Data Driver Slice Enable [7:0]  " ,1,0x4)
        ,REG_INFO_MAC(0x00680CE4, " Loopback Data Driver Slice Enable [11:8] " ,1,0x4)
        ,REG_INFO_MAC(0x00680CE8, " Loopback SST Driver Enable               " ,1,0x4)
        ,REG_INFO_MAC(0x00680CF0, " odctrl                                   " ,1,0x4)
        ,REG_INFO_MAC(0x00680D00, " Spare Observes                           " ,1,0x4)
        ,REG_INFO_MAC(0x00680D04, " Spare Controls                           " ,1,0x4)

        ,REG_INFO_MAC(0x00681040, " Configuration                            " ,1,0x4)
        ,REG_INFO_MAC(0x00681044, " User Test Pattern Selection              " ,1,0x4)
        ,REG_INFO_MAC(0x00681048, " Pattern select 1                         " ,1,0x4)
        ,REG_INFO_MAC(0x00681050, " User Test Pattern 0 [7:0]                " ,1,0x4)
        ,REG_INFO_MAC(0x00681054, " User Test Pattern 0 [15:8]               " ,1,0x4)
        ,REG_INFO_MAC(0x00681058, " User Test Pattern 0 [23:16]              " ,1,0x4)
        ,REG_INFO_MAC(0x0068105C, " User Test Pattern 0 [31:24]              " ,1,0x4)
        ,REG_INFO_MAC(0x00681060, " User Test Pattern 1 [7:0]                " ,1,0x4)
        ,REG_INFO_MAC(0x00681064, " User Test Pattern 1 [15:8]               " ,1,0x4)
        ,REG_INFO_MAC(0x00681068, " User Test Pattern 1 [23:16]              " ,1,0x4)
        ,REG_INFO_MAC(0x0068106C, " User Test Pattern 1 [31:24]              " ,1,0x4)
        ,REG_INFO_MAC(0x00681070, " User Test Pattern 2 [7:0]                " ,1,0x4)
        ,REG_INFO_MAC(0x00681074, " User Test Pattern 2 [15:8]               " ,1,0x4)
        ,REG_INFO_MAC(0x00681078, " User Test Pattern 2 [23:16]              " ,1,0x4)
        ,REG_INFO_MAC(0x0068107C, " User Test Pattern 2 [31:24]              " ,1,0x4)
        ,REG_INFO_MAC(0x00681080, " User Test Pattern 3 [7:0]                " ,1,0x4)
        ,REG_INFO_MAC(0x00681084, " User Test Pattern 3 [15:8]               " ,1,0x4)
        ,REG_INFO_MAC(0x00681088, " User Test Pattern 3 [23:16]              " ,1,0x4)
        ,REG_INFO_MAC(0x0068108C, " User Test Pattern 3 [31:24]              " ,1,0x4)
        ,REG_INFO_MAC(0x00681090, " User Test Pattern 4 [7:0]                " ,1,0x4)
        ,REG_INFO_MAC(0x00681094, " User Test Pattern 4 [15:8]               " ,1,0x4)
        ,REG_INFO_MAC(0x00681098, " User Test Pattern 4 [23:16]              " ,1,0x4)
        ,REG_INFO_MAC(0x0068109C, " User Test Pattern 4 [31:24]              " ,1,0x4)
        ,REG_INFO_MAC(0x006810A0, " Data Driver Slice Enable [7:0]           " ,1,0x4)
        ,REG_INFO_MAC(0x006810A4, " Data Driver Slice Enable [11:8]          " ,1,0x4)
        ,REG_INFO_MAC(0x006810A8, " Common Mode Termination Enable           " ,1,0x4)
        ,REG_INFO_MAC(0x006810AC, " Equal Eye Common Mode Termination Enable " ,1,0x4)
        ,REG_INFO_MAC(0x006810B0, " SST Driver Enable                        " ,1,0x4)
        ,REG_INFO_MAC(0x006810D0, " SST Driver Enable Obs                    " ,1,0x4)
        ,REG_INFO_MAC(0x006810E0, " Loopback Data Driver Slice Enable [7:0]  " ,1,0x4)
        ,REG_INFO_MAC(0x006810E4, " Loopback Data Driver Slice Enable [11:8] " ,1,0x4)
        ,REG_INFO_MAC(0x006810E8, " Loopback SST Driver Enable               " ,1,0x4)
        ,REG_INFO_MAC(0x006810F0, " odctrl                                   " ,1,0x4)
        ,REG_INFO_MAC(0x00681100, " Spare Observes                           " ,1,0x4)
        ,REG_INFO_MAC(0x00681104, " Spare Controls                           " ,1,0x4)

        ,REG_INFO_MAC(0x00681C40, " Configuration                            " ,1,0x4)
        ,REG_INFO_MAC(0x00681C44, " User Test Pattern Selection              " ,1,0x4)
        ,REG_INFO_MAC(0x00681C48, " Pattern select 1                         " ,1,0x4)
        ,REG_INFO_MAC(0x00681C50, " User Test Pattern 0 [7:0]                " ,1,0x4)
        ,REG_INFO_MAC(0x00681C54, " User Test Pattern 0 [15:8]               " ,1,0x4)
        ,REG_INFO_MAC(0x00681C58, " User Test Pattern 0 [23:16]              " ,1,0x4)
        ,REG_INFO_MAC(0x00681C5C, " User Test Pattern 0 [31:24]              " ,1,0x4)
        ,REG_INFO_MAC(0x00681C60, " User Test Pattern 1 [7:0]                " ,1,0x4)
        ,REG_INFO_MAC(0x00681C64, " User Test Pattern 1 [15:8]               " ,1,0x4)
        ,REG_INFO_MAC(0x00681C68, " User Test Pattern 1 [23:16]              " ,1,0x4)
        ,REG_INFO_MAC(0x00681C6C, " User Test Pattern 1 [31:24]              " ,1,0x4)
        ,REG_INFO_MAC(0x00681C70, " User Test Pattern 2 [7:0]                " ,1,0x4)
        ,REG_INFO_MAC(0x00681C74, " User Test Pattern 2 [15:8]               " ,1,0x4)
        ,REG_INFO_MAC(0x00681C78, " User Test Pattern 2 [23:16]              " ,1,0x4)
        ,REG_INFO_MAC(0x00681C7C, " User Test Pattern 2 [31:24]              " ,1,0x4)
        ,REG_INFO_MAC(0x00681C80, " User Test Pattern 3 [7:0]                " ,1,0x4)
        ,REG_INFO_MAC(0x00681C84, " User Test Pattern 3 [15:8]               " ,1,0x4)
        ,REG_INFO_MAC(0x00681C88, " User Test Pattern 3 [23:16]              " ,1,0x4)
        ,REG_INFO_MAC(0x00681C8C, " User Test Pattern 3 [31:24]              " ,1,0x4)
        ,REG_INFO_MAC(0x00681C90, " User Test Pattern 4 [7:0]                " ,1,0x4)
        ,REG_INFO_MAC(0x00681C94, " User Test Pattern 4 [15:8]               " ,1,0x4)
        ,REG_INFO_MAC(0x00681C98, " User Test Pattern 4 [23:16]              " ,1,0x4)
        ,REG_INFO_MAC(0x00681C9C, " User Test Pattern 4 [31:24]              " ,1,0x4)
        ,REG_INFO_MAC(0x00681CA0, " Data Driver Slice Enable [7:0]           " ,1,0x4)
        ,REG_INFO_MAC(0x00681CA4, " Data Driver Slice Enable [11:8]          " ,1,0x4)
        ,REG_INFO_MAC(0x00681CA8, " Common Mode Termination Enable           " ,1,0x4)
        ,REG_INFO_MAC(0x00681CAC, " Equal Eye Common Mode Termination Enable " ,1,0x4)
        ,REG_INFO_MAC(0x00681CB0, " SST Driver Enable                        " ,1,0x4)
        ,REG_INFO_MAC(0x00681CD0, " SST Driver Enable Obs                    " ,1,0x4)
        ,REG_INFO_MAC(0x00681CE0, " Loopback Data Driver Slice Enable [7:0]  " ,1,0x4)
        ,REG_INFO_MAC(0x00681CE4, " Loopback Data Driver Slice Enable [11:8] " ,1,0x4)
        ,REG_INFO_MAC(0x00681CE8, " Loopback SST Driver Enable               " ,1,0x4)
        ,REG_INFO_MAC(0x00681CF0, " odctrl                                   " ,1,0x4)
        ,REG_INFO_MAC(0x00681D00, " Spare Observes                           " ,1,0x4)
        ,REG_INFO_MAC(0x00681D04, " Spare Controls                           " ,1,0x4)

        /* must be last */
        ,LAST_REG_INFO_MAC
    };

/* next are not in Cider of eagle ! */
static REG_INFO falcon_skip_not_in_eagle_d2d_gw_reg_info[] = {
         REG_INFO_MAC(0x00686000, " SERDES MAC TX Config %n                " ,8,0x80)
        ,REG_INFO_MAC(0x00686020, " SERDES MAC TX Error Cause %n           " ,8,0x80)
        ,REG_INFO_MAC(0x00686024, " SERDES MAC TX Error Mask %n            " ,8,0x80)
        ,REG_INFO_MAC(0x00686040, " SERDES MAC RX Config %n                " ,8,0x80)
        ,REG_INFO_MAC(0x00686060, " SERDES MAC RX Error Cause %n           " ,8,0x80)
        ,REG_INFO_MAC(0x00686064, " SERDES MAC RX Error Mask %n            " ,8,0x80)
        /* must be last */
        ,LAST_REG_INFO_MAC
    };

/* using SMI ! compare raven1,D2D1 with raven2,D2D2 */
GT_STATUS prvCpssDxChRavenSmiD2DRegsComparePrint
(
    IN  GT_U8                devNum,
    IN  GT_U32               ravenIndex_1,/* 0 , 1, 2, 3, 7, 6, 5, 4,
                                           8 , 9,10,11,15,14,13,12 */
    IN  GT_U32               d2dIndex_1,  /* 0 , 1 */
    IN  GT_U32               ravenIndex_2,/* 0 , 1, 2, 3, 7, 6, 5, 4,
                                           8 , 9,10,11,15,14,13,12 */
    IN  GT_U32               d2dIndex_2   /* 0 , 1 */
)
{
    REG_INFO *regInfoPtr;
    GT_STATUS rc;
    GT_U32 regAddr_1,regAddr_2,ii,regValue1,regValue2,baseAddr_1,baseAddr_2,maxRavenId;

    devNum = devNum;

    if(!PRV_CPSS_DXCH_PP_MAC(devNum))
    {
        return GT_BAD_PARAM;
    }
    maxRavenId = 4*PRV_CPSS_DXCH_PP_MAC(devNum)->genInfo.multiPipe.numOfTiles;

    regInfoPtr  = &falcon_d2d_gw_reg_info[0];

    if(d2dIndex_1 > 1 || d2dIndex_2 > 1 || ravenIndex_1 >= maxRavenId || ravenIndex_2 >= maxRavenId)
    {
        return GT_BAD_PARAM;
    }


    baseAddr_1 = 0x00680000 + d2dIndex_1 * 0x00010000;
    baseAddr_2 = 0x00680000 + d2dIndex_2 * 0x00010000;
    {
        GT_U32  tileId_1 = ravenIndex_1 / 4;
        GT_U32  flaconAddr_1 =  tileId_1 * 0x20000000 +  (ravenIndex_1 % 4) * 0x01000000;
        GT_U32  tileId_2 = ravenIndex_2 / 4;
        GT_U32  flaconAddr_2 =  tileId_2 * 0x20000000 +  (ravenIndex_2 % 4) * 0x01000000;

        baseAddr_1 += flaconAddr_1;
        baseAddr_2 += flaconAddr_2;
    }

    PR("compare R[%d] D2D[%d] to R[%d] D2D[%d] \n",
        ravenIndex_1,d2dIndex_1,
        ravenIndex_2,d2dIndex_2);

    for(/*no init*/ ; regInfoPtr->regName ; regInfoPtr++)
    {
        for(ii = 0 ; ii < regInfoPtr->numRegs ; ii++)
        {
            regAddr_1 = baseAddr_1 + (regInfoPtr->regOffset & 0xFFFF) +
                ii * regInfoPtr->offsetFormula;

            rc = SMI_READ(devNum,ravenIndex_1, regAddr_1, &regValue1);
            if (rc != GT_OK)
            {
                PR("regRead failed, regAddr %08X, rc %d\n", regAddr_1, rc);
            }

            regAddr_2 = baseAddr_2 + (regInfoPtr->regOffset & 0xFFFF) +
                ii * regInfoPtr->offsetFormula;

            rc = SMI_READ(devNum,ravenIndex_2 , regAddr_2, &regValue2);
            if (rc != GT_OK)
            {
                PR("regRead failed, regAddr %08X, rc %d\n", regAddr_2, rc);
            }

            if(regValue1 != regValue2)
            {
                /* print only registers with value1 != value2 !!!! */
                PR("[0x%8.8x] with value [0x%8.8x] compared to [0x%8.8x] with value [0x%8.8x] 'XOR'[0x%8.8x] [%s] [%d] \n",
                    regAddr_1,regValue1,
                    regAddr_2,regValue2,
                    regValue1^regValue2,
                    regInfoPtr->regName,ii);

            }
        }
    }

    return GT_OK;
}

/* compare the eagle side for the attached raven1,D2D1 with raven2,D2D2 */
GT_STATUS prvCpssDxChEagleD2DRegsComparePrint
(
    IN  GT_U8                devNum,
    IN  GT_U32               ravenIndex_1,/* 0 , 1, 2, 3, 7, 6, 5, 4,
                                           8 , 9,10,11,15,14,13,12 */
    IN  GT_U32               d2dIndex_1,  /* 0 , 1 */
    IN  GT_U32               ravenIndex_2,/* 0 , 1, 2, 3, 7, 6, 5, 4,
                                           8 , 9,10,11,15,14,13,12 */
    IN  GT_U32               d2dIndex_2   /* 0 , 1 */
)
{
    REG_INFO *regInfoPtr,*filterregInfoPtr;
    GT_STATUS rc;
    GT_U32 regAddr_1,regAddr_2,ii,regValue1,regValue2,baseAddr_1,baseAddr_2,maxRavenId;
    GT_U32  tileId_1,tileId_2;
    GT_U32  flaconAddr_1,flaconAddr_2;
    GT_U32  localPipeId_1,localPipeId_2;
    GT_U32  pipeAddr_1,pipeAddr_2;
    GT_U32  skip;

    if(!PRV_CPSS_DXCH_PP_MAC(devNum))
    {
        return GT_BAD_PARAM;
    }
    maxRavenId = 4*PRV_CPSS_DXCH_PP_MAC(devNum)->genInfo.multiPipe.numOfTiles;

    regInfoPtr  = &falcon_d2d_gw_reg_info[0];
    filterregInfoPtr = &falcon_skip_not_in_eagle_d2d_gw_reg_info[0];

    if(d2dIndex_1 > 1 || d2dIndex_2 > 1 || ravenIndex_1 >= maxRavenId || ravenIndex_2 >= maxRavenId)
    {
        return GT_BAD_PARAM;
    }

    tileId_1 = ravenIndex_1 / 4;
    tileId_2 = ravenIndex_2 / 4;

    flaconAddr_1 = tileId_1 * 0x20000000;
    flaconAddr_2 = tileId_2 * 0x20000000;

    localPipeId_1 = (ravenIndex_1 % 4) / 2;
    localPipeId_2 = (ravenIndex_2 % 4) / 2;

    pipeAddr_1 = localPipeId_1 * 0x08000000;
    pipeAddr_2 = localPipeId_2 * 0x08000000;

    flaconAddr_1 += pipeAddr_1;
    flaconAddr_2 += pipeAddr_2;

    /* print before the swap of d2dIndex_1 , d2dIndex_2 */
    PR("compare on EAGLE the D2D that connected to :(T[%d]P[%d]) R[%d] D2D[%d] to (T[%d]P[%d]) R[%d] D2D[%d] \n",
        tileId_1,localPipeId_1,
        ravenIndex_1,d2dIndex_1,
        tileId_2,localPipeId_2,
        ravenIndex_2,d2dIndex_2);


    if(tileId_1 & 1)
    {
        /* d2d between eagle and Raven are swapped */
        d2dIndex_1 = 1 - d2dIndex_1;
    }

    if(tileId_2 & 1)
    {
        /* d2d between eagle and Raven are swapped */
        d2dIndex_2 = 1 - d2dIndex_2;
    }

    baseAddr_1 = ((ravenIndex_1 & 1) ? 0x0D810000 : 0x0D7F0000) + d2dIndex_1 * 0x00010000 + flaconAddr_1;
    baseAddr_2 = ((ravenIndex_2 & 1) ? 0x0D810000 : 0x0D7F0000) + d2dIndex_2 * 0x00010000 + flaconAddr_2;

    for(/*no init*/ ; regInfoPtr->regName ; regInfoPtr++)
    {
        skip = 0;
        for(ii = 0 ; filterregInfoPtr[ii].regName ; ii++)
        {
            if(regInfoPtr->regOffset == filterregInfoPtr[ii].regOffset)
            {
                skip = 1;
                break;
            }
        }
        if(skip)
        {
            continue;
        }

        for(ii = 0 ; ii < regInfoPtr->numRegs ; ii++)
        {
            regAddr_1 = baseAddr_1 + (regInfoPtr->regOffset & 0xFFFF) +
                ii * regInfoPtr->offsetFormula;

            rc = prvCpssDrvHwPpPortGroupReadRegister(devNum,0/*groupId*/,regAddr_1, &regValue1);
            if (rc != GT_OK)
            {
                PR("regRead failed, regAddr %08X, rc %d\n", regAddr_1, rc);
            }

            regAddr_2 = baseAddr_2 + (regInfoPtr->regOffset & 0xFFFF) +
                ii * regInfoPtr->offsetFormula;

            rc = prvCpssDrvHwPpPortGroupReadRegister(devNum,0/*groupId*/, regAddr_2, &regValue2);
            if (rc != GT_OK)
            {
                PR("regRead failed, regAddr %08X, rc %d\n", regAddr_2, rc);
            }

            if(regValue1 != regValue2)
            {
                /* print only registers with value1 != value2 !!!! */
                PR("[0x%8.8x] with value [0x%8.8x] compared to [0x%8.8x] with value [0x%8.8x] 'XOR'[0x%8.8x] [%s] [%d] \n",
                    regAddr_1,regValue1,
                    regAddr_2,regValue2,
                    regValue1^regValue2,
                    regInfoPtr->regName,ii);

            }
        }
    }

    return GT_OK;
}


static REG_INFO short_falcon_d2d_gw_reg_info[] = {

         REG_INFO_MAC(0x00684604, " MAC RX TDM Schedule %n                 " ,160/4,0x4)
        ,REG_INFO_MAC(0x006847E4, " MAC RX FIFO Bandwidth Select %n        "  ,3,0x4)
        ,REG_INFO_MAC(0x00684804, " MAC RX Channel %n                      " ,18,0x4)
        ,REG_INFO_MAC(0x00684904, " MAC RX FIFO Segment Pointer %n         " ,34,0x4)
        ,REG_INFO_MAC(0x00684A04, " MAC RX Stat Good Packets Low %n        " ,17,0x8)
        ,REG_INFO_MAC(0x00684A08, " MAC RX Stat Good Packets High %n       " ,17,0x8)
        ,REG_INFO_MAC(0x00684C04, " MAC RX Stat Bad Packets Low %n         " ,17,0x8)
        ,REG_INFO_MAC(0x00684C08, " MAC RX Stat Bad Packets High %n        " ,17,0x8)
        ,REG_INFO_MAC(0x00684E04, " MAC RX Channel 2 %n                    " ,18,0x4)
        ,REG_INFO_MAC(0x00684FFC, " MAC Debug Frame Enable                 " ,1,0x4)


        ,REG_INFO_MAC(0x00684000, "  MAC Revision ID                       " ,1,0x4)
        ,REG_INFO_MAC(0x00684004, "  MAC TX Channel %n                     " ,18,0x4)
        ,REG_INFO_MAC(0x00684104, "  MAC TX FIFO Segment Pointer %seg      " ,32,0x4)
        ,REG_INFO_MAC(0x00684204, "  MAC TX Stat Good Packets Low %n       " ,17,0x8)
        ,REG_INFO_MAC(0x00684208, "  MAC TX Stat Good Packets High %n      " ,17,0x8)
        ,REG_INFO_MAC(0x00684400, "  MAC Error TX Bad Address              " ,1,0x4)
        ,REG_INFO_MAC(0x00684404, "  MAC TX Stat Bad Packets Low %n        " ,17,0x8)
        ,REG_INFO_MAC(0x00684408, "  MAC TX Stat Bad Packets High %n       " ,17,0x8)
        ,REG_INFO_MAC(0x006845FC, "  MAC TX Error Mask                     " ,1,0x4)
        ,REG_INFO_MAC(0x00684600, "  MAC TX Error Cause                    " ,1,0x4)

/* not exists in Cider Eagle (and debug on Raven)
        ,REG_INFO_MAC(0x00685C00, "   DBG_FRM_CHECK_CFG_%n                 " ,17,0x4)
        ,REG_INFO_MAC(0x00685CFC, "   MAC Debug Check Error Bad Address    " ,1,0x4)
        ,REG_INFO_MAC(0x00685D00, "   DBG_FRM_CHECK_SEQ_%n                 " ,17,0x4)
        ,REG_INFO_MAC(0x00685DFC, "   MAC Debug Check Error Cause          " ,1,0x4)
        ,REG_INFO_MAC(0x00685E00, "   DBG_FRM_CNT_GOOD_%n                  " ,17,0x4)
        ,REG_INFO_MAC(0x00685EFC, "   MAC Debug Check Error Mask           " ,1,0x4)
        ,REG_INFO_MAC(0x00685F00, "   DBG_FRM_CNT_BAD_%n                   " ,17,0x4)

        ,REG_INFO_MAC(0x00685000, " DBG_FRM_GEN_CFG_%n                     " ,17,0x4)
        ,REG_INFO_MAC(0x006850FC, " MAC Debug Gen Error Bad Address        " ,1,0x4)
        ,REG_INFO_MAC(0x00685100, " DBG_FRM_GEN_CFG_STAT_%n                " ,17,0x8)
        ,REG_INFO_MAC(0x006851FC, " MAC Debug Gen Error Cause              " ,1,0x4)
        ,REG_INFO_MAC(0x00685200, " DBG_FRM_TBL_%n                         " ,32,0x4)
        ,REG_INFO_MAC(0x006852FC, " MAC Debug Gen Error Mask               " ,1,0x4)
        ,REG_INFO_MAC(0x00685300, " DBG_FRM_TBL_LEN_%n                     " ,32,0x4)
        ,REG_INFO_MAC(0x00685400, " DBG_FRM_TBL_HEADER_%i_%n               " ,28*32,0x4)
        ,REG_INFO_MAC(0x00685B00, " DBG_FRM_TBL_PAYLOAD_%n                 " ,32,0x4)
*/
        ,REG_INFO_MAC(0x00686038, " SERDES MAC Error Mask                  " ,1,0x4)
        ,REG_INFO_MAC(0x0068603C, " SERDES MAC Error Cause                 " ,1,0x4)
        ,REG_INFO_MAC(0x0068607C, " SERDES MAC Bad Address                 " ,1,0x4)
        ,REG_INFO_MAC(0x00686000, " SERDES MAC TX Config %n                " ,8,0x80)
        ,REG_INFO_MAC(0x00686020, " SERDES MAC TX Error Cause %n           " ,8,0x80)
        ,REG_INFO_MAC(0x00686024, " SERDES MAC TX Error Mask %n            " ,8,0x80)
        ,REG_INFO_MAC(0x00686040, " SERDES MAC RX Config %n                " ,8,0x80)
        ,REG_INFO_MAC(0x00686060, " SERDES MAC RX Error Cause %n           " ,8,0x80)
        ,REG_INFO_MAC(0x00686064, " SERDES MAC RX Error Mask %n            " ,8,0x80)

        ,REG_INFO_MAC(0x00687000, " PCS Global Control                     " ,1,0x4)
        ,REG_INFO_MAC(0x00687004, " PCS Transmit Calendar Control          " ,1,0x4)
        ,REG_INFO_MAC(0x00687008, " PCS Receive Calendar Control           " ,1,0x4)
        ,REG_INFO_MAC(0x0068700C, " PCS Transmit Rate Control              " ,1,0x4)
        ,REG_INFO_MAC(0x00687010, " PCS Transmit Statistic Low part        " ,1,0x4)
        ,REG_INFO_MAC(0x00687014, " PCS Transmit Statistic High part       " ,1,0x4)
        ,REG_INFO_MAC(0x00687018, " PCS Receive Statistic Low part         " ,1,0x4)
        ,REG_INFO_MAC(0x0068701C, " PCS Receive Statistic High part        " ,1,0x4)
        ,REG_INFO_MAC(0x00687020, " PCS Alignment Marker BER Count for Each Lane %n" ,4,0x4)
        ,REG_INFO_MAC(0x00687040, " PCS Event Cause                        " ,1,0x4)
        ,REG_INFO_MAC(0x00687044, " PCS Event Mask                         " ,1,0x4)
        ,REG_INFO_MAC(0x00687048, " PCS Transmit Internal Flowcontrol Mask   " ,1,0x4)
        ,REG_INFO_MAC(0x0068704C, " PCS Transmit Internal Flowcontrol Force  " ,1,0x4)
        ,REG_INFO_MAC(0x00687050, " PCS Transmit Internal Flowcontrol Status      " ,1,0x4)
        ,REG_INFO_MAC(0x00687054, " PCS Transmit Internal Flowcontrol Change Event" ,1,0x4)
        ,REG_INFO_MAC(0x00687058, " PCS Receive Status                     " ,1,0x4)
        ,REG_INFO_MAC(0x0068705C, " PCS Control CPU Slot Arbitration       " ,1,0x4)
        ,REG_INFO_MAC(0x00687060, " PCS Debug PFC Control                  " ,1,0x4)
        ,REG_INFO_MAC(0x00687064, " PCS Debug PFC Read Value 1             " ,1,0x4)
        ,REG_INFO_MAC(0x00687068, " PCS Debug PFC Read Value 0             " ,1,0x4)
        ,REG_INFO_MAC(0x0068706C, " PCS Debug PFC Write Value 1            " ,1,0x4)
        ,REG_INFO_MAC(0x00687070, " PCS Debug PFC Write Value 0            " ,1,0x4)
        ,REG_INFO_MAC(0x00687074, " PCS Debug error inject                 " ,1,0x4)
        ,REG_INFO_MAC(0x00687078, " PCS Metal Fix                          " ,1,0x4)
        ,REG_INFO_MAC(0x0068707C, " PCS MAC mode selector                  " ,1,0x4)
        ,REG_INFO_MAC(0x00687080, " PCS Register Access Status             " ,1,0x4)
        ,REG_INFO_MAC(0x00687084, " PCS Out Of Reset                       " ,1,0x4)
        ,REG_INFO_MAC(0x00687100, " PCS Debug Lane Control for lane %n         " ,4,0x4)
        ,REG_INFO_MAC(0x00687200, " PCS Transmit Calendar Slot Configuration %n" ,32,0x4)
        ,REG_INFO_MAC(0x00687400, " PCS Receive Calendar Slot Configuration %n " ,32,0x4)
        ,REG_INFO_MAC(0x00687600, " PCS Internal Flow Control Remapping_%ch    " ,32,0x4)

        ,REG_INFO_MAC(0x00688004, " PMA Reset Control Per Lane                     " ,1,0x4)
        ,REG_INFO_MAC(0x00688008, " PMA Operation Control                          " ,1,0x4)
        ,REG_INFO_MAC(0x00688010, " PMA Debug Boundary Scan Bypass Overrule Control" ,1,0x4)
        ,REG_INFO_MAC(0x00688014, " PMA Debug Boundary Scan Bypass Receive data    " ,1,0x4)
        ,REG_INFO_MAC(0x00688018, " PMA Debug Boundary Scan Bypass Transmit data   " ,1,0x4)
        ,REG_INFO_MAC(0x0068801C, " PMA Debug Clock Domain Crossing FIFO levels    " ,1,0x4)
        ,REG_INFO_MAC(0x00688020, " PMA Debug Alignment Control %lid               " ,4,0x4)
        ,REG_INFO_MAC(0x00688030, " PMA Sticky Status                              " ,1,0x4)
        ,REG_INFO_MAC(0x00688034, " PMA 66b Alignment Status                       " ,1,0x4)
        ,REG_INFO_MAC(0x00688038, " PMA Debug Clock Domain Crossing Sticky Status  " ,1,0x4)
        ,REG_INFO_MAC(0x0068803C, " PMA Metal Fix                                  " ,1,0x4)
        ,REG_INFO_MAC(0x00688040, " PMA FEC Corrected Error Count for lane%n       " ,4,0x4)
        ,REG_INFO_MAC(0x00688050, " PMA FEC uncorrected err count for lane %n      " ,4,0x4)
        ,REG_INFO_MAC(0x00688060, " PMA Event Cause                                " ,1,0x4)
        ,REG_INFO_MAC(0x00688064, " PMA Event Mask                                 " ,1,0x4)
        ,REG_INFO_MAC(0x00688068, " PMA Register Access Status                     " ,1,0x4)


        ,REG_INFO_MAC(0x00682000, " ID Register                                    " ,1,0x4)
        ,REG_INFO_MAC(0x00682004, " autostart                                      " ,1,0x4)
        ,REG_INFO_MAC(0x00682008, " Autostart error flags                          " ,1,0x4)
        ,REG_INFO_MAC(0x0068200C, " Autostart Calibration Mask                     " ,1,0x4)
        ,REG_INFO_MAC(0x00682010, " Autostart Calibration Status                   " ,1,0x4)
        ,REG_INFO_MAC(0x00682014, " Autostart Rx PLL mask                          " ,1,0x4)
        ,REG_INFO_MAC(0x00682018, " bgcal                                          " ,1,0x4)
        ,REG_INFO_MAC(0x0068201C, " Configuration                                  " ,1,0x4)
        ,REG_INFO_MAC(0x00682040, " Fixed current trim                             " ,1,0x4)
        ,REG_INFO_MAC(0x00682044, " Fixed bias trim                                " ,1,0x4)
        ,REG_INFO_MAC(0x00682048, " iftrimovr                                      " ,1,0x4)
        ,REG_INFO_MAC(0x0068204C, " Fixed bias trim override mode                  " ,1,0x4)
        ,REG_INFO_MAC(0x00682050, " Rx bias configuration                          " ,1,0x4)
        ,REG_INFO_MAC(0x00682054, " bgmode                                         " ,1,0x4)
        ,REG_INFO_MAC(0x00682058, " termovr                                        " ,1,0x4)
        ,REG_INFO_MAC(0x0068205C, " Calibration termination                        " ,1,0x4)
        ,REG_INFO_MAC(0x00682060, " termcalovr                                     " ,1,0x4)
        ,REG_INFO_MAC(0x00682064, " Calibration termination reference current      " ,1,0x4)
        ,REG_INFO_MAC(0x00682068, " Calibration termination common mode voltage    " ,1,0x4)
        ,REG_INFO_MAC(0x0068206C, " compinvovr                                     " ,1,0x4)
        ,REG_INFO_MAC(0x00682070, " Calibration Clock Configuration                " ,1,0x4)
        ,REG_INFO_MAC(0x00682074, " Fixed Bias Cal params                          " ,1,0x4)
        ,REG_INFO_MAC(0x00682078, " Termination cal params                         " ,1,0x4)
        ,REG_INFO_MAC(0x0068207C, " rerun                                          " ,1,0x4)
        ,REG_INFO_MAC(0x00682080, " DAC Configuration                              " ,1,0x4)
        ,REG_INFO_MAC(0x006820D0, " Rx PLL charge pump current                     " ,1,0x4)
        ,REG_INFO_MAC(0x006820D4, " Rx PLL loop filter                             " ,1,0x4)
        ,REG_INFO_MAC(0x006820D8, " Refclk detect config                           " ,1,0x4)
        ,REG_INFO_MAC(0x006820DC, " Rx PLL FLA current                             " ,1,0x4)
        ,REG_INFO_MAC(0x006820F0, " PLL reg level                                  " ,1,0x4)
        ,REG_INFO_MAC(0x006820F4, " PLL reg compensation                           " ,1,0x4)
        ,REG_INFO_MAC(0x006820F8, " PLL reg reference                              " ,1,0x4)
        ,REG_INFO_MAC(0x00682100, " Temperature Sensor                             " ,1,0x4)
        ,REG_INFO_MAC(0x00682140, " Debugclk configuration                         " ,1,0x4)
        ,REG_INFO_MAC(0x00682144, " Anabus Configuration                           " ,1,0x4)
        ,REG_INFO_MAC(0x00682148, " Anabus Selection                               " ,1,0x4)
        ,REG_INFO_MAC(0x0068214C, " Ananbus mux (data tile)                        " ,1,0x4)
        ,REG_INFO_MAC(0x00682158, " Rxcon FSM                                      " ,1,0x4)
        ,REG_INFO_MAC(0x00682180, " Power scheduler                                " ,1,0x4)
        ,REG_INFO_MAC(0x00682184, " Power scheduler delay                          " ,1,0x4)
        ,REG_INFO_MAC(0x00682340, " Spare Observes                                 " ,1,0x4)
        ,REG_INFO_MAC(0x00682344, " Spare Controls                                 " ,1,0x4)

#define shortRXIF_RXi(name,offsetFrom0) \
         REG_INFO_MAC(0x00682400+offsetFrom0,name " Auto calibration mask                          " ,1,0x4)      \
        ,REG_INFO_MAC(0x00682404+offsetFrom0,name " Auto calibration status                        " ,1,0x4)      \
        ,REG_INFO_MAC(0x00682408+offsetFrom0,name " Rerun auto-calibration                         " ,1,0x4)      \
        ,REG_INFO_MAC(0x0068240C+offsetFrom0,name " Analysis mode                                  " ,1,0x4)      \
        ,REG_INFO_MAC(0x00682410+offsetFrom0,name " Coarse offset calibration word count           " ,1,0x4)      \
        ,REG_INFO_MAC(0x00682414+offsetFrom0,name " Fine offset calibration word count             " ,1,0x4)      \
        ,REG_INFO_MAC(0x00682418+offsetFrom0,name " Eyecope Configuration Register                 " ,1,0x4)      \
        ,REG_INFO_MAC(0x0068241C+offsetFrom0,name " Eye scope gating time                          " ,1,0x4)      \
        ,REG_INFO_MAC(0x00682420+offsetFrom0,name " Eyescope level                                 " ,1,0x4)      \
        ,REG_INFO_MAC(0x00682424+offsetFrom0,name " Data invert                                    " ,1,0x4)      \
        ,REG_INFO_MAC(0x00682428+offsetFrom0,name " RX Data Calibration Status Flag Register       " ,1,0x4)      \
        ,REG_INFO_MAC(0x0068242C+offsetFrom0,name " Coarse offset threshold                        " ,1,0x4)      \
        ,REG_INFO_MAC(0x00682430+offsetFrom0,name " Fine offset threshold                          " ,1,0x4)      \
        ,REG_INFO_MAC(0x00682434+offsetFrom0,name " Word synchronization threshold                 " ,1,0x4)      \
        ,REG_INFO_MAC(0x00682438+offsetFrom0,name " Word synchronization cycle count               " ,1,0x4)      \
        ,REG_INFO_MAC(0x00682560+offsetFrom0,name " Data Output Mux Select Override Control Registe" ,1,0x4)      \
        ,REG_INFO_MAC(0x006825C4+offsetFrom0,name " OK state                                       " ,1,0x4)      \
        ,REG_INFO_MAC(0x006825C8+offsetFrom0,name " PRBS Pattern Checker Configuration Register    " ,1,0x4)      \
        ,REG_INFO_MAC(0x006825D0+offsetFrom0,name " Open-loop PRBS select                          " ,1,0x4)      \
        ,REG_INFO_MAC(0x006825D4+offsetFrom0,name " Pattern Checker Reset and Snap Control Register" ,1,0x4)      \
        ,REG_INFO_MAC(0x006825DC+offsetFrom0,name " Error status                                   " ,1,0x4)      \
        ,REG_INFO_MAC(0x006825E0+offsetFrom0,name " UI mask [7:0]                                  " ,1,0x4)      \
        ,REG_INFO_MAC(0x006825E4+offsetFrom0,name " UI mask [15:8]                                 " ,1,0x4)      \
        ,REG_INFO_MAC(0x006825E8+offsetFrom0,name " UI mask [23:16]                                " ,1,0x4)      \
        ,REG_INFO_MAC(0x006825EC+offsetFrom0,name " UI mask [31:24]                                " ,1,0x4)      \
        ,REG_INFO_MAC(0x006825F0+offsetFrom0,name " User pattern                                   " ,(0x0068263C-0x006825F0+4)/4,0x4) \
        ,REG_INFO_MAC(0x00682640+offsetFrom0,name " User pattern mode selection                    " ,1,0x4)      \
        ,REG_INFO_MAC(0x00682644+offsetFrom0,name " XOR mode selection                             " ,1,0x4)      \
        ,REG_INFO_MAC(0x00682648+offsetFrom0,name " Duration of initial CDA run                    " ,1,0x4)      \
        ,REG_INFO_MAC(0x0068264C+offsetFrom0,name " CDA sub-channel select                         " ,1,0x4)      \
        ,REG_INFO_MAC(0x00682650+offsetFrom0,name " Kp constant                                    " ,1,0x4)      \
        ,REG_INFO_MAC(0x00682654+offsetFrom0,name " CDA Configuration Register                     " ,1,0x4)      \
        ,REG_INFO_MAC(0x00682658+offsetFrom0,name " CDA Clock Select Override Register             " ,1,0x4)      \
        ,REG_INFO_MAC(0x0068265C+offsetFrom0,name " CDA clock phase select                         " ,1,0x4)      \
        ,REG_INFO_MAC(0x00682660+offsetFrom0,name " CDA Calibration Smoothing                      " ,1,0x4)      \
        ,REG_INFO_MAC(0x00682664+offsetFrom0,name " CDA calibration cycle count                    " ,1,0x4)      \
        ,REG_INFO_MAC(0x00682680+offsetFrom0,name " PI code value                                  " ,1,0x4)      \
        ,REG_INFO_MAC(0x00682684+offsetFrom0,name " PI code override value                         " ,1,0x4)      \
        ,REG_INFO_MAC(0x006826A8+offsetFrom0,name " Data Jog/Slip Configuration Register           " ,1,0x4)      \
        ,REG_INFO_MAC(0x006826C0+offsetFrom0,name " Scheduler Configuration Register               " ,1,0x4)      \
        ,REG_INFO_MAC(0x006826C4+offsetFrom0,name " Scheduler exponential period                   " ,1,0x4)      \
        ,REG_INFO_MAC(0x006826C8+offsetFrom0,name " Scheduler CDA exponential period               " ,1,0x4)      \
        ,REG_INFO_MAC(0x006826CC+offsetFrom0,name " Scheduler background divider                   " ,1,0x4)      \
        ,REG_INFO_MAC(0x006826D0+offsetFrom0,name " Scheduler state                                " ,1,0x4)      \
        ,REG_INFO_MAC(0x00682724+offsetFrom0,name " Eye centering level                            " ,1,0x4)      \
        ,REG_INFO_MAC(0x00682728+offsetFrom0,name " Eye centering max PI                           " ,1,0x4)      \
        ,REG_INFO_MAC(0x0068272C+offsetFrom0,name " Eye centering duration                         " ,1,0x4)      \
        ,REG_INFO_MAC(0x00682730+offsetFrom0,name " Left side PI steps                             " ,1,0x4)      \
        ,REG_INFO_MAC(0x00682734+offsetFrom0,name " Right side PI steps                            " ,1,0x4)      \
        ,REG_INFO_MAC(0x00682738+offsetFrom0,name " Centering threshold                            " ,1,0x4)      \
                                                                                                                  \
        ,REG_INFO_MAC(0x00682740+offsetFrom0,name " CTLE control                                   " ,1,0x4)      \
        ,REG_INFO_MAC(0x00682744+offsetFrom0,name " Rx PLL Control                                 " ,1,0x4)      \
        ,REG_INFO_MAC(0x00682748+offsetFrom0,name " RX Data Configuration Register                 " ,1,0x4)      \
        ,REG_INFO_MAC(0x0068274C+offsetFrom0,name " PLL unlock status                              " ,1,0x4)      \
        ,REG_INFO_MAC(0x00682750+offsetFrom0,name " PLL lock control                               " ,1,0x4)      \
        ,REG_INFO_MAC(0x00682754+offsetFrom0,name " PLL Calibration Control Register               " ,1,0x4)      \
        ,REG_INFO_MAC(0x00682758+offsetFrom0,name " PLL Calibration Status Register                " ,1,0x4)      \
        ,REG_INFO_MAC(0x0068275C+offsetFrom0,name " PLL Regulator Control                          " ,1,0x4)      \
        ,REG_INFO_MAC(0x00682780+offsetFrom0,name " Rx Termination Config                          " ,1,0x4)      \
        ,REG_INFO_MAC(0x00682784+offsetFrom0,name " CTLE Equalisation                              " ,1,0x4)      \
        ,REG_INFO_MAC(0x00682790+offsetFrom0,name " pregupmask                                     " ,1,0x4)      \
        ,REG_INFO_MAC(0x00682794+offsetFrom0,name " plock_ovr                                      " ,1,0x4)      \
        ,REG_INFO_MAC(0x00682798+offsetFrom0,name " RX Duty Cycle Control Register                 " ,1,0x4)      \
        ,REG_INFO_MAC(0x0068279C+offsetFrom0,name " Loopback Termination trimming.                 " ,1,0x4)      \
        ,REG_INFO_MAC(0x006827A0+offsetFrom0,name " PLL cal startup delay                          " ,1,0x4)      \
        ,REG_INFO_MAC(0x006827A4+offsetFrom0,name " PLL cal DAC delay                              " ,1,0x4)      \
        ,REG_INFO_MAC(0x006827A8+offsetFrom0,name " PLL cal FLA delay                              " ,1,0x4)      \
        ,REG_INFO_MAC(0x006827AC+offsetFrom0,name " PLL calibration lock FLA delay                 " ,1,0x4)      \
        ,REG_INFO_MAC(0x006827C0+offsetFrom0,name " Autostart Duration Counter [7:0]               " ,1,0x4)      \
        ,REG_INFO_MAC(0x006827C4+offsetFrom0,name " Autostart Duration Counter [15:8]              " ,1,0x4)      \
        ,REG_INFO_MAC(0x006827C8+offsetFrom0,name " Autostart Duration Counter [23:16]             " ,1,0x4)      \
        ,REG_INFO_MAC(0x006827E0+offsetFrom0,name " Wire 0 deskew                                  " ,1,0x4)      \
        ,REG_INFO_MAC(0x006827E4+offsetFrom0,name " Wire 1 deskew                                  " ,1,0x4)      \
        ,REG_INFO_MAC(0x006827E8+offsetFrom0,name " Wire 2 deskew                                  " ,1,0x4)      \
        ,REG_INFO_MAC(0x006827EC+offsetFrom0,name " Wire 3 deskew                                  " ,1,0x4)      \
        ,REG_INFO_MAC(0x006827F0+offsetFrom0,name " Wire 4 deskew                                  " ,1,0x4)      \
        ,REG_INFO_MAC(0x006827F4+offsetFrom0,name " Wire 5 deskew                                  " ,1,0x4)      \
        ,REG_INFO_MAC(0x006827F8+offsetFrom0,name " Spare Observes                                 " ,1,0x4)      \
        ,REG_INFO_MAC(0x006827FC+offsetFrom0,name " Spare Controls                                 " ,1,0x4)

        ,shortRXIF_RXi("rx0" ,0x000)
        ,shortRXIF_RXi("rx1" ,0x400)
        ,shortRXIF_RXi("rx2" ,0x800)
        ,shortRXIF_RXi("rx3" ,0xc00)

        ,shortRXIF_RXi("rxbr",0x1800)

        ,REG_INFO_MAC(0x00680004, " APB Wait States                                             " ,1,0x4)
        ,REG_INFO_MAC(0x00680008, " Test Override Switches [7:0]                                " ,1,0x4)
        ,REG_INFO_MAC(0x0068000C, " Test Override Switches [9:8]                                " ,1,0x4)
        ,REG_INFO_MAC(0x00680010, " TX Clock Tile Override Control Register                     " ,1,0x4)
        ,REG_INFO_MAC(0x00680014, " TX Clock Tile Configuration Register                        " ,1,0x4)
        ,REG_INFO_MAC(0x00680018, " Clock Driver Slice Enable [7:0]                             " ,1,0x4)
        ,REG_INFO_MAC(0x0068001C, " Clock Driver Slice Enable [8:8]                             " ,1,0x4)
        ,REG_INFO_MAC(0x00680020, " PLL Lock Count 0                                            " ,1,0x4)
        ,REG_INFO_MAC(0x00680024, " PLL Lock Count 1                                            " ,1,0x4)
        ,REG_INFO_MAC(0x00680028, " Clock Common Mode Termination Enable                        " ,1,0x4)
        ,REG_INFO_MAC(0x0068002C, " Clock Equal Eye Common Mode Termination Enable              " ,1,0x4)
        ,REG_INFO_MAC(0x00680038, " Spare Observes                                              " ,1,0x4)
        ,REG_INFO_MAC(0x0068003C, " Spare Controls                                              " ,1,0x4)
        ,REG_INFO_MAC(0x00680040, " Tx PLL Control                                              " ,1,0x4)
        ,REG_INFO_MAC(0x00680044, " PLL Lock Status                                             " ,1,0x4)
        ,REG_INFO_MAC(0x00680048, " Clock Buffer I Swing Amplitude                              " ,1,0x4)
        ,REG_INFO_MAC(0x0068004C, " Clock Buffer Q Swing Amplitude                              " ,1,0x4)
        ,REG_INFO_MAC(0x00680050, " PLL Charge Pump Control                                     " ,1,0x4)
        ,REG_INFO_MAC(0x00680054, " PLL Loop Control                                            " ,1,0x4)
        ,REG_INFO_MAC(0x00680058, " PLL VCO Capacitive Load control                             " ,1,0x4)
        ,REG_INFO_MAC(0x0068005C, " PLL Regulator Control                                       " ,1,0x4)
        ,REG_INFO_MAC(0x00680060, " PLL Feedback Divider [7:0]                                  " ,1,0x4)
        ,REG_INFO_MAC(0x00680064, " PLL Feedback Divider [8:8]                                  " ,1,0x4)
        ,REG_INFO_MAC(0x00680068, " Debug Clock Control                                         " ,1,0x4)
        ,REG_INFO_MAC(0x00680070, " Correction Clock Control 1                                  " ,1,0x4)
        ,REG_INFO_MAC(0x00680074, " Correction Clock Control 2                                  " ,1,0x4)
        ,REG_INFO_MAC(0x00680078, " Correction Clock Control 3                                  " ,1,0x4)
        ,REG_INFO_MAC(0x0068007C, " corrstatus                                                  " ,1,0x4)
        ,REG_INFO_MAC(0x00680080, " Tx Clock Tile Clock Enable                                  " ,1,0x4)
        ,REG_INFO_MAC(0x00680084, " Tx Clock Tile DCD Correction                                " ,1,0x4)
        ,REG_INFO_MAC(0x00680098, " Bias Current Control                                        " ,1,0x4)
        ,REG_INFO_MAC(0x0068009C, " Tx Clock Tile Autozero Comparator Source Select             " ,1,0x4)
        ,REG_INFO_MAC(0x006800A0, " Tx Clock Tile Test Mode                                     " ,1,0x4)
        ,REG_INFO_MAC(0x006800A4, " Tx Clock Tile Phase Interpolator Bias Current Trim          " ,1,0x4)
        ,REG_INFO_MAC(0x006800A8, " Tx Clock Tile Production Test Phase Interpolator            " ,1,0x4)
        ,REG_INFO_MAC(0x006800AC, " Tx Clock Tile Amplitude Detector Control                    " ,1,0x4)
        ,REG_INFO_MAC(0x006800B4, " Invert sense of slc thresholds                              " ,1,0x4)
        ,REG_INFO_MAC(0x006800B8, " Slice Control Threshold 0                                   " ,1,0x4)
        ,REG_INFO_MAC(0x006800BC, " Slice Control Threshold 1                                   " ,1,0x4)
        ,REG_INFO_MAC(0x006800C0, " Analog Mux Select                                           " ,1,0x4)
        ,REG_INFO_MAC(0x006800C4, " Reference Clock Bias Current Trim                           " ,1,0x4)
        ,REG_INFO_MAC(0x006800C8, " Bias Control 1                                              " ,1,0x4)
        ,REG_INFO_MAC(0x006800CC, " Bias Control 2                                              " ,1,0x4)
        ,REG_INFO_MAC(0x006800D0, " Bias Control 3                                              " ,1,0x4)
        ,REG_INFO_MAC(0x006800D8, " Bias Control 4                                              " ,1,0x4)
        ,REG_INFO_MAC(0x006800DC, " Bias Control 5                                              " ,1,0x4)
        ,REG_INFO_MAC(0x00680100, " Tx Data Tile 0 Clock Enable                                 " ,1,0x4)
        ,REG_INFO_MAC(0x00680104, " Tx Data Tile 0 IQ Skew Control                              " ,1,0x4)
        ,REG_INFO_MAC(0x00680108, " Tx Data Tile 0 Ip DCD Correction                            " ,1,0x4)
        ,REG_INFO_MAC(0x00680110, " Tx Data Tile 0 Qp DCD Correction                            " ,1,0x4)
        ,REG_INFO_MAC(0x00680118, " Data Tile 0 Bias Control                                    " ,1,0x4)
        ,REG_INFO_MAC(0x0068011C, " Tx Data Tile 0 Autozero Comparator Source Select            " ,1,0x4)
        ,REG_INFO_MAC(0x00680140, " Tx Data Tile 0 Clock Enable                                 " ,1,0x4)
        ,REG_INFO_MAC(0x00680144, " Tx Data Tile 0 IQ Skew Control                              " ,1,0x4)
        ,REG_INFO_MAC(0x00680148, " Tx Data Tile 0 Ip DCD Correction                            " ,1,0x4)
        ,REG_INFO_MAC(0x00680150, " Tx Data Tile 0 Qp DCD Correction                            " ,1,0x4)
        ,REG_INFO_MAC(0x00680158, " Data Tile 0 Bias Control                                    " ,1,0x4)
        ,REG_INFO_MAC(0x0068015C, " Tx Data Tile 0 Autozero Comparator Source Select            " ,1,0x4)
        ,REG_INFO_MAC(0x00680180, " Tx Data Tile 0 Clock Enable                                 " ,1,0x4)
        ,REG_INFO_MAC(0x00680184, " Tx Data Tile 0 IQ Skew Control                              " ,1,0x4)
        ,REG_INFO_MAC(0x00680188, " Tx Data Tile 0 Ip DCD Correction                            " ,1,0x4)
        ,REG_INFO_MAC(0x00680190, " Tx Data Tile 0 Qp DCD Correction                            " ,1,0x4)
        ,REG_INFO_MAC(0x00680198, " Data Tile 0 Bias Control                                    " ,1,0x4)
        ,REG_INFO_MAC(0x0068019C, " Tx Data Tile 0 Autozero Comparator Source Select            " ,1,0x4)
        ,REG_INFO_MAC(0x006801C0, " Tx Data Tile 0 Clock Enable                                 " ,1,0x4)
        ,REG_INFO_MAC(0x006801C4, " Tx Data Tile 0 IQ Skew Control                              " ,1,0x4)
        ,REG_INFO_MAC(0x006801C8, " Tx Data Tile 0 Ip DCD Correction                            " ,1,0x4)
        ,REG_INFO_MAC(0x006801D0, " Tx Data Tile 0 Qp DCD Correction                            " ,1,0x4)
        ,REG_INFO_MAC(0x006801D8, " Data Tile 0 Bias Control                                    " ,1,0x4)
        ,REG_INFO_MAC(0x006801DC, " Tx Data Tile 0 Autozero Comparator Source Select            " ,1,0x4)
        ,REG_INFO_MAC(0x006801E4, " SST Driver Enable Obs                                       " ,1,0x4)

        ,REG_INFO_MAC(0x00680440, " Configuration                             " ,1, 0x4)
        ,REG_INFO_MAC(0x00680444, " User Test Pattern Selection               " ,1, 0x4)
        ,REG_INFO_MAC(0x00680448, " Pattern select 1                          " ,1, 0x4)
        ,REG_INFO_MAC(0x00680450, " User Test Pattern 0 [7:0]                 " ,1, 0x4)
        ,REG_INFO_MAC(0x00680454, " User Test Pattern 0 [15:8]                " ,1, 0x4)
        ,REG_INFO_MAC(0x00680458, " User Test Pattern 0 [23:16]               " ,1, 0x4)
        ,REG_INFO_MAC(0x0068045C, " User Test Pattern 0 [31:24]               " ,1, 0x4)
        ,REG_INFO_MAC(0x00680460, " User Test Pattern 1 [7:0]                 " ,1, 0x4)
        ,REG_INFO_MAC(0x00680464, " User Test Pattern 1 [15:8]                " ,1, 0x4)
        ,REG_INFO_MAC(0x00680468, " User Test Pattern 1 [23:16]               " ,1, 0x4)
        ,REG_INFO_MAC(0x0068046C, " User Test Pattern 1 [31:24]               " ,1, 0x4)
        ,REG_INFO_MAC(0x00680470, " User Test Pattern 2 [7:0]                 " ,1, 0x4)
        ,REG_INFO_MAC(0x00680474, " User Test Pattern 2 [15:8]                " ,1, 0x4)
        ,REG_INFO_MAC(0x00680478, " User Test Pattern 2 [23:16]               " ,1, 0x4)
        ,REG_INFO_MAC(0x0068047C, " User Test Pattern 2 [31:24]               " ,1, 0x4)
        ,REG_INFO_MAC(0x00680480, " User Test Pattern 3 [7:0]                 " ,1, 0x4)
        ,REG_INFO_MAC(0x00680484, " User Test Pattern 3 [15:8]                " ,1, 0x4)
        ,REG_INFO_MAC(0x00680488, " User Test Pattern 3 [23:16]               " ,1, 0x4)
        ,REG_INFO_MAC(0x0068048C, " User Test Pattern 3 [31:24]               " ,1, 0x4)
        ,REG_INFO_MAC(0x00680490, " User Test Pattern 4 [7:0]                 " ,1, 0x4)
        ,REG_INFO_MAC(0x00680494, " User Test Pattern 4 [15:8]                " ,1, 0x4)
        ,REG_INFO_MAC(0x00680498, " User Test Pattern 4 [23:16]               " ,1, 0x4)
        ,REG_INFO_MAC(0x0068049C, " User Test Pattern 4 [31:24]               " ,1, 0x4)
        ,REG_INFO_MAC(0x006804A0, " Data Driver Slice Enable [7:0]            " ,1, 0x4)
        ,REG_INFO_MAC(0x006804A4, " Data Driver Slice Enable [11:8]           " ,1, 0x4)
        ,REG_INFO_MAC(0x006804A8, " Common Mode Termination Enable            " ,1, 0x4)
        ,REG_INFO_MAC(0x006804AC, " Equal Eye Common Mode Termination Enable  " ,1, 0x4)
        ,REG_INFO_MAC(0x006804B0, " SST Driver Enable                         " ,1, 0x4)
        ,REG_INFO_MAC(0x006804D0, " SST Driver Enable Obs                     " ,1, 0x4)
        ,REG_INFO_MAC(0x006804E0, " Loopback Data Driver Slice Enable [7:0]   " ,1, 0x4)
        ,REG_INFO_MAC(0x006804E4, " Loopback Data Driver Slice Enable [11:8]  " ,1, 0x4)
        ,REG_INFO_MAC(0x006804E8, " Loopback SST Driver Enable                " ,1, 0x4)
        ,REG_INFO_MAC(0x006804F0, " odctrl                                    " ,1, 0x4)
        ,REG_INFO_MAC(0x00680500, " Spare Observes                            " ,1, 0x4)
        ,REG_INFO_MAC(0x00680504, " Spare Controls                            " ,1, 0x4)

        ,REG_INFO_MAC(0x00680840, " Configuration                             " ,1, 0x4)
        ,REG_INFO_MAC(0x00680844, " User Test Pattern Selection               " ,1, 0x4)
        ,REG_INFO_MAC(0x00680848, " Pattern select 1                          " ,1, 0x4)
        ,REG_INFO_MAC(0x00680850, " User Test Pattern 0 [7:0]                 " ,1, 0x4)
       ,REG_INFO_MAC(0x00680854, " User Test Pattern 0 [15:8]                " ,1, 0x4)
        ,REG_INFO_MAC(0x00680858, " User Test Pattern 0 [23:16]               " ,1, 0x4)
        ,REG_INFO_MAC(0x0068085C, " User Test Pattern 0 [31:24]               " ,1, 0x4)
        ,REG_INFO_MAC(0x00680860, " User Test Pattern 1 [7:0]                 " ,1, 0x4)
        ,REG_INFO_MAC(0x00680864, " User Test Pattern 1 [15:8]                " ,1, 0x4)
        ,REG_INFO_MAC(0x00680868, " User Test Pattern 1 [23:16]               " ,1, 0x4)
        ,REG_INFO_MAC(0x0068086C, " User Test Pattern 1 [31:24]               " ,1, 0x4)
        ,REG_INFO_MAC(0x00680870, " User Test Pattern 2 [7:0]                 " ,1, 0x4)
        ,REG_INFO_MAC(0x00680874, " User Test Pattern 2 [15:8]                " ,1, 0x4)
        ,REG_INFO_MAC(0x00680878, " User Test Pattern 2 [23:16]               " ,1, 0x4)
        ,REG_INFO_MAC(0x0068087C, " User Test Pattern 2 [31:24]               " ,1, 0x4)
        ,REG_INFO_MAC(0x00680880, " User Test Pattern 3 [7:0]                 " ,1, 0x4)
        ,REG_INFO_MAC(0x00680884, " User Test Pattern 3 [15:8]                " ,1, 0x4)
        ,REG_INFO_MAC(0x00680888, " User Test Pattern 3 [23:16]               " ,1, 0x4)
        ,REG_INFO_MAC(0x0068088C, " User Test Pattern 3 [31:24]               " ,1, 0x4)
        ,REG_INFO_MAC(0x00680890, " User Test Pattern 4 [7:0]                 " ,1, 0x4)
        ,REG_INFO_MAC(0x00680894, " User Test Pattern 4 [15:8]                " ,1, 0x4)
        ,REG_INFO_MAC(0x00680898, " User Test Pattern 4 [23:16]               " ,1, 0x4)
        ,REG_INFO_MAC(0x0068089C, " User Test Pattern 4 [31:24]               " ,1, 0x4)
        ,REG_INFO_MAC(0x006808A0, " Data Driver Slice Enable [7:0]            " ,1, 0x4)
        ,REG_INFO_MAC(0x006808A4, " Data Driver Slice Enable [11:8]           " ,1, 0x4)
        ,REG_INFO_MAC(0x006808A8, " Common Mode Termination Enable            " ,1, 0x4)
        ,REG_INFO_MAC(0x006808AC, " Equal Eye Common Mode Termination Enable  " ,1, 0x4)
        ,REG_INFO_MAC(0x006808B0, " SST Driver Enable                         " ,1, 0x4)
        ,REG_INFO_MAC(0x006808D0, " SST Driver Enable Obs                     " ,1, 0x4)
        ,REG_INFO_MAC(0x006808E0, " Loopback Data Driver Slice Enable [7:0]   " ,1, 0x4)
        ,REG_INFO_MAC(0x006808E4, " Loopback Data Driver Slice Enable [11:8]  " ,1, 0x4)
        ,REG_INFO_MAC(0x006808E8, " Loopback SST Driver Enable                " ,1, 0x4)
        ,REG_INFO_MAC(0x006808F0, " odctrl                                    " ,1, 0x4)
        ,REG_INFO_MAC(0x00680900, " Spare Observes                            " ,1, 0x4)
        ,REG_INFO_MAC(0x00680904, " Spare Controls                            " ,1, 0x4)

        ,REG_INFO_MAC(0x00680C40, " Configuration                            " ,1,0x4)
        ,REG_INFO_MAC(0x00680C44, " User Test Pattern Selection              " ,1,0x4)
        ,REG_INFO_MAC(0x00680C48, " Pattern select 1                         " ,1,0x4)
        ,REG_INFO_MAC(0x00680C50, " User Test Pattern 0 [7:0]                " ,1,0x4)
        ,REG_INFO_MAC(0x00680C54, " User Test Pattern 0 [15:8]               " ,1,0x4)
        ,REG_INFO_MAC(0x00680C58, " User Test Pattern 0 [23:16]              " ,1,0x4)
        ,REG_INFO_MAC(0x00680C5C, " User Test Pattern 0 [31:24]              " ,1,0x4)
        ,REG_INFO_MAC(0x00680C60, " User Test Pattern 1 [7:0]                " ,1,0x4)
        ,REG_INFO_MAC(0x00680C64, " User Test Pattern 1 [15:8]               " ,1,0x4)
        ,REG_INFO_MAC(0x00680C68, " User Test Pattern 1 [23:16]              " ,1,0x4)
        ,REG_INFO_MAC(0x00680C6C, " User Test Pattern 1 [31:24]              " ,1,0x4)
        ,REG_INFO_MAC(0x00680C70, " User Test Pattern 2 [7:0]                " ,1,0x4)
        ,REG_INFO_MAC(0x00680C74, " User Test Pattern 2 [15:8]               " ,1,0x4)
        ,REG_INFO_MAC(0x00680C78, " User Test Pattern 2 [23:16]              " ,1,0x4)
        ,REG_INFO_MAC(0x00680C7C, " User Test Pattern 2 [31:24]              " ,1,0x4)
        ,REG_INFO_MAC(0x00680C80, " User Test Pattern 3 [7:0]                " ,1,0x4)
        ,REG_INFO_MAC(0x00680C84, " User Test Pattern 3 [15:8]               " ,1,0x4)
        ,REG_INFO_MAC(0x00680C88, " User Test Pattern 3 [23:16]              " ,1,0x4)
        ,REG_INFO_MAC(0x00680C8C, " User Test Pattern 3 [31:24]              " ,1,0x4)
        ,REG_INFO_MAC(0x00680C90, " User Test Pattern 4 [7:0]                " ,1,0x4)
        ,REG_INFO_MAC(0x00680C94, " User Test Pattern 4 [15:8]               " ,1,0x4)
        ,REG_INFO_MAC(0x00680C98, " User Test Pattern 4 [23:16]              " ,1,0x4)
        ,REG_INFO_MAC(0x00680C9C, " User Test Pattern 4 [31:24]              " ,1,0x4)
        ,REG_INFO_MAC(0x00680CA0, " Data Driver Slice Enable [7:0]           " ,1,0x4)
        ,REG_INFO_MAC(0x00680CA4, " Data Driver Slice Enable [11:8]          " ,1,0x4)
        ,REG_INFO_MAC(0x00680CA8, " Common Mode Termination Enable           " ,1,0x4)
        ,REG_INFO_MAC(0x00680CAC, " Equal Eye Common Mode Termination Enable " ,1,0x4)
        ,REG_INFO_MAC(0x00680CB0, " SST Driver Enable                        " ,1,0x4)
        ,REG_INFO_MAC(0x00680CD0, " SST Driver Enable Obs                    " ,1,0x4)
        ,REG_INFO_MAC(0x00680CE0, " Loopback Data Driver Slice Enable [7:0]  " ,1,0x4)
        ,REG_INFO_MAC(0x00680CE4, " Loopback Data Driver Slice Enable [11:8] " ,1,0x4)
        ,REG_INFO_MAC(0x00680CE8, " Loopback SST Driver Enable               " ,1,0x4)
        ,REG_INFO_MAC(0x00680CF0, " odctrl                                   " ,1,0x4)
        ,REG_INFO_MAC(0x00680D00, " Spare Observes                           " ,1,0x4)
        ,REG_INFO_MAC(0x00680D04, " Spare Controls                           " ,1,0x4)

        ,REG_INFO_MAC(0x00681040, " Configuration                            " ,1,0x4)
        ,REG_INFO_MAC(0x00681044, " User Test Pattern Selection              " ,1,0x4)
        ,REG_INFO_MAC(0x00681048, " Pattern select 1                         " ,1,0x4)
        ,REG_INFO_MAC(0x00681050, " User Test Pattern 0 [7:0]                " ,1,0x4)
        ,REG_INFO_MAC(0x00681054, " User Test Pattern 0 [15:8]               " ,1,0x4)
        ,REG_INFO_MAC(0x00681058, " User Test Pattern 0 [23:16]              " ,1,0x4)
        ,REG_INFO_MAC(0x0068105C, " User Test Pattern 0 [31:24]              " ,1,0x4)
        ,REG_INFO_MAC(0x00681060, " User Test Pattern 1 [7:0]                " ,1,0x4)
        ,REG_INFO_MAC(0x00681064, " User Test Pattern 1 [15:8]               " ,1,0x4)
        ,REG_INFO_MAC(0x00681068, " User Test Pattern 1 [23:16]              " ,1,0x4)
        ,REG_INFO_MAC(0x0068106C, " User Test Pattern 1 [31:24]              " ,1,0x4)
        ,REG_INFO_MAC(0x00681070, " User Test Pattern 2 [7:0]                " ,1,0x4)
        ,REG_INFO_MAC(0x00681074, " User Test Pattern 2 [15:8]               " ,1,0x4)
        ,REG_INFO_MAC(0x00681078, " User Test Pattern 2 [23:16]              " ,1,0x4)
        ,REG_INFO_MAC(0x0068107C, " User Test Pattern 2 [31:24]              " ,1,0x4)
        ,REG_INFO_MAC(0x00681080, " User Test Pattern 3 [7:0]                " ,1,0x4)
        ,REG_INFO_MAC(0x00681084, " User Test Pattern 3 [15:8]               " ,1,0x4)
        ,REG_INFO_MAC(0x00681088, " User Test Pattern 3 [23:16]              " ,1,0x4)
        ,REG_INFO_MAC(0x0068108C, " User Test Pattern 3 [31:24]              " ,1,0x4)
        ,REG_INFO_MAC(0x00681090, " User Test Pattern 4 [7:0]                " ,1,0x4)
        ,REG_INFO_MAC(0x00681094, " User Test Pattern 4 [15:8]               " ,1,0x4)
        ,REG_INFO_MAC(0x00681098, " User Test Pattern 4 [23:16]              " ,1,0x4)
        ,REG_INFO_MAC(0x0068109C, " User Test Pattern 4 [31:24]              " ,1,0x4)
        ,REG_INFO_MAC(0x006810A0, " Data Driver Slice Enable [7:0]           " ,1,0x4)
        ,REG_INFO_MAC(0x006810A4, " Data Driver Slice Enable [11:8]          " ,1,0x4)
        ,REG_INFO_MAC(0x006810A8, " Common Mode Termination Enable           " ,1,0x4)
        ,REG_INFO_MAC(0x006810AC, " Equal Eye Common Mode Termination Enable " ,1,0x4)
        ,REG_INFO_MAC(0x006810B0, " SST Driver Enable                        " ,1,0x4)
        ,REG_INFO_MAC(0x006810D0, " SST Driver Enable Obs                    " ,1,0x4)
        ,REG_INFO_MAC(0x006810E0, " Loopback Data Driver Slice Enable [7:0]  " ,1,0x4)
        ,REG_INFO_MAC(0x006810E4, " Loopback Data Driver Slice Enable [11:8] " ,1,0x4)
        ,REG_INFO_MAC(0x006810E8, " Loopback SST Driver Enable               " ,1,0x4)
        ,REG_INFO_MAC(0x006810F0, " odctrl                                   " ,1,0x4)
        ,REG_INFO_MAC(0x00681100, " Spare Observes                           " ,1,0x4)
        ,REG_INFO_MAC(0x00681104, " Spare Controls                           " ,1,0x4)

        ,REG_INFO_MAC(0x00681C40, " Configuration                            " ,1,0x4)
        ,REG_INFO_MAC(0x00681C44, " User Test Pattern Selection              " ,1,0x4)
        ,REG_INFO_MAC(0x00681C48, " Pattern select 1                         " ,1,0x4)
        ,REG_INFO_MAC(0x00681C50, " User Test Pattern 0 [7:0]                " ,1,0x4)
        ,REG_INFO_MAC(0x00681C54, " User Test Pattern 0 [15:8]               " ,1,0x4)
        ,REG_INFO_MAC(0x00681C58, " User Test Pattern 0 [23:16]              " ,1,0x4)
        ,REG_INFO_MAC(0x00681C5C, " User Test Pattern 0 [31:24]              " ,1,0x4)
        ,REG_INFO_MAC(0x00681C60, " User Test Pattern 1 [7:0]                " ,1,0x4)
        ,REG_INFO_MAC(0x00681C64, " User Test Pattern 1 [15:8]               " ,1,0x4)
        ,REG_INFO_MAC(0x00681C68, " User Test Pattern 1 [23:16]              " ,1,0x4)
        ,REG_INFO_MAC(0x00681C6C, " User Test Pattern 1 [31:24]              " ,1,0x4)
        ,REG_INFO_MAC(0x00681C70, " User Test Pattern 2 [7:0]                " ,1,0x4)
        ,REG_INFO_MAC(0x00681C74, " User Test Pattern 2 [15:8]               " ,1,0x4)
        ,REG_INFO_MAC(0x00681C78, " User Test Pattern 2 [23:16]              " ,1,0x4)
        ,REG_INFO_MAC(0x00681C7C, " User Test Pattern 2 [31:24]              " ,1,0x4)
        ,REG_INFO_MAC(0x00681C80, " User Test Pattern 3 [7:0]                " ,1,0x4)
        ,REG_INFO_MAC(0x00681C84, " User Test Pattern 3 [15:8]               " ,1,0x4)
        ,REG_INFO_MAC(0x00681C88, " User Test Pattern 3 [23:16]              " ,1,0x4)
        ,REG_INFO_MAC(0x00681C8C, " User Test Pattern 3 [31:24]              " ,1,0x4)
        ,REG_INFO_MAC(0x00681C90, " User Test Pattern 4 [7:0]                " ,1,0x4)
        ,REG_INFO_MAC(0x00681C94, " User Test Pattern 4 [15:8]               " ,1,0x4)
        ,REG_INFO_MAC(0x00681C98, " User Test Pattern 4 [23:16]              " ,1,0x4)
        ,REG_INFO_MAC(0x00681C9C, " User Test Pattern 4 [31:24]              " ,1,0x4)
        ,REG_INFO_MAC(0x00681CA0, " Data Driver Slice Enable [7:0]           " ,1,0x4)
        ,REG_INFO_MAC(0x00681CA4, " Data Driver Slice Enable [11:8]          " ,1,0x4)
        ,REG_INFO_MAC(0x00681CA8, " Common Mode Termination Enable           " ,1,0x4)
        ,REG_INFO_MAC(0x00681CAC, " Equal Eye Common Mode Termination Enable " ,1,0x4)
        ,REG_INFO_MAC(0x00681CB0, " SST Driver Enable                        " ,1,0x4)
        ,REG_INFO_MAC(0x00681CD0, " SST Driver Enable Obs                    " ,1,0x4)
        ,REG_INFO_MAC(0x00681CE0, " Loopback Data Driver Slice Enable [7:0]  " ,1,0x4)
        ,REG_INFO_MAC(0x00681CE4, " Loopback Data Driver Slice Enable [11:8] " ,1,0x4)
        ,REG_INFO_MAC(0x00681CE8, " Loopback SST Driver Enable               " ,1,0x4)
        ,REG_INFO_MAC(0x00681CF0, " odctrl                                   " ,1,0x4)
        ,REG_INFO_MAC(0x00681D00, " Spare Observes                           " ,1,0x4)
        ,REG_INFO_MAC(0x00681D04, " Spare Controls                           " ,1,0x4)

        /* must be last */
        ,LAST_REG_INFO_MAC
    };

/* using SMI ! compare raven1,D2D1 with raven2,D2D2 */
GT_STATUS prvCpssDxChShortRavenSmiD2DRegsComparePrint
(
    IN  GT_U8                devNum,
    IN  GT_U32               ravenIndex_1,/* 0 , 1, 2, 3, 7, 6, 5, 4,
                                           8 , 9,10,11,15,14,13,12 */
    IN  GT_U32               d2dIndex_1,  /* 0 , 1 */
    IN  GT_U32               ravenIndex_2,/* 0 , 1, 2, 3, 7, 6, 5, 4,
                                           8 , 9,10,11,15,14,13,12 */
    IN  GT_U32               d2dIndex_2   /* 0 , 1 */
)
{
    REG_INFO *regInfoPtr;
    GT_STATUS rc;
    GT_U32 regAddr_1,regAddr_2,ii,regValue1,regValue2,baseAddr_1,baseAddr_2,maxRavenId;

    devNum = devNum;

    if(!PRV_CPSS_DXCH_PP_MAC(devNum))
    {
        return GT_BAD_PARAM;
    }
    maxRavenId = 4*PRV_CPSS_DXCH_PP_MAC(devNum)->genInfo.multiPipe.numOfTiles;

    regInfoPtr  = &short_falcon_d2d_gw_reg_info[0];

    if(d2dIndex_1 > 1 || d2dIndex_2 > 1 || ravenIndex_1 >= maxRavenId || ravenIndex_2 >= maxRavenId)
    {
        return GT_BAD_PARAM;
    }


    baseAddr_1 = 0x00680000 + d2dIndex_1 * 0x00010000;
    baseAddr_2 = 0x00680000 + d2dIndex_2 * 0x00010000;

    {
        GT_U32  tileId_1 = ravenIndex_1 / 4;
        GT_U32  flaconAddr_1 =  tileId_1 * 0x20000000 +  (ravenIndex_1 % 4) * 0x01000000;
        GT_U32  tileId_2 = ravenIndex_2 / 4;
        GT_U32  flaconAddr_2 =  tileId_2 * 0x20000000 +  (ravenIndex_2 % 4) * 0x01000000;

        baseAddr_1 += flaconAddr_1;
        baseAddr_2 += flaconAddr_2;
    }

    PR("compare R[%d] D2D[%d] to R[%d] D2D[%d] \n",
        ravenIndex_1,d2dIndex_1,
        ravenIndex_2,d2dIndex_2);

    for(/*no init*/ ; regInfoPtr->regName ; regInfoPtr++)
    {
        for(ii = 0 ; ii < regInfoPtr->numRegs ; ii++)
        {
            regAddr_1 = baseAddr_1 + (regInfoPtr->regOffset & 0xFFFF) +
                ii * regInfoPtr->offsetFormula;

            rc = SMI_READ(devNum,ravenIndex_1, regAddr_1, &regValue1);
            if (rc != GT_OK)
            {
                PR("regRead failed, regAddr %08X, rc %d\n", regAddr_1, rc);
            }

            regAddr_2 = baseAddr_2 + (regInfoPtr->regOffset & 0xFFFF) +
                ii * regInfoPtr->offsetFormula;

            rc = SMI_READ(devNum,ravenIndex_2 , regAddr_2, &regValue2);
            if (rc != GT_OK)
            {
                PR("regRead failed, regAddr %08X, rc %d\n", regAddr_2, rc);
            }

            if(regValue1 != regValue2)
            {
                /* print only registers with value1 != value2 !!!! */
                PR("[0x%8.8x] with value [0x%8.8x] compared to [0x%8.8x] with value [0x%8.8x] 'XOR'[0x%8.8x] [%s] [%d] \n",
                    regAddr_1,regValue1,
                    regAddr_2,regValue2,
                    regValue1^regValue2,
                    regInfoPtr->regName,ii);

            }
        }
    }

    return GT_OK;
}

/* compare the eagle side for the attached raven1,D2D1 with raven2,D2D2 */
GT_STATUS prvCpssDxChShortEagleD2DRegsComparePrint
(
    IN  GT_U8                devNum,
    IN  GT_U32               ravenIndex_1,/* 0 , 1, 2, 3, 7, 6, 5, 4,
                                           8 , 9,10,11,15,14,13,12 */
    IN  GT_U32               d2dIndex_1,  /* 0 , 1 */
    IN  GT_U32               ravenIndex_2,/* 0 , 1, 2, 3, 7, 6, 5, 4,
                                           8 , 9,10,11,15,14,13,12 */
    IN  GT_U32               d2dIndex_2   /* 0 , 1 */
)
{
    REG_INFO *regInfoPtr,*filterregInfoPtr;
    GT_STATUS rc;
    GT_U32 regAddr_1,regAddr_2,ii,regValue1,regValue2,baseAddr_1,baseAddr_2,maxRavenId;
    GT_U32  tileId_1,tileId_2;
    GT_U32  flaconAddr_1,flaconAddr_2;
    GT_U32  localPipeId_1,localPipeId_2;
    GT_U32  pipeAddr_1,pipeAddr_2;
    GT_U32  skip;

    if(!PRV_CPSS_DXCH_PP_MAC(devNum))
    {
        return GT_BAD_PARAM;
    }
    maxRavenId = 4*PRV_CPSS_DXCH_PP_MAC(devNum)->genInfo.multiPipe.numOfTiles;

    regInfoPtr  = &short_falcon_d2d_gw_reg_info[0];
    filterregInfoPtr = &falcon_skip_not_in_eagle_d2d_gw_reg_info[0];

    if(d2dIndex_1 > 1 || d2dIndex_2 > 1 || ravenIndex_1 >= maxRavenId || ravenIndex_2 >= maxRavenId)
    {
        return GT_BAD_PARAM;
    }

    tileId_1 = ravenIndex_1 / 4;
    tileId_2 = ravenIndex_2 / 4;

    flaconAddr_1 = tileId_1 * 0x20000000;
    flaconAddr_2 = tileId_2 * 0x20000000;

    localPipeId_1 = (ravenIndex_1 % 4) / 2;
    localPipeId_2 = (ravenIndex_2 % 4) / 2;

    pipeAddr_1 = localPipeId_1 * 0x08000000;
    pipeAddr_2 = localPipeId_2 * 0x08000000;

    flaconAddr_1 += pipeAddr_1;
    flaconAddr_2 += pipeAddr_2;

    /* print before the swap of d2dIndex_1 , d2dIndex_2 */
    PR("compare on EAGLE the D2D that connected to :(T[%d]P[%d]) R[%d] D2D[%d] to (T[%d]P[%d]) R[%d] D2D[%d] \n",
        tileId_1,localPipeId_1,
        ravenIndex_1,d2dIndex_1,
        tileId_2,localPipeId_2,
        ravenIndex_2,d2dIndex_2);


    if(tileId_1 & 1)
    {
        /* d2d between eagle and Raven are swapped */
        d2dIndex_1 = 1 - d2dIndex_1;
    }

    if(tileId_2 & 1)
    {
        /* d2d between eagle and Raven are swapped */
        d2dIndex_2 = 1 - d2dIndex_2;
    }

    baseAddr_1 = ((ravenIndex_1 & 1) ? 0x0D810000 : 0x0D7F0000) + d2dIndex_1 * 0x00010000 + flaconAddr_1;
    baseAddr_2 = ((ravenIndex_2 & 1) ? 0x0D810000 : 0x0D7F0000) + d2dIndex_2 * 0x00010000 + flaconAddr_2;

    for(/*no init*/ ; regInfoPtr->regName ; regInfoPtr++)
    {
        skip = 0;
        for(ii = 0 ; filterregInfoPtr[ii].regName ; ii++)
        {
            if(regInfoPtr->regOffset == filterregInfoPtr[ii].regOffset)
            {
                skip = 1;
                break;
            }
        }
        if(skip)
        {
            continue;
        }

        for(ii = 0 ; ii < regInfoPtr->numRegs ; ii++)
        {
            regAddr_1 = baseAddr_1 + (regInfoPtr->regOffset & 0xFFFF) +
                ii * regInfoPtr->offsetFormula;

            rc = prvCpssDrvHwPpPortGroupReadRegister(devNum,0/*groupId*/,regAddr_1, &regValue1);
            if (rc != GT_OK)
            {
                PR("regRead failed, regAddr %08X, rc %d\n", regAddr_1, rc);
            }

            regAddr_2 = baseAddr_2 + (regInfoPtr->regOffset & 0xFFFF) +
                ii * regInfoPtr->offsetFormula;

            rc = prvCpssDrvHwPpPortGroupReadRegister(devNum,0/*groupId*/, regAddr_2, &regValue2);
            if (rc != GT_OK)
            {
                PR("regRead failed, regAddr %08X, rc %d\n", regAddr_2, rc);
            }

            if(regValue1 != regValue2)
            {
                /* print only registers with value1 != value2 !!!! */
                PR("[0x%8.8x] with value [0x%8.8x] compared to [0x%8.8x] with value [0x%8.8x] 'XOR'[0x%8.8x] [%s] [%d] \n",
                    regAddr_1,regValue1,
                    regAddr_2,regValue2,
                    regValue1^regValue2,
                    regInfoPtr->regName,ii);

            }
        }
    }

    return GT_OK;
}



/* compare the Raven/eagle side for bad D2D compare to other 2 'good' */
GT_STATUS prvCpssDxChFalconBadD2DCheck
(
    IN  GT_U8                devNum,
    IN  GT_U32               ravenSide,     /* 0 = eagle side , else Raven side */
    IN  GT_U32               shortCompare,  /* 0 = long compare , else short compare */
    IN  GT_U32               d2dIndex,      /* 0 , 1 = local d2d in the Raven */
    IN  GT_U32               bad_ravenIndex,/* 0 , 1, 2, 3, 7, 6, 5, 4,
                                           8 , 9,10,11,15,14,13,12 */
    IN  GT_U32               good_ravenIndex_1,/* 0 , 1, 2, 3, 7, 6, 5, 4,
                                           8 , 9,10,11,15,14,13,12 */
    IN  GT_U32               good_ravenIndex_2/* 0 , 1, 2, 3, 7, 6, 5, 4,
                                           8 , 9,10,11,15,14,13,12 */
)
{
    REG_INFO *regInfoPtr,*filterregInfoPtr;
    GT_STATUS rc;
    GT_U32 regAddr_1,regAddr_2,ii,regValue1,regValue2,baseAddr_1,baseAddr_2,maxRavenId;
    GT_U32 regAddr_3,regValue3,baseAddr_3;
    GT_U32  d2dIndex_1,d2dIndex_2,d2dIndex_3;
    GT_U32  tileId_1,tileId_2,tileId_3;
    GT_U32  flaconAddr_1,flaconAddr_2,flaconAddr_3;
    GT_U32  localPipeId_1,localPipeId_2,localPipeId_3;
    GT_U32  pipeAddr_1,pipeAddr_2,pipeAddr_3;
    GT_U32  skip;

    if(!PRV_CPSS_DXCH_PP_MAC(devNum))
    {
        return GT_BAD_PARAM;
    }
    maxRavenId = 4*PRV_CPSS_DXCH_PP_MAC(devNum)->genInfo.multiPipe.numOfTiles;

    if(!shortCompare)
    {
        regInfoPtr  = &falcon_d2d_gw_reg_info[0];
    }
    else
    {
        regInfoPtr  = &short_falcon_d2d_gw_reg_info[0];
    }

    filterregInfoPtr = &falcon_skip_not_in_eagle_d2d_gw_reg_info[0];

    if(d2dIndex > 1 ||
        bad_ravenIndex    >= maxRavenId ||
        good_ravenIndex_1 >= maxRavenId ||
        good_ravenIndex_2 >= maxRavenId)
    {
        return GT_BAD_PARAM;
    }

    d2dIndex_1 = d2dIndex;
    d2dIndex_2 = d2dIndex;
    d2dIndex_3 = d2dIndex;

    if(!ravenSide)
    {
        tileId_1 = bad_ravenIndex / 4;
        tileId_2 = good_ravenIndex_1 / 4;
        tileId_3 = good_ravenIndex_2 / 4;

        flaconAddr_1 = tileId_1 * 0x20000000;
        flaconAddr_2 = tileId_2 * 0x20000000;
        flaconAddr_3 = tileId_3 * 0x20000000;

        localPipeId_1 = (bad_ravenIndex % 4) / 2;
        localPipeId_2 = (good_ravenIndex_1 % 4) / 2;
        localPipeId_3 = (good_ravenIndex_2 % 4) / 2;

        pipeAddr_1 = localPipeId_1 * 0x08000000;
        pipeAddr_2 = localPipeId_2 * 0x08000000;
        pipeAddr_3 = localPipeId_3 * 0x08000000;

        flaconAddr_1 += pipeAddr_1;
        flaconAddr_2 += pipeAddr_2;
        flaconAddr_3 += pipeAddr_3;

        /* print before the swap of d2dIndex */
        PR("compare on EAGLE the D2D that connected to :(T[%d]P[%d]) R[%d] D2D[%d] to (T[%d]P[%d]) R[%d] , (T[%d]P[%d]) R[%d]\n",
            tileId_1,localPipeId_1,
            d2dIndex,d2dIndex,
            tileId_2,localPipeId_2,
            good_ravenIndex_1,
            tileId_3,localPipeId_3,
            good_ravenIndex_2
            );

        if(tileId_1 & 1)
        {
            /* d2d between eagle and Raven are swapped */
            d2dIndex_1 = 1 - d2dIndex_1;
        }

        if(tileId_2 & 1)
        {
            /* d2d between eagle and Raven are swapped */
            d2dIndex_2 = 1 - d2dIndex_2;
        }

        if(tileId_3 & 1)
        {
            /* d2d between eagle and Raven are swapped */
            d2dIndex_3 = 1 - d2dIndex_3;
        }

        baseAddr_1 = ((bad_ravenIndex & 1) ? 0x0D810000 : 0x0D7F0000) + d2dIndex_1 * 0x00010000 + flaconAddr_1;
        baseAddr_2 = ((good_ravenIndex_1 & 1) ? 0x0D810000 : 0x0D7F0000) + d2dIndex_2 * 0x00010000 + flaconAddr_2;
        baseAddr_3 = ((good_ravenIndex_2 & 1) ? 0x0D810000 : 0x0D7F0000) + d2dIndex_3 * 0x00010000 + flaconAddr_3;

        PR("The base global addr are : [0x%8.8x],[0x%8.8x],[0x%8.8x] \n",
            baseAddr_1,
            baseAddr_2,
            baseAddr_3);
    }
    else
    {
        PR("compare on RAVEN : R[%d] D2D[%d] to R[%d] , R[%d] \n",
            bad_ravenIndex,d2dIndex,
            good_ravenIndex_1,
            good_ravenIndex_2);

        baseAddr_1 = 0x00680000 + d2dIndex_1 * 0x00010000;
        baseAddr_2 = 0x00680000 + d2dIndex_2 * 0x00010000;
        baseAddr_3 = 0x00680000 + d2dIndex_3 * 0x00010000;

        tileId_1 = bad_ravenIndex / 4;
        tileId_2 = good_ravenIndex_1 / 4;
        tileId_3 = good_ravenIndex_2 / 4;

        flaconAddr_1 = tileId_1 * 0x20000000 +  (bad_ravenIndex    % 4) * 0x01000000;
        flaconAddr_2 = tileId_2 * 0x20000000 +  (good_ravenIndex_1 % 4) * 0x01000000;
        flaconAddr_3 = tileId_3 * 0x20000000 +  (good_ravenIndex_2 % 4) * 0x01000000;

        baseAddr_1 += flaconAddr_1;
        baseAddr_2 += flaconAddr_2;
        baseAddr_3 += flaconAddr_3;

        PR("The base global addr are : [0x%8.8x],[0x%8.8x],[0x%8.8x] \n",
            baseAddr_1,
            baseAddr_2,
            baseAddr_3);
    }

    for(/*no init*/ ; regInfoPtr->regName ; regInfoPtr++)
    {
        skip = 0;
        for(ii = 0 ; filterregInfoPtr[ii].regName ; ii++)
        {
            if(regInfoPtr->regOffset == filterregInfoPtr[ii].regOffset)
            {
                skip = 1;
                break;
            }
        }

        if(skip)
        {
            continue;
        }

        for(ii = 0 ; ii < regInfoPtr->numRegs ; ii++)
        {
            regAddr_2 = baseAddr_2 + (regInfoPtr->regOffset & 0xFFFF) +
                ii * regInfoPtr->offsetFormula;

            if(!ravenSide)
            {
                rc = prvCpssDrvHwPpPortGroupReadRegister(devNum,0/*groupId*/, regAddr_2, &regValue2);
            }
            else
            {
                rc = SMI_READ(devNum,good_ravenIndex_1, regAddr_2, &regValue2);
            }

            if (rc != GT_OK)
            {
                PR("regRead failed, regAddr %08X, rc %d\n", regAddr_2, rc);
            }

            regAddr_3 = baseAddr_3 + (regInfoPtr->regOffset & 0xFFFF) +
                ii * regInfoPtr->offsetFormula;

            if(!ravenSide)
            {
                rc = prvCpssDrvHwPpPortGroupReadRegister(devNum,0/*groupId*/, regAddr_3, &regValue3);
            }
            else
            {
                rc = SMI_READ(devNum,good_ravenIndex_2, regAddr_3, &regValue3);
            }

            if (rc != GT_OK)
            {
                PR("regRead failed, regAddr %08X, rc %d\n", regAddr_3, rc);
            }

            if(regValue2 != regValue3)
            {
                /* do not check 'bad raven' for register that are not equal on 2 good ravens */
                continue;
            }


            regAddr_1 = baseAddr_1 + (regInfoPtr->regOffset & 0xFFFF) +
                ii * regInfoPtr->offsetFormula;

            if(!ravenSide)
            {
                rc = prvCpssDrvHwPpPortGroupReadRegister(devNum,0/*groupId*/,regAddr_1, &regValue1);
            }
            else
            {
                rc = SMI_READ(devNum,bad_ravenIndex, regAddr_1, &regValue1);
            }

            if (rc != GT_OK)
            {
                PR("regRead failed, regAddr %08X, rc %d\n", regAddr_1, rc);
            }

            if(regValue1 != regValue2)
            {
                /* print only registers with value1 != value2 !!!! */
                PR("[0x%8.8x] with value [0x%8.8x] compared to [0x%8.8x] with value [0x%8.8x] 'XOR'[0x%8.8x] [%s] [%d] \n",
                    regAddr_1,regValue1,
                    regAddr_2,regValue2,
                    regValue1^regValue2,
                    regInfoPtr->regName,ii);

            }
        }
    }

    return GT_OK;
}

