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
 * @brief  functions for set/get  platform specific registers
 *
* @file set_hw_registers_platform.c
*
* $Revision: 2.0 $
 */
#include <cpss/dxCh/dxChxGen/txq/private/Sched/core/prvSchedOsInterface.h>

#include <cpss/dxCh/dxChxGen/txq/private/Sched/core/prvSchedCoreTypes.h>
#include <cpss/dxCh/dxChxGen/txq/private/Sched/platform/prvSchedRegistersInterface.h>

#include <cpss/dxCh/dxChxGen/txq/private/Sched/platform/prvSchedSetHwRegistersImp.h>

#include <cpss/dxCh/dxChxGen/txq/private/Sched/core/prvSetHwRegisters.h>
#include <cpss/dxCh/dxChxGen/txq/private/Sched/core/prvSchedSetLocalDbDefaults.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

#define PRV_CPSS_DXCH_SIP6_TXQ_ON_EMULATOR           cpssDeviceRunCheck_onEmulator



int prvSchedLowLevelSetPerLevelShapingParameters(PRV_CPSS_SCHED_HANDLE hndl, enum schedLevel level)
{
    int rc = -ERANGE;

    struct prvCpssDxChTxqSchedAddressSpace * addressSpacePtr = NULL;


#if 0
    TM_REGISTER_VAR(TM_Sched_PPerCtlConf_Addr)
    TM_REGISTER_VAR(TM_Sched_CPerCtlConf_Addr)
#endif

    TXQ_SCHED_DEVDEPEND_REGISTER_VAR(APerCtlConf_Addr)
    TXQ_SCHED_DEVDEPEND_REGISTER_VAR(QPerCtlConf_Addr)
    TXQ_SCHED_DEVDEPEND_REGISTER_VAR(BPerCtlConf_Addr)

    TM_CTL(ctl, hndl)

    rc =prvSchedAdressSpacePointerGet(hndl,&addressSpacePtr);
    if (rc)return rc;


    if(level >=SCHED_B_LEVEL&&level <=SCHED_P_LEVEL)
    {
         /*No need to configure shaping on those levels on Falcon.Only Q and A levels*/
        rc = 0;
    }

    switch (level)
    {
    case SCHED_Q_LEVEL:
        /* reset register reserved fields */
        TXQ_SCHED_REGISTER_DEVDEPEND_RESET(QPerCtlConf_Addr)
        if (rc) break;
        /* assign register fields */
        TXQ_SCHED_DEVDEPEND_FIELD_PARSE_SET(QPerCtlConf_Addr, DecEn,           ctl->level_data[SCHED_Q_LEVEL].shaper_dec)
        TXQ_SCHED_DEVDEPEND_FIELD_PARSE_SET(QPerCtlConf_Addr, PerInterval ,    ctl->level_data[SCHED_Q_LEVEL].per_interval)
        TXQ_SCHED_DEVDEPEND_FIELD_PARSE_SET(QPerCtlConf_Addr, PerEn ,          ctl->level_data[SCHED_Q_LEVEL].shaping_status)


        /* write register */
        TXQ_SCHED_WRITE_REGISTER_DEVDEPEND(addressSpacePtr->Sched.QPerCtlConf_Addr,QPerCtlConf_Addr)
        break;
    case SCHED_A_LEVEL:
        /* reset register reserved fields */
        TXQ_SCHED_REGISTER_DEVDEPEND_RESET(APerCtlConf_Addr)
        if (rc) break;
        /* assign register fields */
        TXQ_SCHED_DEVDEPEND_FIELD_PARSE_SET(APerCtlConf_Addr, DecEn ,       ctl->level_data[SCHED_A_LEVEL].shaper_dec)
        TXQ_SCHED_DEVDEPEND_FIELD_PARSE_SET(APerCtlConf_Addr, PerInterval , ctl->level_data[SCHED_A_LEVEL].per_interval)
        TXQ_SCHED_DEVDEPEND_FIELD_PARSE_SET(APerCtlConf_Addr, PerEn ,       ctl->level_data[SCHED_A_LEVEL].shaping_status)



        /* write register */
        TXQ_SCHED_WRITE_REGISTER_DEVDEPEND(addressSpacePtr->Sched.APerCtlConf_Addr,APerCtlConf_Addr)
        break;

    case SCHED_B_LEVEL:
        /* reset register reserved fields */
        TXQ_SCHED_REGISTER_DEVDEPEND_RESET(BPerCtlConf_Addr)
        if (rc) break;
        /* assign register fields */
        TXQ_SCHED_DEVDEPEND_FIELD_PARSE_SET(BPerCtlConf_Addr, DecEn ,       ctl->level_data[SCHED_B_LEVEL].shaper_dec)
        TXQ_SCHED_DEVDEPEND_FIELD_PARSE_SET(BPerCtlConf_Addr, PerInterval , ctl->level_data[SCHED_B_LEVEL].per_interval)
        TXQ_SCHED_DEVDEPEND_FIELD_PARSE_SET(BPerCtlConf_Addr, PerEn ,       ctl->level_data[SCHED_B_LEVEL].shaping_status)



        /* write register */
        TXQ_SCHED_WRITE_REGISTER_DEVDEPEND(addressSpacePtr->Sched.BPerCtlConf_Addr,BPerCtlConf_Addr)
        break;
#if 0   /*No need to configure shaping on those levels on Falcon*/

    case SCHED_C_LEVEL:
        /* reset register reserved fields */
        TXQ_SCHED_RESET_REGISTER(addressSpacePtr->Sched.CPerCtlConf_Addr, TM_Sched_CPerCtlConf_Addr)
        /* assign register fields */
        TXQ_SCHED_REGISTER_SET(TM_Sched_CPerCtlConf_Addr, DecEn ,       ctl->level_data[SCHED_C_LEVEL].shaper_dec)
        TXQ_SCHED_REGISTER_SET(TM_Sched_CPerCtlConf_Addr, PerInterval , ctl->level_data[SCHED_C_LEVEL].per_interval)
        TXQ_SCHED_REGISTER_SET(TM_Sched_CPerCtlConf_Addr ,PerEn ,       ctl->level_data[SCHED_C_LEVEL].shaping_status)


        /* write register */
        TXQ_SCHED_WRITE_REGISTER(addressSpacePtr->Sched.CPerCtlConf_Addr, TM_Sched_CPerCtlConf_Addr)
        break;
    case SCHED_P_LEVEL:
        /* reset register reserved fields */
        TXQ_SCHED_RESET_REGISTER(addressSpacePtr->Sched.PPerCtlConf_Addr, TM_Sched_PPerCtlConf_Addr)
        if (rc) break;
        /* assign register fields */
        TXQ_SCHED_REGISTER_SET(TM_Sched_PPerCtlConf_Addr, DecEn ,       ctl->level_data[SCHED_P_LEVEL].shaper_dec)
        TXQ_SCHED_REGISTER_SET(TM_Sched_PPerCtlConf_Addr, PerInterval , ctl->level_data[SCHED_P_LEVEL].per_interval)
        TXQ_SCHED_REGISTER_SET(TM_Sched_PPerCtlConf_Addr, PerEn ,       ctl->level_data[SCHED_P_LEVEL].shaping_status);

        /* write register */
        TXQ_SCHED_WRITE_REGISTER(addressSpacePtr->Sched.PPerCtlConf_Addr, TM_Sched_PPerCtlConf_Addr)
        break;
  #endif
    default:
        break;
    }
    return rc;
}


