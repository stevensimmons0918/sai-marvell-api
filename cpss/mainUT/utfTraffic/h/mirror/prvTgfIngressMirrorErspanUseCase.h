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
* @file prvTgfIngressMirrorErspanUseCase.h
*
* @brief Ingress mirroring ERSPAN Type II and Type III use cases testing
*
* @version   1
********************************************************************************
*/
#ifndef __prvTgfIngressMirrorErspanUseCaseh
#define __prvTgfIngressMirrorErspanUseCaseh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <mirror/prvTgfMirror.h>

#define PRV_UTF_LOG_VERIFY_GT_OK(rc, name)                                   \
    if (GT_OK != (rc))                                                       \
    {                                                                        \
        PRV_UTF_LOG2_MAC("\n[TGF]: %s FAILED, rc = [%d]\n", name, rc);       \
    }

/**
* internal prvTgfLcMirrorSourceFalconIngressErspanConfigSet function
* @endinternal
*
* @brief   Lc Mirror Source Falcon Ingress ERSPAN use case configurations
*/
GT_VOID prvTgfLcMirrorSourceFalconIngressErspanConfigSet
(
    GT_VOID
);

/**
* internal prvTgfLcMirrorTargetFalconIngressErspanConfigSet function
* @endinternal
*
* @brief   Lc Mirror Target Falcon Ingress ERSPAN use case configurations
*/
GT_VOID prvTgfLcMirrorTargetFalconIngressErspanConfigSet
(
    GT_VOID
);

/**
* internal prvTgfLcMirrorTargetFalconIngressErspanIpv6ConfigSet function
* @endinternal
*
* @brief   Lc Mirror Target Falcon Ingress ERSPAN IPv6 use case configurations
*/
GT_VOID prvTgfLcMirrorTargetFalconIngressErspanIpv6ConfigSet
(
    GT_VOID
);

/**
* internal prvTgfLcMirrorSourceFalconEgressErspanConfigSet function
* @endinternal
*
* @brief   Lc Mirror Source Falcon Egress ERSPAN use case configurations
*/
GT_VOID prvTgfLcMirrorSourceFalconEgressErspanConfigSet
(
    GT_VOID
);

/**
* internal prvTgfLcMirrorTargetFalconWithoutVoQEgressErspanConfigSet function
* @endinternal
*
* @brief   Lc Mirror Without VoQ Source Falcon Egress ERSPAN use case configurations
*/
GT_VOID prvTgfLcMirrorTargetFalconWithoutVoQEgressErspanConfigSet
(
    GT_VOID
);

/**
* internal prvTgfLcMirrorTargetFalconEgressErspanConfigSet function
* @endinternal
*
* @brief   Lc Mirror Target Falcon Egress ERSPAN use case configurations
*/
GT_VOID prvTgfLcMirrorTargetFalconEgressErspanConfigSet
(
    GT_VOID
);

/**
* internal prvTgfFalconMirrorSourceFalconIngressErspanConfigSet function
* @endinternal
*
* @brief   Falcon Mirror Source Falcon ERSPAN use case configurations
*/
GT_VOID prvTgfFalconMirrorSourceFalconIngressErspanConfigSet
(
    GT_VOID
);

/**
* internal prvTgfFalconMirrorTargetFalconIngressErspanConfigSet function
* @endinternal
*
* @brief   Falcon Mirror Target Falcon ERSPAN use case configurations
*/
GT_VOID prvTgfFalconMirrorTargetFalconIngressErspanConfigSet
(
    GT_VOID
);

/**
* internal prvTgfFalconMirrorTargetFalconIngressErspanIpv6ConfigSet function
* @endinternal
*
* @brief   Falcon Mirror Target Falcon ERSPAN IPv6 use case configurations
*/
GT_VOID prvTgfFalconMirrorTargetFalconIngressErspanIpv6ConfigSet
(
    GT_VOID
);

