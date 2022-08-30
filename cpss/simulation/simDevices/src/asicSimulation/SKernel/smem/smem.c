/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
*/
/**
********************************************************************************
* @file smem.c
*
* @brief This is a API implementation for SMem module of SKernel.
*
* @version   81
********************************************************************************
*/
#include <asicSimulation/SKernel/smem/smem.h>
#include <asicSimulation/SKernel/skernel.h>
#include <asicSimulation/SKernel/smem/smemSoho.h>
#include <asicSimulation/SKernel/smem/smemCheetah.h>
#include <asicSimulation/SKernel/smem/smemCheetah2.h>
#include <asicSimulation/SKernel/smem/smemCheetah3.h>
#include <asicSimulation/SKernel/smem/smemXCat3.h>
#include <asicSimulation/SKernel/smem/smemAc5.h>
#include <asicSimulation/SKernel/smem/smemLion.h>
#include <asicSimulation/SKernel/smem/smemXCat2.h>
#include <asicSimulation/SInit/sinit.h>
#include <asicSimulation/SKernel/smem/smemPhy.h>
#include <asicSimulation/SKernel/smem/smemMacsec.h>
#include <asicSimulation/SKernel/smem/smemIronman.h>
#include <asicSimulation/SKernel/smem/smemPipe.h>
#include <asicSimulation/SLog/simLogInfoTypeMemory.h>
#include <asicSimulation/SLog/simLog.h>
#include <asicSimulation/SKernel/cheetahCommon/sregCheetah.h>
#include <asicSimulation/SKernel/cheetahCommon/sregPipe.h>

#define MAX_ENTRY_SIZE_IN_WORDS_CNS  64
#define MAX_ALLOC_CNS               0x0FFFFFFF
#define ASICP_socket_max_tbl_entry_size_CNS 256

#if 0/* modify to 1 for debug */
    #define MALLOC_FILE_NAME_DEBUG_PRINT(x) simForcePrintf x
#else
    #define MALLOC_FILE_NAME_DEBUG_PRINT(x)
#endif


/* global active memory for :
    no active memory to the unit.
   and no need to look all over the active memory of the device */
SMEM_ACTIVE_MEM_ENTRY_STC smemEmptyActiveMemoryArr[] =
{
    /* must be last anyway */
    {END_OF_TABLE, SMEM_FULL_MASK_CNS, NULL,0,NULL,0}
};


typedef enum {
          ASICP_opcodes_read_memory_E,
          ASICP_opcodes_write_memory_E
} ASICP_opcodes_ENT;


typedef struct{
    ASICP_opcodes_ENT           message_type;/* must be ASICP_opcodes_write_memory_E */
    GT_U32                     cpu_id;   /* 1 based number */
    GT_U32                     device_id;/* 0 based number */
    GT_U32                     address;  /* address in memory of the device */
    GT_U32                     length;   /* number of words */
    GT_U32                     entry[ASICP_socket_max_tbl_entry_size_CNS];
} ASICP_msg_info_STC;

void  smemReadWriteMem (
                                IN SCIB_MEMORY_ACCESS_TYPE accessType,
                                IN void * deviceObjPtr,
                                IN GT_U32 address,
                                IN GT_U32 memSize,
                                INOUT GT_U32 * memPtr
                              );

static void * smemFindMemory  (
                                IN SKERNEL_DEVICE_OBJECT * deviceObj,
                                IN GT_U32                  address,
                                IN GT_U32                  memSize,
                                OUT SMEM_ACTIVE_MEM_ENTRY_STC ** activeMemPtrPtr,
                                IN SCIB_MEMORY_ACCESS_TYPE accessType
                              );
#if 0 /* old - non-used operations */
static void  smemSendDataToVisualAsic(
                                    IN GT_U32                  deviceObjID,
                                    IN GT_U32                  address,
                                    IN GT_U32                * dataPtr,
                                    IN GT_U32                  dataSize
                              );
#else
#define smemSendDataToVisualAsic(a,b,c,d) /* empty */
#endif /*0*/

static GT_VOID smemChunkInternalWriteDataCheck(
                                    IN SKERNEL_DEVICE_OBJECT    *devObjPtr,
                                    IN GT_U32                   address,
                                    IN SMEM_CHUNK_STC           *chunkPtr,
                                    INOUT GT_U32                *memoryIndexPtr
                                );

static GT_VOID smemUnitChunkOverlapCheck(
                                    IN SMEM_UNIT_CHUNKS_STC    *unitChunksPtr,
                                    IN GT_U32 unitChunkOffset,
                                    IN SMEM_CHUNK_STC * currChunkPtr
                                );
static GT_BOOL smemUnitChunkFormulaOverlapCheck(
                                    IN SMEM_UNIT_CHUNKS_STC     *unitChunksPtr,
                                    IN GT_U32 unitChunkOffset,
                                    IN SMEM_CHUNK_STC * currChunkPtr,
                                    IN GT_U32 addressOffset
                                );
/*flag to allow to by pass the fatal error on accessing to non-exists memory.
    this allow to use test of memory without the need to stop on every memory

    we can get a list of all registers at once
*/
GT_U32   debugModeByPassFatalErrorOnMemNotExists = 0;

/* build address of the table in the relevant unit */
/* replace the 6 bits of the unit in the 'baseAddr' with 6 bits of unit from the 'unit base address'*/
#define SET_MULTI_INSTANCE_ADDRESS_MAC(localAddr,unitBaseAddr) \
        localAddr+=unitBaseAddr


/* check field parameters */
static void paramCheck
(
    IN      GT_U32  startBit ,
    IN      GT_U32  numBits)
{
    if(startBit >= 32)
    {
        skernelFatalError("paramCheck : startBit[%d] >= 32 \n" , startBit);
    }
    if(numBits > 32)
    {
        skernelFatalError("paramCheck : numBits[%d] > 32 \n" , numBits);
    }
    if((startBit + numBits) > 32)
    {
        skernelFatalError("paramCheck : (startBit[%d] + numBits[%d]) > 32 \n" , startBit,numBits);
    }
}

/**
* @internal smemInit function
* @endinternal
*
* @brief   Init memory module for a device.
*
* @note The binding to the scib rw function needs to be indicated if the
*       device is PP or FA.
*
*/
void smemInit
(
    IN SKERNEL_DEVICE_OBJECT * deviceObjPtr
)
{
    GT_U8     isPP=1;
    GT_U32     devId;
    SCIB_RW_MEMORY_FUN rwFun;     /* pointer to the R/W SKernel memory CallBack function */
    GT_U32      ii;/*iterator*/
    GT_U32  addressCompletionStatus;

    if (!deviceObjPtr)
    {
        skernelFatalError("smemInit : illegal pointer\n");
    }

    devId = deviceObjPtr->deviceId ;
    /* Set default memory access function */
    rwFun = smemReadWriteMem;

    /* check device type (including FA) and call device/FA specific init */
    switch(deviceObjPtr->deviceFamily)
    {
         case SKERNEL_SOHO_FAMILY:
              deviceObjPtr->interfaceBmp = SCIB_BUS_INTERFACE_SMI;/* maybe set by INI file to SCIB_BUS_INTERFACE_SMI_INDIRECT */
              smemSohoInit(deviceObjPtr);
         break;
         case SKERNEL_CHEETAH_1_FAMILY:
              smemChtInit(deviceObjPtr) ;
              break;
         case SKERNEL_CHEETAH_2_FAMILY:
              smemCht2Init(deviceObjPtr) ;
              break;
         case SKERNEL_CHEETAH_3_FAMILY:
         case SKERNEL_XCAT_FAMILY:
              smemCht3Init(deviceObjPtr) ;
              break;
         case SKERNEL_XCAT3_FAMILY:
              smemXCat3Init(deviceObjPtr);
              break;
         case SKERNEL_AC5_FAMILY:
              smemAc5Init(deviceObjPtr);
              break;
         case SKERNEL_XCAT2_FAMILY:
              smemXCat2Init(deviceObjPtr);
              break;
         case SKERNEL_LION_PORT_GROUP_SHARED_FAMILY:
              smemLionInit(deviceObjPtr) ;

              /* shared device not bind with the SCIB , because it has no unique
                 PEX base address of */

               /* !!! THIS IS RETURN AND NOT BREAK !!!!! */
               /* !!! THIS IS RETURN AND NOT BREAK !!!!! */
               /* !!! THIS IS RETURN AND NOT BREAK !!!!! */
              return;
         case SKERNEL_LION_PORT_GROUP_FAMILY:
              smemLionInit(deviceObjPtr) ;
              break;
         case SKERNEL_LION2_PORT_GROUP_SHARED_FAMILY:
              smemLion2Init(deviceObjPtr) ;

              /* shared device not bind with the SCIB , because it has no unique
                 PEX base address of */

               /* !!! THIS IS RETURN AND NOT BREAK !!!!! */
               /* !!! THIS IS RETURN AND NOT BREAK !!!!! */
               /* !!! THIS IS RETURN AND NOT BREAK !!!!! */
              return;
         case SKERNEL_LION2_PORT_GROUP_FAMILY:
              smemLion2Init(deviceObjPtr) ;
              break;
         case SKERNEL_PHY_SHELL_FAMILY:
              deviceObjPtr->interfaceBmp = SCIB_BUS_INTERFACE_SMI_INDIRECT;
              smemPhyInit(deviceObjPtr);
              /* shared device not bind with the SCIB , because it has no unique
                 SMI base address */

               /* !!! THIS IS RETURN AND NOT BREAK !!!!! */
               /* !!! THIS IS RETURN AND NOT BREAK !!!!! */
               /* !!! THIS IS RETURN AND NOT BREAK !!!!! */
              return;
         case SKERNEL_PHY_CORE_FAMILY:
              deviceObjPtr->interfaceBmp = SCIB_BUS_INTERFACE_SMI_INDIRECT;
              smemPhyInit(deviceObjPtr);
         break;
         case SKERNEL_MACSEC_FAMILY:
              deviceObjPtr->interfaceBmp = SCIB_BUS_INTERFACE_SMI_INDIRECT;
              smemMacsecInit(deviceObjPtr);
               /* need to register the macsec so the PHY can access to it via the scib interface .
               (so active memories in macsec can be activated)*/
              break;
         case SKERNEL_PUMA3_SHARED_FAMILY:
               /* no bind with the SCIB */
               /* this is the port group shared device */
               /* so we need to fill info about it's port group devices */
               for( ii = 0 ; ii < deviceObjPtr->numOfCoreDevs ;ii++)
               {
                   /* each of the port group devices start his ports at multiple of 16
                      the Puma3 hold only 2 MG --> meaning 2 'cores'
                      so each core has 16 NW ports (0..15) + 16 fabric ports(32..47) */
                   deviceObjPtr->coreDevInfoPtr[ii].startPortNum = 16*ii;
               }
              return;
        case SKERNEL_BOBCAT2_FAMILY:
            smemBobcat2Init(deviceObjPtr);
            break;
        case SKERNEL_BOBK_CAELUM_FAMILY:
        case SKERNEL_BOBK_CETUS_FAMILY:
            smemBobkInit(deviceObjPtr);
            break;
        case SKERNEL_BOBK_ALDRIN_FAMILY:
        case SKERNEL_AC3X_FAMILY:
            smemBobkAldrinInit(deviceObjPtr);
            break;
        case SKERNEL_BOBCAT3_FAMILY:
            smemBobcat3Init(deviceObjPtr);
            break;
        case SKERNEL_ALDRIN2_FAMILY:
            smemAldrin2Init(deviceObjPtr);
            break;
        case SKERNEL_PIPE_FAMILY:
            smemPipeInit(deviceObjPtr);
            break;
        case SKERNEL_FALCON_FAMILY:
            smemFalconInit(deviceObjPtr);
            break;
        case SKERNEL_HAWK_FAMILY:
            smemHawkInit(deviceObjPtr);
            break;
        case SKERNEL_PHOENIX_FAMILY:
            smemPhoenixInit(deviceObjPtr);
            break;
        case SKERNEL_HARRIER_FAMILY:
            smemHarrierInit(deviceObjPtr);
            break;
        case SKERNEL_IRONMAN_FAMILY:
            smemIronmanInit(deviceObjPtr);
            break;
         default:
              skernelFatalError(" smemInit: not valid mode[%d]",
                                   deviceObjPtr->deviceFamily);
         break;
    }

    if(SKERNEL_DEVICE_FAMILY_SOHO_PERIDOT(deviceObjPtr->deviceType))
    {
        /* WA for scibSmiRegRead(...) , scibSmiRegWrite(...) */
        addressCompletionStatus = 1;
    }
    else
    {
        addressCompletionStatus = ADDRESS_COMPLETION_STATUS_GET_MAC(deviceObjPtr);
    }



    /* bind R/W callback with SCIB */
    scibBindRWMemory(devId,
                     deviceObjPtr->deviceHwId,
                     deviceObjPtr,
                     rwFun,
                     isPP,
                     addressCompletionStatus);

    if(!SMEM_CHT_IS_SIP6_GET(deviceObjPtr))
    {
        deviceObjPtr->deviceForceBar0Bar2 = GT_FALSE;/* currently no support on those devices */
    }

    if(deviceObjPtr->deviceForceBar0Bar2)/* supports bar0 (and bar2) */
    {
        SCIB_BIND_EXT_INFO_STC bindExtInfo;

        memset(&bindExtInfo,0,sizeof(bindExtInfo));

        bindExtInfo.update_deviceForceBar0Bar2 = GT_TRUE;
        bindExtInfo.deviceForceBar0Bar2 = GT_TRUE;

        bindExtInfo.update_bar0_size = GT_TRUE;
        bindExtInfo.bar0_size = deviceObjPtr->bar0_size;

        bindExtInfo.update_bar2_size = GT_TRUE;
        bindExtInfo.bar2_size = deviceObjPtr->bar2_size;

        bindExtInfo.update_pciInfo = GT_TRUE;
        bindExtInfo.pciBus  = 0;
        bindExtInfo.pciDev  = deviceObjPtr->deviceId;
        bindExtInfo.pciFunc = 0;

        scibBindExt(devId,&bindExtInfo);
    }
}

/**
* @internal smemInit2 function
* @endinternal
*
* @brief   Init memory module for a device - after the load of the default
*         registers file
* @param[in] deviceObjPtr             - pointer to device object.
*/
void smemInit2
(
    IN SKERNEL_DEVICE_OBJECT * deviceObjPtr
)
{

    switch(deviceObjPtr->deviceFamily)
    {
        case SKERNEL_CHEETAH_1_FAMILY:
        case SKERNEL_CHEETAH_2_FAMILY:
        case SKERNEL_CHEETAH_3_FAMILY:
        case SKERNEL_XCAT_FAMILY:
            smemChtInit2(deviceObjPtr);
            break;
        case SKERNEL_XCAT3_FAMILY:
            smemXCat3Init2(deviceObjPtr);
            break;
         case SKERNEL_AC5_FAMILY:
              smemAc5Init2(deviceObjPtr);
              break;
        case SKERNEL_XCAT2_FAMILY:
            smemXCat2Init2(deviceObjPtr);
            break;
        case SKERNEL_LION_PORT_GROUP_SHARED_FAMILY:
        case SKERNEL_LION_PORT_GROUP_FAMILY:
            smemLionInit2(deviceObjPtr);
            break;
        case SKERNEL_LION2_PORT_GROUP_SHARED_FAMILY:
        case SKERNEL_LION2_PORT_GROUP_FAMILY:
            smemLion2Init2(deviceObjPtr);
            break;
        case SKERNEL_LION3_PORT_GROUP_SHARED_FAMILY:
        case SKERNEL_LION3_PORT_GROUP_FAMILY:
            smemLion3Init2(deviceObjPtr);
            break;
        case SKERNEL_PHY_CORE_FAMILY:
            smemPhyInit2(deviceObjPtr);
            break;
        case SKERNEL_MACSEC_FAMILY:
            smemMacsecInit2(deviceObjPtr);
            break;
        case SKERNEL_BOBCAT2_FAMILY:
            smemBobcat2Init2(deviceObjPtr);
            break;
        case SKERNEL_BOBK_CAELUM_FAMILY:
        case SKERNEL_BOBK_CETUS_FAMILY:
            smemBobkInit2(deviceObjPtr);
            break;
        case SKERNEL_BOBK_ALDRIN_FAMILY:
        case SKERNEL_AC3X_FAMILY:
            smemBobkAldrinInit2(deviceObjPtr);
            break;
        case SKERNEL_BOBCAT3_FAMILY:
            smemBobcat3Init2(deviceObjPtr);
            break;
        case SKERNEL_ALDRIN2_FAMILY:
            smemAldrin2Init2(deviceObjPtr);
            break;
        case SKERNEL_PIPE_FAMILY:
            smemPipeInit2(deviceObjPtr);
            break;
        case SKERNEL_FALCON_FAMILY:
            smemFalconInit2(deviceObjPtr);
            break;
        case SKERNEL_HAWK_FAMILY:
            smemHawkInit2(deviceObjPtr);
            break;
        case SKERNEL_PHOENIX_FAMILY:
            smemPhoenixInit2(deviceObjPtr);
            break;
        case SKERNEL_HARRIER_FAMILY:
            smemHarrierInit2(deviceObjPtr);
            break;
        case SKERNEL_IRONMAN_FAMILY:
            smemIronmanInit2(deviceObjPtr);
            break;
        default:
            break;
    }

    if(SKERNEL_DEVICE_FAMILY_CHEETAH_3_ONLY(deviceObjPtr))
    {
        /* Sleep for 1 seconds */
        /* it needs to prevent crash on ch3 (linux simulation) */
        SIM_OS_MAC(simOsSleep)(1000);
    }

    /* Init done */
    /* State init status to global control registers */
    smemChtInitStateSet(deviceObjPtr);

    return;
}

/**
* @internal smemRegSet function
* @endinternal
*
* @brief   Write data to the register.
*
* @note The function aborts application if address not exist.
*
*/
void smemRegSet
(
    IN SKERNEL_DEVICE_OBJECT * deviceObjPtr,
    IN GT_U32                  address,
    IN GT_U32                  data
)
{
    GT_U32                  * findMemPtr;
    GT_U32                    memSize;
    SCIB_MEMORY_ACCESS_TYPE accessType = SKERNEL_MEMORY_WRITE_E;
    memSize = 1;

    smemConvertDevAndAddrToNewDevAndAddr(deviceObjPtr,address,accessType,
        &deviceObjPtr,&address);

    /* check device type and call device search memory function*/
    findMemPtr = smemFindMemory(deviceObjPtr,address,  memSize, NULL,
                                accessType);

    /* log memory information */
    simLogMemory(deviceObjPtr, SIM_LOG_MEMORY_WRITE_E, SIM_LOG_MEMORY_DEV_E,
                                            address, data, *findMemPtr);

    /* Write memory to the register's memory */
    *findMemPtr = data;

    /* send write register message to Visual address ASIC */
    smemSendDataToVisualAsic(deviceObjPtr->deviceId, address, findMemPtr, 1);
}

/**
* @internal smemRegFldGet function
* @endinternal
*
* @brief   Read data from the register's field
*
* @note The function aborts application if address not exist.
*
*/
void smemRegFldGet
(
    IN SKERNEL_DEVICE_OBJECT * deviceObjPtr,
    IN GT_U32                  address,
    IN GT_U32                  fldFirstBit,
    IN GT_U32                  fldSize,
    OUT GT_U32               * data
)
{
    GT_U32 regValue, fieldMask;

    if (deviceObjPtr == 0)
    {
        skernelFatalError("smemRegSet : illegal pointer\n");
    }

    paramCheck(fldFirstBit,fldSize);

    /* Read register value */
    smemRegGet(deviceObjPtr, address, &regValue);

    /* Align data right according to the first field bit */
    regValue >>= fldFirstBit;

    fldSize = fldSize % 32;
    /* get field's bits mask */
    if (fldSize)
    {
        fieldMask = ~(0xffffffff << fldSize);
    }
    else
    {
        fieldMask = (0xffffffff);
    }

    *data = regValue & fieldMask;
}

/**
* @internal smemPciRegFldGet function
* @endinternal
*
* @brief   Read data from the PCI register's field
*
* @note The function aborts application if address not exist.
*
*/
void smemPciRegFldGet
(
    IN SKERNEL_DEVICE_OBJECT * deviceObjPtr,
    IN GT_U32                  address,
    IN GT_U32                  fldFirstBit,
    IN GT_U32                  fldSize,
    OUT GT_U32               * data
)
{
    GT_U32 * findMemPtr;
    GT_U32  regValue, fieldMask;
    SCIB_MEMORY_ACCESS_TYPE accessType = SKERNEL_MEMORY_READ_PCI_E;

    smemConvertDevAndAddrToNewDevAndAddr(deviceObjPtr,address,accessType,
        &deviceObjPtr,&address);

    paramCheck(fldFirstBit,fldSize);

    /* check device type and call device search memory function*/
    findMemPtr = smemFindMemory(deviceObjPtr, address, 1, NULL,
                                accessType);
    regValue = findMemPtr[0];

    /* Align data right according to the first field bit */
    regValue >>= fldFirstBit;

    fldSize = fldSize % 32;
    /* get field's bits mask */
    if (fldSize)
    {
        fieldMask = ~(0xffffffff << fldSize);
    }
    else
    {
        fieldMask = (0xffffffff);
    }

    *data = regValue & fieldMask;

    /* log memory information */
    simLogMemory(deviceObjPtr, SIM_LOG_MEMORY_PCI_READ_E, SIM_LOG_MEMORY_DEV_E,
                                            address, *data, 0);
}
/**
* @internal smemRegFldSet function
* @endinternal
*
* @brief   Write data to the register's field
*
* @note The function aborts application if address not exist.
*
*/
void smemRegFldSet
(
    IN SKERNEL_DEVICE_OBJECT * deviceObjPtr,
    IN GT_U32                  address,
    IN GT_U32                  fldFirstBit,
    IN GT_U32                  fldSize,
    IN GT_U32                  data
)
{
    GT_U32 regValue, fieldMask;

    if (deviceObjPtr == 0)
    {
        skernelFatalError("smemRegSet : illegal pointer\n");
    }

    paramCheck(fldFirstBit,fldSize);

    fldSize = fldSize % 32;
    /* get field's bits mask */
    if (fldSize)
    {
        fieldMask = ~(0xffffffff << fldSize);
    }
    else
    {
        fieldMask = (0xffffffff);
    }

    /* Get register value */
    smemRegGet(deviceObjPtr, address, &regValue);
    /* Set field value in the register value using
    inverted real field mask in the place of the field */
    regValue &= ~(fieldMask << fldFirstBit);
    regValue |= (data << fldFirstBit);
    smemRegSet(deviceObjPtr, address, regValue);
}


/**
* @internal smemPciRegSet function
* @endinternal
*
* @brief   Write data to the PCI register's
*
* @note The function aborts application if address not exist.
*
*/
void smemPciRegSet
(
    IN SKERNEL_DEVICE_OBJECT * deviceObjPtr,
    IN GT_U32                  address,
    IN GT_U32                  data
)
{
    smemPciRegFldSet(deviceObjPtr, address, 0, 32, data);
}


/**
* @internal smemPciRegGet function
* @endinternal
*
* @brief   Read data from PCI register's
*
* @param[out] dataPtr                  - pointer to register's value.
*
* @note The function aborts application if address not exist.
*
*/
void smemPciRegGet
(
    IN SKERNEL_DEVICE_OBJECT * deviceObjPtr,
    IN GT_U32                  address,
    OUT GT_U32                  *dataPtr
)
{
    smemPciRegFldGet(deviceObjPtr, address, 0, 32, dataPtr);
}

/**
* @internal smemPciRegFldSet function
* @endinternal
*
* @brief   Write data to the PCI register's field
*
* @note The function aborts application if address not exist.
*
*/
void smemPciRegFldSet
(
    IN SKERNEL_DEVICE_OBJECT * deviceObjPtr,
    IN GT_U32                  address,
    IN GT_U32                  fldFirstBit,
    IN GT_U32                  fldSize,
    IN GT_U32                  data
)
{
    GT_U32 * findMemPtr;
    GT_U32 regValue, fieldMask;
    SCIB_MEMORY_ACCESS_TYPE accessType = SKERNEL_MEMORY_WRITE_PCI_E;

    smemConvertDevAndAddrToNewDevAndAddr(deviceObjPtr,address,accessType,
        &deviceObjPtr,&address);

    paramCheck(fldFirstBit,fldSize);

    fldSize = fldSize % 32;
    /* get field's bits mask */
    if (fldSize)
    {
        fieldMask = ~(0xffffffff << fldSize);
    }
    else
    {
        fieldMask = 0xffffffff;
    }

    /* Get register value */
    findMemPtr = smemFindMemory(deviceObjPtr, address, 1, NULL,
                                accessType);
    regValue = findMemPtr[0];
    /* Set field value in the register value using
    inverted real field mask in the place of the field */
    regValue &= ~(fieldMask << fldFirstBit);
    regValue |= (data << fldFirstBit);

    /* log memory information */
    simLogMemory(deviceObjPtr, SIM_LOG_MEMORY_PCI_WRITE_E, SIM_LOG_MEMORY_DEV_E,
                                            address, regValue, *findMemPtr);
    *findMemPtr = regValue;
}

/**
* @internal smemRegGet function
* @endinternal
*
* @brief   Read data from the register.
*
* @param[out] data                     - pointer to register's value.
*
* @note The function aborts application if address not exist.
*
*/
void smemRegGet
(
    IN SKERNEL_DEVICE_OBJECT * deviceObjPtr,
    IN GT_U32                  address,
    OUT GT_U32                * data
)
{
    GT_U32                  * findMemPtr = 0;
    GT_U32                    memSize;
    SCIB_MEMORY_ACCESS_TYPE accessType = SKERNEL_MEMORY_READ_E;

    memSize = 1;

    smemConvertDevAndAddrToNewDevAndAddr(deviceObjPtr,address,accessType,
        &deviceObjPtr,&address);

    /* check device type and call device search memory function */
    findMemPtr = smemFindMemory(deviceObjPtr, address, memSize, NULL,
                                accessType);

    /* log memory information */
    simLogMemory(deviceObjPtr, SIM_LOG_MEMORY_READ_E, SIM_LOG_MEMORY_DEV_E,
                                            address, *findMemPtr, 0);

    /* Read memory from the register's memory */
    *data = *findMemPtr;
}

/*******************************************************************************
*   smemMemGet
*
* DESCRIPTION:
*       Return pointer to the register's or tables's memory.
*
* INPUTS:
*       deviceObj   - pointer to device object.
*       address     - address of memory(register or table).
*
* OUTPUTS:
*       None.
*
* RETURNS:
*        pointer to the memory location
*
* COMMENTS:
*      The function aborts application if address not exist.
*
*******************************************************************************/
void * smemMemGet
(
    IN SKERNEL_DEVICE_OBJECT * deviceObjPtr,
    IN GT_U32                  address
)
{
    GT_U32                  * findMemPtr;
    GT_U32                  memSize;
    SCIB_MEMORY_ACCESS_TYPE accessType = SKERNEL_MEMORY_READ_E;

    smemConvertDevAndAddrToNewDevAndAddr(deviceObjPtr,address,accessType,
        &deviceObjPtr,&address);

    memSize = 1;

    /* check device type and call device search memory function*/
    findMemPtr = smemFindMemory(deviceObjPtr, address, memSize, NULL,
                                accessType);

    /* Does memory exist check */
    if (findMemPtr)
    {
        /* log memory information */
        simLogMemory(deviceObjPtr, SIM_LOG_MEMORY_READ_E, SIM_LOG_MEMORY_DEV_E,
                                                address, *findMemPtr, 0);
    }

    return findMemPtr;
}

/**
* @internal smemMemSet function
* @endinternal
*
* @brief   Write data to a register or table.
*
* @note The function aborts application if address not exist.
*
*/
void smemMemSet
(
    IN SKERNEL_DEVICE_OBJECT * deviceObjPtr,
    IN GT_U32                  address,
    IN GT_U32                * dataPtr,
    IN GT_U32                  dataSize
)
{
    GT_U32                  * findMemPtr;
    GT_U32                    i;
    SCIB_MEMORY_ACCESS_TYPE accessType = SKERNEL_MEMORY_WRITE_E;

    smemConvertDevAndAddrToNewDevAndAddr(deviceObjPtr,address,accessType,
        &deviceObjPtr,&address);

    /* check device type and call device search memory function*/
    findMemPtr = smemFindMemory(deviceObjPtr, address, dataSize, NULL,
                                accessType);

    if (dataSize == 0)
    {
        skernelFatalError("smemMemSet : memory size not valid \n");
    }

    /* Write memory to the register's or tables's memory */
    for (i = 0; i < dataSize; i++)
    {
        /* log memory information */
        simLogMemory(deviceObjPtr, SIM_LOG_MEMORY_WRITE_E, SIM_LOG_MEMORY_DEV_E,
                                                address, dataPtr[i], findMemPtr[i]);
        findMemPtr[i] = dataPtr[i];
    }

    /* send to Visual ASIC */
    smemSendDataToVisualAsic(
            deviceObjPtr->deviceId, address, findMemPtr, dataSize);

}

#define GM_BAD_ADDR_CNS     0x000badad
/* return indication that unit is valid or not*/
static GT_BOOL isUnitValidForCpu(
    IN SKERNEL_DEVICE_OBJECT   * kernelDevObjPtr,
    IN SCIB_MEMORY_ACCESS_TYPE accessType,
    IN GT_U32 address,
    IN GT_U32 *memPtr
)
{
    GT_BOOL unitInUse;

    unitInUse = kernelDevObjPtr->cpssDxChHwIsUnitUsedFuncPtr(
        kernelDevObjPtr->cpssDevNum, 0, address);

    if(unitInUse == GT_FALSE)
    {
        if(accessType == SCIB_MEMORY_READ_E)
        {
            *memPtr = GM_BAD_ADDR_CNS;
        }
    }

    return unitInUse;
}


/**
* @internal simLogBar2AddrCompletion function
* @endinternal
*
* @brief   LOG the BAR2 address completion in case that the 'memory access' is
*          under LOG
*
* @note
*
*/
void simLogBar2AddrCompletion(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32                  oldAddress,
    IN GT_U32                  newAddress
)
{
    /* print read row */
    simLogMessage(NULL, NULL, 0, devObjPtr, SIM_LOG_INFO_TYPE_MEMORY_E,
          "BAR2 convert [0x%08x] to address [0x%08x] \n",
          oldAddress, newAddress);

    return;
}

/**
* @internal smemReadWriteMem function
* @endinternal
*
* @brief   Read/Write data to/from a register or table.
*
* @param[in] accessType               - define access operation Read or Write.
*                                      deviceObj   - pointer to device object.
* @param[in] address                  -  of memory(register or table).
*                                      dataPtr     - new data location for memory .
*                                      dataSize    - size of memory location to be updated.
* @param[in,out] memPtr                   - for Write this pointer to application memory,which
*                                      will be copied to the ASIC memory .
*
* @note The function aborts application if address not exist.
*
*/
void  smemReadWriteMem
(
    IN SCIB_MEMORY_ACCESS_TYPE accessType,
    IN void * deviceObjPtr,
    IN GT_U32 address,
    IN GT_U32 memSize,
    INOUT GT_U32 * memPtr
)
{
    SKERNEL_DEVICE_OBJECT   * kernelDevObjPtr; /* pointer to a deviceObject*/
    SMEM_ACTIVE_MEM_ENTRY_STC * activeMemPtr = NULL;
                                /* pointer to active memory entry*/
    GT_U32                  * findMemPtr; /* pointer to a memory location */
    GT_U32                    i; /* iterator */
    SIM_OS_TASK_COOKIE_INFO_STC  *myTaskInfoPtr = NULL;
    GT_U32                      orig_currentPipeId = 0;
    GT_U32                  memSizeHiddenInfo;
    GT_U32                  isDevMemory;
    GT_U32                  pipeIdFromAddress;
    GT_U32                  usePexBar0 = 0;/* indication that the access address relate to BAR0 */

    SIM_LOG_MEMORY_SOURCE_ENT memAccessSource = SIM_LOG_MEMORY_DEV_E;
    SIM_LOG_MEMORY_ACTION_ENT memAccessAction;

    /* check device type and call device search memory function*/
    kernelDevObjPtr = (SKERNEL_DEVICE_OBJECT *)deviceObjPtr;

    if(!IS_SKERNEL_OPERATION_MAC(accessType))
    {
        memAccessSource = SIM_LOG_MEMORY_CPU_E;

        /* Check if operation type is 'BAR0' / 'BAR2' */
        if(IS_BAR0_OPERATION_MAC(accessType))
        {
            usePexBar0 = 1;
            /* remove the 'PEX BAR0' prefix that the OS (Linux) gave the device */
            address &= (kernelDevObjPtr->bar0_size - 1);
        }
        else
        if(IS_BAR2_OPERATION_MAC(accessType))
        {
            GT_U32  origAddr = address;
            SMEM_CHT_GENERIC_DEV_MEM_INFO *devMemInfoPtr = kernelDevObjPtr->deviceMemory;

            if(NULL == devMemInfoPtr->common.bar2CnMiAtuFunc)
            {
                /* indication of non-completion address */
                skernelFatalError("smemReadWriteMem : BAR2 missing devMemInfoPtr->common.bar2CnMiAtuFunc \n");
                return;
            }

            devMemInfoPtr->common.bar2CnMiAtuFunc(kernelDevObjPtr,
                address,    /* the orig addr as accessed from the PEX */
                &address);  /* the 'cider' address after 'address complition' that the CnM iATU did */

            if(simLogIsOpenFlag && simLogInfoTypeMemory)
            {
                /* LOG the info in case that the 'memory access' is under LOG */
                simLogBar2AddrCompletion(kernelDevObjPtr,origAddr,address);
            }

            if(address == SMAIN_NOT_VALID_CNS)
            {
                /* indication of non-completion address */
                if(!IS_WRITE_OPERATION_MAC(accessType))
                {
                    * memPtr = 0x00badadd;
                }
                return ;
            }

            /* update the access type */
            accessType = accessType == SCIB_MEMORY_READ_BAR2_E ?
                            SCIB_MEMORY_READ_E :
                            SCIB_MEMORY_WRITE_E ;
        }
    }

    if(IS_PCI_OPERATION_MAC(accessType))
    {
        if(!(kernelDevObjPtr->interfaceBmp & SCIB_BUS_INTERFACE_PEX) &&
           address == 0)/* the caller is trying to do 'PCI-scan' */
        {
            if(IS_WRITE_OPERATION_MAC(accessType))
            {
                skernelFatalError("smemReadWriteMem : the device not supports / not connected to the PEX configuration space \n");
            }
            else
            {
                * memPtr = 0xFFFFFFFF;
                return ;
            }
        }

        if(IS_WRITE_OPERATION_MAC(accessType))
        {
            memAccessAction = SIM_LOG_MEMORY_PCI_WRITE_E;
        }
        else
        {
            memAccessAction = SIM_LOG_MEMORY_PCI_READ_E;
        }

        memSizeHiddenInfo = SCIB_MEM_ACCESS_PCI_E + 1;
        isDevMemory = 0;
    }
    else
    if(IS_DFX_OPERATION_MAC(accessType) &&
        SMEM_CHT_IS_DFX_ON_UNIQUE_MEMORY_SPCAE(kernelDevObjPtr))/* indication that the device supports 'DFX memory space' */
    {
        if(IS_WRITE_OPERATION_MAC(accessType))
        {
            memAccessAction = SIM_LOG_MEMORY_DFX_WRITE_E;
        }
        else
        {
            memAccessAction = SIM_LOG_MEMORY_DFX_READ_E;
        }

        memSizeHiddenInfo = SCIB_MEM_ACCESS_DFX_E + 1;
        isDevMemory = 0;
    }
    else
    {
        if(IS_WRITE_OPERATION_MAC(accessType))
        {
            memAccessAction = SIM_LOG_MEMORY_WRITE_E;
        }
        else
        {
            memAccessAction = SIM_LOG_MEMORY_READ_E;
        }

        memSizeHiddenInfo = 0;
        isDevMemory = 1;
    }

    if ((address & 0x3) != 0)
    {
        if(kernelDevObjPtr->supportAnyAddress == GT_FALSE)
        {
            skernelFatalError("\n\n   smemReadWriteMem : illegal address: [0x%8.8x] ((address& 0x3) != 0)\n",address);
        }
    }

    if(usePexBar0)
    {
        /* must NOT convert the address to 'pipe' and or 'mg'     */
        /* as the address hold 'prefix' of the address on the PEX */
    }
    else
    if((kernelDevObjPtr->numOfPipes >= 2 || kernelDevObjPtr->numOfMgUnits >= 2) &&
       (memAccessAction == SIM_LOG_MEMORY_WRITE_E ||
        memAccessAction == SIM_LOG_MEMORY_READ_E))
    {
        myTaskInfoPtr = simOsTaskOwnTaskCookieGet();
        if(myTaskInfoPtr == NULL)
        {
            /* need to register the thread with ability to set pipeId */
            SIM_OS_MAC(simOsTaskOwnTaskPurposeSet)(
                SIM_OS_TASK_PURPOSE_TYPE_CPU_APPLICATION_E,NULL);

            myTaskInfoPtr = simOsTaskOwnTaskCookieGet();
            if(myTaskInfoPtr == NULL)
            {
                skernelFatalError("smemReadWriteMem : failed to register the 'current task' for multi-pipe support \n");
            }
        }
        else
        {
            orig_currentPipeId = myTaskInfoPtr->currentPipeId;
        }

        pipeIdFromAddress = 0;
        /* set currentPipeId for the active memory operation (if needed) */
        (void)SMEM_UNIT_PIPE_OFFSET_GET_MAC(kernelDevObjPtr, address ,
            &pipeIdFromAddress);
        smemSetCurrentPipeId(kernelDevObjPtr,pipeIdFromAddress);

        /* if the address belong to MG unit , we need to update the manager about it */
        smemSetCurrentMgUnitIndex_byRegAddr(kernelDevObjPtr,address);
    }

   /* Avoid memory access to simulation in 'Emulator Mode' for not supported units */
    if (isDevMemory &&
        simulationCheck_onEmulator() &&
        kernelDevObjPtr->cpssDxChHwIsUnitUsedFuncPtr)
    {
        if(GT_FALSE == isUnitValidForCpu(kernelDevObjPtr,accessType,address,memPtr))
        {
            /* the unit not valid for CPU to access */
            /* no more to do */
            return;
        }
    }

    if(usePexBar0)
    {
        /* must NOT convert the address */
        /* as the address hold 'prefix' of the address on the PEX */
    }
    else
    {
        smemConvertDevAndAddrToNewDevAndAddr(kernelDevObjPtr,address,accessType,
            &kernelDevObjPtr,&address);
    }

    findMemPtr = smemFindMemory(kernelDevObjPtr, address, memSize,
                                &activeMemPtr, accessType);

    if (memSize == 0)
    {
        skernelFatalError("smemReadWriteMem : memory size not valid \n");
    }

    /* Read memory from the register's or tables's memory */
    if (SMEM_ACCESS_READ_FULL_MAC(accessType))
    {

        /* check active memory existance */
        if ((activeMemPtr != NULL) && (activeMemPtr->readFun != NULL))
        {
            /* log memory information */
            simLogMemory(deviceObjPtr, memAccessAction, memAccessSource,
                                                      address, findMemPtr[0], 0);

            memSize |= memSizeHiddenInfo << 29;
            activeMemPtr->readFun(kernelDevObjPtr,address,memSize,findMemPtr,
                                    activeMemPtr->readFunParam,memPtr);

            memSize &= 0xFFFF;

            /* those active read may change the value of the registers */
            /* send to Visual ASIC */
            smemSendDataToVisualAsic(kernelDevObjPtr->deviceId,
                                     address,findMemPtr,memSize);
        }
        else
        {
            for (i = 0; i < memSize; i++)
            {
                /* log memory information */
                simLogMemory(deviceObjPtr, memAccessAction, memAccessSource,
                        address, findMemPtr[i]/*value*/, 0);
                memPtr[i] = findMemPtr[i];
            }
        }
    }
    /* Write memory to the register's or tables's memory */
    else
    {
        /* check active memory existance */
        if ((activeMemPtr != NULL) && (activeMemPtr->writeFun != NULL))
        {
            /* log memory information (before the active memory) */
            simLogMemory(deviceObjPtr, memAccessAction, memAccessSource,
                                               address, *memPtr /*new*/, *findMemPtr/*old*/);

            memSize |= memSizeHiddenInfo << 29;
            activeMemPtr->writeFun(kernelDevObjPtr,address,memSize,findMemPtr,
                                    activeMemPtr->writeFunParam,memPtr);
            memSize &= 0xFFFF;

        }
        else
        {
            for (i = 0; i < memSize; i++)
            {
                /* log memory information */
                simLogMemory(deviceObjPtr, memAccessAction, memAccessSource,
                    address, memPtr[i]/* new value*/, findMemPtr[i]/* old value*/);

                findMemPtr[i] = memPtr[i];
            }
        }

        /* send to Visual ASIC */
        smemSendDataToVisualAsic(kernelDevObjPtr->deviceId,
                                 address,findMemPtr,memSize);
    }

    if(myTaskInfoPtr)
    {
        /* restore default pipe-ID .
           needed for skernel that uses SCIB for active memories */
        smemSetCurrentPipeId(kernelDevObjPtr,orig_currentPipeId);
    }
}


