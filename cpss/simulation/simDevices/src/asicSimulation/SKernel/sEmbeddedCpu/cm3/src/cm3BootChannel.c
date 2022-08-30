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
* cm3BootChannel.c
*
* DESCRIPTION:
*       This file implements boot channel
*
* DEPENDENCIES:
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*******************************************************************************/

#include <gtOs/gtGenTypes.h>
#include <cm3FileOps.h>
#include <cm3ConfigFileParser.h>
#include <cm3NetPort.h>
#include <asicSimulation/SCIB/scib.h>
#include <cm3BootChannel.h>

#include <string.h>
#include <stdio.h>      /* printf, NULL */


#define FW_OWNERSHIP_BIT 0
#define NUM_OF_FILES_IN_GROUP 10


#define IPC_SHM_MAGIC       0x17354628
#define IPC_SHM_CHANNELS    6
#define IPC_SHM_NUM_REGS    32

#define IPC_BOOT_CHANNEL_MESSAGE_BUFFER_LEN 252
#define IPC_BOOT_CHANNEL_MESSAGE_MAX_LEN  (IPC_BOOT_CHANNEL_MESSAGE_BUFFER_LEN - 1)
#define BOOT_CHANNEL_COMMAND_STRING 20

/*From freeRTOS/boot/code/bootstrap.c*/
#define __BOOTON_VERSION__ "1.0.19"


#define __FW_VERSION__ "1.4.3"

const GT_CHAR_PTR cm3SimBootonVersion=__BOOTON_VERSION__;

/*Ability to set explicitly FW version.Needed for testing.Not present in real MI*/
GT_CHAR cm3SimFwVersion[BOOT_CHANEL_VERSION_STRING_LEN]={'\0'};
#define MAX_VER_NUM 20

#define CONF_PROCESSOR_MEMORY_SIZE _256K

/*#define CONF_PROCESSOR_MEMORY_SIZE (384*1024)*/


/*Last 2K is for boot channel*/


#define CONF_PROCESSOR_BOOT_CHANNEL_START (CONF_PROCESSOR_MEMORY_SIZE - BOOT_CHANNEL_SIZE)

#define IPC_SHM_ALIGN(_size)     (((_size)+3)&(~3))



typedef enum {  MICRO_INIT_BOOT_CH_FW_ERR_NOR_FLASH_NOT_EXIST_E                = 0x001,
                MICRO_INIT_BOOT_CH_FW_ERR_NOR_FLASH_NOT_SUPPORTED_E            = 0x002,
                MICRO_INIT_BOOT_CH_FW_ERR_PRIMARY_SUPER_IMAGE_INVALID_CRC_E    = 0x010,
                MICRO_INIT_BOOT_CH_FW_ERR_SECONDARY_SUPER_IMAGE_INVALID_CRC_E  = 0x020,
                MICRO_INIT_BOOT_CH_FW_ERR_CUSTOMER_PRIMARY_SUPER_IMAGE_INVALID_CRC_E   = 0x040,
                MICRO_INIT_BOOT_CH_FW_ERR_CUSTOMER_SECONDARY_SUPER_IMAGE_INVALID_CRC_E = 0x080,
                MICRO_INIT_BOOT_CH_FW_ERR_EEPROM_LIKE_FILE_PROCESSING_FAILED_E = 0x200,
                MICRO_INIT_BOOT_CH_FW_ERR_CLI_LIKE_FILE_PROCESSING_FAILED_E    = 0x400
} MICRO_INIT_BOOT_CH_FW_ERR_ENT;

typedef enum {  BOOT_CH_MSG_OPCODE_INVALID         = 0x1,
                BOOT_CH_MSG_OPCODE_PROCESS_FAILURE = 0x2
} MICRO_INIT_BOOT_CH_OPCODE_ERR_ENT;


#define BC_PARAM(i)     msg[2+i*4]  /* First index 0 */
#define BC_PARAM_16(i)  (*(GT_U16*)(void*)(msg+2+i*4))
#define DEV_NUM_0                   SIM_CM3_DEVICE_ID
#define ALL_PORTS_TOKEN             0xFF
#define PORT_STATUS_HWS_MODE_OFFSET 8
#define FEC_MODE_ARR_SIZE           3


/*
 * Typedef: enum IPC_SHM_SYNC_FUNC_MODE_ENT
 *
 * Description: Defines operations for IPC_SHM_SYNC_FUNC
 *
 * Enumerations:
 *      IPC_SHM_SYNC_FUNC_MODE_READ_E
 *                          - Direct read from target's absolute address
 *                            ptr is pointer to store data to
 *                            targetPtr is target's address
 *                            size is data size to read
 *      IPC_SHM_SYNC_FUNC_MODE_WRITE_E
 *                          - Direct write to target's absolute address
 *                            ptr is pointer to data
 *                            targetPtr is target's address
 *                            size is data size to write
 */
typedef enum {
    IPC_SHM_SYNC_FUNC_MODE_READ_E,
    IPC_SHM_SYNC_FUNC_MODE_WRITE_E
} IPC_SHM_SYNC_FUNC_MODE_ENT;

typedef enum {  MICRO_INIT_BOOT_CH_RET_CODE_OK_E = 0x0,
                MICRO_INIT_BOOT_CH_RET_CODE_INVALID_SYNTAX_E = 0x1,
                MICRO_INIT_BOOT_CH_RET_CODE_INVALID_CONFIG_ID_E = 0x10,
                MICRO_INIT_BOOT_CH_RET_CODE_NO_MATCH_FILE_E = 0x11
} MICRO_INIT_BOOT_CH_RET_CODE_ENT;


typedef void (*IPC_SHM_SYNC_FUNC)
(
    IN  void*   cookie,
    IN  IPC_SHM_SYNC_FUNC_MODE_ENT mode,
    IN  void*   ptr,
    IN  GT_UINTPTR targetPtr,
    IN  GT_U32 size
);


/*
 * typedef: struct IPC_SHM_STC
 *
 * Description: A control structure for low-level IPC
 *
 * Fields:
 *
 *   shm          - virtual address IPC shared memory block
 *   shmLen       - length of IPC shared memory block
 *   master       - role (master/slave).
 *                  Each channel is a pair of FIFOs
 *                  For master the FIFOs are Rx,Tx
 *                  For slave - Tx, Rx
 *
 * Comments:
 *   This management structure stored in application memory, not on shared mem.
 *   Therefore, a copy of such structure exists both on master and slave.
 *   shm points to virtual address of the shared memory, from application space.
 *
 */
typedef struct CM3_SIM_IPC_SHM_STCT {
    GT_U32              shmAddr;/*GT_UINTPTR          shm;*/
    GT_U32              shmLen;
    int                 master;
    IPC_SHM_SYNC_FUNC   syncFunc;
    void*               syncCookie;
} CM3_SIM_IPC_SHM_STC;


