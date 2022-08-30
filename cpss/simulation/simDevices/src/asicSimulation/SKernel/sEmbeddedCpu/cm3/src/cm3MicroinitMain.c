/*******************************************************************************
*
*         Copyright 2003, MARVELL SEMICONDUCTOR ISRAEL, LTD.                   *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL.                      *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED AS IS. MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
*                                                                              *
* MARVELL COMPRISES MARVELL TECHNOLOGY GROUP LTD. (MTGL) AND ITS SUBSIDIARIES, *
* MARVELL INTERNATIONAL LTD. (MIL), MARVELL TECHNOLOGY, INC. (MTI), MARVELL    *
* SEMICONDUCTOR, INC. (MSI), MARVELL ASIA PTE LTD. (MAPL), MARVELL JAPAN K.K.  *
* (MJKK), MARVELL SEMICONDUCTOR ISRAEL. (MSIL),  MARVELL TAIWAN, LTD. AND      *
* SYSKONNECT GMBH.                                                             *
********************************************************************************
* microinit_main.c
*
* DESCRIPTION:
*       This file implements microinit entry point
*
* DEPENDENCIES:
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*******************************************************************************/

#include <gtOs/gtGenTypes.h>
#include <cm3FileOps.h>
#include <cm3ConfigFileParser.h>
#include <stdio.h>      /* printf, NULL */
#include <cm3NetPort.h>
#include <cm3BootChannel.h>

#include <asicSimulation/SCIB/scib.h>
CM3_SIM_PP_FAMILY_TYPE_ENT deviceType;


#define MARK_AVAGO_FW_SBUS  1
#define MARK_AVAGO_FW_SPICO 2
#define MARK_AVAGO_FW_SWAP  4

#define MARK_AVAGO_FW_ALL   ((MARK_AVAGO_FW_SBUS) | (MARK_AVAGO_FW_SPICO))

#define NUMBER_OF_MI_PHASES  2

#define DEVICE_FOUND(_devType)\
    do\
    {\
        devType = _devType;\
        deviceName = #_devType;\
    }while(0);

/* support ability to start with 'GT_TRUE' every time microInitCm3Trigger(...)
    is called.
    fix JIRA : IPBUSW-10230 : [CN10KAS] Ping doesn't happen with switch ports
    in u-boot following a reboot from kernel after the first time it works
*/
static GT_BOOL restart = GT_TRUE;

/**
 *
 * microinit_process_files
 *
 * @brief apply supported super image files upon phase
 * @param phase: 0- apply all Avago FW related files; 1- others
 *
 * @return MV_STATUS
 */
static GT_STATUS microinit_process_files(int phase)
{
    GT_STATUS ret;
    file_params_STC file_params, *file_params_ptr;
    static GT_U8 avago_fw_files_tracking = 0;

    if (phase == 0)
        SIM_CM3_LOG("processing files:\n");

    file_params_ptr = &file_params;
    while ((ret = cm3GetNextFile(file_params_ptr, restart)) != GT_FALSE) {
        restart = GT_FALSE;
        /* In phase 0, process until all SERDES FW files found, then quit */

      SIM_CM3_LOG("file: %s, type=%d size=%d address= 0x%08x useBmp=0x%08x \n"
                   "======================================================\n",
        file_params_ptr->name, file_params_ptr->type,
        file_params_ptr->size, file_params_ptr->address,file_params_ptr->useBmp);

        /* Process only files with usage[0] == 1 */
        if(( file_params_ptr->useBmp  & 0x01 ) == 0x0 )
            continue;

        /* Process file by type */
        switch (file_params_ptr->type) {

        case AVAGO_FW_sbus:
             /*not supported*/
            avago_fw_files_tracking |= MARK_AVAGO_FW_SBUS;
            SIM_CM3_LOG("Not parsed\n\n");
            break;

        case AVAGO_FW_spico:
             /*not supported*/
            avago_fw_files_tracking |= MARK_AVAGO_FW_SPICO;
            SIM_CM3_LOG("Not parsed\n\n");
            break;

        case AVAGO_FW_swap:
            /*not supported*/
            avago_fw_files_tracking |= MARK_AVAGO_FW_SWAP;
            SIM_CM3_LOG("Not parsed\n\n");
            break;

        case AP_FW:
            /*not supported*/
            SIM_CM3_LOG("Not parsed\n\n");
            break;

        case PHY_FW:
             /*not supported*/
            SIM_CM3_LOG("Not parsed\n\n");
            break;

        case script:
        case EPROM:
            SIM_CM3_LOG("Parsing started\n\n");
            cm3lnConfigFileParserParse((const char *)file_params_ptr->address, file_params_ptr->size);
            SIM_CM3_LOG("Parsing ended\n\n");
            break;

        default:
            SIM_CM3_LOG("Not parsed\n\n");
            break;
        }
        if (phase == 0)
            if (avago_fw_files_tracking == MARK_AVAGO_FW_ALL)
                break;
    }

    return GT_OK;
}