/**
* @internal smemDebugModeByPassFatalErrorOnMemNotExists function
* @endinternal
*
* @brief   debug utill - set a flag to allow to by pass the fatal error on accessing to
*         non-exists memory. this allow to use test of memory without the need to stop
*         on every memory .
*         we can get a list of all registers at once.
*
* @param[out] byPassFatalErrorOnMemNotExists - bypass or no the fatal error on accessing
*                                      to non exists memory
*                                       pointer to the memory location
*
* @retval NULL                     - if memory not exist
*/
void smemDebugModeByPassFatalErrorOnMemNotExists(
    IN GT_U32 byPassFatalErrorOnMemNotExists
)
{
    debugModeByPassFatalErrorOnMemNotExists = byPassFatalErrorOnMemNotExists;
}

static GT_U32  debugAddress = 0x1;
void set_debugAddress(IN GT_U32 new_debugAddress)
{
    debugAddress = new_debugAddress;
}
/*******************************************************************************
*   smemFindMemory
*
* DESCRIPTION:
*       Return pointer to the register's or tables's memory.
*
* INPUTS:
*       deviceObj   - pointer to device object.
*       address     - address of memory(register or table).
*
* OUTPUTS:
*     activeMemPtrPtr - pointer to the active memory entry or NULL if not exist.
*
* RETURNS:
*        pointer to the memory location
*        NULL - if memory not exist
*
* COMMENTS:
*        find memory of fa adapter memory space also.
*
*******************************************************************************/
static void * smemFindMemory
(
    IN SKERNEL_DEVICE_OBJECT * deviceObjPtr,
    IN GT_U32                  address,
    IN GT_U32                  memSize,
    OUT SMEM_ACTIVE_MEM_ENTRY_STC ** activeMemPtrPtr,
    IN SCIB_MEMORY_ACCESS_TYPE accessType
)
{
    void *  returnMemory;
    static GT_U32   debugMemoryArray[256];
    SMEM_DEV_MEMORY_FIND_FUN   findMemFunc = deviceObjPtr->devFindMemFunPtr;
    static GT_U32   dummyPciConfigMemoryValue_read = SMAIN_NOT_VALID_CNS;/*dummy space to read from*/
    static GT_U32   dummyPciConfigMemoryValue_write;/*dummy space to write to */

    if(address == debugAddress)
    {
        /* put break point here */
        address = debugAddress;
    }

    if (SMEM_ACCESS_PCI_FULL_MAC(accessType) &&
        deviceObjPtr->notSupportPciConfigMemory)
    {
        /* the device NOT supports PCI config memory */
        /* return pointer to dummy value */
        if (SMEM_ACCESS_READ_FULL_MAC(accessType))
        {
            return &dummyPciConfigMemoryValue_read;
        }
        else
        {
            /* dummy place to write to (not impact the 'read' value !)*/
            return &dummyPciConfigMemoryValue_write;
        }


    }



    if ((address & 0x3) != 0)
    {
        if(deviceObjPtr->supportAnyAddress == GT_FALSE)
        {
            skernelFatalError("\n\n   smemFindMemory : illegal address: [0x%8.8x] ((address& 0x3) != 0)\n",address);
        }
    }
    /* Try to find address in common memory first */
    returnMemory = smemFindCommonMemory(deviceObjPtr, address, memSize, accessType , activeMemPtrPtr);
    if(returnMemory)
    {
        return returnMemory;
    }

    returnMemory =  findMemFunc(deviceObjPtr, accessType, address,memSize, activeMemPtrPtr);

    if (returnMemory == NULL)
    {
        if(GT_FALSE == skernelUserDebugInfo.disableFatalError)
        {
            if(debugModeByPassFatalErrorOnMemNotExists)
            {
                returnMemory = debugMemoryArray;

                if(!sinit_global_usePexLogic)
                {
                    debugModeByPassFatalErrorOnMemNotExists--;/* limit the number of times */
                    simWarningPrintf("\n\n smemFindMemory : memory not exist [0x%8.8x]\n",address);
                }
                else
                {
                    /* no decrement on debugModeByPassFatalErrorOnMemNotExists
                       so it can live forever ! */

                    if((address < 0x1000) &&
                       (SMEM_ACCESS_PCI_FULL_MAC(accessType) ||
                        IS_BAR0_OPERATION_MAC(accessType))/* In sip6 we convert PCI config space to 'bar0' transaction */
                       )
                    {
                        /* do not print 'warning' but return ZERO to be like in HW */
                        debugMemoryArray[0] = 0x00000000;
                    }
                    else
                    {
                        /* do not print 'warning' but return 'error value' of the content */
                        debugMemoryArray[0] = 0x00badadd;
                    }
                }
            }
            else
            {
                GT_CHAR*    unitNamePtr ;

                if(SMEM_ACCESS_PCI_FULL_MAC(accessType))
                {
                    unitNamePtr = "PCI/PEX registers";
                }
                else if(SMEM_ACCESS_DFX_FULL_MAC(accessType) &&
                    SMEM_CHT_IS_DFX_ON_UNIQUE_MEMORY_SPCAE(deviceObjPtr))/* indication that the device supports 'DFX memory space' */
                {
                    unitNamePtr = "DFX memory/registers";
                }
                else
                {
                    unitNamePtr = smemUnitNameByAddressGet(deviceObjPtr,address);
                }

                if(unitNamePtr)
                {
                    skernelFatalError("\n\nsmemFindMemory : memory not exist [0x%8.8x] in unit [%s] \n",address,unitNamePtr);
                }
                else
                {
                    skernelFatalError("\n\nsmemFindMemory : memory not exist [0x%8.8x]\n",address);
                }
            }
        }
    }

    return  returnMemory;
}

#if 0 /* old - non-used operations */
/**
* @internal smemSendDataToVisualAsic function
* @endinternal
*
* @brief   send data to VISUAL ASIC on the named pipe
*
* @retval NULL                     - if memory not exist
*
* @note the function will send as many msg as needed to send the total data .
*
*/
static void smemSendDataToVisualAsic
(
    IN GT_U32                      deviceObjID,
    IN GT_U32                      address,
    IN GT_U32                      * dataPtr,
    IN GT_U32                      numWords
)
{
    GT_U32                  i;
    GT_U32                  msgLengh;
    ASICP_msg_info_STC      message;
    GT_U32                  currentNumOfWordsLeft = numWords;
    GT_U32                  numWordsToSendInCurrentLoop;
    GT_U32                  currentAddress = address;
    GT_U32                  isVisualAsicDisabled;

    if(SMEM_SKIP_LOCAL_READ_WRITE)
    {
        skernelFatalError("smemSendDataToVisualAsic : the Application side not holding the Asic so no data to the VisualAsic\n");
        /* the Application side not holding the Asic so no data to the VisualAsic */
        return;
    }

    message.message_type = ASICP_opcodes_write_memory_E;
    message.cpu_id = 1;
    message.device_id = deviceObjID;

    isVisualAsicDisabled = smainIsVisualDisabled();

    /*if visual asic is disabled (1), no need to send messages*/
    if (isVisualAsicDisabled == 1) {
        return;
    }

    do{
        numWordsToSendInCurrentLoop =
            currentNumOfWordsLeft > ASICP_socket_max_tbl_entry_size_CNS ?
            ASICP_socket_max_tbl_entry_size_CNS :
            currentNumOfWordsLeft;

        message.address = currentAddress;
        message.length = numWordsToSendInCurrentLoop;

        for (i = 0; i < numWordsToSendInCurrentLoop; i++)
        {
            message.entry[i] = dataPtr[i];
        }

        msgLengh = sizeof(message.cpu_id) +
                   sizeof(message.device_id) +
                   sizeof(message.message_type) +
                   sizeof(message.address) +
                   sizeof(message.length) +
                   numWordsToSendInCurrentLoop * sizeof(GT_32);

        /*CallNamedPipe(ASICP_STR_NAMED_PIPE_CNS, (void*)&message, msgLengh, NULL,
                      0,&bytesRead, 200);*/
        SIM_OS_MAC(simOsSendDataToVisualAsic)((void*)&message,msgLengh);


        currentNumOfWordsLeft -= numWordsToSendInCurrentLoop;
        currentAddress += numWordsToSendInCurrentLoop*4;
    }while(currentNumOfWordsLeft);
}
#endif
/**
* @internal smemRmuReadWriteMem function
* @endinternal
*
* @brief   Read/Write data to/from a register or table.
*
* @param[in] accessType               - define access operation Read or Write.
*                                      deviceObj   - pointer to device object.
* @param[in] address                  -  of memory(register or table).
*                                      dataPtr     - new data location for memory .
*                                      dataSize    - size of memory location to be updated.
* @param[in,out] memPtr                   - for Write this pointer to application memory,which
*                                      will be copied to the ASIC memory .
*
* @note The function aborts application if address not exist.
*
*/
 void  smemRmuReadWriteMem
(
    IN SCIB_MEMORY_ACCESS_TYPE accessType,
    IN void * deviceObjPtr,
    IN GT_U32 address,
    IN GT_U32 memSize,
    INOUT GT_U32 * memPtr
)
{
    SKERNEL_DEVICE_OBJECT   * kernelDevObjPtr; /* pointer to a deviceObject*/
    SMEM_ACTIVE_MEM_ENTRY_STC * activeMemPtr = NULL;
                                /* pointer to active memory entry*/
    GT_U32                  * findMemPtr; /* pointer to a memory location */
    GT_U32                    i; /* iterator */

    /* check device type and call device search memory function*/
    kernelDevObjPtr = (SKERNEL_DEVICE_OBJECT *)deviceObjPtr;

    smemConvertDevAndAddrToNewDevAndAddr(kernelDevObjPtr,address,accessType,
        &kernelDevObjPtr,&address);

    findMemPtr = smemFindMemory(kernelDevObjPtr, address, memSize,
                                &activeMemPtr, accessType);
    if (findMemPtr == 0)
    {
        skernelFatalError("smemReadWriteMem : memory not exist \n");
    }
    if (memSize == 0)
    {
        skernelFatalError("smemReadWriteMem : memory size not valid \n");
    }
    /* Read memory from the register's or tables's memory */
    if (SMEM_ACCESS_READ_FULL_MAC(accessType))
    {

        /* check active memory existance */
        if ((activeMemPtr != NULL) && (activeMemPtr->readFun != NULL))
        {
            activeMemPtr->readFun(kernelDevObjPtr,address,memSize,findMemPtr,
                                  activeMemPtr->readFunParam  ,
                                  memPtr);
            /* those active read may change the value of the registers */
            /* send to Visual ASIC */
            smemSendDataToVisualAsic(kernelDevObjPtr->deviceId,
                                     address,findMemPtr,memSize);
        }
        else
        {
            for (i = 0; i < memSize; i++)
            {
                memPtr[i] = findMemPtr[i];
            }
        }
    }
    /* Write memory to the register's or tables's memory */
    else
    {
        /* check active memory existance */
        if ((activeMemPtr != NULL) && (activeMemPtr->writeFun != NULL))
        {
            activeMemPtr->writeFun(kernelDevObjPtr,address,memSize,findMemPtr,
                                    activeMemPtr->writeFunParam| 0x80000000,memPtr);
        }
        else
        {
            for (i = 0; i < memSize; i++)
            {
                findMemPtr[i] = memPtr[i];
            }
        }

        /* send to Visual ASIC */
        smemSendDataToVisualAsic(kernelDevObjPtr->deviceId,
                                 address,findMemPtr,memSize);
    }
}

/**
* @internal smemInitBasicMemChunk function
* @endinternal
*
* @brief   init unit memory chunk
*
* @param[in] unitChunksPtr            - pointer to allocated unit chunks
* @param[in] basicMemChunkPtr         - pointer to basic memory chunks info
* @param[in] basicMemChunkIndex       - basic memory chunk index
*
* @param[out] currChunkPtr             - pointer to allocated current memory chunk
* @param[out] fileNamePtr              -  file name that called the allocation
* @param[out] line                     -   in the file that called the allocation
*                                       none
*/
static void smemInitBasicMemChunk
(
    IN SKERNEL_DEVICE_OBJECT  * devObjPtr,
    IN SMEM_UNIT_CHUNKS_STC  *unitChunksPtr,
    IN SMEM_CHUNK_BASIC_STC * basicMemChunkPtr,
    IN GT_U32               basicMemChunkIndex,
    OUT SMEM_CHUNK_STC      * currChunkPtr,
    IN const char*     fileNamePtr,
    IN GT_U32          line
)
{
    GT_U32 totalMemWords; /* total size of allocated memory in words */

    if(basicMemChunkPtr->memFirstAddr == SMAIN_NOT_VALID_CNS)
    {
        memset(currChunkPtr,0,sizeof(currChunkPtr));
        /* indication that this chunk is skipped */
        currChunkPtr->calcIndexType = SMEM_INDEX_IN_CHUNK_TYPE_IGNORED_E;
        return;
    }


    currChunkPtr->memFirstAddr = basicMemChunkPtr->memFirstAddr;
    currChunkPtr->memLastAddr  =
        (basicMemChunkPtr->memFirstAddr + (basicMemChunkPtr->numOfRegisters * 4)) - 4;

    currChunkPtr->enrtyNumWordsAlignement = 0;

    if(basicMemChunkPtr->enrtyNumBytesAlignement)
    {
        if(basicMemChunkPtr->enrtySizeBits == 0)
        {
            skernelFatalError("smemInitBasicMemChunk: memory entry size can not be zero \n" );
        }
        currChunkPtr->lastEntryWordIndex =
            (basicMemChunkPtr->enrtySizeBits - 1) / 32;

        if(basicMemChunkPtr->enrtyNumBytesAlignement % 4 != 0)
        {
            skernelFatalError("smemInitBasicMemChunk: memory entry wrong alignment \n" );
        }
        currChunkPtr->enrtyNumWordsAlignement =
            basicMemChunkPtr->enrtyNumBytesAlignement / 4;

        if(currChunkPtr->lastEntryWordIndex >= currChunkPtr->enrtyNumWordsAlignement)
        {
            skernelFatalError("smemInitBasicMemChunk: memory entry alignment not valid \n" );
        }

        currChunkPtr->forbidDirectWriteToChunk = basicMemChunkPtr->forbidDirectWriteToChunk;
    }

    totalMemWords = basicMemChunkPtr->numOfRegisters;

    if(currChunkPtr->lastEntryWordIndex != 0)
    {
        /* needed support for 'write table on last word' only for memories that
           are not 1 word entry */
        totalMemWords += currChunkPtr->enrtyNumWordsAlignement;
    }

    currChunkPtr->memSize = totalMemWords;
    currChunkPtr->memPtr = smemDeviceObjMemoryAlloc__internal(devObjPtr,totalMemWords , sizeof(SMEM_REGISTER),
        fileNamePtr,line);
    if(currChunkPtr->memPtr == NULL)
    {
        skernelFatalError("smemInitBasicMemChunk: allocation failed for [%d] bytes \n" ,
                            totalMemWords * sizeof(SMEM_REGISTER));
    }
    currChunkPtr->calcIndexType = SMEM_INDEX_IN_CHUNK_TYPE_STRAIGHT_ACCESS_E;
    currChunkPtr->memMask = 0xFFFFFFFC;
    currChunkPtr->rightShiftNumBits = 2;
    currChunkPtr->formulaFuncPtr = NULL;

    if(basicMemChunkPtr->tableOffsetValid)
    {
        currChunkPtr->tableOffsetInBytes = basicMemChunkPtr->tableOffsetInBytes;
    }
    else
    {
        currChunkPtr->tableOffsetInBytes = SMAIN_NOT_VALID_CNS;
    }

    /* Check chunk for overlapping */
    smemUnitChunkOverlapCheck(unitChunksPtr, basicMemChunkIndex, currChunkPtr);
}

/**
* @internal smemInitMemChunk__internal function
* @endinternal
*
* @brief   init unit memory chunk
*
* @param[out] unitChunksPtr            - (pointer to) the unit memories chunks
*                                       none
*/
void smemInitMemChunk__internal
(
    IN SKERNEL_DEVICE_OBJECT  * devObjPtr,
    IN SMEM_CHUNK_BASIC_STC  basicMemChunksArr[],
    IN GT_U32           numOfChunks,
    OUT SMEM_UNIT_CHUNKS_STC  *unitChunksPtr,
    IN const char*     fileNamePtr,
    IN GT_U32          line
)
{
    GT_U32  ii;
    SMEM_CHUNK_STC *currChunkPtr;/*current memory chunk */

    unitChunksPtr->chunksArray = smemDeviceObjMemoryAlloc__internal(devObjPtr,numOfChunks,sizeof(SMEM_CHUNK_STC),fileNamePtr,line);
    if(unitChunksPtr->chunksArray == NULL)
    {
        skernelFatalError("initMemChunk: chunk array allocation failed \n" );
    }

    unitChunksPtr->numOfChunks = numOfChunks;
    currChunkPtr = unitChunksPtr->chunksArray;

    for(ii = 0 ; ii < numOfChunks; ii++, currChunkPtr++ )
    {
        smemInitBasicMemChunk(devObjPtr,unitChunksPtr, &basicMemChunksArr[ii], ii, currChunkPtr,
            fileNamePtr,line);
    }
}

/**
* @internal smemFindMemChunk function
* @endinternal
*
* @brief   init memory chunk
*
* @param[in] devObjPtr                - the device
* @param[in] accessType               - memory access type
* @param[in] unitChunksPtr            - pointer to chunks of memory for a unit
* @param[in] address                  - the  to find in the
*
* @param[out] indexInChunkArrayPtr     - (pointer to) the index in chunksMemArr[x] , that
*                                      is the chosen chunk.
*                                      valid only when the chunk was found (see return value)
* @param[out] indexInChunkMemoryPtr    - (pointer to) the index in chunksMemArr[x]->memPtr[y],
*                                      that represent the 'address'
*                                      valid only when the chunk was found (see return value)
*
* @retval GT_TRUE                  - address belong to the chunks
* @retval GT_FALSE                 - address not belong to the chunks
*/
static GT_BOOL smemFindMemChunk
(
    IN SKERNEL_DEVICE_OBJECT    *devObjPtr,
    IN SCIB_MEMORY_ACCESS_TYPE  accessType,
    IN SMEM_UNIT_CHUNKS_STC    *unitChunksPtr,
    IN GT_U32   address,
    OUT GT_U32  *indexInChunkArrayPtr,
    OUT GT_U32  *indexInChunkMemoryPtr
)
{
    GT_U32  ii;
    GT_U32  offset;
    SMEM_CHUNK_STC  * memChunkArr = unitChunksPtr->chunksArray;
    GT_U32 numOfChunks = unitChunksPtr->numOfChunks;

    /* Set init value */
    *indexInChunkMemoryPtr = 0;

    for(ii = 0 ; ii < numOfChunks; ii++)
    {
        if(address < memChunkArr[ii].memFirstAddr ||
           address > memChunkArr[ii].memLastAddr )
        {
            continue;
        }

        offset = address - memChunkArr[ii].memFirstAddr;

        *indexInChunkArrayPtr = ii;

        switch(memChunkArr[ii].calcIndexType)
        {
            case SMEM_INDEX_IN_CHUNK_TYPE_STRAIGHT_ACCESS_E:
                *indexInChunkMemoryPtr = offset / 4;
                break;
            case SMEM_INDEX_IN_CHUNK_TYPE_MASK_AND_SHIFT_E:
                *indexInChunkMemoryPtr = (offset & memChunkArr[ii].memMask) >> memChunkArr[ii].rightShiftNumBits;
                break;
            case SMEM_INDEX_IN_CHUNK_TYPE_FORMULA_FUNCTION_E:
                if(memChunkArr[ii].formulaFuncPtr == NULL)
                {
                    skernelFatalError("smemFindMemChunk : NULL function pointer \n");
                    break;
                }
                *indexInChunkMemoryPtr = memChunkArr[ii].formulaFuncPtr(devObjPtr,&memChunkArr[ii],address);

                if((*indexInChunkMemoryPtr) == SMEM_ADDRESS_NOT_IN_FORMULA_CNS)
                {
                    /* the address is not in this chunk , need to keep looking
                       for it in other chunks */
                    continue;
                }

                break;
            case SMEM_INDEX_IN_CHUNK_TYPE_IGNORED_E:
                /* this entry is ignored ! */
                continue;
            default:
                skernelFatalError("smemFindMemChunk : bad type \n");
                break;
        }

        if(accessType == SCIB_MEMORY_WRITE_E)
        {
            smemChunkInternalWriteDataCheck(devObjPtr, address, &memChunkArr[ii],
                                            indexInChunkMemoryPtr);
        }

        return GT_TRUE;
    }


    return GT_FALSE;
}

/**
* @internal smemChunkFormulaCellCompare function
* @endinternal
*
* @brief   Function compare two components of formula array
*
* @param[in] cell1Ptr                 - pointer to device formula cell array
* @param[in] cell2Ptr                 - pointer to device formula cell array
*                                       The return value of this function represents whether cell1Ptr is considered
*                                       less than, equal, or grater than cell2Ptr by returning, respectively,
*                                       a negative value, zero or a positive value.
*
* @note function's prototype must be defined return int to avoid warnings and be
*       consistent with qsort() function prototype
*
*/
static int smemChunkFormulaCellCompare
(
    const GT_VOID * cell1Ptr,
    const GT_VOID * cell2Ptr
)
{
    SMEM_FORMULA_DIM_STC * cellFirstPtr = (SMEM_FORMULA_DIM_STC *)cell1Ptr;
    SMEM_FORMULA_DIM_STC * cellNextPtr = (SMEM_FORMULA_DIM_STC *)cell2Ptr;

    return (cellNextPtr->step - cellFirstPtr->step);
}

/**
* @internal smemChunkFormulaSort function
* @endinternal
*
* @brief   Function sorts components of formula array in memory chunk in descending
*         order
* @param[in] formulaCellPtr           - pointer to device formula cell array
* @param[in] formulaCellNum           - formula cell array number
*                                       None
*
* @note Function should be called in phase of memory chunk allocation
*       and ensure descending order of formula steps to be like:
*       0x02040800 + ii 0x8000 + jj 0x1000, where ii 0..5  , jj 0..7
*       This format is used for further calculation of simulation memory indexes.
*
*/
static GT_VOID  smemChunkFormulaSort
(
    IN SMEM_FORMULA_DIM_STC * formulaCellPtr,
    IN GT_U32              formulaCellNum
)
{
    if(formulaCellNum > 1)
    {
        qsort(formulaCellPtr, formulaCellNum, sizeof(SMEM_FORMULA_DIM_STC),
              smemChunkFormulaCellCompare);

    }
}

/**
* @internal smemUnitChunkAddressCompare function
* @endinternal
*
* @brief   Function compare two components of chunks array
*
* @param[in] chunk1Ptr                - pointer to unit chunks array
* @param[in] chunk2Ptr                - pointer to unit chunks array
*                                       The return value of this function represents whether chunk1Ptr is considered
*                                       less than, equal, or grater than chunk2Ptr by returning, respectively,
*                                       a negative value, zero or a positive value.
*
* @note function's prototype must be defined return int to avoid warnings and be
*       consistent with qsort() function prototype
*
*/
int smemUnitChunkAddressCompare
(
    const GT_VOID * chunk1Ptr,
    const GT_VOID * chunk2Ptr
)
{
    SMEM_CHUNK_STC * chunkFirstPtr = (SMEM_CHUNK_STC *)chunk1Ptr;
    SMEM_CHUNK_STC * chunkNextPtr = (SMEM_CHUNK_STC *)chunk2Ptr;

    return (chunkNextPtr->memFirstAddr - chunkFirstPtr->memFirstAddr);
}

/**
* @internal smemFormulaChunkAddressCheck function
* @endinternal
*
* @brief   Check of address belonging to address range according to specific formula
*
* @param[in] address                  -  to be checked
* @param[in] memChunkPtr              - pointer to memory chunk
*
* @param[out] indexArrayPtr            - (pointer to) array of indexes according to
*                                      specific address and formula.
*
* @retval GT_TRUE                  - address match found, GT_FALSE - otherwise
*
* @note If match not found, array indexes value are not relevant.
*
*/
static GT_BOOL  smemFormulaChunkAddressCheck
(
    IN GT_U32               address,
    IN SMEM_CHUNK_STC     * memChunkPtr,
    OUT GT_U32            * indexArrayPtr
)
{
    GT_U32 compIndex;                   /* formula component index */
    GT_U32 addressOffset;               /* offset from chunk base address */
    SMEM_FORMULA_DIM_STC * formulaCellPtr;

    memset(indexArrayPtr, 0, SMEM_FORMULA_CELL_NUM_CNS * sizeof(GT_U32));

    addressOffset = address - memChunkPtr->memFirstAddr;
    /* Address is base address of chunk */
    if(addressOffset == 0)
    {
        return GT_TRUE;
    }

    /* Parse formula components to find indexes  */
    for(compIndex = 0; compIndex < SMEM_FORMULA_CELL_NUM_CNS; compIndex++)
    {
        formulaCellPtr = &memChunkPtr->formulaData[compIndex];

        if (formulaCellPtr->size == 0 || formulaCellPtr->step == 0)
            break;

        indexArrayPtr[compIndex] = addressOffset / formulaCellPtr->step;
        if(indexArrayPtr[compIndex] >= formulaCellPtr->size)
        {
            /* the address not match the formula */
            return GT_FALSE;
        }

        addressOffset %= formulaCellPtr->step;
    }

    /* If reminder not zero - address doesn't belong to chunk range */
    return (addressOffset) ? GT_FALSE : GT_TRUE;
}

/**
* @internal smemFormulaChunkIndexGet function
* @endinternal
*
* @brief   Function which returns the index in the simulation memory chunk
*         that represents the accessed address by formula
* @param[in] devObjPtr                - pointer to device info
* @param[in] memChunkPtr              - pointer to memory chunk info
* @param[in] address                  - the accesses address
*
* @retval index into memChunkPtr   ->memPtr[index] that represent 'address'
*                                       if the return value == SMEM_ADDRESS_NOT_IN_FORMULA_CNS meaning that
*                                       address not belong to formula
*/
static GT_U32  smemFormulaChunkIndexGet
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SMEM_CHUNK_STC        * memChunkPtr,
    IN GT_U32                  address
)
{
    GT_U32 destMemIndex = 0;                /* destination memory index */
    GT_U32 compIndex;                       /* formula component index */
    SMEM_FORMULA_DIM_STC * formulaCellPtr;  /* pointer to formula cell */
    GT_U32 formulaIndexArr[SMEM_FORMULA_CELL_NUM_CNS]; /* array of memory indexes */
    GT_BOOL rc;

    rc = smemFormulaChunkAddressCheck(address, memChunkPtr, formulaIndexArr);
    if(rc == GT_FALSE)
    {
        return SMEM_ADDRESS_NOT_IN_FORMULA_CNS;
    }

    for(compIndex = 0; compIndex < SMEM_FORMULA_CELL_NUM_CNS; compIndex++)
    {
        formulaCellPtr = &memChunkPtr->formulaData[compIndex];

        if (formulaCellPtr->size == 0 || formulaCellPtr->step == 0)
            break;

        /* Memory segment index */
        if(destMemIndex)
        {
            destMemIndex *= formulaCellPtr->size;
        }

        /* Absolute memory index */
        destMemIndex += formulaIndexArr[compIndex];
    }

    return destMemIndex;
}

/**
* @internal smemInitMemChunkExt__internal function
* @endinternal
*
* @brief   Allocates and init unit memory as flat model or using formula
*
* @param[in] chunksMemArrExt[]        - array of extended memory chunks info
* @param[in] numOfChunks              - number of extended chunks
* @param[in] fileNamePtr              -  file name that called the allocation
* @param[in] line                     -   in the file that called the allocation
*
* @param[out] unitChunksPtr            - (pointer to) the unit memories chunks
*                                       none
*/
void smemInitMemChunkExt__internal
(
    IN SKERNEL_DEVICE_OBJECT  * devObjPtr,
    IN SMEM_CHUNK_EXTENDED_STC  chunksMemArrExt[],
    IN GT_U32                   numOfChunks,
    OUT SMEM_UNIT_CHUNKS_STC    *unitChunksPtr,
    IN const char*     fileNamePtr,
    IN GT_U32          line
)
{
    GT_U32  ii;
    GT_U32 memoryWords;
    GT_U32 compIndex;
    SMEM_CHUNK_STC *currChunkPtr;   /*current memory chunk */
    SMEM_FORMULA_DIM_STC * formulaCellPtr;
    GT_U32 totalMemWords; /* total size of allccated memory in words */
    GT_BIT isFormulaHidenParam;/*is the formula hold 'hiden' parameter via the 'numOfRegisters' */

    unitChunksPtr->chunksArray = smemDeviceObjMemoryAlloc__internal(devObjPtr,numOfChunks, sizeof(SMEM_CHUNK_STC),fileNamePtr,line);
    if(unitChunksPtr->chunksArray == NULL)
    {
        skernelFatalError("smemInitMemChunkExt__internal: chunk array allocation failed \n" );
    }

    unitChunksPtr->numOfChunks = numOfChunks;

    currChunkPtr = unitChunksPtr->chunksArray;

    for(ii = 0 ; ii < numOfChunks; ii++, currChunkPtr++ )
    {
        if(chunksMemArrExt[ii].formulaCellArr[0].size ||
           chunksMemArrExt[ii].formulaCellArr[0].step)
        {
            currChunkPtr->enrtyNumWordsAlignement = 0;
            currChunkPtr->memFirstAddr = chunksMemArrExt[ii].memChunkBasic.memFirstAddr;
            memoryWords = 1;
            currChunkPtr->memLastAddr = currChunkPtr->memFirstAddr;

            isFormulaHidenParam = 0;
            /* Memory will be allocated according to formula */
            for(compIndex = 0; compIndex < SMEM_FORMULA_CELL_NUM_CNS; compIndex++)
            {
                formulaCellPtr =
                    &chunksMemArrExt[ii].formulaCellArr[compIndex];

                if (formulaCellPtr->size == 0 || formulaCellPtr->step == 0)
                {
                    /* numOfRegisters == 1 only cause confusion and not change calculations
                       so need to ignore it */
                    if(chunksMemArrExt[ii].memChunkBasic.numOfRegisters > 1)
                    {
                        /* use this info as 'last formula' parameter (size,step) */
                        formulaCellPtr->size = chunksMemArrExt[ii].memChunkBasic.numOfRegisters;
                        formulaCellPtr->step = 4;

                        isFormulaHidenParam = 1;/* cause 'break' after calculating new info */
                    }
                    else
                    {
                        break;
                    }
                }

                memoryWords *= formulaCellPtr->size;
                currChunkPtr->memLastAddr +=
                    (formulaCellPtr->size - 1) * formulaCellPtr->step;

                if(isFormulaHidenParam)
                {
                    /* increment the value due to use by smemChunkFormulaSort() */
                    compIndex++;
                    break;
                }
            }


            totalMemWords = memoryWords;
            currChunkPtr->memSize = totalMemWords;
            currChunkPtr->memPtr = smemDeviceObjMemoryAlloc__internal(devObjPtr,totalMemWords, sizeof(SMEM_REGISTER),fileNamePtr,line);
            if(currChunkPtr->memPtr == 0)
            {
                skernelFatalError("smemInitMemChunkExt__internal: allocation failed for [%d] bytes \n" ,
                    totalMemWords * sizeof(SMEM_REGISTER));
            }
            currChunkPtr->calcIndexType = SMEM_INDEX_IN_CHUNK_TYPE_FORMULA_FUNCTION_E;
            currChunkPtr->formulaFuncPtr = smemFormulaChunkIndexGet;
            memcpy(&currChunkPtr->formulaData[0],
                   &chunksMemArrExt[ii].formulaCellArr[0],
                   sizeof(currChunkPtr->formulaData));

            if(chunksMemArrExt[ii].memChunkBasic.tableOffsetValid)
            {
                currChunkPtr->enrtyNumWordsAlignement = /* support tables bound to this formula*/
                    chunksMemArrExt[ii].memChunkBasic.enrtyNumBytesAlignement / 4;
                currChunkPtr->tableOffsetInBytes = chunksMemArrExt[ii].memChunkBasic.tableOffsetInBytes;
            }
            else
            {
                currChunkPtr->tableOffsetInBytes = SMAIN_NOT_VALID_CNS;
            }

            smemChunkFormulaSort(&currChunkPtr->formulaData[0], compIndex);

            /* Check chunk for overlapping */
            smemUnitChunkOverlapCheck(unitChunksPtr, ii, currChunkPtr);
        }
        else
        {
            smemInitBasicMemChunk(devObjPtr,unitChunksPtr,
                                  &chunksMemArrExt[ii].memChunkBasic,
                                  ii, currChunkPtr,
                                  fileNamePtr,line);
        }
    }
}

/**
* @internal smemInitMemCombineUnitChunks__internal function
* @endinternal
*
* @brief   takes 2 unit chunks and combine them into first unit.
*         NOTEs:
*         1. the function will re-allocate memories for the combined unit
*         2. all the memories of the second unit chunks will be free/reused by
*         first unit after the action done. so don't use those memories afterwards.
*         3. the 'second' memory will have priority over the memory of the 'first' memory.
*         meaning that if address is searched and it may be found in 'first' and
*         in 'second' , then the 'second' memory is searched first !
*         this is to allow 'override' of memories from the 'first' by memories
*         of the 'second'
* @param[in] firstUnitChunks          - (pointer to) the first unit memories chunks
* @param[in] secondUnitChunks         - (pointer to) the second unit memories chunks
* @param[in] fileNamePtr              -  file name that called the allocation
* @param[in] line                     -   in the file that called the allocation
*                                       none
*/
void smemInitMemCombineUnitChunks__internal
(
    IN SKERNEL_DEVICE_OBJECT  * devObjPtr,
    IN SMEM_UNIT_CHUNKS_STC     *firstUnitChunks,
    IN SMEM_UNIT_CHUNKS_STC     *secondUnitChunks,
    IN const char*     fileNamePtr,
    IN GT_U32          line
)
{
    GT_U32 i;
    SMEM_CHUNK_STC currChunk;
    SMEM_CHUNK_STC *tempChunksArray;

    GT_U32  combinedNumChunks = firstUnitChunks->numOfChunks + secondUnitChunks->numOfChunks;

    /* Check for chunks overlapping in source and destination units */
    for(i = 0; i < secondUnitChunks->numOfChunks; i++)
    {
        currChunk.memFirstAddr = secondUnitChunks->chunksArray[i].memFirstAddr;
        currChunk.memLastAddr = secondUnitChunks->chunksArray[i].memLastAddr;
        currChunk.formulaFuncPtr = secondUnitChunks->chunksArray[i].formulaFuncPtr;
        memcpy(&currChunk.formulaData[0],
               &secondUnitChunks->chunksArray[i].formulaData[0],
               sizeof(currChunk.formulaData));
        /* Check chunk for overlapping */
        smemUnitChunkOverlapCheck(firstUnitChunks, firstUnitChunks->numOfChunks,
                                  &currChunk);
    }

    /* resize the allocation of the second (the source) unit */
    tempChunksArray = smemDeviceObjMemoryRealloc__internal(devObjPtr,secondUnitChunks->chunksArray, combinedNumChunks*sizeof(SMEM_CHUNK_STC),
        fileNamePtr,line);
    if (tempChunksArray == NULL) {
        skernelFatalError("smemInitMemCombineUnitChunks__internal: re-allocation failed for [%d] bytes \n" ,
                          combinedNumChunks*sizeof(SMEM_CHUNK_STC));
    }
    secondUnitChunks->chunksArray = tempChunksArray;

    /* copy the info from second unit after the end of the first unit */
    memcpy(&secondUnitChunks->chunksArray[secondUnitChunks->numOfChunks],
           &firstUnitChunks->chunksArray[0],
           sizeof(SMEM_CHUNK_STC)* firstUnitChunks->numOfChunks);

    /* update the number of chunks in the first unit */
    firstUnitChunks->numOfChunks = combinedNumChunks;

    /* free the unused pointer of the first unit */
    smemDeviceObjMemoryPtrFree__internal(devObjPtr,firstUnitChunks->chunksArray,fileNamePtr,line);

    /* update the first unit with the memories of the 'second' */
    firstUnitChunks->chunksArray = secondUnitChunks->chunksArray;

    return;
}

