/*
 * (c), Copyright 2009-2014, Marvell International Ltd.  (Marvell)
 *
 * This code contains confidential information of Marvell.
 * No rights are granted herein under any patent, mask work right or copyright
 * of Marvell or any third party. Marvell reserves the right at its sole
 * discretion to request that this code be immediately returned to Marvell.
 * This code is provided "as is". Marvell makes no warranties, expressed,
 * implied or otherwise, regarding its accuracy, completeness or performance.
 */
/**
 * @brief set default values for TM SW  primitives
 *
* @file tm_set_local_db_defaults.c
*
* $Revision: 2.0 $
 */

#include <cpss/dxCh/dxChxGen/txq/private/Sched/prvSchedErrCodes.h>
#include <cpss/dxCh/dxChxGen/txq/private/Sched/core/prvSchedSetLocalDbDefaults.h>
#include <cpss/dxCh/dxChxGen/txq/private/Sched/core/prvSchedCoreTypes.h>

#include <stdlib.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

static uint16_t convert_elig_func_to_value(struct sched_elig_prio_func_out *elig_func);

/*Default node shaping configuration*/

#define PRV_SCHED_SIP_6_DEFAULT_BW  0 /*(420*1000*1000) *//*420 Gygabyte*/
#define PRV_SCHED_SIP_6_DEFAULT_BURST 0xFFF
#define PRV_SCHED_SIP_6_DEFAULT_TOKEN 256
#define PRV_SCHED_SIP_6_DEFAULT_DIV_EXP 0
#define PRV_SCHED_SIP_6_DEFAULT_RES 1


#define NOT_LEGAL_STATE NOT_ELIGIBLE



/**
 */
int prvSchedSetSwConfDefault(void * hndl)
{
    int i;

    TM_CTL(ctl, hndl);

    ctl->periodic_scheme_state = TM_DISABLE; /* periodic scheme
                                              * configured - yes/no */
    ctl->level_data[SCHED_Q_LEVEL].per_interval = 0x9C4;
    ctl->level_data[SCHED_A_LEVEL].per_interval = 0x9C4;
    ctl->level_data[SCHED_B_LEVEL].per_interval = 0x9C4;
    ctl->level_data[SCHED_C_LEVEL].per_interval = 0x271;
    ctl->level_data[SCHED_P_LEVEL].per_interval = 0x10;

    ctl->level_data[SCHED_Q_LEVEL].scrub_slots = 32;
    ctl->level_data[SCHED_A_LEVEL].scrub_slots = 16;
    ctl->level_data[SCHED_B_LEVEL].scrub_slots = 8;
    ctl->level_data[SCHED_C_LEVEL].scrub_slots = 4;
    ctl->level_data[SCHED_P_LEVEL].scrub_slots = 4;


    for (i=SCHED_Q_LEVEL; i<=SCHED_P_LEVEL; i++)
    {
        ctl->level_data[i].shaping_status = TM_DISABLE;
        /* Periodic shaper update process disabled for all the level Nodes */
        ctl->level_data[i].shaper_dec = TM_DISABLE;
        ctl->level_data[i].L = 0x1;
        ctl->level_data[i].K = 0x2;
        ctl->level_data[i].N = 0x15;
    }

    ctl->dwrr_bytes_burst_limit = 0x40;
    ctl->tree_dwrr_enable_per_priority_mask = 0;
    ctl->tree_deq_status = TM_ENABLE;

    return 0;
}


/**
 */
int prvSchedSetSwGenConfDefault(void * hndl)
{
    TM_CTL(ctl, hndl);
    ctl->port_ext_bp_en = 1;

    return 0;
}


/**
 */
int prvSchedSetSwShapingProfileDefault(struct prvCpssDxChTxqSchedShapingProfile *profilePtr)
{
    /* the following settings represent infinite profile for BW of 420 G */
    profilePtr->level = ALL_LEVELS;
    profilePtr->min_token = PRV_SCHED_SIP_6_DEFAULT_TOKEN;
    profilePtr->min_burst_size = PRV_SCHED_SIP_6_DEFAULT_BURST;
    profilePtr->max_token = 0;
    profilePtr->max_burst_size = PRV_SCHED_SIP_6_DEFAULT_BURST;
    profilePtr->min_token_res = PRV_SCHED_SIP_6_DEFAULT_RES;
    profilePtr->max_token_res = PRV_SCHED_SIP_6_DEFAULT_RES;
    profilePtr->use_counter = 0;
    profilePtr->min_div_exp = PRV_SCHED_SIP_6_DEFAULT_DIV_EXP;
    profilePtr->max_div_exp = PRV_SCHED_SIP_6_DEFAULT_DIV_EXP;
    /* not used , only in order to fill memory */
    profilePtr->use_counter = 0;

    return 0;
}







/**
 */
void prvSchedSetSw_queue_Default(struct tm_queue * queue_node , int bIncludeMapping)
{
    if (bIncludeMapping)
    {
        queue_node->mapping.nodeStatus=TM_NODE_FREE;
    }
    queue_node->installed = TM_DISABLE;
    queue_node->shaping_profile_ref = SCHED_INF_SHP_PROFILE;
    queue_node->dwrr_quantum = 0x40;

    queue_node->elig_prio_func = TM_NODE_DISABLED_FUN;
    queue_node->node_temporary_disabled = 0;
    queue_node->cir_bw = PRV_SCHED_SIP_6_DEFAULT_BW;
    queue_node->eir_bw = 0;
    queue_node->cbs= PRV_SCHED_SIP_6_DEFAULT_BURST;
    queue_node->ebs=PRV_SCHED_SIP_6_DEFAULT_BURST;
}



/**
 */
void prvSchedSetSw_a_node_Default(struct tm_a_node *  node, int bIncludeMapping)
{
    if (bIncludeMapping)
    {
        node->mapping.nodeStatus=TM_NODE_FREE;
    }
    node->shaping_profile_ref = SCHED_INF_SHP_PROFILE;
    node->dwrr_quantum = 0x40;
    node->dwrr_enable_per_priority_mask = 0;

    node->elig_prio_func = TM_NODE_DISABLED_FUN;
    node->node_temporary_disabled = 0;
}


/**
 */
void prvSchedSetSw_b_node_Default(struct tm_b_node * node, int bIncludeMapping)
{
    if (bIncludeMapping)
    {
        node->mapping.nodeStatus=TM_NODE_FREE;
    }
    node->shaping_profile_ref = SCHED_INF_SHP_PROFILE;
    node->dwrr_quantum = 0x40;
    node->dwrr_enable_per_priority_mask = 0;

    node->elig_prio_func = TM_NODE_DISABLED_FUN;
    node->node_temporary_disabled = 0;
}


/**
 */
void prvSchedSetSw_c_node_Default(struct tm_c_node * node, int bIncludeMapping)
{
    if (bIncludeMapping)
    {
        node->mapping.nodeStatus=TM_NODE_FREE;
    }
    node->shaping_profile_ref = SCHED_INF_SHP_PROFILE;
    node->dwrr_quantum = 0x40;
    node->dwrr_enable_per_priority_mask = 0;


    node->elig_prio_func = TM_NODE_DISABLED_FUN;
    node->node_temporary_disabled = 0;
}


/**
 */
void prvSchedSetSwPortDefault(struct tm_port * port_node, int bIncludeMapping)
{

    if (bIncludeMapping)
    {
        port_node->mapping.nodeStatus=TM_NODE_FREE;
    }
    port_node->dwrr_quantum_per_level[0].quantum = 0x10;
    port_node->dwrr_quantum_per_level[1].quantum = 0x10;
    port_node->dwrr_quantum_per_level[2].quantum = 0x10;
    port_node->dwrr_quantum_per_level[3].quantum = 0x10;
    port_node->dwrr_quantum_per_level[4].quantum = 0x10;
    port_node->dwrr_quantum_per_level[5].quantum = 0x10;
    port_node->dwrr_quantum_per_level[6].quantum = 0x10;
    port_node->dwrr_quantum_per_level[7].quantum = 0x10;
    port_node->dwrr_enable_per_priority_mask = 0;
    port_node->cir_token = 0x7FF;
    port_node->eir_token = 0x7FF;
    port_node->periods = 1;
    port_node->min_token_res = 0;
    port_node->max_token_res = 0;
    port_node->cir_burst_size = 0x1FFFF;
    port_node->eir_burst_size = 0x1FFFF;


    port_node->elig_prio_func = TM_NODE_DISABLED_FUN;
    port_node->node_temporary_disabled = 0;
}



#define SHED_PRIO_0     0
#define SHED_PRIO_1     1
#define SHED_PRIO_2     2
#define SHED_PRIO_3     3
#define SHED_PRIO_4     4
#define SHED_PRIO_5     5
#define SHED_PRIO_6     6
#define SHED_PRIO_7     7

#define PROP_PRIO_0     0
#define PROP_PRIO_1     1
#define PROP_PRIO_2     2
#define PROP_PRIO_3     3
#define PROP_PRIO_4     4
#define PROP_PRIO_5     5
#define PROP_PRIO_6     6
#define PROP_PRIO_7     7


#define FIX_PRIO_0      0
#define FIX_PRIO_1      1
#define FIX_PRIO_2      2
#define FIX_PRIO_3      3
#define FIX_PRIO_4      4
#define FIX_PRIO_5      5
#define FIX_PRIO_6      6
#define FIX_PRIO_7      7


#define USE_MIN_TB      1
#define USE_MAX_TB      1
#define NOT_USE_MIN_TB  0
#define NOT_USE_MAX_TB  0

#define POS_MIN_TB      0
#define POS_MAX_TB      0
#define NEG_MIN_TB      1
#define NEG_MAX_TB      1


#define ELIGIBLE(shed_priority, propagated_priority, use_minTB, useMaxTB) \
        ((1<<8) | (shed_priority << 5) | ( propagated_priority << 2) | (use_minTB <<1) | useMaxTB)
#define NOT_ELIGIBLE    0

#define STRICT_PRIORITY(shed_priority, propagated_priority)  ELIGIBLE(shed_priority, propagated_priority, NOT_USE_MIN_TB, NOT_USE_MAX_TB)

#define FIX_PRIORITY(priority)  ELIGIBLE(priority, priority, NOT_USE_MIN_TB, NOT_USE_MAX_TB)


#define Q_ENTRY(min_flag, max_flag) tbl_entry.func_out[2*min_flag+max_flag]

#define QUEUE_ELIG_FUNCTION(function_ID,p0,p1,p2,p3) \
    if((int)function_ID >= (int)PRV_SCHED_ELIG_FUNC_TABLE_SIZE){PRV_SCHED_ASSERT(TM_BW_OUT_OF_RANGE)}\
    func_table[function_ID].p0;\
    func_table[function_ID].p1;\
    func_table[function_ID].p2;\
    func_table[function_ID].p3;

#define QUEUE_ELIG_FUN_FIXED_PRIORITY(function_ID, priority) \
    {\
        QUEUE_ELIG_FUNCTION(function_ID,\
            Q_ENTRY(POS_MIN_TB,POS_MAX_TB) = priority,\
            Q_ENTRY(POS_MIN_TB,NEG_MAX_TB) = priority,\
            Q_ENTRY(NEG_MIN_TB,POS_MAX_TB) = priority,\
            Q_ENTRY(NEG_MIN_TB,NEG_MAX_TB) = priority)\
    }

#define QUEUE_DEQ_DISABLE(function_ID) QUEUE_ELIG_FUN_FIXED_PRIORITY(function_ID, NOT_ELIGIBLE)


#define N_ENTRY( min_flag, max_flag, priority)  tbl_entry[min_flag*4 +max_flag*2 + priority/4 ].func_out[ priority & 3]

#define NODE_ELIG_FUNCTION(function_ID,p0,p1,p2,p3,p4,p5,p6,p7,p8,p9,p10,p11,p12,p13,p14,p15,p16,p17,p18,p19,p20,p21,p22,p23,p24,p25,p26,p27,p28,p29,p30,p31 ) \
    if((int)function_ID >= (int)PRV_SCHED_ELIG_FUNC_TABLE_SIZE){PRV_SCHED_ASSERT(TM_BW_OUT_OF_RANGE)}\
    func_table[function_ID].p0;\
    func_table[function_ID].p1;\
    func_table[function_ID].p2;\
    func_table[function_ID].p3;\
    func_table[function_ID].p4;\
    func_table[function_ID].p5;\
    func_table[function_ID].p6;\
    func_table[function_ID].p7;\
    func_table[function_ID].p8;\
    func_table[function_ID].p9;\
    func_table[function_ID].p10;\
    func_table[function_ID].p11;\
    func_table[function_ID].p12;\
    func_table[function_ID].p13;\
    func_table[function_ID].p14;\
    func_table[function_ID].p15;\
    func_table[function_ID].p16;\
    func_table[function_ID].p17;\
    func_table[function_ID].p18;\
    func_table[function_ID].p19;\
    func_table[function_ID].p20;\
    func_table[function_ID].p21;\
    func_table[function_ID].p22;\
    func_table[function_ID].p23;\
    func_table[function_ID].p24;\
    func_table[function_ID].p25;\
    func_table[function_ID].p26;\
    func_table[function_ID].p27;\
    func_table[function_ID].p28;\
    func_table[function_ID].p29;\
    func_table[function_ID].p30;\
    func_table[function_ID].p31;\

#define NODE_ELIG_FUN_FIXED_PRIORITY(function_ID, priority) \
    {\
        NODE_ELIG_FUNCTION(function_ID,\
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_0) = priority, \
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_1) = priority, \
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_2) = priority, \
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_3) = priority, \
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_4) = priority, \
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_5) = priority, \
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_6) = priority, \
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_7) = priority, \
            /*  ---------------------------------- */\
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_0) = priority, \
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_1) = priority, \
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_2) = priority, \
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_3) = priority, \
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_4) = priority, \
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_5) = priority, \
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_6) = priority, \
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_7) = priority, \
            /*  ---------------------------------- */\
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_0) = priority, \
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_1) = priority, \
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_2) = priority, \
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_3) = priority, \
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_4) = priority, \
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_5) = priority, \
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_6) = priority, \
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_7) = priority, \
            /*  ---------------------------------- */\
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_0) = priority, \
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_1) = priority, \
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_2) = priority, \
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_3) = priority, \
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_4) = priority, \
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_5) = priority, \
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_6) = priority, \
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_7) = priority)\
    }

#define NODE_DEQ_DISABLE(function_ID)   { NODE_ELIG_FUN_FIXED_PRIORITY(function_ID,NOT_ELIGIBLE) }

#define GUARANTEED_BW_WITH_SHAPING_ELIG_FUNC(_name) \
                                 QUEUE_ELIG_FUNCTION(_name,\
                                 Q_ENTRY(POS_MIN_TB,POS_MAX_TB) = ELIGIBLE( SHED_PRIO_7, SHED_PRIO_7 , USE_MIN_TB,     USE_MAX_TB),\
                                 Q_ENTRY(POS_MIN_TB,NEG_MAX_TB) = ELIGIBLE( SHED_PRIO_0, PROP_PRIO_0 , USE_MIN_TB,     NOT_USE_MAX_TB),\
                                 Q_ENTRY(NEG_MIN_TB,POS_MAX_TB) = NOT_ELIGIBLE,\
                                 Q_ENTRY(NEG_MIN_TB,NEG_MAX_TB) = NOT_ELIGIBLE)

