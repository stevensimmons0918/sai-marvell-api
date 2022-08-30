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
 * @brief TM Shaping configuration, including General registers and Shaping Profiles.
 *
* @file tm_shaping.h
*
* $Revision: 2.0 $
 */

#ifndef   	SCHED_SHAPING_H
#define   	SCHED_SHAPING_H

#include <cpss/dxCh/dxChxGen/txq/private/Sched/prvSchedDefs.h>




#ifdef PER_NODE_SHAPING

/** Update shaping for node - the NODE shaping parameters are updated
 * according to parameters passed through API.  If  success - the node
 * is excluded from clients of shaping profile which it was belong ,
 * the node shaping profile ref is set to SCHED_DIRECT_NODE_SHAPING

 * @param[in]     hndl            TM lib handle.
 * @param[in]     level     		a level
 * @param[in]     node_index      index of node to configure shaping for it.
 * @param[in]     node_shaping_params  	      Shaping profile configuration struct pointer
 *
 *   @return an integer return code.
 *   @retval zero on success.
 *   @retval -EINVAL if hndl is NULL.
 *   @retval -EBADF if hndl is invalid.
 *   @retval -ERANGE if level is invalid.
 *   @retval -EADDRNOTAVAIL if index is out of range.
 *   @retval -ENODATA if index is not in use.
 *   @retval -EFAULT  if CBS/EBS  is out of range
 *   @retval TM_BW_OUT_OF_RANGE  if  cir/eir  too big to be configured
 *   @retval TM_CONF_MIN_TOKEN_TOO_LARGE - the cbs value is less than calculated cir token
 *   @retval TM_CONF_MAX_TOKEN_TOO_LARGE - the ebs value is less than calculated eir token
 *   @retval TM_HW_SHAPING_PROF_FAILED if download to HW fails
*/
int prvCpssSchedShapingUpdateNodeShaping
(
         PRV_CPSS_SCHED_HANDLE hndl,
         enum schedLevel level,
         uint32_t node_index,
         struct sched_shaping_profile_params * node_shaping_params,
         uint32_t * actual_cir_bw_received_ptr,
         uint32_t * actual_eir_bw_received_ptr
   );

/** read shaping for node
 * according to parameters passed through API.  If  success - the node
 * is excluded from clients of shaping profile which it was belong ,
 * the node shaping profile ref is set to SCHED_DIRECT_NODE_SHAPING

 * @param[in]     hndl            	TM lib handle.
 * @param[in]     level     		a level
 * @param[in]     node_index      	index of node to read shaping parameters
 * @param[out]    *shaping_profile  shaping profile reference  for node
 * @param[out]     node_shaping  	      Shaping profile configuration struct pointer -
 *
 *   @return an integer return code.
 *   @retval zero on success.
 * 	in this case  *shaping_profile contains  shaping profile reference for  this node
 *  if *shaping_profile == SCHED_DIRECT_NODE_SHAPING  than node_shaping structure is filled by
 *  values used for node shaping configuration.
 *
 *   @retval -EINVAL if hndl is NULL.
 *   @retval -EBADF if hndl is invalid.
 *   @retval -ERANGE if level is invalid.
 *   @retval -EADDRNOTAVAIL if index is out of range.
 *   @retval -ENODATA if index is not in use.
*/

int prvCpssSchedReadNodeShapingConfigFromShadow(PRV_CPSS_SCHED_HANDLE hndl,
                              enum schedLevel level,
                              uint32_t node_index,
                              uint16_t * shaping_profile,
                              struct sched_shaping_profile_params * node_shaping);

#endif /* PER_NODE_SHAPING */







#endif   /* TM_SHAPING_H */