/**
* @internal smemInitMemUpdateUnitChunks__internal function
* @endinternal
*
* @brief   takes 2 unit chunks and combine them into first unit.
*         NOTEs:
*         1. Find all duplicated chunks in destination chunks array and reallocate
*         them according to appended chunk info.
*         2. The function will re-allocate memories for the combined unit
*         3. All the memories of the "append" unit chunks will be free/reused by
*         first unit after the action done. so don't use those memories afterwards.
* @param[in] unitChunksAppendToPtr    - (pointer to) the destination unit memories chunks
* @param[in] unitChunksAppendPtr      - (pointer to) the unit memories chunks to be appended
* @param[in] fileNamePtr              -  file name that called the allocation
* @param[in] line                     -   in the file that called the allocation
*                                       none
*/
void smemInitMemUpdateUnitChunks__internal
(
    IN SKERNEL_DEVICE_OBJECT  * devObjPtr,
    IN SMEM_UNIT_CHUNKS_STC     *unitChunksAppendToPtr,
    IN SMEM_UNIT_CHUNKS_STC     *unitChunksAppendPtr,
    IN const char*     fileNamePtr,
    IN GT_U32          line
)
{
    GT_U32 ii, jj;
    SMEM_CHUNK_STC *newChunkPtr, *foundChunkPtr = 0;
    GT_U32 numRegs;
    GT_U32 totalMemWords; /* total size of allccated memory in words */
    GT_U32  *tempMemPtr;

    ii = 0;
    /* Go through all "append" chunks array */
    while(ii < unitChunksAppendPtr->numOfChunks)
    {
        foundChunkPtr = 0;
        newChunkPtr = &unitChunksAppendPtr->chunksArray[ii];
        /* Find matching in "append to" chunks array */
        for(jj = 0; jj < unitChunksAppendToPtr->numOfChunks; jj++)
        {
            if(newChunkPtr->memFirstAddr ==
               unitChunksAppendToPtr->chunksArray[jj].memFirstAddr)
            {
                /* Chunk found in "append to" chunks array*/
                foundChunkPtr = &unitChunksAppendToPtr->chunksArray[jj];
                /* No chunks duplication in chunks array */
                break;
            }
        }

        if(foundChunkPtr)
        {
            /* Override old chunk info by new one */
            foundChunkPtr->calcIndexType = newChunkPtr->calcIndexType;
            memcpy(foundChunkPtr->formulaData, newChunkPtr->formulaData,
                   sizeof(newChunkPtr->formulaData));
            foundChunkPtr->formulaFuncPtr = newChunkPtr->formulaFuncPtr;
            foundChunkPtr->memFirstAddr = newChunkPtr->memFirstAddr;
            foundChunkPtr->memLastAddr = newChunkPtr->memLastAddr;
            foundChunkPtr->lastEntryWordIndex = newChunkPtr->lastEntryWordIndex;
            foundChunkPtr->forbidDirectWriteToChunk = newChunkPtr->forbidDirectWriteToChunk;
            foundChunkPtr->enrtyNumWordsAlignement = newChunkPtr->enrtyNumWordsAlignement;
            foundChunkPtr->memMask = newChunkPtr->memMask;
            numRegs = (newChunkPtr->memLastAddr - newChunkPtr->memFirstAddr + 4) / 4;

            totalMemWords = numRegs + foundChunkPtr->enrtyNumWordsAlignement;
            foundChunkPtr->memSize = totalMemWords;
            tempMemPtr = smemDeviceObjMemoryRealloc__internal(devObjPtr,foundChunkPtr->memPtr,
                                 totalMemWords * sizeof(SMEM_REGISTER),fileNamePtr,line);
            if (tempMemPtr == NULL) {
                skernelFatalError("smemInitMemUpdateUnitChunks__internal: re-allocation failed for [%d] bytes \n" ,
                                  totalMemWords * sizeof(SMEM_REGISTER));
            }
            foundChunkPtr->memPtr = tempMemPtr;
            foundChunkPtr->rightShiftNumBits = newChunkPtr->rightShiftNumBits;

            if(unitChunksAppendPtr->numOfChunks)
            {
                /* Decrease number of chunks to  be appended to "append to" chunks array */
                unitChunksAppendPtr->numOfChunks--;
                /* Index inside chunks range */
                if(ii < unitChunksAppendPtr->numOfChunks)
                {
                    /* Remove chunk from "append" chunks array */
                    memmove(&unitChunksAppendPtr->chunksArray[ii],
                            &unitChunksAppendPtr->chunksArray[ii + 1],
                            sizeof(SMEM_CHUNK_STC) * (unitChunksAppendPtr->numOfChunks - ii));
                }
            }
        }
        else
        {
            /* Skip to next chunk in "append" chunks array */
            ii++;
        }
    }

    if(unitChunksAppendPtr->numOfChunks)
    {
        /* Combine chunks array */
        smemInitMemCombineUnitChunks__internal(devObjPtr,unitChunksAppendToPtr, unitChunksAppendPtr,
            fileNamePtr,line);
    }
}

/**
* @internal smemUnitChunkAddBasicChunk__internal function
* @endinternal
*
* @brief   add basic chunk to unit chunk.
*         NOTEs:
* @param[in] unitChunkPtr             - (pointer to) unit memories chunk
* @param[in] basicChunkPtr            - (pointer to) basic chunk
* @param[in] numBasicElements         - number of elements in the basic chunk
* @param[in] fileNamePtr              -  file name that called the allocation
* @param[in] line                     -   in the file that called the allocation
*                                       none
*/
void smemUnitChunkAddBasicChunk__internal
(
    IN SKERNEL_DEVICE_OBJECT  * devObjPtr,
    IN SMEM_UNIT_CHUNKS_STC     *unitChunkPtr,
    IN SMEM_CHUNK_BASIC_STC     *basicChunkPtr,
    IN GT_U32                   numBasicElements,
    IN const char*     fileNamePtr,
    IN GT_U32          line
)
{
    SMEM_UNIT_CHUNKS_STC  tmpUnitChunk;

    /* create temp unit */
    smemInitMemChunk__internal(devObjPtr, basicChunkPtr, numBasicElements, &tmpUnitChunk,fileNamePtr,line);

    /*add the tmp unit chunks to the main unit */
    smemInitMemCombineUnitChunks__internal(devObjPtr, unitChunkPtr, &tmpUnitChunk,fileNamePtr,line);
}


/*******************************************************************************
* smemTestDeviceIdToDevPtrConvert
*
* DESCRIPTION:
*        test utility -- convert device Id to device pointer
*
* INPUTS:
*       deviceId       - device id to refer to
*
* OUTPUTS:
*
* RETURN:
*       the device pointer
* COMMENTS:
*   if not found --> fatal error
*
*******************************************************************************/
SKERNEL_DEVICE_OBJECT* smemTestDeviceIdToDevPtrConvert
(
    IN  GT_U32                      deviceId
)
{
    if(deviceId >= SMAIN_MAX_NUM_OF_DEVICES_CNS || smainDeviceObjects[deviceId] == NULL)
    {
        skernelFatalError("snetChtTestDeviceIdToDevPtrConvert: unknown device-id[%d] \n",deviceId);
    }

    return smainDeviceObjects[deviceId];
}

/**
* @internal activeMemoryTableValidity function
* @endinternal
*
* @brief   test utility -- check that all the active memory addresses exists
*         in the device memory
* @param[in] activeMemPtr             - active memory table (of unit / device)
*/
static void  activeMemoryTableValidity(

    IN GT_U32 devNum,
    IN SMEM_ACTIVE_MEM_ENTRY_STC  *activeMemPtr
)
{
    GT_U32  ii;
    GT_U32  data;

    /* read from the registers , if not exists it will cause fatal error */
    for(ii = 0 ;activeMemPtr[ii].address != END_OF_TABLE ;ii++)
    {
        scibReadMemory(devNum,
            activeMemPtr[ii].address & activeMemPtr[ii].mask,
            1,&data);
    }
}

/**
* @internal activeMemoryTestValidity function
* @endinternal
*
* @brief   test utility -- check that all the active memory addresses exists
*         in the device memory
* @param[in] devNum                   - the device index
*/
static GT_U32  activeMemoryTestValidity(
    IN GT_U32 devNum
)
{
    GT_U32  ii;
    SKERNEL_DEVICE_OBJECT    *devObjPtr = smemTestDeviceIdToDevPtrConvert(devNum);
    SMEM_ACTIVE_MEM_ENTRY_STC  *activeMemPtr = devObjPtr->activeMemPtr;
    SMEM_CHT_DEV_COMMON_MEM_INFO  * commonDevMemInfoPtr;

    /* validate the active memory of the device */
    activeMemoryTableValidity(devNum,activeMemPtr);

    /* the device memory always starts with 'common info' */
    commonDevMemInfoPtr = devObjPtr->deviceMemory;

    if(devObjPtr->supportActiveMemPerUnit)
    {
        for(ii = 0 ; ii < SMEM_CHT_NUM_UNITS_MAX_CNS ; ii++)
        {
            if(commonDevMemInfoPtr->specFunTbl[ii].specParam == 0 ||
               commonDevMemInfoPtr->specFunTbl[ii].unitActiveMemPtr == NULL)
            {
                continue;
            }

            /* active memory of the unit */
            activeMemPtr = commonDevMemInfoPtr->specFunTbl[ii].unitActiveMemPtr;

            /* validate the active memory of the unit */
            activeMemoryTableValidity(devNum,activeMemPtr);
        }
    }

    return 0;
}

void dumpUnitActiveMemory
(
    IN GT_U32   devNum,
    IN GT_CHAR* unitName
)
{
    SKERNEL_DEVICE_OBJECT    *devObjPtr = smemTestDeviceIdToDevPtrConvert(devNum);
    SMEM_ACTIVE_MEM_ENTRY_STC  *activeMemPtr;
    SMEM_CHT_DEV_COMMON_MEM_INFO  * commonDevMemInfoPtr;
    GT_U32 unitIndex;
    GT_U32 ii;

    if(SMAIN_NOT_VALID_CNS == smemUnitBaseAddrByNameGet(devObjPtr, unitName, 1))
    {
        simulationPrintf("unitName is wrong: [%s]\n", unitName);
        return;
    }

    unitIndex = UNIT_INDEX_FROM_STRING_GET_MAC(devObjPtr, unitName);
    /* the device memory always starts with 'common info' */
    commonDevMemInfoPtr = devObjPtr->deviceMemory;

    /* active memory of the unit */
    activeMemPtr = commonDevMemInfoPtr->specFunTbl[unitIndex].unitActiveMemPtr;

    for (ii = 0; activeMemPtr[ii].address != END_OF_TABLE; ii++)
    {
        simulationPrintf("[0x%8.8x]\n", activeMemPtr[ii].address);
    }
}

/**
* @internal smemGenericTablesSectionsGet function
* @endinternal
*
* @brief   get the number of tables from each section and the pointer to the sections:
*         section of '1 parameter' tables
*         section of '2 parameters' tables
*         section of '3 parameters' tables
*         section of '4 parameters' tables
*
* @param[out] tablesPtr                - pointer to tables with single parameter
* @param[out] numTablesPtr             -  number of tables with single parameter
* @param[out] tables2ParamsPtr         - pointer to tables with 2 parameters
* @param[out] numTables2ParamsPtr      -number of tables with 2 parameters
* @param[out] tables3ParamsPtr         - pointer to tables with 3 parameters
* @param[out] numTables3ParamsPtr      - number of tables with 3 parameters
* @param[out] tables4ParamsPtr         - pointer to tables with 4 parameters
* @param[out] numTables4ParamsPtr      - number of tables with 4 parameters
*                                      RETURN:
*                                      COMMENTS:
*/
void  smemGenericTablesSectionsGet(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    OUT SKERNEL_TABLE_INFO_STC  **tablesPtr,
    OUT GT_U32  *numTablesPtr,
    OUT SKERNEL_TABLE_2_PARAMETERS_INFO_STC  **tables2ParamsPtr,
    OUT GT_U32  *numTables2ParamsPtr,
    OUT SKERNEL_TABLE_3_PARAMETERS_INFO_STC  **tables3ParamsPtr,
    OUT GT_U32  *numTables3ParamsPtr,
    OUT SKERNEL_TABLE_4_PARAMETERS_INFO_STC  **tables4ParamsPtr,
    OUT GT_U32  *numTables4ParamsPtr
)
{
    *tablesPtr = (SKERNEL_TABLE_INFO_STC*)(void*)(
                (GT_UINTPTR)(void*)&devObjPtr->tablesInfo.placeHolderFor1Parameter + sizeof(devObjPtr->tablesInfo.placeHolderFor1Parameter));
    *numTablesPtr = ((GT_UINTPTR)(void*)&devObjPtr->tablesInfo.placeHolderFor2Parameters) -
                ((GT_UINTPTR)(void*)(*tablesPtr)) ;
    *numTablesPtr /= sizeof(SKERNEL_TABLE_INFO_STC);

    *tables2ParamsPtr = (SKERNEL_TABLE_2_PARAMETERS_INFO_STC*)(void*)(
                (GT_UINTPTR)(void*)&devObjPtr->tablesInfo.placeHolderFor2Parameters + sizeof(devObjPtr->tablesInfo.placeHolderFor2Parameters));
    *numTables2ParamsPtr = ((GT_UINTPTR)(void*)(&devObjPtr->tablesInfo.placeHolderFor3Parameters)) -
                       ((GT_UINTPTR)(void*)(*tables2ParamsPtr));
    *numTables2ParamsPtr /= sizeof(SKERNEL_TABLE_2_PARAMETERS_INFO_STC);

    *tables3ParamsPtr = (SKERNEL_TABLE_3_PARAMETERS_INFO_STC*)(void*)(
                (GT_UINTPTR)(void*)&devObjPtr->tablesInfo.placeHolderFor3Parameters + sizeof(devObjPtr->tablesInfo.placeHolderFor3Parameters));
    *numTables3ParamsPtr = ((GT_UINTPTR)(void*)(&devObjPtr->tablesInfo.placeHolderFor4Parameters)) -
                       ((GT_UINTPTR)(void*)(*tables3ParamsPtr));
    *numTables3ParamsPtr /= sizeof(SKERNEL_TABLE_3_PARAMETERS_INFO_STC);

    *tables4ParamsPtr = (SKERNEL_TABLE_4_PARAMETERS_INFO_STC*)(void*)(
                (GT_UINTPTR)(void*)&devObjPtr->tablesInfo.placeHolderFor4Parameters + sizeof(devObjPtr->tablesInfo.placeHolderFor4Parameters));
    *numTables4ParamsPtr = ((GT_UINTPTR)(void*)(&devObjPtr->tablesInfo.placeHolder_MUST_BE_LAST)) -
                       ((GT_UINTPTR)(void*)(*tables4ParamsPtr));
    *numTables4ParamsPtr /= sizeof(SKERNEL_TABLE_4_PARAMETERS_INFO_STC);

}


/**
* @internal genericTablesTestValidity function
* @endinternal
*
* @brief   test utility -- check that all the general table base addresses exists
*         in the device memory
* @param[in] devNum                   - the device index
*/
GT_U32  genericTablesTestValidity(
    IN GT_U32 devNum
)
{
    GT_U32  ii,jj;
    GT_U32  data;
    SKERNEL_DEVICE_OBJECT  *devObjPtr = smemTestDeviceIdToDevPtrConvert(devNum);
    SKERNEL_TABLE_INFO_STC  *tablesPtr;/* pointer to tables with single parameter */
    GT_U32  numTables;/* number of tables with single parameter */
    SKERNEL_TABLE_2_PARAMETERS_INFO_STC  *tables2ParamsPtr; /* pointer to tables with 2 parameters */
    GT_U32  numTables2Params;/* number of tables with 2 parameters */
    SKERNEL_TABLE_3_PARAMETERS_INFO_STC  *tables3ParamsPtr; /* pointer to tables with 3 parameters */
    GT_U32  numTables3Params;/* number of tables with 3 parameters */
    SKERNEL_TABLE_4_PARAMETERS_INFO_STC  *tables4ParamsPtr; /* pointer to tables with 4 parameters */
    GT_U32  numTables4Params;/* number of tables with 4 parameters */
    GT_U32  baseAddress;
    GT_U32  firstValidAddress;/* allow the first valid address to differ from baseAddress
                                used in validation only if firstValidAddress != 0 */
    GT_U32  localOffset;/*Offset inside the unit*/

    smemGenericTablesSectionsGet(devObjPtr,
                            &tablesPtr,&numTables,
                            &tables2ParamsPtr,&numTables2Params,
                            &tables3ParamsPtr,&numTables3Params,
                            &tables4ParamsPtr,&numTables4Params);

    /* read from the registers , if not exists it will cause fatal error */
    for(ii = 0 ;ii < numTables ;ii++)
    {
        if(tablesPtr[ii].commonInfo.baseAddress == 0)
        {
            /* not supported table in the device */
            continue;
        }

        baseAddress = tablesPtr[ii].commonInfo.baseAddress;

        firstValidAddress = tablesPtr[ii].commonInfo.firstValidAddress ?
            tablesPtr[ii].commonInfo.firstValidAddress :
            baseAddress ;

        smemRegGet(devObjPtr,
                   firstValidAddress,
                   &data);

        /* support multi instance tables (duplications between units) */
        for(jj = 0 ; jj < tablesPtr[ii].commonInfo.multiInstanceInfo.numBaseAddresses ; jj++)
        {
           if(tablesPtr[ii].commonInfo.multiInstanceInfo.multiUnitsBaseAddressValidPtr &&
               tablesPtr[ii].commonInfo.multiInstanceInfo.multiUnitsBaseAddressValidPtr[jj] == GT_FALSE)
            {
                /* this instance of the multi instances is not valid ... skip it */
                continue;
            }

            /*calculate adress inside the unit*/
            localOffset = firstValidAddress - tablesPtr[ii].commonInfo.multiInstanceInfo.multiUnitsBaseAddressPtr[0];

            SET_MULTI_INSTANCE_ADDRESS_MAC(localOffset,
                    tablesPtr[ii].commonInfo.multiInstanceInfo.multiUnitsBaseAddressPtr[jj]);

            smemRegGet(devObjPtr,
                       localOffset,
                       &data);

        }
    }

    /* read from the registers , if not exists it will cause fatal error */
    for(ii = 0 ;ii < numTables2Params ;ii++)
    {
        if(tables2ParamsPtr[ii].commonInfo.baseAddress == 0)
        {
            /* not supported table in the device */
            continue;
        }

        baseAddress = tables2ParamsPtr[ii].commonInfo.baseAddress;

        firstValidAddress = tables2ParamsPtr[ii].commonInfo.firstValidAddress ?
            tablesPtr[ii].commonInfo.firstValidAddress :
            baseAddress ;

        smemRegGet(devObjPtr,
                   firstValidAddress,
                   &data);

        /* support multi instance tables (duplications between units) */
        for(jj = 0 ; jj < tables2ParamsPtr[ii].commonInfo.multiInstanceInfo.numBaseAddresses ; jj++)
        {
           if(tables2ParamsPtr[ii].commonInfo.multiInstanceInfo.multiUnitsBaseAddressValidPtr &&
               tables2ParamsPtr[ii].commonInfo.multiInstanceInfo.multiUnitsBaseAddressValidPtr[jj] == GT_FALSE)
            {
                /* this instance of the multi instances is not valid ... skip it */
                continue;
            }

            /*calculate adress inside the unit*/
            localOffset = firstValidAddress - tables2ParamsPtr[ii].commonInfo.multiInstanceInfo.multiUnitsBaseAddressPtr[0];

            SET_MULTI_INSTANCE_ADDRESS_MAC(localOffset,
                    tables2ParamsPtr[ii].commonInfo.multiInstanceInfo.multiUnitsBaseAddressPtr[jj]);

            smemRegGet(devObjPtr,
                       localOffset,
                       &data);
        }
    }

    /* read from the registers , if not exists it will cause fatal error */
    for(ii = 0 ;ii < numTables3Params ;ii++)
    {
        if(tables3ParamsPtr[ii].commonInfo.baseAddress == 0)
        {
            /* not supported table in the device */
            continue;
        }

        baseAddress = tables3ParamsPtr[ii].commonInfo.baseAddress;

        firstValidAddress = tables3ParamsPtr[ii].commonInfo.firstValidAddress ?
            tablesPtr[ii].commonInfo.firstValidAddress :
            baseAddress ;

        smemRegGet(devObjPtr,
                   firstValidAddress,
                   &data);

        /* support multi instance tables (duplications between units) */
        for(jj = 0 ; jj < tables3ParamsPtr[ii].commonInfo.multiInstanceInfo.numBaseAddresses ; jj++)
        {
            if(tables3ParamsPtr[ii].commonInfo.multiInstanceInfo.multiUnitsBaseAddressValidPtr &&
               tables3ParamsPtr[ii].commonInfo.multiInstanceInfo.multiUnitsBaseAddressValidPtr[jj] == GT_FALSE)
            {
                /* this instance of the multi instances is not valid ... skip it */
                continue;
            }

            /*calculate adress inside the unit*/
            localOffset = firstValidAddress - tables3ParamsPtr[ii].commonInfo.multiInstanceInfo.multiUnitsBaseAddressPtr[0];

            SET_MULTI_INSTANCE_ADDRESS_MAC(localOffset,
                    tables3ParamsPtr[ii].commonInfo.multiInstanceInfo.multiUnitsBaseAddressPtr[jj]);

            smemRegGet(devObjPtr,
                       localOffset,
                       &data);
        }
    }

    /* read from the registers , if not exists it will cause fatal error */
    for(ii = 0 ;ii < numTables4Params ;ii++)
    {
        if(tables4ParamsPtr[ii].commonInfo.baseAddress == 0)
        {
            /* not supported table in the device */
            continue;
        }

        baseAddress = tables4ParamsPtr[ii].commonInfo.baseAddress;

        firstValidAddress = tables4ParamsPtr[ii].commonInfo.firstValidAddress ?
            tablesPtr[ii].commonInfo.firstValidAddress :
            baseAddress ;

        smemRegGet(devObjPtr,
                   firstValidAddress,
                   &data);

        /* support multi instance tables (duplications between units) */
        for(jj = 0 ; jj < tables4ParamsPtr[ii].commonInfo.multiInstanceInfo.numBaseAddresses ; jj++)
        {
         if(tables4ParamsPtr[ii].commonInfo.multiInstanceInfo.multiUnitsBaseAddressValidPtr &&
               tables4ParamsPtr[ii].commonInfo.multiInstanceInfo.multiUnitsBaseAddressValidPtr[jj] == GT_FALSE)
            {
                /* this instance of the multi instances is not valid ... skip it */
                continue;
            }

            /*calculate adress inside the unit*/
            localOffset = firstValidAddress - tables4ParamsPtr[ii].commonInfo.multiInstanceInfo.multiUnitsBaseAddressPtr[0];

            SET_MULTI_INSTANCE_ADDRESS_MAC(localOffset,
                    tables4ParamsPtr[ii].commonInfo.multiInstanceInfo.multiUnitsBaseAddressPtr[jj]);

            smemRegGet(devObjPtr,
                       localOffset,
                       &data);
        }
    }


    return 0;
}



/**
* @internal smemTestDevice function
* @endinternal
*
* @brief   test utility -- function to test memory of device
*
* @param[in] devNum                   - the device index
* @param[in] registersAddrPtr         - array of addresses of registers
* @param[in] numRegisters             - number of registers
* @param[in] tablesInfoPtr            - array of tables info
* @param[in] numTables                - number of tables
* @param[in] testActiveMemory         - to test (or not) the addresses of the active memory to
*                                      see if exists
*/
void   smemTestDevice
(
    IN GT_U32               devNum,
    IN GT_U32               *registersAddrPtr,
    IN GT_U32               numRegisters,
    IN DEV_TABLE_INFO_STC   *tablesInfoPtr,
    IN GT_U32               numTables,
    IN GT_BOOL              testActiveMemory
)
{
    GT_U32 *memAddrPtr;
    GT_U32  ii,jj,kk,data,iiMax,address, *dataPtr;
    DEV_TABLE_INFO_STC *tablePtr;
    GT_U32  tableEntry[MAX_ENTRY_SIZE_IN_WORDS_CNS];/*enough space for 'Table entry' */
    GT_U32  numWordsInTableEntry;
    SKERNEL_DEVICE_OBJECT * devObjPtr = smemTestDeviceIdToDevPtrConvert(devNum);

    if(testActiveMemory == GT_TRUE)
    {
        /* check the active memory table */
        activeMemoryTestValidity(devNum);
    }

    /*check the generic table base addresses */
    genericTablesTestValidity(devNum);

    /**********************/
    /* checking registers */
    /**********************/

    iiMax = numRegisters;
    memAddrPtr = registersAddrPtr;


    /* fill the registers with data */
    for(ii = 0 ; ii < iiMax; ii++,memAddrPtr++)
    {
        /* write to each register it's address as value */
        data = *memAddrPtr;
        address = *memAddrPtr;

        smemMemSet(devObjPtr, address, &data, 1);
    }

    memAddrPtr = registersAddrPtr;
    /* check the values in those registers */
    for(ii = 0 ; ii < iiMax; ii++, memAddrPtr++)
    {
        /* write to each register it's address as value */
        address = *memAddrPtr;

        dataPtr = smemMemGet(devObjPtr, address);
        data = *dataPtr;
        if(data != (*memAddrPtr))
        {
            simForcePrintf("reg : address [0x%8.8x] hold data [0x%8.8x] instead of [0x%8.8x] \n",
            address,data,(*memAddrPtr));
        }
    }

    /********************/
    /* end of registers */
    /********************/

    /*******************/
    /* checking tables */
    /*******************/

    iiMax = numTables;
    tablePtr = tablesInfoPtr;
    /* fill the memories with data */
    for(ii = 0 ; ii < iiMax; ii++,tablePtr++)
    {
        numWordsInTableEntry = (tablePtr->entrySize + 31) / 32;

        if(numWordsInTableEntry >= MAX_ENTRY_SIZE_IN_WORDS_CNS)
        {
            skernelFatalError(" smemTestDevice: entry size [%d] over limit [%d]\n",
                numWordsInTableEntry,
                MAX_ENTRY_SIZE_IN_WORDS_CNS);
        }

        /* loop on all entries of the memory */
        for(kk = 0 ; kk < tablePtr->numOfEntries ; kk++)
        {
            /*calculate the base address of the current memory entry */
            address = tablePtr->baseAddr + (kk * tablePtr->lineAddrAlign * 4);

            /* write to each table word it's address as value */
            for(jj = 0; jj < numWordsInTableEntry; jj++,address+=4)
            {
                tableEntry[jj] = address;
                smemMemSet(devObjPtr, address, &tableEntry[jj], 1);
            }
        }
    }



    tablePtr = tablesInfoPtr;
    /* check the data in the memories */
    for(ii = 0 ; ii < iiMax; ii++,tablePtr++)
    {
        numWordsInTableEntry = (tablePtr->entrySize + 31) / 32;

        /* loop on all entries of the memory */
        for(kk = 0 ; kk < tablePtr->numOfEntries ; kk++)
        {
            /*calculate the base address of the current memory entry */
            address = tablePtr->baseAddr + (kk * tablePtr->lineAddrAlign * 4);

            /* write to each table word it's address as value */
            for(jj = 0; jj < numWordsInTableEntry; jj++,address+=4)
            {
                dataPtr = smemMemGet(devObjPtr, address);
                tableEntry[jj] = *dataPtr;

                if(tableEntry[jj] != address)
                {
                    simForcePrintf("tbl[%s] : address [0x%8.8x] hold data [0x%8.8x] instead of [0x%8.8x] line[%d] word[%d] (table index[%d]) \n",
                    tablePtr->tableName,address,tableEntry[jj],address,kk,jj,ii);
                }
            }
        }
    }
}

/**
* @internal checkIndexOutOfRange function
* @endinternal
*
* @brief   function check if specific index of dimension of table is out of range.
*         if check 'fails' then cause FATAL_ERROR !
* @param[in] tablePtr                 - pointer to table info
* @param[in] indexInTablePtr_paramInfo - index in tablePtr->paramInfo[]
* @param[in] indexToCheck             - index to check
*                                       NONE.
*/
static void checkIndexOutOfRange(
    IN  SKERNEL_TABLE_2_PARAMETERS_INFO_STC    *tablePtr,
    IN  GT_U32                    indexInTablePtr_paramInfo,
    IN  GT_U32                    indexToCheck
)
{
    GT_U32  outOfRangeIndex = tablePtr->paramInfo[indexInTablePtr_paramInfo].outOfRangeIndex;

    if (outOfRangeIndex == 0)
    {
        /* this 'dimension' of the table not hold validation value ! */
        return;
    }

    if (indexToCheck >= outOfRangeIndex)
    {
        skernelFatalError("checkIndexOutOfRange: table[%s] : indexToCheck[%d] out of range[%d] paramIndex[%d]\n",
            tablePtr->commonInfo.nameString,indexToCheck,outOfRangeIndex,indexInTablePtr_paramInfo);
    }
}

static void smemConvertTablePtrAndAddrToNewDevAndAddr(
    IN SKERNEL_TABLE_INFO_STC    *tablePtr,
    IN GT_U32                    tableAddr,
    OUT GT_U32                   *newTableAddrPtr
)
{
    SKERNEL_DEVICE_OBJECT  * devObjPtr;
    GT_U32  ii;


    for(ii = 0;ii < SMAIN_MAX_NUM_OF_DEVICES_CNS; ii++)
    {
        devObjPtr = smainDeviceObjects[ii];
        if(devObjPtr == NULL)
        {
            continue;
        }

        if((GT_UINTPTR)(&devObjPtr->tablesInfo.placeHolderFor1Parameter) < (GT_UINTPTR)tablePtr &&
           (GT_UINTPTR)(&devObjPtr->tablesInfo.placeHolder_MUST_BE_LAST) > (GT_UINTPTR)tablePtr)
        {
            /* found match */
            break;
        }
    }

    if(ii == SMAIN_MAX_NUM_OF_DEVICES_CNS)
    {
        skernelFatalError("smemConvertTablePtrAndAddrToNewDevAndAddr: not found the relevant device \n");
    }


    smemConvertDevAndAddrToNewDevAndAddr(devObjPtr,tableAddr,SKERNEL_MEMORY_READ_E,
        &devObjPtr,newTableAddrPtr);
}


/**
* @internal smemTableEntryAddressGet function
* @endinternal
*
* @brief   function return the address of the entry in the table.
*         NOTE: no checking on the out of range violation
* @param[in] tablePtr                 - pointer to table info
* @param[in] index                    -  of the entry in the table
* @param[in] instanceIndex            - index of instance. for multi-instance support.
*                                      ignored when == SMAIN_NOT_VALID_CNS
*                                       the address of the specified entry in the specified table
*/
GT_U32  smemTableEntryAddressGet(
    IN  SKERNEL_TABLE_INFO_STC    *tablePtr,
    IN  GT_U32                    index,
    IN  GT_U32                    instanceIndex
)
{
    GT_U32  divider;
    GT_U32  baseAddress;
    GT_U32  localOffset;/*Offset inside the unit*/

    divider = tablePtr->paramInfo[0].divider ?
              tablePtr->paramInfo[0].divider : 1;

    if(tablePtr->paramInfo[0].modulo)
    {
        index %= tablePtr->paramInfo[0].modulo;
    }

    baseAddress = tablePtr->commonInfo.baseAddress;
    if(instanceIndex != SMAIN_NOT_VALID_CNS && tablePtr->commonInfo.multiInstanceInfo.numBaseAddresses)
    {
        if(instanceIndex >= tablePtr->commonInfo.multiInstanceInfo.numBaseAddresses)
        {
            skernelFatalError("smemTableEntryAddressGet: table[%s] : instanceIndex[%d] out of range[%d]\n",
                tablePtr->commonInfo.nameString);
        }


        if(tablePtr->commonInfo.multiInstanceInfo.multiUnitsBaseAddressValidPtr &&
           tablePtr->commonInfo.multiInstanceInfo.multiUnitsBaseAddressValidPtr[instanceIndex] == GT_FALSE)
        {
            /* this instance of the multi instances is not valid ... Error */
            skernelFatalError("smemTableEntryAddressGet: table[%s] : instanceIndex[%d] not valid\n",
                tablePtr->commonInfo.nameString,instanceIndex);
        }

        /*calculate adress inside the unit*/
        localOffset = baseAddress - tablePtr->commonInfo.multiInstanceInfo.multiUnitsBaseAddressPtr[0];

        SET_MULTI_INSTANCE_ADDRESS_MAC(localOffset,
                tablePtr->commonInfo.multiInstanceInfo.multiUnitsBaseAddressPtr[instanceIndex]);

        baseAddress = localOffset;


    }

    if(baseAddress == 0)
    {
        skernelFatalError("smemTableEntryAddressGet: table[%s] : hold no valid baseAddress \n",
            tablePtr->commonInfo.nameString);
    }

    checkIndexOutOfRange(tablePtr,0/*index in tablePtr->paramInfo[]*/,index);

    smemConvertTablePtrAndAddrToNewDevAndAddr(tablePtr,baseAddress,&baseAddress);

    return baseAddress +
           tablePtr->paramInfo[0].step * (index / divider);
}

/**
* @internal smemTableEntry2ParamsAddressGet function
* @endinternal
*
* @brief   function return the address of the entry in the table.
*         NOTE: no checking on the out of range violation
* @param[in] tablePtr                 - pointer to table info
* @param[in] index1                   -  of the entry in the table
* @param[in] index2                   -  of the entry in the table
* @param[in] instanceIndex            - index of instance. for multi-instance support.
*                                      ignored when == SMAIN_NOT_VALID_CNS
*                                       the address of the specified entry in the specified table
*/
GT_U32  smemTableEntry2ParamsAddressGet(
    IN  SKERNEL_TABLE_2_PARAMETERS_INFO_STC    *tablePtr,
    IN  GT_U32                    index1,
    IN  GT_U32                    index2,
    IN  GT_U32                    instanceIndex
)
{
    GT_U32  divider1,divider2;
    GT_U32  baseAddress;
    GT_U32  localOffset;/*Offset inside the unit*/

    divider1 = tablePtr->paramInfo[0].divider ? tablePtr->paramInfo[0].divider : 1;
    divider2 = tablePtr->paramInfo[1].divider ? tablePtr->paramInfo[1].divider : 1;

    if(tablePtr->paramInfo[0].modulo)
    {
        index1 %= tablePtr->paramInfo[0].modulo;
    }

    if(tablePtr->paramInfo[1].modulo)
    {
        index2 %= tablePtr->paramInfo[1].modulo;
    }

    baseAddress = tablePtr->commonInfo.baseAddress;
    if(instanceIndex != SMAIN_NOT_VALID_CNS && tablePtr->commonInfo.multiInstanceInfo.numBaseAddresses)
    {
        if(instanceIndex >= tablePtr->commonInfo.multiInstanceInfo.numBaseAddresses)
        {
            skernelFatalError("smemTableEntry2ParamsAddressGet: instanceIndex[%d] out of range[%d]\n",
                instanceIndex,tablePtr->commonInfo.multiInstanceInfo.numBaseAddresses);
        }

        /*calculate adress inside the unit*/
        localOffset = baseAddress - tablePtr->commonInfo.multiInstanceInfo.multiUnitsBaseAddressPtr[0];

        SET_MULTI_INSTANCE_ADDRESS_MAC(localOffset,
                tablePtr->commonInfo.multiInstanceInfo.multiUnitsBaseAddressPtr[instanceIndex]);

        baseAddress = localOffset;

    }

    if(baseAddress == 0)
    {
        skernelFatalError("smemTableEntry2ParamsAddressGet: table[%s] : hold no valid baseAddress \n",
            tablePtr->commonInfo.nameString);
    }

    checkIndexOutOfRange(tablePtr,0/*index in tablePtr->paramInfo[]*/,index1);
    checkIndexOutOfRange(tablePtr,1/*index in tablePtr->paramInfo[]*/,index2);

    smemConvertTablePtrAndAddrToNewDevAndAddr(tablePtr,baseAddress,&baseAddress);

    return baseAddress +
           tablePtr->paramInfo[0].step * (index1 / divider1) +
           tablePtr->paramInfo[1].step * (index2 / divider2) ;
}


/**
* @internal smemTableEntry3ParamsAddressGet function
* @endinternal
*
* @brief   function return the address of the entry in the table.
*         NOTE: no checking on the out of range violation
* @param[in] tablePtr                 - pointer to table info
* @param[in] index1                   -  of the entry in the table
* @param[in] index2                   -  of the entry in the table
* @param[in] index3                   -  of the entry in the table
* @param[in] instanceIndex            - index of instance. for multi-instance support.
*                                      ignored when == SMAIN_NOT_VALID_CNS
*                                       the address of the specified entry in the specified table
*/
GT_U32  smemTableEntry3ParamsAddressGet(
    IN  SKERNEL_TABLE_3_PARAMETERS_INFO_STC    *tablePtr,
    IN  GT_U32                    index1,
    IN  GT_U32                    index2,
    IN  GT_U32                    index3,
    IN  GT_U32                    instanceIndex
)
{
    GT_U32  divider[3];
    GT_U32  ii;
    GT_U32  baseAddress;
    GT_U32  localOffset;/*Offset inside the unit*/

    for(ii = 0 ; ii < 3 ; ii++)
    {
        divider[ii] = tablePtr->paramInfo[ii].divider ?
                      tablePtr->paramInfo[ii].divider : 1;
    }

    ii = 0;
    if(tablePtr->paramInfo[ii].modulo)
    {
        index1 %= tablePtr->paramInfo[ii].modulo;
    }
    ii ++;
    if(tablePtr->paramInfo[ii].modulo)
    {
        index2 %= tablePtr->paramInfo[ii].modulo;
    }
    ii ++;
    if(tablePtr->paramInfo[ii].modulo)
    {
        index3 %= tablePtr->paramInfo[ii].modulo;
    }

    baseAddress = tablePtr->commonInfo.baseAddress;
    if(instanceIndex != SMAIN_NOT_VALID_CNS && tablePtr->commonInfo.multiInstanceInfo.numBaseAddresses)
    {
        if(instanceIndex >= tablePtr->commonInfo.multiInstanceInfo.numBaseAddresses)
        {
            skernelFatalError("smemTableEntry3ParamsAddressGet: instanceIndex[%d] out of range[%d]\n",
                instanceIndex,tablePtr->commonInfo.multiInstanceInfo.numBaseAddresses);
        }

        /*calculate adress inside the unit*/
        localOffset = baseAddress - tablePtr->commonInfo.multiInstanceInfo.multiUnitsBaseAddressPtr[0];

        SET_MULTI_INSTANCE_ADDRESS_MAC(localOffset,
                tablePtr->commonInfo.multiInstanceInfo.multiUnitsBaseAddressPtr[instanceIndex]);

        baseAddress = localOffset;
    }

    if(baseAddress == 0)
    {
        skernelFatalError("smemTableEntry3ParamsAddressGet: table[%s] : hold no valid baseAddress \n",
            tablePtr->commonInfo.nameString);
    }

    checkIndexOutOfRange(tablePtr,0/*index in tablePtr->paramInfo[]*/,index1);
    checkIndexOutOfRange(tablePtr,1/*index in tablePtr->paramInfo[]*/,index2);
    checkIndexOutOfRange(tablePtr,2/*index in tablePtr->paramInfo[]*/,index3);

    smemConvertTablePtrAndAddrToNewDevAndAddr(tablePtr,baseAddress,&baseAddress);

    return baseAddress
           + tablePtr->paramInfo[0].step * (index1 / divider[0])
           + tablePtr->paramInfo[1].step * (index2 / divider[1])
           + tablePtr->paramInfo[2].step * (index3 / divider[2])
           ;
}

