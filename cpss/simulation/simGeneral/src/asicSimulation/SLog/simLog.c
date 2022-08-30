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
* @file simLog.c
*
* @brief simulation logger main file
*
* @version   26
********************************************************************************
*/
#include <os/simTypesBind.h>
#include <stdarg.h>
#include <stdio.h>
#include <os/simTypesBind.h>
#include <asicSimulation/SCIB/scib.h>
#include <asicSimulation/SInit/sinit.h>
#include <asicSimulation/SLog/simLog.h>
#include <asicSimulation/SKernel/suserframes/snet.h>
#include <asicSimulation/SLog/simLogInfoTypePacket.h>
#include <common/Version/sstream.h>
#include <asicSimulation/SEmbedded/simFS.h>

#ifdef GM_USED
static GT_BOOL gmLogSupport = GT_TRUE;
#else
static GT_BOOL gmLogSupport = GT_FALSE;
#endif

#ifdef _VISUALC
/*
MSVC++ 10.0  _MSC_VER = 1600
MSVC++ 9.0   _MSC_VER = 1500
MSVC++ 8.0   _MSC_VER = 1400
MSVC++ 7.1   _MSC_VER = 1310
MSVC++ 7.0   _MSC_VER = 1300
MSVC++ 6.0   _MSC_VER = 1200
MSVC++ 5.0   _MSC_VER = 1100
*/

#if _MSC_VER >= 1600 /* from VC 10 */
    #define  strlwr _strlwr
#endif

#endif /*_VISUALC*/

/* FLAG that when = GT_FALSE --> will allow to generate LOG with 'redundant'
   meta data info , to have easy 'compare' wit legacy LOG files !!!

   see function  simLogBypassCompressedMetadataInfoSet()
*/
static GT_BOOL supportCompressedMetadataInfo = GT_TRUE;

static GT_U32   initDone = 0;
/******************** config vars **********************/
/* output config */
GT_BOOL simLogOutputToLogfile = GT_FALSE;
GT_BOOL simLogOutputToStdout  = GT_FALSE;

/* logger file pointer */
GT_CHAR simLogFileName[255] = "simLog.log";

/* info type flags */
GT_BOOL simLogInfoTypePacket = GT_FALSE;
GT_BOOL simLogInfoTypeDevice = GT_FALSE;
GT_BOOL simLogInfoTypeTcam   = GT_FALSE;
GT_BOOL simLogInfoTypeMemory = GT_FALSE;

/* thread filters */
GT_BOOL simLogFilterTypeCpuApplication           = GT_FALSE;
GT_BOOL simLogFilterTypeCpuIsr                   = GT_TRUE;/* by default filter enabled to remove time driven differences between 2 logs */
GT_BOOL simLogFilterTypePpAgingDaemon            = GT_TRUE;/* by default filter enabled to limit memory access dump to log */
GT_BOOL simLogFilterTypePpPipeProcessingDaemon   = GT_FALSE;
GT_BOOL simLogFilterTypePpPipeSdmaQueueDaemon    = GT_FALSE;
GT_BOOL simLogFilterTypePpPipeOamKeepAliveDaemon = GT_TRUE;/* by default filter enabled to limit memory access dump to log */
GT_BOOL simLogFilterTypePpPipeGeneralPurpose     = GT_TRUE;/* by default filter enabled to limit non related 'packet walkthrough' */
GT_BOOL simLogFilterTypePpPipeSoftReset          = GT_FALSE;
GT_BOOL simLogFilterTypePreqSrfDaemon            = GT_FALSE;

#define STRING_FILTERS_NUM_CNS  16
/* strings that allow to add only lines that 'contain' it */
static GT_CHAR simLogFilterAllowedSpecificString[STRING_FILTERS_NUM_CNS][256] = {{0}};

/* strings (function names) that allow to add only lines from those functions (function names that 'contain' those names)*/
static GT_CHAR simLogFilterAllowedSpecificFunction[STRING_FILTERS_NUM_CNS][256] = {{0}};

/* strings (unit names) that allow to add only lines from those units (unit names that 'exact match' those names)*/
static SIM_LOG_FRAME_UNIT_ENT simLogFilterAllowedSpecificUnits[STRING_FILTERS_NUM_CNS] = {0};

/* strings (packet command names) that DENY to add lines to log (command names that 'exact match' those names)*/
static SIM_LOG_FRAME_COMMAND_TYPE_ENT simLogFilterDeniedSpecificCommands[STRING_FILTERS_NUM_CNS] = {0};


/* dev/port group filter */
SIM_LOG_DEV_PORT_GROUP_FILTERS_STC simLogDevPortGroupFilters[SIM_LOG_MAX_DF] = {{0}};

/****************** externs vars **********************/
GT_STATUS simLogTaskInit(void);
GT_STATUS simLogTaskOpenFile(IN const char *fname);
GT_STATUS simLogTaskCloseFile(void);
GT_BOOL   simLogTaskFileOpened(void);
GT_STATUS simLogTaskAddLog(IN const char *str);
/****************** private vars **********************/
/* indication if the logger should hold line numbers for the info printed to it.
by default --> do not add line numbers.
because the line numbers cause to 'differences' between logs from
2 different image revisions in parts that did not changed but only pushed up/down
in the line number.
*/
static GT_U32   loggerUseLineNumber = 0;
/****************** macros ****************************/
#define GET_INI_LOG_VALUE_MAC(valName)                        \
            SIM_OS_MAC(simOsGetCnfValue)("log", valName,      \
                SIM_OS_CNF_FILE_MAX_LINE_LENGTH_CNS, paramStr)

#define GET_BOOL_VALUE_MAC(iniVal, var)   \
    if(GET_INI_LOG_VALUE_MAC(iniVal))     \
        sscanf(paramStr, "%u", &var);

#define INFO_TYPE_FILTER_CASE_MAC(enumVal, configVar)\
        case enumVal:                                \
            if(!configVar)                           \
                return GT_OK;                        \
            break;

#define THREAD_FILTER_CASE_MAC(enumVal, configVar)  \
        case enumVal:                               \
            if(configVar)                           \
                return GT_OK;                       \
            taskTypeName = #enumVal;                \
            break;

/* maximal number of devices in INI file for log filters parsing */
#define MAX_DEV_NUM         32

/* maximal number of devices' port groups in INI file for parsing */
#define MAX_PORT_GROUP_NUM    8

#define TASK_TYPE_FORMAT_CNS "\n" "taskType[ %s ] \n"

static char  *commandNameArr[SIM_LOG_FRAME_COMMAND_TYPE___LAST___E + 1] =
{
    STR(SIM_LOG_FRAME_COMMAND_TYPE___MUST_BE_FIRST___E),

    STR(SIM_LOG_FRAME_COMMAND_TYPE_GENERAL_E),
    STR(SIM_LOG_FRAME_COMMAND_TYPE_FROM_CPU_E),

    /* must be last one */
    NULL
};


static char  *unisNameArr[SIM_LOG_FRAME_UNIT___ALLOW_ALL_UNITS___E + 1] =
{
    STR(SIM_LOG_FRAME_UNIT___MUST_BE_FIRST___E),

    STR(SIM_LOG_FRAME_UNIT_PHY_INGRESS_E)   ,
    STR(SIM_LOG_FRAME_UNIT_PHY_EGRESS_E)    ,

    STR(SIM_LOG_FRAME_UNIT_PORT_MAC_INGRESS_E),
    STR(SIM_LOG_FRAME_UNIT_RXDMA_E),
    STR(SIM_LOG_FRAME_UNIT_TTI_E),
    STR(SIM_LOG_FRAME_UNIT_PPU_E),
    STR(SIM_LOG_FRAME_UNIT_IPCL_E),
    STR(SIM_LOG_FRAME_UNIT_L2I_E),
    STR(SIM_LOG_FRAME_UNIT_IPVX_E),
    STR(SIM_LOG_FRAME_UNIT_IOAM_E),
    STR(SIM_LOG_FRAME_UNIT_SMU_E),
    STR(SIM_LOG_FRAME_UNIT_IPLR_E),
    STR(SIM_LOG_FRAME_UNIT_MLL_E),
    STR(SIM_LOG_FRAME_UNIT_EQ_E),
    STR(SIM_LOG_FRAME_UNIT_PIPE_PCP_E),

    STR(SIM_LOG_FRAME_UNIT_INGRESS_PIPE_E),/* the whole ingress pipe */

    /* egress Pipe */
    STR(SIM_LOG_FRAME_UNIT_EGF_EFT_SHT_QAG_E),
    STR(SIM_LOG_FRAME_UNIT_TXQ_E),
    STR(SIM_LOG_FRAME_UNIT_HA_E),
    STR(SIM_LOG_FRAME_UNIT_EPCL_E),
    STR(SIM_LOG_FRAME_UNIT_TRAFFIC_MANAGER_QUEUE_MAPPER_E),
    STR(SIM_LOG_FRAME_UNIT_EPLR_E),
    STR(SIM_LOG_FRAME_UNIT_TRAFFIC_MANAGER_DROP_AND_STATISTICS_E),
    STR(SIM_LOG_FRAME_UNIT_ERMRK_E),/*final egress remark*/
    STR(SIM_LOG_FRAME_UNIT_EREP_E),
    STR(SIM_LOG_FRAME_UNIT_PREQ_E),
    STR(SIM_LOG_FRAME_UNIT_TRAFFIC_MANAGER_ENGINE_E),
    STR(SIM_LOG_FRAME_UNIT_TXDMA_E),
    STR(SIM_LOG_FRAME_UNIT_TXFIFO_E),
    STR(SIM_LOG_FRAME_UNIT_PORT_MAC_EGRESS_E),

    STR(SIM_LOG_FRAME_UNIT_PHA_E),

    STR(SIM_LOG_FRAME_UNIT_EGRESS_PIPE_E),/* the whole egress pipe */

    /* general units */
    STR(SIM_LOG_FRAME_UNIT_CNC_E),
    STR(SIM_LOG_FRAME_UNIT_TCAM_E),

    STR(SIM_LOG_FRAME_UNIT_GENERAL_E),/* ALL the general units */

    /* must be last one */
    NULL
};