int prvSchedLowLevelGetPerLevelShapingStatusFromHw
(
    PRV_CPSS_SCHED_HANDLE hndl,
    enum schedLevel level ,
    struct schedPerLevelPeriodicParams *periodic_params
)
{
    int rc = -ERANGE;

    struct prvCpssDxChTxqSchedAddressSpace * addressSpacePtr = NULL;

    TXQ_SCHED_DEVDEPEND_REGISTER_VAR(PPerCtlConf_Addr)
    TXQ_SCHED_DEVDEPEND_REGISTER_VAR(CPerCtlConf_Addr)
    TXQ_SCHED_DEVDEPEND_REGISTER_VAR(BPerCtlConf_Addr)
    TXQ_SCHED_DEVDEPEND_REGISTER_VAR(APerCtlConf_Addr)
    TXQ_SCHED_DEVDEPEND_REGISTER_VAR(QPerCtlConf_Addr)

    TM_CTL(ctl, hndl)

    rc =prvSchedAdressSpacePointerGet(hndl,&addressSpacePtr);
    if (rc)goto out;

    switch(level)
    {
    case SCHED_Q_LEVEL:
        TXQ_SCHED_READ_REGISTER_DEVDEPEND(addressSpacePtr->Sched.QPerCtlConf_Addr,QPerCtlConf_Addr)
        if (rc) goto out;
        /* assign register fields */
        TXQ_SCHED_DEVDEPEND_FIELD_PARSE_GET(QPerCtlConf_Addr, DecEn,           periodic_params->shaper_dec , (uint8_t))
        TXQ_SCHED_DEVDEPEND_FIELD_PARSE_GET(QPerCtlConf_Addr, PerEn ,          periodic_params->per_state , (uint8_t))
        TXQ_SCHED_DEVDEPEND_FIELD_PARSE_GET(QPerCtlConf_Addr, PerInterval ,    periodic_params->per_interval , (uint32_t))
        break;
    case SCHED_A_LEVEL:
        TXQ_SCHED_READ_REGISTER_DEVDEPEND(addressSpacePtr->Sched.APerCtlConf_Addr,APerCtlConf_Addr)
        if (rc) goto out;
        /* assign register fields */
        TXQ_SCHED_DEVDEPEND_FIELD_PARSE_GET(APerCtlConf_Addr, DecEn ,       periodic_params->shaper_dec , (uint8_t))
        TXQ_SCHED_DEVDEPEND_FIELD_PARSE_GET(APerCtlConf_Addr, PerEn ,       periodic_params->per_state , (uint8_t))
        TXQ_SCHED_DEVDEPEND_FIELD_PARSE_GET(APerCtlConf_Addr, PerInterval ,          periodic_params->per_interval , (uint32_t))
        break;
    case SCHED_B_LEVEL:
        TXQ_SCHED_READ_REGISTER_DEVDEPEND(addressSpacePtr->Sched.BPerCtlConf_Addr,BPerCtlConf_Addr)
        if (rc) goto out;
        /* assign register fields */
        TXQ_SCHED_DEVDEPEND_FIELD_PARSE_GET(BPerCtlConf_Addr, DecEn ,       periodic_params->shaper_dec , (uint8_t))
        TXQ_SCHED_DEVDEPEND_FIELD_PARSE_GET(BPerCtlConf_Addr, PerEn ,       periodic_params->per_state , (uint8_t))
        TXQ_SCHED_DEVDEPEND_FIELD_PARSE_GET(BPerCtlConf_Addr, PerInterval ,          periodic_params->per_interval , (uint32_t))
        break;
    case SCHED_C_LEVEL:
        TXQ_SCHED_READ_REGISTER_DEVDEPEND(addressSpacePtr->Sched.CPerCtlConf_Addr,CPerCtlConf_Addr)
        if (rc) goto out;
        /* assign register fields */
        TXQ_SCHED_DEVDEPEND_FIELD_PARSE_GET(CPerCtlConf_Addr, DecEn ,       periodic_params->shaper_dec , (uint8_t))
        TXQ_SCHED_DEVDEPEND_FIELD_PARSE_GET(CPerCtlConf_Addr ,PerEn ,       periodic_params->per_state , (uint8_t))
        break;
    case SCHED_P_LEVEL:
        TXQ_SCHED_READ_REGISTER_DEVDEPEND(addressSpacePtr->Sched.PPerCtlConf_Addr,PPerCtlConf_Addr)
        if (rc) goto out;
        /* assign register fields */
        TXQ_SCHED_DEVDEPEND_FIELD_PARSE_GET(PPerCtlConf_Addr, DecEn ,       periodic_params->shaper_dec , (uint8_t))
        TXQ_SCHED_DEVDEPEND_FIELD_PARSE_GET(PPerCtlConf_Addr, PerEn ,       periodic_params->per_state , (uint8_t))
    }
out:
    return rc;
}