/**
* @internal smemTableEntry4ParamsAddressGet function
* @endinternal
*
* @brief   function return the address of the entry in the table.
*         NOTE: no checking on the out of range violation
* @param[in] tablePtr                 - pointer to table info
* @param[in] index1                   -  of the entry in the table
* @param[in] index2                   -  of the entry in the table
* @param[in] index3                   -  of the entry in the table
* @param[in] index4                   -  of the entry in the table
* @param[in] instanceIndex            - index of instance. for multi-instance support.
*                                      ignored when == SMAIN_NOT_VALID_CNS
*                                       the address of the specified entry in the specified table
*/
GT_U32  smemTableEntry4ParamsAddressGet(
    IN  SKERNEL_TABLE_4_PARAMETERS_INFO_STC    *tablePtr,
    IN  GT_U32                    index1,
    IN  GT_U32                    index2,
    IN  GT_U32                    index3,
    IN  GT_U32                    index4,
    IN  GT_U32                    instanceIndex
)
{
    GT_U32  divider[4];
    GT_U32  ii;
    GT_U32  baseAddress;
    GT_U32  localOffset;/*Offset inside the unit*/

    for(ii = 0 ; ii < 4 ; ii++)
    {
        divider[ii] = tablePtr->paramInfo[ii].divider ?
                      tablePtr->paramInfo[ii].divider : 1;
    }

    ii = 0;
    if(tablePtr->paramInfo[ii].modulo)
    {
        index1 %= tablePtr->paramInfo[ii].modulo;
    }
    ii ++;
    if(tablePtr->paramInfo[ii].modulo)
    {
        index2 %= tablePtr->paramInfo[ii].modulo;
    }
    ii ++;
    if(tablePtr->paramInfo[ii].modulo)
    {
        index3 %= tablePtr->paramInfo[ii].modulo;
    }
    ii ++;
    if(tablePtr->paramInfo[ii].modulo)
    {
        index4 %= tablePtr->paramInfo[ii].modulo;
    }

    baseAddress = tablePtr->commonInfo.baseAddress;
    if(instanceIndex != SMAIN_NOT_VALID_CNS && tablePtr->commonInfo.multiInstanceInfo.numBaseAddresses)
    {
        if(instanceIndex >= tablePtr->commonInfo.multiInstanceInfo.numBaseAddresses)
        {
            skernelFatalError("smemTableEntry4ParamsAddressGet: instanceIndex[%d] out of range[%d]\n",
                instanceIndex,tablePtr->commonInfo.multiInstanceInfo.numBaseAddresses);
        }

        /*calculate adress inside the unit*/
        localOffset = baseAddress - tablePtr->commonInfo.multiInstanceInfo.multiUnitsBaseAddressPtr[0];

        SET_MULTI_INSTANCE_ADDRESS_MAC(localOffset,
                tablePtr->commonInfo.multiInstanceInfo.multiUnitsBaseAddressPtr[instanceIndex]);

        baseAddress = localOffset;

    }

    if(baseAddress == 0)
    {
        skernelFatalError("smemTableEntry4ParamsAddressGet: table[%s] : hold no valid baseAddress \n",
            tablePtr->commonInfo.nameString);
    }

    checkIndexOutOfRange(tablePtr,0/*index in tablePtr->paramInfo[]*/,index1);
    checkIndexOutOfRange(tablePtr,1/*index in tablePtr->paramInfo[]*/,index2);
    checkIndexOutOfRange(tablePtr,2/*index in tablePtr->paramInfo[]*/,index3);
    checkIndexOutOfRange(tablePtr,3/*index in tablePtr->paramInfo[]*/,index4);

    smemConvertTablePtrAndAddrToNewDevAndAddr(tablePtr,baseAddress,&baseAddress);

    return baseAddress
           + tablePtr->paramInfo[0].step * (index1 / divider[0])
           + tablePtr->paramInfo[1].step * (index2 / divider[1])
           + tablePtr->paramInfo[2].step * (index3 / divider[2])
           + tablePtr->paramInfo[3].step * (index4 / divider[3])
           ;
}

/**
* @internal smemChunkInternalWriteDataCheck function
* @endinternal
*
* @brief   Controls internal buffer memory write
*
* @param[in] devObjPtr                - pointer to device pbject
* @param[in] address                  - memory chunk  to be controlled
* @param[in] chunkPtr                 - pointer to chunksMemArr[x] , that is the chosen chunk.
* @param[in,out] memoryIndexPtr           - the index in chunksMemArr[x]->memPtr[y],
*                                      that represent the 'address' valid only when the chunk was found
*                                      OUTPUT:
* @param[in,out] memoryIndexPtr           - the index in local buffer when writing to non last word of entry, or
*                                      original index of the found chunk when writing to last word of entry
*                                       None
*
* @note Returns local buffer while writing to non last word of entry.
*       On last word writing it copies start words from local buffer into
*       real memory and returns last word of real memory.
*
*/
static GT_VOID smemChunkInternalWriteDataCheck
(
    IN SKERNEL_DEVICE_OBJECT    *devObjPtr,
    IN GT_U32                   address,
    IN SMEM_CHUNK_STC           *chunkPtr,
    INOUT GT_U32                *memoryIndexPtr
)
{
    GT_U32 currWordOffset;  /* entry current word offset */
    GT_U32 internalDataOffset; /* internal data entry offset */

    /* check if this memory is "Direct memory write by last word" */
    if (chunkPtr->lastEntryWordIndex == 0)
    {
        return;
    }

    /* Current word offset calculation in words */
    currWordOffset = (address >> 2) % chunkPtr->enrtyNumWordsAlignement;
    /* Get offset of internal entry buffer */
    internalDataOffset = (chunkPtr->memLastAddr - chunkPtr->memFirstAddr + 4) / 4;

    if (currWordOffset == chunkPtr->lastEntryWordIndex)
    {
        if(chunkPtr->forbidDirectWriteToChunk == GT_FALSE)
        {
            /* Copy data from the internal buffer to memory */
            memcpy(&chunkPtr->memPtr[*memoryIndexPtr - currWordOffset],
                   &chunkPtr->memPtr[internalDataOffset],
                   chunkPtr->enrtyNumWordsAlignement * sizeof(SMEM_REGISTER));
        }
        else
        {
            /* we are forbidden from write the data to the entry 'as is'
               and there is expected 'active memory' to catch this address
               to implement the actual write ... look for functions that uses :
               smemTableInternalDataPtrGet(...) */
        }
    }
    else
    {
        /* The word is not last one. Return index to internal memory */
        *memoryIndexPtr = internalDataOffset + currWordOffset;
    }
}

/**
* @internal smemUnitChunksReset function
* @endinternal
*
* @brief   Reset memory chunks by zeroes for Cheetah3 and above devices.
*
* @param[in] devObjPtr                - pointer to device object
*/
void smemUnitChunksReset
(
    IN SKERNEL_DEVICE_OBJECT  * devObjPtr
)
{
    GT_U32 unitsCount;          /* Units count */
    GT_U32 curUnit;
    GT_U32 curChunk;
    SMEM_CHUNK_STC * chunkPtr;
    SMEM_UNIT_CHUNKS_STC  * memPtr;
    GT_U32 byteCount;

    SMEM_CHT_DEV_COMMON_MEM_INFO  * commonMemPtr =
        (SMEM_CHT_DEV_COMMON_MEM_INFO  *)devObjPtr->deviceMemory;

    memPtr = (SMEM_UNIT_CHUNKS_STC *)commonMemPtr->unitChunksBasePtr;

    byteCount = commonMemPtr->unitChunksSizeOf;

    /* Number of allocated structures to be reset */
    unitsCount = byteCount / sizeof(SMEM_UNIT_CHUNKS_STC);
    for(curUnit = 0; curUnit < unitsCount; curUnit++)
    {
        for(curChunk = 0; curChunk < memPtr[curUnit].numOfChunks; curChunk++)
        {
            chunkPtr = &memPtr[curUnit].chunksArray[curChunk];
            memset(chunkPtr->memPtr, 0, chunkPtr->memSize * sizeof(SMEM_REGISTER));
        }
    }
}

/*******************************************************************************
*   smemDevMemoryCalloc
*
* DESCRIPTION:
*       Allocate device memory and store reference to allocations into internal array.
*
* INPUTS:
*       devObjPtr       - pointer to device object.
*       sizeObjCount    - count of objects to be allocated
*       sizeObjSizeof   - size of object to be allocated
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       Pointer to allocated memory
*
* COMMENTS:
*       The function calls standard calloc function.
*       Internal data base will hold reference for all allocations
*       for future use in SW reset
*
*******************************************************************************/
void * smemDevMemoryCalloc
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32 sizeObjCount,
    IN GT_U32 sizeObjSizeof
)
{
    GT_U32 i;
    SMEM_CALLOC_STC * callocMemPtr;

    SMEM_CHT_DEV_COMMON_MEM_INFO  * commonMemPtr =
        (SMEM_CHT_DEV_COMMON_MEM_INFO  *)devObjPtr->deviceMemory;

    if (commonMemPtr->callocMemSize < SMEM_MAX_CALLOC_MEM_SIZE)
    {
        i = commonMemPtr->callocMemSize++;
        callocMemPtr = &commonMemPtr->callocMemArray[i];
        callocMemPtr->regPtr = smemDeviceObjMemoryAlloc(devObjPtr,sizeObjCount, sizeObjSizeof);
        callocMemPtr->regNum = sizeObjCount / 4;

        return callocMemPtr->regPtr;
    }
    else
    {
        skernelFatalError("smemChtCalloc: number of allocations exceeds maximum allowed\n");
    }

    return NULL;
}

/*******************************************************************************
*   smemDevFindInUnitChunk
*
* DESCRIPTION:
*       find the memory in the specified unit chunk
*
* INPUTS:
*       deviceObj   - pointer to device object.
*       unitChunksPtr - pointer to the unit chunk
*       address     - address of memory(register or table).
*       memSize     - size of the requested memory
*       param       - extra parameter --> used as pointer to the memory unit chunk
*
* OUTPUTS:
*       None.
*
* RETURNS:
*        pointer to the memory location
*        NULL - if memory not exist
*
* COMMENTS:
*
*
*******************************************************************************/
GT_U32 *  smemDevFindInUnitChunk
(
    IN SKERNEL_DEVICE_OBJECT *devObjPtr,
    IN SCIB_MEMORY_ACCESS_TYPE accessType,
    IN GT_U32 address,
    IN GT_U32 memSize,
    IN GT_UINTPTR param
)
{
    GT_U32                  chunkIndex;/*index of the chunk that hold the memory */
    GT_U32                  memIndex;  /* index in the memory that represent the address*/
    SMEM_UNIT_CHUNKS_STC    *unitChunksPtr;

    /* convert the param to be the pointer to the unit chunk */
    unitChunksPtr = (SMEM_UNIT_CHUNKS_STC*)(GT_U32*)param;

    if(unitChunksPtr->hugeUnitSupportPtr)
    {
        /* support for huge units */
        unitChunksPtr = unitChunksPtr->hugeUnitSupportPtr;
    }

    if(GT_FALSE == smemFindMemChunk(devObjPtr, accessType, unitChunksPtr, address,
                                    &chunkIndex, &memIndex))
    {
        /* memory was not found */
        return NULL;
    }

    return &unitChunksPtr->chunksArray[chunkIndex].memPtr[memIndex];
}


/*******************************************************************************
*   smemTableInternalDataPtrGet
*
* DESCRIPTION:
*       find the pointer to internal data of a table
*
* INPUTS:
*       deviceObj   - pointer to device object.
*       address     - address in memory of table.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*        pointer to the memory of the internal data
*        NULL - if memory not exist
*
* COMMENTS:
*
*
*******************************************************************************/
GT_U32 *  smemTableInternalDataPtrGet
(
    IN SKERNEL_DEVICE_OBJECT *devObjPtr,
    IN GT_U32 address
)
{
    GT_U32                  chunkIndex;/*index of the chunk that hold the memory */
    GT_U32                  memIndex;  /* index in the memory that represent the address*/
    SMEM_UNIT_CHUNKS_STC    *unitChunksPtr;
    GT_UINTPTR param;
    GT_U32                  index,internalDataOffset;
    SMEM_CHT_GENERIC_DEV_MEM_INFO *devMemInfoPtr = devObjPtr->deviceMemory;
    SMEM_CHUNK_STC           *chunkPtr;

    if(devObjPtr->support_memoryRanges)
    {
        index = UNIT_INDEX_FROM_ADDR_GET_MAC(devObjPtr,address);
    }
    else
    if(SMEM_CHT_IS_SIP5_GET(devObjPtr))
    {
        /* support 8 * 64 = 512 sub units !!! */
        /* not only 64 sub units as in previous device */
        index = address >> SMEM_CHT_UNIT_INDEX_FIRST_BIT_CNS;
    }
    else if(SKERNEL_IS_AC5_BASED_DEV(devObjPtr))
    {
        /* like sip5 : remove the limit of 6 bits unit address */
        /* as the AC5 uses CNM like in Phoenix                 */
        index = address >> SMEM_CHT_UNIT_INDEX_FIRST_BIT_CNS;
    }
    else
    {
        index = (address & REG_SPEC_FUNC_INDEX) >>
                     SMEM_CHT_UNIT_INDEX_FIRST_BIT_CNS;
    }

    if(index >= SMEM_CHT_NUM_UNITS_MAX_CNS)
    {
        if(index == SMAIN_NOT_VALID_CNS)
        {
            /* smemFindMemory will handle it */
            return NULL;
        }
        skernelFatalError("smemGenericFindMem : unitIndex[%d] >= max of [%d] \n" ,
            index,SMEM_CHT_NUM_UNITS_MAX_CNS);
    }


    param   = devMemInfoPtr->common.specFunTbl[index].specParam;

    /* convert the param to be the pointer to the unit chunk */
    unitChunksPtr = (SMEM_UNIT_CHUNKS_STC*)(GT_U32*)param;

    if(unitChunksPtr->hugeUnitSupportPtr)
    {
        /* support for huge units */
        unitChunksPtr = unitChunksPtr->hugeUnitSupportPtr;
    }

    if(GT_FALSE == smemFindMemChunk(devObjPtr, SCIB_MEMORY_WRITE_E, unitChunksPtr, address,
                                    &chunkIndex, &memIndex))
    {
        /* memory was not found */
        return NULL;
    }
    chunkPtr = &unitChunksPtr->chunksArray[chunkIndex];
    /* Get offset of internal entry buffer */
    internalDataOffset = (chunkPtr->memLastAddr - chunkPtr->memFirstAddr + 4) / 4;

    return &chunkPtr->memPtr[internalDataOffset];
}


/*******************************************************************************
*   smemFindCommonMemory
*
* DESCRIPTION:
*       Return pointer to the register's common memory.
*
* INPUTS:
*       devObjPtr   - pointer to device object.
*       address     - address of memory(register or table).
*       memSize     - size of the requested memory
*
* OUTPUTS:
*       activeMemPtrPtr - (pointer to) the active memory entry or NULL if not
*                         exist.
*                         (if activeMemPtrPtr == NULL ignored)
*
* RETURNS:
*        pointer to the memory location
*        NULL - if memory not exist
*
* COMMENTS:
*
*******************************************************************************/
void * smemFindCommonMemory
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32                  address,
    IN GT_U32                  memSize,
    IN SCIB_MEMORY_ACCESS_TYPE accessType,
    OUT struct SMEM_ACTIVE_MEM_ENTRY_T **   activeMemPtrPtr
)
{
    SMEM_CHT_DEV_COMMON_MEM_INFO* deviceCommonMemPtr;
    void * findMemPtr = NULL;
    SMEM_UNIT_CHUNKS_STC * currentChunkMemPtr;
    GT_U32  ii;/*iterator*/
    SMEM_ACTIVE_MEM_ENTRY_STC   *unitActiveMemPtr = NULL;/*active memory dedicated to the unit*/
    SMEM_ACTIVE_MEM_ENTRY_STC   *activeMemPtr = NULL;/*active memory of the device/unit*/

    if (activeMemPtrPtr != NULL)
    {
        *activeMemPtrPtr = NULL;
    }

    deviceCommonMemPtr = (SMEM_CHT_DEV_COMMON_MEM_INFO*)devObjPtr->deviceMemory;

    if (SMEM_ACCESS_PCI_FULL_MAC(accessType))
    {
        if((address & deviceCommonMemPtr->pciUnitBaseAddrMask) == 0)
        {
            /* support accessing the memory also ‘0’ based */
            address |= deviceCommonMemPtr->pciExtMemArr[SMEM_UNIT_PCI_BUS_MBUS_E].unitBaseAddr;
        }
        currentChunkMemPtr =
            &deviceCommonMemPtr->pciExtMemArr[SMEM_UNIT_PCI_BUS_MBUS_E].unitMem;
        /* check if there is dedicated active memory for the unit */
        unitActiveMemPtr =
            currentChunkMemPtr->unitActiveMemPtr;

        activeMemPtr =
            unitActiveMemPtr;

        findMemPtr = (void *)smemDevFindInUnitChunk(devObjPtr, accessType,
                                                    address, memSize,
                                                    (GT_UINTPTR)currentChunkMemPtr);
    }
    else
    if (SMEM_ACCESS_DFX_FULL_MAC(accessType) &&
        SMEM_CHT_IS_DFX_ON_UNIQUE_MEMORY_SPCAE(devObjPtr))/* indication that the device supports 'DFX memory space' */
    {

        currentChunkMemPtr =
            &deviceCommonMemPtr->pciExtMemArr[SMEM_UNIT_PCI_BUS_DFX_E].unitMem;

        /* Device is port group and accessed from SKERNEL scope */
        if (SMEM_ACCESS_SKERNEL_FULL_MAC(accessType))
        {
            /* get representative core */
            devObjPtr = currentChunkMemPtr->otherPortGroupDevObjPtr ?
                    currentChunkMemPtr->otherPortGroupDevObjPtr : devObjPtr;
        }

        /* check if there is dedicated active memory for the unit */
        unitActiveMemPtr =
            currentChunkMemPtr->unitActiveMemPtr;

        findMemPtr = (void *)smemDevFindInUnitChunk(devObjPtr, accessType,
                                                    address, memSize,
                                                    (GT_UINTPTR)currentChunkMemPtr);
        if(findMemPtr == 0)
        {
            skernelFatalError("smemFindCommonMemory : DFX server memory address not found: [0x%8.8x]: \n", address);
        }

        activeMemPtr = unitActiveMemPtr;

    }

    /* find active memory entry */
    if (activeMemPtrPtr != NULL && activeMemPtr)
    {
        *activeMemPtrPtr = NULL;

        for (ii = 0; activeMemPtr[ii].address != END_OF_TABLE; ii++)
        {
            /* check address    */
            if ((address & activeMemPtr[ii].mask)
                 == activeMemPtr[ii].address)
            {
                *activeMemPtrPtr = &activeMemPtr[ii];
                break;
            }
        }
    }

    return findMemPtr;
}

/**
* @internal smemRegUpdateAfterRegFile function
* @endinternal
*
* @brief   The function read the register and write it back , so active memory can
*         be activated on this address after loading register file.
*/
void smemRegUpdateAfterRegFile
(
    IN SKERNEL_DEVICE_OBJECT * deviceObjPtr,
    IN GT_U32                  address,
    IN GT_U32                  dataSize
)
{
    GT_U32  ii;
    GT_U32  regValue;
    GT_U32  regAddr = address;
    GT_U32  pipeId , numOfPipes;
    GT_U32  pipeOffset;
    GT_U32  pipeIdOf_address = 0;

    numOfPipes = deviceObjPtr->numOfPipes ? deviceObjPtr->numOfPipes : 1;

    if(numOfPipes >= 2)
    {
        pipeOffset = SMEM_UNIT_PIPE_OFFSET_GET_MAC(deviceObjPtr, address , &pipeIdOf_address);
        if(pipeIdOf_address != 0)
        {
            /* the caller explicitly aware to multi pipes ... no need to duplicate for it */
            numOfPipes = 1;/* make sure no wrong actions of : regAddr = address + pipeOffset; */
        }
    }

    for(pipeId = 0 ; pipeId < numOfPipes ; pipeId++)
    {
        if(pipeId > 0)
        {
            pipeOffset = SMEM_UNIT_PIPE_OFFSET_GET_MAC(deviceObjPtr, address , NULL);
            regAddr = address + pipeOffset;
        }

        if(GT_FALSE ==
            smemIsDeviceMemoryOwner(deviceObjPtr,regAddr))
        {
            /* the address not belongs to the device */
            continue;
        }

        for(ii = 0 ; ii < dataSize; ii++ , regAddr += 4)
        {
            smemRegGet(deviceObjPtr,regAddr,&regValue);
            scibWriteMemory(deviceObjPtr->deviceId,regAddr,1,&regValue);
        }
    }


    return;
}

/**
* @internal smemBindTablesToMemories function
* @endinternal
*
* @brief   bind tables from devObjPtr->tablesInfo.<tableName> to memories specified
*         in chunksPtr
*/
void smemBindTablesToMemories(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SMEM_UNIT_CHUNKS_STC     *chunksPtr,
    IN GT_U32                   numOfUnits
)
{
    GT_U32  ii;
    GT_U32  jj;
    GT_U32  tblIndex;
    SMEM_UNIT_CHUNKS_STC  *currentChunksPtr;
    SMEM_CHUNK_STC        *currentChunkPtr;
    SKERNEL_TABLE_INFO_STC  *tablesPtr;/* pointer to tables with single parameter */
    GT_U32  numTables;/* number of tables with single parameter */
    SKERNEL_TABLE_2_PARAMETERS_INFO_STC  *tables2ParamsPtr; /* pointer to tables with 2 parameters */
    GT_U32  numTables2Params;/* number of tables with 2 parameters */
    SKERNEL_TABLE_3_PARAMETERS_INFO_STC  *tables3ParamsPtr; /* pointer to tables with 3 parameters */
    GT_U32  numTables3Params;/* number of tables with 3 parameters */
    SKERNEL_TABLE_4_PARAMETERS_INFO_STC  *tables4ParamsPtr; /* pointer to tables with 4 parameters */
    GT_U32  numTables4Params;/* number of tables with 3 parameters */
    GT_U32  numBytesToStartOfTables1Param ;/* number of bytes to start of tables with 1 parameters */
    GT_U32  numBytesToStartOfTables2Params;/* number of bytes to start of tables with 2 parameters */
    GT_U32  numBytesToStartOfTables3Params;/* number of bytes to start of tables with 3 parameters */
    GT_U32  numBytesToStartOfTables4Params;/* number of bytes to start of tables with 4 parameters */
    GT_U32 pipeOffset , pipeId = 0;

    smemGenericTablesSectionsGet(devObjPtr,
                            &tablesPtr,&numTables,
                            &tables2ParamsPtr,&numTables2Params,
                            &tables3ParamsPtr,&numTables3Params,
                            &tables4ParamsPtr,&numTables4Params);

    numBytesToStartOfTables1Param =  ((GT_U8*)(void*)tablesPtr -
                                      (GT_U8*)(void*)(&devObjPtr->tablesInfo));

    numBytesToStartOfTables2Params = ((GT_U8*)(void*)tables2ParamsPtr -
                                      (GT_U8*)(void*)(&devObjPtr->tablesInfo));

    numBytesToStartOfTables3Params = ((GT_U8*)(void*)tables3ParamsPtr -
                                      (GT_U8*)(void*)(&devObjPtr->tablesInfo));

    numBytesToStartOfTables4Params = ((GT_U8*)(void*)tables4ParamsPtr -
                                      (GT_U8*)(void*)(&devObjPtr->tablesInfo));

    currentChunksPtr = &chunksPtr[0];

    for(ii = 0 , currentChunksPtr = chunksPtr; ii < numOfUnits; ii++,currentChunksPtr++)
    {
        currentChunkPtr = &currentChunksPtr->chunksArray[0];
        for(jj = 0 ; jj < currentChunksPtr->numOfChunks ; jj++,currentChunkPtr++)
        {
            if(currentChunkPtr->tableOffsetInBytes == SMAIN_NOT_VALID_CNS)
            {
                continue;
            }

            /**************************************/
            /* check tables with single parameter */
            /**************************************/

                for(tblIndex = 0 ; tblIndex < numTables;tblIndex++)
                {
                    if((currentChunkPtr->tableOffsetInBytes - numBytesToStartOfTables1Param) ==
                        (tblIndex) * sizeof(SKERNEL_TABLE_INFO_STC))
                    {
                        pipeOffset = SMEM_UNIT_PIPE_OFFSET_GET_MAC(devObjPtr, currentChunkPtr->memFirstAddr,&pipeId);
                        /* found the table */
                        if(pipeId != 0)
                        {
                            tablesPtr[tblIndex].commonInfo.baseAddress = currentChunkPtr->memFirstAddr - (pipeOffset);
                        }
                        else
                        {
                            tablesPtr[tblIndex].commonInfo.baseAddress = currentChunkPtr->memFirstAddr;
                        }
                        tablesPtr[tblIndex].paramInfo[0].step =
                            currentChunkPtr->enrtyNumWordsAlignement * 4;/* step in bytes */
                        tablesPtr[tblIndex].commonInfo.memChunkPtr = currentChunkPtr;
                        break;
                    }
                }

            if(tblIndex != numTables)
            {
                /* found table */
                continue;
            }

                /**********************************/
                /* check tables with 2 parameters */
                /**********************************/
                for(tblIndex = 0 ; tblIndex < numTables2Params;tblIndex++)
                {
                    if((currentChunkPtr->tableOffsetInBytes - numBytesToStartOfTables2Params) ==
                        (tblIndex) * sizeof(SKERNEL_TABLE_2_PARAMETERS_INFO_STC))
                    {
                        pipeOffset = SMEM_UNIT_PIPE_OFFSET_GET_MAC(devObjPtr, currentChunkPtr->memFirstAddr,&pipeId);
                        /* found the table */
                        if(pipeId != 0)
                        {
                            tables2ParamsPtr[tblIndex].commonInfo.baseAddress = currentChunkPtr->memFirstAddr - (pipeOffset);
                        }
                        else
                        {
                            tables2ParamsPtr[tblIndex].commonInfo.baseAddress = currentChunkPtr->memFirstAddr;
                        }
                        tables2ParamsPtr[tblIndex].paramInfo[0].step =
                            currentChunkPtr->enrtyNumWordsAlignement * 4;/* step in bytes */
                        tables2ParamsPtr[tblIndex].commonInfo.memChunkPtr = currentChunkPtr;
                        break;
                    }
                }

                if(tblIndex != numTables2Params)
                {
                    /* found table */
                    continue;
                }


                /**********************************/
                /* check tables with 3 parameters */
                /**********************************/
                for(tblIndex = 0 ; tblIndex < numTables3Params;tblIndex++)
                {
                    if((currentChunkPtr->tableOffsetInBytes - numBytesToStartOfTables3Params) ==
                        (tblIndex) * sizeof(SKERNEL_TABLE_3_PARAMETERS_INFO_STC))
                    {
                        pipeOffset = SMEM_UNIT_PIPE_OFFSET_GET_MAC(devObjPtr, currentChunkPtr->memFirstAddr,&pipeId);
                        /* found the table */
                        if(pipeId != 0)
                        {
                            tables3ParamsPtr[tblIndex].commonInfo.baseAddress = currentChunkPtr->memFirstAddr - (pipeOffset);
                        }
                        else
                        {
                            tables3ParamsPtr[tblIndex].commonInfo.baseAddress = currentChunkPtr->memFirstAddr;
                        }
                        tables3ParamsPtr[tblIndex].paramInfo[0].step =
                            currentChunkPtr->enrtyNumWordsAlignement * 4;/* step in bytes */
                        tables3ParamsPtr[tblIndex].commonInfo.memChunkPtr = currentChunkPtr;
                        break;
                    }
                }

                if(tblIndex != numTables3Params)
                {
                    /* found table */
                    continue;
                }

                /**********************************/
                /* check tables with 4 parameters */
                /**********************************/
                for(tblIndex = 0 ; tblIndex < numTables4Params;tblIndex++)
                {
                    if((currentChunkPtr->tableOffsetInBytes - numBytesToStartOfTables4Params) ==
                        (tblIndex) * sizeof(SKERNEL_TABLE_4_PARAMETERS_INFO_STC))
                    {
                        pipeOffset = SMEM_UNIT_PIPE_OFFSET_GET_MAC(devObjPtr, currentChunkPtr->memFirstAddr,&pipeId);
                        /* found the table */
                        if(pipeId != 0)
                        {
                            tables4ParamsPtr[tblIndex].commonInfo.baseAddress = currentChunkPtr->memFirstAddr - (pipeOffset);
                        }
                        else
                        {
                            tables4ParamsPtr[tblIndex].commonInfo.baseAddress = currentChunkPtr->memFirstAddr;
                        }
                        tables4ParamsPtr[tblIndex].paramInfo[0].step =
                            currentChunkPtr->enrtyNumWordsAlignement * 4;/* step in bytes */
                        tables4ParamsPtr[tblIndex].commonInfo.memChunkPtr = currentChunkPtr;
                        break;
                    }
                }

            if(tblIndex != numTables4Params)
            {
                /* found table */
                continue;
            }
        }/*jj*/
    }/*ii*/
}
/**
* @internal smemUnitChunkOverlapCheck function
* @endinternal
*
* @brief   Checks current chunk overlapping in unit's memory chunks
*
* @param[in] unitChunksPtr            - (pointer to) the unit memories chunks
* @param[in] unitChunkOffset          - offset of chunk in the unit
* @param[in,out] currChunkPtr             - (pointer to) current chunk to be allocated
* @param[in,out] currChunkPtr             - (pointer to) current chunk to be allocated
*                                       none
*
* @note Check base address offset for currChunkPtr->memFirstAddr and currChunkPtr->memLastAddr.
*       If zero, make unit based adress:
*       memFirstAddr += unitChunksPtr->chunkIndex << 23
*       memLastAddr += unitChunksPtr->chunkIndex << 23
*
*/
static GT_VOID smemUnitChunkOverlapCheck
(
    IN SMEM_UNIT_CHUNKS_STC * unitChunksPtr,
    IN GT_U32 unitChunkOffset,
    INOUT SMEM_CHUNK_STC * currChunkPtr
)
{
    GT_U32 i;
    GT_BOOL rc;

    for(i = 0; i < unitChunkOffset; i++)
    {
        /* Check chunk FIRST address */
        if(currChunkPtr->memFirstAddr >= unitChunksPtr->chunksArray[i].memFirstAddr &&
           currChunkPtr->memFirstAddr <= unitChunksPtr->chunksArray[i].memLastAddr)
        {
            if(unitChunksPtr->chunksArray[i].formulaFuncPtr)
            {
                rc = smemUnitChunkFormulaOverlapCheck(unitChunksPtr, i,
                                                      currChunkPtr,
                                                      currChunkPtr->memFirstAddr);
                if(rc == GT_TRUE)
                {
                    /* Don't check last address, skip to next chunk */
                    continue;
                }
            }
            else
            {
                simWarningPrintf("smemUnitChunkOverlapCheck: "
                                  "overlapped chunk range:  = [0x%8.8x] [0x%8.8x]\n"
                                  "unit chunk range: [0x%8.8x] [0x%8.8x]\n",
                                  currChunkPtr->memFirstAddr, currChunkPtr->memLastAddr,
                                  unitChunksPtr->chunksArray[i].memFirstAddr,
                                  unitChunksPtr->chunksArray[i].memLastAddr);
                /* Don't check last address, skip to next chunk */
                continue;
            }
        }

        /* Check chunk LAST address */
        if(currChunkPtr->memLastAddr >= unitChunksPtr->chunksArray[i].memFirstAddr &&
           currChunkPtr->memLastAddr <= unitChunksPtr->chunksArray[i].memLastAddr)
        {
            if(unitChunksPtr->chunksArray[i].formulaFuncPtr)
            {
                if(currChunkPtr->formulaFuncPtr)
                {
                    /* already checked all formula cells with memFirstAddr */
                    continue;
                }
                smemUnitChunkFormulaOverlapCheck(unitChunksPtr, i, currChunkPtr,
                                                 currChunkPtr->memLastAddr);
            }
            else
            {
                simWarningPrintf("smemUnitChunkOverlapCheck: "
                                  "overlapped chunk range:  = [0x%8.8x] [0x%8.8x]\n"
                                  "unit chunk range: [0x%8.8x] [0x%8.8x]\n",
                                  currChunkPtr->memFirstAddr, currChunkPtr->memLastAddr,
                                  unitChunksPtr->chunksArray[i].memFirstAddr,
                                  unitChunksPtr->chunksArray[i].memLastAddr);
            }
        }
    }
}




/**
* @internal smemUnitChunkFormulaOverlapCheck_subFormula function
* @endinternal
*
* @brief   Checks (sub) formula of specific chunk in existing unit
*
* @param[in] unitChunksPtr            - (pointer to) the unit memories chunks
* @param[in] unitChunkOffset          - offset of chunk in the unit
* @param[in] currChunkPtr             - (pointer to) current chunk to be checked
* @param[in] currAddr                 - current address that belongs to currChunkPtr
* @param[in] formulaLevel             - index in currChunkPtr->formulaData[] to be checked on
* @param[in] currAddr                 (from previous level)
*
* @retval GT_TRUE                  - chunk is overlapped and GT_FALSE - chunk is not overlapped
*
* @note Recursive function
*
*/
static GT_BOOL smemUnitChunkFormulaOverlapCheck_subFormula
(
    IN SMEM_UNIT_CHUNKS_STC * unitChunksPtr,
    IN GT_U32 unitChunkOffset,
    IN SMEM_CHUNK_STC * currChunkPtr,
    IN GT_U32 currAddr ,
    IN GT_U32 formulaLevel
)
{
    GT_U32 formulaIndexArr[SMEM_FORMULA_CELL_NUM_CNS];  /* array of memory indexes */
    GT_BOOL     rc;
    SMEM_FORMULA_DIM_STC * formulaCellPtr;              /* pointer to formula cell array */
    GT_U32  ii;
    GT_U32  newAddr;
    GT_BOOL lastLevel;

    formulaCellPtr = &currChunkPtr->formulaData[formulaLevel];

    if(formulaCellPtr->size == 0 || formulaCellPtr->step == 0)
    {
        lastLevel = GT_TRUE;
    }
    else
    if((formulaLevel + 1) == SMEM_FORMULA_CELL_NUM_CNS)
    {
        lastLevel = GT_TRUE;
    }
    else
    {
        lastLevel = GT_FALSE;
    }
#if 0
    #define __PRINT_PARAM(param) simGeneralPrintf(" %s = [0x%8.8x] \n",#param,param)
#else
    #define __PRINT_PARAM(param)
#endif

    __PRINT_PARAM(lastLevel);

    if(lastLevel == GT_TRUE)
    {
        newAddr = currAddr;

        rc = smemFormulaChunkAddressCheck(newAddr,
                                          &unitChunksPtr->chunksArray[unitChunkOffset],
                                          formulaIndexArr);
        if(rc == GT_TRUE)
        {
            simWarningPrintf("smemUnitChunkFormulaOverlapCheck_subFormula: "
                             "overlapped chunk with formula start address = [0x%8.8x], current address = [0x%8.8x]\n"
                             "unit chunk range: [0x%8.8x] [0x%8.8x]\n",
                             currChunkPtr->memFirstAddr, newAddr,
                             unitChunksPtr->chunksArray[unitChunkOffset].memFirstAddr,
                             unitChunksPtr->chunksArray[unitChunkOffset].memLastAddr);
            /*return GT_TRUE;*/
        }

        __PRINT_PARAM(rc);
        return rc;
    }


    __PRINT_PARAM(formulaCellPtr->size);
    __PRINT_PARAM(formulaCellPtr->step);


    for(ii = 0; ii < formulaCellPtr->size; ii++)
    {
        newAddr = currAddr + (ii * formulaCellPtr->step);

        __PRINT_PARAM(ii);
        __PRINT_PARAM(newAddr);

        rc = smemUnitChunkFormulaOverlapCheck_subFormula(
                        unitChunksPtr,
                        unitChunkOffset,
                        currChunkPtr,
                        newAddr,
                        (formulaLevel + 1)/*do recursion to check deeper level */ );
        if(rc == GT_TRUE)
        {
            __PRINT_PARAM(rc);
            /* the address (or it's sub formula) from current chunk already
                documented as exists in the unit ... no need to continue with this chunk */
            return GT_TRUE;
        }
    }

    return GT_FALSE;

}

/**
* @internal smemUnitChunkFormulaOverlapCheck function
* @endinternal
*
* @brief   Checks formula chunk's overlapping in unit's memory chunks
*
* @param[in] unitChunksPtr            - (pointer to) the unit memories chunks
* @param[in] unitChunkOffset          - offset of chunk in the unit
* @param[in] currChunkPtr             - (pointer to) current chunk to be allocated
* @param[in] addressOffset            - chunk address offset
*
* @retval GT_TRUE                  - chunk is overlapped and GT_FALSE - chunk is not overlapped
*
* @note Expands chunk formula into addresses and checks for overlapping
*       into the unit chunk
*
*/
static GT_BOOL smemUnitChunkFormulaOverlapCheck
(
    IN SMEM_UNIT_CHUNKS_STC * unitChunksPtr,
    IN GT_U32 unitChunkOffset,
    IN SMEM_CHUNK_STC * currChunkPtr,
    IN GT_U32 addressOffset
)
{
    GT_U32 formulaIndexArr[SMEM_FORMULA_CELL_NUM_CNS];  /* array of memory indexes */
    GT_BOOL rc = GT_FALSE;

    /* Current chunk without formula */
    if(currChunkPtr->formulaFuncPtr == 0)
    {
        rc = smemFormulaChunkAddressCheck(addressOffset,
                                          &unitChunksPtr->chunksArray[unitChunkOffset],
                                          formulaIndexArr);

        if(rc == GT_TRUE)
        {
            simWarningPrintf("smemUnitChunkFormulaOverlapCheck: "
                              "overlapped chunk with formula start address = [0x%8.8x], current address = [0x%8.8x]\n"
                              "unit chunk range: [0x%8.8x] [0x%8.8x]\n",
                              currChunkPtr->memFirstAddr, addressOffset,
                              unitChunksPtr->chunksArray[unitChunkOffset].memFirstAddr,
                              unitChunksPtr->chunksArray[unitChunkOffset].memLastAddr);
        }

        return rc;
    }


    rc = smemUnitChunkFormulaOverlapCheck_subFormula(
                    unitChunksPtr,
                    unitChunkOffset,
                    currChunkPtr,
                    addressOffset,
                    0/*start the recursion*/);

    return rc;
}

/**
* @internal smemGenericUnitBaseAddressesGet function
* @endinternal
*
* @brief   get the base address of the unit chunk
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in,out] unitInfoPtr              - pointer to the unit chunk.
*/
GT_U32  smemGenericUnitBaseAddressesGet(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SMEM_UNIT_CHUNKS_STC *unitInfoPtr
)
{
    if(devObjPtr->support_memoryRanges == 0)
    {
        return (unitInfoPtr->chunkIndex << SMEM_CHT_UNIT_INDEX_FIRST_BIT_CNS);
    }

    /* the unitInfoPtr->chunkIndex is expected to be index into devObjPtr->genericUsedUnitsAddressesArray[] */
    if(unitInfoPtr->chunkIndex >= devObjPtr->genericNumUnitsAddresses)
    {
        skernelFatalError("smemGenericUnitBaseAddressesGet: unit index [%d] is out of range [0..%d] \n" ,
            unitInfoPtr->chunkIndex ,
            devObjPtr->genericNumUnitsAddresses -1);
    }

    return devObjPtr->genericUsedUnitsAddressesArray[unitInfoPtr->chunkIndex].unitBaseAddr;
}


