/* aes.c
 *
 * AES algorithm API implementation for key lengths of 128 bits, 192 bits and
 * 256 bits
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
#include <Kit/SABuilder_MACsec/Example_AES/incl/aes.h>

/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */
#include <Kit/DriverFramework/incl/basic_defs.h>
#include <Kit/DriverFramework/incl/clib.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>
/*****************************************************************************/
/* Defines:                                                                  */
/*****************************************************************************/
#define NB 4 /* No. of columns in a state which is a constant in AES */
#define AES_BLOCK_LEN 16 /* Block length in AES is 128 bytes only */
#define AES_MAX_KEY_EXPN_SIZE  240 /** Maximum size of the expanded key */

typedef uint8_t state_t[4][4]; /* state - array holding the intermediate results */

typedef enum
{
    AES_128,
    AES_192,
    AES_256
} aes_key_types_t;

typedef struct
{
    aes_key_types_t aes_key_type;   /** AES key type, Cipher is working on */
    uint32_t key_len;
    uint16_t no_of_rounds;          /** no of rounds */
    uint8_t no_of_key_words;        /** no of (32 bit) words in the key */
} aes_obj_t;

static const uint8_t sbox[256] =
{
/*|   0  |   1  |   2  |   3  |   4  |   5  |   6  |   7  |   8  |   9  |   a  |   b  |   c  |  d   |  e   |  f   |     */
/*|------|------|------|------|------|------|------|------|------|------|------|------|------|------|------|------|     */
    0x63,  0x7c,  0x77,  0x7b,  0xf2,  0x6b,  0x6f,  0xc5,  0x30,  0x01,  0x67,  0x2b,  0xfe,  0xd7,  0xab,  0x76, /* 0 */
    0xca,  0x82,  0xc9,  0x7d,  0xfa,  0x59,  0x47,  0xf0,  0xad,  0xd4,  0xa2,  0xaf,  0x9c,  0xa4,  0x72,  0xc0, /* 1 */
    0xb7,  0xfd,  0x93,  0x26,  0x36,  0x3f,  0xf7,  0xcc,  0x34,  0xa5,  0xe5,  0xf1,  0x71,  0xd8,  0x31,  0x15, /* 2 */
    0x04,  0xc7,  0x23,  0xc3,  0x18,  0x96,  0x05,  0x9a,  0x07,  0x12,  0x80,  0xe2,  0xeb,  0x27,  0xb2,  0x75, /* 3 */
    0x09,  0x83,  0x2c,  0x1a,  0x1b,  0x6e,  0x5a,  0xa0,  0x52,  0x3b,  0xd6,  0xb3,  0x29,  0xe3,  0x2f,  0x84, /* 4 */
    0x53,  0xd1,  0x00,  0xed,  0x20,  0xfc,  0xb1,  0x5b,  0x6a,  0xcb,  0xbe,  0x39,  0x4a,  0x4c,  0x58,  0xcf, /* 5 */
    0xd0,  0xef,  0xaa,  0xfb,  0x43,  0x4d,  0x33,  0x85,  0x45,  0xf9,  0x02,  0x7f,  0x50,  0x3c,  0x9f,  0xa8, /* 6 */
    0x51,  0xa3,  0x40,  0x8f,  0x92,  0x9d,  0x38,  0xf5,  0xbc,  0xb6,  0xda,  0x21,  0x10,  0xff,  0xf3,  0xd2, /* 7 */
    0xcd,  0x0c,  0x13,  0xec,  0x5f,  0x97,  0x44,  0x17,  0xc4,  0xa7,  0x7e,  0x3d,  0x64,  0x5d,  0x19,  0x73, /* 8 */
    0x60,  0x81,  0x4f,  0xdc,  0x22,  0x2a,  0x90,  0x88,  0x46,  0xee,  0xb8,  0x14,  0xde,  0x5e,  0x0b,  0xdb, /* 9 */
    0xe0,  0x32,  0x3a,  0x0a,  0x49,  0x06,  0x24,  0x5c,  0xc2,  0xd3,  0xac,  0x62,  0x91,  0x95,  0xe4,  0x79, /* a */
    0xe7,  0xc8,  0x37,  0x6d,  0x8d,  0xd5,  0x4e,  0xa9,  0x6c,  0x56,  0xf4,  0xea,  0x65,  0x7a,  0xae,  0x08, /* b */
    0xba,  0x78,  0x25,  0x2e,  0x1c,  0xa6,  0xb4,  0xc6,  0xe8,  0xdd,  0x74,  0x1f,  0x4b,  0xbd,  0x8b,  0x8a, /* c */
    0x70,  0x3e,  0xb5,  0x66,  0x48,  0x03,  0xf6,  0x0e,  0x61,  0x35,  0x57,  0xb9,  0x86,  0xc1,  0x1d,  0x9e, /* d */
    0xe1,  0xf8,  0x98,  0x11,  0x69,  0xd9,  0x8e,  0x94,  0x9b,  0x1e,  0x87,  0xe9,  0xce,  0x55,  0x28,  0xdf, /* e */
    0x8c,  0xa1,  0x89,  0x0d,  0xbf,  0xe6,  0x42,  0x68,  0x41,  0x99,  0x2d,  0x0f,  0xb0,  0x54,  0xbb,  0x16, /* f */
};

