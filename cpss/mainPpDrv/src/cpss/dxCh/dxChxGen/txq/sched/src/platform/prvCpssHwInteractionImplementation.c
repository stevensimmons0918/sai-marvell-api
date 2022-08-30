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
 * @brief basic TM read/write/reset  functions implementation and connection support  for cpss
 *
* @file cpss_hw_interaction_implementation.c
*
* $Revision: 2.0 $
 */
#include <cpss/dxCh/dxChxGen/txq/private/Sched/core/prvSchedCoreTypes.h>
#include <cpssDriver/pp/hardware/prvCpssDrvHwCntl.h>
#include <cpssDriver/pp/hardware/cpssDriverPpHw.h>
#include <cpss/generic/cpssHwInit/private/prvCpssHwRegisters.h>
#include <cpss/common/config/private/prvCpssConfigTypes.h>
#include <cpss/dxCh/dxChxGen/txq/private/Sched/platform/prvSchedCpssRwRegistersProc.h>
#include <cpss/dxCh/dxChxGen/txq/private/Sched/platform/prvSchedRegistersInterface.h>
#include <cpss/dxCh/dxChxGen/txq/private/Sched/core/prvSchedGetGenParamInterface.h>
#include <cpss/dxCh/dxChxGen/txq/private/Sched/prvSchedErrCodes.h>
#include <cpss/dxCh/dxChxGen/txq/private/Sched/core/prvSetHwRegisters.h>
#include <stdio.h>
#include <cpss/dxCh/dxChxGen/txq/private/prvCpssDxChTxqMain.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>


#define CPSS_TO_XEL_ERR_CODE(x)     x


#define TM_FLUSH_MAX_RETRIES     16

#define PRV_SIP_6_PDQ_FREQ_MB 800
#define PRV_SIP_6_10_PDQ_FREQ_MB 816
#define PRV_SIP_6_15_PDQ_FREQ_MB 600 /*same for Ironman*/


/**
 */
int prvSchedRegisterRead(void * environment_handle,
                                GT_U64 reg_addr,
                                GT_U64 *data)
{
        CPSS_ENV(env,environment_handle, -EINVAL, -EBADF)
        return CPSS_TO_XEL_ERR_CODE(prvSchedReadRegisterProc(env->devNumber,reg_addr,data,env->globalAddress));

}


/**
 */
int prvSchedRegisterWrite(void * environment_handle,
                                 GT_U64 reg_addr,
                                 GT_U64 *data)
{
        CPSS_ENV(env,environment_handle, -EINVAL, -EBADF)
        return CPSS_TO_XEL_ERR_CODE(prvSchedWriteRegisterProc(env->devNumber,reg_addr,data,env->globalAddress));
}


/**
 */
int prvSchedTableEntryRead(void * environment_handle,
                                   GT_U64 tbl_addr, /* table address */
                                   uint32_t index,  /* table entry index */
                                   GT_U64 *data)      /* entry value */
{
    GT_U32      offset;
    GT_U32      tempAddr;
    GT_U64      entryAddr;


        CPSS_ENV(env,environment_handle, -EINVAL, -EBADF)

    /* Table index handle */
    offset = index * 8;
    tempAddr = tbl_addr.l[0]/8;

    /* TBD Assumption: index is not big enough to influence the MSB part of the table address */
    if((tempAddr + index) > 0x1FFFFFFF) {
        entryAddr.l[0] = (tempAddr + index - 0x1FFFFFFF)*8 + (tbl_addr.l[0] - tempAddr*8);
        entryAddr.l[1] = (tbl_addr.l[1] + 1);
    }
    else {
        entryAddr.l[0] = (tbl_addr.l[0] + offset);
        entryAddr.l[1] = tbl_addr.l[1];
    }

        return CPSS_TO_XEL_ERR_CODE(prvSchedReadRegisterProc(env->devNumber,entryAddr, data,env->globalAddress));

}

/**
 */
int prvSchedTableEntryWrite
(
    void * environment_handle,
    GT_U64 tbl_addr, /* table address */
    uint32_t index,  /* table entry index */
    GT_U64 *data     /* entry value */
)
{
    GT_U32      offset;
    GT_U32      tempAddr;
    GT_U64      entryAddr;

        CPSS_ENV(env,environment_handle, -EINVAL, -EBADF)

    /* Table index handle */
    offset = index * 8;
    tempAddr = tbl_addr.l[0]/8;


    /* TBD Assumption: index is not big enough to influence the MSB part of the table address */
    if((tempAddr + index) > 0x1FFFFFFF) {
        entryAddr.l[0] = (tempAddr + index - 0x1FFFFFFF)*8 + (tbl_addr.l[0] - tempAddr*8);
        entryAddr.l[1] = (tbl_addr.l[1] + 1);
    }
    else {
        entryAddr.l[0] = (tbl_addr.l[0] + offset);
        entryAddr.l[1] = tbl_addr.l[1];
    }

        return CPSS_TO_XEL_ERR_CODE(prvSchedWriteRegisterProc(env->devNumber,entryAddr, data,env->globalAddress));

}

int prvSchedGetGenParams(PRV_CPSS_SCHED_HANDLE hndl)
{
    int rc = 0;
    GT_U8 devNum;
    GT_BOOL isSip6_10,isSip6_15;

    TM_CTL(ctl, hndl)

    devNum = PRV_SCHED_DEV_NUM_GET(ctl);
    isSip6_10 = PRV_CPSS_SIP_6_10_CHECK_MAC(devNum);
    isSip6_15 = PRV_CPSS_TXQ_LIKE_PHOENIX_MAC(devNum);
     /*Fixed for SIP6*/

    if(GT_TRUE==isSip6_15)
    {
        ctl->freq = PRV_SIP_6_15_PDQ_FREQ_MB;
    }
    else
    {
        ctl->freq = isSip6_10?PRV_SIP_6_10_PDQ_FREQ_MB:PRV_SIP_6_PDQ_FREQ_MB;
    }

    ctl->freq*=1000000;
    /* get other general parameters */
    rc = prvSchedLowLevelGetGeneralHwParams(hndl);

    if (rc)
    {
      return TM_HW_GEN_CONFIG_FAILED;
    }

    return rc;
}


int prvSchedInputFreqGet(PRV_CPSS_SCHED_HANDLE hndl,GT_U32 *freqPtr)
{
    TM_CTL(ctl, hndl)

    if(!freqPtr)
    {
        return TM_CONF_NULL_LOGICAL_NAME;
    }

    *freqPtr = ctl->freq;
    return 0;
}

