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
 * @brief tm_shaping implementation.
 *
* @file tm_shaping.c
*
* $Revision: 2.0 $
 */


#include <cpss/dxCh/dxChxGen/txq/private/Sched/prvSched.h>
#include <cpss/dxCh/dxChxGen/txq/private/Sched/prvSchedShaping.h>
#include <cpss/dxCh/dxChxGen/txq/private/Sched/prvSchedErrCodes.h>
#include <cpss/dxCh/dxChxGen/txq/private/Sched/core/prvSchedSetLocalDbDefaults.h>
#include <cpss/dxCh/dxChxGen/txq/private/Sched/core/prvSetHwRegisters.h>
#include <cpss/dxCh/dxChxGen/txq/private/Sched/core/prvSchedShapingUtils.h>
#include <cpss/dxCh/dxChxGen/txq/private/Sched/core/prvSchedNodesCtlInternal.h>
#include <cpss/dxCh/dxChxGen/txq/private/Sched/core/prvSchedOsInterface.h>
#include <errno.h>
#include <stdlib.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

/**
#define PRINT_SHAPING_RESULTS


*/


int prvCpssSchedShapingInitInfiniteShapingProfile(PRV_CPSS_SCHED_HANDLE hndl)
{
    TM_CTL(ctl, hndl)
    return prvSchedSetSwShapingProfileDefault(&(ctl->infiniteShapingProfile));
}



/* MACRO for update all layer nodes referenced to updated shaping profile */

#define UPDATE_PROFILE_CLIENTS(layer, layer_size) \
    i=0;\
    node_counter=0;\
    while ( (node_counter < sh_profile->use_counter) && (i < layer_size)) \
    {\
        if (prof_index==ctl->layer[i].shaping_profile_ref) \
        {\
            /* update node */\
            if (prvSchedLowLevelSetNodeShapingParametersToHwWithValidation(hndl, level, i) < 0)\
            {\
                rc = TM_HW_SHAPING_PROF_FAILED;\
                goto out;\
            }\
            node_counter++;\
        }\
        i++;\
    }\
/* fprintf(stderr,">>>>>>>>>>>>>>>>>>>>>>>>>>> %s        level=%d,  use_counter=%d ,  node_counter=%d,  index=%d\n",__FUNCTION__, level, sh_profile->use_counter , node_counter, i)     ; */


#define __RETURN_PROF_REF_OOR_ERROR_CODE \
{\
    switch (level)\
    {\
            case SCHED_Q_LEVEL: return TM_CONF_Q_SHAPING_PROF_REF_OOR; \
            case SCHED_A_LEVEL: return TM_CONF_A_SHAPING_PROF_REF_OOR;\
            case SCHED_B_LEVEL: return TM_CONF_B_SHAPING_PROF_REF_OOR;\
            case SCHED_C_LEVEL: return TM_CONF_C_SHAPING_PROF_REF_OOR;\
            default:  return TM_CONF_SHAPING_PROF_REF_OOR; /* not happend */\
    }\
}

int prvCpssSchedShapingCheckShapingProfileValidity( PRV_CPSS_SCHED_HANDLE hndl, uint32_t prof_index, enum schedLevel level)
{
    TM_CTL(ctl, hndl);

#ifdef PER_NODE_SHAPING
    /* direct shaping can't be input parameter - so profile is invalid for  create/update purposes */
    if (prof_index ==SCHED_DIRECT_NODE_SHAPING) __RETURN_PROF_REF_OOR_ERROR_CODE;
#endif
   if (prof_index==SCHED_INF_SHP_PROFILE)  return 0;
    __RETURN_PROF_REF_OOR_ERROR_CODE;

}


#define SET_SHAPING_PROFILE_TO_NODE(profile) \
    switch(level) \
    {\
        case SCHED_Q_LEVEL:ctl->tm_queue_array[node_index].shaping_profile_ref=profile; break;\
        case SCHED_A_LEVEL:ctl->tm_a_node_array[node_index].shaping_profile_ref=profile; break;\
        case SCHED_B_LEVEL:ctl->tm_b_node_array[node_index].shaping_profile_ref=profile; break;\
        case SCHED_C_LEVEL:ctl->tm_c_node_array[node_index].shaping_profile_ref=profile; break;\
        default: break;\
    }