#define GET_SBOX_VALUE(num) (sbox[(num)])

static const uint8_t round_constant[11] =
{
    0x8d, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x1b, 0x36
};

static void
aes_obj_init(
    aes_obj_t *const aes_obj,
    const unsigned int key_len)
{
    cpssOsMemSet(aes_obj, 0, sizeof(aes_obj_t));

    switch (key_len)
    {
    case 16:
        aes_obj->no_of_key_words = 4;
        aes_obj->no_of_rounds = 10;
        aes_obj->aes_key_type = AES_128;
        break;

    case 24:
        aes_obj->no_of_key_words = 6;
        aes_obj->no_of_rounds = 12;
        aes_obj->aes_key_type = AES_192;
        break;

    case 32:
        aes_obj->no_of_key_words = 8;
        aes_obj->no_of_rounds = 14;
        aes_obj->aes_key_type = AES_256;
        break;

    default:
        break;
    }

    aes_obj->key_len = key_len;
}

static void
substitute_word(
    uint8_t word[])
{
    uint8_t i;

    for (i = 0; i < NB; i++) word[i] = GET_SBOX_VALUE(word[i]);
}

static void
rotate_word(
    uint8_t word[],
    const uint8_t no_of_rotations)
{
    uint8_t i = 0;
    uint8_t temp = 0;

    for (i = 0; i < no_of_rotations; i++)
    {
        temp = word[0];
        word[0] = word[1];
        word[1] = word[2];
        word[2] = word[3];
        word[3] = temp;
    }
}

static void
aes_substitute_bytes(
    state_t *const state)
{
    uint8_t i, j;

    for (i = 0; i < 4; i++)
    {
        for (j = 0; j < NB; j++)
        {
            (*state)[j][i] = GET_SBOX_VALUE((*state)[j][i]);
        }
    }
}

static void
aes_shift_rows(
     uint8_t *const state)
{
    uint8_t tmp[16];
    int i = 0;

    tmp[0] = state[0];
    tmp[1] = state[5];
    tmp[2] = state[10];
    tmp[3] = state[15];

    tmp[4] = state[4];
    tmp[5] = state[9];
    tmp[6] = state[14];
    tmp[7] = state[3];

    tmp[8] = state[8];
    tmp[9] = state[13];
    tmp[10] = state[2];
    tmp[11] = state[7];

    tmp[12] = state[12];
    tmp[13] = state[1];
    tmp[14] = state[6];
    tmp[15] = state[11];

    for (i = 0; i < 16; i++) state[i] = tmp[i];
}

/**
 * Multiplies the argument by 2 in GF(2^8)
 *
 * The multiplications implicit within the MixColumns operation are GF(2^8)
 * multiplication operations
 */
static void
aes_xtime(
    uint8_t *const val)
{
    uint8_t val1 = (uint8_t)(*val << 1);
    uint8_t val2 = (uint8_t)(*val >> 7);

    *val = (uint8_t)val1 ^ (uint8_t)((val2 & 1) * 0x1B);
}

/* Performs the mix columns diffusion
 *
 * Takes a 16 char byte array and transforms to new 16 byte char array that
 * represents the GF(256) matrix multiplication of a known matrix times the
 * state. The byte array represents a 4x4 matrix where the first 4 entries
 * represents the first column.
 *
 * state: 16 unsigned char byte array to transform
 */
