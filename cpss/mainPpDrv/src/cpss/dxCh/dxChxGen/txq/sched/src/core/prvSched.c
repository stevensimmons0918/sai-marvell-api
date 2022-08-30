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
* @brief tm_sched implementation.
*
* @file tm_sched.c
*
* $Revision: 2.0 $
*/

#include <stdio.h>
#include <errno.h>
#include <cpss/common/config/private/prvCpssConfigTypes.h>
#include <cpss/dxCh/dxChxGen/txq/private/Sched/prvSched.h>
#include <cpss/dxCh/dxChxGen/txq/private/Sched/prvSchedErrCodes.h>
#include <cpss/dxCh/dxChxGen/txq/private/Sched/core/prvSetHwRegisters.h>
#include <cpss/dxCh/dxChxGen/txq/private/Sched/core/prvSchedSetLocalDbDefaults.h>
#include <cpss/dxCh/dxChxGen/txq/private/Sched/core/prvSchedOsInterface.h>
#include <cpss/dxCh/dxChxGen/txq/private/Sched/core/prvSchedHwConfigurationInterface.h>
#include <cpss/dxCh/dxChxGen/txq/private/Sched/core/prvSchedShapingUtils.h>
#include <cpss/dxCh/dxChxGen/txq/private/prvCpssDxChTxqMain.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

/**

#define PRINT_SHAPING_RESULTS


*/




int prvCpssTxqSchedulerGetPortMaxPeriods(uint16_t   * max_periods)
{

    *max_periods = FALCON_PORT_PERIODS_MAX;

    return 0;
}


int prvCpssTxqSchedulerGetOptimalResolutionExponent(enum schedLevel level,uint8_t * exponent)
{

    switch(level)
    {
    case SCHED_Q_LEVEL:
        *exponent = FALCON_QUEUE_NODE_OPTIMAL_RES_EXP;
        break;
    case SCHED_A_LEVEL:
        *exponent = FALCON_A_NODE_OPTIMAL_RES_EXP;
        break;
    case SCHED_B_LEVEL:
        *exponent = FALCON_B_NODE_OPTIMAL_RES_EXP;
        break;
    case SCHED_C_LEVEL:
        *exponent = FALCON_C_NODE_OPTIMAL_RES_EXP;
        break;

    default:
        return -EINVAL;

    }

    return 0;
}


int prvCpssTxqSchedulerGetMaximalToken(unsigned int * maximal_token)
{
    *maximal_token = FALCON_MAX_POSSIBLE_TOKEN;
    return 0;
}


int prvCpssTxqSchedulerGetOptimalShaperUpdateFrequency
(
    GT_U8 devNum,
    unsigned int * optimal_shaper_update_frequency,
    int is_port
)
{
    GT_BOOL isSip6_10 = PRV_CPSS_SIP_6_10_CHECK_MAC(devNum);
    GT_BOOL isSip6_15 = PRV_CPSS_TXQ_LIKE_PHOENIX_MAC(devNum);

    if(is_port == 0)
    {
        *optimal_shaper_update_frequency = isSip6_10?
            (isSip6_15?PHOENIX_OPTIMAL_NODE_FREQUENCY:HAWK_OPTIMAL_NODE_FREQUENCY):
            FALCON_OPTIMAL_NODE_FREQUENCY;
    }
    else
    {
        *optimal_shaper_update_frequency = FALCON_OPTIMAL_PORT_FREQUENCY;
    }
    return 0;
}

int prvCpssTxqSchedulerGetUnit
(
    GT_U8 devNum,
    enum schedLevel level,
    uint32_t * unit
)
{
    GT_BOOL isSip6_10 = PRV_CPSS_SIP_6_10_CHECK_MAC(devNum);
    GT_BOOL isSip6_15 = PRV_CPSS_TXQ_LIKE_PHOENIX_MAC(devNum);

    switch(level)
    {
    case SCHED_Q_LEVEL:
        *unit = isSip6_10?(isSip6_15?PHOENIX_Q_UNIT:HAWK_Q_UNIT):FALCON_Q_UNIT;
        break;
    case SCHED_A_LEVEL:
        *unit = isSip6_10?(isSip6_15?PHOENIX_A_UNIT:HAWK_A_UNIT):FALCON_A_UNIT;
        break;
    case SCHED_B_LEVEL:
        *unit = isSip6_10?(isSip6_15?PHOENIX_B_UNIT:HAWK_B_UNIT):FALCON_B_UNIT;
        break;
    case SCHED_C_LEVEL:
        *unit = FALCON_C_UNIT;
        break;
    case SCHED_P_LEVEL:
        *unit = FALCON_PORT_UNIT;
        break;
    default:
        return -EINVAL;

    }

    return 0;
}

