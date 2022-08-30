/*
 * (c), Copyright 2009-2013, Marvell International Ltd.  (Marvell)
 *
 * This code contains confidential information of Marvell.
 * No rights are granted herein under any patent, mask work right or copyright
 * of Marvell or any third party. Marvell reserves the right at its sole
 * discretion to request that this code be immediately returned to Marvell.
 * This code is provided "as is". Marvell makes no warranties, expressed,
 * implied or otherwise, regarding its accuracy, completeness or performance.
 */
/**
 * @brief shaping calculation constants  &tm_shaping_uitils interface.
 *
* @file tm_shaping_utils_platform.h
*
* $Revision: 1.0 $
 */
#ifndef SCHED_SHAPING_UTILS_PLATFORM_H
#define SCHED_SHAPING_UTILS_PLATFORM__H


/******************************************************************
*       scheduling  scheme for BC2
* according to preliminary analysis
*******************************************************************/
#define MAX_DIVIDER_EXP     7       /* maximum value of divider exponent for BC2 nodes */

/*******************************************************************************************************
*  shaping configuration definitions
********************************************************************************************************/


#define     TOKEN_WIDTH_BITS            11

#define     MAX_POSSIBLE_TOKEN          (1<<TOKEN_WIDTH_BITS)-1

/* all values below  provides max b/w  100Gbit/s   and maximum accuracy */

#define     PORT_OPTIMAL_RES_EXP        4


/* FALCON unique defines*/

#define     FALCON_QUEUE_NODE_OPTIMAL_RES_EXP       7
#define     FALCON_A_NODE_OPTIMAL_RES_EXP       7
#define     FALCON_B_NODE_OPTIMAL_RES_EXP       7
#define     FALCON_C_NODE_OPTIMAL_RES_EXP       7


#define     FALCON_PORT_PERIODS_MAX         5000

#define     FALCON_TOKEN_WIDTH_BITS         12


#define     FALCON_MAX_POSSIBLE_TOKEN (1<<FALCON_TOKEN_WIDTH_BITS)-1

#define FALCON_PORT_UNIT                2000     /* 0.5 Gbit/second resolution*/

#define FALCON_OPTIMAL_PORT_FREQUENCY   120000000   /* 100 MHz  - this frequency can provide granularity of 1kbit/s for nodes - */


#define FALCON_Q_UNIT               100000       /* -  100 kbit/second resolution */
#define FALCON_A_UNIT               100000       /* -  100 kbit/second resolution */
#define FALCON_B_UNIT               100000       /* -  100 kbit/second resolution */
#define FALCON_C_UNIT               20000       /* -  20 kbit/second resolution */

#define HAWK_Q_UNIT               50000       /* -  50 kbit/second resolution */
#define HAWK_A_UNIT               50000       /* -  50 kbit/second resolution */
#define HAWK_B_UNIT               50000       /* -  50 kbit/second resolution */


#define PHOENIX_Q_UNIT            25000       /* -  25 kbit/second resolution */
#define PHOENIX_A_UNIT            25000       /* -  25 kbit/second resolution */
#define PHOENIX_B_UNIT            25000       /* -  25 kbit/second resolution */




#define FALCON_OPTIMAL_NODE_FREQUENCY     200000000
#define HAWK_OPTIMAL_NODE_FREQUENCY       204000000
#define PHOENIX_OPTIMAL_NODE_FREQUENCY    150000000







#endif   /* TM_SHAPING_UTILS_PLATFORM_H */
