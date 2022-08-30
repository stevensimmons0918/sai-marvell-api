// xpSaiUdf.h

/*******************************************************************************
* copyright (c) 2021 marvell. all rights reserved. the following file is       *
* subject to the limited use license agreement by and between marvell and you, *
* your employer or other entity on behalf of whom you act. in the absence of   *
* such license agreement the following file is subject to marvell’s standard   *
* limited use license agreement.                                               *
********************************************************************************/

#ifndef _xpSaiUdf_h_
#define _xpSaiUdf_h_

#include "xpSai.h"

#ifdef __cplusplus
extern "C" {
#endif

XP_STATUS xpSaiUdfApiInit(uint64_t flag,
                          const sai_service_method_table_t* adapHostServiceMethodTable);
XP_STATUS xpSaiUdfApiDeinit();

#ifdef __cplusplus
}
#endif

#endif //_xpSaiUdf_h_