/**
* internal prvTgfFalconMirrorSourceFalconEgressErspanConfigSet function
* @endinternal
*
* @brief   Falcon Mirror Source Falcon ERSPAN use case configurations
*/
GT_VOID prvTgfFalconMirrorSourceFalconEgressErspanConfigSet
(
    GT_VOID
);

/**
* internal prvTgfFalconMirrorTargetFalconEgressErspanConfigSet function
* @endinternal
*
* @brief   Falcon Mirror Target Falcon ERSPAN use case configurations
*/
GT_VOID prvTgfFalconMirrorTargetFalconEgressErspanConfigSet
(
    GT_VOID
);

/**
* internal prvTgfFalconIngressMirrorTargetFalconErspanTest function
* @endinternal
*
* @brief   Falcon Mirror Target Falcon ERSPAN use case test
*/
GT_VOID prvTgfFalconIngressMirrorTargetFalconErspanTest
(
    GT_BOOL direction
);

/**
* @internal prvTgfIngressMirrorErspanTest function
* @endinternal
*
* @brief   Ingress mirroring ERSPAN use case test.
*/
GT_VOID prvTgfIngressMirrorErspanTest
(
    GT_VOID
);

/**
* @internal prvTgfFalconIngressMirrorErspanTest function
* @endinternal
*
* @brief   Falcon Ingress mirroring Source Falcon ERSPAN use case test.
*/
GT_VOID prvTgfFalconIngressMirrorErspanTest
(
    GT_VOID
);

/**
* @internal prvTgfIngressMirrorErspanVerification function
* @endinternal
*
* @brief   Ingress mirroring ERSPAN use case verification.
*
* @param[in] isLcMirror   - ERSPAN mirror source is LC or Device
*                           GT_TRUE - LC is mirror source
*                           GT_FALSE - Device is mirror source
* @param[in] direction   - ERSPAN packet is ingress or egress based
*                           GT_TRUE - ERSPAN is egress based
*                           GT_FALSE - ERSPAN is ingress based
* @param[in] protocol    - ERSPAN is ipv4 ir ipv6 based
*                           GT_TRUE - ERSPAN is ipv6 based
*                           GT_FALSE - ERSPAN is ipv4 based
* @param[in] isVoQ        - LC with or without VoQ
*                           GT_TRUE - with VoQ
*                           GT_FALSE - without VoQ
*/
GT_VOID prvTgfIngressMirrorErspanVerification
(
    GT_BOOL isLcMirror,
    GT_BOOL direction,
    GT_BOOL protocol,
    GT_BOOL isVoQ
);

/**
* @internal prvTgfLcMirrorSourceFalconIngressErspanVerification function
* @endinternal
*
* @brief   Lc Source Falcon Ingress mirroring ERSPAN use case verification.
*
* @param[in] srcDev      - ERSPAN mirror source device number
* @param[in] direction   - ERSPAN packet is ingress or egress based
*                           GT_TRUE - ERSPAN is egress based
*                           GT_FALSE - ERSPAN is ingress based
*/
GT_VOID prvTgfLcMirrorSourceFalconIngressErspanVerification
(
    GT_U8 srcDev,
    GT_BOOL direction
);

/**
* @internal prvTgfIngressMirrorSourceFalconErspanTest function
* @endinternal
*
* @brief   Ingress mirroring ERSPAN Source Falcon use case verification.
*
* @param[in] direction   - ERSPAN packet is ingress or egress based
*                           GT_TRUE - ERSPAN is egress based
*                           GT_FALSE - ERSPAN is ingress based
*/
GT_VOID prvTgfIngressMirrorSourceFalconErspanTest
(
    GT_BOOL direction
);