/**
 * \brief Shared memory IPC header.Copy of IPC_SHM_DATA_STC
 *
 *  This structure is stored on the start of shared memory, accessible by
 *  master and slave. Actual offset in shared memory is flexible.
 */
typedef struct CM3_SIM_IPC_SHM_DATA_STCT {
    GT_U32      bootChnHdr;         /*!< bit 31: owner, bits 30..18 - size, 0 == free */
    GT_U32      bootChnData[63];    /*!< data[0] == opcode */
    GT_U32      registers[IPC_SHM_NUM_REGS];  /*!< boot channel virtual registers */
    GT_U32      magic;
    GT_U32      size;
    GT_PTR      heap;               /*!< Pointer to available shared memory offset for allocation */
    GT_U32      rxReady[2]; /* slave/master */
    GT_PTR      fifos[IPC_SHM_CHANNELS*2];
} CM3_SIM_IPC_SHM_DATA_STC;




static CM3_SIM_IPC_SHM_STC shm;

#define STR(strname)    \
    #strname

static const char* speedModeToStr[NON_SUP_MODE+1] =
{
/* 0  */ STR(_100Base_FX),
/* 1  */ STR(SGMII),
/* 2  */ STR(_1000Base_X),
/* 3  */ STR(SGMII2_5),
/* 4  */ STR(QSGMII),
/* 5  */ STR(_10GBase_KX4),
/* 6  */ STR(_10GBase_KX2),   /* 10GBase-DHX */
/* 7  */ STR(_10GBase_KR),
/* 8  */ STR(_20GBase_KR2),
/* 9  */ STR(_40GBase_KR),
/* 10 */ STR(_100GBase_KR10),
/* 11 */ STR(HGL),
/* 12 */ STR(RHGL),
/* 13 */ STR(CHGL),   /* CHGL_LR10 */
/* 14 */ STR(RXAUI),
/* 15 */ STR(_20GBase_KX4),   /* HS-XAUI */
/* 16 */ STR(_10GBase_SR_LR),
/* 17 */ STR(_20GBase_SR_LR2),
/* 18 */ STR(_40GBase_SR_LR),
/* 19 */ STR(_12_1GBase_KR),          /* Serdes speed: 12.5G;    Data speed: 12.1G */
/* 20 */ STR(XLHGL_KR4),              /* 48G */
/* 21 */ STR(HGL16G),
/* 22 */ STR(HGS),
/* 23 */ STR(HGS4),
/* 24 */ STR(_100GBase_SR10),
/* 25 */ STR(CHGL_LR12),
/* 26 */ STR(TCAM),
/* 27 */ STR(INTLKN_12Lanes_6_25G),
/* 28 */ STR(INTLKN_16Lanes_6_25G),
/* 29 */ STR(INTLKN_24Lanes_6_25G),
/* 30 */ STR(INTLKN_12Lanes_10_3125G),
/* 31 */ STR(INTLKN_16Lanes_10_3125G),
/* 32 */ STR(INTLKN_12Lanes_12_5G),
/* 33 */ STR(INTLKN_16Lanes_12_5G),
/* 34 */ STR(INTLKN_16Lanes_3_125G),
/* 35 */ STR(INTLKN_24Lanes_3_125G),
/* 36 */ STR(CHGL11_LR12),

/* 37 */ STR(INTLKN_4Lanes_3_125G),
/* 38 */ STR(INTLKN_8Lanes_3_125G),
/* 39 */ STR(INTLKN_4Lanes_6_25G),
/* 40 */ STR(INTLKN_8Lanes_6_25G),

/* 41 */ STR(_2_5GBase_QX),
/* 42 */ STR(_5GBase_DQX),
/* 43 */ STR(_5GBase_HX),
/* 44 */ STR(_12GBaseR),              /* Serdes speed: 12.1875G;    Data speed: 1*11.8G */
/* 45 */ STR(_5_625GBaseR),           /* Serdes speed:   5.625G;    Data speed: 1*5.45G*/
/* 46 */ STR(_48GBaseR4),             /* Serdes speed: 12.1875G;    Data speed: 4*11.8G */
/* 47 */ STR(_12GBase_SR),            /* Serdes speed: 12.1875G;    Data speed: 1*11.8G */
/* 48 */ STR(_48GBase_SR4),           /* Serdes speed: 12.1875G;    Data speed: 4*11.8G */
/* 49 */ STR(_5GBaseR),               /* Serdes speed: 5.15625G;    Data speed: 1*5G */
/* 50 */ STR(_22GBase_SR),            /* Serdes speed: 11.5625G;    Data speed: 2*11.2G */
/* 51 */ STR(_24GBase_KR2),           /* Serdes speed: 12.1875G;    Data speed: 2*11.8G */
/* 52 */ STR(_12_5GBase_KR),          /* Serdes speed: 12.8906G;    Data speed: 1*12.5G */
/* 53 */ STR(_25GBase_KR2),           /* Serdes speed: 12.8906G;    Data speed: 2*12.5G */
/* 54 */ STR(_50GBase_KR4),           /* Serdes speed: 12.8906G;    Data speed: 4*12.5G */

/* 55 */ STR(_25GBase_KR),            /* Serdes speed: 25.78125G;   Data speed: 1*25G */
/* 56 */ STR(_50GBase_KR2),           /* Serdes speed: 25.78125G;   Data speed: 2*25G */
/* 57 */ STR(_100GBase_KR4),          /* Serdes speed: 25.78125G;   Data speed: 4*25G */
/* 58 */ STR(_25GBase_SR),            /* Serdes speed: 25.78125G;   Data speed: 1*25G */
/* 59 */ STR(_50GBase_SR2),           /* Serdes speed: 25.78125G;   Data speed: 2*25G */
/* 60 */ STR(_100GBase_SR4),          /* Serdes speed: 25.78125G;   Data speed: 4*25G */
/* 61 */ STR(_100GBase_MLG),          /* Serdes speed: 25.78125G;   Data speed: 4*25G */
/* 62 */ STR(_107GBase_KR4),          /* Serdes speed: 27.5G;       Data speed: 4*26.66G (106.6G)*/
/* 63 */ STR(_25GBase_KR_C),          /* Consortium - Serdes speed: 25.78125G;   Data speed: 1*25G */
/* 64 */ STR(_50GBase_KR2_C),         /* Consortium - Serdes speed: 25.78125G;   Data speed: 2*25G */
/* 65 */ STR(_40GBase_KR2),           /* Serdes speed: 20.625G;     Data speed: 2*20G */
/* 66 */ STR(_29_09GBase_SR4),        /* Serdes speed: 7.5G;        Data speed: 4*7.272G */

/* 67 */ STR(_40GBase_CR4),
/* 68 */ STR(_25GBase_CR),
/* 69 */ STR(_50GBase_CR2),
/* 70 */ STR(_100GBase_CR4),
/* 71 */ STR(_25GBase_KR_S),
/* 72 */ STR(_25GBase_CR_S),
/* 73 */ STR(_25GBase_CR_C),
/* 74 */ STR(_50GBase_CR2_C),

/* 75 */ STR(_50GBase_KR),              /* Serdes speed: _26_5625G;   Data speed: 1*51.51G */
/* 76 */ STR(_100GBase_KR2),            /* Serdes speed: _26_5625G;   Data speed: 2*51.51G */
/* 77 */ STR(_200GBase_KR4),            /* Serdes speed: _26_5625G;   Data speed: 4*51.51G */
/* 78 */ STR(_200GBase_KR8),            /* Serdes speed:  25.78125G;  Data speed: 8*25G */
/* 79 */ STR(_400GBase_KR8),            /* Serdes speed: _26_5625G;   Data speed: 8*51.51G */

/* 80 */ STR(_102GBase_KR4),            /* Serdes speed: _26.25G;     Data speed: 4*25G (102G)*/
/* 81 */ STR(_52_5GBase_KR2),           /* Serdes speed: _27.1G;      Data speed: 2*26.28G (52.5G) */
/* 82 */ STR(_40GBase_KR4),
/* 83 */ STR(_26_7GBase_KR),            /* Serdes speed: 27.5G;       Data speed: 1*26.66G (26.66G) */
/* 84 */ STR(_40GBase_SR_LR4),
/* 85 */ STR(_50GBase_CR),
/* 86 */ STR(_100GBase_CR2),
/* 87 */ STR(_200GBase_CR4),
/* 88 */ STR(_200GBase_CR8),
/* 89 */ STR(_400GBase_CR8),
/* 90 */ STR(_200GBase_SR_LR4),
/* 91 */ STR(_200GBase_SR_LR8),
/* 92 */ STR(_400GBase_SR_LR8),
/* 93 */ STR(_50GBase_SR_LR),
/* 94 */ STR(_100GBase_SR_LR2),
/* 95 */ STR(_20GBase_KR),
/* 96 */ STR(_106GBase_KR4),

    /* 1 MAC USX modes*/
/* 97 */ STR(_2_5G_SXGMII),          /*Serdes speed: 2.578125G;Data Speed: 1 MAC  (10M..2.5G) */
/* 98 */ STR(_5G_SXGMII),            /*Serdes speed: 5.15625G; Data Speed: 1 MAC  (10M..5G) */
/* 99 */ STR(_10G_SXGMII),           /*Serdes speed: 10.3125G; Data Speed: 1 MAC  (10M..10G) */

    /* 2 MACs USX modes*/
/* 100*/ STR(_5G_DXGMII),            /*Serdes speed: 5.15625G; Data Speed: 2 MACs (10M..2.5G) */
/* 101*/ STR(_10G_DXGMII),           /*Serdes speed: 10.3125G; Data Speed: 2 MACs (10M..5G) */
/* 102*/ STR(_20G_DXGMII),           /*Serdes speed: 20.625G;  Data Speed: 2 MACs (10M..10G) */

    /* 4 MACs USX modes*/
/* 103*/ STR(_5G_QUSGMII),           /*Serdes speed: 5G;       Data Speed: 4 MACs (10M..1G) */
/* 104*/ STR(_10G_QXGMII),           /*Serdes speed: 10.3125G; Data Speed: 4 MACs (10M..2.5G) */
/* 105*/ STR(_20G_QXGMII),           /*Serdes speed: 20.625G;  Data Speed: 4 MACs (10M..5G) */

    /* 8 MACs USX modes*/
/* 106*/ STR(_10G_OUSGMII),          /*Serdes speed: 10G;      Data Speed: 8 MACs (10M..1G) */
/* 107*/ STR(_20G_OXGMII),           /*Serdes speed: 20.625G;  Data Speed: 8 MACs (10M..2.5G) */

/* 108*/ STR(_42GBase_KR4),          /*serdes speed: 10.9375G  Data Speed: 4 MAC  */
/* 109*/ STR(_53GBase_KR2),          /*serdes speed: 27.34375G*/
/* 110*/ STR(_424GBase_KR8),         /*serdes speed: _28_125G_PAM4;   Data speed: 8*54.54G */
/* 111*/ STR(_2500Base_X),           /**/
/* 112*/ STR(_212GBase_KR4),         /*Serdes speed: _28_125G_PAM4;   Data speed: 4*54.54G */
/* 113*/ STR(_106GBase_KR2),         /*Serdes speed: _28_125G_PAM4;   Data speed: 2*54.54G */

    STR(NON_SUP_MODE)

};

