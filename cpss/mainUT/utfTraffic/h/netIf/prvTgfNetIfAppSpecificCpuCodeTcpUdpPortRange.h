/*******************************************************************************
*              (C), Copyright 2001, Marvell International Ltd.                 *
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
* @file prvTgfNetIfAppSpecificCpuCodeTcpUdpPortRange.h
*
* @brief Test to check CPU code assigned based on L4 dest/src port mode (TO_CPU command).
*
* Configurations: RegisterName - TCP/UDP Dest/Src Port Range CPU Code Entry Word 0 and
*                                TCP/UDP Dest/Src Port Range CPU Code Entry Word 1
*
*      At index 3 : configure dst port range : x-y, CPU code 3
*      At index 5 : configure src port range : m-n, CPU code 5
*      At index 7 : configure src+dst port range : a-b, CPU code 7
*
* Note: No overlapping ranges
*
* Traffic:
*      TestCase 1: Send traffic, with dst port in range x-y, src port not in any range
*                  Expect: CPU code 3
*      TestCase 2: Send traffic, with dst port not in any range, src port not in any range
*                  Expect: CPSS_NET_CONTROL_DEST_MAC_TRAP_E
*      TestCase 3: Send traffic, with dst port in range m-n, src port not in any range
*                  Expect: CPSS_NET_CONTROL_DEST_MAC_TRAP_E
*      TestCase 4: Send traffic, with dst port in range a-b, src port not in any range
*                  Expect:  CPU code 7

*      TestCase 5: Send traffic, with dst port not in any range, src port in range x-y
*                  Expect: CPSS_NET_CONTROL_DEST_MAC_TRAP_E
*      TestCase 6: Send traffic, with dst port not in any range, src port in range m-n
*                  Expect: CPU code 5
*      TestCase 7: Send traffic, with dst port not in any range, src port in range a-b
*                  Expect: CPU code 7
*
*      TestCase 8: Send traffic, with dst port in range x-y, src port in range m-n
*                  Expect: CPU code 3
*      TestCase 9: Send traffic, with dst port in range x-y, src port in range a-b
*                  Expect: CPU code 3
*
*      TestCase 10: Send traffic, with dst port in range a-b, src port in range m-n
*                   Expect: CPU code 5
*      TestCase 11: Send traffic, with dst port in range a-b, src port in range a-b
*                   Expect: CPU code 7
*
*
* @version   1
********************************************************************************
*/
#ifndef __prvTgfNetIfAppSpecificCpuCodeTcpUdpPortRange
#define __prvTgfNetIfAppSpecificCpuCodeTcpUdpPortRange

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
* @internal prvTgfNetIfAppSpecificCpuCodeTcpUdpPortRangeTest function
* @endinternal
*
* @brief    Check outgoing packet for CPU code assigned based on L4 dest/src port mode command TO_CPU
*
*/
GT_VOID prvTgfNetIfAppSpecificCpuCodeTcpUdpPortRangeTest
(
    GT_VOID
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfNetIfAppSpecificCpuCodeTcpUdpPortRange */
