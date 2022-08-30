/**
********************************************************************************
* @file simFS.h
*
* @brief Read-only file system API.
* Required for iniFiles and registerFiles to be built-in
* into appDemoSim image
*
* @version   4
********************************************************************************
*/
/***********************************************************************
* simFS.h
*
* DESCRIPTION:
*       Read-only file system API.
*       Required for iniFiles and registerFiles to be built-in
*       into appDemoSim image
*
* DEPENDENCIES:
*
* COMMENTS:
*
* FILE REVISION NUMBER:
*       $Revision: 4 $
************************************************************************/


#ifndef __simFS_h__
#define __simFS_h__

#ifndef LINUX /* win32 only code */
#ifdef _BORLAND /* borland compiler */
#include <dir.h>
#ifndef _POSIX_
#define _POSIX_
#endif
#include <limits.h>
#define CHDIR chdir
#define GET_CURRENT_DIR getcwd
#else           /* VC compiler */
#ifndef __GNUC__
#include <direct.h>
#endif
#include <limits.h>
#define CHDIR _chdir
#define GET_CURRENT_DIR _getcwd
#endif          /* win32 compiler check */

#else           /* linux only code */
#include <unistd.h>
#include <sys/param.h>
#include <limits.h>
#define CHDIR chdir
#define GET_CURRENT_DIR getcwd

#endif          /* win32 only code */

#ifndef PATH_MAX
#define PATH_MAX 512
#endif

/***** Defines  ********************************************************/
/***** Public Types ****************************************************/
/***** Public Data *****************************************************/
extern char  simFSiniFileDirectory[PATH_MAX];/* temp dir name, default empty */

/***** Public Functions ************************************************/

/**
* @internal simFSinit function
* @endinternal
*
* @brief   Initialize simFS, initialize built-in files
*
* @retval 0                        - on success
*/
int simFSinit(void);

/*******************************************************************************
* simFSlastError
*
* DESCRIPTION:
*       Return string with last error description
*
* INPUTS:
*       None
*
* OUTPUTS:
*       None
*
* RETURNS:
*       0   - on success
*
* COMMENTS:
*
*******************************************************************************/
const char* simFSlastError(void);

/**
* @internal simFSopen function
* @endinternal
*
* @brief   Open file for read-only
*/
int simFSopen(const char* name);

/**
* @internal simFSclose function
* @endinternal
*
* @brief   Close a file descriptor
*/
int simFSclose(int fd);

/*******************************************************************************
* simFSgets
*
* DESCRIPTION:
*       Return non-zero if end of file reached
*
* INPUTS:
*       fd      - file descriptor
*       size    - number of bytes to read
*
* OUTPUTS:
*       buf     - buffer to store string
*
* RETURNS:
*       Return pointer to string or NULL if end of file reached
*
* COMMENTS:
*
*******************************************************************************/
char* simFSgets(int fd, char *buf, int size);


/**
* @internal simFSprint function
* @endinternal
*
* @brief   print file name
*/
int simFSprint(const char* name);


/**
* @internal simFSsave function
* @endinternal
*
* @brief   Save embedded ini file and it registers files to temporary directory
*         per user for unique process
* @param[in,out] dirName                  - directory name
*                                      fname   - ini file name
* @param[in,out] dirName                  - directory name
*                                       0 if success, < 0 if error
*/
int simFSsave(INOUT char *dirName, IN const char *fname, IN char files[20][100]);


/**
* @internal simFSprintIniList function
* @endinternal
*
* @brief   print list of embedded ini files to stdout
*/
void simFSprintIniList(void);

#endif /* __simFS_h__ */

