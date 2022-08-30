/* eip164_rules.c
 *
 * EIP-164 Driver Library Packet Classification Rules interface implementation
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

/*----------------------------------------------------------------------------
 * This module implements (provides) the following interface(s):
 */

/* EIP-164 Driver Library Classification Rules API */
#include <Kit/EIP164/incl/eip164_rules.h>


/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

/* EIP-164 Driver Library SecY types API */
#include <Kit/EIP164/incl/eip164_types.h>

/* Driver Framework Basic Definitions API */
#include <Kit/DriverFramework/incl/basic_defs.h>             /* IDENTIFIER_NOT_USED, bool, uint32_t */

/* Driver Framework Device API */
#include <Kit/DriverFramework/incl/device_types.h>           /* Device_Handle_t */

/* EIP-164 register interface */
#include <Kit/EIP164/incl/eip164_level0.h>

/* EIP-164 Internal interface */
#include <Kit/EIP164/incl/eip164_internal.h>


/*----------------------------------------------------------------------------
 * Definitions and macros
 */


/*----------------------------------------------------------------------------
 * Local variables
 */


/*----------------------------------------------------------------------------
 * EIP164_Rules_SecTag_Update
 */
EIP164_Error_t
EIP164_Rules_SecTag_Update(
        const EIP164_IOArea_t * const IOArea_p,
        const unsigned int ChannelId,
        const EIP164_Channel_Rule_SecTAG_t * const SecTag_Rules_p)
{
    Device_Handle_t Device;
    const volatile EIP164_True_IOArea_t * const TrueIOArea_p = IOAREA_IN(IOArea_p);
    int rc = 0;

    EIP164_CHECK_POINTER(IOArea_p);
    EIP164_CHECK_INT_ATMOST(ChannelId, (TrueIOArea_p->MaxChCount - 1));

    Device = TrueIOArea_p->Device;

    rc = EIP164_SAM_NM_PARAMS_WR(Device,
                                 ChannelId,
                                 SecTag_Rules_p->fCompEType,
                                 SecTag_Rules_p->fCheckV,
                                 SecTag_Rules_p->fCheckKay,
                                 SecTag_Rules_p->fCheckCE,
                                 SecTag_Rules_p->fCheckSC,
                                 SecTag_Rules_p->fCheckSL,
                                 SecTag_Rules_p->fCheckPN,
                                 SecTag_Rules_p->fCheckSLExt,
                                 SecTag_Rules_p->EtherType);
    if (rc) return rc;

    return EIP164_NO_ERROR;
}


/*----------------------------------------------------------------------------
 * EIP164_Rules_SecTag_Read
 */
EIP164_Error_t
EIP164_Rules_SecTag_Read(
        const EIP164_IOArea_t * const IOArea_p,
        const unsigned int ChannelId,
        EIP164_Channel_Rule_SecTAG_t * const SecTag_Rules_p)
{
    Device_Handle_t Device;
    const volatile EIP164_True_IOArea_t * const TrueIOArea_p = IOAREA_IN(IOArea_p);
    int rc = 0;

    EIP164_CHECK_POINTER(IOArea_p);
    EIP164_CHECK_INT_ATMOST(ChannelId, (TrueIOArea_p->MaxChCount - 1));

    Device = TrueIOArea_p->Device;

    rc = EIP164_SAM_NM_PARAMS_RD(Device,
                                 ChannelId,
                                 &SecTag_Rules_p->fCompEType,
                                 &SecTag_Rules_p->fCheckV,
                                 &SecTag_Rules_p->fCheckKay,
                                 &SecTag_Rules_p->fCheckCE,
                                 &SecTag_Rules_p->fCheckSC,
                                 &SecTag_Rules_p->fCheckSL,
                                 &SecTag_Rules_p->fCheckPN,
                                 &SecTag_Rules_p->fCheckSLExt,
                                 &SecTag_Rules_p->EtherType);

    if (rc) return rc;

    return EIP164_NO_ERROR;
}


/* end of file eip164_rules.c */
