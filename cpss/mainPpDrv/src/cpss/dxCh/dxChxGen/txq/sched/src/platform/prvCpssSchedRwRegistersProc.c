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
 * @brief basic H/W read wtite utilities for read/write registers implementation for cpss
 *
* @file cpss_hw_registers_proc.c
*
* $Revision: 2.0 $
 */
#include <cpss/dxCh/dxChxGen/txq/private/Sched/core/prvSchedOsInterface.h>
#include <cpss/dxCh/dxChxGen/txq/private/Sched/platform/prvSchedCpssRwRegistersProc.h>
#include <cpssDriver/pp/hardware/prvCpssDrvHwCntl.h>
#include <cpssDriver/pp/hardware/cpssDriverPpHw.h>
#include <cpss/generic/cpssHwInit/private/prvCpssHwRegisters.h>
#include <cpss/common/config/private/prvCpssConfigTypes.h>
#include <cpss/dxCh/dxChxGen/txq/private/Sched/core/prvSchedOsInterface.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <stdio.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

void prvSchedConvertToU64
(
    void *dataIn,
    GT_U64 *dataOut
)
{
    /* JIRA CPSS-10204*/
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    dataOut->l[0]=*((GT_U32 *)dataIn);
    dataOut->l[1]=*(((GT_U32 *)dataIn)+1);
#else
    dataOut->l[1]=*((GT_U32 *)(dataIn));
    dataOut->l[0]=*(((GT_U32 *)dataIn)+1);
#endif

}


void prvSchedConvertFromU64
(
    GT_U64 *dataIn,
    void   *dataOut
)
{
    /* JIRA CPSS-10204*/
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    *(((GT_U32 *)dataOut))   = dataIn->l[0];
    *(((GT_U32 *)dataOut)+1) = dataIn->l[1];
#else
    *(((GT_U32 *)dataOut))   = dataIn->l[1];
    *(((GT_U32 *)dataOut)+1) = dataIn->l[0];
#endif

}


/**
 */
int prvSchedReadRegisterProc
(
    GT_U8  devNumber,
    GT_U64 reg_addr,
    GT_U64 *data,
    GT_U32 unitBaseAdress
)
{
    GT_STATUS   ret;
    GT_U32      regAddr[2];
    GT_U32      regValue[2];


    regAddr[0] = reg_addr.l[0] + unitBaseAdress;
    regAddr[1] = regAddr[0]+4;


    ret = prvCpssHwPpReadRegister(devNumber, regAddr[0], &(regValue[0])); /* LSB */
    if (GT_OK != ret) return ret;

    ret = prvCpssHwPpReadRegister(devNumber, regAddr[1], &(regValue[1])); /* MSB */

    if (GT_OK != ret) return ret;

    data->l[0] = regValue[0];
    data->l[1] = regValue[1];

    return ret;
}



int prvSchedWriteRegisterProc
(
    GT_U8 devNumber,
    GT_U64 reg_addr,
    GT_U64 *data,
    GT_U32 unitBaseAdress
)
{
    GT_STATUS   ret;
    GT_U32      regAddr[2];
    GT_U32      regValue[2];

    regAddr[0] = reg_addr.l[0] + unitBaseAdress;
    regAddr[1] = regAddr[0]+4;


    regValue[0] = data->l[0];
    regValue[1] = data->l[1];

    ret = prvCpssHwPpWriteRam(devNumber, regAddr[0], 1, &(regValue[0]));
    if (GT_OK != ret) return ret;

    ret = prvCpssHwPpWriteRam(devNumber, regAddr[1], 1, &(regValue[1]));

    return ret;
}




/*
*/


int prvSchedPrintRxRegistersLogStatus(GT_U8 devNum)
{
    schedPrintf(" --------------------\n");
    schedPrintf("\n  print write calls : ");
    schedPrintf(PRV_CPSS_DXCH_PP_MAC(devNum)->port.tileConfigsPtr[0].general.schedDbg.schedLogWriteRequests? "yes" : "no");
    schedPrintf("\n  print read  calls : ");
    schedPrintf(PRV_CPSS_DXCH_PP_MAC(devNum)->port.tileConfigsPtr[0].general.schedDbg.schedLogReadRequests? "yes" : "no");
    schedPrintf("\n");
    return 0;
}


int prvSchedSetRegistersLogStatus(GT_U8 devNum,int write_status, int read_status)
{
    switch (write_status)
    {
        case 0:
        case 1:break;
        default : schedPrintf("wrong write flag value , abort ...\n"); return 1;
    }
    switch (read_status)
    {
        case 0:
        case 1:break;
        default : schedPrintf("wrong read flag value , abort ...\n"); return 1;
    }
    PRV_CPSS_DXCH_PP_MAC(devNum)->port.tileConfigsPtr[0].general.schedDbg.schedLogWriteRequests = write_status?GT_TRUE:GT_FALSE;
    PRV_CPSS_DXCH_PP_MAC(devNum)->port.tileConfigsPtr[0].general.schedDbg.schedLogReadRequests = read_status?GT_TRUE:GT_FALSE;
    return 0;
}


GT_BOOL  prvSchedPrintRegistersLogEnableGet(GT_U8 devNum,GT_BOOL read)
{
    if(GT_TRUE==read)
    {
        return PRV_CPSS_DXCH_PP_MAC(devNum)->port.tileConfigsPtr[0].general.schedDbg.schedLogReadRequests;
    }

    return PRV_CPSS_DXCH_PP_MAC(devNum)->port.tileConfigsPtr[0].general.schedDbg.schedLogWriteRequests;
}

