/* Copyright (C) 2018 Marvell International Ltd.
   This program is provided "as is" without any warranty of any kind, and is distributed under the applicable Marvell limited use license agreement. */
/**
********************************************************************************
 * @file iDbgPdlInit.c   
 * @copyright
 *    (c), Copyright 2001, Marvell International Ltd.
 *    THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.
 *    NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT
 *    OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE
 *    DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.
 *    THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,
 *    IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.
********************************************************************************
 * 
 * @brief Debug lib initialization
 * 
 * @version   1 
********************************************************************************
*/
/* *************************************************************************
 *$              EXTERNAL DECLARATIONS (IMPORT)
 *!**************************************************************************
 *!*/
#include <pdl/common/pdlTypes.h>
#include <pdl/lib/pdlLib.h>
#include <pdl/lib/private/prvPdlLib.h>
#include <pdl/init/pdlInit.h>
#include <sys/stat.h>
#include <errno.h>
#include <stdio.h>
#include <time.h>
#include <stdarg.h>
#ifndef _WIN32
#include <dirent.h>
#include <linux/limits.h> /* needed for PATH_MAX */
#else
#include <windows.h>
#include <shlwapi.h>
#endif

#include <iDbgPdl/logger/iDbgPdlLogger.h>

#define PRV_IDBGPDL_FLOW_LOCK_MAC()     prvPdlLock(PDL_OS_LOCK_TYPE_LOGGER_E)
#define PRV_IDBGPDL_FLOW_UNLOCK_MAC()   prvPdlUnlock(PDL_OS_LOCK_TYPE_LOGGER_E)

static PDL_OS_CALLBACK_MEMALLOC_PTR * prvIDbgPdlMallocCallbackPtr;
static PDL_OS_CALLBACK_MEMFREE_PTR * prvIDbgPdlFreeCallbackPtr;
static PDL_OS_CALLBACK_PRINT_PTR * prvIDbgPdlPrintCallbackPtr;
#define PRV_IDBG_PDL_LOGGER_MALLOC_MAC  prvIDbgPdlMallocCallbackPtr
#define PRV_IDBG_PDL_LOGGER_FREE_MAC    prvIDbgPdlFreeCallbackPtr
#define PRV_IDBG_PDL_LOGGER_PRINT_MAC    if (prvIDbgPdlPrintCallbackPtr) prvIDbgPdlPrintCallbackPtr


#define PRV_IDBG_PDL_LOGGER_SUCCESS_OR_FAIL_MAC(result, successIsNegative) \
    (((result) == PDL_OK) ? ((!successIsNegative) ? "SUCCESS" : "FAIL") : ((!successIsNegative) ? "FAIL" : "SUCCESS"))

#ifdef _WIN32
#define PRV_IDBG_PDL_LOGGER_MAX_PATH_CNS    MAX_PATH
#else
#define PRV_IDBG_PDL_LOGGER_MAX_PATH_CNS    PATH_MAX
#endif

static FILE * prvIDbgPdlLoggerOpenedFilePtr = NULL;
static char   prvIDbgPdlLoggerRootPathhNamePtr[PRV_IDBG_PDL_LOGGER_MAX_PATH_CNS] = {0};
static char   prvIDbgPdlLoggerCurrentPathNamePtr[PRV_IDBG_PDL_LOGGER_MAX_PATH_CNS] = {0};
static char   prvIDbgPdlLoggerCurrentFileNamePtr[PRV_IDBG_PDL_LOGGER_MAX_PATH_CNS] = {0};
static UINT_32 prvIDbgPdlLoggerCurrentRunIndex = 0;
static char   *prvIDbgPdlLoggerResultDescriptionBuffer = NULL;
static char   *prvIDbgPdlLoggerResultDescriptionTempBuffer = NULL;
static char   *prvIDbgPdlLoggerApiDescriptionBuffer = NULL;
static char   *prvIDbgPdlLoggerDebugDescriptionBuffer = NULL;
#define PRV_IDBG_PDL_LOGGER_MAX_DESCRIPTION_BUFFER_SIZE_CNS (64*1024)

typedef enum {
    PRV_IDBG_PDL_LOGGER_FILE_COMMAND_START_TAG_E,
    PRV_IDBG_PDL_LOGGER_FILE_COMMAND_RESULT_TAG_E,
    PRV_IDBG_PDL_LOGGER_FILE_COMMAND_END_TAG_E,
    PRV_IDBG_PDL_LOGGER_FILE_API_START_TAG_E,
    PRV_IDBG_PDL_LOGGER_FILE_API_IN_PARAM_TAG_E,
    PRV_IDBG_PDL_LOGGER_FILE_API_AUTO_IN_PARAM_TAG_E,
    PRV_IDBG_PDL_LOGGER_FILE_API_OUT_PARAM_TAG_E,
    PRV_IDBG_PDL_LOGGER_FILE_API_AUTO_OUT_PARAM_TAG_E,
    PRV_IDBG_PDL_LOGGER_FILE_API_RESULT_TEXT_TAG_E,
    PRV_IDBG_PDL_LOGGER_FILE_API_RESULT_TAG_E,
    PRV_IDBG_PDL_LOGGER_FILE_PDL_START_TAG_E,
    PRV_IDBG_PDL_LOGGER_FILE_PDL_IN_PARAM_TAG_E,
    PRV_IDBG_PDL_LOGGER_FILE_PDL_AUTO_IN_PARAM_TAG_E,
    PRV_IDBG_PDL_LOGGER_FILE_PDL_OUT_PARAM_TAG_E,
    PRV_IDBG_PDL_LOGGER_FILE_PDL_AUTO_OUT_PARAM_TAG_E,
    PRV_IDBG_PDL_LOGGER_FILE_PDL_DEBUG_TEXT_TAG_E,
    PRV_IDBG_PDL_LOGGER_FILE_PDL_RESULT_TAG_E
} PRV_IDBG_PDL_LOGGER_FILE_CONTENT_TAGS_ENT;

typedef struct {
    UINT_32     tagId;
    char       *tagNamePtr;
    char       *formatStringPtr;
    char       *paramReadFormatStringPtr;
} PRV_IDBG_PDL_LOGGER_FILE_TAG_DB_STC;

#define PRV_IDBG_PDL_LOGGER_FILE_TAG_2_ID_MAC(tag_name) \
    PRV_IDBG_PDL_LOGGER_BULD_NAME_MAC(PRV_IDBG_PDL_LOGGER_FILE_ , tag_name, _TAG_E)

#define PRV_IDBG_PDL_LOGGER_ADD_FILE_TAG_MAC(tag_name, fileFormat, paramReadFormat) \
    {PRV_IDBG_PDL_LOGGER_FILE_TAG_2_ID_MAC(tag_name), PRV_IDBG_PDL_LOGGER_STRINGIFY_MAC(tag_name), fileFormat, paramReadFormat}

