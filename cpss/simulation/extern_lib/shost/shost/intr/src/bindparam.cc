/*!**************************************************RND Template version 4.1
 *!     F U N C T I O N   D E S I G N   &   I M P L E M E N T A T I O N
 *!==========================================================================
 *$ TITLE: bind interrupt
 *!--------------------------------------------------------------------------
 *$ FILENAME:
 *!--------------------------------------------------------------------------
 *$ SYSTEM, SUBSYSTEM: nste
 *!--------------------------------------------------------------------------
 *$ AUTHORS: AlexV,oren-v,GadiE,LiranP,ArikK
 *!--------------------------------------------------------------------------
 *$ LATEST UPDATE: 14-Dec-2008, 10:10 AM CREATION DATE: 18-Jun-97
 *!**************************************************************************
 *!
 *!**************************************************************************
 *!
 *$ FUNCTION: SHOSTG_bind_interrupt_with_param
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

extern UINT_32 SHOSTG_bind_interrupt_with_param (

    /*!     INPUTS:             */

    UINT_32 priority,

    UINT_32 intr,

    void *fun,

    HANDLE ev,

    UINT_32 param
)
{
/*!*************************************************************************/
/*! L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/*!*************************************************************************/

    UINT_32  /*i,*/ret_val = 0 ;
    SHOSTP_intr_STC *intr_ptr ;
    /*DWORD  dwThId ;*/
/*!*************************************************************************/
/*!                      F U N C T I O N   L O G I C                          */
/*!*************************************************************************/
    if(intr >= SHOSTP_max_intr_CNS)
        SHOSTC_dll_abort("\nOut of bound interrupt line.\n");

    SHOST_DB_PROTECT_TAKE;
    intr_ptr = &SHOSTP_intr_table[intr] ;
    if(!intr_ptr->hnd) {
        intr_ptr->id = intr ;
        intr_ptr->mask = FALSE ;
        intr_ptr->priority = priority ;
        intr_ptr->susp_flag = 0 ;
        intr_ptr->fun = (SHOSTP_intr_FUN *) fun ;
        intr_ptr->param_exist = TRUE;
        intr_ptr->param = param;

        if(!ev) {
            ev = CreateEvent( NULL, FALSE, FALSE, NULL);
            if(!ev)
                SHOSTC_dll_abort("\nCan't create inter event.\n");
        }
        intr_ptr->ev = ev ;
        intr_ptr->hnd = CreateThread(0, 0, SHOSTP_intr_thread, intr_ptr,
                                                 0, &intr_ptr->thd_id) ;
        if(!intr_ptr->hnd)
            SHOSTC_dll_abort("\nCan't create inter thread.\n");

        while(!intr_ptr->thd_created)
            Sleep(1);

        if(!SetThreadPriority(intr_ptr->hnd, THREAD_PRIORITY_TIME_CRITICAL))
            SHOSTC_dll_abort("\nCan't change inter priority.\n");

        if(SHOSTP_intr_susp_all > 0) {
            /* all interrupts are disabled - suspend created interrupt thread */
            PROTECT_TASK_DOING_READ_WRITE_START_MAC;
            if ( dummy_SuspendThread( intr_ptr->hnd ) == 0xffffffff )
                SHOSTC_dll_abort("\nSuspend inter failed.\n");
            PROTECT_TASK_DOING_READ_WRITE_END_MAC;
            intr_ptr->susp_flag = SHOSTP_intr_susp_all ;
        }

        ret_val = intr ;
    }
    else
      SHOSTC_dll_abort("\nInterrupt index already in use.\n");

    SHOST_DB_PROTECT_GIVE;
    return ret_val ;
}
/*$ END OF SHOSTG_bind_interrupt_with_param */

