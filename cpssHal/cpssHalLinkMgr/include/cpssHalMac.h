/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
*******************************************************************************/

// cpssHalMac.h

#ifndef _cpssHalMac_h_
#define _cpssHalMac_h_

#include "xpTypes.h"
//#include "xpEnums.h"
#include "xpsEnums.h"
#include "openXpsMac.h"
#include "xpsAcl.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \file cpssHalMac.h
 * \brief This file contains API prototypes and type definitions
 *        for the cpssHalLink Mac Manager
 */


/**
 * \brief Get various statistics about Port in xpStatistics structure directly
 *
 * \param [in] devId device Id. Valid values are 0-63
 * \param [in] portNum Port number. Valid values are 0-127.
 * \param [out] stat A structure which holds various statistics of a port. Statistics of counters which
 * falls in range of fromStatNum to toStatNum will only be updated.
 * \return [XP_STATUS] On success XP_NO_ERR.
 *             On failure XP_ERR_INVALID_ARG/XP_ERR_VALUE_OUT_OF_RANGE/XP_ERR_INVALID_ARG
 */

XP_STATUS cpssHalMacGetCounterStats(xpsDevice_t devId, uint32_t portNum,
                                    xp_Statistics *stat);


/**
 * \brief To enable/disable read on counter clear
 *
 * \param [in] devId device id. Valid values are 0-63
 * \param [in] portNum  port number. Valid values are 0 to 127 and 255.
 * \param [in] enable
 *
 * \return [XP_STATUS] On success XP_NO_ERR.
 */
XP_STATUS cpssHalMacSetReadCounterClear(xpsDevice_t devId, uint32_t portNum,
                                        uint8_t enable);

/**
 * \brief cpssHalPrintPortStat print statistics
 *
 * \param [in] devId device id. Valid values are 0-63
 * \param [in] portNum  port number. Valid values are 0 to 127 and 255.
 * \param [in] xp_Statistics
 *
 * \return [XP_STATUS] On success XP_NO_ERR.
 */
XP_STATUS cpssHalMacPrintPortStat(int devId, uint32_t portNum,
                                  xp_Statistics *stat);

/**
 * \brief cpssHalPrintAllPortStatus print status
 *
 * \param [in] devId device id. Valid values are 0-63
 *
 * \return none
 */
void cpssHalMacPrintAllPortStatus(xpsDevice_t devId);

/**
 * \brief To reset MAC Counters.
 *
 * \param [in] devId device id. Valid values are 0-63
 * \param [in] portNum  port number. Valid values are 0 to 127 and 255.
 *
 * \return [XP_STATUS] On success XP_NO_ERR.
 */

XP_STATUS cpssHalMacStatCounterReset(xpsDevice_t devId, uint32_t portNum);

/**
 * \brief Retruns speed capability for AP port.
 *
 * \param [in] devId device id. Valid values are 0-63
 * \param [in] portNum  port number. .
 *
 * \return [XP_STATUS] On success XP_NO_ERR.
 */

XP_STATUS cpssHalMacGetPortFdAbility(xpDevice_t devId, uint32_t portNum,
                                     uint32_t *portAdvertFdAbility);



XP_STATUS cpssHalMacStatCounterCreateAclRuleForV4V6(xpsDevice_t devId,
                                                    uint32_t portNum,
                                                    uint32_t *counterId, xpsAclStage_e direction, xpsAclKeyFlds_t keyType);


XP_STATUS cpssHalMacStatCounterRemoveAclRuleForV4V6(xpsDevice_t devId,
                                                    uint32_t portNum,
                                                    uint32_t counterId, xpsAclStage_e direction, xpsAclKeyFlds_t keyType);


XP_STATUS cpssHalMacGetCounterV4V6Stats(xpsDevice_t devId, uint32_t portNum,
                                        uint32_t counterId_Ingress_v4, uint32_t counterId_Ingress_v6,
                                        uint32_t counterId_Egress_v4, uint32_t counterId_Egress_v6,
                                        uint64_t *v4TxPkts,  uint64_t *v4TxBytes,  uint64_t *v4RxPkts,
                                        uint64_t *v4RxBytes,
                                        uint64_t *v6TxPkts,  uint64_t *v6TxBytes,  uint64_t *v6RxPkts,
                                        uint64_t *v6RxBytes);


#ifdef __cplusplus
}
#endif

#endif
