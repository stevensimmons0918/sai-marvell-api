/** @file sa_builder_macsec.h
 *
 * @brief SABuilder API
 * This is API for the MACsec SA Builder. The
 * MACsec SA Builder creates SA data structures required by the MACsec
 * hardware. The SA data structures contain keys, sequence numbers, SCI and
 * salt values, as well as control information describing the operation type.
 *
 *
 *  - Use SABuilder_InitParams to initialize SABuilder_Params_t data
 *    structure.
 *  - Fill in more fields of the SABuilder_Params_t data structure:
 *    - Operation if test operation (encrypt-authenticate) is desired.
 *    - Key_p and KeyByteCount.
 *    - HKey_p
 *    - SCI_p if needed.
 *    - CtxSalt_p for 64-bit sequence numbers..
 *    - Window size.
 *    - SeqNumLo and SeqNumHi when starting at a nonstandard sequence number.
 *    - Any optional flags
 *  - Determine SA buffer size with SABuilder_GetSize
 *  - Allocate a buffer of the appropriate size.
 *  - Build the SA data structure with SABuilder_BuildSA.
 *  - Pass the SA to the hardware using the appropriate driver function.
 *  - Free the SA Buffer.
 *
 * The functions SABuilder_SeqNumOffset_Get,
 * SABuilder_UpdateCtrlOffset_Get, SABuilder_WindowSizeOffset_Get,
 * SABuilder_MTUOffset_Get, SABuilder_UpdateCtrl_Update and
 * SABuilder_MTU_Update are intended for use by the Driver.
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


#ifndef SA_BUILDER_MACSEC_H_
#define SA_BUILDER_MACSEC_H_


/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

/* Driver Framework Basic Definitions API */
#include <Kit/DriverFramework/incl/basic_defs.h>



/*----------------------------------------------------------------------------
 * Definitions and macros
 */

/** Status returned by each of the functions in this API*/
typedef enum
{
    SAB_STATUS_OK, /**< \n */
    SAB_INVALID_PARAMETER, /**< \n */
    SAB_UNSUPPORTED_FEATURE, /**< \n */
    SAB_ERROR /**< \n */
} SABuilder_Status_t;

/** Specify direction: egress (encrypt) or ingress (decrypt) */
typedef enum
{
    SAB_DIRECTION_EGRESS, /**< \n */
    SAB_DIRECTION_INGRESS /**< \n */
} SABuilder_Direction_t;

/** Operation type */
typedef enum
{
    SAB_OP_MACSEC,            /**< MACsec operation (default) */
    SAB_OP_ENCAUTH_AES_GCM,   /**< Test operation for authenticate-encrypt. */
    SAB_OP_ENC_AES_CTR,       /**< Test operation for encryption. */
} SABuilder_Operation_t;

/** Flags for the flags field. Put a bitwise or (or zero) of any
 *  of the flags in this field.*/

/** Use 64-bit sequence number. */
#define SAB_MACSEC_FLAG_LONGSEQ 0x1

/** Retain SecTAG (debugging).*/
#define SAB_MACSEC_FLAG_RETAIN_SECTAG 0x2

/** Retain ICV (debugging). */
#define SAB_MACSEC_FLAG_RETAIN_ICV 0x4

/** Allow sequence number rollover (debugging).*/
#define SAB_MACSEC_FLAG_ROLLOVER 0x8

/** Enable SA auto update. */
#define SAB_MACSEC_FLAG_UPDATE_ENABLE 0x10

/** Generate IRQ when SA is expired. */
#define SAB_MACSEC_FLAG_SA_EXPIRED_IRQ 0x20

/** Update time stamps (only if hardware supports this feature).*/
#define SAB_MACSEC_FLAG_UPDATE_TIME 0x40

/** Replay/Late Check Disabling at Post-Decryption State */
#define SAB_MACSEC_FLAG_NO_LATE_CHECK 0x80

/** Drop Action for Late packet detection at SOP */
#define SAB_MACSEC_FLAG_LATE_HDR_DROP 0x0100

/** Enable/Disable Late Packet detection at SOP */
#define SAB_MACSEC_FLAG_LATE_HDR_CHECK 0x0200

