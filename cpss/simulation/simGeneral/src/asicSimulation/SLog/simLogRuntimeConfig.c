
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
* @file simLogRuntimeConfig.c
*
* @brief simulation logger runtime configuration APIs
*
* @version   6
********************************************************************************
*/
#include <stdio.h>
#include <string.h>
#include <os/simOsBind/simOsBindTask.h>
#include <asicSimulation/SLog/simLog.h>

/* empty call - it needs to link simLogRuntimeConfig.c correctly */
/* it allow to run APIs from cpss command line                   */
GT_VOID simLogEmptyFunc(GT_VOID)
{
    return;
}

/**
* @internal simLogSetThreadTypeFilter function
* @endinternal
*
* @brief   This routine manages threads types in runtime.
*
* @param[in] threadType               - type of the thread
* @param[in] enable                   - value
*
* @retval GT_OK                    - success
* @retval GT_BAD_PARAM             - wrong param given
*/
GT_STATUS simLogSetThreadTypeFilter
(
    IN SIM_OS_TASK_PURPOSE_TYPE_ENT  threadType,
    IN GT_BOOL                       enable
)
{
    switch(threadType)
    {
        case SIM_OS_TASK_PURPOSE_TYPE_CPU_APPLICATION_E:
            simLogFilterTypeCpuApplication = enable;
            break;
        case SIM_OS_TASK_PURPOSE_TYPE_CPU_ISR_E:
            simLogFilterTypeCpuIsr = enable;
            break;
        case SIM_OS_TASK_PURPOSE_TYPE_PP_AGING_DAEMON_E:
            simLogFilterTypePpAgingDaemon = enable;
            break;
        case SIM_OS_TASK_PURPOSE_TYPE_PP_PIPE_PROCESSING_DAEMON_E:
            simLogFilterTypePpPipeProcessingDaemon = enable;
            break;
        case SIM_OS_TASK_PURPOSE_TYPE_PP_PIPE_SDMA_QUEUE_DAEMON_E:
            simLogFilterTypePpPipeSdmaQueueDaemon = enable;
            break;
        case SIM_OS_TASK_PURPOSE_TYPE_PP_PIPE_OAM_KEEP_ALIVE_DAEMON_E:
            simLogFilterTypePpPipeOamKeepAliveDaemon = enable;
            break;
        case SIM_OS_TASK_PURPOSE_TYPE_PP_PIPE_GENERAL_PURPOSE_E:
            simLogFilterTypePpPipeGeneralPurpose = enable;
            break;
        case SIM_OS_TASK_PURPOSE_TYPE_PP_PIPE_SOFT_RESET_E:
            simLogFilterTypePpPipeSoftReset = enable;
            break;
        case SIM_OS_TASK_PURPOSE_TYPE_PREQ_SRF_DAEMON_E:
            simLogFilterTypePreqSrfDaemon = enable;
            break;
        default:
            return GT_BAD_PARAM;
    }

    return GT_OK;
}


/**
* @internal simLogSetInfoTypeEnable function
* @endinternal
*
* @brief   This routine manages information types in runtime.
*
* @param[in] infoType                 - type of the info
* @param[in] enable                   - value
*
* @retval GT_OK                    -  success
* @retval GT_BAD_PARAM             -  wrong param given
*/
GT_STATUS simLogSetInfoTypeEnable
(
    IN SIM_LOG_INFO_TYPE_ENT infoType,
    IN GT_BOOL               enable
)
{
    switch(infoType)
    {
        case SIM_LOG_INFO_TYPE_PACKET_E:
            simLogInfoTypePacket = enable;
            break;
        case SIM_LOG_INFO_TYPE_DEVICE_E:
            simLogInfoTypeDevice = enable;
            break;
        case SIM_LOG_INFO_TYPE_TCAM_E:
            simLogInfoTypeTcam = enable;
            break;
        case SIM_LOG_INFO_TYPE_MEMORY_E:
            simLogInfoTypeMemory = enable;
            break;
        default:
            return GT_BAD_PARAM;
    }

    return GT_OK;
}

