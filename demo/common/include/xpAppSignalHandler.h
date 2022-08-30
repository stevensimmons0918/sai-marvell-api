// xpAppSignalHandler.h

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
********************************************************************************/

#ifndef _XP_APPSIGNAL_HANDLER_H_
#define _XP_APPSIGNAL_HANDLER_H_

//#include "xpApp.h"
#include "xpTypes.h"
#include <signal.h>

#define SIGRT_GLIBC_OFFSET  (-2)
#define XP_RT_SIGNAL (SIGRTMIN + 3 + SIGRT_GLIBC_OFFSET)
/**
 * @public
 * \brief signal handler for different type of signals
 *
 *
 * \param [in] sig
 */
void sdkSignalHandler(int sig);

/**
 * @public
 * \brief Setup what signal handler to user and which all
 *        signals to handle
 *
 *
 * \param [in] sig
 */
void sdkSetupSignals();

/**
 *  \brief Creating thread to listen XP_RT_SIGNAL signal
 *
 *  \return [XP_STATUS] status XP_NO_ERR on Success
 */
XP_STATUS registerDmaInterruptHndlr();


extern XP_STATUS xpAppCleanup();

#endif