#define GUARANTEED_BW_NO_SHAPING_ELIG_FUNC(_name) \
                                 QUEUE_ELIG_FUNCTION(_name,\
                                 Q_ENTRY(POS_MIN_TB,POS_MAX_TB) = ELIGIBLE( SHED_PRIO_7, PROP_PRIO_0 , NOT_USE_MIN_TB,     USE_MAX_TB),\
                                 Q_ENTRY(POS_MIN_TB,NEG_MAX_TB) = ELIGIBLE( SHED_PRIO_0, PROP_PRIO_0 , NOT_USE_MIN_TB,     NOT_USE_MAX_TB),\
                                 Q_ENTRY(NEG_MIN_TB,POS_MAX_TB) = ELIGIBLE( SHED_PRIO_7, PROP_PRIO_0 , NOT_USE_MIN_TB,     USE_MAX_TB),\
                                 Q_ENTRY(NEG_MIN_TB,NEG_MAX_TB) = ELIGIBLE( SHED_PRIO_0, PROP_PRIO_0 , NOT_USE_MIN_TB,     NOT_USE_MAX_TB))


#define GUARANTEED_BW_NO_SHAPING_USE_MIN_BUCKET_ELIG_FUNC(_name) \
                                 QUEUE_ELIG_FUNCTION(_name,\
                                 Q_ENTRY(POS_MIN_TB,POS_MAX_TB) = ELIGIBLE( SHED_PRIO_7, PROP_PRIO_0 , USE_MIN_TB,     NOT_USE_MAX_TB),\
                                 Q_ENTRY(POS_MIN_TB,NEG_MAX_TB) = ELIGIBLE( SHED_PRIO_7, PROP_PRIO_0 , USE_MIN_TB,     NOT_USE_MAX_TB),\
                                 Q_ENTRY(NEG_MIN_TB,POS_MAX_TB) = ELIGIBLE( SHED_PRIO_0, PROP_PRIO_0 , NOT_USE_MIN_TB,     NOT_USE_MAX_TB),\
                                 Q_ENTRY(NEG_MIN_TB,NEG_MAX_TB) = ELIGIBLE( SHED_PRIO_0, PROP_PRIO_0 , NOT_USE_MIN_TB,     NOT_USE_MAX_TB))




/*   Queues default EligibleFunction   table */
/**
 */
int prvSchedInitDefaultQueueEligPrioFuncTable(struct tm_elig_prio_func_queue *func_table,int bucketForShaping)
{
    int i,rc=0;
    /* initialize memory */
    for (i=0; i < PRV_SCHED_ELIG_FUNC_TABLE_SIZE ; i++ ) QUEUE_DEQ_DISABLE(i)
    /*  setup default functions  */
    QUEUE_ELIG_FUNCTION( PRV_SCHED_ELIG_Q_PRIO0,
            Q_ENTRY(POS_MIN_TB,POS_MAX_TB) = FIX_PRIORITY(FIX_PRIO_0),
            Q_ENTRY(POS_MIN_TB,NEG_MAX_TB) = FIX_PRIORITY(FIX_PRIO_0),
            Q_ENTRY(NEG_MIN_TB,POS_MAX_TB) = FIX_PRIORITY(FIX_PRIO_0),
            Q_ENTRY(NEG_MIN_TB,NEG_MAX_TB) = FIX_PRIORITY(FIX_PRIO_0))

    QUEUE_ELIG_FUNCTION( PRV_SCHED_ELIG_Q_PRIO1,
            Q_ENTRY(POS_MIN_TB,POS_MAX_TB) = FIX_PRIORITY(FIX_PRIO_1),
            Q_ENTRY(POS_MIN_TB,NEG_MAX_TB) = FIX_PRIORITY(FIX_PRIO_1),
            Q_ENTRY(NEG_MIN_TB,POS_MAX_TB) = FIX_PRIORITY(FIX_PRIO_1),
            Q_ENTRY(NEG_MIN_TB,NEG_MAX_TB) = FIX_PRIORITY(FIX_PRIO_1))

    QUEUE_ELIG_FUNCTION( PRV_SCHED_ELIG_Q_PRIO2,
            Q_ENTRY(POS_MIN_TB,POS_MAX_TB) = FIX_PRIORITY(FIX_PRIO_2),
            Q_ENTRY(POS_MIN_TB,NEG_MAX_TB) = FIX_PRIORITY(FIX_PRIO_2),
            Q_ENTRY(NEG_MIN_TB,POS_MAX_TB) = FIX_PRIORITY(FIX_PRIO_2),
            Q_ENTRY(NEG_MIN_TB,NEG_MAX_TB) = FIX_PRIORITY(FIX_PRIO_2))

    QUEUE_ELIG_FUNCTION( PRV_SCHED_ELIG_Q_PRIO3,
            Q_ENTRY(POS_MIN_TB,POS_MAX_TB) = FIX_PRIORITY(FIX_PRIO_3),
            Q_ENTRY(POS_MIN_TB,NEG_MAX_TB) = FIX_PRIORITY(FIX_PRIO_3),
            Q_ENTRY(NEG_MIN_TB,POS_MAX_TB) = FIX_PRIORITY(FIX_PRIO_3),
            Q_ENTRY(NEG_MIN_TB,NEG_MAX_TB) = FIX_PRIORITY(FIX_PRIO_3))

    QUEUE_ELIG_FUNCTION( PRV_SCHED_ELIG_Q_MIN_SHP_PRIO0,
            Q_ENTRY(POS_MIN_TB,POS_MAX_TB) = ELIGIBLE( SHED_PRIO_0, PROP_PRIO_0 , USE_MIN_TB, NOT_USE_MAX_TB),
            Q_ENTRY(POS_MIN_TB,NEG_MAX_TB) = ELIGIBLE( SHED_PRIO_0, PROP_PRIO_0 , USE_MIN_TB, NOT_USE_MAX_TB),
            Q_ENTRY(NEG_MIN_TB,POS_MAX_TB) = NOT_ELIGIBLE,
            Q_ENTRY(NEG_MIN_TB,NEG_MAX_TB) = NOT_ELIGIBLE)

    QUEUE_ELIG_FUNCTION( PRV_SCHED_ELIG_Q_MIN_SHP_PRIO1,
            Q_ENTRY(POS_MIN_TB,POS_MAX_TB) = ELIGIBLE( SHED_PRIO_1, PROP_PRIO_1 , USE_MIN_TB, NOT_USE_MAX_TB),
            Q_ENTRY(POS_MIN_TB,NEG_MAX_TB) = ELIGIBLE( SHED_PRIO_1, PROP_PRIO_1 , USE_MIN_TB, NOT_USE_MAX_TB),
            Q_ENTRY(NEG_MIN_TB,POS_MAX_TB) = NOT_ELIGIBLE,
            Q_ENTRY(NEG_MIN_TB,NEG_MAX_TB) = NOT_ELIGIBLE)

    QUEUE_ELIG_FUNCTION( PRV_SCHED_ELIG_Q_MIN_SHP_PRIO2,
            Q_ENTRY(POS_MIN_TB,POS_MAX_TB) = ELIGIBLE( SHED_PRIO_2, PROP_PRIO_2 , USE_MIN_TB, NOT_USE_MAX_TB),
            Q_ENTRY(POS_MIN_TB,NEG_MAX_TB) = ELIGIBLE( SHED_PRIO_2, PROP_PRIO_2 , USE_MIN_TB, NOT_USE_MAX_TB),
            Q_ENTRY(NEG_MIN_TB,POS_MAX_TB) = NOT_ELIGIBLE,
            Q_ENTRY(NEG_MIN_TB,NEG_MAX_TB) = NOT_ELIGIBLE)

    QUEUE_ELIG_FUNCTION( PRV_SCHED_ELIG_Q_MIN_SHP_PRIO3,
            Q_ENTRY(POS_MIN_TB,POS_MAX_TB) = ELIGIBLE( SHED_PRIO_3, PROP_PRIO_3 , USE_MIN_TB, NOT_USE_MAX_TB),
            Q_ENTRY(POS_MIN_TB,NEG_MAX_TB) = ELIGIBLE( SHED_PRIO_3, PROP_PRIO_3 , USE_MIN_TB, NOT_USE_MAX_TB),
            Q_ENTRY(NEG_MIN_TB,POS_MAX_TB) = NOT_ELIGIBLE,
            Q_ENTRY(NEG_MIN_TB,NEG_MAX_TB) = NOT_ELIGIBLE)

    QUEUE_ELIG_FUNCTION( PRV_SCHED_ELIG_Q_PRIO4,
            Q_ENTRY(POS_MIN_TB,POS_MAX_TB) = FIX_PRIORITY(FIX_PRIO_4),
            Q_ENTRY(POS_MIN_TB,NEG_MAX_TB) = FIX_PRIORITY(FIX_PRIO_4),
            Q_ENTRY(NEG_MIN_TB,POS_MAX_TB) = FIX_PRIORITY(FIX_PRIO_4),
            Q_ENTRY(NEG_MIN_TB,NEG_MAX_TB) = FIX_PRIORITY(FIX_PRIO_4))

    QUEUE_ELIG_FUNCTION( PRV_SCHED_ELIG_Q_PRIO5,
            Q_ENTRY(POS_MIN_TB,POS_MAX_TB) = FIX_PRIORITY(FIX_PRIO_5),
            Q_ENTRY(POS_MIN_TB,NEG_MAX_TB) = FIX_PRIORITY(FIX_PRIO_5),
            Q_ENTRY(NEG_MIN_TB,POS_MAX_TB) = FIX_PRIORITY(FIX_PRIO_5),
            Q_ENTRY(NEG_MIN_TB,NEG_MAX_TB) = FIX_PRIORITY(FIX_PRIO_5))

    QUEUE_ELIG_FUNCTION( PRV_SCHED_ELIG_Q_PRIO6,
            Q_ENTRY(POS_MIN_TB,POS_MAX_TB) = FIX_PRIORITY(FIX_PRIO_6),
            Q_ENTRY(POS_MIN_TB,NEG_MAX_TB) = FIX_PRIORITY(FIX_PRIO_6),
            Q_ENTRY(NEG_MIN_TB,POS_MAX_TB) = FIX_PRIORITY(FIX_PRIO_6),
            Q_ENTRY(NEG_MIN_TB,NEG_MAX_TB) = FIX_PRIORITY(FIX_PRIO_6))

    QUEUE_ELIG_FUNCTION( PRV_SCHED_ELIG_Q_PRIO7,
            Q_ENTRY(POS_MIN_TB,POS_MAX_TB) = FIX_PRIORITY(FIX_PRIO_7),
            Q_ENTRY(POS_MIN_TB,NEG_MAX_TB) = FIX_PRIORITY(FIX_PRIO_7),
            Q_ENTRY(NEG_MIN_TB,POS_MAX_TB) = FIX_PRIORITY(FIX_PRIO_7),
            Q_ENTRY(NEG_MIN_TB,NEG_MAX_TB) = FIX_PRIORITY(FIX_PRIO_7))

    QUEUE_ELIG_FUNCTION( PRV_SCHED_ELIG_Q_MIN_SHP_PRIO4,
            Q_ENTRY(POS_MIN_TB,POS_MAX_TB) = ELIGIBLE( SHED_PRIO_4, PROP_PRIO_4 , USE_MIN_TB, NOT_USE_MAX_TB),
            Q_ENTRY(POS_MIN_TB,NEG_MAX_TB) = ELIGIBLE( SHED_PRIO_4, PROP_PRIO_4 , USE_MIN_TB, NOT_USE_MAX_TB),
            Q_ENTRY(NEG_MIN_TB,POS_MAX_TB) = NOT_ELIGIBLE,
            Q_ENTRY(NEG_MIN_TB,NEG_MAX_TB) = NOT_ELIGIBLE)

    QUEUE_ELIG_FUNCTION( PRV_SCHED_ELIG_Q_MIN_SHP_PRIO5,
            Q_ENTRY(POS_MIN_TB,POS_MAX_TB) = ELIGIBLE( SHED_PRIO_5, PROP_PRIO_5 , USE_MIN_TB, NOT_USE_MAX_TB),
            Q_ENTRY(POS_MIN_TB,NEG_MAX_TB) = ELIGIBLE( SHED_PRIO_5, PROP_PRIO_5 , USE_MIN_TB, NOT_USE_MAX_TB),
            Q_ENTRY(NEG_MIN_TB,POS_MAX_TB) = NOT_ELIGIBLE,
            Q_ENTRY(NEG_MIN_TB,NEG_MAX_TB) = NOT_ELIGIBLE)

    QUEUE_ELIG_FUNCTION( PRV_SCHED_ELIG_Q_MIN_SHP_PRIO6,
            Q_ENTRY(POS_MIN_TB,POS_MAX_TB) = ELIGIBLE( SHED_PRIO_6, PROP_PRIO_7 , USE_MIN_TB, NOT_USE_MAX_TB),
            Q_ENTRY(POS_MIN_TB,NEG_MAX_TB) = ELIGIBLE( SHED_PRIO_6, PROP_PRIO_7 , USE_MIN_TB, NOT_USE_MAX_TB),
            Q_ENTRY(NEG_MIN_TB,POS_MAX_TB) = NOT_ELIGIBLE,
            Q_ENTRY(NEG_MIN_TB,NEG_MAX_TB) = NOT_ELIGIBLE)

    QUEUE_ELIG_FUNCTION( PRV_SCHED_ELIG_Q_MIN_SHP_PRIO7,
            Q_ENTRY(POS_MIN_TB,POS_MAX_TB) = ELIGIBLE( SHED_PRIO_7, PROP_PRIO_7 , USE_MIN_TB, NOT_USE_MAX_TB),
            Q_ENTRY(POS_MIN_TB,NEG_MAX_TB) = ELIGIBLE( SHED_PRIO_7, PROP_PRIO_7 , USE_MIN_TB, NOT_USE_MAX_TB),
            Q_ENTRY(NEG_MIN_TB,POS_MAX_TB) = NOT_ELIGIBLE,
            Q_ENTRY(NEG_MIN_TB,NEG_MAX_TB) = NOT_ELIGIBLE)

    QUEUE_ELIG_FUNCTION(PRV_SCHED_ELIG_Q_SHP,
            Q_ENTRY(POS_MIN_TB,POS_MAX_TB) = ELIGIBLE( SHED_PRIO_1, PROP_PRIO_1 , USE_MIN_TB,     NOT_USE_MAX_TB),
            Q_ENTRY(POS_MIN_TB,NEG_MAX_TB) = ELIGIBLE( SHED_PRIO_1, PROP_PRIO_1 , USE_MIN_TB,     NOT_USE_MAX_TB),
            Q_ENTRY(NEG_MIN_TB,POS_MAX_TB) = ELIGIBLE( SHED_PRIO_0, PROP_PRIO_0 , NOT_USE_MIN_TB, USE_MAX_TB),
            Q_ENTRY(NEG_MIN_TB,NEG_MAX_TB) = NOT_ELIGIBLE)

    QUEUE_ELIG_FUNCTION(PRV_SCHED_ELIG_Q_MAX_INC_MIN_SHP,
             Q_ENTRY(POS_MIN_TB,POS_MAX_TB) = ELIGIBLE( SHED_PRIO_1, PROP_PRIO_1 , USE_MIN_TB,     USE_MAX_TB),
             Q_ENTRY(POS_MIN_TB,NEG_MAX_TB) = ELIGIBLE( SHED_PRIO_1, PROP_PRIO_1 , USE_MIN_TB,     USE_MAX_TB),
             Q_ENTRY(NEG_MIN_TB,POS_MAX_TB) = ELIGIBLE( SHED_PRIO_0, PROP_PRIO_0 , NOT_USE_MIN_TB, USE_MAX_TB),
             Q_ENTRY(NEG_MIN_TB,NEG_MAX_TB) = NOT_ELIGIBLE)
 /*minimal BW with shaping*/
    /*0*/GUARANTEED_BW_WITH_SHAPING_ELIG_FUNC(PRV_SCHED_ELIG_Q_SHP_SCHED00_PROP00)
    /*1*/GUARANTEED_BW_WITH_SHAPING_ELIG_FUNC(PRV_SCHED_ELIG_Q_SHP_SCHED10_PROP10)
    /*2*/GUARANTEED_BW_WITH_SHAPING_ELIG_FUNC(PRV_SCHED_ELIG_Q_SHP_SCHED20_PROP20)
    /*3*/GUARANTEED_BW_WITH_SHAPING_ELIG_FUNC(PRV_SCHED_ELIG_Q_SHP_SCHED30_PROP30)
    /*4*/GUARANTEED_BW_WITH_SHAPING_ELIG_FUNC(PRV_SCHED_ELIG_Q_SHP_SCHED40_PROP40)
    /*5*/GUARANTEED_BW_WITH_SHAPING_ELIG_FUNC(PRV_SCHED_ELIG_Q_SHP_SCHED50_PROP50)
    /*6*/GUARANTEED_BW_WITH_SHAPING_ELIG_FUNC(PRV_SCHED_ELIG_Q_SHP_SCHED60_PROP60)
    /*7*/GUARANTEED_BW_WITH_SHAPING_ELIG_FUNC(PRV_SCHED_ELIG_Q_SHP_SCHED70_PROP70)

  if(0==bucketForShaping)
  {
     /*minimal BW no shaping*/
    /*0*/GUARANTEED_BW_NO_SHAPING_ELIG_FUNC(PRV_SCHED_ELIG_Q_SCHED00_PROP00)
    /*1*/GUARANTEED_BW_NO_SHAPING_ELIG_FUNC(PRV_SCHED_ELIG_Q_SCHED10_PROP10)
    /*2*/GUARANTEED_BW_NO_SHAPING_ELIG_FUNC(PRV_SCHED_ELIG_Q_SCHED20_PROP20)
    /*3*/GUARANTEED_BW_NO_SHAPING_ELIG_FUNC(PRV_SCHED_ELIG_Q_SCHED30_PROP30)
    /*4*/GUARANTEED_BW_NO_SHAPING_ELIG_FUNC(PRV_SCHED_ELIG_Q_SCHED40_PROP40)
    /*5*/GUARANTEED_BW_NO_SHAPING_ELIG_FUNC(PRV_SCHED_ELIG_Q_SCHED50_PROP50)
    /*6*/GUARANTEED_BW_NO_SHAPING_ELIG_FUNC(PRV_SCHED_ELIG_Q_SCHED60_PROP60)
    /*7*/GUARANTEED_BW_NO_SHAPING_ELIG_FUNC(PRV_SCHED_ELIG_Q_SCHED70_PROP70)
  }
  else
  {
     /*minimal BW no shaping use minimum backet*/
    /*0*/GUARANTEED_BW_NO_SHAPING_USE_MIN_BUCKET_ELIG_FUNC(PRV_SCHED_ELIG_Q_SCHED00_PROP00)
    /*1*/GUARANTEED_BW_NO_SHAPING_USE_MIN_BUCKET_ELIG_FUNC(PRV_SCHED_ELIG_Q_SCHED10_PROP10)
    /*2*/GUARANTEED_BW_NO_SHAPING_USE_MIN_BUCKET_ELIG_FUNC(PRV_SCHED_ELIG_Q_SCHED20_PROP20)
    /*3*/GUARANTEED_BW_NO_SHAPING_USE_MIN_BUCKET_ELIG_FUNC(PRV_SCHED_ELIG_Q_SCHED30_PROP30)
    /*4*/GUARANTEED_BW_NO_SHAPING_USE_MIN_BUCKET_ELIG_FUNC(PRV_SCHED_ELIG_Q_SCHED40_PROP40)
    /*5*/GUARANTEED_BW_NO_SHAPING_USE_MIN_BUCKET_ELIG_FUNC(PRV_SCHED_ELIG_Q_SCHED50_PROP50)
    /*6*/GUARANTEED_BW_NO_SHAPING_USE_MIN_BUCKET_ELIG_FUNC(PRV_SCHED_ELIG_Q_SCHED60_PROP60)
    /*7*/GUARANTEED_BW_NO_SHAPING_USE_MIN_BUCKET_ELIG_FUNC(PRV_SCHED_ELIG_Q_SCHED70_PROP70)
  }

    /* reserved   DeqDisable function - don't change*/
    QUEUE_DEQ_DISABLE(PRV_SCHED_ELIG_DEQ_DISABLE);
    /* reduced macro example - with assert : function Id < TM_ELIG_FUNC_TABLE_SIZE  failed */
    /* QUEUE_ELIG_FUN_FIXED_PRIORITY(65 ,FIX_PRIORITY(FIX_PRIO_7))  */
out:
    return rc;
}