static void simLogLinkStateCatchUp(void);
static void simLogSlanCatchUp(void);

/* compress file name from:
   x:\cpss\sw\prestera\simulation\simdevices\src\asicsimulation\skernel\suserframes\snetcheetahingress.c
   To:
   snetcheetahingress.c */
static GT_CHAR * compressFileName
(
    IN GT_CHAR const *fileName
)
{
    GT_CHAR *tempName;
    GT_CHAR *compressedFileName = (GT_CHAR *)fileName;

    /* check to skip path defined by '\' for windows stile path */
    while(NULL != (tempName = strchr(compressedFileName,'\\')))
    {
        compressedFileName = tempName+1;
    }

    /* check to skip path defined by '/' for linux stile path */
    while(NULL != (tempName = strchr(compressedFileName,'/')))
    {
        compressedFileName = tempName+1;
    }

    return compressedFileName;
}

/* check strings filters :
INPUT:
    formatStringPtr - string to check if pass the filter
    filtersArr - the array of filters.(16 filters of up to 256 characters)

RETURN:
    GT_TRUE - need to filter the message
    GT_FALSE - do not filter the message

*/
static GT_BOOL stringFiltersCheck(
    IN const GT_CHAR *formatStringPtr,
    IN GT_CHAR       filtersArr[STRING_FILTERS_NUM_CNS][256]
)
{
    GT_U32 ii;
    GT_U32  jj;
    GT_CHAR *currentStringPtr;
    GT_BOOL atLeastOneFilter = GT_FALSE;
    GT_BOOL matchFound = GT_FALSE;
    GT_U32  messageLen;
    GT_U32  allowedStringLen;

    for(jj = 0 ; jj < STRING_FILTERS_NUM_CNS ; jj++)
    {
        currentStringPtr = filtersArr[jj];
        if(currentStringPtr[0])
        {

            if(formatStringPtr == NULL)
            {
                /* there is at least one filter that is set , but there is no
                   ability to check it if the message is ok ... so drop it */
                return GT_TRUE;
            }

            allowedStringLen = (GT_U32)strlen(currentStringPtr);
            messageLen = (GT_U32)strlen(formatStringPtr);

            atLeastOneFilter = GT_TRUE;

            if(messageLen < allowedStringLen)
            {
                /* as far for this filter check ... filter the message */
                continue;
            }

            /* check if the formatted string is allowed */
            for(ii = 0 ; ii < ((messageLen - allowedStringLen) + 1); ii++)
            {
                if(0 == strncmp(&formatStringPtr[ii],
                           currentStringPtr,
                           allowedStringLen))
                {
                    /* the string match */
                    matchFound = GT_TRUE;
                    break;
                }
            }
        }

        if(matchFound == GT_TRUE)
        {
            break;
        }
    }/*jj*/

    if(atLeastOneFilter == GT_TRUE && matchFound == GT_FALSE)
    {
        /* at least one filter enabled */
        /* but the string failed all filters */

        /* filter the message */
        return GT_TRUE;
    }

    return GT_FALSE;/* no filtering */
}

/* check frame info filters :
INPUT:
    devObjPtr - the current device (maybe NULL)
    logInfoType     - log info type

RETURN:
    GT_TRUE - need to filter the message
    GT_FALSE - do not filter the message

*/
static GT_BOOL frameInfoFiltersCheck(
    IN SIM_LOG_INFO_TYPE_ENT        logInfoType
)
{
    GT_BOOL matchFound;
    GT_BOOL needToFilter;
    SIM_LOG_FRAME_INFO_STC *currentLogFrameInfoPtr;
    GT_U32  ii;

    if(logInfoType != SIM_LOG_INFO_TYPE_PACKET_E)
    {
        return GT_FALSE;/* no filtering */
    }

    /*get current info */
    currentLogFrameInfoPtr = simLogPacketFrameMyLogInfoGet();

    if(currentLogFrameInfoPtr == NULL)
    {
        return GT_FALSE;/* no filtering */
    }

    matchFound = GT_FALSE;
    needToFilter = GT_FALSE;

    if(currentLogFrameInfoPtr->unitType != SIM_LOG_FRAME_UNIT___ALLOW_ALL_UNITS___E)
    {
        /* check filter on the units */
        for(ii = 0 ; ii < STRING_FILTERS_NUM_CNS; ii++)
        {
            switch(simLogFilterAllowedSpecificUnits[ii])
            {
                case SIM_LOG_FRAME_UNIT___MUST_BE_FIRST___E :
                    break;
                case SIM_LOG_FRAME_UNIT_INGRESS_PIPE_E :
                    if(SIM_LOG_FRAME_UNIT_IS_INGRESS_PIPE_MAC(currentLogFrameInfoPtr->unitType))
                    {
                        matchFound = GT_TRUE;
                    }
                    break;
                case SIM_LOG_FRAME_UNIT_EGRESS_PIPE_E :
                    if(SIM_LOG_FRAME_UNIT_IS_EGRESS_PIPE_MAC(currentLogFrameInfoPtr->unitType))
                    {
                        matchFound = GT_TRUE;
                    }
                    break;
                case SIM_LOG_FRAME_UNIT_GENERAL_E :
                    if(SIM_LOG_FRAME_UNIT_IS_GENERAL_MAC(currentLogFrameInfoPtr->unitType))
                    {
                        matchFound = GT_TRUE;
                    }
                    break;
                default:
                    if(currentLogFrameInfoPtr->unitType == simLogFilterAllowedSpecificUnits[ii])
                    {
                        matchFound = GT_TRUE;
                    }
                    break;
            }

            if(simLogFilterAllowedSpecificUnits[ii] != SIM_LOG_FRAME_UNIT___MUST_BE_FIRST___E)
            {
                needToFilter = GT_TRUE;
            }


            if(matchFound == GT_TRUE)
            {
                /* found match */
                break;
            }
        }

        if((matchFound == GT_FALSE) && (needToFilter == GT_TRUE))
        {
            /* at least one unit is specified as 'allowed' so if our unit not match , then need to filter */

            /* filter the message */
            return GT_TRUE;
        }
    }
    else
    {
        /* allow 'wild card' to be able to LOG regardless to the filters !!! */
    }

    needToFilter = GT_FALSE;

    /* check filter on the commands */
    for(ii = 0 ; ii < STRING_FILTERS_NUM_CNS; ii++)
    {
        if(currentLogFrameInfoPtr->commandType == simLogFilterDeniedSpecificCommands[ii])
        {
            needToFilter = GT_TRUE;
            break;
        }
    }

    return needToFilter;
}

