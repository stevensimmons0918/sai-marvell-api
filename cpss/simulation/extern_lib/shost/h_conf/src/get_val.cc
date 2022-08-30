/*!**************************************************RND Template version 4.1
 *!     F U N C T I O N   D E S I G N   &   I M P L E M E N T A T I O N
 *!==========================================================================
 *$ TITLE: <Title>
 *!--------------------------------------------------------------------------
 *$ FILENAME:
 *!--------------------------------------------------------------------------
 *$ SYSTEM, SUBSYSTEM: LRE
 *!--------------------------------------------------------------------------
 *$ AUTHORS: YURI_A,AmitK
 *!--------------------------------------------------------------------------
 *$ LATEST UPDATE: 10-Sep-2002, 12:38 PM CREATION DATE: 06-Aug-96
 *!**************************************************************************
 *!
 *!**************************************************************************
 *!
 *$ FUNCTION: <FuncName>
 *!
 *$ GENERAL DESCRIPTION:
 *!
 *$ RETURNS:
 *!
 *$ ALGORITHM:   (local)
 *!
 *$ ASSUMPTIONS:
 *!
 *$ REMARKS: AmitK - 10/09/2002 - check that the match is for the total val_name
 *!                               not only to the same start of string !!!
 *!
 *!**************************************************************************
 *!*/

extern  unsigned  CNFG_get_cnf_value(
    /*!     INPUTS:             */
         char     *chap_name,
         char     *val_name,
         UINT_32  data_len,
    /*!     INPUTS / OUTPUTS:   */
    /*!     OUTPUTS:            */
         char     *val_buf
)
{
/*!*************************************************************************/
/*! L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/*!*************************************************************************/

  FILE      *fp;
  char      *p, *s, buf[ CNFG_max_line_length_CNS];
  unsigned  cmd = FALSE, ret_val = FALSE;

/*!*************************************************************************/
/*!                      F U N C T I O N   L O G I C                          */
/*!*************************************************************************/

  if( ( fp = fopen( CNFG_cofig_file_name, "rt")) != NULL)
   {
     while(  fgets( buf, CNFG_max_line_length_CNS, fp))
      {
       if( ( p = CNFG_chck_str( buf)) == NULL)  continue;
       if( ( s = strchr( buf, '[')) != NULL)
        {
          if( cmd)
                break;
          s = CNFG_chck_str( ++s);
          if( ! strncmp( s, chap_name,  strlen( chap_name)))
                                                      cmd = TRUE;
          else
                                                      cmd = FALSE;
          continue;
        }

       if( cmd)
        {
          if( ! strncmp( p, strlwr( val_name), strlen( val_name))
                &&
              /* check that the match is for the total word not only to the
                 same start of string !!! */
              (isspace(p[strlen( val_name)]) ||
                p[strlen(val_name)] == '=')
            )
           if( ( s = strchr( p, '=')) != NULL)
            if( ( p = CNFG_chck_str( ++s)) != NULL)
             {
              strncpy( val_buf, p, data_len);
              ret_val = TRUE;
              break;
             }
        }
      }
     fclose( fp);
   }

  return  ret_val;
}
/*$ END OF <FuncName> */