/* nodes default EligibleFunction  table*/
/**
 */
int prvSchedInitDefaultNodeEligPrioFuncTable(struct tm_elig_prio_func_node *func_table)
{
    int i,rc=0;
    /* initialize memory */
    for (i=0; i < PRV_SCHED_ELIG_FUNC_TABLE_SIZE ; i++ ) NODE_DEQ_DISABLE(i);
    /*  setup default functions  */
    NODE_ELIG_FUNCTION( PRV_SCHED_ELIG_N_PRIO1,
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_0) = FIX_PRIORITY(FIX_PRIO_1),
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_1) = FIX_PRIORITY(FIX_PRIO_1),
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_2) = FIX_PRIORITY(FIX_PRIO_1),
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_3) = FIX_PRIORITY(FIX_PRIO_1),
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_4) = FIX_PRIORITY(FIX_PRIO_1),
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_5) = FIX_PRIORITY(FIX_PRIO_1),
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_6) = FIX_PRIORITY(FIX_PRIO_1),
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_7) = FIX_PRIORITY(FIX_PRIO_1),
            /*  ---------------------------------- */
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_0) = FIX_PRIORITY(FIX_PRIO_1),
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_1) = FIX_PRIORITY(FIX_PRIO_1),
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_2) = FIX_PRIORITY(FIX_PRIO_1),
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_3) = FIX_PRIORITY(FIX_PRIO_1),
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_4) = FIX_PRIORITY(FIX_PRIO_1),
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_5) = FIX_PRIORITY(FIX_PRIO_1),
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_6) = FIX_PRIORITY(FIX_PRIO_1),
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_7) = FIX_PRIORITY(FIX_PRIO_1),
            /*  ---------------------------------- */
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_0) = FIX_PRIORITY(FIX_PRIO_1),
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_1) = FIX_PRIORITY(FIX_PRIO_1),
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_2) = FIX_PRIORITY(FIX_PRIO_1),
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_3) = FIX_PRIORITY(FIX_PRIO_1),
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_4) = FIX_PRIORITY(FIX_PRIO_1),
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_5) = FIX_PRIORITY(FIX_PRIO_1),
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_6) = FIX_PRIORITY(FIX_PRIO_1),
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_7) = FIX_PRIORITY(FIX_PRIO_1),
            /*  ---------------------------------- */
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_0) = FIX_PRIORITY(FIX_PRIO_1),
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_1) = FIX_PRIORITY(FIX_PRIO_1),
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_2) = FIX_PRIORITY(FIX_PRIO_1),
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_3) = FIX_PRIORITY(FIX_PRIO_1),
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_4) = FIX_PRIORITY(FIX_PRIO_1),
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_5) = FIX_PRIORITY(FIX_PRIO_1),
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_6) = FIX_PRIORITY(FIX_PRIO_1),
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_7) = FIX_PRIORITY(FIX_PRIO_1))

    NODE_ELIG_FUNCTION( PRV_SCHED_ELIG_N_PRIO5,
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_0) = FIX_PRIORITY(FIX_PRIO_5),
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_1) = FIX_PRIORITY(FIX_PRIO_5),
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_2) = FIX_PRIORITY(FIX_PRIO_5),
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_3) = FIX_PRIORITY(FIX_PRIO_5),
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_4) = FIX_PRIORITY(FIX_PRIO_5),
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_5) = FIX_PRIORITY(FIX_PRIO_5),
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_6) = FIX_PRIORITY(FIX_PRIO_5),
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_7) = FIX_PRIORITY(FIX_PRIO_5),
            /*  ---------------------------------- */
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_0) = FIX_PRIORITY(FIX_PRIO_5),
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_1) = FIX_PRIORITY(FIX_PRIO_5),
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_2) = FIX_PRIORITY(FIX_PRIO_5),
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_3) = FIX_PRIORITY(FIX_PRIO_5),
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_4) = FIX_PRIORITY(FIX_PRIO_5),
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_5) = FIX_PRIORITY(FIX_PRIO_5),
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_6) = FIX_PRIORITY(FIX_PRIO_5),
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_7) = FIX_PRIORITY(FIX_PRIO_5),
            /*  ---------------------------------- */
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_0) = FIX_PRIORITY(FIX_PRIO_5),
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_1) = FIX_PRIORITY(FIX_PRIO_5),
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_2) = FIX_PRIORITY(FIX_PRIO_5),
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_3) = FIX_PRIORITY(FIX_PRIO_5),
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_4) = FIX_PRIORITY(FIX_PRIO_5),
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_5) = FIX_PRIORITY(FIX_PRIO_5),
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_6) = FIX_PRIORITY(FIX_PRIO_5),
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_7) = FIX_PRIORITY(FIX_PRIO_5),
            /*  ---------------------------------- */
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_0) = FIX_PRIORITY(FIX_PRIO_5),
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_1) = FIX_PRIORITY(FIX_PRIO_5),
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_2) = FIX_PRIORITY(FIX_PRIO_5),
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_3) = FIX_PRIORITY(FIX_PRIO_5),
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_4) = FIX_PRIORITY(FIX_PRIO_5),
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_5) = FIX_PRIORITY(FIX_PRIO_5),
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_6) = FIX_PRIORITY(FIX_PRIO_5),
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_7) = FIX_PRIORITY(FIX_PRIO_5))


    NODE_ELIG_FUNCTION( PRV_SCHED_ELIG_N_SHP,
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_0) = ELIGIBLE( SHED_PRIO_1, PROP_PRIO_0 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_1) = ELIGIBLE( SHED_PRIO_1, PROP_PRIO_0 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_2) = ELIGIBLE( SHED_PRIO_1, PROP_PRIO_0 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_3) = ELIGIBLE( SHED_PRIO_1, PROP_PRIO_0 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_4) = ELIGIBLE( SHED_PRIO_1, PROP_PRIO_0 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_5) = ELIGIBLE( SHED_PRIO_1, PROP_PRIO_0 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_6) = ELIGIBLE( SHED_PRIO_1, PROP_PRIO_0 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_7) = ELIGIBLE( SHED_PRIO_1, PROP_PRIO_0 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            /*  ---------------------------------- */
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_0) = ELIGIBLE( SHED_PRIO_1, PROP_PRIO_0 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_1) = ELIGIBLE( SHED_PRIO_1, PROP_PRIO_0 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_2) = ELIGIBLE( SHED_PRIO_1, PROP_PRIO_0 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_3) = ELIGIBLE( SHED_PRIO_1, PROP_PRIO_0 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_4) = ELIGIBLE( SHED_PRIO_1, PROP_PRIO_0 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_5) = ELIGIBLE( SHED_PRIO_1, PROP_PRIO_0 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_6) = ELIGIBLE( SHED_PRIO_1, PROP_PRIO_0 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_7) = ELIGIBLE( SHED_PRIO_1, PROP_PRIO_0 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            /*  ---------------------------------- */
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_0) = ELIGIBLE( SHED_PRIO_0, PROP_PRIO_0 , NOT_USE_MIN_TB,     USE_MAX_TB),
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_1) = ELIGIBLE( SHED_PRIO_0, PROP_PRIO_0 , NOT_USE_MIN_TB,     USE_MAX_TB),
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_2) = ELIGIBLE( SHED_PRIO_0, PROP_PRIO_0 , NOT_USE_MIN_TB,     USE_MAX_TB),
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_3) = ELIGIBLE( SHED_PRIO_0, PROP_PRIO_0 , NOT_USE_MIN_TB,     USE_MAX_TB),
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_4) = ELIGIBLE( SHED_PRIO_0, PROP_PRIO_0 , NOT_USE_MIN_TB,     USE_MAX_TB),
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_5) = ELIGIBLE( SHED_PRIO_0, PROP_PRIO_0 , NOT_USE_MIN_TB,     USE_MAX_TB),
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_6) = ELIGIBLE( SHED_PRIO_0, PROP_PRIO_0 , NOT_USE_MIN_TB,     USE_MAX_TB),
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_7) = ELIGIBLE( SHED_PRIO_0, PROP_PRIO_0 , NOT_USE_MIN_TB,     USE_MAX_TB),
            /*  ---------------------------------- */
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_0) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_1) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_2) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_3) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_4) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_5) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_6) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_7) = NOT_ELIGIBLE)

    NODE_ELIG_FUNCTION( PRV_SCHED_ELIG_N_MIN_SHP,
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_0) = ELIGIBLE( SHED_PRIO_5, PROP_PRIO_0 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_1) = ELIGIBLE( SHED_PRIO_5, PROP_PRIO_0 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_2) = ELIGIBLE( SHED_PRIO_5, PROP_PRIO_0 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_3) = ELIGIBLE( SHED_PRIO_5, PROP_PRIO_0 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_4) = ELIGIBLE( SHED_PRIO_5, PROP_PRIO_0 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_5) = ELIGIBLE( SHED_PRIO_5, PROP_PRIO_0 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_6) = ELIGIBLE( SHED_PRIO_5, PROP_PRIO_0 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_7) = ELIGIBLE( SHED_PRIO_5, PROP_PRIO_0 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            /*  ---------------------------------- */
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_0) = ELIGIBLE( SHED_PRIO_5, PROP_PRIO_0 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_1) = ELIGIBLE( SHED_PRIO_5, PROP_PRIO_0 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_2) = ELIGIBLE( SHED_PRIO_5, PROP_PRIO_0 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_3) = ELIGIBLE( SHED_PRIO_5, PROP_PRIO_0 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_4) = ELIGIBLE( SHED_PRIO_5, PROP_PRIO_0 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_5) = ELIGIBLE( SHED_PRIO_5, PROP_PRIO_0 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_6) = ELIGIBLE( SHED_PRIO_5, PROP_PRIO_0 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_7) = ELIGIBLE( SHED_PRIO_5, PROP_PRIO_0 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            /*  ---------------------------------- */
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_0) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_1) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_2) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_3) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_4) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_5) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_6) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_7) = NOT_ELIGIBLE,
            /*  ---------------------------------- */
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_0) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_1) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_2) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_3) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_4) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_5) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_6) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_7) = NOT_ELIGIBLE)

    NODE_ELIG_FUNCTION( PRV_SCHED_ELIG_N_PPA,
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_0) = ELIGIBLE( SHED_PRIO_1, PROP_PRIO_0 , NOT_USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_1) = ELIGIBLE( SHED_PRIO_2, PROP_PRIO_1 , NOT_USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_2) = ELIGIBLE( SHED_PRIO_3, PROP_PRIO_2 , NOT_USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_3) = ELIGIBLE( SHED_PRIO_4, PROP_PRIO_3 , NOT_USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_4) = NOT_ELIGIBLE,
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_5) = NOT_ELIGIBLE,
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_6) = NOT_ELIGIBLE,
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_7) = NOT_ELIGIBLE,
            /*  ---------------------------------- */
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_0) = ELIGIBLE( SHED_PRIO_1, PROP_PRIO_0 , NOT_USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_1) = ELIGIBLE( SHED_PRIO_2, PROP_PRIO_1 , NOT_USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_2) = ELIGIBLE( SHED_PRIO_3, PROP_PRIO_2 , NOT_USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_3) = ELIGIBLE( SHED_PRIO_4, PROP_PRIO_3 , NOT_USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_4) = NOT_ELIGIBLE,
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_5) = NOT_ELIGIBLE,
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_6) = NOT_ELIGIBLE,
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_7) = NOT_ELIGIBLE,
            /*  ---------------------------------- */
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_0) = ELIGIBLE( SHED_PRIO_1, PROP_PRIO_0 , NOT_USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_1) = ELIGIBLE( SHED_PRIO_2, PROP_PRIO_1 , NOT_USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_2) = ELIGIBLE( SHED_PRIO_3, PROP_PRIO_2 , NOT_USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_3) = ELIGIBLE( SHED_PRIO_4, PROP_PRIO_3 , NOT_USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_4) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_5) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_6) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_7) = NOT_ELIGIBLE,
            /*  ---------------------------------- */
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_0) = ELIGIBLE( SHED_PRIO_1, PROP_PRIO_0 , NOT_USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_1) = ELIGIBLE( SHED_PRIO_2, PROP_PRIO_1 , NOT_USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_2) = ELIGIBLE( SHED_PRIO_3, PROP_PRIO_2 , NOT_USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_3) = ELIGIBLE( SHED_PRIO_4, PROP_PRIO_3 , NOT_USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_4) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_5) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_6) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_7) = NOT_ELIGIBLE)

    NODE_ELIG_FUNCTION( PRV_SCHED_ELIG_N_PPA_SP,
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_0) = ELIGIBLE( SHED_PRIO_5, PROP_PRIO_0 , NOT_USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_1) = ELIGIBLE( SHED_PRIO_5, PROP_PRIO_1 , NOT_USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_2) = ELIGIBLE( SHED_PRIO_5, PROP_PRIO_2 , NOT_USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_3) = ELIGIBLE( SHED_PRIO_5, PROP_PRIO_3 , NOT_USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_4) = NOT_ELIGIBLE,
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_5) = NOT_ELIGIBLE,
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_6) = NOT_ELIGIBLE,
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_7) = NOT_ELIGIBLE,
            /*  ---------------------------------- */
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_0) = ELIGIBLE( SHED_PRIO_5, PROP_PRIO_0 , NOT_USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_1) = ELIGIBLE( SHED_PRIO_5, PROP_PRIO_1 , NOT_USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_2) = ELIGIBLE( SHED_PRIO_5, PROP_PRIO_2 , NOT_USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_3) = ELIGIBLE( SHED_PRIO_5, PROP_PRIO_3 , NOT_USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_4) = NOT_ELIGIBLE,
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_5) = NOT_ELIGIBLE,
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_6) = NOT_ELIGIBLE,
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_7) = NOT_ELIGIBLE,
            /*  ---------------------------------- */
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_0) = ELIGIBLE( SHED_PRIO_5, PROP_PRIO_0 , NOT_USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_1) = ELIGIBLE( SHED_PRIO_5, PROP_PRIO_1 , NOT_USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_2) = ELIGIBLE( SHED_PRIO_5, PROP_PRIO_2 , NOT_USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_3) = ELIGIBLE( SHED_PRIO_5, PROP_PRIO_3 , NOT_USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_4) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_5) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_6) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_7) = NOT_ELIGIBLE,
            /*  ---------------------------------- */
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_0) = ELIGIBLE( SHED_PRIO_5, PROP_PRIO_0 , NOT_USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_1) = ELIGIBLE( SHED_PRIO_5, PROP_PRIO_1 , NOT_USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_2) = ELIGIBLE( SHED_PRIO_5, PROP_PRIO_2 , NOT_USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_3) = ELIGIBLE( SHED_PRIO_5, PROP_PRIO_3 , NOT_USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_4) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_5) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_6) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_7) = NOT_ELIGIBLE)

    NODE_ELIG_FUNCTION( PRV_SCHED_ELIG_N_PPA_SHP,
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_0) = ELIGIBLE( SHED_PRIO_1, PROP_PRIO_0 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_1) = ELIGIBLE( SHED_PRIO_2, PROP_PRIO_1 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_2) = ELIGIBLE( SHED_PRIO_3, PROP_PRIO_2 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_3) = ELIGIBLE( SHED_PRIO_4, PROP_PRIO_3 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_4) = NOT_ELIGIBLE,
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_5) = NOT_ELIGIBLE,
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_6) = NOT_ELIGIBLE,
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_7) = NOT_ELIGIBLE,
            /*  ---------------------------------- */
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_0) = ELIGIBLE( SHED_PRIO_1, PROP_PRIO_0 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_1) = ELIGIBLE( SHED_PRIO_2, PROP_PRIO_1 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_2) = ELIGIBLE( SHED_PRIO_3, PROP_PRIO_2 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_3) = ELIGIBLE( SHED_PRIO_4, PROP_PRIO_3 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_4) = NOT_ELIGIBLE,
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_5) = NOT_ELIGIBLE,
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_6) = NOT_ELIGIBLE,
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_7) = NOT_ELIGIBLE,
            /*  ---------------------------------- */
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_0) = ELIGIBLE( SHED_PRIO_0, PROP_PRIO_0 , NOT_USE_MIN_TB,     USE_MAX_TB),
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_1) = ELIGIBLE( SHED_PRIO_0, PROP_PRIO_0 , NOT_USE_MIN_TB,     USE_MAX_TB),
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_2) = ELIGIBLE( SHED_PRIO_0, PROP_PRIO_0 , NOT_USE_MIN_TB,     USE_MAX_TB),
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_3) = ELIGIBLE( SHED_PRIO_0, PROP_PRIO_0 , NOT_USE_MIN_TB,     USE_MAX_TB),
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_4) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_5) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_6) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_7) = NOT_ELIGIBLE,
            /*  ---------------------------------- */
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_0) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_1) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_2) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_3) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_4) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_5) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_6) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_7) = NOT_ELIGIBLE)

    NODE_ELIG_FUNCTION( PRV_SCHED_ELIG_N_PPA_SP_MIN_SHP,
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_0) = ELIGIBLE( SHED_PRIO_5, PROP_PRIO_0 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_1) = ELIGIBLE( SHED_PRIO_5, PROP_PRIO_1 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_2) = ELIGIBLE( SHED_PRIO_5, PROP_PRIO_2 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_3) = ELIGIBLE( SHED_PRIO_5, PROP_PRIO_3 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_4) = NOT_ELIGIBLE,
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_5) = NOT_ELIGIBLE,
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_6) = NOT_ELIGIBLE,
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_7) = NOT_ELIGIBLE,
            /*  ---------------------------------- */
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_0) = ELIGIBLE( SHED_PRIO_5, PROP_PRIO_0 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_1) = ELIGIBLE( SHED_PRIO_5, PROP_PRIO_1 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_2) = ELIGIBLE( SHED_PRIO_5, PROP_PRIO_2 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_3) = ELIGIBLE( SHED_PRIO_5, PROP_PRIO_3 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_4) = NOT_ELIGIBLE,
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_5) = NOT_ELIGIBLE,
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_6) = NOT_ELIGIBLE,
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_7) = NOT_ELIGIBLE,
            /*  ---------------------------------- */
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_0) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_1) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_2) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_3) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_4) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_5) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_6) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_7) = NOT_ELIGIBLE,
            /*  ---------------------------------- */
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_0) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_1) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_2) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_3) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_4) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_5) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_6) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_7) = NOT_ELIGIBLE)

    NODE_ELIG_FUNCTION( PRV_SCHED_ELIG_N_PPA_SHP_IGN,
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_0) = ELIGIBLE( SHED_PRIO_1, PROP_PRIO_0 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_1) = ELIGIBLE( SHED_PRIO_2, PROP_PRIO_1 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_2) = ELIGIBLE( SHED_PRIO_3, PROP_PRIO_2 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_3) = ELIGIBLE( SHED_PRIO_4, PROP_PRIO_3 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_4) = NOT_ELIGIBLE,
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_5) = NOT_ELIGIBLE,
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_6) = NOT_ELIGIBLE,
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_7) = NOT_ELIGIBLE,
            /*  ---------------------------------- */
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_0) = ELIGIBLE( SHED_PRIO_1, PROP_PRIO_0 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_1) = ELIGIBLE( SHED_PRIO_2, PROP_PRIO_1 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_2) = ELIGIBLE( SHED_PRIO_3, PROP_PRIO_2 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_3) = ELIGIBLE( SHED_PRIO_4, PROP_PRIO_3 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_4) = NOT_ELIGIBLE,
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_5) = NOT_ELIGIBLE,
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_6) = NOT_ELIGIBLE,
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_7) = NOT_ELIGIBLE,
            /*  ---------------------------------- */
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_0) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_1) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_2) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_3) = ELIGIBLE( SHED_PRIO_4, PROP_PRIO_3 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_4) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_5) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_6) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_7) = NOT_ELIGIBLE,
            /*  ---------------------------------- */
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_0) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_1) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_2) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_3) = ELIGIBLE( SHED_PRIO_4, PROP_PRIO_3 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_4) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_5) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_6) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_7) = NOT_ELIGIBLE)

    NODE_ELIG_FUNCTION( PRV_SCHED_ELIG_N_PPA_MIN_SHP_SP_IGN,
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_0) = ELIGIBLE( SHED_PRIO_5, PROP_PRIO_0 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_1) = ELIGIBLE( SHED_PRIO_5, PROP_PRIO_1 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_2) = ELIGIBLE( SHED_PRIO_5, PROP_PRIO_2 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_3) = ELIGIBLE( SHED_PRIO_5, PROP_PRIO_3 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_4) = NOT_ELIGIBLE,
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_5) = NOT_ELIGIBLE,
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_6) = NOT_ELIGIBLE,
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_7) = NOT_ELIGIBLE,
            /*  ---------------------------------- */
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_0) = ELIGIBLE( SHED_PRIO_5, PROP_PRIO_0 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_1) = ELIGIBLE( SHED_PRIO_5, PROP_PRIO_1 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_2) = ELIGIBLE( SHED_PRIO_5, PROP_PRIO_2 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_3) = ELIGIBLE( SHED_PRIO_5, PROP_PRIO_3 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_4) = NOT_ELIGIBLE,
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_5) = NOT_ELIGIBLE,
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_6) = NOT_ELIGIBLE,
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_7) = NOT_ELIGIBLE,
            /*  ---------------------------------- */
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_0) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_1) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_2) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_3) = ELIGIBLE( SHED_PRIO_5, PROP_PRIO_3 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_4) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_5) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_6) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_7) = NOT_ELIGIBLE,
            /*  ---------------------------------- */
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_0) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_1) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_2) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_3) = ELIGIBLE( SHED_PRIO_5, PROP_PRIO_3 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_4) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_5) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_6) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_7) = NOT_ELIGIBLE)

    NODE_ELIG_FUNCTION( PRV_SCHED_ELIG_N_FP0,
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_0) = FIX_PRIORITY(FIX_PRIO_0),
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_1) = FIX_PRIORITY(FIX_PRIO_0),
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_2) = FIX_PRIORITY(FIX_PRIO_0),
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_3) = FIX_PRIORITY(FIX_PRIO_0),
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_4) = FIX_PRIORITY(FIX_PRIO_0),
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_5) = FIX_PRIORITY(FIX_PRIO_0),
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_6) = FIX_PRIORITY(FIX_PRIO_0),
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_7) = FIX_PRIORITY(FIX_PRIO_0),
            /*  ---------------------------------- */
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_0) = FIX_PRIORITY(FIX_PRIO_0),
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_1) = FIX_PRIORITY(FIX_PRIO_0),
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_2) = FIX_PRIORITY(FIX_PRIO_0),
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_3) = FIX_PRIORITY(FIX_PRIO_0),
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_4) = FIX_PRIORITY(FIX_PRIO_0),
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_5) = FIX_PRIORITY(FIX_PRIO_0),
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_6) = FIX_PRIORITY(FIX_PRIO_0),
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_7) = FIX_PRIORITY(FIX_PRIO_0),
            /*  ---------------------------------- */
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_0) = FIX_PRIORITY(FIX_PRIO_0),
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_1) = FIX_PRIORITY(FIX_PRIO_0),
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_2) = FIX_PRIORITY(FIX_PRIO_0),
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_3) = FIX_PRIORITY(FIX_PRIO_0),
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_4) = FIX_PRIORITY(FIX_PRIO_0),
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_5) = FIX_PRIORITY(FIX_PRIO_0),
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_6) = FIX_PRIORITY(FIX_PRIO_0),
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_7) = FIX_PRIORITY(FIX_PRIO_0),
            /*  ---------------------------------- */
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_0) = FIX_PRIORITY(FIX_PRIO_0),
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_1) = FIX_PRIORITY(FIX_PRIO_0),
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_2) = FIX_PRIORITY(FIX_PRIO_0),
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_3) = FIX_PRIORITY(FIX_PRIO_0),
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_4) = FIX_PRIORITY(FIX_PRIO_0),
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_5) = FIX_PRIORITY(FIX_PRIO_0),
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_6) = FIX_PRIORITY(FIX_PRIO_0),
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_7) = FIX_PRIORITY(FIX_PRIO_0))

    NODE_ELIG_FUNCTION( PRV_SCHED_ELIG_N_FP1,
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_0) = FIX_PRIORITY(FIX_PRIO_1),
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_1) = FIX_PRIORITY(FIX_PRIO_1),
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_2) = FIX_PRIORITY(FIX_PRIO_1),
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_3) = FIX_PRIORITY(FIX_PRIO_1),
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_4) = FIX_PRIORITY(FIX_PRIO_1),
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_5) = FIX_PRIORITY(FIX_PRIO_1),
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_6) = FIX_PRIORITY(FIX_PRIO_1),
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_7) = FIX_PRIORITY(FIX_PRIO_1),
            /*  ---------------------------------- */
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_0) = FIX_PRIORITY(FIX_PRIO_1),
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_1) = FIX_PRIORITY(FIX_PRIO_1),
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_2) = FIX_PRIORITY(FIX_PRIO_1),
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_3) = FIX_PRIORITY(FIX_PRIO_1),
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_4) = FIX_PRIORITY(FIX_PRIO_1),
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_5) = FIX_PRIORITY(FIX_PRIO_1),
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_6) = FIX_PRIORITY(FIX_PRIO_1),
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_7) = FIX_PRIORITY(FIX_PRIO_1),
            /*  ---------------------------------- */
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_0) = FIX_PRIORITY(FIX_PRIO_1),
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_1) = FIX_PRIORITY(FIX_PRIO_1),
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_2) = FIX_PRIORITY(FIX_PRIO_1),
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_3) = FIX_PRIORITY(FIX_PRIO_1),
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_4) = FIX_PRIORITY(FIX_PRIO_1),
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_5) = FIX_PRIORITY(FIX_PRIO_1),
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_6) = FIX_PRIORITY(FIX_PRIO_1),
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_7) = FIX_PRIORITY(FIX_PRIO_1),
            /*  ---------------------------------- */
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_0) = FIX_PRIORITY(FIX_PRIO_1),
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_1) = FIX_PRIORITY(FIX_PRIO_1),
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_2) = FIX_PRIORITY(FIX_PRIO_1),
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_3) = FIX_PRIORITY(FIX_PRIO_1),
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_4) = FIX_PRIORITY(FIX_PRIO_1),
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_5) = FIX_PRIORITY(FIX_PRIO_1),
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_6) = FIX_PRIORITY(FIX_PRIO_1),
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_7) = FIX_PRIORITY(FIX_PRIO_1))

    NODE_ELIG_FUNCTION( PRV_SCHED_ELIG_N_FP2,
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_0) = FIX_PRIORITY(FIX_PRIO_2),
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_1) = FIX_PRIORITY(FIX_PRIO_2),
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_2) = FIX_PRIORITY(FIX_PRIO_2),
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_3) = FIX_PRIORITY(FIX_PRIO_2),
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_4) = FIX_PRIORITY(FIX_PRIO_2),
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_5) = FIX_PRIORITY(FIX_PRIO_2),
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_6) = FIX_PRIORITY(FIX_PRIO_2),
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_7) = FIX_PRIORITY(FIX_PRIO_2),
            /*  ---------------------------------- */
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_0) = FIX_PRIORITY(FIX_PRIO_2),
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_1) = FIX_PRIORITY(FIX_PRIO_2),
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_2) = FIX_PRIORITY(FIX_PRIO_2),
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_3) = FIX_PRIORITY(FIX_PRIO_2),
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_4) = FIX_PRIORITY(FIX_PRIO_2),
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_5) = FIX_PRIORITY(FIX_PRIO_2),
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_6) = FIX_PRIORITY(FIX_PRIO_2),
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_7) = FIX_PRIORITY(FIX_PRIO_2),
            /*  ---------------------------------- */
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_0) = FIX_PRIORITY(FIX_PRIO_2),
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_1) = FIX_PRIORITY(FIX_PRIO_2),
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_2) = FIX_PRIORITY(FIX_PRIO_2),
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_3) = FIX_PRIORITY(FIX_PRIO_2),
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_4) = FIX_PRIORITY(FIX_PRIO_2),
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_5) = FIX_PRIORITY(FIX_PRIO_2),
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_6) = FIX_PRIORITY(FIX_PRIO_2),
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_7) = FIX_PRIORITY(FIX_PRIO_2),
            /*  ---------------------------------- */
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_0) = FIX_PRIORITY(FIX_PRIO_2),
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_1) = FIX_PRIORITY(FIX_PRIO_2),
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_2) = FIX_PRIORITY(FIX_PRIO_2),
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_3) = FIX_PRIORITY(FIX_PRIO_2),
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_4) = FIX_PRIORITY(FIX_PRIO_2),
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_5) = FIX_PRIORITY(FIX_PRIO_2),
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_6) = FIX_PRIORITY(FIX_PRIO_2),
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_7) = FIX_PRIORITY(FIX_PRIO_2))

    NODE_ELIG_FUNCTION( PRV_SCHED_ELIG_N_FP3,
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_0) = FIX_PRIORITY(FIX_PRIO_3),
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_1) = FIX_PRIORITY(FIX_PRIO_3),
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_2) = FIX_PRIORITY(FIX_PRIO_3),
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_3) = FIX_PRIORITY(FIX_PRIO_3),
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_4) = FIX_PRIORITY(FIX_PRIO_3),
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_5) = FIX_PRIORITY(FIX_PRIO_3),
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_6) = FIX_PRIORITY(FIX_PRIO_3),
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_7) = FIX_PRIORITY(FIX_PRIO_3),
            /*  ---------------------------------- */
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_0) = FIX_PRIORITY(FIX_PRIO_3),
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_1) = FIX_PRIORITY(FIX_PRIO_3),
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_2) = FIX_PRIORITY(FIX_PRIO_3),
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_3) = FIX_PRIORITY(FIX_PRIO_3),
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_4) = FIX_PRIORITY(FIX_PRIO_3),
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_5) = FIX_PRIORITY(FIX_PRIO_3),
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_6) = FIX_PRIORITY(FIX_PRIO_3),
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_7) = FIX_PRIORITY(FIX_PRIO_3),
            /*  ---------------------------------- */
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_0) = FIX_PRIORITY(FIX_PRIO_3),
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_1) = FIX_PRIORITY(FIX_PRIO_3),
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_2) = FIX_PRIORITY(FIX_PRIO_3),
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_3) = FIX_PRIORITY(FIX_PRIO_3),
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_4) = FIX_PRIORITY(FIX_PRIO_3),
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_5) = FIX_PRIORITY(FIX_PRIO_3),
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_6) = FIX_PRIORITY(FIX_PRIO_3),
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_7) = FIX_PRIORITY(FIX_PRIO_3),
            /*  ---------------------------------- */
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_0) = FIX_PRIORITY(FIX_PRIO_3),
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_1) = FIX_PRIORITY(FIX_PRIO_3),
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_2) = FIX_PRIORITY(FIX_PRIO_3),
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_3) = FIX_PRIORITY(FIX_PRIO_3),
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_4) = FIX_PRIORITY(FIX_PRIO_3),
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_5) = FIX_PRIORITY(FIX_PRIO_3),
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_6) = FIX_PRIORITY(FIX_PRIO_3),
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_7) = FIX_PRIORITY(FIX_PRIO_3))

    NODE_ELIG_FUNCTION( PRV_SCHED_ELIG_N_FP4,
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_0) = FIX_PRIORITY(FIX_PRIO_4),
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_1) = FIX_PRIORITY(FIX_PRIO_4),
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_2) = FIX_PRIORITY(FIX_PRIO_4),
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_3) = FIX_PRIORITY(FIX_PRIO_4),
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_4) = FIX_PRIORITY(FIX_PRIO_4),
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_5) = FIX_PRIORITY(FIX_PRIO_4),
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_6) = FIX_PRIORITY(FIX_PRIO_4),
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_7) = FIX_PRIORITY(FIX_PRIO_4),
            /*  ---------------------------------- */
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_0) = FIX_PRIORITY(FIX_PRIO_4),
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_1) = FIX_PRIORITY(FIX_PRIO_4),
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_2) = FIX_PRIORITY(FIX_PRIO_4),
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_3) = FIX_PRIORITY(FIX_PRIO_4),
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_4) = FIX_PRIORITY(FIX_PRIO_4),
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_5) = FIX_PRIORITY(FIX_PRIO_4),
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_6) = FIX_PRIORITY(FIX_PRIO_4),
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_7) = FIX_PRIORITY(FIX_PRIO_4),
            /*  ---------------------------------- */
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_0) = FIX_PRIORITY(FIX_PRIO_4),
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_1) = FIX_PRIORITY(FIX_PRIO_4),
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_2) = FIX_PRIORITY(FIX_PRIO_4),
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_3) = FIX_PRIORITY(FIX_PRIO_4),
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_4) = FIX_PRIORITY(FIX_PRIO_4),
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_5) = FIX_PRIORITY(FIX_PRIO_4),
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_6) = FIX_PRIORITY(FIX_PRIO_4),
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_7) = FIX_PRIORITY(FIX_PRIO_4),
            /*  ---------------------------------- */
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_0) = FIX_PRIORITY(FIX_PRIO_4),
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_1) = FIX_PRIORITY(FIX_PRIO_4),
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_2) = FIX_PRIORITY(FIX_PRIO_4),
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_3) = FIX_PRIORITY(FIX_PRIO_4),
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_4) = FIX_PRIORITY(FIX_PRIO_4),
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_5) = FIX_PRIORITY(FIX_PRIO_4),
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_6) = FIX_PRIORITY(FIX_PRIO_4),
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_7) = FIX_PRIORITY(FIX_PRIO_4))

    NODE_ELIG_FUNCTION( PRV_SCHED_ELIG_N_FP5,
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_0) = FIX_PRIORITY(FIX_PRIO_5),
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_1) = FIX_PRIORITY(FIX_PRIO_5),
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_2) = FIX_PRIORITY(FIX_PRIO_5),
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_3) = FIX_PRIORITY(FIX_PRIO_5),
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_4) = FIX_PRIORITY(FIX_PRIO_5),
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_5) = FIX_PRIORITY(FIX_PRIO_5),
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_6) = FIX_PRIORITY(FIX_PRIO_5),
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_7) = FIX_PRIORITY(FIX_PRIO_5),
            /*  ---------------------------------- */
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_0) = FIX_PRIORITY(FIX_PRIO_5),
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_1) = FIX_PRIORITY(FIX_PRIO_5),
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_2) = FIX_PRIORITY(FIX_PRIO_5),
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_3) = FIX_PRIORITY(FIX_PRIO_5),
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_4) = FIX_PRIORITY(FIX_PRIO_5),
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_5) = FIX_PRIORITY(FIX_PRIO_5),
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_6) = FIX_PRIORITY(FIX_PRIO_5),
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_7) = FIX_PRIORITY(FIX_PRIO_5),
            /*  ---------------------------------- */
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_0) = FIX_PRIORITY(FIX_PRIO_5),
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_1) = FIX_PRIORITY(FIX_PRIO_5),
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_2) = FIX_PRIORITY(FIX_PRIO_5),
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_3) = FIX_PRIORITY(FIX_PRIO_5),
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_4) = FIX_PRIORITY(FIX_PRIO_5),
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_5) = FIX_PRIORITY(FIX_PRIO_5),
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_6) = FIX_PRIORITY(FIX_PRIO_5),
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_7) = FIX_PRIORITY(FIX_PRIO_5),
            /*  ---------------------------------- */
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_0) = FIX_PRIORITY(FIX_PRIO_5),
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_1) = FIX_PRIORITY(FIX_PRIO_5),
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_2) = FIX_PRIORITY(FIX_PRIO_5),
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_3) = FIX_PRIORITY(FIX_PRIO_5),
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_4) = FIX_PRIORITY(FIX_PRIO_5),
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_5) = FIX_PRIORITY(FIX_PRIO_5),
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_6) = FIX_PRIORITY(FIX_PRIO_5),
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_7) = FIX_PRIORITY(FIX_PRIO_5))

    NODE_ELIG_FUNCTION( PRV_SCHED_ELIG_N_FP6,
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_0) = FIX_PRIORITY(FIX_PRIO_6),
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_1) = FIX_PRIORITY(FIX_PRIO_6),
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_2) = FIX_PRIORITY(FIX_PRIO_6),
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_3) = FIX_PRIORITY(FIX_PRIO_6),
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_4) = FIX_PRIORITY(FIX_PRIO_6),
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_5) = FIX_PRIORITY(FIX_PRIO_6),
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_6) = FIX_PRIORITY(FIX_PRIO_6),
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_7) = FIX_PRIORITY(FIX_PRIO_6),
            /*  ---------------------------------- */
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_0) = FIX_PRIORITY(FIX_PRIO_6),
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_1) = FIX_PRIORITY(FIX_PRIO_6),
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_2) = FIX_PRIORITY(FIX_PRIO_6),
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_3) = FIX_PRIORITY(FIX_PRIO_6),
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_4) = FIX_PRIORITY(FIX_PRIO_6),
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_5) = FIX_PRIORITY(FIX_PRIO_6),
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_6) = FIX_PRIORITY(FIX_PRIO_6),
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_7) = FIX_PRIORITY(FIX_PRIO_6),
            /*  ---------------------------------- */
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_0) = FIX_PRIORITY(FIX_PRIO_6),
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_1) = FIX_PRIORITY(FIX_PRIO_6),
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_2) = FIX_PRIORITY(FIX_PRIO_6),
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_3) = FIX_PRIORITY(FIX_PRIO_6),
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_4) = FIX_PRIORITY(FIX_PRIO_6),
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_5) = FIX_PRIORITY(FIX_PRIO_6),
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_6) = FIX_PRIORITY(FIX_PRIO_6),
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_7) = FIX_PRIORITY(FIX_PRIO_6),
            /*  ---------------------------------- */
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_0) = FIX_PRIORITY(FIX_PRIO_6),
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_1) = FIX_PRIORITY(FIX_PRIO_6),
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_2) = FIX_PRIORITY(FIX_PRIO_6),
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_3) = FIX_PRIORITY(FIX_PRIO_6),
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_4) = FIX_PRIORITY(FIX_PRIO_6),
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_5) = FIX_PRIORITY(FIX_PRIO_6),
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_6) = FIX_PRIORITY(FIX_PRIO_6),
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_7) = FIX_PRIORITY(FIX_PRIO_6))

    NODE_ELIG_FUNCTION( PRV_SCHED_ELIG_N_FP7,
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_0) = FIX_PRIORITY(FIX_PRIO_7),
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_1) = FIX_PRIORITY(FIX_PRIO_7),
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_2) = FIX_PRIORITY(FIX_PRIO_7),
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_3) = FIX_PRIORITY(FIX_PRIO_7),
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_4) = FIX_PRIORITY(FIX_PRIO_7),
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_5) = FIX_PRIORITY(FIX_PRIO_7),
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_6) = FIX_PRIORITY(FIX_PRIO_7),
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_7) = FIX_PRIORITY(FIX_PRIO_7),
            /*  ---------------------------------- */
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_0) = FIX_PRIORITY(FIX_PRIO_7),
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_1) = FIX_PRIORITY(FIX_PRIO_7),
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_2) = FIX_PRIORITY(FIX_PRIO_7),
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_3) = FIX_PRIORITY(FIX_PRIO_7),
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_4) = FIX_PRIORITY(FIX_PRIO_7),
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_5) = FIX_PRIORITY(FIX_PRIO_7),
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_6) = FIX_PRIORITY(FIX_PRIO_7),
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_7) = FIX_PRIORITY(FIX_PRIO_7),
            /*  ---------------------------------- */
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_0) = FIX_PRIORITY(FIX_PRIO_7),
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_1) = FIX_PRIORITY(FIX_PRIO_7),
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_2) = FIX_PRIORITY(FIX_PRIO_7),
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_3) = FIX_PRIORITY(FIX_PRIO_7),
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_4) = FIX_PRIORITY(FIX_PRIO_7),
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_5) = FIX_PRIORITY(FIX_PRIO_7),
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_6) = FIX_PRIORITY(FIX_PRIO_7),
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_7) = FIX_PRIORITY(FIX_PRIO_7),
            /*  ---------------------------------- */
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_0) = FIX_PRIORITY(FIX_PRIO_7),
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_1) = FIX_PRIORITY(FIX_PRIO_7),
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_2) = FIX_PRIORITY(FIX_PRIO_7),
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_3) = FIX_PRIORITY(FIX_PRIO_7),
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_4) = FIX_PRIORITY(FIX_PRIO_7),
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_5) = FIX_PRIORITY(FIX_PRIO_7),
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_6) = FIX_PRIORITY(FIX_PRIO_7),
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_7) = FIX_PRIORITY(FIX_PRIO_7))

    NODE_ELIG_FUNCTION( PRV_SCHED_ELIG_N_MIN_SHP_FP0,
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_0) = ELIGIBLE( SHED_PRIO_0, PROP_PRIO_0 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_1) = ELIGIBLE( SHED_PRIO_0, PROP_PRIO_0 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_2) = ELIGIBLE( SHED_PRIO_0, PROP_PRIO_0 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_3) = ELIGIBLE( SHED_PRIO_0, PROP_PRIO_0 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_4) = ELIGIBLE( SHED_PRIO_0, PROP_PRIO_0 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_5) = ELIGIBLE( SHED_PRIO_0, PROP_PRIO_0 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_6) = ELIGIBLE( SHED_PRIO_0, PROP_PRIO_0 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_7) = ELIGIBLE( SHED_PRIO_0, PROP_PRIO_0 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            /*  ---------------------------------- */
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_0) = ELIGIBLE( SHED_PRIO_0, PROP_PRIO_0 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_1) = ELIGIBLE( SHED_PRIO_0, PROP_PRIO_0 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_2) = ELIGIBLE( SHED_PRIO_0, PROP_PRIO_0 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_3) = ELIGIBLE( SHED_PRIO_0, PROP_PRIO_0 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_4) = ELIGIBLE( SHED_PRIO_0, PROP_PRIO_0 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_5) = ELIGIBLE( SHED_PRIO_0, PROP_PRIO_0 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_6) = ELIGIBLE( SHED_PRIO_0, PROP_PRIO_0 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_7) = ELIGIBLE( SHED_PRIO_0, PROP_PRIO_0 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            /*  ---------------------------------- */
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_0) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_1) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_2) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_3) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_4) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_5) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_6) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_7) = NOT_ELIGIBLE,
            /*  ---------------------------------- */
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_0) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_1) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_2) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_3) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_4) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_5) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_6) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_7) = NOT_ELIGIBLE)

    NODE_ELIG_FUNCTION( PRV_SCHED_ELIG_N_MIN_SHP_FP1,
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_0) = ELIGIBLE( SHED_PRIO_1, PROP_PRIO_1 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_1) = ELIGIBLE( SHED_PRIO_1, PROP_PRIO_1 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_2) = ELIGIBLE( SHED_PRIO_1, PROP_PRIO_1 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_3) = ELIGIBLE( SHED_PRIO_1, PROP_PRIO_1 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_4) = ELIGIBLE( SHED_PRIO_1, PROP_PRIO_1 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_5) = ELIGIBLE( SHED_PRIO_1, PROP_PRIO_1 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_6) = ELIGIBLE( SHED_PRIO_1, PROP_PRIO_1 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_7) = ELIGIBLE( SHED_PRIO_1, PROP_PRIO_1 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            /*  ---------------------------------- */
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_0) = ELIGIBLE( SHED_PRIO_1, PROP_PRIO_1 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_1) = ELIGIBLE( SHED_PRIO_1, PROP_PRIO_1 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_2) = ELIGIBLE( SHED_PRIO_1, PROP_PRIO_1 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_3) = ELIGIBLE( SHED_PRIO_1, PROP_PRIO_1 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_4) = ELIGIBLE( SHED_PRIO_1, PROP_PRIO_1 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_5) = ELIGIBLE( SHED_PRIO_1, PROP_PRIO_1 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_6) = ELIGIBLE( SHED_PRIO_1, PROP_PRIO_1 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_7) = ELIGIBLE( SHED_PRIO_1, PROP_PRIO_1 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            /*  ---------------------------------- */
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_0) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_1) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_2) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_3) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_4) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_5) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_6) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_7) = NOT_ELIGIBLE,
            /*  ---------------------------------- */
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_0) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_1) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_2) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_3) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_4) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_5) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_6) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_7) = NOT_ELIGIBLE)

    NODE_ELIG_FUNCTION( PRV_SCHED_ELIG_N_MIN_SHP_FP2,
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_0) = ELIGIBLE( SHED_PRIO_2, PROP_PRIO_2 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_1) = ELIGIBLE( SHED_PRIO_2, PROP_PRIO_2 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_2) = ELIGIBLE( SHED_PRIO_2, PROP_PRIO_2 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_3) = ELIGIBLE( SHED_PRIO_2, PROP_PRIO_2 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_4) = ELIGIBLE( SHED_PRIO_2, PROP_PRIO_2 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_5) = ELIGIBLE( SHED_PRIO_2, PROP_PRIO_2 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_6) = ELIGIBLE( SHED_PRIO_2, PROP_PRIO_2 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_7) = ELIGIBLE( SHED_PRIO_2, PROP_PRIO_2 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            /*  ---------------------------------- */
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_0) = ELIGIBLE( SHED_PRIO_2, PROP_PRIO_2 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_1) = ELIGIBLE( SHED_PRIO_2, PROP_PRIO_2 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_2) = ELIGIBLE( SHED_PRIO_2, PROP_PRIO_2 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_3) = ELIGIBLE( SHED_PRIO_2, PROP_PRIO_2 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_4) = ELIGIBLE( SHED_PRIO_2, PROP_PRIO_2 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_5) = ELIGIBLE( SHED_PRIO_2, PROP_PRIO_2 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_6) = ELIGIBLE( SHED_PRIO_2, PROP_PRIO_2 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_7) = ELIGIBLE( SHED_PRIO_2, PROP_PRIO_2 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            /*  ---------------------------------- */
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_0) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_1) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_2) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_3) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_4) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_5) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_6) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_7) = NOT_ELIGIBLE,
            /*  ---------------------------------- */
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_0) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_1) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_2) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_3) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_4) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_5) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_6) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_7) = NOT_ELIGIBLE)

    NODE_ELIG_FUNCTION( PRV_SCHED_ELIG_N_MIN_SHP_FP3,
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_0) = ELIGIBLE( SHED_PRIO_3, PROP_PRIO_3 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_1) = ELIGIBLE( SHED_PRIO_3, PROP_PRIO_3 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_2) = ELIGIBLE( SHED_PRIO_3, PROP_PRIO_3 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_3) = ELIGIBLE( SHED_PRIO_3, PROP_PRIO_3 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_4) = ELIGIBLE( SHED_PRIO_3, PROP_PRIO_3 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_5) = ELIGIBLE( SHED_PRIO_3, PROP_PRIO_3 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_6) = ELIGIBLE( SHED_PRIO_3, PROP_PRIO_3 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_7) = ELIGIBLE( SHED_PRIO_3, PROP_PRIO_3 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            /*  ---------------------------------- */
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_0) = ELIGIBLE( SHED_PRIO_3, PROP_PRIO_3 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_1) = ELIGIBLE( SHED_PRIO_3, PROP_PRIO_3 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_2) = ELIGIBLE( SHED_PRIO_3, PROP_PRIO_3 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_3) = ELIGIBLE( SHED_PRIO_3, PROP_PRIO_3 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_4) = ELIGIBLE( SHED_PRIO_3, PROP_PRIO_3 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_5) = ELIGIBLE( SHED_PRIO_3, PROP_PRIO_3 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_6) = ELIGIBLE( SHED_PRIO_3, PROP_PRIO_3 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_7) = ELIGIBLE( SHED_PRIO_3, PROP_PRIO_3 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            /*  ---------------------------------- */
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_0) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_1) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_2) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_3) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_4) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_5) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_6) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_7) = NOT_ELIGIBLE,
            /*  ---------------------------------- */
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_0) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_1) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_2) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_3) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_4) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_5) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_6) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_7) = NOT_ELIGIBLE)

    NODE_ELIG_FUNCTION( PRV_SCHED_ELIG_N_MIN_SHP_FP4,
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_0) = ELIGIBLE( SHED_PRIO_4, PROP_PRIO_4 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_1) = ELIGIBLE( SHED_PRIO_4, PROP_PRIO_4 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_2) = ELIGIBLE( SHED_PRIO_4, PROP_PRIO_4 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_3) = ELIGIBLE( SHED_PRIO_4, PROP_PRIO_4 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_4) = ELIGIBLE( SHED_PRIO_4, PROP_PRIO_4 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_5) = ELIGIBLE( SHED_PRIO_4, PROP_PRIO_4 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_6) = ELIGIBLE( SHED_PRIO_4, PROP_PRIO_4 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_7) = ELIGIBLE( SHED_PRIO_4, PROP_PRIO_4 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            /*  ---------------------------------- */
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_0) = ELIGIBLE( SHED_PRIO_4, PROP_PRIO_4 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_1) = ELIGIBLE( SHED_PRIO_4, PROP_PRIO_4 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_2) = ELIGIBLE( SHED_PRIO_4, PROP_PRIO_4 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_3) = ELIGIBLE( SHED_PRIO_4, PROP_PRIO_4 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_4) = ELIGIBLE( SHED_PRIO_4, PROP_PRIO_4 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_5) = ELIGIBLE( SHED_PRIO_4, PROP_PRIO_4 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_6) = ELIGIBLE( SHED_PRIO_4, PROP_PRIO_4 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_7) = ELIGIBLE( SHED_PRIO_4, PROP_PRIO_4 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            /*  ---------------------------------- */
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_0) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_1) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_2) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_3) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_4) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_5) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_6) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_7) = NOT_ELIGIBLE,
            /*  ---------------------------------- */
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_0) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_1) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_2) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_3) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_4) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_5) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_6) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_7) = NOT_ELIGIBLE)

    NODE_ELIG_FUNCTION( PRV_SCHED_ELIG_N_MIN_SHP_FP5,
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_0) = ELIGIBLE( SHED_PRIO_5, PROP_PRIO_5 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_1) = ELIGIBLE( SHED_PRIO_5, PROP_PRIO_5 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_2) = ELIGIBLE( SHED_PRIO_5, PROP_PRIO_5 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_3) = ELIGIBLE( SHED_PRIO_5, PROP_PRIO_5 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_4) = ELIGIBLE( SHED_PRIO_5, PROP_PRIO_5 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_5) = ELIGIBLE( SHED_PRIO_5, PROP_PRIO_5 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_6) = ELIGIBLE( SHED_PRIO_5, PROP_PRIO_5 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_7) = ELIGIBLE( SHED_PRIO_5, PROP_PRIO_5 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            /*  ---------------------------------- */
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_0) = ELIGIBLE( SHED_PRIO_5, PROP_PRIO_5 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_1) = ELIGIBLE( SHED_PRIO_5, PROP_PRIO_5 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_2) = ELIGIBLE( SHED_PRIO_5, PROP_PRIO_5 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_3) = ELIGIBLE( SHED_PRIO_5, PROP_PRIO_5 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_4) = ELIGIBLE( SHED_PRIO_5, PROP_PRIO_5 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_5) = ELIGIBLE( SHED_PRIO_5, PROP_PRIO_5 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_6) = ELIGIBLE( SHED_PRIO_5, PROP_PRIO_5 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_7) = ELIGIBLE( SHED_PRIO_5, PROP_PRIO_5 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            /*  ---------------------------------- */
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_0) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_1) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_2) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_3) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_4) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_5) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_6) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_7) = NOT_ELIGIBLE,
            /*  ---------------------------------- */
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_0) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_1) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_2) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_3) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_4) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_5) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_6) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_7) = NOT_ELIGIBLE)

    NODE_ELIG_FUNCTION( PRV_SCHED_ELIG_N_MIN_SHP_FP6,
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_0) = ELIGIBLE( SHED_PRIO_6, PROP_PRIO_6 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_1) = ELIGIBLE( SHED_PRIO_6, PROP_PRIO_6 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_2) = ELIGIBLE( SHED_PRIO_6, PROP_PRIO_6 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_3) = ELIGIBLE( SHED_PRIO_6, PROP_PRIO_6 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_4) = ELIGIBLE( SHED_PRIO_6, PROP_PRIO_6 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_5) = ELIGIBLE( SHED_PRIO_6, PROP_PRIO_6 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_6) = ELIGIBLE( SHED_PRIO_6, PROP_PRIO_6 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_7) = ELIGIBLE( SHED_PRIO_6, PROP_PRIO_6 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            /*  ---------------------------------- */
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_0) = ELIGIBLE( SHED_PRIO_6, PROP_PRIO_6 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_1) = ELIGIBLE( SHED_PRIO_6, PROP_PRIO_6 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_2) = ELIGIBLE( SHED_PRIO_6, PROP_PRIO_6 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_3) = ELIGIBLE( SHED_PRIO_6, PROP_PRIO_6 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_4) = ELIGIBLE( SHED_PRIO_6, PROP_PRIO_6 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_5) = ELIGIBLE( SHED_PRIO_6, PROP_PRIO_6 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_6) = ELIGIBLE( SHED_PRIO_6, PROP_PRIO_6 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_7) = ELIGIBLE( SHED_PRIO_6, PROP_PRIO_6 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            /*  ---------------------------------- */
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_0) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_1) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_2) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_3) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_4) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_5) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_6) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_7) = NOT_ELIGIBLE,
            /*  ---------------------------------- */
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_0) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_1) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_2) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_3) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_4) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_5) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_6) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_7) = NOT_ELIGIBLE)

    NODE_ELIG_FUNCTION( PRV_SCHED_ELIG_N_MIN_SHP_FP7,
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_0) = ELIGIBLE( SHED_PRIO_7, PROP_PRIO_7 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_1) = ELIGIBLE( SHED_PRIO_7, PROP_PRIO_7 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_2) = ELIGIBLE( SHED_PRIO_7, PROP_PRIO_7 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_3) = ELIGIBLE( SHED_PRIO_7, PROP_PRIO_7 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_4) = ELIGIBLE( SHED_PRIO_7, PROP_PRIO_7 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_5) = ELIGIBLE( SHED_PRIO_7, PROP_PRIO_7 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_6) = ELIGIBLE( SHED_PRIO_7, PROP_PRIO_7 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_7) = ELIGIBLE( SHED_PRIO_7, PROP_PRIO_7 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            /*  ---------------------------------- */
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_0) = ELIGIBLE( SHED_PRIO_7, PROP_PRIO_7 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_1) = ELIGIBLE( SHED_PRIO_7, PROP_PRIO_7 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_2) = ELIGIBLE( SHED_PRIO_7, PROP_PRIO_7 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_3) = ELIGIBLE( SHED_PRIO_7, PROP_PRIO_7 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_4) = ELIGIBLE( SHED_PRIO_7, PROP_PRIO_7 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_5) = ELIGIBLE( SHED_PRIO_7, PROP_PRIO_7 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_6) = ELIGIBLE( SHED_PRIO_7, PROP_PRIO_7 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_7) = ELIGIBLE( SHED_PRIO_7, PROP_PRIO_7 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            /*  ---------------------------------- */
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_0) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_1) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_2) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_3) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_4) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_5) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_6) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_7) = NOT_ELIGIBLE,
            /*  ---------------------------------- */
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_0) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_1) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_2) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_3) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_4) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_5) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_6) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_7) = NOT_ELIGIBLE)

    NODE_ELIG_FUNCTION( PRV_SCHED_ELIG_N_MAX_INC_MIN_SHP,
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_0) = ELIGIBLE( SHED_PRIO_1, PROP_PRIO_0 ,     USE_MIN_TB,     USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_1) = ELIGIBLE( SHED_PRIO_1, PROP_PRIO_0 ,     USE_MIN_TB,     USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_2) = ELIGIBLE( SHED_PRIO_1, PROP_PRIO_0 ,     USE_MIN_TB,     USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_3) = ELIGIBLE( SHED_PRIO_1, PROP_PRIO_0 ,     USE_MIN_TB,     USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_4) = ELIGIBLE( SHED_PRIO_1, PROP_PRIO_0 ,     USE_MIN_TB,     USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_5) = ELIGIBLE( SHED_PRIO_1, PROP_PRIO_0 ,     USE_MIN_TB,     USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_6) = ELIGIBLE( SHED_PRIO_1, PROP_PRIO_0 ,     USE_MIN_TB,     USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_7) = ELIGIBLE( SHED_PRIO_1, PROP_PRIO_0 ,     USE_MIN_TB,     USE_MAX_TB),
            /*  ---------------------------------- */
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_0) = ELIGIBLE( SHED_PRIO_1, PROP_PRIO_0 ,     USE_MIN_TB,     USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_1) = ELIGIBLE( SHED_PRIO_1, PROP_PRIO_0 ,     USE_MIN_TB,     USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_2) = ELIGIBLE( SHED_PRIO_1, PROP_PRIO_0 ,     USE_MIN_TB,     USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_3) = ELIGIBLE( SHED_PRIO_1, PROP_PRIO_0 ,     USE_MIN_TB,     USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_4) = ELIGIBLE( SHED_PRIO_1, PROP_PRIO_0 ,     USE_MIN_TB,     USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_5) = ELIGIBLE( SHED_PRIO_1, PROP_PRIO_0 ,     USE_MIN_TB,     USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_6) = ELIGIBLE( SHED_PRIO_1, PROP_PRIO_0 ,     USE_MIN_TB,     USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_7) = ELIGIBLE( SHED_PRIO_1, PROP_PRIO_0 ,     USE_MIN_TB,     USE_MAX_TB),
            /*  ---------------------------------- */
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_0) = ELIGIBLE( SHED_PRIO_0, PROP_PRIO_0 , NOT_USE_MIN_TB,     USE_MAX_TB),
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_1) = ELIGIBLE( SHED_PRIO_0, PROP_PRIO_0 , NOT_USE_MIN_TB,     USE_MAX_TB),
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_2) = ELIGIBLE( SHED_PRIO_0, PROP_PRIO_0 , NOT_USE_MIN_TB,     USE_MAX_TB),
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_3) = ELIGIBLE( SHED_PRIO_0, PROP_PRIO_0 , NOT_USE_MIN_TB,     USE_MAX_TB),
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_4) = ELIGIBLE( SHED_PRIO_0, PROP_PRIO_0 , NOT_USE_MIN_TB,     USE_MAX_TB),
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_5) = ELIGIBLE( SHED_PRIO_0, PROP_PRIO_0 , NOT_USE_MIN_TB,     USE_MAX_TB),
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_6) = ELIGIBLE( SHED_PRIO_0, PROP_PRIO_0 , NOT_USE_MIN_TB,     USE_MAX_TB),
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_7) = ELIGIBLE( SHED_PRIO_0, PROP_PRIO_0 , NOT_USE_MIN_TB,     USE_MAX_TB),
            /*  ---------------------------------- */
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_0) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_1) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_2) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_3) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_4) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_5) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_6) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_7) = NOT_ELIGIBLE)


    NODE_ELIG_FUNCTION( PRV_SCHED_ELIG_N_PP,
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_0) = ELIGIBLE( SHED_PRIO_0, PROP_PRIO_0 , NOT_USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_1) = ELIGIBLE( SHED_PRIO_1, PROP_PRIO_1 , NOT_USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_2) = ELIGIBLE( SHED_PRIO_2, PROP_PRIO_2 , NOT_USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_3) = ELIGIBLE( SHED_PRIO_3, PROP_PRIO_3 , NOT_USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_4) = ELIGIBLE( SHED_PRIO_4, PROP_PRIO_4 , NOT_USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_5) = ELIGIBLE( SHED_PRIO_5, PROP_PRIO_5 , NOT_USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_6) = ELIGIBLE( SHED_PRIO_6, PROP_PRIO_6 , NOT_USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_7) = ELIGIBLE( SHED_PRIO_7, PROP_PRIO_7 , NOT_USE_MIN_TB, NOT_USE_MAX_TB),
            /*  ---------------------------------- */
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_0) = ELIGIBLE( SHED_PRIO_0, PROP_PRIO_0 , NOT_USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_1) = ELIGIBLE( SHED_PRIO_1, PROP_PRIO_1 , NOT_USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_2) = ELIGIBLE( SHED_PRIO_2, PROP_PRIO_2 , NOT_USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_3) = ELIGIBLE( SHED_PRIO_3, PROP_PRIO_3 , NOT_USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_4) = ELIGIBLE( SHED_PRIO_4, PROP_PRIO_4 , NOT_USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_5) = ELIGIBLE( SHED_PRIO_5, PROP_PRIO_5 , NOT_USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_6) = ELIGIBLE( SHED_PRIO_6, PROP_PRIO_6 , NOT_USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_7) = ELIGIBLE( SHED_PRIO_7, PROP_PRIO_7 , NOT_USE_MIN_TB, NOT_USE_MAX_TB),
            /*  ---------------------------------- */
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_0) = ELIGIBLE( SHED_PRIO_0, PROP_PRIO_0 , NOT_USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_1) = ELIGIBLE( SHED_PRIO_1, PROP_PRIO_1 , NOT_USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_2) = ELIGIBLE( SHED_PRIO_2, PROP_PRIO_2 , NOT_USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_3) = ELIGIBLE( SHED_PRIO_3, PROP_PRIO_3 , NOT_USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_4) = ELIGIBLE( SHED_PRIO_4, PROP_PRIO_4 , NOT_USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_5) = ELIGIBLE( SHED_PRIO_5, PROP_PRIO_5 , NOT_USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_6) = ELIGIBLE( SHED_PRIO_6, PROP_PRIO_6 , NOT_USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_7) = ELIGIBLE( SHED_PRIO_7, PROP_PRIO_7 , NOT_USE_MIN_TB, NOT_USE_MAX_TB),
            /*  ---------------------------------- */
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_0) = ELIGIBLE( SHED_PRIO_0, PROP_PRIO_0 , NOT_USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_1) = ELIGIBLE( SHED_PRIO_1, PROP_PRIO_1 , NOT_USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_2) = ELIGIBLE( SHED_PRIO_2, PROP_PRIO_2 , NOT_USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_3) = ELIGIBLE( SHED_PRIO_3, PROP_PRIO_3 , NOT_USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_4) = ELIGIBLE( SHED_PRIO_4, PROP_PRIO_4 , NOT_USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_5) = ELIGIBLE( SHED_PRIO_5, PROP_PRIO_5 , NOT_USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_6) = ELIGIBLE( SHED_PRIO_6, PROP_PRIO_6 , NOT_USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_7) = ELIGIBLE( SHED_PRIO_7, PROP_PRIO_7 , NOT_USE_MIN_TB, NOT_USE_MAX_TB))

    NODE_ELIG_FUNCTION( PRV_SCHED_ELIG_N_MIN_SHP_PP,
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_0) = ELIGIBLE( SHED_PRIO_0, PROP_PRIO_0 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_1) = ELIGIBLE( SHED_PRIO_1, PROP_PRIO_1 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_2) = ELIGIBLE( SHED_PRIO_2, PROP_PRIO_2 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_3) = ELIGIBLE( SHED_PRIO_3, PROP_PRIO_3 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_4) = ELIGIBLE( SHED_PRIO_4, PROP_PRIO_4 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_5) = ELIGIBLE( SHED_PRIO_5, PROP_PRIO_5 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_6) = ELIGIBLE( SHED_PRIO_6, PROP_PRIO_6 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_7) = ELIGIBLE( SHED_PRIO_7, PROP_PRIO_7 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            /*  ---------------------------------- */
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_0) = ELIGIBLE( SHED_PRIO_0, PROP_PRIO_0 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_1) = ELIGIBLE( SHED_PRIO_1, PROP_PRIO_1 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_2) = ELIGIBLE( SHED_PRIO_2, PROP_PRIO_2 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_3) = ELIGIBLE( SHED_PRIO_3, PROP_PRIO_3 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_4) = ELIGIBLE( SHED_PRIO_4, PROP_PRIO_4 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_5) = ELIGIBLE( SHED_PRIO_5, PROP_PRIO_5 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_6) = ELIGIBLE( SHED_PRIO_6, PROP_PRIO_6 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_7) = ELIGIBLE( SHED_PRIO_7, PROP_PRIO_7 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            /*  ---------------------------------- */
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_0) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_1) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_2) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_3) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_4) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_5) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_6) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_7) = NOT_ELIGIBLE,
            /*  ---------------------------------- */
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_0) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_1) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_2) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_3) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_4) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_5) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_6) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_7) = NOT_ELIGIBLE)

    NODE_ELIG_FUNCTION( PRV_SCHED_ELIG_N_PP_SHP,
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_0) = ELIGIBLE( SHED_PRIO_1, PROP_PRIO_0 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_1) = ELIGIBLE( SHED_PRIO_2, PROP_PRIO_1 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_2) = ELIGIBLE( SHED_PRIO_3, PROP_PRIO_2 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_3) = ELIGIBLE( SHED_PRIO_4, PROP_PRIO_3 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_4) = ELIGIBLE( SHED_PRIO_5, PROP_PRIO_4 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_5) = ELIGIBLE( SHED_PRIO_6, PROP_PRIO_5 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_6) = ELIGIBLE( SHED_PRIO_7, PROP_PRIO_6 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_7) = ELIGIBLE( SHED_PRIO_7, PROP_PRIO_7 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            /*  ---------------------------------- */
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_0) = ELIGIBLE( SHED_PRIO_1, PROP_PRIO_0 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_1) = ELIGIBLE( SHED_PRIO_2, PROP_PRIO_1 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_2) = ELIGIBLE( SHED_PRIO_3, PROP_PRIO_2 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_3) = ELIGIBLE( SHED_PRIO_4, PROP_PRIO_3 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_4) = ELIGIBLE( SHED_PRIO_5, PROP_PRIO_4 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_5) = ELIGIBLE( SHED_PRIO_6, PROP_PRIO_5 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_6) = ELIGIBLE( SHED_PRIO_7, PROP_PRIO_6 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_7) = ELIGIBLE( SHED_PRIO_7, PROP_PRIO_7 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            /*  ---------------------------------- */
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_0) = ELIGIBLE( SHED_PRIO_0, PROP_PRIO_0 , NOT_USE_MIN_TB,     USE_MAX_TB),
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_1) = ELIGIBLE( SHED_PRIO_0, PROP_PRIO_0 , NOT_USE_MIN_TB,     USE_MAX_TB),
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_2) = ELIGIBLE( SHED_PRIO_0, PROP_PRIO_0 , NOT_USE_MIN_TB,     USE_MAX_TB),
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_3) = ELIGIBLE( SHED_PRIO_0, PROP_PRIO_0 , NOT_USE_MIN_TB,     USE_MAX_TB),
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_4) = ELIGIBLE( SHED_PRIO_0, PROP_PRIO_0 , NOT_USE_MIN_TB,     USE_MAX_TB),
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_5) = ELIGIBLE( SHED_PRIO_0, PROP_PRIO_0 , NOT_USE_MIN_TB,     USE_MAX_TB),
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_6) = ELIGIBLE( SHED_PRIO_0, PROP_PRIO_0 , NOT_USE_MIN_TB,     USE_MAX_TB),
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_7) = ELIGIBLE( SHED_PRIO_0, PROP_PRIO_0 , NOT_USE_MIN_TB,     USE_MAX_TB),
            /*  ---------------------------------- */
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_0) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_1) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_2) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_3) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_4) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_5) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_6) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_7) = NOT_ELIGIBLE)

    NODE_ELIG_FUNCTION( PRV_SCHED_ELIG_N_SHP_4P_MIN_4P_MAX,
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_0) = ELIGIBLE( SHED_PRIO_4, PROP_PRIO_4 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_1) = ELIGIBLE( SHED_PRIO_5, PROP_PRIO_5 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_2) = ELIGIBLE( SHED_PRIO_6, PROP_PRIO_6 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_3) = ELIGIBLE( SHED_PRIO_7, PROP_PRIO_7 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_4) = ELIGIBLE( SHED_PRIO_7, PROP_PRIO_7 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_5) = ELIGIBLE( SHED_PRIO_7, PROP_PRIO_7 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_6) = ELIGIBLE( SHED_PRIO_7, PROP_PRIO_7 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_7) = ELIGIBLE( SHED_PRIO_7, PROP_PRIO_7 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            /*  ---------------------------------- */
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_0) = ELIGIBLE( SHED_PRIO_4, PROP_PRIO_4 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_1) = ELIGIBLE( SHED_PRIO_5, PROP_PRIO_5 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_2) = ELIGIBLE( SHED_PRIO_6, PROP_PRIO_6 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_3) = ELIGIBLE( SHED_PRIO_7, PROP_PRIO_7 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_4) = ELIGIBLE( SHED_PRIO_7, PROP_PRIO_7 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_5) = ELIGIBLE( SHED_PRIO_7, PROP_PRIO_7 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_6) = ELIGIBLE( SHED_PRIO_7, PROP_PRIO_7 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_7) = ELIGIBLE( SHED_PRIO_7, PROP_PRIO_7 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            /*  ---------------------------------- */
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_0) = ELIGIBLE( SHED_PRIO_0, PROP_PRIO_0 , NOT_USE_MIN_TB,     USE_MAX_TB),
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_1) = ELIGIBLE( SHED_PRIO_1, PROP_PRIO_1 , NOT_USE_MIN_TB,     USE_MAX_TB),
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_2) = ELIGIBLE( SHED_PRIO_2, PROP_PRIO_2 , NOT_USE_MIN_TB,     USE_MAX_TB),
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_3) = ELIGIBLE( SHED_PRIO_3, PROP_PRIO_3 , NOT_USE_MIN_TB,     USE_MAX_TB),
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_4) = ELIGIBLE( SHED_PRIO_3, PROP_PRIO_3 , NOT_USE_MIN_TB,     USE_MAX_TB),
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_5) = ELIGIBLE( SHED_PRIO_3, PROP_PRIO_3 , NOT_USE_MIN_TB,     USE_MAX_TB),
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_6) = ELIGIBLE( SHED_PRIO_3, PROP_PRIO_3 , NOT_USE_MIN_TB,     USE_MAX_TB),
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_7) = ELIGIBLE( SHED_PRIO_3, PROP_PRIO_3 , NOT_USE_MIN_TB,     USE_MAX_TB),
            /*  ---------------------------------- */
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_0) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_1) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_2) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_3) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_4) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_5) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_6) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_7) = NOT_ELIGIBLE)

    NODE_ELIG_FUNCTION( PRV_SCHED_ELIG_N_SHP_PP_TB,
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_0) = ELIGIBLE( SHED_PRIO_0, PROP_PRIO_0 , NOT_USE_MIN_TB,     USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_1) = ELIGIBLE( SHED_PRIO_1, PROP_PRIO_1 , NOT_USE_MIN_TB,     USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_2) = ELIGIBLE( SHED_PRIO_2, PROP_PRIO_2 , NOT_USE_MIN_TB,     USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_3) = ELIGIBLE( SHED_PRIO_3, PROP_PRIO_3 , NOT_USE_MIN_TB,     USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_4) = ELIGIBLE( SHED_PRIO_4, PROP_PRIO_4 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_5) = ELIGIBLE( SHED_PRIO_5, PROP_PRIO_5 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_6) = ELIGIBLE( SHED_PRIO_6, PROP_PRIO_6 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_7) = ELIGIBLE( SHED_PRIO_7, PROP_PRIO_7 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            /*  ---------------------------------- */
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_0) = NOT_ELIGIBLE,
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_1) = NOT_ELIGIBLE,
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_2) = NOT_ELIGIBLE,
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_3) = NOT_ELIGIBLE,
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_4) = ELIGIBLE( SHED_PRIO_4, PROP_PRIO_4 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_5) = ELIGIBLE( SHED_PRIO_5, PROP_PRIO_5 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_6) = ELIGIBLE( SHED_PRIO_6, PROP_PRIO_6 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_7) = ELIGIBLE( SHED_PRIO_7, PROP_PRIO_7 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            /*  ---------------------------------- */
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_0) = ELIGIBLE( SHED_PRIO_0, PROP_PRIO_0 , NOT_USE_MIN_TB,     USE_MAX_TB),
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_1) = ELIGIBLE( SHED_PRIO_1, PROP_PRIO_1 , NOT_USE_MIN_TB,     USE_MAX_TB),
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_2) = ELIGIBLE( SHED_PRIO_2, PROP_PRIO_2 , NOT_USE_MIN_TB,     USE_MAX_TB),
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_3) = ELIGIBLE( SHED_PRIO_3, PROP_PRIO_3 , NOT_USE_MIN_TB,     USE_MAX_TB),
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_4) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_5) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_6) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_7) = NOT_ELIGIBLE,
            /*  ---------------------------------- */
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_0) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_1) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_2) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_3) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_4) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_5) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_6) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_7) = NOT_ELIGIBLE)

    NODE_ELIG_FUNCTION( PRV_SCHED_ELIG_N_SHP_PP_MAX_TB_0,
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_0) = ELIGIBLE( SHED_PRIO_0, PROP_PRIO_0 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_1) = ELIGIBLE( SHED_PRIO_1, PROP_PRIO_1 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_2) = ELIGIBLE( SHED_PRIO_2, PROP_PRIO_2 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_3) = ELIGIBLE( SHED_PRIO_3, PROP_PRIO_3 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_4) = ELIGIBLE( SHED_PRIO_4, PROP_PRIO_4 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_5) = ELIGIBLE( SHED_PRIO_5, PROP_PRIO_5 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_6) = ELIGIBLE( SHED_PRIO_6, PROP_PRIO_6 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_7) = ELIGIBLE( SHED_PRIO_7, PROP_PRIO_7 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            /*  ---------------------------------- */
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_0) = ELIGIBLE( SHED_PRIO_0, PROP_PRIO_0 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_1) = ELIGIBLE( SHED_PRIO_1, PROP_PRIO_1 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_2) = ELIGIBLE( SHED_PRIO_2, PROP_PRIO_2 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_3) = ELIGIBLE( SHED_PRIO_3, PROP_PRIO_3 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_4) = ELIGIBLE( SHED_PRIO_4, PROP_PRIO_4 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_5) = ELIGIBLE( SHED_PRIO_5, PROP_PRIO_5 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_6) = ELIGIBLE( SHED_PRIO_6, PROP_PRIO_6 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_7) = ELIGIBLE( SHED_PRIO_7, PROP_PRIO_7 ,     USE_MIN_TB, NOT_USE_MAX_TB),
            /*  ---------------------------------- */
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_0) = ELIGIBLE( SHED_PRIO_0, PROP_PRIO_0 , NOT_USE_MIN_TB,     USE_MAX_TB),
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_1) = ELIGIBLE( SHED_PRIO_0, PROP_PRIO_0 , NOT_USE_MIN_TB,     USE_MAX_TB),
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_2) = ELIGIBLE( SHED_PRIO_0, PROP_PRIO_0 , NOT_USE_MIN_TB,     USE_MAX_TB),
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_3) = ELIGIBLE( SHED_PRIO_0, PROP_PRIO_0 , NOT_USE_MIN_TB,     USE_MAX_TB),
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_4) = ELIGIBLE( SHED_PRIO_0, PROP_PRIO_0 , NOT_USE_MIN_TB,     USE_MAX_TB),
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_5) = ELIGIBLE( SHED_PRIO_0, PROP_PRIO_0 , NOT_USE_MIN_TB,     USE_MAX_TB),
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_6) = ELIGIBLE( SHED_PRIO_0, PROP_PRIO_0 , NOT_USE_MIN_TB,     USE_MAX_TB),
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_7) = ELIGIBLE( SHED_PRIO_0, PROP_PRIO_0 , NOT_USE_MIN_TB,     USE_MAX_TB),
            /*  ---------------------------------- */
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_0) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_1) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_2) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_3) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_4) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_5) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_6) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_7) = NOT_ELIGIBLE)

    NODE_ELIG_FUNCTION( PRV_SCHED_ELIG_N_MAX_LIM_SHP_FP0,
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_0) = NOT_ELIGIBLE,
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_1) = NOT_ELIGIBLE,
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_2) = NOT_ELIGIBLE,
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_3) = NOT_ELIGIBLE,
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_4) = NOT_ELIGIBLE,
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_5) = NOT_ELIGIBLE,
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_6) = NOT_ELIGIBLE,
            N_ENTRY( POS_MIN_TB, POS_MAX_TB, PROP_PRIO_7) = NOT_ELIGIBLE,
            /*  ---------------------------------- */
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_0) = ELIGIBLE( SHED_PRIO_0, PROP_PRIO_0 ,     USE_MIN_TB, USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_1) = ELIGIBLE( SHED_PRIO_0, PROP_PRIO_0 ,     USE_MIN_TB, USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_2) = ELIGIBLE( SHED_PRIO_0, PROP_PRIO_0 ,     USE_MIN_TB, USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_3) = ELIGIBLE( SHED_PRIO_0, PROP_PRIO_0 ,     USE_MIN_TB, USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_4) = ELIGIBLE( SHED_PRIO_0, PROP_PRIO_0 ,     USE_MIN_TB, USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_5) = ELIGIBLE( SHED_PRIO_0, PROP_PRIO_0 ,     USE_MIN_TB, USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_6) = ELIGIBLE( SHED_PRIO_0, PROP_PRIO_0 ,     USE_MIN_TB, USE_MAX_TB),
            N_ENTRY( POS_MIN_TB, NEG_MAX_TB, PROP_PRIO_7) = ELIGIBLE( SHED_PRIO_0, PROP_PRIO_0 ,     USE_MIN_TB, USE_MAX_TB),
            /*  ---------------------------------- */
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_0) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_1) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_2) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_3) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_4) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_5) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_6) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, POS_MAX_TB, PROP_PRIO_7) = NOT_ELIGIBLE,
            /*  ---------------------------------- */
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_0) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_1) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_2) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_3) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_4) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_5) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_6) = NOT_ELIGIBLE,
            N_ENTRY( NEG_MIN_TB, NEG_MAX_TB, PROP_PRIO_7) = NOT_ELIGIBLE)

    NODE_DEQ_DISABLE(PRV_SCHED_ELIG_DEQ_DISABLE)