/**
* @internal simLogMessage_va_list function
* @endinternal
*
* @brief   This routine logs message (using the va_list).
*
* @retval GT_OK                    - Logging has been done successfully.
* @retval GT_BAD_PARAM             - Wrong parameter.
* @retval GT_FAIL                  - General failure error. Should never happen.
*/
static GT_STATUS simLogMessage_va_list
(
    IN GT_CHAR               const *funcName,
    IN GT_CHAR               const *fileName,
    IN GT_U32                       lineNum,
    IN SKERNEL_DEVICE_OBJECT const *devObjPtr,
    IN SIM_LOG_INFO_TYPE_ENT        logInfoType,
    IN const GT_CHAR               *formatStringPtr,
    IN va_list                      arg
)
{
    SIM_OS_TASK_PURPOSE_TYPE_ENT threadType;
    GT_U32                       i;
    static GT_CHAR               lastLocationMetaDataString[256] = {0};
    static GT_CHAR               lastTaskTypeName[256];
    static GT_CHAR               extraInfoStr[256];
    static GT_CHAR               extraInfoStr2[256];
    GT_CHAR                      *newFileName;/* new file name after compress*/
    GT_BOOL                     needNewLine;
    GT_U32                      messageLen;
    GT_CHAR                     *taskTypeName;/* set inside THREAD_FILTER_CASE_MAC */
    GT_U32                      deviceId;
    GT_U32                      coreId;
    GT_BOOL                     needToFilter;

    if(!simLogIsOpenFlag)
    {
        /* log is disabled */
        return GT_OK;
    }

    /* process info types filter */
    switch(logInfoType)
    {
        INFO_TYPE_FILTER_CASE_MAC(SIM_LOG_INFO_TYPE_PACKET_E, simLogInfoTypePacket);
        INFO_TYPE_FILTER_CASE_MAC(SIM_LOG_INFO_TYPE_DEVICE_E, simLogInfoTypeDevice);
        INFO_TYPE_FILTER_CASE_MAC(SIM_LOG_INFO_TYPE_TCAM_E, simLogInfoTypeTcam);
        INFO_TYPE_FILTER_CASE_MAC(SIM_LOG_INFO_TYPE_MEMORY_E, simLogInfoTypeMemory);
        default:
            return GT_BAD_PARAM;
    }

    /* process thread types filter */
    SIM_OS_MAC(simOsTaskOwnTaskPurposeGet)(&threadType);
    switch(threadType)
    {
        THREAD_FILTER_CASE_MAC(SIM_OS_TASK_PURPOSE_TYPE_CPU_APPLICATION_E,simLogFilterTypeCpuApplication);
        THREAD_FILTER_CASE_MAC(SIM_OS_TASK_PURPOSE_TYPE_CPU_ISR_E, simLogFilterTypeCpuIsr);
        THREAD_FILTER_CASE_MAC(SIM_OS_TASK_PURPOSE_TYPE_PP_AGING_DAEMON_E, simLogFilterTypePpAgingDaemon);
        THREAD_FILTER_CASE_MAC(SIM_OS_TASK_PURPOSE_TYPE_PP_PIPE_PROCESSING_DAEMON_E, simLogFilterTypePpPipeProcessingDaemon);
        THREAD_FILTER_CASE_MAC(SIM_OS_TASK_PURPOSE_TYPE_PP_PIPE_SDMA_QUEUE_DAEMON_E, simLogFilterTypePpPipeSdmaQueueDaemon);
        THREAD_FILTER_CASE_MAC(SIM_OS_TASK_PURPOSE_TYPE_PP_PIPE_OAM_KEEP_ALIVE_DAEMON_E, simLogFilterTypePpPipeOamKeepAliveDaemon);
        THREAD_FILTER_CASE_MAC(SIM_OS_TASK_PURPOSE_TYPE_PP_PIPE_GENERAL_PURPOSE_E,simLogFilterTypePpPipeGeneralPurpose);
        THREAD_FILTER_CASE_MAC(SIM_OS_TASK_PURPOSE_TYPE_PP_PIPE_SOFT_RESET_E,simLogFilterTypePpPipeSoftReset);
        THREAD_FILTER_CASE_MAC(SIM_OS_TASK_PURPOSE_TYPE_INITIALIZATION_E,simLogFilterTypePpPipeProcessingDaemon);
        THREAD_FILTER_CASE_MAC(SIM_OS_TASK_PURPOSE_TYPE_PREQ_SRF_DAEMON_E,simLogFilterTypePreqSrfDaemon);

        default:
            return GT_BAD_PARAM;
    }

    if(devObjPtr)
    {
        if(devObjPtr->portGroupSharedDevObjPtr)
        {
            deviceId =  devObjPtr->portGroupSharedDevObjPtr->deviceId;
            coreId = devObjPtr->portGroupId;
        }
        else
        {
            deviceId =  devObjPtr->deviceId;
            coreId = 0;
        }

        /* dev - port group filter */
        for(i = 0; i != SIM_LOG_MAX_DF; i++)
        {
            /* search filter in array */
            if( simLogDevPortGroupFilters[i].filterId &&
               (simLogDevPortGroupFilters[i].devNum    == deviceId) &&
               (simLogDevPortGroupFilters[i].portGroup == coreId) )
            {
                /* filter message */
                return GT_OK;
            }
        }

    }
    else
    {
        deviceId = 0;
        coreId = 0;
    }
    /* check filter on the strings */
    needToFilter = stringFiltersCheck(formatStringPtr,
        simLogFilterAllowedSpecificString);
    if(needToFilter == GT_TRUE)
    {
        /* filter the message */
        return GT_OK;
    }
    /* check filter on the functions */
    needToFilter = stringFiltersCheck(funcName,
        simLogFilterAllowedSpecificFunction);
    if(needToFilter == GT_TRUE)
    {
        /* filter the message */
        return GT_OK;
    }

    needToFilter = frameInfoFiltersCheck(logInfoType);
    if(needToFilter == GT_TRUE)
    {
        /* filter the message */
        return GT_OK;
    }

    /* lock the waiting operations */
    scibAccessLock();

    extraInfoStr[0] = 0;
    extraInfoStr2[0] = 0;
    needNewLine = GT_FALSE;

    if(funcName != NULL && fileName != NULL)
    {
        /* compress file name from:
           x:\cpss\sw\prestera\simulation\simdevices\src\asicsimulation\skernel\suserframes\snetcheetahingress.c
           To:
           snetcheetahingress.c */
        newFileName = compressFileName(fileName);

        if(loggerUseLineNumber)
        {
            sprintf(extraInfoStr,"dev[%d] core[%d] func[%s] file[%s] Line[%d] \n",
                    deviceId,coreId,funcName, newFileName, lineNum);
        }
        else
        {
            sprintf(extraInfoStr, "dev[%d] core[%d] func[%s] file[%s] \n",
                    deviceId,coreId,funcName, newFileName);

            if(supportCompressedMetadataInfo == GT_TRUE)
            {
                if(0 == strcmp(lastLocationMetaDataString,extraInfoStr))
                {
                    /* reduce redundancy in the mete data info */
                    extraInfoStr[0] = 0;
                }
                else
                {
                    sprintf(lastLocationMetaDataString, "%s", extraInfoStr);
                }
            }
            else
            {
                /*
                    allow to generate LOG with 'redundant' meta data info , to have easy 'compare' with legacy LOG files !!!
                */
            }
        }
    }

    if(initDone && 0 != strcmp(lastTaskTypeName,taskTypeName))
    {
        /* new task type */
        sprintf(extraInfoStr2, TASK_TYPE_FORMAT_CNS, taskTypeName);
    }

    /* save task type for next time */
    sprintf(lastTaskTypeName, "%s", taskTypeName);

    messageLen = (GT_U32)strlen(formatStringPtr);

    if(strncmp(formatStringPtr + (messageLen-1) , "\n", 1) && (messageLen > 10))
    {
        /* if not end with "\n" ... add it now */
        needNewLine = GT_TRUE;
    }

    if(simLogOutputToLogfile)
    {
        static char buf[4096];

        sprintf(buf, "%s%s", extraInfoStr2, extraInfoStr);
        vsprintf(strchr(buf,0), formatStringPtr, arg);

        if(needNewLine)
        {
            strcat(buf, "\n");
        }
        simLogTaskAddLog(buf);
    }

    if(simLogOutputToStdout)
    {
        if(extraInfoStr2[0] != 0)
        {
            simGeneralPrintf(extraInfoStr2);
        }

        if(extraInfoStr[0] != 0)
        {
            simGeneralPrintf(extraInfoStr);
        }
        vprintf(formatStringPtr, arg);

        if(needNewLine)
        {
            simGeneralPrintf("\n");
        }
    }

    /* UnLock the waiting operations */
    scibAccessUnlock();

    return GT_OK;
}

/**
* @internal simLogMessage function
* @endinternal
*
* @brief   This routine logs message.
*
* @retval GT_OK                    - Logging has been done successfully.
* @retval GT_BAD_PARAM             - Wrong parameter.
* @retval GT_FAIL                  - General failure error. Should never happen.
*
* @note Usage example:
*       simLogMessage(SIM_LOG_FUNC_NAME_MAC(snetChtL2i), devObjPtr,
*       SIM_LOG_INFO_TYPE_PACKET_E, "value is: %d\n", 123);
*
*/
GT_STATUS simLogMessage
(
    IN GT_CHAR               const *funcName,
    IN GT_CHAR               const *fileName,
    IN GT_U32                       lineNum,
    IN SKERNEL_DEVICE_OBJECT const *devObjPtr,
    IN SIM_LOG_INFO_TYPE_ENT        logInfoType,
    IN const GT_CHAR               *formatStringPtr,
    IN ...
)
{
    GT_STATUS rc;
    IN va_list  arg;

    va_start (arg, formatStringPtr);

    rc =  simLogMessage_va_list(funcName,fileName,lineNum,
            devObjPtr,logInfoType,formatStringPtr,arg);

    va_end (arg);

    return rc;
}
/**
* @internal simLogMessage function
* @endinternal
*
* @brief   This routine logs CM3  message.
*
* @retval GT_OK                    - Logging has been done successfully.
* @retval GT_BAD_PARAM             - Wrong parameter.
* @retval GT_FAIL                  - General failure error. Should never happen.
*
* @note Usage example:
*       simCm3LogMessage( cm3DevId,"value is: %d\n", 123);
*
*/
GT_STATUS simCm3LogMessage
(
    IN GT_U32                       cm3DevId,
    IN const GT_CHAR               *formatStringPtr,
    IN ...
)
{
    GT_STATUS rc;
    IN va_list  arg;
    SKERNEL_DEVICE_OBJECT const *devObjPtr;
    static GT_CHAR               extraInfoStr[256];

    devObjPtr = smemTestDeviceIdToDevPtrConvert(cm3DevId);

    sprintf(extraInfoStr,"CM3 id %d message :%s",cm3DevId,formatStringPtr);

    va_start (arg, formatStringPtr);

    rc =  simLogMessage_va_list(NULL,NULL,0,
            devObjPtr,SIM_LOG_INFO_TYPE_MEMORY_E,extraInfoStr,arg);

    va_end (arg);

    return rc;
}