/**
* @internal prvTgfIngressMirrorTargetFalconErspanTest function
* @endinternal
*
* @brief   Ingress mirroring ERSPAN Target Falcon use case verification.
*
* @param[in] direction   - ERSPAN packet is ingress or egress based
*                           GT_TRUE - ERSPAN is egress based
*                           GT_FALSE - ERSPAN is ingress based
* @param[in] isVoQ        - LC with or without VoQ
*                           GT_TRUE - with VoQ
*                           GT_FALSE - without VoQ
*/
GT_VOID prvTgfIngressMirrorTargetFalconErspanTest
(
    GT_BOOL direction,
    GT_BOOL isVoQ
);

/**
* @internal prvTgfEgressMirrorSourceFalconErspanTest function
* @endinternal
*
* @brief   Egress mirroring ERSPAN Source Falcon use case verification.
*/
GT_VOID prvTgfEgressMirrorSourceFalconErspanTest
(
    GT_VOID
);

/**
* @internal prvTgfEgressMirrorTargetFalconErspanTest function
* @endinternal
*
* @brief   Egress mirroring ERSPAN Target Falcon use case verification.
*/
GT_VOID prvTgfEgressMirrorTargetFalconErspanTest
(
    GT_VOID
);

/**
* @internal prvTgfIngressMirrorErspanConfigRestore function
* @endinternal
*
* @brief   Ingress mirroring ERSPAN configurations restore.
*/
GT_VOID prvTgfIngressMirrorErspanConfigRestore
(
    GT_VOID
);

/**
* @internal prvTgfFalconErspanSameDevMirrorTraffic function
* @endinternal
*
* @brief   
*           Test the captures for the configs for ERSPAN.
*           RX:
*               L2 pkt for Vlan5
*           TX:
*               2 pkts on 2 Egr analyzer ports.
*               1. IPv4 encapsulated 
*               2. IPv6 encapsulated 
*/
GT_VOID prvTgfFalconErspanSameDevMirrorTraffic(CPSS_IP_PROTOCOL_STACK_ENT protocol);
/**
* @internal prvTgfFalconErspanSameDevMirrorIpv4ConfigurationSet function
* @endinternal
*
* @brief  Test ingress mirroring functionality with ERSPAN encapsulation
*    configure VLAN;
*    configure an analyzer index, sampling rate and eport;
*    enable Rx mirroring for ingress port;
*    E2phy and IPv4 TS for eport
*    Configure OAM, EPLR & ERMRK for other fields of ERSPAN header
*    configure EPCL rule to trigger IPv4 PHA thread, and pha metadata
*    send traffic and verify expected mirrored traffic on analyzer port;
*/
GT_VOID prvTgfFalconErspanSameDevMirrorIpv4ConfigurationSet();
/**
* @internal prvTgfFalconErspanSameDevMirrorConfigurationReset function
* @endinternal
*
* @brief   ERSPAN same dev mirroring test configurations restore
*/
GT_VOID prvTgfFalconErspanSameDevMirrorConfigurationReset();
/**
* @internal prvTgfFalconErspanSameDevMirrorIpv6ConfigurationSet function
* @endinternal
*
* @brief  Test ingress mirroring functionality with ERSPAN encapsulation
*    configure VLAN;
*    configure an analyzer index, sampling rate and eport;
*    enable Rx mirroring for ingress port;
*    E2phy and IPv6 TS for eport
*    Configure OAM, EPLR & ERMRK for other fields of ERSPAN header
*    configure EPCL rule to trigger IPv4 PHA thread, and pha metadata
*    send traffic and verify expected mirrored traffic on analyzer port;
*/
GT_VOID prvTgfFalconErspanSameDevMirrorIpv6ConfigurationSet();
/**
* @internal prvTgfFalconErspanSameDevMirrorConfigurationReset function
* @endinternal
*
* @brief   ERSPAN same dev mirroring test configurations restore
*/
GT_VOID    prvTgfFalconErspanSameDevMirrorIpv6ConfigurationReset(CPSS_DIRECTION_ENT dir);
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfIngressMirrorErspanUseCaseh */




