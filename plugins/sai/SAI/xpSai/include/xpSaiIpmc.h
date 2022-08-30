// xpSaiIpmc.h

/*******************************************************************************
* copyright (c) 2021 marvell. all rights reserved. the following file is       *
* subject to the limited use license agreement by and between marvell and you, *
* your employer or other entity on behalf of whom you act. in the absence of   *
* such license agreement the following file is subject to marvell’s standard   *
* limited use license agreement.                                               *
********************************************************************************/

#ifndef _xpSaiIpmc_h_
#define _xpSaiIpmc_h_

#include "xpSai.h"

#ifdef __cplusplus
extern "C" {
#endif

/** Defines SAI limit for IP Multicast entries */
#define MAX_IPMC_ENTRIES 1024*16

/**
 * @brief SAI IP Multicast Entry Attributes struct
 */
typedef struct _xpSaiIpmcAttributesT
{
    sai_int32_t pktCmd;
    sai_object_id_t groupId;
    sai_object_id_t rpfGroupId;

} xpSaiIpmcAttributesT;

/**
 * \brief Standard xpSai module initialization interface
 */
sai_status_t xpSaiIpmcApiInit(uint64_t flag,
                              const sai_service_method_table_t* adapHostServiceMethodTable);
sai_status_t xpSaiIpmcApiDeinit();

#ifdef __cplusplus
}
#endif

#endif //_xpSaiIpmc_h_