out:
    return rc;


/*******************************************************************************************/
    /* reduced macro example - with assert : function Id < PRV_SCHED_ELIG_FUNC_TABLE_SIZE  failed */
    /* NODE_ELIG_FUN_FIXED_PRIORITY(64, FIX_PRIORITY(FIX_PRIO_3))   */
}

/**
 */
/* Auxiliary function */
uint16_t convert_elig_func_to_value(struct sched_elig_prio_func_out *elig_func)
{
    uint16_t var;

    var = elig_func->max_tb;
    var = var | (elig_func->min_tb << 1);
    var = var | (elig_func->prop_prio << 2);
    var = var | (elig_func->sched_prio << 5);
    var = var | (elig_func->elig << 8);

    return var;
}

void prvSchedConvertValueToEligFunc(uint16_t elig_val, struct sched_elig_prio_func_out *elig_func)
{
    /*
    FuncOut[8]   - Elig
    FuncOut[7:5] - Scheduling Priority.
    FuncOut[4:2] - Propagated Priority.
    FuncOut[1]   - Use Min Token Bucket.
      FuncOut[0]   - Use Max Token Bucket.
    */
    uint8_t mask = 0x07;

    elig_func->elig = (elig_val >> 8) & 0x01;
    elig_func->max_tb = elig_val & 0x01;
    elig_func->min_tb = (elig_val & 0x02) >> 1;
    elig_func->prop_prio = (elig_val >> 2) & mask;
    elig_func->sched_prio = (elig_val >> 5) & mask;
}