/* assume here that shaping profile is valid */
int prvSchedLowLevelSetNodeShapingParametersToHw(PRV_CPSS_SCHED_HANDLE hndl, enum schedLevel level, uint32_t node_ind, struct prvCpssDxChTxqSchedShapingProfile *profile)
{
    int rc = 0;
    struct prvCpssDxChTxqSchedAddressSpace * addressSpacePtr = NULL;

#if 0
    TM_REGISTER_VAR(ClvlTokenBucketTokenEnDiv)
    TM_REGISTER_VAR(ClvlTokenBucketBurstSize)
#endif
    TXQ_SCHED_DEVDEPEND_REGISTER_VAR(AlvlTokenBucketBurstSize)
    TXQ_SCHED_DEVDEPEND_REGISTER_VAR(BlvlTokenBucketBurstSize)
    TXQ_SCHED_DEVDEPEND_REGISTER_VAR(QueueTokenBucketBurstSize)

    TXQ_SCHED_DEVDEPEND_REGISTER_VAR(AlvlTokenBucketTokenEnDiv)
    TXQ_SCHED_DEVDEPEND_REGISTER_VAR(BlvlTokenBucketTokenEnDiv)
    TXQ_SCHED_DEVDEPEND_REGISTER_VAR(QueueTokenBucketTokenEnDiv)

    TM_CTL(ctl, hndl)

    rc =prvSchedAdressSpacePointerGet(hndl,&addressSpacePtr);
    if (rc)goto out;

    if(level >=SCHED_C_LEVEL&&level <=SCHED_P_LEVEL)
       {
            /*No need to configure shaping on those levels on Falcon.Only Q , A and B levels*/
           return 0;
       }


    /*  profile <--> level conformance test */
    if ((profile->level !=level) && (profile->level != ALL_LEVELS))
    {
        rc = -EFAULT;
        goto out;
    }

    switch (level)
    {
    case SCHED_Q_LEVEL:
        NODE_VALIDATION(ctl->tm_total_queues)
        if (rc) goto out;
        /* profile assignment */
        /* reset register reserved fields */
        TXQ_SCHED_REGISTER_DEVDEPEND_RESET(QueueTokenBucketTokenEnDiv)
        if (rc) goto out;
        /* assign register fields */
        TXQ_SCHED_DEVDEPEND_FIELD_PARSE_SET(QueueTokenBucketTokenEnDiv , MinDivExp ,   profile->min_div_exp);
        TXQ_SCHED_DEVDEPEND_FIELD_PARSE_SET(QueueTokenBucketTokenEnDiv , MaxDivExp ,   profile->max_div_exp);

        TXQ_SCHED_DEVDEPEND_FIELD_PARSE_SET(QueueTokenBucketTokenEnDiv , MinToken ,   profile->min_token )
        TXQ_SCHED_DEVDEPEND_FIELD_PARSE_SET(QueueTokenBucketTokenEnDiv , MaxToken ,   profile->max_token )
        TXQ_SCHED_DEVDEPEND_FIELD_PARSE_SET(QueueTokenBucketTokenEnDiv , MinTokenRes ,profile->min_token_res*ctl->level_data[SCHED_Q_LEVEL].token_res_exp)
        TXQ_SCHED_DEVDEPEND_FIELD_PARSE_SET(QueueTokenBucketTokenEnDiv , MaxTokenRes ,profile->max_token_res*ctl->level_data[SCHED_Q_LEVEL].token_res_exp)

        /* write register */
        TXQ_SCHED_WRITE_TABLE_REGISTER_DEVDEPEND(addressSpacePtr->Sched.QueueTokenBucketTokenEnDiv, node_ind, QueueTokenBucketTokenEnDiv)
        if (rc) goto out;

        /* reset register reserved fields */
        TXQ_SCHED_REGISTER_DEVDEPEND_RESET(QueueTokenBucketBurstSize)
        if (rc) goto out;
        /* assign register fields */
        TXQ_SCHED_DEVDEPEND_FIELD_PARSE_SET(QueueTokenBucketBurstSize , MaxBurstSz , profile->max_burst_size);
        TXQ_SCHED_DEVDEPEND_FIELD_PARSE_SET(QueueTokenBucketBurstSize , MinBurstSz , profile->min_burst_size);
        /* write register */
        TXQ_SCHED_WRITE_TABLE_REGISTER_DEVDEPEND(addressSpacePtr->Sched.QueueTokenBucketBurstSize, node_ind,  QueueTokenBucketBurstSize)
        if (rc) goto out;
        break;

    case SCHED_A_LEVEL:
        NODE_VALIDATION(ctl->tm_total_a_nodes)
        if (rc) goto out;
        /* profile assignment */
        /* reset register reserved fields */
        TXQ_SCHED_REGISTER_DEVDEPEND_RESET(AlvlTokenBucketTokenEnDiv)
        if (rc) goto out;
        /* assign register fields */
        TXQ_SCHED_DEVDEPEND_FIELD_PARSE_SET(AlvlTokenBucketTokenEnDiv , MinDivExp ,    profile->min_div_exp)
        TXQ_SCHED_DEVDEPEND_FIELD_PARSE_SET(AlvlTokenBucketTokenEnDiv , MaxDivExp ,    profile->max_div_exp)


        TXQ_SCHED_DEVDEPEND_FIELD_PARSE_SET(AlvlTokenBucketTokenEnDiv , MinToken ,  profile->min_token )
        TXQ_SCHED_DEVDEPEND_FIELD_PARSE_SET(AlvlTokenBucketTokenEnDiv , MaxToken ,     profile->max_token )
        TXQ_SCHED_DEVDEPEND_FIELD_PARSE_SET(AlvlTokenBucketTokenEnDiv , MinTokenRes ,   profile->min_token_res*ctl->level_data[SCHED_A_LEVEL].token_res_exp)
        TXQ_SCHED_DEVDEPEND_FIELD_PARSE_SET(AlvlTokenBucketTokenEnDiv , MaxTokenRes ,   profile->max_token_res*ctl->level_data[SCHED_A_LEVEL].token_res_exp)



        /* write register */
        TXQ_SCHED_WRITE_TABLE_REGISTER_DEVDEPEND(addressSpacePtr->Sched.AlvlTokenBucketTokenEnDiv, node_ind, AlvlTokenBucketTokenEnDiv)
        if (rc) goto out;

        /* reset register reserved fields */
        TXQ_SCHED_REGISTER_DEVDEPEND_RESET( AlvlTokenBucketBurstSize)
        if (rc) goto out;
        /* assign register fields */
        TXQ_SCHED_DEVDEPEND_FIELD_PARSE_SET(AlvlTokenBucketBurstSize , MaxBurstSz , profile->max_burst_size)
        TXQ_SCHED_DEVDEPEND_FIELD_PARSE_SET(AlvlTokenBucketBurstSize , MinBurstSz , profile->min_burst_size)
        /* write register */
        TXQ_SCHED_WRITE_TABLE_REGISTER_DEVDEPEND(addressSpacePtr->Sched.AlvlTokenBucketBurstSize, node_ind, AlvlTokenBucketBurstSize)
        if (rc) goto out;
        break;

    case SCHED_B_LEVEL:
        NODE_VALIDATION(ctl->tm_total_b_nodes)
        if (rc) goto out;
        /* profile assignment */
        /* reset register reserved fields */
        TXQ_SCHED_REGISTER_DEVDEPEND_RESET(BlvlTokenBucketTokenEnDiv)
        if (rc) goto out;
        /* assign register fields */
        TXQ_SCHED_DEVDEPEND_FIELD_PARSE_SET(BlvlTokenBucketTokenEnDiv , MinDivExp ,    profile->min_div_exp)
        TXQ_SCHED_DEVDEPEND_FIELD_PARSE_SET(BlvlTokenBucketTokenEnDiv , MaxDivExp ,    profile->max_div_exp)

        TXQ_SCHED_DEVDEPEND_FIELD_PARSE_SET(BlvlTokenBucketTokenEnDiv , MinToken ,     profile->min_token)
        TXQ_SCHED_DEVDEPEND_FIELD_PARSE_SET(BlvlTokenBucketTokenEnDiv , MaxToken ,     profile->max_token )
        TXQ_SCHED_DEVDEPEND_FIELD_PARSE_SET(BlvlTokenBucketTokenEnDiv , MinTokenRes ,profile->min_token_res*ctl->level_data[SCHED_B_LEVEL].token_res_exp)
        TXQ_SCHED_DEVDEPEND_FIELD_PARSE_SET(BlvlTokenBucketTokenEnDiv , MaxTokenRes ,profile->max_token_res*ctl->level_data[SCHED_B_LEVEL].token_res_exp)

        /* write register */
        TXQ_SCHED_WRITE_TABLE_REGISTER_DEVDEPEND(addressSpacePtr->Sched.BlvlTokenBucketTokenEnDiv, node_ind, BlvlTokenBucketTokenEnDiv)
        if (rc) goto out;
        /* reset register reserved fields */
        TXQ_SCHED_REGISTER_DEVDEPEND_RESET(BlvlTokenBucketBurstSize)
        if (rc) goto out;
        /* assign register fields */
        TXQ_SCHED_DEVDEPEND_FIELD_PARSE_SET(BlvlTokenBucketBurstSize , MaxBurstSz , profile->max_burst_size)
        TXQ_SCHED_DEVDEPEND_FIELD_PARSE_SET(BlvlTokenBucketBurstSize , MinBurstSz , profile->min_burst_size)
        /* write register */
        TXQ_SCHED_WRITE_TABLE_REGISTER_DEVDEPEND(addressSpacePtr->Sched.BlvlTokenBucketBurstSize, node_ind,  BlvlTokenBucketBurstSize)
        if (rc) goto out;
        break;
#if 0

    case SCHED_C_LEVEL:
        NODE_VALIDATION(ctl->tm_total_c_nodes)
        if (rc) goto out;
        /* profile assignment */
        /* reset register reserved fields */
        TM_RESET_TABLE_REGISTER(addressSpacePtr->Sched.ClvlTokenBucketTokenEnDiv, node_ind, ClvlTokenBucketTokenEnDiv)
        if (rc) goto out;
        /* assign register fields */
        TXQ_SCHED_REGISTER_SET(ClvlTokenBucketTokenEnDiv , MinDivExp ,    profile->min_div_exp)
        TXQ_SCHED_REGISTER_SET(ClvlTokenBucketTokenEnDiv , MaxDivExp ,    profile->max_div_exp)

        TXQ_SCHED_REGISTER_SET(ClvlTokenBucketTokenEnDiv , MinToken ,   profile->min_token )
        TXQ_SCHED_REGISTER_SET(ClvlTokenBucketTokenEnDiv , MaxToken ,   profile->max_token )
        TXQ_SCHED_REGISTER_SET(ClvlTokenBucketTokenEnDiv , MinTokenRes ,profile->min_token_res*ctl->level_data[SCHED_C_LEVEL].token_res_exp)
        TXQ_SCHED_REGISTER_SET(ClvlTokenBucketTokenEnDiv , MaxTokenRes ,profile->max_token_res*ctl->level_data[SCHED_C_LEVEL].token_res_exp)

/*
        let prevent case that  eligible function core is changed (in elig_function table)   ,but clients will not be updated
        set PerEn  always enabled for nodes
        TM_REGISTER_SET(ClvlTokenBucketTokenEnDiv , PerEn ,        1)
*/
        /* write register */
        TXQ_SCHED_WRITE_TABLE_REGISTER(addressSpacePtr->Sched.ClvlTokenBucketTokenEnDiv, node_ind, ClvlTokenBucketTokenEnDiv)
        if (rc) goto out;
        /* reset register reserved fields */
        TM_RESET_TABLE_REGISTER(addressSpacePtr->Sched.ClvlTokenBucketBurstSize, node_ind, ClvlTokenBucketBurstSize)
        if (rc) goto out;
        /* assign register fields */
        TXQ_SCHED_REGISTER_SET(ClvlTokenBucketBurstSize , MaxBurstSz , profile->max_burst_size)
        TXQ_SCHED_REGISTER_SET(ClvlTokenBucketBurstSize , MinBurstSz , profile->min_burst_size)
        /* write register */
        TXQ_SCHED_WRITE_TABLE_REGISTER(addressSpacePtr->Sched.ClvlTokenBucketBurstSize, node_ind, ClvlTokenBucketBurstSize)
        if (rc) goto out;
        break;
#endif
    default:
        rc = -EFAULT;
        break;
    }
out:
    return rc;
}