/*******************************************************************************
* bootChannelGetPortStatus
*
* DESCRIPTION:
*       Fill the bit mask according to the link status of the port
*
* APPLICABLE DEVICES:
*       Any
*
* NOT APPLICABLE DEVICES:
*       None
*
* INPUTS:
*       devNum - device number
*       ifId - interface number
*
* OUTPUTS:
*       bitmask
*           BIT 0: link status - 0-down, 1-up
*       mode: HWS port mode
*       apResDone: is port done AP res
*
* RETURNS:
*       MV_OK in case of success, otherwise - MV_ERROR
*
* COMMENTS:
*
*******************************************************************************/

GT_STATUS cm3SimBootChannelGetPortStatus
(
    GT_U8 devNum,
    GT_U32 ifId,
    GT_U32 *bitmask,
    CM3_SIM_HWS_PORT_STANDARD *mode,
    GT_BOOL *apResDone
)
{
    MV_NET_PORT *port = NULL;
    GT_BOOL link = GT_FALSE;
    size_t size = mvNetPortArrayGetSize();
    GT_STATUS ret;

    *apResDone = GT_FALSE;
    *bitmask = 0;

     /* Handle NET mode ports */
    if (ifId < size) {
        port = mvNetPortArrayGetPortByDevPPort( devNum, ifId );
    } else {
        SIM_CM3_FORCE_PRINT("\r\n ERROR : [%s::%d] mvNetPortArrayGetPortByDevPPort FAILED, devNum#%d, ifId#%d\r\n", __FUNCTION__, __LINE__,devNum, ifId);
        return GT_ERROR;
    }
    if (port) {

        *mode = port->portMode;

        if(GT_FALSE==port->active)
        {
            /* this is not 'ERROR' and therefore should only be 'NOTE :' */
            SIM_CM3_FORCE_PRINT("NOTE : Port %d is not active (was not set with speed,mode)\n", ifId);
            return GT_ERROR;
        }

        if(port->portMode == NON_SUP_MODE){
            SIM_CM3_FORCE_PRINT("ERROR :  Port %d not initialized or non-supported mode\n", ifId);
            return GT_ERROR;
        }
        /*replace bootChannelGetPortStatus*/
        ret =  simCm3BootChannelGetPortStatus(port,&link);
        if ( ret != GT_OK ) {
            SIM_CM3_FORCE_PRINT(" ERROR : Port %d Unavailable, return status = %d\n", ifId , ret);
            return ret;
        }

        *apResDone = port->isAp;

        if(port->portMode <= NON_SUP_MODE)
        {
            SIM_CM3_FORCE_PRINT("port %d is 'link %s' Mode %d (%s) %s \n",
                ifId,
                link ? "Up" : "Down",
                port->portMode,
                speedModeToStr[port->portMode] ?
                speedModeToStr[port->portMode] :
                "unknown speed/mode (please update speedModeToStr[])",
                (port->isAp) ? "AN"/*AutoNeg*/:"");
        }
        else
        {
            SIM_CM3_FORCE_PRINT("port %d is 'link %s' Mode %d \n",
                ifId,
                link ? "Up" : "Down",
                port->portMode);
        }
    }
    else
    {
        SIM_CM3_FORCE_PRINT("port %d does not exist \n",ifId);
    }

    if ( link == GT_TRUE ) {
        /* Port Status Bitmap (32-bit):
                Bit 0 - PORT_LINK STATUS (1 - Link UP, 0- Link Down)
                Bit 1 - PORT_AP_RESOLUTION_DONE (1 - HCD Found, 0 - - Link Down)
                Bit 2 - PORT_AP_RESOLUTION_FAILED (1 - HCD Not found, 0 - Link UP)
            Example:
                Port 10KR nonAP Down:    0x700
                Port 10KR nonAP Up:      0x701
                Port 10KR AN Down:       0x704
                Port 10KR AN Up:         0x703
        */
        if(port->isAp)
        {
            *bitmask |= 0x3;
        }
        else
        {
            *bitmask |= 0x1;
        }
    }
    else
    {
        if(port->isAp)
        {
            *bitmask |= 0x4;
        }
    }

    return GT_OK;
}


