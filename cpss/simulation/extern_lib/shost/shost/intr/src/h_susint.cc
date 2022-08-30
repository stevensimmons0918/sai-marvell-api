/*!**************************************************RND Template version 4.1
 *!     F U N C T I O N   D E S I G N   &   I M P L E M E N T A T I O N
 *!==========================================================================
 *$ TITLE: Suspend aspecified interrupt
 *!--------------------------------------------------------------------------
 *$ FILENAME:
 *!--------------------------------------------------------------------------
 *$ SYSTEM, SUBSYSTEM: nste
 *!--------------------------------------------------------------------------
 *$ AUTHORS: AlexV,oren-v,OrenV,ArikK
 *!--------------------------------------------------------------------------
 *$ LATEST UPDATE: 14-Dec-2008, 11:45 AM CREATION DATE: 19-Jan-98
 *!**************************************************************************
 *!
 *!**************************************************************************
 *!
 *$ FUNCTION: SHOSTG_interrupt_disable_one
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

extern void SHOSTG_interrupt_disable_one (

    /*!     INPUTS:             */

    UINT_32    intr
)
{
/*!*************************************************************************/
/*! L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/*!*************************************************************************/

    SHOSTP_intr_STC *tmp_ptr ;
    /*UINT_32 i,old_mode ;*/
    DWORD thd_id;

/*!*************************************************************************/
/*!                      F U N C T I O N   L O G I C                          */
/*!*************************************************************************/

    //suspend specific interrupt
    SHOST_DB_PROTECT_TAKE;

    thd_id = GetCurrentThreadId() ;

    tmp_ptr = &SHOSTP_intr_table[intr] ;
    if ( tmp_ptr->mask == TRUE ){
            SHOST_DB_PROTECT_GIVE;
            return;
    }

    tmp_ptr->mask = TRUE;
    if( tmp_ptr->susp_flag ){
            SHOST_DB_PROTECT_GIVE;
            return;
    }
    if ( tmp_ptr->thd_id == thd_id ){
            SHOST_DB_PROTECT_GIVE;
            return;
    }
    if ( dummy_SuspendThread( tmp_ptr->hnd ) == 0xffffffff )
               SHOSTC_dll_abort("\nSuspend inter failed.\n");

    SHOST_DB_PROTECT_GIVE;
}
/*$ END OF SHOSTG_interrupt_disable_one */

