/*!**************************************************RND Template version 4.1
 *!     F U N C T I O N   D E S I G N   &   I M P L E M E N T A T I O N
 *!==========================================================================
 *$ TITLE: <Title>
 *!--------------------------------------------------------------------------
 *$ FILENAME:
 *!--------------------------------------------------------------------------
 *$ SYSTEM, SUBSYSTEM: LRE
 *!--------------------------------------------------------------------------
 *$ AUTHORS: MICHAELH,BoazK
 *!--------------------------------------------------------------------------
 *$ LATEST UPDATE: 9-Sep-2003, 5:36 PM CREATION DATE: 06-Aug-96
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

extern unsigned CNFG_set_cnf_value(
    /*!     INPUTS:             */
    char       * chap_name,
    char       * val_name,
    UINT_32      data_len,
    /*!     INPUTS / OUTPUTS:   */
    /*!     OUTPUTS:            */
    char    * val_buf
)
{
/*!*************************************************************************/
/*! L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/*!*************************************************************************/

    FILE        * fp;
    char        * p, * s, buf[CNFG_max_line_length_CNS];
    unsigned      cmd = FALSE, ret_val = FALSE;
    fpos_t        filepos;
    UINT_32       i/*,j*/;
    char          temp;
/*!*************************************************************************/
/*!                      F U N C T I O N   L O G I C                          */
/*!*************************************************************************/



    if ((fp = fopen(CNFG_cofig_file_name, "r+t")) != NULL) {

        fgetpos(fp, & filepos);

        while ((fgets(buf, CNFG_max_line_length_CNS, fp))) {


            if ((p = CNFG_chck_str(buf)) == NULL)
                continue;
            if ((s = strchr(buf, '[')) != NULL) {
                if (cmd)
                    break;
                s = CNFG_chck_str(++s);
                if (!strncmp(s, chap_name, strlen(chap_name)))
                    cmd = TRUE;
                else
                    cmd = FALSE;
            }

            if( cmd){
                if( ! strncmp( p, strlwr( val_name), strlen( val_name))){

                    if (fsetpos(fp, & filepos) != 0){
                        fclose(fp);
                        return FALSE;
                    }
                    for (i = 0; i < CNFG_max_line_length_CNS; i++) {

                        temp = fgetc(fp);

                        if (temp == '=') {
                            /* fputs(val_buf,fp);*/
                            fgetpos(fp, & filepos);
                            fseek(fp,(long)filepos,SEEK_SET);


                            fputs(val_buf,fp);
                         /*   for (j=0;j<data_len;j++)
                            {
                                fputc(val_buf[j],fp);

                            } */

                            fclose(fp);
                            return TRUE;
                        }
                    }

                }
            }


            fgetpos(fp, & filepos);
        }

        fclose(fp);
    }
    fclose(fp);
    return ret_val;
}

/*$ END OF CNFG_get_cnf_value */