/** Update sequence number control, for ingress-only */
#define SAB_MACSEC_DISABLE_UPDATE_SEQ_NUM 0x0400

/** Input parameters for the SA Builder */
typedef struct
{
    /** flags, either 0 or the bitwise or of one or more flag values described above*/
    uint32_t flags;

    /** Direction, egress or ingress */
    SABuilder_Direction_t direction;

    /** Operation type */
    SABuilder_Operation_t operation;

    /** AN inserted in SecTAG (egress). */
    uint8_t AN;

    /** MACsec Key. */
    uint8_t *Key_p;

    /** Size of the MACsec key in bytes. */
    unsigned int KeyByteCount;

    /** authentication key, derived from MACsec key. */
    uint8_t *HKey_p;

    /** 12-byte salt (64-bit sequence numbers). */
    uint8_t *Salt_p;

    /** 4-byte SSCI value (64-bit sequence numbers).*/
    uint8_t *SSCI_p;

    /** 8-byte SCI.*/
    uint8_t *SCI_p;

    /** sequence number.*/
    uint32_t SeqNumLo;

    /** High part of sequence number (64-bit sequence numbers)*/
    uint32_t SeqNumHi;

    /** Size of the replay window (ingress).*/
    uint32_t WindowSize;

    /** digest length for ENCAUTH operation only.*/
    unsigned int ICVByteCount;
} SABuilder_Params_t;

/** Input parameters for SABuilder_UpadateCtrl_Update */
typedef struct
{
    /** Set to true if the SA must be updated. */
    bool fUpdateEnable;

    /** SA Index field is a valid SA. */
    bool fSAIndexValid;

    /** True if SA expired IRQ is to be generated.*/
    bool fExpiredIRQ;

    /** Transfer timestamp to next SA. */
    bool fUpdateTime;

    /** AN of an ingress SA.*/
    uint8_t AN;

    /** SA index of the next chained SA (egress).*/
    unsigned int SAIndex;

    /** SC index where SA is stored.*/
    unsigned int SCIndex;
} SABuilder_UpdCtrl_Params_t;


/** Input parameters for SABuilder_MTU_Update */
typedef struct
{
    /** Set to true if the packet must be dropped when failing the MTU check.*/
    bool fDrop;

    /** Enable MTU checking. */
    bool fEnable;

    /** MTU value in bytes. */
    unsigned int MTU;
} SABuilder_MTU_Params_t;


/**---------------------------------------------------------------------------
 * @typedef SABuilder_AESCallback_t
 *
 * Callback function (provided by application to encrypt a single
 * 16-byte block with AES.
 *
 * @param [in] In_p
 *     input data
 *
 * @param [out] Out_p
 *     output data
 *
 * @param [in] Key_p
 *     AES key
 *
 * @param [in] KeyByteCount
 *     Size of the key in bytes.
 *
 * @return value
 *     None
 */
typedef void (*SABuilder_AESCallback_t)(
        const uint8_t * const In_p,
        uint8_t * const Out_p,
        const uint8_t * const Key_p,
        const unsigned int KeyByteCount);


/**---------------------------------------------------------------------------
 * @fn SABuilder_InitParams(
 *       SABuilder_Params_t *const SAParams_p,
 *       uint8_t AN,
 *       SABuilder_Direction_t direction,
 *       SABuilder_Operation_t operation);
 *
 * Initialize SAParams_p data structure with default values.
 *
 * @param [out] SAParams_p
 *   Pointer to the SA parameters structure.
 *
 * @param [in] AN
 *   The AN of the SA (range 0..3).
 *
 * @param [in] direction
 *   Egress or ingress direction.
 *
 * @param [in] operation
 *   SA Builder Operation type: MACsec.
 *
 * @return SAB_STATUS_OK: success
 * @return SAB_INVALID_PARAMETER: the record referenced by SAParams_p is invalid
      or the AN or direction parameters are invalid.
 * @return SAB_UNSUPPORTED_FEATURE:  SAParams_p describes an operations that
 *    is not supported on the hardware for which this SA builder
 *    is configured.
 */
SABuilder_Status_t
SABuilder_InitParams(
        SABuilder_Params_t *const SAParams_p,
        uint8_t AN,
        SABuilder_Direction_t direction,
        SABuilder_Operation_t operation);

