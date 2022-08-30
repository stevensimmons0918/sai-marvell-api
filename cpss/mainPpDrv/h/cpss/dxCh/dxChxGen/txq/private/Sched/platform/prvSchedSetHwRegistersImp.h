#ifndef _SET_HW_REGISTERS_IMP_H_
#define _SET_HW_REGISTERS_IMP_H_


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
 * @brief useful macro wrappers for read/write/reset register functions.
 *
* @file set_hw_registers_imp.h
*
* $Revision: 2.0 $
 */



#include <cpss/dxCh/dxChxGen/txq/private/Sched/platform/prvSchedRegistersInterface.h>
#include <cpss/dxCh/dxChxGen/txq/private/Sched/core/prvSchedOsInterface.h>
#include <cpss/common/config/private/prvCpssConfigTypes.h>



#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


#include <cpss/dxCh/dxChxGen/txq/private/Sched/platform/prvSchedRegs.h>

#define SHIFT_TABLE
/*#define STRUCTS */


#if defined(SHIFT_TABLE)



#include <cpss/dxCh/dxChxGen/txq/private/Sched/platform/prvShedRegsDescription.h>
#include <cpss/dxCh/dxChxGen/txq/private/Sched/platform/prvShedIronmanRegsDescription.h>

#else

#endif
#include <cpss/dxCh/dxChxGen/txq/private/Sched/core/prvSchedRegistersProcessing.h>





#if __STDC_VERSION__ < 199901L
# if __GNUC__ >= 2
#  define __func__ __FUNCTION__
# else
#  define __func__ "<unknown>"
# endif
#endif


int prvSchedAdressSpacePointerGet(PRV_CPSS_SCHED_HANDLE hndl,struct prvCpssDxChTxqSchedAddressSpace ** adressSpacePtrPtr);