static PRV_IDBG_PDL_LOGGER_FILE_TAG_DB_STC prvIDbgPdlLoggerTagDbArr[] = {
    PRV_IDBG_PDL_LOGGER_ADD_FILE_TAG_MAC(COMMAND_START,              "%s: TYPE=%s ID=<%s> START_TIME=%s"    , "TYPE=%[A-Z0-9_] ID=<%[a-zA-Z0-9_ ]> START_TIME=%[a-zA-Z0-9_ %:]"),
    PRV_IDBG_PDL_LOGGER_ADD_FILE_TAG_MAC(COMMAND_RESULT,             "%s: %s(%d)\n"                         , "%[A-Z](%[0-9])"),
    PRV_IDBG_PDL_LOGGER_ADD_FILE_TAG_MAC(COMMAND_END,                "%s: END_TIME=%s"                      , "END_TIME=%[a-zA-Z0-9_ %:]"),
    PRV_IDBG_PDL_LOGGER_ADD_FILE_TAG_MAC(API_START,                  "%s: NAME=[%s] START_TIME=%s"          , "NAME=%[a-zA-Z0-9_] START_TIME=%[a-zA-Z0-9_ %:]"),
    PRV_IDBG_PDL_LOGGER_ADD_FILE_TAG_MAC(API_IN_PARAM,               "%s: NAME=[%s] VALUE=[%s]\n"           , "NAME=[%[a-zA-Z0-9_]] VALUE=[%[a-zA-Z0-9_]]"),
    PRV_IDBG_PDL_LOGGER_ADD_FILE_TAG_MAC(API_AUTO_IN_PARAM,          "%s: NAME=[%s] VALUE=[%s]\n"           , "NAME=[%[a-zA-Z0-9_]] VALUE=[%[a-zA-Z0-9_]]"),
    PRV_IDBG_PDL_LOGGER_ADD_FILE_TAG_MAC(API_OUT_PARAM,              "%s: NAME=[%s] VALUE=[%s]\n"           , "NAME=[%[a-zA-Z0-9_]] VALUE=[%[a-zA-Z0-9_]]"),
    PRV_IDBG_PDL_LOGGER_ADD_FILE_TAG_MAC(API_AUTO_OUT_PARAM,         "%s: NAME=[%s] VALUE=[%s]\n"           , "NAME=[%[a-zA-Z0-9_]] VALUE=[%[a-zA-Z0-9_]]"),
    PRV_IDBG_PDL_LOGGER_ADD_FILE_TAG_MAC(API_RESULT_TEXT,            "%s: %s\n"                             , "%[a-zA-Z0-9%.,&*^(^)@#!-=+_ ]"),
    PRV_IDBG_PDL_LOGGER_ADD_FILE_TAG_MAC(API_RESULT,                 "%s: RESULT=%s(%s = %d) END_TIME=%s"   , "RESULT=%[A-Z](%[A-Z_] = %[0-9]) END_TIME=%[a-zA-Z0-9_ %:]"),
    PRV_IDBG_PDL_LOGGER_ADD_FILE_TAG_MAC(PDL_START,                  "%s: NAME=%s START_TIME=%s"            , "NAME=%[a-zA-Z0-9_] ID=<%[a-zA-Z0-9_ ]> START_TIME=%[a-zA-Z0-9_ %:]"),
    PRV_IDBG_PDL_LOGGER_ADD_FILE_TAG_MAC(PDL_IN_PARAM,               "%s: NAME=[%s] VALUE=[%s]\n"           , "NAME=[%[a-zA-Z0-9_]] VALUE=[%[a-zA-Z0-9_]]"),
    PRV_IDBG_PDL_LOGGER_ADD_FILE_TAG_MAC(PDL_AUTO_IN_PARAM,          "%s: NAME=[%s] VALUE=[%s]\n"           , "NAME=[%[a-zA-Z0-9_]] VALUE=[%[a-zA-Z0-9_]]"),
    PRV_IDBG_PDL_LOGGER_ADD_FILE_TAG_MAC(PDL_OUT_PARAM,              "%s: NAME=[%s] VALUE=[%s]\n"           , "NAME=[%[a-zA-Z0-9_]] VALUE=[%[a-zA-Z0-9_]]"),
    PRV_IDBG_PDL_LOGGER_ADD_FILE_TAG_MAC(PDL_AUTO_OUT_PARAM,         "%s: NAME=[%s] VALUE=[%s]\n"           , "NAME=[%[a-zA-Z0-9_]] VALUE=[%[a-zA-Z0-9_]]"),
    PRV_IDBG_PDL_LOGGER_ADD_FILE_TAG_MAC(PDL_DEBUG_TEXT,             "%s: %s\n"                             , "%[a-zA-Z0-9%.,&*^(^)@#!-=+_ ]"),
    PRV_IDBG_PDL_LOGGER_ADD_FILE_TAG_MAC(PDL_RESULT,                 "%s: RESULT=%s(%s = %d) END_TIME=%s"   , "RESULT=%[A-Z](%[A-Z_] = %[0-9]) END_TIME=%[a-zA-Z0-9_ %:]")
};

#define PRV_IDBG_PDL_LOGGER_GET_FILE_TAG_NAME_MAC(tag_value) prvIDbgPdlLoggerTagDbArr[PRV_IDBG_PDL_LOGGER_FILE_TAG_2_ID_MAC(tag_value)].tagNamePtr
#define PRV_IDBG_PDL_LOGGER_GET_FILE_TAG_FORMAT_MAC(tag_value) prvIDbgPdlLoggerTagDbArr[PRV_IDBG_PDL_LOGGER_FILE_TAG_2_ID_MAC(tag_value)].formatStringPtr
#define PRV_IDBG_PDL_LOGGER_GET_PARAM_READ_FORMAT_MAC(tag_id) prvIDbgPdlLoggerTagDbArr[tag_id].paramReadFormatStringPtr

typedef struct {
    UINT_32     categoryId;
    char       *categoryNamePtr;
} PRV_IDBG_PDL_LOGGER_CATEGORY_DB_STC;

#define PRV_IDBG_PDL_LOGGER_ADD_CATEGORY_MAC(category_name) \
    {PRV_IDBG_PDL_LOGGER_BULD_NAME_MAC(IDBG_PDL_LOGGER_CATEGORY_ , category_name, _E), PRV_IDBG_PDL_LOGGER_STRINGIFY_MAC(category_name)}

#ifdef INTERFACE
#undef INTERFACE
#endif
#ifdef CPU
#undef CPU
#endif
static PRV_IDBG_PDL_LOGGER_CATEGORY_DB_STC prvPdlLogCategoryDbArr[] = {
    PRV_IDBG_PDL_LOGGER_ADD_CATEGORY_MAC(BUTTON   ),
    PRV_IDBG_PDL_LOGGER_ADD_CATEGORY_MAC(CPU      ),
    PRV_IDBG_PDL_LOGGER_ADD_CATEGORY_MAC(FAN      ),
    PRV_IDBG_PDL_LOGGER_ADD_CATEGORY_MAC(INTERFACE),
    PRV_IDBG_PDL_LOGGER_ADD_CATEGORY_MAC(LED      ),
    PRV_IDBG_PDL_LOGGER_ADD_CATEGORY_MAC(OOB      ),
    PRV_IDBG_PDL_LOGGER_ADD_CATEGORY_MAC(PP       ),
    PRV_IDBG_PDL_LOGGER_ADD_CATEGORY_MAC(PHY      ),
    PRV_IDBG_PDL_LOGGER_ADD_CATEGORY_MAC(POWER    ),
    PRV_IDBG_PDL_LOGGER_ADD_CATEGORY_MAC(SENSOR   ),
    PRV_IDBG_PDL_LOGGER_ADD_CATEGORY_MAC(SERDES   ),
    PRV_IDBG_PDL_LOGGER_ADD_CATEGORY_MAC(SFP      ),
    PRV_IDBG_PDL_LOGGER_ADD_CATEGORY_MAC(SYSTEM   ),
    PRV_IDBG_PDL_LOGGER_ADD_CATEGORY_MAC(LAST     )
};

static void * prvIDbgPdlLoggerGetEntry(void * arrPtr, UINT_32 sizeofEntry, UINT_32 sizeOfArr, UINT_32 value)
{
    UINT_32 i;
    for (i = 0; i < sizeOfArr; i++)
        if (*(UINT_32 *)((UINT_8 *)arrPtr + sizeofEntry * i) == value)
            return ((UINT_8 *)arrPtr + sizeofEntry * i);
    return NULL;
}

/*****************************************************************************
* FUNCTION NAME: prvLoggerIsDirEmpty
*
* DESCRIPTION: 
*
*
*****************************************************************************/
static BOOLEAN prvIDbgPdlLoggerIsDirEmpty (

    /*!     INPUTS:             */
    char*      rootPathPtr,
    char*      dirPathPtr
    /*!     INPUTS / OUTPUTS:   */

    /*!     OUTPUTS:            */
)
{
/*!****************************************************************************/
/*! L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/*!****************************************************************************/
    char           *pathPtr;
    UINT_32         num = 0, path_len;
    BOOLEAN         result = FALSE;
#ifndef _WIN32
    DIR            *dirHndlPtr;
    struct dirent  *de;
#endif
/*!****************************************************************************/
/*!                      F U N C T I O N   L O G I C                          */
/*!****************************************************************************/
    path_len = strlen(rootPathPtr) + strlen(dirPathPtr) + 2;
    pathPtr = (char *)PRV_IDBG_PDL_LOGGER_MALLOC_MAC(path_len);

    if (pathPtr)
    {
        prvPdlOsSnprintf(pathPtr, path_len, "%s/%s", rootPathPtr, dirPathPtr);

#ifndef _WIN32
        dirHndlPtr = opendir(pathPtr);
        if (dirHndlPtr)
        {
            while ((de = readdir(dirHndlPtr)) != NULL)
            {
                num ++;
                if (num > 2)
                    break;
            }

            if (num <= 2)
                result = TRUE;

            closedir(dirHndlPtr);
        }
        else
#else
        if (PathIsDirectoryEmpty((LPCSTR)pathPtr))
#endif
            result = TRUE;

        PRV_IDBG_PDL_LOGGER_FREE_MAC(pathPtr);
    }
    return result;
}
/*$ END OF prvLoggerIsDirEmpty */

