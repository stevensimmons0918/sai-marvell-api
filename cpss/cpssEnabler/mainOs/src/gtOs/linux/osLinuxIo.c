/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE IS A REFERENCE CODE FOR MARVELL SWITCH PRODUCTS.  IT IS PROVIDED   *
* "AS IS" WITH NO WARRANTIES, EXPRESSED, IMPLIED OR OTHERWISE, REGARDING ITS   *
* ACCURACY, COMPLETENESS OR PERFORMANCE.                                       *
* CUSTOMERS ARE FREE TO MODIFY IT AND USE IT ONLY IN THEIR PRODUCTION          *
* SOFTWARE RELEASES WITH MARVELL SWITCH CHIPSETS.                              *
*******************************************************************************/
/**
********************************************************************************
* @file osLinuxIo.c
*
* @brief Linux User Mode Operating System wrapper. Input/output facility.
*
* @version   7
********************************************************************************
*/

#define _GNU_SOURCE
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <errno.h>

#include <gtOs/gtOsIo.h>
#include <extUtils/FS/genFS.h>

#if (defined ASIC_SIMULATION_ENV_FORBIDDEN && defined ASIC_SIMULATION)
    /* 'Forbid' the code to run as ASIC_SIMULATION ... we need 'like' HW compilation ! */
    #undef ASIC_SIMULATION
#endif

/************* Global Arguments ***********************************************/

GT_STATUS (*osPrintSyncUartBindFunc)(char *, GT_U32) = NULL;
GT_BOOL osPrintSyncEnableFlag = GT_FALSE;

#include <gtOs/globalDb/gtOsGlobalDb.h>
#include <gtOs/globalDb/gtOsGlobalDbInterface.h>

/*global variables macros*/

#define PRV_SHARED_MAIN_OS_DIR_OS_LINUX_IO_SRC_GLOBAL_VAR_SET(_var,_value)\
    PRV_HELPER_SHARED_GLOBAL_VAR_SET(cpssEnablerMod.mainOsDir.osLinuxIoSrc._var,_value)

#define PRV_SHARED_MAIN_OS_DIR_OS_LINUX_IO_SRC_GLOBAL_VAR_GET(_var)\
    PRV_HELPER_SHARED_GLOBAL_VAR_GET(cpssEnablerMod.mainOsDir.osLinuxIoSrc._var)


#define PRV_NON_SHARED_MAIN_OS_DIR_OS_LINUX_IO_SRC_GLOBAL_VAR_SET(_var,_value)\
    PRV_HELPER_NON_SHARED_GLOBAL_VAR_SET(cpssEnablerMod.mainOsDir.osLinuxIoSrc._var,_value)

#define PRV_NON_SHARED_MAIN_OS_DIR_OS_LINUX_IO_SRC_GLOBAL_VAR_GET(_var)\
    PRV_HELPER_NON_SHARED_GLOBAL_VAR_GET(cpssEnablerMod.mainOsDir.osLinuxIoSrc._var)

#define PRV_SHARED_DB osSharedGlobalVarsPtr->cpssEnablerMod.mainOsDir.osLinuxIoSrc

/************ Public Functions ************************************************/

/**
* @internal osNullStdOutFunction function
* @endinternal
*
* @brief   Get Stdout handler
*/
int osNullStdOutFunction
(
    GT_VOID_PTR         userPtr,
    const char*         buffer,
    int                 length
)
{
    (void)userPtr;
    (void)buffer;
    (void)length;
    return 0;
}

/**
* @internal osGetStdOutFunction function
* @endinternal
*
* @brief   Get Stdout handler
*
* @param[out] writeFunctionPtrPtr      - poiter to saved output function
* @param[out] writeFunctionParamPtrPtr - poiter to saved output function parameter
*                                       GT_OK
*/
GT_STATUS osGetStdOutFunction
(
    OUT OS_BIND_STDOUT_FUNC_PTR     *writeFunctionPtrPtr,
    OUT GT_VOID_PTR                 *writeFunctionParamPtrPtr
)
{
    *writeFunctionPtrPtr        = PRV_NON_SHARED_MAIN_OS_DIR_OS_LINUX_IO_SRC_GLOBAL_VAR_GET(writeFunctionPtr);
    *writeFunctionParamPtrPtr   = PRV_SHARED_DB.writeFunctionParam;

    return GT_OK;
}