#if defined(STRUCTS)
    #define TXQ_SCHED_RESET_REGISTER(address, register_name)    \
    /*  schedPrintf(" function %s -  reset register reserved fields %s(0x%08X)... \n",__FUNCTION__,#address,address);*/\
        /*rc = tm_register_reset(TM_ENV(ctl), REGISTER_ADDRESS_ARG(address),  TM_REGISTER_VAR_ADDR(register_name));*/\
        /**/if (rc) schedPrintf(" function %s -  failed to reset reserved fields for register %s(0x%08X) \n",__func__,#address,address);/**/
#elif defined(SHIFT_TABLE)
    #define TXQ_SCHED_RESET_REGISTER(address, register_name)    \
    TXQ_SCHED_REGISTER_RESET(register_name)\
    rc = 0;
#else
    #define TXQ_SCHED_RESET_REGISTER(address, register_name)  /* do nothing */
#endif


#if defined(STRUCTS)
    #define TM_RESET_TABLE_REGISTER(address, index , register_name) \
    /*  schedPrintf(" function %s -  reset table register reserved fields %s(base address 0x%08X , shift %d)  value 0x%016X\n",__FUNCTION__,#address,address,index, *((uint64_t*)TM_REGISTER_VAR_ADDR(register_name)));*/\
        /*rc = tm_table_entry_reset(TM_ENV(ctl), REGISTER_ADDRESS_ARG(address), index, TM_REGISTER_VAR_ADDR(register_name));*/\
        /**/if (rc) schedPrintf(" function %s -  failed to reset reserved fields to table  register %s(0x%08X) , index %ld \n",__func__,#address,address,(long int)index);/**/
#elif defined(SHIFT_TABLE)
    #define TM_RESET_TABLE_REGISTER(address, index , register_name) \
    TXQ_SCHED_REGISTER_RESET(register_name)\
    rc = 0;
#else
    #define TM_RESET_TABLE_REGISTER(address,index , register_name)  /* do nothing */
#endif

#define TXQ_SCHED_WRITE_REGISTER(_devNum,address, register_name)    \
    GT_U64  convertWriteDummy;\
    prvSchedConvertToU64(TM_REGISTER_VAR_ADDR(register_name),&convertWriteDummy);\
    if(prvSchedPrintRegistersLogEnableGet(_devNum,GT_FALSE))\
    {\
        schedPrintf("WR  [addr = 0x%08x] REG TILE 0 PDQ %s 0x%08x 0x%08x mask 0xffffffff\n",address.l[0],#register_name,\
        convertWriteDummy.l[1],convertWriteDummy.l[0]);\
    }\
    {\
    rc = prvSchedRegisterWrite(TM_ENV(ctl), REGISTER_ADDRESS_ARG(address),&convertWriteDummy);\
    /**/if (rc) schedPrintf(" function %s -  failed writing to register %s(0x%08X %08X) \n",__func__,#address,address.l[1],address.l[0]);/**/\
    }



#define TXQ_SCHED_WRITE_REGISTER_DEVDEPEND(address,register_name) \
{\
    PDQ_ENV_MAGIC_CHECK(ctl)\
    if (PRV_CPSS_SIP_6_30_CHECK_MAC(PDQ_ENV_DEVNUM_NUM(ctl)) == GT_FALSE)\
    {\
        TXQ_SCHED_WRITE_REGISTER(PDQ_ENV_DEVNUM_NUM(ctl),address, TM_Sched_##register_name)\
    }\
    else\
    {\
        TXQ_SCHED_WRITE_REGISTER(PDQ_ENV_DEVNUM_NUM(ctl),address, PDQ_SIP_6_30_##register_name)\
    }\
}\



#define TXQ_SCHED_READ_REGISTER(_devNum,address, register_name) \
    if(prvSchedPrintRegistersLogEnableGet(_devNum,GT_TRUE))schedPrintf(" function %s -  reading from register %s(0x%08X %08X) ... ",__FUNCTION__, #address, address.l[1], address.l[0]);\
    {\
      GT_U64  convertReadDummy;\
      rc = prvSchedRegisterRead(TM_ENV(ctl), REGISTER_ADDRESS_ARG(address),  &convertReadDummy);\
      prvSchedConvertFromU64(&convertReadDummy,TM_REGISTER_VAR_ADDR(register_name));\
    }\
    if((prvSchedPrintRegistersLogEnableGet(_devNum,GT_TRUE)))\
    {\
        if (rc==0)\
        {\
            schedPrintf(" result 0x%016llX\n",TM_REGISTER_VAR_NAME(register_name));\
        }\
        else schedPrintf(" failed !\n");\
    } \
    /**/if (rc) schedPrintf(" function %s -  failed to read from register %s(0x%08X %08X) \n",__func__,#address,address.l[1],address.l[0]);/**/


#define TXQ_SCHED_READ_REGISTER_DEVDEPEND(address,register_name) \
{\
    PDQ_ENV_MAGIC_CHECK(ctl)\
    if (PRV_CPSS_SIP_6_30_CHECK_MAC(PDQ_ENV_DEVNUM_NUM(ctl)) == GT_FALSE)\
    {\
        TXQ_SCHED_READ_REGISTER(PDQ_ENV_DEVNUM_NUM(ctl),address, TM_Sched_##register_name)\
    }\
    else\
    {\
        TXQ_SCHED_READ_REGISTER(PDQ_ENV_DEVNUM_NUM(ctl),address, PDQ_SIP_6_30_##register_name)\
    }\
}\


#define TXQ_SCHED_NODE_ELIG_FUNC_NUM_GET(_num)\
do \
{\
    PDQ_ENV_MAGIC_CHECK(ctl)\
    if (PRV_CPSS_SIP_6_30_CHECK_MAC(PDQ_ENV_DEVNUM_NUM(ctl)) == GT_TRUE)\
    {\
        _num= 16;\
    }\
    else\
    {\
        _num= 64;\
    }\
}\
while(0)



#define TXQ_SCHED_WRITE_TABLE_REGISTER(_devNum,address, index , register_name)  \
    {\
    GT_U64  convertWriteDummy;\
    /*  schedPrintf(" function %s -  writing to table register %s , shift %ld  value 0x%016llX\n",__FUNCTION__, #address, index, *((uint64_t*)TM_REGISTER_VAR_ADDR(register_name)));*/\
    prvSchedConvertToU64(TM_REGISTER_VAR_ADDR(register_name),&convertWriteDummy);\
    if(prvSchedPrintRegistersLogEnableGet(_devNum,GT_FALSE))\
    {\
        schedPrintf("WR  [addr = 0x%08x] MEM TILE 0 PDQ %s %d 0x%08x 0x%08x mask 0xffffffff\n",address.l[0],#register_name,\
        index,convertWriteDummy.l[1],convertWriteDummy.l[0]);\
    }\
    rc = prvSchedTableEntryWrite(TM_ENV(ctl), REGISTER_ADDRESS_ARG(address), index, &convertWriteDummy);\
    /**/if (rc) schedPrintf(" function %s -  failed writing to table  %s (0x%08X %08X) , index %ld \n",__func__,#address,address.l[1],address.l[0],(long int)index);/**/\
    }\

    #define TXQ_SCHED_WRITE_TABLE_REGISTER_DEVDEPEND(address, index , register_name) \
    {\
        PDQ_ENV_MAGIC_CHECK(ctl)\
        if (PRV_CPSS_SIP_6_30_CHECK_MAC(PDQ_ENV_DEVNUM_NUM(ctl)) == GT_FALSE)\
        {\
            TXQ_SCHED_WRITE_TABLE_REGISTER(PDQ_ENV_DEVNUM_NUM(ctl),address, index ,TM_Sched_##register_name)\
        }\
        else\
        {\
            TXQ_SCHED_WRITE_TABLE_REGISTER(PDQ_ENV_DEVNUM_NUM(ctl),address, index ,PDQ_SIP_6_30_##register_name)\
        }\
    }\

#define TXQ_SCHED_READ_TABLE_REGISTER(_devNum,address, index , register_name)   \
    if(prvSchedPrintRegistersLogEnableGet(_devNum,GT_TRUE))schedPrintf(" function %s -  reading from  table register %s(base address 0x%08X %08X , shift %d) ...",__FUNCTION__,#address,address.l[1],address.l[0],index);\
    {\
     GT_U64  convertReadDummy;\
     rc = prvSchedTableEntryRead(TM_ENV(ctl), REGISTER_ADDRESS_ARG(address), index, &convertReadDummy);\
     prvSchedConvertFromU64(&convertReadDummy,TM_REGISTER_VAR_ADDR(register_name));\
    }\
    if(prvSchedPrintRegistersLogEnableGet(_devNum,GT_TRUE))\
    {\
        if (rc==0)\
        {\
            schedPrintf(" result 0x%016llX\n",TM_REGISTER_VAR_ADDR(register_name));\
        }\
        else schedPrintf(" failed !\n");\
    } \
    /**/if (rc) schedPrintf(" function %s -  failed to read from table %s (0x%08X %08X) , index %ld \n",__func__,#address,address.l[1],address.l[0],(long int)index);/**/

#define TXQ_SCHED_READ_TABLE_REGISTER_DEVDEPEND(address, index , register_name) \
{\
    PDQ_ENV_MAGIC_CHECK(ctl)\
    if (PRV_CPSS_SIP_6_30_CHECK_MAC(PDQ_ENV_DEVNUM_NUM(ctl)) == GT_FALSE)\
    {\
        TXQ_SCHED_READ_TABLE_REGISTER(PDQ_ENV_DEVNUM_NUM(ctl),address, index ,TM_Sched_##register_name)\
    }\
    else\
    {\
        TXQ_SCHED_READ_TABLE_REGISTER(PDQ_ENV_DEVNUM_NUM(ctl),address, index ,PDQ_SIP_6_30_##register_name)\
    }\
}\


#endif  /* _SET_HW_REGISTERS_IMP_H_ */
