// xpsAging.h

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
********************************************************************************/

/**
 * \file xpsAging.h
 * \brief This file contains API prototypes and type definitions
 *        for the XPS Age Manager
 */

#ifndef _xpsAging_h_
#define _xpsAging_h_

#include "openXpsAging.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct xpAgeFifoData
{
    uint32_t fifoNum;
    uint32_t ageMemRowAddr;
    uint32_t ageMemRowData[2];
} xpAgeFifoData;

/**
 * \public
 * \brief This API calls the read Fifo message API to identify aged
 *        out entry. If it will find such entry then API will call
 *        process Fifo message API to call the registered handler
 *        and peform necessary steps on that entry
 *
 * \return XP_STATUS
 *                                       - XP_NO_ERR: Successful execution
 *                                       - XP_ERROR: Otherwise
 */

XP_STATUS xpsAgeFifoNumHandler(xpDevice_t devId, uint32_t fifoNum);



/**
 * \public
 * \brief This API reads the Age Fifo Message from the
 *        global age Configuration.
 *
 * \return XP_STATUS
 *                                       - XP_NO_ERR: Successful execution
 *                                       - XP_ERROR: Otherwise
 */
XP_STATUS xpsReadAgeFifoMessage(xpDevice_t devId, xpAgeFifoData *fifoInfo);

/**
 * \public
 * \brief This API reads the Age Fifo Message from the
 *        global age Configuration.
 *
 * \return XP_STATUS
 *                                       - XP_NO_ERR: Successful execution
 *                                       - XP_ERROR: Otherwise
 */
XP_STATUS xpsProcessAgeFifoMessage(xpDevice_t devId, xpAgeFifoData *fifoInfo);

/**
 * \public
 * \brief This API reads the Aging Mode from the
 *        global age Configuration.
 *
 * \return XP_STATUS
 *                                       - XP_NO_ERR: Successful execution
 *                                       - XP_ERROR: Otherwise
 */
XP_STATUS xpsGetAgingMode(xpDevice_t devId, XP_AGE_MODE_T *ageMode);

/**
 * \public
 * \brief This API reads the Aing Cycle Unit Time from the
 *        global age Configuration.
 *
 * \return XP_STATUS
 *                                       - XP_NO_ERR: Successful execution
 *                                       - XP_ERROR: Otherwise
 */
XP_STATUS  xpsGetAgingCycleUnitTime(xpDevice_t devId, uint32_t* unitTime);

/**
 * \public
 * \brief This API disables Refresh pause global age Configuration.
 *
 * \return XP_STATUS
 *                                       - XP_NO_ERR: Successful execution
 *                                       - XP_ERROR: Otherwise
 */
XP_STATUS xpsDisableRefreshPause(xpDevice_t devId, uint8_t disable);

#ifdef __cplusplus
}
#endif

#endif  //_xpsAging_h_