/**
 */
void prvSchedSetSwQEligPrioFunc(struct tm_elig_prio_func_queue *func_table,
                                   uint16_t func_offset,
                                   union sched_elig_prio_func *queue_func_out_arr)
{

    func_table[func_offset].tbl_entry.func_out[0] = convert_elig_func_to_value(&(queue_func_out_arr->queue_elig_prio_func[0]));
    func_table[func_offset].tbl_entry.func_out[1] = convert_elig_func_to_value(&(queue_func_out_arr->queue_elig_prio_func[1]));
    func_table[func_offset].tbl_entry.func_out[2] = convert_elig_func_to_value(&(queue_func_out_arr->queue_elig_prio_func[2]));
    func_table[func_offset].tbl_entry.func_out[3] = convert_elig_func_to_value(&(queue_func_out_arr->queue_elig_prio_func[3]));

}


/**
 */
void prvSchedSetSwNodeEligPrioFunc(struct tm_elig_prio_func_node *func_table,
                                   uint16_t func_offset,
                                   union sched_elig_prio_func *node_func_out_arr)
{
    int i;
    for (i = 0; i < 8; i++)
    {   /* Entry ID */
            func_table[func_offset].tbl_entry[i].func_out[0] = convert_elig_func_to_value(&(node_func_out_arr->node_elig_prio_func[i][0]));
            func_table[func_offset].tbl_entry[i].func_out[1] = convert_elig_func_to_value(&(node_func_out_arr->node_elig_prio_func[i][1]));
            func_table[func_offset].tbl_entry[i].func_out[2] = convert_elig_func_to_value(&(node_func_out_arr->node_elig_prio_func[i][2]));
            func_table[func_offset].tbl_entry[i].func_out[3] = convert_elig_func_to_value(&(node_func_out_arr->node_elig_prio_func[i][3]));
    }
}