/**
*/
int prvCpssTxqSchedulerConfigurePeriodicScheme(PRV_CPSS_SCHED_HANDLE hndl, struct schedPerLevelPeriodicParams *lvl_params)
{
    int rc;
    int i;

    unsigned int    base_update_frequency;
    unsigned int    node_shaper_update_frequency;
    unsigned int    optimal_node_shaper_update_frequency;
    unsigned int    port_shaper_update_frequency;
    uint16_t port_max_periods;
    double          desired_factor;
    uint16_t        L;
    uint16_t        K;
    uint16_t        N;

    TM_CTL(ctl, hndl)




        if (ctl->periodic_scheme_state == TM_ENABLE)
        {
            rc = -EPERM;
            goto out;
        }


        base_update_frequency = ctl->freq/4;

        /* check if shaping is Enabled/Disabled per level */
        /* no need to indicate the number of elements participating periodic
        * shaper update for Rev B */
        for (i=SCHED_Q_LEVEL; i<=SCHED_P_LEVEL; i++)
        {
            switch (lvl_params[i].per_state)
            {
            case TM_ENABLE:     ctl->level_data[i].shaping_status=TM_ENABLE;break;
            case TM_DISABLE:    ctl->level_data[i].shaping_status=TM_DISABLE;break;
            default:/*all others */
                {
                    rc = -EADDRNOTAVAIL;
                    goto out;
                }
            }
            switch (lvl_params[i].shaper_dec)
            {
            case TM_ENABLE:     ctl->level_data[i].shaper_dec=TM_ENABLE;break;
            case TM_DISABLE:    ctl->level_data[i].shaper_dec=TM_DISABLE;break;
            default:/*all others */
                {
                    rc = -ENODEV;
                    goto out;
                }
            }
        }
        /* here play a little with frequencies */
        rc = prvCpssTxqSchedulerGetOptimalShaperUpdateFrequency(PRV_SCHED_DEV_NUM_GET(ctl),&optimal_node_shaper_update_frequency,0);
        if (rc)
        {
            return rc;
        }


        if (base_update_frequency > optimal_node_shaper_update_frequency)
            node_shaper_update_frequency=optimal_node_shaper_update_frequency;
        else
            node_shaper_update_frequency = (base_update_frequency/1000000)*1000000; /* rounding to integer MHz */

        desired_factor= (double)node_shaper_update_frequency/base_update_frequency;
        if (!prvSchedShapingUtilsFindLKN(desired_factor, &L, &K , &N))  return TM_CONF_PER_RATE_L_K_N_NOT_FOUND;

        /*set periodic update for levels */
        /* queue  */
        ctl->level_data[SCHED_Q_LEVEL].level_update_frequency=node_shaper_update_frequency;

        rc = prvCpssTxqSchedulerGetUnit(PRV_SCHED_DEV_NUM_GET(ctl),SCHED_Q_LEVEL,&(ctl->level_data[SCHED_Q_LEVEL].unit));
        if(rc)return rc;

        ctl->level_data[SCHED_Q_LEVEL].per_interval = node_shaper_update_frequency*8/(1<<MAX_DIVIDER_EXP)/ctl->level_data[SCHED_Q_LEVEL].unit;

        rc = prvCpssTxqSchedulerGetOptimalResolutionExponent(SCHED_Q_LEVEL,&(ctl->level_data[SCHED_Q_LEVEL].token_res_exp));
        if(rc)return rc;

        ctl->level_data[SCHED_Q_LEVEL].L = L;
        ctl->level_data[SCHED_Q_LEVEL].K = K;
        ctl->level_data[SCHED_Q_LEVEL].N = N;
        /* A-level */
        ctl->level_data[SCHED_A_LEVEL].level_update_frequency=node_shaper_update_frequency;

        rc = prvCpssTxqSchedulerGetUnit(PRV_SCHED_DEV_NUM_GET(ctl),SCHED_A_LEVEL,&(ctl->level_data[SCHED_A_LEVEL].unit));
        if(rc)return rc;

        ctl->level_data[SCHED_A_LEVEL].per_interval = node_shaper_update_frequency*8/(1<<MAX_DIVIDER_EXP)/ctl->level_data[SCHED_A_LEVEL].unit;

        rc = prvCpssTxqSchedulerGetOptimalResolutionExponent(SCHED_A_LEVEL,&(ctl->level_data[SCHED_A_LEVEL].token_res_exp));
        if(rc)return rc;

        ctl->level_data[SCHED_A_LEVEL].L = L;
        ctl->level_data[SCHED_A_LEVEL].K = K;
        ctl->level_data[SCHED_A_LEVEL].N = N;
        /* B-level */
        ctl->level_data[SCHED_B_LEVEL].level_update_frequency=node_shaper_update_frequency;

        rc = prvCpssTxqSchedulerGetUnit(PRV_SCHED_DEV_NUM_GET(ctl),SCHED_B_LEVEL,&(ctl->level_data[SCHED_B_LEVEL].unit));
        if(rc)return rc;

        ctl->level_data[SCHED_B_LEVEL].per_interval = node_shaper_update_frequency*8/(1<<MAX_DIVIDER_EXP)/ctl->level_data[SCHED_B_LEVEL].unit;

        rc = prvCpssTxqSchedulerGetOptimalResolutionExponent(SCHED_B_LEVEL,&(ctl->level_data[SCHED_B_LEVEL].token_res_exp));
        if(rc)return rc;

        ctl->level_data[SCHED_B_LEVEL].L = L;
        ctl->level_data[SCHED_B_LEVEL].K = K;
        ctl->level_data[SCHED_B_LEVEL].N = N;
        /* C-level */
        ctl->level_data[SCHED_C_LEVEL].level_update_frequency=node_shaper_update_frequency;

        rc = prvCpssTxqSchedulerGetUnit(PRV_SCHED_DEV_NUM_GET(ctl),SCHED_C_LEVEL,&(ctl->level_data[SCHED_C_LEVEL].unit));
        if(rc)return rc;

        ctl->level_data[SCHED_C_LEVEL].per_interval = node_shaper_update_frequency*8/(1<<MAX_DIVIDER_EXP)/ctl->level_data[SCHED_C_LEVEL].unit;

        rc = prvCpssTxqSchedulerGetOptimalResolutionExponent(SCHED_C_LEVEL,&(ctl->level_data[SCHED_C_LEVEL].token_res_exp));
        if(rc)return rc;

        ctl->level_data[SCHED_C_LEVEL].L = L;
        ctl->level_data[SCHED_C_LEVEL].K = K;
        ctl->level_data[SCHED_C_LEVEL].N = N;
        /*---------------------------------------------------------------------------------------------------------
        */
        rc = prvCpssTxqSchedulerGetOptimalShaperUpdateFrequency(PRV_SCHED_DEV_NUM_GET(ctl),&optimal_node_shaper_update_frequency,1);
        if (rc)
        {
            return rc;
        }
        if (base_update_frequency > optimal_node_shaper_update_frequency)
            port_shaper_update_frequency=optimal_node_shaper_update_frequency;
        else
            port_shaper_update_frequency = (base_update_frequency/5000000)*5000000; /* rounding in order to get more accuracy */
        /* here provide it in by K,L,N */
        desired_factor= (double)port_shaper_update_frequency/base_update_frequency;
        if (prvSchedShapingUtilsFindLKN(desired_factor, &L, &K , &N ))
        {
            ctl->level_data[SCHED_P_LEVEL].L=L;
            ctl->level_data[SCHED_P_LEVEL].K=K;
            ctl->level_data[SCHED_P_LEVEL].N=N;
            ctl->level_data[SCHED_P_LEVEL].level_update_frequency=port_shaper_update_frequency;
            rc = prvCpssTxqSchedulerGetUnit(PRV_SCHED_DEV_NUM_GET(ctl),SCHED_P_LEVEL,&(ctl->level_data[SCHED_P_LEVEL].unit));
            if(rc)return rc;

            rc = prvCpssTxqSchedulerGetPortMaxPeriods(&port_max_periods);
            if(rc)return rc;

            ctl->level_data[SCHED_P_LEVEL].per_interval = port_shaper_update_frequency*8/port_max_periods/ctl->level_data[SCHED_P_LEVEL].unit;

            ctl->level_data[SCHED_P_LEVEL].token_res_exp=PORT_OPTIMAL_RES_EXP;
        }
        else return TM_CONF_PER_RATE_L_K_N_NOT_FOUND;

#ifdef PRINT_SHAPING_RESULTS
        schedPrintf("--------------------------------------------------------------\n");
        schedPrintf("base update frequency = % d \n",base_update_frequency);
        for (i=SCHED_Q_LEVEL ; i <= SCHED_P_LEVEL ; i++)
        {
            schedPrintf(" level : %u, unit: %u frequency : %u , per_interval : %4u , token_res_exp=%d  K=%u, L=%u, N=%2u,(desired factor=%f factor=%f)\n",
                i,
                ctl->level_data[i].unit,
                ctl->level_data[i].level_update_frequency,
                ctl->level_data[i].per_interval,
                ctl->level_data[i].token_res_exp,
                ctl->level_data[i].K,
                ctl->level_data[i].L,
                ctl->level_data[i].N,
                1.0*ctl->level_data[i].level_update_frequency/base_update_frequency,
                1.0-(double)(ctl->level_data[i].K + ctl->level_data[i].L)/(double)((ctl->level_data[i].N+1)*ctl->level_data[i].K+ctl->level_data[i].L*((ctl->level_data[i].N+1)+1)));
        }
        schedPrintf("--------------------------------------------------------------\n");
#endif

        /* for all levels at once */
#if 0
        rc = calculate_scrubbing(hndl);
        if (rc)
            goto out;
#endif
        /* Download to HW */
        rc = prvSchedLowLevelSetHwPeriodicScheme(hndl);
        if (rc)
        {
            rc = TM_HW_CONF_PER_SCHEME_FAILED;
            goto out;
        }

        ctl->periodic_scheme_state = TM_ENABLE;
out:
        return rc;
}

