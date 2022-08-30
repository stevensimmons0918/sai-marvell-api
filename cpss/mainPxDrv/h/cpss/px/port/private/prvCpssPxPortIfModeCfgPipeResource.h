/*******************************************************************************
*              (c), Copyright 2017, Marvell International Ltd.                 *
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
* @file prvCpssPxPortIfModeCfgPipeResource.h
*
* @brief CPSS Pipe implementation for Port interface mode resource configuration.
*
*
* @version   1
********************************************************************************
*/
#ifndef __PRV_CPSS_PX_PORT_IF_MODE_CFG_PIPE_RESOURCE_H
#define __PRV_CPSS_PX_PORT_IF_MODE_CFG_PIPE_RESOURCE_H


#include <cpss/common/cpssTypes.h>
#include <cpss/common/port/cpssPortCtrl.h>

#if 0
Pipe
Global configurations:
    +-----------------------------------------------+-------+-------------------+------------------+--------------------------------------------------------------------------------------------------------
    |                                               |       |     Register      |    Fld           |
    +    Field                                      | Unit  +------------+------+----+-----+-------+    Cider
    |                                               |       |  address   |   N  |per |total|  fld  |
    |                                               |       |            |      |reg |     |       |
    +-----------------------------------------------+-------+------------+------+----+-----+-------+---------------------------------------------------------------------------------------------------------
?   | SCDMA %n CT Packet Identification             |RxDMA-0| 0x01000B00 | 0-16 |  1 |  17 |  0-6  | /Cider/EBU/PIPE/PIPE {Current}/Switching core/<RXDMA_IP> RxDMA IP TLU/Units/RxDMA IP Units/
    |                                               |       |            |      |    |     |       |     Single Channel DMA Configurations/SCDMA %n CT Packet Indentification
    |                                               |       |            |      |    |     |       |     http://cideril.marvell.com:81/Cider/link.php?str=11072719,7
    |                                               |       |            |      |    |     |       |     0x01000B00 + n*0x4 : where n (0-16) represents port, bits 0-6
    |                                               |       |            |      |    |     |       | possible values  1,      for system where all ports are 10G and lower (HW restriction -- see cider ?)
    |                                               |       |            |      |    |     |       |                  2(def)
    +-----------------------------------------------+-------+------------+------+----+-----+-------+----------------------------------------------------------------------------------------------------------
?   | SCDMA %p guarenteed BC for CT                 |TxDMA-0| 0x20003010 | 0-16 |  1 |  17 |  0-9  | /Cider/EBU/PIPE/PIPE {Current}/Switching core/<TXDMA_IP> TXDMA IP TLU/Units/TxDMA IP Units/
    |                                               |       |            |      |    |     |       |     TxDMA Per SCDMA Configurations/SCDMA %p Configurations Reg 1
    |                                               |       |            |      |    |     |       |     http://cideril.marvell.com:81/Cider/link.php?str=11642643,7
    |                                               |       |            |      |    |     |       |     0x02003010 + p*0x20 : where p (0-16) represents SCDMA, bits 0-9
    |                                               |       |            |      |    |     |       | possible values  128,    for system where all ports are 10G and lower (HW restriction -- see cider ?)
    |                                               |       |            |      |    |     |       |                  256 (def)
    +-----------------------------------------------+-------+------------+------+----+-----+-------+----------------------------------------------------------------------------------------------------------
?   | Local Dev Src Port to DMA Number Map %i       |TxDMA-0| 0x02000020 | 0-7  |  4 | 32  | 0-7   | /Cider/EBU/PIPE/PIPE {Current}/Switching core/<TXDMA_IP> TXDMA IP TLU/Units/TxDMA IP Units/
    |                                               |       |            |      |    |     | 8-15  |     TxDMA Global Configurations/Local Dev Src Port to DMA Number Map %i
    |                                               |       |            |      |    |     | 16-23 |     http://cideril.marvell.com:81/Cider/link.php?str=11642866,7
    |  MAPPING ?                                    |       |            |      |    |     | 24-31 |     0x02000020 + i*0x4 : where i (0-7) represents entry bits, 0-7, 8-15, 16-23, 24-31 (src-port)
    |                                               |       |            |      |    |     |       |
    |                                               |       |            |      |    |     |       |
    +-----------------------------------------------+-------+------------+------+----+-----+-------+----------------------------------------------------------------------------------------------------------



Fields:
    +-----------------------------------------------+--------+-------------------+------------------+--------+-------------------------------------------------------------------------------------------------------
    |                                               |        |     Register      |    Fld           | Is     |
    +    Field                                      | Unit   +------------+------+----+-----+-------+changed |   Cider
    |                                               |        |  address   |   N  |per |total|  fld  |        |
    |                                               |        |            |      |reg |     |       |        |
    +-----------------------------------------------+--------+------------+------+----+-----+-------+--------+--------------------------------------------------------------------------------------------------------