/*****************************************************************************
* FUNCTION NAME: prvIDbgPdlLoggerCreateCmdDir
*
* DESCRIPTION: 
*
*
*****************************************************************************/
static PDL_STATUS prvIDbgPdlLoggerCreateCmdDir (

    /*!     INPUTS:             */
#ifndef _WIN32
    DIR*            mainDirPtr,
#endif
     char*          rootPathPtr
    /*!     INPUTS / OUTPUTS:   */

    /*!     OUTPUTS:            */
)
{
/*!****************************************************************************/
/*! L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/*!****************************************************************************/
#ifndef _WIN32
    struct dirent  *de;  /* Pointer for directory entry  */
#else
    HANDLE           hFile;
    WIN32_FIND_DATA  FindFileData;
    BOOLEAN          moreFiles;
#endif
    PDL_STATUS      pdlStatus = PDL_BAD_PARAM;
    UINT_32         dirIndex, maxDirIndex = 0, path_len;
    char           *pathPtr;
/*!****************************************************************************/
/*!                      F U N C T I O N   L O G I C                          */
/*!****************************************************************************/
    path_len = strlen(rootPathPtr) + 11;
    pathPtr = (char *)PRV_IDBG_PDL_LOGGER_MALLOC_MAC(path_len);
    if (pathPtr == NULL)
        return PDL_NO_RESOURCE;

#ifndef _WIN32
    rewinddir(mainDirPtr);
    while ((de = readdir(mainDirPtr)) != NULL)
    {
        sscanf(de->d_name, "run_%d", &dirIndex);
        if (sscanf(de->d_name, "run_%d", &dirIndex) && dirIndex >= maxDirIndex)
        {
            maxDirIndex = dirIndex;
            if (prvIDbgPdlLoggerIsDirEmpty(rootPathPtr, de->d_name) == FALSE)
                maxDirIndex ++;
        }
    }

    /*validate directory is not empty */
    if (maxDirIndex == 0)
    {
        maxDirIndex ++;
    }
#else
    prvPdlOsSnprintf(pathPtr, path_len, "%s/*", rootPathPtr);
    for (moreFiles = ((hFile = FindFirstFile((LPCSTR)pathPtr, &FindFileData)) != INVALID_HANDLE_VALUE) ? TRUE : FALSE;
         moreFiles;
         moreFiles = FindNextFile(hFile, &FindFileData))
    {
        if ((FindFileData.dwFileAttributes == INVALID_FILE_ATTRIBUTES) || !(FindFileData.dwFileAttributes  & FILE_ATTRIBUTE_DIRECTORY))
            continue;

        if (sscanf((char *)FindFileData.cFileName, "run_%d", &dirIndex) && dirIndex >= maxDirIndex)
        {
            maxDirIndex = dirIndex;
            if (prvIDbgPdlLoggerIsDirEmpty(rootPathPtr, (char *)FindFileData.cFileName) == FALSE)
                maxDirIndex ++;
        }
    }

    /*validate directory is not empty */
    if (maxDirIndex == 0)
    {
        maxDirIndex ++;
    }

    if (hFile != INVALID_HANDLE_VALUE)
        FindClose(hFile);
#endif

    /* create next directory */
    prvPdlOsSnprintf(pathPtr, path_len, "%s/run_%05d", rootPathPtr, maxDirIndex);
#ifndef _WIN32
    if (mkdir(pathPtr, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) && errno != EEXIST)
#else
    if ((CreateDirectory((LPCSTR)pathPtr, NULL) == FALSE) && (ERROR_ALREADY_EXISTS != GetLastError()))
#endif
    {
        pdlStatus = PDL_NO_RESOURCE;
    }
    else
    {
        prvPdlOsSnprintf(prvIDbgPdlLoggerRootPathhNamePtr, PRV_IDBG_PDL_LOGGER_MAX_PATH_CNS, "%s", rootPathPtr);
        prvPdlOsSnprintf(prvIDbgPdlLoggerCurrentPathNamePtr, PRV_IDBG_PDL_LOGGER_MAX_PATH_CNS, "%s", pathPtr);
        pdlStatus = PDL_OK;
    }

    PRV_IDBG_PDL_LOGGER_FREE_MAC(pathPtr);

    return pdlStatus;
}
/*$ END OF prvIDbgPdlLoggerCreateCmdDir */

/*****************************************************************************
* FUNCTION NAME: prvIDbgPdlLoggerStrTagGet
*
* DESCRIPTION: 
*
*
*****************************************************************************/
static BOOLEAN prvIDbgPdlLoggerStrTagGet (

    /*!     INPUTS:             */
    IN char    *   inputStrPtr,
    /*!     INPUTS / OUTPUTS:   */
    IN char    *   delimStrPtr,
    /*!     OUTPUTS:            */
    OUT char    *  outputStrPtr
)
{
/*!****************************************************************************/
/*! L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/*!****************************************************************************/
    char * ch;
/*!****************************************************************************/
/*!                      F U N C T I O N   L O G I C                          */
/*!****************************************************************************/
    ch = strstr(inputStrPtr, delimStrPtr);
    if (ch)
    {
        memcpy(outputStrPtr, inputStrPtr, ch - inputStrPtr);
        outputStrPtr[ch - inputStrPtr] = '\0';
        return TRUE;
    }

    return FALSE;
}
/*$ END OF prvIDbgPdlLoggerStrTagGet */

/*****************************************************************************
* FUNCTION NAME: prvIDbgPdlLoggerParseFilename
*
* DESCRIPTION: 
*
*
*****************************************************************************/
static BOOLEAN prvIDbgPdlLoggerParseFilename (

    /*!     INPUTS:             */
    char*           fullFileNamePtr,
    UINT_32*        commandIndexPtr,
    char*           categoryNamePtr,
    char*           apiNamePtr

    /*!     INPUTS / OUTPUTS:   */

    /*!     OUTPUTS:            */
)
{
/*!****************************************************************************/
/*! L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/*!****************************************************************************/
    char                                *ch;
    UINT_32                             i;
    PRV_IDBG_PDL_LOGGER_CATEGORY_DB_STC *entryPtr = NULL;
/*!****************************************************************************/
/*!                      F U N C T I O N   L O G I C                          */
/*!****************************************************************************/

    if ((ch = strtok(fullFileNamePtr, "_")) != NULL)
    {
        if (sscanf(ch, "%05d", commandIndexPtr) == 0)
            return FALSE;
    }
    else
    	return FALSE;

    if ((ch = strtok(NULL, "_")) != NULL)
    {
        if (sscanf(ch, "%s", categoryNamePtr) == 0)
            return FALSE;
        /* validate category name */
        for (i = 0; i < sizeof(prvPdlLogCategoryDbArr)/sizeof(prvPdlLogCategoryDbArr[0]); i++)
        {
            entryPtr = &prvPdlLogCategoryDbArr[i];
            if (strcmp(categoryNamePtr, entryPtr->categoryNamePtr) == 0)
                break;
        }
        if (entryPtr == NULL)
            return FALSE;
    }
    else
    	return FALSE;

    ch += strlen(ch) + 1;
    if (FALSE == prvIDbgPdlLoggerStrTagGet(ch, ".log", apiNamePtr))
    {
    	return FALSE;
    }

    return TRUE;
}
/*$ END OF prvIDbgPdlLoggerParseFilename */

/*****************************************************************************
* FUNCTION NAME: prvIDbgPdlLoggerShowSingleCmdResults
*
* DESCRIPTION: 
*
*
*****************************************************************************/
static PDL_STATUS prvIDbgPdlLoggerShowSingleCmdResults (

    /*!     INPUTS:             */
    char*          fullFileNamePtr,
    char*          currFileNamePtr
    /*!     INPUTS / OUTPUTS:   */

    /*!     OUTPUTS:            */
)
{
/*!****************************************************************************/
/*! L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/*!****************************************************************************/
    UINT_32 commandIndex, i, result = 0;
    char    categoryName[80] = {0}, cmdApi[80] = {0}, funcName[80] = {0},
            tag[80], line[256], time[30], param[80], text[80], cliCmd[80], *lineStartPtr, resultStringPtr[20];
    FILE   *fHandlePtr;
    PRV_IDBG_PDL_LOGGER_FILE_TAG_DB_STC *entryPtr;
/*!****************************************************************************/
/*!                      F U N C T I O N   L O G I C                          */
/*!****************************************************************************/

    if (prvIDbgPdlLoggerParseFilename(currFileNamePtr, &commandIndex, categoryName, cmdApi) == FALSE)
        return PDL_BAD_PARAM;

    fHandlePtr = fopen(fullFileNamePtr, "r");
    if (fHandlePtr == NULL)
        return PDL_NOT_FOUND;

    /* parse file content */
    while (fgets(line, sizeof(line), fHandlePtr))
    {
        if (prvIDbgPdlLoggerStrTagGet(line, ": ", tag) == TRUE)
        {
            lineStartPtr = &line[strlen(tag) + 2];
        }
        else
            continue;

        text[0] = '\0';
        for (i = 0; i < sizeof(prvIDbgPdlLoggerTagDbArr)/sizeof(prvIDbgPdlLoggerTagDbArr[0]); i++)
        {
            entryPtr = &prvIDbgPdlLoggerTagDbArr[i];
            if (strcmp(tag, entryPtr->tagNamePtr) == 0)
            {
                /* handle single line */
                switch (entryPtr->tagId)
                {
                    case PRV_IDBG_PDL_LOGGER_FILE_COMMAND_START_TAG_E           :
                        sscanf(lineStartPtr, PRV_IDBG_PDL_LOGGER_GET_PARAM_READ_FORMAT_MAC(entryPtr->tagId), param, cliCmd, time);
                        PRV_IDBG_PDL_LOGGER_PRINT_MAC("\nCommand [%s] started @ [%s] called from [%s]\n", cmdApi, time, cliCmd);
                        break;
                    case PRV_IDBG_PDL_LOGGER_FILE_COMMAND_RESULT_TAG_E          :
                        sscanf(lineStartPtr, PRV_IDBG_PDL_LOGGER_GET_PARAM_READ_FORMAT_MAC(entryPtr->tagId), param, &result);
                        break;
                    case PRV_IDBG_PDL_LOGGER_FILE_API_RESULT_TEXT_TAG_E:
                    case PRV_IDBG_PDL_LOGGER_FILE_PDL_DEBUG_TEXT_TAG_E          :
                        sscanf(lineStartPtr, PRV_IDBG_PDL_LOGGER_GET_PARAM_READ_FORMAT_MAC(entryPtr->tagId), text);
                        break;
                    case PRV_IDBG_PDL_LOGGER_FILE_COMMAND_END_TAG_E             :
                        sscanf(lineStartPtr, PRV_IDBG_PDL_LOGGER_GET_PARAM_READ_FORMAT_MAC(entryPtr->tagId), time);
                        PRV_IDBG_PDL_LOGGER_PRINT_MAC("Command [%s] ended @ [%s] with <%s> code [%d]\n", cmdApi, time, param, result);
                        if (text[0])
                            PRV_IDBG_PDL_LOGGER_PRINT_MAC("Description == %s ==\n", text);
                        break;
                    case PRV_IDBG_PDL_LOGGER_FILE_API_START_TAG_E               :
                    case PRV_IDBG_PDL_LOGGER_FILE_PDL_START_TAG_E               :
                        sscanf(lineStartPtr, PRV_IDBG_PDL_LOGGER_GET_PARAM_READ_FORMAT_MAC(entryPtr->tagId), funcName, time);
                        PRV_IDBG_PDL_LOGGER_PRINT_MAC("CALLED api [%s] @ [%s]", funcName, time);
                        break;
                    case PRV_IDBG_PDL_LOGGER_FILE_API_IN_PARAM_TAG_E            :
                    case PRV_IDBG_PDL_LOGGER_FILE_API_AUTO_IN_PARAM_TAG_E       :
                    case PRV_IDBG_PDL_LOGGER_FILE_API_OUT_PARAM_TAG_E           :
                    case PRV_IDBG_PDL_LOGGER_FILE_API_AUTO_OUT_PARAM_TAG_E      :
                    case PRV_IDBG_PDL_LOGGER_FILE_PDL_IN_PARAM_TAG_E            :
                    case PRV_IDBG_PDL_LOGGER_FILE_PDL_AUTO_IN_PARAM_TAG_E       :
                    case PRV_IDBG_PDL_LOGGER_FILE_PDL_OUT_PARAM_TAG_E           :
                    case PRV_IDBG_PDL_LOGGER_FILE_PDL_AUTO_OUT_PARAM_TAG_E      :
                        break;
                    case PRV_IDBG_PDL_LOGGER_FILE_API_RESULT_TAG_E              :
                    case PRV_IDBG_PDL_LOGGER_FILE_PDL_RESULT_TAG_E                 :
                        sscanf(lineStartPtr, PRV_IDBG_PDL_LOGGER_GET_PARAM_READ_FORMAT_MAC(entryPtr->tagId), param, resultStringPtr, &result, time);
                        PRV_IDBG_PDL_LOGGER_PRINT_MAC(" ENDED @ [%s] with <%s> code [%d]\n", time, param, result);
                        break;
                    default:
                        PRV_IDBG_PDL_LOGGER_PRINT_MAC("Unknown tag found: %s\n", tag);
                }

                break;
            }
        }
    }
    fclose(fHandlePtr);

    return PDL_OK;
}
/*$ END OF prvIDbgPdlLoggerShowSingleCmdResults */

