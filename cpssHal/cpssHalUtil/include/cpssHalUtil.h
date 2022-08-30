/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
*******************************************************************************/

// cpssHalUtil.h


#ifndef _cpssHalUtil_h_
#define _cpssHalUtil_h_
#include "cpssCommonDefs.h"
#include "openXpsEnums.h"
#include "cpss/dxCh/dxChxGen/config/cpssDxChCfgInit.h"
#include "xpTypes.h"
#include "gtEnvDep.h"
#ifdef __cplusplus
extern "C" {
#endif

#define DEFAULT_MAC_SA_TABLE_INDEX 0
/* CPSS LPM_FDB_EM profile index values. */
typedef enum
{
    CPSS_HAL_MID_L3_MID_L2_NO_EM = 1,
    CPSS_HAL_MID_L3_MID_L2_MIN_EM = 2,
    CPSS_HAL_LOW_MAX_L3_MID_LOW_L2_NO_EM = 5,
    CPSS_HAL_MAX_L3_MIN_L2_NO_EM = 9
} CPSS_HAL_LPM_FBM_EM_PROF_ID;

typedef enum
{
    CPSS_HAL_EPORT_TYPE_GLOBAL = 0,
    CPSS_HAL_EPORT_TYPE_L2ECMP,
    CPSS_HAL_EPORT_TYPE_L2DLB
} CPSS_HAL_EPORT_TYPE;


/**
 * \file cpssHalUtil.h
 * \brief This file contains API prototypes and type definitions
 *        for the cpssHalUtil
 */


/**
 * \brief To convert xpsPktCmd to cpssPktCmd
 *
 * \param [in] cmd of type xpsPktCmd_e
 *
 * \return [CPSS_PACKET_CMD_ENT] On success-returns a suitable packet command related to cmd
 *                               On failure-CPSS_PACKET_CMD_DROP_HARD_E
 */

CPSS_PACKET_CMD_ENT xpsConvertPktCmdToCpssPktCmd(xpsPktCmd_e cmd);

/**
 * \brief To convert cpssPktCmd to xpsPktCmd
 *
 * \param [in] cmd of type CPSS_PACKET_CMD_ENT
 *
 * \return [xpsPktCmd_e] On success-returns a suitable packet command related to cmd
 *                       On failure-XP_PKTCMD_DROP
 */

xpsPktCmd_e xpsConvertPktCmdToXpsPktCmd(CPSS_PACKET_CMD_ENT cmd);

/**
 * \brief Print version of Software
 *
 * \param none
 */
void cpssHalPrintVersion(void);

/**
 * \brief To convert GT_STATUS to XP_STATUS
 *
 * \param [in] return value of cpss GT_STATUS
 *
 * \return [XP_STATUS]
 */

XP_STATUS xpsConvertCpssStatusToXPStatus(GT_STATUS status);

/**
 * \brief To convert CPSS INTERFACE to XPS INTERFACE
 *
 * \param [in] intfId
 * \param [in] intfType
 *
 * \return [xpsInterfaceId_t]
 */

xpsInterfaceId_t xpsUtilCpssToXpsInterfaceConvert(uint32_t intfId,
                                                  xpsInterfaceType_e intfType);
uint32_t xpsUtilXpstoCpssInterfaceConvert(xpsInterfaceId_t intfId,
                                          xpsInterfaceType_e intfType);
#ifdef __cplusplus
}
#endif

#endif
