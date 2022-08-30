/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
*******************************************************************************/

/**
********************************************************************************
* @file cpssHalSysKern.c
*
* @brief defines functions to load/unload Kernel modules
*
* @version   01
********************************************************************************
*/

#include "gtGenTypes.h"

#define _GNU_SOURCE
#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>

#define insmod(module, len, params)     syscall(__NR_init_module, module, len, params)
#define finsmod(fd, params, flags)      syscall(__NR_finit_module, fd, params, flags)
#define rmmod(name, flags)              syscall(__NR_delete_module, name, flags)

#ifndef MODULE_INIT_IGNORE_MODVERSIONS
#define MODULE_INIT_IGNORE_MODVERSIONS  1
#endif
#ifndef MODULE_INIT_IGNORE_VERMAGIC
#define MODULE_INIT_IGNORE_VERMAGIC 2
#endif

GT_STATUS load_kernel_module(char *mod_file_name, char *params)
{
    int fd, tryFinit;
    size_t fileSize;
    struct stat st;
    void *image;

    tryFinit = 1;

    fd = open(mod_file_name, O_RDONLY);
    if (tryFinit)
    {
        if (finsmod(fd, params, 0) != 0)
        {
            perror("finsmod");
            goto tryinit;
        }
        close(fd);
    }
    else
    {
tryinit:
        fstat(fd, &st);
        fileSize = st.st_size;
        image = malloc(fileSize);
        if (read(fd, image, fileSize) == -1)
        {
            perror("read");
        }
#if 0
        if (insmod(image, fileSize, params) != 0)
        {
            close(fd);
            perror("insmod");
            return GT_FAIL;
        }
#else
        if (finsmod(fd, params,
                    MODULE_INIT_IGNORE_MODVERSIONS|MODULE_INIT_IGNORE_VERMAGIC))
        {
            close(fd);
            perror("insmod");
            free(image);
            return GT_FAIL;
        }
#endif
        close(fd);
        free(image);
    }
    return GT_OK;
}

GT_STATUS remove_kernel_module(char *mod_name)
{

    if (rmmod(mod_name, O_NONBLOCK) != 0)
    {
        perror("delete_module");
        return GT_FAIL;
    }
    return GT_OK;
}

