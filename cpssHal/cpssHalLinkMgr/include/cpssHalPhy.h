/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
*******************************************************************************/

// cpssHalPhy.h

#ifndef _cpssHalPhy_h_
#define _cpssHalPhy_h_

#include "xpTypes.h"
//#include "xpEnums.h"
#include "xpsEnums.h"
#include "openXpsMac.h"

#ifdef __cplusplus
extern "C" {
#endif


/**
 * \file cpssHalPhy.h
 * \brief This file contains API prototypes and type definitions
 *        for the Phy (Wrappers)
 */


/**
 * \brief Read PHY register
 *
 * \param [in] devId device Id. Valid values are 0-63
 * \param [in] portNum  Port number.
 * \param [in] pageNum Page Number.
 * \param [in] phyReg  Phy register number.
 * \param [out] reg Phy register value.

 * \return [XP_STATUS] On success XP_NO_ERR.
 */
XP_STATUS cpssHalReadPhyReg(xpsDevice_t devId, uint32_t portNum,
                            uint8_t pageNum,
                            uint8_t phyReg, uint16_t* reg);
/**
 * \brief write PHY register
 *
 * \param [in] devId device Id. Valid values are 0-63
 * \param [in] portNum  Port number.
 * \param [in] pageNum Page Number.
 * \param [in] phyReg  Phy register number.
 * \param [in] reg Phy register value.

 * \return [XP_STATUS] On success XP_NO_ERR.
 */

XP_STATUS cpssHalWritePhyReg(xpsDevice_t devId, uint32_t portNum,
                             uint8_t pageNum, uint8_t phyReg, uint16_t reg);
#ifdef __cplusplus
}
#endif

#endif