+   | TxQ-Descriptor-Counter                        |TxQ-IP  | 0x1E0A4010 | 0-19 |  1 |  20 |  0-15 |fld     | /Cider/EBU/PIPE/PIPE {Current}/Switching core/<TXQ_IP>TXQ_IP/<TXQ_IP> TXQ_QUEUE/Units/
    |                                               |        |            |      |    |     |       |width   |     TXQ_IP_queue/Tail Drop/Tail Drop Counters/Port<%n> Desc Counter
    |                                               |        |            |      |    |     |       |        |     http://cideril.marvell.com:81/Cider/link.php?str=11077134,7
    |                                               |        |            |      |    |     |       |        |     0x1E0A4010 + n*0x4 : where n (0-19) represents Port, bits 0-15
    |                                               |        |            |      |    |     |       |        |wait for TXQ decriptor credit SCDMA %p (see below)
    +-----------------------------------------------+--------+------------+------+----+-----+-------+--------+---------------------------------------------------------------------------------------------------------
+   | TXQ-Credit_Counters_Reset                     |TxQ-DQ  | 0x21000260 | 0-15 | 16 |  16 |  0-0  |        | /Cider/EBU/PIPE/PIPE {Current}/Switching core/<TXQ_IP>TXQ_IP/<TXQ_IP> TXQ_DQ/Units/
    |                                               |        | 0x22000260 | 0-15 | 16 |  16 |  0-0  |        |     TXQ Dequeue %a/Global/Global DQ Config/Credit Counters Reset <%n>
    |                                               |        |            |      |    |     |       |        |     http://cideril.marvell.com:81/Cider/link.php?str=11078602,7
    |                                               |        |            |      |    |     |       |        |     Base: 0x21000000 + 0x1000000 * a: where a (0-1) represents Dequeue Unit
    |                                               |        |            |      |    |     |       |        |     0x21000260 + n*0x4 : where n (0-0) represents RegNum - bit per port
    |                                               |        |            |      |    |     |       |        |
    +-----------------------------------------------+--------+------------+------+----+-----+-------+--------+--------------------------------------------------------------------------------------------------------
+   | TXDMA-SCDMA reset_desc_credits_counter_scdma  |TxDMA-0 | 0x0200301C | 0-16 |  1 |  17 |  0-0  |        | /Cider/EBU/PIPE/PIPE {Current}/Switching core/<TXDMA_IP> TXDMA IP TLU/Units/TxDMA IP Units/
    |                                               |        |            |      |    |     |       |        |     TxDMA Per SCDMA Configurations/SCDMA %p Configurations Reg 2
    |                                               |        |            |      |    |     |       |        |     http://cideril.marvell.com:81/Cider/link.php?str=11643072,7
    |                                               |        |            |      |    |     |       |        |     0x0200301C + 0x20*p : where p (0-16) represents SCDMA, bit 0
    |                                               |        |            |      |    |     |       |        |
    |                                               |        |            |      |    |     |       |        |
    +-----------------------------------------------+--------+------------+------+----+-----+-------+--------+--------------------------------------------------------------------------------------------------------
+   | TXDMA-SCDMA header txfifo counter             |TxDMA-0 | 0x02005200 | 0-16 |  1 |  17 |  0-9  |        |/Cider/EBU/PIPE/PIPE {Current}/Switching core/<TXDMA_IP> TXDMA IP TLU/Units/TxDMA IP Units/
    |                                               |        |            |      |    |     |       |        |    TxDMA Debug/Informative Debug/TxDMA SCDMA %p Status Reg 1
    |                                               |        |            |      |    |     |       |        |    http://cideril.marvell.com:81/Cider/link.php?str=11642673,7
    |                                               |        |            |      |    |     |       |        |    0x02005200 + p*0x4 : where p (0-16) represents SCDMA, bits 0-9
    |                                               |        |            |      |    |     |       |        |
    |                                               |        |            |      |    |     |       |        | wait for 0
    +-----------------------------------------------+--------+------------+------+----+-----+-------+--------+--------------------------------------------------------------------------------------------------------
+   | TXDMA-SCDMA payload txfifo counter            |TxDMA-0 | 0x02005200 | 0-16 |  1 |  17 | 10-19 |        |/Cider/EBU/PIPE/PIPE {Current}/Switching core/<TXDMA_IP> TXDMA IP TLU/Units/TxDMA IP Units/
    |                                               |        |            |      |    |     |       |        |    TxDMA Debug/Informative Debug/TxDMA SCDMA %p Status Reg 1
    |                                               |        |            |      |    |     |       |        |    http://cideril.marvell.com:81/Cider/link.php?str=11642673,7
    |                                               |        |            |      |    |     |       |        |    0x02005200 + p*0x4 : where p (0-16) represents SCDMA, bits 10-19
    |                                               |        |            |      |    |     |       |        |
    |                                               |        |            |      |    |     |       |        |wait for 0
    +-----------------------------------------------+--------+------------+------+----+-----+-------+--------+--------------------------------------------------------------------------------------------------------