/**---------------------------------------------------------------------------
 * @fn SABuilder_GetSize(
 *        SABuilder_Params_t *const SAParams_p,
 *        unsigned int *const SAWord32Count_p);
 *
 * Compute the required size in 32-bit words of the SA buffer.
 *
 * @param [in] SAParams_p
 *   Pointer to the SA parameters structure.
 *
 * @param [out] SAWord32Count_p
 *   The size of the SA buffer
 *
 * The SAParams_p structure must be fully filled in: it must have the
 * same contents as it would have when SABuilder_BuildSA is called.
 *
 * @return SAB_STATUS_OK: success
 * @return SAB_INVALID_PARAMETER: the record referenced by SAParams_p is
 * invalid.
 * @return SAB_UNSUPPORTED_FEATURE: SAParams_p describes an operations that
 *    is not supported on the hardware for which this SA builder
 *    is configured.
 */
SABuilder_Status_t
SABuilder_GetSize(
        SABuilder_Params_t *const SAParams_p,
        unsigned int *const SAWord32Count_p);


/**---------------------------------------------------------------------------
 * @fn SABuilder_BuildSA(
 *      SABuilder_Params_t * const SAParams_p,
 *      uint32_t *const SABuffer_p,
 *      SABuilder_AESCallback_t AES_CB);
 *
 * Construct the SA record for the operation described in SAParams_p in
 * a memory buffer.
 *
 * @param [in] SAParams_p
 *    Pointer to the SA parameters structure.
 *
 * @param [out] SABuffer_p
 *    Pointer to the the SA buffer,
 *
 * @param [in] AES_CB
 *    Callback for function to encrypt block with AES.
 *
 * The SABuffer_p argument must point to a word-aligned
 * memory buffer whose size in words is at least equal to the
 * corresponding size parameter returned by SABuilder_GetSize().
 *
 * @return SAB_STATUS_OK: success
 * @return SAB_INVALID_PARAMETER: SAParams_p is invalid, or the SABuffer_p
 *    parameter is a null pointer.
 * @return SAB_UNSUPPORTED_FEATURE: SAParams_p describes an operations that
 *    is not supported on the hardware for which this SA builder
 *    is configured.
 */
SABuilder_Status_t
SABuilder_BuildSA(
        SABuilder_Params_t * const SAParams_p,
        uint32_t *const SABuffer_p,
        SABuilder_AESCallback_t AES_CB);


/**---------------------------------------------------------------------------
 * @fn SABuilder_SeqNumOffset_Get(
 *       uint32_t ControlWord,
 *       unsigned int * const SeqNumOffset_p,
 *       bool * const fExtPN_p);
 *
 * Return the offset of the sequence number within an existing SA record.
 *
 * @param [in] ControlWord
 *     Control word (first 32-bit word) of an existing SA record.
 *
 * @param [out] SeqNumOffset_p
 *     Offset (counted in 32-bit words) of the Sequence Number in the SA.
 *
 * @param [out] fExtPN_p
 *     True if the SA has 64-bit sequence numbers
 *
 * @return SAB_STATUS_OK: success
 * @return SAB_INVALID_PARAMETER: any of the output parameters is a null
 * pointer.
 */
SABuilder_Status_t
SABuilder_SeqNumOffset_Get(
        uint32_t ControlWord,
        unsigned int * const SeqNumOffset_p,
        bool * const fExtPN_p);


/**---------------------------------------------------------------------------
 * @fn SABuilder_WindowSizeOffset_Get(
 *       uint32_t ControlWord,
 *       unsigned int * const WindowSizeOffset_p);
 *
 * Return the offset of the Window Size within an existing SA record.
 *
 * @param [in] ControlWord
 *     Control word (first 32-bit word) of an existing SA record.
 *
 * @param [out] WindowSizeOffset_p
 *     Offset (counted in 32-bit words) of the Window Size in the SA.
 *
 * @return SAB_STATUS_OK: success
 * @return SAB_INVALID_PARAMETER: any of the output parameters is a null
 * pointer.
 */
SABuilder_Status_t
SABuilder_WindowSizeOffset_Get(
        uint32_t ControlWord,
        unsigned int * const WindowSizeOffset_p);


