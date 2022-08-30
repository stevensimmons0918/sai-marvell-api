/* sa_builder_macsec.c
 *
 * Main implementation file of the MACsec SA builder.
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

/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */
#include <Kit/SABuilder_MACsec/src/c_sa_builder_macsec.h>  /* configuration */
#include <Kit/DriverFramework/incl/basic_defs.h>           /* basuc defubtuibs */
#include <Kit/DriverFramework/incl/clib.h>                 /* cpssOsMemSet */
#include <Kit/Log/incl/log.h>                  /* Log */
#include <Kit/SABuilder_MACsec/src/sa_builder_internal.h>  /* Internal SA Builder API */
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>



/*----------------------------------------------------------------------------
 * Local variables
 */


/*----------------------------------------------------------------------------
 * SABuilderLib_CopyKeyMat
 *
 * Copy a key into the SA.
 *
 * Destination_p (input)
 *   Destination (word-aligned) of the SA record.
 *
 * offset (input)
 *   Word offset of the key in the SA record where it must be stored.
 *
 * Source_p (input)
 *   Source (byte aligned) of the data.
 *
 * KeyByteCount (input)
 *   Size of the key in bytes.
 *
 * Destination_p is allowed to be a null pointer, in which case no key
 * will be written.
 */
static void
SABuilderLib_CopyKeyMat(
        uint32_t * const Destination_p,
        const unsigned int offset,
        const uint8_t * const Source_p,
        const unsigned int KeyByteCount)
{
    uint32_t *dst = Destination_p + offset;
    const uint8_t *src = Source_p;
    unsigned int i,j;
    uint32_t w;
    if (Destination_p == NULL)
        return;
    for(i=0; i<(KeyByteCount+3)/4; i++)
    {
        w=0;
        for(j=0; j<4; j++)
            w=(w>>8)|(*src++ << 24);
        *dst++ = w;
    }
}


/*----------------------------------------------------------------------------
 * SABuilderLib_ControlToParams
 *
 * Convert Control Word of MACsec SA to SA Parameters, so we can compute
 * offsets.
 *
 * ControlWord (input)
 *     Control word (first 32-bit word) of an existing SA record.
 *
 * SAParams_p (output)
 *     SA Parameters.
 */
static void
SABuilderLib_ControlToParams(
        uint32_t ControlWord,
        SABuilder_Params_t * const SAParams_p)
{
    SABuilder_Direction_t direction;

    if ((ControlWord & MASK_1_BIT) == 1)
        direction = SAB_DIRECTION_INGRESS;
    else
        direction = SAB_DIRECTION_EGRESS;

    SABuilder_InitParams(SAParams_p, 0, direction, SAB_OP_MACSEC);

    if (((ControlWord >> 29) & MASK_1_BIT) != 0)
        SAParams_p->flags |= SAB_MACSEC_FLAG_LONGSEQ;

    switch (ControlWord & (MASK_3_BITS << 17))
    {
    case SAB_CW0_AES128:
        SAParams_p->KeyByteCount = 16;
        break;
    case SAB_CW0_AES192:
        SAParams_p->KeyByteCount = 24;
        break;
    case SAB_CW0_AES256:
        SAParams_p->KeyByteCount = 32;
        break;
    }
    if ((ControlWord & BIT_10) != 0)
    {
        SAParams_p->flags |= SAB_MACSEC_FLAG_ROLLOVER;
    }
}

/*----------------------------------------------------------------------------
 * SABuilder_InitParams
 */
SABuilder_Status_t
SABuilder_InitParams(
        SABuilder_Params_t *const SAParams_p,
        uint8_t AN,
        SABuilder_Direction_t direction,
        SABuilder_Operation_t operation)
{
#ifdef SAB_STRICT_ARGS_CHECK
    if (SAParams_p == NULL)
        return SAB_INVALID_PARAMETER;
#endif

    cpssOsMemSet(SAParams_p, 0, sizeof(SABuilder_Params_t));

    SAParams_p->direction = direction;
    SAParams_p->operation = operation;
    SAParams_p->AN = AN;

    if (direction == SAB_DIRECTION_INGRESS)
    {
        SAParams_p->SeqNumLo = 1;
    }

    return SAB_STATUS_OK;
}

