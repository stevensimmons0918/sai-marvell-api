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
* @file simOsLinuxIniFileEmu.c
*
* @brief Operating System wrapper. Ini file facility emulation.
* (For simulation on board)
*
* @version   1
********************************************************************************
*/

#include <os/simTypes.h>
#define EXPLICIT_INCLUDE_TO_SIM_OS_H_FILES
#include <asicSimulation/SInit/sinit.h>
#include <os/simOsIniFile.h>
#include <os/simOsTask.h>

/************* Defines ***********************************************/
typedef struct{
    char    chapterName[SIM_OS_CNF_FILE_MAX_LINE_LENGTH_CNS];
    char    fieldName[SIM_OS_CNF_FILE_MAX_LINE_LENGTH_CNS];
    char    fieldStringValue[SIM_OS_CNF_FILE_MAX_LINE_LENGTH_CNS];
}DB_EMULATED_INI_FILE_STC;

#define DB_EMULATED_INI_FILE_NUM_FIELDS_CNS         50

static GT_U32   dbEmulatedIniFileNumEntries = 0;

static DB_EMULATED_INI_FILE_STC dbEmulatedIniFile[DB_EMULATED_INI_FILE_NUM_FIELDS_CNS];

/************ Public Functions ************************************************/

/**
* @internal simOsGetCnfValue function
* @endinternal
*
* @brief   Gets a specified parameter value from ini file.
*
* @retval GT_OK                    - if the action succeeds
* @retval GT_FAIL                  - if the action fails
*/
GT_BOOL simOsGetCnfValue
(
    IN  char     *chapNamePtr,
    IN  char     *valNamePtr,
    IN  GT_U32   data_len,
    OUT char     *valueBufPtr
)
{
    GT_U32  ii;

    if(chapNamePtr == NULL || valNamePtr == NULL)
    {
        return GT_BAD_PTR;
    }

    for(ii = 0; ii < dbEmulatedIniFileNumEntries ; ii++)
    {
        if(0 == strcmp(dbEmulatedIniFile[ii].chapterName,chapNamePtr) &&
           0 == strcmp(dbEmulatedIniFile[ii].fieldName,valNamePtr))
        {
            if(valueBufPtr && data_len != 0)
            {
                /* we have a match ,return value from the DB */
                strncpy(valueBufPtr,dbEmulatedIniFile[ii].fieldStringValue,data_len);

                printf("simOsGetCnfValue : chapNamePtr[%s] , valNamePtr[%s] , valueBufPtr[%s] \n",
                    chapNamePtr,valNamePtr,valueBufPtr);
            }

            return GT_TRUE;
        }
    }

    return GT_FALSE;
}

/**
* @internal simOsSetCnfFile function
* @endinternal
*
* @brief   Sets the config file name.
*
* @retval NULL                     - if no such parameter found
*/
void simOsSetCnfFile
(
    IN  char *fileNamePtr
)
{
    /* do nothing */
    return;
}


/**
* @internal simOsSetCnfValue function
* @endinternal
*
* @brief   Sets a specified parameter value to the emulated ini file.
*
* @retval GT_OK                    - if the action succeeds
* @retval GT_BAD_PTR               - if one of the pointers is NULL
* @retval GT_BAD_PARAM             - if one of the strings is too long
* @retval GT_FULL                  - if the DB already full
*/
GT_STATUS simOsSetCnfValue
(
    IN  char     *chapNamePtr,
    IN  char     *valNamePtr,
    IN  char     *valueBufPtr
)
{

    printf("simOsSetCnfValue : chapNamePtr[%s] , valNamePtr[%s] , valueBufPtr[%s] \n",
        chapNamePtr ? chapNamePtr : "NULL" ,
        valNamePtr ? valNamePtr : "NULL",
        valueBufPtr ? valueBufPtr : "NULL");

    if(dbEmulatedIniFileNumEntries >= DB_EMULATED_INI_FILE_NUM_FIELDS_CNS)
    {
        return GT_FULL;
    }

    if(chapNamePtr == NULL || valNamePtr == NULL || valueBufPtr == NULL)
    {
        return GT_BAD_PTR;
    }

    /* check length of fields */
    if(strlen(chapNamePtr) >= SIM_OS_CNF_FILE_MAX_LINE_LENGTH_CNS)
    {
        return GT_BAD_PARAM;
    }

    if(strlen(valNamePtr) >= SIM_OS_CNF_FILE_MAX_LINE_LENGTH_CNS)
    {
        return GT_BAD_PARAM;
    }

    if(strlen(valueBufPtr) >= SIM_OS_CNF_FILE_MAX_LINE_LENGTH_CNS)
    {
        return GT_BAD_PARAM;
    }

    /* check if already in DB */
    if(GT_TRUE == simOsGetCnfValue(chapNamePtr,valNamePtr,0,NULL))
    {
        return GT_OK;
    }

    strcpy(dbEmulatedIniFile[dbEmulatedIniFileNumEntries].chapterName      ,chapNamePtr);
    strcpy(dbEmulatedIniFile[dbEmulatedIniFileNumEntries].fieldName        ,valNamePtr);
    strcpy(dbEmulatedIniFile[dbEmulatedIniFileNumEntries].fieldStringValue ,valueBufPtr);

    /* add entry to the DB */
    dbEmulatedIniFileNumEntries++;

    return GT_OK;
}

/**
* @internal simOsDumpCnfValues function
* @endinternal
*
* @brief   Dump all the values in the emulated ini file.
*
* @retval GT_OK                    - if the action succeeds
*/
GT_STATUS simOsDumpCnfValues(void)
{
    GT_U32  ii;

    for(ii = 0; ii < dbEmulatedIniFileNumEntries ; ii++)
    {
        printf("simOsDumpCnfValues : chapNamePtr[%s] , valNamePtr[%s] , valueBufPtr[%s] \n",
            dbEmulatedIniFile[ii].chapterName ,
            dbEmulatedIniFile[ii].fieldName,
            dbEmulatedIniFile[ii].fieldStringValue);
    }

    return GT_OK;
}