/**
* @internal osBindStdOut function
* @endinternal
*
* @brief   Bind Stdout to handler
*
* @param[in] writeFunction            - function to call for output
* @param[in] userPtr                  - first parameter to pass to write function
*                                       GT_OK
*/
GT_STATUS osBindStdOut
(
    IN OS_BIND_STDOUT_FUNC_PTR writeFunction,
    IN void* userPtr
)
{
    PRV_NON_SHARED_MAIN_OS_DIR_OS_LINUX_IO_SRC_GLOBAL_VAR_SET(writeFunctionPtr,writeFunction);
    PRV_SHARED_DB.writeFunctionParam = userPtr;
    return GT_OK;
}

/**
* @internal osPrintf function
* @endinternal
*
* @brief   Write a formatted string to the standard output stream.
*/
int osPrintf(const char* format, ...)
{
    va_list args;
    int     ret;

    va_start(args, format);
    ret = osVprintf(format, args);
    va_end(args);

    return ret;
}

/**
* @internal osPrintSyncEnable function
* @endinternal
*
* @brief   Enable/disable synchronous serial output
*
* @param[in] enable                   - flag to set
*                                       GT_OK
*/
GT_STATUS osPrintSyncEnable(IN GT_BOOL enable)
{
    osPrintSyncEnableFlag = enable;
    return GT_OK;
}

/**
* @internal osVprintf function
* @endinternal
*
* @brief   Write a formatted string to the standard output stream.
*/
int osVprintf(const char* format, va_list args)
{
    int ret;

    if (osNonSharedGlobalVarsPtr&&PRV_NON_SHARED_MAIN_OS_DIR_OS_LINUX_IO_SRC_GLOBAL_VAR_GET(writeFunctionPtr) != NULL)
    {
        char buff[2048];

        ret = vsnprintf(buff, sizeof(buff), format, args);
        if (ret <= 0)
            return ret;

        return (PRV_NON_SHARED_MAIN_OS_DIR_OS_LINUX_IO_SRC_GLOBAL_VAR_GET(writeFunctionPtr))(PRV_SHARED_DB.writeFunctionParam, buff, ret);
    }

    ret = vprintf(format, args);
    if (ret <= 0)
    {
        return ret;
    }

    /* VVV TBD - should be checked if needed */
    fflush(stdout);
    /* fsync(fileno(stdout)); */
    if (osPrintSyncEnableFlag == GT_TRUE)
    {
        tcdrain(fileno(stdout));
    }

    return ret;
}


/**
* @internal osSprintf function
* @endinternal
*
* @brief   Write a formatted string to a buffer.
*/
int osSprintf(char * buffer, const char* format, ...)
{
    va_list args;
    int i;

    va_start(args, format);
    i = vsprintf(buffer, format, args);
    va_end(args);

    return i;
}

/**
* @internal osVsprintf function
* @endinternal
*
* @brief   Write a formatted string to a buffer.
*/
int osVsprintf(char * buffer, const char* format, va_list args)
{
    return vsprintf(buffer, format, args);
}

/**
* @internal osSnprintf function
* @endinternal
*
* @brief   Write a formatted string to a buffer.
*/
int osSnprintf(char * buffer, int size, const char* format, ...)
{
    va_list args;
    int i;

    va_start(args, format);
    i = vsnprintf(buffer, (size_t)size, format, args);
    va_end(args);

    return i;
}

/**
* @internal osVsnprintf function
* @endinternal
*
* @brief   Write a formatted string to a buffer.
*/
int osVsnprintf(char * buffer, int size, const char* format, va_list args)
{
    return vsnprintf(buffer, (size_t)size, format, args);
}