GT_U32 cm3SimBootChannelAdressGet
(
    GT_U32 unitBase
)
{
    return unitBase+CONF_PROCESSOR_MEMORY_OFFSET+CONF_PROCESSOR_BOOT_CHANNEL_START;
}


GT_U32 cm3SimIpcRegionOffsetGet
(
    CM3_SIM_IPC_SHM_DATA_REGION region
)
{
    void * ptr1,* ptr2;
    CM3_SIM_IPC_SHM_DATA_STC shmData;
    GT_U32   offset;

    ptr1=(GT_VOID *)&shmData;

    switch(region)
    {
        case CM3_SIM_IPC_SHM_DATA_REGION_BC_DATA_E:
            ptr2=(GT_VOID *)&(shmData.bootChnData);
            break;
         case CM3_SIM_IPC_SHM_DATA_REGION_REGISTERS_E:
            ptr2=(GT_VOID *)&(shmData.registers);
            break;
        case CM3_SIM_IPC_SHM_DATA_REGION_MAGIC_E:
            ptr2=(GT_VOID *)&(shmData.magic);
            break;
        case CM3_SIM_IPC_SHM_DATA_REGION_SIZE_E:
            ptr2=(GT_VOID *)&(shmData.size);
            break;
         case CM3_SIM_IPC_SHM_DATA_REGION_HEAP_E:
            ptr2=(GT_VOID *)&(shmData.heap);
            break;
        case CM3_SIM_IPC_SHM_DATA_REGION_BC_HEADER_E:
        default:
              ptr2=(GT_VOID *)&shmData;
            break;
    }

    offset= (((uintptr_t)ptr2)&0xFFFFFFFF)-(((uintptr_t)ptr1)&0xFFFFFFFF);

    return offset;
}

static GT_U32 cm3_sim_ipc_read_32(CM3_SIM_IPC_SHM_STC *shm, CM3_SIM_IPC_SHM_DATA_REGION region)
{
    GT_U32 data;
    GT_U32 addr,offset;

    addr = shm->shmAddr;
    offset = cm3SimIpcRegionOffsetGet(region);

    cm3GenSwitchRegisterGet(addr+offset,&data,0xFFFFFFFF);

    return data;
}

static GT_U32 cm3_sim_ipc_relative_read_32(CM3_SIM_IPC_SHM_STC *shm, CM3_SIM_IPC_SHM_DATA_REGION region,GT_U32 lineOffset)
{
    GT_U32 data;
    GT_U32 addr,offset;

    addr = shm->shmAddr;
    offset = cm3SimIpcRegionOffsetGet(region);

    cm3GenSwitchRegisterGet(addr+offset+lineOffset*4,&data,0xFFFFFFFF);

    return data;
}

static GT_U32 cm3_sim_ipc_relative_write_32(CM3_SIM_IPC_SHM_STC *shm, CM3_SIM_IPC_SHM_DATA_REGION region,
    GT_U32 lineOffset,GT_U32 data)
{
    GT_U32 addr,offset;

    addr = shm->shmAddr;
    offset = cm3SimIpcRegionOffsetGet(region);

    cm3GenSwitchRegisterSet(addr+offset+lineOffset*4,data,0xFFFFFFFF);

    return data;
}


static void cm3_sim_ipc_write_32(CM3_SIM_IPC_SHM_STC *shm,CM3_SIM_IPC_SHM_DATA_REGION region, GT_U32 data)
{

    GT_U32 addr,offset;

    addr = shm->shmAddr;
    offset = cm3SimIpcRegionOffsetGet(region);

    cm3GenSwitchRegisterSet(addr+offset,data,0xFFFFFFFF);
}




/**
* @internal shmIpcBootChnReady function
* @endinternal
*
* @brief   Check if RX data available in the boot channel
*         Verify size field > 0, and ownership bit points to correct party:
*         Ownership bit='1' - msg destination is slave, and vice versa
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @retval 0                        - No messages(yet)
* @retval non 0                    - Messages are pending
*/
static int shmIpcBootChnReady(CM3_SIM_IPC_SHM_STC* shm)
{
    GT_U32 hdr;
    hdr = cm3_sim_ipc_read_32(shm,CM3_SIM_IPC_SHM_DATA_REGION_BC_HEADER_E);
    if ((hdr & 0x7ffc0000) == 0)
        return 0;
    return (((int)((hdr >> 31) & 1)) == (shm->master^1));
}


