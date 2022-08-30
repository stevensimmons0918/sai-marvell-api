// xpSaiLog.c

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
********************************************************************************/

#include "xpSaiLog.h"
#include "xpSaiUtil.h"
#include "xpsUtil.h"
#include "xpSai.h"

#include <time.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#define XP_SAI_LOG_API_SIZE (SAI_API_MAX)
#define XP_SAI_LOG_FILENAME_SIZE 30
#define XP_SAI_LOG_LEVEL_NAME_SIZE 10
#define XP_SAI_LOG_API_NAME_SIZE 16
#define XP_SAI_LOG_FUNCTION_NAME_SIZE 35

static void xpSaiTimestampPrint(void);

static FILE *saiLogFile = stdout;

static sai_log_level_t saiLogLevelApi[XP_SAI_LOG_API_SIZE];

static const char *saiLogLevelNames[] =
{
    "DEBUG",
    "INFO",
    "NOTICE",
    "WARNING",
    "ERROR",
    "CRITICAL"
};

static const char *saiApiNames[] =
{
    NULL,
    "SWITCH",
    "PORT",
    "FDB",
    "VLAN",
    "VIRTUAL_ROUTER",
    "ROUTE",
    "NEXT_HOP",
    "NEXT_HOP_GROUP",
    "ROUTER_INTERFACE",
    "NEIGHBOR",
    "ACL",
    "HOST_INTERFACE",
    "MIRROR",
    "SAMPLEPACKET",
    "STP",
    "LAG",
    "POLICER",
    "WRED",
    "QOS_MAPS",
    "QUEUE",
    "SCHEDULER",
    "SCHEDULER_GROUP",
    "BUFFERS",
    "HASH",
    "UDF",
    "TUNNEL",
    "L2MC",
    "IPMC",
    "RPF_GROUP",
    "L2MC_GROUP",
    "IPMC_GROUP",
    "MCAST_FDB",
    "BRIDGE",
    "TAM",
    "SEGMENTROUTE",
    "MPLS",
    "DTEL",
    "BFD",
    "ISOLATION_GROUP",
    "NAT",
    "COUNTER",
    "DEBUG_COUNTER",
    "MACSEC",
    "SYSTEM_PORT",
};

//Func: xpSaiLogInit used to set default log settings before any logger
// functionality is used

void xpSaiLogInit(const char *logFile)
{
    uint32_t i = 0;

    if (logFile != NULL)
    {
        saiLogFile = fopen(logFile, "a");
    }
    else
    {
        saiLogFile = stdout;
    }

    //NOTE: SAI interface requires to use SAI_LOG_LEVEL_WARN as default, but
    // currently xpSai implementation uses DEBUG or ERROR messages only.
    for (i = 0; i < ARRAY_SIZE(saiLogLevelApi); i++)
    {
        saiLogLevelApi[i] = XPSAI_DEFAULT_LOG_LEVEL;
    }
}

void xpSaiLogDeInit(void)
{
    struct stat stat1;
    struct stat stat2;

    if (fstat(fileno(saiLogFile), &stat1) < 0)
    {
        return;
    }

    if (fstat(fileno(stdout), &stat2) < 0)
    {
        return;
    }

    if ((stat1.st_dev != stat2.st_dev) || (stat1.st_ino != stat2.st_ino))
    {
        fclose(saiLogFile);
    }
}

//Func: xpSaiLogSetLevel used to set log settings

void xpSaiLogSetLevel(sai_log_level_t level)
{
    uint32_t i = 0;

    for (i = 0; i < ARRAY_SIZE(saiLogLevelApi); i++)
    {
        saiLogLevelApi[i] = level;
    }
}

//Func: xpSaiApiLog internal log function to print out every log level info with
// priority.

void xpSaiApiLog(const sai_api_t api, const sai_log_level_t logLevel,
                 const uint32_t line, const char *file, const char *funcName,
                 const char *msg, ...)
{
    va_list args;
    char fileName[XP_SAI_LOG_FILENAME_SIZE] = {0};

    if (NULL == saiLogFile)
    {
        return;
    }

    if ((_sai_api_extensions_t)api >= MAX_SAI_API_EXTENSION ||
        api < SAI_API_UNSPECIFIED)
    {
        return;
    }

    if (logLevel > SAI_LOG_LEVEL_CRITICAL || logLevel < SAI_LOG_LEVEL_DEBUG)
    {
        return;
    }

    if (logLevel >= saiLogLevelApi[(int) api])
    {
        xpSaiTimestampPrint();
        // prepare to print filename with a line number
        snprintf(fileName, XP_SAI_LOG_FILENAME_SIZE, "%s:%u",
                 (strrchr(file, '/') ? strrchr(file, '/') + 1 : file), line);
        // print prefix
        fprintf(saiLogFile, "SAI: ");

        //print log level
        fprintf(saiLogFile, "%.*s ", XP_SAI_LOG_LEVEL_NAME_SIZE,
                saiLogLevelNames[logLevel]);

        // print api name
        if (saiApiNames[(int) api])
        {
            fprintf(saiLogFile, "%.*s ", XP_SAI_LOG_API_NAME_SIZE, saiApiNames[(int) api]);
        }

        // print additional debug info
        fprintf(saiLogFile, "%.*s %.*s: ",
                XP_SAI_LOG_FILENAME_SIZE, fileName,
                XP_SAI_LOG_FUNCTION_NAME_SIZE, funcName);

        // print message
        va_start(args, msg);
        vfprintf(saiLogFile, msg, args);
        va_end(args);

        fflush(saiLogFile);
    }
}

//Func: xpSaiTimestampPrint, uses to print timestamp before any log triggers

static void xpSaiTimestampPrint(void)
{
    time_t ltime;
    struct tm *tm;

    ltime = time(NULL);
    tm = localtime(&ltime);

    if (tm)
    {
        fprintf(saiLogFile, "%02d:%02d:%02d ",
                tm->tm_hour,
                tm->tm_min,
                tm->tm_sec);
    }
}

sai_status_t sai_log_set(_In_ sai_api_t sai_api_id,
                         _In_ sai_log_level_t sai_log_level)
{
    if ((sai_api_id > SAI_API_MCAST_FDB) || (sai_api_id < SAI_API_UNSPECIFIED))
    {
        return SAI_STATUS_FAILURE;
    }

    if ((sai_log_level > SAI_LOG_LEVEL_CRITICAL) ||
        (sai_log_level < SAI_LOG_LEVEL_DEBUG))
    {
        return SAI_STATUS_FAILURE;
    }

    saiLogLevelApi[sai_api_id] = sai_log_level;
    /*if SWITCH module, change xps level also*/
    if (sai_api_id == SAI_API_SWITCH)
    {
        xpsSetLogLevel((xpLogLevel)sai_log_level);
    }
    return SAI_STATUS_SUCCESS;
}
