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
 * @brief Resource Manager control functions implementation.
 *
* @file rm_ctl.c
*
* $Revision: 2.0 $
 */

#include <stdlib.h>
#include <errno.h>

#include <cpss/dxCh/dxChxGen/txq/private/Sched/core/prvRmInternalTypes.h>

#include <cpss/dxCh/dxChxGen/txq/private/Sched/core/prvRmCtl.h>

#include <cpss/dxCh/dxChxGen/txq/private/Sched/core/prvRmChunk.h>

#include <cpss/dxCh/dxChxGen/txq/private/Sched/core/prvSchedOsInterface.h>
#include <cpss/dxCh/dxChxGen/txq/private/Sched/core/prvSchedCoreTypes.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

/**
 */

#define CHECK_RM_CREATION(fun)   if (!fun) { prvSchedRmClose((rmctl_t)ctl); return NULL;}


rmctl_t * prvSchedRmOpen
(
    uint32_t total_c_nodes,
    uint32_t total_b_nodes,
    uint32_t total_a_nodes,
    uint32_t total_queues
)
{

    struct rmctl *   ctl = NULL;

    /* Create rmctl instance */
    ctl = schedMalloc(sizeof(struct rmctl));
    if (ctl)
    {
        schedMemSet(ctl, 0, sizeof(*ctl));

        /* Fill in ctl structure */
        ctl->magic = RM_MAGIC;

        ctl->rm_free_nodes[SCHED_A_LEVEL]=prvSchedRmNewChunk(0,total_a_nodes,NULL);
        ctl->rm_free_nodes[SCHED_B_LEVEL]=prvSchedRmNewChunk(0,total_b_nodes,NULL);
        ctl->rm_free_nodes[SCHED_C_LEVEL]=prvSchedRmNewChunk(0,total_c_nodes,NULL);


         /* Looks like this H/W bug is not releavnt to SIP6*/
        ctl->rm_free_nodes[SCHED_Q_LEVEL]=prvSchedRmNewChunk(0,total_queues,NULL);


    }
    return (rmctl_t *)ctl;
}


/**
 */
int prvSchedRmClose(rmctl_t hndl)
{
    RM_HANDLE(ctl, hndl);

    ctl->magic = 0;

    prvSchedRmClearChunkList(ctl->rm_free_nodes[SCHED_Q_LEVEL]);
    prvSchedRmClearChunkList(ctl->rm_free_nodes[SCHED_A_LEVEL]);
    prvSchedRmClearChunkList(ctl->rm_free_nodes[SCHED_B_LEVEL]);
    prvSchedRmClearChunkList(ctl->rm_free_nodes[SCHED_C_LEVEL]);

    /* free rm handle */
    schedFree(ctl);
    return 0;
}