/**
* @internal falcon_flexRangeAddrUnitAlignToBase function
* @endinternal
*
* @brief   align all addresses of the unit according to base address of the unit
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in,out] unitInfoPtr              - pointer to the relevant unit - before alignment of addresses.
* @param[in] isZeroBased              - GT_FALSE - the same as function smemGenericUnitAddressesAlignToBaseAddress
*                                      GT_TRUE  - the addresses in unitInfoPtr are '0' based and
*                                      need to add to all of then the unit base address from unitInfoPtr->chunkIndex
* @param[in,out] unitInfoPtr              - pointer to the relevant unit - after alignment of addresses.
*/
static void falcon_flexRangeAddrUnitAlignToBase(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SMEM_UNIT_CHUNKS_STC *unitInfoPtr,
    IN    GT_BOOL               isZeroBased
)
{
    GT_U32  ii;
    SMEM_CHUNK_STC  *chunkPtr;
    GT_U32  baseAddr = UNIT_CHUNK_BASE_ADDR_GET_MAC(devObjPtr,unitInfoPtr);

    chunkPtr = &unitInfoPtr->chunksArray[0];
    if(chunkPtr->memFirstAddr >= baseAddr)
    {
        /* already initialized */
        return;
    }

    for(ii = 0 ; ii < unitInfoPtr->numOfChunks ; ii++)
    {
        chunkPtr = &unitInfoPtr->chunksArray[ii];

        chunkPtr->memFirstAddr += baseAddr;
        chunkPtr->memLastAddr  += baseAddr;
    }
}

/**
* @internal falcon_flexRangeAddrRegistersAlignToBase function
* @endinternal
*
* @brief   align all addresses of the array of registers according to
*         to base address of the unit
* @param[in] devObjPtr                - pointer to device object.
* @param[in] registersArr[]           - array of registers
* @param[in] numOfRegisters           - number of registers in registersArr
* @param[in] unitInfoPtr              - pointer to the relevant unit .
* @param[in] isZeroBased              - GT_FALSE - the same as function smemGenericRegistersArrayAlignToUnit
*                                      GT_TRUE  - the addresses in unitInfoPtr are '0' based and
*                                      need to add to all of then the unit base address from unitInfoPtr->chunkIndex
*/
static void falcon_flexRangeAddrRegistersAlignToBase(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32               registersArr[],
    IN GT_U32               numOfRegisters,
    IN SMEM_UNIT_CHUNKS_STC *unitInfoPtr,
    IN    GT_BOOL               isZeroBased
)
{
    GT_U32  ii;
    GT_U32  baseAddr = UNIT_CHUNK_BASE_ADDR_GET_MAC(devObjPtr,unitInfoPtr);

    ii = 0;
    while(ii < numOfRegisters && registersArr[ii] == SMAIN_NOT_VALID_CNS)
    {
        ii++;
    }
    /* found first non SMAIN_NOT_VALID_CNS */
    if((ii >= numOfRegisters) ||
        (registersArr[ii] >= baseAddr))
    {
        /* already initialized */
        return;
    }

    for(; ii < numOfRegisters ; ii ++)
    {
        if(registersArr[ii] == SMAIN_NOT_VALID_CNS)
        {
            /* not used register */
            continue;
        }

        registersArr[ii] += baseAddr;
    }
}



/**
* @internal smemGenericUnitAddressesAlignToBaseAddress1 function
* @endinternal
*
* @brief   align all addresses of the unit according to base address of the unit
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in,out] unitInfoPtr              - pointer to the relevant unit - before alignment of addresses.
* @param[in] isZeroBased              - GT_FALSE - the same as function smemGenericUnitAddressesAlignToBaseAddress
*                                      GT_TRUE  - the addresses in unitInfoPtr are '0' based and
*                                      need to add to all of then the unit base address from unitInfoPtr->chunkIndex
* @param[in,out] unitInfoPtr              - pointer to the relevant unit - after alignment of addresses.
*/
void smemGenericUnitAddressesAlignToBaseAddress1(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SMEM_UNIT_CHUNKS_STC *unitInfoPtr,
    IN    GT_BOOL               isZeroBased
)
{
    GT_U32  unitBaseAddressOffset =  SMEM_CHT_UNIT_INDEX_FIRST_BIT_CNS;
    GT_U32  ii;
    SMEM_CHUNK_STC  *chunkPtr;
    GT_U32  unitId;
    GT_U32  baseAddr = (unitInfoPtr->chunkIndex << unitBaseAddressOffset);

    if(devObjPtr->support_memoryRanges)
    {
        /* the base address settings to the chunks can not depend of 'prefix' */
        /* need to check if the address are already in range or not           */
        falcon_flexRangeAddrUnitAlignToBase(devObjPtr,unitInfoPtr,isZeroBased);
        return;
    }


    for(ii = 0 ; ii < unitInfoPtr->numOfChunks ; ii ++)
    {
        chunkPtr = &unitInfoPtr->chunksArray[ii];

        if(isZeroBased == GT_FALSE)
        {
            unitId = SMEM_U32_GET_FIELD(chunkPtr->memFirstAddr,unitBaseAddressOffset,9);

            if( unitId != 0 && unitId != unitInfoPtr->chunkIndex)
            {
                if(unitInfoPtr->chunkType == SMEM_UNIT_CHUNK_TYPE_8_MSB_E &&
                  (((unitId>>1) == 0) || ((unitId>>1) == (unitInfoPtr->chunkIndex >> 1))
                   ))
                {
                    /* all is ok in unit of 8 MSBits */
                }
                else
                {
                    simGeneralPrintf(" segment in chunk ,address : [0x%8.8x] not belong to unit [0x%8.8x] \n",chunkPtr->memFirstAddr,
                        (unitInfoPtr->chunkIndex << unitBaseAddressOffset));
                }
            }

            unitId = SMEM_U32_GET_FIELD(chunkPtr->memLastAddr,unitBaseAddressOffset,9);

            if( unitId != 0 && unitId != unitInfoPtr->chunkIndex)
            {
                if(unitInfoPtr->chunkType == SMEM_UNIT_CHUNK_TYPE_8_MSB_E &&
                  (((unitId>>1) == 0) || ((unitId>>1) == (unitInfoPtr->chunkIndex >> 1))
                   ))
                {
                    /* all is ok in unit of 8 MSBits */
                }
                else
                {
                    simGeneralPrintf(" segment in chunk ,last address : [0x%8.8x] not belong to unit [0x%8.8x] \n",chunkPtr->memLastAddr,
                        (unitInfoPtr->chunkIndex << unitBaseAddressOffset));
                }
            }

            /* update the addresses */
            if(unitInfoPtr->chunkType == SMEM_UNIT_CHUNK_TYPE_8_MSB_E)
            {
                SMEM_U32_SET_FIELD(chunkPtr->memFirstAddr,unitBaseAddressOffset+1,8,unitInfoPtr->chunkIndex>>1);
                SMEM_U32_SET_FIELD(chunkPtr->memLastAddr ,unitBaseAddressOffset+1,8,unitInfoPtr->chunkIndex>>1);
            }
            else
            {
                SMEM_U32_SET_FIELD(chunkPtr->memFirstAddr,unitBaseAddressOffset,9,unitInfoPtr->chunkIndex);
                SMEM_U32_SET_FIELD(chunkPtr->memLastAddr,unitBaseAddressOffset,9,unitInfoPtr->chunkIndex);
            }
        }
        else
        {
            /* use += to support 'huge' units with addresses from several 'sub units' */

            chunkPtr->memFirstAddr += baseAddr;
            chunkPtr->memLastAddr  += baseAddr;
        }
    }


    return;
}

static GT_BOOL addrAlignForceReset = GT_FALSE;
/**
* @internal smemGenericRegistersArrayAlignForceUnitReset function
* @endinternal
*
* @brief   indication that addresses will be overriding the previous address without
*         checking that address belong to unit.
* @param[in] forceReset               - indication to force override
*/
void smemGenericRegistersArrayAlignForceUnitReset(
    IN    GT_BOOL               forceReset
)
{
    addrAlignForceReset = forceReset;
}


/**
* @internal smemGenericUnitAddressesAlignToBaseAddress function
* @endinternal
*
* @brief   align all addresses of the unit according to base address of the unit
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in,out] unitInfoPtr              - pointer to the relevant unit - before alignment of addresses.
* @param[in,out] unitInfoPtr              - pointer to the relevant unit - after alignment of addresses.
*/
void smemGenericUnitAddressesAlignToBaseAddress(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SMEM_UNIT_CHUNKS_STC *unitInfoPtr
)
{
    smemGenericUnitAddressesAlignToBaseAddress1(devObjPtr,unitInfoPtr,GT_FALSE);
}


/**
* @internal smemGenericRegistersArrayAlignToUnit1 function
* @endinternal
*
* @brief   align all addresses of the array of registers according to
*         to base address of the unit
* @param[in] registersArr[]           - array of registers
* @param[in] numOfRegisters           - number of registers in registersArr
* @param[in] unitInfoPtr              - pointer to the relevant unit .
* @param[in] isZeroBased              - GT_FALSE - the same as function smemGenericRegistersArrayAlignToUnit
*                                      GT_TRUE  - the addresses in unitInfoPtr are '0' based and
*                                      need to add to all of then the unit base address from unitInfoPtr->chunkIndex
*/
void smemGenericRegistersArrayAlignToUnit1(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32               registersArr[],
    IN GT_U32               numOfRegisters,
    IN SMEM_UNIT_CHUNKS_STC *unitInfoPtr,
    IN    GT_BOOL               isZeroBased
)
{
    GT_U32  unitBaseAddressOffset;
    GT_U32  ii;
    GT_U32  baseAddr;

    if(devObjPtr->support_memoryRanges)
    {
        /* the base address settings to the chunks can not depend of 'prefix' */
        /* need to check if the address are already in range or not           */
        falcon_flexRangeAddrRegistersAlignToBase(devObjPtr,registersArr,numOfRegisters,unitInfoPtr,isZeroBased);
        return;
    }

    unitBaseAddressOffset =  SMEM_CHT_UNIT_INDEX_FIRST_BIT_CNS;
    baseAddr = (unitInfoPtr->chunkIndex << unitBaseAddressOffset);

    for(ii = 0 ; ii < numOfRegisters ; ii ++)
    {
        if(registersArr[ii] == SMAIN_NOT_VALID_CNS)
        {
            /* not used register */
            continue;
        }

        if(isZeroBased == GT_FALSE)
        {
            if(addrAlignForceReset == GT_FALSE)
            {
                if( SMEM_U32_GET_FIELD(registersArr[ii],unitBaseAddressOffset,9) != 0 &&
                    SMEM_U32_GET_FIELD(registersArr[ii],unitBaseAddressOffset,9) != unitInfoPtr->chunkIndex)
                {
                    simGeneralPrintf(" register, address : [0x%8.8x] not belong to unit [0x%8.8x] \n",registersArr[ii],
                        (unitInfoPtr->chunkIndex << unitBaseAddressOffset));
                }
            }
            /* update the address */
            SMEM_U32_SET_FIELD(registersArr[ii],unitBaseAddressOffset,9,unitInfoPtr->chunkIndex);
        }
        else
        {
            /* use += to support 'huge' units with addresses from several 'sub units' */

            registersArr[ii] += baseAddr;
        }
    }


    return;
}

/**
* @internal smemGenericRegistersArrayAlignToUnit function
* @endinternal
*
* @brief   align all addresses of the array of registers according to
*         to base address of the unit
* @param[in] registersArr[]           - array of registers
* @param[in] numOfRegisters           - number of registers in registersArr
* @param[in] unitInfoPtr              - pointer to the relevant unit .
*/
void smemGenericRegistersArrayAlignToUnit(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32               registersArr[],
    IN GT_U32               numOfRegisters,
    IN SMEM_UNIT_CHUNKS_STC *unitInfoPtr
)
{
    smemGenericRegistersArrayAlignToUnit1(devObjPtr,registersArr,numOfRegisters,unitInfoPtr,GT_FALSE);
}

/*******************************************************************************
*   smemGenericFindMem
*
* DESCRIPTION:
*       Return pointer to the register's or tables's memory.
*
* INPUTS:
*       devObjPtr   - pointer to device object.
*       accessType  - Read/Write operation
*       address     - address of memory(register or table).
*       memsize     - size of memory
*
* OUTPUTS:
*       activeMemPtrPtr - pointer to the active memory entry or NULL if not exist.
*
* RETURNS:
*       pointer to the memory location
*       NULL - if memory not exist
*
* COMMENTS:
*
*
*******************************************************************************/
void * smemGenericFindMem
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SCIB_MEMORY_ACCESS_TYPE accessType,
    IN GT_U32                  address,
    IN GT_U32                  memSize,
    OUT SMEM_ACTIVE_MEM_ENTRY_STC ** activeMemPtrPtr
)
{
    void                    * memPtr = NULL;
    SMEM_CHT_GENERIC_DEV_MEM_INFO  * devMemInfoPtr;
    GT_U32                    index,ii;
    GT_UINTPTR                param;
    SMEM_ACTIVE_MEM_ENTRY_STC   *unitActiveMemPtr = NULL;/*active memory dedicated to the unit*/
    SMEM_ACTIVE_MEM_ENTRY_STC   *activeMemPtr;/*active memory of the device/unit*/
    GT_BIT                    forceIsPex;
    GT_BIT                    mustBeGeneric;
    GT_BOOL                   isAddressFound;

    devMemInfoPtr = devObjPtr->deviceMemory;

    if(IS_BAR0_OPERATION_MAC(accessType))
    {
        /* drop the 'PEX' address that BAR0 hold , and use the 'offset' in it */
        if(devMemInfoPtr->common.isPartOfGeneric && devMemInfoPtr->BAR0_UnitMem.numOfChunks)
        {
            /* the device hold special unit for the PEX registers ..
               not need to look at devMemInfoPtr->common.specFunTbl functions */

            param   = (GT_UINTPTR)(void*)&devMemInfoPtr->BAR0_UnitMem;
            memPtr  = smemDevFindInUnitChunk(devObjPtr, accessType, address, memSize, param);
            if(memPtr == NULL)
            {
                if(!debugModeByPassFatalErrorOnMemNotExists)
                {
                    skernelFatalError("smemGenericFindMem: not found BAR0 memory [0x%8.8x] \n" ,address );
                }
                return NULL;
            }
            /* check if there is dedicated active memory for the unit */
            unitActiveMemPtr =
                devMemInfoPtr->BAR0_UnitMem.unitActiveMemPtr;
        }
    }


    if(devMemInfoPtr->common.accessPexMemorySpaceOnlyOnExplicitAction == 0 &&
        devMemInfoPtr->common.pciUnitBaseAddr &&
       (address & 0xFFFF0000) == devMemInfoPtr->common.pciUnitBaseAddr)
    {
        forceIsPex = 1;
    }
    else
    {
        forceIsPex = 0;
    }

    /* Find PCI registers memory */
    if (SMEM_ACCESS_PCI_FULL_MAC(accessType) || forceIsPex)
    {
        mustBeGeneric = 0;
        if((address & 0xFFFF0000) == devMemInfoPtr->common.pciUnitBaseAddr)
        {
            /* support direct access in offset 0x000F0000 */
        }
        else if ((address & 0xFFFF0000) == 0x00000000)
        {
            /* support direct access in offset 0x00000000 */
            address += devMemInfoPtr->common.pciUnitBaseAddr;
        }
        else
        {
            /* moved the skernelFatalError down from here */
            mustBeGeneric = 1;
        }
        /* continue and find the memory */

        if(devMemInfoPtr->common.isPartOfGeneric && devMemInfoPtr->PEX_UnitMem.numOfChunks)
        {
            /* the device hold special unit for the PEX registers ..
               not need to look at devMemInfoPtr->common.specFunTbl functions */

            param   = (GT_UINTPTR)(void*)&devMemInfoPtr->PEX_UnitMem;
            memPtr  = smemDevFindInUnitChunk(devObjPtr, accessType, address, memSize, param);
            if(memPtr == NULL)
            {
                if(!debugModeByPassFatalErrorOnMemNotExists)
                {
                    /* smemFindMemory will handle it */
                    skernelFatalError("smemGenericFindMem: not found PEX memory [0x%8.8x] \n" ,address );
                }
                return NULL;
            }
            /* check if there is dedicated active memory for the unit */
            unitActiveMemPtr =
                devMemInfoPtr->PEX_UnitMem.unitActiveMemPtr;
        }

        if(mustBeGeneric && memPtr == NULL)
        {
            skernelFatalError("smemGenericFindMem: unknown PEX memory [0x%8.8x] \n" ,address );
        }
    }

    if(memPtr)/* already got memory */
    {
        /* nothing to do as we got memory already
           (and maybe also active memory of unit) */
    }
    else
    {
        if(devObjPtr->support_memoryRanges)
        {
            index = UNIT_INDEX_FROM_ADDR_GET_MAC(devObjPtr,address);
        }
        else
        if(SMEM_CHT_IS_SIP5_GET(devObjPtr))
        {
            /* support 8 * 64 = 512 sub units !!! */
            /* not only 64 sub units as in previous device */
            index = address >> SMEM_CHT_UNIT_INDEX_FIRST_BIT_CNS;
        }
        else if(SKERNEL_IS_AC5_BASED_DEV(devObjPtr))
        {
            /* like sip5 : remove the limit of 6 bits unit address */
            /* as the AC5 uses CNM like in Phoenix                 */
            index = address >> SMEM_CHT_UNIT_INDEX_FIRST_BIT_CNS;
        }
        else
        {
            index = (address & REG_SPEC_FUNC_INDEX) >>
                         SMEM_CHT_UNIT_INDEX_FIRST_BIT_CNS;
        }

        if(index >= SMEM_CHT_NUM_UNITS_MAX_CNS)
        {
            if(index == SMAIN_NOT_VALID_CNS)
            {
                /* smemFindMemory will handle it */
                return NULL;
            }
            skernelFatalError("smemGenericFindMem : unitIndex[%d] >= max of [%d] \n" ,
                index,SMEM_CHT_NUM_UNITS_MAX_CNS);
        }


        param   = devMemInfoPtr->common.specFunTbl[index].specParam;
        memPtr  = devMemInfoPtr->common.specFunTbl[index].specFun(
                            devObjPtr, accessType, address, memSize, param);

        /*  pointer to active memory of the unit.
            when != NULL , using ONLY this active memory.
            when NULL , using the active memory of the device.
        */
        unitActiveMemPtr =
            devMemInfoPtr->common.specFunTbl[index].unitActiveMemPtr;
    }

    /* find active memory entry */
    if (activeMemPtrPtr != NULL)
    {
        *activeMemPtrPtr = NULL;

        activeMemPtr = unitActiveMemPtr ?
                       unitActiveMemPtr :
                       devObjPtr->activeMemPtr;

        for (ii = 0; activeMemPtr[ii].address != END_OF_TABLE; ii++)
        {
            /* Find address in registers DB */
            if (activeMemPtr[ii].extMemAddressFindFun)
            {
                isAddressFound =
                    activeMemPtr[ii].extMemAddressFindFun(devObjPtr,
                                                          address,
                                                          (GT_UINTPTR)&(activeMemPtr[ii]));
                if (isAddressFound == GT_TRUE)
                {
                    *activeMemPtrPtr = &activeMemPtr[ii];
                    break;
                }
            }
            else
            {
                /* check address    */
                if ((address & activeMemPtr[ii].mask)
                     == (activeMemPtr[ii].address & activeMemPtr[ii].mask))
                {
                    *activeMemPtrPtr = &activeMemPtr[ii];
                    break;
                }
            }
        }
    }

    return memPtr;
}


/**
* @internal smemIsDeviceMemoryOwner function
* @endinternal
*
* @brief   Return indication that the device is the owner of the memory.
*         relevant to multi port groups where there is 'shared memory' between port groups.
*
* @retval GT_TRUE                  - the device is   the owner of the memory.
* @retval GT_FALSE                 - the device is NOT the owner of the memory.
*/
GT_BOOL smemIsDeviceMemoryOwner
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32                  address
)
{
    if(devObjPtr->devIsOwnerMemFunPtr)
    {
        return devObjPtr->devIsOwnerMemFunPtr(devObjPtr,address);
    }

    /* the device own the address (BWC) */
    return GT_TRUE;
}

/**
* @internal smemDfxRegFldGet function
* @endinternal
*
* @brief   Read data from the DFX register's field
*
* @note The function aborts application if address not exist.
*
*/
void smemDfxRegFldGet
(
    IN SKERNEL_DEVICE_OBJECT * deviceObjPtr,
    IN GT_U32                  address,
    IN GT_U32                  fldFirstBit,
    IN GT_U32                  fldSize,
    OUT GT_U32               * data
)
{
    GT_U32 * findMemPtr;
    GT_U32  regValue, fieldMask;
    SCIB_MEMORY_ACCESS_TYPE accessType = SKERNEL_MEMORY_READ_DFX_E;

    smemConvertDevAndAddrToNewDevAndAddr(deviceObjPtr,address,accessType,
        &deviceObjPtr,&address);

    paramCheck(fldFirstBit,fldSize);

    /* check device type and call device search memory function*/
    findMemPtr = smemFindMemory(deviceObjPtr, address, 1, NULL,
                                accessType);
    regValue = findMemPtr[0];

    /* Align data right according to the first field bit */
    regValue >>= fldFirstBit;

    fldSize = fldSize % 32;
    /* get field's bits mask */
    if (fldSize)
    {
        fieldMask = ~(0xffffffff << fldSize);
    }
    else
    {
        fieldMask = (0xffffffff);
    }

    *data = regValue & fieldMask;

    /* log memory information */
    simLogMemory(deviceObjPtr, SIM_LOG_MEMORY_DFX_READ_E, SIM_LOG_MEMORY_DEV_E,
                                            address, *data, 0);
}

/**
* @internal smemDfxRegFldSet function
* @endinternal
*
* @brief   Write data to the DFX register's field
*
* @note The function aborts application if address not exist.
*
*/
void smemDfxRegFldSet
(
    IN SKERNEL_DEVICE_OBJECT * deviceObjPtr,
    IN GT_U32                  address,
    IN GT_U32                  fldFirstBit,
    IN GT_U32                  fldSize,
    IN GT_U32                  data
)
{
    GT_U32 * findMemPtr;
    GT_U32 regValue, fieldMask;
    SCIB_MEMORY_ACCESS_TYPE accessType = SKERNEL_MEMORY_WRITE_DFX_E;

    smemConvertDevAndAddrToNewDevAndAddr(deviceObjPtr,address,accessType,
        &deviceObjPtr,&address);

    paramCheck(fldFirstBit,fldSize);

    fldSize = fldSize % 32;
    /* get field's bits mask */
    if (fldSize)
    {
        fieldMask = ~(0xffffffff << fldSize);
    }
    else
    {
        fieldMask = 0xffffffff;
    }

    /* Get register value */
    findMemPtr = smemFindMemory(deviceObjPtr, address, 1, NULL,
                                accessType);
    regValue = findMemPtr[0];
    /* Set field value in the register value using
    inverted real field mask in the place of the field */
    regValue &= ~(fieldMask << fldFirstBit);
    regValue |= (data << fldFirstBit);

    /* log memory information */
    simLogMemory(deviceObjPtr, SIM_LOG_MEMORY_DFX_WRITE_E, SIM_LOG_MEMORY_DEV_E,
                                            address, regValue, *findMemPtr);
    *findMemPtr = regValue;
}

/**
* @internal smemDfxRegSet function
* @endinternal
*
* @brief   Write data to the DFX register's
*
* @note The function aborts application if address not exist.
*
*/
void smemDfxRegSet
(
    IN SKERNEL_DEVICE_OBJECT * deviceObjPtr,
    IN GT_U32                  address,
    IN GT_U32                  data
)
{
    smemDfxRegFldSet(deviceObjPtr, address, 0, 32, data);
}

/**
* @internal smemDfxRegGet function
* @endinternal
*
* @brief   Read data from the DFX register's
*
* @note The function aborts application if address not exist.
*
*/
void smemDfxRegGet
(
    IN SKERNEL_DEVICE_OBJECT * deviceObjPtr,
    IN GT_U32                  address,
    IN GT_U32                * dataPtr
)
{
    smemDfxRegFldGet(deviceObjPtr, address, 0, 32, dataPtr);
}

/**
* @internal smemBar0RegSet function
* @endinternal
*
* @brief   Write data to the BAR0 register's (without active memory)
*
* @note The function aborts application if address not exist.
*
*/
void smemBar0RegSet
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32                  address,
    IN GT_U32                  data
)
{
    GT_U32 * findMemPtr;
    SCIB_MEMORY_ACCESS_TYPE accessType = SKERNEL_MEMORY_WRITE_BAR0_E;

    smemConvertDevAndAddrToNewDevAndAddr(devObjPtr,address,accessType,
        &devObjPtr,&address);

    /* Get register value */
    findMemPtr = smemFindMemory(devObjPtr, address, 1, NULL,
                                accessType);
    *findMemPtr = data;
}
/**
* @internal smemDeviceObjMemoryAllAllocationsFree function
* @endinternal
*
* @brief   free all memory allocations that the device ever done.
*         optionally to including 'own' pointer !!!
*         this device must do 'FULL init sequence' if need to be used.
* @param[in] devObjPtr                - pointer to device object
* @param[in] freeMyObj                - indication to free the memory of 'device itself'
*                                      GT_TRUE  - free devObjPtr
*                                      GT_FALSE - do not free devObjPtr
*/
void smemDeviceObjMemoryAllAllocationsFree
(
    IN SKERNEL_DEVICE_OBJECT  * devObjPtr,
    IN GT_BOOL                  freeMyObj
)
{
    SKERNEL_DEVICE_MEMORY_ALLOCATION_STC *currentPtr;
    SKERNEL_DEVICE_MEMORY_ALLOCATION_STC *nextPtr;
    GT_BIT          isFirst = 1;

    if(devObjPtr == NULL)
    {
        skernelFatalError("smemDeviceObjFreeAllMemoryAllocations : not valid for NULL device \n");
    }

    SCIB_SEM_TAKE;

    currentPtr = &devObjPtr->myDeviceAllocations;
    /* skip first element as this is the 'device itself' */
    currentPtr = currentPtr->nextElementPtr;

    while(currentPtr)
    {
        nextPtr = currentPtr->nextElementPtr;
        /* free the allocation needed by the device */
        if(currentPtr->myMemoryPtr)/* free only if not NULL , because maybe already released */
        {
            MALLOC_FILE_NAME_DEBUG_PRINT(("free ptr[%x] from smemDeviceObjMemoryAllAllocationsFree, that was allocated from file[%s] line[%d] size[%d]bytes \n",
                    currentPtr->myMemoryPtr,
                    currentPtr->fileNamePtr,
                    currentPtr->line,
                    currentPtr->myMemoryNumOfBytes
                    ));

            free(currentPtr->myMemoryPtr);
        }

        if(isFirst == 0)/* first element was not dynamic allocated */
        {
            /* free the element itself */
            free(currentPtr);
        }

        isFirst = 0;

        /* update the 'current' to be 'next' */
        currentPtr = nextPtr;
    }

    if(freeMyObj == GT_TRUE)
    {
        /* free the device itself !!!! */
        free(devObjPtr);
    }
    else
    {
        /* must reset next info */
        devObjPtr->myDeviceAllocations.myMemoryPtr = NULL;
        devObjPtr->myDeviceAllocations.myMemoryNumOfBytes = 0;
        devObjPtr->myDeviceAllocations.nextElementPtr = NULL;
        devObjPtr->lastDeviceAllocPtr = NULL;
    }

    SCIB_SEM_SIGNAL;
}

/* compress file name from:
   x:\cpss\sw\prestera\simulation\simdevices\src\asicsimulation\skernel\suserframes\snetcheetahingress.c
   To:
   snetcheetahingress.c */
static GT_CHAR * compressFileName
(
    IN GT_CHAR const *fileName
)
{
    GT_CHAR *tempName;
    GT_CHAR *compressedFileName = (GT_CHAR *)fileName;
    while(NULL != (tempName = strchr(compressedFileName,'\\')))
    {
        compressedFileName = tempName+1;
    }

    return compressedFileName;
}

/**
* @internal smemDeviceObjMemoryAlloc__internal function
* @endinternal
*
* @brief   allocate memory (calloc style) that is registered with the device object.
*         this call instead of direct call to calloc/malloc needed for 'soft reset'
*         where all the dynamic memory of the device should be free/cleared.
* @param[in] devObjPtr                - pointer to device object (ignored if NULL)
* @param[in] nitems                   - alloc for number of elements (calloc style)
* @param[in] size                     -  of each element         (calloc style)
* @param[in] fileNamePtr              -  file name that called the allocation
* @param[in] line                     -   in the file that called the allocation
*                                       pointer to allocated memory
*/
GT_PTR smemDeviceObjMemoryAlloc__internal
(
    IN SKERNEL_DEVICE_OBJECT  * devObjPtr,
    IN size_t   nitems,
    IN size_t   size,
    IN const char*     fileNamePtr,
    IN GT_U32          line
)
{
    GT_PTR *allocPtr;
    SKERNEL_DEVICE_MEMORY_ALLOCATION_STC    *nextElementPtr;

    if(size > MAX_ALLOC_CNS || nitems > MAX_ALLOC_CNS || (size*nitems) > MAX_ALLOC_CNS)
    {
        /* fix SV.TAINTED.ALLOC_SIZE warning from klockwork that nitems or size comes from 'sscanf'
           and we need to protect the allocation */
        skernelFatalError("smemDeviceObjMemoryAlloc__internal : suspected 'Tainted data' for calloc %d bytes \n" ,
            (nitems*size));
        return NULL;
    }

    allocPtr = calloc(nitems,size);/*allocate memory needed by the caller*/

    if(allocPtr == NULL)
    {
        skernelFatalError("smemDeviceObjMemoryAlloc__internal : calloc of %d bytes failed \n" ,
            (nitems*size));
        return NULL;
    }

    SCIB_SEM_TAKE;

    if(devObjPtr)
    {
        devObjPtr->totalNumberOfBytesAllocated += (nitems*size) + /* allocation done for the caller */
                    sizeof(SKERNEL_DEVICE_MEMORY_ALLOCATION_STC); /* allocation done for the 'nextElementPtr' */

        if(devObjPtr->lastDeviceAllocPtr == NULL)
        {
            /* first allocation register the 'device itself' as first element */
            devObjPtr->lastDeviceAllocPtr = &devObjPtr->myDeviceAllocations;

            devObjPtr->myDeviceAllocations.myMemoryPtr = devObjPtr;
            devObjPtr->myDeviceAllocations.myMemoryNumOfBytes = sizeof(SKERNEL_DEVICE_OBJECT);
        }

        /* new allocation so need new 'element' */
        nextElementPtr = malloc(sizeof(*nextElementPtr));
        if(nextElementPtr == NULL)
        {
            skernelFatalError("smemDeviceObjMemoryAlloc__internal : malloc of %d bytes failed \n" ,
                sizeof(*nextElementPtr));

            SCIB_SEM_SIGNAL;

            return NULL;
        }
        /* bind the previous element to the new element */
        devObjPtr->lastDeviceAllocPtr->nextElementPtr = nextElementPtr;

        nextElementPtr->myMemoryPtr    = allocPtr;
        nextElementPtr->myMemoryNumOfBytes = (GT_U32)(nitems*size);
        nextElementPtr->nextElementPtr = NULL;
        nextElementPtr->fileNamePtr = compressFileName(fileNamePtr);
        nextElementPtr->line = line;

        MALLOC_FILE_NAME_DEBUG_PRINT(("alloc ptr[%x] from file[%s] line[%d] size[%d]bytes \n",
                allocPtr,
                nextElementPtr->fileNamePtr,
                nextElementPtr->line,
                nextElementPtr->myMemoryNumOfBytes
                ));

        /* update last element pointer to the newly allocated element*/
        devObjPtr->lastDeviceAllocPtr = nextElementPtr;
    }
    else
    {
        MALLOC_FILE_NAME_DEBUG_PRINT(("non-dev:alloc ptr[%x] from file[%s] line[%d] size[%d]bytes \n",
                allocPtr,
                compressFileName(fileNamePtr),
                line,
                nitems*size
                ));
    }

    SCIB_SEM_SIGNAL;

    return allocPtr;
}

/*******************************************************************************
*   smemDeviceObjMemoryFind
*
* DESCRIPTION:
*       return pointer to 'STC memory allocation' memory that hold 'searchForPtr'
*
* INPUTS:
*       devObjPtr - pointer to device object
*       searchForPtr - the pointer of memory that was returned by
*           smemDeviceObjMemoryAlloc__internal or smemDeviceObjMemoryRealloc__internal
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       pointer to 'STC memory allocation' memory that hold 'searchForPtr'
* COMMENTS:
*       assuming that function is under 'SCIB lock'
*
*******************************************************************************/
static SKERNEL_DEVICE_MEMORY_ALLOCATION_STC* smemDeviceObjMemoryFind
(
    IN SKERNEL_DEVICE_OBJECT  * devObjPtr,
    IN GT_PTR                   searchForPtr
)
{
    SKERNEL_DEVICE_MEMORY_ALLOCATION_STC *currentPtr;

    if(devObjPtr == NULL)
    {
        skernelFatalError("smemDeviceObjMemoryFind : not valid for NULL device \n");
        return NULL;
    }

    currentPtr = &devObjPtr->myDeviceAllocations;

    do
    {
        if(currentPtr->myMemoryPtr == searchForPtr)
        {
            /* got match */
            return currentPtr;
        }

        /* update the 'current' to be 'next' */
        currentPtr = currentPtr->nextElementPtr;
    }while(currentPtr);

    skernelFatalError("smemDeviceObjMemoryFind : memory not found <-- simulation management error \n");
    return NULL;
}

/**
* @internal smemDeviceObjMemoryRealloc__internal function
* @endinternal
*
* @brief   return new memory allocation after 'realloc' old memory to new size.
*         the oldPointer should be pointer that smemDeviceObjMemoryAlloc__internal(..) returned.
*         this call instead of direct call to realloc needed for 'soft reset'
*         where all the dynamic memory of the device should be free/cleared.
* @param[in] devObjPtr                - pointer to device object (ignored if NULL)
* @param[in] oldPointer               - pointer to the 'old' element (realloc style)
* @param[in] size                     - number of bytes for new segment (realloc style)
* @param[in] fileNamePtr              -  file name that called the re-allocation
* @param[in] line                     -   in the file that called the re-allocation
*                                       pointer to reallocated memory
*/
GT_PTR smemDeviceObjMemoryRealloc__internal
(
    IN SKERNEL_DEVICE_OBJECT  * devObjPtr,
    IN GT_PTR                   oldPointer,
    IN size_t                   size,
    IN const char*              fileNamePtr,
    IN GT_U32                   line
)
{
    SKERNEL_DEVICE_MEMORY_ALLOCATION_STC    *elementPtr;
    GT_PTR *reallocPtr;/*reallocate memory needed by the caller*/

    if(size > MAX_ALLOC_CNS)
    {
        /* fix SV.TAINTED.ALLOC_SIZE warning from klockwork that size comes from 'sscanf'
           and we need to protect the allocation */
        skernelFatalError("smemDeviceObjMemoryRealloc__internal : suspected 'Tainted data' for realloc %d bytes \n" ,
            size);
        return NULL;
    }

    if(devObjPtr == NULL)
    {
        /* bypass 'device' management */
        reallocPtr = realloc(oldPointer,size);

        MALLOC_FILE_NAME_DEBUG_PRINT(("non-dev:realloc new ptr[%x] from file[%s] line[%d] size[%d],that was ptr[%x] (unknown src file/size) \n",
                reallocPtr,
                compressFileName(fileNamePtr),
                line,
                size,
                oldPointer
                ));

        return reallocPtr;
    }

    if(oldPointer == NULL)
    {
        return smemDeviceObjMemoryAlloc__internal(devObjPtr,size,1,fileNamePtr,line);
    }


    SCIB_SEM_TAKE;

    elementPtr = smemDeviceObjMemoryFind(devObjPtr,oldPointer);
    if(elementPtr == NULL)
    {
        SCIB_SEM_SIGNAL;
        /* fatal error already asserted */
        return NULL;
    }

    /* NOTE that : elementPtr->myMemoryPtr == oldPointer !!! */

    reallocPtr = realloc(oldPointer,size);
    if(reallocPtr == NULL)
    {
        skernelFatalError("smemDeviceObjMemoryRealloc__internal : realloc of %d bytes failed \n" ,
            size);

        SCIB_SEM_SIGNAL;

        return NULL;
    }

    MALLOC_FILE_NAME_DEBUG_PRINT(("realloc new ptr[%x] from file[%s] line[%d] size[%d], that was ptr[%x] allocated from file[%s] line[%d] size[%d]bytes \n",
            reallocPtr,
            compressFileName(fileNamePtr),
            line,
            size,
            elementPtr->myMemoryPtr,
            elementPtr->fileNamePtr,
            elementPtr->line,
            elementPtr->myMemoryNumOfBytes
            ));

    elementPtr->myMemoryPtr = reallocPtr;
    elementPtr->myMemoryNumOfBytes = (GT_U32)size;
    elementPtr->fileNamePtr = compressFileName(fileNamePtr);
    elementPtr->line = line;

    SCIB_SEM_SIGNAL;

    return reallocPtr;

}

/**
* @internal smemDeviceObjMemoryPtrFree__internal function
* @endinternal
*
* @brief   free memory that was allocated by smemDeviceObjMemoryAlloc__internal or
*         smemDeviceObjMemoryRealloc__internal
*         this call instead of direct call to free needed for 'soft reset'
*         where all the dynamic memory of the device should be free/cleared.
* @param[in] devObjPtr                - pointer to device object (ignored if NULL)
* @param[in] oldPointer               - pointer to the 'old' pointer that need to be free
* @param[in] fileNamePtr              -  file name that called the free
* @param[in] line                     -   in the file that called the free
*                                       none.
*/
void smemDeviceObjMemoryPtrFree__internal
(
    IN SKERNEL_DEVICE_OBJECT  * devObjPtr,
    IN GT_PTR                   oldPointer,
    IN const char*              fileNamePtr,
    IN GT_U32                   line

)
{
    SKERNEL_DEVICE_MEMORY_ALLOCATION_STC    *elementPtr;

    if(oldPointer == NULL)
    {
        /* it seems that there are places that will get us here !!! */
        return ;
    }

    if(devObjPtr == NULL)
    {
        MALLOC_FILE_NAME_DEBUG_PRINT(("non-dev : free ptr[%x] from file[%s] line[%d](unknown src file/size) \n",
                oldPointer,
                compressFileName(fileNamePtr),
                line
                ));

        /* bypass 'device' management */
        free(oldPointer);
        return;
    }

    SCIB_SEM_TAKE;

    elementPtr = smemDeviceObjMemoryFind(devObjPtr,oldPointer);
    if(elementPtr == NULL)
    {
        SCIB_SEM_SIGNAL;
        /* fatal error already asserted */
        return;
    }

    MALLOC_FILE_NAME_DEBUG_PRINT(("free ptr[%x] from file[%s] line[%d], that was allocated from file[%s] line[%d] size[%d]bytes \n",
            oldPointer,
            compressFileName(fileNamePtr),
            line,
            elementPtr->fileNamePtr,
            elementPtr->line,
            elementPtr->myMemoryNumOfBytes
            ));

    /* NOTE that : elementPtr->myMemoryPtr == oldPointer !!! */
    free(oldPointer);

    elementPtr->myMemoryPtr = NULL;
    elementPtr->myMemoryNumOfBytes = 0;
    elementPtr->fileNamePtr = NULL;
    elementPtr->line = 0;

    SCIB_SEM_SIGNAL;

    /* keep the element and not change the 'link list'*/
    return;
}