/**************************************/
GT_STATUS simLogFileOpen(GT_VOID)
{
    GT_BOOL  old_simLogInfoTypePacket;
    GT_BOOL  old_simLogInfoTypeMemory;
    GT_BOOL  old_simLogOutputToLogfile;
    SKERNEL_DEVICE_OBJECT * devObjPtr = NULL;

    if(gmLogSupport)
    {
        /* the GM responsible for opening it's LOG file */
        simLogOutputToLogfile = GT_TRUE;
        initDone = 1;
        simLogIsOpenFlag = simLogIsOpen();
        /* no more to do */
        return GT_OK;
    }

    simLogTaskOpenFile(simLogFileName);

    scibAccessLock();

    old_simLogOutputToLogfile = simLogOutputToLogfile;
    old_simLogInfoTypeMemory = simLogInfoTypeMemory;
    old_simLogInfoTypePacket = simLogInfoTypePacket;

    simLogInfoTypePacket = GT_TRUE;
    simLogInfoTypeMemory = GT_TRUE;
    simLogOutputToLogfile = GT_TRUE;
    simLogIsOpenFlag = simLogIsOpen();

    /* log the simulation baseline */
    __LOG_NO_LOCATION_META_DATA(("The Log file of the WM simulation \n"));
    __LOG_NO_LOCATION_META_DATA((CHIP_SIMULATION_STREAM_NAME_CNS "\n"));

    /*send to log all the info about the devices.*/
    simLogAddDevicesInfo();

    /* print header of memory access */
    simLogMessage(NULL, NULL, 0, NULL, SIM_LOG_INFO_TYPE_MEMORY_E,
      "************ format of memory access : ************* \n"      );

    simLogMessage(NULL, NULL, 0, NULL, SIM_LOG_INFO_TYPE_MEMORY_E,
      "%6s %6s %3s %5s %10s %10s %10s %9s\n"
      "------------------------------------",
      "Action", "Source", "Dev", "Core", "Address", "Value", "Old value", "XOR");


    /* print header of memory access */
    __LOG_NO_LOCATION_META_DATA(("************ useful packet walkthrough indicators: ************* \n"));

    __LOG_NO_LOCATION_META_DATA((SIM_LOG_IN_PACKET_STR));
    __LOG_NO_LOCATION_META_DATA((SIM_LOG_ENDED_IN_PACKET_STR));
    __LOG_NO_LOCATION_META_DATA((SIM_LOG_OUT_REPLICATION_STR));
    __LOG_NO_LOCATION_META_DATA(("dump packet : device[???] core[???] ingress port \n"));
    __LOG_NO_LOCATION_META_DATA(("dump packet : device[???] core[???] egress port \n"));
    __LOG_NO_LOCATION_META_DATA(("??? packet descr changes: \n"));
    __LOG_NO_LOCATION_META_DATA(("^^^^^^^ Start of Current basic descriptor parameters \n"));

    __LOG_NO_LOCATION_META_DATA((SIM_LOG_INGRESS_PACKET_FROM_CPU_DMA_STR "\n"));
    __LOG_NO_LOCATION_META_DATA((SIM_LOG_INGRESS_PACKET_FROM_CPU_DMA_ENDED_STR "\n"));

    __LOG_NO_LOCATION_META_DATA((SIM_LOG_INGRESS_PACKET_FROM_SLAN_STR "\n"));
    __LOG_NO_LOCATION_META_DATA((SIM_LOG_END_OF_PACKET_DUMP_STR "\n"));

    __LOG_NO_LOCATION_META_DATA((SIM_LOG_READY_TO_START_LOG_STR "\n\n\n"  ));


    /*release the forcing of 'simLogInfoTypeMemory = GT_TRUE' */
    simLogInfoTypeMemory = old_simLogInfoTypeMemory;

    simLogSlanCatchUp();
    simLogLinkStateCatchUp();

    /*release the forcing of 'simLogInfoTypePacket = GT_TRUE' */
    /*release the forcing of 'simLogOutputToLogfile = GT_TRUE' */
    simLogOutputToLogfile = old_simLogOutputToLogfile;
    simLogInfoTypePacket = old_simLogInfoTypePacket;
    simLogIsOpenFlag = simLogIsOpen();
    initDone = 1;
    scibAccessUnlock();

    return GT_OK;
}

/*******************************************************************************/
static GT_VOID simLogIniParseDevFilter(GT_VOID)
{
    GT_CHAR param_str[SIM_OS_CNF_FILE_MAX_LINE_LENGTH_CNS];
    GT_CHAR keyStr[SIM_OS_CNF_FILE_MAX_LINE_LENGTH_CNS];
    GT_U32  id = 0;
    GT_U32  dev;
    GT_U32  portGroup;
    GT_BOOL enable = GT_FALSE;

    for(dev = 0; dev < MAX_DEV_NUM; dev++)
    {
        for(portGroup = 0; portGroup < MAX_PORT_GROUP_NUM; portGroup++)
        {
            sprintf(keyStr, "filter_dev%u_port_group%u", dev, portGroup);

            if(SIM_OS_MAC(simOsGetCnfValue)("log", keyStr,
                             SIM_OS_CNF_FILE_MAX_LINE_LENGTH_CNS, param_str))
            {
                if(id == SIM_LOG_MAX_DF)
                {
                    printf("simLogIniParseDevFilter: too many 'dev,portGroup' filters in the INI file ... only first[%d] applied \n",
                        SIM_LOG_MAX_DF);
                    return;
                }
                sscanf(param_str, "%u", &enable);
                if(enable)
                {
                    simLogDevPortGroupFilters[id].filterId  = id+1;
                    simLogDevPortGroupFilters[id].devNum    = dev;
                    simLogDevPortGroupFilters[id].portGroup = portGroup;
                    id++;
                }
            }
        }
    }
}

/*******************************************************************************/
static GT_VOID simLogIniParse(GT_VOID)
{
    GT_CHAR paramStr[SIM_OS_CNF_FILE_MAX_LINE_LENGTH_CNS];

    /***********************check for output config *********************/
    /* check if output to log file enabled */
    GET_BOOL_VALUE_MAC("log_to_logfile", simLogOutputToLogfile);

    if(gmLogSupport)
    {
        return;
    }

    if(simLogOutputToLogfile)
    {
        /* get log file name */
        if(GET_INI_LOG_VALUE_MAC("file_name"))
        {
            sscanf(paramStr, "%s", &simLogFileName);
        }
        else
        {
            skernelFatalError("simLogIniParse: log file name not found in ini");
        }
    }

    /* check if output to stdout enabled */
    GET_BOOL_VALUE_MAC("log_to_stdout", simLogOutputToStdout);

    /***********************check for info types *********************/
    GET_BOOL_VALUE_MAC("info_type_packet", simLogInfoTypePacket);
    GET_BOOL_VALUE_MAC("info_type_device", simLogInfoTypeDevice);
    GET_BOOL_VALUE_MAC("info_type_tcam",   simLogInfoTypeTcam);
    GET_BOOL_VALUE_MAC("info_type_memory", simLogInfoTypeMemory);

    /*******************check for threads filter *********************/
    GET_BOOL_VALUE_MAC("filter_thread_cpu_application",
                    simLogFilterTypeCpuApplication);
    GET_BOOL_VALUE_MAC("filter_thread_cpu_isr",
                    simLogFilterTypeCpuIsr);
    GET_BOOL_VALUE_MAC("filter_thread_pp_aging_daemon",
                    simLogFilterTypePpAgingDaemon);
    GET_BOOL_VALUE_MAC("filter_thread_pp_pipe_processing_daemon",
                    simLogFilterTypePpPipeProcessingDaemon);
    GET_BOOL_VALUE_MAC("filter_thread_pp_pipe_sdma_queue_daemon",
                    simLogFilterTypePpPipeSdmaQueueDaemon);
    GET_BOOL_VALUE_MAC("filter_thread_pp_pipe_oam_keep_alive_daemon",
                    simLogFilterTypePpPipeOamKeepAliveDaemon);

    /*************check for dev/port group filter ********************/
    simLogIniParseDevFilter();
}

/**
* @internal simLogInit function
* @endinternal
*
* @brief   Init logger
*
* @retval GT_OK                    - Logger has been successfully initialized.
* @retval GT_FAIL                  - General failure error. Should never happen.
*/
GT_STATUS simLogInit()
{
    /* start file logger task */
    simLogTaskInit();
    /* empty call - it needs to link simLogRuntimeConfig.c correctly */
    simLogEmptyFunc();
    /* empty call - it needs to link simLogToRuntime.c correctly */
    simLogToRuntimeEmptyFunc();

    /* parse log section of current ini file */
    simLogIniParse();

    /* open log file */
    if(simLogOutputToLogfile)
    {
        if( GT_OK != simLogFileOpen() )
        {
            skernelFatalError("simLogInit: unable to create log file: %s", simLogFileName);
        }
    }

    return GT_OK;
}

/**
* @internal simLogClose function
* @endinternal
*
* @brief   This routine closes logger.
*
* @retval GT_OK                    - Logger has been successfully closed.
* @retval GT_FAIL                  - General failure error. Should never happen.
*/
GT_STATUS simLogClose()
{
    if(gmLogSupport)
    {
        /* the GM responsible for opening it's LOG file */
        return GT_OK;
    }

    return simLogTaskCloseFile();
}

/**
* @internal simLogDump function
* @endinternal
*
* @brief   Dumps content of pointer of buffers (by length)
*
* @retval GT_OK                    - success
* @retval GT_BAD_PTR               - wrong pointer
* @retval GT_BAD_PARAM             - wrong params
* @retval GT_FAIL                  - general failure error
*/
GT_STATUS simLogDump
(
    IN SKERNEL_DEVICE_OBJECT const *devObjPtr,
    IN SIM_LOG_INFO_TYPE_ENT        infoType,
    IN GT_PTR                       dataPtr,
    IN GT_U32                       length
)
{
    GT_U32  byteCount = 0;
    GT_U32  trancateLength = 256;
    GT_U32  limitedLength = MIN(trancateLength,length);

    ASSERT_PTR(devObjPtr);

    if(NULL == dataPtr)
    {
        return GT_BAD_PTR;
    }

    if(0 == length)
    {
        return GT_BAD_PARAM;
    }

    /* keep the dump as single group */
    scibAccessLock();

    for(byteCount = 0; byteCount < limitedLength; byteCount++)
    {
        if((byteCount & 0x0F) == 0)
        {
            simLogMessage(NULL, NULL, 0, devObjPtr, infoType, "0x%4.4x :", byteCount);
        }

        simLogMessage(NULL, NULL, 0, devObjPtr, infoType, " %2.2x", ((GT_U8*)dataPtr)[byteCount]);

        if((byteCount & 0x0F) == 0x0F)
        {
            simLogMessage(NULL, NULL, 0, devObjPtr, infoType, "\n");
        }
    }

    if(length > trancateLength)
    {
        /* print also at least last 16 bytes of the packet */
        /* so the 4 bytes CRC and the 6 bytes 'PRP trailer' can be seen in the LOG */
        byteCount = (length - 15) &~ 0x0F;
        simLogMessage(NULL, NULL, 0, devObjPtr, infoType, " .. and the last [%d] bytes :\n", length-byteCount);

        for(/*already set*/; byteCount < length; byteCount++)
        {
            if((byteCount & 0x0F) == 0)
            {
                simLogMessage(NULL, NULL, 0, devObjPtr, infoType, "0x%4.4x :", byteCount);
            }

            simLogMessage(NULL, NULL, 0, devObjPtr, infoType, " %2.2x", ((GT_U8*)dataPtr)[byteCount]);

            if((byteCount & 0x0F) == 0x0F)
            {
                simLogMessage(NULL, NULL, 0, devObjPtr, infoType, "\n");
            }
        }

    }
    simLogMessage(NULL, NULL, 0, devObjPtr, infoType, "\n");

    scibAccessUnlock();

    return GT_OK;
}


