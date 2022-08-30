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
 * @brief utilities for work with register bit fields.
 *
* @file tm_register_processing.c
*
* $Revision: 2.0 $
 */


#include <cpss/dxCh/dxChxGen/txq/private/Sched/core/prvSchedRegistersProcessing.h>

/*#include "stdint.h" */
#include <cpss/dxCh/dxChxGen/txq/private/Sched/platform/prvShedPlatformImplementationDefinitions.h>
#include <cpss/dxCh/dxChxGen/txq/private/Sched/prvSchedErrCodes.h>
#include <cpss/generic/log/prvCpssLog.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

int schedSetField
(
    void    *address,
    int     shift,
    int     width,
    int     value,
    char    *fieldName
)
{
    uint64_t * pvar=(uint64_t *) address;
    int valueWidth;


    uint64_t mask= (1 << width) - 1 ;
    uint64_t uvalue=value;
    uvalue &= mask; /* removing extra hi bits (if any) */
    mask = ~(mask << shift);
    uvalue <<= shift;
    *pvar = (*pvar & mask) | uvalue;

#ifndef CPSS_LOG_ENABLE
    GT_UNUSED_PARAM(fieldName);
#endif
    valueWidth = 0;

    while(value)
    {
       valueWidth++;
       value>>=1;
    }
    if(valueWidth>width)
    {
        CPSS_LOG_ERROR_MAC("Error in setting field %s .Expected witdth %d < received width %d .\n",fieldName,width,valueWidth);
       return TM_CONF_PORT_IND_OOR;
    }

    return 0;


}

unsigned int schedGetField(void * address, int shift, int width)
{
    uint64_t * pvar=(uint64_t *) address;
    uint64_t uvalue;
    uint64_t mask= (1 <<width)-1;
    mask <<= shift;
    uvalue= (*pvar & mask);
    uvalue >>=shift;
    return (unsigned int) uvalue;
}
