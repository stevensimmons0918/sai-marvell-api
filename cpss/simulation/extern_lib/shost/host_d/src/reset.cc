/*!**************************************************RND Template version 4.1
 *!     F U N C T I O N   D E S I G N   &   I M P L E M E N T A T I O N
 *!==========================================================================
 *$ TITLE: Host Reset
 *!--------------------------------------------------------------------------
 *$ FILENAME:
 *!--------------------------------------------------------------------------
 *$ SYSTEM, SUBSYSTEM: Routing Server, RS-Host Interface
 *!--------------------------------------------------------------------------
 *$ AUTHORS: Lior,Moish,NimrodS,Radlan User,LiranP
 *!--------------------------------------------------------------------------
 *$ LATEST UPDATE: 23-Jun-2012, 5:54 PM CREATION DATE: 26-Dec-94
 *!**************************************************************************
 *!
 *!**************************************************************************
 *!
 *$ FUNCTION: SHOSTG_reset
 *!
 *$ GENERAL DESCRIPTION:
 *!
 *$ RETURNS:
 *!
 *$ ALGORITHM:   (local)
 *!
 *$ ASSUMPTIONS:
 *!
 *$ REMARKS:     Increas the delay before CreateProcess and add delay after.
 *$              (Tested by NimrodS)
 *!
 *!**************************************************************************
 *!*/

extern void SHOSTG_reset (

    /*!     INPUTS:             */

    unsigned int status

)
{
/*!*************************************************************************/
/*! L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/*!*************************************************************************/

    STARTUPINFO my_inf ;
    PROCESS_INFORMATION p_inf = {0} ;
    HWND hWin ;
    char name[256] = {'\0'};
    RECT win_rect ;
    DWORD dwMilliseconds;
/*!*************************************************************************/
/*!                      F U N C T I O N   L O G I C                       */
/*!*************************************************************************/
   SHOSTG_interrupt_disable();
   printf("\n");
   printf("**************************************************\n");
   printf("************* Reset Process  *************\n");
   printf("**************************************************\n");

    memset(&my_inf,0,sizeof(my_inf)) ;
    my_inf.cb = sizeof(my_inf) ;
    GetConsoleTitle(name, sizeof(name));     // retrieve the text of the title bar of the console window
/*    CNFG_get_cnf_value( (char *)SHOSTC_d_RS_section,"name", 80, name) ; */
    hWin = FindWindow(0,name) ;
    if(hWin && GetWindowRect(hWin,&win_rect)) {
        my_inf.dwX = win_rect.left ;
        my_inf.dwY =  win_rect.top ;
        my_inf.dwXSize = win_rect.right - win_rect.left  ;
        my_inf.dwYSize = win_rect.bottom - win_rect.top ;
        my_inf.dwFlags = STARTF_USESIZE | STARTF_USEPOSITION ;
    }

    //close the flash mapped file
//    SHOSTC_flash_close() ;
    SHOSTC_exit(0x1000); /* Exit all but not the process.*/
    dwMilliseconds = 2000;
    Sleep(dwMilliseconds); /* Make sure we unbind*/
    if(CreateProcess(0,GetCommandLine(),0,0,0,CREATE_NEW_CONSOLE/*DETACHED_PROCESS*/,0,0,&my_inf,&p_inf)) {
        CloseHandle(p_inf.hProcess) ;
        CloseHandle(p_inf.hThread) ;
    }
    Sleep(dwMilliseconds); /* Make sure we create New process (and leave me in your mother) */
    ExitProcess( status);
}
/*$ END OF SHOSTG_reset */