#define GET_NODE_SHAPING_PROFILE(profile) \
    switch(level) \
    {\
        case SCHED_Q_LEVEL:profile = ctl->tm_queue_array[node_index].shaping_profile_ref; break;\
        case SCHED_A_LEVEL:profile = ctl->tm_a_node_array[node_index].shaping_profile_ref; break;\
        case SCHED_B_LEVEL:profile = ctl->tm_b_node_array[node_index].shaping_profile_ref; break;\
        case SCHED_C_LEVEL:profile = ctl->tm_c_node_array[node_index].shaping_profile_ref; break;\
        default:PRV_SCHED_ASSERT(ERANGE);\
    }





int prvCpssSchedShapingAddNodeToShapingProfile(PRV_CPSS_SCHED_HANDLE hndl,
                                uint32_t prof_index,
                                enum schedLevel level,
                                uint32_t node_index)
{

    TM_CTL(ctl, hndl);
    (void)node_index;
#ifdef PER_NODE_SHAPING
    /* this can't happend  */
    if (prof_index==SCHED_DIRECT_NODE_SHAPING)  return 0; /* direct shaping  -  node is not client  */
#endif
    if (prof_index==SCHED_INF_SHP_PROFILE)  return 0; /* default profile not updated, so it doesn't need clients */

    __RETURN_PROF_REF_OOR_ERROR_CODE;
}

int prvCpssSchedShapingRemoveNodeFromShapingProfile(PRV_CPSS_SCHED_HANDLE hndl,
                                uint32_t prof_index,
                                enum schedLevel level,
                                uint32_t node_index)
{

    TM_CTL(ctl, hndl);
    (void)node_index;
#ifdef PER_NODE_SHAPING
    if (prof_index==SCHED_DIRECT_NODE_SHAPING)  return 0; /* direct shaping  -  node is not client  */
#endif
    if (prof_index==SCHED_INF_SHP_PROFILE)  return 0; /* default profile not updated, so it doesn't need clients */

    __RETURN_PROF_REF_OOR_ERROR_CODE;
}