static void
aes_mix_columns(
    state_t *const state)
{
    uint8_t i = 0;

    for (i = 0; i < 4; ++i)
    {
        uint8_t temp1 = 0, temp2 = 0, temp3 = 0;

        temp3 = (*state)[i][0];
        temp1 = (*state)[i][0] ^ (*state)[i][1] ^ (*state)[i][2] ^ (*state)[i][3];
        temp2 = (*state)[i][0] ^ (*state)[i][1];

        aes_xtime(&temp2);

        (*state)[i][0] = (*state)[i][0] ^ (temp2 ^ temp1);

        temp2 = (*state)[i][1] ^ (*state)[i][2];

        aes_xtime(&temp2);

        (*state)[i][1] = (*state)[i][1] ^ (temp2 ^ temp1);

        temp2 = (*state)[i][2] ^ (*state)[i][3];

        aes_xtime(&temp2);

        (*state)[i][2] = (*state)[i][2] ^ (temp2 ^ temp1);

        temp2 = (*state)[i][3] ^ temp3;

        aes_xtime(&temp2);

        (*state)[i][3] = (*state)[i][3] ^ (temp2 ^ temp1);
    }
}

static void
aes_key_schedule(
    const aes_obj_t *const aes_obj,
    uint8_t expanded_key[],
    const uint8_t *const input_key)
{
    uint16_t i, j;
    uint8_t temp[4];

    /* Input key is copied as the first key of the expanded key */
    for (i = 0; i < 4 * aes_obj->no_of_key_words ; i++)
    {
        expanded_key[i] = input_key[i];
    }

    /**
     * Each added word depends on the immediately preceding word, and the
     * word four positions back
     */
    for (i = (uint16_t) (NB * aes_obj->no_of_key_words);
         i < (4 * NB *(aes_obj->no_of_rounds + 1));
         i = (uint16_t) (i + 4))
    {
        for (j = 0; j < NB; j++) temp[j] = expanded_key[i - 4 + j];

        if (i / NB % aes_obj->no_of_key_words == 0)
        {
            rotate_word(temp, 1);

            substitute_word(temp);

            temp[0] = temp[0] ^ round_constant[i / (aes_obj->no_of_key_words * 4)];
        }
        else if (aes_obj->no_of_key_words > 6 &&
                 i / 4 % aes_obj->no_of_key_words == 4)
        {
            substitute_word(temp);
        }

        for (j = 0; j < NB; j++) expanded_key[i + j] = expanded_key[i + j - 4 * aes_obj->no_of_key_words] ^ temp[j];
    }
}

static void
aes_add_round_key(
    state_t *const state,
    const uint8_t *const round_key)
{
    uint8_t i, j;

    for (i = 0; i < 4; i++)
    {
        for (j = 0; j < NB; j++)
        {
            (*state)[i][j] ^= round_key[(i * NB) + j];
        }
    }
}


void
AES_Encrypt(
    const uint8_t *const pInput,
    uint8_t *const pOutput,
    const uint8_t *const pKey,
    const unsigned int nKeyLen)
{
    aes_obj_t aes_obj;
    uint8_t expanded_key[AES_MAX_KEY_EXPN_SIZE];
    uint8_t round = 0;

    if (nKeyLen != 16 && nKeyLen != 24 && nKeyLen != 32)
    {
        return;
    }

    if (NULL == pInput || NULL == pOutput || NULL == pKey)
    {
        return;
    }

    /** Initialise the AES object with the required configuration */
    aes_obj_init(&aes_obj, nKeyLen);

    /** Prepare the round keys */
    aes_key_schedule(&aes_obj, expanded_key, pKey);

    /*
     * Copy the plain data to the output buff. When encryption operation is
     * completed, the output buffer contains the encrypted data
     */
    cpssOsMemCpy(pOutput, pInput, AES_BLOCK_LEN);

    /* Add the First round key to the state before starting the rounds */
    aes_add_round_key((state_t *const)pOutput, expanded_key);

    /* There will be Nr rounds.
     * The first Nr-1 rounds are identical.
     * These Nr-1 rounds are executed in the loop below.
     */
    for (round = 1; round < aes_obj.no_of_rounds; round++)
    {
        aes_substitute_bytes((state_t *) pOutput);
        aes_shift_rows(pOutput);
        aes_mix_columns((state_t *) pOutput);
        aes_add_round_key((state_t *)pOutput, expanded_key + (round * 16));
    }

    aes_substitute_bytes((state_t *) pOutput);
    aes_shift_rows(pOutput);
    aes_add_round_key((state_t *)pOutput, expanded_key + (round * 16));
}

