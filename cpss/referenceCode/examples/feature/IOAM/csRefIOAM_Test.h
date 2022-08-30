/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
*   csRefIOAM_Test.h
*
* DESCRIPTION:
*  This file contains test scenarios for the IOAM feature, using the APIs in csRefIOAM.h.h
*
*         Test-1
*           1.1. configure switch as IOAM node, role of node (Ingress/Transit/Egress) depends on MAC DA of incomming packet
*           1.2. Transmit one non-IOAM packet to port 1 as ingress packet to Ingress node
*           1.3. Check that packet is received on port 2 and on no other ports, verify its IOAM and tunnel fields
*
*         Test-2
*           2.1. Same configuration as in test 1..
*           2.2.Transmit one IOAM packet with 1 record to port 1 as ingress packet to Transit node
*           2.3. Check that packet is received on port 2 and on no other ports, verify its IOAM (two records) and tunnel (MAC DA changed) fields
*
*         Test-3....
*           3.1. Same configuration as in test 1..
*           3.2.Transmit one IOAM packet with 2 records to port 1 as ingress packet to Egress node
*           3.3. Check that packet received on port 3 and on port 2, verify IOAM (three records) and tunnel (IPv6 GRE header added) fields on packet received on port 3
*         Test-4....
*         Test-5....
*
************************* PACKETS USED IN THE TEST****************************************************************
*         all below packets are according to default packet fields, they can be used as is if default values are not changed
*
*         1. Ingress packet - packet that enters Ingress IOAM node, it has no IOAM header
*
*       0000   00 00 00 00 34 02 00 00 00 00 11 11 00 00 00 00
*       0010   00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
*       0020   00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
*       0030   00 00 00 00 00 00 00 00 00 00 00 00
*
*            Parsing:
*                     00 00 00 00 34 02 - default Ingress MAC DA
*
*         2. Packet with 1 IOAM record - packet that exits Ingress IOAM node, and the packet that enters the Transit node
*
*
*       0000   00 ee 11 22 33 55 00 11 12 13 14 15 81 00 00 01
*       0010   86 dd 60 00 00 00 00 6c 11 64 00 00 00 00 00 00
*       0020   00 00 55 55 55 55 77 77 77 77 00 00 00 00 00 00
*       0030   00 00 22 22 22 22 33 33 33 33 00 13 12 b6 00 6c
*       0040   00 00 0f 00 00 81 00 00 98 80 01 08 00 01 fa 00
*       0050   80 00 0a 00 00 0f 00 00 00 24 00 00 00 cc 0c 5f
*       0060   a5 f8 73 a0 5a 08 00 00 00 00 00 00 00 00 34 02
*       0070   00 00 00 00 11 11 00 00 00 00 00 00 00 00 00 00
*       0080   00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
*       0090   00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
*       00a0   00 00 00 00 00 00 ce 98 f6 8c
*
*            Parsing:
*                     00 ee 11 22 33 55 00 - default Transit MAC DA
*                     86 dd -                       IPv6 ethertype
*================ Tunnel Header Start =========================
*                     60 00 00 00 00 6c 11 64 00 00 00 00 00 00 00 00 55 55 55 55 77 77 77 77 00 00 00 00 00 00 00 00 22 22 22 22 33 33 33 33 - IPv6 header, where:
*                                                11                                                                                                                                                     - Next header protocol (17 in decimal = UDP)
*                                                         00 00 00 00 00 00 00 00 55 55 55 55 77 77 77 77                                                                         - destination IP
*                                                                                                                                 00 00 00 00 00 00 00 00 22 22 22 22 33 33 33 33 - source IP
*                     00 13 12 b6 00 6c 00 00 - UDP header, where:
*                              12 b6                    - UDP destination port (4790 in decimal)
*                     0f 00 00 81 00 00 98 80 - VXLAN-GPE, where
*                                  81                   - next Protocol = IOAM
*                     01 08 00 01 fa 00 80 00 - IOAM header, where:
*                     01                               -  IOAM type
*                         08                            - IOAM Header Length in units of 4 bytes
*                                       fa 00          - trace type bitmap:  (Hop_lim and Node ID: YES; Ingress egress inf: YES; Timestamp sec: YES; Timestamp ns: YES; Delay: YES; appData: NO; queue depth: YES; )
*                     0a 00 00 0f 00 00 00 24 00 00 00 cc 0c 5f a5 f8 73 a0 5a 08 00 00 00 00 - IOAM record I, where:
*                     0a                                                                                                     - hop limit
*                         00 00 0f                                                                                         - Node ID
*                                     00 00                                                                                 - Ingress interface 
*                                              00 24                                                                        - Egress interface (port 36)
*                                                       00 00 00 cc                                                       - timestamp (seconds, = 204 seconds)
*                                                                         0c 5f a5 f8                                      - timestamp (nano- seconds)
*                                                                                         73 a0 5a 08                    - delay in ns
*                                                                                                           00 00 00 00  - TX queue depth at time of transmission (0 in our simple case)
*================ Tunnel Header End =========================
*                   00 00 00 00 34 02 00 00 00 00 11 11 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 ce 98 f6 8c 
*                                                                                             - original packet ("passanger") and checksum
*
*
*         3. Packet with 2 IOAM records - packet that exits Transit IOAM node, and the packet that enters the Egress node
*
*       0000   00 aa bb cc dd 01 00 11 12 13 14 15 86 dd 60 00
*       0010   00 00 00 84 11 64 00 00 00 00 00 00 00 00 55 55
*       0020   55 55 77 77 77 77 00 00 00 00 00 00 00 00 22 22
*       0030   22 22 33 33 33 33 00 13 12 b6 00 84 00 00 0f 00
*       0040   00 81 00 00 98 80 01 0e 00 01 fa 00 80 00 64 00
*       0050   00 11 00 00 00 24 00 00 4b 89 00 c7 77 28 7f 38
*       0060   88 d8 00 00 00 00 0a 00 00 0f 00 00 00 24 00 00
*       0070   00 cc 0c 5f a5 f8 73 a0 5a 08 00 00 00 00 00 00
*       0080   00 00 34 02 00 00 00 00 11 11 00 00 00 00 00 00
*       0090   00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
*       00a0   00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
*       00b0   00 00 00 00 00 00 00 00 00 00 d7 bf 76 18
*
*            Parsing:
*                     00 aa bb cc dd 01      - MAC DA - default Transit ARP address
*                     86 dd -                       IPv6 ethertype
*================ Tunnel Header Start =========================
*                     60 00 00 00 00 a4 11 64 00 00 00 00 00 00 00 00 55 55 55 55 77 77 77 77 00 00 00 00 00 00 00 00 22 22 22 22 33 33 33 33 - IPv6 header, where:
*                                                11                                                                                                                                                     - Next header protocol (17 in decimal = UDP)
*                                                    64                                                                                                                                                 - TTL
*                                                         00 00 00 00 00 00 00 00 55 55 55 55 77 77 77 77                                                                         - destination IP
*                                                                                                                                 00 00 00 00 00 00 00 00 22 22 22 22 33 33 33 33 - source IP
*                     00 13 12 b6 00 84 00 00 - UDP header, where:
*                              12 b6                    - UDP destination port (4790 in decimal)
*                     0f 00 00 81 00 00 98 80 - VXLAN-GPE, where
*                                  81                   - next Protocol = IOAM
*                     01 0e 00 01 fa 00 80 00 - IOAM header, where:
*                     01                               -  IOAM type
*                         0e                            - IOAM Header Length in units of 4 bytes
*                                       fa 00          - trace type bitmap:  (Hop_lim and Node ID: YES; Ingress egress inf: YES; Timestamp sec: YES; Timestamp ns: YES; Delay: YES; appData: NO; queue depth: YES; )
*                     64 00 00 11 00 00 00 24 00 00 4b 89 00 c7 77 28 7f 38 88 d8 00 00 00 00 - IOAM record I, where:
*                     64                                                                                                     - hop limit (copied from IP header)
*                         00 00 11                                                                                         - Node ID
*                                     00 00                                                                                 - Ingress interface 
*                                              00 24                                                                        - Egress interface (port 36)
*                                                       00 00 4b 89                                                       - timestamp (seconds, = 204 seconds)
*                                                                         00 c7 77 28                                     - timestamp (nano- seconds)
*                                                                                           7f 38 88 d8                   - delay in ns
*                                                                                                             00 00 00 00 - TX queue depth at time of transmission (0 in our simple case)
*                     0a 00 00 0f 00 00 00 24 00 00 00 cc 0c 5f a5 f8 73 a0 5a 08 00 00 00 00 - IOAM record II (same as record I in "Packet with 1 IOAM record"), where:
*                     0a                                                                                                     - hop limit
*                         00 00 0f                                                                                         - Node ID
*                                     00 00                                                                                 - Ingress interface 
*                                              00 24                                                                        - Egress interface (port 36)
*                                                       00 00 00 cc                                                       - timestamp (seconds, = 204 seconds)
*                                                                         0c 5f a5 f8                                      - timestamp (nano- seconds)
*                                                                                         73 a0 5a 08                    - delay in ns
*                                                                                                           00 00 00 00  - TX queue depth at time of transmission (0 in our simple case)
*================ Tunnel Header End =========================
*                   00 00 00 00 34 02 00 00 00 00 11 11 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 ce 98 f6 8c 
*                                                                                             - original packet ("passanger") and checksum
*
*
*         4. Packet with 3 IOAM records - packet that exits Egress IOAM node to TX analyzer port. IPv6 IOAM header is encapsulated in IPV6 GRE tunnel- double tunnel encapsulation
*
*       0000   00 99 88 77 66 55 00 11 12 13 14 15 86 dd 60 00
*       0010   00 00 00 d6 2f 80 00 00 00 00 00 00 00 00 79 79
*       0020   79 79 de de de de 00 00 00 00 00 00 00 00 58 58
*       0030   58 58 31 31 31 31 00 00 86 dd 00 aa bb cc dd 01
*       0040   00 11 12 13 14 15 86 dd 60 00 00 00 00 9c 11 64
*       0050   00 00 00 00 00 00 00 00 55 55 55 55 77 77 77 77
*       0060   00 00 00 00 00 00 00 00 22 22 22 22 33 33 33 33
*       0070   00 13 12 b6 00 9c 00 00 0f 00 00 81 00 00 98 80
*       0080   01 14 00 01 fa 00 80 00 64 00 00 13 00 00 00 24
*       0090   00 00 4f b6 00 87 a0 18 7f 78 5f e8 00 00 00 00
*       00a0   64 00 00 11 00 00 00 24 00 00 4b 89 00 c7 77 28
*       00b0   7f 38 88 d8 00 00 00 00 0a 00 00 0f 00 00 00 24
*       00c0   00 00 00 cc 0c 5f a5 f8 73 a0 5a 08 00 00 00 00
*       00d0   00 00 00 00 34 02 00 00 00 00 11 11 00 00 00 00
*       00e0   00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
*       00f0   00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
*       0100   00 00 00 00 00 00 00 00 00 00 00 00 59 09 07 70
*
*            Parsing:
*                     00 99 88 77 66 55      - MAC DA - default Monitor MAC DA
*                     86 dd -                       IPv6 ethertype
*================ Tunnel Header Start =========================
*                     60 00 00 00 00 d6 2f 80 00 00 00 00 00 00 00 00 79 79 79 79 de de de de 00 00 00 00 00 00 00 00 58 58 58 58 31 31 31 31 - IPv6 header, where:
*                                                2f                                                                                                                                                     - Next header protocol (47 in decimal = GRE)
*                                                    80                                                                                                                                                 - TTL
*                                                         00 00 00 00 00 00 00 00 79 79 79 79 de de de de                                                                         - destination IP
*                                                                                                                                 00 00 00 00 00 00 00 00 58 58 58 58 31 31 31 31 - source IP
*                     00 00 86 dd                                                                                                                                                                   - GRE for IPv6
*                     00 aa bb cc dd 01 00 11 12 13 14 15                                                                                                                               - MAC of IOAM packet ("Packet with 2 IOAM records")
*                     60 00 00 00 00 a4 11 64 00 00 00 00 00 00 00 00 55 55 55 55 77 77 77 77 00 00 00 00 00 00 00 00 22 22 22 22 33 33 33 33 - IPv6 header of IOAM packet ("Packet with 2 IOAM records"), where:
*                                                11                                                                                                                                                     - Next header protocol (17 in decimal = UDP)
*                                                    64                                                                                                                                                 - TTL
*                                                         00 00 00 00 00 00 00 00 55 55 55 55 77 77 77 77                                                                         - destination IP
*                                                                                                                                 00 00 00 00 00 00 00 00 22 22 22 22 33 33 33 33 - source IP
*                     00 13 12 b6 00 84 00 00 - UDP header, where:
*                              12 b6                    - UDP destination port (4790 in decimal)
*                     0f 00 00 81 00 00 98 80 - VXLAN-GPE, where
*                                  81                   - next Protocol = IOAM
*                     01 0e 00 01 fa 00 80 00 - IOAM header, where:
*                     01                               -  IOAM type
*                         0e                            - IOAM Header Length in units of 4 bytes
*                                       fa 00          - trace type bitmap:  (Hop_lim and Node ID: YES; Ingress egress inf: YES; Timestamp sec: YES; Timestamp ns: YES; Delay: YES; appData: NO; queue depth: YES; )
*                     64 00 00 13 00 00 00 24 00 00 4f b6 00 87 a0 18 7f 78 5f e8 00 00 00 00 - IOAM record I, where
*                     64                                                                                                     - hop limit (copied from IP header)
*                         00 00 13                                                                                         - Node ID
*                                     00 00                                                                                 - Ingress interface 
*                                              00 24                                                                        - Egress interface (port 36)
*                                                       00 00 4f b6                                                       - timestamp (seconds, = 204 seconds)
*                                                                         00 87 a0 18                                     - timestamp (nano- seconds)
*                                                                                           7f 78 5f e8                   - delay in ns
*                                                                                                             00 00 00 00 - TX queue depth at time of transmission (0 in our simple case)
*
*                     64 00 00 11 00 00 00 24 00 00 4b 89 00 c7 77 28 7f 38 88 d8 00 00 00 00 - IOAM record II (same as record I in "Packet with 2 IOAM records"), where:
*                     64                                                                                                     - hop limit (copied from IP header)
*                         00 00 11                                                                                         - Node ID
*                                     00 00                                                                                 - Ingress interface 
*                                              00 24                                                                        - Egress interface (port 36)
*                                                       00 00 4b 89                                                       - timestamp (seconds, = 204 seconds)
*                                                                         00 c7 77 28                                     - timestamp (nano- seconds)
*                                                                                           7f 38 88 d8                   - delay in ns
*                                                                                                             00 00 00 00 - TX queue depth at time of transmission (0 in our simple case)
*                     0a 00 00 0f 00 00 00 24 00 00 00 cc 0c 5f a5 f8 73 a0 5a 08 00 00 00 00 - IOAM record III (same as record I in "Packet with 1 IOAM record"), where:
*                     0a                                                                                                     - hop limit
*                         00 00 0f                                                                                         - Node ID
*                                     00 00                                                                                 - Ingress interface 
*                                              00 24                                                                        - Egress interface (port 36)
*                                                       00 00 00 cc                                                       - timestamp (seconds, = 204 seconds)
*                                                                         0c 5f a5 f8                                      - timestamp (nano- seconds)
*                                                                                         73 a0 5a 08                    - delay in ns
*                                                                                                           00 00 00 00  - TX queue depth at time of transmission (0 in our simple case)
*================ Tunnel Header End =========================
*                   00 00 00 00 34 02 00 00 00 00 11 11 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 ce 98 f6 8c 
*                                                                                             - original packet ("passanger") and checksum
*
* FILE REVISION NUMBER:
*       $Revision: 1.0 $
*
*******************************************************************************/

