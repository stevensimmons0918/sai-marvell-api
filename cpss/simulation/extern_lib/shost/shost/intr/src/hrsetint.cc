/*!**************************************************RND Template version 4.1
 *!     F U N C T I O N   D E S I G N   &   I M P L E M E N T A T I O N
 *!==========================================================================
 *$ TITLE: set interrupt
 *!--------------------------------------------------------------------------
 *$ FILENAME:
 *!--------------------------------------------------------------------------
 *$ SYSTEM, SUBSYSTEM: nste
 *!--------------------------------------------------------------------------
 *$ AUTHORS: AlexV,oren-v,OrenV,BoazK,ArikK
 *!--------------------------------------------------------------------------
 *$ LATEST UPDATE: 14-Dec-2008, 10:28 AM CREATION DATE: 18-Jun-97
 *!**************************************************************************
 *!
 *!**************************************************************************
 *!
 *$ FUNCTION: SHOSTG_set_interrupt
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

extern UINT_32  SHOSTG_set_interrupt (

    /*!     INPUTS:             */

    UINT_32 intr
)
{
/*!*************************************************************************/
/*! L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/*!*************************************************************************/

/*!*************************************************************************/
/*!                      F U N C T I O N   L O G I C                          */
/*!*************************************************************************/

    SHOST_DB_PROTECT_TAKE;

    if(intr <= SHOSTP_max_intr_CNS &&
       SHOSTP_intr_table[intr].hnd && SHOSTP_intr_table[intr].ev) {
        if (SHOSTP_intr_table[intr].mask == TRUE )
            SHOSTP_intr_table[intr].was_ev = TRUE;
        else
            SetEvent(SHOSTP_intr_table[intr].ev) ;
            SHOST_DB_PROTECT_GIVE;
        return 1 ;
    }

    SHOST_DB_PROTECT_GIVE;

    return 0 ;
}
/*$ END OF SHOSTG_set_interrupt */