/**
* @internal smemDeviceObjMemoryPtrMemSetZero function
* @endinternal
*
* @brief   'memset 0' to memory that was allocated by smemDeviceObjMemoryAlloc__internal or
*         smemDeviceObjMemoryRealloc__internal
* @param[in] devObjPtr                - pointer to device object
* @param[in] memPointer               - pointer to the memory that need to be set to 0
*                                       none.
*/
void smemDeviceObjMemoryPtrMemSetZero
(
    IN SKERNEL_DEVICE_OBJECT  * devObjPtr,
    IN GT_PTR                   memPointer
)
{
    SKERNEL_DEVICE_MEMORY_ALLOCATION_STC    *elementPtr;

    if(memPointer == NULL)
    {
        skernelFatalError("smemDeviceObjMemoryPtrMemSetZero : try to memset NULL pointer \n");
        return ;
    }

    if(devObjPtr == NULL)
    {
        skernelFatalError("smemDeviceObjMemoryPtrMemSetZero : can not manage memory that is not attached to device \n");
        return ;
    }

    SCIB_SEM_TAKE;

    elementPtr = smemDeviceObjMemoryFind(devObjPtr,memPointer);
    if(elementPtr == NULL)
    {
        /* fatal error already asserted */
        SCIB_SEM_SIGNAL;
        return;
    }

    /* NOTE that : elementPtr->myMemoryPtr == memPointer !!! */

    /* memset to 0 the memory according to it's length */
    memset(elementPtr->myMemoryPtr,0,elementPtr->myMemoryNumOfBytes);

    SCIB_SEM_SIGNAL;

    return;
}

/**
* @internal smemDeviceObjMemoryAllAllocationsSum function
* @endinternal
*
* @brief   debug function to print the sum of all allocations done for a device
*         allocations that done by smemDeviceObjMemoryAlloc__internal or
*         smemDeviceObjMemoryRealloc__internal
* @param[in] devObjPtr                - pointer to device object
*                                       none.
*/
void smemDeviceObjMemoryAllAllocationsSum
(
    IN SKERNEL_DEVICE_OBJECT  * devObjPtr
)
{
    SKERNEL_DEVICE_MEMORY_ALLOCATION_STC *currentPtr;
    GT_U32      totalNumBytes = 0;
    GT_U32      totalNumElements = 0;
    GT_U32      overheadNumBytes = sizeof(SKERNEL_DEVICE_MEMORY_ALLOCATION_STC);

    if(devObjPtr == NULL)
    {
        /* already did fatal error */
        return ;
    }

    SCIB_SEM_TAKE;

    currentPtr = &devObjPtr->myDeviceAllocations;

    do
    {
        totalNumBytes += currentPtr->myMemoryNumOfBytes;
        totalNumElements ++;

        /* update the 'current' to be 'next' */
        currentPtr = currentPtr->nextElementPtr;
    }while(currentPtr);

    SCIB_SEM_SIGNAL;

    /* total overhead for management is the STC of SKERNEL_DEVICE_MEMORY_ALLOCATION_STC */
    overheadNumBytes *= (totalNumElements - 1);/* minus 1 because first element is not dynamic allocated */

    simGeneralPrintf("device[%d] did dynamic allocated of [%d] bytes  (for usage), in [%d] elements , additional management \n",
        devObjPtr->deviceId,totalNumBytes,totalNumElements);
    simGeneralPrintf("and additional dynamic allocated [%d] bytes for the 'allocations' management  \n" ,
        overheadNumBytes);
    return;

}

/**
* @internal smemConvertDevAndAddrToNewDevAndAddr function
* @endinternal
*
* @brief   Convert (dev,address) to new (dev,address).
*         needed for multi-pipe device.
* @param[in] devObjPtr                - pointer to device object.
* @param[in] address                  -  of memory(register or table).
* @param[in] accessType               - the access type
*                                       None
*
* @note function MUST be called before calling smemFindMemory()
*
*/
void smemConvertDevAndAddrToNewDevAndAddr
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32                  address,
    IN SCIB_MEMORY_ACCESS_TYPE accessType,
    OUT SKERNEL_DEVICE_OBJECT * *newDevObjPtrPtr,
    OUT GT_U32                  *newAddressPtr
)
{
    SMEM_CHT_DEV_COMMON_MEM_INFO  * commonDevMemInfoPtr;

    if(!devObjPtr->isWmDxDevice)
    {
        /* no conversion needed */
        *newDevObjPtrPtr = devObjPtr;
        *newAddressPtr = address;
        return;
    }

    commonDevMemInfoPtr = devObjPtr->deviceMemory;

    if(NULL == commonDevMemInfoPtr->smemConvertDevAndAddrToNewDevAndAddrFunc)
    {
        /* no conversion needed */
        *newDevObjPtrPtr = devObjPtr;
        *newAddressPtr = address;
        return;
    }

    /* call the specific function of the device to convert device,address */
    commonDevMemInfoPtr->smemConvertDevAndAddrToNewDevAndAddrFunc(
        devObjPtr,address,accessType,newDevObjPtrPtr,newAddressPtr);

    if((*newDevObjPtrPtr) == NULL)
    {
        skernelFatalError("smemConvertDevAndAddrToNewDevAndAddr : NULL 'converted' device \n");
    }

    return;
}

/**
* @internal smemConvertGlobalPortToCurrentPipeId function
* @endinternal
*
* @brief   Convert 'global port' of DMA/GOP local port and pipe-id.
*         NOTE: the pipe-id is saved as 'currentPipeId' in 'SKERNEL_TASK_COOKIE_INFO_STC'
*         info 'per task'
*         needed for multi-pipe device.
*         NOTE: if the 'global port' found as 'GOP port' the convert is to 'local GOP port'
*         if the 'global port' found as 'DMA port' the convert is to 'local DMA port' in it's DP[]
* @param[in] devObjPtr                - pointer to device object.
* @param[in] globalPortNum            - global port number of the device.
*                                       None
*
* @note function MUST be called when packet ingress the device.
*
*/
void smemConvertGlobalPortToCurrentPipeId
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32                  globalPortNum,
    OUT GT_U32                 *localPortNumberPtr
)
{
    DECLARE_FUNC_NAME(smemConvertGlobalPortToCurrentPipeId);

    GT_U32  currentPipeId = 0;
    GT_U32  localPort = 0;
    GT_U32  index;
    GT_BIT  found = 0;
    GT_BIT  foundByDma = 0,foundInMirroedPipe = 0;
    GT_BIT  dpIndex = 0;
    GT_U32  mirroredPipeId = 0,tileId = 0;

    if(devObjPtr->numOfPipes == 0)
    {
        skernelFatalError("smemConvertGlobalPortToCurrentPipeId : only for multi-pipe \n");
    }

    if(devObjPtr->gop_specialPortMappingArr)
    {
        /* check for 'special' port numbers */
        for (index = 0 ; devObjPtr->gop_specialPortMappingArr[index].globalPortNumber != SMAIN_NOT_VALID_CNS ; index++)
        {
            if(devObjPtr->gop_specialPortMappingArr[index].globalPortNumber == globalPortNum)
            {
                localPort     = devObjPtr->gop_specialPortMappingArr[index].localPortNumber;
                currentPipeId = devObjPtr->gop_specialPortMappingArr[index].unitIndex;

                found = 1;
                break;
            }
        }

    }

    if(found ==0)
    {
        if(devObjPtr->dma_specialPortMappingArr)
        {
            /* check for 'special' DMA port numbers */
            for (index = 0 ; devObjPtr->dma_specialPortMappingArr[index].globalPortNumber != SMAIN_NOT_VALID_CNS ; index++)
            {
                if(devObjPtr->dma_specialPortMappingArr[index].globalPortNumber == globalPortNum)
                {
                    localPort = devObjPtr->dma_specialPortMappingArr[index].localPortNumber;
                    dpIndex = devObjPtr->dma_specialPortMappingArr[index].unitIndex;
                    currentPipeId = dpIndex /
                                    devObjPtr->multiDataPath.maxDp;/* 3 in BC3 (per pipe) */

                    found = 1;
                    foundByDma = 1;
                    break;
                }
            }
        }
    }


    if(found ==0)
    {
        /* update the src port (relative to the ports of the pipe) */
        localPort       = globalPortNum % devObjPtr->numOfPortsPerPipe;
        currentPipeId   = globalPortNum / devObjPtr->numOfPortsPerPipe;

        if(devObjPtr->numOfTiles && currentPipeId >= devObjPtr->numOfPipesPerTile)
        {
            tileId  = currentPipeId / devObjPtr->numOfPipesPerTile;
            if((1<<tileId) & devObjPtr->mirroredTilesBmp)
            {
                 dpIndex = smemMultiDpUnitIndexGet(devObjPtr,globalPortNum);
                 mirroredPipeId = (currentPipeId & 0xFFFFFFFE) + (1 & (currentPipeId ^ 1));
                 foundInMirroedPipe = 1;

                if(localPort >= devObjPtr->numOfPortsPerPipe)
                {
                    skernelFatalError("smemConvertGlobalPortToCurrentPipeId : localPort[%d] >= numOfPortsPerPipe[%d] \n",
                        localPort,devObjPtr->numOfPortsPerPipe);
                }
            }
        }
    }

    if(foundInMirroedPipe)
    {
        __LOG(("Converted global GOP port [%d] to Global pipe[%d] local GOP port[%d](in the pipe) (Tile[%d] , local pipeId[%d] , local DP in tile[%d])\n",
            globalPortNum , currentPipeId , localPort , tileId ,
            (mirroredPipeId % devObjPtr->numOfPipesPerTile) ,
            dpIndex % (devObjPtr->numOfPipesPerTile * devObjPtr->multiDataPath.maxDp)));
    }
    else
    if(foundByDma == 0)
    {
        __LOG(("Converted global GOP port [%d] to Global pipe[%d] local GOP port[%d](in the pipe) \n",
            globalPortNum , currentPipeId , localPort));
    }
    else
    {
        __LOG(("Converted global DMA port [%d] to Global pipe[%d] local DMA port[%d] (in DP[%d]) \n",
            globalPortNum , currentPipeId , localPort , dpIndex));
    }

    if(currentPipeId >= devObjPtr->numOfPipes)
    {
        skernelFatalError("smemConvertGlobalPortToCurrentPipeId : currentPipeId[%d] >= numOfPipes[%d] \n",
            currentPipeId,devObjPtr->numOfPipes);
    }

    if(localPortNumberPtr)
    {
        *localPortNumberPtr = localPort;
    }

    if(simOsTaskOwnTaskCookieGet() == NULL)
    {
        /* avoid fatal error inside smemSetCurrentPipeId(...)*/

        /* the 'startSimulationLog' may get here when called from 'aplication' .
          when it is doing :
            simLogSlanCatchUp();
            simLogLinkStateCatchUp();
                and calling for all ports : snetLinkStateNotify()
        */
        /*skernelFatalError("smemConvertGlobalPortToCurrentPipeId : not registered task ?! \n");*/
        return;
    }

    /* the function may update more than only 'currentPipeId' ! */
    smemSetCurrentPipeId(devObjPtr,currentPipeId);

    return;
}


/**
* @internal smemConvertCurrentPipeIdAndLocalPortToGlobal function
* @endinternal
*
* @brief   Convert (currentPipeId,port) of local port on pipe to (dev,port) of 'global'.
*         NOTE: the pipe-id is taken from 'currentPipeId' in 'SKERNEL_TASK_COOKIE_INFO_STC'
*         info 'per task'
*         needed for multi-pipe device.
* @param[in] devObjPtr                - pointer to device object of the pipe.
* @param[in] localPortNum             - local port number of the pipe.
* @param[in] allowNotValidPort        - allow not valid port.
*                                      when GT_TRUE - if port not valid --> no fatal error.
*                                      globalPortNum converted to 'SMAIN_NOT_VALID_CNS'
*                                      when GT_FALSE - if port not valid --> fatal error.
*                                       None
*
* @note function MUST be called before packet egress the device.
*
*/
void smemConvertCurrentPipeIdAndLocalPortToGlobal
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32                  localPortNum,
    IN GT_BOOL                 allowNotValidPort,
    OUT GT_U32                 *globalPortNumPtr
)
{
    DECLARE_FUNC_NAME(smemConvertCurrentPipeIdAndLocalPortToGlobal);

    SIM_OS_TASK_COOKIE_INFO_STC  *myTaskInfoPtr;
    GT_U32  currentPipeId;
    GT_U32  globalPort = 0;
    GT_U32  index;
    GT_BIT  found = 0;

    if(devObjPtr->numOfPipes == 0)
    {
        skernelFatalError("smemConvertCurrentPipeIdAndLocalPortToGlobal : only for multi-pipe \n");
    }

    currentPipeId = smemGetCurrentPipeId(devObjPtr);

    if(devObjPtr->gop_specialPortMappingArr)
    {
        /* check for 'special' port numbers */
        for (index = 0 ; devObjPtr->gop_specialPortMappingArr[index].globalPortNumber != SMAIN_NOT_VALID_CNS ; index++)
        {
            if((devObjPtr->gop_specialPortMappingArr[index].localPortNumber == localPortNum) &&
               (devObjPtr->gop_specialPortMappingArr[index].unitIndex       == currentPipeId))
            {
                globalPort = devObjPtr->gop_specialPortMappingArr[index].globalPortNumber;

                found = 1;
                break;
            }
        }
    }

    if(found ==0)
    {
        /* update the port (relative to the ports of the pipe) */
        globalPort = localPortNum + (devObjPtr->numOfPortsPerPipe) * currentPipeId;
    }

    myTaskInfoPtr = simOsTaskOwnTaskCookieGet();
    if(myTaskInfoPtr && myTaskInfoPtr->additionalInfo)
    {
        __LOG(("Converted port [%d] of pipe[%d] to global port[%d] \n",
            localPortNum , currentPipeId , globalPort));
    }
    else
    {
        /* avoid many calls from the enh-UT in test contexts that busy wait on MAC counters :
            taskType[ SIM_OS_TASK_PURPOSE_TYPE_CPU_APPLICATION_E ]
            dev[0] core[0] func[smemConvertCurrentPipeIdAndLocalPortToGlobal] file[smem.c]
            Converted port [22] of pipe[1] to global port[58]
        */
    }

    if(currentPipeId >= devObjPtr->numOfPipes)
    {
        skernelFatalError("smemConvertCurrentPipeIdAndLocalPortToGlobal : pipe device not found ?! \n");
    }

    if(globalPort > devObjPtr->portsNumber)
    {
        if(allowNotValidPort == GT_FALSE)
        {
            skernelFatalError("smemConvertCurrentPipeIdAndLocalPortToGlobal : global Port [%d] out of range ?! \n",globalPort);
        }
        else
        {
            /* indicate the caller that 'not valid port' with in a valid pipe */
            globalPort = SMAIN_NOT_VALID_CNS;
        }
    }

    *globalPortNumPtr = globalPort;

    return;
}

/**
* @internal smemSetCurrentPipeId function
* @endinternal
*
* @brief   set current pipe-id (saved as 'currentPipeId' in 'SKERNEL_TASK_COOKIE_INFO_STC'
*         info 'per task')
*         needed for multi-pipe device.
* @param[in] devObjPtr                - pointer to device object.
* @param[in] currentPipeId            - the current pipeId to set.
*                                       None
*/
void smemSetCurrentPipeId
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32                  currentPipeId
)
{
    SIM_OS_TASK_COOKIE_INFO_STC  *myTaskInfoPtr;

    if(devObjPtr->numOfPipes < 2)
    {
        /***********************************/
        /* by default set also the MG unit */
        /***********************************/
        if(devObjPtr->numOfMgUnits >= 2)
        {
            smemSetCurrentMgUnitIndex_withRelativeMgToTile(devObjPtr,0);/* MG 0 of the tile */
        }

        /* support non multi-pipe devices */
        return;
    }

    if(currentPipeId >= devObjPtr->numOfPipes)
    {
        skernelFatalError("smemSetCurrentPipeId : currentPipeId[%d] >= numOfPipes[%d] \n",
            currentPipeId,devObjPtr->numOfPipes);
    }

    myTaskInfoPtr = simOsTaskOwnTaskCookieGet();
    if(myTaskInfoPtr == NULL)
    {
        skernelFatalError("smemSetCurrentPipeId : not registered task ?! \n");
    }

    if(myTaskInfoPtr->additionalInfo &&
        (myTaskInfoPtr->currentPipeId != currentPipeId))
    {
        __LOG_NO_LOCATION_META_DATA(("Change currentPipeId from [%d] to [%d] \n" ,
            myTaskInfoPtr->currentPipeId,
            currentPipeId));
    }

    myTaskInfoPtr->currentPipeId = currentPipeId;

    /***********************************/
    /* by default set also the MG unit */
    /***********************************/
    if(devObjPtr->numOfMgUnits >= 2)
    {
        smemSetCurrentMgUnitIndex_withRelativeMgToTile(devObjPtr,0);/* MG 0 of the tile */
    }


    return;
}

static void defaultRegisteringCurrTask(void)
{
    /* just bind it to allow access to needed info */
    SIM_OS_MAC(simOsTaskOwnTaskPurposeSet)(SIM_OS_TASK_PURPOSE_TYPE_PP_PIPE_GENERAL_PURPOSE_E,NULL);
}

/**
* @internal smemGetCurrentPipeId function
* @endinternal
*
* @brief   get current pipe-id (saved as 'currentPipeId' in 'SKERNEL_TASK_COOKIE_INFO_STC'
*         info 'per task')
*         needed for multi-pipe device.
* @param[in] devObjPtr                - pointer to device object.
*
* @retval currentPipeId            - the current pipeId to set.
*/
GT_U32 smemGetCurrentPipeId
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr
)
{
    SIM_OS_TASK_COOKIE_INFO_STC  *myTaskInfoPtr;

    if(devObjPtr->numOfPipes < 2)
    {
        return 0;
    }

    myTaskInfoPtr = simOsTaskOwnTaskCookieGet();
    if(myTaskInfoPtr == NULL)
    {
        defaultRegisteringCurrTask();
        myTaskInfoPtr = simOsTaskOwnTaskCookieGet();
        if(myTaskInfoPtr == NULL)
        {
            skernelFatalError("smemGetCurrentPipeId : not registered task ?! \n");
        }
    }

    return myTaskInfoPtr->currentPipeId;
}

/**
* @internal smemSetCurrentMgUnitIndex function
* @endinternal
*
* @brief   set current MG unit Index (saved as 'currentMgUnitIndex' in 'SKERNEL_TASK_COOKIE_INFO_STC'
*         info 'per task')
*         needed for multi-pipe device.
* @param[in] devObjPtr                - pointer to device object.
* @param[in] currentMgUnitIndex       - the current MG unit to set.
* @retval  -  None
*/
void smemSetCurrentMgUnitIndex
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32                  currentMgUnitIndex
)
{
    SIM_OS_TASK_COOKIE_INFO_STC  *myTaskInfoPtr;

    if(devObjPtr->numOfMgUnits < 2)
    {
        /* support non multi-MG devices */
        return;
    }

    if(currentMgUnitIndex >= devObjPtr->numOfMgUnits)
    {
        skernelFatalError("smemSetCurrentMgUnitIndex : currentMgUnitIndex[%d] >= numOfMgUnits[%d] \n",
            currentMgUnitIndex,devObjPtr->numOfMgUnits);
    }

    myTaskInfoPtr = simOsTaskOwnTaskCookieGet();
    if(myTaskInfoPtr == NULL)
    {
        defaultRegisteringCurrTask();
        myTaskInfoPtr = simOsTaskOwnTaskCookieGet();
        if(myTaskInfoPtr == NULL)
        {
            skernelFatalError("smemSetCurrentPipeId : not registered task ?! \n");
        }
    }

    if(myTaskInfoPtr->additionalInfo &&
        (myTaskInfoPtr->currentMgUnitIndex != currentMgUnitIndex))
    {
        __LOG_NO_LOCATION_META_DATA(("Change currentMgUnitIndex from [%d] to [%d] \n" ,
            myTaskInfoPtr->currentMgUnitIndex,
            currentMgUnitIndex));
    }
    /*NOTE: the MnG unit not subject to mirroring because it is single in the 2 units */
    myTaskInfoPtr->currentMgUnitIndex = currentMgUnitIndex;

    return;
}

/**
* @internal smemGetCurrentMgUnitIndex function
* @endinternal
*
* @brief   get current MG unit Index (saved as 'currentMgUnitIndex' in 'SKERNEL_TASK_COOKIE_INFO_STC'
*         info 'per task')
*         needed for multi-MG units device.
* @param[in] devObjPtr                - pointer to device object.
*
* @retval currentMgUnitIndex          - the current MG unit to set.
*/
GT_U32 smemGetCurrentMgUnitIndex
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr
)
{
    SIM_OS_TASK_COOKIE_INFO_STC  *myTaskInfoPtr;

    if(devObjPtr->numOfMgUnits < 2)
    {
        return 0;
    }

    myTaskInfoPtr = simOsTaskOwnTaskCookieGet();
    if(myTaskInfoPtr == NULL)
    {
        defaultRegisteringCurrTask();
        myTaskInfoPtr = simOsTaskOwnTaskCookieGet();
        if(myTaskInfoPtr == NULL)
        {
            skernelFatalError("smemGetCurrentMgUnitIndex : not registered task ?! \n");
        }
    }

    return myTaskInfoPtr->currentMgUnitIndex;
}

/**
* @internal smemSetCurrentMgUnitIndex_withRelativeMgToTile function
* @endinternal
*
* @brief   set current MG unit Index (saved as 'currentMgUnitIndex' in 'SKERNEL_TASK_COOKIE_INFO_STC'
*         info 'per task')
*         needed for multi-pipe device.
* @param[in] devObjPtr                - pointer to device object.
* @param[in] currentMgUnitIndex_relativeToTile  - the current MG unit to set (relative to 'current tile').
* @retval  -  None
*/
void smemSetCurrentMgUnitIndex_withRelativeMgToTile
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32                  currentMgUnitIndex_relativeToTile
)
{
    GT_U32  globalMgUnit,currentPipeId,tileId;
    if(devObjPtr->numOfTiles)
    {
        currentPipeId = smemGetCurrentPipeId(devObjPtr);
        smemConvertGlobalPipeIdToTileAndLocalPipeIdInTile(devObjPtr,currentPipeId,&tileId);
        globalMgUnit = ((tileId * devObjPtr->numOfMgUnits)/devObjPtr->numOfTiles) + currentMgUnitIndex_relativeToTile;

        /*NOTE: the MnG unit not subject to mirroring because it is single in the 2 units */
    }
    else
    {
        globalMgUnit = currentMgUnitIndex_relativeToTile;
    }

    smemSetCurrentMgUnitIndex(devObjPtr,globalMgUnit);
}

/**
* @internal smemSetCurrentMgUnitIndex_byRegAddr function
* @endinternal
*
* @brief   set current MG unit Index according to globalRegAddr
*         needed for multi-pipe device.
* @param[in] devObjPtr      - pointer to device object.
* @param[in] globalRegAddr  - the address of register.(relevant only if the register is one of MG register)
* @retval  -  None
*/
void smemSetCurrentMgUnitIndex_byRegAddr
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32                  globalRegAddr
)
{
    GT_U32  globalMgUnit;

    if(devObjPtr->numOfMgUnits < 2)
    {
        return;
    }

    if(devObjPtr->devMemGetMgUnitIndexFromAddressPtr)
    {
        globalMgUnit = devObjPtr->devMemGetMgUnitIndexFromAddressPtr(devObjPtr,globalRegAddr);
        if(globalMgUnit >= devObjPtr->numOfMgUnits)
        {
            /* we got SMAIN_NOT_VALID_CNS , not MG register */
            return;
        }
    }
    else
    {
        /* not implemented ... the MG1,2,3 are not different memory space */
        return;
    }

    smemSetCurrentMgUnitIndex(devObjPtr,globalMgUnit);
}

/**
* @internal smemSetCurrentTaskExtParamValue function
* @endinternal
*
* @brief   set value at Index (saved as 'extParamArr[index]=value' in 'SKERNEL_TASK_COOKIE_INFO_STC'
*         info 'per task')
*         needed for multi-unit device.
* @param[in] devObjPtr  - pointer to device object.
* @param[in] index      - the index in which to set the value.
* @param[in] value      - the value to set. (at the index)
* @retval  -  None
*/
void smemSetCurrentTaskExtParamValue
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32                  index,
    IN GT_U32                  value
)
{
    SIM_OS_TASK_COOKIE_INFO_STC  *myTaskInfoPtr;

    if(index >= SIM_OS_TASK_COOKIE_EXT_PARAM_MAX_NUM_CNS)
    {
        skernelFatalError("smemSetCurrentTaskExtParamValue : index[%d] >= SIM_OS_TASK_COOKIE_EXT_PARAM_MAX_NUM_CNS[%d] \n",
            index,SIM_OS_TASK_COOKIE_EXT_PARAM_MAX_NUM_CNS);
    }

    myTaskInfoPtr = simOsTaskOwnTaskCookieGet();
    if(myTaskInfoPtr == NULL)
    {
        defaultRegisteringCurrTask();
        myTaskInfoPtr = simOsTaskOwnTaskCookieGet();
        if(myTaskInfoPtr == NULL)
        {
            skernelFatalError("smemSetCurrentTaskExtParamValue : not registered task ?! \n");
        }
    }

    if(myTaskInfoPtr->additionalInfo &&
        (myTaskInfoPtr->extParamArr[index] != value))
    {
        __LOG_NO_LOCATION_META_DATA(("Change myTaskInfoPtr->extParamArr[%d] from [%d] to [%d] \n" ,
            index,
            myTaskInfoPtr->extParamArr[index],
            value));
    }

    myTaskInfoPtr->extParamArr[index] = value;

    return;
}

/**
* @internal smemGetCurrentTaskExtParamValue function
* @endinternal
*
* @brief   get value at Index (was saved as 'extParamArr[index]=value' in 'SKERNEL_TASK_COOKIE_INFO_STC'
*         info 'per task')
*         needed for multi-unit device.
* @param[in] devObjPtr  - pointer to device object.
* @param[in] index      - the index in which to set the value.
* @retval  -  value (that was set by smemSetCurrentTaskExtParamValue)
*/
GT_U32 smemGetCurrentTaskExtParamValue
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32                  index
)
{
    SIM_OS_TASK_COOKIE_INFO_STC  *myTaskInfoPtr;

    if(index >= SIM_OS_TASK_COOKIE_EXT_PARAM_MAX_NUM_CNS)
    {
        skernelFatalError("smemGetCurrentTaskExtParamValue : index[%d] >= SIM_OS_TASK_COOKIE_EXT_PARAM_MAX_NUM_CNS[%d] \n",
            index,SIM_OS_TASK_COOKIE_EXT_PARAM_MAX_NUM_CNS);
    }

    myTaskInfoPtr = simOsTaskOwnTaskCookieGet();
    if(myTaskInfoPtr == NULL)
    {
        defaultRegisteringCurrTask();
        myTaskInfoPtr = simOsTaskOwnTaskCookieGet();
        if(myTaskInfoPtr == NULL)
        {
            skernelFatalError("smemGetCurrentTaskExtParamValue : not registered task ?! \n");
        }
    }

    return myTaskInfoPtr->extParamArr[index];
}

/**
* @internal smemRestCurrentTaskExtParamValues function
* @endinternal
*
* @brief   reset all values at all Indexes (saved as 'extParamArr[index]=value' in 'SKERNEL_TASK_COOKIE_INFO_STC'
*         info 'per task')
*         needed for multi-unit device.
* @param[in] devObjPtr  - pointer to device object.
* @retval  -  None
*/
void smemRestCurrentTaskExtParamValues(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr
)
{
    SIM_OS_TASK_COOKIE_INFO_STC  *myTaskInfoPtr;
    GT_U32  index;

    myTaskInfoPtr = simOsTaskOwnTaskCookieGet();
    if(myTaskInfoPtr == NULL)
    {
        defaultRegisteringCurrTask();
        myTaskInfoPtr = simOsTaskOwnTaskCookieGet();
        if(myTaskInfoPtr == NULL)
        {
            skernelFatalError("smemSetCurrentTaskExtParamValue : not registered task ?! \n");
        }
    }

    for(index = 0;index < SIM_OS_TASK_COOKIE_EXT_PARAM_MAX_NUM_CNS; index++)
    {
        if(myTaskInfoPtr->additionalInfo &&
            (myTaskInfoPtr->extParamArr[index] != 0))
        {
            __LOG_NO_LOCATION_META_DATA(("Change myTaskInfoPtr->extParamArr[%d] from [%d] to [%d] \n" ,
                index,
                myTaskInfoPtr->extParamArr[index],
                0));
        }

        myTaskInfoPtr->extParamArr[index] = 0;
    }
}

/**
* @internal smemConvertGlobalPipeIdToTileAndLocalPipeIdInTile function
* @endinternal
*
* @brief   convert global pipe-id to (tile-Id , local pipeIdInTile
*         needed for multi-tile device.
* @param[in] devObjPtr                - pointer to device object.
* @param[in] globalPipeId             - the global pipeId to convert.
*
* @param[out] tileIdPtr                - the Tile Id.
*                                      (ignored if NULL).
*
* @retval localPipeIdInTile        - the local pipeId in the Tile.
*/
GT_U32 smemConvertGlobalPipeIdToTileAndLocalPipeIdInTile
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32                globalPipeId,
    OUT GT_U32               *tileIdPtr
)
{
    GT_U32  localPipeIdInTile;
    GT_U32  tileId;

    if(devObjPtr->numOfTiles < 2)
    {
        /* no convert needed */
        if(tileIdPtr)
        {
            *tileIdPtr = 0;
        }
        return globalPipeId;
    }

    tileId = globalPipeId / devObjPtr->numOfPipesPerTile;

    if(tileIdPtr)
    {
        *tileIdPtr = tileId;
    }

    localPipeIdInTile = globalPipeId % devObjPtr->numOfPipesPerTile;
    if((1 << tileId) & devObjPtr->mirroredTilesBmp)
    {
        /* mirror the local pipeId */
        localPipeIdInTile = (devObjPtr->numOfPipesPerTile - 1) - localPipeIdInTile;
    }

    return localPipeIdInTile;
}

/**
* @internal smemConvertTileAndLocalPipeIdInTileToGlobalPipeId function
* @endinternal
*
* @brief   convert (tile-Id , local pipeIdInTile) to global pipe-id .
*         needed for multi-tile device.
* @param[in] devObjPtr                - pointer to device object.
* @param[in] tileId                   - the Tile Id.
* @param[in] localPipeIdInTile        - the local pipeId in the Tile.
*
* @retval globalPipeId             - the global pipeId to convert.
*/
GT_U32 smemConvertTileAndLocalPipeIdInTileToGlobalPipeId
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32                tileId,
    IN GT_U32                localPipeIdInTile
)
{
    GT_U32  globalPipeId;

    if(devObjPtr->numOfTiles < 2)
    {
        /* no convert needed */
        return localPipeIdInTile;
    }

    globalPipeId = devObjPtr->numOfPipesPerTile * tileId;

    if(tileId >= devObjPtr->numOfTiles)
    {
        skernelFatalError("smemConvertTileAndLocalPipeIdInTileToGlobalPipeId :tileId not valid ?! \n");
    }

    if(localPipeIdInTile >= devObjPtr->numOfPipesPerTile)
    {
        skernelFatalError("smemConvertTileAndLocalPipeIdInTileToGlobalPipeId :local pipeId not valid ?! \n");
    }

    if((1 << (tileId)) & devObjPtr->mirroredTilesBmp)
    {
        /* mirror the local pipeId */
        globalPipeId += (devObjPtr->numOfPipesPerTile - 1) - localPipeIdInTile;
    }
    else
    {
        globalPipeId += localPipeIdInTile;
    }

    return globalPipeId;
}

/**
* @internal smemConvertPipeIdAndLocalPortToGlobal_withPipeAndDpIndex function
* @endinternal
*
* @brief   Convert (PipeId,dpIndex,localPort) of local port on DP to (dev,port) of 'global'.
*         needed for multi-pipe device.
* @param[in] devObjPtr                - pointer to device object of the pipe.
* @param[in] pipeIndex                - pipeIndex
* @param[in] dpIndex                  -  (relative to pipe)
* @param[in] localPortNum             - local port number of the DP.
* @param[in] isCpuSdma                - indication that the local port is 'CPU SDMA' or 'CPU networt'
*                                       relevant only to DMAs that hold 'mux' for the 'CPU port'
*  None
*/
void smemConvertPipeIdAndLocalPortToGlobal_withPipeAndDpIndex
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32                  pipeIndex,
    IN GT_U32                  dpIndex,
    IN GT_U32                  localPortNum,
    IN GT_BOOL                 isCpuSdma,
    OUT GT_U32                 *globalPortNumPtr
)
{
    DECLARE_FUNC_NAME(smemConvertPipeIdAndLocalPortToGlobal_withPipeAndDpIndex);

    GT_U32  globalPort = 0;
    DATA_PATH_INFO_STC  *dpInfoPtr;
    GT_U32  index;
    GT_BIT  found = 0;
    GT_U32  global_dpIndex;/* the dpIndex is given */

    if(devObjPtr->numOfPipes || dpIndex >= SIM_MAX_DP_CNS)
    {
        if(pipeIndex >= devObjPtr->numOfPipes || dpIndex >= SIM_MAX_DP_CNS)
        {
            skernelFatalError("smemConvertPipeIdAndLocalPortToGlobal_withPipeAndDpIndex : only for multi-pipe \n");
        }
    }



    if (devObjPtr->dma_specialPortMappingArr)
    {
        global_dpIndex = dpIndex + pipeIndex * devObjPtr->multiDataPath.maxDp;

        /* check for 'special' port numbers */
        for (index = 0 ; devObjPtr->dma_specialPortMappingArr[index].globalPortNumber != SMAIN_NOT_VALID_CNS ; index++)
        {
            if(devObjPtr->dma_specialPortMappingArr[index].localPortNumber == localPortNum &&
               devObjPtr->dma_specialPortMappingArr[index].unitIndex == global_dpIndex)
            {
                globalPort = devObjPtr->dma_specialPortMappingArr[index].globalPortNumber;

                if(smemMultiDpUnitIsCpuPortGet(devObjPtr,globalPort))
                {
                    if(smemMultiDpUnitIsCpuSdmaPort_notConnected_Get(devObjPtr,globalPort))
                    {
                        /* SDMA that is not connected */
                        /* so 'not found'  ... regardless to 'sdma/not' */
                    }
                    else
                    {
                        /* SDMA that is connected or 'NETWORK CPU' */
                        if(isCpuSdma && smemMultiDpUnitIsCpuSdmaPortGet(devObjPtr,globalPort,NULL))
                        {
                            /* SDMA that is connected */
                            /* the port found to be SDMA CPU port */
                            found = 1;
                        }
                        else if(!isCpuSdma)
                        {
                            /* the port found */
                            /* 'NETWORK CPU' */
                            found = 1;
                        }
                    }
                }
                else
                {
                    found = 1;
                }

                if(found)
                {
                    break;
                }
            }
        }
    }

    if(found == 0)
    {
        dpInfoPtr = &devObjPtr->multiDataPath.info[dpIndex];

        if(localPortNum >= dpInfoPtr->dataPathNumOfPorts)
        {
            skernelFatalError("smemConvertPipeIdAndLocalPortToGlobal_withPipeAndDpIndex : local port can't belong to DP \n");
        }

        globalPort = pipeIndex * devObjPtr->numOfPortsPerPipe + dpInfoPtr->dataPathFirstPort + localPortNum;
    }

    __LOG(("Converted local port [%d] of relative DP[%d] of pipe[%d] to global port[%d]  \n",
        localPortNum , dpIndex , pipeIndex , globalPort));

    *globalPortNumPtr = globalPort;

    return;
}

/**
* @internal smemMultiDpUnitIndexGet function
* @endinternal
*
* @brief   Get DP unit according to the port number.
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] portNum                  - the port number
*                                       return the index of the DP unit
*/
GT_U32 smemMultiDpUnitIndexGet
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32   portNum
)
{
    DATA_PATH_INFO_STC  *dpInfoPtr;
    GT_U32  index;
    GT_U32  dataPathFirstPort,dataPathNumOfPorts;
    GT_U32  numOfPipes = devObjPtr->numOfPipes ? devObjPtr->numOfPipes : 1;

    if(devObjPtr->dma_specialPortMappingArr)
    {
        /* check for 'special' port numbers */

        for (index = 0 ; devObjPtr->dma_specialPortMappingArr[index].globalPortNumber != SMAIN_NOT_VALID_CNS ; index++)
        {
            if(devObjPtr->dma_specialPortMappingArr[index].globalPortNumber == portNum)
            {
                return devObjPtr->dma_specialPortMappingArr[index].unitIndex;
            }
        }
    }

    dpInfoPtr = &devObjPtr->multiDataPath.info[0];
    for (index = 0 ; index <  (devObjPtr->multiDataPath.maxDp * numOfPipes); index++ ,dpInfoPtr++)
    {
        dataPathFirstPort = dpInfoPtr->dataPathFirstPort;
        dataPathNumOfPorts = dpInfoPtr->dataPathNumOfPorts;

        if(dataPathNumOfPorts == 0)
        {
            /* this DP is ignored ... not valid */
            continue;
        }

        if(((dataPathFirstPort + dataPathNumOfPorts) > portNum) &&
            (dataPathFirstPort <= portNum) )
        {
            /* the port is in the range */
            return index;
        }
    }

    /* not found ?! */
    skernelFatalError("smemMultiDpUnitIndexGet: portNum[%d] is not matched by any 'DP unit' ?! \n",
        portNum);
    return 0;/* dummy just for compiler */
}


/**
* @internal smemMultiDpUnitRelativePortGet function
* @endinternal
*
* @brief   Get DP local port number according to the global port number.
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] portNum                  - the global port number (in the Pipe)
*                                       return the local port number
*/
GT_U32 smemMultiDpUnitRelativePortGet
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32   portNum
)
{
    DATA_PATH_INFO_STC  *dpInfoPtr;
    GT_U32  index;
    GT_U32  dataPathFirstPort,dataPathNumOfPorts;
    GT_U32 *localDmaMapArr; /* local DMAs in specific DP, NULL in cases 1:1 mapping */
    GT_U32  numOfPipes = devObjPtr->numOfPipes ? devObjPtr->numOfPipes : 1;

    if(devObjPtr->dma_specialPortMappingArr)
    {
        /* check for 'special' port numbers */

        for (index = 0 ; devObjPtr->dma_specialPortMappingArr[index].globalPortNumber != SMAIN_NOT_VALID_CNS ; index++)
        {
            if(devObjPtr->dma_specialPortMappingArr[index].globalPortNumber == portNum)
            {
                return devObjPtr->dma_specialPortMappingArr[index].localPortNumber;
            }
        }
    }

    dpInfoPtr = &devObjPtr->multiDataPath.info[0];
    for (index = 0 ; index < (devObjPtr->multiDataPath.maxDp * numOfPipes) ; index++ ,dpInfoPtr++)
    {
        dataPathFirstPort = dpInfoPtr->dataPathFirstPort;
        dataPathNumOfPorts = dpInfoPtr->dataPathNumOfPorts;

        if(dataPathNumOfPorts == 0)
        {
            /* this DP is ignored ... not valid */
            continue;
        }

        if(((dataPathFirstPort + dataPathNumOfPorts) > portNum) &&
            (dataPathFirstPort <= portNum) )
        {
            localDmaMapArr = dpInfoPtr->localDmaMapArr;
            if (localDmaMapArr)
            {
                /* the 'local port' number */
                return localDmaMapArr[portNum - dataPathFirstPort];
            }
            else
            {
                /* the 'local port' number */
                return portNum - dataPathFirstPort;
            }
        }
    }

    /* not found ?! */
    skernelFatalError("smemMultiDpUnitRelativePortGet: portNum[%d] is not matched by any 'DP unit' ?! \n",
        portNum);
    return 0;/* dummy just for compiler */
}

