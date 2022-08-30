/*******************************************************************************
*                Copyright 2001, Marvell International Ltd.
* This code contains confidential information of Marvell semiconductor, inc.
* no rights are granted herein under any patent, mask work right or copyright
* of Marvell or any third party.
* Marvell reserves the right at its sole discretion to request that this code
* be immediately returned to Marvell. This code is provided "as is".
* Marvell makes no warranties, express, implied or otherwise, regarding its
* accuracy, completeness or performance.
********************************************************************************
*/
/**
********************************************************************************
* @file mvHwsRsfecStatisticsUnits.c
*
* @brief Hawk MTI Multiplexer register DB
*
* @version   1
********************************************************************************
*/

#include <gtOs/gtGenTypes.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/siliconAddress.h>



MV_HWS_REG_ADDR_FIELD_STC hawkRsfecStatisticsUnitsDb[] = {
    /* FAST_DATA_HI */
  /*0*/  {/*baseAddr*/ 0x0000,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 32 }, /*LATCHED_COUNTER_HI_FAST */

    /* STATN_STATUS */
  /*1*/  {/*baseAddr*/ 0x0004,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*STAT_MODUlE_BUSY */

    /* STATN_CONFIG */
  /*2*/  {/*baseAddr*/ 0x0008,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*SATURATE */
  /*3*/  {/*baseAddr*/ 0x0008,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*CLEAR_ON_READ */
  /*4*/  {/*baseAddr*/ 0x0008,  /*offsetFormula*/ 0x0, /*fieldStart*/  31, /*fieldLen*/ 1 }, /*SOFT_RESET */

    /* STATN_CONTROL */
  /*5*/  {/*baseAddr*/ 0x000C,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 32 }, /*STATN_CONTROL */
  /*6*/  {/*baseAddr*/ 0x000C,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 8 }, /*CHANMASK */
  /*7*/  {/*baseAddr*/ 0x000C,  /*offsetFormula*/ 0x0, /*fieldStart*/  28, /*fieldLen*/ 1 }, /*CMD_CAPTURE */
  /*8*/  {/*baseAddr*/ 0x000C,  /*offsetFormula*/ 0x0, /*fieldStart*/  29, /*fieldLen*/ 1 }, /*CLEAR_PRE */
  /*9*/  {/*baseAddr*/ 0x000C,  /*offsetFormula*/ 0x0, /*fieldStart*/  31, /*fieldLen*/ 1 }, /*CMD_CLEAR */

    /* STATN_CLEARVALUE_LO */
  /*10*/  {/*baseAddr*/ 0x0010,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 32 }, /*STATM_LOWER_32B */

    /* STATN_CLEARVALUE_HI */
  /*11*/  {/*baseAddr*/ 0x0014,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 32 }, /*STATM_HIGH_32B */

    /* SLOW_DATA_HI */
  /*12*/  {/*baseAddr*/ 0x001C,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 32 }, /*LATCHED_COUNTER_HI_SLOW */

    /* COUNTER_CAPTURE_TOTAL_CW_RECEIVED */
  /*13*/  {/*baseAddr*/ 0x0020,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 32 }, /*CAPTURE_TOTAL_CW_RECEIVED */

    /* COUNTER_CAPTURE_CORRECT */
  /*14*/  {/*baseAddr*/ 0x0024,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 32 }, /*CAPTURE_CORRECT */

    /* COUNTER_CAPTURE_CORRECTED */
  /*15*/  {/*baseAddr*/ 0x0028,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 32 }, /*CAPTURE_CORRECTED */

    /* COUNTER_CAPTURE_UNCORRECTED */
  /*16*/  {/*baseAddr*/ 0x002C,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 32 }, /*CAPTURE_UNCORRECTED */

    /* COUNTER_CAPTURE_CW_CORRECTED_1SE */
  /*17*/  {/*baseAddr*/ 0x0030,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 32 }, /*CAPTURE_CW_CORRECTED_1SE */

    /* COUNTER_CAPTURE_CW_CORRECTED_2SE */
  /*18*/  {/*baseAddr*/ 0x0034,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 32 }, /*CAPTURE_CW_CORRECTED_2SE */

    /* COUNTER_CAPTURE_CW_CORRECTED_3SE */
  /*19*/  {/*baseAddr*/ 0x0038,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 32 }, /*CAPTURE_CW_CORRECTED_3SE */

    /* COUNTER_CAPTURE_CW_CORRECTED_4SE */
  /*20*/  {/*baseAddr*/ 0x003C,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 32 }, /*CAPTURE_CW_CORRECTED_4SE */

    /* COUNTER_CAPTURE_CW_CORRECTED_5SE */
  /*21*/  {/*baseAddr*/ 0x0040,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 32 }, /*CAPTURE_CW_CORRECTED_5SE */

    /* COUNTER_CAPTURE_CW_CORRECTED_6SE */
  /*22*/  {/*baseAddr*/ 0x0044,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 32 }, /*CAPTURE_CW_CORRECTED_6SE */

    /* COUNTER_CAPTURE_CW_CORRECTED_7SE */
  /*23*/  {/*baseAddr*/ 0x0048,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 32 }, /*CAPTURE_CW_CORRECTED_7SE */

    /* COUNTER_CAPTURE_CW_CORRECTED_8SE */
  /*24*/  {/*baseAddr*/ 0x004C,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 32 }, /*CAPTURE_CW_CORRECTED_8SE */

    /* COUNTER_CAPTURE_CW_CORRECTED_9SE */
  /*25*/  {/*baseAddr*/ 0x0050,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 32 }, /*CAPTURE_CW_CORRECTED_9SE */

    /* COUNTER_CAPTURE_CW_CORRECTED_10SE */
  /*26*/  {/*baseAddr*/ 0x0054,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 32 }, /*CAPTURE_CW_CORRECTED_10SE */

    /* COUNTER_CAPTURE_CW_CORRECTED_11SE */
  /*27*/  {/*baseAddr*/ 0x0058,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 32 }, /*CAPTURE_CW_CORRECTED_11SE */

    /* COUNTER_CAPTURE_CW_CORRECTED_12SE */
  /*28*/  {/*baseAddr*/ 0x005C,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 32 }, /*CAPTURE_CW_CORRECTED_12SE */

    /* COUNTER_CAPTURE_CW_CORRECTED_13SE */
  /*29*/  {/*baseAddr*/ 0x0060,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 32 }, /*CAPTURE_CW_CORRECTED_13SE */

    /* COUNTER_CAPTURE_CW_CORRECTED_14SE */
  /*30*/  {/*baseAddr*/ 0x0064,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 32 }, /*CAPTURE_CW_CORRECTED_14SE */

    /* COUNTER_CAPTURE_CW_CORRECTED_15SE */
  /*31*/  {/*baseAddr*/ 0x0068,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 32 }, /*CAPTURE_CW_CORRECTED_15SE */

    /* COUNTER_CAPTURE_TOTAL_SYMBOL_ERRORS_CORRECTED */
  /*32*/  {/*baseAddr*/ 0x006C,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 32 }, /*CAPTURE_TOTAL_SYMBOL_ERRORS_CORRECTED */

    /* TOTAL_CODEWORDS_RECIVED */
  /*33*/  {/*baseAddr*/ 0x0070,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 32 }, /*TOTAL_CW_RECEIVED */

    /* TOTAL_CODEWORDS_CORRECT */
  /*34*/  {/*baseAddr*/ 0x0074,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 32 }, /*TOTAL_CW_CORRECT */

    /* TOTATL_CODEWORDS_CORRECTED */
  /*35*/  {/*baseAddr*/ 0x0078,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 32 }, /*TOTAL_CW_CORRECTED */

    /* TOTATL_CODEWORDS_UNCORRECTABLE */
  /*36*/  {/*baseAddr*/ 0x007c,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 32 }, /*TOTATL_CW_UNCORRECTABLE */

    /* CODEWORDS_CORRECTED_1SE */
  /*37*/  {/*baseAddr*/ 0x0080,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 32 }, /*CW_CORRECTED_1SE */

    /* CODEWORDS_CORRECTED_2SE */
  /*38*/  {/*baseAddr*/ 0x0084,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 32 }, /*CW_CORRECTED_2SE */

    /* CODEWORDS_CORRECTED_3SE */
  /*39*/  {/*baseAddr*/ 0x0088,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 32 }, /*CW_CORRECTED_3SE */

    /* CODEWORDS_CORRECTED_4SE */
  /*40*/  {/*baseAddr*/ 0x008C,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 32 }, /*CW_CORRECTED_4SE */

    /* CODEWORDS_CORRECTED_5SE */
  /*41*/  {/*baseAddr*/ 0x0090,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 32 }, /*CW_CORRECTED_5SE */

    /* CODEWORDS_CORRECTED_6SE */
  /*42*/  {/*baseAddr*/ 0x0094,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 32 }, /*CW_CORRECTED_6SE */

    /* CODEWORDS_CORRECTED_7SE */
  /*43*/  {/*baseAddr*/ 0x0098,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 32 }, /*CW_CORRECTED_7SE */

    /* CODEWORDS_CORRECTED_8SE */
  /*44*/  {/*baseAddr*/ 0x009C,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 32 }, /*CW_CORRECTED_8SE */

    /* CODEWORDS_CORRECTED_9SE */
  /*45*/  {/*baseAddr*/ 0x00A0,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 32 }, /*CW_CORRECTED_9SE */

    /* CODEWORDS_CORRECTED_10SE */
  /*46*/  {/*baseAddr*/ 0x00A4,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 32 }, /*CW_CORRECTED_10SE */

    /* CODEWORDS_CORRECTED_11SE */
  /*47*/  {/*baseAddr*/ 0x00A8,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 32 }, /*CW_CORRECTED_11SE */

    /* CODEWORDS_CORRECTED_12SE */
  /*48*/  {/*baseAddr*/ 0x00AC,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 32 }, /*CW_CORRECTED_12SE */

    /* CODEWORDS_CORRECTED_13SE */
  /*49*/  {/*baseAddr*/ 0x00B0,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 32 }, /*CW_CORRECTED_13SE */

    /* CODEWORDS_CORRECTED_14SE */
  /*50*/  {/*baseAddr*/ 0x00B4,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 32 }, /*CW_CORRECTED_14SE */

    /* CODEWORDS_CORRECTED_15SE */
  /*51*/  {/*baseAddr*/ 0x00B8,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 32 }, /*CW_CORRECTED_15SE */

    /* TOTAL_SYMBOLS_ERROR_CORRECTED */
  /*51*/  {/*baseAddr*/ 0x00BC,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 32 }, /*TOTAL_SYMBOLS_ERROR_CORRECTED */


};