/*----------------------------------------------------------------------------
 * SABuilder_GetSize();
 */
SABuilder_Status_t
SABuilder_GetSize(
        SABuilder_Params_t *const SAParams_p,
        unsigned int *const SAWord32Count_p)
{
    SABuilder_Offsets_t SAOffsets;

#ifdef SAB_STRICT_ARGS_CHECK
    if (SAWord32Count_p == NULL)
        return SAB_INVALID_PARAMETER;
#endif

    /* Compute offsets for various fields. */
    SABuilderLib_SetOffsets(SAParams_p, &SAOffsets);

    /* Use a constant size SA record. */
    *SAWord32Count_p = SAOffsets.MaxOffs + 1;

    return SAB_STATUS_OK;
}


/*----------------------------------------------------------------------------
 * SABuilder_BuildSA();
 */
SABuilder_Status_t
SABuilder_BuildSA(
        SABuilder_Params_t * const SAParams_p,
        uint32_t *const SABuffer_p,
        SABuilder_AESCallback_t AES_CB)
{
    SABuilder_Offsets_t SAOffsets;

#ifdef SAB_STRICT_ARGS_CHECK
    if (SAParams_p == NULL || SABuffer_p == NULL)
        return SAB_INVALID_PARAMETER;
#endif

    if (SAParams_p->direction != SAB_DIRECTION_EGRESS &&
        SAParams_p->direction != SAB_DIRECTION_INGRESS)
    {
        LOG_CRIT("SABuilder_BuildSA: Invalid direction\n");
        return SAB_INVALID_PARAMETER;
    }

    if ( !SABuilderLib_ParamsSupported(SAParams_p))
    {
        return SAB_INVALID_PARAMETER;
    }

    /* Compute offsets for various fields. */
    SABuilderLib_SetOffsets(SAParams_p, &SAOffsets);

    /* Fill the entire SA record with zeros. */
    cpssOsMemSet(SABuffer_p, 0, (SAOffsets.MaxOffs+1) * sizeof(uint32_t));

    /* Fill in Context Control word. */
    switch (SAParams_p->operation)
    {
    case SAB_OP_MACSEC:
        if (SAParams_p->direction == SAB_DIRECTION_EGRESS)
        {
            if ( (SAParams_p->flags & SAB_MACSEC_FLAG_LONGSEQ) != 0)
            {
                SABuffer_p[0] = SAB_CW0_MACSEC_EG64;
            }
            else
            {
                SABuffer_p[0] = SAB_CW0_MACSEC_EG32;
                SAOffsets.CtxSaltOffs = 0;
            }

            SABuffer_p[0] &= SAOffsets.ReservedBitsMask;

            if (SAParams_p->AN > 3)
            {
                LOG_CRIT("SABuilder_BuildSA: AN value out of range: %d\n",
                         SAParams_p->AN);
            }

            SABuffer_p[0] |= (SAParams_p->AN) << 26;
        }
        else /* SAB_DIRECTION_INGRESS */
        {
            if ( (SAParams_p->flags & SAB_MACSEC_FLAG_LONGSEQ) != 0)
            {
                SABuffer_p[0] = SAB_CW0_MACSEC_IG64;
                SAOffsets.IVOffs = 0;
            }
            else
            {
                SABuffer_p[0] = SAB_CW0_MACSEC_IG32;
                SAOffsets.CtxSaltOffs = 0;
            }

            SABuffer_p[0] &= SAOffsets.ReservedBitsMask;

            if ((SAParams_p->flags & SAB_MACSEC_FLAG_RETAIN_ICV) != 0)
                SABuffer_p[0] |= SAB_CW0_KEEP_ICV;

            if ((SAParams_p->flags & SAB_MACSEC_FLAG_RETAIN_SECTAG) != 0)
                SABuffer_p[0] |= SAB_CW0_KEEP_SECTAG;

            if ((SAParams_p->flags & SAB_MACSEC_FLAG_NO_LATE_CHECK) != 0)
                SABuffer_p[0] |= SAB_CW0_NO_LATE_CHECK;

            if ((SAParams_p->flags & SAB_MACSEC_FLAG_LATE_HDR_DROP) != 0)
                SABuffer_p[0] |= SAB_CW0_LATE_HDR_DROP;

            if ((SAParams_p->flags & SAB_MACSEC_FLAG_LATE_HDR_CHECK) != 0)
                SABuffer_p[0] |= SAB_CW0_LATE_HDR_CHECK;

            if ((SAParams_p->flags & SAB_MACSEC_DISABLE_UPDATE_SEQ_NUM) != 0)
                SABuffer_p[0] &= ~(SAB_CW0_UPDATE_SEQ);
        }

        if ((SAParams_p->flags & SAB_MACSEC_FLAG_ROLLOVER) != 0)
            SABuffer_p[0] |= SAB_CW0_ROLLOVER;
        break;

    case SAB_OP_ENCAUTH_AES_GCM:
        SAOffsets.CtxSaltOffs = 0;
        SAOffsets.IVOffs = 0;
        if (SAParams_p->direction == SAB_DIRECTION_EGRESS)
        {
            SABuffer_p[0] = SAB_CW0_ENCAUTH_AES_GCM;
        }
        else
        {
            SABuffer_p[0] = SAB_CW0_AUTHDEC_AES_GCM;
        }

        if (SAParams_p->ICVByteCount == 0)
        {
            /* Treat as 16. */
            SABuffer_p[0] |= BIT_22;
        }
        else if (SAParams_p->ICVByteCount >= 8)
        {
            SABuffer_p[0] |= ((SAParams_p->ICVByteCount % 4) << 26) |
                (((SAParams_p->ICVByteCount - 8) / 4) << 21);
        }
        else
        {
            LOG_CRIT("SABuilder_BuildSA: Invalid ICVByteCount=%d\n",
                SAParams_p->ICVByteCount);
            return SAB_INVALID_PARAMETER;
        }
        break;

    case SAB_OP_ENC_AES_CTR:
        SAOffsets.CtxSaltOffs = 0;
        SAOffsets.IVOffs = 0;
        SAOffsets.HKeyOffs = 0;
        SABuffer_p[0] = SAB_CW0_ENC_AES_CTR;
        break;


    default:
        LOG_CRIT("SABuilder_BuildSA: Invalid operation\n");
        return SAB_INVALID_PARAMETER;
    }

    /* Set cipher algorithm bits in control word. */
    switch (SAParams_p->KeyByteCount)
    {
    case 16:
        SABuffer_p[0] |= SAB_CW0_AES128;
        break;
    case 24:
        SABuffer_p[0] |= SAB_CW0_AES192;
        break;
    case 32:
        SABuffer_p[0] |= SAB_CW0_AES256;
        break;
    default:
        LOG_CRIT("SABuilder_BuildSA: Unsupported AES key size %d\n",
                 SAParams_p->KeyByteCount);
        return SAB_INVALID_PARAMETER;
    }

    /* Copy cipher key. */
#ifdef SAB_STRICT_ARGS_CHECK
    if (SAParams_p->Key_p == NULL)
        return SAB_INVALID_PARAMETER;
#endif

    SABuilderLib_CopyKeyMat(SABuffer_p, SAOffsets.KeyOffs, SAParams_p->Key_p,
                            SAParams_p->KeyByteCount);
    /* Copy HKey. */
    if (SAOffsets.HKeyOffs > 0)
    {
        if (SAParams_p->HKey_p == NULL)
        {
            uint8_t tmp[16];

            if (AES_CB == NULL)
                return  SAB_INVALID_PARAMETER;

            /* Encrypt a single all-zero block */
            AES_CB((uint8_t *)(SABuffer_p+SAOffsets.HKeyOffs),
                   /* HKEY location still contains all zeros */
                   tmp,
                   SAParams_p->Key_p,
                   SAParams_p->KeyByteCount);

            SABuilderLib_CopyKeyMat(SABuffer_p, SAOffsets.HKeyOffs,
                                    tmp, 16);
        }
        else
        {
            SABuilderLib_CopyKeyMat(SABuffer_p, SAOffsets.HKeyOffs,
                                    SAParams_p->HKey_p, 16);
        }
    }

    /* fill in sequence number/seqmask. */
    if (SAParams_p->operation == SAB_OP_MACSEC)
    {
        SABuffer_p[SAOffsets.SeqNumOffs] = SAParams_p->SeqNumLo;

        if ((SAParams_p->flags & SAB_MACSEC_FLAG_LONGSEQ) != 0)
            SABuffer_p[SAOffsets.SeqNumOffs + 1] = SAParams_p->SeqNumHi;

        if (SAParams_p->direction == SAB_DIRECTION_INGRESS)
            SABuffer_p[SAOffsets.MaskOffs] = SAParams_p->WindowSize;
    }

    if (SAB_OP_MACSEC == SAParams_p->operation)
    {
        /* Fill in CtxSalt field. */
        if (SAOffsets.CtxSaltOffs > 0)
        {
            const uint8_t * const salt = SAParams_p->Salt_p;
            const uint8_t * const ssci = SAParams_p->SSCI_p;
            uint32_t w;

#ifdef SAB_STRICT_ARGS_CHECK
            if (salt == NULL || ssci == NULL)
                return SAB_INVALID_PARAMETER;
#endif

            w =  (salt[0] ^ ssci[0])        |
                ((salt[1] ^ ssci[1]) << 8)  |
                ((salt[2] ^ ssci[2]) << 16) |
                ((salt[3] ^ ssci[3]) << 24);

            SABuffer_p[SAOffsets.CtxSaltOffs] = w;

            SABuilderLib_CopyKeyMat(SABuffer_p,
                                    SAOffsets.CtxSaltOffs + 1,
                                    salt + 4,
                                    8);
        }
    }

    /* Fill in IV fields. */
    if (SAOffsets.IVOffs > 0)
    {
#ifdef SAB_STRICT_ARGS_CHECK
        if (SAParams_p->SCI_p == NULL)
            return SAB_INVALID_PARAMETER;
#endif
        SABuilderLib_CopyKeyMat(SABuffer_p, SAOffsets.IVOffs,
                                SAParams_p->SCI_p, 8);
    }

    {
        int i = 0;

        for (i = 0; i < SAOffsets.MaxOffs; i++)
            LOG_INFO("SABuffer[%d] : 0x%X\n", i, SABuffer_p[i]);
    }

    return SAB_STATUS_OK;
}



