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
* @file tm_shaping_utils.h
*
* $Revision: 3.0 $
 */
#ifndef SCHED_SHAPING_UTILS_H
#define SCHED_SHAPING_UTILS_H

#include <cpss/dxCh/dxChxGen/txq/private/Sched/platform/prvShedPlatformImplementationDefinitions.h>
#include <cpss/dxCh/dxChxGen/txq/private/Sched/platform/prvShedShapingUtilsPlatform.h>



int prvSchedShapingUtilsFindLKN(double desired_factor,
                    uint16_t *L_value,
                    uint16_t *K_value,
                    uint16_t *N_value);


/*************************
* returned values:
* 0 - ok
* TM_BW_OUT_OF_RANGE                        failed to configure
* TM_CONF_BURST_TOO_SMALL_FOR_GIVEN_RATE    desired cbs or ebs value is too small for desired cir/eir
*       in this case   values in  pcbs_kb/pebs_kb are updated to minimum possible value.
* TM_BW_UNDERFLOW               not enough accuracy
*************************/
int prvSchedShapingUtilsCalculateNodeShaping(
                            GT_U8       devNum,
                            double      cir_units,
                            double      eir_units,
                            uint16_t *  pcbs_kb,
                            uint16_t *  pebs_kb,
                            uint32_t    token_resolution_multiplyer,
                            uint8_t *   pcir_divider_exp,
                            uint8_t *   peir_divider_exp,
                            uint8_t *   pcir_resolution,
                            uint8_t *   peir_resolution,
                            uint16_t *  pcir_token,
                            uint16_t *  peir_token  ,
                            PRV_CPSS_SCHED_HANDLE hndl,
                            double      * out_cir_accuracy_ptr,
                            double      * out_eir_accuracy_ptr
                            );



/*************************
* returned values:
* 0 - ok
* TM_BW_OUT_OF_RANGE            failed to configure
* TM_BW_UNDERFLOW               not enough accuracy
* TM_CONF_BURST_TOO_SMALL_FOR_GIVEN_RATE    desired cbs or ebs value is too small for desired cir/eir
*       in this case   values in  pcbs_kb/pebs_kb are updated to minimum possible value.
*************************/
int prvSchedShapingUtilsTestNodeShaping( double     cir_units,                  /* input */
                        double      eir_units,                  /* input */
                        uint32_t    token_resolution_multiplyer,/* input */
                        uint32_t *  pcbs_kb,                    /* IN/OUT */
                        uint32_t *  pebs_kb                     /* IN/OUT */
                        );



/*#define       PORT_PERIODS_MAX            4000*/

/****************************************************************
* returned values:
* returned values:
* 0 - ok
* TM_BW_OUT_OF_RANGE            failed to configure
* TM_CONF_BURST_TOO_SMALL_FOR_GIVEN_RATE    desired cbs or ebs value is too small for desired cir/eir
*       in this case   values in  pcbs_kb/pebs_kb are updated to minimum possible value.
********************************************************************/

int prvSchedShapingUtilsCalculatePortShaping( double        cir_units,                  /* input */
                            double      eir_units,                  /* input */
                            uint32_t *  pcbs_kb,                    /* input */
                            uint32_t *  pebs_kb,                    /* input */
                            uint32_t    token_resolution_multiplyer,/* input */
                            uint16_t *  pperiods,                   /* OUT */
                            uint8_t *   pcir_resolution,            /* OUT */
                            uint8_t *   peir_resolution,            /* OUT */
                            uint16_t *  pcir_token,                 /* OUT */
                            uint16_t *  peir_token                  /* OUT */
                            );

GT_STATUS  prvSchedShapingBurstOptimizationSet
(
    GT_U32  devNum,
    GT_BOOL optimizationOn
);
GT_STATUS prvSchedShapingLogResultSet
(
    GT_U32  devNum,
    GT_BOOL logOn
);

