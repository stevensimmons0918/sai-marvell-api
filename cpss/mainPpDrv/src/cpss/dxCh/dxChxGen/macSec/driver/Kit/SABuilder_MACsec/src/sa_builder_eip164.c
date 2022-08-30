/* sa_builder_eip164.c
 *
 * Device specific functions of the EIP-164 SA builder.
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
#include <Kit/SABuilder_MACsec/incl/sa_builder_macsec.h>
#include <Kit/SABuilder_MACsec/src/sa_builder_internal.h>  /* Internal SA Builder API */

/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */
#include <Kit/SABuilder_MACsec/src/c_sa_builder_macsec.h>  /* configuration */
#include <Kit/DriverFramework/incl/basic_defs.h>           /* basuc defubtuibs */
#include <Kit/Log/incl/log.h>                  /* Log */
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>



/*----------------------------------------------------------------------------
 * Local variables
 */
#if SAB_MACSEC_SA_WORD_COUNT != 24
#error "SA word count other than 24 not supported"
#endif

/*----------------------------------------------------------------------------
 * SABuilderLib_ParamsSupported
 */
bool
SABuilderLib_ParamsSupported(
        const SABuilder_Params_t * const SAParams_p)
{
    /* The EIP-164 does not support 192-bit AES keys. */
    if (SAParams_p->KeyByteCount == 24)
    {
        LOG_CRIT("SABuilder_BuildSA: AES-192 not supported\n");
        return false;
    }

    /* The EIP-164 does not support SA time stamps. */
    if ((SAParams_p->flags & SAB_MACSEC_FLAG_UPDATE_TIME) != 0)
    {
        LOG_CRIT("SABuilder_BuildSA: Time stamps not supported\n");
        return false;
    }

    return true;
}


/*----------------------------------------------------------------------------
 * SABuilderLib_SetOffsets
 */
void
SABuilderLib_SetOffsets(
        const SABuilder_Params_t * const SAParams_p,
        SABuilder_Offsets_t * const SAOffsets_p)
{
    cpssOsMemSet(SAOffsets_p, 0, sizeof(SABuilder_Offsets_t));

    SAOffsets_p->ReservedBitsMask = 0xFFFFFFFF;

    if (SAParams_p->direction == SAB_DIRECTION_EGRESS)
    {
        SAOffsets_p->CtxSaltOffs = 17;
        SAOffsets_p->IVOffs = 20;
        SAOffsets_p->MaxOffs = 21;
    }
    else if ((SAParams_p->flags & SAB_MACSEC_FLAG_LONGSEQ) != 0)
    {
        SAOffsets_p->CtxSaltOffs = 17;
        SAOffsets_p->IVOffs = 0;
        SAOffsets_p->MaxOffs = 19;
    }
    else
    {
        SAOffsets_p->CtxSaltOffs = 0;
        SAOffsets_p->MaxOffs = 19;

        if (SAB_OP_MACSEC == SAParams_p->operation)
        {
            SAOffsets_p->IVOffs = 17;
        }
    }

    /* For the EIP-164 all fields have a fixed offset. */
    SAOffsets_p->UpdateCtrlOffs = 1;
    SAOffsets_p->KeyOffs = 2;
    SAOffsets_p->HKeyOffs = 10;
    SAOffsets_p->SeqNumOffs = 14;
    SAOffsets_p->MaskOffs = 16;
    SAOffsets_p->MTUOffs = 16;

    if (SAB_OP_MACSEC == SAParams_p->operation)
    {
        /* Setting 0 for the reserved bits and other flags */
        SAOffsets_p->ReservedBitsMask &= ~(BIT_31 | BIT_30 | BIT_25 | BIT_24 |
                                           BIT_23 | BIT_22 | BIT_21 | BIT_20 |
                                           BIT_16 | BIT_14 | BIT_12 | BIT_11 |
                                           BIT_10 | BIT_9  | BIT_8  | BIT_7  |
                                           BIT_6  | BIT_5  | BIT_4);
    }
}

/* end of file sa_builder_eip164.c */