/*----------------------------------------------------------------------------
 * SABuilder_SeqNumOffset_Get
 */
SABuilder_Status_t
SABuilder_SeqNumOffset_Get(
        uint32_t ControlWord,
        unsigned int * const SeqNumOffset_p,
        bool * const fExtPN_p)
{
    SABuilder_Params_t SAParams;
    SABuilder_Offsets_t SAOffsets;
#ifdef SAB_STRICT_ARGS_CHECK
    if (SeqNumOffset_p == NULL || fExtPN_p == NULL)
    {
        return SAB_INVALID_PARAMETER;
    }
#endif

    SABuilderLib_ControlToParams(ControlWord, &SAParams);
    SABuilderLib_SetOffsets(&SAParams, &SAOffsets);

    *SeqNumOffset_p = SAOffsets.SeqNumOffs;
    *fExtPN_p = ((SAParams.flags & SAB_MACSEC_FLAG_LONGSEQ) != 0);

    return SAB_STATUS_OK;
}


/*----------------------------------------------------------------------------
 * SABuilder_WindowSizeOffset_Get
 */
SABuilder_Status_t
SABuilder_WindowSizeOffset_Get(
        uint32_t ControlWord,
        unsigned int * const WindowSizeOffset_p)
{
    SABuilder_Params_t SAParams;
    SABuilder_Offsets_t SAOffsets;
#ifdef SAB_STRICT_ARGS_CHECK
    if (WindowSizeOffset_p == NULL)
    {
        return SAB_INVALID_PARAMETER;
    }
#endif
    SABuilderLib_ControlToParams(ControlWord, &SAParams);
    SABuilderLib_SetOffsets(&SAParams, &SAOffsets);

    *WindowSizeOffset_p = SAOffsets.MaskOffs;

    return SAB_STATUS_OK;
}