#ifdef PRINT_SHAPING_RESULTS

    #define PRINT_NODE_SHAPING_CALCULATION_RESULT(cir_bw, eir_bw)   \
    {\
        \
        double test_cir_bw;\
        double test_eir_bw;\
        {\
            if (cir_bw)\
            {\
                test_cir_bw=8.0*((double)ctl->level_data[level].level_update_frequency)/(double)ctl->level_data[level].per_interval*cir_token/(1<<min_div_exp);\
                if (cir_resolution) test_cir_bw*=(1<<ctl->level_data[level].token_res_exp);\
                schedPrintf("desired cir=%13lu accuracy=%8.5f (token=%4ld, div_exp=%2d, res_bit=%d) ",cir_bw,(test_cir_bw - cir_bw*1000.0)/test_cir_bw,cir_token, min_div_exp, cir_resolution);\
            }\
            if (eir_bw)\
            {\
                test_eir_bw=8.0*((double)ctl->level_data[level].level_update_frequency)/(double)ctl->level_data[level].per_interval*eir_token/(1<<max_div_exp);\
                if (eir_resolution) test_eir_bw*=(1<<ctl->level_data[level].token_res_exp);\
                schedPrintf("desired eir=%13lu accuracy=%8.5f (token=%4ld, div_exp=%2d, res_bit=%d) ",eir_bw,(test_eir_bw - eir_bw*1000.0)/test_eir_bw, eir_token, max_div_exp,eir_resolution );\
            }\
            schedPrintf("\n");\
        }\
    }
    /*
    #define PRINT_NODE_SHAPING_CALCULATION_RESULT(cir_bw,eir_bw)\
        PRINT_SHAPING_CALCULATION_RESULT(cir_bw,eir_bw,level,div_exp,(1<<div_exp),cir_token,eir_token,cir_resolution, eir_resolution)
    #define PRINT_PORT_SHAPING_CALCULATION_RESULT(cir_bw,eir_bw)\
        PRINT_SHAPING_CALCULATION_RESULT(cir_bw,eir_bw,SCHED_P_LEVEL,periods,periods,min_token,max_token,res_min_bw, res_max_bw)
    */

    #define PRINT_PORT_SHAPING_CALCULATION_RESULT(cir_bw, eir_bw)   \
    {\
        \
        double test_cir_bw;\
        double test_eir_bw;\
        {\
            schedPrintf("periods=%5u  ",periods);\
            if (cir_bw)\
            {\
                test_cir_bw=8.0*((double)ctl->level_data[SCHED_P_LEVEL].level_update_frequency)/(double)ctl->level_data[SCHED_P_LEVEL].per_interval*min_token/periods;\
                if (res_min_bw) test_cir_bw*=(1<<ctl->level_data[SCHED_P_LEVEL].token_res_exp);\
                schedPrintf("desired cir=%13lu accuracy=%8.5f (token=%4ld, res_bit=%d) ",cir_bw,(test_cir_bw - cir_bw*1000.0)/test_cir_bw,min_token,res_min_bw);\
            }\
            if (eir_bw)\
            {\
                test_eir_bw=8.0*((double)ctl->level_data[SCHED_P_LEVEL].level_update_frequency)/(double)ctl->level_data[SCHED_P_LEVEL].per_interval*max_token/periods;\
                if (res_max_bw) test_eir_bw*=(1<<ctl->level_data[SCHED_P_LEVEL].token_res_exp);\
                schedPrintf("desired eir=%13lu accuracy=%8.5f (token=%4ld,res_bit=%d) ",eir_bw,(test_eir_bw - eir_bw*1000.0)/test_eir_bw, max_token,res_max_bw );\
            }\
            schedPrintf("\n");\
        }\
    }
#else
    /* do nothing */
    #define PRINT_NODE_SHAPING_CALCULATION_RESULT(cir_bw, eir_bw)
    #define PRINT_PORT_SHAPING_CALCULATION_RESULT(cir_bw, eir_bw)
#endif


#endif   /* TM_SHAPING_UTILS_H */
