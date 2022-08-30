// main.cpp

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
********************************************************************************/

#include "xpsApp.h"
#include "xpPyInc.h"
#include "xpAppSignalHandler.h"
#include "xpsMac.h"
#include "xpsSerdes.h"
#include "xpsScope.h"
#include "cpssHalDumpUtil.h"
#include "cpssHalUtil.h"

#ifdef __cplusplus
extern "C" {
#endif
extern void SetSharedProfileIdx(int id);
extern void SetL3CounterIdBase(int id);
extern xpTimeStamp_t xpTimeStamp[XP_MAX_FUNCTION];

int appMain(int argc, char *argv[])

{
    XP_STATUS status = XP_NO_ERR;
    const char** moduleNames = NULL;
    // char *confFile = NULL;

    GET_TIME_STAMP(xpTimeStamp[XP_BOOTUP_ALL].startTime);

    sdkSetupSignals();

    strncpy(XP_PROGNAME, argv[0], strlen(argv[0]) + 1);  // Save name of executable.
    getEnvValues();                                     // Process environment variables first,
    getOptValues(argc, argv,
                 moduleNames);              // then process command line options => parsing for both client and wm
    initConfigEntryList();                              // initialize entry data structure of configFile
    if ((status = readConfigFile(XP_CONFIG.configFileName)) !=
        XP_NO_ERR)   // then read configuration file.
    {
        printf("config.txt not parsed completely\n");
    }

    XP_CONFIG.withHw ? xpSetSalType(XP_SAL_HW_TYPE) : xpSetSalType(XP_SAL_WM_TYPE);

    if ((status = (XP_STATUS)xpSalInit(xpAppConf.devType)) != XP_NO_ERR)
    {
        printf("xpSalInit failed. RC: %u\n", status);
        return status;
    }

    if (XP_CONFIG.withHw)
    {
#ifndef MAC
        int ret = 0;
        char platformPath[MAX_FILENAME_LEN] = {0};
        xpAppGetExecutableFileDir(XP_PROGNAME, platformPath, MAX_FILENAME_LEN);
        strncat(platformPath, "../../platform",
                MAX_FILENAME_LEN - strlen(platformPath) - 1);
        if (ret)
        {
            printf("PlatformInit() Failed for [%s] with err: %d\n", XP_CONFIG.withHw, ret);
        }
#endif
        // Set device type
        if ((status = xpSalGetDeviceType(0, &xpAppConf.devType)) != XP_NO_ERR)
        {
            printf("Error <dev = 0>: xpSalGetDeviceType failed.\n");
            return XP_ERR_INIT;
        }
    }

    // Set logger value of different modules
    uint8_t logModIndex = 0;
    while (strcmp(xpAppConf.dbgLogInfo[logModIndex].ModName, "") != 0)
    {
        xpsSdkSetLoggerOptions(xpAppConf.dbgLogInfo[logModIndex].xpLogModuleId,
                               xpAppConf.dbgLogInfo[logModIndex].ModName);
        logModIndex++;
    }

    if (strcmp(xpAppConf.logfile, "None") != 0)
    {
        printf("Logging enabled to File: %s\n", xpAppConf.logfile);
        xpsSdkLogToFile(xpAppConf.logfile);
    }
    else
    {
        // connect logger to console
        xpsSdkLogConsole();
    }

    //xpsApp reuses the arguments provided by the user -s and -q
    //compatible with current xpApp design and changes.
    xpsRangeProfileType_t rpType = RANGE_PROFILE_MAX;

    // Set LPM_FDB profile defaults.
    SetSharedProfileIdx(CPSS_HAL_MID_L3_MID_L2_NO_EM);

    // L3 Counters Hw range 0 to 1023.
    SetL3CounterIdBase(0);

    // Creating scopes if scope is enabled.
    xpsScope_t scopeId;
    if (XP_CONFIG.numScope != 0)
    {
        for (scopeId = 0; scopeId < XP_CONFIG.numScope; scopeId++)
        {
            if ((status = (XP_STATUS)xpsScopeInit((xpsInitType_t)scopeId)) != XP_NO_ERR)
            {
                printf("xpsScopeInit failed. RC: %u\n", status);
                return status;
            }
        }
    }

    xpsInitType_t initType = xpAppConf.initType;

    // global sdk initialization : 1st thing to do
    if ((status = xpsAppInit(xpAppConf.devType, rpType, initType)) != XP_NO_ERR)
    {
        printf("xpsAppInit failed. RC: %u\n", status);
        return status;
    }
    printf("xpsAppInit done.\n");

    if (strcmp(XP_CONFIG.shellConfigFile, "None") != 0)
    {
        // confFile = XP_CONFIG.shellConfigFile;
    }
    // Init python interface
    //
    if ((XP_CONFIG.userCli & XP_CLI_USER) && (XP_CONFIG.debugCli))
    {
        xpPyInit();
        xpPyInvoke(0, XP_SHELL_APP, NULL);
    }

    return xpAppMainLoop(XP_CONFIG.initType);
}

int main(int argc, char *argv[])
{
    return (appMain(argc, argv));
}

#ifdef __cplusplus
}
#endif