/* According to Bond<PROD_ID[2:0]>:
0x0 (SA) : 0x90
0x1 (106s): 0x21
0x2 : 0x22
0x3 : 0x23
0x4 : 0x24
0x5 : 0x25
0x6 : 0x26
0x7 : 0x27
*/
#define IS_HARRIER_FAMILY(_devId)        \
    ( ((_devId & 0xFF00)==0x9000) ? 1 :  \
     (((_devId & 0xFF00)>=0x2100) && (_devId & 0xFF00)<=0x2700) ? 1 : 0)

/**
 *
 * microInitCm3InitHwStructure (Copy from microinit_init_hw_stracture)
 *
 * @brief Detect device type
 *
 * @return MV_STATUS
 */

static GT_STATUS microInitCm3InitHwStructure(void)
{
    CM3_SIM_PP_FAMILY_TYPE_ENT devType;
    GT_U32 data;
    char * deviceName;

    scibPciRegRead(0,0x0,1,&data);
    data>>=16;
    SIM_CM3_LOG("MI HW init: Dev ID = 0x%x\n", data);

    if(IS_HARRIER_FAMILY(data))
    {
        DEVICE_FOUND(CM3_SIM_PP_FAMILY_DXCH_HARRIER_E);
    }
    else
    {
        switch (data&0xFC00)
        {
            case 0x8400:
                DEVICE_FOUND(CM3_SIM_PP_FAMILY_DXCH_FALCON_E);
                break;
            case 0x9400:
                DEVICE_FOUND(CM3_SIM_PP_FAMILY_DXCH_AC5P_E);
                break;
            default:
                SIM_CM3_LOG("unknown device type\n");
                return GT_FAIL;
        }
    }

    deviceType = devType;

    SIM_CM3_LOG("Supported device %s found \n",deviceName);
    return GT_OK;
}

extern GT_CHAR_PTR  bootChannelGetFwVersion(void);
extern GT_U8*       getImageVersion(GT_U8 *imagePartition);

/**
 *
 * microInitCm3Trigger
 *
 * @brief Start micro init code
 *
 * @return MV_STATUS
 */

GT_VOID microInitCm3Trigger
(
    GT_VOID
)
{
    GT_U32 i;
    GT_STATUS rc = GT_OK;

    /* fix JIRA : IPBUSW-10230 : [CN10KAS] Ping doesn't happen with switch ports
       in u-boot following a reboot from kernel after the first time it works */
    restart = GT_TRUE;

    if(GT_OK!=microInitCm3InitHwStructure())
    {
       SIM_CM3_LOG("Device detection failed\n");
       return ;
    }

    /* use 'stderr' so the simics will print it in the main terminal */
    fprintf( stderr,"Started : microInitCm3Trigger \n");

    fprintf( stderr,"cm3SimShmIpcInit() \n");
    cm3SimShmIpcInit();

    cm3SimShmIpcBootChannelStateSet(MICRO_INIT_BOOT_CH_BOOT_INIT_STATE_IN_PROGRESS_E);

    fprintf( stderr,"mvNetPortInit() \n");
    mvNetPortInit();

    fprintf( stderr,"simCm3PollingInit() \n");
    simCm3PollingInit();



    for(i=0;i<NUMBER_OF_MI_PHASES;i++)
    {
        fprintf( stderr,"microinit_process_files(%d) \n",i);

        rc = microinit_process_files(i);
        if(rc!=GT_OK)
        {
            SIM_CM3_LOG("Phase %d fail\n",i);
            break;
        }
    }

    if(rc == GT_OK)
    {
        fprintf( stderr,"The 'Supper Image' version : [%s]\n",getImageVersion(NULL));
        fprintf( stderr,"The 'CM3 FW ' version : [%s]\n",bootChannelGetFwVersion());

       cm3SimShmIpcBootChannelStateSet(MICRO_INIT_BOOT_CH_BOOT_INIT_STATE_DONE_E);
    }
    else
    {
        cm3SimShmIpcBootChannelStateSet(MICRO_INIT_BOOT_CH_BOOT_INIT_STATE_FAILED_E);
    }

    fprintf( stderr,"simCm3LinkUp() \n");
    simCm3LinkUp();

    fprintf( stderr,"ENDED : microInitCm3Trigger \n");
}




CM3_SIM_PP_FAMILY_TYPE_ENT microInitCm3HostPpFamilyGet
(
    GT_VOID
)
{
    return deviceType;
}