Resources:
    +-----------------------------------------------+--------+-------------------+------------------+--------+--------------------------------------------------------------------------------------------------------
    |                                               |        |     Register      |    Fld           | Is     |
    +    Field                                      | Unit   +------------+------+----+-----+-------+changed |    Cider
    |                                               |        |  address   |   N  |per |total|  fld  |        |
    |                                               |        |            |      |reg |     |       |        |
    +-----------------------------------------------+--------+------------+------+----+-----+-------+--------+---------------------------------------------------------------------------------------------------------
+   | RXDNA-SCDMA Interface     width               |RxDMA-0 | 0x01000804 | 0-16 |  1 |  17 |  0-2  |        | /Cider/EBU/PIPE/PIPE {Current}/Switching core/<RXDMA_IP> RxDMA IP TLU/Units/RxDMA IP Units/
    |                                               |        |            |      |    |     |       |        |     Single Channel DMA Configurations/SCDMA %n Configuration 0
    |                                               |        |            |      |    |     |       |        |     http://cideril.marvell.com:81/Cider/link.php?str=11072732,7
    |                                               |        |            |      |    |     |       |        |     0x01000804 + n*0x4 : where n (0-16) represents Port, bits 0-2
    |                                               |        |            |      |    |     |       |        |
    |                                               |        |            |      |    |     |       |        |
    +-----------------------------------------------+--------+------------+------+----+-----+-------+--------+---------------------------------------------------------------------------------------------------------
+   | TXDMA-SCDMA desc_credits_scdma                |TxDMA-0 | 0x02003000 | 0-16 |  1 |  17 |  0-8  |        | /Cider/EBU/PIPE/PIPE {Current}/Switching core/<TXDMA_IP> TXDMA IP TLU/Units/TxDMA IP Units/
    |                                               |        |            |      |    |     |       |        |     TxDMA Per SCDMA Configurations/FIFOs Thresholds Configurations SCDMA %p Reg 1
    |                                               |        |            |      |    |     |       |        |     http://cideril.marvell.com:81/Cider/link.php?str=11642621,7
    |                                               |        |            |      |    |     |       |        |     0x02003000 + p*0x20 : where p (0-16) represents scdma   - bits 0-8
    |                                               |        |            |      |    |     |       |        |
    |                                               |        |            |      |    |     |       |        |
    +-----------------------------------------------+--------+------------+------+----+-----+-------+--------+---------------------------------------------------------------------------------------------------------
+   | TXDMA-SCDMA TxFIFO Header Counter Threshold   |TxDMA-0 | 0x0200300C | 0-16 |  1 |  17 |  0-9  |        | /Cider/EBU/PIPE/PIPE {Current}/Switching core/<TXDMA_IP> TXDMA IP TLU/Units/TxDMA IP Units/
    |                                               |        |            |      |    |     |       |        |     TxDMA Per SCDMA Configurations/TxFIFO Counters Configurations SCDMA %p
    |                                               |        |            |      |    |     |       |        |     http://cideril.marvell.com:81/Cider/link.php?str=11642628,7
    |                                               |        |            |      |    |     |       |        |     0x0200300C + 0x20*p : where p (0-16) represents SCDMA, bits 0-9
    |                                               |        |            |      |    |     |       |        |
    |                                               |        |            |      |    |     |       |        |
    +-----------------------------------------------+--------+------------+------+----+-----+-------+--------+---------------------------------------------------------------------------------------------------------