int prvSchedIsQueueEligFuncUseShaper(struct tm_elig_prio_func_queue * queue_func_table, uint8_t func_index)
{
    if ((func_index==TM_NODE_DISABLED_FUN) || (func_index >= PRV_SCHED_ELIG_FUNC_TABLE_SIZE))  return -1;
    /* if minTB or maxTB bit is used in at least one entry - this function uses shaper */
    if  (queue_func_table[func_index].tbl_entry.func_out[0] & 0x3) return 1;
    if  (queue_func_table[func_index].tbl_entry.func_out[1] & 0x3) return 1;
    if  (queue_func_table[func_index].tbl_entry.func_out[2] & 0x3) return 1;
    if  (queue_func_table[func_index].tbl_entry.func_out[3] & 0x3) return 1;
    return 0;
}

int prvSchedIsNodeEligFuncUseShaper(struct tm_elig_prio_func_node * node_func_table, uint8_t func_index)
{
    int i;
    if ((func_index==TM_NODE_DISABLED_FUN) || (func_index >= PRV_SCHED_ELIG_FUNC_TABLE_SIZE))  return -1;
    for (i=0; i < 8 ; i++)
    {
        /* if minTB or maxTB bit is used in at least one entry - this function uses shaper */
        if (node_func_table[func_index].tbl_entry[i].func_out[0] & 0x3) return 1;
        if (node_func_table[func_index].tbl_entry[i].func_out[1] & 0x3) return 1;
        if (node_func_table[func_index].tbl_entry[i].func_out[2] & 0x3) return 1;
        if (node_func_table[func_index].tbl_entry[i].func_out[3] & 0x3) return 1;
    }
    return 0;
}

