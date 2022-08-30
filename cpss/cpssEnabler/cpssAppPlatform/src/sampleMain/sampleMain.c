/*******************************************************************************
*              (c), Copyright 2018, Marvell International Ltd.                 *
* THIS CODE IS A REFERENCE CODE FOR MARVELL SWITCH PRODUCTS.  IT IS PROVIDED   *
* "AS IS" WITH NO WARRANTIES, EXPRESSED, IMPLIED OR OTHERWISE, REGARDING ITS   *
* ACCURACY, COMPLETENESS OR PERFORMANCE.                                       *
* CUSTOMERS ARE FREE TO MODIFY IT AND USE IT ONLY IN THEIR PRODUCTION          *
* SOFTWARE RELEASES WITH MARVELL SWITCH CHIPSETS.                              *
*******************************************************************************/
/**
********************************************************************************
* @file sampleMain.c
*
* @brief Sample main function for CPSS App Platform.
*
* @version   1
********************************************************************************
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef LINUX_SIMULATION
#include <errno.h>
#endif

#include <cpssAppPlatformSysConfig.h>

#ifdef LUA_CLI_SAMPLE
#include <cmdShell/shell/cmdMain.h>
#include <cmdShell/os/cmdOs.h>
#include <termios.h>
#include <gtOs/gtOsInit.h>

static struct termios tc_saved;
static void restore_tty(void)
{
    tcsetattr(0, TCSAFLUSH, &tc_saved);
    printf("tty settings restored\n");
}

void cmdAppInit(void)
{
    GT_STATUS rc;
    rc = cmdInit(0);
    if (rc != GT_OK)
    {
        printf("cmdInit() failed, rc=%d\n", rc);
    }
}
#endif

#ifdef LINUX_SIMULATION
extern int initializeSimulation(GT_VOID);
/* commandLine should belong to simulation (simulation can be used by another client withot this var) */
extern char commandLine[1280];
char **cmdArgv;
char **cmdEnv;
#endif

extern GT_STATUS userAppInit(GT_VOID);

int main(int argc, const char * argv[], char *envp[] )
{
    int i =0;
    extern CPSS_APP_PLATFORM_PROFILE_STC DB_ALD2_48MG6CG_all[];
#ifdef LINUX_SIMULATION
    int len=0;
    cmdArgv = calloc(argc+1, sizeof(char*));
    if (cmdArgv == NULL)
    {
        fprintf(stderr, "calloc(%d, %d) failed: %s\n",
                argc+1, sizeof(char*), strerror(errno));
        return 1;
    }
    commandLine[0] = 0;
    for (i=0; i<argc; i++)
    {
        len += strlen(argv[i]);
        cmdArgv[i] = argv[i];
        if ((unsigned)len < sizeof(commandLine)-2) /*for ' ' and '\0' */
        {
            strcat(commandLine,argv[i]);
            if (i<argc-1)
            {
                strcat(commandLine," ");
                len++;
            }
        }

    }
    cmdArgv[argc] = 0;
    printf("You are running on Linux Simulation \n");
    initializeSimulation();
#endif

    /* here we will map the pss memory */
    userAppInit();

    CPSS_APP_PLATFORM_PROFILE_STC *profilePtr = DB_ALD2_48MG6CG_all;/*variable name;*/
    cpssAppPlatformSystemInit(profilePtr,GT_FALSE);

#ifdef LUA_CLI_SAMPLE
    for (i=1; i < argc; i++)
    {
        if (strcmp("-luacli", argv[i]) == 0)
        {
            printf("LUA CLI for Sample Main Application......... \n");
            cmdDefaultShellType = CMD_STARTUP_SHELL_LUACLI_E;
            cmdOsConfigFileName = NULL;

            tcgetattr(0, &tc_saved);
            atexit(restore_tty);

            if (osStartEngine(argc, argv, "SampleMain", cmdAppInit) != GT_OK)
                return 1;
        }
    }
#endif
    printf("Exiting Sample Main App-----\n");
    return 0;
}
