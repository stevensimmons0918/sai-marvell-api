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
 * @brief prvCpssDxChTxqSchedShadow implementation.
 *
* @file prvCpssDxChTxqSchedShadow.c
*
* $Revision: 2.0 $
 */



#include <cpss/dxCh/dxChxGen/txq/private/Sched/core/prvSchedCtlInternal.h>
#include <cpss/dxCh/dxChxGen/txq/private/Sched/core/prvSchedSetLocalDbDefaults.h>
#include <errno.h>
#include <cpss/dxCh/dxChxGen/txq/private/Sched/core/prvSchedOsInterface.h>
#include <cpss/dxCh/dxChxGen/txq/private/Sched/core/prvRmCtl.h>
#include <cpss/dxCh/dxChxGen/txq/private/Sched/prvSchedNodesCtl.h>
#include <cpss/dxCh/dxChxGen/txq/private/Sched/core/prvSchedNodesCtlInternal.h>
#include <cpss/dxCh/dxChxGen/txq/private/Sched/core/prvSchedHwConfigurationInterface.h>
#include <cpss/dxCh/dxChxGen/txq/private/Sched/prvSchedEligPrioFunc.h>
#include <cpss/dxCh/dxChxGen/txq/private/Sched/prvSchedErrCodes.h>
#include <cpss/generic/cpssHwInit/private/prvCpssHwRegisters.h>
#include <cpss/dxCh/dxChxGen/txq/private/Sched/core/prvSchedGetGenParamInterface.h>
#include <cpss/dxCh/dxChxGen/txq/private/Sched/core/prvSchedShapingInternal.h>
#include <cpss/dxCh/dxChxGen/txq/private/Sched/core/prvSetHwRegisters.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>


#include <stdlib.h>
#include <stdio.h>

#define TXQ_SCHED_BUILD_DEFAULT_TREE_MAC

#define PRV_SIP6_SCHED_FREE(var)  if (var) { schedFree(var) ; var = NULL ;}


#define goto while(cpssOsPrintf("out: %s(%d)\n", __FILE__, __LINE__) || 1) goto

extern GT_STATUS  prvCpssSip6TxqPdqSchedulerHwParamsByLevelGet
(
    IN GT_U32 devNum,
    OUT  GT_U32 *maxPnodePtr,
    OUT  GT_U32 *maxCnodePtr,
    OUT  GT_U32 *maxBnodePtr,
    OUT  GT_U32 *maxAnodePtr,
    OUT  GT_U32 *maxQnodePtr
);

