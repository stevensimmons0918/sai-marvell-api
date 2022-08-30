/*!*****************************************************RND Template version 4.0
 *!     F U N C T I O N   D E S I G N   &   I M P L E M E N T A T I O N
 *!=============================================================================
 *$ TITLE: ENABLE interrupts.
 *!-----------------------------------------------------------------------------
 *$ FILENAME:
 *!-----------------------------------------------------------------------------
 *$ SYSTEM, SUBSYSTEM: ET16
 *!-----------------------------------------------------------------------------
 *$ AUTHORS: eyran,OrenV,LiranP,ArikK
 *!-----------------------------------------------------------------------------
 *$ LATEST UPDATE: 14-Dec-2008, 11:45 AM CREATION DATE: 07-Dec-92
 *!*****************************************************************************
 *!
 *!*****************************************************************************
 *!
 *$ FUNCTION: SHOSTG_interrupt_enable
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
 *!*****************************************************************************
 *!*/

extern  void  SHOSTG_interrupt_enable (

    UINT_32     new_val
)
{
/*!****************************************************************************/
/*! L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/*!****************************************************************************/


/*!****************************************************************************/
/*!                      F U N C T I O N   L O G I C                          */
/*!****************************************************************************/

   /* take interrupts simulation semaphore  */
   SHOST_DB_PROTECT_TAKE;

    SHOSTP_do_intr_resume(0) ;
    SetThreadPriority( GetCurrentThread(), (int)new_val);

    if(SHOSTP_intr_susp_all > 0)  {
        SHOSTP_intr_susp_all--;

        if(SHOSTP_intr_susp_all == 0)
            SHOSTP_intr_susp_thd_id = 0;
    }
    else {
        SHOSTC_dll_abort("\nSHOSTG_interrupt_enable failed.\n");
    }


    /* release interrupts simulation semaphore */
    SHOST_DB_PROTECT_GIVE;
}
/*$ END OF SHOSTG_interrupt_enable */

