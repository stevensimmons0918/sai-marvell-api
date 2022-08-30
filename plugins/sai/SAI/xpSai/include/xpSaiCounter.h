// xpSaiCounter.h

/*******************************************************************************
* copyright (c) 2021 marvell. all rights reserved. the following file is       *
* subject to the limited use license agreement by and between marvell and you, *
* your employer or other entity on behalf of whom you act. in the absence of   *
* such license agreement the following file is subject to marvell’s standard   *
* limited use license agreement.                                               *
********************************************************************************/

#ifndef _xpSaiCounter_h_
#define _xpSaiCounter_h_

#include "xpSai.h"

#ifdef __cplusplus
extern "C" {
#endif

XP_STATUS xpSaiCounterApiInit(uint64_t flag,
                              const sai_service_method_table_t* adapHostServiceMethodTable);
XP_STATUS xpSaiCounterApiDeinit();

#ifdef __cplusplus
}
#endif

#endif //_xpSaiCounter_h_