#ifdef PER_NODE_SHAPING

    #define COPY_NODE_SHAPING_PARAMS_TO_PTR(source_node, target_ptr) \
        target_ptr->cir_bw=source_node.cir_bw;\
        target_ptr->eir_bw=source_node.eir_bw;\
        target_ptr->cbs=source_node.cbs;\
        target_ptr->ebs=source_node.ebs;

    #define COPY_NODE_SHAPING_PARAMS_FROM_PTR(source_ptr, target_node) \
        target_node.cir_bw=source_ptr->cir_bw;\
        target_node.eir_bw=source_ptr->eir_bw;\
        target_node.cbs=source_ptr->cbs;\
        target_node.ebs=source_ptr->ebs;


    int prvCpssSchedShapingUpdateNodeShapingProc
  (
        PRV_CPSS_SCHED_HANDLE hndl,
        enum schedLevel level,
        uint32_t node_index,
        struct sched_shaping_profile_params * node_shaping_params,
        uint32_t * actual_cir_bw_received_ptr,
        uint32_t * actual_eir_bw_received_ptr
  )
    {
        int rc;
        uint16_t    node_shaping_profile;

        uint16_t    cir_token;
        uint16_t    eir_token;
        uint8_t cir_resolution;
        uint8_t eir_resolution;
        uint8_t min_div_exp;
        uint8_t max_div_exp;
        struct  prvCpssDxChTxqSchedShapingProfile tmp_shp_profile;
        uint16_t        cbs;
        uint16_t        ebs;
        double      cir_accuracy,eir_accuracy;
        uint32_t    providedBw = node_shaping_params->cir_bw;

        TM_CTL(ctl, hndl)


        /* Check node  validity */
        switch (level)
        {
            case SCHED_Q_LEVEL:
                VALIDATE_NODE_ALLOCATED(queue,node_index)
                break;
            case SCHED_A_LEVEL:
                VALIDATE_NODE_ALLOCATED(a_node,node_index)
                break;
            case SCHED_B_LEVEL:
                VALIDATE_NODE_ALLOCATED(b_node,node_index)
                break;
            case SCHED_C_LEVEL:
                VALIDATE_NODE_ALLOCATED(c_node,node_index)
               break;
            case SCHED_P_LEVEL:
                VALIDATE_NODE_ALLOCATED(port,node_index)
                break;
            default:
                rc = -ERANGE;
        }
        if (rc) goto out;

        /* Check shaping params validity */
        if ((node_shaping_params->cbs > TM_4M_kB) || (node_shaping_params->ebs > TM_4M_kB))
        {
            rc = -EFAULT;
            goto out;
        }
        /* calculate shaping  */
        cbs = node_shaping_params->cbs;
        ebs = node_shaping_params->ebs;

        rc = prvSchedShapingUtilsCalculateNodeShaping(
                                    PRV_SCHED_DEV_NUM_GET(ctl),
                                    1000.0 * node_shaping_params->cir_bw/ctl->level_data[level].unit,
                                    1000.0 * node_shaping_params->eir_bw/ctl->level_data[level].unit,
                                    &cbs,
                                    &ebs,
                                    (1 << ctl->level_data[level].token_res_exp),
                                    &min_div_exp,
                                    &max_div_exp,
                                    &cir_resolution,
                                    &eir_resolution,
                                    &cir_token,
                                    &eir_token,hndl,&cir_accuracy,&eir_accuracy);
        if (rc)
            goto out;

        PRINT_NODE_SHAPING_CALCULATION_RESULT(node_shaping_params->cir_bw ,node_shaping_params->eir_bw)
        /* if valid : */
        GET_NODE_SHAPING_PROFILE(node_shaping_profile);
        rc=prvCpssSchedShapingRemoveNodeFromShapingProfile(hndl, node_shaping_profile, level, node_index);
        if (rc) goto out;
        tmp_shp_profile.level=level;
        tmp_shp_profile.cir_bw=node_shaping_params->cir_bw;
        tmp_shp_profile.eir_bw=node_shaping_params->eir_bw;
        tmp_shp_profile.min_token = cir_token;
        tmp_shp_profile.min_burst_size = node_shaping_params->cbs;
        if(level == SCHED_A_LEVEL)
        {
            tmp_shp_profile.max_token = 0xFFF;/*no use in max token in Falcon*/
            tmp_shp_profile.max_burst_size = 0xFFF;/*no use in max token in Falcon*/
        }
        else
        {
            tmp_shp_profile.max_token = eir_token;
            tmp_shp_profile.max_burst_size = node_shaping_params->ebs;
        }
        tmp_shp_profile.min_div_exp = min_div_exp;
        tmp_shp_profile.max_div_exp = max_div_exp;
        tmp_shp_profile.min_token_res = cir_resolution;
        tmp_shp_profile.max_token_res = eir_resolution;

        /*Calculate actual bandwith*/
        if(actual_cir_bw_received_ptr)
        {   providedBw = node_shaping_params->cir_bw;
            providedBw += (uint32_t)(cir_accuracy*providedBw);
            *actual_cir_bw_received_ptr = providedBw;

            #ifdef PRINT_SHAPING_RESULTS
            schedPrintf("\nactual_cir_bw_received :%u  accuracy %lf \n", *actual_cir_bw_received_ptr ,accuracy);
#endif
         }

         if(actual_eir_bw_received_ptr)
        {   providedBw = node_shaping_params->eir_bw;
            providedBw += (uint32_t)(eir_accuracy*providedBw);
            *actual_eir_bw_received_ptr = providedBw;
            #ifdef PRINT_SHAPING_RESULTS
            schedPrintf("\nactual_eir_bw_received :%u  accuracy %lf \n", *actual_eir_bw_received_ptr ,accuracy);
            #endif
         }



        rc = prvSchedLowLevelSetNodeShapingParametersToHw(hndl, level, node_index, &tmp_shp_profile);
        /* Update SW DB */
        if (rc)
        {
            if(rc!=TM_CONF_PORT_IND_OOR)
            {
                rc = TM_HW_SHAPING_PROF_FAILED;
            }
            /* rollback  -return node to clients set   ??? */
             prvCpssSchedShapingAddNodeToShapingProfile(hndl, node_shaping_profile, level, node_index);
             goto out;
        }
        SET_SHAPING_PROFILE_TO_NODE(SCHED_DIRECT_NODE_SHAPING);
        /* store  node shaping parameters */
        switch(level)
        {
            case SCHED_Q_LEVEL:COPY_NODE_SHAPING_PARAMS_FROM_PTR(node_shaping_params ,ctl->tm_queue_array[node_index] ); break;
            case SCHED_A_LEVEL:COPY_NODE_SHAPING_PARAMS_FROM_PTR(node_shaping_params ,ctl->tm_a_node_array[node_index]); break;
            case SCHED_B_LEVEL:COPY_NODE_SHAPING_PARAMS_FROM_PTR(node_shaping_params ,ctl->tm_b_node_array[node_index]); break;
            case SCHED_C_LEVEL:COPY_NODE_SHAPING_PARAMS_FROM_PTR(node_shaping_params ,ctl->tm_c_node_array[node_index]); break;
            default: break;
        }
    out:
        return rc;
    }


    int prvCpssSchedShapingUpdateNodeShapingProRreadNodeShapingProc(PRV_CPSS_SCHED_HANDLE hndl,
                                  enum schedLevel level,
                                  uint32_t node_index,
                                  uint16_t * shaping_profile,
                                  struct sched_shaping_profile_params * node_shaping_params)
    {
        int rc;

        TM_CTL(ctl, hndl);



        /* Check node  validity */
        switch (level)
        {
        case SCHED_Q_LEVEL:
            if (node_index >= ctl->tm_total_queues) rc = -EFAULT;
            else if (ctl->tm_queue_array[node_index].mapping.nodeStatus != TM_NODE_USED)  rc = -ENODATA;
            else rc = 0;
            break;
        case SCHED_A_LEVEL:
            if (node_index >= ctl->tm_total_a_nodes) rc = -EFAULT;
            else if (ctl->tm_a_node_array[node_index].mapping.nodeStatus != TM_NODE_USED)  rc = -ENODATA;
            else rc = 0;
            break;
        case SCHED_B_LEVEL:
            if (node_index >= ctl->tm_total_b_nodes) rc = -EFAULT;
            else if (ctl->tm_b_node_array[node_index].mapping.nodeStatus != TM_NODE_USED)  rc = -ENODATA;
            else rc = 0;
            break;
        case SCHED_C_LEVEL:
            if (node_index >= ctl->tm_total_c_nodes) rc = -EFAULT;
            else if (ctl->tm_c_node_array[node_index].mapping.nodeStatus != TM_NODE_USED)  rc = -ENODATA;
            else rc = 0;
           break;
        default:
            rc = -ERANGE;
        }
        if (rc) goto out;

        if (* shaping_profile != SCHED_DIRECT_NODE_SHAPING)
        {
           GET_NODE_SHAPING_PROFILE(* shaping_profile);
        }

        if (* shaping_profile == SCHED_DIRECT_NODE_SHAPING)
        {
            /* fill  structure by shaping parameters */
            switch(level)
            {
                case SCHED_Q_LEVEL:COPY_NODE_SHAPING_PARAMS_TO_PTR(ctl->tm_queue_array[node_index] ,node_shaping_params); break;
                case SCHED_A_LEVEL:COPY_NODE_SHAPING_PARAMS_TO_PTR(ctl->tm_a_node_array[node_index],node_shaping_params); break;
                case SCHED_B_LEVEL:COPY_NODE_SHAPING_PARAMS_TO_PTR(ctl->tm_b_node_array[node_index],node_shaping_params); break;
                case SCHED_C_LEVEL:COPY_NODE_SHAPING_PARAMS_TO_PTR(ctl->tm_c_node_array[node_index],node_shaping_params); break;
                default: break;
            }
        }

    out:
        return rc;
    }

 int prvCpssSchedShapingUpdateNodeShaping
  (
         PRV_CPSS_SCHED_HANDLE hndl,
         enum schedLevel level,
         uint32_t node_index,
         struct sched_shaping_profile_params * node_shaping_params,
         uint32_t * actual_cir_bw_received_ptr,
         uint32_t * actual_eir_bw_received_ptr
   )
    {

        DECLARE_TM_CTL_PTR(ctl, hndl);

        (void)ctl; /* prevent warning if virtual layer is not used */
        CONVERT_TO_PHYSICAL(level,node_index);
        return prvCpssSchedShapingUpdateNodeShapingProc( hndl, level, node_index, node_shaping_params,actual_cir_bw_received_ptr,actual_eir_bw_received_ptr);
    }


    int prvCpssSchedReadNodeShapingConfigFromShadow(PRV_CPSS_SCHED_HANDLE hndl,
                                  enum schedLevel level,
                                  uint32_t node_index,
                                  uint16_t * shaping_profile,
                                  struct sched_shaping_profile_params * node_shaping_params)
    {
        DECLARE_TM_CTL_PTR(ctl, hndl);
        (void)ctl;/* prevent warning if virtual layer is not used */
        CONVERT_TO_PHYSICAL(level,node_index);
        return prvCpssSchedShapingUpdateNodeShapingProRreadNodeShapingProc( hndl, level, node_index, shaping_profile, node_shaping_params);
    }


#endif /* PER_NODE_SHAPING */