GT_STATUS testOffset(CM3_SIM_IPC_SHM_DATA_REGION reg)
{
    SIM_CM3_LOG("reg %d offset 0x%08X\n",reg,cm3SimIpcRegionOffsetGet(reg));
    return GT_OK;
}

static void shmIpcRegWrite(CM3_SIM_IPC_SHM_STC* shm, int reg, GT_U32 value)
{
    cm3_sim_ipc_relative_write_32(shm, CM3_SIM_IPC_SHM_DATA_REGION_REGISTERS_E, reg,value);
}


/**
* @internal shmIpcInit function
* @endinternal
*
* @brief   Initialize SHM.
*         Master side also initialize SHM data block (reset SHM, set magick, etc)
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] shm_virt_addr            - virtual address of shared memory block
* @param[in] size                     - the  of SHM block
* @param[in] master                   - role: 1 - master, 0 - slave
* @param[out] shm                     - SHM control structure
* @param[in] syncFuncPtr              - pointer to sync data in/out (optional)
* @param[in] syncCookie               - cookie value for syncFuncPtr
*
*/

static void shmIpcInit(
    OUT CM3_SIM_IPC_SHM_STC *shm,
    IN  int          size,
    IN  int          master,
    IN IPC_SHM_SYNC_FUNC   syncFuncPtr,
    IN void*        syncCookie
)
{
    shm->shmAddr = cm3SimBootChannelAdressGet(MG0_CM3_BASE);
    shm->shmLen = size;
    shm->master = master ? 1 : 0;
    shm->syncFunc = syncFuncPtr;
    shm->syncCookie = syncCookie;
    if (master)
    {
        cm3_sim_ipc_write_32(shm,CM3_SIM_IPC_SHM_DATA_REGION_MAGIC_E, IPC_SHM_MAGIC);
        cm3_sim_ipc_write_32(shm,CM3_SIM_IPC_SHM_DATA_REGION_SIZE_E,  shm->shmLen);
        cm3_sim_ipc_write_32(shm,CM3_SIM_IPC_SHM_DATA_REGION_HEAP_E,  IPC_SHM_ALIGN(sizeof(CM3_SIM_IPC_SHM_DATA_STC)));
    }
    else
    {
        /* check magic */
        GT_U32 magic = cm3_sim_ipc_read_32(shm, CM3_SIM_IPC_SHM_DATA_REGION_MAGIC_E);
        if (magic == IPC_SHM_MAGIC)
        {
            magic++;
            cm3_sim_ipc_write_32(shm, CM3_SIM_IPC_SHM_DATA_REGION_MAGIC_E, magic);
        }
    }
}


int cm3SimShmIpcBootChannelReady()
{
    return shmIpcBootChnReady(&shm);
}

void cm3SimShmIpcBootChannelStateSet
(
   CM3_MICRO_INIT_BOOT_CH_BOOT_STATE_ENT state
)
{
    GT_CHAR_PTR stateToString []=
        {
            "N/A",
            "MICRO_INIT_BOOT_CH_BOOT_INIT_STATE_IN_PROGRESS_E",/*1*/
            "MICRO_INIT_BOOT_CH_BOOT_INIT_STATE_DONE_E",       /*2*/
            "MICRO_INIT_BOOT_CH_BOOT_INIT_STATE_FAILED_E"      /*3*/
        };
    GT_U32 data;

    fprintf( stderr,"MI boot state %s \n",stateToString[state]);
    shmIpcRegWrite(&shm,CM3_MICRO_INIT_BOOT_CH_STATUS_REG_BOOT_STATE_E, state);

    /*init magic*/
    if(state == MICRO_INIT_BOOT_CH_BOOT_INIT_STATE_IN_PROGRESS_E)
    {
        shmIpcRegWrite(&shm,CM3_MICRO_INIT_BOOT_CH_STATUS_REG_MAGIC_KEY_E, IPC_RESVD_MAGIC);

        /*last word of memory*/
        cm3_sim_ipc_relative_write_32(&shm,CM3_SIM_IPC_SHM_DATA_REGION_BC_HEADER_E,
        (BOOT_CHANNEL_SIZE>>2)-1
       ,IPC_RESVD_MAGIC);


        data = CONF_PROCESSOR_BOOT_CHANNEL_START;
        data+=cm3SimIpcRegionOffsetGet(CM3_SIM_IPC_SHM_DATA_REGION_REGISTERS_E);

        cm3_sim_ipc_relative_write_32(&shm,CM3_SIM_IPC_SHM_DATA_REGION_BC_HEADER_E,
                (BOOT_CHANNEL_SIZE>>2)-2
               ,data);
    }

    /*sync /Cider/EBU/Harrier/Harrier {Current}/Harrier/Reset and Init/
                        DFX Server/Units/DFX Server Units/DFX Server Registers/
                        Device General Control 2
          */
    switch(state)
    {
        case MICRO_INIT_BOOT_CH_BOOT_INIT_STATE_DONE_E:
            data=0x2;
            break;
        case MICRO_INIT_BOOT_CH_BOOT_INIT_STATE_IN_PROGRESS_E:
        case MICRO_INIT_BOOT_CH_BOOT_INIT_STATE_FAILED_E:
        default:
            data=0x1;
            break;
    }
    cm3GenSwitchRegisterSet(0x000F8258,data,0xFFFFFFFF);
}


GT_VOID cm3SimShmIpcInit
(
   GT_VOID
)
{
    shmIpcInit(&shm, CONF_PROCESSOR_MEMORY_SIZE, FW_OWNERSHIP_BIT, NULL, NULL);
}

GT_STATUS cm3SimShmIpcDump
(
   GT_U32 firstLine,
   GT_U32 lastLine
)
{
    GT_U32 i,data;

    for(i=firstLine;i<=lastLine;i++)
    {
        data = cm3_sim_ipc_relative_read_32(&shm,CM3_SIM_IPC_SHM_DATA_REGION_BC_HEADER_E,i);
        SIM_CM3_LOG("Line %d data 0x%08X\n",i,data);

    }

    return GT_OK;
}


/**
 * \brief Copy a region of shared memory.
 *
 * memcpy cannot be used since access from crossbar must be on word size and
 * boundary. Also flush cache
 */
static void IPC_COPYFROM
(
    CM3_SIM_IPC_SHM_STC *shm,
    void *dest,
    CM3_SIM_IPC_SHM_DATA_REGION region,
    int size/*in bytes*/
)
{
    GT_U32 i;
    GT_U32 * ptr,lines;

    ptr=dest;

    if (size == 0)
        return;

    lines = (size+4)/4;

    for(i=0;i<lines;i++)
    {
        *ptr = cm3_sim_ipc_relative_read_32(shm,region,i);
         ptr++;
    }

}

