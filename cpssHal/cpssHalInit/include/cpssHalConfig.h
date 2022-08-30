/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
*******************************************************************************/

//cpssHalConfig.h

#ifndef _cpssHalConfig_h_
#define _cpssHalConfig_h_

#ifdef __cplusplus
extern "C" {
#endif

#include <cpss/generic/cpssTypes.h>

/**
 * \file cpssHalConfig.h
 * \brief This file contains API for preprocessing config file data and create the PROFILE
 */

/**
 * \brief set file name for port config
 *
 * \param [in] fileName file name
 * \return [XP_STATUS] On success XP_NO_ERR.
 */

void cpssHalPlatformConfigFileNameSet(char *fileName);


/**
 * \brief Get file name for port config
 *
 * \return [Filename] char * or NULL
 */

char * cpssHalPlatformConfigFileNameGet(void);

/**
 * \brief Open targetConfig file
 *
 * \param [in] fileName file name
 * \return [XP_STATUS] On success XP_NO_ERR.
 */

FILE *cpssHalOpenPlatformConfig(char *targetConfig);

/**
 * \brief read config file line by line
 *
 * \param [in] fileName file name
 * \return [XP_STATUS] On success XP_NO_ERR.
 */



GT_STATUS cpssHalReadPlatformConfig(char *targetConfig, PROFILE_STC profile[],
                                    GT_U32 profileSize);

/**
 * \brief process the line read
 *
 * \param [in] line one character line from file
 * \param [in] lineNum lineNum from file
 *
 * \return [XP_STATUS] On success XP_NO_ERR.
 */


GT_STATUS cpssHalProcessPlatformConfigLine(char *line, int lineNum,
                                           PROFILE_STC profile[], GT_U32 profileSize);

/**
 * \brief create data for port
 *
 * \param [in] line one character line from file
 * \param [in] lineNum lineNum from file
 *
 * \return [XP_STATUS] On success XP_NO_ERR.
 */



GT_STATUS cpssHalProcessPortData(int lineNum, char *line,
                                 PROFILE_STC profile[], GT_U32 profileSize);


/**
 * \brief create data for port
 *
 * \param [in] PROFILE_STC
 * \param [in] profileSize
 * \param [in] portNum
 *
 * \return [XP_STATUS] On success XP_NO_ERR.
 */
int cpssHalGetProfileByPort(PROFILE_STC profile[], GT_U32 profileSize,
                            int portNum);

#ifdef __cplusplus
}
#endif

#endif
