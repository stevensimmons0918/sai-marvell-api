/* eip164_internal.h
 *
 * EIP-164 Global Control Driver Library Internal interface
 *
 */

/* -------------------------------------------------------------------------- */
/*                                                                            */
/*   Module        : ddk164                                                   */
/*   Version       : 2.5                                                      */
/*   Configuration : DDK-164                                                  */
/*                                                                            */
/*   Date          : 2020-Feb-27                                              */
/*                                                                            */
/* Copyright (c) 2008-2020 INSIDE Secure B.V. All Rights Reserved             */
/*                                                                            */
/* This confidential and proprietary software may be used only as authorized  */
/* by a licensing agreement from INSIDE Secure.                               */
/*                                                                            */
/* The entire notice above must be reproduced on all authorized copies that   */
/* may only be made to the extent permitted by a licensing agreement from     */
/* INSIDE Secure.                                                             */
/*                                                                            */
/* For more information or support, please go to our online support system at */
/* https://customersupport.insidesecure.com.                                  */
/* In case you do not have an account for this system, please send an e-mail  */
/* to ESSEmbeddedHW-Support@insidesecure.com.                                 */
/* -------------------------------------------------------------------------- */

#ifndef EIP164_INTERNAL_H_
#define EIP164_INTERNAL_H_

/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

/* Default configuration */
#include <Kit/EIP164/incl/c_eip164.h>

/* Driver Framework Device API */
#include <Kit/DriverFramework/incl/device_types.h>       /* Device_Handle_t */

/* EIP-164 Driver Library Types API */
#include <Kit/EIP164/incl/eip164_types.h>       /* EIP164_* types */


/*----------------------------------------------------------------------------
 * Definitions and macros
 */

typedef enum
{
    EIP164_HW_CONF_E,
    EIP164_HW_CONF_I,
    EIP164_HW_CONF_IE
} EIP164_HW_Conf_t;

/* I/O Area, used internally */
typedef struct
{
    Device_Handle_t                 Device;
    EIP164_Device_Mode_t            Mode;
    EIP164_HW_Conf_t                HWConf;
    unsigned int                    MaxSACount;
    unsigned int                    MaxChCount;
    unsigned int                    MaxSCCount;
    unsigned int                    MaxvPortCount;
    bool                            fExternalRxCAM;
    bool                            fReadPending;
} EIP164_True_IOArea_t;

#define IOAREA_IN(_p)    ((const volatile EIP164_True_IOArea_t * const)_p)
#define IOAREA_INOUT(_p) ((volatile EIP164_True_IOArea_t * const)_p)

#ifdef EIP164_STRICT_ARGS
#define EIP164_CHECK_POINTER(_p) \
    if (NULL == (_p)) \
        return EIP164_ARGUMENT_ERROR;
#define EIP164_CHECK_INT_INRANGE(_i, _min, _max) \
    if ((_i) < (_min) || (_i) > (_max)) \
        return EIP164_ARGUMENT_ERROR;
#define EIP164_CHECK_INT_ATLEAST(_i, _min) \
    if ((_i) < (_min)) \
        return EIP164_ARGUMENT_ERROR;
#define EIP164_CHECK_INT_ATMOST(_i, _max) \
    if ((_i) > (_max)) \
        return EIP164_ARGUMENT_ERROR;
#else
/* EIP164_STRICT_ARGS undefined */
#define EIP164_CHECK_POINTER(_p)
#define EIP164_CHECK_INT_INRANGE(_i, _min, _max)
#define EIP164_CHECK_INT_ATLEAST(_i, _min)
#define EIP164_CHECK_INT_ATMOST(_i, _max)
#endif /*end of EIP164_STRICT_ARGS */

#define TEST_SIZEOF(type, size) \
    extern int size##_must_bigger[1 - 2*((int)(sizeof(type) > size))]

/* validate the size of the fake and real IOArea structures */
TEST_SIZEOF(EIP164_True_IOArea_t, EIP164_IOAREA_REQUIRED_SIZE);


#endif /* EIP164_INTERNAL_H_ */


/* end of file eip164_internal.h */
