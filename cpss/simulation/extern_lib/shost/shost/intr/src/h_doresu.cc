/*!**************************************************RND Template version 4.1
 *!     F U N C T I O N   D E S I G N   &   I M P L E M E N T A T I O N
 *!==========================================================================
 *$ TITLE: do inter resume
 *!--------------------------------------------------------------------------
 *$ FILENAME:
 *!--------------------------------------------------------------------------
 *$ SYSTEM, SUBSYSTEM: nste
 *!--------------------------------------------------------------------------
 *$ AUTHORS: eyran, AlexV,Boaz Kahana,oren-v,OrenV
 *!--------------------------------------------------------------------------
 *$ LATEST UPDATE: 11-Jan-2004, 3:34 PM CREATION DATE: 18-Jun-97
 *!**************************************************************************
 *!
 *!**************************************************************************
 *!
 *$ FUNCTION: SHOSTP_do_intr_resume
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

static void SHOSTP_do_intr_resume (

    /*!     INPUTS:             */

    SHOSTP_intr_STC *intr_ptr
)
{
/*!*************************************************************************/
/*! L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/*!*************************************************************************/

    SHOSTP_intr_STC *tmp_ptr ;
    UINT_32 i,old_mode ;
    DWORD thd_id = GetCurrentThreadId() ;

/*!*************************************************************************/
/*!                      F U N C T I O N   L O G I C                          */
/*!*************************************************************************/

    //resume all interrupts with less or equal priority
    for(i=0; i < SHOSTP_max_intr_CNS; i++)
    {
      if(SHOSTP_intr_table[i].hnd)
      {
        tmp_ptr = &SHOSTP_intr_table[i] ;
        if(tmp_ptr->thd_id == thd_id ||  !tmp_ptr->susp_flag ||
           (intr_ptr && tmp_ptr->priority > intr_ptr->priority))
            continue ;
        if ( tmp_ptr->mask == FALSE ){
                if (( tmp_ptr->susp_flag == 0 )||( dummy_ResumeThread(tmp_ptr->hnd) == 0xffffffff ))
                            SHOSTC_dll_abort("\nResume inter failed.\n");
        }
        tmp_ptr->susp_flag-- ;
      }
    }

    t_preemmption(1,&old_mode) ; //enable.. other tasks
}
/*$ END OF SHOSTP_do_intr_resume */