static void IPC_COPYTO
(
    CM3_SIM_IPC_SHM_STC *shm,
    CM3_SIM_IPC_SHM_DATA_REGION region,
    const void *src,
    int size
)
{
    GT_U32 i,lines;
    GT_U32 * ptr;

    if (size == 0)
        return;

    lines = (size+4)/4;
    ptr = (GT_U32 *)src;

    for(i=0;i<lines;i++)
    {
        cm3_sim_ipc_relative_write_32(shm,region,i,*ptr);
        ptr++;
    }

}

/*******************************************************************************
* bootChannelGetBootonVersion
*
* DESCRIPTION:
*       Get the Booton version number
*
* APPLICABLE DEVICES:
*       Any
*
* NOT APPLICABLE DEVICES:
*       None
*
* INPUTS:
*       None
*
* OUTPUTS:
*       None
*
* RETURNS:
*       Pointer to string with Booton version
*
* COMMENTS:
*
*******************************************************************************/
static GT_CHAR_PTR bootChannelGetBootonVersion()
{
    return &cm3SimBootonVersion[0];
}

/*
we look for next string in image :
MicroInit: %s

*/
#define local_isprint(c)     ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9'))

GT_STATUS cm3FindFwVersionInBinFile(void)
{
    GT_U32 value,ii;
    GT_U32 address = MG0_CM3_BASE + CONF_PROCESSOR_MEMORY_OFFSET;
    char   currStr[5];
    char   *str;
    char   _char;
    char    microInit_str[14] = {0};
    GT_U32 found = 0;
    static char    versionStr[BOOT_CHANEL_VERSION_STRING_LEN] = {0};
    GT_U32  tmp_address,index=0;

    currStr[4] = 0;
    microInit_str[13] = 0;

#define move_chars_in_str(str,newchar) \
    str[0] = str[1];                   \
    str[1] = str[2];                   \
    str[2] = str[3];                   \
    str[3] = str[4];                   \
    str[4] = str[5];                   \
    str[5] = str[6];                   \
    str[6] = str[7];                   \
    str[7] = str[8];                   \
    str[8] = str[9];                   \
    str[9]  = str[10];                 \
    str[10] = str[11];                 \
    str[11] = str[12];                 \
    str[12] = newchar

#define put_chars_in_str(str,index,newchar) \
    str[index++] = newchar


    for ( ; address < MG0_CM3_BASE + CONF_PROCESSOR_MEMORY_OFFSET + (256*1024) ;
        address += 4)
    {
        scibReadMemory(SIM_CM3_DEVICE_ID, address , 1, &value);

        currStr[0] = (char)(value);
        currStr[1] = (char)(value >>  8);
        currStr[2] = (char)(value >> 16);
        currStr[3] = (char)(value >> 24);

        str = &currStr[0];
        for(ii = 0; ii < 4; ii++,str++)
        {
            _char = (*str);

            move_chars_in_str(microInit_str,_char);
            if(0 == strcmp(microInit_str,"MicroInit: %s"))
            {
                found = 1;
                ii++,str++;
                break;
            }
        }

        if(found)
        {
            /* jump the 'nil' (10 char) next char after "MicroInit: %s"*/
            /* take the rest of what left from current word */
            for(; ii < 4; ii++,str++)
            {
                _char = (*str);

                if(_char == 0 || (_char == ((char)0xa)))
                {
                    continue;
                }

                put_chars_in_str(versionStr,index,_char);
            }

            tmp_address = address + 4;

            for ( ; tmp_address < address + (BOOT_CHANEL_VERSION_STRING_LEN) ;
                tmp_address += 4)
            {
                scibReadMemory(SIM_CM3_DEVICE_ID, tmp_address, 1, &value);

                currStr[0] = (char)(value);
                currStr[1] = (char)(value >>  8);
                currStr[2] = (char)(value >> 16);
                currStr[3] = (char)(value >> 24);

                str = &currStr[0];

                for(ii = 0 ; ii < 4; ii++,str++)
                {
                    _char = (*str);

                    if(_char == 0 || (_char == ((char)0xa)))
                    {
                        if(index < 2)
                        {
                            /* found the rest of endings after the "MicroInit: %s" */
                            index = 0;
                            continue;
                        }

                        versionStr[index] = 0;
                        sprintf(cm3SimFwVersion,"%s",versionStr);
                        return GT_OK;
                    }

                    put_chars_in_str(versionStr,index,_char);
                }
            }

            return GT_ERROR;
        }

    }

    return GT_NOT_FOUND;
}

GT_CHAR_PTR bootChannelGetFwVersion(void)
{
    if(strlen(cm3SimFwVersion))
    {
        return cm3SimFwVersion;
    }

    if(GT_OK == cm3FindFwVersionInBinFile())
    {
        /* was copied now to cm3SimFwVersion */
        return cm3SimFwVersion;
    }

    return __FW_VERSION__;
}


/*******************************************************************************
* bootChannelGetAllFilesById
*
* DESCRIPTION:
*       List all script files from super image with usage_bitmap[cfgId] = '1', in groups of 10
*
* APPLICABLE DEVICES:
*       Any
*
* NOT APPLICABLE DEVICES:
*       None
*
* INPUTS:
*       cfgId - config-id number (representing bit location in usage bitmap)
*       grpId - number of group of 10 files
*
* OUTPUTS:
*       msg - output string
*
* RETURNS:
*       MV_OK in case of success, otherwise - MV_ERROR
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS cm3SimBootChannelGetAllFilesById(GT_U32 cfgId, GT_U32 grpId, char **msg)
{
    GT_U32 counter = 0;
    GT_U32 dataLength = 0;
    GT_BOOL restart = GT_TRUE;
    file_params_STC file_params;
    file_params_STC  *file_params_ptr = &file_params;
    char *data = *msg;
    GT_BOOL keepRunning = GT_TRUE;
    GT_BOOL groupStarted = (grpId)?GT_FALSE:GT_TRUE;
    data[0] = 0;

    while (keepRunning && cm3GetNextFile(file_params_ptr, restart) != GT_FALSE) {
        restart = GT_FALSE;
        switch (file_params_ptr->type) {
        case script:
        case EPROM:
            if (file_params_ptr->useBmp & (1 << cfgId)) {
                SIM_CM3_LOG("-- file: %12s,   type=%d,   useBmp=%d,   size=%6d,  address=0x%08x\n",
                    file_params_ptr->name, file_params_ptr->type, file_params_ptr->useBmp,
                    file_params_ptr->size, file_params_ptr->address);
                if (!groupStarted) {
                    /* skip files until start of group */
                    if (++counter == (NUM_OF_FILES_IN_GROUP*grpId)){
                        groupStarted = GT_TRUE;
                        counter=0;
                    }
                } else {
                    sprintf(data+dataLength,"[%s]",file_params_ptr->name);
                    dataLength += (2+strlen(file_params_ptr->name));
                    if (!(++counter % NUM_OF_FILES_IN_GROUP)){
                        keepRunning = GT_FALSE;
                    }
                }
            }
            break;
        default:
            break;
        }
    }
    if (counter % NUM_OF_FILES_IN_GROUP){
        strcat(*msg,"@");
    } else {
        strcat(data,":");
    }
    SIM_CM3_LOG("Config list:\r\n%s\r\n",*msg);

    return (counter ? GT_OK : GT_ERROR);
}