/**
* @internal simLogSetDevPortGroupFilter function
* @endinternal
*
* @brief   This routine manages dev num and specific port group in runtime.
*
* @param[in] devNum                   - device number
* @param[in] portGroup                -  number
* @param[in] enable                   - value
*
* @retval GT_OK                    -  success
* @retval GT_BAD_PARAM             -  wrong param given
*/
GT_STATUS simLogSetDevPortGroupFilter
(
    IN GT_U32     devNum,
    IN GT_U32     portGroup,
    IN GT_BOOL    enable
)
{
    GT_U32    i;
    GT_BOOL   found = GT_FALSE;

    /* search filter in array */
    for(i = 0; i != SIM_LOG_MAX_DF; i++)
    {
        if( simLogDevPortGroupFilters[i].filterId &&
           (simLogDevPortGroupFilters[i].devNum    == devNum) &&
           (simLogDevPortGroupFilters[i].portGroup == portGroup) )
        {
            found = GT_TRUE;
            if(enable)
            {
                simLogDevPortGroupFilters[i].devNum    = devNum;
                simLogDevPortGroupFilters[i].portGroup = portGroup;
                return GT_ALREADY_EXIST;
            }
            else
            {
                /* disable it */
                simLogDevPortGroupFilters[i].filterId = 0;
                return GT_OK;
            }
        }
    }

    if(!found)
    {
        /* add new filter */
        for(i = 0; i < SIM_LOG_MAX_DF; i++)
        {
            if(simLogDevPortGroupFilters[i].filterId == 0)
            {
                simLogDevPortGroupFilters[i].filterId  = i+1;
                simLogDevPortGroupFilters[i].devNum    = devNum;
                simLogDevPortGroupFilters[i].portGroup = portGroup;
                return GT_OK;
            }
        }
    }

    return GT_FAIL;
}


/**
* @internal simLogSetOutput function
* @endinternal
*
* @brief   This routine set ouput configuration.
*
* @param[in] outputType               - type of the output
*
* @retval GT_OK                    -  success
* @retval GT_BAD_PARAM             -  wrong param given
*/
GT_STATUS simLogSetOutput
(
    IN SIM_LOG_OUTPUT_TYPE_ENT outputType
)
{
    switch(outputType)
    {
        case SIM_LOG_OUTPUT_DISABLED_E:
            simLogOutputToLogfile = 0;
            simLogOutputToStdout  = 0;
            break;
        case SIM_LOG_OUTPUT_LOGFILE_E:
            simLogOutputToLogfile = 1;
            simLogOutputToStdout  = 0;
            break;
        case SIM_LOG_OUTPUT_STDOUT_E:
            simLogOutputToLogfile = 0;
            simLogOutputToStdout  = 1;
            break;
        case SIM_LOG_OUTPUT_BOTH_E:
            simLogOutputToLogfile = 1;
            simLogOutputToStdout  = 1;
            break;
        default:
            return GT_BAD_PARAM;
    }
    simLogIsOpenFlag = simLogIsOpen();
    return GT_OK;
}

/**
* @internal simLogSetFileName function
* @endinternal
*
* @brief   This routine set log file name.
*
* @param[in] fnamePtr                 - (pointer to) fileName.
*
* @retval GT_OK                    -  success
* @retval GT_BAD_PTR               -  wrong pointer
* @retval GT_FAIL                  -  General failure error. Should never happen.
*/
GT_STATUS simLogSetFileName
(
     IN GT_CHAR *fnamePtr
)
{
    if(NULL == fnamePtr)
    {
        simGeneralPrintf("simLogSetFileName: Wrong file name given\n");
        return GT_BAD_PTR;
    }

    strcpy(simLogFileName, fnamePtr);

    /* reopen log file */
    simLogClose();

    if( GT_OK != simLogFileOpen() )
    {
        return GT_FAIL;
    }

    return GT_OK;
}