/*----------------------------------------------------------------------------
 * SABuilder_MTUOffset_Get
 */
SABuilder_Status_t
SABuilder_MTUOffset_Get(
        uint32_t ControlWord,
        unsigned int * const MTUOffset_p)
{
    SABuilder_Params_t SAParams;
    SABuilder_Offsets_t SAOffsets;
#ifdef SAB_STRICT_ARGS_CHECK
    if (MTUOffset_p == NULL)
    {
        return SAB_INVALID_PARAMETER;
    }
#endif
    SABuilderLib_ControlToParams(ControlWord, &SAParams);
    SABuilderLib_SetOffsets(&SAParams, &SAOffsets);

    *MTUOffset_p = SAOffsets.MTUOffs;

    return SAB_STATUS_OK;
}


/*----------------------------------------------------------------------------
 * SABuilder_UpdateCtrlOffset_Get
 */
SABuilder_Status_t
SABuilder_UpdateCtrlOffset_Get(
        uint32_t ControlWord,
        unsigned int * const UpdCtrlOffset_p)
{
    SABuilder_Params_t SAParams;
    SABuilder_Offsets_t SAOffsets;
#ifdef SAB_STRICT_ARGS_CHECK
    if (UpdCtrlOffset_p == NULL)
    {
        return SAB_INVALID_PARAMETER;
    }
#endif
    SABuilderLib_ControlToParams(ControlWord, &SAParams);
    if ((SAParams.flags & SAB_MACSEC_FLAG_ROLLOVER) == 0)
    {
        SABuilderLib_SetOffsets(&SAParams, &SAOffsets);

        *UpdCtrlOffset_p = SAOffsets.UpdateCtrlOffs;
    }
    else
    {
        *UpdCtrlOffset_p = 0;
    }

    return SAB_STATUS_OK;
}