int prvSchedLibHwInit(PRV_CPSS_SCHED_HANDLE hndl)
{
    int rc,data;
    GT_U32 maxPnodes,maxQueues,maxAnodes,maxBnodes,maxCnodes;
    GT_U8 devNum;
    CPSS_SYSTEM_RECOVERY_INFO_STC tempSystemRecovery_Info;
    TM_CTL(ctl, hndl);

   /* Get general parameters : LAD frequency,  dwrr params  - they are readed from H/W resources*/
    rc = prvSchedGetGenParams(ctl);
    if (rc) goto out;

    data = prvCpssSchedDevNumGet(TM_ENV(ctl));

    if(-1!=data)
    {
        devNum = (GT_U8)data;
    }
    else
    {
        return data;
    }

    /* On initialization we need to initialize all HW ,so we will use HW defenition in order to get size of nodes*/

    rc = prvCpssSip6TxqPdqSchedulerHwParamsByLevelGet(devNum,&maxPnodes,&maxCnodes,&maxBnodes,&maxAnodes,&maxQueues);
    if (rc)  goto out;

    /*in case of HS only update SW and return */
    cpssSystemRecoveryStateGet(&tempSystemRecovery_Info);
    if (tempSystemRecovery_Info.systemRecoveryProcess == CPSS_SYSTEM_RECOVERY_PROCESS_HITLESS_STARTUP_E )
    {
        rc = prvSchedConfigEligPrioFuncTable((PRV_CPSS_SCHED_HANDLE)ctl,0); /* with no H/W update*/
        return rc;
    }

    rc = prvSchedConfigEligPrioFuncTable((PRV_CPSS_SCHED_HANDLE)ctl,1); /* with H/W update*/
    if (rc)  goto out;

    rc = prvSchedLowLevelDisablePortEligPrioFunc((PRV_CPSS_SCHED_HANDLE) ctl, maxPnodes);
    if (rc) goto out;


#if defined (TXQ_SCHED_BUILD_DEFAULT_TREE_MAC)

     rc = prvSchedLowLevelDisableQlevelEligPrioFunc((PRV_CPSS_SCHED_HANDLE) ctl, maxQueues);
     if (rc) goto out;

     rc = prvSchedLowLevelDisableAlevelEligPrioFunc((PRV_CPSS_SCHED_HANDLE) ctl, maxAnodes);
     if (rc) goto out;

     rc = prvSchedLowLevelDisableBlevelEligPrioFunc((PRV_CPSS_SCHED_HANDLE) ctl, maxBnodes);
     if (rc) goto out;

     rc = prvSchedLowLevelDisableClevelEligPrioFunc((PRV_CPSS_SCHED_HANDLE) ctl,maxCnodes);
     if (rc) goto out;

     rc = prvSchedLowLevelSetDefaultPortMapping((PRV_CPSS_SCHED_HANDLE) ctl,maxPnodes);
     if (rc) goto out;

     rc = prvSchedLowLevelSetDefaultCnodeMapping((PRV_CPSS_SCHED_HANDLE) ctl, maxCnodes);
     if (rc) goto out;

     rc = prvSchedLowLevelSetDefaultBnodeMapping((PRV_CPSS_SCHED_HANDLE) ctl, maxBnodes);
     if (rc) goto out;

     rc = prvSchedLowLevelSetDefaultAnodeMapping((PRV_CPSS_SCHED_HANDLE) ctl, maxAnodes);
     if (rc) goto out;

     rc = prvSchedLowLevelSetDefaultQnodeMapping((PRV_CPSS_SCHED_HANDLE) ctl, maxQueues);
     if (rc) goto out;


#endif

out:
  return rc;
}



/**
 */