int prvSchedLowLevelSetPortShapingParametersToHw(PRV_CPSS_SCHED_HANDLE hndl, uint8_t port_ind)
{
    #if 0
    int rc =  -EFAULT;
    struct tm_port *port = NULL;
    #endif

    /* avoid compilation errors/warnings */
    hndl = hndl;
    port_ind = port_ind;

    /*no shaping on ports*/
      return 0;

#if 0

    TM_REGISTER_VAR(TM_Sched_PortTokenBucketTokenEnDiv)
    TM_REGISTER_VAR(TM_Sched_PortTokenBucketBurstSize)

    TM_CTL(ctl, hndl)






    if (port_ind < ctl->tm_total_ports)
    {
        port = &(ctl->tm_port_array[port_ind]);

        /* reset register reserved fields */
        TM_RESET_TABLE_REGISTER(addressSpacePtr->Sched.PortTokenBucketTokenEnDiv , port_ind , TM_Sched_PortTokenBucketTokenEnDiv)
        if (rc) goto out;
        /* assign register fields */
        TXQ_SCHED_REGISTER_SET(TM_Sched_PortTokenBucketTokenEnDiv , Periods ,      port->periods)


        TXQ_SCHED_REGISTER_SET(TM_Sched_PortTokenBucketTokenEnDiv , MinToken ,      port->cir_token)
        TXQ_SCHED_REGISTER_SET(TM_Sched_PortTokenBucketTokenEnDiv , MaxToken ,      port->eir_token )
        TXQ_SCHED_REGISTER_SET(TM_Sched_PortTokenBucketTokenEnDiv , MinTokenRes ,   ctl->level_data[SCHED_P_LEVEL].token_res_exp)
        TXQ_SCHED_REGISTER_SET(TM_Sched_PortTokenBucketTokenEnDiv , MaxTokenRes ,   ctl->level_data[SCHED_P_LEVEL].token_res_exp)


        /* write register */
        TXQ_SCHED_WRITE_TABLE_REGISTER(addressSpacePtr->Sched.PortTokenBucketTokenEnDiv , port_ind , TM_Sched_PortTokenBucketTokenEnDiv)
        if (rc) goto out;

        /* reset register reserved fields */
        TM_RESET_TABLE_REGISTER(addressSpacePtr->Sched.PortTokenBucketBurstSize , port_ind , TM_Sched_PortTokenBucketBurstSize)
        if (rc) goto out;
        /* assign register fields */
        TXQ_SCHED_REGISTER_SET(TM_Sched_PortTokenBucketBurstSize , MaxBurstSz , port->eir_burst_size)
        TXQ_SCHED_REGISTER_SET(TM_Sched_PortTokenBucketBurstSize , MinBurstSz , port->cir_burst_size)
        /* write register */
        TXQ_SCHED_WRITE_TABLE_REGISTER(addressSpacePtr->Sched.PortTokenBucketBurstSize , port_ind , TM_Sched_PortTokenBucketBurstSize)
        if (rc) goto out;
    }
out:
    return rc;
#endif
}