/**
* @internal shmIpcBootChnRecv function
* @endinternal
*
* @brief   Receive message from IPC boot channel
*         Verify size field > 0, and ownership bit points to correct party:
*         Ownership bit='1' - msg destination is slave, and vice versa
*         After reading contents, clear size + ret code field to mark buffer empty.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @param[in] shm                      - SHM control structure
* @param[in] maxSize                  - data buffer size
*
* @param[out] retCodePtr               - return code (header bits 0..7), can be NULL
*                                      data        - data buffer
*
* @retval >0                       - size of received message received
* @retval 0                        - No data (no messafe)
*                                       -2     - Boot channel busy
*/
static int shmIpcBootChnRecv(
    IN  CM3_SIM_IPC_SHM_STC *shm,
    OUT int         *retCodePtr,
    OUT void        *msg,
    IN  int          maxSize
)
{

    GT_U32 hdr, size;
    hdr = cm3_sim_ipc_read_32(shm, CM3_SIM_IPC_SHM_DATA_REGION_BC_HEADER_E);
    if (!hdr)
        return 0; /* no message ready */
    size = (hdr >> 18) & 0x1fff;
    if (retCodePtr != NULL)
        *retCodePtr = hdr & 0xff;
    /* Checking ownership bit to know if this msg is for us or other side */
    hdr = (hdr >> 31) & 1;
    if ((GT_U32)(shm->master) != hdr)
    {
        /* Message is for us, copy content and clear size + ret code fields */
        if ((GT_U32)maxSize < size)
            size = maxSize;
        IPC_COPYFROM(shm, msg,CM3_SIM_IPC_SHM_DATA_REGION_BC_DATA_E, size);
        /* Mark buffer as empty / free */
        cm3_sim_ipc_write_32(shm,CM3_SIM_IPC_SHM_DATA_REGION_BC_HEADER_E, shm->master ? 0 : 0x80000000);
        return size;
    }
    return -2; /* busy */
}

static int shmIpcBootChnSend(
    IN  CM3_SIM_IPC_SHM_STC *shm,
    IN  int          retCode,
    IN  const void  *msg,
    IN  int          size
)
{
    CM3_SIM_IPC_SHM_DATA_STC *shmData;
    GT_U32 hdr;

    /* check buffer size field. Only if size = 0, msg can be sent */
    if (( cm3_sim_ipc_read_32(shm, CM3_SIM_IPC_SHM_DATA_REGION_BC_HEADER_E) & 0x7fffffff) != 0)
        return -2; /* busy */

    if ((unsigned)size > sizeof(shmData->bootChnData))
        return -3; /* bad parameter: message too long */

    /* copy msg to buffer,  */
    IPC_COPYTO(shm, CM3_SIM_IPC_SHM_DATA_REGION_BC_DATA_E, msg, size);
    /* Update size, ret code and ownership fields */
    hdr = (size << 18) | (retCode & 0xff);
    if (shm->master)
        hdr |= 0x80000000;

    cm3_sim_ipc_write_32(shm,CM3_SIM_IPC_SHM_DATA_REGION_BC_HEADER_E, hdr);

    return 0;
}



