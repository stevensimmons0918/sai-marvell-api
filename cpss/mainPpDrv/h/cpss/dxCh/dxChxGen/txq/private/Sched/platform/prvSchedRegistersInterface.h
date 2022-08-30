#ifndef _SCHED_RW_REGISTERS_INTERFACE_H_
#define _SCHED_RW_REGISTERS_INTERFACE_H_


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
 * @brief interface of  tm read/write/ reset functions
 * placed in platform section because of different per platform  dellaration of Address parameter
 *
* @file prvSchedRegistersInterface.h
*
* $Revision: 2.0 $
 */


#include <cpss/common/cpssTypes.h>




int prvSchedRegisterRead
(
  void * environment_handle,
  GT_U64 reg_addr,
  GT_U64 *data
);



int prvSchedRegisterWrite
(
    void * environment_handle,
    GT_U64 reg_addr,
    GT_U64 *data
);

int prvSchedTableEntryRead(void * environment_handle, GT_U64 tbl_addr, /* table address */
                                   uint32_t index,  /* table entry index */
                                   GT_U64 *data);      /* entry value */

int prvSchedTableEntryWrite
(
    void * environment_handle,
    GT_U64 tbl_addr, /* table address */
    uint32_t index,  /* table entry index */
    GT_U64 *data     /* entry value */
);


void prvSchedConvertToU64
(
    void *dataIn,
    GT_U64 *dataOut
);

void prvSchedConvertFromU64
(
    GT_U64 *dataIn,
    void   *dataOut
);


GT_BOOL  prvSchedPrintRegistersLogEnableGet(GT_U8 devNum,GT_BOOL read);

#define	REGISTER_ADDRESS_ARG(addr)	addr






#endif	/* _TM_RW_REGISTERS_INTERFACE_H_ */