/**
* @internal simLogIsOpen function
* @endinternal
*
* @brief   Is the Logger open ?
*
* @retval 0                        - No. logger is NOT open.
* @retval else                     - Yes. logger is open.
*/
GT_U32 simLogIsOpen(void)
{
#ifdef GM_USED
    /* we not write the LOG file of simulation ! */
    /* this is only for write to GM LOG file */
    return 0;
#else
    if ((simLogTaskFileOpened() != GT_TRUE) ||
        ((simLogOutputToLogfile == 0) && (simLogOutputToStdout == 0)))
    {
        return 0;
    }

    return 1;
#endif
}

/* 'Log is open' flag to avoid call overhead */
GT_BOOL simLogIsOpenFlag = GT_FALSE;

/**
* @internal simLogUseLineNumber function
* @endinternal
*
* @brief   state if the 'line numbers' should appear in the logger.
*         by default if this function is not called --> no 'line numbers' indication.
*
* @retval GT_OK                    - on all cases
*/
GT_STATUS simLogUseLineNumber(
    IN GT_U32   useLineNumber
)
{
    loggerUseLineNumber = useLineNumber;

    return GT_OK;
}

/**
* @internal simLogFilterAllowedSpecificStringSet function
* @endinternal
*
* @brief   set filter to allow only lines that 'contain' the 'allowString' (up to 256 characters) .
*         up to 16 strings
* @param[in] allowString              - the allowed string.
*                                      NULL means --> remove the filter.
* @param[in] index                    - filter  (0..15)
*
* @retval GT_OK                    - on all cases
*/
GT_STATUS   simLogFilterAllowedSpecificStringSet(
    IN GT_CHAR* allowString,
    IN GT_U32   index
)
{
    if(index >= STRING_FILTERS_NUM_CNS)
    {
        return GT_BAD_PARAM;
    }

    if(allowString)
    {
        strncpy(&simLogFilterAllowedSpecificString[index][0],allowString,sizeof(simLogFilterAllowedSpecificString[0]));
    }
    else
    {
        simLogFilterAllowedSpecificString[index][0] = 0;
    }

    return GT_OK;
}

/**
* @internal simLogFilterAllowedSpecificFunctionSet function
* @endinternal
*
* @brief   set filter to allow only lines that 'contain' the 'allowFuncName' (up to 256 characters) .
*         up to 16 strings(functions)
* @param[in] allowFuncName            - the allowed function name.
*                                      NULL means --> remove the filter.
* @param[in] index                    - filter  (0..15)
*
* @retval GT_OK                    - on all cases
*/
GT_STATUS   simLogFilterAllowedSpecificFunctionSet(
    IN GT_CHAR* allowFuncName,
    IN GT_U32   index
)
{
    if(index >= STRING_FILTERS_NUM_CNS)
    {
        return GT_BAD_PARAM;
    }

    if(allowFuncName)
    {
        strncpy(&simLogFilterAllowedSpecificFunction[index][0],allowFuncName,sizeof(simLogFilterAllowedSpecificFunction[0]));
    }
    else
    {
        simLogFilterAllowedSpecificFunction[index][0] = 0;
    }

    return GT_OK;
}

/* DB for the use of simLogInternalLog,simLogInfoSave */
static struct INTERNAL_INFO_STCT
{
    GT_CHAR               const *funcName;
    GT_CHAR               const *fileName;
    GT_U32                       lineNum;
    SKERNEL_DEVICE_OBJECT const *devObjPtr;
    SIM_LOG_INFO_TYPE_ENT        logInfoType;
} currentInfo =
{
    "",    /*funcName;    */
    "",    /*fileName;    */
    0,     /*lineNum;     */
    NULL,  /*devObjPtr;   */
    0      /*logInfoType; */
};

/**
* @internal simLogInternalLog function
* @endinternal
*
* @brief   This routine logs message , with the help of simLogInfoSave that 'saved'
*         info for it's use .
* @param[in] formatStringPtr          - (pointer to) format string.
*                                      ...             - params
*                                       None.
*/
void simLogInternalLog
(
    IN const GT_CHAR *formatStringPtr,
    IN ...
)
{
    IN va_list  arg;

    va_start (arg, formatStringPtr);

    simLogMessage_va_list(
                currentInfo.funcName,
                currentInfo.fileName,
                currentInfo.lineNum,
                currentInfo.devObjPtr,
                currentInfo.logInfoType,
                formatStringPtr,
                arg);

    va_end (arg);

    return;
}

/**
* @internal simLogInfoSave function
* @endinternal
*
* @brief   This routine 'save' info for the use of simLogInternalLog()
*/
void simLogInfoSave
(
    IN GT_CHAR               const *funcName,
    IN GT_CHAR               const *fileName,
    IN GT_U32                       lineNum,
    IN SKERNEL_DEVICE_OBJECT const *devObjPtr,
    IN SIM_LOG_INFO_TYPE_ENT        logInfoType
)
{
    /* save in the DB */
    currentInfo.funcName     = funcName     ;
    currentInfo.fileName     = fileName     ;
    currentInfo.lineNum      = lineNum      ;
    currentInfo.devObjPtr    = devObjPtr    ;
    currentInfo.logInfoType  = logInfoType  ;

    return;
}

/**
* @internal simLogBypassCompressedMetadataInfoSet function
* @endinternal
*
* @brief   Function to state if allow to generate LOG with 'redundant' meta data info or not.
*         by default the mode is 'bypassCompressing = GT_FALSE' ,
*         but when bypassCompressing == GT_TRUE it will allow to generate LOG with
*         'redundant' meta data info have easy 'compare' wit legacy LOG files !!!
* @param[in] bypassCompressing        -
*                                       None.
*/
void simLogBypassCompressedMetadataInfoSet
(
    IN GT_BOOL bypassCompressing
)
{
    if (bypassCompressing == GT_FALSE)
    {
        supportCompressedMetadataInfo = GT_TRUE;
    }
    else
    {
        supportCompressedMetadataInfo = GT_FALSE;
    }
}

/**
* @internal simLogFilterAllowedSpecificUnitSet function
* @endinternal
*
* @brief   set filter to allow only lines that 'belongs' to the 'allowedUnitName'
*         (up to 256 characters) .
*         up to 16 strings (units)
* @param[in] allowedUnitName          - the allowed unit name. (see SIM_LOG_FRAME_UNIT_ENT)
*                                      so names are (lower case ):
*                                      "tti" , "ipcl" , .. "ingress_pipe" ,
*                                      "txq" , "ha" , "epcl" .. "egress_pipe" ,
*                                      "cnc" , "tcam" , .. "general"
*                                      NULL means --> remove the filter.
* @param[in] index                    - filter  (0..15)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on unknown unit name
*/
GT_STATUS   simLogFilterAllowedSpecificUnitSet(
    IN GT_CHAR* allowedUnitName,
    IN GT_U32   index
)
{
    static GT_CHAR tmpName1[255] = "";
    static GT_CHAR tmpName2[255] = "";
    GT_U32  ii;

    if(index >= STRING_FILTERS_NUM_CNS)
    {
        return GT_BAD_PARAM;
    }

    if(allowedUnitName)
    {
        if(strlen(allowedUnitName) > 100)
        {
            return  GT_BAD_PARAM;
        }

        for(ii = 0 ; (unisNameArr[ii] != NULL) ; ii++)
        {
            /* all names in unisNameArr hold , so convert allowedUnitName to be like it
                prefix      :       SIM_LOG_FRAME_UNIT_
                and postfix :       _E
                */
            sprintf(tmpName1,"SIM_LOG_FRAME_UNIT_%s_E",allowedUnitName);
            sprintf(tmpName2,"%s",unisNameArr[ii]);
            strlwr(tmpName1);
            strlwr(tmpName2);

            if(0 ==
                strcmp(tmpName1 , tmpName2))
            {
                break;
            }
        }

        if(unisNameArr[ii] == NULL)
        {
            /* not found */
            return GT_BAD_PARAM;
        }

        simLogFilterAllowedSpecificUnits[index] = ii;
    }
    else
    {
        simLogFilterAllowedSpecificUnits[index] = SIM_LOG_FRAME_UNIT___MUST_BE_FIRST___E;
    }

    return GT_OK;
}


/**
* @internal simLogFilterDenySpecificPacketCommandSet function
* @endinternal
*
* @brief   set filter to DENY lines that 'belongs' to the 'PacketCommandName'
*         (up to 256 characters) .
*         up to 16 strings (units)
* @param[in] deniedPacketCommandName  - the denied unit name. (see SIM_LOG_FRAME_COMMAND_TYPE_ENT)
*                                      so names are (lower case ):
*                                      "from_cpu"
*                                      NULL means --> remove the filter.
* @param[in] index                    - filter  (0..15)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on unknown packet command name
*/
GT_STATUS   simLogFilterDenySpecificPacketCommandSet(
    IN GT_CHAR* deniedPacketCommandName,
    IN GT_U32   index
)
{
    static GT_CHAR tmpName1[255] = "";
    static GT_CHAR tmpName2[255] = "";
    GT_U32  ii;

    if(index >= STRING_FILTERS_NUM_CNS)
    {
        return GT_BAD_PARAM;
    }

    if(deniedPacketCommandName)
    {
        if(strlen(deniedPacketCommandName) > 100)
        {
            return  GT_BAD_PARAM;
        }

        for(ii = 0 ; (commandNameArr[ii] != NULL) ; ii++)
        {
            /* all names in unisNameArr hold , so convert allowedUnitName to be like it
                prefix      :       SIM_LOG_FRAME_UNIT_
                and postfix :       _E
                */
            sprintf(tmpName1,"SIM_LOG_FRAME_COMMAND_TYPE_%s_E",deniedPacketCommandName);
            sprintf(tmpName2,"%s",commandNameArr[ii]);
            strlwr(tmpName1);
            strlwr(tmpName2);

            if(0 ==
                strcmp(tmpName1 , tmpName2))
            {
                break;
            }
        }

        if(commandNameArr[ii] == NULL)
        {
            /* not found */
            return GT_BAD_PARAM;
        }

        simLogFilterDeniedSpecificCommands[index] = ii;
    }
    else
    {
        simLogFilterDeniedSpecificCommands[index] = SIM_LOG_FRAME_COMMAND_TYPE___MUST_BE_FIRST___E;
    }

    return GT_OK;
}