/*******************************************************************************
* bootChannelRecv
*
* DESCRIPTION:
*       Handle message received from IPC boot channel
*
* APPLICABLE DEVICES:
*       Any
*
* NOT APPLICABLE DEVICES:
*       None
*
* INPUTS:
*       shm - SHM control structure
*
* OUTPUTS:
*       None
*
* RETURNS:
*       MICRO_INIT_BOOT_CH_RET_CODE_OK_E in case of success
*       error code from MICRO_INIT_BOOT_CH_RET_CODE_ENT otherwise
*
* COMMENTS:
*
*******************************************************************************/
int cm3SimBootChannelRecv
(
    GT_VOID
)
{

    int rc = 0;
    MICRO_INIT_BOOT_CH_RET_CODE_ENT bc_rc = MICRO_INIT_BOOT_CH_RET_CODE_OK_E;

    int msg_len;
    char msg[IPC_BOOT_CHANNEL_MESSAGE_BUFFER_LEN];
    char commandString[BOOT_CHANNEL_COMMAND_STRING];
    GT_U32 bcFwErr;

    int boot_channel_op;
    int prc;
    int i;
    GT_U32 ifNum;
    GT_U32 bmask = 0;
    char *pmsg = &msg[10]; /* Message Op-Code (2 bytes) + [IN] param1 (32-bit) + [IN] param2 (32-bit) */
    GT_U32 cfgId = 0;
    GT_U32 grpId = 0;

    msg_len = shmIpcBootChnRecv(&shm, NULL, &msg, IPC_BOOT_CHANNEL_MESSAGE_MAX_LEN);
    if( (msg_len <= 0) || (msg_len >= (IPC_BOOT_CHANNEL_MESSAGE_BUFFER_LEN-2)) )
        return 0;

    boot_channel_op = msg[0] + (msg[1] << 8);


    SIM_CM3_LOG("boot_channel_op = %d\n",boot_channel_op);

   /* Not all opcodes contain these fields, nor 'ifNum' */

    ifNum = BC_PARAM(1);

    switch (boot_channel_op)
    {
        case MICRO_INIT_BOOT_CH_OPCODE_EPROM_LIKE_E:
        case MICRO_INIT_BOOT_CH_OPCODE_CLI_LIKE_E:
            msg[2+msg_len]=0;
            SIM_CM3_LOG("Command  %s\n",msg+2);

            if ( (prc = cm3ParseCommand(&msg[2])) != 0 )
            {
                SIM_CM3_LOG("error: bootChannelRecv, CLI_LIKE_BC_OP_CODE, %d\n", prc);
                bc_rc = MICRO_INIT_BOOT_CH_RET_CODE_INVALID_SYNTAX_E;

                /* Report Boot Channel FW Error Code*/
                (MICRO_INIT_BOOT_CH_OPCODE_CLI_LIKE_E == boot_channel_op) ? (bcFwErr = MICRO_INIT_BOOT_CH_FW_ERR_CLI_LIKE_FILE_PROCESSING_FAILED_E):(bcFwErr = MICRO_INIT_BOOT_CH_FW_ERR_EEPROM_LIKE_FILE_PROCESSING_FAILED_E);
                shmIpcRegWrite(&shm,CM3_MICRO_INIT_BOOT_CH_STATUS_REG_FW_ERR_CODE_E, bcFwErr);
            }
            break;
        case MICRO_INIT_BOOT_CH_OPCODE_RERUN_FILES_E:
         if (msg[2] > 31) {
            SIM_CM3_LOG("error: bootChannelRecv, rerun configId currently support only 32 bits, "
                   "requested configId bit %d (>31)\n", msg[2]);
            bc_rc = MICRO_INIT_BOOT_CH_RET_CODE_INVALID_CONFIG_ID_E;
         } else {
            strcpy(commandString,"config rerun  ");
            commandString[strlen("config rerun")+1] = msg[2] + '0';
            if ( (prc = cm3ParseCommand(commandString)) != 0 ) {
                SIM_CM3_LOG("error: bootChannelRecv, RERUN_FILES_BC_OP_CODE, %d\n", prc);

                if ( GT_NOT_FOUND == prc ) {
                    bc_rc = MICRO_INIT_BOOT_CH_RET_CODE_NO_MATCH_FILE_E;
                } else {
                    bc_rc = MICRO_INIT_BOOT_CH_RET_CODE_INVALID_SYNTAX_E;
                }
            }
        }
        break;

        case MICRO_INIT_BOOT_CH_OPCODE_GET_VERSION:
               {
                   memset(pmsg,0,IPC_BOOT_CHANNEL_MESSAGE_BUFFER_LEN-10);
                   sprintf(pmsg, "%s", getImageVersion(NULL));
                   i = BOOT_CHANEL_VERSION_STRING_LEN;
                   sprintf(pmsg+i, "%s",bootChannelGetFwVersion());
                   i+= BOOT_CHANEL_VERSION_STRING_LEN;
                   sprintf(pmsg+i, "%s",bootChannelGetBootonVersion());
                   i+= BOOT_CHANEL_VERSION_STRING_LEN;

                   SIM_CM3_LOG("\r\nSuperImage version: %s\r\nMicroinit version: %s\r\nBooton version: %s\r\n",pmsg,pmsg+32,pmsg+64);

                   shmIpcBootChnSend(&shm, GT_OK, pmsg, i);
               }
               return GT_OK;
       case  MICRO_INIT_BOOT_CH_OPCODE_GET_PORT_STATUS:
        {
            CM3_SIM_HWS_PORT_STANDARD  mode;
            GT_BOOL apResDone;
            GT_U32 buf[16] ={0};
            GT_U32 first_port, last_port;

            if (ifNum == ALL_PORTS_TOKEN)
                first_port=0, last_port=15;
            else
                first_port=ifNum, last_port=ifNum;

            for (ifNum=first_port; ifNum<=last_port; ifNum++) {
                rc = cm3SimBootChannelGetPortStatus(DEV_NUM_0, ifNum, &bmask, &mode, &apResDone);
                SIM_CM3_LOG("rc %d ifnum %d bmask %d mode %d apResDone %d\n", rc, ifNum, bmask, mode, apResDone);
                if (GT_OK != rc) {
                    shmIpcRegWrite(&shm,CM3_MICRO_INIT_BOOT_CH_STATUS_REG_OPCODE_ERR_CODE_E,
                        MICRO_INIT_BOOT_CH_RET_CODE_INVALID_SYNTAX_E);
                    bc_rc = MICRO_INIT_BOOT_CH_RET_CODE_INVALID_SYNTAX_E;
                    break;
                }

                buf[ifNum-first_port] =
                    (bmask & 0xFF) | (mode << PORT_STATUS_HWS_MODE_OFFSET);
            }

            shmIpcBootChnSend(&shm, GT_OK, &buf, 4*(last_port-first_port+1));
            return GT_OK;
        }
        break;
       case  MICRO_INIT_BOOT_CH_OPCODE_GET_ALL_FILES:
        {
            cfgId = msg[2];
            grpId = msg[6];
            memset(pmsg,0,IPC_BOOT_CHANNEL_MESSAGE_BUFFER_LEN-10);
            rc = cm3SimBootChannelGetAllFilesById(cfgId, grpId, &pmsg);
            if ( GT_OK == rc ) {
                shmIpcBootChnSend(&shm, GT_OK, pmsg, IPC_BOOT_CHANNEL_MESSAGE_BUFFER_LEN-10);
                return rc;
            } else {
                shmIpcRegWrite(&shm,CM3_MICRO_INIT_BOOT_CH_STATUS_REG_OPCODE_ERR_CODE_E,
                    MICRO_INIT_BOOT_CH_RET_CODE_NO_MATCH_FILE_E);
                bc_rc = MICRO_INIT_BOOT_CH_RET_CODE_NO_MATCH_FILE_E;
            }
        }
        break;

       case MICRO_INIT_BOOT_CH_OPCODE_GET_MI_STATE:
        {
             GT_U32 miState = cm3_sim_ipc_relative_read_32(&shm,CM3_SIM_IPC_SHM_DATA_REGION_REGISTERS_E,
                CM3_MICRO_INIT_BOOT_CH_STATUS_REG_BOOT_STATE_E);
             shmIpcBootChnSend(&shm, GT_OK, &miState, 4);
             return GT_OK;
        }
        break;

    default:
        /* Undefined boot channel op code.*/
        shmIpcRegWrite(&shm,CM3_MICRO_INIT_BOOT_CH_STATUS_REG_OPCODE_ERR_CODE_E, BOOT_CH_MSG_OPCODE_INVALID);
        bc_rc = BOOT_CH_MSG_OPCODE_INVALID;
        break;
    }

    /* Write return status to the boot channel header.
       We return a dummy msg, to avoid empty msg which the BC cannot handle */
    shmIpcBootChnSend(&shm, bc_rc, msg/*dummy*/, 4);

    return rc;
}

GT_STATUS  cm3SimDbgFwVersionSet
(
    GT_U32 num0,
    GT_U32 num1,
    GT_U32 num2
)
{
    if(num0>MAX_VER_NUM||num1>MAX_VER_NUM||num2>MAX_VER_NUM)
    {
        return GT_BAD_PARAM;
    }
    sprintf(cm3SimFwVersion,"%d.%d.%d",num0,num1,num2);
    return GT_OK;
}

GT_STATUS  cm3SimDbgFwVersionReset
(
    GT_VOID
)
{
    cm3SimFwVersion[0]='\0';
    return GT_OK;
}

