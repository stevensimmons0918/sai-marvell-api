
/* sa_builder_internal.h
 *
 * Internal API of the MAcsec SA Builder.
 * - layout of the control words.
 * - Headers for shared functions and protocol-specific functions.
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


#ifndef SA_BUILDER_INTERNAL_H_
#define SA_BUILDER_INTERNAL_H_
#include <Kit/SABuilder_MACsec/src/c_sa_builder_macsec.h>
#include <Kit/SABuilder_MACsec/incl/sa_builder_macsec.h>

/*----------------------------------------------------------------------------
 * Definitions and macros
 */

/* Context control word values for MACsec */
#define SAB_CW0_MACSEC_EG32 0x1000A000
#define SAB_CW0_MACSEC_IG32 0x1000A001
#define SAB_CW0_MACSEC_EG64 0x2000A000
#define SAB_CW0_MACSEC_IG64 0x2000A081


/* Context control word values for basic encryption/authentication */
#define SAB_CW0_ENCAUTH_AES_GCM  0x82018006
#define SAB_CW0_AUTHDEC_AES_GCM  0x8201800f
#define SAB_CW0_ENC_AES_CTR      0x80010004

/* Context control word values for various AES key lengths. */
#define SAB_CW0_AES128 0x000a0000
#define SAB_CW0_AES192 0x000c0000
#define SAB_CW0_AES256 0x000e0000

/* Various options for MACsec SAs */
#define SAB_CW0_UPDATE_SEQ       0x00002000
#define SAB_CW0_ROLLOVER         0x00000400
#define SAB_CW0_KEEP_SECTAG      0x00000200
#define SAB_CW0_KEEP_ICV         0x00000100
#define SAB_CW0_NO_LATE_CHECK    0x00000040
#define SAB_CW0_LATE_HDR_DROP    0x00000020
#define SAB_CW0_LATE_HDR_CHECK   0x00000010
/* Data structure to represent offsets of various fields.  If an
   offset is zero, the corresponding field is not present. */
typedef struct {
    uint8_t KeyOffs;
    uint8_t HKeyOffs;
    uint8_t SeqNumOffs;
    uint8_t MaskOffs;
    uint8_t CtxSaltOffs;
    uint8_t IVOffs;
    uint8_t UpdateCtrlOffs;
    uint8_t MTUOffs;
    uint8_t MaxOffs;
    uint32_t ReservedBitsMask;
} SABuilder_Offsets_t;


/*----------------------------------------------------------------------------
 * SABuilderLib_ParamsSupported
 *
 * This is a device-specific function.
 * Check if device supports the supplied parameters. Check only those
 * that are supported on some MACsec devices, but not on others.
 *
 * SAParams_p (input)
 *    SA parameters to be checked.
 *
 * Return:
 *    true if device supports the supplied parameters/
 *    false if device does not support the supplied parameters
 */
bool
SABuilderLib_ParamsSupported(
        const SABuilder_Params_t * const SAParams_p);


/*----------------------------------------------------------------------------
 * SABuilderLib_SetOffsets
 *
 * This is a device-specific function.
 * Compute the offsets of the various fields in the SA record. depending
 * on the specified operation. Fields not supported are returned as zero.
 *
 * SAParams_p (input)
 *    SA parameters specifying the operation.
 * SAOffsets_p (output)
 *    Data structure containing the offsets of the various fields.
 */
void
SABuilderLib_SetOffsets(
        const SABuilder_Params_t * const SAParams_p,
        SABuilder_Offsets_t * const SAOffsets_p);


#endif /* SA_BUILDER_INTERNAL_H_ */


/* end of file sa_builder_internal.h */