/* assume here that shaping profile is valid */
int prvSchedLowLevelGetNodeShapingParametersFromHw
(
    PRV_CPSS_SCHED_HANDLE hndl,
    enum schedLevel level,
    uint32_t node_ind,
    struct prvCpssDxChTxqSchedShapingProfile *profile
 )
{
    int rc = 0;

    struct prvCpssDxChTxqSchedAddressSpace * addressSpacePtr = NULL;

    TXQ_SCHED_DEVDEPEND_REGISTER_VAR(AlvlTokenBucketTokenEnDiv)
    TXQ_SCHED_DEVDEPEND_REGISTER_VAR(BlvlTokenBucketTokenEnDiv)
    TXQ_SCHED_DEVDEPEND_REGISTER_VAR(QueueTokenBucketTokenEnDiv)
    TXQ_SCHED_DEVDEPEND_REGISTER_VAR(AlvlTokenBucketBurstSize)
    TXQ_SCHED_DEVDEPEND_REGISTER_VAR(BlvlTokenBucketBurstSize)
    TXQ_SCHED_DEVDEPEND_REGISTER_VAR(QueueTokenBucketBurstSize)

    TM_CTL(ctl, hndl)

    rc =prvSchedAdressSpacePointerGet(hndl,&addressSpacePtr);
    if (rc)goto out;

    switch (level)
    {
    case SCHED_Q_LEVEL:
        NODE_VALIDATION(ctl->tm_total_queues)
        if (rc) goto out;

        TXQ_SCHED_READ_TABLE_REGISTER_DEVDEPEND(addressSpacePtr->Sched.QueueTokenBucketTokenEnDiv, node_ind,QueueTokenBucketTokenEnDiv)
        if (rc) goto out;
        /* assign register fields */
        TXQ_SCHED_DEVDEPEND_FIELD_PARSE_GET(QueueTokenBucketTokenEnDiv, MinDivExp,   profile->min_div_exp , (uint8_t))
        TXQ_SCHED_DEVDEPEND_FIELD_PARSE_GET(QueueTokenBucketTokenEnDiv, MinToken , profile->min_token , (uint16_t))
        TXQ_SCHED_DEVDEPEND_FIELD_PARSE_GET(QueueTokenBucketTokenEnDiv, MinTokenRes , profile->min_token_res , (uint8_t))

        TXQ_SCHED_DEVDEPEND_FIELD_PARSE_GET(QueueTokenBucketTokenEnDiv, MaxDivExp,   profile->max_div_exp , (uint8_t))
        TXQ_SCHED_DEVDEPEND_FIELD_PARSE_GET(QueueTokenBucketTokenEnDiv, MaxToken , profile->max_token , (uint16_t))
        TXQ_SCHED_DEVDEPEND_FIELD_PARSE_GET(QueueTokenBucketTokenEnDiv, MaxTokenRes , profile->max_token_res , (uint8_t))


        TXQ_SCHED_READ_TABLE_REGISTER_DEVDEPEND(addressSpacePtr->Sched.QueueTokenBucketBurstSize, node_ind,QueueTokenBucketBurstSize)
        if (rc) goto out;
        /* assign register fields */
        TXQ_SCHED_DEVDEPEND_FIELD_PARSE_GET(QueueTokenBucketBurstSize, MinBurstSz,   profile->min_burst_size , (uint16_t))
        TXQ_SCHED_DEVDEPEND_FIELD_PARSE_GET(QueueTokenBucketBurstSize, MaxBurstSz,   profile->max_burst_size , (uint16_t))

        break;

      case SCHED_A_LEVEL:
        NODE_VALIDATION(ctl->tm_total_a_nodes)
        if (rc) goto out;

        TXQ_SCHED_READ_TABLE_REGISTER_DEVDEPEND(addressSpacePtr->Sched.AlvlTokenBucketTokenEnDiv, node_ind,AlvlTokenBucketTokenEnDiv)
        if (rc) goto out;
        /* assign register fields */
        TXQ_SCHED_DEVDEPEND_FIELD_PARSE_GET(AlvlTokenBucketTokenEnDiv, MinDivExp,   profile->min_div_exp , (uint8_t))
        TXQ_SCHED_DEVDEPEND_FIELD_PARSE_GET(AlvlTokenBucketTokenEnDiv, MinToken , profile->min_token , (uint16_t))
        TXQ_SCHED_DEVDEPEND_FIELD_PARSE_GET(AlvlTokenBucketTokenEnDiv, MinTokenRes , profile->min_token_res , (uint8_t))


        TXQ_SCHED_READ_TABLE_REGISTER_DEVDEPEND(addressSpacePtr->Sched.AlvlTokenBucketBurstSize, node_ind,AlvlTokenBucketBurstSize)
        if (rc) goto out;
        /* assign register fields */
        TXQ_SCHED_DEVDEPEND_FIELD_PARSE_GET(AlvlTokenBucketBurstSize, MinBurstSz,   profile->min_burst_size , (uint16_t))

        break;
       case SCHED_B_LEVEL:
        NODE_VALIDATION(ctl->tm_total_b_nodes)
        if (rc) goto out;

        TXQ_SCHED_READ_TABLE_REGISTER_DEVDEPEND(addressSpacePtr->Sched.BlvlTokenBucketTokenEnDiv, node_ind,BlvlTokenBucketTokenEnDiv)
        if (rc) goto out;
        /* assign register fields */
        TXQ_SCHED_DEVDEPEND_FIELD_PARSE_GET(BlvlTokenBucketTokenEnDiv, MinDivExp,   profile->min_div_exp , (uint8_t))
        TXQ_SCHED_DEVDEPEND_FIELD_PARSE_GET(BlvlTokenBucketTokenEnDiv, MinToken , profile->min_token , (uint16_t))
        TXQ_SCHED_DEVDEPEND_FIELD_PARSE_GET(BlvlTokenBucketTokenEnDiv, MinTokenRes , profile->min_token_res , (uint8_t))


        TXQ_SCHED_READ_TABLE_REGISTER_DEVDEPEND(addressSpacePtr->Sched.BlvlTokenBucketBurstSize, node_ind,BlvlTokenBucketBurstSize)
        if (rc) goto out;
        /* assign register fields */
        TXQ_SCHED_DEVDEPEND_FIELD_PARSE_GET(BlvlTokenBucketBurstSize, MinBurstSz,   profile->min_burst_size , (uint16_t))
        break;


    default:
        rc = -EFAULT;
        break;
    }
out:
    return rc;
}