/*****************************************************************************
* FUNCTION NAME: iDbgPdlLoggerShowSingleRunResults
*
* DESCRIPTION: 
*
*
*****************************************************************************/
static PDL_STATUS iDbgPdlLoggerShowSingleRunResults (

    /*!     INPUTS:             */
     char*          rootPathPtr,
     char*          currDirNamePtr
    /*!     INPUTS / OUTPUTS:   */

    /*!     OUTPUTS:            */
)
{
/*!****************************************************************************/
/*! L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/*!****************************************************************************/
#ifndef _WIN32
    DIR            *dirHandlePtr;
    struct dirent  *de;  /* Pointer for directory entry  */
#else
    HANDLE           hFile;
    WIN32_FIND_DATA  FindFileData;
    BOOLEAN          moreFiles;
#endif
    char            dirName[PRV_IDBG_PDL_LOGGER_MAX_PATH_CNS] = {0};
    char            fileName[PRV_IDBG_PDL_LOGGER_MAX_PATH_CNS] = {0};
    PDL_STATUS      pdlStatus = PDL_OK;
/*!****************************************************************************/
/*!                      F U N C T I O N   L O G I C                          */
/*!****************************************************************************/
    prvPdlOsSnprintf(dirName, PRV_IDBG_PDL_LOGGER_MAX_PATH_CNS, "%s/%s", rootPathPtr, currDirNamePtr);

#ifndef _WIN32
    dirHandlePtr = opendir(dirName);
    if (dirHandlePtr == NULL)
        return PDL_NOT_FOUND;

    while ((de = readdir(dirHandlePtr)) != NULL)
    {
        if (strcmp(".", de->d_name) == 0 || strcmp("..", de->d_name) == 0)
            continue;
        prvPdlOsSnprintf(fileName, PRV_IDBG_PDL_LOGGER_MAX_PATH_CNS, "%s/%s", dirName, de->d_name);
        pdlStatus = prvIDbgPdlLoggerShowSingleCmdResults(fileName, de->d_name);
    }

    /* validate directory is not empty */
    if (dirHandlePtr)
    {
        closedir(dirHandlePtr);
    }
#else
    strcat(dirName, "/*");
    for (moreFiles = ((hFile = FindFirstFile((LPCSTR)dirName, &FindFileData)) != INVALID_HANDLE_VALUE) ? TRUE : FALSE;
         moreFiles;
         moreFiles = FindNextFile(hFile, &FindFileData))
    {
        if ((FindFileData.dwFileAttributes == INVALID_FILE_ATTRIBUTES) || (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
            continue;

        prvPdlOsSnprintf(fileName, PRV_IDBG_PDL_LOGGER_MAX_PATH_CNS, "%s/%s/%s", rootPathPtr, currDirNamePtr, (char *)FindFileData.cFileName);
        pdlStatus = prvIDbgPdlLoggerShowSingleCmdResults(fileName, (char *)FindFileData.cFileName);
    }

    if (hFile != INVALID_HANDLE_VALUE)
        FindClose(hFile);
#endif

    return pdlStatus;
}
/*$ END OF iDbgPdlLoggerShowSingleRunResults */

/*****************************************************************************
* FUNCTION NAME: prvIDbgPdlLoggerInit
*
* DESCRIPTION: 
*
*
*****************************************************************************/
static PDL_STATUS prvIDbgPdlLoggerInit (

    /*!     INPUTS:             */
#ifndef _WIN32
    DIR*         mainDirPtr,
#endif
     char*      rootPathPtr
    /*!     INPUTS / OUTPUTS:   */

    /*!     OUTPUTS:            */
)
{
/*!****************************************************************************/
/*! L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/*!****************************************************************************/
    PDL_STATUS      pdlStatus = PDL_BAD_PARAM;
/*!****************************************************************************/
/*!                      F U N C T I O N   L O G I C                          */
/*!****************************************************************************/
    pdlStatus = prvIDbgPdlLoggerCreateCmdDir(
#ifndef _WIN32
                                        mainDirPtr,
#endif
                                        rootPathPtr);

    return pdlStatus;
}
/*$ END OF prvIDbgPdlLoggerLoggerInit */

extern PDL_STATUS iDbgPdlLoggerCommandLogStart(
    IN char                         *cliCommandPtr, 
    IN IDBG_PDL_LOGGER_CATEGORY_ENT categoryType, 
    IN const char                   *cliFuncNamePtr
)
{
    PRV_IDBG_PDL_LOGGER_CATEGORY_DB_STC * ptr;
    time_t t;

    PRV_IDBGPDL_FLOW_LOCK_MAC();

    if (prvIDbgPdlLoggerOpenedFilePtr)
    {
        PRV_IDBGPDL_FLOW_UNLOCK_MAC();
        return PDL_BAD_STATE;
    }

    ptr = (PRV_IDBG_PDL_LOGGER_CATEGORY_DB_STC *)
        prvIDbgPdlLoggerGetEntry(prvPdlLogCategoryDbArr, 
                                 sizeof(prvPdlLogCategoryDbArr[0]),
                                 sizeof(prvPdlLogCategoryDbArr)/sizeof(prvPdlLogCategoryDbArr[0]),
                                 IDBG_PDL_LOGGER_CMD_CLEAR_TEST_FLAG_MAC(categoryType));
    if (ptr == NULL)
    {
        PRV_IDBGPDL_FLOW_UNLOCK_MAC();
        return PDL_BAD_PARAM;
    }

    /* determine current file name if hasn't already */
    prvPdlOsSnprintf(prvIDbgPdlLoggerCurrentFileNamePtr, PRV_IDBG_PDL_LOGGER_MAX_PATH_CNS, "%s/%05d_%s_%s.log", prvIDbgPdlLoggerCurrentPathNamePtr, prvIDbgPdlLoggerCurrentRunIndex ++, ptr->categoryNamePtr, cliFuncNamePtr);
    prvIDbgPdlLoggerOpenedFilePtr = fopen(prvIDbgPdlLoggerCurrentFileNamePtr, "w+");
    if (prvIDbgPdlLoggerOpenedFilePtr == NULL)
    {
        PRV_IDBGPDL_FLOW_UNLOCK_MAC();
        return PDL_CREATE_ERROR;
    }

    time(&t);
    fprintf(prvIDbgPdlLoggerOpenedFilePtr,
            PRV_IDBG_PDL_LOGGER_GET_FILE_TAG_FORMAT_MAC(COMMAND_START),
            PRV_IDBG_PDL_LOGGER_GET_FILE_TAG_NAME_MAC(COMMAND_START),
            IDBG_PDL_LOGGER_CMD_IS_TEST_MAC(categoryType) ? "TEST" : "COMMAND",
            cliCommandPtr,
            ctime(&t));
    if (ferror (prvIDbgPdlLoggerOpenedFilePtr))
    {
        PRV_IDBGPDL_FLOW_UNLOCK_MAC();
        return PDL_NOT_READY;
    }

    PRV_IDBGPDL_FLOW_UNLOCK_MAC();
    return PDL_OK;
}

static PDL_STATUS prvIDbgPdlLoggerSomeDescriptionAdd(
    IN BOOLEAN  isDebug, /* no output to result log in case of debug text */
    IN char   * format,
    IN va_list  varList
)
{
    UINT_32 index = 0, totalLen, bufIndex = 0, textIndex;
    char    *descPtr = prvIDbgPdlLoggerResultDescriptionTempBuffer, *tagIdNamePtr;
    char    *textPtr = (isDebug) ? prvIDbgPdlLoggerDebugDescriptionBuffer : prvIDbgPdlLoggerApiDescriptionBuffer;
    BOOLEAN wasNewLine;

    if ((prvIDbgPdlLoggerOpenedFilePtr == NULL) ||
        (prvIDbgPdlLoggerResultDescriptionBuffer == NULL) ||
        (prvIDbgPdlLoggerResultDescriptionTempBuffer == NULL))
    {
        if (isDebug)
            return PDL_OK;
        else
            return PDL_BAD_STATE;
    }

    if (format == NULL)
        return PDL_BAD_PARAM;

    bufIndex = strlen(prvIDbgPdlLoggerResultDescriptionBuffer);

    /* save formatted string in temp buffer */
    vsprintf(descPtr, format, varList);

    totalLen = strlen(descPtr);
    textIndex = strlen(textPtr);

    /* copy to file */
    if (textIndex == 0 || textPtr[textIndex - 1] == '\n')
        wasNewLine = TRUE;
    else
        wasNewLine = FALSE;

    tagIdNamePtr = (isDebug) ? PRV_IDBG_PDL_LOGGER_GET_FILE_TAG_NAME_MAC(PDL_DEBUG_TEXT) : PRV_IDBG_PDL_LOGGER_GET_FILE_TAG_NAME_MAC(API_RESULT_TEXT);
    while (index < totalLen)
    {
        if (wasNewLine)
        {
            /* flush buffer */
            textPtr[textIndex] = '\0';
            fprintf(prvIDbgPdlLoggerOpenedFilePtr,  "%s: %s\n", tagIdNamePtr, textPtr);
            if (ferror (prvIDbgPdlLoggerOpenedFilePtr))
                return PDL_NOT_READY;

            if (!isDebug && (bufIndex + textIndex < PRV_IDBG_PDL_LOGGER_MAX_DESCRIPTION_BUFFER_SIZE_CNS - 1))
            {
                strcpy(&prvIDbgPdlLoggerResultDescriptionBuffer[bufIndex], textPtr);
                bufIndex += textIndex;
            }

            textPtr[0] = '\0';
            textIndex = 0;
        }

        wasNewLine = (descPtr[index] != '\n') ? FALSE : TRUE;

        if (descPtr[index] != '\r')
        {
            textPtr[textIndex ++] = descPtr[index];
        }

        index ++;
    }

    if (wasNewLine)
    {
        /* flush buffer */
        textPtr[textIndex] = '\0';
        fprintf(prvIDbgPdlLoggerOpenedFilePtr,  "%s: %s\n", tagIdNamePtr, textPtr);
        if (ferror (prvIDbgPdlLoggerOpenedFilePtr))
            return PDL_NOT_READY;

        if (!isDebug && (bufIndex + textIndex < PRV_IDBG_PDL_LOGGER_MAX_DESCRIPTION_BUFFER_SIZE_CNS - 1))
        {
            strcpy(&prvIDbgPdlLoggerResultDescriptionBuffer[bufIndex], textPtr);
        }

        textIndex = 0;
    }

    textPtr[textIndex] = '\0';
    return PDL_OK;
}

extern PDL_STATUS iDbgPdlLoggerApiOutDescriptionAdd(
    IN char * format,
    ...
)
{
    va_list     ap;
    PDL_STATUS  pdlStatus;

    PRV_IDBGPDL_FLOW_LOCK_MAC();

    /* save formatted string in temp buffer */
    va_start (ap, format);
    pdlStatus = prvIDbgPdlLoggerSomeDescriptionAdd(FALSE, format, ap);
    va_end(ap);

    PRV_IDBGPDL_FLOW_UNLOCK_MAC();
    return pdlStatus;
}

extern PDL_STATUS iDbgPdlLoggerPdlDebugTextAdd(
    IN char * format,
    ...
)
{
    va_list     ap;
    PDL_STATUS  pdlStatus;

    PRV_IDBGPDL_FLOW_LOCK_MAC();

    /* save formatted string in temp buffer */
    va_start (ap, format);
    pdlStatus = prvIDbgPdlLoggerSomeDescriptionAdd(TRUE, format, ap);
    va_end(ap);

    PRV_IDBGPDL_FLOW_UNLOCK_MAC();
    return pdlStatus;
}

extern PDL_STATUS iDbgPdlLoggerCommandEnd(
    IN IDBG_PDL_LOGGER_RESULT_DATA_STC * resultDataPtr
)
{
    time_t  t;

    if (resultDataPtr == NULL)
        return PDL_BAD_PARAM;

    PRV_IDBGPDL_FLOW_LOCK_MAC();

    if (prvIDbgPdlLoggerOpenedFilePtr == NULL)
    {
        PRV_IDBGPDL_FLOW_UNLOCK_MAC();
        return PDL_BAD_STATE;
    }

    fprintf(prvIDbgPdlLoggerOpenedFilePtr,
            PRV_IDBG_PDL_LOGGER_GET_FILE_TAG_FORMAT_MAC(COMMAND_RESULT),
            PRV_IDBG_PDL_LOGGER_GET_FILE_TAG_NAME_MAC(COMMAND_RESULT),
            PRV_IDBG_PDL_LOGGER_SUCCESS_OR_FAIL_MAC(resultDataPtr->result, resultDataPtr->successIsNegativeResult),
            resultDataPtr->result);
    if (ferror (prvIDbgPdlLoggerOpenedFilePtr))
    {
        PRV_IDBGPDL_FLOW_UNLOCK_MAC();
        return PDL_NOT_READY;
    }

    time(&t);
    fprintf(prvIDbgPdlLoggerOpenedFilePtr,
            PRV_IDBG_PDL_LOGGER_GET_FILE_TAG_FORMAT_MAC(COMMAND_END),
            PRV_IDBG_PDL_LOGGER_GET_FILE_TAG_NAME_MAC(COMMAND_END),
            ctime(&t));
    if (ferror (prvIDbgPdlLoggerOpenedFilePtr))
    {
        PRV_IDBGPDL_FLOW_UNLOCK_MAC();
        return PDL_NOT_READY;
    }

    fclose(prvIDbgPdlLoggerOpenedFilePtr);
    prvIDbgPdlLoggerOpenedFilePtr = NULL;

    PRV_IDBGPDL_FLOW_UNLOCK_MAC();
    return PDL_OK;
}

static PDL_STATUS prvIDbgPdlLoggerSomeApiStart(
    IN PRV_IDBG_PDL_LOGGER_FILE_CONTENT_TAGS_ENT    startTagId,
    IN char                                       * apiNamePtr, 
    IN va_list                                      varList
)
{
    time_t      t;
    UINT_32     type;
    UINT_8      val8;
    UINT_16     val16;
    UINT_32     val32;
    BOOLEAN     valB;
    void       *ptr = NULL;
    char        numberStr[11], *paramNamePtr;
    UINT_32     typeWOFlags;

    if ((startTagId != PRV_IDBG_PDL_LOGGER_FILE_API_START_TAG_E) && (startTagId != PRV_IDBG_PDL_LOGGER_FILE_PDL_START_TAG_E))
    {
        return PDL_BAD_PARAM;
    }

    time(&t);
    fprintf(prvIDbgPdlLoggerOpenedFilePtr,
            prvIDbgPdlLoggerTagDbArr[startTagId].formatStringPtr,
            prvIDbgPdlLoggerTagDbArr[startTagId].tagNamePtr,
            apiNamePtr,
            ctime(&t));
    if (ferror (prvIDbgPdlLoggerOpenedFilePtr))
        return PDL_NOT_READY;

    for (type = va_arg(varList, UINT_32); type != IDBG_PDL_LOGGER_TYPEOF_LAST_CNS; type = va_arg(varList, UINT_32))
    {
        paramNamePtr = va_arg(varList, char *);

        /* skip out params */
        if (IDBG_PDL_LOGGER_PARAM_TYPE_IS_OUT_PARAM_MAC(type))
        {
            ptr = va_arg(varList, void *);
            continue;
        }

        typeWOFlags = IDBG_PDL_LOGGER_PARAM_TYPE_FLAGS_CLEAR_MAC(type);
        switch (typeWOFlags)
        {
            case IDBG_PDL_LOGGER_TYPEOF_UINT_8_CNS   :
                val8 = (UINT_8)va_arg(varList, int);
                prvPdlOsSnprintf(numberStr, 11, "%d", val8);
                break;
            case IDBG_PDL_LOGGER_TYPEOF_UINT_16_CNS  :
                val16 = (UINT_16)va_arg(varList, int);
                prvPdlOsSnprintf(numberStr, 11, "%d", val16);
                break;
            case IDBG_PDL_LOGGER_TYPEOF_UINT_32_CNS  :
                val32 = va_arg(varList, UINT_32);
                prvPdlOsSnprintf(numberStr, 11, "%lu", val32);
                break;
            case IDBG_PDL_LOGGER_TYPEOF_BOOLEAN_CNS  :
                valB = (BOOLEAN)va_arg(varList, int);
                prvPdlOsSnprintf(numberStr, 11, "%s", valB ? "true" : "false");
                break;
            case IDBG_PDL_LOGGER_TYPEOF_CHAR_PTR_CNS :
                ptr = (char *)va_arg(varList, char *);
                break;
            case IDBG_PDL_LOGGER_TYPEOF_PTR_CNS      :
                ptr = (void *)va_arg(varList, void *);
                break;
            default:
                return PDL_NOT_SUPPORTED;
        }

        fprintf(prvIDbgPdlLoggerOpenedFilePtr,
                prvIDbgPdlLoggerTagDbArr[startTagId + 2].formatStringPtr,
                prvIDbgPdlLoggerTagDbArr[startTagId + 2].tagNamePtr,
                paramNamePtr,
                /*IDBG_PDL_LOGGER_TYPEOF_TEXT_MAC(typeWOFlags),*/
                (typeWOFlags == IDBG_PDL_LOGGER_TYPEOF_UINT_8_CNS) ? numberStr :
                (typeWOFlags == IDBG_PDL_LOGGER_TYPEOF_UINT_16_CNS) ? numberStr :
                (typeWOFlags == IDBG_PDL_LOGGER_TYPEOF_UINT_32_CNS) ? numberStr :
                (typeWOFlags == IDBG_PDL_LOGGER_TYPEOF_BOOLEAN_CNS) ? numberStr :
                (typeWOFlags == IDBG_PDL_LOGGER_TYPEOF_CHAR_PTR_CNS) ? (char *)ptr : "PTR");
        if (ferror (prvIDbgPdlLoggerOpenedFilePtr))
            return PDL_NOT_READY;
    }

    return PDL_OK;
}

extern PDL_STATUS iDbgPdlLoggerApiRunStart(
    IN char * apiNamePtr, 
     ... /* arg list  of type/value, ends with typeof(last) */ 
)
{
    PDL_STATUS  pdlStatus;
    va_list     varList;

    PRV_IDBGPDL_FLOW_LOCK_MAC();

    if (!prvIDbgPdlLoggerOpenedFilePtr)
    {
        PRV_IDBGPDL_FLOW_UNLOCK_MAC();
        return PDL_BAD_STATE;
    }

    prvIDbgPdlLoggerResultDescriptionBuffer[0] = '\0';
    prvIDbgPdlLoggerApiDescriptionBuffer[0] = '\0';
    prvIDbgPdlLoggerDebugDescriptionBuffer[0] = '\0';

    va_start(varList, apiNamePtr);
    pdlStatus = prvIDbgPdlLoggerSomeApiStart(PRV_IDBG_PDL_LOGGER_FILE_TAG_2_ID_MAC(API_START), apiNamePtr, varList);
    va_end(varList);

    PRV_IDBGPDL_FLOW_UNLOCK_MAC();
    return pdlStatus;
}

static PDL_STATUS prvIDbgPdlLoggerSomeApiResultLog (
    IN PRV_IDBG_PDL_LOGGER_FILE_CONTENT_TAGS_ENT    endTagId,
    IN IDBG_PDL_LOGGER_RESULT_DATA_STC             *resultDataPtr,
    IN va_list                                      varList
)
{
    time_t      t;
    UINT_32     type;
    UINT_8     *val8Ptr;
    UINT_16    *val16Ptr;
    UINT_32    *val32Ptr;
    BOOLEAN    *valBPtr;
    void       *ptr = NULL;
    char        numberStr[11], *paramNamePtr;
    UINT_32     typeWOFlags;
    char       *pdlStatusStr;

    if (resultDataPtr->result == PDL_OK)
    {
        for (type = va_arg(varList, UINT_32); type != IDBG_PDL_LOGGER_TYPEOF_LAST_CNS; type = va_arg(varList, UINT_32))
        {
            paramNamePtr = va_arg(varList, char *);
            typeWOFlags = IDBG_PDL_LOGGER_PARAM_TYPE_FLAGS_CLEAR_MAC(type);

            /* skip in params */
            if (IDBG_PDL_LOGGER_PARAM_TYPE_IS_IN_PARAM_MAC(type))
            {
                switch (typeWOFlags)
                {
                    case IDBG_PDL_LOGGER_TYPEOF_UINT_8_CNS  :
                        va_arg(varList, int);
                        break;
                    case IDBG_PDL_LOGGER_TYPEOF_UINT_16_CNS :
                        va_arg(varList, int);
                        break;
                    case IDBG_PDL_LOGGER_TYPEOF_UINT_32_CNS :
                        va_arg(varList, UINT_32);
                        break;
                    case IDBG_PDL_LOGGER_TYPEOF_BOOLEAN_CNS :
                        va_arg(varList, int);
                        break;
                    case IDBG_PDL_LOGGER_TYPEOF_CHAR_PTR_CNS:
                        va_arg(varList, char *);
                        break;
                    case IDBG_PDL_LOGGER_TYPEOF_PTR_CNS     :
                        va_arg(varList, void *);
                        break;
                    default:
                        return PDL_NOT_SUPPORTED;

                }
                continue;
            }

            switch (typeWOFlags)
            {
                case IDBG_PDL_LOGGER_TYPEOF_UINT_8_CNS  :
                    val8Ptr = (UINT_8 *)va_arg(varList, UINT_8 *);
                    prvPdlOsSnprintf(numberStr, 11, "%d", *val8Ptr);
                    break;
                case IDBG_PDL_LOGGER_TYPEOF_UINT_16_CNS :
                    val16Ptr = (UINT_16 *)va_arg(varList, UINT_16 *);
                    prvPdlOsSnprintf(numberStr, 11, "%d", *val16Ptr);
                    break;
                case IDBG_PDL_LOGGER_TYPEOF_UINT_32_CNS :
                    val32Ptr = (UINT_32 *)va_arg(varList, UINT_32 *);
                    prvPdlOsSnprintf(numberStr, 11, "%lu", *val32Ptr);
                    break;
                case IDBG_PDL_LOGGER_TYPEOF_BOOLEAN_CNS :
                    valBPtr = (BOOLEAN *)va_arg(varList, BOOLEAN *);
                    prvPdlOsSnprintf(numberStr, 11, "%s", *valBPtr ? "true" : "false");
                    break;
                case IDBG_PDL_LOGGER_TYPEOF_CHAR_PTR_CNS:
                    ptr = (char *)va_arg(varList, char *);
                    break;
                case IDBG_PDL_LOGGER_TYPEOF_PTR_CNS     :
                    ptr = (void *)va_arg(varList, void *);
                    break;
                default:
                    return PDL_NOT_SUPPORTED;

            }

            fprintf(prvIDbgPdlLoggerOpenedFilePtr,
                    prvIDbgPdlLoggerTagDbArr[endTagId - 2].formatStringPtr,
                    prvIDbgPdlLoggerTagDbArr[endTagId - 2].tagNamePtr,
                    paramNamePtr,
                    /*IDBG_PDL_LOGGER_TYPEOF_TEXT_MAC(typeWOFlags),*/
                    (typeWOFlags == IDBG_PDL_LOGGER_TYPEOF_UINT_8_CNS) ? numberStr :
                    (typeWOFlags == IDBG_PDL_LOGGER_TYPEOF_UINT_16_CNS) ? numberStr :
                    (typeWOFlags == IDBG_PDL_LOGGER_TYPEOF_UINT_32_CNS) ? numberStr :
                    (typeWOFlags == IDBG_PDL_LOGGER_TYPEOF_BOOLEAN_CNS) ? numberStr :
                    (typeWOFlags == IDBG_PDL_LOGGER_TYPEOF_CHAR_PTR_CNS) ? (char *)ptr : "PTR");
            if (ferror (prvIDbgPdlLoggerOpenedFilePtr))
                return PDL_NOT_READY;
        }
    }

    if (FALSE == pdlStatusToString(resultDataPtr->result, &pdlStatusStr))
        pdlStatusStr = "UNKNOWN";

    time(&t);
    fprintf(prvIDbgPdlLoggerOpenedFilePtr,
            prvIDbgPdlLoggerTagDbArr[endTagId].formatStringPtr,
            prvIDbgPdlLoggerTagDbArr[endTagId].tagNamePtr,
            PRV_IDBG_PDL_LOGGER_SUCCESS_OR_FAIL_MAC(resultDataPtr->result, resultDataPtr->successIsNegativeResult),
            pdlStatusStr,
            resultDataPtr->result,
            ctime(&t));
    if (ferror (prvIDbgPdlLoggerOpenedFilePtr))
        return PDL_NOT_READY;

    return PDL_OK;
}

extern PDL_STATUS iDbgPdlLoggerApiResultLog (
    IN IDBG_PDL_LOGGER_RESULT_DATA_STC *resultDataPtr,
    ... /* arg list  of type/value, ends with typeof(last) */ 
)
{
    va_list     varList;
    PDL_STATUS  pdlStatus;

    if (resultDataPtr == NULL)
        return PDL_BAD_PARAM;

    PRV_IDBGPDL_FLOW_LOCK_MAC();

    if (!prvIDbgPdlLoggerOpenedFilePtr)
    {
        PRV_IDBGPDL_FLOW_UNLOCK_MAC();
        return PDL_BAD_STATE;
    }

    /* flush buffers */
    if (strlen(prvIDbgPdlLoggerDebugDescriptionBuffer))
        iDbgPdlLoggerPdlDebugTextAdd("\n");
    if (strlen(prvIDbgPdlLoggerApiDescriptionBuffer))
        iDbgPdlLoggerApiOutDescriptionAdd("\n");

    if (resultDataPtr->result == PDL_OK)
    {
        va_start(varList, resultDataPtr);
    }
    else {
        PRV_IDBGPDL_FLOW_UNLOCK_MAC();
        return PDL_BAD_VALUE;
    }

    pdlStatus = prvIDbgPdlLoggerSomeApiResultLog(PRV_IDBG_PDL_LOGGER_FILE_TAG_2_ID_MAC(API_RESULT), resultDataPtr, varList);

    if (resultDataPtr->result == PDL_OK)
    {
        va_end(varList);
    }
    
    if ((resultDataPtr->outDescriptionPtr == NULL) && prvIDbgPdlLoggerResultDescriptionBuffer[0])
    {
        /* copy description to buffer */
        resultDataPtr->outDescriptionPtr = prvIDbgPdlLoggerResultDescriptionBuffer;
    }

    PRV_IDBGPDL_FLOW_UNLOCK_MAC();
    return pdlStatus;
}

extern PDL_STATUS iDbgPdlLoggerPdlRunStart(
    IN char * apiNamePtr, 
     ... /* arg list  of type/value, ends with typeof(last) */ 
)
{
    PDL_STATUS  pdlStatus;
    va_list     varList;

    PRV_IDBGPDL_FLOW_LOCK_MAC();

    if (!prvIDbgPdlLoggerOpenedFilePtr)
    {
        PRV_IDBGPDL_FLOW_UNLOCK_MAC();
        return PDL_OK;
    }

    va_start(varList, apiNamePtr);
    pdlStatus = prvIDbgPdlLoggerSomeApiStart(PRV_IDBG_PDL_LOGGER_FILE_TAG_2_ID_MAC(PDL_START), apiNamePtr, varList);
    va_end(varList);

    PRV_IDBGPDL_FLOW_UNLOCK_MAC();
    return pdlStatus;
}

static PDL_STATUS prvIDbgPdlLoggerSomeParamLog(
    IN PRV_IDBG_PDL_LOGGER_FILE_CONTENT_TAGS_ENT    paramTagId,
    IN char                                       * paramNamePtr,
    IN UINT_32                                      paramType,
    IN void                                       * paramValuePtr
)
{
    UINT_8      *val8Ptr;
    UINT_16     *val16Ptr;
    UINT_32     *val32Ptr;
    BOOLEAN     *valBPtr;
    char        numberStr[11], *ptr = NULL, *formatPtr = IDBG_PDL_LOGGER_PARAM_TYPE_IS_HEX_PARAM_MAC(paramType) ? "0x%08x" : "%lu";
    UINT_32     paramTypeWOFlags = IDBG_PDL_LOGGER_PARAM_TYPE_FLAGS_CLEAR_MAC(paramType);

    if (paramNamePtr == NULL || paramValuePtr == NULL)
        return PDL_BAD_PARAM;

    switch (paramTypeWOFlags)
    {
        case IDBG_PDL_LOGGER_TYPEOF_UINT_8_CNS  :
            val8Ptr = (UINT_8 *)paramValuePtr;
            prvPdlOsSnprintf(numberStr, 11, formatPtr, *val8Ptr);
            break;
        case IDBG_PDL_LOGGER_TYPEOF_UINT_16_CNS :
            val16Ptr = (UINT_16 *)paramValuePtr;
            prvPdlOsSnprintf(numberStr, 11, formatPtr, *val16Ptr);
            break;
        case IDBG_PDL_LOGGER_TYPEOF_UINT_32_CNS :
            val32Ptr = (UINT_32 *)paramValuePtr;
            prvPdlOsSnprintf(numberStr, 11, formatPtr, *val32Ptr);
            break;
        case IDBG_PDL_LOGGER_TYPEOF_BOOLEAN_CNS :
            valBPtr = (BOOLEAN *)paramValuePtr;
            prvPdlOsSnprintf(numberStr, 11, "%s", *valBPtr ? "true" : "false");
            break;
        case IDBG_PDL_LOGGER_TYPEOF_CHAR_PTR_CNS:
            ptr = (char *)paramValuePtr;
            break;
        case IDBG_PDL_LOGGER_TYPEOF_PTR_CNS     :
            break;
        default:
            return PDL_NOT_SUPPORTED;
    }

    fprintf(prvIDbgPdlLoggerOpenedFilePtr,
        IDBG_PDL_LOGGER_PARAM_TYPE_IS_IN_PARAM_MAC(paramType) ? prvIDbgPdlLoggerTagDbArr[paramTagId].formatStringPtr : prvIDbgPdlLoggerTagDbArr[paramTagId + 2].formatStringPtr,
        IDBG_PDL_LOGGER_PARAM_TYPE_IS_IN_PARAM_MAC(paramType) ? prvIDbgPdlLoggerTagDbArr[paramTagId].tagNamePtr : prvIDbgPdlLoggerTagDbArr[paramTagId + 2].tagNamePtr,
        paramNamePtr,
        /*IDBG_PDL_LOGGER_TYPEOF_TEXT_MAC(paramTypeWOFlags),*/
        (paramTypeWOFlags == IDBG_PDL_LOGGER_TYPEOF_UINT_8_CNS) ? numberStr :
        (paramTypeWOFlags == IDBG_PDL_LOGGER_TYPEOF_UINT_16_CNS) ? numberStr :
        (paramTypeWOFlags == IDBG_PDL_LOGGER_TYPEOF_UINT_32_CNS) ? numberStr :
        (paramTypeWOFlags == IDBG_PDL_LOGGER_TYPEOF_BOOLEAN_CNS) ? numberStr :
        (paramTypeWOFlags == IDBG_PDL_LOGGER_TYPEOF_CHAR_PTR_CNS) ? (char *)ptr : "PTR");
    if (ferror (prvIDbgPdlLoggerOpenedFilePtr))
        return PDL_NOT_READY;

    return PDL_OK;
}

extern PDL_STATUS iDbgPdlLoggerApiParamLog(
    IN char  * paramNamePtr,
    IN UINT_32 paramType,
    IN void  * paramValuePtr
    )
{
    PDL_STATUS  pdlStatus;

    PRV_IDBGPDL_FLOW_LOCK_MAC();

    if (!prvIDbgPdlLoggerOpenedFilePtr)
        pdlStatus = PDL_BAD_STATE;
    else
        pdlStatus = prvIDbgPdlLoggerSomeParamLog(PRV_IDBG_PDL_LOGGER_FILE_TAG_2_ID_MAC(API_IN_PARAM), paramNamePtr, paramType, paramValuePtr);

    PRV_IDBGPDL_FLOW_UNLOCK_MAC();
    return pdlStatus;
}

extern PDL_STATUS iDbgPdlLoggerPdlParamLog(
    IN char  * paramNamePtr,
    IN UINT_32 paramType,
    IN void  * paramValuePtr
)
{
    PDL_STATUS pdlStatus;

    PRV_IDBGPDL_FLOW_LOCK_MAC();

    if (!prvIDbgPdlLoggerOpenedFilePtr)
        pdlStatus = PDL_OK;
    else
        pdlStatus = prvIDbgPdlLoggerSomeParamLog(PRV_IDBG_PDL_LOGGER_FILE_TAG_2_ID_MAC(PDL_IN_PARAM), paramNamePtr, paramType, paramValuePtr);

    PRV_IDBGPDL_FLOW_UNLOCK_MAC();
    return pdlStatus;
}

extern PDL_STATUS iDbgPdlLoggerPdlAnyResultLog(
    IN char    * resultValueStringPtr,
    IN UINT_32 * resultValueIntPtr
)
{
    time_t      t;
    char        numberStr[11];

    if (resultValueStringPtr == NULL && resultValueIntPtr == NULL)
        return PDL_BAD_PARAM;

    PRV_IDBGPDL_FLOW_LOCK_MAC();

    if (!prvIDbgPdlLoggerOpenedFilePtr)
    {
        PRV_IDBGPDL_FLOW_UNLOCK_MAC();
        return PDL_BAD_STATE;
    }

    if (resultValueIntPtr)
        prvPdlOsSnprintf(numberStr, 11, "%lu", *resultValueIntPtr);

    time(&t);
    fprintf(prvIDbgPdlLoggerOpenedFilePtr,
            PRV_IDBG_PDL_LOGGER_GET_FILE_TAG_FORMAT_MAC(PDL_RESULT),
            PRV_IDBG_PDL_LOGGER_GET_FILE_TAG_NAME_MAC(PDL_RESULT),
            resultValueStringPtr ? resultValueStringPtr : numberStr,
            resultValueIntPtr ? numberStr : "0",
            ctime(&t));
    if (ferror (prvIDbgPdlLoggerOpenedFilePtr))
    {
        PRV_IDBGPDL_FLOW_UNLOCK_MAC();
        return PDL_NOT_READY;
    }

    PRV_IDBGPDL_FLOW_UNLOCK_MAC();
    return PDL_OK;
}

extern PDL_STATUS iDbgPdlLoggerPdlResultLog(
    IN PDL_STATUS resulltValue,
    ...
)
{
    va_list                         varList;
    PDL_STATUS                      pdlStatus;
    IDBG_PDL_LOGGER_RESULT_DATA_STC resultStc;

    PRV_IDBGPDL_FLOW_LOCK_MAC();

    if (!prvIDbgPdlLoggerOpenedFilePtr)
    {
        PRV_IDBGPDL_FLOW_UNLOCK_MAC();
        return PDL_OK;
    }

    memset(&resultStc, 0, sizeof(resultStc));

    resultStc.result = resulltValue;

    if (resulltValue == PDL_OK)
    {
        va_start(varList, resulltValue);
    }
    else {
        PRV_IDBGPDL_FLOW_UNLOCK_MAC();
        return PDL_BAD_VALUE;
    }

    pdlStatus = prvIDbgPdlLoggerSomeApiResultLog(PRV_IDBG_PDL_LOGGER_FILE_TAG_2_ID_MAC(PDL_RESULT), &resultStc, varList);

    if (resulltValue == PDL_OK)
    {
        va_end(varList);
    }

    PRV_IDBGPDL_FLOW_UNLOCK_MAC();
    return pdlStatus;
}

extern PDL_STATUS iDbgPdlLoggerShowResults(
    IN BOOLEAN showCurrentOnly
)
{
#ifndef _WIN32
    DIR*            dir = NULL;
    struct dirent  *de;
#else
    HANDLE           hFile;
    WIN32_FIND_DATA  FindFileData;
    BOOLEAN          moreFiles;
    char            *pathPtr;
    UINT_32         path_len;
#endif
    char           *namePtr;
    UINT_32         dirIndex;

    PDL_UNUSED_PARAM(showCurrentOnly);

    PRV_IDBGPDL_FLOW_LOCK_MAC();

    if (prvIDbgPdlLoggerIsDirEmpty(prvIDbgPdlLoggerRootPathhNamePtr, ""))
    {
        printf("Nothing to show !\n");
        PRV_IDBGPDL_FLOW_UNLOCK_MAC();
        return PDL_OK;
    }

#ifndef _WIN32
    dir = opendir(prvIDbgPdlLoggerRootPathhNamePtr);

    if (!dir)
    {
        printf("Cant open root directory !\n");
        PRV_IDBGPDL_FLOW_UNLOCK_MAC();
        return PDL_BAD_STATE;
    }

    /* get next directory to scan */
    while ((de = readdir(dir)) != NULL)
    {
        namePtr = de->d_name;
#else
    path_len = strlen(prvIDbgPdlLoggerRootPathhNamePtr) + 3;
    pathPtr = (char *)PRV_IDBG_PDL_LOGGER_MALLOC_MAC(path_len);
    if (pathPtr == NULL)
    {
        PRV_IDBGPDL_FLOW_UNLOCK_MAC();
        return PDL_NO_RESOURCE;
    }

    prvPdlOsSnprintf(pathPtr, path_len, "%s/*", prvIDbgPdlLoggerRootPathhNamePtr);

    for (moreFiles = ((hFile = FindFirstFile((LPCSTR)pathPtr, &FindFileData)) != INVALID_HANDLE_VALUE) ? TRUE : FALSE;
         moreFiles;
         moreFiles = FindNextFile(hFile, &FindFileData))
    {
        if ((FindFileData.dwFileAttributes == INVALID_FILE_ATTRIBUTES) || !(FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
            continue;

        namePtr = (char *)FindFileData.cFileName;
#endif

        if (sscanf(namePtr, "run_%d", &dirIndex))
        {
            iDbgPdlLoggerShowSingleRunResults(prvIDbgPdlLoggerRootPathhNamePtr, namePtr);
        }
    }

    /* close handle */
#ifndef _WIN32
    if (dir)
        closedir(dir);
#else
    if (hFile != INVALID_HANDLE_VALUE)
        FindClose(hFile);
    PRV_IDBG_PDL_LOGGER_FREE_MAC(pathPtr);
#endif

    PRV_IDBGPDL_FLOW_UNLOCK_MAC();
    return PDL_OK;
}

/*****************************************************************************
* FUNCTION NAME: iDbgPdlLoggerInit
*
* DESCRIPTION: 
*      
*
*****************************************************************************/
PDL_STATUS iDbgPdlLoggerInit (
    IN PDL_OS_CALLBACK_API_STC *callbacksPtr,
    IN char                     *rootPathPtr
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
#ifndef _WIN32
    DIR*            dir = NULL;
    int             status;
#else
    char            folder[PRV_IDBG_PDL_LOGGER_MAX_PATH_CNS + 1], *endChPtr;
#endif
    PDL_STATUS      pdlStatus = PDL_BAD_PARAM;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/   
    if (callbacksPtr == NULL || callbacksPtr->mallocPtr == NULL || callbacksPtr->freePtr == NULL)
        return PDL_BAD_PARAM;

    PRV_IDBGPDL_FLOW_LOCK_MAC();

    PRV_IDBG_PDL_LOGGER_MALLOC_MAC = callbacksPtr->mallocPtr;
    PRV_IDBG_PDL_LOGGER_FREE_MAC   = callbacksPtr->freePtr;
    prvIDbgPdlPrintCallbackPtr     = callbacksPtr->printStringPtr;

#ifndef _WIN32
    dir = opendir(rootPathPtr);

    if (!dir && ENOENT == errno)
    {
        /* Directory does not exist. */
        status = mkdir(rootPathPtr, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH );
        if (!status)
        {
            dir = opendir(rootPathPtr);
        }
    }

    if (dir)
    {
        /* Directory exists. */
        pdlStatus = prvIDbgPdlLoggerInit(dir, rootPathPtr);
    }

    if (dir)
        closedir(dir);
#else
    if (FALSE == PathIsDirectory((LPCSTR)rootPathPtr))
    {
        /* Directory doesnt exist. - create all directories one-by-one */
        endChPtr = rootPathPtr;
        while ((endChPtr = strchr(endChPtr, '/')) != NULL)
        {
            memcpy(folder, rootPathPtr, endChPtr - rootPathPtr);
            folder[endChPtr - rootPathPtr] = '\0';
            CreateDirectory((LPCSTR)folder, NULL);
            endChPtr ++;
        }
        CreateDirectory((LPCSTR)rootPathPtr, NULL);
    }

    if (PathIsDirectory((LPCSTR)rootPathPtr))
    {
        /* Directory exists. */
        pdlStatus = prvIDbgPdlLoggerInit(rootPathPtr);
    }
#endif

    prvIDbgPdlLoggerResultDescriptionBuffer = (char *)PRV_IDBG_PDL_LOGGER_MALLOC_MAC(PRV_IDBG_PDL_LOGGER_MAX_DESCRIPTION_BUFFER_SIZE_CNS);
    prvIDbgPdlLoggerResultDescriptionTempBuffer = (char *)PRV_IDBG_PDL_LOGGER_MALLOC_MAC(PRV_IDBG_PDL_LOGGER_MAX_DESCRIPTION_BUFFER_SIZE_CNS);
    prvIDbgPdlLoggerApiDescriptionBuffer = (char *)PRV_IDBG_PDL_LOGGER_MALLOC_MAC(PRV_IDBG_PDL_LOGGER_MAX_DESCRIPTION_BUFFER_SIZE_CNS);
    prvIDbgPdlLoggerDebugDescriptionBuffer = (char *)PRV_IDBG_PDL_LOGGER_MALLOC_MAC(PRV_IDBG_PDL_LOGGER_MAX_DESCRIPTION_BUFFER_SIZE_CNS);
    if ((prvIDbgPdlLoggerResultDescriptionBuffer == NULL) || (prvIDbgPdlLoggerResultDescriptionTempBuffer == NULL) ||
        (prvIDbgPdlLoggerApiDescriptionBuffer == NULL) || (prvIDbgPdlLoggerDebugDescriptionBuffer == NULL))
    {
        if (prvIDbgPdlLoggerResultDescriptionBuffer)
            PRV_IDBG_PDL_LOGGER_FREE_MAC(prvIDbgPdlLoggerResultDescriptionBuffer);
        if (prvIDbgPdlLoggerResultDescriptionTempBuffer)
            PRV_IDBG_PDL_LOGGER_FREE_MAC(prvIDbgPdlLoggerResultDescriptionTempBuffer);
        if (prvIDbgPdlLoggerApiDescriptionBuffer)
            PRV_IDBG_PDL_LOGGER_FREE_MAC(prvIDbgPdlLoggerApiDescriptionBuffer);
        if (prvIDbgPdlLoggerDebugDescriptionBuffer)
            PRV_IDBG_PDL_LOGGER_FREE_MAC(prvIDbgPdlLoggerDebugDescriptionBuffer);

        PRV_IDBGPDL_FLOW_UNLOCK_MAC();
        return PDL_OUT_OF_CPU_MEM;
    }

    PRV_IDBGPDL_FLOW_UNLOCK_MAC();

    return pdlStatus;
}
/*$ END OF iDbgPdlLoggerInit */
