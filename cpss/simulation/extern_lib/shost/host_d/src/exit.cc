/*!**************************************************RND Template version 4.1
 *!     F U N C T I O N   D E S I G N   &   I M P L E M E N T A T I O N
 *!==========================================================================
 *$ TITLE: PC exit function
 *!--------------------------------------------------------------------------
 *$ FILENAME:
 *!--------------------------------------------------------------------------
 *$ SYSTEM, SUBSYSTEM:
 *!--------------------------------------------------------------------------
 *$ AUTHORS: Lior,BoazK,Moish
 *!--------------------------------------------------------------------------
 *$ LATEST UPDATE: 14-Dec-2004, 3:46 PM CREATION DATE: 01-Jan-95
 *!**************************************************************************
 *!
 *!**************************************************************************
 *!
 *$ FUNCTION:   Host_exit
 *!
 *$ GENERAL DESCRIPTION:
 *!
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

extern  void SHOSTC_exit(UINT_32  status)

{
/*!*************************************************************************/
/*! L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/*!*************************************************************************/

/*!*************************************************************************/
/*!                      F U N C T I O N   L O G I C                          */
/*!*************************************************************************/

   SHOSTG_interrupt_disable();

   fcloseall();

   //close the flash mapped file
//   SHOSTC_flash_close() ;

   //close the nvram mapped file
//   SHOSTC_nvram_close() ;

//   SHOSTC_dram_close();
#ifdef PSOS
   t_resume_all_alien();
#endif /*PSOS*/

   //SASIC's exit
//   SASICG_exit();
   if (status<1000){
    ExitProcess( status);
    }
}
/*$ END OF Host_exit */