+   | TXDMA-SCDMA TxFIFO Payload Counter Threshold  |TxDMA-0 | 0x0200300C | 0-16 |  1 |  17 | 10-19 |        | /Cider/EBU/PIPE/PIPE {Current}/Switching core/<TXDMA_IP> TXDMA IP TLU/Units/TxDMA IP Units/
    |                                               |        |            |      |    |     |       |        |     TxDMA Per SCDMA Configurations/TxFIFO Counters Configurations SCDMA %p
    |                                               |        |            |      |    |     |       |        |     http://cideril.marvell.com:81/Cider/link.php?str=11642628,7
    |                                               |        |            |      |    |     |       |        |     0x0200300C + 0x20*p : where p (0-16) represents SCDMA, bits 10-19
    |                                               |        |            |      |    |     |       |        |
    |                                               |        |            |      |    |     |       |        |
    +-----------------------------------------------+--------+------------+------+----+-----+-------+--------+---------------------------------------------------------------------------------------------------------
    | TXDMA-SCDMA Burst almost full threshold       |TxDMA-0 | 0x02003018 | 0-16 |  1 |  17 |  0-15 |        | /Cider/EBU/PIPE/PIPE {Current}/Switching core/<TXDMA_IP> TXDMA IP TLU/Units/TxDMA IP Units/
    |                                               |        |            |      |    |     |       |        |     TxDMA Per SCDMA Configurations/Burst Limiter SCDMA %p
    |                                               |        |            |      |    |     |       |        |     http://cideril.marvell.com:81/Cider/link.php?str=11642632,7
    |                                               |        |            |      |    |     |       |        |     0x02003018 + 0x20*p : where p (0-16) represents SCDMA, bits 0-15
    |                                               |        |            |      |    |     |       |        |
    |                                               |        |            |      |    |     |       |        |
    +-----------------------------------------------+--------+------------+------+----+-----+-------+--------+--------+---------------------------------------------------------------------------------------------------------
    | TXDMA-SCDMA Burst almost full threshold       |TxDMA-0 | 0x02003018 | 0-16 |  1 |  17 | 16-31 |        | /Cider/EBU/PIPE/PIPE {Current}/Switching core/<TXDMA_IP> TXDMA IP TLU/Units/TxDMA IP Units/
    |                                               |        |            |      |    |     |       |        |     TxDMA Per SCDMA Configurations/Burst Limiter SCDMA %p
    |                                               |        |            |      |    |     |       |        |     http://cideril.marvell.com:81/Cider/link.php?str=11642632,7
    |                                               |        |            |      |    |     |       |        |     0x02003018 + 0x20*p : where p (0-16) represents SCDMA, bits 16-31
    |                                               |        |            |      |    |     |       |        |
    |                                               |        |            |      |    |     |       |        |
    +-----------------------------------------------+--------+------------+------+----+-----+-------+--------+---------------------------------------------------------------------------------------------------------
+   | TXDMA-SCDMA TxFIFO Payload Min Threshold      |TxFIFO-0| 0x04000008 | 0-16 |  1 |  17 |  0-6  |        | /Cider/EBU/PIPE/PIPE {Current}/Switching core/<TXFIFO_IP> TxFIFO IP TLU/Units/
    |                    for transmission           |        |            |      |   |     |       |        |     TxFIFO IP Units/TxFIFO Global Configuration/SCDMA %p Payload Threshold
    |                                               |        |            |      |   |     |       |        |     http://cideril.marvell.com:81/Cider/link.php?str=11074921,7
    |                                               |        |            |      |   |     |       |        |     0x04000008 + p*0x4 : where p (0-16) represents SCDMA, bits 0-6
    |                                               |        |            |      |   |     |       |        |
    |                                               |        |            |      |   |     |       |        |
    +-----------------------------------------------+--------+------------+------+----+-----+-------+--------+---------------------------------------------------------------------------------------------------------
+   | TXFIFO-SCDMA Outgoing bus width               |TxFIFO-0| 0x04000600 | 0-16 |  1 |  17 |  0-2  | coding | /Cider/EBU/PIPE/PIPE {Current}/Switching core/<TXFIFO_IP> TxFIFO IP TLU/Units/
    |                                               |        |            |      |    |     |       |changed |     TxFIFO IP Units/TxFIFO Shifters Configuration/SCDMA %p Shifters Configuration
    |                                               |        |            |      |    |     |       |        |     http://cideril.marvell.com:81/Cider/link.php?str=11075028,7
    |                                               |        |            |      |    |     |       |        |     0x04000600 + 0x4*p : where p (0-16) represents SCDMA, bits 0-2
    |                                               |        |            |      |    |     |       |        |
    |                                               |        |            |      |    |     |       |        | 0x3 -- 8B  == 1G, 2.5G, 5G, 10G, 12G, 25G
    |                                               |        |            |      |    |     |       |        | 0x5 -- 32B == 20G, 24G, 50G
    |                                               |        |            |      |    |     |       |        | 0x6 -- 64B == 40G, 48G, 100G, 110G, 120G
    +-----------------------------------------------+--------+------------+------+----+-----+-------+--------+----------------------------------------------------------------------------------------------------

GOPS THRESHOLD CONFIGURATIONS
    +-----------------------------------------------+--------+-------------------+------------------+--------------------------------------------------------------------------------------------------------
    |                                               |        |     Register      |    Fld           |
    +    Field                                      | Unit   +------------+------+----+-----+-------+    Cider
    |                                               |        |  address   |   N  |per |total|  fld  |
    |                                               |        |            |      |reg |     |       |
    +-----------------------------------------------+--------+------------+------+----+-----+-------+---------------------------------------------------------------------------------------------------------
