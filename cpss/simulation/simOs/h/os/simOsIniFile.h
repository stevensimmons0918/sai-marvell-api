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
* @file simOsIniFile.h
*
* @brief Operating System wrapper. Ini file facility.
*
* @version   1
********************************************************************************
*/

#ifndef EXPLICIT_INCLUDE_TO_SIM_OS_H_FILES
    #error "include to those H files should be only for bind purposes"
#endif /*!EXPLICIT_INCLUDE_TO_SIM_OS_H_FILES*/

#ifndef __simOsIniFileh
#define __simOsIniFileh

/************* Includes *******************************************************/

#ifdef __cplusplus
extern "C" {
#endif

/**
* @internal simOsGetCnfValue function
* @endinternal
*
* @brief   Gets a specified parameter value from ini file.
*
* @retval GT_TRUE                  - if the action succeeds
* @retval GT_FALSE                 - if the action fails
*/
GT_BOOL simOsGetCnfValue
(
    IN  char     *chapNamePtr,
    IN  char     *valNamePtr,
    IN  GT_U32   data_len,
    OUT char     *valueBufPtr
);

/**
* @internal simOsSetCnfFile function
* @endinternal
*
* @brief   Sets the config file name.
*
* @param[in] fileNamePtr              - pointer to file name
*                                       The value of the desired parameter
*
* @retval NULL                     - if no such parameter found
*/
void simOsSetCnfFile
(
    IN  char *fileNamePtr
);

#ifdef __cplusplus
}
#endif

#endif  /* __simOsIniFileh */