/**
* @internal internal_simLogPacketFrameUnitSet function
* @endinternal
*
* @brief   Set the unit Id of current thread (own thread).
*
* @param[in] frameUnit                -  the unit id
*                                       None.
*/
static GT_VOID internal_simLogPacketFrameUnitSet(
    IN SIM_LOG_FRAME_UNIT_ENT frameUnit
)
{
    SIM_LOG_FRAME_INFO_STC *currentLogFrameInfoPtr =
        simLogPacketFrameMyLogInfoGet();
    SIM_LOG_FRAME_UNIT_ENT  currUnit;
    SIM_LOG_FRAME_UNIT_ENT  restoredUnit;
    SKERNEL_DEVICE_OBJECT * devObjPtr = NULL;

    if(currentLogFrameInfoPtr == NULL)
    {
        return;
    }

    if(currentLogFrameInfoPtr->unitType == frameUnit)
    {
        /* no need for double printings of the same unit */
        return;
    }

    restoredUnit =  SIM_LOG_FRAME_UNIT___LAST___E;/* no restore indication */

    if(frameUnit == SIM_LOG_FRAME_UNIT___RESTORE_PREVIOUS_UNIT___E)
    {
        if(currentLogFrameInfoPtr->unitType == SIM_LOG_FRAME_UNIT___ALLOW_ALL_UNITS___E)
        {
            /* we need to restore previous unit after been into 'allow_all_units' */
            currentLogFrameInfoPtr->unitType =
                currentLogFrameInfoPtr->prevUnitType;

            return;
        }
        else /*if(currentLogFrameInfoPtr->unitType == SIM_LOG_FRAME_UNIT_CNC_E ||
                currentLogFrameInfoPtr->unitType == SIM_LOG_FRAME_UNIT_TCAM_E)*/
        {
            /* we need to restore previous unit after been into CNC/TCAM */
            /* so behave as if the new mode should be 'previous' (restore previous) */
            restoredUnit = currentLogFrameInfoPtr->prevUnitType;
            frameUnit = restoredUnit;
        }
#if 0
        else
        {
            /* ignored !!! */
            return;
        }
#endif/*0*/
    }

    /* current value -- before modify to new state */
    currUnit = currentLogFrameInfoPtr->unitType;
    /*set new state */
    currentLogFrameInfoPtr->unitType = frameUnit;

    if(currentLogFrameInfoPtr->prevUnitType == frameUnit &&
        currUnit == SIM_LOG_FRAME_UNIT___ALLOW_ALL_UNITS___E)
    {
        /* no need for double printings of the same unit  , as we were into 'wild card'*/
        return;
    }

    if(currUnit == SIM_LOG_FRAME_UNIT___ALLOW_ALL_UNITS___E)
    {
        /* the current is wild card , do not modify 'previous' */
    }
    else
    {
        currentLogFrameInfoPtr->prevUnitType = currUnit;
    }

    if(frameUnit == SIM_LOG_FRAME_UNIT___ALLOW_ALL_UNITS___E)
    {
        /* wildcard that allow the caller to temporary bypass ALL the 'filters'
           on unit names */
    }
    else
    if(frameUnit < SIM_LOG_FRAME_UNIT___ALLOW_ALL_UNITS___E)
    {
        if(restoredUnit != SIM_LOG_FRAME_UNIT___LAST___E)
        {
            /* we restored unit */
            __LOG_NO_LOCATION_META_DATA__WITH_SCIB_LOCK(("-------- Restore unit [%s] after [%s] -------- \n",
                unisNameArr[frameUnit] ?
                    unisNameArr[frameUnit] :
                    "unknown",
                unisNameArr[currentLogFrameInfoPtr->prevUnitType] ?
                    unisNameArr[currentLogFrameInfoPtr->prevUnitType] :
                    "unknown"
                    ));
        }
        else
        {
            __LOG_NO_LOCATION_META_DATA__WITH_SCIB_LOCK(("-------- Start unit [%s] -------- \n",
                unisNameArr[frameUnit] ?
                    unisNameArr[frameUnit] :
                    "unknown"));
        }
    }

    return;
}

/**
* @internal simLogPacketFrameUnitSet function
* @endinternal
*
* @brief   Set the unit Id of current thread (own thread).
*
* @param[in] frameUnit                -  the unit id
*                                       None.
*/
GT_VOID simLogPacketFrameUnitSet(
    IN SIM_LOG_FRAME_UNIT_ENT frameUnit
)
{
    if(!simLogIsOpenFlag)
    {
        return;
    }

    internal_simLogPacketFrameUnitSet(frameUnit);

}

/**
* @internal simLogPacketFrameCommandSet function
* @endinternal
*
* @brief   Set the command of current thread (own thread).
*
* @param[in] frameCommand             -  the command id
*                                       None.
*/
GT_VOID simLogPacketFrameCommandSet(
    IN SIM_LOG_FRAME_COMMAND_TYPE_ENT frameCommand
)
{
    SIM_LOG_FRAME_INFO_STC *currentLogFrameInfoPtr =
        simLogPacketFrameMyLogInfoGet();

    if(currentLogFrameInfoPtr == NULL)
    {
        return;
    }

    currentLogFrameInfoPtr->commandType = frameCommand;

    return;
}


#define SKERNEL_DEV_MAX_SUPPORTED_PORTS_CNS (16*17) /* was 128. In Falcon 12.8T we needed 16*17 MACs */
/* support ports per device */
#define MAX_PORTS_CNS   SKERNEL_DEV_MAX_SUPPORTED_PORTS_CNS

static GT_U32   devicePortsLinkArr[SMAIN_MAX_NUM_OF_DEVICES_CNS * (MAX_PORTS_CNS/32)] = {0};
static GT_U32   doPortsLinkCatchUp = 0;
/**
* @internal simLogLinkStateNotify function
* @endinternal
*
* @brief   handle indications about link change from the SLANs
*
* @param[in] devObjPtr                - pointer to device object
* @param[in] port                     -  number.
* @param[in] linkState                - link state (0 - down, 1 - up)
*/
void simLogLinkStateNotify
(
    IN SKERNEL_DEVICE_OBJECT      *     devObjPtr,
    IN GT_U32      port,
    IN GT_U32      linkState
)
{
    GT_U32   oldLinkState;
    GT_U32   allowDumpToLog = 0;
    GT_U32  deviceId,coreId;

    if(devObjPtr->portGroupSharedDevObjPtr)
    {
        deviceId =  devObjPtr->portGroupSharedDevObjPtr->deviceId;
        coreId   = devObjPtr->portGroupId;
    }
    else
    {
        deviceId =  devObjPtr->deviceId;
        coreId   = 0;
    }

    if(port >= MAX_PORTS_CNS)
    {
        skernelFatalError("simLogLinkStateNotify : port[%d] out of range \n" ,
             port);
        return;
    }

    oldLinkState = snetFieldValueGet(devicePortsLinkArr,
        (devObjPtr->deviceId * MAX_PORTS_CNS) + port ,
        1);

    if(doPortsLinkCatchUp &&
        linkState == 1)
    {
        /* allow dump to log when we do 'Catch up' of the ports that are 'UP' into the LOG */
        allowDumpToLog = 1;
    }
    else
    if(linkState != oldLinkState)
    {
        /* allow dump into the LOG when link state changed */
        allowDumpToLog = 1;
    }

    if(simLogIsOpenFlag == 0)
    {
        /* no dump to log when log is not open */
        allowDumpToLog = 0;
    }

    if(allowDumpToLog)
    {
        __LOG_NO_LOCATION_META_DATA__WITH_SCIB_LOCK((
            SIM_LOG_PORT_LINK_STATUS_CHANGED_PREFIX_STR
            SIM_LOG_PORT_LINK_STATUS_CHANGED_DETAILS_STR  "\n" ,
                deviceId , coreId , port ,linkState));
    }

    /* save the info for the case that the LOGGER will be opened later */
    snetFieldValueSet(devicePortsLinkArr,
        (devObjPtr->deviceId * MAX_PORTS_CNS) + port ,
        1 ,
        linkState );

}

/**
* @internal simLogLinkStateCatchUp function
* @endinternal
*
* @brief   Catch-Up the port link changes that where registered in the LOG , for the
*         'LOG to runtime' feature.
*/
static void simLogLinkStateCatchUp(void)
{
    GT_U32      linkState,deviceIndex,port;

    doPortsLinkCatchUp = 1;

    for(deviceIndex = 0 ; deviceIndex < SMAIN_MAX_NUM_OF_DEVICES_CNS; deviceIndex++)
    {
        if(smainDeviceObjects[deviceIndex] == NULL)
        {
            continue;
        }

        for(port = 0 ; port < MAX_PORTS_CNS; port++)
        {
            linkState = snetFieldValueGet(devicePortsLinkArr,deviceIndex*MAX_PORTS_CNS + port , 1);
            if(linkState == 0)
            {
                continue;
            }

            /* calling this function will also call : simLogLinkStateNotify(...) */
            snetLinkStateNotify(smainDeviceObjects[deviceIndex] , port , linkState);
        }
    }

    doPortsLinkCatchUp = 0;

    return;
}

