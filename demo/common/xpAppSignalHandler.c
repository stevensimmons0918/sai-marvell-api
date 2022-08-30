// xpAppSignalHandler.c

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
********************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "xpAppSignalHandler.h"
#include "xpsXpImports.h"
extern void xpsSetExitStatus();
#ifndef MAC
extern pthread_t
gSigThread;   /* global instance for XP_RT_SIGNAL signal handling thread */
#endif

void sdkSetupSignals()
{
    struct sigaction sigIntHandler;
    sigIntHandler.sa_handler = sdkSignalHandler;
    sigIntHandler.sa_flags = 0;
    sigemptyset(&sigIntHandler.sa_mask);
    sigaction(SIGINT, &sigIntHandler, NULL);

    sigset_t sig;
    sigemptyset(&sig);
    sigaddset(&sig, SIGTTOU);
    sigaddset(&sig, SIGTTIN);
    sigprocmask(SIG_BLOCK, &sig, 0);
}

void sdkSignalHandler(int sig)
{
    if (sig == SIGINT || sig == SIGKILL || sig == SIGTERM || sig == SIGABRT
        || sig == SIGFPE || sig == SIGILL  || sig == SIGQUIT || sig == SIGSEGV
        || sig == SIGHUP || sig == SIGUSR2)
    {

        printf("Handling signal: %1d \n", sig);
        xpsSetExitStatus();
#ifndef MAC
        if (gSigThread)
        {
            if (!pthread_kill(gSigThread, 0))
            {
                pthread_cancel(gSigThread);
                gSigThread = 0;
            }
        }
#endif
    }
    else
    {
        fprintf(stderr, "Received signal %1d.\n", sig);
    }
}

/**
 * @brief Creating thread to listen XP_RT_SIGNAL signal
 * @param [in] devid - Device id
 *
 * @return [XP_STATUS] status XP_NO_ERR on Success
 */

XP_STATUS registerDmaInterruptHndlr()
{
#ifndef MAC
    XP_STATUS ret = XP_NO_ERR;
    pthread_t sigThread = 0UL;
    sigset_t mask;
    xpEventSignalHandler signalHndlr;

    /*Block XP_RT_SIGNAL singal for main thread*/
    sigemptyset(&mask);
    sigaddset(&mask, XP_RT_SIGNAL);
    pthread_sigmask(SIG_BLOCK, &mask, NULL);
    signalHndlr = NULL;

    /*Create separate thread to listen XP_RT_SIGNAL signal continuously*/
    if ((xpGetSalType() != XP_SAL_WM_TYPE) &&
        (xpGetSalType() != XP_SAL_KERN_WM_TYPE))
    {
        ret = xpSalThreadCreate(&sigThread, NULL, signalHndlr, NULL);
        if (ret == 0)
        {
            gSigThread = sigThread;
        }
    }
#endif
    return XP_NO_ERR;
}

