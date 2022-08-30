/*!*****************************************************RND Template version 4.0
 *!     F U N C T I O N   D E S I G N   &   I M P L E M E N T A T I O N
 *!=============================================================================
 *$ TITLE: Check if we are inside an interrupt routine
 *!-----------------------------------------------------------------------------
 *$ FILENAME:
 *!-----------------------------------------------------------------------------
 *$ SYSTEM, SUBSYSTEM: ET16
 *!-----------------------------------------------------------------------------
 *$ AUTHORS: eyran,BoazK,oren-v
 *!-----------------------------------------------------------------------------
 *$ LATEST UPDATE: 14-Oct-2003, 12:27 PM CREATION DATE: 09-Dec-92
 *!*****************************************************************************
 *!
 *!*****************************************************************************
 *!
 *$ FUNCTION: SHOSTG_check_if_into_interrupt
 *!
 *$ GENERAL DESCRIPTION:
 *!
 *$ RETURNS:
 *!
 *!    TRUE if we are inside an interrupt.
 *!    FALSE if not.
 *!
 *$ ALGORITHM:   (local)
 *!
 *$ ASSUMPTIONS:
 *!
 *$ REMARKS:     (local)
 *!
 *!*****************************************************************************
 *!*/

extern unsigned int SHOSTG_check_if_into_interrupt (
    void
)
{
/*!****************************************************************************/
/*! L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/*!****************************************************************************/

    UINT_32  i ;
    DWORD thd_id = GetCurrentThreadId() ;
    HANDLE  curthread;

/*!****************************************************************************/
/*!                      F U N C T I O N   L O G I C                          */
/*!****************************************************************************/

    for(i=0 ; i < SHOSTP_max_intr_CNS; i++)
    {
      if(SHOSTP_intr_table[i].hnd)
      {
        if(thd_id == SHOSTP_intr_table[i].thd_id)
            return TRUE ;
      }
    }

   curthread = GetCurrentThread();
    if ((UINT_32)GetThreadPriority( curthread) == THREAD_PRIORITY_TIME_CRITICAL)
        return TRUE ;

    return FALSE;
}
/*$ END OF SHOSTG_check_if_into_interrupt */