/**
* @internal simLogAddDevicesInfo function
* @endinternal
*
* @brief   send to log all the info about the devices.
*/
void simLogAddDevicesInfo(void)
{
    GT_U32  deviceIndex,deviceId,coreId;
    SKERNEL_DEVICE_OBJECT *devObjPtr;
    static GT_U32 alreadyDumpInfo = 0;

    if(simulationInitReady == 0 || alreadyDumpInfo == 1 || (0 == simLogIsOpenFlag))
    {
        return;
    }

    alreadyDumpInfo = 1;

    devObjPtr = NULL;
    __LOG_NO_LOCATION_META_DATA__WITH_SCIB_LOCK(("start info about the devices in the system: \n"));

    for(deviceIndex = 0 ; deviceIndex < SMAIN_MAX_NUM_OF_DEVICES_CNS; deviceIndex++)
    {
        devObjPtr = smainDeviceObjects[deviceIndex];

        if(devObjPtr == NULL)
        {
            continue;
        }

        if(devObjPtr->shellDevice)
        {
            continue;
        }


        if(devObjPtr->portGroupSharedDevObjPtr)
        {
            deviceId =  devObjPtr->portGroupSharedDevObjPtr->deviceId;
            coreId   = devObjPtr->portGroupId;
        }
        else
        {
            deviceId =  devObjPtr->deviceId;
            coreId   = 0;
        }

        __LOG_NO_LOCATION_META_DATA__WITH_SCIB_LOCK(("deviceName[%s] deviceId [%d] coreId[%d] deviceRevisionId[%d] \n",
            devObjPtr->deviceName,
            deviceId , coreId,
            devObjPtr->deviceRevisionId));
    }

    devObjPtr = NULL;
    __LOG_NO_LOCATION_META_DATA__WITH_SCIB_LOCK(("End info about the devices in the system \n"));

}


typedef struct{
    SKERNEL_DEVICE_OBJECT       *devObjPtr;
    GT_U32                      portNumber;
    GT_BOOL                     bindRx;
    GT_BOOL                     bindTx;
    GT_CHAR                     *slanNamePtr;
}SIM_LOG_PORT_SLAN_INFO_STC;


static SIM_LOG_PORT_SLAN_INFO_STC simLogPortSlanInfoArr[MAX_PORTS_CNS];
static GT_U32 simLogPortSlanInfoArr_numElements = sizeof(simLogPortSlanInfoArr) / sizeof(simLogPortSlanInfoArr[0]);

/**
* @internal simLogSlanBind function
* @endinternal
*
* @brief   For the LOG to indicate the ports that do Bind/Unbind to slan.
*
* @param[in] slanNamePtr              - (pointer to) slan name , if NULL -->'unbind'
* @param[in] devObjPtr                (pointer to) the device object
* @param[in] portNumber               - port number
* @param[in] bindRx                   - bind to Rx direction ? GT_TRUE - yes , GT_FALSE - no
* @param[in] bindTx                   - bind to Tx direction ? GT_TRUE - yes , GT_FALSE - no
*                                       None
*/
void simLogSlanBind (
    IN char                         *slanNamePtr,
    IN SKERNEL_DEVICE_OBJECT        *devObjPtr,
    IN GT_U32                       portNumber,
    IN GT_BOOL                      bindRx,
    IN GT_BOOL                      bindTx
)
{
    GT_U32  ii;
    GT_U32  emptyIndex = SMAIN_NOT_VALID_CNS;
    GT_BOOL foundMatch = GT_FALSE;
    GT_U32  index;
    GT_U32  deviceId,coreId;

    if(devObjPtr->portGroupSharedDevObjPtr)
    {
        deviceId =  devObjPtr->portGroupSharedDevObjPtr->deviceId;
        coreId   = devObjPtr->portGroupId;
    }
    else
    {
        deviceId =  devObjPtr->deviceId;
        coreId   = 0;
    }

    if(simLogIsOpenFlag)
    {
        if(slanNamePtr) /*bind*/
        {
            __LOG_NO_LOCATION_META_DATA__WITH_SCIB_LOCK((
                SIM_LOG_SLAN_BIND_PREFIX_STR     /*bind*/
                SIM_LOG_SLAN_BIND_OPERATION_STR
                SIM_LOG_SLAN_BIND_DETAILS_STR  "\n" ,
                    slanNamePtr,
                    deviceId ,
                    coreId ,
                    portNumber ,
                    bindRx,
                    bindTx));
        }
        else /*unbind*/
        {
            __LOG_NO_LOCATION_META_DATA__WITH_SCIB_LOCK((
                SIM_LOG_SLAN_UNBIND_PREFIX_STR       /*unbind*/
                SIM_LOG_SLAN_BIND_DETAILS_STR  "\n" ,
                    deviceId ,
                    coreId ,
                    portNumber ,
                    bindRx,
                    bindTx));
        }
    }

    for(ii = 0 ; ii < simLogPortSlanInfoArr_numElements; ii++)
    {
        if(simLogPortSlanInfoArr[ii].slanNamePtr == NULL)
        {
            if(emptyIndex == SMAIN_NOT_VALID_CNS)
            {
                emptyIndex = ii;
            }

            /*the entry is not bound*/
            continue;
        }

        if(simLogPortSlanInfoArr[ii].devObjPtr != devObjPtr)
        {
            /* not the same device*/
            continue;
        }

        if(simLogPortSlanInfoArr[ii].portNumber != portNumber)
        {
            /* not the same port*/
            continue;
        }

        foundMatch = GT_TRUE;
        break;
    }

    if(foundMatch == GT_TRUE)
    {
        index = ii;
    }
    else
    {
        if(slanNamePtr == NULL)
        {
            skernelFatalError("simLogSlanBind: needed to unbind but not found in the DB ?! \n");
            return;
        }

        if(emptyIndex == SMAIN_NOT_VALID_CNS)
        {
            skernelFatalError("simLogSlanBind: DB full ?! \n");
            return;
        }
        index = emptyIndex;
    }

    simLogPortSlanInfoArr[index].devObjPtr    = devObjPtr;
    simLogPortSlanInfoArr[index].portNumber   = portNumber;
    simLogPortSlanInfoArr[index].slanNamePtr  = slanNamePtr;

    if(slanNamePtr == NULL)
    {
        if(bindRx == GT_TRUE)/*unbind*/
        {
            simLogPortSlanInfoArr[index].bindRx = GT_FALSE;
        }

        if(bindTx == GT_TRUE)/*unbind*/
        {
            simLogPortSlanInfoArr[index].bindTx = GT_FALSE;
        }
    }
    else
    {
        simLogPortSlanInfoArr[index].bindRx = bindRx;
        simLogPortSlanInfoArr[index].bindTx = bindTx;
    }
}

/**
* @internal simLogSlanCatchUp function
* @endinternal
*
* @brief   Catch-Up the SLAN changes that where registered in the LOG , for the
*         'LOG to runtime' feature.
*/
static void simLogSlanCatchUp(void)
{
    GT_U32  ii;

    for(ii = 0 ; ii < simLogPortSlanInfoArr_numElements; ii++)
    {
        if(simLogPortSlanInfoArr[ii].slanNamePtr == NULL)
        {
            /*the entry is not bound*/
            continue;
        }

        simLogSlanBind(simLogPortSlanInfoArr[ii].slanNamePtr,
                       simLogPortSlanInfoArr[ii].devObjPtr,
                       simLogPortSlanInfoArr[ii].portNumber,
                       simLogPortSlanInfoArr[ii].bindRx,
                       simLogPortSlanInfoArr[ii].bindTx);
    }

    return;
}

static GT_CHAR explicitName[257] = {0};
static GT_CHAR explicitFullPathName[257] = {0};
/* indication to log packets+memory+from_cpu to the log */
static GT_U32   simLogIsFull = 0;
/* indication to log packets+from_cpu to the log */
static GT_U32   simLogWithFromCpu = 0;

static char* lastModeStr = "ssl : packets";

/*
open GM/WM log
for WM :
    the log is opened for under c:\temp\dtemp\testLog.???.txt
for GM : (limited to BC2 only)
    The log is opened according to info in the 'gm.INI' file
Note: file override previous file by the same name.
*/
void wm__startSimulationLog(void)
{
    static GT_CHAR logFileName[512];

    if(gmLogSupport == GT_TRUE)
    {
        /* to log */
        simLogSetOutput  (GT_TRUE);
        simulationPrintf("sleep 3 sec ... to allow GM start log \n");
        SIM_OS_MAC(simOsSleep)(3000);
    }
    else
    {
        simulationPrintf("the WM log started with next FLAGS:[%s] (options are:memory/from_cpu/traffic/full) \n",
            lastModeStr);

        if (explicitFullPathName[0])
        {
            /* full path name + file name */
            sprintf(logFileName,"%s",explicitFullPathName);
            simulationPrintf("opening simulation LOG file [%s] \n",logFileName);
        }
        else
        {
            sprintf(logFileName,
#if (defined _WIN32)
                "c:\\temp\\dtemp\\testLog.%s.txt",  /*win32*/
#else
                "testLog.%s.txt",                  /*linux*/
#endif
                explicitName[0] ?
                    explicitName : "tgf");

#if (defined _WIN32)
            simulationPrintf("opening simulation LOG file [%s] \n",logFileName);
#else
            {
                static char dirName[1024];
                if (GET_CURRENT_DIR(dirName, sizeof(dirName)) == NULL)
                {
                    dirName[0] = 0;
                }
                simulationPrintf("opening simulation LOG file [%s/%s] \n",dirName,logFileName);
            }
#endif
        }


        if (GT_OK != simLogSetFileName (logFileName))
        {
            printf("SIMULATION ERROR: failed to open LOG file: [%s] \n",logFileName);
            return;
        }

        /* to log */
        simLogSetOutput  (GT_TRUE);
        /* packets processing*/
        simLogSetInfoTypeEnable (SIM_LOG_INFO_TYPE_PACKET_E,1);/*SIM_LOG_INFO_TYPE_PACKET_E*/

        /* read/write operations by CPU and by the device */
        simLogSetInfoTypeEnable (SIM_LOG_INFO_TYPE_MEMORY_E,simLogIsFull);/*SIM_LOG_INFO_TYPE_MEMORY_E*/

        /* simLogIsFull : allow to see memory on PCI config space */
        simLogSetThreadTypeFilter(SIM_OS_TASK_PURPOSE_TYPE_PP_PIPE_GENERAL_PURPOSE_E,simLogIsFull ? GT_FALSE : GT_TRUE);

        #define FROM_CPU_INDEX  0 /* 0 is the index */
        if(simLogWithFromCpu == 0)
        {
            /* don't allow to see the 'FROM_CPU' in the LOG !!!! */
            simLogFilterDenySpecificPacketCommandSet("from_cpu",FROM_CPU_INDEX);/* 0 is the index */
        }
        else
        {
            /* allow to see the 'FROM_CPU' in the LOG !!!! */
            simLogFilterDenySpecificPacketCommandSet(NULL,FROM_CPU_INDEX);/* 0 is the index */
        }

    }
}
/*stop dumping into the log*/
void wm__stopSimulationLog(void)
{
    simLogSetOutput  (GT_FALSE);

    if(gmLogSupport == GT_TRUE)
    {
        SIM_OS_MAC(simOsSleep)(3000);
    }

    simulationPrintf(" 'disabled' sending to the log \n");

    explicitName[0] = 0;
}
/*do log that include packets,memory,from_cpu*/
void wm__startSimulationLogFull(void)
{
    lastModeStr = "FULL : packets,memory,from_cpu";

    simLogIsFull = 1;
    simLogWithFromCpu = 1;

    wm__startSimulationLog();
    simLogIsFull = 0;
    simLogWithFromCpu = 0;
}

