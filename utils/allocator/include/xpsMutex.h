// xpsMutex.h

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
********************************************************************************/

#ifndef __XP_MUTEX_H__
#define __XP_MUTEX_H__

#if defined (__unix__) || (defined (__APPLE__) && defined (__MACH__))
#include <unistd.h>
#endif

#include "xpTypes.h"


// ****************************************************************************
// xpBaseLog
// ****************************************************************************

class xpsMutex
{
public:

    xpsMutex(void);
    ~xpsMutex(void);
    void lock(void);
    void unlock(void);
};

#endif // __XP_MUTEX_H__