#include "csRefIOAM.h"
#include "../../infrastructure/csRefSysConfig/csRefGlobalSystemInfo.h"

#include <gtOs/gtOsMem.h>
#include <gtOs/gtOsInet.h>

#include <cpss/generic/cpssTypes.h>
#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/generic/cpssCommonDefs.h>


/**
* @internal csRefIoamNodeConfig function
* @endinternal
*
* @brief   The function creates configuration for IOAM node. the switch can serve as either ingress, transit or egress node, nodes are recognized by
*             MAC DA of incoming packets
*
* @note   APPLICABLE DEVICES:       Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum         -  device number
* @param[in] ingressPort        port number where IOAM packet enters
* @param[in] egressPort         port number where IOAM packet egresses
* @param[in] analyzerTxPort   port number where IOAM packet is copied to a monitoring interface when switch works as an egress node
* @param[in] traceTypeBmp - trace bitmap in IOAM header - defines what type of information will be added by the node to the IOAM header
*
* @retval GT_OK                                   - on success
* @retval GT_BAD_PARAM                     - on wrong parameters
* @retval GT_OUT_OF_RANGE                - parameter value more then HW bit field
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                                 - otherwise
*
* @note  demostrates configuration for IOAM using PHA; Same ingress port serves for any IOAM node role
*          use like: csRefIoamNodeConfig 0,0,36,32,0xfa00
*
*/
GT_STATUS csRefIoamNodeConfig
(
    IN GT_U8                devNum,
    IN GT_PHYSICAL_PORT_NUM ingressPort,
    IN GT_PHYSICAL_PORT_NUM egressPort,
    IN GT_PHYSICAL_PORT_NUM analyzerTxPort,
    IN GT_U32               traceTypeBmp
);

/**
* @internal csRefIoamNodeConfigClear function
* @endinternal
*
* @brief   The function cleans all configurations created by csRefIoamNodeConfigCreate .
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum           - device number
*
* @retval GT_OK                                  - on success
* @retval GT_BAD_PARAM                     - on wrong parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                                 - otherwise
*
* @note
*
*/
GT_STATUS csRefIoamNodeConfigClear
(
  IN GT_U8         devNum
);