/*do log that include packets,from_cpu*/
void wm__startSimulationLogWithFromCpu(void)
{
    lastModeStr = "withFromCpu : packets,from_cpu";

    simLogWithFromCpu = 1;
    wm__startSimulationLog();
    simLogWithFromCpu = 0;
}

void startSimulationLogWithFileName(IN char* explicitUniqueName)
{
    strcpy(explicitName,explicitUniqueName);
    wm__startSimulationLog();
}
/* set full path name */
void setSimulationLogFullPathName(IN const char* fullPathName)
{
    strcpy(explicitFullPathName,fullPathName);
}
/* start simulation with full path name */
void startSimulationLogWithFullPathName(IN const char* fullPathName)
{
    setSimulationLogFullPathName(fullPathName);
    wm__startSimulationLog();
}

GT_STATUS simLogDevFilterPortGroupSet(
    IN GT_U32   simDevNum ,
    IN GT_U32  portGroupId ,
    IN GT_BOOL enableFilter)
{
    GT_U32  ii;
    GT_U32  iiFree = SMAIN_NOT_VALID_CNS;

    DEVICE_ID_CHECK_MAC(simDevNum);

    for(ii = 0; ii < SIM_LOG_MAX_DF; ii++)
    {
        /* search filter in array */
        if( simLogDevPortGroupFilters[ii].filterId &&
           (simLogDevPortGroupFilters[ii].devNum    == simDevNum) &&
           (simLogDevPortGroupFilters[ii].portGroup == portGroupId) )
        {
            if(enableFilter == GT_FALSE)
            {
                simLogDevPortGroupFilters[ii].filterId = 0;
            }

            return GT_OK;
        }
        else
        if(simLogDevPortGroupFilters[ii].filterId == 0 &&
           iiFree == SMAIN_NOT_VALID_CNS)
        {
            /* first free index */
            iiFree = ii;
        }
    }

    if(enableFilter == GT_FALSE)
    {
        /* not found ... never mind */
        return GT_OK;
    }

    if(iiFree == SMAIN_NOT_VALID_CNS)
    {
        /* the table is full */
        return GT_FULL;
    }

    simLogDevPortGroupFilters[iiFree].filterId = iiFree + 1;
    simLogDevPortGroupFilters[iiFree].devNum    = simDevNum;
    simLogDevPortGroupFilters[iiFree].portGroup = portGroupId;

    return GT_OK;
}

GT_STATUS simLogDevFilterSet(
    IN GT_U32   simDevNum ,
    IN GT_BOOL enableFilter)
{
    GT_STATUS   rc;
    SKERNEL_DEVICE_OBJECT * devObjPtr;

    DEVICE_ID_CHECK_MAC(simDevNum);

    devObjPtr = smemTestDeviceIdToDevPtrConvert(simDevNum);
    if(devObjPtr->shellDevice == GT_TRUE)
    {
        GT_U32                 coreId;           /* core iterator */
        for(coreId = 0 ; coreId < devObjPtr->numOfCoreDevs ; coreId++)
        {
            rc = simLogDevFilterPortGroupSet(simDevNum,coreId,enableFilter);
            if(rc != GT_OK)
            {
                return rc;
            }
        }
    }
    else
    {
        rc = simLogDevFilterPortGroupSet(simDevNum,0,enableFilter);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    return GT_OK;
}


GT_STATUS simLogDevFilterAllSet(
    IN GT_BOOL enableFilter)
{
    GT_STATUS   rc;
    SKERNEL_DEVICE_OBJECT * devObjPtr;
    GT_U32  deviceId,ii,deviceId_father;
    GT_U32                 coreId;           /* core iterator */

    /* clear the filter in array */
    for(ii = 0; ii < SIM_LOG_MAX_DF; ii++)
    {
        simLogDevPortGroupFilters[ii].filterId = 0;
    }
    if(enableFilter == GT_FALSE)
    {
        return GT_OK;
    }

    /* remove all devices exclude 'me' */
    for(deviceId = 0 ; deviceId < SMAIN_MAX_NUM_OF_DEVICES_CNS; deviceId++)
    {
        devObjPtr = smainDeviceObjects[deviceId];
        if(devObjPtr == NULL)
        {
            continue;
        }

        if(devObjPtr->portGroupSharedDevObjPtr)
        {
            /* core in the device */
            deviceId_father =  devObjPtr->portGroupSharedDevObjPtr->deviceId;
            coreId = devObjPtr->portGroupId;
            rc = simLogDevFilterPortGroupSet(deviceId_father,coreId,enableFilter);
        }
        else
        if(devObjPtr->shellDevice == GT_TRUE)
        {
            /* no LOG info for this 'shell' device */
        }
        else
        {
            rc = simLogDevFilterPortGroupSet(deviceId,0,enableFilter);
            if(rc != GT_OK)
            {
                return rc;
            }
        }
    }

    return GT_OK;
}

GT_STATUS simLogDevFilterAllButOneSet(
    IN GT_U32   simDevNum ,
    IN GT_BOOL enableFilter)
{
    GT_STATUS   rc;
    SKERNEL_DEVICE_OBJECT * devObjPtr;
    GT_U32      deviceId_father;
    GT_U32      coreId;           /* core iterator */

    DEVICE_ID_CHECK_MAC(simDevNum);

    rc = simLogDevFilterAllSet(enableFilter);
    if(rc != GT_OK)
    {
        return rc;
    }

    if(enableFilter == GT_FALSE)
    {
        return GT_OK;
    }

    devObjPtr = smemTestDeviceIdToDevPtrConvert(simDevNum);
    if(devObjPtr->portGroupSharedDevObjPtr)
    {
        /* core in the device */
        deviceId_father =  devObjPtr->portGroupSharedDevObjPtr->deviceId;
        coreId = devObjPtr->portGroupId;
        return simLogDevFilterPortGroupSet(deviceId_father,coreId,GT_FALSE);
    }
    else
    if(devObjPtr->shellDevice == GT_TRUE)
    {
        GT_U32                 coreId;           /* core iterator */
        SKERNEL_DEVICE_OBJECT *currDevObjPtr; /* current device object pointer */
        for(coreId = 0 ; coreId < devObjPtr->numOfCoreDevs ; coreId++)
        {
            currDevObjPtr = devObjPtr->coreDevInfoPtr[coreId].devObjPtr;
            if(currDevObjPtr != NULL)
            {
                rc = simLogDevFilterPortGroupSet(simDevNum,coreId,enableFilter);
                if(rc != GT_OK)
                {
                    return rc;
                }
            }
        }
    }
    else
    {
        return simLogDevFilterPortGroupSet(simDevNum,0,GT_FALSE);
    }

    return rc;
}

GT_STATUS simLogDevFilterAllButOnePortGroupSet(
    IN GT_U32   simDevNum ,
    IN GT_U32  portGroupId ,
    IN GT_BOOL enableFilter)
{
    SKERNEL_DEVICE_OBJECT * devObjPtr;

    DEVICE_ID_CHECK_MAC(simDevNum);

    devObjPtr = smemTestDeviceIdToDevPtrConvert(simDevNum);
    if(devObjPtr->portGroupSharedDevObjPtr)
    {
        /* core in the device */
        simDevNum =  devObjPtr->portGroupSharedDevObjPtr->deviceId;
    }

    return simLogDevFilterAllButOneSet(simDevNum , enableFilter);
}

/* short name for 'wm__startSimulationLog' to be called from terminal*/
GT_STATUS wm__ssl(void)
{
    lastModeStr = "ssl : packets";

    wm__startSimulationLog();
    return GT_OK;/* make the 'shell-execute' happy */
}
/* short name for 'wm__startSimulationLogWithFromCpu' to be called from terminal*/
GT_STATUS wm__sslw(void)
{
    wm__startSimulationLogWithFromCpu();
    return GT_OK;/* make the 'shell-execute' happy */
}
/* short name for 'wm__startSimulationLogFull' to be called from terminal*/
GT_STATUS wm__sslf(void)
{
    wm__startSimulationLogFull();
    return GT_OK;/* make the 'shell-execute' happy */
}

/* short name for 'wm__stopSimulationLog' to be called from terminal
   nssl stands for 'no start simulation log' */
GT_STATUS wm__nssl(void)
{
    wm__stopSimulationLog();
    return GT_OK;/* make the 'shell-execute' happy */
}