int prvCpssTxqSchedulerPeriodicUpdateStatusSet(PRV_CPSS_SCHED_HANDLE hndl, enum schedLevel level, uint8_t status)
{
    int rc;
    uint8_t old_status;

    TM_CTL(ctl, hndl)



        /* check parameters validity */
        if ((uint32_t)level > SCHED_P_LEVEL)
        {
            rc = -EFAULT;
            goto out;
        }
        if ((status != TM_ENABLE) && (status != TM_DISABLE))
        {
            rc = -EACCES;
            goto out;
        }
        old_status = ctl->level_data[level].shaping_status;
        ctl->level_data[level].shaping_status = status; /* TM_ENABLE/TM_DISABLE */

        rc = prvSchedLowLevelSetPerLevelShapingParameters((PRV_CPSS_SCHED_HANDLE)ctl, level);
        if (rc < 0)
        {
            /* set to default */
            ctl->level_data[level].shaping_status = old_status;
            rc = TM_HW_PERIODIC_UPDATE_STATUS_SET_FAILED;
        }
out:
        return rc;
}


int prvCpssTxqSchedulerPeriodicUpdateStatusGet(PRV_CPSS_SCHED_HANDLE hndl, struct schedPerLevelPeriodicParams  *status)
{
    int rc = 0;
    int i;

    TM_CTL(ctl, hndl)

        for (i=SCHED_Q_LEVEL; i<=SCHED_P_LEVEL; i++)
        {
            rc = prvSchedLowLevelGetPerLevelShapingStatusFromHw(hndl, i,  &(status[i]));
            /*
            //status[i].per_state = ctl->level_data[i].shaping_status;
            //status[i].shaper_dec = ctl->level_data[i].shaper_dec;
            */
        }

        return rc;
}



