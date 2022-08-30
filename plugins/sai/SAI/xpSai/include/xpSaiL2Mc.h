// xpSaiL2Mc.h

/*******************************************************************************
* copyright (c) 2021 marvell. all rights reserved. the following file is       *
* subject to the limited use license agreement by and between marvell and you, *
* your employer or other entity on behalf of whom you act. in the absence of   *
* such license agreement the following file is subject to marvell’s standard   *
* limited use license agreement.                                               *
********************************************************************************/

#ifndef _xpSaiL2Mc_h_
#define _xpSaiL2Mc_h_

#include "xpSai.h"

#ifdef __cplusplus
extern "C" {
#endif

/** Defines SAI limit for L2 Multicast entries */
#define MAX_L2_MC_ENTRIES 1024*16

/**
 * @brief SAI L2 Multicast Entry Attributes struct
 */
typedef struct _xpSaiL2McAttributesT
{
    sai_int32_t pktCmd;
    sai_object_id_t groupId;

} xpSaiL2McAttributesT;

typedef struct xpSaiL2McDbEntry_t
{
    sai_object_id_t switch_id;
    sai_object_id_t bv_id;
    sai_l2mc_entry_type_t type;
    sai_ip_address_t destination;
    sai_ip_address_t source;

    xpSaiL2McAttributesT l2McAttr;

} xpSaiL2McDbEntry_t;


/**
 * \brief Standard xpSai module initialization interface
 */
sai_status_t xpSaiL2McApiInit(uint64_t flag,
                              const sai_service_method_table_t* adapHostServiceMethodTable);
sai_status_t xpSaiL2McApiDeinit();

sai_status_t xpSaiL2McInit(xpsDevice_t xpSaiDevId);
sai_status_t xpSaiL2McDeInit(xpsDevice_t xpSaiDevId);


#ifdef __cplusplus
}
#endif

#endif //_xpSaiL2Mc_h_
