// xpSaiMcastFdb.h

/*******************************************************************************
* copyright (c) 2021 marvell. all rights reserved. the following file is       *
* subject to the limited use license agreement by and between marvell and you, *
* your employer or other entity on behalf of whom you act. in the absence of   *
* such license agreement the following file is subject to marvell’s standard   *
* limited use license agreement.                                               *
********************************************************************************/

#ifndef _xpSaiMcastFdb_h_
#define _xpSaiMcastFdb_h_

#include "xpSai.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief SAI Multicast FDB Attributes struct
 */
typedef struct _xpSaiMcastFdbAttributesT
{
    sai_int32_t pktCmd;
    sai_object_id_t groupId;

} xpSaiMcastFdbAttributesT;

/**
 * \brief Standard xpSai module initialization interface
 */
sai_status_t xpSaiMcastFdbApiInit(uint64_t flag,
                                  const sai_service_method_table_t* adapHostServiceMethodTable);
sai_status_t xpSaiMcastFdbApiDeinit();

#ifdef __cplusplus
}
#endif

#endif //_xpSaiMcastFdb_h_