int prvCpssTxqSchedulerMaxSelectedUnit(PRV_CPSS_SCHED_HANDLE hndl, unsigned mtu)
{
    TM_CTL(ctl, hndl)

        return ctl->min_pkg_size + mtu;
}

int prvCpssTxqSchedulerPortMaxSelectedUnit(PRV_CPSS_SCHED_HANDLE hndl, unsigned mtu)
{
    uint32_t bytes;
    uint32_t msu;

    TM_CTL(ctl, hndl)

        bytes = 16 * 4 * ctl->port_ch_emit * ctl->dwrr_bytes_burst_limit;
    msu = ctl->min_pkg_size + mtu;

    if (msu >= bytes)
        return msu;
    else
        return bytes;
}

int prvCpssTxqSchedulerGetPortQuantumLimits(PRV_CPSS_SCHED_HANDLE hndl, struct sched_quantum_limits * port_quantum_limits)
{
    TM_CTL(ctl, hndl)
        port_quantum_limits->resolution  = ctl->port_quantum_chunk_size_bytes;
    port_quantum_limits->minQuantum  = ctl->min_port_quantum_bytes;
    port_quantum_limits->maxQuantum  = ctl->max_port_quantum_bytes;
    return 0;
}
int prvCpssTxqSchedulerGetNodeQuantumLimits(PRV_CPSS_SCHED_HANDLE hndl, struct  sched_quantum_limits * node_quantum_limits)
{
    TM_CTL(ctl, hndl)
        node_quantum_limits->resolution  = ctl->node_quantum_chunk_size_bytes;
    node_quantum_limits->minQuantum  = ctl->min_node_quantum_bytes;
    node_quantum_limits->maxQuantum  = ctl->max_node_quantum_bytes;
    return 0;
}