/**---------------------------------------------------------------------------
 * @fn SABuilder_MTUOffset_Get(
 *       uint32_t ControlWord,
 *       unsigned int * const MTUOffset_p);
 *
 * Return the offset of the MTU within an existing SA record.
 *
 * @param [in] ControlWord
 *     Control word (first 32-bit word) of an existing SA record.
 *
 * @param [out] MTUOffset_p
 *     Offset (counted in 32-bit words) of the MTU in the SA.
 *
 * @return SAB_STATUS_OK: success
 * @return SAB_INVALID_PARAMETER: any of the output parameters is a null
 * pointer.
 */
SABuilder_Status_t
SABuilder_MTUOffset_Get(
        uint32_t ControlWord,
        unsigned int * const MTUOffset_p);


/**---------------------------------------------------------------------------
 * @fn SABuilder_UpdateCtrlOffset_Get(
 *       uint32_t ControlWord,
 *       unsigned int * const UpdCtrlOffset_p);
 *
 * Return the offset of the update control word within an existing SA record.
 *
 * @param [in] ControlWord
 *     Control word (first 32-bit word) of an existing SA record.
 *
 * @param [out] UpdCtrlOffset_p
 *     Offset (counted in 32-bit words) of the Update Control word in the SA.
 *
 * @return SAB_STATUS_OK: success
 * @return SAB_INVALID_PARAMETER: any of the output parameters is a null
 * pointer.
 */
SABuilder_Status_t
SABuilder_UpdateCtrlOffset_Get(
        uint32_t ControlWord,
        unsigned int * const UpdCtrlOffset_p);


/**---------------------------------------------------------------------------
 * @fn SABuilder_UpdateCtrl_Update(
 *       const SABuilder_UpdCtrl_Params_t * const UpdateParams_p,
 *       uint32_t * const SAWord_p);
 *
 * Compose the SA Update Control word from the supplied parameters.
 *
 * @param [in] UpdateParams_p
 *     Parameters describing the SA Update Control word.
 *
 * @param [out] SAWord_p
 *     32-bit SA Update control word to be written to the SA record.
 *
 * @return SAB_STATUS_OK: success
 * @return SAB_INVALID_PARAMETER: any of the output parameters is a null
 * pointer.
 */
SABuilder_Status_t
SABuilder_UpdateCtrl_Update(
        const SABuilder_UpdCtrl_Params_t * const UpdateParams_p,
        uint32_t * const SAWord_p);


/**---------------------------------------------------------------------------
 * @fn SABuilder_MTU_Update(
 *       const SABuilder_MTU_Params_t * const UpdateParams_p,
 *       uint32_t * const SAWord_p);
 *
 * Compose the SA MTU word from the supplied parameters.
 *
 * @param [in] UpdateParams_p
 *     Parameters describing the SA Update Control word.
 *
 * @param [out] SAWord_p
 *     32-bit SA Update control word to be written to the SA record.
 *
 * @return SAB_STATUS_OK: success
 * @return SAB_INVALID_PARAMETER: any of the output parameters is a null
 * pointer.
 */
SABuilder_Status_t
SABuilder_MTU_Update(
        const SABuilder_MTU_Params_t * const UpdateParams_p,
        uint32_t * const SAWord_p);


/**---------------------------------------------------------------------------
 * @fn SABuilder_MTU_Decode(
 *       const uint32_t SAWord,
 *       SABuilder_MTU_Params_t * const UpdateParams_p);
 *
 * Decode the SA MTU word into a parameter structure that can be supplied
 * to SABuilder_MTU_Update
 *
 * @param [in] SAWord
 *     32-bit SA MTU word to be written to the SA record.
 *
 * @param [out] UpdateParams_p
 *     Parameters describing the SA MTU word.
 *
 * @return SAB_STATUS_OK: success
 * @return SAB_INVALID_PARAMETER: any of the output parameters is a null
 * pointer.
 */
SABuilder_Status_t
SABuilder_MTU_Decode(
        const uint32_t SAWord,
        SABuilder_MTU_Params_t * const UpdateParams_p);


#endif /* SA_BUILDER_MACSEC_H_ */


/* end of file sa_builder_macsec.h */
