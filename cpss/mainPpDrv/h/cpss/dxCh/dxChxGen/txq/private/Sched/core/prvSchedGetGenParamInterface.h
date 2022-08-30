#ifndef _SCHED_GET_GEN_PARAMS_H_
#define _SCHED_GET_GEN_PARAMS_H_

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
 * @brief interface to reading general scheduler parameters.
 *
* @file prvSchedGetGenParamInterface.h
*
* $Revision: 2.0 $
 */


#include <cpss/dxCh/dxChxGen/txq/private/Sched/core/prvSchedCoreTypes.h>


int prvSchedGetGenParams(PRV_CPSS_SCHED_HANDLE hndl);
int prvSchedInputFreqGet(PRV_CPSS_SCHED_HANDLE hndl,GT_U32 *freqPtr);



#endif /* _TM_GET_GEN_PARAMS_H_ */