int prvSchedLibSwInit(PRV_CPSS_SCHED_HANDLE hEnv, PRV_CPSS_SCHED_HANDLE *htm, struct PRV_CPSS_SCHED_LIB_INIT_PARAMS_STC *init_params_PTR)
{
    struct prvCpssDxChTxqSchedShadow *ctl = NULL;

    int rc = 0;
    unsigned int i;
    unsigned int j;


    if ((*htm) != NULL) return  -EINVAL;

    *htm = NULL;

    if (prvCpssSchedInitHardwareConfiguration(hEnv))  return  TM_CONF_INVALID_PROD_NAME;

    /* Allocate handle */
    ctl = schedMalloc(sizeof(*ctl) * 1);
    if (!ctl) return -GT_OUT_OF_CPU_MEM;

    schedMemSet(ctl, 0, sizeof(*ctl));
    ctl->magic = TM_MAGIC;
    ctl->hEnv=hEnv;
#ifdef VIRTUAL_LAYER
    ctl->hVirtualLayer=NULL;
#endif

    ctl->tm_queue_array = NULL;
    ctl->tm_a_node_array = NULL;
    ctl->tm_b_node_array = NULL;
    ctl->tm_c_node_array = NULL;
    ctl->tm_port_array = NULL;


    /* tm lib init params */
    if (init_params_PTR)
        schedMemCpy(&ctl->init_params, init_params_PTR, sizeof(struct PRV_CPSS_SCHED_LIB_INIT_PARAMS_STC));

    ctl->init_params.schedLibMtu = MAX(SCHED_MTU, ctl->init_params.schedLibMtu);
    /* calculate quantum limits */

   /* Get general parameters : LAD frequency,  dwrr params  - they are readed from H/W resources*/
    rc = prvSchedLowLevelGetGeneralHwParams(ctl);
    if (rc) goto out;

    /* Total number of nodes per level */
    ctl->tm_total_ports =   prvCpssSchedGetPortCount(hEnv);
    ctl->tm_total_c_nodes = prvCpssSchedGetCnodesCount(hEnv);
    ctl->tm_total_b_nodes = prvCpssSchedGetBnodesCount(hEnv);
    ctl->tm_total_a_nodes = prvCpssSchedGetAnodesCount(hEnv);
    ctl->tm_total_queues =  prvCpssSchedGetQNodesCount(hEnv);


    ctl->rm = prvSchedRmOpen(ctl->tm_total_c_nodes, ctl->tm_total_b_nodes, ctl->tm_total_a_nodes, ctl->tm_total_queues);
    if (!ctl->rm)
    {
        rc = -EINVAL;
        goto out;
    }

    /* allocate global arrays */
    ctl->tm_port_array = schedMalloc(sizeof(struct tm_port) * ctl->tm_total_ports);
    if (ctl->tm_port_array == NULL)
    {
        rc = -GT_OUT_OF_CPU_MEM;
        goto out;
    }
    ctl->tm_c_node_array = schedMalloc(sizeof(struct tm_c_node) * ctl->tm_total_c_nodes);
    if (ctl->tm_c_node_array == NULL)
    {
        rc = -GT_OUT_OF_CPU_MEM;
        goto out;
    }
    ctl->tm_b_node_array = schedMalloc(sizeof(struct tm_b_node) * ctl->tm_total_b_nodes);
    if (ctl->tm_b_node_array == NULL)
    {
        rc = -GT_OUT_OF_CPU_MEM;
        goto out;
    }
    ctl->tm_a_node_array = schedMalloc(sizeof(struct tm_a_node) * ctl->tm_total_a_nodes);
    if (ctl->tm_a_node_array == NULL)
    {
        rc = -GT_OUT_OF_CPU_MEM;
        goto out;
    }
    ctl->tm_queue_array = schedMalloc(sizeof(struct tm_queue) * ctl->tm_total_queues);
    if (ctl->tm_queue_array == NULL)
    {
        rc = -GT_OUT_OF_CPU_MEM;
        goto out;
    }

    /* set default values for tree nodes INCLUDING DEFAULT MAPPING INITIALIZATION (second parameter is 1 ) */
    for (i=0 ; i < ctl->tm_total_ports; i++)
        prvSchedSetSwPortDefault(&(ctl->tm_port_array[i]),1);
    for (i=0 ; i < ctl->tm_total_c_nodes; i++)
        prvSchedSetSw_c_node_Default(&(ctl->tm_c_node_array[i]),1);
    for (i=0 ; i < ctl->tm_total_b_nodes; i++)
        prvSchedSetSw_b_node_Default(&(ctl->tm_b_node_array[i]),1);
    for (i=0 ; i < ctl->tm_total_a_nodes; i++)
        prvSchedSetSw_a_node_Default(&(ctl->tm_a_node_array[i]),1);
    for (i=0 ; i < ctl->tm_total_queues; i++)
        prvSchedSetSw_queue_Default(&(ctl->tm_queue_array[i]),1);



    /* Shaping Profile #0 is reserved in the library and represents no
     * shaping. If the user application doesn't want to apply shaping
     * for a node the SCHED_INF_SHP_PROFILE must be used for
     * shaping profile referece */
    rc=prvCpssSchedShapingInitInfiniteShapingProfile((PRV_CPSS_SCHED_HANDLE)ctl);
    if (rc) goto out;

#ifdef USE_DEFAULT_CURVES
    /* Drop Profile #0 & WRED Curve #0 are reserved in the library and
     * represent no drop. If the user application doesn't want to
     * apply drop for a node the TM_NO_DROP_PROFILE must be used for
     * drop profile referece */
    /* Create default curves & drop profiles for each level */
#else
   /* Drop Profile #0 is reserved in the library and
     * represent no drop. If the user application doesn't want to
     * apply drop for a node the TM_NO_DROP_PROFILE must be used for
     * drop profile referece */
    /* Create default drop profiles for each level */
#endif




    /* scheduling configuration */
    rc = prvSchedSetSwConfDefault((PRV_CPSS_SCHED_HANDLE)ctl);
    if (rc) goto out;

    /* other scheduler registers */
    rc = prvSchedSetSwGenConfDefault((PRV_CPSS_SCHED_HANDLE)ctl);
    if (rc) goto out;

    rc = prvSchedConfigEligPrioFuncTable((PRV_CPSS_SCHED_HANDLE)ctl,0); /* without H/W update*/
    if (rc) goto out;

    ctl->node_quantum_chunk_size_bytes = prvCpssSchedGetNodeQuantumChunkSize();
    ctl->port_quantum_chunk_size_bytes = prvCpssSchedGetPortQuantumChunkSize();

    /*
    min_port_quant = 4*ctl->port_ch_emit*ctl->dwrr_bytes_burst_limit
    max_pkg_len_bursts = (ctl->mtu + ctl->min_pkg_size)/16
    min_port_quantum_bytes= MAX (min_port_quant, max_pkg_len_bursts)
    min_port_quant_bursts = min_port_quantum_bytes / 0x40
    */

    /*
    ctl->min_pkg_size has updated at prvSchedLowLevelGetGeneralHwParams
    ctl->dwrr_bytes_burst_limit has updated at prvSchedSetSwConfDefault
    */

    /* port quantum limits */
    ctl->min_port_quantum_bytes = MAX((ctl->min_pkg_size + ctl->init_params.schedLibMtu)/*/16*/ ,(uint32_t)(4*ctl->port_ch_emit*ctl->dwrr_bytes_burst_limit));

    /* aligned port quantum limits */
    j = (ctl->min_port_quantum_bytes + ctl->port_quantum_chunk_size_bytes - 1) /  ctl->port_quantum_chunk_size_bytes;
    ctl->min_port_quantum_bytes = (j * ctl->port_quantum_chunk_size_bytes);

    ctl->max_port_quantum_bytes = prvCpssSchedGetPortMaxQuantumChunks()*prvCpssSchedGetPortQuantumChunkSize();

    /* node quantum limits */
    ctl->min_node_quantum_bytes = ctl->init_params.schedLibMtu + ctl->min_pkg_size;

    /* aligned node quantum limits */
    j = (ctl->min_node_quantum_bytes + ctl->node_quantum_chunk_size_bytes - 1) /  ctl->node_quantum_chunk_size_bytes;
    ctl->min_node_quantum_bytes = (j * ctl->node_quantum_chunk_size_bytes);

    ctl->max_node_quantum_bytes = ctl->min_node_quantum_bytes * prvCpssSchedGetNodeQuantumRange();
    if (ctl->max_node_quantum_bytes > prvCpssSchedGetNodeMaxQuantumChunks()*ctl->node_quantum_chunk_size_bytes)
        ctl->max_node_quantum_bytes = prvCpssSchedGetNodeMaxQuantumChunks()*ctl->node_quantum_chunk_size_bytes;


#ifdef VIRTUAL_LAYER
    rc = allocate_virtual_layer(&ctl->hVirtualLayer);
    if (rc < 0) goto out;

#else
    /* Reshuffling changes list */
    ctl->list.next = NULL;

#endif

    /* set the handle */
    *htm = (PRV_CPSS_SCHED_HANDLE)ctl;

    return rc;

out:
    prvSchedLibCloseSw(ctl);
    return rc;
}


/**original tm function is tm_lib_close_nolock
 */


int prvSchedLibCloseSw(PRV_CPSS_SCHED_HANDLE hndl)
{

    TM_CTL(ctl,hndl)

    prvSchedRmClose(ctl->rm);

    PRV_SIP6_SCHED_FREE(ctl->tm_queue_array);
    PRV_SIP6_SCHED_FREE(ctl->tm_a_node_array);
    PRV_SIP6_SCHED_FREE(ctl->tm_b_node_array);
    PRV_SIP6_SCHED_FREE(ctl->tm_c_node_array);
    PRV_SIP6_SCHED_FREE(ctl->tm_port_array);

    /* Clean reshuffling list */
    prvSchedCleanReshafflingList(ctl);

    PRV_SIP6_SCHED_FREE(ctl->hEnv);

   /* release TM lib handle */
    PRV_SIP6_SCHED_FREE(ctl);

    return 0;
}


