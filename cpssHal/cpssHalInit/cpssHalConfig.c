/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
*******************************************************************************/

// file cpssHalConfig.c
//


#include "xpsInit.h"
#include "xpsLock.h"
#include "cpssHalUtil.h"
#include "gtEnvDep.h"
#include "cpss/common/cpssTypes.h"
#include "openXpsEnums.h"
#include "cpssHalProfile.h"
#include "cpssHalConfig.h"
#include "cpssHalDevice.h"


/* Format of Platform config File
 *
    #Port    lane    speed    DC  BW  HF  LF  sqlch   minLf   maxLf   minHf   maxHf
    56       0       25000   0   3   3   11  68      0       3       0       0
 *
 */
static char cpssHalPlatformConfigFile[XP_MAX_FILE_NAME_LEN] = {0};


void cpssHalPlatformConfigFileNameSet(char *fileName)
{
    memcpy(cpssHalPlatformConfigFile, fileName, XP_MAX_FILE_NAME_LEN);
}

char * cpssHalPlatformConfigFileNameGet(void)
{
    return cpssHalPlatformConfigFile;
}

FILE *cpssHalOpenPlatformConfig(char *platformConfig)
{
    FILE *fpConfig = NULL;

    if ((platformConfig == NULL) || (strcmp(platformConfig, "") <= 0))
    {
        printf("cpssHalOpenPlatformConfig(): No file to read *\n");
        return NULL;
    }

    fpConfig = fopen(platformConfig, "r");
    return fpConfig;
}

GT_STATUS cpssHalReadPlatformConfig(char *platformConfig, PROFILE_STC profile[],
                                    GT_U32 profileSize)
{
    FILE *fpConfig;
    GT_STATUS ret = GT_OK;
    char line[XP_MAX_CONFIG_LINE_LEN];
    int lineNum = 1;
    int errorCode = 0;

    fpConfig = cpssHalOpenPlatformConfig(platformConfig);
    if (fpConfig == NULL)
    {
        printf("Unable to open platform configuration file [%s]\n", platformConfig);
        return XP_ERR_FILE_OPEN;
    }

    printf("Parsing platform configuration file [%s]\n", platformConfig);

    while (!feof(fpConfig))
    {
        if (fgets(line, XP_MAX_CONFIG_LINE_LEN, fpConfig) != NULL)
        {
            ret = cpssHalProcessPlatformConfigLine(line, lineNum, profile, profileSize);
            if (ret != XP_NO_ERR)
            {
                printf("Failed to process line # %d file [%s]: %d.\n", lineNum, platformConfig,
                       ret);
                //fclose(fpConfig);
                //return ret;
            }
        }
        else if ((errorCode = ferror(fpConfig)) != 0)
        {
            printf("Error reading line # %d of file [%s]: %s \n", lineNum, platformConfig,
                   strerror(errorCode));
            fclose(fpConfig);
            return XP_ERR_INVALID_DATA;
        }
        ++lineNum;
    }

    fclose(fpConfig);
    return XP_NO_ERR;
}


GT_STATUS cpssHalProcessPlatformConfigLine(char *line, int lineNum,
                                           PROFILE_STC profile[], GT_U32 profileSize)
{
    GT_STATUS ret = GT_OK;
    int notSpace = 0;

    notSpace = strspn(line, " \t\n\r");
    if (line[notSpace] != '\0' &&
        line[notSpace] != '#')    // If not blank line or comment
    {
        ret = cpssHalProcessPortData(lineNum, line, profile, profileSize);
        if (ret != GT_OK)
        {
            return ret;
        }
    }

    return ret;
}



GT_STATUS cpssHalProcessPortData(int lineNum, char *line, PROFILE_STC profile[],
                                 GT_U32 profileSize)
{
    int ret = 0;
    int portIdx;
    int portNum = 0, laneNum = 0, speed = 0, sqlch = 0;
    int BW = 0, HF = 0, LF = 0, DC = 0, minLf = 0, maxLf = 0, minHf = 0, maxHf = 0;

    ret = sscanf(line, "%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\n",
                 &portNum, &laneNum, &speed, &DC, &BW, &HF, &LF, &sqlch, &minLf, &maxLf, &minHf,
                 &maxHf);

    portIdx = cpssHalGetProfileByPort(profile, profileSize, portNum);
    if ((portIdx != -1) && (laneNum < 8))
    {
        if (ret >= 12)
        {
            profile[portIdx].profileValue.portMap.rxParam[laneNum].avago.DC = DC;
            profile[portIdx].profileValue.portMap.rxParam[laneNum].avago.BW = BW;
            profile[portIdx].profileValue.portMap.rxParam[laneNum].avago.HF = HF;
            profile[portIdx].profileValue.portMap.rxParam[laneNum].avago.LF = LF;
            profile[portIdx].profileValue.portMap.rxParam[laneNum].avago.minLf = minLf;
            profile[portIdx].profileValue.portMap.rxParam[laneNum].avago.maxLf = maxLf;
            profile[portIdx].profileValue.portMap.rxParam[laneNum].avago.minHf = minHf;
            profile[portIdx].profileValue.portMap.rxParam[laneNum].avago.maxHf = maxHf;
            profile[portIdx].profileValue.portMap.rxParam[laneNum].avago.sqlch = sqlch;

            profile[portIdx].profileValue.portMap.isRxTxParamValid |= 1;
        }
        else if (ret >= 7)
        {
            profile[portIdx].profileValue.portMap.txParam[laneNum].avago.post = DC;
            profile[portIdx].profileValue.portMap.txParam[laneNum].avago.pre = BW;
            profile[portIdx].profileValue.portMap.txParam[laneNum].avago.pre2 = HF;
            profile[portIdx].profileValue.portMap.txParam[laneNum].avago.pre3 = LF;
            profile[portIdx].profileValue.portMap.txParam[laneNum].avago.atten = sqlch;

            profile[portIdx].profileValue.portMap.isRxTxParamValid |= 2;
        }
        else
        {
            printf("ERROR: Parsing line # %d in platform configuration. ret args=%d (expected 12 for Rx or 8 for Tx)\n",
                   lineNum, ret);
            printf("%s\n", line);
            return XP_ERR_INVALID_DATA;
        }
    }

    return GT_OK;
}

int cpssHalGetProfileByPort(PROFILE_STC profile[], GT_U32 profileSize,
                            int portNum)
{
    int index;

    for (index = 0; index < profileSize; index++)
    {
        if (profile[index].profileType == PROFILE_TYPE_PORT_MAP_E)
        {
            if (portNum == profile[index].profileValue.portMap.portNum)
            {
                return index;
            }
        }
        else if (profile[index].profileType == PROFILE_TYPE_LAST_E)
        {
            break;
        }
    }

    return -1;
}

