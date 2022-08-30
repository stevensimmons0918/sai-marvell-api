/* aes.h
*
* Include file for AES definitions
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

#ifndef AES_H
#define AES_H

/*************************** HEADER FILES ***************************/
#include <Kit/DriverFramework/incl/basic_defs.h>


/*----------------------------------------------------------------------------
* AES_Encrypt
*
* Wrapper function as required by the SA Builder. Encrypts a single
* data block using AES.
*
* pInput
*     Pointer to a 16 byte buffer with the plain data.
*
* pOutput
*     Pointer to a 16 bytes buffer with received the encrypted data.
*
* pKey
*     Pointer to the encryption key.
*
* nKeyLen
*     The length of the encryption key in bytes.
*     This must be a value if 16, 24 or 32.
*
* Return Value:
*     No return value.
*/
void
AES_Encrypt(
    const uint8_t *const pInput,
    uint8_t *const pOutput,
    const uint8_t *const pKey,
    const unsigned int nKeyLen);

#endif   /* AES_H */

/* end of file aes.h */