/*******************************************************************************
* osGets
*
* DESCRIPTION:
*       Reads characters from the standard input stream into the array
*       'buffer' until end-of-file is encountered or a new-line is read.
*       Any new-line character is discarded, and a null character is written
*       immediately after the last character read into the array.
*
* INPUTS:
*       buffer  - pointer to buffer to write to
*
* OUTPUTS:
*       buffer  - buffer with read data
*
* RETURNS:
*       A pointer to 'buffer', or a null pointer if end-of-file is
*       encountered and no characters have been read, or there is a read error.
*
* COMMENTS:
*       None
*
*******************************************************************************/
char * osGets(char * buffer)
{
     /* In Linux, linker returns:
      * "the `gets' function is dangerous and should not be used.
      * So we use fgets.
      */
     GT_CHAR  *tmpRes;          /* Pointer to read buffer or NULL if read error occurs */

     tmpRes = fgets(buffer, 1024, stdin);

     return tmpRes;
}

/**
* @internal osPrintSync function
* @endinternal
*
* @brief   Write a formatted string to the standard output stream, in polling mode
*         The device driver will print the string in the same instance this function
*         is called, with no delay.
*/
int osPrintSync(const char* format, ...)
{
    char buff[2048];
    va_list args;
    int retVal = 0;

    if ( osPrintSyncUartBindFunc != NULL )
    {
        va_start(args, format);
        vsprintf(buff, format, args);
        va_end(args);

        retVal = (int)((osPrintSyncUartBindFunc)(buff, (GT_U32)strlen(buff)));
    }
#ifdef  ASIC_SIMULATION /* no UART support */
    else /* similar to osPrintf(...) */
    {
        va_start(args, format);
        retVal = osVprintf(format, args);
        va_end(args);
    }
#endif /*ASIC_SIMULATION*/

    return (retVal);
}

/*******************************************************************************
* osFlastErrorStr
*
* DESCRIPTION:
*       returns error string for last error
*
* INPUTS:
*       None
*
* OUTPUTS:
*       None
*
* RETURNS:
*       string
*
* COMMENTS:
*       None
*
*******************************************************************************/
const char * osFlastErrorStr(void)
{
    return strerror(errno);
}

/**
* @internal osFopen function
* @endinternal
*
* @brief   Opens the file whose name is specified in the parameter filename and associates it with a stream
*          that can be identified in future operations by the OS_FILE pointer returned.
* @param[in] fileNamePtr              - file name string
* @param[in] modePtr                  - pointer to file mode string, such as "w" or "wr" for standard file
* @param[inout] fp                    - pointer to struct with file descriptor and type (REGULAR / RAMFS)
*                                       fp is used for subsequent calls to close, read, write, etc.
*/
CPSS_OS_FILE osFopen(const char *fileNamePtr, const char *modePtr, CPSS_OS_FILE_TYPE_STC *fp)
{
    if (fp->type != CPSS_OS_FILE_RAMFS)
    {
        fp->fd = (CPSS_OS_FILE) fopen(fileNamePtr, modePtr);
        if (!(fp->fd))
            fp->fd = CPSS_OS_FILE_INVALID;
        return fp->fd;
    }
    else
    {
        genFS_STC *cmdFS = (genFS_STC *) fp->param;
        return cmdFS->open(fileNamePtr,(GT_UINTPTR)modePtr);
    }
}

#define CPSS_FILE_TO_FD(_stream) \
    FILE *fd = (FILE *)(_stream); \
    if ((_stream) == CPSS_OS_FILE_STDOUT) \
        fd = stdout; \
    if ((_stream) == CPSS_OS_FILE_STDERR) \
        fd = stderr;

/**
* @internal osFclose function
* @endinternal
*
* @brief   Closes the file associated with the stream and disassociates it.
*
* @note Nome
*
*/
int osFclose(CPSS_OS_FILE_TYPE_STC *stream)
{
    if (stream->type != CPSS_OS_FILE_RAMFS)
    {
        CPSS_FILE_TO_FD(stream->fd);
        return fclose(fd);
    }
    else
    {
        genFS_STC *cmdFS = (genFS_STC *) stream->param;
        return cmdFS->close(stream->fd);
    }
}