/**
* @internal isCpuSdmaPortGet function
* @endinternal
*
* @brief   check if the global DMA port is 'CPU SDMA' and return the MG unit Id
*         that connected to the DP unit (of this CPU SDMA port).
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] globalDmaNum             - the global DMA port number
* @param[out] mgUnitIndexPtr          - (pointer to) the global MG unit Id that connected to the DP unit of this CPU SDMA port
*                                       ignored if NULL
*
* return GT_TRUE  - the global DMA port is     'CPU SDMA'
*        GT_FALSE - the global DMA port is NOT 'CPU SDMA'
*/
static GT_BOOL isCpuSdmaPortGet
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32   globalDmaNum,
    IN GT_BOOL   checkForMgUnit,
    OUT GT_U32  *mgUnitIndexPtr
)
{
    GT_U32  index;
    GT_U32  single_dmaNumOfCpuPort;

    if(devObjPtr->cpuPortSdma_specialPortMappingArr == NULL)
    {
        single_dmaNumOfCpuPort = devObjPtr->dmaNumOfCpuPort ? devObjPtr->dmaNumOfCpuPort : SNET_CHT_CPU_PORT_CNS;

        /* support single CPU SDMA port */
        if (checkForMgUnit == GT_TRUE && single_dmaNumOfCpuPort)
        {
            if(checkForMgUnit && mgUnitIndexPtr)
            {
                *mgUnitIndexPtr = 0;
            }
            return GT_TRUE;
        }
        return GT_FALSE;
    }


    for (index = 0 ; devObjPtr->cpuPortSdma_specialPortMappingArr[index].globalPortNumber != SMAIN_NOT_VALID_CNS ; index++)
    {
        if(devObjPtr->cpuPortSdma_specialPortMappingArr[index].globalPortNumber == globalDmaNum)
        {
            if(checkForMgUnit == GT_TRUE && mgUnitIndexPtr)
            {
                *mgUnitIndexPtr = devObjPtr->cpuPortSdma_specialPortMappingArr[index].unitIndex;
            }

            return GT_TRUE;
        }
    }

    if(checkForMgUnit == GT_FALSE && devObjPtr->gop_specialPortMappingArr)
    {
        for (index = 0 ; devObjPtr->gop_specialPortMappingArr[index].globalPortNumber != SMAIN_NOT_VALID_CNS ; index++)
        {
            if(devObjPtr->gop_specialPortMappingArr[index].globalPortNumber == globalDmaNum)
            {
                return GT_TRUE;
            }
        }
    }

    /* not found as CPU port */
    return GT_FALSE;
}

/**
* @internal smemMultiDpUnitIsCpuSdmaPortGet function
* @endinternal
*
* @brief   check if the global DMA port is 'CPU SDMA' and return the MG unit Id
*         that connected to the DP unit (of this CPU SDMA port).
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] globalDmaNum             - the global DMA port number
* @param[out] mgUnitIndexPtr          - (pointer to) the global MG unit Id that connected to the DP unit of this CPU SDMA port
*                                       ignored if NULL
*
* return GT_TRUE  - the global DMA port is     'CPU SDMA'
*        GT_FALSE - the global DMA port is NOT 'CPU SDMA'
*/
GT_BOOL smemMultiDpUnitIsCpuSdmaPortGet
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32   globalDmaNum,
    OUT GT_U32  *mgUnitIndexPtr
)
{
    GT_U32  globalDp;
    GT_BOOL  isSdma;

    if(GT_FALSE == isCpuSdmaPortGet(devObjPtr,globalDmaNum,GT_TRUE,mgUnitIndexPtr))
    {
        return GT_FALSE;
    }

    /* the fact that a DMA is capable to be connected to the MG ... does not mean
       that the DP unit is connected to it */

    globalDp = smemMultiDpUnitIndexGet(devObjPtr,globalDmaNum);
    /* need to check if the TxFifo connected to the MG or to the GOP direction */
    isSdma = smemMultiDpUnitIsTxFifoEnableSdmaCpuPortGet(devObjPtr,globalDp);

    return isSdma;
}
/**
* @internal smemMultiDpUnitIsCpuSdmaPort_notConnected_Get function
* @endinternal
*
* @brief   check if the global DMA port is 'CPU SDMA' that is NOT connected.
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] globalDmaNum             - the global DMA port number
* @param[out] mgUnitIndexPtr          - (pointer to) the global MG unit Id that connected to the DP unit of this CPU SDMA port
*                                       ignored if NULL
*
* return GT_TRUE  - the global DMA port is     'CPU SDMA' but is not connected.
*        GT_FALSE - the global DMA port is NOT 'CPU SDMA' or is connected !
*/
GT_BOOL smemMultiDpUnitIsCpuSdmaPort_notConnected_Get
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32   globalDmaNum
)
{
    GT_U32  globalDp;
    GT_BOOL  isSdma;

    if(GT_FALSE == isCpuSdmaPortGet(devObjPtr,globalDmaNum,GT_TRUE,NULL))
    {
        /* not SDMA port */
        return GT_FALSE;
    }

    /* check if the SDMA connected to the TxFifo */

    /* the fact that a DMA is capable to be connected to the MG ... does not mean
       that the DP unit is connected to it */
    globalDp = smemMultiDpUnitIndexGet(devObjPtr,globalDmaNum);
    /* need to check if the TxFifo connected to the MG or to the GOP direction */
    isSdma = smemMultiDpUnitIsTxFifoEnableSdmaCpuPortGet(devObjPtr,globalDp);

    return (isSdma == GT_TRUE) ?
            GT_FALSE /* meaning connected */ :
            GT_TRUE  /* meaning not connected */;
}

/**
* @internal smemMultiDpUnitIsCpuPortGet function
* @endinternal
*
* @brief   check if the global DMA port is 'CPU network' or 'CPU SDMA'.
*         that connected to the DP unit (of this CPU SDMA port).
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] globalDmaNum             - the global DMA port number.
*
* return GT_TRUE  - the global DMA port is     'CPU network' or 'CPU SDMA'
*        GT_FALSE - the global DMA port is NOT 'CPU network' or 'CPU SDMA'
*/
GT_BOOL smemMultiDpUnitIsCpuPortGet
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32   globalDmaNum
)
{
    return isCpuSdmaPortGet(devObjPtr,globalDmaNum,GT_FALSE,NULL);
}

/**
* @internal smemMultiDpUnitIsTxFifoEnableSdmaCpuPortGet function
* @endinternal
*
* @brief   check if the TxFifo unit enabled for 'CPU SDMA' or for 'CPU network port'
*         that connected to the DP unit (of this CPU SDMA port).
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] globalDpIndex            - the global TxFifo unit number
*
* return GT_TRUE  - the TxFifo unit in mode :     'CPU SDMA'
*        GT_FALSE - the TxFifo unit in mode : NOT 'CPU SDMA'
*/
GT_BOOL smemMultiDpUnitIsTxFifoEnableSdmaCpuPortGet
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32   globalDpIndex
)
{
    GT_U32  value;
    GT_U32  currentPipeId = smemGetCurrentPipeId(devObjPtr);
    GT_U32  address = SMEM_SIP6_TXFIFO_GLOBAL_CONFIG_1_REG_MAC(devObjPtr,globalDpIndex);

    /* set current pipeId = 0 , because we calculate absolute address of:
        address and we not want this to be added with the base of  currentPipeId */
    smemSetCurrentPipeId(devObjPtr,0);

    /* <Enable SDMA Port>  */
    smemRegFldGet(devObjPtr,
        address,
        0,1,&value);

    /* restore the currentPipeId */
    smemSetCurrentPipeId(devObjPtr,currentPipeId);

    return value ?  GT_TRUE : GT_FALSE;
}


/**
* @internal smemMultiDpUnitFromCurrentMgGet function
* @endinternal
*
* @brief   get the DP index of the 'current MG' that is connected to.
*
* @param[in] devObjPtr                - pointer to device object.
*
* return the DP index of the 'current MG' that is connected to
*/
GT_U32 smemMultiDpUnitFromCurrentMgGet
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr
)
{
    GT_U32  index;
    GT_U32  currMgUnitId;

    if(devObjPtr->cpuPortSdma_specialPortMappingArr == NULL)
    {
        if(devObjPtr->dmaNumOfCpuPort           == 0   ||
           devObjPtr->dma_specialPortMappingArr == NULL)
        {
            return 0;/* DP[0] serves the single MG unit */
        }

        /* get the DP[] that serves the single MG unit */
        return smemMultiDpUnitIndexGet(devObjPtr,devObjPtr->dmaNumOfCpuPort);
    }

    currMgUnitId = smemGetCurrentMgUnitIndex(devObjPtr);

    for (index = 0 ; devObjPtr->cpuPortSdma_specialPortMappingArr[index].globalPortNumber != SMAIN_NOT_VALID_CNS ; index++)
    {
        if(devObjPtr->cpuPortSdma_specialPortMappingArr[index].unitIndex == currMgUnitId)
        {
            /* get the DP[] that serves the current MG unit */
            return smemMultiDpUnitIndexGet(devObjPtr,
                devObjPtr->cpuPortSdma_specialPortMappingArr[index].globalPortNumber);
        }
    }

    /* not found ?! */
    skernelFatalError("smemMultiDpUnitFromCurrentMgGet: currMgUnitId[%d] is not matched by any 'MG unit' ?! \n",currMgUnitId);
    return 0;/* dummy just for compiler */
}

/**
* @internal smemMultiDpGlobalCpuSdmaNumFromCurrentMgGet function
* @endinternal
*
* @brief   get the CPU SDMA port number from the 'current MG' that is connected to.
*
* @param[in] devObjPtr                - pointer to device object.
*
* return the CPU SDMA port number from the 'current MG' that is connected to
*/
GT_U32 smemMultiDpGlobalCpuSdmaNumFromCurrentMgGet
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr
)
{
    GT_U32  index;
    GT_U32  currMgUnitId;

    if(devObjPtr->cpuPortSdma_specialPortMappingArr == NULL)
    {
        return devObjPtr->dmaNumOfCpuPort ? devObjPtr->dmaNumOfCpuPort : SNET_CHT_CPU_PORT_CNS;
    }

    currMgUnitId = smemGetCurrentMgUnitIndex(devObjPtr);

    for (index = 0 ; devObjPtr->cpuPortSdma_specialPortMappingArr[index].globalPortNumber != SMAIN_NOT_VALID_CNS ; index++)
    {
        if(devObjPtr->cpuPortSdma_specialPortMappingArr[index].unitIndex == currMgUnitId)
        {
            return devObjPtr->cpuPortSdma_specialPortMappingArr[index].globalPortNumber;
        }
    }

    /* not found ?! */
    skernelFatalError("smemMultiDpGlobalCpuSdmaNumFromCurrentMgGet: currMgUnitId[%d] is not matched by any 'MG unit' ?! \n",currMgUnitId);
    return 0;/* dummy just for compiler */
}

/* Macro to set pointer with value if the pointer is not NULL */
#define SET_PTR_IF_NOT_NULL(param)  if(param##Ptr)(*(param##Ptr))=param


/* number of Ravens under single pipe in Falcon */
#define FALCON_NUM_RAVENS_PER_PIPE_CNS  2
/* number of channel groups in Raven */
#define RAVEN_NUM_CHANNEL_GROUPS_CNS  2
/* number of channels per group in Raven (without CPU port) */
#define RAVEN_NUM_CHANNEL_PORTS_CNS  FALCON_PORTS_PER_DP /*8*/
/* number of ports in Raven (without CPU port) */
#define RAVEN_NUM_PORTS_CNS (RAVEN_NUM_CHANNEL_GROUPS_CNS * RAVEN_NUM_CHANNEL_PORTS_CNS) /*16*/

/* Convert global port to Raven die index in the device */
#define RAVEN_INDEX_GET(dev, port) \
    ((port) / RAVEN_NUM_PORTS_CNS)  /* Raven die index [0..15] */

/* Convert Falcon port to Raven channel group in die */
#define RAVEN_CHANNEL_GROUP_GET(dev, port) \
    (((port) % RAVEN_NUM_PORTS_CNS) / RAVEN_NUM_CHANNEL_PORTS_CNS)   /* Channel group index [0..1] */

/* Convert Falcon port to Raven channel index in die */
#define RAVEN_CHANNEL_INDEX_GET(dev, port) \
    (((port) % RAVEN_NUM_PORTS_CNS) % RAVEN_NUM_CHANNEL_PORTS_CNS)   /* channel index [0..7]       */
/**
* @internal smemRavenGlobalPortConvertGet function
* @endinternal
*
* @brief   convert global MAC number to Global Raven number , local MAC number in Raven....
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] globalPortNum            - the global MAC number
*
* @param[out] globalRavenNumPtr        - (pointer to) the global Raven number.
* @param[out] relativeRavenNumInPipePtr- (pointer to) the relative Raven number in Pipe.
* @param[out] localMacNumPtr           - (pointer to) the local MAC number in Raven.
* @param[out] localChannelIdInGroupPtr - (pointer to) the local channel Id in the channel group.
* @param[out] channelGroupPtr          - (pointer to) the local channel group.
* @param[out] isCpuPortPtr             - (pointer to) indication that the port is 'Cpu Port'
*
* @return none
*/
void smemRavenGlobalPortConvertGet
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32               globalPortNum,
    OUT GT_U32              *globalRavenNumPtr,
    OUT GT_U32              *relativeRavenNumInPipePtr,
    OUT GT_U32              *localMacNumPtr,
    OUT GT_U32              *localChannelIdInGroupPtr,
    OUT GT_U32              *localChannelGroupPtr,
    OUT GT_U32              *isCpuPortPtr
)
{
    GT_U32  globalRavenNum,localMacNum,localChannelIdInGroup,localChannelGroup,relativeRavenNumInPipe,isCpuPort;
    GT_U32 currentPipeId,currentTileId;
    SPECIAL_PORT_MAPPING_ENHANCED_CNS    *currInfoPtr;

    if(devObjPtr->numOfRavens == 0)
    {
        skernelFatalError("smemRavenGlobalPortConvertGet: the device not supports 'Ravens' \n");
    }


    if(devObjPtr->ravens_specialPortMappingArr)
    {
        /* check for 'special' port numbers */
        currInfoPtr = &devObjPtr->ravens_specialPortMappingArr[0];

        for (/*already init*/ ; currInfoPtr->globalPortNumber != SMAIN_NOT_VALID_CNS ; currInfoPtr++)
        {
            if(currInfoPtr->globalPortNumber == globalPortNum)
            {
                globalRavenNum          = currInfoPtr->unitIndex;
                localMacNum             = currInfoPtr->localPortNumber;
                localChannelIdInGroup   = currInfoPtr->extParamArr[0];
                localChannelGroup       = currInfoPtr->extParamArr[1];

                goto fillInfoToOutParam_lbl;
            }
        }
    }

    /******************************/
    /* not found in special cases */
    /******************************/
    globalRavenNum          = RAVEN_INDEX_GET(devObjPtr, globalPortNum);

    if(globalRavenNum >= devObjPtr->numOfRavens)
    {
        /* not valid port ... probably relate to 'SDMA' */
        globalRavenNum        = SMAIN_NOT_VALID_CNS;
        localChannelIdInGroup = SMAIN_NOT_VALID_CNS;
        localMacNum           = SMAIN_NOT_VALID_CNS;
        localChannelIdInGroup = SMAIN_NOT_VALID_CNS;
        localChannelGroup     = SMAIN_NOT_VALID_CNS;
        isCpuPort             = SMAIN_NOT_VALID_CNS;
        relativeRavenNumInPipe= SMAIN_NOT_VALID_CNS;
        SET_PTR_IF_NOT_NULL(globalRavenNum);
        SET_PTR_IF_NOT_NULL(relativeRavenNumInPipe);
        SET_PTR_IF_NOT_NULL(localMacNum);
        SET_PTR_IF_NOT_NULL(localChannelIdInGroup);
        SET_PTR_IF_NOT_NULL(localChannelGroup);
        SET_PTR_IF_NOT_NULL(isCpuPort);

        return;
    }

    localChannelIdInGroup   = RAVEN_CHANNEL_INDEX_GET(devObjPtr, globalPortNum);
    localChannelGroup       = RAVEN_CHANNEL_GROUP_GET(devObjPtr, globalPortNum);
    localMacNum             = globalPortNum % RAVEN_NUM_PORTS_CNS;

fillInfoToOutParam_lbl:
    relativeRavenNumInPipe  = globalRavenNum % FALCON_NUM_RAVENS_PER_PIPE_CNS;
    isCpuPort               = (localChannelIdInGroup == RAVEN_NUM_CHANNEL_PORTS_CNS);

    currentPipeId = smemGetCurrentPipeId(devObjPtr);
    /* Tile index [0..3] */
    currentTileId = currentPipeId / devObjPtr->numOfPipesPerTile;
    if ((1 << currentTileId) & devObjPtr->mirroredTilesBmp)
    {
        /* Inverse die index for tiles 1, 3 */
        relativeRavenNumInPipe = (FALCON_NUM_RAVENS_PER_PIPE_CNS/*2*/ - 1) - relativeRavenNumInPipe;
        /* NO mirroring inside the RAVEN !!! it is not mirrored !!!
            localChannelGroup      = (RAVEN_NUM_CHANNEL_GROUPS_CNS  - 1) - localChannelGroup;
        */
    }

    SET_PTR_IF_NOT_NULL(globalRavenNum);
    SET_PTR_IF_NOT_NULL(relativeRavenNumInPipe);
    SET_PTR_IF_NOT_NULL(localMacNum);
    SET_PTR_IF_NOT_NULL(localChannelIdInGroup);
    SET_PTR_IF_NOT_NULL(localChannelGroup);
    SET_PTR_IF_NOT_NULL(isCpuPort);

    return;
}

/**
* @internal smemRavenGlobalPortToRavenIndexGet function
* @endinternal
*
* @brief   Get Raven index according to the port number.
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] globalPortNum            - the global port number
*
* return the Raven index
*/
GT_U32 smemRavenGlobalPortToRavenIndexGet
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32   globalPortNum
)
{
    GT_U32  globalRavenNum;

    smemRavenGlobalPortConvertGet(devObjPtr,globalPortNum,
        &globalRavenNum,NULL,NULL,NULL,NULL,NULL);

    return globalRavenNum;
}

/**
* @internal smemRavenGlobalPortToChannelGroupGet function
* @endinternal
*
* @brief   Get Raven local channel group according to the port number.
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] globalPortNum            - the global port number
*
* return the Raven local channel group
*/
GT_U32 smemRavenGlobalPortToChannelGroupGet
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32   globalPortNum
)
{
    GT_U32  localChannelGroup;

    smemRavenGlobalPortConvertGet(devObjPtr,globalPortNum,
        NULL,NULL,NULL,NULL,&localChannelGroup,NULL);

    return localChannelGroup;
}

/**
* @internal smemRavenMtiPortInterruptCauseRegisterGet function
* @endinternal
*
* @brief   Get Port<n> Interrupt Cause Register for sip6 devices
*
* @param[in] devObjPtr           - device object PTR.
* @param[in] portNum             - port number
*/
GT_U32 smemRavenMtiPortInterruptCauseRegisterGet
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32 portNum
)
{
    GT_U32 globalRavenNum,cpuPortIndex;
    GT_U32 isCpuPort;
    GT_U32 regAddress;
    GT_U32  is_mti_segmented;
    ENHANCED_PORT_INFO_STC portInfo;

    if(devObjPtr->numOfRavens == 0)/*Hawk*/
    {
        isCpuPort         = devObjPtr->portsArr[portNum].state == SKERNEL_PORT_STATE_MTI_CPU_E;
        if(isCpuPort)
        {
            devObjPtr->devMemPortInfoGetPtr(devObjPtr,SMEM_UNIT_TYPE_MTI_MAC_CPU_E  ,portNum,&portInfo);
            cpuPortIndex      = portInfo.simplePortInfo.unitIndex;
        }
        else
        {
            cpuPortIndex = 0;/* don't care */
        }
    }
    else
    {
        /* Convert physical port to Raven port, channel and die indexes */
        smemRavenGlobalPortConvertGet(devObjPtr, portNum,
                                      &globalRavenNum,
                                      NULL, NULL, NULL, NULL,
                                      &isCpuPort);
        cpuPortIndex = globalRavenNum;
    }

    if(isCpuPort)
    {
        regAddress = SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI_cpu_port[cpuPortIndex].MTI_EXT_PORT.portInterruptCause;
    }
    else
    {
        /* get indication if the port is segmented or not */
        if((devObjPtr->deviceFamily == SKERNEL_HAWK_FAMILY) && (SMEM_MTI_UNIT_TYPE_GET(devObjPtr,portNum) == SMEM_UNIT_TYPE_MTI_MAC_USX_E))
        {
            regAddress = SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI[portNum].USX_MAC_PORT.portInterruptCause;
        }
        else
        {
            is_mti_segmented = snetChtPortMacFieldGet(devObjPtr, portNum,
                SNET_CHT_PORT_MAC_FIELDS_is_mti_segmented_E);
            regAddress = is_mti_segmented ?
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI[portNum].MTI_EXT_PORT.segPortInterruptCause :
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI[portNum].MTI_EXT_PORT.portInterruptCause;
        }
    }

    return regAddress;
}

/**
* @internal smemGenericUnitAlloc function
* @endinternal
*
* @brief   function to alloc units in the unitsArr[]
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] unitsArr[]               - array of units info to alloc
*                                       None.
*/
void smemGenericUnitAlloc(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN  SMEM_DEVICE_UNIT_ALLOCATION_STC unitsArr[]
)
{
    GT_U32  ii;
    SMEM_CHT_GENERIC_DEV_MEM_INFO *devMemInfoPtr = devObjPtr->deviceMemory;
    SMEM_UNIT_CHUNKS_STC    *currUnitChunkPtr;
    GT_U32  unitIndex;

    for(ii = 0 ; unitsArr[ii].unitNameStr ; ii++ )
    {
        if(SMAIN_NOT_VALID_CNS == smemUnitBaseAddrByNameGet(devObjPtr,unitsArr[ii].unitNameStr,1))
        {
            /* support Phoenix that get CP units from Hawk , and need to skip the SHM unit */
            continue;
        }

        unitIndex = UNIT_INDEX_FROM_STRING_GET_MAC(devObjPtr,unitsArr[ii].unitNameStr);
        currUnitChunkPtr = &devMemInfoPtr->unitMemArr[unitIndex];
        if(currUnitChunkPtr->numOfChunks == 0)
        {
            unitsArr[ii].allocUnitFunc(devObjPtr,currUnitChunkPtr);
        }
    }
}

/**
* @internal smemGenericUnitSizeCheck function
* @endinternal
*
* @brief   check that no unit exceed the size of it's unit
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] unitInfoArr[]            - array of units info
*/
void smemGenericUnitSizeCheck (
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SMEM_GEN_UNIT_INFO_STC   unitInfoArr[]
)
{
    GT_U32  ii,jj;
    SMEM_CHT_GENERIC_DEV_MEM_INFO *devMemInfoPtr = devObjPtr->deviceMemory;
    SMEM_UNIT_CHUNKS_STC    *currUnitChunkPtr;
    SMEM_CHUNK_STC    *memChunkPtr;
    GT_U32  minAddr,maxAddr;
    GT_CHAR* unitNamePtr;
    SMEM_GEN_UNIT_INFO_STC *unitInfoPtr = &unitInfoArr[0];

    for(ii = 0 ; unitInfoPtr->size != SMAIN_NOT_VALID_CNS; ii++,unitInfoPtr++)
    {
        jj = UNIT_INDEX_FROM_ADDR_GET_MAC(devObjPtr,unitInfoPtr->base_addr);

        currUnitChunkPtr = &devMemInfoPtr->unitMemArr[jj];

        minAddr = unitInfoPtr->base_addr;
        maxAddr = unitInfoPtr->base_addr + unitInfoPtr->size;
        unitNamePtr = smemUnitNameByAddressGet(devObjPtr,minAddr);

        if(strcmp(unitInfoPtr->nameStr,unitNamePtr))
        {
            simForcePrintf("Unit [%s] got 'other name' [%s] from the base addr [0x%8.8x]\n",
                unitInfoPtr->nameStr , unitNamePtr , minAddr);
        }

        memChunkPtr = &currUnitChunkPtr->chunksArray[0];
        for(jj = 0 ; jj < currUnitChunkPtr->numOfChunks ; jj++ , memChunkPtr++)
        {
            if(minAddr > memChunkPtr->memFirstAddr)
            {
                simForcePrintf("Unit [%s] hold address [0x%8.8x] that is lower than unit's base address [0x%8.8x] \n",
                    unitNamePtr , memChunkPtr->memFirstAddr ,
                    minAddr);
            }

            if(maxAddr <= memChunkPtr->memLastAddr)
            {
                simForcePrintf("Unit [%s] hold address [0x%8.8x] that is higher than unit's max address [0x%8.8x] (relate to start addr[0x%8.8x] , unit size is[0x%8.8x]) \n",
                    unitNamePtr , memChunkPtr->memLastAddr ,
                    maxAddr , memChunkPtr->memFirstAddr,
                    unitInfoPtr->size);
            }
        }

    }
}


static GT_CHAR* hiddenMemNames[SMEM_GENERIC_HIDDEN_MEM___LAST___E+1] = {
     STR(FDB      )/*SMEM_GENERIC_HIDDEN_MEM_FDB_E      */
    ,STR(FDB_TILE1)/*SMEM_GENERIC_HIDDEN_MEM_FDB_TILE1_E*/
    ,STR(FDB_TILE2)/*SMEM_GENERIC_HIDDEN_MEM_FDB_TILE2_E*/
    ,STR(FDB_TILE3)/*SMEM_GENERIC_HIDDEN_MEM_FDB_TILE3_E*/
    ,STR(EXACT_MATCH       )/*SMEM_GENERIC_HIDDEN_MEM_EXACT_MATCH_E       */
    ,STR(EXACT_MATCH_TILE1 )/*SMEM_GENERIC_HIDDEN_MEM_EXACT_MATCH_TILE1_E */
    ,STR(EXACT_MATCH_TILE2 )/*SMEM_GENERIC_HIDDEN_MEM_EXACT_MATCH_TILE2_E */
    ,STR(EXACT_MATCH_TILE3 )/*SMEM_GENERIC_HIDDEN_MEM_EXACT_MATCH_TILE3_E */

    ,NULL
};
/**
* @internal smemGenericHiddenFindMem function
* @endinternal
*
* @brief   Get pointer to the proper entry index in the specified hidden memory
* @param[in] deviceObj                - pointer to device object.
* @param[in] memType                  - the memory type of hidden memory.
* @param[in] entryIndex               - the index in the hidden memory.
* @param[out] instanceIdPtr           - (pointer to)the instance ID (ignored if NULL).
*
*/
GT_U32 * smemGenericHiddenFindMem
(
    IN SKERNEL_DEVICE_OBJECT        * devObjPtr,
    IN SMEM_GENERIC_HIDDEN_MEM_ENT  memType,
    IN GT_U32                       entryIndex,
    OUT GT_U32                      *instanceIdPtr
)
{
    SMEM_GENERIC_HIDDEN_MEM_STC *infoPtr;
    GT_U32  instanceId = 0;
    GT_U32  pipeId,tileId;

    if(!SMEM_CHT_IS_SIP6_GET(devObjPtr))
    {
        skernelFatalError("smemGenericHiddenFindMem: implemented only for SIP6 devices  \n");
    }

    switch(memType)
    {
        case SMEM_GENERIC_HIDDEN_MEM_FDB_E      :
        case SMEM_GENERIC_HIDDEN_MEM_EXACT_MATCH_E:
            pipeId = smemGetCurrentPipeId(devObjPtr);
            smemConvertGlobalPipeIdToTileAndLocalPipeIdInTile(devObjPtr,pipeId,&tileId);
            instanceId = tileId;
            break;
        case SMEM_GENERIC_HIDDEN_MEM_FDB_TILE1_E:
        case SMEM_GENERIC_HIDDEN_MEM_FDB_TILE2_E:
        case SMEM_GENERIC_HIDDEN_MEM_FDB_TILE3_E:
        case SMEM_GENERIC_HIDDEN_MEM_EXACT_MATCH_TILE1_E:
        case SMEM_GENERIC_HIDDEN_MEM_EXACT_MATCH_TILE2_E:
        case SMEM_GENERIC_HIDDEN_MEM_EXACT_MATCH_TILE3_E:
        case SMEM_GENERIC_HIDDEN_MEM_AAC_MEM_CHANNEL0_E:
        case SMEM_GENERIC_HIDDEN_MEM_AAC_MEM_CHANNEL1_E:
        case SMEM_GENERIC_HIDDEN_MEM_AAC_MEM_CHANNEL2_E:
        case SMEM_GENERIC_HIDDEN_MEM_AAC_MEM_CHANNEL3_E:
        case SMEM_GENERIC_HIDDEN_MEM_AAC_MEM_CHANNEL4_E:
        case SMEM_GENERIC_HIDDEN_MEM_AAC_MEM_CHANNEL5_E:
        case SMEM_GENERIC_HIDDEN_MEM_AAC_MEM_CHANNEL6_E:
        case SMEM_GENERIC_HIDDEN_MEM_AAC_MEM_CHANNEL7_E:
            instanceId = 0;/* explicit access to the tile */
            break;
        default:
            skernelFatalError("smemGenericHiddenFindMem: try to access memType [%d] that is unknown \n",
            memType);
    }

    if((memType + instanceId) >= SMEM_GENERIC_HIDDEN_MEM___LAST___E)
    {
        skernelFatalError("smemGenericHiddenFindMem: memType[%s] with instanceId[%d] cause to out of range \n",
            hiddenMemNames[memType],instanceId);
    }


    infoPtr = SMEM_HIDDEN_MEM_INFO_GET(devObjPtr,memType,instanceId);
    if(infoPtr->startOfMemPtr == NULL)
    {
        skernelFatalError("smemGenericHiddenFindMem: memType[%s] hold no pointer to memory \n",
            hiddenMemNames[memType]);
    }

    if(instanceIdPtr)
    {
        *instanceIdPtr = instanceId;
    }

    return infoPtr->startOfMemPtr +
           (infoPtr->alignmentOfEntryInWords * entryIndex);
}
/**
* @internal smemGenericHiddenMemSet function
* @endinternal
*
* @brief   Write data to a register or table.
* @param[in] devObjPtr                - pointer to device object.
* @param[in] memType                  - the memory type of hidden memory.
* @param[in] entryIndex               - the index in the hidden memory.
* @param[in] dataPtr                  - new data location for memory .
* @param[in] dataSize                 - size of memory location to be updated.
*
* @note The function aborts application if memory/index not exist.
*
*/
void smemGenericHiddenMemSet
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SMEM_GENERIC_HIDDEN_MEM_ENT  memType,
    IN GT_U32                       entryIndex,
    IN GT_U32        *         dataPtr,
    IN GT_U32                  dataSize
)
{
    GT_U32                  * findMemPtr;
    GT_U32                    ii;
    GT_U32                    instanceId;

    findMemPtr = smemGenericHiddenFindMem(devObjPtr,memType,entryIndex,&instanceId);

    /* Write memory to the register's or tables's memory */
    for (ii = 0; ii < dataSize; ii++)
    {
        /* log memory information */
        simGenericHiddenLogMemory(devObjPtr, SIM_LOG_MEMORY_WRITE_E, SIM_LOG_MEMORY_DEV_E,
                        hiddenMemNames[memType],instanceId,entryIndex,
                        dataPtr[ii], findMemPtr[ii]);
        findMemPtr[ii] = dataPtr[ii];
    }
}


uint32_t* pha_findMem(uint32_t addr)
{
    MEMORY_RANGE *currentMemEntryPtr;
    uint32_t ii;
    uint32_t    sizeof_uint32_t = sizeof(uint32_t);
    static uint32_t dummyReg = 0x00badadd;
    SKERNEL_DEVICE_OBJECT *devObjPtr = current_fw_devObjPtr;

    if(addr & 0x3)
    {
        skernelFatalError("Try to access non-aligned address[0x%8.8x] \n",addr);
        return &dummyReg;
    }

    if(!devObjPtr || !devObjPtr->PHA_FW_support.pha_memoryMap)
    {
        skernelFatalError("The device not hold needed support for PHA memory \n");
        return &dummyReg;
    }


    currentMemEntryPtr = &devObjPtr->PHA_FW_support.pha_memoryMap[0];

    for(ii = 0 ; currentMemEntryPtr->numOfBytes != 0 ;ii++,currentMemEntryPtr++)
    {
        if(addr >= currentMemEntryPtr->startAddr &&
           addr < currentMemEntryPtr->startAddr + (currentMemEntryPtr->numOfBytes))
        {
#if 0 /* too much printings */
            __LOG(("Access to Address[0x%4.4x] (offset of [0x%4.4x] from start of [%s])\n",
                    addr,
                    addr - currentMemEntryPtr->startAddr,
                    currentMemEntryPtr->namePtr));
#endif
            if(currentMemEntryPtr->actualMemoryPtr == NULL)
            {
                currentMemEntryPtr->actualMemoryPtr = (uint32_t*)smemDeviceObjMemoryAlloc(devObjPtr,1,currentMemEntryPtr->numOfBytes+(sizeof_uint32_t-1));
                /*  NOTE:
                    fatal error occur if no memory to allocate .. so no check needed here */
            }

            return &currentMemEntryPtr->actualMemoryPtr[(addr - currentMemEntryPtr->startAddr)/sizeof_uint32_t];
        }
    }

    skernelFatalError("Address[0x%8.8x] was not found in the memory \n",addr);
    return &dummyReg;
}

static ACTIVE_MEM_FUNC * pha_findActiveMem(uint32_t addr , READ_OR_WRITE readOrWrite)
{
    DECLARE_FUNC_NAME(pha_findActiveMem);

    SKERNEL_DEVICE_OBJECT *devObjPtr = current_fw_devObjPtr;/* needed for __LOG */
    ACTIVE_MEMORY_RANGE *currentMemEntryPtr;
    uint32_t ii,jj;


    currentMemEntryPtr = &devObjPtr->PHA_FW_support.pha_activeMemoryMap[0];

    for(ii = 0 ; currentMemEntryPtr->numOfRegisters != 0 ;ii++,currentMemEntryPtr++)
    {
        for(jj = 0 ; jj < currentMemEntryPtr->numOfRegisters ; jj++)
        {
            if(addr == currentMemEntryPtr->startAddr + (jj * currentMemEntryPtr->stepsBetweenRegisters))
            {
                if(readOrWrite == READ_OPER)
                {
                    if(currentMemEntryPtr->readFuncPtr)
                    {
                        __LOG(("Found 'Read' Active Address[0x%8.8x] relative to [%s]\n",
                                addr,
                                currentMemEntryPtr->namePtr));

                        return currentMemEntryPtr->readFuncPtr;
                    }
                }
                else
                {
                    if(currentMemEntryPtr->writeFuncPtr)
                    {
                        __LOG(("Found 'Write' Active Address[0x%8.8x] relative to [%s]\n",
                                addr,
                                currentMemEntryPtr->namePtr));

                        return currentMemEntryPtr->writeFuncPtr;
                    }
                }

                /* address found but not hold active memory for the current 'read/write' operation */
                return NULL;
            }
        }
    }

    /* address not found to hold any active memory for 'read/write' operation */
    return NULL;
}

static void ppaFwSpWriteMask(uint32_t addr,uint32_t data,uint32_t mask)
{
    uint32_t *memPtr = pha_findMem(addr);

    uint32_t memWord = ((*memPtr) & (~mask)) | /*clear the bits */
                        (data & mask); /*add the new bits*/
    ACTIVE_MEM_FUNC *activeMemPtr = pha_findActiveMem(addr,WRITE_OPER);
    if(activeMemPtr)
    {
        activeMemPtr(addr,memPtr,memWord);
    }
    else
    {
        *memPtr = memWord;
    }
}
void ppaFwSpWrite(uint32_t addr,uint32_t data)
{
    uint32_t swappedData = (data);
    ppaFwSpWriteMask(addr,swappedData,0xFFFFFFFF);
}


void ppaFwSpShortWrite(uint32_t addr,uint16_t data)
{
    uint32_t swappedData = (data);
    uint32_t shift = 16 - ((addr % 4) * 8);
    uint32_t newAddr = addr & 0xFFFFFFFC;/* the shift hold compensation about the address */

    ppaFwSpWriteMask(newAddr,swappedData<<shift,0xFFFF<<shift);
}
void ppaFwSpByteWrite(uint32_t addr,uint8_t data)
{
    uint32_t swappedData = (data);
    uint32_t shift = 24 - ((addr % 4) * 8);
    uint32_t newAddr = addr & 0xFFFFFFFC;/* the shift hold compensation about the address */

    ppaFwSpWriteMask(newAddr,swappedData<<shift,0xFF<<shift);
}
static uint32_t ppaFwSpReadMask(uint32_t addr,uint32_t mask)
{
    uint32_t *memPtr = pha_findMem(addr);

    ACTIVE_MEM_FUNC *activeMemPtr = pha_findActiveMem(addr,READ_OPER);
    if(activeMemPtr)
    {
        uint32_t memWord = activeMemPtr(addr,memPtr,0/*dummy*/);
        return (memWord) & mask;
    }
    else
    {
        return (*memPtr) & mask;
    }
}

uint32_t ppaFwSpRead(uint32_t addr)
{
    uint32_t readData = ppaFwSpReadMask(addr,0xFFFFFFFF);

    return readData;
}
uint16_t ppaFwSpShortRead(uint32_t addr)
{
    uint32_t shift = 16 - ((addr % 4) * 8);
    uint32_t newAddr = addr & 0xFFFFFFFC;/* the shift hold compensation about the address */
    uint32_t readData = ppaFwSpReadMask(newAddr,0xFFFF<<shift);

    return (uint16_t)(readData>>shift);
}
uint8_t ppaFwSpByteRead(uint32_t addr)
{
    uint32_t shift = 24 - ((addr % 4) * 8);
    uint32_t newAddr = addr & 0xFFFFFFFC;/* the shift hold compensation about the address */
    uint32_t readData = ppaFwSpReadMask(newAddr,0xFF<<shift);

    return (uint8_t)(readData>>shift);
}


static char* accelOperNameGet(uint32_t regOffset)
{
    uint32_t ii;
    ACCEL_INFO_STC *accelInfoArr;
    SKERNEL_DEVICE_OBJECT * devObjPtr = current_fw_devObjPtr;

    accelInfoArr = devObjPtr->PHA_FW_support.pha_acceleratorInfoPtr;

    for(ii = 0 ; accelInfoArr[ii].operationNamePtr != NULL ; ii++)
    {
        if(accelInfoArr[ii].registerOffset == regOffset)
        {
            if(accelInfoArr[ii].threadId == 0 ||             /* support PIPE   */
               accelInfoArr[ii].threadId == current_fw_ThreadId) /* support Falcon */
            {
                /* NOTE: the values in the ARRAY hold "enum_" as prefix */
                /* let's jump over it ... hence the '+5' */

                return accelInfoArr[ii].operationNamePtr + 5;
            }
        }
    }

    return NULL;
}

