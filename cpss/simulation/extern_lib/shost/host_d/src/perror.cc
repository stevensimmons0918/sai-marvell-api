/*!**************************************************RND Template version 4.1
 *!     F U N C T I O N   D E S I G N   &   I M P L E M E N T A T I O N
 *!==========================================================================
 *$ TITLE: <Title>
 *!--------------------------------------------------------------------------
 *$ FILENAME: u:\next\simulat\shost\host_d\src\perror.c
 *!--------------------------------------------------------------------------
 *$ SYSTEM, SUBSYSTEM: ????
 *!--------------------------------------------------------------------------
 *$ AUTHORS: AlexV     
 *!--------------------------------------------------------------------------
 *$ LATEST UPDATE: 12-Jul-98  19:04:49         CREATION DATE: 12-Jul-98
 *!**************************************************************************
 *!
 *!**************************************************************************
 *!
 *$ FUNCTION: <FuncName>
 *!
 *$ GENERAL DESCRIPTION:
 *!
 *$ RETURNS:
 *!
 *$ ALGORITHM:   (local)
 *!
 *$ ASSUMPTIONS:
 *!
 *$ REMARKS:     (local)
 *!
 *!**************************************************************************
 *!*/

extern void Perror(

    /*!     INPUTS:             */

    LPTSTR msg

    /*!     INPUTS / OUTPUTS:   */
    /*!     OUTPUTS:            */
)
{
/*!*************************************************************************/
/*! L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/*!*************************************************************************/

    PVOID lpMsgBuf;
    DWORD err = GetLastError();

/*!*************************************************************************/
/*!                      F U N C T I O N   L O G I C                          */
/*!*************************************************************************/

    if(!FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | 
                      FORMAT_MESSAGE_FROM_SYSTEM, NULL, err, 
                      MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                      (LPTSTR) &lpMsgBuf, 0, NULL))
    {
        printf("Perror: FormatMessage: Error %d\n", GetLastError());
        return;
    }
    printf("%s : Error %d: %s", msg, err, (LPTSTR)lpMsgBuf);
    LocalFree(lpMsgBuf);

}
/*$ END OF <FuncName> */
