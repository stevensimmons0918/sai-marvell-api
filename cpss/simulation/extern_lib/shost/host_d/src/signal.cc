/*!**************************************************RND Template version 4.1
 *!     F U N C T I O N   D E S I G N   &   I M P L E M E N T A T I O N
 *!==========================================================================
 *$ TITLE: <Title>
 *!--------------------------------------------------------------------------
 *$ FILENAME: u:\virt_plt\host\h_center\src\signal.c
 *!--------------------------------------------------------------------------
 *$ SYSTEM, SUBSYSTEM: LRE
 *!--------------------------------------------------------------------------
 *$ AUTHORS: YURI_A    
 *!--------------------------------------------------------------------------
 *$ LATEST UPDATE: 05-Dec-96  15:42:01         CREATION DATE: 28-Nov-95
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

extern BOOL STARTG_ctrl_handler(DWORD type)
{
    if(type == CTRL_BREAK_EVENT)
        DebugBreak() ;
    else {
      printf("------------- Aborted ----------------\n");
      SetConsoleCtrlHandler((PHANDLER_ROUTINE)STARTG_ctrl_handler,FALSE) ;

      SHOSTC_exit( 128 );
    }
    return TRUE ;
}