/**
* @internal osRewind function
* @endinternal
*
* @brief   Sets the position indicator associated with stream to the beginning of the file.
*
* @note Nome
*
*/
void osRewind(CPSS_OS_FILE stream)
{
    CPSS_FILE_TO_FD(stream);
    rewind(fd);
}

/**
* @internal osFprintf function
* @endinternal
*
* @brief   Write a formatted string to the stream.
*/
int osFprintf(CPSS_OS_FILE stream, const char* format, ...)
{
    va_list args;
    char buffer[2048];
    CPSS_FILE_TO_FD(stream);

    va_start(args, format);
    vsprintf(buffer, format, args);
    va_end(args);

    return fprintf(fd, "%s", buffer);
}

/*******************************************************************************
* osFgets
*
* DESCRIPTION:
*       Reads characters from stream and stores them as a string into buffer until (num-1) characters have been read
*       or either a newline or the end-of-file is reached, whichever happens first.
*
* INPUTS:
*       bufferPtr  - (pointer to) buffer to write to
*       num - maximum number of characters to be copied into str (including the terminating null-character)
*       stream  - OS_FILE object that specifies the stream.
*
* OUTPUTS:
*       bufferPtr  - (pointer to)buffer with read data
*
* RETURNS:
*       A pointer to 'bufferPtr', or a null pointer if end-of-file is
*       encountered and no characters have been read, or there is a read error.
*
* COMMENTS:
*       None
*
*******************************************************************************/
char * osFgets(char * bufferPtr, int num, CPSS_OS_FILE stream)
{
    CPSS_FILE_TO_FD(stream);
    return fgets(bufferPtr, num, fd);
}

/**
* @internal osFwrite function
* @endinternal
*
* @brief   Writes nmemb elements of data, each size bytes long, to the stream,
*         obtaining them from the location given by ptr.
* @param[in] ptr                      - (pointer from) buffer to write from
* @param[in] size                     - element size
* @param[in] nmemb                    - number of elements to write
* @param[in] stream                   - OS_FILE object that specifies the stream.
*                                       Number of items written or less than 0
*/
int osFwrite(
    IN const void *ptr,
    IN int         size,
    IN int         nmemb,
    IN CPSS_OS_FILE_TYPE_STC   *stream
)
{
    if (stream->type != CPSS_OS_FILE_RAMFS)
    {
        CPSS_FILE_TO_FD(stream->fd);
        return (int)fwrite(ptr,size,nmemb,fd);
    }
    else
    {
        genFS_STC *cmdFS = (genFS_STC *) stream->param;
        return cmdFS->write(stream->fd, ptr, nmemb);
    }
}

/**
* @internal osFread function
* @endinternal
*
* @brief   Reads nmemb elements of data, each size bytes long, from the stream,
*         to the location given by ptr.
* @param[in] size                     - element size
* @param[in] nmemb                    - number of elements to write
* @param[in] stream                   - OS_FILE object that specifies the stream.
*
* @param[out] ptr                      - (pointer to) buffer to read to
*                                       Number of items read or less than 0
*/
int osFread(
    OUT void        *ptr,
    IN  int          size,
    IN  int          nmemb,
    IN  CPSS_OS_FILE_TYPE_STC    *stream
)
{
    if (stream->type != CPSS_OS_FILE_RAMFS)
    {
        CPSS_FILE_TO_FD(stream->fd);
        return (int)fread(ptr,size,nmemb,fd);
    }
    else
    {
        genFS_STC *cmdFS = (genFS_STC *) stream->param;
        return cmdFS->read(stream->fd, ptr, nmemb);
    }
}
/**
* @internal osFgetLength function
* @endinternal
*
* @brief   Returns lenhth of current file
*
* @param[in] stream                   - OS_FILE object that specifies the stream.
*                                       file length or 0
*/
GT_UINTPTR osFgetLength(
    IN  CPSS_OS_FILE stream
)
{
    long curr, length;
    CPSS_FILE_TO_FD(stream);
    curr = ftell(fd);
    fseek(fd, 0, SEEK_END);
    length = ftell(fd);
    fseek(fd, curr, SEEK_SET);
    return (GT_UINTPTR)length;
}



