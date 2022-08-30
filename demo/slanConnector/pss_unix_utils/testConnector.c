// testConnector.c

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
********************************************************************************/

#include <stdio.h>
#include <unistd.h>
#include "slanLib.h"

int main()
{
    SLAN_ID slan;

    slanLibInit(1,"testConnector");

    return slanLibBind("testSlan", (SLAN_LIB_EVENT_HANDLER_FUNC)NULL, NULL, &slan);
}
