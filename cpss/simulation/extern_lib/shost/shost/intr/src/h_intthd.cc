/*!**************************************************RND Template version 4.1
 *!     F U N C T I O N   D E S I G N   &   I M P L E M E N T A T I O N
 *!==========================================================================
 *$ TITLE: the interrupt thread
 *!--------------------------------------------------------------------------
 *$ FILENAME:
 *!--------------------------------------------------------------------------
 *$ SYSTEM, SUBSYSTEM: nste
 *!--------------------------------------------------------------------------
 *$ AUTHORS: eyran, AlexV , Nimrod,oren-v,OrenV,BoazK,LiranP,ArikK
 *!--------------------------------------------------------------------------
 *$ LATEST UPDATE: 14-Dec-2008, 11:54 AM CREATION DATE: 18-Jun-97
 *!**************************************************************************
 *!
 *!**************************************************************************
 *!
 *$ FUNCTION: SHOSTP_intr_thread
 *!
 *$ GENERAL DESCRIPTION:
 *!
 *$ RETURNS:
 *!
 *$ ALGORITHM:   (local)
 *!
 *$ ASSUMPTIONS:
 *!
 *$ REMARKS:     20-Jan-2000 added fun calls to prevent deadlock on CIB mutex
 *!
 *!**************************************************************************
 *!*/

static DWORD __stdcall  SHOSTP_intr_thread(PVOID pPtr)
{
/*!*************************************************************************/
/*! L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/*!*************************************************************************/

  SHOSTP_intr_STC *intr_ptr = (SHOSTP_intr_STC *) pPtr ;
  SHOSTP_intr_FUN *fun ;
  SHOSTP_intr_param_FUN *fun_param;

/*!*************************************************************************/
/*!                      F U N C T I O N   L O G I C                          */
/*!*************************************************************************/

    intr_ptr->thd_created = TRUE;

    for( ;intr_ptr && intr_ptr->ev ; ) {
        WaitForSingleObject(intr_ptr->ev,INFINITE) ;
        fun = intr_ptr->fun;
        fun_param = (SHOSTP_intr_param_FUN *)intr_ptr->fun;
        if(!fun) /* || !SHOSTP_intr_enable) */
            continue ;

        SHOST_DB_PROTECT_TAKE;

        SHOSTP_do_intr_susp(intr_ptr) ;

        SHOST_DB_PROTECT_GIVE;

        if ( intr_ptr->mask == FALSE )
        {
          if(intr_ptr->param_exist == TRUE)
            (*fun_param)(intr_ptr->param);
          else
            (*fun)();
        }
        else
              intr_ptr->was_ev = TRUE;

        SHOST_DB_PROTECT_TAKE;

        SHOSTP_do_intr_resume(intr_ptr) ;

        SHOST_DB_PROTECT_GIVE;

    }

    return 0 ;
}
/*$ END OF SHOSTP_intr_thread */

