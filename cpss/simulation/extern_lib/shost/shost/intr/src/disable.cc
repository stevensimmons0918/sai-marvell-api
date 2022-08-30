/*!*****************************************************RND Template version 4.0
 *!     F U N C T I O N   D E S I G N   &   I M P L E M E N T A T I O N
 *!=============================================================================
 *$ TITLE: DISABLE interrupts.
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
 *$ FUNCTION: SHOSTG_interrupt_disable
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

extern  UINT_32 SHOSTG_interrupt_disable (void)

{
/*!****************************************************************************/
/*! L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/*!****************************************************************************/

   UINT_32  val;
   HANDLE  curthread;


/*!****************************************************************************/
/*!                      F U N C T I O N   L O G I C                          */
/*!****************************************************************************/

   /* take interrupts simulation semaphore   */
   SHOST_DB_PROTECT_TAKE;

   curthread = GetCurrentThread();
   val = (UINT_32)GetThreadPriority( curthread);

   SetThreadPriority( curthread, THREAD_PRIORITY_TIME_CRITICAL);//THREAD_PRIORITY_HIGHEST);
   SHOSTP_do_intr_susp(0) ;

   SHOSTP_intr_susp_all++;
   SHOSTP_intr_susp_thd_id = GetCurrentThreadId();

   /* release interrupts simulation semaphore    */
   SHOST_DB_PROTECT_GIVE;

   return   val;
}
/*$ END OF SHOSTG_interrupt_disable */

