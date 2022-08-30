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
 * @brief TM nodes control interface.
 *
* @file tm_nodes_ctl.h
*
* $Revision: 2.0 $
 */

#ifndef SCHED_NODES_CTL_H
#define SCHED_NODES_CTL_H

#include <cpss/dxCh/dxChxGen/txq/private/Sched/prvSchedDefs.h>


#ifdef LOGICAL_LAYER
/*
* low level reshuffling functions are not necessary
*/
#else

	/***************************************************************************
	 * Reshuffling
	 ***************************************************************************/

	/** Read next tree index/range change after reshuffling.
	 *
	 *   @param[in]     hndl            TM lib handle.
	 *   @param[out]    change          Change structure pointer.
	 *
	 *   @return an integer return code.
	 *   @retval zero on success.
	 *   @retval -@EINVAL if hndl is NULL.
	 *   @retval -@EBADF if hndl is invalid.
	 *   @retval -@ENOBUFS if list is empty.
	 */
	int prvSchedNodesReadNextChange(PRV_CPSS_SCHED_HANDLE hndl, struct sched_tree_change *change);


	/** Empty list of reshuffling changes.
	 *
	 *   @param[in]     hndl            TM lib handle.
	 *
	 *   @return an integer return code.
	 *   @retval zero on success.
	 *   @retval -@EINVAL if hndl is NULL.
	 *   @retval -@EBADF if hndl is invalid.
	 */
#endif

#endif   /* TM_NODES_CTL_H */

