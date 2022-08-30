// xpAppL3Uc.h

/*******************************************************************************
* copyright (c) 2021 marvell. all rights reserved. the following file is       *
* subject to the limited use license agreement by and between marvell and you, *
* your employer or other entity on behalf of whom you act. in the absence of   *
* such license agreement the following file is subject to marvell’s standard   *
* limited use license agreement.                                               *
********************************************************************************/

#ifndef _xpAppL3Uc_h_
#define _xpAppL3Uc_h_

#include "xpTypes.h"

XP_STATUS xpAppL3UcPLInit(void);
XP_STATUS xpAppL3UcFLInit(void);
XP_STATUS xpAppL3UcDevInit(xpDevice_t devId, XP_PIPE_MODE mode);

/**
 * \brief Ipv4 Scale Demo.  Typically used with route-centric profile
 *
 * To launch, either use GDB or insert after init finished.  In previous demos, placed call in xpsAppPopulateTables like so:
 *
 * if (configFile.filename == NULL)
 * {
 *      uint32_t egressPorts[6] = {25,26,28,29,30,31};
 *      xpAppL3UcAddPrefix(devId, 24, egressPorts, 6, 3, 15);
 *   ...
 *
 * Then to launch: dist/xpsApp -t <TARGET> -m <SPEED> -q 1 -s 1 -u
 *
 * Note: There currently is no XPS API for discovering profile type.  Selection of txt file is currently hardcoded
 * 
 * \param [in] devId        Device ID
 * \param [in] ingressPort  Ingress Physical Port Number to send traffic into
 * \param [in] egressPorts  Array of Egress Physical Port Numbers to ECMP load balance across
 * \param [in] egressPortSize Number of elements in egressPorts
 * \param [in] numNhGroups  Number of Next Hop Groups
 * \param [in] ecmpSize     Size of ECMP Next Hop Group
 *
 * \return XP_STATUS
 */
XP_STATUS xpAppL3UcAddPrefix (xpDevice_t devId, uint32_t ingressPort, uint32_t* egressPorts, uint32_t egressPortSize, uint32_t numNhGroups, uint32_t ecmpSize);

XP_STATUS xpAppL3UcDisplayPrefixes (xpDevice_t devId);
XP_STATUS xpAppL3UcAddIpv6Prefix (xpDevice_t devId);
XP_STATUS xpAppL3UcNhProgramming(xpDevice_t devId);
#endif  //_xpAppL3Uc_h_
