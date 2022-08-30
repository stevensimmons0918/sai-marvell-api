/*!**************************************************RND Template version 4.1
 *!     F U N C T I O N   D E S I G N   &   I M P L E M E N T A T I O N
 *!==========================================================================
 *$ TITLE: <Title>
 *!--------------------------------------------------------------------------
 *$ FILENAME:
 *!--------------------------------------------------------------------------
 *$ SYSTEM, SUBSYSTEM: LRE
 *!--------------------------------------------------------------------------
 *$ AUTHORS: MICHAELH
 *!--------------------------------------------------------------------------
 *$ LATEST UPDATE: 1-Jan-2002, 10:59 AM CREATION DATE: 06-Aug-96
 *!**************************************************************************
 *!
 *!**************************************************************************
 *!
 *$ FUNCTION: CNFG_get_cnf_value
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

extern  BOOLEAN  CNFG_create_backup_file(
    /*!     INPUTS:             */
         void

)
{
/*!*************************************************************************/
/*! L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/*!*************************************************************************/

  FILE      *in, *out;
  char      /**p, *s, buf[ CNFG_max_line_length_CNS];*/
  unsigned  cmd = FALSE, ret_val = FALSE;
  /*fpos_t filepos;*/
  /*UINT_32 i;*/
  /*char temp;*/
  UINT_32 len;
/*!*************************************************************************/
/*!                      F U N C T I O N   L O G I C                          */
/*!*************************************************************************/

  if( ( in = fopen( CNFG_cofig_file_name, "rt")) == NULL)
  {
    return FALSE;
  }
  len = strlen(CNFG_cofig_file_name);
  strcpy(CNFG_cofig_file_name_back,CNFG_cofig_file_name);
  strcpy((CNFG_cofig_file_name_back+len),"bakup");
  if( ( out = fopen( CNFG_cofig_file_name_back, "w")) == NULL)
  {
    return FALSE;
  }

  while (!feof(in))
      fputc(fgetc(in), out);

  fclose(in);

  fclose(out);

  return FALSE;

}
/*$ END OF CNFG_get_cnf_value */