int prvCpssTxqSchedulerQLevelPeriodSet(PRV_CPSS_SCHED_HANDLE hndl, uint32_t period,uint32_t token_res_exp)
{
    unsigned int    node_shaper_update_frequency;
    unsigned int    optimal_node_shaper_update_frequency;
    unsigned int    base_update_frequency;
    int rc;

    TM_CTL(ctl, hndl)

    base_update_frequency = ctl->freq/4;

    /* here play a little with frequencies */
     rc = prvCpssTxqSchedulerGetOptimalShaperUpdateFrequency(PRV_SCHED_DEV_NUM_GET(ctl),&optimal_node_shaper_update_frequency,0);
     if (rc)
     {
      return rc;
     }

    if (base_update_frequency > optimal_node_shaper_update_frequency)
       node_shaper_update_frequency=optimal_node_shaper_update_frequency;
    else
       node_shaper_update_frequency = (base_update_frequency/1000000)*1000000; /* rounding to integer MHz */

    /*Lets calculate unit*/

    ctl->level_data[SCHED_Q_LEVEL].per_interval = period;

    ctl->level_data[SCHED_Q_LEVEL].unit = node_shaper_update_frequency*8/(1<<MAX_DIVIDER_EXP)/ctl->level_data[SCHED_Q_LEVEL].per_interval;

    if(token_res_exp)
    {
        ctl->level_data[SCHED_Q_LEVEL].token_res_exp =token_res_exp;
        schedPrintf("token_res_exp %d Kbps\n",ctl->level_data[SCHED_Q_LEVEL].token_res_exp);
     }

    schedPrintf("New unit is %d Kbps\n",ctl->level_data[SCHED_Q_LEVEL].unit);


    /* Download to HW */
    rc = prvSchedLowLevelSetHwPeriodicScheme(hndl);
    if (rc)
    {
      rc = TM_HW_CONF_PER_SCHEME_FAILED;
      goto out;
    }

    ctl->periodic_scheme_state = TM_ENABLE;

    out:
        return rc;

}





