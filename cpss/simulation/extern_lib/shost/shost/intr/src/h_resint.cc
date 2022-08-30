/*!**************************************************RND Template version 4.1
 *!     F U N C T I O N   D E S I G N   &   I M P L E M E N T A T I O N
 *!==========================================================================
 *$ TITLE: The function resume a specified interrupt
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
 *$ FUNCTION: SHOSTG_interrupt_enable_one
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

extern void SHOSTG_interrupt_enable_one (

    /*!     INPUTS:             */

    UINT_32    intr
)
{
/*!*************************************************************************/
/*! L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/*!*************************************************************************/

    SHOSTP_intr_STC *tmp_ptr ;

/*!*************************************************************************/
/*!                      F U N C T I O N   L O G I C                          */
/*!*************************************************************************/

    //resume all interrupts with less or equal priority
    SHOST_DB_PROTECT_TAKE;

    tmp_ptr = &SHOSTP_intr_table[intr] ;
    if ( tmp_ptr->mask == FALSE )
    {
        SHOST_DB_PROTECT_GIVE;
        return;
    }

    tmp_ptr->mask = FALSE;
    if( tmp_ptr->susp_flag )
    {
        SHOST_DB_PROTECT_GIVE;
        return;
    }
    if ( dummy_ResumeThread(tmp_ptr->hnd) == 0xffffffff )
        SHOSTC_dll_abort("\nResume inter failed.\n");

       if(intr && intr <= SHOSTP_max_intr_CNS &&
                                                    tmp_ptr->hnd && tmp_ptr->ev ) {

            if (tmp_ptr->was_ev)
             SetEvent(tmp_ptr->ev) ;

                    tmp_ptr->was_ev = FALSE;
    }
    else{
            SHOSTC_dll_abort("\nResume spec int failed.\n");
    }

    SHOST_DB_PROTECT_GIVE;
}
/*$ END OF SHOSTG_interrupt_enable_one */

