/*******************************************************************************
*              (c), Copyright 2021, Marvell International Ltd.                 *
* THIS CODE IS A REFERENCE CODE FOR MARVELL SWITCH PRODUCTS.  IT IS PROVIDED   *
* "AS IS" WITH NO WARRANTIES, EXPRESSED, IMPLIED OR OTHERWISE, REGARDING ITS   *
* ACCURACY, COMPLETENESS OR PERFORMANCE.                                       *
* CUSTOMERS ARE FREE TO MODIFY IT AND USE IT ONLY IN THEIR PRODUCTION          *
* SOFTWARE RELEASES WITH MARVELL SWITCH CHIPSETS.                              *
*******************************************************************************/
/**
********************************************************************************
* @file linuxsimStubs.h
*
* @brief Used for linuxSim/stubs.c
*
* @version   1
********************************************************************************
*/

#ifndef __gtOsLinuxSimStubsh
#define __gtOsLinuxSimStubsh

#ifdef __cplusplus
extern "C" {
#endif

/* win32 names */
#define HANDLE undef
typedef GT_U32 UINT_32,HANDLE, DWORD;

#define SHOSTP_max_intr_CNS 32

typedef UINT_32 SHOSTP_intr_FUN(void);
typedef UINT_32 SHOSTP_intr_param_FUN(UINT_32 param);

typedef struct SHOSTP_intr_STCT
{
    HANDLE hnd;
    GT_UINTPTR thd_id;
    UINT_32 id;
    UINT_32 mask;
    UINT_32 priority;
    UINT_32 susp_flag;
    HANDLE ev;
    UINT_32 was_ev;
    SHOSTP_intr_FUN *fun;
    int param_exist;
    UINT_32 param;
    int disabled;
} SHOSTP_intr_STC;

#ifdef __cplusplus
}
#endif

#endif  /* __gtOsLinuxSimStubss */
/* Do Not Add Anything Below This Line */



