#ifndef _TM_RW_REGISTERS_INTERFACE_H_
#define _TM_RW_REGISTERS_INTERFACE_H_


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
* @file tm_rw_registers_interface.h
*
* $Revision: 2.0 $
 */


#include <cpss/common/cpssTypes.h>



int tm_register_reset(void * environment_handle , GT_U64   Address , void * vpData);

int tm_table_entry_reset(void * environment_handle , GT_U64 Address , long int index, void * vpData);



int tm_register_read(void * environment_handle, GT_U64 reg_addr, GT_U64 *data);

int tm_register_write(void * environment_handle, GT_U64 reg_addr, GT_U64 *data);

int tm_table_entry_read(void * environment_handle, GT_U64 tbl_addr, /* table address */
                                   uint32_t index,  /* table entry index */
                                   GT_U64 *data);      /* entry value */

int tm_table_entry_write(void * environment_handle,
                                    GT_U64 tbl_addr, /* table address */
                                    uint32_t index,  /* table entry index */
                                    GT_U64 *data);      /* entry value */

void tm_convert_to_u64
(
    void *dataIn,
    GT_U64 *dataOut
);

void tm_convert_from_u64
(
    GT_U64 *dataIn,
    void   *dataOut
);




#define	REGISTER_ADDRESS_ARG(addr)	addr






#endif	/* _TM_RW_REGISTERS_INTERFACE_H_ */
