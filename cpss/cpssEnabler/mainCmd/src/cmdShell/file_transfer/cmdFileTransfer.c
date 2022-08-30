/**
********************************************************************************
* @file cmdFileTransfer.c
*
* @brief File transfer to/from cmdFS
*
* @version   20
********************************************************************************
*/
/*******************************************************************************
* cmdFileTransfer.c
*
* DESCRIPTION:
*       File transfer to/from cmdFS
*
* DEPENDENCIES:
*
* FILE REVISION NUMBER:
*       $Revision: 20 $
*******************************************************************************/

/***** Include files ***************************************************/

#include <cmdShell/common/cmdCommon.h>
#include <cmdShell/os/cmdStreamImpl.h>
#include <cmdShell/FS/cmdFS.h>

/***** Defines  ********************************************************/
#define     FT_CMD_BUFFER       300
#define     FT_INPUT_BUFFER     1024

#define     FT_MSG_M_HELLO              "200"
#define     FT_MSG_M_UPLOAD             "201"
#define     FT_MSG_M_GOTFILE            "202"
#define     FT_MSG_M_FILELIST           "210"
#define     FT_MSG_M_DELETED            "203"
#define     FT_MSG_M_FILE_FOLLOW        "204"
#define     FT_MSG_M_FILE_DONE          "205"
#define     FT_MSG_E_FCREATE            "501"
#define     FT_MSG_E_INPUT              "502"
#define     FT_MSG_E_DIRERR             "503"
#define     FT_MSG_E_DELETE             "504"
#define     FT_MSG_E_NOTEXISTS          "505"
#define     FT_MSG_E_FOPEN              "506"
#define     FT_MSG_E_NOTFILE            "507"


#define IS_CMD_LS(s) ( cmdOsMemCmp((s), "ls", 2) == 0 && ((s)[2] == 0 || (s)[2] == ' '))
#define CMD_LS_DATA(s) ( (s)[2] == ' ' ? (s) + 3 : NULL )
#define IS_CMD_PUSHFILE(s) ( cmdOsMemCmp((s), "push ", 5) == 0 )
#define CMD_PUSHFILE_DATA(s) ( (s) + 5 )
#define IS_CMD_ZPUSHFILE(s) ( cmdOsMemCmp((s), "zpush ", 6) == 0 )
#define CMD_ZPUSHFILE_DATA(s) ( (s) + 6 )
#define IS_CMD_DELETE(s) ( cmdOsMemCmp((s), "del ", 4) == 0 )
#define CMD_DELETE_DATA(s) ( (s) + 4 )
#define IS_CMD_GETFILE(s) ( cmdOsMemCmp((s), "get ", 4) == 0 )
#define CMD_GETFILE_DATA(s) ( (s) + 4 )

/***** Global macros ***************************************************/

/***** Private Types ***************************************************/

/***** Private Data ****************************************************/

/***** Private Functions ***********************************************/