/* accumulator as part of accelerator for checksum calculation  */
static uint32_t acceleratorTwoByteOnesComplementSum(IN uint32_t startAddr ,IN uint32_t   length /*bytes*/)
{
    SKERNEL_DEVICE_OBJECT * devObjPtr = current_fw_devObjPtr;/*needed for __LOG_NO_LOCATION_META_DATA*/
    uint16_t    ii;
    uint32_t    u32_checksum = 0;
    uint16_t    u16Value;

    __LOG_NO_LOCATION_META_DATA(("Calc checksum : read '2 bytes' from address = [0x%4.4x] length [%d] bytes\n",
        startAddr,length));
    for(ii = 0 ; ii < length; ii += 2)/* address jump by 2 bytes */
    {
        u16Value = ppaFwSpShortRead(startAddr + ii);
        __LOG_NO_LOCATION_META_DATA(("u16Value = [0x%4.4x]\n",
            u16Value));
        u32_checksum += u16Value;
    }

    return u32_checksum;
}

typedef enum {ZONE_TYPE_PKT,ZONE_TYPE_DESC , ZONE_TYPE_CONFIG} ZONE_TYPE;

static char* zoneNames[] = {
     STR(ZONE_TYPE_PKT)
    ,STR(ZONE_TYPE_DESC)
    ,STR(ZONE_TYPE_CONFIG)
};

/* NOTE : the H file included from Falcon hold more options than those for PIPE */
#include <asicSimulation/SKernel/sEmbeddedCpu/firmware/falcon_pha/Default/ppa_fw_accelerator_commands.h>

#define val_ACCEL_CMD_SRC_CONST     ACCEL_CMD_SRC_CONST(1)

/* 2 checksums as 'singletone' protected by the SCIB mutex */
static uint32_t     checksum    = 0;

uint32_t pha_fw_activeMem_write_ACCEL_CMDS_TRIG_BASE_ADDR(uint32_t addr, uint32_t *memPtr , uint32_t writeValue)
{
    DECLARE_FUNC_NAME(pha_fw_activeMem_write_ACCEL_CMDS_TRIG_BASE_ADDR);
    uint32_t ii;
    SKERNEL_DEVICE_OBJECT * devObjPtr = current_fw_devObjPtr;
    uint32_t offsetFromBase = addr - devObjPtr->PHA_FW_support.addr_ACCEL_CMDS_TRIG_BASE_ADDR/*01040*/;
    uint32_t addrOfData     = devObjPtr->PHA_FW_support.addr_ACCEL_CMDS_CFG_BASE_ADDR/*40*/ + offsetFromBase;
    uint32_t *acceleratorDataPtr;
    uint32_t acceleratorData;
    uint32_t srcByte;
    uint32_t srcByteType;
    uint32_t dstByte;
    uint32_t dstByteType;
    uint32_t length;
    uint32_t srcBit;
    uint32_t dstBit;
    ZONE_TYPE srcZone;/* descriptor zone or metadata zone */
    ZONE_TYPE dstZone;/* descriptor zone or metadata zone */
    uint32_t value,value2;
    uint32_t *srcMemPtr;
    uint32_t *dstMemPtr;
    char* accelOperName = accelOperNameGet(offsetFromBase);
    uint32_t  srcAddr;
    uint32_t  dstAddr;
    uint32_t  constValue;
    uint32_t  useConst;
    uint32_t  isBitOperation;/* 1 - bit operation , 0 - bytes operation */
    uint32_t  accelerator_packet_header_access_offset; /* value used for offset in packets */
    uint32_t  zero_remaining_destination; /*Affects bit field only <can be the base for a clean byte field op> */
    uint16_t  old_checksum;
    uint32_t  isCopyBits;

    __LOG(("start acceleration operation : %s \n",accelOperName));

    acceleratorDataPtr = pha_findMem(addrOfData);
    acceleratorData = *acceleratorDataPtr;

    /*Accelerator Packet Header Access Offset*/
    if(SMEM_CHT_IS_SIP6_GET(devObjPtr))
    {
        /* <header offset ignore> */
        if(writeValue & 1) /* bit 0*/
        {
            /* ignore the offset from the register !!! */
            accelerator_packet_header_access_offset = 0;

            __LOG(("NOTE: the accelerator_packet_header_access_offset considered ZERO for current operation \n"));
        }
        else    /* get the offset from the register (default) */
        {
            smemRegFldGet(devObjPtr ,
                SMEM_SIP6_PHA_PPN_PKT_HEADER_ACCESS_OFFSET_REG(devObjPtr),
                0,8,&accelerator_packet_header_access_offset);
        }

        zero_remaining_destination = (writeValue & 2) ? 1 : 0;  /* bit 1 */
    }
    else /* PIPE */
    {
        /* get the offset from the register */
        smemRegFldGet(devObjPtr ,
            SMEM_PIPE_PHA_PPN_PKT_HEADER_ACCESS_OFFSET_REG(devObjPtr),
            0,8,&accelerator_packet_header_access_offset);

        zero_remaining_destination = 0;
    }


    /* update the value in the register ... but it has no effect on the actual operation */
    *memPtr = writeValue;

    if(val_ACCEL_CMD_SRC_CONST & acceleratorData)
    {
        constValue = (SMEM_U32_GET_FIELD(acceleratorData,20,3) << 8) | (SMEM_U32_GET_FIELD(acceleratorData,0,8));
        useConst = 1;
    }
    else
    {
        constValue = 0;
        useConst = 0;
    }

    if(!useConst)
    {
        srcByte     = SMEM_U32_GET_FIELD(acceleratorData,0,7); /*ACCEL_CMD_SRC_BYTE()*/
        srcByteType = SMEM_U32_GET_FIELD(acceleratorData,7,1); /*ACCEL_CMD_SRC_BYTE_TYPE()*/
    }
    else
    {
        srcByte     = 0;/*don't care*/
        srcByteType = 0;/*don't care*/
    }

    dstByte     = SMEM_U32_GET_FIELD(acceleratorData,8,7); /*ACCEL_CMD_DST_BYTE()*/
    dstByteType = SMEM_U32_GET_FIELD(acceleratorData,15,1);/*ACCEL_CMD_DST_BYTE_TYPE()*/

    if(!useConst)
    {
        if(srcByteType == PKT)
        {
            srcAddr = devObjPtr->PHA_FW_support.addr_PKT_REGs_lo;
            srcZone = ZONE_TYPE_PKT;

            srcByte += accelerator_packet_header_access_offset;
        }
        else
        {
            srcZone = (srcByte & 64) ? ZONE_TYPE_DESC : ZONE_TYPE_CONFIG;
            srcByte &= ~64;

            srcAddr = (srcZone == ZONE_TYPE_DESC) ? devObjPtr->PHA_FW_support.addr_DESC_REGs_lo : devObjPtr->PHA_FW_support.addr_CFG_REGs_lo;

            if(srcZone == ZONE_TYPE_DESC &&
               devObjPtr->PHA_FW_support.phaFwApplyChangesInDescriptor)
            {
                /* we need to reload values that we saved in the descriptor */
                /* to get 'up-to-date' values */
                devObjPtr->PHA_FW_support.phaFwApplyChangesInDescriptor(devObjPtr,GT_FALSE);
            }
        }
    }
    else
    {
        srcAddr = 0;/*don't care*/
        if(srcByteType == PKT)
        {
            srcZone = ZONE_TYPE_PKT;
        }
        else
        {
            srcZone = (srcByte & 64) ? ZONE_TYPE_DESC : ZONE_TYPE_CONFIG;
        }
    }

    if(dstByteType == PKT)
    {
        dstAddr = devObjPtr->PHA_FW_support.addr_PKT_REGs_lo;
        dstZone = ZONE_TYPE_PKT;

        dstByte += accelerator_packet_header_access_offset;
    }
    else
    {
        dstZone = (dstByte & 64) ? ZONE_TYPE_DESC : ZONE_TYPE_CONFIG;
        dstByte &= ~64;

        dstAddr = (dstZone == ZONE_TYPE_DESC) ? devObjPtr->PHA_FW_support.addr_DESC_REGs_lo : devObjPtr->PHA_FW_support.addr_CFG_REGs_lo;
    }


    if(!useConst)
    {
        srcMemPtr = pha_findMem(srcAddr);
        __LOG(("SRC of operation = [%s] byte [%d] \n",
            zoneNames[srcZone],
            srcByte));
    }
    else
    {
        srcMemPtr = NULL;

        __LOG(("SRC of operation = [CONST] value [%d] \n",
            constValue));
    }

    dstMemPtr = pha_findMem(dstAddr);
    __LOG(("DEST of operation = [%s] byte [%d] \n",
        zoneNames[dstZone],
        dstByte));

    isBitOperation = (acceleratorData & (1 << 26)) ? 1 : 0;

    if(isBitOperation)
    {
        /* bits 26..30 (not include bit 31 !)*/
        /* bit 31 in Falcon used as 'MSBit' of 'length' for 'bitField' operation */
        switch(acceleratorData & (0x1f << 26))
        {
            case                           ACCEL_CMD_BIT_SUB:
            case                           ACCEL_CMD_BIT_ADD:
            case val_ACCEL_CMD_SRC_CONST | ACCEL_CMD_BIT_SUB:
            case val_ACCEL_CMD_SRC_CONST | ACCEL_CMD_BIT_ADD:
            case val_ACCEL_CMD_SRC_CONST | ACCEL_CMD_BIT_COPY:
                /***************/
                /* new in sip6 */
                /***************/
                if(!SMEM_CHT_IS_SIP6_GET(devObjPtr))
                {
                    goto unknownAccelerator_lbl;
                }
                /* fall through ... */

            case                           ACCEL_CMD_BIT_COPY:
                /* copy bits : copy any contiguous bit field of up to 16 bits
                    from anywhere to anywhere in the packet Register file ,
                    with the following exceptions:
                    1. Cannot copy a bit field from "packet" to "packet" (requires a 2nd huge mux)
                    2. Source or destination cannot cross a 4B bound
                */
                isCopyBits = 0;
                /* bits 26..29 (not include bit 30)*/
                /* bit 30 in Falcon used as 'SRC const' indication */
                switch((acceleratorData & (0x0f << 26)))
                {
                    case ACCEL_CMD_BIT_COPY:
                        __LOG(("copy value (bits operation):\n"));
                        isCopyBits = 1;
                        break;

                    case ACCEL_CMD_BIT_ADD:
                        __LOG(("add value (bits operation):\n"));
                        break;

                    case ACCEL_CMD_BIT_SUB:
                        __LOG(("subtract value (bits operation):\n"));
                        break;
                }

                if(isCopyBits &&
                   dstZone == ZONE_TYPE_PKT &&
                   srcZone == ZONE_TYPE_PKT)
                {
                    /* operation not supported */
                    __LOG(("ERROR : Cannot copy a bit field from 'packet' to 'packet' (requires a 2nd huge mux) \n"));
                    break;
                }

                length = 1 + SMEM_U32_GET_FIELD(acceleratorData,16,4);/*ACCEL_CMD_LEN() / ACCEL_CMD_LEN_BIT() */
                if(SMEM_CHT_IS_SIP6_GET(devObjPtr))
                {
                    /* bit 31 in Falcon used as 'MSBit' of 'length' for 'bitField' operation */
                    /* ACCEL_CMD_LEN_BIT() */
                    if(SMEM_U32_GET_FIELD(acceleratorData,31,1))
                    {
                        length += 16;
                    }
                }

                if(!useConst)
                {
                    srcBit = SMEM_U32_GET_FIELD(acceleratorData,20,3);/*ACCEL_CMD_SRC_BIT*/
                }
                else
                {
                    srcBit = 0;/*don't care*/
                }
                dstBit = SMEM_U32_GET_FIELD(acceleratorData,23,3);/*ACCEL_CMD_DST_BIT*/

                __LOG_PARAM(length);
                if(!useConst)
                {
                    __LOG_PARAM(srcBit);
                }
                __LOG_PARAM(dstBit);

                srcBit += srcByte * 8;
                dstBit += dstByte * 8;

                /* the offsets are in BIG endian order (bytes and bits) */
                /* need to adjust it according actual endianity of our CPU (that runs the simulation) */
                /* in BIG endian    : the 'srcBit' is start bit + 'length' , but */
                /* in little endian : the 'srcBit' is END bit - 'length' */
                /* meaning that if need to remove the length from the 'srcBit' (and from the 'dstBit')*/
                /* but this removal is only after getting '31 bits complement' ... */
                if(length)
                {
                    GT_U32  complemet31;

                    if(!useConst)
                    {
                        /*handle src*/
                        complemet31 = 31 - (srcBit & 0x1f);
                        /* clear the 5 LSBits , and set with proper offset*/
                        srcBit = (srcBit & 0xffffffe0) + (complemet31 - (length-1));
                    }

                    /*handle dst*/
                    complemet31 = 31 - (dstBit & 0x1f);
                    /* clear the 5 LSBits , and set with proper offset*/
                    dstBit = (dstBit & 0xffffffe0) + (complemet31 - (length-1));
                }

                if(!srcMemPtr || useConst)
                {
                    value = constValue;

                    __LOG(("use 'CONST' value [0x%x] \n",
                        value));
                }
                else
                {
                    /* get value from the 'src' */
                    value = snetFieldValueGet(srcMemPtr,srcBit,length);
                }

                /* bits 26..29 (not include bit 30)*/
                /* bit 30 in Falcon used as 'SRC const' indication */
                switch((acceleratorData & (0x0f << 26)))
                {
                    case ACCEL_CMD_BIT_COPY:
                        __LOG((" copy bits : value[0x%x] ended \n", value));
                        break;

                    case ACCEL_CMD_BIT_ADD:
                        value2 = snetFieldValueGet(dstMemPtr,dstBit,length);
                        __LOG((" add value in bits : value[0x%x] added to [0x%x] (total [0x%x])ended \n",
                            value,
                            value2 ,
                            value2 + value));
                        value2 += value;

                        value = value2;
                        break;

                    case ACCEL_CMD_BIT_SUB:
                        value2 = snetFieldValueGet(dstMemPtr,dstBit,length);
                        __LOG((" subtract value in bits : value[0x%x] subtract from [0x%x] (total [0x%x])ended \n",
                            value,
                            value2 ,
                            value2 - value));
                        value2 -= value;

                        value = value2;
                        break;
                }

                if(zero_remaining_destination)
                {
                    /* clear bits around the WORD that the 'dst field' belongs to */
                    dstMemPtr[dstBit>>5] = 0;
                }
                /* set value to the 'dst' */
                snetFieldValueSet(dstMemPtr,dstBit,length,value);


                break;
            default:
                unknownAccelerator_lbl:
                /* unknown operation ?! */
                skernelFatalError("Accelerator : unknown operation , register[0x%8.8x] value[0x%8.8x] \n",
                    addrOfData , acceleratorData);
                break;
        }
    }
    else
    if(acceleratorData & (0x3f << 26)) /* any checksum operation */
    {
        if((acceleratorData & ACCEL_CMD_CSUM_STORE) == ACCEL_CMD_CSUM_STORE)
        {
            if(((checksum & 0x80000000) == 0) &&
                checksum >> 16)
            {
                checksum = 1 + (checksum & 0xFFFF) + (checksum >> 16) ;
            }

            checksum = 0 - checksum;
            if(acceleratorData & ACCEL_CMD_CSUM_STORE_IS_UDP(1))
            {
                /*******************************/
                /* The UDP do next extra logic */
                /*******************************/
                if(checksum == 0x0)
                {
                    checksum = 0xFFFF; /* ~0x0 */
                }

                __LOG(("UDP checksum 'store' operation : value [0x%4.4x]\n",
                    checksum));
            }
            else
            {
                __LOG(("checksum 'store' operation : value [0x%4.4x]\n",
                    checksum));
            }

            /*  The command stores ~SC in the specified location.
                Then SC is reset: SC=0
            */

            /* set value to the 'dst' */
            ppaFwSpShortWrite(dstAddr + dstByte, checksum);

            checksum = 0;
        }
        else
        if(acceleratorData & ACCEL_CMD_CSUM_LOAD)
        {
            /* length is bytes !!! */
            length = 1 + SMEM_U32_GET_FIELD(acceleratorData,16,5);/* ACCEL_CMD_LEN_BYTE() */
            /* this is the number of bytes , but the value must be even ... ignore the LSBit */
            length &= ~(1);

            old_checksum = checksum;
            /*****************************************/
            /* Csum_loads can support length up to 8 */
            /*****************************************/
            if(length > 8)
            {
                /* operation not supported */
                __LOG(("ERROR : Csum_loads can support length up to 8 (got value[%d])  \n",
                    length));
            }
            else
            if(acceleratorData & ACCEL_CMD_CSUM_LOAD_IS_OLD(1))
            {
                checksum -= acceleratorTwoByteOnesComplementSum(srcAddr + srcByte , length);

                __LOG(("checksum 'load old' operation : for [%d] bytes checksum changed from[0x%4.4x] to [0x%4.4x]\n",
                    length,
                    old_checksum,
                    checksum));
            }
            else
            {
                checksum += acceleratorTwoByteOnesComplementSum(srcAddr + srcByte , length);

                __LOG(("checksum 'load new' operation : for [%d] bytes checksum changed from[0x%4.4x] to [0x%4.4x]\n",
                    length,
                    old_checksum,
                    checksum));
            }

        }
        else
        {
            goto unknownAccelerator_lbl;
        }


    }
    else /* BYTE operation */
    {
        /* copy bytes : copy any contiguous byte field, up to 32B long, in any of the following directions:
            Packet -> Packet
            Packet -> Configuration
            Configuration -> Packet
            Configuration -> Configuration
        */
        __LOG(("copy bytes:\n"));

        length = 1 + SMEM_U32_GET_FIELD(acceleratorData,16,5);/* ACCEL_CMD_LEN() / ACCEL_CMD_LEN_BYTE() */

        __LOG(("start to Copy [%d] bytes :", length));

        /* LOG the the bytes .
            we do it before the actual action so we have it in proper bytes order
            regardless to actual copy order.
        */
        if(simLogIsOpenFlag)
        {
            for(ii = 0 ; ii < length; ii++)
            {
                /* get value from the 'src' */
                value = ppaFwSpByteRead(srcAddr+ (srcByte+ii));
                __LOG(("0x%2.2x,",value));
            }
        }

        /* a copy that requires 'reverse' */
        if((dstMemPtr == srcMemPtr) &&
           (srcByte < dstByte) &&
           ((srcByte + length) > dstByte))
        {
            srcByte += (length - 1);
            dstByte += (length - 1);

            /* need to copy in reverse to avoid overrun byte that not copied yet */
            for(ii = 0 ; ii < length; ii++)
            {
                /* get value from the 'src' */
                value = ppaFwSpByteRead(srcAddr+ (srcByte-ii));

                /* set value to the 'dst' */
                ppaFwSpByteWrite(dstAddr+ (dstByte-ii),value);
            }
        }
        else
        {
            for(ii = 0 ; ii < length; ii++)
            {
                /* get value from the 'src' */
                value = ppaFwSpByteRead(srcAddr+ (srcByte+ii));

                /* set value to the 'dst' */
                ppaFwSpByteWrite(dstAddr+ (dstByte+ii),value);
            }
        }

        __LOG(("\n copy bytes ended \n"));
    }

    if(dstZone == ZONE_TYPE_DESC &&
       devObjPtr->PHA_FW_support.phaFwApplyChangesInDescriptor )
    {
        /* we must reload the changes into the descriptor object */
        devObjPtr->PHA_FW_support.phaFwApplyChangesInDescriptor(devObjPtr,GT_TRUE);
    }


    __LOG(("ended acceleration operation : %s \n",accelOperName));

    return 0;/* dummy --> return value is only for 'read' active memories */
}

extern GT_STATUS simulationAc5pFirmwareThreadsInfoSet
(
    IN SKERNEL_DEVICE_OBJECT *devObjPtr,
    IN GT_U32  threadId,/*0..63*/
    IN GT_U32  instruction_pointer,
    IN GT_U32  firmwareImageId,
    IN GT_U32  firmwareVersionId,
    IN GT_BOOL firstTime
);
extern GT_STATUS simulationFalconFirmwareThreadsInfoSet
(
    IN SKERNEL_DEVICE_OBJECT *devObjPtr,
    IN GT_U32  threadId,/*0..63*/
    IN GT_U32  instruction_pointer,
    IN GT_U32  firmwareImageId,
    IN GT_U32  firmwareVersionId,
    IN GT_BOOL firstTime
);
extern GT_STATUS simulationPipeFirmwareThreadIdToInstructionPointerSet
(
    IN GT_U32  threadId,/*0..31*/
    IN GT_U32 instruction_pointer
);



extern void smemSohoDirectAccess_wm_internal(
    IN  GT_U8    hostDeviceNumber,
    IN  GT_U8    deviceNumber,
    IN  GT_U32   DevAddr,
    IN  GT_U32   RegAddr,
    IN  GT_U32   data,/*for write*/
    IN  GT_BOOL   doRead/*GT_TRUE - read , GT_FALSE - write*/
);
extern void wm__simDisconnectAllOuterPorts(void);
extern void smemCheetahUpdateCoreClockRegister
(
    IN SKERNEL_DEVICE_OBJECT * deviceObjPtr,
    IN GT_U32                  coreClockInMHz,
    IN GT_U32                  hwFieldValue
);
extern GT_STATUS skernelPortLinkStatusChange
(
    IN  GT_U32   deviceNumber,
    IN  GT_U32   portNumber,
    IN  GT_BOOL  newStatus
);

void smemUpdateCoreClockRegister
(
    IN GT_U32   wmDevNum,
    IN GT_U32   coreClockInMHz
)
{
    SKERNEL_DEVICE_OBJECT* devObjPtr = smemTestDeviceIdToDevPtrConvert(wmDevNum);
    smemCheetahUpdateCoreClockRegister(devObjPtr,coreClockInMHz,0);
}

/* wrapper to info  from skernelDevPortSlanGet to put on the 'IPC' with the CPSS */
static GT_STATUS wm__skernelDevPortSlanGet
(
    IN  SKERNEL_DEVICE_OBJECT * devObjPtr,/* param needed for the response */
    IN  GT_U32                deviceId,
    IN  GT_U32                portNum
)
{
    GT_CHAR              slanName[13];
    GT_BOOL              portBound;
    GT_U32               regVal[3];
    GT_U32               ii;

    /* get the info */
    skernelDevPortSlanGet(deviceId,portNum,12,&portBound,&slanName[0]);

    if(portBound == GT_FALSE)
    {
        regVal[0] = regVal[1] = regVal[2] = 0;
    }
    else
    {
        for(ii = 0 ; ii < 3 ; ii++)
        {
            regVal[ii] =  slanName[ii*4 + 0] << 24 |
                          slanName[ii*4 + 1] << 16 |
                          slanName[ii*4 + 2] <<  8 |
                          slanName[ii*4 + 3] <<  0  ;
        }
    }

    /* put the info into the response memory */
    smemChtAnswerToIpcCpssToWm(devObjPtr->deviceId,regVal[0],regVal[1],regVal[2]);

    return GT_OK;
}

/* wrapper to info from skernelBindDevPort2Slan to put on the 'IPC' with the CPSS */
static GT_STATUS wm__skernelBindDevPort2Slan
(
    IN  GT_U32                      deviceId,
    IN  GT_U32                      portNum,
    IN  GT_U32                      unbindOtherPortsOnThisSlan,
    IN  GT_U32                      *regVlauePtr/* the slan name */
)
{
    GT_CHAR              slanName[13];
    GT_U32               ii;

    for(ii = 0 ; ii < 3 ; ii ++)
    {
        slanName[ii*4 + 0] = (GT_CHAR)(regVlauePtr[ii] >> 24);
        slanName[ii*4 + 1] = (GT_CHAR)(regVlauePtr[ii] >> 16);
        slanName[ii*4 + 2] = (GT_CHAR)(regVlauePtr[ii] >>  8);
        slanName[ii*4 + 3] = (GT_CHAR)(regVlauePtr[ii] >>  0);
    }
    slanName[12] = 0;

    skernelBindDevPort2Slan(deviceId,portNum,slanName,
        unbindOtherPortsOnThisSlan?GT_TRUE:GT_FALSE);

    return GT_OK;
}

extern GT_STATUS wmForDebugDmaRead64BitAddr(
    IN GT_U32   addr_high,
    IN GT_U32   addr_low,
    IN GT_U32   numOfWords
);
extern GT_STATUS wmForDebugDmaWrite64BitAddr(
    IN GT_U32   addr_high,
    IN GT_U32   addr_low,
    IN GT_U32   numOfWords,
    IN GT_U32   pattern
);

/*  enum to state the 'IPC of CPSS to WM' message types.
    SIM_IPC_TYPE_STRING_E - a free style string (str) that will be operated on the WM side with : osShellExecute(str)
    SIM_IPC_TYPE_PHA_THREAD_PIPE_E - message for PHA for threadId,firmwareInstructionPointer in pipe
    SIM_IPC_TYPE_PHA_THREAD_FALCON_E - message for PHA for threadId,firmwareInstructionPointer in falcon
    SIM_IPC_TYPE_PHA_THREAD_AC5P_AC5X_HARRIER_E - message for PHA for threadId,firmwareInstructionPointer in Ac5p,Ac5x and Harrier
    SIM_IPC_TYPE_DX_BYPASS_SOHO_DIRECT_REG_E - message for 'DX device' to by bypassed and trigger the SOHO address directly.
    SIM_IPC_TYPE_DISCONNECT_ALL_OUTER_PORTS_E - message for all devices to disconnect from outer ports
    SIM_IPC_TYPE_CORE_CLOCK_SET_E - message for the devices to set coreClock
    SIM_IPC_TYPE_SLAN_NAME_GET_E  - message to get from WN the name of SLAN of the port
    SIM_IPC_TYPE_SLAN_CONNECT_E   - message to connect SLAN to a port
    SIM_IPC_TYPE_SLAN_DISCONNECT_E- message to disconnect SLAN from a port
    SIM_IPC_TYPE_PORT_LINK_CHANGED_E - message to notify WM to change the link status of a port
    SIM_IPC_TYPE_TEST_SCIB_DMA_WRITE_64_BIT_ADDR_E - message to notify WM to start write to DMA to test it.
    SIM_IPC_TYPE_TEST_SCIB_DMA_READ_64_BIT_ADDR_E  - message to notify WM to start read from DMA to test it.

*/
enum {
    SIM_IPC_TYPE_STRING_E               = 0,
    SIM_IPC_TYPE_PHA_THREAD_PIPE_E,
    SIM_IPC_TYPE_PHA_THREAD_FALCON_E,
    SIM_IPC_TYPE_PHA_THREAD_AC5P_AC5X_HARRIER_E,
    SIM_IPC_TYPE_DX_BYPASS_SOHO_DIRECT_REG_E,
    SIM_IPC_TYPE_DISCONNECT_ALL_OUTER_PORTS_E,
    SIM_IPC_TYPE_CORE_CLOCK_SET_E,
    SIM_IPC_TYPE_SLAN_NAME_GET_E,
    SIM_IPC_TYPE_SLAN_CONNECT_E,
    SIM_IPC_TYPE_SLAN_DISCONNECT_E,
    SIM_IPC_TYPE_PORT_LINK_CHANGED_E,
    SIM_IPC_TYPE_TEST_SCIB_DMA_WRITE_64_BIT_ADDR_E,
    SIM_IPC_TYPE_TEST_SCIB_DMA_READ_64_BIT_ADDR_E,

    SIM_IPC_TYPE_PHA_THREAD___LAST___E = 0x7fffffff
};
/**
* @internal smemChtActiveWriteIpcCpssToWm function
* @endinternal
*
* @brief   The CPSS triggered 'string' to operate on the WM.
*/
void smemChtActiveWriteIpcCpssToWm (
    IN         SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN         GT_U32   address,
    IN         GT_U32   memSize,
    IN         GT_U32 * memPtr,
    IN         GT_UINTPTR   param,
    IN         GT_U32 * inMemPtr
)
{
#define NUM_OF_BYTES_CNS    (256-4*(1/*last address (trigger)*/ + 3/*3 registers for 'Get' operation*/))
    char    stringFromRegister[NUM_OF_BYTES_CNS+1];
    GT_U32  *startOfMemoryPtr;
    GT_U32  ii;

    if((* inMemPtr) != 0xFFFFFFFF)
    {
        * memPtr = * inMemPtr;
        return;
    }

    /* reset previous 3 'get' answers */
    startOfMemoryPtr = smemMemGet(devObjPtr,address & 0xFFFFFFF0);
    for(ii = 0 ; ii < 3 ; ii++)
    {
        startOfMemoryPtr[ii] = 0;
    }

    /* start getting info about the new operation */
    startOfMemoryPtr = smemMemGet(devObjPtr,address & 0xFFFFFF00);

    if(startOfMemoryPtr[0] != SIM_IPC_TYPE_STRING_E)
    {
        GT_U32  params[NUM_OF_BYTES_CNS/4 - 1];
        GT_U32  enumType = startOfMemoryPtr[0];
        GT_U32  numParams= startOfMemoryPtr[1];

        for(ii = 0 ; ii < numParams && ii < (NUM_OF_BYTES_CNS/4) ; ii++)
        {
            params[ii] = startOfMemoryPtr[2 + ii];
        }

        switch(enumType)
        {
            case SIM_IPC_TYPE_PHA_THREAD_PIPE_E:
                simulationPipeFirmwareThreadIdToInstructionPointerSet(params[0],params[1]);
                break;
            case SIM_IPC_TYPE_PHA_THREAD_FALCON_E:
                simulationFalconFirmwareThreadsInfoSet(devObjPtr,params[0],params[1],params[2],params[3],params[4]);
                break;
            case SIM_IPC_TYPE_PHA_THREAD_AC5P_AC5X_HARRIER_E:
                simulationAc5pFirmwareThreadsInfoSet(devObjPtr,params[0],params[1],params[2],params[3],params[4]);
                break;
            case SIM_IPC_TYPE_DX_BYPASS_SOHO_DIRECT_REG_E:
                smemSohoDirectAccess_wm_internal(devObjPtr->deviceId/*hostDeviceNumber*/,
                    params[0],  /*deviceNumber*/
                    params[1],  /*DevAddr     */
                    params[2],  /*RegAddr     */
                    params[3],  /*data        */
                    params[4]); /*doRead      */
                break;
            case SIM_IPC_TYPE_DISCONNECT_ALL_OUTER_PORTS_E:
                wm__simDisconnectAllOuterPorts(/*no params*/);
                break;
/*          the case removed as for JIRA : CPSS-11748 : WM Simulation: Can't simulate non-default clock speed
            case SIM_IPC_TYPE_CORE_CLOCK_SET_E:
                smemCheetahUpdateCoreClockRegister(devObjPtr,params[1],params[2]);
                break;
*/
            case SIM_IPC_TYPE_SLAN_NAME_GET_E:
                /* wrapper to skernelDevPortSlanGet */
                wm__skernelDevPortSlanGet(devObjPtr,params[0],params[1]);
                break;
            case SIM_IPC_TYPE_SLAN_CONNECT_E:
                /* wrapper to skernelBindDevPort2Slan */
                wm__skernelBindDevPort2Slan(params[0],params[1],params[2],&params[3]);
                break;
            case SIM_IPC_TYPE_SLAN_DISCONNECT_E:
                skernelUnbindDevPort2Slan(params[0],params[1]);
                break;
            case SIM_IPC_TYPE_PORT_LINK_CHANGED_E:
                skernelPortLinkStatusChange(params[0],params[1],params[2]);
                break;

            case SIM_IPC_TYPE_TEST_SCIB_DMA_WRITE_64_BIT_ADDR_E:
                wmForDebugDmaWrite64BitAddr(params[0],params[1],params[2],params[3]);
                break;

            case SIM_IPC_TYPE_TEST_SCIB_DMA_READ_64_BIT_ADDR_E:
                wmForDebugDmaRead64BitAddr(params[0],params[1],params[2]);
                break;

            default:
                simGeneralPrintf("WM Triggered for unknown 'enum' type: [%d] \n",enumType);
                break;
        }
    }
    else
    {
        startOfMemoryPtr++;

        for(ii = 0 ; ii < (NUM_OF_BYTES_CNS/4) ; ii++)
        {
            stringFromRegister[ii*4 + 0] = (startOfMemoryPtr[ii] >> 24) & 0xFF;
            stringFromRegister[ii*4 + 1] = (startOfMemoryPtr[ii] >> 16) & 0xFF;
            stringFromRegister[ii*4 + 2] = (startOfMemoryPtr[ii] >>  8) & 0xFF;
            stringFromRegister[ii*4 + 3] = (startOfMemoryPtr[ii] >>  0) & 0xFF;
        }

        stringFromRegister[ii*4] = 0;

        simGeneralPrintf("WM Triggered for : [%s] \n",stringFromRegister);

        SIM_OS_MAC(simOsShellExecute)(stringFromRegister) ;
    }

    * memPtr = 0;
}

/**
* @internal smemChtAnswerToIpcCpssToWm function
* @endinternal
*
* @brief   The CPSS triggered 'string' to operate on the WM.
*/
void smemChtAnswerToIpcCpssToWm (
    IN GT_U32   wmDevNum,
    IN GT_U32   answer1,
    IN GT_U32   answer2,
    IN GT_U32   answer3
)
{
    SKERNEL_DEVICE_OBJECT* devObjPtr = smemTestDeviceIdToDevPtrConvert(wmDevNum);
    GT_U32  baseAddr;
    GT_U32  *startOfMemoryPtr;

    if(devObjPtr->devMemUnitNameAndIndexPtr)
    {
        baseAddr = smemUnitBaseAddrByNameGet(devObjPtr,STR(UNIT_MG),0);
    }
    else
    {
        /* sip4 devices not support this (and MG is at addr 0) */
        baseAddr = 0;
    }

    startOfMemoryPtr = smemMemGet(devObjPtr,baseAddr + 0x000FFFF0);
    startOfMemoryPtr[0] = answer1;
    startOfMemoryPtr[1] = answer2;
    startOfMemoryPtr[2] = answer3;

    return;
}

/* Flag is set when simulation SoftReset is done */
extern GT_U32 simulationSoftResetDone;
/**
* @internal smemChtAnswerToIpcCpssToWm function
* @endinternal
*
* @brief   The CPSS triggered 'string' to operate on the WM.
*/
void simulationSoftResetDoneGet (
    IN GT_U32   wmDevNum
)
{
    /* answer to the remote caller */
    smemChtAnswerToIpcCpssToWm(wmDevNum,simulationSoftResetDone,0,0);
}


/**
* @internal smemDevSoftResetCommonStart function
* @endinternal
*
* @brief   common operations for sip5,sip6,AC5 on triggering soft reset
*          called in context of 'active memory'
*          (called from :
*           smemFalconActiveWriteCnmRfuSystemSoftResetReg ,
*           smemLion3ActiveWriteDfxServerResetControlReg
*
*/
void smemDevSoftResetCommonStart(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32                  globalControlRegisterContent
)
{
    DECLARE_FUNC_NAME(smemDevSoftResetCommonStart);

    SBUF_BUF_ID bufferId;               /* Buffer ID */
    GT_U8  * dataPtr;                   /* Pointer to the data in the buffer */
    GT_U32 dataSize;                    /* Data size */

    simulationSoftResetDone = 0;

    /* Get buffer + suspend the pool to ignore other allocations */
    bufferId = sbufAllocAndPoolSuspend(devObjPtr->bufPool, sizeof(GT_U32));

    if (bufferId == NULL)
    {
        simWarningPrintf("smemDevSoftResetCommonStart: no buffers to trigger soft reset \n");
        return;
    }


    /**********************************************************************/
    /* NOTE: skernelDeviceSoftResetGeneric(...) will process this message */
    /**********************************************************************/

    /* allow using __LOG like for packets .. to debug the 'soft reset' feature */
    SIM_OS_MAC(simOsTaskOwnTaskPurposeSet)(SIM_OS_TASK_PURPOSE_TYPE_PP_PIPE_PROCESSING_DAEMON_E, NULL);

    __LOG(("Soft reset detected by the 'active memory' and starting operations \n"));

    {

        /* according to PP designer :
            even when <PEX Skip Init if MG Soft Reset> = SKIP INIT ON (no pex reset)

            if the CPU will try to access the device while it is doing 'soft reset' :
            Behavior in this case in un-predictable (probably the device will hang)

            so simulation is 'unbind' from SCIB ... so trying to access to it will cause 'FATAL ERROR' !!!
        */

        __LOG(("Unbind SCIB interface for device[%d]\n",
            devObjPtr->deviceId));
        /*************************/
        /* Unbind SCIB interface */
        /*************************/
        scibUnBindDevice(devObjPtr->deviceId);
    }

    /******************************************************************/
    /* prepare message for the device to start process the soft reset */
    /******************************************************************/

    /* Get actual data pointer */
    sbufDataGet(bufferId, &dataPtr, &dataSize);

    /* Copy Global Control Register to buffer */
    memcpy(dataPtr, &globalControlRegisterContent, sizeof(GT_U32));

    /* Set source type of buffer */
    bufferId->srcType = SMAIN_SRC_TYPE_CPU_E;

    /* Set message type of buffer */
    bufferId->dataType = SMAIN_MSG_TYPE_SOFT_RESET_E;

    __LOG(("Put buffer to queue  + suspend the queue \n"));
    /* Put buffer to queue  + suspend the queue to ignore other buffers
       trying to ingress the queue (from other tasks in the device)


       NOTE: that CPU can not do any 'Access to PP' because we did 'Unbind SCIB interface'
       and it will be re-open only when we process the soft reset in
       skernelDeviceSoftResetGeneric(...)
    */
    squeBufPutAndQueueSuspend(devObjPtr->queueId, SIM_CAST_BUFF(bufferId));

    /*restore the thread 'purpose' */
    SIM_OS_MAC(simOsTaskOwnTaskPurposeSet)(SIM_OS_TASK_PURPOSE_TYPE_CPU_APPLICATION_E, NULL);
}

/**
* @internal smemMallocAndCopyActiveTable function
* @endinternal
*
* @brief   malloc new table according to orig 'active memory table and copy the
*          orig to the duplicated one.
*          NOTE: the new malloc is done into the 'device memory' , so it is freed
*          as part of 'soft reset' (or when device ends/fatal error)
*/
SMEM_ACTIVE_MEM_ENTRY_STC* smemMallocAndCopyActiveTable(
    IN SKERNEL_DEVICE_OBJECT        * devObjPtr,
    IN const SMEM_ACTIVE_MEM_ENTRY_STC * orgActiveTablePtr,
    IN GT_U32                       numOfEntries
)
{
    SMEM_ACTIVE_MEM_ENTRY_STC* dupTablePtr;
    GT_U32  ii;

    dupTablePtr = smemDeviceObjMemoryAlloc(devObjPtr,numOfEntries,sizeof(SMEM_ACTIVE_MEM_ENTRY_STC));
    for(ii = 0 ; ii < numOfEntries; ii++)
    {
        dupTablePtr[ii] = orgActiveTablePtr[ii];
    }

    return dupTablePtr;
}

