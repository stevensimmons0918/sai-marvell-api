/*===========================================================================*/
#ifndef exp_uart_PKS_INCLUDED
#define exp_uart_PKS_INCLUDED
/*!**************************************************RND Template version 4.1
*!                      P A C K A G E       S P E C I F I C A T I O N
*!==========================================================================
*$ TITLE: host io
*!--------------------------------------------------------------------------
*$ FILENAME: c:\temp\radlan\gen\uart\EXP\uart.h
*!--------------------------------------------------------------------------
*$ SYSTEM, SUBSYSTEM: ET16
*!--------------------------------------------------------------------------
*$ AUTHORS: eyran
*!--------------------------------------------------------------------------
*$ LATEST UPDATE:                              CREATION DATE: 16-Jan-94
*!**************************************************************************
*!
*!**************************************************************************
*!
*$ GENERAL DESCRIPTION:
*!
*! PROCESS AND ALGORITHM: (local)
*!
*$ PACKAGE GLOBAL SERVICES:
*!     (A list of package global services).
*!
*$ PACKAGE LOCAL SERVICES:  (local)
*!     (A list of package local services).
*!
*$ PACKAGE USAGE:
*!     (How to use the package services,
*!     routines calling order, restrictions, etc.)
*!
*$ ASSUMPTIONS:
*!
*$ SIDE EFFECTS:
*!
*$ RELATED DOCUMENTS:     (local)
*!
*$ REMARKS:               (local)
*!
*!**************************************************************************
*!*/
/*!**************************************************************************
*$              EXTERNAL DECLARATIONS (IMPORT AND EXPORT)
*!**************************************************************************
*!*/
#if (defined WINDOWS_COMPATIBLE)
    #include <conio.h>
#endif /*(defined WINDOWS_COMPATIBLE)*/
#include <HOST_D/EXP/HOST_D.H>
/*!**************************************************************************
*$              PUBLIC DECLARATIONS (EXPORT)
*!**************************************************************************
*!*/
/*===========================================================================*/
/*!**************************************************RND Template version 4.1
 *!                 D A T A   T Y P E   D E F I N I T I O N
 *!==========================================================================
 *$ TITLE: Exported types
 *!--------------------------------------------------------------------------
 *$ FILENAME: g:\shost\v1\v100\gen\uart\inc\io_exp.dtd
 *!--------------------------------------------------------------------------
 *$ SYSTEM, SUBSYSTEM: Simulation, SHOST
 *!--------------------------------------------------------------------------
 *$ AUTHORS: Michael
 *!--------------------------------------------------------------------------
 *$ LATEST UPDATE: 19-Jul-98  13:48:31         CREATION DATE: 19-Jul-98
 *!**************************************************************************
 *!
 *!**************************************************************************
 *!
 *$ GENERAL DESCRIPTION:
 *!
 *$ REMARKS:
 *!
 *!**************************************************************************
 *!*/

typedef  unsigned int SHOSTG_io_input_FUN  (
    UINT_8      ch
);

typedef  unsigned int SHOSTG_io_output_FUN  (
    void
);

/*===========================================================================*/
/*!**************************************************************************
*$              PUBLIC VARIABLE DEFINITIONS (EXPORT)
*!**************************************************************************
*!*/
/*!**************************************************************************
*$              PUBLIC FUNCTION DEFINITIONS (EXPORT)
*!**************************************************************************
*!*/
/*===========================================================================*/
/**
* @internal SHOSTC_log_file_insert function
* @endinternal
*
*/

extern void SHOSTC_log_file_insert (

    /*!     INPUTS:             */

    char *msg
);
/*$ END OF SHOSTC_log_file_insert */
/*===========================================================================*/
/*===========================================================================*/
/**
* @internal SHOSTG_io_set_input function
* @endinternal
*
*/

extern void SHOSTG_io_set_input (

    /*!     INPUTS:             */

    SHOSTG_io_input_FUN *input_fnc_PTR
);
/*$ END OF SHOSTG_io_set_input */
/*===========================================================================*/
/*===========================================================================*/
/**
* @internal SHOSTG_io_set_output function
* @endinternal
*
*/

extern void SHOSTG_io_set_output (

    /*!     INPUTS:             */

    SHOSTG_io_output_FUN *output_fnc_PTR
);
/*$ END OF SHOSTG_io_set_output */
/*===========================================================================*/
/*===========================================================================*/
/**
* @internal SHOSTG_io_putchar function
* @endinternal
*
*/

extern void SHOSTG_io_putchar (

    /*!     INPUTS:             */

    UINT_8 ch
);
/*$ END OF SHOSTG_io_putchar */
/*===========================================================================*/
/*===========================================================================*/
/**
* @internal SHOSTC_printf function
* @endinternal
*
*/

extern UINT_32 SHOSTC_printf (

    /*!     INPUTS:             */
    const  char   *format,
           ...
);
/*$ END OF SHOSTC_printf */
/*===========================================================================*/
/*===========================================================================*/
/**
* @internal SHOSTC_init_io_pkg function
* @endinternal
*
*/

extern void SHOSTC_init_io_pkg (

         UINT_8 *log_file_name
);
/*$ END OF SHOSTC_init_io_pkg */
/*===========================================================================*/
/*===========================================================================*/
/**
* @internal SHOSTC_init_re_io_pkg function
* @endinternal
*
*/

extern void SHOSTC_init_re_io_pkg (

         UINT_8 *log_file_name
);
/*$ END OF SHOSTC_init_re_io_pkg */
/*===========================================================================*/
/*===========================================================================*/
/**
* @internal SHOSTC_HexStr_to_bin function
* @endinternal
*
*/

extern UINT_32 SHOSTC_HexStr_to_bin (

    /*!     INPUTS:             */
    UINT_8   *dst_ptr,           /*!*/
    char     *src_ptr,           /*!*/
    UINT_32  length           /*!*/
);
/*$ END OF SHOSTC_HexStr_to_bin */
/*===========================================================================*/
/*===========================================================================*/
/**
* @internal SHOSTG_uart_go function
* @endinternal
*
*/

extern void SHOSTG_uart_go (

    void
);
/*$ END OF SHOSTG_uart_go */
/*===========================================================================*/

#endif
/*$ END OF uart */

