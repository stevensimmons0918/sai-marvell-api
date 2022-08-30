// xpSaiHash.h

/*******************************************************************************
* copyright (c) 2021 marvell. all rights reserved. the following file is       *
* subject to the limited use license agreement by and between marvell and you, *
* your employer or other entity on behalf of whom you act. in the absence of   *
* such license agreement the following file is subject to marvell’s standard   *
* limited use license agreement.                                               *
********************************************************************************/

#ifndef _xpSaiHash_h_
#define _xpSaiHash_h_

#include "xpSai.h"

#ifdef __cplusplus
extern "C" {
#endif

#define XP_SAI_HASH_NATIVE_FIELDS_COUNT SAI_NATIVE_HASH_FIELD_NONE
/**
 * \brief State structure maintained by HASH for single layer
 *
 * This structure state contains the HASH configuration for single layer
 *
 * This state is internal and not exposed to the user
 */
typedef struct xpSaiHwHashState_t
{
    bool enable;
    xpHashField fieldList[XP_SAI_HASH_NATIVE_FIELDS_COUNT];
    sai_uint8_t fieldsNumber;
} xpSaiHwHashState_t;

/**
 * \brief State structure maintained by HASH
 *
 * This structure state contains the HASH configuration
 *
 * This state is internal and not exposed to the user
 */
typedef struct xpSaiHashDbEntry_t
{
    sai_object_id_t keyHashId; //< HASH identifier. It is as a key
    bool nativeHashFields[XP_SAI_HASH_NATIVE_FIELDS_COUNT];
    sai_uint32_t hashFieldNumber;
    xpSaiHwHashState_t hwState[NUM_LAYERS];

} xpSaiHashDbEntry_t;

/**
 * \file xpSaiHash.h
 * \brief This file contains API prototypes, defines and type definitions
 *        for the Hash
 */

/**
 * \brief API that initializes the Hash
 *
 * Will register for the state databases that are
 * needed by the Hash
 *
 * \return XP_STATUS
 */
XP_STATUS xpSaiHashApiInit(uint64_t flag,
                           const sai_service_method_table_t* adapHostServiceMethodTable);

/**
 * \brief API to De-Init the Hash
 *
 * This API will Deregister all state databases for Hash
 *
 * \return XP_STATUS
 */
XP_STATUS xpSaiHashApiDeinit();

/**
 * \brief API to register the Hash
 *
 * This API will register Hash
 *
 * \return sai_status_t
 */
sai_status_t xpSaiHashInit(xpsDevice_t xpsDevId);

/**
 * \brief API to deregister the Hash
 *
 * This API will deregister Hash
 *
 * \return sai_status_t
 */
sai_status_t xpSaiHashDeInit(xpsDevice_t devId);

sai_status_t xpSaiHashFieldsToXpHashFieldsConvert(const sai_int32_t *saiFields,
                                                  uint32_t count, xpSaiHwHashState_t *state, sai_object_id_t hashId);

sai_status_t xpSaiHashSetSwitchHash(uint32_t hashType,
                                    sai_object_id_t hash_oid);

sai_status_t xpSaiHashReconfigueHashMaskTables(void);

sai_status_t xpSaiHashFieldSet(uint32_t hashType, sai_object_id_t hashId,
                               bool setXOR, bool setCRC);

#ifdef __cplusplus
}
#endif

#endif //_xpSaiHash_h_
