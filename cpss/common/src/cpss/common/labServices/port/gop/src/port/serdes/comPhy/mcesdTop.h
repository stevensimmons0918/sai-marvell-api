#ifndef MCESD_TOP_H
#define MCESD_TOP_H

#define MCESD_API_MAJOR_VERSION 2
#define MCESD_API_MINOR_VERSION 5
#define MCESD_API_BUILD_ID      4

#define C12GP4      /* COMPHY_12G_PIPE4 */
#define C12GP41P2V  /* COMPHY_12G_PIPE4_1P2V */
#define C28GP4X1    /* COMPHY_28G_PIPE4_RPLL_1P2V */
#define C28GP4X2    /* COMPHY_28G_PIPE4_X2 */
#define C28GP4X4    /* COMPHY_28G_PIPE4_RPLL_X4_1P2V */
#define C56GX4      /* COMPHY_56G_X4 */
#define C112GX4     /* COMPHY_112G_X4 */
#define C56GP5X4    /* COMPHY_56G_PIPE5_2PLL_X4 */
#define C112GADCX4  /* COMPHY_112G_ADC_X4 */

/** CPSS addition - start **/
/* Flags to mark changes in MCESD file that were done by CPSS */
#define CPSS_ADDITION_START /* CPSS addition - start Flag */
#define CPSS_ADDITION_END   /* CPSS addition - end Flag */

#define N5XC56GP5X4	/* 5FFP_COMPHY_56G_PIPE5_X4_4PLL - X9380 */
/*
    N5XC56GP5X4 was taken from MCESD 2.8.2 :
    MCESD_API_MAJOR_VERSION 2
    MCESD_API_MINOR_VERSION 8
    MCESD_API_BUILD_ID      2
*/

#include <cpss/common/labServices/port/gop/port/serdes/comPhy/mvHwsComphySerdes.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>

/** CPSS addition - end **/
CPSS_ADDITION_END
#endif /* defined MCESD_TOP_H */