?   | Giga Mac Port Fifo Min Threshold              |GigeMac0| 0x1000001C | 0-15 |  1 |  16 |  6-13 | /Cider/EBU/PIPE/PIPE {Current}/Switching core/GOP/GIGE_MAC_IP/Units/
    |                                               |        |            |      |   |     |       |     <GIGE_MAC_IP> Gige MAC IP %a/Port Internal/Port FIFO Configuration Register 1
    |                                               |        |            |      |   |     |       |     http://cideril.marvell.com:81/Cider/link.php?str=11064270,7
    |                                               |        |            |      |   |     |       |     Base: 0x10000000 + 0x1000*a: where a (0-15) represents Port_num
    |                                               |        |            |      |   |     |       |     0x1000001C, bits 6-13
    |                                               |        |            |      |   |     |       |  The following conditions need to exist: TxFifoHiWM > TxFifoLowWM > TxFifoMinTh
    +-----------------------------------------------+--------+------------+------+----+-----+-------+---------------------------------------------------------------------------------------------------------
+   | XLG Mac Port Fifo Min Threshold (TxRdThr      |XLGMac_0| 0x100C0010 | 0-0  |  1 |  1  | 11-15 | /Cider/EBU/PIPE/PIPE {Current}/Switching core/GOP/<XLG_MAC_IP> XLG MAC IP/Units/
    |                                               |        |            |      |    |     |       |     <XLG_MAC_IP> XLG MAC IP %a/Port FIFOs Thresholds Configuration
    |                                               |        |            |      |    |     |       |     http://cideril.marvell.com:81/Cider/link.php?str=11966107,7
    |                                               |        |            |      |    |     |       |     0x100C0010 bits 11-15
    |                                               |        |            |      |    |     |       |
    +-----------------------------------------------+--------+------------+------+----+-----+-------+---------------------------------------------------------------------------------------------------------

DMA speed
    +-----------------------------------------------+--------+-------------------+------------------+--------------------------------------------------------------------------------------------------------
    |                                               |        |     Register      |    Fld           |
    +    Field                                      | Unit   +------------+------+----+-----+-------+    Cider
    |                                               |        |  address   |   N  |per |total|  fld  |
    |                                               |        |            |      |reg |     |       |
    +-----------------------------------------------+--------+------------+------+----+-----+-------+---------------------------------------------------------------------------------------------------------
    | TRG port speed                                |TxDMA-0 | 0x02003010 | 0-16 |  1 | 17  | 16-19 | /Cider/EBU/PIPE/PIPE {Current}/Switching core/<TXDMA_IP> TXDMA IP TLU/Units/TxDMA IP Units/
    |                                               |        |            |      |    |     |       |     TxDMA Per SCDMA Configurations/SCDMA %p Configurations Reg 1
    |                                               |        |            |      |    |     |       |     http://cideril.marvell.com:81/Cider/link.php?str=11642643,7
    |                                               |        |            |      |    |     |       |     0x02003010 + p*0x20 : where p (0-16) represents SCDMA, bits 16-19
    |                                               |        |            |      |    |     |       |
    |                                               |        |            |      |    |     |       |
    +-----------------------------------------------+--------+------------+------+----+-----+-------+---------------------------------------------------------------------------------------------------------
    | SRC port speed                                |TxDMA-0 | 0x02000220 | 0-2  |  8 | 24  |  0-4  | /Cider/EBU/PIPE/PIPE {Current}/Switching core/<TXDMA_IP> TXDMA IP TLU/Units/
    |                                               |        |            |      |    |     |       |     TxDMA IP Units/TxDMA Global Configurations/Src DMA Speed Table %n
    |                                               |        |            |      |    |     |       |     http://cideril.marvell.com:81/Cider/link.php?str=11642873,7
    |                                               |        |            |      |    |     |       |     0x02000220 + n*0x4 : where n (0-2) represents entry, bits 0-7
    |                                               |        |            |      |    |     |       |
    |                                               |        |            |      |    |     |       | for each DP {configure SRS DMA speed TBL} end
    +-----------------------------------------------+--------+------------+------+----+-----+-------+---------------------------------------------------------------------------------------------------------
#endif


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define DIV_0_32   0x00000000
#define DIV_1_32   0x80000000
#define DIV_2_32   0x80008000
#define DIV_3_32   0x80200400
#define DIV_4_32   0x80808080
#define DIV_5_32   0x82081040
#define DIV_6_32   0x84208420
#define DIV_7_32   0x88442210
#define DIV_8_32   0x88888888
#define DIV_9_32   0x91224488
#define DIV_10_32  0x92489248
#define DIV_11_32  0xa4924924
#define DIV_12_32  0xa4a4a4a4
#define DIV_13_32  0xa94a5294
#define DIV_14_32  0xaa54aa54
#define DIV_15_32  0xaaaa5554
#define DIV_16_32  0xaaaaaaaa
#define DIV_17_32  0xd555aaaa
#define DIV_18_32  0xd5aad5aa
#define DIV_19_32  0xd6b5ad6a
#define DIV_20_32  0xdadadada
#define DIV_21_32  0xdb6db6da
#define DIV_22_32  0xedb6edb6
#define DIV_23_32  0xeeddbb76
#define DIV_24_32  0xeeeeeeee
#define DIV_25_32  0xf7bbddee
#define DIV_26_32  0xfbdefbde
#define DIV_27_32  0xfdf7efbe
#define DIV_28_32  0xfefefefe
#define DIV_29_32  0xffdffbfe
#define DIV_30_32  0xfffefffe
#define DIV_31_32  0xfffffffe

typedef struct
{
    GT_U32  real_coreClockMHz;
    GT_U32  computation_coreClockMHz;
    GT_U32  placeInArr;
}PRV_CPSS_PX_CORECLOCK_PLACE_STC;

typedef struct
{
    CPSS_PORT_SPEED_ENT  speedEnm;
    GT_U32               speedMbps;
    GT_U32               placeInArr;
}PRV_CPSS_PX_SPEED_PLACE_STC;

#define PRV_CPSS_PX_PORT_RESOURCE_SPEEDS_NUMOF_CNS 20

extern const PRV_CPSS_PX_SPEED_PLACE_STC prv_PIPE_speedPlaceArr[];
extern const PRV_CPSS_PX_CORECLOCK_PLACE_STC prv_PIPE_coreClockPlaceArr[];
extern const GT_U32 PIPE_TXDMA_SCDMA_TxQDescriptorCredit_Arr[
    5][PRV_CPSS_PX_PORT_RESOURCE_SPEEDS_NUMOF_CNS];
extern const GT_U32 PIPE_TXDMA_SCDMA_burstAlmostFullThreshold_Arr[
    5][PRV_CPSS_PX_PORT_RESOURCE_SPEEDS_NUMOF_CNS];
extern const GT_U32 PIPE_TXDMA_SCDMA_burstFullThreshold_Arr[
    5][PRV_CPSS_PX_PORT_RESOURCE_SPEEDS_NUMOF_CNS];
extern const GT_U32 PIPE_TXDMA_SCDMA_TxFIFOHeaderCreditThreshold_Arr[
    5][PRV_CPSS_PX_PORT_RESOURCE_SPEEDS_NUMOF_CNS];
extern const GT_U32 PIPE_TXDMA_SCDMA_TxFIFOPayloadCreditThreshold_Arr[
    5][PRV_CPSS_PX_PORT_RESOURCE_SPEEDS_NUMOF_CNS];
extern const GT_U32 PIPE_TXFIFO_SCDMA_PayloadStartTransmThreshold_Arr[
    5][PRV_CPSS_PX_PORT_RESOURCE_SPEEDS_NUMOF_CNS];

typedef enum
{
    PRV_CPSS_PX_RXDMA_IfWidth_64_E = 0,  /* For up to 10G */
    PRV_CPSS_PX_RXDMA_IfWidth_256_E = 2, /* For 40G and 20G */
    PRV_CPSS_PX_RXDMA_IfWidth_512_E = 3, /* For 100G */
    PRV_CPSS_PX_RXDMA_IfWidth_MAX_E
}PRV_CPSS_PX_RXDMA_IfWidth_ENT;


typedef enum
{
    PRV_CPSS_PX_TxFIFO_OutGoingBusWidth_8B_E = 3,    /*1G/2.5G/5G/10G/12G/20G/25G*/
    PRV_CPSS_PX_TxFIFO_OutGoingBusWidth_32B_E = 5,   /*40G/50G*/
    PRV_CPSS_PX_TxFIFO_OutGoingBusWidth_64B_E = 6,   /*100G*/
    PRV_CPSS_PX_TxFIFO_OutGoingBusWidth_MAX_E
}PRV_CPSS_PX_TxFIFO_OutGoingBusWidth_ENT;


#define PRV_CPSS_PX_PORT_RESOURCE_BAD_VALUE_CNS               0xFFFFFBAD

typedef enum
{
    PRV_CPSS_PX_REG_FIELDS_RXDMA_SCDMA_INCOMING_BUS_WIDTH_E,
    PRV_CPSS_PX_REG_FIELDS_TXQ_Q_PORT_DESC_COUNTER_E,
    PRV_CPSS_PX_REG_FIELDS_TXQ_DQ_TXDMA_PORT_CREDIT_COUNTER_E,
    PRV_CPSS_PX_REG_FIELDS_TXQ_DQ_TXDMA_CREDIT_COUNTERS_RESET_E,
    PRV_CPSS_PX_REG_FIELDS_TXFIFO_SCDMA_PAYLOAD_MIN_THRESHOLD_4_TRANSM_E,
    PRV_CPSS_PX_REG_FIELDS_TXFIFO_SCDMA_SHIFTERS_OUTGOING_BUS_WIDTH_E,
    PRV_CPSS_PX_REG_FIELDS_TXDMA_SCDMA_RESET_DESC_CREDITS_COUNTER_E,
    PRV_CPSS_PX_REG_FIELDS_TXDMA_DESC_CREDITS_SCDMA_E,
    PRV_CPSS_PX_REG_FIELDS_TXDMA_SCDMA_HEADER_TXFIFO_COUNTER_E,
    PRV_CPSS_PX_REG_FIELDS_TXDMA_SCDMA_PAYLOAD_TXFIFO_COUNTER_E,
    PRV_CPSS_PX_REG_FIELDS_TXDMA_SCDMA_TXFIFO_HEADER_THRESHOLD_E,
    PRV_CPSS_PX_REG_FIELDS_TXDMA_SCDMA_TXFIFO_PAYLOAD_THRESHOLD_E,
    PRV_CPSS_PX_REG_FIELDS_TXDMA_SCDMA_BURST_ALMOST_FULL_THRESHOLD_E,
    PRV_CPSS_PX_REG_FIELDS_TXDMA_SCDMA_BURST_FULL_THRESHOLD_E,
    PRV_CPSS_PX_REG_FIELDS_TXDMA_SCDMA_RATE_LIMIT_INTEGER_THRESHOLD_E,
    PRV_CPSS_PX_REG_FIELDS_TXDMA_SCDMA_RATE_LIMIT_RESIDUE_VECT_THRESHOLD_E,
    PRV_CPSS_PX_REG_FIELDS_TXDMA_GLOBAL_SOURCE_DMA_SPEED_TBL_E,
    PRV_CPSS_PX_REG_FIELDS_TXDMA_GLOBAL_TARGET_DMA_SPEED_TBL_E,
    PRV_CPSS_PX_REG_FIELDS_XLG_MAC_FIFO_TX_WRITE_THRESHOLD_E,
    PRV_CPSS_PX_REG_FIELDS_LAST
}PRV_CPSS_PX_REG_FIELDS_ENT;

typedef struct
{
    GT_U32  rxdmaScdmaIncomingBusWidth;
    GT_U32  txqQPortDescriptorsCounter;
    GT_U32  txqDqPortCreditCounter;
    GT_U32  txqDqCreditCountersReset;
    GT_U32  txfifoScdmaPayloadStartTransmThreshold;
    GT_U32  txfifoScdmaShiftersOutgoingBusWidth;
    GT_U32  txdmaScdmaResetDescCreditsCounter;
    GT_U32  txdmaDescCreditScdma;
    GT_U32  txdmaScdmaHeaderTxfifoCounter;
    GT_U32  txdmaScdmaPayloadTxfifoCounter;
    GT_U32  txdmaScdmaHeaderTxfifoThrshold;
    GT_U32  txdmaScdmaPayloadTxfifoThrshold;
    GT_U32  txdmaScdmaBurstAmostFullThrshold;
    GT_U32  txdmaScdmaBurstFullThrshold;
    GT_U32  txdmaScdmaRateLimitIntegerThreshold;
    GT_U32  txdmaScdmaRateLimitResidueVectThreshold;
    GT_U32  txdmaGlobalSourceDmaSpeed;
    GT_U32  txdmaGlobalTargetDmaSpeed;
    GT_U32  xlgMacFifoTxWriteThreshold;
}PRV_CPSS_PX_PIPE_PORT_RESOURCE_STC;



/**
* @internal prvCpssPxPortResourcesInit function
* @endinternal
*
* @brief   Initialize data structure for port resource allocation
*
* @note   APPLICABLE DEVICES:      Pipe;
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
*
* @retval GT_OK                    - on success
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssPxPortResourcesInit
(
    IN  GT_SW_DEV_NUM   devNum
);

/**
* @internal prvCpssPxPortResourcesConfig function
* @endinternal
*
* @brief   Pipe port resourse configuration
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] ifMode                   - port interface mode
* @param[in] speed                    - port interface speed
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number, device, ifMode, speed
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssPxPortResourcesConfig
(
    IN GT_SW_DEV_NUM                   devNum,
    IN GT_PHYSICAL_PORT_NUM            portNum,
    IN CPSS_PORT_INTERFACE_MODE_ENT    ifMode,
    IN CPSS_PORT_SPEED_ENT             speed
);

/**
* @internal prvCpssPxPortResourcesRelease function
* @endinternal
*
* @brief   Remove static resources allocated to the port
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - on not supported interface for given port
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssPxPortResourcesRelease
(
    IN GT_SW_DEV_NUM                    devNum,
    IN GT_PHYSICAL_PORT_NUM             portNum
);

/**
* @internal prvCpssPxPortApHighLevelConfigurationSet function
* @endinternal
*
* @brief   Allocate/release high level port data path resources (pizza etc.) for
*         port where AP enabled, while physical port interface configuration made
*         by HWS in co-processor.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] portCreate               - GT_TRUE - port create;
*                                      GT_FALSE - port delete;
* @param[in] ifMode                   - interface mode
* @param[in] speed                    - port data speed
*
* @retval GT_OK                    - on success - port not in use by AP processor
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_NOT_INITIALIZED       - AP engine or library not initialized
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssPxPortApHighLevelConfigurationSet
(
    IN  GT_SW_DEV_NUM                   devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  GT_BOOL                         portCreate,
    IN  CPSS_PORT_INTERFACE_MODE_ENT    ifMode,
    IN  CPSS_PORT_SPEED_ENT             speed
);

/**
* @internal prvCpssPxPortApPortModeAckSet function
* @endinternal
*
* @brief   Send Ack to Service CPU indicate pizza resources allocate
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - number of physical port
*                                      mode       - mode in CPSS format
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PARAM             - on wrong port number or device
*/
GT_STATUS prvCpssPxPortApPortModeAckSet
(
    IN  GT_SW_DEV_NUM                   devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  CPSS_PORT_INTERFACE_MODE_ENT    ifMode,
    IN  CPSS_PORT_SPEED_ENT             speed
);

/**
* @internal prvCpssPxPortResourcesConfigDbAdd function
* @endinternal
*
* @brief   Resource data structure DB add operation.
*
* @note   APPLICABLE DEVICES:      Pipe;
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] dpIndex                  - dpIndex
*                                      speedValueMbps      - speed Mbps
*                                      maxDescCredits      - descriptor credit
*                                      maxHeaderCredits    - header credit
*                                      maxPayloadCredits   - payload credit
*
* @retval GT_OK                    - on success
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssPxPortResourcesConfigDbAdd
(
    IN  GT_SW_DEV_NUM   devNum,
    IN  GT_U32          dpIndex,
    IN  GT_U32          txQDescrCredits,
    IN  GT_U32          txFifoHeaderCredits,
    IN  GT_U32          txFifoPayloadCredits
);

/**
* @internal prvCpssPxPortResourcesConfigDbDelete function
* @endinternal
*
* @brief   Resource data structure DB delete operation.
*
* @note   APPLICABLE DEVICES:      Pipe;
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] dpIndex                  - dpIndex
*                                      speedValueMbps      - speed Mbps
*                                      maxDescCredits      - descriptor credit
*                                      maxHeaderCredits    - header credit
*                                      maxPayloadCredits   - payload credit
*
* @retval GT_OK                    - on success
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssPxPortResourcesConfigDbDelete
(
    IN  GT_SW_DEV_NUM   devNum,
    IN  GT_U32          dpIndex,
    IN  GT_U32          txQDescrCredits,
    IN  GT_U32          txFifoHeaderCredits,
    IN  GT_U32          txFifoPayloadCredits
);

/**
* @internal prvCpssPxPipePortResourceConfigGet function
* @endinternal
*
* @brief   Get resources allocated to the port
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - on not supported interface for given port
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssPxPipePortResourceConfigGet
(
    IN  GT_SW_DEV_NUM                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                        portNum,
    OUT PRV_CPSS_PX_PIPE_PORT_RESOURCE_STC          *resourcePtr
);

/**
* @internal prvCpssPxPortResourcesSpeedIndexGet function
* @endinternal
*
* @brief   Gets speed index in database
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] speed                    - port interface speed
*
* @param[out] speedIndexPtr            - (pointer to) speed DB
* @param[out] speedValuePtr            - (pointer to) speed value in Mbps
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong speed
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssPxPortResourcesSpeedIndexGet
(
    IN  CPSS_PORT_SPEED_ENT  speed,
    OUT GT_U32               *speedIndexPtr,
    OUT GT_U32               *speedValuePtr
);

/**
* @internal prvCpssPxPortResourcesRateLimitSet function
* @endinternal
*
* @brief    Jumbo with loopback issue:
*           Due to the loopback in the MAC, the S/D-PCS is no
*           longer limiting the rate of the port to exactly
*           10Gbps. The port speed is determined by the
*           MAC_CLK/Divider (in this case 833Mhz/4 * 64 =
*           13.33Gbps). Because the rate limiter of the loopback
*           port was enabled for exactly 10Gbps, the port got an
*           underrun (when rate limiter was disabled or shaped
*           to port speed of 13.33, there are no more issue).
*           Solution: As long as there is no Cut-Through traffic
*           towards the loopback port - disable the loopback
*           port rate limiter By writing 0x2 into TxDMA %a/TxDMA
*           Per SCDMA Configurations/SCDMA %p Configurations Reg
*           1/<SCDMA %p Rlimit Integer Thr>
*
* @note   APPLICABLE DEVICES:      pipe.
* @note   NOT APPLICABLE DEVICES:  none.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] speed                    - port interface speed
* @param[in] enable                   - enable rate limit
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS prvCpssPxPortResourcesRateLimitSet
(
    IN GT_U8                           devNum,
    IN GT_PHYSICAL_PORT_NUM            portNum,
    IN CPSS_PORT_SPEED_ENT             speed,
    IN GT_BOOL                         enable
);

#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /*__PRV_CPSS_PX_PORT_IF_MODE_CFG_PIPE_RESOURCE_H*/

