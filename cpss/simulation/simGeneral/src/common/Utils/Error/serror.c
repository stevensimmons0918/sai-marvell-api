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
* @file serror.c
*
* @brief The module is Error API function
*
* @version   2
********************************************************************************
*/
#include <stdarg.h>
#include <assert.h>
#include <common/Utils/Error/serror.h>

/* variables */

/* Specific fatal error function */
SUTILS_FATAL_ERROR_FUN sUtilsFatalErrorFun = NULL;
/**
* @internal sUtilsFatalError function
* @endinternal
*
* @brief   Fatal error handler for Simulation's common libs
*
* @param[in] format                   -  for printing.
*/
void sUtilsFatalError
(
    IN char * format, ...
)
{
    char buff[2048];
    va_list args;

    va_start(args, format);
    vsprintf(buff, format, args);
    va_end(args);

    if (sUtilsFatalErrorFun == NULL)
    {
        /* screan print */
        printf(buff);
    }
    else
    {
        /* call specific function */
        sUtilsFatalErrorFun(buff);
    }

    exit(0);/* avoid all warnings/errors from klockwork that program keeps running after fatal error asserted */
}
/**
* @internal sUtilsFatalErrorBind function
* @endinternal
*
* @brief   Bind specific fatal error function.
*
* @param[in] fatalErrorFunPtr         - entry point to the fatal error function.
*/
void sUtilsFatalErrorBind
(
    IN SUTILS_FATAL_ERROR_FUN fatalErrorFunPtr
)
{
    sUtilsFatalErrorFun = fatalErrorFunPtr;
}