/*----------------------------------------------------------------------------
 * SABuilder_UpdateCtrl_Update
 */
SABuilder_Status_t
SABuilder_UpdateCtrl_Update(
        const SABuilder_UpdCtrl_Params_t * const UpdateParams_p,
        uint32_t * const SAWord_p)
{
    uint32_t Value;
#ifdef SAB_STRICT_ARGS_CHECK
    if (UpdateParams_p == NULL || SAWord_p == NULL)
    {
        return SAB_INVALID_PARAMETER;
    }
#endif

    Value = (UpdateParams_p->SAIndex & MASK_13_BITS) |
        ((UpdateParams_p->SCIndex & MASK_13_BITS) << 16) |
        ((UpdateParams_p->AN & MASK_2_BITS) << 29);
    if (UpdateParams_p->fUpdateEnable)
        Value |= BIT_31;
    if (UpdateParams_p->fSAIndexValid)
        Value |= BIT_15;
    if (UpdateParams_p->fExpiredIRQ)
        Value |= BIT_14;
    if (UpdateParams_p->fUpdateTime)
        Value |= BIT_13;

    *SAWord_p = Value;

    return SAB_STATUS_OK;
}


/*----------------------------------------------------------------------------
 * SABuilder_MTU_Update
 */
SABuilder_Status_t
SABuilder_MTU_Update(
        const SABuilder_MTU_Params_t * const UpdateParams_p,
        uint32_t * const SAWord_p)
{
    uint32_t Value;
#ifdef SAB_STRICT_ARGS_CHECK
    if (UpdateParams_p == NULL || SAWord_p == NULL)
    {
        return SAB_INVALID_PARAMETER;
    }
#endif

    Value = UpdateParams_p->MTU & MASK_16_BITS;
    if (UpdateParams_p->fEnable)
        Value |= BIT_16;
    if (UpdateParams_p->fDrop)
        Value |= BIT_17;

    *SAWord_p = Value;

    return SAB_STATUS_OK;
}


/*----------------------------------------------------------------------------
 * SABuilder_MTU_Decode
 */
SABuilder_Status_t
SABuilder_MTU_Decode(
        const uint32_t SAWord,
        SABuilder_MTU_Params_t * const UpdateParams_p)
{
#ifdef SAB_STRICT_ARGS_CHECK
    if (UpdateParams_p == NULL)
    {
        return SAB_INVALID_PARAMETER;
    }
#endif

    UpdateParams_p->MTU     = SAWord & MASK_16_BITS;
    UpdateParams_p->fEnable = (SAWord & BIT_16) != 0;
    UpdateParams_p->fDrop   = (SAWord & BIT_17) != 0;

    return SAB_STATUS_OK;
}


/* end of file sa_builder_macsec.c */
